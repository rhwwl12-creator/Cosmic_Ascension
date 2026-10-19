# A0 헤더 초안 — 성역(4a)과 첫 생애 행(4b-lite)

상태: **승인 완료 (2026-10-19 결정 6개 전원 권장안 채택, Core/CosmicIDs.h 신설)**. 승인에 따라 Windows에서 4a 성역 → 4b 첫 생애 행 구현·빌드·테스트 2회·`run_checks.ps1 -UpdateBaseline`으로 기준값을 다시 잰다. 근거: `docs/10_DESIGN/crisis_and_transcendents.md` §4.5 ①, `docs/10_DESIGN/systems.md` §5.10·§5.11, `docs/21_DATA_MODEL_REFERENCE.md` §7.4 생성 규칙 3·9·11항, `docs/50_ROADMAP.md` '①-4a·①-4b' 행, `OPEN_QUESTIONS.md` #74·#80·#81·#92·#94, `CLAUDE.md` 불변 규칙 13·14·15.

## 0. 범위와 전제

- 한 헤더, 커밋 둘(4a 성역 → 4b 첫 생애 행). 플레이어 동사를 더하지 않는다 — 영구 ID·시뮬 규칙 한 줄·표현의 그림만 바꾼다.
- 세이브 호환은 만들지 않는다(출시 전): 개발 세이브는 지운다. `SaveVersion`은 그대로 1(델타 저장의 기본값 불변 규칙은 지킨다 — 새 필드는 없다).
- 성역에 닿는 레인과 성역을 지나는 셈(#80)은 **단계 5(실제 하늘)의 몫**이다. A0의 성역은 레인이 없는 노드다 — 지도는 성역 없이도 이어지고(결정 7), 레인을 세는 어떤 규칙도 바뀌지 않는다.
- 가칭 식별자는 아래 §1의 값으로 **일괄 확정**한다(세이브와 스키마에 남아 배포 뒤 동결 — §16 #74).

## 1. 식별자 확정 제안

| 가칭 | 확정 제안 | 자리 |
|---|---|---|
| `Law.Hedge` | `Law.Hedge` 그대로 | `Config/DefaultGameplayTags.ini` 5번째 법, `DT_Rules.ImmutableLaws` 5번째 원소 |
| `CosmicState::IsSanctuary` | `CosmicState::IsSanctuary(FName SystemID)` — ID만 본다(`SystemID == CosmicIDs::Sanctuary`), 상태를 읽지 않는 순수 함수 | 새 헤더 `Core/CosmicIDs.h`(`CosmicIDs::Sanctuary = "sol"`, `CosmicIDs::FirstHome = "sys_0"`, 술어 둘) — Core에 두어 Sim·World·UI·Tests가 같은 답을 읽는다 |
| `sys_0` / `sol` | 그대로. `sys_0` = 첫 생애의 모성계(배열 0번), `sol` = 성역(배열 끝, 노드 61) | `GalaxySimulationSubsystem.cpp` `NewGalaxy` 1항·11항 |
| `ShownByName` | `FCosmicKnownView::ShownByName`(`TBitArray<>`, 성계 인덱스) + `IsShownByName(int32)` | `World/CosmicMapLook.h` — 앎을 읽는 곳은 여전히 `KnownViewOf` 하나(불변 규칙 14). A0에서 이 비트가 참인 성계는 성역 하나 |
| 정착 사유 | `ECosmicSettleBlock::Sanctuary` — 열거형 **맨 앞**(`None` 다음), ST_CA `Settle.Block.Sanctuary` | `Sim/CosmicMegaprojects.h`, `Tools/data/ST_CA.csv` |
| 영적 시야의 말 | ST_CA `Sight.Sanctuary` "이곳에는 압력이 닿지 않는다." / 주인 줄 `Panel.Sanctuary` "이곳은 지켜지고 있다." | `ST_CA.csv`, `CosmicMapLook::SpiritOf`·`SpiritTipOf`·`OwnerLineOf` |
| `arch_seeker` / `Trait.Arch.Seeker` | 그대로 | `DT_Civs.json` 새 행(`arch_terran` 행과 `Trait.Arch.Terran` 태그는 지운다 — 개발 세이브를 지우므로 가능), `DT_Rules.PlayerArchetypeRow`, `FCosmicRulesDef::PlayerArchetypeRow` C++ 기본값, `CosmicRows::ArchSeeker` |
| 첫 줄 | `log_first_life` = "{HomeSystem}. 이 백성이 당신에게 맡겨졌다." (변형 1개 허용 — §7.3) | `DT_LogTemplates.json` |
| 테스트 | `Cosmic_Ascension.Sim.SanctuaryKept`, `…Data.Integrity` 항목 추가, `…UI.KnownFilter` 성역 경우 | `Tests/` |

## 2. 변경 목록 (파일 → 바꾸는 것)

### 4a 성역 (커밋 1)

| 파일 | 변경 |
|---|---|
| `Config/DefaultGameplayTags.ini` | `+Law.Hedge`(DevComment: "Hedge: sol belongs to no people - never owned, settled, claimed, cored, targeted or destroyed; pressure 0; vessels never stand there (section 4.5 (1)).") |
| `Tools/data/DT_Rules.json` · `schema.json` | `ImmutableLaws`에 `Law.Hedge` 추가(5개). 스키마: `ImmutableLaws`가 다섯 태그를 모두 담아야 한다는 규칙(`contains_all`) — 빠지면 `check_data` 실패 |
| `Core/CosmicIDs.h`(새) | `namespace CosmicIDs { inline const FName Sanctuary(TEXT("sol")); inline const FName FirstHome(TEXT("sys_0")); }` `namespace CosmicState { inline bool IsSanctuary(FName SystemID) { return SystemID == CosmicIDs::Sanctuary; } }`. `GalaxySimulationSubsystem.cpp:45`의 지역 상수 `PlayerHomeSystemID`는 지우고 `CosmicIDs::FirstHome`를 쓴다 |
| `Sim/GalaxySimulationSubsystem.cpp` `NewGalaxy` | 1항: 0번 성계의 ID를 `sys_0`으로(자리·등급·풍부도 추첨은 지금 그대로 — 추첨 수 불변). 2항(모성계)·9항(초기 코어 — 거리를 `sys_0`에서 잰다)·`Install(CosmicIDs::FirstHome, …)`. **11항(새)**: 모든 추첨이 끝난 뒤 성역을 배열 끝에 붙인다 — `SystemID = sol`, `GalaxyRow` = 고향 행, `DisplayName` = ST_CA `System.Sanctuary`("태양계" — UI 라벨에만; 백성의 목소리는 '옛 땅'), `Position` = 원반 밖 고정 자리(반지름 1.15R, 각도 0 — 추첨 없음, `Derive`도 쓰지 않는다), `Lanes` 비움, `StarClass` `Star.Class.G`, `ResourceRichness` 0, 주인·위기·프로젝트 없음 |
| `Sim/GalaxySimulationSubsystem.cpp` 틱 | `ComputeSpiritualPressure(sol)` → 0을 바로 돌려준다(원천 합산 전). 그 밖의 단계는 '정착된 성계'·'무주 성계' 술어 위에서 돌아 성역(주인 없음·레인 없음)을 이미 지나친다 — 그 사실을 `Sim.SanctuaryKept`가 20,000틱 동안 단언한다(코드 분기를 더하지 않는다) |
| `Sim/CosmicMegaprojects.h/.cpp` | `ECosmicSettleBlock::Sanctuary`를 `None` 다음에 두고 `SettleBlockOf`가 첫 사유로 답한다. `PeopleSettleBlockOf`(AI의 대상 고르기)는 성역을 후보에서 뺀다(12번 작업의 `static_assert` 둘을 새 값에 맞춘다). 초기 코어 후보(`NewGalaxy` 9항)도 `IsSanctuary`로 거른다 |
| `Sim/CosmicHazards.cpp` | `Place`의 첫 가드에 `IsSanctuary → 거절(에러 한 줄 없이 false)` — 효과 `SpawnHazard`가 성역을 가리켜도 아무 일이 없다. 주장 규칙은 무주 성계를 이미 거르지만 명시 가드를 둔다(법은 한 곳에서 읽힌다) |
| `Player/PlayerRoleSubsystem.cpp` | `Install`·`BecomeWanderer`·(단계 4b 뒤의 `TravelTo`)가 `sol`을 받으면 에러 한 줄 + 무동작. 그릇의 `CurrentSystemID`는 결코 `sol`이 아니다(#81) |
| `Save/CosmicSaveSubsystem.cpp` | `LoadGalaxy`: 마지막 성계가 `sol`이 아니거나 `sol`에 주인·위기·프로젝트가 있으면 로드 거절(에러 한 줄, 새 판 안내). 개발 세이브는 지운다 |
| `World/CosmicMapLook.h/.cpp` | `FCosmicKnownView::ShownByName` 비트(성역 = 참). `ShownLookOf`·`LightSlots`·`PickCandidatesOf`·`KnownLanesOf`가 `IsKnown ∨ IsShownByName`으로 성역을 그리고 고른다(이름표·고리·레인·표식 없음 — 별 하나와 이름). `OwnerLineOf`의 첫 줄 = `Panel.Sanctuary`. `SpiritOf` 첫 경우: 색 없음·압력 0; `SpiritTipOf` → `Sight.Sanctuary`. `CosmicUI::NoLifeLineSystemID`를 `IsSanctuary`로 바꾼다 |
| `World/CosmicGameMode.cpp`·`UI/CosmicSystemPanelWidget.cpp` | `SelectSystem`·성계 패널의 앎 문(`Refresh`)이 `ShownByName`을 함께 읽는다. 패널의 정착 단추: 사유 `Sanctuary`면 회색 + 값 줄 없음 |
| `Tools/data/ST_CA.csv` | `System.Sanctuary` "태양계", `Panel.Sanctuary` "이곳은 지켜지고 있다.", `Settle.Block.Sanctuary` "이곳은 팔리지도 차지되지도 않는다.", `Sight.Sanctuary` "이곳에는 압력이 닿지 않는다." |
| `Tests/CosmicSanctuaryTests.cpp`(새) | `Sim.SanctuaryKept`: 시드 42·7, 전부 쌓는 모드, 20,000틱 — 매 틱 `sol`은 배열 끝·주인 없음·`ActiveHazard` 없음·프로젝트 없음·`ComputeSpiritualPressure` 0·레인 0, 초기 코어에 없음, `Knowing.KnownSystems`에 없음(앎에 쓰이지 않는다), `SettleBlockOf(any, sol) == Sanctuary`, `Place(sol)` 거절, `Install(sol)` 거절. `Data.Integrity`: `ImmutableLaws` 5개·`Law.Hedge` 선언. `UI.KnownFilter`: 새 판에서 보이는 성계 = `sys_0` + 성역(이름만) |

### 4b-lite 첫 생애 행 (커밋 2)

| 파일 | 변경 |
|---|---|
| `Tools/data/DT_Civs.json` | `arch_terran` 행 삭제, `arch_seeker` 행 추가: `DisplayName` ""(백성 이름은 `NameBank`에서 — 고정 뱅크 3개는 **개발자 후보**: "갈라진 집"·"찾는 자들의 연맹"·"첫 불의 의회" → 단계 7에서 다시 쓴다), `Traits` [`Trait.Arch.Seeker`, `Trait.Society.Divided`, `Trait.Biology.Shortlived`], `EthicsPreset` 0/0/+0.2, `Weight` 0, `HazardSpawnMultipliers` {`Hazard.AIAwakening`: 1.2}, `RevelationMode` `Revelation.Seeking`, `RevelationTemplates` ["어느 제단에도 이름이 없었다. 그들은 빈 자리에 제단을 하나 더 세웠다." — `arch_default`의 줄을 씨앗으로, 단계 7에서 제 줄], `HomeSystemNames` ["(후보) 첫 해"] — 그 백성이 제 해를 부르는 이름, 숫자 없음(**개발자가 고른다**, #92의 별 이름과는 별개), `HomeStarClass` `Star.Class.G`, `DedicationBanks` [`Faith.Bank.Nations`, `Faith.Bank.Civic`], `PersonNameBank` = `arch_default`의 12 이름을 임시로 복사(단계 7에서 외계의 이름 12로), `Color` 0.25/0.55/1.0 |
| `Tools/data/DT_Rules.json` · `CosmicDefinitionRows.h:786` | `PlayerArchetypeRow` = `arch_seeker` (JSON과 C++ 기본값 둘 다) |
| `Tools/data/schema.json` | `DT_Civs.required_rows` += `arch_seeker`; 규칙: `PlayerArchetypeRow`가 가리키는 행은 `Weight` 0이고 `Trait.Arch.Seeker`를 첫 원소로 갖는다; `Trait.Arch.Terran` 참조 금지. 낱말 목록 `other_works`·`holy_and_myth`(낱말 경계·조사 떼기)와 이름 3단계 검사의 `DT_Transcendents.Title`·`Lure` 확장은 **텍스트가 들어오는 단계 7로 미룬다**(A0에는 빈 목록만 선언해 스키마 꼴을 잡는다) |
| `Config/DefaultGameplayTags.ini` | `+Trait.Arch.Seeker`, `-Trait.Arch.Terran` |
| `Tools/data/DT_LogTemplates.json` | `log_first_life` 템플릿 → "{HomeSystem}. 이 백성이 당신에게 맡겨졌다." |
| `Data/CosmicRowNames.h` | `ArchSeeker` 추가(`ArchDefault`는 폴백으로 유지) |
| `Tests/` | `Sim.FirstLife`: `arch_terran`·`sol` 단언을 `arch_seeker`·`sys_0`으로, 첫 줄 단언 교체. `Data.Integrity`: 위 스키마 규칙 |

## 3. 재측정·갱신 목록 (세계가 바뀐다 — 성계 61개, 기준값 전부)

`Sim.FirstLife`·`GenerationRules`·`LayersInRange`·`SeedDerive`·`PhaseTwoExit`(증거 시드 다시 고르기)·`KnowingInert`·`Feats.Reachable`·`RealmCap`·`Notables` 둘·`FirstContactTimeline`·`UI.CameraAndSelection`·`UI.KnownFilter`(처음 아는 성계는 하나 → 둘)·`UI.ZoomLevels`·`UI.Backdrop` §3, `Tools/baselines/seed42.json`(`run_checks.ps1 -UpdateBaseline`), `docs/CHANGELOG.md`에 새 해시, `STATUS.md`의 코드 사실.

## 4. 순서와 검증

1. 개발자 승인(§5의 결정) → `/effort xhigh`, ultracode 끔(단독 구현) → 4a 구현 → `run_checks.ps1 -Tier full` 2회 → 커밋 `[A0-4a]`.
2. 4b 구현 → `run_checks.ps1 -Tier full -Data` → `-UpdateBaseline` → 커밋 `[A0-4b]`(메시지에 새 해시).
3. 리뷰 1회(저장·결정론·'법은 한 곳에서 읽힌다' — Agent 도구 2~3개로 충분).
4. 문서: `CLAUDE.md` 15번의 '현재 코드' 줄과 `docs/50_ROADMAP.md` A0 행을 끝으로, `OPEN_QUESTIONS` #74 닫고 `DECISIONS_LOG`에 날짜·근거.

## 5. 개발자 결정 (2026-10-19 승인)

1. **식별자 확정**: §1 표 전체 확정. `Core/CosmicIDs.h` 신설 완료 (`CosmicIDs::Sanctuary = "sol"`, `CosmicIDs::FirstHome = "sys_0"`).
2. **성역 레인**: A0에서는 **레인 없는 노드**(권장안)로 확정 (지도는 성역 없이 연결, 실제 하늘 단계 5에서 레인 검토).
3. **성역의 자리**: 원반 밖 고정 자리(권장안 — 반지름 1.15R, 각도 0)로 확정.
4. **아키타입 정리**: `arch_terran` 행과 `Trait.Arch.Terran` 태그 삭제(권장안) 확정 (개발 세이브 지움).
5. **첫 생애 이름 뱅크**: 현재 후보("갈라진 집"·"찾는 자들의 연맹"·"첫 불의 의회" / "첫 해")를 자리 채움으로 유지하고 단계 7에서 외계 이름으로 확장(권장안) 확정.
6. **금지어 목록**: A0에서는 스키마에 빈 목록(`other_works`·`holy_and_myth`)만 선언하고 채우는 것은 단계 7 텍스트 작업으로 미룸(권장안) 확정.
