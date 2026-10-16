// Cosmic Ascension - Log layer.

#include "Log/CosmicLogSubsystem.h"

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

void UCosmicLogSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UCosmicLogSubsystem::Push(FGameplayTag Category, uint8 Severity, const FText& Template, const FFormatNamedArguments& Args, FName FocusSystem, FName SourceRow, FName FocusCiv)
{
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

	// The subject (section 8.1): a met people not three rungs or more above the viewer (a lower one is never held
	// back), and a known system. The calling's own lines always hold.
	const int32 CivIndex = Line.FocusCiv.IsNone() ? INDEX_NONE : Sim->GetStateIndex().FindCiv(State, Line.FocusCiv);
	const FCivilizationState* Civ = State.Civs.IsValidIndex(CivIndex) ? &State.Civs[CivIndex] : nullptr;
	bool bSubject = true;
	if (Line.Category != CallingTag)
	{
		if (!Line.FocusCiv.IsNone())
		{
			const FName CivID = Line.FocusCiv;
			const bool bMet = Knowing.MetCivs.ContainsByPredicate([CivID](const FKnownMark& Mark) { return Mark.ID == CivID; });
			const bool bFarAbove = Civ && static_cast<int32>(Civ->Tier) - static_cast<int32>(Viewer) >= 3;
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
	// Out of scope: the mythic lines and the two books' lines are kept untold, the rest is not stored.
	if (Line.Severity >= 2 || Line.Category == ClaimTag || Line.Category == RecordTag)
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

void UCosmicLogSubsystem::Append(FGameplayTag Category, uint8 Severity, FString&& Rendered, FName FocusSystem, FName FocusCiv, FName SourceRow, bool bUntold)
{
	static const FGameplayTag ClaimTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Claim")));
	static const FGameplayTag RecordTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Log.Record")));

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

	// Over the cap the oldest entries leave: notable and mythic ones, and every claim and record, move to the chronicle;
	// ordinary ones are dropped (section 8.4).
	while (State.Log.Num() > MaxLogEntries)
	{
		const FCosmicLogEntry& Oldest = State.Log[0];
		if (Oldest.Severity >= 1 || Oldest.Category == ClaimTag || Oldest.Category == RecordTag)
		{
			State.Chronicle.Add(Oldest);
		}
		State.Log.RemoveAt(0, 1, EAllowShrinking::No);
	}

	// An untold entry is kept for the chronicle and the two books, never announced (section 8.1).
	if (!bUntold && Sim->IsBroadcastEnabled())
	{
		Sim->BeginBroadcast();
		OnLogEntry.Broadcast(Entry);
		Sim->EndBroadcast();
	}
}
