# [B-6b6] implementation brief (final): data ① third rung to fourth (the extinction row, `project_ark`, `feat_passage_by_ark`, decision 5's code-death pin, bundle A)

The lead runs this as a lean ultracode workflow once [B-6b5] is committed. I checked every HEAD line cited here with `git show HEAD:` at `cef8cc7`.

**Short names:**
- [H''] = the tree after [B-6b4] and [B-6b5].
- H = `docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md`; N = its `_notes.md`.
- B4brief and B5brief = `scratchpad\b6b4\brief_final.md` and `scratchpad\b6b5\brief_final.md`.
- Paths are under `Source/Cosmic_Ascension/` unless they start with `Tools/`, `docs/` or `Config/`.

Per H:392 this commit is effort high. It runs the runner `-Tier full -Data` once (N:709), checks H §5.4 ⓐ–ⓒ (N:597), and takes an ultracode balance review. **Values:** c stays at 1e-4. Only the ark's per-tick cost may move in this commit (§4.6).

---

## 0. Start state: what to re-check on [H''] before any agent runs

1. **Both earlier commits are in.**
   - `git log --oneline -3` shows `[B-6b5]` on top of `[B-6b4]`.
   - `git status` is clean apart from the user's known local changes.
   - If [B-6b5] is not committed, stop: the row texts do not exist yet.

2. **Group C texts.** Take them verbatim from N §1.5 as committed. At HEAD, N:469-471 is empty.
   - Use `scratchpad\b6b5\merged.md` only if N §1.5 is still empty, and record that in N §5.
   - You need:
     - C1 (`crisis_species_extinction`): DisplayName '절멸 위기', and Outbreak, Expire and Resolve ×3 each;
     - C2 (`project_ark`): DisplayName and CompleteTemplates ×3;
     - C3 (`feat_passage_by_ark`): Title, MythicNarrative and LogTemplates ×3.
   - The spec is B5brief:257-300. Re-read the [B-6b5] block of N §5 and its "[B-6b6] hand-offs" (B5brief:643-650).

3. **Schema at [H''].** Diff `Tools/data/schema.json`, `Tools/cosmic_data_schema.py` and `Tools/test_data_schema.py` against HEAD. Confirm that:
   - the nine new lists exist (B5brief:582-592);
   - the new DT_Hazards rule `when HazardTag is Hazard.Extinction` puts `stay_words`, `death_frame_words` and `desolation_words` on all three template columns, and `self_rescue_words` on `ResolveTemplates` (B5brief:625-627);
   - the DT_Feats text columns carry the long list (B5brief:612);
   - DT_Megaprojects has the `project_binding` rule (B5brief:628-630);
   - which columns' `forbidden` value is still a string. At HEAD, DT_Hazards `DisplayName` is the string `"verdict"` (`schema.json:697`); the template columns are :711-713.

4. **Re-derive every line number** cited below on [H''], C++ ones included, and patch targets too. HEAD lines include `Tests/CosmicDataTests.cpp:2660-2711`, `Sim/CosmicHazards.cpp:751` and `Tests/CosmicTestHelpers.h:268-271`/`:375-387`.

5. **[B-6b4] tests on game tables.** On [H''], run `git grep -n "HasJudgedRowAskingAt\|HasJudgedRowAt" Source/Cosmic_Ascension/Tests`.
   - Any assertion that reads "false on the game tables" for `Tier3_Causal` + `CivHasTreaty` becomes true here and changes in this commit (§2.4).
   - The B4brief pins on the T4 row (B4brief:409-411) stay false until [B-6b7].
   - `UI.PeoplesTab`'s "no tip with the board's tables" (B4brief:503-504) holds. `LayBoard` uses `CosmicTests::WithoutProjects`, which empties DT_Feats (`Tests/CosmicTestHelpers.h:471-486`).

6. **Mutation apply state.** In a scratch copy of [H''], run `git apply --check` on every patch under `Tools/mutations/` and save the result as `scratchpad\b6b6\apply_before.txt` (B5brief:901-905).

7. **Clean build first.** On a clean [H''] (`git status` clean, no leftover mutation), build both targets before any base run. Earlier workflows end with mutation runs. A leftover Tests-module mutation (for example in `WithoutB6bData`, or in the `race:` counters) would not show in the keys.

8. **Base copies for ⓑ.** On that build, before any edit, run the five commandlet runs:
   - with the runner's exact arguments (`Tools/run_checks.ps1:116-132`, clock on);
   - into `Saved\Balance\b6b6_base\`, with an absolute `-Out` and each run's own `-AbsLog`.

   Each key must equal `Tools/baselines/seed42.json` at [H''], which still holds the five [B-6b5] lines (B5brief:964-970).

9. **Nothing else may be running.** No UnrealEditor or UBT process from another workflow (§4.2).

---

## 1. Data rows

### 1.1 `Tools/data/DT_Hazards.json`: append after `crisis_rebellion` (ends at :261)

- **Model row:** `crisis_pre_supernova` (`DT_Hazards.json:145-194`).
- **Values:** from H:97-99 and decision 4 (H:421).
- **c = 1e-4:** N:527.

```json
	{
		"Name": "crisis_species_extinction",
		"HazardTag": "Hazard.Extinction",
		"SourceDoctrine": "",
		"DisplayName": "절멸 위기",
		"CountdownTicks": 250,
		"SpawnRule": "Draw",
		"SpawnChancePerTick": 0.0001,
		"SpawnConditions": [
			{"Type": "CivTierAtLeast", "Tag": "", "Key": "None", "Value": 3.0}
		],
		"OnExpireEffects": [],
		"BranchChance": 0.0,
		"OnExpireBranchEffects": [],
		"OnExpireNeighbourEffects": [],
		"OnResolveEffects": [],
		"RulerResolutions": ["project_ark"],
		"WandererResolutions": [],
		"LogCategory": "Log.Crisis",
		"Severity": 1,
		"OutbreakTemplates": [<N §1.5 C1 Outbreak v1>, <v2>, <v3>],
		"ExpireTemplates": [<C1 Expire v1>, <v2>, <v3>],
		"ResolveTemplates": [<C1 Resolve v1>, <v2>, <v3>],
		"OutbreakRow": "None"
	}
```

- Keep the file's tabs. The previous row's closing `}` becomes `},`.
- No row-name constant (§1.4). No `WandererResolutions` (#164, H:99).
- **Draw sum.** Today the Draw shares add up to 4e-4 (`DT_Hazards.json:9`, :109, :152). This row makes it 5e-4, so `Pick` is never rescaled (`Sim/GalaxySimulationSubsystem.cpp:2396`).
- **Order.** The new share is appended last (`Data/CosmicDataTables.cpp:143-148`; 's' > 'r'). Every earlier `Reached` stays bit-identical (:2398-2410).
- **No '방주' in any C1 cell** (B5brief:257).

### 1.2 `Tools/data/DT_Megaprojects.json`: append after `project_binding` (ends at :73)

```json
	{
		"Name": "project_ark",
		"DisplayName": <C2 DisplayName>,
		"RequiredTier": "Tier3_Causal",
		"CostPerTick": {"Energy": 3.0, "Matter": 3.0, "Insight": 0.0, "Influence": 0.0, "Causality": 0.0},
		"RequiredProgress": 150.0,
		"CompleteTemplates": [<C2 v1>, <v2>, <v3>]
	}
```

**Cost (recommended, counts as approved): Energy 3.0 and Matter 3.0.**
- It is `project_stabilize`'s 2/2 (`DT_Megaprojects.json:54`) scaled by `TierMultiplier` from the second rung's ×1.5 to the third's ×2.2 (`DT_Rules.json:60`): 2 × 2.2/1.5 = 2.93 → 3.
- Causality is 0 (rule 7; income 0, `Sim/GalaxySimulationSubsystem.cpp:1768-1772`).
- Influence is 0, so the ark never competes for Influence with the treaty that the feat needs (the treaty step pays 50, :2961-2965).
- The per-tick cost is the one value this commit may change after the balance lens (§4.6). `RequiredProgress` (P_ark) and c belong to [B-6b9] (H:338, H:403).

**Margin.** `Data.Integrity`'s ark margin now bites on game data: 150 / `AIAscensionSpeedMultiplier` 1.0 < 250 (`Tests/CosmicDataTests.cpp:2195-2235`).

### 1.3 `Tools/data/DT_Feats.json`: append after `feat_three_worlds` (ends at :66)

```json
	{
		"Name": "feat_passage_by_ark",
		"Title": <C3 Title>,
		"MythicNarrative": <C3 MythicNarrative>,
		"RequiredTier": "Tier3_Causal",
		"TargetTier": "Tier4_Transcendent",
		"Path": "Ruler",
		"CompletionConditions": [
			{"Type": "MegaprojectComplete", "Tag": "", "Key": "project_ark", "Value": 0.0},
			{"Type": "CivHasTreaty", "Tag": "", "Key": "None", "Value": 1.0}
		],
		"AscensionCost": [
			{"Type": "ShiftEthics", "Tag": "", "Key": "Openness", "Value": -0.1, "DurationTicks": 0}
		],
		"GrantedPowers": "(GameplayTags=)",
		"DedicationBank": "",
		"FX": "",
		"Sound": "",
		"LogTemplates": [<C3 v1>, <v2>, <v3>]
	}
```

**Schema fit:**
- `ShiftEthics` Key is a `word: axis` (`schema.json:1236`, `words.axis` :34).
- `CivHasTreaty` takes no tag (:852, rule 6′), and Value is an integer ≥ 1 (:1228).
- The `MegaprojectComplete` Key names a DT_Megaprojects row (:1205).
- The tier pair meets the T3→T4 rule.

**Text:** the slots are `{Civ}` and `{HomeSystem}` only. `{HomeSystem}` is the capital (`Ascension/AscensionSubsystem.cpp:196-205`). No '다시는'.

At HEAD this would be the first `Tier4_Transcendent` target in DT_Feats (`DT_Feats.json:7`, :29, :50), so the off side has no T3→T4 row.

### 1.4 Row-name constants

**Recommended (counts as approved): move none of the three into `CosmicRows::`.**
- No game code calls these rows by name. The hook reads the tag (`Sim/CosmicHazards.cpp:746-752`), and the feat and ark walks go by table order (H:247 "코드가 부르지 않아 상수 없음").
- A `CosmicRows::` constant forces a `required_rows` entry (`schema.json:28`, `Tools/cosmic_data_schema.py` `load_code_rows`), which would falsely say that the code calls the row.

**Edit only `Tests/CosmicB6bRowNames.h`'s comment:**
- `:1` "the names of B6b's rows that the game's data does not hold yet" becomes "the names of B6b's rows that no game code calls by name".
- `:4-8` keep the reason for no constant (the checker holds each constant to a required row). Replace "so a name cannot be a constant before the commit that brings its row" and ":8" with:

  > "A name moves to CosmicRows:: only with the commit whose code calls its row by name; the data commits [B-6b6]-[B-6b8] add rows the code reads by tag or by table walk, so their names stay here."
- `:17`, :21, :24: "- [B-6b6]" becomes "- in the data from [B-6b6]".

This is a header edit, so it needs a full build.

### 1.5 Tag description (`Config/DefaultGameplayTags.ini:20`)

Replace with (ASCII):

> "Extinction crisis (B6b): a Draw row, crisis_species_extinction, drawn on the settled systems of a people on the third rung or above, 250 ticks. Answered by project_ark in the response slot. Unresolved: the grace of the remnant is asked first (event_grace_remnant); otherwise CosmicHazards::LetPeopleGo lets every system of the people go and its ruler dies (CosmicHazards::KillsTheRulerOf, Death.Hazard), then DeposeRuler if the people holds nothing; the crossing stage removes the people."

### 1.6 Schema and checker tests (the hand-offs at B5brief:643-650)

1. **New list `words.ark_words = ["방주"]`**, appended after the [B-6b5] lists. Its `about` says: decision 33, '방주' only in the ark and the fourth-rung feat.

2. **`"노아"` appended at the end of `words.holy_and_myth`** (`schema.json:44`).
   - The existing test `Tools/test_data_schema.py:1809-1813` already guards it over every `read_sources()` file. That is every DT json and every `ST_*.csv` (`cosmic_data_schema.py:284-297`), with `schema.json` excepted.
   - Check that '노아' is not hidden inside a common word, against the hand list at :1815.

3. **The ark's row rule** in DT_Megaprojects, appended after [B-6b5]'s `project_binding` rule: `{"why": …, "when": [{"row": "project_ark"}], "then": […]}`.
   - **`then`:** `CompleteTemplates` lacks `self_rescue_words` and `holy_and_myth`; `DisplayName` lacks `rescue_words`, `title_words`, `holy_and_myth` and `self_rescue_words`. That is 6 tests. Two of them go beyond the hand-off; record them in N §5.
   - **`why` (people as subject, no verse):**

     > "the people crosses the extinction in the ark it built; the ark is no one's rescuer - no rescue, title or holy name in its name, no self-rescue, new birth, washing, new land or Flood picture in its completion lines (B6b header 3.3 rule 13, 3.5 rule 2, decision 33)"

4. **The '방주' guard (recommended, counts as approved).**

   **(a) Hard gate in `check_data`.** It refuses before any asset is written. Add `ark_words`:
   - to the `forbidden` of DT_Hazards `DisplayName` (`schema.json:697`; the string becomes `["verdict", …, "ark_words"]`, keeping any [B-6b5] lists) and of `OutbreakTemplates`, `ExpireTemplates` and `ResolveTemplates` (:711-713);
   - to `DT_Events.LogTemplates` (:450) and `DT_LogTemplates.Templates` (:326).

   Add two `row_not` rules (`cosmic_data_schema.py:617-623`, `holds` :1321-1322):
   - DT_Megaprojects `when [{"row_not":"project_ark"}]`: `DisplayName` and `CompleteTemplates` lack `ark_words`;
   - DT_Feats `when [{"row_not":"feat_passage_by_ark"}]`: `Title`, `MythicNarrative` and `LogTemplates` lack `ark_words`.

   **(b) Whole-project test** in `test_data_schema.py`, in the form of :1809-1813.
   - Over `read_sources()` minus `schema.json`, parse each DT json, skip the cells of the two exception rows by `Name`, and assert that '방주' stands nowhere else, ST_CA included.
   - This is a test, not a `check_data` gate. When it fails, the runner still writes the assets (`run_checks.ps1:35-39`, :48-53), but it records the failure and exits 1 (:157), so nothing can be committed. Its mutation is filed as "fails `test_data_schema.py` only".
   - At HEAD, ST_CA has no '방주', and neither does any [B-6b4] draft (B4brief:332).

5. **Exact pins.** On [H''], update every `assertEqual` that pins the `forbidden` list of a column this commit edits (the form of `Tools/test_data_schema.py:1804-1806`). Find them with a grep for each column's current list.

6. **Python harness.**
   - **`fixture_tables`** (`Tools/test_data_schema.py:276-302`) appends `megaproject_row("project_ark", RequiredTier="Tier3_Causal")` and `feat_row("feat_passage_by_ark", …)`. Copy `feat_test_climb`'s fields and change only the tiers to T3→T4. Reason: the checker refuses a `row`/`row_not` rule whose row the table neither requires nor has (`cosmic_data_schema.py:600-623`). Fix any case that assumed one feat row.
   - **`rule_setups`** (:3440-3530):
     - the ark rule → `(NOTHING, "project_ark")`;
     - the two `row_not` rules → `(NOTHING, "project_test_build")` and `(NOTHING, "feat_test_climb")`.
   - **`walked`** (:3563): +11. Set it from the final schema.
   - **New method `test_the_ark_and_its_word`:**
     - pins: `ark_words == ["방주"]` and `"노아" in holy_and_myth`;
     - one breaking case per new `forbidden` place: '방주' in the fixture hazard's DisplayName, Outbreak, Expire and Resolve, in an event line, and in an LT line;
     - named cases:
       - '{Civ}|hpp(은,는) 방주로 백성을 건져 냈다' on the ark's CompleteTemplates (`self_rescue_words`);
       - '노아의 배' on the ark's DisplayName (`holy_and_myth`);
       - '방주' in `feat_test_climb`'s LogTemplates (`row_not`);
       - '방주' in `project_test_build`'s CompleteTemplates (`row_not`);
       - '방주' in the two exception rows' own cells passes;
     - the whole-project test of 4(b), plus two injected cases through `read_sources`/`check_sources` that must fail: one in a DT_Encounters Body and one in an `ST_CA.csv` text.
   - **Reword the comment at :3398** to "the fixture's own extinction - the game's row lives in the project data, which the fixture replaces".

7. **Docs of the schema.**
   - **`docs/70` rule 7:** [B-6b5]'s "…지금은 글 감사가, `[B-6b6]`부터는 스키마가 본다" (B5brief:688) becomes "스키마가 본다(`ark_words` — 절멸·은혜·로그 줄의 열과 두 행 밖의 프로젝트·업적; 나머지는 검사기 단위 테스트가 원본 전체를 읽는다)".
   - **`docs/21:1266` ('스키마의 꼴', §7.3 on [H''])** gains:
     - `ark_words` and its places;
     - the two `row_not` rules;
     - the ark's row rule;
     - '노아' in `holy_and_myth`.

8. **The new rows pass every rule already in [H''].** That means rule 5 (`schema.json:778-788`), rule 6′, rule 7, [B-6b5]'s extinction word rule and feat lists, and the unique `HazardTag` (:716-718).
   - Gate: `python -I Tools/check_data.py` reports 0, and `python -I Tools/test_data_schema.py` passes.

### 1.7 Assets to commit

Run `Tools/make_data_tables.py` through the runner's `-Data`. Commit exactly:
- `Content/CA/Data/DT_Hazards.uasset`, `DT_Megaprojects.uasset` and `DT_Feats.uasset`;
- `Tools/data/generated.sha1`.

Any other `DT_*.uasset` or `ST_CA.uasset` change means stop and find out why.

---

## 2. Tests

### 2.1 Decision 5's code-death pin in `Data.Integrity` (H:369, N:434, `DECISIONS_LOG.md:34`)

In `Tests/CosmicDataTests.cpp`, in the block at :2660-2702 (HEAD), after the `KillPlayersOf` pin (:2669-2672):

- Walk `Game.GetHazardRowNames()` and collect each row whose `CosmicHazards::KillsTheRulerOf(Row->HazardTag)` is true.
- `TestTrue` that the list is exactly `{ CosmicB6bRows::CrisisSpeciesExtinction }`. Message:

  > "The code's one death is the extinction's expiry (decision 5, DECISIONS_LOG B6b [B-6b2b]): exactly crisis_species_extinction kills the ruler at its expiry - another source of death is a new decision (found: %s)"
- `TestTrue` that this row's `OnExpireEffects`, `OnExpireBranchEffects` and `OnExpireNeighbourEffects` are empty and that `BranchChance == 0`.
- `KillPlayersOf`'s pin stays as it is.
- Comment at :2663-2666: "; [B-6b6] pins that row" becomes "; from [B-6b6] the next pin holds that code death to the one row".

**Mutations:**
- **`b6b6_kills_ruler_not_extinction`:** `Sim/CosmicHazards.cpp:751` returns `false`. It should fail `Data.Integrity`, and `Sim.ExtinctionExpiry` too.
- **`b6b6_data_extinction_expiry_effect`** (an asset-rebuild data patch). It does three things:
  - drops the three `max_len` 0 tests and `BranchChance is 0` from rule 5 (`schema.json:778-788`, keeping `rows_of`);
  - gives `crisis_species_extinction.OnExpireEffects` one `ChangeStability` (a valid effect, `schema.json:1239`; not `KillPlayer`, so `KillPlayersOf` stays quiet);
  - rebuilds the assets.

  It must fail the new empty-expiry clause of `Data.Integrity`. List it in README:5-7 among the patches that rebuild assets.
- **Rerun `b6b2b_kills_ruler_other_tag`.** It now also fails `Data.Integrity` (two rows). Add that to its README row.

### 2.2 The `WithoutB6bData` tripwire (`CosmicDataTests.cpp:2693-2711`)

- `Today == TArray<FName>{ CosmicB6bRows::FeatPassageByArk, CosmicB6bRows::ProjectArk, CosmicRows::ProjectBinding, CosmicB6bRows::CrisisSpeciesExtinction }`. This is the `Note` order of `Tests/CosmicTestHelpers.h:375-387`.
- Message: "Today's data: WithoutB6bData takes out [B-6b6]'s three rows and project_binding (taken: %s) - [B-6b7] and [B-6b8] update this pin with their rows".
- Comment: "(1) Today's data holds four of B6b's ten rows".
- The full copy (:2712-2779) still holds.
- Rerun `b6b3b_without_b6b_*` (README:868-873).

### 2.3 Ark-margin comment (`CosmicDataTests.cpp:2195-2204`)

- "The game holds no extinction row before [B-6b6] …" becomes "The game's extinction and ark are held by the first assertion (from [B-6b6]); the copies below hold the edges - …".
- The `Arked` copies still pass (`Expect` is any-carries, :1215-1218).
- Rerun `b6b2c_data_ark_margin_half` (README:792). It now also fails on the game row.

### 2.4 Tests whose meaning changes once data is on

**Rule (N:425):** on any break, find the cause before changing a seed or an expected value. Record every outcome in N §5.

| Test | Expected | On a break |
|---|---|---|
| `Sim.EndingRecordOnce`, `Sim.EndingInert` (`Tests/CosmicEndingTests.cpp:655`, :784) | Unchanged (`EndingTables` applies `WithoutB6bData`, :155-158) | A leak through `WithoutB6bData`: fix the helper |
| `Sim.SightWalkMoved` (`Tests/CosmicKnowingTests.cpp:1851`, :1875) | CRC unchanged | Same |
| `Sim.SuccessionPossible` (`Tests/CosmicSuccessionTests.cpp:610`, :631) | Unchanged | Same |
| `Sim.NotablesFromEventsOnly` (`Tests/CosmicHazardTests.cpp:2983`) | Rerun | Widen only for the extinction's death and records closed at the fourth rung |
| `Sim.WandererLoop` (`Tests/CosmicWandererTests.cpp:1347`), seed 4 | Rerun | Find the cause first (#122 `FarAbove`, or an extinction deletion); record it in N §5 |
| `Sim.RealmCap`, `Sim.CivAIProjects`, `Sim.NotablesCapped` (`CosmicSettlementTests.cpp:1006`, `CosmicAscensionTests.cpp:1160`, `CosmicHazardTests.cpp:2932`) | Rerun, same shape | Cause first |
| `Sim.PhaseTwoExit` (`Tests/CosmicSimTests.cpp:2260`), seed 18, 20,000 ticks | Rerun | The cause is the extinction or the AI treaty step; record it, then change the seed only if it is not a leak |
| `Sim.StartBlockReasons` (`CosmicAscensionTests.cpp:951`) | Its sweep now includes `project_ark` | Fix the expected reason for an answer row if needed |
| `UI.TopBarAndPanel` ([B-6b4]'s extinction section) | A real row by tag, and a greyed ark button (`UI/CosmicSystemPanelWidget.cpp:1042` at HEAD) | Adjust only the counts that assumed no row |
| `UI.NextRung` 9b | `AddRow` replaces game rows of the same names | — |
| **Any other test that ticks a T3+ people on the game's DT_Feats or DT_Hazards** (32 test files assign T3–T5 at HEAD, for example `CosmicDiplomacyTests.cpp` and `CosmicEdictTests.cpp`) | The **AI treaty step** (a judged row with `CivHasTreaty`, `GalaxySimulationSubsystem.cpp:2955-2966`) and the **extinction share** now act. `WithoutProjects` empties feats but keeps the game's hazards (`CosmicTestHelpers.h:471-486`) | If the test's subject is not B6b, put `CosmicTests::WithoutB6bData` outermost (the [B-6b3b] precedent). Never change expected values or seeds to absorb it. Record each case in N §5 |

**`Feats.Reachable` (decision).** The test runs seed 42 at 20,000 ticks on the home row (`CosmicAscensionTests.cpp:1760-1780`; `SliceSystems`/`SliceCivs` = 0, `CosmicTestHelpers.h:48-49`). It counts **non-ruled** peoples only (:1772-1777).
- Decide from the runner's `checks_20000.csv` row at Tick 20000: the `Tier` of each `CivID` other than the ruled one. Do not use `race:` part 5, which counts the ruled people too (`CosmicBalanceCommandlet.cpp:3966`). Or run the raised assertion once on the build.
- **If ≥ 1 AI stands on T4 or above:**
  - raise the count to `>= Tier4_Transcendent`, with the message "An AI people reaches the fourth rung in 20,000 ticks of the game's data" (N:445);
  - add the mutation `b6b6_brake_always_shut` (`CosmicAscent::FourthRungOpenOf` returns false);
  - update `docs/40:238`.

  `docs/10_DESIGN/systems.md` has no `Feats.Reachable` at HEAD (N:348's `systems.md:409` is stale). Edit it only if a grep on [H''] finds one.
- **If not:** leave the test. If it fails, every T3 AI was erased; record that and send it to the balance lens.

**`Calling.DeathPipeline` extinction block (proactive fix).**
- `Tests/CosmicDeathTests.cpp:797-830` copies every game hazard (`QuietHazards`, :106-119) and adds its own `Hazard.Extinction` row. That would leave two rows of one tag.
- Fix: `Hazards->RemoveRow(CosmicB6bRows::CrisisSpeciesExtinction)` before `AddRow`. Include `Tests/CosmicB6bRowNames.h`.
- Reword :798-799 to "the quiet copy swaps the game's extinction for one of the test's".

**Stale comments, reworded only:**
- `Tests/CosmicTestHelpers.h:268-271`: "none in the game's data before [B-6b6]" becomes "the game's crisis_species_extinction from [B-6b6]";
- `Tests/CosmicUpperRungTests.cpp:776`;
- `Tests/CosmicWarningLineTests.cpp:110-111`.

### 2.5 Mutations (H:360 naming; N:709 speed rule)

**New patches** (each must fail before it is listed):

| Patch | Fails |
|---|---|
| `b6b6_kills_ruler_not_extinction` | `Data.Integrity` (code-death pin), `Sim.ExtinctionExpiry` |
| `b6b6_data_extinction_expiry_effect` (asset rebuild) | `Data.Integrity` (empty-expiry clause) |
| `b6b6_brake_always_shut` | `Feats.Reachable` (only if raised) |
| `b6b6_schema_ark_rule_dropped` | walker count; the '건져 냈다' case |
| `b6b6_schema_ark_guard_feats_dropped` | walker count; the feat `row_not` case |
| `b6b6_schema_ark_guard_projects_dropped` | walker count; the `project_test_build` case |
| `b6b6_schema_ark_words_off_hazards` | the hazard cases |
| `b6b6_schema_ark_words_off_lines` (off `DT_Events.LogTemplates` and `DT_LogTemplates.Templates`) | the event and LT cases |
| `b6b6_schema_myth_without_noah` | the list pin |
| `b6b6_data_extinction_says_ark` (C1 Resolve v1 + '방주') | `check_data.py` |
| `b6b6_data_ark_rescues` (C2 CompleteTemplates[0] + '구했다') | `check_data.py` |
| `b6b6_data_ark_word_in_card` (an encounter Body + '방주') | `test_data_schema.py` only (the whole-project test; it passes `check_data`, `schema.json:939`) |

**Rerun set, built mechanically (no hand-listed prefixes).** Every old patch whose `+++ b/` file this commit edits, plus every old patch whose README "Breaks" test is defined in a test file this commit edits. At HEAD that already means:
- 35 patches on `schema.json` (including `b3b_`/`b3d*_`/`b4b_`/`b4d1_schema_*` and `b6b2a_data_reach_word_renamed`; all Python only);
- 17 on `Tests/CosmicTestHelpers.h` (including `b2c_helper_keeps_edict`, `b6a6_exalt_as_before_is_the_game`, `b6b3b_fourth_cap_*`, `b6b3b_without_b6b_*` and `b5d2_without_death_keeps_expiry`);
- 15 on `Tests/CosmicDataTests.cpp`;
- 2 on `DT_Hazards.json` and 1 on `DT_Megaprojects.json`;
- the patches whose filter is `Calling.DeathPipeline`, `UI.ClaimMayFollow` (`CosmicWarningLineTests.cpp`) or a `CosmicUpperRungTests.cpp` test;
- and those of `CosmicAscensionTests.cpp`/`CosmicPanelTests.cpp` if `Feats.Reachable`, `StartBlockReasons` or `TopBarAndPanel` change.

Do not count `b4b_theodicy_one_bonus`/`_no_card_delay` as broken by this commit: they were already dead before B6b.

**Retakes.**
- Every patch that goes from "applies" in `apply_before.txt` to "does not apply" in `apply_after.txt`.
- Known now: `B4/b4d1_schema_myth_takes_baro` and `B4/b4d1_schema_verdict_without_soul_has_soul` (context `schema.json:44`), and patches whose context is :697 or :711-713.
- Retake each under a `b6b6_` name and mark the old README row "(no longer applies since [B-6b6] - taken again as `b6b6_…`)".

**Running them.** Serially, after the final build, never alongside a commandlet (§4.2). This is about 2-3 minutes per C++ patch, so plan for 2-3 hours. The whole-B6b rerun stays with [B-6b12].

---

## 3. Determinism and baselines

### 3.1 When the world can diverge

No new draw point exists:
- the spawn numbers are drawn before any row is read (`GalaxySimulationSubsystem.cpp:2310-2320`);
- the new share is appended last, and `CheckAll(SpawnConditions)` is pure (:2388);
- the expiry still draws its two numbers (:2299-2308).

Let T = min(first AI T3, ruled T3).
- **Nothing reads the new rows at or before T.**
  - The T3 judgment closes the AI stage (:2973-2976), after that tick's crisis stage.
  - The feat judgment filters by rung (`Data/CosmicDefinitionRows.h:300-303`).
  - The policy's one UI read is the ruled people's rung (`World/CosmicRulerPolicy.cpp:496`).
- **The world can diverge from T+1 at the earliest**, through whichever comes first:
  - an extinction placed in the crisis stage (which runs before the AI stage), or
  - the AI treaty step (`GalaxySimulationSubsystem.cpp:2955-2966`), once `AITreatyPartnerOf` finds a bordering non-ruled people that passes `TreatyBlockOf` (Hostile, Refused and Cost refuse, `Sim/CosmicDiplomacy.cpp:415-426`, :461-478).
- H:246's "절멸이 처음 놓인 틱부터만" is true of the draw table alone. Record this as a deviation in N §5 and as one clause in H §11.
- **Per run, record:**
  - `AITreatyFirst` (part 9);
  - the first extinction placement (`ExtinctionOnRuledFirst`, or the first `_rows.csv` row with `SourceRow` `crisis_species_extinction` in input-less runs);
  - the first differing CSV sample against the base.

### 3.2 Re-recording (`Tools/baselines/seed42.json`; H:358)

1. **Final run:** `run_checks.ps1 -Tier full -Data -UpdateBaseline`, editor closed. It rewrites the five lines and `verified_at` (`run_checks.ps1:102-107`). The two hash runs are compared with the commandlet's 3,000 lines (:139-155).

2. **`last_recorded_in_changelog`, by hand:**
   - `task`: "B6b B-6b6 (data on, third to fourth rung: crisis_species_extinction, project_ark, feat_passage_by_ark) - the first input-less world move of B6b";
   - the four input-less hashes;
   - `note`: the reason (§3.1, with each run's measured first mover) and the new hashes, keeping "Older values: …".

   UTF-8, no BOM.

3. **The breakable checks:**
   - **ⓐ** Five runs on the [B-6b6] build with the runner's exact arguments plus `-WithoutB6bData=1` give [B-6b5]'s keys letter for letter, the `wrote …` tail removed (`run_checks.ps1:76`):

     | Run | Hash / world |
     |---|---|
     | 3,000 | 2816687688 / 3300228750 |
     | ruler 3,000 | 3561771757 / 2462331303 |
     | wanderer 6,000 | 2665013209 / 2229239127 |
     | 20,000 | 2359424635 / 726895141 |
     | ruler 20,000 | 1611658162 / 29194274 |

   - **ⓑ** For each of the five on-runs (in `Saved\Balance\b6b6_on\`, §6.1 steps 3-4), take T from its `ending:` line (`CosmicBalanceCommandlet.cpp:3890`).
     - Every CSV and `_rows.csv` row with **Tick ≤ T** equals the `b6b6_base` copy on every column except `TickMicros`, `SaveBytes` and `SaveRawBytes` (N:597 "이하", N:659).
     - The on-run's T equals the base run's.
     - A difference at Tick ≤ T stops the commit (§6.2 item 7).
   - **ⓒ** Every log has 0 `CosmicBalance end check` and 0 `race check` lines (`CosmicBalanceCommandlet.cpp:1968`, :2025, :3575).
     - Note in N §5 that `race check` = 0 relies on wiring that [B-6b7]'s one-off probe tests (N:704-705). This commit does not probe it.

4. **Hand-off to [B-6b7]** (N:598): for each of the five runs, whether `race:` part 2 shows an AI T4 inside the horizon, and at what tick.

5. **Facts that may move:**
   - `STATUS.md:27` ('기준값': ruled T3 2,375, cards 16, starts 4, decisions 44, the crowning at 3,121 on `civ_7`; T2 at 1,175 stays);
   - `docs/40:25`;
   - the `Tools/run_checks.ps1:121-124` comment, if the crowning moved;
   - `docs/50:46` and CHANGELOG wait for [B-6b12].

6. **`docs/30`** is not regenerated (H:377). `make_config_reference.py --check` confirms it.

---

## 4. Bundle A (H §5.2, H:325-329)

### 4.1 Runs

Every run uses `-Rules=ProvisionalEndTick:0`, the home row, and the ticks given.

| Cell | Seeds | Ticks | Arguments |
|---|---|---|---|
| R | 1-30 | 20,000 | `-Answer=1 -Build=1 -Focus=1 -Edict=1 -Diplomacy=1 -TellAll=0` |
| input-less | 1-30 | 20,000 | none |
| wanderer | 1-60 | 6,000 | R + `-Abdicate=1200 -Wander=1` |

Each run is done on and with `-WithoutB6bData=1`. That is 240 bundle runs. The total is 255 commandlet processes: 240 + 5 ⓐ + 5 base (§0.8) + 5 on (§6.1 steps 3-4), not counting the runner.

### 4.2 Harness

- **Script:** `scratchpad\b6b6\bundleA.py`, modelled on `scratchpad\b6b3b_b\runs.py`.
  - `ThreadPoolExecutor` with **6 workers** (the B6a precedent). Use 8 only if free memory, checked first, covers 8 editor commandlets.
  - The 20,000-tick jobs go first.
- **Command:** `[UnrealEditor-Cmd.exe, <uproject>, "-run=CosmicBalance", "-Seed=s", "-Ticks=t", "-Out=<abs>/<cell>_<s>[_off].csv", *args, "-unattended", "-nosplash", "-AbsLog=<abs>/<cell>_<s>[_off].log"]`, with stdout and stderr sent to DEVNULL.
- **Folder:** `Saved\Balance\b6b_bundleA\` (about 160 MB).
- **Never overlap** Build.bat, `make_data_tables`, a mutation or another workflow. Start only when `tasklist` shows no UnrealEditor or UBT process.
  - Take sha1 of `Binaries/Win64/UnrealEditor-Cosmic_Ascension*.dll` and the three `DT_*.uasset` before and after the bundle. If any changed, discard the bundle.
- **Fail a run on any of:**
  - an exit code other than 0;
  - no `CosmicBalance: seed` line or no `race:` line;
  - an `end check` or `race check` line;
  - a `LogCosmic: Error`;
  - a missing `.csv`, `_rows.csv` or `_claims.csv`.

  Rerun a failed run once, then report it.
- **Switch controls** (the pair check alone cannot tell whether the switch acted):
  - every off log has exactly `rows taken out [feat_passage_by_ark project_ark project_binding crisis_species_extinction]` (`CosmicBalanceCommandlet.cpp:1596-1597`), and no on log has that line;
  - every off run shows 0 extinctions, 0 arks, 0 AI-step treaties and no AI T4;
  - over the on side: placements > 0, AI-step treaties > 0, and at least one pair differs after T.
- **Time:** about 10-12 minutes with 6 workers. AI µs per tick comes from the serial runner runs, or as an on/off difference within a pair.

### 4.3 The breakable check (per pair)

- T comes from the on-run's `ending:` line, and both sides report the same T.
- CSV and `_rows.csv` rows with **Tick ≤ T** are equal (same columns as ⓑ).
- Any mismatch stops the commit.

### 4.4 What is recorded (N §3 "묶음 A", N:622-624)

**Header line:** date, build, 255 processes, failures, folder, 0 check lines, the switch controls, the sha1 guard.

**Fields** come from `race:` (`CosmicBalanceCommandlet.cpp:3958-3992`), on side, with the off side where it means anything. Every share gets a Wilson 95% interval. Judge only at n ≥ 30, otherwise write '재지 않음 — 잰 N'.

- **② first values, all from the R runs' own `race:` lines.**
  - First and second AI T4 (part 2 prints `AIT4Tick[0]`/`[1]` under `-TellAll=0`; only the feat name is hidden, :544, and only one T3→T4 feat exists).
  - Ruled T3, T4 and T5 (part 3), as medians and quartiles per cell.
  - Brake-held AI-ticks and peoples (part 6).
  - T4 civs at the end and at most (part 5).

  Input-less runs feed only the model constants `AIOFF`/`FIRST`.
- **Ruled T3→T4 time, split into three parts:**
  - waiting for the strike (ruled T3 → first placement on the ruled people);
  - building the ark (placement → completion, from `_rows.csv`);
  - waiting for the treaty (completion → T4).
- **Extinction (part 8):**
  - placements on ruled and on AI peoples;
  - arks completed;
  - **completed within the countdown = `ArksCompleted` / (`ArksCompleted` + `Expired`)**;
  - `KeptByGrace` (with keys) and `ArksLostBeforeT4` reported apart (the latter counts completed arks whose system changed hands, :2275-2288);
  - ruled expiries (died, deposed, neither), ruler deaths;
  - peoples erased by cause (N:525).

  No output carries unpaid ticks or "expired with an ark standing" (:2364-2367, `hazards:` :3610). Write those '재지 않음'. Judge the 3/3 cost from the completed share together with the CSV's `Energy`/`Matter` of the struck people during its countdowns.
- **AI treaties (part 9):** treaties signed by the step and `AITreatyFirst`, AIs on T3+ holding one, ruled treaties that met an AI's ask. AI sieges, on against off (`conflict:` line; N ⓖ, N:571).
- **c review (N:527):** deleted AIs, and the ruled T4 tick against the model's 2,913 and 4,310 medians.
- **The grace's two paths (N:675 ④).**
  - Today, a grace empties the people's other extinction slots and voids the ark being built there; the paid value is lost.
  - The alternative never places a second extinction on the same people, which would change the spawn share.
  - Model: P(a second extinction on the same people within 250) ≈ 1−(1−c)^((k−1)·250), about 7% at c 1e-4 and k 4.
  - Measure grace events whose people held more than one extinction slot, where `_rows.csv` and the grace keys allow it; otherwise '재지 않음 — 잰 N'.
  - Also record grace spent on an AI's extinction against a ruled one or a supernova.
  - Record the verdict in N §5. The alternative changes a spawn share, so it is never this commit's change: [개발자] or [B-6b9].
- **Record only:** AI-stage µs per tick (B6a 10.4-10.7), and the B1 guard (wanderer `crowned at`, on against off).

### 4.5 Recomputing N §2 before [B-6b7] (H:325, H:329-330)

Write the result into N §2 ⓒ and ⓓ under a new paragraph headed "`[B-6b6]`의 다시 셈". N §2.3 and H §5.3 stay frozen (H:336).

- **Inputs** replace the constants of `scratchpad\b6b\review_measure\remodel.py`:
  - `L` (ruled T3 − first AI T3 over the 30 R runs);
  - `AIOFF` and `FIRST` (input-less runs);
  - k (CSV `Systems` at T3 and T4);
  - the measured T3→T4 duration in place of `wait()+150`.

  An erased people never reaches T4.
- **Direct resample (no model draws).** For each of the 30 R runs and each (P_bind, r) on the ladder 0.25, 0.3125, 0.375, 0.4375 and 0.5:
  - decide who binds first from that run's measured ruled T4 and AI T4 ticks;
  - a would-be binder that the run shows erased before T4 + P (ruled) or T4 + P/r (AI) does not bind.

  Report this beside the Monte Carlo (seed 7, 6,000 runs).
- **Values (recommended, counts as approved).** They are recorded in N §2 for [B-6b7] (H:393), not committed as data here.
  - Keep P_bind = 1,800 (H:429, N:552, DT_Megaprojects.json:67 - corrected by the lead from a typo 1,200), N = 1,200 and r = 0.375 if the recomputed share of the other order lies inside 15-35%. Otherwise move r the fewest ladder steps into the band, nearest 26%.
  - If the recomputed median fourth-rung season exceeds 3,600, lower P_bind first.
  - [개발자], with the numbers, only if no ladder value fits, or if both P_bind and N would have to drop below 1,200 (H:344).
- **t** = the median ruled T4 tick over the R runs that reach T4, rounded up to the next 100 (H:330).
  - Write n beside it, and also the censored median over all 30 runs (a run that never reaches T4 counts as +∞).
  - If n < 15, flag it in N §5.
  - If n = 0, t = 4,400 (the model's 4,310 rounded up), flagged in H §11.

### 4.6 Questions for the ultracode balance-review lens (each finding gets a rebut-verify)

1. **c is two-sided.** It punishes (deleted AIs; fourth- and fifth-rung peoples are struck again, H:421), and it is also the only gate to the fourth rung: the feat needs that people's own completed ark, an answer row is never built unprompted (`Sim/CosmicMegaprojects.cpp:372`), and completion is per system (`Events/CosmicEventSubsystem.cpp:486-489`).
   - Read the three-part T3→T4 split, the deleted-AI count, and the extinction share per 600-tick window.
   - **c stays 1e-4 in this commit** (H:338, H:403). The lens may only recommend a value for [B-6b9] or [개발자].
2. **The ark at 3/3.**
   - What share completes within the countdown?
   - Are expiries driven by cost (low Energy/Matter during the countdown) or by the 250 ticks?
   - This per-tick cost is the one value this commit may change. A change reruns the runner `-Data` and the whole bundle.
3. **The AI treaty step.**
   - How many AI non-aggressions are signed, and how much do AI-against-AI sieges and #129's transfers drop (on against off)?
   - Does the 50 Influence delay the AI's next treaty or another Influence cost (edicts, diplomacy)?
4. **The brake.**
   - Is the first AI T4 before ruled T4 in more than about 84% of R runs (the model's r = 1 share)?
   - Is T4 civs-most (part 5) ever above the home cap 2 for AI climbs? Is the `race check` count 0?
   - Report brake-held AI-ticks (part 6) separately.
5. **The recomputed values.** Are N, P_bind, r and t stable under the direct resample, erasures included?
6. **The grace's two paths** (§4.4), and how often an AI's extinction spends the board's one grace first.
7. **Wanderer runs.** Did #122's `FarAbove` refusals or extinction deletions change the crowning count, on against off?
8. **AI-stage µs per tick** (serial) against B6a's 10.4-10.7. Above +50% is a notice for bundle B.
9. **Anything else.** Only the ark's per-tick cost may move here. Everything else is [B-6b9] or [개발자].

---

## 5. Docs and records

**H §11.** One line under 400 bytes; check with `wc -c`. HEAD is 146,814 bytes.

> "- `[B-6b6]`(2026-11-xx): 데이터 ① — 절멸 행(c 1e-4)·`project_ark`(에너지·물질 3)·`feat_passage_by_ark`, 결정 5의 코드 죽음 핀. 다르게: 세계는 첫 셋째 칸 +1부터 갈릴 수 있다(절멸 또는 AI 조약 걸음 — §3.1은 추첨 표만); 상수는 `CosmicB6bRows`에; '방주' 지킴·'노아'. 세부 노트 §5."

That is 351 bytes. Add a `Feats.Reachable` clause only if it fits. Line drift and other outcomes go to N §5.

**N §5 `[B-6b6]` block** (form of N:700-709):
- 나눔: none.
- 코드: tests, comments and the header comment.
- 데이터: the three rows and their texts' source.
- 스키마: `ark_words`, '노아', the ark rule, two `row_not` rules, `forbidden` on six columns, the exact pins.
- 테스트: counts, and every §2.4 case.
- 변이: new, retaken and rerun counts (from the apply diff).
- 리뷰: counts.
- 기준값: old → new; ⓐ ⓑ ⓒ; each run's first mover; the [B-6b7] hand-off.
- 헤더와 다르게 하거나 더한 것:
  - ① the first mover;
  - ② no `CosmicRows::` constant;
  - ③ the ark cost 3/3;
  - ④ the two extra lists in the ark rule;
  - ⑤ the '방주' guard split between check_data and the test;
  - ⑥ the DeathPipeline copy fix;
  - ⑦ `Feats.Reachable`;
  - ⑧ markers dropped now;
  - ⑨ `race check` = 0 rests on wiring that [B-6b7] probes;
  - ⑩ the grace two-path verdict;
  - ⑪ the bundle metrics that are '재지 않음' (unpaid ticks).

**N §3 "묶음 A"**: §4.4. **N §2 ⓒ·ⓓ**: §4.5.

**`DECISIONS_LOG.md`.** One `[B-6b6]` line after [B-6b5]'s (form of :34-37). It covers:
- c 1e-4 and its basis, kept for [B-6b9];
- the ark cost basis;
- decision 5's code-death pin (closing :34's requirement);
- bundle A's headline numbers;
- the recomputed N, P_bind, r and t;
- the grace two-path verdict;
- the new baselines;
- review counts.

**Markers that become false now.** Drop or reword "[B-6b6]부터" where the row is now in code. Keep every [B-6b7]/[B-6b8] marker. Re-find each on [H''] by the paragraph's first words.
- `docs/10_DESIGN/crisis_and_transcendents.md`: :24, :40, **:67**, :115, :139, :175, :177, :266;
- `pillars.md`: :105, :108;
- `systems.md`: :65, :91, :229, :417;
- `docs/15_GLOSSARY.md:151`;
- `docs/20_ARCHITECTURE.md`: :109, :212, :289;
- `docs/21_DATA_MODEL_REFERENCE.md`: :629, :763, :973, :1518, :1520, :1585, :1658;
- `docs/40_DEV_GUIDE.md`:
  - :237: the ark-margin text becomes "the game row is held", and **add the code-death pin** to the `Data.Integrity` row;
  - :238: if `Feats.Reachable` changes;
  - :496;
- `docs/70:129` ([B-6b5]'s "`[B-6b6]`·`[B-6b7]`부터" becomes "`[B-6b7]`부터");
- wherever `docs/21` names `KillsTheRulerOf`'s readers (re-find; :1458-1463 region at HEAD), add `Data.Integrity`'s pin.

Everything else stays with [B-6b12] (H:380).

**`STATUS.md`:**
- '지금' and '다음 3작업': [B-6b6] is done; next are [B-6b7], [B-6b8] and the hand checks.
- '코드 사실': the test count, and DT_Hazards 6, DT_Megaprojects 7, DT_Feats 4.
- '기준값': old → new.
- The B6a hand checks (§6.2 item 1).

**`docs/CHANGELOG.md`:** the lead's work-log line. **`docs/50`**, **`docs/30`**: no change.

**Commit message.**
- **Title:**

  > `[B-6b6] 데이터 ① 셋째 → 넷째 칸 — 절멸 위기 행(c 1e-4)·방주 project_ark·업적 feat_passage_by_ark, 코드 죽음의 핀(결정 5), 방주 낱말의 지킴·'노아', 묶음 A, <moved lines>`

  `<moved lines>` is "기준값 다섯과 해시 판 둘이 옮았다" only if step 3 and step 7 show all of them moved. Otherwise name the lines that moved.
- **Body:**
  - an "옮은 기준값" block (old → new, the hash runs equal the commandlet's 3,000 lines), with each line's first mover;
  - ⓐ ⓑ ⓒ;
  - bundle A's verdict (processes, failures, check lines, the switch controls, the per-pair check);
  - mutation counts;
  - review counts;
  - the trailer `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.

Push to origin.

---

## 6. Order of work, speed rules, risks

### 6.1 Order (one lean ultracode workflow)

1. **Start state (§0):** clean build on [H''], base copies, `apply_before.txt`.
2. **Implementers, in parallel:**
   - I1, data: §1.1-1.3, §1.5, §1.6; `check_data` = 0; the Python tests;
   - I2, C++: §1.4, §2.1-2.4, and the patch files (not yet run).
3. **Build pass:** `run_checks.ps1 -Tier smoke -Data -UpdateBaseline`, editor closed. Record the three moved keys. Right away, copy `checks.csv`, `checks_ruler.csv` and `checks_wanderer.csv` (and their `_rows`/`_claims`) from `Saved\Balance` into `Saved\Balance\b6b6_on\`.
4. **On this build:**
   - the two 20,000-tick on-runs with the runner's exact arguments into `b6b6_on\` (absolute `-Out`, `-AbsLog`);
   - ⓐ (5 runs) and ⓑ;
   - bundle A (§4), then ⓒ over all logs;
   - the §4.5 recomputation;
   - the `Feats.Reachable` reading (§2.4).
5. **Review lenses**, each with a rebut-verify per finding:
   - L1: data, schema and theology (rows verbatim from N §1.5, the guard, the guardrails);
   - L2: determinism and baselines;
   - L3: tests and mutations;
   - L4: **ultracode balance** (§4.6);
   - L5: docs and records.
6. **Fix.** If a fix touches data or sim, repeat steps 3-4.
7. **The one final runner:** `-Tier full -Data -UpdateBaseline`. Its two in-process passes are the two consecutive test runs (N:709). Its five lines must equal the five in `b6b6_on\`.
8. **Mutations** (§2.5), serially. Then `apply_after.txt`, the README rows, and **a clean rebuild of both targets** so that [B-6b7] starts unmutated.
9. **Records** (§5), `check_docs`, commit and push.

### 6.2 Risks and open points (recommended = approved)

1. **B6a hand checks ①, ③ and ⑨.** H:46 asks for them before [B-6b6], and H:410 says "블록을 막지 않는다". Do not block on them. They can be rebuilt at any time in `git worktree add ..\CA_b6a 5455e56`, with evidence under `Saved\HandCheck\B6a_*`; write this in STATUS. **[개발자]** The hand checks themselves (`STATUS.md:30-32`).
2. **N §1.5 still empty and merged.md missing.** Stop (H:391).
3. **Ruled T4 below 30 in R.** t is still computed, with n and the censored median. The judgments wait for bundle B.
4. **`Feats.Reachable`** may fail on deletions. Use the §2.4 reading only.
5. **Ark cost 3/3.** Judged from the completed share and stockpiles. A change stays inside this commit and reruns steps 3-4.
6. **Mutation context churn** from the `forbidden` edits and :697. Attribution is by the apply diff only.
7. **A world move at or before T.** Something reads the new rows early. Stop and fix it here (H:403).
8. **Mutated or half-built binaries under a commandlet.** The §0.7 clean build, the bundle's sha1 guard, and the §6.1 step 8 rebuild cover this.

### 추가 이야기와 보완

- Keep `scratchpad\b6b6\bundleA.py` together with the per-pair check, the switch controls, the sha1 guard and the ⓐ key builder (which reuses `Remove-WrotePath`'s rule), so that [B-6b8] and [B-6b11] run the same check. Committing it under `Tools/` is optional and outside H §8; if it is committed, `docs/40` gets one line.
- Optional: a `CosmicTests::HazardsWithout(Original, Names)` helper, for the `QuietHazards` swap and future boards.
- `desolation_words` could later also cover the supernova lines (B5brief:1071). Leave that to the Phase 4 text audit.
- A measurement-only counter for "expired with the ark standing, and its progress" would make the ark-cost judgment direct. It belongs in the Tests module, bit-identical, with a `Sim.RaceMeasure` case. If wanted, [B-6b7] or [B-6b8] adds it before bundle B.

---

## Critique dispositions

### Completeness critique

1. **Folded.** Added `b6b6_data_extinction_expiry_effect` (rule 5 relaxed + `ChangeStability`, `schema.json:1239`, assets rebuilt) and dropped the "no asset-level mutation" line (§2.1, §2.5, §5). Verified: rule 5 at `schema.json:778-788` is the only stop.
2. **Folded.** The rerun set is now mechanical (§2.5). Verified at HEAD: 35 patches on `schema.json`, 17 on `CosmicTestHelpers.h`, 15 on `CosmicDataTests.cpp`, 2 on `DT_Hazards.json`, 1 on `DT_Megaprojects.json`.
3. **Folded.** The grace two-path comparison is in §4.4 and Q6, with the model and N §5 (N:675 ④ verified: "둘을 `[B-6b6]`의 c 미리 셈과 함께 견준다").
4. **Folded.** c is locked at 1e-4 (H:338, H:403); only the per-tick ark cost may move (§1.2, §4.6).
5. **Folded.** Unpaid and "ark standing" are '재지 않음' (`CosmicBalanceCommandlet.cpp:3976-3980`, :2364-2367). The cost is judged from the completed share and stockpiles. The counter is offered for later (§4.4, 추가 이야기).
6. **Folded.** Tick ≤ T in ⓑ and §4.3 (H:329, N:597).
7. **Folded.** New §2.4 row naming the treaty step and the extinction share; the fix is `WithoutB6bData` outermost (`CosmicTestHelpers.h:471-486`).
8. **Folded.** Exact pins (§1.6.5), `docs/21:1266` (§1.6.7), the `docs/40:237` code-death pin, and `docs/21`'s readers of `KillsTheRulerOf` (§5). The last is conditional, because HEAD has no explicit reader list (`docs/21:1458-1463`).
9. **Folded.** [개발자] only if both P_bind and N must drop below 1,200 (H:344); the values are recorded for [B-6b7] (§4.5).
10. **Folded.** Added `crisis_and_transcendents.md:67` (verified) and `docs/70:129` (B5brief:679).
11. **Folded, with a correction.** `DT_Megaprojects.json:54` is fixed, and `CosmicB6bRowNames.h:1-8` is reworded. The critique's `systems.md:415` is the realm-cap paragraph: `systems.md` has no `Feats.Reachable` at HEAD, so only `docs/40:238` changes unless a grep on [H''] finds one. C++ lines are re-derived in §0.4.
12. **Folded.** `Feats.Reachable` is decided from non-ruled CSV tiers (`CosmicAscensionTests.cpp:1772-1777`) or by running the raised assertion.
13. **Folded in part.** Added `b6b6_schema_ark_guard_projects_dropped` and `b6b6_schema_ark_words_off_lines`. The ST_CA side is covered by an injected `ST_CA.csv` case in the new method rather than a walk-skipping mutation, which the critique allowed.
14. **Folded.** Q4 asks about T4-most above cap 2 and `race check` = 0; brake-held ticks are reported separately (`CosmicBalanceCommandlet.cpp:2025`).
15. **Folded.** 6 workers, 8 only after a memory check.
16. **Folded as a note** (§3.2.3 ⓒ, N §5 ⑨). The optional probe is rejected: the `-FourthRungCap` switch lowers the brake as well, so a real probe needs an uncommitted code edit, and N:705 gives it to [B-6b7].

### Determinism critique

1. **Folded.** The share is `ArksCompleted`/(`ArksCompleted` + `Expired`); `KeptByGrace` and `ArksLostBeforeT4` are reported apart; unpaid is '재지 않음' (§4.4).
2. **Folded.** The R runs' own part 2 ticks are used (`CosmicBalanceCommandlet.cpp:3959-3961`, :544); input-less runs feed only `AIOFF`/`FIRST`.
3. **Folded.** "T+1 at the earliest, whichever comes first" (`Sim/CosmicDiplomacy.cpp:415-426`, :461-478). Per-run first movers are recorded, and the commit title is conditional (§3.1, §5).
4. **Folded.** Same as completeness 6.
5. **Folded.** Switch controls, including the exact `rows taken out` line (`CosmicBalanceCommandlet.cpp:1596-1597`, `Note` order verified), and the sha1 guard (§4.2).
6. **Folded.** The 20,000-tick on-runs and the copy of the smoke CSVs into `b6b6_on\` (§6.1 steps 3-4). Verified: the runner's relative `-Out` lands in `Saved/Balance` (`run_checks.ps1:85`, `CosmicBalanceCommandlet.cpp:1525`), and the 20,000 lines are full-tier only (:129-132).
7. **Folded.** Clean build before the base runs (§0.7) and a rebuild after the mutations (§6.1 step 8).
8. **Folded.** Q1 is two-sided, and the T3→T4 time is split into three parts (§4.4, §4.6).
9. **Folded.** Erasure before T4 + P (ruled) or T4 + P/r (AI) means no binding (§4.5).
10. **Folded.** The censored median and an n < 15 flag; t keeps H:330's definition.
11. **Folded (second option).** Half (b) stays a test; its patch is refiled as "`test_data_schema.py` only"; the runner's exit 1 blocks the commit; the walk skips `schema.json`. The checker-level "confined words" key is rejected as more code than this data commit needs.
12. **Folded.** The `why` now has the people as subject (B5brief:283, H:100).
13. **Folded.** Same as completeness 2.
14. **Folded.** Same as completeness 12.
15. **Folded.** The trimmed draft is 351 bytes (measured); drift and other outcomes go to N §5.
16. **Folded.** `:54`, DisplayName `:697` (a string that becomes an array), templates `:711-713`, `CosmicB6bRowNames.h:1`.
17. **Folded.** Q3 asks about delays to the next treaty and to other Influence costs.
18. **Folded.** 240 bundle runs + 5 ⓐ + 5 base + 5 on = 255 processes.

**Verified and kept from the determinism critique's "these hold":**
- the appended Draw share;
- no new draw point;
- the ⓐ keys (B5brief:964-970);
- the tripwire order (`CosmicTestHelpers.h:375-387`);
- `b6b2b_kills_ruler_other_tag` gives two rows.

The schema's claim to block every data mutation of the empty-expiry clause was true only for a single-file data patch, so completeness 1 stands.