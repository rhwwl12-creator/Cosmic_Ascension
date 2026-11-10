export const meta = {
  name: 'b6b8-side-rows-bundleB',
  description: 'Implement [B-6b8] data 3 (event_era_endured, enc_witness, witness words rules, NoCheck counter, WitnessFirst), build pass and seed-42 probe, four review lenses, fix, mutations, final runner, bundle B with the frozen judgments, records with an L5 check, the deferred windowless shot',
  phases: [
    { title: 'Start', detail: 'section 0 start state and placeholders; harness back-port and self-checks in parallel' },
    { title: 'Implement', detail: 'I1 data/schema/assets/data docs, I2 C++ tests/counters/patch files (no build)' },
    { title: 'Probe', detail: 'full build, smoke -Data -UpdateBaseline, seed-42 five on-runs, checks a-c, bundleB --max-seed 2 smoke' },
    { title: 'Review', detail: 'L1 data/text/theology, L2 determinism/baselines, L3 tests/mutations, L4 harness and judgment code' },
    { title: 'Verify', detail: 'one refuter per finding' },
    { title: 'Fix', detail: 'apply kept findings, repeat the probe if data/measurement/sim changed' },
    { title: 'Mutations', detail: 'new and touched C++/asset/commandlet patches serially, Python ones, apply_after, retakes, clean rebuild' },
    { title: 'Final', detail: 'runner full -Data -UpdateBaseline once, seed42 shape, N 5 items 8 and 10 before the bundle' },
    { title: 'Bundle', detail: 'bundle B 540 runs on the final build, judgments, archive; records drafted in parallel' },
    { title: 'Records', detail: 'numbers into the records, L5 check with refuters, fixes; the deferred windowless shot in parallel' },
  ],
}

const SCR = 'C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad'
const B8 = `${SCR}/b6b8`
const B7 = `${SCR}/b6b7`
const BRIEF = `${B8}/brief_final.md`
const WORK = `${B8}/brief_working.md`
const EFF = 'high'

const COMMON = `
Project: Unreal C++ game Cosmic Ascension (UE 5.8.3), project root = your working directory. No other workflow builds or runs the
commandlet in this tree. HEAD = 220129d [B-6b7] (on top of 6ae9060 [B-6b6], 10205b6 [B-6b5], 529cab9 [B-6b4]) = the brief's
H-triple-prime. This commit is [B-6b8] data 3: the side rows event_era_endured and enc_witness, then bundle B and the judgments
written ahead in H section 5.3 (H section 8 row; decisions 20-22). THE SPEC is ${BRIEF} (sections 0-7 and the critique dispositions
bind; its HEAD line numbers are older - re-find every edit by its text). After the start stage the filled placeholders live in ${WORK}
(it wins over the brief where they differ). N = docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race_notes.md (section 1.5 holds the
audited row texts C6/C7 - authoritative), H = docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md (now 148,283 bytes; keep it
< 149,000 - this commit's section 11 line < 400 bytes; move H body bytes to N if needed).
Baselines now (= [B-6b7]'s, Tools/baselines/seed42.json): 3,000 2557850423 / 2906170550; ruler 3,000 1189870756 / 3359656751;
wanderer 6,000 1631315934 / 489794244; 20,000 1957158585 / 3764704052; ruler 20,000 1375149537 / 510604211. Saved\\Balance\\b6b7_on\\
holds the five on-runs with exactly these keys and the binaries are a clean Rebuild.bat of HEAD (end of [B-6b7]). The
-WithoutB6bData=1 pair must give [B-6b5]'s keys: 2816687688 / 3300228750; 3561771757 / 2462331303; 2665013209 / 2229239127;
2359424635 / 726895141; 1611658162 / 29194274. Values: r 0.375, P_bind 1,800, N 1,200, M_AWAKE 0.5, T_DEATH 5,000 (N section 2
'[B-6b6]의 다시 셈': t = 5,000).
CHANGES SINCE THE BRIEF WAS WRITTEN ([B-6b7] 220129d): the WithoutB6bData tripwire pins eight names (this commit makes it ten);
Sim.PhaseTwoExit already runs on CosmicTests::WithoutB6bData(Original) (finding a seed that works on full game data is [B-6b12]'s) -
the brief's 2.4 PhaseTwoExit row is therefore moot, keep it so (the switch removes the era and witness rows too); Feats.Reachable is
at the fifth rung; DT_Edicts 10 rows, DT_Feats 5 rows; [B-6b7]'s balance hand-offs to bundle B (N section 3 '묶음 A′', N section 5
[B-6b7]): (L4-2) the ruled fourth->fifth season is read as measured in judgment (3); (L4-3) record the ruled people's third-rung
workload under AI-held home-galaxy laws per 600-tick window as a record-only item if the bundle's lines give it, else '재지 않음';
#136 stays with bundle C.
ORDER CHANGE (lead decision; record it in N section 5 [B-6b8] as a deviation from brief section 7): the C++/asset/commandlet mutations
run BEFORE the final runner and bundle B, so that a test fixed for a surviving mutation is already in the build bundle B runs on;
after bundle B only records and analysis change. The [B-6b8b] design already exists (${SCR}/b6b8b/brief_final.md) - do not start one.
Harnesses: ${B8}/bundleB.py is bundle B's (self-test ${B8}/bundleB_selftest.py). ${SCR}/b6b9/ belongs to [B-6b9] and holds frozen
readings: never edit anything under ${SCR}/b6b9/ (b6b9/brief_final.md sections 1.1-1.5 hash to b6b9/readings.sha1).
Hard rules: never commit or push; never touch STATUS.md or docs/OVERVIEW.html (the lead does STATUS); never read or run anything under
C:/Users/rhwwl/Downloads/jehoba; docs/90_LORE/cosmic_species_codex.md is read-only; add no Bible verse; source files ASCII only (Korean
only in Tools/data, ST_CA.csv and docs; Korean in tests as \\u escapes); every file you write keeps LF endings and files you did not
mean to change stay byte-identical to HEAD. Regenerate assets with python -I Tools/make_data_tables.py (editor closed; docs/40) - the
commit set is DT_Events.uasset, DT_Encounters.uasset and generated.sha1 only; any other modified asset: stop and find out why. Header
changes and new source files need a full editor-target build (Build.bat, docs/40). Verify any engine API you have not seen in this
codebase in the local engine source (docs/45). Run the editor runner with the PowerShell tool or powershell.exe and a Windows path
(bash eats the backslash in Tools\\run_checks.ps1). Never run two editor/UBT/commandlet processes that build or write binaries at once;
the bundle harness refuses to start while one runs. Snapshot: python -I -X utf8 ${SCR}/b6b/snap_all.py snap|restore|check <dir>;
restore after a stopped run: python -I -X utf8 ${SCR}/b6b/restore_tree.py <snapshot dir> (it keeps old mtimes, so rebuild with
Rebuild.bat after a restore). C++ mutation runner: python -I -X utf8 ${SCR}/b6b/run_mutations2.py Tools/mutations/<dir>/<patch>=<test
filter> ...; Python mutation runner ${SCR}/b6b/run_py_mutations.py. Bash heredocs here eat backslashes - write scripts with a
file-writing tool.
LESSONS (apply them): (1) git apply / git apply -R in this tree writes CRLF - after a revert put LF back or restore from the snapshot;
(2) make_data_tables runs the checker unit tests before it writes, so an asset patch that breaks a checker test never reaches the
assets and falsely 'survives' - an asset patch must keep the checker tests passing so it reaches the asset and breaks the C++ test;
(3) PowerShell 5.1's ConvertTo-Json reshapes Tools/baselines/seed42.json - after a runner with -UpdateBaseline put it back to HEAD's
shape (indent 4, LF, final newline, no BOM, no \\u0027) - adapt ${B7}/fix_seed42.py (it also compares the five lines with an on-run
folder) into ${B8}/; (4) OPEN_QUESTIONS #164-#166 are reserved for [B-6b12] and #167 is taken - a new question takes #168 on;
(5) records are dated 2026-11-10; (6) the smoke tier does not run every test (in [B-6b7] Sim.PhaseTwoExit failed only in the full
runner) - grep the tests that run long game-data worlds or put a wanderer on claimed systems before trusting the smoke pass;
(7) a game-data change can make an old patch survive because a test fixture no longer tells the cases apart (in [B-6b7]:
b6b4_clock_tip_caller_always_rung, b4b_theodicy_cap_only) - fix the test, never the patch; (8) bundleB.py --check-only writes
summary.json/.md into the checked folder unless the harness stage back-ported [B-6b9]'s F4 guard - never point it at an archive
under the project tree without --summary-dir.
SPEED RULES (developer 2026-11-09, standing - N section 5 [B-6b3b] '일의 속도'): never run the same verification twice on the same
bytes; the probe stage's smoke pass with -Data -UpdateBaseline is the build pass and only the final stage runs Tools\\run_checks.ps1
-Tier full; mutations = new ones plus old ones whose patched file or filter test this commit touches; no whole-block rerun (that is
[B-6b12]).
Theology (standing): the holy order (God) is never an entity, speaker, number, lever or end condition; first person only for the
fallen; no soul-counting; no saviour titles, no 'became god'; a wanderer's card on the claim is a sign seen or measured, never
testimony - the sign has no voice, no claimant is named, no '+0.2' or '+0.1' appears in a line, the vessel gets no title.
Plain Korean in your final report, no coined shorthand.`

phase('Start')
const [start, harness] = await parallel([
  () => agent(`You prepare the start state of commit [B-6b8] (brief section 0, items 1-10; order-of-work step 1).${COMMON}

1. Items 1-2: git log --oneline -5 and a clean tree (the ~240 ' M' entries whose bytes equal HEAD are OneDrive stat noise - confirm
   with git diff --numstat); N section 1.5 holds C6 (event_era_endured) and C7 (enc_witness) - if empty, STOP and report.
2. Item 3: the schema at HEAD (dawn_words, silence_words, desolation_words, the DT_Events era rule) and the walker-count pin's value
   in Tools/test_data_schema.py.
3. Item 4: placeholders, read never invented: T_DEATH (expect 5000), N / P_bind / r from data (1200 / 1800 / 0.375),
   A_SEC_PER_20K from N section 3 '묶음 A' header line, B_MINUTES = A_SEC_PER_20K x 540 / 6 / 60, B6b7_KEYS = the five lines of
   seed42.json.
4. Item 5: wc -c H and whether this commit's line fits under 149,000.
5. Item 6: re-derive every HEAD line the brief cites (sections 1-6) and write the new numbers into the working copy.
6. Item 7: apply_before.txt in a scratch copy of HEAD (never git apply in the project) into ${B8}/apply_before.txt.
7. Items 8-9: no UnrealEditor/UBT process (tasklist). Verify Saved\\Balance\\b6b7_on\\'s five keys equal seed42.json letter for
   letter (ruler and wanderer tails included, the 'wrote' tail aside) and copy that folder to Saved\\Balance\\b6b8_base\\; only if they
   differ, build clean and run the five base runs with the runner's exact arguments (Tools/run_checks.ps1).
8. Item 10: the tripwire reads the eight names before EventEraEndured and EncWitness.
9. Grep (lesson 6) the tests that run a ruled people under pressure for >= 600 ticks on the game's DT_Events or put a wanderer on a
   claimed system on the game's DT_Encounters, beyond the brief's 2.4 table; list them with their tier (smoke or full only).
10. Write ${WORK}: a copy of the brief with every placeholder filled, the re-derived lines, a header saying where each value came
   from, and the COMMON 'changes since the brief' and 'order change' folded in.
Change no project file. Final report (Korean): each item's result and value, the files written, anything that stops the commit.`,
    { label: 'start', phase: 'Start', effort: EFF }),
  () => agent(`You check bundle B's harness before it is used (brief section 4.2 'Self-test first'; the harness is already written).${COMMON}

1. Compare ${B8}/bundleB.py with ${SCR}/b6b9/bundleB.py and ${SCR}/b6b9/review/bundleB_before_fix.py (the [B-6b9] copy, extended for
   B-prime and fixed by a review: F1-F6). Back-port into ${B8}/bundleB.py only the fixes that touch bundle-B mode (not the B-prime,
   probe, classify or ladders additions): at least F4 - --check-only must never write summary.json/.md under --root or into a folder
   holding manifest.sha1; add --summary-dir. Add matching cases to ${B8}/bundleB_selftest.py and run it (all pass). Keep LF.
   Never edit anything under ${SCR}/b6b9/.
2. sha1 Saved\\Balance\\b6b_bundleA\\summary.json, summary.md and runs.json; run --check-only on that folder with --cell-set A and
   --summary-dir ${B8}/selfcheck_A (brief 4.2: bundle A's verdict must reproduce - compare with b6b_bundleA\\summary.json); sha1
   again - unchanged.
3. --dry-run with --t-death 5000 --r 0.375: 540 jobs, and the B_MINUTES estimate (bundle A's wall seconds per 20,000-tick run from
   N section 3 '묶음 A' or b6b_bundleA\\runs.json).
4. Check the parser against HEAD's Tests/CosmicBalanceCommandlet.cpp (git show HEAD:...): the race, ending, off and wanderer line
   formats; the brief's 2.3 new side layout; EXPECTED_REMOVED holds the ten names; the off line's exact HEAD text for
   'AIUpperRungProgressMultiplier 1 (was 0.375), FleetDelayCountdownShare 0 (was 0)'.
Write nothing in the project. Final report (Korean): what was back-ported, the self-test count, the bundle A reproduction and the sha1
check, the dry run, the format checks, and what the probe stage's --max-seed 2 smoke must still prove.`,
    { label: 'harness', phase: 'Start', effort: EFF }),
])

phase('Implement')
const impls = await parallel([
  () => agent(`You are implementer I1 (data, schema, assets, data docs) of commit [B-6b8].${COMMON}

Start-state report:
${start}

YOUR PART (from ${WORK}): sections 1.1 and 1.2 (the two rows - texts copied letter for letter from N section 1.5 C6 and C7, never
re-worded), 1.4 (Rules W and E, the two word lists, Tools/test_data_schema.py: the Rule E passing-case fixes first, then the walker
pin set from the schema, rule_setups, test_the_witness_words; every exact list or forbidden pin the lists touch), 1.5 (assets with
make_data_tables - commit set DT_Events.uasset, DT_Encounters.uasset, generated.sha1 only), the schema/data patch files of section 5
(b6b8_schema_witness_rule_dropped, b6b8_schema_witness_words_off_body, b6b8_schema_witness_past_forms_dropped,
b6b8_data_witness_says_jeungeon, b6b8_schema_era_shape_dropped, b6b8_data_era_no_ruled, and the two asset patches
b6b8_data_era_borders_0_3 and b6b8_data_witness_reverence_0_3 - lesson 2) with README rows; run the Python-only ones with the Python
mutation runner (all KILLED) - the asset ones run later in the mutation stage. Section 6 docs that are about data: docs/21 section 7.3
(lists, Rules W and E), the markers of section 6 that name these rows (docs/10_DESIGN crisis_and_transcendents, pillars, systems.md
:196 with '카드 다섯' -> 여섯, docs/20, docs/21) - present tense where now in data. Then the python -I -X utf8 checks: Tools/check_data.py,
Tools/test_data_schema.py, Tools/check_layers.py --strict, Tools/make_config_reference.py --check, Tools/check_docs.py. Do NOT build
and do NOT touch Source/ (implementer I2 works there in parallel). Final report (Korean): every change, check results, Python mutations,
anything you read differently from the brief.`, { label: 'implement:I1-data', phase: 'Implement', effort: EFF }),
  () => agent(`You are implementer I2 (C++) of commit [B-6b8].${COMMON}

Start-state report:
${start}

YOUR PART (from ${WORK}): section 1.3 (the row-name comments), 2.1 (the tripwire: ten names), 2.2 (Sim.EraEndured and Sim.WitnessCard
in a new Tests/CosmicSideRowTests.cpp, or in the existing files if copying helpers would take more than ~60 lines - say which), 2.3
(NoCheckByRow in World/CosmicWandererPolicy.h/.cpp, WitnessFirst and the reprinted side part in Tests/CosmicBalanceCommandlet.cpp -
no draw, no state, not saved, not hashed), 2.4 by reading (decide what can be decided without a run; leave the rest for the probe
stage). The C++ patch files of section 5 (b6b8_wanderer_nocheck_not_by_row, b6b8_cmdlet_witness_first_any_card) with README rows,
not run. The rerun set of section 5, built mechanically from the files this commit edits, into ${B8}/rerun_set.txt (patch=filter per
line). docs/40: the race-line paragraph's new fields and the test-table rows for the two new tests and the tripwire text. Do NOT
build, do NOT run make_data_tables and do NOT touch Tools/data, schema.json, test_data_schema.py, ST_CA.csv or Config (I1 works there
in parallel; the probe stage builds both). Final report (Korean): every change, each new assertion and the patch that must break it,
every test whose expectation may change and why, the rerun set count.`, { label: 'implement:I2-cpp', phase: 'Implement', effort: EFF }),
])

phase('Probe')
const PROBE_STEPS = `1. Editor closed: a full build of the editor target (headers and a new file), then powershell -ExecutionPolicy Bypass -File
   Tools\\run_checks.ps1 -Tier smoke -Data -UpdateBaseline (it builds both targets). If the build or a test fails, find the smallest
   cause: a test's wrong assumption about the world is fixed in the test under the brief's 2.4 rules (Sim.WandererLoop: never switch
   off B6b data - find the cause and record it; if seed 4's loop no longer closes because of the witness or the era, that is a
   [개발자] finding, report it); a real defect stops and is reported. Repeat.
2. The seed-42 probe (brief 3.3): the five on-runs with the runner's exact arguments in Saved\\Balance\\b6b8_on\\ (copy the runner's own
   outputs - csv, _rows, _claims, logs, the two hash-run logs - and run the two 20,000-tick ones), each race line's era and witness
   parts; check a: five off-runs with -WithoutB6bData=1 give [B-6b5]'s keys letter for letter (the 'wrote' tail aside) and each off
   log holds the ten-name line with 'AIUpperRungProgressMultiplier 1 (was 0.375)'; check b against Saved\\Balance\\b6b8_base\\ under the
   brief's 3.3 2b rules (T_B; a run whose race line says era 0 and witness 0 gives B6b7_KEYS letter for letter); check c: 0 'end check',
   0 'race check', 0 'LogCosmic: Error' in every log. A failure of a or b stops (brief risk 2): something reads the rows early - find
   it and report.
3. python -I -X utf8 ${B8}/bundleB.py --max-seed 2 --t-death 5000 --r 0.375 --folder ${B8}/smoke2 --no-archive (18 processes, outside
   OneDrive): every control passes and the new side layout parses (witness first at, the NoCheck count).`
const probe = await agent(`You run the build pass and the seed-42 probe of commit [B-6b8] (order-of-work step 3; brief 3.3).${COMMON}

Start: ${start}
Harness: ${harness}
I1: ${impls[0]}
I2: ${impls[1]}

${PROBE_STEPS}
Final report (Korean): the moved lines (old -> new, each line's first mover E or W, which stayed bit-identical), checks a-c, the smoke,
every test fixed and why, files written.`, { label: 'probe', phase: 'Probe', effort: EFF })

const FIND = {
  type: 'object',
  properties: { findings: { type: 'array', items: { type: 'object', properties: {
    id: { type: 'string' }, severity: { type: 'string' }, where: { type: 'string' }, claim: { type: 'string' },
    evidence: { type: 'string' }, fix: { type: 'string' } }, required: ['id', 'severity', 'where', 'claim', 'evidence', 'fix'] } } },
  required: ['findings'],
}
const LENSES = [
  { key: 'data', prompt: "L1 data, text and theology: both rows letter for letter against N section 1.5 (C6, C7); the labels reread; the epitaph question (brief risk 6 - decide with the precedent stated: default 'leave it as it is'; take the risk-6 edits only if the title reads as the claimant's shadow framing a death); Category Event.Civ.Society; Rules W and E (lists, scope, the past forms, '기록' kept out of the non-success columns); guardrails: no claimant named, no voice for the sign, no echo of testimony, no '+0.2' or '+0.1' in a line, no title on the vessel; the docs markers turned present tense." },
  { key: 'determinism', prompt: "L2 determinism and baselines: section 3.1 'nothing reads either row earlier' (StandingHeld, the Standing walk's CheckAll, the ruler's petition walks skip wanderer cards); the knowing-column and waiting-slot confirmation behind the per-cell T_B of 3.2 (no CSV/_rows/_claims column reads Knowing; a waiting wanderer card gates no world path in a run without answers) - confirm or require --w-all-cells; checks a-c really ran and pass; which lines moved and their first movers; NoCheckByRow and WitnessFirst take no draw and touch no state." },
  { key: 'tests', prompt: "L3 tests, mutations and build: section 2.2's two tests (every brief case present; the stream checks real), the tripwire, section 2.4 (every test whose meaning changed updated for the right reason; lesson 6's grep list), every new assertion has a patch that will break it, the rerun set is mechanical and complete (README 'Breaks' filters in edited test files too), the apply_before sweep, README rows, the full build after header edits." },
  { key: 'measurement', prompt: "L4 measurement: ${B8}/bundleB.py and its judgment code against H section 5.3's frozen text and the brief's 4.3 frozen arithmetic (lower median, half-open whole windows with end <= the end tick, 0 windows = fail, twice-failed runs, the (가)/(나)/(다) split of judgment 2 with BindingRuledRung 1-based, judgment 3's n3 rule with RuledCivID[1] == RuledCivID[2], judgment 4's C0/C rule), the switch and field controls of 4.2, the per-cell T_B pair check, the back-ported --summary-dir guard, the record-only items with on/off/(on - off) and the same world counted once, and the --max-seed 2 smoke's output. Report only real defects with evidence." },
]

phase('Review')
const reviews = await pipeline(
  LENSES,
  l => agent(`You are review lens '${l.key}' for commit [B-6b8] (uncommitted - read git diff HEAD, ${WORK}, the harness and the probe outputs).${COMMON}

Probe report:
${probe}

YOUR LENS: ${l.prompt}
Read-only except your scratch files under ${B8}/review_${l.key}/. Report only real defects with evidence; each fix exact.`,
    { label: `review:${l.key}`, phase: 'Review', schema: FIND, effort: EFF }),
  async (res, l) => {
    const fs = (res && res.findings) || []
    const vs = await parallel(fs.map(f => () => agent(`Adversarially verify one review finding for commit [B-6b8].${COMMON}

Try to refute it: re-read every cited line and output yourself. Does the rule apply? Is the fix right, inside this commit's scope (no
value is tuned here - values are [B-6b9]'s), and does it break anything? Answer confirmed / partial (with the corrected fix) / refuted
with evidence.

FINDING (lens ${l.key}): ${JSON.stringify(f)}`, { label: `verify:${l.key}:${f.id}`, phase: 'Verify', effort: EFF,
      schema: { type: 'object', properties: { verdict: { type: 'string', enum: ['confirmed', 'partial', 'refuted'] },
        corrected_fix: { type: 'string' }, reason: { type: 'string' } }, required: ['verdict', 'corrected_fix', 'reason'] } })
      .then(v => ({ ...f, lens: l.key, verdict: v }))))
    return vs.filter(Boolean)
  },
)
const findings = reviews.filter(Boolean).flat()
const kept = findings.filter(f => f.verdict && f.verdict.verdict !== 'refuted')
log(`findings ${findings.length}, kept ${kept.length}`)

phase('Fix')
let fix = 'no kept findings'
if (kept.length) {
  fix = await agent(`You fix the kept review findings of commit [B-6b8].${COMMON}

Probe report (for the steps you may need to repeat):
${probe}

Apply each finding below (the verifier's corrected fix for partial ones). If a fix changes data, Tests-module measurement code or sim
code, repeat the probe stage's steps (below) on the new bytes and update what they wrote; otherwise run only the tests and Python checks
your fixes touch, once. Write N section 5 [B-6b8]'s review line (findings, confirmed, partial, refuted - the refuted count is
${findings.length - kept.length} - and what was fixed). Final report (Korean): per finding what changed and what you re-ran, and a
hand-off file ${B8}/fix/handoff.md for the next stages.

PROBE STEPS:
${PROBE_STEPS}

FINDINGS:
${JSON.stringify(kept, null, 1)}`, { label: 'fix:b6b8', phase: 'Fix', effort: EFF })
}

phase('Mutations')
const muts = await agent(`You run the mutations of commit [B-6b8] (brief section 5) - BEFORE the final runner and bundle B (COMMON 'order change').${COMMON}

Fix report:
${fix}

1. Editor closed, nothing else building. Snapshot the tree first. Estimate the time and keep it near 60-75 minutes.
2. Serially: every new b6b8_ C++ and asset-rebuild patch, the retakes (apply_before vs a fresh apply_after on the current tree; each
   patch that stopped applying is taken again as b6b8_... and its old README row marked), and the rerun set (${B8}/rerun_set.txt,
   completed by the review's tests lens if it said so). b6b8_cmdlet_witness_first_any_card: build with it and run
   python -I -X utf8 ${B8}/bundleB.py --max-seed 2 --cells wanderer --t-death 5000 --r 0.375 --folder ${B8}/mut_smoke --no-archive -
   its W controls must stop it. Every patch must be KILLED; if one survives, fix the test (lesson 7), never the patch, and rerun it
   and its file's neighbours once. Python-only patches touched by the fix stage: rerun them with the Python runner.
3. Write ${B8}/apply_after.txt, the README rows (new, retaken, marked), restore the tree from your snapshot (LF everywhere, the
   snapshot check clean), and a clean Rebuild.bat of both targets so the final runner starts unmutated.
Final report (Korean): count run, all KILLED or not, test changes made for survivors (file, assertion, why), retakes, what is left to
[B-6b12] (the rest of the edited test files' patches), the time taken.`, { label: 'mutations', phase: 'Mutations', effort: EFF })

phase('Final')
const final = await agent(`You run the final runner of commit [B-6b8] and prepare bundle B's frozen readings (order-of-work step 6).${COMMON}

Probe report: ${probe}
Fix report: ${fix}
Mutation report: ${muts}

1. Editor closed, run once: powershell -ExecutionPolicy Bypass -File Tools\\run_checks.ps1 -Tier full -Data -UpdateBaseline, output to
   ${B8}/final_full.txt. It must end 'all passed' with 2 x (test count) Success lines and no Fail. Its five lines must equal the
   five in Saved\\Balance\\b6b8_on\\ (if the commandlet's source or the data changed after the probe, rerun the probe's checks a-c on this
   build first and refresh b6b8_on). The two hash runs equal the commandlet's 3,000 lines. Put seed42.json back into HEAD's shape and
   fill last_recorded_in_changelog by hand only if a line moved (brief 3.3 step 4; adapt ${B7}/fix_seed42.py into ${B8}/).
   If seed 42's wanderer crowning or counts moved, also update Tools/run_checks.ps1's comment and docs/40:25 (brief 3.3 step 4).
2. BEFORE bundle B starts, write into N section 5 [B-6b8] (block head dated 2026-11-10 if not there yet) item 8 - the reading of
   H:343 for judgment 2 (the (가)/(나)/(다) split and the same-side rule), labelled "[B-6b9]'s reading, not this commit's action",
   consistent with ${SCR}/b6b9/readings_1.1-1.5.md (point to it, never edit it) - and item 10 (the frozen arithmetic of brief 4.3),
   each with the local time you wrote it (date and hh:mm). python -I -X utf8 Tools/check_docs.py.
3. Record the final build's sha1 set the harness guards (the editor DLLs incl. Live Coding patch DLLs, Content/CA/Data/*.uasset,
   Tools/data/generated.sha1) in ${B8}/final_build_sha1.txt.
Final report (Korean): the runner's last line and Success count, the five lines verbatim (old -> new, which stayed), the hash runs,
seed42.json state, the time items 8 and 10 were written, the sha1 file.`, { label: 'final', phase: 'Final', effort: EFF })

phase('Bundle')
const [bundle, drafts] = await parallel([
  () => agent(`You run bundle B of commit [B-6b8] on the final build (brief section 4; order-of-work step 7) and print its judgments.${COMMON}

Final-runner report: ${final}

1. No UnrealEditor/UBT process; compare the guarded sha1 set with ${B8}/final_build_sha1.txt (must match). Check free memory and
   choose 6 workers (8 only if free memory clearly covers 8 editors at their measured peak plus 2 GB).
2. Run: python -I -X utf8 ${B8}/bundleB.py --t-death 5000 --r 0.375 --folder ${B8}/bundleB --serial-logs <the final runner's log
   folder holding the 'stages:' lines> [--a-sec-per-20k from the start report] (archive to Saved\\Balance\\b6b_bundleB\\ on). Run it in
   the background and wait (check every few minutes; never start anything else that builds). Rerun failed runs once as the harness
   does; two or more twice-failed runs in one cell -> report for [개발자] before any judgment.
3. When it ends: the verdict, the controls, the pair checks, the sha1 guard, judgments 1-4 with Wilson intervals, slowR's 2, E_1..E_60,
   C0 and C, 4(i)/(ii), the (가)/(나)/(다) split, the record-only items, B_LOCKED, B_AI_US, B_MINUTES. If the folder copy to
   Saved\\Balance\\b6b_bundleB\\ (logs + manifest) did not happen, do it. A failed judgment does not stop the commit - it is recorded and
   handed to [B-6b9].
Write nothing in the project except the archive. Final report (Korean): every number above with its denominator, the paths of
summary.json/.md, anything that stops the commit.`, { label: 'bundleB', phase: 'Bundle', effort: EFF }),
  () => agent(`You draft the records of commit [B-6b8] while bundle B runs (brief section 6; no build, no commandlet).${COMMON}

Probe report: ${probe}
Fix report: ${fix}
Mutation report: ${muts}
Final-runner report: ${final}

Write directly in the project every record that does not need bundle B's numbers, and leave each bundle number as a visible
placeholder of the form <<B:NAME>> (J1, J2, SPLIT, J3_LEN, J3_DENSITY, E_LIST, C0, C, J4_I, J4_II, B_LOCKED, B_AI_US, B_MINUTES, and the
record-only items): N section 5 [B-6b8] block (나눔, 데이터, 스키마, 코드, 테스트, 변이, 리뷰 line kept, 기준값, 헤더와 다르게 1-12 -
items 8 and 10 are already written: keep their text and time; item 11 = the windowless shot, left as <<B:SHOT>>; add the order change
of COMMON as a 13th item), N section 3 '묶음 B' skeleton, the H section 11 line (with placeholders; < 400 bytes once filled - plan the
length), the DECISIONS_LOG line, the markers of brief section 6 not done by I1, docs/40 (race-line paragraph, test rows, tripwire
text) and docs/21 section 7.3 if I1/I2 left them, docs/CHANGELOG.md's work-log line. Draft the commit message in ${B8}/commit_b6b8.txt
(brief section 6 form, placeholders allowed). Then python -I -X utf8 Tools/check_docs.py (placeholders must not break it - if they do,
keep those parts in ${B8}/records_draft/ instead). Never touch Source/, Tools/data, Content/ or anything the bundle guards. Final
report (Korean): what you wrote, where every placeholder is.`, { label: 'records-draft', phase: 'Bundle', effort: EFF }),
])

phase('Records')
const [records, shot] = await parallel([
  () => agent(`You fill the records of commit [B-6b8] with bundle B's numbers (brief sections 4.3 and 6).${COMMON}

Bundle report: ${bundle}
Draft report: ${drafts}

1. Replace every <<B:...>> placeholder (except <<B:SHOT>>, which the shot stage fills) with the bundle's numbers (summary.json in
   ${B8}/bundleB, archive Saved\\Balance\\b6b_bundleB\\): N section 3 '묶음 B' (header line: date, build, processes, failures, folder and
   its copy, 0 check lines, controls, sha1, B_MINUTES; then judgments 1-4, slowR's 2, every E_i, C0 and C, 4(i)/(ii), record-only items as
   on/off/(on - off)), N section 5, H section 11 (< 400 bytes; header < 149,000), DECISIONS_LOG, the commit message draft
   ${B8}/commit_b6b8.txt. If a judgment failed, write the hand-off to [B-6b9]; if 4 gives no C, the [개발자] note (decision 35).
2. grep the project for any '<<B:' left except <<B:SHOT>>; python -I -X utf8 Tools/check_docs.py and Tools/make_config_reference.py
   --check.
Final report (Korean): every number written and where, the judgments' pass/fail, the hand-offs.`, { label: 'records-fill', phase: 'Records', effort: EFF }),
  () => agent(`You take the windowless shot deferred from [B-6b4] (brief section 7 risk 8; H:390, H:412 item 6).${COMMON}

Bundle report: ${bundle}

Only while no other process builds or runs the commandlet (bundle B is over; the records stage edits docs only). Back up
Saved/SaveGames first and restore it at the end - leave no hand-check save behind. Witness: seed s and W from the wanderer cell, save
with R + -Abdicate=1200 -Wander=1 -Seed=<s> -SaveAt=<W-30> in the game's shape (first confirm bSaveShape accepts -Abdicate and -Wander;
if it refuses, record that and skip the witness shot). Era: from the R cell, R -Seed=<s> -SaveAt=<E-30>. Start -game -RenderOffScreen
-Speed=<n> -ShotAfterSeconds=<s> (the B4 brief section 6 form; it continues the newest slot if the save gate is on -
World/CosmicGameMode.cpp). Look at each screenshot yourself (Read the image). If the log does not say Continued, write the fallback.
Do not edit the project except the N section 5 placeholder <<B:SHOT>>: replace it with one sentence (what was shot, where the
images are, what they show, or why not). Final report (Korean): the shots, what they show, the restore check.`,
    { label: 'shot', phase: 'Records', effort: EFF }),
])

const RECFIND = FIND
const l5 = await agent(`You are review lens L5 (records and docs) for commit [B-6b8] (uncommitted).${COMMON}

Records report: ${records}
Shot report: ${shot}

Check every record against its source: the numbers in N section 3/5, H section 11, DECISIONS_LOG and the commit draft against
${B8}/bundleB/summary.json, the probe and the final runner; dates; the markers of brief section 6 (present tense only where now in data);
no stale quote of the old baselines outside STATUS.md; header < 149,000 bytes and the section 11 line < 400; LF, no BOM; check_docs 0.
Report only real defects with evidence; each fix exact.`, { label: 'review:L5-records', phase: 'Records', schema: FIND, effort: EFF })
const l5f = (l5 && l5.findings) || []
const l5v = await parallel(l5f.map(f => () => agent(`Adversarially verify one records finding for commit [B-6b8].${COMMON}

Try to refute it by re-reading the cited lines and sources. Answer confirmed / partial (with the corrected fix) / refuted with evidence.

FINDING: ${JSON.stringify(f)}`, { label: `verify:L5:${f.id}`, phase: 'Records', effort: EFF,
  schema: { type: 'object', properties: { verdict: { type: 'string', enum: ['confirmed', 'partial', 'refuted'] },
    corrected_fix: { type: 'string' }, reason: { type: 'string' } }, required: ['verdict', 'corrected_fix', 'reason'] } })
  .then(v => ({ ...f, verdict: v }))))
const l5kept = l5v.filter(Boolean).filter(f => f.verdict && f.verdict.verdict !== 'refuted')

const closing = await agent(`You close commit [B-6b8] without committing.${COMMON}

Records report: ${records}
Shot report: ${shot}
L5 findings to apply (corrected fix for partial ones): ${JSON.stringify(l5kept, null, 1)}

1. Apply the L5 findings (records only - after bundle B no code, data or test changes; a code or data finding is written as a hand-off
   to [B-6br] or [B-6b9] instead) and add their count to N section 5's review line.
2. Checks: python -I -X utf8 Tools/check_docs.py, Tools/make_config_reference.py --check, Tools/check_data.py; LF everywhere and no BOM
   in changed files; ASCII in Source/; the asset set (DT_Events, DT_Encounters, generated.sha1 only); wc -c H < 149,000 and the
   section 11 line < 400; CLAUDE.md unchanged; no '<<B:' left anywhere; Saved/SaveGames restored.
3. Finalize ${B8}/commit_b6b8.txt (title and body per brief section 6, ending with the line
   Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>).
Final report (Korean): the commit file list, the five baseline lines old -> new, the judgments with numbers and hand-offs, test and
checker counts, mutation and review counts, row counts (DT_Events, DT_Encounters), and the facts the lead must put in STATUS.md
('지금', '다음 3작업' - [B-6b8b], then [B-6b9] if a judgment failed, else [B-6b10] -, '코드 사실', '기준값').`, { label: 'closing', phase: 'Records', effort: EFF })

return { start, harness, impls, probe, findings, fix, muts, final, bundle, drafts, records, shot, l5: l5v, closing }
