export const meta = {
  name: 'b7x-followups-r1',
  description: 'Small follow-ups on the rebased B7x integration branch: stale count prose (D1), B7x board helpers for main\'s new side-row test board (D2), the deferred TheodicyProblems parasite term (si3) with its mutation, re-cut of the eight main b6b8_ patches that stop on B7x; folded into the right commits; targeted build and tests; review',
  phases: [
    { title: 'Apply', detail: 'fixups folded by autosquash; per-commit Python checks' },
    { title: 'Prove', detail: 'build through the junction, targeted tests, kill the new mutation' },
    { title: 'Review', detail: 'read-only check' },
  ],
}

const SCR = 'C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad'
const INT = SCR + '/b7x/int'
const J = 'C:/Users/rhwwl/AppData/Local/Temp/b7xi'
const OUT = SCR + '/b7x/followups1'
const LF = SCR + '/b7x/tools/lf_norm.py'
const UE = 'C:/Program Files/Epic Games/UE_5.8'

const CONTEXT = `
Project: Cosmic Ascension (UE 5.8.3 C++). The B7x integration branch integrate/b7x in INT = ${INT} was just rebased onto main 8b6c5a7 and reshaped into five commits: bf0627c [B-7x0], 668786a [B-7x1], a62226d [B-7x4m], d281529 [B-7x3], 864c7df [B-7x4] (tip; built and green: 238 tests twice, CSVs equal to main's). Never touch the MAIN checkout (C:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension - block B6b's [B-6br] runs there now) and never push. Build only through the junction J = ${J} (Remove-Item Env:PROJECT_ROOT first; -WaitMutex -NoHotReloadFromIDE). Git commands that write files in INT get -c core.eol=lf, then python -I -X utf8 ${LF} "${INT}". Fold every change into the commit it belongs to with git commit --fixup=<commit> and then GIT_SEQUENCE_EDITOR=true git -c core.eol=lf -c rerere.enabled=true rebase -i --autosquash 8b6c5a7 (non-interactive). Source ASCII; includes from the module root. Busy courtesy before builds/tests: if a python process with bundleK, bundleA, bundleB, bundleC, lanes, run_mutations, run_py, run_timing or timing in its command line is running, wait (check every 60 s, up to 60 min) - the [B-6br] timing session needs a quiet machine; keep your machine use short. The review report of the rebase is in ${SCR}/b7x/rebase1/review/ (read it).
`

const STEP = {
  type: 'object',
  properties: {
    status: { type: 'string', enum: ['ok', 'partial', 'blocked'] },
    commits: { type: 'array', items: { type: 'string' } },
    done: { type: 'array', items: { type: 'string' } },
    remaining: { type: 'array', items: { type: 'string' } },
    results: { type: 'array', items: { type: 'string' } },
  },
  required: ['status', 'commits', 'done', 'remaining', 'results'],
}

phase('Apply')
const apply = await agent(`${CONTEXT}

YOUR TASK (effort high): four follow-ups, each folded into its commit:
D1 (docs, into [B-7x4] or the commit whose text it is): the stale walked/test counts the review lists - Tools/mutations/README.md rows b7x2_schema_k4_without_star_tow '(the walked count 358)' -> the real count at [B-7x1] (372), b7x4_schema_card_title_without_crossroads and b7x4_schema_lived_lines_without_silence '(the count 388)' -> 402, and the other stale count sentences the review names (header section 11, notes) - each into the commit where that text first appears.
D2 (into [B-7x1]): main's [B-6b8] test file Source/Cosmic_Ascension/Tests/CosmicSideRowTests.cpp builds a seed-42 board from the game's rules; give it the B7x board helpers exactly as notes 5.4 '시1' requires for every such board (WithInertBricks on the rules, StripBricks after NewGalaxy) so B7x data, when it comes, cannot move Sim.EraEndured / Sim.WitnessCard.
si3 (into [B-7x1]): notes 5.4 '시3' (deferred until main had [B-6b8b] - it has now): in Tests/CosmicDataTests.cpp TheodicyProblems, after the edict-bonus loop, Longest += CosmicBricks::ParasiteBonusBoundOf(Rules, Tag); with #include "Sim/CosmicExistenceBricks.h" and the comment the notes give; recompute the expected break threshold on today's data (the [B-6b7] law bonus +50 makes the plague's longest stand 350; use ${SCR}/b6b8b/w_recount.py on INT's Tools/data to confirm, and say the threshold) and add the mutation patch Tools/mutations/B7x/b7x2_parasite_bonus_400.patch (the knob BrickParasiteCountdownBonusTicks set to 400 on... per the notes: a data patch that must break Data.Integrity) with its README row.
R (records, into [B-7x1] or the commit that first breaks each): re-cut main's eight b6b8_ patches that stop applying on B7x as b7x<N>_ copies against the commit where they first stop (the review lists them: at [B-7x1] b6b8_data_extinction_expiry_effect, b6b8_data_era_no_ruled_past_checker, b6b8_schema_witness_past_forms_dropped, b6b8_schema_witness_adnominal_forms_dropped; at [B-7x4m] b6b8_wanderer_nocheck_inverted, b6b8_wanderer_nocheck_reads_next_choice; at [B-7x4] b6b8_schema_era_shape_dropped, b6b8_schema_witness_rule_dropped) - same logical mutation, git apply --check passes at that commit and at the tip; mark main's rows '(B7x 뒤로는 b7x<N>_…가 대신)' in the README.
After the autosquash: at each of the five commits run the five light Python checks (export with git -c core.autocrlf=false -c core.eol=lf archive to a scratch dir, or check out each commit in a temporary worktree via the junction-free path) and record them in OUT/per_commit.md; LF check; clean tree.
Return status, commits (new hashes), done, remaining, results.`, { label: 'apply', phase: 'Apply', schema: STEP, effort: 'high' })

let prove = null
if (apply && apply.status !== 'blocked') {
  phase('Prove')
  prove = await agent(`${CONTEXT}

Apply report: ${JSON.stringify(apply)}
YOUR TASK (effort high): busy courtesy; build the editor target at the tip through J; run the targeted tests with UnrealEditor-Cmd "${J}/Cosmic_Ascension.uproject" -ExecCmds="Automation RunTests Cosmic_Ascension.Data.Integrity+Cosmic_Ascension.Sim.EraEndured+Cosmic_Ascension.Sim.WitnessCard;Quit" -unattended -nullrhi -nosplash -log=<OUT file> (rebuild the assets first if Tools/data changed: run INT's Tools/make_data_tables.py through the editor as the runner does, or the runner's -Data step only); then apply the new si3 mutation patch, rebuild assets/DLL as needed, run Data.Integrity (must fail), revert, rebuild clean. Record everything in OUT/prove.md. Revert generated files (seed42.json, generated.sha1, Content/**/*.uasset) at the end; clean tree.
Return status, commits [], done, remaining, results.`, { label: 'prove', phase: 'Prove', schema: STEP, effort: 'high' })
}

phase('Review')
const review = await agent(`${CONTEXT}

YOUR TASK (read-only, effort high): reports APPLY ${JSON.stringify(apply)} PROVE ${JSON.stringify(prove)}. Check each fixup landed in the right commit (git log -p of the five commits), the si3 term and threshold against the notes' arithmetic, the D2 helpers match the other boards' pattern, the eight re-cut patches mutate the same logic as main's originals, the per-commit Python checks are recorded, and the tree is clean. Report defects with exact fixes, or confirm.
Return status, commits [], done [], remaining (defects), results.`, { label: 'review', phase: 'Review', schema: STEP, effort: 'high' })

return { apply, prove, review }
