export const meta = {
  name: 'b7x-stage3-draft',
  description: 'B7x stage 3 drafted in a second integration worktree (no build): [B-7x4m] measurement switches, [B-7x3] presentation with the audited keys and scan face, then [B-7x4] text rules and DevComments; integrator commits three commits; adversarial read-review and fix',
  phases: [
    { title: 'Implement', detail: '[B-7x4m] and [B-7x3] in parallel on disjoint files, then [B-7x4]' },
    { title: 'Integrate', detail: 'checks and three commits' },
    { title: 'Review', detail: 'compile, behaviour, tests, texts lenses' },
    { title: 'Verify', detail: 'one refuter per medium/high finding' },
    { title: 'Fix', detail: 'apply surviving findings' },
  ],
}

const SCR = 'C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad'
const W3 = SCR + '/b7x/int3'
const OUT = SCR + '/b7x/stage3'
const LF = SCR + '/b7x/tools/lf_norm.py'
const MAIN = 'C:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension'
const H = 'docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md'
const N = 'docs/60_DECISIONS/headers/B7_composable_existence_and_evolution_notes.md'
const D = 'docs/10_DESIGN/EXISTENCE_EVOLUTION_AND_ECOLOGY.md'
const TEXTS = OUT + '/texts/audited_texts.md'
const RULINGS = OUT + '/texts/lead_rulings.md'

const CONTEXT = `
Project: Cosmic Ascension, UE 5.8.3 C++ game module Cosmic_Ascension (Windows, solo developer, Korean docs, ASCII-only source with English comments).
Block B6b is being built and measured in the MAIN checkout (${MAIN}) by another workflow, and a third workflow is compiling the integration worktree scratchpad/b7x/int right now. YOU work ONLY in a second worktree W3 = ${W3} (local branch integrate/b7x-s3, starting at 1333de4 = the B7x integration branch before its first compile). Never touch MAIN or scratchpad/b7x/int (read-only reading is fine). Scratch folder OUT = ${OUT}.
Nothing is built here (no Binaries in W3; the build machine is busy). This code will be compiled later, after these commits are cherry-picked onto integrate/b7x. So write C++ that compiles against UE 5.8 and W3's real declarations: open every header you call into and match exact signatures (engine headers under C:/Program Files/Epic Games/UE_5.8/Engine/Source); warnings are errors (shadowing C4456/C4457/C4458, unused variables, unreachable code).

B7x documents in W3 (the code wins over the docs where they disagree; ask nothing, follow them): header ${H} (section 4 presentation incl. decision 13 rules (a)-(d); section 5 commandlet and measurement - written before data, binding; section 6 tests; section 8 commit order and what each commit proves; section 10 decisions, all answered 2026-11-10 '권장대로'), notes ${N} (section 5.4 hand-offs per commit: '[B-7x4m]' 코8, '[B-7x3]' 시9 and 데3 Shape., '[B-7x4]' 데2/데3/F11), accepted design doc ${D}. Audited texts: ${TEXTS}; the lead's rulings ${RULINGS} WIN over the audited texts where they differ (voice tilts kept per main convention; C10 title '손대지 않은 칙령'; C12 title '집을 싣는 배'; C12 choice 2 keeps Reverence +0.05).

RULES
- Bit-identical with B7x data off: in W3 no B7x data row exists yet (the twelve crossroads cards, nine event_brick rows, Society on arch_seeker, knob values and BrickSeedTags arrive only after B6b ends - do NOT add them, do NOT port branch commit aa09543). Every new path you add must leave the world, both hashes and every default commandlet line unchanged while those rows are absent and the knobs are inert. Measurement switches change nothing unless given.
- Layers (CLAUDE.md rules 1, 13, 14): Sim/Core/Data never include UI/World; presentation reads knowing only through CosmicMapLook::KnownViewOf; owner-revealing words only when the owner is shown (OwnerShownOf / IsKnown).
- Line endings: LF. Pass -c core.eol=lf to git commands that write files in W3 and run python -I -X utf8 ${LF} "${W3}" afterwards (tree first, --check second).
- Light Python checks only, from W3's root: python -I -X utf8 Tools/check_data.py; Tools/test_data_schema.py; Tools/check_layers.py --strict; Tools/make_config_reference.py --check; Tools/check_docs.py. A stale generated.sha1 is expected (no editor here).
- Korean only in Tools/data, ST_CA.csv and docs; rule 11 for every Korean line (no verdict words in narrator text, no counts of people, plain native words; the holy order never an entity/speaker/lever).
- The commandlet header's class comment is one UHT string literal (MSVC limit 16,380 bytes) - keep new comments there short; details go to docs/40.
- Do not commit (the integrator commits).
`

const STEP = {
  type: 'object',
  properties: {
    files_changed: { type: 'array', items: { type: 'string' } },
    done: { type: 'array', items: { type: 'string' } },
    not_done: { type: 'array', items: { type: 'string' } },
    compile_risks: { type: 'array', items: { type: 'string' } },
    mutations: { type: 'array', items: { type: 'string' } },
    notes_for_others: { type: 'array', items: { type: 'string' } },
  },
  required: ['files_changed', 'done', 'not_done', 'compile_risks', 'mutations', 'notes_for_others'],
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

phase('Implement')
const measureTask = () => agent(`${CONTEXT}

YOUR TASK ([B-7x4m] measurement, effort xhigh). You own ONLY: Source/Cosmic_Ascension/Tests/CosmicBalanceCommandlet.cpp/.h, Source/Cosmic_Ascension/World/CosmicRulerPolicy.cpp/.h, Source/Cosmic_Ascension/World/CosmicWandererPolicy.cpp/.h, Source/Cosmic_Ascension/Tests/CosmicTestHelpers.h (the WithoutB7xData helper only - other agents do not touch this file now), the Data.Integrity tripwire part of Source/Cosmic_Ascension/Tests/CosmicDataTests.cpp, docs/40_DEV_GUIDE.md (commandlet/measurement sections and test table rows for your tests), and new patches under Tools/mutations/B7x/ with README rows. Another agent is doing [B-7x3] presentation (UI widgets, PlayerRoleSubsystem, ST_CA.csv, schema.json, DT_LogTemplates) at the same time - do not touch those.
Implement header section 5 exactly as written ('정책의 건너뜀' for the ruler and wanderer policies - on crossroads cards pick only the no-value choice ③; '-WithoutB7xData=1' and '=2' and the test helper CosmicTests::WithoutB7xData; '-Crossroads=<k>' (the policy picks brick choices k and lets C10 choice 1 through DeposeRuler); the new commandlet line 'bricks:' with the counts the header lists; the -SaveAt refusal list; the Data.Integrity tripwire that pins exactly the set the helper strips) and notes 5.4 '[B-7x4m]' 코8. Mirror how B6b did the same things (search the commandlet and helpers for WithoutB6bData, 'race:' line, -FourthRungCap, the B6b tripwire in Data.Integrity) so the code matches existing patterns. Rows that do not exist yet: the helper and the policy must find B7x rows by their conditions/names the way the header says, and do nothing when none exist (bit-identical default runs).
Write the two mutation patches the hand-off names (skip removed - proven with a bench card built inside the test; off switch leaving seeds/knobs) as unified diffs against W3's files after your edits (git -C "${W3}" apply --check must pass; apply and reverse once, then the LF normalizer), plus one patch per other new assertion you add; README rows in the folder's style. Nothing is built: patches are proven to apply, not yet to fail.
Return files_changed, done, not_done, compile_risks (every identifier you could not verify), mutations (patch -> test that should fail), notes_for_others.`, { label: 'impl:B-7x4m', phase: 'Implement', schema: STEP, effort: 'xhigh' })

const presentTask = () => agent(`${CONTEXT}

YOUR TASK ([B-7x3] presentation, effort xhigh). You own ONLY: Source/Cosmic_Ascension/UI/** (chronicle widget, system panel, edict/diplomacy widget), Source/Cosmic_Ascension/Player/PlayerRoleSubsystem.cpp/.h (the scan line choice only), Source/Cosmic_Ascension/Data/CosmicRowNames.h (one constant), Source/Cosmic_Ascension/World/CosmicMapLook* only if the knowing view truly needs a new answer, UI/chronicle/panel/scan tests under Source/Cosmic_Ascension/Tests/ (not CosmicTestHelpers.h, not CosmicBalanceCommandlet.cpp, not CosmicDataTests.cpp - another agent owns those now), Tools/data/ST_CA.csv, Tools/data/DT_LogTemplates.json (the one row log_brick_scan_spread), Tools/data/schema.json and Tools/test_data_schema.py (required_rows for that row, the Shape. checked prefix and the shape_merit_words list), and new patches under Tools/mutations/B7x/ (+ README rows - the [B-7x4m] agent also adds rows there; add yours at the end of the B7x table and keep its rows).
Implement header section 4 and notes 5.4 '[B-7x3]' (시9, 데3 Shape.):
- The chronicle life-chapter fact line gets the shape word under decision 13 ㉢: (a) only the life's last ruled people (Life.RuledCivIDs last), none for a life that never ruled; (b) only if that people is still in State.Civs; (c) when several lives ruled the same people, only the last such life's chapter; (d) the word reads as that people's present way of life ('살림') and may change if reopened after the end. Show nothing for a people whose counts are all zero and that has no shape tag (ShapeOf answers BuiltOrLinked then - ask first). The promise 'only the reason line differs between endings' stays.
- The system panel's people line repeats the same word, only when the owner is shown (ECosmicOwnerShown::Shown).
- The scan face: log_brick_scan_spread (DT_LogTemplates, three variants from the audited texts section 4.1, Category Log.Discovery like log_scan_wild), chosen in UPlayerRoleSubsystem::ScanCurrentSystem instead of log_scan_wild under the S15 condition the audited texts describe; CosmicRows constant + schema required_rows. The line names only {System} (the people's name has not passed the knowing gate).
- The diplomacy window tooltip shows the drift target's new brick term (find how stage 2 added the Diplomacy key and the TargetRelationOf brick term; with inert knobs the term is 0 and the tooltip must read as before).
- ST_CA.csv: the twelve Shape.* keys and the Settle.Block.Inward text exactly as the audited texts section 3 gives (new keys only at the end; keep main's keys untouched; the Inward key may already exist - then update only its text if the audit changed it).
- schema: string_table.checked_prefixes 'Shape.' with the lists the audited texts section 7.2 gives, words.shape_merit_words ['견디','남은'], and test_data_schema.py cases (one break per list + a pin of the Shape. forbidden list, like the Chronicle. pin); recount the walked pin by running the test.
- Tests: UI.Chronicle cases for (a), (b), (c), the life that ruled A then B, and the earlier-life case (notes 시9 lists five) + one mutation (call ShapeOf on a removed people / drop the Civs check); a panel assertion for the word under OwnerShown; a scan-face case.
Every new path must be bit-identical while no people carries a brick (data off). Return files_changed, done, not_done, compile_risks, mutations, notes_for_others (exact facts the [B-7x4] text agent and the integrator need).`, { label: 'impl:B-7x3', phase: 'Implement', schema: STEP, effort: 'xhigh' })

const firstWave = await parallel([measureTask, presentTask])
const [resM, resP] = firstWave
log(`implement wave 1: B-7x4m ${resM ? 'ok' : 'FAILED'}, B-7x3 ${resP ? 'ok' : 'FAILED'}`)

const textTask = await agent(`${CONTEXT}

YOUR TASK ([B-7x4] texts, effort high). The [B-7x4m] and [B-7x3] agents have finished (uncommitted in W3). Their reports: M ${JSON.stringify(resM)} P ${JSON.stringify(resP)}.
You own: Tools/data/schema.json and Tools/test_data_schema.py (the crossroads text rules only - P added the Shape. parts; keep them), Config/DefaultGameplayTags.ini (the seventeen Brick tag DevComments only), and the B7x documents ${H}, ${N}, ${D}.
Apply notes 5.4 '[B-7x4]' and the audited texts + lead rulings:
- words.crossroads_words ['침묵','울타리','언약','방주','목자','순례','성소','불멸','하나가 되'] and the condition-keyed rules of the audited texts section 7.3 that can stand before the rows exist (DT_Encounters: rows with CivTimeVectorAtLeast; DT_Events: rows with CivTimeVectorAtLeast or BorderingCivHasTrait) - the three row-name-keyed rules (event_brick_shut_star, _drift_plenty, _ember_wake) and the DT_LogTemplates row rule wait for the commit that adds those rows (the checker rejects a rule naming a missing row) EXCEPT log_brick_scan_spread, which [B-7x3] added - add its row rule now. Test cases in test_data_schema.py (one failing case per rule/list + pins), recount the walked pin by running the test.
- The seventeen DevComments from the audited texts section 5 (ASCII; the KeptLaw one follows the narrowed code).
- Documents: header section 3.2 (the six shape words the audit changed - final values from the audited texts section 3), section 2.4 line about the single-system premise (audited texts 6.3, F6), the C10 title change ('손대지 않은 칙령' - why: '거두' is in soul_verbs) recorded in notes 5.4 데2 and design doc 4.3 C10, the C12 title '집을 싣는 배', and the other doc lines audited texts section 6.3 lists EXCEPT its first item (the 'tilt only on closing results' sentence - rejected by the lead's rulings; keep the design doc's convention sentence) - align the design doc 4.3 choice-2 effect lines with the lead rulings' effect lists.
Return files_changed, done, not_done, compile_risks (empty unless an ini/C++ reader is affected), mutations (schema mutation names if you add any), notes_for_others.`, { label: 'impl:B-7x4', phase: 'Implement', schema: STEP, effort: 'high' })

phase('Integrate')
const integ = await agent(`${CONTEXT}

YOUR TASK (integrator, effort high). Reports: M ${JSON.stringify(resM)} P ${JSON.stringify(resP)} T ${JSON.stringify(textTask)}.
1. Check each agent stayed in its files (git -C "${W3}" -c core.quotepath=false diff --numstat, ignoring 0/0). Regenerate docs/30 with python -I -X utf8 Tools/make_config_reference.py if --check is stale. Run the five light Python checks and the LF check; fix trivial issues.
2. Make THREE commits on integrate/b7x-s3, staging explicit file lists (git add <paths>), in this order:
   a) '[B-7x4m] 측정 — 정책의 건너뜀·-WithoutB7xData·-Crossroads·bricks: 줄·-SaveAt 거절, 도우미와 걸림줄(통합 브랜치, 빌드 전)' - M's files;
   b) '[B-7x3] 표현 — 끝의 꼴 낱말(㉢)·패널의 백성 줄·스캔 얼굴·외교 툴팁의 블록 항, Shape 키와 낱말 규칙(통합 브랜치, 빌드 전)' - P's files;
   c) '[B-7x4] 글 — 갈림 낱말 규칙·DevComment 열일곱·헤더와 설계서의 글(통합 브랜치, 빌드 전)' - T's files.
   If two commits must share a file (e.g. Tools/mutations/README.md rows of a and b), put each commit's hunks in its own commit with git add -p is not available non-interactively - instead commit README rows with the later commit that needs them and say so. Each body: '- ' bullets in plain Korean (what, why, what is not done: build, tests, mutation kill runs come later), last line exactly: Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>.
3. Return files_changed, done (the three commit hashes and titles), not_done, compile_risks (merged and deduplicated from all agents), mutations (all new patches), notes_for_others.`, { label: 'integrate', phase: 'Integrate', schema: STEP, effort: 'high' })

const LENSES = [
  { key: 'compile', effort: 'xhigh', prompt: 'LENS COMPILE: read every C++ line changed on integrate/b7x-s3 since 1333de4 as a strict MSVC compiler for UE 5.8 would (identifiers, includes from the module root, overloads, const, shadowing, unused variables, lambdas, UHT rules incl. the commandlet class-comment limit, unity-build clashes). Open the real declarations in W3 and in the engine. Report only real compile errors/warnings-as-errors with exact fixes.' },
  { key: 'behaviour', effort: 'xhigh', prompt: 'LENS BEHAVIOUR: with B7x data absent, is every default run bit-identical (no new draw, no world/hash change, default commandlet lines unchanged; the scan line chooses log_scan_wild exactly as before; the chronicle and panel show nothing new)? Do the presentation rules follow decision 13 (a)-(d) and CLAUDE.md rule 14 (knowing only through KnownViewOf; owner word only when shown)? Does the measurement match header section 5 word for word (switch semantics, bricks: fields, -Crossroads behaviour incl. C10 choice 1 through DeposeRuler, the -SaveAt refusals)?' },
  { key: 'tests', effort: 'high', prompt: 'LENS TESTS AND MUTATIONS: do the new tests assert what the hand-offs ask (notes 5.4 시9, 코8, 데3) and would each new patch under Tools/mutations/B7x/ make its test fail (trace it)? Are the schema test cases real breaks, the walked pin recounted, the Shape. and crossroads pins present? Missing assertions for any new code path?' },
  { key: 'texts', effort: 'high', prompt: 'LENS TEXTS AND DOCS: are the ST_CA lines, the scan template, the DevComments and the doc edits exactly the audited texts with the lead rulings applied (C10/C12 titles, tilt convention kept, Shape words), rule 11 clean, no design names on screen, plain Korean; do docs/40 and the B7x header/notes/design doc match the code?' },
]

phase('Review')
const reviewed = await pipeline(
  LENSES,
  (lens) => agent(`${CONTEXT}

You are a reviewer (read-only - do not edit). integrate/b7x-s3 holds three new commits on 1333de4 (git -C "${W3}" log --oneline 1333de4..HEAD). Integrator report: ${JSON.stringify(integ)}

${lens.prompt}

Report only real defects with evidence (file:line, quoted text). Severity: high = will not compile, breaks bit-identity or a project rule; medium = wrong behaviour or a test that cannot catch its mutation; low = wording. Each finding needs an exact fix.`, { label: `review:${lens.key}`, phase: 'Review', schema: FINDINGS, effort: lens.effort }),
  (res, lens) => {
    const findings = (res && res.findings) || []
    log(`lens ${lens.key}: ${findings.length} findings`)
    return parallel(findings.map((f) => () => {
      if (f.severity === 'low') return Promise.resolve({ lens: lens.key, finding: f, verdict: { verdict: 'confirmed', reason: 'low - not verified; fixer judges', corrected_fix: '' } })
      return agent(`${CONTEXT}

You are a skeptical verifier. Try to REFUTE this finding about integrate/b7x-s3 in W3 (read the files; open real declarations for compile claims). If wrong: refuted. If right but the fix is wrong: partial with corrected_fix. If right: confirmed.
Finding (${lens.key}): ${JSON.stringify(f)}`, { label: `verify:${lens.key}:${f.id}`, phase: 'Verify', schema: VERDICT, effort: 'high' })
        .then((v) => ({ lens: lens.key, finding: f, verdict: v }))
    }))
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

YOUR TASK (fixer, effort high): apply every kept finding to W3 (corrected_fix when partial; skip with a reason when wrong). Re-cut any mutation patch whose target lines you change (git apply --check must pass). Re-run the five light Python checks and the LF check. Commit the fixes as one commit '[B-7x3·4m·4] 리뷰 고침(통합 브랜치, 빌드 전)' with '- ' bullets in plain Korean, last line exactly: Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
KEPT: ${JSON.stringify(kept.map((x) => ({ lens: x.lens, id: x.finding.id, severity: x.finding.severity, file: x.finding.file, location: x.finding.location, claim: x.finding.claim, fix: x.finding.fix, verdict: x.verdict.verdict, corrected_fix: x.verdict.corrected_fix })), null, 1)}
REFUTED (do not apply): ${JSON.stringify(refuted.map((x) => ({ id: x.finding.id, claim: x.finding.claim, reason: x.verdict.reason })))}`, { label: 'fix:review', phase: 'Fix', schema: STEP, effort: 'high' })
}

return { measure: resM, present: resP, texts: textTask, integ, findings_total: all.length, kept: kept.map((x) => ({ lens: x.lens, id: x.finding.id, severity: x.finding.severity, claim: x.finding.claim })), refuted: refuted.map((x) => ({ id: x.finding.id, claim: x.finding.claim })), fix }
