export const meta = {
  name: 'b6b10-clock',
  description: '[B-6b10] the clock: set C from the pre-written rule (bundle B gave C = 9,000, branch M), embedded tests, helper move, save case (5), docs; start checks with an independent recount, I1/I2, build and targeted run, four lenses with refuters, fix, mutations, one full runner with -Data -UpdateBaseline, records; no commit',
  phases: [
    { title: 'Start', detail: 'brief section 0 on the new parent, branch decision, independent recount of C' },
    { title: 'Implement', detail: 'I1 code/tests/patches, I2 docs/records drafts' },
    { title: 'Probe', detail: 'assets, build, targeted run; lenses L1-L3' },
    { title: 'Verify', detail: 'one refuter per finding' },
    { title: 'Fix', detail: 'apply findings' },
    { title: 'Mutations', detail: 'serially; clean rebuild' },
    { title: 'Final', detail: 'one runner full -Data -UpdateBaseline; checks' },
    { title: 'Records', detail: 'fill placeholders, H bytes moved to N, commit message' },
  ],
}

const SCR = 'C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad'
const ROOT = 'C:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension'
const W = SCR + '/b6b10'
const BRIEF = W + '/brief_final.md'
const WORKING = W + '/brief_working.md'

const CONTEXT = `
Project: Cosmic Ascension (UE 5.8.3 C++ module Cosmic_Ascension), repository ROOT = ${ROOT} (main; HEAD 5ed07e1 = [B-6br], pushed; before it 8b6c5a7 [B-6b8b], 84f0d36 [B-6b8]). You implement B6b's step [B-6b10] 'the clock' exactly as BRIEF = ${BRIEF} says. The brief was written at 10205b6; its {P} (the parent) is now 5ed07e1 ([B-6br] - the time-gate fix, bit-identical; [B-6b9] was NOT opened because every bundle-B judgment passed). Re-derive every anchor on {P}. ${WORKING}, once written, wins over the brief.
Known facts from [B-6b8] (N section 3 '묶음 B' and N section 5 [B-6b8]): judgments 1-4 passed; judgment 4 gave C0 = 9,000 and C = 9,000 (the 48th E = 8,439; (i) 56 >= 30; (ii) 55/55); slow-R judgment 2 = 31/60 = 0.517 [0.393, 0.638], wholly above 35% - already shown to the developer (2026-11-10) with C's acceptance; not a gate (brief risk 10). Branch M is expected - confirm by the brief's section 0.3 gates.
Header H = docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md is 148,981 bytes (19 bytes under the self-imposed 149,000): this commit's H section 11 line must move the same bytes from H to N in the same commit (brief 6.4 and risk 9); insert it after the [B-6br] line.
RULES
- Runner: from PowerShell, editor closed: Remove-Item Env:PROJECT_ROOT -ErrorAction SilentlyContinue; powershell -ExecutionPolicy Bypass -File Tools\\run_checks.ps1 ... Implementers never run it; ONE runner -Tier full -Data -UpdateBaseline at the end (its two passes are the two consecutive test runs). After -UpdateBaseline restore seed42.json's committed shape with python -I -X utf8 ${SCR}/b6b8/fix_seed42.py (no flags) and then edit last_recorded_in_changelog / the note as the brief says.
- Mutations before the final runner (the order the brief already gives); harness ${SCR}/b6b/run_mutations2.py (it now rebuilds clean before a data patch after a C++ mutant); snapshot first with ${SCR}/b6b/snap_all.py, restore with restore_tree.py if anything stops; survivors -> fix the TEST.
- LF, Source ASCII, plain Korean in docs, CLAUDE.md rule 11. Never touch docs/OVERVIEW.html or scratchpad/b7x/** (block B7x integrates in its own worktree through the junction C:/Users/rhwwl/AppData/Local/Temp/b7xi - its builds share the machine; Build.bat's lock serializes builds).
- Stop rules of the brief (branch X, risks 4/5/6/8) -> status 'stop' with numbers; nothing committed. Do not commit or push. Trailer: Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
`

const OUTS = { type: 'object', properties: { status: { type: 'string', enum: ['ok', 'stop', 'partial'] }, summary: { type: 'string' }, files_changed: { type: 'array', items: { type: 'string' } }, numbers: { type: 'array', items: { type: 'string' } }, open_points: { type: 'array', items: { type: 'string' } } }, required: ['status', 'summary', 'files_changed', 'numbers', 'open_points'] }
const FINDINGS = { type: 'object', properties: { findings: { type: 'array', items: { type: 'object', properties: { id: { type: 'string' }, severity: { type: 'string', enum: ['high', 'medium', 'low'] }, file: { type: 'string' }, location: { type: 'string' }, claim: { type: 'string' }, evidence: { type: 'string' }, fix: { type: 'string' } }, required: ['id', 'severity', 'file', 'location', 'claim', 'evidence', 'fix'] } } }, required: ['findings'] }
const VERDICT = { type: 'object', properties: { verdict: { type: 'string', enum: ['confirmed', 'partial', 'refuted'] }, reason: { type: 'string' }, corrected_fix: { type: 'string' } }, required: ['verdict', 'reason', 'corrected_fix'] }

phase('Start')
const [start, recount] = await parallel([
  () => agent(`${CONTEXT}

YOUR TASK (start, effort high): brief section 0.1 on {P} = 5ed07e1 (history, anchors re-derived, status_before.txt, the parent CSV and log copy of step 4), 0.2 (fill every placeholder from N section 3 '묶음 B' - never invent), 0.3 (decide the branch by all five gates); write ${WORKING}. Branch X -> status 'stop' with the numbers.
Return status, summary, files_changed, numbers (every placeholder value, the branch), open_points.`, { label: 'start', phase: 'Start', schema: OUTS, effort: 'high' }),
  () => agent(`${CONTEXT}

YOUR TASK (L4 independent recount of C, effort high, read-only): from the archived bundle B (Saved/Balance/b6b_bundleB logs and runs.json, and scratchpad/b6b8/bundleB/ CSVs if present) recompute E_1..E_60 for the slowR cell, C0 (the 48th E ascending rounded up to a multiple of 1,000), C = min(18,000, max(6,000, C0)), (i) and (ii) exactly as H section 5.3 / the [B-6b8] brief judge_4 define them, WITHOUT importing bundleB.py's judging code (write your own small script in ${W}/recount/). Compare with N section 3. Report any difference.
Return status ('ok' if equal), summary, files_changed, numbers, open_points.`, { label: 'L4:recount', phase: 'Start', schema: OUTS, effort: 'high' }),
])
if (!start || start.status === 'stop' || (recount && recount.status === 'stop')) return { start, recount }

phase('Implement')
const impl = await parallel([
  () => agent(`${CONTEXT}

Start: ${JSON.stringify(start)}
YOUR TASK (I1, effort high): brief 1.1-1.5 for the decided branch (data, the comment edits, the helper move, the embedded tests, Sim.SaveOldEndFlows case (5)), the patches of section 5.1 from clean copies and their README rows. Do not build or run the runner.
Return status, summary, files_changed, numbers, open_points.`, { label: 'I1', phase: 'Implement', schema: OUTS, effort: 'high' }),
  () => agent(`${CONTEXT}

Start: ${JSON.stringify(start)}
YOUR TASK (I2, effort high): drafts of brief 6.1-6.5 with the hash placeholders still in (docs sentences, DECISIONS_LOG, OPEN_QUESTIONS #2 rewritten in place, H section 11 line with the byte move to N, N section 5 block, STATUS). Do not run make_config_reference yet (the probe does). Do not touch source.
Return status, summary, files_changed, numbers, open_points.`, { label: 'I2', phase: 'Implement', schema: OUTS, effort: 'high' }),
])

phase('Probe')
const LENS = (txt) => `${CONTEXT}

You are a reviewer (read-only). Start: ${JSON.stringify(start)} Implementers: ${JSON.stringify(impl)}
${txt}
Report real defects with evidence and exact fixes (severity high/medium/low).`
const [probe, l1, l2, l3] = await parallel([
  () => agent(`${CONTEXT}

Implementers: ${JSON.stringify(impl)}
YOUR TASK (probe, effort high): brief 7.1 steps 4-5: editor closed; assets (make_data_tables via the runner's -Data step or directly), then python -I -X utf8 Tools/make_config_reference.py and --check; full build of both targets; the quick targeted run of the nine tests the brief lists (one pass, not the gate). A break -> risk 6 (name the cause; never change C or a seed). Write ${W}/probe/probe_out.md.
Return status, summary, files_changed, numbers, open_points.`, { label: 'probe', phase: 'Probe', schema: OUTS, effort: 'high' }),
  () => agent(LENS('L1 determinism and baselines: brief section 3; no knob reader at or below 3,000 ticks; no new draw or field; the commandlet readers; which of the five lines must move (the 20,000 lines and the wanderer if their parent record was Clock@6000) and which must stay (the 3,000 lines).'), { label: 'L1', phase: 'Probe', schema: FINDINGS, effort: 'high' }),
  () => agent(LENS('L2 tests and mutations: case (5) catches both patches; the Rules-reference ordering; the pin; the rerun list recomputed on {P} with the exemption and the dead patches (narrow rule).'), { label: 'L2', phase: 'Probe', schema: FINDINGS, effort: 'high' }),
  () => agent(LENS('L3 records and words: the three verbatim quotes (brief 6.2 check); the docs sentence list against N section 1.2; theology guardrails and plain words on the new Korean; the H/N byte budget (H must stay under 149,000 after the line, by moving bytes to N).'), { label: 'L3', phase: 'Probe', schema: FINDINGS, effort: 'high' }),
])
if (probe && probe.status === 'stop') return { start, recount, impl, probe }

phase('Verify')
const verified = await parallel([['L1', l1], ['L2', l2], ['L3', l3]].flatMap(([k, r]) => ((r && r.findings) || []).map((f) => () =>
  agent(`${CONTEXT}

Skeptical verifier: try to REFUTE this finding (read the files). refuted / partial (corrected_fix) / confirmed.
Finding (${k}): ${JSON.stringify(f)}`, { label: `verify:${k}:${f.id}`, phase: 'Verify', schema: VERDICT, effort: 'high' })
    .then((v) => ({ lens: k, finding: f, verdict: v })))))
const kept = verified.filter(Boolean).filter((x) => x.verdict && x.verdict.verdict !== 'refuted')

phase('Fix')
let fix = null
if (kept.length) {
  fix = await agent(`${CONTEXT}

YOUR TASK (fix, effort high): apply the kept findings (corrected_fix when partial); if source or data changed, repeat the probe's build and targeted run. Write ${W}/fix/handoff.md.
KEPT: ${JSON.stringify(kept.map((x) => ({ lens: x.lens, id: x.finding.id, severity: x.finding.severity, file: x.finding.file, location: x.finding.location, claim: x.finding.claim, fix: x.finding.fix, verdict: x.verdict.verdict, corrected_fix: x.verdict.corrected_fix })), null, 1)}
Return status, summary, files_changed, numbers, open_points.`, { label: 'fix', phase: 'Fix', schema: OUTS, effort: 'high' })
}

phase('Mutations')
const muts = await agent(`${CONTEXT}

Reports: probe ${JSON.stringify(probe)} fix ${JSON.stringify(fix)}
YOUR TASK (mutations, effort high): brief section 5 and 7.1 step 8 - new patches and the narrow rerun set (recomputed on the tree), serially from ROOT with the editor closed; survivors -> fix the test and rerun; apply_after.txt vs apply_before; README rows; then git status --porcelain must equal status_before.txt plus this commit's files (risk 12); clean rebuild of both targets and the assets.
Return status, summary (killed/survived), files_changed, numbers, open_points.`, { label: 'mutations', phase: 'Mutations', schema: OUTS, effort: 'high' })

phase('Final')
const final = await agent(`${CONTEXT}

Reports: mutations ${JSON.stringify(muts)}
YOUR TASK (final, effort high): brief 7.1 step 9: the ONE runner -Tier full -Data -UpdateBaseline (branch M) -> ${W}/final_full.txt; fix_seed42.py (shape); the seed42.json diff check (the 3,000 lines must not move - risk 4; the moved lines must be exactly those whose parent record was Clock@6000 - risk 5), the 'clock at' check, csv_cmp.py (${W}/csv_cmp.py --clock, as the brief says) against the parent copies, the error-line check of the five logs; record the new hash pairs and tests per pass. Any stop condition -> status 'stop'.
Return status, summary, files_changed, numbers, open_points.`, { label: 'final', phase: 'Final', schema: OUTS, effort: 'high' })
if (!final || final.status === 'stop') return { start, recount, impl, probe, kept: kept.length, fix, muts, final }

phase('Records')
const records = await agent(`${CONTEXT}

Reports: start ${JSON.stringify(start)} recount ${JSON.stringify(recount)} final ${JSON.stringify(final)} mutations ${JSON.stringify(muts)}; review ${verified.length} findings, kept ${kept.length}.
YOUR TASK (records, effort high): brief 7.1 steps 10-11 - fill every hash/count placeholder (seed42.json note and last_recorded_in_changelog, N section 5, DECISIONS_LOG, STATUS '기준값' and next steps -> [B-6b11] bundle C at C = 9,000 -> [B-6b12], H section 11 line after [B-6br] with the byte move to N so H stays under 149,000), re-run check_docs and make_config_reference --check, the placeholder grep (0), the five Python checks, CR/BOM check, H bytes, and write the commit message file ${W}/commit_b6b10.txt per brief 6.6. List the real changed files (numstat without 0 0, plus untracked except docs/OVERVIEW.html).
Return status, summary, files_changed, numbers, open_points.`, { label: 'records', phase: 'Records', schema: OUTS, effort: 'high' })

return { start, recount, impl, probe, kept: kept.map((x) => ({ lens: x.lens, id: x.finding.id, claim: x.finding.claim })), fix, muts, final, records }
