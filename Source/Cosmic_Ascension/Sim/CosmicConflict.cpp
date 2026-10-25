// Cosmic Ascension - Sim layer (Core Logic).

#include "Sim/CosmicConflict.h"

#include "Core/CosmicIDs.h"
#include "Core/CosmicTags.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Math/NumericLimits.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"

namespace CosmicConflictImpl
{
	// A factor read at least 0 and finite, else Fallback (comparisons, no CRT call - rule 6).
	float FactorOr(float Value, float Fallback)
	{
		return (Value >= 0.f && Value <= TNumericLimits<float>::Max()) ? Value : Fallback;
	}

	// Whether the system is one of the people's: an entry of OwnedSystems whose owner is the people.
	bool IsOwnedBy(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCivilizationState& Civ, FName SystemID, int32& OutIndex)
	{
		OutIndex = Index.FindSystem(State, SystemID);
		return State.Systems.IsValidIndex(OutIndex) && State.Systems[OutIndex].OwnerCivID == Civ.CivID;
	}

	// Whether a settled system the people CivIndex owns lies one lane from Target, in Target's GalaxyRow.
	bool IsNextToSettled(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex, const FStarSystemState& Target)
	{
		const FCivilizationState& Civ = State.Civs[CivIndex];
		for (const FName SystemID : Civ.OwnedSystems)
		{
			int32 Foothold = INDEX_NONE;
			if (IsOwnedBy(State, Index, Civ, SystemID, Foothold) && CosmicState::IsSettled(State.Systems[Foothold])
				&& State.Systems[Foothold].GalaxyRow == Target.GalaxyRow && State.Systems[Foothold].Lanes.Contains(Target.SystemID))
			{
				return true;
			}
		}
		return false;
	}

	int32 TierGap(EAscensionTier Above, EAscensionTier Below)
	{
		return FMath::Max(0, static_cast<int32>(Above) - static_cast<int32>(Below));
	}
}

int32 CosmicConflict::SettledCountOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex)
{
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return 0;
	}
	int32 Count = 0;
	for (const FName SystemID : State.Civs[CivIndex].OwnedSystems)
	{
		int32 SystemIndex = INDEX_NONE;
		Count += (CosmicConflictImpl::IsOwnedBy(State, Index, State.Civs[CivIndex], SystemID, SystemIndex) && CosmicState::IsSettled(State.Systems[SystemIndex])) ? 1 : 0;
	}
	return Count;
}

int32 CosmicConflict::OwnedCountOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex)
{
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return 0;
	}
	int32 Count = 0;
	for (const FName SystemID : State.Civs[CivIndex].OwnedSystems)
	{
		int32 SystemIndex = INDEX_NONE;
		Count += CosmicConflictImpl::IsOwnedBy(State, Index, State.Civs[CivIndex], SystemID, SystemIndex) ? 1 : 0;
	}
	return Count;
}

float CosmicConflict::FleetCapOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex, const FCosmicRulesDef& Rules)
{
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return 0.f;
	}
	const int32 At = static_cast<int32>(State.Civs[CivIndex].Tier);
	const float PerSettled = Rules.FleetPerSettledByTier.IsValidIndex(At) ? Rules.FleetPerSettledByTier[At] : 0.f;
	if (!(PerSettled > 0.f && PerSettled <= TNumericLimits<float>::Max()))
	{
		return 0.f;
	}
	return PerSettled * static_cast<float>(SettledCountOf(State, Index, CivIndex));
}

float CosmicConflict::FleetGrowthMultiplierOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables)
{
	float Product = 1.f;
	for (const FName Row : Civ.ActiveEdicts)
	{
		const FEdictDef* Def = Tables.FindEdict(Row);
		if (Def && Def->FleetGrowthMultiplier > 0.f && Def->FleetGrowthMultiplier <= TNumericLimits<float>::Max())
		{
			Product *= Def->FleetGrowthMultiplier;
		}
	}
	return Product;
}

float CosmicConflict::EffectivePowerOf(float Power, int32 TierGap, const FCosmicRulesDef& Rules)
{
	if (!(Power > 0.f && Power <= TNumericLimits<float>::Max()))
	{
		return 0.f;
	}
	const int32 At = FMath::Clamp(TierGap, 0, 2);
	const float Factor = Rules.FleetEffectivenessByTierGap.IsValidIndex(At) ? CosmicConflictImpl::FactorOr(Rules.FleetEffectivenessByTierGap[At], 0.f) : 0.f;
	return Power * Factor;
}

int32 CosmicConflict::AttackGapOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 Attacker, int32 SystemIndex)
{
	if (!State.Civs.IsValidIndex(Attacker) || !State.Systems.IsValidIndex(SystemIndex))
	{
		return 0;
	}
	const int32 Owner = Index.FindCiv(State, State.Systems[SystemIndex].OwnerCivID);
	return State.Civs.IsValidIndex(Owner) ? CosmicConflictImpl::TierGap(State.Civs[Owner].Tier, State.Civs[Attacker].Tier) : 0;
}

int32 CosmicConflict::BesiegerOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 SystemIndex)
{
	if (!State.Systems.IsValidIndex(SystemIndex))
	{
		return INDEX_NONE;
	}
	const FStarSystemState& System = State.Systems[SystemIndex];
	for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
	{
		const FCivilizationState& Civ = State.Civs[CivIndex];
		if (Civ.FleetTargetSystemID == System.SystemID && !System.OwnerCivID.IsNone() && System.OwnerCivID != Civ.CivID)
		{
			return CivIndex;
		}
	}
	return INDEX_NONE;
}

bool CosmicConflict::IsBesieging(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex)
{
	if (!State.Civs.IsValidIndex(CivIndex) || State.Civs[CivIndex].FleetTargetSystemID.IsNone())
	{
		return false;
	}
	const int32 Target = Index.FindSystem(State, State.Civs[CivIndex].FleetTargetSystemID);
	return State.Systems.IsValidIndex(Target) && !State.Systems[Target].OwnerCivID.IsNone() && State.Systems[Target].OwnerCivID != State.Civs[CivIndex].CivID;
}

CosmicConflict::ECosmicSiegeBlock CosmicConflict::SiegeBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules,
	int32 Attacker, int32 SystemIndex, bool bAssumeHostile)
{
	// Law.Hedge (rule 13): the first reason, whoever asks.
	if (State.Systems.IsValidIndex(SystemIndex) && CosmicState::IsSanctuary(State.Systems[SystemIndex].SystemID))
	{
		return ECosmicSiegeBlock::Sanctuary;
	}
	if (!State.Civs.IsValidIndex(Attacker) || !State.Systems.IsValidIndex(SystemIndex))
	{
		return ECosmicSiegeBlock::NoSystem;
	}
	const FStarSystemState& System = State.Systems[SystemIndex];
	if (CosmicHazards::IsDeadNode(System))
	{
		return ECosmicSiegeBlock::Dead;
	}
	const int32 OwnerIndex = Index.FindCiv(State, System.OwnerCivID);
	if (!State.Civs.IsValidIndex(OwnerIndex))
	{
		return ECosmicSiegeBlock::Unowned;
	}
	if (OwnerIndex == Attacker)
	{
		return ECosmicSiegeBlock::Own;
	}
	if (!CosmicState::IsSettled(System))
	{
		return ECosmicSiegeBlock::NotSettled;
	}
	const FCivilizationState& Civ = State.Civs[Attacker];
	const FCivilizationState& Owner = State.Civs[OwnerIndex];
	if (Owner.HomeSystemID == System.SystemID)
	{
		return ECosmicSiegeBlock::Capital;
	}
	if (CosmicHazards::IsLastSettledSystem(State, Index, SystemIndex))
	{
		return ECosmicSiegeBlock::LastSettled;
	}
	if (!CosmicConflictImpl::IsNextToSettled(State, Index, Attacker, System))
	{
		return ECosmicSiegeBlock::NotAdjacent;
	}
	for (int32 Other = 0; Other < State.Civs.Num(); ++Other)
	{
		if (Other != Attacker && Other != OwnerIndex && State.Civs[Other].FleetTargetSystemID == System.SystemID)
		{
			return ECosmicSiegeBlock::Besieged;
		}
	}
	if (OwnedCountOf(State, Index, Attacker) >= CosmicMegaprojects::RealmCapOf(Rules, Civ.Tier))
	{
		return ECosmicSiegeBlock::RealmCap;
	}
	if (!(EffectivePowerOf(Civ.FleetPower, CosmicConflictImpl::TierGap(Owner.Tier, Civ.Tier), Rules) > 0.f))
	{
		return ECosmicSiegeBlock::NoFleet;
	}
	if (CosmicDiplomacy::HasTreaty(Civ, Owner.CivID, CosmicTags::TreatyNonAggression()))
	{
		return ECosmicSiegeBlock::NonAggression;
	}
	if (!bAssumeHostile && CosmicDiplomacy::StanceOf(Civ, Owner.CivID) != CosmicTags::StanceHostile())
	{
		return ECosmicSiegeBlock::NotHostile;
	}
	return ECosmicSiegeBlock::None;
}

CosmicConflict::ECosmicGuardBlock CosmicConflict::GuardBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules,
	int32 CivIndex, int32 SystemIndex)
{
	if (State.Systems.IsValidIndex(SystemIndex) && CosmicState::IsSanctuary(State.Systems[SystemIndex].SystemID))
	{
		return ECosmicGuardBlock::Sanctuary;
	}
	if (!State.Civs.IsValidIndex(CivIndex) || !State.Systems.IsValidIndex(SystemIndex))
	{
		return ECosmicGuardBlock::NoSystem;
	}
	const FStarSystemState& System = State.Systems[SystemIndex];
	if (CosmicHazards::IsDeadNode(System))
	{
		return ECosmicGuardBlock::Dead;
	}
	if (System.OwnerCivID != State.Civs[CivIndex].CivID)
	{
		return ECosmicGuardBlock::NotOwn;
	}
	if (!CosmicState::IsSettled(System))
	{
		return ECosmicGuardBlock::NotSettled;
	}
	if (!(FleetCapOf(State, Index, CivIndex, Rules) > 0.f))
	{
		return ECosmicGuardBlock::NoFleet;
	}
	return ECosmicGuardBlock::None;
}

float CosmicConflict::SiegeOddsOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules, int32 Attacker, int32 SystemIndex)
{
	if (!State.Civs.IsValidIndex(Attacker) || !State.Systems.IsValidIndex(SystemIndex))
	{
		return 0.f;
	}
	const FStarSystemState& System = State.Systems[SystemIndex];
	const int32 OwnerIndex = Index.FindCiv(State, System.OwnerCivID);
	if (!State.Civs.IsValidIndex(OwnerIndex) || OwnerIndex == Attacker)
	{
		return 0.f;
	}
	const FCivilizationState& Owner = State.Civs[OwnerIndex];
	const float A = EffectivePowerOf(State.Civs[Attacker].FleetPower, CosmicConflictImpl::TierGap(Owner.Tier, State.Civs[Attacker].Tier), Rules);
	if (!(A > 0.f))
	{
		return 0.f;
	}
	const int32 ShareAt = (Owner.FleetTargetSystemID == System.SystemID) ? 0 : Owner.FleetTargetSystemID.IsNone() ? 1 : 2;
	const float Share = Rules.SiegeDefenceShares.IsValidIndex(ShareAt) ? CosmicConflictImpl::FactorOr(Rules.SiegeDefenceShares[ShareAt], 0.f) : 0.f;
	const float Power = CosmicConflictImpl::FactorOr(Owner.FleetPower, 0.f);
	const float D = Power * Share;
	if (!(D > 0.f))
	{
		return 1.f;
	}
	return A / (A + D);
}

bool CosmicConflict::Takes(float Number, float Odds)
{
	return Number < Odds;
}

int64 CosmicConflict::SiegeEndTickOf(const FCivilizationState& Civ, const FCosmicRulesDef& Rules)
{
	return Civ.FleetTargetSinceTick + static_cast<int64>(FMath::Max(1, Rules.SiegeTicks));
}

bool CosmicConflict::IsSiegeDue(const FCivilizationState& Civ, int64 Tick, const FCosmicRulesDef& Rules)
{
	return Tick >= SiegeEndTickOf(Civ, Rules);
}

int32 CosmicConflict::OddsBandOf(float OursOdds)
{
	if (OursOdds >= 0.6f)
	{
		return 2;
	}
	return (OursOdds <= 0.4f) ? 0 : 1;
}

int32 CosmicConflict::FleetDelayOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
	const FCosmicRulesDef& Rules, int32 SystemIndex, const FHazardDef& Row)
{
	if (!State.Systems.IsValidIndex(SystemIndex) || Row.SpawnRule == ECosmicHazardSpawn::Capital)
	{
		return 0;
	}
	const FStarSystemState& System = State.Systems[SystemIndex];
	const int32 OwnerIndex = Index.FindCiv(State, System.OwnerCivID);
	if (!State.Civs.IsValidIndex(OwnerIndex) || State.Civs[OwnerIndex].FleetTargetSystemID != System.SystemID)
	{
		return 0;
	}
	const FCivilizationState& Owner = State.Civs[OwnerIndex];
	if (!(Owner.FleetPower > 0.f))
	{
		return 0;
	}
	// The gap to the hazard's source: a claim's is its claimant's tier, every other hazard's is 0 (header section 2.3).
	int32 Gap = 0;
	if (Row.SpawnRule == ECosmicHazardSpawn::ClaimRule)
	{
		const int32 Claimant = CosmicHazards::ClaimantOf(State, Index, Tables, Rules, SystemIndex);
		const FTranscendentDef* Def = State.Transcendents.IsValidIndex(Claimant) ? Tables.FindTranscendent(State.Transcendents[Claimant].TranscendentID) : nullptr;
		Gap = Def ? CosmicConflictImpl::TierGap(Def->Tier, Owner.Tier) : 0;
	}
	const float PerPower = CosmicConflictImpl::FactorOr(Rules.FleetDelayPerPower, 0.f);
	const float Cap = FMath::Min(CosmicConflictImpl::FactorOr(Rules.FleetDelayCap, 0.f), 100000.f);
	const float Delay = PerPower * EffectivePowerOf(Owner.FleetPower, Gap, Rules);
	if (!(Delay > 0.f))
	{
		return 0;
	}
	return static_cast<int32>(Delay < Cap ? Delay : Cap);
}

void CosmicConflict::LiftTargetsOn(FGalaxyState& State, FName SystemID)
{
	if (SystemID.IsNone())
	{
		return;
	}
	for (FCivilizationState& Civ : State.Civs)
	{
		if (Civ.FleetTargetSystemID == SystemID)
		{
			Civ.FleetTargetSystemID = NAME_None;
			Civ.FleetTargetSinceTick = State.Tick;
		}
	}
}
