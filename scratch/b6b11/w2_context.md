# [B-6b11] facts for the readings, saves and docs agents (written by the lead, 2026-11-10)

Repo root: C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension (main HEAD 4448705 [B-6b10], pushed).
SP = C:\Users\rhwwl\AppData\Local\Temp\claude\c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension\37de2919-0c44-45d8-94cd-4dcf0860263e\scratchpad
Brief: SP\b6b11\brief_final.md (sections 0-7; line cites were read at 10205b6 and have drifted - find spots by text).
H = docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md (148,990 bytes at the start), N = its _notes.md,
DL = docs/60_DECISIONS/DECISIONS_LOG.md, OQ = docs/60_DECISIONS/OPEN_QUESTIONS.md, cmd = Source/Cosmic_Ascension/Tests/CosmicBalanceCommandlet.cpp.

## Start values (brief 0.2 / 0.4)
- <BUILD_SHA> 4448705; <B9_RAN> no (the 3 grep hits are mentions inside the bodies of [B-6b6], [B-6b8], [B-6b10]; no commit is titled [B-6b9]);
  <BR_WORLD> empty ([B-6br] 5ed07e1 bit-identical); <K_MOVED>: R none, wanderer none (N section 5 [B-6b8b], bundle K K3 0/60 and 0/60).
- C 9,000; r 0.375; share 0.5; W_EASED 1.2; CHANCE 0.02 (EventSlotTicks 10); RATIO 0.79; T_DEATH 5,000 < C, so the death cell is kept: 620 jobs.
- <A_SEC_PER_20K> 15.9 s (bundle B: 35.9 min, 4 workers - not 6 - 541 attempts). <BUNDLE_B_CSV> true (1,620 csv in SP\b6b8\bundleB, logs match Saved\Balance\b6b_bundleB\manifest.sha1).
- <BASE_B6b10> (Tools/baselines/seed42.json): 3,000 2557850423 / 2906170550; ruler 3,000 1189870756 / 3359656751; wanderer 6,000 2158489212 / 1214038630;
  input-less 20,000 4192287602 / 2387995655; ruler 20,000 1375149537 / 510604211; the two game hash runs equal the two 3,000 lines.

## What ran, in order
1. Runner `-Tier full` once, 2026-11-10 15:55-16:05, on the tree as is (no -Data, no -UpdateBaseline): "run_checks: all passed";
   227 automation tests x 2 passes = 454 successes, 0 failures; smoke ok; the five lines and the two hash runs equal <BASE_B6b10> (bit-identical);
   stage times of the ruler 20,000 line: AI 8.02 us, economy 2.62 us, crisis 23.32 us; free memory 10.6 GB. Output: SP\b6b11\runner_full.txt.
2. Harness check and review workflow (16:00-16:35):
   - self-test 197/197 at the start (also against cmd at 4448705: SP\b6b11\cmd_4448705.cpp);
   - `--check-only Saved\Balance\b6b_bundleB --cell-set B`: 540/540 logs read, 0 unreadable, 0 run failures, may-move {} (no ON log of bundle B has a Hazard.AIAwakening delay sum > 75; the only one is wanderer_60_off = 100, an off log), PASS;
     B-c over bundle B's wanderer_off CSVs vs b6a_bundle1 60/60; B-e on bundle B off logs 40/60, median 1,350, paths 22/15/3 (= B6a bundle 1);
   - dry run 620 jobs (300 at 9,000, 320 at 20,000; ProvisionalEndTick in no argument; 420 jobs with no -Rules, 200 with one);
   - review, three lenses (measurement and controls; frozen statistics; hand checks and form): 52 candidates, 43 rebutted, 9 kept (all 'partial'):
     L1-1 a rerun deleted the first attempt's log (now kept as <name>.try1.log and its end/race check counts are added to the header);
     L1-2 one twice-failed off run blocked all of B1 (now that seed is left out and listed; a broken bridge still stops R4; B-e is checked seed by seed against B6a's own logs plus the 40/1,350/22-15-3 totals);
     L1-3 the memory guard never acted at 6 workers (now 0.8 GB per worker must be free or the worker count drops);
     L2-1 R10 still merges a death key after T_DEATH or a wanderer key after 1,200 with an R key of the same (seed, tick, system) - NOT applied: it would change the frozen R10 rule; merges are counted and printed ('merged after the boundary');
     L2-2 R9's pool (wanderer + input-less) leaves out B5's walking cell, so the harness also prints B5's like-for-like figure 28/37 = 75.7% next to 44/59;
     L2-3 R6's 'early' bucket now counts only successions at or before the record;
     HC-1 the '-SaveAt <t> wrote' parser also matched the failure line 'wrote no slot' (now only real .sav paths; helper take_save(log, root, save_dir) moves exactly that file);
     HC-2 pick (3) now prefers the smallest seed whose countdown ends by its record; expected texts include the record;
     HC-3 an aborted or sha1-discarded bundle is not archived.
     Applied 8 of 9; self-test 221/221 after the fixes.
   - Freeze: the frozen arithmetic (brief 4.3) was written into N section 5 as the first item of the new [B-6b11] block (block head line `**`[B-6b11]`**(2026-11-10 — 다시 잼(묶음 C)):`, item '- **측정**...') and saved byte for byte to SP\b6b11\frozen_4_3.md (8,703 bytes, sha1 e21d9d832ea362996e99fb81d22d7ef138920e54). Do not change one byte of that item.
3. Smoke before the bundle: seed 1 of every cell (9 runs, --max-seed 1 --no-archive), PASS; its folder is SP\b6b11\smoke (not part of the record).
4. First full bundle C, 16:37-16:48: ABORTED at job 324 of 620 (643 s) - the per-job busy guard saw 'dotnet ... UnrealBuildTool.dll -Mode=ValidatePlatforms', which every commandlet start launches itself ('Launching UnrealBuildTool... Build.bat -Mode=ValidatePlatforms ... AutoSDKInfo.txt' is in all 540 bundle B logs and in all 328 logs of the attempt). sha1 guard unchanged; not archived (HC-3). Folder kept aside: SP\b6b11\bundleC_abort1 (not used for any reading).
   Harness fix: the busy check drops dotnet lines with -Mode=ValidatePlatforms (a UBT build or another editor still counts); two new self-test checks (223/223; with the fix reverted 2 fail).
5. Second full bundle C (the record): started about 16:52, folder SP\b6b11\bundleC, archive Saved\Balance\b6b_bundleC; console output SP\b6b11\bundleC_run_out.txt (it prints the freeze sha1 of bundleC.py, bundleC_selftest.py, bundleB.py, bundleA.py, frozen_4_3.md before job 1).

## Rules for every agent
- Docs only: no Source/, Tools/data, ST_CA.csv, Config or Content change; no build; no runner. LF line ends, UTF-8 without BOM; check that no edited file gained a CR.
- Never call plain `git status`; use `git --no-optional-locks diff --stat` etc.
- Do not touch docs/OVERVIEW.html (untracked, the developer's) or docs/90_LORE/cosmic_species_codex.md.
- The developer said on 2026-11-10: any decision that needs the developer is taken with the recommended option. Record each [개발자] item with its numbers and the recommended resolution; nothing blocks the commit. A knob change would be its own commit [B-6b11b] (brief 7.2 R8) - it is NOT made in this session; record it for the next session if a reading calls for it.
- After this commit the developer moves [B-6b12] to another tool (Antigravity) on this PC, and the GitHub-only work (B7x rebase, docs drafts) to a cloud Claude Code session. STATUS.md must say so plainly.
- Plain Korean in docs, the style of the neighbouring blocks; no coined shorthand; numbers only from the bundle output or this file - never invented.
- The five editor-free Python checks on this PC (Git Bash 'python3' is a Store stub - use 'python'): python -I Tools/check_data.py; python -I Tools/test_data_schema.py; python -I Tools/check_layers.py; python -I Tools/make_config_reference.py --check; python -I Tools/check_docs.py.
