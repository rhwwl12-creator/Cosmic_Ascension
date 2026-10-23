// Cosmic Ascension - automation tests of the slot form, the backup and the continue (MVP item 5, commit A-5a). Design:
// docs/60_DECISIONS/headers/A5_save_minimum.md section 3, 8; docs/20 section 9.1; 16 #33 (2) (3), #70.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Containers/StringConv.h"
#include "Core/GalaxyState.h"
#include "Engine/GameInstance.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Misc/Crc.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "Misc/Timespan.h"
#include "Save/CosmicSaveGame.h"
#include "Save/CosmicSaveSlot.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Templates/UniquePtr.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/Package.h"

namespace CosmicSaveSlotTests
{
	UCosmicSaveSubsystem* SaveOf(UGalaxySimulationSubsystem& Sim)
	{
		return Sim.GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>();
	}

	// The slot bytes of the live state under another GalaxyID (or its own, ID invalid), with the time given.
	TArray<uint8> SlotOf(UGalaxySimulationSubsystem& Sim, const FGuid& ID, int64 SavedAt, CosmicSaveSlot::ECodec Codec = CosmicSaveSlot::ECodec::Oodle, int32 SaveVersion = 0)
	{
		Sim.SyncRngState();
		UCosmicSaveGame* Carrier = NewObject<UCosmicSaveGame>(GetTransientPackage());
		Carrier->State = Sim.GetState();
		if (ID.IsValid())
		{
			Carrier->State.GalaxyID = ID;
		}
		if (SaveVersion > 0)
		{
			Carrier->State.SaveVersion = SaveVersion;
		}
		TArray<uint8> Slot;
		CosmicSaveSlot::Encode(*Carrier, SavedAt, Slot, nullptr, Codec);
		return Slot;
	}

	// Writes the bytes of the header field at At (Value's size), then the header CRC again: a slot that is intact but says
	// something else.
	template <typename T>
	TArray<uint8> Rewritten(TArray<uint8> Bytes, int32 At, T Value)
	{
		FMemory::Memcpy(Bytes.GetData() + At, &Value, sizeof(T));
		const uint32 Crc = FCrc::MemCrc32(Bytes.GetData(), CosmicSaveSlot::HeaderCrcBytes);
		FMemory::Memcpy(Bytes.GetData() + CosmicSaveSlot::HeaderCrcAt, &Crc, sizeof(Crc));
		return Bytes;
	}

	TArray<uint8> Flipped(TArray<uint8> Bytes, int32 At)
	{
		Bytes[At] ^= 0xFF;
		return Bytes;
	}

	TArray<uint8> Cut(TArray<uint8> Bytes, int32 Length)
	{
		Bytes.SetNum(Length);
		return Bytes;
	}

	ECosmicSlotRead CheckOf(TConstArrayView<uint8> Bytes, const FGuid& ID)
	{
		CosmicSaveSlot::FHeader Header;
		return CosmicSaveSlot::Check(Bytes, &ID, Header);
	}

	bool WriteFile(const FString& Path, const TArray<uint8>& Bytes)
	{
		return FFileHelper::SaveArrayToFile(Bytes, *Path);
	}

	TArray<uint8> ReadFile(const FString& Path)
	{
		TArray<uint8> Bytes;
		FFileHelper::LoadFileToArray(Bytes, *Path, FILEREAD_Silent);
		return Bytes;
	}

	int64 HeaderTickOf(const FString& Path)
	{
		const TArray<uint8> Bytes = ReadFile(Path);
		CosmicSaveSlot::FHeader Header;
		CosmicSaveSlot::ReadHeader(Bytes, Bytes.Num(), nullptr, Header);
		return Header.Tick;
	}

	int64 TicksAgo(double Seconds)
	{
		return (FDateTime::UtcNow() - FTimespan::FromSeconds(Seconds)).GetTicks();
	}

	TArray<FString> SlotFilesIn(const FString& Directory)
	{
		TArray<FString> Files;
		IFileManager::Get().FindFiles(Files, *Directory, TEXT(".sav"));
		Files.Sort();
		return Files;
	}

	void EmptyFolder(const FString& Directory)
	{
		for (const FString& File : SlotFilesIn(Directory))
		{
			IFileManager::Get().Delete(*(Directory / File), false, true, true);
		}
	}
}

// The slot form (16 #70): the header's fields, the round trip through the codec and the engine's reader, the Stored path,
// and every refusal with its reason - one case each. No file is written.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveSlotFormTest, "Cosmic_Ascension.Sim.SaveSlotForm", CosmicTests::Flags)
bool FCosmicSimSaveSlotFormTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSaveSlotTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	ON_SCOPE_EXIT { Sim->NewGalaxy(42, 5, 3); };
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(1000);
	Sim->SyncRngState();
	const FGuid ID = Sim->GetState().GalaxyID;
	UCosmicSaveGame* Carrier = NewObject<UCosmicSaveGame>(GetTransientPackage());
	Carrier->State = Sim->GetState();
	TArray<uint8> Engine;
	UGameplayStatics::SaveGameToMemory(Carrier, Engine);
	TArray<uint8> Slot;
	int64 RawBytes = 0;
	if (!TestTrue(TEXT("The state packs into a slot"), CosmicSaveSlot::Encode(*Carrier, 123456789, Slot, &RawBytes)))
	{
		return false;
	}

	// ---- The header ----
	CosmicSaveSlot::FHeader Header;
	TestTrue(TEXT("The slot reads"), CosmicSaveSlot::ReadHeader(Slot, Slot.Num(), &ID, Header) == ECosmicSlotRead::Ok);
	TestTrue(TEXT("Its first word is CASV"), Slot.Num() > 4 && Slot[0] == 'C' && Slot[1] == 'A' && Slot[2] == 'S' && Slot[3] == 'V');
	TestEqual(TEXT("form 1"), static_cast<int32>(Header.Form), 1);
	TestTrue(TEXT("codec Oodle"), Header.Codec == CosmicSaveSlot::ECodec::Oodle);
	TestEqual(TEXT("byte 7 is 0"), static_cast<int32>(Slot[7]), 0);
	TestTrue(TEXT("its galaxy"), Header.GalaxyID == ID);
	TestEqual(TEXT("its time"), Header.SavedAtUtcTicks, static_cast<int64>(123456789));
	TestEqual(TEXT("its tick"), Header.Tick, static_cast<int64>(1000));
	TestEqual(TEXT("RawBytes is the engine bytes' length"), Header.RawBytes, static_cast<int64>(Engine.Num()));
	TestEqual(TEXT("(Encode reports it too)"), RawBytes, static_cast<int64>(Engine.Num()));
	TestEqual(TEXT("The file is the header and the packed bytes"), static_cast<int64>(Slot.Num()), CosmicSaveSlot::HeaderBytes + Header.PackedBytes);
	TestTrue(TEXT("The packed bytes are fewer than the engine's"), Header.PackedBytes < Header.RawBytes);

	// ---- The round trip ----
	{
		const uint32 HashBefore = Sim->ComputeStateHash();
		TArray<uint8> Raw;
		CosmicSaveSlot::FHeader Read;
		TestTrue(TEXT("The slot unpacks"), CosmicSaveSlot::Unpack(Slot, &ID, Read, Raw) == ECosmicSlotRead::Ok);
		TestTrue(TEXT("into the engine bytes exactly"), Raw == Engine);
		const UCosmicSaveGame* Loaded = Cast<UCosmicSaveGame>(UGameplayStatics::LoadGameFromMemory(Raw));
		if (TestNotNull(TEXT("The engine reads them"), Loaded))
		{
			Sim->LoadState(Loaded->State);
			TestEqual(TEXT("and the loaded state has the hash of the state packed"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(HashBefore));
		}
		TArray<uint8> Stored;
		CosmicSaveSlot::Encode(*Carrier, 1, Stored, nullptr, CosmicSaveSlot::ECodec::Stored);
		CosmicSaveSlot::FHeader StoredHeader;
		TestTrue(TEXT("The Stored path unpacks"), CosmicSaveSlot::Unpack(Stored, &ID, StoredHeader, Raw) == ECosmicSlotRead::Ok);
		TestTrue(TEXT("(Stored: codec 0, the two lengths equal, the same engine bytes)"), StoredHeader.Codec == CosmicSaveSlot::ECodec::Stored
			&& StoredHeader.PackedBytes == StoredHeader.RawBytes && Raw == Engine);
	}

	// ---- The refusals, each with its reason ----
	const FGuid Other(0x5A5E00FF, 1, 1, 1);
	const int32 Packed = Slot.Num() - CosmicSaveSlot::HeaderBytes;
	TestTrue(TEXT("Empty: Short"), CheckOf(TArray<uint8>(), ID) == ECosmicSlotRead::Short);
	TestTrue(TEXT("63 bytes: Short"), CheckOf(Cut(Slot, 63), ID) == ECosmicSlotRead::Short);
	TestTrue(TEXT("The header alone: Truncated"), CheckOf(Cut(Slot, 64), ID) == ECosmicSlotRead::Truncated);
	TestTrue(TEXT("One byte short: Truncated"), CheckOf(Cut(Slot, Slot.Num() - 1), ID) == ECosmicSlotRead::Truncated);
	{
		TArray<uint8> Longer = Slot;
		Longer.Add(0);
		TestTrue(TEXT("One byte long: Truncated"), CheckOf(Longer, ID) == ECosmicSlotRead::Truncated);
	}
	TestTrue(TEXT("The engine's own form: OldForm"), CheckOf(Engine, ID) == ECosmicSlotRead::OldForm);
	{
		TArray<uint8> Foreign = Slot;
		Foreign[0] = 'X'; Foreign[1] = 'X'; Foreign[2] = 'X'; Foreign[3] = 'X';
		TestTrue(TEXT("Another first word: NotOurs"), CheckOf(Foreign, ID) == ECosmicSlotRead::NotOurs);
	}
	TestTrue(TEXT("Form 2 with its CRC: NewerForm"), CheckOf(Rewritten<uint16>(Slot, CosmicSaveSlot::FormAt, 2), ID) == ECosmicSlotRead::NewerForm);
	{
		TArray<uint8> Form2 = Slot;
		Form2[CosmicSaveSlot::FormAt] = 2;
		TestTrue(TEXT("Form 2 without its CRC: HeaderDamaged (the CRC before the form)"), CheckOf(Form2, ID) == ECosmicSlotRead::HeaderDamaged);
	}
	TestTrue(TEXT("Form 0: BadSizes"), CheckOf(Rewritten<uint16>(Slot, CosmicSaveSlot::FormAt, 0), ID) == ECosmicSlotRead::BadSizes);
	TestTrue(TEXT("A flipped byte of the header's tick: HeaderDamaged"), CheckOf(Flipped(Slot, 32), ID) == ECosmicSlotRead::HeaderDamaged);
	TestTrue(TEXT("Codec 9: BadSizes"), CheckOf(Rewritten<uint8>(Slot, 6, 9), ID) == ECosmicSlotRead::BadSizes);
	TestTrue(TEXT("RawBytes of 65 MB: BadSizes"), CheckOf(Rewritten<int64>(Slot, 40, 65ll * 1024 * 1024), ID) == ECosmicSlotRead::BadSizes);
	TestTrue(TEXT("Another name: OtherGalaxy"), CheckOf(Slot, Other) == ECosmicSlotRead::OtherGalaxy);
	TestTrue(TEXT("A flipped byte of the payload: PayloadDamaged"), CheckOf(Flipped(Slot, CosmicSaveSlot::HeaderBytes + Packed / 2), ID) == ECosmicSlotRead::PayloadDamaged);
	{
		// 16 #70: a cut inside a property never reaches the engine's reader. The Stored slot cut where the GalaxyID tag starts
		// (its name's length, right after TellRngState's value) is refused by the length.
		TArray<uint8> Stored;
		CosmicSaveSlot::Encode(*Carrier, 1, Stored, nullptr, CosmicSaveSlot::ECodec::Stored);
		const auto Name = StringCast<ANSICHAR>(TEXT("GalaxyID"));
		int32 At = INDEX_NONE;
		for (int32 Index = 0; Index + Name.Length() < Engine.Num(); ++Index)
		{
			if (FMemory::Memcmp(Engine.GetData() + Index, Name.Get(), Name.Length()) == 0)
			{
				At = Index;
				break;
			}
		}
		if (TestTrue(TEXT("(the engine bytes hold the GalaxyID tag)"), At >= 4))
		{
			TestTrue(TEXT("A Stored slot cut inside the properties: Truncated"),
				CheckOf(Cut(Stored, CosmicSaveSlot::HeaderBytes + At - 4), ID) == ECosmicSlotRead::Truncated);
		}
	}

	// ---- Slot names ----
	const FString Digits = ID.ToString(EGuidFormats::Digits);
	FGuid Parsed;
	bool bBackup = true;
	TestTrue(TEXT("Galaxy_ + 32 capitals is a slot"), CosmicSaveSlot::ParseSlotName(TEXT("Galaxy_") + Digits, Parsed, bBackup) && Parsed == ID && !bBackup);
	TestTrue(TEXT("lower case too"), CosmicSaveSlot::ParseSlotName(TEXT("Galaxy_") + Digits.ToLower(), Parsed, bBackup) && Parsed == ID);
	TestTrue(TEXT("and its backup"), CosmicSaveSlot::ParseSlotName(TEXT("Galaxy_") + Digits + TEXT("_bak"), Parsed, bBackup) && Parsed == ID && bBackup);
	TestFalse(TEXT("31 digits are not"), CosmicSaveSlot::ParseSlotName(TEXT("Galaxy_") + Digits.Left(31), Parsed, bBackup));
	TestFalse(TEXT("nor a temporary file"), CosmicSaveSlot::ParseSlotName(TEXT("Galaxy_") + Digits + TEXT("_tmp"), Parsed, bBackup));
	TestFalse(TEXT("nor a copy"), CosmicSaveSlot::ParseSlotName(TEXT("Galaxy_") + Digits + TEXT(" (1)"), Parsed, bBackup));
	TestFalse(TEXT("nor a conflict copy"), CosmicSaveSlot::ParseSlotName(TEXT("Galaxy_") + Digits + TEXT("-DESKTOP"), Parsed, bBackup));
	return true;
}

// The backup (16 #33 (2)): the first save has none, the second copies the verified slot; a damaged slot loads from the
// backup and is not copied over it; a later version, a later form and a slot that did not read are refused without the
// backup and never replaced; a backup that cannot be written keeps the slot.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveBackupTest, "Cosmic_Ascension.Sim.SaveBackup", CosmicTests::Flags)
bool FCosmicSimSaveBackupTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSaveSlotTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicSaveSubsystem* Save = SaveOf(*Sim);
	AddExpectedMessage(TEXT("read from its backup"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 2);
	AddExpectedMessage(TEXT("is damaged \\(Truncated\\): not copied, the backup stays"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1);
	AddExpectedMessage(TEXT("; its backup: "), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1);
	AddExpectedMessage(TEXT("has save version"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1);
	AddExpectedMessage(TEXT("has slot form 2"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1);
	AddExpectedMessage(TEXT("is NewerForm: it is not replaced"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1);
	AddExpectedMessage(TEXT("could not be read: the backup is not tried"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1);
	AddExpectedMessage(TEXT("is ReadFailed: it is not replaced"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1);
	AddExpectedMessage(TEXT("was not written: .* is kept as it was"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1);

	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	const FGuid ID = Sim->GetState().GalaxyID;
	const FString MainPath = Save->SlotPathOf(ID);
	const FString BackupPath = Save->SlotPathOf(ID, true);
	ON_SCOPE_EXIT
	{
		FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*BackupPath, false);
		FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*MainPath, false);
		Save->DeleteSave(ID);
		Sim->NewGalaxy(42, 5, 3);
	};
	Save->DeleteSave(ID);
	bool bFromBackup = false;

	// The first save has no backup; the second copies the first.
	Sim->RunHeadless(100);
	TestTrue(TEXT("The first save succeeds"), Save->SaveCurrentGalaxy());
	const uint32 HashAt100 = Sim->ComputeStateHash();
	TestTrue(TEXT("and leaves no backup"), IFileManager::Get().FileSize(*BackupPath) < 0);
	Sim->RunHeadless(100);
	TestTrue(TEXT("The second save succeeds"), Save->SaveCurrentGalaxy());
	TestEqual(TEXT("Its backup is the first save (tick 100)"), HeaderTickOf(BackupPath), static_cast<int64>(100));
	TestEqual(TEXT("and the slot the second (tick 200)"), HeaderTickOf(MainPath), static_cast<int64>(200));

	// A damaged slot loads from its backup.
	WriteFile(MainPath, Cut(ReadFile(MainPath), ReadFile(MainPath).Num() / 2));
	TestTrue(TEXT("A slot cut in half loads from its backup"), Save->LoadRun(ID, bFromBackup) == ECosmicSlotRead::Ok && bFromBackup);
	TestEqual(TEXT("at tick 100"), Sim->GetState().Tick, static_cast<int64>(100));
	TestEqual(TEXT("with the hash of that save"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(HashAt100));

	// A damaged slot is not copied over the backup.
	Sim->RunHeadless(50);
	TestTrue(TEXT("A save over a damaged slot succeeds"), Save->SaveCurrentGalaxy());
	TestEqual(TEXT("and leaves the backup as it was (tick 100)"), HeaderTickOf(BackupPath), static_cast<int64>(100));

	// Another galaxy's intact bytes under this name: damage, so the backup.
	WriteFile(MainPath, SlotOf(*Sim, FGuid(0x5A5E00FE, 1, 1, 1), TicksAgo(1)));
	TestTrue(TEXT("A slot of another galaxy loads from the backup"), Save->LoadRun(ID, bFromBackup) == ECosmicSlotRead::Ok && bFromBackup);

	// Both damaged: refused, nothing changes.
	{
		WriteFile(MainPath, Cut(ReadFile(MainPath), 100));
		WriteFile(BackupPath, Cut(ReadFile(BackupPath), 100));
		const uint32 Before = Sim->ComputeStateHash();
		TestTrue(TEXT("A slot and a backup both damaged are refused"), Save->LoadRun(ID, bFromBackup) == ECosmicSlotRead::Truncated && !bFromBackup);
		TestEqual(TEXT("and the galaxy is untouched"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Before));
	}

	// A later version and a later form are refused without the backup; a later form is never replaced.
	WriteFile(BackupPath, SlotOf(*Sim, FGuid(), TicksAgo(2)));
	WriteFile(MainPath, SlotOf(*Sim, FGuid(), TicksAgo(1), CosmicSaveSlot::ECodec::Oodle, FGalaxyState::CurrentSaveVersion + 1));
	TestTrue(TEXT("A slot of a later version is refused"), Save->LoadRun(ID, bFromBackup) == ECosmicSlotRead::NewerSave && !bFromBackup);
	{
		const TArray<uint8> Later = Rewritten<uint16>(SlotOf(*Sim, FGuid(), TicksAgo(1)), CosmicSaveSlot::FormAt, 2);
		WriteFile(MainPath, Later);
		TestTrue(TEXT("A slot of a later form is refused"), Save->LoadRun(ID, bFromBackup) == ECosmicSlotRead::NewerForm && !bFromBackup);
		TestFalse(TEXT("A save does not replace a slot of a later form"), Save->SaveCurrentGalaxy());
		TestTrue(TEXT("(the slot is as it was)"), ReadFile(MainPath) == Later);
	}

	// A slot that does not read (a lock) is neither replaced by the backup nor overwritten.
	{
		const TArray<uint8> Intact = SlotOf(*Sim, FGuid(), TicksAgo(1));
		WriteFile(MainPath, Intact);
		IFileHandle* Lock = FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*MainPath, /*bAppend*/ true, /*bAllowRead*/ false);
		{
			ON_SCOPE_EXIT { delete Lock; };
			TestNotNull(TEXT("(the slot is locked)"), Lock);
			TestTrue(TEXT("A slot that does not read is refused without the backup"), Save->LoadRun(ID, bFromBackup) == ECosmicSlotRead::ReadFailed && !bFromBackup);
			TestFalse(TEXT("and a save does not overwrite it"), Save->SaveCurrentGalaxy());
		}
		TestTrue(TEXT("(the slot is as it was)"), ReadFile(MainPath) == Intact);
	}

	// A backup that cannot be written keeps the slot.
	{
		const TArray<uint8> Before = ReadFile(MainPath);
		FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*BackupPath, true);
		TestFalse(TEXT("A save whose backup cannot be written fails"), Save->SaveCurrentGalaxy());
		FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*BackupPath, false);
		TestTrue(TEXT("and keeps the slot as it was"), ReadFile(MainPath) == Before);
	}

	TestTrue(TEXT("DeleteSave deletes the run's files"), Save->DeleteSave(ID));
	TestFalse(TEXT("both of them"), Save->DoesSaveExist(ID));
	return true;
}

// The continue (16 #33 (3)): the newest run by its header's time, a run's time the later of its slot and backup, a header
// that does not read ordered by the file's time; the newest run opens from its slot or its backup, or not at all - never an
// older run; only slot names count. The test's own folder; the game's folder is not touched.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveFindTest, "Cosmic_Ascension.Sim.SaveFind", CosmicTests::Flags)
bool FCosmicSimSaveFindTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSaveSlotTests;
	using CosmicSaveSlot::FSeen;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicSaveSubsystem* Save = SaveOf(*Sim);
	AddExpectedMessage(TEXT("read from its backup"), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 1);
	AddExpectedMessage(TEXT("; its backup: "), ELogVerbosity::Warning, EAutomationExpectedMessageFlags::Contains, 2);

	const FGuid A(0x5A5E0001, 1, 1, 1);
	const FGuid B(0x5A5E0002, 1, 1, 1);
	const FGuid C(0x5A5E0003, 1, 1, 1);
	const FGuid D(0x5A5E0004, 1, 1, 1);

	// ---- NewestRun, pure ----
	{
		const auto Seen = [](const FGuid& ID, bool bBackup, int64 At) { FSeen Each; Each.GalaxyID = ID; Each.bBackup = bBackup; Each.UtcTicks = At; return Each; };
		TestTrue(TEXT("The later time comes first"), CosmicSaveSlot::NewestRun({ Seen(A, false, 5), Seen(B, false, 8) }) == B);
		TestTrue(TEXT("A run's time is the later of its slot and backup"), CosmicSaveSlot::NewestRun({ Seen(A, false, 5), Seen(A, true, 9), Seen(B, false, 8) }) == A);
		TestTrue(TEXT("A file's time stands in for a header that does not read"), CosmicSaveSlot::NewestRun({ Seen(B, false, 99), Seen(A, false, 5) }) == B);
		TestTrue(TEXT("The same time goes to the later GUID text"), CosmicSaveSlot::NewestRun({ Seen(B, false, 7), Seen(A, false, 7) }) == B);
		TestFalse(TEXT("No slot, no run"), CosmicSaveSlot::NewestRun({}).IsValid());
	}

	// ---- On the disk ----
	const FString GameFolder = FPaths::ProjectSavedDir() / TEXT("SaveGames");
	const TArray<FString> GameBefore = SlotFilesIn(GameFolder);
	Save->SetFolderForTest(TEXT("Automation/Find/"));
	const FString Folder = GameFolder / TEXT("Automation/Find");
	EmptyFolder(Folder);
	ON_SCOPE_EXIT
	{
		EmptyFolder(Folder);
		Save->PrepareForTests();
		Sim->NewGalaxy(42, 5, 3);
	};
	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	WriteFile(Save->SlotPathOf(A), SlotOf(*Sim, A, TicksAgo(30)));
	WriteFile(Save->SlotPathOf(B), SlotOf(*Sim, B, TicksAgo(20)));
	const TArray<uint8> CIntact = SlotOf(*Sim, C, TicksAgo(10));
	WriteFile(Save->SlotPathOf(C), CIntact);
	TestTrue(TEXT("The newest run opens from its slot"), Save->ContinueNewest() == ECosmicContinue::Newest && Sim->GetState().GalaxyID == C);

	// C's slot cut in its payload, its backup a little older: the run is still the newest, opened from the backup.
	WriteFile(Save->SlotPathOf(C, true), SlotOf(*Sim, C, TicksAgo(15)));
	WriteFile(Save->SlotPathOf(C), Cut(CIntact, CIntact.Num() - 10));
	TestTrue(TEXT("A newest run whose slot is cut opens from its backup"), Save->ContinueNewest() == ECosmicContinue::Backup && Sim->GetState().GalaxyID == C);

	// Both of C's payloads damaged: Unreadable, never B; nothing changes, the files stay.
	{
		Sim->NewGalaxy(7, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
		const uint32 Before = Sim->ComputeStateHash();
		const TArray<uint8> Backup = ReadFile(Save->SlotPathOf(C, true));
		WriteFile(Save->SlotPathOf(C), Flipped(CIntact, CIntact.Num() - 5));
		WriteFile(Save->SlotPathOf(C, true), Flipped(Backup, Backup.Num() - 5));
		TestTrue(TEXT("A newest run that does not open is Unreadable"), Save->ContinueNewest() == ECosmicContinue::Unreadable);
		TestEqual(TEXT("and leaves the galaxy as it was (no older run)"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Before));
		TestTrue(TEXT("and its files"), Save->DoesSaveExist(C) && IFileManager::Get().FileSize(*Save->SlotPathOf(C, true)) > 0);

		// Both of C's files empty: their file time is now, so C is still the newest - and still not opened.
		WriteFile(Save->SlotPathOf(C), TArray<uint8>());
		WriteFile(Save->SlotPathOf(C, true), TArray<uint8>());
		TestTrue(TEXT("A newest run of empty files is Unreadable"), Save->ContinueNewest() == ECosmicContinue::Unreadable);
		TestEqual(TEXT("and B is not opened in its place"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Before));
	}

	// Only slot names count: a later header under other names is not a run.
	WriteFile(Save->SlotPathOf(C), CIntact);
	IFileManager::Get().Delete(*Save->SlotPathOf(C, true), false, true, true);
	const TArray<uint8> Later = SlotOf(*Sim, D, TicksAgo(-9));
	const FString DName = UCosmicSaveSubsystem::MakeSlotName(D);
	WriteFile(Folder / (DName + TEXT("_tmp.sav")), Later);
	WriteFile(Folder / (DName + TEXT(" (1).sav")), Later);
	WriteFile(Folder / TEXT("Galaxy_123.sav"), Later);
	WriteFile(Folder / TEXT("Other.sav"), Later);
	TestTrue(TEXT("Files that are not slots of a run are passed over"), Save->ContinueNewest() == ECosmicContinue::Newest && Sim->GetState().GalaxyID == C);

	TestTrue(TEXT("The game's folder is as it was"), SlotFilesIn(GameFolder) == GameBefore);
	return true;
}

// Saving changes nothing of the run: a run saved every 100 ticks ends as one never saved - both hashes and both streams.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimSaveNeutralTest, "Cosmic_Ascension.Sim.SaveNeutral", CosmicTests::Flags)
bool FCosmicSimSaveNeutralTest::RunTest(const FString& Parameters)
{
	using namespace CosmicSaveSlotTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	UCosmicSaveSubsystem* Save = SaveOf(*Sim);
	ON_SCOPE_EXIT { Sim->NewGalaxy(42, 5, 3); };

	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	const FGuid Saved = Sim->GetState().GalaxyID;
	ON_SCOPE_EXIT { Save->DeleteSave(Saved); };
	for (int32 Step = 0; Step < 20; ++Step)
	{
		Sim->RunHeadless(100);
		const uint32 Before = Sim->ComputeStateHash();
		TestTrue(TEXT("(the save succeeds)"), Save->SaveCurrentGalaxy());
		if (Step == 0)
		{
			TestEqual(TEXT("A save leaves the hash as it was"), static_cast<int64>(Sim->ComputeStateHash()), static_cast<int64>(Before));
		}
	}
	const uint32 SavedHash = Sim->ComputeStateHash();
	const uint32 SavedWorld = Sim->ComputeWorldHash();
	const int32 SavedSeed = Sim->GetRng().GetCurrentSeed();
	const int32 SavedTell = Sim->GetState().TellRngState;

	Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs);
	Sim->RunHeadless(2000);
	TestEqual(TEXT("A run saved every 100 ticks has the full hash of one never saved"), static_cast<int64>(SavedHash), static_cast<int64>(Sim->ComputeStateHash()));
	TestEqual(TEXT("and its world hash"), static_cast<int64>(SavedWorld), static_cast<int64>(Sim->ComputeWorldHash()));
	TestEqual(TEXT("and its sim stream"), SavedSeed, Sim->GetRng().GetCurrentSeed());
	TestEqual(TEXT("and its tell stream"), SavedTell, Sim->GetState().TellRngState);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
