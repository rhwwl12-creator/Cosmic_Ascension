// Cosmic Ascension - Sim layer.

#include "Sim/GalaxySimulationSubsystem.h"

#include "Containers/ArrayView.h"
#include "Containers/BitArray.h"
#include "Core/CosmicSeed.h"
#include "Cosmic_Ascension.h"
#include "Data/CosmicRowNames.h"
#include "Engine/GameInstance.h"
#include "Events/CosmicEventSubsystem.h"
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

	// At most this many fallen transcendents are active in a run (section 4.2).
	constexpr int32 MaxFallenRoster = 3;

	// Index for the tables with one entry per tier.
	int32 TierIndex(EAscensionTier Tier)
	{
		// An enumerator name missing from this build loads as the enum's _MAX value, one past the last tier.
		const int32 Max = static_cast<int32>(EAscensionTier::Tier5_EternalOrder);
		const int32 Index = static_cast<int32>(Tier);
		ensureMsgf(Index <= Max, TEXT("Invalid EAscensionTier %d"), Index);
		return FMath::Clamp(Index, 0, Max);
	}

	// Hue wheel colour of an AI civ (saturation 180/255, value 1). The same arithmetic as FLinearColor::MakeFromHSV8, but
	// compiled in this module: Core follows the floating point mode of the target (fast in game targets) and Color is
	// hashed, so a hashed float never comes out of an engine function that is not inline (section 9.2).
	FLinearColor CivWheelColor(uint8 Hue)
	{
		constexpr float Saturation = 180.f / 255.f;
		const float HueDiv60 = static_cast<float>(Hue) * (6.f / 256.f);
		const int32 Sector = static_cast<int32>(HueDiv60);
		const float Fraction = HueDiv60 - static_cast<float>(Sector);
		const float P = 1.f - Saturation;
		const float Q = 1.f - Fraction * Saturation;
		const float T = 1.f - (1.f - Fraction) * Saturation;
		switch (Sector)
		{
		case 0: return FLinearColor(1.f, T, P);
		case 1: return FLinearColor(Q, 1.f, P);
		case 2: return FLinearColor(P, 1.f, T);
		case 3: return FLinearColor(P, Q, 1.f);
		case 4: return FLinearColor(T, P, 1.f);
		default: return FLinearColor(1.f, P, Q);
		}
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

	// Weight of a pressure source by its tier (section 4.2). A table too short for the tier reads as 0.
	float PressureWeight(const FCosmicRulesDef& Rules, EAscensionTier Tier)
	{
		const int32 Index = TierIndex(Tier);
		return Rules.PressureBySourceTier.IsValidIndex(Index) ? FMath::Max(0.f, Rules.PressureBySourceTier[Index]) : 0.f;
	}

	float ReachFactor(ECosmicSphereReach Reach, const FCosmicRulesDef& Rules)
	{
		switch (Reach)
		{
		case ECosmicSphereReach::Inside:
			return 1.f;
		case ECosmicSphereReach::Adjacent:
			// The range of the knob is a lint; the guard is here. A comparison and not FMath::Clamp, which hands back the
			// upper bound for a value that is not a number: such a value reads as 0, like one below zero.
			return Rules.PressureAdjacencyFalloff > 0.f ? FMath::Min(Rules.PressureAdjacencyFalloff, 1.f) : 0.f;
		default:
			return 0.f;
		}
	}

	// One term of the sum. A larger term takes the lead and an equal one never does, so the order of the calls is the
	// tie rule: transcendents in array order, then the civilization, then the wanderer.
	void AddPressureTerm(FCosmicPressure& Out, float& InOutLargest, float Term, ECosmicPressureSource Source, FName SourceID, EAscensionTier Tier)
	{
		if (!(Term > 0.f))
		{
			return;
		}
		Out.P += Term;
		if (Term > InOutLargest)
		{
			InOutLargest = Term;
			Out.DominantSource = Source;
			Out.DominantSourceID = SourceID;
			Out.DominantTier = Tier;
		}
	}

	// The rows of the transcendents that press, along State.Transcendents: null for a sealed one and for one whose row the
	// tables do not hold.
	using FPressingDefs = TArray<const FTranscendentDef*, TInlineAllocator<4>>;

	FPressingDefs PressingDefsOf(const FGalaxyState& InState, const FCosmicDataTables& InTables)
	{
		FPressingDefs Defs;
		Defs.Reserve(InState.Transcendents.Num());
		for (const FTranscendentState& Transcendent : InState.Transcendents)
		{
			Defs.Add(Transcendent.bSealed ? nullptr : InTables.FindTranscendent(Transcendent.TranscendentID));
		}
		return Defs;
	}

	/**
	 * The pressure on one system (section 4.2), with the rows of the pressing transcendents handed in as Defs (along
	 * Transcendents, see PressingDefsOf) and the reach of every transcendent as ReachOf(index into Transcendents).
	 * ComputePressureIn hands in SphereReachIn, the economy stage its table of the tick: one body, so both give the
	 * same number for the same state.
	 */
	template <typename ReachOfType>
	FCosmicPressure PressureWithReach(const FGalaxyState& InState, const FCosmicStateIndex& InIndex, const FCosmicRulesDef& InRules, TConstArrayView<const FTranscendentDef*> Defs, int32 SystemIndex, ReachOfType&& ReachOf)
	{
		FCosmicPressure Out;
		if (!InState.Systems.IsValidIndex(SystemIndex))
		{
			return Out;
		}
		const FStarSystemState& System = InState.Systems[SystemIndex];
		float Largest = 0.f;

		for (int32 TranscendentIndex = 0; TranscendentIndex < InState.Transcendents.Num(); ++TranscendentIndex)
		{
			const FTranscendentState& Transcendent = InState.Transcendents[TranscendentIndex];
			const FTranscendentDef* Def = Defs.IsValidIndex(TranscendentIndex) ? Defs[TranscendentIndex] : nullptr;
			if (!Def)
			{
				continue;
			}
			// Depth inside the tier (section 3.2). The range is a lint; the guard is here, and a value that is not a number
			// reads as 0.
			const float Depth = Def->Depth > 0.f ? FMath::Min(Def->Depth, 1.f) : 0.f;
			const float Term = PressureWeight(InRules, Def->Tier) * (1.f + Depth) * ReachFactor(ReachOf(TranscendentIndex), InRules);
			AddPressureTerm(Out, Largest, Term, ECosmicPressureSource::Transcendent, Transcendent.TranscendentID, Def->Tier);
		}

		// The owner presses with its own tier, whoever rules it.
		const int32 OwnerIndex = InIndex.FindCiv(InState, System.OwnerCivID);
		if (InState.Civs.IsValidIndex(OwnerIndex))
		{
			const FCivilizationState& Owner = InState.Civs[OwnerIndex];
			AddPressureTerm(Out, Largest, PressureWeight(InRules, Owner.Tier), ECosmicPressureSource::Civilization, Owner.CivID, Owner.Tier);
		}

		// A wanderer presses where it stands. During the interregnum Player still holds the dead vessel: it adds nothing.
		if (InState.Calling.LifeState == ELifeState::Embodied && InState.Player.Role == EPlayerRole::Wanderer && InState.Player.CurrentSystemID == System.SystemID)
		{
			AddPressureTerm(Out, Largest, PressureWeight(InRules, InState.Calling.PersonalTier), ECosmicPressureSource::Wanderer, NAME_None, InState.Calling.PersonalTier);
		}
		return Out;
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
	if (Tables.Rules && !RulesRow)
	{
		UE_LOG(LogCosmic, Error, TEXT("DT_Rules has no row '%s': running on the C++ defaults."), *CosmicRows::Rules.ToString());
	}
	Rules = RulesRow ? *RulesRow : FCosmicRulesDef();
}

void UGalaxySimulationSubsystem::Tick(float DeltaTime)
{
	Accumulator = FMath::Max(0.f, Accumulator + DeltaTime * TimeScale);

	// Re-checked every step: a pause, NewGalaxy or LoadState issued from an OnSimTick handler (the tick boundary) ends the
	// frame's work. Delegates fired inside a stage (OnLogEntry) must not replace the state.
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
	ensureMsgf(!bInTickStages, TEXT("NewGalaxy from inside a tick stage: issue it from OnSimTick or from input"));

	SystemCount = FMath::Clamp(SystemCount, 1, MaxSystems);
	CivCount = FMath::Clamp(CivCount, 1, SystemCount);

	State = FGalaxyState();
	State.SaveVersion = FGalaxyState::CurrentSaveVersion;
	State.Seed = Seed;
	// Slot identity, deliberately not drawn from the sim stream (section 9.2).
	State.GalaxyID = FGuid::NewGuid();
	Rng.Initialize(Seed);
	// The tell stream starts from a derived value, before Install tells the first line (section 9.2).
	State.TellRngState = static_cast<int32>(CosmicSeed::Derive(Seed, "", CosmicSeed::Purpose::Tell, 0));

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
		Civ.Color = CivWheelColor(static_cast<uint8>((CivIndex * 255) / CivCount));
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

	// The fallen roster (section 4.2). Silence is fixed; the others are drawn. Every sphere starts empty: the starting
	// systems of a sphere are placed by the generation change of Phase 2.
	{
		const FGameplayTag AbyssalTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Realm.Abyssal")));
		auto AddTranscendent = [this](FName RowName, const FTranscendentDef& Def)
		{
			FTranscendentState& Transcendent = State.Transcendents.AddDefaulted_GetRef();
			Transcendent.TranscendentID = RowName;
			Transcendent.Doctrine = Def.Doctrine;
		};

		// The range of the knob is a lint; the guard is here, with one line when it bites.
		const int32 Places = FMath::Clamp(Rules.FallenTranscendentCount, 0, MaxFallenRoster);
		if (Places != Rules.FallenTranscendentCount)
		{
			UE_LOG(LogCosmic, Error, TEXT("DT_Rules: FallenTranscendentCount %d is outside 0..%d, read as %d"), Rules.FallenTranscendentCount, MaxFallenRoster, Places);
		}

		const FTranscendentDef* Silence = Tables.FindTranscendent(CosmicRows::TransSilence);
		if (Places >= 1 && Silence && Silence->Allegiance == AbyssalTag)
		{
			AddTranscendent(CosmicRows::TransSilence, *Silence);
		}

		// Candidates in lexical row order, never the table's own (section 9.2).
		TArray<FName> Candidates;
		for (const FName RowName : Tables.GetTranscendentRowNames())
		{
			const FTranscendentDef* Row = Tables.FindTranscendent(RowName);
			// A weight that is zero, below zero, not a number or not finite is no candidate.
			if (Row && RowName != CosmicRows::TransSilence && Row->Allegiance == AbyssalTag && Row->SpawnWeight > 0.f && FMath::IsFinite(Row->SpawnWeight))
			{
				Candidates.Add(RowName);
			}
		}

		// One draw per remaining place of the roster, whatever the table holds: a draw without a candidate is thrown
		// away, so the number of draws follows the knob alone.
		for (int32 Place = 1; Place < Places; ++Place)
		{
			const float Roll = Rng.FRand();

			// A doctrine is active at most once.
			Candidates.RemoveAll([this](FName RowName)
			{
				const FTranscendentDef* Row = Tables.FindTranscendent(RowName);
				return State.Transcendents.ContainsByPredicate([Row](const FTranscendentState& Active) { return Active.Doctrine == Row->Doctrine; });
			});
			if (Candidates.IsEmpty())
			{
				continue;
			}

			float RosterWeight = 0.f;
			for (const FName RowName : Candidates)
			{
				RosterWeight += Tables.FindTranscendent(RowName)->SpawnWeight;
			}
			const float Mark = Roll * RosterWeight;
			int32 Picked = Candidates.Num() - 1;
			float Reached = 0.f;
			for (int32 CandidateIndex = 0; CandidateIndex < Candidates.Num(); ++CandidateIndex)
			{
				Reached += Tables.FindTranscendent(Candidates[CandidateIndex])->SpawnWeight;
				if (Mark < Reached)
				{
					Picked = CandidateIndex;
					break;
				}
			}
			const FName PickedRow = Candidates[Picked];
			Candidates.RemoveAt(Picked);
			AddTranscendent(PickedRow, *Tables.FindTranscendent(PickedRow));
		}
	}

	StateIndex.Rebuild(State);

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
	if (IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("StepOnce called from a handler of a Core Logic delegate: a handler may only read the state"));
		return;
	}
	SimulateTick();
}

void UGalaxySimulationSubsystem::RunHeadless(int64 Ticks)
{
	if (IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("RunHeadless called from a handler of a Core Logic delegate: a handler may only read the state"));
		return;
	}
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

FCosmicEvalContext UGalaxySimulationSubsystem::MakeEvalContext()
{
	if (IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("MakeEvalContext called from a handler of a Core Logic delegate: a handler may only read the state"));
	}
	return FCosmicEvalContext(State, Rng, StateIndex, Tables, Rules);
}

float UGalaxySimulationSubsystem::ComputeSpiritualPressure(FName SystemID, FName& OutDominantSourceID) const
{
	const FCosmicPressure Reading = ComputePressureIn(State, StateIndex, Tables, Rules, StateIndex.FindSystem(State, SystemID));
	OutDominantSourceID = Reading.DominantSourceID;
	return Reading.P;
}

FCosmicPressure UGalaxySimulationSubsystem::ComputePressureIn(const FGalaxyState& InState, const FCosmicStateIndex& InIndex, const FCosmicDataTables& InTables, const FCosmicRulesDef& InRules, int32 SystemIndex)
{
	const FPressingDefs Defs = PressingDefsOf(InState, InTables);
	return PressureWithReach(InState, InIndex, InRules, Defs, SystemIndex, [&InState, SystemIndex](int32 TranscendentIndex)
	{
		return SphereReachIn(InState, InState.Transcendents[TranscendentIndex], SystemIndex);
	});
}

ECosmicSphereReach UGalaxySimulationSubsystem::SphereReachIn(const FGalaxyState& InState, const FTranscendentState& Transcendent, int32 SystemIndex)
{
	if (!InState.Systems.IsValidIndex(SystemIndex))
	{
		return ECosmicSphereReach::None;
	}
	const FStarSystemState& System = InState.Systems[SystemIndex];
	if (Transcendent.InfluencedSystems.Contains(System.SystemID))
	{
		return ECosmicSphereReach::Inside;
	}
	for (const FName Lane : System.Lanes)
	{
		if (Transcendent.InfluencedSystems.Contains(Lane))
		{
			return ECosmicSphereReach::Adjacent;
		}
	}
	return ECosmicSphereReach::None;
}

uint32 UGalaxySimulationSubsystem::DrawTell(int32 Count)
{
	if (Count < 1)
	{
		UE_LOG(LogCosmic, Error, TEXT("DrawTell: Count %d is not a variant count"), Count);
		return 0;
	}

	// No live stream object: the saved value is the stream, so it is current at every save and every hash (section 9.2).
	FRandomStream Tell(State.TellRngState);
	const int32 Drawn = Tell.RandHelper(Count);
	State.TellRngState = Tell.GetCurrentSeed();
	return static_cast<uint32>(Drawn);
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
	ensureMsgf(!bInTickStages, TEXT("LoadState from inside a tick stage: issue it from OnSimTick or from input"));

	State = In;
	StateIndex.Rebuild(State);
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
	return HashState(false);
}

uint32 UGalaxySimulationSubsystem::ComputeWorldHash() const
{
	return HashState(true);
}

uint32 UGalaxySimulationSubsystem::HashState(bool bWorldOnly) const
{
	static const FName GalaxyIDName = GET_MEMBER_NAME_CHECKED(FGalaxyState, GalaxyID);
	static const FName RngStateName = GET_MEMBER_NAME_CHECKED(FGalaxyState, RngState);
	// Knowing and telling: saved and part of the full hash, but not world state (section 8.1).
	static const FName NotWorldNames[] = {
		GET_MEMBER_NAME_CHECKED(FGalaxyState, Knowing),
		GET_MEMBER_NAME_CHECKED(FGalaxyState, Log),
		GET_MEMBER_NAME_CHECKED(FGalaxyState, Chronicle),
		GET_MEMBER_NAME_CHECKED(FGalaxyState, TellRngState),
	};

	FStateHasher Hasher;
	for (TFieldIterator<FProperty> It(FGalaxyState::StaticStruct()); It; ++It)
	{
		const FName PropertyName = It->GetFName();
		if (PropertyName == GalaxyIDName)
		{
			continue;
		}
		if (bWorldOnly)
		{
			bool bNotWorld = false;
			for (const FName NotWorldName : NotWorldNames)
			{
				bNotWorld |= PropertyName == NotWorldName;
			}
			if (bNotWorld)
			{
				continue;
			}
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
	ensureMsgf(!bInTickStages, TEXT("SimulateTick re-entered from inside a tick stage: StepOnce or RunHeadless from an OnLogEntry handler"));

	// Tick 0 is the generated state: no event raised inside a tick stage carries tick 0, so the "0 = unset" tick fields
	// stay unambiguous there. Player input outside the tick can still run at tick 0.
	++State.Tick;

	{
		// The stages hold references into the state, and OnLogEntry fires from inside them (section 9.4).
		TGuardValue<bool> StageGuard(bInTickStages, true);
		TickEconomy();
		TickResearch();
		TickDiplomacy();
		TickConflict();
		TickHazards();
		TickCivAI();
		TickCrossings();
		// The calling rests on no vessel during the interregnum leap (section 4.4): no travel and no sight.
		if (State.Calling.LifeState == ELifeState::Embodied)
		{
			TickPlayerTravel();
			TickSight();
		}
	}
	// Outside the guard: death and deposition replace the vessel, and the leap of Phase 4b runs ticks from here.
	ResolvePendingRoleEffect();

	SyncRngState();
	if (bBroadcastEnabled)
	{
		OnSimTick.Broadcast(State.Tick);
	}
}

void UGalaxySimulationSubsystem::TickEconomy()
{
	// The rows of the transcendents that press (unsealed, with a row; null for the others) and which systems lie in the
	// sphere of each, by system index. Built once: the pressures of this stage are read from here, not by one call of
	// the public function per system and not by one row lookup per system (section 9.4).
	const int32 TranscendentCount = State.Transcendents.Num();
	const FPressingDefs ActiveDefs = PressingDefsOf(State, Tables);
	const bool bAnyPresser = ActiveDefs.ContainsByPredicate([](const FTranscendentDef* Def) { return Def != nullptr; });
	TArray<TBitArray<>, TInlineAllocator<4>> InSphere;
	for (int32 TranscendentIndex = 0; TranscendentIndex < TranscendentCount; ++TranscendentIndex)
	{
		const FTranscendentState& Transcendent = State.Transcendents[TranscendentIndex];
		const FTranscendentDef* Def = ActiveDefs[TranscendentIndex];
		TBitArray<>& Members = InSphere.Emplace_GetRef(false, State.Systems.Num());
		for (const FName SystemID : Transcendent.InfluencedSystems)
		{
			const int32 SystemIndex = StateIndex.FindSystem(State, SystemID);
			if (Def && Members.IsValidIndex(SystemIndex))
			{
				Members[SystemIndex] = true;
			}
		}
	}

	// The context of the per-tick sources: the only place where MultiplyIncome applies (section 5.8).
	FCosmicEvalContext Sources = MakeEvalContext();
	Sources.bPerTickSource = true;

	for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
	{
		FCivilizationState& Civ = State.Civs[CivIndex];
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

		// The per-tick sources, between the income and the stockpile (section 7.4). Edict and trait rows join in Phase 4.
		// Spiritual pressure, once per civilization and never once per system (section 4.2): the largest P among its
		// settled systems whose dominant source stands on a higher tier, and the first settled system inside the sphere
		// of each transcendent.
		float LargestPressure = 0.f;
		TArray<int32, TInlineAllocator<4>> FirstInside;
		FirstInside.Init(INDEX_NONE, TranscendentCount);
		for (const FName SystemID : Civ.OwnedSystems)
		{
			const int32 SystemIndex = StateIndex.FindSystem(State, SystemID);
			if (!State.Systems.IsValidIndex(SystemIndex) || State.Systems[SystemIndex].OwnerCivID != Civ.CivID || !CosmicState::IsSettled(State.Systems[SystemIndex]))
			{
				continue;
			}
			// The lanes are asked only when a transcendent presses: without one nothing reads them.
			TArray<int32, TInlineAllocator<8>> LaneIndices;
			if (bAnyPresser)
			{
				for (const FName Lane : State.Systems[SystemIndex].Lanes)
				{
					LaneIndices.Add(StateIndex.FindSystem(State, Lane));
				}
			}
			const FCosmicPressure Reading = PressureWithReach(State, StateIndex, Rules, ActiveDefs, SystemIndex, [&](int32 TranscendentIndex)
			{
				const TBitArray<>& Members = InSphere[TranscendentIndex];
				if (Members[SystemIndex])
				{
					return ECosmicSphereReach::Inside;
				}
				for (const int32 LaneIndex : LaneIndices)
				{
					if (Members.IsValidIndex(LaneIndex) && Members[LaneIndex])
					{
						return ECosmicSphereReach::Adjacent;
					}
				}
				return ECosmicSphereReach::None;
			});
			if (Reading.DominantSource != ECosmicPressureSource::None && TierIndex(Reading.DominantTier) > TierIndex(Civ.Tier))
			{
				LargestPressure = FMath::Max(LargestPressure, Reading.P);
			}
			for (int32 TranscendentIndex = 0; TranscendentIndex < TranscendentCount; ++TranscendentIndex)
			{
				if (FirstInside[TranscendentIndex] == INDEX_NONE && InSphere[TranscendentIndex][SystemIndex])
				{
					FirstInside[TranscendentIndex] = SystemIndex;
				}
			}
		}
		// The range of the knob is a lint; the guard is here: a knob below zero or not a number takes nothing. The loss is
		// tested, not the pressure, so that nothing that is not a number reaches Stability.
		const float Loss = (Rules.SpiritualPressureStabilityDrain > 0.f ? Rules.SpiritualPressureStabilityDrain : 0.f) * LargestPressure;
		if (Loss > 0.f)
		{
			Civ.Stability = FMath::Max(0.f, Civ.Stability - Loss);
		}
		for (int32 TranscendentIndex = 0; TranscendentIndex < TranscendentCount; ++TranscendentIndex)
		{
			const FTranscendentDef* Def = ActiveDefs[TranscendentIndex];
			if (!Def || FirstInside[TranscendentIndex] == INDEX_NONE || TierIndex(Def->Tier) <= TierIndex(Civ.Tier))
			{
				continue;
			}
			Sources.CivIndex = CivIndex;
			Sources.SystemIndex = FirstInside[TranscendentIndex];
			Sources.OtherID = State.Transcendents[TranscendentIndex].TranscendentID;
			Sources.SourceRow = State.Transcendents[TranscendentIndex].TranscendentID;
			Sources.GalaxyRow = Civ.GalaxyRow;
			UCosmicEventSubsystem::ApplyAll(Def->PressureEffectsPerTick, Sources);
		}

		Civ.Stockpile.Energy += Civ.IncomePerTick.Energy;
		Civ.Stockpile.Matter += Civ.IncomePerTick.Matter;
		Civ.Stockpile.Insight += Civ.IncomePerTick.Insight;
		Civ.Stockpile.Influence += Civ.IncomePerTick.Influence;
	}

	// Counted last, over the Stability every civilization has at the end of this stage (section 7.4).
	for (FCivilizationState& Civ : State.Civs)
	{
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
	// (a) resolution, (b) countdown and expiry, (c) spawn; Hazard.Claim (section 4.1, 4.2): the crisis handling work of Phase 2.

	// (d) The event slots: the last step of this stage (section 7.4).
	UCosmicEventSubsystem* Events = GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>();
	check(Events);
	Events->EvaluateAndFire(State, Rng);
}

void UGalaxySimulationSubsystem::TickCivAI()
{
	// Policy profiles of AI civs and regents (section 3.1).
}

void UGalaxySimulationSubsystem::TickCrossings()
{
	// Cleanup of civilizations without systems (Phase 2), then arrivals and departures (Phase 6a) (section 7.4).
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

void UGalaxySimulationSubsystem::TickSight()
{
	// Sight range, meetings and first contact: reads the world, writes State.Knowing and the telling (section 7.4, 8.1).
}

void UGalaxySimulationSubsystem::ResolvePendingRoleEffect()
{
	// Death and deposition recorded in Calling.PendingEffect start their pipeline here, at the tick boundary (section 4.4, 5.10).
}

const FCivilizationState* UGalaxySimulationSubsystem::FindCiv(FName CivID) const
{
	const int32 Index = StateIndex.FindCiv(State, CivID);
	return State.Civs.IsValidIndex(Index) ? &State.Civs[Index] : nullptr;
}

const FStarSystemState* UGalaxySimulationSubsystem::FindSystem(FName SystemID) const
{
	const int32 Index = StateIndex.FindSystem(State, SystemID);
	return State.Systems.IsValidIndex(Index) ? &State.Systems[Index] : nullptr;
}
