// Cosmic Ascension - Save layer. Design: guideline.md section 6.1, 9.1.
// RULE (section 6.1): files under Save/ must never include UUserWidget or AActor headers.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CosmicSaveSubsystem.generated.h"

/**
 * FGalaxyState <-> USaveGame, one slot per galaxy (ironman, section 9.1). The slot name comes from FGalaxyState.GalaxyID.
 * There is no manual save or load menu. The autosave triggers of section 9.1 call SaveCurrentGalaxy at a tick boundary.
 */
UCLASS()
class COSMIC_ASCENSION_API UCosmicSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static FString MakeSlotName(const FGuid& GalaxyID);

	// Saves the running galaxy into its slot. Call at a tick boundary only (after the last tick stage), never during a leap.
	UFUNCTION(BlueprintCallable)
	bool SaveCurrentGalaxy();

	// Loads a galaxy's slot into the simulation. Fails on a missing slot and on a save newer than this build.
	UFUNCTION(BlueprintCallable)
	bool LoadGalaxy(const FGuid& GalaxyID);

	UFUNCTION(BlueprintPure)
	bool DoesSaveExist(const FGuid& GalaxyID) const;

	UFUNCTION(BlueprintCallable)
	bool DeleteSave(const FGuid& GalaxyID);
};
