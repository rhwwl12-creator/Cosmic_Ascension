// Cosmic Ascension - Player layer.

#include "Player/PlayerRoleSubsystem.h"
#include "Cosmic_Ascension.h"

void UPlayerRoleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogCosmic, Log, TEXT("UPlayerRoleSubsystem initialized (Phase 0 shell)"));
}

void UPlayerRoleSubsystem::Deinitialize()
{
	Super::Deinitialize();
}
