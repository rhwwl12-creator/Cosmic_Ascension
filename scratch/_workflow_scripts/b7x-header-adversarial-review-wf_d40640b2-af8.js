export const meta = {
  name: 'b7x-header-adversarial-review',
  description: 'Read-only ultracode adversarial review of the B7x header draft and its pre-build branch (design/b7x-composable-existence) against main and the remaining B6b plan, with a refuter per finding and a synthesized fix and rebase plan',
  phases: [
    { title: 'Review', detail: 'six lenses: design and rules, determinism/save/hash, theology and narrative, measurement and balance, static build and code, integration with B6b' },
    { title: 'Verify', detail: 'refuters, up to six findings each' },
    { title: 'Synthesize', detail: 'report: kept findings as exact header/code edits, which depend on developer answers, the rebase plan' },
  ],
}

const SCR = 'C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad'
const OUT = `${SCR}/b7x`
const BR = 'origin/design/b7x-composable-existence'
const HDR = 'docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md'
const NOTES = 'docs/60_DECISIONS/headers/B7_composable_existence_and_evolution_notes.md'
const DESIGN = 'docs/10_DESIGN/EXISTENCE_EVOLUTION_AND_ECOLOGY.md'

const READONLY = `
STRICT READ-ONLY. The working tree is main and another workflow is implementing, building and testing B6b commits in it right now.
Never edit, create or delete a project file; never run git checkout/switch/stash/reset/apply/add/commit/worktree, Build.bat,
UnrealEditor, Tools/run_checks.ps1, make_data_tables or anything that writes. You may write only under ${OUT}/ (your own files).
WHAT IS UNDER REVIEW: the branch ${BR} (tip 0bffb8e), made on 2026-11-09 on a PC WITHOUT the engine on top of main cef8cc7 ([B-6b3b]):
the B7x header draft ${HDR}, its notes ${NOTES}, the design document ${DESIGN}, and pre-build code/data commits [B-7x1] (fields),
[B-7x2]+[B-7x2r]+[B-7x2c] (mechanism, never compiled), [B-7x5a] (data draft, not to be turned on as is). Read branch files with
git show ${BR}:<path>, its changes with git diff cef8cc7 ${BR} -- <path> and git log -p cef8cc7..${BR}. The hand-off text the other PC
wrote is ${OUT}/branch/promptforother.txt (copies of header/notes/design are in ${OUT}/branch/ too).
CURRENT MAIN: origin/main = 529cab9 ([B-6b4], committed); [B-6b5] (text audit, bit-identical) is being finished in the tree (not
committed - do not read the working tree). The remaining B6b plan: B6b header docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md
section 8 (git show origin/main:...) and the reviewed implementation briefs ${SCR}/b6b5/brief_final.md, ${SCR}/b6b6/brief_final.md,
${SCR}/b6b7/brief_final.md, ${SCR}/b6b8/brief_final.md, ${SCR}/b6b8b/brief_final.md ([B-6b9]..[B-6b12] are not designed yet; see the
B6b header). B7x will be rebased onto main only after B6b ends.
Engine: UE 5.8.3 at C:/Program Files/Epic Games/UE_5.8 (read-only) - verify any engine API claim in its headers (docs/45 lists verified
paths). Project rules: CLAUDE.md (loaded for you), docs/20_ARCHITECTURE.md (git show origin/main:).
The developer's standing words for this block (2026-11-09): "불변 규칙과 부딪히는 자리는 좀 유연하게 변경시켜가면서 진행" - the header
receives conflicts by narrow rule changes R1-R10 and keeps K1-K10. Judge the header by whether each R is narrow, stated and tested,
and whether K1-K10 really hold in the code.
Theology guardrails (standing): the holy order (God) is never an entity, speaker, number, lever or ending condition; first person only
for fallen transcendents; no soul-counting; no saviour/messiah titles, no 'became god'; only 개역한글 1961 and WEB may be quoted and
no new verse is added; never open anything under C:/Users/rhwwl/Downloads/jehoba. docs/90_LORE/cosmic_species_codex.md is read-only.
Cite file:line (branch or main, say which) for every claim; quote Korean as it is. Report only real defects or real risks with
evidence; each finding gets an exact fix (header/notes/design text, or code/data change) and says whether it needs a developer
decision (the header's [개발자] decisions are 1,2,3,4,6,8,11,13,14,15,16,18,20,22).`

const FIND = {
  type: 'object',
  properties: {
    findings: { type: 'array', items: { type: 'object', properties: {
      id: { type: 'string' }, severity: { type: 'string', enum: ['high', 'medium', 'low'] },
      where: { type: 'string' }, claim: { type: 'string' }, evidence: { type: 'string' }, fix: { type: 'string' },
      needs_developer: { type: 'string', description: 'empty, or which [개발자] decision it touches and why' },
    }, required: ['id', 'severity', 'where', 'claim', 'evidence', 'fix', 'needs_developer'] } },
    summary: { type: 'string' },
  },
  required: ['findings', 'summary'],
}

const LENSES = [
  { key: 'design', prompt: `DESIGN AND RULES. Check the header (all sections) and notes against CLAUDE.md rules 1-15 and docs/20's invariant
text: is every rule the branch bends listed as an R (R1-R10) with the exact sentence to change, narrow and tested? Do K1-K10 hold in the
branch code (e.g. K1 the world never reads knowing/log/chronicle/pending cards; K3 draw counts fixed before rows/bricks; K4 rungs only
by feat rows; K8 no absolute immunity; K9 Hazard.Claim never newly freed or delayed; K10 identity never crosses bodies)? Single windows
(rule 13: settle block, hazard placement, edicts) - are new reasons/steps routed through the existing windows? Is the decision list
complete and are the recommendations consistent with the header body, the notes' disposition table (60 bricks) and the design doc
sections 3.3/4.3/4.4/5? Are slices B/C (나/다) specified enough to be designed later? Anything in the design doc the header claims to
receive but the code does not do, or the reverse.` },
  { key: 'determinism', prompt: `DETERMINISM, SAVE AND HASH. Rule 3 (three RNG sources; draw counts fixed before looking at rows/bricks), the
new derived-integer purpose (CosmicSeed purpose 13 'Brick' - frozen formula, appended at the end), the tick order of the new steps (★
cumulative counters at the end of the economy step; mutation step at the end of the AI step before feat judging) against docs/20's
tick order; save rules (rule 7: fields only added, frozen defaults, SaveVersion 1, enumerators at the end, row names kept); hash rules
(rule 6: ComputeStateHash/ComputeWorldHash, FText excluded, TMap ordering) - do [B-7x1]/[B-7x2] really move only the two hashes while
every deterministic CSV column stays equal for tag-less peoples (read the code paths the notes section 5.1 lists, letter by letter)?
Any float operation outside rule 6b's allowed list (check_layers rule 3b)? Any place where a knob at its no-op value still changes
behaviour?` },
  { key: 'theology', prompt: `THEOLOGY AND NARRATIVE (rule 11, rule 12, the name sieve systems.md:383-385, docs/70). Read the header,
the design doc's new sections 3.3/4.3/4.4/5 and the [B-7x5a] data draft texts (DT_Encounters/DT_Events rows, ST_CA keys, Shape words,
brick tag names and DevComments). Find: the holy order as agent/number/lever/end; anyone 'becoming' law/covenant/hedge/god; titles or
glory for persons; first person outside the fallen; soul-counting; verdict words in narrator lines; numbers of people; invented names
that are other works'/myths'/Bible proper names/real persons (decision 3's six leaf names: UnseenLaw, SilenceFeed, PlanckMicro,
DysonBrain, KesslerTapestry, AbyssalMaw - and any other); church idiom; the parasite/eater/hive framing (no celebration of
assimilation - docs on the 동화 doctrine as a counterfeit); and whether the 'not received' five (decision 18) are received safely. The
full text audit is a later commit ([B-7x4]) - report here what blocks approval or changes a decision.` },
  { key: 'measurement', prompt: `MEASUREMENT AND BALANCE. Header section 5 (commandlet, the skip in the measuring policy, the off-pair
switch -WithoutB7xData, the new 'bricks:' line, 'what to measure before values', judgments 0-5), section 3.4 (seventeen knobs at no-op
C++ values, start values in [B-7x5a]), section 8's checks per commit, section 9 hand checks. Can every judgment fail? Are thresholds
written before measuring? Does the plan reuse the B6b harness (${SCR}/b6b6/bundleA.py, ${SCR}/b6b8/bundleB.py) and its pair check? Do
the B7x knobs or seeds interact with B6b's measured quantities (race, fourth/fifth rung, extinction, laws held, edict revocation by AI -
note Brick.Form.UnseenLaw changes CosmicEdicts::AIRevokesOf which B6b's AI law step also uses)? Is the seed brick (decision 23) going
to shift B6b's tuned values when B7x data turns on? What must be re-measured after B7x lands?` },
  { key: 'build', prompt: `STATIC BUILD AND CODE REVIEW of the never-compiled code (git diff cef8cc7 ${BR} -- Source Config Tools). The
module rules: CLAUDE.md 'Module / UHT / compile rules' (unique header names, .generated.h last, include from module root, no transitive
includes - every used header included directly, BuildSettingsVersion V7 + IncludeOrder Unreal5_8: C++20, UE_LOG format checks,
undefined identifier/return type/dangling/unreachable warnings are ERRORS), UPROPERTY rules, USTRUCT/UENUM reflection, GameplayTag
declaration and RequestGameplayTag, check_layers rules (layer includes, rule 3b FMath/CRT list - read Tools/check_layers.py on main),
ASCII-only sources. Notes section 5.2 lists likely problems (shadowed names, TestEqual overloads, test-board assumptions) - verify each
in the engine source (e.g. Runtime/Core/Public/Misc/AutomationTest.h for TestEqual overloads) and find more: every function called
exists with that signature on cef8cc7, every enum value exists, every new header compiles alone, every test helper exists. Give each
likely compile or test error with the exact fix.` },
  { key: 'integration', prompt: `INTEGRATION WITH B6b (the rebase plan). The branch is based on cef8cc7; main is already at 529cab9
([B-6b4]) and B6b will add [B-6b5]..[B-6b12] (read the briefs and the B6b header section 8). List every textual conflict the rebase will
meet (git diff cef8cc7 origin/main vs git diff cef8cc7 ${BR} on the same files/hunks, plus what the briefs say the remaining B6b
commits will change: FCosmicRulesDef's end (knobs), EConditionType's end, ECosmicSettleBlock's end, CosmicSeed purposes,
Config/DefaultGameplayTags.ini, Tools/data/schema.json and Tools/test_data_schema.py's pinned counts, ST_CA.csv's end, DT_Rules.json,
DT_Encounters/DT_Events ends, docs/30, Tests (CosmicDataTests, CosmicSimTests, CosmicEventTests, CosmicPanelTests,
CosmicBalanceCommandlet), Sim/GalaxySimulationSubsystem tick steps, CosmicEdicts::AIRevokesOf, CosmicMegaprojects settle block, the
commandlet) and every SEMANTIC interaction (e.g. B6b's new AI steps and step order vs B7x's mutation step; B6b's galaxy laws and AI
law step vs Brick.Form.UnseenLaw; B6b's extinction/ark/binding vs the brick shares on hazards; enumerator values assigned on both sides
- the later one must move to the end; the B7x purpose number vs any B6b purpose; the five baselines both move). For each: the
resolution (who goes first - main first), and the order of the rebased commits with what each must verify. Also: what in the branch is
stale against main (line numbers, facts such as test counts 223, 'B6b header took #164-#166', file names).` },
]

phase('Review')
const reviews = await pipeline(
  LENSES,
  l => agent(`You are review lens '${l.key}' of the ultracode adversarial review of the B7x header draft and its branch.${READONLY}

YOUR LENS: ${l.prompt}
Also save your findings as ${OUT}/review_${l.key}.json.`, { label: `lens:${l.key}`, phase: 'Review', schema: FIND, effort: 'xhigh' }),
  async (res, l) => {
    const fs = (res && res.findings) || []
    if (!fs.length) return { lens: l.key, summary: res ? res.summary : 'no result', findings: [] }
    const batches = []
    for (let i = 0; i < fs.length; i += 6) batches.push(fs.slice(i, i + 6))
    const vs = await parallel(batches.map((b, i) => () => agent(`You adversarially verify review findings on the B7x header draft and branch.${READONLY}

For EACH finding below (lens ${l.key}): try to refute it. Re-read every cited line yourself (branch via git show ${BR}:<path>, main via
git show origin/main:<path>, engine headers). Is it real on the branch as written? Is the rule applied correctly (the developer allows
narrow, stated rule changes - a bent rule that is listed as an R with its sentence is not a defect by itself)? Is the fix right and does
it break something else? Answer confirmed / partial (with the corrected fix) / refuted (with evidence) for every id.

FINDINGS:
${JSON.stringify(b, null, 1)}`, {
      label: `verify:${l.key}#${i + 1}`, phase: 'Verify', effort: 'high',
      schema: { type: 'object', properties: { verdicts: { type: 'array', items: { type: 'object', properties: {
        id: { type: 'string' }, verdict: { type: 'string', enum: ['confirmed', 'partial', 'refuted'] },
        corrected_fix: { type: 'string' }, reason: { type: 'string' } }, required: ['id', 'verdict', 'corrected_fix', 'reason'] } } },
        required: ['verdicts'] } })))
    const verdicts = vs.filter(Boolean).flatMap(v => v.verdicts)
    return { lens: l.key, summary: res.summary, findings: fs.map(f => ({ ...f, verdict: verdicts.find(v => v.id === f.id) || null })) }
  },
)
const all = reviews.filter(Boolean)
const flat = all.flatMap(r => r.findings)
const cnt = k => flat.filter(f => f.verdict && f.verdict.verdict === k).length
log(`findings ${flat.length}: confirmed ${cnt('confirmed')}, partial ${cnt('partial')}, refuted ${cnt('refuted')}, unverified ${flat.filter(f => !f.verdict).length}`)

phase('Synthesize')
const report = await agent(`You synthesize the ultracode adversarial review of the B7x header draft and its branch.${READONLY}

Write ${OUT}/review_report.md (Korean headings and prose for the developer-facing parts; quote code/paths as they are) with:
1. 요약: counts (findings, confirmed, partial, refuted) per lens; the verdict on approval readiness.
2. 개발자에게 물을 것: for each [개발자] decision (1,2,3,4,6,8,11,13,14,15,16,18,20,22) the header's recommendation, anything the review
   found that changes or sharpens it (with finding ids), and the consequence the developer must see (e.g. decision 18 and the left law
   that keeps a stability-draining edict under a regency).
3. 고칠 것 (kept findings - confirmed and partial): grouped (header/notes/design text; code; data; tests), each with the exact fix (use
   the verifier's corrected fix for partial ones), which commit it belongs to ([B-7x0] approval edits, or [B-7x1]/[B-7x2] at rebase,
   [B-7x4], [B-7x5]...), and whether it waits for a developer answer.
4. 리베이스 계획: the order of the rebased commits on top of the finished B6b, every expected conflict and its resolution (main first),
   every semantic interaction with B6b and what to re-verify, and what each commit must prove (hashes move / CSV deterministic columns
   equal / world moves).
5. 반박된 것: refuted findings with the reason, one line each.
Then return a short Korean summary (counts, top five issues, the decisions most affected).

ALL FINDINGS WITH VERDICTS:
${JSON.stringify(all, null, 1)}`, { label: 'synthesize', phase: 'Synthesize', effort: 'xhigh' })

return { counts: { findings: flat.length, confirmed: cnt('confirmed'), partial: cnt('partial'), refuted: cnt('refuted') }, report, lenses: all.map(r => ({ lens: r.lens, n: r.findings.length, summary: r.summary })) }
