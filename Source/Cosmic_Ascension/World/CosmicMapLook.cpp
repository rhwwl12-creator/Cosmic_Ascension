// Cosmic Ascension - World layer (Presentation).

#include "World/CosmicMapLook.h"

#include "Containers/Map.h"
#include "Containers/Set.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "GameplayTagContainer.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicMegaprojects.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicMapLookImpl
{
	struct FStarClassLook
	{
		const TCHAR* Tag;
		FLinearColor Color;
		float Scale;
	};

	// The colour and size of each star class, hot and large to cool and small (presentation only, no rule reads it).
	const FStarClassLook StarClasses[] = {
		{ TEXT("Star.Class.O"), FLinearColor(0.3f, 0.45f, 1.f), 2.f },
		{ TEXT("Star.Class.B"), FLinearColor(0.45f, 0.6f, 1.f), 1.7f },
		{ TEXT("Star.Class.A"), FLinearColor(0.75f, 0.82f, 1.f), 1.4f },
		{ TEXT("Star.Class.F"), FLinearColor(1.f, 0.95f, 0.78f), 1.2f },
		{ TEXT("Star.Class.G"), FLinearColor(1.f, 0.8f, 0.4f), 1.f },
		{ TEXT("Star.Class.K"), FLinearColor(1.f, 0.55f, 0.22f), 0.85f },
		{ TEXT("Star.Class.M"), FLinearColor(1.f, 0.3f, 0.15f), 0.7f },
	};

	void ApplyStarClass(FGameplayTag StarClass, FCosmicSystemLook& Look)
	{
		for (const FStarClassLook& Each : StarClasses)
		{
			if (StarClass == FGameplayTag::RequestGameplayTag(FName(Each.Tag), false))
			{
				Look.StarColor = Each.Color;
				Look.StarScale = Each.Scale;
				return;
			}
		}
	}

	// The progress of a slot as a share of what its row needs; -1 for an empty slot or a row the tables do not hold. A
	// compare, so a progress that is not a number reads as 0.
	float ShareOf(const FCosmicDataTables& Tables, FName RowName, float Progress)
	{
		const FMegaprojectDef* Row = RowName.IsNone() ? nullptr : Tables.FindMegaproject(RowName);
		if (!Row)
		{
			return -1.f;
		}
		const float Share = Progress / CosmicMegaprojects::RequiredProgressOf(*Row);
		return Share > 0.f ? FMath::Min(Share, 1.f) : 0.f;
	}
}

FCosmicSystemLook CosmicMapLook::LookOf(const FGalaxyState& State, const FCosmicDataTables& Tables, int32 SystemIndex)
{
	using namespace CosmicMapLookImpl;

	FCosmicSystemLook Look;
	if (!State.Systems.IsValidIndex(SystemIndex))
	{
		return Look;
	}
	const FStarSystemState& System = State.Systems[SystemIndex];
	Look.Name = System.DisplayName;
	ApplyStarClass(System.StarClass, Look);
	if (CosmicHazards::IsDeadNode(System))
	{
		Look.bDead = true;
		return Look;
	}
	const FCivilizationState* Owner = System.OwnerCivID.IsNone() ? nullptr
		: State.Civs.FindByPredicate([&System](const FCivilizationState& Civ) { return Civ.CivID == System.OwnerCivID; });
	if (Owner)
	{
		Look.bOwned = true;
		Look.OwnerColor = Owner->Color;
	}
	Look.bSettling = System.ActiveMegaproject == CosmicRows::ProjectSettle;
	Look.bHazard = System.ActiveHazard.IsValid();
	Look.ConstructionShare = ShareOf(Tables, System.ActiveMegaproject, System.MegaprojectProgress);
	Look.ResponseShare = ShareOf(Tables, System.ResponseMegaproject, System.ResponseProgress);
	return Look;
}

TArray<TPair<int32, int32>> CosmicMapLook::LanesOf(const FGalaxyState& State)
{
	TMap<FName, int32> IndexOf;
	IndexOf.Reserve(State.Systems.Num());
	for (int32 Index = 0; Index < State.Systems.Num(); ++Index)
	{
		IndexOf.Add(State.Systems[Index].SystemID, Index);
	}
	// A lane listed by one end only is drawn too, and a lane listed by both ends once.
	TArray<TPair<int32, int32>> Lanes;
	TSet<uint64> Seen;
	for (int32 Index = 0; Index < State.Systems.Num(); ++Index)
	{
		const FStarSystemState& System = State.Systems[Index];
		if (CosmicHazards::IsDeadNode(System))
		{
			continue;
		}
		for (const FName Lane : System.Lanes)
		{
			const int32* Other = IndexOf.Find(Lane);
			if (!Other || *Other == Index || CosmicHazards::IsDeadNode(State.Systems[*Other]))
			{
				continue;
			}
			const int32 Low = FMath::Min(Index, *Other);
			const int32 High = FMath::Max(Index, *Other);
			bool bAlready = false;
			Seen.Add((static_cast<uint64>(Low) << 32) | static_cast<uint32>(High), &bAlready);
			if (!bAlready)
			{
				Lanes.Emplace(Low, High);
			}
		}
	}
	return Lanes;
}
