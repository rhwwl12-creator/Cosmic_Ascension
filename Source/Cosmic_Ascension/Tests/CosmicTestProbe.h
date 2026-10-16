// Cosmic Ascension - test probe (guideline.md section 9.3). A UObject whose UFUNCTIONs can be bound to the dynamic
// delegates of the Core Logic subsystems; an automation test hangs its own code on the function members.
// Outside WITH_DEV_AUTOMATION_TESTS on purpose: UHT does not evaluate that macro, so a reflected class cannot sit behind it.

#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"
#include "UObject/Object.h"
#include "Core/GalaxyState.h"
#include "Events/CosmicEventSubsystem.h"
#include "CosmicTestProbe.generated.h"

UCLASS()
class COSMIC_ASCENSION_API UCosmicTestProbe : public UObject
{
	GENERATED_BODY()

public:
	// Bind to UCosmicLogSubsystem::OnLogEntry.
	UFUNCTION()
	void HandleLogEntry(const FCosmicLogEntry& Entry);

	// Bind to UCosmicEventSubsystem::OnEventFired.
	UFUNCTION()
	void HandleEventFired(const FCosmicEventNotice& Notice);

	// Bind to UAscensionSubsystem::OnTierAscended.
	UFUNCTION()
	void HandleTierAscended(EAscensionTier NewTier, FName FeatRow, EPlayerRole Path);

	TFunction<void(const FCosmicLogEntry&)> OnLog;
	TFunction<void(const FCosmicEventNotice&)> OnEvent;
	TFunction<void(EAscensionTier, FName, EPlayerRole)> OnTier;
};
