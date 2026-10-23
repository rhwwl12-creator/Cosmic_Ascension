# MVP ⑤ 헤더 초안 — 저장 최소형(검사하는 슬롯과 압축·백업 1·틱 경계 자동 저장·이어하기·연대기 이관과 상한)

상태: **승인 (2026-10-23, 개발자 "다음 진행해줘" — §12의 결정 18개 전원 권장안) — 구현 끝(2026-10-23, `[A-5a]`·`[A-5b]`·`[A-5c]`·`[A-5r]`·`[A-5d]` — `git log --grep 'A-5'`; 헤더와 다르게 하거나 더한 것은 `DECISIONS_LOG` 2026-10-23). 구현 리뷰(2026-10-23, 서로 독립인 리뷰 에이전트 넷 — ultracode 꺼짐) 발견 15건(높음 1 — 창 닫기가 종료 저장에 닿지 않았다): 코드·테스트로 반영 14(두 리뷰가 같은 것 하나를 따로 찾았다; 그 하나는 일부만 — 끝의 쓰기 자체가 실패한 뒤 실제 시간으로 다시 쓰기는 받지 않았다), 문서(A-5d)로 1 — 기록은 `docs/CHANGELOG.md`. 승인 전의 기록: 세 설계안(안전 먼저·플레이어 관점·결정론과 데이터 먼저)을 주제마다 견주어 고르고 서로의 좋은 생각을 접붙인 종합이다. 적대 리뷰(2026-10-23, ultracode — 다섯 관점) 발견 74건: 반영 68(둘은 일부 기각), 개발자 결정으로 올림 6, 받지 않은 물음 1. 독립 검증 뒤 남은 17건 반영(검증의 물음 하나는 결정 18로 올림).** 새 저장 필드 **1**(최상위 `ChronicleEviction` — 최상위 멤버 18 → 19), 새 USTRUCT **1**(`FChronicleEviction`, 멤버 둘), 새 UENUM **0**, 틱 단계 변경 **0**, 추첨 지점 **0**이다. `SaveVersion`은 1 그대로다. 그 밖에 드는 것은 새 파일 `Save/CosmicSaveSlot.h/.cpp`(바이트의 순수 함수), 저장 상태가 아닌 평범한 열거 넷(`ECosmicSlotRead`·`ECosmicContinue`·`ECosmicSaveCause`, 그리고 슬롯 머리의 1바이트라 끝에만 더하는 `CosmicSaveSlot::ECodec`), 기존 열거 `ECosmicAutoPause`의 끝 열거자 넷, DT_Rules 노브 하나(`ChronicleCap` 1,500), ST_CA 새 키 넷과 고친 키 하나, 자동화 테스트 106 → **114종**이다. 세계 해시는 모든 기준값에서 그대로이고, 전체 해시(상태 해시)는 A-5b에서 바뀐다. 이 헤더가 승인할 것은 아래의 C++ 선언, 슬롯 머리의 바이트 꼴, ST_CA 키와 글, §12의 결정이다. 긴 본문·private 멤버·테스트의 값과 변이 목록·코드와 엔진 사실의 전체 표·고칠 문서의 목록은 **구현 메모** `docs/60_DECISIONS/headers/A5_save_minimum_notes.md`에 있다. 코드 사실은 HEAD `e71751b`의 작업 트리에서 파일을 열어 확인했고, 엔진 호출은 모두 `C:\Program Files\Epic Games\UE_5.8\Engine\Source`의 선언과 본문을 열어 확인했다. **먼저 볼 결정은 1(이어하기의 꼴)·2(흐르던 판을 버리는 길 — 리뷰 뒤 권장을 바꿨다)·3(백업과 쓰는 차례, 전원이 꺼지는 경우 — 리뷰 뒤 권장을 바꿨다)이다.**

근거: `docs/50_ROADMAP.md` A ⑤·종료 조건 ⓐ·ⓓ, `OPEN_QUESTIONS` #33 ①②③·#40 ⑦·#70·#9·#117, 헤더 `A4_provisional_end.md` §4.4·§5.1·§10·결정 4·11, `docs/20` §6.3(77행)·§9.1(172~181행)·§9.2(195·230~232행)·§9.4(242·249·251행)·부록(290·293·295·300행), `docs/21` 477·479·527·529·530·1204·1206·1215·1271·1431~1432행, `docs/70` §8.4(161·163·167행), `crisis_and_transcendents.md` §4.4(138·146·150·210행), `WORKPLAN.md` §2(59·65·69·73행), `CLAUDE.md` 불변 규칙 1·2·3·4·5·6·7·8·10·11.

## 0. 범위와 전제

**⑤가 하는 일 여섯**
1. **검사하는 슬롯**(#70): 엔진이 만든 세이브 바이트(`SaveGameToMemory`)를 Oodle로 압축하고, 그 앞에 64바이트 머리(첫 낱말·꼴·코덱·GalaxyID·쓴 시각·틱·두 길이·CRC32 둘)를 붙여 슬롯 파일로 쓴다. 읽을 때는 머리와 CRC를 먼저 보고, 통과한 바이트만 엔진의 객체 리더(`LoadGameFromMemory`)에 넘긴다.
2. **백업 1**(#33 ②): 덮어쓰기 전에 **검사를 통과한** 주 슬롯을 `Galaxy_<GUID>_bak`으로 옮겨 쓰고 디스크까지 내린 뒤에 주 슬롯을 쓴다(결정 3). 로드는 주 슬롯이 상했을 때만 백업으로 간다.
3. **틱 경계 자동 저장**(#33 ①): 저장 Subsystem이 `OnSimTick`(열째 단계와 `SyncRngState` 뒤, Core Logic 방송의 괄호 밖)에서 '디스크에 든 것'의 사본(**디스크 사본**, §4.5)과 지금의 상태를 견주어 승천·역할 전환·임시 끝·10분마다를 잡는다. 입력 자리 셋(받아들인 청원의 답, 새 판, 종료)은 컨트롤러가 부탁한다.
4. **이어하기**(#33 ③): `ACosmicGameMode::StartPlay`에서 `Super::StartPlay()` **앞**에 가장 새로 쓰인 판을 연다. 끝난 판은 두 `BeginPlay`의 래치(한 번 서면 그 판 동안 남는 끝의 기억 — `docs/20` §6.3)가 끝난 채 세운다(A4 §4.4 ⑤). 흐르던 판은 멈춘 채 까닭 한 줄과 함께 열린다.
5. **연대기 이관 거르기와 상한**(#40 ⑦): 로그가 넘칠 때 떠나는 줄 가운데 `CosmicChronicle::Admits`가 **모든 생애의 백성**으로 받는 줄만 `Chronicle`로 옮긴다. 상한 노브 `ChronicleCap` = 1,500을 넘으면 오래된 Severity 1만 밀어내고, 밀어낸 것을 새 저장 필드(**밀어냄 기록**)에 적는다.
6. **표현**: 상단 바의 정지 까닭 줄 넷(이어 엶·백업에서 엶·읽지 못함·저장 실패)과 `Chronicle.NewRunTip`의 고침. 상단 바의 '새 판' 단추는 결정 2의 대안이다.

**그대로인 것**
- `SaveVersion` 1. `UCosmicSaveGame { UPROPERTY() FGalaxyState State; }`의 꼴. 슬롯 이름 `Galaxy_<GUID 32자리>`, 판당 슬롯 1(아이언맨), 수동 저장·로드 메뉴 없음(`docs/20` §9.1, `docs/50` 컷 리스트).
- 임시 끝은 저장하지 않는다(A4 결정 4). 로드가 `Tick`과 모든 생애의 `PeakTier`에서 같은 답을 다시 낸다.
- 틱 열 단계, 추첨 지점, 난수 출처 셋. 저장·이어하기·거르기는 아무것도 뽑지 않는다(규칙 3).
- **세계 해시 셋**: 시드 42 3,000틱 `1078735008`, 정책 줄 `4271872285`, 20,000틱 `885018087`. `run_checks.ps1`이 커밋마다 견주는 것은 3,000틱의 두 줄뿐이고, 그 길이에서는 로그가 넘치지 않아(6,000틱에도 834줄) 거르기와 상한이 돌지 않는다. 그래서 **거르기가 세계에 닿지 않는다는 증명은 20,000틱의 세계 해시**(A-5b 뒤의 손 측정 — 구현 메모 §7)와 `Sim.KnowingInert`이고, **상한과 밀어냄 기록의 증명은 `Log.ChronicleCap`의 '상한 0과 10의 세계 해시가 같다'**다. 3,000틱의 두 줄이 지키는 것은 '`NotWorldNames`에 새 이름을 잊으면 곧바로 어긋난다'(0인 멤버도 해시에 든다)이다(§2, §10).
- `MaxLogEntries` 2,000, `MaxLifeLines` 40, `Admits`의 본문, v0.7 예약 멤버 25, 래치 넷(`docs/20` §6.3), 연대기 쪽의 꼴.

**하지 않는 것**: 관조 모드(#9), 비동기 쓰기, 임시 파일 → 바꿔 넣기(§3.4에서 기각), 사망·공백기·계승의 저장 지점(Phase 4b — 그 헤더의 `BeginInterlude`), 엔딩의 저장 기록(B6, #117), 패키지 게임의 저장 폴더 확인(⑨), 판 간 메타 세이브(`crisis` 150행), 상단 바의 '새 판'(결정 2).

**커밋**: 단계 0 → **A-5a**(슬롯의 꼴·백업·찾기·테스트 폴더·`MayLetGo`·커맨드렛 측정 — 두 해시 그대로) → **A-5b**(이관 거르기·상한·밀어냄 기록·노브 — 전체 해시 기준값 갱신, 세계 해시 그대로) → **A-5c**(자동 저장·이어하기·화면 — 두 해시 그대로) → 구현 리뷰(결정 17) → 손 확인 ⓐ → **A-5d**(남은 문서·STATUS). 저장 필드와 슬롯의 꼴을 바꾸는 커밋은 같은 커밋에서 `docs/20`을 먼저 고친다(CLAUDE.md 작업 방식 — A-5a는 §9.1의 쓰는 함수 문장, A-5b는 §9.1의 새 필드·§9.2의 제외·`docs/21`·CLAUDE.md 규칙 6. 목록은 구현 메모 §9).

## 1. 기대는 사실과 측정 (코드 자리의 전체 표는 구현 메모 §0, 엔진의 전체 표는 §0.1)

### 1.1 코드
| 사실 | 자리 |
|---|---|
| 게임 코드는 저장 함수를 한 번도 부르지 않는다. 부르는 곳은 테스트와 커맨드렛뿐이다 | `Save/CosmicSaveSubsystem.h:28-42`, `Tests/*`, `Tests/CosmicBalanceCommandlet.cpp:287-295` |
| `SaveCurrentGalaxy`는 `SyncRngState` → 운반 객체 복사 → `SaveGameToSlot` 한 번이다. 방송·틱 단계 거절이 없다 | `Save/CosmicSaveSubsystem.cpp:36-54` |
| `LoadGalaxy`: 방송 거절 → `DoesSaveGameExist ? LoadGameFromSlot` → GalaxyID → 버전 → 성역 → `LoadState` | `:56-108` |
| `StartPlay`: `GalaxyID`가 없을 때만 `NewGalaxy(Seed, 0, 0)`과 `SetTimeScale(-Speed=)`, 그 뒤 `Super::StartPlay()` | `World/CosmicGameMode.cpp:96-117` |
| 두 `BeginPlay`가 각각 `FollowEnd(false)`를 부른다. HUD는 카드를 만든 직후 `FollowPetition(false)`를, 연대기 쪽을 만든 뒤 `FollowEnd(false)`를 부른다 | 컨트롤러 `:552`, HUD `:264`·`:274-279`·`:283` |
| `LoadState`는 방송 문이 열려 있으면 `OnGalaxyGenerated`를 방송하고 `TimeScale`은 건드리지 않는다 | `Sim/GalaxySimulationSubsystem.cpp:1274-1293` |
| `SimulateTick`의 꼬리: `ResolvePendingRoleEffect` → `SyncRngState` → `OnSimTick.Broadcast`(Begin/EndBroadcast 밖). `OnSimTick`은 동적 델리게이트다 | `.cpp:1386-1394`, `.h:18, 308` |
| `bInTickStages`는 private(구역은 `.h:332`부터)이고 읽는 접근자가 없다. `IsBroadcasting`은 public이다. 방송 문은 `SetBroadcastEnabled`로 여닫는다 | `.h:502`, `.h:261`, `.h:304-305` |
| `HashState`는 최상위 속성을 반사로 돌며 이름으로만 거른다(이름·순번은 해시에 들지 않는다. 0인 멤버도 든다) | `.cpp:1305-1349` |
| 이관 고리: 넘치면 머리(가장 오래된 줄)부터 떠나고, `Severity >= 1 ∨ Claim ∨ Record`를 연대기로 옮긴다. 연대기에는 상한이 없다 | `Log/CosmicLogSubsystem.cpp:206-216` |
| `MaxLogEntries`는 `static constexpr`다(테스트가 바꾸지 못한다) | `Log/CosmicLogSubsystem.h:61` |
| `BuildCapped`는 `Admits`가 받지 않는 줄을 되풀이 규칙보다 **먼저** 건너뛴다 | `Log/CosmicChronicle.cpp:145` |
| 상단 바는 `DisableNativeTick`이고 `NativeTick`이 없다(시간으로 라벨을 되돌릴 자리가 없다) | `UI/CosmicTopBarWidget.h:35` |
| 테스트·스모크는 `-game -nullrhi -unattended`로 두 프로세스에서 돌고 `-Seed=`가 없다. smoke 거름은 `Cosmic_Ascension.Sim.Save`를 든다 | `Tools/run_checks.ps1:58-70` |
| 산 Sim을 쓰는 테스트는 모두 `CosmicTests::AcquireSim`을 지난다 | `Tests/CosmicTestHelpers.h:72-93` |
| 레이어 린트는 Core Logic(Save 포함)의 `Engine/World.h`를 막는다 | `Tools/check_layers.py:20-21` |

### 1.2 엔진에서 이 설계가 기대는 점 (UE 5.8 — 선언·본문의 자리는 구현 메모 §0.1)
- **엔진 세이브 바이트에는 검사가 없다**: `LoadGameFromMemory`는 길이·체크섬을 보지 않고, 속성 안에서 잘린 바이트는 `checkf(Tag.Size == Loaded)`(`CoreUObject/Private/UObject/Class.cpp:1927`)에 닿는다 → 우리 머리가 먼저다.
- **파일 쓰기**: `FWindowsPlatformFile::OpenWrite`는 최종 경로를 `CREATE_ALWAYS`로 바로 자르고(임시 파일 없음), `IFileManager::Move`는 대상을 먼저 지운다(원자적이지 않다). 엔진 세이브의 쓰기 길(`SaveDataToSlot` → `FFileHelper::SaveArrayToFile`)은 닫을 때 `FlushFileBuffers`를 부르지 않는다. 쓰기 손잡이의 `IFileHandle::Flush`는 Win64에서 `FlushFileBuffers`다(`WindowsPlatformFile.cpp:933-936`).
- **읽기**: `FFileHelper::LoadFileToArray`는 파일을 통째로 읽고, `FILEREAD_Silent`가 없으면 열지 못한 파일에 `LogStreaming` Warning을 남긴다(`FileHelper.cpp:42-72`).
- **저장 폴더**: 일반 저장 시스템의 경로는 `ProjectSavedDir()/SaveGames/<슬롯>.sav`이고 목록은 그 폴더의 `*.sav`만 본다(하위 폴더 아님 — `SaveGameSystem.h:137-149, 169-172`). 슬롯 이름에 `Automation/`을 붙이면 하위 폴더가 된다.
- **Oodle**: 압축기는 스트림에 기록되고 수준은 기록되지 않는다(`OodleDataCompression.h:175`). 엔진의 대표 인코드 속도(`:78-88`): Kraken VeryFast(2) 2.32:1·157.4 MB/s, Fast(3) 2.39:1·34.8 MB/s, Normal(4) 2.44:1·22.4 MB/s. 깃발 없는 `FCompression::CompressMemory(NAME_Oodle)`는 Kraken·Fast를 고른다(`OodleDataCompressionFormat.cpp:274-290`. 명령줄 설정은 `COMPRESS_ForPackaging` 길에만 든다 — 같은 파일 `:218`).
- **종료 차례**: `World->EndPlay(Quit)`(액터마다)가 `GameInstance->Shutdown()`보다 먼저다(`GameEngine.cpp:1373 → 1377`). Subsystem `Deinitialize`의 차례는 보장이 없다.
- **맵 다시 열기**: `UEngine::LoadMap`은 로컬 플레이어의 컨트롤러를 `DestroyActor`로 먼저 없앤다(`UnrealEngine.cpp:16249` → `Actor.cpp:3313`, 까닭 `Destroyed`). 월드의 `EndPlay(LevelTransition)`(`:16259`)은 그 뒤다. 어느 쪽도 `Quit`이 아니라 종료 저장이 일지 않는다. 새 맵에서는 컨트롤러(Login)와 HUD(PostLogin → `ClientSetHUD`)가 `World->BeginPlay`(`:16661`)보다 먼저 서고, `NotifyBeginPlay`가 레벨의 액터 차례로 돈다(`WorldSettings.cpp:370-375`) → **컨트롤러의 `BeginPlay`가 먼저, HUD가 뒤**다.
- **테스트의 맵 열기**: `FLoadGameMapCommand`는 `Open <맵>`을 부를 뿐이고 이동은 다음 엔진 틱에 일어난다(`UnrealEngine.cpp:15410`). 지연 명령 고리는 끝난 명령 뒤의 명령을 같은 프레임에 돌린다(`AutomationTest.cpp:684-707`). 그래서 `FWaitForMapToLoadCommand`(`AutomationCommon.cpp:863-877` — `HasMatchStarted`만 본다)는 **옛 월드를 보고 곧바로 끝날 수 있다** → 테스트는 새 월드를 기다리는 제 지연 명령을 쓴다(§8).
- **문의 재료**: `GIsEditor`는 에디터 프로세스(에디터 안 플레이 포함)에서 참, `-game` 프로세스(에디터의 'Standalone Game'도 `-game`으로 뜬다)에서 거짓이다(`LaunchEngineLoop.cpp:2269-2273, 2296-2300`). `GIsAutomationTesting`은 테스트마다 `StartTestByName`부터 `StopTest`까지만 참이다(`AutomationTest.cpp:651, 673`). `FApp::IsUnattended()`는 `-unattended` 또는 `GIsAutomationTesting`이다(`App.cpp:274-280`). `UWorld::IsPlayInEditor`는 쓰지 않는다 — `Engine/World.h`를 레이어 린트가 막는다.
- **GUID 이름**: `FGuid::ParseExact(Digits)`는 32자가 모두 `FChar::IsHexDigit`인지 본다(소문자도 받는다 — `Guid.cpp:621`).

### 1.3 측정
| 출처 | 틱 | SaveBytes(압축 전) | Log | Chronicle |
|---|---|---|---|---|
| `Saved/Balance/a3_20k.csv`(시드 42, 전부 말하기, 입력 없음) | 6,000 | 632,651 | 834 | 0 |
| 같음 | 10,000 | 882,097 | 1,250 | 0 |
| 같음 | 20,000 | 1,380,703 | 2,000 | 85 |
| `checks_ruler.csv`(`-TellAll=0 -Answer=1 -Build=1`) | 3,000 | 310,722 | 294 | 0 |

- 로그 항목 하나는 약 600 B다(10,000 → 20,000틱의 차로 약 597 B, `docs/20:249`와 맞는다). 20,000틱의 세계 몫은 약 0.13 MB다.
- **상한까지 찬 판**(VS, 지울 수 없는 줄이 1,500 아래일 때): `Log` 2,000 × 600 B + 연대기 1,500 × 600 B + 세계 ≈ **2.2 MB(압축 전)**. 지울 수 없는 줄(S2·`Log.Calling`·`Log.Claim`·`Log.Record`)은 상한을 넘어 자라므로(§6.3) 참 최악은 그 수가 정한다. 거르기 전의 옛 EA 측정(전부 쌓기, 20,000틱)은 4.2 MB·연대기 3,922줄이었다(`docs/20:249, 251`). 지울 수 없는 줄의 수는 거르기 전과 뒤가 같다(`Admits`가 그 넷을 모두 받는다). 그래서 A-5a가 EA 20,000틱에서 `kept for good`과 슬롯 바이트를 재면 예산의 답이 선다. 예산은 ≤ 1 MB / ≤ 100 ms(`docs/20:249`)라 압축이 함께 든다(A4 §4.4 ④).
- **압축의 시간**: 엔진의 대표 속도로 2.2 MB는 Kraken·Fast 약 63 ms, Normal 약 98 ms, VeryFast 약 14 ms다. 복사와 직렬화(20~62 ms, `docs/20:249`), 쓰기 둘과 내리기 둘이 같은 100 ms 안에 들어야 하므로 VeryFast로 시작한다(결정 10).
- **게임 길이의 판(≤ 6,000틱)에서는 이관이 일어나지 않는다.** 그래서 MVP 플레이어는 거르기·상한·밀어냄 기록을 보지 않는다. 압축 없이도 6,000틱 세이브는 1 MB 안이다.
- **재지 않은 것**: 압축 뒤 크기와 압축·쓰기·내리기 시간, 보통 모드(`-TellAll=0`) 20,000틱의 크기, EA 20,000틱의 지울 수 없는 줄 수. 모두 A-5a가 잰다(§10).

## 2. 상태 — 새 저장 필드 하나

`Core/GalaxyState.h`, `FGalaxyState` 앞에 구조체를 두고 `Chronicle`(771행) 바로 뒤에 멤버를 둔다.

```cpp
/**
 * What the chronicle's cap has let go (MVP item 5; docs/70 section 8.4; 16 #40 (7)). When FGalaxyState.Chronicle holds more
 * than DT_Rules.ChronicleCap entries after a move out of the log, its oldest Severity 1 entries leave it - never a Severity 2,
 * Log.Calling, Log.Claim or Log.Record entry (Law.Judgment). UCosmicLogSubsystem is the one writer; CosmicChronicle::BuildCapped
 * reads it: a chapter whose range reaches ThroughTick closes with ST_CA Chronicle.More (no count).
 * Telling, not world: saved and part of ComputeStateHash, left out of ComputeWorldHash by its top-level member name.
 * Both initializers are frozen (0 = nothing let go): a run whose chronicle never reached the cap writes nothing here, and a
 * save from before the cap reads as "nothing let go", which is what happened.
 */
USTRUCT(BlueprintType)
struct COSMIC_ASCENSION_API FChronicleEviction
{
	GENERATED_BODY()

	// Entries let go in this run; only grows. 0 (frozen) = none - the only test of "anything let go".
	UPROPERTY()
	int32 Count = 0;

	// Tick of the newest entry let go; read only when Count > 0. Only grows: the chronicle is in time order and its oldest
	// Severity 1 entries go first.
	UPROPERTY()
	int64 ThroughTick = 0;
};

	// (FGalaxyState, right after Chronicle)
	// What the cap of Chronicle let go (MVP item 5). Top level on purpose: ComputeWorldHash leaves it out by name.
	UPROPERTY()
	FChronicleEviction ChronicleEviction;
```

**델타 저장(기본값과 같은 값은 쓰지 않는 저장)에 안전한 까닭**
- `FGalaxyState`에 직접 든 구조체라, 두 멤버가 초기값이면 쓰이지 않고 로드하는 빌드의 초기값으로 읽힌다(`docs/20:174`, `GalaxyState.h:8-10`).
- ⑤ 전의 세이브에는 상한이 없었으므로 '0 = 밀어낸 것 없음'이 사실이다. 두 초기값은 첫 세이브부터 동결된다(규칙 7, `docs/21:479`).
- **'없음'을 `Count`가 맡는 까닭**: 틱 0에도 Severity 1 줄이 생길 수 있어(멈춘 틱 0의 입력) `ThroughTick == 0`만으로는 '없음'과 '틱 0의 줄을 밀어냄'을 가르지 못한다.
- **`SaveVersion` 1 그대로**: 기본값이 '없음'인 추가이고, `Chronicle` 원소의 뜻('로그에서 나온, 연대기가 보일 수 있는 줄')은 바뀌지 않는다. 슬롯 꼴의 바뀜은 머리의 `Form`이 맡는다. `SaveVersion`은 상태의 뜻이고 슬롯의 꼴과 섞지 않는다.

**해시**
- 전체 해시 안, 세계 해시 밖이다. `HashState`의 `NotWorldNames`(`Sim/GalaxySimulationSubsystem.cpp:1310-1315`)에 `GET_MEMBER_NAME_CHECKED(FGalaxyState, ChronicleEviction)`를 더한다. 말하기 범위에 따라 연대기 길이가 달라지므로, 세계 해시에 들면 '앎이 세계를 바꾸지 않는다'를 재는 자가 깨진다.
- 이 줄을 잊으면 3,000틱 기준값의 세계 해시가 곧바로 어긋난다(0인 멤버도 해시에 든다). 기준값 비교가 그대로 변이 검사 하나가 된다.
- 반사 해시는 0인 멤버도 더하므로 **모든 상태의 전체 해시가 바뀐다.** 받아들이고 `-UpdateBaseline`으로 고친다. 대안('Count가 0이면 해시에서 건너뛴다')은 반사로 도는 해시에 영원한 예외를 남겨 버린다.
- `GalaxyState.h` 머리 주석(19행의 제외 목록), `docs/20:195`, `CLAUDE.md` 규칙 6에 이름을 더한다 — A-5b가 같은 커밋에서 코드보다 먼저 고친다.

**셈**: 최상위 멤버 18 → **19**, v0.7 예약 25 그대로. `FillEmptyContainers`가 밀어냄 기록을 `{3, 17}`로 둔다(0인 채로 지우면 해시가 안 움직여 분류 테스트가 거짓으로 깨진다 — `docs/21:479`).

**상태가 아닌 것**: 슬롯 머리의 시각·틱은 파일의 바이트다. 저장 Subsystem의 디스크 사본(§4.5)은 디스크의 사본이다. 이어하기의 결과는 프로세스 수명의 기록이다.

## 3. 슬롯의 꼴 (`Save/CosmicSaveSlot.h/.cpp` — 새 파일, 바이트의 순수 함수)

### 3.1 머리 (64바이트, 리틀 엔디언 — Win64, `FMemoryWriter`로 필드 하나씩 이 차례로)
| 자리 | 크기 | 필드 | 값·검사 | 봉투 |
|---|---|---|---|---|
| 0 | 4 | `Magic`(첫 낱말) | `0x56534143`(파일 차례로 'C' 'A' 'S' 'V'). 엔진 꼴('GVAS')은 `OldForm`, 그 밖은 `NotOurs` | 고정 |
| 4 | 2 | `Form` | 1. 0은 `BadSizes`, 이 빌드보다 높으면 `NewerForm`(거절 — 결정 7) | 고정 |
| 6 | 1 | `Codec` | 0 = 압축 안 함(Stored), 1 = Oodle. 끝에만 더한다 | — |
| 7 | 1 | (비움) | 0으로 쓰고 읽을 때 보지 않는다. 쓴 까닭은 로그 줄이 말한다 | — |
| 8 | 16 | `GalaxyID` | 슬롯 이름의 GUID와 같아야 하고, 풀어 낸 `State.GalaxyID`와도 같아야 한다 | 고정 |
| 24 | 8 | `SavedAtUtcTicks` | `FDateTime::UtcNow().GetTicks()`(같은 프로세스 안에서는 앞의 쓰기보다 늘 크게). 이어하기의 차례에만 쓴다. 상태도 해시도 아니다 | 고정 |
| 32 | 8 | `Tick` | 페이로드의 `State.Tick`(로그 줄·이어하기 줄·테스트) | — |
| 40 | 8 | `RawBytes` | `SaveGameToMemory` 출력의 길이. 1 이상 64 MB 이하 | — |
| 48 | 8 | `PackedBytes` | 머리 뒤 바이트 수. 파일 길이 = 64 + `PackedBytes`(정확히) | — |
| 56 | 4 | `PayloadCrc` | 페이로드 전체의 `FCrc::MemCrc32` | — |
| 60 | 4 | `HeaderCrc` | 앞 60바이트의 `FCrc::MemCrc32` | 고정 |

**봉투**(모든 꼴이 옮기지 않는 자리): 첫 낱말(0)·`Form`(4)·`GalaxyID`(8)·`SavedAtUtcTicks`(24)·`HeaderCrc`(60, 앞 60바이트). 뒤의 꼴은 6·7·32~59바이트의 뜻과 페이로드를 바꿀 수 있어도 이 다섯은 옮기지 않는다. 그래서 이 빌드도 새 꼴의 판을 알아보고(이어하기의 차례에 든다) 상함으로 잘못 읽지 않고 거절한다(결정 7).

### 3.2 압축
- `FOodleDataCompression::Compress(…, ECompressor::Kraken, ECompressionLevel::VeryFast)`를 **직접** 부른다(결정 10). 낡은 깃발 표(엔진 주석이 '새 코드는 Oodle을 바로 부르라'고 한다 — `OodleDataCompression.h:124-131`)를 지나지 않고, 쿠킹·명령줄 설정이 수준을 바꾸지 못한다. 버퍼는 `CompressedBufferSizeNeeded`(Kraken 기준, `OodleDataCompression.cpp:227-231`)만큼 잡는다.
- 압축기가 0을 내면 `Codec = Stored`로 원래 바이트를 그대로 싣는다. 저장이 압축 때문에 실패하지 않는다.
- 수준은 스트림에 기록되지 않으므로, 측정 뒤 수준을 바꿔도 옛 슬롯은 그대로 풀린다.
- 압축 결과의 바이트 결정론은 필요 없다(바이트는 해시에 들지 않는다).

### 3.3 읽기의 검사 차례 (첫 실패가 까닭이다)
0. (읽기 전, Subsystem) 파일이 없다 → `Missing` / 있는데 두 번 읽어도 읽히지 않는다(잠금·입출력 오류) → `ReadFailed`(상함이 아니다 — 결정 5) / 슬롯이 될 수 없을 만큼 크다(`64 + CompressedBufferSizeNeeded(MaxRawBytes)` 초과) → `BadSizes`(읽지 않는다)
1. 길이 < 64 → `Short`
2. 첫 낱말이 'GVAS' → `OldForm`(⑤ 전의 모든 세이브 — #70의 '지금의 개발 세이브는 읽지 않는다', `Saved/SaveGames/`는 지금 비어 있다) / 그 밖 → `NotOurs`
3. 머리 CRC가 다르다 → `HeaderDamaged`. 봉투의 자리라 꼴과 상관없이 **꼴보다 먼저** 본다. 그 뒤로만 다른 필드를 믿는다. `Form` 바이트의 비트 뒤집힘도 여기서 상함으로 잡힌다.
4. `Form` > `FormVersion` → `NewerForm`(새 빌드가 쓴 온전한 슬롯 — 거절이고 백업으로 가지 않는다)
5. `Form` 0, 모르는 코덱, `RawBytes` 범위 밖, `PackedBytes` ≤ 0, Stored인데 두 길이가 다름 → `BadSizes`
6. 파일 길이 ≠ 64 + `PackedBytes` → `Truncated`(잘렸거나 덧붙음)
7. 머리의 GalaxyID ≠ 기대한 이름 → `OtherGalaxy`
8. 페이로드 CRC가 다르다 → `PayloadDamaged`
9. 풀기가 정확히 `RawBytes`를 내지 못한다 → `CodecRefused`
10. `LoadGameFromMemory`가 `UCosmicSaveGame`을 내지 않는다 → `Unreadable`
11. (Subsystem에서) `State.GalaxyID` ≠ 이름 → `OtherGalaxy`, `SaveVersion` > `CurrentSaveVersion` → `NewerSave`, 마지막 성계가 지킨 성역이 아님 → `Sanctuary`(지금의 Error 줄 그대로)

엔진의 태그 속성 리더(`CoreUObject/Private/UObject/Class.cpp:1927`의 `checkf(Tag.Size == Loaded)`)에는 **길이와 CRC를 통과한 바이트만** 닿는다. #70의 '속성 안에서 잘린 파일이 체크 빌드를 죽인다'가 여기서 닫힌다. (우리가 쓰지 않은 바이트에 CRC를 맞춰 붙인 슬롯은 막지 않는다 — 막을 일이 아니다.)

### 3.4 백업과 쓰는 차례 (`SaveCurrentGalaxy`와 자동 저장이 함께 쓰는 한 길)
1. **거절**: `IsBroadcasting()` → Error 한 줄(규칙 5), `IsInTickStages()` → Error 한 줄(새 접근자), `GalaxyID` 없음 → Warning(지금 그대로). false.
2. `SyncRngState` → 운반 객체(`NewObject<UCosmicSaveGame>`)에 상태 복사 → `CosmicSaveSlot::Encode`.
3. **백업**: 주 파일이 있으면 읽고 `Check(Old, 이 판의 ID)`.
   - `Ok` → `_bak`에 쓰고 내린다. **이 쓰기가 실패하면 주 슬롯을 덮지 않고** Warning과 함께 false.
   - `ReadFailed`(읽히지 않음)·`NewerForm`(새 빌드의 슬롯) → **주를 덮지 않는다**. Warning, false. 읽지 못한 것과 이 빌드가 모르는 것은 덮지 않는다(결정 5·7).
   - 그 밖(상함) → 옮기지 않는다(Warning 한 줄). 백업이 마지막의 좋은 세이브로 남는다.
4. 주에 쓰고 내린다. 실패하면 Warning, false, 실패를 적는다(§4.4 — 게임의 길이면 까닭이 남아 다시 쓴다, 결정 12).
5. 디스크 사본을 새로 둔다(§4.5). Display 한 줄: `CosmicSave: wrote <slot> (<cause>) tick <n>, hash <h>, <packed> of <raw> bytes in <ms> ms`. 전체 해시를 찍는 값(약 4 ms, `docs/20:251`)은 손 확인 ⓐ를 '저장 줄의 해시 = 이어하기 줄의 해시'로 정확하게 한다. 게임이 해시를 셈하는 것은 쓰기·로드·종료 비교 때 한 번씩이고 틱마다가 아니다(경계의 디스크 사본 비교는 해시를 셈하지 않는다).
6. **쓴 뒤 다시 읽어 비교하지 않는다**(결정 3).

**한 번의 쓰기**(결정 3): `IPlatformFile::OpenWrite(경로)` → `Write` → `Flush(true)`(Win64 `FlushFileBuffers`) → 닫기. 경로는 `SlotPathOf` = `ProjectSavedDir()/SaveGames/<폴더><이름>.sav` — 일반 저장 시스템과 같은 계산이다(Win64 전용, ⑨가 다시 본다). 폴더가 없으면 `CreateDirectoryTree`를 먼저 부른다. 열기(읽기 전용 파일 포함)·쓰기·내리기의 실패는 모두 쓰기 실패다. 읽기는 `FFileHelper::LoadFileToArray(…, FILEREAD_Silent)`, 있음은 `IFileManager::FileSize ≥ 0`, 지움은 파일이 있을 때만 `IFileManager::Delete(경로, true, false, true)`다(`RequireExists`·`EvenReadOnly` 거짓·`Quiet` — 엔진의 `DeleteGameInSlot`이 지나는 `FGenericSaveGameSystem::DeleteGame`과 같은 인자, `SaveGameSystem.h:163`). 기본 인자의 `Delete`는 없는 파일에도 참을 내고, 읽기 전용 파일을 지우지 못하면 Warning을 남긴다(`FileManagerGeneric.cpp:255-291`) — `DeleteSave`의 '하나라도 지웠으면 참'이 거짓이 된다. 엔진의 슬롯 함수(`SaveDataToSlot` 등)는 저장 층이 부르지 않는다 — 테스트가 '옛 꼴' 슬롯을 만들 때만 쓴다.

**어느 순간에나 좋은 사본이 하나 있다**
| 순간 | 주 | 백업 | 좋은 사본 |
|---|---|---|---|
| 3의 쓰기 중 | 옛 M1(좋음) | 쓰는 중 | 주 |
| 3이 실패함 | M1(덮지 않음) | 상했을 수 있음 | 주 |
| 4의 쓰기 중 | 쓰는 중 | M1(디스크에 내려짐) | 백업 |
| 4 바로 뒤 전원이 꺼짐 | 새것(내려짐) | M1(내려짐) | 둘 다 |
| 주가 상한 채 저장 | 쓰는 중 | 그 앞의 B0(옮기지 않음) | 백업 |
| 첫 저장 | 쓰는 중 | 없음 | 메모리의 상태뿐(그 전에도 없었다) |

**임시 슬롯 → 바꿔 넣기는 기각한다**: `ISaveGameSystem`에는 이름 바꾸기가 없다. `IFileManager::Move`는 대상을 먼저 지우고 바꿔 넣기 깃발 없는 `MoveFileW`를 부른다 — 원자적이지 않다. 진짜 원자적 교체(`ReplaceFileW`)는 플랫폼 헤더가 든다. 위 차례가 같은 보장('좋은 사본 하나')을 엔진의 파일 API만으로 준다.

**덮는 것과 덮지 못하는 것**: 프로세스가 죽는 경우(작업 관리자)와 전원이 꺼지는 경우를 덮는다 — 쓰기마다 디스크까지 내리므로 4가 시작될 때 백업은 이미 디스크에 있다. 덮지 못하는 것은 디스크의 고장과, 밖의 도구가 두 파일을 함께 지우거나 바꾸는 경우다. (내리지 않는 대안 ㉮에서는 전원이 꺼질 때 두 쓰기가 OS 캐시에만 있어 둘 다 잃을 수 있다.)

### 3.5 로드 (`LoadGalaxy` → `LoadRun`)
1. 방송 중·틱 단계 안이면 거절(지금 문구 그대로 + 단계 안의 한 줄).
2. 주 슬롯: 읽기 → `Unpack` → `LoadGameFromMemory` → Cast → §3.3의 11.
3. **상함**(`Missing`~`Unreadable` — `CosmicSaveSlot::IsDamage`)이면 같은 차례로 `_bak`을 본다. **로드 하나에 Warning 한 줄이다**: 백업에서 열리면 `LoadGalaxy: slot <name>: <까닭>; read from its backup (tick <n>)`, 주와 백업이 둘 다 없으면 `LoadGalaxy: slot <name> is missing`, 그 밖은 `LoadGalaxy: slot <name>: <까닭>; its backup: <까닭>`. 백업을 읽을 때 `TryRead`는 줄을 내지 않는다 — 백업이 새 버전(`NewerSave`)이거나 성역이 없어도(`Sanctuary`) 그 까닭은 이 한 줄의 `its backup: <까닭>`이 말한다(버전 Warning·성역 Error는 주 슬롯을 읽을 때만 선다). 주가 없을 때(`Missing`)도 백업으로 간다(결정 5).
4. **거절은 백업으로 가지 않는다**: `ReadFailed`(Warning `LoadGalaxy: slot <name> could not be read: the backup is not tried`), `NewerForm`(Warning `LoadGalaxy: slot <name> has slot form <f>, this build reads up to <F>`), `NewerSave`·`Sanctuary`(지금 문구 그대로 — 그 줄은 슬롯을 읽는 자리가 낸다). 읽지 못했거나 이 빌드가 모르는 세이브 대신 옛 백업을 열면 말없이 되감긴다.
5. 열리면 `Sim->LoadState`, 디스크 사본을 읽은 상태로 맞춘다. 거절된 로드는 시뮬을 건드리지 않는다.

### 3.6 슬롯 찾기 (`ContinueNewest`, #33 ③ — 로드 메뉴 없이 이어하기 한 줄)
1. 방송 중이면 Error 한 줄과 `NoSlot`.
2. 슬롯 폴더의 `*.sav`를 `IFileManager::FindFiles`로 모은다(`FGenericSaveGameSystem::GetSaveGameNames`와 같은 호출에 폴더를 붙인 것 — Win64 전용, ⑨가 다시 본다).
3. 이름을 `ParseSlotName`으로 거른다: `Galaxy_` + 16진 32자리 + 선택 `_bak`만(39자 또는 43자). OneDrive 충돌 사본·임시 파일은 빠진다.
4. **머리만 읽는다**: 슬롯마다 파일 크기(`IFileManager::FileSize`)와 앞 64바이트(`IFileManager::CreateFileReader(경로, FILEREAD_Silent)`)로 `ReadHeader(머리, 파일 크기, 이름의 GUID)`를 본다. 페이로드는 고른 판에서만 읽는다(판이 쌓여도 시작이 느려지지 않는다).
   - 머리가 서면(`Ok`·`Truncated`·`NewerForm` — 봉투와 머리가 온전하면 판의 이름과 시각이 선다. 잘린 페이로드와 새 꼴은 `LoadRun`이 가린다) 그 슬롯의 시각은 머리의 `SavedAtUtcTicks`다. 첫 쓰기 도중에 죽어 페이로드만 잘린 새 판도 그래서 보인다.
   - `OldForm`(⑤ 전의 슬롯)은 이어할 판이 아니다 — Log 한 줄을 남기고 후보에서 뺀다(#70: 지금의 개발 세이브는 읽지 않는다).
   - 그 밖(머리가 상함·읽히지 않음·0바이트)은 **후보로 남기고** 시각은 그 파일의 `IFileManager::GetTimeStamp`(UTC — 차례 정하기에만 쓰는 대체값)다.
5. **가장 새 판 하나**: 판의 시각은 제 슬롯(주·백업) 가운데 가장 늦은 것이다. 새것이 먼저, 같으면 GUID 글자순으로 뒤의 것.
6. 그 판을 `LoadRun` → 주에서 열림 `Newest` / 백업에서 열림 `Backup` / 열리지 않음 `Unreadable`. **옛 판으로 내려가지 않는다**(결정 8) — 가장 새 판의 파일이 잠겨 있거나 머리부터 상했어도 그 앞의 판이 아니라 `Unreadable`이다.
7. **아무것도 지우지 않는다.** Display `CosmicContinue: <outcome> <slot> at tick <n>, hash <h> (<k> runs seen)`.

### 3.7 폴더와 이름
- 게임의 슬롯은 `Saved/SaveGames/` 바로 아래다(지금 그대로). **테스트의 슬롯은 `Saved/SaveGames/Automation/`이다**: `AcquireSim`이 `PrepareForTests()`로 폴더를 바꾼다. 게임의 찾기는 바로 아래만 보므로(목록은 하위 폴더를 보지 않는다) 테스트가 남긴 슬롯은 게임에 닿지 않는다. 디스크 찾기를 시험하는 `Sim.SaveFind`는 `SetFolderForTest("Automation/Find/")`로 제 폴더를 쓴다 — 두 프로세스에 걸친 `Sim.SaveRestart`의 슬롯이 `Automation/`에 있기 때문이다(§8).
- 슬롯 이름 = 폴더 + `MakeSlotName(GUID)`(+ `_bak`). `MakeSlotName`은 그대로다(`Galaxy_<32자리>`, `docs/21:473`) — 폴더를 모르므로 **테스트는 슬롯 이름과 경로를 `SlotNameOf`·`SlotPathOf`로만 만든다**. 엔진의 슬롯 함수를 직접 부르는 자리도 같다(`MakeSlotName`으로 만들면 게임의 폴더에 닿는다).
- A-5a에서 `Sim.SaveRestart`의 '다른 빌드의 기록' 가지는 `Automation/`의 슬롯과 함께 게임 폴더의 옛 슬롯(`MakeSlotName(RecordedGalaxyID)` — 그 전의 바이너리가 남긴 것)도 지운다. 이것만이 테스트가 일부러 게임 폴더에 닿는 자리다.

### 3.8 선언 — `Save/CosmicSaveSlot.h`
include는 `CoreMinimal.h`·`Containers/ArrayView.h`·`Misc/Guid.h`, 앞선 선언은 `UCosmicSaveGame`. `.cpp`는 `Kismet/GameplayStatics.h`(`docs/20:49`가 Save에 허락), `Compression/OodleDataCompression.h`, `Misc/Crc.h`, `Serialization/MemoryWriter.h`·`MemoryReader.h`, `Save/CosmicSaveGame.h`만 읽는다. 같은 이름의 헤더는 타깃에 없다.

```cpp
// Cosmic Ascension - Save layer (Core Logic). Design: docs/20 section 9.1, 16 #70, #33 (2) (3), headers/A5_save_minimum.md.
// RULE: no UI/World include. The slot form: the engine's save bytes (UGameplayStatics::SaveGameToMemory) packed with Oodle
// behind a checked 64-byte header. Pure functions over bytes - no file is read or written here, no state changed, nothing
// hashed or drawn. Win64 byte order.

/** What a slot was found to be, in the order of the checks. Missing..Unreadable are damage: the load then tries the backup
 * (16 #33 (2)). ReadFailed, NewerForm, NewerSave, Sanctuary and Refused refuse without trying it: a slot that did not read,
 * or one this build cannot read, is never replaced by an older copy. Log text only and never saved; CosmicSaveSlot::IsDamage
 * names its members one by one (a switch, no range), so the order is for reading only. */
enum class ECosmicSlotRead : uint8
{
	Ok,
	Missing,         // no file of that name
	Short,           // fewer bytes than a header
	OldForm,         // the engine's own form ('GVAS' first): every slot from before MVP item 5
	NotOurs,         // another first word
	HeaderDamaged,   // HeaderCrc does not match (checked before Form: the envelope is the same in every form)
	BadSizes,        // Form 0, an unknown codec, RawBytes out of bounds, PackedBytes not positive, Stored with RawBytes != PackedBytes, or a file too large for a slot
	Truncated,       // the file is not exactly HeaderBytes + PackedBytes long (cut short, or bytes appended)
	OtherGalaxy,     // the header or the state names another galaxy than the slot name
	PayloadDamaged,  // PayloadCrc does not match
	CodecRefused,    // the codec did not give exactly RawBytes
	Unreadable,      // the engine bytes give no UCosmicSaveGame
	ReadFailed,      // the file exists and did not read twice (a lock, an I/O error): not damage
	NewerForm,       // an intact header of a later slot form (Form above CosmicSaveSlot::FormVersion)
	NewerSave,       // SaveVersion above FGalaxyState::CurrentSaveVersion
	Sanctuary,       // the last system is not a kept sanctuary
	Refused,         // asked from a handler of a Core Logic delegate or inside the tick stages: nothing read
};

namespace CosmicSaveSlot
{
	inline constexpr uint32 Magic = 0x56534143;        // 'C' 'A' 'S' 'V' in file order
	inline constexpr uint32 EngineMagic = 0x53415647;  // UE_SAVEGAME_FILE_TYPE_TAG (GameplayStatics.cpp:87): 'GVAS' in file order
	inline constexpr uint16 FormVersion = 1;
	inline constexpr int32 HeaderBytes = 64;
	inline constexpr int32 HeaderCrcBytes = 60;        // HeaderCrc covers the bytes before it
	inline constexpr int64 MaxRawBytes = 64ll * 1024 * 1024;   // a bound before any allocation

	// The envelope every form keeps (byte offsets): a later build's slot is still seen by the continue and refused, never
	// taken for damage.
	inline constexpr int32 FormAt = 4;
	inline constexpr int32 GalaxyIDAt = 8;
	inline constexpr int32 SavedAtAt = 24;
	inline constexpr int32 HeaderCrcAt = 60;

	enum class ECodec : uint8 { Stored = 0, Oodle = 1 };   // in the file: never renumber, add at the end

	struct FHeader
	{
		uint16 Form = 0;
		ECodec Codec = ECodec::Stored;
		FGuid GalaxyID;
		int64 SavedAtUtcTicks = 0;
		int64 Tick = 0;
		int64 RawBytes = 0;
		int64 PackedBytes = 0;
		uint32 PayloadCrc = 0;
		uint32 HeaderCrc = 0;
	};

	/** One slot file as the continue sees it before any load. */
	struct FSeen
	{
		FGuid GalaxyID;
		bool bBackup = false;
		int64 UtcTicks = 0;            // the header's SavedAtUtcTicks when the header reads, else the file's time (order only)
		bool bTimeFromHeader = false;
	};

	COSMIC_ASCENSION_API bool IsDamage(ECosmicSlotRead Read);        // Missing through Unreadable, each by name
	COSMIC_ASCENSION_API const TCHAR* NameOf(ECosmicSlotRead Read);

	/** Galaxy_<32 hex digits>, or the same with _bak; anything else (a conflict copy, a temp file) is no slot of a run. */
	COSMIC_ASCENSION_API bool ParseSlotName(const FString& SlotName, FGuid& OutGalaxyID, bool& bOutBackup);

	/** The largest file a slot can be: HeaderBytes + the Kraken bound of MaxRawBytes. */
	COSMIC_ASCENSION_API int64 MaxSlotBytes();

	/** The slot bytes of Save: SaveGameToMemory, then Oodle (Kraken, VeryFast - fixed here, so no cook or command-line
	 * setting changes it), Stored when the codec gives nothing or Want is Stored (tests); the header with both CRCs. Writes no
	 * file. False (OutSlot empty) when a step fails. OutRawBytes (optional): the engine bytes' length (the commandlet). */
	COSMIC_ASCENSION_API bool Encode(UCosmicSaveGame& Save, int64 SavedAtUtcTicks, TArray<uint8>& OutSlot,
		int64* OutRawBytes = nullptr, ECodec Want = ECodec::Oodle);

	/** Checks 1-7 over the first HeaderBytes of Head (it may hold the whole slot) and the file's length FileBytes (Expected:
	 * the galaxy the slot name says; null = any). The continue reads 64 bytes and passes the file's size. */
	COSMIC_ASCENSION_API ECosmicSlotRead ReadHeader(TConstArrayView<uint8> Head, int64 FileBytes, const FGuid* Expected, FHeader& OutHeader);

	/** ReadHeader over the whole Slot, then the payload CRC: what the backup step reads before it copies a slot. */
	COSMIC_ASCENSION_API ECosmicSlotRead Check(TConstArrayView<uint8> Slot, const FGuid* Expected, FHeader& OutHeader);

	/** Check, then the engine bytes (exactly RawBytes) for UGameplayStatics::LoadGameFromMemory - which only ever sees bytes
	 * the length and the CRC passed (16 #70: a cut inside a property reaches a fatal check of the engine). */
	COSMIC_ASCENSION_API ECosmicSlotRead Unpack(TConstArrayView<uint8> Slot, const FGuid* Expected, FHeader& OutHeader, TArray<uint8>& OutRaw);

	/** The run the continue opens: the latest UtcTicks of any of its slots, a tie to the later GUID text; invalid for none. */
	COSMIC_ASCENSION_API FGuid NewestRun(TConstArrayView<FSeen> Seen);
}
```

## 4. 자동 저장 — 트리거와 자리

### 4.1 원칙
- **틱 안에서는 저장하지 않는다**(#33 ①). 승천(여섯째 단계)·폐위(열째 단계)·임시 끝은 같은 틱의 `OnSimTick`에서 디스크 사본과의 비교로 보인다. #33 ①의 '보류 요청을 남기고 가장 바깥 `SimulateTick`의 끝에서 저장'이 이것이다. 시뮬에 새 멤버가 없다.
- `OnSimTick`은 규칙 5의 세 핸들러(`OnLogEntry`·`OnEventFired`·`OnTierAscended`)가 아니고 Begin/EndBroadcast 밖이다. 그 자리에서는 `bInTickStages`도 내려가 있다(`.cpp:1386-1394`). 그래서 거기서 쓰는 것은 규칙 5와 부딪치지 않는다.
- 세 핸들러 안의 `SaveCurrentGalaxy`는 Error 한 줄과 무동작이다. 같은 자리의 `RequestSave`는 깃발 하나만 남기고(상태·틱·평가기·명령·앎·로그 어느 것도 아니다), 다음 틱 경계가 쓴다.
- 게임의 길의 쓰기가 실패하면 그 까닭이 남고(보류), 60초 뒤의 첫 경계가 다시 쓴다(결정 12). 어느 길이든 쓰기가 성공하면 남은 까닭과 기다림을 지운다(그 쓰기가 지금의 상태를 디스크에 두었다).
- Phase 4b의 공백기 도약은 `RunHeadless`가 방송 문을 닫으므로(`.cpp:1083-1096`) `OnSimTick`이 오지 않는다. '사망과 계승 사이에는 저장하지 않는다'(`docs/20` §9.1)가 저절로 서고, 그 헤더가 `BeginInterlude`의 명시적 저장 지점을 둔다.
- 경계 하나에 쓰기는 많아야 하나다. 멈춘 동안에는 경계가 없어 쓰지 않는다.
- 저장 Subsystem과 컨트롤러의 `OnSimTick` 핸들러는 서로의 차례에 기대지 않는다. 저장은 상태만 읽고(끝도 `ProvisionalEndOf`로 스스로 본다), 컨트롤러 핸들러는 배속과 표현만 바꾼다.

### 4.2 트리거 표
| 트리거(`docs/20` §9.1) | 지금 있나 | 자리 | 방식(쓴 까닭) |
|---|---|---|---|
| 새 판(게임 시작·'새 판') | 있다 | `ACosmicGameMode::StartPlay`의 새 은하 가지(`Super::StartPlay()` 앞), `ACosmicPlayerController::StartNewRun`(`:735-745`)의 `NewGalaxy` 뒤 | 입력: `RequestSave(NewRun)` — 틱 0이 곧바로 디스크에 선다. 다만 가장 새 판을 읽지 못해(`Unreadable`) 연 새 판은 `HoldStartRun`으로 적어 두고 첫 틱 경계가 쓴다(결정 6). 경계의 'GalaxyID가 다르다'가 뒷받침한다 |
| 승천 직후 | 있다(여섯째 단계 `EvaluateFeats`) | 저장 Subsystem의 `OnSimTick` 핸들러 | 디스크 사본과 비교: `CompletedFeats` 수(`Ascended`) |
| 역할 전환 직후 — 폐위 | 있다(열째 단계 `DeposeRuler`) | 같은 자리 | `Role`·`RuledCivID`(`Role`). 자진 이탈·즉위·복귀(Phase 4)도 같은 비교가 받는다. 사망·계승(Phase 4b)은 그 헤더가 디스크 사본에 생애 수와 `LifeState`를 더한다 |
| 임시 끝이 선 틱 | 있다 | 같은 자리 | `ProvisionalEndOf != None`이 디스크 사본과 다르다(`Ended`) |
| 10분마다 | 있다 | 같은 자리 | 마지막 쓰기부터 실제 시간 600초 ∧ 틱이 움직임(`Interval`, 결정 13) |
| 받아들인 청원의 답 | `docs/20` §9.1에는 없다(`crisis` 210행 '선택 해결 뒤') | `ACosmicPlayerController::AnswerPetition`(`:1497`), `Refused`가 아닐 때 | 입력: `RequestSave(Answer)`. 카드가 시간을 멈춘 채라 경계가 오지 않는다. 강제 종료 뒤 같은 카드를 다시 굴리는 길이 닫힌다(결정 11) |
| 종료(창 닫기·Alt+F4·`quit`) | 있다 | 창 닫기·Alt+F4: 게임 뷰포트의 `OnCloseRequested`(컨트롤러가 `BeginPlay`에 묶는다 — 창을 닫으면 로컬 플레이어가 지워지며 컨트롤러는 `Destroyed`로 끝나 `Quit`을 보지 못한다: `GameEngine.cpp:768-777` → 다음 프레임 `:1807-1818`의 `CleanupGameViewport` → `GameInstance.cpp:1003`. 구현 리뷰가 찾았다). `quit`·`RequestExit`: `ACosmicPlayerController::EndPlay`, 까닭이 `Quit`일 때 묶음을 풀기 전 | `RequestSave(Exit)`. 둘 다 닿아도 한 번만 쓴다(같은 해시). 디스크 사본의 해시와 같으면 쓰지 않는다. `HoldStartRun`이 적은 판(읽지 못한 뒤 연 새 판)이 아직 쓰이지 않았고 틱이 0이면 쓰지 않는다 — 그 판 하나만이다(결정 6). `World->EndPlay(Quit)`가 `GameInstance->Shutdown()`보다 먼저라 상태가 살아 있다 |
| 사망 직후·계승 직후·귀환 선택 뒤 | 없다(`KillPlayer`는 기록만) | Phase 4b의 `BeginInterlude` 명시 지점 | — |

**쓰지 않는 것**: 프로젝트 시작(`StartMegaproject` — 추첨이 없는 입력이라 10분과 종료가 덮는다, 결정 11의 대안 ㉯), 맵 다시 열기(컨트롤러의 `EndPlay` 까닭은 `Destroyed`, 월드는 `LevelTransition` — `Quit`이 아니다, §1.2), Subsystem의 `Deinitialize`(차례가 보장되지 않는다).

### 4.3 게임 저장의 문 (`IsGameSavingOn` — 결정 4)
- 문은 **게임의 길**(시작의 이어하기, 경계의 자동 저장, 입력·종료의 `RequestSave`)에만 선다. `SaveCurrentGalaxy`·`LoadGalaxy`·`ContinueNewest`·`DeleteSave`는 문을 보지 않는다(테스트·도구의 명시 호출).
- **닫히는 경우**: `-NoSave` · `-unattended`(테스트·스모크·스크립트) · `-Seed=`(재현 실행 — 이어하기를 가로채지 않는다) · 에디터 프로세스(`GIsEditor` — 에디터 안 플레이. `-game` 프로세스와 에디터의 'Standalone Game'은 열린다) · 자동화 테스트가 도는 동안(`GIsAutomationTesting`) · `AcquireSim`이 `PrepareForTests()`를 부른 뒤 그 게임 인스턴스 내내.
- 문은 순수 함수 둘로 짓는다: `CommandLineAllows(명령줄)`(`-NoSave`·`-unattended`·`-Seed=`를 그 글에서 읽는다)와 `GateClosedBy(…)`(닫은 까닭의 이름, 열렸으면 null). 테스트 프로세스는 늘 `-unattended`라 산 문으로는 다른 까닭을 볼 수 없으므로, `Sim.SaveTriggers`가 까닭마다 한 경우를 이 둘로 단언한다.
- 문이 닫힌 시작은 `CosmicContinue: Off (<까닭>)` 한 줄을 남긴다(`command line`·`tests`·`automation`·`editor`). 에디터의 Play로 손 확인을 하면 이 줄이 까닭을 말한다.
- 테스트가 게임의 길을 시험할 때만 `SetGameSavingForTest(true)`로 켜고, 끝에 끈다(다음 테스트의 `AcquireSim`도 끈다).
- `run_checks.ps1`의 테스트와 스모크는 이미 `-unattended`라 명령을 고치지 않는다. 손 확인 ⓐ는 `-Seed=`·`-NoSave`·`-unattended` 없이 명령줄 `-game -windowed`나 에디터의 Play ▸ Standalone Game으로 띄운다(에디터 안 플레이는 저장하지 않는다 — docs/40에 적는다).
- **종료 조건 ⓓ**(쿠킹 빌드의 시드 42 세계 해시 = 에디터)의 실행은 `-Seed=42`라 문이 닫혀, 이어하기가 기준 판을 가로채지 않는다. 패키지의 저장 폴더는 ⑨가 본다.

### 4.4 선언 — `Save/CosmicSaveSubsystem.h` (바뀐 꼴)
include에 `Misc/Optional.h`·`Core/CosmicTypes.h`·`Save/CosmicSaveSlot.h`를 더하고(`Misc/Guid.h`는 이미 있다), `struct FGalaxyState; struct FCosmicRulesDef; class UGalaxySimulationSubsystem;`을 앞서 선언한다. `.cpp`가 `Core/GalaxyState.h`·`Data/CosmicDefinitionRows.h`·`Sim/GalaxySimulationSubsystem.h`·`Sim/CosmicEndings.h`·`HAL/PlatformFileManager.h`·`HAL/FileManager.h`·`HAL/PlatformTime.h`·`Misc/CommandLine.h`(`FCommandLine` — `CoreMinimal`에 없다)·`Misc/FileHelper.h`·`Misc/Paths.h`를 읽는다(Save → Core·Data·Sim, 규칙 1 — 전이 include에 기대지 않는다). private 멤버와 본문은 구현 메모 §2.

```cpp
/** Why a slot was written: log lines and tests only (not in the file, not state, not hashed). */
enum class ECosmicSaveCause : uint8 { Asked, NewRun, Ascended, Role, Ended, Interval, Answer, Exit };

/** What the start of the game did (ACosmicGameMode::StartPlay through BeginSession). Never saved. */
enum class ECosmicContinue : uint8
{
	NotBegun,     // no start yet in this game instance
	Off,          // the game's saving is off (IsGameSavingOn): a new galaxy, nothing read or written - as before item 5
	NoSlot,       // no slot of a run: a new galaxy, written at once (NewRun)
	Newest,       // the newest run, from its slot
	Backup,       // the newest run, from its backup (its slot was damaged)
	Unreadable,   // the newest run did not open: a new galaxy, first written at a tick boundary (decision 6); the files stay
};

namespace CosmicSave
{
	COSMIC_ASCENSION_API const TCHAR* NameOf(ECosmicSaveCause Cause);
	COSMIC_ASCENSION_API const TCHAR* NameOf(ECosmicContinue Outcome);
}

/** What the slot of the running galaxy holds, as far as the tick-boundary check needs it: a cache of the disk (the disk copy),
 * replaced whole by every write and every load (losing it costs one more write, nothing else). Not saved, not hashed, read
 * by nothing but the save layer; no fact here that the state does not hold (rule 2). Phase 4b adds the number of lives and
 * the LifeState for death and succession. */
struct FCosmicSavedMark
{
	FGuid GalaxyID;                                // invalid = nothing written or loaded for this run
	int64 Tick = -1;
	uint32 StateHash = 0;                          // ComputeStateHash at the write or the load (the exit compares it)
	int32 Feats = 0;                               // Calling.CompletedFeats.Num(): an ascension adds one
	EPlayerRole Role = EPlayerRole::Ruler;         // deposition now; abdication, enthronement, return (Phase 4)
	FName RuledCivID;
	bool bEnded = false;                           // CosmicEndings::ProvisionalEndOf != None
	double RealSeconds = 0.0;                      // FPlatformTime::Seconds at the write or the load
};

/**
 * FGalaxyState <-> the slot (docs/20 section 9.1): one slot per run (ironman) and its one backup; no manual save or load
 * menu. The game's own saving - the continue at start, the tick-boundary autosave, the saves of input and exit - goes
 * through one gate (IsGameSavingOn); the explicit calls below it do not.
 */
UCLASS()
class COSMIC_ASCENSION_API UCosmicSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// InitializeDependency<UGalaxySimulationSubsystem>, binds OnSimTick, reads -NoSave, -unattended and -Seed= once
	// (CommandLineAllows).
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// Unbinds; writes nothing (the order of shutdown is not fixed - the exit save is the controller's EndPlay(Quit)).
	virtual void Deinitialize() override;

	static FString MakeSlotName(const FGuid& GalaxyID);                       // unchanged: Galaxy_<32 hex digits>
	FString SlotNameOf(const FGuid& GalaxyID, bool bBackup = false) const;   // the slot folder + MakeSlotName (+ "_bak")
	// ProjectSavedDir()/SaveGames/<SlotNameOf>.sav - the generic save system's path (Win64; MVP item 9 checks the package).
	FString SlotPathOf(const FGuid& GalaxyID, bool bBackup = false) const;

	// ---- Explicit calls (tests, tools): the gate does not apply ----

	// Writes the running galaxy now (section 3.4 of the MVP item 5 header): the verified slot to the backup first, flushed
	// to the disk - a failed copy, a slot that did not read and a slot of a later form keep the slot and return false; a
	// damaged slot is not copied - then the slot, flushed. Cause Asked. Refused with one error line inside a handler of a
	// Core Logic delegate (IsBroadcasting) and inside the tick stages (IsInTickStages).
	UFUNCTION(BlueprintCallable)
	bool SaveCurrentGalaxy();

	// Loads a run: its slot, or its backup when the slot is damaged (CosmicSaveSlot::IsDamage) or missing; a slot that did
	// not read, one of a later form or version and one without a kept sanctuary are refused without trying the backup. A
	// refused load leaves the simulation untouched. Refused with one error line inside a handler or the tick stages.
	UFUNCTION(BlueprintCallable)
	bool LoadGalaxy(const FGuid& GalaxyID);
	ECosmicSlotRead LoadRun(const FGuid& GalaxyID, bool& bOutFromBackup);

	// The continue (16 #33 (3)): the slots of the slot folder, 64 header bytes each, the newest run loaded
	// (CosmicSaveSlot::NewestRun); never an older run, never a delete. Newest, Backup, Unreadable or NoSlot. Refused (NoSlot,
	// one error line) in a handler.
	ECosmicContinue ContinueNewest();

	UFUNCTION(BlueprintPure)
	bool DoesSaveExist(const FGuid& GalaxyID) const;   // the slot or its backup

	UFUNCTION(BlueprintCallable)
	bool DeleteSave(const FGuid& GalaxyID);            // both; true when one was deleted

	// ---- The game's own saving ----

	// GateClosedBy over this process: off with -NoSave, -unattended, -Seed=, in the editor process, while an automation test
	// runs, and after PrepareForTests - unless SetGameSavingForTest(true).
	bool IsGameSavingOn() const;
	// The gate as pure functions (Sim.SaveTriggers: one case per reason). CommandLineAllows: none of -NoSave, -unattended,
	// -Seed= in CommandLine. GateClosedBy: the first reason that closes it ("command line", "tests", "automation",
	// "editor"), null when open; bTestOverride opens it whatever the rest.
	static bool CommandLineAllows(const TCHAR* CommandLine);
	static const TCHAR* GateClosedBy(bool bCommandLineAllows, bool bBlockedForTests, bool bAutomationTesting, bool bEditor, bool bTestOverride);

	// Once per start, from ACosmicGameMode::StartPlay before Super::StartPlay: Off (one line with the reason) when the gate
	// is closed, else ContinueNewest. The load's OnGalaxyGenerated reaches no listener yet; both BeginPlay calls read the
	// state (headers/A4 4.4 (5)).
	ECosmicContinue BeginSession();
	// The controller's BeginPlay takes it once (then NotBegun): a later map load without a start of its own says nothing.
	ECosmicContinue TakeStartOutcome();
	// What the last BeginSession or ContinueNewest did (tests, log lines); never cleared by TakeStartOutcome.
	ECosmicContinue GetLastContinue() const;
	// StartPlay, right after the new galaxy of an Unreadable start (decision 6): notes that galaxy's GalaxyID. That run alone
	// is first written at a tick boundary and its exit save is skipped while it is unwritten at tick 0, so the run that did
	// not read stays the newest on disk; a later new run of the session is written at once. BeginSession clears it.
	void HoldStartRun();

	// Input time (no handler, no stage): writes now - except Exit when the slot already holds this state (same hash), and
	// Exit at tick 0 of the run HoldStartRun noted while it is unwritten (decision 6). From a handler or a stage, and after a
	// failed write of the game's path: kept and written at the next tick boundary (after a failure, once RetryRealSeconds
	// have passed). A write that succeeds, on any path, clears what was kept and the wait. Nothing while the gate is closed
	// or without a galaxy.
	void RequestSave(ECosmicSaveCause Cause);

	// True once per game instance, after the first failed write of the game's saving (the controller stops the time once).
	bool TakeFirstFailure();

	// ---- Tests ----
	void PrepareForTests();                         // CosmicTests::AcquireSim: folder Automation/, gate off, override off, timing reset
	void SetFolderForTest(const FString& Folder);   // e.g. "Automation/Find/": the continue lists that folder only (no descent)
	void SetGameSavingForTest(bool bOn);            // opens the gate for one test; the next PrepareForTests closes it
	void ContinueAtNextStartForTest();              // the next StartPlay continues even over a running galaxy (a map reload)
	bool ConsumeContinueAtStart();                  // StartPlay reads it once
	void SetTimingForTest(double IntervalSeconds, double RetrySeconds);
	int32 GetSavesWritten() const;
	TOptional<ECosmicSaveCause> GetLastCause() const;

	// docs/20 section 9.1's "every ten minutes": real seconds since the last write, looked at on a tick boundary (decision 13).
	static constexpr double AutosaveRealSeconds = 600.0;
	// After a failed write of the game's path the boundary waits this long before it writes again (decision 12).
	static constexpr double RetryRealSeconds = 60.0;

	// The boundary rule as pure functions (tests). DueAt: NewRun when the galaxy differs, else Ended, else Ascended, else Role,
	// else Interval (IntervalSeconds passed and the tick moved), else unset. MarkOf leaves StateHash and RealSeconds to the write.
	static TOptional<ECosmicSaveCause> DueAt(const FCosmicSavedMark& Saved, const FCosmicSavedMark& Now, double NowRealSeconds, double IntervalSeconds);
	static FCosmicSavedMark MarkOf(const FGalaxyState& State, const FCosmicRulesDef& Rules);

private:
	UFUNCTION()
	void HandleSimTick(int64 Tick);
	// the rest: implementation notes section 2
};
```

`Sim/GalaxySimulationSubsystem.h`: `IsBroadcasting()`(261행) 옆, public 구역에 `bool IsInTickStages() const { return bInTickStages; }`를 둔다(`bInTickStages`는 332행부터의 private 구역 502행이다). 252~260행의 거절 목록 주석에는 `UCosmicSaveSubsystem::SaveCurrentGalaxy`·`LoadRun`·`ContinueNewest`만 더한다(`LoadGalaxy`는 이미 있다). `Log/CosmicLogSubsystem.h:122`의 "SaveCurrentGalaxy waits for section 16 #33 (1)"은 "is refused there; RequestSave waits for the tick boundary"로 고친다.

### 4.5 디스크 사본이 규칙 2의 '생애 종속 상태'가 아닌 까닭 (결정 16)
- 디스크 사본(`FCosmicSavedMark`)은 **디스크가 무엇을 들었는지의 사본**이다. 상태에 없는 사실이 하나도 없고, 저장도 해시도 되지 않고, 저장 층 밖에서 읽는 곳이 없다.
- 모든 쓰기와 로드가 디스크 사본을 통째로 바꾼다. 새 은하는 그것을 건드리지 않으므로 GalaxyID가 달라 새 판으로 잡힌다. 잃으면 저장이 한 번 더 일어날 뿐이다.
- 저장 규칙을 표현 층(컨트롤러)이 갖지 않고 Core Logic의 저장 층이 갖는다. 컨트롤러는 입력 셋을 부탁하고 실패를 알릴 뿐이다. 그래서 경계 규칙(`DueAt`·`MarkOf`)은 순수 함수로, 컨트롤러 없이 시험된다.
- 다만 산 경계를 도는 `Sim.SaveTriggers`에는 테스트 프로세스의 컨트롤러도 `OnSimTick`에 묶여 있다. 동적 멀티캐스트는 목록의 차례대로 부르고(`Core/Public/UObject/ScriptDelegates.h:1438-1447`) 저장 Subsystem이 `Initialize`에서 먼저 묶이므로, 경계의 쓰기가 실패하면 컨트롤러의 `HandleSimTick` → `FollowSaveFailure`가 같은 방송 안에서 `TakeFirstFailure`를 가져간다(목록의 정리 `RemoveAllSwap`이 차례를 바꿀 수 있어 어느 쪽에도 기대지 않는다 — `:1600-1605`). 그래서 그 테스트는 실패 깃발을 경계가 없는 입력 때의 부탁(`RequestSave(Answer)`)으로 단언한다(§8).

### 4.6 저장은 해시를 바꾸지 않는다
`SyncRngState`는 해시가 이미 읽는 실시간 스트림 위치(`.cpp:1337-1341`)를 상태에 복사할 뿐이다. `ComputeStateHash`는 const이고 `Encode`는 상태를 읽기만 한다. 저장은 로그 줄을 상태에 쓰지 않는다(`UE_LOG`만). `Sim.SaveNeutral`이 '저장을 자주 한 판과 하지 않은 판의 해시·스트림이 같다'를 단언한다.

## 5. 이어하기와 화면

### 5.1 게임 시작 (`ACosmicGameMode::StartPlay` 96~117행의 새 꼴 — 요지, 본문은 구현 메모 §3.1)
1. `Sim`이 있고 (`GalaxyID`가 없거나 `Save->ConsumeContinueAtStart()`)이면 `Start = Save->BeginSession()`.
2. 은하가 여전히 없거나 `Start`가 `Off`·`NoSlot`·`Unreadable`이면 지금처럼 `NewGalaxy(Seed, 0, 0)`(시드는 `-Seed=`, 없으면 시계). `NoSlot`이면 곧바로 `Save->RequestSave(NewRun)`(문이 닫혀 있으면 아무것도 하지 않는다). `Unreadable`이면 쓰지 않고 `Save->HoldStartRun()`으로 그 판을 적는다 — 플레이어가 시간을 흘린 첫 틱 경계가 'GalaxyID가 다르다'로 쓰고, 그 전에 끄면 종료 저장이 그 판만 건너뛰어 다음 시작이 옛 판을 다시 읽어 본다(결정 6).
3. 배속: `-Speed=`가 있으면 그 값, 없으면 1을 `SetTimeScale`(이어 연 판에도 — 지금은 새 은하 가지 안에만 있다).
4. 그다음에 `Super::StartPlay()`. 로드가 핸들러가 묶이기 전이라 `OnGalaxyGenerated`는 아무에게도 닿지 않고, 두 `BeginPlay`가 상태를 읽는다(첫 판의 `NewGalaxy`와 같은 길).

### 5.2 끝난 판 — 끝난 채 선다 (A4 §4.4 ⑤, §5.1 221행)
- **엔진의 차례(§1.2 — 컨트롤러가 먼저)**: 컨트롤러의 `FollowEnd(false)`(`:552`)가 래치를 세우고 `ApplySpeed(0, RunEnd)`를 건다. HUD의 위젯이 아직 없어 래치만 선다 — HUD 액터는 이미 있다(PostLogin의 `ClientSetHUD`가 `PlayerController.cpp:1387`에서 낳았고, 그 `BeginPlay`는 아직이다). 그래서 `ApplySpeed` → `RefreshPanel` → `ShowRightSide`가 시작 전의 HUD 위에서 돈다. 지금은 모두 null에 안전하고(`World/CosmicGameMode.cpp:305-358`), 이 길을 처음 걷는 것이 ⑤다(이어 연 흐르던 판의 `FollowStart`도 같다 — 구현 메모 §3.2). 그 뒤 HUD의 `BeginPlay`에서, 카드를 만든 직후의 `FollowPetition(false)`(`:264`)는 끝난 판이라 곧 돌아오고, 연대기 쪽을 만든 뒤의 `FollowEnd(false)`(`:283`)가 쪽을 다 드러난 채 연다.
- HUD가 먼저인 차례(A4가 대비한 길 — 엔진의 차례가 바뀌면): `FollowPetition(false)`가 대기 카드를 한 번 올리고, 이어지는 `FollowEnd(false)`가 래치를 세우며 카드를 접고(`:623-626`) 쪽을 연다. 컨트롤러의 `FollowEnd(false)`는 아무것도 바꾸지 않는다.
- 어느 차례든 **래치·다 드러난 쪽·카드 없음·배속 0·까닭 `RunEnd`**다. 대기 카드의 행은 앎(`Knowing.PendingPetitionRow`)에 그대로 남는다. 이어 연 정지 줄은 서지 않는다(쪽의 '새 판'이 나가는 길이다).
- 끝난 판을 이어 열면 디스크 사본이 `bEnded = true`로 맞춰져 아무 트리거도 걸리지 않고, 종료 저장도 해시가 같아 쓰지 않는다. '새 판'을 누를 때까지 몇 번을 켜도 끝난 채 열린다.
- 이때 래치가 남기는 `CosmicEnd:` 줄은 판의 길이가 아니라 '끝난 채 열었다'를 말한다(§5.6).
- `UI.ContinueEnded`가 실제 맵 다시 열기로 이것을 단언한다(§8 — A4 §10이 넘긴 '두 `BeginPlay`의 부름'. 뒤에 오는 HUD의 부름을 지킨다).

### 5.3 흐르던 판 — 멈춘 채 까닭 한 줄 (결정 1)
- 컨트롤러 `BeginPlay`의 `FollowEnd(false)` 뒤, 끝나지 않았고 `TakeStartOutcome()`이 `Newest`·`Backup`·`Unreadable`이고 `-Speed=`가 없으면 `ApplySpeed(0, Continued | ContinuedFromBackup | SaveUnreadable)`. `ApplySpeed`가 지금 배속(StartPlay가 둔 1)을 `SpeedBeforePause`에 적으므로(`:1574`) Space가 ×1로 다시 흐르게 한다(`:1619`). 배속 단추도 그대로 흐르게 한다.
- 대기 카드가 있으면 카드가 선다(지금의 `FollowPetition(false)`). 피드는 저장된 `Log`에서 다시 짓고, 지도는 `BeginPlay`에서 다시 짓기를 건다. 선택·보기(영적/물리)·카메라 틀·힌트는 처음 상태다(`docs/20:68, 114` — 표현은 상태에서 다시 그린다). 시계 단위·자동 정지는 `GGameUserSettingsIni`에 있어 그대로다.
- **흐르던 판을 버리는 길은 끝까지 가는 것이다**(결정 2 — ×10에서 6,000까지 약 10분, 끝의 쪽에 '새 판'이 있다). 개발자는 `-NoSave`로 켜면 늘 새 판이다(§4.3). 상단 바의 '새 판'은 결정 2의 대안 ㉯다(구현 메모 §3.4).
- `HandleGalaxyGenerated`가 지금처럼 까닭을 지운다.
- 저장 실패: 컨트롤러가 `HandleSimTick`의 끝과 입력 저장(답·새 판) 뒤에 `Save->TakeFirstFailure()`를 보고, 참이면 한 번 `ApplySpeed(0, SaveFailed)`(결정 12).

### 5.4 선언 — 컨트롤러·자동 정지 열거
```cpp
	// ACosmicPlayerController - public
	// After FollowEnd(false) in BeginPlay: a continued run that is not over stands still with its line (MVP item 5) - unless
	// -Speed= was given. Takes UCosmicSaveSubsystem::TakeStartOutcome; never from a handler of a Core Logic delegate.
	void FollowStart();

	// ECosmicAutoPause (a plain enum class, never saved) - appended at the end. MVP item 5: not automatic pauses - set
	// whatever the viewer's choice; cleared by the next speed asked for.
	Continued,             // the game opened on a continued run
	ContinuedFromBackup,   // ... on its backup (the last save did not read)
	SaveUnreadable,        // the newest run's save did not read: a new run
	SaveFailed,            // a write of the game's saving failed (once per game instance)
```
`WhyKeyOf`(`World/CosmicAutoPause.cpp:50-66`)가 넷을 §5.5의 키로 잇는다. `EndPlay`(`:556`)는 묶음을 풀기 전에 `if (EndPlayReason == EEndPlayReason::Quit) Save->RequestSave(ECosmicSaveCause::Exit)`, `AnswerPetition`은 `Refused`가 아니면 `RequestSave(Answer)`, `StartNewRun`은 `NewGalaxy` 뒤 `RequestSave(NewRun)`을 더한다. 끝난 판에서는 `AnswerPetition`이 이미 `Refused`라 저장하지 않는다.

### 5.5 ST_CA 키와 글 (`Tools/data/ST_CA.csv` 끝에 넷, 140행을 고친다 — A-5c)
| 키 | 글 | 자리 |
|---|---|---|
| `Save.Why.Continued` | 정지 — 하던 판을 이어 열었다 | 상단 바 까닭 줄(`AutoPause.Why.*`의 꼴) |
| `Save.Why.Backup` | 정지 — 그 앞의 저장으로 이어 열었다 | 같은 자리 |
| `Save.Why.Unreadable` | 정지 — 저장을 읽지 못해 새 판을 열었다 | 같은 자리 |
| `Save.Why.Failed` | 정지 — 이 판을 저장하지 못했다 | 같은 자리(세션에 한 번). 폴더의 빈 자리·쓰기 권한의 안내는 Warning 로그 줄과 테스터 안내에 둔다 |
| `Chronicle.NewRunTip`(고침) | 새 은하에서 새 판을 연다. 이 판으로는 다시 돌아오지 않는다. | 쪽의 '새 판' 툴팁. 새 판의 첫 쓰기가 성공하면 결정 9의 두 안(남김·지움) 모두에서 참이다. 그 쓰기가 실패하면 `Save.Why.Failed`가 알리고, 문이 닫힌 세션(`-Seed=`·`-NoSave`)의 '새 판'은 디스크에 닿지 않아 다음 보통 시작이 옛 판을 잇는다(§11) |

새 까닭 줄은 17·21·23·18자로, 지금의 셋(11~20자)보다 셋까지 길다(`Save.Why.Backup` 21자, `Save.Why.Unreadable` 23자). 상단 바에서 줄어드는 칸은 연구 몫 하나뿐이라(`Fill`과 말줄임 — `UI/CosmicTopBarWidget.cpp:123-132`) 긴 까닭 줄은 연구 몫이 '…'로 줄어 받는다. 손 확인 ⓐ 4가 1280×720 창에서 본다. **규칙 11에 비춘 점검**: `words.verdict`의 일곱 낱말이 없다. 숫자가 없다(사람·천체·자원의 수, 주기도 없다). 하나님은 화자도 조건도 아니다. 모두 UI 안내이고 서술자 문장도 새 DT 행도 아니다. '판'·'저장'은 게임의 말이고 끝이나 운명을 말하지 않는다. **검사기**: 키와 같은 A-5c 커밋에서 `schema.json`의 `string_table.checked_prefixes`에 `"Save.": {"forbidden": "verdict", "no_digits": true}`를 더한다(검사기 코드는 그대로, 단위 테스트 74 그대로).

### 5.6 판 길이를 재는 줄
`FollowEnd`의 `CosmicEnd:` 줄(`:628`)에 초를 세기 시작한 틱을 더한다: `CosmicEnd: <까닭> at tick <틱>; <초> s since tick <n>, <초> s of them stopped`. 컨트롤러가 `RunBeganRealSeconds`를 두는 두 자리(`BeginPlay`·`HandleGalaxyGenerated`)에서 private `RunBeganTick`(초기값 −1)도 둔다. 새 판은 0이고 이어 연 판은 이어 연 틱이라, 세션을 나눈 판도 줄 하나로 읽힌다. **이미 끝난 판을 열어** 래치가 서면(`bArrived`가 거짓이고 `RunBeganTick`이 −1이거나 지금 틱) 초를 세지 않은 줄 `CosmicEnd: <까닭> at tick <틱>; opened ended (continued)`를 남긴다. 판 길이의 근거는 판이 실제로 끝난 세션의 로그에 있다(다시 띄운 뒤에는 `Saved/Logs/Cosmic_Ascension-backup-*.log`). A4 결정 1의 '30~60분' 판단은 이 줄과 `CosmicSave:`·`CosmicContinue:` 줄을 함께 읽는다. 세션을 넘는 판의 초는 저장하지 않는다(새 필드를 만들 만큼의 값이 아니다).

## 6. 연대기 이관과 상한 (#40 ⑦ — A4 §4.4 ①~④)

### 6.1 `UCosmicLogSubsystem::Append`의 넘침 고리 (`Log/CosmicLogSubsystem.cpp:206-216`를 바꾼다)
- 넘칠 때 한 번 `Peoples = CosmicChronicle::PeoplesOfEveryLife(State.Calling)`을 만들고, 머리에서 떠나는 줄마다 `Admits(줄, Peoples)`이면 `Chronicle`로 옮기고 아니면 버린다.
- 옮긴 줄이 있으면 고리 뒤에 한 번 `TrimChronicle(State.Chronicle, State.ChronicleEviction, Sim->GetRules().ChronicleCap)`(Log Subsystem의 private static, 본문은 구현 메모 §4).
- `Append`가 `Log`·`Chronicle`·`ChronicleEviction`의 유일한 쓰는 곳으로 남는다. 이관은 틱 단계 안(Push 안)에서도 일어나지만 이야기 층이 이야기 층에 쓰는 것이고, 추첨도 세계 읽기도 없다.
- 지금 규칙과 다른 점 둘: **다른 백성에 관한 S1 줄과 초점 백성이 없는 S1 줄은 버린다**(첫 접촉 `log_first_contact`는 남긴다). Severity에 상관없이 모든 `Log.Calling`을 남긴다(지금 두 행은 S2라 실제로 달라지는 것은 없다).

### 6.2 `Log/CosmicChronicle.h`에 더하는 순수 함수 둘 (파일 머리의 '아무것도 쓰지 않는다'는 그대로)
`MayLetGo`는 A-5a에 둔다(커맨드렛의 `kept for good`이 읽는다). `PeoplesOfEveryLife`는 A-5b다.
```cpp
	/** Every life's peoples: PeoplesOf of each FLifeRecord of Calling.Lives, in life order, without repeats (MVP item 5's move
	 * filter, 16 #40 (7)); empty when there is no life. */
	COSMIC_ASCENSION_API TArray<FName> PeoplesOfEveryLife(const FCallingState& Calling);

	/** Whether the chronicle's cap may let Entry go: Severity exactly 1 and none of Log.Calling, Log.Claim, Log.Record
	 * (headers/A4 4.4 (2), Law.Judgment). Severity 2 and the three categories are never removed. */
	COSMIC_ASCENSION_API bool MayLetGo(const FCosmicLogEntry& Entry);
```

### 6.3 상한의 뜻 (결정 15)
- 옮긴 뒤 연대기 **전체**가 `Cap`을 넘을 때만, 가장 오래된 `MayLetGo` 줄부터 나간다. 한 번 훑는 안정된 거르기(남는 줄의 차례 그대로 — 꽉 찬 상한에서 `RemoveAt`을 되풀이하면 n²이다)다. `Count`는 나간 수만큼 늘고 `ThroughTick`은 나간 줄 가운데 가장 늦은 틱이 된다.
- 지울 수 없는 줄만 남으면 상한을 넘어 자란다(S2는 지우지 않는다). 그 수는 A-5a가 EA에서 잰다(§1.3, §11).
- `Cap` 0 이하 = 상한 없음(`ProvisionalEndTick` 0 = 꺼짐과 같은 관례).

### 6.4 `BuildCapped`의 걸음 (3b) (`Log/CosmicChronicle.cpp:202` 뒤, (3)과 (4) 사이)
`State.ChronicleEviction.Count > 0`이고 `Chapter.FromTick <= ThroughTick`이면 `Chapter.bMore = true`. 화면에는 이미 있는 `Chronicle.More`("그 외에도 있었다.")가 나오고 수는 보이지 않는다(규칙 11, `docs/70:163`). 밀어냄 기록은 어느 생애의 줄인지 모르므로, 범위가 닿는 장은 모두 이 줄을 단다 — 사실보다 많이 말할 수는 있어도 덜 말하지는 않는다. `CosmicChronicle.h:28`의 `bMore` 주석도 고친다.

### 6.5 남는 것과 잃는 것
- **생애가 하나인 동안(MVP 전체)** 모든 생애의 백성은 `PeoplesOf(Lives[0])`와 같고, `BuildCapped`는 `Admits`가 받지 않는 줄을 되풀이 규칙보다 먼저 건너뛴다(`.cpp:145`). 그래서 **연대기 쪽은 거르기 전과 줄 하나 다르지 않다.** `Log.CapAndChronicle`이 단언하고, 커맨드렛의 `chronicle life 1 shown 40 of 63`(A4:129)도 20,000틱에서 그대로여야 한다.
- **잃는 경우 하나**: 한 생애가 *나중에* 다스리게 된 백성에 관해 그 앞에 쓰인 S1 줄은, 그 전에 로그를 떠났다면 돌아오지 않는다. 즉위(`BecomeRuler`, Phase 4)부터 생긴다. 세이브에서 버린 줄은 되돌릴 수 없으므로 Phase 4 즉위 헤더와 Phase 4b 다이제스트 헤더가 이관 범위를 다시 본다(`OPEN_QUESTIONS`에 적는다).
- 두 책(B5)이 읽을 `Log.Claim`·`Log.Record`는 언제나 남는다.

## 7. 데이터·노브·검사·커맨드렛

**`Data/CosmicDefinitionRows.h`**, `ProvisionalEndTick`(988행) 뒤, 구조체 닫기(989행) 앞:
```cpp
	// ---- The saved chronicle (MVP item 5) ----

	// The cap of FGalaxyState.Chronicle (docs/70 section 8.4, 16 #40 (7)): after a move out of the log, while it holds more
	// entries than this, its oldest Severity 1 entries leave - never Severity 2, Log.Calling, Log.Claim or Log.Record, so the
	// chronicle may hold more. FGalaxyState.ChronicleEviction records it and the chapter says Chronicle.More. 0 or below = no
	// cap. Story only: no world rule reads it and the world hash does not move with it. Start value 1500: a run filled to the
	// cap, while the lines that cannot leave stay under it, saves about 2.2 MB before packing with the log's 2,000 (docs/20
	// section 9.4); the lines that cannot leave may grow past it. Not a display cap - CosmicChronicle::MaxLifeLines is.
	UPROPERTY(EditAnywhere)
	int32 ChronicleCap = 1500;
```

**데이터와 검사**
- `Tools/data/DT_Rules.json`: `"ProvisionalEndTick": 6000` 뒤에 `"ChronicleCap": 1500`(A-5b).
- `Tools/data/schema.json`: DT_Rules 열 끝(137행) 뒤에 `"ChronicleCap": {"kind": "number", "min": 0, "integer": true}`(A-5b). `string_table.checked_prefixes`의 `"Save."`는 키와 함께 A-5c다(§5.5).
- `Data.Integrity`의 `CheckColumnsOf`(`Tests/CosmicDataTests.cpp:316-342`)가 구조체와 스키마를 두 방향으로 잰다. `Tools/test_data_schema.py`는 DT_Rules 열 이름을 적지 않으므로(일반 `Knob` 사례, 377-380·636행) 그대로다 — A4의 `ProvisionalEndTick`과 같다.
- **에셋 다시 만들기**: A-5b는 **구조체 필드가 생기므로 먼저 빌드하고 나서 `-Data`**(DT_Rules). A-5c는 ST_CA 키와 스키마 접두어가 바뀌므로 다시 `-Data` — 도장 `Tools/data/generated.sha1`에 `ST_CA.csv`·`schema.json`이 들어 있어 `Data.SourcesInSync`(`Tests/CosmicDataTests.cpp:1035-1080`)가 본다. `docs/30`은 `make_config_reference.py`로 다시 만든다: 노브 53 → 54, 읽힘 35 → 36(`Log/CosmicLogSubsystem.cpp`가 읽는다), 예약 18 그대로.
- **레이어 린트**: `Append`의 static `ClaimTag`·`RecordTag`(187-188행)를 지우므로 태그 글자 수가 41 → 39가 된다. A-5b가 `Tools/check_layers.py:24`의 `BASELINE`과 그 주석을 고친다.
- **자동 저장 간격은 노브가 아니다**(`UCosmicSaveSubsystem::AutosaveRealSeconds`): 상태를 바꾸지 않는 입출력 일정이라 규칙 데이터가 아니다.

**커맨드렛**(`Tests/CosmicBalanceCommandlet.cpp`)
- `SaveBytesOf`(285-295)가 `CosmicSaveSlot::Encode(*Save, 0, Slot, &Raw)`를 지나 잰다(#70, `docs/20:249`). 돌려주는 값은 슬롯 바이트와 압축 전 바이트, 시간은 복사 + 직렬화 + 압축 + CRC다(파일 쓰기와 내리기는 들지 않는다 — 게임의 `CosmicSave:` 줄이 그것까지 잰다).
- **`SaveBytes` 열의 뜻이 압축 전 → 슬롯 바이트로 바뀐다.** 옛 CSV(`a3_20k.csv`)와 견줄 때는 새 열 `SaveRawBytes`(A-5a)를 본다. 새 열 둘(`SaveRawBytes`·`ChronicleLetGo` — 뒤의 것은 A-5b)은 `ChronicleLen` 뒤, `Expansion` 앞에 둔다(`Expansion`은 끝이어야 한다 — `:309-310`). `Tools/`에 이 열을 읽는 곳은 없다.
- 요약 줄(646행)의 저장 몫은 A-5a에서 `save %d bytes (%lld before packing) in %.1f ms; log %d, chronicle %d (%d kept for good), told %lld`, A-5b에서 `told` 앞에 `let go %d, `를 더한다(압축 전 길이는 `int64`라 `%lld` — UE_LOG 꼴은 컴파일 때 검사된다). 이 줄은 기준값 비교 대상이 아니다(`run_checks.ps1`은 `CosmicBalance: seed`와 `ruler:` 줄만 본다). `kept for good` = 연대기에서 `!MayLetGo`인 줄 수.
- A-5a의 측정 몫으로 마지막 표본의 엔진 바이트를 Kraken SuperFast·VeryFast·Fast로 따로 압축해 크기와 ms를 한 줄(`CosmicBalance pack:`)로 찍는다(구현 메모 §5·§7).

## 8. 테스트 (새 단언은 변이로 한 번씩 깨뜨린다)
새 테스트 8종으로 106 → **114종**이다(A-5a +4, A-5b +1, A-5c +3). 새 저장 테스트는 이름이 `Sim.Save*`라 smoke 묶음(`Cosmic_Ascension.Sim.Save`)에 든다. 디스크에 쓰는 테스트는 모두 `Saved/SaveGames/Automation/`에만 쓰고(§3.7 — `Sim.SaveFind`는 `Automation/Find/`), **슬롯 이름과 경로는 모두 `SlotNameOf`·`SlotPathOf`로 만든다**. 고정 GUID를 쓰는 테스트는 시작과 끝에 제 슬롯을 지우고(`DeleteSave`), 심은 파일(미끼 이름)은 경로로 지운다(`DeleteSave`는 그것을 지우지 않는다). 읽기 전용·잠금은 `ON_SCOPE_EXIT`에서 되돌린다. `Automation/`을 비우지는 않는다(두 프로세스에 걸친 `Sim.SaveRestart`의 슬롯이 그 안에 있다). 맵을 다시 여는 테스트는 `FWaitForMapToLoadCommand` 대신 새 월드를 기다리는 제 지연 명령을 쓴다(§1.2). 단계·값·변이 목록은 구현 메모 §6.

| 테스트 | 단언 | 변이 |
|---|---|---|
| `Sim.SaveSlotForm`(새, 디스크 없음) | **머리**: 첫 낱말·꼴 1·코덱 Oodle·비운 바이트 0·GalaxyID·시각·Tick·`RawBytes` = 같은 운반 객체의 `SaveGameToMemory` 길이·길이 = 64 + `PackedBytes`·`PackedBytes < RawBytes` · **풀기**: 풀린 바이트 = `SaveGameToMemory` 출력, `LoadState`한 해시 = 묶기 전 해시 · Stored 길의 왕복 · **거절과 그 까닭**(단언 하나에 경우 하나): 빈 것·63바이트 `Short` / 머리만·1바이트 짧음·1바이트 김 `Truncated` / 엔진 꼴 `OldForm` / 다른 첫 낱말 `NotOurs` / 꼴 2(CRC 다시 셈) `NewerForm` / 꼴 2(CRC 그대로) `HeaderDamaged` / 꼴 0(다시 셈) `BadSizes` / 머리 `Tick` 바이트 뒤집음 `HeaderDamaged` / 코덱 9·`RawBytes` 65 MB(다시 셈) `BadSizes` / 다른 이름 `OtherGalaxy` / 페이로드 가운데 바이트 뒤집음 `PayloadDamaged` / **Stored 슬롯을 `GalaxyID` 태그가 시작하는 자리(`TellRngState` 값의 끝)에서 자름** `Truncated`(#70 — 속성 안에서 잘린 바이트가 엔진 리더에 닿지 않는다) · **`ParseSlotName`**: 대문자·소문자 39·43자 받음, 31자리·`_tmp`·충돌 사본 이름 거절 | 5 |
| `Sim.SaveBackup`(새) | 첫 저장에는 백업이 없다 · 둘째 저장 뒤 백업의 머리 `Tick` = 첫 저장의 틱 · 주를 반으로 자르면 백업에서 열리고(그 해시) Warning 한 줄 · **상한 주는 백업으로 옮기지 않는다**(다음 저장 뒤에도 백업 틱 그대로) · 주가 다른 은하의 바이트면 백업 · 둘 다 상하면 거절, 해시 그대로 · **주가 새 버전(`NewerSave`)·새 꼴(`NewerForm`)이면 거절이고 백업으로 가지 않으며, 새 꼴의 주는 저장이 덮지 않는다** · **주를 잠그면(`ReadFailed`) 로드는 백업으로 가지 않고 저장은 false, 주 그대로** · **백업 쓰기가 실패하면(읽기 전용 `_bak`) false이고 주는 그대로** · `DeleteSave`는 둘 다 지운다 · 경우마다 Warning 수를 센다 | 6 |
| `Sim.SaveFind`(새, 제 폴더 `Automation/Find/`) | **`NewestRun`**(순수): 늦은 시각이 먼저 · 판의 시각은 주와 백업 중 늦은 것 · 머리가 상한 슬롯은 파일 시각으로 든다 · 같은 시각은 GUID 글자순 뒤 · **디스크**(고정 GUID 셋, 지난 시각): `Newest` / 가장 새 판의 주가 페이로드에서 잘림 → `Backup` / 주·백업의 페이로드가 다 상함 → `Unreadable`이고 옛 판으로 내려가지 않으며 파일은 그대로 / 가장 새 판의 두 파일이 0바이트 → `Unreadable`(옛 판을 열지 않는다) / 이름 거름: 더 새 머리를 가진 다른 GUID의 `_tmp`·충돌 사본, `Galaxy_123`·`Other` → 고른 판이 그대로 · 게임 폴더(`Saved/SaveGames/*.sav`)의 목록이 앞뒤로 같다 · 판의 절대 수는 단언하지 않는다 | 4 |
| `Sim.SaveNeutral`(새) | 슬라이스 2,000틱: 100틱마다 `SaveCurrentGalaxy`를 한 판과 하지 않은 판의 전체 해시·세계 해시·두 스트림 위치가 같다 · 저장 직전과 직후의 해시가 같다 | 2 |
| `Log.ChronicleCap`(새) | `WithRules` 상한 10: 섞은 줄 묶음(S1 24·지울 수 없는 줄 6)을 **먼저** 밀고 S0 채움 2,000을 뒤에 밀어 묶음만 로그를 떠나게 한다 · 넘친 뒤 `Chronicle.Num() == 10` · 지울 수 없는 줄은 모두 남는다 · 남은 S1은 가장 새것들 · `Count` = 나간 S1 수 · `ThroughTick` = 나간 것 가운데 가장 늦은 틱 · 차례 그대로 · 상한 3(< 지울 수 없는 줄 수)이면 S1이 모두 나가고 `Num() > 3` · 상한 0이면 아무것도 나가지 않는다 · **상한 0과 10의 세계 해시가 같고 전체 해시는 다르다** | 5 |
| `Sim.SaveTriggers`(새, 문을 연 테스트) | 새 은하 → `RequestSave(NewRun)` 1 · 바뀐 것 없는 5틱 0 · 업적 하나 → 1(`Ascended`) · 역할 바꿈 → 1(`Role`) · `ProvisionalEndTick − 1`에서 한 틱 → 1(`Ended`) · 간격 0 → 1(`Interval`) · 업적과 역할이 한 경계에 → 1(`Ascended`) · 바뀐 것 없는 `RequestSave(Exit)` 0, 바뀐 뒤 1 · `OnLogEntry` 핸들러 안의 `SaveCurrentGalaxy`는 Error 1개로 거절, 같은 자리의 `RequestSave(Answer)`는 다음 경계에서 1 · **실패**(깃발은 경계 없는 입력으로 본다 — 산 컨트롤러가 같은 방송에서 깃발을 가져갈 수 있다, §4.5): 입력 때의 `RequestSave(Answer)`가 실패하면 0이고 `TakeFirstFailure`는 참 다음 거짓 · 파일을 되돌려도 기다림 안의 경계는 0 · 기다림이 풀린 경계가 남은 `Answer`를 1 · 경계의 실패는 간격을 1e9로 돌린 뒤에도 남은 까닭(`Interval`)으로 1 · **결정 6의 건너뜀은 그 판 하나**: `HoldStartRun` 뒤 `RequestSave(Exit)` 0, 다른 새 판의 `Exit` 1 · **문의 순수 함수**: `CommandLineAllows`(`-NoSave`·`-unattended`·`-Seed=42` 거짓, 셋 다 없으면 참)와 `GateClosedBy`의 까닭마다 한 경우 · 문이 닫히면(`SetGameSavingForTest(false)`) 어느 것도 쓰지 않는다 · `DueAt` 경우마다 순수 단언 | 7 |
| `UI.ContinueEnded`(새, 지연 명령 — 맵 다시 열기) | 끝난 상태(틱 = `ProvisionalEndTick`, 대기 카드 있음)를 쓰고, 문을 열고, `ContinueAtNextStartForTest` 뒤 맵을 다시 열고 새 월드를 기다린다: GalaxyID·전체 해시 = 쓴 것 · 래치 · 쪽이 열리고 다 드러남 · 카드 없음(`PendingPetitionRow`는 앎에 그대로) · 배속 0·까닭 `RunEnd` · 이어 연 정지 줄 없음 · `CosmicContinue: Newest` | 2 |
| `UI.ContinueRunning`(새, 지연) | 흐르던 상태(대기 카드 있음) → 맵 다시 열기 → 해시 = 쓴 것 · 배속 0·까닭 `Continued` · 카드가 선다 · **주를 읽기 전용으로 두고 답(`AnswerPetition(1)`) → 쓰기 실패 → 배속 0·까닭 `SaveFailed`, 되돌린 뒤 한 경계 → 그 답이 쓰인다(`GetLastCause() == Answer`, 슬롯 머리의 `Tick` = 지금 틱)** · Space → ×1이고 줄이 사라진다 | 3 |
| `Log.CapAndChronicle`(고쳐 씀, `Tests/CosmicLogTests.cpp:140-215`) | 기대값을 테스트 자신의 식으로 센다(`Admits`를 부르지 않는다). S1을 그릇의 백성·다른 백성·초점 없음·`log_first_contact`로 섞어 민다 · **다른 백성의 S1은 연대기에 없다** · 그릇 백성의 S1과 첫 접촉은 있다 · 둘째 생애를 손으로 더하면 첫 생애 백성의 S1도 남는다 · **쪽은 아무것도 떠나지 않은 것처럼 읽힌다**(민 줄을 모두 손으로 모은 참조 상태와 `BuildCapped`의 줄·`Candidates`가 같다) · 차례·'평범한 것 없음'은 그대로 | 3 |
| `Log.ChronicleBuild`(넓힘) | 생애 둘(둘째 `BornTick` 500): 밀어냄 기록 {1, 300}이면 첫 장만 `bMore`, {1, 500}이면 둘 다(경계), {0, 900}이면 둘 다 아님 | 3 |
| `Sim.SaveRoundTrip`(고침, `CosmicSimTests.cpp:1680-1844`) | 새 감싸기를 지난다 · 엔진 머리 끝 자름 → 슬롯 자름 여럿(백업 없음 → 거절) · 옛 꼴 슬롯 거절 · 다른 은하·없는 슬롯·새 버전은 그대로 · **나중 멤버** `ChronicleEviction`을 초기값 밖에 두고(`FillEmptyContainers`가 {3, 17}로 둔다 — 값을 두는 함수는 그 하나) 로드 뒤 필드별 비교(새 도우미는 비교 `SameLaterMembers` 하나, 25의 셈은 그대로) · **0인 밀어냄 기록은 기록되지 않는다**(풀린 엔진 바이트에 `ChronicleEviction` 이름이 없고, 세우면 있다) · 모든 직접 호출이 `SlotNameOf`·`SlotPathOf` · 기대 경고 문구를 새 줄로 | 2 |
| `Sim.TellStreamSeparate`(고침, `:1617-1649`) | 최상위 **19**를 분류한다 · `ChronicleEviction`은 전체 해시 안·세계 해시 밖 | 2 |
| `Sim.NotablesFromEventsOnly`(고침, `CosmicHazardTests.cpp:2541-2575`) | 20,000틱 전부 말하기에서 다스리지 않은 백성의 S1 `NameNotable` 줄(`event_rebellion_rises`, S1)은 이제 버려진다 → 단언을 '찾았거나, 그 틱이 `Log[0]`의 틱 이하이고 그 백성이 어느 생애의 백성도 아니다'로 고친다 | — |
| `Sim.SaveRestart`(고침 — '다른 빌드의 기록' 가지) | 게임 폴더의 옛 슬롯도 지운다(§3.7) | — |
| `Events.RowAtomic`(넓힘, `CosmicEventTests.cpp:1719` 옆) | `OnEventFired` 핸들러의 `SaveCurrentGalaxy`는 거절되고 슬롯이 생기지 않는다 | 1 |
| `UI.AutoPause`(넓힘, `CosmicAutoPauseTests.cpp:156`) | 까닭 고리에 새 넷 → 각자의 ST_CA 키 | 1 |
| 그대로여야 할 것 | `Sim.SavePetition` · `Sim.SaveSanctuary`(성역 거절은 백업으로 가지 않는다, Error 5 그대로) · `Sim.SightGrowsOnly` · `UI.RunEnd` ⑦ · `Sim.KnowingInert` · 세계 해시 셋 | — |

**테스트가 덮지 않는 것**: 종료 저장(`EndPlay(Quit)` — 테스트 프로세스는 `Quit`으로 끝나지 않는다)과 시작의 `NoSlot`·`Unreadable` 가지(테스트 프로세스의 첫 시작은 문이 닫혀 있다). 결정 6의 종료 건너뜀 자체는 `Sim.SaveTriggers`가 `HoldStartRun`과 `RequestSave(Exit)`로 본다. 손 확인 ⓐ의 1·3·8이 본다(스모크의 저장은 '추가 이야기').

## 9. 문서
고칠 문서와 코드 주석의 목록은 구현 메모 §9로 옮겼다(커밋마다 — A-5a·A-5b는 그 커밋에서 코드보다 먼저, 나머지는 A-5d). A4 헤더는 승인된 기록이라 고치지 않는다.

## 10. 순서·검증·effort
새 USTRUCT와 헤더 변경이 있으므로 단계마다 에디터를 닫고 전체 빌드한다. 커밋마다 `run_checks.ps1 -Tier full`이 두 프로세스에서 연속 2회 통과해야 한다(`SaveRestart`가 두 번째를 쓴다). **effort**(결정 18): 권장대로 A-5a~A-5c는 high에 ultracode 끔이다 — STATUS '다음 3작업' 3('구현 high에 ultracode 끔')과 MVP ①~④가 그렇게 했고, 헤더의 적대 리뷰는 이미 돌았으며, 구현 뒤 ultracode 리뷰 1회(결정 17, 아래 5단계)가 남는다. 구현 리뷰는 xhigh에 켬, A-5d(문서)는 `WORKPLAN` §2 '설계 문서 변경' 행(65행)대로 xhigh다. 대안을 고르면 A-5a~A-5c도 xhigh다 — `WORKPLAN.md` §2 Phase 표의 '저장 작업' 행이 xhigh이고(59행), Phase 표와 작업 유형 표가 부딪치면 Phase 표가 이긴다(69행의 제목. 세션의 effort는 그 Phase 행에서 읽는다 — `CLAUDE.md` 작업 방식). 그때는 A-5d가 STATUS '다음 3작업' 3의 '구현 high'를 고친다. ultracode 명령: 켬 `/effort ultracode on`(그 턴만이면 프롬프트에 `ultracode`), 끔 `/effort ultracode off`(`WORKPLAN.md:31` — `/ultracode` 명령은 없다).

| 단계 | effort · ultracode | 명령 | 하는 일 |
|---|---|---|---|
| 0. 시작 전(개발자) | low · 끔 | `/effort low`, `/effort ultracode off` | `DECISIONS_LOG.md`의 커밋하지 않은 바깥 수정(빈 첫 줄, 지워진 2026-10-19 A0 항목)을 남길지 되돌릴지 정한다. ⑤의 기록과 섞지 않는다 |
| 1. 헤더 적대 리뷰 → 승인 | xhigh · **켬** | `/effort xhigh`, `/effort ultracode on` → 끝나면 `/effort ultracode off` | 리뷰(2026-10-23)의 발견은 본문에 넣었고 개발자 결정만 §12에 남겼다. 결정 1~3을 먼저 본다 |
| 2. [A-5a] 슬롯·백업·찾기 | high · 끔(결정 18 대안: xhigh) | `/effort high`, `/effort ultracode off` | §3, `IsInTickStages`, 테스트 폴더(`PrepareForTests`, `AcquireSim` 한 줄, `SaveRestart`의 옛 슬롯 지움), `MayLetGo`, 커맨드렛, 테스트 넷 + `SaveRoundTrip`·`Events.RowAtomic`·`SaveRestart`, `docs/20` §9.1의 쓰는 함수 문장(먼저). **측정**(CHANGELOG에 적는다): 시드 42 전부 말하기 6,000·20,000틱, 정책 줄 3,000틱, `-TellAll=0` 20,000틱, `-Systems=300 -Civs=48` 전부 말하기 20,000틱 — 슬롯·압축 전 바이트, ms(압축만 SuperFast·VeryFast·Fast 따로, 직렬화, 게임의 쓰기와 내리기), `kept for good`. **멈춤 규칙(두 쪽)**: EA 20,000틱의 슬롯이 1 MB를 넘으면 ms가 100 안에 머무는 동안만 수준을 올린다. 크기와 시간을 함께 맞출 수 없으면 멈추고 묻는다(비동기 쓰기 — '추가 이야기'). full 2회, 기준값 그대로 |
| 3. [A-5b] 거르기·상한·밀어냄 기록 | high · 끔(대안: xhigh) | 같음 | `docs/20` §9.1·§9.2·`docs/21`·CLAUDE.md 규칙 6·`GalaxyState.h:19`(먼저), §2, §6, §7(`"Save."` 접두어 빼고), 린트 기준 41 → 39. **먼저 빌드하고 나서** `run_checks.ps1 -Data`. 테스트 여섯(새 1 + 고침 5) → full 2회 → `-UpdateBaseline`. 커밋 메시지에 새 전체 해시 셋(3,000틱 지금 `2941370122`, 정책 줄 `2361069549`, 20,000틱 `2410091205` — 20,000틱은 손 측정이라 `seed42.json`의 기록 칸과 STATUS에 손으로 적는다)과 '세계 해시 셋 그대로'(20,000틱은 손 측정 `885018087`)를 적는다. 세계 해시가 하나라도 움직이면 설계가 틀린 것이니 멈춘다 |
| 4. [A-5c] 자동 저장·이어하기·화면 | high · 끔(대안: xhigh) | 같음 | §4, §5, ST_CA 넷과 고친 키 하나·스키마 접두어 `"Save."`, `FScopedProbe`를 `Tests/CosmicTestHelpers.h`로 옮김(구현 메모 §6.1), 테스트 셋 + `UI.AutoPause`, `Sim/CosmicEndings.h:29`의 주석(읽는 곳에 저장 층). **먼저 빌드하고 `run_checks.ps1 -Tier full -Data`**(ST_CA 에셋과 도장). full 2회, 기준값 그대로 |
| 5. 구현 리뷰(결정 17) | xhigh · **켬** | `/effort xhigh`, `/effort ultracode on` → 끝나면 `/effort ultracode off` | 자름·백업 차례·내리기·핸들러 가드·문·이어하기 경로·테스트 폴더를 적대적으로 검증한다 |
| 6. 손 확인 ⓐ(개발자) | — | `Saved/SaveGames/`를 비운 뒤, `-Seed=`·`-NoSave`·`-unattended` 없이 `-game -windowed`(또는 에디터의 Play ▸ Standalone Game) | 아래 목록 |
| 7. [A-5d] 문서 | xhigh · 단독, 끝에 재감사 1회 | `/effort xhigh`, 쓰는 동안 `/effort ultracode off`, 재감사 때 `/effort ultracode on` | 구현 메모 §9의 A-5d 몫. `WORKPLAN` §2 '설계 문서 변경'(65행)의 재감사 |

**손 확인 ⓐ 목록**(로그는 `Saved/Logs/Cosmic_Ascension.log`)
1. 저장 없이 시작하면 새 판이 ×1로 흐르고 `CosmicSave: wrote Galaxy_<id> (NewRun) tick 0`이 선다.
2. 첫 청원(30틱)에 답한다 → `(Answer)` 줄. ×10으로 첫 위기(150틱)를 지난다. 주기·다스리는 백성·피드의 마지막 줄을 적어 둔다.
3. 창을 닫는다 → `(Exit)` 줄과 `Galaxy_<id>_bak.sav`.
4. 다시 띄운다 → 같은 은하·같은 주기, '정지 — 하던 판을 이어 열었다', 같은 피드와 대기 카드, **`CosmicSave:`의 해시 = `CosmicContinue:`의 해시**. 1280×720 창에서 까닭 줄이 상단 바 안에 드는지 본다. Space → ×1, 줄이 사라진다.
5. 끝까지 간다(셋째 칸, 또는 ×10으로 6,000까지 약 10분). 쪽에서 창을 닫고 다시 띄운다 → 기다림 없이 다 드러난 쪽, 카드 없음, 배속 0. 쪽의 '새 판' 두 번 → 새 슬롯, 옛 슬롯은 남는다. 끄고 켜면 새 판이 이어 열린다.
6. (고를 수 있음) 답한 직후 작업 관리자로 끝내고 다시 띄운다 → 답이 남아 있다.
7. `CosmicSave:`·`CosmicContinue:`·`CosmicEnd:` 줄을 `CHANGELOG`에 한 줄로 적는다(A4 결정 1의 판 길이 근거). `CosmicEnd:`의 초는 판이 끝난 세션의 로그에서 옮긴다(다시 띄운 뒤에는 `Cosmic_Ascension-backup-*.log` — 이어 연 끝난 판의 줄은 `opened ended`다).
8. (고를 수 있음, 결정 6) 창을 닫은 뒤 그 판의 두 파일을 다른 폴더로 옮기고, 같은 이름의 빈 파일 둘을 둔 채 띄운다 → '정지 — 저장을 읽지 못해 새 판을 열었다'. 곧바로 닫고 원래 파일을 되돌려 띄우면 옛 판이 이어 열린다.

## 11. 확인하지 못한 것
- **압축률과 저장 시간**: `FObjectAndNameAsStringProxyArchive`가 이름을 글자로 되풀이해 쓰므로 잘 줄 것으로 보지만, 최악이 1 MB 아래로, 복사 + 직렬화 + 압축 + 읽기 하나·쓰기 둘·내리기 둘이 100 ms 아래로 드는지는 A-5a의 측정이 정한다. 개발 PC의 `Saved/`는 OneDrive 안이라 내리기의 시간이 보통 디스크와 다를 수 있다.
- **지울 수 없는 줄의 수**: 긴 판에서 연대기가 상한을 넘어 계속 자랄 수 있다. 크면 B5의 `Log.Claim` 묶음 키(#31)를 당긴다.
- **지연 테스트의 맵 다시 열기**: 이 프로젝트의 첫 지연 테스트 둘이다. `-game -nullrhi`의 대기열 안에서 맵을 다시 연 뒤 뒤의 UI 테스트가 새 컨트롤러·HUD를 잡는지(`AcquireSim`은 매번 `GetFirstPlayerController`로 찾는다)는 돌려 보지 않았다. 흔들리면 두 테스트만 따로 거르는 줄을 `run_checks.ps1`에 두거나, `UI.ContinueRunning`을 맵 다시 열기 없이(산 컨트롤러에서 `BeginSession` + `FollowStart`) 바꾼다 — 그러면 `BeginPlay`의 `FollowStart` 부름을 지우는 변이는 잡히지 않는다.
- **Oodle 풀기의 `check(DecodeSize == InUncompressedSize)`**(`OodleDataCompression.cpp:564`)가 실패가 아닌 다른 크기로 닿을 수 있는지는 Oodle 바이너리 안의 일이라 볼 수 없었다. 우리 길에서는 CRC가 앞에 서므로 우리가 쓴 바이트만 닿는다. 그래서 테스트는 CRC를 맞춰 `RawBytes`를 속이는 슬롯을 만들지 않는다.
- **OneDrive**: 동기화가 `.sav`를 잡으면 읽기나 쓰기가 실패할 수 있다(보지 못했다). 읽기는 `ReadFailed`(백업으로 가지 않는다), 쓰기는 실패 줄과 60초 뒤 다시 쓰기가 받는다. 충돌 사본은 이름 거름이 뺀다.
- **파일 시각의 대체값**: 머리가 읽히지 않는 슬롯은 파일 시각으로 차례에 든다. 동기화가 옛 판의 상한 파일 시각을 올리면 그 판이 가장 새것으로 보여 `Unreadable`이 된다(옛 판을 열지는 않는다).
- **패키지 게임의 저장 폴더**(`FPaths::ShouldSaveToUserDir`, `Paths.cpp:183-192`)와 다른 플랫폼의 저장 시스템: ⑨가 본다. 슬롯의 경로(`SlotPathOf`)·찾기의 `FindFiles`·머리 읽기·내리는 쓰기는 Win64의 일반 저장 시스템과 같은 폴더를 전제한다.
- **시계를 되돌린 PC**: UTC 시각으로 줄 세우므로 가장 새 판을 놓칠 수 있다(같은 프로세스 안에서는 '앞의 쓰기보다 늘 크게'가 막는다).
- **'새 판' 툴팁의 참**: 문이 닫힌 세션(`-Seed=`·`-NoSave`)에서 '새 판'을 누르면 새 판은 디스크에 닿지 않아, 다음 보통 시작이 옛 판을 잇는다. 새 판의 첫 쓰기가 실패한 경우도 같다(그때는 `Save.Why.Failed`가 알린다).
- **끝난 MVP 판의 세이브**: ⑤ 뒤 끝난 판은 세이브에 남는다. B6이 임시 끝을 바꾸거나 노브가 바뀌면 끝난 세이브가 흐르는 판으로 열릴 수 있다 — A-5d가 #117에 이 물음을 적는다(B6 헤더가 정한다).

## 12. 개발자 결정 (권장안 — 중요한 것부터) — 1~3을 먼저 본다
리뷰가 새로 올린 것은 5·6·7이고, 리뷰 뒤 권장이 바뀐 것은 2·3·10이다. 리뷰 뒤의 독립 검증이 올린 것은 18(구현 단계의 effort)이다. 번호는 리뷰 전과 다르다(옛 1→1, 2→2, 8→3, 4→4, 12→8, 3→9, 7→10, 10→11, 11→12, 9→13, 6→14, 13→15, 5→16, 14→17).

1. **이어하기의 꼴**. **권장**: 시작할 때 가장 새 판을 저절로 이어 열고, 흐르던 판은 **멈춘 채** 상단 바의 까닭 한 줄('정지 — 하던 판을 이어 열었다')로 연다. 끝난 판은 끝난 채(쪽 다 드러남, 카드 없음)다. 새 위젯·메뉴가 없고, 돌아온 사람이 보고 나서 흐르게 한다(#33 ③의 '이어하기 한 줄'). `-Speed=`가 있으면 그 배속으로 흐른다. 대안 ㉮: ×1로 바로 흐르고 줄도 없다(지금의 새 판 시작과 같다 — 돌아온 사람이 무엇이 일어났는지 모른 채 시간이 간다). 대안 ㉯: 은하보다 먼저 '이어하기·새 판' 두 단추의 시작 화면(메뉴가 생기고 `docs/50` 컷 리스트·`docs/20` §9.1과 닿는다).
2. **흐르던 판을 버리는 길**. ⑤ 전에는 다시 켜면 새 판이었지만, ⑤ 뒤에는 끝까지 가거나 파일을 지우는 수밖에 없다. **권장(리뷰 뒤 바꿈)**: 따로 두지 않는다 — 끝까지 가면 쪽의 '새 판'이 있다(×10에서 6,000까지 약 10분, 가장 단단한 아이언맨). 상단 바의 단추·확인 창·그 테스트 고리와 맵 다시 열기 하나가 빠진다(#33 ③과 A4 §4.4 ⑤가 바라지 않는 것). 개발자는 `-NoSave`로 켜면 늘 새 판이다. 대안 ㉯(초안의 권장): 이어 연 정지 상태에서만 상단 바에 '새 판'(3초 안에 두 번 — 상단 바에는 틱이 없어 확인을 되돌릴 `NativeTick`이 든다, 구현 메모 §3.4). 대안 ㉰: 상단 바에 언제나 둔다(판을 손쉽게 버리게 된다). 대안 ㉱: 개발 콘솔 명령만(테스터에게는 길이 없다).
3. **백업과 쓰는 차례, 전원이 꺼지는 경우**. **권장(리뷰 뒤 바꿈)**: 검사를 통과한 주만 `_bak`으로 → 그 쓰기가 실패하면 주를 덮지 않는다 → 주를 쓴다. 두 쓰기를 `IPlatformFile`의 손잡이로 하고 `Flush`(Win64 `FlushFileBuffers`)로 디스크까지 내린다 — 전원이 꺼져도 좋은 사본 하나가 남는다(#70의 '전원 차단'). 값은 쓰기마다 내리기 둘(A-5a가 잰다)과 엔진 저장 시스템 밖의 경로 계산(목록과 같은 Win64 전제, ⑨)이다. #70의 기본값 문장은 `SaveDataToSlot`·`LoadDataFromSlot`을 이름으로 드는데, 권장은 같은 경로에 같은 바이트를 쓰되 내리기를 더하려고 파일 API로 바꾼다(#70을 닫을 때 그 문장을 고친다). 쓴 뒤 다시 읽지 않는다. 대안 ㉮(초안의 권장): 엔진의 `SaveDataToSlot`, 내리기 없음(전원이 꺼지면 두 파일을 함께 잃을 수 있다). 대안 ㉯: 쓴 뒤 다시 읽어 견준다(1 MB 이하라 싸지만 OS 캐시를 읽을 뿐이다). 대안 ㉰: 두 슬롯을 번갈아 쓰고 머리에 일련번호(저장이 마지막 좋은 사본을 자르지 않는다 — 이름 규칙과 찾기가 바뀐다). 대안 ㉱: 임시 슬롯 → `IFileManager::Move`(대상을 먼저 지워 원자적이지 않다).
4. **개발·테스트 실행의 저장**. **권장**: 게임 저장의 문을 `-NoSave`·`-unattended`·`-Seed=`·에디터 프로세스(`GIsEditor` — 에디터 안 플레이)·자동화 테스트 중·`PrepareForTests` 뒤에 닫고, 테스트의 명시 저장은 `Saved/SaveGames/Automation/`에만 쓴다. 테스트·스모크·스크린샷 명령은 고칠 것이 없고, 테스트가 남긴 슬롯은 게임에 닿지 않는다. 대안 ㉮: `-NoSave` 하나(모든 명령에 붙여야 하고, 잊으면 테스트가 진짜 판 위에 '더 새 판'을 남긴다). 대안 ㉯: 같은 폴더에 두고 머리의 표시로 테스트 슬롯을 거른다(테스트가 표시를 속이면 샌다). 대안 ㉰: `-Seed=` 실행도 저장한다(재현 판이 다음 시작의 이어하기를 가로챈다). 대안 ㉱: 스모크만 `-SaveDir=Smoke`로 문을 열어 게임 바이너리에서 종료 저장을 돌린다(추가 이야기).
5. **주 슬롯이 없거나 읽히지 않을 때**(리뷰가 올림). **권장**: 없으면(`Missing`) 백업을 연다 — 백신·동기화가 파일을 치운 경우를 받는다. 파일 하나를 지워 한 저장을 되감는 길이 열리지만, 로컬 게임에서는 폴더를 복사해 두는 길이 이미 같은 일을 한다. 있는데 읽히지 않으면(`ReadFailed` — 잠금·입출력 오류) 한 번 더 읽고, 그래도 안 되면 백업으로 가지 않고 거절한다(그 판은 `Unreadable` — 다음 시작이 다시 읽어 본다, 결정 6). 쓰기도 읽지 못한 주를 덮지 않는다. 대안 ㉮: 주가 없으면 저장이 없는 판으로 본다(백업을 열지 않는다 — 지워서 되감는 길을 닫는다. 이어하기는 주가 있는 판만 센다). 대안 ㉯(초안): 둘 다 상함으로 보고 백업을 연다(잠금이 풀린 뒤 다음 저장이 새 주를 옛 상태로 덮는다).
6. **읽지 못한 뒤의 새 판을 언제 쓰나**(리뷰가 올림). **권장**: 가장 새 판이 열리지 않아(`Unreadable`) 새 판으로 열면 곧바로 쓰지 않는다 — 플레이어가 시간을 흘린 첫 틱 경계가 쓴다. 그 전에 끄면 종료 저장도 건너뛰어, 다음 시작이 옛 판을 한 번 더 읽어 본다(잠금처럼 지나가는 까닭이면 돌아온다). 건너뛰는 것은 그 판 하나다 — 시작이 `HoldStartRun`으로 적은 GalaxyID. 같은 세션에서 '새 판'으로 연 판은 곧바로 쓰고, 그 쓰기가 실패해도 종료가 다시 쓴다. 대안 ㉮(초안): 곧바로 쓴다(그 뒤로는 새 판이 늘 더 새것이라 옛 판은 이어하기로 돌아오지 않는다. 파일은 남는다).
7. **새 빌드가 쓴 슬롯**(리뷰가 올림). **권장**: 봉투(첫 낱말·꼴·GalaxyID·쓴 시각·머리 CRC의 자리 — §3.1)를 모든 꼴에 고정하고 CRC를 꼴보다 먼저 본다. CRC가 맞고 꼴이 이 빌드보다 높으면 `NewerForm` — `NewerSave`처럼 거절하고 백업으로 가지 않으며, 쓰기는 그 주를 덮지 않고, 이어하기는 그 판을 보고(봉투의 시각) `Unreadable`로 연다. 꼴 바이트의 비트 뒤집힘은 CRC가 먼저 잡아 상함(백업)이다. 대안 ㉮(초안): 꼴을 CRC보다 먼저 보고 다른 꼴을 상함으로 본다(새 빌드에서 옛 빌드로 돌아오면 옛 백업을 말없이 열고, 다음 저장이 새 꼴의 주를 옛 상태로 덮는다).
8. **이어하기가 고르는 판**. **권장**: 머리의 `SavedAtUtcTicks`가 가장 큰 판 하나(주, 상하면 백업. 머리가 읽히지 않는 슬롯은 파일 시각을 차례에만 쓴다). 그 판이 열리지 않으면 새 판을 멈춘 채 열고 파일은 둔다 — 옛(대개 끝난) 판으로 내려가지 않는다. 대안 ㉮: 열릴 때까지 옛 판으로 내려간다('그 앞의 판'이 열려 낯설다). 대안 ㉯: 파일 시각만(`IFileManager::GetTimeStamp` — 복사·동기화가 바꾼다). 대안 ㉰: 마지막 판을 가리키는 작은 슬롯(맞춰 둘 파일이 하나 는다).
9. **'새 판' 뒤 옛 판의 슬롯**. **권장**: 남긴다. 저절로 지우는 일은 되돌릴 수 없고, 찾기의 버그가 엉뚱한 판을 지울 수 있다. 메뉴가 없어 다시 열리지는 않는다(판당 0.3~1 MB. 이어하기는 머리 64바이트만 읽어 판이 쌓여도 시작이 느려지지 않는다). 대안 ㉮: 새 판의 첫 쓰기가 성공한 뒤 옛 판의 주와 백업을 지운다(폴더가 깨끗하다). 대안 ㉯: 최근 N판만 남긴다.
10. **압축**. **권장(리뷰 뒤 바꿈)**: `FOodleDataCompression::Compress`를 Kraken·VeryFast로 직접 부르고, 코덱이 0을 내면 Stored. 엔진의 대표 속도로 Fast보다 약 4.5배 빠르고 약 3% 크다 — 100 ms 예산을 지킨다(2.2 MB에 약 14 ms). A-5a가 SuperFast·VeryFast·Fast를 재고, 머리의 코덱 칸과 '수준은 스트림에 없다' 덕에 나중에 바꿔도 옛 슬롯이 읽힌다. #40 ⑦의 기본값 문장은 `FCompression::CompressMemory`를 이름으로 드는데(`OPEN_QUESTIONS.md:38`), 권장은 Oodle을 직접 부른다 — #40 ⑦을 닫을 때 그 문장을 고친다(결정 3이 #70의 문장에 하는 것과 같다). 대안 ㉮(초안의 권장): Kraken·Fast(엔진이 깃발 없이 고르는 값 — 약 63 ms). 대안 ㉯: Kraken·Normal(더 작고 더 느리다 — 약 98 ms). 대안 ㉰: Mermaid·VeryFast(더 빠르고 크다). 대안 ㉱: Zlib(느리다).
11. **받아들인 청원의 답 직후 저장**. **권장**: 한다(추첨이 하나 있는 유일한 입력이라, 강제 종료로 다시 굴리는 길을 닫는다). 대안 ㉮: `docs/20` §9.1 그대로 틱 경계에서만(판당 끊김이 열 번쯤 준다). 대안 ㉯: 프로젝트 시작(`StartMegaproject`)에도 곧바로 쓴다(강제 종료로 다른 프로젝트를 고르는 길이 닫히지만, 프로젝트에는 추첨이 없어 다시 고르기는 되돌리기일 뿐 다시 굴리기가 아니다).
12. **저장 실패 알림**. **권장**: 게임 인스턴스의 첫 실패에 한 번 시간을 멈추고 `Save.Why.Failed`. 실패 뒤 60초는 경계가 다시 쓰지 않고(디스크가 고장 난 판이 매 틱 끊기지 않게), 실패한 쓰기의 까닭은 남아 60초 뒤의 첫 경계가 다시 쓴다(답의 저장도 다시 쓰인다). 대안 ㉮: 로그만. 대안 ㉯: 회색 '저장됨'(2초)과 노란 실패 표시를 상단 바에 둔다(메뉴가 없어 테스터가 저장을 알 길이 이것뿐이지만, 표시가 하나 는다).
13. **'10분마다'의 시계**. **권장**: 마지막 쓰기부터 실제 시간 600초 ∧ 틱이 움직임, 틱 경계에서 본다. 멈춘 동안에는 바뀌는 것이 입력뿐이고 답은 스스로 저장한다. 대안 ㉮: 600틱(×10에서 1분, ×25에서 24초마다). 대안 ㉯: 멈춘 시간을 뺀 실제 시간.
14. **밀어냄 기록의 꼴**. **권장**: 최상위 `FChronicleEviction {int32 Count = 0; int64 ThroughTick = 0;}`(멤버 19). 장의 '그 외에도'를 정확히 정하고 커맨드렛이 수를 잰다. 대안 ㉮: `int64 ChronicleTrimmedThroughTick = -1` 하나(USTRUCT가 없다. 수는 모른다). 대안 ㉯: `int32` 수 하나(장의 경계를 남은 줄에서 어림한다). 대안 ㉰: 연대기 배열 안의 표시 줄(새 필드가 없지만 연대기를 읽는 열 곳이 가짜 줄을 걸러야 한다). 대안 ㉱: `FLifeRecord` 안(세계 해시가 바뀌어 거절).
15. **상한의 뜻**. **권장**: 연대기 전체 1,500, 나가는 것은 S1만, 0 = 상한 없음, 오래된 것부터 한 번 훑기. 대안 ㉮: S1의 수만 1,500으로 센다(최악이 더 크다). 대안 ㉯: 단단한 상한(S2도 나간다 — A4 §4.4 ②·`Law.Judgment`에 어긋나 거절). 대안 ㉰: 스키마 최솟값 1(0 = 없음을 막는다).
16. **저장 규칙의 자리**. **권장**: 저장 Subsystem이 `OnSimTick`에 묶여 경계마다 디스크 사본과 견준다. 컨트롤러는 입력 셋(답·새 판·종료)을 부탁하고 실패를 알릴 뿐이다(§4.5). 대안: 컨트롤러의 `HandleSimTick`이 견주고 쓴다(표현 층이 저장 규칙을 갖고, 경계 규칙의 테스트에 산 컨트롤러가 든다 — `OwnHazardsSeen`과 같은 자리라 규칙 2의 물음은 없다).
17. **구현 뒤 ultracode 리뷰 1회**(A-5c 뒤, 손 확인 앞). **권장**: 한다. 여기서 버그가 나면 플레이어가 판을 잃는다(`WORKPLAN` §2 59행 '헤더 승인 + ultracode 리뷰', 77행 '저장·결정론 버그는 xhigh'). 대안: 구현은 끔으로 끝내고 손 확인으로 갈음한다.
18. **구현 단계의 effort**(검증이 올림). **권장**: A-5a~A-5c는 high에 ultracode 끔 — STATUS '다음 3작업' 3('구현 high에 ultracode 끔')과 MVP ①~④의 관례 그대로다. 헤더의 적대 리뷰(ultracode)가 이미 돌았고, 저장 경로는 A-5c 뒤의 ultracode 리뷰 1회(결정 17)가 따로 본다. 구현 리뷰는 xhigh, A-5d(문서)는 `WORKPLAN` §2 '설계 문서 변경' 행(65행)대로 xhigh다. 대안: 구현 커밋마다 xhigh(`WORKPLAN.md` §2 Phase 표의 '저장 작업' 행, 59행 — Phase 표가 작업 유형 표보다 우선한다, 69행). 값의 차이: xhigh는 세 커밋 모두 생각에 드는 시간과 토큰이 늘지만 구현 중의 저장·결정론 실수를 리뷰 전에 더 잡고, high는 그 몫을 결정 17의 리뷰 한 번에 맡긴다.

## 추가 이야기와 보완
- **스모크의 저장**: 결정 4의 대안 ㉱처럼 `-SaveDir=Smoke`(문을 여는 개발 스위치)를 두면 `run_checks`의 스모크가 게임 바이너리에서 틱 0 저장과 종료 저장을 돌리고 `CosmicSave:` 줄을 grep할 수 있다. 끝난 고정 슬롯을 놓고 둘째 스모크를 띄우면 ⓐ의 끝난 판 쪽 손 확인과 '테스트가 덮지 않는 것'(§8)이 스크립트가 된다. 범위 밖이라 제안으로만 둔다.
- **비동기 쓰기**: 측정이 100 ms를 넘으면 직렬화는 게임 스레드에 두고 압축·CRC·두 쓰기와 내리기만 작업 스레드로 보낸다(`AsyncSaveGameToSlot`의 꼴, `GameplayStatics.cpp:2403-2426`). 종료 저장은 동기로 남긴다. 백업 차례를 지키려면 쓰기를 한 줄로 세워야 한다.
- **20,000틱의 세계 해시를 러너에**: `run_checks.ps1 -Tier full`에 셋째 `Compare-Balance -Ticks=20000`을 두면 거르기의 증명(§0)이 손 측정에서 러너로 옮겨 간다(커맨드렛 몇 초). 러너의 바꿈이라 ⑤ 밖에 둔다.
- **테스터 안내**: 테스터에게 저장이 어디 있는지(⑨에서 확인한 폴더)와 '창을 닫으면 저장되고 다시 켜면 이어진다', '저장을 못 하면 시간이 멈추고 한 줄이 선다 — 저장 폴더의 빈 자리와 쓰기 권한을 본다'를 한 줄씩 적어 둔다.
- **리뷰에서 받지 않은 것**(상태 줄의 '일부 기각 둘'과 '받지 않은 물음 1'): ① A5-S1의 (d) '까닭 줄을 합치자'는 기각했다 — 플레이어가 되감김(그 앞의 저장으로 엶)과 잃은 판(읽지 못해 새 판)을 아는 길이 그 줄뿐이다. A5-S1의 나머지(결정 2의 권장을 바꿈)는 반영했다. ② A5-S2의 (4) '`RunBeganTick`을 빼자'는 기각했다 — 그것이 없으면 이어 연 판의 `CosmicEnd:` 줄이 판의 길이를 틀리게 말한다(§5.6). A5-S2의 나머지는 반영했다. ③ 관점의 물음 '상한과 밀어냄 기록을 ⑤에서 빼고 로그를 넘는 판이 오는 빌드(Phase 4·B5)로 미루자'는 A4 §4.4 ②③('저장 상한은 오래된 S1만', '밀어냄 기록은 ⑤ 헤더의 일')과 2026-10-18 결정('연대기 이관 #40 ⑦ + 상한 노브')이 ⑤에 두라고 해서 받지 않았다. 리뷰 표의 74건 가운데 20건은 다른 관점이 먼저 낸 발견과 같은 것이라(표에 'Same as'로 적힌 줄), 겹친 것을 합친 서로 다른 발견은 54건이다.
