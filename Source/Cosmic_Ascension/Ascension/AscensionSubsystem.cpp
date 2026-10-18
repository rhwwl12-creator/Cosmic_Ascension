// Cosmic Ascension - Ascension layer.

#include "Ascension/AscensionSubsystem.h"

#include "Core/GalaxyState.h"
#include "Cosmic_Ascension.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/GameInstance.h"
#include "Events/CosmicEventSubsystem.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Log/CosmicLogSubsystem.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"

void UAscensionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogCosmic, Log, TEXT("UAscensionSubsystem initialized"));
}

void UAscensionSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UAscensionSubsystem::EvaluateFeats(FGalaxyState& State, FRandomStream& Rng)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	// The lines go to the live log and the follow-up notes refuse another state: a copy would split the ascension.
	if (&State != &Sim->GetState())
	{
		UE_LOG(LogCosmic, Error, TEXT("EvaluateFeats: the state handed in is not the live state"));
		return;
	}
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("EvaluateFeats called from a handler of a Core Logic delegate: a handler may only read the state"));
		return;
	}

	// The people the vessel rules first, judged from the birth of the vessel (section 4.4).
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	int32 RuledIndex = INDEX_NONE;
	if (State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Ruler)
	{
		RuledIndex = Index.FindCiv(State, State.Player.RuledCivID);
		if (State.Civs.IsValidIndex(RuledIndex) && State.Civs[RuledIndex].bIsPlayerRuled)
		{
			JudgePeople(State, Rng, RuledIndex, State.Calling.BornTick, true);
		}
	}

	// Then every other people, judged from the beginning.
	for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
	{
		if (!State.Civs[CivIndex].bIsPlayerRuled)
		{
			JudgePeople(State, Rng, CivIndex, 0, false);
		}
	}
}

bool UAscensionSubsystem::JudgePeople(FGalaxyState& State, FRandomStream& Rng, int32 CivIndex, int64 SinceTick, bool bPlayer)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	const FCosmicDataTables& Tables = Sim->GetTables();
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const FCosmicRulesDef& Rules = Sim->GetRules();

	// The settled systems the people owns, in OwnedSystems order: a feat holds on one of them. An entry of OwnedSystems
	// whose owner is another people counts for nothing, as in the economy stage.
	TArray<int32, TInlineAllocator<8>> Settled;
	for (const FName SystemID : State.Civs[CivIndex].OwnedSystems)
	{
		const int32 SystemIndex = Index.FindSystem(State, SystemID);
		if (State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == State.Civs[CivIndex].CivID && CosmicState::IsSettled(State.Systems[SystemIndex]))
		{
			Settled.Add(SystemIndex);
		}
	}
	if (Settled.IsEmpty())
	{
		return false;
	}

	for (const FName FeatRow : Tables.GetFeatRowNames())
	{
		const FAscensionFeatDef* Feat = Tables.FindFeat(FeatRow);
		const EAscensionTier Tier = State.Civs[CivIndex].Tier;
		if (!Feat || !Feat->IsJudgedRulerRowAt(Tier))
		{
			// A feat is one rung reached by a deed (section 3.2): a Ruler row of this rung that is not would lift a people
			// every tick, skip a rung or pull it down, and one without a condition would lift every people of its rung at once.
			if (Feat && Feat->Path == EPlayerRole::Ruler && Feat->RequiredTier == Tier)
			{
				UE_LOG(LogCosmic, Error, TEXT("DT_Feats row %s is not one rung up with a condition: not judged"), *FeatRow.ToString());
			}
			continue;
		}
		for (const int32 SystemIndex : Settled)
		{
			FCosmicEvalView View(State, Index, Tables, Rules);
			View.CivIndex = CivIndex;
			View.SystemIndex = SystemIndex;
			View.GalaxyRow = State.Civs[CivIndex].GalaxyRow;
			View.SourceRow = FeatRow;
			View.SinceTick = SinceTick;
			if (UCosmicEventSubsystem::CheckAll(Feat->CompletionConditions, View))
			{
				FCosmicEvalContext Ctx(State, Rng, Index, Tables, Rules);
				Ctx.CivIndex = CivIndex;
				Ctx.SystemIndex = SystemIndex;
				Ctx.GalaxyRow = State.Civs[CivIndex].GalaxyRow;
				Ctx.SourceRow = FeatRow;
				Ctx.SinceTick = SinceTick;
				Ascend(Ctx, FeatRow, *Feat, bPlayer);
				return true;
			}
		}
	}
	return false;
}

void UAscensionSubsystem::Ascend(FCosmicEvalContext& Ctx, FName FeatRow, const FAscensionFeatDef& Feat, bool bPlayer)
{
	static const FGameplayTag AscensionCategory = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Ascension")));
	static const FGameplayTag AscendantTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Notable.Ascendant")));
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	UCosmicEventSubsystem* Events = GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>();
	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Sim && Events && Log);
	FGalaxyState& State = Ctx.State;
	const int32 CivIndex = Ctx.CivIndex;

	// The rung, then its price (section 3.2).
	State.Civs[CivIndex].Tier = Feat.TargetTier;
	UCosmicEventSubsystem::ApplyAll(Feat.AscensionCost, Ctx);

	// What the calling keeps (section 4.4). CompletedFeats is the ordered log of the climbs; the life's peak counts the rung
	// of the people the vessel rules (section 16 #67 (1), default).
	if (bPlayer)
	{
		State.Calling.CompletedFeats.Add(FeatRow);
		if (!State.Calling.Lives.IsEmpty())
		{
			FLifeRecord& Life = State.Calling.Lives.Last();
			Life.Feats.Add(FeatRow);
			if (static_cast<int32>(Feat.TargetTier) > static_cast<int32>(Life.PeakTier))
			{
				Life.PeakTier = Feat.TargetTier;
			}
		}
		State.Calling.Powers.AppendTags(Feat.GrantedPowers);
	}

	// Notable hooks (section 7.5): the one who crossed first, and the end of persons at the fourth rung.
	const int32 NewTier = static_cast<int32>(Feat.TargetTier);
	if (!bPlayer && NewTier == static_cast<int32>(EAscensionTier::Tier3_Causal))
	{
		FCosmicEffect Ascendant;
		Ascendant.Type = EEffectType::NameNotable;
		Ascendant.Tag = AscendantTag;
		UCosmicEventSubsystem::Apply(Ascendant, Ctx);
	}
	if (NewTier >= static_cast<int32>(EAscensionTier::Tier4_Transcendent))
	{
		for (FNotableRecord& Record : State.Civs[CivIndex].Notables)
		{
			if (Record.LastTick == 0)
			{
				Record.LastTick = State.Tick;
			}
		}
	}

	// What the vessel's people received through the price is known, and the people it came from met, before any line; the
	// line, then what the price did. The received system is never the capital the line names, but its giver is the
	// ascender the line is about (a transfer never goes from a people to itself): the meeting has to come before the line,
	// or an unmet ascender's line would fall out of scope (FollowUpNotes would follow it only after the line).
	Events->FollowUpKnowing(Ctx);
	const FCivilizationState& Civ = State.Civs[CivIndex];
	const int32 Capital = Ctx.Index.FindSystem(State, Civ.HomeSystemID);
	FCosmicLine Line;
	Line.Category = AscensionCategory;
	Line.Severity = 2;
	Line.FocusSystem = State.Systems.IsValidIndex(Capital) ? State.Systems[Capital].SystemID : FName();
	Line.FocusCiv = Civ.CivID;
	Line.SourceRow = FeatRow;
	FFormatNamedArguments Args;
	Args.Add(TEXT("Civ"), Civ.CivName);
	Args.Add(TEXT("HomeSystem"), State.Systems.IsValidIndex(Capital) ? State.Systems[Capital].DisplayName : FText::GetEmpty());
	Log->PushVariants(Line, Feat.LogTemplates, Args);
	Events->FollowUpNotes(Ctx);

	if (bPlayer && Sim->IsBroadcastEnabled())
	{
		Sim->BeginBroadcast();
		OnTierAscended.Broadcast(Feat.TargetTier, FeatRow, Feat.Path);
		Sim->EndBroadcast();
	}
}
