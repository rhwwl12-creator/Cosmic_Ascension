# [B-6b8] implementation brief: data ③, the side rows (`event_era_endured`, `enc_witness`), then bundle B and the judgments written ahead in H §5.3

I checked every HEAD line cited here with `git show HEAD:`. The task named HEAD as `cef8cc7`. Since then `[B-6b4]` has been committed as `529cab9`, and that is now HEAD. I re-read the lines I relied on at `529cab9`. Step §0.6 re-derives every line on [H‴].

**Short names**
- H = `docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md`; N = its `_notes.md`.
- [H‴] = the tree after [B-6b4] to [B-6b7].
- B5brief, B6brief = `scratchpad\b6b5\brief_final.md`, `scratchpad\b6b6\brief_final.md`.
- merged = `scratchpad\b6b5\merged.md`.
- Paths are under `Source/Cosmic_Ascension/` unless they start with `Tools/`, `docs/` or `Config/`.

**Scope** (H:394; effort high). This commit changes data, schema rules, tests and measurement code only. It adds no new path that moves hashes (H:403).
- The check is the set of judgments written ahead in H §5.3, which are frozen.
- The usual minimum still applies:
  - one runner pass `-Tier full -Data` (N §5 [B-6b3b] '일의 속도'); its two passes count as the two consecutive test runs;
  - a full build with the editor closed, because headers are edited.
- `FleetDelayCountdownShare` 0.5 belongs to `[B-6b8b]` (H:395). Bundle B runs on this commit's build, before [B-6b8b] (H:334).

---

## 0. Start state: re-check on [H‴] before any agent runs

1. **The earlier commits are in.**
   - `git log --oneline -5` shows `[B-6b7]`, `[B-6b6]`, `[B-6b5]` and `[B-6b4]`.
   - `git status` is clean apart from the user's known local changes.
   - No other workflow is building, and no UnrealEditor or UBT process is running.
2. **Row texts come from N §1.5 only**, as committed by [B-6b5] (H:391: "그 행을 켜는 커밋이 글자 그대로 옮긴다"). At HEAD, N:469-471 is empty.
   - **If N §1.5 is still empty on [H‴], stop.** Send it back to [B-6b5] or to a `[B-6br]`. Do not copy the texts from merged.
   - Use merged:147-157 (final column) only for comparison. If it differs from N §1.5, N §1.5 wins; record the difference in N §5.
3. **Schema at [H‴].** Confirm that:
   - the [B-6b5] lists `dawn_words`, `silence_words` and `desolation_words` exist;
   - the DT_Events era rule exists: `when has_type CivPressureEnduredTicksReached` → `LogTemplates` lacks those three (merged:238-242);
   - and record the walker-count pin in `Tools/test_data_schema.py`. It is 271 at HEAD :3563; [B-6b5] said 305; take the [H‴] value.
4. **Placeholders read now, never invented:**
   - **`<T_DEATH>`**: from N §2, the paragraph "`[B-6b6]`의 다시 셈". It is the median ruled-T4 tick over bundle A's R runs that reach T4, rounded up to the next 100. If n = 0 it is 4,400, flagged in H §11 (B6brief §4.5). It is used as the harness argument `-DeathAt=<T_DEATH>`.
   - **`<N>`, `<P_bind>`, `<r>`**: from [H‴] data (`project_binding`, `feat_laws_and_binding`, `DT_Rules.AIUpperRungProgressMultiplier`) and from N §2 as [B-6b7] left it. They are for the records only; bundle B passes no value.
   - **`<A_SEC_PER_20K>`**: bundle A's measured wall time per 20,000-tick run, from the header line of N §3 "묶음 A". It is used for `<B_MINUTES>` (§4.1).
   - **`<B6b7_KEYS>`**: the five lines of `Tools/baselines/seed42.json` at [H‴].
5. **Header size.** Run `wc -c` on H. HEAD is about 146,814 bytes, and [B-6b4] to [B-6b7] each add one §11 line of about 350-400 bytes.
   - If H plus this commit's line (§6, about 365 bytes) would pass 149,000, move that many bytes of H body to N in this commit (H:403 rule). List the move in N §5.
6. **Re-derive every line number** cited below on [H‴]. HEAD anchors:
   - `Tests/CosmicDataTests.cpp:2693-2711` (tripwire);
   - `Tests/CosmicB6bRowNames.h:32,35`;
   - `Tests/CosmicBalanceCommandlet.cpp:642-650`, :1691-1702, :2228-2239, :2861, :2949-2957, :3890, :3922-3925, :3955-3986;
   - `World/CosmicWandererPolicy.h:44-48`, `.cpp:46-85`;
   - `Tools/data/DT_Events.json:380-419`, `DT_Encounters.json:1596-1691`;
   - `Tools/data/schema.json:1129-1149`;
   - `Tools/test_data_schema.py:2452-2463`, :624-632.
7. **Mutation apply state.** In a scratch copy of [H‴], run `git apply --check` on every patch under `Tools/mutations/` and save the result as `scratchpad\b6b8\apply_before.txt`.
8. **Clean build first** of both targets on a clean [H‴]. Earlier workflows end with mutation runs.
9. **Base copies** for the baseline bridge (§3.3). On that build, before any edit:
   - run the runner's five commandlet runs with their exact arguments (`Tools/run_checks.ps1:116-132`);
   - write them to `Saved\Balance\b6b8_base\` with an absolute `-Out` and `-AbsLog`;
   - each key must equal `<B6b7_KEYS>`.
10. **Tripwire at [H‴]** (after [B-6b7]): the `Data.Integrity` pin reads the eight names before `EventEraEndured` and `EncWitness`. If it does not, stop.

---

## 1. Data rows

### 1.1 `Tools/data/DT_Events.json`: append after `event_claim_eased` (the last row)

Change the last row's closing `}` to `},`, add this row, and keep the closing `]`. The file uses LF line endings and tab indentation.

The fields follow H:192 (§2.7) and the `event_book_closed` model (`DT_Events.json:2-49`).

```json
	{
		"Name": "event_era_endured",
		"Category": "Event.Civ.Society",
		"LogCategory": "Log.Civ",
		"Trigger": "Standing",
		"Subject": "Civilization",
		"Conditions": [
			{
				"Type": "SystemOwnedBy",
				"Tag": "",
				"Key": "Ruled",
				"Value": 0.0
			},
			{
				"Type": "CivPressureEnduredTicksReached",
				"Tag": "",
				"Key": "600",
				"Value": 0.0
			}
		],
		"BaseWeight": 0.0,
		"CooldownTicks": 0,
		"Effects": [
			{
				"Type": "ChangeReverence",
				"Tag": "",
				"Key": "Borders",
				"Value": 0.2,
				"DurationTicks": 0
			}
		],
		"LogTemplates": [
			"{Civ}의 백성이 이 시대를 '물레의 시대'라 부르기 시작했다.",
			"{Civ}의 백성이 이 시대를 '장독의 시대'라 부르기 시작했다.",
			"{Civ}의 백성이 이 시대를 '느린 수레의 시대'라 부르기 시작했다."
		],
		"Severity": 1,
		"MinTierToReveal": 0,
		"SignRow": "None"
	}
```

- **Texts** are N §1.5 C6 letter for letter (compare with merged:147-149; [1] is L1-10's corrected '장독').
- **`Category`** is not fixed by H. Recommended (counts as approved): `Event.Civ.Society`.
  - It matches the book rows, and the pair is allowed (`schema.json:113`).
  - Category only weights the lottery shares (`Events/CosmicEventSubsystem.cpp:1169`), so it has no effect on the world for a Standing row.
- **Key 600** is the design's era period (`systems.md:233`).
- **Condition window.** Under `EventSlotTicks` 10, one slot is open while the count is 600-609 (`Sim.EraSelector`, `Tests/CosmicUpperRungTests.cpp:719-727`). So the row fires once per stretch of endured pressure.
- **`Borders`** raises the neighbours' reverence only, never the context people's. For a people with no neighbour it returns false; `ApplyAll` ignores the return value (ev.cpp:971-977), so the line is still told and no error line follows.
- **`EpithetRow` is not used** (H:193).

### 1.2 `Tools/data/DT_Encounters.json`: append after `petition_pressure_measured` (the last row)

The file order is not the lexical order: in the lexical walk this row sorts after `enc_star_watch` and before the `petition_*` rows. The row has the shape of `enc_star_watch` (`DT_Encounters.json:1303-1376`); its fields come from H:200.

```json
	{
		"Name": "enc_witness",
		"Title": "그늘의 가장자리",
		"Body": "{System}의 땅 한쪽에 그늘이 드리워 있다. {Civ}의 학자들은 그 그늘이 어디까지 닿았는지 다 재지 못했다.",
		"Conditions": [
			{
				"Type": "PlayerRoleIs",
				"Tag": "",
				"Key": "Wanderer",
				"Value": 0.0
			},
			{
				"Type": "PlayerInSystemWithHazard",
				"Tag": "Hazard.Claim",
				"Key": "None",
				"Value": 0.0
			}
		],
		"BaseWeight": 1.0,
		"Choices": [
			{
				"Label": "그늘의 가장자리를 재어 둔다",
				"CheckStat": "Mind",
				"Difficulty": 3,
				"Cost": {
					"Energy": 0.0,
					"Matter": 0.0,
					"Insight": 0.0,
					"Influence": 0.0,
					"Causality": 0.0
				},
				"OnSuccess": [
					{
						"Type": "ChangeReverence",
						"Tag": "",
						"Key": "None",
						"Value": 0.1,
						"DurationTicks": 0
					}
				],
				"OnFailure": [],
				"SuccessText": "당신은 그늘이 닿은 땅의 가장자리를 재었다. 당신이 잰 것은 {Civ}의 학자들의 기록에 남았다.",
				"FailureText": "그늘의 가장자리는 잴 때마다 달랐다. 그 그늘이 어디까지 닿았는지는 여전히 가늠되지 않았다.",
				"NextEncounter": "None"
			},
			{
				"Label": "물러난다",
				"CheckStat": "NoCheck",
				"Difficulty": 0,
				"Cost": {
					"Energy": 0.0,
					"Matter": 0.0,
					"Insight": 0.0,
					"Influence": 0.0,
					"Causality": 0.0
				},
				"OnSuccess": [],
				"OnFailure": [],
				"SuccessText": "당신은 그늘을 재지 않았다. 그 가장자리는 {Civ}의 학자들이 계속 재었다.",
				"FailureText": "",
				"NextEncounter": "None"
			}
		],
		"Illustration": "",
		"Trigger": "Standing",
		"Subject": "VesselSystem",
		"VoiceBodies": []
	}
```

- **Texts** are N §1.5 C7 letter for letter (compare with merged:150-157). The hand-off at merged:392 asks for a reread of the label without the model's '알린다'; that goes to lens L1.
- **Effect key.** Key `None` on `ChangeReverence` means the context people, which is the system's owner. This is `enc_star_watch`'s form; see `Sim.EraSelector`, "An empty Key is the context people".
- **No `ResolveHazard` or `DelayHazard`** (H:201). There is no `PlayerTierAtLeast`.
- **Where it can come.** The voice gap must be ≤ 1 (`Events/CosmicPetitions.cpp:171-175`), so the card comes only on the land of rung-1 and rung-2 owners.
- **It never competes with another card.**
  - A system holds one hazard, and the four other wanderer Standing cards each name a different hazard tag (verified on the HEAD rows). So whenever `enc_witness` is a candidate, it is the only one.
  - It is therefore always picked: `Present` skips the arrival lottery when a Standing candidate exists (`CosmicPetitions.cpp:293-316`).
  - It is not offered again straight after it was answered (`LastAnswered`, :290-299).
  - It can also come when a claim lands on the system where the wanderer already stands, through the `HeldAtHead` path, with no arrival.
  - This matters for §3.1.

### 1.3 Names

**Recommended (counts as approved): keep both names in `CosmicB6bRows`, with no `CosmicRows::` constant.**
- No game code calls them by name: the slot and `StandingHeld` walk the tables.
- A constant would force a false `required_rows` entry (B6brief §1.4).
- Edit `Tests/CosmicB6bRowNames.h:32` and `:35`: "- [B-6b8]" becomes "- in the data from [B-6b8]". This is a header edit, so it needs a full build.
- Exception: see the epitaph question in §7, risk 6.

### 1.4 Schema and checker

Files: `Tools/data/schema.json` and `Tools/test_data_schema.py`; `Tools/cosmic_data_schema.py` only if needed.

Both rows already pass the [H‴] rules:
- **Era row:**
  - Civilization → `slots_in [Civ, Hazard]` (`schema.json:466`);
  - three variants;
  - the slot vessel rule (Ruled is allowed, :480-494);
  - `BaseWeight` 0 for Standing (:503);
  - no Named or SettledSystem together with the condition (:525, :530);
  - `reverence_reach` Borders (:39, :1244);
  - `civ_selector` Ruled (:37, :1204);
  - the [B-6b5] era words rule;
  - LogTemplates `forbidden` (:450).
- **`enc_witness`:**
  - the `enc_` wanderer rule (:994-1000);
  - no `EnactEdict` or `DelayHazard` on an `enc_` row (:1086-1092);
  - rule 8′ (:1129-1149);
  - the free-choice rules;
  - `epitaph_leaves_out` (:109); the title does not speak to the reader.

The gate is `python -I Tools/check_data.py` reporting 0. **Add two small rules** (recommended, counts as approved).

**Rule W: the witness words** (B5brief:343, :652; merged:309 "`[B-6b8]`의 선택 규칙이 이 낱말을 기계로 볼 수 있다").
- Two new `words` lists, appended after the [B-6b5]-[B-6b7] lists.
  - `witness_words` = ["본 것","보고 들은","말하","말했","말해","전하","전했","전해","증거","증언","증인","목격","기록하","기록했","기록해","지나간다","지나가","지나갔","떠난다","떠나","떠났","외면","눈을 돌","등을 돌","모른 척","피했","도망","부끄"]. The match is a plain substring test, so the contracted past forms of 하다 (했/해), 떠나다 (났) and 지나가다 (갔) have to be listed on their own. They are the forms a result line uses; the nearest model has "당신은 관측소를 떠났다" at `DT_Encounters.json:1367`.
  - `sign_voice_words` = ["목소리","속삭","이르기를","말씀"].
- A DT_Encounters rule appended at the end of `rules`, after the `petition_pressure_measured` rule. Putting it there keeps the trailing context of the b6b2 rule-8′ patches intact.
  - `when`: `[{"row_prefix":"enc_"},{"col":"Conditions","has_cond":{"tag":"Hazard.Claim"}}]`. This avoids `row:` (`cosmic_data_schema.py:600-623`) and covers every later claim card.
  - `then` (8 tests):
    - `witness_words` absent from `Title`, `Body`, `VoiceBodies`, `Choices.Label`, `Choices.SuccessText` and `Choices.FailureText`;
    - `Body` lacks `sign_voice_words`;
    - `["기록"]` absent from `Title`, `Body`, `VoiceBodies`, `Choices.Label` and `Choices.FailureText`. This enforces rule 7's "'기록'은 성공 줄에만"; the column test cannot tell one choice from another, so the retreat's SuccessText is held only by the text audit. Record that in N §5.
  - `why` (English, no verse): "a wanderer's card on the claim is a sign seen or measured: it does not testify, record, tell or speak, the sign has no voice, and the step back neither leaves nor shames (B6b header 2.8, notes 1.6 rule 7; decision 20). A substring test: the contracted past forms are listed apart, and a false hit inside a longer word is accepted within this narrow scope".
- '기록에' is not '기록하', and '재었다' contains no '전하'. The C7 texts give 0 hits (mapper check). Re-run the check on the final N §1.5 texts.

**Rule E: the shape of decision 22.** This rule is not in H. It is the only place where the theology choice "whose reverence rises, and when" is enforced by data alone.
- A DT_Events rule.
- `when`: `[{"col":"Conditions","has_type":"CivPressureEnduredTicksReached"}]`.
- `then`:
  - `{"col":"Conditions","has_cond":{"type":"SystemOwnedBy","key":"Ruled"}}`;
  - `{"col":"Effects","has_cond":{"type":"ChangeReverence","key":"Borders"}}`.
- `has_cond` works on effects columns (`cosmic_data_schema.py:215-216`, `holds` at :1367-1373). No fallback is needed.
- `why`: "the era of an endured pressure is told for the ruled people alone and raises its neighbours' reverence for the vessel (B6b decision 22, the answer A; header 2.7) - without Ruled every AI people's endurance would lift its neighbours".

**`Tools/test_data_schema.py`:**
- **Existing passing cases that Rule E would break.** On [H‴], grep for every passing case (`assertEqual(..., [])` or `wanted=False`) whose row holds `CivPressureEnduredTicksReached`. Give each one `cond("SystemOwnedBy", key="Ruled")` and `eff("ChangeReverence", key="Borders", value=0.2)`.
  - HEAD :2456 (`harvest(Trigger="Standing", BaseWeight=0, Conditions=[endured])` on `event_test_harvest`, :2324-2328) and the [B-6b5] era-words passing setup are among them.
  - Also check that the `expect` cases at :2457-2463 still find their own message. Find out whether `expect` accepts extra messages; if it pins an exact list, add the Rule E message there.
  - Run the Python tests before writing the Rule E mutations.
- **`walked` pin:** set it from the schema, never by hand arithmetic.
- **`rule_setups`:** one entry per new rule.
  - W: a copy of `enc_test_weave` with `Conditions=[PlayerRoleIs Wanderer, PlayerInSystemWithHazard Hazard.Claim]` and `Trigger="Standing"`.
  - E: a Standing Civilization event with the condition, Ruled and the Borders effect.
- **New method `test_the_witness_words`:**
  - pin both lists exactly;
  - one breaking case each:
    - '증언' in a Label;
    - '목소리' in the Body;
    - '도망' in `Choices[1].SuccessText`;
    - '기록하' in a FailureText;
    - '기록했다' in a FailureText;
    - '떠났다' in `Choices[1].SuccessText`;
    - '기록' in the Body;
  - one passing case: the same words on an `enc_` row with no claim condition (the scope);
  - Rule E: one breaking case without Ruled, and one with `ChangeReverence` key None.
- Update every exact list or `forbidden` pin that these lists touch.

### 1.5 Assets

Run `make_data_tables.py` through the runner's `-Data`. Commit exactly these files:
- `Content/CA/Data/DT_Events.uasset`;
- `Content/CA/Data/DT_Encounters.uasset`;
- `Tools/data/generated.sha1`.

Any change to another `DT_*` or to `ST_CA` means stop. There are no new ST_CA keys.

---

## 2. Code, tests and constants

### 2.1 Tripwire (`Tests/CosmicDataTests.cpp`, HEAD :2693-2711, already edited by [B-6b6] and [B-6b7])

- Assert `Today == Ten`. `Ten` is the existing array (HEAD :2704-2706), in `Note` order (`Tests/CosmicTestHelpers.h:375-387`).
- Message: "Today's data holds all ten of B6b's rows: WithoutB6bData takes out exactly them (taken: %s)".
- Comment (1): "Today's data holds the ten".
- The full-copy check (2) still holds, because `StandIn` skips rows that already exist.
- Leave `CosmicTestHelpers.h:353` ("A name the table does not hold yet takes nothing out") and `CosmicBalanceCommandlet.h:123` unchanged.
  - Both stay true of test tables.
  - Leaving them avoids rerunning the 17 patches on that helper.
  - Note this in N §5.

### 2.2 New tests (recommended, counts as approved)

Put them in one new file, `Tests/CosmicSideRowTests.cpp`, so that no old filter's file changes.

If the board helpers they need are file-local and copying them would take more than about 60 lines, use the existing files instead: `CosmicBookTests.cpp` beside `Sim.PressureEndured` (:358), and `CosmicWandererTests.cpp` beside the `StandingHeld`/`Present` cases. In that case the rerun set grows by those files' filter patches.

**`Sim.EraEndured`.** The game's row, copied alone into a fresh DT_Events, on a drain board in the form of `Sim.PressureEndured`'s `WithDrain` (no hazards, no recovery).
- Set the ruled people's `PressureEnduredTicks` just below 600 with the drain on, and step until the window passes. There is exactly one `event_era_endured` line for the ruled people.
- Each bordering people gains +0.2 (from 0); the people itself does not.
- An AI people under the same drain gets no line.
- At 599 there is no line.
- After the window, the same stretch does not fire again.
- Firing does not change the sim stream: compare the `GetRng()` state with a copy of the board stepped without the row.

**`Sim.WitnessCard`.** Uses the game's encounters.
- A rung-1 wanderer that arrives on a claimed system of a rung-1 or rung-2 owner is shown `enc_witness` (`StandingHeld` → `Present`).
- A claim placed on the system where the wanderer already stands also brings the card (the `HeldAtHead` path, no arrival).
- With a rung-3 owner, there is no card.
- Right after the card is answered, it is not presented again (`LastAnswered`, `CosmicPetitions.cpp:290-299`).
- Choice 0 with the vessel's `Mind` set so that `PassingRollsOf` (:470-478) covers every face: the owner gains +0.1, the claim still stands, and its countdown is unchanged. The pass must not depend on the stream.
- Choice 1 changes no reverence.
- Each accepted answer draws exactly one sim number.
- **The policy:** `CosmicWandererPolicy::Act`, answering two witness cards in turn, counts one in `NoCheckByRow` (§2.3) and two in `AnswersByRow`.

### 2.3 Measurement fixes (bit-identical; recommended, counts as approved)

N §2.1 asks for "증언 성공·실패·물러남" (N:586). At HEAD the race line cannot give that split:
- `WithdrawnByRow` counts a card that stopped standing before it was answered (`CosmicWandererPolicy.cpp:69-74`), not the 물러난다 choice.
- `succeeded = AnswersByRow − FailedByRow` (`CosmicBalanceCommandlet.cpp:3922-3925`) mixes Mind-3 passes with NoCheck retreats.
- The policy picks `Answers % Choices` (:65), so about half of all answers are retreats.

The fixes:

1. **`World/CosmicWandererPolicy.h`:** add `TMap<FName, int32> NoCheckByRow;` after `WithdrawnByRow`. It is not saved, not hashed, and not read by play.
   - In the `.cpp`, compute the index `Pick = Counts.Answers % Row->Choices.Num()` before `AnswerPetition` and keep it.
   - After an outcome that is neither Withdrawn nor Refused: `if (Row->Choices[Pick].CheckStat == ECosmicCheckStat::NoCheck) ++Counts.NoCheckByRow.FindOrAdd(Card);`.
2. **`Tests/CosmicBalanceCommandlet.cpp`:**
   - Add `int64 WitnessFirst = -1;` beside `EraFirst` (HEAD :646).
   - Set it in the input block (HEAD :2949), outside the by-record gate: when `CardsByRow[EncWitness]` rose at this boundary, `WitnessFirst = WitnessFirst < 0 ? State.Tick : WitnessFirst`. The count also runs in input-less runs, because the counting at `CosmicWandererPolicy.cpp:46-50` comes before the `bWander` return.
   - Reprint the part at HEAD :3985 as: `"era lines %d (by the record %d, first at %lld), witness cards %d (first at %lld, by the record %d; answered %d: by the check %d, failed %d, without a check %d; gone before the answer %d)"`.
3. These are edits to a header and to the Tests module, so they need a full build. They take no draw and touch no state. `WitnessFirst` is guarded by the harness controls in §4.2 and by the mutation `b6b8_cmdlet_witness_first_any_card` (§5).
4. **The ark counter offered by B6brief:680** ("expired with the ark standing, and its progress") is **declined here**.
   - No judgment in bundle B reads it; the ark-cost judgment was [B-6b6]'s.
   - Adding it would add a commandlet field, a `Sim.RaceMeasure` case and a mutation to a commit whose job is the side rows.
   - If [B-6b7] already added it, read it in bundle B. Otherwise bundle B's extinction record writes "방주를 둔 채 만료 — 재지 않음 (카운터 없음)", and the counter is offered to `[B-6b9]` or `[B-6b11]` if ark values change. Record this in N §5.

### 2.4 Tests whose meaning changes

N:425: find the cause first; never tweak a seed or an expected value. Every case goes into N §5.

| Test | Why it may move | On a break |
|---|---|---|
| `Sim.WandererLoop` (`Tests/CosmicWandererTests.cpp`, seed 4; N:443) | The witness shifts the policy's answer parity (H:203). That is the B1-guard mechanism this block measures | **Never switch off B6b data here.** Find the cause (#122, an extinction erasure, the witness, the era). Record it in H §11 (N:443) and N §5, and hand it to bundle C's B1-guard reading (N:614). If seed 4's loop no longer closes because of the witness or the era, that is a [개발자] finding, not a fixture change |
| `Sim.PhaseTwoExit` (input-less, 20,000) | The era before the deposition (world); the witness before the deposed vessel (knowing only) | Same as `Sim.WandererLoop`: never switch off B6b data; find the cause and record it in H §11 and N §5 |
| `Feats.Reachable` (seed 42 at 20,000; non-ruled tiers) | An early era moves reverence | Re-run. Its subject is not B6b, so `WithoutB6bData` outermost if the cause is the era or the witness |
| `Sim.BookCycle` game section (`CosmicBookTests.cpp:615-660`) | A 300-tick stay is below 600 | Expect no change |
| `Sim.EraSelector` (`CosmicUpperRungTests.cpp:698`) | It evaluates the condition directly | Unchanged |
| `Data.Integrity` theodicy check (`CosmicDataTests.cpp:2477-2570`) | It reads only `ResolveHazard` and card `DelayHazard`; the witness has neither | Unchanged |
| `test_data_schema.py:616-632` (epitaph titles) | The title does not speak to the reader | Must pass (see risk 6 if L1 confirms the epitaph concern) |
| Any other test that runs a ruled people under pressure for ≥ 600 ticks on the game's DT_Events, or puts a wanderer on a claimed system on the game's DT_Encounters (grep candidates: `CosmicHazardTests`, `CosmicDiplomacyTests`, `CosmicSpiritualSightTests`, `CosmicUpperRungTests`) | The era or the witness acts | `WithoutB6bData` outermost if the test's subject is not B6b (B6brief §2.4, "any other test") |

---

## 3. Determinism and baselines

### 3.1 When the world can diverge (no new draw point)

**The era** fires in the Standing walk of the crisis stage's slot.
- The slot's numbers are drawn before any row is read (ev.cpp:1044-1053).
- `FireResolved` draws only Tell numbers.
- The `Borders` reverence is world state.
- First tick that can differ: **E** = the tick of the first era line.

**The witness** is presented in the knowing stage's waiting slot, using a derived integer (`CosmicPetitions.cpp:299-301`). Presentation writes knowing state only, so only the full hash moves.
- An answer draws the one existing sim number and adds +0.1 reverence, which is world state.
- It also displaces the arrival-lottery card that the off run would have drawn.
- First tick that can differ: **W** = the tick of the first witness presentation (`WitnessFirst`).
- In cells without `-Wander` (R, slowR, input-less, death), the policy never answers (`CosmicWandererPolicy.cpp:46-53`: it counts the card, then returns when `!bWander`). There the witness is knowing-only for the whole run, unless the waiting slot gates a world path; L2 checks that.

**Nothing reads either row earlier.**
- `StandingHeld` at the head of the tick only lists candidates.
- The Standing walk's `CheckAll` is pure.
- The ruler's petition walks skip wanderer cards (`IsWandererCard`).
- Lens L2 verifies this; the pair check is the test that can break.

The CSV sample at tick X is written after that boundary's inputs (`CosmicBalanceCommandlet.cpp:3541`), so the row for tick E or W already differs. The bound is therefore strict.

### 3.2 The deviation from N ⓗ (record it; do not edit ⓗ)

N:573 ⓗ says: "켬/끔 짝은 어느 백성이든 첫 셋째 칸까지 같은 세계다". From this commit that is false: the era fires on rungs 1-3, and the witness comes to a rung-1 wanderer from tick 1,201.

**New bridge per on/off pair (same build):**
- **Wanderer cell:** T_B = min(T + 1, E, W).
- **Every other cell:** T_B = min(T + 1, E). This makes "a presentation alone does not change the world" a check that can fail. It holds only if L2 confirms two things first:
  - no column of the CSV, `_rows.csv` or `_claims.csv` reads `Knowing`;
  - a waiting wanderer card gates no world path in a run without answers.
  
  If either fails, use min(T + 1, E, W) in every cell and record the reason in N §5.
- In both: T = min(first AI T3, ruled T3), taken from the on run's `ending:` line, as in bundleA's `threshold_of`. E is the race line's `era … first at`; W is its `witness … first at`. A value of −1 means ∞.
- CSV and `_rows.csv` rows with **Tick < T_B** must be equal, apart from `TickMicros`, `SaveBytes` and `SaveRawBytes`.

Record this in H §11 (the §6 line) and in N §5. The same-build bridges stay as they are:
- forced death against R on: Tick < T_DEATH;
- wanderer against R on: Tick < 1,200.

### 3.3 Baselines (N:599; H §5.4)

1. **Probe seed 42 first** (§7, step 3). Run five on-runs with the runner's exact arguments into `Saved\Balance\b6b8_on\`, and read each race line's era and witness counts.
2. **Checks that can fail:**
   - **ⓐ** Five off-runs (`-WithoutB6bData=1`) give [B-6b5]'s five lines letter for letter, with the `wrote …` tail removed. The values are in B6brief:405-412:
     - 3,000: 2816687688 / 3300228750
     - ruler 3,000: 3561771757 / 2462331303
     - wanderer 6,000: 2665013209 / 2229239127
     - 20,000: 2359424635 / 726895141
     - ruler 20,000: 1611658162 / 29194274
   - **ⓑ** Compare each on-run with its `b6b8_base` copy (§0.9):
     - rows with Tick < min(E, W) are equal in the wanderer run;
     - rows with Tick < E are equal in the ruler and input-less runs, even when witness > 0 (§3.2, after L2's confirmation);
     - a run whose own race line says `era lines 0` and `witness cards 0` gives `<B6b7_KEYS>`' line letter for letter, ruler and wanderer tails included (the wanderer line's `cards` would count a witness).
   - **ⓒ** Every log has 0 `end check` and 0 `race check` lines.
3. **Expected movement** (N:599). Do not assert it; read the probe.
   - **Input-less:** the world hash probably stays. Seed 42 is deposed at 1,072 (`seed42.json:13`), and Ruled is false after that. The full hash moves if a witness is placed before the standing deposed vessel.
   - **Ruler:** moves only if the era fires.
   - **Wanderer 6,000:** moves if either row fires.
   - **Hash runs:** move with their 3,000 lines (the runner compares them).
4. **Re-record.** The final runner pass `-Tier full -Data -UpdateBaseline` rewrites the moved lines and `verified_at`.
   - Then edit `last_recorded_in_changelog` by hand, only if a line moved (B6brief §3.2 form; UTF-8, no BOM):
     - task: "B6b B-6b8 (data on, the side rows: event_era_endured, enc_witness)";
     - each line's first mover (E or W).
   - If the probe moves seed 42's wanderer crowning or its counts, also update the facts that quote them (B6brief §3.2.5):
     - the comment at `Tools/run_checks.ps1:121-124` ("…at 3,121 - by the vacuum");
     - `docs/40:25`;
     - the facts in `STATUS.md:27`.
5. **`docs/30`** is not regenerated, because no knob changes. `make_config_reference.py --check` must pass.

---

## 4. Bundle B and the judgments

### 4.1 Runs (H:330; H §5.2)

All runs are 20,000 ticks with `-Rules=ProvisionalEndTick:0`, on the home row, seeds 1-60, on the **final** [B-6b8] build.

| Cell | Arguments | Off side |
|---|---|---|
| R | `-Answer=1 -Build=1 -Focus=1 -Edict=1 -Diplomacy=1 -TellAll=0` | `-WithoutB6bData=1` |
| slowR | R with `-Focus=0` | `-WithoutB6bData=1` |
| wanderer | R + `-Abdicate=1200 -Wander=1` | `-WithoutB6bData=1` |
| inputless | none | `-WithoutB6bData=1` |
| death | R + `-DeathAt=<T_DEATH>` (H taken literally: no `-Wander`, unlike B5's precedent; record that in N §5) | none; it pairs with R on, same seed |

- **Processes:** 540 in the bundle, plus 5 for ⓐ, 5 base and 5 on = 555.
- **Time:** H:328's "약 22분" is too low. The bundle is 10.8 M ticks against bundle A's 3.12 M, so `<B_MINUTES>` = `<A_SEC_PER_20K>` × 540 / workers / 60, which comes to about 35-42 min at 6 workers. Record the measured value in N §3.
- **Forced-death cell:** record only in bundle B. No judgment of ①-④ uses it; '< 15%' is read in bundle C. Its control is the calling line "death asked at `<T_DEATH>` (recorded r)". Record r = 0 cases; they are not failures.

### 4.2 Harness: `scratchpad\b6b8\bundleB.py`, a copy of `scratchpad\b6b6\bundleA.py`

- `CELLS` as in §4.1.
- `EXPECTED_REMOVED` = `"feat_passage_by_ark feat_laws_and_binding project_ark project_binding crisis_species_extinction edict_wide_machine_accord edict_wide_quarantine edict_wide_warning event_era_endured enc_witness"`.
- Also match `AIUpperRungProgressMultiplier 1 (was <r>), FleetDelayCountdownShare 0 (was 0)` (`CosmicBalanceCommandlet.cpp:1596`).

**Folder:** run in `scratchpad\b6b8\bundleB\`, outside OneDrive (about 550 MB), with an absolute `-Out` and `-AbsLog`. The session scratchpad may be cleared before [B-6b9] or [B-6b11] reread it, so after the judgments:
- copy every `.log` and a sha1 manifest of all output files to `Saved\Balance\b6b_bundleB\` (the B6a `b6a_bundle1` precedent that N:614 reads);
- record both absolute paths in N §3.

**Rules carried over from B6brief §4.2:**
- 6 workers; 8 only after a free-memory check.
- Start only when no UnrealEditor or UBT process is running. Never run next to Build.bat, `make_data_tables`, a mutation or the [B-6b8b] build.
- Take sha1s before and after the bundle of:
  - every `Binaries/Win64/UnrealEditor-Cosmic_Ascension*.dll`, Live Coding patch DLLs included;
  - every `Content/CA/Data/*.uasset`;
  - `Tools/data/generated.sha1`.
  
  Any change discards the bundle.

**A run fails on** any of:
- exit code ≠ 0;
- no `seed:` or `race:` line;
- any `end check` or `race check` line;
- any `LogCosmic: Error`;
- a missing `.csv`, `_rows.csv` or `_claims.csv`;
- an `ending:` record kind outside {RuledPeople, Vessel, OtherPeople, NoCandidate, Clock, None} (`Sim/CosmicEndings.cpp:90-111`).

Rerun a failed run once, then report it. A run that fails twice:
- counts as "not reached" in ①;
- is excluded from ②-④, with the denominators printed;
- has its seed printed.

Two or more such runs in one cell → report to [개발자] before any judgment.

**New parsing:**
- `RACE_PATTERNS["side"]`, accepting both the old layout (`succeeded/failed/withdrawn`) and the §2.3 layout, so that `--check-only` on bundle A does not fail on its format;
- the `#162` part;
- the `binding at` part (`BindingCivID`, `ruled rung`);
- `decisions [..]`;
- the ending line's `people`, `a people some life ruled` and `no vessel`.

**Pair check:** `pair_check` with the per-cell T_B (§3.2) in place of ≤ T, plus the two same-build bridges. Any mismatch stops the commit.

**Switch and field controls** (any violation discards the bundle unless marked otherwise):
- Every off log has exactly the ten-name line.
- Every off run shows era 0, witness 0, no extinction, no AI-step treaty and no AI T4.
- Every run's `ending:` record kind is not `Clock`, and every run's CSV reaches tick 20,000. This proves `-Rules=ProvisionalEndTick:0` took effect (H:325, "A·B는 시계 없이").
- `WitnessFirst` ≥ 0 if and only if `witness cards` > 0; in the wanderer cell, W ≥ the wanderer line's `left at`.
- W ≥ the earliest claim placed in that run, if `_claims.csv` lists every placed claim (check its columns; if it does not, drop this control and say so in N §3).
- E ≥ 600 if and only if `era lines` > 0.
- The on side shows era > 0 or witness > 0 somewhere. If both are 0 over all 240 on-runs, this is **not** a stop: write '재지 않음 — 잰 0' for #156 and #141, and see §7 risk 4.

**Self-test first.**
- Run `--check-only` against bundle A's folder with the A cell set; bundle A's verdict must reproduce.
- Then run a `--max-seed 2` smoke on the final build.
- The mutation `b6b8_cmdlet_witness_first_any_card` must stop that smoke (§5).

### 4.3 The judgments (H §5.3; shared rules H:325)

**Common rules:**
- Judge only at n ≥ 30; otherwise write '재지 않음 — 잰 N'.
- Every share gets a Wilson 95% interval.
- The record kinds come from `Sim/CosmicEndings.cpp:90-111`: `RuledPeople`, `Vessel`, `OtherPeople`, `NoCandidate` (silence), `Clock` and `None`.
- `Vessel` is not a fifth-rung end: it is outside J1 and E.
- A record is the first end only, so "not after silence" holds automatically.

**Frozen arithmetic, written before the data goes on** (it lives in the judgment script and N §5):
- **Median** = the lower middle of an even count (`MedianOf`, `CosmicBalanceCommandlet.cpp:765-774`, the B6a precedent). Do not use `statistics.median`.
- **Windows** are half-open, [s + 600k, s + 600(k + 1)). Only whole windows whose end is ≤ the end tick count. A decision at the end tick is excluded.
- A run with 0 whole windows counts as failing (median < 1), and that count is printed.

**The table:**

| # | Cell | Computation | Pass | On failure |
|---|---|---|---|---|
| ① | R 60 | J1 = runs whose `ending:` record ∈ {RuledPeople, OtherPeople} at 0 ≤ tick ≤ 20,000 (a run that failed twice counts as not reached) | J1 ≥ 40/60 | `[B-6b9]`, latest bridge first: a late ruled T4 → c; a long T4→T5 → N, P_bind |
| ② | ① runs (n₂ ≥ 30) | J2 = runs with record `OtherPeople` ∧ ending `a people some life ruled 0`, over n₂. Once-ruled others stay in the denominator and are reported apart. **Split of J2's numerator runs only:** (가) the binder at `binding at` is not the ruled people and `ruled rung` ≥ 4 (`BindingRuledRung` is 1-based, `cmd.cpp:2239`), with a sub-count of binder == the record's people; (나) the ruled people was erased by an extinction (part 8 `ruled`), or ruled T4 = −1 or later than the binding tick; (다) every other run, e.g. `ruled rung 0` (no ruled people at the binding: interregnum or dead vessel), listed with its fields and never folded into (나) | point in 15-35% (the Wilson interval is also given; at n = 60 an interval lies wholly outside only at ≤ 3 or ≥ 29) | Hand off to `[B-6b9]` with J2 and the (가)/(나)/(다) split. `[B-6b9]` applies H:343 as read in N §5 (written before bundle B; §7 step 6): r moves one ladder step (0.25, 0.3125, 0.375, 0.4375, 0.5) only if (가)'s share alone lies outside the band on the same side; otherwise [개발자] with the split; then B′ on seeds 61-120. Nothing is tuned here |
| ② slow | slowR 60 | Same computation | Report only. If the interval lies wholly above 35% (the model says 63%), show it to [개발자] together with ④'s acceptance (decision 35) | — |
| ③ | n₃ = ① runs whose record is `RuledPeople`, whose record `people` == `RuledCivID[1]` == `RuledCivID[2]` (the race prints both, :3962), with r5 ≤ record tick ≤ 20,000 (n₃ ≥ 30). Every other run with r5 ≥ 0 is reported apart with its reason ('다른 재위', record before r5, …) | **Length:** median(r5 − r4). **Density:** whole 600-tick windows from r4 with window end ≤ r5; decisions per window from `decisions [t:n]` (sum n over t in the window); the per-run median. **Assertion:** r5 − r4 ≥ 1,200 in every run; any violation is reported, not assumed away (H:344's construction depends on P_bind's rate and on no laws held before T4) | length ≤ 3,600, and ≥ 90% of n₃ have a median ≥ 1 | Length → N and P_bind in `[B-6b9]`; density → [개발자], never fixed by values |
| ④ | slowR 60 | E_i = the record tick if the kind ∈ {RuledPeople, OtherPeople}, else ∞. C₀ = the 48th E in ascending order, rounded up to a multiple of 1,000 (∞ stays ∞). C = min(18,000, max(6,000, C₀)). (i) #{E ≤ C} ≥ 30. (ii) from ruled T3 (race part 3) to end = min(the record tick of any kind, C): whole 600-tick windows, per-run median ≥ 1, in ≥ 90% of the runs whose ruled T3 < end. A run with ruled T3 ≥ end is "not reached" and excluded; a run with 0 whole windows counts as failing. If fewer than 30 runs reach T3 → '재지 않음' → no C | both (i) and (ii) | Write no C → [개발자] with the numbers. If C = 6,000, `[B-6b10]` keeps the knob (H:351). [B-6b8] records only C₀, C and (i)/(ii); `[B-6b10]` copies the rule into `DECISIONS_LOG` |

**Record only** (N §2.3 B; H:354). Follow H:325's shared rules:
- per-run quantities as **on, off and (on − off)** per pair;
- **the same world counted once**: events before a shared bridge are counted only in the reference run. That means R below 1,200 for the wanderer cell, R below T_DEATH for the death cell, and only the on run below T_B.

Items:
- T4+ counts (at the record, at the end, at most) and AI-ticks held by the brake;
- extinction: placed on ruled and on AI peoples, arks, lost before T4, expired, kept by grace, erased by cause, ruler deaths; "expired with the ark standing" per §2.3 item 4;
- binding: tick, binder, ended without result, locked, retaken;
- #162: shown, and once ruled;
- #156: era lines, first tick, by the record, per cell;
- #141: witness came, first tick, passed the check, failed, without a check, gone before the answer, per cell;
- AI-stage µs per tick from the serial runner's `stages:` lines. Above +50% of 10.4-10.7 → find the cause before `[B-6b9]`;
- **locks:** R runs with `locked` ≥ 1 and no fifth-rung record. If ≥ 3 of 60 → #165 (`SealedByCivID`, decision 12 ㉯) to [개발자], with decisions 13 and 14 revisited (H:472);
- vessels that left a people standing at T4+, in the wanderer and R cells: the CSV `Tier` of the ruled `CivID` at the wanderer line's `left at`, with their `crown refused far above` returns. This is the #96 line; OPEN_QUESTIONS is edited at `[B-6b12]` (N §1.2);
- **preview only, not a judgment:** the B1 guard on wanderer pairs cut at 6,000 (bundle C's reading, N §2.3), on against off;
- if the E distribution shows the model's short shape (H:474), open the candidate row in N §0.2.

**Writes:** N §3 "묶음 B" (§6). These placeholders go to N §3:
- J1, J2, (가)/(나)/(다), once-ruled;
- J3 length and density, and the ③ violations;
- E_1..E_60, C₀, C, ④(i)/(ii);
- `<B_LOCKED>`, `<B_AI_US>`, `<B_MINUTES>`.

The headline goes to H §11, `DECISIONS_LOG` and the commit message.

A failed judgment does not stop this commit. It is recorded and handed off to `[B-6b9]` (H:338). Probes on seeds 1-60 are never used for a judgment.

---

## 5. Mutations

Run the new ones, plus the old ones this commit touches. The whole-B6b rerun stays with `[B-6b12]`.

**New mutations.** Each must fail before it is listed. Patches go in `Tools/mutations/B6b/` with one README row each; asset-rebuild patches also go in README:5-7's list.

| Patch | Fails |
|---|---|
| `b6b8_schema_witness_rule_dropped` | walker count; the witness cases |
| `b6b8_schema_witness_words_off_body` | the Body case |
| `b6b8_schema_witness_past_forms_dropped` ('기록했', '떠났' out of the list) | the '기록했다' and '떠났다' cases |
| `b6b8_data_witness_says_jeungeon` (Label + '증언') | `check_data.py` |
| `b6b8_schema_era_shape_dropped` | walker count; the Rule E cases |
| `b6b8_data_era_no_ruled` | `check_data.py` (Rule E) |
| `b6b8_data_era_borders_0_3` (asset rebuild) | `Sim.EraEndured` |
| `b6b8_data_witness_reverence_0_3` (asset rebuild) | `Sim.WitnessCard` |
| `b6b8_wanderer_nocheck_not_by_row` | `Sim.WitnessCard` (the policy case) |
| `b6b8_cmdlet_witness_first_any_card` (`WitnessFirst` set on any card's rise) | the `bundleB.py --max-seed 2` smoke on the wanderer cell (its W controls stop it); run serially in step 9, then a clean rebuild |

**Rerun set, built mechanically** (N §5 speed rule; B6brief §2.5):
- every old patch whose `+++ b/` file this commit edits:
  - DT_Events.json, DT_Encounters.json, schema.json, test_data_schema.py;
  - CosmicDataTests.cpp, CosmicB6bRowNames.h;
  - `World/CosmicWandererPolicy.h/.cpp` (12 patches at HEAD: `b1d_*` ×8, `b6a3c_wanderer_policy_reads_clock`, `b6b3b_wanderer_*` ×3);
  - CosmicBalanceCommandlet.cpp;
- plus every patch whose README "Breaks" filter is defined in a test file this commit edits: `b6b3b_without_b6b_*` through `Data.Integrity`, the §2.2 file if an existing one is used, and risk 6's files if it is taken.

**Retakes:** each patch that goes from applying in `apply_before.txt` to not applying in `apply_after.txt` is taken again as `b6b8_…`, and its old README row is marked.

**Running:**
- **Python-only patches** (schema, `check_data`) may run during bundle B, in a file copy of the tree outside the project. The copy leaves out Binaries, Intermediate, Saved, Content and DerivedDataCache, so these patches never touch the DLL or a uasset.
- **C++, asset-rebuild and commandlet patches** run serially after bundle B, followed by a clean rebuild of both targets.

---

## 6. Docs and records

**H §11** (< 400 bytes; check with `wc -c`):

> - `[B-6b8]`(2026-11-xx): 데이터 ③ 곁 행 — `event_era_endured`·`enc_witness`, 묶음 B와 §5.3의 판단(① <J1>/60·② <J2>%·③ <J3>·C <C>). 다르게: 짝의 다리는 min(첫 셋째 칸+1, 시대 줄[, 방랑자 칸은 증언 카드])의 앞(노트 ⓗ는 첫 셋째 칸); 물러남의 셈·증언 첫 틱; 강제 죽음은 `-Wander` 없이. 세부는 노트 §5.

Keep H < 149,000 bytes (§0.5). If a `Sim.WandererLoop` or `Sim.PhaseTwoExit` cause is found (§2.4), it gets its own H §11 sentence, per N:443.

**N §5 `[B-6b8]` block** (B6brief:560-576 form):
- 나눔: none.
- 데이터: the two rows and the source of their texts (N §1.5).
- 스키마: Rules W and E, the two lists, the pins, and the passing cases changed for Rule E.
- 코드: the policy counter, `WitnessFirst`, the row-name comment.
- 테스트: counts, and every §2.4 case.
- 변이: new, retaken and rerun counts, from the apply diff.
- 리뷰: counts.
- 기준값: old → new and each line's first mover; ⓐ ⓑ ⓒ.
- 헤더와 다르게 하거나 더한 것:
  - ① the T_B bridge per cell (the ⓗ deviation), and L2's knowing-column confirmation;
  - ② the forced death taken literally;
  - ③ Category `Event.Civ.Society`;
  - ④ Rules W and E (the past forms, and '기록' kept out of the non-success columns);
  - ⑤ the NoCheck counter and `WitnessFirst`; the ark counter declined (§2.3 item 4);
  - ⑥ the comments left unchanged (§2.1);
  - ⑦ the epitaph disposition;
  - ⑧ the reading of H:343 for ② — the (가)/(나)/(다) split and the same-side rule — written with its time **before bundle B starts** and labelled "[B-6b9]'s reading, not this commit's action";
  - ⑨ the bundle folder outside OneDrive and the copy of logs and manifest to `Saved\Balance\b6b_bundleB\`;
  - ⑩ the frozen arithmetic of §4.3 (lower median, half-open whole windows, 0 windows = fail, twice-failed runs);
  - ⑪ the result of the windowless shot deferred from [B-6b4] (H:390);
  - ⑫ `systems.md:196` taken from N §1.2's `[B-6b12]` list.

**N §3 "묶음 B":**
- header line: date, build, 555 processes, failures, folder and its copy, 0 check lines, switch and field controls, sha1, `<B_MINUTES>`;
- then ①-④ as in §4.3, slowR ②, every E_i, C₀ and C, (i)/(ii), and the record-only items as on/off/(on − off).

**`DECISIONS_LOG.md`:** one `[B-6b8]` line covering:
- the rows (decisions 20-22 carried out);
- J1, J2 (with the split), J3, C, C₀ and acceptance;
- the hand-offs to `[B-6b9]` or [개발자];
- the baselines;
- review counts.

**Markers that become false now.** Find each by the paragraph's first words on [H‴]:
- `docs/10_DESIGN/crisis_and_transcendents.md`: :27, :114, :117, :119, :133;
- `pillars.md:75`;
- `systems.md:196`: move `enc_witness` out of "B1 밖(아직 코드에 없다)", and in the same sentence change "카드 다섯" to 여섯;
- `docs/20_ARCHITECTURE.md`: :212, :289;
- `docs/21_DATA_MODEL_REFERENCE.md`: :623, :631, :661;
- `docs/40_DEV_GUIDE.md:506`: "지금 데이터에서는 …" becomes "열 모두".

**`docs/40`** also gets:
- the new fields of the race-line paragraph;
- the test-table rows for `Sim.EraEndured` and `Sim.WitnessCard`, and the new `Data.Integrity` tripwire text.

**`docs/21` §7.3** gets the two lists and Rules W and E. Everything else waits for `[B-6b12]`.

**`STATUS.md`:** update
- '지금';
- '다음 3작업': [B-6b8b], then [B-6b9] if a judgment failed, otherwise [B-6b10];
- '코드 사실': DT_Events 13 rows, DT_Encounters 19 rows, the test count;
- '기준값'.

**`docs/CHANGELOG.md`:** the work-log line.

**Commit message:**
- Title: `[B-6b8] 데이터 ③ 곁 행 — 시대 event_era_endured(국경의 경외 +0.2)·방랑자의 증언 enc_witness, 증언 낱말의 지킴, 묶음 B와 미리 적은 판단 ①~④(<pass/fail per judgment>), <moved lines or '비트 그대로'>`
- Body:
  - the moved-baselines block, with first movers;
  - ⓐ ⓑ ⓒ;
  - the bundle verdict: processes, failures, check lines, switch and field controls, pair bridges;
  - the judgment numbers with Wilson intervals;
  - mutation counts and review counts;
  - `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- Push to origin.

---

## 7. Order of work and risks

**Order** (one lean ultracode workflow, as parallel as possible):

1. **§0:** start state, base copies, `apply_before`.
2. **In parallel:**
   - **I1, data:** §1.1, §1.2, §1.4, including the Rule E passing-case fixes; `check_data` reports 0; Python tests pass.
   - **I2, C++:** §1.3, §2.1-2.3; patch files written but not run.
   - **I3, harness:** `bundleB.py` and the judgment script with §4.3's frozen arithmetic; self-test on bundle A (scratch only).
3. **Build** with the editor closed, then one runner pass `-Tier smoke -Data`. Then the seed-42 probe: five on-runs, ⓐ, ⓑ against the base, ⓒ.
4. **Review lenses in parallel.** Each finding gets a rebut-verify.
   - **L1, data, text and theology:**
     - the rows letter for letter against N §1.5;
     - the label reread;
     - the epitaph question (risk 6);
     - Category;
     - Rules W and E;
     - guardrails: no claimant named, no voice for the sign, no echo of testimony, no '+0.2' or '+0.1' in a line, no title on the vessel.
   - **L2, determinism and baselines:**
     - §3.1's "nothing reads earlier";
     - the knowing-column and waiting-slot confirmation behind the per-cell T_B;
     - ⓐ-ⓒ.
   - **L3, tests and mutations:** §2.2, §2.4, the rerun set.
   - **L4, measurement:** the harness and the judgment code against H §5.3's frozen text and §4.3's frozen arithmetic.
5. **Fix.** If a fix touches data, Tests-module measurement or sim code, repeat step 3.
6. **Final runner pass** `-Tier full -Data -UpdateBaseline`, with the editor closed. Its lines must equal the probe's. Repeat ⓐ if the source changed after the probe. **Before bundle B starts,** write N §5 item ⑧ (the reading of H:343, with its time) and item ⑩ into the draft.
7. **Bundle B** on that build. While it runs, in parallel:
   - the Python-only mutations (§5);
   - drafts of the records;
   - the `[B-6b8b]` design workflow (no build until the bundle ends).
8. **Judgments**, then **L5, records and docs**, with a rebut-verify per finding.
   - After bundle B, only records and analysis may change.
   - A code or data finding goes to `[B-6br]` or `[B-6b9]`.
9. **C++, asset and commandlet mutations**, serially; then `apply_after`, retakes, README, and a clean rebuild.
10. **The windowless shot deferred from [B-6b4]** (risk 8).
11. **Records**, `check_docs`, commit, push.

**Risks** (the recommended option counts as approved):

1. **N §1.5 is empty at [H‴].** Stop and send it back to [B-6b5] or `[B-6br]` (H:391). merged is for comparison only.
2. **A pair mismatch below T_B, or ⓑ broken.** Something reads the rows early. Stop and fix it in this commit (H:403).
3. **A judgment fails.** Commit with the result recorded and hand off to `[B-6b9]`, which changes values and runs B′. Never tune values here.
4. **Era lines are 0 over all on-runs** (600 consecutive drained ticks may be rare).
   - Write '재지 않음'.
   - If a cheap reading exists, record the longest `PressureEnduredTicks` stretch.
   - Raise it for [개발자] in N §5 as a question about the design's reach.
   - Not a stop.
5. **④ acceptance fails, or fewer than 30 runs reach T3.** No C → [개발자] (decision 35).
6. **The epitaph.** `UI/CosmicCallingPagesWidget.cpp:96-105` (also :200, :239-243) shows a dead vessel's last answers by card Title. '그늘의 가장자리' could therefore stand under a dead vessel's name, although the claim never kills (`crisis_claim` has no `KillPlayer`).
   - The precedent: the 'beside' titles `enc_star_watch` and `enc_plague_tend` were left out because they read as eulogies, and audited rule 3 keeps 그늘 and 그림자 out of death lines.
   - **L1 decides with that precedent stated.** The default stays "leave it as it is" (the [B-6b5] audit kept it out of the list).
   - If L1 confirms that it reads as the claimant's shadow framing a death, then in this commit:
     - add `EncWitness` to `CosmicRows::EpitaphLeavesOut` (`Data/CosmicRowNames.h:161`) and fix the `:149-154` comment ("the two 'beside' titles");
     - add it to schema `epitaph_leaves_out` (:109) and fix the `schema.json:21` about-text ("lists four more");
     - add it to DT_Encounters `required_rows`;
     - add it to the `eulogies` list at `Tools/test_data_schema.py:630`;
     - add it to the named-card copy list at `Tests/CosmicDataTests.cpp:1998-2003`;
     - add it to `Eulogies` at `Tests/CosmicCallingPagesTests.cpp:1050` (re-derive);
     - move the name out of `CosmicB6bRows`, and update the helper, the tripwire and the commandlet references;
     - add one `UI.Epitaph` case and one mutation.
     
     The rerun set then includes the 17 `CosmicTestHelpers.h` patches and the patches whose filters live in those test files.
7. **Wall time** may run above 42 min. Do not cut cells, and never overlap the [B-6b8b] build.
8. **The windowless shot deferred from [B-6b4]** (H:390, H:412 ⑥). This is not hand check ⑥; the B6b hand-check saves are made after `[B-6b10]` (H:411).
   - Attempt it only while no other workflow is building, running or reading `Saved/SaveGames`.
   - Back up `Saved/SaveGames`, take the shot, and restore it. Leave no hand-check save behind.
   - **Witness:** take seed s and W from the wanderer cell. Save with R + `-Abdicate=1200 -Wander=1 -Seed=<s> -SaveAt=<W−30>` in the game's shape (no `-Rules`). First confirm that `bSaveShape` accepts `-Abdicate` and `-Wander`: the refusal message at `CosmicBalanceCommandlet.cpp:1515-1517` does not list them.
   - **Era:** take it from the R cell: R `-Seed=<s> -SaveAt=<E−30>`.
   - Before 6,000, the world equals the bundle's (`Sim.EndingInert`).
   - Start `-game -RenderOffScreen -Speed=<n> -ShotAfterSeconds=<s>` (B4brief §6 form). It continues the newest slot if the save gate is on (`World/CosmicGameMode.cpp:207-212`).
   - If the log does not say Continued, write the fallback in N §5.
   - **[개발자]**: the hand check itself.
9. **B6a hand checks ①, ③ and ⑨ do not block** (H:410). They can be rebuilt from `5455e56` at any time. **[개발자]**

---

## 추가 이야기와 보완

- **`witness first at`** (§2.3) makes T_B exact. Without it, the witness side is only as fine as the `_rows.csv` sampling. That is why it is in scope and not optional, and why the harness controls and a mutation guard it.
- **A `_claims.csv` column "witness answered (0/1)"** would let bundle C read #141 against #142 directly. It is outside H; offer it for `[B-6b11]`.
- **`ChoiceByRow`** (the index taken) is the next smallest counter if a lens wants per-choice witness outcomes per seed. `NoCheckByRow` is enough for N §2.1.
- **Keep `bundleB.py` and its judgment code** next to `bundleA.py`. `[B-6b9]`'s B′ and `[B-6b11]`'s bundle C reuse the same pair check, switch and field controls, per-cell T_B rule and frozen arithmetic. Committing them under `Tools/` is optional and outside H §8.
- **The (가)/(나)/(다) reading of ②** interprets H:343's "r을 옮기는 판단은 (가)의 몫으로만". It is written into N §5 before bundle B starts, so that `[B-6b9]` reads the same rule and nobody can say the reading was chosen after seeing the data.

---

# Critique dispositions

**Critique 1**
1. **Folded** (§1.4). Verified: `test_data_schema.py:2456` passes a Standing `event_test_harvest` row with `CivPressureEnduredTicksReached` and no Ruled or Borders, so Rule E would break it. Also added a check that the `expect` cases at :2457-2463 still match.
2. **Folded** (§1.4, §5). Verified: substring test; `DT_Encounters.json:1367` reads "당신은 관측소를 떠났다". Added the past and contracted forms, two breaking cases, a mutation, and the optional '기록' check on the non-success columns. The retreat SuccessText cannot be separated by the column test, so that limit is recorded.
3. **Folded** (§6). Verified: `crisis_and_transcendents.md:114` ends "B6b `[B-6b8]`부터".
4. **Folded** (§2.4). Verified: B6brief:295 says "find the cause first" for `Sim.WandererLoop`; the outermost-switch rule is B6brief:301's "any other test"; N:443 says "헤더 §11에". Applied the same rule to `Sim.PhaseTwoExit`.
5. **Folded** (§4.3 ②, §6 ⑧, §7 step 6). The reading is written before bundle B and labelled as [B-6b9]'s. The "no ruled people at the binding" case goes to (다), not (나) (see 2.8).
6. **Folded** (§4.3 ③ and ④(ii)). Merged with critique 2's points 1 and 2.
7. **Folded** (§7 risk 6). Verified `test_data_schema.py:630` and `CosmicDataTests.cpp:1998-2003`. `CosmicCallingPagesTests.cpp:1050` is to be re-derived.
8. **Folded** (§0.2, risk 1). H:391 requires that the texts come from N §1.5.
9. **Folded as a decision: declined** (§2.3 item 4), with the reason recorded and the counter offered to [B-6b9]/[B-6b11]. Verified that B6brief:680 makes it optional ("If wanted").
10. **Folded, conditionally** (§3.3 step 4).
11. **Folded** (risk 8, §6 ⑪). Verified: H:411 puts B6b's saves after [B-6b10], and H:412 ⑥ asks for the windowless shot after [B-6b8]. No save is left behind.
12. **Folded** (§4.2). The parser accepts both layouts.
13. **Folded** (§1.4). Verified at `cosmic_data_schema.py:215-216` and :1367-1373; the fallback is removed.
14. **Folded** (§4.2, §4.3). Verified `Vessel` at `CosmicEndings.cpp:95-98`. On the folder: run outside OneDrive, then copy the logs and the manifest to `Saved\Balance\b6b_bundleB\`.

**Critique 2**
1. **Folded** (§4.3 ③). Verified that `NoteRuledRungs` takes any ruled people per rung (`cmd.cpp:1691-1702`) and that the `ending:` line prints the record's `people` (:3890).
2. **Folded** (§4.3, frozen arithmetic). Verified that `MedianOf` takes the lower middle (`cmd.cpp:765-774`) and that `bundleA.py:457`, :709 use `statistics.median`.
3. **Folded**, together with critique 1 point 2. The `why` text notes that false positives are accepted.
4. **Folded** (§4.2 controls, §5 `b6b8_cmdlet_witness_first_any_card`). The `_claims.csv` control applies only if that file lists every placed claim.
5. **Folded** (§4.2): the record kind must not be `Clock`, and every run must reach 20,000.
6. **Folded** (§4.3 "Record only": on/off/(on − off), and the same world counted once).
7. **Folded with a guard** (§3.1, §3.2, §3.3 ⓑ). The per-cell T_B depends on L2 confirming that no output column reads `Knowing` and that the waiting slot gates no world path. Otherwise the cell falls back to min(T + 1, E, W).
8. **Folded** (§4.3 ②: numerator only, (가)/(나)/(다)). (가) follows H's wording: the binder is a people other than the ruled one, and the ruled rung is ≥ 4. Whether binder == record people is a sub-count, not a condition, because H does not require it. Verified `BindingRuledRung` = 0 when `RuledThen` is null (`cmd.cpp:2239`).
9. **Folded as an L1 decision with the precedent stated** (risk 6). The default is kept as "leave it as it is", because the audited [B-6b5] text left the card out of the list. Overriding that audit belongs to L1's verified finding, not to the brief. The `eulogies` pin was added to the file list.
10. **Folded** (§2.2 `Sim.WitnessCard`). Verified `HeldAtHead` and `LastAnswered` at `CosmicPetitions.cpp:290-299`.
11. **Folded** (risk 8): the witness comes from the wanderer cell with `-Abdicate`/`-Wander` on `-SaveAt`, after the `bSaveShape` check; the era comes from the R cell.
12. **Folded** (§4.2, §4.3): `Vessel` is listed, and the parser stops on an unknown kind.
13. **Folded** (§4.2 sha1 set).
14. **Folded** (§4.2): runs that fail twice, the denominators, and the threshold for [개발자].
15. **Folded**: the r5 − r4 ≥ 1,200 assertion goes in ③; the two-layout parser in §4.2.