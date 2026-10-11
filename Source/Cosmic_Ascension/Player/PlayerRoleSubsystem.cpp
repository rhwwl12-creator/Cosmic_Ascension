// Cosmic Ascension - Player layer.

#include "Player/PlayerRoleSubsystem.h"

#include "Cosmic_Ascension.h"
#include "Core/GalaxyState.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
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

	ensureMsgf(Civ || Role != EPlayerRole::Ruler, TEXT("Install: no civ %s to rule"), *CivID.ToString());

	const bool bFirstLife = Calling.Lives.IsEmpty();

	// The vessel starts from nothing: stats, pool, relics and status do not cross over (section 4.4).
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
	Life.SuccessionKind = FGameplayTag::RequestGameplayTag(FName(bFirstLife ? TEXT("Succession.Installed") : TEXT("Succession.Called")));
	// NameIndex stays 0 until the person-name bank of DT_Civs can be read; the draw from GetRng() belongs here.

	if (Role == EPlayerRole::Ruler && Civ)
	{
		State.Player.RuledCivID = CivID;
		Civ->bIsPlayerRuled = true;

		// One entry in each of the three reign arrays: an empty end reason marks the open reign (section 7.2).
		Life.RuledCivIDs.Add(CivID);
		Life.ReignEndReasons.AddDefaulted();
		Life.ReignTicks.Add(0);
	}
}
