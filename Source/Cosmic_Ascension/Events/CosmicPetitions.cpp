// Cosmic Ascension - Events layer.

#include "Events/CosmicPetitions.h"

#include "Core/CosmicSeed.h"
#include "Core/CosmicKnowing.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicRowNames.h"
#include "Events/CosmicEventSubsystem.h"
#include "GameplayTagContainer.h"
#include "Internationalization/StringTableCoreFwd.h"
#include "Math/NumericLimits.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicWanderer.h"

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

	// The most recent row of the life's last choices that is a wanderer's card (bWanderer) or a petition (!bWanderer) and,
	// with bLotteryOnly, a Lottery row; NAME_None when none is. A name no row holds counts as a petition (B1: each role
	// remembers its own rows in the one array).
	FName MostRecentOf(const FGalaxyState& State, const FCosmicDataTables& Tables, bool bWanderer, bool bLotteryOnly)
	{
		const TArray<FName>& Recent = State.Calling.Lives.IsEmpty() ? TArray<FName>() : State.Calling.Lives.Last().LastChoiceRows;
		for (int32 At = Recent.Num() - 1; At >= 0; --At)
		{
			const FEncounterDef* Row = Tables.FindEncounter(Recent[At]);
			const bool bOfWanderer = Row && CosmicPetitions::IsWandererCard(*Row);
			if (bOfWanderer == bWanderer && (!bLotteryOnly || (Row && Row->Trigger == ECosmicEventTrigger::Lottery)))
			{
				return Recent[At];
			}
		}
		return NAME_None;
	}

	// Picks one candidate by weight with Value in [0, 1): the first whose running weight passes Value * the total. Null
	// without a candidate of positive weight.
	const FCandidate* PickByWeight(TConstArrayView<FCandidate> Candidates, double Value)
	{
		double Total = 0.0;
		for (const FCandidate& Candidate : Candidates)
		{
			Total += Candidate.Weight;
		}
		if (!(Total > 0.0))
		{
			return nullptr;
		}
		const double Pick = Value * Total;
		double Reached = 0.0;
		for (const FCandidate& Candidate : Candidates)
		{
			Reached += Candidate.Weight;
			if (Candidate.Weight > 0.f && Pick < Reached)
			{
				return &Candidate;
			}
		}
		return nullptr;
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

	// The stance gate of one effect of a choice (B3): a SetStance on the card's other people (an empty Key) - None for any
	// other effect, for no other people, and when the stance is held already (Same).
	CosmicDiplomacy::ECosmicStanceBlock StanceBlockOfEffect(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules,
		int32 CivIndex, FName OtherCivID, const FCosmicEffect& Effect)
	{
		if (Effect.Type != EEffectType::SetStance || !(Effect.Key.IsNone() || Effect.Key.ToString().IsEmpty()) || OtherCivID.IsNone())
		{
			return CosmicDiplomacy::ECosmicStanceBlock::None;
		}
		const CosmicDiplomacy::ECosmicStanceBlock Block = CosmicDiplomacy::StanceBlockOf(State, Index, Rules, CivIndex, OtherCivID, Effect.Tag, true);
		return Block == CosmicDiplomacy::ECosmicStanceBlock::Same ? CosmicDiplomacy::ECosmicStanceBlock::None : Block;
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

FCosmicVesselSubject CosmicPetitions::VesselSubjectOf(const FGalaxyState& State, const FCosmicStateIndex& Index, bool bTravelAtOrigin)
{
	FCosmicVesselSubject Subject;
	Subject.SystemIndex = bTravelAtOrigin ? CosmicWanderer::PlaceIndexOf(State, Index) : CosmicWanderer::StandingIndexOf(State, Index);
	if (Subject.SystemIndex != INDEX_NONE)
	{
		const FName Owner = State.Systems[Subject.SystemIndex].OwnerCivID;
		const int32 CivIndex = Owner.IsNone() ? INDEX_NONE : Index.FindCiv(State, Owner);
		Subject.CivIndex = State.Civs.IsValidIndex(CivIndex) ? CivIndex : INDEX_NONE;
	}
	return Subject;
}

bool CosmicPetitions::WithinWandererGap(const FGalaxyState& State, int32 CivIndex)
{
	// The card's own threshold on the gap rule's count of rungs (not the gap rule's 'far above').
	return !State.Civs.IsValidIndex(CivIndex) || CosmicKnowing::RungsAbove(State.Civs[CivIndex].Tier, State.Calling.PersonalTier) <= 1;
}

bool CosmicPetitions::IsFirstTickOfReign(const FGalaxyState& State)
{
	if (State.Calling.LifeState != ELifeState::Embodied || State.Player.Role != EPlayerRole::Ruler || State.Calling.Lives.IsEmpty())
	{
		return false;
	}
	const FLifeRecord& Life = State.Calling.Lives.Last();
	return !Life.ReignTicks.IsEmpty() && !Life.ReignEndReasons.IsEmpty() && !Life.ReignEndReasons.Last().IsValid() && Life.ReignTicks.Last() == 0;
}

bool CosmicPetitions::IsFirstTickOfLife(const FGalaxyState& State)
{
	return State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Wanderer && State.Calling.Lives.Num() > 1
		&& State.Tick == State.Calling.BornTick;
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
	const bool bPeople = State.Civs.IsValidIndex(CivIndex);
	if (!bPeople && !(IsWandererCard(Row) && State.Systems.IsValidIndex(SystemIndex)))
	{
		return false;
	}
	FCosmicEvalView View(State, Index, Tables, Rules);
	View.CivIndex = bPeople ? CivIndex : INDEX_NONE;
	View.SystemIndex = SystemIndex;
	View.GalaxyRow = bPeople ? State.Civs[CivIndex].GalaxyRow : State.Systems[SystemIndex].GalaxyRow;
	View.SourceRow = RowName;
	return UCosmicEventSubsystem::CheckAll(Row.Conditions, View);
}

TArray<FCosmicPetitionUnit> CosmicPetitions::StandingHeld(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
	const FCosmicRulesDef& Rules, bool bTravelAtOrigin)
{
	using namespace CosmicPetitionsImpl;
	TArray<FCosmicPetitionUnit> Held;
	if (!State.Knowing.PendingPetitionRow.IsNone())
	{
		return Held;
	}
	const int32 CivIndex = RuledCivIndexOf(State, Index);
	if (CivIndex == INDEX_NONE)
	{
		// A wanderer that is not travelling - or, at the head of a tick, one that is, at the system it left: its cards on the
		// system it stands in (B1).
		const FCosmicVesselSubject Vessel = VesselSubjectOf(State, Index, bTravelAtOrigin);
		if (Vessel.SystemIndex == INDEX_NONE || !WithinWandererGap(State, Vessel.CivIndex))
		{
			return Held;
		}
		for (const FName RowName : Tables.GetEncounterRowNames())
		{
			const FEncounterDef* Row = Tables.FindEncounter(RowName);
			if (Row && Row->Trigger == ECosmicEventTrigger::Standing && IsWandererCard(*Row)
				&& Holds(State, Index, Tables, Rules, RowName, *Row, Vessel.CivIndex, Vessel.SystemIndex))
			{
				Held.Add({ RowName, Vessel.SystemIndex });
			}
		}
		return Held;
	}
	for (const FName RowName : Tables.GetEncounterRowNames())
	{
		const FEncounterDef* Row = Tables.FindEncounter(RowName);
		if (!Row || Row->Trigger != ECosmicEventTrigger::Standing || IsWandererCard(*Row))
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
	TConstArrayView<FCosmicPetitionUnit> HeldAtHead, bool bArrived)
{
	using namespace CosmicPetitionsImpl;
	if (!State.Knowing.PendingPetitionRow.IsNone())
	{
		return false;
	}
	const int32 CivIndex = RuledCivIndexOf(State, Index);
	if (CivIndex == INDEX_NONE)
	{
		// A wanderer that is not travelling (B1 header section 2.5). The gap holds for the cards about the owner people -
		// StandingHeld leaves them out beyond it -, not for the arrival card, which speaks of the place alone.
		const FCosmicVesselSubject Vessel = VesselSubjectOf(State, Index);
		if (Vessel.SystemIndex == INDEX_NONE)
		{
			return false;
		}
		// The life's most recent answered card does not come again at once: stepping a lane away and back brings no card
		// of the same crisis twice in a row (no saved field - the one array of the last choices).
		const FName LastAnswered = MostRecentOf(State, Tables, true, false);
		TArray<FCandidate, TInlineAllocator<4>> Candidates;
		for (const FCosmicPetitionUnit& Unit : StandingHeld(State, Index, Tables, Rules))
		{
			if (!HeldAtHead.Contains(Unit) && Unit.Row != LastAnswered)
			{
				Candidates.Add({ Unit.Row, Unit.SystemIndex, WeightOf(*Tables.FindEncounter(Unit.Row)) });
			}
		}
		// One derived integer of the place and the tick: the arrival card's share and the pick - no number of the sim stream.
		const FName SystemID = State.Systems[Vessel.SystemIndex].SystemID;
		const double Unit = static_cast<double>(CosmicSeed::DeriveForName(State.Seed, SystemID, CosmicSeed::Purpose::Encounter, static_cast<uint32>(State.Tick)))
			/ 4294967296.0;
		double PickValue = Unit;
		if (Candidates.IsEmpty())
		{
			const float Share = Rules.EncounterChancePerArrival > 0.f ? FMath::Min(Rules.EncounterChancePerArrival, 1.f) : 0.f;
			if (!bArrived || !(Unit < static_cast<double>(Share)))
			{
				return false;
			}
			for (const FName RowName : Tables.GetEncounterRowNames())
			{
				const FEncounterDef* Row = Tables.FindEncounter(RowName);
				if (Row && Row->Trigger == ECosmicEventTrigger::Lottery && IsWandererCard(*Row) && RowName != LastAnswered
					&& Holds(State, Index, Tables, Rules, RowName, *Row, Vessel.CivIndex, Vessel.SystemIndex))
				{
					Candidates.Add({ RowName, Vessel.SystemIndex, WeightOf(*Row) });
				}
			}
			// The value spread again over [0, 1): the pick reaches every row, not only the first share of the weight.
			PickValue = Unit / static_cast<double>(Share);
		}
		const FCandidate* Picked = PickByWeight(Candidates, PickValue);
		if (!Picked)
		{
			return false;
		}
		State.Knowing.PendingPetitionRow = Picked->Row;
		State.Knowing.PendingPetitionSystemID = SystemID;
		return true;
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

	// Without one, the first contact card (B3 header section 2.9): its own slot can wait one answer, a crisis card that
	// came to hold in this tick cannot (it would not come again). No draw, no period, no last-choice rule; the row's
	// conditions still hold for it.
	if (Candidates.IsEmpty())
	{
		if (const FEncounterDef* Contact = Tables.FindEncounter(CosmicRows::PetitionFirstContact);
			Contact && !IsWandererCard(*Contact) && State.Civs.IsValidIndex(Index.FindCiv(State, State.Knowing.PendingContactCivID))
			&& Holds(State, Index, Tables, Rules, CosmicRows::PetitionFirstContact, *Contact, CivIndex,
				SubjectOf(State, Index, CivIndex, Contact->Subject, NAME_None)))
		{
			State.Knowing.PendingPetitionRow = CosmicRows::PetitionFirstContact;
			State.Knowing.PendingPetitionSystemID = NAME_None;
			return true;
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
		// The last choices of the life that are petitions: a wanderer's card of the same life does not count (B1).
		TArray<FName> Recent;
		for (const FName Answered : State.Calling.Lives.IsEmpty() ? TArray<FName>() : State.Calling.Lives.Last().LastChoiceRows)
		{
			const FEncounterDef* Row = Tables.FindEncounter(Answered);
			if (!Row || !IsWandererCard(*Row))
			{
				Recent.Add(Answered);
			}
		}
		// The most recent Lottery petition among the last choices. When every Lottery row that holds is among them (a pool
		// of two or three periodic cards against the three remembered choices), only that one is passed over (MVP item 8):
		// the same periodic card never comes twice in a row, and the slot does not fall silent for the rest of the life.
		const FName LastPeriodic = MostRecentOf(State, Tables, false, true);
		for (int32 Pass = 0; Pass < 2 && Candidates.IsEmpty(); ++Pass)
		{
			for (const FName RowName : Tables.GetEncounterRowNames())
			{
				const FEncounterDef* Row = Tables.FindEncounter(RowName);
				const bool bPassedOver = Pass == 0 ? Recent.Contains(RowName) : RowName == LastPeriodic;
				if (!Row || Row->Trigger != ECosmicEventTrigger::Lottery || IsWandererCard(*Row) || bPassedOver)
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
	const uint32 Derived = CosmicSeed::DeriveForName(State.Seed, State.Civs[CivIndex].CivID, CosmicSeed::Purpose::Petition, static_cast<uint32>(State.Tick));
	const FCandidate* Picked = PickByWeight(Candidates, static_cast<double>(Derived) / 4294967296.0);
	if (!Picked)
	{
		return false;
	}
	const FEncounterDef* Row = Tables.FindEncounter(Picked->Row);
	const bool bAboutPlace = Row && Row->Subject == ECosmicEventSubject::SettledSystem && State.Systems.IsValidIndex(Picked->SystemIndex);
	State.Knowing.PendingPetitionRow = Picked->Row;
	State.Knowing.PendingPetitionSystemID = bAboutPlace ? State.Systems[Picked->SystemIndex].SystemID : FName();
	return true;
}

bool CosmicPetitions::StillStands(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules)
{
	const FName RowName = State.Knowing.PendingPetitionRow;
	const FEncounterDef* Row = RowName.IsNone() ? nullptr : Tables.FindEncounter(RowName);
	if (!Row)
	{
		return false;
	}
	if (IsWandererCard(*Row))
	{
		// A wanderer that still stands, not travelling, in the system of its card.
		const FCosmicVesselSubject Vessel = VesselSubjectOf(State, Index);
		return Vessel.SystemIndex != INDEX_NONE && State.Systems[Vessel.SystemIndex].SystemID == State.Knowing.PendingPetitionSystemID
			&& Holds(State, Index, Tables, Rules, RowName, *Row, Vessel.CivIndex, Vessel.SystemIndex);
	}
	const int32 CivIndex = RuledCivIndexOf(State, Index);
	if (CivIndex == INDEX_NONE)
	{
		return false;
	}
	const int32 SystemIndex = SubjectOf(State, Index, CivIndex, Row->Subject, State.Knowing.PendingPetitionSystemID);
	if (Row->Subject == ECosmicEventSubject::SettledSystem && SystemIndex == INDEX_NONE)
	{
		return false;
	}
	// The first contact card stands while the people it is about does (B3).
	if (RowName == CosmicRows::PetitionFirstContact && !State.Civs.IsValidIndex(Index.FindCiv(State, State.Knowing.PendingContactCivID)))
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

FFormatNamedArguments CosmicPetitions::ArgsOf(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 CivIndex, int32 SystemIndex, FName OtherCivID,
	bool bNamesOther)
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
	// The other people of a card (B3: the first contact card's) - empty words for a card about none; the card's own words
	// for a people it cannot find (the crossing stage's cleanup clears the slot of a people removed while the card waits,
	// and the card shows again after a load - it names no one by an empty name until its answer withdraws it).
	const FCivilizationState* Other = OtherCivID.IsNone() ? nullptr
		: State.Civs.FindByPredicate([OtherCivID](const FCivilizationState& Each) { return Each.CivID == OtherCivID; });
	Args.Add(TEXT("OtherCiv"), Other ? Other->CivName
		: (bNamesOther ? FText::FromStringTable(CosmicLabels::TableId, TEXT("Petition.OtherGone"), EStringTableLoadingPolicy::FindOrFullyLoad) : FText::GetEmpty()));
	return Args;
}

CosmicDiplomacy::ECosmicStanceBlock CosmicPetitions::StanceBlockOfChoice(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules,
	int32 CivIndex, FName OtherCivID, const FEncounterChoice& Choice)
{
	for (const FCosmicEffect& Effect : Choice.OnSuccess)
	{
		const CosmicDiplomacy::ECosmicStanceBlock Block = CosmicPetitionsImpl::StanceBlockOfEffect(State, Index, Rules, CivIndex, OtherCivID, Effect);
		if (Block != CosmicDiplomacy::ECosmicStanceBlock::None)
		{
			return Block;
		}
	}
	return CosmicDiplomacy::ECosmicStanceBlock::None;
}

FCosmicChoiceGate CosmicPetitions::ChoiceGateOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
	const FCosmicRulesDef& Rules, int32 CivIndex, int32 SystemIndex, FName OtherCivID, const FEncounterChoice& Choice)
{
	FCosmicChoiceGate Gate;
	for (int32 At = 0; At < Choice.OnSuccess.Num(); ++At)
	{
		const FCosmicEffect& Effect = Choice.OnSuccess[At];
		Gate.EffectAt = At;
		if (Effect.Type == EEffectType::SetStance)
		{
			Gate.Stance = CosmicPetitionsImpl::StanceBlockOfEffect(State, Index, Rules, CivIndex, OtherCivID, Effect);
			Gate.Kind = Gate.Stance != CosmicDiplomacy::ECosmicStanceBlock::None ? ECosmicChoiceGateKind::Stance : ECosmicChoiceGateKind::None;
		}
		else if (Effect.Type == EEffectType::EnactEdict)
		{
			Gate.Edict = CosmicEdicts::EnactBlockOf(State, Index, Tables, Rules, CivIndex, Effect.Key);
			Gate.Kind = Gate.Edict != CosmicEdicts::ECosmicEdictBlock::None ? ECosmicChoiceGateKind::Edict : ECosmicChoiceGateKind::None;
		}
		else if (Effect.Type == EEffectType::DelayHazard)
		{
			Gate.Resist = CosmicConflict::ResistBlockOf(State, Index, Tables, Rules, CivIndex, SystemIndex, Effect.Tag);
			Gate.Kind = Gate.Resist != CosmicConflict::ECosmicResistBlock::None ? ECosmicChoiceGateKind::Resist : ECosmicChoiceGateKind::None;
		}
		if (!Gate.IsOpen())
		{
			return Gate;
		}
	}
	return FCosmicChoiceGate();
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
