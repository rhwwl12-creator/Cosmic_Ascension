// Cosmic Ascension - Events layer.

#include "Events/CosmicPetitions.h"

#include "Core/CosmicSeed.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Events/CosmicEventSubsystem.h"
#include "GameplayTagContainer.h"
#include "Internationalization/StringTableCoreFwd.h"
#include "Math/NumericLimits.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicStateIndex.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicPetitionsImpl
{
	// The most a check value is (the vessel's stats run 0..10, section 5.9).
	constexpr int32 MaxCheckValue = 10;

	// A choice's roll has six faces, 0..5 (section 5.9).
	constexpr int32 RollFaces = 6;

	// A Stability written as 0.7 is a little below it as a float: read with this much added, it is 7, not 6.
	constexpr float StabilityReadSlack = 0.0001f;

	struct FCandidate
	{
		FName Row;
		int32 SystemIndex = INDEX_NONE;
		float Weight = 0.f;
	};

	float WeightOf(const FEncounterDef& Row)
	{
		return (Row.BaseWeight > 0.f && Row.BaseWeight <= TNumericLimits<float>::Max()) ? Row.BaseWeight : 0.f;
	}

	float CostMember(float Value)
	{
		return (Value > 0.f && Value <= TNumericLimits<float>::Max()) ? Value : 0.f;
	}

	// The units of a row on the people CivIndex: a Civilization row once, on its capital (INDEX_NONE without a settled
	// capital); a SettledSystem row on each settled system of the people, in OwnedSystems order.
	TArray<int32, TInlineAllocator<8>> UnitsOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex, ECosmicEventSubject Subject)
	{
		TArray<int32, TInlineAllocator<8>> Units;
		const FCivilizationState& Civ = State.Civs[CivIndex];
		if (Subject == ECosmicEventSubject::Civilization)
		{
			Units.Add(CosmicPetitions::SubjectOf(State, Index, CivIndex, Subject, NAME_None));
			return Units;
		}
		for (const FName SystemID : Civ.OwnedSystems)
		{
			const int32 SystemIndex = CosmicPetitions::SubjectOf(State, Index, CivIndex, Subject, SystemID);
			if (SystemIndex != INDEX_NONE)
			{
				Units.Add(SystemIndex);
			}
		}
		return Units;
	}

	// The name of the people's open rebel record (section 7.2: Bank[NameIndex % Bank.Num()] of its archetype); empty
	// when it has none or its archetype has no name bank.
	FText RebelNameOf(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 CivIndex)
	{
		if (!State.Civs.IsValidIndex(CivIndex))
		{
			return FText::GetEmpty();
		}
		static const FGameplayTag RebelRole = FGameplayTag::RequestGameplayTag(FName(TEXT("Notable.Rebel")));
		const FCivilizationState& Civ = State.Civs[CivIndex];
		const FNotableRecord* Rebel = Civ.Notables.FindByPredicate([](const FNotableRecord& Record) { return Record.LastTick == 0 && Record.Role == RebelRole; });
		const FCivArchetypeDef* Archetype = Tables.FindCiv(Civ.ArchetypeRow);
		if (!Rebel || !Archetype || Archetype->PersonNameBank.IsEmpty() || Rebel->NameIndex < 0)
		{
			return FText::GetEmpty();
		}
		return Archetype->PersonNameBank[Rebel->NameIndex % Archetype->PersonNameBank.Num()];
	}
}

int32 CosmicPetitions::RuledCivIndexOf(const FGalaxyState& State, const FCosmicStateIndex& Index)
{
	if (State.Calling.LifeState != ELifeState::Embodied || State.Player.Role != EPlayerRole::Ruler || State.Player.RuledCivID.IsNone())
	{
		return INDEX_NONE;
	}
	const int32 CivIndex = Index.FindCiv(State, State.Player.RuledCivID);
	return State.Civs.IsValidIndex(CivIndex) ? CivIndex : INDEX_NONE;
}

int32 CosmicPetitions::SubjectOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex, ECosmicEventSubject Subject, FName SystemID)
{
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return INDEX_NONE;
	}
	const FCivilizationState& Civ = State.Civs[CivIndex];
	const FName Wanted = Subject == ECosmicEventSubject::Civilization ? Civ.HomeSystemID : SystemID;
	const int32 SystemIndex = Wanted.IsNone() ? INDEX_NONE : Index.FindSystem(State, Wanted);
	if (!State.Systems.IsValidIndex(SystemIndex) || State.Systems[SystemIndex].OwnerCivID != Civ.CivID || !CosmicState::IsSettled(State.Systems[SystemIndex]))
	{
		return INDEX_NONE;
	}
	return SystemIndex;
}

bool CosmicPetitions::Holds(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules,
	FName RowName, const FEncounterDef& Row, int32 CivIndex, int32 SystemIndex)
{
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return false;
	}
	FCosmicEvalView View(State, Index, Tables, Rules);
	View.CivIndex = CivIndex;
	View.SystemIndex = SystemIndex;
	View.GalaxyRow = State.Civs[CivIndex].GalaxyRow;
	View.SourceRow = RowName;
	return UCosmicEventSubsystem::CheckAll(Row.Conditions, View);
}

TArray<FCosmicPetitionUnit> CosmicPetitions::StandingHeld(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
	const FCosmicRulesDef& Rules)
{
	using namespace CosmicPetitionsImpl;
	TArray<FCosmicPetitionUnit> Held;
	const int32 CivIndex = RuledCivIndexOf(State, Index);
	if (CivIndex == INDEX_NONE || !State.Knowing.PendingPetitionRow.IsNone())
	{
		return Held;
	}
	for (const FName RowName : Tables.GetEncounterRowNames())
	{
		const FEncounterDef* Row = Tables.FindEncounter(RowName);
		if (!Row || Row->Trigger != ECosmicEventTrigger::Standing)
		{
			continue;
		}
		for (const int32 SystemIndex : UnitsOf(State, Index, CivIndex, Row->Subject))
		{
			if (Holds(State, Index, Tables, Rules, RowName, *Row, CivIndex, SystemIndex))
			{
				Held.Add({ RowName, SystemIndex });
			}
		}
	}
	return Held;
}

bool CosmicPetitions::Present(FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules,
	TConstArrayView<FCosmicPetitionUnit> HeldAtHead)
{
	using namespace CosmicPetitionsImpl;
	const int32 CivIndex = RuledCivIndexOf(State, Index);
	if (CivIndex == INDEX_NONE || !State.Knowing.PendingPetitionRow.IsNone())
	{
		return false;
	}

	// The Standing rows whose conditions came to hold in this tick: they hold now and did not at its head.
	TArray<FCandidate, TInlineAllocator<4>> Candidates;
	for (const FCosmicPetitionUnit& Unit : StandingHeld(State, Index, Tables, Rules))
	{
		if (!HeldAtHead.Contains(Unit))
		{
			Candidates.Add({ Unit.Row, Unit.SystemIndex, WeightOf(*Tables.FindEncounter(Unit.Row)) });
		}
	}

	// Without one, the periodic slot of the life: the Lottery rows that hold and were not among its last choices.
	if (Candidates.IsEmpty())
	{
		const int64 Period = Rules.PetitionPeriodTicks > 1 ? Rules.PetitionPeriodTicks : 1;
		const int64 Age = State.Tick - State.Calling.BornTick;
		if (Age <= 0 || Age % Period != 0)
		{
			return false;
		}
		const TArray<FName> Recent = State.Calling.Lives.IsEmpty() ? TArray<FName>() : State.Calling.Lives.Last().LastChoiceRows;
		// The most recent Lottery row among the last choices. When every Lottery row that holds is among them (a pool of two
		// or three periodic cards against the three remembered choices), only that one is passed over (MVP item 8): the same
		// periodic card never comes twice in a row, and the slot does not fall silent for the rest of the life.
		FName LastPeriodic = NAME_None;
		for (int32 At = Recent.Num() - 1; At >= 0; --At)
		{
			const FEncounterDef* Row = Tables.FindEncounter(Recent[At]);
			if (Row && Row->Trigger == ECosmicEventTrigger::Lottery)
			{
				LastPeriodic = Recent[At];
				break;
			}
		}
		for (int32 Pass = 0; Pass < 2 && Candidates.IsEmpty(); ++Pass)
		{
			for (const FName RowName : Tables.GetEncounterRowNames())
			{
				const FEncounterDef* Row = Tables.FindEncounter(RowName);
				const bool bPassedOver = Pass == 0 ? Recent.Contains(RowName) : RowName == LastPeriodic;
				if (!Row || Row->Trigger != ECosmicEventTrigger::Lottery || bPassedOver)
				{
					continue;
				}
				for (const int32 SystemIndex : UnitsOf(State, Index, CivIndex, Row->Subject))
				{
					if (Holds(State, Index, Tables, Rules, RowName, *Row, CivIndex, SystemIndex))
					{
						Candidates.Add({ RowName, SystemIndex, WeightOf(*Row) });
					}
				}
			}
		}
	}

	// One candidate by weight, picked with the derived integer of the tick: the cards are put before the player and
	// change nothing of the world, so no number of the sim stream is drawn (section 9.2).
	double Total = 0.0;
	for (const FCandidate& Candidate : Candidates)
	{
		Total += Candidate.Weight;
	}
	if (!(Total > 0.0))
	{
		return false;
	}
	const uint32 Derived = CosmicSeed::DeriveForName(State.Seed, State.Civs[CivIndex].CivID, CosmicSeed::Purpose::Petition, static_cast<uint32>(State.Tick));
	const double Pick = static_cast<double>(Derived) / 4294967296.0 * Total;
	double Reached = 0.0;
	for (const FCandidate& Candidate : Candidates)
	{
		Reached += Candidate.Weight;
		if (Candidate.Weight > 0.f && Pick < Reached)
		{
			const FEncounterDef* Row = Tables.FindEncounter(Candidate.Row);
			const bool bAboutPlace = Row && Row->Subject == ECosmicEventSubject::SettledSystem && State.Systems.IsValidIndex(Candidate.SystemIndex);
			State.Knowing.PendingPetitionRow = Candidate.Row;
			State.Knowing.PendingPetitionSystemID = bAboutPlace ? State.Systems[Candidate.SystemIndex].SystemID : FName();
			return true;
		}
	}
	return false;
}

bool CosmicPetitions::StillStands(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules)
{
	const int32 CivIndex = RuledCivIndexOf(State, Index);
	const FName RowName = State.Knowing.PendingPetitionRow;
	const FEncounterDef* Row = RowName.IsNone() ? nullptr : Tables.FindEncounter(RowName);
	if (CivIndex == INDEX_NONE || !Row)
	{
		return false;
	}
	const int32 SystemIndex = SubjectOf(State, Index, CivIndex, Row->Subject, State.Knowing.PendingPetitionSystemID);
	if (Row->Subject == ECosmicEventSubject::SettledSystem && SystemIndex == INDEX_NONE)
	{
		return false;
	}
	return Holds(State, Index, Tables, Rules, RowName, *Row, CivIndex, SystemIndex);
}

int32 CosmicPetitions::CheckValueOf(const FGalaxyState& State, int32 CivIndex, ECosmicCheckStat Stat)
{
	using namespace CosmicPetitionsImpl;
	int32 Value = 0;
	switch (Stat)
	{
	case ECosmicCheckStat::Body:
		Value = State.Player.Body;
		break;
	case ECosmicCheckStat::Mind:
		Value = State.Player.Mind;
		break;
	case ECosmicCheckStat::Causality:
		Value = State.Player.Causality;
		break;
	case ECosmicCheckStat::Exaltation:
		Value = State.Player.Exaltation;
		break;
	case ECosmicCheckStat::Stability:
		if (State.Civs.IsValidIndex(CivIndex))
		{
			// Stability is 0..1; a value outside (a slip) or not a number reads as the nearer bound, or 0.
			const float Stability = State.Civs[CivIndex].Stability;
			const float Held = Stability > 0.f ? (Stability < 1.f ? Stability : 1.f) : 0.f;
			Value = static_cast<int32>(Held * static_cast<float>(MaxCheckValue) + StabilityReadSlack);
		}
		break;
	case ECosmicCheckStat::NoCheck:
		break;
	}
	return FMath::Clamp(Value, 0, MaxCheckValue);
}

bool CosmicPetitions::Passes(const FGalaxyState& State, int32 CivIndex, const FEncounterChoice& Choice, int32 Roll)
{
	return Choice.CheckStat == ECosmicCheckStat::NoCheck || CheckValueOf(State, CivIndex, Choice.CheckStat) + Roll >= Choice.Difficulty;
}

int32 CosmicPetitions::PassingRollsOf(const FGalaxyState& State, int32 CivIndex, const FEncounterChoice& Choice)
{
	using namespace CosmicPetitionsImpl;
	if (Choice.CheckStat == ECosmicCheckStat::NoCheck)
	{
		return RollFaces;
	}
	return FMath::Clamp(CheckValueOf(State, CivIndex, Choice.CheckStat) + RollFaces - Choice.Difficulty, 0, RollFaces);
}

FResourcePool CosmicPetitions::CostOf(const FEncounterChoice& Choice)
{
	using namespace CosmicPetitionsImpl;
	FResourcePool Cost;
	Cost.Energy = CostMember(Choice.Cost.Energy);
	Cost.Matter = CostMember(Choice.Cost.Matter);
	Cost.Insight = CostMember(Choice.Cost.Insight);
	Cost.Influence = CostMember(Choice.Cost.Influence);
	Cost.Causality = CostMember(Choice.Cost.Causality);
	return Cost;
}

FFormatNamedArguments CosmicPetitions::ArgsOf(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 CivIndex, int32 SystemIndex)
{
	using namespace CosmicPetitionsImpl;
	const FCivilizationState* Civ = State.Civs.IsValidIndex(CivIndex) ? &State.Civs[CivIndex] : nullptr;
	const FStarSystemState* System = State.Systems.IsValidIndex(SystemIndex) ? &State.Systems[SystemIndex] : nullptr;
	FText HazardName;
	if (System)
	{
		if (const FHazardDef* Hazard = Tables.FindHazardByTag(System->ActiveHazard))
		{
			HazardName = Hazard->DisplayName;
		}
	}
	FText Rebel = RebelNameOf(State, Tables, CivIndex);
	if (Rebel.IsEmpty())
	{
		Rebel = FText::FromStringTable(CosmicLabels::TableId, TEXT("Role.Rebel"), EStringTableLoadingPolicy::FindOrFullyLoad);
	}
	FFormatNamedArguments Args;
	Args.Add(TEXT("Civ"), Civ ? Civ->CivName : FText::GetEmpty());
	Args.Add(TEXT("System"), System ? System->DisplayName : FText::GetEmpty());
	Args.Add(TEXT("Hazard"), HazardName);
	Args.Add(TEXT("Rebel"), Rebel);
	return Args;
}

void CosmicPetitions::Clear(FGalaxyState& State)
{
	State.Knowing.PendingPetitionRow = NAME_None;
	State.Knowing.PendingPetitionSystemID = NAME_None;
}

void CosmicPetitions::Remember(FGalaxyState& State, FName RowName)
{
	if (State.Calling.Lives.IsEmpty())
	{
		return;
	}
	TArray<FName>& Rows = State.Calling.Lives.Last().LastChoiceRows;
	Rows.Add(RowName);
	while (Rows.Num() > RememberedChoices)
	{
		Rows.RemoveAt(0);
	}
}
