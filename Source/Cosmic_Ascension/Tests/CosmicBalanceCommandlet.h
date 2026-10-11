// Cosmic Ascension - balance commandlet. Design: guideline.md section 9.3.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CosmicBalanceCommandlet.generated.h"

/**
 * Headless balance run: generates a galaxy, runs it and writes one CSV row per civilization every 100 ticks
 * (resources, Tier, Stability, hazard count).
 *
 *   UnrealEditor-Cmd Cosmic_Ascension.uproject -run=CosmicBalance -Seed=42 -Ticks=20000 -Systems=60 -Civs=8 -Out=balance.csv
 *
 * -Out is relative to Saved/Balance unless it is an absolute path. A commandlet has no GameInstance, so Main creates
 * one to get the subsystems. The policy, death and archetype arguments of section 9.3 arrive with Phase 5.
 */
UCLASS()
class COSMIC_ASCENSION_API UCosmicBalanceCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	virtual int32 Main(const FString& Params) override;
};
