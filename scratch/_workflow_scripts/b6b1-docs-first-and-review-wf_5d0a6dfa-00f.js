export const meta = {
  name: 'b6b1-docs-first-and-review',
  description: 'B-6b1: edit the docs that must come before code (docs/20, docs/21, design docs, DECISIONS_LOG), then review code, docs and rule files, verify, fix docs',
  phases: [
    { title: 'Edit', detail: 'one editor per document group, each edits only its own files' },
    { title: 'Review', detail: 'three read-only lenses: saved state and hash code, docs-first audit, tests and rule files' },
    { title: 'Verify', detail: 'one skeptic tries to refute every finding' },
    { title: 'Fix', detail: 'one fixer applies confirmed doc findings; code findings go back to the lead' },
  ],
}

const ROOT = 'C:\\Users\\rhwwl\\OneDrive\\Documents\\Unreal Projects\\Cosmic_Ascension'
const HEADER = 'docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md'
const NOTES = 'docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race_notes.md'

const CONTEXT = `
You work in the Cosmic Ascension repository at ${ROOT} (UE 5.8 C++ game, docs in Korean). The B6b block header is APPROVED (commit b15fffb, 2026-11-08):
- header: ${HEADER} (read sections 0, 2.1-2.12, 3, 7, 8, 10)
- notes: ${NOTES} (section 1.2 "문서 커밋의 목록" lists, per commit, which document lines change; the part "**문서 먼저 — \`[B-6b1]\`**" is THIS commit; section 0.4 holds the determinism table and the same-tick order table that is part of header 2.11)
We are implementing commit [B-6b1] "필드와 문서 먼저". The code of this commit is already in the working tree (see \`git diff -- Source\`):
- Core/GalaxyState.h: FCivilizationState gains two saved int32 fields after FallenSphereTicks: LawsHeldTicks = 0 (field only; NOTHING counts it until [B-6b2]) and PressureEnduredTicks = 0.
- Sim/GalaxySimulationSubsystem.cpp TickEconomy: PressureEnduredTicks is counted NOW (current code) in the last loop of the economy stage, beside LowStabilityTicks and FallenSphereTicks, for every people: +1 when this tick's drain Loss > 0 and the capital (HomeSystemID) lies in no sphere of an unsealed fallen (Realm.Abyssal) pressing transcendent; otherwise 0. No draw, no knowing read.
- Sim/CosmicHazards.cpp MoveCapital: resets PressureEnduredTicks to 0 when the capital moves.
- Tests: Sim.SaveLawsHeld and Sim.HashCoversNewCounters (new Tests/CosmicUpperRungTests.cpp), Sim.PressureEndured (Tests/CosmicBookTests.cpp).
- SaveVersion stays 1; both hashes move for every baseline line (reflection hash covers the new UPROPERTYs); CSV deterministic columns are unchanged.
- CLAUDE.md is ALREADY edited by the lead (do not edit it): rule 4 now says "경제·외교·분쟁 단계 안의 차례(B2·B3)와 B6b의 걸음(경제 끝의 셈 둘·결박 완성·절멸 만료·AI의 조약·법칙 걸음·제동 — 추첨 없음)은 docs/20 전문." in place of the B2 and B3 parentheticals; rule 13 lists the new windows with "(B6b — \`[B-6b2]\`부터)"; rule 14 adds "(\`[B-6b3]\`부터 Core 술어 하나)". CLAUDE.md is at 16,377 of 16,384 bytes.
Developer answers (2026-11-08), to be recorded in DECISIONS_LOG by this commit: A1 '칸에 적힌 그대로 전부' (whole docs/50 B6b cell, one block); A2 '설계 표 그대로' (pillars.md design-table rows: T3->T4 '절멸 위기 종족을 차원 방주로 이주(프로젝트 + 외교 조약)', T4->T5 '은하 법칙 3개를 N틱 이상 유지 + 타락 초월자 1체 봉인'); A3 '요구를 떼고 칸은 따로 (권장)' (#1 closed: no personal-rung requirement, no resonance); A4 '재고 나서 정함 (권장)' (clock by a pre-written rule after measuring; B6a saves stay ended, SaveVersion 1). The fourteen [개발자] decisions of the header: 1, 2, 3, 5, 9, 14, 20, 21, 32, 35, 38, 41, 42 answered as recommended; decision 22 answered with alternative A (the design's neighbour reverence +0.2, ChangeReverence Key Borders) while its mechanism (counter, condition, selector Ruled, one Standing row) is as recommended.
Conventions you must keep (CLAUDE.md is in your context): a sentence about something not yet in code carries a marker such as "(B6b — \`[B-6b2]\`부터)" or "(\`[B-6b3]\`부터)" naming the commit that brings it (header section 8 says which commit brings what); an unmarked sentence is current rule. The same sentence is not written in two documents - link instead. Plain Korean like the surrounding text; match the document's existing style and density. Line numbers in the notes are of HEAD 5455e56 - find paragraphs by their first words. Never touch docs/OVERVIEW.html, docs/90_LORE/cosmic_species_codex.md or docs/archive/. Do not run git commands that change anything (no add, commit, checkout, stash). After editing run \`python -I Tools/check_docs.py\` and keep it at 0 errors.
`

const EDITORS = [
  {
    key: 'docs20',
    files: 'docs/20_ARCHITECTURE.md only',
    task: `Edit docs/20_ARCHITECTURE.md per notes 1.2 "[B-6b1]" (the docs/20 bullet) and header sections 2.1-2.12:
(a) In the appendix '불변 규칙 전문', rule 4's full text (the paragraph beginning '틱 처리 순서(열 단계)'): first confirm that every phrase the lead removed from CLAUDE.md rule 4 is present in this full text ('틱당 영압·칙령 효과와 칙령 유지비는 경제 단계 안(B2)'; '외교 단계는 우호의 유지비·국경 쌍의 관계 표류, 분쟁 단계는 함대 → 끝에 이른 공성마다 1회 뽑기 → 목표 점검 → 공성의 끝(B3)') and add whatever is missing. Then add one B6b paragraph in the style of the '**칙령**(B2 …)' and '**외교와 분쟁**(B3 …)' paragraphs: the two counts at the economy end (endured pressure - current; laws held - from [B-6b2]); in the crisis stage (a) the binding completion that first writes bSealed/SealedTick in one place; (b) the extinction expiry order (last-system gate/grace → LetPeopleGo → KillPlayer → DeposeRuler → record line); (c) the galaxy-law hooks on spawn chance and on Place's countdown; AI stage (a) the treaty and law steps (only when the judged row asks; after the edict revocation, before TickAIDiplomacy) and (b) the fourth-rung brake inside the judgement; the transit stage removes an extinct people; no new sim draw site. Device parts carry "(B6b — \`[B-6b2]\`부터)"; only the endured-pressure count is unmarked. Point to header section 2.11 and notes 0.4 for the table rather than copying it.
(b) Section 9.1 (save fields): one paragraph on the two FCivilizationState counters - frozen 0, delta save (Civs elements written whole, old saves read 0), SaveVersion 1, B6a saves ended at 6,000 stay ended (A4), the laws count arrives in [B-6b2].
(c) Section 9.2 (draw ledger): B6b adds no sim draw site (header 2.12): the extinction expiry uses the two existing draws, the deaths use the tenth-stage draws, the AI steps and the brake draw nothing, the era row and enc_witness are data (slot draw and answer draw as now). Mark device parts as above.
Return a short Korean summary of what you changed (sections and one line each).`,
  },
  {
    key: 'docs21',
    files: 'docs/21_DATA_MODEL_REFERENCE.md only',
    task: `Edit docs/21_DATA_MODEL_REFERENCE.md per notes 1.2 "[B-6b1]" (the docs/21 bullet) and the header: section 7.2 (the FCivilizationState copy: the two new members with their comments copied exactly from Core/GalaxyState.h in the working tree - the code is the original); section 7.3 (the three new condition enumerators CivHasTreaty, CivLawsHeldTicksAtLeast, CivPressureEnduredTicksReached appended after CivHasEdict; FEdictDef.Scope and UENUM ECosmicEdictScope {People, HomeGalaxy}; the SystemOwnedBy selector word Ruled that section 7.3 already names; the ChangeReverence Key Borders (decision 22 answer A); the three new DT_Rules knobs GalaxyLawsToHold, AIUpperRungProgressMultiplier, FleetDelayCountdownShare) - all marked "(B6b — \`[B-6b2]\`부터)"; section 7.4 (tick stage contents: remove '예정' from the extinction sentences that header 2.2 now decides but mark them "(B6b — \`[B-6b2]\`부터; 행은 \`[B-6b6]\`)", the brake now read (\`[B-6b2]\`), the AI steps, the endured-pressure count current, the world functions LetPeopleGo, KillsTheRulerOf, CosmicSeal::BindTargetOf, CosmicAscent::FourthRungOpenOf, GuardDelayCapOf with their commits); section 7.5 (the third condition reading SinceTick: CivLawsHeldTicksAtLeast reads Min(LawsHeldTicks, Tick − SinceTick) - player feat judgement uses BornTick, AI 0 - marked \`[B-6b2]\`). Keep the doc's rule: the code is the original; where the code is not yet written write the planned form with the marker. Return a short Korean summary of what you changed.`,
  },
  {
    key: 'design',
    files: 'docs/10_DESIGN/systems.md, docs/10_DESIGN/crisis_and_transcendents.md and docs/10_DESIGN/pillars.md only',
    task: `Edit the design documents per the "설계:" bullet of notes 1.2 "[B-6b1]" (systems.md :133, :166, :169, :162, :65, :91, :95, :411; crisis_and_transcendents.md :24, :65, :113, :115, :121, :128, :134, :135, :170, :172; pillars.md :105-108) so that the design says what header B6b decided (design → code order): the extinction is the people's own crisis answered by the ark, with the expiry order and the ruler's death as the second death source; galaxy laws = edicts whose scope widens to the home galaxy and widen only the two crisis hooks; the AI's narrow row-driven treaty and law steps (the six things the AI does by itself become eight); the binding is a construction project whose completer's record is read (the people's act said as a third-person fact, the binding itself only in the agentless passive '…묶였다'); the brake is read; '넷째 칸 이상의 존재' counted as header decision 40 says; #1 closed (no personal-rung requirement, no rung resonance - remove the pointer to resonance); #141 split (enc_witness for the first-rung wanderer now; facing and anchor of mind to the wanderer-rung block #164); #138 evacuation = the extinction's answer (ark); #156 era row with the design's neighbour reverence +0.2 (decision 22 answer A). Mark what is not yet in code with the commit that brings it (rows: [B-6b6]/[B-6b7]/[B-6b8]; devices [B-6b2]/[B-6b3]). Theology guardrails (CLAUDE.md rule 11): the holy order is never an entity/speaker/number/condition; no '봉인했다' with a creature as actor; no saviour titles. Return a short Korean summary of what you changed (file:paragraph, one line each).`,
  },
  {
    key: 'log',
    files: 'docs/60_DECISIONS/DECISIONS_LOG.md only',
    task: `Add a B6b section to docs/60_DECISIONS/DECISIONS_LOG.md in the style of its B6a section (read that section first; newest blocks are placed where the B6a section's position convention says). Record, dated 2026-11-08, per header decision 39 and notes 1.2: the developer's four scope answers A1-A4 with the chosen labels verbatim and the alternatives that were asked (header section 0 has both); the fourteen [개발자] answers (thirteen recommended; decision 22 alternative A - and say plainly that the post-review recommendation had been 'no effect, the line only' and the developer chose the design's +0.2); that B6b replaces the sealing part of B6a decision 1 (binding is in B6b; the T5 threshold, the two ways and the rest of #163 stay Phase 6+; the frozen B6a header is not edited); #1 closed (independent rungs, no resonance; the schema sentence changes in [B-6b2], the OPEN_QUESTIONS row goes in [B-6b12]); and a pointer that the new death source (decision 5) is recorded by [B-6b2] and the #142 rule by [B-6b11]. Do not record anything else ahead of its commit. Return a short Korean summary.`,
  },
]

phase('Edit')
const edits = await parallel(EDITORS.map(e => () =>
  agent(`${CONTEXT}\nYOUR FILES: ${e.files}. Edit no other file.\n\n${e.task}`, { label: `edit:${e.key}`, phase: 'Edit' })
    .then(r => ({ key: e.key, summary: r }))))

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
          kind: { type: 'string', enum: ['code', 'test', 'doc'] },
          where: { type: 'string' },
          claim: { type: 'string' },
          evidence: { type: 'string' },
          fix: { type: 'string' },
        },
        required: ['id', 'severity', 'kind', 'where', 'claim', 'evidence', 'fix'],
      },
    },
  },
  required: ['findings'],
}

const editSummary = edits.filter(Boolean).map(e => `## ${e.key}\n${e.summary}`).join('\n\n')

const LENSES = [
  {
    key: 'state',
    prompt: `Review lens: SAVED STATE, HASH AND DETERMINISM of the code of [B-6b1] (\`git diff -- Source\`). Check against header B6b sections 2.1, 2.7, 2.11, 2.12 and CLAUDE.md rules 2, 3, 6, 7: the two fields' placement and comments; frozen initializers; delta-save behaviour of Civs elements (read Core/GalaxyState.h's top comment and Save/CosmicSaveSubsystem.cpp); that the endured count matches header 2.7 exactly (drain Loss > 0 of THIS tick, capital outside every unsealed fallen sphere, every people, MoveCapital resets, no draw, no knowing read; what happens with an invalid HomeSystemID, a sealed transcendent, a non-fallen presser, a people with no system, the vessel during the interregnum); that nothing else in the code reads or writes the fields yet; that the world hash includes them (Sim/GalaxySimulationSubsystem.cpp ComputeWorldHash exclusions) and rule 6 holds; that the order inside TickEconomy (counted after the edicts' upkeep, with the other two counters) is what header 2.1/2.7 says; and whether any other caller of MoveCapital or any capital change bypasses the reset (search for writes of HomeSystemID). Report only real defects with file:line evidence.`,
  },
  {
    key: 'docs',
    prompt: `Review lens: DOCS-FIRST AUDIT. The editors of this run changed docs/20_ARCHITECTURE.md, docs/21_DATA_MODEL_REFERENCE.md, docs/10_DESIGN/{systems,crisis_and_transcendents,pillars}.md and docs/60_DECISIONS/DECISIONS_LOG.md (\`git diff -- docs\`; their summaries are below). Check every changed sentence against header B6b (sections 0, 2, 3, 8, 10) and against the code in the working tree: is it true, does it carry the right commit marker for what is not yet code (and no marker for what is current: the two fields and the endured count), does it contradict another document, is any sentence duplicated across two documents instead of linked, is anything from notes 1.2's "[B-6b1]" list missing, did the docs/20 rule-4 full text keep every phrase the lead removed from CLAUDE.md rule 4 (see CONTEXT), does DECISIONS_LOG record exactly what decision 39 assigns to [B-6b1] (no more, no less, decision 22 answer A stated), and do the design edits keep CLAUDE.md rule 11's theology. Also read the lead's CLAUDE.md diff (\`git diff -- CLAUDE.md\`) for meaning lost or a wrong marker. Report only real defects with exact text and a concrete fix.\n\nEDITOR SUMMARIES:\n${editSummary}`,
  },
  {
    key: 'tests',
    prompt: `Review lens: TESTS of [B-6b1]. Read Tests/CosmicUpperRungTests.cpp (Sim.SaveLawsHeld, Sim.HashCoversNewCounters) and Sim.PressureEndured in Tests/CosmicBookTests.cpp against header 6 and notes 1.3 "[B-6b1]". For each: would the mutation named in its comment really break it (initializer 1; naming a counter in the world-hash exclusions; removing the MoveCapital reset)? Are there cases a plausible bug would pass (e.g. counting the capital as inside when only another system is inside; counting a tick without drain; counting the start-of-stage sphere instead of this tick; the old-save byte rename trick hitting other names or silently loading nothing)? Is any assertion fragile (depends on the game's DT_Rules values, the slice size, the board helpers' capitals)? Check helper facts in Tests/CosmicTestHelpers.h (Board, Give, WireChain, SliceCivs) and the pressure formula in Sim/GalaxySimulationSubsystem.cpp (PressureWithReach, TranscendentTermOf, DrainLossOf) so that the expected counts (4, 0, 2, 0, 0, 3, 0, 1) are right. Report only real defects with a concrete fix.`,
  },
]

phase('Review')
const reviews = await parallel(LENSES.map(l => () =>
  agent(`${CONTEXT}\nYou are READ-ONLY: do not edit any file.\n\n${l.prompt}`, { label: `review:${l.key}`, phase: 'Review', schema: FINDINGS })
    .then(r => ({ lens: l.key, findings: (r && r.findings) || [] }))))

const all = reviews.filter(Boolean).flatMap(r => r.findings.map(f => ({ ...f, lens: r.lens })))
log(`${all.length} findings to verify`)

const VERDICTS = {
  type: 'object',
  properties: {
    verdicts: {
      type: 'array',
      items: {
        type: 'object',
        properties: {
          id: { type: 'string' },
          verdict: { type: 'string', enum: ['confirmed', 'partial', 'refuted'] },
          reason: { type: 'string' },
          fix: { type: 'string' },
        },
        required: ['id', 'verdict', 'reason', 'fix'],
      },
    },
  },
  required: ['verdicts'],
}

let verdicts = []
if (all.length) {
  phase('Verify')
  const v = await agent(`${CONTEXT}\nYou are READ-ONLY. You are a skeptic: for EACH finding below, try to refute it by reading the cited files and the header. Default to 'refuted' when the evidence does not hold at the cited place; 'partial' when the defect is real but the fix is wrong or too wide (give the narrower fix). Return one verdict per finding id.\n\nFINDINGS:\n${JSON.stringify(all, null, 1)}`, { label: 'verify:all', phase: 'Verify', schema: VERDICTS })
  verdicts = (v && v.verdicts) || []
}
const byId = new Map(verdicts.map(v => [v.id, v]))
const kept = all.map(f => ({ ...f, verdict: byId.get(f.id) })).filter(f => f.verdict && f.verdict.verdict !== 'refuted')
const docFixes = kept.filter(f => f.kind === 'doc')
const codeFixes = kept.filter(f => f.kind !== 'doc')

let fixed = 'no doc findings survived'
if (docFixes.length) {
  phase('Fix')
  fixed = await agent(`${CONTEXT}\nApply these verified DOC findings (use the verifier's fix when it differs from the finder's). Edit only docs/20_ARCHITECTURE.md, docs/21_DATA_MODEL_REFERENCE.md, docs/10_DESIGN/*.md and docs/60_DECISIONS/DECISIONS_LOG.md; if a finding needs CLAUDE.md or code, do not do it - list it. Run \`python -I Tools/check_docs.py\` (0 errors). Return a Korean report: per finding id, what you did.\n\nFINDINGS:\n${JSON.stringify(docFixes, null, 1)}`, { label: 'fix:docs', phase: 'Fix' })
}

return { edits, findings: kept, codeFixes, refuted: all.length - kept.length, fixed }
