// Cosmic Ascension - automation tests of the death pipeline, the interregnum's countdown and the succession (milestone B
// item B5, [B-5b]): the death of a wanderer and of a ruler at the tenth stage, the lock of every throne on every end of a
// reign, the leap's one number, the interregnum without a vessel, the hard reset of a succession and the first tick of a
// new life; and ([B-5d1]) the tenth stage's two lines, the death line and the inauguration line. Design:
// docs/60_DECISIONS/headers/B5_death_succession_two_books.md section 2.1-2.6, 3.3 and 6.

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Events/CosmicEventSubsystem.h"
#include "Events/CosmicPetitions.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Log/CosmicChronicle.h"
#include "Math/RandomStream.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Sim/CosmicEvalContext.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/CosmicSuccession.h"
#include "Sim/CosmicWanderer.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Templates/Function.h"
#include "Tests/CosmicB6bRowNames.h"
#include "Tests/CosmicTestHelpers.h"
#include "Tests/CosmicTestProbe.h"

#include <initializer_list>
#include <limits>

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicDeathTests
{
	FGameplayTag Tag(const TCHAR* Name)
	{
		return FGameplayTag::RequestGameplayTag(FName(Name));
	}

	FCosmicCondition Cond(EConditionType Type, const TCHAR* TagName = nullptr, const TCHAR* Key = nullptr)
	{
		FCosmicCondition Condition;
		Condition.Type = Type;
		Condition.Tag = TagName ? Tag(TagName) : FGameplayTag();
		Condition.Key = Key ? FName(Key) : FName();
		return Condition;
	}

	// The rows and names of these tests. A death's source row is a hazard row of the game (the quiet copy holds it): the death
	// line names it ({Hazard} - [B-5d1]). These records are made by hand, on the precursor's row - which kills nobody in the
	// game's data: [B-5d2] opens the AI awakening's own expiry alone, and Calling.LethalSource pins that one.
	const FName CardRow(TEXT("enc_test_death_tend"));
	const FName DeathRow(TEXT("crisis_pre_supernova"));
	const FName LeftEdict(TEXT("edict_test_left"));
	const FName ClimbRow(TEXT("feat_test_leap_climb"));
	const FName TwinA(TEXT("arch_test_twin_a"));
	const FName TwinB(TEXT("arch_test_twin_b"));

	// A Standing wanderer's card on a plague where the vessel stands (the B1 form of CosmicWandererTests), two plain choices.
	UDataTable* PlagueCard()
	{
		FEncounterChoice Stay;
		Stay.Label = FText::AsCultureInvariant(TEXT("stay"));
		Stay.CheckStat = ECosmicCheckStat::NoCheck;
		Stay.SuccessText = FText::AsCultureInvariant(TEXT("stayed."));
		FEncounterChoice Leave = Stay;
		Leave.Label = FText::AsCultureInvariant(TEXT("leave"));
		Leave.SuccessText = FText::AsCultureInvariant(TEXT("left."));
		FEncounterDef Row;
		Row.Title = FText::AsCultureInvariant(TEXT("card"));
		Row.Body = FText::AsCultureInvariant(TEXT("{System} waits."));
		Row.Trigger = ECosmicEventTrigger::Standing;
		Row.Subject = ECosmicEventSubject::VesselSystem;
		Row.Conditions = { Cond(EConditionType::PlayerRoleIs, nullptr, TEXT("Wanderer")), Cond(EConditionType::PlayerInSystemWithHazard, TEXT("Hazard.Plague")) };
		Row.Choices = { Stay, Leave };
		Row.BaseWeight = 1.f;
		UDataTable* Table = CosmicTests::NewTable(FEncounterDef::StaticStruct());
		Table->AddRow(CardRow, Row);
		return Table;
	}

	// A rung of the ruler's path a people climbs while a plague stands on one of its settled systems, at no price.
	FAscensionFeatDef ClimbFeat()
	{
		FAscensionFeatDef Row;
		Row.Title = FText::AsCultureInvariant(TEXT("<climb>"));
		Row.Path = EPlayerRole::Ruler;
		Row.RequiredTier = EAscensionTier::Tier1_Planetary;
		Row.TargetTier = EAscensionTier::Tier2_Stellar;
		Row.CompletionConditions = { Cond(EConditionType::SystemHasHazard, TEXT("Hazard.Plague")) };
		Row.LogTemplates = { FText::AsCultureInvariant(TEXT("{Civ} climbed one")), FText::AsCultureInvariant(TEXT("{Civ} climbed two")) };
		return Row;
	}

	// The game's hazard rows that no draw places: what a test puts in a slot counts down as in the game, and nothing else
	// arrives.
	UDataTable* QuietHazards(const FCosmicDataTables& Original)
	{
		UDataTable* Table = CosmicTests::NewTable(FHazardDef::StaticStruct());
		for (const FName RowName : Original.Hazards ? Original.Hazards->GetRowNames() : TArray<FName>())
		{
			if (const FHazardDef* Row = Original.Hazards->FindRow<FHazardDef>(RowName, TEXT("CosmicDeathTests")))
			{
				FHazardDef Quiet = *Row;
				Quiet.SpawnChancePerTick = 0.f;
				Table->AddRow(RowName, Quiet);
			}
		}
		return Table;
	}

	// The game's DT_Edicts and one more row with no cost and no effect: an AI people revokes it at once (CosmicEdicts::
	// AIRevokesOf), a ruled people keeps it - what the AI stage does to a people shows on it.
	UDataTable* EdictsWithLeft(const FCosmicDataTables& Original)
	{
		UDataTable* Table = CosmicTests::CopyWithout<FEdictDef>(Original.Edicts, {});
		if (!Table)
		{
			Table = CosmicTests::NewTable(FEdictDef::StaticStruct());
		}
		FEdictDef Row;
		Row.DisplayName = FText::AsCultureInvariant(TEXT("<left>"));
		Table->AddRow(LeftEdict, Row);
		return Table;
	}

	// The game's DT_Civs and two rows no people is drawn from (Weight 0): copies of arch_default whose banks hold the same
	// two names, made apart (two archetypes sharing a bank - B5 decision 8).
	UDataTable* CivsWithTwins(const FCosmicDataTables& Original)
	{
		UDataTable* Table = CosmicTests::CopyWithout<FCivArchetypeDef>(Original.Civs, {});
		if (!Table)
		{
			Table = CosmicTests::NewTable(FCivArchetypeDef::StaticStruct());
		}
		const FCivArchetypeDef* Default = Original.FindCiv(CosmicRows::ArchDefault);
		FCivArchetypeDef Twin = Default ? *Default : FCivArchetypeDef();
		Twin.Weight = 0.f;
		Twin.PersonNameBank = { FText::AsCultureInvariant(TEXT("<first name>")), FText::AsCultureInvariant(TEXT("<second name>")) };
		Table->AddRow(TwinA, Twin);
		Twin.PersonNameBank = { FText::AsCultureInvariant(TEXT("<first name>")), FText::AsCultureInvariant(TEXT("<second name>")) };
		Table->AddRow(TwinB, Twin);
		return Table;
	}

	// The game's tables with quiet hazards, no projects or feats, the cards given (none when null), the left edict, the twin
	// archetypes, and rules without the inherited edict, the event slot, the first crisis, the periodic petition, drift,
	// fleets, spread and settlement; Tune changes the rules last. The leap knobs are the game's unless Tune sets them.
	FCosmicDataTables TablesWith(const FCosmicDataTables& Original, UDataTable* Encounters, TFunctionRef<void(FCosmicRulesDef&)> Tune)
	{
		const FCosmicRulesDef* GameRules = Original.FindRules();
		FCosmicRulesDef Rules = CosmicTests::WithoutStartingEdict(GameRules ? *GameRules : FCosmicRulesDef());
		Rules.EventChancePerSlot = 0.f;
		Rules.FirstCrisisTick = 0;
		Rules.PetitionPeriodTicks = 100000;
		Rules.RelationDriftRate = 0.f;
		Rules.FleetPerSettledByTier = { 0.f, 0.f, 0.f, 0.f, 0.f };
		Rules.PlagueSpreadMultiplier = 1.f;
		Rules.RealmCapByTier = { 1, 1, 1, 1, 1 };
		Tune(Rules);
		FCosmicDataTables Tables = CosmicTests::WithoutProjects(CosmicTests::WithRules(Original, Rules));
		Tables.Encounters = Encounters ? Encounters : CosmicTests::NewTable(FEncounterDef::StaticStruct());
		Tables.Hazards = QuietHazards(Original);
		Tables.Edicts = EdictsWithLeft(Original);
		Tables.Civs = CivsWithTwins(Original);
		return Tables;
	}

	// The board of these tests: the game's galaxy of seed 42 with twelve systems on the chain of CosmicTests::WireChain
	// (0-1-...-9, 4-10, 10-11). The vessel rules people 0 from system 0. With four peoples people 1 holds 5 and 6, people 2
	// holds 8 and people 3 holds 11; with two, people 1 holds 5 alone (the one candidate of a death in system 0). The rest is
	// wild. Every people on the first rung at Stability 0.8, with no reverence, lock, notable or edict; no hazard, project,
	// transcendent or flag; the calling on the first rung knows system 0 and has met people 0 (no first contact); the tick is
	// 400, past the birth protection of the first life (born at 0). Null when the galaxy does not give that.
	FGalaxyState* LayBoard(UGalaxySimulationSubsystem& Sim, const FCosmicDataTables& Tables, int32 Peoples = 4)
	{
		Sim.SetTables(Tables);
		Sim.NewGalaxy(42, 12, Peoples);
		FGalaxyState& State = Sim.GetMutableState();
		if (!CosmicTests::WireChain(State) || State.Civs.Num() != Peoples || State.Player.RuledCivID != State.Civs[0].CivID)
		{
			return nullptr;
		}
		CosmicTests::ClearOwners(State);
		CosmicTests::Give(State, 0, { 0 });
		if (Peoples == 2)
		{
			CosmicTests::Give(State, 1, { 5 });
		}
		else
		{
			CosmicTests::Give(State, 1, { 5, 6 });
			CosmicTests::Give(State, 2, { 8 });
			CosmicTests::Give(State, 3, { 11 });
		}
		State.Transcendents.Reset();
		State.Flags.Reset();
		State.Log.Reset();
		for (FStarSystemState& System : State.Systems)
		{
			CosmicHazards::EmptyHazardSlot(System);
			System.ActiveMegaproject = NAME_None;
			System.MegaprojectProgress = 0.f;
		}
		for (FCivilizationState& Civ : State.Civs)
		{
			Civ.Tier = EAscensionTier::Tier1_Planetary;
			Civ.Stability = 0.8f;
			Civ.LowStabilityTicks = 0;
			Civ.ReverenceToPlayer = 0.f;
			Civ.ThroneLockedUntilTick = 0;
			Civ.Notables.Reset();
			Civ.ActiveEdicts.Reset();
		}
		State.Calling.PersonalTier = EAscensionTier::Tier1_Planetary;
		State.Player.CurrentSystemID = State.Systems[0].SystemID;
		State.Knowing = FKnowingState();
		CosmicTests::SetKnowing(State, { 0 }, { 0 });
		State.Tick = 400;
		Sim.SyncRngState();
		return &State;
	}

	// The vessel leaves its throne where it stands (BecomeWanderer as a deposition leaves it - no draw): a wanderer in
	// SystemIndex that knows it and the lanes around.
	void Walk(UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role, int32 SystemIndex)
	{
		Sim.GetMutableState().Player.CurrentSystemID = Sim.GetState().Systems[SystemIndex].SystemID;
		Role.BecomeWanderer(Tag(TEXT("AI.Profile.Conservative")), Tag(TEXT("Reign.Deposed")));
		Sim.SyncRngState();
	}

	// A death recorded on the live state the way Apply leaves one (a hazard's expiry): the tenth stage of the next tick runs it.
	void RecordDeath(FGalaxyState& State)
	{
		State.Calling.PendingEffect = Tag(TEXT("Effect.KillPlayer"));
		State.Calling.PendingCause = Tag(TEXT("Death.Hazard"));
		State.Calling.PendingSourceRow = DeathRow;
	}

	// A role effect of Type with the cause Cause about the people CivIndex and no system, through Apply as an event's effect
	// would record it, with SourceRow as the row behind it. Whether it was recorded.
	bool RecordByApply(UGalaxySimulationSubsystem& Sim, EEffectType Type, const TCHAR* Cause, int32 CivIndex, FName SourceRow)
	{
		FCosmicEvalContext Ctx = Sim.MakeEvalContext();
		Ctx.CivIndex = CivIndex;
		Ctx.SystemIndex = INDEX_NONE;
		Ctx.SourceRow = SourceRow;
		FCosmicEffect Effect;
		Effect.Type = Type;
		Effect.Tag = Tag(Cause);
		return UCosmicEventSubsystem::Apply(Effect, Ctx);
	}

	// How many numbers a stream drew between two of its states. INDEX_NONE beyond a bound no tick of a test reaches.
	int32 DrawsBetween(int32 FromSeed, int32 ToSeed)
	{
		FRandomStream Stream(FromSeed);
		for (int32 Count = 0; Count <= 100000; ++Count)
		{
			if (Stream.GetCurrentSeed() == ToSeed)
			{
				return Count;
			}
			Stream.GetFraction();
		}
		return INDEX_NONE;
	}

	// The N-th number (from 1) a stream at Seed draws.
	float NthFraction(int32 Seed, int32 N)
	{
		FRandomStream Stream(Seed);
		float Value = -1.f;
		for (int32 At = 0; At < N; ++At)
		{
			Value = Stream.GetFraction();
		}
		return Value;
	}

	// One tick from Quiet without a death, then the same tick from Quiet with the death Record makes (the live state is the
	// death's afterwards). A death acts at the tenth stage alone - the last - so the stream at the end of the tick without it
	// is where the death's tick starts drawing for it: ExtraDraws counts the death's numbers, drawn from SeedWithout on.
	struct FDeathTick
	{
		int32 ExtraDraws = INDEX_NONE;
		int32 SeedWithout = 0;
		FGalaxyState Without;
	};

	FDeathTick StepDeath(UGalaxySimulationSubsystem& Sim, const FGalaxyState& Quiet, TFunctionRef<void()> Record)
	{
		FDeathTick Out;
		Sim.LoadState(Quiet);
		Sim.StepOnce();
		Out.Without = Sim.GetState();
		Out.SeedWithout = Out.Without.RngState;
		Sim.LoadState(Quiet);
		Record();
		Sim.StepOnce();
		Out.ExtraDraws = DrawsBetween(Out.SeedWithout, Sim.GetState().RngState);
		return Out;
	}

	bool SameMarks(const TArray<FKnownMark>& A, const TArray<FKnownMark>& B)
	{
		if (A.Num() != B.Num())
		{
			return false;
		}
		for (int32 At = 0; At < A.Num(); ++At)
		{
			if (A[At].ID != B[At].ID || A[At].Tick != B[At].Tick)
			{
				return false;
			}
		}
		return true;
	}

	// Whether After begins with Before: the same marks in the same order, ticks included.
	bool IsPrefix(const TArray<FKnownMark>& Before, const TArray<FKnownMark>& After)
	{
		if (Before.Num() > After.Num())
		{
			return false;
		}
		for (int32 At = 0; At < Before.Num(); ++At)
		{
			if (Before[At].ID != After[At].ID || Before[At].Tick != After[At].Tick)
			{
				return false;
			}
		}
		return true;
	}

	// The three arrays of Knowing and the first contact (docs/50 Phase 4b: the knowing a leap must not move).
	bool SameKnowing(const FKnowingState& A, const FKnowingState& B)
	{
		return SameMarks(A.KnownSystems, B.KnownSystems) && SameMarks(A.MetCivs, B.MetCivs) && SameMarks(A.MetGalaxies, B.MetGalaxies)
			&& A.FirstContactCivID == B.FirstContactCivID;
	}

	bool Knows(const FGalaxyState& State, int32 SystemIndex)
	{
		const FName ID = State.Systems[SystemIndex].SystemID;
		return State.Knowing.KnownSystems.ContainsByPredicate([ID](const FKnownMark& Mark) { return Mark.ID == ID; });
	}

	bool HasMet(const FGalaxyState& State, FName CivID)
	{
		return State.Knowing.MetCivs.ContainsByPredicate([CivID](const FKnownMark& Mark) { return Mark.ID == CivID; });
	}

	// Every people's notables, all five fields, in the same order (the peoples too).
	bool SameNotables(const FGalaxyState& A, const FGalaxyState& B)
	{
		if (A.Civs.Num() != B.Civs.Num())
		{
			return false;
		}
		for (int32 CivIndex = 0; CivIndex < A.Civs.Num(); ++CivIndex)
		{
			const TArray<FNotableRecord>& X = A.Civs[CivIndex].Notables;
			const TArray<FNotableRecord>& Y = B.Civs[CivIndex].Notables;
			if (A.Civs[CivIndex].CivID != B.Civs[CivIndex].CivID || X.Num() != Y.Num())
			{
				return false;
			}
			for (int32 At = 0; At < X.Num(); ++At)
			{
				if (X[At].Role != Y[At].Role || X[At].NameIndex != Y[At].NameIndex || X[At].FirstTick != Y[At].FirstTick || X[At].LastTick != Y[At].LastTick
					|| X[At].SourceRow != Y[At].SourceRow)
				{
					return false;
				}
			}
		}
		return true;
	}

	int32 OpenRecords(const FCivilizationState& Civ, const TCHAR* Role)
	{
		int32 Count = 0;
		for (const FNotableRecord& Record : Civ.Notables)
		{
			Count += (Record.LastTick == 0 && Record.Role == Tag(Role)) ? 1 : 0;
		}
		return Count;
	}

	// Whether every people's throne is locked until Until or later (false with no people).
	bool EveryThroneLockedFrom(const FGalaxyState& State, int64 Until)
	{
		bool bEvery = !State.Civs.IsEmpty();
		for (const FCivilizationState& Civ : State.Civs)
		{
			bEvery &= Civ.ThroneLockedUntilTick >= Until;
		}
		return bEvery;
	}

	bool NoReverence(const FGalaxyState& State)
	{
		bool bNone = true;
		for (const FCivilizationState& Civ : State.Civs)
		{
			bNone &= Civ.ReverenceToPlayer == 0.f;
		}
		return bNone;
	}

	// The source string of a life's name: Bank[NameIndex % Bank.Num()] of its birth archetype, or of arch_default when that
	// bank is empty (Install's rule). Empty when neither has a bank.
	FString NameSourceOf(const FCosmicDataTables& Tables, const FLifeRecord& Life)
	{
		const FCivArchetypeDef* Archetype = Tables.FindCiv(Life.BirthArchetypeRow);
		if (!Archetype || Archetype->PersonNameBank.IsEmpty())
		{
			Archetype = Tables.FindCiv(CosmicRows::ArchDefault);
		}
		if (!Archetype || Archetype->PersonNameBank.IsEmpty() || Life.NameIndex < 0)
		{
			return FString();
		}
		const FString* Source = FTextInspector::GetSourceString(Archetype->PersonNameBank[Life.NameIndex % Archetype->PersonNameBank.Num()]);
		return Source ? *Source : FString();
	}

	// What one run of Calling.FirstTickOfLife saw.
	struct FLifeRun
	{
		bool bDied = false;
		bool bBorn = false;
		bool bFirstInLeap = true;
		bool bFirstAtBirth = false;
		bool bFirstAfter = true;
		bool bNoCardAtBirth = false;
		bool bHazardAtBirth = false;
		FName BirthSystemID;
		FName Card;
		FName CardSystemID;
		int32 Seed = 0;
		uint32 WorldHash = 0;
		FGalaxyState AtBirth;
	};

	// [B-5d1] The entries of the log whose row is Row: how many, and the newest (null for none).
	int32 CountLinesOf(const FGalaxyState& State, FName Row)
	{
		int32 Count = 0;
		for (const FCosmicLogEntry& Entry : State.Log)
		{
			Count += Entry.SourceRow == Row ? 1 : 0;
		}
		return Count;
	}

	const FCosmicLogEntry* NewestLineOf(const FGalaxyState& State, FName Row)
	{
		for (int32 Index = State.Log.Num() - 1; Index >= 0; --Index)
		{
			if (State.Log[Index].SourceRow == Row)
			{
				return &State.Log[Index];
			}
		}
		return nullptr;
	}

	// What a handler of the death line saw of the state when the line was told (header section 2.2 Die (b): told before what
	// the death changes). Reverence is people 0's.
	struct FAtDeathLine
	{
		bool bHeard = false;
		bool bEmbodied = false;
		bool bRules = false;
		int32 Regents = 0;
		float Reverence = -1.f;
		int64 PendingLeap = -1;
		int32 SimSeed = 0;
	};

	// StepDeath with the broadcast gate open and a handler on the death line (OnLogEntry - a handler only reads); the gate is
	// put back as it was.
	FDeathTick StepDeathHeard(UGalaxySimulationSubsystem& Sim, const FGalaxyState& Quiet, TFunctionRef<void()> Record, FAtDeathLine& OutHeard)
	{
		const bool bWasBroadcasting = Sim.IsBroadcastEnabled();
		Sim.SetBroadcastEnabled(true);
		FDeathTick Out;
		{
			CosmicTests::FScopedProbe Scoped(Sim);
			UGalaxySimulationSubsystem* Live = &Sim;
			Scoped.Probe->OnLog = [Live, &OutHeard](const FCosmicLogEntry& Entry)
			{
				if (OutHeard.bHeard || Entry.SourceRow != CosmicRows::LogDeathHazard)
				{
					return;
				}
				const FGalaxyState& State = Live->GetState();
				OutHeard.bHeard = true;
				OutHeard.bEmbodied = State.Calling.LifeState == ELifeState::Embodied;
				OutHeard.bRules = State.Player.Role == EPlayerRole::Ruler && !State.Civs.IsEmpty() && State.Civs[0].bIsPlayerRuled;
				OutHeard.Regents = State.Civs.IsEmpty() ? 0 : OpenRecords(State.Civs[0], TEXT("Notable.Regent"));
				OutHeard.Reverence = State.Civs.IsEmpty() ? -1.f : State.Civs[0].ReverenceToPlayer;
				OutHeard.PendingLeap = State.Calling.PendingLeapTicks;
				OutHeard.SimSeed = Live->GetRng().GetCurrentSeed();
			};
			Out = StepDeath(Sim, Quiet, Record);
			Scoped.Probe->OnLog = nullptr;
		}
		Sim.SetBroadcastEnabled(bWasBroadcasting);
		return Out;
	}

	// The death line of a death on the live state (section 3.3): one entry of CosmicRows::LogDeathHazard on the death's tick,
	// Log.Calling, Severity 2, told, with no focus, naming the closed life by its name and the hazard behind the death; the last
	// line of a wanderer's tick. What prefixes each assertion.
	void ExpectDeathLine(FAutomationTestBase& Test, const UGalaxySimulationSubsystem& Sim, const FLifeRecord& Life, const TCHAR* Who)
	{
		const FGalaxyState& State = Sim.GetState();
		const FCosmicLogEntry* Line = NewestLineOf(State, CosmicRows::LogDeathHazard);
		Test.TestEqual(*FString::Printf(TEXT("%s: the death tells one death line"), Who), CountLinesOf(State, CosmicRows::LogDeathHazard), 1);
		if (!Test.TestNotNull(*FString::Printf(TEXT("%s: (the death line)"), Who), Line))
		{
			return;
		}
		Test.TestEqual(*FString::Printf(TEXT("%s: on the death's tick"), Who), Line->Tick, Life.DiedTick);
		Test.TestTrue(*FString::Printf(TEXT("%s: the calling's line, Severity 2, told"), Who), Line->Category == Tag(TEXT("Log.Calling")) && Line->Severity == 2 && !Line->bUntold);
		Test.TestTrue(*FString::Printf(TEXT("%s: with no focus - it calls no system and no people"), Who), Line->FocusSystemID.IsNone() && Line->FocusCivID.IsNone());
		const FString Message = Line->Message.ToString();
		const FString Name = CosmicChronicle::VesselNameOf(Life, Sim.GetTables()).ToString();
		Test.TestTrue(*FString::Printf(TEXT("%s: it names the vessel by its name (the third person - decision 12-da)"), Who), !Name.IsEmpty() && Message.Contains(Name));
		const FHazardDef* Hazard = Sim.GetTables().FindHazard(Life.CauseSourceRow);
		Test.TestTrue(*FString::Printf(TEXT("%s: and the hazard behind the death"), Who), Hazard && !Hazard->DisplayName.IsEmpty() && Message.Contains(Hazard->DisplayName.ToString()));
	}

	// [B-5d2] The game's one lethal row (decision 6-ga) and the wanderer's card that answers it.
	const FName AwakeningRow(TEXT("crisis_ai_awakening"));
	const FName AwakeningTalk(TEXT("enc_awakening_talk"));

	// [B-5d2] What the crisis stage of one tick noted of the expiry in a system (FCosmicStageClock::FExpiry - a measurement's
	// note: the side the expiry took, and whether a KillPlayer on its system would reach the vessel then; it draws nothing and
	// writes nothing to the state). bNoted false when no expiry ran there.
	struct FExpiryNote
	{
		bool bNoted = false;
		bool bBranch = false;
		bool bReached = false;
	};

	// One tick from From with the stage clock on (off again after): the note of the expiry in SystemIndex.
	FExpiryNote StepNoted(UGalaxySimulationSubsystem& Sim, const FGalaxyState& From, int32 SystemIndex)
	{
		const FName SystemID = From.Systems[SystemIndex].SystemID;
		Sim.LoadState(From);
		Sim.GetStageClock() = FCosmicStageClock();
		Sim.GetStageClock().bOn = true;
		Sim.StepOnce();
		FExpiryNote Note;
		for (const FCosmicStageClock::FExpiry& Expiry : Sim.GetStageClock().Expiries)
		{
			if (Expiry.SystemID == SystemID)
			{
				Note.bNoted = true;
				Note.bBranch = Expiry.bBranch;
				Note.bReached = Expiry.bReached;
			}
		}
		Sim.GetStageClock() = FCosmicStageClock();
		return Note;
	}

	// StepNoted from From whose sim stream is moved on through numbers (RngState + Try, Try below 64) until the expiry's own
	// number falls on the side asked (bBranch: its branch effects): the live state is that tick's afterwards and OutBefore the
	// state it began from. bNoted false when no try took that side.
	FExpiryNote ExpireOnSide(UGalaxySimulationSubsystem& Sim, const FGalaxyState& From, int32 SystemIndex, bool bBranch, FGalaxyState& OutBefore)
	{
		for (int32 Try = 0; Try < 64; ++Try)
		{
			OutBefore = From;
			OutBefore.RngState = static_cast<int32>(static_cast<uint32>(From.RngState) + static_cast<uint32>(Try));
			const FExpiryNote Note = StepNoted(Sim, OutBefore, SystemIndex);
			if (Note.bNoted && Note.bBranch == bBranch)
			{
				return Note;
			}
		}
		return FExpiryNote();
	}

	// Whether the calling still rests on its first vessel, which never died and has no death waiting.
	bool LivesOn(const FGalaxyState& State)
	{
		return State.Calling.LifeState == ELifeState::Embodied && State.Calling.Lives.Num() == 1 && State.Calling.Lives.Last().DiedTick == 0
			&& !State.Calling.PendingEffect.IsValid() && CountLinesOf(State, CosmicRows::LogDeathHazard) == 0;
	}
}

// Pins the death of a wanderer at the tenth stage (B5 header section 2.2 Die (a), (d)-(g), section 2.5 and section 6
// Calling.DeathPipeline): no life is added or removed, the calling enters the interregnum with a leap of one number of the
// sim stream (decision 3), every people's reverence is 0 (decision 23-na), the journey and both waiting cards are gone, the
// knowing is the calling's and stays (section 2.1), the life records its end, and the countdown starts the next tick. [B-5d1]
// (section 2.2 Die (b), 3.3): one death line - Log.Calling, Severity 2, no focus, the vessel's name and the hazard's -, told
// before the death changes anything and from the tell stream alone (one number; none of the sim stream), the tick's last
// line; a cause with no row of its own (Death.Age) tells none and draws nothing. B6b [B-6b2b]: a death whose row kills the
// ruler (the extinction) tells log_death_extinction instead, with the same one number.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicCallingDeathPipelineTest, "Cosmic_Ascension.Calling.DeathPipeline", CosmicTests::Flags)
bool FCosmicCallingDeathPipelineTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDeathTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The role subsystem exists"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };

	// No look of the sight stage in these ticks: the knowing moves only by what the death does.
	FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, PlagueCard(), [](FCosmicRulesDef& Rules) { Rules.SightIntervalTicks = 100000; }));
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	const FCosmicRulesDef& Rules = Sim->GetRules();
	Walk(*Sim, *Role, 0);
	// What a death clears and what it keeps, set by hand: a journey under way, a card and a first contact card waiting, a
	// first contact made, a galaxy met, reverence at every people.
	CosmicTests::SetKnowing(*State, { 0, 1 }, { 0, 1 });
	FKnownMark& Galaxy = State->Knowing.MetGalaxies.AddDefaulted_GetRef();
	Galaxy.ID = Rules.HomeGalaxyRow;
	Galaxy.Tick = State->Tick;
	State->Knowing.FirstContactCivID = State->Civs[1].CivID;
	State->Knowing.PendingContactCivID = State->Civs[2].CivID;
	State->Knowing.PendingPetitionRow = CardRow;
	State->Knowing.PendingPetitionSystemID = State->Systems[0].SystemID;
	State->Player.TravelTargetSystemID = State->Systems[1].SystemID;
	State->Player.TravelTicksRemaining = 10;
	for (int32 CivIndex = 0; CivIndex < State->Civs.Num(); ++CivIndex)
	{
		State->Civs[CivIndex].ReverenceToPlayer = 0.2f + 0.1f * static_cast<float>(CivIndex);
	}
	// Outside the interregnum Succeed does nothing and draws nothing.
	{
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		Role->Succeed();
		TestEqual(TEXT("Succeed while a vessel lives draws nothing"), Sim->GetRng().GetCurrentSeed(), Seed);
		TestEqual(TEXT("and adds no life"), State->Calling.Lives.Num(), 1);
	}
	Sim->SyncRngState();
	const FGalaxyState Quiet = *State;

	FAtDeathLine Heard;
	const FDeathTick Death = StepDeathHeard(*Sim, Quiet, [Sim]() { RecordDeath(Sim->GetMutableState()); }, Heard);
	const FCallingState& Calling = State->Calling;
	TestTrue(TEXT("(without the death the life goes on)"), Death.Without.Calling.LifeState == ELifeState::Embodied && Death.Without.Calling.Lives.Last().DiedTick == 0);
	TestEqual(TEXT("A death adds no life and takes none"), Calling.Lives.Num(), Quiet.Calling.Lives.Num());
	TestTrue(TEXT("The calling is in the interregnum"), Calling.LifeState == ELifeState::Interregnum);
	TestEqual(TEXT("A wanderer's death draws exactly one number more than the same tick without it (the leap)"), Death.ExtraDraws, 1);
	const int64 Leap = CosmicSuccession::LeapOf(Rules, Quiet.Calling.LeapTicksTotal, NthFraction(Death.SeedWithout, 1));
	TestEqual(TEXT("The remaining leap is that one number through LeapOf"), Calling.PendingLeapTicks, Leap);
	TestEqual(TEXT("The run total takes it once"), Calling.LeapTicksTotal, Quiet.Calling.LeapTicksTotal + Leap);
	TestTrue(TEXT("The leap lies between the game's knobs"), Leap >= static_cast<int64>(Rules.LeapTicksMin) && Leap <= static_cast<int64>(Rules.LeapTicksMax));

	TestFalse(TEXT("(without the death the peoples keep their reverence)"), NoReverence(Death.Without));
	TestTrue(TEXT("Every people's reverence is 0 (decision 23-na - not the deposition's decay)"), NoReverence(*State));

	TestTrue(TEXT("(without the death the journey goes on)"), Death.Without.Player.TravelTicksRemaining == 9 && !Death.Without.Player.TravelTargetSystemID.IsNone());
	TestTrue(TEXT("The journey is cleared"), State->Player.TravelTargetSystemID.IsNone() && State->Player.TravelTicksRemaining == 0);
	TestEqual(TEXT("The dead vessel's place is kept (the succession reads its galaxy row there - section 2.1)"), State->Player.CurrentSystemID, Quiet.Player.CurrentSystemID);
	TestTrue(TEXT("The dead vessel is still the wanderer it was"), State->Player.Role == EPlayerRole::Wanderer);

	TestTrue(TEXT("(without the death both cards still wait)"), Death.Without.Knowing.PendingPetitionRow == CardRow && Death.Without.Knowing.PendingContactCivID == Quiet.Knowing.PendingContactCivID);
	TestTrue(TEXT("The waiting card is cleared"), State->Knowing.PendingPetitionRow.IsNone() && State->Knowing.PendingPetitionSystemID.IsNone());
	TestTrue(TEXT("The first contact card waiting is cleared"), State->Knowing.PendingContactCivID.IsNone());
	TestTrue(TEXT("The three arrays of Knowing and the first contact are those of the same tick without the death"), SameKnowing(State->Knowing, Death.Without.Knowing));
	TestEqual(TEXT("The first contact stays the calling's"), State->Knowing.FirstContactCivID, Quiet.Knowing.FirstContactCivID);
	TestTrue(TEXT("(and the arrays are not empty)"), State->Knowing.KnownSystems.Num() == 2 && State->Knowing.MetCivs.Num() == 2 && State->Knowing.MetGalaxies.Num() == 1);

	const FLifeRecord& Life = Calling.Lives.Last();
	TestEqual(TEXT("The life closes on the tick of the death"), Life.DiedTick, State->Tick);
	TestTrue(TEXT("with its cause"), Life.CauseOfDeath == Tag(TEXT("Death.Hazard")));
	TestEqual(TEXT("and the row behind it"), Life.CauseSourceRow, DeathRow);
	TestTrue(TEXT("The record is cleared"), !Calling.PendingEffect.IsValid() && !Calling.PendingCause.IsValid() && Calling.PendingSourceRow.IsNone());

	// [B-5d1] The death line (section 2.2 Die (b), 3.3): told before what the death changes, from the tell stream alone.
	TestEqual(TEXT("(without the death no death line)"), CountLinesOf(Death.Without, CosmicRows::LogDeathHazard), 0);
	ExpectDeathLine(*this, *Sim, Life, TEXT("A wanderer's death"));
	TestTrue(TEXT("It is the last line of the death's tick (the tenth stage is the last)"), !State->Log.IsEmpty() && State->Log.Last().SourceRow == CosmicRows::LogDeathHazard);
	TestTrue(TEXT("(a handler heard it)"), Heard.bHeard);
	TestTrue(TEXT("It is told while the vessel still stands - before the interregnum and its leap"), Heard.bEmbodied && Heard.PendingLeap == 0);
	TestEqual(TEXT("before the reverence is cleared"), Heard.Reverence, Quiet.Civs[0].ReverenceToPlayer);
	TestEqual(TEXT("and before any number of the death is drawn"), Heard.SimSeed, Death.SeedWithout);
	TestEqual(TEXT("It draws one number of the tell stream more than the same tick without the death"), DrawsBetween(Death.Without.TellRngState, State->TellRngState), 1);

	// Die without a vessel does nothing.
	{
		const int32 Seed = Sim->GetRng().GetCurrentSeed();
		const int32 TellSeed = State->TellRngState;
		const int64 Died = Life.DiedTick;
		Role->Die();
		TestEqual(TEXT("Die in the interregnum draws nothing"), Sim->GetRng().GetCurrentSeed(), Seed);
		TestTrue(TEXT("and changes nothing"), Calling.PendingLeapTicks == Leap && Calling.LeapTicksTotal == Quiet.Calling.LeapTicksTotal + Leap
			&& Calling.Lives.Num() == 1 && Calling.Lives.Last().DiedTick == Died);
		TestTrue(TEXT("nor tells a second death line"), CountLinesOf(*State, CosmicRows::LogDeathHazard) == 1 && State->TellRngState == TellSeed);
	}

	// The countdown (section 2.2 (0)).
	Sim->StepOnce();
	TestEqual(TEXT("The next tick's tenth stage takes one tick off the leap"), Calling.PendingLeapTicks, Leap - 1);
	TestTrue(TEXT("and the calling stays in the interregnum"), Calling.LifeState == ELifeState::Interregnum && Calling.Lives.Num() == 1);
	TestEqual(TEXT("The run total does not move with the countdown"), Calling.LeapTicksTotal, Quiet.Calling.LeapTicksTotal + Leap);

	// A journey that would end on the death's tick: a vessel whose death the tick recorded neither moves, looks nor gets a
	// card in it (the eighth and ninth stages wait on the record). One tick from system 5 to its living lane neighbour 6, both
	// settled by people 1, which the vessel has not met; the plague of the card stands on the neighbour.
	{
		FGalaxyState* Board = LayBoard(*Sim, TablesWith(Original, PlagueCard(), [](FCosmicRulesDef& Knobs) { Knobs.SightIntervalTicks = 100000; }));
		if (!TestNotNull(TEXT("The board of the journey is laid"), Board))
		{
			return false;
		}
		Walk(*Sim, *Role, 5);
		CosmicTests::SetKnowing(*Board, { 5 }, { 0 });
		Board->Systems[6].ActiveHazard = Tag(TEXT("Hazard.Plague"));
		Board->Systems[6].HazardTicksRemaining = 400;
		const FName Origin = Board->Systems[5].SystemID;
		const FName Neighbour = Board->Systems[6].SystemID;
		const FGalaxyState Standing = *Board;
		Board->Player.TravelTargetSystemID = Neighbour;
		Board->Player.TravelTicksRemaining = 1;
		const FGalaxyState Journey = *Board;

		// Without the death the journey ends at the neighbour, the vessel looks there, and the card of its plague waits.
		Sim->LoadState(Journey);
		Sim->StepOnce();
		TestTrue(TEXT("(without the death the vessel arrives at the neighbour and comes to know it)"), Board->Player.CurrentSystemID == Neighbour && Knows(*Board, 6));
		TestTrue(TEXT("(and meets its people)"), HasMet(*Board, Board->Civs[1].CivID));
		TestTrue(TEXT("(and gets the card of the plague there)"), Board->Knowing.PendingPetitionRow == CardRow && Board->Knowing.PendingPetitionSystemID == Neighbour);

		// The death without the journey, then with it.
		Sim->LoadState(Standing);
		RecordDeath(Sim->GetMutableState());
		Sim->StepOnce();
		TestTrue(TEXT("(the vessel that stood still died)"), Board->Calling.LifeState == ELifeState::Interregnum);
		const FKnowingState StillKnowing = Board->Knowing;
		Sim->LoadState(Journey);
		RecordDeath(Sim->GetMutableState());
		Sim->StepOnce();
		TestTrue(TEXT("(the vessel died on the tick its journey would have ended)"), Board->Calling.LifeState == ELifeState::Interregnum);
		TestEqual(TEXT("A vessel whose death the tick recorded does not move: it stays at the journey's origin"), Board->Player.CurrentSystemID, Origin);
		TestTrue(TEXT("and its journey is gone"), Board->Player.TravelTargetSystemID.IsNone() && Board->Player.TravelTicksRemaining == 0);
		TestFalse(TEXT("It does not look at the neighbour"), Knows(*Board, 6));
		TestFalse(TEXT("nor meet the neighbour's people through a look or a card"), HasMet(*Board, Board->Civs[1].CivID));
		TestTrue(TEXT("Its Knowing is that of the same tick without the journey"), SameKnowing(Board->Knowing, StillKnowing));
		const CosmicSuccession::FCosmicSuccessionCandidates FromOrigin = CosmicSuccession::CandidatesOf(*Board, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 5);
		TestFalse(TEXT("The origin is the death place: no candidate of the succession"), FromOrigin.Systems.Contains(5));
		TestTrue(TEXT("the neighbour the journey would have reached is one"), FromOrigin.Systems.Contains(6));
		TestTrue(TEXT("(the origin is a candidate of a death elsewhere)"), CosmicSuccession::CandidatesOf(*Board, Sim->GetStateIndex(), Sim->GetTables(), Sim->GetRules(), 0).Systems.Contains(5));
	}

	// [B-5d1] A cause with no row of its own - the measurement's borrowed Death.Age (header section 5.1; aging is 0) - is told
	// by no line: the death is carried out as any other, and the tell stream draws nothing for it.
	{
		FGalaxyState* Board = LayBoard(*Sim, TablesWith(Original, nullptr, [](FCosmicRulesDef& Knobs) { Knobs.SightIntervalTicks = 100000; }));
		if (!TestNotNull(TEXT("The board of the borrowed cause is laid"), Board))
		{
			return false;
		}
		Walk(*Sim, *Role, 0);
		const FGalaxyState Aged = *Board;
		const FDeathTick AgeDeath = StepDeath(*Sim, Aged, [Sim]()
		{
			FGalaxyState& Live = Sim->GetMutableState();
			RecordDeath(Live);
			Live.Calling.PendingCause = Tag(TEXT("Death.Age"));
		});
		TestTrue(TEXT("(the death by the borrowed cause is carried out)"), Board->Calling.LifeState == ELifeState::Interregnum
			&& Board->Calling.Lives.Last().CauseOfDeath == Tag(TEXT("Death.Age")));
		TestEqual(TEXT("A cause with no row of its own tells no death line"), CountLinesOf(*Board, CosmicRows::LogDeathHazard), 0);
		TestEqual(TEXT("and draws no number of the tell stream"), DrawsBetween(AgeDeath.Without.TellRngState, Board->TellRngState), 0);
		TestEqual(TEXT("(its numbers of the sim stream are a wanderer's death's)"), AgeDeath.ExtraDraws, 1);
	}

	// B6b [B-6b2b] (B6b header section 2.2, decision 5): a Death.Hazard whose source row's tag kills the ruler
	// (CosmicHazards::KillsTheRulerOf - the extinction; the quiet copy swaps the game's extinction for one of the test's)
	// is told by its own row, log_death_extinction, naming the vessel and the hazard - not by log_death_hazard (mutation:
	// DeathRowOf picks log_death_hazard); the cause stays Death.Hazard, and the line draws the one number of the tell stream.
	{
		const FName ExtinctRow(TEXT("crisis_test_death_extinct"));
		FCosmicDataTables Tables = TablesWith(Original, nullptr, [](FCosmicRulesDef& Knobs) { Knobs.SightIntervalTicks = 100000; });
		UDataTable* Hazards = QuietHazards(Original);
		Hazards->RemoveRow(CosmicB6bRows::CrisisSpeciesExtinction);
		FHazardDef Extinction;
		Extinction.HazardTag = Tag(TEXT("Hazard.Extinction"));
		Extinction.DisplayName = FText::AsCultureInvariant(TEXT("<the extinction>"));
		Extinction.CountdownTicks = 250;
		Extinction.SpawnRule = ECosmicHazardSpawn::Draw;
		Hazards->AddRow(ExtinctRow, Extinction);
		Tables.Hazards = Hazards;
		FGalaxyState* Board = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the extinction's death is laid"), Board))
		{
			return false;
		}
		Walk(*Sim, *Role, 0);
		const FGalaxyState Before = *Board;
		const FDeathTick Extinct = StepDeath(*Sim, Before, [Sim, ExtinctRow]()
		{
			FGalaxyState& Live = Sim->GetMutableState();
			RecordDeath(Live);
			Live.Calling.PendingSourceRow = ExtinctRow;
		});
		const FLifeRecord& Ended = Board->Calling.Lives.Last();
		TestTrue(TEXT("(the extinction's death is carried out with its cause and its row)"), Board->Calling.LifeState == ELifeState::Interregnum
			&& Ended.CauseOfDeath == Tag(TEXT("Death.Hazard")) && Ended.CauseSourceRow == ExtinctRow);
		TestEqual(TEXT("The extinction's death tells its own death line"), CountLinesOf(*Board, CosmicRows::LogDeathExtinction), 1);
		TestEqual(TEXT("and not the line of one person a star lost"), CountLinesOf(*Board, CosmicRows::LogDeathHazard), 0);
		const FCosmicLogEntry* Told = NewestLineOf(*Board, CosmicRows::LogDeathExtinction);
		if (Told)
		{
			const FString Message = Told->Message.ToString();
			TestTrue(TEXT("Log.Calling, Severity 2, told, with no focus"), Told->Category == Tag(TEXT("Log.Calling")) && Told->Severity == 2 && !Told->bUntold
				&& Told->FocusSystemID.IsNone() && Told->FocusCivID.IsNone());
			TestTrue(TEXT("naming the vessel and the hazard"), Message.Contains(CosmicChronicle::VesselNameOf(Ended, Sim->GetTables()).ToString()) && Message.Contains(TEXT("<the extinction>")));
		}
		TestEqual(TEXT("It draws one number of the tell stream more than the same tick without the death"), DrawsBetween(Extinct.Without.TellRngState, Board->TellRngState), 1);
	}
	return true;
}

// Pins a ruler's death (B5 header section 2.2 Die (c), section 2.3, section 2.5 and section 6 Calling.RulerDeath): the throne
// is left as an abdication leaves it - the people no longer ruled, a regent profile and a regent's record named by the death's
// row, the reign closed by Reign.Died, the vessel a wanderer with no status - but with no new eye; three numbers in the order
// regent profile, regent name, leap; every throne locked (B5 decision 4, old 16 #143); the people runs under the AI from the next tick; and a
// people that held only settlements lets them go on the death's tick without the crossing stage's error line (decision 7),
// the next tick's cleanup taking it. [B-5d1]: the death line is told before the throne is left (the vessel still rules, no
// regent yet, none of the three numbers drawn), from the tell stream alone.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicCallingRulerDeathTest, "Cosmic_Ascension.Calling.RulerDeath", CosmicTests::Flags)
bool FCosmicCallingRulerDeathTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDeathTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	if (!Sim)
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	// No look of the sight stage in these ticks: what the calling knows moves only by what the death does.
	const FCosmicDataTables Tables = TablesWith(Original, nullptr, [](FCosmicRulesDef& Rules) { Rules.SightIntervalTicks = 100000; });

	// 1. The ruler of people 0 dies in system 0.
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		const FCosmicRulesDef& Rules = Sim->GetRules();
		const FName Ruled = State->Civs[0].CivID;
		State->Civs[0].ActiveEdicts.Add(LeftEdict);
		State->Civs[0].ReverenceToPlayer = 0.7f;
		State->Civs[3].ReverenceToPlayer = 0.4f;
		const FGalaxyState Quiet = *State;
		bool bRecorded = false;
		FAtDeathLine Heard;
		const FDeathTick Death = StepDeathHeard(*Sim, Quiet, [&bRecorded, Sim]() { bRecorded = RecordByApply(*Sim, EEffectType::KillPlayer, TEXT("Death.Hazard"), 0, DeathRow); },
			Heard);
		TestTrue(TEXT("The death is recorded through Apply (no system: it reaches the ruler of the people)"), bRecorded);
		TestEqual(TEXT("A ruler's death draws exactly three numbers more than the same tick without it"), Death.ExtraDraws, 3);

		// [B-5d1] The death line is told before the throne is left - the deposition's form: while the vessel still rules, before
		// the regent is named and before any of the three numbers is drawn; from the tell stream alone.
		ExpectDeathLine(*this, *Sim, State->Calling.Lives.Last(), TEXT("A ruler's death"));
		TestTrue(TEXT("(a handler heard the death line)"), Heard.bHeard);
		TestTrue(TEXT("The death line is told while the vessel still rules its people"), Heard.bEmbodied && Heard.bRules);
		TestEqual(TEXT("before the regent is named"), Heard.Regents, 0);
		TestEqual(TEXT("and before the regent's profile, the regent's name and the leap draw their numbers"), Heard.SimSeed, Death.SeedWithout);
		TestEqual(TEXT("It draws one number of the tell stream more than the same tick without the death (the regent tells no line)"),
			DrawsBetween(Death.Without.TellRngState, State->TellRngState), 1);
		TestTrue(TEXT("It is the last line of the death's tick"), !State->Log.IsEmpty() && State->Log.Last().SourceRow == CosmicRows::LogDeathHazard);

		const FCivilizationState& Civ = State->Civs[0];
		TestTrue(TEXT("(without the death the vessel still rules)"), Death.Without.Civs[0].bIsPlayerRuled && Death.Without.Player.Role == EPlayerRole::Ruler);
		TestFalse(TEXT("The people is no longer ruled by the vessel"), Civ.bIsPlayerRuled);
		TestTrue(TEXT("Its regent's profile is the first number's"), Civ.AIProfile == CosmicState::PickAIProfile(NthFraction(Death.SeedWithout, 1)));
		TestEqual(TEXT("The leap is the third number's"), State->Calling.PendingLeapTicks, CosmicSuccession::LeapOf(Rules, 0, NthFraction(Death.SeedWithout, 3)));
		TestTrue(TEXT("The calling is in the interregnum"), State->Calling.LifeState == ELifeState::Interregnum && State->Calling.Lives.Num() == 1);

		const FLifeRecord& Life = State->Calling.Lives.Last();
		TestTrue(TEXT("The open reign is closed by the death"), !Life.ReignEndReasons.IsEmpty() && Life.ReignEndReasons.Last() == Tag(TEXT("Reign.Died"))
			&& Life.RuledCivIDs.Last() == Ruled);
		TestTrue(TEXT("and the life by it, with the row behind it"), Life.DiedTick == State->Tick && Life.CauseSourceRow == DeathRow && Life.CauseOfDeath == Tag(TEXT("Death.Hazard")));
		TestEqual(TEXT("A regent holds the throne"), OpenRecords(Civ, TEXT("Notable.Regent")), 1);
		const FNotableRecord* Regent = Civ.Notables.FindByPredicate([](const FNotableRecord& Record) { return Record.LastTick == 0 && Record.Role == Tag(TEXT("Notable.Regent")); });
		TestTrue(TEXT("named on the death's tick by the death's row (not the empty throne's line)"), Regent && Regent->SourceRow == DeathRow && Regent->FirstTick == State->Tick);
		// The regent's name is the second number's: NameNotable's pick (Events/CosmicEventSubsystem.cpp AddNotable) over the
		// names no record of the people carries - the whole bank of its archetype, as the people held no record before.
		{
			const FCivArchetypeDef* Archetype = Sim->GetTables().FindCiv(Quiet.Civs[0].ArchetypeRow.IsNone() ? CosmicRows::ArchDefault : Quiet.Civs[0].ArchetypeRow);
			const int32 BankSize = Archetype ? Archetype->PersonNameBank.Num() : 0;
			if (TestTrue(TEXT("(the people's bank holds more than one name, and no record of the people takes one)"), BankSize > 1 && Quiet.Civs[0].Notables.IsEmpty()))
			{
				const int32 SecondPick = FMath::Min(static_cast<int32>(NthFraction(Death.SeedWithout, 2) * static_cast<float>(BankSize)), BankSize - 1);
				TestEqual(TEXT("The regent's name is the second number's"), Regent ? Regent->NameIndex : static_cast<int32>(INDEX_NONE), SecondPick);
			}
		}

		TestTrue(TEXT("The vessel is a wanderer that rules nothing"), State->Player.Role == EPlayerRole::Wanderer && State->Player.RuledCivID.IsNone());
		TestEqual(TEXT("It remembers the people it left"), State->Player.FormerCivID, Ruled);
		TestEqual(TEXT("A death leaves no status"), State->Player.StatusTags.Num(), 0);
		TestEqual(TEXT("The dead vessel's place is kept"), State->Player.CurrentSystemID, State->Systems[0].SystemID);

		TestFalse(TEXT("(without the death the lane neighbour of the vessel's place is unknown: a new eye would show it)"), Knows(Death.Without, 1));
		TestTrue(TEXT("A death has no new eye: it knows no system the same tick without it does not"), SameMarks(State->Knowing.KnownSystems, Death.Without.Knowing.KnownSystems));
		TestTrue(TEXT("and meets no people"), SameMarks(State->Knowing.MetCivs, Death.Without.Knowing.MetCivs));

		TestFalse(TEXT("(without the death no throne is locked)"), EveryThroneLockedFrom(Death.Without, Death.Without.Tick + Rules.ThroneLockTicks));
		TestTrue(TEXT("Every people's throne is locked for ThroneLockTicks (B5 decision 4)"), EveryThroneLockedFrom(*State, State->Tick + Rules.ThroneLockTicks));
		TestTrue(TEXT("Every people's reverence is 0"), NoReverence(*State));

		// The AI stage of the death's tick ran before the tenth: the people was still ruled. The next tick it runs it.
		TestTrue(TEXT("(without the death the ruled people keeps what its ruler enacted)"), Death.Without.Civs[0].ActiveEdicts.Contains(LeftEdict));
		TestTrue(TEXT("On the death's tick the AI stage has not yet run the people"), Civ.ActiveEdicts.Contains(LeftEdict));
		Sim->StepOnce();
		TestFalse(TEXT("The next tick the AI stage runs it: the regent revokes what the ruler left"), State->Civs[0].ActiveEdicts.Contains(LeftEdict));
		TestTrue(TEXT("and the people stays with its system"), State->Civs.Num() == 4 && State->Civs[0].CivID == Ruled && State->Systems[0].OwnerCivID == Ruled);
	}

	// 2. The people the vessel rules holds only a settlement (decision 7): the crossing stage lets it go. Without a recorded
	// end of the reign that is an error line; with a death recorded in the tick it is none - the expected error below is the
	// control's one, and a second would fail the count.
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the settlement is laid"), State))
		{
			return false;
		}
		const FName Ruled = State->Civs[0].CivID;
		State->Systems[0].ActiveMegaproject = CosmicRows::ProjectSettle;
		const FGalaxyState Settling = *State;
		AddExpectedErrorPlain(TEXT("TickCrossings"), EAutomationExpectedErrorFlags::Contains, 1);
		Sim->StepOnce();
		TestTrue(TEXT("(without a death the crossing stage lets the settlement go and keeps the ruled people)"), State->Systems[0].OwnerCivID.IsNone() && State->Civs[0].bIsPlayerRuled);

		Sim->LoadState(Settling);
		RecordDeath(*State);
		Sim->StepOnce();
		TestTrue(TEXT("With the death recorded the settlement goes all the same"), State->Systems[0].OwnerCivID.IsNone());
		const int32 CivIndex = Sim->GetStateIndex().FindCiv(*State, Ruled);
		if (TestTrue(TEXT("The people is kept through the death's tick"), State->Civs.IsValidIndex(CivIndex)))
		{
			TestFalse(TEXT("and let go of by the tenth stage"), State->Civs[CivIndex].bIsPlayerRuled);
			TestEqual(TEXT("with its regent named"), OpenRecords(State->Civs[CivIndex], TEXT("Notable.Regent")), 1);
		}
		TestTrue(TEXT("The calling is in the interregnum"), State->Calling.LifeState == ELifeState::Interregnum);
		Sim->StepOnce();
		TestEqual(TEXT("The next tick's cleanup takes the people that holds no system"), Sim->GetStateIndex().FindCiv(*State, Ruled), static_cast<int32>(INDEX_NONE));
		TestEqual(TEXT("The vessel still remembers it"), State->Player.FormerCivID, Ruled);
	}
	return true;
}

// Pins the lock of every throne on every way a reign ends (B5 header section 2.3, section 6 Sim.ThroneLockEveryPath; B5
// decision 4, old 16 #143): a deposition, an abdication and a ruler's death each lock every people's throne until at least Tick +
// ThroneLockTicks (the form of Sim.Abdication); a wanderer's death locks nothing anew and lowers no earlier lock; and a leap
// of 0 (the cap reached - the succession on the death's tick) keeps the death's lock, so the new vessel cannot be crowned by
// the people it was born into.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimThroneLockEveryPathTest, "Cosmic_Ascension.Sim.ThroneLockEveryPath", CosmicTests::Flags)
bool FCosmicSimThroneLockEveryPathTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDeathTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The role subsystem exists"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FCosmicDataTables Tables = TablesWith(Original, nullptr, [](FCosmicRulesDef&) {});

	// A deposition; then, as the wanderer it made, a death.
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		const int64 Lock = Sim->GetRules().ThroneLockTicks;
		TestTrue(TEXT("The deposition is recorded"), RecordByApply(*Sim, EEffectType::DeposeRuler, TEXT("Reign.Deposed"), 0, FName(TEXT("crisis_test_revolt"))));
		Sim->StepOnce();
		TestTrue(TEXT("(the vessel was deposed)"), State->Player.Role == EPlayerRole::Wanderer && State->Player.StatusTags.HasTagExact(Tag(TEXT("Status.FallenMonarch"))));
		TestTrue(TEXT("A deposition locks every throne"), EveryThroneLockedFrom(*State, State->Tick + Lock));

		for (int32 Step = 0; Step < 5; ++Step)
		{
			Sim->StepOnce();
		}
		// One lock far beyond any the death could set: lowering it would show.
		State->Civs[3].ThroneLockedUntilTick = State->Tick + 5000;
		TArray<int64> Locks;
		for (const FCivilizationState& Civ : State->Civs)
		{
			Locks.Add(Civ.ThroneLockedUntilTick);
		}
		RecordDeath(*State);
		Sim->StepOnce();
		TestTrue(TEXT("(the wanderer died)"), State->Calling.LifeState == ELifeState::Interregnum);
		bool bKept = State->Civs.Num() == Locks.Num();
		for (int32 CivIndex = 0; bKept && CivIndex < Locks.Num(); ++CivIndex)
		{
			bKept &= State->Civs[CivIndex].ThroneLockedUntilTick == Locks[CivIndex];
		}
		TestTrue(TEXT("A wanderer's death locks no throne anew and lowers no earlier lock"), bKept);
	}

	// An abdication.
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		TestTrue(TEXT("The abdication is recorded"), Role->Abdicate());
		Sim->StepOnce();
		TestTrue(TEXT("(the vessel abdicated)"), State->Player.Role == EPlayerRole::Wanderer && State->Player.StatusTags.HasTagExact(Tag(TEXT("Status.Abdicated"))));
		TestTrue(TEXT("An abdication locks every throne"), EveryThroneLockedFrom(*State, State->Tick + Sim->GetRules().ThroneLockTicks));
	}

	// A ruler's death.
	{
		FGalaxyState* State = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		RecordDeath(*State);
		Sim->StepOnce();
		TestTrue(TEXT("(the ruler died)"), State->Calling.LifeState == ELifeState::Interregnum && !State->Civs[0].bIsPlayerRuled);
		TestTrue(TEXT("A ruler's death locks every throne"), EveryThroneLockedFrom(*State, State->Tick + Sim->GetRules().ThroneLockTicks));
	}

	// A ruler's death with the leaps' cap reached: a leap of 0, the succession on the death's tick.
	{
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, nullptr, [](FCosmicRulesDef& Rules) { Rules.LeapTotalCap = 0; }));
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		RecordDeath(*State);
		Sim->StepOnce();
		const FCallingState& Calling = State->Calling;
		if (!TestTrue(TEXT("(a second life stands on the death's tick)"), Calling.Lives.Num() == 2 && Calling.LifeState == ELifeState::Embodied
			&& Calling.Lives[0].DiedTick == State->Tick && Calling.Lives[1].BornTick == State->Tick))
		{
			return false;
		}
		TestTrue(TEXT("A ruler's death with a leap of 0 locks every throne too"), EveryThroneLockedFrom(*State, State->Tick + Sim->GetRules().ThroneLockTicks));
		const FName Birth = Calling.Lives[1].BirthCivID;
		TestTrue(TEXT("so the people the new vessel was born into cannot crown it yet"),
			CosmicWanderer::CrownBlockOf(*State, Sim->GetStateIndex(), Sim->GetRules(), Birth, true, false) == CosmicWanderer::ECrownBlock::ThroneLocked);
	}
	return true;
}

// Pins the leap's one number (B5 header decision 3, section 2.5 and section 6 Sim.LeapDraw): CosmicSuccession::LeapOf maps
// 0 to the shortest leap and a number just below 1 to the longest, reads a longest below the shortest as the shortest, a
// shortest below 0 and a number below 0 or not a number as 0, is cut to what the cap leaves and is 0 at the cap; and in the
// world a death draws that one number whatever the knobs hold (the leap of 0 adds the succession's two and succeeds on the
// death's tick).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimLeapDrawTest, "Cosmic_Ascension.Sim.LeapDraw", CosmicTests::Flags)
bool FCosmicSimLeapDrawTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDeathTests;
	// The pure function.
	FCosmicRulesDef Knobs;
	Knobs.LeapTicksMin = 800;
	Knobs.LeapTicksMax = 1600;
	Knobs.LeapTotalCap = 4500;
	// The largest float below 1 (1 - 2^-24).
	const float Top = 0.99999994f;
	const float NaN = std::numeric_limits<float>::quiet_NaN();
	auto Expect = [this](const TCHAR* What, const FCosmicRulesDef& Rules, int64 Total, float Number, int64 Expected)
	{
		TestEqual(What, CosmicSuccession::LeapOf(Rules, Total, Number), Expected);
	};
	Expect(TEXT("0 is the shortest leap"), Knobs, 0, 0.f, 800);
	Expect(TEXT("A number just below 1 is the longest"), Knobs, 0, Top, 1600);
	Expect(TEXT("A half is the middle of the 801 lengths"), Knobs, 0, 0.5f, 1200);
	Expect(TEXT("1 itself reads as the longest"), Knobs, 0, 1.f, 1600);
	// A number at 1 or above - an infinity too - reads as the longest, before the cast (no undefined conversion).
	const float Infinity = std::numeric_limits<float>::infinity();
	Expect(TEXT("1e30 reads as the longest"), Knobs, 0, 1e30f, 1600);
	Expect(TEXT("Infinity reads as the longest"), Knobs, 0, Infinity, 1600);
	Expect(TEXT("Minus infinity reads as 0"), Knobs, 0, -Infinity, 800);
	Expect(TEXT("1 is cut by what the cap leaves"), Knobs, 4000, 1.f, 500);
	Expect(TEXT("1e30 is cut by what the cap leaves"), Knobs, 4000, 1e30f, 500);
	Expect(TEXT("Infinity is cut by what the cap leaves"), Knobs, 4000, Infinity, 500);
	Expect(TEXT("Infinity at the cap is 0"), Knobs, 4500, Infinity, 0);
	Expect(TEXT("A number below 0 reads as 0"), Knobs, 0, -0.5f, 800);
	Expect(TEXT("A number that is not a number reads as 0"), Knobs, 0, NaN, 800);
	FCosmicRulesDef Even = Knobs;
	Even.LeapTicksMax = 800;
	Expect(TEXT("The shortest equal to the longest is the one length"), Even, 0, Top, 800);
	FCosmicRulesDef Inverted = Knobs;
	Inverted.LeapTicksMin = 1200;
	Inverted.LeapTicksMax = 100;
	Expect(TEXT("A longest below the shortest reads as the shortest (0)"), Inverted, 0, 0.f, 1200);
	Expect(TEXT("A longest below the shortest reads as the shortest (just below 1)"), Inverted, 0, Top, 1200);
	FCosmicRulesDef Below = Knobs;
	Below.LeapTicksMin = -50;
	Below.LeapTicksMax = 10;
	Expect(TEXT("A shortest below 0 reads as 0"), Below, 0, 0.f, 0);
	Expect(TEXT("and the longest stays"), Below, 0, Top, 10);
	Expect(TEXT("What the cap leaves cuts a longer draw (the shortest)"), Knobs, 4000, 0.f, 500);
	Expect(TEXT("What the cap leaves cuts a longer draw (the longest)"), Knobs, 4000, Top, 500);
	Expect(TEXT("A draw the cap still holds is not cut"), Knobs, 3700, 0.f, 800);
	Expect(TEXT("At the cap the leap is 0"), Knobs, 4500, Top, 0);
	Expect(TEXT("Past the cap it is 0"), Knobs, 6000, 0.5f, 0);
	FCosmicRulesDef NoCap = Knobs;
	NoCap.LeapTotalCap = 0;
	Expect(TEXT("A cap of 0 leaves no leap"), NoCap, 0, Top, 0);
	NoCap.LeapTotalCap = -5;
	Expect(TEXT("A cap below 0 leaves none either"), NoCap, 0, 0.5f, 0);

	// The world: a wanderer's death under four sets of knobs.
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The role subsystem exists"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	struct FCase
	{
		const TCHAR* What;
		int32 Min;
		int32 Max;
		int32 Cap;
		int64 TotalBefore;
		int64 Leap;
		int32 Draws;
	};
	const FCase Cases[] = {
		{ TEXT("the shortest equal to the longest"), 1000, 1000, 4500, 0, 1000, 1 },
		{ TEXT("the longest below the shortest"), 1200, 100, 4500, 0, 1200, 1 },
		{ TEXT("a cap that leaves less than the draw"), 800, 1600, 4500, 4200, 300, 1 },
		{ TEXT("a cap of 0 (the leap's one and the succession's two)"), 800, 1600, 0, 0, 0, 3 },
	};
	for (const FCase& Case : Cases)
	{
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, nullptr, [&Case](FCosmicRulesDef& Rules)
		{
			Rules.LeapTicksMin = Case.Min;
			Rules.LeapTicksMax = Case.Max;
			Rules.LeapTotalCap = Case.Cap;
		}));
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		Walk(*Sim, *Role, 0);
		State->Calling.LeapTicksTotal = Case.TotalBefore;
		const FGalaxyState Quiet = *State;
		const FDeathTick Death = StepDeath(*Sim, Quiet, [Sim]() { RecordDeath(Sim->GetMutableState()); });
		const FCallingState& Calling = State->Calling;
		TestEqual(*FString::Printf(TEXT("With %s the death's tick draws the numbers of its points only"), Case.What), Death.ExtraDraws, Case.Draws);
		if (Case.Leap > 0)
		{
			TestTrue(*FString::Printf(TEXT("With %s the calling waits in the interregnum"), Case.What), Calling.LifeState == ELifeState::Interregnum && Calling.Lives.Num() == 1);
			TestEqual(*FString::Printf(TEXT("With %s the leap is the knobs' answer"), Case.What), Calling.PendingLeapTicks, Case.Leap);
			TestEqual(*FString::Printf(TEXT("With %s the run total takes it"), Case.What), Calling.LeapTicksTotal, Case.TotalBefore + Case.Leap);
		}
		else
		{
			TestTrue(TEXT("A cap of 0 succeeds on the death's tick"), Calling.Lives.Num() == 2 && Calling.LifeState == ELifeState::Embodied
				&& Calling.Lives[0].DiedTick == State->Tick && Calling.Lives[1].BornTick == State->Tick);
			TestTrue(TEXT("as a called wanderer"), Calling.Lives[1].SuccessionKind == Tag(TEXT("Succession.Called")) && State->Player.Role == EPlayerRole::Wanderer);
			TestTrue(TEXT("with no leap left and none added"), Calling.PendingLeapTicks == 0 && Calling.LeapTicksTotal == 0);
		}
	}
	return true;
}

// Pins the interregnum without a vessel (B5 header section 2.2 (0) and 'the leap runs no tick', section 6
// Calling.NoPlayerDuringLeap; docs/50 Phase 4b): over 50 ticks of the leap the countdown takes one a tick and nothing else
// ends the interregnum, the three arrays of Knowing and the first contact stay as at the death though the sight stage's
// ticks pass and an unmet people owns the place, the dead vessel does not move, no card comes though its card's crisis stands
// there, and the calling completes no feat while a people climbs in the meantime.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicCallingNoPlayerDuringLeapTest, "Cosmic_Ascension.Calling.NoPlayerDuringLeap", CosmicTests::Flags)
bool FCosmicCallingNoPlayerDuringLeapTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDeathTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The role subsystem exists"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	// The sight stage looks every ten ticks: the death's tick (401) is not one of them, ten of the leap's are.
	FCosmicDataTables Tables = TablesWith(Original, PlagueCard(), [](FCosmicRulesDef& Rules) { Rules.SightIntervalTicks = 10; });
	Tables.Feats->AddRow(ClimbRow, ClimbFeat());
	FGalaxyState* State = LayBoard(*Sim, Tables);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	// The wanderer stands in system 5 of people 1, which it has not met; it knows only that system.
	Walk(*Sim, *Role, 5);
	CosmicTests::SetKnowing(*State, { 5 }, { 0 });
	RecordDeath(*State);
	Sim->StepOnce();
	if (!TestTrue(TEXT("(the wanderer died)"), State->Calling.LifeState == ELifeState::Interregnum))
	{
		return false;
	}
	const int64 Leap = State->Calling.PendingLeapTicks;
	if (!TestTrue(TEXT("(the leap outlasts the ticks of this test)"), Leap > 50))
	{
		return false;
	}
	TestTrue(TEXT("(no people met first yet)"), State->Knowing.FirstContactCivID.IsNone() && !HasMet(*State, State->Civs[1].CivID));
	// The crisis of the card where the dead vessel stands, and of the feat on people 1.
	State->Systems[5].ActiveHazard = Tag(TEXT("Hazard.Plague"));
	State->Systems[5].HazardTicksRemaining = 400;
	const FGalaxyState AtDeath = *State;

	bool bCountdown = true;
	bool bStill = true;
	bool bKnowing = true;
	bool bNoCard = true;
	for (int32 Step = 1; Step <= 50; ++Step)
	{
		Sim->StepOnce();
		const FCallingState& Calling = State->Calling;
		bCountdown &= Calling.LifeState == ELifeState::Interregnum && Calling.PendingLeapTicks == Leap - Step && Calling.Lives.Num() == 1
			&& Calling.LeapTicksTotal == AtDeath.Calling.LeapTicksTotal;
		bStill &= State->Player.CurrentSystemID == AtDeath.Player.CurrentSystemID && State->Player.TravelTargetSystemID.IsNone() && State->Player.TravelTicksRemaining == 0;
		bKnowing &= SameKnowing(State->Knowing, AtDeath.Knowing);
		bNoCard &= State->Knowing.PendingPetitionRow.IsNone();
	}
	TestTrue(TEXT("Every tick of the leap takes one off it and nothing else ends the interregnum"), bCountdown);
	TestTrue(TEXT("The three arrays of Knowing and the first contact stay as at the death"), bKnowing);
	TestTrue(TEXT("The dead vessel does not move"), bStill);
	TestTrue(TEXT("No card waits, though its crisis stands where the vessel died"), bNoCard);
	TestTrue(TEXT("(the people whose system holds the crisis climbed meanwhile: the feat judgment ran)"), State->Civs[1].Tier == EAscensionTier::Tier2_Stellar);
	TestTrue(TEXT("The calling completes no feat"), State->Calling.CompletedFeats == AtDeath.Calling.CompletedFeats);
	TestTrue(TEXT("and the closed life records none"), State->Calling.Lives.Last().Feats == AtDeath.Calling.Lives.Last().Feats
		&& State->Calling.Lives.Last().PeakTier == AtDeath.Calling.Lives.Last().PeakTier);
	return true;
}

// Pins the hard reset of a succession (B5 header section 2.2 Succeed 3-4, decision 8, section 6 Calling.HardReset; docs/50
// Phase 4b): the vessel's fields start from nothing; the calling keeps what is its own - a personal tier of the third rung
// (one below goes back to the first, read into the new life's peak), the Way.* powers (others go); the knowing before the
// succession begins the knowing after it, the first contact unchanged; the new name's source string is never the previous
// life's, even across two archetypes sharing a bank where a plain draw would repeat it; and the succession makes no notable
// record and closes none. [B-5d1] (section 2.2 Succeed 5, 3.3): the inauguration line after Install - Log.Calling, Severity 2,
// the new vessel's name, its birth system and people, the tick's last line and the one that opens its chapter -, one number of
// the tell stream and the succession's two of the sim stream unchanged; with a leap of 0 the death line and then the
// inauguration line end the tick; the silence tells none and draws nothing of the tell stream.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicCallingHardResetTest, "Cosmic_Ascension.Calling.HardReset", CosmicTests::Flags)
bool FCosmicCallingHardResetTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDeathTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The role subsystem exists"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };

	// 1. The second rung: a leap of two ticks, the one candidate system 5 of people 1.
	{
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, nullptr, [](FCosmicRulesDef& Rules) { Rules.LeapTicksMin = 2; Rules.LeapTicksMax = 2; }), 2);
		if (!TestNotNull(TEXT("The board is laid"), State))
		{
			return false;
		}
		Walk(*Sim, *Role, 0);
		// The first life as if born to one twin; the people of the candidate is of the other (two archetypes, one bank).
		State->Calling.Lives.Last().BirthArchetypeRow = TwinA;
		State->Civs[1].ArchetypeRow = TwinB;
		// What the vessel carries (with the status and the people left from the walk).
		FPlayerEntityState& Player = State->Player;
		Player.Body = 6;
		Player.Mind = 5;
		Player.Causality = 3;
		Player.Exaltation = 2;
		Player.PersonalPool.Energy = 10.f;
		Player.PersonalPool.Insight = 4.f;
		Player.Relics = { FName(TEXT("relic_test_ring")) };
		State->Calling.PersonalTier = EAscensionTier::Tier2_Stellar;
		State->Calling.Powers.AddTag(Tag(TEXT("Way.Received")));
		// A declared tag outside Way.* (no Power.* tag is declared yet): what the tier would give again.
		State->Calling.Powers.AddTag(Tag(TEXT("Law.Humility")));
		CosmicTests::SetKnowing(*State, { 0, 1 }, { 0 });
		FKnownMark& Galaxy = State->Knowing.MetGalaxies.AddDefaulted_GetRef();
		Galaxy.ID = Sim->GetRules().HomeGalaxyRow;
		Galaxy.Tick = State->Tick;
		// Records a succession must neither make nor close: an open ruler of people 1, a closed rebel of people 0.
		FNotableRecord& Leader = State->Civs[1].Notables.AddDefaulted_GetRef();
		Leader.Role = Tag(TEXT("Notable.Ruler"));
		Leader.NameIndex = 1;
		Leader.FirstTick = 10;
		Leader.SourceRow = FName(TEXT("event_test_crowned"));
		FNotableRecord& Rebel = State->Civs[0].Notables.AddDefaulted_GetRef();
		Rebel.Role = Tag(TEXT("Notable.Rebel"));
		Rebel.FirstTick = 5;
		Rebel.LastTick = 50;
		Rebel.SourceRow = FName(TEXT("crisis_test_unrest"));

		RecordDeath(*State);
		Sim->StepOnce();
		const FGalaxyState AtDeath = *State;
		TestTrue(TEXT("(the vessel died, with a leap of two)"), State->Calling.LifeState == ELifeState::Interregnum && State->Calling.PendingLeapTicks == 2);
		Sim->StepOnce();
		TestTrue(TEXT("(one tick of the leap is left)"), State->Calling.LifeState == ELifeState::Interregnum && State->Calling.PendingLeapTicks == 1);
		const FGalaxyState BeforeSucceed = *State;

		// A first pass of the succession's tick learns the name's number - the last the tick draws (Install, after the
		// succession's own) - so that the previous life can be given the name a plain draw of the new bank would land on.
		Sim->StepOnce();
		if (!TestEqual(TEXT("(the countdown reaches 0 and the succession installs a second life)"), State->Calling.Lives.Num(), 2))
		{
			return false;
		}
		const int32 Probe = State->RngState;
		const int32 Drawn = DrawsBetween(BeforeSucceed.RngState, Probe);
		if (!TestTrue(TEXT("(the succession's tick drew its two numbers at least)"), Drawn >= 2))
		{
			return false;
		}
		const int32 PlainPick = FMath::Min(static_cast<int32>(NthFraction(BeforeSucceed.RngState, Drawn) * 2.f), 1);
		Sim->LoadState(BeforeSucceed);
		State->Calling.Lives[0].NameIndex = PlainPick;
		const FString PreviousName = NameSourceOf(Sim->GetTables(), State->Calling.Lives[0]);
		Sim->StepOnce();

		const FCallingState& Calling = State->Calling;
		TestEqual(TEXT("The previous life's name changes no number of the succession's tick"), State->RngState, Probe);
		if (!TestEqual(TEXT("A second life stands"), Calling.Lives.Num(), 2))
		{
			return false;
		}
		const FLifeRecord& Life = Calling.Lives[1];
		TestTrue(TEXT("It is called, born in the one candidate to its people and that people's archetype"), Life.SuccessionKind == Tag(TEXT("Succession.Called"))
			&& Life.BirthSystemID == State->Systems[5].SystemID && Life.BirthCivID == State->Civs[1].CivID && Life.BirthArchetypeRow == TwinB);
		TestTrue(TEXT("on the tick the countdown reached 0"), Life.BornTick == State->Tick && Calling.BornTick == State->Tick && Calling.LifeIndex == 2 && Life.LifeIndex == 2);
		TestTrue(TEXT("The calling is entrusted to it with no leap left"), Calling.LifeState == ELifeState::Embodied && Calling.PendingLeapTicks == 0 && Calling.LeapTicksTotal == 2);

		const FPlayerEntityState Fresh = FPlayerEntityState();
		const FPlayerEntityState& Vessel = State->Player;
		TestTrue(TEXT("The vessel's stats start from nothing"), Vessel.Body == Fresh.Body && Vessel.Mind == Fresh.Mind && Vessel.Causality == Fresh.Causality
			&& Vessel.Exaltation == Fresh.Exaltation);
		TestTrue(TEXT("its pool too"), Vessel.PersonalPool.Energy == Fresh.PersonalPool.Energy && Vessel.PersonalPool.Matter == Fresh.PersonalPool.Matter
			&& Vessel.PersonalPool.Insight == Fresh.PersonalPool.Insight && Vessel.PersonalPool.Influence == Fresh.PersonalPool.Influence
			&& Vessel.PersonalPool.Causality == Fresh.PersonalPool.Causality);
		TestEqual(TEXT("It carries no relic"), Vessel.Relics.Num(), 0);
		TestEqual(TEXT("and no status"), Vessel.StatusTags.Num(), 0);
		TestTrue(TEXT("A wanderer that rules nothing and left no people"), Vessel.Role == EPlayerRole::Wanderer && Vessel.RuledCivID.IsNone() && Vessel.FormerCivID.IsNone());
		TestTrue(TEXT("standing where it was born, going nowhere"), Vessel.CurrentSystemID == Life.BirthSystemID && Vessel.TravelTargetSystemID.IsNone() && Vessel.TravelTicksRemaining == 0);

		TestTrue(TEXT("A personal tier below the third goes back to the first"), Calling.PersonalTier == EAscensionTier::Tier1_Planetary);
		TestTrue(TEXT("before the new life's peak is read from it"), Life.PeakTier == EAscensionTier::Tier1_Planetary);
		TestTrue(TEXT("The way is kept"), Calling.Powers.HasTagExact(Tag(TEXT("Way.Received"))));
		TestFalse(TEXT("another power is not"), Calling.Powers.HasTagExact(Tag(TEXT("Law.Humility"))));
		TestEqual(TEXT("(the way alone)"), Calling.Powers.Num(), 1);

		TestTrue(TEXT("(the leap kept the knowing of the death)"), SameKnowing(BeforeSucceed.Knowing, AtDeath.Knowing));
		TestTrue(TEXT("The systems known before the succession begin those known after it"), IsPrefix(BeforeSucceed.Knowing.KnownSystems, State->Knowing.KnownSystems));
		TestTrue(TEXT("the peoples met too"), IsPrefix(BeforeSucceed.Knowing.MetCivs, State->Knowing.MetCivs));
		TestTrue(TEXT("and the galaxies"), IsPrefix(BeforeSucceed.Knowing.MetGalaxies, State->Knowing.MetGalaxies));
		TestEqual(TEXT("The first contact is the one before"), State->Knowing.FirstContactCivID, BeforeSucceed.Knowing.FirstContactCivID);
		TestTrue(TEXT("The new life knows where it was born and has met its people"), Knows(*State, 5) && HasMet(*State, State->Civs[1].CivID)
			&& State->Knowing.KnownSystems.Num() > BeforeSucceed.Knowing.KnownSystems.Num());

		const FString NewName = NameSourceOf(Sim->GetTables(), Life);
		TestTrue(TEXT("(both names are read from a bank)"), !PreviousName.IsEmpty() && !NewName.IsEmpty());
		TestFalse(TEXT("The new name's source string is not the previous life's, across two archetypes sharing a bank (decision 8)"), NewName == PreviousName);
		TestTrue(TEXT("The succession makes no notable record and closes none"), SameNotables(BeforeSucceed, *State));

		// [B-5d1] The inauguration line (section 2.2 Succeed 5, 3.3): after Install, the last line of the succession's tick.
		{
			const FCosmicLogEntry* Line = NewestLineOf(*State, CosmicRows::LogInauguration);
			TestEqual(TEXT("The succession tells one inauguration line"), CountLinesOf(*State, CosmicRows::LogInauguration), 1);
			if (TestNotNull(TEXT("(the inauguration line)"), Line))
			{
				TestEqual(TEXT("on the succession's tick"), Line->Tick, Life.BornTick);
				TestTrue(TEXT("the calling's line, Severity 2, told"), Line->Category == Tag(TEXT("Log.Calling")) && Line->Severity == 2 && !Line->bUntold);
				TestTrue(TEXT("about the new life's birth system and people"), Line->FocusSystemID == Life.BirthSystemID && Line->FocusCivID == Life.BirthCivID);
				const FString Message = Line->Message.ToString();
				const FString Name = CosmicChronicle::VesselNameOf(Life, Sim->GetTables()).ToString();
				TestTrue(TEXT("It names the new vessel by its name ({Player} - the third person, decision 12-da)"), !Name.IsEmpty() && Message.Contains(Name));
				TestTrue(TEXT("its birth system ({System})"), Message.Contains(State->Systems[5].DisplayName.ToString()));
				TestTrue(TEXT("and its birth people ({Civ})"), Message.Contains(State->Civs[1].CivName.ToString()));
				TestTrue(TEXT("It is the last line of the succession's tick - told after Install"), &State->Log.Last() == Line);
				TestTrue(TEXT("It is the line that opens the new life's chapter"), CosmicChronicle::OpensCalledChapter(*Line, Life));
			}
		}
		// From the tell stream alone: against the same tick with one more tick of leap left (no succession), the succession's
		// tick draws its two numbers of the sim stream (the succession's and the name's - the line none) and one of the tell
		// stream (the line). Last in this case: the run is laid again.
		{
			FGalaxyState Longer = BeforeSucceed;
			Longer.Calling.PendingLeapTicks = 2;
			const FDeathTick Succession = StepDeath(*Sim, Longer, [Sim]() { Sim->GetMutableState().Calling.PendingLeapTicks = 1; });
			TestTrue(TEXT("(the succession stands, and one more tick of leap stands none)"), State->Calling.Lives.Num() == 2 && Succession.Without.Calling.Lives.Num() == 1);
			TestEqual(TEXT("The succession's tick draws its two numbers of the sim stream and no more"), Succession.ExtraDraws, 2);
			TestEqual(TEXT("The inauguration line draws one number of the tell stream"), DrawsBetween(Succession.Without.TellRngState, State->TellRngState), 1);
		}
	}

	// 2. The third rung, with a leap of 0 (the succession on the death's tick).
	{
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, nullptr, [](FCosmicRulesDef& Rules) { Rules.LeapTotalCap = 0; }), 2);
		if (!TestNotNull(TEXT("The board of the third rung is laid"), State))
		{
			return false;
		}
		Walk(*Sim, *Role, 0);
		State->Calling.PersonalTier = EAscensionTier::Tier3_Causal;
		State->Calling.Powers.AddTag(Tag(TEXT("Way.Seized")));
		State->Calling.Powers.AddTag(Tag(TEXT("Law.Humility")));
		const FGalaxyState BeforeDeath = *State;
		RecordDeath(*State);
		Sim->StepOnce();
		const FCallingState& Calling = State->Calling;
		if (!TestEqual(TEXT("(a leap of 0: the second life stands on the death's tick)"), Calling.Lives.Num(), 2))
		{
			return false;
		}
		TestTrue(TEXT("A personal tier of the third rung is kept"), Calling.PersonalTier == EAscensionTier::Tier3_Causal);
		TestTrue(TEXT("and is the new life's peak"), Calling.Lives[1].PeakTier == EAscensionTier::Tier3_Causal);
		TestTrue(TEXT("The way is kept and the rest goes"), Calling.Powers.HasTagExact(Tag(TEXT("Way.Seized"))) && !Calling.Powers.HasTagExact(Tag(TEXT("Law.Humility")))
			&& Calling.Powers.Num() == 1);
		TestFalse(TEXT("The new name's source string is not the previous life's"), NameSourceOf(Sim->GetTables(), Calling.Lives[1]) == NameSourceOf(Sim->GetTables(), Calling.Lives[0]));
		TestTrue(TEXT("The systems known before the death begin those known after the succession"), IsPrefix(BeforeDeath.Knowing.KnownSystems, State->Knowing.KnownSystems)
			&& IsPrefix(BeforeDeath.Knowing.MetCivs, State->Knowing.MetCivs));

		// [B-5d1] One tick, both lines: the death line, then the inauguration line - the tick's last two, in that order, each
		// naming its own life.
		const TArray<FCosmicLogEntry>& Lines = State->Log;
		const int32 Count = Lines.Num();
		if (TestTrue(TEXT("With a leap of 0 the tick ends with the death line, then the inauguration line"), Count >= 2
			&& Lines[Count - 2].SourceRow == CosmicRows::LogDeathHazard && Lines[Count - 1].SourceRow == CosmicRows::LogInauguration
			&& Lines[Count - 2].Tick == State->Tick && Lines[Count - 1].Tick == State->Tick))
		{
			TestTrue(TEXT("the death line names the life that closed"), Lines[Count - 2].Message.ToString().Contains(CosmicChronicle::VesselNameOf(Calling.Lives[0], Sim->GetTables()).ToString()));
			TestTrue(TEXT("the inauguration line the life that stood up"), Lines[Count - 1].Message.ToString().Contains(CosmicChronicle::VesselNameOf(Calling.Lives[1], Sim->GetTables()).ToString()));
		}
	}

	// 3. [B-5d1] The silence (no candidate - section 2.11): no inauguration line and no number of the tell stream, the
	// succession's two numbers of the sim stream all the same.
	{
		FGalaxyState* State = LayBoard(*Sim, TablesWith(Original, nullptr, [](FCosmicRulesDef& Rules) { Rules.LeapTicksMin = 2; Rules.LeapTicksMax = 2; }), 2);
		if (!TestNotNull(TEXT("The board of the silence is laid"), State))
		{
			return false;
		}
		Walk(*Sim, *Role, 0);
		// The one candidate of a death in system 0 leaves the death place's galaxy row.
		State->Systems[5].GalaxyRow = FName(TEXT("galaxy_test_apart"));
		RecordDeath(*State);
		Sim->StepOnce();
		Sim->StepOnce();
		if (!TestTrue(TEXT("(the vessel died; one tick of the leap is left)"), State->Calling.LifeState == ELifeState::Interregnum && State->Calling.PendingLeapTicks == 1))
		{
			return false;
		}
		FGalaxyState Longer = *State;
		Longer.Calling.PendingLeapTicks = 2;
		const FDeathTick Silence = StepDeath(*Sim, Longer, [Sim]() { Sim->GetMutableState().Calling.PendingLeapTicks = 1; });
		TestTrue(TEXT("(the silence: no new life, no leap left)"), State->Calling.LifeState == ELifeState::Interregnum && State->Calling.PendingLeapTicks == 0
			&& State->Calling.Lives.Num() == 1);
		TestEqual(TEXT("The silence tells no inauguration line"), CountLinesOf(*State, CosmicRows::LogInauguration), 0);
		TestEqual(TEXT("and draws no number of the tell stream"), DrawsBetween(Silence.Without.TellRngState, State->TellRngState), 0);
		TestEqual(TEXT("(the succession's two numbers of the sim stream all the same)"), Silence.ExtraDraws, 2);
	}
	return true;
}

// Pins the first tick of a life a succession began (B5 header section 2.2 'the first tick of a life also compares with an
// empty head', section 6 Calling.FirstTickOfLife): CosmicPetitions::IsFirstTickOfLife is true at the head of the tick after
// the succession only (an embodied wanderer, Tick == BornTick, not the first life); a new vessel born where a crisis already
// stands gets that crisis's wanderer card on that tick; and the card moves no number of the sim stream and nothing of the
// world - the same run whose table holds no such card ends that tick with the same stream and the same world hash.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicCallingFirstTickOfLifeTest, "Cosmic_Ascension.Calling.FirstTickOfLife", CosmicTests::Flags)
bool FCosmicCallingFirstTickOfLifeTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDeathTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The role subsystem exists"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FCosmicDataTables WithCard = TablesWith(Original, PlagueCard(), [](FCosmicRulesDef& Rules) { Rules.LeapTicksMin = 2; Rules.LeapTicksMax = 2; });
	FCosmicDataTables WithoutCard = WithCard;
	WithoutCard.Encounters = CosmicTests::NewTable(FEncounterDef::StaticStruct());

	// A wanderer in system 0; the plague stands in system 5, the one candidate, long enough for the succession's filter.
	FGalaxyState* State = LayBoard(*Sim, WithCard, 2);
	if (!TestNotNull(TEXT("The board is laid"), State))
	{
		return false;
	}
	Walk(*Sim, *Role, 0);
	State->Systems[5].ActiveHazard = Tag(TEXT("Hazard.Plague"));
	State->Systems[5].HazardTicksRemaining = 500;
	const FGalaxyState Quiet = *State;
	{
		FGalaxyState FirstLife = Quiet;
		FirstLife.Tick = FirstLife.Calling.BornTick;
		TestFalse(TEXT("The first life's first tick is not a succession's"), CosmicPetitions::IsFirstTickOfLife(FirstLife));
	}

	// The death, two ticks of leap (the second succeeds), then the new life's first tick.
	auto Run = [Sim](const FCosmicDataTables& Tables, const FGalaxyState& From)
	{
		FLifeRun Out;
		Sim->SetTables(Tables);
		Sim->LoadState(From);
		FGalaxyState& Live = Sim->GetMutableState();
		RecordDeath(Live);
		Sim->StepOnce();
		Out.bDied = Live.Calling.LifeState == ELifeState::Interregnum;
		Out.bFirstInLeap = CosmicPetitions::IsFirstTickOfLife(Live);
		Sim->StepOnce();
		Sim->StepOnce();
		Out.bBorn = Live.Calling.Lives.Num() == 2 && Live.Calling.LifeState == ELifeState::Embodied;
		Out.bFirstAtBirth = CosmicPetitions::IsFirstTickOfLife(Live);
		Out.bNoCardAtBirth = Live.Knowing.PendingPetitionRow.IsNone();
		Out.BirthSystemID = Live.Calling.Lives.Last().BirthSystemID;
		Out.bHazardAtBirth = Live.Systems[5].ActiveHazard == Tag(TEXT("Hazard.Plague"));
		Out.AtBirth = Live;
		Sim->StepOnce();
		Out.bFirstAfter = CosmicPetitions::IsFirstTickOfLife(Live);
		Out.Card = Live.Knowing.PendingPetitionRow;
		Out.CardSystemID = Live.Knowing.PendingPetitionSystemID;
		Out.Seed = Live.RngState;
		Out.WorldHash = Sim->ComputeWorldHash();
		return Out;
	};
	const FLifeRun Card = Run(WithCard, Quiet);
	const FLifeRun NoCard = Run(WithoutCard, Quiet);

	if (!TestTrue(TEXT("(the vessel died and a second life stood two ticks later)"), Card.bDied && Card.bBorn && NoCard.bDied && NoCard.bBorn))
	{
		return false;
	}
	TestTrue(TEXT("(born in system 5, where the crisis already stands)"), Card.BirthSystemID == Quiet.Systems[5].SystemID && Card.bHazardAtBirth);
	TestFalse(TEXT("The interregnum has no first tick of a life"), Card.bFirstInLeap);
	TestTrue(TEXT("The tick after the succession is the life's first"), Card.bFirstAtBirth);
	TestFalse(TEXT("the one after it is not"), Card.bFirstAfter);
	{
		FGalaxyState AsRuler = Card.AtBirth;
		AsRuler.Player.Role = EPlayerRole::Ruler;
		TestFalse(TEXT("A ruler's first tick is the reign's, not this"), CosmicPetitions::IsFirstTickOfLife(AsRuler));
	}
	TestTrue(TEXT("(no card waits when the new life stands)"), Card.bNoCardAtBirth);
	TestEqual(TEXT("On its first tick the new vessel gets the card of the crisis that already stood where it was born"), Card.Card, CardRow);
	TestEqual(TEXT("about that system"), Card.CardSystemID, Quiet.Systems[5].SystemID);
	TestTrue(TEXT("(without the card's row none comes)"), NoCard.Card.IsNone());
	TestEqual(TEXT("The card moves no number of the sim stream"), Card.Seed, NoCard.Seed);
	TestTrue(TEXT("and nothing of the world (the world hash)"), Card.WorldHash == NoCard.WorldHash);
	return true;
}

// Pins the one lethal source of the game's data ([B-5d2] - B5 header section 3.2, 3.5, 2.8, 2.9; decision 6-ga): the game's
// tables carry one KillPlayer, by Death.Hazard, in the own expiry effects of the AI awakening (crisis_ai_awakening), so
// CosmicHazards::KillsAVesselOf is true for that hazard and false for every other, and CosmicTests::WithoutDeath takes it
// away. On the game's rows (the quiet copy - no draw places a hazard): an embodied wanderer standing where the awakening
// expires on its own side dies on that tick, by Death.Hazard from crisis_ai_awakening, and the death line names the hazard;
// the branch side spares it; an awakening the card ended (enc_awakening_talk's success) never expires; a ruler of that
// system, a vessel elsewhere and a vessel inside the birth protection do not die (born one tick earlier it does); a vessel
// that set out from that system dies there, for it stands where it left until it arrives (section 2.8). The succession
// never offers a system holding a live awakening, not even as the one place left.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicCallingLethalSourceTest, "Cosmic_Ascension.Calling.LethalSource", CosmicTests::Flags)
bool FCosmicCallingLethalSourceTest::RunTest(const FString& Parameters)
{
	using namespace CosmicDeathTests;
	UGalaxySimulationSubsystem* Sim = CosmicTests::AcquireSim(*this);
	UPlayerRoleSubsystem* Role = Sim ? Sim->GetGameInstance()->GetSubsystem<UPlayerRoleSubsystem>() : nullptr;
	if (!Sim || !TestNotNull(TEXT("The role subsystem exists"), Role))
	{
		return false;
	}
	const FCosmicDataTables Original = Sim->GetTables();
	ON_SCOPE_EXIT { Sim->GetStageClock() = FCosmicStageClock(); Sim->SetTables(Original); Sim->NewGalaxy(42, 5, 3); };
	const FGameplayTag Awakening = Tag(TEXT("Hazard.AIAwakening"));
	const FGameplayTag Cause = Tag(TEXT("Death.Hazard"));

	// ---- 1. The data ----
	FName RowName;
	const FHazardDef* Row = Original.FindHazardByTag(Awakening, &RowName);
	if (!TestNotNull(TEXT("The game has the AI awakening's row"), Row) || !TestEqual(TEXT("(crisis_ai_awakening)"), RowName, AwakeningRow) || !Original.Hazards)
	{
		return false;
	}
	auto KillsIn = [](const TArray<FCosmicEffect>& Effects, const FGameplayTag& Of)
	{
		int32 Count = 0;
		for (const FCosmicEffect& Effect : Effects)
		{
			Count += (Effect.Type == EEffectType::KillPlayer && (!Of.IsValid() || Effect.Tag == Of)) ? 1 : 0;
		}
		return Count;
	};
	TestEqual(TEXT("Its own expiry effects carry one KillPlayer, by Death.Hazard"), KillsIn(Row->OnExpireEffects, Cause), 1);
	TestEqual(TEXT("and no other"), KillsIn(Row->OnExpireEffects, FGameplayTag()), 1);
	TestTrue(TEXT("Its branch, the damage to its neighbours and its resolution kill nobody"), KillsIn(Row->OnExpireBranchEffects, FGameplayTag()) == 0
		&& KillsIn(Row->OnExpireNeighbourEffects, FGameplayTag()) == 0 && KillsIn(Row->OnResolveEffects, FGameplayTag()) == 0);
	TestTrue(TEXT("(its branch is a side of its own: a chance of it)"), Row->BranchChance > 0.f && Row->BranchChance < 1.f);
	TestTrue(TEXT("KillsAVesselOf: the AI awakening kills a vessel"), CosmicHazards::KillsAVesselOf(Original, Awakening));
	int32 Others = 0;
	for (const FName Name : Original.Hazards->GetRowNames())
	{
		const FHazardDef* Other = Original.FindHazard(Name);
		if (Other && Name != AwakeningRow)
		{
			++Others;
			TestFalse(*FString::Printf(TEXT("KillsAVesselOf: %s kills nobody"), *Other->HazardTag.ToString()), CosmicHazards::KillsAVesselOf(Original, Other->HazardTag));
		}
	}
	TestTrue(TEXT("(the game has other hazards)"), Others > 0);
	TestFalse(TEXT("CosmicTests::WithoutDeath takes the one death away"), CosmicHazards::KillsAVesselOf(CosmicTests::WithoutDeath(Original), Awakening));

	// ---- 2. The world, on the game's rows ----
	auto NoSight = [](FCosmicRulesDef& Rules) { Rules.SightIntervalTicks = 100000; };
	const FCosmicDataTables Tables = TablesWith(Original, nullptr, NoSight);
	const FCosmicDataTables Carded = TablesWith(Original, CosmicTests::CopyWithout<FEncounterDef>(Original.Encounters, {}), NoSight);
	TestTrue(TEXT("(the quiet copy keeps the game's lethal row)"), CosmicHazards::KillsAVesselOf(Tables, Awakening) && CosmicHazards::KillsAVesselOf(Carded, Awakening));
	// LayBoard's board with the awakening in system 5 (people 1's), Left ticks to go, and the vessel a wanderer walked from
	// there.
	constexpr int32 Lethal = 5;
	auto Lay = [&](const FCosmicDataTables& With, int32 Left) -> FGalaxyState*
	{
		FGalaxyState* Board = LayBoard(*Sim, With);
		if (!Board)
		{
			return nullptr;
		}
		Walk(*Sim, *Role, Lethal);
		Board->Systems[Lethal].ActiveHazard = Awakening;
		Board->Systems[Lethal].HazardTicksRemaining = Left;
		return Board;
	};
	const FGalaxyState& State = Sim->GetState();
	FGalaxyState Before;

	// The own side: the wanderer standing there dies on the expiry's tick.
	{
		const FGalaxyState* Board = Lay(Tables, 1);
		if (!TestNotNull(TEXT("The board is laid"), Board))
		{
			return false;
		}
		const FGalaxyState From = *Board;
		const FExpiryNote Note = ExpireOnSide(*Sim, From, Lethal, false, Before);
		if (!TestTrue(TEXT("(the awakening expired on its own side)"), Note.bNoted))
		{
			return false;
		}
		TestTrue(TEXT("(a KillPlayer on the system reached the vessel standing there)"), Note.bReached);
		TestTrue(TEXT("The awakening's own expiry kills the wanderer standing there: the calling is in the interregnum"),
			State.Calling.LifeState == ELifeState::Interregnum && State.Calling.Lives.Num() == 1);
		const FLifeRecord& Life = State.Calling.Lives.Last();
		TestEqual(TEXT("on the expiry's tick"), Life.DiedTick, State.Tick);
		TestTrue(TEXT("by Death.Hazard"), Life.CauseOfDeath == Cause);
		TestEqual(TEXT("from the awakening's row"), Life.CauseSourceRow, AwakeningRow);
		ExpectDeathLine(*this, *Sim, Life, TEXT("The awakening's death"));
		const FCosmicLogEntry* Line = NewestLineOf(State, CosmicRows::LogDeathHazard);
		TestTrue(TEXT("The death line is log_death_hazard, told with the game row's DisplayName"), Line && Line->Message.ToString().Contains(Row->DisplayName.ToString()));
		TestTrue(TEXT("The system stays its people's, its slot empty"), State.Systems[Lethal].OwnerCivID == From.Systems[Lethal].OwnerCivID
			&& !State.Systems[Lethal].ActiveHazard.IsValid());
	}

	// The branch side spares the vessel.
	{
		const FGalaxyState* Board = Lay(Tables, 1);
		if (!TestNotNull(TEXT("The board of the branch is laid"), Board))
		{
			return false;
		}
		const FGalaxyState From = *Board;
		const FExpiryNote Note = ExpireOnSide(*Sim, From, Lethal, true, Before);
		if (!TestTrue(TEXT("(the awakening expired on its branch)"), Note.bNoted))
		{
			return false;
		}
		TestTrue(TEXT("(the vessel was in reach: only the side spares it)"), Note.bReached);
		TestTrue(TEXT("The branch side's expiry kills nobody: the vessel lives on, with no death line"), LivesOn(State));
	}

	// The card's success ends the awakening before it expires.
	{
		const FGalaxyState* Board = Lay(Carded, 1);
		if (!TestNotNull(TEXT("The board of the card is laid"), Board))
		{
			return false;
		}
		const FGalaxyState From = *Board;
		if (!TestTrue(TEXT("(unanswered, the awakening expires on its own side and kills)"), ExpireOnSide(*Sim, From, Lethal, false, Before).bNoted
			&& State.Calling.LifeState == ELifeState::Interregnum))
		{
			return false;
		}
		const FEncounterDef* Card = Carded.FindEncounter(AwakeningTalk);
		const bool bEnds = Card && !Card->Choices.IsEmpty() && Card->Choices[0].OnSuccess.ContainsByPredicate([&Awakening](const FCosmicEffect& Effect)
		{
			return Effect.Type == EEffectType::ResolveHazard && Effect.Tag == Awakening;
		});
		if (!TestTrue(TEXT("(the game's card of the awakening: its first choice ends it on success)"), bEnds))
		{
			return false;
		}
		// The same tick, the card answered first (between ticks, as the player answers), with a Mind no roll can fail.
		Sim->LoadState(Before);
		FGalaxyState& Live = Sim->GetMutableState();
		Live.Player.Mind = FMath::Max(Live.Player.Mind, Card->Choices[0].Difficulty);
		Live.Knowing.PendingPetitionRow = AwakeningTalk;
		Live.Knowing.PendingPetitionSystemID = Live.Systems[Lethal].SystemID;
		TestTrue(TEXT("The card's first choice succeeds"), Role->AnswerPetition(0).Outcome == ECosmicPetitionOutcome::Succeeded);
		TestFalse(TEXT("and ends the awakening"), State.Systems[Lethal].ActiveHazard.IsValid());
		const FGalaxyState Answered = State;
		TestFalse(TEXT("An awakening the card ended does not expire"), StepNoted(*Sim, Answered, Lethal).bNoted);
		TestTrue(TEXT("and kills nobody"), LivesOn(State));
	}

	// A ruler standing in the system: what happens to a system never kills a ruler.
	{
		FGalaxyState* Board = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the ruler is laid"), Board))
		{
			return false;
		}
		Board->Systems[0].ActiveHazard = Awakening;
		Board->Systems[0].HazardTicksRemaining = 1;
		const FGalaxyState From = *Board;
		const FExpiryNote Note = ExpireOnSide(*Sim, From, 0, false, Before);
		if (!TestTrue(TEXT("(the awakening expired on its own side in the ruler's own system)"), Note.bNoted))
		{
			return false;
		}
		TestFalse(TEXT("(a KillPlayer on the system did not reach the ruler)"), Note.bReached);
		TestTrue(TEXT("A ruler of that system standing there does not die"), LivesOn(State) && State.Player.Role == EPlayerRole::Ruler && State.Civs[0].bIsPlayerRuled);
	}

	// A wanderer elsewhere: the next system.
	{
		FGalaxyState* Board = Lay(Tables, 1);
		if (!TestNotNull(TEXT("The board of the neighbour is laid"), Board))
		{
			return false;
		}
		Board->Player.CurrentSystemID = Board->Systems[4].SystemID;
		const FGalaxyState From = *Board;
		const FExpiryNote Note = ExpireOnSide(*Sim, From, Lethal, false, Before);
		TestTrue(TEXT("A wanderer one lane away does not die"), Note.bNoted && !Note.bReached && LivesOn(State));
	}

	// The birth protection: shielded on the expiry's tick it lives; born one tick earlier it dies.
	{
		FGalaxyState* Board = Lay(Tables, 1);
		const int32 Protect = Sim->GetRules().BirthProtectionTicks;
		if (!TestNotNull(TEXT("The board of the protection is laid"), Board) || !TestTrue(TEXT("(the game shields a newborn vessel for a while)"), Protect > 1 && Board->Tick + 1 > Protect))
		{
			return false;
		}
		const FGalaxyState From = *Board;
		const int64 Expiry = From.Tick + 1;
		auto BornAt = [&From](int64 Born)
		{
			FGalaxyState Out = From;
			Out.Calling.BornTick = Born;
			Out.Calling.Lives.Last().BornTick = Born;
			return Out;
		};
		const FExpiryNote Inside = ExpireOnSide(*Sim, BornAt(Expiry - Protect + 1), Lethal, false, Before);
		if (!TestTrue(TEXT("(the awakening expired on its own side on the protection's last tick)"), Inside.bNoted))
		{
			return false;
		}
		TestTrue(TEXT("A vessel inside the birth protection does not die"), !Inside.bReached && LivesOn(State));
		const FExpiryNote Past = ExpireOnSide(*Sim, BornAt(Expiry - Protect), Lethal, false, Before);
		TestTrue(TEXT("Born one tick earlier, the same expiry kills it"), Past.bNoted && Past.bReached && State.Calling.LifeState == ELifeState::Interregnum);
	}

	// A vessel that set out from the system: under way it stands where it left (section 2.8).
	{
		FGalaxyState* Board = Lay(Tables, 1);
		if (!TestNotNull(TEXT("The board of the journey is laid"), Board))
		{
			return false;
		}
		FGalaxyState From = *Board;
		const FName Origin = From.Systems[Lethal].SystemID;
		From.Player.TravelTargetSystemID = From.Systems[Lethal + 1].SystemID;
		From.Player.TravelTicksRemaining = 10;
		const FExpiryNote Note = ExpireOnSide(*Sim, From, Lethal, false, Before);
		if (!TestTrue(TEXT("(the awakening expired on its own side while the vessel was under way)"), Note.bNoted))
		{
			return false;
		}
		TestTrue(TEXT("A vessel that set out from that system dies there: it has not arrived"), Note.bReached && State.Calling.LifeState == ELifeState::Interregnum
			&& State.Calling.Lives.Last().CauseSourceRow == AwakeningRow);
		TestEqual(TEXT("at the journey's origin"), State.Player.CurrentSystemID, Origin);
		TestTrue(TEXT("and its journey is gone"), State.Player.TravelTargetSystemID.IsNone() && State.Player.TravelTicksRemaining == 0);
	}

	// ---- 3. The succession never offers a live awakening's system ----
	{
		FGalaxyState* Board = LayBoard(*Sim, Tables);
		if (!TestNotNull(TEXT("The board of the candidates is laid"), Board))
		{
			return false;
		}
		const FCosmicRulesDef& Rules = Sim->GetRules();
		const int32 Long = FMath::Max(1000, Rules.SuccessionMinHazardTicks + 1);
		Board->Systems[6].ActiveHazard = Awakening;
		Board->Systems[6].HazardTicksRemaining = Long;
		Board->Systems[5].ActiveHazard = Tag(TEXT("Hazard.Plague"));
		Board->Systems[5].HazardTicksRemaining = Long;
		const CosmicSuccession::FCosmicSuccessionCandidates Found = CosmicSuccession::CandidatesOf(*Board, Sim->GetStateIndex(), Sim->GetTables(), Rules, 0);
		TestFalse(TEXT("The succession leaves out a settled system that holds a live AI awakening"), Found.Systems.Contains(6));
		TestTrue(TEXT("not one whose hazard kills nobody"), Found.Systems.Contains(5));
		TestTrue(TEXT("(without the death in the data it is a candidate: the lethal filter left it out)"),
			CosmicSuccession::CandidatesOf(*Board, Sim->GetStateIndex(), CosmicTests::WithoutDeath(Sim->GetTables()), Rules, 0).Systems.Contains(6));
	}
	{
		// Two peoples: people 1 holds system 5 alone, the one place a death in system 0 could go.
		FGalaxyState* Board = LayBoard(*Sim, Tables, 2);
		if (!TestNotNull(TEXT("The board of the one place is laid"), Board))
		{
			return false;
		}
		const FCosmicRulesDef& Rules = Sim->GetRules();
		Board->Systems[Lethal].ActiveHazard = Awakening;
		Board->Systems[Lethal].HazardTicksRemaining = FMath::Max(1000, Rules.SuccessionMinHazardTicks + 1);
		TestEqual(TEXT("Not even as the one place left: the lethal filter never relaxes"),
			CosmicSuccession::CandidatesOf(*Board, Sim->GetStateIndex(), Sim->GetTables(), Rules, 0).Systems.Num(), 0);
		TestTrue(TEXT("(without the death in the data the one place is offered)"),
			CosmicSuccession::CandidatesOf(*Board, Sim->GetStateIndex(), CosmicTests::WithoutDeath(Sim->GetTables()), Rules, 0).Systems.Contains(Lethal));
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
