// Cosmic Ascension - Player layer.

#include "Player/PlayerRoleSubsystem.h"

#include "Cosmic_Ascension.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicRowNames.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Log/CosmicLogSubsystem.h"
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

	if (bFirstLife)
	{
		// The first line of the run calls the home system by name (section 2.4).
		const FStarSystemState* Home = State.Systems.FindByPredicate([SystemID](const FStarSystemState& System) { return System.SystemID == SystemID; });
		FFormatNamedArguments Args;
		Args.Add(TEXT("HomeSystem"), Home ? Home->DisplayName : FText::GetEmpty());
		GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>()->PushRow(CosmicRows::LogFirstLife, 2, Args, SystemID);
	}
}
