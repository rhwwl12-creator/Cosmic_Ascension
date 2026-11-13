// Cosmic Ascension - Sim layer (Core Logic).

#include "Sim/CosmicExistenceBricks.h"

#include "Core/CosmicIDs.h"
#include "Core/CosmicSeed.h"
#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "GameplayTagContainer.h"
#include "Math/NumericLimits.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"
#include "UObject/NameTypes.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicBricksImpl
{
	// A flat knob (an amount a tick): above 0 and finite, else 0 - a compare, so a value that is not a number gives nothing.
	float FlatOf(float Value)
	{
		return (Value > 0.f && Value <= TNumericLimits<float>::Max()) ? Value : 0.f;
	}

	// A multiplier the schema keeps above 0 (a progress, a share): above 0 and finite, else 1.
	float PositiveFactorOf(float Value)
	{
		return (Value > 0.f && Value <= TNumericLimits<float>::Max()) ? Value : 1.f;
	}

	// A multiplier the schema keeps at 0 or above (the star eater's Energy): 0 or above and finite, else 1.
	float FactorOf(float Value)
	{
		return (Value >= 0.f && Value <= TNumericLimits<float>::Max()) ? Value : 1.f;
	}

	// A term of a drift target: in [-1, 1], else 0 (a compare, so a value that is not a number gives nothing).
	float TargetTermOf(float Value)
	{
		return (Value >= -1.f && Value <= 1.f) ? Value : 0.f;
	}

	// One more tick of a cumulative vector, never past MAX_int32.
	int32 Bump(int32 Value)
	{
		return Value < MAX_int32 ? Value + 1 : Value;
	}

	// The index of an entry of the people's OwnedSystems when the people owns it (settled or being settled); INDEX_NONE for an
	// entry whose owner is another people, as in the economy stage.
	int32 OwnedIndexOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCivilizationState& Civ, FName SystemID)
	{
		const int32 SystemIndex = Index.FindSystem(State, SystemID);
		return (State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == Civ.CivID) ? SystemIndex : INDEX_NONE;
	}

	// The settled systems the people owns, in OwnedSystems order.
	TArray<int32, TInlineAllocator<16>> SettledOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCivilizationState& Civ)
	{
		TArray<int32, TInlineAllocator<16>> Settled;
		for (const FName SystemID : Civ.OwnedSystems)
		{
			const int32 SystemIndex = OwnedIndexOf(State, Index, Civ, SystemID);
			if (SystemIndex != INDEX_NONE && CosmicState::IsSettled(State.Systems[SystemIndex]))
			{
				Settled.Add(SystemIndex);
			}
		}
		return Settled;
	}

	// The words of the six vectors, the Key of CivTimeVectorAtLeast (schema words.time_vectors), in the fixed order.
	const FName BuiltWord(TEXT("Built"));
	const FName ConsumedWord(TEXT("Consumed"));
	const FName DriftedWord(TEXT("Drifted"));
	const FName LinkedWord(TEXT("Linked"));
	const FName IsolatedWord(TEXT("Isolated"));
	const FName EnduredWord(TEXT("Endured"));
}

CosmicBricks::FCosmicTimeFacts CosmicBricks::TimeFactsOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules, int32 CivIndex)
{
	using namespace CosmicBricksImpl;

	FCosmicTimeFacts Facts;
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return Facts;
	}
	const FCivilizationState& Civ = State.Civs[CivIndex];

	// One walk over the systems it owns: the two project slots, the settlement, the settled count and the standing hazards.
	int32 Settled = 0;
	for (const FName SystemID : Civ.OwnedSystems)
	{
		const int32 SystemIndex = OwnedIndexOf(State, Index, Civ, SystemID);
		if (SystemIndex == INDEX_NONE)
		{
			continue;
		}
		const FStarSystemState& System = State.Systems[SystemIndex];
		const bool bSettling = System.ActiveMegaproject == CosmicRows::ProjectSettle;
		Facts.bDrifted |= bSettling;
		Facts.bBuilt |= (!System.ActiveMegaproject.IsNone() && !bSettling) || !System.ResponseMegaproject.IsNone();
		if (CosmicState::IsSettled(System))
		{
			++Settled;
			Facts.bEndured |= System.ActiveHazard.IsValid();
		}
	}

	// The fleet aimed at a system of another people, or the starting edict held.
	bool bAimedAway = false;
	if (!Civ.FleetTargetSystemID.IsNone())
	{
		const int32 Target = Index.FindSystem(State, Civ.FleetTargetSystemID);
		bAimedAway = State.Systems.IsValidIndex(Target) && !State.Systems[Target].OwnerCivID.IsNone() && State.Systems[Target].OwnerCivID != Civ.CivID;
	}
	Facts.bConsumed = bAimedAway || (!Rules.StartingEdict.IsNone() && Civ.ActiveEdicts.Contains(Rules.StartingEdict));

	// The ties: an any-question, so the order of the map does not matter.
	const FGameplayTag Friendly = CosmicTags::StanceFriendly();
	bool bBond = false;
	bool bStanding = false;
	for (const TPair<FName, FCosmicTie>& Pair : Civ.Ties)
	{
		const bool bTreaty = Pair.Value.Treaties.Num() > 0;
		bBond |= bTreaty || Pair.Value.Stance == Friendly;
		bStanding |= bTreaty || Pair.Value.Stance.IsValid();
	}
	Facts.bLinked = bBond;
	Facts.bIsolated = Settled == 1 && !bStanding;
	Facts.bEndured |= Civ.PressureEnduredTicks > 0 || Civ.LawsHeldTicks > 0;
	return Facts;
}

void CosmicBricks::CountTimeVectors(FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules)
{
	using namespace CosmicBricksImpl;

	for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
	{
		const FCosmicTimeFacts Facts = TimeFactsOf(State, Index, Rules, CivIndex);
		FCosmicTimeVectors& Vectors = State.Civs[CivIndex].TimeVectors;
		Vectors.Built = Facts.bBuilt ? Bump(Vectors.Built) : Vectors.Built;
		Vectors.Consumed = Facts.bConsumed ? Bump(Vectors.Consumed) : Vectors.Consumed;
		Vectors.Drifted = Facts.bDrifted ? Bump(Vectors.Drifted) : Vectors.Drifted;
		Vectors.Linked = Facts.bLinked ? Bump(Vectors.Linked) : Vectors.Linked;
		Vectors.Isolated = Facts.bIsolated ? Bump(Vectors.Isolated) : Vectors.Isolated;
		Vectors.Endured = Facts.bEndured ? Bump(Vectors.Endured) : Vectors.Endured;
	}
}

bool CosmicBricks::VectorOf(const FCosmicTimeVectors& Vectors, FName Word, int32& OutTicks)
{
	using namespace CosmicBricksImpl;

	OutTicks = 0;
	if (Word.IsNone())
	{
		return false;
	}
	const FName Words[] = { BuiltWord, ConsumedWord, DriftedWord, LinkedWord, IsolatedWord, EnduredWord };
	const int32 Ticks[] = { Vectors.Built, Vectors.Consumed, Vectors.Drifted, Vectors.Linked, Vectors.Isolated, Vectors.Endured };
	static_assert(UE_ARRAY_COUNT(Words) == UE_ARRAY_COUNT(Ticks), "One word per vector");
	for (int32 At = 0; At < static_cast<int32>(UE_ARRAY_COUNT(Words)); ++At)
	{
		// The schema holds the case of the word; the evaluator, like every Key, compares names.
		if (Word == Words[At])
		{
			OutTicks = Ticks[At];
			return true;
		}
	}
	return false;
}

int32 CosmicBricks::SpanOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex)
{
	using namespace CosmicBricksImpl;

	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return 0;
	}
	const TArray<int32, TInlineAllocator<16>> Settled = SettledOf(State, Index, State.Civs[CivIndex]);
	int32 Widest = 0;
	// [B-7x5r] (B7x header section 5 judgment 4): one pair of buffers for every walk, and each walk stops once the later
	// settled systems have their distances - the same distances, so the same answer, bit for bit.
	TArray<int32> Distances;
	TArray<int32> Queue;
	for (int32 From = 0; From + 1 < Settled.Num(); ++From)
	{
		const TConstArrayView<int32> Later(Settled.GetData() + From + 1, Settled.Num() - From - 1);
		CosmicState::LaneDistancesInto(State, Index, Settled[From], Distances, Queue, Later);
		for (const int32 To : Later)
		{
			const int32 Lanes = Distances.IsValidIndex(To) ? Distances[To] : INDEX_NONE;
			Widest = FMath::Max(Widest, Lanes);
		}
	}
	return Widest;
}

bool CosmicBricks::IsIdleOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex)
{
	using namespace CosmicBricksImpl;

	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return false;
	}
	const FCivilizationState& Civ = State.Civs[CivIndex];
	const TArray<int32, TInlineAllocator<16>> Settled = SettledOf(State, Index, Civ);
	for (const int32 SystemIndex : Settled)
	{
		const FStarSystemState& System = State.Systems[SystemIndex];
		if (System.ActiveHazard.IsValid() || !System.ActiveMegaproject.IsNone() || !System.ResponseMegaproject.IsNone())
		{
			return false;
		}
	}
	// A siege by the people.
	if (!Civ.FleetTargetSystemID.IsNone())
	{
		const int32 Target = Index.FindSystem(State, Civ.FleetTargetSystemID);
		if (State.Systems.IsValidIndex(Target) && !State.Systems[Target].OwnerCivID.IsNone() && State.Systems[Target].OwnerCivID != Civ.CivID)
		{
			return false;
		}
	}
	// A siege on the people: another people's fleet aimed at one of its settled systems.
	for (int32 Other = 0; Other < State.Civs.Num(); ++Other)
	{
		const FName Aim = State.Civs[Other].FleetTargetSystemID;
		if (Other == CivIndex || Aim.IsNone())
		{
			continue;
		}
		const int32 Target = Index.FindSystem(State, Aim);
		if (Settled.Contains(Target))
		{
			return false;
		}
	}
	return true;
}

bool CosmicBricks::IsSparedHazard(const FGameplayTag& Hazard)
{
	return Hazard == CosmicTags::HazardClaim() || Hazard == CosmicTags::HazardExtinction() || Hazard == CosmicTags::HazardPreSupernova();
}

int32 CosmicBricks::ParasiteHazardsOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex)
{
	using namespace CosmicBricksImpl;

	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return 0;
	}
	TArray<bool> Borders;
	CosmicDiplomacy::BordersOf(State, Index, CivIndex, Borders);
	int32 Count = 0;
	for (int32 Other = 0; Other < Borders.Num() && Other < State.Civs.Num(); ++Other)
	{
		if (!Borders[Other])
		{
			continue;
		}
		for (const int32 SystemIndex : SettledOf(State, Index, State.Civs[Other]))
		{
			const FGameplayTag Hazard = State.Systems[SystemIndex].ActiveHazard;
			Count += (Hazard.IsValid() && !IsSparedHazard(Hazard)) ? 1 : 0;
		}
	}
	return Count;
}

void CosmicBricks::AddIncomeShares(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules, int32 CivIndex, FResourcePool& Income)
{
	using namespace CosmicBricksImpl;

	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return;
	}
	const FGameplayTagContainer& Traits = State.Civs[CivIndex].Traits;
	if (Traits.Num() == 0)
	{
		return;
	}
	// Energy: the star eater's factor on the income of its stars first, then the flat shares.
	if (Traits.HasTagExact(CosmicTags::BrickDietStarEater()))
	{
		const float Factor = FactorOf(Rules.BrickStarEaterEnergyMultiplier);
		if (Factor != 1.f)
		{
			Income.Energy = Income.Energy * Factor;
		}
	}
	if (Traits.HasTagExact(CosmicTags::BrickDietDarkMatter()))
	{
		const float Flat = FlatOf(Rules.BrickDarkMatterEnergyPerTick);
		if (Flat > 0.f)
		{
			Income.Energy += Flat;
		}
	}
	if (Traits.HasTagExact(CosmicTags::BrickDietDistanceTension()))
	{
		const float PerLane = FlatOf(Rules.BrickSpanEnergyPerLane);
		const int32 Span = PerLane > 0.f ? SpanOf(State, Index, CivIndex) : 0;
		if (Span > 0)
		{
			Income.Energy += PerLane * static_cast<float>(Span);
		}
	}
	// Insight: the idle tick, then the neighbours' crises.
	if (Traits.HasTagExact(CosmicTags::BrickDietIdleFeed()))
	{
		const float Idle = FlatOf(Rules.BrickIdleInsightPerTick);
		if (Idle > 0.f && IsIdleOf(State, Index, CivIndex))
		{
			Income.Insight += Idle;
		}
	}
	if (Traits.HasTagExact(CosmicTags::BrickDietCrisisParasite()))
	{
		const float PerHazard = FlatOf(Rules.BrickParasiteInsightPerHazard);
		const int32 Hazards = PerHazard > 0.f ? ParasiteHazardsOf(State, Index, CivIndex) : 0;
		if (Hazards > 0)
		{
			Income.Insight += PerHazard * static_cast<float>(Hazards);
		}
	}
}

int32 CosmicBricks::ParasiteBonusBoundOf(const FCosmicRulesDef& Rules, const FGameplayTag& Hazard)
{
	if (!Hazard.IsValid() || IsSparedHazard(Hazard))
	{
		return 0;
	}
	return FMath::Clamp(Rules.BrickParasiteCountdownBonusTicks, 0, 100000);
}

int32 CosmicBricks::ParasiteCountdownBonusOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules, int32 SystemIndex, const FGameplayTag& Hazard)
{
	// The bound is the bonus itself: once a hazard, whatever the number of parasites that border the owner.
	const int32 Bonus = ParasiteBonusBoundOf(Rules, Hazard);
	if (Bonus <= 0 || !State.Systems.IsValidIndex(SystemIndex))
	{
		return 0;
	}
	const int32 OwnerIndex = Index.FindCiv(State, State.Systems[SystemIndex].OwnerCivID);
	if (!State.Civs.IsValidIndex(OwnerIndex))
	{
		return 0;
	}
	TArray<bool> Borders;
	CosmicDiplomacy::BordersOf(State, Index, OwnerIndex, Borders);
	const FGameplayTag Parasite = CosmicTags::BrickDietCrisisParasite();
	for (int32 Other = 0; Other < Borders.Num() && Other < State.Civs.Num(); ++Other)
	{
		if (Borders[Other] && State.Civs[Other].Traits.HasTagExact(Parasite))
		{
			return Bonus;
		}
	}
	return 0;
}

float CosmicBricks::ResearchMultiplierOf(const FCivilizationState& Civ, const FCosmicRulesDef& Rules)
{
	using namespace CosmicBricksImpl;

	float Factor = 1.f;
	if (Civ.Traits.HasTagExact(CosmicTags::BrickInstinctSwarmTech()))
	{
		Factor *= PositiveFactorOf(Rules.BrickSwarmResearchMultiplier);
	}
	if (IsInward(Civ))
	{
		Factor *= PositiveFactorOf(Rules.BrickInwardResearchMultiplier);
	}
	return Factor;
}

float CosmicBricks::SettleProgressOf(const FCivilizationState& Civ, const FCosmicRulesDef& Rules, float Step)
{
	using namespace CosmicBricksImpl;

	if (!Civ.Traits.HasTagExact(CosmicTags::BrickSpaceNomad()))
	{
		return Step;
	}
	const float Factor = PositiveFactorOf(Rules.BrickNomadSettleProgressMultiplier);
	return Factor != 1.f ? Step * Factor : Step;
}

float CosmicBricks::HazardSpawnShareOf(const FCivilizationState& Owner, const FCosmicRulesDef& Rules, const FGameplayTag& Hazard, float Share)
{
	using namespace CosmicBricksImpl;

	float Out = Share;
	// The inward people's damping spares the hazards no brick touches (IsSparedHazard): the claim, the extinction and the
	// supernova precursor keep their shares.
	if (Owner.Traits.HasTagExact(CosmicTags::BrickSpaceFoldedSmall()) && !IsSparedHazard(Hazard))
	{
		const float Factor = PositiveFactorOf(Rules.BrickInwardHazardSpawnMultiplier);
		Out = Factor != 1.f ? Out * Factor : Out;
	}
	// The stellar crisis on the system of a people that lives off its star (once, whichever of the two bricks it carries).
	if (Hazard == CosmicTags::HazardPreSupernova()
		&& (Owner.Traits.HasTagExact(CosmicTags::BrickFormShutSphere()) || Owner.Traits.HasTagExact(CosmicTags::BrickDietStarEater())))
	{
		const float Factor = PositiveFactorOf(Rules.BrickStellarCrisisSpawnMultiplier);
		Out = Factor != 1.f ? Out * Factor : Out;
	}
	return Out;
}

float CosmicBricks::RelationTargetTermOf(const FCivilizationState& Seen, const FCosmicRulesDef& Rules)
{
	using namespace CosmicBricksImpl;

	float Term = 0.f;
	if (Seen.Traits.HasTagExact(CosmicTags::BrickDietStarEater()))
	{
		Term += TargetTermOf(Rules.BrickStarEaterRelationTarget);
	}
	if (Seen.Traits.HasTagExact(CosmicTags::BrickInstinctHiveMerge()))
	{
		Term += TargetTermOf(Rules.BrickMergeRelationTarget);
	}
	return Term;
}

bool CosmicBricks::IsInward(const FCivilizationState& Civ)
{
	return Civ.Traits.HasTagExact(CosmicTags::BrickSpaceClosedPort()) || Civ.Traits.HasTagExact(CosmicTags::BrickSpaceFoldedSmall());
}

FGameplayTag CosmicBricks::MutationDueOf(const FCivilizationState& Civ, const FCosmicRulesDef& Rules)
{
	const int32 Threshold = Rules.BrickMutationThresholdTicks;
	if (Threshold <= 0)
	{
		return FGameplayTag();
	}
	const FGameplayTagContainer& Traits = Civ.Traits;
	const FCosmicTimeVectors& Vectors = Civ.TimeVectors;
	if (!Traits.HasTagExact(CosmicTags::BrickMutationStarTow()) && Traits.HasTagExact(CosmicTags::BrickSpaceNomad()) && Vectors.Drifted >= Threshold)
	{
		return CosmicTags::BrickMutationStarTow();
	}
	if (!Traits.HasTagExact(CosmicTags::BrickMutationDustBand()) && Traits.HasTagExact(CosmicTags::BrickInstinctSwarmTech())
		&& Vectors.Built >= Threshold && Vectors.Linked >= Threshold)
	{
		return CosmicTags::BrickMutationDustBand();
	}
	if (!Traits.HasTagExact(CosmicTags::BrickMutationInwardHunger()) && Traits.HasTagExact(CosmicTags::BrickDietStarEater()) && Vectors.Consumed >= Threshold)
	{
		return CosmicTags::BrickMutationInwardHunger();
	}
	return FGameplayTag();
}

bool CosmicBricks::MutationStepDueAt(const FCosmicRulesDef& Rules, int64 Tick)
{
	const int64 Period = Rules.BrickEvolutionPeriodTicks;
	return Period > 0 && Tick > 0 && Tick % Period == 0;
}

int32 CosmicBricks::MutateAIPeoples(FGalaxyState& State, const FCosmicRulesDef& Rules)
{
	int32 Mutated = 0;
	for (FCivilizationState& Civ : State.Civs)
	{
		if (Civ.bIsPlayerRuled)
		{
			continue;
		}
		const FGameplayTag Due = MutationDueOf(Civ, Rules);
		if (Due.IsValid())
		{
			// As the effect AddTrait adds a tag (UCosmicEventSubsystem::Apply).
			Civ.Traits.AddTag(Due);
			++Mutated;
		}
	}
	return Mutated;
}

FGameplayTag CosmicBricks::SeedBrickOf(const FCosmicRulesDef& Rules, int32 Seed, const FCivilizationState& Civ)
{
	if (Rules.BrickSeedTags.Num() == 0)
	{
		return FGameplayTag();
	}
	for (const FGameplayTag& Held : Civ.Traits)
	{
		if (CosmicTags::IsBrick(Held))
		{
			return FGameplayTag();
		}
	}
	TArray<FGameplayTag> Seeds;
	for (const FGameplayTag& Listed : Rules.BrickSeedTags)
	{
		if (CosmicTags::IsBrick(Listed))
		{
			Seeds.Add(Listed);
		}
	}
	if (Seeds.IsEmpty())
	{
		return FGameplayTag();
	}
	Seeds.Sort([](const FGameplayTag& A, const FGameplayTag& B) { return FNameLexicalLess()(A.GetTagName(), B.GetTagName()); });
	const uint32 Pick = CosmicSeed::DeriveForName(Seed, Civ.CivID, CosmicSeed::Purpose::Brick, 0) % static_cast<uint32>(Seeds.Num());
	return Seeds[static_cast<int32>(Pick)];
}

FGameplayTag CosmicBricks::DimmerStarClassOf(const FGameplayTag& StarClass)
{
	if (!StarClass.IsValid())
	{
		return StarClass;
	}
	for (int32 At = 0; At < CosmicTags::StarClassCount; ++At)
	{
		if (CosmicTags::StarClassAt(At) == StarClass)
		{
			return At + 1 < CosmicTags::StarClassCount ? CosmicTags::StarClassAt(At + 1) : StarClass;
		}
	}
	return StarClass;
}

bool CosmicBricks::StarDimDueOf(const FCivilizationState& Civ, const FCosmicRulesDef& Rules, int64 Tick)
{
	const int64 Period = Rules.BrickStarEaterDimPeriodTicks;
	return Period > 0 && Tick > 0 && Tick % Period == 0 && Civ.Traits.HasTagExact(CosmicTags::BrickDietStarEater());
}

bool CosmicBricks::DimCapitalStar(FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex)
{
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return false;
	}
	const FCivilizationState& Civ = State.Civs[CivIndex];
	const int32 Capital = Index.FindSystem(State, Civ.HomeSystemID);
	if (!State.Systems.IsValidIndex(Capital))
	{
		return false;
	}
	FStarSystemState& System = State.Systems[Capital];
	if (System.OwnerCivID != Civ.CivID || !CosmicState::IsSettled(System) || CosmicHazards::IsDeadNode(System) || CosmicState::IsSanctuary(System.SystemID))
	{
		return false;
	}
	const FGameplayTag Dimmer = DimmerStarClassOf(System.StarClass);
	if (Dimmer == System.StarClass)
	{
		return false;
	}
	System.StarClass = Dimmer;
	return true;
}

CosmicBricks::ECosmicShape CosmicBricks::ShapeOf(const FCivilizationState& Civ)
{
	const FGameplayTagContainer& Traits = Civ.Traits;
	if (Traits.HasTagExact(CosmicTags::BrickFormKeptLaw()))
	{
		return ECosmicShape::KeptLaw;
	}
	if (Traits.HasTagExact(CosmicTags::BrickSpaceFoldedSmall()))
	{
		return ECosmicShape::FoldedSmall;
	}
	if (Traits.HasTagExact(CosmicTags::BrickSpaceClosedPort()) || Traits.HasTagExact(CosmicTags::BrickFormShutSphere()))
	{
		return ECosmicShape::ShutHouse;
	}
	if (Traits.HasTagExact(CosmicTags::BrickFormLivingNebula()) || Traits.HasTagExact(CosmicTags::BrickDietDarkMatter()))
	{
		return ECosmicShape::SpreadBody;
	}
	if (Traits.HasTagExact(CosmicTags::BrickSpaceNomad()))
	{
		return ECosmicShape::StarHerd;
	}
	if (Traits.HasTagExact(CosmicTags::BrickInstinctSwarmTech()))
	{
		return ECosmicShape::WovenSwarm;
	}
	if (Traits.HasTagExact(CosmicTags::BrickDietStarEater()))
	{
		return ECosmicShape::LastHunger;
	}
	if (Traits.HasTagExact(CosmicTags::BrickInstinctHiveMerge()))
	{
		return ECosmicShape::Entwined;
	}
	// The largest vector, the first in the fixed order on a tie (a strict compare keeps the earlier one).
	const FCosmicTimeVectors& Vectors = Civ.TimeVectors;
	const int32 Ticks[] = { Vectors.Built, Vectors.Consumed, Vectors.Drifted, Vectors.Linked, Vectors.Isolated, Vectors.Endured };
	const ECosmicShape Shapes[] = { ECosmicShape::BuiltOrLinked, ECosmicShape::ConsumedOrDrifted, ECosmicShape::ConsumedOrDrifted,
		ECosmicShape::BuiltOrLinked, ECosmicShape::Isolated, ECosmicShape::Endured };
	static_assert(UE_ARRAY_COUNT(Ticks) == UE_ARRAY_COUNT(Shapes), "One shape per vector");
	int32 Largest = 0;
	for (int32 At = 1; At < static_cast<int32>(UE_ARRAY_COUNT(Ticks)); ++At)
	{
		if (Ticks[At] > Ticks[Largest])
		{
			Largest = At;
		}
	}
	return Shapes[Largest];
}
