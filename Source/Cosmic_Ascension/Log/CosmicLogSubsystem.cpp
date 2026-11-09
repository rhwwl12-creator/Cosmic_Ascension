// Cosmic Ascension - Log layer.

#include "Log/CosmicLogSubsystem.h"

#include "Core/CosmicTags.h"
#include "Core/CosmicKnowing.h"
#include "Core/CosmicTypes.h"
#include "Cosmic_Ascension.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "Engine/GameInstance.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"

void UCosmicLogSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogCosmic, Log, TEXT("UCosmicLogSubsystem initialized"));
}

TArray<FCosmicChronicleChapter> UCosmicLogSubsystem::BuildChronicle() const
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	return Sim ? CosmicChronicle::Build(Sim->GetState()) : TArray<FCosmicChronicleChapter>();
}

bool UCosmicLogSubsystem::HasClaimLineThisTick(FName SystemID) const
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	if (!Sim || SystemID.IsNone())
	{
		return false;
	}
	const FGalaxyState& State = Sim->GetState();
	const FGameplayTag ClaimTag = CosmicTags::LogClaim();
	// Newest first: this tick's entries are the tail of the log (and of the chronicle, when more than the log's cap of lines
	// came in one tick). An entry of a later tick - only a state a test laid - is passed by.
	for (const TArray<FCosmicLogEntry>* Entries : { &State.Log, &State.Chronicle })
	{
		for (int32 Index = Entries->Num() - 1; Index >= 0 && (*Entries)[Index].Tick >= State.Tick; --Index)
		{
			const FCosmicLogEntry& Entry = (*Entries)[Index];
			if (Entry.Tick == State.Tick && Entry.Category == ClaimTag && Entry.FocusSystemID == SystemID)
			{
				return true;
			}
		}
	}
	return false;
}

void UCosmicLogSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UCosmicLogSubsystem::Push(FGameplayTag Category, uint8 Severity, const FText& Template, const FFormatNamedArguments& Args, FName FocusSystem, FName SourceRow, FName FocusCiv)
{
	const UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("Push called from a handler of a Core Logic delegate: a handler may only read the state"));
		return;
	}
	FCosmicLine Line;
	Line.Category = Category;
	Line.Severity = Severity;
	Line.FocusSystem = FocusSystem;
	Line.FocusCiv = FocusCiv;
	Line.SourceRow = SourceRow;
	Tell(Line, FString(FText::Format(Template, Args).ToString()));
}

bool UCosmicLogSubsystem::PushVariants(const FCosmicLine& Line, TConstArrayView<FText> Variants, const FFormatNamedArguments& Args)
{
	if (const UGalaxySimulationSubsystem* Guard = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>(); Guard && Guard->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("PushVariants called from a handler of a Core Logic delegate: a handler may only read the state"));
		return false;
	}
	if (Variants.IsEmpty())
	{
		return false;
	}

	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);

	// The tell stream: which variant is told never moves the sim stream (section 9.2).
	const int32 Count = Variants.Num();
	int32 Index = static_cast<int32>(Sim->DrawTell(Count));

	// The same sentence never follows itself in the feed: the line it is held against is the last one told (an entry
	// marked bUntold is passed by).
	FString Rendered = FText::Format(Variants[Index], Args).ToString();
	if (Count > 1)
	{
		const TArray<FCosmicLogEntry>& Log = Sim->GetState().Log;
		int32 LastTold = Log.Num() - 1;
		while (LastTold >= 0 && Log[LastTold].bUntold)
		{
			--LastTold;
		}
		if (LastTold >= 0 && Log[LastTold].Message.ToString() == Rendered)
		{
			Index = (Index + 1) % Count;
			Rendered = FText::Format(Variants[Index], Args).ToString();
		}
	}

	Tell(Line, MoveTemp(Rendered));
	return true;
}

bool UCosmicLogSubsystem::PushWithSign(const FCosmicLine& Line, TConstArrayView<FText> Variants, FName SignRow, const FFormatNamedArguments& Args)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("PushWithSign called from a handler of a Core Logic delegate: a handler may only read the state"));
		return false;
	}
	if (Variants.IsEmpty())
	{
		return false;
	}
	// Tier numbers start at 1, so the lowest window with anybody below it begins at 2 (and 0 as an upper bound would mean
	// "no bound"). Whenever the data allow it, the sign is told: the windows add no tell draw (section 8.1).
	if (!SignRow.IsNone() && Line.MinViewerTier > 1)
	{
		if (const FLogTemplateDef* Sign = Sim->GetTables().FindLogTemplate(SignRow))
		{
			FCosmicLine SignLine = Line;
			SignLine.Category = Sign->Category;
			SignLine.Severity = FMath::Min<uint8>(Line.Severity, 1);
			SignLine.MinViewerTier = 0;
			SignLine.MaxViewerTier = Line.MinViewerTier - 1;
			PushVariants(SignLine, Sign->Templates, Args);
		}
		else
		{
			UE_LOG(LogCosmic, Error, TEXT("The line of %s names the sign row %s, which DT_LogTemplates does not hold"), *Line.SourceRow.ToString(), *SignRow.ToString());
		}
	}
	return PushVariants(Line, Variants, Args);
}

bool UCosmicLogSubsystem::PushRow(FName RowName, uint8 Severity, const FFormatNamedArguments& Args, FName FocusSystem, FName FocusCiv)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);

	const FLogTemplateDef* Row = Sim->GetTables().FindLogTemplate(RowName);
	if (!Row)
	{
		return false;
	}

	FCosmicLine Line;
	Line.Category = Row->Category;
	Line.Severity = Severity;
	Line.FocusSystem = FocusSystem;
	Line.FocusCiv = FocusCiv;
	Line.SourceRow = RowName;
	Line.MinViewerTier = Row->MinTierToReveal;
	return PushVariants(Line, Row->Templates, Args);
}

void UCosmicLogSubsystem::Tell(const FCosmicLine& Line, FString&& Rendered)
{
	static const FGameplayTag CallingTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Calling")));
	static const FGameplayTag ClaimTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Claim")));
	static const FGameplayTag RecordTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Record")));
	static const FGameplayTag AscensionTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Ascension")));

	// The first part of the two books (B5 decision 11-na): a record line is kept untold whatever the scope and the tell-all
	// mode - never announced, never in the feed, and no far line stands in for it; the two books read it. Its variant was
	// drawn all the same.
	if (Line.Category == RecordTag)
	{
		Append(Line.Category, Line.Severity, MoveTemp(Rendered), Line.FocusSystem, Line.FocusCiv, Line.SourceRow, true);
		return;
	}
	if (bTellAll)
	{
		Append(Line.Category, Line.Severity, MoveTemp(Rendered), Line.FocusSystem, Line.FocusCiv, Line.SourceRow, false);
		return;
	}

	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	const FGalaxyState& State = Sim->GetState();
	const FKnowingState& Knowing = State.Knowing;
	const EAscensionTier Viewer = Sim->GetViewerTier();

	// The subject (section 8.1): a met people not far above the viewer (the one Core predicate CosmicKnowing::IsFarAbove -
	// a lower one is never held back), and a known system. The calling's own lines always hold.
	const int32 CivIndex = Line.FocusCiv.IsNone() ? INDEX_NONE : Sim->GetStateIndex().FindCiv(State, Line.FocusCiv);
	const FCivilizationState* Civ = State.Civs.IsValidIndex(CivIndex) ? &State.Civs[CivIndex] : nullptr;
	bool bSubject = true;
	if (Line.Category != CallingTag)
	{
		if (!Line.FocusCiv.IsNone())
		{
			const FName CivID = Line.FocusCiv;
			const bool bMet = Knowing.MetCivs.ContainsByPredicate([CivID](const FKnownMark& Mark) { return Mark.ID == CivID; });
			const bool bFarAbove = Civ && CosmicKnowing::IsFarAbove(Civ->Tier, Viewer);
			bSubject = bMet && !bFarAbove;
		}
		if (bSubject && !Line.FocusSystem.IsNone())
		{
			const FName SystemID = Line.FocusSystem;
			bSubject = Knowing.KnownSystems.ContainsByPredicate([SystemID](const FKnownMark& Mark) { return Mark.ID == SystemID; });
		}
	}
	// The window of the viewer's tier number (0 = no bound on that side).
	const int32 ViewerNumber = static_cast<int32>(Viewer) + 1;
	const bool bWindow = (Line.MinViewerTier == 0 || ViewerNumber >= Line.MinViewerTier) && (Line.MaxViewerTier == 0 || ViewerNumber <= Line.MaxViewerTier);

	if (bSubject && bWindow)
	{
		Append(Line.Category, Line.Severity, MoveTemp(Rendered), Line.FocusSystem, Line.FocusCiv, Line.SourceRow, false);
		return;
	}
	// Out of scope: the mythic lines and the first person of a claim (the two books' second part) are kept untold, the rest is
	// not stored.
	if (Line.Severity >= 2 || Line.Category == ClaimTag)
	{
		Append(Line.Category, Line.Severity, MoveTemp(Rendered), Line.FocusSystem, Line.FocusCiv, Line.SourceRow, true);
	}

	// The clock is read (section 8.1): a mythic line about a subject out of scope - not one hidden by its window alone -
	// is followed by a far line with no subject. An ascent to the third rung or above is told every time; anything else
	// at most once per FarLineCooldownTicks.
	if (bSubject || Line.Severity < 2)
	{
		return;
	}
	if (Line.Category == AscensionTag && Civ && static_cast<int32>(Civ->Tier) >= static_cast<int32>(EAscensionTier::Tier3_Causal))
	{
		PushRow(CosmicRows::LogFarAscent, 1, FFormatNamedArguments(), NAME_None);
		return;
	}
	FGalaxyState& Mutable = Sim->GetMutableState();
	if (Mutable.Tick >= Mutable.Knowing.FarLineUntilTick && PushRow(CosmicRows::LogFarEvent, 1, FFormatNamedArguments(), NAME_None))
	{
		const int32 Cooldown = Sim->GetRules().FarLineCooldownTicks > 0 ? Sim->GetRules().FarLineCooldownTicks : 0;
		Mutable.Knowing.FarLineUntilTick = Mutable.Tick + Cooldown;
	}
}

void UCosmicLogSubsystem::TrimChronicle(TArray<FCosmicLogEntry>& Chronicle, FChronicleEviction& Eviction, int32 Cap)
{
	if (Cap <= 0 || Chronicle.Num() <= Cap)
	{
		return;
	}
	// One stable pass: the oldest entries the cap may let go leave until the chronicle is at the cap or none is left; the rest
	// keeps its order (removing one by one from a full chronicle would be quadratic).
	int32 Excess = Chronicle.Num() - Cap;
	int32 Write = 0;
	for (int32 Read = 0; Read < Chronicle.Num(); ++Read)
	{
		if (Excess > 0 && CosmicChronicle::MayLetGo(Chronicle[Read]))
		{
			++Eviction.Count;
			Eviction.ThroughTick = FMath::Max(Eviction.ThroughTick, Chronicle[Read].Tick);
			--Excess;
			continue;
		}
		if (Write != Read)
		{
			Chronicle[Write] = MoveTemp(Chronicle[Read]);
		}
		++Write;
	}
	Chronicle.SetNum(Write, EAllowShrinking::No);
}

void UCosmicLogSubsystem::Append(FGameplayTag Category, uint8 Severity, FString&& Rendered, FName FocusSystem, FName FocusCiv, FName SourceRow, bool bUntold)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);
	FGalaxyState& State = Sim->GetMutableState();

	FCosmicLogEntry Entry;
	Entry.Tick = State.Tick;
	Entry.Category = Category;
	Entry.Severity = Severity;
	Entry.Message = FText::AsCultureInvariant(MoveTemp(Rendered));
	Entry.FocusSystemID = FocusSystem;
	Entry.FocusCivID = FocusCiv;
	Entry.SourceRow = SourceRow;
	Entry.bUntold = bUntold;
	State.Log.Add(Entry);
	UE_LOG(LogCosmic, Verbose, TEXT("[%lld]%s %s"), Entry.Tick, bUntold ? TEXT(" (untold)") : TEXT(""), *Entry.Message.ToString());

	// Over the cap the oldest entries leave. What the chronicle can show - CosmicChronicle::Admits over the peoples of every
	// life (MVP item 5, 16 #40 (7)) - moves to the chronicle; the rest is dropped (section 8.4). Then the chronicle's own cap.
	if (State.Log.Num() > MaxLogEntries)
	{
		const TArray<FName> Peoples = CosmicChronicle::PeoplesOfEveryLife(State.Calling);
		bool bMoved = false;
		while (State.Log.Num() > MaxLogEntries)
		{
			if (CosmicChronicle::Admits(State.Log[0], Peoples))
			{
				State.Chronicle.Add(State.Log[0]);
				bMoved = true;
			}
			State.Log.RemoveAt(0, 1, EAllowShrinking::No);
		}
		if (bMoved)
		{
			TrimChronicle(State.Chronicle, State.ChronicleEviction, Sim->GetRules().ChronicleCap);
		}
	}

	// An untold entry is kept for the chronicle and the two books, never announced (section 8.1).
	if (!bUntold && Sim->IsBroadcastEnabled())
	{
		Sim->BeginBroadcast();
		OnLogEntry.Broadcast(Entry);
		Sim->EndBroadcast();
	}
}
