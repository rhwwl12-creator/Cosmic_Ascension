// Cosmic Ascension - Player layer.

#include "Player/PlayerRoleSubsystem.h"

#include "Cosmic_Ascension.h"
#include "Core/CosmicIDs.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/GameInstance.h"
#include "Events/CosmicEventSubsystem.h"
#include "Events/CosmicPetitions.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Log/CosmicLogSubsystem.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicPlayerRoleImpl
{
	// The new eye of a wanderer (section 4.4): the system it stands in and its lane neighbours in the same galaxy row
	// become known, in Lanes order, without a line. Meeting their owners is the next sight stage's.
	void KnowAround(UGalaxySimulationSubsystem& Sim, FName SystemID)
	{
		const FGalaxyState& State = Sim.GetState();
		const int32 SystemIndex = Sim.GetStateIndex().FindSystem(State, SystemID);
		if (!State.Systems.IsValidIndex(SystemIndex))
		{
			return;
		}
		Sim.NoteKnown(SystemID);
		const TArray<FName> Lanes = State.Systems[SystemIndex].Lanes;
		const FName GalaxyRow = State.Systems[SystemIndex].GalaxyRow;
		for (const FName Neighbour : Lanes)
		{
			const int32 NeighbourIndex = Sim.GetStateIndex().FindSystem(State, Neighbour);
			if (State.Systems.IsValidIndex(NeighbourIndex) && State.Systems[NeighbourIndex].GalaxyRow == GalaxyRow)
			{
				Sim.NoteKnown(Neighbour);
			}
		}
	}
}

void UPlayerRoleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogCosmic, Log, TEXT("UPlayerRoleSubsystem initialized"));
}

void UPlayerRoleSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UPlayerRoleSubsystem::Install(FName SystemID, FName CivID, EPlayerRole Role)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("Install called from a handler of a Core Logic delegate: a handler may only read the state"));
		return;
	}
	// No vessel stands in the sanctuary (Law.Hedge, section 4.5 (1), 16 #81): refused before anything changes or draws.
	if (CosmicState::IsSanctuary(SystemID))
	{
		UE_LOG(LogCosmic, Error, TEXT("Install: no vessel is born in the sanctuary %s"), *SystemID.ToString());
		return;
	}
	FGalaxyState& State = Sim->GetMutableState();
	FCallingState& Calling = State.Calling;
	FCivilizationState* Civ = State.Civs.FindByPredicate([CivID](const FCivilizationState& Candidate) { return Candidate.CivID == CivID; });

	// A ruler needs a throne: without the civ the life starts as a wanderer.
	if (!ensureMsgf(Civ || Role != EPlayerRole::Ruler, TEXT("Install: no civ %s to rule"), *CivID.ToString()))
	{
		Role = EPlayerRole::Wanderer;
	}

	const bool bFirstLife = Calling.Lives.IsEmpty();
	const FName PreviousBirthCivID = bFirstLife ? FName() : Calling.Lives.Last().BirthCivID;
	const int32 PreviousNameIndex = bFirstLife ? 0 : Calling.Lives.Last().NameIndex;

	// The vessel starts from nothing: stats, pool, relics and status do not cross over (section 4.4).
	// Birth stats (section 5.9) are the struct defaults plus the birth civ's DT_Traits modifiers, once DT_Traits exists.
	State.Player = FPlayerEntityState();
	// A card put before the last life is not this life's (section 5.10).
	CosmicPetitions::Clear(State);
	State.Player.Role = Role;
	State.Player.CurrentSystemID = SystemID;

	FLifeRecord& Life = Calling.Lives.AddDefaulted_GetRef();
	Calling.LifeIndex = Calling.Lives.Num();
	Calling.BornTick = State.Tick;
	Calling.LifeState = ELifeState::Embodied;

	Life.LifeIndex = Calling.LifeIndex;
	Life.BornTick = State.Tick;
	Life.BirthSystemID = SystemID;
	Life.BirthCivID = CivID;
	Life.BirthArchetypeRow = Civ ? Civ->ArchetypeRow : NAME_None;
	Life.PeakTier = Calling.PersonalTier;
	// The return lure (Succession.Returned, which keeps the previous archetype row and name) arrives with Phase 4b.
	Life.SuccessionKind = FGameplayTag::RequestGameplayTag(FName(bFirstLife ? TEXT("Succession.Installed") : TEXT("Succession.Called")));

	// The vessel's name: an index into the person-name bank of the birth archetype, or of arch_default when that bank is
	// empty. A life born into the same civ as the previous one never repeats its name (section 7.5).
	const FCosmicDataTables& Tables = Sim->GetTables();
	const FCivArchetypeDef* Archetype = Tables.FindCiv(Life.BirthArchetypeRow);
	if (!Archetype || Archetype->PersonNameBank.IsEmpty())
	{
		Archetype = Tables.FindCiv(CosmicRows::ArchDefault);
	}
	// One draw per Install, before the bank is looked at, thrown away when the bank is empty: the number of sim draws
	// never depends on name banks, so filling one later does not move the stream (section 9.2).
	const float NameRoll = Sim->GetRng().GetFraction();
	const int32 NameCount = Archetype ? Archetype->PersonNameBank.Num() : 0;
	if (NameCount > 1 && !bFirstLife && PreviousBirthCivID == CivID)
	{
		const int32 Previous = PreviousNameIndex % NameCount;
		const int32 Drawn = FMath::Min(static_cast<int32>(NameRoll * static_cast<float>(NameCount - 1)), NameCount - 2);
		Life.NameIndex = (Drawn >= Previous) ? Drawn + 1 : Drawn;
	}
	else if (NameCount > 0)
	{
		Life.NameIndex = FMath::Min(static_cast<int32>(NameRoll * static_cast<float>(NameCount)), NameCount - 1);
	}

	if (Role == EPlayerRole::Ruler)
	{
		State.Player.RuledCivID = CivID;
		Civ->bIsPlayerRuled = true;

		// One entry in each of the three reign arrays: an empty end reason marks the open reign (section 7.2).
		Life.RuledCivIDs.Add(CivID);
		Life.ReignEndReasons.AddDefaulted();
		Life.ReignTicks.Add(0);
	}
	Sim->SyncRngState();

	// A life knows where it was born, without a line (section 8.1). The first contact is not written here.
	FKnowingState& Knowing = State.Knowing;
	if (!SystemID.IsNone() && !Knowing.KnownSystems.ContainsByPredicate([SystemID](const FKnownMark& Mark) { return Mark.ID == SystemID; }))
	{
		FKnownMark& Mark = Knowing.KnownSystems.AddDefaulted_GetRef();
		Mark.ID = SystemID;
		Mark.Tick = State.Tick;
	}
	if (Civ && !Knowing.MetCivs.ContainsByPredicate([CivID](const FKnownMark& Mark) { return Mark.ID == CivID; }))
	{
		FKnownMark& Mark = Knowing.MetCivs.AddDefaulted_GetRef();
		Mark.ID = CivID;
		Mark.Tick = State.Tick;
	}
	// A life that starts as a wanderer sees around it at once (section 4.4); a ruler's eye waits for the sight stage.
	if (Role == EPlayerRole::Wanderer)
	{
		CosmicPlayerRoleImpl::KnowAround(*Sim, SystemID);
	}

	if (bFirstLife)
	{
		// The first line of the run calls the home system by name (section 2.4).
		const FStarSystemState* Home = State.Systems.FindByPredicate([SystemID](const FStarSystemState& System) { return System.SystemID == SystemID; });
		FFormatNamedArguments Args;
		Args.Add(TEXT("HomeSystem"), Home ? Home->DisplayName : FText::GetEmpty());
		GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>()->PushRow(CosmicRows::LogFirstLife, 2, Args, SystemID);
	}
}

void UPlayerRoleSubsystem::DeposeRuler()
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Sim && Log);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("DeposeRuler called from a handler of a Core Logic delegate: a handler may only read the state"));
		return;
	}
	FGalaxyState& State = Sim->GetMutableState();
	const FPlayerEntityState& Player = State.Player;
	if (State.Calling.LifeState != ELifeState::Embodied || Player.Role != EPlayerRole::Ruler)
	{
		return;
	}
	const FName RuledCivID = Player.RuledCivID;
	const int32 CivIndex = Sim->GetStateIndex().FindCiv(State, RuledCivID);

	// The line first: the fall is told before what it changes (section 5.10).
	FFormatNamedArguments Args;
	Args.Add(TEXT("Civ"), State.Civs.IsValidIndex(CivIndex) ? State.Civs[CivIndex].CivName : FText::GetEmpty());
	Log->PushRow(CosmicRows::LogFallenMonarch, 2, Args, Player.CurrentSystemID, RuledCivID);

	// One number for the regent, whether or not a people is there to take it (section 9.2).
	const float RegentRoll = Sim->GetRng().GetFraction();
	BecomeWanderer(CosmicState::PickAIProfile(RegentRoll), FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Deposed"))));

	// The ranges are lints; the guards are comparisons, so a value that is not a number takes the safe side.
	const FCosmicRulesDef& Rules = Sim->GetRules();
	const float Decay = Rules.ReverenceDecay > 0.f ? FMath::Min(Rules.ReverenceDecay, 1.f) : 0.f;
	// Every people remembers the fallen name less, as after a death (section 4.4, 5.10): none crowns it at once.
	for (FCivilizationState& Other : State.Civs)
	{
		Other.ReverenceToPlayer = FMath::Clamp(Other.ReverenceToPlayer * Decay, 0.f, 1.f);
	}
	if (State.Civs.IsValidIndex(CivIndex))
	{
		FCivilizationState& Civ = State.Civs[CivIndex];
		const float Ceiling = Rules.PostRebellionStability > 0.f ? FMath::Min(Rules.PostRebellionStability, 1.f) : 0.f;
		Civ.Stability = FMath::Min(Civ.Stability, Ceiling);
		Civ.ThroneLockedUntilTick = State.Tick + FMath::Max(0, Rules.ThroneLockTicks);
	}
	Sim->SyncRngState();
}

void UPlayerRoleSubsystem::BecomeWanderer(FGameplayTag RegentProfile, FGameplayTag ReignEndReason)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("BecomeWanderer called from a handler of a Core Logic delegate: a handler may only read the state"));
		return;
	}
	FGalaxyState& State = Sim->GetMutableState();
	FPlayerEntityState& Player = State.Player;
	if (State.Calling.LifeState != ELifeState::Embodied || Player.Role != EPlayerRole::Ruler)
	{
		return;
	}
	// A wanderer walks where the vessel stands, and no vessel stands in the sanctuary (Law.Hedge, section 4.5 (1)): a state
	// that says otherwise is refused, not walked from.
	if (CosmicState::IsSanctuary(Player.CurrentSystemID))
	{
		UE_LOG(LogCosmic, Error, TEXT("BecomeWanderer: the vessel stands in the sanctuary %s, where no vessel stands"), *Player.CurrentSystemID.ToString());
		return;
	}
	const int32 CivIndex = Sim->GetStateIndex().FindCiv(State, Player.RuledCivID);
	if (State.Civs.IsValidIndex(CivIndex))
	{
		FCivilizationState& Civ = State.Civs[CivIndex];
		Civ.bIsPlayerRuled = false;
		if (RegentProfile.IsValid())
		{
			Civ.AIProfile = RegentProfile;
		}
	}

	Player.FormerCivID = Player.RuledCivID;
	Player.RuledCivID = NAME_None;
	Player.Role = EPlayerRole::Wanderer;

	// Why the vessel walks (section 5.10). A death leaves no status: the next vessel starts from nothing.
	if (ReignEndReason == FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Abdicated"))))
	{
		Player.StatusTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Status.Abdicated"))));
	}
	else if (ReignEndReason == FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Deposed"))))
	{
		Player.StatusTags.AddTag(FGameplayTag::RequestGameplayTag(FName(TEXT("Status.FallenMonarch"))));
	}

	// The open reign ends with its reason (section 7.2).
	if (!State.Calling.Lives.IsEmpty())
	{
		FLifeRecord& Life = State.Calling.Lives.Last();
		if (!Life.ReignEndReasons.IsEmpty() && !Life.ReignEndReasons.Last().IsValid())
		{
			Life.ReignEndReasons.Last() = ReignEndReason;
		}
	}

	// A role change ends a first-contact card that was waiting (section 8.1), and a petition card: a wanderer rules no
	// people to petition (section 5.10).
	State.Knowing.PendingContactCivID = NAME_None;
	CosmicPetitions::Clear(State);

	// The new eye of a wanderer: where it stands and the lanes around (section 4.4).
	CosmicPlayerRoleImpl::KnowAround(*Sim, Player.CurrentSystemID);
}

bool UPlayerRoleSubsystem::StartMegaproject(FName SystemID, FName RowName)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("StartMegaproject called from a handler of a Core Logic delegate: a handler may only read the state"));
		return false;
	}
	FGalaxyState& State = Sim->GetMutableState();
	if (State.Calling.LifeState != ELifeState::Embodied || State.Player.Role != EPlayerRole::Ruler || State.Player.RuledCivID.IsNone())
	{
		return false;
	}
	const int32 SystemIndex = Sim->GetStateIndex().FindSystem(State, SystemID);
	if (RowName == CosmicRows::ProjectSettle)
	{
		// The target is a wild system: the rules are those of the AI's settlement, without its draw (section 5.7). The
		// system the people starts to settle is known at once (section 8.1).
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		const bool bSettles = CosmicMegaprojects::Settle(Ctx, Sim->GetStateIndex().FindCiv(State, State.Player.RuledCivID), SystemIndex);
		if (bSettles)
		{
			GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>()->FollowUpKnowing(Ctx);
		}
		return bSettles;
	}
	if (!State.Systems.IsValidIndex(SystemIndex) || State.Systems[SystemIndex].OwnerCivID != State.Player.RuledCivID)
	{
		return false;
	}
	return CosmicMegaprojects::Start(State, Sim->GetStateIndex(), Sim->GetTables(), SystemIndex, RowName);
}

bool UPlayerRoleSubsystem::StartNextMegaproject()
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("StartNextMegaproject called from a handler of a Core Logic delegate: a handler may only read the state"));
		return false;
	}
	const FGalaxyState& State = Sim->GetState();
	if (State.Calling.LifeState != ELifeState::Embodied || State.Player.Role != EPlayerRole::Ruler)
	{
		return false;
	}
	const int32 CivIndex = Sim->GetStateIndex().FindCiv(State, State.Player.RuledCivID);
	const TArray<TPair<int32, FName>> Choices = CosmicMegaprojects::Choices(State, Sim->GetStateIndex(), Sim->GetTables(), CivIndex, State.Calling.BornTick);
	if (!Choices.IsEmpty())
	{
		return StartMegaproject(State.Systems[Choices[0].Key].SystemID, Choices[0].Value);
	}
	// Nothing to build or answer: the first target of a settlement, when there is one.
	const TArray<int32> Targets = CosmicMegaprojects::SettleTargets(State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), CivIndex);
	return !Targets.IsEmpty() && StartMegaproject(State.Systems[Targets[0]].SystemID, CosmicRows::ProjectSettle);
}

FCosmicPetitionAnswer UPlayerRoleSubsystem::AnswerPetition(int32 ChoiceIndex)
{
	FCosmicPetitionAnswer Answer;
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	UCosmicEventSubsystem* Events = GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>();
	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Sim && Events && Log);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("AnswerPetition called from a handler of a Core Logic delegate: a handler may only read the state"));
		return Answer;
	}
	FGalaxyState& State = Sim->GetMutableState();
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const FCosmicDataTables& Tables = Sim->GetTables();
	const int32 CivIndex = CosmicPetitions::RuledCivIndexOf(State, Index);
	const FName RowName = State.Knowing.PendingPetitionRow;
	if (CivIndex == INDEX_NONE || RowName.IsNone())
	{
		return Answer;
	}
	const FEncounterDef* Row = Tables.FindEncounter(RowName);
	if (!Row)
	{
		UE_LOG(LogCosmic, Error, TEXT("AnswerPetition: the waiting card %s is no row of DT_Encounters"), *RowName.ToString());
		return Answer;
	}
	if (!Row->Choices.IsValidIndex(ChoiceIndex))
	{
		UE_LOG(LogCosmic, Error, TEXT("AnswerPetition: %s has no choice %d"), *RowName.ToString(), ChoiceIndex);
		return Answer;
	}
	const FEncounterChoice& Choice = Row->Choices[ChoiceIndex];
	const FResourcePool Cost = CosmicPetitions::CostOf(Choice);
	if (!CosmicMegaprojects::CanPay(State.Civs[CivIndex].Stockpile, Cost))
	{
		UE_LOG(LogCosmic, Error, TEXT("AnswerPetition: the stockpile cannot pay choice %d of %s"), ChoiceIndex, *RowName.ToString());
		return Answer;
	}

	// One number of the sim stream for every answer taken, before the choice is read any further (section 9.2).
	FCosmicEvalContext Ctx = Sim->MakeEvalContext();
	Answer.Roll = FMath::Min(static_cast<int32>(Ctx.Stream.GetFraction() * 6.f), 5);

	if (!CosmicPetitions::StillStands(State, Index, Tables, Sim->GetRules()))
	{
		CosmicPetitions::Clear(State);
		Answer.Outcome = ECosmicPetitionOutcome::Withdrawn;
		Sim->SyncRngState();
		return Answer;
	}
	const FName SystemID = State.Knowing.PendingPetitionSystemID;
	const int32 SystemIndex = CosmicPetitions::SubjectOf(State, Index, CivIndex, Row->Subject, SystemID);
	// The arguments of the texts before the effects: the hazard the card is about, also when the answer resolves it.
	const FFormatNamedArguments Args = CosmicPetitions::ArgsOf(State, Tables, CivIndex, SystemIndex);

	FResourcePool& Stockpile = State.Civs[CivIndex].Stockpile;
	Stockpile.Energy -= Cost.Energy;
	Stockpile.Matter -= Cost.Matter;
	Stockpile.Insight -= Cost.Insight;
	Stockpile.Influence -= Cost.Influence;
	Stockpile.Causality -= Cost.Causality;

	const bool bPasses = CosmicPetitions::Passes(State, CivIndex, Choice, Answer.Roll);
	Ctx.CivIndex = CivIndex;
	Ctx.SystemIndex = SystemIndex;
	Ctx.GalaxyRow = State.Civs[CivIndex].GalaxyRow;
	Ctx.SourceRow = RowName;
	Ctx.bPerTickSource = false;
	UCosmicEventSubsystem::ApplyAll(bPasses ? Choice.OnSuccess : Choice.OnFailure, Ctx);
	Events->FollowUpKnowing(Ctx);

	// The outcome line: about the people, and about the place for a card on a system (as an event row's line).
	static const FGameplayTag CivCategory = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Civ")));
	const FText& Template = bPasses ? Choice.SuccessText : Choice.FailureText;
	if (!Template.IsEmpty())
	{
		const bool bAboutPlace = Row->Subject == ECosmicEventSubject::SettledSystem && State.Systems.IsValidIndex(SystemIndex);
		Answer.Line = FText::Format(Template, Args);
		Log->Push(CivCategory, 1, Template, Args, bAboutPlace ? State.Systems[SystemIndex].SystemID : FName(), RowName, State.Civs[CivIndex].CivID);
	}
	Events->FollowUpNotes(Ctx);

	CosmicPetitions::Remember(State, RowName);
	CosmicPetitions::Clear(State);
	// The next card of a chain, on the same subject, when its conditions hold: put by name, no draw.
	if (const FEncounterDef* Next = Choice.NextEncounter.IsNone() ? nullptr : Tables.FindEncounter(Choice.NextEncounter))
	{
		const int32 NextSystemIndex = CosmicPetitions::SubjectOf(State, Index, CivIndex, Next->Subject, SystemID);
		const bool bHasSubject = Next->Subject == ECosmicEventSubject::Civilization || NextSystemIndex != INDEX_NONE;
		if (bHasSubject && CosmicPetitions::Holds(State, Index, Tables, Sim->GetRules(), Choice.NextEncounter, *Next, CivIndex, NextSystemIndex))
		{
			State.Knowing.PendingPetitionRow = Choice.NextEncounter;
			State.Knowing.PendingPetitionSystemID = Next->Subject == ECosmicEventSubject::SettledSystem ? SystemID : FName();
		}
	}

	Answer.Outcome = bPasses ? ECosmicPetitionOutcome::Succeeded : ECosmicPetitionOutcome::Failed;
	Sim->SyncRngState();
	return Answer;
}
