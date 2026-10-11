// Cosmic Ascension - Save layer.

#include "Save/CosmicSaveSubsystem.h"

#include "Cosmic_Ascension.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Save/CosmicSaveGame.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UObject/Package.h"

namespace
{
	// Single local player.
	constexpr int32 SaveUserIndex = 0;
}

void UCosmicSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogCosmic, Log, TEXT("UCosmicSaveSubsystem initialized"));
}

void UCosmicSaveSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

FString UCosmicSaveSubsystem::MakeSlotName(const FGuid& GalaxyID)
{
	return FString::Printf(TEXT("Galaxy_%s"), *GalaxyID.ToString(EGuidFormats::Digits));
}

bool UCosmicSaveSubsystem::SaveCurrentGalaxy()
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	if (!Sim->GetState().GalaxyID.IsValid())
	{
		UE_LOG(LogCosmic, Warning, TEXT("SaveCurrentGalaxy: no galaxy to save"));
		return false;
	}

	Sim->SyncRngState();

	UCosmicSaveGame* SaveObject = NewObject<UCosmicSaveGame>(GetTransientPackage());
	SaveObject->State = Sim->GetState();
	const FString SlotName = MakeSlotName(SaveObject->State.GalaxyID);
	const bool bSaved = UGameplayStatics::SaveGameToSlot(SaveObject, SlotName, SaveUserIndex);
	UE_CLOG(!bSaved, LogCosmic, Warning, TEXT("SaveCurrentGalaxy: writing slot %s failed"), *SlotName);
	return bSaved;
}

bool UCosmicSaveSubsystem::LoadGalaxy(const FGuid& GalaxyID)
{
	const FString SlotName = MakeSlotName(GalaxyID);
	const UCosmicSaveGame* Loaded = Cast<UCosmicSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, SaveUserIndex));
	if (!Loaded)
	{
		UE_LOG(LogCosmic, Warning, TEXT("LoadGalaxy: slot %s is missing or unreadable"), *SlotName);
		return false;
	}
	if (Loaded->State.SaveVersion > FGalaxyState::CurrentSaveVersion)
	{
		UE_LOG(LogCosmic, Warning, TEXT("LoadGalaxy: slot %s has save version %d, this build reads up to %d"),
			*SlotName, Loaded->State.SaveVersion, FGalaxyState::CurrentSaveVersion);
		return false;
	}
	// Migration functions for SaveVersion < CurrentSaveVersion go here once a version bump exists (section 9.1).

	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	Sim->LoadState(Loaded->State);
	return true;
}

bool UCosmicSaveSubsystem::DoesSaveExist(const FGuid& GalaxyID) const
{
	return UGameplayStatics::DoesSaveGameExist(MakeSlotName(GalaxyID), SaveUserIndex);
}

bool UCosmicSaveSubsystem::DeleteSave(const FGuid& GalaxyID)
{
	return UGameplayStatics::DeleteGameInSlot(MakeSlotName(GalaxyID), SaveUserIndex);
}
