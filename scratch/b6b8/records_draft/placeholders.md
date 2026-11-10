# [B-6b8] placeholders <<B:NAME>> - where they are and what fills them (2026-11-10, records stage)

Files (project root): N = docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race_notes.md, H = docs/60_DECISIONS/headers/
B6b_upper_rungs_and_the_race.md, D = docs/60_DECISIONS/DECISIONS_LOG.md; C = scratchpad/b6b8/commit_b6b8.txt;
CL = scratchpad/b6b8/records_draft/changelog_line.md. Source of the values: bundleB.py's summary.md / summary.json
(sections '판단', 'N §3 자리', '적기만').

| Name | Where | Fill with |
|---|---|---|
| B_PROCS | N:743, D:42, C, CL | 555 if no rerun (540 + 15), else 540 + reruns + 15 |
| B_RERUNS | N:743 | reruns in the bundle (number) |
| B_FAILED_TWICE | N:743, D:42, C | '없음' or the seeds per cell |
| B_FOLDER | N:743 | the bundle's absolute folder (brief: scratchpad\b6b8\bundleB\) |
| B_CHECKS | N:743 | '0·0·0 (555 모두)' |
| B_CONTROLS | N:743, C | '통과' or what failed |
| B_CLAIMS_CONTROL | N:743 | whether `_claims.csv` lists every placed claim (else 'dropped - say so') |
| B_PAIRS | N:743, D:42, C, CL | e.g. '240/240 통과' |
| B_BRIDGES | N:743, C | e.g. '120/120 통과' |
| B_SHA1 | N:743, C | 'unchanged (14 files)' |
| B_MINUTES | N:743, D:42, C | measured wall minutes |
| J1, J1_VERDICT | N:745 (J1 also N:907, D:42, C, CL) | 'k' (count only - '/60' follows) and '통과/실패' with Wilson |
| J1_WILSON | C | Wilson interval of J1 |
| N2, J2, J2_VERDICT | N:746 (J2 also N:907, D:42, C, CL) | n2, 'k/n2 = p [lo, hi]', verdict |
| SPLIT | N:746, N:907, D:42, C | (가) a (binder = record people b), (나) c, (다) d (+ fields of (다)) |
| ONCE_RULED | N:746 | once-ruled other orders with seeds |
| J2_SLOW | N:746, C | slowR share, interval, split |
| N3, J3_LEN, J3_DENSITY, J3_VIOLATIONS, J3_ZERO, J3_APART, J3_VERDICT, J3_BY_RESETS | N:747 (J3_LEN/J3_DENSITY also N:907, D:42, C, CL) | judgment ③ and the L4-2 split |
| E_LIST, C0, C, J4_I, J4_II, J4_VERDICT | N:748 (C0/C/J4_I/J4_II also N:907, D:42, C; C also CL) | judgment ④ |
| REC_T4, REC_EXTINCTION, REC_BINDING, REC_162, REC_156, REC_141 | N:750-755 | record-only, on / off / on - off per cell |
| B_AI_US | N:756, N:907, C | AI-stage us per tick (serial runner + bundle) |
| B_LOCKED | N:757, N:907, D:42, C | R runs locked >= 1 with no fifth-rung record (and the #165 note if >= 3) |
| REC_96, REC_WORKLOAD, REC_B1, REC_DEATH, REC_SHORT_SHAPE | N:758-762 | record-only |
| SHOT | N:902 (item 11, a whole sentence ending with '.') | the windowless shot's result, or why it was not taken |
| HANDOFF | N:907, D:42, C, CL | hand-offs to [B-6b9] / [개발자] (or '없음') |
| D_156, D_141 | D:42, C | short: era-line runs / witness runs (e.g. 'R 3/60'), or '재지 않음 — 잰 0' |
| H_J1, H_J2, H_J3, H_C | H:473 only | SHORT forms: the four together <= 42 bytes UTF-8 (e.g. '45', '27%', '2,524·93%', '12,000'); the filled line must stay < 400 bytes (check with wc -c); H stays < 149,000 |
| TITLE_VERDICTS | C (title) | e.g. '① 통과·② 통과·③ 실패·④ C 12,000' |
| SHOT_SHORT | C | one clause on the shot |

If era and witness are 0 over all 240 on-runs (brief risk 4): REC_156 / REC_141 / D_156 / D_141 = '재지 않음 — 잰 0',
record the longest PressureEnduredTicks stretch if cheap, and raise a [개발자] question in N section 5 (a new
OPEN_QUESTIONS number would be #168 - #164-#166 are [B-6b12]'s, #167 is taken).
