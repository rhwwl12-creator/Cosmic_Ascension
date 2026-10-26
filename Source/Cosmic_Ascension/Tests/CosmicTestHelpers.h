// Cosmic Ascension - shared helpers of the automation tests (guideline.md section 9.3).

#pragma once

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Core/CosmicIDs.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Events/CosmicEventSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Log/CosmicLogSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Save/CosmicSaveSubsystem.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicTestProbe.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "World/CosmicGameMode.h"

#include <initializer_list>

namespace CosmicTests
{
	// The tests run with -game, so ClientContext is required (CLAUDE.md).
	inline constexpr EAutomationTestFlags Flags = EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter;

	// Vertical slice size: 0 reads the home galaxy row (60 and 12, section 3.1). The performance budget size: the
	// Early Access start values (section 9.4).
	inline constexpr int32 SliceSystems = 0;
	inline constexpr int32 SliceCivs = 0;
	inline constexpr int32 BudgetSystems = 300;
	inline constexpr int32 BudgetCivs = 48;

	// The systems of State but the sanctuary that NewGalaxy puts at the end of the array (rule 11 of the generation): the
	// living worlds a count of NewGalaxy asks for.
	inline int32 LivingSystems(const FGalaxyState& State)
	{
		return State.Systems.Num() - ((!State.Systems.IsEmpty() && CosmicState::IsSanctuary(State.Systems.Last().SystemID)) ? 1 : 0);
	}

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

	// The simulation of the running game, paused so that frame ticks cannot interleave with a test, with the log in its
	// tell-all mode (every line is stored, as the tests expect - section 8.1; a test that turns the mode off turns it back
	// on when it ends), and the provisional end of the player controller switched on and forgotten (MVP item 4: a run a test
	// before ended would hold the speed at 0 and refuse the commands), and the save layer writing under
	// Saved/SaveGames/Automation/ only (MVP item 5: no test slot reaches the game's continue). The tests replace its galaxy
	// and leave it paused.
	// Null (with a test error) when there is no GameInstance.
	inline UGalaxySimulationSubsystem* AcquireSim(FAutomationTestBase& Test)
	{
		UGameInstance* GameInstance = FindGameInstance();
		UGalaxySimulationSubsystem* Sim = GameInstance ? GameInstance->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
		if (!Sim)
		{
			Test.AddError(TEXT("No GameInstance: run the automation tests with -game (see CLAUDE.md)."));
			return nullptr;
		}
		if (ACosmicPlayerController* Controller = Cast<ACosmicPlayerController>(GameInstance->GetWorld() ? GameInstance->GetWorld()->GetFirstPlayerController() : nullptr))
		{
			Controller->SetProvisionalEndEnabled(true);
			Controller->ClearRunEnd();
		}
		if (UCosmicSaveSubsystem* Save = GameInstance->GetSubsystem<UCosmicSaveSubsystem>())
		{
			Save->PrepareForTests();
		}
		Sim->SetTimeScale(0.f);
		if (UCosmicLogSubsystem* Log = GameInstance->GetSubsystem<UCosmicLogSubsystem>())
		{
			Log->SetTellAll(true);
		}
		return Sim;
	}

	// A probe on both delegates for the lifetime of the object.
	struct FScopedProbe
	{
		FScopedProbe(UGalaxySimulationSubsystem& Sim)
			: Log(Sim.GetGameInstance()->GetSubsystem<UCosmicLogSubsystem>())
			, Events(Sim.GetGameInstance()->GetSubsystem<UCosmicEventSubsystem>())
			, Probe(NewObject<UCosmicTestProbe>())
		{
			Probe->AddToRoot();
			Log->OnLogEntry.AddDynamic(Probe, &UCosmicTestProbe::HandleLogEntry);
			Events->OnEventFired.AddDynamic(Probe, &UCosmicTestProbe::HandleEventFired);
		}

		~FScopedProbe()
		{
			Log->OnLogEntry.RemoveDynamic(Probe, &UCosmicTestProbe::HandleLogEntry);
			Events->OnEventFired.RemoveDynamic(Probe, &UCosmicTestProbe::HandleEventFired);
			Probe->RemoveFromRoot();
		}

		UCosmicLogSubsystem* Log;
		UCosmicEventSubsystem* Events;
		UCosmicTestProbe* Probe;
	};

	// The construction row a test may put in a slot under WithoutProjects.
	inline FName StandingProject()
	{
		return FName(TEXT("project_test"));
	}

	// An empty table of a row struct in the transient package: a test fills it and hands it in through SetTables.
	inline UDataTable* NewTable(UScriptStruct* RowStruct)
	{
		UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
		Table->RowStruct = RowStruct;
		return Table;
	}

	// The rules without the inherited edict (B2 header section 2.7): a board that reads Stability or income as constants
	// lays its peoples without the game's StartingEdict; a test of the inherited edict keeps it.
	inline FCosmicRulesDef WithoutStartingEdict(const FCosmicRulesDef& Rules)
	{
		FCosmicRulesDef Out = Rules;
		Out.StartingEdict = NAME_None;
		return Out;
	}

	// The player's first act in a game of the game's data (B2): the inherited edict revoked, as the edict hint teaches - for a
	// test that plays the game's path (a climb, a chronicle, a reign) rather than the neglect the inherited edict ends in
	// a deposition. True when there was one to revoke and the command took it.
	inline bool RevokeInheritedEdict(UGalaxySimulationSubsystem& Sim)
	{
		UPlayerRoleSubsystem* Role = Sim.GetGameInstance() ? Sim.GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
		const FName Inherited = Sim.GetRules().StartingEdict;
		return Role && !Inherited.IsNone() && Role->RevokeEdict(Inherited);
	}

	// The tables with DT_Rules replaced by one row, everything else as given - the game's projects and feats too: a test
	// that ticks a board takes them out with WithoutProjects (below).
	inline FCosmicDataTables WithRules(const FCosmicDataTables& Base, const FCosmicRulesDef& RulesRow)
	{
		UDataTable* RulesTable = NewTable(FCosmicRulesDef::StaticStruct());
		RulesTable->AddRow(CosmicRows::Rules, RulesRow);
		FCosmicDataTables Tables = Base;
		Tables.Rules = RulesTable;
		return Tables;
	}

	// The tables with diplomacy and conflict off (B3 header section 6): the rules of Base (the C++ defaults without a rules
	// row) with no drift, no fleet and no spread, and DT_Encounters without the first contact card - for a test that reads
	// Stability, income or ownership as constants on a board whose peoples border each other (Board puts peoples 1 and 2 on
	// lane neighbours). Everything else as given.
	inline FCosmicDataTables WithoutDiplomacy(const FCosmicDataTables& Base)
	{
		const FCosmicRulesDef* GameRules = Base.FindRules();
		FCosmicRulesDef Off = GameRules ? *GameRules : FCosmicRulesDef();
		Off.RelationDriftRate = 0.f;
		Off.FleetPerSettledByTier = { 0.f, 0.f, 0.f, 0.f, 0.f };
		Off.PlagueSpreadMultiplier = 1.f;
		FCosmicDataTables Tables = WithRules(Base, Off);
		// The row of the first contact card (B3 section 2.9).
		const FName FirstContact = CosmicRows::PetitionFirstContact;
		if (Base.Encounters && Base.Encounters->GetRowMap().Contains(FirstContact))
		{
			UDataTable* Encounters = NewTable(FEncounterDef::StaticStruct());
			for (const TPair<FName, uint8*>& Row : Base.Encounters->GetRowMap())
			{
				if (Row.Key != FirstContact)
				{
					Encounters->AddRow(Row.Key, *reinterpret_cast<const FEncounterDef*>(Row.Value));
				}
			}
			Tables.Encounters = Encounters;
		}
		return Tables;
	}

	// A copy of Table with the rows of Names taken out (the struct T is the table's own); null stays null.
	template <typename T>
	UDataTable* CopyWithout(const UDataTable* Table, std::initializer_list<FName> Names)
	{
		if (!Table)
		{
			return nullptr;
		}
		UDataTable* Copy = NewTable(T::StaticStruct());
		for (const TPair<FName, uint8*>& Row : Table->GetRowMap())
		{
			bool bOut = false;
			for (const FName& Name : Names)
			{
				bOut |= Name == Row.Key;
			}
			if (!bOut)
			{
				Copy->AddRow(Row.Key, *reinterpret_cast<const T*>(Row.Value));
			}
		}
		return Copy;
	}

	// The tables without B4's pressure data (B4 header section 6, the data commit [B-4d2]): DT_Encounters without the two
	// pressure cards, DT_Events without the vigil's draw row, DT_Edicts without the seventh edict, and every transcendent's
	// ClaimTemplates empty - for a test that reads the game's tables as a board of its own and lays rows of these kinds
	// itself (the theodicy's boards of Data.Integrity). Everything else as given (WithoutDiplomacy's form).
	inline FCosmicDataTables WithoutPressure(const FCosmicDataTables& Base)
	{
		FCosmicDataTables Tables = Base;
		Tables.Encounters = CopyWithout<FEncounterDef>(Base.Encounters, { CosmicRows::PetitionPressureClaim, CosmicRows::PetitionPressureMeasured });
		Tables.Events = CopyWithout<FCosmicEventDef>(Base.Events, { CosmicRows::EventClaimEased });
		Tables.Edicts = CopyWithout<FEdictDef>(Base.Edicts, { FName(TEXT("edict_against_silence")) });
		if (UDataTable* Transcendents = CopyWithout<FTranscendentDef>(Base.Transcendents, {}))
		{
			for (const TPair<FName, uint8*>& Row : Transcendents->GetRowMap())
			{
				reinterpret_cast<FTranscendentDef*>(Row.Value)->ClaimTemplates.Reset();
			}
			Tables.Transcendents = Transcendents;
		}
		return Tables;
	}

	// The tables with DT_Galaxies replaced, everything else as given (as WithTranscendents). A test that hands it to SetTables
	// puts a copy of the game's home row in it: a table without the home row is an error line and the C++ defaults (7.4).
	inline FCosmicDataTables WithGalaxies(const FCosmicDataTables& Base, UDataTable* Galaxies)
	{
		FCosmicDataTables Tables = Base;
		Tables.Galaxies = Galaxies;
		return Tables;
	}

	// A DT_Galaxies row as a test writes it: display only (the four simulation values 0, BridgeTicks 0), DisplayName and Kind
	// (requested by name), RadiusLy and MapPosition as given; the texts and pools empty.
	inline FGalaxyDef GalaxyRowOf(const TCHAR* DisplayName, const TCHAR* Kind, int32 RadiusLy, const FVector2D& MapPosition)
	{
		FGalaxyDef Row;
		Row.DisplayName = FText::AsCultureInvariant(DisplayName);
		Row.Kind = FGameplayTag::RequestGameplayTag(FName(Kind), false);
		Row.LivingWorlds = 0;
		Row.Peoples = 0;
		Row.AIFourthRungCap = 0;
		Row.HomeRadiusShare = 0.f;
		Row.RadiusLy = RadiusLy;
		Row.BridgeTicks = 0;
		Row.MapPosition = MapPosition;
		return Row;
	}

	// The tables with DT_Transcendents replaced, everything else as given.
	inline FCosmicDataTables WithTranscendents(const FCosmicDataTables& Base, UDataTable* Transcendents)
	{
		FCosmicDataTables Tables = Base;
		Tables.Transcendents = Transcendents;
		return Tables;
	}

	// A transcendent instance put in the state by the test (clear State.Transcendents first: NewGalaxy brings the game's
	// roster - CLAUDE.md), with its sphere.
	inline FTranscendentState& AddInstance(FGalaxyState& State, FName TranscendentID, const TArray<FName>& Sphere)
	{
		FTranscendentState& Transcendent = State.Transcendents.AddDefaulted_GetRef();
		Transcendent.TranscendentID = TranscendentID;
		Transcendent.InfluencedSystems = Sphere;
		return Transcendent;
	}

	// The tables handed in without the game's projects and feats (section 5.7). With them every AI people of a board would
	// start the construction its next feat names on the first tick and pay for it from the stockpiles a test reasons
	// about, on values Phase 5 changes. DT_Megaprojects keeps two rows no stockpile of a test can pay: StandingProject,
	// which a test may put in a construction slot (step (a) of the crisis stage leaves it standing, so only a world
	// function empties the slot), and project_settle (a row the code calls by name - Tier 1 and unpaid: under the game's
	// realm caps an AI people of a board that may settle still draws its settlement number and starts a settlement that
	// never completes; a test that must not see that sets RealmCapByTier in its own rules). DT_Feats is empty. A test of
	// the projects brings its own rows.
	inline FCosmicDataTables WithoutProjects(const FCosmicDataTables& Base)
	{
		FMegaprojectDef Unpaid;
		Unpaid.DisplayName = FText::AsCultureInvariant(TEXT("<unpaid>"));
		Unpaid.CostPerTick.Energy = 1.0e9f;
		Unpaid.RequiredProgress = 1.0e9f;
		UDataTable* Projects = NewObject<UDataTable>(GetTransientPackage());
		Projects->RowStruct = FMegaprojectDef::StaticStruct();
		Projects->AddRow(StandingProject(), Unpaid);
		Projects->AddRow(CosmicRows::ProjectSettle, Unpaid);
		UDataTable* Feats = NewObject<UDataTable>(GetTransientPackage());
		Feats->RowStruct = FAscensionFeatDef::StaticStruct();
		FCosmicDataTables Tables = Base;
		Tables.Megaprojects = Projects;
		Tables.Feats = Feats;
		return Tables;
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

	// Every system known and every people met, appended after what Knowing holds, in array order, at the current tick (the
	// ruled people of the first life stays first in the meeting order): for a test that draws or shows a whole board as
	// before Phase 3 item 8. Writes the state only - no NoteMet, no line, no draw. The state hash moves (Knowing is in it);
	// the world hash does not.
	inline void KnowAll(FGalaxyState& State)
	{
		auto Append = [&State](TArray<FKnownMark>& Marks, FName ID)
		{
			if (!Marks.ContainsByPredicate([ID](const FKnownMark& Mark) { return Mark.ID == ID; }))
			{
				FKnownMark& Mark = Marks.AddDefaulted_GetRef();
				Mark.ID = ID;
				Mark.Tick = State.Tick;
			}
		};
		for (const FStarSystemState& System : State.Systems)
		{
			Append(State.Knowing.KnownSystems, System.SystemID);
		}
		for (const FCivilizationState& Civ : State.Civs)
		{
			Append(State.Knowing.MetCivs, Civ.CivID);
		}
	}

	// Knowing as a test sets it: exactly these systems (indices into Systems) known and these peoples (indices into Civs)
	// met, in the order given (the meeting order of the palette), at the current tick. FirstContactCivID,
	// PendingContactCivID and FarLineUntilTick stay.
	inline void SetKnowing(FGalaxyState& State, std::initializer_list<int32> SystemIndices, std::initializer_list<int32> CivIndices)
	{
		State.Knowing.KnownSystems.Reset();
		State.Knowing.MetCivs.Reset();
		for (const int32 SystemIndex : SystemIndices)
		{
			FKnownMark& Mark = State.Knowing.KnownSystems.AddDefaulted_GetRef();
			Mark.ID = State.Systems.IsValidIndex(SystemIndex) ? State.Systems[SystemIndex].SystemID : NAME_None;
			Mark.Tick = State.Tick;
		}
		for (const int32 CivIndex : CivIndices)
		{
			FKnownMark& Mark = State.Knowing.MetCivs.AddDefaulted_GetRef();
			Mark.ID = State.Civs.IsValidIndex(CivIndex) ? State.Civs[CivIndex].CivID : NAME_None;
			Mark.Tick = State.Tick;
		}
	}

	// Everything the text blocks of a widget's own tree show, one block per line.
	inline FString ReadTexts(const UUserWidget& Widget)
	{
		FString Shown;
		if (Widget.WidgetTree)
		{
			Widget.WidgetTree->ForEachWidget([&Shown](UWidget* Child)
			{
				if (const UTextBlock* Block = Cast<UTextBlock>(Child))
				{
					Shown += Block->GetText().ToString();
					Shown += TEXT("\n");
				}
			});
		}
		return Shown;
	}
}

#endif // WITH_DEV_AUTOMATION_TESTS
