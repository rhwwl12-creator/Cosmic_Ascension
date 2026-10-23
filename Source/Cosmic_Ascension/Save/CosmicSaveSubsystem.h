// Cosmic Ascension - Save layer. Design: guideline.md section 6.1, 9.1; headers/A5_save_minimum.md (MVP item 5).
// RULE (section 6.1): files under Save/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"
#include "Misc/Optional.h"
#include "Save/CosmicSaveSlot.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CosmicSaveSubsystem.generated.h"

class UCosmicSaveGame;
class UGalaxySimulationSubsystem;

/** Why a slot was written: log lines and tests only (not in the file, not state, not hashed). */
enum class ECosmicSaveCause : uint8 { Asked, NewRun, Ascended, Role, Ended, Interval, Answer, Exit };

/** What the start of the game did, or the last ContinueNewest. Never saved. */
enum class ECosmicContinue : uint8
{
	NotBegun,     // no start yet in this game instance
	Off,          // the game's saving is off: a new galaxy, nothing read or written - as before item 5
	NoSlot,       // no slot of a run: a new galaxy, written at once (NewRun)
	Newest,       // the newest run, from its slot
	Backup,       // the newest run, from its backup (its slot was damaged)
	Unreadable,   // the newest run did not open: a new galaxy, first written at a tick boundary (decision 6); the files stay
};

namespace CosmicSave
{
	COSMIC_ASCENSION_API const TCHAR* NameOf(ECosmicSaveCause Cause);
	COSMIC_ASCENSION_API const TCHAR* NameOf(ECosmicContinue Outcome);
}

/**
 * FGalaxyState <-> the slot (docs/20 section 9.1): one slot per run (ironman) and its one backup; no manual save or load
 * menu. The slot form is CosmicSaveSlot's (a checked header, the engine bytes packed with Oodle). A write copies the slot
 * that passed its checks to the backup first, then writes the slot - each flushed to the disk, so one good copy is always
 * there. A load falls back to the backup only when the slot is damaged or missing. Test slots live under
 * Saved/SaveGames/Automation/ (PrepareForTests), out of the game's sight.
 */
UCLASS()
class COSMIC_ASCENSION_API UCosmicSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static FString MakeSlotName(const FGuid& GalaxyID);                       // Galaxy_<32 hex digits>
	FString SlotNameOf(const FGuid& GalaxyID, bool bBackup = false) const;   // the slot folder + MakeSlotName (+ "_bak")
	// ProjectSavedDir()/SaveGames/<SlotNameOf>.sav - the generic save system's path (Win64; MVP item 9 checks the package).
	FString SlotPathOf(const FGuid& GalaxyID, bool bBackup = false) const;

	// Writes the running galaxy now (section 3.4 of the MVP item 5 header): the verified slot to the backup first, flushed
	// to the disk - a failed copy, a slot that did not read and a slot of a later form keep the slot and return false; a
	// damaged slot is not copied - then the slot, flushed. Refused with one error line inside a handler of a Core Logic
	// delegate (IsBroadcasting) and inside the tick stages (IsInTickStages).
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

	// What the last ContinueNewest did (tests, log lines).
	ECosmicContinue GetLastContinue() const { return LastContinue; }

	// ---- Tests ----
	void PrepareForTests();                         // CosmicTests::AcquireSim: the slot folder Automation/
	void SetFolderForTest(const FString& Folder);   // e.g. "Automation/Find/": the continue lists that folder only (no descent)
	int32 GetSavesWritten() const { return SavesWritten; }
	TOptional<ECosmicSaveCause> GetLastCause() const { return LastCause; }

private:
	// The one write (section 3.4): refusals, backup, slot.
	bool Write(ECosmicSaveCause Cause);
	// One slot, read and checked through the sanctuary; Out is the loaded object when Ok. The version and sanctuary lines are
	// logged for the slot only (bBackup false): a backup's reason is told by LoadRun's one line.
	ECosmicSlotRead TryRead(const FGuid& GalaxyID, bool bBackup, UCosmicSaveGame*& Out, CosmicSaveSlot::FHeader& OutHeader) const;
	// The file of a slot: Missing (no file), BadSizes (larger than a slot), ReadFailed (did not read twice), else Ok.
	ECosmicSlotRead ReadSlotFile(const FString& Path, TArray<uint8>& Out) const;
	// IPlatformFile::OpenWrite, Write, Flush(true) (FlushFileBuffers on Win64), close. False on any failure.
	bool WriteSlotFile(const FString& Path, TConstArrayView<uint8> Bytes) const;
	UGalaxySimulationSubsystem* GetSim() const;

	FString Folder;                                       // "" (the game), "Automation/" (PrepareForTests) or a test's own
	ECosmicContinue LastContinue = ECosmicContinue::NotBegun;
	int64 LatestSavedAt = 0;                              // every SavedAtUtcTicks this process wrote or loaded: the next is later
	int32 SavesWritten = 0;
	TOptional<ECosmicSaveCause> LastCause;
};
