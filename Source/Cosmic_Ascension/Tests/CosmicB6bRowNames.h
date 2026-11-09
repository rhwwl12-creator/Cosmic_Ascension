// Cosmic Ascension - the names of B6b's rows that no game code calls by name, for the measurement only (B6b header
// section 3.1, 5.1 - [B-6b3b]): CosmicTests::WithoutB6bData takes them out of a copy of the tables (the data-off side of a
// pair, -WithoutB6bData=1), and the balance commandlet's race line reads the rows it names (the era's line, the witness card).
// They are not CosmicRows:: constants (Data/CosmicRowNames.h): the data checker holds every constant of that header - a
// row_name_sources file (Tools/data/schema.json) - to a required row the data holds, both ways (Tools/cosmic_data_schema.py,
// load_code_rows), so a constant would say that the code calls its row; and project_ark is a row the code never calls
// (B6b header section 3.1: "no constant"). A name moves to CosmicRows:: only with the commit whose code calls its row by
// name; the data commits [B-6b6]-[B-6b8] add rows the code reads by tag or by table walk, so their names stay here. The
// precedent is WithoutPressure's FName(TEXT("edict_against_silence")) (Tests/CosmicTestHelpers.h).
// Not behind WITH_DEV_AUTOMATION_TESTS: the commandlet builds in every target.

#pragma once

#include "CoreMinimal.h"

namespace CosmicB6bRows
{
	// DT_Feats: the feats of the third rung (the passage by the ark) and the fourth (the laws and the binding) - in the data from [B-6b6], [B-6b7].
	inline const FName FeatPassageByArk(TEXT("feat_passage_by_ark"));
	inline const FName FeatLawsAndBinding(TEXT("feat_laws_and_binding"));

	// DT_Megaprojects: the extinction's answer (the response slot) - in the data from [B-6b6].
	inline const FName ProjectArk(TEXT("project_ark"));

	// DT_Hazards: the extinction (Hazard.Extinction) - in the data from [B-6b6].
	inline const FName CrisisSpeciesExtinction(TEXT("crisis_species_extinction"));

	// DT_Edicts: the three galaxy laws (Scope HomeGalaxy) - in the data from [B-6b7].
	inline const FName EdictWideMachineAccord(TEXT("edict_wide_machine_accord"));
	inline const FName EdictWideQuarantine(TEXT("edict_wide_quarantine"));
	inline const FName EdictWideWarning(TEXT("edict_wide_warning"));

	// DT_Events: the era of the endured pressure - in the data from [B-6b8].
	inline const FName EventEraEndured(TEXT("event_era_endured"));

	// DT_Encounters: the wanderer's witness - in the data from [B-6b8].
	inline const FName EncWitness(TEXT("enc_witness"));
}
