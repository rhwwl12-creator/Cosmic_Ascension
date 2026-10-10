// Cosmic Ascension - Log layer.

#include "Log/CosmicLogSubsystem.h"
#include "Cosmic_Ascension.h"

void UCosmicLogSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogCosmic, Log, TEXT("UCosmicLogSubsystem initialized (Phase 0 shell)"));
}

void UCosmicLogSubsystem::Deinitialize()
{
	Super::Deinitialize();
}
