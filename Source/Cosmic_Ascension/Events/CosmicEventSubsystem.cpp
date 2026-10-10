// Cosmic Ascension - Events layer.

#include "Events/CosmicEventSubsystem.h"
#include "Cosmic_Ascension.h"

void UCosmicEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogCosmic, Log, TEXT("UCosmicEventSubsystem initialized (Phase 0 shell)"));
}

void UCosmicEventSubsystem::Deinitialize()
{
	Super::Deinitialize();
}
