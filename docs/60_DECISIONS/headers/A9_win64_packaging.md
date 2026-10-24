# MVP ⑨ 헤더 초안 — Win64 패키징과 쿠킹 빌드의 세계 해시

상태: **구현 끝 (2026-10-24 — `[A-9a]`~`[A-9c]`, 문서 재감사 `[A-9d]`, `git log --grep 'A-9'`; 다르게 하거나 더한 것은 `DECISIONS_LOG` 2026-10-24 MVP ⑨). 손 확인 ⓐ(§4.1)와 ⓔ(§4.2)가 남았다.** 승인 (2026-10-24, 개발자 "다음작업들 이어서 계속 진행해줘" — §9의 결정 15개 전원 권장안). 승인 전의 기록: 초안 (2026-10-24) — 조사 워크플로(넷이 나란히: 게임 바이너리의 코드 경로·엔진 5.8.3의 패키징 사실·문서에 이미 정해진 것·쿠킹 빌드의 해시를 재는 길 — 엔진 사실은 모두 로컬 엔진 소스의 줄로), 코드 변경 없는 시험 패키지 한 번(§1.6), 적대 리뷰(ultracode 워크플로 — 네 관점: 엔진과 빌드의 사실·해시 판이 ⓓ를 증명하는가·도구와 실패의 길·결정과 손 확인과 플레이어, 관점마다 적대 검증 하나) 발견 48건, 남김 47(확인 30·일부 17, 반박 1), 높음 0·중간 15·낮음 32 — 모두 본문에 넣었다. 리뷰가 바꾼 설계: 패키지마다 새 폴더와 저장 없는 건넬 zip(§3.2·§4.2), 해시 판의 스위치가 셋의 상태(없음·틀림·맞음 — 틀린 값은 거절하고 끝난다)와 틱 0(§2.1), 정책 판도 쿠킹 빌드에서(§2.2 — 결정 3), 러너는 같은 실행의 커맨드렛 줄과 견준다(§3.1), 로그·종료 코드·시간 제한의 판정(§3.2), 빌드를 알리는 `BuildInfo.txt`(§2.4), Core Logic의 비인라인 수학을 막는 린트(§3.3), 스트링 테이블 점검의 낱말, effort 표(§7). 개발자 결정은 §9.

근거: `docs/50_ROADMAP.md` A ⑨('Win64 패키징')와 종료 조건 ⓐ(새 판 → 저장 → 재시작 → 이어하기)·ⓓ(쿠킹 빌드 시드 42 세계 해시 = 에디터)·ⓔ(외부 1명), 같은 문서의 위험 표('쿠킹 해시 미검증' — 완화는 ⓓ)와 Phase 1에서 미룬 확인 8(`:94` — '첫 패키징 때 세계 해시로 비교한다(테스터와 시드를 나누기 전)'), `docs/00_VISION.md:32`('Win64 패키징 1회'), `docs/20_ARCHITECTURE.md` §9.1(슬롯의 자리)·§9.2(결정론 — 마지막 문장)·부록 불변 규칙 6, 헤더 `A5_save_minimum.md` §10(손 확인 ⓐ의 여덟 차례)·§11('패키지의 저장 폴더는 ⑨가 본다'), `docs/45_ENGINE_NOTES.md:97`, `STATUS.md` 다음 작업 1·2, `WORKPLAN.md` §2(일의 종류별 effort), `CLAUDE.md` 불변 규칙 1·6·7·10과 '빌드 / 실행 / 테스트'.

---

## 0. 범위와 전제

**⑨가 하는 일**: 처음으로 쿠킹한 Win64 게임 빌드를 만들고, 그 빌드로 두 가지를 보인다 — **ⓓ 쿠킹 빌드가 에디터와 같은 세계를 만든다**(시드 42의 idle 판과 통치자 정책 판의 두 해시가 에디터와 같다)와 **ⓐ 패키지에서 새 판 → 저장 → 재시작 → 이어하기가 된다**(개발자의 손 확인). ⓔ(외부 1명)에게는 같은 빌드를 저장 없는 zip으로 건넨다(§4.2).

**세계에 닿지 않는다 — 그래도 헤더를 먼저 쓴다**: 새 저장 필드·열거자·틱 단계·추첨 지점은 없다(`SaveVersion` 1, 최상위 멤버 19 그대로). 두 해시와 기준값은 바뀌지 않는다 — 커맨드렛의 정책 단계를 공유 함수로 옮기는 것(§2.2)도 기준값 그대로임을 보인다. 그래도 결정론을 **증명하는 길**(새 개발 스위치)과 빌드·저장 폴더·로그의 뜻이 바뀌므로 헤더 먼저(`CLAUDE.md` 작업 방식 — '결정론에 닿는 코드').

**하지 않는 것**: Shipping 배포 빌드(로그·콘솔이 없다 — §1.3; 컴파일만 한 번 — 결정 1), 스토어·아이콘·제품 이름(#7), 크래시 리포터, 코드 서명, 테스트·커맨드렛을 에디터 전용 모듈로 옮기기(C), 개발 스위치·치트 명령의 제거(Phase 5 테스터 빌드 전 — 결정 11), 국제화 설정(C — 결정 10은 눈 확인만), 끝내기 단추(결정 12의 안내는 Alt+F4), 프로젝트를 OneDrive 밖으로 옮기기(#8).

---

## 1. 기대는 사실 (2026-10-24, HEAD `3f77025`, 엔진 5.8.3 CL 58210709 — 설치(런처) 엔진)

### 1.1 게임 바이너리는 이미 컴파일된다 — 쿠킹이 처음이다

- `run_checks.ps1`이 매번 게임 타깃(`Build.bat Cosmic_Ascension Win64 Development`)을 짓는다(`Tools/run_checks.ps1:55`). 그 exe는 쿠킹된 데이터 없이는 돌지 않는다(`WindowsPlatformProperties.h:66-69` — `RequiresCookedData = !HAS_EDITOR_DATA`).
- 패키징 설정은 `DefaultGame.ini`의 `DirectoriesToAlwaysCook` 셋(`/Game/CA/Data`·`Materials`·`UI` — 경로로만 읽는 데이터 테이블·스트링 테이블·머티리얼·`WBP_LogEntry`)뿐이고 나머지는 엔진 기본값이다(`Engine/Config/BaseGame.ini:87-118`: pak·IoStore·Zen 켬, Oodle Kraken, `BuildConfiguration=PPBC_Development`, `InternationalizationPreset=English`, `CulturesToStage=en`, `IncludeCrashReporter=False`). 맵 `L_Galaxy`는 `GameDefaultMap`이라 따로 쿠킹되고(`CookOnTheFlyServer.cpp:8784-8826`), 생성자의 `ConstructorHelpers`가 읽는 엔진 메시(`/Engine/BasicShapes/Sphere`·`Plane`)와 UMG 기본 글꼴 Roboto(→ `Faces/DroidSansFallback` — 한글·▲▼)는 시작 패키지로 쿠킹된다(`:11912-11917`, Roboto.uasset의 참조).
- 게임 코드(테스트 밖)가 빌드에 따라 갈리는 곳은 `GIsEditor`·`GIsAutomationTesting` 둘뿐이다(저장 문 — `Save/CosmicSaveSubsystem.cpp:116-120`, 보는 사람의 설정 쓰기 — `World/CosmicGameMode.cpp:440,921`). `WITH_EDITOR`·`UE_BUILD_*` 분기는 테스트에만 있다. FName의 대소문자는 에디터에서만 보존되지만(`NameTypes.h:32-33`) 해시는 이름·태그를 소문자로, 지도를 대소문자 없는 `FName::Compare` 차례로 쓴다(`GalaxySimulationSubsystem.cpp:414-420`, `NameTypes.h:1691-1696`).
- 경로로 읽는 자산이 빠지면 조용히 바뀐다: 데이터 테이블이 없으면 `LogCosmic` Warning 'Data table … is missing.' 한 줄과 null, 시뮬은 C++ 기본값이나 빈 행으로 돈다(`Data/CosmicDataTables.cpp:11-24`, `GalaxySimulationSubsystem.cpp:556-575`). 스트링 테이블의 항목이 없으면 엔진은 화면에 자리 글을 그리고 로그에는 `LogStringTable: Warning: Failed to find string table entry for …`를 쓴다(`StringTableRegistry.cpp:156`, `TextHistory.cpp:2797-2799` — 화면의 `<MISSING STRING TABLE ENTRY>`는 로그에 나오지 않는다, `StringTableCore.cpp:94`). 해시는 FText를 넣지 않으므로 **ⓓ는 스트링 테이블의 빠짐을 잡지 못한다** — 시작 점검의 낱말이 잡는다(§3.2).

### 1.2 쿠킹 빌드는 오늘 세계 해시를 낼 길이 없다

- 세계 해시를 찍는 것은 커맨드렛 `CosmicBalance` 하나다(`Tests/CosmicBalanceCommandlet.cpp:707` — `CosmicBalance: seed %d, %lld ticks, hash %u, world %u, wrote %s`). 게임은 `CosmicSave:`·`CosmicContinue:` 줄에 **전체 해시**만 찍고, 그것도 저장 문이 열렸을 때뿐인데 `-Seed=`가 문을 닫는다(`CosmicSaveSubsystem.cpp:84-89`).
- 커맨드렛은 게임 exe에 링크되어 있지만 거절된다: `UCommandlet`의 생성자가 `IsEditor = true`이고(`Commandlet.cpp:31-33`) 이 커맨드렛은 생성자를 두지 않아, 에디터 없는 exe는 'Cannot run editor commandlet … with game executable.'을 찍고 끝난다(`LaunchEngineLoop.cpp:3994-4001` — §1.6에서 보았다).
- 전체 해시는 같은 `TellAll`이면 견줄 수 있다. 커맨드렛의 idle 판은 `-TellAll=1`(기본, `NewGalaxy` 전에 켠다 — `CosmicBalanceCommandlet.cpp:479-483`), 정책 판은 `-TellAll=0`이고, 게임은 이야기의 범위(`bTellAll=false` — `Log/CosmicLogSubsystem.h:138`)로 말한다. 세계 해시는 어느 쪽이든 같다(`Sim.KnowingInert`).
- 커맨드렛의 판은 틱마다 `RunHeadless(1)` 뒤 상태를 읽고(CSV·슬롯 바이트·연대기 — 쓰지 않는다고 주장한다, `CosmicBalanceCommandlet.h:48-50`), 정책 판이면 그 틱 경계에서 입력을 넣는다(답 — 비축으로 낼 수 있는 첫 선택지를 답한 수부터 돌아가며, 프로젝트 키를 거절될 때까지, 임시 끝 전이면 '다음 칸의 연구'로 중점 — `:562-600`, `PolicyChoiceOf :110-127`). 그 정책은 커맨드렛 안에만 있다.
- 게임의 프레임 길로는 정확한 틱에 멈출 수 없다: 자동 정지(카드·위기·Severity 2 줄), 임시 끝의 래치, 프레임당 최대 10틱(`GalaxySimulationSubsystem.cpp:574-593`).

### 1.3 구성(Development·Shipping)과 설치 엔진

- 설치 엔진은 Win64 게임 구성으로 Development·DebugGame·Shipping만 준다(Test 없음 — `BaseEngine.ini:4003-4007`).
- **Shipping은 로그·콘솔·`-ExecCmds`가 없다**: `NO_LOGGING = !USE_LOGGING_IN_SHIPPING`(기본 0 — `Build.h:345-351`, UE_LOG는 Fatal만 남는다 — `LogMacros.h:184-194`), `ALLOW_CONSOLE_IN_SHIPPING` 0, `-ExecCmds`는 `!UE_BUILD_SHIPPING`에서만(`UnrealEngine.cpp:2543-2552`), `WITH_DEV_AUTOMATION_TESTS=0`. `bUseLoggingInShipping`·`bUseConsoleInShipping`은 `[RequiresUniqueBuildEnvironment]`이고 설치 엔진은 고유 빌드 환경을 거절한다(`RulesAssembly.cs:710-713`). `bOverrideBuildEnvironment`로 그 검사를 건너뛸 수는 있지만(`UEBuildTarget.cs:1515`, `TargetRules.cs:3077-3081`) 미리 지은 Shipping 엔진은 `NO_LOGGING`으로 컴파일되어 정의가 섞인다(시험하지 않음). 손 확인 ⓐ의 `CosmicSave:`·`CosmicContinue:`·`CosmicEnd:` 줄과 ⓓ의 해시 줄은 Shipping에 없다.
- 명령줄 스위치(`-Seed=`·`-Speed=`·`-NoSave` 등)는 어느 구성에서나 읽힌다(`CommandLine.h:14-21`). Exec 명령의 기본 인자는 에디터에서만 채워진다(`ScriptCore.cpp:1473-1510`) — 쿠킹 빌드에서는 인자를 모두 적는다.

### 1.4 결정론: 두 바이너리의 부동소수점

- 이 모듈의 번역 단위는 두 바이너리에서 같은 `/fp:precise`로 컴파일된다: `Build.cs`가 에디터 아닌 타깃에 `FPSemantics = Precise`를 주고(`Cosmic_Ascension.Build.cs:18-24`), 에디터 타깃은 V7 기본이 이미 Precise다(`VCToolChain.cs:1313-1335`, `TargetRules.cs:3739`). 두 `.rsp`가 같은 `/fp:precise /Oi /Gw /Gy /Ob2 /Ox /Ot`이고 `/arch`·`/GL`(LTCG)·PGO가 없다(SSE2 스칼라, FMA 없음).
- **엔진 코드는 다르다**: 게임 exe가 링크하는 Epic의 미리 지은 `UnrealGame` 엔진 목적 파일은 `/fp:fast`, 에디터 DLL은 `/fp:precise`다(`UnrealGame.Target.cs:9-15`, `CorePreciseFP.Build.cs:9`). 그리고 **모놀리식 게임 exe에서는 '모듈 안의 인라인'도 보장이 아니다**: 인라인되지 않은 헤더의 inline 함수·템플릿은 COMDAT이고 링커는 처음 만난 사본 하나를 남기는데, 게임 exe의 링크 차례는 엔진 목적 파일이 먼저다(`Intermediate/…/Cosmic_Ascension.exe.rsp` — Core 86행, Engine 430행, 모듈 753행부터). 에디터의 모듈 DLL은 제 사본을 가진다. 지금 해시에 드는 셈은 `FMath::Max/Min/Clamp/Sqrt/RoundToInt32/IsNaN`, 강제 인라인 `FVector::DistSquared`(`UE_FORCEINLINE_HINT` — `Vector.h:1041,2476`, 레인 고르기), 헤더뿐인 `FRandomStream`이고(그 가운데 `FRandomStream`의 몇 멤버는 FORCEINLINE이 아닌 inline — `RandomStream.h:115,131,209`), 해시 길의 엔진 호출 하나(`FNumericProperty::GetFloatingPointPropertyValue`)는 float → double 넓히기라 어느 모드에서도 정확하다. 비인라인 엔진 수학(`FMath::Fmod`·`Atan2` — `GenericPlatformMath.h:517-518,548-549`, CRT의 `pow`·`sin`…)은 불변 규칙 6이 막지만 그것을 강제하는 도구가 없다(`check_layers.py:43`은 `FMath::Rand*`만 막는다) — §3.3. 이 갈림을 에디터 테스트는 잡지 못한다; ⓓ만 잡는다.

### 1.5 패키지의 자리 — 저장·로그·설정·창

- `ProjectSavedDir() = ProjectUserDir() + Saved`이고, `ProjectUserDir`은 `FApp::IsInstalled()`(Shipping 데스크톱의 기본 참)나 `-SaveToUserDir`이면 `%LOCALAPPDATA%/Cosmic_Ascension/`, 아니면 프로젝트 자리다(`Paths.cpp:183-192,451-475`, `App.cpp:212-239`). **Development 패키지는 저장과 로그를 패키지 안 `Cosmic_Ascension\Saved\`에**(`SaveGames\`·`Logs\Cosmic_Ascension.log`·`Config\Windows\GameUserSettings.ini` — 보는 사람의 설정은 에디터의 `WindowsEditor`와 따로, 기본값에서 시작), Shipping은 `%LOCALAPPDATA%\Cosmic_Ascension\Saved\`에 둔다. 슬롯 이름은 그대로(`CosmicSaveSubsystem.cpp:281-295`).
- 스테이징은 매번 스테이지 폴더를 통째로 지우고(`CopyBuildToStagingDirectory.Automation.cs:6346-6370`), 아카이브는 덮어 쓰기만 한다(`ArchiveCommand.Automation.cs:40-50`) — 한 자리의 아카이브는 지난 빌드의 `Saved\`(세이브·로그·설정)를 다음 빌드에 남긴다. 기본 스테이지는 `<프로젝트>\Saved\StagedBuilds\Windows` — OneDrive 안이다(`ProjectParams.cs:1677-1691`). 그래서 패키지마다 OneDrive 밖의 **새 폴더**에 스테이징한다(§3.2).
- 레이아웃: `Windows\Cosmic_Ascension.exe`는 진짜 exe를 부르는 부트스트랩(인자를 넘긴다 — §1.6; VC++ 런타임이 없으면 영어 설치 창과 UAC를 띄운다 — 부트스트랩의 문자열 'The following component(s) are required to run this program:'), 진짜 exe는 `Windows\Cosmic_Ascension\Binaries\Win64\Cosmic_Ascension.exe`(이름 장식 없음 — `UEBuildTarget.cs:5473-5496`), 콘텐츠는 `Content\Paks\`의 IoStore 컨테이너다. 둘 다 서명되지 않았다(부트스트랩의 서명은 UAT가 리소스를 고칠 때 잘린다) — 다른 PC에서는 SmartScreen이 막는다. 게임 타깃은 `-Cmd` 콘솔 exe가 없는 GUI 프로그램이다(`TargetRules.cs:1819-1823`).
- **종료 코드**: 우아한 종료(`RequestExitWithStatus(false, …)`)의 코드는 버려진다 — `PostQuitMessage`만 하고(`WindowsPlatformMisc.cpp:1514-1518`) 프로세스는 `EngineInit`의 ErrorLevel 0을 돌려준다(`Launch.cpp:172,204`). 충돌은 3이다(`LaunchWindows.cpp:283-293`, `WindowsErrorOutputDevice.cpp:107`). `-AbsLog=`의 파일을 열지 못하면 엔진은 조용히 `<이름>_2.log`에 쓴다(`OutputDeviceFile.cpp:496-511`); 확장자가 `.log`·`.txt`가 아니면 그 이름을 버린다(`GenericPlatformOutputDevices.cpp:93-98`).
- **창**: 인자 없이 띄우면 창 모드의 기본값은 테두리 없는 전체 화면이다(`GameUserSettings.cpp:830-834`). 게임에는 끝내기 단추가 없다 — Alt+F4·작업 표시줄의 닫기, Alt+Enter는 창으로(`DefaultInput.ini:61`). 창을 닫은 뒤 프로세스가 끝나기까지 시험 패키지에서 한 번 약 19초, 첫 패키지에서는 1초 안팎이었다(§1.6·§1.7 — `-nullrhi`에서는 바로 끝난다); 그 사이에 다시 띄우면 둘째 프로세스가 같은 `Saved\`를 쓰고 로그는 `Cosmic_Ascension_2.log`로 간다.
- UAT를 `-unattended`로 부르면 UAT의 실행 단계(`-run`)가 게임에도 `-unattended`를 넘겨 저장 문이 닫힌다(`RunProjectCommand.Automation.cs:682-685`) — 패키지는 UAT의 `-run` 없이 손이나 스크립트로 띄운다.

### 1.6 시험 패키지 (2026-10-24 — 코드 변경 없이, `C:\CA_Builds\probe`)

명령: `RunUAT.bat BuildCookRun -project=<uproject> -platform=Win64 -clientconfig=Development -build -cook -stage -pak -iostore -prereqs -stagingdirectory=C:\CA_Builds\probe\Stage -archive -archivedirectory=C:\CA_Builds\probe\Archive -utf8output -unattended -noP4` (CPU AMD Ryzen 7 9700X, 디스크 여유 216 GB).

| 단계 | 잰 것 |
|---|---|
| UAT 전체 | **4.3분**, `BUILD SUCCESSFUL`, 종료 코드 0. 빌드 35초 — UAT의 인자로 makefile을 다시 만들어 unity 파일 넷을 다시 컴파일하고 게임 exe를 다시 링크했다(에디터 DLL은 그대로; **패키지의 exe는 `run_checks`가 지은 exe가 아니다**) · 쿠킹 177초(`Packages Cooked: 510`, 플랫폼으로 건너뜀 7, 쿠킹 요약 '0 error(s), 1 warning(s)') · 스테이징 43초 · 아카이브 0.5초. Zen 서버는 설치되어 있었고 그대로 썼다(`zen.version` 5.8.13). 경고는 `LogSockets: GetAddressInfo … SE_HOST_NOT_FOUND`(오프라인의 이름 풀기)뿐 — 빠진 패키지·`UnexpectedLoad` 없음(손으로 읽었다) |
| 크기 | 아카이브 1,020 MiB(스테이지도 같은 크기 — 두 벌): exe 335 MB, PDB 380 MB, `Content\Paks` 217 MB(`.ucas` 203 MB·`.pak` 11 MB·`global.ucas` 3 MB — 부분은 10진 MB), `Engine\Extras\Redist\en-us\vc_redist.x64.exe`·`arm64` |
| 레이아웃 | `Windows\Cosmic_Ascension.exe`(부트스트랩) · `Windows\Cosmic_Ascension\Binaries\Win64\Cosmic_Ascension.exe`(진짜) · `Content\Paks\` — §1.5대로 |
| 시작 점검 | 진짜 exe `-nullrhi -nosound -unattended -nosplash -ExecCmds=quit -AbsLog=…` → 2.8초, 종료 코드 0. 서브시스템 다섯 초기화, `CosmicSave: the game's saving is off by the command line`, `CosmicContinue: Off (command line)`, `NewGalaxy: seed …, 60 systems and the sanctuary, 12 civs, galaxy gal_milkyway` — 'Data table … is missing'·`LogStringTable` 줄 없음 |
| 부트스트랩 | `Windows\Cosmic_Ascension.exe`에 준 인자가 진짜 exe의 `Command Line:`에 그대로 있다(`-AbsLog`의 로그가 생겼다) |
| 커맨드렛 | 진짜 exe `-run=CosmicBalance -Seed=42 -Ticks=30` → `LogInit: Error: Cannot run editor commandlet Class /Script/Cosmic_Ascension.CosmicBalanceCommandlet with game executable.`, 종료 코드 1 |
| 창으로 한 판 | 진짜 exe `-windowed -ResX=1280 -ResY=720 -ShotAfterSeconds=15`(저장 문 열림): `CosmicSave: the game's saving is allowed by the command line` → `CosmicContinue: NoSlot` → `CosmicSave: wrote Galaxy_… (NewRun) tick 0, … 8298 of 118913 bytes in 4.5 ms` → 30초 뒤 창 닫기(`CloseMainWindow`) → `(Exit) tick 26 … in 5.7 ms` → `RequestExit(0, UGameEngine::Tick.ViewportClosed)` — **창 닫기의 종료 저장이 패키지에서도 닿는다**(MVP ⑤ 리뷰의 높음). 그 뒤 프로세스가 끝나기까지 약 19초('Slate User Destroyed' 12:00:55 → 'Preparing to exit' 12:01:15). 종료 코드 0 |
| 자리 | `Archive\Windows\Cosmic_Ascension\Saved\` 아래: `SaveGames\Galaxy_<id>.sav`·`_bak.sav`, `Logs\Cosmic_Ascension.log`, `Config\Windows\GameUserSettings.ini`, `Screenshots\Windows\ScreenShot00000.png`, `Config\CrashReportClient\UECC-Windows-*\CrashReportClient.ini`(엔진이 실행마다 만든다). `%LOCALAPPDATA%\Cosmic_Ascension`은 생기지 않았다 |
| 화면 | 1280×720 한 장: 상단 바·성계 패널('이른 해'·'첫 불의 모임'·연구 중점 단추 넷)·로그('[0] 이른 해. 이 백성이 당신에게 맡겨졌다.')의 한글이 네모 없이 그려지고 줄바꿈도 에디터와 같아 보인다. 경고는 프로파일러 DLL을 못 찾았다는 `LogWindows` 넷뿐 |

읽은 것: 5.8의 기본값(Zen·IoStore·pak)으로 첫 패키지가 막힘 없이 나온다. 쿠킹 빌드는 데이터를 모두 찾고 한글을 그리며, 저장과 창 닫기의 종료 저장이 패키지 안 `Saved\`에서 돈다. 남은 것은 ⓓ의 숫자 — 쿠킹 빌드가 세계 해시를 낼 길이 없다(§1.2) — 하나다.

### 1.7 첫 패키지와 ⓓ (2026-10-24, `[A-9b]` — `Tools/package.ps1 -Ticks20000 -Shipping`, 커밋 `f58a0f4`)

빌드 폴더 `C:\CA_Builds\20261024-2234-f58a0f4`, 건넬 zip `C:\CA_Builds\20261024-2234-f58a0f4-handoff.zip`. **`package: all passed`.**

| 단계 | 잰 것 |
|---|---|
| UAT | 101초(쿠킹 36초, '0 error(s), 0 warning(s)' — 510개를 모두 다시 쿠킹한 전체 쿠킹(증분 건너뜀 0); 빨라진 것은 시험 패키지가 데운 Zen 서버의 파생 데이터 캐시 `ue.ddc` 덕) |
| 건넬 zip | 28초, 484 MiB, 파일 51(빌드의 `Windows\` 파일 수와 같다 — 폴더 항목을 더하면 zip 항목 80), `Saved\` 없음, `Cosmic_Ascension\BuildInfo.txt` 있음. 빌드 폴더 1,021 MiB(로그 포함) |
| 시작 점검 | 2.4초 — `CosmicBuild: version 0.1.0, commit f58a0f4, cooked`, `NewGalaxy:`, 빠진 테이블·스트링 테이블·`LogCosmic: Error` 없음 |
| **ⓓ 3,000틱 idle** | 패키지 = 에디터 = `seed42.json`: **hash 4061194924, world 681643174** (11.6초 — 두 판) |
| **ⓓ 3,000틱 정책** | 패키지 = 에디터 = `seed42.json`: **hash 847130296, world 1448576366** (11.4초) |
| ⓓ 20,000틱 idle | 패키지 = 에디터: hash 1123943493, world 1128422968 — CHANGELOG에 손으로 적어 둔 값과도 같다 (11.4초) |
| ⓓ 20,000틱 정책 | 패키지 = 에디터: hash 3928640348, world 2194209278 (12.5초 — 처음 잰 값) |
| 기준 파일 | 에디터 = `seed42.json`의 두 줄 |
| Shipping 컴파일 | 47초, 통과(`Cosmic_Ascension-Win64-Shipping.exe` — 쿠킹·실행 없음) |

도구의 검사를 한 번씩 깨뜨렸다(스크립트의 함수를 그대로 떼어 이 빌드에): `-Seed=` 없는 판 → 거절 줄을 '요청한 판이 아니다'로 실패, idle 로그에 정책 열쇠를 물음 → 실패, 다른 틱 → 실패, world만 다른 두 열쇠 → 'the world hash differ', 지난 통과 로그를 두고 로그를 쓰지 않는 프로세스 → '이 실행이 쓴 로그 없음', 시작 로그에 `LogStringTable: Warning` 한 줄 → 실패; 대조군(맞는 판)은 읽힌다.

손 확인 ⓐ의 기계적인 차례를 스크립트로 한 번 미리 했다(부트스트랩으로, 창 1280×720): 새 판 `(NewRun) tick 0` → 25초 뒤 창 닫기 `(Exit) tick 22, hash 1030996466` → 다시 띄우기 `loaded … at tick 22, hash 1030996466` = `CosmicContinue: Newest … hash 1030996466` → 창 닫기(멈춘 채라 `exit - the slot already holds this state` — 이제 Log 줄). 이번에는 창을 닫은 뒤 1초 안팎에 끝났다(시험 패키지의 19초는 다시 나오지 않았다). 화면 한 장에 한글이 그려진다. 그 `Saved\`는 `Logs\rehearsal_Saved`로 옮겨, 개발자의 손 확인은 빈 `Saved\`에서 시작한다. 시험 패키지 폴더(`C:\CA_Builds\probe`)는 지웠다.

읽은 것: **종료 조건 ⓓ를 채운다** — 쿠킹한 Development 빌드가 시드 42의 idle 판과 통치자 정책 판에서 에디터와 같은 두 해시를 3,000틱과 20,000틱 모두에서 낸다(정책 판은 답의 추첨·카드의 효과·프로젝트 시작·연구 중점·이야기의 범위를 지난다). 남은 것은 개발자의 손 확인 ⓐ(§4.1)과 ⓔ(§4.2)다.

---

## 2. 해시 판 — 쿠킹 빌드가 두 해시를 내는 길 (결정 2·3)

### 2.1 `World/CosmicHashRun.h/.cpp` (새 — 상태 없는 함수 이름공간, 추첨 없음)

```cpp
#pragma once

#include "CoreMinimal.h"

class UGalaxySimulationSubsystem;
class UCosmicLogSubsystem;
class UPlayerRoleSubsystem;

// MVP item 9 (docs/60_DECISIONS/headers/A9_win64_packaging.md): the cooked build's proof of determinism. A development
// switch read once at the start of a game process runs a seed headless - as the balance commandlet's idle run, or its
// ruler-policy run - writes lines with both hashes and asks the engine to quit. Nothing a player keeps is touched: the
// run needs -Seed=, which closes the game's saving, and refuses while saving is on.
namespace CosmicHashRun
{
	inline constexpr int64 MaxTicks = 100000;

	// -CosmicHashRun=<ticks>: Absent when the switch is not on the command line; Bad when its value is not 1 to 6 ASCII
	// digits or lies above MaxTicks ("=abc", "=", "=+3000", "=3000x", "=-5", "=100001"); else Ticks (0 = the generated
	// galaxy alone, no tick). The value is read as text (FParse::Value into an FString), never as an int64 - that
	// overload stops at the first non-digit and takes "3000x" for 3000.
	enum class ESwitch : uint8 { Absent, Bad, Ticks };
	struct FSwitch
	{
		ESwitch Kind = ESwitch::Absent;
		int64 Ticks = 0;
		int64 Every = 0;      // -CosmicHashEvery=<ticks>: a checkpoint line at tick 0 and every Every ticks (0 = none; Bad as above)
		bool bPolicy = false; // -CosmicHashPolicy: the ruler policy of the commandlet's -Answer=1 -Build=1 -Focus=1 -TellAll=0
		FString Text;         // the value as given, for the refusal line
	};
	FSwitch SwitchOf(const TCHAR* CommandLine);

	enum class EOutcome : uint8
	{
		Ran,          // the hashes are read
		MissingData,  // a table, the rules row or the home galaxy row is missing: no NewGalaxy, no hashes
	};

	struct FResult
	{
		EOutcome Outcome = EOutcome::MissingData;
		int32 Seed = 0;
		int64 Ticks = 0;
		bool bPolicy = false;
		uint32 StateHash = 0;
		uint32 WorldHash = 0;
		TArray<FString> Checkpoints; // "CosmicHashAt: tick 0, hash H, world W", ... when Every > 0
		FString Missing;             // what is missing, in table order ("DT_Rules, DT_Galaxies:home"), when MissingData
	};

	// Idle: the log telling all (the commandlet's default -TellAll=1), switched before NewGalaxy(Seed, 0, 0) so the
	// lines of tick 0 are told the same way, then RunHeadless in steps of Every (or one step), then both hashes.
	// Policy: the log in the game's scope (-TellAll=0), NewGalaxy, then one tick at a time with CosmicRulerPolicy::Act
	// between ticks - the commandlet's loop. The log's mode comes back afterwards. Refuses before NewGalaxy when any of
	// the ten tables, the rules row or the home galaxy row is missing (the hash would come from C++ defaults).
	FResult Run(UGalaxySimulationSubsystem& Sim, UCosmicLogSubsystem& Log, UPlayerRoleSubsystem& Role, int32 Seed,
		const FSwitch& Switch);

	// "CosmicHash: seed 42, 3000 ticks, hash 4061194924, world 681643174" (policy: "CosmicHash: seed 42, 3000 ticks,
	// hash 847130296, world 1448576366, policy") - the key of the commandlet's line without ", wrote <path>";
	// MissingData gives "CosmicHash: refused - missing <Missing>".
	FString LineOf(const FResult& Result);
}
```

- **구현(A-9a)**: `FSwitch`에는 `EveryKind`·`EveryText`·`IsGood()`(틱 수가 읽혔고 체크포인트 수가 주어졌다면 그것도)이 더 있다 — `World/CosmicHashRun.h`가 원본이다. 값은 다음 공백까지 글로 읽는다(`FParse::Value`의 기본처럼 쉼표·닫는 괄호에서 끊으면 `=3,000`이 3틱이 된다 — 구현 리뷰).
- **층**: `World/`(Sim·Log·Player를 부르고, 정책은 UI의 순수 함수 `CosmicNextRung`을 부른다 — World와 UI는 같은 층, `check_layers.py`의 `LAYER`). Core Logic은 바뀌지 않는다.
- **같은 숫자인 까닭과 그 증명**: 커맨드렛의 idle 판은 입력 없이 상태를 읽기만 하므로 그 세계는 `RunHeadless(Ticks)`의 세계라고 주장한다(`CosmicBalanceCommandlet.h:48-50`) — 그 주장을 이 헤더가 처음으로 매번 본다: §3.1의 러너 단계가 같은 에디터 바이너리에서 같은 실행의 커맨드렛 줄과 해시 판 줄을 견준다(idle과 정책 둘 다). `RunHeadless`를 나눠 부른 판과 한 번에 부른 판이 같은 세계라는 것은 테스트가 본다(§5 ⑥).

### 2.2 `World/CosmicRulerPolicy.h/.cpp` (새 — 커맨드렛의 정책 단계를 옮긴다)

```cpp
// The ruler policy of the balance commandlet (MVP item 3, -Answer/-Build/-Focus) as one function the commandlet and the
// hash run share (MVP item 9): the inputs a script gives between ticks, in the commandlet's order.
struct FCosmicRulerPolicy { bool bAnswer = false; bool bBuild = false; bool bFocus = false; };

struct FCosmicRulerPolicyCounts
{
	int32 Answers = 0;      // accepted answers, withdrawn cards not counted (the round of PolicyChoiceOf reads it)
	int32 Started = 0;      // projects started
	int32 FocusChanges = 0; // accepted focus changes
	int32 CardsCome = 0;    // cards seen to come, at the boundary and after each answer
	bool bEnded = false;    // a provisional end has been read at a boundary: no focus input from then on
};

namespace CosmicRulerPolicy
{
	// The answer of the -Answer policy: the first choice the stockpile can pay, from choice (answers so far) on, round.
	int32 ChoiceOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FEncounterDef& Row, int32 AnswersSoFar);

	// One tick boundary, after RunHeadless(1): reads the provisional end into bEnded first, then answers (guard 8),
	// starts (guard 8) and the next rung's research focus while not ended - exactly the commandlet's lines 562-600.
	// CardBefore is the pending card before the tick (for CardsCome).
	void Act(const FCosmicRulerPolicy& Policy, UGalaxySimulationSubsystem& Sim, UPlayerRoleSubsystem& Role, FName CardBefore,
		FCosmicRulerPolicyCounts& Counts);
}
```

- 커맨드렛은 제 `PolicyChoiceOf`와 정책 본문을 지우고 `CosmicRulerPolicy::Act`를 부른다(그 밖의 셈 — 임시 끝의 틱·장의 셈·CSV — 은 그대로). **증명: 옮긴 뒤 `ruler_summary_line`(847130296 / 1448576366과 ruler 줄의 셈)이 그대로다.**
- 정책 판은 같은 커밋의 커맨드렛 줄과 견주므로 정책이 바뀌어도 두 길이 함께 바뀐다.

### 2.3 `ACosmicGameMode::StartPlay`의 맨 앞 (선언 변화 없음 — .cpp 본문)

```cpp
// MVP item 9: a hash run instead of a game (CosmicHashRun). Before BeginSession, so no continue reads a slot; StartPlay
// runs inside FEngineLoop::Init (GEngine->Start -> LoadMap), so after the exit request the engine loop never ticks.
const CosmicHashRun::FSwitch Switch = CosmicHashRun::SwitchOf(FCommandLine::Get());
if (Switch.Kind != CosmicHashRun::ESwitch::Absent)
{
	if Switch.Kind == Bad                       -> Error "CosmicHash: refused - bad -CosmicHashRun=<Text>"
	else if no -Seed=                           -> Error "CosmicHash: refused - no -Seed="
	else if (Save && Save->IsGameSavingOn())    -> Error "CosmicHash: refused - the game's saving is on"
	else                                        -> Display each checkpoint, then Display LineOf(Run(...)) (MissingData -> Error)
	FPlatformMisc::RequestExitWithStatus(false, 0, TEXT("CosmicHashRun"));
	return; // Super::StartPlay is not called: no actor begins play, so no EndPlay exit save either
}
```

- **들어가는 조건과 거절 줄(구현)**: 세 스위치(`-CosmicHashRun=`·`-CosmicHashEvery=`·`-CosmicHashPolicy`) 가운데 하나라도 있으면 해시 판으로 들어간다 — 그래서 `-CosmicHashRun=` 없는 나머지 둘도 게임을 시작하지 않고 거절된다. 거절 줄(Error): `CosmicHash: refused - no -CosmicHashRun=` · `bad -CosmicHashRun=<글>` · `bad -CosmicHashEvery=<글>` · `no -Seed=` · `the game's saving is on` · `no simulation`(서브시스템이 없을 때), 그리고 `Run`의 `missing <이름>`.
- **판정은 줄이다**: 우아한 종료의 코드는 늘 0이다(§1.5). 스크립트는 종료 코드 0(충돌은 3)과 로그의 `CosmicHash:` 줄을 함께 요구하고, 줄이 없거나 `refused`면 실패다.
- **틀린 스위치는 게임을 시작하지 않는다**: 값이 틀리면 거절하고 끝난다(Bad). 이름을 잘못 쓴 스위치(`-CosmicHashRuns=`)는 없는 것으로 읽히므로 스크립트는 해시 판마다 `-ExecCmds=quit`를 함께 넘긴다 — 맞는 판은 Init 안에서 끝나 그 quit가 돌지 않고, 이름이 틀린 판은 첫 프레임에 `CosmicHash:` 줄 없이 끝나 실패로 읽힌다.
- **저장**: `-Seed=`가 문을 닫는다(이미 있는 규칙). 테스트의 덮어쓰기가 문을 연 판에서는 거절한다. 액터가 `BeginPlay`를 받지 않으므로 컨트롤러의 `EndPlay` 종료 저장도 오지 않는다(`Actor.cpp:3221-3229`).

### 2.4 시작의 빌드 줄 (결정 8)

- `DefaultGame.ini` `[/Script/EngineSettings.GeneralProjectSettings]`에 `ProjectVersion=0.1.0`(MVP 동안 그대로 — 빌드를 가르는 것은 아래의 커밋이다).
- `package.ps1`이 UAT 뒤 `<빌드 폴더>\Windows\Cosmic_Ascension\BuildInfo.txt`를 쓴다: 짧은 커밋, 깨끗/더러움, UTC 시각, UAT 인자. 패키지의 `FPaths::ProjectDir()`가 그 폴더다.
- `ACosmicGameMode::StartPlay`의 맨 앞(해시 판보다 먼저) Display 한 줄: `CosmicBuild: version 0.1.0, commit 3f77025, cooked` — `ProjectVersion`, `ProjectDir()/BuildInfo.txt`의 커밋(없으면 `-`), `FPlatformProperties::RequiresCookedData()`로 `cooked`/`editor`. 구성(Development)은 엔진이 이미 `LogInit: Build Configuration:`으로 찍는다. 테스터의 로그가 어느 커밋의 빌드인지 알게 한다. Shipping에서는 찍히지 않는다(§1.3) — `BuildInfo.txt`는 폴더에 남는다.

### 2.5 건너뛴 종료 저장의 줄 (결정 9)

- `CosmicSave: exit - the slot already holds this state`를 Verbose에서 Log로(`CosmicSaveSubsystem.cpp:220-226`). 손 확인 ⓐ 3걸음은 `(Exit)` 줄을 기대하는데, 답을 저장한 직후처럼 상태가 같으면 쓰기를 건너뛰고 그 흔적이 기본 수준에서 보이지 않았다. 상태·해시에 닿지 않는 낱말 하나.

---

## 3. 도구

### 3.1 `run_checks.ps1` — 에디터 바이너리의 해시 판 단계 둘 (두 등급 모두)

- 두 균형 단계(`balance: seed 42 vs baseline`·`… with the ruler policy …`) **뒤에** 새 단계 둘:
  - `hash run: editor -game vs commandlet`: `UnrealEditor-Cmd.exe <uproject> -game -nullrhi -nosound -unattended -nosplash -Seed=42 -CosmicHashRun=3000 -ExecCmds=quit -AbsLog=<Saved/Logs/HashRun.log>` (`&`로 부른다 — 기다리고 `$LASTEXITCODE`가 선다) → 마지막 `CosmicHash:` 줄의 열쇠(`seed 42, 3000 ticks, hash H, world W`)가 **같은 실행의** `Saved/Logs/checks.log`의 마지막 `CosmicBalance: seed` 줄에서 자른 열쇠와 같아야 한다.
  - `hash run: editor -game policy vs commandlet`: 같은 명령에 `-CosmicHashPolicy` → 열쇠가 `checks_ruler.log`의 `CosmicBalance: seed` 줄의 열쇠와 같아야 한다.
  - 두 단계는 기준 파일을 읽지 않는다 — '커맨드렛 = 기준값'은 균형 단계의 일이고, 이 단계는 '해시 판 = 커맨드렛'만 본다. 그래서 `-UpdateBaseline`에 따로 할 것이 없다. 실행 전에 로그 파일을 지운다.
- `Compare-Balance`도 경로 없는 열쇠로 견준다(지금은 `wrote <경로>`까지 견주어 프로젝트를 옮기면(#8) 어긋난다 — `run_checks.ps1:94`). ruler 줄의 ` | ` 뒤는 그대로 견준다.

### 3.2 `Tools/package.ps1` (새 — 결정 4·5)

```text
powershell -ExecutionPolicy Bypass -File Tools\package.ps1 [-Root C:\CA_Builds] [-Ticks20000] [-Shipping] [-AllowDirty]
```

에디터를 닫고 돌린다(UAT `-build`는 에디터 타깃도 짓는다 — `BuildProjectCommand.Automation.cs:90-95`; 라이브 코딩 중이면 빌드가 거절된다). 모든 실행은 `Start-Process -PassThru`(인자는 따옴표를 넣은 한 문자열) → `WaitForExit(시간 제한)` → 넘치면 `taskkill /T /F`로 프로세스 나무째 끝내고 실패; 실행 전에 그 `-AbsLog` 파일과 `<이름>_2.log`를 지우고, 뒤에 종료 코드 0·로그가 시작 뒤에 쓰였음·긍정 줄을 모두 요구한다.

1. **전제**: `UnrealEditor` 프로세스가 없을 것. 루트(`-Root` 또는 환경 변수 `COSMIC_BUILDS`, 기본 `C:\CA_Builds`)에 공백이 없을 것. `git status --porcelain --untracked-files=no`가 비어 있고 빌드가 읽는 자리(`Source`·`Content`·`Config`·`Tools/data`·uproject)에 추적하지 않는 파일이 없을 것(구현 리뷰 — 새 파일을 `git add`하지 않은 빌드가 깨끗한 커밋을 이름 대지 않게; `-AllowDirty`면 모든 결과에 `DIRTY`). `python -I Tools/check_data.py`와 `generated.sha1`의 다시 셈(CR을 뺀 SHA1 — `make_data_tables.py:266`과 같은 셈, 에디터 없이)이 맞을 것.
2. **빌드 폴더**: `<Root>\<yyyyMMdd-HHmm>-<짧은 커밋>` — 패키지마다 새 폴더(지난 빌드의 `Saved\`·컨테이너가 섞이지 않는다).
3. **UAT**: `RunUAT.bat BuildCookRun -project=<uproject> -platform=Win64 -clientconfig=Development -build -cook -stage -pak -iostore -prereqs -stagingdirectory=<빌드 폴더> -utf8output -unattended -noP4` (`-archive` 없음 — 빌드 폴더가 곧 보관본). 출력을 `<빌드 폴더>\Logs\uat.log`에, 쿠킹 로그(`%APPDATA%\Unreal Engine\AutomationTool\Logs\…\Cook-*.txt`의 가장 새 것)를 그 옆에 둔다. 쿠킹 요약 'N error(s), M warning(s)'에서 N > 0이면 실패, 알려진 목록(`LogSockets … SE_HOST_NOT_FOUND`) 밖의 경고는 찍는다.
4. **`BuildInfo.txt`**(§2.4)를 쓰고, **건넬 zip**을 만든다: `<빌드 폴더>-handoff.zip` ← `Windows\`(아직 아무것도 돌지 않아 `Saved\`가 없다; PDB는 넣는다 — 충돌의 호출 스택). 빌드 폴더는 지우지 않는다(테스터의 빌드와 맞는 PDB가 남는다).
5. **시작 점검**: 진짜 exe `-nullrhi -nosound -unattended -nosplash -ExecCmds=quit -AbsLog=<빌드 폴더>\Logs\start.log` → 긍정 줄 `CosmicBuild:`·`NewGalaxy:`(새 은하가 ST_CA의 성역 이름을 읽는다 — `GalaxySimulationSubsystem.cpp:1047`), 부정 줄 `Data table .* is missing`·`LogStringTable: Warning`·`LogCosmic: Error`.
6. **ⓓ — 3,000틱**: 에디터 `-game`과 패키지의 진짜 exe를 같은 인자(`-Seed=42 -CosmicHashRun=3000 -nullrhi -nosound -unattended -nosplash -ExecCmds=quit -AbsLog=…`)로 → **두 열쇠가 같아야 한다(`hash`와 `world` 둘 다 — 하나만 달라도 실패, 다른 쪽을 찍는다)**. 같은 일을 `-CosmicHashPolicy`로 한 번 더. 그리고 따로 이름 붙인 검사로 에디터 열쇠 = `seed42.json`의 열쇠(어긋나면 'baseline stale — run_checks -UpdateBaseline'; 바이너리의 갈림으로 읽지 않는다).
7. `-Ticks20000`: 6을 20,000틱으로(기준 파일의 20,000틱 값은 손으로 적는 기록이라 관문으로 쓰지 않는다 — 같은 호출의 에디터 판과만 견준다).
8. `-Shipping`: Shipping 게임 타깃을 컴파일만 한다(`Build.bat Cosmic_Ascension Win64 Shipping -project=<uproject>`) — 쿠킹·실행 없음(결정 1).
9. 끝에 빌드 폴더·zip의 크기, 걸음마다의 시간, 저장과 로그의 자리, 쿠킹 경고의 수를 찍는다.

- **시간 제한과 판정(구현)**: UAT 90분, 해시 판(게임·에디터) 900초(20,000틱 3,600초), 시작 점검 300초, Shipping 컴파일 45분 — 넘치면 프로세스 나무째 끝내고(`taskkill /T`) 실패. 에디터 판도 같은 함수로 띄운다(인자는 따옴표를 넣은 한 문자열 — 프로젝트 경로에 공백이 있다). zip은 `-ErrorAction Stop`으로 만들고 안의 파일 수가 빌드의 파일 수와 같아야 한다. 열쇠는 요청한 판(시드 42·그 틱·정책 꼬리표)과 맞아야 읽힌다. 기준 파일과의 비교는 따로 이름 붙인 단계(`baseline: editor vs seed42.json`)다. UAT의 쿠킹 로그는 `uebp_LogFolder` 또는 `%APPDATA%\Unreal Engine\AutomationTool\Logs` 아래에서 이 실행 뒤에 쓰인 것을 찾는다.

- ⓓ가 어긋나면 `-CosmicHashEvery=100`(또는 1)으로 6을 다시 돌려 두 로그의 `CosmicHashAt:` 줄을 견주어 처음 갈리는 틱을 찾는다 — 틱 0에서 갈리면 생성·데이터, 틱 t에서 처음 갈리면 그 틱의 단계(§8).
- 도구의 검사도 한 번씩 깨뜨린다(§7): 6에서 `-Seed=`를 빼면 `refused`로 실패, 5의 낱말에 걸리는 줄을 넣으면 실패, 지난 로그를 남기고 exe 경로를 틀리면 '통과'가 아니라 실패.

### 3.3 `Tools/check_layers.py` — 비인라인 수학을 막는 규칙 (결정 15)

- **규칙 3b**(셋째 규칙 '`FMath::Rand*` 없음' 옆): Core Logic(Core·Data·Sim·Events·Ascension·Log·Player·Save)은 `FMath`(`FPlatformMath`·`FGenericPlatformMath`로 써도)의 짧은 목록(`Max`·`Min`·`Clamp`·`Abs`·`Sign`·`Square`·`Sqrt`·`RoundToInt32`·`IsNaN` — 인라인의 + − × ÷와 √)만 부르고, CRT 수학은 `sqrt` 말고 부르지 않는다(`pow`·`exp`·`log`·삼각·쌍곡·`atan2`·`fmod`·`cbrt`·`hypot` … — 이름을 댄 함수의 거부 목록, 그냥·`std::`·`::`, `f`·`l` 꼴까지). 목록은 필요할 때 일부러 늘린다. 문자열 안의 낱말('the log (…)')은 부름이 아니다(문자열을 지운 뒤 찾는다). 오늘의 코드는 통과한다. 불변 규칙 6의 첫 도구이고, 모놀리식 exe의 COMDAT 갈림(§1.4) 가운데 가장 흔한 길 — 비인라인 엔진 수학(`Fmod`·`Atan2`·`TruncateToHalfIfClose` …) — 을 막는다. (구현 리뷰가 `FMath` 쪽의 처음 거부 목록을 허용 목록으로 바꿨고, `::pow` 꼴은 A-9d 재감사에서 더했다. 헤더의 벡터·난수 도우미는 린트 밖이다 — `docs/20` §9.2.)

---

## 4. 패키지에서 하는 손 확인과 건네기

### 4.1 손 확인 ⓐ (결정 6)

헤더 A5 §10의 여덟 걸음을 **패키지에서** 한다(에디터 `-game`의 ⓐ는 세지 않는다 — 테스터가 돌리는 것은 패키지다). STATUS 다음 작업 1의 다른 몫(Phase 3의 60fps·성역, MVP ⑥⑦⑧의 눈 확인, ⓒ)도 같은 패키지에서 한 자리로 한다 — 그래서 손 확인은 A-9b 뒤로 간다. 바뀌는 것만:

- **띄우기**: `<빌드 폴더>\Windows\Cosmic_Ascension.exe -windowed -ResX=1280 -ResY=720` (`package.ps1`이 찍은 폴더; 부트스트랩이 인자를 넘긴다). `-Seed=`·`-NoSave`·`-unattended`·`-Speed=` 없이. 로그의 앞부분(서브시스템 초기화와 StartPlay 즈음)에 `CosmicSave: the game's saving is allowed by the command line`과 `CosmicBuild: version 0.1.0, commit <커밋>, cooked`가 있어야 한다.
- **자리**: 저장 `<빌드 폴더>\Windows\Cosmic_Ascension\Saved\SaveGames\`, 로그 `…\Saved\Logs\Cosmic_Ascension.log`(다시 띄우면 이전 것은 `Cosmic_Ascension-backup-<이전 로그를 마지막으로 쓴 UTC 시각>.log` — 이름의 시각이 한국 시각보다 9시간 이르다; 판이 끝난 세션은 이름이 아니라 내용으로 고른다 — `Select-String -Path …\Saved\Logs\*.log -Pattern 'CosmicEnd:'`). 새 빌드 폴더라 '새 판부터'가 저절로다.
- **다시 띄우기 전**: 창을 닫은 뒤 프로세스가 끝나기를 기다린다(첫 패키지에서 1초 안팎, 시험 패키지에서 한 번 약 19초 — 작업 관리자에 `Cosmic_Ascension`이 없을 때까지, 또는 `Get-Process Cosmic_Ascension`이 비면). `Cosmic_Ascension_2.log`가 생기면 너무 일찍 띄운 것이다.
- **6걸음(강제 종료, 고를 수 있음)**: 작업 관리자 대신 `taskkill /F /T /IM Cosmic_Ascension.exe` — 부트스트랩과 게임이 같은 이름의 두 프로세스이고 부트스트랩을 끝내도 게임은 남는다. 다시 띄운 뒤 끝낸 세션의 로그에 `(Answer)` 줄이 있고 `(Exit)` 줄이 없어야 한다.
- **더하는 눈 확인**: 한글 라벨·카드·로그 피드가 네모 없이 그려지는지, 상단 바의 ▲▼, 조사(`|hpp` — '{Civ}은/는'), 줄바꿈(결정 10); ⓒ는 새 판에서 마우스만으로(×1·×10 단추를 누른다 — Space 대신) 셋째 칸 → 연대기.
- **인자 없이 한 번**: `Saved\Config`를 지우고 부트스트랩을 더블클릭 — 테두리 없는 전체 화면(테스터가 보는 것)에서 상단 바·패널·카드가 화면 해상도에 맞게 놓이는지, Alt+F4로 끝내고 `(Exit)` 줄.
- 끝나면 `CosmicSave:`·`CosmicContinue:`·`CosmicEnd:` 줄과 위 자리를 CHANGELOG에 한 줄.

### 4.2 ⓔ에 건네는 것 (결정 12)

- `package.ps1`이 만든 `<빌드 폴더>-handoff.zip`만 건넨다(`Saved\` 없음, PDB 있음, `BuildInfo.txt` 있음). 아카이브 폴더나 손 확인을 한 폴더는 건네지 않는다 — 개발자의 세이브·로그·설정이 따라가고, 테스터의 첫 시작이 개발자의 판을 이어 연다.
- **기술 안내**(게임을 설명하지 않는다 — '설명 없이'는 그대로): zip을 모두 풀어 쓸 수 있는 폴더(OneDrive·Program Files 밖, 예 `C:\Games`)에 둔다(Development는 저장을 exe 옆에 둔다), 위의 `Cosmic_Ascension.exe`를 띄운다, SmartScreen이 막으면 '추가 정보 → 실행', VC++ 설치 창이 뜰 수 있다, 끝내기는 Alt+F4, 돌려줄 것은 `Cosmic_Ascension\Saved\Logs\*.log`. 최소 사양: Windows 64비트, DirectX 11/12 GPU. 안내의 글은 ⓔ 걸음에서 쓴다(이 헤더는 내용만 정한다).
- **패키지의 세이브는 배포가 아니다**: 다음 빌드로 이어 쓰지 않는다(새 빌드는 새 폴더) — '배포된 세이브 없음'(`docs/20` §9.1)과 행 이름 동결 시점(Phase 5)이 그대로다.

---

## 5. 테스트 (새 단언은 변이로 한 번씩 깨뜨린다)

| 테스트 | 단언 |
|---|---|
| `Cosmic_Ascension.Sim.HashRun` (새 — `Tests/CosmicHashRunTests.cpp`) | ① `SwitchOf`: `-CosmicHashRun=3000` → Ticks 3000, 대소문자 무관, `=0` → Ticks 0, `=100000` → 100000; 없음 → Absent; `=abc`·`=`·`=+3000`·`=3000x`·`=-5`·`=100001` → Bad(그 글); `-CosmicHashEvery=`의 같은 경우; `-CosmicHashPolicy` → bPolicy ② idle `Run(42, 300)`의 두 해시가 같은 sim에서 `SetTellAll(true)` → `NewGalaxy(42, 0, 0)` → `RunHeadless(300)`의 두 해시와 같고 틱이 300; `Ticks 0`은 생성된 은하의 두 해시(틱 0) ③ 말하기가 범위(`false`)였어도 idle 판은 전부 말하기의 해시를 내고(범위로 돈 판의 전체 해시와 다르다 — 대조군; 세계 해시는 같다) 끝난 뒤 범위로 돌아온다; 로그 줄(`OnLog`)에 묶은 탐침이 `NewGalaxy` 안의 첫 줄 때 `IsTellAll() == true`를 읽는다(켜기가 `NewGalaxy` 앞 — 구현에서 `OnGalaxyGenerated`를 바꿨다) ④ 열 테이블을 하나씩 비우면 매번 `MissingData`와 그 이름, 규칙 행·고향 은하 행이 없어도 그렇다; 그때 `NewGalaxy`를 부르지 않는다(`GalaxyID`·틱 그대로) ⑤ 정책 판 `Run(42, 300, policy)`가 같은 sim에서 손으로 짠 루프(`SetTellAll(false)` → `NewGalaxy` → 틱마다 `RunHeadless(1)`과 `CosmicRulerPolicy::Act`)와 같은 두 해시이고, 입력이 실제로 들어갔다(답·시작이 0보다 크다 — 대조군; idle 판과 세계가 다르다) ⑥ `Every 100`의 300틱 판이 체크포인트 넷(0·100·200·300)을 내고 마지막 해시가 한 번에 돈 판과 같다(나눠 부른 `RunHeadless` = 한 번) ⑦ `LineOf`의 세 꼴(idle·정책·거절)이 문자열 그대로 |
| `run_checks` 단계 둘(§3.1) | 에디터 바이너리의 해시 판 열쇠 = 같은 실행의 커맨드렛 열쇠(idle·정책) — 테스트가 아니라 러너의 단계다 |
| 커맨드렛의 정책 옮김(§2.2) | `ruler_summary_line` 그대로(균형 단계) |
| `check_layers.py` 규칙 3b(§3.3) | 지금의 코드는 통과; Sim 파일에 `FMath::Fmod`·`FPlatformMath::Fmod`·`FMath::TruncateToHalfIfClose`·`std::pow`·`asinf`·`std::tanh`·`::pow`(A-9d)를 넣으면 에러, `TEXT("… the log (…)")`는 통과(변이 — 되돌린다) |

- `StartPlay`의 분기: 러너와 `package.ps1`은 두 길(스위치 없음·맞는 스위치)만 본다(게임 모드를 테스트에서 다시 시작하지 않는다). 거절은 손으로 본다 — A-9a에 둘(`-CosmicHashRun=abc -Seed=42`, `-CosmicHashRun=10`만), A-9d에 지금의 바이너리로 다섯(그 둘과 `-CosmicHashPolicy -Seed=42`·`-CosmicHashEvery=5 -Seed=42` → `no -CosmicHashRun=`, `-CosmicHashRun=10 -CosmicHashEvery=x -Seed=42` → `bad -CosmicHashEvery=x`; 모두 Error 한 줄, `NewGalaxy` 없이 약 9초에 끝남 — 대조군 `-CosmicHashRun=10 -CosmicHashEvery=5`는 체크포인트 셋과 줄). 저장 켬(`-Seed=`가 저장을 끈다)·시뮬 없음은 명령 줄로 닿지 않는 방어다.
- **구현(A-9a)**: ①의 Bad에 `=0000001`·`=3,000`·`=30)`을 더했다(값은 다음 공백까지 글로 읽는다) — 단언의 원본은 `docs/40`의 `Sim.HashRun` 행.

---

## 6. 문서 (A-9c)

`docs/40_DEV_GUIDE.md`(빌드·실행·테스트 — 패키징 명령과 `package.ps1`, 러너의 새 단계 둘과 경로 없는 열쇠, PowerShell 5.1의 `Start-Process` 따옴표와 기다림, 패키지에서 하는 손 확인 ⓐ와 자리, ⓔ의 건넬 zip, 테스트 표 `Sim.HashRun`, `check_layers.py`의 규칙 3b; `CLAUDE.md`의 빌드 요약은 '-Ticks=20000'이라 적었지만 러너는 3,000이다 — 바로잡는다), `docs/45_ENGINE_NOTES.md`(UAT·스테이징·레이아웃·Zen/IoStore와 대체 길, 저장·로그 자리와 `FApp::IsInstalled`·`-SaveToUserDir`, Shipping의 로그·콘솔과 설치 엔진, 게임 exe의 커맨드렛 거절, Exec 기본 인자, 우아한 종료의 코드와 충돌 3, `-AbsLog`의 `_2`, 창의 기본 모드와 19초, 모놀리식 링크의 COMDAT, 스트링 테이블의 로그 낱말, `:97`의 '⑨가 본다'를 사실로), `docs/20_ARCHITECTURE.md` §9.1(패키지의 슬롯 자리)·§9.2(쿠킹 빌드의 비교 — 두 해시, idle·정책 판, 결과와 되풀이 시점; '헤더 인라인 포함'의 문장을 COMDAT에 맞게)·`:235`·부록 규칙 6 전문(`:295`·`:300` — 쿠킹 빌드는 두 해시로 견준다, 린트), `docs/21_DATA_MODEL_REFERENCE.md`(`CosmicHashRun`·`CosmicRulerPolicy`), `docs/50_ROADMAP.md`(A ⑨ 끝, ⓓ 결과, 위험 표, 미룬 확인 8), `docs/00_VISION.md`(필요하면), OPEN_QUESTIONS(#8에 패키지 자리 한 줄, 새 행 — 테스터 빌드 전의 구성·스위치·치트·저장 폴더·서명·끝내기 단추), `DECISIONS_LOG`, `STATUS.md`(다음 작업 1을 패키지의 한 자리로), `CHANGELOG.md`, `CLAUDE.md`(규칙 6에 린트 한 마디·규칙 15의 ⑨·빌드 요약), `WORKPLAN.md` §2(마일스톤 A·패키징의 effort 행), `Save/CosmicSaveSubsystem.h:81`의 주석.

---

## 7. 순서·검증·effort

| 걸음 | 내용 | effort · ultracode |
|---|---|---|
| A-9a | `World/CosmicHashRun`·`World/CosmicRulerPolicy`(커맨드렛의 정책을 옮김)·`StartPlay`의 맨 앞·빌드 줄·저장 줄 한 낱말·`DefaultGame.ini` `ProjectVersion`·`check_layers.py` 규칙 3b·`Tests/CosmicHashRunTests.cpp`·`run_checks.ps1`의 단계 둘과 경로 없는 열쇠. 새 .h → 에디터를 닫고 전체 빌드. `run_checks.ps1 -Tier full` 두 번(기준값 그대로 — 정책 옮김이 `ruler_summary_line`을 바꾸지 않고, 새 단계 둘이 해시 판 = 커맨드렛을 보인다). 변이: `Sim.HashRun`(⑦까지 — 예: `SetTellAll(true)`를 `NewGalaxy` 뒤로, 테이블 하나의 검사를 빼기, `Every`의 마지막 조각 빼기), 러너 단계(`-CosmicHashRun=2999`로 실패), 린트(`FMath::Fmod`). 거절 둘을 손으로. 구현 리뷰(`StartPlay`의 새 길과 저장 문, 정책 옮김 — 워크플로) | xhigh(`WORKPLAN` — 결정론에 닿는 코드), ultracode는 구현 리뷰에만 |
| A-9b | `Tools/package.ps1`, 첫 패키지, ⓓ(3,000틱 idle·정책, `-Ticks20000`), `-Shipping` 컴파일 한 번, 시간·크기·자리를 이 헤더의 측정 절(§1.7 — 새로)에. 도구의 검사를 한 번씩 깨뜨린다(§3.2 끝). **ⓓ가 어긋나면 A-9b는 멈춘다** — `-CosmicHashEvery`로 좁힌 뒤의 코드 수정은 보탬 헤더(`docs/20` §9.2 먼저, `run_checks` 두 번, 세계가 움직일 때만 `-UpdateBaseline`) | medium(기계적인 빌드와 측정), 어긋나면 xhigh |
| A-9c | §6의 문서, 에디터 없는 점검 5종, `run_checks.ps1 -Tier smoke`, 문서 재감사 | xhigh(`WORKPLAN` — 설계 문서 변경), 재감사는 ultracode 워크플로 |
| 손 확인 | §4.1 — 개발자, 패키지에서 한 자리로. 끝나면 CHANGELOG 한 줄. ⓔ는 그 뒤(§4.2의 zip과 안내) | — |

---

## 8. 위험과 어긋났을 때의 길

- **ⓓ가 어긋나면**: (1) 시작 점검의 낱말(빠진 테이블·스트링 테이블·`LogCosmic: Error`)과 해시 판의 `MissingData`, (2) 같은 소스인지 — `package.ps1`은 깨끗한 트리·데이터 도장·같은 호출의 빌드와 에디터 판을 요구하므로 남는 것은 바이너리의 갈림이다, (3) `-CosmicHashEvery`로 처음 갈리는 틱을 찾는다: 틱 0이면 생성(데이터 행·FName과 태그·`Sqrt`의 자리), 틱 t면 그 틱의 단계에서 비인라인 엔진 수학·COMDAT로 엔진 사본을 탄 inline 함수(§1.4)·결정에 쓰인 FName의 `ToString`·태그 등록을 의심한다. 멤버별 해시나 틱별 추적은 ⓓ가 어긋날 때만 보탬 헤더로 짓는다. 고치기 전에는 테스터와 시드를 나누지 않는다(로드맵 미룬 확인 8).
- **Zen 서버가 뜨지 않으면**: `DefaultGame.ini`의 `[/Script/UnrealEd.ProjectPackagingSettings]`에 `bUseZenStore=False` 하나(쿠커가 읽는 값은 `bUseZenStore && bUseIoStore` — `ProjectPackagingSettings.h:667-670`), 또는 ini는 두고 UAT에 `-AdditionalCookerOptions=-SkipZenStore`(`CookCommandlet.cpp:425-427`). `-iostore`는 그대로 둔다(ⓓ가 잰 런타임 로더 그대로). 다시 패키징하기 전에 `Saved\Cooked\Windows`를 지운다(빌드 폴더는 늘 새것이다).
- **OneDrive**: 빌드 폴더는 OneDrive 밖에 둔다(결정 5). 프로젝트의 `Saved\Cooked`·`Intermediate`는 OneDrive 안에 남는다(#8).
- **미래의 갈림**: 비인라인 엔진 수학(린트가 막는다 — §3.3), 인라인되지 않은 헤더 함수의 COMDAT, `/arch`·LTCG·PGO, 결정에 쓰이는 FName의 `ToString`, `check()` 안의 부수 효과(Shipping에서 사라진다), 상태의 `FString` — 에디터 테스트는 어느 것도 잡지 못한다. ⓓ를 되풀이하는 때는 결정 13.
- **디스크**: 빌드 폴더 하나와 zip이 약 1.6 GB(zip은 압축된 컨테이너라 크게 줄지 않는다). 지난 빌드 폴더는 손으로 지운다(테스터에게 건넨 빌드의 폴더는 그 PDB 때문에 남긴다).

---

## 9. 개발자 결정 (권장안 — 2·3·5·6을 먼저 본다)

1. **구성**: ⑨의 패키지는 **Development**(ⓐ·ⓓ·ⓔ 모두 — 로그·`-ExecCmds`가 있어야 두 확인이 된다). Shipping은 게임 타깃을 **컴파일만 한 번**(`package.ps1 -Shipping`) — 쿠킹·실행은 Phase 5 테스터 빌드 전에(그때 로그 없는 판정 길과 `%LOCALAPPDATA%` 저장을 함께 정한다).
2. **ⓓ의 해시를 내는 길**: 게임 스위치 `-CosmicHashRun=<틱>`(§2 — `World/`, `StartPlay` 맨 앞, `-Seed=` 필수, 저장이 켜져 있거나 값이 틀리면 거절하고 끝남, `0`은 생성된 은하, `-CosmicHashEvery`의 체크포인트). *대안*: 커맨드렛 하위 클래스(`IsEditor=false`·`IsClient=false`)를 패키지에서 `-run=`으로 — 같은 코드지만 게임 엔진 아래의 커맨드렛(둘째 GameInstance, Slate 없는 시작)이 확인되지 않은 엔진 길이다.
3. **견주는 것**: 3,000틱의 **idle 판과 통치자 정책 판 둘 다**, 두 해시 모두(하나만 달라도 실패) — 정책 판을 위해 커맨드렛의 정책을 `World/CosmicRulerPolicy`로 옮겨 둘이 함께 쓴다(§2.2 — 쿠킹 빌드가 답의 추첨·카드의 효과·프로젝트 시작·연구 중점·이야기의 범위를 지난다). 20,000틱은 `-Ticks20000`이 같은 호출의 에디터 판과 견준다. *대안*: idle만(정책 길은 Phase 5까지 쿠킹 빌드에서 확인되지 않은 채로 둔다).
4. **러너**: `run_checks.ps1`의 두 등급에 '에디터 `-game` 해시 판 = 같은 실행의 커맨드렛'(idle·정책) 단계 둘을 균형 단계 뒤에, 그리고 `Compare-Balance`를 경로 없는 열쇠로.
5. **자리와 보관**: 패키지마다 `C:\CA_Builds\<yyyyMMdd-HHmm>-<커밋>`(환경 변수 `COSMIC_BUILDS`로 바꿈 — 공백 없이)에 스테이징만(`-archive` 없음 — 새 폴더라 지난 빌드의 세이브·컨테이너가 섞이지 않는다). IoStore·Zen은 5.8 기본, `-prereqs` 켬, PDB는 빌드 폴더와 zip 모두에. 깨끗한 트리에서만(`-AllowDirty`는 `DIRTY`를 찍는다). `-SkipBuild`는 두지 않는다(35초를 아끼려다 다른 커밋의 바이너리를 섞는다).
6. **손 확인은 패키지에서 한 자리로**(§4.1): ⓐ는 패키지에서만 세고, STATUS 다음 작업 1의 다른 몫(60fps·성역·MVP ⑥⑦⑧의 눈 확인·ⓒ)도 같은 패키지에서 — 그래서 다음 작업 1은 A-9b 뒤로 간다. 인자 없는 더블클릭 한 번을 더한다.
7. **저장 폴더**: 구성의 기본값(Development는 패키지 안 `Cosmic_Ascension\Saved\SaveGames\`) — 실제 자리를 문서에 적는다. `-SaveToUserDir`(어디에 풀어도 `%LOCALAPPDATA%`)는 테스터 빌드 전의 열린 물음.
8. **빌드 줄**: `ProjectVersion=0.1.0`(MVP 동안 그대로)과 `BuildInfo.txt`(커밋·깨끗함·시각·UAT 인자), 시작의 `CosmicBuild:` Display 한 줄(버전·커밋·cooked/editor).
9. **건너뛴 종료 저장의 줄을 Log로**(§2.5).
10. **국제화**: 엔진 기본(English 프리셋, `en`) 그대로 — 시험 패키지에서 한글이 그려졌다. 손 확인에서 줄바꿈·조사가 어긋나면 `InternationalizationPreset=EFIGSCJK`(영어 ICU를 남긴다 — `CJK`는 영어를 뺀다, `ProjectPackagingSettings.h:51-55`)와 `+CulturesToStage=ko`(표시뿐 — 해시는 FText를 넣지 않는다).
11. **개발 스위치·Exec 명령(콘솔의 `CosmicSeeAll` 등)·테스트 코드·커맨드렛은 패키지에 남는다**(ⓔ도 개발 빌드를 받는다). Phase 5 테스터 빌드 전에 다시 본다 — 열린 물음 새 행(구성·스위치·치트·저장 폴더·서명·끝내기 단추를 함께).
12. **ⓔ에게는 저장 없는 zip만**(§4.2 — PDB 포함, 기술 안내의 내용을 여기서 정하고 글은 ⓔ 걸음에서), 패키지의 세이브는 다음 빌드로 이어 쓰지 않는다.
13. **ⓓ를 되풀이하는 때**(정해진 자리): 마일스톤마다의 끝, B8(실제 하늘 — 기준값 다시 잡기), Phase 5 테스터 빌드 전, `Build.cs`·`Target.cs`의 컴파일 설정이나 엔진 버전을 바꾼 뒤 — `package.ps1`로.
14. **effort**(§7 표): A-9a xhigh(결정론에 닿는 코드 — 구현 리뷰는 ultracode 워크플로), A-9b medium(어긋나면 xhigh), A-9c xhigh(재감사 워크플로). `WORKPLAN.md` §2에 마일스톤 A·패키징 행을 더한다.
15. **비인라인 수학 린트**(§3.3): `check_layers.py`의 규칙 3b — 불변 규칙 6을 처음으로 도구가 지킨다.

---

## 추가 이야기와 보완

- **테스터의 끝내기**: 게임에 끝내기 단추가 없다(Alt+F4·Alt+Enter뿐). ⓔ의 안내 한 줄로 넘기지만, 30분 시험의 첫 인상에는 상단 바의 작은 '끝내기'(창 닫기와 같은 길 — 종료 저장)가 낫다. 작은 UI라 B1 전에 따로 고를 수 있다(설계를 바꾸지 않으면 그 작업의 커밋에 바로).
- **창을 닫은 뒤의 19초**(시험 패키지에서 한 번 — 첫 패키지(§1.7)에서는 다시 나오지 않았다): 렌더러를 내리는 쪽으로 보이지만(`-nullrhi`에서는 없다) 원인을 보지 않았다. 손 확인 ⓐ의 다시 띄우기를 늦게 하는 것 말고는 해가 없으나, 테스터에게는 '안 꺼진다'로 읽힐 수 있다 — 손 확인 때 시간을 한 번 재고 길면 프로파일한다.
- **정책 판의 공유 함수**는 B의 AI 정책(B2 — 연구 중점의 AI)과 스크립트 입력(리플레이)의 씨앗이 될 수 있다 — 이번에는 커맨드렛의 동작을 비트 하나 바꾸지 않고 옮기기만 한다.
