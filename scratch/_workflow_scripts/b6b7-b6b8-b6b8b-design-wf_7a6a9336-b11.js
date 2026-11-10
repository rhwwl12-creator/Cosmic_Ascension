export const meta = {
  name: 'b6b7-b6b8-b6b8b-design',
  description: 'Read-only designs of [B-6b7] (data 2: laws, fourth->fifth feat, binding values), [B-6b8] (data 3: era row, enc_witness, bundle B and the section 5.3 judgments) and [B-6b8b] (#132 share 0.5) in parallel',
  phases: [
    { title: 'Map', detail: 'read-only mappers per commit' },
    { title: 'Design', detail: 'one designer per commit' },
    { title: 'Critique', detail: 'adversarial critics per commit' },
    { title: 'Revise', detail: 'final brief per commit' },
  ],
}

const SCR = 'C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad'
const EFF = 'high'

const READONLY = `
STRICT READ-ONLY. Other workflows are running: [B-6b4] (presentation) is editing, building and mutation-testing this working tree, and
the [B-6b5] text audit is reading it. Do NOT edit, write, create or delete any file in the project; do NOT run Build.bat, UnrealEditor,
Tools/run_checks.ps1, make_data_tables, a mutation, git apply/checkout/stash/reset/add/commit, or anything that writes. Read the TRUE
committed content with \`git show HEAD:<path>\` (HEAD = cef8cc7, [B-6b3b]) - never the working tree. The commits before yours are
designed in these briefs (outside the project - read them directly where your commit builds on them):
  [B-6b4] presentation (UI, ST_CA keys, UI tests; bit-identical): ${SCR}/b6b4/brief_final.md
  [B-6b5] text audit (texts in place, schema word lists, the audited drafts of the rows [B-6b6]-[B-6b8] turn on in notes 1.5; its
          merged texts will be in ${SCR}/b6b5/merged.md): ${SCR}/b6b5/brief_final.md
  [B-6b6] data 1 (extinction row, project_ark, feat_passage_by_ark, decision 5 pins, bundle A - FIRST world move, new baselines, the
          notes section 2 recomputation of N, P_bind, r and the forced death tick t): ${SCR}/b6b6/brief_final.md
Values that only bundle A / bundle B will produce must be written as named placeholders with the exact rule that fills them (where the
number comes from, which file:line receives it) - never invent them.
Context: UE 5.8.3 C++ project Cosmic Ascension (CLAUDE.md is loaded for you). Milestone B, block B6b. Header
docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md (H) and notes B6b_upper_rungs_and_the_race_notes.md (N); H section 8 table
(~:385-400) gives each commit's scope, effort and checks. Standing speed rules (developer 2026-11-09, N section 5 [B-6b3b] '일의 속도'):
runner -Tier full once per commit (its two in-process passes are the two consecutive test runs); mutations = new ones plus old ones
whose patched file or filter test the commit touches; the whole-B6b mutation rerun only at [B-6b12]; measurement bundles run in parallel
processes (the [B-6b6] brief section 4.2 harness). Theology guardrails (standing): the holy order (God) is never an entity, speaker,
number, lever or ending condition; first person only for fallen transcendents; no soul-counting; no saviour titles, no 'became god';
no new verse; never read anything under C:/Users/rhwwl/Downloads/jehoba. Answer in English; quote Korean as-is; cite file:line at HEAD.`

const COMMITS = [
  {
    key: 'b6b7',
    title: "[B-6b7] data 2 fourth -> fifth: the three galaxy-law edict rows (edict_wide_machine_accord, edict_wide_quarantine, edict_wide_warning), feat_laws_and_binding, project_binding's values, the first values of N, P_bind and r (AIUpperRungProgressMultiplier 1 -> start value 0.375 per H:43), race check 0, docs/30",
    maps: [
      `Requirements: every H and N sentence about [B-6b7] (git grep -n "B-6b7" HEAD -- docs Source Tools Config; H sections 0, 2.4, 2.5, the rows table ~:246-252, 3.x knobs ~:285-295, 5 (what bundle A hands to [B-6b7], H:325-330), 6, 7, 8, 11; N sections 1.3, 1.5 (drafts of the law rows and the fifth-rung feat), 2 (the pre-computation of N, P_bind, r - what formula, which inputs come from bundle A), 5 blocks' hand-offs to [B-6b7]); decisions 9-15. Return a numbered checklist with file:line and the file each item changes.`,
      `Machinery: DT_Edicts (Scope column ECosmicEdictScope from [B-6b2a], how galaxy laws count, Data/CosmicDataTables.h:119-120 order comment, the edict window order UI/CosmicEdictWidget.cpp ~:689-698), the AI law step and AIRevokesOf exception, GalaxyLawsToHold and Data.Integrity's slot-end/HomeGalaxy-row pins, DT_Feats feat_laws_and_binding conditions (CivLawsHeldTicksAtLeast? MegaprojectComplete project_binding? NoFallen?), project_binding's current start values in DT_Megaprojects and what [B-6b7] changes, AIUpperRungProgressMultiplier where read, FourthRungOpenOf / race check, CosmicB6bRowNames constants that move, WithoutB6bData's row list, every test pinning absence of these rows, schema rules and test_data_schema cases, docs/30 regeneration (Tools/make_config_reference.py), mutation patches whose context these files touch. Precise file:line facts and the edit list.`,
      `Determinism and measurement: how [B-6b7] moves the world after [B-6b6] (which draws/steps the new rows feed: AI law step, edict multiplication order, the fifth-rung end via EndingOf), what bit-identity remains (the -WithoutB6bData pair reproducing the [B-6b5] lines; pairs equal up to which tick), how baselines are re-recorded (Tools/baselines/seed42.json, run_checks -UpdateBaseline - per the [B-6b6] brief section 3.2), race check, H section 5.4 for this commit, and how N/P_bind/r first values are computed from bundle A outputs (N section 2 formulas; the race: line parts and _claims.csv columns they read). Precise facts and a checklist.`,
    ],
  },
  {
    key: 'b6b8',
    title: "[B-6b8] data 3 side rows: event_era_endured (decision 22 answer ㉮, Borders key, slot order by name - H:251) and enc_witness (decisions 20-21, the witness card and its choices - H:252), then bundle B (H:330 - R 60, slow R 60, wanderer 60, input-less 60 x 20,000 all paired, forced death R 60 + -DeathAt=<t>) and the pre-written judgments of H section 5.3 (1-4)",
    maps: [
      `Requirements: every H and N sentence about [B-6b8] (git grep -n "B-6b8" HEAD -- docs Source Tools Config; H sections 2.7, 2.8, rows table ~:251-252, 5.2-5.4 (bundle B and the judgments 1-4 - exactly what is judged, thresholds, what each outcome triggers: [B-6b9] values, [B-6br], etc.), 6, 7, 8 row, 9 hand checks (6 after [B-6b8]), 11; N 1.3, 1.5 (era and witness drafts), 2, 3 (bundle B), section 5 hand-offs); decisions 20-22. Numbered checklist with file:line and the file each item changes.`,
      `Machinery: DT_Events event_era_endured (conditions from [B-6b2a]: PressureEnduredTicks, selector Ruled, key Borders; slot order by name Data/CosmicDataTables.h:102-103; the +0.2 effect), DT_Encounters enc_witness (B1 wanderer cards as models, Events/CosmicPetitions.h:161 one crisis per system, CardsByRow/AnswersByRow/WithdrawnByRow policy counts from [B-6b3b], the +0.1 effect), CosmicB6bRowNames constants, WithoutB6bData, tests pinning absence, schema rules and test cases, the -DeathAt switch (exists? Tests/CosmicBalanceCommandlet.cpp at HEAD) and the slow-R policy switch, the measurement harness for 240 x 20,000 + 60 forced-death runs (reuse ${SCR}/b6b6/brief_final.md section 4.2), mutation patches whose context these files touch. Precise file:line facts and the edit list.`,
    ],
  },
  {
    key: 'b6b8b',
    title: "[B-6b8b] #132: FleetDelayCountdownShare 0 -> 0.5 and recompute W (decision 23, H section 2.10 ~:217-219; Data.Integrity; Sim.FleetDelay gains the awakening case 150 -> 225 with fleet 3)",
    maps: [
      `Everything for [B-6b8b]: H 2.10, the knob row ~:291, section 8 row, decision 23, N's W computation and hand-offs, CosmicConflict::GuardDelayCapOf and its callers (FleetDelayOf, the theodicy invariant), DT_Rules entry and schema range [0,1] and its test, Data.Integrity's W pin, Sim.FleetDelay at HEAD, docs/30 regeneration, how the world moves (only guarded awakening crises - R uses -Guard=0, World/CosmicRulerPolicy.cpp ~:686-720) and the baseline re-record, which mutation patches touch these files. Numbered checklist with file:line plus the edit list.`,
    ],
  },
]

const designPipeline = async c => {
  const maps = await parallel(c.maps.map((m, i) => () => agent(`${READONLY}\n\nCOMMIT: ${c.title}\n\nTASK (mapper ${i + 1}): ${m}`,
    { label: `map:${c.key}#${i + 1}`, phase: 'Map', effort: EFF })))
  const mapText = maps.map((m, i) => `=== MAP ${i + 1} ===\n${m || '(empty)'}`).join('\n\n')
  const brief = await agent(`${READONLY}\n\nCOMMIT: ${c.title}\n\nYou are the designer. Write the implementation brief the lead will run as a lean
ultracode workflow when this commit's turn comes (implementers -> review lenses with a rebut-verify per finding -> fix -> one runner
full -Data; measurement bundles inside). Verify every claim you rely on at HEAD. Order: 0 start state (what to re-check on the tree
after the earlier commits, placeholders filled from which outputs); 1 data rows in full JSON (texts verbatim from the audited N 1.5
drafts - name the source); 2 code/tests/constants and every test whose meaning changes; 3 determinism and baselines; 4 measurement
(if any) and the pre-written judgments; 5 mutations (new + touched only) and retakes; 6 docs and records (H section 11 line < 400
bytes, header < 149,000 bytes, N section 5 block, DECISIONS_LOG, docs/20/21/30/50, STATUS, commit message); 7 order of work and
risks with recommended resolutions (recommended = approved; [개발자] for developer-only items); 추가 이야기와 보완.\n\n${mapText}`,
    { label: `design:${c.key}`, phase: 'Design', effort: EFF })
  const critics = c.key === 'b6b8b'
    ? ['completeness, correctness of every file:line, determinism and the baseline re-record']
    : ['completeness and scoping against H/N and the earlier briefs (missed rows, tests, mutations, docs; wrong commit)',
       'determinism, balance, measurement (can every check fail? pairs, hashed fields, RNG draw points fixed before rows - rule 3) and the theology guardrails in every row text']
  const crits = await parallel(critics.map((k, i) => () => agent(`${READONLY}\n\nCOMMIT: ${c.title}\n\nAdversarial critic (${k}) of the brief
below. Return a numbered list: severity, claim, evidence (file:line at HEAD), exact correction.\n\n=== BRIEF ===\n${brief}`,
    { label: `critique:${c.key}#${i + 1}`, phase: 'Critique', effort: EFF })))
  const final = await agent(`${READONLY}\n\nCOMMIT: ${c.title}\n\nYou are the reviser. Fold every critique finding you verify into the brief (or
reject it with a one-line reason). Return the FINAL brief (complete, same order), then 'Critique dispositions'.\n\n=== BRIEF ===\n${brief}\n\n${crits.map((x, i) => `=== CRITIQUE ${i + 1} ===\n${x || '(none)'}`).join('\n\n')}`,
    { label: `revise:${c.key}`, phase: 'Revise', effort: EFF })
  return { key: c.key, final }
}

const out = await parallel(COMMITS.map(c => () => designPipeline(c)))
return Object.fromEntries(out.filter(Boolean).map(o => [o.key, o.final]))
