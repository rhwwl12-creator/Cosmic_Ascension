// Cosmic Ascension - World layer (Presentation).

#include "World/CosmicDigest.h"

#include "Core/CosmicSeed.h"
#include "Core/CosmicTags.h"
#include "Core/CosmicTypes.h"
#include "Core/GalaxyState.h"
#include "Data/CosmicConditionTypes.h"
#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Data/CosmicRowNames.h"
#include "GameplayTagContainer.h"
#include "Internationalization/Text.h"
#include "Log/CosmicChronicle.h"
#include "Sim/CosmicHazards.h"
#include "Sim/CosmicStateIndex.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "UI/CosmicUILabels.h"
#include "World/CosmicMapLook.h"

// Helpers of this file. A named namespace: the module is built as one unity file.
namespace CosmicDigestImpl
{
	// The variant keys of each item (ST_CA), in the order of ECosmicDigestItem. Display only: nothing is saved or hashed,
	// but an order or count changed here changes every page's choice - append variants only with VariantCount.
	const TCHAR* const LostPeopleKeys[] = { TEXT("Digest.LostPeople.1"), TEXT("Digest.LostPeople.2"), TEXT("Digest.LostPeople.3") };
	const TCHAR* const RoseKeys[] = { TEXT("Digest.Rose.1"), TEXT("Digest.Rose.2"), TEXT("Digest.Rose.3") };
	const TCHAR* const LostKeys[] = { TEXT("Digest.Lost.1"), TEXT("Digest.Lost.2"), TEXT("Digest.Lost.3") };
	const TCHAR* const AnnexedKeys[] = { TEXT("Digest.Annexed.1"), TEXT("Digest.Annexed.2"), TEXT("Digest.Annexed.3") };
	const TCHAR* const OwnerKeys[] = { TEXT("Digest.Owner.1"), TEXT("Digest.Owner.2"), TEXT("Digest.Owner.3") };
	const TCHAR* const ThroneKeys[] = { TEXT("Digest.Throne.1"), TEXT("Digest.Throne.2"), TEXT("Digest.Throne.3") };
	const TCHAR* const ThroneUnnamedKeys[] = { TEXT("Digest.ThroneUnnamed.1"), TEXT("Digest.ThroneUnnamed.2"), TEXT("Digest.ThroneUnnamed.3") };
	static_assert(static_cast<int32>(UE_ARRAY_COUNT(LostPeopleKeys)) == CosmicDigest::VariantCount && static_cast<int32>(UE_ARRAY_COUNT(RoseKeys)) == CosmicDigest::VariantCount
		&& static_cast<int32>(UE_ARRAY_COUNT(LostKeys)) == CosmicDigest::VariantCount && static_cast<int32>(UE_ARRAY_COUNT(AnnexedKeys)) == CosmicDigest::VariantCount
		&& static_cast<int32>(UE_ARRAY_COUNT(OwnerKeys)) == CosmicDigest::VariantCount && static_cast<int32>(UE_ARRAY_COUNT(ThroneKeys)) == CosmicDigest::VariantCount
		&& static_cast<int32>(UE_ARRAY_COUNT(ThroneUnnamedKeys)) == CosmicDigest::VariantCount, "Every digest item has VariantCount keys");

	// What the lines of the span say about one system the viewer knows.
	struct FSystemNote
	{
		bool bTouched = false;
		// The tick of the last line about it.
		int64 LastTick = 0;
		// The owner the last and the first hazard or project line about it named (those lines name the owner at the time: the
		// destroyed system's is the last).
		FName LastCivID;
		bool bFirstOwner = false;
		FName FirstOwnerID;
		// A line of a claim row about it, and the tick of the last one.
		bool bClaim = false;
		int64 ClaimTick = 0;
	};

	// One item before it is rendered: its kind, subject and tick, and the order it was found in (systems, then peoples).
	struct FFound
	{
		ECosmicDigestItem Kind = ECosmicDigestItem::Lost;
		FName SubjectID;
		FName CivID;
		int64 Tick = 0;
		int32 Order = 0;
		FText Civ;
		FText System;
		FText Ruler;
	};

	// The cap's rank of a kind: the enumerator's order (a people gone first, a seat left last).
	int32 RankOf(ECosmicDigestItem Kind)
	{
		return static_cast<int32>(Kind);
	}

	// Lines a far line tells (no subject: news from beyond what the viewer knows - section 8.1).
	bool IsFarLine(const FCosmicLogEntry& Entry)
	{
		return Entry.SourceRow == CosmicRows::LogFarEvent || Entry.SourceRow == CosmicRows::LogFarAscent;
	}

	// A hazard or project line: the rows whose lines name the owner of their system at the time they are told (the hazard
	// stage's TellHazardLine and the completion line both pass the system's owner).
	bool NamesOwner(const FCosmicLogEntry& Entry, const FCosmicDataTables& Tables)
	{
		return !Entry.SourceRow.IsNone() && (Tables.FindHazard(Entry.SourceRow) || Tables.FindMegaproject(Entry.SourceRow));
	}

	// A line of a claim row (the row the claim rule places: SpawnRule ClaimRule - its tag is Hazard.Claim, the schema).
	bool IsClaimLine(const FCosmicLogEntry& Entry, const FCosmicDataTables& Tables)
	{
		const FHazardDef* Row = Entry.SourceRow.IsNone() ? nullptr : Tables.FindHazard(Entry.SourceRow);
		return Row && Row->SpawnRule == ECosmicHazardSpawn::ClaimRule;
	}

	// Whether a transcendent's sphere holds the system now.
	bool InAnySphere(const FGalaxyState& State, FName SystemID)
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

	// The name of the people's open Notable.Ruler record (the latest begun): Bank[NameIndex % Bank.Num()] of its archetype,
	// arch_default for none (the bank NameNotable drew it from); empty when it has none or the bank is empty.
	FText RulerNameOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables)
	{
		const FNotableRecord* Open = nullptr;
		for (const FNotableRecord& Record : Civ.Notables)
		{
			if (Record.LastTick == 0 && Record.Role == CosmicTags::NotableRuler() && (!Open || Record.FirstTick > Open->FirstTick))
			{
				Open = &Record;
			}
		}
		const FCivArchetypeDef* Archetype = Tables.FindCiv(Civ.ArchetypeRow.IsNone() ? CosmicRows::ArchDefault : Civ.ArchetypeRow);
		if (!Open || !Archetype || Archetype->PersonNameBank.IsEmpty())
		{
			return FText::GetEmpty();
		}
		const int32 Count = Archetype->PersonNameBank.Num();
		return Archetype->PersonNameBank[((Open->NameIndex % Count) + Count) % Count];
	}

	// The rows of a rebellion (section 4.1, 7.4 notable hook 2), found by the tag - no row name: the hazard row of
	// Hazard.Rebellion (its expiry and resolution lines carry it as their SourceRow) and its outbreak row, the event that
	// names the rebel (the record carries it as its SourceRow). Both None - the converted rebel is then not read - when the
	// tables have no such row or outbreak row, when the row has no expiry line (an expiry told by no line leaves nothing to
	// find), or when the outbreak row names a ruler or a regent itself (such a record would be no rebel made the ruler).
	struct FRebellionRows
	{
		FName Hazard;
		FName Outbreak;
	};

	FRebellionRows RebellionRowsOf(const FCosmicDataTables& Tables)
	{
		FRebellionRows Rows;
		FName HazardRow;
		const FHazardDef* Row = Tables.FindHazardByTag(CosmicTags::HazardRebellion(), &HazardRow);
		const FCosmicEventDef* Outbreak = (Row && !Row->OutbreakRow.IsNone()) ? Tables.FindEvent(Row->OutbreakRow) : nullptr;
		if (!Outbreak || Row->ExpireTemplates.IsEmpty() || Outbreak->Effects.ContainsByPredicate([](const FCosmicEffect& Effect)
			{
				return Effect.Type == EEffectType::NameNotable && (Effect.Tag == CosmicTags::NotableRuler() || Effect.Tag == CosmicTags::NotableRegent());
			}))
		{
			return Rows;
		}
		Rows.Hazard = HazardRow;
		Rows.Outbreak = Row->OutbreakRow;
		return Rows;
	}

	// The tick a rebellion made Record the people's ruler - a Notable.Ruler record the outbreak row named, so a rebel: only
	// ChangeThrone turns a record into a ruler, at the expiry of the rebellion that stood on the people's capital when the
	// rebel was named. A people holds one rebellion at a time (the capital holds it, and the capital does not move while it
	// stands), and every other end of it - a resolution, a transfer, the unseating, a rebel named in its place - closes the
	// rebel; so the expiry line is the first line of the hazard row about the people after the record's FirstTick (the
	// expiries of a tick run before any rebel of that tick is named). Read from the outbreak's own told line on, in State.Log
	// alone: the log keeps every line told after its first, so no line after the outbreak is missing, and the expiry - about
	// the same people and system - was told as the outbreak line was (knowledge only grows; a people below the fourth rung is
	// never three rungs above a viewer). 0 when the outbreak line is not there: the item would rather say nothing than name a
	// throne that did not change.
	int64 TakenTickOf(const FGalaxyState& State, FName CivID, const FNotableRecord& Record, const FRebellionRows& Rows)
	{
		int32 At = 0;
		while (At < State.Log.Num() && !(State.Log[At].Tick == Record.FirstTick && State.Log[At].SourceRow == Rows.Outbreak
			&& State.Log[At].FocusCivID == CivID && !State.Log[At].bUntold))
		{
			++At;
		}
		for (++At; At < State.Log.Num(); ++At)
		{
			const FCosmicLogEntry& Entry = State.Log[At];
			if (Entry.Tick > Record.FirstTick && Entry.SourceRow == Rows.Hazard && Entry.FocusCivID == CivID)
			{
				return Entry.Tick;
			}
		}
		return 0;
	}

	// The last tick in (After, Through] at which the people's seat changed hands; 0 for none. Only marks that cannot stand
	// without a change: a Ruler or Regent record that closed in the span (a rebellion that took the throne closes them, and so
	// does a ruler named in their place), and a rebel that a rebellion made the ruler in the span (TakenTickOf - the first
	// rebel to take a throne with no record before it closes none). A people of the fourth rung or above has no persons (its
	// records all closed at its climb - section 3.2): none for it.
	int64 SeatChangedTickOf(const FGalaxyState& State, const FCivilizationState& Civ, int64 After, int64 Through, const FRebellionRows& Rows)
	{
		if (static_cast<int32>(Civ.Tier) >= static_cast<int32>(EAscensionTier::Tier4_Transcendent))
		{
			return 0;
		}
		int64 Changed = 0;
		for (const FNotableRecord& Record : Civ.Notables)
		{
			if ((Record.Role == CosmicTags::NotableRuler() || Record.Role == CosmicTags::NotableRegent()) && Record.LastTick > 0 && Record.LastTick > After && Record.LastTick <= Through)
			{
				Changed = FMath::Max(Changed, Record.LastTick);
			}
			if (Record.Role == CosmicTags::NotableRuler() && !Rows.Outbreak.IsNone() && Record.SourceRow == Rows.Outbreak)
			{
				const int64 Taken = TakenTickOf(State, Civ.CivID, Record, Rows);
				if (Taken > After && Taken <= Through)
				{
					Changed = FMath::Max(Changed, Taken);
				}
			}
		}
		return Changed;
	}

	FText Render(const FFound& Item, int32 Variant)
	{
		const TCHAR* Key = CosmicDigest::KeyOf(Item.Kind, Variant);
		if (!Key)
		{
			return FText::GetEmpty();
		}
		FFormatNamedArguments Args;
		Args.Add(TEXT("System"), Item.System);
		Args.Add(TEXT("Civ"), Item.Civ);
		Args.Add(TEXT("Ruler"), Item.Ruler);
		return FText::Format(CosmicUI::Label(Key), Args);
	}
}

const TCHAR* CosmicDigest::KeyOf(ECosmicDigestItem Kind, int32 Variant)
{
	using namespace CosmicDigestImpl;

	const int32 At = FMath::Clamp(Variant, 0, VariantCount - 1);
	switch (Kind)
	{
	case ECosmicDigestItem::LostPeople:
		return LostPeopleKeys[At];
	case ECosmicDigestItem::Rose:
		return RoseKeys[At];
	case ECosmicDigestItem::Lost:
		return LostKeys[At];
	case ECosmicDigestItem::Annexed:
		return AnnexedKeys[At];
	case ECosmicDigestItem::Owner:
		return OwnerKeys[At];
	case ECosmicDigestItem::Throne:
		return ThroneKeys[At];
	case ECosmicDigestItem::ThroneUnnamed:
		return ThroneUnnamedKeys[At];
	}
	return nullptr;
}

int32 CosmicDigest::VariantOf(int32 Seed, FName SubjectID, int64 Tick)
{
	return static_cast<int32>(CosmicSeed::DeriveForName(Seed, SubjectID, CosmicSeed::Purpose::Digest, static_cast<uint32>(Tick)) % static_cast<uint32>(VariantCount));
}

int32 CosmicDigest::LastClosedLifeOf(const FGalaxyState& State)
{
	const TArray<FLifeRecord>& Lives = State.Calling.Lives;
	const int32 At = State.Calling.LifeState == ELifeState::Interregnum ? Lives.Num() - 1 : Lives.Num() - 2;
	return (Lives.IsValidIndex(At) && Lives[At].DiedTick > 0) ? At : INDEX_NONE;
}

FCosmicDigest CosmicDigest::DigestOf(const UGalaxySimulationSubsystem& Sim, int32 ClosedLifeIndex, bool bSeeAll)
{
	FCosmicDigest Out;
	const FGalaxyState& State = Sim.GetState();
	const TArray<FLifeRecord>& Lives = State.Calling.Lives;
	if (!Lives.IsValidIndex(ClosedLifeIndex) || Lives[ClosedLifeIndex].DiedTick <= 0)
	{
		return Out;
	}
	const int64 After = Lives[ClosedLifeIndex].DiedTick;
	int64 Through = 0;
	// The life called after it: its BornTick tick is read up to where its chapter opens, with the knowledge from before it.
	const FLifeRecord* Next = Lives.IsValidIndex(ClosedLifeIndex + 1) ? &Lives[ClosedLifeIndex + 1] : nullptr;
	if (Next)
	{
		Through = Next->BornTick;
	}
	else if (State.Calling.LifeState == ELifeState::Interregnum)
	{
		// The interregnum lasts (the run ended in it, or the page is asked before the leap ran): up to the state's tick.
		Through = State.Tick;
	}
	else
	{
		return Out;
	}
	if (Through == After)
	{
		// A leap of 0 stood the next life up on the tick of the death; the interregnum at its own DiedTick has nothing yet.
		Out.AfterTick = After;
		Out.ThroughTick = Through;
		Out.bNoLeap = Next != nullptr;
		Out.bValid = Out.bNoLeap;
		return Out;
	}
	return DigestOver(Sim, After, Through, bSeeAll, Next);
}

FCosmicDigest CosmicDigest::DigestOver(const UGalaxySimulationSubsystem& Sim, int64 AfterTick, int64 ThroughTick, bool bSeeAll)
{
	return DigestOver(Sim, AfterTick, ThroughTick, bSeeAll, nullptr);
}

FCosmicDigest CosmicDigest::DigestOver(const UGalaxySimulationSubsystem& Sim, int64 AfterTick, int64 ThroughTick, bool bSeeAll, const FLifeRecord* Next)
{
	using namespace CosmicDigestImpl;

	FCosmicDigest Out;
	Out.AfterTick = AfterTick;
	Out.ThroughTick = ThroughTick;
	if (ThroughTick <= AfterTick)
	{
		return Out;
	}
	Out.bValid = true;

	const FGalaxyState& State = Sim.GetState();
	const FCosmicStateIndex& Index = Sim.GetStateIndex();
	const FCosmicDataTables& Tables = Sim.GetTables();
	// The one reading of knowledge (invariant rule 14), with the viewer tier of now (no tier of the past is kept): after a
	// closed life, what the calling knew before the next life - the view cut at its BornTick, so what the succession's
	// Install marked there is out of knowledge (Digest.Elsewhere); with no next life, the view the map and the panels read.
	const FCosmicKnownView View = Next ? CosmicMapLook::KnownViewOf(State, Index, Sim.GetViewerTier(), bSeeAll, Next->BornTick)
		: CosmicMapLook::KnownViewOf(State, Index, Sim.GetViewerTier(), bSeeAll);
	const FRebellionRows Rebellion = RebellionRowsOf(Tables);

	TArray<FSystemNote> Systems;
	Systems.SetNum(State.Systems.Num());
	TArray<int64> RoseTicks;
	RoseTicks.Init(0, State.Civs.Num());

	// The lines of the span, the chronicle before the log (older entries moved there; each is in time order): the world's
	// lines only - never Log.Calling (the vessel's lines), Log.Claim (the first person) or Log.Record (the two books') -,
	// and on the next life's BornTick tick only those before the line that opens its chapter (the chronicle's own cut).
	bool bOpened = false;
	auto Read = [&](const TArray<FCosmicLogEntry>& Entries)
	{
		for (const FCosmicLogEntry& Entry : Entries)
		{
			if (Next && Entry.Tick == Next->BornTick && CosmicChronicle::OpensCalledChapter(Entry, *Next))
			{
				bOpened = true;
			}
			if (bOpened)
			{
				break;
			}
			if (Entry.Tick <= AfterTick || Entry.Tick > ThroughTick || Entry.Category == CosmicTags::LogCalling() || Entry.Category == CosmicTags::LogClaim()
				|| Entry.Category == CosmicTags::LogRecord())
			{
				continue;
			}
			if (IsFarLine(Entry))
			{
				Out.bElsewhere = true;
				continue;
			}
			const int32 SystemIndex = Entry.FocusSystemID.IsNone() ? INDEX_NONE : Index.FindSystem(State, Entry.FocusSystemID);
			const int32 CivIndex = Entry.FocusCivID.IsNone() ? INDEX_NONE : Index.FindCiv(State, Entry.FocusCivID);
			const bool bSystemKnown = View.IsKnown(SystemIndex);
			// The subject rule of the scope of telling, as the view answers it: a system the view knows and a people it shows -
			// a gone people only through a known system (its name no longer resolves).
			const bool bSystemSeen = Entry.FocusSystemID.IsNone() || bSystemKnown;
			const bool bCivSeen = Entry.FocusCivID.IsNone()
				|| (CivIndex != INDEX_NONE ? View.ShownOrderOf(CivIndex) != INDEX_NONE : bSystemKnown);
			if (!bSystemSeen || !bCivSeen)
			{
				Out.bElsewhere = true;
			}
			if (bSystemKnown && Systems.IsValidIndex(SystemIndex))
			{
				FSystemNote& Note = Systems[SystemIndex];
				Note.bTouched = true;
				Note.LastTick = Entry.Tick;
				if (!Entry.FocusCivID.IsNone() && NamesOwner(Entry, Tables))
				{
					Note.LastCivID = Entry.FocusCivID;
					if (!Note.bFirstOwner)
					{
						Note.bFirstOwner = true;
						Note.FirstOwnerID = Entry.FocusCivID;
					}
				}
				if (IsClaimLine(Entry, Tables))
				{
					Note.bClaim = true;
					Note.ClaimTick = Entry.Tick;
				}
			}
			if (RoseTicks.IsValidIndex(CivIndex) && View.ShownOrderOf(CivIndex) != INDEX_NONE && !Entry.SourceRow.IsNone() && Tables.FindFeat(Entry.SourceRow))
			{
				RoseTicks[CivIndex] = Entry.Tick;
			}
		}
	};
	Read(State.Chronicle);
	Read(State.Log);

	TArray<FFound> Found;
	// The systems the viewer knows, in array order.
	for (int32 SystemIndex = 0; SystemIndex < Systems.Num(); ++SystemIndex)
	{
		const FSystemNote& Note = Systems[SystemIndex];
		if (!Note.bTouched)
		{
			continue;
		}
		const FStarSystemState& System = State.Systems[SystemIndex];
		FFound Item;
		Item.SubjectID = System.SystemID;
		Item.System = System.DisplayName;
		Item.Tick = Note.LastTick;
		if (CosmicHazards::IsDeadNode(System))
		{
			const bool bPeopleGone = !Note.LastCivID.IsNone() && Index.FindCiv(State, Note.LastCivID) == INDEX_NONE;
			Item.Kind = bPeopleGone ? ECosmicDigestItem::LostPeople : ECosmicDigestItem::Lost;
		}
		else if (Note.bClaim && InAnySphere(State, System.SystemID))
		{
			Item.Kind = ECosmicDigestItem::Annexed;
			Item.Tick = Note.ClaimTick;
		}
		else if (Note.bFirstOwner && Note.FirstOwnerID != System.OwnerCivID)
		{
			const int32 OwnerIndex = System.OwnerCivID.IsNone() ? INDEX_NONE : Index.FindCiv(State, System.OwnerCivID);
			const int32 FormerIndex = Index.FindCiv(State, Note.FirstOwnerID);
			// Told only between peoples the view shows (a gone former owner tells nothing of itself): a change to or from a
			// people it does not show is a change out of knowledge.
			const bool bFormerShown = FormerIndex == INDEX_NONE || View.ShownOrderOf(FormerIndex) != INDEX_NONE;
			if (State.Civs.IsValidIndex(OwnerIndex) && bFormerShown && CosmicMapLook::OwnerShownOf(State, View, SystemIndex) == ECosmicOwnerShown::Shown)
			{
				Item.Kind = ECosmicDigestItem::Owner;
				Item.CivID = State.Civs[OwnerIndex].CivID;
				Item.Civ = State.Civs[OwnerIndex].CivName;
			}
			else
			{
				Out.bElsewhere = true;
				continue;
			}
		}
		else
		{
			continue;
		}
		Item.Order = Found.Num();
		Found.Add(Item);
	}

	// The peoples, in array order: the rung, then the seat.
	TArray<FFound> Thrones;
	for (int32 CivIndex = 0; CivIndex < State.Civs.Num(); ++CivIndex)
	{
		const FCivilizationState& Civ = State.Civs[CivIndex];
		const bool bShown = View.ShownOrderOf(CivIndex) != INDEX_NONE;
		if (RoseTicks[CivIndex] > 0)
		{
			FFound Item;
			Item.Kind = ECosmicDigestItem::Rose;
			Item.SubjectID = Civ.CivID;
			Item.CivID = Civ.CivID;
			Item.Civ = Civ.CivName;
			Item.Tick = RoseTicks[CivIndex];
			Item.Order = Found.Num();
			Found.Add(Item);
		}
		const int64 SeatLeft = SeatChangedTickOf(State, Civ, AfterTick, ThroughTick, Rebellion);
		if (SeatLeft > 0)
		{
			if (!bShown)
			{
				Out.bElsewhere = true;
				continue;
			}
			FFound Item;
			Item.SubjectID = Civ.CivID;
			Item.CivID = Civ.CivID;
			Item.Civ = Civ.CivName;
			Item.Ruler = RulerNameOf(Civ, Tables);
			Item.Kind = Item.Ruler.IsEmpty() ? ECosmicDigestItem::ThroneUnnamed : ECosmicDigestItem::Throne;
			Item.Tick = SeatLeft;
			Thrones.Add(Item);
		}
	}
	// At most MaxThrones seats, the latest (equal ticks: the earlier people).
	Thrones.StableSort([](const FFound& A, const FFound& B) { return A.Tick > B.Tick; });
	for (int32 At = 0; At < Thrones.Num() && At < MaxThrones; ++At)
	{
		FFound Item = Thrones[At];
		Item.Order = Found.Num();
		Found.Add(Item);
	}

	// The cap: by rank, the later first within a rank; then the page's order - the tick, then the rank, then as found.
	if (Found.Num() > MaxItems)
	{
		Found.StableSort([](const FFound& A, const FFound& B)
		{
			return RankOf(A.Kind) != RankOf(B.Kind) ? RankOf(A.Kind) < RankOf(B.Kind) : A.Tick > B.Tick;
		});
		Found.SetNum(MaxItems);
		Out.bMore = true;
	}
	Found.StableSort([](const FFound& A, const FFound& B)
	{
		if (A.Tick != B.Tick)
		{
			return A.Tick < B.Tick;
		}
		return RankOf(A.Kind) != RankOf(B.Kind) ? RankOf(A.Kind) < RankOf(B.Kind) : A.Order < B.Order;
	});

	for (const FFound& Each : Found)
	{
		FCosmicDigestItem& Item = Out.Items.AddDefaulted_GetRef();
		Item.Kind = Each.Kind;
		Item.SubjectID = Each.SubjectID;
		Item.CivID = Each.CivID;
		Item.Tick = Each.Tick;
		Item.Variant = VariantOf(State.Seed, Each.SubjectID, Each.Tick);
		Item.Line = Render(Each, Item.Variant);
	}
	return Out;
}

TArray<FText> CosmicDigest::LinesOf(const FCosmicDigest& Digest)
{
	using CosmicUI::Label;

	TArray<FText> Lines;
	if (!Digest.bValid)
	{
		return Lines;
	}
	if (Digest.bNoLeap)
	{
		Lines.Add(Label(NoLeapKey));
		return Lines;
	}
	Lines.Add(FText::Format(Label(HeadKey), FText::AsNumber(Digest.ThroughTick - Digest.AfterTick)));
	if (Digest.Items.IsEmpty())
	{
		// No item: one line true on every path - it says the world went on, never that nothing happened (the digest sees no
		// change that left no line, and makes no item of many lines). The closing line would only say it twice.
		Lines.Add(Label(QuietKey));
		return Lines;
	}
	for (const FCosmicDigestItem& Item : Digest.Items)
	{
		Lines.Add(Item.Line);
	}
	if (Digest.bMore)
	{
		Lines.Add(Label(MoreKey));
	}
	if (Digest.bElsewhere)
	{
		Lines.Add(Label(ElsewhereKey));
	}
	return Lines;
}

TArray<FText> CosmicDigest::LinesOf(const UGalaxySimulationSubsystem& Sim, int32 ClosedLifeIndex, bool bSeeAll)
{
	return LinesOf(DigestOf(Sim, ClosedLifeIndex, bSeeAll));
}
