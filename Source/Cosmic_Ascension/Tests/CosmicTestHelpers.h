// Cosmic Ascension - shared helpers of the automation tests (guideline.md section 9.3).

#pragma once

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"
#include "Sim/GalaxySimulationSubsystem.h"

namespace CosmicTests
{
	// The tests run with -game, so ClientContext is required (CLAUDE.md).
	inline constexpr EAutomationTestFlags Flags = EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter;

	// Vertical slice size (section 12.1) and the performance budget size (section 9.4).
	inline constexpr int32 SliceSystems = 20;
	inline constexpr int32 SliceCivs = 6;
	inline constexpr int32 BudgetSystems = 120;
	inline constexpr int32 BudgetCivs = 12;

	inline UGameInstance* FindGameInstance()
	{
		if (GEngine)
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				if (Context.OwningGameInstance)
				{
					return Context.OwningGameInstance;
				}
			}
		}
		return nullptr;
	}

	// The simulation of the running game, paused so that frame ticks cannot interleave with a test. The tests replace
	// its galaxy and leave it paused. Null (with a test error) when there is no GameInstance.
	inline UGalaxySimulationSubsystem* AcquireSim(FAutomationTestBase& Test)
	{
		UGameInstance* GameInstance = FindGameInstance();
		UGalaxySimulationSubsystem* Sim = GameInstance ? GameInstance->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
		if (!Sim)
		{
			Test.AddError(TEXT("No GameInstance: run the automation tests with -game (see CLAUDE.md)."));
			return nullptr;
		}
		Sim->SetTimeScale(0.f);
		return Sim;
	}
}

#endif // WITH_DEV_AUTOMATION_TESTS
