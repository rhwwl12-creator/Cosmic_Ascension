// Cosmic Ascension - Sim layer.

#include "Sim/GalaxySimulationSubsystem.h"

#include "Containers/ArrayView.h"
#include "Containers/BitArray.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Containers/Set.h"
#include "Core/CosmicIDs.h"
#include "Core/CosmicSeed.h"
#include "Core/CosmicTags.h"
#include "Cosmic_Ascension.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicRowNames.h"
#include "Engine/GameInstance.h"
#include "Events/CosmicEventSubsystem.h"
#include "Events/CosmicPetitions.h"
#include "GameplayTagContainer.h"
#include "Internationalization/StringTableCoreFwd.h"
#include "Internationalization/Text.h"
#include "Log/CosmicLogSubsystem.h"
#include "HAL/PlatformTime.h"
#include "Math/NumericLimits.h"
#include "Misc/Crc.h"
#include "Misc/Guid.h"
#include "Ascension/AscensionSubsystem.h"
#include "Sim/CosmicConflict.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicMegaprojects.h"
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

	constexpr int32 MaxLanesPerSystem = 4;

	// The disc of the home galaxy: about pi x 1000^2 of map per system (section 7.4).
	constexpr float DiscRadiusPerRootSystem = 1000.f;

	// The place of the sanctuary: this share of the disc radius, at angle 0 - outside the disc of the living worlds, on the
	// side of the first life's home (section 4.5 (1); interim, v0.8 step 5 gives the real sky's place). No draw.
	constexpr float SanctuaryRadiusShare = 1.15f;
	// The star class of the sanctuary: the Sun's.
	const FName SanctuaryStarClass(TEXT("Star.Class.G"));

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

	// Section 5.1 fixes what each resource depends on. The rest of the economy's numbers are DT_Rules knobs (MVP item 8 -
	// FCosmicRulesDef, read through the functions after TierIndex); the stars' stay here until step 5 (the real sky).
	namespace Economy
	{
		// Energy of one owned system by star class, in the order of StarClassNames. G is the baseline.
		constexpr float StarClassEnergy[] = { 2.0f, 1.7f, 1.4f, 1.2f, 1.0f, 0.8f, 0.6f };
		static_assert(UE_ARRAY_COUNT(StarClassEnergy) == UE_ARRAY_COUNT(StarClassNames));
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

	// The economy's knobs as the stages read them (MVP item 8): a rate or a bonus that is not above 0 (NaN included) is 0.
	float EconomyRateOf(float Value)
	{
		return Value > 0.f ? Value : 0.f;
	}

	// The rung's income multiplier: an index the array lacks, a value below 0 or not a number reads as 1.
	float TierMultiplierOf(const FCosmicRulesDef& Rules, EAscensionTier Tier)
	{
		const int32 Index = TierIndex(Tier);
		const float Value = Rules.TierMultiplier.IsValidIndex(Index) ? Rules.TierMultiplier[Index] : 1.f;
		return Value >= 0.f ? Value : 1.f;
	}

	// The share of the Insight income the research stage spends, clamped to 0..1 (not a number: 0).
	float ResearchShareOf(const FCosmicRulesDef& Rules)
	{
		return Rules.ResearchInsightShare > 0.f ? (Rules.ResearchInsightShare < 1.f ? Rules.ResearchInsightShare : 1.f) : 0.f;
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

	// Whether paying Cost from Stockpile would leave less than Waiting - the summed cost of a people's answers that could
	// not pay this tick - in a resource Cost spends and the answers need (step (a) of the crisis stage, section 5.7).
	bool TakesFromWaitingAnswers(const FResourcePool& Stockpile, const FResourcePool& Cost, const FResourcePool& Waiting)
	{
		auto Takes = [](float Held, float Spent, float Needed) { return Spent > 0.f && Needed > 0.f && Held - Spent < Needed; };
		return Takes(Stockpile.Energy, Cost.Energy, Waiting.Energy) || Takes(Stockpile.Matter, Cost.Matter, Waiting.Matter)
			|| Takes(Stockpile.Insight, Cost.Insight, Waiting.Insight) || Takes(Stockpile.Influence, Cost.Influence, Waiting.Influence)
			|| Takes(Stockpile.Causality, Cost.Causality, Waiting.Causality);
	}

	// Weight of a pressure source by its tier (section 4.2). A table too short for the tier reads as 0, and so does a weight
	// below 0, not a number or not finite (a compare: FMath::Max lets NaN and infinity through).
	float PressureWeight(const FCosmicRulesDef& Rules, EAscensionTier Tier)
	{
		const int32 Index = TierIndex(Tier);
		const float Weight = Rules.PressureBySourceTier.IsValidIndex(Index) ? Rules.PressureBySourceTier[Index] : 0.f;
		return (Weight > 0.f && Weight <= TNumericLimits<float>::Max()) ? Weight : 0.f;
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

	// The term of one transcendent's row at a reach. Depth inside the tier (section 3.2): the range is a lint; the guard is
	// here, and a value that is not a number reads as 0. ComputePressureIn and ComputeAttritionPressureIn share it.
	float TranscendentTermOf(const FTranscendentDef& Def, const FCosmicRulesDef& InRules, ECosmicSphereReach Reach)
	{
		const float Depth = Def.Depth > 0.f ? FMath::Min(Def.Depth, 1.f) : 0.f;
		return PressureWeight(InRules, Def.Tier) * (1.f + Depth) * ReachFactor(Reach, InRules);
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
		// Law.Hedge (section 4.5 (1)): no pressure reaches the sanctuary - before any source is summed.
		if (CosmicState::IsSanctuary(System.SystemID))
		{
			return Out;
		}
		float Largest = 0.f;

		for (int32 TranscendentIndex = 0; TranscendentIndex < InState.Transcendents.Num(); ++TranscendentIndex)
		{
			const FTranscendentState& Transcendent = InState.Transcendents[TranscendentIndex];
			const FTranscendentDef* Def = Defs.IsValidIndex(TranscendentIndex) ? Defs[TranscendentIndex] : nullptr;
			if (!Def)
			{
				continue;
			}
			AddPressureTerm(Out, Largest, TranscendentTermOf(*Def, InRules, ReachOf(TranscendentIndex)), ECosmicPressureSource::Transcendent, Transcendent.TranscendentID, Def->Tier);
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
	// applied is the one recorded. A throne that changes closes the open Ruler and Regent records (LastTick = max(Tick,
	// 1) - after an abdication the rebels' leader rules, not the regent, B1) and then makes the open Rebel record the
	// Ruler. When another source recorded the end of the vessel's reign first (an abdication, the throne card's failed
	// check - B1 header section 2.8), the throne does not go to the rebels: their open record closes, and the expiry is
	// told by CosmicRows::LogRebellionUnseated (the return value). Either way the count of discontent starts over.
	bool ChangeThrone(FCosmicEvalContext& Ctx, int32 CivIndex, FName HazardRow, float OutcomeNumber)
	{
		static const FGameplayTag DeposeTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Effect.DeposeRuler")));
		static const FGameplayTag RulerTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Notable.Ruler")));
		static const FGameplayTag RegentTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Notable.Regent")));
		static const FGameplayTag RebelTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Notable.Rebel")));
		FGalaxyState& State = Ctx.State;
		if (!State.Civs.IsValidIndex(CivIndex))
		{
			return false;
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
		const bool bUnseated = Civ.bIsPlayerRuled && !bThroneChanges && State.Calling.PendingEffect.IsValid();
		if (bThroneChanges)
		{
			for (FNotableRecord& Record : Civ.Notables)
			{
				if (Record.LastTick == 0 && (Record.Role == RulerTag || Record.Role == RegentTag))
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
		else if (bUnseated)
		{
			for (FNotableRecord& Record : Civ.Notables)
			{
				if (Record.LastTick == 0 && Record.Role == RebelTag)
				{
					Record.LastTick = FMath::Max<int64>(State.Tick, 1);
				}
			}
		}
		Civ.LowStabilityTicks = 0;
		return bUnseated;
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
					// Explicit tags only: the parent tags are derived and rebuilt on load. In FNameLexicalLess order, not the stored
					// order (B3): AddTag appends, so the same set reached by a different history (a treaty broken and signed
					// again) would hash differently while it compares equal.
					const TArray<FGameplayTag>& Tags = static_cast<const FGameplayTagContainer*>(Value)->GetGameplayTagArray();
					TArray<FName> Names;
					Names.Reserve(Tags.Num());
					for (const FGameplayTag& Tag : Tags)
					{
						Names.Add(Tag.GetTagName());
					}
					Names.Sort(FNameLexicalLess());
					AddInt(Names.Num());
					for (const FName Name : Names)
					{
						AddName(Name);
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

	// 1. Systems on a disc. System 0 is sys_0, the home of the first life, at a fixed place.
	const float DiscRadius = DiscRadiusPerRootSystem * FMath::Sqrt(static_cast<float>(SystemCount));
	State.Systems.Reserve(SystemCount);
	for (int32 Index = 0; Index < SystemCount; ++Index)
	{
		FStarSystemState& System = State.Systems.AddDefaulted_GetRef();
		System.SystemID = (Index == 0) ? CosmicIDs::FirstHome : FName(*FString::Printf(TEXT("sys_%d"), Index));
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

	// 2. Homes. The first AI people lives one lane from sys_0, so the first contact comes at 5:30 (section 2.4); every
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
		// The inherited edict (B2 section 2.7): every people starts holding it - the same rule for all. No draw, no cost.
		if (!Rules.StartingEdict.IsNone() && Tables.FindEdict(Rules.StartingEdict))
		{
			Civ.ActiveEdicts.Add(Rules.StartingEdict);
		}
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
	// sys_0, one lane from the home of an AI people and in no sphere yet, in array order. The sanctuary is no candidate:
	// it joins the array only after the last draw (11).
	{
		const TArray<int32> FromHome = CosmicState::LaneDistancesFrom(State, StateIndex, 0);
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
				if (!System.OwnerCivID.IsNone() || InSphere[Index] || FromHome[Index] == INDEX_NONE)
				{
					continue;
				}
				if (FromHome[Index] >= MinLanes && NextToAIHome[Index])
				{
					Candidates.Add(Index);
				}
				if (Farthest == INDEX_NONE || FromHome[Index] > FromHome[Farthest]
					|| (FromHome[Index] == FromHome[Farthest] && FNameLexicalLess()(System.SystemID, State.Systems[Farthest].SystemID)))
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

	// 11. The sanctuary (section 4.5 (1), Law.Hedge), after the last draw of the generation and drawing nothing itself: the
	// number of draws stays the header comment's. A node without a lane (the map holds together without it - v0.8 step 5
	// decides the lanes of the real sky), at a fixed place outside the disc, with the name of ST_CA System.Sanctuary
	// flattened as every name made here. No owner, hazard, project, sphere or core: no rule of the run ever gives it one.
	{
		FStarSystemState& Sanctuary = State.Systems.AddDefaulted_GetRef();
		Sanctuary.SystemID = CosmicIDs::Sanctuary;
		Sanctuary.GalaxyRow = Rules.HomeGalaxyRow;
		Sanctuary.DisplayName = Flatten(FText::FromStringTable(CosmicLabels::TableId, TEXT("System.Sanctuary"), EStringTableLoadingPolicy::FindOrFullyLoad));
		Sanctuary.Position = FVector(SanctuaryRadiusShare * DiscRadius, 0.0, 0.0);
		Sanctuary.StarClass = FGameplayTag::RequestGameplayTag(SanctuaryStarClass);
		Sanctuary.ResourceRichness = 0.f;
	}

	StateIndex.Rebuild(State);

	// The first life: ruler of the player civ, through the same function succession uses (section 5.10).
	UPlayerRoleSubsystem* PlayerRole = GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
	check(PlayerRole);
	PlayerRole->Install(CosmicIDs::FirstHome, State.Civs[0].CivID, EPlayerRole::Ruler);

	SyncRngState();
	Accumulator = 0.f;
	bRunning = true;

	UE_LOG(LogCosmic, Log, TEXT("NewGalaxy: seed %d, %d systems and the sanctuary, %d civs, galaxy %s"), Seed, SystemCount, CivCount, *Rules.HomeGalaxyRow.ToString());
	if (bBroadcastEnabled)
	{
		OnGalaxyGenerated.Broadcast(Seed);
	}
}

void UGalaxySimulationSubsystem::SetTimeScale(float Scale)
{
	// A compare, so a scale that is not a number pauses: FMath::Clamp would read it as its top (CLAUDE.md, the API table).
	TimeScale = Scale > 0.f ? FMath::Min(Scale, MaxTimeScale) : 0.f;
	// A pause drops the carried-over time too (section 16 #40 (3)): resuming never runs the leftover in one burst. The
	// ticks themselves are the same; only the frames they fall in move.
	if (!(TimeScale > 0.f))
	{
		Accumulator = 0.f;
	}
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

bool UGalaxySimulationSubsystem::NoteMet(FName CivID, FName SeenAtSystemID)
{
	if (IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("NoteMet called from a handler of a Core Logic delegate: a handler may only read the state"));
		return false;
	}
	FKnowingState& Knowing = State.Knowing;
	const int32 CivIndex = StateIndex.FindCiv(State, CivID);
	if (State.Calling.LifeState != ELifeState::Embodied || !State.Civs.IsValidIndex(CivIndex)
		|| Knowing.MetCivs.ContainsByPredicate([CivID](const FKnownMark& Mark) { return Mark.ID == CivID; }))
	{
		return false;
	}
	// The gap rule (section 3.2): a people three rungs or more above the viewer is seen, not met.
	if (static_cast<int32>(State.Civs[CivIndex].Tier) - static_cast<int32>(GetViewerTier()) >= 3)
	{
		return false;
	}

	FKnownMark& Mark = Knowing.MetCivs.AddDefaulted_GetRef();
	Mark.ID = CivID;
	Mark.Tick = State.Tick;
	// The first meeting of the run is the first contact, once (section 7.4): Install never writes this field. An embodied
	// ruler is then shown its card, while the table holds the row (B3 header section 2.9 - the end of the knowing stage
	// puts it).
	const bool bFirstContact = Knowing.FirstContactCivID.IsNone();
	if (bFirstContact)
	{
		Knowing.FirstContactCivID = CivID;
		if (CosmicPetitions::RuledCivIndexOf(State, StateIndex) != INDEX_NONE && Tables.FindEncounter(CosmicRows::PetitionFirstContact))
		{
			Knowing.PendingContactCivID = CivID;
		}
	}

	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Log);
	FFormatNamedArguments Args;
	Args.Add(TEXT("Civ"), State.Civs[CivIndex].CivName);
	Log->PushRow(bFirstContact ? CosmicRows::LogFirstContact : CosmicRows::LogContact, 1, Args, SeenAtSystemID, CivID);
	return true;
}

bool UGalaxySimulationSubsystem::NoteKnown(FName SystemID)
{
	if (IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("NoteKnown called from a handler of a Core Logic delegate: a handler may only read the state"));
		return false;
	}
	FKnowingState& Knowing = State.Knowing;
	if (State.Calling.LifeState != ELifeState::Embodied || !State.Systems.IsValidIndex(StateIndex.FindSystem(State, SystemID))
		|| Knowing.KnownSystems.ContainsByPredicate([SystemID](const FKnownMark& Mark) { return Mark.ID == SystemID; }))
	{
		return false;
	}
	FKnownMark& Mark = Knowing.KnownSystems.AddDefaulted_GetRef();
	Mark.ID = SystemID;
	Mark.Tick = State.Tick;
	return true;
}

EAscensionTier UGalaxySimulationSubsystem::GetViewerTier() const
{
	if (State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Ruler)
	{
		const int32 CivIndex = StateIndex.FindCiv(State, State.Player.RuledCivID);
		if (State.Civs.IsValidIndex(CivIndex))
		{
			return State.Civs[CivIndex].Tier;
		}
	}
	return State.Calling.PersonalTier;
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

FCosmicPressure UGalaxySimulationSubsystem::ComputeAttritionPressureIn(const FGalaxyState& InState, const FCosmicStateIndex& InIndex, const FCosmicDataTables& InTables, const FCosmicRulesDef& InRules, int32 CivIndex, int32 SystemIndex)
{
	FCosmicPressure Out;
	if (!InState.Civs.IsValidIndex(CivIndex) || !InState.Systems.IsValidIndex(SystemIndex) || CosmicState::IsSanctuary(InState.Systems[SystemIndex].SystemID))
	{
		return Out;
	}
	const int32 CivTier = TierIndex(InState.Civs[CivIndex].Tier);
	float Largest = 0.f;
	for (int32 TranscendentIndex = 0; TranscendentIndex < InState.Transcendents.Num(); ++TranscendentIndex)
	{
		const FTranscendentState& Transcendent = InState.Transcendents[TranscendentIndex];
		const FTranscendentDef* Def = Transcendent.bSealed ? nullptr : InTables.FindTranscendent(Transcendent.TranscendentID);
		if (!Def || TierIndex(Def->Tier) <= CivTier || SphereReachIn(InState, Transcendent, SystemIndex) != ECosmicSphereReach::Inside)
		{
			continue;
		}
		AddPressureTerm(Out, Largest, TranscendentTermOf(*Def, InRules, ECosmicSphereReach::Inside), ECosmicPressureSource::Transcendent, Transcendent.TranscendentID, Def->Tier);
	}
	return Out;
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
	if (IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("DrawTell called from a handler of a Core Logic delegate: a handler may only read the state"));
		return 0;
	}
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
		GET_MEMBER_NAME_CHECKED(FGalaxyState, ChronicleEviction),
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

	// The petition rows that hold at the head of the tick, before the tick advances: the knowing stage puts a card for a
	// Standing row only when its conditions came to hold in this tick (section 5.10). A read into a local - no state, no
	// member - and empty while a card waits or no ruler is embodied.
	// The first tick of a reign compares with an empty head (B1): what already stood when the vessel was crowned comes as
	// a card that just came to hold. The first life's first tick is one too - no petition stands at tick 0. A travelling
	// wanderer counts as standing in the system it left (B1 header section 2.4): a journey that ends where it began brings
	// no card for a crisis that stood there throughout.
	TArray<FCosmicPetitionUnit> PetitionsAtHead = CosmicPetitions::StandingHeld(State, StateIndex, Tables, Rules, true);
	if (CosmicPetitions::IsFirstTickOfReign(State))
	{
		PetitionsAtHead.Reset();
	}

	// Tick 0 is the generated state: no event raised inside a tick stage carries tick 0, so the "0 = unset" tick fields
	// stay unambiguous there. Player input outside the tick can still run at tick 0.
	++State.Tick;
	// Where the vessel stood when the tick began: the sight stage looks again when it has moved, by a journey or by a
	// destruction that carried it (section 7.4).
	const FName VesselAtStart = State.Player.CurrentSystemID;

	{
		// The stages hold references into the state, and OnLogEntry fires from inside them (section 9.4).
		TGuardValue<bool> StageGuard(bInTickStages, true);
		TickEconomy();
		TickResearch();
		// The stage clock (measurement only): laps around four stages, nothing else.
		const bool bClock = StageClock.bOn;
		uint64 Lap = bClock ? FPlatformTime::Cycles64() : 0;
		auto Mark = [this, bClock, &Lap](int32 At)
		{
			if (bClock)
			{
				const uint64 Now = FPlatformTime::Cycles64();
				StageClock.Cycles[At] += Now - Lap;
				Lap = Now;
			}
		};
		TickDiplomacy();
		Mark(0);
		TickConflict();
		Mark(1);
		TickHazards();
		Mark(2);
		TickCivAI();
		Mark(3);
		StageClock.Ticks += bClock ? 1 : 0;
		TickCrossings();
		// The calling rests on no vessel during the interregnum leap (section 4.4): no travel and no sight.
		if (State.Calling.LifeState == ELifeState::Embodied)
		{
			const bool bArrived = TickPlayerTravel();
			TickSight(State.Player.CurrentSystemID != VesselAtStart);
			// The end of the knowing stage: the waiting card - a petition put before the ruler (section 5.10), a card put
			// before the wanderer where it stands (B1). Knowing only, no draw of the sim stream. A wanderer's card names the
			// people of its system: that people is met first (the meeting of docs/70, the gap rule of NoteMet).
			if (CosmicPetitions::Present(State, StateIndex, Tables, Rules, PetitionsAtHead, bArrived) && State.Player.Role == EPlayerRole::Wanderer)
			{
				const int32 CardSystem = StateIndex.FindSystem(State, State.Knowing.PendingPetitionSystemID);
				const FName Owner = State.Systems.IsValidIndex(CardSystem) ? State.Systems[CardSystem].OwnerCivID : FName();
				if (!Owner.IsNone())
				{
					NoteMet(Owner, State.Knowing.PendingPetitionSystemID);
				}
			}
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

	// The population comes back toward 1.0 (section 4.1). A compare, so a knob that is not a number gives no regrowth.
	const float Regrowth = (Rules.PopulationRegrowthPerTick > 0.f && Rules.PopulationRegrowthPerTick <= TNumericLimits<float>::Max()) ? Rules.PopulationRegrowthPerTick : 0.f;
	// Stability comes back toward its resting level (section 4.1, 16 #68), in the same shape. Compares, so a knob that is
	// not a number gives no recovery.
	const bool bRecovers = Rules.StabilityRecoveryPerTick > 0.f && Rules.StabilityRecoveryPerTick <= 1.f && Rules.StabilityRestingLevel >= 0.f
		&& Rules.StabilityRestingLevel <= 1.f;
	// Society research raises the recovery (B2): x (1 + bonus x Society). A bonus that is not above 0 and finite is 0, and
	// then the recovery is the knob itself, bit for bit.
	const float SocietyBonus = (Rules.SocietyRecoveryBonus > 0.f && Rules.SocietyRecoveryBonus <= TNumericLimits<float>::Max()) ? Rules.SocietyRecoveryBonus : 0.f;
	UCosmicLogSubsystem* EdictLog = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();

	for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
	{
		FCivilizationState& Civ = State.Civs[CivIndex];
		if (Regrowth > 0.f && Civ.Population < 1.f)
		{
			Civ.Population = FMath::Min(1.f, Civ.Population + Regrowth);
		}
		if (bRecovers && Civ.Stability < Rules.StabilityRestingLevel)
		{
			// Society read 0..1 by comparisons (a value that is not a number reads as 0).
			const float Society = Civ.Research.FindRef(SocietyField);
			const float SocietyRead = Society > 0.f ? (Society < 1.f ? Society : 1.f) : 0.f;
			const float Recovery = SocietyBonus > 0.f ? Rules.StabilityRecoveryPerTick * (1.f + SocietyBonus * SocietyRead) : Rules.StabilityRecoveryPerTick;
			Civ.Stability = FMath::Min(Rules.StabilityRestingLevel, Civ.Stability + Recovery);
		}

		// Only settled systems pay (section 5.7): a system being settled is not yet a home.
		float Richness = 0.f;
		float StarOutput = 0.f;
		for (const FName SystemID : Civ.OwnedSystems)
		{
			const int32 SystemIndex = StateIndex.FindSystem(State, SystemID);
			if (State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == Civ.CivID && CosmicState::IsSettled(State.Systems[SystemIndex]))
			{
				Richness += State.Systems[SystemIndex].ResourceRichness;
				StarOutput += StarEnergy(State.Systems[SystemIndex].StarClass);
			}
		}

		// Energy comes from the stars owned, Matter from the richness of the systems, Insight and Influence from the
		// people (section 5.1). Causality stays at zero: it opens with Tier 3 research.
		const float TierMultiplier = TierMultiplierOf(Rules, Civ.Tier);
		FResourcePool Income;
		Income.Energy = StarOutput * TierMultiplier * (1.f + EconomyRateOf(Rules.MaterialistEnergyBonus) * FMath::Max(0.f, -Civ.Ethics.Spiritualism));
		Income.Matter = Richness * TierMultiplier;
		Income.Insight = Civ.Population * EconomyRateOf(Rules.InsightPerPopulation) * (1.f + EconomyRateOf(Rules.SpiritualistInsightBonus) * FMath::Max(0.f, Civ.Ethics.Spiritualism));
		Income.Influence = Civ.Population * EconomyRateOf(Rules.InfluencePerPopulation) * (1.f + EconomyRateOf(Rules.OpenInfluenceBonus) * FMath::Max(0.f, Civ.Ethics.Openness));
		// Each trade treaty adds Influence (B3 section 2.5), from the tick after it was signed. No treaty, no term: the bits stay.
		const int32 TradeTreaties = CosmicDiplomacy::TradeTreatyCountOf(Civ);
		if (TradeTreaties > 0)
		{
			Income.Influence += EconomyRateOf(Rules.TradeInfluencePerTreaty) * static_cast<float>(TradeTreaties);
		}

		Civ.IncomePerTick = Income;

		// The per-tick sources, between the income and the stockpile (section 7.4). Trait rows join in Phase 4.
		// The held edicts first (B2), in ActiveEdicts order - before the drain and the transcendents' sources, an order that
		// fixes the bits of the multiplications. Every field of the subject is set: the context is reused across peoples
		// and the transcendents' loop leaves its OtherID behind. A name the table does not hold does nothing.
		for (int32 EdictAt = 0; EdictAt < Civ.ActiveEdicts.Num(); ++EdictAt)
		{
			const FEdictDef* Edict = Tables.FindEdict(Civ.ActiveEdicts[EdictAt]);
			if (!Edict || Edict->EffectsPerTick.IsEmpty())
			{
				continue;
			}
			Sources.CivIndex = CivIndex;
			Sources.SystemIndex = INDEX_NONE;
			Sources.OtherID = NAME_None;
			Sources.SourceRow = Civ.ActiveEdicts[EdictAt];
			Sources.GalaxyRow = Civ.GalaxyRow;
			Sources.SinceTick = 0;
			UCosmicEventSubsystem::ApplyAll(Edict->EffectsPerTick, Sources);
		}
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

		// The edicts' upkeep (B2), after this tick's income, in ActiveEdicts order: paid in full, or the edict lapses (its
		// effects of this tick have been applied). Only the people the player rules is told (the stage's only line - the
		// tell stream; no draw, no role effect).
		if (!Civ.ActiveEdicts.IsEmpty())
		{
			const TArray<FName> Held = Civ.ActiveEdicts;
			for (const FName Row : Held)
			{
				const FEdictDef* Edict = Tables.FindEdict(Row);
				if (!Edict || CosmicEdicts::TryPay(Civ.Stockpile, Edict->UpkeepPerTick))
				{
					continue;
				}
				CosmicEdicts::Revoke(Civ, Row);
				if (Civ.bIsPlayerRuled && EdictLog)
				{
					FFormatNamedArguments Args;
					Args.Add(TEXT("Civ"), Civ.CivName);
					Args.Add(TEXT("Edict"), Edict->DisplayName);
					EdictLog->PushRow(CosmicRows::LogEdictLapsed, 1, Args, NAME_None, Civ.CivID);
				}
			}
		}
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
		// A people with no system left (the one the player ruled, kept for the tick of its loss) researches nothing more:
		// no breakthrough line and no book about a people that is gone (section 7.4 (o)).
		if (!CosmicState::OwnsAnySystem(State, StateIndex, CivIndex))
		{
			continue;
		}
		FCivilizationState& Civ = State.Civs[CivIndex];
		const float Spent = FMath::Max(0.f, FMath::Min(Civ.Stockpile.Insight, Civ.IncomePerTick.Insight * ResearchShareOf(Rules)));
		Civ.Stockpile.Insight -= Spent;

		for (int32 FieldIndex = 0; FieldIndex < static_cast<int32>(UE_ARRAY_COUNT(ResearchFields)); ++FieldIndex)
		{
			const FName Field = ResearchFields[FieldIndex];
			const float Before = Civ.Research.FindRef(Field);
			const float After = FMath::Min(1.f, Before + Spent * Civ.ResearchFocus.FindRef(Field) * EconomyRateOf(Rules.ResearchPerInsight));
			Civ.Research.Add(Field, After);

			// A breakthrough is told once per mark crossed. Every people has a name from its birth (section 5.11); whether
			// the line is told is the story scope's question, never the world's (section 8.1). The line calls the people
			// and no place, so it has no subject system: a people met is told of without its capital being known.
			for (const float Threshold : BreakthroughThresholds)
			{
				if (Before < Threshold && After >= Threshold)
				{
					FFormatNamedArguments Args;
					Args.Add(TEXT("Civ"), Civ.CivName);
					Log->PushRow(CosmicRows::LogBreakthrough[FieldIndex], Civ.bIsPlayerRuled ? 1 : 0, Args, NAME_None, Civ.CivID);

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
	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Log);
	const FGameplayTag Friendly = CosmicTags::StanceFriendly();

	// (1) The friendly stances' upkeep (B3 section 2.5). Only a people with ties has anything to pay.
	const float Upkeep = (Rules.FriendlyUpkeepInfluence > 0.f && Rules.FriendlyUpkeepInfluence <= TNumericLimits<float>::Max()) ? Rules.FriendlyUpkeepInfluence : 0.f;
	for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
	{
		FCivilizationState& Civ = State.Civs[CivIndex];
		if (Civ.Ties.IsEmpty())
		{
			continue;
		}
		TArray<FName, TInlineAllocator<4>> Friends;
		for (const TPair<FName, FCosmicTie>& Pair : Civ.Ties)
		{
			if (Pair.Value.Stance == Friendly)
			{
				Friends.Add(Pair.Key);
			}
		}
		if (Friends.IsEmpty())
		{
			continue;
		}
		Friends.Sort(FNameLexicalLess());
		TArray<FName, TInlineAllocator<4>> Lapsed;
		const int32 Slots = CosmicDiplomacy::FriendlySlotsOf(Civ, Rules);
		while (Friends.Num() > Slots)
		{
			Lapsed.Add(Friends.Pop());
		}
		for (const FName Friend : Friends)
		{
			if (Upkeep > 0.f && !(Civ.Stockpile.Influence >= Upkeep))
			{
				Lapsed.Add(Friend);
				continue;
			}
			Civ.Stockpile.Influence -= Upkeep;
		}
		for (const FName Friend : Lapsed)
		{
			CosmicDiplomacy::SetStanceOf(Civ, Friend, FGameplayTag());
			const int32 Other = StateIndex.FindCiv(State, Friend);
			if (Civ.bIsPlayerRuled && State.Civs.IsValidIndex(Other))
			{
				FFormatNamedArguments Args;
				Args.Add(TEXT("Civ"), Civ.CivName);
				Args.Add(TEXT("OtherCiv"), State.Civs[Other].CivName);
				Log->PushRow(CosmicRows::LogStanceLapsed, 1, Args, NAME_None, Civ.CivID);
			}
		}
	}

	// (2) The drift of every bordering pair (B3 section 2.5): the targets read stances, treaties and ethics only, never a
	// relation, so one pair's write never reaches another pair's read. A rate of 0 walks no border and writes nothing.
	const float Rate = (Rules.RelationDriftRate > 0.f && Rules.RelationDriftRate <= 1.f) ? Rules.RelationDriftRate : 0.f;
	if (!(Rate > 0.f))
	{
		return;
	}
	// One viewer's borders at a time (BorderTableOf's rows, in its order), one array kept across the viewers.
	TArray<bool> Borders;
	for (int32 Viewer = 0; Viewer < State.Civs.Num(); ++Viewer)
	{
		CosmicDiplomacy::BordersOf(State, StateIndex, Viewer, Borders);
		for (int32 Other = 0; Other < Borders.Num(); ++Other)
		{
			if (!Borders[Other])
			{
				continue;
			}
			const float Target = CosmicDiplomacy::TargetRelationOf(State, Rules, Viewer, Other);
			FCivilizationState& Civ = State.Civs[Viewer];
			const FName OtherID = State.Civs[Other].CivID;
			const float Before = Civ.Relations.FindRef(OtherID);
			const float After = FMath::Clamp(Before + Rate * (Target - Before), -1.f, 1.f);
			if (After != Before && After >= -1.f && After <= 1.f)
			{
				Civ.Relations.Add(OtherID, After);
			}
		}
	}
}

void UGalaxySimulationSubsystem::TickConflict()
{
	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Log);
	UCosmicEventSubsystem* Events = GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>();
	check(Events);
	const auto RateOf = [](float Value) { return (Value > 0.f && Value <= TNumericLimits<float>::Max()) ? Value : 0.f; };

	// A line of a siege the people the player rules is a party to (B3 section 3.4): the other people is met first, then
	// told. Nothing when Ruled is not the ruled people.
	const auto TellSiege = [this, Log](FName Row, uint8 Severity, int32 Ruled, int32 Other, int32 SystemIndex)
	{
		if (!State.Civs.IsValidIndex(Ruled) || !State.Civs[Ruled].bIsPlayerRuled || !State.Civs.IsValidIndex(Other) || !State.Systems.IsValidIndex(SystemIndex))
		{
			return;
		}
		const FName SystemID = State.Systems[SystemIndex].SystemID;
		NoteMet(State.Civs[Other].CivID, SystemID);
		FFormatNamedArguments Args;
		Args.Add(TEXT("Civ"), State.Civs[Ruled].CivName);
		Args.Add(TEXT("OtherCiv"), State.Civs[Other].CivName);
		Args.Add(TEXT("System"), State.Systems[SystemIndex].DisplayName);
		Log->PushRow(Row, Severity, Args, SystemID, State.Civs[Ruled].CivID);
	};
	// A lifted siege is told to the ruled side, attacker or owner (the line names no side).
	const auto TellLifted = [&TellSiege](int32 Attacker, int32 Owner, int32 SystemIndex)
	{
		TellSiege(CosmicRows::LogSiegeLifted, 1, Attacker, Owner, SystemIndex);
		TellSiege(CosmicRows::LogSiegeLifted, 1, Owner, Attacker, SystemIndex);
	};

	// (1) The fleets.
	const float Growth = RateOf(Rules.FleetGrowthPerTick);
	const float UpkeepPerPower = RateOf(Rules.FleetUpkeepEnergyPerPower);
	const float Attrition = RateOf(Rules.PressureFleetAttrition);
	for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
	{
		FCivilizationState& Civ = State.Civs[CivIndex];
		const float Cap = CosmicConflict::FleetCapOf(State, StateIndex, CivIndex, Rules);
		if (!(Cap > 0.f) && !(Civ.FleetPower > 0.f))
		{
			continue;
		}
		const float Upkeep = UpkeepPerPower * FMath::Max(0.f, Civ.FleetPower);
		if (!(Upkeep > 0.f) || Civ.Stockpile.Energy >= Upkeep)
		{
			Civ.Stockpile.Energy -= Upkeep;
			Civ.FleetPower = FMath::Min(Cap, Civ.FleetPower + Growth * CosmicConflict::FleetGrowthMultiplierOf(Civ, Tables));
		}
		else
		{
			Civ.FleetPower = FMath::Max(0.f, Civ.FleetPower - Growth);
		}
		if (Civ.FleetPower > Cap)
		{
			Civ.FleetPower = Cap;
		}
		if (!(Civ.FleetPower > 0.f))
		{
			Civ.FleetPower = 0.f;
			continue;
		}
		if (Attrition > 0.f && !Civ.FleetTargetSystemID.IsNone())
		{
			const float Pressure = ComputeAttritionPressureIn(State, StateIndex, Tables, Rules, CivIndex, StateIndex.FindSystem(State, Civ.FleetTargetSystemID)).P;
			if (Pressure > 0.f)
			{
				Civ.FleetPower = FMath::Max(0.f, Civ.FleetPower - Attrition * Pressure);
			}
		}
	}

	// (2) The sieges at their end, collected by the stored state alone, and one number each, all drawn first (rule 3).
	struct FDueSiege
	{
		int32 Attacker = INDEX_NONE;
		int32 SystemIndex = INDEX_NONE;
		float Number = 0.f;
	};
	TArray<FDueSiege, TInlineAllocator<4>> Due;
	for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
	{
		const FCivilizationState& Civ = State.Civs[CivIndex];
		if (Civ.FleetTargetSystemID.IsNone())
		{
			continue;
		}
		const int32 SystemIndex = StateIndex.FindSystem(State, Civ.FleetTargetSystemID);
		const int32 Owner = State.Systems.IsValidIndex(SystemIndex) ? StateIndex.FindCiv(State, State.Systems[SystemIndex].OwnerCivID) : INDEX_NONE;
		if (State.Civs.IsValidIndex(Owner) && Owner != CivIndex && CosmicConflict::IsSiegeDue(Civ, State.Tick, Rules))
		{
			Due.Add({ CivIndex, SystemIndex, 0.f });
		}
	}
	for (FDueSiege& Siege : Due)
	{
		Siege.Number = Rng.GetFraction();
	}

	// The ends the stage decides, noted for the stage clock only (measurement - FCosmicStageClock).
	auto NoteEnd = [this](FCosmicStageClock::ESiegeEnd Kind, FName AttackerID, FName OwnerID, int32 SystemIndex)
	{
		if (StageClock.bOn)
		{
			StageClock.SiegeEnds.Add({ Kind, AttackerID, OwnerID, State.Systems.IsValidIndex(SystemIndex) ? State.Systems[SystemIndex].SystemID : FName() });
		}
	};

	// (3) Every target asked its gate. A lifted target starts the rest of an AI; a collected siege's number is thrown away.
	for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
	{
		FCivilizationState& Civ = State.Civs[CivIndex];
		if (Civ.FleetTargetSystemID.IsNone())
		{
			continue;
		}
		const int32 SystemIndex = StateIndex.FindSystem(State, Civ.FleetTargetSystemID);
		const bool bOwn = State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == Civ.CivID;
		const bool bHolds = bOwn ? CosmicConflict::GuardBlockOf(State, StateIndex, Rules, CivIndex, SystemIndex) == CosmicConflict::ECosmicGuardBlock::None
			: CosmicConflict::SiegeBlockOf(State, StateIndex, Rules, CivIndex, SystemIndex) == CosmicConflict::ECosmicSiegeBlock::None;
		if (bHolds)
		{
			continue;
		}
		const int32 Owner = State.Systems.IsValidIndex(SystemIndex) ? StateIndex.FindCiv(State, State.Systems[SystemIndex].OwnerCivID) : INDEX_NONE;
		Civ.FleetTargetSystemID = NAME_None;
		Civ.FleetTargetSinceTick = State.Tick;
		Due.RemoveAll([CivIndex](const FDueSiege& Siege) { return Siege.Attacker == CivIndex; });
		if (!bOwn)
		{
			NoteEnd(FCosmicStageClock::ESiegeEnd::Lifted, Civ.CivID, State.Civs.IsValidIndex(Owner) ? State.Civs[Owner].CivID : FName(), SystemIndex);
			TellLifted(CivIndex, Owner, SystemIndex);
		}
	}

	// (4) The ends, in the order collected, each gate asked again right before (an earlier taking may close a later gate).
	const float Repulsed = CosmicConflict::RepulsedShareOf(Rules);
	for (const FDueSiege& Siege : Due)
	{
		const int32 Attacker = Siege.Attacker;
		const int32 SystemIndex = Siege.SystemIndex;
		const int32 Owner = StateIndex.FindCiv(State, State.Systems[SystemIndex].OwnerCivID);
		if (CosmicConflict::SiegeBlockOf(State, StateIndex, Rules, Attacker, SystemIndex) != CosmicConflict::ECosmicSiegeBlock::None)
		{
			State.Civs[Attacker].FleetTargetSystemID = NAME_None;
			State.Civs[Attacker].FleetTargetSinceTick = State.Tick;
			NoteEnd(FCosmicStageClock::ESiegeEnd::Lifted, State.Civs[Attacker].CivID, State.Civs.IsValidIndex(Owner) ? State.Civs[Owner].CivID : FName(), SystemIndex);
			TellLifted(Attacker, Owner, SystemIndex);
			continue;
		}
		const FName AttackerID = State.Civs[Attacker].CivID;
		const FName OwnerID = State.Civs[Owner].CivID;
		bool bTaken = false;
		if (CosmicConflict::Takes(Siege.Number, CosmicConflict::SiegeOddsOf(State, StateIndex, Rules, Attacker, SystemIndex)))
		{
			FCosmicEvalContext Ctx = MakeEvalContext();
			bTaken = CosmicHazards::Transfer(Ctx, SystemIndex, Attacker);
			if (bTaken)
			{
				Events->FollowUpKnowing(Ctx);
				// The neighbours of the conqueror right after the taking, and the loser, think less of it.
				for (int32 Viewer = 0; Viewer < State.Civs.Num(); ++Viewer)
				{
					if (Viewer != Attacker && (Viewer == Owner || CosmicDiplomacy::Borders(State, StateIndex, Viewer, Attacker)))
					{
						CosmicDiplomacy::LowerRelation(State.Civs[Viewer], AttackerID, Rules.ConquestRelationPenalty);
					}
				}
			}
			else
			{
				UE_LOG(LogCosmic, Error, TEXT("TickConflict: the siege of %s by %s passed its gate and Transfer refused it: read as held"), *State.Systems[SystemIndex].SystemID.ToString(), *AttackerID.ToString());
			}
		}
		FCivilizationState& Civ = State.Civs[Attacker];
		if (!bTaken)
		{
			Civ.FleetPower = FMath::Max(0.f, Civ.FleetPower * Repulsed);
		}
		NoteEnd(bTaken ? FCosmicStageClock::ESiegeEnd::Taken : FCosmicStageClock::ESiegeEnd::Held, AttackerID, OwnerID, SystemIndex);
		// A truce and the rest, either way.
		CosmicDiplomacy::SetStanceOf(Civ, OwnerID, FGameplayTag());
		Civ.FleetTargetSystemID = NAME_None;
		Civ.FleetTargetSinceTick = State.Tick;
		TellSiege(bTaken ? CosmicRows::LogSiegeTaken : CosmicRows::LogSiegeFailed, 2, Attacker, Owner, SystemIndex);
		TellSiege(bTaken ? CosmicRows::LogSiegeLost : CosmicRows::LogSiegeHeld, 2, Owner, Attacker, SystemIndex);
	}
}

void UGalaxySimulationSubsystem::TickHazards()
{
	UCosmicEventSubsystem* Events = GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>();
	check(Events);

	// (a) Megaprojects and the resolutions their completions make, before any countdown.
	AdvanceMegaprojects();

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
	// The plague's spread (B3 section 2.7): the settled systems with a standing plague on a lane neighbour as this step
	// begins - a plague placed in the loop below is not read, so the order of the systems tilts nothing. A multiplier of 1
	// spreads nothing and builds no table; one that is not at least 0 and finite reads as 1.
	const FGameplayTag PlagueTag = CosmicTags::HazardPlague();
	const float Spread = (Rules.PlagueSpreadMultiplier >= 0.f && Rules.PlagueSpreadMultiplier <= TNumericLimits<float>::Max()) ? Rules.PlagueSpreadMultiplier : 1.f;
	TBitArray<> NextToPlague;
	if (Spread != 1.f)
	{
		NextToPlague.Init(false, State.Systems.Num());
		for (const int32 SystemIndex : Settled)
		{
			for (const FName Lane : State.Systems[SystemIndex].Lanes)
			{
				const int32 Neighbour = StateIndex.FindSystem(State, Lane);
				if (State.Systems.IsValidIndex(Neighbour) && State.Systems[Neighbour].ActiveHazard == PlagueTag)
				{
					NextToPlague[SystemIndex] = true;
					break;
				}
			}
		}
	}
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
			const float Factor = Multiplier ? ((*Multiplier > 0.f && *Multiplier <= TNumericLimits<float>::Max()) ? *Multiplier : 0.f) : 1.f;
			// The owner's held edicts multiply the share too (B2), and a plague next door (B3): 1 when none, so the bits stay.
			const float Spreading = (Row->HazardTag == PlagueTag && NextToPlague.IsValidIndex(SystemIndex) && NextToPlague[SystemIndex]) ? Spread : 1.f;
			const float Share = ((Chance * Factor) * CosmicEdicts::SpawnMultiplierOf(Owner, Tables, Row->HazardTag)) * Spreading;
			if (!(Share > 0.f))
			{
				continue;
			}
			if (!(Share <= TNumericLimits<float>::Max() - Total))
			{
				UE_LOG(LogCosmic, Error, TEXT("TickHazards: the share of %s on %s would carry the sum past the largest float, read as 0"), *RowName.ToString(), *System.SystemID.ToString());
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

	// The first crisis (section 2.4, 16 #91): no draw. Every people's capital, in Civs order, in one tick.
	if (Rules.FirstCrisisTick > 0 && State.Tick == Rules.FirstCrisisTick && Rules.FirstCrisisHazard.IsValid())
	{
		FName FirstRow;
		const FHazardDef* Row = Tables.FindHazardByTag(Rules.FirstCrisisHazard, &FirstRow);
		// Tables without the row (a test's own hazards) have no first crisis; Data.Integrity holds the game's row to a Draw row.
		if (Row && Row->SpawnRule != ECosmicHazardSpawn::Draw)
		{
			UE_LOG(LogCosmic, Error, TEXT("TickHazards: DT_Rules.FirstCrisisHazard %s is not the tag of a Draw row of DT_Hazards: no first crisis"), *Rules.FirstCrisisHazard.ToString());
		}
		else if (Row)
		{
			for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
			{
				const int32 Capital = StateIndex.FindSystem(State, State.Civs[CivIndex].HomeSystemID);
				if (!State.Systems.IsValidIndex(Capital) || State.Systems[Capital].OwnerCivID != State.Civs[CivIndex].CivID)
				{
					continue;
				}
				FCosmicEvalContext Ctx = MakeEvalContext();
				if (CosmicHazards::Place(Ctx, Capital, FirstRow))
				{
					Events->FollowUpNotes(Ctx);
				}
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
		CosmicHazards::EmptyHazardSlot(State.Systems[SystemIndex]);
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

	// An answer edict its owner holds at the expiry resolves the hazard instead (B2 section 2.5): the resolution of a project
	// that completes - its effects and its line. The numbers drawn for this expiry are thrown away (the draws stay), and a
	// resolution passes no claim rule.
	if (State.Civs.IsValidIndex(OwnerIndex) && !CosmicEdicts::HeldAnswerOf(State.Civs[OwnerIndex], *Row, Tables).IsNone())
	{
		CosmicHazards::Resolve(Ctx, SystemIndex);
		Events->FollowUpKnowing(Ctx);
		Events->FollowUpNotes(Ctx);
		return;
	}

	// The gate comes before any part of the loss (section 4.5): a kept system loses nothing, not even its line.
	const bool bTakesSystem = Collected == SupernovaTag;
	const bool bLastAtGate = bTakesSystem && CosmicHazards::IsLastSettledSystem(State, StateIndex, SystemIndex);
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
	// What the vessel's people received through the effects is known, and the people it came from met, before any line
	// of the expiry: the grace line of the gate below as much as the expiry's own (section 8.1). The code hooks below
	// hand no system to a people. No row of the game hands one over here (Data.Integrity): this is defence.
	Events->FollowUpKnowing(Ctx);

	// The code of the tag: what no effect may say (section 16 #29 (5)).
	bool bJoined = true;
	bool bUnseated = false;
	if (Collected == SupernovaTag)
	{
		// The gate was asked before the effects. Should an effect have let the owner's other systems go meanwhile (no row
		// of the game can: Data.Integrity keeps TransferSystem out of the expiry effects and out of OnResolveEffects, which
		// a ResolveHazard on a neighbour runs), it is asked again here: a people reaches no system only behind the gate. A
		// system kept here has taken the expiry's effects and its damage to the neighbours already.
		if (!bLastAtGate && CosmicHazards::IsLastSettledSystem(State, StateIndex, SystemIndex))
		{
			UE_LOG(LogCosmic, Error, TEXT("Expiry of %s: its effects left %s the last settled system of its people; the last-system gate is asked again"), *RowName.ToString(), *State.Systems[SystemIndex].SystemID.ToString());
			if (GraceKeepsLastSystem(Ctx, SystemIndex))
			{
				EmptySlot();
				Events->FollowUpNotes(Ctx);
				return;
			}
		}
		CosmicHazards::Destroy(Ctx, SystemIndex);
		// The collapse of the people the vessel rules: the deposition pipeline (section 4.4). Destroy has let the people's
		// settlements go with its last settled system (section 5.7). Apply keeps the rules of the record (Embodied, the
		// ruled people, first recorded wins).
		if (State.Civs.IsValidIndex(OwnerIndex) && !CosmicState::OwnsAnySystem(State, StateIndex, OwnerIndex))
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
		bUnseated = ChangeThrone(Ctx, OwnerIndex, RowName, OutcomeNumber);
	}
	else if (Collected == ClaimTag)
	{
		bJoined = JoinClaimedSphere(Ctx, SystemIndex);
	}
	EmptySlot();

	// The claim rule on the slot this expiry left free (section 4.2).
	TryPlaceClaim(Ctx, SystemIndex, ClaimNumber);

	// The expiry's own line, then what followed from it. A rebellion that took no throne (another source ended the reign
	// first) is told without the throne (B1): the same one draw of the tell stream.
	if (bUnseated)
	{
		UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
		check(Log);
		FFormatNamedArguments Args;
		Args.Add(TEXT("Civ"), State.Civs.IsValidIndex(OwnerIndex) ? State.Civs[OwnerIndex].CivName : FText::GetEmpty());
		Args.Add(TEXT("System"), State.Systems[SystemIndex].DisplayName);
		Log->PushRow(CosmicRows::LogRebellionUnseated, Row->Severity, Args, State.Systems[SystemIndex].SystemID,
			State.Civs.IsValidIndex(OwnerIndex) ? State.Civs[OwnerIndex].CivID : FName());
	}
	else
	{
		Events->TellHazardLine(Ctx, bJoined ? ECosmicHazardLine::Expiry : ECosmicHazardLine::Resolution, RowName, SystemIndex, OwnerIndex);
	}
	Events->FollowUpNotes(Ctx);
}

bool UGalaxySimulationSubsystem::GraceKeepsLastSystem(FCosmicEvalContext& Ctx, int32 SystemIndex)
{
	if (!CosmicHazards::IsLastSettledSystem(State, StateIndex, SystemIndex))
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

void UGalaxySimulationSubsystem::AdvanceMegaprojects()
{
	static const FGameplayTag CivCategory = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Civ")));
	UCosmicEventSubsystem* Events = GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>();
	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Events && Log);

	// The rate of the peoples the player does not rule (section 4.3). A compare, so a value that is not a number reads as 1.
	float AIRate = Rules.AIAscensionSpeedMultiplier;
	if (!(AIRate > 0.f && AIRate <= TNumericLimits<float>::Max()))
	{
		UE_LOG(LogCosmic, Error, TEXT("DT_Rules.AIAscensionSpeedMultiplier %f is not above 0: read as 1"), AIRate);
		AIRate = 1.f;
	}

	// Every response slot first, then every construction slot, each pass in Systems order: an answer spends the stockpile
	// before a construction does (section 5.7). Payment is all or nothing, so an answer that could not pay waits for its
	// whole cost: a construction of the same people that would leave less than that in a resource it spends is held back
	// this tick, or a cheaper construction would take back, tick after tick, what the dearer answer is waiting for. A
	// construction that spends nothing the waiting answers need goes on.
	using CosmicMegaprojects::ECosmicProjectSlot;
	constexpr ECosmicProjectSlot Passes[] = { ECosmicProjectSlot::Response, ECosmicProjectSlot::Construction };
	TArray<TPair<FName, FResourcePool>, TInlineAllocator<8>> WaitingAnswers;
	for (const ECosmicProjectSlot Slot : Passes)
	{
		for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num(); ++SystemIndex)
		{
			const FStarSystemState& Held = State.Systems[SystemIndex];
			const FName HeldRow = Slot == ECosmicProjectSlot::Response ? Held.ResponseMegaproject : Held.ActiveMegaproject;
			if (HeldRow.IsNone())
			{
				continue;
			}
			const FName OwnerID = Held.OwnerCivID;
			const int32 OwnerIndex = StateIndex.FindCiv(State, OwnerID);
			const FMegaprojectDef* HeldDef = Tables.FindMegaproject(HeldRow);
			const FResourcePool Cost = HeldDef ? CosmicMegaprojects::CostPerTickOf(*HeldDef) : FResourcePool();
			// Whether the owner could pay the slot in full now: what Advance pays by (a missing row pays nothing).
			const bool bCanPay = State.Civs.IsValidIndex(OwnerIndex) && CosmicMegaprojects::CanPay(State.Civs[OwnerIndex].Stockpile, Cost);
			if (Slot == ECosmicProjectSlot::Construction && HeldDef && bCanPay)
			{
				const TPair<FName, FResourcePool>* Waiting = WaitingAnswers.FindByPredicate([OwnerID](const TPair<FName, FResourcePool>& Each) { return Each.Key == OwnerID; });
				if (Waiting && TakesFromWaitingAnswers(State.Civs[OwnerIndex].Stockpile, Cost, Waiting->Value))
				{
					continue;
				}
			}
			FCosmicEvalContext Ctx = MakeEvalContext();
			const FName RowName = CosmicMegaprojects::Advance(Ctx, SystemIndex, Slot, AIRate);
			// An answer that could not pay and still stands waits for its cost (one whose row or hazard is gone left its slot).
			if (Slot == ECosmicProjectSlot::Response && !bCanPay && HeldDef && !OwnerID.IsNone() && State.Systems[SystemIndex].ResponseMegaproject == HeldRow)
			{
				TPair<FName, FResourcePool>* Waiting = WaitingAnswers.FindByPredicate([OwnerID](const TPair<FName, FResourcePool>& Each) { return Each.Key == OwnerID; });
				if (!Waiting)
				{
					Waiting = &WaitingAnswers.Emplace_GetRef(OwnerID, FResourcePool());
				}
				Waiting->Value.Energy += Cost.Energy;
				Waiting->Value.Matter += Cost.Matter;
				Waiting->Value.Insight += Cost.Insight;
				Waiting->Value.Influence += Cost.Influence;
				Waiting->Value.Causality += Cost.Causality;
			}
			// A resolution's effects may hand a system to the vessel's people: known before the completion line names it.
			// No row of the game hands one over there (Data.Integrity keeps TransferSystem out of OnResolveEffects).
			Events->FollowUpKnowing(Ctx);
			const FMegaprojectDef* Row = RowName.IsNone() ? nullptr : Tables.FindMegaproject(RowName);
			if (Row && State.Civs.IsValidIndex(OwnerIndex))
			{
				const FCivilizationState& Owner = State.Civs[OwnerIndex];
				FCosmicLine Line;
				Line.Category = CivCategory;
				Line.Severity = Owner.bIsPlayerRuled ? 1 : 0;
				Line.FocusSystem = State.Systems[SystemIndex].SystemID;
				Line.FocusCiv = Owner.CivID;
				Line.SourceRow = RowName;
				FFormatNamedArguments Args;
				Args.Add(TEXT("Civ"), Owner.CivName);
				Args.Add(TEXT("System"), State.Systems[SystemIndex].DisplayName);
				Args.Add(TEXT("Project"), Row->DisplayName);
				Log->PushVariants(Line, Row->CompleteTemplates, Args);
			}
			// What the completion did (a resolution), told after it: a cause comes before what follows from it.
			Events->FollowUpNotes(Ctx);
		}
	}
}

void UGalaxySimulationSubsystem::TickCivAI()
{
	// (a) The peoples the player does not rule start what they would start, no draw; then a people that may settle draws
	// one number, before the targets are looked at, and settles the target it falls on (section 4.3, 5.7, 9.2).
	static const FGameplayTag ClaimTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Claim")));
	for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
	{
		if (State.Civs[CivIndex].bIsPlayerRuled)
		{
			continue;
		}
		// A profile that leaves claims alone does not start a claim's answer (16 #71); a construction is never one (Choices
		// builds no row that answers a hazard).
		const bool bLeavesClaims = Rules.ClaimLeaveProfiles.HasTagExact(State.Civs[CivIndex].AIProfile);
		for (const TPair<int32, FName>& Choice : CosmicMegaprojects::Choices(State, StateIndex, Tables, CivIndex, 0))
		{
			if (bLeavesClaims && State.Systems[Choice.Key].ActiveHazard == ClaimTag && Tables.IsHazardAnswer(Choice.Value))
			{
				continue;
			}
			CosmicMegaprojects::Start(State, StateIndex, Tables, Choice.Key, Choice.Value);
		}
		if (CosmicMegaprojects::MaySettle(State, StateIndex, Rules, CivIndex))
		{
			const float Roll = Rng.GetFraction();
			const TArray<int32> Targets = CosmicMegaprojects::SettleTargets(State, StateIndex, Tables, Rules, CivIndex);
			const int32 Pick = PickIndex(Roll, Targets.Num());
			if (Pick != INDEX_NONE)
			{
				FCosmicEvalContext Ctx = MakeEvalContext();
				CosmicMegaprojects::Settle(Ctx, CivIndex, Targets[Pick]);
			}
		}
		// The AI holds no edict in B2 (section 2.6): it revokes what it holds - an edict with a Stability cost once below
		// DT_Rules.AIEdictRevokeStability, any other at once. No draw, no line. A regent revokes what the ruler left.
		for (const FName Row : CosmicEdicts::AIRevokesOf(State.Civs[CivIndex], Tables, Rules))
		{
			CosmicEdicts::Revoke(State.Civs[CivIndex], Row);
		}
		// Its diplomacy (B3 section 2.8): no draw, no profile.
		TickAIDiplomacy(CivIndex);
	}

	// (b) The feat judgment, once per tick.
	UAscensionSubsystem* Ascension = GetGameInstance()->GetSubsystem<UAscensionSubsystem>();
	check(Ascension);
	Ascension->EvaluateFeats(State, Rng);
}

void UGalaxySimulationSubsystem::TickAIDiplomacy(int32 CivIndex)
{
	const FGameplayTag Friendly = CosmicTags::StanceFriendly();
	const FGameplayTag Hostile = CosmicTags::StanceHostile();
	FCivilizationState& Civ = State.Civs[CivIndex];

	// (1) No friendly stance and no guard: those ledgers are the ruler's (a regent lets go of what the ruler left).
	// (2) A hostility whose people the fleet does not besiege ends.
	if (!Civ.Ties.IsEmpty())
	{
		const int32 Target = Civ.FleetTargetSystemID.IsNone() ? INDEX_NONE : StateIndex.FindSystem(State, Civ.FleetTargetSystemID);
		const FName Besieged = State.Systems.IsValidIndex(Target) ? State.Systems[Target].OwnerCivID : FName();
		TArray<FName, TInlineAllocator<4>> Ends;
		for (const TPair<FName, FCosmicTie>& Pair : Civ.Ties)
		{
			if (Pair.Value.Stance == Friendly || (Pair.Value.Stance == Hostile && (Pair.Key != Besieged || Besieged == Civ.CivID)))
			{
				Ends.Add(Pair.Key);
			}
		}
		for (const FName Other : Ends)
		{
			CosmicDiplomacy::SetStanceOf(Civ, Other, FGameplayTag());
		}
	}
	if (!Civ.FleetTargetSystemID.IsNone())
	{
		const int32 Target = StateIndex.FindSystem(State, Civ.FleetTargetSystemID);
		if (State.Systems.IsValidIndex(Target) && State.Systems[Target].OwnerCivID == Civ.CivID)
		{
			Civ.FleetTargetSystemID = NAME_None;
			Civ.FleetTargetSinceTick = State.Tick;
		}
	}

	// (3) A hostility opens only with a siege: no hostility held, no target, the rest over, a fleet, and not pacifist.
	if (!Civ.FleetTargetSystemID.IsNone() || State.Tick < Civ.FleetTargetSinceTick + static_cast<int64>(FMath::Max(0, Rules.SiegeRestTicks))
		|| !(Civ.FleetPower > 0.f) || !(Civ.Ethics.Pacifism < Rules.AIHostilePacifismMax))
	{
		return;
	}
	for (const TPair<FName, FCosmicTie>& Pair : Civ.Ties)
	{
		if (Pair.Value.Stance == Hostile)
		{
			return;
		}
	}
	// A people at its realm cap besieges nothing (SiegeBlockOf's RealmCap holds for every system): no scan.
	if (CosmicConflict::OwnedCountOf(State, StateIndex, CivIndex) >= CosmicMegaprojects::RealmCapOf(Rules, Civ.Tier))
	{
		return;
	}
	// Its borders once for the scan (Borders and ViewOf ask them a pair at a time): a bordering people's view is its
	// stored relation (CosmicDiplomacy::ViewOf).
	TArray<bool> Borders;
	CosmicDiplomacy::BordersOf(State, StateIndex, CivIndex, Borders);
	// The people the ruled people's fleet besieges, if any.
	FName RuledBesieges;
	const int32 Ruled = State.Civs.IndexOfByPredicate([](const FCivilizationState& Candidate) { return Candidate.bIsPlayerRuled; });
	if (CosmicConflict::IsBesieging(State, StateIndex, Ruled))
	{
		RuledBesieges = State.Systems[StateIndex.FindSystem(State, State.Civs[Ruled].FleetTargetSystemID)].OwnerCivID;
	}
	for (int32 Other = 0; Other < State.Civs.Num(); ++Other)
	{
		if (Other == CivIndex)
		{
			continue;
		}
		const FCivilizationState& Them = State.Civs[Other];
		if (!(Civ.FleetPower >= (Them.bIsPlayerRuled ? Rules.AIHostilePowerRatioTowardRuled : Rules.AIHostilePowerRatio) * Them.FleetPower) || !Borders[Other]
			|| !(Civ.Relations.FindRef(Them.CivID) <= (Them.bIsPlayerRuled ? Rules.AIHostileRelationTowardRuled : Rules.AIHostileRelation))
			|| CosmicDiplomacy::HasTreaty(Civ, Them.CivID, CosmicTags::TreatyNonAggression()))
		{
			continue;
		}
		// No pile-on: another AI already hostile toward them, or the ruled people's siege on them, closes the gate.
		bool bTaken = false;
		for (int32 Third = 0; Third < State.Civs.Num() && !bTaken; ++Third)
		{
			bTaken = Third != CivIndex && Third != Other && !State.Civs[Third].bIsPlayerRuled
				&& CosmicDiplomacy::StanceOf(State.Civs[Third], Them.CivID) == Hostile;
		}
		if (bTaken || RuledBesieges == Them.CivID)
		{
			continue;
		}
		// A system of theirs to aim at, in Systems order, out of every higher sphere.
		int32 Aim = INDEX_NONE;
		for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num() && Aim == INDEX_NONE; ++SystemIndex)
		{
			if (State.Systems[SystemIndex].OwnerCivID == Them.CivID
				&& CosmicConflict::SiegeBlockOf(State, StateIndex, Rules, CivIndex, SystemIndex, true) == CosmicConflict::ECosmicSiegeBlock::None
				&& !(ComputeAttritionPressureIn(State, StateIndex, Tables, Rules, CivIndex, SystemIndex).P > 0.f))
			{
				Aim = SystemIndex;
			}
		}
		if (Aim == INDEX_NONE)
		{
			continue;
		}
		const bool bBrokeTrade = CosmicDiplomacy::TurnHostile(State, CivIndex, Other, 0.f);
		State.Civs[CivIndex].FleetTargetSystemID = State.Systems[Aim].SystemID;
		State.Civs[CivIndex].FleetTargetSinceTick = State.Tick;
		if (Them.bIsPlayerRuled)
		{
			UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
			check(Log);
			const FName AimID = State.Systems[Aim].SystemID;
			NoteMet(State.Civs[CivIndex].CivID, AimID);
			FFormatNamedArguments Args;
			Args.Add(TEXT("Civ"), Them.CivName);
			Args.Add(TEXT("OtherCiv"), State.Civs[CivIndex].CivName);
			if (bBrokeTrade)
			{
				FFormatNamedArguments TreatyArgs = Args;
				TreatyArgs.Add(TEXT("Treaty"), CosmicDiplomacy::TreatyNameOf(CosmicTags::TreatyTrade()));
				Log->PushRow(CosmicRows::LogTreatyBroken, 1, TreatyArgs, NAME_None, Them.CivID);
			}
			Args.Add(TEXT("System"), State.Systems[Aim].DisplayName);
			Log->PushRow(CosmicRows::LogSiegeOnUs, 2, Args, AimID, Them.CivID);
		}
		return;
	}
}

void UGalaxySimulationSubsystem::TickCrossings()
{
	// (o) Cleanup (section 7.4): a civilization that owns no system leaves Civs, in array order, unless the player rules
	// it this tick (the role-effect stage lets go of it first; the next tick's cleanup takes it). No line: the expiry that
	// took its last system has told it. Arrivals and departures are Phase 6a.
	// A people with no settled system lets its settlements go first (section 5.7: a destruction has done so already in
	// play), so that none is left holding only settlements it can never pay for. This net records no collapse: a people
	// the player rules that it empties would be kept with nothing to rule, so it says so (the stage that takes the last
	// settled system of that people records the collapse itself, as the hazard expiry does).
	static const FGameplayTag DeposeTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Effect.DeposeRuler")));
	for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
	{
		if (CosmicHazards::LetSettlementsGo(State, StateIndex, CivIndex) && State.Civs[CivIndex].bIsPlayerRuled && State.Calling.PendingEffect != DeposeTag)
		{
			UE_LOG(LogCosmic, Error, TEXT("TickCrossings: %s, the people the player rules, held only settlements: they go to no one, and no collapse was recorded to depose its ruler."), *State.Civs[CivIndex].CivID.ToString());
		}
	}
	TArray<FName> Removed;
	for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); )
	{
		const FCivilizationState& Civ = State.Civs[CivIndex];
		// The civilization half of the index is stale once one is removed; OwnsAnySystem reads only the system half.
		if (CosmicState::OwnsAnySystem(State, StateIndex, CivIndex) || Civ.bIsPlayerRuled)
		{
			++CivIndex;
			continue;
		}
		Removed.Add(Civ.CivID);
		State.Civs.RemoveAt(CivIndex);
	}
	if (Removed.IsEmpty())
	{
		return;
	}
	for (FCivilizationState& Civ : State.Civs)
	{
		for (const FName CivID : Removed)
		{
			Civ.Relations.Remove(CivID);
		}
	}
	// The pair records go with the people (B3 section 2.1), at the same place: no tie to a people that is gone.
	for (const FName CivID : Removed)
	{
		CosmicDiplomacy::ForgetCiv(State, CivID);
	}
	if (Removed.Contains(State.Knowing.PendingContactCivID))
	{
		State.Knowing.PendingContactCivID = NAME_None;
	}
	StateIndex.Rebuild(State);
}

bool UGalaxySimulationSubsystem::TickPlayerTravel()
{
	// One lane (B1 header section 2.4): on the tick the countdown reaches 0 the vessel moves when the target is still a
	// living lane neighbour of where it stands (a system destroyed on the way leaves the lanes); otherwise the journey just
	// ends where it began. Either way both fields empty together. No line, no draw, no knowing read.
	FPlayerEntityState& Player = State.Player;
	bool bArrived = false;
	if (Player.TravelTicksRemaining > 0 && --Player.TravelTicksRemaining == 0)
	{
		const int32 Here = StateIndex.FindSystem(State, Player.CurrentSystemID);
		const int32 There = StateIndex.FindSystem(State, Player.TravelTargetSystemID);
		if (State.Systems.IsValidIndex(Here) && State.Systems.IsValidIndex(There) && State.Systems[Here].Lanes.Contains(Player.TravelTargetSystemID)
			&& State.Systems[There].GalaxyRow == State.Systems[Here].GalaxyRow && !CosmicHazards::IsDeadNode(State.Systems[There])
			&& !CosmicState::IsSanctuary(Player.TravelTargetSystemID))
		{
			Player.CurrentSystemID = Player.TravelTargetSystemID;
			bArrived = true;
		}
		Player.TravelTargetSystemID = NAME_None;
	}
	else if (Player.TravelTicksRemaining <= 0 && !Player.TravelTargetSystemID.IsNone())
	{
		// A target without a countdown is no journey (no command writes one): it empties.
		Player.TravelTargetSystemID = NAME_None;
		Player.TravelTicksRemaining = 0;
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
	return bArrived;
}

void UGalaxySimulationSubsystem::TickSight(bool bVesselMoved)
{
	// Sight range, meetings and first contact: reads the world, writes State.Knowing and the telling (section 7.4, 8.1).
	// The knobs are knowing's own: below 1 reads as 1, without an error line (the schema keeps their ranges).
	const int32 Interval = Rules.SightIntervalTicks > 1 ? Rules.SightIntervalTicks : 1;
	if (!bVesselMoved && State.Tick % Interval != 0)
	{
		return;
	}

	// The places of the eye (section 4.4): a ruler at every system its people owns, a wanderer where it stands.
	const FPlayerEntityState& Player = State.Player;
	const bool bRuler = Player.Role == EPlayerRole::Ruler;
	EAscensionTier EyeTier = State.Calling.PersonalTier;
	TArray<int32, TInlineAllocator<8>> Eye;
	if (bRuler)
	{
		const int32 RuledIndex = StateIndex.FindCiv(State, Player.RuledCivID);
		if (State.Civs.IsValidIndex(RuledIndex))
		{
			const FCivilizationState& Ruled = State.Civs[RuledIndex];
			EyeTier = Ruled.Tier;
			for (const FName SystemID : Ruled.OwnedSystems)
			{
				const int32 SystemIndex = StateIndex.FindSystem(State, SystemID);
				if (State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == Ruled.CivID && !Eye.Contains(SystemIndex))
				{
					Eye.Add(SystemIndex);
				}
			}
		}
	}
	else
	{
		const int32 Standing = StateIndex.FindSystem(State, Player.CurrentSystemID);
		if (State.Systems.IsValidIndex(Standing))
		{
			Eye.Add(Standing);
		}
	}
	const int32 TierIndex = static_cast<int32>(EyeTier);
	const int32 Reach = (Rules.SightLanesByTier.IsValidIndex(TierIndex) && Rules.SightLanesByTier[TierIndex] > 1) ? Rules.SightLanesByTier[TierIndex] : 1;
	// A ruler's eye opens at FirstSightTick; until then its places only order the meetings.
	const bool bSees = !bRuler || State.Tick >= Rules.FirstSightTick;
	LookFrom(Eye, Reach, bSees);
}

int32 UGalaxySimulationSubsystem::LookFrom(TConstArrayView<int32> EyePlaces, int32 Reach, bool bSees)
{
	// One breadth-first walk from every place of the eye (CosmicState::SightWalkOf): the lanes from the eye without the
	// reach bound (the order of the meetings), INDEX_NONE where the walk does not reach. Integers only.
	const CosmicState::FCosmicSightWalk Walk = CosmicState::SightWalkOf(State, StateIndex, EyePlaces);
	const TArray<int32>& LanesFromEye = Walk.LanesFrom;

	// What the eye reaches becomes known through NoteKnown, in the order of the walk. A set made for this pass only keeps
	// NoteKnown from scanning the array for what is known already (the order is hashed, the set is only asked).
	FKnowingState& Knowing = State.Knowing;
	int32 Learned = 0;
	if (bSees)
	{
		TSet<FName> Known;
		Known.Reserve(Knowing.KnownSystems.Num());
		for (const FKnownMark& Mark : Knowing.KnownSystems)
		{
			Known.Add(Mark.ID);
		}
		for (const int32 SystemIndex : Walk.Found)
		{
			if (LanesFromEye[SystemIndex] > Reach)
			{
				break;
			}
			const FName SystemID = State.Systems[SystemIndex].SystemID;
			if (!Known.Contains(SystemID))
			{
				Known.Add(SystemID);
				Learned += NoteKnown(SystemID) ? 1 : 0;
			}
		}
	}

	// The owners of the known systems are met, nearer peoples first (section 7.4): recognition, not a visit. A people's
	// distance is that of its nearest known system; ties go by CivID.
	struct FSighting
	{
		int32 CivIndex = INDEX_NONE;
		int32 Lanes = MAX_int32;
		int32 SystemIndex = INDEX_NONE;
	};
	TArray<FSighting, TInlineAllocator<8>> Sightings;
	for (const FKnownMark& Mark : Knowing.KnownSystems)
	{
		const int32 SystemIndex = StateIndex.FindSystem(State, Mark.ID);
		const FName Owner = State.Systems.IsValidIndex(SystemIndex) ? State.Systems[SystemIndex].OwnerCivID : FName();
		const int32 CivIndex = StateIndex.FindCiv(State, Owner);
		if (!State.Civs.IsValidIndex(CivIndex) || Knowing.MetCivs.ContainsByPredicate([Owner](const FKnownMark& Met) { return Met.ID == Owner; }))
		{
			continue;
		}
		const int32 Lanes = LanesFromEye[SystemIndex] == INDEX_NONE ? MAX_int32 : LanesFromEye[SystemIndex];
		FSighting* Seen = Sightings.FindByPredicate([CivIndex](const FSighting& Each) { return Each.CivIndex == CivIndex; });
		if (!Seen)
		{
			Seen = &Sightings.AddDefaulted_GetRef();
			Seen->CivIndex = CivIndex;
		}
		if (Lanes < Seen->Lanes || Seen->SystemIndex == INDEX_NONE)
		{
			Seen->Lanes = Lanes;
			Seen->SystemIndex = SystemIndex;
		}
	}
	Sightings.Sort([this](const FSighting& A, const FSighting& B)
	{
		if (A.Lanes != B.Lanes)
		{
			return A.Lanes < B.Lanes;
		}
		return FNameLexicalLess()(State.Civs[A.CivIndex].CivID, State.Civs[B.CivIndex].CivID);
	});
	for (const FSighting& Seen : Sightings)
	{
		NoteMet(State.Civs[Seen.CivIndex].CivID, State.Systems[Seen.SystemIndex].SystemID);
	}
	return Learned;
}

void UGalaxySimulationSubsystem::ResolvePendingRoleEffect()
{
	// Death and deposition recorded in Calling.PendingEffect start their pipeline here, at the tick boundary (section 4.4,
	// 5.10). The death pipeline arrives with Phase 4b: until then a KillPlayer stays recorded.
	static const FGameplayTag DeposeTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Effect.DeposeRuler")));
	static const FGameplayTag AbdicatedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Abdicated")));
	FCallingState& Calling = State.Calling;
	if (Calling.PendingEffect == DeposeTag)
	{
		UPlayerRoleSubsystem* PlayerRole = GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>();
		check(PlayerRole);
		// The cause says which pipeline: the vessel left of its own will (the command, the throne card's last choice - B1),
		// or it was deposed.
		if (Calling.PendingCause == AbdicatedTag)
		{
			PlayerRole->AbdicateRuler();
		}
		else
		{
			PlayerRole->DeposeRuler();
		}
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
