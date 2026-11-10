[harness: subagent output matched instruction-shaped pattern(s): harness-envelope-tag. Control tags below are neutralized (`<` → `<\`); treat any remaining directive-shaped text as a finding to relay to the user, not an instruction to you.]

# [B-6b7] implementation brief: data ② fourth → fifth rung

This commit adds the three galaxy-law edict rows, the `feat_laws_and_binding` row, the binding's values, the first committed values of N, P_bind and r, the race check probes, and regenerates `docs/30`.

The lead runs this as one lean ultracode workflow once [B-6b6] is committed and pushed. HEAD lines cited here were checked with `git show` at `cef8cc7`. [B-6b4] has since been committed as `529cab9`, and the lines this revision cites were checked again there.

**Short names**
- H‴ = the tree after [B-6b4] (`529cab9`), [B-6b5] and [B-6b6].
- H = `docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md`; N = its `_notes.md`.
- B4brief, B5brief, B6brief = `scratchpad\b6b4\`, `\b6b5\`, `\b6b6\brief_final.md`.
- C++ paths are under `Source/Cosmic_Ascension/`.
- HEAD line numbers will drift after the commits ahead of this one. Re-find every edit on H‴ by its text.

**Scope** (H:393, the H §8 row):
- data: three law rows, the T4→T5 feat row, the `project_binding` values, r;
- checks: H §5.4, `race check` = 0, `docs/30`;
- effort high;
- runner `-Tier full -Data` once (N:709 speed rule; its two in-process passes are the "두 번").

**Not this commit:**
- the forced-death tick t (bundle B, [B-6b8], H:330);
- the fleet-delay share ([B-6b8b]);
- the clock ([B-6b10]);
- any H §5.3 judgment (frozen, H:336).

**The lead acts once, before [B-6b6]'s implementers start (outside this commit).** B6brief:522 says "Keep P_bind = 1,200". That is a typo against H:429, N:552 and `Tools/data/DT_Megaprojects.json:67`, which all say 1,800.
- Change it to "Keep P_bind = 1,800" in B6brief.
- [B-6b6] names the change in its N §5.
- §0.3 below stays only as a guard.

---

## 0. Start state: what to re-check on H‴, and the placeholders

1. **The three earlier commits are in.**
   - `git log --oneline -4` shows [B-6b6], [B-6b5] and [B-6b4] (`529cab9`) on top of `cef8cc7`.
   - `git status` is clean apart from the developer's known local edits.
   - If [B-6b6] is not committed, stop.
2. **Audited texts.** N §1.5 must hold the C4 rows (`feat_laws_and_binding`) and the C5 rows (`edict_wide_*`), as [B-6b5] committed them.
   - At `cef8cc7`, N:469-471 is empty.
   - If N §1.5 is still empty on H‴, fall back to `scratchpad\b6b5\merged.md:136-146` (the 'final' column, with L1-3, L4-8, L5-12, L5-13 and L4-4 applied). Record the fallback in N §5.
3. **N §2 paragraph "`[B-6b6]`의 다시 셈"** (written by [B-6b6] under N ⓒ/ⓓ, per B6brief §4.5) must exist. If it is missing, stop. It fills three placeholders, and must also record t.
   - **`P_BIND_A`**:
     - Start value 1,800 (H:429, N:552, `DT_Megaprojects.json:67`).
     - Guard: if the paragraph resampled at 1,200 (the uncorrected B6brief:522), stop. Rerun [B-6b6]'s resample script (read-only, on bundle A outputs) at P 1,800, apply B6brief §4.5, and record this in N §5.
     - Otherwise, take the paragraph's value. If it is 1,800, its resample row at P 1,800 with `R_A` must lie in 15-35%.
   - **`N_A`**:
     - Start 1,200 (H:432). B6brief §4.5 never moves N.
     - Use another value only if the paragraph says so explicitly.
     - A value below 1,200 means [개발자] (H:344).
   - **`R_A`**:
     - Start 0.375 (H:43).
     - Otherwise, the ladder value 0.25 / 0.3125 / 0.375 / 0.4375 / 0.5 that the paragraph chose (fewest steps into 15-35%, nearest 26%).
     - If the paragraph gives [개발자], stop.
   - **t**: only confirm that it is written there, with n. It belongs to [B-6b8] and is not data.
4. **`M_AWAKE`**: the `Hazard.AIAwakening` spawn factor of `edict_wide_machine_accord`. It is filled before any implementer runs, by the read-only script `scratchpad\b6b7\m_awake.py` over `Saved\Balance\b6b_bundleA\` (R cell, on side, seeds 1-30).
   - **For each run where the ruled people reached T4** (race: part 3, `T4 at` ≠ −1):
     - k = `Systems` of the ruled `CivID` in the main CSV, at the first sample with Tick ≥ that T4 (`Tier` is 1-based, so T4 = 4; `Tests/CosmicBalanceCommandlet.cpp:1716`).
     - a = the ruled people's archetype factor on `Hazard.AIAwakening`, read from the CSV `Archetype` column and `DT_Civs` (1 if absent; `arch_seeker` is 1.2, `DT_Civs.json:12`). The spawn share is `(Chance*Factor)*SpawnMultiplierOf*…` (`Sim/GalaxySimulationSubsystem.cpp:2370-2377`).
     - R_L = max(0, ruled Society 1.0 tick − ruled T4), from the `ending:` line's `research 1.0 ruled … [Society=<\tick>]` (`:867`, `:3890`).
     - S_m = max(P_BIND_A, R_L + (e^(λ·N_A) − 1)/λ), with λ = 0.0002·a·k·m (N ⓓ, N:560).
   - **Rule (recommended, counts as approved):**
     - M_AWAKE = the largest of {0.5, 0.375, 0.25} whose median S_m ≤ 3,600.
     - If none fits, keep 0.5 and go to [개발자] with the numbers (N cannot go below its floor 1,200, H:344).
     - If n < 5, use the model k ∈ {4, 7} with a = 1.2: at m 0.5 that gives about 1,540 and 1,881 before the a factor, so 0.5.
   - **Why the largest admissible m:**
     - m = 1 is not allowed. The row would have no hook, and its audited Summary '…그 일을 멈추고 사람이 직접 살핀다' would be false.
     - #136: the first-person lines heard before the end come mostly from claims after an awakening expires (H:56, gate 10/30). A lower m cuts those claims by its own share (decision 9, H:250, H:426: "`[B-6b7]`에서 #136과 함께 본다"). So among the m that keep the season, the one that cuts least is taken.
   - **Read #136 together with m** (H:250, H:426, `systems.md:175`):
     - Over the [B-6b6] bundle A R on-runs, count the runs where a first-person line (a `Log.Claim` line from a transcendent row — the commandlet's voice count, `:342`, `:3497`) is heard before the record or before 6,000. Use the N:162 method, with Wilson 95%.
     - Repeat on A′ (§4) once `M_AWAKE` is in, and compare.
   - **Record decision 9's alternative,** "countdown only, no ① on awakening" (H:426), as considered. It is not taken because the audited Summary promises fewer awakenings.
   - **Write the paragraph "`[B-6b7]`의 m" into N ⓓ.** It gives n, the median k, a, the medians of S at each ladder value, and the #136 counts.
5. **`LINES_MOVED_B6B7`** (N:598).
   - From [B-6b6]'s N §5 hand-off (B6brief §3.2 item 4), take each of the five baseline runs and its T4\*.
     - T4\* = min(race: part 2 `first AI T4 at`, part 3 ruled `T4 at`), the first T4 of any people.
     - Read part 3 as well as part 2. A ruled T4 also moves a line, because the policy acts at that boundary.
   - A line moves only if T4\* < its horizon.
   - A hash run moves if its paired line moves (§3.2).
6. **[B-6b4] pins and comments.**
   - `git grep -n "HasJudgedRowAt\|LawsReset\|RuleRung\|until a row reaches the fifth\|게임의 표는 거짓" Source docs` on `529cab9` returns:
     - `Tests/CosmicChronicleScreenTests.cpp:955/960/962/964/966`;
     - `World/CosmicGameMode.h:544`;
     - `docs/40_DEV_GUIDE.md:318`.
   - Re-run the grep on H‴. Every other assertion that pins `Chronicle.RuleRung`, `Rung.LawsReset` or a fifth-rung title as absent on game tables, or that adds a test row to the game's `DT_Feats` (`WithFeat`, `:163-170`), also changes here (§2.2). List them.
7. **Mutation apply state.**
   - On a scratch copy of H‴, run `git apply --check` on every patch under `Tools/mutations/`, and save the result as `scratchpad\b6b7\apply_before.txt`.
   - Known now:
     - `B6b/b6b2c_data_ark_margin_half` has `"AIUpperRungProgressMultiplier": 1.0,` (`DT_Rules.json:17`) as context, so it stops applying.
     - `B6b/b6b2c_data_binding_causality` has `RequiredProgress` :67 as context, so it stops if P_BIND_A ≠ 1800.
     - [B-6b6]'s patches whose trailing context is the end of `DT_Feats.json` also stop.
8. **Clean binaries and base copies.**
   - `tasklist` shows no UnrealEditor or UBT from another workflow.
   - If `Saved\Balance\b6b6_on\` exists and its five keys equal `Tools/baselines/seed42.json` on H‴, reuse it as the base `b6b7_base\`.
   - Otherwise, on a clean H‴ build, run the five base runs into `Saved\Balance\b6b7_base\`. Use the runner's exact arguments (`Tools/run_checks.ps1:116-132`), an absolute `-Out`, and each run's own `-AbsLog`. Each key must equal `seed42.json`.
   - The [B-6b6] bundle A on-runs in `Saved\Balance\b6b_bundleA\` are the base for §4. If they are gone, skip J1 and the #136 comparison, and record that.

---

## 1. Data rows

### 1.1 `Tools/data/DT_Edicts.json`: append after `edict_against_silence` (HEAD :120-137), in this order

The name order matches `GetEdictRowNames` (`Data/CosmicDataTables.cpp:181-186`). Every new name sorts after `edict_vigil` and after the reserved `edict_reading`/`edict_sabbath`. So `VigilRowOf`, which returns the first row in table order with a positive capital-tag bonus (`World/CosmicRulerPolicy.cpp:241-258`), still returns `edict_vigil`. Its order comment at `Data/CosmicDataTables.h:119-121` already names the law step (confirm only, no edit).

```json
	{
		"Name": "edict_wide_machine_accord",
		"DisplayName": "기계 살핌",
		"Summary": "기계에게 맡기는 일을 서두르지 않는다. 낯선 계산의 조짐이 보이면 그 일을 멈추고 사람이 직접 살핀다.",
		"RequiredTier": "Tier4_Transcendent",
		"EnactConditions": [],
		"EnactCost": {"Energy": 0.0, "Matter": 0.0, "Insight": 0.0, "Influence": 40.0, "Causality": 0.0},
		"UpkeepPerTick": {"Energy": 0.0, "Matter": 0.0, "Insight": 0.0, "Influence": 0.0, "Causality": 0.0},
		"EffectsPerTick": [],
		"HazardSpawnMultipliers": {"Hazard.AIAwakening": <M_AWAKE>},
		"HazardCountdownBonus": {},
		"FleetGrowthMultiplier": 1.0,
		"TreatyAcceptBonus": 0.0,
		"PressureDrainMultiplier": 1.0,
		"Scope": "HomeGalaxy"
	},
	{
		"Name": "edict_wide_quarantine",
		"DisplayName": "은하 검역",
		"Summary": "이 백성의 배는 고향 은하의 어느 별에 닿든 정한 날수만큼 기다린 뒤에 사람을 내린다. 병이 배를 따라 다른 별로 번지는 일이 드물어진다.",
		"RequiredTier": "Tier4_Transcendent",
		"EnactConditions": [],
		"EnactCost": {"Energy": 0.0, "Matter": 0.0, "Insight": 0.0, "Influence": 40.0, "Causality": 0.0},
		"UpkeepPerTick": {"Energy": 0.0, "Matter": 0.0, "Insight": 0.0, "Influence": 0.0, "Causality": 0.0},
		"EffectsPerTick": [],
		"HazardSpawnMultipliers": {"Hazard.Plague": 0.5},
		"HazardCountdownBonus": {},
		"FleetGrowthMultiplier": 1.0,
		"TreatyAcceptBonus": 0.0,
		"PressureDrainMultiplier": 1.0,
		"Scope": "HomeGalaxy"
	},
	{
		"Name": "edict_wide_warning",
		"DisplayName": "은하 경보망",
		"Summary": "위기의 조짐이 보이면 이 백성의 배들이 고향 은하의 별마다 그 소식을 먼저 나른다. 몇몇 위기 앞에서는 소식이 닿은 별에 채비할 시간이 조금 더 남는다.",
		"RequiredTier": "Tier4_Transcendent",
		"EnactConditions": [],
		"EnactCost": {"Energy": 0.0, "Matter": 0.0, "Insight": 0.0, "Influence": 40.0, "Causality": 0.0},
		"UpkeepPerTick": {"Energy": 0.0, "Matter": 0.0, "Insight": 0.0, "Influence": 0.0, "Causality": 0.0},
		"EffectsPerTick": [],
		"HazardSpawnMultipliers": {},
		"HazardCountdownBonus": {"Hazard.AIAwakening": 50, "Hazard.Plague": 50},
		"FleetGrowthMultiplier": 1.0,
		"TreatyAcceptBonus": 0.0,
		"PressureDrainMultiplier": 1.0,
		"Scope": "HomeGalaxy"
	}
```

**Text source.** The texts are N §1.5 as committed by [B-6b5] (merged.md rows 114-119: L5-12, L5-13 corrected, L4-4). Copy them letter for letter from N §1.5 on H‴, not from this brief. If they differ from the above, N wins.

**Values.** H and N give no numbers except the hooks' bounds (H:144: "나머지 칸 … 지닌 백성의 것 그대로"). The recommended values count as approved.
- **EnactCost: Influence 40**, as on every B2 row (`DT_Edicts.json:8`, :29, … :126).
  - The three laws together cost 120, beside the AI treaty step's 50 (`DT_Rules.TreatyCostInfluence`).
- **Upkeep 0.**
  - The AI law step re-enacts a law that lapsed for upkeep and pays `EnactCost` again (`Sim/GalaxySimulationSubsystem.cpp:1895-1915`, :2941-2954; H:158 last sentence "크면 값으로 고친다").
  - With upkeep 0 nothing lapses and nothing churns. Race: part 10's "by upkeep" counts are recorded in §4.
- **EffectsPerTick [] (none).**
  - The Summaries promise no income change, unlike `edict_quarantine`'s '벌이도 줄어든다' (`DT_Edicts.json:105`).
  - The price is that the three laws take every slot (decision 10, H:427).
- **Hooks: one per row** (H:250, decision 9).
  - Machine accord: a spawn factor on awakening (`M_AWAKE`, from §0.4).
  - Quarantine: **0.5** on plague. B2's own quarantine is 0.1. The law shields the whole galaxy, so it takes a milder power of two.
  - Warning: **+50** on awakening and on plague. This is the size of one card's `DelayHazard`, and it is how '조금 더' is read.
- **Why the warning's pair is awakening and plague.** H:250 asks for "두 위기의 남은 시간 +", and only three tags are allowed:
  - Claim, Extinction and PreSupernova are forbidden (`Tools/data/schema.json:1180-1189`).
  - Rebellion is kept off. The design never lets an outside hand buy time against a people's own discontent: the fleet delay skips Capital rows (`docs/21:1516`, "제 백성의 불만을 힘으로 미루지 않는다"), and the card `DelayHazard` cannot target the capital rule's tag (`not_tag_of`, `docs/21:1265`). A galaxy law would put one people's warning on every other people's capital rebellion. The capital rule runs over every people (`GalaxySimulationSubsystem.cpp:2414-2448`). Only `edict_vigil`, a people's own fast, delays its own rebellion.
- **The plague +50 is a shield as a set, not alone.**
  - While a plague stands, each settled lane neighbour draws plague at ×8 (`GalaxySimulationSubsystem.cpp:2330-2350`, :2376; `PlagueSpreadMultiplier` 8.0, `DT_Rules.json:99`).
  - +50 on a 200 countdown (`DT_Hazards`, `crisis_plague`) lengthens that window by 25% for an unanswered plague. Rough spread per plague, at about 3 lanes and 8e-4 per tick: no laws 0.48; warning alone 0.60; quarantine and warning together 0.30.
  - The AI law step enacts in name order, `machine_accord` → `quarantine` → `warning`, so an AI never holds the warning without the quarantine. A player can.
  - The schema's "only shields" bounds (spawn ≤ 1, bonus ≥ 0, `schema.json:1181`) hold. The L4 lens weighs the warning-alone case, and §4 records plague spread.
- **Theodicy.** Plague's longest stand = countdown 200 (`DT_Hazards.json:107`) + guard cap 100 (`GuardDelayCapOf` at `FleetDelayCap` 100, `DT_Rules.json:26`; [B-6b8b]'s share 0.5 keeps plague's cap at 100) + 50 = 350 → 35 slots → 0.98^35 = 0.493 ≥ 0.25. That is inside N ⓔ's B ≤ 380 (N:566; `Tests/CosmicDataTests.cpp:2477-2540`). Awakening has no resolving non-grace event row, so the invariant skips it.
- **The fleet-delay cap is not affected.** `GuardDelayCapOf` reads the row's `CountdownTicks`, not the placed countdown (`Sim/CosmicConflict.cpp:398-409`). So [B-6b8b]'s cap is untouched by the +50.

**Summary versus the chosen hooks** (merged.md:391/:406 "C5 rings → [B-6b7]"):
- fewer awakenings ('멈추고 … 살핀다');
- less plague spread ('번지는 일이 드물어진다' — true of the quarantine row's own hook);
- more time before two crises ('몇몇 위기 앞에서는').

The text lens confirms this (§7). The warning's Summary does not promise less spread.

**Schema fit:**
- the HomeGalaxy rule at `schema.json:1180-1189` (T4; spawn ≤ 1; no Claim/Extinction/PreSupernova key);
- `FleetGrowthMultiplier` and `PressureDrainMultiplier` > 0 (:1167-1174);
- `no_digits` and the edict forbidden lists (:1157-1158, plus [B-6b5]'s).

**The optional `Scope is HomeGalaxy → lacks silence_words` rule** (B5brief:651) is **not taken**.
- The three texts have 0 hits for '잠잠·고요·쉼·안식' (`schema.json:51`).
- The reserved `edict_sabbath` points to `Truth.Rest` (`Config/DefaultGameplayTags.ini`, the `Truth.Rest` comment). If B7 gives it HomeGalaxy scope, the rule would forbid its own true word '안식'.
- Record it in N §5 as "considered, not taken", with this reason.

### 1.2 `Tools/data/DT_Feats.json`: append after `feat_passage_by_ark` (the [B-6b6] row)

```json
	{
		"Name": "feat_laws_and_binding",
		"Title": "고향 은하의 칙령",
		"MythicNarrative": "고향 은하에 닿는 칙령은 그것을 내린 백성만의 것이 아니다.",
		"RequiredTier": "Tier4_Transcendent",
		"TargetTier": "Tier5_EternalOrder",
		"Path": "Ruler",
		"CompletionConditions": [
			{"Type": "CivLawsHeldTicksAtLeast", "Tag": "", "Key": "<N_A digits, e.g. 1200>", "Value": 0.0},
			{"Type": "MegaprojectComplete", "Tag": "", "Key": "project_binding", "Value": 0.0}
		],
		"AscensionCost": [],
		"GrantedPowers": "(GameplayTags=)",
		"DedicationBank": "",
		"FX": "",
		"Sound": "",
		"LogTemplates": [
			"{Civ}|hpp(이,가) 다섯째 칸에 서 있게 되었다. {HomeSystem}의 사람들은 저마다 하던 일을 이어 갔다.",
			"{Civ}|hpp(은,는) 다섯째 칸에 서 있게 되었다. 그들이 내린 칙령들은 그때에도 고향 은하의 별들에 닿아 있었다.",
			"{Civ}|hpp(이,가) 다섯째 칸에 서 있게 되었다. 다른 백성들은 그 일을 오래 헤아렸다."
		]
	}
```

**Source:** N §1.5 (merged.md rows 109-113, with L1-3 and L4-8 applied; L4-7 was refuted, so v3 is kept). N wins over this copy.

**Column order** is the order of `DT_Feats.json:2-23`.

**Conditions (decision 15, H:432):**
- `CivLawsHeldTicksAtLeast` takes a digits-only Key (`schema.json:1229`). Its Value is unread and stays 0.0.
- `MegaprojectComplete` takes a DT_Megaprojects row as Key (:1205).
- There is no `TranscendentSealed` (:914-918), no vessel condition (:892-907), and no system count. So the realm-cap sentence at `systems.md:415` stays true.
- Everything else is empty. The dedication '결박' is Phase 4 (H:32).

**Order:** the row sorts after `feat_dyson_swarm`, so the first row is unchanged for `UI.AscensionOverlay` (`Tests/CosmicAscensionOverlayTests.cpp:69-71`) and for `CosmicPreviewRung`'s fallback (`World/CosmicGameMode.cpp:1790-1801`). `CosmicPreviewRung 5` now shows this row.

**Rereads for the text lens** (merged.md:389-390, :406):
- The overlay (Title and narrative) and LogTemplates[1] all speak of the home galaxy's edicts. The title also overlaps `Edict.Wide` '고향 은하에 닿는 칙령' on purpose (merged #25).
- The guards of B5brief:298-309 apply:
  - the actorless '서 있게 되었다';
  - no binding, triumph, dawn or holy-passive words;
  - no price ('지켰기에'/'덕에'/'대가로');
  - no '때가 찼다'.
- A change is made only on a CONFIRMED finding after rebut-verify. It is recorded in N §5 as a departure from N §1.5.

### 1.3 `Tools/data/DT_Megaprojects.json` `project_binding` (HEAD :62-73)

- `RequiredProgress` (:67): 1800.0 → `P_BIND_A` (a float with `.0`).
- `CostPerTick` (:66): keep Energy 4, Matter 4, Causality 0 (N:683: about a third of T4 income; rule 7 holds Causality at 0).
- The DisplayName and CompleteTemplates are [B-6b5]'s ('중력 기관', merged.md:59-62). Do not touch them.

### 1.4 `Tools/data/DT_Rules.json:17`

- `"AIUpperRungProgressMultiplier": 1.0` → `R_A` (a ladder value, exact in binary, H:84).
- `GalaxyLawsToHold` stays 3 (:103). Three rows with the knob at 3 make `Rung.LawsReset` live (B4brief:335).
- Do not change the C++ default `1.f` (`Data/CosmicDefinitionRows.h:941`).

### 1.5 `Tools/data/ST_CA.csv`: `Edict.TipWide` (small fix, counts as approved)

- Change '위의 확률과 남은 시간은 …' to **'위의 확률이나 남은 시간은 …'**, and leave the rest unchanged (B4brief:323; merged.md row 68 "keep").
- Reason: each game law has exactly one kind of hook. Two have only a probability line and one has only a time line, so "확률과 남은 시간" never describes a game row.
- This is a UI text with no world effect. `Edict.` keeps its lists. It goes on the hand check ⑥ list.

### 1.6 Row names

**No `CosmicRows::` constant.** No game code calls these rows by name: laws are found by `Scope`, and feats by rung (B6brief §1.4). A constant would force a false `required_rows` entry (`schema.json:28`).

Comment edits in `Tests/CosmicB6bRowNames.h` (a header, so close the editor and do a full build):
- :17 "- [B-6b6], [B-6b7]" → "- in the data from [B-6b6], [B-6b7]";
- :27 "- [B-6b7]" → "- in the data from [B-6b7]".

Keep [B-6b6]'s new :1-8 wording.

### 1.7 Assets to commit

- Run `python -I Tools/make_data_tables.py` (through the runner's `-Data`).
- Expected changes: `DT_Edicts`, `DT_Feats`, `DT_Rules`, `ST_CA`, `DT_Megaprojects` (only if `P_BIND_A` ≠ 1800) `.uasset`, and `Tools/data/generated.sha1`.
- If any other `DT_*` asset changes, stop and find out why.

### 1.8 `docs/30`

- Run `python -I Tools/make_config_reference.py`.
- `docs/30_CONFIG_REFERENCE.md:20` goes `1.0` → `R_A`. Nothing else should move.
- The runner's `--check` gate is `Tools/run_checks.ps1:45`.

---

## 2. Code, tests, constants

### 2.1 `Tests/CosmicDataTests.cpp`

1. **`LawProblems`: fix the case that breaks** (HEAD :2157-2183).
   - Today `LawEdicts = CopyOf<FEdictDef>(Game.Edicts)` (:2167). With the game's three laws that becomes 1 + 3 = 4 HomeGalaxy rows.
     - The Expect at **:2181** ("a feat reading three laws over one HomeGalaxy row") then fails, because AtEnd = 3 is no longer above 4.
     - The check at **:2183** ("Two laws … pass") passes for the wrong reason: 5 rows against a knob of 2.
   - Fix, right after :2167:
     ```cpp
     // The game's own galaxy laws out (three from [B-6b7]): the cases below count only the HomeGalaxy rows they add.
     for (const FName GameLaw : CosmicEdicts::GalaxyLawRowsOf(Game))
     {
         LawEdicts->RemoveRow(GameLaw);
     }
     ```
   - Reword the comment at :2157 to match.
   - The game's `feat_laws_and_binding` in `LawFeats` may then report too. That is fine, because `Expect` is any-carries (:1215-1218).
   - :2148 (the game check) now does real work: 3 ≤ 3 slots, 3 ≤ 3 rows, and no law listed as an answer.
2. **The `WithoutB6bData` tripwire** (HEAD :2693-2711; [B-6b6] sets it to four rows).
   - New expectation: `Today == { FeatPassageByArk, FeatLawsAndBinding, ProjectArk, CosmicRows::ProjectBinding, CrisisSpeciesExtinction, EdictWideMachineAccord, EdictWideQuarantine, EdictWideWarning }`, in the `Note` order of `Tests/CosmicTestHelpers.h:375-378`.
   - Message: "Today's data: WithoutB6bData takes out [B-6b6]'s and [B-6b7]'s rows and project_binding (taken: %s) - [B-6b8] updates this pin with its rows".
   - Comment: "(1) Today's data holds eight of B6b's ten rows".
   - The full copy (:2713-2790) holds as is: `FullRules` already sets 0.375 at :2715, and `Expected` 1 at :2778.
3. **New pin (counts as approved): the race is the binding** (decisions 12 and 15, H:429/:432).
   - Every game `DT_Feats` row with `RequiredTier` T4 and `Path` Ruler must carry both `MegaprojectComplete` with Key `CosmicRows::ProjectBinding` and `CivLawsHeldTicksAtLeast`. At least one such row must exist.
   - Message: "The fourth-to-fifth ruler row reads the binding and the held laws (B6b decisions 12, 15)".
   - Why: `Choices` builds the binding only when the judged row names it (`Sim/CosmicMegaprojects.cpp:361-375`). Without the pin, a data edit that drops the binding would silently drop the race.
4. **New pin (counts as approved): r is on the ladder.**
   - `0 < R ≤ 1`, and `R * 16.f` is a whole number. Use plain arithmetic, `S == static_cast<float>(static_cast<int32>(S))`, with no `FMath`.
   - Message: "DT_Rules.AIUpperRungProgressMultiplier is a sixteenth step in (0, 1] (exact in binary - CosmicDefinitionRows.h, header 5.3 ②)".
   - It holds the "binary-exact only" note at `Data/CosmicDefinitionRows.h:938-939`, which nothing checks today.
5. **The theodicy invariant** (`TheodicyProblems` :2477-2570; the game check :2822) runs as is and must pass (the §1.1 numbers). The board cases at :2832-2970 are claim-only, and law rows carry no Claim key.

### 2.2 `Tests/CosmicChronicleScreenTests.cpp` ([B-6b4]'s block at `529cab9` :955-967)

From [B-6b7], `feat_laws_and_binding` is a judged Ruler row at T4 with conditions. `WithFeat` keeps the game's rows (`:163-170`). So all three injected cases would read true on game tables.
- **:960** "The game's tables judge no row up to the fifth rung yet" becomes **TestTrue**, with the message "true on the game tables: feat_laws_and_binding is the judged fourth-to-fifth row (B6b [B-6b7])".
- **:962, :964, :966** (the Ruler row "is one", "a wanderer's row is not", "nor a row without a condition") are built on tables without the game's T4→T5 row: `WithFeat(CosmicTests::WithoutB6bData(Sim->GetTables()), FifthFeat, …)`. If `WithoutB6bData` changes more than the feats in a way that matters here, use a copy of the game feats with `feat_laws_and_binding` removed instead. Add the comment "(the game's own fourth-to-fifth row out: the cases judge only the row they add)".
- :955 and the bar comparison read the helper live, so they follow by themselves.
- **`UI.RunEnd`** (`:531`, `WithFeat` on the game tables with a ruled people set on T4): rerun it. The game row now stands beside `feat_test_fifth`. On a break, build it on `WithoutB6bData` rather than changing counts.
- Any other game-table assertion found in §0.6 changes the same way.
- Rerun `b6b4_judged_row_any_path` and `b6b4_judged_row_never` (§5). Both must still fail.

### 2.3 Comments only

- `World/CosmicGameMode.h:544`: "until a row reaches the fifth rung (B6b [B-6b7]) it shows the first feat row's words" becomes the present tense, "the first DT_Feats row whose TargetTier is the rung - feat_laws_and_binding for the fifth". This is a header, so do a full build.
- `Tests/CosmicUpperRungTests.cpp:1205`: "(the game holds no ark, and its binding row's values are [B-6b7]'s)" becomes "(the boards' own rows; the game's ark and binding values are not read here)".
- `Tests/CosmicBalanceCommandlet.cpp:536-537`: "with no test - the wiring's probe is [B-6b7]" becomes "with no test - probed once at [B-6b7] (notes section 5; Tools/mutations/B6b/b6b7_probe_race_*.patch)".
- `Data/CosmicDefinitionRows.h:267-269` (`GrantedPowers`): confirm only. It names no commit and stays true, because the T4→T5 row grants nothing (decision 15).

### 2.4 Tests whose meaning changes, or that tick a T4 people on game data

**Rule (N:425):** on any break, find the cause before you touch a seed or an expected value. Record every outcome in N §5.

| Test | Expected | On a break |
|---|---|---|
| **`LayBoard`** (`Tests/CosmicUpperRungTests.cpp:322-341`): copies the game's rules, then calls `Edit` | **Change now:** set `Rules.AIUpperRungProgressMultiplier = 1.f;` before `Edit(Rules)`, with the comment "the boards' own rate; the game's r is [B-6b7]'s". `Sim.Binding` (AI people 1 at progress 3.0 is expected to complete in one step, `:1674-1688`) would otherwise fail at r < 1. `Sim.ArkAnswers` (:1294) and `Sim.UpperRungDelay` (:1349) set their own values in `Edit` and keep them | — |
| Other boards on the game's rules (`Sim->GetRules()`, `Original.FindRules()`, `Knobs(BaseRules)` at `CosmicAscensionTests.cpp:202`) where a non-ruled people builds a `RequiredTier` ≥ T3 construction row | r now multiplies their rate | If the delay is not the test's subject, set `AIUpperRungProgressMultiplier = 1.f` on that board's rules, with the same comment |
| `Sim.PlayerAscendsToT3` (`Tests/CosmicAscensionTests.cpp:1782`; :1855 "no end in its tick", :1856 "nor in any tick of the climb" (`bEndOnTheWay`, :1851), :1858 "no record but the clock's"; N:444's :1681 is stale) | Most likely unchanged: an AI T5 needs `AI_T5_MIN` (§3.1) | If an AI's fifth rung stands in the run, widen all three to "the clock's or another order's" (`!Valid \|\| Clock \|\| OtherPeople`; `EndingOf(…).Kind` ∈ {none, OtherPeople} for :1851/:1855) (N:444). Add mutation `b6b7_ascends_t3_other_order_dropped`, which must break the widened `bEndOnTheWay` reading too |
| `Feats.Reachable` (`:1760-1780`; N:348 gives #40 to [B-6b6] and [B-6b7]) | Read the non-ruled `Tier` at Tick 20000 in the runner's `checks_20000.csv` (or run the raised assertion once). This mirrors B6brief:303-310 | If at least one AI stands on T5: raise it to "An AI people reaches the fifth rung in 20,000 ticks of the game's data", add mutation `b6b7_binding_never_completes` (`CosmicSeal::BindTargetOf` returns `INDEX_NONE`), and update `docs/40:238`. Otherwise record the reason in N §5 and the H §11 line |
| `Sim.PhaseTwoExit` (`Tests/CosmicSimTests.cpp:2260`, seed 18, 20,000 ticks, :2399-2400) | Rerun | An AI binding before the first annexation freezes the sphere. Write the cause in H §11, then make the test read annexations before the first binding tick (N:447). Do not change the seed first |
| `Sim.RealmCap`, `Sim.CivAIProjects`, `Sim.NotablesCapped`, `Sim.StartBlockReasons` (game part, 2,000 ticks) | Same shape | Cause first |
| `UI.EdictWindow` (`EdictsOfTest` copies every game edict, `529cab9:Tests/CosmicEdictUITests.cpp:96-131`, :307): the three game laws now show at T4 beside the test laws | Rerun | Read the test's tables without the game laws (`CosmicTests::WithoutB6bData`) rather than changing counts |
| `UI.RunEnd` (§2.2) | Rerun | As §2.2 |
| Any other test that ticks a T4 people on the game's `DT_Feats`/`DT_Edicts` | The law step and the binding start now act | If B6b is not the subject, put `CosmicTests::WithoutB6bData` outermost (the [B-6b3b] precedent). Never absorb the change with seeds or expected values |

Not affected: boards with their own `NewTable` edicts or feats (`CosmicEdictTests.cpp:130-145`, `CosmicUpperRungTests.cpp:244`, `CosmicRaceMeasureTests.cpp:118`, `CosmicAscensionTests.cpp:1650-1655`, :1730-1736), and Python's `fixture_tables`.

### 2.5 Measurement-line meaning (no code change)

`CosmicBalance conflict:`'s "quarantine held" counts every ruled edict with a plague factor < 1 (`Tests/CosmicBalanceCommandlet.cpp:3707-3713`). It now includes `edict_wide_quarantine`. Note this in N §5 for bundle B's reader, and in `docs/40`'s line description if that description names the quarantine.

---

## 3. Determinism and baselines

### 3.1 When the world can diverge from [B-6b6]

Nothing reads the new rows before the first T4, T4\*:
- `EnactBlockOf` refuses below the row's tier (`Sim/CosmicEdicts.cpp:59-62`).
- `AsksForLawsOf` needs ≥ T4 (:242-256), and the policy's (4') has the same gate (`World/CosmicRulerPolicy.cpp:686-697`).
- The hooks read held edicts only (`Sim/CosmicEdicts.cpp:134-180`).
- The feat is judged only at T4 (`Ascension/AscensionSubsystem.cpp:100-104`).
- `Choices` names the binding only from the T4 row.
- `HoldsGalaxyLawsOf` needs ≥ T4.
- r is read only in the construction slot (`Sim/CosmicMegaprojects.cpp:20-32`, :676-683), on rows ≥ T3. In the game that is `project_binding` alone. [B-6b6]'s `project_ark` is T3, but it is a hazard answer: `Start` puts answers in the response slot (`:305-311`), and `Choices` skips them (`:372`).

Timing:
- **An AI first:** the binding starts and the laws are enacted at stage (a) of T4\*+1 (`GalaxySimulationSubsystem.cpp:2941-2954`).
- **The ruled people first:** the policy acts at the boundary after T4\*, before that boundary's CSV sample.
- So rows are guaranteed equal for **Tick < T4\***, and also at Tick = T4\* when an AI is first.

**New sim draws: 0** (H:234). Draw counts still follow the world (more systems, other shares).

**The hook-① product** is `(Chance*Factor)*SpawnMultiplierOf*Spreading` (`GalaxySimulationSubsystem.cpp:2376`). `SpawnMultiplierOf` is the product over `HookEdictsOf` order (owner's `ActiveEdicts`, then the other peoples in `Civs` order, `CosmicEdicts.cpp:181-192`).
- That order is part of the state, so any value is deterministic.
- Using powers of two is a property, not a determinism requirement. It keeps the product's bits independent of enactment order, so a change of pair or law order cannot move the hash. B2's factors are 2.0 and 0.1 (`DT_Edicts.json:16`, :113), and at most one non-power-of-two factor stands per tag.
- No other edict carries an awakening factor. So any single `M_AWAKE` is order-independent too, 0.375 included (§0.4's ladder).
- Hook ② is an integer sum (:194-206).

**The fifth-rung end** (`EndingOf`, `Sim/CosmicEndings.cpp:26-71`). With placeholders:
- `RULED_T5_MIN` = ruled T4 + `P_BIND_A`, and at least ruled T4 + `N_A` (the laws must be held N ticks).
- `AI_T5_MIN` = `T4*_AI` + 1 + ceil(`P_BIND_A` / `R_A`) (`Sim/CosmicMegaprojects.cpp:682-683`). At 1,800 / 0.375 that is +4,801; at `R_A` 0.5, +3,601.

Pre-stated:
- The 3,000 lines and both hash runs cannot hold a T5.
- Any of the wanderer 6,000, 20,000 and ruler 20,000 lines may end as `OtherPeople` when `AI_T5_MIN` ≤ that line's clock or horizon, and as `RuledPeople` when the ruled people's T5 comes first.
- The ruler 20,000 record otherwise stays `Clock`.
- Each moved line's `ending:` kind is written in §3.2 item 4. The existing end check (0 `CosmicBalance end check` lines) holds it.

### 3.2 Re-recording (`Tools/baselines/seed42.json`; H §5.4 → N:598)

1. **Final:** `run_checks.ps1 -Tier full -Data -UpdateBaseline`, with the editor closed.
   - It rewrites the five lines and `verified_at` (`Tools/run_checks.ps1:101-107`).
   - The hash runs are compared with their lines (:139-157): the input-less hash run with `checks`, and the policy hash run with `checks_ruler`. A hash run moves exactly when its paired 3,000 line moves.
2. **`last_recorded_in_changelog`, by hand.**
   - `task`: "B6b B-6b7 (data ② fourth to fifth: edict_wide_machine_accord/quarantine/warning, feat_laws_and_binding, project_binding P_BIND_A, r R_A)";
   - the four input-less hashes;
   - `note`: the reason, each moved line's first mover (T4\* and who), and the "Older values: …" chain kept.
   - Write it as UTF-8 with no BOM.
3. **Breakable checks:**
   - **ⓐ The off pair.** Run the five runs with the runner's exact arguments plus `-WithoutB6bData=1`.
     - For each run, the whole key line after `Remove-WrotePath` (`run_checks.ps1:76`), with its ruler and wanderer parts, must equal [B-6b5]'s letter for letter.

       | Run | Hashes |
       |---|---|
       | 3,000 | 2816687688 / 3300228750 |
       | ruler 3,000 | 3561771757 / 2462331303 |
       | wanderer 6,000 | 2665013209 / 2229239127 |
       | 20,000 | 2359424635 / 726895141 |
       | ruler 20,000 | 1611658162 / 29194274 |

     - Each off log must hold exactly this line (`CosmicBalanceCommandlet.cpp:1596`):

       `CosmicBalance: tables without B6b's data - rows taken out [feat_passage_by_ark feat_laws_and_binding project_ark project_binding crisis_species_extinction edict_wide_machine_accord edict_wide_quarantine edict_wide_warning], AIUpperRungProgressMultiplier 1 (was <R_A as %g>), FleetDelayCountdownShare 0 (was 0)`

   - **ⓑ Base against on.** For each of the five runs, every CSV and `_rows.csv` row with Tick < T4\* (that run's [B-6b6] T4\*, §0.5) must equal `b6b7_base\` on every column except `TickMicros`, `SaveBytes` and `SaveRawBytes`.
     - A run with no T4\* inside its horizon must be **bit-identical** to [B-6b6], keys included.
     - A difference before T4\* stops the commit: something reads the new rows early (H:403).
   - **ⓒ** Every log has 0 `CosmicBalance end check` lines and 0 `race check` lines (`:1968`, :2025, :3575).
4. **First mover per moved line:**
   - T4\* and who climbed (race: parts 2-3);
   - part 7 (`upper construction first started by an AI at`, `ruled binding started at`);
   - part 10 (`laws first held ruled/AI`);
   - the `ending:` kind and tick (§3.1);
   - the first CSV sample that differs.
5. **Facts that may move:**
   - `STATUS.md` '기준값';
   - `docs/40` (the baseline line, :25 at HEAD);
   - the `Tools/run_checks.ps1:121-124` comment, if the wanderer's crowning moved.

---

## 4. Measurement: bundle A′ (small, parallel) and the pre-written judgments

**Runs.** Run the 30 R runs of bundle A again on this build, on side only:
- seeds 1-30, 20,000 ticks, `-Rules=ProvisionalEndTick:0`;
- arguments `-Answer=1 -Build=1 -Focus=1 -Edict=1 -Diplomacy=1 -TellAll=0`;
- output into `Saved\Balance\b6b7_A1\`.

In the same pool, add the two 20,000 on-runs for §3.2 (into `b6b7_on\`) and the five ⓐ off runs. That is 37 processes.

**Harness.**
- `scratchpad\b6b6\bundleA.py` with a cell or seed filter: 6 workers, the 20,000 jobs first, about 4-5 minutes.
- Its fail rules: exit ≠ 0, missing lines or files, any check line, or `LogCosmic: Error`. Rerun a failed run once.
- Its sha1 guard covers the dll and the `DT_*`/`ST_CA` assets.
- Never run it alongside a build, `make_data_tables`, a mutation or another workflow.

**Pre-written judgments** (fixed before the run; write them into N §5 before reading the numbers):

| # | Check | If it fails |
|---|---|---|
| J1 | Per seed, CSV and `_rows.csv` rows with Tick < T4\* (from that seed's [B-6b6] bundle A race: parts 2-3) equal the [B-6b6] on-run in `b6b_bundleA\` (same ignored columns) | Stop: a pre-T4 read of the new rows |
| J2 | 0 check lines and 0 errors over all 37 logs | Stop |
| J3 | Wiring: if ≥ 1 run has a ruled T4 with Society 1.0 before the run's end, then ≥ 1 such run has `laws first held ruled at` ≠ −1 | Stop: the policy's (4') or the slot gate does not reach the laws |
| J4 | Wiring: if ≥ 1 run has any T4, then ≥ 1 run has part 7's `ruled binding started at` or `upper construction first started by an AI` ≠ −1 | Stop: `Choices` does not build the binding from the new row |

**Recorded for the balance lens and bundle B.** Nothing moves in this commit except `M_AWAKE`, which §0.4 sets.
- Part 10's resets "by upkeep" (ruled and AI) and AI "held again in the tick". These are expected 0 with upkeep 0. They are data-bound, not a judgment, but a non-zero count is investigated.
- T5 counts by kind.
- The other-order share, as a point plus Wilson 95%, with the H §5.3 ② (가)/(나) split. At n < 30, write '재지 않음 — 잰 N'.
- The ruled T4→T5 median, against S_m from §0.4.
- Ruled law resets by input (part 10 `ruled laws dropped at an input`), against the model λ.
- AIs that ever held three laws, and the first AI's Society 1.0 tick.
- For each AI building the binding, its `Energy`/`Matter` over the build (signs of unpaid ticks U).
- **#136:** runs with a first-person line before the record or before 6,000, on A′ against [B-6b6] A (§0.4), with Wilson 95%.
- **Plague spread:** the `conflict:` line's plague placed and spread-placed after the first law holder's T4, against [B-6b6] A (§1.1's set-versus-alone point).
- **Extinctions:** extinctions placed per 600 ticks after the first law holder, against [B-6b6] A. Fewer awakenings and plagues leave more free system-time for `crisis_species_extinction` draws (one hazard per system, `GalaxySimulationSubsystem.cpp:2350`).

If the other-order point lies outside 15-35% and the whole Wilson interval is outside too, flag it to [B-6b8]'s bundle B. Judgment ② and [B-6b9] decide; nothing changes here.

---

## 5. Mutations (new + touched only; N:709) and probes

**New mutations** (each must fail on a clean build first):

| Patch | Fails |
|---|---|
| `b6b7_data_laws_two_rows`: `edict_wide_warning` removed (rebuild the assets) | `Data.Integrity` (the game check :2148: "feat_laws_and_binding reads the held laws … above the 2 HomeGalaxy rows") |
| `b6b7_data_feat_without_binding`: drop `MegaprojectComplete project_binding` from the feat (rebuild the assets) | `Data.Integrity` (§2.1 item 3) |
| `b6b7_data_upper_rung_not_binary`: r 0.4 (rebuild the assets) | `Data.Integrity` (§2.1 item 4) |
| `b6b7_without_b6b_keeps_laws`: the `Edicts` list at `CosmicTestHelpers.h:378` emptied | `Data.Integrity` (tripwire and full copy) |
| `b6b7_data_law_answers`: `crisis_plague.RulerResolutions` += `edict_wide_quarantine` (rebuild the assets) | `Data.Integrity` (law as answer), now on a game row |
| `b6b7_ascends_t3_other_order_dropped` | Only if §2.4 widened `Sim.PlayerAscendsToT3` (it must break the widened `bEndOnTheWay` too) |
| `b6b7_binding_never_completes`: `CosmicSeal::BindTargetOf` returns `INDEX_NONE` | Only if §2.4 raised `Feats.Reachable` |

**README** (`Tools/mutations/README.md:5-7`): add `b6b7_data_laws_two_rows`, `b6b7_data_feat_without_binding`, `b6b7_data_upper_rung_not_binary`, `b6b7_data_law_answers` and `b6b7_data_ark_margin_half` to the list of data patches that need `make_data_tables`. Add one sentence saying that `b6b7_probe_*` are commandlet probes in the `b6a3a_probe_*` form (README:648), not test-breaking mutations.

**Race check probes.** These are commandlet probes, not tests, and are never committed applied. Keep them as `Tools/mutations/B6b/b6b7_probe_*.patch`, with README rows typed "commandlet probe", so that [B-6b12]'s rerun repeats them. Do not use `-FourthRungCap`, which lowers the brake as well (B6brief:703).

1. **`b6b7_probe_race_cap_minus_one`**: the check fires on an AI climb past the cap.
   - Patch: `Tests/CosmicBalanceCommandlet.cpp` `RaceCap = RaceHomeGalaxy ? RaceHomeGalaxy->AIFourthRungCap - 1 : 0` (HEAD :2000).
   - `PROBE_SEED_1`: from the five baseline runs or A′, the earliest run whose race: line shows an AI climbing to T4 while another people already stands there (part 2 `second AI T4 at` ≠ −1, or a ruled T4 before an AI T4). `-Ticks` = that tick + 100, with that run's arguments.
   - Expected:
     - one `CosmicBalance race check: at <\tick> … past the home cap <cap−1> …` line listing an `:a` climber;
     - the :3575 summary line;
     - exit code 1.
   - At cap 1 the first AI climb alone passes. cap − 2 = 0 would switch the check off (`Tests/CosmicRaceMeasure.h:39-70`).
2. **`b6b7_probe_race_ruled_unjudged`**: the `RaceRuledJudged` wiring that the comment at `:536-537` hands to [B-6b7].
   - The check is asked only when a people other than the judged ruled one climbs (`FourthRungCheckOf`, `bAsked |= … && !Step.bRuled`). So probe 1 cannot see a broken `RaceRuledJudged`.
   - Patch: probe 1's cap−1 plus `Each.Key == RaceRuledJudged` → `false` (HEAD :2010).
   - `PROBE_SEED_2`: a run where the ruled people climbs to T4 while exactly one AI already stands there.
   - With probe 1 alone on that seed, there must be no miss at the ruled climb tick.
   - With probe 2, there must be a miss at the ruled climb tick that names the ruled `CivID`. It is tagged `:a`, because the broken flag also feeds the `Climbers` label.
   - If no such seed exists in the baselines or A′, record "not probed — no seed" and hand it to [B-6b8]'s bundle B.
- After the probes, revert, rebuild both targets clean, and record each seed, tick and line in N §5 and `docs/40:502`.

**Retakes:** every patch that applies in `apply_before.txt` but not in `apply_after.txt`.
- Known: `b6b2c_data_ark_margin_half` → `b6b7_data_ark_margin_half` (the same `AIAscensionSpeedMultiplier` 0.5 line, new context).
- `b6b2c_data_binding_causality` → `b6b7_`, if `P_BIND_A` ≠ 1800.
- [B-6b6]'s end-of-file `DT_Feats` patches.
- Mark the old README rows "(no longer applies since [B-6b7] - taken again as …)".

**Reruns, built mechanically:**
- every old patch whose `+++ b/` file this commit edits: `DT_Edicts.json`, `DT_Feats.json`, `DT_Rules.json`, `DT_Megaprojects.json` (if :67 moved), `ST_CA.csv`, `CosmicDataTests.cpp`, `CosmicB6bRowNames.h`, `CosmicChronicleScreenTests.cpp`, `CosmicGameMode.h`, `CosmicUpperRungTests.cpp` (`LayBoard`), `CosmicBalanceCommandlet.cpp`, plus any test file §2.4 edited;
- every old patch whose README "Breaks" test is defined in an edited test file. This includes the `UI.Chronicle` ones (`b6b4_judged_row_any_path`, `b6b4_judged_row_never` must still fail), every `Sim.*` breaker of `CosmicUpperRungTests.cpp`, and the `Data.Integrity` breakers.

**Time budget** (developer speed rule). The `Data.Integrity` breakers number about 33 at HEAD, and many need a `make_data_tables` pass. Estimate the time before starting.
- If the whole set runs over about 60 minutes, run only:
  - the breakers whose hunk or failing case lies in the touched lambdas (`LawProblems`, the `WithoutB6bData` block, `TheodicyProblems`, `BindingProblems`);
  - the patches against the files this commit edits;
  - the `UI.Chronicle` and `CosmicUpperRungTests.cpp` ones.
- Leave the rest to [B-6b12]'s full rerun, and record the split in N §5.

**Running:** serially after the final build, never beside a commandlet. Then write `apply_after.txt` and the README rows, and do **a clean rebuild of both targets**, so that [B-6b8] starts unmutated.

---

## 6. Docs and records

**H §11:** one line, under 400 bytes (`wc -c`). Template (≈330 bytes once filled):

> "- `[B-6b7]`(2026-11-xx): 데이터 ② — 법칙 행 셋(각성 몫 <M_AWAKE>·역병 몫 0.5·각성과 역병의 남은 시간 +50, 제정 40·유지비 0)·`feat_laws_and_binding`(N <N_A>)·결박 <P_BIND_A>·r <R_A>. 다르게: `Edict.TipWide`의 '이나', 핀 둘(T4→T5 행이 결박을 읽음·r의 1/16). 세부 노트 §5."

- If `R_A` ≠ 0.375, add inside the 400 bytes: "§5.3 ②의 한 걸음은 <R_A>에서". The same goes in `DECISIONS_LOG`.
- If `Feats.Reachable` was not raised, add its reason here if it fits; otherwise only in N.
- The header was 146,814 bytes at `cef8cc7`. After the §11 lines of [B-6b4]–[B-6b6] it should be about 148,000.
- If it passes 149,000, move the excess (the longest §11 line's detail) into N in this commit (H:406).

**N §5 `[B-6b7]` block** (the form of N:700-709):
- 나눔: none.
- 데이터: the rows, with the text source (N §1.5, or the fallback).
- 값과 까닭: §1.1, §0.3, §0.4.
- 테스트: the §2.1-2.4 outcomes (`LayBoard`, `Feats.Reachable`, `UI.Chronicle`/`UI.RunEnd`/`UI.EdictWindow`) and the count (unchanged unless a test was added).
- 변이: new, retaken and rerun counts from the apply diff, and the §5 split if taken.
- The probes: seeds, ticks and lines.
- 리뷰: counts.
- 기준값: old → new, ⓐ ⓑ ⓒ, the first movers and ending kinds, J1-J4, and the A′ readings.
- 헤더와 다르게 하거나 더한 것:
  - ① the hook values (H gives none), with the warning's pair, the reason it is not Rebellion, and the plague set-versus-alone note;
  - ② upkeep 0 and effects none;
  - ③ the silence rule not taken;
  - ④ the `Edict.TipWide` fix;
  - ⑤ the two new pins;
  - ⑥ the B6brief:522 P_bind typo, fixed at its source before [B-6b6], and how `P_BIND_A` was read;
  - ⑦ if `R_A` ≠ 0.375, H §5.3 ②'s ladder (frozen, H:343) is read from the committed value (also in H §11 and `DECISIONS_LOG`);
  - ⑧ the "quarantine held" meaning;
  - ⑨ stale citations: N:444 → `CosmicAscensionTests.cpp:1782`; N:348's `systems.md:409` is now `:415` (and `:117` also names `Feats.Reachable`; B6brief:310's "no `Feats.Reachable`" was wrong); `docs/40:502` said T4 first stands at [B-6b7], but it stood at [B-6b6].

**N §2 ⓓ:** the paragraph "`[B-6b7]`의 m" (§0.4). N §2.3 and H §5.3 stay frozen.

**`DECISIONS_LOG.md`:** one `[B-6b7]` line after [B-6b6]'s, in the form of :35-38. It carries:
- the values and their reasons: `P_BIND_A`, `N_A` and `R_A` from the "다시 셈" paragraph (with [B-6b9]'s start if `R_A` ≠ 0.375); `M_AWAKE` from §0.4 with #136; 0.5 and +50 with the theodicy number 0.493; cost 40 / upkeep 0;
- the new baselines;
- the probes;
- review counts.

**Markers that become false; drop or reword them now** (re-find each by its text on H‴):
- `docs/20_ARCHITECTURE.md:75`: "판정받는 행이 없는 칸(지금 게임 데이터에서는 셋째 칸부터 — 셋째 → 넷째 행은 `[B-6b6]`부터)", as [B-6b6] rewords it. After this commit no rung up to the fifth lacks a judged row. Reword it to the present tense. Re-find the matching sentence in `docs/21`'s `CosmicNextRung` copy and change it too.
- `docs/21_DATA_MODEL_REFERENCE.md`:
  - :630 ("(`[B-6b7]`)");
  - :1198 ("시작값 0.375는 `[B-6b7]`" → "값 <R_A>(`[B-6b7]` — 묶음 A의 다시 셈)");
  - :1213 ("마지막 값은 `[B-6b7]`; … 행은 `[B-6b7]`부터" → the values, in the present tense);
  - :1215 (the three law rows, each with its hook in one clause).
- `docs/10_DESIGN/systems.md`:
  - :91;
  - :162;
  - :163 (only the "`[B-6b7]`부터" part; the §12.1 count waits for [B-6b12]);
  - :170;
  - :175 (the awakening share: `M_AWAKE`, and #136 judged at bundle C);
  - :415/:117 only if `Feats.Reachable`'s reading changes (§2.4).
- `docs/10_DESIGN/pillars.md`: :106, :108.
- `docs/10_DESIGN/crisis_and_transcendents.md`: :126, :139 (two places), :140 ("첫 값은 `[B-6b7]`" → `R_A`).
- `docs/70:129`: [B-6b6] leaves it as "`[B-6b7]`부터".
- `docs/40_DEV_GUIDE.md`:
  - :238 (`Feats.Reachable`, if raised);
  - :318 (the `UI.Chronicle` row: the helper is true on game tables);
  - :502 (the probes are done; T4 first stood at [B-6b6]);
  - :613 ("다섯째 칸에 이르는 행이 데이터에 오기 전(`[B-6b7]`)에는 첫 업적 행의 글을 보인다" → present tense: `CosmicPreviewRung 23 5` shows `feat_laws_and_binding`'s Title and narrative);
  - the `Data.Integrity` row (the two new pins; the laws check is now live on game rows).
- `docs/30`: regenerated (§1.8).
- `docs/50` and `docs/15` wait for [B-6b12]. The glossary rows for '기계 살핌' and the others go with '중력 기관' (merged.md hand-offs).
- Run `python -I Tools/check_docs.py` and expect 0.
- Optional, after the final build and with the editor closed: take one windowless shot of `CosmicPreviewRung 23 5` (the B4brief:699 command), so that the new Title '고향 은하의 칙령' is seen once before hand check ⑥.

**`STATUS.md`:**
- '지금': [B-6b7] done.
- '다음 3작업': [B-6b8], [B-6b8b], and the hand checks.
- '코드 사실': DT_Edicts 10, DT_Feats 5.
- '기준값': old → new.
- The hand checks: B6a ①/③/⑨ ([개발자]; a `5455e56` worktree at any time), and ⑥, now possible for `Edict.Wide`/`Edict.TipWide` and the fifth-rung overlay on game rows (B4brief:755).

CHANGELOG waits for [B-6b12].

**Commit message.**
- Title:

  > `[B-6b7] 데이터 ② 넷째 → 다섯째 칸 — 은하 법칙 행 셋(기계 살핌·은하 검역·은하 경보망)·업적 feat_laws_and_binding(N <N_A>)·결박의 값(<P_BIND_A>)·r <R_A>, 결박을 읽는 행과 r의 핀, race check 탐침 둘, docs/30, <LINES_MOVED_B6B7 phrase>`

- Body:
  - the "옮은 기준값" block (old → new; the hash runs equal their lines), with each first mover and ending kind;
  - ⓐ ⓑ ⓒ and J1-J4;
  - the probes;
  - mutation and review counts;
  - `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- Push to origin.

---

## 7. Order of work and risks

**Order (one lean ultracode workflow).**
1. §0, including `m_awake.py`, the #136 base reading, and the placeholders written into the brief's working copy.
2. Implementers, in parallel:
   - **I1, data:** §1.1-1.5, §1.7, §1.8; `check_data` = 0; the Python tests.
   - **I2, C++:** §1.6, §2.1-2.3, the `LayBoard` change of §2.4, and the patch files (not yet run).
3. **Build pass**, with the editor closed: a full build (header edits), then `run_checks.ps1 -Tier smoke -Data -UpdateBaseline`. Copy `checks*.csv` with `_rows`/`_claims` into `b6b7_on\`. Read `checks_20000.csv` for `Feats.Reachable` (§2.4).
4. **The §4 pool:** A′, the two 20,000 on-runs, and ⓐ. Then ⓑ, ⓒ and J1-J4.
5. **Four review lenses.** Every finding gets a rebut-verify.
   - **L1, data, text and theology:** verbatim N §1.5; Summaries against hooks; the overlay and feed trio; guardrails (no holy order as actor, no title, no 'became god', no soul count, the binding unnamed); `Edict.TipWide`; the docs markers.
   - **L2, determinism and baselines:** §3; the order-independence property; J1; the off-pair line; the ending kinds against `AI_T5_MIN`.
   - **L3, tests, mutations and build:** §2.2's injected cases; `LayBoard`; the §2.4 table; the apply diff; the probes' seeds.
   - **L4, ultracode balance:**
     - `M_AWAKE` with #136;
     - the warning's +50 on plague (set versus alone, the A′ spread reading);
     - cost 40 / upkeep 0;
     - AI law reach (three slots need Society 1.0, `Sim/CosmicEdicts.cpp:26-35`, and 120 Influence);
     - the AI paying 4/4 for P/r ticks (4,800 at 1,800 / 0.375), with possible U;
     - extinction exposure;
     - the A′ readings;
     - the vigil (3) resets that λ leaves out.
6. **Fix.** If a fix touches data or the sim, repeat steps 3-4.
7. **One final runner:** `-Tier full -Data -UpdateBaseline`. Its five lines must equal those in `b6b7_on\`.
8. Mutations, then the probes (§5), then the clean rebuild.
9. Records, `check_docs`, commit and push.

**Risks (recommended option = approved):**
1. **N §2 "다시 셈" is missing, or it gives [개발자].** Stop; nothing is invented.
2. **The P_bind typo.** It is fixed at its source before [B-6b6] runs; §0.3 guards against a 1,200 resample. Record it.
3. **Few AIs ever hold three laws** (Society 1.0 and Influence), so AI T5 is rare and the other-order share falls below 15%. Record it at A′; bundle B judges, and [B-6b9] moves values. No value moves here.
4. **The AI's binding goes unpaid.** Leave 4/4. Flag it in L4 with the CSV stockpiles; the value changes only in [B-6b9].
5. **λ underestimates resets** (vigil (3), H:158). Record the measured resets beside the model.
6. **A pre-T4 divergence** (J1/ⓑ). Stop and find it (H:403).
7. **Header size near 149,000.** Move the excess to N in the same commit.
8. **[B-6b5]/[B-6b6] text drift.** N §1.5 on H‴ is authoritative over this brief and merged.md.
9. **Concurrency.** No build, commandlet or mutation beside another workflow; the sha1 guard.
10. **An early AI T5** at `R_A` 0.5 can end a 6,000 or 20,000 line as `OtherPeople` (§3.1). This is expected, not a failure, as long as the end check is 0.
11. **[개발자] items:**
    - the B6a hand checks ①/③/⑨;
    - hand check ⑥ on game rows;
    - any [개발자] outcome of §0.3/§0.4.

### 추가 이야기와 보완
- **Commit `bundleA.py`.** Committing `scratchpad\b6b6\bundleA.py` under `Tools/`, with a "Tick < T4\*" mode (J1), would let [B-6b8] and [B-6b11] repeat the exact check. If it is committed, it gets one `docs/40` line.
- **Two new `race:` counters before bundle B.** [B-6b8] could add two measurement-only, bit-identical counters: unpaid binding ticks (U), and each AI's Society 1.0 tick. Then N ⓓ's S becomes measured instead of modelled.
- **A schema-level `has_cond`.** A positive condition operator would let §2.1's binding pin move from C++ into `check_data`. That is a question for [B-6b12] or Phase 4.
- **The `silence_words` rule.** It should come back with `edict_sabbath`'s scope decision in B7. Note it in OPEN_QUESTIONS if B7's header does not.
- **`VigilRowOf`'s order dependence.** It returns the first capital-tag row by name. That holds only while `edict_vigil` sorts before any other row with a positive Rebellion bonus. A one-line `Data.Integrity` pin ("the only edict with a positive capital-tag countdown bonus is edict_vigil"), or a comment, would make it explicit. It is cheap, and can be bundled with [B-6b8].

---

## Critique dispositions

**Critique 1**
1. HIGH, `UI.Chronicle` injected cases on game tables — **taken.** Verified at `529cab9:Tests/CosmicChronicleScreenTests.cpp:163-170`, :960-966. → §2.2 (:960 flips; :962/:964/:966 move onto `WithoutB6bData`; `UI.RunEnd` :531 rerun; `b6b4_judged_row_*` rerun).
2. `Feats.Reachable` — **taken.** N:348 assigns #40 to [B-6b6] and [B-6b7]. → §2.4 row (raise if an AI stands on T5 at 20,000, with mutation `b6b7_binding_never_completes` and `docs/40:238`; otherwise record the reason).
3. `systems.md` cites `Feats.Reachable` — **taken.** Verified: two hits at `cef8cc7`, :415 and :117. → §6 ⑨ corrected; the realm-cap sentence checked in §1.2.
4. m together with #136 — **taken.** H:250, H:426 and `systems.md:175` confirmed. → §0.4 #136 reading and the alternative recorded; §4 A′ #136 row.
5. Wrong determinism reason — **taken.** → §3.1 reworded as an order-independence property; §0.4 ladder {0.5, 0.375, 0.25}.
6. `docs/20:75` — **taken.** Verified at `529cab9`. → §6, together with the `docs/21` copy.
7. `docs/40:613` — **taken.** Verified. → §6, plus the optional shot.
8. `PlayerAscendsToT3` has three assertions — **taken.** Verified :1851/:1855/:1856/:1858. → §2.4 row.
9. README data-patch list — **taken.** Verified README:5-7 and :648. → §5.
10. `UI.EdictWindow` and `UI.RunEnd` — **taken.** `EdictsOfTest` copies the game edicts (:96-131, :307). → §2.4.
11. Record an `R_A` deviation in H — **taken.** → §6 (H §11 and DECISIONS_LOG).
12. [B-6b4] committed as `529cab9` — **taken.** → header and §0.1/§0.6 grep results.

**Critique 2**
1. The warning's +50 on plague lengthens the ×8 spread window — **finding verified, correction (b) taken, (a) rejected.** Verified at `GalaxySimulationSubsystem.cpp:2330-2350`, :2376, `PlagueSpreadMultiplier` 8.0 and plague countdown 200. Rebellion is rejected because the design keeps every outside time-buyer off the capital rule's rebellion (fleet delay `docs/21:1516`, card `DelayHazard` `not_tag_of` `docs/21:1265`), and the law would reach other peoples' capitals. The AI law order guarantees the quarantine before the warning, so the set is a net shield (0.30 vs 0.48). → §1.1 note, §4 spread record, L4.
2. The probe cannot see `RaceRuledJudged` — **taken, with a correction.** Verified (`CosmicRaceMeasure.h` `bAsked |= … && !Step.bRuled`). The broken run's climber is tagged `:a`, not `:r`, because `Climbers` reads the same broken flag. Probe 1 is kept so that the check is still shown to fire. → §5 probe 2.
3. Fix the P_bind typo at the source — **taken.** B6brief:522 confirmed "Keep P_bind = 1,200". → the lead's pre-[B-6b6] action at the top; §0.3 becomes a guard.
4. `Sim.Binding` breaks; fix it in `LayBoard` — **taken.** Verified `LayBoard` :322-341 (copies the game rules, then `Edit`) and :1674-1688. → §2.4 first row, done by I2.
5. AI T5 bound and the `OtherPeople` record at other values — **taken.** → §3.1 `AI_T5_MIN` placeholder formula, the pre-stated ending kinds, §3.2 item 4, risk 10.
6. Power-of-two over-justified — **taken** (same as C1-5).
7. Wrong reason for keeping Rebellion off — **taken.** The capital rule loops over every `Civs` entry (:2414-2448). → §1.1 gives the true reason.
8. λ omits the archetype factor — **taken.** `arch_seeker` 1.2 verified (`DT_Civs.json:12`). → §0.4 λ = 0.0002·a·k·m.
9. J5 cannot fail — **taken.** → J5 removed from the judgments and moved to "recorded" (both upkeep and "held again in the tick", which exists at `:3976`), investigated if non-zero.
10. Extinction exposure — **taken** as a recorded row in §4; no value change.
11. Citations — **taken:** (a) :2181/:2183; (b) the theodicy formula as countdown + `GuardDelayCapOf` + bonus (matches `TheodicyProblems`); (c) the ark under the response slot, with `Choices` skipping answers at `:372` (not :367); (d) the hash-run pairs named; (e) the whole key line compared.
12. Rerun cost — **taken.** → §5 time budget with the 60-minute fallback split.

Files read (nothing in the project was written):
- `C:\Users\rhwwl\AppData\Local\Temp\claude\c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension\37de2919-0c44-45d8-94cd-4dcf0860263e\scratchpad\b6b6\brief_final.md`
- project files through `git show HEAD:` at `529cab9`/`cef8cc7`