// Cosmic Ascension - Sim layer.

#include "Sim/GalaxySimulationSubsystem.h"

#include "Cosmic_Ascension.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
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

	// Home system of the first life (section 5.10).
	const FName PlayerHomeSystemID(TEXT("sol"));

	// Archetype of every AI civ until NewGalaxy can draw from DT_Civs.
	const FName DefaultArchetypeRow(TEXT("arch_default"));

	// Star system actors are budgeted at 150 (section 9.4).
	constexpr int32 MaxSystems = 150;
	constexpr int32 MaxLanesPerSystem = 4;

	// Provisional numbers. Section 5.1 fixes what each resource depends on; the values belong to the balance pass (section 9.3).
	namespace Economy
	{
		constexpr float TierMultiplier[] = { 1.0f, 1.5f, 2.2f, 3.0f, 3.0f };
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
			if (Value == 0.0)
			{
				Value = 0.0;
			}
			Hash = FCrc::MemCrc32(&Value, sizeof(Value), Hash);
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
				AddProperty(*It, It->ContainerPtrToValuePtr<void>(Data));
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

void UGalaxySimulationSubsystem::Tick(float DeltaTime)
{
	Accumulator += DeltaTime * TimeScale;

	const int32 Steps = FMath::Min(FMath::FloorToInt32(Accumulator / SecondsPerTick), MaxTicksPerFrame);
	for (int32 Step = 0; Step < Steps; ++Step)
	{
		SimulateTick();
	}

	// What is left carries over, but never more than one frame's budget: a long hitch must not turn into a catch-up spiral.
	Accumulator = FMath::Min(Accumulator - Steps * SecondsPerTick, MaxTicksPerFrame * SecondsPerTick);
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

	const FGameplayTag StarClasses[] = {
		FGameplayTag::RequestGameplayTag(FName(TEXT("Star.Class.O"))),
		FGameplayTag::RequestGameplayTag(FName(TEXT("Star.Class.B"))),
		FGameplayTag::RequestGameplayTag(FName(TEXT("Star.Class.A"))),
		FGameplayTag::RequestGameplayTag(FName(TEXT("Star.Class.F"))),
		FGameplayTag::RequestGameplayTag(FName(TEXT("Star.Class.G"))),
		FGameplayTag::RequestGameplayTag(FName(TEXT("Star.Class.K"))),
		FGameplayTag::RequestGameplayTag(FName(TEXT("Star.Class.M"))),
	};
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
		System.StarClass = StarClasses[Rng.RandRange(0, static_cast<int32>(UE_ARRAY_COUNT(StarClasses)) - 1)];
		System.ResourceRichness = 0.5f + Rng.FRand();
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
		if (!bPlayerCiv)
		{
			const int32 Pick = Rng.RandRange(0, FreeHomes.Num() - 1);
			HomeIndex = FreeHomes[Pick];
			FreeHomes.RemoveAtSwap(Pick);
		}
		FStarSystemState& Home = State.Systems[HomeIndex];

		FCivilizationState& Civ = State.Civs.AddDefaulted_GetRef();
		Civ.CivID = FName(*FString::Printf(TEXT("civ_%02d"), CivIndex));
		Civ.HomeSystemID = Home.SystemID;
		Civ.ArchetypeRow = bPlayerCiv ? Rules.PlayerArchetypeRow : DefaultArchetypeRow;
		Civ.Color = FLinearColor::MakeFromHSV8(static_cast<uint8>((CivIndex * 255) / CivCount), 180, 255);
		if (!bPlayerCiv)
		{
			Civ.Ethics.Pacifism = Rng.FRand() * 2.f - 1.f;
			Civ.Ethics.Spiritualism = Rng.FRand() * 2.f - 1.f;
			Civ.Ethics.Openness = Rng.FRand() * 2.f - 1.f;
			Civ.AIProfile = AIProfiles[Rng.RandRange(0, static_cast<int32>(UE_ARRAY_COUNT(AIProfiles)) - 1)];
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
	int32 Band = static_cast<int32>(BaselineByTier[static_cast<int32>(Civ->Tier)]);

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
		Hasher.AddProperty(*It, It->ContainerPtrToValuePtr<void>(&State));
	}
	return Hasher.Hash;
}

void UGalaxySimulationSubsystem::SimulateTick()
{
	// Tick 0 is the generated state, so no event ever carries tick 0 and the "0 = unset" tick fields stay unambiguous.
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
		for (const FName SystemID : Civ.OwnedSystems)
		{
			if (const FStarSystemState* System = FindSystem(SystemID))
			{
				Richness += System->ResourceRichness;
			}
		}

		// Causality stays at zero: it opens with Tier 3 research (section 5.1).
		const float TierMultiplier = Economy::TierMultiplier[static_cast<int32>(Civ.Tier)];
		FResourcePool Income;
		Income.Energy = Richness * TierMultiplier * (1.f + Economy::MaterialistEnergyBonus * FMath::Max(0.f, -Civ.Ethics.Spiritualism));
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
	for (FCivilizationState& Civ : State.Civs)
	{
		const float Spent = FMath::Min(Civ.Stockpile.Insight, Civ.IncomePerTick.Insight * Economy::ResearchInsightShare);
		Civ.Stockpile.Insight -= Spent;

		for (const FName Field : ResearchFields)
		{
			const float Focus = Civ.ResearchFocus.FindRef(Field);
			float& Progress = Civ.Research.FindOrAdd(Field);
			Progress = FMath::Min(1.f, Progress + Spent * Focus * Economy::ResearchPerInsight);
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
