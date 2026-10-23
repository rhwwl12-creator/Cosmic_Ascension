// Cosmic Ascension - balance commandlet.

#include "Tests/CosmicBalanceCommandlet.h"

#include "Core/GalaxyState.h"
#include "Cosmic_Ascension.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicRowNames.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Events/CosmicPetitions.h"
#include "GameMapsSettings.h"
#include "GameplayTagContainer.h"
#include "Log/CosmicChronicle.h"
#include "Math/NumericLimits.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "Log/CosmicLogSubsystem.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Player/PlayerRoleSubsystem.h"
#include "Save/CosmicSaveGame.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicEndings.h"
#include "Sim/CosmicMegaprojects.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"

namespace
{
	constexpr int64 SampleEveryTicks = 100;
	// SaveBytes is measured on every tenth sample only: one save of the whole state takes tens of milliseconds.
	constexpr int64 SaveEverySamples = 10;

	// The claims that ended one way, with what their owners held when they ended (section 9.3: why a claim runs out).
	struct FClaimEnds
	{
		int32 Count = 0;
		// Ended with an answer (project_relief in the game's data) in the response slot of the system.
		int32 WithAnswer = 0;
		// The owner's Influence before the tick in which the claim ended, summed.
		double OwnerInfluence = 0.0;
	};

	// What the run counts tick by tick, read from the state only (no hook into the simulation, no draw).
	struct FRunCounts
	{
		int64 FirstClaimTick = -1;
		// Where the first claim stood and the tick its slot emptied (-1 while it stands): the spiritual sight's claim
		// screenshot reads them (CLAUDE.md, the screenshot recipe).
		FName FirstClaimSystemID;
		int64 FirstClaimEndTick = -1;
		int64 FirstJoinTick = -1;
		int64 DeposedTick = -1;
		// The first tick at which some people holds two settled systems: claims can stand from then on (Law.Remnant).
		int64 FirstTwoSettledTick = -1;
		int32 ClaimsAfterTwoSettled = 0;
		FClaimEnds ClaimsJoined;
		FClaimEnds ClaimsStilled;
		int64 ToldLines = 0;
		TMap<FString, int32> Placed;
		TMap<FString, int32> Answered;
		TMap<FString, int32> Expired;
		TMap<FString, int32> Ended;
		// Why the peoples the player does not rule started no settlement, one count per (sample, people) - the tick-0 sample
		// included -, and how many such pairs there were, counted apart from the reasons (PeoplesNotRuled at every sample:
		// the reasons must add up to it).
		TMap<FString, int32> Expansion;
		int64 ExpansionPairs = 0;
		// The ruler's side (-Answer, -Build): the cards that came, the answers taken, the projects started, their sum (the
		// decisions) and the sums and told lines at the ends of the first 600 and 6,000 ticks; the ticks at which the ruled
		// people reached the second and the third rung (-1 while it has not).
		int32 CardsCome = 0;
		int32 Answers = 0;
		int32 Started = 0;
		int64 DecisionsAt600 = -1;
		int64 DecisionsAt6000 = -1;
		int64 ToldAt600 = -1;
		int64 ToldAt6000 = -1;
		int64 RuledT2Tick = -1;
		int64 RuledT3Tick = -1;
		// The provisional end (MVP item 4), looked at after every tick and before the policy's input - nothing stops at it: the
		// first cause and its tick, the first ticks of each cause, and the last chapter of the chronicle at the first end.
		ECosmicProvisionalEnd FirstEnd = ECosmicProvisionalEnd::None;
		int64 FirstEndTick = -1;
		int64 RungTick = -1;
		int64 ClockTick = -1;
		int32 ChapterLife = -1;
		int32 ChapterShown = -1;
		int32 ChapterCandidates = -1;
		int32 ChapterMore = -1;
		// The last chapter's candidates by the cap's rank (Log.Calling, own Severity 2, admitted Severity 1, other peoples'
		// Severity 2): what the cap of CosmicChronicle::MaxLifeLines is set from.
		int32 ChapterRanks[4] = { -1, -1, -1, -1 };
	};

	// The answer of the -Answer policy: the first choice the stockpile can pay, from choice (answers so far) on, round
	// (so that every choice of a card is taken in turn over a run). INDEX_NONE when none can be paid.
	int32 PolicyChoiceOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FEncounterDef& Row, int32 AnswersSoFar)
	{
		const int32 CivIndex = CosmicPetitions::RuledCivIndexOf(State, Index);
		const int32 Count = Row.Choices.Num();
		if (!State.Civs.IsValidIndex(CivIndex) || Count <= 0)
		{
			return INDEX_NONE;
		}
		for (int32 Step = 0; Step < Count; ++Step)
		{
			const int32 Choice = (AnswersSoFar + Step) % Count;
			if (CosmicMegaprojects::CanPay(State.Civs[CivIndex].Stockpile, CosmicPetitions::CostOf(Row.Choices[Choice])))
			{
				return Choice;
			}
		}
		return INDEX_NONE;
	}

	// A slot before the tick: its hazard and count, the answer in its response slot, the owner's Influence, and the
	// construction slot (a settlement that paid nothing in the tick is a stalled one).
	struct FSlotBefore
	{
		FGameplayTag Tag;
		int32 Remaining = 0;
		FName Answer;
		float OwnerInfluence = 0.f;
		CosmicMegaprojects::FCosmicSlotReading Construction;
	};

	// The names of CosmicMegaprojects::ECosmicSettleBlock in its order, as the CSV and the log print them. NoLane of a people
	// with no settled system prints NoFoothold (the reason is NoLane in the rule; the commandlet tells the two apart).
	const TCHAR* const SettleBlockNames[] = {
		TEXT("None"),
		TEXT("Sanctuary"),
		TEXT("NoPeople"),
		TEXT("Owned"),
		TEXT("Dead"),
		TEXT("NoLane"),
		TEXT("NoRow"),
		TEXT("RowTier"),
		TEXT("RealmCap"),
		TEXT("Settling"),
		TEXT("Sphere"),
	};
	static_assert(static_cast<int32>(UE_ARRAY_COUNT(SettleBlockNames)) == static_cast<int32>(CosmicMegaprojects::ECosmicSettleBlock::Count), "One name per reason, in the order of the enum");

	// Why the people CivIndex starts no settlement now (CosmicMegaprojects::PeopleSettleBlockOf by name): NoFoothold for a
	// people with no settled system, SettlingStalled for a settlement under way that paid nothing in the tick just run (its
	// construction slot against Before - an empty Before, the tick-0 sample, says Settling).
	FString ExpansionOf(const UGalaxySimulationSubsystem& Sim, int32 CivIndex, const TArray<FSlotBefore>& Before)
	{
		const FGalaxyState& State = Sim.GetState();
		const FCosmicStateIndex& Index = Sim.GetStateIndex();
		const CosmicMegaprojects::ECosmicSettleBlock Block = CosmicMegaprojects::PeopleSettleBlockOf(State, Index, Sim.GetTables(), Sim.GetRules(), CivIndex);
		const FCivilizationState& Civ = State.Civs[CivIndex];
		if (Block == CosmicMegaprojects::ECosmicSettleBlock::NoLane)
		{
			const bool bFoothold = Civ.OwnedSystems.ContainsByPredicate([&](FName SystemID)
			{
				const int32 SystemIndex = Index.FindSystem(State, SystemID);
				return State.Systems.IsValidIndex(SystemIndex) && State.Systems[SystemIndex].OwnerCivID == Civ.CivID && CosmicState::IsSettled(State.Systems[SystemIndex]);
			});
			return bFoothold ? TEXT("NoLane") : TEXT("NoFoothold");
		}
		if (Block == CosmicMegaprojects::ECosmicSettleBlock::Settling)
		{
			for (const FName SystemID : Civ.OwnedSystems)
			{
				const int32 SystemIndex = Index.FindSystem(State, SystemID);
				if (!State.Systems.IsValidIndex(SystemIndex) || SystemIndex >= Before.Num() || State.Systems[SystemIndex].OwnerCivID != Civ.CivID
					|| State.Systems[SystemIndex].ActiveMegaproject != CosmicRows::ProjectSettle)
				{
					continue;
				}
				const CosmicMegaprojects::FCosmicSlotReading Now = CosmicMegaprojects::ReadSlot(State.Systems[SystemIndex], CosmicMegaprojects::ECosmicProjectSlot::Construction);
				return CosmicMegaprojects::PaidNothing(Before[SystemIndex].Construction, Now) ? TEXT("SettlingStalled") : TEXT("Settling");
			}
		}
		const int32 At = static_cast<int32>(Block);
		return (At >= 0 && At < static_cast<int32>(UE_ARRAY_COUNT(SettleBlockNames))) ? SettleBlockNames[At] : TEXT("?");
	}

	int32 SphereSize(const FGalaxyState& State)
	{
		int32 Size = 0;
		for (const FTranscendentState& Transcendent : State.Transcendents)
		{
			Size += Transcendent.InfluencedSystems.Num();
		}
		return Size;
	}

	// The sphere of every instance, as "<TranscendentID> <size>".
	TArray<FString> SpheresOf(const FGalaxyState& State)
	{
		TArray<FString> Spheres;
		for (const FTranscendentState& Transcendent : State.Transcendents)
		{
			Spheres.Add(FString::Printf(TEXT("%s %d"), *Transcendent.TranscendentID.ToString(), Transcendent.InfluencedSystems.Num()));
		}
		return Spheres;
	}

	bool HoldsTwoSettled(const FGalaxyState& State)
	{
		TMap<FName, int32> Settled;
		for (const FStarSystemState& System : State.Systems)
		{
			if (!System.OwnerCivID.IsNone() && CosmicState::IsSettled(System) && ++Settled.FindOrAdd(System.OwnerCivID) >= 2)
			{
				return true;
			}
		}
		return false;
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

	// The beings of the fourth rung and above as section 4.2 counts them (layer 4 of section 3.1): the unsealed Tier 4
	// fallen with a sphere, the independents, and the Tier 4 peoples the player does not rule.
	int32 T4PlusCount(const FGalaxyState& State, const FCosmicDataTables& Tables)
	{
		static const FGameplayTag Abyssal = FGameplayTag::RequestGameplayTag(FName(TEXT("Realm.Abyssal")));
		static const FGameplayTag Independent = FGameplayTag::RequestGameplayTag(FName(TEXT("Realm.Independent")));
		const int32 Fourth = static_cast<int32>(EAscensionTier::Tier4_Transcendent);
		int32 Count = 0;
		for (const FCivilizationState& Civ : State.Civs)
		{
			Count += (static_cast<int32>(Civ.Tier) >= Fourth && !Civ.bIsPlayerRuled) ? 1 : 0;
		}
		for (const FTranscendentState& Transcendent : State.Transcendents)
		{
			const FTranscendentDef* Def = Tables.FindTranscendent(Transcendent.TranscendentID);
			const bool bFallen = Def && Def->Allegiance == Abyssal && static_cast<int32>(Def->Tier) >= Fourth && !Transcendent.bSealed
				&& !Transcendent.InfluencedSystems.IsEmpty();
			Count += (bFallen || (Def && Def->Allegiance == Independent)) ? 1 : 0;
		}
		return Count;
	}

	// The size of the state serialized as the game's save serializes it (SaveGameToMemory of a UCosmicSaveGame), and the
	// time of the copy and the serialization. The file write of SaveGameToSlot is not in it. One save object for the run.
	int32 SaveBytesOf(UCosmicSaveGame* Save, const FGalaxyState& State, double& OutMilliseconds)
	{
		TArray<uint8> Bytes;
		const double Start = FPlatformTime::Seconds();
		Save->State = State;
		UGameplayStatics::SaveGameToMemory(Save, Bytes);
		OutMilliseconds = (FPlatformTime::Seconds() - Start) * 1000.0;
		return Bytes.Num();
	}

	// The peoples the player does not rule: the (sample, people) pairs one sample adds to the expansion counts, counted
	// apart from AppendSample, so that a people it skips or a filter it gets wrong shows as a sum that does not add up.
	int32 PeoplesNotRuled(const FGalaxyState& State)
	{
		int32 Count = 0;
		for (const FCivilizationState& Civ : State.Civs)
		{
			Count += Civ.bIsPlayerRuled ? 0 : 1;
		}
		return Count;
	}

	// One CSV row per civilization, the galaxy columns on each, and last the expansion reason of a people the player does
	// not rule (empty for the people the player rules), which it counts into Counts.Expansion.
	void AppendSample(FString& Csv, const UGalaxySimulationSubsystem& Sim, const TArray<FSlotBefore>& Before, FRunCounts& Counts, double TickMicros, const FString& SaveBytes)
	{
		const FGalaxyState& State = Sim.GetState();
		const FString FirstClaim = Counts.FirstClaimTick >= 0 ? FString::Printf(TEXT("%lld"), Counts.FirstClaimTick) : FString();
		const FString Galaxy = FString::Printf(TEXT("%d,%d,%d,%s,%d,%s,%lld,%.1f,%s,%d,%d"),
			State.Knowing.KnownSystems.Num(), MetCivs(State), WildSystems(State), *FirstContactTick(State), T4PlusCount(State, Sim.GetTables()),
			*FirstClaim, Counts.ToldLines, TickMicros, *SaveBytes, State.Log.Num(), State.Chronicle.Num());
		for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
		{
			const FCivilizationState& Civ = State.Civs[CivIndex];
			int32 Hazards = 0;
			for (const FStarSystemState& System : State.Systems)
			{
				Hazards += (System.OwnerCivID == Civ.CivID && System.ActiveHazard.IsValid()) ? 1 : 0;
			}
			FString Expansion;
			if (!Civ.bIsPlayerRuled)
			{
				Expansion = ExpansionOf(Sim, CivIndex, Before);
				Counts.Expansion.FindOrAdd(Expansion)++;
			}
			Csv += FString::Printf(TEXT("%lld,%s,%s,%d,%d,%.4f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%s,%s\n"),
				State.Tick, *Civ.CivID.ToString(), *Civ.ArchetypeRow.ToString(), static_cast<int32>(Civ.Tier) + 1, Civ.OwnedSystems.Num(),
				Civ.Stability, Civ.Stockpile.Energy, Civ.Stockpile.Matter, Civ.Stockpile.Insight, Civ.Stockpile.Influence, Civ.Stockpile.Causality, Hazards,
				*Galaxy, *Expansion);
		}
	}

	// -Rules=Name:Value;Name:Value - DT_Rules fields replaced for this run only (section 9.3: a knob measured before it is
	// written into the data). Each value is read as the data importer reads it (ImportText). False, with an error line, for
	// a field the row lacks or a value it cannot read.
	bool OverrideRules(UGalaxySimulationSubsystem& Sim, const FString& Text)
	{
		FCosmicRulesDef Row = Sim.GetRules();
		TArray<FString> Pairs;
		Text.ParseIntoArray(Pairs, TEXT(";"));
		for (const FString& Pair : Pairs)
		{
			FString Name;
			FString Value;
			FProperty* Property = Pair.Split(TEXT(":"), &Name, &Value) ? FCosmicRulesDef::StaticStruct()->FindPropertyByName(FName(*Name)) : nullptr;
			if (!Property || !Property->ImportText_Direct(*Value, Property->ContainerPtrToValuePtr<void>(&Row), nullptr, PPF_None))
			{
				UE_LOG(LogCosmic, Error, TEXT("CosmicBalance: -Rules cannot set '%s'"), *Pair);
				return false;
			}
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance: rules %s = %s"), *Name, *Value);
		}
		UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
		Table->RowStruct = FCosmicRulesDef::StaticStruct();
		Table->AddRow(CosmicRows::Rules, Row);
		FCosmicDataTables Tables = Sim.GetTables();
		Tables.Rules = Table;
		Sim.SetTables(Tables);
		return true;
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

	FString ClaimEndsText(const TCHAR* Name, const FClaimEnds& Ends)
	{
		return FString::Printf(TEXT("%d %s (answer running %d, owner Influence mean %.1f)"), Ends.Count, Name, Ends.WithAnswer,
			Ends.Count > 0 ? Ends.OwnerInfluence / Ends.Count : 0.0);
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
	// The ruler's policy (section 9.3; Phase 5 adds -PlayerPolicy): -Answer=1 answers every card the tick it comes
	// (PolicyChoiceOf), -Build=1 starts every project the ruler's project key would start (StartNextMegaproject: the
	// answers and the next feat's construction, then a settlement) - both between ticks, as a player's input. 0 (the
	// default) leaves the ruler idle: the world of RunHeadless(Ticks).
	int32 Answer = 0;
	int32 Build = 0;
	FString RulesText;
	FString Out = TEXT("balance.csv");
	FParse::Value(*Params, TEXT("Seed="), Seed);
	FParse::Value(*Params, TEXT("Ticks="), Ticks);
	FParse::Value(*Params, TEXT("Systems="), Systems);
	FParse::Value(*Params, TEXT("Civs="), Civs);
	FParse::Value(*Params, TEXT("TellAll="), TellAll);
	FParse::Value(*Params, TEXT("Answer="), Answer);
	FParse::Value(*Params, TEXT("Build="), Build);
	FParse::Value(*Params, TEXT("Rules="), RulesText, false);
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
	UPlayerRoleSubsystem* Role = GameInstance->GetSubsystem<UPlayerRoleSubsystem>();
	// A run on rules it could not set would print numbers of other rules: it does not run.
	const bool bRules = Sim && (RulesText.IsEmpty() || OverrideRules(*Sim, RulesText));
	if (Sim && Role && bRules)
	{
		// Every line is stored unless -TellAll=0 asks for the scope of telling of a game (section 8.1). The world hash is
		// the same either way.
		if (UCosmicLogSubsystem* Log = GameInstance->GetSubsystem<UCosmicLogSubsystem>())
		{
			Log->SetTellAll(TellAll != 0);
		}
		Sim->NewGalaxy(Seed, Systems, Civs);
		const FGalaxyState& State = Sim->GetState();
		UCosmicSaveGame* Save = NewObject<UCosmicSaveGame>();
		Save->AddToRoot();

		FRunCounts Counts;
		const int32 SphereStart = SphereSize(State);
		const TArray<FString> SpheresStart = SpheresOf(State);
		for (const FCosmicLogEntry& Entry : State.Log)
		{
			Counts.ToldLines += Entry.bUntold ? 0 : 1;
		}
		FString Csv = TEXT("Tick,CivID,Archetype,Tier,Systems,Stability,Energy,Matter,Insight,Influence,Causality,Hazards,KnownSystems,MetCivs,WildSystems,FirstContactTick,T4PlusCount,FirstClaimTick,ToldLines,TickMicros,SaveBytes,LogLen,ChronicleLen,Expansion\n");
		FString Rows = TEXT("Tick,SourceRow,Lines\n");
		double Unused = 0.0;
		TArray<double> TickMicros;
		TArray<FSlotBefore> Before;
		// The tick-0 sample: no tick has run yet, so a settlement under way reads as Settling.
		AppendSample(Csv, *Sim, Before, Counts, 0.0, FString::FromInt(SaveBytesOf(Save, State, Unused)));
		Counts.ExpansionPairs += PeoplesNotRuled(State);
		TMap<FString, int32> SampleRows;
		double SampleMicros = 0.0;
		int64 SampleTicks = 0;
		int64 Samples = 0;
		for (int64 Done = 0; Done < Ticks; ++Done)
		{
			Before.Reset();
			for (const FStarSystemState& System : State.Systems)
			{
				const FCivilizationState* Owner = State.Civs.FindByPredicate([&System](const FCivilizationState& Civ) { return Civ.CivID == System.OwnerCivID; });
				Before.Add({ System.ActiveHazard, System.HazardTicksRemaining, System.ResponseMegaproject, Owner ? Owner->Stockpile.Influence : 0.f,
					CosmicMegaprojects::ReadSlot(System, CosmicMegaprojects::ECosmicProjectSlot::Construction) });
			}
			const EPlayerRole RoleBefore = State.Player.Role;
			const int64 TickBefore = State.Tick;
			const int32 SphereBefore = SphereSize(State);
			const FName CardBefore = State.Knowing.PendingPetitionRow;

			const uint64 Start = FPlatformTime::Cycles64();
			Sim->RunHeadless(1);
			const double Micros = static_cast<double>(FPlatformTime::Cycles64() - Start) * FPlatformTime::GetSecondsPerCycle64() * 1.0e6;
			TickMicros.Add(Micros);
			SampleMicros += Micros;
			++SampleTicks;

			// The provisional end, read as the presentation reads it at this tick boundary (before the ruler's input of it).
			{
				const ECosmicProvisionalEnd End = CosmicEndings::ProvisionalEndOf(State, Sim->GetRules());
				const int32 EndTick = Sim->GetRules().ProvisionalEndTick;
				Counts.RungTick = (Counts.RungTick < 0 && End == ECosmicProvisionalEnd::Rung) ? State.Tick : Counts.RungTick;
				Counts.ClockTick = (Counts.ClockTick < 0 && EndTick > 0 && State.Tick >= EndTick) ? State.Tick : Counts.ClockTick;
				if (Counts.FirstEnd == ECosmicProvisionalEnd::None && End != ECosmicProvisionalEnd::None)
				{
					Counts.FirstEnd = End;
					Counts.FirstEndTick = State.Tick;
					const TArray<FCosmicChronicleChapter> Chapters = CosmicChronicle::Build(State);
					if (!Chapters.IsEmpty() && State.Calling.Lives.IsValidIndex(Chapters.Last().LifeAt))
					{
						Counts.ChapterLife = State.Calling.Lives[Chapters.Last().LifeAt].LifeIndex;
						Counts.ChapterShown = Chapters.Last().Lines.Num();
						Counts.ChapterCandidates = Chapters.Last().Candidates;
						Counts.ChapterMore = Chapters.Last().bMore ? 1 : 0;
						static const FGameplayTag CallingTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Calling")));
						const TArray<FCosmicChronicleChapter> Whole = CosmicChronicle::BuildCapped(State, TNumericLimits<int32>::Max());
						FMemory::Memzero(Counts.ChapterRanks);
						for (const FCosmicChronicleLine& Line : Whole.Last().Lines)
						{
							const int32 Rank = Line.Entry.Category == CallingTag ? 0 : !Line.bOwn ? 3 : Line.Entry.Severity >= 2 ? 1 : 2;
							++Counts.ChapterRanks[Rank];
						}
					}
				}
			}

			// The ruler's input between ticks, before anything below reads the tick: its lines carry this tick and are counted
			// with it, and a hazard an answer ends is seen ending in it.
			Counts.CardsCome += (!State.Knowing.PendingPetitionRow.IsNone() && State.Knowing.PendingPetitionRow != CardBefore) ? 1 : 0;
			for (int32 Guard = 0; Answer != 0 && Guard < 8 && !State.Knowing.PendingPetitionRow.IsNone(); ++Guard)
			{
				const FName Card = State.Knowing.PendingPetitionRow;
				const FEncounterDef* Row = Sim->GetTables().FindEncounter(Card);
				const int32 Choice = Row ? PolicyChoiceOf(State, Sim->GetStateIndex(), *Row, Counts.Answers) : INDEX_NONE;
				if (Choice == INDEX_NONE)
				{
					break;
				}
				const FCosmicPetitionAnswer Taken = Role->AnswerPetition(Choice);
				if (Taken.Outcome == ECosmicPetitionOutcome::Refused)
				{
					break;
				}
				Counts.Answers += (Taken.Outcome == ECosmicPetitionOutcome::Withdrawn) ? 0 : 1;
				Counts.CardsCome += (!State.Knowing.PendingPetitionRow.IsNone() && State.Knowing.PendingPetitionRow != Card) ? 1 : 0;
			}
			for (int32 Guard = 0; Build != 0 && Guard < 8 && Role->StartNextMegaproject(); ++Guard)
			{
				++Counts.Started;
			}

			if (Counts.FirstTwoSettledTick < 0 && HoldsTwoSettled(State))
			{
				Counts.FirstTwoSettledTick = State.Tick;
			}
			// A hazard is placed when a slot holds a tag it did not hold or holds it with a full count again; it ended when
			// its tag is gone or counted again. An ended hazard was answered when the answer in its response slot completed on
			// this tick (its completion record moved to this tick), and it expired when it was not answered and its count was
			// 1 before the tick (the crisis stage runs the answers before the countdown). A hazard placed and ended inside one
			// tick is not seen here (its lines are in the rows file).
			for (int32 SystemIndex = 0; SystemIndex < State.Systems.Num() && SystemIndex < Before.Num(); ++SystemIndex)
			{
				const FStarSystemState& System = State.Systems[SystemIndex];
				const FSlotBefore& Slot = Before[SystemIndex];
				const bool bAgain = System.ActiveHazard == Slot.Tag && System.HazardTicksRemaining >= Slot.Remaining;
				if (Slot.Tag.IsValid() && (System.ActiveHazard != Slot.Tag || bAgain))
				{
					const FString Tag = Slot.Tag.ToString();
					Counts.Ended.FindOrAdd(Tag)++;
					const bool bAnswered = !Slot.Answer.IsNone() && CosmicMegaprojects::HasCompleted(System, Slot.Answer, State.Tick);
					if (bAnswered)
					{
						Counts.Answered.FindOrAdd(Tag)++;
					}
					else if (Slot.Remaining == 1)
					{
						Counts.Expired.FindOrAdd(Tag)++;
					}
					if (Slot.Tag == ClaimTag)
					{
						if (Counts.FirstClaimEndTick < 0 && System.SystemID == Counts.FirstClaimSystemID)
						{
							Counts.FirstClaimEndTick = State.Tick;
						}
						FClaimEnds& Ends = InSomeSphere(State, System.SystemID) ? Counts.ClaimsJoined : Counts.ClaimsStilled;
						++Ends.Count;
						Ends.WithAnswer += Slot.Answer.IsNone() ? 0 : 1;
						Ends.OwnerInfluence += Slot.OwnerInfluence;
					}
				}
				if (System.ActiveHazard.IsValid() && (System.ActiveHazard != Slot.Tag || bAgain))
				{
					Counts.Placed.FindOrAdd(System.ActiveHazard.ToString())++;
					if (System.ActiveHazard == ClaimTag)
					{
						if (Counts.FirstClaimTick < 0)
						{
							Counts.FirstClaimTick = State.Tick;
							Counts.FirstClaimSystemID = System.SystemID;
						}
						Counts.ClaimsAfterTwoSettled += Counts.FirstTwoSettledTick >= 0 ? 1 : 0;
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
			if (const FCivilizationState* Ruled = State.Civs.FindByPredicate([](const FCivilizationState& Civ) { return Civ.bIsPlayerRuled; }))
			{
				Counts.RuledT2Tick = (Counts.RuledT2Tick < 0 && Ruled->Tier >= EAscensionTier::Tier2_Stellar) ? State.Tick : Counts.RuledT2Tick;
				Counts.RuledT3Tick = (Counts.RuledT3Tick < 0 && Ruled->Tier >= EAscensionTier::Tier3_Causal) ? State.Tick : Counts.RuledT3Tick;
			}
			if (State.Tick == 600 || State.Tick == 6000)
			{
				(State.Tick == 600 ? Counts.DecisionsAt600 : Counts.DecisionsAt6000) = Counts.Answers + Counts.Started;
				(State.Tick == 600 ? Counts.ToldAt600 : Counts.ToldAt6000) = Counts.ToldLines;
			}

			if ((Done + 1) % SampleEveryTicks == 0 || Done + 1 == Ticks)
			{
				++Samples;
				FString SaveBytes;
				if (Samples % SaveEverySamples == 0 || Done + 1 == Ticks)
				{
					SaveBytes = FString::FromInt(SaveBytesOf(Save, State, Unused));
				}
				AppendSample(Csv, *Sim, Before, Counts, SampleTicks > 0 ? SampleMicros / SampleTicks : 0.0, SaveBytes);
				Counts.ExpansionPairs += PeoplesNotRuled(State);
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
			const int32 SaveBytes = SaveBytesOf(Save, State, SaveMilliseconds);
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance summary: tell-all %d, %d systems, %d peoples; claims %d placed, %d ended, %d joined; first claim %lld on %s until %lld, first join %lld; first two-settled people %lld, claims after it %d; sphere %d->%d; wild %d; vessel deposed at %lld; tick us mean %.1f p99 %.1f max %.1f; save serialized %d bytes in %.1f ms; log %d, chronicle %d, told %lld"),
				TellAll, State.Systems.Num(), State.Civs.Num(), Counts.Placed.FindRef(ClaimTag.ToString()), Counts.ClaimsJoined.Count + Counts.ClaimsStilled.Count,
				Counts.ClaimsJoined.Count, Counts.FirstClaimTick, *Counts.FirstClaimSystemID.ToString(), Counts.FirstClaimEndTick, Counts.FirstJoinTick, Counts.FirstTwoSettledTick, Counts.ClaimsAfterTwoSettled, SphereStart,
				SphereSize(State), WildSystems(State), Counts.DeposedTick,
				TickMicros.Num() ? Sum / TickMicros.Num() : 0.0, TickMicros.Num() ? TickMicros[FMath::Min(TickMicros.Num() - 1, static_cast<int32>(TickMicros.Num() * 0.99))] : 0.0,
				TickMicros.Num() ? TickMicros.Last() : 0.0, SaveBytes, SaveMilliseconds, State.Log.Num(), State.Chronicle.Num(), Counts.ToldLines);
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance claims: %s | %s; spheres %s -> %s"), *ClaimEndsText(TEXT("joined"), Counts.ClaimsJoined),
				*ClaimEndsText(TEXT("stilled"), Counts.ClaimsStilled), *FString::Join(SpheresStart, TEXT(", ")), *FString::Join(SpheresOf(State), TEXT(", ")));
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance hazards: placed%s | answered%s | expired%s | ended%s"), *CountsText(Counts.Placed), *CountsText(Counts.Answered),
				*CountsText(Counts.Expired), *CountsText(Counts.Ended));
			// Why the AI did not widen its realm (section 9.3): one reason per people the player does not rule at every
			// sample, the tick-0 sample included. The reasons add up to the pairs counted apart, or an error line says so.
			int64 ExpansionSum = 0;
			for (const TPair<FString, int32>& Each : Counts.Expansion)
			{
				ExpansionSum += Each.Value;
			}
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance expansion: %lld (sample, people) pairs of peoples the player does not rule, the tick-0 sample included;%s"),
				Counts.ExpansionPairs, *CountsText(Counts.Expansion));
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance ruler: answer %d build %d; cards %d, answers %d, started %d, decisions %d (by 600 %lld, by 6000 %lld); told by 600 %lld, by 6000 %lld; ruled people T2 at %lld, T3 at %lld"),
				Answer, Build, Counts.CardsCome, Counts.Answers, Counts.Started, Counts.Answers + Counts.Started, Counts.DecisionsAt600, Counts.DecisionsAt6000,
				Counts.ToldAt600, Counts.ToldAt6000, Counts.RuledT2Tick, Counts.RuledT3Tick);
			UE_LOG(LogCosmic, Display, TEXT("CosmicBalance end: first %s at %lld; rung at %lld, clock at %lld, deposed at %lld; chronicle life %d shown %d of %d, more %d, ranks %d/%d/%d/%d"),
				CosmicEndings::NameOf(Counts.FirstEnd), Counts.FirstEndTick, Counts.RungTick, Counts.ClockTick, Counts.DeposedTick, Counts.ChapterLife,
				Counts.ChapterShown, Counts.ChapterCandidates, Counts.ChapterMore, Counts.ChapterRanks[0], Counts.ChapterRanks[1], Counts.ChapterRanks[2], Counts.ChapterRanks[3]);
			// The rung end and the ruled people's third rung are the same tick, but for a deposition in that very tick (the
			// feats are judged before the tenth stage deposes; afterwards no people is ruled). A measure, not a gate: Result
			// stays, and the line has a prefix of its own so that a tool taking the last "end:" line never takes it.
			const bool bSameTickDeposition = Counts.RuledT3Tick < 0 && Counts.DeposedTick >= 0 && Counts.DeposedTick == Counts.RungTick;
			if (Counts.RungTick != Counts.RuledT3Tick && !bSameTickDeposition)
			{
				UE_LOG(LogCosmic, Display, TEXT("CosmicBalance end check: the rung end at %lld is not the ruled people's third rung at %lld"), Counts.RungTick, Counts.RuledT3Tick);
			}
			if (ExpansionSum != Counts.ExpansionPairs)
			{
				UE_LOG(LogCosmic, Error, TEXT("CosmicBalance expansion: the counts add up to %lld, not %lld"), ExpansionSum, Counts.ExpansionPairs);
				Result = 1;
			}
		}
		else
		{
			UE_LOG(LogCosmic, Error, TEXT("CosmicBalance: could not write %s or %s"), *OutPath, *RowsPath);
			Result = 1;
		}
		Save->RemoveFromRoot();
	}
	else
	{
		UE_LOG(LogCosmic, Error, TEXT("CosmicBalance: no simulation or role subsystem, or -Rules refused: nothing run"));
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
