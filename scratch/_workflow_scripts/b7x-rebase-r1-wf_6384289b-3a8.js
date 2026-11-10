export const meta = {
  name: 'b7x-rebase-r1',
  description: 'Rebase the B7x integration branch onto current main 8b6c5a7 and reshape it into the final header-8 commits ([B-7x0], [B-7x1] squashed, [B-7x4m], [B-7x3], [B-7x4] with the review fixes folded in), resolving conflicts main-first; Python checks per commit; then build and test the tip through the junction; review',
  phases: [
    { title: 'Rebase', detail: 'backup ref, rebuild the five commits on 8b6c5a7, resolve, per-commit Python checks' },
    { title: 'Test', detail: 'tip build and runner through the junction; CSV equality with main; triage' },
    { title: 'Review', detail: 'read-only check of the resolutions and the folding' },
  ],
}

const SCR = 'C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad'
const INT = SCR + '/b7x/int'
const J = 'C:/Users/rhwwl/AppData/Local/Temp/b7xi'
const OUT = SCR + '/b7x/rebase1'
const LF = SCR + '/b7x/tools/lf_norm.py'
const MAIN = 'C:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension'
const UE = 'C:/Program Files/Epic Games/UE_5.8'
const H = 'docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md'

const CONTEXT = `
Project: Cosmic Ascension (UE 5.8.3 C++ module Cosmic_Ascension, Windows). MAIN = ${MAIN} is on main 8b6c5a7 ([B-6b8b]); block B6b continues there (another workflow, step [B-6br]) - never edit, build, stage or git-write in MAIN. Block B7x lives in INT = ${INT} on the LOCAL branch integrate/b7x, built through the junction J = ${J} (UBT's 260-char limit). integrate/b7x today (base 220129d, all compiled and green, data off bit-identical):
  672aa5e [B-7x0] approval docs; 40cc67b fields+mechanism; b2c7348 leaf renames; 423adff review hand-offs; 1333de4 their review fixes; 5b6f0a2 [B-7x4m] measurement; 9ce7709 [B-7x3] presentation; 555afed [B-7x4] texts; db9bf9f review fixes of the last three.
The developer asked to integrate B7x in parallel as far as possible; main must not receive B7x before B6b ends, but the LOCAL integration branch may be rebased onto main's latest any time. A session on another PC ran 'step 0' (git merge-tree of the old design branch onto main) and confirmed the true conflict sites; for integrate/b7x vs 8b6c5a7 git merge-tree reports conflicts in: Tools/data/schema.json, Tools/mutations/README.md, Tools/test_data_schema.py, docs/20_ARCHITECTURE.md, docs/21_DATA_MODEL_REFERENCE.md, docs/40_DEV_GUIDE.md (DT_Encounters/DT_Events do not conflict - B7x data rows are not in the branch yet; they come later from port scripts). Auto-merged but semantically shared: CosmicBalanceCommandlet.cpp (main [B-6b8] witness counters vs B7x bricks:/-WithoutB7xData), CosmicWandererPolicy.cpp/.h (main [B-6b8] NoCheckByRow/WitnessFirst vs B7x PickOf - the B7x header section 8.1 '[B-7x4m]' rebase item says how: read Row->Choices[Pick].CheckStat after Pick), CosmicDataTests.cpp (main's WithoutB6bData tripwire of ten names vs B7x's tripwire), CosmicDiplomacyTests.cpp (main [B-6b8b] Sim.FleetDelay vs B7x board helpers), CosmicDefinitionRows.h, DT_Rules.json (main changed two values; B7x knobs at the end), systems.md, docs/30 (regenerate, never merge by hand).
Resolution rules (main first - header ${H} section 8.1): keep every main line; schema 'words': all of main's lists unchanged (incl. [B-6b8]'s witness_words, sign_voice_words, witness_record_words), then B7x's time_vectors, shape_merit_words, crossroads_words at the end; schema rule lists: main's rules first, B7x's after; README: both sides' rows (B6b rows stay in the B6b table, B7x rows in the B7x table); docs: keep both sides' sentences, re-reading for contradictions; test_data_schema.py 'walked' pin: never add by hand - run the test and set the real count, per commit; Tools/data/generated.sha1: take main's (assets are rebuilt by the runner later).
RULES: work only in INT; never push; LF (git commands that write files get -c core.eol=lf; then python -I -X utf8 ${LF} "${INT}"); Source ASCII, includes from the module root; trailer of every commit message: Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>; OUT = ${OUT} for scratch.
`

const STEP = {
  type: 'object',
  properties: {
    status: { type: 'string', enum: ['ok', 'partial', 'blocked'] },
    commits: { type: 'array', items: { type: 'string' } },
    resolved: { type: 'array', items: { type: 'string' } },
    remaining: { type: 'array', items: { type: 'string' } },
    results: { type: 'array', items: { type: 'string' } },
    notes_for_lead: { type: 'array', items: { type: 'string' } },
  },
  required: ['status', 'commits', 'resolved', 'remaining', 'results', 'notes_for_lead'],
}

phase('Rebase')
const rebase = await agent(`${CONTEXT}

YOUR TASK (rebase and reshape, effort xhigh):
1. Backup: git -C "${INT}" branch integrate/b7x-on-220129d db9bf9f (keep it until the lead deletes it).
2. Make INT's integrate/b7x start at main: git -C "${INT}" -c core.eol=lf checkout -B integrate/b7x 8b6c5a7 (then the LF normalizer).
3. Rebuild the history as FIVE commits, in this order, with git -c core.eol=lf -c rerere.enabled=true cherry-pick (use -n to accumulate where commits are squashed), resolving each conflict by the rules:
   (1) [B-7x0] = 672aa5e (title kept, body: add one bullet 'main 8b6c5a7 위로 다시 쌓음').
   (2) [B-7x1] = 40cc67b + b2c7348 + 423adff + 1333de4 squashed; title '[B-7x1] 필드와 장치 — 시간 셈 여섯·HostCivID·Brick 태그 열일곱과 CosmicBricks, 잎 이름(결정 3), 리뷰 넘김(통합 브랜치)'; body: the four source commits' bullets condensed in plain Korean + one bullet that it is not yet proven per commit (that proof is the final re-stack after B6b).
   (3) [B-7x4m] = 5b6f0a2 + the hunks of db9bf9f that belong to it (measurement files: commandlet, ruler/wanderer policies, CosmicTestHelpers WithoutB7xData, the Data.Integrity B7x tripwire, CosmicCrossroadsPolicyTests.cpp, CosmicB7xRowNames.h if its first users are here, docs/40 measurement rows, the b7x4m_ patches and README rows).
   (4) [B-7x3] = 9ce7709 + db9bf9f's presentation hunks (UI, PlayerRoleSubsystem scan line, CosmicRowNames.h, DT_LogTemplates row, ST_CA keys incl. Diplomacy.TipBricks text, schema Shape. parts, the scan/chronicle/panel tests, b7x3_ patches).
   (5) [B-7x4] = 555afed + db9bf9f's remaining hunks (crossroads text rules, DevComments, header/notes/design-doc text, b7x4_ patches).
   Split db9bf9f by file and, for shared files, by hunk (git show db9bf9f -- <file>; apply the pieces with git apply --3way or by hand) so that each of (3)-(5) holds its own fixes; the end tree after (5) must equal what the plain rebase of all nine commits would give (prove it: keep a scratch plain-rebase result in a temporary worktree under OUT or compare against git merge-tree --write-tree main db9bf9f plus your resolutions, and diff the trees; any difference must be explained by a resolution).
   Semantic merges: the wanderer Act (main's NoCheckByRow/WitnessFirst with B7x's PickOf - per header section 8.1, read CheckStat of Row->Choices[Pick] after Pick, keep main's witness-first behaviour), the commandlet (both sides' counters and lines, main's lines first), the tripwires (both), Sim.FleetDelay with the B7x board helpers (both).
4. After EACH commit: the five light Python checks from INT's root (check_data, test_data_schema - recount the walked pin and amend that commit if needed, check_layers --strict, make_config_reference --check - regenerate docs/30 if stale and amend, check_docs) and the LF check. Record results per commit in OUT/per_commit.md.
5. git -C INT log --oneline 8b6c5a7..HEAD must show the five commits.
Return status, commits (hash + title), resolved (each conflict and how), remaining, results, notes_for_lead (e.g. the walked pins, anything that needs the build to judge).`, { label: 'rebase', phase: 'Rebase', schema: STEP, effort: 'xhigh' })
log(`rebase: ${rebase ? rebase.status : 'FAILED'}`)

let test = null
if (rebase && rebase.status !== 'blocked') {
  phase('Test')
  test = await agent(`${CONTEXT}

Rebase report: ${JSON.stringify(rebase)}
YOUR TASK (tip build and test, effort xhigh):
1. Busy courtesy first: no B6b harness running (python processes whose command line contains bundleK, bundleA, bundleB, bundleC, lanes, run_mutations, run_py, run_checks, timing) - else wait (check every 60 s, up to 60 min). Build.bat's lock serializes builds.
2. Build editor and game targets through J (Remove-Item Env:PROJECT_ROOT -ErrorAction SilentlyContinue; Build.bat ... "-Project=${J}/Cosmic_Ascension.uproject" -WaitMutex -NoHotReloadFromIDE). Fix compile errors in the commit that introduced them (git commit --fixup=<that commit> then GIT_SEQUENCE_EDITOR=true git -c core.eol=lf rebase -i --autosquash 8b6c5a7), never as a loose commit.
3. Runner through J: & '${J}/Tools/run_checks.ps1' -Tier full -Data -UpdateBaseline -> OUT/runner_1.txt. Then compare: (a) the five balance CSVs + _rows + _claims of INT (Saved/Balance/checks*.csv in INT) with MAIN's (Saved/Balance/checks*.csv - written by main's [B-6b8b] final runner, same arguments; read-only) - identical apart from TickMicros, SaveBytes, SaveRawBytes (B7x data is off, so the world must be main's); (b) the five summary lines with hash/world masked against MAIN's Tools/baselines/seed42.json; record the new hash pairs (they move: new saved fields).
4. Triage failures: fix in the commit that owns the code (fixup + autosquash). Re-run until green (up to three rounds).
5. Revert generated files (seed42.json, generated.sha1, Content/**/*.uasset) with -c core.eol=lf checkout; LF check; the tree must be clean.
Return status, commits (final hashes), resolved, remaining, results (tests per pass, CSV comparison, masked lines, new hashes), notes_for_lead.`, { label: 'test', phase: 'Test', schema: STEP, effort: 'xhigh' })
}

phase('Review')
const review = await agent(`${CONTEXT}

YOUR TASK (read-only reviewer, effort xhigh): reports: REBASE ${JSON.stringify(rebase)} TEST ${JSON.stringify(test)}.
Check: (1) every main line survived in the conflicted files (diff 8b6c5a7 against the new tip for schema.json words/rules, README, test_data_schema.py, docs/20/21/40 - nothing of main removed); (2) the semantic merges (wanderer Act, commandlet, tripwires, Sim.FleetDelay) keep both sides' behaviour; (3) db9bf9f's hunks each landed in the right commit and nothing was lost (the end tree vs the plain-rebase reference); (4) per-commit Python checks recorded; (5) generated files reverted, LF clean, backup branch present. Report concrete defects with exact fixes; say clearly if the branch is good to keep.
Return status, commits [], resolved [], remaining (defects), results, notes_for_lead.`, { label: 'review', phase: 'Review', schema: STEP, effort: 'xhigh' })

return { rebase, test, review }
