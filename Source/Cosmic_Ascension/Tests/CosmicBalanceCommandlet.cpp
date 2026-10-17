// Cosmic Ascension - balance commandlet.

#include "Tests/CosmicBalanceCommandlet.h"

#include "Core/GalaxyState.h"
#include "Cosmic_Ascension.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameMapsSettings.h"
#include "GameplayTagContainer.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "Log/CosmicLogSubsystem.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Save/CosmicSaveGame.h"
#include "Sim/CosmicHazards.h"
#include "Sim/GalaxySimulationSubsystem.h"

namespace
{
	constexpr int64 SampleEveryTicks = 100;
	// SaveBytes is measured on every tenth sample only: one save of the whole state takes tens of milliseconds.
	constexpr int64 SaveEverySamples = 10;

	// What the run counts tick by tick, read from the state only (no hook into the simulation, no draw).
	struct FRunCounts
	{
		int64 FirstClaimTick = -1;
		int64 FirstJoinTick = -1;
		int64 DeposedTick = -1;
		int32 ClaimsJoined = 0;
		int32 ClaimsEnded = 0;
		int64 ToldLines = 0;
		TMap<FString, int32> Placed;
		TMap<FString, int32> Expired;
		TMap<FString, int32> Ended;
	};

	struct FSlotBefore
	{
		FGameplayTag Tag;
		int32 Remaining = 0;
	};

	int32 SphereSize(const FGalaxyState& State)
	{
		int32 Size = 0;
		for (const FTranscendentState& Transcendent : State.Transcendents)
		{
			Size += Transcendent.InfluencedSystems.Num();
		}
		return Size;
	}

	bool InSomeSphere(const FGalaxyState& State, FName SystemID)
	{
		for (const FTranscendentState& Transcendent : State.Transcendents)
		{
			if (Transcendent.InfluencedSystems.Contains(SystemID))
			{
				return true;
			}
		}
		return false;
	}

	int32 WildSystems(const FGalaxyState& State)
	{
		int32 Wild = 0;
		for (const FStarSystemState& System : State.Systems)
		{
			Wild += (System.OwnerCivID.IsNone() && !CosmicHazards::IsDeadNode(System)) ? 1 : 0;
		}
		return Wild;
	}

	// The peoples met, the birth peoples of the lives left out (section 9.3), and the tick of the first contact (empty while
	// there is none).
	int32 MetCivs(const FGalaxyState& State)
	{
		int32 Met = 0;
		for (const FKnownMark& Mark : State.Knowing.MetCivs)
		{
			const bool bBirth = State.Calling.Lives.ContainsByPredicate([&Mark](const FLifeRecord& Life) { return Life.BirthCivID == Mark.ID; });
			Met += bBirth ? 0 : 1;
		}
		return Met;
	}

	FString FirstContactTick(const FGalaxyState& State)
	{
		const FName First = State.Knowing.FirstContactCivID;
		const FKnownMark* Mark = First.IsNone() ? nullptr : State.Knowing.MetCivs.FindByPredicate([First](const FKnownMark& Each) { return Each.ID == First; });
		return Mark ? FString::Printf(TEXT("%lld"), Mark->Tick) : FString();
	}

	int32 T4PlusCount(const FGalaxyState& State)
	{
		int32 Count = 0;
		for (const FCivilizationState& Civ : State.Civs)
		{
			Count += static_cast<int32>(Civ.Tier) >= static_cast<int32>(EAscensionTier::Tier4_Transcendent) ? 1 : 0;
		}
		return Count + State.Transcendents.Num();
	}

	int32 SaveBytesOf(const FGalaxyState& State, double& OutMilliseconds)
	{
		UCosmicSaveGame* Save = NewObject<UCosmicSaveGame>();
		Save->State = State;
		TArray<uint8> Bytes;
		const double Start = FPlatformTime::Seconds();
		UGameplayStatics::SaveGameToMemory(Save, Bytes);
		OutMilliseconds = (FPlatformTime::Seconds() - Start) * 1000.0;
		return Bytes.Num();
	}

	void AppendSample(FString& Csv, const FGalaxyState& State, const FRunCounts& Counts, double TickMicros, const FString& SaveBytes)
	{
		const FString FirstClaim = Counts.FirstClaimTick >= 0 ? FString::Printf(TEXT("%lld"), Counts.FirstClaimTick) : FString();
		const FString Galaxy = FString::Printf(TEXT("%d,%d,%d,%s,%d,%s,%lld,%.1f,%s,%d,%d"),
			State.Knowing.KnownSystems.Num(), MetCivs(State), WildSystems(State), *FirstContactTick(State), T4PlusCount(State),
			*FirstClaim, Counts.ToldLines, TickMicros, *SaveBytes, State.Log.Num(), State.Chronicle.Num());
		for (const FCivilizationState& Civ : State.Civs)
		{
			int32 Hazards = 0;
			for (const FStarSystemState& System : State.Systems)
			{
				Hazards += (System.OwnerCivID == Civ.CivID && System.ActiveHazard.IsValid()) ? 1 : 0;
			}
			Csv += FString::Printf(TEXT("%lld,%s,%s,%d,%d,%.4f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%s\n"),
				State.Tick, *Civ.CivID.ToString(), *Civ.ArchetypeRow.ToString(), static_cast<int32>(Civ.Tier) + 1, Civ.OwnedSystems.Num(),
				Civ.Stability, Civ.Stockpile.Energy, Civ.Stockpile.Matter, Civ.Stockpile.Insight, Civ.Stockpile.Influence, Civ.Stockpile.Causality, Hazards,
				*Galaxy);
		}
	}

	FString CountsText(const TMap<FString, int32>& Counts)
	{
		TArray<FString> Keys;
		Counts.GetKeys(Keys);
		Keys.Sort();
		FString Text;
		for (const FString& Key : Keys)
		{
			Text += FString::Printf(TEXT(" %s %d"), *Key, Counts[Key]);
		}
		return Text.IsEmpty() ? TEXT(" none") : Text;
	}
}

int32 UCosmicBalanceCommandlet::Main(const FString& Params)
{
	static const FGameplayTag ClaimTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Hazard.Claim")));
	int32 Seed = 42;
	int64 Ticks = 20000;
	// 0 = the home galaxy row (DT_Galaxies: LivingWorlds, Peoples). Give them only to measure another size (section 9.3).
	int32 Systems = 0;
	int32 Civs = 0;
	int32 TellAll = 1;
	FString Out = TEXT("balance.csv");
	FParse::Value(*Params, TEXT("Seed="), Seed);
	FParse::Value(*Params, TEXT("Ticks="), Ticks);
	FParse::Value(*Params, TEXT("Systems="), Systems);
	FParse::Value(*Params, TEXT("Civs="), Civs);
	FParse::Value(*Params, TEXT("TellAll="), TellAll);
	FParse::Value(*Params, TEXT("Out="), Out);
	const FString OutPath = FPaths::IsRelative(Out) ? FPaths::ProjectSavedDir() / TEXT("Balance") / Out : Out;
	const FString RowsPath = FPaths::GetPath(OutPath) / (FPaths::GetBaseFilename(OutPath) + TEXT("_rows.csv"));

	// The commandlet runs under the editor engine without a GameInstance, and the subsystems live on one (section 9.3).
	UClass* GameInstanceClass = GetDefault<UGameMapsSettings>()->GameInstanceClass.TryLoadClass<UGameInstance>();
	UGameInstance* GameInstance = NewObject<UGameInstance>(GEngine, GameInstanceClass ? GameInstanceClass : UGameInstance::StaticClass());
	GameInstance->AddToRoot();
	GameInstance->InitializeStandalone();

	int32 Result = 0;
	UGalaxySimulationSubsystem* Sim = GameInstance->GetSubsystem<UGalaxySimulationSubsystem>();
	if (Sim)
	{
		// Every line is stored unless -TellAll=0 asks for the scope of telling of a game (section 8.1). The world hash is
		// the same either way.
		if (UCosmicLogSubsystem* Log = GameInstance->GetSubsystem<UCosmicLogSubsystem>())
		{
			Log->SetTellAll(TellAll != 0);
		}
		Sim->NewGalaxy(Seed, Systems, Civs);
		const FGalaxyState& State = Sim->GetState();

		FRunCounts Counts;
		const int32 SphereStart = SphereSize(State);
		for (const FCosmicLogEntry& Entry : State.Log)
		{
			Counts.ToldLines += Entry.bUntold ? 0 : 1;
		}
		FString Csv = TEXT("Tick,CivID,Archetype,Tier,Systems,Stability,Energy,Matter,Insight,Influence,Causality,Hazards,KnownSystems,MetCivs,WildSystems,FirstContactTick,T4PlusCount,FirstClaimTick,ToldLines,TickMicros,SaveBytes,LogLen,ChronicleLen\n");
		FString Rows = TEXT("Tick,SourceRow,Lines\n");
		double Unused = 0.0;
		AppendSample(Csv, State, Counts, 0.0, FString::FromInt(SaveBytesOf(State, Unused)));

		TArray<double> TickMicros;
		TickMicros.Reserve(static_cast<int32>(FMath::Min<int64>(Ticks, MAX_int32)));
		TArray<FSlotBefore> Before;
		TMap<FString, int32> SampleRows;
		double SampleMicros = 0.0;
		int64 SampleTicks = 0;
		int64 Samples = 0;
		for (int64 Done = 0; Done < Ticks; ++Done)
		{
			Before.Reset();
			for (const FStarSystemState& System : State.Systems)
			{
				Before.Add({ System.ActiveHazard, System.HazardTicksRemaining });
			}
			const EPlayerRole RoleBefore = State.Player.Role;
			const int64 TickBefore = State.Tick;
			const int32 SphereBefore = SphereSize(State);

			const uint64 Start = FPlatformTime::Cycles64();
			Sim->RunHeadless(1);
			const double Micros = static_cast<double>(FPlatformTime::Cycles64() - Start) * FPlatformTime::GetSecondsPerCycle64() * 1.0e6;
			TickMicros.Add(Micros);
			SampleMicros += Micros;
			++SampleTicks;

			// A hazard is placed when a slot holds a tag it did not hold or holds it with a full count again; it ended when
			// its tag is gone or counted again, and it expired when its count was 1 or less before the tick.
			for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num() && SystemIndex < Before.Num(); ++SystemIndex)
			{
				const FStarSystemState& System = State.Systems[SystemIndex];
				const FSlotBefore& Slot = Before[SystemIndex];
				const bool bAgain = System.ActiveHazard == Slot.Tag && System.HazardTicksRemaining >= Slot.Remaining;
				if (Slot.Tag.IsValid() && (System.ActiveHazard != Slot.Tag || bAgain))
				{
					Counts.Ended.FindOrAdd(Slot.Tag.ToString())++;
					if (Slot.Remaining <= 1)
					{
						Counts.Expired.FindOrAdd(Slot.Tag.ToString())++;
					}
					if (Slot.Tag == ClaimTag)
					{
						++Counts.ClaimsEnded;
						Counts.ClaimsJoined += InSomeSphere(State, System.SystemID) ? 1 : 0;
					}
				}
				if (System.ActiveHazard.IsValid() && (System.ActiveHazard != Slot.Tag || bAgain))
				{
					Counts.Placed.FindOrAdd(System.ActiveHazard.ToString())++;
					if (System.ActiveHazard == ClaimTag && Counts.FirstClaimTick < 0)
					{
						Counts.FirstClaimTick = State.Tick;
					}
				}
			}
			if (Counts.FirstJoinTick < 0 && SphereSize(State) > SphereBefore)
			{
				Counts.FirstJoinTick = State.Tick;
			}
			if (Counts.DeposedTick < 0 && RoleBefore == EPlayerRole::Ruler && State.Player.Role != EPlayerRole::Ruler)
			{
				Counts.DeposedTick = State.Tick;
			}
			for (int32 Index = State.Log.Num() - 1; Index >= 0 && State.Log[Index].Tick > TickBefore; --Index)
			{
				SampleRows.FindOrAdd(State.Log[Index].SourceRow.ToString())++;
				Counts.ToldLines += State.Log[Index].bUntold ? 0 : 1;
			}

			if ((Done + 1) % SampleEveryTicks == 0 || Done + 1 == Ticks)
			{
				++Samples;
				FString SaveBytes;
				if (Samples % SaveEverySamples == 0 || Done + 1 == Ticks)
				{
					SaveBytes = FString::FromInt(SaveBytesOf(State, Unused));
				}
				AppendSample(Csv, State, Counts, SampleTicks > 0 ? SampleMicros / SampleTicks : 0.0, SaveBytes);
				TArray<FString> Keys;
				SampleRows.GetKeys(Keys);
				Keys.Sort();
				for (const FString& Key : Keys)
				{
					Rows += FString::Printf(TEXT("%lld,%s,%d\n"), State.Tick, *Key, SampleRows[Key]);
				}
				SampleRows.Reset();
				SampleMicros = 0.0;
				SampleTicks = 0;
			}
		}

		if (FFileHelper::SaveStringToFile(Csv, *OutPath) && FFileHelper::SaveStringToFile(Rows, *RowsPath))
		{
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance: seed %d, %lld ticks, hash %u, world %u, wrote %s"), Seed, Ticks, Sim->ComputeStateHash(), Sim->ComputeWorldHash(), *OutPath);
			TickMicros.Sort();
			double Sum = 0.0;
			for (const double Each : TickMicros)
			{
				Sum += Each;
			}
			double SaveMilliseconds = 0.0;
			const int32 SaveBytes = SaveBytesOf(State, SaveMilliseconds);
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance summary: tell-all %d, %d systems, %d peoples; claims %d placed, %d ended, %d joined; first claim %lld, first join %lld; sphere %d->%d; wild %d; vessel deposed at %lld; tick us mean %.1f p99 %.1f max %.1f; save %d bytes in %.1f ms; log %d, chronicle %d, told %lld"),
				TellAll, State.Systems.Num(), State.Civs.Num(), Counts.Placed.FindRef(ClaimTag.ToString()), Counts.ClaimsEnded, Counts.ClaimsJoined, Counts.FirstClaimTick,
				Counts.FirstJoinTick, SphereStart, SphereSize(State), WildSystems(State), Counts.DeposedTick,
				TickMicros.Num() ? Sum / TickMicros.Num() : 0.0, TickMicros.Num() ? TickMicros[FMath::Min(TickMicros.Num() - 1, static_cast<int32>(TickMicros.Num() * 0.99))] : 0.0,
				TickMicros.Num() ? TickMicros.Last() : 0.0, SaveBytes, SaveMilliseconds, State.Log.Num(), State.Chronicle.Num(), Counts.ToldLines);
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance hazards: placed%s | expired%s | ended%s"), *CountsText(Counts.Placed), *CountsText(Counts.Expired), *CountsText(Counts.Ended));
		}
		else
		{
			UE_LOG(LogCosmic, Error, TEXT("CosmicBalance: could not write %s or %s"), *OutPath, *RowsPath);
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
