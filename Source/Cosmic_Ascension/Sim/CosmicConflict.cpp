// Cosmic Ascension - Sim layer (Core Logic).

#include "Sim/CosmicConflict.h"

#include "Core/CosmicIDs.h"
#include "Core/CosmicTags.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "GameplayTagContainer.h"
#include "Math/NumericLimits.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"

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

	// Whether a rung's FleetPerSettledByTier value gives a fleet: above 0 and finite (FleetCapOf, FirstFleetRungOf).
	bool GivesFleet(float PerSettled)
	{
		return PerSettled > 0.f && PerSettled <= TNumericLimits<float>::Max();
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
	if (!CosmicConflictImpl::GivesFleet(PerSettled))
	{
		return 0.f;
	}
	return PerSettled * static_cast<float>(SettledCountOf(State, Index, CivIndex));
}

int32 CosmicConflict::FirstFleetRungOf(const FCosmicRulesDef& Rules, EAscensionTier From)
{
	for (int32 At = FMath::Max(0, static_cast<int32>(From)); At < Rules.FleetPerSettledByTier.Num(); ++At)
	{
		if (CosmicConflictImpl::GivesFleet(Rules.FleetPerSettledByTier[At]))
		{
			return At;
		}
	}
	return INDEX_NONE;
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

namespace CosmicConflictImpl
{
	// SiegeBlockOf's body. bLiftHeld opens Capital and LastSettled (AIAimOf's measurement - open question #126); every other
	// caller goes through SiegeBlockOf, which never lifts them.
	CosmicConflict::ECosmicSiegeBlock SiegeBlockWith(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules,
		int32 Attacker, int32 SystemIndex, bool bAssumeHostile, bool bLiftHeld);
}

CosmicConflict::ECosmicSiegeBlock CosmicConflict::SiegeBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules,
	int32 Attacker, int32 SystemIndex, bool bAssumeHostile)
{
	return CosmicConflictImpl::SiegeBlockWith(State, Index, Rules, Attacker, SystemIndex, bAssumeHostile, false);
}

CosmicConflict::ECosmicSiegeBlock CosmicConflictImpl::SiegeBlockWith(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules,
	int32 Attacker, int32 SystemIndex, bool bAssumeHostile, bool bLiftHeld)
{
	using namespace CosmicConflict;

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
	if (!bLiftHeld && Owner.HomeSystemID == System.SystemID)
	{
		return ECosmicSiegeBlock::Capital;
	}
	if (!bLiftHeld && CosmicHazards::IsLastSettledSystem(State, Index, SystemIndex))
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

CosmicConflict::ECosmicSiegeBlock CosmicConflict::SiegeNameOf(ECosmicGuardBlock Block)
{
	switch (Block)
	{
	case ECosmicGuardBlock::None: return ECosmicSiegeBlock::None;
	case ECosmicGuardBlock::Sanctuary: return ECosmicSiegeBlock::Sanctuary;
	case ECosmicGuardBlock::NoSystem: return ECosmicSiegeBlock::NoSystem;
	case ECosmicGuardBlock::Dead: return ECosmicSiegeBlock::Dead;
	case ECosmicGuardBlock::NotOwn: return ECosmicSiegeBlock::NoSystem;
	case ECosmicGuardBlock::NotSettled: return ECosmicSiegeBlock::NotSettled;
	case ECosmicGuardBlock::NoFleet: return ECosmicSiegeBlock::NoFleet;
	}
	return ECosmicSiegeBlock::NoSystem;
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
	// The row's own bound (B6b header 2.10): at the share's 0 it is FleetDelayCap, which GuardDelayTicksOf keeps already.
	const int32 Delay = GuardDelayTicksOf(Owner, Gap, Rules);
	const int32 RowCap = GuardDelayCapOf(Rules, Row.CountdownTicks);
	return Delay < RowCap ? Delay : RowCap;
}

int32 CosmicConflict::GuardDelayCapOf(const FCosmicRulesDef& Rules, int32 CountdownTicks)
{
	const float Cap = FMath::Min(CosmicConflictImpl::FactorOr(Rules.FleetDelayCap, 0.f), 100000.f);
	const float Share = Rules.FleetDelayCountdownShare;
	// A compare, so a share that is not a number is no share.
	if (!(Share > 0.f))
	{
		return static_cast<int32>(Cap);
	}
	const float ByCountdown = Share * static_cast<float>(CountdownTicks > 0 ? CountdownTicks : 0);
	return static_cast<int32>(ByCountdown < Cap ? ByCountdown : Cap);
}

int32 CosmicConflict::GuardDelayTicksOf(const FCivilizationState& Owner, int32 Gap, const FCosmicRulesDef& Rules)
{
	const float PerPower = CosmicConflictImpl::FactorOr(Rules.FleetDelayPerPower, 0.f);
	const float Cap = FMath::Min(CosmicConflictImpl::FactorOr(Rules.FleetDelayCap, 0.f), 100000.f);
	const float Delay = PerPower * EffectivePowerOf(Owner.FleetPower, Gap, Rules);
	if (!(Delay > 0.f))
	{
		return 0;
	}
	return static_cast<int32>(Delay < Cap ? Delay : Cap);
}

float CosmicConflict::RepulsedShareOf(const FCosmicRulesDef& Rules)
{
	return (Rules.SiegeRepulsedFleetFactor >= 0.f && Rules.SiegeRepulsedFleetFactor <= 1.f) ? Rules.SiegeRepulsedFleetFactor : 0.f;
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

CosmicConflict::ECosmicResistBlock CosmicConflict::ResistBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
	const FCosmicRulesDef& Rules, int32 CivIndex, int32 SystemIndex, const FGameplayTag& Tag)
{
	if (!State.Systems.IsValidIndex(SystemIndex) || !Tag.IsValid() || !State.Systems[SystemIndex].ActiveHazard.IsValid()
		|| !State.Systems[SystemIndex].ActiveHazard.MatchesTag(Tag))
	{
		return ECosmicResistBlock::NoHazard;
	}
	const FStarSystemState& System = State.Systems[SystemIndex];
	if (!State.Civs.IsValidIndex(CivIndex) || System.OwnerCivID != State.Civs[CivIndex].CivID)
	{
		return ECosmicResistBlock::NotOwn;
	}
	const FHazardDef* Row = Tables.FindHazardByTag(System.ActiveHazard);
	if (Row && Row->SpawnRule == ECosmicHazardSpawn::Capital)
	{
		return ECosmicResistBlock::Capital;
	}
	if (!(FleetCapOf(State, Index, CivIndex, Rules) > 0.f))
	{
		return ECosmicResistBlock::NoFleet;
	}
	if (!(State.Civs[CivIndex].FleetPower > 0.f))
	{
		return ECosmicResistBlock::Spent;
	}
	return ECosmicResistBlock::None;
}

float CosmicConflict::ResistShareOf(const FCosmicRulesDef& Rules)
{
	return (Rules.ResistFleetFactor >= 0.f && Rules.ResistFleetFactor <= 1.f) ? Rules.ResistFleetFactor : 1.f;
}

int64 CosmicConflict::ResistTicksOf(float Value)
{
	if (!(Value >= 1.f) || !(Value <= TNumericLimits<float>::Max()))
	{
		return 0;
	}
	return static_cast<int64>(Value < 2147483647.f ? Value : 2147483647.f);
}

CosmicConflict::FCosmicAIAim CosmicConflict::AIAimOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
	const FCosmicRulesDef& Rules, int32 CivIndex, bool bLiftHeld)
{
	FCosmicAIAim Aim;
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return Aim;
	}
	const FGameplayTag Hostile = CosmicTags::StanceHostile();
	const FCivilizationState& Civ = State.Civs[CivIndex];
	// A hostility opens only with a siege: no hostility held, no target, the rest over, a fleet, and not pacifist.
	if (!Civ.FleetTargetSystemID.IsNone() || State.Tick < Civ.FleetTargetSinceTick + static_cast<int64>(FMath::Max(0, Rules.SiegeRestTicks))
		|| !(Civ.FleetPower > 0.f) || !(Civ.Ethics.Pacifism < Rules.AIHostilePacifismMax))
	{
		return Aim;
	}
	for (const TPair<FName, FCosmicTie>& Pair : Civ.Ties)
	{
		if (Pair.Value.Stance == Hostile)
		{
			return Aim;
		}
	}
	// A people at its realm cap besieges nothing (SiegeBlockOf's RealmCap holds for every system): no scan.
	if (OwnedCountOf(State, Index, CivIndex) >= CosmicMegaprojects::RealmCapOf(Rules, Civ.Tier))
	{
		return Aim;
	}
	// Its borders once for the scan (Borders and ViewOf ask them a pair at a time): a bordering people's view is its
	// stored relation (CosmicDiplomacy::ViewOf).
	TArray<bool> Borders;
	CosmicDiplomacy::BordersOf(State, Index, CivIndex, Borders);
	// The people the ruled people's fleet besieges, if any.
	FName RuledBesieges;
	const int32 Ruled = State.Civs.IndexOfByPredicate([](const FCivilizationState& Candidate) { return Candidate.bIsPlayerRuled; });
	if (IsBesieging(State, Index, Ruled))
	{
		RuledBesieges = State.Systems[Index.FindSystem(State, State.Civs[Ruled].FleetTargetSystemID)].OwnerCivID;
	}
	for (int32 Other = 0; Other < State.Civs.Num(); ++Other)
	{
		if (Other == CivIndex)
		{
			continue;
		}
		const FCivilizationState& Them = State.Civs[Other];
		if (!(Civ.FleetPower >= (Them.bIsPlayerRuled ? Rules.AIHostilePowerRatioTowardRuled : Rules.AIHostilePowerRatio) * Them.FleetPower) || !Borders[Other]
			|| !(Civ.Relations.FindRef(Them.CivID) <= (Them.bIsPlayerRuled ? Rules.AIHostileRelationTowardRuled : Rules.AIHostileRelation))
			|| CosmicDiplomacy::HasTreaty(Civ, Them.CivID, CosmicTags::TreatyNonAggression()))
		{
			continue;
		}
		// No pile-on: another AI already hostile toward them, or the ruled people's siege on them, closes the gate.
		bool bTaken = false;
		for (int32 Third = 0; Third < State.Civs.Num() && !bTaken; ++Third)
		{
			bTaken = Third != CivIndex && Third != Other && !State.Civs[Third].bIsPlayerRuled
				&& CosmicDiplomacy::StanceOf(State.Civs[Third], Them.CivID) == Hostile;
		}
		if (bTaken || RuledBesieges == Them.CivID)
		{
			continue;
		}
		// A system of theirs to aim at, in Systems order, out of every higher sphere.
		for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num(); ++SystemIndex)
		{
			if (State.Systems[SystemIndex].OwnerCivID == Them.CivID
				&& CosmicConflictImpl::SiegeBlockWith(State, Index, Rules, CivIndex, SystemIndex, true, bLiftHeld) == ECosmicSiegeBlock::None
				&& !(UGalaxySimulationSubsystem::ComputeAttritionPressureIn(State, Index, Tables, Rules, CivIndex, SystemIndex).P > 0.f))
			{
				Aim.Other = Other;
				Aim.System = SystemIndex;
				return Aim;
			}
		}
	}
	return Aim;
}
