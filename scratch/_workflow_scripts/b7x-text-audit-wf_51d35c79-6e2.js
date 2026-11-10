export const meta = {
  name: 'b7x-text-audit',
  description: 'B7x [B-7x4] text audit drafted in parallel with B6b: the twelve crossroads cards, nine event_brick rows, the Shape/Inward keys, the scan-face template and the Brick tag DevComments - drafts, five-lens audit, verify, fix, completeness critic; output to scratch only',
  phases: [
    { title: 'Draft', detail: 'three drafters on disjoint row groups' },
    { title: 'Audit', detail: 'five lenses over all drafts' },
    { title: 'Verify', detail: 'one refuter per medium/high finding' },
    { title: 'Fix', detail: 'one fixer writes the audited file' },
    { title: 'Critic', detail: 'completeness critic, then a last fix' },
  ],
}

const SCR = 'C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad'
const OUT = SCR + '/b7x/stage3/texts'
const MAIN = 'C:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension'
const H = 'docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md'
const N = 'docs/60_DECISIONS/headers/B7_composable_existence_and_evolution_notes.md'
const D = 'docs/10_DESIGN/EXISTENCE_EVOLUTION_AND_ECOLOGY.md'

const CONTEXT = `
Project: Cosmic Ascension (UE 5.8 grand-scale simulation; Korean in-game text; solo developer). You are doing the text part of block B7x ([B-7x4] '글') ahead of time, in parallel with block B6b. Output goes ONLY to scratch files under OUT = ${OUT} (create it). Do not edit any repository file and do not run git write commands anywhere. Read-only sources:
- B7x documents, reviewed and answered by the developer on 2026-11-10, at design-branch commit 79f2583 (read with: git -C "${MAIN}" show 79f2583:<path>): header ${H} (section 3.1 rows, 3.2 Shape keys and word rule, 3.3 schema rules, 3.5 text rules, 4 presentation incl. decision 13 rule (d), 10 decisions), notes ${N} (section 5.4 '[B-7x4] 글' 데2/데3, '[B-7x5c]' 데1, slice-다 데3), accepted design doc ${D} (section 4.3 cards C01-C12 with their intent and draft lines, section 3.3 rules S01-S15, 4.4 shape table).
- The data draft at commit aa09543 (git -C "${MAIN}" show aa09543:Tools/data/DT_Encounters.json, :Tools/data/DT_Events.json, :Tools/data/schema.json): the twelve crossroads rows (petition_cross_* eleven, enc_cross_* one) and the nine event_brick_* rows, with their current draft texts, conditions and effects (old leaf names there - irrelevant for Korean text; new names: KeptLaw, IdleFeed, FoldedSmall, ShutSphere, DustBand, InwardHunger, StarTow, ClosedPort).
- Main's live files for style and rules (read-only): ${MAIN}/Tools/data/DT_Encounters.json and DT_Events.json (existing card/event texts and their template grammar), ${MAIN}/Tools/data/schema.json (named word lists: verdict, holy_words, rescue_words, silence_words, rung_grasp_words, title_words, desolation_words, remnant_words, dawn_words, holy_passive, ark_words, and their rule hooks), ${MAIN}/Tools/data/ST_CA.csv (key style), ${MAIN}/docs/70_CONTENT_GUIDE.md (log/template grammar like {Civ}|hpp(은,는), narrative rules, two voices), ${MAIN}/docs/15_GLOSSARY.md.

BINDING DECISIONS (developer answers 2026-11-10, all as recommended)
- Leaf names: KeptLaw, IdleFeed, FoldedSmall, ShutSphere, DustBand, InwardHunger, StarTow, ClosedPort (design names never appear on screen anyway - header 3.5).
- Decision 11 (나): a second-cross card's choice 2 does not close the card, so it carries NO gain that stacks over repeats - only a cost and a small one-off effect in the petition_road / petition_feast shape (read those rows in main to see the shape). At least petition_cross_law_stays choice 2 loses PlayerStatDelta Exaltation +1. Anything that accumulates in a saved stat with each repeat (Exaltation, Reverence, permanent traits, ethics shifts) counts as a stacking gain and must go; judge each second-cross choice 2 (C05-C11, C09's choice 2 is the undo) and propose the exact effect list.
- Decision 18 (KeptLaw narrowed): a people keeping the edicts after the throne empties still revokes claim-hook and crisis-answer edicts; the text must not promise more.
- Decision 13 ㉢ (d): a shape word reads as 'that people's present way of life' (살림), never a title, verdict, merit or ending name.
- Decision 16: births are not in this slice; no text may say an old people survived, returned or was reborn.
- Notes 5.4 데2 fixes (apply): C10 title '비워 둔 왕좌' -> '거두지 않은 칙령'; C10 choice 1 SuccessText -> '{Civ}|hpp(은,는) 정해 둔 칙령을 손대지 않은 채 두었다. 칙령을 정한 이는 자리에서 내려왔다.'; drop C10 choice 2's ShiftEthics Spiritualism +0.05; the voice line '견딘 날들 내내' -> '눌린 날들 내내'; petition_cross_into_dust '견딘 날이 길었다' -> '떠돈 날이 길었다' (the audit settled on 떠돈, review L4-18). C11 choice 1's value (ChangeStability +0.08) is not changed now.
- Header 3.5 text rules: design names of bricks/time counts/shapes never on screen - choices are named by what they do, shapes only with '살림' words; at most two sentences, no digits, no counts of people; a people's change is told as the fact of what it did and where it then stands; the two voices of a petition ask for DIFFERENT choices and neither is written as right; banned words: 구원·중보, 침묵·고요·잠잠·쉼·안식, 울타리·언약·방주, 목자·순례·성소, 불멸·영원·영겁, '신이 되'·'하나가 되'·일체화; after an abdication no picture of waiting, return or an empty seat; branches that resemble a fallen doctrine (entwinement, idle days) are never written as a good end. New list crossroads_words ['침묵','울타리','언약','방주','목자','순례','성소','불멸','하나가 되'] hooks the twelve cards' text columns, the nine event_brick rows and log_brick_scan_spread together with silence_words, rung_grasp_words, title_words.
- Shape.* keys (header 3.2 drafts): BuiltOrLinked '짓고 이은 살림', ConsumedOrDrifted '넓히고 삼킨 살림', Isolated '홀로 선 살림', Endured '눌림 속에 이어 간 살림', WovenSwarm '촘촘히 엮은 일손의 살림', StarHerd '머물지 않는 살림', SpreadBody '별들 사이에 번진 살림', FoldedSmall '티끌 속으로 접힌 살림', ShutHouse '한 별에 닫아 건 살림', KeptLaw '법을 남긴 살림', LastHunger '끝없이 삼키는 살림', Entwined '이웃과 뒤엉킨 살림'. Rule: Chronicle-prefix lists plus remnant_words and desolation_words, plus new shape_merit_words ['견디','남은']; never '오래 버틴 살림'.
- Project rule 11 (CLAUDE.md): God/the holy order is never an entity, speaker, number, lever or ending; first-person log speakers only the fallen; no verdict words in narrator text; no counts of people as numbers; invented names are Korean descriptive names (no names from other works, myths, Bible proper names, real people); no saviour/messiah titles for the player, never 'became god'; never add a Bible verse.
`

const DRAFT_SCHEMA = {
  type: 'object',
  properties: {
    file: { type: 'string' },
    rows: { type: 'array', items: { type: 'string' } },
    effect_changes: { type: 'array', items: { type: 'string' } },
    open_points: { type: 'array', items: { type: 'string' } },
  },
  required: ['file', 'rows', 'effect_changes', 'open_points'],
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
          row: { type: 'string' },
          field: { type: 'string' },
          claim: { type: 'string' },
          evidence: { type: 'string' },
          fix: { type: 'string' },
        },
        required: ['id', 'severity', 'row', 'field', 'claim', 'evidence', 'fix'],
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

const FIX_SCHEMA = {
  type: 'object',
  properties: {
    file: { type: 'string' },
    applied: { type: 'array', items: { type: 'string' } },
    skipped: { type: 'array', items: { type: 'string' } },
    schema_changes: { type: 'array', items: { type: 'string' } },
    st_ca_lines: { type: 'array', items: { type: 'string' } },
    open_points_for_lead: { type: 'array', items: { type: 'string' } },
  },
  required: ['file', 'applied', 'skipped', 'schema_changes', 'st_ca_lines', 'open_points_for_lead'],
}

const FORMAT = `Write your file as UTF-8 (LF) markdown. For every row: a heading with the row name and its card/rule ID (C01..C12, S-rule for events), the row's conditions and effects in one short line each (as they will be after the binding decisions), then one fenced json block holding ONLY the text fields exactly as they should be pasted into the data (same field names and nesting as the data draft: Title, Body, VoiceBodies, Choices[].Label, SuccessText, FailureText, OutcomeText/variants or whatever the row type uses - keep every template token such as {Civ}|hpp(은,는) intact and valid), then a 'why' line per changed field (old -> new, which rule). Keep untouched fields too, so the block is complete.`

phase('Draft')
const GROUPS = [
  { key: 'first', label: 'first-cross cards', what: 'the first-crossroads petitions (the cards whose condition is CivHasTrait Brick.Form.Society - they close together by removing Society; from the design doc 4.3 these are the first crossroads, C01-C04 or as the doc numbers them) - all text fields' },
  { key: 'second', label: 'second-cross cards + wanderer card', what: 'the second-crossroads petitions (C05-C12 as numbered in the design doc 4.3, closed by CivLacksTrait of the tag choice 1 gives; C09 is the undo) and the wanderer card enc_cross_* - all text fields, AND for every choice 2 the exact effect list after decision 11 (나) with one line of reason per removed or kept effect' },
  { key: 'events', label: 'event rows, keys, scan face, DevComments', what: 'the nine event_brick_* rows (all text fields and variants), the twelve Shape.* string keys and Settle.Block.Inward (final Korean text for each, one ST_CA.csv line each in the file\'s exact CSV format), the scan-face log template log_brick_scan_spread (DT_LogTemplates row text - S15: one rumour line, only beside its own star, never \'everywhere\'), and the English DevComment of each of the seventeen Brick tags (read them in the integration tree C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad/b7x/int/Config/DefaultGameplayTags.ini - another workflow may be editing that tree, read only) checked against K6 and rule 11' },
]

const drafts = await parallel(GROUPS.map((g) => () => agent(`${CONTEXT}

YOUR TASK (drafter '${g.label}', effort high): draft the final texts for ${g.what}. Start from the aa09543 draft text, apply every binding decision and the header 3.5 rules, keep the card's intent from the design doc 4.3/3.3, and match main's card style (length, register, two-voice petitions, template grammar). Where the draft already obeys every rule, keep it word for word.
${FORMAT}
Write to ${OUT}/draft_${g.key}.md. Return file, rows (row names covered), effect_changes (data changes you propose beyond text, e.g. choice-2 effects - exact before/after), open_points (anything that needs the lead or the developer).`, { label: `draft:${g.key}`, phase: 'Draft', schema: DRAFT_SCHEMA, effort: 'high' })))

const draftFiles = drafts.filter(Boolean).map((d) => d.file)
log(`drafts: ${draftFiles.length}/3`)

const LENSES = [
  { key: 'theology', effort: 'xhigh', prompt: 'LENS THEOLOGY AND NARRATIVE RULE 11: check every sentence against rule 11 and K6 (no people/ruler becoming law, covenant, god or holy order; God never an entity/speaker/lever/ending; no verdict words in narrator text; no saviour titles; no new verse; first person only for the fallen), the banned words of header 3.5 and every named schema list the row will be checked against (read the lists in main schema.json and the rule hooks for DT_Encounters, DT_Events, DT_LogTemplates, Chronicle./Shape./Settle.Block. prefixes). Also: fallen-doctrine-like branches (entwinement, idle days) never framed as a good end; abdication lines with no waiting/return/empty-seat picture; decision 16 (no survival/return/rebirth).' },
  { key: 'voices', effort: 'high', prompt: 'LENS TWO VOICES AND CHOICES: every petition card has two voices that ask for DIFFERENT choices and neither is written as right; the choice labels name what the choice does (not a design name, not a shape name); the dismiss/no-cost choice exists where schema rule 4 needs it and reads as a plain pass; the wanderer card voice fits a wanderer meeting another people. Compare with main\'s existing petitions for register.' },
  { key: 'grammar', effort: 'high', prompt: 'LENS KOREAN AND TEMPLATE GRAMMAR: particles and template tokens ({Civ}|hpp(은,는) and the other forms in docs/70) valid for every name ending; sentence count (at most two), no digits, natural plain Korean, no coined shorthand, consistent terms with the glossary (docs/15) and existing cards; ST_CA.csv lines correctly quoted/escaped for the file format; JSON blocks valid (parse them).' },
  { key: 'mechanics', effort: 'xhigh', prompt: 'LENS TEXT VS MECHANICS: each text says only what the row really does (read the row conditions/effects in aa09543 and the proposed effect changes): success/failure lines match the effects, costs mentioned match, the KeptLaw card does not promise keeping claim-hook or crisis-answer edicts (decision 18), choice 2 of second-cross cards after decision 11 (나) has no stacking gain (judge each proposed effect list: is anything accumulating per repeat?), the C10 ShiftEthics removal is applied, shape words fit decision 13 (d) and do not read as titles/merit, the scan face is a single rumour line beside its own star (S15, decision 18 \'everywhere\' not accepted).' },
  { key: 'consistency', effort: 'high', prompt: 'LENS CONSISTENCY AND COVERAGE: all twelve cards, nine event rows, twelve Shape keys + Settle.Block.Inward, the scan template and seventeen DevComments are covered exactly once across the three draft files; no row from aa09543 is missing; names and IDs (C01..C12) match the design doc 4.3; the same thing is called by the same word across rows (e.g. the same act in a card and its event row); proposed schema changes (crossroads_words, shape_merit_words, which prefixes/rows they hook) are written down exactly.' },
]

phase('Audit')
const audited = await pipeline(
  LENSES,
  (lens) => agent(`${CONTEXT}

You are an auditor (read-only). The drafts are: ${draftFiles.join(', ')}. Drafters' reports: ${JSON.stringify(drafts)}

${lens.prompt}

Report only real defects with evidence (row, field, quoted text). Severity: high = a rule broken (rule 11, banned word, a schema list the row is hooked to, a text that misstates mechanics); medium = voices/choices wrong, grammar error, inconsistency; low = wording. Each finding needs an exact replacement text or action.`, { label: `audit:${lens.key}`, phase: 'Audit', schema: FINDINGS_SCHEMA, effort: lens.effort }),
  (res, lens) => {
    const findings = (res && res.findings) || []
    log(`lens ${lens.key}: ${findings.length} findings`)
    return parallel(findings.map((f) => () => {
      if (f.severity === 'low') return Promise.resolve({ lens: lens.key, finding: f, verdict: { verdict: 'confirmed', reason: 'low severity - not verified, fixer judges', corrected_fix: '' } })
      return agent(`${CONTEXT}

You are a skeptical verifier. Try to REFUTE this audit finding about the drafts (${draftFiles.join(', ')}). Read the draft and the sources. If wrong: refuted. If right but the fix is wrong: partial with corrected_fix. If right: confirmed.
Finding (${lens.key}): ${JSON.stringify(f)}`, { label: `verify:${lens.key}:${f.id}`, phase: 'Verify', schema: VERDICT_SCHEMA, effort: 'high' })
        .then((v) => ({ lens: lens.key, finding: f, verdict: v }))
    }))
  },
)

const all = audited.filter(Boolean).flat().filter(Boolean)
const kept = all.filter((x) => x.verdict && x.verdict.verdict !== 'refuted')
const refuted = all.filter((x) => x.verdict && x.verdict.verdict === 'refuted')
log(`audit: ${all.length} findings, kept ${kept.length}, refuted ${refuted.length}`)

phase('Fix')
const fix = await agent(`${CONTEXT}

YOUR TASK (fixer, effort high): merge the three drafts (${draftFiles.join(', ')}) into ONE audited file ${OUT}/audited_texts.md with these parts: (1) the twelve cards, (2) the nine event rows, (3) ST_CA.csv lines (Shape.* twelve, Settle.Block.Inward), (4) the scan template row, (5) the seventeen DevComments, (6) proposed data effect changes (choice 2 of second-cross cards etc.), (7) schema changes (crossroads_words, shape_merit_words, their hooks, any list addition), (8) a short record: how many findings per lens, kept/refuted, and what was left to the lead. Apply every kept finding below (low ones: apply when right, skip with a reason when not). Keep each row's ${FORMAT}
Parse every json block you write (python -I -X utf8 -c 'import json,sys; ...') and grep the file for the banned words of header 3.5 and the verdict words; report counts.
KEPT: ${JSON.stringify(kept.map((x) => ({ lens: x.lens, id: x.finding.id, severity: x.finding.severity, row: x.finding.row, field: x.finding.field, claim: x.finding.claim, fix: x.finding.fix, verdict: x.verdict.verdict, corrected_fix: x.verdict.corrected_fix })), null, 1)}
REFUTED (do not apply): ${JSON.stringify(refuted.map((x) => ({ id: x.finding.id, claim: x.finding.claim, reason: x.verdict.reason })))}`, { label: 'fix:texts', phase: 'Fix', schema: FIX_SCHEMA, effort: 'high' })

phase('Critic')
const critic = await agent(`${CONTEXT}

YOUR TASK (completeness critic, effort high, read-only): read ${OUT}/audited_texts.md against the sources. What is missing or wrong? A row, field, variant or key not covered; a binding decision not applied; a choice-2 stacking gain left in; a text that a schema list will reject; a JSON block that does not parse; a template token broken; a shape word that reads as a title or merit; a DevComment that says a people becomes law/holy. Report concrete gaps only, with exact fixes.`, { label: 'critic', phase: 'Critic', schema: FINDINGS_SCHEMA, effort: 'high' })

let last = null
const gaps = (critic && critic.findings) || []
if (gaps.length > 0) {
  last = await agent(`${CONTEXT}

YOUR TASK (last fix, effort high): apply these critic findings to ${OUT}/audited_texts.md (skip with a reason any that is wrong), re-parse the json blocks and re-run the banned-word grep, and update part (8)'s record.
FINDINGS: ${JSON.stringify(gaps, null, 1)}`, { label: 'fix:critic', phase: 'Critic', schema: FIX_SCHEMA, effort: 'high' })
}

return { drafts, findings_total: all.length, kept: kept.length, refuted: refuted.map((x) => ({ id: x.finding.id, claim: x.finding.claim })), fix, critic_gaps: gaps.length, last }
