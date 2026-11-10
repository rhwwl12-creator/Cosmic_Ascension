export const meta = {
  name: 'b6b7-data-implement',
  description: 'Implement [B-6b7] data 2 (three galaxy-law rows, feat_laws_and_binding, binding values, r), build pass, bundle A-prime pool with checks a-c and J1-J4, four review lenses incl. the ultracode balance review, fix, final runner, mutations and race-check probes',
  phases: [
    { title: 'Start', detail: 'section 0: earlier commits, N 1.5 texts, the [B-6b6] recomputation, M_AWAKE by m_awake.py, T4* per line, apply_before, base copies' },
    { title: 'Implement', detail: 'I1 data/schema/strings/docs-30, I2 C++ tests, LayBoard and patch files (no build)' },
    { title: 'Measure', detail: 'full build, smoke -Data -UpdateBaseline, A-prime pool of 37, checks a-c, J1-J4, recorded readings' },
    { title: 'Review', detail: 'L1 data/text/theology, L2 determinism/baselines, L3 tests/mutations/build, L4 balance' },
    { title: 'Verify', detail: 'one refuter per finding' },
    { title: 'Fix', detail: 'apply kept findings, re-measure if data or sim changed' },
    { title: 'Final', detail: 'runner full -Data -UpdateBaseline once, seed42 shape, mutations, probes, clean rebuild, records' },
  ],
}

const SCR = 'C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad'
const B6 = `${SCR}/b6b6`
const B7 = `${SCR}/b6b7`
const BRIEF = `${B7}/brief_final.md`
const WORK = `${B7}/brief_working.md`
const EFF = 'high'

const COMMON = `
Project: Unreal C++ game Cosmic Ascension (UE 5.8.3), project root = your working directory. No other workflow builds or runs the
commandlet in this tree (a read-only design workflow may read it). HEAD = 6ae9060 [B-6b6] (on top of 10205b6 [B-6b5], 529cab9 [B-6b4])
= the brief's H-triple-prime. This commit is [B-6b7] data 2: fourth -> fifth rung (H section 8 row; decisions 9-15). THE SPEC is
${BRIEF} (sections 0-7 and the critique dispositions bind; HEAD line numbers in it are older - re-find every edit by its text). After
the start stage, the filled placeholders live in ${WORK} (a working copy of the brief; it wins over the brief where they differ).
N = docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race_notes.md (its section 1.5 holds the audited row texts - authoritative over
the brief), H = docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md.
Baselines now (= [B-6b6]'s, Tools/baselines/seed42.json): 3,000 2557850423 / 2906170550; ruler 3,000 1189870756 / 3359656751; wanderer
6,000 271068805 / 309834531; 20,000 3168555397 / 1640870674; ruler 20,000 4053448003 / 129517356. The -WithoutB6bData=1 pair must give
[B-6b5]'s keys: 2816687688 / 3300228750; 3561771757 / 2462331303; 2665013209 / 2229239127; 2359424635 / 726895141; 1611658162 / 29194274.
[B-6b6]'s recomputation (N section 2): r 0.375, P_bind 1,800, N 1,200, t 5,000 (direct resample 9/30 = 0.30 at r 0.375).
Hard rules: never commit or push; never touch STATUS.md or docs/OVERVIEW.html (the lead does STATUS); never read or run anything under
C:/Users/rhwwl/Downloads/jehoba; docs/90_LORE/cosmic_species_codex.md is read-only; add no Bible verse; source files ASCII only (Korean
only in Tools/data, ST_CA.csv and docs; Korean in tests as \\u escapes); every file you write keeps LF endings and files you did not mean
to change stay byte-identical to HEAD (git show HEAD:<path> bytes back). Regenerate assets with python -I Tools/make_data_tables.py
(editor closed; docs/40) - the commit set is the changed JSON's DT_*.uasset, ST_CA.uasset only if ST_CA.csv changed, generated.sha1;
any other modified asset: stop and find out why. Header changes need a full editor-target build (Build.bat, docs/40). Verify any engine
API you have not seen in this codebase in the local engine source (docs/45). Run the editor runner with the PowerShell tool or
powershell.exe and a Windows path (bash eats the backslash in Tools\\run_checks.ps1). Never run two editor/UBT/commandlet processes
that build or write binaries at once; the bundle harness refuses to start while one runs. Snapshot: python -I -X utf8
${SCR}/b6b/snap_all.py snap|restore|check <dir>; restore after a stopped run: python -I -X utf8 ${SCR}/b6b/restore_tree.py <snapshot dir>
(it keeps old mtimes, so rebuild with Rebuild.bat, not an incremental build, after a restore). C++ mutation runner: python -I -X utf8
${SCR}/b6b/run_mutations2.py Tools/mutations/<dir>/<patch>=<test filter> ...; Python mutation runner ${SCR}/b6b/run_py_mutations.py.
Bash heredocs here eat backslashes - write scripts with a file-writing tool.
LESSONS FROM [B-6b6] (apply them): (1) git apply / git apply -R in this tree writes CRLF - after a revert put LF back or restore from
the snapshot; (2) make_data_tables runs the checker unit tests before it writes, so a data patch that breaks a checker test never
reaches the assets and falsely 'survives' - an asset patch must keep the checker tests passing (adjust walked counts etc.) so that it
reaches the asset and breaks the C++ test; (3) PowerShell 5.1's ConvertTo-Json reshapes Tools/baselines/seed42.json - after the final
runner put it back to HEAD's shape (indent 4, LF, final newline, no BOM, no \\u0027) - ${B6}/fix/fix_seed42.py shows how (copy and adapt
it to this commit's hashes into ${B7}/); (4) OPEN_QUESTIONS #164-#166 are reserved by N section 0.2 for [B-6b12] and #167 is taken -
a new question takes #168 on; (5) records are dated 2026-11-10.
SPEED RULES (developer 2026-11-09, standing - N section 5 [B-6b3b] '일의 속도'): never run the same verification twice on the same
bytes; only the final stage runs Tools\\run_checks.ps1 -Tier full (the measure stage's smoke pass with -Data -UpdateBaseline is the
build pass); mutations = new ones plus old ones whose patched file or filter test this commit touches, inside the brief's 60-minute
budget (section 5); no whole-block rerun (that is [B-6b12]).
Theology (standing): the holy order (God) is never an entity, speaker, number, lever or end condition; first person only for the
fallen; no soul-counting; no saviour titles, no 'became god'; the binding never names or acts as the holy order; a law is a people's
own rule, never a decree from above. Plain Korean in your final report, no coined shorthand.`

phase('Start')
const start = await agent(`You prepare the start state of commit [B-6b7] (brief section 0, items 1-8, and order-of-work step 1).${COMMON}

1. Items 1-3: the three earlier commits (git log --oneline -4) and a clean tree (the ~240 ' M' entries whose bytes equal HEAD are
   OneDrive stat noise - confirm with git diff being empty for them); N section 1.5 holds the C4 (feat_laws_and_binding) and C5
   (edict_wide_*) rows; N section 2's paragraph '[B-6b6]의 다시 셈' - apply the guards and take P_BIND_A, N_A, R_A (expected 1800,
   1200, 0.375) and confirm t is written with n.
2. Item 4: run python -I -X utf8 ${B7}/m_awake.py (read its --help first; its self-test ${B7}/m_awake_selftest.py passed 62/62) over
   Saved\\Balance\\b6b_bundleA\\ and apply the rule to get M_AWAKE; do the #136 base reading over the [B-6b6] bundle A R on-runs (Wilson
   95%); write the paragraph '[B-6b7]의 m' into N section 2 (n, median k, a, median S at each ladder value, the #136 counts, the
   alternative recorded as considered).
3. Item 5: T4* for each of the five baseline runs from [B-6b6]'s outputs (Saved\\Balance\\b6b6_on\\ logs, race: parts 2-3) and
   LINES_MOVED_B6B7 (which lines and hash runs can move).
4. Item 6: the grep on HEAD and the list of assertions/WithFeat users that change with section 2.2.
5. Item 7: apply_before.txt in a scratch copy of HEAD (never git apply in the project) into ${B7}/apply_before.txt.
6. Item 8: no other UnrealEditor/UBT process (tasklist); if Saved\\Balance\\b6b6_on\\ exists and its five keys equal seed42.json, copy it
   as Saved\\Balance\\b6b7_base\\ (else build clean and run the five base runs with the runner's exact arguments). Confirm
   Saved\\Balance\\b6b_bundleA\\ is there. Check whether ${B6}/bundleA.py has a cell/seed filter for the section 4 pool; if it does not,
   write a filtered copy ${B7}/bundleA7.py (scratch only) and self-test it with --dry-run.
7. Write ${WORK}: a copy of the brief with every placeholder filled (P_BIND_A, N_A, R_A, M_AWAKE, LINES_MOVED_B6B7, T4* per line,
   AI_T5_MIN if the brief's section 3.1 formula can be filled now, the grep results) and a short header saying where each value came from.
Change no project file except N section 2's paragraph. Final report (Korean): each item's result and value, the files written, anything
that stops the commit.`, { label: 'start', phase: 'Start', effort: EFF })

phase('Implement')
const impls = await parallel([
  () => agent(`You are implementer I1 (data, schema, strings, docs/30) of commit [B-6b7].${COMMON}

Start-state report:
${start}

YOUR PART (from ${WORK}): sections 1.1-1.5 (the three edict_wide_* rows, feat_laws_and_binding, project_binding values, DT_Rules r,
the ST_CA Edict.TipWide fix - texts verbatim from N section 1.5: copy, do not re-word), 1.7 (the asset set), 1.8 (regenerate docs/30
with Tools/make_config_reference.py), any schema/checker work the brief puts in this commit with its Python mutations run by the Python
mutation runner (all KILLED), and the docs/records items of section 6 that are about data. Then make_data_tables and the python -I -X utf8
checks: Tools/check_data.py, Tools/test_data_schema.py, Tools/check_layers.py --strict, Tools/make_config_reference.py --check,
Tools/check_docs.py. Do NOT build and do NOT touch Source/ (implementer I2 works there in parallel). Final report (Korean): every
change, check results, Python mutations, anything you read differently.`, { label: 'implement:I1-data', phase: 'Implement', effort: EFF }),
  () => agent(`You are implementer I2 (C++) of commit [B-6b7].${COMMON}

Start-state report:
${start}

YOUR PART (from ${WORK}): section 1.6 (row names), sections 2.1-2.3 (Data.Integrity pins, the [B-6b4] UI.Chronicle block of section 2.2
with its injected cases, comments), the LayBoard change of section 2.4 and every other section 2.4 row that can be decided without running
(leave the ones that need a run - e.g. raising Feats.Reachable - as conditional patches under ${B7}/i2_conditional/ like [B-6b6] did),
and the C++ patch files and race-check probe files of section 5 (Tools/mutations/B6b/b6b7_*.patch with README rows; do not run them -
the final stage does). Do NOT build, do NOT run make_data_tables and do NOT touch Tools/data, ST_CA.csv or Config (implementer I1 works
there in parallel; the measure stage builds both). Final report (Korean): every change, each new assertion and the patch that must break
it, every test whose expectation changed and why, the conditional patches and their condition.`, { label: 'implement:I2-cpp', phase: 'Implement', effort: EFF }),
])

phase('Measure')
const measure = await agent(`You run the build pass and the measurements of commit [B-6b7] (order-of-work steps 3-4; sections 2.4, 3, 4).${COMMON}

Start-state report:
${start}

Implementer reports:
I1: ${impls[0]}
I2: ${impls[1]}

1. Before reading any number, write the pre-written judgments J1-J4 (section 4 table, with this commit's T4* values) into N section 5
   under a new '[B-6b7]' block head (dated 2026-11-10; leave the rest of the block as an HTML comment for the final stage).
2. Editor closed: a full build of the editor target (header edits), then powershell -ExecutionPolicy Bypass -File Tools\\run_checks.ps1
   -Tier smoke -Data -UpdateBaseline (it builds both targets). If the build or a test fails, find the smallest cause (a test's wrong
   assumption about the world is fixed in the test, like [B-6b6]'s Sim.Determinism; a real defect stops and is reported) and repeat.
   Copy checks*.csv with _rows/_claims and the logs into Saved\\Balance\\b6b7_on\\ right away. Read checks_20000.csv for the
   Feats.Reachable decision of section 2.4 and, if its condition holds, apply I2's conditional patch (LF), move its mutation file into
   Tools/mutations/B6b/ with its README row, and fix the docs line the brief names; rebuild the editor target and run that test once.
3. The section 4 pool of 37 (A-prime 30 R on-runs into Saved\\Balance\\b6b7_A1\\, the two 20,000 on-runs into b6b7_on\\, the five
   check-a off runs with -WithoutB6bData=1) with the harness the start stage named (6 workers, 20,000 jobs first; never beside a build).
   Then check a (the five off keys equal [B-6b5]'s letter for letter and each off log holds the section 3.2 'rows taken out [...]' line
   with R_A), check b (Tick < T4* rows equal b6b7_base; a run with no T4* inside its horizon is bit-identical including keys), check c
   (0 end check / race check lines, 0 LogCosmic errors), J1-J4. Any failure of a, b, J1-J4 stops: find the cause and report.
4. The recorded readings of section 4 (resets by upkeep, T5 by kind, other-order share with Wilson 95% and the (가)/(나) split,
   ruled T4->T5 median against S_m, ruled law resets by input against lambda, AIs that ever held three laws and the first AI Society
   1.0 tick, AI binding stockpiles, #136 on A-prime against [B-6b6] A, plague spread, extinctions per 600 ticks). Write them as N section
   3's 'A-prime' record and the '[B-6b7]의 m' comparison line.
Final report (Korean): the moved keys (which lines moved and which stayed bit-identical, against LINES_MOVED_B6B7), checks a-c,
J1-J4, the Feats.Reachable decision, the readings with their numbers, every stop condition met or not, files written.`, { label: 'measure', phase: 'Measure', effort: EFF })

const FIND = {
  type: 'object',
  properties: { findings: { type: 'array', items: { type: 'object', properties: {
    id: { type: 'string' }, severity: { type: 'string' }, where: { type: 'string' }, claim: { type: 'string' },
    evidence: { type: 'string' }, fix: { type: 'string' } }, required: ['id', 'severity', 'where', 'claim', 'evidence', 'fix'] } } },
  required: ['findings'],
}
const LENSES = [
  { key: 'data', prompt: 'L1 data, text and theology: the rows equal N section 1.5 verbatim; each Summary against its hooks (what the row really does); the overlay and feed trio of the fifth rung; the guardrails (no holy order as actor, no title, no became-god, no soul count, the binding unnamed and never the holy order\'s act); Edict.TipWide; the docs markers ([B-6b7]부터 -> present tense where now in code).' },
  { key: 'determinism', prompt: 'L2 determinism and baselines: section 3 (where the world can first diverge, the order-independence property of section 3.1, draw points fixed before rows - rule 3), J1, checks a-c really ran and pass, the off-pair line with R_A, which lines moved against LINES_MOVED_B6B7, the ending kinds against AI_T5_MIN, seed42.json and its note, the hand-off facts.' },
  { key: 'tests', prompt: 'L3 tests, mutations and build: section 2.2\'s injected cases, LayBoard, the section 2.4 table (every test whose meaning changed is updated for the right reason), every new assertion has a patch that will break it, the probes\' seeds and expected lines, the apply_before sweep and the retakes, README rows, the full build after header edits.' },
  { key: 'balance', prompt: 'L4 ULTRACODE BALANCE REVIEW (order-of-work step 5 list): M_AWAKE with #136; the warning\'s +50 on plague (set versus alone, the A-prime spread reading); cost 40 / upkeep 0; AI law reach (three slots need Society 1.0 and 120 Influence); the AI paying 4/4 for P/r ticks with possible unpaid ticks; extinction exposure; the A-prime readings; the vigil resets lambda leaves out. Nothing but M_AWAKE moves in this commit - everything else is a recommendation for [B-6b8] bundle B, [B-6b9] or [개발자]. Report findings only where the numbers call for an action or a record.' },
]

phase('Review')
const reviews = await pipeline(
  LENSES,
  l => agent(`You are review lens '${l.key}' for commit [B-6b7] (uncommitted - read git diff HEAD, ${WORK} and the measure outputs).${COMMON}

Measure report:
${measure}

YOUR LENS: ${l.prompt}
Read-only except your scratch files under ${B7}/review_${l.key}/. Report only real defects with evidence; each fix exact.`,
    { label: `review:${l.key}`, phase: 'Review', schema: FIND, effort: EFF }),
  async (res, l) => {
    const fs = (res && res.findings) || []
    const vs = await parallel(fs.map(f => () => agent(`Adversarially verify one review finding for commit [B-6b7].${COMMON}

Try to refute it: re-read every cited line and output yourself. Does the rule apply? Is the fix right, inside this commit's scope (among
values only M_AWAKE may change), and does it break anything? Answer confirmed / partial (with the corrected fix) / refuted with evidence.

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
  fix = await agent(`You fix the kept review findings of commit [B-6b7].${COMMON}

Apply each finding below (the verifier's corrected fix for partial ones). If a fix changes data or sim behaviour, repeat the measure
stage's steps 2-4 (smoke -Data -UpdateBaseline, the pool, checks a-c, J1-J4, the readings) and update the N records. Otherwise run only
the tests and Python checks your fixes touch, once. Write N section 5 [B-6b7]'s review line (findings, confirmed, partial, refuted - the
refuted count is ${findings.length - kept.length} - and what was fixed). Final report (Korean): per finding what changed and what you
re-ran, and a hand-off file ${B7}/fix/handoff_final.md for the final stage.

FINDINGS:
${JSON.stringify(kept, null, 1)}`, { label: 'fix:b6b7', phase: 'Fix', effort: EFF })
}

phase('Final')
const final = await agent(`You run the final stage of commit [B-6b7] (order-of-work steps 7-9 and sections 3.2, 5, 6, without committing).${COMMON}

Fix report:
${fix}

1. Editor closed, run once: powershell -ExecutionPolicy Bypass -File Tools\\run_checks.ps1 -Tier full -Data -UpdateBaseline, output to
   ${B7}/final_full.txt. It must end 'all passed' with 2 x (test count) Success lines and no Fail; its five lines must equal the five in
   Saved\\Balance\\b6b7_on\\ and the two hash runs must equal the commandlet's 3,000 lines. Then fill last_recorded_in_changelog by hand
   (section 3.2 item 2, the 'Older values' chain kept) and put seed42.json back into HEAD's shape (lesson 3).
2. Mutations (section 5), serially, editor closed, estimate the time first and keep the 60-minute budget split if needed: every new
   b6b7_ patch, the retakes (apply_before vs apply_after), and the old ones whose patched file or filter test this commit touched; all
   must be KILLED (fix the test, not the patch, if one survives, and report it; an asset patch must reach the asset - lesson 2). Then
   the two race-check probes with their seeds and expected lines (record 'not probed - no seed' if the brief's condition holds). Then
   ${B7}/apply_after.txt, README rows, restore the tree from your snapshot (LF), and a clean rebuild (Rebuild.bat) of both targets so the
   next commit starts unmutated.
3. Records (section 6): N section 5 [B-6b7] block complete (baseline bullet with the new five lines, two hash runs and the
   -WithoutB6bData pair; mutation counts; the split left to [B-6b12]), H section 11 line (< 400 bytes; header < 149,000 bytes - move the
   excess to N), DECISIONS_LOG, the docs the brief names, every stale quote of the old baselines outside STATUS.md; then python -I -X utf8
   Tools/check_docs.py and Tools/make_config_reference.py --check.
4. Checks: LF everywhere, ASCII in Source/, the asset set, CLAUDE.md bytes unchanged.
Final report (Korean): the runner's last line, the five new baseline lines and two hash runs verbatim (old -> new, which stayed), the
-WithoutB6bData pair result, mutations and probes (count, all KILLED?, what was left to [B-6b12]), the commit file list, the header
section 11 line bytes, and the facts the lead must put in STATUS.md (new baselines with the ruler/wanderer summary facts that moved,
test count, checker unit-test count, row counts, the A-prime headline numbers, M_AWAKE and the #136 reading, new open questions).`, { label: 'final', phase: 'Final', effort: EFF })

return { start, impls, measure, findings, fix, final }
