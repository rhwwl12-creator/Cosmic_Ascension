// Cosmic Ascension - Save layer.

#include "Save/CosmicSaveSubsystem.h"

#include "Cosmic_Ascension.h"
#include "Core/CosmicIDs.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/GameInstance.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Save/CosmicSaveGame.h"
#include "Sim/CosmicEndings.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Templates/UniquePtr.h"
#include "UObject/Package.h"

bool CosmicSave::KeepsSanctuary(const FGalaxyState& State)
{
	// Every galaxy ends with the sanctuary, kept by Law.Hedge (section 4.5 (1)): no owner, hazard or project.
	const FStarSystemState* Last = State.Systems.IsEmpty() ? nullptr : &State.Systems.Last();
	return Last && CosmicState::IsSanctuary(Last->SystemID) && Last->OwnerCivID.IsNone() && !Last->ActiveHazard.IsValid()
		&& Last->ActiveMegaproject.IsNone() && Last->ResponseMegaproject.IsNone();
}

const TCHAR* CosmicSave::NameOf(ECosmicSaveCause Cause)
{
	switch (Cause)
	{
	case ECosmicSaveCause::Asked: return TEXT("Asked");
	case ECosmicSaveCause::NewRun: return TEXT("NewRun");
	case ECosmicSaveCause::Ascended: return TEXT("Ascended");
	case ECosmicSaveCause::Role: return TEXT("Role");
	case ECosmicSaveCause::Ended: return TEXT("Ended");
	case ECosmicSaveCause::Interval: return TEXT("Interval");
	case ECosmicSaveCause::Answer: return TEXT("Answer");
	case ECosmicSaveCause::Exit: return TEXT("Exit");
	}
	return TEXT("?");
}

const TCHAR* CosmicSave::NameOf(ECosmicContinue Outcome)
{
	switch (Outcome)
	{
	case ECosmicContinue::NotBegun: return TEXT("NotBegun");
	case ECosmicContinue::Off: return TEXT("Off");
	case ECosmicContinue::NoSlot: return TEXT("NoSlot");
	case ECosmicContinue::Newest: return TEXT("Newest");
	case ECosmicContinue::Backup: return TEXT("Backup");
	case ECosmicContinue::Unreadable: return TEXT("Unreadable");
	}
	return TEXT("?");
}

void UCosmicSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (UGalaxySimulationSubsystem* Sim = Collection.InitializeDependency<UGalaxySimulationSubsystem>())
	{
		Sim->OnSimTick.AddUniqueDynamic(this, &UCosmicSaveSubsystem::HandleSimTick);
	}
	bCommandLineAllows = CommandLineAllows(FCommandLine::Get());
	UE_LOG(LogCosmic, Log, TEXT("CosmicSave: the game's saving is %s by the command line"), bCommandLineAllows ? TEXT("allowed") : TEXT("off"));
}

void UCosmicSaveSubsystem::Deinitialize()
{
	// The simulation may be down already (the order of Deinitialize is not fixed).
	if (UGalaxySimulationSubsystem* Sim = GetSim())
	{
		Sim->OnSimTick.RemoveDynamic(this, &UCosmicSaveSubsystem::HandleSimTick);
	}
	Super::Deinitialize();
}

bool UCosmicSaveSubsystem::CommandLineAllows(const TCHAR* CommandLine)
{
	int32 GivenSeed = 0;
	return !FParse::Param(CommandLine, TEXT("NoSave")) && !FParse::Param(CommandLine, TEXT("unattended"))
		&& !FParse::Value(CommandLine, TEXT("Seed="), GivenSeed);
}

const TCHAR* UCosmicSaveSubsystem::GateClosedBy(bool bAllows, bool bBlocked, bool bAutomation, bool bEditor, bool bOverride)
{
	if (bOverride)
	{
		return nullptr;
	}
	if (!bAllows)
	{
		return TEXT("command line");
	}
	if (bBlocked)
	{
		return TEXT("tests");
	}
	if (bAutomation)
	{
		return TEXT("automation");
	}
	if (bEditor)
	{
		return TEXT("editor");
	}
	return nullptr;
}

bool UCosmicSaveSubsystem::IsGameSavingOn() const
{
	// GIsEditor (CoreGlobals.h): true in the editor process, so play in editor; false under -game and Standalone Game.
	return GateClosedBy(bCommandLineAllows, bBlockedForTests, GIsAutomationTesting, GIsEditor, bTestOverride) == nullptr;
}

FCosmicSavedMark UCosmicSaveSubsystem::MarkOf(const FGalaxyState& State, const FCosmicRulesDef& Rules)
{
	FCosmicSavedMark Mark;
	Mark.GalaxyID = State.GalaxyID;
	Mark.Tick = State.Tick;
	Mark.Feats = State.Calling.CompletedFeats.Num();
	Mark.Role = State.Player.Role;
	Mark.RuledCivID = State.Player.RuledCivID;
	Mark.bEnded = CosmicEndings::ProvisionalEndOf(State, Rules) != ECosmicProvisionalEnd::None;
	return Mark;
}

TOptional<ECosmicSaveCause> UCosmicSaveSubsystem::DueAt(const FCosmicSavedMark& Saved, const FCosmicSavedMark& Now, double NowRealSeconds, double InIntervalSeconds)
{
	if (Now.GalaxyID != Saved.GalaxyID)
	{
		return ECosmicSaveCause::NewRun;
	}
	if (Now.bEnded && !Saved.bEnded)
	{
		return ECosmicSaveCause::Ended;
	}
	if (Now.Feats != Saved.Feats)
	{
		return ECosmicSaveCause::Ascended;
	}
	if (Now.Role != Saved.Role || Now.RuledCivID != Saved.RuledCivID)
	{
		return ECosmicSaveCause::Role;
	}
	if (Now.Tick != Saved.Tick && NowRealSeconds - Saved.RealSeconds >= InIntervalSeconds)
	{
		return ECosmicSaveCause::Interval;
	}
	return {};
}

void UCosmicSaveSubsystem::SetMarkFromState(uint32 StateHash)
{
	const UGalaxySimulationSubsystem* Sim = GetSim();
	if (!Sim)
	{
		return;
	}
	Mark = MarkOf(Sim->GetState(), Sim->GetRules());
	Mark.StateHash = StateHash;
	Mark.RealSeconds = FPlatformTime::Seconds();
}

void UCosmicSaveSubsystem::HandleSimTick(int64 Tick)
{
	// OnSimTick: after the tenth stage and SyncRngState, outside the brackets of the Core Logic broadcasts (rule 5) - the
	// tick boundary of 16 #33 (1). At most one write per boundary; no boundary while the time stands still.
	if (!IsGameSavingOn())
	{
		Pending.Reset();
		return;
	}
	const UGalaxySimulationSubsystem* Sim = GetSim();
	if (!Sim || !Sim->GetState().GalaxyID.IsValid())
	{
		return;
	}
	const double Now = FPlatformTime::Seconds();
	const TOptional<ECosmicSaveCause> Due = DueAt(Mark, MarkOf(Sim->GetState(), Sim->GetRules()), Now, IntervalSeconds);
	// After a failure the kept cause and the due trigger wait - all but the end of the run: the controller's latch stops the
	// time in this very tick, so no later boundary would come for it (once: a failed end write is kept like any other).
	const bool bEnd = Due == ECosmicSaveCause::Ended;
	if (Now < RetryNotBefore && !bEnd)
	{
		return;
	}
	const TOptional<ECosmicSaveCause> Cause = bEnd ? Due : (Pending ? Pending : Due);
	if (!Cause)
	{
		return;
	}
	Pending.Reset();
	Write(*Cause, true);   // a failure keeps the cause again
}

void UCosmicSaveSubsystem::RequestSave(ECosmicSaveCause Cause)
{
	UGalaxySimulationSubsystem* Sim = GetSim();
	if (!IsGameSavingOn() || !Sim || !Sim->GetState().GalaxyID.IsValid())
	{
		return;
	}
	// A handler or a stage may only leave a mark: the next tick boundary writes it.
	if (Sim->IsBroadcasting() || Sim->IsInTickStages())
	{
		if (!Pending)
		{
			Pending = Cause;
		}
		return;
	}
	const FGalaxyState& State = Sim->GetState();
	if (Cause == ECosmicSaveCause::Exit)
	{
		if (Mark.GalaxyID == State.GalaxyID && Mark.StateHash == Sim->ComputeStateHash())
		{
			UE_LOG(LogCosmic, Verbose, TEXT("CosmicSave: exit - the slot already holds this state"));
			return;
		}
		// Decision 6: the new run of an Unreadable start (HoldStartRun) is first written at a tick boundary, so a quit before
		// that leaves the run that did not read as the newest - the next start reads it again. That run only: a later new
		// run of the session is written.
		if (HeldRunID.IsValid() && State.GalaxyID == HeldRunID && Mark.GalaxyID != HeldRunID && State.Tick == 0)
		{
			UE_LOG(LogCosmic, Log, TEXT("CosmicSave: exit - the new run after an unreadable save is not written yet"));
			return;
		}
	}
	Write(Cause, true);
}

bool UCosmicSaveSubsystem::TakeFirstFailure()
{
	if (bFailureOpen && !bFailureTaken)
	{
		bFailureTaken = true;
		return true;
	}
	return false;
}

ECosmicContinue UCosmicSaveSubsystem::BeginSession()
{
	HeldRunID.Invalidate();
	const TCHAR* ClosedBy = GateClosedBy(bCommandLineAllows, bBlockedForTests, GIsAutomationTesting, GIsEditor, bTestOverride);
	if (ClosedBy)
	{
		StartOutcome = LastContinue = ECosmicContinue::Off;
		UE_LOG(LogCosmic, Log, TEXT("CosmicContinue: Off (%s)"), ClosedBy);
		return StartOutcome;
	}
	StartOutcome = ContinueNewest();
	return StartOutcome;
}

ECosmicContinue UCosmicSaveSubsystem::TakeStartOutcome()
{
	const ECosmicContinue Outcome = StartOutcome;
	StartOutcome = ECosmicContinue::NotBegun;
	return Outcome;
}

void UCosmicSaveSubsystem::HoldStartRun()
{
	const UGalaxySimulationSubsystem* Sim = GetSim();
	HeldRunID = Sim ? Sim->GetState().GalaxyID : FGuid();
}

UGalaxySimulationSubsystem* UCosmicSaveSubsystem::GetSim() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
}

FString UCosmicSaveSubsystem::MakeSlotName(const FGuid& GalaxyID)
{
	return FString::Printf(TEXT("Galaxy_%s"), *GalaxyID.ToString(EGuidFormats::Digits));
}

FString UCosmicSaveSubsystem::SlotNameOf(const FGuid& GalaxyID, bool bBackup) const
{
	return Folder + MakeSlotName(GalaxyID) + (bBackup ? TEXT("_bak") : TEXT(""));
}

FString UCosmicSaveSubsystem::SlotPathOf(const FGuid& GalaxyID, bool bBackup) const
{
	// FGenericSaveGameSystem::GetSaveGamePath (SaveGameSystem.h): the same folder and extension the engine's slots use.
	return FPaths::ProjectSavedDir() / TEXT("SaveGames") / (SlotNameOf(GalaxyID, bBackup) + TEXT(".sav"));
}

bool UCosmicSaveSubsystem::SaveCurrentGalaxy()
{
	return Write(ECosmicSaveCause::Asked, false);
}

bool UCosmicSaveSubsystem::Write(ECosmicSaveCause Cause, bool bGamePath)
{
	UGalaxySimulationSubsystem* Sim = GetSim();
	if (!Sim)
	{
		return false;
	}
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("SaveCurrentGalaxy called from a handler of a Core Logic delegate: a save waits for the tick boundary"));
		return false;
	}
	if (Sim->IsInTickStages())
	{
		UE_LOG(LogCosmic, Error, TEXT("SaveCurrentGalaxy called inside the tick stages: save from OnSimTick or from input"));
		return false;
	}
	const FGuid ID = Sim->GetState().GalaxyID;
	if (!ID.IsValid())
	{
		UE_LOG(LogCosmic, Warning, TEXT("SaveCurrentGalaxy: no galaxy to save"));
		return false;
	}

	// A failure of the game's path is noted once (the controller stops the time), waits RetrySeconds and keeps its cause for
	// the first boundary after the wait (decision 12). An explicit call's failure is only its return value.
	auto Fail = [this, Cause, bGamePath]()
	{
		if (bGamePath)
		{
			bFailureOpen = true;
			RetryNotBefore = FPlatformTime::Seconds() + RetrySeconds;
			if (!Pending)
			{
				Pending = Cause;
			}
		}
		return false;
	};
	// A state the load would refuse (a broken sanctuary - a defect of the simulation) is not written: the slot and the
	// backup keep the last state that loads, instead of the next write rotating it out (TryRead refuses Sanctuary without
	// the backup).
	if (!CosmicSave::KeepsSanctuary(Sim->GetState()))
	{
		UE_LOG(LogCosmic, Error, TEXT("CosmicSave: %s does not end with a kept sanctuary: not written, the last slot that loads stays"), *SlotNameOf(ID));
		return Fail();
	}
	const double Start = FPlatformTime::Seconds();
	Sim->SyncRngState();
	UCosmicSaveGame* Carrier = NewObject<UCosmicSaveGame>(GetTransientPackage());
	Carrier->State = Sim->GetState();
	// Later than every write and load of this process, even when the clock steps back.
	const int64 SavedAt = FMath::Max(FDateTime::UtcNow().GetTicks(), LatestSavedAt + 1);
	TArray<uint8> Slot;
	int64 RawBytes = 0;
	const FString Main = SlotNameOf(ID);
	if (!CosmicSaveSlot::Encode(*Carrier, SavedAt, Slot, &RawBytes))
	{
		UE_LOG(LogCosmic, Warning, TEXT("CosmicSave: %s could not be packed"), *Main);
		return Fail();
	}

	// The backup: only a slot that passed its checks is copied, and a failed copy keeps the slot as it was - so one good copy
	// is on the disk at every moment. A slot that did not read, or one of a later form, is never replaced.
	const FString MainPath = SlotPathOf(ID);
	if (IFileManager::Get().FileSize(*MainPath) >= 0)
	{
		TArray<uint8> Old;
		CosmicSaveSlot::FHeader OldHeader;
		ECosmicSlotRead OldRead = ReadSlotFile(MainPath, Old);
		if (OldRead == ECosmicSlotRead::Ok)
		{
			OldRead = CosmicSaveSlot::Check(Old, &ID, OldHeader);
		}
		if (OldRead == ECosmicSlotRead::Ok)
		{
			if (!WriteSlotFile(SlotPathOf(ID, true), Old))
			{
				UE_LOG(LogCosmic, Warning, TEXT("CosmicSave: the backup of %s was not written: %s is kept as it was"), *Main, *Main);
				return Fail();
			}
		}
		else if (OldRead == ECosmicSlotRead::ReadFailed || OldRead == ECosmicSlotRead::NewerForm)
		{
			UE_LOG(LogCosmic, Warning, TEXT("CosmicSave: %s is %s: it is not replaced"), *Main, CosmicSaveSlot::NameOf(OldRead));
			return Fail();
		}
		else
		{
			UE_LOG(LogCosmic, Warning, TEXT("CosmicSave: %s is damaged (%s): not copied, the backup stays"), *Main, CosmicSaveSlot::NameOf(OldRead));
		}
	}
	LatestSavedAt = SavedAt;
	if (!WriteSlotFile(MainPath, Slot))
	{
		UE_LOG(LogCosmic, Warning, TEXT("CosmicSave: %s was not written"), *Main);
		return Fail();
	}
	++SavesWritten;
	LastCause = Cause;
	// On any path: this write put the state on the disk, so nothing kept is owed and the boundary does not wait.
	Pending.Reset();
	RetryNotBefore = 0.0;
	bFailureOpen = false;
	const uint32 Hash = Sim->ComputeStateHash();
	SetMarkFromState(Hash);
	UE_LOG(LogCosmic, Display, TEXT("CosmicSave: wrote %s (%s) tick %lld, hash %u, %d of %lld bytes in %.1f ms"), *Main, CosmicSave::NameOf(Cause),
		Carrier->State.Tick, Hash, Slot.Num(), RawBytes, (FPlatformTime::Seconds() - Start) * 1000.0);
	return true;
}

bool UCosmicSaveSubsystem::WriteSlotFile(const FString& Path, TConstArrayView<uint8> Bytes) const
{
	IPlatformFile& File = FPlatformFileManager::Get().GetPlatformFile();
	File.CreateDirectoryTree(*FPaths::GetPath(Path));
	// Fails on a read-only file. OpenWrite truncates in place (no temporary file): the backup order above is the safety.
	TUniquePtr<IFileHandle> Handle(File.OpenWrite(*Path));
	if (!Handle)
	{
		return false;
	}
	// Flush(true): FlushFileBuffers on Win64 - the bytes are on the disk before the next write starts.
	const bool bOk = Handle->Write(Bytes.GetData(), Bytes.Num()) && Handle->Flush(true);
	Handle.Reset();
	return bOk;
}

ECosmicSlotRead UCosmicSaveSubsystem::ReadSlotFile(const FString& Path, TArray<uint8>& Out) const
{
	Out.Reset();
	const int64 Size = IFileManager::Get().FileSize(*Path);
	if (Size < 0)
	{
		return ECosmicSlotRead::Missing;
	}
	if (Size > CosmicSaveSlot::MaxSlotBytes())
	{
		return ECosmicSlotRead::BadSizes;
	}
	// A lock or an I/O error is not damage: tried twice, a quarter second apart (a sync client or a scanner holds a file
	// for a moment), then ReadFailed (decision 5). Only a failed first try waits.
	for (int32 Attempt = 0; Attempt < 2; ++Attempt)
	{
		if (Attempt > 0)
		{
			FPlatformProcess::Sleep(0.25f);
		}
		if (FFileHelper::LoadFileToArray(Out, *Path, FILEREAD_Silent))
		{
			return ECosmicSlotRead::Ok;
		}
	}
	Out.Reset();
	return ECosmicSlotRead::ReadFailed;
}

ECosmicSlotRead UCosmicSaveSubsystem::TryRead(const FGuid& GalaxyID, bool bBackup, UCosmicSaveGame*& Out, CosmicSaveSlot::FHeader& OutHeader) const
{
	Out = nullptr;
	TArray<uint8> Bytes;
	const ECosmicSlotRead FileRead = ReadSlotFile(SlotPathOf(GalaxyID, bBackup), Bytes);
	if (FileRead != ECosmicSlotRead::Ok)
	{
		return FileRead;
	}
	TArray<uint8> Raw;
	const ECosmicSlotRead Read = CosmicSaveSlot::Unpack(Bytes, &GalaxyID, OutHeader, Raw);
	if (Read != ECosmicSlotRead::Ok)
	{
		return Read;
	}
	// Only bytes the length and the CRC passed reach the engine's reader (16 #70).
	UCosmicSaveGame* Loaded = Cast<UCosmicSaveGame>(UGameplayStatics::LoadGameFromMemory(Raw));
	if (!Loaded)
	{
		return ECosmicSlotRead::Unreadable;
	}
	if (Loaded->State.GalaxyID != GalaxyID)
	{
		return ECosmicSlotRead::OtherGalaxy;
	}
	const FString SlotName = SlotNameOf(GalaxyID, bBackup);
	if (Loaded->State.SaveVersion > FGalaxyState::CurrentSaveVersion)
	{
		UE_CLOG(!bBackup, LogCosmic, Warning, TEXT("LoadGalaxy: slot %s has save version %d, this build reads up to %d"),
			*SlotName, Loaded->State.SaveVersion, FGalaxyState::CurrentSaveVersion);
		return ECosmicSlotRead::NewerSave;
	}
	// Migration functions for SaveVersion < CurrentSaveVersion go here once a version bump exists (section 9.1).

	// Every galaxy ends with the sanctuary, kept by Law.Hedge (section 4.5 (1)): no owner, hazard or project. A save from
	// before v0.8 step 4 (sol was the first life's home then) or one whose sanctuary is not kept is refused - development
	// saves are not carried over (no SaveVersion bump: no field changed).
	if (!CosmicSave::KeepsSanctuary(Loaded->State))
	{
		UE_CLOG(!bBackup, LogCosmic, Error, TEXT("LoadGalaxy: slot %s does not end with a kept sanctuary (a save from before v0.8 step 4, or a broken one): start a new galaxy"), *SlotName);
		return ECosmicSlotRead::Sanctuary;
	}
	Out = Loaded;
	return ECosmicSlotRead::Ok;
}

ECosmicSlotRead UCosmicSaveSubsystem::LoadRun(const FGuid& GalaxyID, bool& bOutFromBackup)
{
	bOutFromBackup = false;
	UGalaxySimulationSubsystem* Sim = GetSim();
	if (!Sim)
	{
		return ECosmicSlotRead::Refused;
	}
	// Before the slot is read: a load asked for from a handler or a stage is refused, not reported as done.
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("LoadGalaxy called from a handler of a Core Logic delegate: a handler may only read the state"));
		return ECosmicSlotRead::Refused;
	}
	if (Sim->IsInTickStages())
	{
		UE_LOG(LogCosmic, Error, TEXT("LoadGalaxy called inside the tick stages: load from OnSimTick or from input"));
		return ECosmicSlotRead::Refused;
	}
	UCosmicSaveGame* Loaded = nullptr;
	CosmicSaveSlot::FHeader Header;
	const FString Main = SlotNameOf(GalaxyID);
	ECosmicSlotRead Read = TryRead(GalaxyID, false, Loaded, Header);
	// One warning line per load: the backup's reason is told here, never by TryRead.
	if (CosmicSaveSlot::IsDamage(Read))
	{
		const ECosmicSlotRead FromBackup = TryRead(GalaxyID, true, Loaded, Header);
		if (FromBackup == ECosmicSlotRead::Ok)
		{
			UE_LOG(LogCosmic, Warning, TEXT("LoadGalaxy: slot %s: %s; read from its backup (tick %lld)"), *Main, CosmicSaveSlot::NameOf(Read), Header.Tick);
			bOutFromBackup = true;
			Read = ECosmicSlotRead::Ok;
		}
		else if (Read == ECosmicSlotRead::Missing && FromBackup == ECosmicSlotRead::Missing)
		{
			UE_LOG(LogCosmic, Warning, TEXT("LoadGalaxy: slot %s is missing"), *Main);
		}
		else
		{
			UE_LOG(LogCosmic, Warning, TEXT("LoadGalaxy: slot %s: %s; its backup: %s"), *Main, CosmicSaveSlot::NameOf(Read), CosmicSaveSlot::NameOf(FromBackup));
		}
	}
	else if (Read == ECosmicSlotRead::ReadFailed)
	{
		UE_LOG(LogCosmic, Warning, TEXT("LoadGalaxy: slot %s could not be read: the backup is not tried"), *Main);
	}
	else if (Read == ECosmicSlotRead::NewerForm)
	{
		UE_LOG(LogCosmic, Warning, TEXT("LoadGalaxy: slot %s has slot form %d, this build reads up to %d"), *Main,
			static_cast<int32>(Header.Form), static_cast<int32>(CosmicSaveSlot::FormVersion));
	}
	if (Read != ECosmicSlotRead::Ok || !Loaded)
	{
		return Read;
	}
	Sim->LoadState(Loaded->State);
	LatestSavedAt = FMath::Max(LatestSavedAt, Header.SavedAtUtcTicks);
	SetMarkFromState(Sim->ComputeStateHash());
	UE_LOG(LogCosmic, Display, TEXT("CosmicSave: loaded %s%s at tick %lld, hash %u"), *Main, bOutFromBackup ? TEXT(" from its backup") : TEXT(""),
		Loaded->State.Tick, Mark.StateHash);
	return ECosmicSlotRead::Ok;
}

bool UCosmicSaveSubsystem::LoadGalaxy(const FGuid& GalaxyID)
{
	bool bFromBackup = false;
	return LoadRun(GalaxyID, bFromBackup) == ECosmicSlotRead::Ok;
}

ECosmicContinue UCosmicSaveSubsystem::ContinueNewest()
{
	UGalaxySimulationSubsystem* Sim = GetSim();
	if (!Sim || Sim->IsBroadcasting())
	{
		UE_CLOG(Sim != nullptr, LogCosmic, Error, TEXT("ContinueNewest called from a handler of a Core Logic delegate: a handler may only read the state"));
		return ECosmicContinue::NoSlot;
	}
	// The slots of the slot folder only (the engine's listing does not descend either): the game's continue never sees the
	// test folder below it.
	TArray<FString> Files;
	const FString Directory = FPaths::ProjectSavedDir() / TEXT("SaveGames") / Folder;
	IFileManager::Get().FindFiles(Files, *Directory, TEXT(".sav"));
	TArray<CosmicSaveSlot::FSeen> Seen;
	TSet<FGuid> Runs;
	for (const FString& File : Files)
	{
		FGuid ID;
		bool bBackup = false;
		const FString SlotName = FPaths::GetBaseFilename(File);
		if (!CosmicSaveSlot::ParseSlotName(SlotName, ID, bBackup))
		{
			continue;
		}
		// The file found (its name passed the filter, so it is the slot's own path).
		const FString Path = Directory / File;
		const int64 Size = IFileManager::Get().FileSize(*Path);
		// The header alone: a pile of old runs does not slow the start.
		TArray<uint8> Head;
		if (Size >= CosmicSaveSlot::HeaderBytes)
		{
			TUniquePtr<FArchive> Reader(IFileManager::Get().CreateFileReader(*Path, FILEREAD_Silent));
			if (Reader)
			{
				Head.SetNumUninitialized(CosmicSaveSlot::HeaderBytes);
				Reader->Serialize(Head.GetData(), CosmicSaveSlot::HeaderBytes);
				if (Reader->IsError())
				{
					Head.Reset();
				}
			}
		}
		CosmicSaveSlot::FHeader Header;
		const ECosmicSlotRead Read = CosmicSaveSlot::ReadHeader(Head, Size, &ID, Header);
		CosmicSaveSlot::FSeen Each;
		Each.GalaxyID = ID;
		Each.bBackup = bBackup;
		if (Read == ECosmicSlotRead::Ok || Read == ECosmicSlotRead::Truncated || Read == ECosmicSlotRead::NewerForm)
		{
			// The envelope stands: the run's name and time are those of the header (the load sorts out the rest).
			Each.UtcTicks = Header.SavedAtUtcTicks;
			Each.bTimeFromHeader = true;
			LatestSavedAt = FMath::Max(LatestSavedAt, Header.SavedAtUtcTicks);
		}
		else if (Read == ECosmicSlotRead::OldForm)
		{
			UE_LOG(LogCosmic, Log, TEXT("CosmicContinue: %s is in the slot form before item 5 - not a run to continue"), *SlotName);
			continue;
		}
		else
		{
			// A header that does not read still names a run: the file's time orders it (16 #33 (3)).
			Each.UtcTicks = IFileManager::Get().GetTimeStamp(*Path).GetTicks();
		}
		Seen.Add(Each);
		Runs.Add(ID);
	}

	ECosmicContinue Outcome = ECosmicContinue::NoSlot;
	FGuid Newest;
	if (!Seen.IsEmpty())
	{
		Newest = CosmicSaveSlot::NewestRun(Seen);
		bool bFromBackup = false;
		const ECosmicSlotRead Read = LoadRun(Newest, bFromBackup);
		// Never an older run: the newest that does not open is Unreadable, its files kept for the next start.
		Outcome = Read == ECosmicSlotRead::Ok ? (bFromBackup ? ECosmicContinue::Backup : ECosmicContinue::Newest) : ECosmicContinue::Unreadable;
	}
	LastContinue = Outcome;
	const bool bOpened = Outcome == ECosmicContinue::Newest || Outcome == ECosmicContinue::Backup;
	UE_LOG(LogCosmic, Display, TEXT("CosmicContinue: %s %s at tick %s, hash %s (%d runs seen)"), CosmicSave::NameOf(Outcome),
		Newest.IsValid() ? *SlotNameOf(Newest) : TEXT("-"), bOpened ? *FString::Printf(TEXT("%lld"), Sim->GetState().Tick) : TEXT("-"),
		bOpened ? *FString::Printf(TEXT("%u"), Sim->ComputeStateHash()) : TEXT("-"), Runs.Num());
	return Outcome;
}

bool UCosmicSaveSubsystem::DoesSaveExist(const FGuid& GalaxyID) const
{
	return IFileManager::Get().FileSize(*SlotPathOf(GalaxyID)) >= 0 || IFileManager::Get().FileSize(*SlotPathOf(GalaxyID, true)) >= 0;
}

bool UCosmicSaveSubsystem::DeleteSave(const FGuid& GalaxyID)
{
	bool bDeleted = false;
	for (const bool bBackup : { false, true })
	{
		const FString Path = SlotPathOf(GalaxyID, bBackup);
		// The engine's own delete (FGenericSaveGameSystem::DeleteGame): RequireExists, not EvenReadOnly, Quiet. The default
		// arguments answer true for a missing file and warn on a read-only one.
		if (IFileManager::Get().FileSize(*Path) >= 0)
		{
			bDeleted |= IFileManager::Get().Delete(*Path, true, false, true);
		}
	}
	return bDeleted;
}

void UCosmicSaveSubsystem::PrepareForTests()
{
	Folder = TEXT("Automation/");
	bBlockedForTests = true;
	bTestOverride = false;
	bContinueAtNextStart = false;
	Pending.Reset();
	IntervalSeconds = AutosaveRealSeconds;
	RetrySeconds = RetryRealSeconds;
	RetryNotBefore = 0.0;
	bFailureOpen = false;
	bFailureTaken = false;
	Mark = FCosmicSavedMark();
	LastContinue = ECosmicContinue::NotBegun;
	HeldRunID.Invalidate();
}

void UCosmicSaveSubsystem::SetGameSavingForTest(bool bOn)
{
	bTestOverride = bOn;
}

void UCosmicSaveSubsystem::ContinueAtNextStartForTest()
{
	bContinueAtNextStart = true;
}

bool UCosmicSaveSubsystem::ConsumeContinueAtStart()
{
	const bool bWas = bContinueAtNextStart;
	bContinueAtNextStart = false;
	return bWas;
}

void UCosmicSaveSubsystem::SetTimingForTest(double InIntervalSeconds, double InRetrySeconds)
{
	IntervalSeconds = InIntervalSeconds;
	RetrySeconds = InRetrySeconds;
	RetryNotBefore = 0.0;
}

void UCosmicSaveSubsystem::SetFolderForTest(const FString& InFolder)
{
	Folder = InFolder;
}
