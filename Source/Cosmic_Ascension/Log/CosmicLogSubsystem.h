// Cosmic Ascension - Log layer. Design: guideline.md section 6.1, 7.5, 8.
// RULE (section 6.1): files under Log/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/GalaxyState.h"
#include "CosmicLogSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLogEntry, const FCosmicLogEntry&, Entry);

/**
 * Narrative log (section 7.5, 8). Renders templates into FGalaxyState.Log, keeps the cap and moves what must be kept
 * into FGalaxyState.Chronicle. Holds no state of its own: the log lives in the saved state and is hashed (all but the
 * text). So far: Push, PushRow, the cap. PickTemplate by revelation context, slot resolution, the interlude mode,
 * BuildChronicle and BuildWorldDigest arrive with Phase 4 and 4b.
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
	 */
	void Push(FGameplayTag Category, uint8 Severity, const FText& Template, const FFormatNamedArguments& Args, FName FocusSystem, FName SourceRow);

	/**
	 * Pushes one line of the DT_LogTemplates row RowName; the row gives the Category and becomes the SourceRow.
	 * Draws exactly one number from the sim stream to pick a template. If the result reads the same as the previous log
	 * line, the next template of the row is used instead, without a second draw: the same sentence never appears twice in
	 * a row, and neither the number of draws nor the number of entries depends on the text (section 9.2).
	 * Returns false and pushes nothing (no draw) when the row is missing or has no template.
	 */
	bool PushRow(FName RowName, uint8 Severity, const FFormatNamedArguments& Args, FName FocusSystem);

	// Fired for every appended entry while the global broadcast gate is open (section 6.1). With the gate closed the
	// entry is only stored. Fired synchronously, also from inside a tick stage: a handler may read the state and call
	// SetTimeScale, but must not call NewGalaxy, LoadGalaxy, SaveCurrentGalaxy, StepOnce or RunHeadless (section 9.4).
	UPROPERTY(BlueprintAssignable)
	FOnLogEntry OnLogEntry;

private:
	void Append(FGameplayTag Category, uint8 Severity, FString&& Rendered, FName FocusSystem, FName SourceRow);
};
