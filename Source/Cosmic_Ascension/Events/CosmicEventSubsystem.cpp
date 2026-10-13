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
#include "Misc/CString.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"

// Helpers of this file. A named namespace: the module is built as one unity file, so an anonymous one would share its
// names with every other .cpp.
namespace CosmicEventImpl
{
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

		// Guard 3: at most one open record per role. The seat changed hands.
		for (FNotableRecord& Record : Civ.Notables)
		{
			if (Record.LastTick == 0 && Record.Role == Effect.Tag)
			{
				Record.LastTick = State.Tick;
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
		const int32 Ticks = Condition.Key.IsNone() ? 0 : FCString::Atoi(*Condition.Key.ToString());
		return Civ && Ticks > 0 && Civ->LowStabilityTicks >= Ticks;
	}

	case EConditionType::CivRelationAtLeast:
	{
		// A missing entry is 0, neutral (section 5.5).
		const FName Other = OtherOf(View, Condition.Key);
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
		return System && System->MegaprojectCompletedTick > 0 && System->MegaprojectCompletedTick >= View.SinceTick;

	case EConditionType::SystemPressureAtLeast:
		// Spiritual pressure arrives with the pressure step of Phase 2 (section 4.2).
		return false;

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

	switch (Effect.Type)
	{
	case EEffectType::AddResource:
	{
		ECosmicResource Resource = ECosmicResource::Energy;
		if (!Civ || !ResourceOf(Effect.Key, Resource))
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
		if (!Civ || !Ctx.bPerTickSource)
		{
			return false;
		}
		ECosmicResource Resource = ECosmicResource::Energy;
		if (ResourceOf(Effect.Key, Resource))
		{
			Civ->IncomePerTick.Add(Resource, Civ->IncomePerTick.Get(Resource) * (Effect.Value - 1.f));
			return true;
		}
		if (!Effect.Key.IsNone())
		{
			return false;
		}
		// An empty Key scales all five.
		Civ->IncomePerTick.Energy *= Effect.Value;
		Civ->IncomePerTick.Matter *= Effect.Value;
		Civ->IncomePerTick.Insight *= Effect.Value;
		Civ->IncomePerTick.Influence *= Effect.Value;
		Civ->IncomePerTick.Causality *= Effect.Value;
		return true;
	}

	case EEffectType::ShiftEthics:
	{
		float* Axis = Civ ? const_cast<float*>(EthicsAxisOf(Civ->Ethics, Effect.Key)) : nullptr;
		if (!Axis)
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
		if (!Civ)
		{
			return false;
		}
		Civ->Stability = FMath::Clamp(Civ->Stability + Effect.Value, 0.f, 1.f);
		return true;

	case EEffectType::ChangeRelation:
	{
		// The entry is made on first write; a missing one reads as 0 (section 5.5).
		const FName Other = OtherOf(Ctx, Effect.Key);
		if (!Civ || Other.IsNone())
		{
			return false;
		}
		float& Relation = Civ->Relations.FindOrAdd(Other);
		Relation = FMath::Clamp(Relation + Effect.Value, -1.f, 1.f);
		return true;
	}

	case EEffectType::SpawnHazard:
	case EEffectType::ResolveHazard:
	case EEffectType::TransferSystem:
		UE_LOG(LogCosmic, Error, TEXT("Apply: effect type %d of row %s reaches into the hazard stage, which has no path for it yet (hazard step of Phase 2)"),
			static_cast<int32>(Effect.Type), *Ctx.SourceRow.ToString());
		return false;

	case EEffectType::ChangeReverence:
		if (!Civ)
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
		if (!Stat)
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
		if (!Civ)
		{
			return false;
		}
		Civ->Population = FMath::Max(0.f, Civ->Population * (1.f + Effect.Value));
		return true;

	case EEffectType::NameNotable:
	{
		// One draw per application, before any guard: the draws of a row follow its effect list alone (section 9.2).
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

	if (Rules.EventSlotTicks < 1 || Rules.EventChancePerSlot < 0.f)
	{
		UE_LOG(LogCosmic, Error, TEXT("DT_Rules: EventSlotTicks %d and EventChancePerSlot %f are out of range, read as 1 and 0"), Rules.EventSlotTicks, Rules.EventChancePerSlot);
	}
	const int64 SlotTicks = FMath::Max(1, Rules.EventSlotTicks);
	const float Chance = FMath::Max(0.f, Rules.EventChancePerSlot);

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
				if (const int64* Until = State.EventCooldownUntilTick.Find(CooldownKey(RowName, CivID)); Until && *Until > State.Tick)
				{
					continue;
				}
				View.SourceRow = RowName;
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
		float Total = 0.f;
		for (const FSlotPair& Pair : Lottery)
		{
			// The weight factor of transcendents, traits and edicts arrives with the pressure step; until then it is 1.
			Total += FMath::Max(0.f, Pair.Row->BaseWeight) * Chance;
		}
		const float Scale = Total > 1.f ? 1.f / Total : 1.f;
		float Reached = 0.f;
		for (const FSlotPair& Pair : Lottery)
		{
			Reached += FMath::Max(0.f, Pair.Row->BaseWeight) * Chance * Scale;
			if (Rolls[Slot] < Reached)
			{
				FirePair(Pair);
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

	// 1. The world: every effect, in array order.
	Ctx.SourceRow = RowName;
	Ctx.bPerTickSource = false;
	ApplyAll(Row.Effects, Ctx);

	// 2. The line. A Civilization row is about the people alone, a SettledSystem row about the place and its people.
	const FGalaxyState& State = Ctx.State;
	const FCivilizationState* Civ = State.Civs.IsValidIndex(Ctx.CivIndex) ? &State.Civs[Ctx.CivIndex] : nullptr;
	const FStarSystemState* System = State.Systems.IsValidIndex(Ctx.SystemIndex) ? &State.Systems[Ctx.SystemIndex] : nullptr;
	const int32 HomeIndex = Civ ? Ctx.Index.FindSystem(State, Civ->HomeSystemID) : INDEX_NONE;

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
	Args.Add(TEXT("HomeSystem"), State.Systems.IsValidIndex(HomeIndex) ? State.Systems[HomeIndex].DisplayName : FText::GetEmpty());

	Log->PushVariants(Line, Row.LogTemplates, Args);

	// The lower speaker's sentence, for the viewers the line above is not told to. Always told when the row has one, so
	// the tell draws of a fire follow the row and never the viewer (section 8.1).
	if (!Row.SignRow.IsNone() && Row.MinTierToReveal > 0)
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

	// 3. The presentation layer.
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
