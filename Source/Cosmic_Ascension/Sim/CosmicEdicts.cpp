// Cosmic Ascension - Sim layer (Core Logic).

#include "Sim/CosmicEdicts.h"

#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Events/CosmicEventSubsystem.h"
#include "Math/NumericLimits.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicStateIndex.h"

namespace CosmicEdictsImpl
{
	// A member of a cost: above 0 and finite, else 0 (comparisons, no CRT call - section 9.2).
	float CostMember(float Value)
	{
		return (Value > 0.f && Value <= TNumericLimits<float>::Max()) ? Value : 0.f;
	}

	const FName SocietyField(TEXT("Society"));
}

int32 CosmicEdicts::SlotsOf(const FCivilizationState& Civ, const FCosmicRulesDef& Rules)
{
	const float Society = Civ.Research.FindRef(CosmicEdictsImpl::SocietyField);
	int32 Slots = 1;
	for (const float Mark : Rules.EdictSlotSocietyMarks)
	{
		Slots += (Society >= Mark) ? 1 : 0;
	}
	return Slots;
}

int32 CosmicEdicts::HeldCountOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables)
{
	int32 Count = 0;
	for (const FName Row : Civ.ActiveEdicts)
	{
		Count += Tables.FindEdict(Row) ? 1 : 0;
	}
	return Count;
}

CosmicEdicts::ECosmicEdictBlock CosmicEdicts::EnactBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
	const FCosmicRulesDef& Rules, int32 CivIndex, FName Row)
{
	const FEdictDef* Def = Row.IsNone() ? nullptr : Tables.FindEdict(Row);
	if (!Def || !State.Civs.IsValidIndex(CivIndex))
	{
		return ECosmicEdictBlock::NoRow;
	}
	const FCivilizationState& Civ = State.Civs[CivIndex];
	if (Civ.ActiveEdicts.Contains(Row))
	{
		return ECosmicEdictBlock::Held;
	}
	if (static_cast<uint8>(Civ.Tier) < static_cast<uint8>(Def->RequiredTier))
	{
		return ECosmicEdictBlock::Tier;
	}
	if (!Def->EnactConditions.IsEmpty())
	{
		FCosmicEvalView View(State, Index, Tables, Rules);
		View.CivIndex = CivIndex;
		View.SystemIndex = Index.FindSystem(State, Civ.HomeSystemID);
		View.GalaxyRow = Civ.GalaxyRow;
		View.SourceRow = Row;
		if (!UCosmicEventSubsystem::CheckAll(Def->EnactConditions, View))
		{
			return ECosmicEdictBlock::Conditions;
		}
	}
	if (HeldCountOf(Civ, Tables) >= SlotsOf(Civ, Rules))
	{
		return ECosmicEdictBlock::Slots;
	}
	FResourcePool Probe = Civ.Stockpile;
	if (!TryPay(Probe, Def->EnactCost))
	{
		return ECosmicEdictBlock::Cost;
	}
	return ECosmicEdictBlock::None;
}

FName CosmicEdicts::HeldAnswerOf(const FCivilizationState& Civ, const FHazardDef& Hazard, const FCosmicDataTables& Tables)
{
	for (const FName Answer : Hazard.RulerResolutions)
	{
		if (Civ.ActiveEdicts.Contains(Answer) && Tables.FindEdict(Answer))
		{
			return Answer;
		}
	}
	return NAME_None;
}

bool CosmicEdicts::AnswersStanding(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 CivIndex,
	FName Row)
{
	if (!State.Civs.IsValidIndex(CivIndex) || Row.IsNone())
	{
		return false;
	}
	const FCivilizationState& Civ = State.Civs[CivIndex];
	for (const FName SystemID : Civ.OwnedSystems)
	{
		const int32 SystemIndex = Index.FindSystem(State, SystemID);
		if (!State.Systems.IsValidIndex(SystemIndex))
		{
			continue;
		}
		const FStarSystemState& System = State.Systems[SystemIndex];
		if (System.OwnerCivID != Civ.CivID || !CosmicState::IsSettled(System) || !System.ActiveHazard.IsValid())
		{
			continue;
		}
		const FHazardDef* Hazard = Tables.FindHazardByTag(System.ActiveHazard);
		if (Hazard && Hazard->RulerResolutions.Contains(Row))
		{
			return true;
		}
	}
	return false;
}

bool CosmicEdicts::IsAnswerEdict(const FCosmicDataTables& Tables, FName Row)
{
	return !Row.IsNone() && Tables.FindEdict(Row) && Tables.IsHazardAnswer(Row);
}

TArray<const FEdictDef*, TInlineAllocator<8>> CosmicEdicts::HookEdictsOf(const FGalaxyState& State, const FCosmicStateIndex& Index,
	const FCosmicDataTables& Tables, int32 SystemIndex)
{
	TArray<const FEdictDef*, TInlineAllocator<8>> Hooks;
	if (!State.Systems.IsValidIndex(SystemIndex))
	{
		return Hooks;
	}
	const FStarSystemState& System = State.Systems[SystemIndex];
	const int32 OwnerIndex = Index.FindCiv(State, System.OwnerCivID);
	// The owner's held edicts first, both scopes, in ActiveEdicts order: without a galaxy law the list - and the order of the
	// product over it - is the one of B2.
	TArray<FName, TInlineAllocator<8>> Names;
	if (State.Civs.IsValidIndex(OwnerIndex))
	{
		for (const FName Row : State.Civs[OwnerIndex].ActiveEdicts)
		{
			if (const FEdictDef* Def = Tables.FindEdict(Row))
			{
				Names.Add(Row);
				Hooks.Add(Def);
			}
		}
	}
	// Then the galaxy laws the other peoples of the system's galaxy hold, in Civs order, each row once (B6b header section 2.4).
	for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
	{
		const FCivilizationState& Civ = State.Civs[CivIndex];
		if (CivIndex == OwnerIndex || Civ.ActiveEdicts.IsEmpty() || Civ.GalaxyRow != System.GalaxyRow)
		{
			continue;
		}
		for (const FName Row : Civ.ActiveEdicts)
		{
			const FEdictDef* Def = Tables.FindEdict(Row);
			if (Def && Def->Scope == ECosmicEdictScope::HomeGalaxy && !Names.Contains(Row))
			{
				Names.Add(Row);
				Hooks.Add(Def);
			}
		}
	}
	return Hooks;
}

float CosmicEdicts::SpawnMultiplierOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 SystemIndex,
	FGameplayTag Tag)
{
	float Product = 1.f;
	for (const FEdictDef* Def : HookEdictsOf(State, Index, Tables, SystemIndex))
	{
		const float* Factor = Def->HazardSpawnMultipliers.Find(Tag);
		if (Factor)
		{
			Product *= (*Factor >= 0.f && *Factor <= TNumericLimits<float>::Max()) ? *Factor : 0.f;
		}
	}
	return Product;
}

int32 CosmicEdicts::CountdownBonusOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 SystemIndex,
	FGameplayTag Tag)
{
	int64 Sum = 0;
	for (const FEdictDef* Def : HookEdictsOf(State, Index, Tables, SystemIndex))
	{
		const int32* Bonus = Def->HazardCountdownBonus.Find(Tag);
		if (Bonus && *Bonus > 0)
		{
			Sum += *Bonus;
		}
	}
	return static_cast<int32>(FMath::Clamp<int64>(Sum, 0, 100000));
}

bool CosmicEdicts::IsGalaxyLaw(const FCosmicDataTables& Tables, FName Row)
{
	const FEdictDef* Def = Row.IsNone() ? nullptr : Tables.FindEdict(Row);
	return Def && Def->Scope == ECosmicEdictScope::HomeGalaxy;
}

TArray<FName> CosmicEdicts::GalaxyLawRowsOf(const FCosmicDataTables& Tables)
{
	TArray<FName> Laws = Tables.GetEdictRowNames();
	Laws.RemoveAll([&Tables](FName Row) { return !IsGalaxyLaw(Tables, Row); });
	return Laws;
}

int32 CosmicEdicts::GalaxyLawCountOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables)
{
	int32 Count = 0;
	for (const FName Row : Civ.ActiveEdicts)
	{
		Count += IsGalaxyLaw(Tables, Row) ? 1 : 0;
	}
	return Count;
}

int32 CosmicEdicts::GalaxyLawsToHoldOf(const FCosmicRulesDef& Rules)
{
	return FMath::Max(1, Rules.GalaxyLawsToHold);
}

bool CosmicEdicts::HoldsGalaxyLawsOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules)
{
	return static_cast<uint8>(Civ.Tier) >= static_cast<uint8>(EAscensionTier::Tier4_Transcendent) && GalaxyLawCountOf(Civ, Tables) >= GalaxyLawsToHoldOf(Rules);
}

bool CosmicEdicts::AsksForLawsOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables)
{
	if (static_cast<uint8>(Civ.Tier) < static_cast<uint8>(EAscensionTier::Tier4_Transcendent))
	{
		return false;
	}
	for (const FName FeatRow : Tables.GetFeatRowNames())
	{
		const FAscensionFeatDef* Feat = Tables.FindFeat(FeatRow);
		if (Feat && Feat->IsJudgedRulerRowAt(Civ.Tier) && Feat->CarriesCondition(EConditionType::CivLawsHeldTicksAtLeast))
		{
			return true;
		}
	}
	return false;
}

float CosmicEdicts::PressureDrainMultiplierOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables)
{
	float Product = 1.f;
	for (const FName Row : Civ.ActiveEdicts)
	{
		const FEdictDef* Def = Tables.FindEdict(Row);
		if (Def && Def->PressureDrainMultiplier > 0.f && Def->PressureDrainMultiplier <= TNumericLimits<float>::Max())
		{
			Product *= Def->PressureDrainMultiplier;
		}
	}
	return Product;
}

bool CosmicEdicts::HasStabilityCost(const FEdictDef& Row)
{
	for (const FCosmicEffect& Effect : Row.EffectsPerTick)
	{
		if (Effect.Type == EEffectType::ChangeStability && Effect.Value < 0.f)
		{
			return true;
		}
	}
	return false;
}

float CosmicEdicts::StabilityDrainOf(const FEdictDef& Row)
{
	float Drain = 0.f;
	for (const FCosmicEffect& Effect : Row.EffectsPerTick)
	{
		if (Effect.Type == EEffectType::ChangeStability && Effect.Value >= -TNumericLimits<float>::Max() && Effect.Value <= TNumericLimits<float>::Max())
		{
			Drain -= Effect.Value;
		}
	}
	return Drain;
}

FResourcePool CosmicEdicts::CostOf(const FResourcePool& Cost)
{
	using namespace CosmicEdictsImpl;
	FResourcePool Read;
	Read.Energy = CostMember(Cost.Energy);
	Read.Matter = CostMember(Cost.Matter);
	Read.Insight = CostMember(Cost.Insight);
	Read.Influence = CostMember(Cost.Influence);
	Read.Causality = CostMember(Cost.Causality);
	return Read;
}

bool CosmicEdicts::TryPay(FResourcePool& Stockpile, const FResourcePool& Cost)
{
	const FResourcePool Read = CostOf(Cost);
	if (Stockpile.Energy < Read.Energy || Stockpile.Matter < Read.Matter || Stockpile.Insight < Read.Insight || Stockpile.Influence < Read.Influence
		|| Stockpile.Causality < Read.Causality)
	{
		return false;
	}
	Stockpile.Energy -= Read.Energy;
	Stockpile.Matter -= Read.Matter;
	Stockpile.Insight -= Read.Insight;
	Stockpile.Influence -= Read.Influence;
	Stockpile.Causality -= Read.Causality;
	return true;
}

void CosmicEdicts::Enact(FCivilizationState& Civ, FName Row, const FEdictDef& Def)
{
	TryPay(Civ.Stockpile, Def.EnactCost);
	Civ.ActiveEdicts.Add(Row);
}

bool CosmicEdicts::Revoke(FCivilizationState& Civ, FName Row)
{
	const int32 At = Civ.ActiveEdicts.Find(Row);
	if (At == INDEX_NONE)
	{
		return false;
	}
	Civ.ActiveEdicts.RemoveAt(At);
	return true;
}

TArray<FName> CosmicEdicts::AIRevokesOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules)
{
	TArray<FName> Revokes;
	for (const FName Row : Civ.ActiveEdicts)
	{
		const FEdictDef* Def = Tables.FindEdict(Row);
		// A galaxy law stays (B6b decision 11): the AI's law step enacts the laws its row asks for, and keeps them.
		if (!Def || Def->Scope == ECosmicEdictScope::HomeGalaxy)
		{
			continue;
		}
		if (!HasStabilityCost(*Def) || Civ.Stability < Rules.AIEdictRevokeStability)
		{
			Revokes.Add(Row);
		}
	}
	return Revokes;
}

bool CosmicEdicts::InheritedDrainOutrunsRecovery(const FCosmicRulesDef& Rules, const FCosmicDataTables& Tables)
{
	if (Rules.StartingEdict.IsNone())
	{
		return true;
	}
	const FEdictDef* Def = Tables.FindEdict(Rules.StartingEdict);
	const float Drain = Def ? StabilityDrainOf(*Def) : 0.f;
	if (!(Drain > 0.f))
	{
		return false;
	}
	const float Rate = (Rules.StabilityRecoveryPerTick > 0.f && Rules.StabilityRecoveryPerTick <= 1.f) ? Rules.StabilityRecoveryPerTick : 0.f;
	const float Bonus = (Rules.SocietyRecoveryBonus > 0.f && Rules.SocietyRecoveryBonus <= TNumericLimits<float>::Max()) ? Rules.SocietyRecoveryBonus : 0.f;
	return Rate * (1.f + Bonus) < Drain;
}
