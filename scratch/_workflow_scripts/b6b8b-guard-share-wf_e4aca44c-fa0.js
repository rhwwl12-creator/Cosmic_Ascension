export const meta = {
  name: 'b6b8b-guard-share',
  description: '[B-6b8b] #132 guard share 0 -> 0.5 with W recounted: start checks, data+docs and tests+patches, build pass with checks (a)-(e) and bundle K, three review lenses with refuters, fix, mutations before the final runner, final runner, records; no commit',
  phases: [
    { title: 'Start', detail: 'brief section 0 checks and placeholders into brief_working.md' },
    { title: 'Implement', detail: 'I1 data and docs, I2 tests and patches' },
    { title: 'Probe', detail: 'build pass, checks (a)-(e), bundle K; lenses L2 and L3 in parallel' },
    { title: 'Review', detail: 'L1 on the probe results; one refuter per finding' },
    { title: 'Fix', detail: 'apply findings; re-probe if data or sim changed' },
    { title: 'Mutations', detail: 'three new and the narrow rerun set, serially; clean rebuild' },
    { title: 'Final', detail: 'runner full -Data -UpdateBaseline once' },
    { title: 'Records', detail: 'N section 5, H section 11, DECISIONS_LOG, STATUS facts, commit message' },
  ],
}

const SCR = 'C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad'
const ROOT = 'C:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension'
const W = SCR + '/b6b8b'
const BRIEF = W + '/brief_final.md'
const WORKING = W + '/brief_working.md'

const CONTEXT = `
Project: Cosmic Ascension (UE 5.8.3 C++ module Cosmic_Ascension), repository ROOT = ${ROOT} (main; HEAD 84f0d36 = [B-6b8], pushed). You implement block B6b's step [B-6b8b] exactly as its brief says: BRIEF = ${BRIEF} (written and reviewed before; {H8B} in it = 84f0d36). Its section 7 gives the order of work; this workflow follows it with ONE lead change already decided: mutations run BEFORE the final runner (as in [B-6b8]), so the build that is measured and the build that is committed are the same - record this order change in N section 5. The working copy of the brief (filled placeholders, re-derived line numbers, drift) is ${WORKING}; once written it wins over BRIEF.
B6b header H = docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md, notes N = ..._notes.md.

RULES (all agents)
- Plain Korean in every doc line; English ASCII comments in source. CLAUDE.md applies in full.
- Speed rules: the runner -Tier full runs ONCE (its two in-process passes are the two consecutive test runs); mutations = this step's new patches + the narrow rerun set of brief section 5; never the whole B6b set.
- Runner: from PowerShell, editor closed: Remove-Item Env:PROJECT_ROOT -ErrorAction SilentlyContinue; powershell -ExecutionPolicy Bypass -File Tools\\run_checks.ps1 -Tier <smoke|full> [-Data] [-UpdateBaseline]. After -UpdateBaseline put Tools/baselines/seed42.json back into its committed shape with python -I -X utf8 ${SCR}/b6b8/fix_seed42.py (no flags - shape only; its --from-on/--final read b6b8_on, do not use them here).
- Line endings: LF. git apply / PowerShell can write CRLF - check changed files for CR before handing over.
- Never touch docs/OVERVIEW.html. Never edit or run anything in scratchpad/b7x/** (another workflow integrates block B7x in separate worktrees - scratchpad/b7x/int via the junction C:/Users/rhwwl/AppData/Local/Temp/b7xi, and scratchpad/b7x/int3).
- BUSY RULE: the bundle K harness (a copy of bundleA.py) refuses to START while any UnrealEditor/UBT process runs. Processes of the B7x worktrees (their command line contains 'b7x' or 'b7xi' or 'b6bt') belong to another workflow - never kill them; wait for them (PowerShell loop: check every 60 s with Get-CimInstance Win32_Process, up to 60 minutes) and then start. Build.bat holds a lock file, so a build simply waits for another build.
- Do not commit or push (the lead does). Do not edit STATUS.md lines beyond those the brief assigns (:7/:38/:40 by I1, :27/:32 by the records agent).
- Trailer for the commit message file: Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
`

const OUT = {
  type: 'object',
  properties: {
    status: { type: 'string', enum: ['ok', 'stop', 'partial'] },
    summary: { type: 'string' },
    files_changed: { type: 'array', items: { type: 'string' } },
    numbers: { type: 'array', items: { type: 'string' } },
    open_points: { type: 'array', items: { type: 'string' } },
  },
  required: ['status', 'summary', 'files_changed', 'numbers', 'open_points'],
}

const FINDINGS = {
  type: 'object',
  properties: {
    findings: {
      type: 'array',
      items: {
        type: 'object',
        properties: {
          id: { type: 'string' },
          severity: { type: 'string', enum: ['high', 'medium', 'low'] },
          file: { type: 'string' },
          location: { type: 'string' },
          claim: { type: 'string' },
          evidence: { type: 'string' },
          fix: { type: 'string' },
        },
        required: ['id', 'severity', 'file', 'location', 'claim', 'evidence', 'fix'],
      },
    },
  },
  required: ['findings'],
}

const VERDICT = {
  type: 'object',
  properties: {
    verdict: { type: 'string', enum: ['confirmed', 'partial', 'refuted'] },
    reason: { type: 'string' },
    corrected_fix: { type: 'string' },
  },
  required: ['verdict', 'reason', 'corrected_fix'],
}

phase('Start')
const start = await agent(`${CONTEXT}

YOUR TASK (start, effort high): do brief section 0 items 1-6 on {H8B} = HEAD 84f0d36 and write ${WORKING} (a copy of the brief with every placeholder filled, every line number re-derived, every drift from the brief listed at the top, the order change noted). Stop conditions in section 0 (a new non-comment .cpp/.h hit of FleetDelayCountdownShare, the W probe's claim line not 550 / 0.2629, a dirty tree) -> status 'stop' with the reason and do nothing else. Save hits_head.txt, hits_h8b.txt, apply_before.txt in ${W}. Record {PRED} from the current Saved/Logs/checks*.log (the [B-6b8] final runner's logs).
Return status, summary, files_changed (scratch files), numbers (W_TABLE lines, H bytes, PRED per run), open_points.`, { label: 'start', phase: 'Start', schema: OUT, effort: 'high' })
log(`start: ${start ? start.status : 'FAILED'}`)
if (!start || start.status === 'stop') {
  return { start }
}

phase('Implement')
const impl = await parallel([
  () => agent(`${CONTEXT}

YOUR TASK (I1 data and docs, effort high): per ${WORKING} sections 1 and 6 and section 7 step 2 'I1': the DT_Rules value, the header comment in Data/CosmicDefinitionRows.h (ASCII), docs/30 regeneration (python -I -X utf8 Tools/make_config_reference.py), every doc sentence the brief lists as made false (docs/20, docs/21 two places, docs/40 two rows, docs/10_DESIGN/systems.md), the OPEN_QUESTIONS #132 row deleted with the B7 line placed as the brief says, STATUS.md :7/:38/:40 as the brief says (re-derive the lines). Do not write the records (N section 5, H section 11, DECISIONS_LOG, STATUS :27/:32) - the records agent does that after the measurements. python -I -X utf8 Tools/check_data.py must report 0; check_docs 0.
Return status, summary, files_changed, numbers, open_points.`, { label: 'I1:data+docs', phase: 'Implement', schema: OUT, effort: 'high' }),
  () => agent(`${CONTEXT}

YOUR TASK (I2 tests and patches, effort high): per ${WORKING} section 2 'Sim.FleetDelay' (file comment, the 0.5 pin, the relabel, the game awakening pair 225 and the share-0 pair 250 exactly as written) and section 5's three new patch files under Tools/mutations/B6b/ (b6b8b_data_share_off, b6b8b_data_share_third, b6b8b_share_zero_is_a_share) written as unified diffs against the tree after I1's DT_Rules edit (I1 runs in parallel - write the data patches against the 0.5 line the brief gives; prove each with git apply --check after I1 is done if you can, else say so). Do not run them. Do not touch Tools/data or docs.
Return status, summary, files_changed, numbers, open_points.`, { label: 'I2:tests+patches', phase: 'Implement', schema: OUT, effort: 'high' }),
])
log(`implement: ${impl.map((x) => (x ? x.status : 'FAILED')).join(', ')}`)

phase('Probe')
const PROBE_PROMPT = `${CONTEXT}

Implementers' reports: ${JSON.stringify(impl)}
YOUR TASK (probe, effort high): ${WORKING} section 7 steps 3 and 4 'Machine':
1. Editor closed. Runner -Tier smoke -Data -UpdateBaseline; fix_seed42.py shape; diff seed42.json against HEAD: the input-less summary_line must be unchanged (check (a) first half).
2. Check (b): the five runs with the runner's exact arguments plus -Rules=FleetDelayCountdownShare:0 into Saved/Balance/b6b8b_share0/ (absolute -Out, own -AbsLog), each log showing the rules line and no '-Rules cannot set'. (c) optional - skip unless time is plenty.
3. Bundle K (brief section 4): create ${W}/bundleK.py from ${SCR}/b6b6/bundleA.py exactly as section 4 says (6 workers - use 4 if free memory is under 8 GB; sha1 guard on the module DLL and DT_Rules.uasset; ONE -Rules argument for the twin), obey the BUSY RULE, run the 300 runs into Saved/Balance/b6b8b_bundleK/, read K1-K3.
4. (d) and (e) over all logs.
Write everything to ${W}/probe/ (logs, a probe_out.md with every number and judgment). If a judgment says stop, return status 'stop' with the numbers.
Return status, summary, files_changed, numbers (every check and K value), open_points.`
const LENS = (key, prompt) => `${CONTEXT}

You are a reviewer (read-only - do not edit). Implementers' reports: ${JSON.stringify(impl)}
${prompt}
Report real defects only with evidence (file:line, quoted text) and an exact fix. Severity: high = wrong value/assertion, a broken rule, a false sentence left; medium = inconsistency; low = wording.`

const [probe, l2, l3] = await parallel([
  () => agent(PROBE_PROMPT, { label: 'probe', phase: 'Probe', schema: OUT, effort: 'high' }),
  () => agent(LENS('L2', `LENS L2 tests and mutations: label truth of Sim.FleetDelay; the new assertions against the three patches (would each patch fail them?); the narrow rerun set (re-derive it on the tree: patches touching Tools/data/DT_Rules.json and README rows whose Breaks column is Sim.FleetDelay, plus [B-6b6]-[B-6b8] additions); the expected double breaks of brief section 5 (confirm with python -I ${W}/w_recount.py Tools\\data 0.5 and the patched values).`), { label: 'L2:tests', phase: 'Probe', schema: FINDINGS, effort: 'high' }),
  () => agent(LENS('L3', `LENS L3 docs and records: every sentence the brief lists as made false is fixed exactly; no other sentence about the share/cap/#132 is left false (grep FleetDelayCountdownShare, '#132', '66.7', '지금 셈', '지금 값' across docs/ and STATUS.md); the #132 row deletion and the B7 line; theology/plain words; H byte budget (wc -c H; the margin 149,000).`), { label: 'L3:docs', phase: 'Probe', schema: FINDINGS, effort: 'high' }),
])
log(`probe: ${probe ? probe.status : 'FAILED'}`)
if (probe && probe.status === 'stop') {
  return { start, impl, probe, l2, l3 }
}

phase('Review')
const l1 = await agent(LENS('L1', `LENS L1 determinism and baselines: read ${W}/probe/probe_out.md and the logs it names. Check the H :334 addition and the one-tick reign tail as the brief words them; checks (a)-(e) and K1/K2 applied exactly (the two-sided rule (i)-(iii) per pair from the twin's conflict: Hazard.AIAwakening=<sum>(<count>)); the prediction PRED against what happened; whether any input-less run moved (risk 2).`), { label: 'L1:determinism', phase: 'Review', schema: FINDINGS, effort: 'high' })

const lensResults = [['L1', l1], ['L2', l2], ['L3', l3]]
const verified = await parallel(lensResults.flatMap(([key, res]) => ((res && res.findings) || []).map((f) => () =>
  agent(`${CONTEXT}

You are a skeptical verifier. Try to REFUTE this finding about the [B-6b8b] working tree (read the files and logs). If wrong: refuted. If right but the fix is wrong: partial with corrected_fix. If right: confirmed.
Finding (${key}): ${JSON.stringify(f)}`, { label: `verify:${key}:${f.id}`, phase: 'Review', schema: VERDICT, effort: 'high' })
    .then((v) => ({ lens: key, finding: f, verdict: v })))))
const all = verified.filter(Boolean)
const kept = all.filter((x) => x.verdict && x.verdict.verdict !== 'refuted')
log(`review: ${all.length} findings, kept ${kept.length}`)

phase('Fix')
let fix = null
if (kept.length > 0) {
  fix = await agent(`${CONTEXT}

YOUR TASK (fix, effort high): apply the kept findings below (corrected_fix when partial). If a fix changes Tools/data, the sim or the measurement, repeat the probe's step 1 and the affected checks and say so; if it changes only tests or docs, do not re-run the bundle. Write ${W}/fix/handoff.md listing what changed and what the next stages must redo.
KEPT: ${JSON.stringify(kept.map((x) => ({ lens: x.lens, id: x.finding.id, severity: x.finding.severity, file: x.finding.file, location: x.finding.location, claim: x.finding.claim, fix: x.finding.fix, verdict: x.verdict.verdict, corrected_fix: x.verdict.corrected_fix })), null, 1)}
Return status, summary, files_changed, numbers, open_points.`, { label: 'fix', phase: 'Fix', schema: OUT, effort: 'high' })
}

phase('Mutations')
const muts = await agent(`${CONTEXT}

Reports: probe ${JSON.stringify(probe)} fix ${JSON.stringify(fix)}
YOUR TASK (mutations, effort high): brief section 5 - the three new patches and the narrow rerun set (re-derive mechanically on the tree; write the list to ${W}/rerun_set.txt), run SERIALLY with the editor closed from the repository root: C++/asset patches with python -I -X utf8 ${SCR}/b6b/run_mutations2.py <patch>=Cosmic_Ascension.<test> ... (data patches need the assets rebuilt after apply and after revert - follow how [B-6b7]/[B-6b8] ran their data patches, see ${SCR}/b6b8/fix/handoff.md and ${SCR}/b6b8/rerun_set.txt), Python-only patches outside Tools/mutations/B6b with ${SCR}/b6b8/fix/run_py_paths.py. Snapshot the tree first with python -I -X utf8 ${SCR}/b6b/snap_all.py snap ${W}/mut_snap and restore with ${SCR}/b6b/restore_tree.py if anything stops. A surviving patch means: fix the TEST (never the patch), then re-run that patch. Then git apply --check every patch under Tools/mutations into ${W}/apply_after.txt and compare with apply_before.txt (retake any patch that went from applies to fails under a b6b8b_ name); add the README rows (data names into the data-patch list at the top, the expected double breaks and the b4d2 row note exactly as the brief says). Finally rebuild both targets clean (Rebuild.bat, editor closed) so the final runner starts unmutated, and check the tree has no CR in changed files.
Return status, summary (killed/survived counts, which test fixes), files_changed, numbers, open_points.`, { label: 'mutations', phase: 'Mutations', schema: OUT, effort: 'high' })

phase('Final')
const final = await agent(`${CONTEXT}

Reports: probe ${JSON.stringify(probe)} mutations ${JSON.stringify(muts)}
YOUR TASK (final runner, effort high): editor closed; runner -Tier full -Data -UpdateBaseline once; save the whole output to ${W}/final_full.txt; then fix_seed42.py (shape); its five lines must equal the probe's build-pass lines and bundle K's on-side lines (compare and write the result); if anything moved, fill MOVED_RUNS/NEW_HASHES and edit last_recorded_in_changelog by hand as brief section 3 says, else confirm 'bit-identical' (only verified_at changes). Record the test count and pass results. If any test fails, stop and report (status 'stop') - do not fix here.
Return status, summary, files_changed, numbers (five lines, hashes, tests passed per pass), open_points.`, { label: 'final', phase: 'Final', schema: OUT, effort: 'high' })
if (!final || final.status === 'stop') {
  return { start, impl, probe, l1, l2, l3, kept: kept.length, fix, muts, final }
}

phase('Records')
const records = await agent(`${CONTEXT}

Reports: start ${JSON.stringify(start)} probe ${JSON.stringify(probe)} fix ${JSON.stringify(fix)} mutations ${JSON.stringify(muts)} final ${JSON.stringify(final)}; review: ${all.length} findings, kept ${kept.length}.
YOUR TASK (records, effort high): brief section 6 exactly - N section 5 [B-6b8b] block (with the order change, the review counts, the K1-K3 values, W_TABLE, the line corrections), the H section 11 line (moved or bit-identical variant; measure H with wc -c and use the short form plus N if H + line >= 149,000), the DECISIONS_LOG line (pick the side of '|' that applies), STATUS.md :27 and :32 (re-derive; :32's next steps: AI-stage time cause search (bundle B's time gate G1 - serial AI stage above 16.05 us; the lead re-measured on a quieter machine: data on 17.3-23.0, off 11.5-12.3) -> [B-6b10] clock (C = 9,000; [B-6b9] not opened) -> [B-6b11] -> [B-6b12]), and the commit message file ${W}/commit_b6b8b.txt in the brief's order (title, NE_TEXT verbatim, line numbers now, W_TABLE, five lines, bundle K, mutations, trailer). Then run the five Python checks (check_data, test_data_schema, check_layers --strict, make_config_reference --check, check_docs), check no CR/BOM in changed files, H bytes, and list the real changed files (git -c core.quotepath=false diff --numstat, ignoring 0 0 lines, plus untracked except docs/OVERVIEW.html).
Return status, summary (everything the lead needs to commit), files_changed, numbers, open_points.`, { label: 'records', phase: 'Records', schema: OUT, effort: 'high' })

return { start, impl, probe, l1, l2, l3, kept: kept.map((x) => ({ lens: x.lens, id: x.finding.id, claim: x.finding.claim })), fix, muts, final, records }
