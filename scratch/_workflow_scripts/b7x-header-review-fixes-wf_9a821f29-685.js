export const meta = {
  name: 'b7x-header-review-fixes',
  description: 'Apply the decision-independent fixes of the B7x adversarial review to the header, notes and design doc in the design-branch worktree, record code/data/test fixes as rebase hand-offs, verify, fix',
  phases: [
    { title: 'Apply', detail: 'one writer applies the decision-independent text fixes and records the rest' },
    { title: 'Check', detail: 'two lenses: fidelity to the review, theology and doc rules' },
    { title: 'Verify', detail: 'one refuter per finding' },
    { title: 'Fix', detail: 'apply kept findings' },
  ],
}

const SCR = 'C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad'
const B7 = `${SCR}/b7x`
const WT = `${B7}/wt`
const REPORT = `${B7}/review_report.md`
const HDR = 'docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md'
const NOTES = 'docs/60_DECISIONS/headers/B7_composable_existence_and_evolution_notes.md'
const DESIGN = 'docs/10_DESIGN/EXISTENCE_EVOLUTION_AND_ECOLOGY.md'

const COMMON = `
Context: Unreal C++ game Cosmic Ascension. The B7x block (composable existence) arrived as branch design/b7x-composable-existence, made
on a PC without the engine (never compiled). An ultracode adversarial review just finished: ${REPORT} (Korean; 63 findings, 62 kept -
section 2 the developer decisions, section 3 the fixes with their target commit and whether they wait for a developer answer, section
4 the rebase plan, section 5 refuted claims; the per-lens JSON with verdicts is ${B7}/review_<lens>.json for lens in design,
determinism, theology, measurement, build, integration).
YOU WORK ONLY IN THE WORKTREE ${WT} (a separate checkout of the branch; its files are CRLF on disk because of '* text=auto' - write
the files you edit with LF (python open(..., 'w', encoding='utf-8', newline='\\n')); git normalizes on commit, so measure byte limits on
LF content). NEVER touch the main project tree C:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension (B6b is being
built and measured there); never run Build.bat, UnrealEditor, run_checks, make_data_tables; never commit or push (the lead does).
Files you may edit in the worktree: ${HDR}, ${NOTES}, ${DESIGN}. Do NOT edit code, data, Config, Tools, tests, CLAUDE.md, STATUS.md or
any other living doc (those change at rebase/approval time, after B6b ends).
THE DEVELOPER HAS NOT ANSWERED the [개발자] decisions (1,2,3,4,6,8,11,13,14,15,16,18,20,22, plus whether to raise 9/23 and the archive
question for the original design text and docs/OVERVIEW_as_of_B6a.html). So: apply every kept fix that does NOT wait for a developer
answer; for every fix that does wait, do not decide it - write it into header section 10 under that decision as the review's sharpened
recommendation and alternatives, clearly marked '(리뷰 반영 — 개발자 답 대기)', with the consequence the developer must see.
Code, data and test fixes (report 3.2-3.4) are NOT applied now: record each in notes section 5 as a rebase hand-off (finding ids, the
exact fix - the verifier's corrected fix for partial ones -, the target commit per report section 4, and the test or mutation that must
show it).
Rules: keep the documents' style (plain Korean, the existing section numbering; '추가 이야기와 보완' ends reports); cite file:line;
add no Bible verse (only checked verses may be cited; others 'pattern only, 대조 필요' and only in the notes); never open anything under
C:/Users/rhwwl/Downloads/jehoba; docs/90_LORE/cosmic_species_codex.md is read-only; theology guardrails: the holy order (God) is never an
entity, speaker, number, lever or end condition; first person only for fallen transcendents; no soul-counting; no saviour titles, no
'became god'. Run python -I -X utf8 Tools/check_docs.py inside the worktree (cd there) - it must stay at 0 errors.`

phase('Apply')
const apply = await agent(`You are the writer who applies the B7x review's decision-independent fixes.${COMMON}

Do, in the worktree:
1. Header: every kept text fix of report 3.1 that does not wait for an answer (R list completeness: add the missing Rs the review names,
   e.g. R11 seed bricks narrowing rule 3, R12/R13 as the report numbers them, each with the exact sentence to change and its test; the
   K9 narrowing proposal stays a decision-18 item; section 5 measurement: judgment 6 (re-measure the B6b race under seed bricks with a
   breakable check), the [B-7x5] split into [B-7x5s] seeds/knobs and [B-7x5c] cards/events (+ [B-7x5r] if knobs change), the measured
   lines the design doc promised (claims, theodicy, lost peoples); section 8 order and conflicts exactly as report section 4 (the true
   conflict list, the 'words' resolution - time_vectors line appended - and the re-stack order: [B-7x1]+[B-7x2] rebuilt as one, data
   only after the measuring skip/off-pair/bricks: line and [B-7x4]); stale facts (line numbers, test counts, '#164-#166', file names)
   corrected against main 10205b6 (git -C "C:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension" show 10205b6:<path> is
   read-only and allowed); the slice B/C (나/다) spec contradictions that the report says to fix now without a decision.
2. Header section 10: for decisions 1,3,11,13,14,15,16,18,20,22 (and the 9/23 raise question, and the archive question) write the
   review's sharpened recommendation and alternatives with '(리뷰 반영 — 개발자 답 대기)'. Keep the header's existing recommendations
   unless the report says the review changed them (decision 3's names: IdleFeed, DustBand, InwardHunger, StarTow, optional ClosedPort).
3. Notes: section 5 gets the rebase hand-offs (report 3.2-3.4, grouped by target commit) and the review record; section 2/3 updates the
   report calls for (new open questions drafts if any).
4. Design doc: only the status marks and corrections the report assigns to [B-7x0] that do not wait for decision 21's archive answer.
5. Header section 11: one record line for this review and fix (counts per lens: findings 63, confirmed 32, partial 30, refuted 1;
   what was applied now, what waits).
6. check_docs 0 errors; header size (LF bytes) reported; list every change with its finding id in ${B7}/fix_log.md.
Final report (Korean): what you applied (by finding id), what you recorded as waiting for which decision, the hand-off count per target
commit, check_docs, sizes.`, { label: 'apply', phase: 'Apply', effort: 'xhigh' })

const FIND = {
  type: 'object',
  properties: { findings: { type: 'array', items: { type: 'object', properties: {
    id: { type: 'string' }, severity: { type: 'string' }, where: { type: 'string' }, claim: { type: 'string' },
    evidence: { type: 'string' }, fix: { type: 'string' } }, required: ['id', 'severity', 'where', 'claim', 'evidence', 'fix'] } } },
  required: ['findings'],
}
const LENSES = [
  { key: 'fidelity', prompt: `FIDELITY: compare the worktree diff (cd ${WT}; git diff) with ${REPORT} and ${B7}/fix_log.md. Every kept
finding (confirmed and partial - partial with the verifier's corrected fix) is either applied as text, written into section 10 as a
waiting decision item, or recorded as a hand-off in notes section 5 - none dropped, none applied wrongly, none decided on the
developer's behalf (a [개발자] item must not read as decided). Refuted claims (report section 5) were not applied. Section 8's conflict
list and order equal report section 4.` },
  { key: 'rules', prompt: `THEOLOGY, PLAIN WORDS AND DOC RULES: read every new or changed sentence in the diff. No Bible verse added
(book abbreviation + chapter:verse) outside the notes' 'pattern only, 대조 필요'; the holy order never an agent/speaker/number/lever/end;
no titles; no coined shorthand without a gloss (plain words); no rule weakened beyond what an R item states; LF in edited files;
check_docs 0; header size reasonable.` },
]

phase('Check')
const checks = await pipeline(
  LENSES,
  l => agent(`You are check lens '${l.key}' over the B7x review-fix edits (uncommitted in the worktree).${COMMON}

Writer's report:
${apply}

YOUR LENS: ${l.prompt}
Read-only (do not edit). Report only real defects with evidence; each fix exact.`, { label: `check:${l.key}`, phase: 'Check', schema: FIND, effort: 'high' }),
  async (res, l) => {
    const fs = (res && res.findings) || []
    const vs = await parallel(fs.map(f => () => agent(`Adversarially verify one finding about the B7x review-fix edits.${COMMON}

Try to refute it: re-read the cited lines in the worktree and the report yourself. Answer confirmed / partial (with the corrected fix) /
refuted with evidence. Read-only.

FINDING (lens ${l.key}): ${JSON.stringify(f)}`, { label: `verify:${l.key}:${f.id}`, phase: 'Verify', effort: 'high',
      schema: { type: 'object', properties: { verdict: { type: 'string', enum: ['confirmed', 'partial', 'refuted'] },
        corrected_fix: { type: 'string' }, reason: { type: 'string' } }, required: ['verdict', 'corrected_fix', 'reason'] } })
      .then(v => ({ ...f, lens: l.key, verdict: v }))))
    return vs.filter(Boolean)
  },
)
const found = checks.filter(Boolean).flat()
const kept = found.filter(f => f.verdict && f.verdict.verdict !== 'refuted')
log(`check findings ${found.length}, kept ${kept.length}`)

phase('Fix')
let fix = 'no kept findings'
if (kept.length) {
  fix = await agent(`You fix the kept check findings on the B7x review-fix edits in the worktree.${COMMON}

Apply each (the verifier's corrected fix for partial ones), update ${B7}/fix_log.md, run check_docs in the worktree. Final report
(Korean): per finding what changed.

FINDINGS:
${JSON.stringify(kept, null, 1)}`, { label: 'fix', phase: 'Fix', effort: 'xhigh' })
}

return { apply, found, fix }
