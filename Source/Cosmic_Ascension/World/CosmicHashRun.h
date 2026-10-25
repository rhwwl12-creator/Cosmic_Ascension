// Cosmic Ascension - World layer: the hash run of a game process (MVP item 9).

#pragma once

#include "CoreMinimal.h"

class UCosmicLogSubsystem;
class UGalaxySimulationSubsystem;
class UPlayerRoleSubsystem;

// MVP item 9 (docs/60_DECISIONS/headers/A9_win64_packaging.md section 2): the cooked build's proof of determinism. A
// development switch read once at the start of a game process (ACosmicGameMode::StartPlay) runs a seed headless - as the
// balance commandlet's idle run, or its ruler-policy run - writes lines with both hashes and asks the engine to quit.
// Nothing a player keeps is touched: the run needs -Seed=, which closes the game's saving, and it refuses while saving
// is on.
namespace CosmicHashRun
{
	inline constexpr int64 MaxTicks = 100000;

	// A switch with a tick count: Absent when the key is not on the command line; Bad when its value is not 1 to 6 ASCII
	// digits or lies above MaxTicks ("=abc", "=", "=+3000", "=3000x", "=3,000", "=-5", "=100001"); else Ticks. The value is
	// read as text up to the next whitespace, never as an int64 (that overload stops at the first non-digit and takes
	// "3000x" for 3000).
	enum class ESwitch : uint8
	{
		Absent,
		Bad,
		Ticks,
	};

	struct FSwitch
	{
		// -CosmicHashRun=<ticks>: 0 = the generated galaxy alone, no tick.
		ESwitch Kind = ESwitch::Absent;
		int64 Ticks = 0;
		// -CosmicHashEvery=<ticks>: a checkpoint line at tick 0 and after every Every ticks (Absent or 0 = none; Bad as above).
		ESwitch EveryKind = ESwitch::Absent;
		int64 Every = 0;
		// -CosmicHashPolicy: the ruler policy of the commandlet's -Answer=1 -Build=1 -Focus=1 -Edict=1 -Diplomacy=1 -TellAll=0.
		bool bPolicy = false;
		// The values as given, for the refusal line.
		FString Text;
		FString EveryText;

		// Whether the run may start: the tick count read and the checkpoint count, if given, too. -CosmicHashEvery or
		// -CosmicHashPolicy without -CosmicHashRun is refused (StartPlay enters the hash run on any of the three).
		bool IsGood() const { return Kind == ESwitch::Ticks && EveryKind != ESwitch::Bad; }
	};

	FSwitch SwitchOf(const TCHAR* CommandLine);

	enum class EOutcome : uint8
	{
		// The hashes are read.
		Ran,
		// A table, the rules row or the home galaxy row is missing: no NewGalaxy, no hashes.
		MissingData,
	};

	struct FResult
	{
		EOutcome Outcome = EOutcome::MissingData;
		int32 Seed = 0;
		int64 Ticks = 0;
		bool bPolicy = false;
		uint32 StateHash = 0;
		uint32 WorldHash = 0;
		// "CosmicHashAt: tick 0, hash H, world W", ... when Every > 0.
		TArray<FString> Checkpoints;
		// What is missing, in table order ("DT_Rules, DT_Galaxies:home"), when MissingData.
		FString Missing;
	};

	// Idle: the log telling all (the commandlet's default -TellAll=1), switched before NewGalaxy(Seed, 0, 0) so the lines
	// of tick 0 are told the same way, then RunHeadless in steps of Every (or in one step), then both hashes. Policy: the
	// log in the game's scope (-TellAll=0), NewGalaxy, then one tick at a time with CosmicRulerPolicy::Act between ticks -
	// the commandlet's loop. The log's mode comes back afterwards. Refuses before NewGalaxy when any of the eleven tables,
	// the rules row or the home galaxy row is missing (the hash would come from C++ defaults). Switch must be IsGood.
	FResult Run(UGalaxySimulationSubsystem& Sim, UCosmicLogSubsystem& Log, UPlayerRoleSubsystem& Role, int32 Seed, const FSwitch& Switch);

	// "CosmicHash: seed 42, 3000 ticks, hash 4061194924, world 681643174" (", policy" after the policy run) - the key of
	// the commandlet's "CosmicBalance: seed %d, %lld ticks, hash %u, world %u, wrote %s" without the path. MissingData
	// gives "CosmicHash: refused - missing <Missing>".
	FString LineOf(const FResult& Result);
}
