# MVP ② 헤더 초안 — 청원 카드 10장과 카드 위젯

상태: **승인 (2026-10-20, 아래 10의 결정 8개 전원 권장안) — 구현 완료**(같은 날, `[A-2a]` 스키마·`[A-2b]` 시뮬·`[A-2c]` 카드 위젯 — `git log --grep 'A-2'`; 다르게 한 것은 `DECISIONS_LOG` 2026-10-20에도 적었다). 카드 글 감사(2026-10-21, ultracode)로 글과 표시 문구가 바뀌었다 — 글의 원본은 `Tools/data/DT_Encounters.json`·`ST_CA.csv`이고, 아래 §5 표의 이름·선택지 낱말과 §6의 문구는 승인 때의 가칭이다. 바뀐 것과 그 까닭은 `DECISIONS_LOG` 2026-10-21. 구현에서 이 초안과 다르게 한 것: 판정 없음의 열거자는 `None`이 아니라 `NoCheck`다(데이터가 빈 이름을 "None"으로 써서 검사기의 시험이 둘을 가르지 못한다). 목소리 없는 카드는 `Body`, 목소리 있는 카드는 `VoiceBodies` 둘만 쓰고 `Body`를 비운다(스키마 규칙 둘 — 빈 목소리 원소는 두지 않는다). 그래서 '지혜로운 마음'은 목소리 없는 카드이고 의회가 말한다는 것은 본문이 적는다. 위젯의 목소리 줄은 목소리 있는 카드에만 선다. 근거: `docs/10_DESIGN/systems.md` §5.9·§5.10·§5.11, `docs/10_DESIGN/crisis_and_transcendents.md` §4.1(반란 ③)·§4.2, `docs/20_ARCHITECTURE.md` '자동 일시정지'·'틱 순서'·'난수', `docs/21_DATA_MODEL_REFERENCE.md` `FEncounterDef`·`FEncounterChoice`·`FKnowingState`, `docs/50_ROADMAP.md` A ②, `docs/70_CONTENT_GUIDE.md` §8.2·§8.3, `OPEN_QUESTIONS.md` #24·#26·#30·#93, `CLAUDE.md` 불변 규칙 2·3·4·5·14.

## 0. 범위와 전제

- 통치자가 받는 청원 카드만 만든다. 방랑자의 조우 카드(B1)와 첫 접촉 청원(#24 태세 저장 위치가 정해지는 Phase 4)은 이 헤더 밖이다. 같은 행 구조체(`FEncounterDef`)와 같은 위젯을 그때 그대로 쓴다.
- 커밋 셋: **A-2a** 스키마와 중첩 구조체 검사(기준값 그대로) → **A-2b** 청원의 시뮬(저장 필드·제시·답 명령·10행; 전체 해시만 바뀌고 세계 해시는 그대로) → **A-2c** 카드 위젯(표현 계층).
- 세이브 호환: 새 저장 필드는 끝에 붙고 기본값이 '카드 없음'이라 지금의 세이브를 그대로 읽는다. `SaveVersion` 1 그대로.
- 핵심 성질 하나를 세운다: **답하지 않은 카드는 세계를 바꾸지 않는다**(`docs/70` §8.1이 첫 접촉 카드에 이미 요구한 성질). 카드를 내놓는 일은 앎 층에만 쓰고 시뮬 난수를 뽑지 않는다. 세계를 바꾸는 것은 플레이어의 답(틱 밖 입력)뿐이다. 증명은 커맨드렛이다. 아무도 답하지 않는 시드 42 판의 **세계 해시가 지금 기준값과 같아야** 한다(전체 해시는 대기 카드만큼 바뀐다).

## 1. 설계 문서의 '수직 슬라이스 10장'과 MVP 10장

`systems.md` §5.10의 10장 가운데 넷은 아직 없는 체계에 기대고, 로드맵은 그 체계를 B에 두었다. 빠지는 넷 자리에 지금 효과로 성립하는 넷을 넣는다. 빠진 넷은 그 단계에서 설계대로 더한다(행 이름은 그때 정한다).

| 설계의 카드 | MVP | 까닭 |
|---|---|---|
| '지혜로운 마음'·파벌 청원 3·'왕좌를 지키겠는가'·헤롯 패턴 청원 | **넣는다**(6장) | 지금 있는 효과로 성립한다. 새로 드는 것은 조건 하나(아래 4.2)와 효과 하나(아래 4.3)뿐이다. 왕좌 카드의 '명예로운 양위'는 방랑자 모드(B1)와 함께 셋째 선택지로 **끝에** 붙인다. 로컬라이제이션 키가 배열 위치라 중간에는 넣지 못한다 |
| 영압 체인 3 | **B4로**(로드맵 'B4 초월자 목소리·교리 3행·영압 카드' 그대로) | 조공(Phase 4)·철야 칙령(B2)·교리 공개(B4)·방주(`Hazard.Extinction`)에 기댄다 |
| 첫 접촉 `petition_first_contact` | **Phase 4로** | 답이 '태세'를 쓰는데 저장 자리(#24)가 없다. 그때까지는 지금처럼 줄만 나온다 |
| (새) 위기 청원 3 | **넣는다** | 트리거 'Hazard 발생'(§5.10)의 몫이다. 역병·깨어난 기계·흔들리는 별 — 다스리는 백성의 성계에 위기가 막 섰을 때 온다 |
| (새) 주기 청원 하나 더 | **넣는다** | 600틱 주기 슬롯이 고를 행이 하나뿐이면 같은 카드가 되풀이된다. 주기 행을 둘로 한다 |

## 2. 상태 (저장 필드)

`FKnowingState`(`Core/GalaxyState.h`)의 **끝**에 둘을 붙인다. 첫 접촉 카드의 자리(`PendingContactCivID`)가 이미 앎 층에 있는 것과 같은 까닭이다. 카드는 '플레이어 앞에 놓인 것'이라 세계 해시 밖이고, 세계(틱 단계)는 이 둘을 읽지 않는다.

```cpp
// The petition card waiting for the player's answer (section 5.10): a DT_Encounters row. NAME_None (frozen initializer)
// = no card. Written by the knowing stage (CosmicPetitions::Present) and by the answer (a chained card); cleared by the
// answer, Install, BecomeWanderer and the removal of the ruled people. The world never reads it.
UPROPERTY()
FName PendingPetitionRow;

// The system the waiting card is about: a settled system of the ruled people for a SettledSystem row, NAME_None for a
// Civilization row (the people itself).
UPROPERTY()
FName PendingPetitionSystemID;
```

- 해시: `HashState`가 리플렉션으로 걷는다. 그래서 둘은 전체 해시에 저절로 들고 세계 해시에서는 `Knowing`째로 빠진다(코드 변경 없음).
- 비우는 자리: 답(대기 카드를 지운다), `Install`(새 생애), `BecomeWanderer`(폐위 포함), `TickCrossings`가 다스리는 백성을 지울 때. `PendingContactCivID`와 같은 자리들이다. `NewGalaxy`는 새 상태라 저절로 빈다.
- 기존 필드 `FLifeRecord.LastChoiceRows`('최근 3개 조우·청원 행')에 처음으로 쓴다. 답할 때마다 `Lives.Last()`의 끝에 행 이름을 붙이고 셋을 넘으면 맨 앞을 지운다. 주기 슬롯은 이 셋을 후보에서 뺀다(되풀이 방지). 비문 화면이 읽는 것은 Phase 4b다.
- **#26 닫음**: 열린 카드를 세이브에 둔다. 판정 난수는 답할 때 뽑는다(아래 3.3). 그래서 카드가 열린 채 저장한 뒤 로드해도 같은 카드가 다시 뜨고, 같은 답이 같은 결과를 낸다. 중단 없이 돌린 판과 갈리지 않는다.

## 3. 틱과 추첨

### 3.1 제시 — 앎 단계의 끝 (규칙 4의 아홉째 단계 안, 세계를 읽고 앎에만 쓴다)

새 파일 `Events/CosmicPetitions.h/.cpp`(순수 함수 namespace — `CosmicHazards`·`CosmicMegaprojects`와 같은 꼴)의 `Present`를 `SimulateTick`이 부른다. 생애가 몸을 입은 통치자일 때(`Embodied` ∧ `Ruler` ∧ 다스리는 백성이 있음)만, 매 틱 부른다. 시야의 간격(`SightIntervalTicks`)과 무관하다.

1. **틱 첫머리 읽기**: `++State.Tick` **전에**, 앎 단계가 견줄 값으로 `Standing` 행마다 조건이 지금 서 있는지를 읽어 둔다. 단위는 (행, 성계)의 쌍이고 지역 변수에만 둔다. 저장도 멤버 상태도 아니다. 대기 카드가 있으면 이 읽기를 건너뛴다.
2. **조건이 막 선 행**: 앎 단계에서 `Standing` 행의 조건(`CheckAll`)이 서 있는데 틱 첫머리에는 서 있지 않았으면 그 쌍이 후보다. 조건이 서 있는 동안 카드가 되풀이되지 않는 것은 이 규칙 덕이다. 쿨다운 저장이 들지 않는다. 틱 사이의 입력(답·명령)이 세운 조건은 첫머리에 이미 서 있으므로 카드를 부르지 않는다.
3. **주기 슬롯**: 후보가 없고 `(Tick − Calling.BornTick)`이 `DT_Rules.PetitionPeriodTicks`(새 노브, 600)의 양의 배수이면, 조건이 서 있고 `LastChoiceRows`에 없는 `Lottery` 행이 후보다.
4. **고르기**: 후보 가운데 `BaseWeight`의 가중으로 하나를 고른다. 추첨값은 `CosmicSeed::DeriveForName(Seed, 다스리는 CivID, Purpose::Petition = 10, (uint32)Tick)`이다(상태 없는 파생 정수, 새 용도 번호). **시뮬 스트림은 뽑지 않는다.** 고른 결과는 앎에만 쓰이므로 규칙 3의 '세계를 바꾸는 추첨'이 아니다. 같은 틱의 나머지 후보는 버린다.
5. **대기 카드가 있으면 아무것도 내놓지 않는다.** 줄을 세우지 않는다(카드는 한 장씩). 그동안 막 선 조건은 사라진다.
6. 단위: `Civilization` 행은 백성 한 번(현재 성계 = 수도, 이벤트와 같다). `SettledSystem` 행은 다스리는 백성의 정착된 성계마다 `OwnedSystems` 순서로 본다. `Named` 행은 슬롯이 보지 않고 사슬(`NextEncounter`)과 뒤의 코드(첫 접촉)만 부른다.

### 3.2 답 — 틱 밖의 명령 `UPlayerRoleSubsystem::AnswerPetition(int32 ChoiceIndex)`

`docs/21`의 가칭 `ResolveEncounterChoice`를 이 이름으로 정한다. 방랑자의 조우는 B1에서 같은 함수를 쓰거나 이름을 넓힌다. 몸통은 `CosmicPetitions::Answer(FCosmicEvalContext&, …)`이고 명령은 가드·문맥·`SyncRngState`만 맡는다(`StartMegaproject`의 꼴).

순서는 다음과 같다.

1. **거절**(추첨 없이 `false`): 방송 중, 몸을 입은 통치자가 아님, 대기 카드 없음, 대기 행이 표에 없음, 선택지 번호 밖, 비축이 비용에 못 미침(`CosmicMegaprojects::CanPay`). 방송 중(규칙 5)과 단추가 막아야 하는 마지막 셋은 에러 한 줄이고, 둘째·셋째는 소리 없이 거절한다.
2. **추첨 1회**: `Roll = (int32)(Rng.GetFraction() × 6)`(0~5)를 뽑는다. 받아들인 답마다 정확히 한 번이고, 판정 없는 선택지와 불가능한 선택지에서도 뽑는다(규칙 3: 추첨 수는 행의 내용을 보기 전에 정해진다).
3. **때가 지난 카드**: 행의 조건을 같은 주어(다스리는 백성 + `PendingPetitionSystemID` — 그 성계가 아직 그 백성의 정착된 성계여야 한다)로 다시 보고, 서 있지 않으면 카드를 지우고 '때가 지났다'로 끝난다. 효과·비용·줄은 없고, 뽑은 수는 이미 쓰였다.
4. **비용**: 다스리는 백성의 `Stockpile`에서 낸다(`PayInFull`). 성공·실패와 무관하다.
5. **판정**: `CheckStat`가 `NoCheck`면 성공이다. 아니면 `능력치 + Roll ≥ Difficulty`이면 성공이다. 능력치 Body·Mind·Causality·Exaltation은 그릇의 값(0~10)이다. 새 낱말 `Stability`는 `(int32)(Stability × 10)`(0~10)이다(**#30 ① 닫음** — Influence는 판정에 쓰지 않고 비용으로만 쓴다). `Difficulty ≥ 16`은 늘 실패한다(10 + 5 < 16).
6. **효과 → 받은 성계의 앎 → 결과 줄 → 뒤따르는 줄**: 이벤트 행의 원자와 같은 순서다. `ApplyAll(OnSuccess|OnFailure, Ctx)`(주어: 다스리는 백성, 성계, `SourceRow` = 행) → `FollowUpKnowing` → 결과 줄 `Push`(`Log.Civ`, Severity 1, `SuccessText|FailureText`, 초점 백성 + `SettledSystem` 행이면 성계, 말하기 스트림 추첨 없음) → `FollowUpNotes`. 델리게이트는 더하지 않는다. 위젯은 명령의 반환값을 읽는다.
7. **기록과 사슬**: `LastChoiceRows`에 붙이고 대기 카드를 지운다. `NextEncounter`가 있고 그 행의 조건이 같은 주어로 서 있으면 그 행이 새 대기 카드가 된다(추첨 없음).
8. `SyncRngState()`.

반환은 `FCosmicPetitionAnswer { EPetitionOutcome Outcome (Refused / Withdrawn / Succeeded / Failed); int32 Roll; FText Line; }`이다. 저장되지 않는 값이다(열거형도 저장되지 않는다).

- 카드에 실을 수 없는 효과(스키마 규칙): `KillPlayer`(사망 파이프라인 Phase 4b), `MultiplyIncome`(틱당 효과만), `GrantPower`·`SpawnHazard`·`TransferSystem`·`NameNotable`(쓰는 행이 오는 단계에서 연다). `DeposeRuler`는 실을 수 있다. 지금처럼 `PendingRoleEffect`에 기록만 되고, 다음 틱의 열째 단계가 폐위하며 `BecomeWanderer`가 대기 카드를 지운다.
- 규칙 2와의 관계: 답 명령은 앎의 대기 카드를 읽는 유일한 Core 코드다. '플레이어는 자기 앞에 놓인 카드에만 답한다'는 입력의 정의이고, 틱 단계는 여전히 앎을 읽지 않는다. `Sim.KnowingInert`는 그대로 성립해야 한다.

### 3.3 결정론 요약

| 지점 | 스트림 | 횟수 |
|---|---|---|
| 제시의 고르기 | `DeriveForName(…, Purpose::Petition, Tick)` — 상태 없음 | 0 (시뮬·말하기 둘 다) |
| 답의 판정 | 시뮬 스트림, 틱 밖(입력 순서의 일부 — `Install`·`DeposeRuler`와 같은 자격) | 받아들인 답마다 1 |
| 결과 줄 | `Push` — 추첨 없음 | 0 |
| 사슬의 다음 카드 | 없음 | 0 |

헤드리스 판(커맨드렛·테스트의 `RunHeadless`)은 답하지 않는다. 그래서 첫 카드('지혜로운 마음', 틱 30)가 끝까지 대기하고 그 뒤로는 카드가 오지 않는다. 그 판의 세계는 지금과 같다. 첫 600틱 '결정 ≥ 15' 측정에 쓸 답 정책(커맨드렛 `-Answer=`)은 MVP ③의 몫이다.

## 4. 데이터

### 4.1 행 구조체 (`Data/CosmicDefinitionRows.h` — 저장되지 않는 정의 행, 에셋이 아직 없어 꼴을 바꿀 수 있다)

| 바꾸는 것 | 내용 |
|---|---|
| `FEncounterChoice::CheckStat` | `FName` → 새 열거형 `ECosmicCheckStat { NoCheck, Body, Mind, Causality, Exaltation, Stability }`(정의 전용, 끝에만 추가). 스키마 `enum` 종류가 그대로 검사한다 |
| `FEncounterDef` 끝에 열 셋 | `ECosmicEventTrigger Trigger = Named`, `ECosmicEventSubject Subject = Civilization`(이벤트 행의 두 열거형을 그대로 쓴다 — 청원에서의 뜻은 위 3.1), `TArray<FText> VoiceBodies`(비었거나 둘: [0] 의회, [1] 회중의 장로들. 둘이면 `Body`는 비운다) |
| 텍스트 슬롯 | `Title`·`Body`·`VoiceBodies`·`Label`·`SuccessText`·`FailureText`는 `{Civ}`·`{System}`(`SettledSystem` 행만)·`{Hazard}`·`{Rebel}`. `{Rebel}`은 다스리는 백성의 열린 `Notable.Rebel` 기록의 이름이고, 없으면 ST_CA `Role.Rebel.Fallback` "반란군"(`docs/70` §8.2의 집단 폴백) |

### 4.2 조건 하나 (`EConditionType` 끝에 추가)

`SystemHazardTicksAtMost` — Tag = `Hazard.*`, Value = 틱(1 이상의 정수). 주어 성계가 그 위기를 갖고 `HazardTicksRemaining ≤ Value`이면 참이다. '왕좌를 지키겠는가'(반란 잔여 50틱)가 쓴다. 카운트다운은 틱마다 1씩 주므로 '막 선 때'가 정확히 50틱에 한 번 온다. 그래서 설계의 '잔여 50틱'(§4.1)과 '< 50'(§5.10)의 엇갈림을 '50틱이 남은 때'로 맞춘다.

### 4.3 효과 하나 (`EEffectType` 끝에 추가)

`FocusResearch` — Key = 분야(Physics·Society·Engineering·Transcendence), Value = 몫(0 초과 1 이하). 주어 백성의 `ResearchFocus[Key] = Value`로 두고 나머지 셋이 `(1 − Value) / 3`씩 나눠 가진다. 합은 1이다(+ − × ÷만). 틱당 출처에는 실을 수 없다. '지혜로운 마음'이 쓰고, MVP ⑦ 연구 중점 프리셋이 같은 함수를 단추로 부른다.

### 4.4 규칙 노브 하나

`DT_Rules.PetitionPeriodTicks` = 600(정수, 1 이상). `FCosmicRulesDef`와 스키마에 추가하고, 설정 표(`docs/30_CONFIG_REFERENCE.md`)를 다시 만든다. 노브는 47개가 되고 읽히는 것은 29개다.

### 4.5 스키마와 중첩 구조체 검사 (A-2a — 로드맵의 '선행')

지금은 구조체 배열이 어느 검사에도 닿지 않는다(C++은 `"nested"` 표지로 막고 Python은 그 종류를 모른다). 닫힌 목록에 종류 둘과 규칙의 경로 하나를 더한다.

| 자리 | 변경 |
|---|---|
| 종류 `structs` | 구조체의 배열이다. 옵션 `columns`(하위 열마다 지금의 종류와 옵션 — `structs`는 다시 담지 않는다)와 `min`·`len_in`. 원소는 하위 열과 **정확히 같은 키**를 가져야 한다(빠진 멤버가 C++ 기본값으로 조용히 남는 구멍을 원본에서 막는다). 텍스트 옵션(`slots`·`forbidden`·`no_digits`·`hedged`)과 `effects`·`conditions`·`row`·`enum` 검사가 하위 열에도 그대로 선다 |
| 종류 `pool` | `FResourcePool`. 다섯 멤버를 모두 적고 각각 유한한 수, 옵션 `min`. 지금 `plain`인 `DT_Megaprojects.CostPerTick`도 `pool`·`min 0`으로 옮긴다(`docs/40` 228행의 미뤄 둔 규칙) |
| 규칙의 하위 열 경로 | `then`·`when`의 열 이름에 `Choices.OnFailure`처럼 점을 쓴다. 모든 원소에서 성립해야 참이다. `lacks_type`·`is`·`in`·`set`·`min_len`·`max_len`만 받는다 |
| C++ `Data.Integrity` | `KindsFor`가 구조체 배열에 `structs`를, `FResourcePool`에 `pool`을 낸다(`"nested"` 표지와 그 단언은 지운다). `CheckSchemaAgainstCode`가 하위 열을 안쪽 구조체의 프로퍼티와 양방향으로 대조하고 종류·`integer`를 본다. `CheckCell`이 원소마다 하위 열로 내려가 효과·조건의 태그를 ini에 묶고 `row` 참조를 확인한다. `KindCases`에 둘의 틀린 종류를 더한다 |
| `test_data_schema.py` | `encounter_row` 고정 행, 두 종류와 경로의 단위 테스트, 규칙·슬롯·금지어 걷기가 하위 열까지 내려간다(셈이 늘어난다). `test_tables`의 'columns null' 예는 다른 표로 옮긴다 |
| `DT_Encounters` 열 | `Title` text(required, forbidden verdict, no_digits), `Body` text, `Conditions`, `BaseWeight` number(min 0), `Choices` structs(len_in [2, 3]: `Label` text required, `CheckStat` enum, `Difficulty` number integer 0~16, `Cost` pool min 0, `OnSuccess`·`OnFailure` effects, `SuccessText`·`FailureText` text required, `NextEncounter` row optional DT_Encounters), `Illustration` plain, `Trigger`·`Subject` enum, `VoiceBodies` texts(len_in [0, 2]). 규칙: 모든 행이 `PlayerRoleIs Ruler`를 갖고(새 시험 `has_type` — 아래 4.6), 하위 열 효과는 위 3.2의 다섯을 갖지 않으며, `Civilization` 행의 글은 `{System}`을 부르지 않는다(이벤트 행과 같은 `slots_in` 규칙) |
| 로더 | `FCosmicDataTables::Encounters` + `FindEncounter` + `GetEncounterRowNames`(어휘 순), `make_data_tables.py`의 `TABLES`, `must_exist`, 도장(`generated.sha1`) 한 줄 |
| 로컬라이제이션 | 선택지 배열의 키는 위치(`<행>_Choices_Index1_Label` — 엔진 `StabilizeLocalizationKeysForStruct`로 확인했다)라서 **끝에만 붙인다**. 검사기는 Phase 5까지 없다. `docs/70` §8.5에 한 줄을 적는다 |

### 4.6 `PlayerRoleIs` 규칙의 자리

`petition_` 행에 `PlayerRoleIs Ruler`가 반드시 있어야 하는데, 지금 시험에는 '조건 종류를 갖는다'가 없다(`lacks_type`의 반대). 닫힌 목록에 `has_type`을 더한다. 위반 예는 `test_data_schema`가 조건을 지운 행으로 만든다. `when`은 `row`에 접두어를 걸 수 없으므로, 규칙은 모든 행에 건다. 방랑자 조우(`enc_`)가 오는 B1에서 `when`에 `Trigger` 같은 열을 걸어 가른다.

## 5. 카드 10장 (행 이름은 배포 뒤 동결 — 세이브의 `PendingPetitionRow`·`LastChoiceRows`에 남는다)

수치는 시작값이다. 비용은 커맨드렛의 다스리는 백성 비축(틱마다 대략 에너지 1·물질 1·통찰 0.5·영향력 0.5)에 맞췄다. 판정의 성공 눈은 '6에 n'(아래 6)이다. 고칠 때는 MVP ③에서 커맨드렛으로 잰다. 본문과 선택지 글은 A-2b에서 쓰고, 그 묶음은 ultracode 감사(`docs/70` §8.3)를 받는다.

| # | 행 | 이름(가칭) | 언제 | 목소리 | 선택지(비용 · 판정 → 성공 / 실패) |
|---|---|---|---|---|---|
| 1 | `petition_wise_heart` | 지혜로운 마음 | Standing·백성: `GalaxyTickAtLeast 30` — 판의 첫 청원(단계 6부터는 서막의 마지막 카드로 옮긴다) | 목소리 없음 — 본문이 의회의 청을 적는다(Tier A 어휘, §5.11 완화 ①) | 지혜를 구한다 → `FocusResearch Society 0.55` / 부를 구한다 → `Engineering 0.55` / 안전을 구한다 → `Physics 0.55` (셋 다 비용·판정 없음) |
| 2 | `petition_walls` | 성벽인가 기다림인가 | Standing·백성: `CivStabilityBelow 0.5`가 막 선 때 | 둘 | 성벽을 높인다: 물질 80 → 안정 +0.08, Spiritualism −0.05 / 기다리며 공의를 행한다: 안정 판정 5 → 안정 +0.1·Spir +0.05 / 안정 −0.03·Spir +0.05 |
| 3 | `petition_bread` | 곳간을 열라 | Standing·백성: `CivStabilityBelow 0.3`이 막 선 때 | 둘 | 곳간을 연다: 에너지 60·물질 60 → 안정 +0.15·Spir −0.05 / 함께 금식한다: 안정 판정 4 → 안정 +0.12·Spir +0.05 / 안정 −0.05·Spir +0.05 / 듣지 않는다 → 안정 −0.05 |
| 4 | `petition_road` | 새 길 | Lottery·백성: 주기 슬롯 | 둘 | 더 짓자: 에너지 100 → 물질 +160·Spir −0.05 / 쉬게 하자 → 안정 +0.05·Spir +0.05 |
| 5 | `petition_feast` | 쉼의 날 | Lottery·백성: 주기 슬롯 | 둘 | 기록을 남기자: 통찰 60 → 영향력 +80·Spir −0.05 / 감사의 날을 지키자: 에너지 60 → 안정 +0.08·Spir +0.05 |
| 6 | `petition_keep_throne` | 왕좌를 지키겠는가 | Standing·성계: `SystemHazardTicksAtMost Rebellion 50`이 막 선 때(반란 잔여 50틱) | 둘(`{Rebel}`을 부른다) | 버틴다: 안정 판정 6 → `ResolveHazard`·안정 +0.15 / `DeposeRuler Reign.Deposed` / 개혁한다: 영향력 200 → `ResolveHazard`·안정 +0.05 (B1에서 셋째 '명예로운 양위'를 끝에 붙인다. 특성 '굳건한'은 그 특성을 읽는 코드가 올 때 붙인다) |
| 7 | `petition_plague` | 역병 | Standing·성계: `SystemHasHazard Plague`가 막 선 때 | 둘 | 성문을 닫는다: 에너지 120 · 안정 판정 5 → `ResolveHazard` / 안정 −0.05 / 앓는 이들 곁에 선다 → 안정 +0.05·Reverence +0.1·Spir +0.05(위기는 남는다) |
| 8 | `petition_awakening` | 깨어난 기계 | Standing·성계: `SystemHasHazard AIAwakening`가 막 선 때 | 둘 | 이름을 준다 → Openness +0.05·Spir −0.05(위기는 남고 만료의 자원을 기다린다) / 멈추게 한다: 에너지 80 → `ResolveHazard`·Spir +0.05 |
| 9 | `petition_unstable_star` | 흔들리는 별 | Standing·성계: `SystemHasHazard PreSupernova`가 막 선 때 | 둘 | 학자들의 길: 통찰 150·에너지 100 · 안정 판정 5 → `ResolveHazard` / 효과 없음(비용은 낸다) / 함께 기다린다 → 안정 +0.05·Spir +0.05(항성 안정화 프로젝트는 그대로 열려 있다) |
| 10 | `petition_exalt` | 그들이 당신을 높이려 한다 | Standing·백성: `PlayerTierAtLeast 2`가 막 선 때(둘째 칸에 오른 틱 — 설계의 '할아버지의 할아버지' 웅성거림 행이 Phase 4라 그 전의 자리) | 둘 — 의회 "당신을 신이라 부르면 백성이 한마음이 될 것이라고 의회가 말했다." / 회중 "회중의 몇이 그의 앞에 엎드리려 했다." | 거절한다 → Exaltation −1(바닥 0)·Reverence −0.05 / 침묵한다 → 효과 없음 / 받아들인다 → Exaltation +2·안정 +0.1·Reverence +0.2(유혹은 값을 산다 — 끝은 그리지 않는다, `docs/archive/v08_rulings.md` 567) |

- 목소리(§5.10 '화자는 윤리 축이 고른 파벌'): 카드가 뜰 때 다스리는 백성의 Spiritualism이 0보다 크면 회중의 장로들, 아니면 의회가 들고 온다. 상태에 적지 않는 표현의 선택이고, 카드가 열려 있는 동안 위젯이 붙들고 있다. 선택이 Spiritualism을 ±0.05 움직이므로 고른 길이 다음 청원의 목소리를 바꾼다. 첫 생애 백성의 윤리 시작값은 Spiritualism 0이라 첫 목소리는 의회다. 신앙 어휘는 회중의 본문에만 쓴다(§5.11 완화 ④).
- 헤롯 패턴 청원을 10장에 넣는 것은 **#93의 기본값**이다. 결과 줄은 rulings의 세 줄을 쓰되 서술의 사람을 '당신'으로 맞춘다(`docs/70` §8.2의 `log_first_life` 줄 — 서막 전이라 '그'가 아직 없다). 감사에서 다시 본다.

## 6. 카드 위젯 (A-2c, 표현 계층)

| 자리 | 내용 |
|---|---|
| 앎 읽기(규칙 14) | `FCosmicKnownView`에 `PendingPetitionRow`·`PendingPetitionSystemID`를 더해 `KnownViewOf`가 채운다. 앎을 읽는 곳은 여전히 하나다 |
| `UI/CosmicPetitionCardWidget`(새, C++·WBP 없음 — `docs/40`의 `WBP_EncounterCard`를 이 이름으로 바꾼다) | 화면 가운데, 너비 560, Z 5(패널 위, 승천 연출 10 아래). 위에서부터: 제목(18) · 목소리 줄(13, 회색 — ST_CA `Petition.Voice.Council` "의회가 청한다" / `Petition.Voice.Elders` "회중의 장로들이 청한다", 목소리 없는 카드는 비운다) · 본문(15, 줄바꿈) · 선택지마다 `UCosmicActionButton`과 그 밑의 값 줄(12, 회색 — 비용 '에너지 60 · 물질 60'과 판정 '안정 판정 · 6에 4'; 판정 없는 선택지는 판정 칸이 없고, 성공 눈 0은 '6에 0'). 비축이 모자란 선택지는 회색이고 툴팁은 `Petition.Short` "비축이 모자란다"다. 맞고 틀림을 말하는 표시는 없다(`docs/00` 원칙). 지도 클릭은 카드 밖으로 지나간다 |
| 답 뒤 | 단추 자리에 결과 줄(그 글 그대로)과 단추 하나('닫기', 사슬의 다음 카드가 있으면 '다음 청원'). 때가 지난 카드는 `Petition.Withdrawn` "때가 지났다."와 '닫기'다 |
| 컨트롤러(`ACosmicPlayerController`) | `HandleSimTick`(틱 경계)마다 `FollowPetition()`: 보는 대기 카드가 바뀌었으면 카드를 띄우고, 틱이 낸 새 카드면 `RequestAutoPause(Card)`를 부른다. `HandleGalaxyGenerated`(새 판·로드)는 대기 카드를 멈춤 없이 띄운다. `AnswerPetition`은 컨트롤러를 거쳐 부르고, 그 뒤 HUD·지도를 새로 그린다(`StartProject`와 같다). '닫기'를 누르면 카드가 시간을 멈춘 채(까닭 = 카드, 배속 0)일 때만 멈추기 전 배속을 돌려준다(승천 연출의 선례). 자동 정지가 꺼져 있으면 카드는 달리는 판 위에 뜨고, 시간은 답을 기다리지 않는다(`crisis_and_transcendents` 248) |

## 7. 테스트 (새 단언은 변이로 한 번씩 깨뜨린다)

| 테스트 | 단언 |
|---|---|
| `Data.Integrity`(넓힘) + `test_data_schema.py` | `structs`·`pool`·하위 열 경로·`has_type`의 양방향 대조와 위반 예, `DT_Encounters` 10행이 통과, `CostPerTick`이 `pool` |
| `Sim.PetitionPresent`(새) | 첫 카드가 정확히 틱 30에 뜨고 29·31에는 뜨지 않는다. 안정이 0.5 밑으로 막 내려간 틱에 한 번만 뜨고, 머무는 동안은 다시 뜨지 않는다. 대기 카드가 새 카드를 막는다. 주기 슬롯은 `LastChoiceRows`의 행을 고르지 않는다. 고르기는 시드·틱이 같으면 같다. 방랑자·몸 없는 상태에는 오지 않는다. **`DT_Encounters`를 비운 판과 세계 해시가 같다**(제시가 세계를 바꾸지 않는다). 시뮬 스트림 상태가 제시 전후로 같다 |
| `Sim.PetitionAnswer`(새) | 거절 여섯은 추첨이 없다. 받아들인 답은 스트림을 정확히 한 번 나아가게 한다. 비용, 판정의 경계(`능력치 + Roll = Difficulty`는 성공, 하나 모자라면 실패, 16은 늘 실패), `Stability` 척도, 성공·실패 효과와 결과 줄(Severity 1, 초점), 때가 지난 카드(효과 없음, 추첨은 1), `LastChoiceRows` 셋, 사슬, 왕좌 카드 실패 → 다음 틱에 폐위되고 대기 카드가 지워진다, `Install`·`BecomeWanderer`·백성 제거가 지운다, 방송 중 거절, `FocusResearch`의 합 1, `SystemHazardTicksAtMost`의 경계 |
| `Sim.SavePetition`(새) | 카드가 열린 채 저장 → 로드 → 같은 답이 중단 없이 돌린 판과 같은 전체 해시를 낸다 |
| `Sim.PetitionSoak`(새) | 시드 42·7, 6,000틱. 대기 카드가 오면 정해진 규칙(틱 % 선택지 수)으로 답한다. 에러 줄 0, 안정 0~1, 같은 입력의 두 판이 같은 해시 |
| `UI.PetitionCard`(새) | 제목·목소리·본문·값 줄, 모자란 선택지의 회색과 툴팁, 누르면 답하고 결과 줄과 '닫기'가 선다, 새 카드가 달리는 판을 멈추고(까닭 '정지 — 카드가 왔다') 닫으면 배속이 돌아온다, 끔이면 멈추지 않는다, 로드한 대기 카드는 멈춤 없이 뜬다, 때가 지난 카드 |
| 고칠 것 | `UI.AutoPause`의 카드 경우를 실제 카드로, `Sim.KnowingInert`·`Sim.FrameStep`(틱 30의 카드가 프레임 실행을 멈추지 않는지 — 자동 정지를 끈 채 잰다)·`Sim.FirstLife`, 시드 42 기준값의 전체 해시 |

## 8. 문서

`docs/20`(앎 단계의 제시·추첨 표·자동 일시정지의 셋째 경우), `docs/21`(구조체·열거형·필드·명령·`FCosmicKnownView`), `docs/40`(테스트 표·스키마 종류·`WBP_EncounterCard` 줄), `docs/10_DESIGN/systems.md` §5.10(MVP의 10장과 빠진 넷의 자리), `docs/70`(목소리·로컬라이제이션 한 줄), `docs/50` A ②, `OPEN_QUESTIONS` #26·#30 ①·#93을 닫고 `DECISIONS_LOG`에 날짜와 근거를 적는다. CLAUDE.md 규칙 4에 '앎 단계 끝 = 청원 제시'를 한 구절 더한다. STATUS·CHANGELOG.

## 9. 순서·검증·effort

1. 승인 → **A-2a**(effort high, ultracode 끔): 스키마·검사기·구조체 꼴. `run_checks.ps1 -Tier full` 2회. 기준값은 그대로다.
2. **A-2b**(high, 끔): 상태·제시·답·10행. `-Data -UpdateBaseline`으로 **전체 해시만** 바뀌고 세계 해시가 같은 것을 커밋 메시지에 적는다. 커맨드렛 20,000틱 CSV의 결정적인 칸이 모두 같아야 한다.
3. **A-2c**(high, 끔): 위젯. 오프스크린 한 장을 찍어 카드를 본다.
4. 문서·STATUS. 카드 글 묶음 감사는 개발자가 프롬프트에 `ultracode`를 적을 때 한 번 한다(서사 규칙 11·`docs/70` §8.3 — 금지어·숫자·목소리·'당신').

## 10. 개발자 결정 (권장안)

1. **MVP의 10장**: 영압 체인 3은 B4로, 첫 접촉은 Phase 4로 미루고, 그 자리에 위기 청원 3과 주기 청원 하나를 넣는다(위 1·5).
2. **대기 카드의 자리**: `FKnowingState` 끝의 두 필드(위 2)에 둔다. 제시는 상태 없는 파생 정수로 고르고 시뮬 난수를 뽑지 않는다. 그래서 답하지 않은 판의 세계 해시가 그대로다. 대안 `FPlayerEntityState.PendingEncounterRow`(#26의 대안)는 제시가 시뮬 스트림을 뽑게 되어 모든 기준값을 다시 잰다.
3. **언제 오는가**: 조건이 막 선 틱(틱 첫머리 대비) + 600틱 주기 슬롯. 대기 카드가 있으면 새 카드는 버린다(줄 세우지 않는다).
4. **판정 척도(#30 ①)**: 안정 = `(int32)(Stability × 10)`. Influence는 판정에 쓰지 않고 비용에만 쓴다.
5. **목소리**: 카드마다 의회·회중 본문 둘(`VoiceBodies`)을 두고, 다스리는 백성의 Spiritualism 부호로 표현 계층이 고른다.
6. **확률 표시**: 판정이 있는 선택지 밑에 '안정 판정 · 6에 4'처럼 성공하는 눈의 수를 보인다. 대안은 기준치만 보이기와 숨기기다.
7. **'지혜로운 마음'의 세 길**: 지혜 → 사회, 부 → 공학, 안전 → 물리. 고른 분야 0.55, 나머지 0.15씩.
8. **헤롯 패턴 청원(#93)**: 기본값대로 10장에 넣는다. 트리거는 둘째 칸에 오른 틱이다(웅성거림 행이 오는 Phase 4에 그 행 뒤로 옮긴다).
