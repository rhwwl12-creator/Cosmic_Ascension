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
#include "Math/NumericLimits.h"
#include "Misc/Crc.h"
#include "Misc/Guid.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"
#include "UObject/NameTypes.h"
#include "Player/PlayerRoleSubsystem.h"
#include "UObject/EnumProperty.h"
#include "UObject/TextProperty.h"
#include "UObject/UnrealType.h"

namespace
{
	// Research fields (section 5.4). Sim code walks this array and never the Research maps themselves (section 9.2).
	const FName ResearchFields[] = { FName(TEXT("Physics")), FName(TEXT("Society")), FName(TEXT("Engineering")), FName(TEXT("Transcendence")) };

	// The field whose breakthroughs may find a book that was shut (section 5.11).
	const FName& SocietyField = ResearchFields[1];

	// One DT_LogTemplates row per field for its breakthroughs (section 8.2).
	static_assert(UE_ARRAY_COUNT(CosmicRows::LogBreakthrough) == UE_ARRAY_COUNT(ResearchFields));

	// One breakthrough opens at each of these progress marks (section 5.4).
	constexpr float BreakthroughThresholds[] = { 0.25f, 0.5f, 0.75f, 1.0f };

	// Home system of the first life (section 5.10).
	const FName PlayerHomeSystemID(TEXT("sol"));

	constexpr int32 MaxLanesPerSystem = 4;

	// The disc of the home galaxy: about pi x 1000^2 of map per system (section 7.4).
	constexpr float DiscRadiusPerRootSystem = 1000.f;

	// Rejection sampling of a place: at most this many tries per system; the last one is halved if it misses too, and a
	// point of the square halved always lies in the disc (section 9.2).
	constexpr int32 MaxPlaceTries = 16;

	// One draw read as an index in [0, Count). The number is drawn by the caller whether or not it is used, so the draws
	// never depend on Count (section 9.2). INDEX_NONE when there is nothing to pick.
	int32 PickIndex(float Roll, int32 Count)
	{
		return Count > 0 ? FMath::Min(static_cast<int32>(Roll * static_cast<float>(Count)), Count - 1) : INDEX_NONE;
	}

	// The star class one draw gives under cumulative shares (DT_Rules.LivingWorldStarShares): the first class whose
	// share lies above the draw. A draw above every share (shares that do not end at 1, or are not numbers) is the last.
	int32 StarClassIndex(float Roll, const TArray<float>& Shares)
	{
		for (int32 Index = 0; Index < Shares.Num(); ++Index)
		{
			if (Roll < Shares[Index])
			{
				return Index;
			}
		}
		return Shares.Num() - 1;
	}

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

	// The claim rule after an expiry (section 4.2): on a settled system whose slot the expiry left free, the claimant
	// places Hazard.Claim unless its sphere is full or the system is its owner's last unclaimed settled system. The claim
	// number stands for the weight of the faith traits (section 4.5 (4)): 1 until the trait rows of Phase 4, so it always
	// passes. It is drawn whether or not it is read.
	void TryPlaceClaim(FCosmicEvalContext& Ctx, int32 SystemIndex, float ClaimNumber)
	{
		static const FGameplayTag ClaimTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Claim")));
		const FGalaxyState& State = Ctx.State;
		if (!State.Systems.IsValidIndex(SystemIndex) || !CosmicState::IsSettled(State.Systems[SystemIndex]) || State.Systems[SystemIndex].ActiveHazard.IsValid())
		{
			return;
		}
		const int32 Claimant = CosmicHazards::ClaimantOf(State, Ctx.Index, Ctx.Tables, Ctx.Rules, SystemIndex);
		const FTranscendentDef* Def = State.Transcendents.IsValidIndex(Claimant) ? Ctx.Tables.FindTranscendent(State.Transcendents[Claimant].TranscendentID) : nullptr;
		if (!Def || State.Transcendents[Claimant].InfluencedSystems.Num() >= Def->MaxInfluencedSystems
			|| CosmicHazards::IsLastUnclaimed(State, Ctx.Index, Ctx.Tables, SystemIndex))
		{
			return;
		}
		constexpr float FaithWeight = 1.f;
		FName ClaimRow;
		if (!(ClaimNumber < FaithWeight) || !Ctx.Tables.FindHazardByTag(ClaimTag, &ClaimRow))
		{
			return;
		}
		CosmicHazards::Place(Ctx, SystemIndex, ClaimRow);
	}

	// The expiry of a claim (section 4.2): the claimant of now joins the system to its sphere, unless the system is no
	// longer settled, the sphere is full, or Law.Remnant forbids it. True when it joined.
	bool JoinClaimedSphere(FCosmicEvalContext& Ctx, int32 SystemIndex)
	{
		FGalaxyState& State = Ctx.State;
		if (!State.Systems.IsValidIndex(SystemIndex) || !CosmicState::IsSettled(State.Systems[SystemIndex]))
		{
			return false;
		}
		const int32 Claimant = CosmicHazards::ClaimantOf(State, Ctx.Index, Ctx.Tables, Ctx.Rules, SystemIndex);
		const FTranscendentDef* Def = State.Transcendents.IsValidIndex(Claimant) ? Ctx.Tables.FindTranscendent(State.Transcendents[Claimant].TranscendentID) : nullptr;
		if (!Def || State.Transcendents[Claimant].InfluencedSystems.Num() >= Def->MaxInfluencedSystems
			|| CosmicHazards::IsLastUnclaimed(State, Ctx.Index, Ctx.Tables, SystemIndex))
		{
			return false;
		}
		State.Transcendents[Claimant].InfluencedSystems.Add(State.Systems[SystemIndex].SystemID);
		return true;
	}

	// The expiry of a rebellion (section 4.1, 7.4 notable hook 2). The throne changes for an AI people always, with a new
	// profile picked by the outcome number; for the people the vessel rules only when the DeposeRuler this row just
	// applied is the one recorded. A throne that changes closes the open Ruler record (LastTick = max(Tick, 1)) and then
	// makes the open Rebel record the Ruler. Either way the count of discontent starts over.
	void ChangeThrone(FCosmicEvalContext& Ctx, int32 CivIndex, FName HazardRow, float OutcomeNumber)
	{
		static const FGameplayTag DeposeTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Effect.DeposeRuler")));
		static const FGameplayTag RulerTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Notable.Ruler")));
		static const FGameplayTag RebelTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Notable.Rebel")));
		FGalaxyState& State = Ctx.State;
		if (!State.Civs.IsValidIndex(CivIndex))
		{
			return;
		}
		FCivilizationState& Civ = State.Civs[CivIndex];
		bool bThroneChanges = true;
		if (Civ.bIsPlayerRuled)
		{
			bThroneChanges = State.Calling.PendingEffect == DeposeTag && State.Calling.PendingSourceRow == HazardRow;
		}
		else
		{
			Civ.AIProfile = CosmicState::PickAIProfile(OutcomeNumber);
		}
		if (bThroneChanges)
		{
			for (FNotableRecord& Record : Civ.Notables)
			{
				if (Record.LastTick == 0 && Record.Role == RulerTag)
				{
					Record.LastTick = FMath::Max<int64>(State.Tick, 1);
				}
			}
			for (FNotableRecord& Record : Civ.Notables)
			{
				if (Record.LastTick == 0 && Record.Role == RebelTag)
				{
					Record.Role = RulerTag;
				}
			}
		}
		Civ.LowStabilityTicks = 0;
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

	const FGalaxyDef* HomeRow = Tables.FindGalaxy(Rules.HomeGalaxyRow);
	if (Tables.Galaxies && !HomeRow)
	{
		UE_LOG(LogCosmic, Error, TEXT("DT_Galaxies has no row '%s' (DT_Rules.HomeGalaxyRow): the home galaxy runs on the C++ defaults."), *Rules.HomeGalaxyRow.ToString());
	}
	HomeGalaxy = HomeRow ? *HomeRow : FGalaxyDef();
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
	if (IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("NewGalaxy called from a handler of a Core Logic delegate: a handler may only read the state"));
		return;
	}
	ensureMsgf(!bInTickStages, TEXT("NewGalaxy from inside a tick stage: issue it from OnSimTick or from input"));

	// A count of 0 or less is the home row's (section 3.1). No upper bound: the actor budget is the presentation's.
	SystemCount = FMath::Max(SystemCount > 0 ? SystemCount : HomeGalaxy.LivingWorlds, 1);
	CivCount = FMath::Clamp(CivCount > 0 ? CivCount : HomeGalaxy.Peoples, 1, SystemCount);

	// The range checks of the two knobs are lints; the guards are here, with one line when they bite.
	const FCosmicRulesDef DefaultRules;
	const TArray<float>& StarShares = (Rules.LivingWorldStarShares.Num() == static_cast<int32>(UE_ARRAY_COUNT(StarClassNames))) ? Rules.LivingWorldStarShares : DefaultRules.LivingWorldStarShares;
	if (&StarShares != &Rules.LivingWorldStarShares)
	{
		UE_LOG(LogCosmic, Error, TEXT("DT_Rules: LivingWorldStarShares has %d entries, not %d: the C++ default is used"), Rules.LivingWorldStarShares.Num(), static_cast<int32>(UE_ARRAY_COUNT(StarClassNames)));
	}
	float HomeShare = HomeGalaxy.HomeRadiusShare;
	if (!(HomeShare >= 0.f && HomeShare <= 1.f))
	{
		HomeShare = FGalaxyDef().HomeRadiusShare;
		UE_LOG(LogCosmic, Error, TEXT("DT_Galaxies: HomeRadiusShare %f of the home row is outside 0..1, read as %f"), HomeGalaxy.HomeRadiusShare, HomeShare);
	}
	const int32 Places = FMath::Clamp(Rules.FallenTranscendentCount, 0, MaxFallenRoster);
	if (Places != Rules.FallenTranscendentCount)
	{
		UE_LOG(LogCosmic, Error, TEXT("DT_Rules: FallenTranscendentCount %d is outside 0..%d, read as %d"), Rules.FallenTranscendentCount, MaxFallenRoster, Places);
	}

	State = FGalaxyState();
	State.SaveVersion = FGalaxyState::CurrentSaveVersion;
	State.Seed = Seed;
	// Slot identity, deliberately not drawn from the sim stream (section 9.2).
	State.GalaxyID = FGuid::NewGuid();
	Rng.Initialize(Seed);
	// The tell stream starts from a derived value, before Install tells the first line (section 9.2).
	State.TellRngState = static_cast<int32>(CosmicSeed::Derive(Seed, "", CosmicSeed::Purpose::Tell, 0));

	// The draws below are made in the order NewGalaxy's header comment gives, whether or not they are used, so their
	// number follows Seed, the two counts and the roster knob and never what the tables hold (section 7.4, 9.2).

	// 1. Systems on a disc. System 0 is sol, the home of the first life, at a fixed place.
	const float DiscRadius = DiscRadiusPerRootSystem * FMath::Sqrt(static_cast<float>(SystemCount));
	State.Systems.Reserve(SystemCount);
	for (int32 Index = 0; Index < SystemCount; ++Index)
	{
		FStarSystemState& System = State.Systems.AddDefaulted_GetRef();
		System.SystemID = (Index == 0) ? PlayerHomeSystemID : FName(*FString::Printf(TEXT("sys_%d"), Index));
		System.GalaxyRow = Rules.HomeGalaxyRow;

		if (Index == 0)
		{
			System.Position = FVector(HomeShare * DiscRadius, 0.0, 0.0);
		}
		else
		{
			// A point of the square, thrown away while it lies outside the disc. No trigonometry (section 9.2).
			float X = 0.f;
			float Y = 0.f;
			for (int32 Try = 1; ; ++Try)
			{
				X = Rng.FRand() * 2.f - 1.f;
				Y = Rng.FRand() * 2.f - 1.f;
				if (X * X + Y * Y <= 1.f)
				{
					break;
				}
				if (Try == MaxPlaceTries)
				{
					X *= 0.5f;
					Y *= 0.5f;
					break;
				}
			}
			System.Position = FVector(X * DiscRadius, Y * DiscRadius, 0.0);
		}

		const float ClassRoll = Rng.FRand();
		System.StarClass = FGameplayTag::RequestGameplayTag(StarClassNames[StarClassIndex(ClassRoll, StarShares)]);

		// The player's home stays ordinary: no numeric edge (section 5.11).
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

	// Lanes join systems of the one galaxy (section 5.9). A spanning tree first, so every system can be reached: each
	// system links to the nearest earlier one. Then one more lane per system to its nearest unlinked neighbour, while
	// both ends have room. No draw.
	for (int32 Index = 1; Index < SystemCount; ++Index)
	{
		AddLane(Index, FindNearest(Index, Index, false));
	}
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
	StateIndex.Rebuild(State);

	// 2. Homes. The first AI people lives one lane from sol, so the first contact comes at 5:30 (section 2.4); every
	// other AI people draws its home among the systems still free.
	TArray<int32> HomeIndices;
	HomeIndices.Reserve(CivCount);
	HomeIndices.Add(0);
	TArray<int32> FreeSystems;
	FreeSystems.Reserve(SystemCount - 1);
	for (int32 Index = 1; Index < SystemCount; ++Index)
	{
		FreeSystems.Add(Index);
	}
	if (CivCount >= 2)
	{
		TArray<int32> Neighbours;
		for (const FName Lane : State.Systems[0].Lanes)
		{
			Neighbours.Add(StateIndex.FindSystem(State, Lane));
		}
		const int32 Neighbour = Neighbours[PickIndex(Rng.FRand(), Neighbours.Num())];
		HomeIndices.Add(Neighbour);
		FreeSystems.Remove(Neighbour);
	}
	while (HomeIndices.Num() < CivCount)
	{
		const int32 Pick = PickIndex(Rng.FRand(), FreeSystems.Num());
		HomeIndices.Add(FreeSystems[Pick]);
		FreeSystems.RemoveAt(Pick);
	}

	// The peoples. Civ 0 is the player's; every CivID comes from NextCivSerial (section 7.2).
	State.Civs.Reserve(CivCount);
	for (int32 CivIndex = 0; CivIndex < CivCount; ++CivIndex)
	{
		FStarSystemState& Home = State.Systems[HomeIndices[CivIndex]];
		FCivilizationState& Civ = State.Civs.AddDefaulted_GetRef();
		Civ.CivID = FName(*FString::Printf(TEXT("civ_%d"), State.NextCivSerial++));
		Civ.GalaxyRow = Rules.HomeGalaxyRow;
		Civ.HomeSystemID = Home.SystemID;
		Civ.ArchetypeRow = (CivIndex == 0) ? Rules.PlayerArchetypeRow : CosmicRows::ArchDefault;
		Civ.Color = CivWheelColor(static_cast<uint8>((CivIndex * 255) / CivCount));
		for (const FName Field : ResearchFields)
		{
			Civ.Research.Add(Field, 0.f);
			Civ.ResearchFocus.Add(Field, 1.f / static_cast<float>(UE_ARRAY_COUNT(ResearchFields)));
		}
		Civ.OwnedSystems.Add(Home.SystemID);
		Home.OwnerCivID = Civ.CivID;
	}

	// 3. Archetype, ethics and profile of every AI people. AI archetypes: the DT_Civs rows with a positive, finite
	// Weight, in lexical row order so the draw never depends on the table's own order (section 9.2). A row may be
	// drawn more than once.
	TArray<FName> ArchetypeRows;
	TArray<float> ArchetypeWeights;
	float TotalWeight = 0.f;
	for (const FName RowName : Tables.GetCivRowNames())
	{
		const FCivArchetypeDef* Row = Tables.FindCiv(RowName);
		if (Row && Row->Weight > 0.f && Row->Weight <= TNumericLimits<float>::Max())
		{
			ArchetypeRows.Add(RowName);
			ArchetypeWeights.Add(Row->Weight);
			TotalWeight += Row->Weight;
		}
	}
	for (int32 CivIndex = 0; CivIndex < CivCount; ++CivIndex)
	{
		FCivilizationState& Civ = State.Civs[CivIndex];
		const bool bPlayerCiv = (CivIndex == 0);
		float ArchetypeRoll = 0.f;
		float EthicsRolls[3] = { 0.f, 0.f, 0.f };
		float ProfileRoll = 0.f;
		if (!bPlayerCiv)
		{
			ArchetypeRoll = Rng.FRand();
			for (float& Roll : EthicsRolls)
			{
				Roll = Rng.FRand() * 2.f - 1.f;
			}
			ProfileRoll = Rng.FRand();
			if (!ArchetypeRows.IsEmpty())
			{
				float Mark = ArchetypeRoll * TotalWeight;
				int32 RowIndex = 0;
				while (RowIndex < ArchetypeRows.Num() - 1 && Mark >= ArchetypeWeights[RowIndex])
				{
					Mark -= ArchetypeWeights[RowIndex];
					++RowIndex;
				}
				Civ.ArchetypeRow = ArchetypeRows[RowIndex];
			}
		}
		if (const FCivArchetypeDef* Archetype = Tables.FindCiv(Civ.ArchetypeRow))
		{
			Civ.Traits = Archetype->Traits;
			Civ.Ethics = Archetype->EthicsPreset;
			Civ.AIProfile = Archetype->AIProfile;
			if (bPlayerCiv)
			{
				Civ.Color = Archetype->Color;
			}
			if (Archetype->HomeStarClass.IsValid())
			{
				State.Systems[HomeIndices[CivIndex]].StarClass = Archetype->HomeStarClass;
			}
		}
		if (!bPlayerCiv)
		{
			if (Civ.ArchetypeRow == CosmicRows::ArchDefault)
			{
				Civ.Ethics.Pacifism = EthicsRolls[0];
				Civ.Ethics.Spiritualism = EthicsRolls[1];
				Civ.Ethics.Openness = EthicsRolls[2];
			}
			if (!Civ.AIProfile.IsValid())
			{
				Civ.AIProfile = CosmicState::PickAIProfile(ProfileRoll);
			}
		}
	}

	// 4. Names, made once and flattened (section 5.11, 8.5). Pools without replacement are partial shuffles of an index
	// array: the k-th taker swaps a drawn index into place k.
	auto TakeWithoutReplacement = [](TArray<int32>& Order, int32& Taken, float Roll) -> int32
	{
		if (Taken >= Order.Num())
		{
			return INDEX_NONE;
		}
		Order.Swap(Taken, Taken + PickIndex(Roll, Order.Num() - Taken));
		return Order[Taken++];
	};
	TArray<int32> StarOrder;
	for (int32 Index = 0; Index < HomeGalaxy.StarNameBank.Num(); ++Index)
	{
		StarOrder.Add(Index);
	}
	int32 StarsTaken = 0;
	for (int32 CivIndex = 0; CivIndex < CivCount; ++CivIndex)
	{
		FCivilizationState& Civ = State.Civs[CivIndex];
		FStarSystemState& Home = State.Systems[HomeIndices[CivIndex]];
		const float HomeRoll = Rng.FRand();
		const float NameRoll = Rng.FRand();
		const FCivArchetypeDef* Archetype = Tables.FindCiv(Civ.ArchetypeRow);
		if (Archetype && !Archetype->HomeSystemNames.IsEmpty())
		{
			Home.DisplayName = Flatten(Archetype->HomeSystemNames[PickIndex(HomeRoll, Archetype->HomeSystemNames.Num())]);
		}
		else
		{
			const int32 Star = TakeWithoutReplacement(StarOrder, StarsTaken, HomeRoll);
			Home.DisplayName = (Star != INDEX_NONE) ? Flatten(HomeGalaxy.StarNameBank[Star]) : FText::GetEmpty();
		}
		if (Archetype && !Archetype->NameBank.IsEmpty())
		{
			// Name patterns call the home system by name.
			FFormatNamedArguments NameArgs;
			NameArgs.Add(TEXT("Star"), Home.DisplayName);
			Civ.CivName = Flatten(FText::Format(Archetype->NameBank[PickIndex(NameRoll, Archetype->NameBank.Num())], NameArgs));
		}
	}
	const int32 StemCount = HomeGalaxy.CatalogStems.Num();
	TArray<int32> PairOrder;
	for (int32 Index = 0; Index < HomeGalaxy.CatalogPrefixes.Num() * StemCount; ++Index)
	{
		PairOrder.Add(Index);
	}
	int32 PairsTaken = 0;
	for (FStarSystemState& System : State.Systems)
	{
		if (!System.OwnerCivID.IsNone())
		{
			continue;
		}
		const int32 Pair = TakeWithoutReplacement(PairOrder, PairsTaken, Rng.FRand());
		if (Pair != INDEX_NONE)
		{
			const int32 Prefix = Pair / StemCount;
			const int32 Stem = Pair - Prefix * StemCount;
			System.DisplayName = FText::AsCultureInvariant(HomeGalaxy.CatalogPrefixes[Prefix].ToString() + TEXT(" ") + HomeGalaxy.CatalogStems[Stem].ToString());
		}
	}

	// 5. The fallen roster (section 4.2). Silence is fixed; the others are drawn.
	{
		const FGameplayTag AbyssalTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Realm.Abyssal")));
		auto AddTranscendent = [this](FName RowName, const FTranscendentDef& Def)
		{
			FTranscendentState& Transcendent = State.Transcendents.AddDefaulted_GetRef();
			Transcendent.TranscendentID = RowName;
			Transcendent.Doctrine = Def.Doctrine;
		};

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
			if (Row && RowName != CosmicRows::TransSilence && Row->Allegiance == AbyssalTag && Row->SpawnWeight > 0.f && Row->SpawnWeight <= TNumericLimits<float>::Max())
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

	// 6. Starting cores (section 5.7): one draw per place of the roster. Candidates: unowned systems far enough from
	// sol, one lane from the home of an AI people and in no sphere yet, in array order.
	{
		const TArray<int32> FromSol = CosmicState::LaneDistancesFrom(State, StateIndex, 0);
		const int32 MinLanes = FMath::Max(0, Rules.FallenCoreMinLanesFromHome);
		TBitArray<> NextToAIHome(false, SystemCount);
		for (int32 CivIndex = 1; CivIndex < CivCount; ++CivIndex)
		{
			for (const FName Lane : State.Systems[HomeIndices[CivIndex]].Lanes)
			{
				NextToAIHome[StateIndex.FindSystem(State, Lane)] = true;
			}
		}
		TBitArray<> InSphere(false, SystemCount);
		for (int32 Place = 0; Place < Places; ++Place)
		{
			const float Roll = Rng.FRand();
			if (!State.Transcendents.IsValidIndex(Place))
			{
				continue;
			}
			FTranscendentState& Transcendent = State.Transcendents[Place];
			const FTranscendentDef* Def = Tables.FindTranscendent(Transcendent.TranscendentID);
			if (!Def || Def->InitialInfluencedSystems <= 0)
			{
				continue;
			}
			TArray<int32> Candidates;
			int32 Farthest = INDEX_NONE;
			for (int32 Index = 0; Index < SystemCount; ++Index)
			{
				const FStarSystemState& System = State.Systems[Index];
				if (!System.OwnerCivID.IsNone() || InSphere[Index] || FromSol[Index] == INDEX_NONE)
				{
					continue;
				}
				if (FromSol[Index] >= MinLanes && NextToAIHome[Index])
				{
					Candidates.Add(Index);
				}
				if (Farthest == INDEX_NONE || FromSol[Index] > FromSol[Farthest]
					|| (FromSol[Index] == FromSol[Farthest] && FNameLexicalLess()(System.SystemID, State.Systems[Farthest].SystemID)))
				{
					Farthest = Index;
				}
			}
			const int32 Core = Candidates.IsEmpty() ? Farthest : Candidates[PickIndex(Roll, Candidates.Num())];
			if (Core != INDEX_NONE)
			{
				Transcendent.InfluencedSystems.Add(State.Systems[Core].SystemID);
				InSphere[Core] = true;
			}
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

	UE_LOG(LogCosmic, Log, TEXT("NewGalaxy: seed %d, %d systems, %d civs, galaxy %s"), Seed, SystemCount, CivCount, *Rules.HomeGalaxyRow.ToString());
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
	if (IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("LoadState called from a handler of a Core Logic delegate: a handler may only read the state"));
		return;
	}
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
	// A backstop: StepOnce and RunHeadless already refuse a tick asked for from a handler.
	ensureMsgf(!bInTickStages, TEXT("SimulateTick re-entered from inside a tick stage"));

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

	// The spheres FallenSphereTicks counts: those of the pressing transcendents whose row is fallen (section 5.11).
	const FGameplayTag AbyssalTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Realm.Abyssal")));
	TBitArray<> FallenPresser(false, TranscendentCount);
	for (int32 TranscendentIndex = 0; TranscendentIndex < TranscendentCount; ++TranscendentIndex)
	{
		FallenPresser[TranscendentIndex] = ActiveDefs[TranscendentIndex] && ActiveDefs[TranscendentIndex]->Allegiance == AbyssalTag;
	}
	// By index into Civs: a settled system of the civilization lies inside a fallen sphere this tick.
	TBitArray<> InFallenSphere(false, State.Civs.Num());

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
		for (int32 TranscendentIndex = 0; TranscendentIndex < TranscendentCount; ++TranscendentIndex)
		{
			if (FallenPresser[TranscendentIndex] && FirstInside[TranscendentIndex] != INDEX_NONE)
			{
				InFallenSphere[CivIndex] = true;
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

	// Counted last, over the Stability every civilization has at the end of this stage (section 7.4), and over the
	// sphere membership of its start (section 5.11).
	for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
	{
		FCivilizationState& Civ = State.Civs[CivIndex];
		Civ.LowStabilityTicks = (Civ.Stability < LowStabilityThreshold) ? Civ.LowStabilityTicks + 1 : 0;
		Civ.FallenSphereTicks = InFallenSphere[CivIndex] ? Civ.FallenSphereTicks + 1 : 0;
	}
}

void UGalaxySimulationSubsystem::TickResearch()
{
	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Log);
	UCosmicEventSubsystem* Events = GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>();
	check(Events);

	for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
	{
		FCivilizationState& Civ = State.Civs[CivIndex];
		const float Spent = FMath::Max(0.f, FMath::Min(Civ.Stockpile.Insight, Civ.IncomePerTick.Insight * Economy::ResearchInsightShare));
		Civ.Stockpile.Insight -= Spent;

		for (int32 FieldIndex = 0; FieldIndex < static_cast<int32>(UE_ARRAY_COUNT(ResearchFields)); ++FieldIndex)
		{
			const FName Field = ResearchFields[FieldIndex];
			const float Before = Civ.Research.FindRef(Field);
			const float After = FMath::Min(1.f, Before + Spent * Civ.ResearchFocus.FindRef(Field) * Economy::ResearchPerInsight);
			Civ.Research.Add(Field, After);

			// A breakthrough is told once per mark crossed. Every people has a name from its birth (section 5.11); whether
			// the line is told is the story scope's question, never the world's (section 8.1).
			for (const float Threshold : BreakthroughThresholds)
			{
				if (Before < Threshold && After >= Threshold)
				{
					FFormatNamedArguments Args;
					Args.Add(TEXT("Civ"), Civ.CivName);
					Log->PushRow(CosmicRows::LogBreakthrough[FieldIndex], Civ.bIsPlayerRuled ? 1 : 0, Args, Civ.HomeSystemID, Civ.CivID);

					// After its line, a Society breakthrough may find a book that was shut: the row's conditions pick the
					// people (section 5.11). Its subject is the civilization, with its capital while that is settled.
					if (Field == SocietyField)
					{
						const int32 Capital = StateIndex.FindSystem(State, Civ.HomeSystemID);
						FCosmicEvalContext Found = MakeEvalContext();
						Found.CivIndex = CivIndex;
						Found.SystemIndex = (State.Systems.IsValidIndex(Capital) && State.Systems[Capital].OwnerCivID == Civ.CivID && CosmicState::IsSettled(State.Systems[Capital])) ? Capital : INDEX_NONE;
						Found.GalaxyRow = Civ.GalaxyRow;
						Events->FireRow(CosmicRows::EventBookFound, Found);
					}
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
	UCosmicEventSubsystem* Events = GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>();
	check(Events);

	// (a) Resolution by a completed means arrives with the megaprojects (work item 8).

	// (b) Countdown and expiry. Every countdown first, so a hazard placed later in this stage keeps its count this tick.
	TArray<TPair<int32, FGameplayTag>, TInlineAllocator<8>> Expiring;
	for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num(); ++SystemIndex)
	{
		FStarSystemState& System = State.Systems[SystemIndex];
		if (System.ActiveHazard.IsValid() && --System.HazardTicksRemaining <= 0)
		{
			Expiring.Emplace(SystemIndex, System.ActiveHazard);
		}
	}
	// Two numbers per expiry, all of them before the first one runs (section 9.2).
	TArray<float, TInlineAllocator<16>> ExpiryNumbers;
	for (int32 Index = 0; Index < Expiring.Num() * 2; ++Index)
	{
		ExpiryNumbers.Add(Rng.GetFraction());
	}
	for (int32 Index = 0; Index < Expiring.Num(); ++Index)
	{
		ExpireHazard(Expiring[Index].Key, Expiring[Index].Value, ExpiryNumbers[Index * 2], ExpiryNumbers[Index * 2 + 1]);
	}

	// (c) Spawn. One number per settled system, all drawn first, whether its slot is free or not.
	TArray<int32, TInlineAllocator<64>> Settled;
	TArray<float, TInlineAllocator<64>> SpawnNumbers;
	for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num(); ++SystemIndex)
	{
		if (CosmicState::IsSettled(State.Systems[SystemIndex]))
		{
			Settled.Add(SystemIndex);
			SpawnNumbers.Add(Rng.GetFraction());
		}
	}
	const TArray<FName> HazardRows = Tables.GetHazardRowNames();
	for (int32 Index = 0; Index < Settled.Num(); ++Index)
	{
		const int32 SystemIndex = Settled[Index];
		const FStarSystemState& System = State.Systems[SystemIndex];
		const int32 OwnerIndex = StateIndex.FindCiv(State, System.OwnerCivID);
		if (!CosmicState::IsSettled(System) || System.ActiveHazard.IsValid() || !State.Civs.IsValidIndex(OwnerIndex))
		{
			continue;
		}
		const FCivilizationState& Owner = State.Civs[OwnerIndex];
		const FCivArchetypeDef* Archetype = Tables.FindCiv(Owner.ArchetypeRow.IsNone() ? CosmicRows::ArchDefault : Owner.ArchetypeRow);

		// The share of every Draw row on this system: its chance times the owner's multiplier, where its conditions pass.
		// The ranges are lints; the guards are comparisons, so a value below 0 or not a number takes nothing.
		FCosmicEvalView View(State, StateIndex, Tables, Rules);
		View.CivIndex = OwnerIndex;
		View.SystemIndex = SystemIndex;
		View.GalaxyRow = System.GalaxyRow;
		TArray<TPair<FName, float>, TInlineAllocator<8>> Shares;
		float Total = 0.f;
		for (const FName RowName : HazardRows)
		{
			const FHazardDef* Row = Tables.FindHazard(RowName);
			if (!Row || Row->SpawnRule != ECosmicHazardSpawn::Draw)
			{
				continue;
			}
			const float Chance = Row->SpawnChancePerTick > 0.f ? FMath::Min(Row->SpawnChancePerTick, 1.f) : 0.f;
			const float* Multiplier = Archetype ? Archetype->HazardSpawnMultipliers.Find(Row->HazardTag) : nullptr;
			const float Factor = Multiplier ? (*Multiplier > 0.f ? FMath::Min(*Multiplier, TNumericLimits<float>::Max()) : 0.f) : 1.f;
			const float Share = Chance * Factor;
			if (!(Share > 0.f))
			{
				continue;
			}
			View.SourceRow = RowName;
			if (!UCosmicEventSubsystem::CheckAll(Row->SpawnConditions, View))
			{
				continue;
			}
			Shares.Emplace(RowName, Share);
			Total += Share;
		}
		// Shares that add up to more than 1 are scaled down together by scaling the number instead (as the event slots do).
		const float Pick = Total > 1.f ? SpawnNumbers[Index] * Total : SpawnNumbers[Index];
		float Reached = 0.f;
		for (const TPair<FName, float>& Share : Shares)
		{
			Reached += Share.Value;
			if (Pick < Reached)
			{
				FCosmicEvalContext Ctx = MakeEvalContext();
				if (CosmicHazards::Place(Ctx, SystemIndex, Share.Key))
				{
					Events->FollowUpNotes(Ctx);
				}
				break;
			}
		}
	}

	// The capital rule: no draw. A people's discontent stands in its capital (section 4.1).
	for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
	{
		const FCivilizationState& Civ = State.Civs[CivIndex];
		const int32 Capital = StateIndex.FindSystem(State, Civ.HomeSystemID);
		if (!State.Systems.IsValidIndex(Capital) || State.Systems[Capital].OwnerCivID != Civ.CivID || !CosmicState::IsSettled(State.Systems[Capital]))
		{
			continue;
		}
		for (const FName RowName : HazardRows)
		{
			const FHazardDef* Row = Tables.FindHazard(RowName);
			if (State.Systems[Capital].ActiveHazard.IsValid())
			{
				break;
			}
			if (!Row || Row->SpawnRule != ECosmicHazardSpawn::Capital)
			{
				continue;
			}
			FCosmicEvalView View(State, StateIndex, Tables, Rules);
			View.CivIndex = CivIndex;
			View.SystemIndex = Capital;
			View.GalaxyRow = Civ.GalaxyRow;
			View.SourceRow = RowName;
			if (!UCosmicEventSubsystem::CheckAll(Row->SpawnConditions, View))
			{
				continue;
			}
			FCosmicEvalContext Ctx = MakeEvalContext();
			if (CosmicHazards::Place(Ctx, Capital, RowName))
			{
				Events->FollowUpNotes(Ctx);
			}
		}
	}

	// (d) The event slots: the last step of this stage (section 7.4).
	Events->EvaluateAndFire(State, Rng);
}

void UGalaxySimulationSubsystem::ExpireHazard(int32 SystemIndex, FGameplayTag Collected, float OutcomeNumber, float ClaimNumber)
{
	static const FGameplayTag SupernovaTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.PreSupernova")));
	static const FGameplayTag RebellionTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Rebellion")));
	static const FGameplayTag ClaimTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Claim")));
	static const FGameplayTag DeposedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Deposed")));

	// A slot an earlier expiry of this pass emptied, or filled again: a hazard placed after the countdown pass keeps its
	// whole count, whatever its tag (every collected slot is at 0 or below, every placement at 1 or above).
	if (!State.Systems.IsValidIndex(SystemIndex) || State.Systems[SystemIndex].ActiveHazard != Collected || State.Systems[SystemIndex].HazardTicksRemaining > 0)
	{
		return;
	}
	UCosmicEventSubsystem* Events = GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>();
	check(Events);

	FName RowName;
	const FHazardDef* Row = Tables.FindHazardByTag(Collected, &RowName);
	const int32 OwnerIndex = StateIndex.FindCiv(State, State.Systems[SystemIndex].OwnerCivID);
	auto EmptySlot = [this, SystemIndex]()
	{
		State.Systems[SystemIndex].ActiveHazard = FGameplayTag();
		State.Systems[SystemIndex].HazardTicksRemaining = 0;
	};
	FCosmicEvalContext Ctx = MakeEvalContext();
	if (!Row)
	{
		// A tag no row holds has no effects and no line: it leaves its slot as a resolution without a row does (a
		// rebellion still ends its leader and its count).
		CosmicHazards::Resolve(Ctx, SystemIndex);
		return;
	}

	Ctx.CivIndex = OwnerIndex;
	Ctx.SystemIndex = SystemIndex;
	Ctx.SourceRow = RowName;
	Ctx.GalaxyRow = State.Systems[SystemIndex].GalaxyRow;

	// The gate comes before any part of the loss (section 4.5): a kept system loses nothing, not even its line.
	const bool bTakesSystem = Collected == SupernovaTag;
	const bool bLastAtGate = bTakesSystem && CosmicHazards::IsLastOwnedSystem(State, StateIndex, SystemIndex);
	if (bLastAtGate && GraceKeepsLastSystem(Ctx, SystemIndex))
	{
		EmptySlot();
		return;
	}

	// The effects of the expiry, then its damage to the neighbours.
	const bool bBranch = Row->BranchChance > 0.f && OutcomeNumber < Row->BranchChance;
	UCosmicEventSubsystem::ApplyAll(bBranch ? Row->OnExpireBranchEffects : Row->OnExpireEffects, Ctx);
	if (!Row->OnExpireNeighbourEffects.IsEmpty())
	{
		const TArray<FName> Lanes = State.Systems[SystemIndex].Lanes;
		for (const FName Lane : Lanes)
		{
			const int32 Neighbour = StateIndex.FindSystem(State, Lane);
			if (!State.Systems.IsValidIndex(Neighbour) || CosmicHazards::IsDeadNode(State.Systems[Neighbour]))
			{
				continue;
			}
			Ctx.SystemIndex = Neighbour;
			Ctx.CivIndex = StateIndex.FindCiv(State, State.Systems[Neighbour].OwnerCivID);
			Ctx.GalaxyRow = State.Systems[Neighbour].GalaxyRow;
			UCosmicEventSubsystem::ApplyAll(Row->OnExpireNeighbourEffects, Ctx);
		}
		Ctx.SystemIndex = SystemIndex;
		Ctx.CivIndex = OwnerIndex;
		Ctx.GalaxyRow = State.Systems[SystemIndex].GalaxyRow;
	}

	// The code of the tag: what no effect may say (section 16 #29 (5)).
	bool bJoined = true;
	if (Collected == SupernovaTag)
	{
		// The gate was asked before the effects. Should an effect have let the owner's other systems go meanwhile (no row
		// of the game can: Data.Integrity keeps TransferSystem out of the expiry effects and out of OnResolveEffects, which
		// a ResolveHazard on a neighbour runs), it is asked again here: a people reaches no system only behind the gate. A
		// system kept here has taken the expiry's effects and its damage to the neighbours already.
		if (!bLastAtGate && CosmicHazards::IsLastOwnedSystem(State, StateIndex, SystemIndex))
		{
			UE_LOG(LogCosmic, Error, TEXT("Expiry of %s: its effects left %s the last owned system of its people; the last-system gate is asked again"), *RowName.ToString(), *State.Systems[SystemIndex].SystemID.ToString());
			if (GraceKeepsLastSystem(Ctx, SystemIndex))
			{
				EmptySlot();
				Events->FollowUpNotes(Ctx);
				return;
			}
		}
		CosmicHazards::Destroy(Ctx, SystemIndex);
		// The collapse of the people the vessel rules: the deposition pipeline (section 4.4). Apply keeps the rules of the
		// record (Embodied, the ruled people, first recorded wins).
		if (State.Civs.IsValidIndex(OwnerIndex) && State.Civs[OwnerIndex].OwnedSystems.IsEmpty())
		{
			FCosmicEffect Collapse;
			Collapse.Type = EEffectType::DeposeRuler;
			Collapse.Tag = DeposedTag;
			Ctx.SystemIndex = INDEX_NONE;
			UCosmicEventSubsystem::Apply(Collapse, Ctx);
			Ctx.SystemIndex = SystemIndex;
		}
	}
	else if (Collected == RebellionTag)
	{
		ChangeThrone(Ctx, OwnerIndex, RowName, OutcomeNumber);
	}
	else if (Collected == ClaimTag)
	{
		bJoined = JoinClaimedSphere(Ctx, SystemIndex);
	}
	EmptySlot();

	// The claim rule on the slot this expiry left free (section 4.2).
	TryPlaceClaim(Ctx, SystemIndex, ClaimNumber);

	// The expiry's own line, then what followed from it.
	Events->TellHazardLine(Ctx, bJoined ? ECosmicHazardLine::Expiry : ECosmicHazardLine::Resolution, RowName, SystemIndex, OwnerIndex);
	Events->FollowUpNotes(Ctx);
}

bool UGalaxySimulationSubsystem::GraceKeepsLastSystem(FCosmicEvalContext& Ctx, int32 SystemIndex)
{
	if (!CosmicHazards::IsLastOwnedSystem(State, StateIndex, SystemIndex))
	{
		return false;
	}
	UCosmicEventSubsystem* Events = GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>();
	check(Events);
	const FCosmicEvalSubject Saved = Ctx;
	Ctx.SystemIndex = SystemIndex;
	Ctx.CivIndex = StateIndex.FindCiv(State, State.Systems[SystemIndex].OwnerCivID);
	Ctx.OtherID = NAME_None;
	Ctx.GalaxyRow = State.Systems[SystemIndex].GalaxyRow;
	const bool bFired = Events->FireRow(CosmicRows::EventGraceRemnant, Ctx);
	static_cast<FCosmicEvalSubject&>(Ctx) = Saved;
	return bFired;
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
	// Death and deposition recorded in Calling.PendingEffect start their pipeline here, at the tick boundary (section 4.4,
	// 5.10). The death pipeline arrives with Phase 4b: until then a KillPlayer stays recorded.
	static const FGameplayTag DeposeTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Effect.DeposeRuler")));
	FCallingState& Calling = State.Calling;
	if (Calling.PendingEffect == DeposeTag)
	{
		UPlayerRoleSubsystem* PlayerRole = GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
		check(PlayerRole);
		PlayerRole->DeposeRuler();
		Calling.PendingEffect = FGameplayTag();
		Calling.PendingCause = FGameplayTag();
		Calling.PendingSourceRow = NAME_None;
	}
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
