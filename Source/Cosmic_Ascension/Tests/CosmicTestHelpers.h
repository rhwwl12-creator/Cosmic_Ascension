// Cosmic Ascension - shared helpers of the automation tests (guideline.md section 9.3).

#pragma once

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/GalaxyState.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Misc/AutomationTest.h"
#include "Sim/GalaxySimulationSubsystem.h"

#include <initializer_list>

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

	// A topology a test knows, whatever the generator made (the generation rules change, a test of reach should not):
	// every lane is replaced by a chain 0-1-2-...-9 over the systems in array order, a branch from system 4 to system
	// 10, and system 11 behind 10. So system 4 has three neighbours (3, 5 and 10). False when the state has fewer than
	// twelve systems.
	inline bool WireChain(FGalaxyState& State)
	{
		if (State.Systems.Num() < 12)
		{
			return false;
		}
		for (FStarSystemState& System : State.Systems)
		{
			System.Lanes.Reset();
		}
		auto Link = [&State](int32 A, int32 B)
		{
			State.Systems[A].Lanes.Add(State.Systems[B].SystemID);
			State.Systems[B].Lanes.Add(State.Systems[A].SystemID);
		};
		for (int32 Index = 0; Index < 9; ++Index)
		{
			Link(Index, Index + 1);
		}
		Link(4, 10);
		Link(10, 11);
		return true;
	}

	// Nobody owns anything. HomeSystemID, megaprojects, hazards and spheres stay as they were generated.
	inline void ClearOwners(FGalaxyState& State)
	{
		for (FStarSystemState& System : State.Systems)
		{
			System.OwnerCivID = NAME_None;
		}
		for (FCivilizationState& Civ : State.Civs)
		{
			Civ.OwnedSystems.Reset();
		}
	}

	// Hands systems (indices into Systems) to a civilization. The order given is its OwnedSystems order. Call it after
	// ClearOwners: it does not take a system away from a people that holds it. A people whose capital is not among its
	// systems afterwards gets the first system handed over as its capital, so that no people is left with a capital
	// another people owns.
	inline void Give(FGalaxyState& State, int32 CivIndex, std::initializer_list<int32> SystemIndices)
	{
		for (const int32 SystemIndex : SystemIndices)
		{
			State.Systems[SystemIndex].OwnerCivID = State.Civs[CivIndex].CivID;
			State.Civs[CivIndex].OwnedSystems.Add(State.Systems[SystemIndex].SystemID);
		}
		FCivilizationState& Civ = State.Civs[CivIndex];
		if (SystemIndices.size() > 0 && !Civ.OwnedSystems.Contains(Civ.HomeSystemID))
		{
			Civ.HomeSystemID = State.Systems[*SystemIndices.begin()].SystemID;
		}
	}
}

#endif // WITH_DEV_AUTOMATION_TESTS
