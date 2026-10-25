// Cosmic Ascension - World layer: the hash run of a game process (MVP item 9).

#include "World/CosmicHashRun.h"

#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/DataTable.h"
#include "Log/CosmicLogSubsystem.h"
#include "Misc/Parse.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "World/CosmicRulerPolicy.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicHashRunImpl
{
	// One count: Absent without the key; Ticks for 1 to 6 ASCII digits within MaxTicks; else Bad. The value runs to the next
	// whitespace (not to ',' or ')' as FParse::Value's default would: "3,000" must be Bad, not 3).
	CosmicHashRun::ESwitch ReadCount(const TCHAR* CommandLine, const TCHAR* Key, int64& OutCount, FString& OutText)
	{
		OutCount = 0;
		OutText.Reset();
		if (!FParse::Value(CommandLine, Key, OutText, /*bShouldStopOnSeparator*/ false))
		{
			return CosmicHashRun::ESwitch::Absent;
		}
		if (OutText.IsEmpty() || OutText.Len() > 6)
		{
			return CosmicHashRun::ESwitch::Bad;
		}
		int64 Count = 0;
		for (const TCHAR Char : OutText)
		{
			if (Char < TEXT('0') || Char > TEXT('9'))
			{
				return CosmicHashRun::ESwitch::Bad;
			}
			Count = Count * 10 + (Char - TEXT('0'));
		}
		if (Count > CosmicHashRun::MaxTicks)
		{
			return CosmicHashRun::ESwitch::Bad;
		}
		OutCount = Count;
		return CosmicHashRun::ESwitch::Ticks;
	}

	// What of the data the simulation would run without, in table order; empty when nothing is missing.
	FString MissingOf(const UGalaxySimulationSubsystem& Sim)
	{
		const FCosmicDataTables& Tables = Sim.GetTables();
		struct FNamed
		{
			const TCHAR* Name;
			const UDataTable* Table;
		};
		const FNamed Named[] = {
			{ TEXT("DT_Rules"), Tables.Rules.Get() }, { TEXT("DT_Civs"), Tables.Civs.Get() }, { TEXT("DT_LogTemplates"), Tables.LogTemplates.Get() },
			{ TEXT("DT_Galaxies"), Tables.Galaxies.Get() }, { TEXT("DT_Events"), Tables.Events.Get() }, { TEXT("DT_Transcendents"), Tables.Transcendents.Get() },
			{ TEXT("DT_Hazards"), Tables.Hazards.Get() }, { TEXT("DT_Megaprojects"), Tables.Megaprojects.Get() }, { TEXT("DT_Feats"), Tables.Feats.Get() },
			{ TEXT("DT_Encounters"), Tables.Encounters.Get() }, { TEXT("DT_Edicts"), Tables.Edicts.Get() },
		};
		TArray<FString> Missing;
		for (const FNamed& Each : Named)
		{
			if (!Each.Table)
			{
				Missing.Add(Each.Name);
			}
		}
		if (Tables.Rules && !Tables.FindRules())
		{
			Missing.Add(TEXT("DT_Rules:rules"));
		}
		if (Tables.Galaxies && !Tables.FindGalaxy(Sim.GetRules().HomeGalaxyRow))
		{
			Missing.Add(TEXT("DT_Galaxies:home"));
		}
		return FString::Join(Missing, TEXT(", "));
	}

	FString CheckpointOf(const UGalaxySimulationSubsystem& Sim)
	{
		return FString::Printf(TEXT("CosmicHashAt: tick %lld, hash %u, world %u"), Sim.GetState().Tick, Sim.ComputeStateHash(), Sim.ComputeWorldHash());
	}
}

CosmicHashRun::FSwitch CosmicHashRun::SwitchOf(const TCHAR* CommandLine)
{
	FSwitch Switch;
	Switch.Kind = CosmicHashRunImpl::ReadCount(CommandLine, TEXT("CosmicHashRun="), Switch.Ticks, Switch.Text);
	Switch.EveryKind = CosmicHashRunImpl::ReadCount(CommandLine, TEXT("CosmicHashEvery="), Switch.Every, Switch.EveryText);
	Switch.bPolicy = FParse::Param(CommandLine, TEXT("CosmicHashPolicy"));
	return Switch;
}

CosmicHashRun::FResult CosmicHashRun::Run(UGalaxySimulationSubsystem& Sim, UCosmicLogSubsystem& Log, UPlayerRoleSubsystem& Role, int32 Seed, const FSwitch& Switch)
{
	FResult Result;
	Result.Seed = Seed;
	Result.Ticks = Switch.Ticks;
	Result.bPolicy = Switch.bPolicy;
	Result.Missing = CosmicHashRunImpl::MissingOf(Sim);
	if (!Result.Missing.IsEmpty())
	{
		return Result;
	}

	const bool bTellAllBefore = Log.IsTellAll();
	// Idle: the commandlet's default -TellAll=1; policy: its -TellAll=0. Before NewGalaxy, so the lines of tick 0 are told
	// the same way.
	Log.SetTellAll(!Switch.bPolicy);
	Sim.NewGalaxy(Seed, 0, 0);
	const int64 Every = Switch.EveryKind == ESwitch::Ticks ? Switch.Every : 0;
	if (Every > 0)
	{
		Result.Checkpoints.Add(CosmicHashRunImpl::CheckpointOf(Sim));
	}
	if (Switch.bPolicy)
	{
		// The commandlet's loop: one tick, then the ruler's input at the boundary.
		const FCosmicRulerPolicy Policy{ true, true, true, true, NAME_None };
		FCosmicRulerPolicyCounts Counts;
		for (int64 Done = 0; Done < Switch.Ticks; ++Done)
		{
			const FName CardBefore = Sim.GetState().Knowing.PendingPetitionRow;
			Sim.RunHeadless(1);
			CosmicRulerPolicy::Act(Policy, Sim, Role, CardBefore, Counts);
			if (Every > 0 && (Done + 1) % Every == 0)
			{
				Result.Checkpoints.Add(CosmicHashRunImpl::CheckpointOf(Sim));
			}
		}
	}
	else
	{
		for (int64 Done = 0; Done < Switch.Ticks;)
		{
			const int64 Step = Every > 0 ? FMath::Min(Every, Switch.Ticks - Done) : Switch.Ticks;
			Sim.RunHeadless(Step);
			Done += Step;
			if (Every > 0 && (Done % Every == 0))
			{
				Result.Checkpoints.Add(CosmicHashRunImpl::CheckpointOf(Sim));
			}
		}
	}
	Result.StateHash = Sim.ComputeStateHash();
	Result.WorldHash = Sim.ComputeWorldHash();
	Result.Outcome = EOutcome::Ran;
	Log.SetTellAll(bTellAllBefore);
	return Result;
}

FString CosmicHashRun::LineOf(const FResult& Result)
{
	if (Result.Outcome == EOutcome::MissingData)
	{
		return FString::Printf(TEXT("CosmicHash: refused - missing %s"), *Result.Missing);
	}
	return FString::Printf(TEXT("CosmicHash: seed %d, %lld ticks, hash %u, world %u%s"), Result.Seed, Result.Ticks, Result.StateHash, Result.WorldHash,
		Result.bPolicy ? TEXT(", policy") : TEXT(""));
}
