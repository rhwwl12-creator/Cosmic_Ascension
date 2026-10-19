# 개발 가이드 (DEV GUIDE)

> 2026-10-18 재편. 앞부분은 규약(검사할 수 있는 한 문장만 — 괄호는 검사 수단), 뒷부분은 옛 guideline §10·§9.3·§14와 옛 CLAUDE.md의 빌드·실행·테스트 명령 전문을 그대로 옮긴 것이다. 규약과 원문이 다르면 규약이 이긴다. 불변 규칙의 요약은 `CLAUDE.md`, 전문은 `docs/20_ARCHITECTURE.md`.

## 1. 코드 규약

- 소스 파일은 ASCII, 주석은 영문, 한국어는 DataTable/ST_CA에만. (비ASCII grep)
- 헤더 이름은 타깃 전체에서 유일(도메인 접두어 `Cosmic*`), `.generated.h`는 마지막 include, `#pragma once`. include는 모듈 루트 기준. (UHT·빌드)
- 함수는 150줄을 넘기지 않고 새 함수는 100줄 이하. `NewGalaxy`(464줄)는 생성 단계별 private 함수로 쪼갠다(추첨 순서 불변, `Sim.Determinism`이 확인). (리뷰)
- 태그·ST_CA 키·행 이름을 리터럴로 쓰지 않는다 — `CosmicRows::`·`CosmicTags::`·`CosmicLabels::` 상수만(지금 `RequestGameplayTag(FName(TEXT(...)))` 리터럴 59곳·핸들러 거절 가드 22곳 복붙은 로드맵 0단계에서 걷어낸다). (grep)
- 호출자·데이터 오류는 `UE_LOG(LogCosmic, Error)` 한 줄 + 무동작. `ensure`/`check`는 호출자 오류에 쓰지 않는다(테스트로 덮을 수 없다).
- UE 표준 접두어(U/A/F/E) 외 프로젝트 접두어 없음; 이름에 `Cosmic`/`Galaxy`/`Ascension` 도메인 낱말. DataTable 행 이름 `snake_case` + 테이블 접두어(`event_`·`crisis_`·`feat_`·`enc_`/`petition_`·`arch_`·`trans_`·`log_`·`project_`·`edict_`·`gal_`, 단일 행 `rules`). GameplayTag `Domain.Sub.Leaf` 최대 3단계. 에셋 접두어 `DT_`·`ST_`·`WBP_`·`NS_`·`M_`·`MI_`·`L_`.

## 2. 모듈·의존 방향

- `Core ← Data ← Sim ← {Events, Ascension, Log, Player, Save} ← {World, UI}`. 역방향 include 금지. 단일 모듈이라 빌드가 강제하지 않으므로 `Tools/check_layers.py`(로드맵 0단계)가 include 방향·태그 리터럴·`FMath::Rand`를 grep으로 검사한다.
- Sim은 형제 Subsystem을 틱 단계 함수에서만 부른다(`GetSubsystem<>()`). Sim↔Events의 .cpp 수준 상호 호출은 '한 길' 설계로 허용하되 평가 문맥 타입(`FCosmicEvalSubject`·`View`·`Context`·`WorldNote`)은 `Sim/CosmicEvalContext.h` 독립 헤더로 내린다.
- 표현 상태(선택·카메라·시계 단위)는 Controller/ini에 두고 `FGalaxyState`·해시 밖이다.

## 3. 상태·저장·결정론

- 런타임 상태는 `FGalaxyState` 안의 USTRUCT만. Subsystem 멤버에 플레이어·생애 종속 상태 없음.
- 저장 필드의 기본 초기값은 세이브가 생긴 뒤 불변, 열거자는 끝에만, `SaveVersion` 인상은 마이그레이션 함수 + 테스트 동반.
- 세계를 바꾸는 추첨은 `GetRng()`만, 행·뱅크를 보기 전에 정해진 횟수. 말하기 스트림은 `PushVariants`만. `FMath::Rand*` 0건. (grep)
- 해시에 드는 산술은 모듈 안의 `+ − × ÷ √`만. 시드 42 세계 해시 기준값은 `Tools/baselines/seed42.json`에 파일로 두고 커맨드렛이 비교한다(로드맵 0단계 — 지금은 CHANGELOG의 기록).
- 자동 저장(승천·역할 전환·10분·종료)은 틱 경계에서만, 핸들러 안에서는 금지. 쿠킹 빌드의 해시 일치를 MVP 종료 조건에 넣는다.

## 4. 데이터·설정 관리

- 원본은 `Tools/data/*.json`·`ST_CA.csv`, `.uasset`은 산출물. 컬럼·열거자 추가 = `schema.json` + `test_data_schema.py`의 경계 케이스 동반. 검사는 `python -I Tools/check_data.py`(에디터 불필요).
- 플레이 밸런스 수치는 `DT_Rules` 또는 행 컬럼에 둔다. cpp `constexpr`는 '법'(틱 길이·로스터 상한 3·칸 5·저안정 문턱 0.3)에만 허용하고 주석에 `[law]`. 경제·연구 상수 9개(`TierMultiplier`·`StarClassEnergy`·`InsightPerPopulation`·`InfluencePerPopulation`·보너스 3·`ResearchInsightShare`·`ResearchPerInsight`)는 MVP에서 `DT_Rules`로 옮긴다(`StarClassEnergy`는 단계 5까지 C++).
- `DT_Rules` 노브는 읽는 코드가 있을 때만 추가한다. 예약 노브(계승 6·도약 3·함대 4·귀환 3·`ImmutableLaws`·`VigilClaimExtension`)는 스키마에 `since: "Phase N"` 표지를 달고 `Data.Integrity`가 '읽는 코드 없는 노브'를 알린다. 노브 표 `docs/30_CONFIG_REFERENCE.md`는 손으로 쓰지 않고 JSON+schema에서 생성한다(A0 뒤).
- ini `DevComment`·헤더 주석·스키마 `why`에 설계 수치를 적지 않는다 — 어디를 보라는 한 줄만. 수치의 원본은 데이터(JSON)와 docs/10_DESIGN 둘.

## 5. 테스트

- 필수: 결정론(해시·추첨 수)·저장 왕복·데이터 무결성·세계 규칙 변경. 불필요: 문안·색·배치 같은 표현 값.
- 새 단언은 변이로 한 번 깨뜨린다(패치는 `Tools/mutations/`에 남긴다 — 로드맵 0단계). 단언 하나에 경우 하나. 에러 메시지 단언은 접두어만 매치.
- 20,000틱 판은 같은 시드 42로 묶어 한 번 돌린 결과를 나눠 쓴다. 증거 테스트(`Sim.FirstLife`·`PhaseTwoExit`·`GenerationRules`·`LayersInRange`·`Feats.Reachable`·`RealmCap`)는 A0(성역·첫 생애 행)·단계 5(실제 하늘)에서 깨지므로 그 작업의 '재측정 목록'에 미리 적는다.
- 빌드 뒤 자동화 테스트는 **두 프로세스에서 연속 2회**(`SaveRestart`가 두 번째 실행을 쓴다).

## 6. 커밋·문서·세션

- 커밋 = 컴파일 + 테스트 2회 통과. 제목: 한국어 요약 + 작업 접두어(예 `[P3-12]`), 본문 끝 `Co-Authored-By` 트레일러. 작업 표에는 날짜만, 해시 기록 커밋 없음. 기준값이 바뀌는 커밋은 메시지에 새 해시.
- 설계 변경은 docs/10_DESIGN(또는 해당 문서) 먼저, 코드 둘째, `STATUS.md` 셋째. 규칙 문장에는 `(v0.8 — 단계 N부터)` 같은 표지로 예정을 구분하고, 코드에 들어오면 표지를 지운다. 같은 문장을 두 문서에 적지 않는다.
- 헤더 승인 → 단독 구현 → 적대 리뷰(ultracode) → 변경 범위 재감사의 골격은 유지하되, 헤더 승인은 저장 필드·열거형·틱 단계·결정론 코드에만. ultracode 켬 = ① 세이브·해시에 닿는 변경 ② 틱 단계 본문 신설 ③ 텍스트 묶음 ≥ 20문장 ④ 단계 종료 리뷰. 끔 = UI·행 추가·버그 수정. Agent 도구 병렬 리뷰는 ultracode와 동등한 수단.
- 일정은 '주'가 아니라 작업일(세션)로 적는다. 1세션 = 1작업. 주 1회 '코드 없이 플레이만 하는 날'(발견은 STATUS의 열린 결정으로).

## 7. 환경

- 개발 PC는 Windows(PowerShell 5.1). 경로는 `$env:UE_ROOT`·`$env:PROJECT_ROOT`로 변수화한다(아래 원문 명령의 절대 경로를 그대로 쓰되 다른 PC에서는 두 변수만 바꾼다). Linux 체크아웃에서는 데이터 검사·스키마 단위 테스트·레이어 린트만 돈다.
- `Tools/run_checks.(ps1|sh) -Tier smoke|full`(로드맵 0단계): 데이터 검사 → 레이어 린트 → 빌드 → 테스트 2회 → `-nullrhi` 스모크 → 커맨드렛 기준값 비교.

---

# 원문 (2026-10-18 이관, 내용 무변경)

> 아래는 옛 guideline.md §10(프로젝트 구조·코딩 규약)·§9.3(테스트와 1인 밸런싱)·§14(Claude Code 협업 규칙)와 옛 CLAUDE.md의 '빌드 / 실행 / 테스트' 절이다. §10.1의 폴더 트리는 설계 시점의 목표이고 실제 자산은 `Content/CA`·`Source`를 따른다(2026-10-18: WBP는 `WBP_LogEntry`만, `L_MainMenu`·`M_Lane` 없음, 위젯은 C++). 절 번호 위치는 `docs/SECTION_MAP.md`.
