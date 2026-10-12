// Cosmic Ascension - Log layer.

#include "Log/CosmicLogSubsystem.h"

#include "Cosmic_Ascension.h"
#include "Data/CosmicDataTables.h"
#include "Engine/GameInstance.h"
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
	Append(Category, Severity, FString(FText::Format(Template, Args).ToString()), FocusSystem, FocusCiv, SourceRow);
}

bool UCosmicLogSubsystem::PushRow(FName RowName, uint8 Severity, const FFormatNamedArguments& Args, FName FocusSystem, FName FocusCiv)
{
	UGalaxySimulationSubsystem* Sim = GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>();
	check(Sim);

	const FLogTemplateDef* Row = Sim->GetTables().FindLogTemplate(RowName);
	if (!Row || Row->Templates.IsEmpty())
	{
		return false;
	}

	// The tell stream: which variant is told never moves the sim stream (section 9.2).
	const int32 Count = Row->Templates.Num();
	int32 Index = static_cast<int32>(Sim->DrawTell(Count));

	FString Rendered = FText::Format(Row->Templates[Index], Args).ToString();
	const TArray<FCosmicLogEntry>& Log = Sim->GetState().Log;
	if (Count > 1 && !Log.IsEmpty() && Log.Last().Message.ToString() == Rendered)
	{
		Index = (Index + 1) % Count;
		Rendered = FText::Format(Row->Templates[Index], Args).ToString();
	}

	Append(Row->Category, Severity, MoveTemp(Rendered), FocusSystem, FocusCiv, RowName);
	return true;
}

void UCosmicLogSubsystem::Append(FGameplayTag Category, uint8 Severity, FString&& Rendered, FName FocusSystem, FName FocusCiv, FName SourceRow)
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
	State.Log.Add(Entry);
	UE_LOG(LogCosmic, Verbose, TEXT("[%lld] %s"), Entry.Tick, *Entry.Message.ToString());

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

	if (Sim->IsBroadcastEnabled())
	{
		OnLogEntry.Broadcast(Entry);
	}
}
