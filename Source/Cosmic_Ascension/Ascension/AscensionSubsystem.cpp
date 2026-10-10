// Cosmic Ascension - Ascension layer.

#include "Ascension/AscensionSubsystem.h"
#include "Cosmic_Ascension.h"

void UAscensionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogCosmic, Log, TEXT("UAscensionSubsystem initialized (Phase 0 shell)"));
}

void UAscensionSubsystem::Deinitialize()
{
	Super::Deinitialize();
}
