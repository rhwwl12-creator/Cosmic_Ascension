// Cosmic Ascension - Sim layer.

#include "Sim/GalaxySimulationSubsystem.h"
#include "Cosmic_Ascension.h"

void UGalaxySimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogCosmic, Log, TEXT("UGalaxySimulationSubsystem initialized (Phase 0 shell)"));
}

void UGalaxySimulationSubsystem::Deinitialize()
{
	UE_LOG(LogCosmic, Log, TEXT("UGalaxySimulationSubsystem deinitialized"));
	Super::Deinitialize();
}
