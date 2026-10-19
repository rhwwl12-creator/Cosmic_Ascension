// Cosmic Ascension - UI layer (Presentation).

#include "UI/CosmicNextRung.h"

#include "Containers/ContainerAllocationPolicies.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Events/CosmicEventSubsystem.h"
#include "Math/UnrealMathUtility.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "UI/CosmicUILabels.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicNextRungImpl
{
	// The four research fields a condition can name, with their labels: compared as FNames (the case of a name is kept in
	// an editor build only, so never through FName::ToString).
	struct FResearchField
	{
		FName Key;
		const TCHAR* LabelKey;
	};

	const FResearchField ResearchFields[] = {
		{ FName(TEXT("Physics")), TEXT("Research.Physics") },
		{ FName(TEXT("Society")), TEXT("Research.Society") },
		{ FName(TEXT("Engineering")), TEXT("Research.Engineering") },
		{ FName(TEXT("Transcendence")), TEXT("Research.Transcendence") },
	};

	// The label key of a research field; null for a key that is not one.
	const TCHAR* ResearchLabelKey(FName Key)
	{
		for (const FResearchField& Field : ResearchFields)
		{
			if (Field.Key == Key)
			{
				return Field.LabelKey;
			}
		}
		return nullptr;
	}

	// A whole percent in the bar's percent form.
	FText PercentText(int32 Whole)
	{
		return CosmicUI::Percent(static_cast<float>(Whole) / 100.f);
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

FCosmicNextRung CosmicNextRung::Of(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules)
{
	using namespace CosmicNextRungImpl;

	FCosmicNextRung Next;
	// Whom the judgment judges from the vessel's birth: the people an embodied ruler rules.
	if (State.Calling.LifeState != ELifeState::Embodied || State.Player.Role != EPlayerRole::Ruler || State.Player.RuledCivID.IsNone())
	{
		return Next;
	}
	const int32 CivIndex = Index.FindCiv(State, State.Player.RuledCivID);
	if (!State.Civs.IsValidIndex(CivIndex) || !State.Civs[CivIndex].bIsPlayerRuled)
	{
		return Next;
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
		return Next;
	}

	// The row the judgment looks at first.
	const FAscensionFeatDef* Feat = nullptr;
	for (const FName FeatRow : Tables.GetFeatRowNames())
	{
		const FAscensionFeatDef* Candidate = Tables.FindFeat(FeatRow);
		if (Candidate && Candidate->IsJudgedRulerRowAt(Civ.Tier))
		{
			Feat = Candidate;
			Next.FeatRow = FeatRow;
			break;
		}
	}
	if (!Feat)
	{
		return Next;
	}

	// The judgment's view of one settled system.
	auto ViewOn = [&](int32 SystemIndex)
	{
		FCosmicEvalView View(State, Index, Tables, Rules);
		View.CivIndex = CivIndex;
		View.SystemIndex = SystemIndex;
		View.GalaxyRow = Civ.GalaxyRow;
		View.SourceRow = Next.FeatRow;
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
		}
		else if (Condition.Type == EConditionType::CivResearchAtLeast && ResearchLabelKey(Condition.Key))
		{
			Part.Kind = ECosmicRungPartKind::Research;
			Part.NeededShare = Condition.Value;
			Part.Share = Civ.Research.FindRef(Condition.Key);
		}
		else
		{
			Part.Kind = ECosmicRungPartKind::Other;
		}
	}
	return Next;
}

FText CosmicNextRung::TextOf(const FCosmicNextRung& Next, const FCosmicDataTables& Tables)
{
	using namespace CosmicNextRungImpl;
	using CosmicUI::Label;

	if (Next.FeatRow.IsNone())
	{
		return FText::GetEmpty();
	}
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
			Named.Add(FText::Format(INVTEXT("{0} {1}({2})"), LabelKey ? Label(LabelKey) : FText::GetEmpty(), PercentText(WholePercent(Part.NeededShare)),
				PercentText(HeldPercent(Part.Share, Part.NeededShare, Part.bMet))));
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
	return FText::Format(Label(TEXT("Rung.Next")), CosmicUI::JoinList(Named));
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
