// Cosmic Ascension - Save layer.

#include "Save/CosmicSaveSubsystem.h"
#include "Cosmic_Ascension.h"

void UCosmicSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogCosmic, Log, TEXT("UCosmicSaveSubsystem initialized (Phase 0 shell)"));
}

void UCosmicSaveSubsystem::Deinitialize()
{
	Super::Deinitialize();
}
