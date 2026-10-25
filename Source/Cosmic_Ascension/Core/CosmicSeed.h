// Cosmic Ascension - Core layer. Design: guideline.md section 9.2 (the three sources of randomness).
// CosmicSeed::Derive is the third source: a stateless integer derived from the run seed, a name and a purpose.
// It gives the starting value of the tell stream, the values and the departure schedule of summary galaxies, names of
// peoples that crossed, the phase of a civilization's event slot, and what the presentation layer draws (backdrop stars,
// scenery galaxies). It never draws from the sim stream and never seeds it; Depart and EventSlot only decide in which tick
// a fixed sim draw falls.
//
// The expression and the constants are frozen (section 9.2 prints them, the test Sim.SeedDerive holds reference values):
// unsigned 32-bit arithmetic only, no engine hash (GetTypeHash and FCrc differ in meaning across uses), no floating point.

#pragma once

#include "CoreMinimal.h"
#include "Containers/StringConv.h"
#include "Containers/StringView.h"
#include "Containers/UnrealString.h"
#include "UObject/NameTypes.h"

namespace CosmicSeed
{
	/** What a derived value is for. Fixed numbers: never renumber, append only. */
	namespace Purpose
	{
		inline constexpr uint32 Tell = 1;       // starting value of the tell stream (name is the empty string)
		inline constexpr uint32 Summary = 2;    // values of a summary galaxy (Phase 6a)
		inline constexpr uint32 Name = 3;       // name of a people that crossed (Phase 6a)
		inline constexpr uint32 Life = 4;       // the life word shown for a system (presentation)
		inline constexpr uint32 Backdrop = 5;   // backdrop stars (presentation)
		inline constexpr uint32 Scenery = 6;    // scenery galaxies (presentation)
		inline constexpr uint32 Depart = 7;     // departure schedule of a summary galaxy (Phase 6a)
		inline constexpr uint32 Archetype = 8;  // archetype of a group that crosses from a summary galaxy (Phase 6a)
		inline constexpr uint32 EventSlot = 9;  // phase of a civilization's event slot (name = CivID), section 7.5
		inline constexpr uint32 Petition = 10;  // the pick among the petition cards of a moment (name = ruled CivID, index = tick), section 5.10
		inline constexpr uint32 Encounter = 11; // a wanderer's card: the arrival card's share and pick, the pick among Standing rows (name = the vessel's SystemID, index = tick), B1
	}

	/**
	 * Stateless integer from (Seed, name, purpose, index). LowerAsciiName is a row name or an ID in lower-case ASCII:
	 * the caller lower-cases it, and every byte of the view is folded in as it is.
	 * FNV-1a 32 over the name, then Seed, Purpose and Index are mixed in, then a 32-bit finalizer.
	 * A caller that holds an FName must not pass Name.ToString() as it is: outside the editor target an FName prints in
	 * the casing of whichever string registered it first, so tests would pass and the game binary would differ. Such a
	 * caller uses DeriveForName below.
	 */
	constexpr uint32 Derive(int32 Seed, FAnsiStringView LowerAsciiName, uint32 Purpose, uint32 Index = 0)
	{
		uint32 Hash = 0x811C9DC5u;
		for (const ANSICHAR Char : LowerAsciiName)
		{
			Hash ^= static_cast<uint8>(Char);
			Hash *= 0x01000193u;
		}
		Hash ^= static_cast<uint32>(Seed);
		Hash *= 0x9E3779B1u;
		Hash ^= Purpose;
		Hash *= 0x85EBCA6Bu;
		Hash ^= Index;
		Hash *= 0xC2B2AE35u;
		Hash ^= Hash >> 16;
		Hash *= 0x85EBCA6Bu;
		Hash ^= Hash >> 13;
		Hash *= 0xC2B2AE35u;
		Hash ^= Hash >> 16;
		return Hash;
	}

	// Reference values of section 9.2, checked at compile time. Sim.SeedDerive checks them again at run time: those two
	// hold the values. Sim.SaveRestart also writes one value into its record (same build in both halves: no extra guard).
	static_assert(Derive(42, "", Purpose::Tell, 0) == 272114134u, "CosmicSeed::Derive is frozen (guideline section 9.2)");
	static_assert(Derive(42, "gal_andromeda", Purpose::Summary, 0) == 569404729u, "CosmicSeed::Derive is frozen (guideline section 9.2)");
	static_assert(Derive(7, "sol", Purpose::Life, 3) == 2833998181u, "CosmicSeed::Derive is frozen (guideline section 9.2)");
	static_assert(Derive(-1, "gal_lmc", Purpose::Depart, 2) == 668976756u, "CosmicSeed::Derive is frozen (guideline section 9.2)");
	static_assert(Derive(0, "", Purpose::Tell, 0) == 4196651879u, "CosmicSeed::Derive is frozen (guideline section 9.2)");
	static_assert(Derive(42, "", Purpose::Tell) == 272114134u, "CosmicSeed::Derive is frozen (guideline section 9.2): Index defaults to 0");

	/**
	 * Derive for a caller that holds an FName (a CivID, a SystemID, a row name): the name is printed, lower-cased and
	 * narrowed to ASCII first, so the value does not depend on the casing the name was registered in. NAME_None is
	 * derived as the empty name.
	 */
	inline uint32 DeriveForName(int32 Seed, FName Name, uint32 Purpose, uint32 Index = 0)
	{
		if (Name.IsNone())
		{
			return Derive(Seed, FAnsiStringView(), Purpose, Index);
		}
		FString Text = Name.ToString();
		Text.ToLowerInline();
		const auto Ansi = StringCast<ANSICHAR>(*Text);
		return Derive(Seed, FAnsiStringView(Ansi.Get(), Ansi.Length()), Purpose, Index);
	}
}
