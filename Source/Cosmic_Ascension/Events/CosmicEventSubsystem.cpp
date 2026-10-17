// Cosmic Ascension - Events layer.

#include "Events/CosmicEventSubsystem.h"

#include "Core/CosmicSeed.h"
#include "Core/CosmicTypes.h"
#include "Cosmic_Ascension.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/GameInstance.h"
#include "Internationalization/Text.h"
#include "Log/CosmicLogSubsystem.h"
#include "Math/NumericLimits.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Templates/UnrealTemplate.h"

// Helpers of this file. A named namespace: the module is built as one unity file, so an anonymous one would share its
// names with every other .cpp.
namespace CosmicEventImpl
{
	// How deep outbreak rows nest through FollowUpNotes (an outbreak row that resolves and places again what fired it would
	// never end). Transient: it is back to 0 whenever no FollowUpNotes runs, so it is no state of the run.
	constexpr int32 MaxNestedOutbreakRows = 4;
	int32 NestedOutbreakRows = 0;

	const FName KeyFormer(TEXT("Former"));
	const FName KeyRuler(TEXT("Ruler"));
	const FName KeyWanderer(TEXT("Wanderer"));
	const FName KeyBody(TEXT("Body"));
	const FName KeyMind(TEXT("Mind"));
	const FName KeyCausality(TEXT("Causality"));
	const FName KeyExaltation(TEXT("Exaltation"));
	const FName KeyPacifism(TEXT("Pacifism"));
	const FName KeySpiritualism(TEXT("Spiritualism"));
	const FName KeyOpenness(TEXT("Openness"));
	const FName ResourceKeys[] = { FName(TEXT("Energy")), FName(TEXT("Matter")), FName(TEXT("Insight")), FName(TEXT("Influence")), FName(TEXT("Causality")) };

	// Personal stats run from 0 to 10 (section 5.9).
	constexpr int32 MaxPersonalStat = 10;

	bool IsEmbodied(const FGalaxyState& State)
	{
		return State.Calling.LifeState == ELifeState::Embodied;
	}

	// The effects a per-tick source may apply (section 7.5): the ones that change an amount. Every other effect is turned
	// away there, so the economy stage never draws (NameNotable), never records a role effect (KillPlayer, DeposeRuler),
	// never touches the vessel (PlayerStatDelta, GrantPower), never sets a mark that a row sets once (AddTrait,
	// RemoveTrait, SetFlag) and never reaches into the hazard stage. A closed list on purpose: an effect type appended to
	// the enum is refused by a per-tick source until it is added here.
	bool IsPerTickEffect(EEffectType Type)
	{
		switch (Type)
		{
		case EEffectType::AddResource:
		case EEffectType::MultiplyIncome:
		case EEffectType::ShiftEthics:
		case EEffectType::ChangeStability:
		case EEffectType::ChangeRelation:
		case EEffectType::ChangeReverence:
		case EEffectType::ChangePopulation:
			return true;
		default:
			return false;
		}
	}

	// The Value of an effect that adds to an amount or scales one has to be a finite number. Its range is a lint
	// (Data.Integrity); the guard is here, as comparisons that are false for a value that is not a number: such an effect
	// is turned away, so nothing that is not a number is clamped to a bound or written into the state.
	bool IsAmount(float Value)
	{
		return Value >= TNumericLimits<float>::Lowest() && Value <= TNumericLimits<float>::Max();
	}

	// Tier number 1..5, the unit of every Tier value in definition data (section 7.3).
	float TierNumber(EAscensionTier Tier)
	{
		return static_cast<float>(static_cast<int32>(Tier) + 1);
	}

	bool ResourceOf(FName Key, ECosmicResource& OutResource)
	{
		for (int32 Index = 0; Index < static_cast<int32>(UE_ARRAY_COUNT(ResourceKeys)); ++Index)
		{
			if (ResourceKeys[Index] == Key)
			{
				OutResource = static_cast<ECosmicResource>(Index);
				return true;
			}
		}
		return false;
	}

	const float* EthicsAxisOf(const FEthicsAxes& Ethics, FName Key)
	{
		if (Key == KeyPacifism)
		{
			return &Ethics.Pacifism;
		}
		if (Key == KeySpiritualism)
		{
			return &Ethics.Spiritualism;
		}
		if (Key == KeyOpenness)
		{
			return &Ethics.Openness;
		}
		return nullptr;
	}

	const int32* PlayerStatOf(const FPlayerEntityState& Player, FName Key)
	{
		if (Key == KeyBody)
		{
			return &Player.Body;
		}
		if (Key == KeyMind)
		{
			return &Player.Mind;
		}
		if (Key == KeyCausality)
		{
			return &Player.Causality;
		}
		if (Key == KeyExaltation)
		{
			return &Player.Exaltation;
		}
		return nullptr;
	}

	// The civilization a civ selector names: an empty Key is the context civilization, Former the civilization the
	// vessel left (only while there is a vessel). Any other word is not resolved (section 16 #29).
	int32 SelectCiv(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicEvalSubject& Subject, FName Key)
	{
		if (Key.IsNone())
		{
			return State.Civs.IsValidIndex(Subject.CivIndex) ? Subject.CivIndex : INDEX_NONE;
		}
		if (Key == KeyFormer && IsEmbodied(State))
		{
			return Index.FindCiv(State, State.Player.FormerCivID);
		}
		return INDEX_NONE;
	}

	// The other party a condition or an effect names: its Key, or the OtherID of the subject when the Key is empty.
	FName OtherOf(const FCosmicEvalSubject& Subject, FName Key)
	{
		return Key.IsNone() ? Subject.OtherID : Key;
	}

	const FTranscendentState* FindTranscendent(const FGalaxyState& State, FName TranscendentID)
	{
		if (TranscendentID.IsNone())
		{
			return nullptr;
		}
		return State.Transcendents.FindByPredicate([TranscendentID](const FTranscendentState& Each) { return Each.TranscendentID == TranscendentID; });
	}

	// The other party of a relation (section 5.5): OtherOf, resolved. A key of Relations is a living people, a
	// transcendent of the run or a celestial row; any other name (a selector word nothing resolves, a people that is
	// gone, a misspelt row, a fallen row the roster did not draw) is NAME_None, so that it is neither read as a neutral
	// relation nor written into the save. A celestial transcendent never has a state instance (section 4.2) and is a
	// party all the same, by the name of its DT_Transcendents row. A fallen or independent row is a party only as an
	// instance of the run.
	FName RelationPartyOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicEvalSubject& Subject, FName Key)
	{
		const FName Other = OtherOf(Subject, Key);
		if (Other.IsNone())
		{
			return NAME_None;
		}
		if (FindTranscendent(State, Other) || Index.FindCiv(State, Other) != INDEX_NONE)
		{
			return Other;
		}
		static const FGameplayTag CelestialTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Realm.Celestial")));
		const FTranscendentDef* Row = Tables.FindTranscendent(Other);
		return (Row && Row->Allegiance == CelestialTag) ? Other : FName();
	}

	// Weight factor of a Lottery row of Category on a unit (section 4.2): the product, over the unsealed transcendents in
	// array order whose sphere holds the unit's system or lies one lane from it, of min(3, modifier x (1 + step x size
	// of the sphere)), never below 0. 1 when no transcendent names that Category or the unit has no system. The
	// unit's system is the system of the pair: the capital for a Civilization row, the system itself for a
	// SettledSystem row.
	float SphereWeightFactor(const FGalaxyState& State, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, FGameplayTag Category, int32 SystemIndex)
	{
		float Factor = 1.f;
		if (!Category.IsValid() || !State.Systems.IsValidIndex(SystemIndex))
		{
			return Factor;
		}
		for (const FTranscendentState& Transcendent : State.Transcendents)
		{
			const FTranscendentDef* Def = Transcendent.bSealed ? nullptr : Tables.FindTranscendent(Transcendent.TranscendentID);
			const float* Modifier = Def ? Def->EventWeightModifiers.Find(Category) : nullptr;
			if (!Modifier || UGalaxySimulationSubsystem::SphereReachIn(State, Transcendent, SystemIndex) == ECosmicSphereReach::None)
			{
				continue;
			}
			// The ranges of the modifier and of the step are a lint; the guard is here: a factor is never below 0 and never
			// above 3. A comparison and not FMath::Clamp, which hands back the upper bound for a value that is not a number.
			const float Raw = *Modifier * (1.f + Rules.SphereEventWeightStep * static_cast<float>(Transcendent.InfluencedSystems.Num()));
			Factor *= Raw > 0.f ? FMath::Min(Raw, 3.f) : 0.f;
		}
		return Factor;
	}

	// A count of ticks written as a Key: digits only, nine at most. Anything else is 0, which means "no count": the
	// caller tests the count for being above zero before it compares (every counter is at least 0), so a malformed Key
	// makes its condition false and is never read by its numeric prefix.
	int32 TickCountOf(FName Key)
	{
		if (Key.IsNone())
		{
			return 0;
		}
		const FString Text = Key.ToString();
		if (Text.Len() > 9)
		{
			return 0;
		}
		int32 Count = 0;
		for (const TCHAR Char : Text)
		{
			if (Char < TEXT('0') || Char > TEXT('9'))
			{
				return 0;
			}
			Count = Count * 10 + static_cast<int32>(Char - TEXT('0'));
		}
		return Count;
	}

	bool IsSettledSystemOf(const FGalaxyState& State, int32 SystemIndex, FName CivID)
	{
		return State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == CivID && CosmicState::IsSettled(State.Systems[SystemIndex]);
	}

	// Writes a role effect into the calling when the slot is free. First recorded wins (section 7.4).
	bool RecordRoleEffect(FCosmicEvalContext& Ctx, const TCHAR* EffectTagName, FGameplayTag Cause)
	{
		FCallingState& Calling = Ctx.State.Calling;
		if (Calling.PendingEffect.IsValid())
		{
			return false;
		}
		Calling.PendingEffect = FGameplayTag::RequestGameplayTag(FName(EffectTagName));
		Calling.PendingCause = Cause;
		Calling.PendingSourceRow = Ctx.SourceRow;
		return true;
	}

	// NameNotable (section 7.5). The caller has already made the one draw of the effect: Roll is in [0, 1).
	bool AddNotable(const FCosmicEffect& Effect, FCosmicEvalContext& Ctx, float Roll)
	{
		static const FGameplayTag NoSelfTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Trait.Mind.NoSelf")));
		static const FGameplayTag RulerTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Notable.Ruler")));
		static const FGameplayTag RegentTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Notable.Regent")));

		FGalaxyState& State = Ctx.State;
		const int32 CivIndex = SelectCiv(State, Ctx.Index, Ctx, Effect.Key);
		if (CivIndex == INDEX_NONE || !Effect.Tag.IsValid())
		{
			return false;
		}
		FCivilizationState& Civ = State.Civs[CivIndex];

		// Guard 1: a people of the fourth rung or above has no individuals left (section 3.2).
		if (TierNumber(Civ.Tier) > 3.f)
		{
			return false;
		}
		// Guard 2: a nameless people.
		const FCivArchetypeDef* Archetype = Ctx.Tables.FindCiv(Civ.ArchetypeRow.IsNone() ? CosmicRows::ArchDefault : Civ.ArchetypeRow);
		const int32 BankSize = Archetype ? Archetype->PersonNameBank.Num() : 0;
		if (BankSize == 0 || Civ.Traits.HasTag(NoSelfTag))
		{
			return false;
		}

		// Guard 3: at most one open record per role. The seat changed hands. 0 is the mark of an open record, so a record
		// closed before the first tick is closed at 1 (section 16 #35).
		for (FNotableRecord& Record : Civ.Notables)
		{
			if (Record.LastTick == 0 && Record.Role == Effect.Tag)
			{
				Record.LastTick = FMath::Max<int64>(State.Tick, 1);
			}
		}

		// Guard 4: the roster is short. A closed record leaves first (the one closed longest ago, then the oldest, then
		// array order); with none closed an open one that is neither ruler nor regent (the oldest); otherwise nobody
		// is added: the line is still told.
		const int32 Cap = FMath::Max(1, Ctx.Rules.MaxNotablesPerCiv);
		while (Civ.Notables.Num() >= Cap)
		{
			int32 Evict = INDEX_NONE;
			for (int32 Index = 0; Index < Civ.Notables.Num(); ++Index)
			{
				const FNotableRecord& Record = Civ.Notables[Index];
				if (Record.LastTick == 0)
				{
					continue;
				}
				if (Evict == INDEX_NONE || Record.LastTick < Civ.Notables[Evict].LastTick
					|| (Record.LastTick == Civ.Notables[Evict].LastTick && Record.FirstTick < Civ.Notables[Evict].FirstTick))
				{
					Evict = Index;
				}
			}
			if (Evict == INDEX_NONE)
			{
				for (int32 Index = 0; Index < Civ.Notables.Num(); ++Index)
				{
					const FNotableRecord& Record = Civ.Notables[Index];
					if (Record.Role == RulerTag || Record.Role == RegentTag)
					{
						continue;
					}
					if (Evict == INDEX_NONE || Record.FirstTick < Civ.Notables[Evict].FirstTick)
					{
						Evict = Index;
					}
				}
			}
			if (Evict == INDEX_NONE)
			{
				return false;
			}
			Civ.Notables.RemoveAt(Evict);
		}

		// The name: one of the bank indices no record of this people carries, picked by the number drawn before the
		// guards. When every name is taken the whole bank is open again.
		TArray<int32, TInlineAllocator<32>> FreeNames;
		for (int32 NameIndex = 0; NameIndex < BankSize; ++NameIndex)
		{
			const bool bTaken = Civ.Notables.ContainsByPredicate([NameIndex, BankSize](const FNotableRecord& Record) { return Record.NameIndex % BankSize == NameIndex; });
			if (!bTaken)
			{
				FreeNames.Add(NameIndex);
			}
		}
		if (FreeNames.IsEmpty())
		{
			for (int32 NameIndex = 0; NameIndex < BankSize; ++NameIndex)
			{
				FreeNames.Add(NameIndex);
			}
		}
		const int32 Pick = FMath::Min(static_cast<int32>(Roll * static_cast<float>(FreeNames.Num())), FreeNames.Num() - 1);

		FNotableRecord& Record = Civ.Notables.AddDefaulted_GetRef();
		Record.Role = Effect.Tag;
		Record.NameIndex = FreeNames[Pick];
		Record.FirstTick = State.Tick;
		Record.SourceRow = Ctx.SourceRow;
		return true;
	}

	// The slot units of one civilization: its settled systems in OwnedSystems order, and its capital when that is one
	// of them (INDEX_NONE otherwise).
	struct FSlotUnits
	{
		TArray<int32, TInlineAllocator<16>> SettledSystems;
		int32 Capital = INDEX_NONE;
	};

	FSlotUnits UnitsOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCivilizationState& Civ)
	{
		FSlotUnits Units;
		for (const FName SystemID : Civ.OwnedSystems)
		{
			const int32 SystemIndex = Index.FindSystem(State, SystemID);
			if (IsSettledSystemOf(State, SystemIndex, Civ.CivID))
			{
				Units.SettledSystems.Add(SystemIndex);
				if (SystemID == Civ.HomeSystemID)
				{
					Units.Capital = SystemIndex;
				}
			}
		}
		return Units;
	}

	// A row on one unit, as the slot gathered it.
	struct FSlotPair
	{
		FName RowName;
		const FCosmicEventDef* Row = nullptr;
		// INDEX_NONE for a Civilization row whose capital is not settled.
		int32 SystemIndex = INDEX_NONE;
	};
}

void UCosmicEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogCosmic, Log, TEXT("UCosmicEventSubsystem initialized"));
}

void UCosmicEventSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool UCosmicEventSubsystem::Check(const FCosmicCondition& Condition, const FCosmicEvalView& View)
{
	using namespace CosmicEventImpl;

	const FGalaxyState& State = View.State;
	const FCivilizationState* Civ = State.Civs.IsValidIndex(View.CivIndex) ? &State.Civs[View.CivIndex] : nullptr;
	const FStarSystemState* System = State.Systems.IsValidIndex(View.SystemIndex) ? &State.Systems[View.SystemIndex] : nullptr;
	const FPlayerEntityState& Player = State.Player;

	switch (Condition.Type)
	{
	case EConditionType::CivTierAtLeast:
		return Civ && TierNumber(Civ->Tier) >= Condition.Value;

	case EConditionType::CivHasTrait:
		return Civ && Condition.Tag.IsValid() && Civ->Traits.HasTag(Condition.Tag);

	case EConditionType::CivResourceAtLeast:
	{
		ECosmicResource Resource = ECosmicResource::Energy;
		return Civ && ResourceOf(Condition.Key, Resource) && Civ->Stockpile.Get(Resource) >= Condition.Value;
	}

	case EConditionType::CivStabilityBelow:
		return Civ && Civ->Stability < Condition.Value;

	case EConditionType::CivEthicsAxisAbove:
	{
		const float* Axis = Civ ? EthicsAxisOf(Civ->Ethics, Condition.Key) : nullptr;
		return Axis && *Axis > Condition.Value;
	}

	case EConditionType::CivStabilityBelowFor:
	{
		// Key = the number of ticks. The counter itself is fixed at the low-stability threshold (section 4.1, 16 #25).
		const int32 Ticks = TickCountOf(Condition.Key);
		return Civ && Ticks > 0 && Civ->LowStabilityTicks >= Ticks;
	}

	case EConditionType::CivRelationAtLeast:
	{
		// A missing entry is 0, neutral (section 5.5). A party that does not resolve has no relation to read.
		const FName Other = RelationPartyOf(State, View.Index, View.Tables, View, Condition.Key);
		return Civ && !Other.IsNone() && Civ->Relations.FindRef(Other) >= Condition.Value;
	}

	case EConditionType::CivReverenceAtLeast:
		return Civ && Civ->ReverenceToPlayer >= Condition.Value;

	case EConditionType::SystemHasHazard:
		return System && System->ActiveHazard.IsValid() && (!Condition.Tag.IsValid() || System->ActiveHazard.MatchesTag(Condition.Tag));

	case EConditionType::SystemOwnedBy:
		// An empty Key = the context civilization; otherwise a CivID. The selector words wait for section 16 #29.
		if (!System || System->OwnerCivID.IsNone())
		{
			return false;
		}
		return Condition.Key.IsNone() ? (Civ && System->OwnerCivID == Civ->CivID) : System->OwnerCivID == Condition.Key;

	case EConditionType::MegaprojectComplete:
		// The record of the Key's row on the subject system, since the life the judgment counts from (section 4.4, 5.7).
		return System && CosmicMegaprojects::HasCompleted(*System, Condition.Key, View.SinceTick);

	case EConditionType::SystemPressureAtLeast:
		// P of the subject system, derived over the state of the view (section 4.2).
		return System && UGalaxySimulationSubsystem::ComputePressureIn(State, View.Index, View.Tables, View.Rules, View.SystemIndex).P >= Condition.Value;

	case EConditionType::SystemInfluencedBy:
	{
		const FTranscendentState* Transcendent = FindTranscendent(State, OtherOf(View, Condition.Key));
		return System && Transcendent && Transcendent->InfluencedSystems.Contains(System->SystemID);
	}

	case EConditionType::PlayerTierAtLeast:
		return IsEmbodied(State) && TierNumber(State.Calling.PersonalTier) >= Condition.Value;

	case EConditionType::PlayerInSystemWithHazard:
	{
		if (!IsEmbodied(State))
		{
			return false;
		}
		const int32 Standing = View.Index.FindSystem(State, Player.CurrentSystemID);
		return State.Systems.IsValidIndex(Standing) && State.Systems[Standing].ActiveHazard.IsValid()
			&& (!Condition.Tag.IsValid() || State.Systems[Standing].ActiveHazard.MatchesTag(Condition.Tag));
	}

	case EConditionType::PlayerStatAtLeast:
	{
		const int32* Stat = IsEmbodied(State) ? PlayerStatOf(Player, Condition.Key) : nullptr;
		return Stat && static_cast<float>(*Stat) >= Condition.Value;
	}

	case EConditionType::PlayerHasRelic:
		return IsEmbodied(State) && !Condition.Key.IsNone() && Player.Relics.Contains(Condition.Key);

	case EConditionType::PlayerRoleIs:
		if (!IsEmbodied(State))
		{
			return false;
		}
		return (Condition.Key == KeyRuler && Player.Role == EPlayerRole::Ruler) || (Condition.Key == KeyWanderer && Player.Role == EPlayerRole::Wanderer);

	case EConditionType::PlayerHasStatus:
		return IsEmbodied(State) && Condition.Tag.IsValid() && Player.StatusTags.HasTag(Condition.Tag);

	case EConditionType::PlayerLifeIndexAtLeast:
		return IsEmbodied(State) && static_cast<float>(State.Calling.LifeIndex) >= Condition.Value;

	case EConditionType::PlayerAgeAtLeast:
		// Only with aging on (section 4.4).
		return IsEmbodied(State) && View.Rules.BaseLifespanTicks > 0 && static_cast<float>(State.Tick - State.Calling.BornTick) >= Condition.Value;

	case EConditionType::GalaxyTickAtLeast:
		return static_cast<float>(State.Tick) >= Condition.Value;

	case EConditionType::TranscendentSealed:
	{
		const FTranscendentState* Transcendent = FindTranscendent(State, OtherOf(View, Condition.Key));
		return Transcendent && Transcendent->bSealed && Transcendent->SealedTick >= View.SinceTick;
	}

	case EConditionType::TranscendentAwareOfPlayer:
	{
		const FTranscendentState* Transcendent = FindTranscendent(State, OtherOf(View, Condition.Key));
		return Transcendent && Transcendent->bAwareOfPlayer;
	}

	case EConditionType::FlagSet:
		return Condition.Tag.IsValid() && State.Flags.HasTagExact(Condition.Tag);

	case EConditionType::FlagNotSet:
		return Condition.Tag.IsValid() && !State.Flags.HasTagExact(Condition.Tag);

	case EConditionType::TranscendentDoctrinePresent:
		return Condition.Tag.IsValid() && State.Transcendents.ContainsByPredicate([&Condition](const FTranscendentState& Each)
		{
			return !Each.bSealed && Each.Doctrine.MatchesTagExact(Condition.Tag);
		});

	case EConditionType::CivFallenSphereTicksReached:
	{
		// Key = the number of ticks. The count of the stay grows by one a tick and the slot of a civilization comes once in
		// EventSlotTicks ticks, so exactly one slot of a long enough stay finds it in the window (section 5.11, 16 #25).
		const int32 Ticks = TickCountOf(Condition.Key);
		const int32 Window = FMath::Max(1, View.Rules.EventSlotTicks);
		if (!Civ || Ticks <= 0 || Civ->FallenSphereTicks < Ticks || Civ->FallenSphereTicks - Ticks >= Window)
		{
			return false;
		}
		if (!Condition.Tag.IsValid())
		{
			return true;
		}
		// A doctrine as well: a settled system of the civilization lies, now, in the sphere of an unsealed transcendent of it.
		for (const FTranscendentState& Transcendent : State.Transcendents)
		{
			if (Transcendent.bSealed || !Transcendent.Doctrine.MatchesTagExact(Condition.Tag))
			{
				continue;
			}
			for (const FName SystemID : Transcendent.InfluencedSystems)
			{
				if (IsSettledSystemOf(State, View.Index.FindSystem(State, SystemID), Civ->CivID))
				{
					return true;
				}
			}
		}
		return false;
	}

	case EConditionType::CivResearchAtLeast:
	{
		// A research field is a key the civilization's Research holds; any other Key is false (section 5.4).
		const float* Progress = Civ ? Civ->Research.Find(Condition.Key) : nullptr;
		return Progress && *Progress >= Condition.Value;
	}

	case EConditionType::CivHasNotable:
	{
		const int32 Selected = SelectCiv(State, View.Index, View, Condition.Key);
		return Selected != INDEX_NONE && Condition.Tag.IsValid() && State.Civs[Selected].Notables.ContainsByPredicate([&Condition](const FNotableRecord& Record)
		{
			return Record.LastTick == 0 && Record.Role == Condition.Tag;
		});
	}

	default:
		// An enumerator this build does not know (a row written for a later build).
		return false;
	}
}

bool UCosmicEventSubsystem::CheckAll(TConstArrayView<FCosmicCondition> Conditions, const FCosmicEvalView& View)
{
	for (const FCosmicCondition& Condition : Conditions)
	{
		if (!Check(Condition, View))
		{
			return false;
		}
	}
	return true;
}

bool UCosmicEventSubsystem::Apply(const FCosmicEffect& Effect, FCosmicEvalContext& Ctx)
{
	using namespace CosmicEventImpl;

	FGalaxyState& State = Ctx.State;
	FCivilizationState* Civ = State.Civs.IsValidIndex(Ctx.CivIndex) ? &State.Civs[Ctx.CivIndex] : nullptr;
	const FStarSystemState* System = State.Systems.IsValidIndex(Ctx.SystemIndex) ? &State.Systems[Ctx.SystemIndex] : nullptr;
	FPlayerEntityState& Player = State.Player;

	// Before anything else, the draw of NameNotable included: what a per-tick source may not apply changes nothing.
	if (Ctx.bPerTickSource && !IsPerTickEffect(Effect.Type))
	{
		return false;
	}

	switch (Effect.Type)
	{
	case EEffectType::AddResource:
	{
		ECosmicResource Resource = ECosmicResource::Energy;
		if (!Civ || !ResourceOf(Effect.Key, Resource) || !IsAmount(Effect.Value))
		{
			return false;
		}
		// A stockpile never goes below zero.
		Civ->Stockpile.Add(Resource, FMath::Max(Effect.Value, -Civ->Stockpile.Get(Resource)));
		return true;
	}

	case EEffectType::MultiplyIncome:
	{
		// Only a per-tick source reaches the stockpile: IncomePerTick is rebuilt every tick (section 5.8, 16 #27).
		if (!Civ || !Ctx.bPerTickSource || !IsAmount(Effect.Value))
		{
			return false;
		}
		// The range of Value is a lint; the guard is here. A factor below zero reads as 0: an income never turns
		// negative, so no stockpile goes below zero through it.
		const float Factor = Effect.Value > 0.f ? Effect.Value : 0.f;
		ECosmicResource Resource = ECosmicResource::Energy;
		if (ResourceOf(Effect.Key, Resource))
		{
			// The same single multiplication as the form without a Key below: both give the same float.
			Civ->IncomePerTick.Scale(Resource, Factor);
			return true;
		}
		if (!Effect.Key.IsNone())
		{
			return false;
		}
		// An empty Key scales all five.
		Civ->IncomePerTick.Energy *= Factor;
		Civ->IncomePerTick.Matter *= Factor;
		Civ->IncomePerTick.Insight *= Factor;
		Civ->IncomePerTick.Influence *= Factor;
		Civ->IncomePerTick.Causality *= Factor;
		return true;
	}

	case EEffectType::ShiftEthics:
	{
		float* Axis = Civ ? const_cast<float*>(EthicsAxisOf(Civ->Ethics, Effect.Key)) : nullptr;
		if (!Axis || !IsAmount(Effect.Value))
		{
			return false;
		}
		*Axis = FMath::Clamp(*Axis + Effect.Value, -1.f, 1.f);
		return true;
	}

	case EEffectType::AddTrait:
		if (!Civ || !Effect.Tag.IsValid())
		{
			return false;
		}
		Civ->Traits.AddTag(Effect.Tag);
		return true;

	case EEffectType::RemoveTrait:
		if (!Civ || !Effect.Tag.IsValid())
		{
			return false;
		}
		Civ->Traits.RemoveTag(Effect.Tag);
		return true;

	case EEffectType::ChangeStability:
		if (!Civ || !IsAmount(Effect.Value))
		{
			return false;
		}
		Civ->Stability = FMath::Clamp(Civ->Stability + Effect.Value, 0.f, 1.f);
		return true;

	case EEffectType::ChangeRelation:
	{
		// The entry is made on first write; a missing one reads as 0 (section 5.5). A party that does not resolve gets
		// no entry: Relations is saved, and its keys are peoples, transcendents of the run and celestial rows only.
		const FName Other = RelationPartyOf(State, Ctx.Index, Ctx.Tables, Ctx, Effect.Key);
		if (!Civ || Other.IsNone() || !IsAmount(Effect.Value))
		{
			return false;
		}
		float& Relation = Civ->Relations.FindOrAdd(Other);
		Relation = FMath::Clamp(Relation + Effect.Value, -1.f, 1.f);
		return true;
	}

	case EEffectType::SpawnHazard:
	{
		// The one path of every hazard; the claim is placed by the claim rule alone, which holds Law.Remnant (section 4.2).
		FName HazardRow;
		const FHazardDef* Row = System ? Ctx.Tables.FindHazardByTag(Effect.Tag, &HazardRow) : nullptr;
		if (!Row || Row->SpawnRule == ECosmicHazardSpawn::ClaimRule)
		{
			return false;
		}
		return CosmicHazards::Place(Ctx, Ctx.SystemIndex, HazardRow);
	}

	case EEffectType::ResolveHazard:
		if (!System || !System->ActiveHazard.IsValid() || (Effect.Tag.IsValid() && !System->ActiveHazard.MatchesTag(Effect.Tag)))
		{
			return false;
		}
		return CosmicHazards::Resolve(Ctx, Ctx.SystemIndex);

	case EEffectType::TransferSystem:
	{
		// An empty Key lets the system go to no one (section 16 #29 (6)); any other Key is a living civilization.
		int32 ToCivIndex = INDEX_NONE;
		if (!Effect.Key.IsNone())
		{
			ToCivIndex = Ctx.Index.FindCiv(State, Effect.Key);
			if (ToCivIndex == INDEX_NONE)
			{
				return false;
			}
		}
		return System && CosmicHazards::Transfer(Ctx, Ctx.SystemIndex, ToCivIndex);
	}

	case EEffectType::ChangeReverence:
		if (!Civ || !IsAmount(Effect.Value))
		{
			return false;
		}
		Civ->ReverenceToPlayer = FMath::Clamp(Civ->ReverenceToPlayer + Effect.Value, 0.f, 1.f);
		return true;

	case EEffectType::GrantPower:
		// The calling gains it through the vessel: nothing during the interregnum.
		if (!IsEmbodied(State) || !Effect.Tag.IsValid())
		{
			return false;
		}
		State.Calling.Powers.AddTag(Effect.Tag);
		return true;

	case EEffectType::PlayerStatDelta:
	{
		int32* Stat = IsEmbodied(State) ? const_cast<int32*>(PlayerStatOf(Player, Effect.Key)) : nullptr;
		if (!Stat || !IsAmount(Effect.Value))
		{
			return false;
		}
		*Stat = FMath::Clamp(*Stat + FMath::RoundToInt32(Effect.Value), 0, MaxPersonalStat);
		return true;
	}

	case EEffectType::SetFlag:
		if (!Effect.Tag.IsValid())
		{
			return false;
		}
		State.Flags.AddTag(Effect.Tag);
		return true;

	case EEffectType::KillPlayer:
	{
		if (!IsEmbodied(State))
		{
			return false;
		}
		// What happens to a system reaches only a wanderer standing in it; without a system the effect reaches the ruler
		// of the subject civilization (its extinction), or the vessel wherever it is when no civilization is named.
		bool bReaches = false;
		if (System)
		{
			bReaches = Player.Role == EPlayerRole::Wanderer && Player.CurrentSystemID == System->SystemID;
		}
		else if (Civ)
		{
			bReaches = Player.Role == EPlayerRole::Ruler && Player.RuledCivID == Civ->CivID;
		}
		else
		{
			bReaches = true;
		}
		// Birth protection: a shielded kill leaves the slot free.
		if (!bReaches || State.Tick - State.Calling.BornTick < Ctx.Rules.BirthProtectionTicks)
		{
			return false;
		}
		return RecordRoleEffect(Ctx, TEXT("Effect.KillPlayer"), Effect.Tag);
	}

	case EEffectType::DeposeRuler:
		if (!IsEmbodied(State) || !Civ || Player.Role != EPlayerRole::Ruler || Player.RuledCivID != Civ->CivID)
		{
			return false;
		}
		return RecordRoleEffect(Ctx, TEXT("Effect.DeposeRuler"), Effect.Tag);

	case EEffectType::ChangePopulation:
		// Value = ratio, on the people as a whole: there is no population per system (section 4.1).
		if (!Civ || !IsAmount(Effect.Value))
		{
			return false;
		}
		Civ->Population = FMath::Max(0.f, Civ->Population * (1.f + Effect.Value));
		return true;

	case EEffectType::NameNotable:
	{
		// One draw per application, before any guard of its own: the draws of a row follow its effect list alone
		// (section 9.2). A per-tick source never gets this far.
		const float Roll = Ctx.Stream.GetFraction();
		return AddNotable(Effect, Ctx, Roll);
	}

	default:
		return false;
	}
}

void UCosmicEventSubsystem::ApplyAll(TConstArrayView<FCosmicEffect> Effects, FCosmicEvalContext& Ctx)
{
	for (const FCosmicEffect& Effect : Effects)
	{
		Apply(Effect, Ctx);
	}
}

FName UCosmicEventSubsystem::CooldownKey(FName EventRow, FName CivID)
{
	return FName(*FString::Printf(TEXT("%s@%s"), *EventRow.ToString(), *CivID.ToString()));
}

int32 UCosmicEventSubsystem::EvaluateAndFire(FGalaxyState& State, FRandomStream& Stream)
{
	using namespace CosmicEventImpl;

	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	if (&State != &Sim->GetState())
	{
		UE_LOG(LogCosmic, Error, TEXT("EvaluateAndFire: the state handed in is not the live state"));
		return 0;
	}
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("EvaluateAndFire called from a handler of a Core Logic delegate: a handler may only read the state"));
		return 0;
	}

	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const FCosmicDataTables& Tables = Sim->GetTables();
	const FCosmicRulesDef& Rules = Sim->GetRules();

	// An entry that is over and a missing one mean the same, so the sweep changes nothing but the size of the save.
	if (State.Tick % CooldownSweepTicks == 0)
	{
		for (auto It = State.EventCooldownUntilTick.CreateIterator(); It; ++It)
		{
			if (It.Value() <= State.Tick)
			{
				It.RemoveCurrent();
			}
		}
	}

	// The ranges of the two knobs are a lint; the guard is here. Each report names its own knob and nothing else.
	if (Rules.EventSlotTicks < 1)
	{
		UE_LOG(LogCosmic, Error, TEXT("DT_Rules: EventSlotTicks %d is below 1, read as 1"), Rules.EventSlotTicks);
	}
	const int64 SlotTicks = FMath::Max(1, Rules.EventSlotTicks);
	// Written so that a value that is not a number, or not finite, is caught as well.
	const bool bChanceInRange = Rules.EventChancePerSlot >= 0.f && Rules.EventChancePerSlot <= TNumericLimits<float>::Max();
	if (!bChanceInRange)
	{
		UE_LOG(LogCosmic, Error, TEXT("DT_Rules: EventChancePerSlot %f is below 0 or not finite, read as 0"), Rules.EventChancePerSlot);
	}
	const float Chance = bChanceInRange ? Rules.EventChancePerSlot : 0.f;

	// 1. Who has a slot in this tick, and one number for each, before any row is looked at.
	TArray<int32, TInlineAllocator<16>> SlotCivs;
	TArray<float, TInlineAllocator<16>> Rolls;
	const int64 TickPhase = State.Tick % SlotTicks;
	for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
	{
		const FCivilizationState& Civ = State.Civs[CivIndex];
		const int64 Phase = static_cast<int64>(CosmicSeed::DeriveForName(State.Seed, Civ.CivID, CosmicSeed::Purpose::EventSlot) % static_cast<uint32>(SlotTicks));
		if (Phase != TickPhase || UnitsOf(State, Index, Civ).SettledSystems.IsEmpty())
		{
			continue;
		}
		SlotCivs.Add(CivIndex);
		Rolls.Add(Stream.GetFraction());
	}
	if (SlotCivs.IsEmpty())
	{
		return 0;
	}

	// Rows in row-name order, never in the table's own (section 9.2).
	const TArray<FName> RowNames = Tables.GetEventRowNames();
	int32 Fired = 0;

	// 2. Each civilization at its turn, against the state as it is then.
	for (int32 Slot = 0; Slot < SlotCivs.Num(); ++Slot)
	{
		const int32 CivIndex = SlotCivs[Slot];
		const FName CivID = State.Civs[CivIndex].CivID;

		auto Gather = [&](ECosmicEventTrigger Trigger, TArray<FSlotPair, TInlineAllocator<16>>& OutPairs)
		{
			const FSlotUnits Units = UnitsOf(State, Index, State.Civs[CivIndex]);
			FCosmicEvalView View(State, Index, Tables, Rules);
			View.CivIndex = CivIndex;
			View.GalaxyRow = State.Civs[CivIndex].GalaxyRow;
			for (const FName RowName : RowNames)
			{
				const FCosmicEventDef* Row = Tables.FindEvent(RowName);
				if (!Row || Row->Trigger != Trigger)
				{
					continue;
				}
				// Conditions first. The cooldown key is a string built from two names, so it is made at most once per
				// row and only for a row with a unit that passed: most rows fail a condition (section 9.4).
				View.SourceRow = RowName;
				const int32 PairsBefore = OutPairs.Num();
				if (Row->Subject == ECosmicEventSubject::SettledSystem)
				{
					for (const int32 SystemIndex : Units.SettledSystems)
					{
						View.SystemIndex = SystemIndex;
						if (CheckAll(Row->Conditions, View))
						{
							OutPairs.Add({ RowName, Row, SystemIndex });
						}
					}
				}
				else
				{
					View.SystemIndex = Units.Capital;
					if (CheckAll(Row->Conditions, View))
					{
						OutPairs.Add({ RowName, Row, Units.Capital });
					}
				}
				if (OutPairs.Num() > PairsBefore)
				{
					if (const int64* Until = State.EventCooldownUntilTick.Find(CooldownKey(RowName, CivID)); Until && *Until > State.Tick)
					{
						OutPairs.SetNum(PairsBefore, EAllowShrinking::No);
					}
				}
			}
		};

		auto FirePair = [&](const FSlotPair& Pair)
		{
			if (Pair.Row->CooldownTicks > 0)
			{
				State.EventCooldownUntilTick.Add(CooldownKey(Pair.RowName, CivID), State.Tick + Pair.Row->CooldownTicks);
			}
			FCosmicEvalContext Ctx(State, Stream, Index, Tables, Rules);
			Ctx.CivIndex = CivIndex;
			Ctx.SystemIndex = Pair.SystemIndex;
			Ctx.GalaxyRow = State.Civs[CivIndex].GalaxyRow;
			FireResolved(Pair.RowName, *Pair.Row, Ctx);
			++Fired;
		};

		// a. Standing rows: every pair that still passes right before its turn.
		TArray<FSlotPair, TInlineAllocator<16>> Standing;
		Gather(ECosmicEventTrigger::Standing, Standing);
		for (const FSlotPair& Pair : Standing)
		{
			// The unit, the conditions and the cooldown again: an earlier row of this slot may have changed any of them.
			int32 SystemIndex = Pair.SystemIndex;
			if (Pair.Row->Subject == ECosmicEventSubject::SettledSystem)
			{
				if (!IsSettledSystemOf(State, SystemIndex, CivID))
				{
					continue;
				}
			}
			else
			{
				SystemIndex = UnitsOf(State, Index, State.Civs[CivIndex]).Capital;
			}
			if (const int64* Until = State.EventCooldownUntilTick.Find(CooldownKey(Pair.RowName, CivID)); Until && *Until > State.Tick)
			{
				continue;
			}
			FCosmicEvalView View(State, Index, Tables, Rules);
			View.CivIndex = CivIndex;
			View.SystemIndex = SystemIndex;
			View.GalaxyRow = State.Civs[CivIndex].GalaxyRow;
			View.SourceRow = Pair.RowName;
			if (!CheckAll(Pair.Row->Conditions, View))
			{
				continue;
			}
			FirePair({ Pair.RowName, Pair.Row, SystemIndex });
		}

		// b. Lottery rows: at most one, picked by the number drawn in step 1. Each pair has an absolute share.
		TArray<FSlotPair, TInlineAllocator<16>> Lottery;
		Gather(ECosmicEventTrigger::Lottery, Lottery);
		// The share of every pair, once: BaseWeight x weight factor x chance. The factor is the sphere weight of the
		// transcendents (section 4.2); traits and edicts add theirs in Phase 4.
		TArray<float, TInlineAllocator<16>> Shares;
		float Total = 0.f;
		for (const FSlotPair& Pair : Lottery)
		{
			// A weight below 0, not a number or not finite reads as 0 (a compare: FMath::Max lets NaN and infinity through, and
			// one such share would silence every row of the slot).
			const float Weight = (Pair.Row->BaseWeight > 0.f && Pair.Row->BaseWeight <= TNumericLimits<float>::Max()) ? Pair.Row->BaseWeight : 0.f;
			const float Share = Weight * SphereWeightFactor(State, Tables, Rules, Pair.Row->Category, Pair.SystemIndex) * Chance;
			Shares.Add(Share);
			Total += Share;
		}
		// Shares that add up to more than 1 are scaled down together by scaling the number instead: the sum below repeats
		// the additions that made Total and so ends exactly on it, and a number below 1 times Total is below Total.
		// Dividing every share would round, and leave numbers at the top on which no row fires.
		const float Pick = Total > 1.f ? Rolls[Slot] * Total : Rolls[Slot];
		float Reached = 0.f;
		for (int32 PairIndex = 0; PairIndex < Lottery.Num(); ++PairIndex)
		{
			Reached += Shares[PairIndex];
			if (Pick < Reached)
			{
				FirePair(Lottery[PairIndex]);
				break;
			}
		}
	}
	return Fired;
}

bool UCosmicEventSubsystem::FireRow(FName RowName, const FCosmicEvalContext& Ctx)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	if (&Ctx.State != &Sim->GetState())
	{
		UE_LOG(LogCosmic, Error, TEXT("FireRow: the context is not over the live state"));
		return false;
	}
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("FireRow called from a handler of a Core Logic delegate: a handler may only read the state"));
		return false;
	}

	const FCosmicEventDef* Row = Ctx.Tables.FindEvent(RowName);
	if (!Row)
	{
		return false;
	}

	// A context of its own: the caller's subject, nothing else of the caller's.
	FCosmicEvalContext Own(Ctx.State, Ctx.Stream, Ctx.Index, Ctx.Tables, Ctx.Rules);
	static_cast<FCosmicEvalSubject&>(Own) = Ctx;
	Own.SourceRow = RowName;
	if (!CheckAll(Row->Conditions, Own.View()))
	{
		return false;
	}
	FireResolved(RowName, *Row, Own);
	return true;
}

void UCosmicEventSubsystem::FireResolved(FName RowName, const FCosmicEventDef& Row, FCosmicEvalContext& Ctx)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Sim && Log);

	// The hazard the subject system holds before the effects: the {Hazard} of the line, also when an effect resolved it.
	FText HazardName;
	if (Ctx.State.Systems.IsValidIndex(Ctx.SystemIndex))
	{
		if (const FHazardDef* Hazard = Ctx.Tables.FindHazardByTag(Ctx.State.Systems[Ctx.SystemIndex].ActiveHazard))
		{
			HazardName = Hazard->DisplayName;
		}
	}

	// 1. The world: every effect, in array order; then what the vessel's people received is known before any line.
	Ctx.SourceRow = RowName;
	Ctx.bPerTickSource = false;
	ApplyAll(Row.Effects, Ctx);
	FollowUpKnowing(Ctx);

	// 2. The line. A Civilization row is about the people alone, a SettledSystem row about the place and its people.
	const FGalaxyState& State = Ctx.State;
	const FCivilizationState* Civ = State.Civs.IsValidIndex(Ctx.CivIndex) ? &State.Civs[Ctx.CivIndex] : nullptr;
	const FStarSystemState* System = State.Systems.IsValidIndex(Ctx.SystemIndex) ? &State.Systems[Ctx.SystemIndex] : nullptr;

	FCosmicLine Line;
	Line.Category = Row.LogCategory;
	Line.Severity = Row.Severity;
	Line.FocusCiv = Civ ? Civ->CivID : FName();
	Line.FocusSystem = (Row.Subject == ECosmicEventSubject::SettledSystem && System) ? System->SystemID : FName();
	Line.SourceRow = RowName;
	Line.MinViewerTier = Row.MinTierToReveal;

	FFormatNamedArguments Args;
	Args.Add(TEXT("Civ"), Civ ? Civ->CivName : FText::GetEmpty());
	Args.Add(TEXT("System"), System ? System->DisplayName : FText::GetEmpty());
	Args.Add(TEXT("Hazard"), HazardName);

	Log->PushVariants(Line, Row.LogTemplates, Args);

	// The lower speaker's sentence, for the viewers the line above is not told to. It goes with a line (a row without
	// templates tells nothing) and needs a viewer tier below the window of that line: tier numbers start at 1, so the
	// lowest MinTierToReveal with anybody below it is 2 (and 0 as an upper bound would mean "no bound"). Whenever the
	// row's data allow it, it is told: the windows add no tell draw (section 8.1). The sign row's templates call {Civ}
	// and {Hazard} only (Data.Integrity): it copies the focus of the line above, which names no system for a Civilization
	// row, and a sign row may serve rows of either subject.
	if (!Row.SignRow.IsNone() && !Row.LogTemplates.IsEmpty() && Row.MinTierToReveal > 1)
	{
		if (const FLogTemplateDef* Sign = Ctx.Tables.FindLogTemplate(Row.SignRow))
		{
			FCosmicLine SignLine = Line;
			SignLine.Category = Sign->Category;
			SignLine.Severity = FMath::Min<uint8>(Row.Severity, 1);
			SignLine.MinViewerTier = 0;
			SignLine.MaxViewerTier = Row.MinTierToReveal - 1;
			Log->PushVariants(SignLine, Sign->Templates, Args);
		}
		else
		{
			UE_LOG(LogCosmic, Error, TEXT("Event row %s names the sign row %s, which DT_LogTemplates does not hold"), *RowName.ToString(), *Row.SignRow.ToString());
		}
	}

	// 3. What the effects did, told after the row's own line: a cause comes before what follows from it.
	FollowUpNotes(Ctx);

	// 4. The presentation layer.
	if (Sim->IsBroadcastEnabled())
	{
		FCosmicEventNotice Notice;
		Notice.EventRow = RowName;
		Notice.Category = Row.Category;
		Notice.Severity = Row.Severity;
		Notice.SystemID = System ? System->SystemID : FName();
		Notice.CivID = Civ ? Civ->CivID : FName();
		Notice.Tick = State.Tick;
		Sim->BeginBroadcast();
		OnEventFired.Broadcast(Notice);
		Sim->EndBroadcast();
	}
}

void UCosmicEventSubsystem::TellHazardLine(const FCosmicEvalContext& Ctx, ECosmicHazardLine Line, FName HazardRow, int32 SystemIndex, int32 CivIndex)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Sim && Log);
	if (&Ctx.State != &Sim->GetState())
	{
		UE_LOG(LogCosmic, Error, TEXT("TellHazardLine: the context is not over the live state"));
		return;
	}
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("TellHazardLine called from a handler of a Core Logic delegate: a handler may only read the state"));
		return;
	}
	const FHazardDef* Row = Ctx.Tables.FindHazard(HazardRow);
	if (!Row)
	{
		return;
	}
	const TArray<FText>& Templates = Line == ECosmicHazardLine::Outbreak ? Row->OutbreakTemplates
		: Line == ECosmicHazardLine::Expiry ? Row->ExpireTemplates : Row->ResolveTemplates;

	const FGalaxyState& State = Ctx.State;
	const FCivilizationState* Civ = State.Civs.IsValidIndex(CivIndex) ? &State.Civs[CivIndex] : nullptr;
	const FStarSystemState* System = State.Systems.IsValidIndex(SystemIndex) ? &State.Systems[SystemIndex] : nullptr;

	FCosmicLine HazardLine;
	HazardLine.Category = Row->LogCategory;
	HazardLine.Severity = Row->Severity;
	HazardLine.FocusSystem = System ? System->SystemID : FName();
	HazardLine.FocusCiv = Civ ? Civ->CivID : FName();
	HazardLine.SourceRow = HazardRow;

	FFormatNamedArguments Args;
	Args.Add(TEXT("Civ"), Civ ? Civ->CivName : FText::GetEmpty());
	Args.Add(TEXT("System"), System ? System->DisplayName : FText::GetEmpty());
	Args.Add(TEXT("Hazard"), Row->DisplayName);
	Log->PushVariants(HazardLine, Templates, Args);
}

void UCosmicEventSubsystem::FollowUpKnowing(FCosmicEvalContext& Ctx)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	if (&Ctx.State != &Sim->GetState())
	{
		UE_LOG(LogCosmic, Error, TEXT("FollowUpKnowing: the context is not over the live state"));
		return;
	}
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("FollowUpKnowing called from a handler of a Core Logic delegate: a handler may only read the state"));
		return;
	}

	// Taken out in their order; the other notes stay for FollowUpNotes.
	TArray<FCosmicWorldNote, TInlineAllocator<2>> Received;
	for (const FCosmicWorldNote& Note : Ctx.Notes)
	{
		if (Note.Kind == ECosmicWorldNote::SystemReceived)
		{
			Received.Add(Note);
		}
	}
	if (Received.IsEmpty())
	{
		return;
	}
	Ctx.Notes.RemoveAll([](const FCosmicWorldNote& Note) { return Note.Kind == ECosmicWorldNote::SystemReceived; });

	// Only what the people the vessel rules received: the calling sees through its own people (section 8.1).
	const FGalaxyState& State = Ctx.State;
	if (State.Calling.LifeState != ELifeState::Embodied || State.Player.Role != EPlayerRole::Ruler)
	{
		return;
	}
	for (const FCosmicWorldNote& Note : Received)
	{
		if (State.Civs.IsValidIndex(Note.CivIndex) && State.Civs[Note.CivIndex].CivID == State.Player.RuledCivID && State.Systems.IsValidIndex(Note.SystemIndex))
		{
			// Known first, so that the meeting line about the place is told: the people that gave it is the other party
			// of the event (section 8.1).
			const FName SystemID = State.Systems[Note.SystemIndex].SystemID;
			Sim->NoteKnown(SystemID);
			if (State.Civs.IsValidIndex(Note.FromCivIndex))
			{
				Sim->NoteMet(State.Civs[Note.FromCivIndex].CivID, SystemID);
			}
		}
	}
}

void UCosmicEventSubsystem::FollowUpNotes(FCosmicEvalContext& Ctx)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	if (&Ctx.State != &Sim->GetState())
	{
		UE_LOG(LogCosmic, Error, TEXT("FollowUpNotes: the context is not over the live state"));
		return;
	}
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("FollowUpNotes called from a handler of a Core Logic delegate: a handler may only read the state"));
		return;
	}

	// What the vessel's people received is known before any line below (a caller that told its own line first has
	// followed it already).
	FollowUpKnowing(Ctx);

	// Taken out first: a row fired below follows its own notes on its own context, never these.
	const TArray<FCosmicWorldNote, TInlineAllocator<2>> Notes = MoveTemp(Ctx.Notes);
	Ctx.Notes.Reset();
	for (const FCosmicWorldNote& Note : Notes)
	{
		switch (Note.Kind)
		{
		case ECosmicWorldNote::HazardPlaced:
		{
			TellHazardLine(Ctx, ECosmicHazardLine::Outbreak, Note.HazardRow, Note.SystemIndex, Note.CivIndex);
			const FHazardDef* Row = Ctx.Tables.FindHazard(Note.HazardRow);
			if (Row && !Row->OutbreakRow.IsNone() && CosmicEventImpl::NestedOutbreakRows >= CosmicEventImpl::MaxNestedOutbreakRows)
			{
				UE_LOG(LogCosmic, Error, TEXT("FollowUpNotes: the outbreak row %s of %s is not fired: outbreak rows nested %d deep (does a row place again what its outbreak row resolves?)"), *Row->OutbreakRow.ToString(), *Note.HazardRow.ToString(), CosmicEventImpl::NestedOutbreakRows);
			}
			else if (Row && !Row->OutbreakRow.IsNone())
			{
				TGuardValue<int32> Nesting(CosmicEventImpl::NestedOutbreakRows, CosmicEventImpl::NestedOutbreakRows + 1);
				// The place and its people are the subject; the stream is the caller's, so a name the row gives is drawn there.
				FCosmicEvalContext Outbreak(Ctx.State, Ctx.Stream, Ctx.Index, Ctx.Tables, Ctx.Rules);
				Outbreak.CivIndex = Note.CivIndex;
				Outbreak.SystemIndex = Note.SystemIndex;
				Outbreak.GalaxyRow = Ctx.State.Systems.IsValidIndex(Note.SystemIndex) ? Ctx.State.Systems[Note.SystemIndex].GalaxyRow : FName();
				FireRow(Row->OutbreakRow, Outbreak);
			}
			break;
		}
		case ECosmicWorldNote::HazardResolved:
			TellHazardLine(Ctx, ECosmicHazardLine::Resolution, Note.HazardRow, Note.SystemIndex, Note.CivIndex);
			break;
		case ECosmicWorldNote::SystemReceived:
			// Followed above, never told.
			break;
		}
	}
}
