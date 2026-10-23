// Cosmic Ascension - Save layer (Core Logic). Design: docs/20 section 9.1, 16 #70, #33 (2) (3), headers/A5_save_minimum.md.
// RULE: no UI/World include. The slot form: the engine's save bytes (UGameplayStatics::SaveGameToMemory) packed with Oodle
// behind a checked 64-byte header. Pure functions over bytes - no file is read or written here, no state changed, nothing
// hashed or drawn. Win64 byte order.

#pragma once

#include "CoreMinimal.h"
#include "Containers/ArrayView.h"
#include "Misc/Guid.h"

class UCosmicSaveGame;

/** What a slot was found to be, in the order of the checks. Missing..Unreadable are damage: the load then tries the backup
 * (16 #33 (2)). ReadFailed, NewerForm, NewerSave, Sanctuary and Refused refuse without trying it: a slot that did not read,
 * or one this build cannot read, is never replaced by an older copy. Log text only and never saved; CosmicSaveSlot::IsDamage
 * names its members one by one (a switch, no range), so the order is for reading only. */
enum class ECosmicSlotRead : uint8
{
	Ok,
	Missing,         // no file of that name
	Short,           // fewer bytes than a header
	OldForm,         // the engine's own form ('GVAS' first): every slot from before MVP item 5
	NotOurs,         // another first word
	HeaderDamaged,   // HeaderCrc does not match (checked before Form: the envelope is the same in every form)
	BadSizes,        // Form 0, an unknown codec, RawBytes out of bounds, PackedBytes not positive, Stored with RawBytes != PackedBytes, or a file too large for a slot
	Truncated,       // the file is not exactly HeaderBytes + PackedBytes long (cut short, or bytes appended)
	OtherGalaxy,     // the header or the state names another galaxy than the slot name
	PayloadDamaged,  // PayloadCrc does not match
	CodecRefused,    // the codec refused the payload (Oodle returned 0); a CRC-true payload that decodes to another length is only a hand-made file (section 3.3) and stops at the engine's check
	Unreadable,      // the engine bytes give no UCosmicSaveGame
	ReadFailed,      // the file exists and did not read twice (a lock, an I/O error): not damage
	NewerForm,       // an intact header of a later slot form (Form above CosmicSaveSlot::FormVersion)
	NewerSave,       // SaveVersion above FGalaxyState::CurrentSaveVersion
	Sanctuary,       // the last system is not a kept sanctuary
	Refused,         // asked from a handler of a Core Logic delegate or inside the tick stages: nothing read
};

namespace CosmicSaveSlot
{
	inline constexpr uint32 Magic = 0x56534143;        // 'C' 'A' 'S' 'V' in file order
	inline constexpr uint32 EngineMagic = 0x53415647;  // UE_SAVEGAME_FILE_TYPE_TAG (GameplayStatics.cpp:87): 'GVAS' in file order
	inline constexpr uint16 FormVersion = 1;
	inline constexpr int32 HeaderBytes = 64;
	inline constexpr int32 HeaderCrcBytes = 60;        // HeaderCrc covers the bytes before it
	inline constexpr int64 MaxRawBytes = 64ll * 1024 * 1024;   // a bound before any allocation

	// The envelope every form keeps (byte offsets): a later build's slot is still seen by the continue and refused, never
	// taken for damage.
	inline constexpr int32 FormAt = 4;
	inline constexpr int32 GalaxyIDAt = 8;
	inline constexpr int32 SavedAtAt = 24;
	inline constexpr int32 HeaderCrcAt = 60;

	enum class ECodec : uint8 { Stored = 0, Oodle = 1 };   // in the file: never renumber, add at the end

	struct FHeader
	{
		uint16 Form = 0;
		ECodec Codec = ECodec::Stored;
		FGuid GalaxyID;
		int64 SavedAtUtcTicks = 0;
		int64 Tick = 0;
		int64 RawBytes = 0;
		int64 PackedBytes = 0;
		uint32 PayloadCrc = 0;
		uint32 HeaderCrc = 0;
	};

	/** One slot file as the continue sees it before any load. */
	struct FSeen
	{
		FGuid GalaxyID;
		bool bBackup = false;
		int64 UtcTicks = 0;            // the header's SavedAtUtcTicks when the header reads, else the file's time (order only)
		bool bTimeFromHeader = false;
	};

	COSMIC_ASCENSION_API bool IsDamage(ECosmicSlotRead Read);        // Missing through Unreadable, each by name
	COSMIC_ASCENSION_API const TCHAR* NameOf(ECosmicSlotRead Read);

	/** Galaxy_<32 hex digits>, or the same with _bak; anything else (a conflict copy, a temp file) is no slot of a run. */
	COSMIC_ASCENSION_API bool ParseSlotName(const FString& SlotName, FGuid& OutGalaxyID, bool& bOutBackup);

	/** The largest file a slot can be: HeaderBytes + the Kraken bound of MaxRawBytes. */
	COSMIC_ASCENSION_API int64 MaxSlotBytes();

	/** The slot bytes of Save: SaveGameToMemory, then Oodle (Kraken, VeryFast - fixed here, so no cook or command-line
	 * setting changes it), Stored when the codec gives nothing or Want is Stored (tests); the header with both CRCs. Writes no
	 * file. False (OutSlot empty) when a step fails. OutRawBytes (optional): the engine bytes' length (the commandlet). */
	COSMIC_ASCENSION_API bool Encode(UCosmicSaveGame& Save, int64 SavedAtUtcTicks, TArray<uint8>& OutSlot,
		int64* OutRawBytes = nullptr, ECodec Want = ECodec::Oodle);

	/** Checks 1-7 over the first HeaderBytes of Head (it may hold the whole slot) and the file's length FileBytes (Expected:
	 * the galaxy the slot name says; null = any). The continue reads 64 bytes and passes the file's size. */
	COSMIC_ASCENSION_API ECosmicSlotRead ReadHeader(TConstArrayView<uint8> Head, int64 FileBytes, const FGuid* Expected, FHeader& OutHeader);

	/** ReadHeader over the whole Slot, then the payload CRC: what the backup step reads before it copies a slot. */
	COSMIC_ASCENSION_API ECosmicSlotRead Check(TConstArrayView<uint8> Slot, const FGuid* Expected, FHeader& OutHeader);

	/** Check, then the engine bytes (exactly RawBytes) for UGameplayStatics::LoadGameFromMemory - which only ever sees bytes
	 * the length and the CRC passed (16 #70: a cut inside a property reaches a fatal check of the engine). */
	COSMIC_ASCENSION_API ECosmicSlotRead Unpack(TConstArrayView<uint8> Slot, const FGuid* Expected, FHeader& OutHeader, TArray<uint8>& OutRaw);

	/** The run the continue opens: the latest UtcTicks of any of its slots, a tie to the later GUID text; invalid for none. */
	COSMIC_ASCENSION_API FGuid NewestRun(TConstArrayView<FSeen> Seen);
}
