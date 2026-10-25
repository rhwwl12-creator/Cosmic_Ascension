// Cosmic Ascension - Sim layer (Core Logic).

#include "Sim/CosmicEdicts.h"

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

float CosmicEdicts::SpawnMultiplierOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables, FGameplayTag Tag)
{
	float Product = 1.f;
	for (const FName Row : Civ.ActiveEdicts)
	{
		const FEdictDef* Def = Tables.FindEdict(Row);
		const float* Factor = Def ? Def->HazardSpawnMultipliers.Find(Tag) : nullptr;
		if (Factor)
		{
			Product *= (*Factor >= 0.f && *Factor <= TNumericLimits<float>::Max()) ? *Factor : 0.f;
		}
	}
	return Product;
}

int32 CosmicEdicts::CountdownBonusOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables, FGameplayTag Tag)
{
	int64 Sum = 0;
	for (const FName Row : Civ.ActiveEdicts)
	{
		const FEdictDef* Def = Tables.FindEdict(Row);
		const int32* Bonus = Def ? Def->HazardCountdownBonus.Find(Tag) : nullptr;
		if (Bonus && *Bonus > 0)
		{
			Sum += *Bonus;
		}
	}
	return static_cast<int32>(FMath::Clamp<int64>(Sum, 0, 100000));
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
		if (!Def)
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
