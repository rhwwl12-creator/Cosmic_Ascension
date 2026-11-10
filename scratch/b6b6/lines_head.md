# [B-6b6] line map on [H''] (HEAD 10205b6 = [B-6b5] on 529cab9 [B-6b4]) - brief section 0.4

The brief's line numbers are cef8cc7's. Below: first what moved or reads differently on [H''] (act on these), then the
mechanical map of every cited line (git diff -U0 cef8cc7 HEAD hunks; "same text" = the line at the new number is the
cited line), then the rerun set of mutations. Re-find every edit by its text anyway.

## A. What moved or differs on [H''] (the ones that matter)

### Data
- `Tools/data/DT_Hazards.json`: unchanged since cef8cc7 - `crisis_rebellion` still ends at :261 (file ends :262 `]`),
  `crisis_pre_supernova` :145-194, Draw shares :9 (0.0002), :109 (0.0001), :152 (0.0001) - sum 4e-4 as the brief says.
- `Tools/data/DT_Megaprojects.json`: [B-6b5] changed texts only; line numbers hold - `project_binding` ends :73,
  `project_stabilize` cost :54 (2/2), P_bind :67 = `"RequiredProgress": 1800.0` (the lead's correction; not 1,200).
  `project_binding`'s DisplayName is now '항성 기관'.
- `Tools/data/DT_Feats.json`: texts changed, lines hold - `feat_three_worlds` ends :66; TargetTier :7 Tier2, :29 Tier3, :50 Tier3.
- `Tools/data/DT_Rules.json:60` TierMultiplier [1.0, 1.5, 2.2, 3.0, 3.0] - unchanged.
- N section 1.5 is at N:473-527 on [H''] (the brief's "N:469-471 empty" is cef8cc7's). C1 = N:477-484, C2 = N:486-491,
  C3 = N:493-498. In N the pipes of `|hpp(...)` are written `\|hpp(...)` (markdown escape) - the JSON cells take `|hpp(`.

### Schema (`Tools/data/schema.json` - +27 lines at :28, +43 by :870; see the map)
- `row_name_sources` :28 -> :29; `words.axis` :34 -> :35; `holy_and_myth` :44 -> :45 (57 names, ends '마라나타'; no '노아').
- `DT_LogTemplates.Templates` forbidden :326 -> :345; `DT_Events.LogTemplates` :450 -> :472.
- `DT_Hazards.DisplayName` :697 -> :724, still the string `"verdict"`; Outbreak/Expire/Resolve :711-713 -> :738-740;
  `unique` HazardTag :716-718 -> :743-745; rule 5 (the extinction's loss rule) :778-788 -> :805-815; [B-6b5]'s extinction
  word rule right after it (DT_Hazards rule index 12, 10 tests).
- `DT_Megaprojects.CompleteTemplates` :833 -> :870; DT_Megaprojects `project_binding` rule = DT_Megaprojects rule index 1
  (6 tests) - the ark rule goes after it.
- DT_Feats Title/MythicNarrative/LogTemplates :847/:848/:858 -> :890/:891/:901; `CompletionConditions` untagged
  CivHasTreaty :852 -> :895; DT_Feats fifth-rung rule = rule index 11 (6 tests), last of the table.
- `DT_Encounters.Body` :939 -> :988 (forbidden still the string "verdict" - so an encounter Body with '방주' passes check_data).
- Conditions/effects: MegaprojectComplete :1205 -> :1254; CivHasTreaty :1228 -> :1277; ShiftEthics :1236 -> :1285;
  ChangeStability :1239 -> :1288.
- `Tools/cosmic_data_schema.py`: unchanged since cef8cc7 - read_sources :284-297, row_is_known/row rules :600-623
  (row/row_not :617-623), holds row_not :1321-1322.

### Python tests (`Tools/test_data_schema.py`)
- `fixture_tables` :276-302 (unchanged lines). It sets `DT_Feats = [feat_row()]` (one row) and `DT_Megaprojects =
  [megaproject_row()] + required rows` (:290-291).
- holy_and_myth whole-project guard :1809-1813 -> :1812-1816; hand list :1815 -> :1818-1820; ClaimTemplates pin
  :1804-1806 -> :1807-1809.
- **Exact `forbidden` pins this commit must update** (brief 1.6.5): `test_upper_rung_text_words` (:2213) `places` at
  :2271-2275 - `DT_Events.LogTemplates` (:2273), `DT_LogTemplates.Templates` (:2274) and DT_Hazards Outbreak/Expire/Resolve
  (:2275) are pinned with assertEqual at :2276-2278. DT_Hazards `DisplayName` has no exact pin (it is a string today).
  The loop (2) at :2285-2290 walks only the nine [B-6b5] lists (and every list of DT_Feats places) - ark_words does not enter it.
- Already on [H'']: `:2345-2348` puts '{Civ}|hpp(은,는) 방주로 백성을 건져 냈다.' on `feat_test_climb` (T3->T4) and expects
  '건져'; after [B-6b6] the feat `row_not` rule also fires on that cell (expect() reads one message - check it still passes).
- Comment "(the fixture's hazard made one - the game holds none before [B-6b6])" :3398 -> :3617.
- `rule_setups` starts at :3552 (cef8cc7 :3333); the brief's :3440-3530 -> :3659-3749; `project_binding`'s entry :3766-3767
  (last DT_Megaprojects entry); the fifth-rung feat entry :3760; the extinction word rule's entry follows :3617-3619.
- `walked` pin :3563 -> :3792, now **305** (brief: +11 -> 316, set from the final schema).
- `test_every_rule_of_the_schema_can_fail` :3776.

### Runner (`Tools/run_checks.ps1`) - unchanged since cef8cc7; every cited line holds
- :35-39 Step, :48-53 -Data, :76 Remove-WrotePath, :85 commandlet call, :102-107 -UpdateBaseline, :115-132 the five runs
  (:115 checks, :119 ruler, :125 wanderer 6,000, :130 20,000, :131 ruler 20,000), :139-155 hash runs, :157 exit 1.

### C++ (Source/Cosmic_Ascension) - all unchanged since cef8cc7 except two files
- Unchanged, every cited line holds: `Sim/GalaxySimulationSubsystem.cpp` (:1768-1772, :2299-2308, :2310-2320, :2388, :2396,
  :2398-2410, :2879, :2892, :2955-2966, :2973-2976), `Data/CosmicDataTables.cpp:143-148`, `Data/CosmicDefinitionRows.h:300-303`,
  `Sim/CosmicHazards.cpp:746-752` (:751 `return Tag.IsValid() && Tag == ExtinctionTag;` - the b6b6_kills_ruler_not_extinction
  target), `Sim/CosmicMegaprojects.cpp:372`, `Sim/CosmicDiplomacy.cpp:415-426`/`:461-478`, `Events/CosmicEventSubsystem.cpp:486-489`,
  `Ascension/AscensionSubsystem.cpp:196-205`, `World/CosmicRulerPolicy.cpp:496`, `Tests/CosmicBalanceCommandlet.cpp` (every cite,
  e.g. :1596-1597 rows-taken-out line, :1968/:2025/:3575 check lines, :3890 ending, :3958-3992 race parts).
- `Tests/CosmicDataTests.cpp` unchanged: ark margin :2195-2235 (comment :2195-2204), death pins :2660-2702 (comment
  "...; [B-6b6] pins that row." ends :2666; KillPlayersOf pin :2669-2672), the tripwire :2693-2711 (`Today ==
  { CosmicRows::ProjectBinding }` at :2711, its TestTrue at :2710, message "takes out project_binding alone ... [B-6b6] updates this pin"),
  full copy :2712-2779; `Expect` any-carries :1215-1218.
- `Tests/CosmicB6bRowNames.h` unchanged: :1 "...that the game's data does not hold yet...", :4-8 the no-constant reason
  (":8" = "The data commit that adds a row moves its name to CosmicRows:: and deletes it here."), :17/:21/:24 "- [B-6b6]".
- `Tests/CosmicTestHelpers.h` unchanged: :48-49 slices, :268-271 ("none in the game's data before [B-6b6]" spans :269-270),
  :375-387 the Note order (Feats, Projects, Hazards, Edicts, Events, Encounters), :471-486 `WithoutProjects` (DT_Feats emptied).
- `Tests/CosmicDeathTests.cpp` unchanged: `QuietHazards` :106-119; the extinction block :797-830 (comment :798-799 "the game
  holds no such row before [B-6b6], so the quiet copy takes one"; `Hazards->AddRow(ExtinctRow, ...)` :811 - the RemoveRow goes
  before it). Note the test's row name is `crisis_test_death_extinct`, so the clash is of tag, not name.
- `Tests/CosmicUpperRungTests.cpp:776-777` unchanged (the comment spans :776-777).
- `Tests/CosmicWarningLineTests.cpp` changed by [B-6b4]: :110-111 -> **:111-112**.
- `UI/CosmicSystemPanelWidget.cpp` changed by [B-6b4]: :1042 -> **:1065** (the greyed answer button test).
- Test locations (all unchanged): EndingRecordOnce :655, EndingInert :784, EndingTables WithoutB6bData :155-158;
  SightWalkMoved :1851/:1875; SuccessionPossible :610/:631; NotablesCapped :2932, NotablesFromEventsOnly :2983;
  WandererLoop :1347; RealmCap :1006; StartBlockReasons :951, CivAIProjects :1160, Feats.Reachable :1760-1780 (count
  :1772-1777); PhaseTwoExit :2260.
- `Config/DefaultGameplayTags.ini:20` unchanged.

### Docs
- H (`B6b_upper_rungs_and_the_race.md`): one [B-6b5] hunk, every cited H line holds (:97, :99, :100, :246, :247, :325,
  :329-330, :336, :338, :344, :358, :360, :369, :377, :380, :391-393, :403, :410, :421, :429 - P_bind "시작값 1,800").
- N: +59 lines after :468. N:348/:425/:434/:445 hold; N:525 -> :584, :527 -> :586, :552 -> :611, :571 -> :630,
  :597 -> :656, :598 -> :657, :622 -> :681 ("묶음 A"), :624 -> :683 ("(비어 있다 — [B-6b6]이 채운다.)"), :659 -> :718,
  :674 -> :733, :675 -> :734 (④ grace two paths), :700 -> :759, :704 -> :763, :705 -> :764, :709 -> :768 (speed rule).
  The [B-6b5] block of N section 5 is N:781-791; its hand-offs to [B-6b6] are ⑧ (ark rule, '방주' row_not guard, '노아'),
  ㉔ (re-read '건넜다' vs the crossing word '건넘'), ㉗ (hand-read '방주' beside '절멸' on the answer button and
  '방주(절멸 위기에 답할 때)'), ㉘ (hand-read '업적 — 방주와 조약' on the epitaph; decide whether the epitaph drops the title).
- `DECISIONS_LOG.md:34-37` hold (:34 [B-6b2b] line); the [B-6b5] line is the last B6b line - the new [B-6b6] line goes after it.
- `docs/70_CONTENT_GUIDE.md`: rule 7 is at **:169** ("지금은 글 감사가, `[B-6b6]`부터는 스키마가 본다"); :129 holds
  "`[B-6b6]`·`[B-6b7]`부터".
- `docs/21`: '스키마의 꼴' paragraph starts at **:1259** (brief's :1266 is inside it, touched by [B-6b5]); the world functions
  list :1458-1463 holds (no explicit reader list of `KillsTheRulerOf`).
- `docs/40`: :25, :237 (Data.Integrity row: ark margin "게임에는 절멸 행이 `[B-6b6]`까지 없어" and the WithoutB6bData
  "project_binding 하나만 뺀다(`[B-6b6]`이 제 행과 함께 고칠 일부러...)" - both clauses change), :238 Feats.Reachable,
  :496 hold.
- Markers "[B-6b6]부터" on [H''] (grep): crisis_and_transcendents.md :24 :40 :67 :115 :139 :175 :177 :266; pillars.md
  :105 :108; systems.md :65 :91 :229 :417; 15_GLOSSARY :151; **docs/20 :75 (new, not in the brief's list)**, :109
  ("(`feat_passage_by_ark`, B6b `[B-6b6]`)부터"), :212 (two), :289 (two); docs/21 :763 :973 :1518 (two) :1520 :1585 (two) (and :629,
  :1658 name [B-6b6] without "부터" - :1658 "행은 `[B-6b6]`" is false after this commit); docs/40 :237 :496; docs/70 :129 :169.
- `systems.md` has no `Feats.Reachable` (grep on [H''] - 0 hits), so only docs/40:238 changes if the test is raised.
- `STATUS.md:27` and `:30-32` (the lead's file - read only here).

### [B-6b4] tests on game tables (brief 0.5)
- `git grep HasJudgedRowAskingAt|HasJudgedRowAt Source/Cosmic_Ascension/Tests` = 5 lines, all in
  `Tests/CosmicChronicleScreenTests.cpp:955-966`, all `Tier4_Transcendent` (the T4 pins: :960 "The game's tables judge no
  row up to the fifth rung yet" - stays false until [B-6b7]). No assertion pins Tier3_Causal + CivHasTreaty false on the game
  tables - nothing to change for 2.4 there. The one game reader of `HasJudgedRowAskingAt(Tier3_Causal, CivHasTreaty)` is
  `UI/CosmicEdictWidget.cpp:1451` (the treaty tip turns on with the new feat row in a real game).
- `UI.PeoplesTab` (`Tests/CosmicDiplomacyUITests.cpp:272`): its board comes from `LayBoard` (:141, `WithoutProjects` at :172,
  DT_Feats empty) - "no tip with the board's tables" (:634-635) holds; the asking/unasking cases build their own rows (:636-653).

