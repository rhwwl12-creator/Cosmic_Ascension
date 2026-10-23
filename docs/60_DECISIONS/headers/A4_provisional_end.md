# MVP ④ 헤더 초안 — 임시 끝(통치 백성 T3 ∨ 틱 6,000)·연대기 최소형·연대기 화면

상태: **승인 (2026-10-23, 개발자 "커밋 셋 진행해줘" — 아래 §11의 결정 16개 전원 권장안) — 구현 중**(`git log --grep 'A-4'`). 첫 초안(세 설계안과 판정의 종합)에 적대 리뷰의 발견 21건과 빠진 것 15건을 넣었고, 그 뒤 검증의 발견 19건을 다시 넣었다. 고칠 수 있는 것은 본문에서 고쳤고, 개발자가 정할 것은 §11로 올렸다. **먼저 볼 결정은 1(판의 길이)·2(고정 줄의 자리와 임시 끝의 쪽)·3(폐위 뒤)이다.** 새 저장 필드·UENUM·USTRUCT·틱 단계 변경·추첨 지점은 **0**이다. 새로 드는 것은 DT_Rules 노브 하나, `schema.json` 최상위 키 `string_table`, 저장하지 않는 열거 하나(`ECosmicProvisionalEnd`)와 기존 열거의 끝 열거자 하나(`ECosmicAutoPause::RunEnd`), 새 UCLASS 하나(연대기 화면), ST_CA 키 22다. 이 헤더가 승인할 것은 아래의 C++ 선언과 ST_CA 키·글이다. 코드 사실은 HEAD `2269ff9`의 작업 트리에서 파일을 열어 확인했다(자리와 행 번호의 전체 표는 구현 메모 §0). 위젯의 꼴(색·크기·글씨), 개발 도구, 긴 코드 본문, private 멤버는 **구현 메모(헤더 승인 뒤 A-4b의 메모)** `docs/60_DECISIONS/headers/A4_provisional_end_notes.md`로 옮겼다.

근거: `docs/50_ROADMAP.md` A ④·⑤·종료 조건 ⓐ·ⓒ·ⓔ·위험 칸, `STATUS.md` 다음 작업 3·열린 결정 1·'2026-10-18 개발자 결정'(MVP = 통치자 단일 모드, ×10 기준 30~60분, 끝 = T3 ∨ 틱 6,000, 연대기 이관 #40 ⑦ + 상한 노브), `crisis_and_transcendents.md` §4.4(엔딩 이후·비조소 규칙·⑤ 책 = 연대기), `docs/70` §8.3·§8.4, `docs/20` §6.3·§9.1, `docs/21` 529·1204·1428행, `OPEN_QUESTIONS` #31 ②·#40 ⑦·#49·#67 ①, 헤더 A2·A3, `CLAUDE.md` 불변 규칙 2~8·10·11·14.

## 0. 범위와 전제

**④가 하는 일 다섯**
1. **끝의 판정**: `Sim/CosmicEndings.h`의 순수 함수 `ProvisionalEndOf`. 칸은 C++ 상수 `EndRung` = T3, 시계는 DT_Rules `ProvisionalEndTick` = 6,000이다.
2. **시간을 멈추는 일**은 표현 계층이 한다. 컨트롤러가 끝을 **래치**(한 번 서면 그 판 동안 들고 있는 표지)로 잡고, 그 뒤로는 `ApplySpeed` 한 자리가 배속을 0에 묶는다. 답과 프로젝트는 컨트롤러가 거절한다.
3. **연대기 최소형**: `Log/CosmicChronicle.h`의 `Admits`·`Build`·`VesselNameOf`와, 그것을 감싸는 `UCosmicLogSubsystem::BuildChronicle()`.
4. **연대기 화면**: 새 C++ 위젯, 상단 바·패널의 잠금, ST_CA `Chronicle.*`와 그 검사.
5. **커맨드렛의 일곱째 줄** `CosmicBalance end:`. 알리기만 한다.

**그대로인 것**: 두 해시, 기준값 셋(시드 42의 3,000·20,000틱, `ruler_summary_line`), `SaveVersion` 1, 최상위 멤버 18, 예약 25. `SimulateTick`·`StepOnce`·`RunHeadless`·커맨드렛·Core 명령은 끝을 모른다. 증명은 A-4a·A-4b 뒤 `run_checks.ps1 -Tier full`이 `-UpdateBaseline` 없이 통과하는 것이다.

**판의 길이 — 두 결정이 그대로는 함께 서지 않는다(결정 1).**
- **계산과 측정**: `SecondsPerTick` = 1.0이라(`Sim/GalaxySimulationSubsystem.h:503`) ×10은 초당 10틱이고, 6,000틱은 쉬지 않고 10분이다. 통치자 정책 30판 중 26판이 칸으로 끝나고(T3 중앙값 4,425틱 ≈ 7.4분, A3 헤더 §1.2), 시계로 끝나는 판은 4판이다.
- **어긋남**: '×10 기준 30~60분'(2026-10-18)과 '끝 = T3 ∨ 틱 6,000'은 **30~60분이 카드·연출·자동 정지로 멈춘 시간까지 센 벽시계일 때만** 함께 선다. 6,000틱까지 결정은 중앙값 19다. 결정마다 1분을 멈춰도 칸 끝 판은 20분 안팎이다(어림).
- **노브가 할 수 없는 일**: `ProvisionalEndTick`은 **시계로 끝나는 판만** 늘린다. 칸 끝을 늦추는 수단은 업적 문턱이고, 그것은 세계 해시와 기준값이 바뀌는 밸런스 작업이다.
- **재는 줄**: 래치가 설 때 `CosmicEnd: <까닭> at tick <틱>; <초> s since the run began, <초> s of them stopped`를 남긴다. 기준점은 컨트롤러 `BeginPlay` 또는 마지막 새 판·로드(`HandleGalaxyGenerated`)다.
- **ⓔ**: 로드맵 문장 그대로 **'외부 1명이 설명 없이 30분 뒤 "다음에 뭘 할지 알겠다"'**이다. 칸 끝 판은 30분 전에 끝날 수 있어, 그 시험 안에 연대기와 '새 판'이 들어온다. 그때의 '다음'은 쪽 아래 안내와 '새 판'이 말한다.

**나머지 범위**
- **⑤로 넘긴다**: #40 ⑦의 이관 거르기와 상한 노브 1,500, #70 압축, 틱 경계 자동 저장, 끝난 판의 이어하기, 백업 1. 게임 길이의 판에서는 이관이 일어나지 않는다(§1). 제약은 §4.4, STATUS의 문장은 로드맵 A ⑤에 맞춘다(결정 11).
- **그 뒤로**: B5는 두 책 장·비문·사망·폐위 화면·`BuildWorldDigest`·'거절한' 줄들이고, B6은 엔딩 넷과 그 저장 기록·시뮬의 거절이다. 단계 6은 서막 변형(#84), B1은 방랑자의 줄, EA 이후는 영겁 모드다.
- **커밋**: 단계 0 → **A-4a**(판정·연대기 순수 함수·노브·커맨드렛·측정) → **A-4b**(화면·잠금·ST_CA·검사) → 손 확인 → **A-4c**(문서·STATUS).

## 1. 이 초안이 기대는 사실과 측정 (자리와 행 번호의 전체 표는 구현 메모 §0)

**코드 사실**(결정에 쓰이는 넷만. 나머지는 구현 메모 §0)
- **게임 시작**: `NewGalaxy`는 `Super::StartPlay()` 앞, 곧 컨트롤러가 `OnGalaxyGenerated`에 묶이기 전에 돈다(`World/CosmicGameMode.cpp:89-108`). 새 은하·로드 뒤에는 `HandleMapRebuilt`가 선택·보기·카메라 틀을 되돌린다.
- **`PeakTier`**: 다스리는 백성의 업적에서만 오른다. 판정은 그릇이 Ruler인 동안이라 같은 틱의 폐위(열째 단계)보다 앞이다. 계승은 새 생애를 덧붙이고(`Lives`에 더하는 곳은 `Player/PlayerRoleSubsystem.cpp:97` 하나) 그 `PeakTier`를 개인 Tier로 연다(107행).
- **승천 연출과 자동 정지**: 둘 다 **달리는** 시뮬만 멈춘다(`RequestAutoPause`는 배속 > 0일 때만 — `CosmicGameMode.cpp:613`, 연출의 `Show`는 그때만 `PausedSpeed`를 쥔다 — `CosmicAscensionOverlayWidget.cpp:235`). `Close`가 그 배속을 돌려준다. 자동 정지가 켜져 있으면 승천 줄(S2)이 `OnTierAscended`보다 먼저 멈춘다(`docs/20` §6.3).
- **테스트**: 산 Sim을 쓰는 99종 모두가 `CosmicTests::AcquireSim`(`Tests/CosmicTestHelpers.h:69`)을 지나고, 그것은 배속을 `SetTimeScale(0)`으로 직접 둔다(`ApplySpeed`를 거치지 않는다). 방송을 켠 `StepOnce`로 6,000이나 T3를 넘는 테스트가 있다.

**측정**
- **시드 42, 전부 쌓기, 입력 없음**(`Saved/Balance/a3_20k.csv`): 6,000틱에 `Log` 834, `Chronicle` 0. 연대기가 처음 비지 않는 표본은 16,800틱이다(17,000에 13, 20,000에 85).
- **정책 판 3,000틱**(`checks_ruler.csv`): `Log` 294, `Chronicle` 0. 그래서 `Build`는 `Chronicle` 다음에 `Log`를 읽는다. 이관 규칙(`Log/CosmicLogSubsystem.cpp:202-210`)과 `Log.CapAndChronicle`은 그대로다.
- **정책 30판**: 다스리는 백성의 T2 중앙값 2,203, T3 4,425(26판).
- **입력 없는 그릇**: 30판 중 8판에서 폐위되고, 가장 이른 틱은 6,090이다(`DECISIONS_LOG` 2026-10-22). 실패한 왕좌 카드는 '다음 틱에 폐위'라 정책 판에서는 더 이를 수 있다.
- **재지 않은 것**: 폐위부터 끝까지의 간격, 6,000틱 판의 장 후보 줄 수(어림 50~80). 둘 다 A-4a가 잰다.

## 2. 상태 — 새 저장 필드 없음 (결정 4)

**저장하지 않는 까닭**
1. 술어의 입력(`Tick`, **모든 생애의** `PeakTier`)은 이미 저장되어 있고 줄지 않는다. `Lives`는 덧붙기만 하고 각 `PeakTier`는 오르기만 하므로, 계승(B5) 뒤에도 한 번 선 끝은 판 내내 서고 로드도 같은 답을 낸다. 그래서 B5와 B6 사이에도 이 술어가 그대로 쓰인다. 생애가 하나인 MVP에서는 `Lives.Last()`와 결과가 같다. B6의 저장 기록이 오면 `FollowEnd`가 그것을 읽는다.
2. 임시 규칙이 세이브에 영구 흔적을 남기지 않는다(규칙 7). 기준값·30판·정책 판에 끝을 끄는 인자도 들지 않는다.
3. 잃는 성질은 '끝난 판은 뒤의 `RunHeadless`·`StepOnce`가 시뮬하지 않는다'(`docs/21` 1204행) 하나다. 두 함수는 `BlueprintCallable`이고 `Exec`가 아니라 콘솔 길이 없다. 부르는 곳은 테스트와 커맨드렛뿐이다.
4. 노브를 바꾸면 시계로 끝난 판이 다시 열린다. 개발 중의 임시 규칙에는 맞는 성질이다.
5. 끝은 `DiedTick`(죽음이 아니다), `ReignEndReasons`(재위의 끝이 아니다), 로그 줄(책과 전체 해시에 남는다), `Knowing`과 대기 카드(숨길 뿐이다)를 쓰지 않는다.

**뒤 엔딩의 저장 기록 — ④에서는 넣지 않는다.** `docs/21`에는 '(B5·B6 — 예정) 엔딩의 저장 기록은 그 헤더가 정한다' 한 줄만 둔다. 스케치(`FGalaxyState` 끝의 `FCosmicEndingRecord`, 태그 `Ending.*`, 얼린 기본값 '빈 Kind = 판이 계속된다')는 구현 메모 §9와 `OPEN_QUESTIONS`의 새 행에 남긴다. 해시 포함 여부·쓰는 자리·명령의 거절은 B6 헤더가 정하고, 그때 `FollowEnd`가 그 기록을 읽는다.

## 3. 끝의 판정과 틱 (결정론)

### 3.1 술어 — `Sim/CosmicEndings.h` (새 파일, `CosmicHazards` 꼴의 순수 함수 namespace)
include는 `CoreMinimal.h`·`Core/CosmicTypes.h`이고, 앞선 선언은 `FGalaxyState`·`FCosmicRulesDef`다. `.cpp`는 `Core/GalaxyState.h`·`Data/CosmicDefinitionRows.h`만 읽는다(Sim → Core·Data). 같은 이름의 헤더는 타깃에 없다.

```cpp
// Cosmic Ascension - Sim layer (Core Logic). Design: crisis_and_transcendents.md section 4.4, headers/A4_provisional_end.md (MVP item 4).
// The single home of the conditions that end a run: pure functions of the state - no draw, no write, never Knowing. The four endings (B5, B6) come here too.

/** Why the MVP's provisional end holds. Not a UENUM: never saved, hashed or a data column (the append-only rule does not bind it); it goes with the four endings. */
enum class ECosmicProvisionalEnd : uint8
{
	None,
	Rung,   // a life's PeakTier reached EndRung (it rises only while the vessel rules; a climb in the tick of a deposition counts: feats are judged before the tenth stage)
	Clock,  // State.Tick reached DT_Rules.ProvisionalEndTick
};

namespace CosmicEndings
{
	/** The rung that ends an MVP run (2026-10-18: "T3 or tick 6,000"). A C++ constant, not a knob. */
	inline constexpr EAscensionTier EndRung = EAscensionTier::Tier3_Causal;
	/** None when Calling.Lives is empty; otherwise Rung when any Lives[i].PeakTier >= EndRung (the MVP has one life); otherwise Clock when Rules.ProvisionalEndTick > 0
	 * and State.Tick >= it; otherwise None (Rung wins a tie). Reads State.Tick, Calling.Lives and the knob only - never Civs, Knowing, the log or the chronicle. Both
	 * inputs only grow - Lives is append-only and a PeakTier only rises, through succession too - so once it holds it holds for the run and a load answers alike.
	 * Read by FollowEnd and the commandlet; no stage, command or run reads it. */
	COSMIC_ASCENSION_API ECosmicProvisionalEnd ProvisionalEndOf(const FGalaxyState& State, const FCosmicRulesDef& Rules);
}
```

**왜 `PeakTier`인가**(결정 5): 다스리는 동안 업적으로 오른 칸만 세고, 폐위 뒤에도 계승 뒤에도 남는다(어느 생애든 보므로 `Lives.Last()`가 바뀌어도 끝이 되돌아가지 않는다). 손으로 Tier를 올리는 UI 테스트 셋이 끝에 걸리지 않는다. 커맨드렛의 `RuledT3Tick`과 같은 틱이다(예외 하나는 §3.4). 그 대신 끝이 #67 ① 기본값에 기대어 그 기본값이 사실상 굳으므로, A-4c가 #67 ①에 그것을 적는다.

### 3.2 노브 — `FCosmicRulesDef`(`Data/CosmicDefinitionRows.h`) 끝, `FirstCrisisHazard`(980행) 뒤

```cpp
	// ---- The provisional end (MVP item 4; goes with the four endings of B5, B6) ----
	// The MVP run's clock end (CosmicEndings::ProvisionalEndOf): from the tick boundary where State.Tick reaches this - unless the ruled people reached the third
	// rung first - the presentation keeps the time stopped and opens the chronicle; the run's last tick is this one. 0 or below = no clock end. Nothing in the
	// simulation stops at it (the commandlet reports it). Start value 6000 = ten minutes of running at x10; it lengthens only a run the clock ends.
	UPROPERTY(EditAnywhere)
	int32 ProvisionalEndTick = 6000;
```

T3는 C++ 상수다(결정 6). DT_Rules는 저장되지 않는 정의 행이라 규칙 7이 걸리지 않는다. 커맨드렛은 `-Rules=ProvisionalEndTick:N`으로 다른 길이를 재고, 그래도 세계는 그대로다.

### 3.3 래치 넷 (표현 계층, 결정 7)
**거는 자리 — 모두 `FollowEnd`를 지난다**
1. `HandleSimTick`(틱 경계): `FollowEnd(true)`.
2. `HandleGalaxyGenerated`: `FollowSpeedGate()`를 지금처럼 맨 앞에 둔다(그 안의 `SetSpeed`는 잠금이 0으로 묶는다). → `ClearRunEnd()` → 까닭·위기 기억 지우기 → `FollowEnd(false)` → 끝나지 않았을 때만 `FollowOwnHazards`·`FollowPetition(false)`.
3. 컨트롤러 `BeginPlay`: 핸들러를 묶은 뒤 `FollowEnd(false)`. 게임 시작의 `NewGalaxy`(⑤의 이어하기도 같은 자리일 가능성이 크다)는 묶이기 전에 돈다. 이 자리가 없으면 끝난 상태로 선 게임이 흐른다.
4. HUD `BeginPlay` 끝: `FollowEnd(false)`. 두 `BeginPlay`의 순서에는 기대지 않는다. 래치는 한 번만 서고, 쪽은 둘째 부름이 연다(§5.2의 계약).

**읽는 쪽과 테스트**: `ApplySpeed`는 래치만 읽는다. 술어를 새로 읽으면 `RunHeadless(20000)`이 남긴 상태(`CosmicLogTests.cpp:134`의 `Log.NoConsecutiveRepeat`) 위에서 UI 테스트가 끝에 걸린다. `PlayerTick`은 래치를 걸지 않고, 기다림과 멈춘 초만 센다. 래치는 컨트롤러 멤버라 테스트를 넘어 남는다. `Sim.PhaseTwoExit`가 6,000을, `Sim.PlayerAscendsToT3`가 T3를 넘고, `LoadState`·`LoadGalaxy`는 언급 73줄(부름 65)이다. 그래서 grep에 기대지 않고 `CosmicTests::AcquireSim`이 첫 컨트롤러에 `SetProvisionalEndEnabled(true)`와 `ClearRunEnd()`를 부른다(Tests는 `World/`를 include할 수 있다).

**시계 끝**: 6,000번째 틱이 열 단계를 다 돈다. → 그 틱의 `OnSimTick(6000)`(열째 단계와 `SyncRngState` 뒤)이 래치를 걸고 배속을 0으로 둔다. 프레임 고리는 걸음마다 배속을 다시 보고(`Sim/GalaxySimulationSubsystem.cpp:571`) `SetTimeScale(0)`은 이월을 버리므로, 한 프레임이 여러 틱을 돌던 중이어도 6,001은 돌지 않는다.

**칸 끝(틱 t)의 두 순서**
- **자동 정지 켬(기본)**: 업적 판정이 `PeakTier` T3를 쓴다. → 승천 줄의 `OnLogEntry`가 먼저 `MythicLine`으로 멈춘다(그래서 연출은 `PausedSpeed`를 쥐지 않는다). → 나머지 단계. → `OnSimTick(t)`에 래치가 서고 까닭이 `RunEnd`로 바뀐다.
- **끔**: 연출이 `SetSpeed(0)`을 걸고 `PausedSpeed`를 쥔다. → `OnSimTick(t)`에 래치. → `Close`의 되돌림은 잠금에 걸려 0이다.
- 둘 다 같은 틱의 청원 카드는 앎에 남고 화면에 오르지 않는다.

### 3.4 끝 뒤의 입력·결정론·커맨드렛

| 길 | 끝 뒤 |
|---|---|
| `ApplySpeed`(단추, 키 0~5·Space, 연출·카드의 되돌림, `FollowSpeedGate`) | 맨 앞에서 `Speed = 0`, `Reason = RunEnd`. 문은 여기 하나다. 끝은 자동 정지가 아니다: 토글과 상관없이 멈추고 `RequestAutoPause`를 지나지 않는다 |
| 컨트롤러 `AnswerPetition`·`StartProject`(패널, `CosmicStartProject`) | Core를 부르지 않고 `Refused`·`false`를 낸다(추첨·쓰기 0). `FollowPetition`은 곧 돌아오고, 카드는 `FollowEnd`가 접는다 |
| Core 명령·`StepOnce`·`RunHeadless` | **끝을 모른다.** 그래야 정책 판이 6,000 뒤에도 같은 세계를 만든다. `RunHeadless` 주석에 "the MVP's provisional end is not such an ending: it never stops a run"을 더한다 |
| 컨트롤러 없는 상단 바(테스트 길, `CosmicTopBarWidget.cpp:245-249`) | 문이 없다. 게임에는 없는 길이다 |

**결정론**: 틱 단계·추첨·두 스트림·`CosmicSeed` 용도·두 해시가 그대로다. 술어와 연대기는 읽기만 하고, 노브는 상태 밖이고, 래치는 컨트롤러 멤버다. 앎은 읽지도 쓰지도 않는다. '새 판'의 시드는 입력이지 추첨이 아니다.

**커맨드렛**(A-4a)
- **보는 자리**: 틱마다 `RunHeadless(1)` 바로 뒤, 정책의 입력 앞에서 술어를 본다. 판은 멈추지 않는다. 처음 선 원인·틱과 칸·시계가 선 틱을 적고, 처음 선 틱의 상태로 `CosmicChronicle::Build`를 한 번 불러 마지막 장을 센다.
- **일곱째 줄**(헤더 주석 'six' → 'seven'): `CosmicBalance end: first Rung at 4425; rung at 4425, clock at 6000, deposed at -1; chronicle life 1 shown 40 of 63, more 1`. 닿지 않으면 −1, 끝이 없으면 `first None at -1`이다.
- **불변식**: 칸 틱 = `RuledT3Tick`, **또는** `RuledT3Tick` = −1 ∧ `DeposedTick` = 칸 틱. 뒤의 것은 같은 틱의 폐위다. 위기 단계가 폐위를 적고 → AI 단계가 아직 다스리는 그릇의 `PeakTier`를 올리고 → 열째 단계가 폐위한다. 그래서 틱 뒤에는 다스리는 백성이 없어 `RuledT3Tick`(`CosmicBalanceCommandlet.cpp:556-560`)이 서지 않는다.
- **어겼을 때**: 접두어가 다른 `Display` 한 줄 `CosmicBalance end check: …`를 내고 **`Result`는 바꾸지 않는다**(측정이지 문이 아니다 — `expansion` 합의 `Result = 1`과 다르다). 일곱째 줄을 `Select-Object -Last 1`로 집는 도구가 이 줄을 집지 않는다. A-4a의 60판에서 이 접두어의 줄은 0이어야 한다.
- **그대로**: `run_checks`·`ruler` 줄·`EndingType`(Phase 5). 'as long as there are no endings' 뒤에 "the MVP's provisional end belongs to the presentation and stops nothing"을 더한다.

## 4. 연대기 최소형

### 4.1 #31 ②의 앞 절반 — 8필드만으로 (결정 8)
- **든다**: `Log.Calling`·`Log.Claim`·`Log.Record`와 모든 S2. S1은 `FocusCivID`가 그 생애의 백성(`BirthCivID` ∪ `RuledCivIDs`)일 때(청원 결과가 여기 든다), 또는 `SourceRow` = `log_first_contact`일 때(`PushRow`가 행 이름을 적는다 — `CosmicLogSubsystem.cpp:100`) 든다.
- **안 든다**: S0, `log_contact`, 초점 없는 먼 줄. 먼 줄이 대신 말하던 `bUntold` S2 원문은 S2로 들어온다.
- **읽지 않는다**: `bUntold`와 행 표. 격차 창으로 말해지지 않은 줄도 끝에는 실린다(#49 기본값 '싣는다'의 뜻). ⑤의 이관 거르기도 이 함수를 모든 생애의 백성으로 부른다.
- **#31 ②에 남는 것**: `Log.Claim` 묶음 키(B5)와 `{OtherCiv}` 둘째 주제(기본값 '인자만', Phase 4). 행은 연 채 둔다.

### 4.2 생애 장 — `Log/CosmicChronicle.h` (새 파일, 순수 함수 namespace, USTRUCT 아님)
include는 `CoreMinimal.h`·`Containers/ArrayView.h`·`Core/GalaxyState.h`, 앞선 선언은 `FCosmicDataTables`다. 파일 머리에 Log 층의 RULE 줄과 "pure functions of the saved state - nothing saved, hashed, drawn or written; Knowing is never read (16 #49)"를 둔다.

```cpp
/** One line of a chapter: an entry of State.Chronicle or State.Log as stored (its Message already flattened). */
struct FCosmicChronicleLine
{
	FCosmicLogEntry Entry;
	bool bOwn = false;   // the life's own (Log.Calling, its peoples' Severity 2, its admitted Severity 1); false for another people's Severity 2 (dimmed)
};

/** One life's chapter ("the Nth", section 8.4). Built at display time, never saved. */
struct FCosmicChronicleChapter
{
	int32 LifeAt = INDEX_NONE;            // index into Calling.Lives ("the Nth" is Lives[LifeAt].LifeIndex)
	int64 FromTick = 0;                   // docs/21's BornTick/DiedTick range: from BornTick (the first life also every earlier tick) ...
	int64 ToTick = 0;                     // ... to DiedTick when set, else to the next life's BornTick - 1, else to State.Tick
	TArray<FCosmicChronicleLine> Lines;   // shown, in time order; copies, so the page outlives the next tick
	int32 Candidates = 0;                 // the lines left after the first repeat rule, before the cap (the commandlet's "of")
	bool bMore = false;                   // the cap left lines out: the chapter closes with ST_CA Chronicle.More (no count)
};

namespace CosmicChronicle
{
	/** Working cap of a life chapter's lines, set from the A-4a measurement. A display cap: nothing leaves the state. */
	inline constexpr int32 MaxLifeLines = 40;
	/** BirthCivID, then RuledCivIDs, without NAME_None and without repeats. */
	COSMIC_ASCENSION_API TArray<FName> PeoplesOf(const FLifeRecord& Life);
	/** 16 #31 (2) from the eight fields alone: Log.Calling, Log.Claim, Log.Record and every Severity 2; a Severity 1 entry when FocusCivID is one of Peoples or
	 * SourceRow is CosmicRows::LogFirstContact. Reads neither bUntold nor a table. MVP item 5's Chronicle filter (16 #40 (7)) calls it with every life's peoples. */
	COSMIC_ASCENSION_API bool Admits(const FCosmicLogEntry& Entry, TConstArrayView<FName> Peoples);
	/** One chapter per life (none for no life), over State.Chronicle then State.Log (already in time order), in four steps: gather what Admits holds for for the
	 * life's peoples, but Log.Claim and Log.Record (the two books', B5); drop a repeat of the entry just before (Candidates counts the rest); over MaxLifeLines keep
	 * by rank - Log.Calling whole, own Severity 2, admitted Severity 1, other peoples' Severity 2 - thinning the first rank that does not fit (Severity 1: first
	 * ceil(k/2) and last floor(k/2); others: index (i * n) / k), the later ranks none, bMore set; in time order again, drop a repeat once more (not in bMore). */
	COSMIC_ASCENSION_API TArray<FCosmicChronicleChapter> Build(const FGalaxyState& State);
	/** {Player}: PersonNameBank[NameIndex % Num] of BirthArchetypeRow, else of arch_default (Install's rule); empty when neither resolves ("the Nth" alone). */
	COSMIC_ASCENSION_API FText VesselNameOf(const FLifeRecord& Life, const FCosmicDataTables& Tables);
}
```

**규칙의 차례**(표시 규칙, 상태 없음 — 결정 9)

| 걸음 | 하는 일 |
|---|---|
| ① 모음 | 장 범위 안의 들인 줄을 시간순으로 모은다(`Claim`·`Record`는 뺀다) |
| ② 되풀이 ① | 바로 앞 줄과 같은 `Message`를 뺀다(`docs/21` 1428행). 남은 수가 `Candidates`다. 같은 문장이 상한 자리를 먹지 않는다 |
| ③ 상한 40 | 등급 차례로 채운다: r0 `Log.Calling`(깎지 않는다) → r1 그 생애 백성의 S2 → r2 들인 S1 → r3 다른 백성의 S2(`bUntold` 포함). 다 들지 못한 첫 등급은 남은 자리 k로 솎는다. r2는 처음 ⌈k/2⌉와 마지막 ⌊k/2⌋를 남긴다('지혜로운 마음'과 끝 무렵의 선택이 함께 남는다). 그 밖은 `(i × n) / k`를 남긴다. 뒤 등급은 0이고 `bMore`가 선다 |
| ④ 되풀이 ② | 시간순으로 되돌린 뒤, 솎기가 붙여 놓은 같은 문장을 한 번 더 뺀다(`bMore`에 넣지 않는다) |

정수 + − × ÷만 쓰고, 앎으로 줄을 세우지 않는다. **상한 40은 가칭이다.** A-4a의 30판으로 28~64 사이에서 정한다. 잣대는 '정책 판 중앙값에서 r0~r2가 들고, 상한은 r3에서만 문다'이다. **장의 범위는 `docs/21` 529행(BornTick/DiedTick)을 따른다.** MVP에는 생애가 하나라 어느 정의든 같다. 공백기의 줄은 어느 장에도 없고, B5가 자리를 정한다. 틱에서 장을 찾는 함수는 부르는 곳이 없어 두지 않는다(`Build`는 범위를 스스로 센다). B5의 공백기 배치가 필요하면 그 헤더가 더한다.

### 4.3 진입점·쪽의 차례·층
**진입점**: `UCosmicLogSubsystem`에 `TArray<FCosmicChronicleChapter> BuildChronicle() const;`를 둔다. 주석은 "the chronicle up to the current tick: CosmicChronicle::Build over the live state; reads only and draws nothing, so a handler may call it; Phase 4b adds the two books, the epitaph and the digest around the same chapters"다. `docs/21`의 가칭 `TArray<FCosmicLogEntry>`를 장 배열로 바꾼다(장과 넘침 표지는 평면 배열에 실리지 않는다). 클래스 주석 `CosmicLogSubsystem.h:47-48`도 고친다.

**쪽의 차례 — 문서를 따른다(결정 2).** `docs/70` §8.4는 두 책 장을 '생애별 장들 뒤·에필로그 앞'에, 고정 문장 둘을 '에필로그 첫·둘째 줄'에 둔다. `docs/21` 1428행도 같다.
1. **생애마다**: 제목(`Chronicle.Life` + `VesselNameOf`를 `List.Pair`로 — "제 1대 · 이소란", 이름이 비면 제목만) → 사실 줄(다스린 백성의 `CivName` · `Panel.Tier` `PeakTier + 1` · `Chronicle.Stamp`(`ToTick − BornTick`)를 `CosmicUI::JoinList`로 — UI 줄이라 칸 번호를 숫자로 쓴다, §8) → 줄마다 `Chronicle.Stamp`(늘 주기 — `UI/CosmicUILabels.h:22-24`) + `Message` → 넘치면 `Chronicle.More`.
2. (두 책 — B5)
3. `Chronicle.Opened`, 이어 `Chronicle.Unopened`.
4. `Chronicle.Horizon`.

스크롤 밖 **아래**에 까닭 줄·`Chronicle.Provisional`·단추 둘을 둔다(UI 안내다). 까닭 줄은 고정 첫 줄과 붙지 않는다 — 사이에 `Unopened`·`Horizon`과 스크롤의 끝이 있다. **두 끝의 쪽은 같고, 임시 끝의 쪽에도 고정 줄 셋을 싣는다**(결정 2 — `docs/70` §8.4 '모든 엔딩 동일', `crisis` §4.4 비조소 규칙). 다른 것은 기록된 사건과 까닭 한 줄뿐이다. 비문 칸은 없다. 선택·업적·완성·폐위(`log_fallen_monarch`, `Log.Calling` S2)가 이미 줄로 있다.

**층**: `Log/CosmicChronicle.*`는 Core·Data만 include하고, 읽는 곳은 UI·World·커맨드렛이다. 연대기는 앎이 아니라 책이라(#49) 규칙 14의 `KnownViewOf`와 부딪치지 않는다. 피드와 지도의 앎은 그대로다. 태그 리터럴 셋이 늘어 린트 기준이 38 → 41이 된다(대안은 결정 16).

### 4.4 #40 ⑦ — ⑤가 지킬 제약 (A-4c가 #40 ⑦에 적는다)
1. 이관 거르기는 `Admits`를 모든 생애의 백성으로 부른다. `Admits`는 #40 ⑦ 기본값('그릇의 백성에 관한 Severity 1')보다 넓다 — `log_first_contact` S1(초점은 상대 백성)과 Severity에 상관없는 모든 `Log.Calling`을 들이고, ⑤의 이관도 그것을 남긴다. A-4c가 #40 ⑦의 기본값 문장을 `Admits`로 바꾼다(결정 8).
2. 저장 상한은 오래된 S1만 밀어낸다. S2·`Log.Calling`·`Log.Claim`·`Log.Record`는 지우지 않는다(`Law.Judgment`).
3. 밀어냈다는 표지는 저장 필드라서 ⑤ 헤더의 일이다.
4. 최악은 `Log` 2,000 × 600 B + 연대기 1,500 × 600 B ≈ 2.1 MB다. 그래서 #70 압축이 함께 든다.
5. 이어하기의 로드는 `Super::StartPlay()` 앞(BeginPlay가 잡는다)이나 `OnGalaxyGenerated`를 방송하는 길로 한다. 그래야 끝난 판이 끝난 채 선다. ⑤의 이어하기 테스트는 끝난 세이브로 게임을 시작해 래치·다 드러난 쪽·카드 없음을 단언한다(④의 테스트는 두 `BeginPlay`의 부름을 지키지 않는다 — §10).

## 5. UI

### 5.1 흐름 (결정 12·13)
- **칸 끝**: 승천 연출(그대로) → 닫힘(되돌림 0) → 영적 보기면 "이제 보인다."(3초) → 멈춘 지도 **1.5초** → 베일 1초 → 줄이 0.25초 간격으로 드러난다(쪽 전체 6초 안). **베일을 한 번 누르면 다 드러난다.** 단추는 다 드러난 뒤에만 서서, 엉뚱한 클릭이 '새 판'에 닿지 않는다. 영적 시야는 '지도 보기'에서 다시 켤 수 있고, ×25 단추는 서되 회색이다.
- **시계 끝**: `OnSimTick(6000)` → 정지(소리 없음)·카드 접힘 → 1.5초 → 연대기.
- **폐위된 그릇이 끝까지 보는 것**(결정 3): 명령 단추가 없고(방랑자의 패널은 Phase 4), 청원도 없다(MVP ②의 카드는 다스리는 그릇에게만 온다). 할 수 있는 것은 배속·카메라·선택·피드다. 새로 드는 것은 상단 바의 회색 줄 하나다. 폐위 틱부터 `Chronicle.Until`("이 판은 주기 6,000에 멈춘다.")이고, 마지막 600주기에는 `Chronicle.Soon`이다.
- **끝 뒤**: '지도 보기'를 누르면 쪽이 접히고 시간은 멈춘 채다(카메라·선택·패널·피드·시야 토글은 된다). 상단 바 '연대기'가 다 드러난 쪽을 다시 연다. '새 판'은 **3초 안에 두 번** 누른다(④에는 저장이 없다). 시간을 다시 흐르게 하는 길은 없다.
- **끝난 판으로 서는 게임**(로드, 두 BeginPlay): 기다림 없이 다 드러난 채 열리고, 대기 카드는 오르지 않는다.
- **'새 판'이 지우는 것**: `StartNewRun` = `NewGalaxy(시계 시드, 0, 0)` → `SetSpeed(1)`이다. `HandleGalaxyGenerated`가 래치·기다림·쪽·까닭·위기 기억을 지우고 끝의 초 셈을 새로 잡는다. 선택·은하 선택·보기(영적)·카메라 틀은 지도의 다시 짓기 뒤 `HandleMapRebuilt`가 되돌린다(지금의 새 은하와 같은 길). 연출과 드러냄은 쪽이 열리기 전에 이미 끝나 있고, `SightWasOpen`은 `PlayerTick`이 따라간다. 남는 것은 힌트 진행·시계 단위·자동 정지 설정·개발 스위치다. **시드는 늘 시계이고, `-Seed=`는 첫 판만이다.** `StartPlay`의 식을 cpp 안 `CosmicControlImpl::ClockSeed()`로 옮겨 함께 쓴다.

### 5.2 컨트롤러·HUD·자동 정지 열거 (`World/CosmicGameMode.h`, `World/CosmicAutoPause.h`)

```cpp
	// ACosmicPlayerController - public
	/** The provisional end as FollowEnd latched it (MVP item 4) - never read afresh elsewhere, so a state a test left behind ends nothing until a tick boundary,
	 * a new galaxy, a load or a BeginPlay says so. None while the run goes on or the end is switched off. Never saved, outside both hashes. */
	ECosmicProvisionalEnd GetRunEnd() const { return RunEnd; }
	bool IsRunEnded() const { return RunEnd != ECosmicProvisionalEnd::None; }
	static bool IsRunEndedFor(const UUserWidget& Widget);   // the widget's owning controller holds an ended run; false without one
	/** Looks at the end (nothing while it is switched off). bArrived (HandleSimTick; tests in its place): a cause seen while none is latched latches it, stops the
	 * time whatever the automatic pause (ApplySpeed(0, RunEnd)), folds the card, logs one CosmicEnd line and starts the wait. Without it (HandleGalaxyGenerated, the
	 * BeginPlay of this controller and of the HUD): the same latch with no wait, and while latched the page opens at once, fully shown, if the HUD has it and it is
	 * closed - whichever BeginPlay runs second opens it. Calls nothing a Core Logic handler may not call (only SetTimeScale reaches the simulation). */
	void FollowEnd(bool bArrived);
	void AdvanceRunEnd(float DeltaSeconds);   // real seconds, counted only while no ascension presentation or reveal shows (PlayerTick; tests); opens after HoldSeconds
	void ClearRunEnd();                       // forgets the latch, the wait and the page; switch and speed stay, the RunEnd reason goes (to None) (HandleGalaxyGenerated first; CosmicTests::AcquireSim)
	void SetProvisionalEndEnabled(bool bOn);  // on; -ProvisionalEnd=0 turns it off for the session, tests in memory; off = ClearRunEnd
	bool IsProvisionalEndEnabled() const { return bProvisionalEnd; }
	void LookAtMap();                         // the page folds over the stopped map
	void ReopenChronicle();                   // the top bar's 'chronicle': the page again, fully shown
	void StartNewRun();                       // NewGalaxy from the clock's seed (-Seed= names the first galaxy only), then x1; an input, never from a handler
	UFUNCTION(Exec)
	void CosmicPreviewChronicle(float DelaySeconds);   // development: the page over the current state after DelaySeconds, fully shown, no cause; changes nothing

	// ACosmicHUD - public, and its private UPROPERTY
	UCosmicChronicleWidget* GetChronicle() const { return Chronicle; }
	UPROPERTY(Transient)
	TObjectPtr<UCosmicChronicleWidget> Chronicle;

	// ECosmicAutoPause (a plain enum class, never saved) - appended at the end
	RunEnd,   // the run is over - not an automatic pause: set whatever the viewer's choice; every speed asked for afterwards stays 0 (ApplySpeed)

	// UCosmicAscensionOverlayWidget - public (tests: UI.RunEnd (6))
	float GetPausedSpeed() const { return PausedSpeed; }   // the speed Close gives back; 0 when the simulation was already stopped at Show
```

- **private 멤버**(구현 메모 §1): `RunEnd`, `bProvisionalEnd`, `ChronicleWait`, 그리고 `CosmicEnd` 줄의 두 초 `RunBeganRealSeconds`·`StoppedRealSeconds`(`UWorld::GetRealTimeSeconds` 기준).
- **까닭 줄**: `WhyKeyOf(RunEnd)` = `Chronicle.Why`이고, 상단 바(`CosmicTopBarWidget.cpp:446`)가 읽는다.
- **`HandleSimTick`의 차례**: `FollowSpeedGate` → `FollowEnd(true)` → 끝났으면 돌아온다 → `FollowOwnHazards` → `FollowPetition(true)`.
- **나머지 자리**: `PlayerTick`은 `AdvanceRunEnd`를 부르고 멈춘 초를 센다. `PostInitializeComponents`가 `-ProvisionalEnd=`를 읽는다. `HandleWheel`은 쪽이 펼쳐진 동안 쉰다. HUD는 위젯을 `AddToViewport(20)`에 접어 둔다.

### 5.3 연대기 위젯 — `UI/CosmicChronicleWidget.h/.cpp` (새 UCLASS, C++로 짓는다, WBP 없음)
상수 `CosmicChronicleScreen::`(선언은 구현 메모 §3): `HoldSeconds` 1.5 · `VeilSeconds` 1 · `LineSeconds` 0.25 · `RevealCapSeconds` 6 · `WorldLineOpacity` 0.7 · `ConfirmSeconds` 3 · `EndSoonTicks` 600(int64).

```cpp
/** The chronicle screen (MVP item 4, section 8.4's minimal form): a dark veil, one centred column in a scroll box (life chapters, the epilogue's two fixed
 * lines, the horizon line) and, outside the scroll, the cause note, the provisional notice and two buttons. A click on the page is not a map click. */
UCLASS()
class COSMIC_ASCENSION_API UCosmicChronicleWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void Open(ECosmicProvisionalEnd Cause, bool bAtOnce);   // Cause picks the note (None = the development preview: no note, no provisional notice, no 'new run'); bAtOnce: no fade, no reveal
	void Close();                                           // a 'new run' half pressed is forgotten
	bool IsOpen() const { return bOpen; }
	bool IsRevealing() const { return bOpen && !bRevealed; }   // lines still to show (the buttons wait)
	void Advance(float DeltaSeconds);                       // the fade, the reveal and the confirm window (its tick; tests)
	void PressVeil();                                       // tests in place of clicks: every line at once, then the buttons
	void PressToMap();
	void PressNewRun();                                     // the first press changes the words; a second within ConfirmSeconds calls StartNewRun
	TArray<FText> GetPageTexts() const;                     // tests: the page's texts in order
	TArray<float> GetLineOpacities() const;
	const UTextBlock* GetNoteText() const { return NoteText; }
	UCosmicActionButton* GetToMapButton() const { return ToMapButton; }
	UCosmicActionButton* GetNewRunButton() const { return NewRunButton; }
	// protected overrides (RebuildWidget, NativeTick, mouse down / double click) and private members: implementation notes section 3
};
```

- **승천 오버레이를 다시 쓰지 않는다**: 5초짜리 제목 연출이라 스크롤 목록을 담지 못한다. `docs/70` §8.4의 '새 위젯 금지'는 사망·폐위 3단의 규칙이다. 이 쪽은 B5·B6의 엔딩이 그대로 쓸 뼈대다.
- **상단 바**: 끝난 판에서는 배속 단추가 모두 회색이고 `Press`도 막힌다(까닭 줄 `Chronicle.Why`). 끝났고 쪽이 접혔을 때만 `Chronicle.Reopen` 단추가 선다. 시계 툴팁 셋째 줄은 `Chronicle.Rule`이다(`{0}` = `CosmicUI::ClockLabel(노브, Cycle)` — 툴팁은 한 번만 지어 늘 주기라 "주기 6,000"이고, `Until`의 주기 꼴과 같다. 노브 ≤ 0이면 `RuleRung`, 꺼짐이면 없다). 회색 줄은 끝 전에만 선다. 남은 틱이 1~600이면 `Chronicle.Soon`(`CosmicUI::TimeLeftLabel`), 그 밖에 폐위된 그릇이면 `Chronicle.Until`(`CosmicUI::ClockLabel(노브)`)이다.
- **성계 패널**: `IsRunEndedFor(*this)`이면 명령 단추를 지금의 사유 길(회색 + 짧은 사유 + 사유가 앞인 툴팁)로 `Chronicle.Suffix`·`Chronicle.Block`에 둔다. **정착 단추도 같다** — 따로 가는 길이라(단추 아래 사유 줄이 `ReasonOf(ShownSettleBlock)`을 읽는다 — `CosmicSystemPanelWidget.cpp:703-704`) 툴팁의 사유와 아래 사유 줄 모두 `Chronicle.Block`이고, 값 줄과 짧은 사유는 붙이지 않는다(정착 단추에는 짧은 사유 꼴이 없다).

## 6. 데이터와 문구

**DT_Rules와 에셋**: `DT_Rules.json` 끝에 `"ProvisionalEndTick": 6000`, `schema.json`에 `{"kind": "number", "min": 0, "integer": true}`를 둔다. 노브는 52 → 53(읽힘 34 → 35, 예약 18 그대로)이다. `make_data_tables.py`로 `DT_Rules`·`ST_CA` 에셋을 다시 만들고 `docs/30`도 다시 만든다.

**ST_CA**(`Tools/data/ST_CA.csv` 끝, 이름 키)

| 키 | 글 | 자리 |
|---|---|---|
| `Chronicle.Life` | 제 {0}대 | 장 제목(허용된 꼴) |
| `Chronicle.Stamp` | {0}주기 | 줄 머리, 사실 줄의 주기 |
| `Chronicle.More` | 그 외에도 있었다. | 넘침. 문서 고정(#49, '그 외 N개' 금지) |
| `Chronicle.Opened` | 기록이 펼쳐졌다. 네가 한 일이 네 앞에 놓인다. | 에필로그 첫 줄. 문서 고정(`crisis` §4.4 ⑤, `docs/70` §8.4) |
| `Chronicle.Unopened` | 다른 책은 펴지지 않았다. | 에필로그 둘째 줄. 문서 고정 |
| `Chronicle.Horizon` | 이 연대기는 손으로 짓지 않은 나라를 기다리는 자들의 기록이다. | 쪽 끝. 문서 고정(마침표만 더했다) |
| `Chronicle.Note.Rung` | {Civ}\|hpp(이,가) 셋째 칸에 이르렀다 — 이 판은 여기까지다. | 까닭(UI). `{Civ}` = `PeakTier ≥ EndRung`인 첫 생애의 `RuledCivIDs`를 뒤에서부터 보아, `Civs`에 남아 있고 Tier ≥ `EndRung`인 첫 백성 |
| `Chronicle.Note.RungNoCiv` | 다스리던 백성이 셋째 칸에 이르렀다 — 이 판은 여기까지다. | 그런 백성이 없을 때 |
| `Chronicle.Note.Clock` | {0}주기가 흘렀다 — 이 판은 여기까지다. | 까닭(UI), `{0}` = 끝의 틱 |
| `Chronicle.Provisional` | 시험판의 끝이다. 이 다음의 이야기는 아직 만들어지지 않았다. | 테스터 안내(ⓔ) |
| `Chronicle.ToMap` | 지도 보기 | 단추 |
| `Chronicle.NewRun` | 새 판 | 단추 |
| `Chronicle.NewRunConfirm` | 한 번 더 누르면 새 판이 열린다 | 단추(첫 누름 뒤 3초) |
| `Chronicle.NewRunTip` | 새 은하에서 새 판을 연다. 이 판은 남지 않는다. | 단추 툴팁(⑤가 판을 남기면 고친다) |
| `Chronicle.Reopen` | 연대기 | 상단 바 단추 |
| `Chronicle.Why` | 정지 — 이 판은 여기까지다 | 상단 바 까닭 줄(`AutoPause.Why.*`의 꼴) |
| `Chronicle.Block` | 이 판은 여기까지다 — 명령을 받지 않는다. | 꺼진 명령 단추 툴팁의 첫 줄(`Start.Block.*`의 꼴). 초안의 `Chronicle.Stopped`("…시간은 더 흐르지 않는다." — 종말 진술로 읽힌다)를 바꿨다 |
| `Chronicle.Suffix` | (멈춘 판) | 이름 뒤 짧은 사유(`Start.Suffix.*`의 꼴) |
| `Chronicle.Rule` | 이 판은 다스리는 백성이 셋째 칸에 오르거나 {0}에 이르면 멈춘다. | 시계 툴팁 셋째 줄, `{0}` = "주기 6,000"(`ClockLabel`, 늘 주기) |
| `Chronicle.RuleRung` | 이 판은 다스리는 백성이 셋째 칸에 오르면 멈춘다. | 노브가 0 이하일 때 |
| `Chronicle.Soon` | 이 판은 곧 멈춘다 — {0} | 회색 줄(마지막 600주기) |
| `Chronicle.Until` | 이 판은 {0}에 멈춘다. | 회색 줄(폐위 뒤), `{0}` = "주기 6,000" 또는 "6,000년" |

**규칙 11에 비춘 점검**: `words.verdict`의 일곱 낱말이 없다. 사람·성계의 수, 거리, 자원 수가 없고, 숫자는 주기와 칸뿐이며 모두 `{}` 인자다. 끝은 판의 규칙으로만 말한다('이 판은 여기까지다'·'멈춘다'). 하나님은 화자도 끝의 조건도 아니고, '때가 찼다'·'허락되었다'·'시간이 더 없다'를 쓰지 않는다. '기록이 펼쳐진다'는 `Law.Judgment`의 수동 읽힘이라 에필로그 첫 줄에만 둔다. 예고·툴팁·까닭에 묶으면 거룩한 질서를 타이머, 곧 끝의 조건에 가깝게 만든다. 같은 읽기가 쪽에서 생기지 않게, 쪽의 첫 줄은 그 판의 기록 뒤에 서고 까닭("…주기가 흘렀다 — 이 판은 여기까지다")은 스크롤 밖 아래의 UI 안내로 따로 선다 — 끝은 판의 규칙이 말하고, 첫 줄은 무엇이 끝을 냈든 같은 읽힘이다(비조소 규칙). 그래서 시계 끝의 쪽에도 싣는다(결정 2). 칸에 오른 주어는 백성이고, '신이 되었다'·칭호·승패·GAME OVER가 없다. 서술자 문장은 문서 고정 넷뿐이고, 새 서술 문장도 새 DT 행도 없다.

**검사기**
- `schema.json`에 새 최상위 키 `"string_table": {"checked_prefixes": {"Chronicle.": {"forbidden": "verdict", "no_digits": true}}}`를 둔다.
- `cosmic_data_schema.py`: `TOP_KEYS`에 그 키를 더하고, `check_string_table`이 스키마를 받는다. `no_digits`는 **`{…}` 밖의 숫자만** 막는다(DT 열의 `no_digits`(811행)는 글 전체를 본다는 차이를 주석에 적는다). 없는 목록은 스키마 문제로 알린다(646행).
- `test_data_schema.py`의 `test_string_table`에 다섯 경우를 더한다: 금지어 실패, 인자 밖 숫자 실패, `{0}주기` 통과, 접두어 밖 통과, 없는 목록. 단위 테스트는 73종 그대로다.

## 7. 테스트 (새 단언은 변이로 한 번씩 깨뜨린다)
새 파일은 `Tests/CosmicChronicleTests.cpp`(Sim·Log)와 `Tests/CosmicChronicleScreenTests.cpp`(UI)이고, 102 → **106종**이 된다. 아래 표는 승인할 단언의 이름과 변이 수만 둔다. 단계·값(5,997, `HoldSeconds − 0.1` 등)과 변이의 목록은 구현 메모 §7.1에 있다.

| 테스트 | 단언 | 변이 |
|---|---|---|
| `Sim.ProvisionalEnd`(새) | **표**: 생애 없음 → None · 칸 · 시계(경계 6,000) · 노브 0·음수 · 같은 틱은 Rung · `Civs[].Tier`만 오른 판은 None · **생애 둘에서 앞 생애의 T3도 Rung** · 앎·로그·연대기·말하기 스트림과 무관. **순수**(두 해시·`RngState`). **멈추지 않는다**(`RunHeadless`·`StepOnce`, 노브 200·0의 같은 세계) | 7 |
| `Sim.PlayerAscendsToT3`(넓힘) | T3 틱에 처음 Rung, 그 전 틱 None | 1 |
| `Log.ChronicleBuild`(새) | **`Admits` 11칸**과 `bUntold` 무관 · 장에 Claim·Record 없음 · 두 배열의 차례 · **되풀이 ①·②** · **상한**(40/41, r3 솎기, r2 앞뒤) · **범위**(두 생애, `DiedTick`, 마지막 `ToTick`) · 앎 무관·두 해시 · `VesselNameOf` 셋 · `BuildChronicle()` = `Build` | 10 |
| `UI.RunEnd`(새, 산 컨트롤러·HUD) | ① 시계 끝은 정확히 6,000 ② 끝 전 배속 ③ 잠금(배속·키·명령·**정착 단추**) ④ 카드 접힘 ⑤ 기다림 ⑥ **칸 끝의 두 순서**(승천 틱 앞에 `SetSpeed(10)` — 켬: 연출의 `PausedSpeed` 0, 틱 뒤 `RunEnd` / 끔: `PausedSpeed` 10, `Close` 뒤 배속 0) ⑦ 끝난 사본의 로드 ⑧ `FollowEnd(false)`의 계약 ⑨ 새 판 ⑩ 스위치 끔 ⑪ `AcquireSim` 리셋(래치·`RunEnd` 까닭) ⑫ 두 해시 | 10 |
| `UI.Chronicle`(새) | **쪽**(문서 순, 스탬프는 늘 주기, 불투명도 1/0.7, 색 `ColorOf`) · **까닭**(Rung·RungNoCiv·Clock 뒤 `Provisional` / 미리보기는 까닭·`Provisional`·'새 판' 없음) · **드러냄** · **'새 판'의 확인** · **상단 바**(다시 열기·`Rule`·예고·폐위 뒤 줄·끝 뒤 없음) | 8 |
| `UI.MouseOnlyAscent`(꼬리 고쳐 씀, `CosmicPanelTests.cpp:753-770`) | 단추만으로 T3과 시야 단추까지는 그대로다. ×25 구간을 **종료 조건 ⓒ**로 바꾼다: 칸 끝 → 쪽 → '지도 보기', ×25 단추는 서되 회색이고 누름·`SetSpeed(25)` 모두 0. 'and its press runs x25'는 지운다 | 1 |
| `UI.AutoPause` · `test_data_schema.py`(넓힘) | 까닭 고리(`CosmicAutoPauseTests.cpp:156`)에 `RunEnd` → `Chronicle.Why` · 검사기 다섯 경우와 노브 범위 | 1 · 2 |
| 그대로여야 할 것 | `Sim.TellStreamSeparate`(18), `Sim.SaveRoundTrip`(25), `Log.CapAndChronicle`, `Log.TellScope`, `Sim.KnowingInert`, `Sim.FrameStep`, `UI.ThirdRungSpeed`·`AscensionOverlay`·`SpiritualSight`(PeakTier T1), 기준값 셋 | — |

## 8. 문서 (A-4c)
`docs/20` §6.3에 '임시 끝(MVP ④)' 단락(술어 하나, 래치 넷, 문 하나, 칸 끝의 두 순서, `CosmicEnd` 줄, '끝은 자동 정지가 아니다'·'시뮬은 끝을 모른다')을 두고, §9.1에 '저장하지 않는다 — 줄지 않는 `Tick`과 모든 생애의 `PeakTier`에서 다시 읽힌다'를 적는다. `docs/21`에는 새 타입·함수·노브와 `BuildChronicle`의 새 꼴을 적는다. 1204행은 '(B5·B6 — 네 엔딩과 함께. MVP의 임시 끝은 들지 않는다)'로, 529행에는 'Build는 Log와 함께 읽는다'를 더하고, **엔딩의 저장 기록은 한 줄만** 둔다. `docs/70` §8.4에는 `Admits`·네 걸음과 상한·④가 문서 연대기의 어느 부분인지·고정 줄의 키와 검사·임시 끝의 쪽에도 고정 줄 셋을 싣는 것(결정 2)·예고에 '기록이 펼쳐진다'를 쓰지 않는 까닭·'장 머리의 사실 줄은 UI 줄이다 — 칸 번호를 숫자로 쓴다(§8.3의 연대기 숫자 규칙은 서술 줄에만)'·2인칭 메모를, `crisis` §4.4 '엔딩 이후'에는 한 줄을 둔다. `docs/40`(테스트 여덟, `end:`·`CosmicEnd` 줄, 두 개발 도구, `string_table`), `docs/45`(`UScrollBox::ScrollToStart`(`Components/ScrollBox.h:313`), `UPanelWidget::AddChild`·`ClearChildren`(`PanelWidget.h:59, 122`), `UWorld::GetRealTimeSeconds`(`Engine/World.h:2861`)), `docs/15`('임시 끝' 행), `docs/50`(A ④ 날짜·링크, A ⑤ 제약, 결정 1의 결과)을 고치고 `docs/30`을 다시 만든다. `OPEN_QUESTIONS`: #31 ② 앞 절반을 닫고(Claim 키·`{OtherCiv}`는 남긴다), #49를 기본값으로 닫고, #40 ⑦에 §4.4를 적고 그 기본값 문장('그릇의 백성에 관한 Severity 1')을 `Admits`로 바꾸며, #67 ①에 '임시 끝이 이 기본값에 기댄다'를 적고, 새 행 '엔딩의 저장 기록(B6 헤더 전)'을 스케치와 함께 둔다. `DECISIONS_LOG`, `STATUS`(열린 결정 1 → ⑤의 #40 ⑦), `CHANGELOG`, `CLAUDE.md` 규칙 15 한 구절. 코드 주석은 `RunHeadless`(`.h:170-173`), 커맨드렛 헤더, `CosmicLogSubsystem.h:47-48`, `GalaxyState.h:768`, `check_layers.py`.

## 9. 순서·검증·effort
**effort와 ultracode**: STATUS 다음 작업 3은 '헤더 먼저(xhigh), 구현 high, ultracode 끔'이다. 한편 `WORKPLAN` §2 작업 유형 표는 '헤더·구조체·열거형 설계: 초안 단독 → ultracode 적대 리뷰 → 수정'이라고 한다. 이 헤더 단계는 개발자가 ultracode를 켜 두었다(이 리뷰가 그것이다). **권장**: 헤더 단계는 켬, 구현은 high에 끔. A-4c에서 STATUS의 문장을 '구현 단계의 ultracode 끔'으로 고친다. 새 UCLASS와 헤더 변경이 있으므로 에디터를 닫고 전체 빌드한다.

0. **시작 전(개발자와) — low, 끔**: 커밋하지 않은 `DECISIONS_LOG.md` 변경이 있다(빈 첫 줄, 지워진 '확정 완료 (2026-10-19, A0 구현 …)' 13줄). 되돌릴지 정하고, A-4c의 기록과 섞지 않는다.
1. **승인 — xhigh, 켬**: 결정 1·2·3을 먼저 본다.
2. **[A-4a] — high, 끔**: §3, §4.1~4.3, 노브(데이터·스키마·에셋·설정 표), 린트 기준, Sim·Log 테스트 셋을 짓는다. **측정**은 정책 30판(`-TellAll=0 -Answer=1 -Build=1 -Ticks=6000`, 시드 1~29와 42)과 입력 없는 30판(`-TellAll=0 -Ticks=6000`, 같은 시드 — 끝의 원인과 시계 끝의 쪽 길이)이다. 원인별 끝 틱, 마지막 장의 후보·보인 줄·넘침, `end check:` 줄 0을 본다. 6,000 전 폐위와 **폐위부터 끝까지의 간격**(결정 3)은 정책 30판에서만 잡힌다(입력 없는 판의 가장 이른 폐위는 6,090이다). 그 값으로 `MaxLifeLines`를 정한다. full 2회를 돌리고, 기준값 셋이 그대로인 것을 커밋 메시지에 적는다.
3. **[A-4b] — high, 끔**: §5, §6, `AcquireSim` 리셋, UI 테스트 넷. full 2회. 창 없이 찍은 두 장(구현 메모 §6)을 보인다.
4. **손 확인(개발자, ⓒ)**: 새 판부터 마우스만으로 ×10, T3 → 연출 → 이제 보인다 → 연대기 → 지도 보기 → 연대기 → 새 판(두 번)을 지나 본다. `CosmicEnd`의 두 초(결정 1), 쪽의 읽힘, 제목의 이름이 낯선지를 `CHANGELOG`에 한 줄로 적는다.
5. **[A-4c] — xhigh, 단독으로 쓰고 끝에 ultracode 재감사 1회**(문서 사이의 같은 문장·교차 참조): `WORKPLAN` §2 '설계 문서 변경' 행(65행)은 섹션 단위 변경에 '리뷰 → 적용 → 재감사'를 요구한다. A-4c는 `docs/20` §6.3의 새 단락, `docs/70` §8.4의 여러 줄, `OPEN_QUESTIONS`의 닫기와 새 행, `CLAUDE.md` 규칙 15를 고치므로 그 행에 든다. 리뷰는 이 헤더의 리뷰가 했고, 남는 것이 재감사다. 새 서술 문장이 없어 글 감사는 따로 하지 않는다. 2인칭(결정 14)과 시험판 안내는 다음 묶음 감사에 싣는다.

## 10. 확인하지 못한 것
- **장의 길이와 벽시계 길이**: 상한 40·등급 잣대·'20분 안팎'은 어림이다. A-4a와 손 확인이 잰다.
- **두 BeginPlay의 순서와 부름**: 순서는 엔진 소스에서 보지 않았다. `FollowEnd(false)`의 계약이 두 순서를 다 덮는다. **두 `BeginPlay`의 부름은 ④의 테스트가 지키지 않는다** — ④에는 끝난 상태로 서는 시작이 없고, ⑧은 `FollowEnd(false)`를 직접 부른다. 둘 중 하나를 지워도 깨지는 테스트가 없으므로, ⑤의 이어하기 테스트가 그 단언을 맡는다(§4.4 ⑤).
- **장 제목의 그릇 이름**: 이 쪽이 처음 보이는 자리다. 낯설게 읽히면 단계 6까지 제목에서 뺀다(빈 이름이면 '제 1대'만 남는 길이 있다).
- **스크롤 상자 끝의 휠**: 뷰포트로 넘어가는지 보지 않았다. `HandleWheel`의 문이 막는다.
- **ST_CA의 `{Civ}|hpp(이,가)`**: 같은 기제를 `Sight.Claimed`(`{Transcendent}|hpp(이,가)`, `ST_CA.csv:50`)와 `log_fallen_monarch`(`{Civ}|hpp(은,는)`, `DT_LogTemplates.json:62`)가 이미 쓴다. 받침 있는 이름과 없는 이름을 화면에서 본다.

## 11. 개발자 결정 (권장안) — 1~3이 개발자의 판단을 먼저 요한다
1. **판의 길이**('×10 기준 30~60분' vs '끝 = T3 ∨ 틱 6,000'). **권장 ①**: 30~60분을 멈춘 시간까지 센 벽시계로 읽고, 끝 조건과 노브는 그대로 둔다. `CosmicEnd` 줄로 ⓒ·ⓔ에서 잰다. 모자라면 노브가 아니라 업적 문턱(세계 해시·기준값이 바뀌는 밸런스 작업, ④ 밖)을 다시 정한다. 대안 ②: 기준 배속을 고쳐 적는다(×2면 칸 끝 중앙값이 쉬지 않고 약 37분, ×5면 약 15분). 대안 ③: 업적 문턱을 ④ 앞에서 올린다(30판을 다시 재고 `-UpdateBaseline`). 대안 ④: 끝을 시계 하나로 둔다(26판도 6,000까지 가서 ×10에서 쉬지 않고 10분이다. 2026-10-18 결정이 바뀐다).
2. **고정 줄의 자리와 임시 끝의 쪽**: 문서 순서(생애 장 → (두 책) → `Opened` → `Unopened` → `Horizon`)를 따르고, **시험판의 끝(칸·시계 둘 다)에도 고정 줄 셋을 싣는다**(`docs/70` §8.4 '모든 엔딩 동일', `crisis` §4.4 비조소 규칙). 까닭 줄은 스크롤 밖 아래에 두어 고정 첫 줄과 붙지 않는다(근거는 §6 규칙 11 점검). 대안 ㉮: ④에는 `Horizon`과 `Provisional`만 두고 `Opened`·`Unopened`는 B6 엔딩부터 싣는다(시계가 연 쪽이 '읽힘'을 말하지 않는다. 그 대신 B6에서 쪽이 바뀌고 손 확인이 엔딩의 첫 줄을 보지 못한다). 대안 ㉯: 첫 줄과 둘째 줄을 쪽 맨 위에 두고, `docs/70` §8.4의 '에필로그 앞'과 `docs/21` 1428행을 고친 뒤 `DECISIONS_LOG`에 근거를 적는다(계 20:12의 구조를 따르는 자리라 조용히 고르지 않는다).
3. **폐위는 끝이 아니다, 폐위 뒤에는 회색 줄**(2026-10-18 결정과 로드맵의 'B2의 실패 끝' 그대로). 상단 바가 폐위 틱부터 `Chronicle.Until`을 보인다. A-4a 정책 판에 6,000 전 폐위가 있고 그 간격 중앙값이 1,500틱(×10에서 2.5분)을 넘으면, 아래 대안을 권장안으로 올려 다시 묻는다. 대안: 폐위(`ReignEndReasons`의 마지막 칸이 비지 않음)를 셋째 원인으로 둔다(파생·저장 없음·같은 쪽).
4. **끝을 저장하지 않는다**: 순수 술어로 판정하고, 시간은 표현이 멈춘다. 뒤 엔딩의 기록은 `docs/21`에 한 줄만 둔다. 대안: `EndedTick`·`EndCause`를 저장하고 `RunHeadless`·`StepOnce`가 거절한다(기준값·30판이 6,000에서 끊기고, 멤버가 18 → 20이 되고, 임시 규칙이 세이브에 남는다).
5. **칸 끝 = 어느 생애든 `PeakTier ≥ T3`**: MVP에는 생애가 하나라 `Lives.Last()`와 같고, `Lives`가 덧붙기만 해서 계승(B5) 뒤에도 단조다. 같은 틱이면 칸이 이기고, #67 ① 기본값에 기댄다. 대안 ㉮: `Lives.Last()`만 보고 B5 헤더가 술어를 다시 정한다(`OPEN_QUESTIONS`에 새 행). 대안 ㉯: 다스리는 백성의 지금 `Civs[].Tier`(손으로 Tier를 올리는 UI 테스트 셋이 끝에 걸린다).
6. **노브 하나 `ProvisionalEndTick` 6,000과 상수 `EndRung`**: 노브는 시계 끝 판만 늘린다. 대안: 둘 다 C++ 상수(길이를 바꿀 때마다 빌드가 들고 `-Rules=`가 닿지 않는다).
7. **래치 넷, 테스트마다 `AcquireSim` 리셋, 개발 스위치.** 대안: `ApplySpeed`가 매번 술어를 새로 읽는다(`UI.MouseOnlyAscent`가 깨지고, UI 테스트가 앞 Sim 테스트의 상태에 기댄다).
8. **#31 ② 앞 절반 = `Admits`**(8필드만, Claim 키와 `{OtherCiv}`는 연 채). `Admits`는 #40 ⑦ 기본값보다 넓다 — `log_first_contact` S1과 모든 `Log.Calling`을 들이고, ⑤의 이관도 그것을 남긴다. A-4c가 #40 ⑦의 기본값 문장을 `Admits`로 바꾼다. 대안: 결정의 줄만 들이는 표 기반 술어(⑤의 거르기가 표 내용에 묶인다).
9. **생애 장**: 상한 40(가칭, A-4a가 28~64에서 정한다), 네 걸음, `Log.Calling`은 깎지 않는다, 범위는 `docs/21` 529행. 대안: 그릇의 백성 줄에는 상한을 두지 않는다(50~80줄이라 최소형 쪽에 길다).
10. **두 책 장은 B5로**: 주장·기록 줄을 내는 코드가 없어, 주장이 서는 판에서는 빈 꼴 "주장된 것이 없었다."가 사실이 아니다(주장의 40%가 영향권에 든다 — A3 §1.1). 대안: 빈 두 책 장과 닫는 줄을 지금 넣는다.
11. **#40 ⑦과 상한 노브 1,500은 ⑤로**(제약 다섯, §4.4). 대안: ④에서 이관 거르기와 노브까지 한다(전체 해시·`Log.CapAndChronicle`이 바뀌고, 압축 없이는 1 MB를 넘는다).
12. **흐름과 '새 판'**: 연출 → 드러냄 → 1.5초 → 베일이고, 클릭 한 번이면 다 드러나며, 단추는 다 드러난 뒤에 선다. '새 판'은 3초 안에 두 번 누르고, 시드는 늘 시계다. 대안: 드러냄 없이 '새 판' 하나만 둔다. 시드의 대안은 `-Seed=`를 새 판에도 쓰는 것이다(손 확인을 되풀이하기는 쉽지만, 테스터에게 같은 은하가 다시 선다).
13. **끝을 미리 읽히게 하는 UI**: 규칙 줄, 600주기 예고, 폐위 뒤 줄, 까닭과 시험판 안내, `Chronicle.` 접두어 검사. 모두 게임의 규칙으로 말한다. 대안: 까닭 줄만 두고, 금지어는 글 감사에 맡긴다.
14. **2인칭**: 고정 줄은 글자 그대로 쓴다. '너'(고정 첫 줄)와 '당신'(`log_first_life`·`log_fallen_monarch`)이 갈리는 것은 다음 묶음 감사에 올린다. 대안: 고정 첫 줄을 지금 '당신'으로 바꾸고, `docs/70` §8.4와 `crisis` §4.4를 고친다.
15. **이름을 `Provisional` 하나로**: `ProvisionalEndTick`, `-ProvisionalEnd=0`, `SetProvisionalEndEnabled`, 헤더 파일 `A4_provisional_end.md`. 노브는 칸 이름이라 세이브에 남지 않는다. 대안: 초안의 `Temporary` 계열(`TemporaryEndTick`, `-TemporaryEnd=0`)을 노브와 스위치에 둔다.
16. **태그 리터럴**: `CosmicChronicle.cpp`에 static 셋을 두고 기준을 38 → 41로 올린다(MVP ②·③의 꼴). 대안: 린트가 이미 면제하는 `Core/CosmicTags.h`를 세워 세 태그를 두고, `CosmicLogSubsystem.cpp:107-109, 181-182`도 그것을 읽게 한다(기준은 내려가지만 범위가 넓어진다).
