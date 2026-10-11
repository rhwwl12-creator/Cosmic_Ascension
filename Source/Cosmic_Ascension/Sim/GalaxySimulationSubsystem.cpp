// Cosmic Ascension - Sim layer.

#include "Sim/GalaxySimulationSubsystem.h"

#include "Cosmic_Ascension.h"
#include "Data/CosmicRowNames.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Log/CosmicLogSubsystem.h"
#include "Misc/Crc.h"
#include "Misc/Guid.h"
#include "Player/PlayerRoleSubsystem.h"
#include "UObject/EnumProperty.h"
#include "UObject/TextProperty.h"
#include "UObject/UnrealType.h"

namespace
{
	// Research fields (section 5.4). Sim code walks this array and never the Research maps themselves (section 9.2).
	const FName ResearchFields[] = { FName(TEXT("Physics")), FName(TEXT("Society")), FName(TEXT("Engineering")), FName(TEXT("Transcendence")) };

	// One DT_LogTemplates row per field for its breakthroughs (section 8.2).
	static_assert(UE_ARRAY_COUNT(CosmicRows::LogBreakthrough) == UE_ARRAY_COUNT(ResearchFields));

	// One breakthrough opens at each of these progress marks (section 5.4).
	constexpr float BreakthroughThresholds[] = { 0.25f, 0.5f, 0.75f, 1.0f };

	// Home system of the first life (section 5.10).
	const FName PlayerHomeSystemID(TEXT("sol"));

	// Star system actors are budgeted at 150 (section 9.4).
	constexpr int32 MaxSystems = 150;
	constexpr int32 MaxLanesPerSystem = 4;

	// Star classes from the hottest to the coolest.
	const FName StarClassNames[] = {
		FName(TEXT("Star.Class.O")), FName(TEXT("Star.Class.B")), FName(TEXT("Star.Class.A")), FName(TEXT("Star.Class.F")),
		FName(TEXT("Star.Class.G")), FName(TEXT("Star.Class.K")), FName(TEXT("Star.Class.M"))
	};

	// Provisional numbers. Section 5.1 fixes what each resource depends on; the values belong to the balance pass (section 9.3).
	namespace Economy
	{
		constexpr float TierMultiplier[] = { 1.0f, 1.5f, 2.2f, 3.0f, 3.0f };
		static_assert(static_cast<int32>(UE_ARRAY_COUNT(TierMultiplier)) == static_cast<int32>(EAscensionTier::Tier5_EternalOrder) + 1);

		// Energy of one owned system by star class, in the order of StarClassNames. G is the baseline.
		constexpr float StarClassEnergy[] = { 2.0f, 1.7f, 1.4f, 1.2f, 1.0f, 0.8f, 0.6f };
		static_assert(UE_ARRAY_COUNT(StarClassEnergy) == UE_ARRAY_COUNT(StarClassNames));

		constexpr float MaterialistEnergyBonus = 0.2f;
		constexpr float InsightPerPopulation = 1.0f;
		constexpr float SpiritualistInsightBonus = 0.2f;
		constexpr float InfluencePerPopulation = 0.5f;
		constexpr float OpenInfluenceBonus = 0.2f;
		constexpr float ResearchInsightShare = 0.5f;
		constexpr float ResearchPerInsight = 0.003f;
	}

	// Stability below this counts toward FCivilizationState.LowStabilityTicks (section 4.1).
	constexpr float LowStabilityThreshold = 0.3f;

	// Index for the tables with one entry per tier.
	int32 TierIndex(EAscensionTier Tier)
	{
		// An enumerator name missing from this build loads as the enum's _MAX value, one past the last tier.
		const int32 Max = static_cast<int32>(EAscensionTier::Tier5_EternalOrder);
		const int32 Index = static_cast<int32>(Tier);
		ensureMsgf(Index <= Max, TEXT("Invalid EAscensionTier %d"), Index);
		return FMath::Clamp(Index, 0, Max);
	}

	float StarEnergy(const FGameplayTag& StarClass)
	{
		const FName ClassName = StarClass.GetTagName();
		for (int32 Index = 0; Index < static_cast<int32>(UE_ARRAY_COUNT(StarClassNames)); ++Index)
		{
			if (StarClassNames[Index] == ClassName)
			{
				return Economy::StarClassEnergy[Index];
			}
		}
		return 1.f;
	}

	// Names made at galaxy creation are stored flattened, like log lines (section 8.5).
	FText Flatten(const FText& Text)
	{
		return FText::AsCultureInvariant(Text.ToString());
	}

	/**
	 * Canonical state hasher (section 9.2). Everything is reduced to bytes that do not depend on the process:
	 * names and tags as lower-cased strings, map entries in lexical key order, -0.0 folded into +0.0.
	 */
	struct FStateHasher
	{
		uint32 Hash = 0;

		void AddInt(int64 Value)
		{
			Hash = FCrc::MemCrc32(&Value, sizeof(Value), Hash);
		}

		void AddFloat(double Value)
		{
			// Fold -0.0 into +0.0 on the bits: a value equal to its default is not saved, so a -0.0 reloads as +0.0.
			uint64 Bits;
			FMemory::Memcpy(&Bits, &Value, sizeof(Bits));
			if ((Bits << 1) == 0)
			{
				Bits = 0;
			}
			Hash = FCrc::MemCrc32(&Bits, sizeof(Bits), Hash);
		}

		void AddName(FName Name)
		{
			FString Text = Name.ToString();
			Text.ToLowerInline();
			AddInt(Text.Len());
			Hash = FCrc::StrCrc32(*Text, Hash);
		}

		void AddStruct(const UStruct* Struct, const void* Data)
		{
			for (TFieldIterator<FProperty> It(Struct); It; ++It)
			{
				for (int32 Index = 0; Index < It->ArrayDim; ++Index)
				{
					AddProperty(*It, It->ContainerPtrToValuePtr<void>(Data, Index));
				}
			}
		}

		void AddProperty(const FProperty* Property, const void* Value)
		{
			if (CastField<FTextProperty>(Property))
			{
				return;
			}
			if (const FNameProperty* NameProperty = CastField<FNameProperty>(Property))
			{
				AddName(NameProperty->GetPropertyValue(Value));
				return;
			}
			if (const FBoolProperty* BoolProperty = CastField<FBoolProperty>(Property))
			{
				AddInt(BoolProperty->GetPropertyValue(Value) ? 1 : 0);
				return;
			}
			if (const FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
			{
				AddInt(EnumProperty->GetUnderlyingProperty()->GetSignedIntPropertyValue(Value));
				return;
			}
			if (const FNumericProperty* NumericProperty = CastField<FNumericProperty>(Property))
			{
				if (NumericProperty->IsFloatingPoint())
				{
					AddFloat(NumericProperty->GetFloatingPointPropertyValue(Value));
				}
				else
				{
					AddInt(NumericProperty->GetSignedIntPropertyValue(Value));
				}
				return;
			}
			if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
			{
				if (StructProperty->Struct == FGameplayTag::StaticStruct())
				{
					AddName(static_cast<const FGameplayTag*>(Value)->GetTagName());
				}
				else if (StructProperty->Struct == FGameplayTagContainer::StaticStruct())
				{
					// Explicit tags only: the parent tags are derived and rebuilt on load.
					const TArray<FGameplayTag>& Tags = static_cast<const FGameplayTagContainer*>(Value)->GetGameplayTagArray();
					AddInt(Tags.Num());
					for (const FGameplayTag& Tag : Tags)
					{
						AddName(Tag.GetTagName());
					}
				}
				else
				{
					AddStruct(StructProperty->Struct, Value);
				}
				return;
			}
			if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
			{
				FScriptArrayHelper Helper(ArrayProperty, Value);
				AddInt(Helper.Num());
				for (int32 Index = 0; Index < Helper.Num(); ++Index)
				{
					AddProperty(ArrayProperty->Inner, Helper.GetRawPtr(Index));
				}
				return;
			}
			if (const FMapProperty* MapProperty = CastField<FMapProperty>(Property))
			{
				const FNameProperty* KeyProperty = CastField<FNameProperty>(MapProperty->KeyProp);
				if (!ensureMsgf(KeyProperty, TEXT("ComputeStateHash: map %s is not keyed by FName"), *Property->GetName()))
				{
					return;
				}

				// Map order differs between a running and a loaded game, so entries are hashed in lexical key order.
				FScriptMapHelper Helper(MapProperty, Value);
				TArray<TPair<FName, const uint8*>> Entries;
				Entries.Reserve(Helper.Num());
				for (int32 Index = 0; Index < Helper.GetMaxIndex(); ++Index)
				{
					if (Helper.IsValidIndex(Index))
					{
						Entries.Emplace(KeyProperty->GetPropertyValue(Helper.GetKeyPtr(Index)), Helper.GetValuePtr(Index));
					}
				}
				Entries.Sort([](const TPair<FName, const uint8*>& A, const TPair<FName, const uint8*>& B)
				{
					return FNameLexicalLess()(A.Key, B.Key);
				});

				AddInt(Entries.Num());
				for (const TPair<FName, const uint8*>& Entry : Entries)
				{
					AddName(Entry.Key);
					AddProperty(MapProperty->ValueProp, Entry.Value);
				}
				return;
			}

			ensureMsgf(false, TEXT("ComputeStateHash: unsupported property type %s on %s"), *Property->GetClass()->GetName(), *Property->GetName());
		}
	};
}

void UGalaxySimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FCosmicDataTables Loaded;
	Loaded.LoadFromFixedPaths();
	SetTables(Loaded);

	UE_LOG(LogCosmic, Log, TEXT("UGalaxySimulationSubsystem initialized"));
	SetTickableTickType(ETickableTickType::Conditional);
}

void UGalaxySimulationSubsystem::Deinitialize()
{
	SetTickableTickType(ETickableTickType::Never);
	bRunning = false;
	UE_LOG(LogCosmic, Log, TEXT("UGalaxySimulationSubsystem deinitialized"));
	Super::Deinitialize();
}

void UGalaxySimulationSubsystem::SetTables(const FCosmicDataTables& InTables)
{
	Tables = InTables;
	const FCosmicRulesDef* RulesRow = Tables.FindRules();
	Rules = RulesRow ? *RulesRow : FCosmicRulesDef();
}

void UGalaxySimulationSubsystem::Tick(float DeltaTime)
{
	Accumulator = FMath::Max(0.f, Accumulator + DeltaTime * TimeScale);

	// Re-checked every step: a pause, NewGalaxy or LoadState issued from inside a tick ends the frame's work.
	int32 Steps = 0;
	while (Accumulator >= SecondsPerTick && Steps < MaxTicksPerFrame && bRunning && TimeScale > 0.f)
	{
		Accumulator -= SecondsPerTick;
		SimulateTick();
		++Steps;
	}

	// What is left carries over, but never more than one frame's budget: a long hitch must not turn into a catch-up spiral.
	Accumulator = FMath::Min(Accumulator, MaxTicksPerFrame * SecondsPerTick);
}

void UGalaxySimulationSubsystem::NewGalaxy(int32 Seed, int32 SystemCount, int32 CivCount)
{
	SystemCount = FMath::Clamp(SystemCount, 1, MaxSystems);
	CivCount = FMath::Clamp(CivCount, 1, SystemCount);

	State = FGalaxyState();
	State.SaveVersion = FGalaxyState::CurrentSaveVersion;
	State.Seed = Seed;
	// Slot identity, deliberately not drawn from the sim stream (section 9.2).
	State.GalaxyID = FGuid::NewGuid();
	Rng.Initialize(Seed);

	const FGameplayTag AIProfiles[] = {
		FGameplayTag::RequestGameplayTag(FName(TEXT("AI.Profile.Conservative"))),
		FGameplayTag::RequestGameplayTag(FName(TEXT("AI.Profile.Expansionist"))),
		FGameplayTag::RequestGameplayTag(FName(TEXT("AI.Profile.Scholarly"))),
		FGameplayTag::RequestGameplayTag(FName(TEXT("AI.Profile.Militarist"))),
	};

	// Systems on a disc. System 0 is the player's home.
	const float DiscRadius = 1000.f * FMath::Sqrt(static_cast<float>(SystemCount));
	State.Systems.Reserve(SystemCount);
	for (int32 Index = 0; Index < SystemCount; ++Index)
	{
		FStarSystemState& System = State.Systems.AddDefaulted_GetRef();
		System.SystemID = (Index == 0) ? PlayerHomeSystemID : FName(*FString::Printf(TEXT("sys_%03d"), Index));

		const float Angle = Rng.FRand() * UE_TWO_PI;
		const float Distance = FMath::Sqrt(Rng.FRand()) * DiscRadius;
		System.Position = FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0);
		System.StarClass = FGameplayTag::RequestGameplayTag(StarClassNames[Rng.RandRange(0, static_cast<int32>(UE_ARRAY_COUNT(StarClassNames)) - 1)]);

		// Drawn for every system so the stream does not shift. The player's home stays ordinary: no numeric edge (section 5.11).
		const float Richness = 0.5f + Rng.FRand();
		System.ResourceRichness = (Index == 0) ? 1.f : Richness;
	}

	auto AddLane = [this](int32 A, int32 B)
	{
		FStarSystemState& SystemA = State.Systems[A];
		FStarSystemState& SystemB = State.Systems[B];
		SystemA.Lanes.Add(SystemB.SystemID);
		SystemB.Lanes.Add(SystemA.SystemID);
	};

	// Nearest system among the first Limit systems that is not From and not already linked to it. Ties keep the lowest index.
	auto FindNearest = [this](int32 From, int32 Limit, bool bNeedsFreeLane) -> int32
	{
		const FStarSystemState& Origin = State.Systems[From];
		int32 Best = INDEX_NONE;
		double BestDistance = 0.0;
		for (int32 Index = 0; Index < Limit; ++Index)
		{
			const FStarSystemState& Other = State.Systems[Index];
			if (Index == From || Origin.Lanes.Contains(Other.SystemID) || (bNeedsFreeLane && Other.Lanes.Num() >= MaxLanesPerSystem))
			{
				continue;
			}
			const double Distance = FVector::DistSquared(Origin.Position, Other.Position);
			if (Best == INDEX_NONE || Distance < BestDistance)
			{
				Best = Index;
				BestDistance = Distance;
			}
		}
		return Best;
	};

	// A spanning tree first, so every system can be reached: each system links to the nearest earlier one.
	for (int32 Index = 1; Index < SystemCount; ++Index)
	{
		AddLane(Index, FindNearest(Index, Index, false));
	}
	// Then one more lane per system to its nearest unlinked neighbour, while both ends have room.
	for (int32 Index = 0; Index < SystemCount; ++Index)
	{
		if (State.Systems[Index].Lanes.Num() < MaxLanesPerSystem)
		{
			const int32 Neighbour = FindNearest(Index, SystemCount, true);
			if (Neighbour != INDEX_NONE)
			{
				AddLane(Index, Neighbour);
			}
		}
	}

	// AI archetypes: the DT_Civs rows with a positive Weight, in lexical row order so the draw never depends on the
	// table's own order (section 9.2). A row may be drawn more than once.
	TArray<FName> ArchetypeRows;
	TArray<float> ArchetypeWeights;
	float TotalWeight = 0.f;
	for (const FName RowName : Tables.GetCivRowNames())
	{
		const FCivArchetypeDef* Row = Tables.FindCiv(RowName);
		if (Row && Row->Weight > 0.f)
		{
			ArchetypeRows.Add(RowName);
			ArchetypeWeights.Add(Row->Weight);
			TotalWeight += Row->Weight;
		}
	}

	// Civilizations. Civ 0 is the player civ on system 0; AI civs draw distinct home systems from the rest.
	TArray<int32> FreeHomes;
	FreeHomes.Reserve(SystemCount - 1);
	for (int32 Index = 1; Index < SystemCount; ++Index)
	{
		FreeHomes.Add(Index);
	}

	State.Civs.Reserve(CivCount);
	for (int32 CivIndex = 0; CivIndex < CivCount; ++CivIndex)
	{
		const bool bPlayerCiv = (CivIndex == 0);
		int32 HomeIndex = 0;
		FName ArchetypeRow = bPlayerCiv ? Rules.PlayerArchetypeRow : CosmicRows::ArchDefault;
		if (!bPlayerCiv)
		{
			const int32 Pick = Rng.RandRange(0, FreeHomes.Num() - 1);
			HomeIndex = FreeHomes[Pick];
			FreeHomes.RemoveAtSwap(Pick);

			if (!ArchetypeRows.IsEmpty())
			{
				float Roll = Rng.FRand() * TotalWeight;
				int32 RowIndex = 0;
				while (RowIndex < ArchetypeRows.Num() - 1 && Roll >= ArchetypeWeights[RowIndex])
				{
					Roll -= ArchetypeWeights[RowIndex];
					++RowIndex;
				}
				ArchetypeRow = ArchetypeRows[RowIndex];
			}
		}
		const FCivArchetypeDef* Archetype = Tables.FindCiv(ArchetypeRow);
		FStarSystemState& Home = State.Systems[HomeIndex];

		FCivilizationState& Civ = State.Civs.AddDefaulted_GetRef();
		Civ.CivID = FName(*FString::Printf(TEXT("civ_%02d"), CivIndex));
		Civ.HomeSystemID = Home.SystemID;
		Civ.ArchetypeRow = ArchetypeRow;
		Civ.Color = FLinearColor::MakeFromHSV8(static_cast<uint8>((CivIndex * 255) / CivCount), 180, 255);
		if (Archetype)
		{
			Civ.Traits = Archetype->Traits;
			Civ.Ethics = Archetype->EthicsPreset;
			Civ.AIProfile = Archetype->AIProfile;
			if (bPlayerCiv)
			{
				Civ.Color = Archetype->Color;
			}
		}
		if (!bPlayerCiv)
		{
			if (ArchetypeRow == CosmicRows::ArchDefault)
			{
				Civ.Ethics.Pacifism = Rng.FRand() * 2.f - 1.f;
				Civ.Ethics.Spiritualism = Rng.FRand() * 2.f - 1.f;
				Civ.Ethics.Openness = Rng.FRand() * 2.f - 1.f;
			}
			if (!Civ.AIProfile.IsValid())
			{
				Civ.AIProfile = AIProfiles[Rng.RandRange(0, static_cast<int32>(UE_ARRAY_COUNT(AIProfiles)) - 1)];
			}
		}
		if (Archetype)
		{
			if (Archetype->HomeStarClass.IsValid())
			{
				Home.StarClass = Archetype->HomeStarClass;
			}
			if (!Archetype->HomeSystemNames.IsEmpty())
			{
				Home.DisplayName = Flatten(Archetype->HomeSystemNames[Rng.RandRange(0, Archetype->HomeSystemNames.Num() - 1)]);
			}
			if (!Archetype->NameBank.IsEmpty())
			{
				// Name patterns may call the home system by name.
				FFormatNamedArguments NameArgs;
				NameArgs.Add(TEXT("Star"), Home.DisplayName);
				Civ.CivName = Flatten(FText::Format(Archetype->NameBank[Rng.RandRange(0, Archetype->NameBank.Num() - 1)], NameArgs));
			}
		}
		for (const FName Field : ResearchFields)
		{
			Civ.Research.Add(Field, 0.f);
			Civ.ResearchFocus.Add(Field, 1.f / static_cast<float>(UE_ARRAY_COUNT(ResearchFields)));
		}
		Civ.OwnedSystems.Add(Home.SystemID);
		Home.OwnerCivID = Civ.CivID;
	}

	// The first life: ruler of the player civ, through the same function succession uses (section 5.10).
	UPlayerRoleSubsystem* PlayerRole = GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
	check(PlayerRole);
	PlayerRole->Install(PlayerHomeSystemID, State.Civs[0].CivID, EPlayerRole::Ruler);

	SyncRngState();
	Accumulator = 0.f;
	bRunning = true;

	UE_LOG(LogCosmic, Log, TEXT("NewGalaxy: seed %d, %d systems, %d civs"), Seed, SystemCount, CivCount);
	if (bBroadcastEnabled)
	{
		OnGalaxyGenerated.Broadcast(Seed);
	}
}

void UGalaxySimulationSubsystem::SetTimeScale(float Scale)
{
	TimeScale = FMath::Clamp(Scale, 0.f, static_cast<float>(MaxTicksPerFrame));
}

void UGalaxySimulationSubsystem::StepOnce()
{
	SimulateTick();
}

void UGalaxySimulationSubsystem::RunHeadless(int64 Ticks)
{
	TGuardValue<bool> GateGuard(bBroadcastEnabled, false);
	for (int64 Index = 0; Index < Ticks; ++Index)
	{
		SimulateTick();
		// Section 4.4: once endings exist, the ending conditions are checked here every tick and stop the run early.
	}
}

void UGalaxySimulationSubsystem::SyncRngState()
{
	State.RngState = Rng.GetCurrentSeed();
}

EPopulationBand UGalaxySimulationSubsystem::GetPopulationBand(FName CivID) const
{
	const FCivilizationState* Civ = FindCiv(CivID);
	if (!Civ)
	{
		return EPopulationBand::Few;
	}

	// Baseline by civ Tier: T1 billions, T2 hundred billions, T3 trillions, T4 and T5 countless (section 8.3).
	static constexpr EPopulationBand BaselineByTier[] = {
		EPopulationBand::Billions, EPopulationBand::HundredBillions, EPopulationBand::Trillions, EPopulationBand::Countless, EPopulationBand::Countless
	};
	static_assert(static_cast<int32>(UE_ARRAY_COUNT(BaselineByTier)) == static_cast<int32>(EAscensionTier::Tier5_EternalOrder) + 1);
	int32 Band = static_cast<int32>(BaselineByTier[TierIndex(Civ->Tier)]);

	// Thresholds are ascending and exactly three (Data.Integrity). A malformed row leaves the baseline untouched.
	const TArray<float>& Thresholds = Rules.PopulationBandThresholds;
	if (Thresholds.Num() == 3)
	{
		if (Civ->Population < Thresholds[0])
		{
			return EPopulationBand::Few;
		}
		if (Civ->Population < Thresholds[1])
		{
			Band = FMath::Max(Band - 1, static_cast<int32>(EPopulationBand::Few));
		}
		else if (Civ->Population > Thresholds[2])
		{
			Band = FMath::Min(Band + 1, static_cast<int32>(EPopulationBand::Countless));
		}
	}
	return static_cast<EPopulationBand>(Band);
}

void UGalaxySimulationSubsystem::LoadState(const FGalaxyState& In)
{
	State = In;
	Rng.Initialize(State.RngState);
	Accumulator = 0.f;
	bRunning = true;

	if (bBroadcastEnabled)
	{
		OnGalaxyGenerated.Broadcast(State.Seed);
	}
}

uint32 UGalaxySimulationSubsystem::ComputeStateHash() const
{
	static const FName GalaxyIDName = GET_MEMBER_NAME_CHECKED(FGalaxyState, GalaxyID);
	static const FName RngStateName = GET_MEMBER_NAME_CHECKED(FGalaxyState, RngState);

	FStateHasher Hasher;
	for (TFieldIterator<FProperty> It(FGalaxyState::StaticStruct()); It; ++It)
	{
		const FName PropertyName = It->GetFName();
		if (PropertyName == GalaxyIDName)
		{
			continue;
		}
		if (PropertyName == RngStateName)
		{
			// The live stream position, so the hash is right even between two SyncRngState calls.
			Hasher.AddInt(Rng.GetCurrentSeed());
			continue;
		}
		for (int32 Index = 0; Index < It->ArrayDim; ++Index)
		{
			Hasher.AddProperty(*It, It->ContainerPtrToValuePtr<void>(&State, Index));
		}
	}
	return Hasher.Hash;
}

void UGalaxySimulationSubsystem::SimulateTick()
{
	// Tick 0 is the generated state: no event raised inside a tick stage carries tick 0, so the "0 = unset" tick fields
	// stay unambiguous there. Player input outside the tick can still run at tick 0.
	++State.Tick;

	TickEconomy();
	TickResearch();
	TickDiplomacy();
	TickConflict();
	TickHazards();
	TickCivAI();
	// The calling rests on no vessel during the interregnum leap (section 4.4).
	if (State.Calling.LifeState == ELifeState::Embodied)
	{
		TickPlayerTravel();
	}
	ResolvePendingRoleEffect();

	SyncRngState();
	if (bBroadcastEnabled)
	{
		OnSimTick.Broadcast(State.Tick);
	}
}

void UGalaxySimulationSubsystem::TickEconomy()
{
	for (FCivilizationState& Civ : State.Civs)
	{
		float Richness = 0.f;
		float StarOutput = 0.f;
		for (const FName SystemID : Civ.OwnedSystems)
		{
			if (const FStarSystemState* System = FindSystem(SystemID))
			{
				Richness += System->ResourceRichness;
				StarOutput += StarEnergy(System->StarClass);
			}
		}

		// Energy comes from the stars owned, Matter from the richness of the systems, Insight and Influence from the
		// people (section 5.1). Causality stays at zero: it opens with Tier 3 research.
		const float TierMultiplier = Economy::TierMultiplier[TierIndex(Civ.Tier)];
		FResourcePool Income;
		Income.Energy = StarOutput * TierMultiplier * (1.f + Economy::MaterialistEnergyBonus * FMath::Max(0.f, -Civ.Ethics.Spiritualism));
		Income.Matter = Richness * TierMultiplier;
		Income.Insight = Civ.Population * Economy::InsightPerPopulation * (1.f + Economy::SpiritualistInsightBonus * FMath::Max(0.f, Civ.Ethics.Spiritualism));
		Income.Influence = Civ.Population * Economy::InfluencePerPopulation * (1.f + Economy::OpenInfluenceBonus * FMath::Max(0.f, Civ.Ethics.Openness));

		Civ.IncomePerTick = Income;
		Civ.Stockpile.Energy += Income.Energy;
		Civ.Stockpile.Matter += Income.Matter;
		Civ.Stockpile.Insight += Income.Insight;
		Civ.Stockpile.Influence += Income.Influence;

		Civ.LowStabilityTicks = (Civ.Stability < LowStabilityThreshold) ? Civ.LowStabilityTicks + 1 : 0;
	}
}

void UGalaxySimulationSubsystem::TickResearch()
{
	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Log);

	for (FCivilizationState& Civ : State.Civs)
	{
		const float Spent = FMath::Max(0.f, FMath::Min(Civ.Stockpile.Insight, Civ.IncomePerTick.Insight * Economy::ResearchInsightShare));
		Civ.Stockpile.Insight -= Spent;

		for (int32 FieldIndex = 0; FieldIndex < static_cast<int32>(UE_ARRAY_COUNT(ResearchFields)); ++FieldIndex)
		{
			const FName Field = ResearchFields[FieldIndex];
			const float Before = Civ.Research.FindRef(Field);
			const float After = FMath::Min(1.f, Before + Spent * Civ.ResearchFocus.FindRef(Field) * Economy::ResearchPerInsight);
			Civ.Research.Add(Field, After);

			// A breakthrough is told once per mark crossed. A civ without a display name cannot be told of (section 8.2).
			if (Civ.CivName.IsEmpty())
			{
				continue;
			}
			for (const float Threshold : BreakthroughThresholds)
			{
				if (Before < Threshold && After >= Threshold)
				{
					FFormatNamedArguments Args;
					Args.Add(TEXT("Civ"), Civ.CivName);
					Log->PushRow(CosmicRows::LogBreakthrough[FieldIndex], Civ.bIsPlayerRuled ? 1 : 0, Args, Civ.HomeSystemID);
				}
			}
		}
	}
}

void UGalaxySimulationSubsystem::TickDiplomacy()
{
	// Relations drift, stances and treaties (section 5.5).
}

void UGalaxySimulationSubsystem::TickConflict()
{
	// Fleet power comparison and system ownership changes (section 5.6).
}

void UGalaxySimulationSubsystem::TickHazards()
{
	// Hazard spawn, countdown, expiry and resolution; Hazard.Claim (section 4.1, 4.2).
}

void UGalaxySimulationSubsystem::TickCivAI()
{
	// Policy profiles of AI civs and regents (section 3.1).
}

void UGalaxySimulationSubsystem::TickPlayerTravel()
{
	FPlayerEntityState& Player = State.Player;
	if (Player.TravelTicksRemaining > 0 && --Player.TravelTicksRemaining == 0)
	{
		Player.CurrentSystemID = Player.TravelTargetSystemID;
		Player.TravelTargetSystemID = NAME_None;
	}

	// The open reign of the current life counts one more tick (section 7.2).
	if (Player.Role == EPlayerRole::Ruler && !State.Calling.Lives.IsEmpty())
	{
		FLifeRecord& Life = State.Calling.Lives.Last();
		if (!Life.ReignTicks.IsEmpty() && !Life.ReignEndReasons.IsEmpty() && !Life.ReignEndReasons.Last().IsValid())
		{
			++Life.ReignTicks.Last();
		}
	}
}

void UGalaxySimulationSubsystem::ResolvePendingRoleEffect()
{
	// Death and deposition recorded in Calling.PendingEffect start their pipeline here, at the tick boundary (section 4.4, 5.10).
}

const FCivilizationState* UGalaxySimulationSubsystem::FindCiv(FName CivID) const
{
	return State.Civs.FindByPredicate([CivID](const FCivilizationState& Civ) { return Civ.CivID == CivID; });
}

const FStarSystemState* UGalaxySimulationSubsystem::FindSystem(FName SystemID) const
{
	return State.Systems.FindByPredicate([SystemID](const FStarSystemState& System) { return System.SystemID == SystemID; });
}
