# [B-6b8b] implementation brief: #132 guard share 0 → 0.5, with W recounted (decision 23)

Everything below was checked read-only at HEAD `cef8cc7`. "{H8B}" means the tree this commit starts on (the `[B-6b8]` commit). I ran one Python probe outside the project at HEAD: `C:\Users\rhwwl\AppData\Local\Temp\claude\c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension\37de2919-0c44-45d8-94cd-4dcf0860263e\scratchpad\b6b8b\w_recount.py`. It is a Python copy of `TheodicyProblems` (`Tests/CosmicDataTests.cpp:2477-2568`). At share 0 and at share 0.5 it gives exactly N ⓔ's numbers:
- claim: 300+100+100+50 = 550, 55 slots, 0.2629 stays, W 1.2445;
- plague: 300 ticks, 0.5455;
- at share 0.3 the claim's longest stand is 540.

Scope: one data value, one test change, one header comment, doc sentences, the closing of OPEN_QUESTIONS #132, and the baselines recorded again (or confirmed bit-identical). No new draw, no saved field, `SaveVersion` stays 1, and no data row is turned on (H `:403`).

**Most likely outcome.** The five lines and both hash runs stay bit-identical. Every runner log in `Saved/Logs/checks*.log` on the current tree prints `delay bought per tag []` and `guarded crises 0 by tag []`. Plan for both outcomes; §3 decides which applies.

---

## 0. Start state (on {H8B}, before any agent runs)

1. **Earlier commits.**
   - `git log --oneline -6` must show `[B-6b4]`, `[B-6b5]`, `[B-6b6]`, `[B-6b7]` and `[B-6b8]`, and `[B-6b9]` only if bundle B called for it. If `[B-6b8]` is not committed, stop.
   - `git status` must be clean apart from the user's known local changes.
   - No UnrealEditor or UBT process may be running (`tasklist`).
2. **Re-derive every line number below on {H8B}.** These HEAD anchors will drift:
   - `DT_Rules.json:27`
   - `Tests/CosmicDiplomacyTests.cpp:1406-1512` (the B6b block `:1474-1510`)
   - `CosmicDataTests.cpp:2477-2568`, `:2822`, `:2948-2967`
   - `Data/CosmicDefinitionRows.h:988-994`
   - `Sim/CosmicConflict.cpp:366-412`
   - `UI/CosmicSystemPanelWidget.cpp:1098` (`[B-6b4]` edits the panel)
   - `World/CosmicRulerPolicy.cpp:711-780`
   - docs: `docs/20:289`, `docs/21:1199/:1518`, `docs/40:237/:366`, `systems.md:78`, `docs/30:30`
   - `OPEN_QUESTIONS.md:101`
   - `STATUS.md:7/:27/:32/:38/:40`
   - H §11's last bullet (`:468` at HEAD, before `## 추가 이야기와 보완` at `:470`) and N §5's end
   - the last B6b line of `DECISIONS_LOG` (`:37` at HEAD; `:39` is the B6a heading)
3. **Confirm that nothing touched the knob in between.** Save `git grep -n FleetDelayCountdownShare cef8cc7` (34 hits) as `<scratchpad>\b6b8b\hits_head.txt` and the same grep on {H8B} as `hits_h8b.txt`, then diff them.
   - The HEAD readers that matter are:
     - `CosmicConflict.cpp` (`GuardDelayCapOf`);
     - the commandlet's `-WithoutB6bData` (`:1594/:1596`) and its `race:` AtCap, which calls `GuardDelayCapOf`;
     - `CosmicTestHelpers.h` (`WithoutB6bData`, `:363`);
     - `CosmicDataTests.cpp:2716/:2779`;
     - `CosmicDiplomacyTests.cpp:1480-1507`.
   - The other hits are comments (`CosmicConflict.h:151`, `CosmicBalanceCommandlet.h:124`, `CosmicTestHelpers.h:351`), mutation patches and README `:800`, `schema.json:153`, `test_data_schema.py:1493`, and docs.
   - **Stop only on a new hit in a non-comment `.cpp`/`.h` line.** New doc or patch hits only need re-reading.
   - `DT_Rules` must still hold `0.0`.
4. **Fill these placeholders** from the tree at the start:
   - `{B6b8_BASELINE}`: the result of `git show HEAD:Tools/baselines/seed42.json`, using its keys `summary_line`, `summary_line_20000`, `ruler_summary_line`, `ruler_summary_line_20000`, `wanderer_summary_line` and `last_recorded_in_changelog`. The file has no hash-run key. The hash runs are compared by the runner's `Compare-HashRun` against the commandlet's own lines (`run_checks.ps1:139-155`).
   - `{H_BYTES0}`: `wc -c` of H.
     - H §8's own limit is "150,000바이트 안".
     - This brief adds a **self-imposed margin of 149,000**.
     - H was 146,814 bytes at HEAD, and the five data and presentation commits add about 1.9 KB, so the margin can be under 400 bytes (§6).
   - `{NE_TEXT}`: N §2 ⓔ, verbatim from {H8B} (`:562-567` at HEAD). Earlier commits may have edited §2.
   - `{W_TABLE}`:
     - First check that `python -I <scratchpad>\b6b8b\w_recount.py Tools\data 0.0` on {H8B} gives the same claim line.
     - Then run `python -I <scratchpad>\b6b8b\w_recount.py Tools\data 0.5` on {H8B}.
     - It yields:
       - `{PLAGUE_BONUS_SUM}`: the plague's longest stand − 300; it must stay ≤ 380 per ⓔ;
       - `{CLAIM_BONUS}`: must stay 100;
       - `{CARD_DELAY_CLAIM}`: must stay 50;
       - `{SKIPPED_ROWS}`: at HEAD the skipped rows are awakening, pre-supernova and rebellion. Extinction is skipped unless some non-grace event resolves `Hazard.Extinction`. The ark is a megaproject, not an event.
     - The galaxy laws from `[B-6b7]` and `enc_witness` from `[B-6b8]` enter here.
     - If the claim line is not 550 / 0.2629, stop. An earlier commit changed W, and ⓔ's claim "W 그대로" needs re-reading before this commit.
   - `{PRED}`: the §3 prediction from {H8B}'s runner logs. It picks between the "moved" and "bit-identical" record variants of §6.
5. **Mutation apply state.** In a scratch copy of {H8B}, run `git apply --check` on every patch under `Tools/mutations/` and save the result as `<scratchpad>\b6b8b\apply_before.txt`.
6. **Clean build.** `git status` must be clean with no leftover mutation; earlier workflows end with mutation runs.

## 1. Data rows

No new rows and no texts. N §1.5 has nothing for this commit.

**`Tools/data/DT_Rules.json:27`** is the only data edit:
```json
		"FleetDelayCountdownShare": 0.5,
```
- Schema `schema.json:153` (`min 0, max 1`) and `test_data_schema.py:1493` already exist, so 0.5 passes `check_data`. No schema change.
- The runner's `-Data` rebuilds the assets. Commit `DT_Rules.uasset` and `generated.sha1`.

## 2. Code, tests, constants

**No C++ logic change.**
- `GuardDelayCapOf` (`Sim/CosmicConflict.cpp:398-408`): 0.5 × 150 = 75 exactly in float.
- `FleetDelayOf` (`:366-396`) already takes min(own delay, row cap). It returns 0 for Capital-rule rows (the rebellion) at `:369`.
- `Place` (`Sim/CosmicHazards.cpp:219`) is the only path into the world.

The panel reads only `GuardDelayTicksOf` (`UI/CosmicSystemPanelWidget.cpp:1098` at HEAD). Its keys `Guard.Delay` and `Guard.DelayTip` (`ST_CA.csv:339-340`) carry no number, so the panel does not change.

**Which placements change.**
- Only a row with CountdownTicks < 200 has a cap below 100. In the data that is `crisis_ai_awakening` alone (150, `DT_Hazards.json:7`).
- Plague is 200, claim and pre-supernova 300, extinction 250 (cap min(100, 125) = 100). The rebellion gets 0 either way.
- The awakening row is reached only when the owner's `FleetTargetSystemID` is that system and 50 × FleetPower > 75, i.e. **FleetPower > 1.5**.

Two sources cannot reach it:
- The first crisis (tick 150, `Hazard.AIAwakening` on every capital, `GalaxySimulationSubsystem.cpp:2451-2474`) goes through `Place`. At that point the ruled people is still on the first rung, and `FleetPerSettledByTier[0]` is 0 (`DT_Rules.json:89`).
- AI peoples drop a guard on their own system in the AI stage (`:3005-3011`) and clear the target after a siege ends (`:2217`, `:2274`).

One edge to keep in mind:
- After an abdication, death or deposition (tenth stage), the former ruled people keeps its guard. No role path clears a guard on its own system; `PlayerRoleSubsystem.cpp:1189` clears only a besieging target.
- That guard lasts until its own AI-stage clear in the next tick, which runs after that tick's crisis stage.
- So an awakening placed in that one tick is capped for a non-ruled owner. The commandlet's `conflict:` "delay bought per tag" counts it, because it counts every owner. `pressure:` does not, because it counts only `bIsPlayerRuled` owners (`CosmicBalanceCommandlet.cpp:2676-2677`).

**Header comment** `Data/CosmicDefinitionRows.h:992` (ASCII):
```
	// 0..1 (schema). The C++ default 0 is off; DT_Rules holds 0.5 from [B-6b8b] (decision 23): the awakening's cap
	// 100 -> 75 (0.5 x 150); plague, pre-supernova, claim and extinction keep FleetDelayCap's 100; the rebellion (Capital rule) gains none.
```
This is a header edit, so build with the editor closed; the runner's `Build.bat` covers it. Mirror it in Korean in `docs/21:1199` (§6).

**`Sim.FleetDelay`** (`Tests/CosmicDiplomacyTests.cpp`, HEAD line numbers):
- File comment `:1406-1410`: add "B6b [B-6b8b] (decision 23): the game's share is 0.5 - the game's awakening row (150) gains 75 from fleet 3, 100 at the share's 0."
- `:1480` would fail at 0.5. Replace it with:
  `TestEqual(TEXT("The game's share is 0.5 ([B-6b8b], decision 23)"), Game.FleetDelayCountdownShare, 0.5f);`
- `:1481` keeps its value (100), but its label becomes false. Relabel it to:
  `TEXT("so the plague's 200 keeps FleetDelayCap's 100 (0.5 x 200 = 100)")`
- Insert right after `const FCosmicDataTables Kept = Sim->GetTables();` (`:1483`) and before `auto WithShare`. `Guard.FleetPower` is 3 and the target is system 1.
  ```cpp
  		// B6b [B-6b8b] (decision 23): the game's awakening row on the board - 150 + the game's share of it, 75, not FleetDelayCap's 100.
  		const FName Awake(TEXT("crisis_test_awake"));
  		const FHazardDef* GameAwakening = Original.FindHazardByTag(Tag(TEXT("Hazard.AIAwakening")));
  		if (TestNotNull(TEXT("(the game's awakening row)"), GameAwakening))
  		{
  			TestEqual(TEXT("(the game's awakening counts 150)"), GameAwakening->CountdownTicks, 150);
  			FHazardDef Row = *GameAwakening;
  			Row.SpawnChancePerTick = 0.f;
  			UDataTable* Copy = CosmicTests::CopyWithout<FHazardDef>(Kept.Hazards, {});
  			Copy->AddRow(Awake, Row);
  			FCosmicDataTables WithAwake = Kept;
  			WithAwake.Hazards = Copy;
  			Sim->SetTables(WithAwake);
  			TestEqual(TEXT("Fleet 3 adds the awakening the game's share of its 150, 75: 225"), PlaceOn(1, Awake), 225);
  			FCosmicRulesDef Zero = Game;
  			Zero.FleetDelayCountdownShare = 0.f;
  			Sim->SetTables(CosmicTests::WithRules(WithAwake, Zero));
  			TestEqual(TEXT("At a share of 0 the row's cap is FleetDelayCap's 100"), CosmicConflict::GuardDelayCapOf(Zero, 150), 100);
  			TestEqual(TEXT("so fleet 3 adds the awakening 100: 250, the bound before [B-6b8b]"), PlaceOn(1, Awake), 250);
  			Sim->SetTables(Kept);
  		}
  ```
  Why this shape:
  - It copies the table instead of calling `AddRow` on the board's own table (`CopyWithout`, `Tests/CosmicTestHelpers.h:222`; `FindHazardByTag`, `CosmicDataTables.h:97`).
  - `Place` does not check spawn conditions. The board's own `Endless` row also uses this tag (`:112`), but it comes first, so tag lookups are unchanged. There is no ticking.
  - `LayBoard` copies the game's rules (`:134-154`), so the 0.5 pin sees the game's value.
  - The fleet-1 case from the map is dropped. It repeats the plague's fleet-1 case (250), and no mutation would break it.
  - The share-0 pair restores a guard on the "share 0 = FleetDelayCap" branch. Once the game's share is 0.5, nothing else in the suite checks that branch.
- All older assertions hold at 0.5: 200 / 250 / 300 / 200 / 200 / 6 / 300 / 325 / 50 / MAX_int32 / 200, and every `WithShare` case.

**Tests that do not change (verified at HEAD):**
- `Data.Integrity`'s theodicy check:
  - The game run at `:2822` reads `GuardDelayCapOf` (`:2529`). The claim's cap stays 100, so 550 holds.
  - The board cases `:2948-2967` are unchanged: 1.2 passes, 549 is refused and 550 passes, 1.3 fails.
- `:2716` and `:2779`.
- Everything built on `CosmicTests::WithoutB6bData`, which zeroes the share (`CosmicTestHelpers.h:363`): `CosmicEndingTests.cpp:158`, `CosmicKnowingTests.cpp:1875`, `CosmicSuccessionTests.cpp:631`.
- `CosmicDiplomacyUITests.cpp:701`, which reads `GuardDelayTicksOf`.

**Tests whose meaning could change.** These run the ruler policy with `-Diplomacy` on game data:
- `Sim.WandererLoop` (`CosmicWandererTests.cpp:1370-1386`; seed 4, 6,000 ticks, return path before 6,000).
- `CosmicPressureCardTests.cpp:713-815`.
- `Sim.HashRun` (`CosmicHashRunTests.cpp:201/228`). It compares two paths, not pinned numbers, so it is safe.

Rule for any other failure in the full pass:
1. Apply `b6b8b_data_share_off` (§5), rebuild the assets, and rerun that one test.
2. If it passes at share 0, add an `AddInfo` that names the guarded awakening (tick, FleetPower). Pin it again with a label citing `[B-6b8b]` and record it in N §5.
3. If it does not pass at share 0, stop: something besides the knob moved.

**Constants:** none. No `CosmicRows::` entry is needed; tests already name `crisis_ai_awakening` by literal, as in `CosmicDeathTests.cpp:550`.

## 3. Determinism and baselines

**Where the policy guards (an addition to H §5.2 `:334`).** H `:334` is correct: the knob reaches only awakenings on systems the policy guarded, and the parenthesis "R은 `-Guard=0`" only says that R does not run B4's `-Guard=1`. Two things are added:
- **The citation is stale.** At HEAD, `World/CosmicRulerPolicy.cpp:686-708` is the tail of ActEdicts. `ActDiplomacy` starts at `:711`, its role gate is at `:719`, and step (1), the guard of a besieged own system under `-Diplomacy=1 -Guard=0`, runs `:733-780`.
- **The wanderer run guards outside the first reign as well.** The gate checks only `Role == Ruler`, and the commandlet calls `Act` at every boundary (`CosmicBalanceCommandlet.cpp:2835`). So the run can guard before `-Abdicate=1200` and again after a crowning or return.
- Also add the one-tick tail after a reign ends (§2).
- Record these in N §5 and in the H §11 line.

**Prediction from {H8B}'s logs (no new runs).** For each of the five runs, read the runner's `checks_*.log`:
- `CosmicBalance conflict:` "delay bought per tag [...]" (`:3700-3715`; every owner, Delay > 0). This list decides the prediction.
- `pressure:` "guarded crises … by tag" (`:3819-3821`; the ruled people only). Use it for cross-reading.

Let the share-0 entry be `Hazard.AIAwakening=<sum>(<count>)`:
- Absent, or sum ≤ 75: the run is predicted bit-identical.
- sum > 75 × count: the run certainly moves.
- Otherwise: the run may move.

At HEAD all five lists are empty. Input-less runs must have no entry. Record the result as `{PRED}`.

**Breakable checks (on the new build):**
- **(a)** The input-less keys `summary_line` and `summary_line_20000` equal `{B6b8_BASELINE}` letter for letter, with the `wrote …` tail removed (`run_checks.ps1:76`). The runner's two steps `hash run: editor -game vs commandlet` and `… policy vs commandlet` must pass.
- **(b)** Five commandlet runs with the runner's exact arguments (`run_checks.ps1:116-132`; the runner passes no `-Rules`) plus `-Rules=FleetDelayCountdownShare:0` reproduce `{B6b8_BASELINE}` letter for letter.
  - The override is `OverrideRules` (`CosmicBalanceCommandlet.cpp:1365-1390`). Each log must show `CosmicBalance: rules FleetDelayCountdownShare = 0` and no `-Rules cannot set`.
  - Use an absolute `-Out` into `Saved\Balance\b6b8b_share0\` and each run's own `-AbsLog`.
  - This proves the knob is the only change.
- **(c)** Optional (five runs): `-WithoutB6bData=1` still prints the `[B-6b5]` lines, because the switch zeroes the share. These are the lines quoted in the `[B-6b6]` brief §3.2 ⓐ, as carried in seed42.json's note.
- **(d)** A two-sided rule for each of the five pairs (new build vs its (b) twin). It reads the (b) twin's `conflict:` `Hazard.AIAwakening=<sum>(<count>)`:
  - (i) A pair that differs (any CSV or `_rows.csv` cell other than `TickMicros`, `SaveBytes` and `SaveRawBytes`, or either hash) must have the tag with sum > 75. A move without it means stop.
  - (ii) A pair whose twin has sum > 75 × count must differ.
  - (iii) A pair whose twin lacks the tag, or has sum ≤ 75, must be identical over the whole CSV and `_rows`, minus the three timing columns.
  - For each moved run, record `{FIRST_MOVED_TICK_<run>}` = the first differing sample. This is a record, not a check.
- **(e)** 0 `end check` and 0 `race check` lines in every log.

**Recording the baselines again.** The final runner `-Tier full -Data -UpdateBaseline` always rewrites `verified_at`, and rewrites the five lines when they moved.
- **If any run moved:**
  - Fill `{MOVED_RUNS}` and `{NEW_HASHES}` from the runner's output.
  - Edit `last_recorded_in_changelog` by hand (UTF-8, no BOM):
    - `task`: "B6b B-6b8b (#132: FleetDelayCountdownShare 0 -> 0.5, decision 23 - only a guarded awakening's cap moves, 100 -> 75)";
    - the hashes;
    - `note`: the reason and, for each moved run, its first moved tick. Keep "Older values: …".
  - The policy hash run moves exactly when ruler 3,000 moves (N §2.2 `:600`).
- **If nothing moved (the expected case):**
  - `last_recorded_in_changelog` stays as it is; only `verified_at` changes.
  - The records use the "bit-identical" variants (§6).
  - Check (b) and bundle K (§4) still run. Together with `Sim.FleetDelay`, they are the commit's only evidence about the world.

## 4. Measurement: bundle K (mandatory; runs while the review lenses read)

H §8 asks for no bundle. Bundle K is still mandatory: if the five lines do not move, it is the only measured evidence that the knob moves only guarded awakenings. It also gives counts for N §5 and for the bundle C hand-off (N §2.3 `:613`).

**Harness.** Copy `<scratchpad>\b6b6\bundleA.py` to `<scratchpad>\b6b8b\bundleK.py`:
- 6 workers;
- a sha1 guard on the module DLL and `DT_Rules.uasset`;
- the same fail rules.
- `bundleA.py:47` already passes `COMMON_ARGS = ["-Rules=ProvisionalEndTick:0"]`. `FParse::Value(…"Rules="…)` (`CosmicBalanceCommandlet.cpp:1482`) reads only the first `-Rules=`, so the twin's argument must be **one** `-Rules=ProvisionalEndTick:0;FleetDelayCountdownShare:0`, never a second `-Rules=`.
- The switch control becomes: every twin log has both `CosmicBalance: rules ProvisionalEndTick = 0` and `CosmicBalance: rules FleetDelayCountdownShare = 0`, and no `-Rules cannot set`. Every on-side log has the first and not the second.

**Runs.** Each run has a share-0 twin, all in `Saved\Balance\b6b8b_bundleK\`:
- R: 60 × 20,000 (the runner's ruler arguments, seeds 1-60);
- wanderer: 60 × 6,000 (the runner's wanderer arguments);
- input-less: 30 × 20,000.

That is 300 runs, about 12-15 minutes. Never run them alongside `Build.bat`, `make_data_tables` or a mutation.

**Judgments, written before the run:**
- **K1:** every input-less pair is identical (the whole CSV and `_rows` minus the three timing columns, plus both hashes), unless that twin's `conflict:` lists `Hazard.AIAwakening` with sum > 75. In that case apply risk 2. Any other difference means stop.
- **K2:** the two-sided rule of §3 (d)(i)-(iii) on every R and wanderer pair, read from each twin's `conflict:` per-tag list. That list counts every owner, so it also covers the one-tick tail after a reign ends.
- **K3 (record only):**
  - `{K_MOVED_R}/60` and `{K_MOVED_W}/60` moved pairs;
  - the awakening's `atcap/all` from `race:` (`:2691-2696`) at 0.5, with the cap now 75;
  - the ends of guarded awakenings (`pressure:` answered / expired / edict);
  - the twins' total `Hazard.AIAwakening` count.
  - The awakening's delay share is at most 75/150 = 50% by construction (N ⓔ), so a band test would be empty.
  - Bundle C reads the 48-50% expectation in the trial cell. If the share here is below 10% of the guarded awakenings' countdown, record "the guard lost its fleet" for bundle C.
  - If every twin's list is empty, record "K: 지킨 각성 0 — 노브가 닿는 판 없음". The judgment for bundle C stays with C.

## 5. Mutations

**New patches.** Each must fail before it gets a README row. Add both data names to the data-patch list at README `:5`.

| Patch | Edit | Breaks |
|---|---|---|
| `B6b/b6b8b_data_share_off.patch` | DT_Rules `0.5` → `0.0` (rebuild the assets after apply and after revert) | `Sim.FleetDelay` (the 0.5 pin; 225 becomes 250) |
| `B6b/b6b8b_data_share_third.patch` | `0.5` → `0.3` (rebuild the assets) | `Data.Integrity`: the claim's longest stand becomes 300+90+100+50 = 540 (probe), so "a throne lock of 549" reports nothing and its `Expect` fails. This shows that the W pin reads the game's share. `Sim.FleetDelay` breaks too. |
| `B6b/b6b8b_share_zero_is_a_share.patch` | `CosmicConflict.cpp:403` (HEAD; `:402` is the comment) `if (!(Share > 0.f))` → `if (Share < 0.f)` | `Sim.FleetDelay`: the new share-0 pair (cap 0, so 150 instead of 250). NaN < 0 is false, NaN × 150 = NaN, and `NaN < Cap` is false, so the result is the cap of 100. The NaN and below-0 cases therefore still pass, and the patch is specific to share 0. |

**Old patches to rerun.** This is the narrow reading of the standing rule (recommended = approved): rerun a patch if it patches a file this commit edits, or if its filter test is one this commit edits. Re-derive the set on {H8B} mechanically:
- `git grep -l 'b/Tools/data/DT_Rules.json' -- Tools/mutations`. At HEAD: `b6b2a_data_laws_knob_four`, `b6b2c_data_ark_margin_half` and `b4d2_data_resist_factor`.
- The README rows whose Breaks column is `Sim.FleetDelay`. There are 6 at HEAD: `b3b_delay_on_rebellion`, `_unguarded`, `_ignores_gap`, `_outside_cut`, `b6b2c_guard_cap_ignored` and `b6b2c_guard_cap_no_fleet_cap`.
- Any `[B-6b6]`-`[B-6b8]` additions to either set.

No patch touches `CosmicDiplomacyTests.cpp` (0 at HEAD).

Expected results:
- `b6b2c_guard_cap_ignored` now also breaks the new 225, since 150 + 100 = 250.
- `b6b2c_guard_cap_no_fleet_cap` (the share's product alone) now also breaks **`Data.Integrity` (the game theodicy)**. Under the game's 0.5 the claim's cap becomes 150, the longest stand 600, 60 slots, and 0.976^60 ≈ 0.233 < 0.25. The 549 board case then reports 600, not 550 (`CosmicDataTests.cpp:2957-2962`). Confirm with `w_recount.py` before the run, and add "also `Data.Integrity` (game theodicy)" to its README row (`:791` at HEAD).
- `b4d2_data_resist_factor` has not applied since `[B-6b2a]`. Its context has `"ResistFleetFactor": 0.5` without a comma, while HEAD `:102` has one. This commit did not break it. Mark its README row (`:398` at HEAD) "(no longer applies since [B-6b2a] — not retaken; [B-6b12]'s whole-B6b rerun decides)" and note it in N §5.

**Retakes.** Retake every patch that goes from "applies" in `apply_before.txt` to "does not apply" in `apply_after.txt`, under a `b6b8b_` name. None is expected: the HEAD DT_Rules hunks are `:13-19` and `:97-103`, and the value edit at `:27` shifts no line.

**Running them.**
- Run them serially, after the final runner, never alongside a commandlet.
- Then save `apply_after.txt`.
- Then **rebuild both targets and the assets clean**, so that `[B-6b9]`/`[B-6b10]` start unmutated.

## 6. Docs and records

**docs/30.** Run `python -I Tools/make_config_reference.py`. `docs/30_CONFIG_REFERENCE.md:30` changes from `0.0` to `0.5`; the readers stay `Sim/CosmicConflict.cpp`. The runner's `--check` verifies it.

**Sentences this commit makes false** (fix them in the same commit; re-derive each on {H8B}):
- `docs/20_ARCHITECTURE.md:289`: "(B6b; 몫 `FleetDelayCountdownShare`는 `[B-6b8b]` 전까지 0 — 지금 셈)" → "(B6b; 몫 `FleetDelayCountdownShare` 0.5 — `[B-6b8b]`부터: 각성 150의 상한 75, 역병·초신성 전조·주장·절멸은 `FleetDelayCap` 100, 반란은 0)".
- `docs/21_DATA_MODEL_REFERENCE.md:1199`: "0 이하·수가 아니면 FleetDelayCap만(지금 셈). 장치 값 0(몫 없음 — 비트 그대로), 0.5는 `[B-6b8b]`(그 커밋이 W를 다시 셈한다)" → "0 이하·수가 아니면 FleetDelayCap만(B6b 전의 셈). C++ 기본 0(몫 없음), `DT_Rules` 0.5(`[B-6b8b]`부터 — 각성의 상한 100 → 75; 역병·초신성 전조·주장·절멸은 100, 반란은 0; W 1.2445·가장 긴 머묾 550 그대로)".
- `docs/21:1518`: "몫 0이면 지금 값" → "몫 0이면 B6b 전의 값".
- `docs/40_DEV_GUIDE.md:237`:
  - "(몫 0이면 `FleetDelayCap` — 지금 값)" → "(몫 0이면 `FleetDelayCap`; 게임의 몫은 `[B-6b8b]`부터 0.5 — 주장 300의 상한은 그대로 100)";
  - in the same row's longest-stand formula, "지킴의 `FleetDelayCap`" → "지킴의 `GuardDelayCapOf`".
- `docs/40_DEV_GUIDE.md:366` (the `Sim.FleetDelay` row):
  - "게임의 몫 0이면 `FleetDelayCap`(100, 지금 값)" → "몫 0이면 `FleetDelayCap`(100, B6b 전의 값)";
  - append "`[B-6b8b]`(결정 23): 게임의 몫 0.5의 핀(변이 `b6b8b_data_share_off`), 게임의 각성 행(150)을 판에 두면 함대 3이 75를 더해 225, 몫 0이면 250(변이 `b6b8b_share_zero_is_a_share`)".
- `docs/10_DESIGN/systems.md:78`: "(상한 100 — `FleetDelayPerPower`·`FleetDelayCap`;" → "(상한 100, 그 위기 행 카운트다운의 0.5까지 — 각성은 75; `FleetDelayPerPower`·`FleetDelayCap`·`FleetDelayCountdownShare`;". This file is outside N §1.2's list, but the commit makes the sentence false.
- **`OPEN_QUESTIONS.md:101` (the #132 row): delete it.** This follows N §0.2 `:195` ("**닫는다** … 결정 23 — `[B-6b8b]`") and CLAUDE.md ("결정이 바뀌면 … OPEN_QUESTIONS의 행을 지운다"), and the commit makes the row's "위 끝 66.7%" false.
  - The closure goes into the DECISIONS_LOG line below.
  - The drain side, as N §0.2 asks, gets one B7 line: "영향권 안 소모(#132에서 넘어옴 — 넷째 칸 이상에서 일어나지 않고 AI는 지키지 않는다; B7이 잰다)". Put it in docs/50's B7 entry. At HEAD, B7's work is named inside `docs/50_ROADMAP.md:12`; re-derive the place on {H8B}. If no B7 entry exists there, the DECISIONS_LOG line's "B7로" carries it and `[B-6b12]` places it.
  - The row's stale `CosmicDataTests.cpp:2234` citation goes with the row.
  - The #142 row (`:108` at HEAD) only cites "#132의 셈" for the 550, which stays true. Leave it.
- `STATUS.md:7`: "지킴 상한 `CosmicConflict::GuardDelayCapOf`(몫 `FleetDelayCountdownShare` 0)" → "(몫 `FleetDelayCountdownShare` 0.5 — `[B-6b8b]`부터, 각성의 상한 75)".
- `STATUS.md:38`: after "(위 끝은 `FleetDelayCap` 100 / 150 = 66.7%)", add "— #132는 `[B-6b8b]`가 닫았다(몫 0.5, 위 끝 75/150 = 50%)".
- `STATUS.md:40`: "#131·#132는 위의 2" → "#131은 위의 2(#132는 `[B-6b8b]`가 닫았다)".
- Unchanged and still true: `docs/21:1264/:1463/:1516-1517`, `crisis_and_transcendents.md:108/:113`, `docs/70:202`, `docs/20:212/:320`, `STATUS.md:8` (B6a history), and the B4/B6a history in `docs/50:12` and `DECISIONS_LOG`.
- Left to `[B-6b12]` per N §1.2: `docs/15_GLOSSARY.md:89` and the CHANGELOG. Say so in STATUS.

**H §11.** Insert one line after the last §11 bullet (`:468` at HEAD), before `## 추가 이야기와 보완`.

Moved variant:
> `- \`[B-6b8b]\`({DATE}): #132 몫 0.5(결정 23) — 각성의 상한만 100 → 75, W 1.2445·가장 긴 머묾 550·잠금 600 그대로; {MOVED_RUNS}. 더한 것: 정책은 재즉위·복귀 뒤에도, 양위·죽음 뒤 한 틱에도 제 성계의 지킴이 남는다; §5.2의 인용 \`:686-720\` → \`ActDiplomacy\` (1) \`:733-780\`. 세부 노트 §5.`

Bit-identical variant: the same line with "{MOVED_RUNS}" replaced by "다섯 줄과 해시 판 둘 비트 그대로".

Measure the line with `wc -c`. If `{H_BYTES0}` + its size ≥ 149,000, cut it to "- `[B-6b8b]`({DATE}): #132 몫 0.5(결정 23) — 각성만 75, W 그대로. 세부 노트 §5." (about 110 bytes) and move the rest to N §5.

**N §5 `[B-6b8b]` block** (in the form of the `[B-6b3b]` block):
- **나눔**: none.
- **데이터**: the one value.
- **코드**: the header comment only.
- **테스트**: the pin, the relabel, the awakening pair.
- **변이**: 3 new, the reruns with the expected double breaks, `b4d2` already dead.
- **리뷰**: counts.
- **기준값**: old → new, or "비트 그대로"; `{MOVED_RUNS}`; (a)-(e); each moved run's `{FIRST_MOVED_TICK}`.
- **묶음 K**: K1-K3.
- **W 다시 셈**: `{W_TABLE}`.
- **헤더와 다르게 하거나 더한 것**:
  - ① The H `:334` citation `:686-720` → `ActDiplomacy` step (1) `:733-780`. The wanderer guards after a crowning or return, and the former ruled people's own-system guard lasts one tick past the end of a reign.
  - ② The fleet-1 case is dropped and the share-0 pair is added.
  - ③ Stale citations. Re-derive each on {H8B}:
    - ⓔ: `DT_Rules.json:37/:38/:18` → `:39/:40/:19`;
    - ⓔ: `CosmicConflict.cpp:395-405` → `GuardDelayTicksOf` `:411-`;
    - ⓔ: `CosmicDataTests.cpp:2620-2622` → the 549/550 case (`:2957-2963`);
    - ⓔ: `:2288-2293` → the edict-bonus loop (`:2530-2535`);
    - H §2.10: `GuardDelayTicksOf` `:395-405` → `:411-`;
    - H §2.10: panel `:1084` → `UI/CosmicSystemPanelWidget.cpp:1098`;
    - H §2.10: `FleetDelayOf` `:366-392` → `:366-396`.
  - ④ `DECISIONS_LOG`'s B6b block ends at `:37`, not `:39`.
  - ⑤ #132 is closed here (N §0.2 `:195`), not in `[B-6b12]`.

**DECISIONS_LOG.** Add after the last B6b line:

> "- `[B-6b8b]`({DATE} — 결정 23의 값, 헤더 §2.10, #132 닫음): 지킴의 지연 상한의 몫 `FleetDelayCountdownShare` 0 → 0.5. 상한 = min(`FleetDelayCap` 100, 0.5 × 행의 카운트다운) — 각성 150의 상한만 100 → 75로 내려가고 역병·초신성 전조·주장·절멸은 100 그대로(반란은 0). 신정론의 W 다시 셈: 주장의 가장 긴 머묾 550(300 + 100 + 철야 100 + 카드 50), 슬롯 55, 남는 몫 0.2629 ≥ 0.25, W ≤ 1.2445, 왕좌 잠금 600 그대로 {W_TABLE의 한 줄}. 영향권 안 소모의 몫은 '넷째 칸 이상에서 일어나지 않고 AI는 지키지 않는다'로 닫고 B7로. 입력 없는 판은 그대로(AI는 지키지 않는다); 정책 판은 공성 받는 제 성계를 지키는 동안(그리고 양위·죽음 뒤 그 백성의 AI 단계 전 한 틱) 선 각성만 옮는다 — {MOVED_RUNS}, 새 해시 {NEW_HASHES} | 다섯 줄과 해시 판 둘 비트 그대로(시드 42의 판은 지킨 각성이 없다; 묶음 K {K_MOVED_R}/60·{K_MOVED_W}/60)."

Use the side of "|" that applies.

**STATUS.**
- `:27`: the new baseline facts and hashes, or "`[B-6b8b]`도 비트 그대로(시드 42의 판에 지킨 각성이 없다)".
- `:32`: `[B-6b8b]` done, with the `[B-6b12]` leftovers (glossary `:89`, CHANGELOG).
- `:7`, `:38` and `:40` as above.

**Commit message:**
- Title: `[B-6b8b] 지킴의 지연 상한 — 몫 0.5(결정 23·#132 닫음), 각성의 상한만 100 → 75, W 1.2445 그대로, Sim.FleetDelay에 각성 225, {MOVED_RUNS 요약 | 비트 그대로}`.
- Body, in order:
  1. `{NE_TEXT}` **verbatim**;
  2. the line "줄 번호(지금): …" with ③'s corrections (the quote itself is not edited);
  3. `{W_TABLE}`;
  4. the five lines old → new, or "비트 그대로";
  5. bundle K's K1-K3;
  6. the mutations;
  7. `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.

**Theology.** No text is added. The doc sentences above use no verdict word and no number of people.

## 7. Order of work, parallelism, risks

**Order** (one lean ultracode workflow, effort high):
1. **§0:** the checks, the hit-list diff, `apply_before.txt`, the `{W_TABLE}` probe and `{PRED}`. The probe is Python only and can run at any time.
2. **Implementers, in parallel:**
   - **I1, data and docs:** `DT_Rules.json`, the comment in `CosmicDefinitionRows.h`, the doc sentences, the OPEN_QUESTIONS row and the B7 line, STATUS `:7/:38/:40`, and the `docs/30` regeneration. `python -I Tools/check_data.py` must report 0.
   - **I2, tests and patches:** `Sim.FleetDelay`, plus the three patch files (written but not run).
3. **Build pass:** `run_checks.ps1 -Tier smoke -Data -UpdateBaseline`, with the editor closed. Diff `seed42.json` against {H8B}: the input-less `summary_line` must be unchanged (the first half of check (a)).
4. **In parallel:**
   - **Machine:** check (b) (5 runs), the optional (c), then bundle K (§4), then (d) and (e) over all logs. These share one machine queue, so run them in sequence on it.
   - **Lenses** (read-only; each finding gets a rebut-verify):
     - L1, determinism and baselines: the H `:334` addition and the reign tail, (a)-(e), K1 and K2.
     - L2, tests and mutations: the label truth, the new assertions vs the three patches, the rerun set and the expected double breaks.
     - L3, docs and records: the false sentences, the #132 closure, the byte budget, the ⓔ quote, theology.
5. **Fix.** If a fix touches data or the sim, repeat steps 3-4.
6. **The one final runner:** `-Tier full -Data -UpdateBaseline`. Its two in-process passes are the two consecutive test runs. Its five lines must equal the build-pass lines and the on-side lines of bundle K.
7. **Mutations** (§5), serially.
   - Draft the records (§6) in parallel, both variants; they touch only docs.
   - Then `apply_after.txt`, the README rows, and a clean rebuild of both targets and the assets.
8. `python -I Tools/check_docs.py`, `wc -c` on H, commit, push.

**Risks and resolutions** (recommended = approved):
1. **Another test fails in the full pass.** Use the §2 share-off probe. If it is explained, pin it again with a label. If it is not explained, stop.
2. **An input-less run moves** (K1 or (a)).
   - `FleetDelayOf` reads any owner's target. An AI could keep its own target through a crisis stage only through an edge, such as the one-tick tail after a reign or a transfer between its AI stage and the next crisis stage.
   - Do not change `FleetDelayOf` in this commit; that would add a new hash path (H `:403`).
   - If check (b) still holds (the knob is the only change) and the twin's `conflict:` lists the awakening, accept the move. Record the seed and tick, and correct "AI는 지키지 않는다" in N §5 and the DECISIONS_LOG line.
   - Otherwise stop.
3. **Header over 149,000 bytes** (the self-imposed margin under H §8's 150,000). Use the short H line and move the rest to N §5.
4. **`{W_TABLE}`'s claim line is not 550 / 0.2629 at {H8B}** (a law or card from `[B-6b7]` or `[B-6b8]` changed it). Stop before the edit and re-read ⓔ. `Data.Integrity` is the judge either way.
5. **Mutation rerun scope.** The `[B-6b6]` brief's mechanical rule ("Breaks test defined in an edited test file") would pull in about 119 `CosmicDiplomacyTests.cpp` rows (about 4-6 hours). Use the narrow rule instead: the filter test or the patched file, which is 9 patches at HEAD plus any additions. The whole-B6b rerun stays with `[B-6b12]`.
6. **Bundle K's cost or overlap.** If the lenses finish first, wait for the bundle; never start a build under it. If time is short, cut it to input-less 30 + R 30 + wanderer 30 pairs, never to zero. K1 and K2 keep their meaning.
7. **[개발자]** Nothing in this commit needs the developer. The B6a hand checks ①, ③ and ⑨ do not block it (as in `STATUS.md`).

## 추가 이야기와 보완

- `<scratchpad>\b6b8b\w_recount.py` reads the JSON and repeats the invariant's arithmetic, including the sphere factor. `[B-6b9]` to `[B-6b11]` can rerun it whenever a value commit touches `EventChancePerSlot`, `ThroneLockTicks`, the vigil bonus or a card delay, so the commit message can quote W without hand arithmetic. If it is ever committed under `Tools/`, `docs/40` needs one line.
- The commandlet's `conflict:` "delay bought per tag" already predicts which runs a hazard-cap knob can move. One sentence in `docs/40`'s "measurement switches" section, such as "a guard knob moves only runs whose share-0 log lists the tag with a delay above the new cap", would save the next knob commit the derivation. It could go into this commit's `docs/40:366` edit.
- `Guard.DelayTip` (`ST_CA.csv:340`) has no number, so the 75 is invisible to the player. That is consistent with rule 11. If a later presentation commit wants "위기마다 다르다", it is a text-only change.
- The guard that lasts one tick past the end of a reign (§2) is harmless and deterministic, but it is a guard nobody ordered. B7, which gives AI peoples their own guarding, could clear the target in the tenth stage when a reign ends. That would be a new hash path, so it is not done here.

---

## Critique dispositions

1. **Accepted.** I verified that `seed42.json` at HEAD has no hash-run key and that the hash runs go through `Compare-HashRun` (`run_checks.ps1:139-155`). (a) and §0.4 were rewritten.
2. **Accepted.** (d) and K2 were tautologies. They are replaced with the two-sided rule (i)-(iii) on the twin's `conflict:` `Hazard.AIAwakening=<sum>(<count>)`, and `{FIRST_MOVED_TICK}` is now only recorded.
3. **Accepted.** N §0.2 `:195` says "**닫는다** … `[B-6b8b]`", and CLAUDE.md requires deleting the row. The #132 row is now deleted in this commit, with a B7 line for the drain side and STATUS `:40` adjusted. N §5 item ⑤ records it.
4. **Accepted.** I verified that STATUS `:7` says "(몫 `FleetDelayCountdownShare` 0)" and `:38` says "위 끝 66.7%". Both are edited, and `:40` too (from finding 3).
5. **Accepted.** I verified that all five current `checks*.log` print `delay bought per tag []`. Bundle K is now mandatory, both record variants are drafted, the `verified_at`-only note is added, and `{PRED}` picks the variant.
6. **Accepted.** H `:334` is correct as written and its citation `:686-720` is stale (`ActDiplomacy` `:711`, gate `:719`, step (1) `:733-780`). "모자람" was dropped, and the item is now an addition plus a citation fix.
7. **Accepted as plausible.** No role path clears a guard on the people's own system (`PlayerRoleSubsystem.cpp:1189` clears only a besieging target), and the AI-stage clear `:3005-3011` runs after the crisis stage. The one-tick tail is added to §2, §3, the DECISIONS_LOG line and the H line. Attribution uses the `conflict:` list, which counts every owner.
8. **Accepted.** I verified 34 hits at HEAD, including comments, patches and docs. §0.3 is now a saved-list diff that stops only on a new hit in a non-comment `.cpp`/`.h` line.
9. **Accepted.** I verified that `:403` is the `if` and `:402` the comment. The patch row cites `:403`, and the NaN reasoning is included.
10. **Accepted.** Under the game's 0.5 the claim's cap of 150 gives 600 and 60 slots, with 0.976^60 below 0.25. The expected double break goes into §5 and the README row (`:791`), to be confirmed with the probe first.
11. **Accepted.** I verified "지금 셈" at `docs/21:1199`, "지금 값" at `:1518` and `docs/40:366`, and the `FleetDelayCap` term in `docs/40:237`'s formula. The rebellion's 0 comes from `FleetDelayOf` `:369`. All are fixed, and docs/20 now names the four rows that keep 100.
12. **Accepted.** The header comment now says "cap 100 -> 75 (0.5 x 150)", names the rows that keep 100, and says the rebellion gains none.
13. **Accepted.** H §8's limit is 150,000, and the 149,000 is now named as a self-imposed margin. The H line is inserted after the last §11 bullet (`:468`), before `## 추가 이야기와 보완` (`:470`).
14. **Accepted.** I verified H §2.10's stale `:395-405` and `:1084` and the panel's real `:1098`. They are added to ③ (with `:366-392`), and the brief's own panel citation is corrected.
15. **Accepted.** I verified that `bundleA.py:47` already passes `-Rules=ProvisionalEndTick:0` and that only the first `-Rules=` is read (`:1482`). The twin now joins both values with `;`, and the switch control requires both `rules … = 0` log lines.