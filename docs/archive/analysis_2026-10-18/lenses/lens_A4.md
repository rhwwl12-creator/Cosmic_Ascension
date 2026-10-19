# 렌즈 A4 — 아키텍처·코드 건강·개발 가이드라인

## 0. 먼저 바로잡을 전제

요청문은 "전체 아키텍처와 기본/상세 설정값이 확립되지 않았다"고 했지만, 코드를 열면 반대다. 아키텍처는 1인 프로젝트로는 드물게 엄격하다(단일 상태 구조체, 틱 열 단계, 난수 출처 셋, 리플렉션 해시, JSON→스키마→에셋 파이프라인). 설정값도 없는 것이 아니라 **네 곳(guideline·ini 주석·헤더 주석·JSON)에 흩어져 있고, DT_Rules 44개 노브 중 18개는 아무 코드도 읽지 않는다.** 진짜 공백은 아키텍처가 아니라 그 위에 올라갈 **플레이어 명령층**(Player 모듈의 명령은 `StartMegaproject` 하나)과 **저장·UI 피드백**이다.

## 1. 현황 평가 (전부 저장소에서 직접 확인)

| 영역 | 확인된 사실 | 근거 | 판정 |
|---|---|---|---|
| 레이어 규칙 | Core/Data/Sim/Events/Ascension/Log/Player/Save에 `UUserWidget`·`AActor`·UI/World include **0건** | grep 전수 | 잘 지켜짐. 단 단일 모듈(Build.cs:16)이라 컴파일러가 강제하지 않고 린트 스크립트도 없음 |
| 헤더 의존 | `Sim/CosmicHazards.h:14`·`Sim/CosmicMegaprojects.h:15`가 `Events/CosmicEventSubsystem.h`를, `Events/CosmicEventSubsystem.cpp:15-18`이 Sim 헤더 4개를 include | 파일 확인 | **Sim↔Events 순환**. 평가 문맥 타입(FCosmicEvalContext)이 Events에 살아서 생김 |
| 런타임 결합 | Sim이 Events·Log·Ascension·Player를 `GetSubsystem<>()`+`check()`로 11회 호출 | GalaxySimulationSubsystem.cpp | GameInstance 없이 생성 불가 → 커맨드렛이 GameInstance를 손으로 만드는 이유 |
| 상태 모델 | FGalaxyState 단일 USTRUCT, 세계→앎→이야기를 최상위 멤버 이름으로 분리, FText·GalaxyID만 해시 밖 | Core/GalaxyState.h:15-28, 677-763 | 잘 됨. `Position`은 '뷰포트 배치' 주석인데 세계 해시에 든다(:122) |
| 틱 | 열 단계 고정, 1틱=1초, 프레임당 10틱 | Sim .cpp:1323-1360, .h:491-494 | 외교·분쟁은 빈 함수(:1585-1593). `SetTimeScale`이 `MaxTicksPerFrame`(10)으로 clamp(:1036) → 설계된 ×25 배속은 UI를 고쳐도 열리지 않음 |
| 결정론 | 시뮬/말하기/파생 스트림 분리, '행 보기 전 무조건 뽑기', `FMath::Rand*` 0건, 모든 비에디터 타깃 FP Precise, Derive 골든값 static_assert | Core/CosmicSeed.h:43-89, Build.cs:21-24 | 상용 수준. 다만 시드 42 세계 해시 기준값이 테스트 파일이 아닌 CLAUDE.md 산문에만 있음 |
| 저장 | 왕복·두 프로세스 재시작 테스트 통과 | Save/CosmicSaveSubsystem.cpp | **게임 코드 호출자 0** (`SaveCurrentGalaxy`·`LoadGalaxy`는 테스트만 호출, Log .h:117 주석뿐). 자동 저장·슬롯 열거·백업·핸들러 가드 없음 — 지금 빌드는 창을 닫으면 판이 사라짐 |
| 데이터 파이프라인 | JSON 9+CSV → schema.json(테이블 10·조건 30·효과 18) → make_data_tables(라운드트립 비교) → sha1 도장 → Data.SourcesInSync/Integrity, Python 단위 테스트 69건 | Tools/ | 가장 잘 닫힌 층. plain 컬럼 범위·중첩 구조체·DT_Encounters(columns null)는 미검사 |
| 노브 소유권 | DT_Rules 44키 중 18개(Leap 3·Succession 6·Fleet 4·ImmutableLaws·Vigil·ReturnLure·ReturnedLives·LureAccept) 런타임 참조 0; 경제·연구 상수 9개는 cpp `constexpr`(Sim .cpp:83-100); 틱 길이·배속 사다리·로그 2,000·피드 500도 코드 | DT_Rules.json | 규칙 부재: 무엇이 노브이고 무엇이 법인지 정해지지 않음 |
| 이벤트 어휘 | 조건 30·효과 18 단일 평가기, 틱당 효과 화이트리스트 7, 발화 원자 고정 | Events .cpp | 잘 됨. `DurationTicks` 미읽음, 선택자 `Former`만 |
| 로그 엔진 | `PushVariants` 단일 경로, 범위·bUntold·먼 줄·2,000 상한 | Log .cpp | 잘 됨. BuildChronicle·Digest·슬롯 해석 없음, Chronicle 무상한(세이브 1MB 초과 원인) |
| UI | WBP 없는 C++ 위젯 13파일, 명령은 Controller 한 경로 | UI/, World/ | 테마·DPI 없음, 매직 넘버 분산, 배속 사다리가 TopBar와 키 핸들러에 2중 정의, Enhanced Input 설정 위 레거시 BindKey |
| 크기 | 비테스트 13,319줄 / 테스트 18,746줄(1.4배) / 테스트 76개 | wc | `GalaxySimulationSubsystem.cpp` 2,241줄, `NewGalaxy` 464줄 |

**중복·냄새(개수 확인):** 핸들러 거절 가드 `IsBroadcasting()` 22곳 복붙, `RequestGameplayTag(FName(TEXT(...)))` 리터럴 59곳(태그 상수 헤더 없음), '소유이고 정착된 성계' 술어 6곳, 칸↔번호 변환 7곳, 테스트의 `Tag()` 헬퍼 10파일 복제, `ensureMsgf`로 호출자 오류 알림(PlayerRoleSubsystem.cpp:71 — "ensure는 테스트로 덮을 수 없다"는 자체 규약 위반).

## 2. 개발 가이드라인 초안 (2단계에 CLAUDE.md·DEV_GUIDE로 들어갈 것)

규칙은 "검사할 수 있는 한 문장"만 적는다. 괄호는 검사 수단.

**코드 컨벤션**
- 소스는 ASCII, 주석 영문, 한국어는 DataTable/ST_CA에만. (비ASCII grep)
- 헤더 이름 모듈 전역 유일, `.generated.h` 마지막, `#pragma once`. (UHT)
- 함수 150줄 초과 금지, 새 함수 100줄 이하. (리뷰)
- 태그·ST_CA 키·행 이름 리터럴 금지 — `CosmicRows::`/`CosmicTags::`/`CosmicLabels::` 상수만. (grep)
- 호출자 오류는 `UE_LOG(LogCosmic, Error)` 한 줄 + 무동작. `ensure`/`check`는 데이터·호출자 오류에 쓰지 않는다.

**모듈·의존 방향**
- `Core ← Data ← Sim ← {Events, Ascension, Log, Player, Save} ← {World, UI}`. 역방향 include 금지. (Tools/check_layers.py)
- Sim은 형제 Subsystem을 틱 단계 함수에서만 부른다. 헤더 순환 금지.
- 평가 문맥 타입은 `Sim/CosmicEvalContext.h`(독립 헤더)에 둔다.

**상태·저장·결정론**
- 런타임 상태는 FGalaxyState 안 USTRUCT만. Subsystem 멤버에 플레이어 종속 상태 금지.
- 저장 필드 초기값은 세이브 생성 뒤 불변, 열거자는 끝에만, `SaveVersion` 인상은 마이그레이션 함수+테스트 동반.
- 세계를 바꾸는 추첨은 `GetRng()`만, 행을 보기 전에 고정 횟수. `FMath::Rand*` 0건. (grep)
- 해시에 드는 산술은 모듈 안 `+ − × ÷ √`만. 시드 42 세계 해시 기준값은 `Tools/baselines/`에 파일로 두고 커맨드렛이 비교한다.
- 표현 상태(선택·시계 단위·카메라)는 Controller/ini, 해시 밖.

**데이터·설정**
- 원본은 Tools/data JSON·CSV, .uasset은 산출물. 컬럼·열거자 추가 = schema.json + test_data_schema.py 경계 케이스 동반.
- 플레이 밸런스 수치는 DT_Rules 또는 행 컬럼. cpp `constexpr`는 '법'(로스터 상한 3·칸 5·1틱=1초)에만 허용하고 주석에 `[law]`.
- DT_Rules 노브는 읽는 코드가 있을 때만 추가. 예약 노브는 스키마에 `since: "Phase 4b"` 표지.
- 행 이름 접두어·테이블 가로지른 유일성·`@` 금지. 코드가 부르는 태그는 `schema.required_tags`에 등록.

**테스트**
- 필수: 결정론(해시·추첨 수)·저장 왕복·데이터 무결성·세계 규칙 변경. 불필요: 문안·색·배치 같은 표현 값.
- 새 단언은 변이로 한 번 깨뜨린다. 단언 하나에 경우 하나. 경우의 수는 리플렉션으로 센다.
- 20,000틱 판은 `full` 티어에만. `smoke` 티어는 3분 이내.
- 게임 데이터의 시드 의존 단언은 '잰 시드'로 표기하고 기준값 변경 시 재측정 목록에 올린다.
- 에러 메시지 단언은 접두어만 매치한다.

**커밋·브랜치·문서**
- 커밋 = 컴파일 + 테스트 2회 통과. 한국어 요약 + 트레일러. 기능 단위 브랜치는 선택(1인이라 main 직행 허용, 기준값이 바뀌는 커밋은 메시지에 새 해시).
- 설계 변경은 SSoT 먼저, 코드 둘째, STATUS 셋째. 같은 문장을 두 문서에 적지 않고 링크한다.
- 규칙 문서에는 `(v0.N — 단계 M부터)` 괄호 대신 **현행/예정 열**을 둔다.

## 3. CLAUDE.md 118KB → 10KB

| 절 | 현재 | 처분 |
|---|---|---|
| 프로젝트 | 1.6KB | 유지(0.5KB). `C:\…` 절대 경로 6줄 → `$UE_ROOT`·저장소 상대 경로 |
| 아키텍처 불변 규칙 | 26.4KB, 37줄에 `v0.8 — 단계` 11·`v0.7 —` 5·`가칭` 4 | **현행 규칙만** 20줄(~4KB)로 재작성. 예정 규칙은 로드맵, 상세는 DEV_GUIDE 링크 |
| 작업 방식 | 1.0KB | 유지 |
| 빌드/실행/테스트 | 21.2KB(cmd+PowerShell 2벌·10명령) | 러너 호출 5줄(~2KB)만 남기고 전문은 `docs/dev/build.md` |
| 모듈/UHT 규칙 | 1.3KB | 유지 |
| 검증된 엔진 헤더 경로 | 31.3KB | `docs/dev/engine_notes.md`로 이동 — 가치는 높지만 매 세션 컨텍스트에는 불필요 |
| 렌더/플러그인 결정 | 2.0KB | `docs/adr/0001-render.md` |
| 현재 단계 | 33.6KB | `STATUS.md`(현재 10줄) + `CHANGELOG.md`(이력). WORKPLAN 부록 A·guideline §11 상태 줄과의 3중 복제 해소 |

결과 약 9~10KB. 세션 읽기 비용이 1/12이 되고 "불변"이라는 제목이 사실이 된다.

## 4. 테스트 전략

| 묶음 | 수 | 고정하는 것 | 처분 |
|---|---|---|---|
| 결정론·저장(Determinism·NoNaN·FrameStep·SeedDerive·TellStreamSeparate·SaveRoundTrip·SaveRestart·KnowingInert) | 8 | 해시·추첨 수·두 프로세스 | **MVP 필수, smoke**(KnowingInert 8×20,000틱만 full) |
| 데이터(Data.Integrity·SourcesInSync + Python 69) | 2+69 | 스키마↔코드↔에셋·도장 | smoke |
| 이벤트 규칙(Events.*) | 12 | 행 순서 무관·슬롯·원자성·역할 효과 | 유지 |
| 세계 규칙(위기·정착·영압·앎·업적 Sim.*) | ~37 | 규칙 | 유지, 20,000틱 12건은 full |
| 증거 테스트(PhaseTwoExit·FirstLife·GenerationRules·LayersInRange) | 4 | 시드 42·7, `sol`·`arch_terran` 고정(CosmicSimTests.cpp:532-555) | **v0.8 단계 4·5에서 깨짐 예정** — 재측정 목록 선등록, 규칙 단언과 증거 단언 분리 |
| UI.* | 7 | 위젯 트리·픽·카메라 수치 | 유지, 수치 리터럴은 제품 상수 참조로 |

비용 신호: `AddExpectedError` 46건(문구 변경에 취약), 소형 헬퍼 10파일 복제, 가장 긴 RunTest ≈660줄(앞 블록 실패가 뒤 블록을 가림). 테스트 양 자체는 과하지 않다 — **티어 분리·헬퍼 통합·증거/규칙 분리**가 유지비를 낮춘다.

## 5. 빌드·검증 워크플로

현재: CLAUDE.md의 명령 10개를 손으로 치고 "두 번 연속 실행"도 사람이 기억. Tools/에 러너 없음. 제안 `Tools/run_checks.ps1 -Tier smoke|full`:
1. `python -I Tools/check_data.py` + `test_data_schema.py`
2. JSON 변경 시 `make_data_tables.py`(도장 갱신)
3. `python Tools/check_layers.py`(include 방향·리터럴 태그 grep) — 에디터 없이 Linux에서도 돈다
4. 에디터·게임 타깃 빌드
5. 자동화 테스트 2회 연속(티어 필터)
6. `-game -nullrhi` 스모크 + 오프스크린 스크린샷 1장
7. 커맨드렛 시드 42 → `Tools/baselines/seed42.json` 해시·요약 비교(의도한 변경이면 `-UpdateBaseline`)

`.claude/settings.json` 훅으로 JSON 저장 뒤 1단계를 자동 실행하면 데이터 작업 루프가 닫힌다.

## 6. 1인 개발 범위
지금 할 것: 저장 최소형, CLAUDE.md 다이어트, 러너+레이어 린트, 헤더 순환 해소(반나절), 노브 소유권 표기. 미룰 것: Tests 별도 모듈, Enhanced Input 전환, 리플레이 — MVP 뒤.

## 권장안
- [P0] **저장/이어하기 최소형을 Phase 4 앞에 넣는다** — SaveCurrentGalaxy/LoadGalaxy의 게임 코드 호출자가 0이다. 틱 경계 자동 저장(승천·역할 전환·10분·종료), Galaxy_* 슬롯 열거와 '이어하기', SaveCurrentGalaxy의 IsBroadcasting/bInTickStages 가드, 백업 슬롯 1개까지만. 머리·CRC·압축(§16 #70)은 그 다음.
- [P0] **CLAUDE.md를 10KB 이하로 줄이고 네 문서로 분리한다** — 유지: 프로젝트 0.5KB·현행 불변 규칙 20줄·작업 방식·모듈/UHT·러너 호출 5줄. 이동: 엔진 헤더 표(31KB)→docs/dev/engine_notes.md, 빌드 전문(21KB)→docs/dev/build.md, 렌더 결정→docs/adr/, 현재 단계(34KB)→STATUS.md(10줄)+CHANGELOG.md. 불변 규칙 안의 'v0.8 — 단계 N' 표지 11개는 로드맵으로 옮긴다.
- [P0] **Sim↔Events 헤더 순환을 끊는다** — FCosmicEvalSubject/View/Context/WorldNote를 Sim/CosmicEvalContext.h로 내리고 CosmicHazards.h·CosmicMegaprojects.h가 Events 헤더 대신 그것을 include하게 한다. 반나절 작업이며 이후 레이어 린트의 전제가 된다.
- [P0] **러너 스크립트와 레이어 린트를 만든다** — Tools/run_checks.ps1 -Tier smoke|full(데이터 검사→빌드→테스트 2회→스모크→커맨드렛 기준값 비교)과 Tools/check_layers.py(include 방향·RequestGameplayTag 리터럴·FMath::Rand grep, 에디터 불필요). 시드 42 해시 기준값을 Tools/baselines/seed42.json으로 옮기고 CLAUDE.md 산문에서 뺀다.
- [P0] **테스트를 smoke/full 두 티어로 나누고 v0.8에 깨질 테스트를 선등록한다** — smoke: 결정론·저장·데이터 약 20개(3분 이내). full: 20,000틱 12건 포함 전체. Sim.FirstLife·PhaseTwoExit·GenerationRules·LayersInRange는 sol·arch_terran·시드 42/7을 고정하므로 단계 4·5 작업 목록에 '재측정'으로 미리 적고, 규칙 단언과 증거 단언을 다른 테스트로 분리한다.
- [P1] **노브 소유권 규칙을 정하고 DT_Rules를 정리한다** — DT_Rules 44키 중 런타임 참조 0인 18개에 스키마 since 표지(Phase 4/4b/6a)를 달거나 뺀다. Sim .cpp:83-100의 경제·연구 상수 9개(TierMultiplier·StarClassEnergy·ResearchPerInsight 등)는 DT_Rules로 올린다. 배속 사다리·로그 상한·틱 길이는 '[law]' 주석으로 코드 상수임을 명시한다.
- [P1] **태그 상수 헤더와 가드 헬퍼로 복붙을 걷어낸다** — Data/CosmicTags.h(코드가 부르는 태그 약 20종)·UI/CosmicLabels.h(ST_CA 키 37)·핸들러 거절 매크로 RefuseInHandler(TEXT("Install"))로 리터럴 59곳·가드 22곳을 모은다. Tests/CosmicTestHelpers.h에 Tag()·FindMap()·WithRules()·Eff()/Cond()를 올려 10파일 복제를 없앤다.
- [P1] **NewGalaxy(464줄)를 생성 단계 7개로 쪼개고 틱 단계별 계측을 넣는다** — 좌표·레인·모성계·아키타입·이름·로스터·초기 코어를 private 함수로 분리하되 추첨 순서는 그대로(해시 불변을 Sim.Determinism이 확인). 열 단계마다 SCOPE_CYCLE_COUNTER를 두어 §9.4의 '가장 긴 틱 2.9ms 원인 미상'을 닫는다.
- [P1] **배속 상한과 프레임당 틱 상한을 분리한다** — SetTimeScale이 MaxTicksPerFrame(10)으로 clamp해 설계된 셋째 칸 ×25가 열리지 않는다. MaxTimeScale(25)과 MaxTicksPerFrame(10)을 다른 상수로 두고 이월 규칙·Sim.FrameStep 단언을 함께 갱신한다. 배속 사다리는 한 배열에서 TopBar와 키 핸들러가 읽게 한다.
- [P1] **'예약 필드·예약 노브' 목록을 한 곳에 둔다** — FLifeRecord 8필드·PendingLeapTicks·FleetPower·ActiveEdicts·PersonalPool·Transits 등 저장·해시에 들지만 읽는 코드가 없는 필드와 DT_Rules 미사용 18노브를 docs/dev/reserved.md 표로 모은다. 헤더 주석의 'Phase N' 표지 대신 이 표가 기준이 되어 '구현됨'으로 오해되는 일을 막는다.
- [P1] **SSoT 문서에는 '현행/예정' 열을, 코드 주석에는 § 번호 대신 규칙 ID를 쓴다** — 코드 주석의 guideline § 참조가 수십 곳이라 문서 재편 때 전부 낡는다. 규칙마다 안정 ID(예: R-DET-03)를 부여하고 주석·스키마 why·테스트 설명이 그 ID를 가리키게 한다. 문서 쪽은 괄호 표지 대신 표의 열로 상태를 보인다.
- [P2] **Tests를 별도 모듈(Cosmic_AscensionTests)로 뗀다** — Json·EngineSettings 의존과 UCosmicTestProbe·커맨드렛 UCLASS가 게임 바이너리에서 빠지고 레이어 경계를 빌드 시스템이 강제한다. 76개 테스트의 include 정리가 필요해 MVP 뒤로 미룬다.
- [P2] **입력 아키텍처를 결정한다(ADR)** — DefaultInput.ini는 Enhanced Input 클래스를 지정하지만 바인딩은 레거시 BindKey다. 키 재배치·패드가 목표가 아니면 레거시를 명시적으로 채택하고 ini를 맞추거나, 조우 카드 UI 전에 IMC/IA 에셋으로 전환한다. 둘 중 하나를 docs/adr/에 적는다.
- [P2] **환경 이식성을 확보한다** — CLAUDE.md의 C:\ 절대 경로 6줄과 PowerShell 전제를 환경 변수($UE_ROOT·$PROJECT_ROOT)로 바꾸고, Linux 체크아웃(현재 저장소 위치)에서 에디터 없이 돌 수 있는 단계(데이터 검사·레이어 린트·스키마 단위 테스트)를 러너에서 분리해 표시한다.

## claims
- A4-1 [factual/medium] Core Logic 폴더(Core·Data·Sim·Events·Ascension·Log·Player·Save)에는 UUserWidget·AActor·UI/·World/ 헤더 include가 한 건도 없어 §6.1 레이어 규칙이 지켜지고 있으나, 단일 모듈이라 이를 강제하는 빌드 설정이나 린트 스크립트는 없다.  (근거: grep -rn '#include .*(UserWidget|GameFramework/Actor|/UI/|/World/|Widget)' Source/Cosmic_Ascension/{Core,Data,Sim,Events,Ascension,Log,Player,Save} → 0건; Source/Cosmic_Ascension/Cosmic_Ascension.Build.cs:16(단일 모듈, PrivateIncludePaths); Tools/ 목록에 린트 스크립트 없음)
- A4-2 [factual/medium] Sim과 Events 사이에 헤더 수준 순환 의존이 있다: Sim/CosmicHazards.h와 Sim/CosmicMegaprojects.h가 Events/CosmicEventSubsystem.h를 include하고, Events/CosmicEventSubsystem.cpp가 Sim 헤더 네 개를 include한다.  (근거: Source/Cosmic_Ascension/Sim/CosmicHazards.h:14; Source/Cosmic_Ascension/Sim/CosmicMegaprojects.h:15; Source/Cosmic_Ascension/Events/CosmicEventSubsystem.cpp:15-18)
- A4-3 [factual/high] Tools/data/DT_Rules.json의 노브 44개 중 18개(LeapTicksMin/Max/TotalCap, Succession* 6, PressureFleetAttrition, FleetEffectivenessByTierGap, FleetDelayPerPower/Cap, ImmutableLaws, VigilClaimExtension, ReturnLureMaxLanes, ReturnedLivesToSeize, LureAcceptByPolicy)는 테스트 밖 런타임 .cpp에서 참조가 0이고, 경제·연구 밸런스 상수 9개(TierMultiplier·StarClassEnergy·ResearchPerInsight 등)는 DT_Rules가 아닌 cpp constexpr다.  (근거: Tools/data/DT_Rules.json(키 44개); grep -rl <노브명> Source/Cosmic_Ascension --include=*.cpp | grep -v Tests | grep -v CosmicDefinitionRows → 각 0건; Source/Cosmic_Ascension/Sim/GalaxySimulationSubsystem.cpp:83-100)
- A4-4 [factual/high] 저장 서브시스템은 테스트 밖 게임 코드에서 한 번도 호출되지 않는다 — SaveCurrentGalaxy·LoadGalaxy의 비테스트 참조는 Log/CosmicLogSubsystem.h:117의 주석뿐이며, 자동 저장 트리거·슬롯 열거·핸들러 가드가 없어 현재 빌드는 세션을 닫으면 판을 잃는다.  (근거: grep -rn 'SaveCurrentGalaxy\|LoadGalaxy(' Source/Cosmic_Ascension --include=*.cpp --include=*.h | grep -v Tests | grep -v Save/ → Log/CosmicLogSubsystem.h:117 주석 1건; Source/Cosmic_Ascension/Save/CosmicSaveSubsystem.cpp:34-52(IsBroadcasting 가드 없음); World/CosmicGameMode.cpp:77-95(항상 NewGalaxy))
- A4-5 [factual/medium] CLAUDE.md 118KB 가운데 '현재 단계' 33.6KB·'검증된 엔진 헤더 경로' 31.3KB·'빌드/실행/테스트' 21.2KB·'아키텍처 불변 규칙' 26.4KB가 95%를 차지하고, 불변 규칙 37줄(L12-48) 안에 'v0.8 — 단계' 표지 11개·'v0.7 —' 5개·'가칭' 4개가 섞여 있으며 Windows 절대 경로가 6줄 박혀 있다.  (근거: awk 섹션 바이트 집계 of /home/mechtro/Desktop/bbb/Cosmic_Ascension/CLAUDE.md(## 헤더 기준); sed -n 12,48p CLAUDE.md | grep -o 'v0\.8 — 단계\|v0\.7 —\|가칭' | sort | uniq -c; grep -cF 'C:\' CLAUDE.md → 6)
- A4-6 [factual/medium] 자동화 테스트는 76개(Sim 49·Events 12·UI 7·Log 4·Feats 2·Data 2)이고 테스트 코드 18,746줄이 비테스트 코드 13,319줄의 1.4배이며, Sim.FirstLife는 플레이어 모성계 sol·아키타입 arch_terran을 이름으로 고정 단언해 v0.8 단계 4b(성역·arch_seeker)에서 깨질 예정이다.  (근거: grep -c IMPLEMENT_SIMPLE_AUTOMATION_TEST Source/Cosmic_Ascension/Tests/*.cpp 합계 76; wc -l Tests/*.cpp *.h → 18746, 비테스트 → 13319; Source/Cosmic_Ascension/Tests/CosmicSimTests.cpp:532-555; guideline.md:3655('고칠 테스트' 목록))
- A4-7 [judgment/high] 사용자의 전제('아키텍처와 기본 설정값이 확립되지 않았다')는 코드 기준으로 틀리다. 결정론·상태 모델·데이터 파이프라인은 상용 수준으로 닫혀 있고, 문제는 설정값의 부재가 아니라 네 곳 분산과 절반 가까운 미사용 노브다. MVP를 막는 진짜 공백은 플레이어 명령층(Player 모듈의 명령 1개)·저장·UI 피드백이다.  (근거: Source/Cosmic_Ascension/Core/CosmicSeed.h:43-89(골든값 static_assert); Sim/GalaxySimulationSubsystem.cpp:371-511(리플렉션 해시); Tools/data/schema.json(테이블 10·조건 30·효과 18); Player/PlayerRoleSubsystem.h:63-76(명령 API StartMegaproject만); A4-3·A4-4 근거)
- A4-8 [judgment/medium] 76개 테스트는 양이 아니라 구성이 문제다: 결정론·저장·데이터 약 20개는 MVP 내내 smoke로 지켜야 하고, 20,000틱 장기 판 12건과 시드 의존 증거 테스트 4종은 full 티어로 분리해 v0.8 전환 때 재측정 목록으로 관리해야 유지비가 통제된다.  (근거: grep -rn '20000' Source/Cosmic_Ascension/Tests/*.cpp | wc -l → 12; grep -rc AddExpectedError Tests/*.cpp 합계 46; grep -l 'FGameplayTag Tag(const TCHAR' Tests/*.cpp | wc -l → 10; Tests/CosmicSimTests.cpp:2057-2181(PhaseTwoExit 시드 42·7))
