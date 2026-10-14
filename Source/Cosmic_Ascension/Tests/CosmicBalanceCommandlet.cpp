// Cosmic Ascension - balance commandlet.

#include "Tests/CosmicBalanceCommandlet.h"

#include "Core/GalaxyState.h"
#include "Cosmic_Ascension.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameMapsSettings.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Sim/GalaxySimulationSubsystem.h"

namespace
{
	constexpr int64 SampleEveryTicks = 100;

	void AppendSample(FString& Csv, const FGalaxyState& State)
	{
		for (const FCivilizationState& Civ : State.Civs)
		{
			int32 Hazards = 0;
			for (const FStarSystemState& System : State.Systems)
			{
				Hazards += (System.OwnerCivID == Civ.CivID && System.ActiveHazard.IsValid()) ? 1 : 0;
			}
			Csv += FString::Printf(TEXT("%lld,%s,%s,%d,%d,%.4f,%.2f,%.2f,%.2f,%.2f,%.2f,%d\n"),
				State.Tick, *Civ.CivID.ToString(), *Civ.ArchetypeRow.ToString(), static_cast<int32>(Civ.Tier) + 1, Civ.OwnedSystems.Num(),
				Civ.Stability, Civ.Stockpile.Energy, Civ.Stockpile.Matter, Civ.Stockpile.Insight, Civ.Stockpile.Influence, Civ.Stockpile.Causality, Hazards);
		}
	}
}

int32 UCosmicBalanceCommandlet::Main(const FString& Params)
{
	int32 Seed = 42;
	int64 Ticks = 20000;
	// 0 = the home galaxy row (DT_Galaxies: LivingWorlds, Peoples). Give them only to measure another size (section 9.3).
	int32 Systems = 0;
	int32 Civs = 0;
	FString Out = TEXT("balance.csv");
	FParse::Value(*Params, TEXT("Seed="), Seed);
	FParse::Value(*Params, TEXT("Ticks="), Ticks);
	FParse::Value(*Params, TEXT("Systems="), Systems);
	FParse::Value(*Params, TEXT("Civs="), Civs);
	FParse::Value(*Params, TEXT("Out="), Out);
	const FString OutPath = FPaths::IsRelative(Out) ? FPaths::ProjectSavedDir() / TEXT("Balance") / Out : Out;

	// The commandlet runs under the editor engine without a GameInstance, and the subsystems live on one (section 9.3).
	UClass* GameInstanceClass = GetDefault<UGameMapsSettings>()->GameInstanceClass.TryLoadClass<UGameInstance>();
	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine, GameInstanceClass ? GameInstanceClass : UGameInstance::StaticClass());
	GameInstance->AddToRoot();
	GameInstance->InitializeStandalone();

	int32 Result = 0;
	UGalaxySimulationSubsystem* Sim = GameInstance->GetSubsystem<UGalaxySimulationSubsystem>();
	if (Sim)
	{
		Sim->NewGalaxy(Seed, Systems, Civs);

		FString Csv = TEXT("Tick,CivID,Archetype,Tier,Systems,Stability,Energy,Matter,Insight,Influence,Causality,Hazards\n");
		AppendSample(Csv, Sim->GetState());
		for (int64 Done = 0; Done < Ticks; Done += SampleEveryTicks)
		{
			Sim->RunHeadless(FMath::Min(SampleEveryTicks, Ticks - Done));
			AppendSample(Csv, Sim->GetState());
		}

		if (FFileHelper::SaveStringToFile(Csv, *OutPath))
		{
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance: seed %d, %lld ticks, hash %u, world %u, wrote %s"), Seed, Ticks, Sim->ComputeStateHash(), Sim->ComputeWorldHash(), *OutPath);
		}
		else
		{
			UE_LOG(LogCosmic, Error, TEXT("CosmicBalance: could not write %s"), *OutPath);
			Result = 1;
		}
	}
	else
	{
		UE_LOG(LogCosmic, Error, TEXT("CosmicBalance: no simulation subsystem"));
		Result = 1;
	}

	// InitializeStandalone made a world context and a dummy world; Shutdown leaves both behind, and a world destroyed by
	// the exit-time collection alone trips an ensure in its subsystems.
	UWorld* World = GameInstance->GetWorld();
	GameInstance->Shutdown();
	if (World)
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	}
	GameInstance->RemoveFromRoot();
	return Result;
}
