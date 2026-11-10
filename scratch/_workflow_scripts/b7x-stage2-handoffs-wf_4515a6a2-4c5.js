export const meta = {
  name: 'b7x-stage2-handoffs',
  description: 'B7x integration stage 2 (unbuilt, integration worktree): [B-7x0] docs commit, leaf renames, review hand-offs for the merged [B-7x1+2] (code, tests, schema, docs, mutation patches), then an adversarial read-review and fix',
  phases: [
    { title: 'Docs+Rename', detail: 'one agent: [B-7x0] docs commit, rename_leaves.py commit' },
    { title: 'Implement', detail: 'code, tests, schema, docs on disjoint files; then mutation patches' },
    { title: 'Integrate', detail: 'checks, notes marks, commit' },
    { title: 'Review', detail: 'compile, behaviour, tests, docs lenses' },
    { title: 'Verify', detail: 'one refuter per finding' },
    { title: 'Fix', detail: 'apply surviving findings, amend' },
  ],
}

const SCR = 'C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad'
const INT = SCR + '/b7x/int'
const OUT = SCR + '/b7x/stage2'
const LF = SCR + '/b7x/tools/lf_norm.py'
const REN = SCR + '/b7x/answers/rename/rename_leaves.py'
const MAIN = 'C:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension'
const H = 'docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md'
const N = 'docs/60_DECISIONS/headers/B7_composable_existence_and_evolution_notes.md'
const D = 'docs/10_DESIGN/EXISTENCE_EVOLUTION_AND_ECOLOGY.md'

const CONTEXT = `
Project: Cosmic Ascension, UE 5.8.3 C++ game module Cosmic_Ascension (Windows, solo developer, Korean docs, ASCII-only source with English comments).
Block B6b is being built and measured in the MAIN checkout by another workflow right now. Block B7x (composable existence: brick tags on peoples, six time counts, the CosmicBricks rule module) is being integrated IN PARALLEL in a separate integration worktree, UNBUILT for now: it is rebased onto the final main, built and tested after B6b (and compile-checked in B6b's build-free windows).

Places:
- INT = ${INT}: git worktree on LOCAL branch integrate/b7x. HEAD 40cc67b = one commit '[B-7x1+2] 필드와 장치 — 통합 브랜치, 빌드 전' on main's 220129d. You may commit ONLY on integrate/b7x, never push, never switch branches.
- B7x documents, reviewed and answered, are on the design branch at commit 79f2583 (read any of them with git -C "${INT}" show 79f2583:<path>): header ${H}, notes ${N}, design doc ${D}. The developer answered every decision 2026-11-10 ('권장대로'). The notes section 5.4 lists the review's hand-offs per target commit (문 = doc text, 코 = code, 데 = data/schema, 시 = tests/mutations); line numbers there are branch 0bffb8e lines - find the places by content in INT.
- MAIN = ${MAIN}: read-only for you. Never run git write commands or edit files there.
- OUT = ${OUT}: scratch folder (create it).

HARD RULES
- No Unreal builds, no editor, no Tools/run_checks.ps1, no Tools/make_data_tables.py (it launches the editor). Light Python checks only, run from INT's root: python -I -X utf8 Tools/check_data.py; python -I -X utf8 Tools/test_data_schema.py; python -I -X utf8 Tools/check_layers.py --strict; python -I -X utf8 Tools/make_config_reference.py --check; python -I -X utf8 Tools/check_docs.py. A stale Tools/data/generated.sha1 (Data.SourcesInSync) is expected until the editor rebuilds the assets.
- Line endings: the project keeps LF on disk, but git here writes CRLF when it touches files. Pass -c core.eol=lf to every git command that writes files in INT, and run python -I -X utf8 ${LF} "${INT}" after it; files you write yourself must be LF, UTF-8 without BOM. 'M' lines in git status with numstat 0/0 are stat noise - judge real changes with git -c core.quotepath=false diff --numstat (ignore 0/0 lines).
- Source/Config: ASCII only (comments in English); Korean only in Tools/data, ST_CA.csv and docs. Includes from the module root ("Sim/CosmicEdicts.h"). Header file names unique. UE_LOG category LogCosmic only. No FMath::Rand*. Enumerators only appended at the end. Saved-field default values never change once saves exist.
- This code has NEVER been compiled. Write C++ that compiles against UE 5.8 and the INT tree's real declarations: open every header you call into and match its exact signature; never guess an API (engine headers are under C:/Program Files/Epic Games/UE_5.8/Engine/Source). Warnings are errors (shadowing C4456/C4457/C4458, unreachable code, unused variables).
- Narrative rule 11 for any Korean text you add: the holy order (God) is never an entity/speaker/number/lever/ending; no verdict words (천국·지옥·구원받·멸망받·영혼·저주받·심판받) in narrator text; no counts of people as numbers; plain native words.
`

const STEP_SCHEMA = {
  type: 'object',
  properties: {
    commits: { type: 'array', items: { type: 'string' } },
    files_changed: { type: 'array', items: { type: 'string' } },
    done: { type: 'array', items: { type: 'string' } },
    not_done: { type: 'array', items: { type: 'string' } },
    compile_risks: { type: 'array', items: { type: 'string' } },
    python_checks: { type: 'array', items: { type: 'string' } },
    notes_for_others: { type: 'array', items: { type: 'string' } },
  },
  required: ['commits', 'files_changed', 'done', 'not_done', 'compile_risks', 'python_checks', 'notes_for_others'],
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

const VERDICT_SCHEMA = {
  type: 'object',
  properties: {
    verdict: { type: 'string', enum: ['confirmed', 'partial', 'refuted'] },
    reason: { type: 'string' },
    corrected_fix: { type: 'string' },
  },
  required: ['verdict', 'reason', 'corrected_fix'],
}

phase('Docs+Rename')
const first = await agent(`${CONTEXT}

YOUR TASK (one agent, sequential, effort high). Two commits on integrate/b7x, in this order.

COMMIT 1 - '[B-7x0] 승인 문서(통합 브랜치)':
a. Bring the B7x documents from 79f2583 into INT: git -C "${INT}" -c core.eol=lf checkout 79f2583 -- ${H} ${N} ${D} docs/archive/EXISTENCE_EVOLUTION_original_2026-11-09.md docs/archive/OVERVIEW_as_of_B6a.html ; then the LF normalizer. (docs/OVERVIEW_as_of_B6a.html never existed on main - only the archive path is added. Do not touch docs/OVERVIEW.html.)
b. The low-churn main-side approval edits that header section 7 and notes section 5.4 '[B-7x0]' name (read both from 79f2583): the reversal marker on the ruling in docs/10_DESIGN/systems.md (the line that says no Trait.Diet / no ecology sim, around :381 - R1, decision 2; use the wording the header's section 0 R1 row and section 7 give), the B7x marker in docs/60_DECISIONS/headers/B2_edicts_and_research.md section 2.6 (around :125 and :129 - R12), and the three lines at the end of docs/SECTION_MAP.md's table that notes 5.4 [B-7x0] item 6 gives verbatim. The living documents STATUS.md, DECISIONS_LOG.md, OPEN_QUESTIONS.md, docs/50_ROADMAP.md and CLAUDE.md are NOT edited now (B6b keeps changing them; they are written after B6b ends) - say so in the commit body.
c. python -I -X utf8 Tools/check_docs.py must report 0 errors. Commit with a message file you write to OUT/msg_b7x0.txt: title '[B-7x0] 승인 문서 — 통합 브랜치: B7x 헤더·노트·받은 설계서·동결본 둘, systems.md의 번복 표지, B2 헤더의 표지, SECTION_MAP 세 줄'; body '- ' bullets in plain Korean (what came from 79f2583, which main docs got markers, what waits for after B6b: STATUS·DECISIONS_LOG·OPEN_QUESTIONS(번호)·docs/50·CLAUDE.md의 docs/archive 행); last line exactly: Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>

COMMIT 2 - '[B-7x1+2] 잎 이름 여덟(결정 3)':
a. Run the prepared, tested script: python -I -X utf8 ${REN} "${INT}" ; then run it again (must report 0 changes and 'no old name left'). Read its printed follow-ups: regenerate docs/30 with python -I -X utf8 Tools/make_config_reference.py (then --check must pass). The asset rebuild it mentions is for the build PC later - not now.
b. By hand (the script does not): add to the IsInward declaration comment in Source/Cosmic_Ascension/Sim/CosmicExistenceBricks.h: 'Inward here means the two folded-in peoples (Brick.Space.ClosedPort / FoldedSmall) only; it does not read Brick.Mutation.InwardHunger, a different fact (S11)'; and make the ShutSphere tag DevComment in Config/DefaultGameplayTags.ini and its accessor comment in Source/Cosmic_Ascension/Core/CosmicTags.h say 'the shell closed around its own star; not the fallen sphere (InSphere/FallenSphereTicks) nor Trait.Faith.BookShut' (notes 5.4 [B-7x1] item 2, 코7). ASCII only.
c. Check: no old leaf name left in Source/, Config/, Tools/ (case-sensitive: UnseenLaw, SilenceFeed, QuietFeed, BrickQuietInsight, IsQuietOf, PlanckMicro, DysonBrain, KesslerTapestry, AbyssalMaw, StarShepherd, HermitMonolith; case-insensitive prose: kessler, planck, tapestry, abyssal maw, star shepherd, hermit monolith, dyson brain, unseen law, silence feed, quiet feed, quiet tick - but Revelation.Quiet and Digest.Quiet stay); the five light Python checks; LF check (lf_norm --check prints 0).
d. Commit with OUT/msg_b7x12_names.txt: title '[B-7x1+2] 잎 이름 여덟(결정 3) — 통합 브랜치, 빌드 전'; body: the eight renames plus knob BrickIdleInsightPerTick and function IsIdleOf, the count of changed places and files from the script, docs/30 regenerated, the two hand comments; trailer line as above.
Return commits (hash + title), files_changed, done, not_done, compile_risks (anything the renames could break at compile time, e.g. a renamed identifier still referenced in a form the script did not catch), python_checks, notes_for_others.`, { label: 'docs+rename', phase: 'Docs+Rename', schema: STEP_SCHEMA, effort: 'high' })
log(`docs+rename: ${first ? first.commits.join(' | ') : 'FAILED'}`)

phase('Implement')
const codeTask = () => agent(`${CONTEXT}

YOUR TASK (implementer CODE, effort xhigh). You own ONLY: Source/Cosmic_Ascension/Sim/**, Source/Cosmic_Ascension/Core/**, Source/Cosmic_Ascension/Data/CosmicDefinitionRows.h, and the DevComment text of Brick tags in Config/DefaultGameplayTags.ini. Other agents edit Tests/, Tools/ and docs/ at the same time - do not touch those, and do not commit (the integrator commits).
Apply the code hand-offs of notes 5.4 (read: git -C "${INT}" show 79f2583:${N}) for the merged commit, with the new leaf names (the renames are already committed in INT):
- 코1 (decision 18, the narrowing): replace CosmicEdicts::AIRevokesOf in Source/Cosmic_Ascension/Sim/CosmicEdicts.cpp with the version the notes give (KeptLaw people keep held edicts after the throne empties EXCEPT edicts that add to the claim's countdown (HazardCountdownBonus contains Hazard.Claim) or answer a crisis (Tables.IsHazardAnswer), and except while its row asks for galaxy laws (AsksForLawsOf)); check every identifier against the real declarations (CosmicTags accessor names after the rename, FEdictDef fields, FCosmicDataTables::IsHazardAnswer signature, ECosmicEdictScope, HasStabilityCost, Rules.AIEdictRevokeStability). Update the comments the notes name (TickCivAI comment in Sim/GalaxySimulationSubsystem.cpp and the AIRevokesOf declaration comment in Sim/CosmicEdicts.h).
- 코2: the FoldedSmall hazard-share damping must spare the claim, the extinction and the supernova precursor (IsSparedHazard) - Sim/CosmicExistenceBricks.cpp and the doc comment in the .h.
- 코3: add int32 CosmicBricks::ParasiteBonusBoundOf(const FCosmicRulesDef& Rules, const FGameplayTag& Hazard) (0 for an invalid or spared hazard, else FMath::Clamp(Rules.BrickParasiteCountdownBonusTicks, 0, 100000)) and make ParasiteCountdownBonusOf use it - once per hazard. FMath::Clamp is allowed in Core Logic only if Tools/check_layers.py rule 3b's allow-list has it - check the list; if not allowed, write the clamp with plain comparisons.
- 코4: Source/Cosmic_Ascension/Sim/CosmicDiplomacy.cpp - the brick term is added like the stance term and is NOT clamped there (the drift stage clamps the relation); fix the comments in CosmicDiplomacy.cpp/.h as the notes say.
- 코5: CosmicTags::IsBrick in Source/Cosmic_Ascension/Core/CosmicTags.h - case-insensitive StartsWith(TEXT("Brick."), ESearchCase::IgnoreCase) with Tag.IsValid() first.
- 코6 (comments only, bit-identical): Data/CosmicDefinitionRows.h knob comment, the ini DevComments for the KeptLaw and ShutSphere tags (text in the notes, with the new names and the narrowing), Core/CosmicSeed.h head list line for the seed brick.
Do not change behaviour beyond these items. Keep the old path bit-identical for peoples without bricks. Record each change with file:line. Return files_changed, done, not_done, compile_risks (every identifier you could not verify, with why), python_checks (run check_layers --strict at least), notes_for_others (exact facts the tests implementer must match: function names, new signatures, comment-only changes).`, { label: 'impl:code', phase: 'Implement', schema: STEP_SCHEMA, effort: 'xhigh' })

const testsTask = () => agent(`${CONTEXT}

YOUR TASK (implementer TESTS, effort xhigh). You own ONLY: Source/Cosmic_Ascension/Tests/** (C++ automation tests and helpers). Another agent changes Sim/Core code right now (notes 5.4 코1-코6 - the KeptLaw narrowing in CosmicEdicts::AIRevokesOf, FoldedSmall sparing claim/extinction/precursor, new CosmicBricks::ParasiteBonusBoundOf, diplomacy brick term no longer clamped, case-insensitive IsBrick); write tests against the code as those hand-offs describe it. Do not touch Tools/ or docs/, do not commit.
Apply the test hand-offs of notes 5.4 (git -C "${INT}" show 79f2583:${N}) with the new leaf names:
- 시1: helpers WithInertBricks(Rules) and StripBricks(State) in Tests/CosmicTestHelpers.h exactly as the notes describe (inert = the sixteen numeric knobs back to a default-constructed FCosmicRulesDef's values and BrickSeedTags reset; strip = remove every CosmicTags::IsBrick tag from each Civ.Traits, archetype tags stay). Use them in Tests/CosmicExistenceBricksTests.cpp's LayBoard (after NewGalaxy(42, 12, 4)) and in every shared seed-42 board that reads the game's rules and would now see seed bricks or knobs (start with Tests/CosmicDiplomacyTests.cpp LayBoard - the notes say Sim.DiplomacyRules would go from -0.12 to 0.13). Find the other boards by grepping for the game-rules accessor the tests use and judging which boards a seeded brick could move; list them.
- 시2: Sim.SeedBricks Pass 0 on WithInertBricks(*Game); Pass 1 on that copy with Seeding.BrickSeedTags = Rules.BrickSeedTags; expected count per people = number of Brick.* tags in its archetype row's Traits + (1 if Pass 1 and CivIndex > 0).
- 시4 (decision 18): a test (extend Sim.MutationStep or a new Sim.KeptLawEdicts) for both branches: (a) a KeptLaw AI people revokes edict_vigil and edict_machine_rights and keeps edict_mobilization; (b) a KeptLaw AI people whose judged row asks for galaxy laws (Society >= 1.0 so three slots, fourth rung, a row with CivLawsHeldTicksAtLeast, stability under AIEdictRevokeStability, one people-scope edict with no claim hook and no crisis answer - edict_mobilization - and ample stock) ends the AI stage holding the three galaxy laws and HoldsGalaxyLawsOf is true; the control without the narrowing stops at two laws. Use the existing test helpers and the real row names (Data/CosmicRowNames.h constants or the test row-name headers; no new string literals for rows the code names by constant).
- 시5: in Sim.BrickShares, a FoldedSmall owner's Hazard.Extinction and Hazard.PreSupernova spawn shares equal those of an owner without the brick.
- 시6 (decision 8): DimCapitalStar's three refusals (capital being settled, another people's capital, no people index) as three asserts placed AFTER the 'one class' assertion (the notes explain why after), then the class unchanged.
- 시7: the diplomacy expectation without the clamp (Before - 0.4f within 1e-6) and remove/replace the '[-1, 1] 안' expectation; add a board whose base target is below -1 to show the brick term lowers it by exactly the term.
- 시3 waits for [B-6b8b] (TheodicyProblems) - do not do it.
Every new assertion must be one a mutation can break: for each, write in notes_for_others the exact one-line source mutation (file, old text, new text) that should make it fail - the mutation agent will cut patches from your list.
Return files_changed, done, not_done, compile_risks (each test API you used and whether you verified its declaration), python_checks (check_layers --strict), notes_for_others (the mutation list, one entry per assertion group).`, { label: 'impl:tests', phase: 'Implement', schema: STEP_SCHEMA, effort: 'xhigh' })

const schemaTask = () => agent(`${CONTEXT}

YOUR TASK (implementer SCHEMA, effort high). You own ONLY: Tools/data/schema.json, Tools/cosmic_data_schema.py (only if a new rule kind is truly needed), Tools/test_data_schema.py. Do not touch Source/, docs/ or other Tools files, do not commit.
Apply notes 5.4 (git -C "${INT}" show 79f2583:${N}): 데3 - the K4 feat rule in DT_Feats rules (why 'K4 - a rung comes from a feat's deed, never from a brick or a mutation (B7x header K4)'; lacks_type BorderingCivHasTrait; for each of the seventeen Brick. tags (read them from Config/DefaultGameplayTags.ini in INT - new names) lacks_cond {CivHasTrait, Brick.X} and {CivLacksTrait, Brick.X}, in the same shape the schema's existing lacks_cond lists use, tags in alphabetical order); 데3 - Settle.Block.Inward in string_table.checked_prefixes after Settle.Block.RealmCapLast with forbidden [verdict, holy_words, rescue_words, rung_grasp_words, title_words] and no_digits true (check each list name exists in schema words); 시8 - breaking cases in Tools/test_data_schema.py: a feat row with CivHasTrait Brick.Mutation.StarTow must fail, Settle.Block.Inward text carrying a forbidden word must fail, and pin the Inward forbidden list. Then run the schema test, read the real 'walked' count and set the pin to it (never add by hand); record the arithmetic. Run the five light Python checks.
Return files_changed, done, not_done, compile_risks (empty unless a C++ reader is affected), python_checks, notes_for_others (e.g. the new walked pin, anything the docs agent must say).`, { label: 'impl:schema', phase: 'Implement', schema: STEP_SCHEMA, effort: 'high' })

const docsTask = () => agent(`${CONTEXT}

YOUR TASK (implementer DOCS, effort high). You own ONLY: docs/20_ARCHITECTURE.md, docs/21_DATA_MODEL_REFERENCE.md, docs/40_DEV_GUIDE.md (and docs/30 only by re-running Tools/make_config_reference.py if a knob comment changed - another agent may change Data/CosmicDefinitionRows.h comments, so leave docs/30 to the integrator). Do not touch Source/, Tools/, the B7x header/notes, do not commit.
Write what header section 7 '[B-7x1]과 함께, 코드 앞에' and notes 5.4 ([B-7x1] items 1 and 4, 문13) ask (git -C "${INT}" show 79f2583:${H} and :${N}), using the new leaf names and describing the code that is in INT (read the actual fields and steps in Source/Cosmic_Ascension/Core/GalaxyState.h, Sim/CosmicExistenceBricks.h, Sim/GalaxySimulationSubsystem.cpp):
- docs/20_ARCHITECTURE.md: the save paragraph (new saved fields: FCosmicTimeVectors TimeVectors on each civilization, HostCivID on each system with NAME_None and its frozen meaning, rules (ㄱ)(ㄴ); both hashes move once with the merged [B-7x1]); the tick order (the economy-stage-end cumulative count step and the AI-stage-end mutation step, before the feat judgement - check the exact place in the code); the three derived-integer sentences for the seed brick (use 13 'Brick') that header section 7 gives verbatim with their anchors (docs/20 :204, :208, :220 at 10205b6 - find by content now).
- docs/21_DATA_MODEL_REFERENCE.md sections 7.2, 7.3, 7.4: the new types/fields/functions (FCosmicTimeVectors, TimeVectors, HostCivID with (ㄱ)(ㄴ), the CosmicBricks window functions incl. the settle gate's Inward reason, the conditions CivTimeVectorAtLeast/CivLacksTrait/BorderingCivHasTrait, the AIRevokesOf narrowing for KeptLaw per decision 18, ParasiteBonusBoundOf) - docs/21 is a copy of the code; the code wins.
- docs/40_DEV_GUIDE.md: the test table rows for the B7x tests (Tests/CosmicExistenceBricksTests.cpp test names - read them; new ones being added now: the KeptLaw edicts test, the BrickShares extinction case, the DimCapitalStar refusals, the diplomacy no-clamp expectation, helpers WithInertBricks/StripBricks) and the mutation folder Tools/mutations/B7x/.
Plain Korean in the documents' style; mark nothing as done that is not in the code. Run check_docs (0 errors).
Return files_changed, done, not_done, compile_risks (empty), python_checks, notes_for_others.`, { label: 'impl:docs', phase: 'Implement', schema: STEP_SCHEMA, effort: 'high' })

const impl = await parallel([codeTask, testsTask, schemaTask, docsTask])
const [resCode, resTests, resSchema, resDocs] = impl
log(`implement: code ${resCode ? 'ok' : 'FAILED'}, tests ${resTests ? 'ok' : 'FAILED'}, schema ${resSchema ? 'ok' : 'FAILED'}, docs ${resDocs ? 'ok' : 'FAILED'}`)

const mutations = await agent(`${CONTEXT}

YOUR TASK (mutation patches, effort high). You own ONLY Tools/mutations/** in INT. The code, tests, schema and docs implementers have finished (uncommitted in INT). Their reports:
CODE: ${JSON.stringify(resCode)}
TESTS: ${JSON.stringify(resTests)}
SCHEMA: ${JSON.stringify(resSchema)}
1. Read Tools/mutations/README.md for the folder and row conventions (how B6b patches are named, cut and listed). Create Tools/mutations/B7x/ and cut one patch per new assertion group from the TESTS report's mutation list, plus the mutations notes 5.3 item 5 and 5.4 name for this commit (git -C "${INT}" show 79f2583:${N}: initial value 1, hash skips TimeVectors, the count goes back, Inward not asked, parasite touches the claim, KeptLaw's first question removed, seed draws from the sim stream; 시4's b7x2_kept_law_keeps_claim_edicts and b7x2_kept_law_keeps_slots; 시5's b7x2_inward_damps_extinction; 시6's guard removal) - names b7x2_<what>.patch. Each patch: a unified diff against the current INT working files, LF, minimal (one logical change), and git -C "${INT}" apply --check passes; apply and reverse each once to prove it is clean (git -C "${INT}" -c core.eol=lf apply / apply -R, then the LF normalizer), leaving the tree exactly as before. Add one README row per patch in the folder's table style (patch, what it breaks, which test should fail).
2. 시12: re-spin the five main patches the merge stalls (Tools/mutations/B3/b3b_no_trade_income.patch, B6b/b6b2a_data_laws_knob_four.patch, b6b2a_endured_window_inclusive.patch, b6b2a_laws_counted_before_upkeep.patch, b6b2c_delay_second_rung.patch): find why each no longer applies in INT, re-cut it so it makes the same logical change against INT's text and apply --check passes; do not change what it mutates. Then check every patch under Tools/mutations/ with git apply --check in INT and list any other that fails (do not fix older-block patches the project already accepts as stale - the README marks those 'no longer applies since ...'; list only patches that applied at 220129d and fail now).
3. Nothing is built: the patches are proven to apply, not yet to be killed - say so in a README note line for B7x.
Return files_changed, done (patch name -> target test), not_done, compile_risks (empty), python_checks, notes_for_others.`, { label: 'impl:mutations', phase: 'Implement', schema: STEP_SCHEMA, effort: 'high' })

phase('Integrate')
const integ = await agent(`${CONTEXT}

YOUR TASK (integrator, effort high). Reports of this stage's implementers (uncommitted work in INT):
DOCS+RENAME: ${JSON.stringify(first)}
CODE: ${JSON.stringify(resCode)}
TESTS: ${JSON.stringify(resTests)}
SCHEMA: ${JSON.stringify(resSchema)}
DOCS: ${JSON.stringify(resDocs)}
MUTATIONS: ${JSON.stringify(mutations)}
1. Check that each agent stayed in its files (git -C "${INT}" -c core.quotepath=false diff --numstat, ignoring 0/0 lines) and that nothing outside Source/, Config/, Tools/, docs/ changed.
2. Regenerate docs/30 with python -I -X utf8 Tools/make_config_reference.py if --check says stale (comments of knobs may have changed).
3. In the B7x notes in INT (${N}), section 5.4: after each hand-off item this stage did, add a short mark '(통합 브랜치에서 했다 — 빌드 전, 2026-11-10)'; for 시12 add the two extra re-spun patches the re-stack found (b6b5_schema_desolation_without_lamp, b6b6_data_extinction_expiry_effect - already in commit 40cc67b) and any other the mutation agent found. Keep the file LF.
4. Run the five light Python checks and the LF check; fix trivial problems yourself (a stray CRLF, a missing README row) and report anything bigger.
5. Commit everything as one commit with OUT/msg_b7x12_handoffs.txt: title '[B-7x1+2] 리뷰 넘김 — 남긴 법의 좁힘·위기 몫의 거름·기생 덤 한 번·외교 항·IsBrick·주석, 시험 보드와 새 단언, K4 업적 규칙·Inward 글 규칙, docs/20·21·40, 변이 패치(통합 브랜치, 빌드 전)'; body '- ' bullets in plain Korean listing what was done per hand-off and what is not done yet (시3 after [B-6b8b]; build, tests, mutation kill runs after B6b or in a build-free B6b window); last line exactly: Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
Return commits, files_changed, done, not_done, compile_risks (merged list from all agents, deduplicated), python_checks, notes_for_others.`, { label: 'integrate', phase: 'Integrate', schema: STEP_SCHEMA, effort: 'high' })
log(`integrate: ${integ ? integ.commits.join(' | ') : 'FAILED'}`)

const LENSES = [
  { key: 'compile', effort: 'xhigh', prompt: 'LENS COMPILE: read every C++ line changed on integrate/b7x since 220129d (git -C INT diff 220129d -- Source/) as a strict MSVC compiler with UE 5.8 would: every identifier, include, overload, const-correctness, shadowing (C4456/C4457/C4458 are errors), unused variables, lambdas, TestEqual/TestTrue overloads, macros, UPROPERTY/USTRUCT rules (UHT: .generated.h last include, no braces issues), unity-build name clashes. Open the declarations you need (INT headers and C:/Program Files/Epic Games/UE_5.8/Engine/Source). Report only real compile errors or warnings-as-errors with the exact fix.' },
  { key: 'behaviour', effort: 'xhigh', prompt: 'LENS BEHAVIOUR AND DETERMINISM: check the B7x code changes against the project rules: the KeptLaw narrowing (decision 18) keeps K9 (no claim newly released or delayed) and the law slots; FoldedSmall spares claim/extinction/precursor; ParasiteBonusBoundOf once per hazard; the diplomacy term unclamped is clamped by the drift stage; no new draw from any stream (three random sources rule); peoples without bricks follow the old path bit for bit (old lines identical - name each place you checked); saved fields TimeVectors/HostCivID defaults and hashing (ComputeStateHash/ComputeWorldHash cover them as docs say); no layer violation (Sim/Core/Data never include UI/World); rule 13 single windows (the settle gate reason Inward goes through CosmicMegaprojects::SettleBlockOf).' },
  { key: 'tests', effort: 'xhigh', prompt: 'LENS TESTS AND MUTATIONS: for each new or changed assertion, check it tests what the hand-off says and that the matching patch under Tools/mutations/B7x/ would really make it fail (trace the code path); check the boards no longer inherit game values that B7x data or B6b data could move (WithInertBricks/StripBricks used where the game rules are read); check expectations are right numerically (e.g. Before - 0.4f, the three galaxy laws, the share equality); check the re-spun main patches still mutate the same logic; list missing assertions for any code change in this stage that no test would catch.' },
  { key: 'docs', effort: 'high', prompt: 'LENS DOCS: check docs/20, docs/21, docs/40, docs/30 and the B7x notes changes on integrate/b7x against the code in INT (the code wins): field names, defaults, step order, function names and signatures, test names, mutation folder; the [B-7x0] commit (systems.md reversal marker, B2 header marker, SECTION_MAP lines) against header section 7 and notes 5.4 at 79f2583; plain Korean, no coined shorthand, rule 11 words; nothing claimed as built or tested.' },
]

phase('Review')
const reviewed = await pipeline(
  LENSES,
  (lens) => agent(`${CONTEXT}

You are a reviewer (read-only - do not edit). integrate/b7x now holds: 40cc67b (re-stack), the [B-7x0] docs commit, the renames commit and the hand-offs commit (see git -C "${INT}" log --oneline 220129d..HEAD). Integrator report: ${JSON.stringify(integ)}

${lens.prompt.replace(/INT/g, INT)}

Report only real defects with evidence (file:line, quoted text). Severity: high = will not compile or breaks a project rule/determinism; medium = wrong behaviour or a test that cannot catch its mutation; low = comment/doc wording. Each finding needs an exact fix.`, { label: `review:${lens.key}`, phase: 'Review', schema: FINDINGS_SCHEMA, effort: lens.effort }),
  (res, lens) => {
    const findings = (res && res.findings) || []
    log(`lens ${lens.key}: ${findings.length} findings`)
    return parallel(findings.map((f) => () =>
      agent(`${CONTEXT}

You are a skeptical verifier. Try to REFUTE this review finding about integrate/b7x in INT (read the actual files; for compile claims open the real declarations). If wrong: refuted. If right but the fix is wrong or incomplete: partial with corrected_fix. If right: confirmed (sharpen the fix).
Finding (${lens.key}): ${JSON.stringify(f)}`, { label: `verify:${lens.key}:${f.id}`, phase: 'Verify', schema: VERDICT_SCHEMA, effort: 'high' })
        .then((v) => ({ lens: lens.key, finding: f, verdict: v }))))
  },
)

const all = reviewed.filter(Boolean).flat().filter(Boolean)
const kept = all.filter((x) => x.verdict && x.verdict.verdict !== 'refuted')
const refuted = all.filter((x) => x.verdict && x.verdict.verdict === 'refuted')
log(`review: ${all.length} findings, kept ${kept.length}, refuted ${refuted.length}`)

let fix = null
if (kept.length > 0) {
  phase('Fix')
  fix = await agent(`${CONTEXT}

YOUR TASK (fixer, effort high): apply every kept finding below to integrate/b7x in INT (use corrected_fix when partial; if two conflict, follow the notes' hand-off and say so). If a fix changes a line a mutation patch under Tools/mutations/ targets, re-cut that patch so git apply --check still passes. Re-run the five light Python checks and the LF check. Commit the fixes as one new commit with title '[B-7x1+2] 리뷰 고침(통합 브랜치, 빌드 전)' and '- ' bullets in plain Korean, last line exactly: Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
KEPT: ${JSON.stringify(kept.map((x) => ({ lens: x.lens, id: x.finding.id, severity: x.finding.severity, file: x.finding.file, location: x.finding.location, claim: x.finding.claim, fix: x.finding.fix, verdict: x.verdict.verdict, corrected_fix: x.verdict.corrected_fix })), null, 1)}
REFUTED (do not apply): ${JSON.stringify(refuted.map((x) => ({ id: x.finding.id, claim: x.finding.claim, reason: x.verdict.reason })))}`, { label: 'fix:review', phase: 'Fix', schema: STEP_SCHEMA, effort: 'high' })
}

return {
  first, code: resCode, tests: resTests, schema: resSchema, docs: resDocs, mutations, integ,
  findings_total: all.length,
  kept: kept.map((x) => ({ lens: x.lens, id: x.finding.id, severity: x.finding.severity, claim: x.finding.claim, verdict: x.verdict.verdict })),
  refuted: refuted.map((x) => ({ lens: x.lens, id: x.finding.id, claim: x.finding.claim, reason: x.verdict.reason })),
  fix,
}
