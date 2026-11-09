// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicNextRung.h"

#include "Containers/ContainerAllocationPolicies.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Events/CosmicEventSubsystem.h"
#include "Math/UnrealMathUtility.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicSeal.h"
#include "Sim/CosmicStateIndex.h"
#include "UI/CosmicUILabels.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicNextRungImpl
{
	// The four research fields a condition can name, with their labels (the table the panel and the top bar share).
	using CosmicUI::ResearchLabelKey;

	// A whole percent in the bar's percent form.
	FText PercentText(int32 Whole)
	{
		return CosmicUI::Percent(static_cast<float>(Whole) / 100.f);
	}

	FText PartsText(const FCosmicNextRung& Next, const FCosmicDataTables& Tables, ECosmicTimeUnit Unit);

	// Whether a part of Next counts held galaxy laws: the tooltip then ends with Rung.LawsReset.
	bool HasLawsPart(const FCosmicNextRung& Next)
	{
		return Next.Parts.ContainsByPredicate([](const FCosmicRungPart& Part) { return Part.Kind == ECosmicRungPartKind::Laws; });
	}
}

bool FCosmicNextRung::AllMet() const
{
	if (FeatRow.IsNone())
	{
		return false;
	}
	for (const FCosmicRungPart& Part : Parts)
	{
		if (!Part.bMet)
		{
			return false;
		}
	}
	return true;
}

TArray<FCosmicNextRung> CosmicNextRung::AllOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules)
{
	using namespace CosmicNextRungImpl;

	TArray<FCosmicNextRung> Rungs;
	// Whom the judgment judges from the vessel's birth: the people an embodied ruler rules.
	if (State.Calling.LifeState != ELifeState::Embodied || State.Player.Role != EPlayerRole::Ruler || State.Player.RuledCivID.IsNone())
	{
		return Rungs;
	}
	const int32 CivIndex = Index.FindCiv(State, State.Player.RuledCivID);
	if (!State.Civs.IsValidIndex(CivIndex) || !State.Civs[CivIndex].bIsPlayerRuled)
	{
		return Rungs;
	}
	const FCivilizationState& Civ = State.Civs[CivIndex];

	// The settled systems the people owns, in OwnedSystems order: a feat holds on one of them (an entry another people
	// owns counts for nothing, as in the judgment).
	TArray<int32, TInlineAllocator<8>> Settled;
	for (const FName SystemID : Civ.OwnedSystems)
	{
		const int32 SystemIndex = Index.FindSystem(State, SystemID);
		if (State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == Civ.CivID && CosmicState::IsSettled(State.Systems[SystemIndex]))
		{
			Settled.Add(SystemIndex);
		}
	}
	if (Settled.IsEmpty())
	{
		return Rungs;
	}

	// Every row the judgment looks at, in its order.
	for (const FName FeatRow : Tables.GetFeatRowNames())
	{
		const FAscensionFeatDef* Feat = Tables.FindFeat(FeatRow);
		if (!Feat || !Feat->IsJudgedRulerRowAt(Civ.Tier))
		{
			continue;
		}
		FCosmicNextRung& Next = Rungs.AddDefaulted_GetRef();
		Next.FeatRow = FeatRow;

		// The judgment's view of one settled system.
		auto ViewOn = [&](int32 SystemIndex)
		{
			FCosmicEvalView View(State, Index, Tables, Rules);
			View.CivIndex = CivIndex;
			View.SystemIndex = SystemIndex;
			View.GalaxyRow = Civ.GalaxyRow;
			View.SourceRow = FeatRow;
			View.SinceTick = State.Calling.BornTick;
			return View;
		};

		// The settled system on which most parts hold, the first in OwnedSystems order among equals.
		int32 MostHeld = -1;
		for (const int32 SystemIndex : Settled)
		{
			const FCosmicEvalView View = ViewOn(SystemIndex);
			int32 Held = 0;
			for (const FCosmicCondition& Condition : Feat->CompletionConditions)
			{
				Held += UCosmicEventSubsystem::Check(Condition, View) ? 1 : 0;
			}
			if (Held > MostHeld)
			{
				MostHeld = Held;
				Next.SystemIndex = SystemIndex;
			}
		}

		const FCosmicEvalView View = ViewOn(Next.SystemIndex);
		for (const FCosmicCondition& Condition : Feat->CompletionConditions)
		{
			FCosmicRungPart& Part = Next.Parts.AddDefaulted_GetRef();
			Part.bMet = UCosmicEventSubsystem::Check(Condition, View);
			Part.Key = Condition.Key;
			const FMegaprojectDef* Project = (Condition.Type == EConditionType::MegaprojectComplete && !Condition.Key.IsNone()) ? Tables.FindMegaproject(Condition.Key) : nullptr;
			if (Project)
			{
				Part.Kind = ECosmicRungPartKind::Project;
				Part.Needed = CosmicMegaprojects::RequiredProgressOf(*Project);
				const bool bAnswer = Tables.IsHazardAnswer(Condition.Key);
				if (!Part.bMet)
				{
					// Where it runs now: the first settled system of the people whose slot holds the row.
					for (const int32 SystemIndex : Settled)
					{
						const CosmicMegaprojects::FCosmicSlotReading Slot = CosmicMegaprojects::ReadSlot(State.Systems[SystemIndex],
							bAnswer ? CosmicMegaprojects::ECosmicProjectSlot::Response : CosmicMegaprojects::ECosmicProjectSlot::Construction);
						if (Slot.Row == Condition.Key)
						{
							Part.Progress = Slot.Progress;
							break;
						}
					}
				}
				if (bAnswer)
				{
					for (const FName HazardRow : Tables.GetHazardRowNames())
					{
						const FHazardDef* Hazard = Tables.FindHazard(HazardRow);
						if (Hazard && Hazard->RulerResolutions.Contains(Condition.Key))
						{
							Part.HazardRow = HazardRow;
							break;
						}
					}
				}
				if (Condition.Key == CosmicRows::ProjectBinding && !Part.bMet)
				{
					// Closed in this run: no fallen one left to bind, and no system holds this life's completion - a
					// completion on a system the people lost can be taken back (decision 14), so every system is read, and such a
					// completion reads as lost rather than not started.
					bool bAnyRecord = false;
					for (const FStarSystemState& System : State.Systems)
					{
						if (CosmicMegaprojects::HasCompleted(System, Condition.Key, View.SinceTick, Civ.CivID))
						{
							bAnyRecord = true;
							break;
						}
					}
					Part.bClosed = CosmicSeal::BindTargetOf(State, Tables) == INDEX_NONE && !bAnyRecord;
					Part.bLost = bAnyRecord;
				}
			}
			else if (Condition.Type == EConditionType::CivResearchAtLeast && ResearchLabelKey(Condition.Key))
			{
				Part.Kind = ECosmicRungPartKind::Research;
				Part.NeededShare = Condition.Value;
				Part.Share = Civ.Research.FindRef(Condition.Key);
			}
			else if (Condition.Type == EConditionType::CivSettledSystemsAtLeast)
			{
				// The settled systems as the condition counts them (the list above is the same set).
				Part.Kind = ECosmicRungPartKind::Realm;
				Part.Progress = static_cast<float>(Settled.Num());
				Part.Needed = Condition.Value;
			}
			else if (Condition.Type == EConditionType::CivHasTreaty)
			{
				// The evaluator's own count: the peoples sharing the treaty (any treaty for an empty Tag).
				Part.Kind = ECosmicRungPartKind::Treaty;
				Part.Progress = static_cast<float>(CosmicDiplomacy::TreatyPartnersOf(Civ, Condition.Tag));
				Part.Needed = Condition.Value;
			}
			else if (Condition.Type == EConditionType::CivLawsHeldTicksAtLeast)
			{
				Part.Kind = ECosmicRungPartKind::Laws;
				Part.Progress = static_cast<float>(CosmicEdicts::GalaxyLawCountOf(Civ, Tables));
				Part.Needed = static_cast<float>(CosmicEdicts::GalaxyLawsToHoldOf(Rules));
				// Every law held: the ticks still to hold within this life, as the evaluator counts them.
				const int32 Ticks = CosmicConditions::TickCountOf(Condition.Key);
				if (Part.Progress >= Part.Needed && !Part.bMet && Ticks > 0)
				{
					Part.TicksLeft = FMath::Max<int64>(0, Ticks - FMath::Min<int64>(Civ.LawsHeldTicks, State.Tick - View.SinceTick));
				}
			}
			else
			{
				Part.Kind = ECosmicRungPartKind::Other;
			}
		}
	}
	return Rungs;
}

FCosmicNextRung CosmicNextRung::Of(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules)
{
	TArray<FCosmicNextRung> Rungs = AllOf(State, Index, Tables, Rules);
	return Rungs.IsEmpty() ? FCosmicNextRung() : MoveTemp(Rungs[0]);
}

// The parts of one row as TextOf joins them inside Rung.Next.
FText CosmicNextRungImpl::PartsText(const FCosmicNextRung& Next, const FCosmicDataTables& Tables, ECosmicTimeUnit Unit)
{
	using CosmicUI::Label;

	TArray<FText> Named;
	bool bAnyOther = false;
	bool bOthersMet = true;
	for (const FCosmicRungPart& Part : Next.Parts)
	{
		if (Part.Kind == ECosmicRungPartKind::Project)
		{
			const FMegaprojectDef* Row = Tables.FindMegaproject(Part.Key);
			const FText Name = Row ? Row->DisplayName : FText::AsCultureInvariant(Part.Key.ToString());
			FText Status;
			if (Part.bMet)
			{
				Status = Label(TEXT("Rung.Done"));
			}
			else if (Part.bClosed)
			{
				Status = Label(TEXT("Rung.NoFallen"));
			}
			else if (Part.bLost)
			{
				Status = Label(TEXT("Rung.Lost"));
			}
			else if (Part.Progress >= 0.f)
			{
				Status = FText::Format(INVTEXT("{0}/{1}"), CosmicUI::WholeNumber(Part.Progress), CosmicUI::WholeNumber(Part.Needed));
			}
			else if (!Part.HazardRow.IsNone())
			{
				const FHazardDef* Hazard = Tables.FindHazard(Part.HazardRow);
				Status = FText::Format(Label(TEXT("Rung.WhenHazard")), Hazard ? Hazard->DisplayName : FText::AsCultureInvariant(Part.HazardRow.ToString()));
			}
			else
			{
				Status = Label(TEXT("Rung.NotStarted"));
			}
			Named.Add(FText::Format(INVTEXT("{0}({1})"), Name, Status));
		}
		else if (Part.Kind == ECosmicRungPartKind::Research)
		{
			const TCHAR* LabelKey = ResearchLabelKey(Part.Key);
			Named.Add(FText::Format(INVTEXT("{0} {1}({2})"), LabelKey ? Label(LabelKey) : FText::GetEmpty(), PercentText(CosmicNextRung::WholePercent(Part.NeededShare)),
				PercentText(CosmicNextRung::HeldPercent(Part.Share, Part.NeededShare, Part.bMet))));
		}
		else if (Part.Kind == ECosmicRungPartKind::Realm)
		{
			Named.Add(FText::Format(Label(TEXT("Rung.Realm")), CosmicUI::WholeNumber(Part.Progress), CosmicUI::WholeNumber(Part.Needed)));
		}
		else if (Part.Kind == ECosmicRungPartKind::Treaty)
		{
			Named.Add(FText::Format(Label(TEXT("Rung.Treaty")), CosmicUI::WholeNumber(Part.Progress), CosmicUI::WholeNumber(Part.Needed)));
		}
		else if (Part.Kind == ECosmicRungPartKind::Laws)
		{
			Named.Add(Part.TicksLeft > 0
				? FText::Format(Label(TEXT("Rung.LawsLeft")), CosmicUI::WholeNumber(Part.Progress), CosmicUI::WholeNumber(Part.Needed), CosmicUI::DurationLabel(Part.TicksLeft, Unit))
				: FText::Format(Label(TEXT("Rung.Laws")), CosmicUI::WholeNumber(Part.Progress), CosmicUI::WholeNumber(Part.Needed)));
		}
		else
		{
			bAnyOther = true;
			bOthersMet = bOthersMet && Part.bMet;
		}
	}
	// Every other condition in one part, with no number: a pressure or a count is never shown (section 8.3).
	if (bAnyOther)
	{
		Named.Add(FText::Format(INVTEXT("{0}({1})"), Label(TEXT("Rung.Other")), Label(bOthersMet ? TEXT("Rung.Met") : TEXT("Rung.NotMet"))));
	}
	return CosmicUI::JoinList(Named);
}

FText CosmicNextRung::TextOf(const FCosmicNextRung& Next, const FCosmicDataTables& Tables)
{
	return TextOf(Next, Tables, ECosmicTimeUnit::Cycle);
}

FText CosmicNextRung::TextOf(const FCosmicNextRung& Next, const FCosmicDataTables& Tables, ECosmicTimeUnit Unit)
{
	if (Next.FeatRow.IsNone())
	{
		return FText::GetEmpty();
	}
	const FText Text = FText::Format(CosmicUI::Label(TEXT("Rung.Next")), CosmicNextRungImpl::PartsText(Next, Tables, Unit));
	// What breaks the count of held laws, once, as the last line.
	return CosmicNextRungImpl::HasLawsPart(Next) ? FText::Format(INVTEXT("{0}\n{1}"), Text, CosmicUI::Label(TEXT("Rung.LawsReset"))) : Text;
}

FText CosmicNextRung::TextOfAll(const TArray<FCosmicNextRung>& Rungs, const FCosmicDataTables& Tables)
{
	return TextOfAll(Rungs, Tables, ECosmicTimeUnit::Cycle);
}

FText CosmicNextRung::TextOfAll(const TArray<FCosmicNextRung>& Rungs, const FCosmicDataTables& Tables, ECosmicTimeUnit Unit)
{
	using CosmicUI::Label;

	if (Rungs.Num() <= 1)
	{
		return Rungs.IsEmpty() ? FText::GetEmpty() : TextOf(Rungs[0], Tables, Unit);
	}
	FString Text = Label(TEXT("Rung.NextAny")).ToString();
	bool bAnyLaws = false;
	for (const FCosmicNextRung& Next : Rungs)
	{
		const FAscensionFeatDef* Feat = Tables.FindFeat(Next.FeatRow);
		const FText Title = (Feat && !Feat->Title.IsEmpty()) ? Feat->Title : FText::AsCultureInvariant(Next.FeatRow.ToString());
		Text += TEXT("\n") + FText::Format(Label(TEXT("Rung.Row")), Title, CosmicNextRungImpl::PartsText(Next, Tables, Unit)).ToString();
		bAnyLaws = bAnyLaws || CosmicNextRungImpl::HasLawsPart(Next);
	}
	// Once for every row, after the last.
	if (bAnyLaws)
	{
		Text += TEXT("\n") + Label(TEXT("Rung.LawsReset")).ToString();
	}
	return FText::AsCultureInvariant(Text);
}

TArray<FCosmicRungPart> CosmicNextRung::ResearchFieldsOf(const TArray<FCosmicNextRung>& Rungs)
{
	TArray<FCosmicRungPart> Fields;
	for (const FCosmicNextRung& Rung : Rungs)
	{
		for (const FCosmicRungPart& Part : Rung.Parts)
		{
			if (Part.Kind == ECosmicRungPartKind::Research)
			{
				Fields.Add(Part);
			}
		}
	}
	return Fields;
}

bool CosmicNextRung::HasJudgedRowAt(const FCosmicDataTables& Tables, EAscensionTier Tier)
{
	for (const FName FeatRow : Tables.GetFeatRowNames())
	{
		const FAscensionFeatDef* Feat = Tables.FindFeat(FeatRow);
		if (Feat && Feat->IsJudgedRulerRowAt(Tier))
		{
			return true;
		}
	}
	return false;
}

bool CosmicNextRung::HasJudgedRowAskingAt(const FCosmicDataTables& Tables, EAscensionTier Tier, EConditionType Type)
{
	for (const FName FeatRow : Tables.GetFeatRowNames())
	{
		const FAscensionFeatDef* Feat = Tables.FindFeat(FeatRow);
		if (Feat && Feat->IsJudgedRulerRowAt(Tier) && Feat->CarriesCondition(Type))
		{
			return true;
		}
	}
	return false;
}

int32 CosmicNextRung::WholePercent(float Share)
{
	if (!(Share > 0.f))
	{
		return 0;
	}
	// FText::AsPercent multiplies in the value's own type, then the formatter rounds the double: the same steps here (a
	// share far above 1 is cut, so the integer cannot overflow).
	const float Scaled = FMath::Min(Share, 1.0e6f) * 100.f;
	return static_cast<int32>(FMath::RoundHalfToEven(FMath::TruncateToHalfIfClose(static_cast<double>(Scaled))));
}

int32 CosmicNextRung::HeldPercent(float Share, float NeededShare, bool bMet)
{
	const int32 Held = WholePercent(Share);
	if (bMet)
	{
		return Held;
	}
	return FMath::Max(0, FMath::Min(Held, WholePercent(NeededShare) - 1));
}
