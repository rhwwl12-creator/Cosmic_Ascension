export const meta = {
  name: 'b7x-first-compile',
  description: 'First compile and test of the B7x integration branch (never compiled before) in its own worktree: LFS checkout, editor build with compile fixes, runner full -Data with triage of failures, a read-only check of the fixes; nothing generated is committed',
  phases: [
    { title: 'Build', detail: 'LFS checkout, editor and game targets, compile fixes' },
    { title: 'Test', detail: 'runner full -Data -UpdateBaseline in the worktree, triage and fix, revert generated files' },
    { title: 'Check', detail: 'read-only review of the fixes against the header and notes' },
  ],
}

const SCR = 'C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad'
const INT = SCR + '/b7x/int'
const OUT = SCR + '/b7x/compile1'
const LF = SCR + '/b7x/tools/lf_norm.py'
const MAIN = 'C:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension'
const UE = 'C:/Program Files/Epic Games/UE_5.8'

const CONTEXT = `
Project: Cosmic Ascension, UE 5.8.3 C++ game module Cosmic_Ascension (Windows; PowerShell 5.1 and Git Bash available).
The B7x block is being integrated IN PARALLEL with block B6b. B6b runs in the MAIN checkout (${MAIN}) - never edit, build, stage or run git write commands there, and never run its runner. B7x lives in the integration worktree INT = ${INT} (local branch integrate/b7x, HEAD 1333de4 on main's 220129d; five commits, NEVER COMPILED). Scratch folder OUT = ${OUT} (create it).
The B7x header and notes are in INT: docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md and ..._notes.md (section 8 = commit order and what each commit must prove, section 5.4 of the notes = the review hand-offs that were applied).

RULES
- Work only in INT. Commit only on integrate/b7x (never push, never switch branches). Every git command that writes files in INT gets -c core.eol=lf, then run python -I -X utf8 ${LF} "${INT}" (tree first, --check second when checking).
- The environment variable PROJECT_ROOT makes the runner and tools use another tree: in every PowerShell command run 'Remove-Item Env:PROJECT_ROOT -ErrorAction SilentlyContinue' first, and always call INT's own Tools\\run_checks.ps1 from INT's root.
- Builds: Build.bat holds a lock file, so a build of MAIN by the other workflow simply waits for yours (and the reverse) - that is fine. Do not start the editor GUI. Do not use Live Coding.
- Source stays ASCII (English comments). Includes from the module root. Warnings are errors. Keep the intent of the header and notes exactly; fix what does not compile with the smallest faithful change, and never change behaviour for peoples without bricks (the old path must stay bit-identical - header section 8).
- Nothing generated is committed in this check: Tools/baselines/seed42.json, Tools/data/generated.sha1 and Content/**/*.uasset changes are reverted at the end (git -C INT -c core.eol=lf checkout -- <path>). Only source/test/doc fixes are committed.
`

const STEP = {
  type: 'object',
  properties: {
    status: { type: 'string', enum: ['ok', 'partial', 'blocked'] },
    commits: { type: 'array', items: { type: 'string' } },
    errors_fixed: { type: 'array', items: { type: 'string' } },
    remaining: { type: 'array', items: { type: 'string' } },
    results: { type: 'array', items: { type: 'string' } },
    notes_for_lead: { type: 'array', items: { type: 'string' } },
  },
  required: ['status', 'commits', 'errors_fixed', 'remaining', 'results', 'notes_for_lead'],
}

phase('Build')
const build = await agent(`${CONTEXT}

YOUR TASK (build, effort xhigh):
1. git -C "${INT}" lfs checkout  (the worktree was made with LFS smudge skipped - Content assets are pointer files; the LFS objects are in the shared repository cache). Check one asset is real now (e.g. Content/CA/Data/DT_Rules.uasset is not a 'version https://git-lfs' text file).
2. Build the editor target of INT (PowerShell): Remove-Item Env:PROJECT_ROOT -ErrorAction SilentlyContinue; & '${UE}/Engine/Build/BatchFiles/Build.bat' Cosmic_AscensionEditor Win64 Development "-Project=${INT}/Cosmic_Ascension.uproject" -WaitMutex  - save the full log to OUT/build_editor_<n>.txt. A first build of a fresh worktree compiles the whole module (several minutes).
3. Fix every compile error and warning-as-error with the smallest faithful change (read the real declarations in INT and in ${UE}/Engine/Source; the notes 5.4 and header say what each piece must do). Rebuild until it succeeds (up to eight rounds; if still failing, stop and report precisely).
4. Then build the game target the same way (Cosmic_Ascension Win64 Development) - the runner builds both; fix what fails.
5. Commit the fixes (if any) as one commit '[B-7x1+2] 첫 컴파일 고침(통합 브랜치)' with '- ' bullets in plain Korean naming each error and its fix, last line exactly: Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>. LF check before committing.
Return status, commits, errors_fixed (file:line - error - fix), remaining, results (build times and outcome), notes_for_lead.`, { label: 'build', phase: 'Build', schema: STEP, effort: 'xhigh' })
log(`build: ${build ? build.status : 'FAILED'}`)

let test = null
if (build && build.status === 'ok') {
  phase('Test')
  test = await agent(`${CONTEXT}

The build step reported: ${JSON.stringify(build)}

YOUR TASK (test and triage, effort xhigh):
1. From INT's root in PowerShell: Remove-Item Env:PROJECT_ROOT -ErrorAction SilentlyContinue; powershell -ExecutionPolicy Bypass -File "${INT}/Tools/run_checks.ps1" -Tier full -Data -UpdateBaseline  - save the whole output to OUT/runner_1.txt. -Data rebuilds the assets and generated.sha1 (needed: Tools/data changed); -UpdateBaseline because the new saved fields move both hashes (expected) - we compare, we do not keep the file.
2. Before reverting, compare the five summary lines of the old Tools/baselines/seed42.json (git -C INT show HEAD:Tools/baselines/seed42.json) with the new one: report each line with the hash and world numbers masked - with B7x data off (all knobs inert, BrickSeedTags empty) the lines other than the two hashes should be letter for letter the same (header section 8: CSV identical apart from TickMicros/SaveBytes). Any other difference is a finding (the old path changed).
3. Triage every failing automation test and Python check: (a) a bug in a B7x test (wrong expectation, wrong board) - fix the test faithfully to the notes 5.4 hand-off; (b) a bug in B7x code - fix it; (c) an old test broken by B7x - decide by the header/notes (boards that read game rules must use WithInertBricks/StripBricks; otherwise the old behaviour must not change - then it is a code bug). For each fix, re-run just that test: & '${UE}/Engine/Binaries/Win64/UnrealEditor-Cmd.exe' "${INT}/Cosmic_Ascension.uproject" -ExecCmds="Automation RunTests Cosmic_Ascension.<Name>;Quit" -unattended -nullrhi -nosplash -log=<file> (rebuild the editor target first when source changed). Up to three full rounds; then run the runner once more without -UpdateBaseline? No - run it again WITH -UpdateBaseline -Data to confirm all green, and repeat step 2's comparison.
4. Revert generated files: git -C "${INT}" -c core.eol=lf checkout -- Tools/baselines/seed42.json Tools/data/generated.sha1 and any changed Content/**/*.uasset; LF normalizer; confirm with git status that only your source/test/doc fixes remain, then commit them as '[B-7x1+2] 첫 테스트 고침(통합 브랜치)' with bullets per failure (test name - cause - fix) and the trailer line exactly: Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>.
Return status, commits, errors_fixed (one entry per failing test and its fix), remaining (anything still failing, with the reason), results (runner summary: tests passed/failed per pass, Python checks, the masked summary-line comparison), notes_for_lead (e.g. the new hash values seen, for later).`, { label: 'test', phase: 'Test', schema: STEP, effort: 'xhigh' })
  log(`test: ${test ? test.status : 'FAILED'}`)
}

phase('Check')
const check = await agent(`${CONTEXT}

YOUR TASK (read-only reviewer, effort high - do not edit, do not build): read every change committed by this check on integrate/b7x after 1333de4 (git -C "${INT}" log --oneline 1333de4..HEAD; git -C "${INT}" diff 1333de4..HEAD). Reports: BUILD ${JSON.stringify(build)} TEST ${JSON.stringify(test)}.
For each fix: does it keep the header/notes intent (notes 5.4 hand-offs, header sections 2 and 6), keep peoples without bricks on the old path, avoid weakening a test to make it pass (an expectation changed only because the code was wrong, or the expectation was wrong by the notes' own numbers), and keep ASCII/LF/includes rules? Report concerns with file:line and the reason; say clearly if a test was weakened.
Return status ok/partial, commits [], errors_fixed [], remaining (concerns), results (what you checked), notes_for_lead.`, { label: 'check', phase: 'Check', schema: STEP, effort: 'high' })

return { build, test, check }
