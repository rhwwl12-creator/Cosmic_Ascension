// Cosmic Ascension - Log layer. Design: guideline.md section 6.1, 7.5, 8.
// RULE (section 6.1): files under Log/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "Containers/ArrayView.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/GalaxyState.h"
#include "Log/CosmicChronicle.h"
#include "CosmicLogSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLogEntry, const FCosmicLogEntry&, Entry);

/**
 * What a line is about and who may be told it (section 8.1). The arguments of PushVariants: nothing here is saved
 * beyond the fields FCosmicLogEntry already has.
 */
struct FCosmicLine
{
	// Log.*
	FGameplayTag Category;

	// 0 normal, 1 notable, 2 mythic.
	uint8 Severity = 0;

	// Subject system and subject civilization (NAME_None = none). The scope of telling judges both.
	FName FocusSystem;
	FName FocusCiv;

	// Row behind the line: event, hazard, encounter, feat, transcendent or log-template row.
	FName SourceRow;

	// Viewer tier window, tier numbers 1..5, 0 = no bound on that side. Outside the window the line is out of scope
	// exactly as a line about an unmet people is: stored bUntold when it is Severity 2, Log.Claim or Log.Record, not
	// stored otherwise - but a window alone never calls for a far line (section 8.1).
	uint8 MinViewerTier = 0;
	uint8 MaxViewerTier = 0;
};

/**
 * Narrative log (section 7.5, 8). Renders templates into FGalaxyState.Log, judges each line by the scope of telling
 * (section 8.1), keeps the cap and moves what must be kept into FGalaxyState.Chronicle. Holds no state of its own but
 * the tell-all mode: the log lives in the saved state, is hashed by ComputeStateHash (all but the text) and is left out
 * of ComputeWorldHash. So far: Push, PushVariants, PushRow, the scope with bUntold and the far lines, the tell-all mode,
 * the cap, and the chronicle's minimal form (BuildChronicle - MVP item 4). PickTemplateRow by revelation context, slot
 * resolution, the interlude mode, the two books, the epitaph and BuildWorldDigest arrive with Phase 4 and 4b.
 */
UCLASS()
class COSMIC_ASCENSION_API UCosmicLogSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// FGalaxyState.Log keeps the most recent entries up to this count (section 8.4).
	static constexpr int32 MaxLogEntries = 2000;

	// The tell-all mode (section 8.1): with it on every line is stored and nothing is judged by the scope of telling (no
	// bUntold, no far line). Not saved: a mode of the run, off in the game, on in the tests (CosmicTests::AcquireSim) and
	// in the balance commandlet. The world hash is the same either way.
	void SetTellAll(bool bOn) { bTellAll = bOn; }
	bool IsTellAll() const { return bTellAll; }

	// The chronicle up to the current tick: CosmicChronicle::Build over the live state; reads only and draws nothing, so a
	// handler may call it. Phase 4b adds the two books, the epitaph and the digest around the same chapters.
	TArray<FCosmicChronicleChapter> BuildChronicle() const;

	/**
	 * Renders Template with Args and appends one entry stamped with the current tick. The stored Message is the flattened
	 * result (culture-invariant text): no format history reaches the save and the line keeps its language (section 7.5).
	 * SourceRow names the row behind the line (event, hazard, encounter, feat or log-template row).
	 * FocusSystem and FocusCiv name the subject system and the subject civilization of the line (section 8.1): the system
	 * the line calls by name, and its people. The line is judged by the scope of telling as PushVariants judges one (no
	 * draw here; a far line that follows draws its own). From a handler of a Core Logic delegate: an error line and
	 * nothing (IsBroadcasting).
	 */
	void Push(FGameplayTag Category, uint8 Severity, const FText& Template, const FFormatNamedArguments& Args, FName FocusSystem, FName SourceRow, FName FocusCiv = NAME_None);

	/**
	 * Tells one line out of a caller-owned array of sentence variants: an event row's LogTemplates, the Templates of a
	 * DT_LogTemplates row, a transcendent row's ClaimTemplates or VoiceTemplates. The single home of the variant rule
	 * and the only caller of DrawTell (section 7.5, 9.2).
	 * Draws exactly one number from the tell stream, also for a single variant, before the scope is looked at, and never
	 * from the sim stream. If the rendered result reads the same as the last line that was told (entries marked bUntold
	 * are passed by) and there is more than one variant, the next variant is used, without a second draw: the same
	 * sentence never appears twice in a row in the feed, and the number of draws does not depend on the text.
	 * Then the scope of telling (section 8.1), judged against the state and Knowing at this moment - unless the tell-all
	 * mode is on, which stores every line:
	 * - The subject holds when Category is Log.Calling, or when FocusCiv is none or a met people whose Tier is not three
	 *   or more above the viewer's (UGalaxySimulationSubsystem::GetViewerTier; a lower people is never held back), and
	 *   FocusSystem is none or a known system. The line is in scope when the subject holds and the viewer's tier number
	 *   lies inside [MinViewerTier, MaxViewerTier] (0 = no bound on that side).
	 * - In scope: stored, and broadcast by OnLogEntry. Out of scope, Severity 2 or Log.Claim or Log.Record: stored with
	 *   bUntold and never broadcast. Otherwise: not stored.
	 * - A Severity 2 line out of scope because its subject does not hold (a window alone calls for none) is followed by
	 *   one far line with no subject, through its own PushRow and its own draw: CosmicRows::LogFarAscent when the line is
	 *   Log.Ascension and its people stands on the third rung or above, every time; CosmicRows::LogFarEvent otherwise,
	 *   only when State.Tick >= Knowing.FarLineUntilTick, which it then sets to Tick + DT_Rules.FarLineCooldownTicks
	 *   (read as at least 0).
	 * Returns false and tells nothing (no draw) when Variants is empty, and when called from a handler of a Core Logic
	 * delegate (an error line - a handler may only read the state); otherwise true, whatever the scope.
	 */
	bool PushVariants(const FCosmicLine& Line, TConstArrayView<FText> Variants, const FFormatNamedArguments& Args);

	/**
	 * Pushes one line of the DT_LogTemplates row RowName, a named row: PushVariants with the row's Category and
	 * Templates, SourceRow = RowName and MinViewerTier = the row's MinTierToReveal, so the scope and the far line are
	 * those of PushVariants. Returns false and pushes nothing (no draw) when the row is missing or has no template, and
	 * when it is called from a handler of a Core Logic delegate (an error line, through PushVariants).
	 */
	bool PushRow(FName RowName, uint8 Severity, const FFormatNamedArguments& Args, FName FocusSystem, FName FocusCiv = NAME_None);

	// Fired for every appended entry that is told (never for one marked bUntold) while the global broadcast gate is open
	// (section 6.1, 8.1). With the gate closed the entry is only stored. Fired synchronously, also from inside a tick
	// stage: a handler may read the state and call SetTimeScale and nothing else - the calls that are refused with an error
	// line are listed at UGalaxySimulationSubsystem::IsBroadcasting (Push, PushVariants and PushRow among them), and
	// SaveCurrentGalaxy is refused there; RequestSave waits for the tick boundary. OnEventFired and OnTierAscended have the
	// same contract (section 9.4).
	UPROPERTY(BlueprintAssignable)
	FOnLogEntry OnLogEntry;

private:
	// Judges Line by the scope of telling and stores Rendered accordingly; then the far line, when one is due. The one
	// place behind Push and PushVariants.
	void Tell(const FCosmicLine& Line, FString&& Rendered);

	void Append(FGameplayTag Category, uint8 Severity, FString&& Rendered, FName FocusSystem, FName FocusCiv, FName SourceRow, bool bUntold);

	// The chronicle's cap (MVP item 5, DT_Rules.ChronicleCap; 0 or below = none): over it, the oldest entries
	// CosmicChronicle::MayLetGo allows leave in one stable pass and Eviction records them.
	static void TrimChronicle(TArray<FCosmicLogEntry>& Chronicle, FChronicleEviction& Eviction, int32 Cap);

	bool bTellAll = false;
};
