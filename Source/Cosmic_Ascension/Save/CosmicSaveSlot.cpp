// Cosmic Ascension - Save layer.

#include "Save/CosmicSaveSlot.h"

#include "Compression/OodleDataCompression.h"
#include "Containers/Map.h"
#include "Math/NumericLimits.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Crc.h"
#include "Save/CosmicSaveGame.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicSaveSlotImpl
{
	// The header into the first HeaderBytes of Out: the 60 bytes field by field (FArchive does not swap on Win64:
	// little-endian), then their CRC.
	void WriteHead(TArray<uint8>& Out, const CosmicSaveSlot::FHeader& H)
	{
		TArray<uint8> Head;
		FMemoryWriter Writer(Head);
		uint32 Magic = CosmicSaveSlot::Magic;
		uint16 Form = H.Form;
		uint8 Codec = static_cast<uint8>(H.Codec);
		uint8 Reserved = 0;   // byte 7: written 0, never read
		FGuid ID = H.GalaxyID;
		int64 SavedAt = H.SavedAtUtcTicks;
		int64 Tick = H.Tick;
		int64 Raw = H.RawBytes;
		int64 Packed = H.PackedBytes;
		uint32 PayloadCrc = H.PayloadCrc;
		Writer << Magic << Form << Codec << Reserved << ID << SavedAt << Tick << Raw << Packed << PayloadCrc;
		check(Head.Num() == CosmicSaveSlot::HeaderCrcBytes);
		uint32 HeaderCrc = FCrc::MemCrc32(Head.GetData(), Head.Num());
		Writer << HeaderCrc;
		check(Head.Num() == CosmicSaveSlot::HeaderBytes && Out.Num() >= CosmicSaveSlot::HeaderBytes);
		FMemory::Memcpy(Out.GetData(), Head.GetData(), CosmicSaveSlot::HeaderBytes);
	}
}

bool CosmicSaveSlot::IsDamage(ECosmicSlotRead Read)
{
	switch (Read)
	{
	case ECosmicSlotRead::Missing:
	case ECosmicSlotRead::Short:
	case ECosmicSlotRead::OldForm:
	case ECosmicSlotRead::NotOurs:
	case ECosmicSlotRead::HeaderDamaged:
	case ECosmicSlotRead::BadSizes:
	case ECosmicSlotRead::Truncated:
	case ECosmicSlotRead::OtherGalaxy:
	case ECosmicSlotRead::PayloadDamaged:
	case ECosmicSlotRead::CodecRefused:
	case ECosmicSlotRead::Unreadable:
		return true;
	case ECosmicSlotRead::Ok:
	case ECosmicSlotRead::ReadFailed:
	case ECosmicSlotRead::NewerForm:
	case ECosmicSlotRead::NewerSave:
	case ECosmicSlotRead::Sanctuary:
	case ECosmicSlotRead::Refused:
		return false;
	}
	return false;
}

const TCHAR* CosmicSaveSlot::NameOf(ECosmicSlotRead Read)
{
	switch (Read)
	{
	case ECosmicSlotRead::Ok: return TEXT("Ok");
	case ECosmicSlotRead::Missing: return TEXT("Missing");
	case ECosmicSlotRead::Short: return TEXT("Short");
	case ECosmicSlotRead::OldForm: return TEXT("OldForm");
	case ECosmicSlotRead::NotOurs: return TEXT("NotOurs");
	case ECosmicSlotRead::HeaderDamaged: return TEXT("HeaderDamaged");
	case ECosmicSlotRead::BadSizes: return TEXT("BadSizes");
	case ECosmicSlotRead::Truncated: return TEXT("Truncated");
	case ECosmicSlotRead::OtherGalaxy: return TEXT("OtherGalaxy");
	case ECosmicSlotRead::PayloadDamaged: return TEXT("PayloadDamaged");
	case ECosmicSlotRead::CodecRefused: return TEXT("CodecRefused");
	case ECosmicSlotRead::Unreadable: return TEXT("Unreadable");
	case ECosmicSlotRead::ReadFailed: return TEXT("ReadFailed");
	case ECosmicSlotRead::NewerForm: return TEXT("NewerForm");
	case ECosmicSlotRead::NewerSave: return TEXT("NewerSave");
	case ECosmicSlotRead::Sanctuary: return TEXT("Sanctuary");
	case ECosmicSlotRead::Refused: return TEXT("Refused");
	}
	return TEXT("?");
}

bool CosmicSaveSlot::ParseSlotName(const FString& SlotName, FGuid& OutGalaxyID, bool& bOutBackup)
{
	static const FString Stem(TEXT("Galaxy_"));
	static const FString BackupSuffix(TEXT("_bak"));
	constexpr int32 DigitsLen = 32;
	const int32 MainLen = Stem.Len() + DigitsLen;
	const bool bMain = SlotName.Len() == MainLen;
	const bool bBackup = SlotName.Len() == MainLen + BackupSuffix.Len();
	if ((!bMain && !bBackup) || !SlotName.StartsWith(Stem, ESearchCase::CaseSensitive)
		|| (bBackup && !SlotName.EndsWith(BackupSuffix, ESearchCase::CaseSensitive)))
	{
		return false;
	}
	FGuid ID;
	if (!FGuid::ParseExact(SlotName.Mid(Stem.Len(), DigitsLen), EGuidFormats::Digits, ID) || !ID.IsValid())
	{
		return false;
	}
	OutGalaxyID = ID;
	bOutBackup = bBackup;
	return true;
}

int64 CosmicSaveSlot::MaxSlotBytes()
{
	return HeaderBytes + FOodleDataCompression::CompressedBufferSizeNeeded(MaxRawBytes);
}

bool CosmicSaveSlot::Encode(UCosmicSaveGame& Save, int64 SavedAtUtcTicks, TArray<uint8>& OutSlot, int64* OutRawBytes, ECodec Want)
{
	OutSlot.Reset();
	TArray<uint8> Raw;
	if (!UGameplayStatics::SaveGameToMemory(&Save, Raw) || Raw.IsEmpty() || Raw.Num() > MaxRawBytes)
	{
		return false;
	}
	if (OutRawBytes)
	{
		*OutRawBytes = Raw.Num();
	}
	FHeader H;
	H.Form = FormVersion;
	H.GalaxyID = Save.State.GalaxyID;
	H.SavedAtUtcTicks = SavedAtUtcTicks;
	H.Tick = Save.State.Tick;
	H.RawBytes = Raw.Num();
	int64 Packed = 0;
	if (Want == ECodec::Oodle)
	{
		const int64 Bound = FOodleDataCompression::CompressedBufferSizeNeeded(Raw.Num());
		if (Bound > 0 && HeaderBytes + Bound < MAX_int32)
		{
			OutSlot.SetNumUninitialized(HeaderBytes + static_cast<int32>(Bound));
			Packed = FOodleDataCompression::Compress(OutSlot.GetData() + HeaderBytes, Bound, Raw.GetData(), Raw.Num(),
				FOodleDataCompression::ECompressor::Kraken, FOodleDataCompression::ECompressionLevel::VeryFast);
		}
	}
	if (Packed > 0)
	{
		H.Codec = ECodec::Oodle;
		OutSlot.SetNum(HeaderBytes + static_cast<int32>(Packed), EAllowShrinking::No);
	}
	else
	{
		// The codec gave nothing (or a test asked for it): the engine bytes as they are.
		H.Codec = ECodec::Stored;
		Packed = Raw.Num();
		OutSlot.SetNumUninitialized(HeaderBytes + Raw.Num());
		FMemory::Memcpy(OutSlot.GetData() + HeaderBytes, Raw.GetData(), Raw.Num());
	}
	H.PackedBytes = Packed;
	H.PayloadCrc = FCrc::MemCrc32(OutSlot.GetData() + HeaderBytes, static_cast<int32>(Packed));
	CosmicSaveSlotImpl::WriteHead(OutSlot, H);
	return true;
}

ECosmicSlotRead CosmicSaveSlot::ReadHeader(TConstArrayView<uint8> Head, int64 FileBytes, const FGuid* Expected, FHeader& OutHeader)
{
	OutHeader = FHeader();
	if (FileBytes < HeaderBytes || Head.Num() < HeaderBytes)
	{
		return ECosmicSlotRead::Short;
	}
	FMemoryReaderView Reader(MakeArrayView(Head.GetData(), HeaderBytes));
	uint32 First = 0;
	uint16 Form = 0;
	uint8 Codec = 0;
	uint8 Reserved = 0;
	FHeader H;
	Reader << First << Form << Codec << Reserved << H.GalaxyID << H.SavedAtUtcTicks << H.Tick << H.RawBytes << H.PackedBytes << H.PayloadCrc << H.HeaderCrc;
	if (First == EngineMagic)
	{
		return ECosmicSlotRead::OldForm;
	}
	if (First != Magic)
	{
		return ECosmicSlotRead::NotOurs;
	}
	// The envelope's CRC before the form: a flipped Form byte is damage, not a later form.
	if (FCrc::MemCrc32(Head.GetData(), HeaderCrcBytes) != H.HeaderCrc)
	{
		return ECosmicSlotRead::HeaderDamaged;
	}
	H.Form = Form;
	H.Codec = static_cast<ECodec>(Codec);
	OutHeader = H;
	if (Form > FormVersion)
	{
		// Only the envelope (GalaxyID, SavedAtUtcTicks) is to be trusted in OutHeader now.
		return ECosmicSlotRead::NewerForm;
	}
	const bool bKnownCodec = Codec == static_cast<uint8>(ECodec::Stored) || Codec == static_cast<uint8>(ECodec::Oodle);
	if (Form == 0 || !bKnownCodec || H.RawBytes < 1 || H.RawBytes > MaxRawBytes || H.PackedBytes <= 0
		|| (H.Codec == ECodec::Stored && H.RawBytes != H.PackedBytes))
	{
		return ECosmicSlotRead::BadSizes;
	}
	if (FileBytes != HeaderBytes + H.PackedBytes)
	{
		return ECosmicSlotRead::Truncated;
	}
	if (Expected && *Expected != H.GalaxyID)
	{
		return ECosmicSlotRead::OtherGalaxy;
	}
	return ECosmicSlotRead::Ok;
}

ECosmicSlotRead CosmicSaveSlot::Check(TConstArrayView<uint8> Slot, const FGuid* Expected, FHeader& OutHeader)
{
	const ECosmicSlotRead Read = ReadHeader(Slot, Slot.Num(), Expected, OutHeader);
	if (Read != ECosmicSlotRead::Ok)
	{
		return Read;
	}
	// The length check came first: the CRC reads inside the buffer only.
	if (FCrc::MemCrc32(Slot.GetData() + HeaderBytes, static_cast<int32>(OutHeader.PackedBytes)) != OutHeader.PayloadCrc)
	{
		return ECosmicSlotRead::PayloadDamaged;
	}
	return ECosmicSlotRead::Ok;
}

ECosmicSlotRead CosmicSaveSlot::Unpack(TConstArrayView<uint8> Slot, const FGuid* Expected, FHeader& OutHeader, TArray<uint8>& OutRaw)
{
	OutRaw.Reset();
	const ECosmicSlotRead Read = Check(Slot, Expected, OutHeader);
	if (Read != ECosmicSlotRead::Ok)
	{
		return Read;
	}
	// Zeroed: whatever the codec leaves, the engine's reader never sees memory this process did not write.
	OutRaw.SetNumZeroed(static_cast<int32>(OutHeader.RawBytes));
	if (OutHeader.Codec == ECodec::Oodle)
	{
		if (!FOodleDataCompression::Decompress(OutRaw.GetData(), OutHeader.RawBytes, Slot.GetData() + HeaderBytes, OutHeader.PackedBytes))
		{
			OutRaw.Reset();
			return ECosmicSlotRead::CodecRefused;
		}
	}
	else
	{
		FMemory::Memcpy(OutRaw.GetData(), Slot.GetData() + HeaderBytes, OutHeader.RawBytes);
	}
	return ECosmicSlotRead::Ok;
}

FGuid CosmicSaveSlot::NewestRun(TConstArrayView<FSeen> Seen)
{
	TMap<FGuid, int64> Latest;
	for (const FSeen& Each : Seen)
	{
		int64& At = Latest.FindOrAdd(Each.GalaxyID, TNumericLimits<int64>::Lowest());
		At = FMath::Max(At, Each.UtcTicks);
	}
	FGuid Best;
	int64 BestAt = 0;
	FString BestText;
	for (const TPair<FGuid, int64>& Each : Latest)
	{
		const FString Text = Each.Key.ToString(EGuidFormats::Digits);
		if (!Best.IsValid() || Each.Value > BestAt || (Each.Value == BestAt && Text > BestText))
		{
			Best = Each.Key;
			BestAt = Each.Value;
			BestText = Text;
		}
	}
	return Best;
}
