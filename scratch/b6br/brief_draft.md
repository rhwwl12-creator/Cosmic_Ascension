# [B-6br] implementation brief: 시간 문 G1의 고침 — a memo of the sorted row names and an `IsHazardAnswer` without a name array (bit-identical)

Everything below was checked read-only at HEAD `8b6c5a7` (`[B-6b8b]`, committed while this brief was being written). The G1 evidence was taken at `84f0d36`. `[B-6b8b]` changed neither `Data/CosmicDataTables.*` nor any caller of the functions this commit touches (`git diff --stat 84f0d36 8b6c5a7`: DT_Rules, one header comment, `Sim.FleetDelay`, patches, docs).

- **{HBR}** is the tree this commit starts on.
- **`<SP>`** is the session scratchpad `C:\Users\rhwwl\AppData\Local\Temp\claude\c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension\37de2919-0c44-45d8-94cd-4dcf0860263e\scratchpad`. **`<OUT>`** is `<SP>\b6br`.
- HEAD line numbers are hints only. Find every anchor by its content (§0.2).

**Evidence** (read in full for this brief):
- `<SP>\b6b\timing\judgement.md`: the verdict. The cause is code, class (b), avoidable, and goes to `[B-6br]`; there is no question for the developer.
- `read_map.md`, `runs.md` (56 runs), `prototype.diff` and `probe.diff` in the same folder.
- The lead's quiet-machine re-measure: `<SP>\b6b8\timing\*.log` and `run_timing.ps1`.

**Rules that bind this commit:**
- CLAUDE.md rules 2, 3, 6 and 7, and rule 10 (ASCII source, includes from the module root, unique header names).
- "Header first" for code that touches determinism.
- A commit compiles and passes the tests twice.
- `Tools/check_layers.py` rule 3b. `Data` is Core Logic (`check_layers.py:29`).
- H §8: the `[B-6br]` row ("리뷰의 고침이 크면 그 고침만"), and the rule that a fix which moves baselines is never folded into a mechanism commit.
- N §5 `[B-6b3b]` '일의 속도' (`:844`): one full runner per commit, and mutations limited to new ones plus the old ones of touched files and tests.
- H is `docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md`; N is its `_notes.md`.

## Scope and decisions

**In.**
- **A.** An exact memo inside every `FCosmicDataTables` accessor that sorts. There are eight: `GetCivRowNames`, `GetEventRowNames`, `GetTranscendentRowNames`, `GetHazardRowNames`, `GetFeatRowNames`, `GetEncounterRowNames`, `GetEdictRowNames` and `GetGalaxyRowNames` (`Data/CosmicDataTables.cpp:129-200`).
  - No other `FCosmicDataTables` function sorts, and no Sim, Events or Ascension code calls `GetRowNames()` itself.
  - The one other sort in the module is the system panel's own sort of `DT_Megaprojects` names (`UI/CosmicSystemPanelWidget.cpp:1053-1054`). It is presentation, outside the AI stage, and out of scope.
  - Each accessor keeps the key sequence it sorted from and the sorted array.
  - It answers from the memo only while the table's `RowMap` keys, in iteration order, equal the remembered keys element by element, with case and number compared.
  - It returns the same `TArray<FName>` by value, so no signature changes.
- **B.** `IsHazardAnswer` walks the row map without building a name array.
- **One small print-only aid** in `Tools/run_checks.ps1` (§2.6). This follows the judgement's 추가 이야기.

**Not in (decided here, with the numbers):**
- **C — skip the border walk in the treaty partner search: deferred.**
  - It is exact. `ViewOf` returns either the stored relation or the base target, and when both fall on the same side of the bar the answer does not depend on the border.
  - But it is not cheap to prove:
    - It changes the single window `CosmicDiplomacy::TreatyBlockOf` (rule 13). The command and the UI read its first reason.
    - The existing tests (`Tests/CosmicDiplomacyTests.cpp:521-538` at HEAD) kill two of its four plausible branch mistakes: "a mixed case passes" and "above judged by the stored view alone".
    - Nothing pins "bordering, stored ≥ bar > base → None" or "not bordering, stored < bar ≤ base → None". It needs two new assertions and four mutations.
    - 11 old patches touch `Sim/CosmicDiplomacy.cpp` and would have to be rerun.
  - What it would gain: the partner-search lap is 0.28 / 2.19 / 0.53 µs/tick (input-less 3,000 / input-less 20,000 / ruler 20,000; `runs.md`). The modelled gain is at most about 1.1 µs, unmeasured, and only on one line.
  - G1 does not need it: with A alone the worst line is 12.90 (prototype), 3.15 under 16.05.
  - Where it goes: `[B-6b12]`'s cleanup or B7 (추가 이야기).
- **D — build the crisis stage's edict hooks once per system: deferred.**
  - G1 measures the AI stage only (`Cycles[3]` = `TickCivAI`). D changes the crisis stage, which has no gate (H §5.3, N §2.3 `:694` freeze the AI stage only).
  - The cost D removes appears only once galaxy laws stand:
    - Crisis stage at 20,000 ticks: 29.17 (on) vs 14.64 (off) µs/tick on the probe build; 27.00 vs 12.09 with the memo.
    - `FindEdict` runs 919 times a tick (on) vs 1.5 (off).
    - At 3,000 ticks the crisis stage is 8.63 vs 7.76 (memo build).
  - The whole tick (memo build, input-less 20,000) averages 48.0 µs. At `MaxTicksPerFrame` 10 that is about 0.5 ms of a frame, and D would save about 0.07 ms of it.
  - D adds a declaration to `Sim/CosmicEdicts.h` on a hash path (the product order of the spawn shares).
  - Its two files carry 19 + 157 old patches (`Sim/CosmicEdicts.cpp`, `Sim/GalaxySimulationSubsystem.cpp`), which the rerun rule would pull in: hours of mutation runs for an invisible gain.
  - Where it goes: when a tick budget exists (B8) or `[B-6b12]`. Record the numbers (§7.4).

**Expected outcome.**
- All five runner lines and both hash runs stay bit-identical, by construction (§2.4).
- The serial AI stage drops from about 17.3-23.0 to about 6-12 µs/tick on a quiet machine:
  - prototype runner lines: 7.16 / 6.67 / 8.66 / 12.90 / 10.80;
  - minus B: about 0.9 / 0.9 / 1.1 / 1.4 / 1.4 (model: about 125 ns saved per call × 6.9-11.1 calls a tick).
- G1 closes with the threshold unchanged.
- No developer question.
- No saved field, no draw, no hash path, `SaveVersion` stays 1, and no data changes (`-Data` is not needed).

---

## 0. Start state (on {HBR}, before any agent runs)

1. **Commits and the machine.**
   - `git log --oneline -4` must show `[B-6b8b]` (`8b6c5a7`) on top of `[B-6b8]` (`84f0d36`). `[B-6b9]` does not open: STATUS `:32` says all four judgments passed.
   - If `[B-6b10]` or any later commit is already on top, stop and ask the lead. §4's lines and §6's "before" tree assume the clock is not in yet; STATUS `:32` orders the G1 cause before `[B-6b10]`.
   - `git diff --ignore-cr-at-eol --stat HEAD` must be empty. At HEAD the working tree differs only by line endings.
   - None of these processes may be running: UnrealEditor, UnrealEditor-Cmd, UnrealBuildTool or ShaderCompileWorker, or a python whose command line holds `bundle`, `run_mutation`, `run_checks` or `run_py`.
2. **Re-derive these anchors on {HBR}** (HEAD hints):
   - `Data/CosmicDataTables.h`: the accessor comments `:99-126`, `IsHazardAnswer` `:128-131`, the closing `};` `:132`.
   - `Data/CosmicDataTables.cpp`: the anonymous namespace `:9-32`, the eight bodies `:129-200`, `IsHazardAnswer` `:164-179`.
   - `Tools/run_checks.ps1`: `Compare-Balance` `:81-112`, the five balance steps `:114-133`.
   - `docs/21_DATA_MODEL_REFERENCE.md:1536` (`UPROPERTY(Transient) FCosmicDataTables Tables;`).
   - `docs/40_DEV_GUIDE.md`: `:54` (the runner line), `:237` (the `Data.Integrity` row).
   - `docs/45_ENGINE_NOTES.md`: `:26` (`FNameLexicalLess`), `:72` (`UDataTable::AddRow`).
   - N: `:694` (the threshold "AI 단계 µs/틱 — B6a 10.4~10.7의 +50%를 넘으면…"), `:757` (the sub-bullet "  - AI 단계 µs/틱(러너의 직렬 판과 묶음의 켬 − 끔"), and the end of file (the `[B-6b8b]` block).
   - H §11: the last bullet (`[B-6b8b]`), before `## 추가 이야기와 보완`.
   - `DECISIONS_LOG.md:43` (the `[B-6b8b]` line, the last B6b line before the B6a heading).
   - STATUS `:3` (갱신), `:26` (코드 사실), `:27` (기준값), `:32` (다음 작업 2).
3. **Callers and threads** (save the outputs under `<OUT>`).
   - `git grep -n "Get\(Civ\|Event\|Transcendent\|Hazard\|Feat\|Encounter\|Edict\|Galaxy\)RowNames()" -- Source` → `callers_hbr.txt`. At HEAD every caller takes the value: a range-for over the returned array or a copy into a local. `GalaxyLawRowsOf` mutates its copy with `RemoveAll` (`Sim/CosmicEdicts.cpp:217`). A value return keeps all of them.
   - `git grep -n "IsHazardAnswer(" -- Source` → `answer_callers_hbr.txt`. There are 10 call sites at HEAD.
   - `git grep -nE "ParallelFor|AsyncTask|Async\(|FRunnable|UE::Tasks|TGraphTask" -- Source` must give 0 hits, as at HEAD. **Stop on a hit that reaches the tables**: the memo is game-thread-only.
4. **Placeholders.**
   - `{BASE}`: the five lines of `git show {HBR}:Tools/baselines/seed42.json`, without the `wrote …` tail: `summary_line`, `ruler_summary_line`, `wanderer_summary_line`, `summary_line_20000`, `ruler_summary_line_20000`. At HEAD the hash pairs are:
     - input-less 3,000: 2557850423 / 2906170550;
     - ruler 3,000: 1189870756 / 3359656751;
     - wanderer 6,000: 1631315934 / 489794244;
     - input-less 20,000: 1957158585 / 3764704052;
     - ruler 20,000: 1375149537 / 510604211.
   - `{H_BYTES0}`: `wc -c` of H (148,811 at HEAD).
   - `{TESTS0}`: the automation test count (225 at HEAD; `git grep -c IMPLEMENT_SIMPLE_AUTOMATION_TEST`).
5. **The "before" runs on the start build** (ROOT, editor closed, before any edit).
   - First run `Build.bat Cosmic_AscensionEditor Win64 Development -Project="<ROOT>\Cosmic_Ascension.uproject" -WaitMutex`. `[B-6b8b]`'s last runner built this tree, so nothing should build; record the output.
   - Then run ten commandlet runs, each with an absolute `-Out=<ROOT>\Saved\Balance\b6br_before\<name>.csv` and `-AbsLog=<OUT>\before\<name>.log`:
     - the runner's five, with exactly its arguments (`run_checks.ps1:114-133`): `checks`, `checks_ruler`, `checks_wanderer`, `checks_20000`, `checks_ruler_20000`;
     - the same five with `-WithoutB6bData=1`, named `off_<name>`.
   - **Check:** the five on-lines equal `{BASE}` letter for letter.
   - Record the five off-lines as `{OFF}`. The expected values are the `[B-6b5]` lines (N `[B-6b5]` block):
     - 2816687688 / 3300228750;
     - 3561771757 / 2462331303;
     - 2665013209 / 2229239127;
     - 2359424635 / 726895141;
     - 1611658162 / 29194274.

     A difference from these is recorded, not a stop: this commit's check is before = after.
   - Keep the three files of every run (`<name>.csv`, `_rows.csv`, `_claims.csv`). They are §4's "before".
6. **The "before" tree for §6.** It can be prepared while the implementers edit. Never run it during a timed run or a mutation build.
   - Create it: `git -C <ROOT> worktree add --detach <OUT>\before_wt {HBR}`. The G1 measurement's SAND (`<SP>\b6b\timing_wt`) is gone: `git worktree list` at HEAD shows only ROOT and the two b7x trees.
   - Check that `<OUT>\before_wt\Content\CA\Data\DT_Feats.uasset` is a real asset: larger than 1 KB, first bytes `C1 83 2A 9E`.
     - The committed blobs are 130-byte LFS pointers. git-lfs 3.7.1 is installed and smudges them from the shared store.
     - If one is still a pointer, run `git -C <OUT>\before_wt lfs checkout`.
     - Read-map open point 6 explains why: a pointer would empty every table.
   - Make a short path, as the G1 measurement did with `b6bt`: `cmd /c mklink /J C:\Users\rhwwl\AppData\Local\Temp\b6br_b <OUT>\before_wt`.
   - Build it: `Build.bat Cosmic_AscensionEditor Win64 Development -Project="C:\Users\rhwwl\AppData\Local\Temp\b6br_b\Cosmic_Ascension.uproject" -WaitMutex -NoHotReloadFromIDE`. This is a first build of the module, about 10-25 minutes; UBT's mutex makes it wait for, or hold off, any ROOT build.
   - Sanity: one `checks` run there prints `{BASE}`'s first line.
7. **Patch apply state.** Run `python -I <SP>\b6b8b\apply_state.py <ROOT> <copy of {HBR}> <OUT>\apply_before.txt` (that tool's usage line). There are 1,078 patches at HEAD.
8. **Engine facts.** If the engine was updated after 2026-11-10, re-check §1.5 before writing code.

---

## 1. The header (header first)

### 1.1 What changes

`Data/CosmicDataTables.h` gains:
- one plain struct, `FCosmicSortedRowNames`, which is not reflected;
- eight `mutable` private members of `FCosmicDataTables`, which are not `UPROPERTY`.

No function is added, removed or re-signed. This is a header change, so build with the editor closed. The runner's `Build.bat` covers it.

### 1.2 Approval: header first

**Why the rule applies.**
- No saved field, enum, tick stage or draw point changes.
- But the sorted lists are the orders of draws (`CosmicDataTables.h:99-110`):
  - `GetHazardRowNames`: the shares of one spawn draw and the capital rule;
  - `GetEventRowNames`: the event slots;
  - `GetCivRowNames`: the archetype draw;
  - `GetTranscendentRowNames`: the roster draw.
- A memo whose answer must equal the uncached one exactly is therefore code that touches determinism, and CLAUDE.md asks for the header first.

**The steps.**
- §1.3 is the header text.
- The lead approves it **before the implementer writes the `.cpp`**, under the developer's standing approval for milestone B ("다 승인할테니 B작업을 끝까지 이어서 진행해줘", `DECISIONS_LOG.md` B1-B3 headings; the recommended option is the approved one).
- Record the line "헤더 먼저 — 지침서 §1.3의 머리 글을 리드가 {APPROVAL_DATE}에 승인(개발자의 상시 승인, 권장안)한 뒤 소스" in N §5 and the commit body.

### 1.3 The header text

**Insert before `USTRUCT()` (after the `CosmicLabels` namespace, `:14-17`).**

```cpp
// The row names of one table in lexical order, kept with the keys they were sorted from (B6b [B-6br] - the AI stage asked
// GetFeatRowNames about thirty times a tick, and every call gathered the names into a fresh array and sorted them again).
// FCosmicDataTables keeps one per sorting accessor. A memo answers only while its table's keys, in RowMap order, are the
// remembered ones element by element, case and number compared: the array it returns is then the one GetRowNames sorted
// with FNameLexicalLess returns. Any other table - a row added, removed or renamed in place, another table, a copy whose
// pointer was replaced - is sorted again. Not a UPROPERTY and not state: never saved, never hashed, read by no rule; it
// holds FNames only (no object reference for the collector). A copy of FCosmicDataTables carries its memos, which the
// check makes harmless. Game thread only, like every read of the tables.
struct FCosmicSortedRowNames
{
	TArray<FName> Keys;
	TArray<FName> Sorted;
};
```

**Replace the end of the struct.**

Before (HEAD `:128-132`):

```cpp
	// Whether some DT_Hazards row lists RowName in RulerResolutions. Called with DT_Megaprojects rows: such a project
	// answers a hazard and runs in the response slot (section 5.7). It is true for a listed DT_Edicts row too (B2), but no
	// caller passes one - an edict answers at the expiry (CosmicEdicts::HeldAnswerOf).
	bool IsHazardAnswer(FName RowName) const;
};
```

After (the comment above `IsHazardAnswer` is unchanged):

```cpp
	// Whether some DT_Hazards row lists RowName in RulerResolutions. Called with DT_Megaprojects rows: such a project
	// answers a hazard and runs in the response slot (section 5.7). It is true for a listed DT_Edicts row too (B2), but no
	// caller passes one - an edict answers at the expiry (CosmicEdicts::HeldAnswerOf).
	bool IsHazardAnswer(FName RowName) const;

private:
	// The memo of each sorting accessor above (FCosmicSortedRowNames - B6b [B-6br]). mutable: the accessors stay const
	// reads; every call checks its memo against its table (Data/CosmicDataTables.cpp, SortedRowNamesOf).
	mutable FCosmicSortedRowNames CivNamesMemo;
	mutable FCosmicSortedRowNames EventNamesMemo;
	mutable FCosmicSortedRowNames TranscendentNamesMemo;
	mutable FCosmicSortedRowNames HazardNamesMemo;
	mutable FCosmicSortedRowNames FeatNamesMemo;
	mutable FCosmicSortedRowNames EncounterNamesMemo;
	mutable FCosmicSortedRowNames EdictNamesMemo;
	mutable FCosmicSortedRowNames GalaxyNamesMemo;
};
```

**Checks done for this text at HEAD:**
- No code aggregate-initializes `FCosmicDataTables` (`git grep "FCosmicDataTables *{"` gives 0), so the `private:` section breaks nothing.
- No code takes its `sizeof`.
- The only by-value copies outside tests are `Loaded` in `Initialize` (`Sim/GalaxySimulationSubsystem.cpp:648`) and the commandlet's setup copies (`Tests/CosmicBalanceCommandlet.cpp:1391`, `:1598`). Nothing copies the struct per tick.
- `FCosmicSortedRowNames` and the eight member names are new in the module (`git grep` gives 0).

### 1.4 Why this form

- **Per instance, not the prototype's function-local `static`.**
  - The `static` memo is shared by every `FCosmicDataTables` in the process: the live tables, test copies, the commandlet's `WithoutB6bData` copy.
  - It is still exact, because of the key check, but it is hidden global state and not safe across threads.
  - The per-call work is the same. The prototype measured the form; the member only moves where the memo lives.
- **No `bValid`** (the prototype and the judgement had one). An empty memo matches only an empty table, and its empty `Sorted` is that table's answer.
- **A value return, not a `const TArray<FName>&`.**
  - (1) C++20 range-for does not extend the life of a temporary `FCosmicDataTables`, so `for (FName X : TablesByValue().GetFeatRowNames())` would dangle. No caller does this today, but nothing would stop one.
  - (2) A rebuild during a caller's iteration of the reference would invalidate it.
  - (3) The copy costs about 30 ns × about 30 calls, roughly 1 µs/tick, and that is already inside the prototype's numbers.
- **The key sequence as the check**, not a pointer or a counter.
  - Tests edit tables in place with `RemoveRow`/`AddRow`: `Tests/CosmicAscensionTests.cpp:720`, `CosmicThirdRungTests.cpp:734`, `CosmicDeathTests.cpp:807`, `CosmicDataTests.cpp:1734` and more. Pointer identity misses those edits.
  - UDataTable keeps no change counter. Its change delegate, `OnDataTableChanged()` (`DataTable.h:185-189`, present in every build), is fired by `RemoveRow`/`AddRow` through `DATATABLE_CHANGE_SCOPE` (`DataTable.cpp:84-87`). Using it would take one subscription per table and per copy, with handles whose lifetimes must outlive the copies.
  - The sequence is the sort's input itself and needs no bookkeeping. Checking it costs 1-20 FName compares and no allocation.
- **Not "a list handed down once per tick"** (read_map C1/C3/C7). That changes Sim/Ascension signatures; the judgement §4 rejected it.

### 1.5 Engine facts the proof uses (UE 5.8 source, verified 2026-11-10; paths under `Engine/Source/Runtime`)

1. **`UDataTable::GetRowNames()` returns `RowMap.GetKeys(Keys)`** (`Engine/Private/DataTable.cpp:1045-1050`).
   - `TMap::GetKeys` builds a temporary `TSet` and walks the pairs in iteration order, skipping keys already seen (`Core/Public/Containers/Map.h.inl:242-267`).
   - A map's keys are unique, so the result is the iteration sequence.
2. **`GetRowMap()` is virtual and returns `RowMap`** (`Engine/Classes/Engine/DataTable.h:107-108`). No engine class overrides it: a grep for `GetRowMap() const override` and `GetRowMap() override` in Runtime and Editor finds nothing.
3. **`FindRowInternal`** (`DataTable.cpp:1068-1101`):
   - a null `RowStruct` gives an Error line and null;
   - a wrong type gives null (a log line only when `bWarnIfRowMissing`, which `FindRowQuiet` passes as false);
   - `NAME_None` gives null;
   - otherwise it returns `GetRowMap().Find`.
4. **`FNameLexicalLess` is `A.Compare(B) < 0`** (`Core/Public/UObject/NameTypes.h:1691-1696`).
   - `FName::Compare`: the same comparison index gives the number difference; otherwise a case-insensitive alphabetical compare (`Core/Private/UObject/UnrealNames.cpp:3508-3552`).
   - So it is nonzero for any two distinct keys of a `TMap`, whose `==` compares the comparison index and the number: a strict total order on a key set.
5. **`FName::IsEqual(Other, ENameCase::CaseSensitive, true)` compares the display index and the number** (`NameTypes.h:1624-1628`; `UE_FNAME_OUTLINE_NUMBER` is 0 at `:39`; `ENameCase` is at `:205`).
   - Display indices keep case only where `WITH_CASE_PRESERVING_NAME` = `WITH_EDITORONLY_DATA` (`:32-33`). That holds in the editor binaries, which run the tests and the commandlet.
   - A cooked game's FName has no case to lose.
6. **`RemoveRow`/`AddRow` edit `RowMap` in place** (`DataTable.cpp:498-561`).
   - `TMap` is a sparse set by default (`UE_USE_COMPACT_SET_AS_DEFAULT` 0, `Core/Public/Containers/ContainerAllocationPolicies.h:1909-1912`).
   - The next add takes the most recently freed slot first (`Core/Public/Containers/SparseArray.h:117-141`).
   - So a remove followed by an add keeps the count and changes the sequence at that slot.
7. **Non-UPROPERTY `mutable` members in a USTRUCT** have engine precedent: `Engine/Classes/Animation/InputScaleBias.h:86`, `:112`. C4251 is off: `Core/Public/Windows/WindowsPlatformCompilerSetup.h:142`, and UBT passes `/wd4251` (`Programs/UnrealBuildTool/Platform/Windows/VCToolChain.cs:1395`).

---

## 2. The source

### 2.1 `Data/CosmicDataTables.cpp`: the helper

Add it to the anonymous namespace, after `FindRowQuiet` (`:27-31`). No new include: `UObject/NameTypes.h` (with `ENameCase` and `FNameLexicalLess`) is already included at `:7`, and `Engine/DataTable.h` comes through the header.

```cpp
	// B6b [B-6br] (the G1 time gate): whether Table's keys, in RowMap order, are Keys element by element - case and number
	// compared (FName::IsEqual with ENameCase::CaseSensitive), so that a row renamed in place to another case or number
	// is not taken for the old one. GetRowNames returns exactly this sequence (TMap::GetKeys walks the same pairs).
	bool SameKeySequence(const UDataTable& Table, const TArray<FName>& Keys)
	{
		const TMap<FName, uint8*>& RowMap = Table.GetRowMap();
		if (RowMap.Num() != Keys.Num())
		{
			return false;
		}
		int32 At = 0;
		for (const TPair<FName, uint8*>& Row : RowMap)
		{
			if (!Row.Key.IsEqual(Keys[At], ENameCase::CaseSensitive, true))
			{
				return false;
			}
			++At;
		}
		return true;
	}

	// The row names of Table in lexical order through Memo (FCosmicSortedRowNames). While the keys are the remembered ones,
	// GetRowNames would return the remembered array itself (the same FNames, display and number), and sorting the same
	// array with the same predicate gives the remembered order - the answer of the code before [B-6br]. Otherwise it is
	// sorted again exactly as before (GetRowNames, then Sort with FNameLexicalLess) and remembered. A null table answers
	// an empty array and leaves the memo alone.
	TArray<FName> SortedRowNamesOf(const UDataTable* Table, FCosmicSortedRowNames& Memo)
	{
		if (!Table)
		{
			return TArray<FName>();
		}
		if (!SameKeySequence(*Table, Memo.Keys))
		{
			Memo.Keys = Table->GetRowNames();
			Memo.Sorted = Memo.Keys;
			Memo.Sorted.Sort(FNameLexicalLess());
		}
		return Memo.Sorted;
	}
```

### 2.2 The eight bodies

Every body becomes one line. Example, before (HEAD `:150-155`):

```cpp
TArray<FName> FCosmicDataTables::GetFeatRowNames() const
{
	TArray<FName> Names = Feats ? Feats->GetRowNames() : TArray<FName>();
	Names.Sort(FNameLexicalLess());
	return Names;
}
```

After:

```cpp
TArray<FName> FCosmicDataTables::GetFeatRowNames() const
{
	return SortedRowNamesOf(Feats, FeatNamesMemo);
}
```

| Accessor (HEAD line) | Table | Memo |
|---|---|---|
| `GetEventRowNames` (`:129`) | `Events` | `EventNamesMemo` |
| `GetTranscendentRowNames` (`:136`) | `Transcendents` | `TranscendentNamesMemo` |
| `GetHazardRowNames` (`:143`) | `Hazards` | `HazardNamesMemo` |
| `GetFeatRowNames` (`:150`) | `Feats` | `FeatNamesMemo` |
| `GetEncounterRowNames` (`:157`) | `Encounters` | `EncounterNamesMemo` |
| `GetEdictRowNames` (`:181`) | `Edicts` | `EdictNamesMemo` |
| `GetGalaxyRowNames` (`:188`) | `Galaxies` | `GalaxyNamesMemo` |
| `GetCivRowNames` (`:195`) | `Civs` | `CivNamesMemo` |

`TObjectPtr<UDataTable>` converts to `const UDataTable*` exactly as it does in the existing `FindRowQuiet(Feats, …)` calls.

### 2.3 `IsHazardAnswer` without a name array

Before (HEAD `:164-179`):

```cpp
bool FCosmicDataTables::IsHazardAnswer(FName RowName) const
{
	if (RowName.IsNone() || !Hazards)
	{
		return false;
	}
	for (const FName HazardRow : Hazards->GetRowNames())
	{
		const FHazardDef* Row = FindHazard(HazardRow);
		if (Row && Row->RulerResolutions.Contains(RowName))
		{
			return true;
		}
	}
	return false;
}
```

After:

```cpp
bool FCosmicDataTables::IsHazardAnswer(FName RowName) const
{
	if (RowName.IsNone() || !Hazards)
	{
		return false;
	}
	// B6b [B-6br]: the keys straight from the row map, without the array GetRowNames builds on every call (a TSet and a
	// copy): the same keys in the same order through the same FindHazard, and whether some row lists the name does not
	// depend on the order anyway.
	for (const TPair<FName, uint8*>& Row : Hazards->GetRowMap())
	{
		const FHazardDef* Def = FindHazard(Row.Key);
		if (Def && Def->RulerResolutions.Contains(RowName))
		{
			return true;
		}
	}
	return false;
}
```

**Why this form and not the judgement's value walk.** The value walk checks the row struct once, reads the values directly and skips `NAME_None` keys.
- It would save about 40 ns more per call, about 0.4 µs/tick at 20,000.
- But it differs in one place: a table without a row struct. `FindRowInternal` writes an Error line per row there (fact 3), and the value walk would be silent.
- The key walk keeps every call and every log line, and its proof is one sentence.
- Estimated gain: about 200 → about 75 ns per call (6 × `FindHazard` at 10.8 ns), so −0.9 / −1.4 / −1.4 µs/tick at 6.87 / 11.05 / 11.00 calls a tick (`runs.md`).

### 2.4 Why every answer is the same (the proof the reviewers check)

- **P1 (memo hit).**
  - The check passed, so every current key equals the remembered one in display index and number. The FName values are identical; the comparison index follows from the display entry (fact 5).
  - `GetRowNames()` today would return exactly `Memo.Keys` (facts 1, 2).
  - `Memo.Sorted` is that same array sorted with the same predicate (`TArray::Sort` is deterministic on the same input), so it is the uncached code's array element for element.
  - Fact 4 adds that the lexical order of a key set is unique, so even a set check would do. The sequence check is stronger.
- **P2 (memo miss).** The code is the uncached code (`GetRowNames`, then `Sort(FNameLexicalLess())`), and its result is remembered.
- **P3 (null table).** It returns an empty array, as before.
- **P4 (callers).** Signatures and value returns are unchanged, so every caller's code and lifetime stays as it is (§0.3).
- **P5 (rules).**
  - The memo lives outside `FGalaxyState` (rule 2). It is derived from the tables alone, like `FCosmicStateIndex` from the state: not player or life state.
  - Draw counts and draw orders are unchanged (rule 3).
  - `ComputeStateHash` reads `FGalaxyState` only (rule 6).
  - `FCosmicDataTables` is `Transient` and never saved (rule 7).
- **P6 (`IsHazardAnswer`).** It visits the same keys in the same order (facts 1, 2), makes the same `FindHazard` calls with the same log lines (fact 3), and returns at the same first hit.
- **P7 (threads).** None (§0.3).
- **P8 (layers).** No `FMath` and no CRT math. The comments are ASCII, no include changes, and no header file is new.

### 2.5 What gets faster with no further edit

- `FindHazardByTag`, through `GetHazardRowNames`: the crisis stage and `Choices`.
- `GalaxyLawRowsOf`: the law step.
- The crisis stage's `GetHazardRowNames` each tick, and the event slots' `GetEventRowNames`.
- The UI: `CosmicNextRung`, the edict window.

The prototype's crisis stage at 20,000 ticks fell 29.17 → 27.00 µs/tick for this reason (`runs.md`).

### 2.6 Small print-only aid: `Tools/run_checks.ps1` `Compare-Balance`

Insert just before `$baselinePath = …` (`:101` at HEAD). This adds no comparison and no new failure: the whole addition sits inside `try`.

```powershell
    # The stage times beside the line, and the free memory (B6b [B-6br]): a loaded machine shows itself - in bundle B the
    # economy stage, which no B6b step touches, swelled x1.5-1.8 with the AI stage. Printed only; nothing is compared.
    try {
        $stages = Select-String -Path $log -Pattern 'CosmicBalance stages:.*crisis ([0-9.]+) us, AI ([0-9.]+) us.*economy ([0-9.]+) us' | Select-Object -Last 1
        if ($stages) {
            $freeGB = [math]::Round((Get-CimInstance Win32_OperatingSystem).FreePhysicalMemory / 1MB, 1)
            Write-Host ("   stages: AI {0} us, economy {1} us, crisis {2} us (free memory {3} GB)" -f $stages.Matches[0].Groups[2].Value, $stages.Matches[0].Groups[3].Value, $stages.Matches[0].Groups[1].Value, $freeGB)
        }
    } catch { }
```

Add the matching clause to `docs/40:54` (§7.2). No old patch touches `Tools/run_checks.ps1` (0 at HEAD).

---

## 3. Tests: a new file, `Source/Cosmic_Ascension/Tests/CosmicTableNamesTests.cpp`

**Why a new file.**
- 15 old patches touch `Tests/CosmicDataTests.cpp`. The narrow rerun rule ("rerun a patch that patches a file this commit edits") would pull them all in.
- A new file pulls in none.
- Both tests sit in the smoke filter (`Cosmic_Ascension.Data`).

The count goes from `{TESTS0}` (225) to 227.

### 3.1 File head and helpers

```cpp
// Cosmic Ascension - automation tests of the table reads B6b [B-6br] made cheaper without changing an answer (the time
// gate G1 of bundle B): the sorted row names of FCosmicDataTables, answered from a memo checked against the table's keys
// on every call, and IsHazardAnswer, which walks the row map without building an array of names.
// Design: docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race_notes.md section 5 [B-6br]; engine facts in
// docs/45_ENGINE_NOTES.md (UDataTable::GetRowNames, FNameLexicalLess, FName::IsEqual).

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Data/CosmicDataTables.h"
#include "Data/CosmicDefinitionRows.h"
#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "Sim/GalaxySimulationSubsystem.h"
#include "Tests/CosmicB6bRowNames.h"
#include "Tests/CosmicTestHelpers.h"
#include "UObject/NameTypes.h"

#include <initializer_list>

namespace CosmicTableNamesTests
{
	// The code before [B-6br]: the table's names gathered and sorted on every call.
	TArray<FName> Uncached(const UDataTable* Table)
	{
		TArray<FName> Names = Table ? Table->GetRowNames() : TArray<FName>();
		Names.Sort(FNameLexicalLess());
		return Names;
	}

	// Element by element as text, case and number included (TArray<FName>::operator== does not see case).
	bool Same(const TArray<FName>& Got, const TArray<FName>& Want)
	{
		if (Got.Num() != Want.Num())
		{
			return false;
		}
		for (int32 At = 0; At < Got.Num(); ++At)
		{
			if (!Got[At].ToString().Equals(Want[At].ToString(), ESearchCase::CaseSensitive))
			{
				return false;
			}
		}
		return true;
	}

	bool Same(const TArray<FName>& Got, std::initializer_list<const TCHAR*> Want)
	{
		if (Got.Num() != static_cast<int32>(Want.size()))
		{
			return false;
		}
		int32 At = 0;
		for (const TCHAR* Each : Want)
		{
			if (!Got[At++].ToString().Equals(Each, ESearchCase::CaseSensitive))
			{
				return false;
			}
		}
		return true;
	}

	// The walk IsHazardAnswer made before [B-6br].
	bool UncachedAnswer(const FCosmicDataTables& Tables, FName RowName)
	{
		if (RowName.IsNone() || !Tables.Hazards)
		{
			return false;
		}
		for (const FName HazardRow : Tables.Hazards->GetRowNames())
		{
			const FHazardDef* Row = Tables.FindHazard(HazardRow);
			if (Row && Row->RulerResolutions.Contains(RowName))
			{
				return true;
			}
		}
		return false;
	}
}
```

### 3.2 `Cosmic_Ascension.Data.SortedNamesFollowTable`

Use `IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicDataSortedNamesFollowTableTest, "Cosmic_Ascension.Data.SortedNamesFollowTable", CosmicTests::Flags)`. A label in parentheses marks a precondition. Every other assertion appears in §5's table with the mutation that breaks it. There is one assertion per case, without `&&` (docs/40 `:622`).

**(i) The game's tables, every accessor, twice.**
- Two instances:
  - `Fresh`: `FCosmicDataTables Fresh; Fresh.LoadFromFixedPaths();`;
  - `Live`: `Sim->GetTables()` after `Sim->NewGalaxy(42, CosmicTests::SliceSystems, CosmicTests::SliceCivs); Sim->RunHeadless(300);`. This is the instance the stages use, warmed by them, with whatever earlier tests left in it.
- Loop over a local table of the eight accessors: `{ TEXT("GetCivRowNames"), &FCosmicDataTables::GetCivRowNames, &FCosmicDataTables::Civs }, …`. The member types are `TArray<FName> (FCosmicDataTables::*)() const` and `TObjectPtr<UDataTable> FCosmicDataTables::*`.
- For each accessor:
  - `const UDataTable* Table = (Set->*A.Table).Get();`
  - `if (!TestNotNull("(the <set> tables hold the table of <accessor>)", Table)) continue;`
  - `Want = Uncached(Table)`;
  - **(i-1)** `"<accessor> on the <set> tables gives the uncached names"`: `Same((Set->*A.Get)(), Want)`;
  - **(i-2)** `"and so does its second call, from the memo (<accessor>, <set>)"`: the same assertion again.

**(ii) One table edited in place after the memo was made.** A stale memo would answer the old names.

```cpp
UDataTable* Feats = CosmicTests::NewTable(FAscensionFeatDef::StaticStruct());
Feats->AddRow(FName(TEXT("feat_b")), FAscensionFeatDef());
Feats->AddRow(FName(TEXT("feat_a")), FAscensionFeatDef());
FCosmicDataTables Board;
Board.Feats = Feats;
```

| # | Then | Assertion (label) | Expected |
|---|---|---|---|
| ii-1 | — | "Rows added b, a are named a, b" | `{feat_a, feat_b}` |
| ii-2 | (call again) | "and again from the memo" | `{feat_a, feat_b}` |
| ii-3 | `AddRow(feat_c)` | "A row added after the first call is named" | `{feat_a, feat_b, feat_c}` |
| ii-4 | `RemoveRow(feat_a)`, `AddRow(feat_d)` | "A row taken out and another put in (the same count) is followed" | `{feat_b, feat_c, feat_d}` |
| ii-5 | `Board.Feats = nullptr` | "No table: no names, not the memo's" | empty |
| ii-6 | `Board.Feats = NewTable(FAscensionFeatDef)` (empty) | "An empty table: no names" | empty |

ii-5 must come **right after** a non-empty memo; ii-6 comes after it.

**(ii′) The case and the number of a row's name.** Use a second table and instance. Add `feat_c`, `feat_b`, `feat_d` in that order and remove nothing before the renames.
- The renamed row is then always the last one added. Its place does not move: a sparse set reuses its slot, and a compact set would append it again.
- So only the name differs from the memo's key.

| # | Then | Assertion | Expected (compared as text, case-sensitive) |
|---|---|---|---|
| ii′-1 | — | "Rows added c, b, d are named b, c, d" | `{feat_b, feat_c, feat_d}` |
| ii′-2 | `RemoveRow(feat_d)`, `AddRow(FName(TEXT("Feat_D")))` | "A row renamed in another case is named in its new case" | `{feat_b, feat_c, Feat_D}` |
| ii′-3 | `RemoveRow(Feat_D)`, `AddRow(FName(TEXT("feat_n_1")))` | "A row with a number is named with it" | `{feat_b, feat_c, feat_n_1}` |
| ii′-4 | `RemoveRow(feat_n_1)`, `AddRow(FName(TEXT("feat_n_2")))` | "A row renumbered is named with its new number" | `{feat_b, feat_c, feat_n_2}` |

Notes on the names:
- `FName(TEXT("feat_n_1"))` and `FName(TEXT("feat_n_2"))` share the display entry `feat_n` and differ only in the number. `ToString()` gives them back as written.
- The case pair relies on the editor binary keeping case (fact 5). The tests run there, as `Data.Integrity`'s "arch_Other" case already does (`Tests/CosmicDataTests.cpp:1765`).

**(iii) A copy whose table was replaced.** This is the pattern of `CosmicTests::WithoutB6bData` and the commandlet's `-WithoutB6bData=1`.

```cpp
const TArray<FName> GameFeats = Fresh.GetFeatRowNames();   // warm
if (TestTrue(TEXT("(the game has feat rows)"), GameFeats.Num() > 1))
{
	FCosmicDataTables Copy = Fresh;                       // carries Fresh's memo
	Copy.Feats = CosmicTests::CopyWithout<FAscensionFeatDef>(Fresh.Feats, { GameFeats[0] });
	TestTrue(TEXT("A copy whose feat table lost a row gives that table's names"), Same(Copy.GetFeatRowNames(), Uncached(Copy.Feats)));
}
```

### 3.3 `Cosmic_Ascension.Data.HazardAnswerAnyRow`

No sim is needed.

- **A board.**
  - `Hazards = CosmicTests::NewTable(FHazardDef::StaticStruct())` with three rows, in this order:
    - `crisis_test_a`: no answers;
    - `crisis_test_b`: `RulerResolutions = { project_test_one }`;
    - `crisis_test_c`: `RulerResolutions = { project_test_two }`.
  - `FCosmicDataTables Board; Board.Hazards = Hazards;`
  - **(b-1)** "A project only the last row lists answers a hazard": `IsHazardAnswer(project_test_two)`. The last row comes last in both the table's order and the lexical one.
  - **(b-2)** "A project no row lists answers none": `!IsHazardAnswer(project_test_none)`.
- **The game's rows.**
  - `FCosmicDataTables Game; Game.LoadFromFixedPaths();`
  - Precondition `TestTrue("(the game's ark answers the extinction, its last hazard row)", UncachedAnswer(Game, CosmicB6bRows::ProjectArk))`. `DT_Hazards.json` holds `crisis_species_extinction` last, listing `project_ark`; the first row is `crisis_ai_awakening`, listing `edict_machine_rights`.
  - **(b-3)** For every row name of `Game.Megaprojects` and `Game.Edicts`: `TestEqual("IsHazardAnswer(<row>) is the answer of the walk before [B-6br]", Game.IsHazardAnswer(Row), UncachedAnswer(Game, Row))`. These are the two kinds of names the callers pass (`Sim/CosmicEdicts.cpp:131` passes edict rows).

---

## 4. Proof that nothing moved (breakable checks; any failure is a stop — §9 R1)

- **(a) The runner.** Run `run_checks.ps1 -Tier full` once (no `-Data`, no `-UpdateBaseline`).
  - All five balance steps pass. The runner throws if a line differs from `{BASE}`.
  - Both `hash run: … vs commandlet` steps pass.
  - `git diff --quiet -- Tools/baselines/seed42.json` holds.
- **(b) Tests twice.** The runner's two passes: 227 + 227 Success, 0 failures.
- **(c) CSVs.** Run `python -I -X utf8 <OUT>\csv_same.py --before <ROOT>\Saved\Balance\b6br_before --after <ROOT>\Saved\Balance --runs checks,checks_ruler,checks_wanderer,checks_20000,checks_ruler_20000`.
  - Every file of every run (`<name>.csv`, `_rows.csv`, `_claims.csv`) must be equal cell for cell.
  - The only exceptions are the main CSV's `TickMicros`, `SaveBytes` and `SaveRawBytes` columns.
  - Any other column that differs is a stop, never a new exclusion.
- **(d) The off runs on the final build.**
  - The five `-WithoutB6bData=1` runs into `Saved\Balance\b6br_after_off\` print `{OFF}`.
  - Their CSVs equal §0.5's (the same script, `--runs off_checks,…`).
  - These runs rebuild memos from copied tables: `WithoutB6bData` replaces six tables with copies, and the copies carry the live memos.
- **(e) Error lines.** Every log of (a) and (d) has 0 `CosmicBalance end check:`, 0 `CosmicBalance race check:` and 0 `LogCosmic: Error`.
- **(f) Free extra evidence.** All of §6's timing runs, on both sides, print `{BASE}` or `{OFF}`.

**`<OUT>\csv_same.py`.**
- It is Python stdlib only, `-I`, and writes nothing.
- Base it on `<SP>\b6b10\csv_cmp.py`: take its reader, `DROPPED` and `SUFFIXES`, and drop its clock-cut logic. Compare every row of every file.
- Output: one PASS/FAIL line per file pair, and a last line `csv_same: PASS|FAIL`.
- Exit codes: 0 when every pair passes, 1 on any fail, and 2 when a file is missing.
- **Self-test first** (`<OUT>\csv_same_selftest.py`, on copies of §0.5's files in a temporary folder):
  - identical files → PASS;
  - only `TickMicros` changed → PASS;
  - one `Systems` cell changed → FAIL;
  - one `_claims` row removed → FAIL;
  - one `_rows` file missing → exit 2.

---

## 5. Mutations

New patches go under `Tools/mutations/B6b/`, with one README row each, after the last `b6b8b_` row (`README.md:1021` at HEAD). The filter test of each is named in the table. A patch gets its README row only after it has been seen to fail. All are C++ patches: none touches `Tools/data`, so README `:5` does not change.

| Patch | Edit (`Data/CosmicDataTables.cpp`) | Breaks (`Cosmic_Ascension.Data.…`: assertions) |
|---|---|---|
| `b6br_memo_unsorted` | `SortedRowNamesOf`: drop `Memo.Sorted.Sort(FNameLexicalLess());` | `SortedNamesFollowTable`: (i-1)/(i-2) for the tables `Tools/data` does not hold in lexical order (DT_Feats, DT_Edicts, DT_Events, DT_Encounters, DT_Civs, DT_Galaxies; DT_Hazards and DT_Transcendents are), ii-1, ii-2, ii′-1 |
| `b6br_memo_never_refreshes` | check only while the memo is empty: `if (!SameKeySequence(*Table, Memo.Keys))` → `if (Memo.Keys.IsEmpty() && !SameKeySequence(*Table, Memo.Keys))` | `SortedNamesFollowTable`: ii-3, ii-4, ii-6, ii′-2, ii′-3, ii′-4, (iii) |
| `b6br_memo_count_only` | `SameKeySequence` compares the counts only (the loop removed) | `SortedNamesFollowTable`: ii-4, ii′-2, ii′-3, ii′-4 |
| `b6br_memo_case_blind` | `ENameCase::CaseSensitive` → `ENameCase::IgnoreCase` | `SortedNamesFollowTable`: ii′-2 |
| `b6br_memo_number_blind` | `IsEqual(Keys[At], ENameCase::CaseSensitive, true)` → `…, false)` | `SortedNamesFollowTable`: ii′-4 |
| `b6br_memo_null_reads_memo` | `if (!Table) { return TArray<FName>(); }` → `if (!Table) { return Memo.Sorted; }` | `SortedNamesFollowTable`: ii-5 |
| `b6br_memo_wrong_table` | `GetEncounterRowNames` sorts `Events`: `SortedRowNamesOf(Encounters, EncounterNamesMemo)` → `SortedRowNamesOf(Events, EncounterNamesMemo)` | `SortedNamesFollowTable`: (i-1)/(i-2) for `GetEncounterRowNames` on both sets |
| `b6br_answer_first_row_only` | `IsHazardAnswer` reads only the first row: the loop body becomes `const FHazardDef* Def = FindHazard(Row.Key); return Def && Def->RulerResolutions.Contains(RowName);` | `HazardAnswerAnyRow`: b-1, and b-3 for `project_ark`, `project_relief`, `project_stabilize` and `project_reform` |
| `b6br_answer_miss_true` | `IsHazardAnswer`'s last `return false;` → `return true;` | `HazardAnswerAnyRow`: b-2, and b-3 for every row no hazard lists |

Every non-precondition assertion of §3 is broken by at least one patch. For each run, the log must name the expected labels. If a patch breaks its test but not the named assertion, rewrite the patch; it must not "pass" on another assertion.

**The old patches to rerun: none.** Re-derive this on {HBR} mechanically:
- `git grep -l "b/Source/Cosmic_Ascension/Data/CosmicDataTables" -- Tools/mutations` gives 0 at HEAD.
- `git grep -l "b/Tools/run_checks.ps1" -- Tools/mutations` gives 0.
- The new test file has no old patch.
- This commit edits no existing test.

**Apply state.** Save `<OUT>\apply_after.txt` (§0.7 tool). It must equal `apply_before.txt`, apart from the nine new patches that apply. No old patch's context is in the edited files.

**Running them.**
- Serially, with no other process running: `powershell -File <SP>\run_mutation.ps1 -Patches b6br_memo_unsorted,… -Filter Cosmic_Ascension.Data.SortedNamesFollowTable`, and the two `answer_` patches with `-Filter Cosmic_Ascension.Data.HazardAnswerAnyRow`.
- The script applies, builds the editor target, runs, prints the failing lines, and reverts.
- Expect about 9 × 3-4 minutes.
- Afterwards, `git status` must show no patch hunks. The final runner's build then makes clean binaries.

---

## 6. The time gate re-measure (G1)

**Purpose.** Answer G1 with the build this commit makes. G1 is H §5.3 and N §2.3 `:694`: "AI 단계 µs/틱 — B6a 10.4~10.7의 +50%를 넘으면 `[B-6b9]` 전에 까닭을 찾는다"; the threshold is 16.05.
- The before and after builds run in one session, alternating, on a quiet machine.
- Bundle B's 23.93-31.90 were inflated by load: free memory was 2.4 GB, and the economy stage swelled ×1.51-1.83 (judgement §2).

**When.** After the final runner (§9 step 9), on its binaries. The only work allowed during the session is this harness: no build, mutation, bundle or editor.

**Harness.** `<OUT>\run_timing_ab.ps1`, derived from `<SP>\b6b8\timing\run_timing.ps1`.
- **Sides:**
  - `before` = `C:\Users\rhwwl\AppData\Local\Temp\b6br_b\Cosmic_Ascension.uproject` (§0.6);
  - `after` = `<ROOT>\Cosmic_Ascension.uproject`.
- **Runs, seed 42, with the runner's exact arguments:**
  - the gate's five: `checks` 3,000, `checks_ruler` 3,000, `checks_wanderer` 6,000, `checks_20000`, `checks_ruler_20000`;
  - two for the record only: `off_checks` 3,000 and `off_checks_20000`, with `-WithoutB6bData=1`.
- **Three rounds.** In round r, run i goes before then after when (r + i) is even, after then before otherwise. Each run writes:
  - its CSV to `<side root>\Saved\Balance\b6br_timing\<side>_<name>_r<r>.csv`;
  - its log to `<OUT>\timing\<side>_<name>_r<r>.log`.
- **Quiet check before every process** (poll every 30 s for up to 30 minutes; otherwise abort and tell the lead):
  - no UnrealEditor, UnrealEditor-Cmd, UnrealBuildTool or ShaderCompileWorker other than the harness's own child;
  - no python with `bundle`, `run_mutation`, `run_checks` or `run_py` in its command line;
  - free physical memory ≥ 8 GB.
- **One TSV row per run:** side, name, round, the summary line, AI, economy and crisis µs/tick (from `CosmicBalance stages:`), the tick mean (from the CSV's `TickMicros`), free GB at the start, wall seconds.
- About 42 processes, about 15 minutes.

**Summary.** `<OUT>\timing_summary.py` (`-I`, stdlib) writes `<OUT>\timing\summary.md` and exits 0 only when every check below holds.
- **Lines:** every run prints its expected line (`{BASE}`; `{OFF}` for the off runs) on both sides.
- **Load control:** for each pair, the economy stage after/before lies within 0.8-1.25; the economy code is the same on both sides. A pair outside that range is rerun once. If it is still outside, the round is void.
- **The gate:** for each of the five gate runs, the **median of the three after-runs < 16.05 µs/tick**.
  - Record the max as well.
  - A single after-run ≥ 16.05 with a passing median is recorded with its economy ratio. It is not a fail.
- **Record only:**
  - before medians, after medians, the after/before ratio;
  - the crisis stage and the tick mean on both sides;
  - the free memory range.
  - The before medians are expected near the lead's quiet values 18.00 / 17.27 / 20.06 / 22.96 / 19.65. If they are more than ±20% off, note it.

**Outcomes.**
- **Expected:** after ≈ 6-12, before ≈ 17-23. G1 closes and §7 records the numbers.
- **A line ≥ 16.05 on a valid session:** see §9 R3.

---

## 7. Docs and records

Re-derive every anchor on {HBR}. The texts below are Korean doc prose: the ASCII rule binds source only. They hold no verdict word and no number of people (rule 11). They are all record or technical text, so no theology question arises.

### 7.1 `docs/21_DATA_MODEL_REFERENCE.md:1536`

docs/21 does not list `FCosmicDataTables`' functions; it was checked at HEAD, and `:1388` only names `GetTables`. The one place that copies the struct is the sim's member, and the memo is a member of that struct.

Before:

> `UPROPERTY(Transient) FCosmicDataTables Tables; // 정의 테이블. UDataTable 참조를 쥐므로 UPROPERTY`

After:

> `UPROPERTY(Transient) FCosmicDataTables Tables; // 정의 테이블. UDataTable 참조를 쥐므로 UPROPERTY. 정렬하는 접근자 여덟(Get…RowNames)은 저마다 기억(FCosmicSortedRowNames — 정렬할 때의 키 차례와 정렬한 이름, UPROPERTY가 아닌 mutable 멤버, B6b [B-6br])으로 답한다: 표의 키 차례가 기억과 원소마다(대소문자·번호까지) 같을 때만 쓰고 아니면 전처럼 모아 정렬하므로 답은 같은 배열이다. 저장·해시 밖이고 규칙이 읽지 않는다 — 테이블에서만 나오므로 멤버라도 플레이어·생애 종속 상태가 아니다(불변 규칙 2)`

These stay true and are left as they are:
- `docs/21:1487` (`GalaxyLawRowsOf`: "표 차례(사전순 …)");
- `docs/20:249` ("순서가 중요한 곳은 `TArray`를 정렬해서 쓴다 … 키를 `FNameLexicalLess`로 정렬해 쓰거나");
- `docs/40:290` and `:424` (`Events.RowOrderInert`, `Sim.HazardRowOrderInert` — they now also rebuild the memo from a reversed table);
- the accessor comments in `CosmicDataTables.h`.

### 7.2 `docs/40_DEV_GUIDE.md`

**Two rows after the `Cosmic_Ascension.Data.Integrity` row (`:237`):**

> `| \`Cosmic_Ascension.Data.SortedNamesFollowTable\` | (B6b \`[B-6br]\`, \`Tests/CosmicTableNamesTests.cpp\` — 시간 문 G1, 노트 §5 \`[B-6br]\`) \`FCosmicDataTables\`의 정렬하는 접근자 여덟이 기억으로 답해도 정렬을 새로 한 답(\`GetRowNames()\` 뒤 \`Sort(FNameLexicalLess())\`)과 대소문자·번호까지 같다: 게임의 표(새로 읽은 것과 300틱 돈 시뮬의 것)에서 첫 부름과 둘째 부름; 첫 부름 뒤 제자리에서 고친 표 — 행을 더함, 같은 수로 빼고 넣음, 표 없음, 빈 표, 대소문자만 바꾼 이름, 번호만 바꾼 이름(바꾸는 행은 마지막에 더한 행 — 맵의 자리가 움직이지 않는다); 표를 바꾼 사본(\`WithoutB6bData\`의 꼴). 변이 \`b6br_memo_\` 일곱 |`

> `| \`Cosmic_Ascension.Data.HazardAnswerAnyRow\` | (B6b \`[B-6br]\`, 같은 파일) 이름 배열 없이 행 지도를 걷는 \`IsHazardAnswer\`: 마지막 행만 지닌 답은 답이고 아무 행도 지니지 않은 이름은 아니다; 게임의 \`DT_Megaprojects\`·\`DT_Edicts\` 행 모두가 옛 걸음(\`GetRowNames()\`와 \`FindHazard\`)과 같은 답(방주는 마지막 위기 행의 답). 변이 \`b6br_answer_\` 둘 |`

**`:54` (the runner line).** Append before the last sentence ("`Tools/run_checks.sh`…"):

> "균형 줄마다 그 판의 단계 시간(AI·경제·위기 µs/틱)과 여유 메모리를 곁에 찍는다 — 비교하지 않는다(부하가 걸린 판은 B6b가 손대지 않는 경제 단계가 함께 부풀어 드러난다 — B6b `[B-6br]`)."

### 7.3 `docs/45_ENGINE_NOTES.md`: the verified facts of §1.5

**`:26` (`FNameLexicalLess`).** Append to the last cell:

> "; `FName::Compare`(= `FNameLexicalLess`)는 비교 인덱스가 같으면 번호의 차, 다르면 대소문자를 가리지 않는 사전 순이라(`Core/Private/UObject/UnrealNames.cpp:3508-3552`) `TMap`의 서로 다른 키 사이에서 0이 되지 않는다 — 정렬은 키 집합만의 함수다. `FName::IsEqual(Other, ENameCase::CaseSensitive, true)`는 표시 인덱스와 번호를 본다(`NameTypes.h:1624` — 대소문자는 `WITH_CASE_PRESERVING_NAME` = `WITH_EDITORONLY_DATA`인 에디터 바이너리만 지닌다, `:32-33`)"

**`:72` (`UDataTable::AddRow`).** Replace "`GetRowNames()`는 넣은 순서다" with:

> "`GetRowNames()`는 `RowMap`의 차례다(`RowMap.GetKeys` — 부를 때마다 임시 `TSet`과 배열을 짓는다, `Engine/Private/DataTable.cpp:1045`, `Core/Public/Containers/Map.h.inl:242-267`; `GetRowMap()`은 가상이고 `RowMap`을 준다 — 엔진에 재정의 없음, `DataTable.h:107-108`): 넣은 순서이되 지운 자리는 다음에 더한 행이 먼저 쓴다(기본은 성긴 집합 — `ContainerAllocationPolicies.h:1912`, `SparseArray.h:117-141`) — 개수가 같아도 차례가 바뀐다"

Then append at the end of the cell:

> "; `FCosmicDataTables`의 정렬하는 접근자는 이 키 차례로 기억을 맞춰 본다(B6b `[B-6br]`, 테스트 `Data.SortedNamesFollowTable`)"

### 7.4 N (the notes)

**§3 '묶음 B': a new sub-bullet right after the time line (`:757`, "  - AI 단계 µs/틱(러너의 직렬 판과 묶음의 켬 − 끔…"), with the same indent:**

> `  - 까닭(2026-11-10 판정, \`[B-6br]\`({DATE})이 고쳤다): 직렬 러너의 23.93~31.90은 기계 부하가 ×1.33~1.41 부풀린 값이다(B6b가 손대지 않는 경제 단계는 ×1.51~1.83). 한가한 기계(여유 11 GB)에서는 17.27~22.96이고 그래도 16.05를 넘었다. 넘는 까닭은 코드였다 — \`FCosmicDataTables::GetFeatRowNames\`가 부를 때마다 엔진의 \`GetRowNames\`(임시 \`TSet\`과 배열)로 이름을 모으고 대소문자를 가리지 않는 비교로 다시 정렬했는데, AI 단계가 틱마다 약 30번, 칸과 상관없이 불렀고 B6b의 업적 행 둘이 한 번의 값을 176 → 350 ns로 올려 9.3~10.6 µs/틱(단계의 41~55%)이 되었다. 설명되지 않은 몫은 없다(같은 마지막 상태의 벤치 × 부른 수가 켬 − 끔의 82~104%). 다시 잰 값은 §5 \`[B-6br]\`.`

**§5: a new block at the end of the file, after the `[B-6b8b]` block.** It follows the form of the `[B-6b8b]` block. Fill the `{…}` from the run.

> **`[B-6br]`**({DATE} — 시간 문 G1의 고침: 묶음 B의 직렬 AI 단계가 B6a 10.7의 1.5배 16.05를 넘은 까닭 — 위 §2.3 `:694`의 문턱, 헤더 §8의 `[B-6br]` 행; 까닭은 위 §3 '묶음 B'의 시간 줄 뒤):
> - **나눔**: 없음(기억과 위기의 답 한 커밋). 설계 워크플로가 쓴 지침서(저장소 밖)를 구현 일꾼 둘이 나눠 맡았다 — 머리·소스·러너의 한 줄, 테스트·변이 파일·하네스. 이어 빌드 판(러너 `-Tier smoke`) → 기계(끈 판·CSV 대조)와 관점 셋의 리뷰 → 반박 검증 → 고침 → 변이 → 마지막 러너 `-Tier full` → 시간 다시 잼 → 기록.
> - **헤더 먼저**: 지침서의 머리 글 — `Data/CosmicDataTables.h`의 구조체 `FCosmicSortedRowNames`와 `FCosmicDataTables`의 `mutable` 비공개 멤버 여덟(UPROPERTY 아님) — 을 리드가 {APPROVAL_DATE}에 소스보다 먼저 승인했다(개발자의 상시 승인 — 권장안). 저장 필드·열거형·틱 단계·추첨 지점은 그대로이고, 정렬한 이름이 추첨의 차례(위기의 몫·이벤트 슬롯·아키타입·초월자 명부)라 결정론에 닿는 코드로 보았다.
> - **코드**: 정렬하는 접근자 여덟(`GetCivRowNames`·`GetEventRowNames`·`GetTranscendentRowNames`·`GetHazardRowNames`·`GetFeatRowNames`·`GetEncounterRowNames`·`GetEdictRowNames`·`GetGalaxyRowNames`)이 저마다 기억(정렬할 때의 키 차례와 정렬한 이름)을 쓴다 — 표의 `RowMap` 키가 기억의 키와 원소마다(대소문자·번호까지 — `FName::IsEqual(…, ENameCase::CaseSensitive, true)`) 같을 때만 기억의 사본을 주고, 아니면 전과 똑같이 `GetRowNames()` 뒤 `Sort(FNameLexicalLess())`로 다시 지어 기억한다. 값 반환 그대로(부르는 곳 그대로). `IsHazardAnswer`는 이름 배열 없이 `GetRowMap()`의 키를 걷는다 — 같은 키·같은 차례·같은 `FindHazard`(같은 로그 줄). 러너는 균형 줄 곁에 단계 시간과 여유 메모리를 찍는다(비교하지 않는다). 새 추첨·저장 필드·해시 길 없음, `SaveVersion` 1 그대로.
> - **같은 까닭**: 키 차례가 같으면 지금의 `GetRowNames()`가 기억의 키와 FName 값까지 같은 배열이고(표시 인덱스와 번호), 같은 배열을 같은 비교로 정렬한 것이 기억의 이름이다; 다르면 옛 코드 그대로. 키 집합이 같으면 차례와 상관없이 정렬은 하나다(`FNameLexicalLess`는 맵의 서로 다른 키 사이에서 엄격한 전순서) — 차례 검사는 그보다 강하다. 엔진의 사실은 `docs/45`.
> - **테스트**(225 → 227; 마지막 러너의 두 판 {SUCCESS} Success·실패 0): 새 파일 `Tests/CosmicTableNamesTests.cpp` — `Data.SortedNamesFollowTable`(게임의 표 — 새로 읽은 것과 300틱 돈 시뮬의 것 — 에서 접근자 여덟의 첫 부름과 기억의 부름이 정렬을 새로 한 답과 대소문자·번호까지 같다; 첫 부름 뒤 제자리에서 고친 표 — 행 더함, 같은 수로 빼고 넣음, 표 없음, 빈 표, 대소문자만 바꾼 이름, 번호만 바꾼 이름; 표를 바꾼 사본), `Data.HazardAnswerAnyRow`(마지막 행만 지닌 답, 아무 행도 지니지 않은 이름, 게임의 거대 프로젝트·칙령 행 모두가 옛 걸음과 같은 답). 새 파일인 까닭: `Tests/CosmicDataTests.cpp`를 깁는 옛 패치 열다섯을 다시 돌림에 끌어오지 않는다.
> - **변이**(모두 깨졌다): 새 `b6br_` 아홉 — `memo_unsorted`·`memo_never_refreshes`·`memo_count_only`·`memo_case_blind`·`memo_number_blind`·`memo_null_reads_memo`·`memo_wrong_table`·`answer_first_row_only`·`answer_miss_true`(README 행마다 하나). 다시 돌린 옛 변이 0 — 이 커밋이 고친 파일(`Data/CosmicDataTables.*`·`Tools/run_checks.ps1`)을 깁는 옛 패치가 없고 고친 테스트가 없다; `git apply --check` 앞뒤 같음(새 아홉만 더해짐).
> - **리뷰**: {REVIEW — 발견 n, 들인 것 m, 반박 k; 고친 것}.
> - **기준값**: **비트 그대로 — 세계를 옮기지 않았다.** 다섯 줄 = `seed42.json`(통치자·방랑자의 꼬리까지, `wrote` 경로 밖), 해시 판 둘 = 커맨드렛; 다섯 판의 CSV 셋(`.csv`·`_rows.csv`·`_claims.csv`)이 이 커밋 전의 빌드와 칸마다 같다(`TickMicros`·`SaveBytes`·`SaveRawBytes` 밖); `-WithoutB6bData=1` 다섯 줄 {OFF}와 그 CSV도 전과 같다; `end check`·`race check`·`LogCosmic: Error` 0.
> - **시간(G1)**: 한가한 기계(여유 메모리 {FREE_MIN}~{FREE_MAX} GB, 다른 하네스·편집기 없음)에서 한 세션에 번갈아 잰 직렬 AI 단계 µs/틱(판마다 세 번의 중앙값; 전 = 이 커밋 앞의 빌드 `{HBR}`의 작업 트리, 뒤 = 이 커밋의 빌드): 입력 없음 3,000 {B1} → {A1}, 통치자 3,000 {B2} → {A2}, 방랑자 6,000 {B3} → {A3}, 입력 없음 20,000 {B4} → {A4}, 통치자 20,000 {B5} → {A5}(경제 단계 {E_RANGE} — 뒤/전 {E_RATIO}; 위기 단계 20,000틱 {C4B} → {C4A}). **다섯 줄 모두 16.05 아래 — 문턱(B6a 10.7의 1.5배)은 그대로이고 G1이 닫혔다.** 끈 판도 3,000 {OB1} → {OA1}, 20,000 {OB2} → {OA2}(B6a의 코드도 같은 정렬을 하고 있었다). 시제품(함수 안 `static`, 탐침 빌드)의 7.16 / 6.67 / 8.66 / 12.90 / 10.80과 견준다.
> - **남는 몫**(설계의 일 — 모형): 기억과 이름 배열 없는 위기의 답 뒤에도 켬 − 끔이 약 +1~+5 µs/틱 남는다(입력 없음 3,000 +1.3, 20,000 +4.9, 통치자 20,000 +2.4). 조약이 없는 셋째 칸 백성이 틱마다 다시 묻고 국경의 짝을 찾는다(거절되면 다음 틱에 — 결정 1); 넷째 칸 위 백성이 틱마다 법칙 걸음을 지난다(결정 11); 업적 행이 다섯이라 판정마다 둘을 더 본다; 넷째·다섯째 칸 백성과 불가침 조약 때문에 세계가 다르다.
> - **미룬 것**: ① 짝 찾기의 국경 걷기 생략(조약의 문 `TreatyBlockOf`의 거절 검사 — 저장된 관계와 기본 목표가 문턱의 같은 쪽이면 국경을 걷지 않는다; 비트 그대로) — 짝 찾기 덩어리는 0.28 / 2.19 / 0.53 µs/틱이고 얻는 것은 많아야 약 1.1(재지 않음); 단일 창구라 섞인 경우의 새 단언 둘과 변이 넷, `Sim/CosmicDiplomacy.cpp`를 깁는 옛 패치 열하나가 든다 → `[B-6b12]` 또는 B7. ② 위기 단계의 칙령 훅을 성계마다 한 번(문 밖): 20,000틱 위기 단계 켬 27.0 대 끔 12.1 µs/틱(기억을 넣은 빌드), `FindEdict` 틱마다 919번(끔 1.5); 3,000틱에서는 8.6 대 7.8 — 은하 법칙이 선 뒤의 값이다. 틱 전체 평균은 기억으로 61.6 → 48.0(입력 없음 20,000)이라 한 프레임 열 틱이면 약 0.5 ms이고, 위기 단계에는 문이 없다. `Sim/CosmicEdicts.h`에 선언 하나가 들고 두 파일을 깁는 옛 패치가 176(19 + 157) → 틱 예산이 생길 때(B8) 또는 `[B-6b12]`.
> - **기록**: `docs/21:1536`(시뮬의 `Tables` 멤버 — 기억), `docs/40`(새 테스트 행 둘·러너 줄), `docs/45:26`·`:72`(엔진의 사실), 위 §3 '묶음 B'의 까닭 줄, 헤더 §11의 짧은 줄, `DECISIONS_LOG`, STATUS `:3`·`:26`·`:27`·`:32`. CHANGELOG는 §1.2대로 `[B-6b12]`.
> - **헤더와 다르게 하거나 더한 것**: ① `[B-6br]`은 리뷰의 고침이 아니라 묶음 B의 시간 문(G1)의 고침이다 — 헤더 §8의 `[B-6br]` 행('리뷰의 고침이 크면 그 고침만')을 `[B-6b9]` 지침서의 G1 갈래(코드의 까닭 → `[B-6br]`)가 썼다; 헤더 §8의 행은 그대로 두었다(헤더 {H_BYTES}바이트 — 지침서가 스스로 둔 여유 149,000). ② 판정의 시제품은 함수 안 `static`이었다 — 인스턴스 멤버로 옮겼고(같은 일, 숨은 전역 없음), 판정이 권한 `bValid`는 두지 않았다(빈 기억은 빈 표에만 맞고 그 답이 빈 배열이다). ③ 판정이 고른 `IsHazardAnswer`의 꼴(행 구조체를 한 번 보고 값을 바로 걸음)이 아니라 키를 걸으며 `FindHazard`를 그대로 부른다 — 행 구조체가 없는 표의 에러 줄까지 같고, 얻는 것은 약 0.4 µs/틱 덜하다. ④ 시간 다시 잼의 '전' 트리는 판정의 SAND가 아니라 새 작업 트리였다(SAND는 지워져 있었다).

### 7.5 H §11: one line after the `[B-6b8b]` bullet, before `## 추가 이야기와 보완`

> `- \`[B-6br]\`({DATE}): 시간 문 G1 — 정렬한 이름의 기억과 할당 없는 위기의 답(비트 그대로). 세부는 노트 §5.`

- With a 10-character date the line is 142 bytes. H goes 148,811 → 148,953, under the brief's self-imposed 149,000 (H §8's limit is 150,000).
- Check `{H_BYTES0}` + the line's `wc -c` < 149,000. If it is not, use "- `[B-6br]`({DATE}): G1의 고침 — 이름 정렬의 기억·할당 없는 위기의 답, 비트 그대로. 세부는 노트 §5." (136 bytes).
- **Make no other edit to H.** After this line only about 47 bytes are left under 149,000.

### 7.6 `DECISIONS_LOG.md`: after the `[B-6b8b]` line (`:43`)

> "- `[B-6br]`({DATE} — 시간 문 G1의 고침, 헤더 §8의 `[B-6br]` 행·노트 §2.3 `:694`의 문턱): 묶음 B의 직렬 AI 단계 23.93~31.90 µs/틱이 B6a 10.7의 1.5배 16.05를 넘은 까닭 — 기계 부하(×1.33~1.41; B6b가 손대지 않는 경제 단계는 ×1.51~1.83)와 그 밑의 코드: `FCosmicDataTables::GetFeatRowNames`가 부를 때마다 행 이름을 모아(엔진의 `GetRowNames` — 임시 `TSet`) 대소문자를 가리지 않는 비교로 다시 정렬했고, AI 단계가 틱마다 약 30번 불렀으며 B6b의 업적 행 둘이 한 번의 값을 176 → 350 ns로 올렸다(단계의 41~55%; 설명되지 않은 몫 없음 — 개발자에게 갈 물음 없음, 문턱 그대로). 고침: 정렬하는 접근자 여덟이 표마다 기억(키 차례와 정렬한 이름)을 두고 표의 키 차례가 원소마다(대소문자·번호까지) 같을 때만 쓴다 — 같은 입력의 같은 정렬이라 답은 같은 배열; `IsHazardAnswer`는 이름 배열 없이 행 지도를 걷는다(같은 키·차례·`FindHazard`). 머리(`Data/CosmicDataTables.h` — UPROPERTY가 아닌 `mutable` 멤버)는 소스보다 먼저 상시 승인으로. 비트 그대로(다섯 줄·해시 판 둘·CSV — 시간 칸 셋 밖·`-WithoutB6bData=1` 다섯 줄). 한가한 기계에서 한 세션에 번갈아 잰 직렬 AI 단계: {B1}·{B2}·{B3}·{B4}·{B5} → {A1}·{A2}·{A3}·{A4}·{A5} µs/틱 — 다섯 줄 모두 16.05 아래, G1 닫음. 남는 켬 − 끔(모형 +1~+5)은 설계의 일(결정 1·11). 미룬 것: 짝 찾기의 국경 걷기 생략(많아야 약 1.1 — `[B-6b12]`/B7), 위기 단계의 칙령 훅(20,000틱 위기 단계 켬 27 대 끔 12 — 문 밖, 틱 예산이 생길 때). **테스트**: 227(새 `Data.SortedNamesFollowTable`·`Data.HazardAnswerAnyRow`). **변이**: 새 `b6br_` 아홉 모두 깨짐, 다시 돌린 옛 변이 0. 리뷰: {REVIEW}. 세부는 B6b 노트 §5 `[B-6br]`."

### 7.7 STATUS

- **`:3` (갱신):** add `·\`[B-6br]\`` after the last B6b commit named in the parenthesis.
- **`:26` (코드 사실):** "자동화 테스트 225종 통과(" becomes "자동화 테스트 227종 통과(B6b `[B-6br]`은 새 파일 `Tests/CosmicTableNamesTests.cpp`의 `Data.SortedNamesFollowTable`·`Data.HazardAnswerAnyRow` — 정렬한 이름의 기억과 이름 배열 없는 위기의 답, 비트 그대로; ". The rest is unchanged.
- **`:27` (기준값):** prepend "`[B-6br]`도 비트 그대로(정렬한 이름의 기억 — 답은 같은 배열), ".
- **`:32` (다음 작업 2):**
  - "…`[B-6b8]`·`[B-6b8b]`은 끝" becomes "…`[B-6b8]`·`[B-6b8b]`·`[B-6br]`은 끝".
  - "AI 단계 시간의 까닭 찾기(묶음 B의 시간 문: … 코드의 까닭이면 `[B-6br]`) →" becomes "`[B-6br]` 끝(묶음 B의 시간 문 G1 — 까닭은 매 부름의 업적 이름 정렬과 기계 부하; 고친 뒤 직렬 AI 단계 {A1}~{A4} µs/틱, 모두 16.05 아래) →".
- **CHANGELOG:** left to `[B-6b12]`, as `[B-6b8b]` did (N §1.2).

---

## 8. Commit message

- **Title:** `[B-6br] 시간 문 G1의 고침 — 정렬하는 표 접근자 여덟의 기억과 이름 배열 없는 위기의 답, 직렬 AI 단계 {B_MAX} → {A_MAX} µs/틱(다섯 줄 모두 16.05 아래), 비트 그대로`
- **Body, in order:**
  1. **The cause** (two sentences): bundle B's 23.93-31.90 were load (×1.33-1.41), and under the load sat the per-call sort of feat names (176 → 350 ns, about 30 calls a tick, 41-55% of the stage).
  2. The **헤더 먼저** line (§1.2).
  3. **The fix:** A and B, one line each.
  4. **Why the answers are the same:** one line (P1, P6).
  5. **Bit-identical:** the five hash pairs, the two hash runs, the CSVs (three timing columns excluded), and `{OFF}`.
  6. **Time:** the five lines before → after, the economy ratio, free memory.
  7. **Tests:** 227. **Mutations:** 9 new, all broken; 0 old.
  8. **Deferred:** C and D, one line each.
  9. `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`
- Push after the commit (origin, as usual).

---

## 9. Order of work, effort, risks

**Effort.** High: H §8 gives this row "고침대로", and the fix is small but touches determinism. Run one lean ultracode implementation workflow; the implementers do not run the runner ('일의 속도').

**Order.**
1. **§0.** Run §0.1-§0.5 and §0.7. Start the before tree's build (§0.6) as soon as §0.5 is done.
2. **Approve the header.** The lead approves §1.3 and records the date.
3. **Implementers, in parallel:**
   - **I1:** the header (§1.3), `CosmicDataTables.cpp` (§2.1-§2.3), `run_checks.ps1` (§2.6).
   - **I2:**
     - the test file (§3);
     - the nine patches (written against I1's text; not run);
     - the nine README rows;
     - `csv_same.py` with its self-test (§4);
     - `run_timing_ab.ps1` and `timing_summary.py`, dry-tested on §0.5's logs as a one-sided "after" set.
4. **Build pass:** `run_checks.ps1 -Tier smoke` with the editor closed. It waits for the before tree's UBT if that is still running. It must pass with no baseline change, including the new tests.
5. **In parallel:**
   - **Machine (early):** §4 (c) on the smoke tier's three runs (`checks`, `checks_ruler`, `checks_wanderer`), then the off runs and their CSVs (§4 (d)).
   - **Lenses** (read-only; each finding gets a rebut-verify):
     - **L1, proof and determinism:** §2.4 P1-P8 against the code, §1.5 facts, the before/after evidence.
     - **L2, tests and mutations:** the assertion ↔ patch table, label truth, the rerun derivation.
     - **L3, docs and records:** anchors, H bytes, rule 11, and that `[B-6b11]`'s `<BR_WORLD>` reads "no" from N §5's 기준값 item.
6. **Fix.** If a fix touches source, repeat step 4.
7. **Mutations (§5),** serially. Then `apply_after.txt` and a clean `git status`.
8. **The one final runner:** `run_checks.ps1 -Tier full`. Its two passes are the "tests twice".
9. **Machine (final):** §4 (c) on all five, (d) and (e).
10. **Time (§6)** on a quiet machine. Nothing else may run.
11. **Records (§7).** These are docs only, so there is no rebuild.
    - `python -I Tools/check_docs.py` must pass.
    - `wc -c` H.
    - Commit (§8) and push.
    - Then `cmd /c rmdir C:\Users\rhwwl\AppData\Local\Temp\b6br_b` (removes the junction, not the tree) and `git -C <ROOT> worktree remove --force <OUT>\before_wt`, unless the lead keeps the tree for a later timing.

**Risks and resolutions** (recommended = approved):
- **R1. A line, a hash run or a CSV cell moves** (§4).
  - The memo or the walk is not identical. **Stop**: never `-UpdateBaseline` in this commit (H §8).
  - Apply a temporary probe that restores the old bodies, `b6br_probe_uncached`: a copy of the HEAD bodies. Rerun the moved run on both builds and diff the first differing CSV sample to find the caller.
  - Suspects, in order:
    1. a caller that mutates the returned array in place through a reference (none at HEAD);
    2. a table that is edited between two calls without a key change (not possible: rows are compared by key only, and the names depend on keys only);
    3. a `GetRowMap()` override (none in the engine).
- **R2. UHT or the compiler refuses the `mutable` private members.** Fallbacks, in order:
  1. drop `private:` (members public, with the same comment);
  2. one non-reflected member `mutable FCosmicTableMemos Memos;` that holds the eight;
  3. last, the prototype's function-local `static` (exact through the key check, but shared — note it in N §5 ②).

  Each fallback keeps §2.4 and the tests as they are.
- **R3. A gate line ≥ 16.05 on a valid session.**
  - First rerun that line's three pairs.
  - If it still fails, the remainder is design, class (a) (judgement §3): C becomes the first candidate (exact, at most about 1.1 µs).
  - If even the modelled C cannot bring the line under 16.05, take the numbers to [개발자]. **Do not change the threshold** (judgement §6: 16.05 was frozen before the data).
  - The commit itself may still land, since it is bit-identical. But N §5 and DECISIONS_LOG then say "G1 열림 — {numbers}", and `[B-6b10]` waits for the lead.
- **R4. The machine never gets quiet** (free memory < 8 GB, or another harness running).
  - Wait; never measure under load.
  - If the session cannot run within the day, ask the lead. The records must not carry unmeasured numbers.
- **R5. The before tree cannot be built** (path length, LFS pointers).
  - Use the junction path, check the asset magic bytes, and run `lfs checkout`.
  - If it is still impossible: the lead decides whether §0.5's start-build stage lines (one sample per line, on the same machine) may stand in as "before". Record the deviation in N §5 ④.
- **R6. `b6br_memo_case_blind` survives.**
  - That would mean the build does not keep case, or the name pool reused a display entry, and ii′-2 is then vacuous.
  - Check `FName(TEXT("Feat_D")).ToString()`.
  - If case is not kept, drop that patch and ii′-2, keep `CaseSensitive` in the code (a stronger check is harmless), and record it.
- **R7. An existing test fails in the full pass.**
  - Rerun it with `b6br_probe_uncached` applied.
  - If it passes uncached and fails with the memo, stop (R1).
  - If it fails both ways, it is not this commit's: find it first.
- **R8. Header bytes.** Use the 136-byte line. No other H edit.
- **R9. Order.** If the lead wants `[B-6b10]` first, this brief still holds, but §0.4's `{BASE}` and §6's before tree must be re-derived on that tree. `[B-6b11]`'s R11 already expects a `[B-6br]` that may land late.
- **R10. [개발자].** Nothing in this commit needs the developer, unless R3 is reached.

---

## 추가 이야기와 보완

- **C later (exact, ready).**
  - In `TreatyBlockOf`, compute `Bar = TreatyBarOf(Civ, Tables, Rules)`, `Stored = Other.Relations.FindRef(Civ.CivID)` and `Base = CosmicDiplomacyImpl::BaseTargetOf(State, Rules, OtherIndex, CivIndex)`.
    - Both `!(… >= Bar)`: `Refused` without `ViewOf`.
    - Both `>= Bar`: past the refusal.
    - Otherwise: `ViewOf` as now.
  - It is exact because `ViewOf` (`Sim/CosmicDiplomacy.cpp:291-302`) is exactly `Stored` when bordering and `Base` otherwise, and `!(x >= Bar)` matches NaN either way.
  - Two new assertions in the first-reason block of `Tests/CosmicDiplomacyTests.cpp` (`:491-540` at HEAD):
    - a bordering pair with a stored view ≥ the bar and a base below it gives `None` (set `Civs[1].Ethics.Pacifism = -1` and `Relations` to 0.5);
    - a pair that does not border, with a stored view below the bar and a base at or above it, gives `None`.
  - Four mutations:
    1. mixed → `Refused`: the first new assertion breaks.
    2. mixed → pass: the existing "-0.1 refuses" and "does not border … -0.2" cases break.
    3. "both below" read from `Stored` only: the second new assertion breaks.
    4. "both above" read from `Stored` only: the existing "does not border … -0.2" case breaks.
- **D later.**
  - Add `SpawnMultiplierOf(Hooks, Tag)` beside the current one in `Sim/CosmicEdicts.h`. The crisis stage's row loop (`Sim/GalaxySimulationSubsystem.cpp:2344-2377` at HEAD) takes `HookEdictsOf(State, StateIndex, Tables, SystemIndex)` once per system.
  - The same definitions in the same order give the same product order. `CheckAll` writes nothing.
  - A smaller, body-only step: in `HookEdictsOf`, test `!Names.Contains(Row)` before `FindEdict`. It is a pure reorder of a conjunction and avoids most of the repeated finds of the three laws.
  - Either way, `Sim/CosmicEdicts.cpp` carries 19 old patches.
- **`FindHazardByTag`** still copies the memo's array and walks it (read_map C4). A walk over the row map that keeps the lexically smallest matching name would remove the copy; it is exact for the same reason as B. It is worth about 0.1 µs/tick and not worth a commit alone. Fold it into C or D.
- **`AsksForTreatyOf` without a rung gate** (judgement 추가 이야기, read_map C2). After the memo it costs under 0.4 µs. If the design wants the code to say "only when a judged row of this rung asks", it is a body-only change for `[B-6b12]`.
- **A per-stage time line in bundle C.** `[B-6b11]`'s harness could print the economy stage and free memory next to its serial AI line, as §2.6 now does for the runner. A loaded run then shows itself without a separate judgement.
- **The before-tree pattern.** If later commits need before/after timing, the worktree + junction + `run_timing_ab.ps1` of §0.6/§6 can be kept as a reusable harness in `<SP>`. They are not in the repo; a `docs/40` line would be needed only if they ever are.
