// Cosmic Ascension - Sim layer.

#include "Sim/CosmicStateIndex.h"

#include "Cosmic_Ascension.h"
#include "GameplayTagContainer.h"

void FCosmicStateIndex::Rebuild(const FGalaxyState& State)
{
	CivByID.Reset();
	CivByID.Reserve(State.Civs.Num());
	for (int32 Index = 0; Index < State.Civs.Num(); ++Index)
	{
		CivByID.Add(State.Civs[Index].CivID, Index);
	}

	SystemByID.Reset();
	SystemByID.Reserve(State.Systems.Num());
	for (int32 Index = 0; Index < State.Systems.Num(); ++Index)
	{
		SystemByID.Add(State.Systems[Index].SystemID, Index);
	}
}

int32 FCosmicStateIndex::FindCiv(const FGalaxyState& State, FName CivID) const
{
	const int32* Found = CivID.IsNone() ? nullptr : CivByID.Find(CivID);
	if (!Found)
	{
		return INDEX_NONE;
	}
	if (!State.Civs.IsValidIndex(*Found) || State.Civs[*Found].CivID != CivID)
	{
		UE_LOG(LogCosmic, Error, TEXT("State index is stale for civ %s: rebuild it after changing the number of civs"), *CivID.ToString());
		return INDEX_NONE;
	}
	return *Found;
}

int32 FCosmicStateIndex::FindSystem(const FGalaxyState& State, FName SystemID) const
{
	const int32* Found = SystemID.IsNone() ? nullptr : SystemByID.Find(SystemID);
	if (!Found)
	{
		return INDEX_NONE;
	}
	if (!State.Systems.IsValidIndex(*Found) || State.Systems[*Found].SystemID != SystemID)
	{
		UE_LOG(LogCosmic, Error, TEXT("State index is stale for system %s: rebuild it after changing the number of systems"), *SystemID.ToString());
		return INDEX_NONE;
	}
	return *Found;
}

TArray<int32> CosmicState::LaneDistancesFrom(const FGalaxyState& State, const FCosmicStateIndex& Index, int32 FromSystemIndex, int32 MaxLanes)
{
	TArray<int32> Distances;
	Distances.Init(INDEX_NONE, State.Systems.Num());
	if (!State.Systems.IsValidIndex(FromSystemIndex) || MaxLanes < 0)
	{
		return Distances;
	}
	const FName Galaxy = State.Systems[FromSystemIndex].GalaxyRow;
	TArray<int32> Queue;
	Queue.Reserve(State.Systems.Num());
	Distances[FromSystemIndex] = 0;
	Queue.Add(FromSystemIndex);
	for (int32 Head = 0; Head < Queue.Num(); ++Head)
	{
		const int32 Current = Queue[Head];
		if (Distances[Current] >= MaxLanes)
		{
			continue;
		}
		for (const FName Lane : State.Systems[Current].Lanes)
		{
			const int32 Next = Index.FindSystem(State, Lane);
			if (Next != INDEX_NONE && Distances[Next] == INDEX_NONE && State.Systems[Next].GalaxyRow == Galaxy)
			{
				Distances[Next] = Distances[Current] + 1;
				Queue.Add(Next);
			}
		}
	}
	return Distances;
}

FGameplayTag CosmicState::PickAIProfile(float Roll)
{
	static const FName Profiles[] = {
		FName(TEXT("AI.Profile.Conservative")), FName(TEXT("AI.Profile.Expansionist")),
		FName(TEXT("AI.Profile.Scholarly")), FName(TEXT("AI.Profile.Militarist"))
	};
	constexpr int32 Count = static_cast<int32>(UE_ARRAY_COUNT(Profiles));
	// A comparison, not a cast of whatever came in: a number outside [0, 1) or one that is not a number picks an end.
	const int32 Pick = Roll > 0.f ? FMath::Min(static_cast<int32>(FMath::Min(Roll, 1.f) * static_cast<float>(Count)), Count - 1) : 0;
	return FGameplayTag::RequestGameplayTag(Profiles[Pick]);
}
