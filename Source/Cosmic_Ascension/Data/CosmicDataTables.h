// Cosmic Ascension - Data layer. Design: guideline.md section 7.3, 10.1.
// The definition tables the Core Logic reads. They are found by fixed asset path under /Game/CA/Data; there is no
// settings asset. Tests replace them through UGalaxySimulationSubsystem::SetTables.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/CosmicDefinitionRows.h"
#include "CosmicDataTables.generated.h"

// The String Table of the labels (ST_CA, section 8.5): the UI's labels, and the one fixed name the simulation flattens into
// the state when it makes a galaxy (the sanctuary's, ST_CA System.Sanctuary). Read with FindOrFullyLoad.
namespace CosmicLabels
{
	inline const FName TableId(TEXT("/Game/CA/Data/ST_CA.ST_CA"));
}

USTRUCT()
struct COSMIC_ASCENSION_API FCosmicDataTables
{
	GENERATED_BODY()

	// DT_Rules: a single row named "rules".
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> Rules;

	// DT_Civs: one row per civilization archetype.
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> Civs;

	// DT_LogTemplates: lines that belong to no event row. Named rows are addressed by row name, selector rows by
	// (Category, Context) (section 8.2).
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> LogTemplates;

	// DT_Galaxies: one row per galaxy that has a name (section 5.11). The simulation reads the home row alone
	// (DT_Rules.HomeGalaxyRow); a missing table or home row means the C++ defaults of FGalaxyDef. The presentation draws
	// every row on the universe level (Phase 3 item 9).
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> Galaxies;

	// DT_Events: one row per event (section 7.3). A missing table means no rows; the event slots still draw.
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> Events;

	// DT_Transcendents: one row per transcendent, row name == TranscendentID (section 4.2). A missing table means a run
	// without transcendents.
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> Transcendents;

	// DT_Hazards: one row per hazard, HazardTag unique across rows (section 4.1). A missing table means no hazard is ever
	// placed; the spawn draw still draws.
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> Hazards;

	// DT_Megaprojects: one row per megaproject (section 5.7). A missing table means no project can start; the stage still
	// runs.
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> Megaprojects;

	// DT_Feats: one row per Mythic Feat (section 3.2). A missing table means nobody ascends.
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> Feats;

	// Loads every table that has an asset from its fixed path: DT_Rules, DT_Civs, DT_LogTemplates, DT_Galaxies,
	// DT_Events, DT_Transcendents, DT_Hazards, DT_Megaprojects and DT_Feats. A missing asset leaves its pointer null and is
	// reported once in LogCosmic.
	void LoadFromFixedPaths();

	// Null when the table or the row is missing. They never warn: the caller decides what a missing row means.
	const FCosmicRulesDef* FindRules() const;
	const FCivArchetypeDef* FindCiv(FName RowName) const;
	const FLogTemplateDef* FindLogTemplate(FName RowName) const;
	const FGalaxyDef* FindGalaxy(FName RowName) const;
	const FCosmicEventDef* FindEvent(FName RowName) const;
	const FTranscendentDef* FindTranscendent(FName RowName) const;
	const FHazardDef* FindHazard(FName RowName) const;
	const FMegaprojectDef* FindMegaproject(FName RowName) const;
	const FAscensionFeatDef* FindFeat(FName RowName) const;

	// The DT_Hazards row whose HazardTag is Tag exactly, and its name in OutRowName when one is asked for (NAME_None when
	// there is no such row). Rows are looked at in lexical order, so a table that broke the uniqueness lint answers the
	// same in every process.
	const FHazardDef* FindHazardByTag(FGameplayTag Tag, FName* OutRowName = nullptr) const;

	// Row names of DT_Civs in lexical order. Draws over the rows use this order, never the table's own (section 9.2).
	TArray<FName> GetCivRowNames() const;

	// Row names of DT_Events in lexical order: the order in which the event slots look at rows (section 7.5).
	TArray<FName> GetEventRowNames() const;

	// Row names of DT_Transcendents in lexical order: the order of the roster draw in NewGalaxy (section 4.2, 9.2).
	TArray<FName> GetTranscendentRowNames() const;

	// Row names of DT_Hazards in lexical order: the order of the shares of one spawn draw and of the capital rule
	// (section 7.4, 9.2).
	TArray<FName> GetHazardRowNames() const;

	// Row names of DT_Feats in lexical order: the order in which the feat judgment and the AI look at rows (section 7.5).
	TArray<FName> GetFeatRowNames() const;

	// Row names of DT_Galaxies in lexical order: the order the universe level draws and labels the named galaxies in
	// (CosmicUniverseLook::MarksOf - a table's own order is insertion order). The simulation never walks the table.
	TArray<FName> GetGalaxyRowNames() const;

	// Whether some DT_Hazards row lists the DT_Megaprojects row RowName in RulerResolutions: the project answers a hazard
	// and runs in the response slot (section 5.7).
	bool IsHazardAnswer(FName RowName) const;
};
