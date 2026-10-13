// Cosmic Ascension - Log layer. Design: guideline.md section 6.1, 7.5, 8.
// RULE (section 6.1): files under Log/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "Containers/ArrayView.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/GalaxyState.h"
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
	// stored otherwise. Carried from now on; read by the scope of telling (the known scope and log work of Phase 2).
	uint8 MinViewerTier = 0;
	uint8 MaxViewerTier = 0;
};

/**
 * Narrative log (section 7.5, 8). Renders templates into FGalaxyState.Log, keeps the cap and moves what must be kept
 * into FGalaxyState.Chronicle. Holds no state of its own: the log lives in the saved state, is hashed by
 * ComputeStateHash (all but the text) and is left out of ComputeWorldHash. So far: Push, PushVariants, PushRow, the cap.
 * The scope of telling, bUntold and SetTellAll arrive with the known scope and log work of Phase 2; PickTemplateRow by revelation
 * context, slot resolution, the interlude mode, BuildChronicle and BuildWorldDigest with Phase 4 and 4b.
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

	/**
	 * Renders Template with Args and appends one entry stamped with the current tick. The stored Message is the flattened
	 * result (culture-invariant text): no format history reaches the save and the line keeps its language (section 7.5).
	 * SourceRow names the row behind the line (event, hazard, encounter, feat or log-template row).
	 * FocusSystem and FocusCiv name the subject system and the subject civilization of the line (section 8.1); the
	 * scope of telling reads them from Phase 2.
	 */
	void Push(FGameplayTag Category, uint8 Severity, const FText& Template, const FFormatNamedArguments& Args, FName FocusSystem, FName SourceRow, FName FocusCiv = NAME_None);

	/**
	 * Tells one line out of a caller-owned array of sentence variants: an event row's LogTemplates, the Templates of a
	 * DT_LogTemplates row, a transcendent row's ClaimTemplates or VoiceTemplates. The single home of the variant rule
	 * and the only caller of DrawTell (section 7.5, 9.2).
	 * Draws exactly one number from the tell stream, also for a single variant, and never from the sim stream. If the
	 * rendered result reads the same as the last stored log line and there is more than one variant, the next variant
	 * is used, without a second draw: the same sentence never appears twice in a row, and neither the number of draws
	 * nor the number of entries depends on the text. Returns false and tells nothing (no draw) when Variants is empty;
	 * otherwise true.
	 */
	bool PushVariants(const FCosmicLine& Line, TConstArrayView<FText> Variants, const FFormatNamedArguments& Args);

	/**
	 * Pushes one line of the DT_LogTemplates row RowName, a named row: PushVariants with the row's Category and
	 * Templates, SourceRow = RowName and MinViewerTier = the row's MinTierToReveal. Returns false and pushes nothing
	 * (no draw) when the row is missing or has no template.
	 */
	bool PushRow(FName RowName, uint8 Severity, const FFormatNamedArguments& Args, FName FocusSystem, FName FocusCiv = NAME_None);

	// Fired for every appended entry while the global broadcast gate is open (section 6.1). With the gate closed the
	// entry is only stored. Fired synchronously, also from inside a tick stage: a handler may read the state and call
	// SetTimeScale, but must not call NewGalaxy, LoadGalaxy, SaveCurrentGalaxy, StepOnce or RunHeadless, nor the evaluator
	// (UCosmicEventSubsystem::EvaluateAndFire, FireRow, Apply). OnEventFired has the same contract (section 9.4).
	UPROPERTY(BlueprintAssignable)
	FOnLogEntry OnLogEntry;

private:
	void Append(FGameplayTag Category, uint8 Severity, FString&& Rendered, FName FocusSystem, FName FocusCiv, FName SourceRow);
};
