// Cosmic Ascension - Sim layer.

#include "Sim/CosmicHazards.h"

#include "Cosmic_Ascension.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Core/CosmicIDs.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Events/CosmicEventSubsystem.h"
#include "GameplayTagContainer.h"
#include "Sim/CosmicStateIndex.h"
#include "Templates/UnrealTemplate.h"

// Helpers of this file. A named namespace: the module is built as one unity file, so an anonymous one would share its
// names with every other .cpp.
namespace CosmicHazardsImpl
{
	// Stability a people loses with its capital (section 5.10).
	constexpr float CapitalLossStability = 0.3f;

	FGameplayTag TagOf(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	// The weight of a pressure source by its tier, as the pressure function reads it (section 4.2): a table too short for
	// the tier, or a value below 0 or not a number, reads as 0.
	float TierWeight(const FCosmicRulesDef& Rules, EAscensionTier Tier)
	{
		const int32 Index = FMath::Clamp(static_cast<int32>(Tier), 0, static_cast<int32>(EAscensionTier::Tier5_EternalOrder));
		const float Weight = Rules.PressureBySourceTier.IsValidIndex(Index) ? Rules.PressureBySourceTier[Index] : 0.f;
		return Weight > 0.f ? Weight : 0.f;
	}

	// How deep resolutions nest through OnResolveEffects (a row that places again what it then resolves would never end).
	// Transient: it is back to 0 whenever no Resolve runs, so it is no state of the run.
	constexpr int32 MaxNestedResolutions = 4;
	int32 NestedResolutions = 0;

	// A rebellion that leaves its slot without a change of throne (resolved, or ended with its slot by a transfer): its
	// leader leaves the record and the count of discontent starts over (section 4.1).
	void EndRebellion(FGalaxyState& State, int32 CivIndex)
	{
		static const FGameplayTag RebelTag = TagOf(TEXT("Notable.Rebel"));
		if (!State.Civs.IsValidIndex(CivIndex))
		{
			return;
		}
		FCivilizationState& Civ = State.Civs[CivIndex];
		for (FNotableRecord& Record : Civ.Notables)
		{
			if (Record.LastTick == 0 && Record.Role == RebelTag)
			{
				Record.LastTick = FMath::Max<int64>(State.Tick, 1);
			}
		}
		Civ.LowStabilityTicks = 0;
	}

	// Whether a fallen instance (its row Realm.Abyssal, sealed or not) holds SystemID in its sphere.
	bool InFallenSphere(const FGalaxyState& State, const FCosmicDataTables& Tables, FName SystemID)
	{
		static const FGameplayTag AbyssalTag = TagOf(TEXT("Realm.Abyssal"));
		for (const FTranscendentState& Transcendent : State.Transcendents)
		{
			const FTranscendentDef* Def = Tables.FindTranscendent(Transcendent.TranscendentID);
			if (Def && Def->Allegiance == AbyssalTag && Transcendent.InfluencedSystems.Contains(SystemID))
			{
				return true;
			}
		}
		return false;
	}
}

void CosmicHazards::EmptyHazardSlot(FStarSystemState& System)
{
	System.ActiveHazard = FGameplayTag();
	System.HazardTicksRemaining = 0;
	System.ResponseMegaproject = NAME_None;
	System.ResponseProgress = 0.f;
}

bool CosmicHazards::IsLastSettledSystem(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 SystemIndex)
{
	if (!State.Systems.IsValidIndex(SystemIndex) || !CosmicState::IsSettled(State.Systems[SystemIndex]))
	{
		return false;
	}
	const FStarSystemState& System = State.Systems[SystemIndex];
	const int32 OwnerIndex = Index.FindCiv(State, System.OwnerCivID);
	if (!State.Civs.IsValidIndex(OwnerIndex))
	{
		return false;
	}
	const FCivilizationState& Owner = State.Civs[OwnerIndex];
	const bool bOtherSettled = Owner.OwnedSystems.ContainsByPredicate([&State, &Index, &Owner, &System](FName SystemID)
	{
		const int32 Other = Index.FindSystem(State, SystemID);
		return SystemID != System.SystemID && State.Systems.IsValidIndex(Other) && State.Systems[Other].OwnerCivID == Owner.CivID
			&& CosmicState::IsSettled(State.Systems[Other]);
	});
	return Owner.OwnedSystems.Contains(System.SystemID) && !bOtherSettled;
}

bool CosmicHazards::LetSettlementsGo(FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex)
{
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return false;
	}
	FCivilizationState& Civ = State.Civs[CivIndex];
	TArray<int32, TInlineAllocator<4>> Owned;
	for (const FName SystemID : Civ.OwnedSystems)
	{
		const int32 SystemIndex = Index.FindSystem(State, SystemID);
		if (State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == Civ.CivID)
		{
			if (CosmicState::IsSettled(State.Systems[SystemIndex]))
			{
				return false;
			}
			Owned.Add(SystemIndex);
		}
	}
	for (const int32 SystemIndex : Owned)
	{
		FStarSystemState& System = State.Systems[SystemIndex];
		Civ.OwnedSystems.Remove(System.SystemID);
		System.OwnerCivID = NAME_None;
		EmptyHazardSlot(System);
		System.ActiveMegaproject = NAME_None;
		System.MegaprojectProgress = 0.f;
	}
	return !Owned.IsEmpty();
}

bool CosmicHazards::Place(FCosmicEvalContext& Ctx, int32 SystemIndex, FName HazardRow)
{
	FGalaxyState& State = Ctx.State;
	const FHazardDef* Row = Ctx.Tables.FindHazard(HazardRow);
	if (!Row || !Row->HazardTag.IsValid() || !State.Systems.IsValidIndex(SystemIndex))
	{
		return false;
	}
	FStarSystemState& System = State.Systems[SystemIndex];
	// Law.Hedge (section 4.5 (1)): no hazard and no claim on the sanctuary, whoever asks - refused before anything else.
	if (CosmicState::IsSanctuary(System.SystemID))
	{
		return false;
	}
	const int32 OwnerIndex = Ctx.Index.FindCiv(State, System.OwnerCivID);
	// A hazard threatens a people that lives there: never an unowned system, never one being settled (section 5.7).
	if (!CosmicState::IsSettled(System) || !State.Civs.IsValidIndex(OwnerIndex) || System.ActiveHazard.IsValid())
	{
		return false;
	}
	// The discontent of a people stands in its capital (section 4.1).
	if (Row->SpawnRule == ECosmicHazardSpawn::Capital && State.Civs[OwnerIndex].HomeSystemID != System.SystemID)
	{
		return false;
	}
	System.ActiveHazard = Row->HazardTag;
	System.HazardTicksRemaining = FMath::Max(1, Row->CountdownTicks);
	Ctx.Notes.Add({ ECosmicWorldNote::HazardPlaced, SystemIndex, OwnerIndex, HazardRow });
	return true;
}

bool CosmicHazards::Resolve(FCosmicEvalContext& Ctx, int32 SystemIndex)
{
	using namespace CosmicHazardsImpl;

	static const FGameplayTag RebellionTag = TagOf(TEXT("Hazard.Rebellion"));

	FGalaxyState& State = Ctx.State;
	if (!State.Systems.IsValidIndex(SystemIndex) || !State.Systems[SystemIndex].ActiveHazard.IsValid())
	{
		return false;
	}
	FStarSystemState& System = State.Systems[SystemIndex];
	const FGameplayTag Hazard = System.ActiveHazard;
	FName RowName;
	const FHazardDef* Row = Ctx.Tables.FindHazardByTag(Hazard, &RowName);
	const int32 OwnerIndex = Ctx.Index.FindCiv(State, System.OwnerCivID);

	// The slot first (and the response that answered it): whatever the effects below do, they find it empty.
	EmptyHazardSlot(System);

	// A rebellion that ends: its leader leaves the record, and the count of discontent starts over (section 4.1).
	if (Hazard == RebellionTag)
	{
		EndRebellion(State, OwnerIndex);
	}

	if (!Row)
	{
		return true;
	}
	// The resolution line comes before the lines of anything its effects do.
	Ctx.Notes.Add({ ECosmicWorldNote::HazardResolved, SystemIndex, OwnerIndex, RowName });
	if (NestedResolutions >= MaxNestedResolutions)
	{
		UE_LOG(LogCosmic, Error, TEXT("Resolve: the effects of %s are not applied: resolutions nested %d deep (does a row resolve again what its effects place?)"), *RowName.ToString(), NestedResolutions);
		return true;
	}
	TGuardValue<int32> Nesting(NestedResolutions, NestedResolutions + 1);

	const FCosmicEvalSubject Saved = Ctx;
	Ctx.CivIndex = OwnerIndex;
	Ctx.SystemIndex = SystemIndex;
	Ctx.OtherID = NAME_None;
	Ctx.SourceRow = RowName;
	Ctx.GalaxyRow = System.GalaxyRow;
	UCosmicEventSubsystem::ApplyAll(Row->OnResolveEffects, Ctx);
	static_cast<FCosmicEvalSubject&>(Ctx) = Saved;
	return true;
}

bool CosmicHazards::Transfer(FCosmicEvalContext& Ctx, int32 SystemIndex, int32 ToCivIndex)
{
	using namespace CosmicHazardsImpl;

	static const FGameplayTag RebellionTag = TagOf(TEXT("Hazard.Rebellion"));
	FGalaxyState& State = Ctx.State;
	if (!State.Systems.IsValidIndex(SystemIndex) || (ToCivIndex != INDEX_NONE && !State.Civs.IsValidIndex(ToCivIndex)))
	{
		return false;
	}
	FStarSystemState& System = State.Systems[SystemIndex];
	const int32 FromCivIndex = Ctx.Index.FindCiv(State, System.OwnerCivID);
	// The sanctuary changes hands by no path (Law.Hedge, section 4.5 (1)).
	if (CosmicState::IsSanctuary(System.SystemID) || IsDeadNode(System) || FromCivIndex == ToCivIndex || IsLastSettledSystem(State, Ctx.Index, SystemIndex))
	{
		return false;
	}

	if (System.ActiveHazard.IsValid())
	{
		const FHazardDef* Row = Ctx.Tables.FindHazardByTag(System.ActiveHazard);
		// A rebellion is the old owner's discontent whether a row holds its tag or not.
		if (ToCivIndex == INDEX_NONE || System.ActiveHazard == RebellionTag || (Row && Row->SpawnRule == ECosmicHazardSpawn::Capital))
		{
			if (System.ActiveHazard == RebellionTag)
			{
				EndRebellion(State, FromCivIndex);
			}
			EmptyHazardSlot(System);
		}
	}
	// Both slots were the old owner's work: a hazard that goes with the system is no longer answered.
	System.ActiveMegaproject = NAME_None;
	System.MegaprojectProgress = 0.f;
	System.ResponseMegaproject = NAME_None;
	System.ResponseProgress = 0.f;

	bool bWasCapital = false;
	if (State.Civs.IsValidIndex(FromCivIndex))
	{
		FCivilizationState& From = State.Civs[FromCivIndex];
		From.OwnedSystems.Remove(System.SystemID);
		bWasCapital = From.HomeSystemID == System.SystemID;
	}
	if (ToCivIndex != INDEX_NONE)
	{
		FCivilizationState& To = State.Civs[ToCivIndex];
		System.OwnerCivID = To.CivID;
		To.OwnedSystems.Add(System.SystemID);
		// Never told: FollowUpKnowing makes it known, and meets the old owner, when the people is the vessel's (section 8.1).
		Ctx.Notes.Add({ ECosmicWorldNote::SystemReceived, SystemIndex, ToCivIndex, NAME_None, FromCivIndex });
	}
	else
	{
		System.OwnerCivID = NAME_None;
	}
	if (bWasCapital)
	{
		MoveCapital(State, Ctx.Index, FromCivIndex);
	}
	return true;
}

void CosmicHazards::MoveCapital(FGalaxyState& State, const FCosmicStateIndex& Index, int32 CivIndex)
{
	using namespace CosmicHazardsImpl;

	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return;
	}
	FCivilizationState& Civ = State.Civs[CivIndex];
	// The first settled system the people owns, or with none the first system it owns: an entry another people owns is
	// never its capital.
	FName NewHome;
	FName FirstOwned;
	for (const FName SystemID : Civ.OwnedSystems)
	{
		const int32 SystemIndex = Index.FindSystem(State, SystemID);
		if (!State.Systems.IsValidIndex(SystemIndex) || State.Systems[SystemIndex].OwnerCivID != Civ.CivID)
		{
			continue;
		}
		if (CosmicState::IsSettled(State.Systems[SystemIndex]))
		{
			NewHome = SystemID;
			break;
		}
		if (FirstOwned.IsNone())
		{
			FirstOwned = SystemID;
		}
	}
	if (NewHome.IsNone())
	{
		NewHome = FirstOwned;
	}
	if (NewHome.IsNone())
	{
		return;
	}
	Civ.HomeSystemID = NewHome;
	Civ.Stability = FMath::Max(0.f, Civ.Stability - CapitalLossStability);
}

void CosmicHazards::Destroy(FCosmicEvalContext& Ctx, int32 SystemIndex)
{
	using namespace CosmicHazardsImpl;

	static const FGameplayTag KillTag = TagOf(TEXT("Effect.KillPlayer"));

	FGalaxyState& State = Ctx.State;
	// Nothing destroys the sanctuary (Law.Hedge, section 4.5 (1)).
	if (!State.Systems.IsValidIndex(SystemIndex) || IsDeadNode(State.Systems[SystemIndex]) || CosmicState::IsSanctuary(State.Systems[SystemIndex].SystemID))
	{
		return;
	}
	FStarSystemState& System = State.Systems[SystemIndex];
	const FName SystemID = System.SystemID;
	const TArray<FName> OldLanes = System.Lanes;
	FPlayerEntityState& Player = State.Player;
	const bool bCarryVessel = State.Calling.LifeState == ELifeState::Embodied && Player.CurrentSystemID == SystemID && State.Calling.PendingEffect != KillTag;

	// The owner lets it go, with its settlements under way when this was its last settled system (section 5.7), and a
	// lost capital moves.
	const int32 OwnerIndex = Ctx.Index.FindCiv(State, System.OwnerCivID);
	System.OwnerCivID = NAME_None;
	if (State.Civs.IsValidIndex(OwnerIndex))
	{
		State.Civs[OwnerIndex].OwnedSystems.Remove(SystemID);
		LetSettlementsGo(State, Ctx.Index, OwnerIndex);
		if (State.Civs[OwnerIndex].HomeSystemID == SystemID)
		{
			MoveCapital(State, Ctx.Index, OwnerIndex);
		}
	}

	// No lane leads to it any more, from either end.
	for (const FName Lane : OldLanes)
	{
		const int32 Neighbour = Ctx.Index.FindSystem(State, Lane);
		if (State.Systems.IsValidIndex(Neighbour))
		{
			State.Systems[Neighbour].Lanes.Remove(SystemID);
		}
	}
	System.Lanes.Reset();
	System.StarClass = FGameplayTag();
	EmptyHazardSlot(System);
	System.ActiveMegaproject = NAME_None;
	System.MegaprojectProgress = 0.f;

	// A dead star is nobody's sphere: no source, and not counted against a cap (section 16 #29 (5)).
	for (FTranscendentState& Transcendent : State.Transcendents)
	{
		Transcendent.InfluencedSystems.Remove(SystemID);
	}

	if (bCarryVessel)
	{
		// A journey from here ends with the system it began in (B1 header section 2.4): the vessel is carried, not travelling.
		Player.TravelTargetSystemID = NAME_None;
		Player.TravelTicksRemaining = 0;
		// A ruler goes to the capital of its people when the people owns it (not merely lists it).
		FName Target;
		const int32 RuledIndex = Ctx.Index.FindCiv(State, Player.RuledCivID);
		if (Player.Role == EPlayerRole::Ruler && State.Civs.IsValidIndex(RuledIndex))
		{
			const FCivilizationState& Ruled = State.Civs[RuledIndex];
			const int32 Home = Ctx.Index.FindSystem(State, Ruled.HomeSystemID);
			if (State.Systems.IsValidIndex(Home) && State.Systems[Home].OwnerCivID == Ruled.CivID)
			{
				Target = Ruled.HomeSystemID;
			}
		}
		// Never into the sanctuary, where no vessel stands (Law.Hedge, section 16 #81).
		for (int32 LaneIndex = 0; Target.IsNone() && LaneIndex < OldLanes.Num(); ++LaneIndex)
		{
			const int32 Neighbour = Ctx.Index.FindSystem(State, OldLanes[LaneIndex]);
			if (State.Systems.IsValidIndex(Neighbour) && !IsDeadNode(State.Systems[Neighbour]) && !CosmicState::IsSanctuary(OldLanes[LaneIndex]))
			{
				Target = OldLanes[LaneIndex];
			}
		}
		if (!Target.IsNone())
		{
			Player.CurrentSystemID = Target;
		}
	}
}

int32 CosmicHazards::ClaimantOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 SystemIndex)
{
	using namespace CosmicHazardsImpl;

	static const FGameplayTag AbyssalTag = TagOf(TEXT("Realm.Abyssal"));
	if (!State.Systems.IsValidIndex(SystemIndex))
	{
		return INDEX_NONE;
	}
	const FName SystemID = State.Systems[SystemIndex].SystemID;
	const int32 MaxLanes = FMath::Max(0, Rules.ClaimAdjacencyLanes);
	// The same reach as the pressure function's one lane, as a comparison: a value that is not a number reads as 0.
	const float Falloff = Rules.PressureAdjacencyFalloff > 0.f ? FMath::Min(Rules.PressureAdjacencyFalloff, 1.f) : 0.f;

	TArray<int32> Distances;
	int32 Best = INDEX_NONE;
	float BestTerm = 0.f;
	for (int32 TranscendentIndex = 0; TranscendentIndex < State.Transcendents.Num(); ++TranscendentIndex)
	{
		const FTranscendentState& Transcendent = State.Transcendents[TranscendentIndex];
		const FTranscendentDef* Def = Transcendent.bSealed ? nullptr : Tables.FindTranscendent(Transcendent.TranscendentID);
		if (!Def || Def->Allegiance != AbyssalTag || Transcendent.InfluencedSystems.Contains(SystemID))
		{
			continue;
		}
		if (Distances.IsEmpty())
		{
			Distances = CosmicState::LaneDistancesFrom(State, Index, SystemIndex, MaxLanes);
		}
		int32 Nearest = INDEX_NONE;
		for (const FName Member : Transcendent.InfluencedSystems)
		{
			const int32 MemberIndex = Index.FindSystem(State, Member);
			if (Distances.IsValidIndex(MemberIndex) && Distances[MemberIndex] != INDEX_NONE && (Nearest == INDEX_NONE || Distances[MemberIndex] < Nearest))
			{
				Nearest = Distances[MemberIndex];
			}
		}
		if (Nearest == INDEX_NONE)
		{
			continue;
		}
		// The term this transcendent lays on the system: its tier's weight, scaled by its depth, one lane away only.
		const float Depth = Def->Depth > 0.f ? FMath::Min(Def->Depth, 1.f) : 0.f;
		const float Term = TierWeight(Rules, Def->Tier) * (1.f + Depth) * (Nearest == 1 ? Falloff : 0.f);
		if (Best == INDEX_NONE || Term > BestTerm)
		{
			Best = TranscendentIndex;
			BestTerm = Term;
		}
	}
	return Best;
}

bool CosmicHazards::IsLastUnclaimed(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, int32 SystemIndex)
{
	using namespace CosmicHazardsImpl;

	if (!State.Systems.IsValidIndex(SystemIndex))
	{
		return false;
	}
	const FStarSystemState& System = State.Systems[SystemIndex];
	const int32 OwnerIndex = Index.FindCiv(State, System.OwnerCivID);
	if (!State.Civs.IsValidIndex(OwnerIndex) || !CosmicState::IsSettled(System) || InFallenSphere(State, Tables, System.SystemID))
	{
		return false;
	}
	const FCivilizationState& Owner = State.Civs[OwnerIndex];
	for (const FName SystemID : Owner.OwnedSystems)
	{
		const int32 OtherIndex = Index.FindSystem(State, SystemID);
		if (OtherIndex == SystemIndex || !State.Systems.IsValidIndex(OtherIndex))
		{
			continue;
		}
		const FStarSystemState& Other = State.Systems[OtherIndex];
		if (Other.OwnerCivID == Owner.CivID && CosmicState::IsSettled(Other) && !InFallenSphere(State, Tables, SystemID))
		{
			return false;
		}
	}
	return true;
}
