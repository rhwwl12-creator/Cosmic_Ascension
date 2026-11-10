export const meta = {
  name: 'b6b11-harness-review-freeze',
  description: 'B-6b11 bundle C: check the harness on real text, three verified review lenses, fix, then write the frozen 4.3 block',
  phases: [
    { title: 'Harness', detail: 'self-test, --check-only over bundle B logs, dry run, parser vs commandlet source' },
    { title: 'Review', detail: 'three lenses, each finding rebut-verified' },
    { title: 'Fix', detail: 'apply surviving findings, rerun the checks' },
    { title: 'Freeze', detail: 'frozen 4.3 block into N section 5 and frozen_4_3.md' },
  ],
}

const SP = 'C:\\Users\\rhwwl\\AppData\\Local\\Temp\\claude\\c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension\\37de2919-0c44-45d8-94cd-4dcf0860263e\\scratchpad'
const ROOT = 'C:\\Users\\rhwwl\\OneDrive\\Documents\\Unreal Projects\\Cosmic_Ascension'

const CTX = `
You are working on step [B-6b11] (bundle C re-measure) of the Cosmic Ascension project (UE 5.8 C++; repo root ${ROOT}; main HEAD 4448705).
The step's brief is ${SP}\\b6b11\\brief_final.md - read sections 0-7 fully first. Its line cites were read at 10205b6 and have drifted; find each cited spot by its text.
Abbreviations: H = docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md, N = its _notes.md, DL = docs/60_DECISIONS/DECISIONS_LOG.md, cmd = Source/Cosmic_Ascension/Tests/CosmicBalanceCommandlet.cpp, SP = ${SP}.
Harness (outside the repo): ${SP}\\b6b11\\bundleC.py (usage in its top docstring), self-test ${SP}\\b6b11\\bundleC_selftest.py (last run 2026-11-10 02:01: 197/197 - written BEFORE [B-6b8], [B-6b8b], [B-6br] and [B-6b10] were committed, so recheck against today's code and logs). It loads ${SP}\\b6b8\\bundleB.py (changed by [B-6b8]: count fixes M2-M5) and ${SP}\\b6b6\\bundleA.py.

Start values, already determined by the lead (use them, do not re-derive differently):
- <BUILD_SHA> = 4448705 (last commit touching Source/, Tools/data or Content/CA/Data - [B-6b10] changed DT_Rules and its asset).
- <B9_RAN> = no. (git log --grep '\\[B-6b9\\]' finds 3 commits, but only because the bodies of [B-6b6], [B-6b8] and [B-6b10] MENTION [B-6b9]; no commit is titled [B-6b9].)
- <BR_WORLD> = empty ([B-6br] 5ed07e1 was bit-identical: N section 5 [B-6br] block).
- <K_MOVED>: {K_MOVED_R} = none, {K_MOVED_W} = none (N section 5 [B-6b8b] block, bundle K: K3 moved pairs R 0/60, wanderer 0/60; Saved\\Balance\\b6b8b_bundleK\\).
- At 4448705: <C> = 9000 (DT_Rules ProvisionalEndTick), <R_COMMITTED> = 0.375, <SHARE_COMMITTED> = 0.5, <W_EASED> = 1.2 (event_claim_eased BaseWeight), <CHANCE> = 0.02 with EventSlotTicks 10, <RATIO> = 0.79.
- <T_DEATH> = 5000 (N section 3 '묶음 B'; every death log in Saved\\Balance\\b6b_bundleB\\ should say 'death asked at 5000'). T_DEATH < C, so the death cell is KEPT: 620 jobs.
- Bundle B took 35.9 minutes with 4 workers (not 6) for 541 attempts, so <A_SEC_PER_20K> = 35.9*60*4/541 = about 15.9 s.
- <BASE_B6b10>: Tools/baselines/seed42.json at 4448705 (3,000 2557850423/2906170550; ruler 3,000 1189870756/3359656751; wanderer 6,000 2158489212/1214038630; 20,000 4192287602/2387995655; ruler 20,000 1375149537/510604211; the two game hash runs equal the 3,000 lines).
- <H_BYTES> = 148,990 at the start.
- Bundle B CSVs: ${SP}\\b6b8\\bundleB\\ (1,620 csv files); logs + sha1 manifest: ${ROOT}\\Saved\\Balance\\b6b_bundleB\\. B6a bundle 1 (decision 36 bridge): ${ROOT}\\Saved\\Balance\\b6a_bundle1\\.

Machine rules right now:
- The lead is running the full runner (Tools\\run_checks.ps1 -Tier full) in the background: it builds and runs UnrealEditor processes for about 40-60 minutes. Do NOT build, do NOT run UnrealEditor/UnrealEditor-Cmd, do NOT launch bundle C jobs, do NOT run make_data_tables or mutations. The harness self-test uses a mocked commandlet; if a self-test case fails ONLY because the real busy-process check sees the runner's UnrealEditor/UBT processes, say so, wait (poll Get-Process UnrealEditor*,UnrealBuildTool every 2 minutes) and rerun it once they are gone.
- Never call plain 'git status' in the repo (it rewrites .git/index); use 'git --no-optional-locks ...'.
- Do not edit anything in the repo except where your task says so. Python: run with 'python -I -X utf8'; set PROJECT_ROOT only if a tool needs it, or pass --root explicitly. Write Python files for any text containing backslashes (heredocs eat them).
- The developer said on 2026-11-10: any decision that needs the developer is taken with the brief's recommended option. Record such items, do not stop on them.
`

const HARNESS_SCHEMA = {
  type: 'object',
  properties: {
    selftest: { type: 'string', description: 'command and result line (checks passed/failed)' },
    check_b: { type: 'string', description: 'the --check-only --cell-set B run over the real bundle B logs: command, where it wrote, result, any parser or control failure' },
    dry_run: { type: 'string', description: 'job count, two jobs per cell quoted, ProvisionalEndTick absent, at most one -Rules per job' },
    parser_vs_source: { type: 'array', items: { type: 'string' }, description: 'each log line format the harness parses for C-form runs, checked against cmd at 4448705: ok or mismatch' },
    changes: { type: 'array', items: { type: 'string' }, description: 'every edit made to bundleC.py or bundleC_selftest.py, with the reason' },
    blockers: { type: 'array', items: { type: 'string' } },
  },
  required: ['selftest', 'check_b', 'dry_run', 'parser_vs_source', 'changes', 'blockers'],
}

const FINDINGS_SCHEMA = {
  type: 'object',
  properties: {
    findings: {
      type: 'array',
      items: {
        type: 'object',
        properties: {
          id: { type: 'string' },
          where: { type: 'string', description: 'file:line or function in the harness (or brief section)' },
          defect: { type: 'string' },
          evidence: { type: 'string', description: 'the text/code that proves it, with cites' },
          rebuttal_tried: { type: 'string', description: 'how you tried to refute it and why it survived' },
          verdict: { type: 'string', enum: ['confirmed', 'partial'] },
          fix: { type: 'string', description: 'exact change to make, and the self-test case that must break without it' },
          severity: { type: 'string', enum: ['wrong-reading', 'wrong-control', 'robustness', 'record-only'] },
        },
        required: ['id', 'where', 'defect', 'evidence', 'rebuttal_tried', 'verdict', 'fix', 'severity'],
      },
    },
    rebutted: { type: 'array', items: { type: 'string' }, description: 'candidate findings you refuted, one line each' },
  },
  required: ['findings', 'rebutted'],
}

phase('Harness')
const harness = await agent(`${CTX}
Your task (brief section 7.1 steps 2-3, harness side only):
1. Run the self-test. Report the result line.
2. Run 'bundleC.py --check-only <folder> --cell-set B' over the REAL bundle B logs to exercise the parsers on real text (brief section 2, last bullet). First read the code to see where --check-only writes its outputs (summary, readings). It must NOT overwrite anything in ${ROOT}\\Saved\\Balance\\b6b_bundleB\\ or ${SP}\\b6b8\\bundleB\\: if it would, copy the logs (and the csv files it needs) to ${SP}\\b6b11\\checkB_input\\ and run there, or point its outputs at ${SP}\\b6b11\\checkB\\. Report the result and any parser/control failure on real text.
3. Run --dry-run with all start values passed explicitly (--root, --c 9000 --t-death 5000 --build-sha 4448705 --r 0.375 --share 0.5 --chance 0.02 --w-eased 1.2 --ratio 0.79 --b9-ran no --br-world '' (or whatever the code expects for empty) --k-moved-r/--k-moved-w as empty, --workers 6, --folder ${SP}\\b6b11\\bundleC, --archive-dir ${ROOT}\\Saved\\Balance\\b6b_bundleC, --bundle-b-logs, --bundle-b-csv, --bundle-k-dir, --b6a-dir, --a-sec-per-20k 15.9). Confirm 620 jobs (300 at 9,000 ticks: R, slowR, death, wanderer on+off... recount per the brief: R 60, slowR 60, death 60, wanderer 60 on + 60 off at C; inputless 60 on + 60 off, inputless075 60, test 140 at 20,000), quote two jobs per cell, confirm no argument contains ProvisionalEndTick and each job has at most one -Rules=. Save the exact dry-run command line to ${SP}\\b6b11\\run_cmd.txt (the lead will reuse it without --dry-run).
4. Check every log line format the harness parses for C-form runs against cmd at 4448705 (git show 4448705:Source/Cosmic_Ascension/Tests/CosmicBalanceCommandlet.cpp): the ending head 'record <kind> at <t>' and the kind names (Clock, RuledPeople, OtherPeople, NoCandidate, Vessel, None), the race part, the [B-6b8] side-part layout (era lines / witness cards), the off-log 'rows taken out [...]' list (the ten names) and tail 'AIUpperRungProgressMultiplier 1 (was 0.375), FleetDelayCountdownShare 0 (was 0.5)', 'CosmicBalance: rules <key> = <value>', 'death asked at', 'calling deaths:' successions 'tick:...:D<n>', the wanderer line 'crowned at'/'left at', '#135 claim trigger ... keys [..]', '#132 guarded', 'delay bought per tag', the pressure line band '401-550', '-SaveAt <t> wrote <path>', the _claims.csv header columns (Ruled, CardTick, RemainingAtCard, VigilKept, Relief, OwnerChanged, EndKind, PlacedTick, AfterBinding), and the CSV header (B-c bridge). For each: ok or mismatch.
5. Fix ONLY clear parser bugs or stale expectations caused by [B-6b8]/[B-6b8b]/[B-6b10] (for example share 0 vs 0.5 in F3, or C-form clock records), each with a self-test case that breaks without the fix, then rerun steps 1-3. Do not change the frozen arithmetic of brief section 4.3 or any control's meaning. Keep every planted failure of the self-test.
Return the structured result.`, { label: 'harness-check', phase: 'Harness', schema: HARNESS_SCHEMA, effort: 'high' })

const LENSES = [
  { key: 'controls', text: 'Lens 1 - measurement and controls: F1-F8 and the bridges B-a to B-e as implemented in bundleC.py, against the brief section 1.2, the H and N text they cite (H race check / end check definitions, N ⓗ same-world rules, decision 36 text) and the ruler policy (Source/Cosmic_Ascension/World/CosmicRulerPolicy.cpp guard steps) and the wanderer policy. Check also the sha1 guard file set, the per-job busy check, the memory guard, rerun-once and the withheld-cell rule.' },
  { key: 'statistics', text: 'Lens 2 - frozen statistics: R1-R11 as implemented (the functions that compute each reading, Wilson intervals, the sign test, the #142 acceptance region with p_e = 1-(1-0.02*1.2)^47.5 = 0.6846, the #10 per-death rule, the #158③ dedup with tick > T_DEATH / tick > 1,200, the B1 own-vessel reading with crowned-before-first-death and median after leaving, the record-kind shares) against the brief section 4.3 and the texts it cites (H:325 shared rules, N ⓕ, N ⓗ, DL decision 24 / B4 model interval, B5 notes on the #10 and #154 cells, B6a notes on B1: 40/60, 1,350, paths 22/15/3).' },
  { key: 'handcheck', text: 'Lens 3 - hand checks and form: the hand-check seed selection of brief section 4.4 as implemented (smallest qualifying seed, which cells may give seeds given bSaveShape in cmd, the save tick rules ① T4-50, ② record-30, ③ first placement-10 preferring ruled expiries and died, ⑥ (i) era first-30 and (ii) witness first-30 with the wanderer fallback), the printed commands (game form, -SaveAt accepted arguments - check bSaveShape in cmd at 4448705), how the save file is taken from the log line "-SaveAt <t> wrote <path>" and moved to Saved\\HandCheck\\B6b_<n>_<what>_seed<s>_<t>\\ without touching other saves, and the archive step (every .log plus a sha1 manifest of every output to Saved\\Balance\\b6b_bundleC\\).' },
]

phase('Review')
const reviews = await parallel(LENSES.map(l => () => agent(`${CTX}
Harness check result from the previous agent (it may have edited the harness): ${JSON.stringify(harness)}

You are one of three independent reviewers of the bundle C harness before it runs. ${l.text}
Find defects that would make a reading, control or bridge wrong, or the run unsafe. For EACH candidate finding, try hard to refute it by reading the actual code and the cited text (and, where cheap, by a tiny python experiment against the self-test fixtures - never launch UnrealEditor). Return only findings that survive as confirmed or partial, each with an exact fix and the self-test case that must break without it. List refuted candidates in 'rebutted'. Do not edit any file.`, { label: `review:${l.key}`, phase: 'Review', schema: FINDINGS_SCHEMA, effort: 'high' })))

const allFindings = reviews.filter(Boolean).flatMap((r, i) => (r.findings || []).map(f => ({ ...f, lens: LENSES[i] ? LENSES[i].key : 'unknown' })))
const rebuttedCount = reviews.filter(Boolean).reduce((n, r) => n + ((r.rebutted || []).length), 0)
log(`review: ${allFindings.length} findings kept, ${rebuttedCount} refuted`)

phase('Fix')
const fix = await agent(`${CTX}
Harness check result: ${JSON.stringify(harness)}
Review findings that survived rebuttal (three lenses): ${JSON.stringify(allFindings)}

You are the fix worker. For each finding: apply the fix to bundleC.py / bundleC_selftest.py (and nothing else), adding the self-test case that breaks without it; if on a second reading a finding is wrong, do not apply it and say why. Do not change the meaning of the frozen arithmetic (brief 4.3) - if a finding says the brief's own arithmetic is wrong, do not change the harness for it; report it under 'arithmetic_notes' so the lead can record it as a deviation. Then rerun: the self-test, the --check-only --cell-set B run (same safe output place as before), and the --dry-run (same command as ${SP}\\b6b11\\run_cmd.txt). Return a short report: per finding applied/not applied with reason, the three rerun results, arithmetic_notes, and any blocker.`, { label: 'fix', phase: 'Fix', effort: 'high' })

phase('Freeze')
const freeze = await agent(`${CTX}
Fix report: ${typeof fix === 'string' ? fix : JSON.stringify(fix)}

You write the freeze text (brief section 4.3 'Write this section into the N section 5 block before the first run' and section 7.1 step 6) - nothing else.
1. In N (docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race_notes.md, UTF-8, LF only), section 5, insert right after the end of the [B-6b10] block (the block whose head starts with **\`[B-6b10]\`**) a new block head line exactly in the form the other blocks use: **\`[B-6b11]\`**(2026-11-10 — 다시 잼(묶음 C)) followed by one item '- **측정**(묶음 C의 첫 판 전에 고정한 셈 — 지침서 §4.3; 이 항목의 글자 그대로를 \`scratchpad\\b6b11\\frozen_4_3.md\`에 두고 하네스가 첫 판 전에 sha1을 찍는다): ...' that states R1-R11 of the brief section 4.3 in Korean, faithfully: every rule, cut, filter, threshold, branch and outcome word of 4.3, with the start values filled (C 9,000; T_DEATH 5,000; CHANCE 0.02; W_EASED 1.2; p_e = 1 − (1 − 0.02·1.2)^47.5 = 0.6846; n = 60 per cell; test 140 seeds) and with log strings, CSV column names and code identifiers kept verbatim in backticks. Do not add rules, numbers or judgments that 4.3 does not have; keep the outcome strings that 4.3 quotes in Korean exactly as quoted (for example '닫는다', '열어 둔다', '재지 않음 — 잰 N', '모형 구간 안', 'B6b가 줄였다', '잡음', '깨짐 — 중앙값 <m>', '[개발자]'). Write it in the plain style of the neighbouring N section 5 blocks (one long item is fine; sub-items with ' · ' or ';' as they do). Add nothing else to the block yet - the docs step fills the rest later.
2. Save the EXACT bytes of that '- **측정**...' item (from '- **측정**' to the end of its last line, LF, UTF-8, no BOM, ending with one newline) to ${SP}\\b6b11\\frozen_4_3.md, and verify with a python script that the file's text occurs verbatim in N.
3. Check N has no CR bytes and that python -I Tools/check_docs.py still reports 0 (run from the repo root).
Return: the byte size of frozen_4_3.md, its sha1, the N line number where the block starts, and the check_docs result.`, { label: 'freeze', phase: 'Freeze', effort: 'high' })

return { harness, findings: allFindings, rebuttedCount, fix, freeze }
