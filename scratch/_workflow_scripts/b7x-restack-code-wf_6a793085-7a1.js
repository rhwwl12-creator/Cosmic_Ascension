export const meta = {
  name: 'b7x-restack-code',
  description: 'Re-stack the B7x code commits (fields + mechanism) onto main 220129d in the integration worktree, resolving conflicts by header §8.1, squash into one unbuilt commit, then verify the delta is equivalent and fix',
  phases: [
    { title: 'Restack', detail: 'cherry-pick four code commits, resolve, squash, regenerate docs/30, light Python checks, API drift' },
    { title: 'Verify', detail: 'independent delta-equivalence and section 8.1 check' },
    { title: 'Fix', detail: 'apply confirmed discrepancies, amend the commit' },
  ],
}

const SCR = 'C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad'
const INT = SCR + '/b7x/int'
const WT = SCR + '/b7x/wt'
const OUT = SCR + '/b7x/restack'
const LF = SCR + '/b7x/tools/lf_norm.py'
const MAIN = 'C:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension'
const H = 'docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md'
const N = 'docs/60_DECISIONS/headers/B7_composable_existence_and_evolution_notes.md'

const CONTEXT = `
Project: Cosmic Ascension, UE 5.8.3 C++ game module Cosmic_Ascension (Windows). Block B6b is being built and measured in the MAIN checkout right now by another workflow; block B7x (composable existence) was written on a PC without the engine as branch design/b7x-composable-existence (base cef8cc7). The developer wants B7x integrated in parallel as far as possible WITHOUT touching main or the build machine until B6b ends. So we re-stack B7x onto main in a separate integration worktree, unbuilt for now; it will be rebased onto the final main, built and tested after B6b.

Places:
- INT = ${INT}: git worktree on local branch integrate/b7x, now at main's 220129d ([B-6b7]). You may commit ONLY on integrate/b7x here. Never push.
- WT = ${WT}: the design-branch worktree (another workflow is editing its docs right now) - read B7x documents only through git show at b57449c, e.g. git -C "${INT}" show b57449c:${H} (the design commits are in the shared repository, so any worktree can show them).
- MAIN = ${MAIN}: read-only for you. Never run git write commands there, never edit there.
- OUT = ${OUT}: your scratch folder (create it).
- B7x code commits to re-stack, in order: 01eef6f [B-7x1] fields, 492c4f0 [B-7x2] mechanism, 5969bd5 [B-7x2r] review fixes, 85f297c [B-7x2c] comment section numbers. (Design docs 4555c89/0bffb8e/b57449c and the data draft aa09543 are NOT part of this step.)
- The header's section 8.1 (git show b57449c:${H}, search '### 8.1') lists the true conflicts and how to resolve them; notes section 5.4/5.5 (git show b57449c:${N}) list the review's rebase hand-offs - this step does only the conflict resolution and the mechanical API drift; the review hand-offs and the leaf renames are a later step (do not apply them now).

HARD RULES
- No Unreal builds, no editor, no Tools/run_checks.ps1, no Tools/make_data_tables.py (it launches the editor). Light Python checks only, run from INT's root: python -I -X utf8 Tools/check_data.py; python -I -X utf8 Tools/test_data_schema.py; python -I -X utf8 Tools/check_layers.py --strict; python -I -X utf8 Tools/make_config_reference.py --check; python -I -X utf8 Tools/check_docs.py. (Asset stamps cannot be regenerated without the editor - a stamp/sources-in-sync complaint caused only by changed Tools/data files is expected; say so.)
- Line endings: the project keeps LF on disk, but git here writes CRLF when it touches files. Pass -c core.eol=lf to every git command that writes files in INT (cherry-pick, checkout, reset --hard, apply, rebase), and run python -I -X utf8 ${LF} "${INT}" after each such command; finish with python -I -X utf8 ${LF} "${INT}" --check (must print 0). 'M' lines in git status with numstat 0/0 are stat noise - judge real changes with git -c core.quotepath=false diff --numstat and ignore 0/0 lines.
- Use GIT_EDITOR=true for cherry-pick --continue so no editor opens. rerere is enabled in this repository (keep it on: -c rerere.enabled=true) so later rebases reuse your resolutions.
- Source stays ASCII (comments in English); Korean only in Tools/data and ST_CA.csv. Include paths from the module root.
- Keep main's lines: main has B6b commits after the branch base (cef8cc7 -> 220129d: [B-6b4] presentation, [B-6b5] text and voice, [B-6b6] data 1, [B-6b7] data 2). Never drop or revert a main change to make a branch hunk fit.
`

const RESTACK_SCHEMA = {
  type: 'object',
  properties: {
    commit: { type: 'string' },
    conflicts: { type: 'array', items: { type: 'string' } },
    api_drift_fixed: { type: 'array', items: { type: 'string' } },
    compile_risks: { type: 'array', items: { type: 'string' } },
    python_checks: { type: 'array', items: { type: 'string' } },
    walked_pin: { type: 'string' },
    settle_block_count: { type: 'string' },
    lf_check: { type: 'string' },
    report_file: { type: 'string' },
  },
  required: ['commit', 'conflicts', 'api_drift_fixed', 'compile_risks', 'python_checks', 'walked_pin', 'settle_block_count', 'lf_check', 'report_file'],
}

const VERIFY_SCHEMA = {
  type: 'object',
  properties: {
    discrepancies: {
      type: 'array',
      items: {
        type: 'object',
        properties: {
          id: { type: 'string' },
          severity: { type: 'string', enum: ['high', 'medium', 'low'] },
          file: { type: 'string' },
          what: { type: 'string' },
          evidence: { type: 'string' },
          fix: { type: 'string' },
        },
        required: ['id', 'severity', 'file', 'what', 'evidence', 'fix'],
      },
    },
    equivalence_summary: { type: 'string' },
  },
  required: ['discrepancies', 'equivalence_summary'],
}

const FIX_SCHEMA = {
  type: 'object',
  properties: {
    applied: { type: 'array', items: { type: 'string' } },
    skipped: { type: 'array', items: { type: 'string' } },
    commit: { type: 'string' },
    python_checks: { type: 'array', items: { type: 'string' } },
    lf_check: { type: 'string' },
  },
  required: ['applied', 'skipped', 'commit', 'python_checks', 'lf_check'],
}

phase('Restack')
const restack = await agent(`${CONTEXT}

YOUR TASK (re-stack, effort xhigh):
1. Read header section 8.1 and the conflict notes first. Confirm INT is clean at 220129d (git -C "${INT}" log --oneline -1; real changes 0).
2. Cherry-pick the four commits in order into INT: GIT_EDITOR=true git -C "${INT}" -c core.eol=lf -c rerere.enabled=true cherry-pick <commit>. On a conflict, resolve by section 8.1:
   - Tools/data/ST_CA.csv: keep every main line; the branch's new key line ("Settle.Block.Inward", ...) goes after main's last line.
   - Tools/data/schema.json 'words': keep every main entry exactly (incl. [B-6b5]'s holy_passive with 13 words and its nine lists, [B-6b6]'s ark_words and '노아'); drop the branch's old copies of holy_passive/record_object_words; add only the branch's time_vectors entry, as the LAST entry of 'words'. Other branch schema changes (root list, rules) apply as the branch wrote them, placed after main's entries where order matters.
   - Tools/test_data_schema.py: keep main's tests; take the branch's new tests; the 'walked' pin is not added up by hand - run the test file, read the real walk count and set the pin to it; record main's count and the branch's delta in OUT/restack_report.md.
   - Tools/data/DT_Rules.json: keep main's values (e.g. AIUpperRungProgressMultiplier 0.375, and every [B-6b6]/[B-6b7] value); the branch's new knobs go at the end with the branch's no-op values.
   - docs/30_CONFIG_REFERENCE.md: take main's text during the picks; after the last pick regenerate it with Tools/make_config_reference.py (read its usage first) so --check passes.
   - Settle-block reason count: read main's ECosmicSettleBlock at 220129d; the branch adds Inward - set the three static_asserts the branch touches (Sim/CosmicMegaprojects.cpp, UI/CosmicSystemPanelWidget.cpp, Tests/CosmicBalanceCommandlet.cpp) to the real count and keep main's ReasonOf(..., Tier) / SettleReason wiring.
   - Source/test files (GalaxySimulationSubsystem.*, CosmicEdicts.*, CosmicHazards.*, CosmicMegaprojects.*, CosmicDiplomacy.*, CosmicEventSubsystem.*, CosmicDefinitionRows.h, CosmicConditionTypes.h, CosmicTags.h, GalaxyState.h, CosmicSeed.h, Tests/*.cpp): keep main's B6b code; place the branch's additions at the same logical place. Enumerators stay appended at the end (rule: enumerators only at the end) - if main appended enumerators after the branch base, the branch's new enumerators go after main's.
   After each resolved file: git -C "${INT}" add <file>; then GIT_EDITOR=true git -C "${INT}" -c core.eol=lf cherry-pick --continue. Run the LF normalizer after every pick.
3. Squash the four picks into one commit: git -C "${INT}" reset --soft 220129d, then commit with a message file you write to OUT/msg_b7x12.txt (UTF-8, LF): title '[B-7x1+2] 필드와 장치 — 통합 브랜치, 빌드 전(01eef6f·492c4f0·5969bd5·85f297c를 220129d 위로 다시 쌓음)'; body: short '- ' bullets in plain Korean on what was resolved and that this is an unbuilt working commit of the integration branch (it is rewritten after B6b, when it is built and tested; the leaf renames and the review hand-offs come in the next step); last line exactly: Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
4. API drift: list every function, field, enumerator, include and macro the B7x code uses from main code whose declaration changed between cef8cc7 and 220129d (git -C "${INT}" diff cef8cc7 220129d -- Source/). For each use, compare with the current declaration in INT. Fix purely mechanical mismatches (renamed parameter order, an added argument with an obvious value, a moved include) in the squashed commit (git commit --amend with the same message plus one bullet 'API 맞춤: ...'); list anything that needs judgment as a compile risk with file:line and the reason. Also read the new files Sim/CosmicExistenceBricks.h/.cpp and Tests/CosmicExistenceBricksTests.cpp once for anything that cannot compile against 220129d (missing include, a symbol that main renamed, a signature mismatch) - this code has never been compiled.
5. Run the five light Python checks from INT's root and record exit codes and the key lines.
6. python -I -X utf8 ${LF} "${INT}" --check must print 0. Write OUT/restack_report.md (Korean or English) with: each conflict and its resolution, the walked pin arithmetic, the settle-block count, the API drift list, the compile-risk list, the Python check results, the final commit hash.
Return the structured result.`, { label: 'restack:code', phase: 'Restack', schema: RESTACK_SCHEMA, effort: 'xhigh' })

log(`restack: ${restack ? restack.commit : 'FAILED'}`)

phase('Verify')
const verify = await agent(`${CONTEXT}

YOUR TASK (independent verifier, effort xhigh - do not edit anything; you may write scratch scripts in ${OUT}/verify/).
The re-stack step reported:
${JSON.stringify(restack, null, 1)}

Check that integrate/b7x's single commit on top of 220129d carries exactly the branch's code delta and loses nothing of main:
1. Branch delta = git -C "${INT}" diff cef8cc7 85f297c (these four commits touch only Source/, Config/, Tools/ and docs/30). Integration delta = git -C "${INT}" diff 220129d integrate/b7x. Compare them file by file, ignoring line numbers: every added and removed line of the branch delta must be in the integration delta, except at the resolution spots that header section 8.1 prescribes (ST_CA end, schema words with only time_vectors added at the end, the walked pin recounted, DT_Rules main values kept, docs/30 regenerated, the settle-block static_asserts) and the API-drift fixes the report lists. Any extra '-' line in the integration delta that removes main content (a line that exists at 220129d and is not removed by the branch relative to cef8cc7) is a defect. Write a small Python script for this comparison and keep its output in ${OUT}/verify/.
2. Check each section 8.1 rule is applied as written (read git -C "${INT}" show b57449c:${H}, section 8.1).
3. Check enumerators were appended at the end after main's, ASCII-only Source, includes from the module root, no b'\\r' in changed files.
4. Read the API-drift and compile-risk lists critically: confirm or refute each mechanical fix against the current declarations; add any compile risk the step missed in the files it touched (e.g. a call site that now needs an argument B6b added).
Return discrepancies (each with an exact fix) and an equivalence summary (files compared, lines matched, exceptions explained).`, { label: 'verify:delta', phase: 'Verify', schema: VERIFY_SCHEMA, effort: 'xhigh' })

const discrepancies = (verify && verify.discrepancies) || []
log(`verify: ${discrepancies.length} discrepancies`)

let fix = null
if (discrepancies.length > 0) {
  phase('Fix')
  fix = await agent(`${CONTEXT}

YOUR TASK (fixer, effort high): apply the verifier's discrepancies below to integrate/b7x in INT, re-run the five light Python checks and the LF check, and amend the single commit (git -C "${INT}" commit --amend - keep the title, add one bullet per kind of fix). If a discrepancy is wrong on inspection, skip it and say why.
Re-stack report: ${JSON.stringify(restack)}
Discrepancies: ${JSON.stringify(discrepancies, null, 1)}`, { label: 'fix:restack', phase: 'Fix', schema: FIX_SCHEMA, effort: 'high' })
}

return { restack, verify, fix }
