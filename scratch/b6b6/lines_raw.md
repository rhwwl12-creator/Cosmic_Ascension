# [B-6b6] line map: brief (cef8cc7) -> HEAD (10205b6, [H''])

Each row: old line(s) at cef8cc7 -> new line(s) at HEAD; TOUCHED = the old line lies inside a hunk that [B-6b4]/[B-6b5] changed (re-check by text).
Unchanged files keep every line. First and last text of the new range are shown for re-finding.

## `Tools/data/DT_Hazards.json` (unchanged since cef8cc7 - every line holds) - crisis_rebellion end / pre_supernova row / Draw shares
- :261 = :261 | `}`
- :145-194 = :145-194 | `{`
- :9 = :9 | `"SpawnChancePerTick": 0.0002,`
- :109 = :109 | `"SpawnChancePerTick": 0.0001,`
- :152 = :152 | `"SpawnChancePerTick": 0.0001,`

## `Tools/data/DT_Megaprojects.json` (changed since cef8cc7: 2 hunks) - project_binding end / stabilize cost / P_bind
- :73 -> :73 | `}` (same text)
- :54 -> :54 | `"CostPerTick": {"Energy": 2.0, "Matter": 2.0, "Insight": 0.0, "Influence": 0.0, "Causality` (same text)
- :67 -> :67 | `"RequiredProgress": 1800.0,` (same text)

## `Tools/data/DT_Feats.json` (changed since cef8cc7: 5 hunks) - feat_three_worlds end / TargetTier lines
- :66 -> :66 | `}` (same text)
- :7 -> :7 | `"TargetTier": "Tier2_Stellar",` (same text)
- :29 -> :29 | `"TargetTier": "Tier3_Causal",` (same text)
- :50 -> :50 | `"TargetTier": "Tier3_Causal",` (same text)

## `Tools/data/DT_Rules.json` (unchanged since cef8cc7 - every line holds) - TierMultiplier
- :60 = :60 | `"TierMultiplier": [1.0, 1.5, 2.2, 3.0, 3.0],`

## `Tools/data/schema.json` (changed since cef8cc7: 23 hunks) - schema cites
- :28 -> :29 | `"row_name_sources": ["Source/Cosmic_Ascension/Data/CosmicRowNames.h"],` (same text)
- :34 -> :35 | `"axis": ["Pacifism", "Spiritualism", "Openness"],` (same text)
- :44 -> :45 | `"holy_and_myth": ["여호와", "예수", "그리스도", "보혜사", "메시아", "사탄", "루시퍼", "미가엘", "가브리엘", "바알", "몰렉` (same text)
- :326 -> :345 TOUCHED | `"forbidden": ["verdict", "soul_verbs", "holy_words", "rescue_words", "rung_grasp_words", "` (old text `"forbidden": ["verdict", "soul_verbs", "holy_words", "rescue_words"]`)
- :450 -> :472 TOUCHED | `"LogTemplates": {"kind": "texts", "slots": ["Civ", "System", "Hazard"], "forbidden": ["ver` (same text)
- :697 -> :724 | `"DisplayName": {"kind": "text", "required": true, "slots": [], "forbidden": "verdict"},` (same text)
- :711-713 -> :738-740 TOUCHED | `"OutbreakTemplates": {"kind": "texts", "slots": ["Civ", "System", "Hazard"], "forbidden": ` ... `"ResolveTemplates": {"kind": "texts", "slots": ["Civ", "Syst` (same text)
- :716-718 -> :743-745 | `"unique": [` ... `],` (same text)
- :778-788 -> :805-815 | `{` ... `},` (same text)
- :833 -> :870 TOUCHED | `"CompleteTemplates": {"kind": "texts", "min": 3, "slots": ["Civ", "System", "Project"], "f` (same text)
- :847 -> :890 TOUCHED | `"Title": {"kind": "text", "required": true, "slots": [], "forbidden": ["verdict", "holy_wo` (old text `"Title": {"kind": "text", "required": true, "slots": [], "forbidden": `)
- :848 -> :891 TOUCHED | `"MythicNarrative": {"kind": "text", "required": true, "slots": [], "forbidden": ["verdict"` (old text `"MythicNarrative": {"kind": "text", "required": true, "slots": [], "fo`)
- :852 -> :895 | `"CompletionConditions": {"kind": "conditions", "untagged": ["CivHasTreaty"]},` (same text)
- :858 -> :901 TOUCHED | `"LogTemplates": {"kind": "texts", "min": 3, "slots": ["Civ", "HomeSystem"], "forbidden": [` (old text `"LogTemplates": {"kind": "texts", "min": 3, "slots": ["Civ", "HomeSyst`)
- :939 -> :988 | `"Body": {"kind": "text", "slots": ["Civ", "System", "Hazard", "Rebel", "OtherCiv"], "no_di` (same text)
- :1205 -> :1254 | `"MegaprojectComplete": {"key": {"row": "DT_Megaprojects"}},` (same text)
- :1228 -> :1277 | `"CivHasTreaty": {"tag": {"root": "Treaty.", "optional": true}, "value": {"min": 1, "intege` (same text)
- :1236 -> :1285 | `"ShiftEthics": {"key": {"word": "axis"}, "value": {"min": -2, "max": 2}},` (same text)
- :1239 -> :1288 | `"ChangeStability": {"value": {"min": -1, "max": 1}},` (same text)

## `Tools/cosmic_data_schema.py` (unchanged since cef8cc7 - every line holds) - read_sources / row rules / holds
- :284-297 = :284-297 | `def read_sources(root=None):`
- :600-623 = :600-623 | `def row_is_known(self, table_name, name):`
- :617-623 = :617-623 | `if isinstance(test, dict) and ("row" in test or "row_not" in test):`
- :1321-1322 = :1321-1322 | `if "row_not" in test:`

## `Tools/test_data_schema.py` (changed since cef8cc7: 11 hunks) - harness
- :276-302 -> :276-302 | `def fixture_tables(schema, tables):` ... `return tables` (same text)
- :1804-1806 -> :1807-1809 | `self.assertEqual(columns["ClaimTemplates"]["forbidden"], columns["Lure"]["forbidden"])` ... `["verdict_without_soul", "holy_words", "holy_and_myth", "rea` (same text)
- :1809-1813 -> :1812-1816 | `# No name of holy_and_myth is in any source of the project (matched letter for letter: one` ... `self.assertEqual([name for name in lists["holy_and_myth"] if` (same text)
- :1815 -> :1818 | `for hidden in ("에서", "함", "셈", "요한", "단", "마라", "미가", "바로", "그룹", "엘리", "사라", "누가", "하와", ` (same text)
- :3398 -> :3617 | `# B6b [B-6b2b]: the extinction's row (the fixture's hazard made one - the game holds none ` (same text)
- :3440-3530 -> :3659-3749 | `("DT_Galaxies", "a galaxy other than the home holds no peoples, no brake, no home share an` ... `(NOTHING, "log_siege_held_far"),` (same text)
- :3517 -> :3736 TOUCHED | `("DT_LogTemplates", "the death line of a ruler at an extinction's expiry is the death line` (same text)
- :3520 -> :3739 TOUCHED | `("DT_LogTemplates", "the line of the binding is told in the actorless passive by the name ` (same text)
- :3563 -> :3792 TOUCHED | `self.assertEqual(walked, 305, "the number of then-tests in schema.json changed: look at wh` (old text `self.assertEqual(walked, 271, "the number of then-tests in schema.json`)

## `Tools/run_checks.ps1` (unchanged since cef8cc7 - every line holds) - runner
- :35-39 = :35-39 | `function Step([string] $Name, [scriptblock] $Body) {`
- :48-53 = :48-53 | `if ($Data) {`
- :76 = :76 | `function Remove-WrotePath([string] $Line) { return ($Line -replace ', wrote .*?(?= \| |$)'`
- :85 = :85 | `& $Editor $Project -run=CosmicBalance -Seed=42 "-Ticks=$Ticks" "-Out=$Name.csv" @Extra -un`
- :102-107 = :102-107 | `if ($UpdateBaseline) {`
- :116-132 = :116-132 | ``
- :121-124 = :121-124 | `# B1 (header section 5): the same ruler, who leaves the throne at tick 1,200 and walks (Wo`
- :129-132 = :129-132 | `if ($Tier -eq 'full') {`
- :139-155 = :139-155 | `function Compare-HashRun([string] $Name, [string] $BalanceLog, [string[]] $Extra) {`
- :157 = :157 | `if ($failed.Count -gt 0) { Write-Host "run_checks: FAILED -> $($failed -join '; ')"; exit `

## `Tools/mutations/README.md` (changed since cef8cc7: 10 hunks) - README
- :5-7 -> :5-7 TOUCHED | `A patch of `Tools/data` (the `b4d2_data_` ones, `b6b2a_data_laws_knob_four`, `b6b2a_data_r` ... ``python -I Tools/check_data.py`): the checker refuses them b` (same text)
- :792 -> :792 | `| `B6b/` | `b6b2c_data_ark_margin_half.patch` — `DT_Rules.AIAscensionSpeedMultiplier` 0.5 ` (same text)
- :868-873 -> :868-873 | `| `B6b/` | `b6b3b_without_b6b_keeps_binding.patch` — `CosmicTests::WithoutB6bData` keeps `` ... `| `B6b/` | `b6b3b_without_b6b_drops_other_rows.patch` — `Wit` (same text)

## `Config/DefaultGameplayTags.ini` (unchanged since cef8cc7 - every line holds) - extinction tag
- :20 = :20 | `+GameplayTagList=(Tag="Hazard.Extinction",DevComment="Extinction crisis. 250 ticks. Unreso`

## `Source/Cosmic_Ascension/Sim/GalaxySimulationSubsystem.cpp` (unchanged since cef8cc7 - every line holds) - sim
- :1768-1772 = :1768-1772 | `// people (section 5.1). Causality stays at zero: it opens with Tier 3 research.`
- :2299-2308 = :2299-2308 | `// Two numbers per expiry, all of them before the first one runs (section 9.2).`
- :2310-2320 = :2310-2320 | `// (c) Spawn. One number per settled system, all drawn first, whether its slot is free or `
- :2388 = :2388 | `if (!UCosmicEventSubsystem::CheckAll(Row->SpawnConditions, View))`
- :2396 = :2396 | `const float Pick = Total > 1.f ? SpawnNumbers[Index] * Total : SpawnNumbers[Index];`
- :2398-2410 = :2398-2410 | `for (const TPair<FName, float>& Share : Shares)`
- :2879 = :2879 | `Args.Add(TEXT("Project"), Row->DisplayName);`
- :2892 = :2892 | `Events->FollowUpNotes(Ctx);`
- :2955-2966 = :2955-2966 | `// The treaty step (decision 1): under a row whose CivHasTreaty does not hold, at most one`
- :2961-2965 = :2961-2965 | `if (Partner != INDEX_NONE)`
- :2973-2976 = :2973-2976 | `// (b) The feat judgment, once per tick.`

## `Source/Cosmic_Ascension/Data/CosmicDataTables.cpp` (unchanged since cef8cc7 - every line holds) - draw order
- :143-148 = :143-148 | `TArray<FName> FCosmicDataTables::GetHazardRowNames() const`

## `Source/Cosmic_Ascension/Data/CosmicDefinitionRows.h` (unchanged since cef8cc7 - every line holds) - feat by rung
- :300-303 = :300-303 | `bool IsJudgedRulerRowAt(EAscensionTier Tier) const`

## `Source/Cosmic_Ascension/Sim/CosmicHazards.cpp` (unchanged since cef8cc7 - every line holds) - KillsTheRulerOf
- :746-752 = :746-752 | `bool CosmicHazards::KillsTheRulerOf(const FGameplayTag& Tag)`
- :751 = :751 | `return Tag.IsValid() && Tag == ExtinctionTag;`

## `Source/Cosmic_Ascension/Sim/CosmicMegaprojects.cpp` (unchanged since cef8cc7 - every line holds) - answer row never unprompted
- :372 = :372 | `if (Condition.Type != EConditionType::MegaprojectComplete || Tables.IsHazardAnswer(Conditi`

## `Source/Cosmic_Ascension/Sim/CosmicDiplomacy.cpp` (unchanged since cef8cc7 - every line holds) - TreatyBlockOf
- :415-426 = :415-426 | `if (StanceOf(Civ, OtherID) == CosmicTags::StanceHostile() || StanceOf(Other, Civ.CivID) ==`
- :461-478 = :461-478 | `int32 CosmicDiplomacy::AITreatyPartnerOf(const FGalaxyState& State, const FCosmicStateInde`

## `Source/Cosmic_Ascension/Events/CosmicEventSubsystem.cpp` (unchanged since cef8cc7 - every line holds) - completion per system
- :486-489 = :486-489 | `case EConditionType::MegaprojectComplete:`

## `Source/Cosmic_Ascension/Ascension/AscensionSubsystem.cpp` (unchanged since cef8cc7 - every line holds) - HomeSystem = capital
- :196-205 = :196-205 | `const int32 Capital = Ctx.Index.FindSystem(State, Civ.HomeSystemID);`

## `Source/Cosmic_Ascension/World/CosmicRulerPolicy.cpp` (unchanged since cef8cc7 - every line holds) - policy reads ruled rung
- :496 = :496 | `for (const FCosmicRungPart& Part : CosmicNextRung::ResearchFieldsOf(CosmicNextRung::AllOf(`

## `Source/Cosmic_Ascension/UI/CosmicSystemPanelWidget.cpp` (changed since cef8cc7: 10 hunks) - ark button
- :1042 -> :1065 | `if (!Tables.IsHazardAnswer(Row) && RuledRung + 1 < static_cast<int32>(Def->RequiredTier))` (same text)

## `Source/Cosmic_Ascension/Tests/CosmicDataTests.cpp` (unchanged since cef8cc7 - every line holds) - data tests
- :1215-1218 = :1215-1218 | `auto Expect = [this](const FString& What, const TArray<FString>& Problems, std::initialize`
- :2195-2204 = :2195-2204 | `// B6b [B-6b2c] (docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md section 2.2, 2.`
- :2195-2235 = :2195-2235 | `// B6b [B-6b2c] (docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md section 2.2, 2.`
- :2660-2702 = :2660-2702 | `// B5 [B-5d2] (header 3.2, 3.5, decision 6-ga): the loaded tables hold the one death the s`
- :2660-2711 = :2660-2711 | `// B5 [B-5d2] (header 3.2, 3.5, decision 6-ga): the loaded tables hold the one death the s`
- :2663-2666 = :2663-2666 | `// exactly it away and keeps every row of the tables it copies. One of two pins with Tools`
- :2669-2672 = :2669-2672 | `const TArray<FString> Deaths = KillPlayersOf(Game);`
- :2693-2711 = :2693-2711 | `// B6b [B-6b3b] (header section 5.1, decision 34): CosmicTests::WithoutB6bData, the data-o`
- :2712-2779 = :2712-2779 | ``

## `Source/Cosmic_Ascension/Tests/CosmicB6bRowNames.h` (unchanged since cef8cc7 - every line holds) - row names
- :1 = :1 | `// Cosmic Ascension - the names of B6b's rows that the game's data does not hold yet, for `
- :4-8 = :4-8 | `// They are not CosmicRows:: constants (Data/CosmicRowNames.h): the data checker holds eve`
- :17 = :17 | `// DT_Feats: the feats of the third rung (the passage by the ark) and the fourth (the laws`
- :21 = :21 | `// DT_Megaprojects: the extinction's answer (the response slot) - [B-6b6].`
- :24 = :24 | `// DT_Hazards: the extinction (Hazard.Extinction) - [B-6b6].`

## `Source/Cosmic_Ascension/Tests/CosmicTestHelpers.h` (unchanged since cef8cc7 - every line holds) - helpers
- :48-49 = :48-49 | `inline constexpr int32 SliceSystems = 0;`
- :268-271 = :268-271 | `// Everything else as given. The data's deaths only: the ruler's death at an extinction's `
- :375-387 = :375-387 | `const std::initializer_list<FName> Feats = { CosmicB6bRows::FeatPassageByArk, CosmicB6bRow`
- :471-486 = :471-486 | `inline FCosmicDataTables WithoutProjects(const FCosmicDataTables& Base)`

## `Source/Cosmic_Ascension/Tests/CosmicEndingTests.cpp` (unchanged since cef8cc7 - every line holds)
- :155-158 = :155-158 | `// death is code - CosmicTests::WithoutB6bData, outside the WithRules), with Rules as DT_R`
- :655 = :655 | `IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimEndingRecordOnceTest, "Cosmic_Ascension.Sim.End`
- :784 = :784 | `IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimEndingInertTest, "Cosmic_Ascension.Sim.EndingIn`

## `Source/Cosmic_Ascension/Tests/CosmicKnowingTests.cpp` (unchanged since cef8cc7 - every line holds)
- :1851 = :1851 | `IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSightWalkMovedTest, "Cosmic_Ascension.Sim.SightWal`
- :1875 = :1875 | `Sim->SetTables(CosmicTests::WithoutB6bData(CosmicTests::WithExaltAsBefore(CosmicTests::Wit`

## `Source/Cosmic_Ascension/Tests/CosmicSuccessionTests.cpp` (unchanged since cef8cc7 - every line holds)
- :610 = :610 | `IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSuccessionPossibleTest, "Cosmic_Ascension.Sim.Succ`
- :631 = :631 | `Sim->SetTables(CosmicTests::WithoutB6bData(CosmicTests::WithoutDeath(CosmicTests::WithRule`

## `Source/Cosmic_Ascension/Tests/CosmicHazardTests.cpp` (unchanged since cef8cc7 - every line holds)
- :2932 = :2932 | `IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicNotablesCappedTest, "Cosmic_Ascension.Sim.Notables`
- :2983 = :2983 | `IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicNotablesFromEventsOnlyTest, "Cosmic_Ascension.Sim.`

## `Source/Cosmic_Ascension/Tests/CosmicWandererTests.cpp` (unchanged since cef8cc7 - every line holds)
- :1347 = :1347 | `IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimWandererLoopTest, "Cosmic_Ascension.Sim.Wandere`

## `Source/Cosmic_Ascension/Tests/CosmicSettlementTests.cpp` (unchanged since cef8cc7 - every line holds)
- :1006 = :1006 | `IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicRealmCapTest, "Cosmic_Ascension.Sim.RealmCap", Cos`

## `Source/Cosmic_Ascension/Tests/CosmicAscensionTests.cpp` (unchanged since cef8cc7 - every line holds)
- :951 = :951 | `IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicStartBlockReasonsTest, "Cosmic_Ascension.Sim.Start`
- :1160 = :1160 | `IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicCivAIProjectsTest, "Cosmic_Ascension.Sim.CivAIProj`
- :1760-1780 = :1760-1780 | `IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicFeatsReachableTest, "Cosmic_Ascension.Feats.Reacha`
- :1772-1777 = :1772-1777 | `const FGalaxyState& State = Sim->GetState();`

## `Source/Cosmic_Ascension/Tests/CosmicSimTests.cpp` (unchanged since cef8cc7 - every line holds)
- :2260 = :2260 | `IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCosmicSimPhaseTwoExitTest, "Cosmic_Ascension.Sim.PhaseTw`

## `Source/Cosmic_Ascension/Tests/CosmicDeathTests.cpp` (unchanged since cef8cc7 - every line holds)
- :106-119 = :106-119 | `UDataTable* QuietHazards(const FCosmicDataTables& Original)`
- :797-830 = :797-830 | ``
- :798-799 = :798-799 | `// B6b [B-6b2b] (B6b header section 2.2, decision 5): a Death.Hazard whose source row's ta`

## `Source/Cosmic_Ascension/Tests/CosmicUpperRungTests.cpp` (unchanged since cef8cc7 - every line holds)
- :776 = :776 | `// The rows of the extinction's boards: the extinction (the game holds no Hazard.Extinctio`

## `Source/Cosmic_Ascension/Tests/CosmicWarningLineTests.cpp` (changed since cef8cc7: 3 hunks)
- :110-111 -> :111-112 | `// row (300 ticks), a supernova of the tests (one tick) and an extinction of the tests (on` ... `// Hazard.Extinction row before [B-6b6]). The transcendents:` (same text)

## `Source/Cosmic_Ascension/Tests/CosmicBalanceCommandlet.cpp` (unchanged since cef8cc7 - every line holds) - commandlet
- :544 = :544 | `// row of the climb (its Log.Ascension line - missing under -TellAll=0 when untold).`
- :1525 = :1525 | `const FString OutPath = FPaths::IsRelative(Out) ? FPaths::ProjectSavedDir() / TEXT("Balanc`
- :1596-1597 = :1596-1597 | `UE_LOG(LogCosmic, Display, TEXT("CosmicBalance: tables without B6b's data - rows taken out`
- :1968 = :1968 | `UE_LOG(LogCosmic, Error, TEXT("CosmicBalance end check: at %lld (%s) the record is %s, the`
- :2025 = :2025 | `UE_LOG(LogCosmic, Error, TEXT("CosmicBalance race check: at %lld an AI people climbed to t`
- :2275-2288 = :2275-2288 | `{`
- :2364-2367 = :2364-2367 | `else if (!Owner.IsNone())`
- :3575 = :3575 | `UE_LOG(LogCosmic, Error, TEXT("CosmicBalance race check: the home cap was passed at %lld b`
- :3610 = :3610 | `UE_LOG(LogCosmic, Display, TEXT("CosmicBalance hazards: placed%s | answered%s | expired%s `
- :3890 = :3890 | `UE_LOG(LogCosmic, Display, TEXT("CosmicBalance ending: record %s at %lld (people %s, a peo`
- :3958-3992 = :3958-3992 | `Parts.Add(FString::Printf(TEXT("home %s cap %d"), *RaceNameOf(RaceHome), RaceHomeGalaxy ? `
- :3959-3961 = :3959-3961 | `Parts.Add(FString::Printf(TEXT("first AI T4 at %lld (%s, feat %s), second AI T4 at %lld (%`
- :3966 = :3966 | `Parts.Add(FString::Printf(TEXT("T4 civs at the record %d, at the end %d, most %d"), Race.T`
- :3976-3980 = :3976-3980 | `Parts.Add(FString::Printf(TEXT("laws first held ruled at %lld, AI at %lld (%s), longest he`

## `docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md` (changed since cef8cc7: 1 hunks) - H
- :10 -> :10 | `- **경주**: 다스리는 백성과 다스려지지 않는 백성 가운데 누가 먼저 다섯째 칸에 서는가. 먼저 선 쪽이 판의 끝을 적는다 — 집단 승천(`Ending.Rul` (same text)
- :45 -> :45 | `- **지키는 것**(`CLAUDE.md` 규칙 11·12 — 글의 규칙은 §3.5): 거룩한 질서는 엔티티·화자·수치·엔딩 조건이 아니다 — 결박은 행위자 없는` (same text)
- :46 -> :46 | `- **판의 끝과 B6b가 보이는 곳**: 집단 승천과 다른 질서가 처음 판 안에 닿는다. 마일스톤 B의 '엔딩 1회 콘솔 없이'(B6a 결정 21)는 `[B-6` (same text)
- :97 -> :97 | `**행**(결정 4): `DT_Hazards`의 새 `Draw` 행 — `Hazard.Extinction`, '절멸 위기', 카운트다운 250(`crisis_an` (same text)
- :99 -> :99 | `- `OnExpireEffects`·`OnResolveEffects`는 비운다(놓음과 죽음은 아래 코드 — 데이터의 `TransferSystem`은 만료에 서지 ` (same text)
- :100 -> :100 | `- **누구의 절멸인가**(결정 2 [개발자]): 그 백성 자신의 것 — '이주'는 상태가 옮기지 않고(`Civs` 그대로) 글이 '방주로 위기를 건넜다'로 말한` (same text)
- :246 -> :246 | `| `crisis_species_extinction` | `DT_Hazards`, `[B-6b6]`(결정 4) | 추첨 위기의 몫은 이름 차례로 쌓인다(`Data` (same text)
- :247 -> :247 | `| `project_ark` | `DT_Megaprojects`, `[B-6b6]`(결정 6) | 절멸의 답(대응 칸). 프로젝트 표를 걷는 세계 규칙은 없고 패` (same text)
- :325 -> :325 | `B6a의 일하는 법 그대로(B6a 헤더 §5.2): **30건 이상일 때만 판단**, 모자라면 '재지 않음 — 잰 N'; 몫에는 95% 구간(Wilson); 판 ` (same text)
- :329 -> :329 | `| **A** | `[B-6b6]` | R 30 × 20,000(1~30), 입력 없는 30 × 20,000(1~30), 방랑자 60 × 6,000(1~60), ` (same text)
- :330 -> :330 | `| **B** | `[B-6b8]` — A4의 '장치 빌드'(장치와 데이터 모두, 시계 전) | R 60·느린 R 60·방랑자 60·입력 없는 60 × 20,00` (same text)
- :336 -> :336 | `### 5.3 미리 적은 판단과 시계의 규칙 (결정 35 [개발자] — 데이터를 켜기 전에 이 글 그대로 고정한다)` (same text)
- :338 -> :338 | `**묶음 B의 판단**(어긋나면 `[B-6b9]` — 탐침은 `-Rules`(r)·`-FourthRungCap`, 행의 값(c·P_ark·P_bind·N)은 커밋` (same text)
- :344 -> :344 | `| ③ | 넷째 칸의 계절 | ①의 판 가운데 다스리는 백성이 다섯째 칸에 선 판(≥ 30) | 넷째 → 다섯째 틱의 중앙값 ≤ 3,600(아래 끝 1,200은 ` (same text)
- :358 -> :358 | `커밋마다 옮을 기준값과 그 깨질 수 있는 확인은 노트 §2.2의 표(C = 6,000일 때의 `[B-6b10]`은 §5.3). `[B-6b11]`·`[B-6b12` (same text)
- :360 -> :360 | `## 6. 테스트 (새 단언은 변이로 한 번씩 깨뜨린다 — 패치는 `Tools/mutations/B6b/b6b<N>_<이름>.patch`, README에 한 줄씩` (same text)
- :369 -> :369 | `- **`[B-6b5]`** — 변형 수(커밋 메시지의 한 줄)·새 낱말 목록과 접두어마다 깨지는 경우. **`[B-6b6]`** — `Data.Integrity` (same text)
- :377 -> :377 | `- **장치의 두 커밋**(`[B-6b2]`·`[B-6b3]` — 주석은 그것이 말하는 코드를 바꾸는 커밋에). **`docs/30` 다시 만듦**(러너의 문서 ` (same text)
- :380 -> :380 | `- **문서 커밋**(`[B-6b12]` — 끝에 재감사 워크플로): `docs/50`(B6b 칸, Phase 4 목록 `:197-198` — 절멸의 행·훅·방주` (same text)
- :391 -> :391 | `| `[B-6b5]` | **글과 목소리**: 먼 줄 셋, 새 ST_CA 키, 새 행 모두의 감사한 초안 — 데이터 행(절멸·방주·업적 둘·법칙 셋·시대·`enc` (same text)
- :392 -> :392 | `| `[B-6b6]` | **데이터 ① 셋째 → 넷째**: 절멸 행·`project_ark`·업적 행, 결정 5의 테스트 → 묶음 A | high | 러너 ful` (same text)
- :393 -> :393 | `| `[B-6b7]` | **데이터 ② 넷째 → 다섯째**: 법칙 행 셋·업적 행·`project_binding`의 값, N·P_bind·r의 첫 값 | high` (same text)
- :403 -> :403 | `- **해시를 옮기는 새 길과 데이터 켜기는 한 커밋에 두지 않는다**(`WORKPLAN.md:61`; B3 `[B-3d1]`·`[B-3d2]`): 새 길은 필드` (same text)
- :410 -> :410 | `1. **B6a의 ①·③·⑨를 먼저**(B6a 헤더 §9): 셋은 시계 6,000과 `[B-6a6]`의 증거 세이브(`Saved\HandCheck\B6a_*`)에` (same text)
- :421 -> :421 | `4. **절멸 위기의 행**(권장): `crisis_species_extinction`(마지막 이름 — `crisis_pre_supernova` 뒤로 정렬되어 앞` (same text)
- :429 -> :429 | `12. **결박이 일어나는 길**(권장 ㉮): 건설 프로젝트 행 `project_binding`(넷째 칸, 인과율 없음, 진척 P_bind — 시작값 1,800,` (same text)

## `docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race_notes.md` (changed since cef8cc7: 6 hunks) - N
- :348 -> :348 | `| 40 | `Feats.Reachable`의 뜻('AI 셋째 칸 ≥ 1' — `Tests/CosmicAscensionTests.cpp:1583`; `docs/4` (same text)
- :425 -> :425 | `## 1.3 테스트 (헤더 §6에서 — 커밋마다의 단언과 변이, 그리고 데이터와 시계가 뜻을 바꾸는 테스트; 커밋마다 돌려 보고 고친다, 깨지면 시드를 바꾸기 전` (same text)
- :434 -> :434 | `- **`[B-6b6]`** — `Data.Integrity`(더함, 결정 5): 표의 죽음은 각성의 것 하나 그대로(`:2418-2430`), 코드의 죽음은 절` (same text)
- :445 -> :445 | `| `Feats.Reachable`(`:1583-1602`) | AI 셋째 칸 ≥ 1 | `[B-6b6]` | AI 넷째 칸 ≥ 1 — 묶음 A가 슬라이스 판의 ` (same text)
- :469 -> :454 TOUCHED | `| 키 | B6b의 글(`[B-6b5]`) | `[B-6b4]`의 초안에서 |` (old text `## 1.5 데이터 행의 감사한 글 (헤더 §8 `[B-6b5]`에서 — 그 행을 켜는 커밋이 글자 그대로 옮긴다)`)
- :471 -> :456 TOUCHED | `| `Chronicle.RuleRung`(`:146`) | '이 판은 어느 백성이든 다섯째 칸에 서게 되면 멈춘다.' — 두 끝에 같은 줄(B6a 결정 6-마) ` (old text `(비어 있다 — `[B-6b5]`의 글 감사가 행마다 채운다: `crisis_species_extinction`(화면 이름과 `)
- :525 -> :584 | `- **넷째 칸 뒤에도 친다**: 조건이 `CivTierAtLeast 3`이라 넷째·다섯째 칸 백성도 맞고, 언약 면역(#166)이 없어 판의 나머지 동안 백성마` (same text)
- :527 -> :586 | `- 시작값의 권장(헤더 결정 4): **c = 1e-4**(지금 역병·초신성 전조와 같은 자릿수 — `DT_Hazards.json:109`·`:152`; AI의 ` (same text)
- :552 -> :611 | `- **r = 1이면 다스리는 백성이 먼저 결박하는 몫 = 넷째 칸에 첫 AI보다 먼저 서는 몫**(0.16 — c 1e-4·P 1,200의 줄; 노출이 열한 배` (same text)
- :571 -> :630 | `**ⓖ AI 조약 걸음이 바꾸는 세계**(결정 1). 셋째 칸 AI가 행이 조약을 부르는 틱에 첫 국경 백성과 불가침을 맺으면(영향력 50), 그 짝은 서로 겨누` (same text)
- :597 -> :656 | `| `[B-6b6]` | 두 해시 — 첫 AI 셋째 칸 1,521부터 | 같다(1,521) | 같다(1,558) | 둘 다 | ⓐ 짝이 `[B-6b5]`의 다섯 ` (same text)
- :598 -> :657 | `| `[B-6b7]` | `[B-6b6]`의 기준값 판 자신의 `race:` 줄이 지평 안에 넷째 칸을 보인 줄 — 모형으로는 시드 42의 첫 AI 넷째 칸이 약` (same text)
- :622 -> :681 | `### 묶음 A (`[B-6b6]` — 데이터 켬 ①, 시계 0)` (same text)
- :624 -> :683 | `(비어 있다 — `[B-6b6]`이 채운다.)` (same text)
- :659 -> :718 | `- **기준값**(헤더 §5.4 — 노트 §2.2의 `[B-6b1]` 행 그대로: 두 해시가 모두 옮고 세계는 그대로): 입력 없는 3,000 4236280797` (same text)
- :674 -> :733 | `- **데이터**: `DT_LogTemplates` 끝에 행 둘(초안 — `[B-6b5]`의 글 감사가 정한다; 변형 셋씩, 노트 §1.6의 3과 규칙 11): ` (same text)
- :675 -> :734 | `- **헤더와 다르게 하거나 더한 것**: ① `LetPeopleGo`가 `Transfer`처럼 반란 지도자의 기록을 닫고 백성 없는 완성 기록에 그 백성을 적는` (same text)
- :700 -> :759 | `**`[B-6b3b]`**(2026-11-09 — 장치 — 앎과 측정을 둘로 나눈 둘째: 측정):` (same text)
- :704 -> :763 | `- **변이 마흔**(`Tools/mutations/B6b/b6b3b_*.patch`, README에 한 줄씩 — 모두 깨졌다; 지침서가 정한 서른둘 + 술어 단` (same text)
- :705 -> :764 | `- **헤더와 다르게 하거나 더한 것**: ① 세 테스트의 `WithoutB6bData`를 `[B-6b6]`이 아니라 여기서 건다(헤더 §5.1·§8의 '도우미 ` (same text)
- :709 -> :768 | `- **일의 속도**(개발자 2026-11-09 '더 빠르게' — 이 커밋부터): 러너 `-Tier full`은 커밋마다 한 번이다 — 그 안의 두 판이 CLAU` (same text)

## `docs/60_DECISIONS/DECISIONS_LOG.md` (changed since cef8cc7: 1 hunks)
- :34-37 -> :34-37 | `- `[B-6b2b]`(2026-11-09 — 결정 5의 장치, 헤더 §2.2·§3.1, §3.3 규칙 5와 규칙 8의 로그 행 몫): 절멸 위기의 만료 훅, `` ... `- `[B-6b3b]`(2026-11-09 — 결정 34의 장치, 헤더 §5.1 전부·노트 §2.1): 헤더` (same text)

## `docs/10_DESIGN/crisis_and_transcendents.md` (changed since cef8cc7: 1 hunks) - markers
- :24 -> :24 | `| `Hazard.Extinction` | 절멸 위기 — 그 성계를 가진 백성 자신의 위기(셋째 칸 이상인 백성의 정착된 성계에 추첨으로 선다 — 아래 '절멸 위` (same text)
- :40 -> :40 | `**절멸 위기(`Hazard.Extinction`)**(B6b — 만료의 코드는 지금 코드, 행 `crisis_species_extinction`과 방주는 `[B` (same text)
- :67 -> :67 | `'한 은하에 넷째 칸 이상의 존재 3~7'(§3.1)은 설계 값이고 이렇게 센다: 그 은하에 영향권을 둔 미봉인 Tier 4 타락(≤ 3) + 독립 초월자(Pha` (same text)
- :115 -> :115 | `- ⑤ 피난(Evacuate) — 영압 카드의 선택이 아니라 절멸 위기의 답이다(B6b 헤더 결정 21 — §16 #138을 닫았다): 피난할 사람이 있는 위기는` (same text)
- :139 -> :139 | `AI 문명도 같은 업적 조건으로 승천한다 — 플레이어와 같은 `DT_Feats` 행과 조건으로 판정받는다. 설계가 '단순화된 판정'이라 부른 것은 셋이다: 생애가` (same text)
- :175 -> :175 TOUCHED | `**사망**: `EEffectType::KillPlayer` 효과로만 발생한다(사인 태그 `Death.*` + SourceRow). 발생 지점은 데이터 3곳: `` (same text)
- :177 -> :177 | `**사망 파이프라인**(B5 — `UPlayerRoleSubsystem::Die`·`Succeed`; 고정 순서, §7.4 틱 순서처럼 결정론 대상 — 세부는 [` (same text)
- :266 -> :266 | `**은혜 이벤트 3종** (`DT_Events`, Category = `Event.Grace.*`, Severity 2, 쿨다운 없음(`CooldownTicks`` (same text)

## `docs/10_DESIGN/pillars.md` (unchanged since cef8cc7 - every line holds) - markers
- :105 = :105 | `| T3→T4 | 절멸 위기 종족을 차원 방주로 이주 (프로젝트 + 외교 조약) — 제 백성의 절멸 위기에 방주로 답하고 조약 하나를 지닌다(`feat_passa`
- :108 = :108 | `업적 정의는 데이터(`DT_Feats`)이며, 조건은 §7.3의 조건 구조체로 표현한다. 위 표는 전체 8개이고 수직 슬라이스는 통치자 경로의 다섯 행이다 — T`

## `docs/10_DESIGN/systems.md` (unchanged since cef8cc7 - every line holds) - markers
- :65 = :65 | `- **AI의 외교**: AI는 우호를 지니지 않고 불가침을 깨지 않는다. 조약을 청하는 길은 하나뿐이다 — 판정받는 행이 조약을 물을 때만 서는 **조약 걸음*`
- :91 = :91 | `**두 칸 — 건설 칸과 대응 칸**(거대 프로젝트와 승천 판정 작업, §16 #23의 개혁·구호 쪽): 성계마다 프로젝트 칸이 둘이다. 어느 Hazard 행의 `
- :229 = :229 | `| 사망(강제) | `EEffectType::KillPlayer` (§4.4) | §4.4 사망 파이프라인(B5): 생애를 닫고(`FLifeRecord`), 통치`
- :409 = :409 | `**성식종과 항성 위기**: 항성 위기의 문구는 별 등급마다다 — M·K·G '항성 폭풍', O·B·백색왜성 쌍성 '초신성', 업적·프로젝트 표시명 '항성 안정화`
- :415 = :415 | `**① 보유 상한** `DT_Rules.RealmCapByTier = {1, 3, 4, 7, 10}`(T1~T5. 인덱스 = `static_cast<int32>(`
- :417 = :417 | `**② 넷째 칸의 제동** `FGalaxyDef.AIFourthRungCap`(기본 2, 0 = 제동 없음). AI 간이 판정 전용 노브다 — 업적 판정이 AI `

## `docs/15_GLOSSARY.md` (unchanged since cef8cc7 - every line holds) - markers
- :151 = :151 | `| 넷째 칸 이상의 존재 / 넷째 칸의 제동 | Beings of the fourth rung and above / Fourth-rung brake | `FGal`

## `docs/20_ARCHITECTURE.md` (changed since cef8cc7: 12 hunks) - markers
- :109 -> :109 | `- **보이는 백성** = 만났고 보는 쪽보다 세 칸 이상 위가 아닌 백성 — 이야기의 범위의 주제 규칙 그대로다(`CosmicMapLook::IsShownPeo` (same text)
- :212 -> :212 | `**B6b와 추첨**(헤더 `docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md` §2.12 — 입력·자리마다` (same text)
- :289 -> :289 | `틱 처리 순서(열 단계): 경제 → 연구 → 외교 → 분쟁 → 위기 → AI → 건넘 → 플레이어 이동 → 앎 → 역할 효과 해소. 변경 시 `docs/21` §` (same text)

## `docs/21_DATA_MODEL_REFERENCE.md` (changed since cef8cc7: 8 hunks) - markers
- :629 -> :629 | `CivHasTreaty // (B6b — 끝에 더했다 — 헤더 `headers/B6b_upper_rungs_and_the_race.md` 2.3절): Tag = ` (same text)
- :763 -> :763 | `UPROPERTY(EditAnywhere) TArray<FCosmicEffect> OnExpireEffects; // 주제 = 그 성계와 주인, SourceRow` (same text)
- :973 -> :973 TOUCHED | `// DT_LogTemplates (§5.11 계시 양식, §8.2). 이벤트가 아닌 틱 로그(경제·연구·소유권 변동)와 방문·사자 템플릿. 만남의 줄(log_f` (same text)
- :1266 -> :1266 TOUCHED | `- 그 밖: `words`(닫힌 낱말 목록 — 키와 값이 쓰는 아홉 resource·axis·stat·role·civ_selector(Former·Ruled — ` (same text)
- :1458-1463 -> :1458-1463 TOUCHED | `// 세계 함수(공개, static이 아닌 이름공간 — 상태·색인·테이블을 받고 줄을 내지 않는다): CosmicHazards::Place·Resolve·Tran` ... `// B6b(헤더 docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_` (same text)
- :1518 -> :1518 | `void TickHazards(); // 발생 추첨은 정착된 성계만 돈다 — 무주 성계와 정착 중인 성계는 뽑지 않고 건너뛴다(추첨 수는 소유를 따르고 시드에 대` (same text)
- :1520 -> :1520 | `// 인물 훅 ② (§1.3, §7.2, §7.5): crisis_rebellion 발생 처리가 전조 ② 행을 직접 Fire하고 그 효과 NameNotable(N` (same text)
- :1585 -> :1585 | `- ⓞ **정리**: 소유 성계가 0인 문명을 배열 순서로 제거한다(파생 기준이라 표지를 저장하지 않는다). 제거할 때 다른 모든 문명의 `Relations`와 ` (same text)
- :1658 -> :1658 | `// - AI 간이 판정의 T3→T4에는 넷째 칸의 제동(FGalaxyDef.AIFourthRungCap)이 걸린다 — 셈의 정의는 §7.4 건넘 단계이고 B6b` (same text)

## `docs/40_DEV_GUIDE.md` (changed since cef8cc7: 10 hunks) - markers
- :25 -> :25 | `- 해시에 드는 산술은 모듈 안의 `+ − × ÷ √`만. 시드 42 세계 해시 기준값은 `Tools/baselines/seed42.json`에 두고 `Tools` (same text)
- :237 -> :237 | `| `Cosmic_Ascension.Data.Integrity` | B6b `[B-6b2c]`(헤더 §2.2·§2.5·§2.6·§2.10, 노트 §2 ⓑ): 방주` (same text)
- :238 -> :238 | `| `Cosmic_Ascension.Feats.Reachable` | AI만으로 20,000틱(시드 42, 게임 데이터) 돌려 최소 1개 문명이 T3 도달. Ha` (same text)
- :496 -> :496 | `**부름의 줄과 측정 인자**(B5, 2026-10-30·31 — 헤더 `docs/60_DECISIONS/headers/B5_death_succession_two` (same text)

## `docs/70_CONTENT_GUIDE.md` (changed since cef8cc7: 7 hunks) - markers
- :129 -> :129 TOUCHED | `수치·자원·칙령·유물·Tier 이름에 성령·은혜·기름부음·성결을 쓰지 않는다(성령력·은혜 포인트·기름부음 게이지 금지 — 스키마가 지키는 자리는 아래 '칙령의 이` (same text)

## `STATUS.md` (changed since cef8cc7: 4 hunks) - lead file (read only)
- :27 -> :27 TOUCHED | `- **기준값**(다섯 줄과 해시 판 둘의 두 해시 모두 B6b `[B-6b1]`부터 — 백성의 셈 둘이 해시에 들고 세계는 그대로, CSV는 `[B-6a7]`과` (same text)
- :30-32 -> :30-32 TOUCHED | `` ... `2. **B6b 구현**(헤더 B6b §8의 차례 — `[B-6b1]`·`[B-6b2a]`·`[B-6b2b]` (same text)

