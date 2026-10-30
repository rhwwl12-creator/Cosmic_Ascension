// Cosmic Ascension - Player layer.

#include "Player/PlayerRoleSubsystem.h"

#include "Cosmic_Ascension.h"
#include "Core/CosmicIDs.h"
#include "Core/CosmicTags.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/GameInstance.h"
#include "Events/CosmicEventSubsystem.h"
#include "Events/CosmicPetitions.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Log/CosmicLogSubsystem.h"
#include "Sim/CosmicConflict.h"
#include "Sim/CosmicDiplomacy.h"
#include "Sim/CosmicEdicts.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicResearch.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicSuccession.h"
#include "Sim/CosmicWanderer.h"
#include "Sim/GalaxySimulationSubsystem.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicPlayerRoleImpl
{
	// The thrones a vessel left: every people's ThroneLockedUntilTick to Tick + ThroneLockTicks, never earlier than it was
	// (B1 header section 2.7 - written on every people, so it holds when the people left is gone).
	void LockEveryThrone(FGalaxyState& State, const FCosmicRulesDef& Rules)
	{
		const int64 Until = State.Tick + FMath::Max(0, Rules.ThroneLockTicks);
		for (FCivilizationState& Civ : State.Civs)
		{
			Civ.ThroneLockedUntilTick = FMath::Max(Civ.ThroneLockedUntilTick, Until);
		}
	}

	// The people the vessel rules: Embodied, Ruler, a RuledCivID the state holds; INDEX_NONE otherwise.
	int32 RuledIndexOf(const UGalaxySimulationSubsystem& Sim)
	{
		const FGalaxyState& State = Sim.GetState();
		const bool bRuler = State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Ruler && !State.Player.RuledCivID.IsNone();
		const int32 CivIndex = bRuler ? Sim.GetStateIndex().FindCiv(State, State.Player.RuledCivID) : INDEX_NONE;
		return State.Civs.IsValidIndex(CivIndex) ? CivIndex : INDEX_NONE;
	}

	// Whether the calling has met the people CivID: the input definition of a diplomacy command (rule 2).
	bool IsMet(const FGalaxyState& State, FName CivID)
	{
		return State.Knowing.MetCivs.ContainsByPredicate([CivID](const FKnownMark& Met) { return Met.ID == CivID; });
	}

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
	const FName PreviousArchetypeRow = bFirstLife ? FName() : Calling.Lives.Last().BirthArchetypeRow;
	const int32 PreviousNameIndex = bFirstLife ? 0 : Calling.Lives.Last().NameIndex;

	// The vessel starts from nothing: stats, pool, relics and status do not cross over (section 4.4).
	// Birth stats (section 5.9) are the struct defaults plus the birth civ's DT_Traits modifiers, once DT_Traits exists.
	State.Player = FPlayerEntityState();
	// A card put before the last life is not this life's (section 5.10) - nor the first contact card waiting (B3).
	CosmicPetitions::Clear(State);
	State.Knowing.PendingContactCivID = NAME_None;
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
	// The return lure (Succession.Returned, which would keep the previous archetype row and name) is not B5's (16 #146).
	Life.SuccessionKind = FGameplayTag::RequestGameplayTag(FName(bFirstLife ? TEXT("Succession.Installed") : TEXT("Succession.Called")));

	// The vessel's name: an index into the person-name bank of the birth archetype, or of arch_default when that bank is
	// empty. A life never repeats the previous life's name (section 7.5; B5 decision 8): the names are compared by their
	// source strings - the bank is TArray<FText>, and the display culture must not pick another index (the world hash holds
	// it) - and the first entry that matches is passed over.
	const FCosmicDataTables& Tables = Sim->GetTables();
	auto BankOf = [&Tables](FName ArchetypeRow)
	{
		const FCivArchetypeDef* Found = Tables.FindCiv(ArchetypeRow);
		return (!Found || Found->PersonNameBank.IsEmpty()) ? Tables.FindCiv(CosmicRows::ArchDefault) : Found;
	};
	const FCivArchetypeDef* Archetype = BankOf(Life.BirthArchetypeRow);
	int32 Previous = INDEX_NONE;
	const FCivArchetypeDef* PreviousArchetype = bFirstLife ? nullptr : BankOf(PreviousArchetypeRow);
	if (Archetype && PreviousArchetype && !PreviousArchetype->PersonNameBank.IsEmpty())
	{
		const FString* PreviousName = FTextInspector::GetSourceString(PreviousArchetype->PersonNameBank[PreviousNameIndex % PreviousArchetype->PersonNameBank.Num()]);
		for (int32 At = 0; PreviousName && At < Archetype->PersonNameBank.Num() && Previous == INDEX_NONE; ++At)
		{
			const FString* Name = FTextInspector::GetSourceString(Archetype->PersonNameBank[At]);
			Previous = (Name && *Name == *PreviousName) ? At : INDEX_NONE;
		}
	}
	// One draw per Install, before the bank is looked at, thrown away when the bank is empty: the number of sim draws
	// never depends on name banks, so filling one later does not move the stream (section 9.2).
	const float NameRoll = Sim->GetRng().GetFraction();
	const int32 NameCount = Archetype ? Archetype->PersonNameBank.Num() : 0;
	if (NameCount > 1 && Previous != INDEX_NONE)
	{
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
	// Every people remembers the fallen name less (section 4.4, 5.10): none crowns it at once. A death sets it to 0 (B5 - Die).
	for (FCivilizationState& Other : State.Civs)
	{
		Other.ReverenceToPlayer = FMath::Clamp(Other.ReverenceToPlayer * Decay, 0.f, 1.f);
	}
	if (State.Civs.IsValidIndex(CivIndex))
	{
		FCivilizationState& Civ = State.Civs[CivIndex];
		const float Ceiling = Rules.PostRebellionStability > 0.f ? FMath::Min(Rules.PostRebellionStability, 1.f) : 0.f;
		Civ.Stability = FMath::Min(Civ.Stability, Ceiling);
	}
	CosmicPlayerRoleImpl::LockEveryThrone(State, Rules);
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
	if (ReleaseThrone(RegentProfile, ReignEndReason, CosmicRows::LogThroneVacated))
	{
		// The new eye of a wanderer: where it stands and the lanes around (section 4.4).
		CosmicPlayerRoleImpl::KnowAround(*Sim, Sim->GetState().Player.CurrentSystemID);
	}
}

bool UPlayerRoleSubsystem::ReleaseThrone(FGameplayTag RegentProfile, FGameplayTag ReignEndReason, FName RegentSourceRow)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	FGalaxyState& State = Sim->GetMutableState();
	FPlayerEntityState& Player = State.Player;
	if (State.Calling.LifeState != ELifeState::Embodied || Player.Role != EPlayerRole::Ruler)
	{
		return false;
	}
	// A wanderer walks where the vessel stands, and no vessel stands in the sanctuary (Law.Hedge, section 4.5 (1)): a state
	// that says otherwise is refused, not walked from.
	if (CosmicState::IsSanctuary(Player.CurrentSystemID))
	{
		UE_LOG(LogCosmic, Error, TEXT("BecomeWanderer: the vessel stands in the sanctuary %s, where no vessel stands"), *Player.CurrentSystemID.ToString());
		return false;
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
	static const FGameplayTag AbdicatedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Abdicated")));
	const FGameplayTag DiedTag = CosmicTags::ReignDied();
	if (ReignEndReason == AbdicatedTag)
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
	Player.TravelTargetSystemID = NAME_None;
	Player.TravelTicksRemaining = 0;

	// Notable hook 1 (section 7.4, 7.5): a reign left without a fall - of the vessel's own will, or by its death (B5) -
	// leaves a regent: the NameNotable rule through Apply, one number of the name before its guards, whether or not a record
	// is made (B1). A deposed vessel leaves none: the rebels' leader holds the throne.
	if (ReignEndReason == AbdicatedTag || ReignEndReason == DiedTag)
	{
		FCosmicEvalContext Ctx = Sim->MakeEvalContext();
		Ctx.CivIndex = CivIndex;
		Ctx.SystemIndex = INDEX_NONE;
		Ctx.GalaxyRow = State.Civs.IsValidIndex(CivIndex) ? State.Civs[CivIndex].GalaxyRow : FName();
		Ctx.SourceRow = RegentSourceRow;
		FCosmicEffect Regent;
		Regent.Type = EEffectType::NameNotable;
		Regent.Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Notable.Regent")));
		UCosmicEventSubsystem::Apply(Regent, Ctx);
	}
	return true;
}

void UPlayerRoleSubsystem::Die()
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("Die called from a handler of a Core Logic delegate: a handler may only read the state"));
		return;
	}
	FGalaxyState& State = Sim->GetMutableState();
	FCallingState& Calling = State.Calling;
	if (Calling.LifeState != ELifeState::Embodied || Calling.Lives.IsEmpty())
	{
		return;
	}
	// No vessel stands in the sanctuary (Install refuses it): a ruler that says otherwise could not leave its throne, and a death
	// half done would leave a ruler in the interregnum. Refused before anything changes or draws.
	if (State.Player.Role == EPlayerRole::Ruler && CosmicState::IsSanctuary(State.Player.CurrentSystemID))
	{
		UE_LOG(LogCosmic, Error, TEXT("Die: the vessel stands in the sanctuary %s, where no vessel stands"), *State.Player.CurrentSystemID.ToString());
		return;
	}
	const FCosmicRulesDef& Rules = Sim->GetRules();

	// (a) The life closes.
	FLifeRecord& Life = Calling.Lives.Last();
	Life.DiedTick = State.Tick;
	Life.CauseOfDeath = Calling.PendingCause;
	Life.CauseSourceRow = Calling.PendingSourceRow;

	// (b) The death line is told before what it changes - [B-5d1].

	// (c) A ruler leaves the throne: one number for the regent's profile, one in ReleaseThrone for the regent's name (the
	// deposition's form), whether or not a people is there; every throne is locked (16 #143 - the three ends of a reign lock
	// alike). No new eye: the dead vessel looks at nothing.
	if (State.Player.Role == EPlayerRole::Ruler)
	{
		const float RegentRoll = Sim->GetRng().GetFraction();
		ReleaseThrone(CosmicState::PickAIProfile(RegentRoll), CosmicTags::ReignDied(), Calling.PendingSourceRow);
		CosmicPlayerRoleImpl::LockEveryThrone(State, Rules);
	}

	// (d) No people reveres the next vessel for this one's sake (decision 23-na), before the succession draws.
	for (FCivilizationState& Civ : State.Civs)
	{
		Civ.ReverenceToPlayer = 0.f;
	}

	// (e) Nothing waits for a vessel that is gone.
	CosmicPetitions::Clear(State);
	State.Knowing.PendingContactCivID = NAME_None;
	State.Player.TravelTargetSystemID = NAME_None;
	State.Player.TravelTicksRemaining = 0;

	// (f) The interregnum and its leap: one number, whatever the knobs hold (decision 3).
	Calling.LifeState = ELifeState::Interregnum;
	const int64 Leap = CosmicSuccession::LeapOf(Rules, Calling.LeapTicksTotal, Sim->GetRng().GetFraction());
	Calling.LeapTicksTotal += Leap;
	Calling.PendingLeapTicks = Leap;
	Sim->SyncRngState();

	// (g) A leap of 0 (the cap reached) succeeds on the tick of the death.
	if (Leap == 0)
	{
		Succeed();
	}
}

void UPlayerRoleSubsystem::Succeed()
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("Succeed called from a handler of a Core Logic delegate: a handler may only read the state"));
		return;
	}
	FGalaxyState& State = Sim->GetMutableState();
	FCallingState& Calling = State.Calling;
	if (Calling.LifeState != ELifeState::Interregnum)
	{
		return;
	}
	// One number for the succession before the candidates are looked at (decision 8).
	const float SuccessionRoll = Sim->GetRng().GetFraction();
	const int32 DeathSystemIndex = Sim->GetStateIndex().FindSystem(State, State.Player.CurrentSystemID);
	const CosmicSuccession::FCosmicSuccessionCandidates Found = CosmicSuccession::CandidatesOf(State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), DeathSystemIndex);
	const int32 Pick = CosmicSuccession::PickOf(SuccessionRoll, Found.Systems.Num());
	// Install refuses only the sanctuary, which no candidate is (it has no owner): a state that says otherwise is the silence,
	// with one error line, before the calling is touched.
	const bool bRefused = Pick != INDEX_NONE && CosmicState::IsSanctuary(State.Systems[Found.Systems[Pick]].SystemID);
	if (bRefused)
	{
		UE_LOG(LogCosmic, Error, TEXT("Succeed: the candidate %s is the sanctuary, where no vessel is born"), *State.Systems[Found.Systems[Pick]].SystemID.ToString());
	}
	if (Pick == INDEX_NONE || bRefused)
	{
		// The silence: the name's number is drawn and thrown away (two numbers at every succession - rule 3), and the calling
		// stays in the interregnum with no leap left. It is never drawn again.
		Sim->GetRng().GetFraction();
		Calling.PendingLeapTicks = 0;
		Sim->SyncRngState();
		return;
	}
	// The calling keeps what is its own: a tier below the third goes back to the first (Install reads it into PeakTier),
	// and of its powers the way it took (Way.*) - the rest is given again from the tier (no row gives one yet).
	if (Calling.PersonalTier < EAscensionTier::Tier3_Causal)
	{
		Calling.PersonalTier = EAscensionTier::Tier1_Planetary;
	}
	FGameplayTagContainer Kept;
	for (const FGameplayTag& Power : Calling.Powers)
	{
		if (Power.GetTagName().ToString().StartsWith(TEXT("Way.")))
		{
			Kept.AddTag(Power);
		}
	}
	Calling.Powers = Kept;
	const FStarSystemState& Birth = State.Systems[Found.Systems[Pick]];
	Install(Birth.SystemID, Birth.OwnerCivID, EPlayerRole::Wanderer);
	// The inauguration line - [B-5d1].
	Sim->SyncRngState();
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
	const FName RowName = State.Knowing.PendingPetitionRow;
	if (RowName.IsNone())
	{
		return Answer;
	}
	const FEncounterDef* Row = Tables.FindEncounter(RowName);
	if (!Row)
	{
		UE_LOG(LogCosmic, Error, TEXT("AnswerPetition: the waiting card %s is no row of DT_Encounters"), *RowName.ToString());
		return Answer;
	}
	// The first contact card is about the people met (B3 header section 2.9): its answer's other people, cleared with it.
	const bool bFirstContact = RowName == CosmicRows::PetitionFirstContact;
	const FName OtherCivID = bFirstContact ? State.Knowing.PendingContactCivID : NAME_None;
	// Who answers: the ruler a petition, the wanderer its own card (B1). The other role is refused - nothing drawn.
	const bool bWandererCard = CosmicPetitions::IsWandererCard(*Row);
	const int32 RuledIndex = CosmicPetitions::RuledCivIndexOf(State, Index);
	if (bWandererCard ? !CosmicWanderer::IsEmbodiedWanderer(State) : RuledIndex == INDEX_NONE)
	{
		return Answer;
	}
	if (!Row->Choices.IsValidIndex(ChoiceIndex))
	{
		UE_LOG(LogCosmic, Error, TEXT("AnswerPetition: %s has no choice %d"), *RowName.ToString(), ChoiceIndex);
		return Answer;
	}
	const FEncounterChoice& Choice = Row->Choices[ChoiceIndex];
	const FResourcePool Cost = bWandererCard ? FResourcePool() : CosmicPetitions::CostOf(Choice);
	if (!bWandererCard && !CosmicMegaprojects::CanPay(State.Civs[RuledIndex].Stockpile, Cost))
	{
		UE_LOG(LogCosmic, Error, TEXT("AnswerPetition: the stockpile cannot pay choice %d of %s"), ChoiceIndex, *RowName.ToString());
		return Answer;
	}
	// A choice its gate would refuse (B3: a stance the friendly slots refuse; B4: an edict, the fleet's time - B4 header
	// section 2.4) is refused as an unpaid cost, before the draw - the card greys it. A card that no longer stands (its
	// people gone, its subject lost) is withdrawn below instead, with its draw.
	if (!bWandererCard && CosmicPetitions::StillStands(State, Index, Tables, Sim->GetRules()))
	{
		const int32 GateSystem = CosmicPetitions::SubjectOf(State, Index, RuledIndex, Row->Subject, State.Knowing.PendingPetitionSystemID);
		if (!CosmicPetitions::ChoiceGateOf(State, Index, Tables, Sim->GetRules(), RuledIndex, GateSystem, OtherCivID, Choice).IsOpen())
		{
			UE_LOG(LogCosmic, Error, TEXT("AnswerPetition: the choice's gate refuses choice %d of %s"), ChoiceIndex, *RowName.ToString());
			return Answer;
		}
	}

	// One number of the sim stream for every answer taken, before the choice is read any further (section 9.2).
	FCosmicEvalContext Ctx = Sim->MakeEvalContext();
	Answer.Roll = FMath::Min(static_cast<int32>(Ctx.Stream.GetFraction() * 6.f), 5);

	if (!CosmicPetitions::StillStands(State, Index, Tables, Sim->GetRules()))
	{
		CosmicPetitions::Clear(State);
		if (bFirstContact)
		{
			State.Knowing.PendingContactCivID = NAME_None;
		}
		Answer.Outcome = ECosmicPetitionOutcome::Withdrawn;
		Sim->SyncRngState();
		return Answer;
	}
	const FName SystemID = State.Knowing.PendingPetitionSystemID;
	// The subject: the ruled people and the card's system; for a wanderer's card the system the vessel stands in and its
	// owner (none for a wild system).
	const FCosmicVesselSubject Vessel = bWandererCard ? CosmicPetitions::VesselSubjectOf(State, Index) : FCosmicVesselSubject();
	const int32 CivIndex = bWandererCard ? Vessel.CivIndex : RuledIndex;
	const int32 SystemIndex = bWandererCard ? Vessel.SystemIndex : CosmicPetitions::SubjectOf(State, Index, CivIndex, Row->Subject, SystemID);
	// The arguments of the texts before the effects: the hazard the card is about, also when the answer resolves it.
	const FFormatNamedArguments Args = CosmicPetitions::ArgsOf(State, Tables, CivIndex, SystemIndex, OtherCivID, bFirstContact);

	if (!bWandererCard)
	{
		FResourcePool& Stockpile = State.Civs[CivIndex].Stockpile;
		Stockpile.Energy -= Cost.Energy;
		Stockpile.Matter -= Cost.Matter;
		Stockpile.Insight -= Cost.Insight;
		Stockpile.Influence -= Cost.Influence;
		Stockpile.Causality -= Cost.Causality;
	}

	const bool bPasses = CosmicPetitions::Passes(State, CivIndex, Choice, Answer.Roll);
	Ctx.CivIndex = CivIndex;
	Ctx.SystemIndex = SystemIndex;
	Ctx.GalaxyRow = State.Civs.IsValidIndex(CivIndex) ? State.Civs[CivIndex].GalaxyRow
		: State.Systems.IsValidIndex(SystemIndex) ? State.Systems[SystemIndex].GalaxyRow : FName();
	Ctx.SourceRow = RowName;
	Ctx.bPerTickSource = false;
	// Only the first contact card names another people: every other card's effects keep no other party.
	Ctx.OtherID = OtherCivID;
	UCosmicEventSubsystem::ApplyAll(bPasses ? Choice.OnSuccess : Choice.OnFailure, Ctx);
	Events->FollowUpKnowing(Ctx);

	// The outcome line: about the people, and about the place for a card on a system (as an event row's line).
	static const FGameplayTag CivCategory = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Civ")));
	const FText& Template = bPasses ? Choice.SuccessText : Choice.FailureText;
	if (!Template.IsEmpty())
	{
		const bool bAboutPlace = CosmicPetitions::IsAboutPlace(*Row) && State.Systems.IsValidIndex(SystemIndex);
		Answer.Line = FText::Format(Template, Args);
		Log->Push(CivCategory, 1, Template, Args, bAboutPlace ? State.Systems[SystemIndex].SystemID : FName(), RowName,
			State.Civs.IsValidIndex(CivIndex) ? State.Civs[CivIndex].CivID : FName());
	}
	Events->FollowUpNotes(Ctx);

	CosmicPetitions::Remember(State, RowName);
	CosmicPetitions::Clear(State);
	if (bFirstContact)
	{
		State.Knowing.PendingContactCivID = NAME_None;
	}
	// The next card of a chain, on the same subject, when its conditions hold: put by name, no draw.
	// A chain stays with its role: a petition's next card is a petition on the same subject, a wanderer's next card is
	// a wanderer's card where the vessel stands.
	if (const FEncounterDef* Next = Choice.NextEncounter.IsNone() ? nullptr : Tables.FindEncounter(Choice.NextEncounter))
	{
		if (CosmicPetitions::IsWandererCard(*Next) == bWandererCard)
		{
			const FCosmicVesselSubject NextVessel = bWandererCard ? CosmicPetitions::VesselSubjectOf(State, Index) : FCosmicVesselSubject();
			const int32 NextCivIndex = bWandererCard ? NextVessel.CivIndex : CivIndex;
			const int32 NextSystemIndex = bWandererCard ? NextVessel.SystemIndex : CosmicPetitions::SubjectOf(State, Index, CivIndex, Next->Subject, SystemID);
			const bool bHasSubject = Next->Subject == ECosmicEventSubject::Civilization || NextSystemIndex != INDEX_NONE;
			if (bHasSubject && CosmicPetitions::Holds(State, Index, Tables, Sim->GetRules(), Choice.NextEncounter, *Next, NextCivIndex, NextSystemIndex))
			{
				State.Knowing.PendingPetitionRow = Choice.NextEncounter;
				State.Knowing.PendingPetitionSystemID = CosmicPetitions::IsAboutPlace(*Next) && State.Systems.IsValidIndex(NextSystemIndex)
					? State.Systems[NextSystemIndex].SystemID : FName();
			}
		}
	}

	Answer.Outcome = bPasses ? ECosmicPetitionOutcome::Succeeded : ECosmicPetitionOutcome::Failed;
	Sim->SyncRngState();
	return Answer;
}

bool UPlayerRoleSubsystem::SetResearchFocus(FName Field)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("SetResearchFocus called from a handler of a Core Logic delegate: a handler may only read the state"));
		return false;
	}
	FGalaxyState& State = Sim->GetMutableState();
	if (State.Calling.LifeState != ELifeState::Embodied || State.Player.Role != EPlayerRole::Ruler || State.Player.RuledCivID.IsNone())
	{
		return false;
	}
	const int32 CivIndex = Sim->GetStateIndex().FindCiv(State, State.Player.RuledCivID);
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return false;
	}
	TMap<FName, float>& ResearchFocus = State.Civs[CivIndex].ResearchFocus;
	const TMap<FName, float> Before = ResearchFocus;
	if (Field.IsNone())
	{
		CosmicResearch::Even(ResearchFocus);
	}
	else
	{
		if (!ResearchFocus.Contains(Field))
		{
			return false;
		}
		// A share at or below the even one does not lean (1/4 for the four fields), and one above 1 is no share: a data
		// fault the schema keeps out (0.3..1).
		const float Share = Sim->GetRules().ResearchFocusShare;
		if (!(Share > 1.f / static_cast<float>(ResearchFocus.Num()) && Share <= 1.f))
		{
			UE_LOG(LogCosmic, Error, TEXT("SetResearchFocus: DT_Rules.ResearchFocusShare %f cannot focus a field (above the even share, at most 1)"), Share);
			return false;
		}
		CosmicResearch::Focus(ResearchFocus, Field, Share);
	}
	// The focus asked is already the people's: nothing changed, and it is not a decision.
	return !ResearchFocus.OrderIndependentCompareEqual(Before);
}

bool UPlayerRoleSubsystem::SetResearchShares(const TMap<FName, int32>& Weights)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("SetResearchShares called from a handler of a Core Logic delegate: a handler may only read the state"));
		return false;
	}
	FGalaxyState& State = Sim->GetMutableState();
	if (State.Calling.LifeState != ELifeState::Embodied || State.Player.Role != EPlayerRole::Ruler || State.Player.RuledCivID.IsNone())
	{
		return false;
	}
	const int32 CivIndex = Sim->GetStateIndex().FindCiv(State, State.Player.RuledCivID);
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return false;
	}
	TMap<FName, float>& ResearchFocus = State.Civs[CivIndex].ResearchFocus;
	const TMap<FName, float> Before = ResearchFocus;
	if (!CosmicResearch::SharesOf(ResearchFocus, Weights))
	{
		return false;
	}
	return !ResearchFocus.OrderIndependentCompareEqual(Before);
}

CosmicEdicts::ECosmicEdictBlock UPlayerRoleSubsystem::EnactEdict(FName Row)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Sim && Log);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("EnactEdict called from a handler of a Core Logic delegate: a handler may only read the state"));
		return CosmicEdicts::ECosmicEdictBlock::Handler;
	}
	FGalaxyState& State = Sim->GetMutableState();
	const int32 CivIndex = (State.Calling.LifeState == ELifeState::Embodied && State.Player.Role == EPlayerRole::Ruler && !State.Player.RuledCivID.IsNone())
		? Sim->GetStateIndex().FindCiv(State, State.Player.RuledCivID) : INDEX_NONE;
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return CosmicEdicts::ECosmicEdictBlock::NotRuler;
	}
	const CosmicEdicts::ECosmicEdictBlock Block = CosmicEdicts::EnactBlockOf(State, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), CivIndex, Row);
	if (Block != CosmicEdicts::ECosmicEdictBlock::None)
	{
		return Block;
	}
	FCivilizationState& Civ = State.Civs[CivIndex];
	const FEdictDef* Def = Sim->GetTables().FindEdict(Row);
	check(Def);
	CosmicEdicts::Enact(Civ, Row, *Def);
	FFormatNamedArguments Args;
	Args.Add(TEXT("Civ"), Civ.CivName);
	Args.Add(TEXT("Edict"), Def->DisplayName);
	Log->PushRow(CosmicRows::LogEdictEnacted, 1, Args, NAME_None, Civ.CivID);
	return CosmicEdicts::ECosmicEdictBlock::None;
}

bool UPlayerRoleSubsystem::RevokeEdict(FName Row)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Sim && Log);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("RevokeEdict called from a handler of a Core Logic delegate: a handler may only read the state"));
		return false;
	}
	FGalaxyState& State = Sim->GetMutableState();
	if (State.Calling.LifeState != ELifeState::Embodied || State.Player.Role != EPlayerRole::Ruler || State.Player.RuledCivID.IsNone())
	{
		return false;
	}
	const int32 CivIndex = Sim->GetStateIndex().FindCiv(State, State.Player.RuledCivID);
	if (!State.Civs.IsValidIndex(CivIndex))
	{
		return false;
	}
	FCivilizationState& Civ = State.Civs[CivIndex];
	if (!CosmicEdicts::Revoke(Civ, Row))
	{
		return false;
	}
	// A name the table no longer holds leaves without a line (it has no name to tell).
	if (const FEdictDef* Def = Sim->GetTables().FindEdict(Row))
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("Civ"), Civ.CivName);
		Args.Add(TEXT("Edict"), Def->DisplayName);
		Log->PushRow(CosmicRows::LogEdictRevoked, 1, Args, NAME_None, Civ.CivID);
	}
	return true;
}

CosmicDiplomacy::ECosmicStanceBlock UPlayerRoleSubsystem::SetStance(FName CivID, FGameplayTag Stance)
{
	using CosmicDiplomacy::ECosmicStanceBlock;
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Sim && Log);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("SetStance called from a handler of a Core Logic delegate: a handler may only read the state"));
		return ECosmicStanceBlock::Handler;
	}
	FGalaxyState& State = Sim->GetMutableState();
	const int32 CivIndex = CosmicPlayerRoleImpl::RuledIndexOf(*Sim);
	if (CivIndex == INDEX_NONE)
	{
		return ECosmicStanceBlock::NotRuler;
	}
	const ECosmicStanceBlock Block = CosmicDiplomacy::StanceBlockOf(State, Sim->GetStateIndex(), Sim->GetRules(), CivIndex, CivID, Stance,
		CosmicPlayerRoleImpl::IsMet(State, CivID));
	if (Block != ECosmicStanceBlock::None)
	{
		return Block;
	}
	const int32 Other = Sim->GetStateIndex().FindCiv(State, CivID);
	bool bBrokeTrade = false;
	if (Stance == CosmicTags::StanceHostile())
	{
		bBrokeTrade = CosmicDiplomacy::TurnHostile(State, CivIndex, Other, Sim->GetRules().BetrayalRelationPenalty);
	}
	else
	{
		CosmicDiplomacy::SetStanceOf(State.Civs[CivIndex], CivID, Stance);
	}
	FFormatNamedArguments Args;
	Args.Add(TEXT("Civ"), State.Civs[CivIndex].CivName);
	Args.Add(TEXT("OtherCiv"), State.Civs[Other].CivName);
	FFormatNamedArguments StanceArgs = Args;
	StanceArgs.Add(TEXT("Stance"), CosmicDiplomacy::StanceNameOf(Stance));
	Log->PushRow(CosmicRows::LogStanceChanged, 1, StanceArgs, NAME_None, State.Civs[CivIndex].CivID);
	if (bBrokeTrade)
	{
		Args.Add(TEXT("Treaty"), CosmicDiplomacy::TreatyNameOf(CosmicTags::TreatyTrade()));
		Log->PushRow(CosmicRows::LogTreatyBroken, 1, Args, NAME_None, State.Civs[CivIndex].CivID);
	}
	return ECosmicStanceBlock::None;
}

CosmicDiplomacy::ECosmicTreatyBlock UPlayerRoleSubsystem::ProposeTreaty(FName CivID, FGameplayTag Treaty)
{
	using CosmicDiplomacy::ECosmicTreatyBlock;
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Sim && Log);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("ProposeTreaty called from a handler of a Core Logic delegate: a handler may only read the state"));
		return ECosmicTreatyBlock::Handler;
	}
	FGalaxyState& State = Sim->GetMutableState();
	const int32 CivIndex = CosmicPlayerRoleImpl::RuledIndexOf(*Sim);
	if (CivIndex == INDEX_NONE)
	{
		return ECosmicTreatyBlock::NotRuler;
	}
	const FCosmicRulesDef& Rules = Sim->GetRules();
	const ECosmicTreatyBlock Block = CosmicDiplomacy::TreatyBlockOf(State, Sim->GetStateIndex(), Sim->GetTables(), Rules, CivIndex, CivID, Treaty,
		CosmicPlayerRoleImpl::IsMet(State, CivID));
	if (Block != ECosmicTreatyBlock::None)
	{
		return Block;
	}
	FResourcePool Cost;
	Cost.Influence = Rules.TreatyCostInfluence;
	CosmicEdicts::TryPay(State.Civs[CivIndex].Stockpile, Cost);
	const int32 Other = Sim->GetStateIndex().FindCiv(State, CivID);
	CosmicDiplomacy::Sign(State, CivIndex, Other, Treaty);
	FFormatNamedArguments Args;
	Args.Add(TEXT("Civ"), State.Civs[CivIndex].CivName);
	Args.Add(TEXT("OtherCiv"), State.Civs[Other].CivName);
	Args.Add(TEXT("Treaty"), CosmicDiplomacy::TreatyNameOf(Treaty));
	Log->PushRow(CosmicRows::LogTreatySigned, 1, Args, NAME_None, State.Civs[CivIndex].CivID);
	return ECosmicTreatyBlock::None;
}

bool UPlayerRoleSubsystem::BreakTreaty(FName CivID, FGameplayTag Treaty)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Sim && Log);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("BreakTreaty called from a handler of a Core Logic delegate: a handler may only read the state"));
		return false;
	}
	FGalaxyState& State = Sim->GetMutableState();
	const int32 CivIndex = CosmicPlayerRoleImpl::RuledIndexOf(*Sim);
	const int32 Other = Sim->GetStateIndex().FindCiv(State, CivID);
	if (CivIndex == INDEX_NONE || !State.Civs.IsValidIndex(Other) || Other == CivIndex || !CosmicDiplomacy::HasTreaty(State.Civs[CivIndex], CivID, Treaty))
	{
		return false;
	}
	CosmicDiplomacy::Break(State, CivIndex, Other, Treaty);
	CosmicDiplomacy::LowerRelation(State.Civs[Other], State.Civs[CivIndex].CivID, Sim->GetRules().BetrayalRelationPenalty);
	FFormatNamedArguments Args;
	Args.Add(TEXT("Civ"), State.Civs[CivIndex].CivName);
	Args.Add(TEXT("OtherCiv"), State.Civs[Other].CivName);
	Args.Add(TEXT("Treaty"), CosmicDiplomacy::TreatyNameOf(Treaty));
	Log->PushRow(CosmicRows::LogTreatyBroken, 1, Args, NAME_None, State.Civs[CivIndex].CivID);
	return true;
}

CosmicConflict::ECosmicSiegeBlock UPlayerRoleSubsystem::AllocateFleet(FName SystemID)
{
	using CosmicConflict::ECosmicSiegeBlock;
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Sim && Log);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("AllocateFleet called from a handler of a Core Logic delegate: a handler may only read the state"));
		return ECosmicSiegeBlock::Handler;
	}
	FGalaxyState& State = Sim->GetMutableState();
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	const int32 CivIndex = CosmicPlayerRoleImpl::RuledIndexOf(*Sim);
	if (CivIndex == INDEX_NONE)
	{
		return ECosmicSiegeBlock::NotRuler;
	}
	FCivilizationState& Civ = State.Civs[CivIndex];
	if (SystemID.IsNone())
	{
		if (!Civ.FleetTargetSystemID.IsNone())
		{
			Civ.FleetTargetSystemID = NAME_None;
			Civ.FleetTargetSinceTick = State.Tick;
		}
		return ECosmicSiegeBlock::None;
	}
	const int32 SystemIndex = Index.FindSystem(State, SystemID);
	const bool bOwn = State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == Civ.CivID;
	ECosmicSiegeBlock Block = ECosmicSiegeBlock::None;
	if (bOwn)
	{
		// A guard's reasons, in the siege's names (NotOwn cannot come: the system is the people's).
		Block = CosmicConflict::SiegeNameOf(CosmicConflict::GuardBlockOf(State, Index, Sim->GetRules(), CivIndex, SystemIndex));
	}
	else
	{
		Block = CosmicConflict::SiegeBlockOf(State, Index, Sim->GetRules(), CivIndex, SystemIndex);
	}
	if (Block != ECosmicSiegeBlock::None || Civ.FleetTargetSystemID == SystemID)
	{
		return Block;
	}
	Civ.FleetTargetSystemID = SystemID;
	Civ.FleetTargetSinceTick = State.Tick;
	if (!bOwn)
	{
		const int32 Owner = Index.FindCiv(State, State.Systems[SystemIndex].OwnerCivID);
		FFormatNamedArguments Args;
		Args.Add(TEXT("Civ"), Civ.CivName);
		Args.Add(TEXT("OtherCiv"), State.Civs[Owner].CivName);
		Args.Add(TEXT("System"), State.Systems[SystemIndex].DisplayName);
		Log->PushRow(CosmicRows::LogSiegeBegun, 1, Args, SystemID, Civ.CivID);
	}
	return ECosmicSiegeBlock::None;
}

CosmicWanderer::ETravelBlock UPlayerRoleSubsystem::TravelTo(FName SystemID)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("TravelTo called from a handler of a Core Logic delegate: a handler may only read the state"));
		return CosmicWanderer::ETravelBlock::Handler;
	}
	FGalaxyState& State = Sim->GetMutableState();
	const CosmicWanderer::ETravelBlock Block = CosmicWanderer::TravelBlockOf(State, Sim->GetStateIndex(), SystemID, !State.Knowing.PendingPetitionRow.IsNone());
	if (Block != CosmicWanderer::ETravelBlock::None)
	{
		return Block;
	}
	State.Player.TravelTargetSystemID = SystemID;
	State.Player.TravelTicksRemaining = CosmicWanderer::LaneTicksOf(Sim->GetRules());
	return Block;
}

int32 UPlayerRoleSubsystem::ScanCurrentSystem()
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Sim && Log);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("ScanCurrentSystem called from a handler of a Core Logic delegate: a handler may only read the state"));
		return INDEX_NONE;
	}
	const FGalaxyState& State = Sim->GetState();
	const int32 Standing = CosmicWanderer::StandingIndexOf(State, Sim->GetStateIndex());
	if (Standing == INDEX_NONE)
	{
		return INDEX_NONE;
	}
	// The wanderer's own reach widened by the scan's lanes (section 5.9): the knobs below their floor read as the floor.
	const FCosmicRulesDef& Rules = Sim->GetRules();
	const int32 TierIndex = static_cast<int32>(State.Calling.PersonalTier);
	const int32 Sight = (Rules.SightLanesByTier.IsValidIndex(TierIndex) && Rules.SightLanesByTier[TierIndex] > 1) ? Rules.SightLanesByTier[TierIndex] : 1;
	const int32 Learned = Sim->LookFrom(MakeArrayView(&Standing, 1), Sight + FMath::Max(0, Rules.ScanSightBonusLanes), true);
	// A scan of a system no people owns that showed something new is told (section 5.9: log_scan_wild).
	const FStarSystemState& Here = State.Systems[Standing];
	if (Learned > 0 && Here.OwnerCivID.IsNone())
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("System"), Here.DisplayName);
		Log->PushRow(CosmicRows::LogScanWild, 1, Args, Here.SystemID);
	}
	return Learned;
}

CosmicWanderer::ECrownBlock UPlayerRoleSubsystem::BecomeRuler(FName CivID)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Sim && Log);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("BecomeRuler called from a handler of a Core Logic delegate: a handler may only read the state"));
		return CosmicWanderer::ECrownBlock::Handler;
	}
	FGalaxyState& State = Sim->GetMutableState();
	const FCosmicStateIndex& Index = Sim->GetStateIndex();
	// Only a met people crowns the vessel: what the player knows is the input's definition (section 5.10, rule 2).
	const bool bMet = State.Knowing.MetCivs.ContainsByPredicate([CivID](const FKnownMark& Met) { return Met.ID == CivID; });
	const CosmicWanderer::ECrownBlock Block = CosmicWanderer::CrownBlockOf(State, Index, Sim->GetRules(), CivID, bMet, !State.Knowing.PendingPetitionRow.IsNone());
	if (Block != CosmicWanderer::ECrownBlock::None)
	{
		return Block;
	}
	const bool bReturn = CosmicWanderer::IsReturn(State, CivID);
	const int32 CivIndex = Index.FindCiv(State, CivID);
	FPlayerEntityState& Player = State.Player;

	// The throne (section 5.10): the people is the vessel's, and a new reign opens in the three arrays together.
	State.Civs[CivIndex].bIsPlayerRuled = true;
	Player.Role = EPlayerRole::Ruler;
	Player.RuledCivID = CivID;
	if (!State.Calling.Lives.IsEmpty())
	{
		FLifeRecord& Life = State.Calling.Lives.Last();
		Life.RuledCivIDs.Add(CivID);
		Life.ReignEndReasons.Add(FGameplayTag());
		Life.ReignTicks.Add(0);
	}
	// Whoever held the throne leaves it: the rebels' leader that took it, the regent the vessel left (B1).
	static const FGameplayTag RulerRole = FGameplayTag::RequestGameplayTag(FName(TEXT("Notable.Ruler")));
	static const FGameplayTag RegentRole = FGameplayTag::RequestGameplayTag(FName(TEXT("Notable.Regent")));
	for (FNotableRecord& Record : State.Civs[CivIndex].Notables)
	{
		if (Record.LastTick == 0 && (Record.Role == RulerRole || Record.Role == RegentRole))
		{
			Record.LastTick = FMath::Max<int64>(State.Tick, 1);
		}
	}
	Player.TravelTargetSystemID = NAME_None;
	Player.TravelTicksRemaining = 0;
	CosmicPetitions::Clear(State);
	State.Knowing.PendingContactCivID = NAME_None;
	// B3 section 2.8: the people's treaties stay, but no siege on a system the new ruler may not know and no hostility it
	// could not change - a target on another people's system is lifted and every hostile stance turns neutral.
	if (CosmicConflict::IsBesieging(State, Index, CivIndex))
	{
		State.Civs[CivIndex].FleetTargetSystemID = NAME_None;
		State.Civs[CivIndex].FleetTargetSinceTick = State.Tick;
	}
	TArray<FName, TInlineAllocator<4>> Hostilities;
	for (const TPair<FName, FCosmicTie>& Pair : State.Civs[CivIndex].Ties)
	{
		if (Pair.Value.Stance == CosmicTags::StanceHostile())
		{
			Hostilities.Add(Pair.Key);
		}
	}
	for (const FName Other : Hostilities)
	{
		CosmicDiplomacy::SetStanceOf(State.Civs[CivIndex], Other, FGameplayTag());
	}

	// The new eye of a ruler: every system of the people is known at once (a role change writes knowing in its own call,
	// section 4.4). No line: the people was met before it crowned the vessel.
	const TArray<FName> Owned = State.Civs[CivIndex].OwnedSystems;
	for (const FName SystemID : Owned)
	{
		const int32 SystemIndex = Index.FindSystem(State, SystemID);
		if (State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == CivID)
		{
			Sim->NoteKnown(SystemID);
		}
	}

	FFormatNamedArguments Args;
	Args.Add(TEXT("Civ"), State.Civs[CivIndex].CivName);
	const int32 Here = Index.FindSystem(State, Player.CurrentSystemID);
	Args.Add(TEXT("System"), State.Systems.IsValidIndex(Here) ? State.Systems[Here].DisplayName : FText::GetEmpty());
	Log->PushRow(bReturn ? CosmicRows::LogReturned : CosmicRows::LogCrowned, 2, Args, Player.CurrentSystemID, CivID);
	return Block;
}

bool UPlayerRoleSubsystem::Abdicate()
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("Abdicate called from a handler of a Core Logic delegate: a handler may only read the state"));
		return false;
	}
	FGalaxyState& State = Sim->GetMutableState();
	const int32 CivIndex = CosmicPetitions::RuledCivIndexOf(State, Sim->GetStateIndex());
	if (CivIndex == INDEX_NONE || State.Calling.PendingEffect.IsValid())
	{
		return false;
	}
	// Recorded like every role effect (rule 13): the tenth stage of the next tick runs the pipeline. No draw here.
	FCosmicEvalContext Ctx = Sim->MakeEvalContext();
	Ctx.CivIndex = CivIndex;
	Ctx.SystemIndex = INDEX_NONE;
	Ctx.GalaxyRow = State.Civs[CivIndex].GalaxyRow;
	Ctx.SourceRow = CosmicRows::LogThroneVacated;
	FCosmicEffect Leave;
	Leave.Type = EEffectType::DeposeRuler;
	Leave.Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Abdicated")));
	return UCosmicEventSubsystem::Apply(Leave, Ctx);
}

void UPlayerRoleSubsystem::AbdicateRuler()
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	UCosmicLogSubsystem* Log = GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>();
	check(Sim && Log);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("AbdicateRuler called from a handler of a Core Logic delegate: a handler may only read the state"));
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

	// The line first, as for a fall (section 5.10).
	FFormatNamedArguments Args;
	Args.Add(TEXT("Civ"), State.Civs.IsValidIndex(CivIndex) ? State.Civs[CivIndex].CivName : FText::GetEmpty());
	const int32 Here = Sim->GetStateIndex().FindSystem(State, Player.CurrentSystemID);
	Args.Add(TEXT("System"), State.Systems.IsValidIndex(Here) ? State.Systems[Here].DisplayName : FText::GetEmpty());
	Log->PushRow(CosmicRows::LogThroneVacated, 2, Args, Player.CurrentSystemID, RuledCivID);

	// One number for the regent's profile, one in BecomeWanderer for the regent's name: two, always (section 9.2).
	const float RegentRoll = Sim->GetRng().GetFraction();
	BecomeWanderer(CosmicState::PickAIProfile(RegentRoll), FGameplayTag::RequestGameplayTag(FName(TEXT("Reign.Abdicated"))));

	// The people left keeps its reverence (a departure, not a fall) and loses a little of its calm; no throne takes the
	// vessel back for a while.
	const FCosmicRulesDef& Rules = Sim->GetRules();
	if (State.Civs.IsValidIndex(CivIndex))
	{
		const float Loss = Rules.AbdicationStabilityLoss > 0.f ? FMath::Min(Rules.AbdicationStabilityLoss, 1.f) : 0.f;
		FCivilizationState& Civ = State.Civs[CivIndex];
		Civ.Stability = FMath::Max(0.f, Civ.Stability - Loss);
	}
	CosmicPlayerRoleImpl::LockEveryThrone(State, Rules);
	Sim->SyncRngState();
}
