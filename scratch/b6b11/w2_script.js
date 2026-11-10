export const meta = {
  name: 'b6b11-readings-saves-docs',
  description: 'B-6b11: verify the bundle C readings independently, make the hand-check saves, write the records, docs lens',
  phases: [
    { title: 'Readings', detail: 'verify verdict, controls, bridges; recompute key readings independently' },
    { title: 'Saves', detail: 'hand-check saves one at a time (brief 4.4)' },
    { title: 'Docs', detail: 'N and H; DL, OQ, STATUS, CHANGELOG' },
    { title: 'Lens', detail: 'bytes, verbatim, placeholders, numbers, frozen hash, python checks, commit message' },
  ],
}

const SP = 'C:\\Users\\rhwwl\\AppData\\Local\\Temp\\claude\\c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension\\37de2919-0c44-45d8-94cd-4dcf0860263e\\scratchpad'
const CTX = `You work on step [B-6b11] (bundle C re-measure) of Cosmic Ascension. FIRST read ${SP}\\b6b11\\w2_context.md (the lead's fact sheet: start values, what ran, the aborted first attempt, rules) and the brief ${SP}\\b6b11\\brief_final.md sections 0-7. The bundle of record is in ${SP}\\b6b11\\bundleC (console output ${SP}\\b6b11\\bundleC_run_out.txt; summary and per-run files in the folder), archived to <repo>\\Saved\\Balance\\b6b_bundleC.`

const READ_SCHEMA = {
  type: 'object',
  properties: {
    verdict: { type: 'string' },
    header_facts: { type: 'string', description: 'date, build, processes (jobs, attempts, reruns, failures), wall minutes, folder and archive (file count, manifest), end/race check counts, sha1 guard, freeze sha1 of the five files as printed' },
    controls: { type: 'string', description: 'F1-F8 results with counts and notes' },
    bridges: { type: 'string', description: 'B-a to B-e: results, which B-d sides ran or were skipped and why, may-move seeds, moved count, each pair G that was recorded' },
    readings: { type: 'object', description: 'one string per placeholder, numbers exactly as the harness printed them (Korean outcome words kept): C_MINUTES, FROZEN_SHA1, J136, N142, A132, B1, R129, R154, LT15, KINDS, R10, R158, RUNLEN, HC' },
    per_cell_table: { type: 'string', description: 'per cell: seeds x ticks, record kinds counts' },
    recompute: { type: 'array', items: { type: 'string' }, description: 'each independent recomputation: what, harness value, your value, match or mismatch (and cause if mismatch)' },
    developer_items: { type: 'array', items: { type: 'string' }, description: 'each [개발자] item raised by the readings: numbers + the recommended resolution from the brief/H (developer accepted recommended options on 2026-11-10); say if a knob change [B-6b11b] would be the recommendation' },
    notes: { type: 'array', items: { type: 'string' } },
  },
  required: ['verdict', 'header_facts', 'controls', 'bridges', 'readings', 'per_cell_table', 'recompute', 'developer_items', 'notes'],
}

const LENS_SCHEMA = {
  type: 'object',
  properties: {
    fixed: { type: 'array', items: { type: 'string' }, description: 'each problem found and fixed by you (file, what)' },
    open: { type: 'array', items: { type: 'string' }, description: 'problems found but not fixed, with reason' },
    checks: { type: 'string', description: 'results of the five python checks, H bytes, CLAUDE.md untouched, CR scan, frozen item sha1 re-check, placeholder scan' },
    counts: { type: 'string', description: 'docs lens: findings found / fixed / rebutted' },
    commit_message_path: { type: 'string' },
    files_changed: { type: 'array', items: { type: 'string' } },
  },
  required: ['fixed', 'open', 'checks', 'counts', 'commit_message_path', 'files_changed'],
}

phase('Readings')
const [readings, saves] = await parallel([
  () => agent(`${CTX}
Task: verify bundle C and gather every number the records need (brief 7.1 step 8; 4.3; 6.1). Do NOT launch UnrealEditor (another agent is making saves now) and do not edit any repo file.
1. From the console output and the folder: verdict, header facts, F1-F8, B-a..B-e (which B-d sides ran: <BUNDLE_B_CSV> true, <B9_RAN> no, <BR_WORLD> empty - so both sides are breakable; the may-move seeds and the moved count = H:334's '거의'), sha1 guard before/after, the archive (count files, check manifest.sha1 against the files with a short python script), the per-cell table of record kinds.
2. Independently recompute, with your OWN small python scripts reading the raw logs/CSVs (not bundleC's functions): R1 k (#136 first-person claim lines by the record >= 1) for R and slowR; R7 '< 15%' k per cell (record OtherPeople with no vessel 1) for inputless, wanderer, death; R8 record-kind counts for R and slowR; R11 record tick min/median/p90 and the share at the clock for R and slowR; R4 own-vessel counts on/off, L, G and the exact two-sided sign test p; R5 median transfers per run for 0.79 on/off and 0.75; R2 n and k_e from the test cell _claims.csv with the frozen filter; R9 n and k for the pool. Report match/mismatch for each; on a mismatch find the cause (read the harness code) before reporting.
3. List [개발자] items raised (brief 7.2 R6) with numbers and the recommended resolution; the developer accepted recommended options. If a reading's recommendation would be a knob change, say [B-6b11b] would be its own commit in a later session.
Return the structured result.`, { label: 'readings', phase: 'Readings', schema: READ_SCHEMA, effort: 'high' }),
  () => agent(`${CTX}
Task: the hand-check saves (brief 4.4, 7.1 step 9). The selection is printed at the end of ${SP}\\b6b11\\bundleC_run_out.txt under '## Hand checks' (commands included).
- Run each save command ONE AT A TIME with UnrealEditor-Cmd (the printed command; set its -Out and -AbsLog under <repo>\\Saved\\Balance\\b6b_handcheck\\ with names like B6b_<n>_<what>_seed<s>_<t>.csv/.log - absolute paths). Nothing else runs UE now; still check Get-Process UnrealEditor*,UnrealBuildTool is empty before each run (dotnet -Mode=ValidatePlatforms launched by your own run is normal).
- After each run: read the log's '-SaveAt <t> wrote <path>' line; move ONLY that .sav file into <repo>\\Saved\\HandCheck\\B6b_<n>_<what>_seed<s>_<t>\\ (the folder names printed by the selection). You may use bundleC.take_save(log, root, save_dir) (import ${SP}\\b6b11\\bundleC.py with importlib; it refuses unless exactly one wrote line, the file exists under <repo>/Saved/SaveGames and the folder is absent or empty). Never move, delete or open any other file in Saved\\SaveGames.
- Check each expected tick against the log of the bundle cell the seed came from (and against the save run's own log: the same world up to the save tick, so the save run's record/first-at values before the save tick must match).
- Items: ① ruled T4 (save at T4-50), ② other order (record-30), ③ extinction on ruled (first placement-10, note the expected expiry), ⑥(i) era line (first-30), ⑥(ii) witness (none in this bundle if the selection says 없음 - then nothing to run), ④ uses ①'s save, ⑤ no save.
Return a plain report: per item n/what/seed/cell, the exact command, expected tick and what the log showed, the wrote line, the destination file path and size, and any refusal or failure.`, { label: 'handcheck-saves', phase: 'Saves', effort: 'high' }),
])

phase('Docs')
const shared = `Bundle readings (verified by the readings agent): ${JSON.stringify(readings)}
Hand-check saves report: ${typeof saves === 'string' ? saves : JSON.stringify(saves)}`

const docs = await parallel([
  () => agent(`${CTX}
${shared}
Task: write N and H for [B-6b11] (brief 6.1, 6.2, 6.3). Only N and H.
1. N section 3: replace the placeholder line '(비어 있다 — \`[B-6b11]\`이 채운다.)' under '### 묶음 C' with the record in the form of B6a notes section 3 '묶음 1' (B6a N:272-300 form) and of the '묶음 B' record just above it: one long header paragraph (date 2026-11-10; build 4448705 with the DLL sha1 the guard kept; 620 processes plus reruns; failures; folder and archive; end check and race check 0; F1-F8; B-a..B-e with the B-d sides, may-move seeds and moved count; sha1 unchanged; the freeze sha1 of the five files and <FROZEN_SHA1>; <C_MINUTES>; the runner line; the smoke and the aborted first attempt with its cause and the harness fix, in one sentence each), then a per-cell table (seeds x ticks, record kinds), then R1-R11 with numbers and Wilson; record-only items as on/off/(on − off); #129 also 0.75 against 0.79. Use only numbers from the readings/saves reports or w2_context.md.
2. N section 5, inside the [B-6b11] block, AFTER the frozen '- **측정**' item (do not change one byte of it - its sha1 must stay e21d9d832ea362996e99fb81d22d7ef138920e54): add items in the order of brief 6.2 - '나눔: 없다.'; a pointer for 측정 to section 3 '묶음 C' (one short line, separate from the frozen item); '손 확인의 시드' with each command, expected tick, the wrote line and the destination; '기준값 그대로' with the five lines and two hash runs as numbers; '리뷰' with counts by lens (harness review: 52 candidates, 9 kept partial, 43 rebutted, 8 applied, 1 not applied - L2-1; the docs lens counts are added later by the lens agent - leave '문서 관점: (lens)' for it to fill); '헤더와 다르게 하거나 더한 것' with every item of brief 6.2's list plus: the four arithmetic notes (L2-1 merges kept as frozen and counted; L1-2 bridge reading per seed; L2-3 early bucket at or before the record; L2-2 the R9 pool without B5's walking cell and the like-for-like 28/37), the harness fixes L1-1, L1-3, HC-1, HC-2, HC-3, the aborted first attempt (job 324/620, the busy guard and our own commandlets' UBT -Mode=ValidatePlatforms query, fix + 2 self-test checks, whole bundle rerun, folder kept aside and not read), the <BUNDLE_B_CSV> and <A_SEC_PER_20K> facts (bundle B ran on 4 workers), the two drifted cites of the frozen N section 2.3 ('< 15%' target now docs/40_DEV_GUIDE.md section with that text; the 0.25 floor now in Tests/CosmicDataTests.cpp - find the current lines), the [B-6b11b] label vs H:43, and each [개발자] item with its recommended resolution (the developer accepted recommended options on 2026-11-10).
3. H section 11: add one line right after the [B-6b10] line, in the brief 6.3 form, under 400 bytes (target <= 370). Then the size rule: measure H; if H > 149,000 bytes, move the whole body of H's '## 추가 이야기와 보완' section (its items) to N section 0 under a new heading \`헤더 '추가 이야기와 보완'(옮김 — [B-6b11])\` placed at the end of N section 0, and leave in H the heading plus a pointer of about 100 bytes (for example '- 이 절의 항목은 노트 §0 「헤더 '추가 이야기와 보완'(옮김 — \`[B-6b11]\`)」으로 옮겼다(헤더 크기 한도).'). Keep every moved item byte for byte. H must end at or below 149,000 bytes.
4. Run python -I Tools/check_docs.py (repo root) - 0 errors. Check N and H have no CR. Re-hash the frozen item: the exact bytes of SP\\b6b11\\frozen_4_3.md must still occur once in N.
Return: what you wrote where (N line numbers), H bytes before/after, check_docs result, anything you could not fill.`, { label: 'docs:N+H', phase: 'Docs', effort: 'high' }),
  () => agent(`${CTX}
${shared}
Task: the other records (brief 6.4-6.7). Do NOT edit N or H (another agent is editing them now).
1. DL, B6b section: one [B-6b11] line right after the [B-6b10] line (find '- \`[B-6b10]\`(2026-11-10 — 결정 35'). It carries decision 24's #142 rule as written into this section (B4 model interval with the frozen p_e = 1 − (1 − 0.02·1.2)^47.5 = 0.6846 and the counted-claim filter; it replaces B6a decision 12's rule) with <N142>; the B1 re-judgment <B1> (closes the DL line that says 'B6b가 B6a의 마지막 빌드와 짝을 지어 다시 판단한다' through the same-binary pair plus the decision 36 bridge); the numbers #136, #132, #129 (0.75/0.79), #154, '< 15%', ends under the clock, #10, #158③, <RUNLEN>; each [개발자] item with the recommended resolution taken (developer, 2026-11-10: recommended options). Plain Korean, the style of the neighbouring lines.
2. OQ #10 (the row whose text has '44/59 = 74.6%'): rewrite that B5 sentence with C's per-death numbers from R9 (and B5's like-for-like 28/37 as the comparison, saying B5's 44/59 included the walking cell), keep '도약의 길이 자체는 Phase 5 그대로' (or its exact current wording). Change nothing else in OQ.
3. STATUS.md: '지금' (the [B-6b11] result in one paragraph at the top of the current-state text, in the existing style); '다음 3작업' item 2 -> next is [B-6b12] (to be run in Antigravity on this PC by the developer's choice; brief SCR\\b6b12) and the B6b hand checks with their seeds and save folders (from the saves report); Top 5 item 1 (#136 with C's number), item 2 (C's #158③ in place of #135's '재지 않음 — 잰 15'), item 3 (<RUNLEN> instead of '7/30' and the B6a run length), item 4 (C's #129 transfer numbers next to B3's). Also one line saying the GitHub-only work (B7x rebase onto main, docs drafts) moves to a cloud Claude Code session on branches cloud/*, and that the hand-off notes are on origin/handoff/cloud. Keep STATUS's structure and size discipline.
4. docs/CHANGELOG.md: the journal entry for [B-6b11] in the existing form (date, what ran, results, review counts: harness review 52/9/43, docs lens counts '(lens)' placeholder for the lens agent to fill), at the place the latest entries go.
5. python -I Tools/check_docs.py must report 0 errors; no CR in edited files.
Return: what you changed (file, section, line numbers) and anything not done.`, { label: 'docs:DL+OQ+STATUS+CHANGELOG', phase: 'Docs', effort: 'high' }),
])

phase('Lens')
const lens = await agent(`${CTX}
${shared}
Docs writers' reports: ${JSON.stringify(docs)}
You are the docs lens (brief 7.1 step 11) and you may fix what you find. Check, over the diff (git --no-optional-locks diff) of N, H, DL, OQ, STATUS.md and docs/CHANGELOG.md:
- bytes: H <= 149,000 (and that the moved '추가 이야기와 보완' items, if moved, are byte for byte in N section 0); CLAUDE.md untouched;
- verbatim carry-over: N section 2.3 untouched; the frozen item still byte-identical to ${SP}\\b6b11\\frozen_4_3.md (sha1 e21d9d832ea362996e99fb81d22d7ef138920e54) and occurring once;
- placeholders: no '<C>', '<J136>', '<N142>', '<B1>', '(lens)' etc. left (fill '(lens)' with your own counts: findings found / fixed / rebutted);
- no invented numbers: every number in the new text traceable to the readings/saves reports, w2_context.md or the bundle output (spot-check at least 25 numbers against ${SP}\\b6b11\\bundleC_run_out.txt and the bundle folder);
- consistency: the same number the same everywhere (N section 3, N section 5, DL, OQ, STATUS, CHANGELOG, H section 11 line); plain words; no verdict words (천국·지옥·구원받·멸망받·영혼·저주받·심판받) in any new text; no CR; UTF-8 without BOM;
- run the five python checks from the repo root: python -I Tools/check_data.py; python -I Tools/test_data_schema.py; python -I Tools/check_layers.py; python -I Tools/make_config_reference.py --check; python -I Tools/check_docs.py - all must pass.
Then write the commit message to ${SP}\\b6b11\\commit_b6b11.txt (UTF-8, LF): title in the brief 6.8 form filled with the real numbers ('[B-6b11] 다시 잼(묶음 C) — 시계 9,000의 빌드로 620판, #136 <k>/60·#142 <...>·B1 짝 L <L>/G <G>(p <p>)·#129 0.75/0.79·#10·#158③, 손 확인 시드와 세이브, 비트 그대로' - keep it one line), a blank line, a body of short Korean lines: the runner result (227 x 2 passes, 0 failures, five lines and hash runs unchanged), the harness and its review counts, the aborted first attempt and the fix, the bundle result (controls, bridges), the key readings, the [개발자] items with the recommended resolutions taken, the hand-check saves, the docs lens counts; end with the line 'Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>'.
Return the structured result.`, { label: 'docs-lens', phase: 'Lens', schema: LENS_SCHEMA, effort: 'high' })

return { readings, saves, docs, lens }
