// Cosmic Ascension - Core layer. Design: guideline.md section 7.1.
// Shared enums and small value structs used by runtime state (Core/GalaxyState.h) and definition rows (Data/).
// Save rule (section 9.1): enumerators are serialized by name. Append new ones at the end; never rename or reorder.

#pragma once

#include "CoreMinimal.h"
#include "CosmicTypes.generated.h"

/**
 * Ascension Tier (section 3.2). Civilizations and the calling each hold one. The ladder has no sixth rung and no Tier 0.
 * Underlying value = tier number - 1 (Tier1_Planetary == 0). Data columns that store a tier as a number hold the tier
 * number 1..5 (compare with static_cast<int32>(Tier) + 1); arrays indexed by tier use static_cast<int32>(Tier).
 */
UENUM(BlueprintType)
enum class EAscensionTier : uint8
{
	Tier1_Planetary UMETA(DisplayName = "Tier 1 - Planetary"),
	Tier2_Stellar UMETA(DisplayName = "Tier 2 - Stellar"),
	Tier3_Causal UMETA(DisplayName = "Tier 3 - Causal"),
	Tier4_Transcendent UMETA(DisplayName = "Tier 4 - Transcendent"),
	Tier5_EternalOrder UMETA(DisplayName = "Tier 5 - Eternal Order")
};

/** Player mode (section 3.3, 5.10). The first life is always Ruler. */
UENUM(BlueprintType)
enum class EPlayerRole : uint8
{
	Ruler,
	Wanderer
};

/** The five resources (section 5.1). */
UENUM(BlueprintType)
enum class ECosmicResource : uint8
{
	Energy,
	Matter,
	Insight,
	Influence,
	Causality
};

/**
 * Population band (section 1.3, 8.3). Derived, never saved, never hashed:
 * UGalaxySimulationSubsystem::GetPopulationBand = civ Tier baseline adjusted by Population (section 7.4).
 * The only way the size of a people reaches UI, log or chronicle. "Remnant" is not a band.
 */
UENUM(BlueprintType)
enum class EPopulationBand : uint8
{
	Few,
	Millions,
	Billions,
	HundredBillions,
	Trillions,
	Countless
};

/** Resource amounts (section 5.1). Used for stockpiles, per-tick income and costs. */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FResourcePool
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Energy = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Matter = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Insight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Influence = 0.f;

	// Unlocked at Tier 3.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Causality = 0.f;

	float Get(ECosmicResource Resource) const;
	void Add(ECosmicResource Resource, float Amount);
	// One multiplication on one member: the same float as Member *= Factor (section 9.2).
	void Scale(ECosmicResource Resource, float Factor);
};

/** Civilization ethics, three axes in [-1, 1] (section 5.2). They drift with crisis outcomes and ascension costs. */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FEthicsAxes
{
	GENERATED_BODY()

	// -1 militarist .. +1 pacifist
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float Pacifism = 0.f;

	// -1 materialist .. +1 spiritualist
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float Spiritualism = 0.f;

	// -1 closed .. +1 open
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float Openness = 0.f;
};
