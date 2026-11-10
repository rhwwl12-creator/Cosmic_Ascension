import sys
p=sys.argv[1]
s=open(p,encoding='utf-8',newline='').read()
cut="// An MVP save that ended by the old rules opens as a run that goes on"
assert s.count(cut)==1
i=s.index(cut); head,t=s[:i],s[i:]
old_hdr="""// An MVP save that ended by the old rules opens as a run that goes on (B6a header section 2.10, decision 2-da; [B-6a3c]): a
// save made before the ending record holds none (the delta save never wrote an empty one - Sim.SaveEndingRecord), so (1) a
// run whose life reached the third rung (the old end) loads with no latch and no record, its next tick writes none, and it
// ends by the clock at 6,000 - the record and the latch in that tick; (2) a run saved past the clock (6,200) loads with no
// latch and an open disk mark and ends at its next tick, 6,201, with Ending.Clock of that tick (mutation: a latch or MarkOf
// that reads the clock or the rungs instead of the record latches right after the load); (3) a run saved in the silence (the
// interregnum with no leap left, B5) loads with no record and no latch and ends at its next tick with Ending.NoCandidate of
// that tick, met with no vessel, and latches it (mutation: the tick's write gate skipping a silence that stood when the tick
// began); (4) a run saved past its record (a record of 6,300 run on to 6,350 with the end switched off) latches the record on
// the load, and its CosmicEnd line says the record's tick, not the state's (mutation: the line prints State.Tick).
"""
new_hdr="""// An MVP save that ended by the old rules opens as a run that goes on (B6a header section 2.10, decision 2-da; [B-6a3c]): a
// save made before the ending record holds none (the delta save never wrote an empty one - Sim.SaveEndingRecord), so (1) a
// run whose life reached the third rung (the old end) loads with no latch and no record, its next tick writes none, and it
// ends by the game's clock (Clock) - the record and the latch in that tick; (2) a run saved past the clock (Clock + 200)
// loads with no latch and an open disk mark and ends at its next tick, Clock + 201, with Ending.Clock of that tick (mutation:
// a latch or MarkOf that reads the clock or the rungs instead of the record latches right after the load); (3) a run saved in
// the silence (the interregnum with no leap left, B5) loads with no record and no latch and ends at its next tick with
// Ending.NoCandidate of that tick, met with no vessel, and latches it (mutation: the tick's write gate skipping a silence that
// stood when the tick began); (4) a run saved past its record (a record at Clock + 300 run on to Clock + 350 with the end
// switched off) latches the record on the load, and its CosmicEnd line says the record's tick, not the state's (mutation: the
// line prints State.Tick); (5) a B6a save ended by the clock at 6,000 (record Ending.Clock at 6,000, saved at 6,050) opened
// under a clock of 7,000 latches its record and its disk copy is ended - the record decides, never the knob (mutation: the
// latch or MarkOf comparing the record's tick with the knob).
"""
R=[(old_hdr,new_hdr),
("""	const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
	TArray<FGuid> Written;""","""	const bool bWasBroadcasting = Sim->IsBroadcastEnabled();
	const FCosmicDataTables Original = Sim->GetTables();
	TArray<FGuid> Written;"""),
("""		Sim->SetBroadcastEnabled(bWasBroadcasting);
		Sim->NewGalaxy(42, 5, 3);""","""		Sim->SetBroadcastEnabled(bWasBroadcasting);
		Sim->SetTables(Original);
		Sim->NewGalaxy(42, 5, 3);"""),
('TEXT("It ends by the clock: Ending.Clock at 6,000")','TEXT("It ends by the clock: Ending.Clock at the clock\'s tick")'),
('TEXT("with that tick (6,201), not the clock\'s")','TEXT("with that tick (Clock + 201), not the clock\'s")'),
("	// The record's tick (6,300) is said by no other case","	// The record's tick (Clock + 300) is said by no other case"),
("""		TestTrue(TEXT("(its disk copy is ended)"), UCosmicSaveSubsystem::MarkOf(Loaded, Rules).bEnded);
	}
	return true;
}""","""		TestTrue(TEXT("(its disk copy is ended)"), UCosmicSaveSubsystem::MarkOf(Loaded, Rules).bEnded);
	}

	// ---- (5) A B6a save ended by the clock at 6,000 stays ended under a later clock (B6b header section 5.3 (f), A4) ----
	// The record decides, never the knob: the clock is put at 7,000 so the saved state (record Ending.Clock at 6,000, tick
	// 6,050) lies under it - a latch or MarkOf that compared the record with the knob would open it as a run that goes on
	// (mutations b6b10_latch_reads_the_knob, b6b10_markof_reads_the_knob). With the knob at 6,000 the record and the knob
	// agree and nothing would be caught, so the 7,000 holds whatever the game's clock is. Last case: Rules follows the tables.
	constexpr int64 OldClock = 6000;
	Sim->SetTables(CosmicTests::WithRules(Original, CosmicTests::EndingClockAt(Sim->GetRules(), 7000)));
	if (TestEqual(TEXT("(the clock is put at 7,000 for this save and load)"), Rules.ProvisionalEndTick, 7000)
		&& SaveAndLoad(OldClock + PastRecordBy, EOldSave::PastRecord))
	{
		const FGalaxyState& Loaded = Sim->GetState();
		TestTrue(TEXT("(the loaded state holds Ending.Clock at 6,000 and stands at 6,050, under the clock)"), Loaded.Ending.Kind == CosmicTags::EndingClock()
			&& Loaded.Ending.Tick == OldClock && Loaded.Tick == OldClock + PastRecordBy);
		TestEqual(TEXT("A save ended by the clock at 6,000 latches its record under a later clock"), Controller->GetRunEnd(), CosmicTags::EndingClock());
		TestTrue(TEXT("and its disk copy is ended"), UCosmicSaveSubsystem::MarkOf(Loaded, Rules).bEnded);
	}
	return true;
}""")]
for a,b in R:
    n=t.count(a)
    assert n==1,(n,a[:70])
    t=t.replace(a,b)
open(p,'w',encoding='utf-8',newline='').write(head+t)
print("ok")
