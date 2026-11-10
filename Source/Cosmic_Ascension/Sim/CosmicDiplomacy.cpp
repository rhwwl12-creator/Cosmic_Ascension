// Cosmic Ascension - Sim layer (Core Logic).

#include "Sim/CosmicDiplomacy.h"

#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Events/CosmicEventSubsystem.h"
#include "Internationalization/StringTableCoreFwd.h"
#include "Internationalization/Text.h"
#include "Math/NumericLimits.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicExistenceBricks.h"
#include "Sim/CosmicStateIndex.h"

namespace CosmicDiplomacyImpl
{
	// A finite number (comparisons, no CRT call - rule 6).
	bool IsFinite(float Value)
	{
		return Value >= TNumericLimits<float>::Lowest() && Value <= TNumericLimits<float>::Max();
	}

	// A knob read as a term: itself when finite, else 0.
	float Term(float Value)
	{
		return IsFinite(Value) ? Value : 0.f;
	}

	bool IsStance(FGameplayTag Stance)
	{
		return !Stance.IsValid() || Stance == CosmicTags::StanceFriendly() || Stance == CosmicTags::StanceHostile();
	}

	bool IsTreaty(FGameplayTag Treaty)
	{
		return Treaty == CosmicTags::TreatyTrade() || Treaty == CosmicTags::TreatyNonAggression();
	}

	// Removes the entry of OtherID when it holds no stance and no treaty (the normal form).
	void Tidy(FCivilizationState& Civ, FName OtherID)
	{
		const FCosmicTie* Tie = Civ.Ties.Find(OtherID);
		if (Tie && !Tie->Stance.IsValid() && Tie->Treaties.IsEmpty())
		{
			Civ.Ties.Remove(OtherID);
		}
	}

	// The settled systems of Civ (entries of OwnedSystems whose owner is the people), in OwnedSystems order.
	template <typename AllocatorType>
	void SettledSystemsOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCivilizationState& Civ, TArray<int32, AllocatorType>& Out)
	{
		Out.Reset();
		for (const FName SystemID : Civ.OwnedSystems)
		{
			const int32 SystemIndex = Index.FindSystem(State, SystemID);
			if (State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == Civ.CivID
				&& CosmicState::IsSettled(State.Systems[SystemIndex]))
			{
				Out.Add(SystemIndex);
			}
		}
	}

	// Marks in Out (one entry per element of State.Civs) every people that owns a settled system one lane from a settled
	// system of the people CivIndex, within that system's GalaxyRow. The people itself is never marked.
	void MarkNeighboursOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex, TArray<bool>& Out)
	{
		Out.Init(false, State.Civs.Num());
		const FCivilizationState& Civ = State.Civs[CivIndex];
		TArray<int32, TInlineAllocator<16>> Settled;
		SettledSystemsOf(State, Index, Civ, Settled);
		for (const int32 SystemIndex : Settled)
		{
			const FStarSystemState& System = State.Systems[SystemIndex];
			for (const FName LaneID : System.Lanes)
			{
				const int32 Neighbour = Index.FindSystem(State, LaneID);
				if (!State.Systems.IsValidIndex(Neighbour))
				{
					continue;
				}
				const FStarSystemState& Next = State.Systems[Neighbour];
				if (Next.GalaxyRow != System.GalaxyRow || Next.OwnerCivID == Civ.CivID || !CosmicState::IsSettled(Next))
				{
					continue;
				}
				const int32 Owner = Index.FindCiv(State, Next.OwnerCivID);
				if (State.Civs.IsValidIndex(Owner) && Owner != CivIndex)
				{
					Out[Owner] = true;
				}
			}
		}
	}

	// The target without the stance term: the ethics term and the treaty term.
	float BaseTargetOf(const FGalaxyState& State, const FCosmicRulesDef& Rules, int32 Viewer, int32 Other)
	{
		const FCivilizationState& A = State.Civs[Viewer];
		const FCivilizationState& B = State.Civs[Other];
		return -Term(Rules.RelationEthicsWeight) * CosmicDiplomacy::EthicsDistanceOf(A.Ethics, B.Ethics)
			+ Term(Rules.RelationTreatyTerm) * static_cast<float>(CosmicDiplomacy::TreatyCountOf(A, B.CivID));
	}
}

const FCosmicTie* CosmicDiplomacy::TieOf(const FCivilizationState& Civ, FName OtherID)
{
	return OtherID.IsNone() ? nullptr : Civ.Ties.Find(OtherID);
}

FGameplayTag CosmicDiplomacy::StanceOf(const FCivilizationState& Civ, FName OtherID)
{
	const FCosmicTie* Tie = TieOf(Civ, OtherID);
	return Tie ? Tie->Stance : FGameplayTag();
}

bool CosmicDiplomacy::HasTreaty(const FCivilizationState& Civ, FName OtherID, FGameplayTag Treaty)
{
	const FCosmicTie* Tie = TieOf(Civ, OtherID);
	return Tie && Treaty.IsValid() && Tie->Treaties.HasTagExact(Treaty);
}

int32 CosmicDiplomacy::TreatyCountOf(const FCivilizationState& Civ, FName OtherID)
{
	const FCosmicTie* Tie = TieOf(Civ, OtherID);
	return Tie ? Tie->Treaties.Num() : 0;
}

int32 CosmicDiplomacy::TradeTreatyCountOf(const FCivilizationState& Civ)
{
	int32 Count = 0;
	for (const TPair<FName, FCosmicTie>& Pair : Civ.Ties)
	{
		Count += Pair.Value.Treaties.HasTagExact(CosmicTags::TreatyTrade()) ? 1 : 0;
	}
	return Count;
}

int32 CosmicDiplomacy::TreatyPartnersOf(const FCivilizationState& Civ, FGameplayTag Treaty)
{
	int32 Count = 0;
	for (const TPair<FName, FCosmicTie>& Pair : Civ.Ties)
	{
		Count += (Treaty.IsValid() ? Pair.Value.Treaties.HasTagExact(Treaty) : !Pair.Value.Treaties.IsEmpty()) ? 1 : 0;
	}
	return Count;
}

int32 CosmicDiplomacy::FriendlyCountOf(const FCivilizationState& Civ)
{
	int32 Count = 0;
	for (const TPair<FName, FCosmicTie>& Pair : Civ.Ties)
	{
		Count += (Pair.Value.Stance == CosmicTags::StanceFriendly()) ? 1 : 0;
	}
	return Count;
}

int32 CosmicDiplomacy::FriendlySlotsOf(const FCivilizationState& Civ, const FCosmicRulesDef& Rules)
{
	const int32 At = static_cast<int32>(Civ.Tier);
	return Rules.FriendlySlotsByTier.IsValidIndex(At) ? FMath::Max(0, Rules.FriendlySlotsByTier[At]) : 0;
}

void CosmicDiplomacy::SetStanceOf(FCivilizationState& Civ, FName OtherID, FGameplayTag Stance)
{
	if (OtherID.IsNone())
	{
		return;
	}
	const FGameplayTag Read = CosmicDiplomacyImpl::IsStance(Stance) ? Stance : FGameplayTag();
	if (!Read.IsValid())
	{
		if (FCosmicTie* Tie = Civ.Ties.Find(OtherID))
		{
			Tie->Stance = FGameplayTag();
			CosmicDiplomacyImpl::Tidy(Civ, OtherID);
		}
		return;
	}
	Civ.Ties.FindOrAdd(OtherID).Stance = Read;
}

void CosmicDiplomacy::Sign(FGalaxyState& State, int32 A, int32 B, FGameplayTag Treaty)
{
	if (!State.Civs.IsValidIndex(A) || !State.Civs.IsValidIndex(B) || A == B || !CosmicDiplomacyImpl::IsTreaty(Treaty))
	{
		return;
	}
	State.Civs[A].Ties.FindOrAdd(State.Civs[B].CivID).Treaties.AddTag(Treaty);
	State.Civs[B].Ties.FindOrAdd(State.Civs[A].CivID).Treaties.AddTag(Treaty);
}

bool CosmicDiplomacy::Break(FGalaxyState& State, int32 A, int32 B, FGameplayTag Treaty)
{
	if (!State.Civs.IsValidIndex(A) || !State.Civs.IsValidIndex(B) || A == B || !Treaty.IsValid())
	{
		return false;
	}
	bool bHeld = false;
	const auto RemoveFrom = [&bHeld, Treaty](FCivilizationState& Civ, FName OtherID)
	{
		if (FCosmicTie* Tie = Civ.Ties.Find(OtherID))
		{
			bHeld |= Tie->Treaties.RemoveTag(Treaty);
			CosmicDiplomacyImpl::Tidy(Civ, OtherID);
		}
	};
	RemoveFrom(State.Civs[A], State.Civs[B].CivID);
	RemoveFrom(State.Civs[B], State.Civs[A].CivID);
	return bHeld;
}

void CosmicDiplomacy::ForgetCiv(FGalaxyState& State, FName CivID)
{
	if (CivID.IsNone())
	{
		return;
	}
	for (FCivilizationState& Civ : State.Civs)
	{
		Civ.Ties.Remove(CivID);
	}
}

bool CosmicDiplomacy::Borders(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 A, int32 B)
{
	if (!State.Civs.IsValidIndex(A) || !State.Civs.IsValidIndex(B) || A == B)
	{
		return false;
	}
	TArray<bool> Marks;
	CosmicDiplomacyImpl::MarkNeighboursOf(State, Index, A, Marks);
	return Marks[B];
}

void CosmicDiplomacy::BordersOf(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 A, TArray<bool>& Out)
{
	if (!State.Civs.IsValidIndex(A))
	{
		Out.Init(false, State.Civs.Num());
		return;
	}
	CosmicDiplomacyImpl::MarkNeighboursOf(State, Index, A, Out);
}

TArray<TArray<int32>> CosmicDiplomacy::BorderTableOf(const FGalaxyState& State, const FCosmicStateIndex& Index)
{
	TArray<TArray<int32>> Table;
	Table.SetNum(State.Civs.Num());
	TArray<bool> Marks;
	for (int32 A = 0; A < State.Civs.Num(); ++A)
	{
		CosmicDiplomacyImpl::MarkNeighboursOf(State, Index, A, Marks);
		for (int32 B = 0; B < Marks.Num(); ++B)
		{
			if (Marks[B])
			{
				Table[A].Add(B);
			}
		}
	}
	return Table;
}

float CosmicDiplomacy::EthicsDistanceOf(const FEthicsAxes& A, const FEthicsAxes& B)
{
	return (FMath::Abs(A.Pacifism - B.Pacifism) + FMath::Abs(A.Spiritualism - B.Spiritualism) + FMath::Abs(A.Openness - B.Openness)) / 3.f;
}

float CosmicDiplomacy::TargetRelationOf(const FGalaxyState& State, const FCosmicRulesDef& Rules, int32 Viewer, int32 Other)
{
	if (!State.Civs.IsValidIndex(Viewer) || !State.Civs.IsValidIndex(Other) || Viewer == Other)
	{
		return 0.f;
	}
	float Target = CosmicDiplomacyImpl::BaseTargetOf(State, Rules, Viewer, Other);
	const FGameplayTag Theirs = StanceOf(State.Civs[Other], State.Civs[Viewer].CivID);
	const int32 TermAt = (Theirs == CosmicTags::StanceFriendly()) ? 0 : (Theirs == CosmicTags::StanceHostile()) ? 1 : INDEX_NONE;
	if (Rules.RelationStanceTerms.IsValidIndex(TermAt))
	{
		Target += CosmicDiplomacyImpl::Term(Rules.RelationStanceTerms[TermAt]);
	}
	// The bricks of the people seen (B7x [B-7x2]): a star eater is seen worse, a merging people better - 0 for a people that
	// carries neither or under the inert knobs, and then the target keeps its bits. A term added like the stance term; the
	// drift stage clamps the relation.
	const float BrickTerm = CosmicBricks::RelationTargetTermOf(State.Civs[Other], Rules);
	if (BrickTerm != 0.f)
	{
		Target += BrickTerm;
	}
	return Target;
}

float CosmicDiplomacy::ViewOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules, int32 Viewer, int32 Other)
{
	if (!State.Civs.IsValidIndex(Viewer) || !State.Civs.IsValidIndex(Other) || Viewer == Other)
	{
		return 0.f;
	}
	if (Borders(State, Index, Viewer, Other))
	{
		return State.Civs[Viewer].Relations.FindRef(State.Civs[Other].CivID);
	}
	return CosmicDiplomacyImpl::BaseTargetOf(State, Rules, Viewer, Other);
}

int32 CosmicDiplomacy::ViewBandOf(float View)
{
	// The band edges are constants, not knobs (header section 3.1); a value that is not a number reads as indifferent.
	if (View <= -0.5f)
	{
		return 0;
	}
	if (View <= -0.2f)
	{
		return 1;
	}
	if (View < 0.2f)
	{
		return 2;
	}
	if (View < 0.5f)
	{
		return 3;
	}
	return (View >= 0.5f) ? 4 : 2;
}

float CosmicDiplomacy::TreatyAcceptBonusOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables)
{
	float Sum = 0.f;
	for (const FName Row : Civ.ActiveEdicts)
	{
		const FEdictDef* Def = Tables.FindEdict(Row);
		if (Def && Def->TreatyAcceptBonus > 0.f && Def->TreatyAcceptBonus <= TNumericLimits<float>::Max())
		{
			Sum += Def->TreatyAcceptBonus;
		}
	}
	return Sum;
}

float CosmicDiplomacy::TreatyBarOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules)
{
	return CosmicDiplomacyImpl::Term(Rules.TreatyAcceptRelation) - TreatyAcceptBonusOf(Civ, Tables);
}

float CosmicDiplomacy::TreatyCostOf(const FCosmicRulesDef& Rules)
{
	return (Rules.TreatyCostInfluence > 0.f && Rules.TreatyCostInfluence <= TNumericLimits<float>::Max()) ? Rules.TreatyCostInfluence : 0.f;
}

CosmicDiplomacy::ECosmicStanceBlock CosmicDiplomacy::StanceBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index,
	const FCosmicRulesDef& Rules, int32 CivIndex, FName OtherID, FGameplayTag Stance, bool bMet)
{
	const int32 OtherIndex = Index.FindCiv(State, OtherID);
	if (!State.Civs.IsValidIndex(CivIndex) || !State.Civs.IsValidIndex(OtherIndex) || !CosmicDiplomacyImpl::IsStance(Stance))
	{
		return ECosmicStanceBlock::NoCiv;
	}
	if (OtherIndex == CivIndex)
	{
		return ECosmicStanceBlock::Self;
	}
	if (!bMet)
	{
		return ECosmicStanceBlock::NotMet;
	}
	const FCivilizationState& Civ = State.Civs[CivIndex];
	if (StanceOf(Civ, OtherID) == Stance)
	{
		return ECosmicStanceBlock::Same;
	}
	if (Stance == CosmicTags::StanceFriendly() && FriendlyCountOf(Civ) >= FriendlySlotsOf(Civ, Rules))
	{
		return ECosmicStanceBlock::FriendlySlots;
	}
	if (Stance == CosmicTags::StanceHostile())
	{
		if (static_cast<uint8>(Civ.Tier) < static_cast<uint8>(EAscensionTier::Tier2_Stellar))
		{
			return ECosmicStanceBlock::Tier;
		}
		if (HasTreaty(Civ, OtherID, CosmicTags::TreatyNonAggression()))
		{
			return ECosmicStanceBlock::NonAggression;
		}
	}
	return ECosmicStanceBlock::None;
}

CosmicDiplomacy::ECosmicTreatyBlock CosmicDiplomacy::TreatyBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index,
	const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 CivIndex, FName OtherID, FGameplayTag Treaty, bool bMet)
{
	const int32 OtherIndex = Index.FindCiv(State, OtherID);
	if (!State.Civs.IsValidIndex(CivIndex) || !State.Civs.IsValidIndex(OtherIndex) || !CosmicDiplomacyImpl::IsTreaty(Treaty))
	{
		return ECosmicTreatyBlock::NoCiv;
	}
	if (OtherIndex == CivIndex)
	{
		return ECosmicTreatyBlock::Self;
	}
	if (!bMet)
	{
		return ECosmicTreatyBlock::NotMet;
	}
	const FCivilizationState& Civ = State.Civs[CivIndex];
	const FCivilizationState& Other = State.Civs[OtherIndex];
	if (HasTreaty(Civ, OtherID, Treaty))
	{
		return ECosmicTreatyBlock::Held;
	}
	if (static_cast<uint8>(Civ.Tier) < static_cast<uint8>(EAscensionTier::Tier2_Stellar))
	{
		return ECosmicTreatyBlock::Tier;
	}
	if (StanceOf(Civ, OtherID) == CosmicTags::StanceHostile() || StanceOf(Other, Civ.CivID) == CosmicTags::StanceHostile())
	{
		return ECosmicTreatyBlock::Hostile;
	}
	if (!(ViewOf(State, Index, Rules, OtherIndex, CivIndex) >= TreatyBarOf(Civ, Tables, Rules)))
	{
		return ECosmicTreatyBlock::Refused;
	}
	if (Civ.Stockpile.Influence < TreatyCostOf(Rules))
	{
		return ECosmicTreatyBlock::Cost;
	}
	return ECosmicTreatyBlock::None;
}

bool CosmicDiplomacy::AsksForTreatyOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
	const FCosmicRulesDef& Rules, int32 CivIndex)
{
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return false;
	}
	const FCivilizationState& Civ = State.Civs[CivIndex];
	for (const FName FeatRow : Tables.GetFeatRowNames())
	{
		const FAscensionFeatDef* Feat = Tables.FindFeat(FeatRow);
		if (!Feat || !Feat->IsJudgedRulerRowAt(Civ.Tier) || !Feat->CarriesCondition(EConditionType::CivHasTreaty))
		{
			continue;
		}
		FCosmicEvalView View(State, Index, Tables, Rules);
		View.CivIndex = CivIndex;
		View.SystemIndex = Index.FindSystem(State, Civ.HomeSystemID);
		View.GalaxyRow = Civ.GalaxyRow;
		View.SourceRow = FeatRow;
		for (const FCosmicCondition& Condition : Feat->CompletionConditions)
		{
			if (Condition.Type == EConditionType::CivHasTreaty && !UCosmicEventSubsystem::Check(Condition, View))
			{
				return true;
			}
		}
	}
	return false;
}

int32 CosmicDiplomacy::AITreatyPartnerOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables,
	const FCosmicRulesDef& Rules, int32 CivIndex)
{
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return INDEX_NONE;
	}
	TArray<bool> Bordering;
	BordersOf(State, Index, CivIndex, Bordering);
	for (int32 Other = 0; Other < State.Civs.Num(); ++Other)
	{
		if (Bordering[Other] && !State.Civs[Other].bIsPlayerRuled
			&& TreatyBlockOf(State, Index, Tables, Rules, CivIndex, State.Civs[Other].CivID, CosmicTags::TreatyNonAggression(), true) == ECosmicTreatyBlock::None)
		{
			return Other;
		}
	}
	return INDEX_NONE;
}

bool CosmicDiplomacy::TurnHostile(FGalaxyState& State, int32 CivIndex, int32 Other, float Penalty)
{
	if (!State.Civs.IsValidIndex(CivIndex) || !State.Civs.IsValidIndex(Other) || CivIndex == Other)
	{
		return false;
	}
	SetStanceOf(State.Civs[CivIndex], State.Civs[Other].CivID, CosmicTags::StanceHostile());
	const bool bBroke = Break(State, CivIndex, Other, CosmicTags::TreatyTrade());
	if (bBroke)
	{
		LowerRelation(State.Civs[Other], State.Civs[CivIndex].CivID, Penalty);
	}
	return bBroke;
}

FText CosmicDiplomacy::StanceNameOf(FGameplayTag Stance)
{
	const FString Key = Stance.IsValid() ? Stance.GetTagName().ToString() : FString(TEXT("Stance.Neutral"));
	return FText::FromStringTable(CosmicLabels::TableId, Key, EStringTableLoadingPolicy::FindOrFullyLoad);
}

FText CosmicDiplomacy::TreatyNameOf(FGameplayTag Treaty)
{
	return FText::FromStringTable(CosmicLabels::TableId, Treaty.GetTagName().ToString(), EStringTableLoadingPolicy::FindOrFullyLoad);
}

void CosmicDiplomacy::LowerRelation(FCivilizationState& Viewer, FName OtherID, float Amount)
{
	if (OtherID.IsNone() || !(Amount > 0.f && Amount <= TNumericLimits<float>::Max()))
	{
		return;
	}
	float& Relation = Viewer.Relations.FindOrAdd(OtherID);
	Relation = FMath::Clamp(Relation - Amount, -1.f, 1.f);
}
