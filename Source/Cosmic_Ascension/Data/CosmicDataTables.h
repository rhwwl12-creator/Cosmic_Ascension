// Cosmic Ascension - Data layer. Design: guideline.md section 7.3, 10.1.
// The definition tables the Core Logic reads. They are found by fixed asset path under /Game/CA/Data; there is no
// settings asset. Tests replace them through UGalaxySimulationSubsystem::SetTables.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Data/CosmicDefinitionRows.h"
#include "CosmicDataTables.generated.h"

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

	// DT_Events: one row per event (section 7.3). A missing table means no rows; the event slots still draw.
	// Not loaded from a fixed path yet: the asset arrives with the first row, tests inject the table until then.
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> Events;

	// Loads every table from its fixed path. A missing asset leaves its pointer null and is reported once in LogCosmic.
	void LoadFromFixedPaths();

	// Null when the table or the row is missing. They never warn: the caller decides what a missing row means.
	const FCosmicRulesDef* FindRules() const;
	const FCivArchetypeDef* FindCiv(FName RowName) const;
	const FLogTemplateDef* FindLogTemplate(FName RowName) const;
	const FCosmicEventDef* FindEvent(FName RowName) const;

	// Row names of DT_Civs in lexical order. Draws over the rows use this order, never the table's own (section 9.2).
	TArray<FName> GetCivRowNames() const;

	// Row names of DT_Events in lexical order: the order in which the event slots look at rows (section 7.5).
	TArray<FName> GetEventRowNames() const;
};
