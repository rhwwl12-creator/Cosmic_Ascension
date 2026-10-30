// Cosmic Ascension - Save layer. Design: guideline.md section 6.1, 9.1; headers/A5_save_minimum.md (MVP item 5).
// RULE (section 6.1): files under Save/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "Core/CosmicTypes.h"
#include "Misc/Guid.h"
#include "Misc/Optional.h"
#include "Save/CosmicSaveSlot.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CosmicSaveSubsystem.generated.h"

struct FCosmicRulesDef;
struct FGalaxyState;
class UCosmicSaveGame;
class UGalaxySimulationSubsystem;

/** Why a slot was written: log lines and tests only (not in the file, not state, not hashed). Death and Succession (B5):
 * the boundary at which the calling entered the interregnum, and the one at which a new life began. */
enum class ECosmicSaveCause : uint8 { Asked, NewRun, Ascended, Role, Ended, Interval, Answer, Exit, Death, Succession };

/** What the start of the game did (ACosmicGameMode::StartPlay through BeginSession), or the last ContinueNewest. Never saved. */
enum class ECosmicContinue : uint8
{
	NotBegun,     // no start yet in this game instance
	Off,          // the game's saving is off (IsGameSavingOn): a new galaxy, nothing read or written - as before item 5
	NoSlot,       // no slot of a run: a new galaxy, written at once (NewRun)
	Newest,       // the newest run, from its slot
	Backup,       // the newest run, from its backup (its slot was damaged)
	Unreadable,   // the newest run did not open: a new galaxy, first written at a tick boundary (decision 6); the files stay
};

namespace CosmicSave
{
	COSMIC_ASCENSION_API const TCHAR* NameOf(ECosmicSaveCause Cause);
	COSMIC_ASCENSION_API const TCHAR* NameOf(ECosmicContinue Outcome);
	// The last system is the sanctuary with no owner, hazard or project (section 4.5 (1)): a load refuses a state without it
	// (Sanctuary) and a write does not put one on the disk.
	COSMIC_ASCENSION_API bool KeepsSanctuary(const FGalaxyState& State);
}

/** What the slot of the running galaxy holds, as far as the tick-boundary check needs it: a cache of the disk (the disk copy),
 * replaced whole by every write and every load (losing it costs one more write, nothing else). Not saved, not hashed, read
 * by nothing but the save layer; no fact here that the state does not hold (rule 2). B5 added the number of lives and the
 * LifeState for death and succession (the slot's header is unchanged). */
struct FCosmicSavedMark
{
	FGuid GalaxyID;                                // invalid = nothing written or loaded for this run
	int64 Tick = -1;
	uint32 StateHash = 0;                          // ComputeStateHash at the write or the load (the exit compares it)
	int32 Feats = 0;                               // Calling.CompletedFeats.Num(): an ascension adds one
	EPlayerRole Role = EPlayerRole::Ruler;         // a deposition, an abdication, an enthronement (B1) or a ruler's death (B5 - Death first); the return is 16 #146
	FName RuledCivID;
	bool bEnded = false;                           // CosmicEndings::ProvisionalEndOf != None
	int32 Lives = 0;                               // Calling.Lives.Num(): a succession adds one (B5)
	bool bInterregnum = false;                     // Calling.LifeState == Interregnum: a death makes it true (B5)
	double RealSeconds = 0.0;                      // FPlatformTime::Seconds at the write or the load
};

/**
 * FGalaxyState <-> the slot (docs/20 section 9.1): one slot per run (ironman) and its one backup; no manual save or load
 * menu. The slot form is CosmicSaveSlot's (a checked header, the engine bytes packed with Oodle). A write copies the slot
 * that passed its checks to the backup first, then writes the slot - each flushed to the disk, so one good copy is always
 * there. A load falls back to the backup only when the slot is damaged or missing. The game's own saving - the continue at
 * start, the tick-boundary autosave, the saves of input and exit - goes through one gate (IsGameSavingOn); the explicit
 * calls do not. Test slots live under Saved/SaveGames/Automation/ (PrepareForTests), out of the game's sight.
 */
UCLASS()
class COSMIC_ASCENSION_API UCosmicSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// InitializeDependency<UGalaxySimulationSubsystem>, binds OnSimTick, reads -NoSave, -unattended and -Seed= once
	// (CommandLineAllows).
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// Unbinds; writes nothing (the order of shutdown is not fixed - the exit save is the controller's: the game viewport's
	// OnCloseRequested for a window close or Alt+F4, EndPlay(Quit) for quit or RequestExit).
	virtual void Deinitialize() override;

	static FString MakeSlotName(const FGuid& GalaxyID);                       // Galaxy_<32 hex digits>
	FString SlotNameOf(const FGuid& GalaxyID, bool bBackup = false) const;   // the slot folder + MakeSlotName (+ "_bak")
	// ProjectSavedDir()/SaveGames/<SlotNameOf>.sav - the generic save system's path (Win64; in a Development package that is
	// <package>/Cosmic_Ascension/Saved/SaveGames, in Shipping %LOCALAPPDATA%/Cosmic_Ascension/Saved/SaveGames - MVP item 9).
	FString SlotPathOf(const FGuid& GalaxyID, bool bBackup = false) const;

	// ---- Explicit calls (tests, tools): the gate does not apply ----

	// Writes the running galaxy now (section 3.4 of the MVP item 5 header): the verified slot to the backup first, flushed
	// to the disk - a failed copy, a slot that did not read and a slot of a later form keep the slot and return false; a
	// damaged slot is not copied - then the slot, flushed. Cause Asked. Refused with one error line inside a handler of a
	// Core Logic delegate (IsBroadcasting) and inside the tick stages (IsInTickStages).
	UFUNCTION(BlueprintCallable)
	bool SaveCurrentGalaxy();

	// Loads a run: its slot, or its backup when the slot is damaged (CosmicSaveSlot::IsDamage) or missing; a slot that did
	// not read, one of a later form or version and one without a kept sanctuary are refused without trying the backup. A
	// refused load leaves the simulation untouched. Refused with one error line inside a handler or the tick stages.
	UFUNCTION(BlueprintCallable)
	bool LoadGalaxy(const FGuid& GalaxyID);
	ECosmicSlotRead LoadRun(const FGuid& GalaxyID, bool& bOutFromBackup);

	// The continue (16 #33 (3)): the slots of the slot folder, 64 header bytes each, the newest run loaded
	// (CosmicSaveSlot::NewestRun); never an older run, never a delete. Newest, Backup, Unreadable or NoSlot. Refused (NoSlot,
	// one error line) in a handler.
	ECosmicContinue ContinueNewest();

	UFUNCTION(BlueprintPure)
	bool DoesSaveExist(const FGuid& GalaxyID) const;   // the slot or its backup

	UFUNCTION(BlueprintCallable)
	bool DeleteSave(const FGuid& GalaxyID);            // both; true when one was deleted

	// ---- The game's own saving ----

	// GateClosedBy over this process: off with -NoSave, -unattended, -Seed=, in the editor process, while an automation test
	// runs, and after PrepareForTests - unless SetGameSavingForTest(true).
	bool IsGameSavingOn() const;
	// The gate as pure functions (Sim.SaveTriggers: one case per reason). CommandLineAllows: none of -NoSave, -unattended,
	// -Seed= in CommandLine. GateClosedBy: the first reason that closes it ("command line", "tests", "automation",
	// "editor"), null when open; bTestOverride opens it whatever the rest.
	static bool CommandLineAllows(const TCHAR* CommandLine);
	static const TCHAR* GateClosedBy(bool bCommandLineAllows, bool bBlockedForTests, bool bAutomationTesting, bool bEditor, bool bTestOverride);

	// Once per start, from ACosmicGameMode::StartPlay before Super::StartPlay: Off (one line with the reason) when the gate
	// is closed, else ContinueNewest. The load's OnGalaxyGenerated reaches no listener yet; both BeginPlay calls read the
	// state (headers/A4 4.4 (5)).
	ECosmicContinue BeginSession();
	// The controller's BeginPlay takes it once (then NotBegun): a later map load without a start of its own says nothing.
	ECosmicContinue TakeStartOutcome();
	// What the last BeginSession or ContinueNewest did (tests, log lines); never cleared by TakeStartOutcome.
	ECosmicContinue GetLastContinue() const { return LastContinue; }
	// StartPlay, right after the new galaxy of an Unreadable start (decision 6): notes that galaxy's GalaxyID. That run alone
	// is first written at a tick boundary and its exit save is skipped while it is unwritten at tick 0, so the run that did
	// not read stays the newest on disk; a later new run of the session is written at once. BeginSession clears it.
	void HoldStartRun();

	// Input time (no handler, no stage): writes now - except Exit when the slot already holds this state (same hash), and
	// Exit at tick 0 of the run HoldStartRun noted while it is unwritten (decision 6). From a handler or a stage, and after a
	// failed write of the game's path: kept and written at the next tick boundary (after a failure, once RetryRealSeconds
	// have passed). A write that succeeds, on any path, clears what was kept and the wait. Nothing while the gate is closed
	// or without a galaxy.
	void RequestSave(ECosmicSaveCause Cause);

	// True once per game instance, for the first failed write of the game's saving that no later write has put right yet (the
	// controller stops the time once; a failure written over before the controller looked is not told).
	bool TakeFirstFailure();

	// ---- Tests ----
	void PrepareForTests();                         // CosmicTests::AcquireSim: folder Automation/, gate off, override off, timing reset
	void SetFolderForTest(const FString& Folder);   // e.g. "Automation/Find/": the continue lists that folder only (no descent)
	void SetGameSavingForTest(bool bOn);            // opens the gate for one test; the next PrepareForTests closes it
	void ContinueAtNextStartForTest();              // the next StartPlay continues even over a running galaxy (a map reload)
	bool ConsumeContinueAtStart();                  // StartPlay reads it once
	void SetTimingForTest(double InIntervalSeconds, double InRetrySeconds);
	int32 GetSavesWritten() const { return SavesWritten; }
	TOptional<ECosmicSaveCause> GetLastCause() const { return LastCause; }

	// docs/20 section 9.1's "every ten minutes": real seconds since the last write, looked at on a tick boundary (decision 13).
	static constexpr double AutosaveRealSeconds = 600.0;
	// After a failed write of the game's path the boundary waits this long before it writes again (decision 12).
	static constexpr double RetryRealSeconds = 60.0;

	// The boundary rule as pure functions (tests). DueAt: NewRun when the galaxy differs, else Ended, else Ascended, else Death
	// (the calling entered the interregnum - B5), else Succession (a life was added), else Role, else Interval (IntervalSeconds
	// passed and the tick moved), else unset. MarkOf leaves StateHash and RealSeconds to the write.
	static TOptional<ECosmicSaveCause> DueAt(const FCosmicSavedMark& Saved, const FCosmicSavedMark& Now, double NowRealSeconds, double IntervalSeconds);
	static FCosmicSavedMark MarkOf(const FGalaxyState& State, const FCosmicRulesDef& Rules);

private:
	UFUNCTION()
	void HandleSimTick(int64 Tick);

	// The one write (section 3.4): refusals, backup, slot, disk copy. bGamePath: the gate's paths - a failure there counts
	// for TakeFirstFailure, the retry wait and the kept cause; an explicit SaveCurrentGalaxy's does not.
	bool Write(ECosmicSaveCause Cause, bool bGamePath);
	// One slot, read and checked through the sanctuary; Out is the loaded object when Ok. The version and sanctuary lines are
	// logged for the slot only (bBackup false): a backup's reason is told by LoadRun's one line.
	ECosmicSlotRead TryRead(const FGuid& GalaxyID, bool bBackup, UCosmicSaveGame*& Out, CosmicSaveSlot::FHeader& OutHeader) const;
	// The file of a slot: Missing (no file), BadSizes (larger than a slot), ReadFailed (did not read twice), else Ok.
	ECosmicSlotRead ReadSlotFile(const FString& Path, TArray<uint8>& Out) const;
	// IPlatformFile::OpenWrite, Write, Flush(true) (FlushFileBuffers on Win64), close. False on any failure.
	bool WriteSlotFile(const FString& Path, TConstArrayView<uint8> Bytes) const;
	void SetMarkFromState(uint32 StateHash);
	UGalaxySimulationSubsystem* GetSim() const;

	FString Folder;                                       // "" (the game), "Automation/" (PrepareForTests) or a test's own
	FCosmicSavedMark Mark;                                // the disk copy
	TOptional<ECosmicSaveCause> Pending;                  // asked from a handler or a stage, or failed: the next due boundary writes it; any successful write clears it
	ECosmicContinue StartOutcome = ECosmicContinue::NotBegun;
	ECosmicContinue LastContinue = ECosmicContinue::NotBegun;
	FGuid HeldRunID;                                      // HoldStartRun: the new run of an Unreadable start (decision 6); invalid otherwise
	bool bCommandLineAllows = false;                      // CommandLineAllows(FCommandLine::Get()) in Initialize
	bool bBlockedForTests = false;
	bool bTestOverride = false;
	bool bContinueAtNextStart = false;
	bool bFailureOpen = false;                            // a write of the game's path failed and none has succeeded since
	bool bFailureTaken = false;
	double RetryNotBefore = 0.0;                          // FPlatformTime::Seconds; boundary writes wait for it after a failure
	double IntervalSeconds = AutosaveRealSeconds;
	double RetrySeconds = RetryRealSeconds;
	int64 LatestSavedAt = 0;                              // every SavedAtUtcTicks this process wrote or loaded: the next is later
	int32 SavesWritten = 0;
	TOptional<ECosmicSaveCause> LastCause;
};
