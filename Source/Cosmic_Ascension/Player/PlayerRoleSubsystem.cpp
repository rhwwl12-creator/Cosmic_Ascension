// Cosmic Ascension - Player layer.

#include "Player/PlayerRoleSubsystem.h"

#include "Cosmic_Ascension.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Log/CosmicLogSubsystem.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"

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

	if (State.Civs.IsValidIndex(CivIndex))
	{
		const FCosmicRulesDef& Rules = Sim->GetRules();
		FCivilizationState& Civ = State.Civs[CivIndex];
		// The ranges are lints; the guards are comparisons, so a value that is not a number takes the safe side.
		const float Decay = Rules.ReverenceDecay > 0.f ? FMath::Min(Rules.ReverenceDecay, 1.f) : 0.f;
		Civ.ReverenceToPlayer = FMath::Clamp(Civ.ReverenceToPlayer * Decay, 0.f, 1.f);
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
	FGalaxyState& State = Sim->GetMutableState();
	FPlayerEntityState& Player = State.Player;
	if (State.Calling.LifeState != ELifeState::Embodied || Player.Role != EPlayerRole::Ruler)
	{
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

	// A role change ends a first-contact card that was waiting (section 8.1).
	State.Knowing.PendingContactCivID = NAME_None;
}
