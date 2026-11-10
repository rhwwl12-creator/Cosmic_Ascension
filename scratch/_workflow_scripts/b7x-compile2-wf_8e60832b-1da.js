export const meta = {
  name: 'b7x-compile2',
  description: 'Second compile and test of the B7x integration branch: stage 3 ([B-7x4m], [B-7x3], [B-7x4] and review fixes, never compiled) built through the short junction, runner full -Data -UpdateBaseline, triage and fix, masked baseline comparison; generated files reverted',
  phases: [
    { title: 'Build', detail: 'editor and game targets through the junction; compile fixes' },
    { title: 'Test', detail: 'runner full -Data -UpdateBaseline; triage; revert generated files' },
    { title: 'Check', detail: 'read-only review of any fixes' },
  ],
}

const SCR = 'C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad'
const INT = SCR + '/b7x/int'
const J = 'C:/Users/rhwwl/AppData/Local/Temp/b7xi'
const OUT = SCR + '/b7x/compile2'
const LF = SCR + '/b7x/tools/lf_norm.py'
const MAIN = 'C:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension'
const UE = 'C:/Program Files/Epic Games/UE_5.8'

const CONTEXT = `
Project: Cosmic Ascension, UE 5.8.3 C++ module Cosmic_Ascension (Windows). Block B6b runs in MAIN (${MAIN}) - never edit, build, stage or git-write there. Block B7x is integrated in the worktree INT = ${INT} (local branch integrate/b7x, HEAD db9bf9f on main's 220129d). Commits up to 1333de4 were compiled and tested already (no fixes needed, 234 tests twice, data-off lines identical with hashes masked); the stage-3 commits on top - 5b6f0a2 [B-7x4m] measurement, 9ce7709 [B-7x3] presentation, 555afed [B-7x4] texts, db9bf9f review fixes - have NEVER been compiled. OUT = ${OUT}.
- Build and run ONLY through the short junction J = ${J} (it points at INT; the long path breaks UBT's 260-character limit). In PowerShell first: Remove-Item Env:PROJECT_ROOT -ErrorAction SilentlyContinue.
- INT/Saved/UnrealBuildTool/BuildConfiguration.xml already sets bAllowHotReloadFromIDE=false (Live Coding guard from MAIN's processes); if a build is still refused, add -NoHotReloadFromIDE.
- Every git command that writes files in INT gets -c core.eol=lf, then python -I -X utf8 ${LF} "${INT}"; finish with --check (0).
- Commit only on integrate/b7x; never push; never switch branches. Source ASCII; includes from the module root; warnings are errors; keep the header/notes intent; old paths bit-identical while B7x data is absent.
- Busy courtesy: before the runner, check that no B6b harness is running (python whose command line contains bundleK, bundleA, bundleB, bundleC, lanes, run_mutations, run_py) - if one is, wait (check every 60 s, up to 60 min). Build.bat's lock file serializes builds by itself.
- Generated files are never committed here: Tools/baselines/seed42.json, Tools/data/generated.sha1, Content/**/*.uasset -> revert at the end with git -C INT -c core.eol=lf checkout -- <paths>.
- Trailer for fix commits: Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
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

YOUR TASK (build, effort xhigh): build the editor target (& '${UE}/Engine/Build/BatchFiles/Build.bat' Cosmic_AscensionEditor Win64 Development "-Project=${J}/Cosmic_Ascension.uproject" -WaitMutex) and then the game target (Cosmic_Ascension Win64 Development, same project path). Save logs to OUT/build_*.txt. Fix every compile error and warning-as-error with the smallest faithful change (read the real declarations; the B7x header sections 4-6 and notes 5.4 in INT say what each piece must do); rebuild until both succeed (up to eight rounds, else stop and report). Also compile the files the stage-3 commits changed one by one (-SingleFile, as the first compile did) to catch include dependencies a unity build hides. Commit fixes (if any) as one commit '[B-7x3·4m·4] 컴파일 고침(통합 브랜치)' with '- ' bullets in plain Korean (each error and fix).
Return status, commits, errors_fixed, remaining, results, notes_for_lead.`, { label: 'build', phase: 'Build', schema: STEP, effort: 'xhigh' })

let test = null
if (build && build.status === 'ok') {
  phase('Test')
  test = await agent(`${CONTEXT}

Build report: ${JSON.stringify(build)}
YOUR TASK (test and triage, effort xhigh):
1. Runner through the junction: Remove-Item Env:PROJECT_ROOT -ErrorAction SilentlyContinue; & '${J}/Tools/run_checks.ps1' -Tier full -Data -UpdateBaseline  -> OUT/runner_1.txt (the stage-3 data files changed: ST_CA Shape keys, DT_LogTemplates scan row, schema).
2. Compare the five summary lines of the new seed42.json with INT's HEAD seed42.json with hash/world/wrote masked (must be identical), AND compare the hash/world numbers with the first compile's preview (stage 3 must not move them - data off): 3,000 2172948422 / 1868952007; ruler 3,000 1023765488 / 3511480669; wanderer 6,000 3662401041 / 309340336; 20,000 195851862 / 1941613769; ruler 20,000 3035997636 / 3729142648. Any difference is a finding (the measurement/presentation code moved the world or the full hash).
3. Triage failing automation tests and Python checks: a B7x test bug -> fix the test faithfully to the notes; a code bug -> fix the code; an old test broken -> decide by the header/notes. Re-run single tests with UnrealEditor-Cmd "${J}/Cosmic_Ascension.uproject" -ExecCmds="Automation RunTests Cosmic_Ascension.<Name>;Quit" -unattended -nullrhi -nosplash -log=<file> after rebuilding. Up to three rounds; then run the runner once more (-Data -UpdateBaseline) to confirm green twice inside it.
4. Revert generated files, LF check, commit source/test fixes as '[B-7x3·4m·4] 첫 테스트 고침(통합 브랜치)' with bullets per failure (test - cause - fix).
Return status, commits, errors_fixed, remaining, results (tests per pass, Python checks, the masked comparison, the hash comparison), notes_for_lead.`, { label: 'test', phase: 'Test', schema: STEP, effort: 'xhigh' })
}

phase('Check')
const check = await agent(`${CONTEXT}

YOUR TASK (read-only reviewer, effort high): read every commit this check made after db9bf9f (git -C "${INT}" log --oneline db9bf9f..HEAD; diff). Reports: BUILD ${JSON.stringify(build)} TEST ${JSON.stringify(test)}. For each fix: intent kept (header sections 4-6, notes 5.4), old path bit-identical, no test weakened to pass (say clearly if one was), ASCII/LF/includes. If no commits were made, confirm the tree is clean and generated files were reverted.
Return status, commits [], errors_fixed [], remaining (concerns), results, notes_for_lead.`, { label: 'check', phase: 'Check', schema: STEP, effort: 'high' })

return { build, test, check }
