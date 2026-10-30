// Cosmic Ascension - Sim layer (Core Logic).

#include "Sim/CosmicSuccession.h"

#include "Core/CosmicTags.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "GameplayTagContainer.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicSuccessionImpl
{
	// The largest float below 1: a number at 1 or past it (an infinity too) reads as the last - the cast below stays defined.
	constexpr float BelowOne = 0.99999994f;

	// Whether the sphere of an unsealed fallen (Realm.Abyssal) instance holds SystemID.
	bool InUnsealedFallenSphere(const FGalaxyState& State, const FCosmicDataTables& Tables, FName SystemID)
	{
		const FGameplayTag AbyssalTag = CosmicTags::RealmAbyssal();
		for (const FTranscendentState& Transcendent : State.Transcendents)
		{
			const FTranscendentDef* Def = Transcendent.bSealed ? nullptr : Tables.FindTranscendent(Transcendent.TranscendentID);
			if (Def && Def->Allegiance == AbyssalTag && Transcendent.InfluencedSystems.Contains(SystemID))
			{
				return true;
			}
		}
		return false;
	}
}

CosmicSuccession::FCosmicSuccessionCandidates CosmicSuccession::CandidatesOf(const FGalaxyState& State, const FCosmicStateIndex& Index,
	const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 DeathSystemIndex)
{
	FCosmicSuccessionCandidates Found;
	if (!State.Systems.IsValidIndex(DeathSystemIndex))
	{
		return Found;
	}
	const FName GalaxyRow = State.Systems[DeathSystemIndex].GalaxyRow;
	// The pool the two fixed filters leave: the settled systems of the row but the death place and a lethal hazard's system.
	TArray<int32> Pool;
	for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num(); ++SystemIndex)
	{
		const FStarSystemState& System = State.Systems[SystemIndex];
		if (SystemIndex != DeathSystemIndex && System.GalaxyRow == GalaxyRow && CosmicState::IsSettled(System)
			&& !(System.ActiveHazard.IsValid() && CosmicHazards::KillsAVesselOf(Tables, System.ActiveHazard)))
		{
			Pool.Add(SystemIndex);
		}
	}
	// The relaxing filters, in the order they are let go of: 0 both, 1 the sphere only, 2 none.
	for (int32 Relaxed = 0; Relaxed <= 2; ++Relaxed)
	{
		for (const int32 SystemIndex : Pool)
		{
			const FStarSystemState& System = State.Systems[SystemIndex];
			const bool bShortHazard = System.ActiveHazard.IsValid() && System.HazardTicksRemaining < Rules.SuccessionMinHazardTicks;
			if ((Relaxed < 1 && bShortHazard) || (Relaxed < 2 && CosmicSuccessionImpl::InUnsealedFallenSphere(State, Tables, System.SystemID)))
			{
				continue;
			}
			Found.Systems.Add(SystemIndex);
		}
		if (!Found.Systems.IsEmpty())
		{
			Found.Relaxed = Relaxed;
			return Found;
		}
	}
	return Found;
}

int32 CosmicSuccession::PickOf(float Number, int32 Count)
{
	if (Count <= 0)
	{
		return INDEX_NONE;
	}
	const float Read = Number >= 0.f ? (Number < 1.f ? Number : CosmicSuccessionImpl::BelowOne) : 0.f;
	return FMath::Min(static_cast<int32>(Read * static_cast<float>(Count)), Count - 1);
}

int64 CosmicSuccession::LeapOf(const FCosmicRulesDef& Rules, int64 LeapTicksTotal, float Number)
{
	const int64 Lo = FMath::Max<int64>(0, Rules.LeapTicksMin);
	const int64 Hi = FMath::Max<int64>(Lo, Rules.LeapTicksMax);
	const float Read = Number >= 0.f ? (Number < 1.f ? Number : CosmicSuccessionImpl::BelowOne) : 0.f;
	const int64 Span = Hi - Lo;
	const int64 Leap = Lo + FMath::Min<int64>(static_cast<int64>(Read * static_cast<float>(Span + 1)), Span);
	return FMath::Min<int64>(Leap, FMath::Max<int64>(0, Rules.LeapTotalCap - LeapTicksTotal));
}
