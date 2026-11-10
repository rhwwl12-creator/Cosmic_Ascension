export const meta = {
  name: 'b7x-data-port-prep',
  description: 'Prepare (not commit, not measure) the B7x data port for after B6b: two idempotent scripts that write [B-7x5s] (knob values, seed bricks) and [B-7x5c] (twelve crossroads cards with the audited texts and the lead rulings, nine event_brick rows, Society on arch_seeker, schema rules and tests) into a tree; self-tested on a scratch export of integrate/b7x',
  phases: [
    { title: 'Write', detail: 'port_b7x5s.py and port_b7x5c.py' },
    { title: 'Test', detail: 'apply to a scratch export twice, five Python checks, diff review' },
  ],
}

const SCR = 'C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad'
const INT = SCR + '/b7x/int'
const OUT = SCR + '/b7x/port'
const TEXTS = SCR + '/b7x/stage3/texts/audited_texts.md'
const RULINGS = SCR + '/b7x/stage3/texts/lead_rulings.md'
const REN = SCR + '/b7x/answers/rename/rename_leaves.py'

const CONTEXT = `
Project: Cosmic Ascension (UE 5.8.3; data source of truth Tools/data/*.json, ST_CA.csv, Tools/data/schema.json; checker Tools/cosmic_data_schema.py, unit tests Tools/test_data_schema.py). Block B7x is being integrated on the local branch integrate/b7x in worktree INT = ${INT} (HEAD db9bf9f: fields, mechanism, measurement, presentation and text rules - compiled and green, but NO B7x data yet). The B7x data must not be committed or turned on before block B6b ends (header section 8 order; it will be rebased onto the final main first). So this task only PREPARES two scripts and proves them on a scratch export - nothing is committed, INT is not modified, nothing is built or measured.
Sources (read-only):
- the branch's data draft commit aa09543 (git -C "${INT}" show aa09543:Tools/data/DT_Rules.json, DT_Encounters.json, DT_Events.json, DT_Civs.json, schema.json; git -C "${INT}" show aa09543 -- Tools/test_data_schema.py for its tests) - it uses the OLD leaf names; the rename script ${REN} maps them (or apply its mapping table yourself);
- the B7x header and notes in INT (docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md section 3, section 8 rows [B-7x5s]/[B-7x5c]; notes 5.4 '[B-7x5s]' 데4 and '[B-7x5c]' 데1/데2/데5, and the schema hand-offs);
- the audited texts ${TEXTS} (cards section 1, events section 2, schema rules section 7) and the lead's rulings ${RULINGS} (they WIN: voice tilts kept per main's convention; choice-2 effects exactly as the rulings' JSON; C10 title '손대지 않은 칙령', C12 title '집을 싣는 배', C12 choice 2 keeps ChangeReverence +0.05 with its original success line).
Write only under OUT = ${OUT}.
`

const OUTS = {
  type: 'object',
  properties: {
    status: { type: 'string', enum: ['ok', 'partial', 'blocked'] },
    summary: { type: 'string' },
    files: { type: 'array', items: { type: 'string' } },
    numbers: { type: 'array', items: { type: 'string' } },
    open_points: { type: 'array', items: { type: 'string' } },
  },
  required: ['status', 'summary', 'files', 'numbers', 'open_points'],
}

phase('Write')
const writers = await parallel([
  () => agent(`${CONTEXT}

YOUR TASK (effort high): write ${OUT}/port_b7x5s.py - usage 'python -I -X utf8 port_b7x5s.py <tree-root> [--dry-run]'. It writes the [B-7x5s] share of aa09543 into the tree: the seventeen knob values in Tools/data/DT_Rules.json (the branch's starting values from header section 3.4's second paragraph), BrickSeedTags (eight seed bricks, NEW leaf names), and the variant/mutation knobs aa09543 sets; any schema.json / test_data_schema.py lines aa09543 adds for these knobs (and the seed-tag rule of header 3.3 rule 6 if aa09543 has it). Rules: edit the JSON by key, not by line number (the tree will be main after B6b, with other values around); keep the file's exact formatting style (indentation, key order, LF, no BOM) - read how Tools/data files are formatted; idempotent (a second run changes nothing and says so); refuse with a clear message if a key exists with a different value than both the inert default and the target. Print every change.
Return status, summary, files, numbers (knob -> value list), open_points.`, { label: 'write:5s', phase: 'Write', schema: OUTS, effort: 'high' }),
  () => agent(`${CONTEXT}

YOUR TASK (effort high): write ${OUT}/port_b7x5c.py - usage 'python -I -X utf8 port_b7x5c.py <tree-root> [--dry-run]'. It writes the [B-7x5c] share: the twelve crossroads rows into Tools/data/DT_Encounters.json (rows and effects from aa09543 with NEW leaf names; every text field replaced by the audited texts' JSON blocks; the effect lists changed exactly as the lead's rulings say - decision 11 (나), C10 choice 2 without Exaltation/Reverence/tilt, C05/C07/C11 choice 2 as the rulings' JSON, C12 choice 2 kept; append rows at the end of the table), the nine event_brick rows into DT_Events.json (aa09543 rows, audited texts), Brick.Form.Society added to arch_seeker's Traits in DT_Civs.json, the schema.json rules aa09543 adds for these rows (header 3.3 rules 4 and 5 - the lacks_cond lists sorted alphabetically by tag with the new names; rule 5's why text as notes 5.4 데1 gives it) plus the row-name word rules of the audited texts section 7.3 that need the rows (event_brick_shut_star, _drift_plenty, _ember_wake), rule_setups/fixture_tables entries aa09543 adds, and the tests aa09543 adds to Tools/test_data_schema.py (with the walked pin left for the test run to recount - print the instruction). Same rules as any data port: by key/row name not line number, the files' exact formatting, idempotent, refuse on conflicting existing rows, print every change. Keep the Korean text byte-exact from the audited blocks (UTF-8).
Return status, summary, files, numbers (rows, rules, tests added), open_points.`, { label: 'write:5c', phase: 'Write', schema: OUTS, effort: 'high' }),
])

phase('Test')
const test = await agent(`${CONTEXT}

Writers: ${JSON.stringify(writers)}
YOUR TASK (effort high): prove the two scripts on a scratch export, never on INT:
1. git -C "${INT}" archive --format=tar db9bf9f | tar -x -C ${OUT}/try1 (create it). Run port_b7x5s.py then port_b7x5c.py on it; run both again (must report no change).
2. In the export: python -I -X utf8 Tools/check_data.py (0 problems expected), Tools/test_data_schema.py (expect failures only from the walked pin - recount it by running the test, set the pin in the export and re-run: OK), Tools/check_layers.py --strict, Tools/make_config_reference.py --check (regenerate docs/30 in the export if knob values appear there and say so), Tools/check_docs.py (the export path may be long - use a short junction or copy if it fails on length).
3. Diff the export against db9bf9f (git diff --no-index or a Python diff): check every Korean text equals the audited block (after the lead's rulings), every effect list equals the rulings, no old leaf name anywhere (rename_leaves.py --dry-run on the export prints 0), formatting matches the files' style, LF/no BOM.
4. Write ${OUT}/port_check.md with the results and the exact commands the re-stack session must run after B6b (order: rebase, port_b7x5s.py -> build? no: data only -> run_checks -Data in its commit; port_b7x5c.py in its own commit).
Fix the scripts if a check fails (you may edit ${OUT}/*.py) and repeat on a fresh export try2.
Return status, summary, files, numbers, open_points.`, { label: 'test', phase: 'Test', schema: OUTS, effort: 'high' })

return { writers, test }
