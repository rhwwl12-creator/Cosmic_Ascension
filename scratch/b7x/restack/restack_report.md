# B7x 다시 쌓기 보고 — integrate/b7x (빌드 전)

- 작업 사본: `scratchpad/b7x/int` (브랜치 `integrate/b7x`), 바탕 `220129d` ([B-6b7]). 푸시하지 않았다. main 작업 사본은 건드리지 않았다.
- 최종 커밋: **`3f76f308e3b6cde4dce7ae1be13e81be406a0aac`** `[B-7x1+2] 필드와 장치 — 통합 브랜치, 빌드 전(01eef6f·492c4f0·5969bd5·85f297c를 220129d 위로 다시 쌓음)`. 메시지 파일: `restack/msg_b7x12.txt`.
- 고른 차례: 01eef6f → 97d1365 (충돌 없음), 492c4f0 → 131b4a6 (충돌 셋), 5969bd5 → 886d3a0 (충돌 하나), 85f297c → d13c130 (충돌 없음). 그다음 `reset --soft 220129d`로 하나로 합쳤다(32 파일, +2761 −57).
- rerere: 풀이 넷이 기록되었다(`ST_CA.csv`, `schema.json`, `test_data_schema.py` 두 번). B6b가 끝난 뒤 리베이스할 때 같은 충돌은 저절로 풀린다. 다만 `walked` 핀은 그때 다시 세야 한다.
- 시작할 때 하나 고쳤다. 작업 사본을 만든 직후 index의 stat에는 CRLF 크기가 적혀 있었고, 디스크 파일은 LF였다. git은 크기가 다르면 내용을 견주지 않고 '바뀜'으로 보기 때문에 cherry-pick이 '로컬 변경이 덮인다'며 멈췄다. 진짜 변경이 0인 것을 확인한 뒤 `git -c core.eol=lf reset --hard HEAD`로 stat만 다시 적었다. 내용은 바뀌지 않았다.

## 1. 충돌과 해소 (헤더 §8.1대로, main을 앞에)

| 파일 | 어디서 | 해소 |
|---|---|---|
| `Tools/data/ST_CA.csv` | 492c4f0. main의 끝(`Rung.*`, [B-6b4]·[B-6b5]의 키, `Rung.Lost`)과 브랜치의 새 줄이 같은 자리에 붙었다 | main의 줄을 모두 두고, main의 마지막 줄 `Rung.Lost`(462행) 뒤에 `"Settle.Block.Inward"`(463행)를 두었다. `Edict.TipWide`([B-6b7]의 제자리 고침)는 닿지 않았다 |
| `Tools/data/schema.json` `words` | 492c4f0 | main의 목록은 글자 그대로 두었다([B-6b5]의 13낱말 `holy_passive`와 아홉 목록, [B-6b6]의 `ark_words`). 브랜치의 옛 `holy_passive`(8낱말)·`record_object_words` 사본은 버렸다. `time_vectors` 한 줄만 `words`의 마지막 항목으로 더했다(77행, `ark_words` 뒤). JSON 파싱과 `check_data`에서 중복 키가 나지 않았다 |
| `Tools/test_data_schema.py` `walked` 핀 | 492c4f0 (316 대 277), 5969bd5 (322 대 278) | 손으로 더하지 않았다. main 값으로 둔 채 `SchemaTests.test_every_rule_of_the_schema_can_fail`를 돌려 실제 수를 읽고 그 수로 맞췄다(§2) |

브랜치의 나머지 스키마 변경은 브랜치가 쓴 대로 저절로 들어갔다. 위치는 모두 main 항목 뒤다.

- 뿌리 목록 `["Trait.", "Brick."]` 넷
- `DT_Rules` 열 열일곱. `GalaxyLawsToHold` 뒤에 붙었다.
- 배수 규칙 하나. `AIUpperRungProgressMultiplier` 규칙 뒤다.
- `DT_Events`·`DT_Hazards`의 `BorderingCivHasTrait` 금지 규칙 둘
- 조건 셋. `CivPressureEnduredTicksReached` 뒤다.

**저절로 합쳐진 곳도 다시 읽었다.**

- `Config/DefaultGameplayTags.ini`: [B-6b6]의 `:20` 고침과 B7x 태그 열일곱이 서로 다른 자리에 있다.
- `Tools/data/DT_Rules.json`: main 값을 그대로 두었다(`AIUpperRungProgressMultiplier` 0.375 등). 노브 열일곱은 끝에 무동작 값으로 붙었다.
- `Tests/CosmicDataTests.cpp`, `Tests/CosmicSimTests.cpp`, `Tests/CosmicPanelTests.cpp`, `Tests/CosmicBalanceCommandlet.cpp`, `UI/CosmicSystemPanelWidget.cpp`: 문제없다.
- `docs/30_CONFIG_REFERENCE.md`: 저절로 합쳐진 글이 이미 생성기의 결과와 같았다. 마지막 고르기 뒤 `make_config_reference.py`로 다시 썼고(노브 117, 읽힘 108), 차이는 없다.

**알짜 차이 대조.** 브랜치의 알짜 차이(`9be5b3e..85f297c`, 같은 32 파일)와 통합의 알짜 차이(`220129d..3f76f30`)를 `index`·`@@` 줄을 빼고 견주었다. 다른 곳은 세 가지뿐이다.

- main의 문맥 줄
- 위 세 해소
- 패널 hunk 곁의 main 줄 `ReasonOf(…, Tier)`·`SettleReason = Reason;`

## 2. walked 핀의 셈

| 시점 | 수 | 까닭 |
|---|---|---|
| 브랜치 바탕 `cef8cc7` | 271 | |
| 브랜치 492c4f0 / 5969bd5 | 277 / 278 | 브랜치 몫 +6, +1 |
| main `220129d` | **316** | [B-6b5]·[B-6b6]·[B-6b7]이 더한 then-test |
| 492c4f0을 고른 뒤 실제로 센 수 | 322 | +6. `DT_Rules` 배수 규칙의 then 넷, `DT_Events`·`DT_Hazards`의 `BorderingCivHasTrait` 하나씩 |
| 5969bd5를 고른 뒤 실제로 센 수 | **323** | +1. 배수 규칙에 `BrickStellarCrisisSpawnMultiplier` |
| 85f297c 뒤(최종) | 323 | 0. 까닭 글만 바뀌었다 |

정리하면 main 316에 브랜치 몫 +7을 더해 323이다. 323은 테스트를 돌려 읽은 수다(`Tools/test_data_schema.py:3978`). B6b가 끝난 main 위로 다시 쌓을 때는 [B-6b8] 이후의 then-test가 더해지므로 다시 센다.

## 3. 정착 사유의 수

- main `220129d`의 `ECosmicSettleBlock`(`Sim/CosmicMegaprojects.h:184`)은 `cef8cc7` 뒤로 바뀌지 않았다. Count 앞의 값은 11개다(None, Sanctuary, NoPeople, Owned, Dead, NoLane, NoRow, RowTier, RealmCap, Settling, Sphere).
- 끝에 `Inward`를 더해 **Count = 12**가 되었다.
- static_assert와 단언은 모두 12에 맞는다.
  - `Sim/CosmicMegaprojects.cpp:569` `== 12`
  - `UI/CosmicSystemPanelWidget.cpp:1101` `== 12`
  - `UI/CosmicSystemPanelWidget.cpp:85`: 열쇠 배열 12칸 == Count, 끝이 `Settle.Block.Inward`
  - `Tests/CosmicBalanceCommandlet.cpp:1099`: 이름 배열 12칸 == Count, 끝이 `Inward`
  - `Tests/CosmicExistenceBricksTests.cpp:825`: TestEqual 12
- main의 연결은 그대로다: `UI/CosmicSystemPanelWidget.cpp:1097`의 `ReasonOf(ShownSettleBlock, Tables, State.Civs[RuledIndex].Tier)`와 `SettleReason = Reason;`.
- `Settle.Block.RealmCapLast`는 열거자가 아니라 `ReasonOf`가 RealmCap에서 고르는 키다. `Inward`는 `SettleBlockKeys[11]`로 `Settle.Block.Inward`를 읽는다.

## 4. API 드리프트 (`git diff cef8cc7 220129d -- Source/`)

main이 바탕 뒤에 고친 Source는 `Tests/`·`UI/`·`World/`의 35 파일뿐이다. `Sim/`·`Core/`·`Data/`·`Events/`·`Ascension/`·`Player/`·`Log/`·`Save/`는 한 줄도 바뀌지 않았다. 그래서 B7x 코드가 부르는 시뮬·코어·데이터·이벤트 선언은 브랜치가 본 것과 같다.

선언이 바뀐 것과 B7x의 사용:

| main 선언 변화 | 커밋 | B7x 사용 |
|---|---|---|
| `UCosmicSystemPanelWidget::ReasonOf(Block, Tables)` → `+ EAscensionTier RuledTier = Tier1_Planetary` | [B-6b4] | 직접 부르지 않는다. B7x hunk(열쇠 배열·static_assert·`bHint`)가 main의 부름 곁에 합쳐졌고, main의 인자가 남았다. 고칠 것 없음 |
| `UCosmicTopBarWidget::ClockTipOf(EndTick, bEndOn)` → `+ bool bRungRule` | [B-6b4] | 쓰지 않음 |
| `CosmicNextRung::TextOf`/`TextOfAll` → `+ ECosmicTimeUnit Unit`; 새 `HasJudgedRowAt`/`HasJudgedRowAskingAt`; 열거형에 `Treaty`·`Laws`; `FCosmicNextRung` + `TicksLeft`·`bClosed`·`bLost` | [B-6b4] | 쓰지 않음. 새 조건 셋은 다음 칸 툴팁에서 'Other'로 접힌다 |
| `CosmicAscensionOverlay::LookOf`, `FCosmicAscensionLook`, 오버레이 getter | [B-6b4] | 쓰지 않음 |
| `UCosmicEdictWidget::RungWordsOf`, `GetRowWideMark`, `GetPeopleRung`, 새 UPROPERTY 둘 | [B-6b4] | 쓰지 않음(외교 창 툴팁은 [B-7x3]의 몫) |
| `CosmicAutoPause::RulerDiesAtExpiryOf` | [B-6b4] | 쓰지 않음 |
| `Tests/CosmicTestHelpers.h`, `Tests/CosmicB6bRowNames.h` | [B-6b6]·[B-6b7] | 주석만 바뀌었다. B7x 시험이 쓰는 도우미(`AcquireSim`·`WithRules`·`WithoutProjects`·`WithoutStartingEdict`·`NewTable`·`WireChain`·`ClearOwners`·`Give`·`StandingProject`·`SliceSystems`·`SliceCivs`·`Flags`)의 선언은 같다 |
| 반대 방향: B7x가 지운 파일 안 `StarClassNames` | — | main 코드는 그것을 부르지 않는다(grep 0) |
| 반대 방향: B7x가 바꾼 공개 선언 | — | 없다. 지운 줄은 주석, 파일 안 상수, 식 안의 곱 자리뿐이다 |

**기계적으로 고칠 것이 없어 amend하지 않았다.** 커밋 메시지에 'API 맞춤' 항목도 없다.

## 5. 컴파일 위험 (판단이 필요한 것 — 이 코드는 한 번도 컴파일되지 않았다)

새 파일 셋(`Sim/CosmicExistenceBricks.h/.cpp`, `Tests/CosmicExistenceBricksTests.cpp`)과 B7x hunk를 220129d의 선언에 대어 읽었다. **확정된 컴파일 오류는 찾지 못했다.** 확인한 것과 컴파일러만 판정할 수 있는 것을 적는다.

1. **이름 가림(C4456/C4457/C4458)**. 노트 §5.2 (1)이 짚은 자리를 읽었다.
   - `Sim/CosmicMegaprojects.cpp:698`의 `Step`: `Advance(Ctx, SystemIndex, Slot, AIRate)` 안에 같은 이름의 바깥 선언이 없다. `CosmicMegaprojectsImpl`과 모듈 전역에도 `Step`이 없다.
   - `Tests/CosmicExistenceBricksTests.cpp`의 `Case`(:403), `Income`(:547), `Lay`(:828), `Holds`(:1179), `ShapeWith`(:1015), `Moves`(:213): 매개변수와 지역 이름이 앞서 선언된 바깥 지역 이름과 겹치지 않는다.
   - `FAutomationTestBase` 멤버(`TestName`·`TestParameterContext`·`ExecutionInfo`·`ExpectedMessages`·`ActionCS`·`bComplexTask`·`bRunOnSeparateThread`·`bSuppressLogs`)를 쓰는 지역 이름도 없다.
   - 판정은 빌드가 한다. 위험은 낮다.
2. **`TestEqual`의 오버로드**(노트 §5.2 (2)). UE 5.8 `Misc/AutomationTest.h:1985-2003`에 int32·int64·SIZE_T·float(허용 오차)·double·FName 오버로드가 있다.
   - `Tests/CosmicExistenceBricksTests.cpp:1078`의 `TestEqual(…, CosmicSeed::Purpose::Brick, 13u)`는 템플릿 `TestEqual<uint32>`(:2186)로 간다.
   - main의 `Tests/CosmicDigestTests.cpp:274`가 같은 꼴(`Purpose::Digest, 12u`)로 이미 컴파일된다. 위험 아님.
3. **`Sim/CosmicExistenceBricks.cpp:20-82`의 이름공간 `CosmicBricksImpl`**. inline이 아닌 함수 일곱과 FName 상수 여섯이 있다.
   - 이름공간 이름이 모듈에서 유일해 unity 빌드에서도 겹치지 않는다(grep).
   - FName 상수는 이름공간 범위의 `const`라 내부 연결이다.
4. **이 파일이 부르는 선언을 모두 220129d에서 대조했다.** 시그니처가 다른 것은 없다.
   - `FCosmicStateIndex::FindSystem`/`FindCiv`
   - `CosmicState::IsSettled`·`LaneDistancesFrom`(4번째 기본 인자)·`IsSanctuary`
   - `CosmicHazards::IsDeadNode`·`Place`·`EmptyHazardSlot`
   - `CosmicDiplomacy::BordersOf`·`TargetRelationOf`
   - `CosmicMegaprojects::SettleBlockOf`·`SettleTargets`·`PeopleSettleBlockOf`·`MaySettle`·`Settle`
   - `CosmicEdicts::AIRevokesOf`
   - `UCosmicEventSubsystem::Check`(public static)
   - `FCosmicEvalContext`/`FCosmicEvalView` 생성자
   - `UGalaxySimulationSubsystem::GetStateIndex`·`RebuildStateIndex`·`SyncRngState`·`GetRng`·`StepOnce`·`RunHeadless`
   - `CosmicSeed::DeriveForName`·`Purpose::Brick`
   - `CosmicTags::StarClassAt`·`StarClassCount`
   - `FNameLexicalLess`(엔진 `UObject/NameTypes.h:1691`)
5. **`TickHazards`의 `CosmicBricks::HazardSpawnShareOf(Owner, …)`**(`Sim/GalaxySimulationSubsystem.cpp:2399` 근처). 같은 블록에 `const FCivilizationState& Owner`(:2372)가 있다. `NewGalaxy`의 `Seed`·`bPlayerCiv`·`Rules`도 그 자리에 있다. 위험 아님.

## 6. 시험 위험 (컴파일이 아니라 빌드 뒤 테스트에서 드러날 것 — 다음 단계의 몫)

- **`Data.SourcesInSync`**: 낡은 채로 실패한다. 에디터 없이는 `generated.sha1`과 에셋을 다시 만들 수 없다. 이 커밋이 `DT_Rules.json`(노브 열일곱), `ST_CA.csv`(키 하나), `schema.json`을 바꿨으니 예상된 실패다. 파이썬 검사 다섯은 이 도장을 보지 않는다.
- **패널 시험**(`UI.TopBarAndPanel`의 사유 글): `Settle.Block.Inward`의 글은 ST 에셋을 다시 만들기 전까지 비어 있다(노트 §5.2 (5)).
- **게임 값을 읽는 시험 보드**: `LayBoard`(`Tests/CosmicExistenceBricksTests.cpp:303`)와 `Sim.SeedBricks` Pass 0(:1127)은 게임의 `DT_Rules`를 읽는다. main은 이제 [B-6b6]·[B-6b7]의 값(`AIUpperRungProgressMultiplier` 0.375, 은하 법칙 행, 절멸·방주 행)을 가진다. 보드는 Edicts·Hazards·Events·Encounters·Transcendents를 자기 표로 바꾸고 백성을 첫 칸에 두므로 지금 값으로 깨질 자리는 보이지 않는다. 다만 노트 §5.4의 시1·시2(`WithInertBricks`·`StripBricks`)가 다음 단계의 고침이다.
- **리뷰가 넘긴 고침은 적용하지 않았다**(과제의 범위 밖 — 노트 §5.4 코1~코7·데3·시1~시12). 특히 다음 둘이 남아 있다.
  - 코1: `CosmicEdicts::AIRevokesOf`의 넓은 판(`Sim/CosmicEdicts.cpp`의 UnseenLaw 조기 반환). K9를 깬다. 결정 18이 권장안으로 답해졌으므로 다음 단계에서 좁힌다.
  - 코5: `Core/CosmicTags.h`의 `IsBrick`. 아직 `ESearchCase::CaseSensitive`다.
- **잎 이름 바꾸기(결정 3)도 적용하지 않았다.** `KeptLaw`·`FoldedSmall` 등 열거자 이름은 이미 결정 3의 꼴이다. 태그·접근자·노브의 이름은 다음 단계에서 바꾼다.

## 7. 파이썬 검사 (INT 뿌리에서, `-I -X utf8`)

| 검사 | exit | 요점 |
|---|---|---|
| `Tools/check_data.py` | 0 | `check_data: 0 problem(s)` |
| `Tools/test_data_schema.py` | 0 | `Ran 97 tests … OK`(main 95 + B7x 2: `test_the_trait_roots_hold_the_bricks`, `test_existence_vocabulary`) |
| `Tools/check_layers.py --strict` | 0 | `177 files, 0 error(s), 0 warning(s)`; NOTE 태그 리터럴 48(기준 49), DT_Rules 노브 117 중 읽는 코드 없는 것 9(기준 9) |
| `Tools/make_config_reference.py --check` | 0 | `up to date` |
| `Tools/check_docs.py` | 0 | `43 files, 0 error(s)`(SECTION_MAP NOTE만) |

기록 파일: `restack/chk_data.txt`, `chk_schema.txt`, `chk_layers.txt`, `chk_cfg.txt`, `chk_docs.txt`.

## 8. 줄끝과 깨끗함

- `lf_norm.py <INT> --check`: `still CRLF 0 file(s)`.
- 진짜 변경(numstat에서 0/0이 아닌 줄)은 0이다.
- `git status`는 깨끗하다.
- 더한 Source·Config 줄에 ASCII가 아닌 글자는 0이다.
