# MVP ⑤ 구현 메모 — 저장 최소형 (헤더 승인 뒤 A-5a~A-5d의 메모)

헤더 `docs/60_DECISIONS/headers/A5_save_minimum.md`에서 옮긴 것이다. 승인할 것은 헤더의 선언·슬롯 머리의 바이트 꼴·ST_CA 키와 글·결정이다. 이 파일은 구현자가 따를 본문 스케치, private 멤버, 테스트의 값과 변이 목록, 코드와 엔진 사실의 표, 고칠 문서의 목록이다. 헤더와 어긋나면 헤더가 이긴다. 결정을 대안으로 정하면 해당 절을 그 대안으로 바꿔 읽는다(결정 2 대안 ㉯ → §3.4의 상단 바 '새 판', 결정 3 대안 ㉮ → §2.3의 `WriteSlotFile`을 `SaveDataToSlot`으로, 결정 6 대안 ㉮ → §3.1의 `Unreadable` 가지에서 `HoldStartRun` 대신 `RequestSave(NewRun)`(그러면 `HoldStartRun`·`HeldRunID`·종료 건너뜀과 `Sim.SaveTriggers`의 변이 ⑦이 빠진다), 결정 9 대안 ㉮ → §2.3의 '지우기', 결정 14 대안 ㉮ → §4의 밀어냄 기록을 `int64` 하나로, 결정 16 대안 → §2.4의 핸들러를 컨트롤러로). 2026-10-23 적대 리뷰(ultracode — 다섯 관점)의 발견을 헤더와 함께 넣었다.

## 0. 코드 사실 — 자리와 행 번호 (HEAD `e71751b`, 2026-10-23에 파일을 열어 확인)

`S/` = `Source/Cosmic_Ascension/`, `E/` = `C:\Program Files\Epic Games\UE_5.8\Engine\Source\`.

| 자리 | 파일:행 | 사실 |
|---|---|---|
| 저장 객체 | `S/Save/CosmicSaveGame.h:16-24` | `UCosmicSaveGame : USaveGame { UPROPERTY() FGalaxyState State; }`. 주석 13-14는 '`SaveGameToSlot`으로 CDO 대비 델타' |
| 저장 Subsystem 선언 | `S/Save/CosmicSaveSubsystem.h:15-43` | `MakeSlotName`(24, static), `SaveCurrentGalaxy`(28), `LoadGalaxy`(36), `DoesSaveExist`(39), `DeleteSave`(42). 클래스 주석 11-14 '자동 저장 트리거가 틱 경계에서 부른다'. include는 `CoreMinimal`·`Misc/Guid.h`·`Subsystems/GameInstanceSubsystem.h`뿐 |
| 저장 본문 | `S/Save/CosmicSaveSubsystem.cpp:17, 20-29, 31-34, 36-54` | `SaveUserIndex = 0`. `Initialize`는 로그만, `Deinitialize`는 빈 것. 이름 = `"Galaxy_" + ToString(Digits)`. 저장 = GalaxyID 검사 → `SyncRngState`(46) → `NewObject` 복사 → `SaveGameToSlot`(51). **방송·틱 단계 거절 없음** |
| 로드 본문 | `S/Save/CosmicSaveSubsystem.cpp:56-108` | 방송 Error(63) → `DoesSaveGameExist ? LoadGameFromSlot`(68-69) → "missing or unreadable"(73) → "truncated or holds another galaxy"(83) → 버전 Warning(88) → 성역 Error(99-102) → `LoadState`(106) |
| 있음·지움 | `S/Save/CosmicSaveSubsystem.cpp:110-118` | `DoesSaveGameExist` / `DeleteGameInSlot`, 주 슬롯 하나만 |
| 게임 코드의 부름 | (검색) | 게임 코드는 저장 함수를 부르지 않는다. 유일한 언급은 주석 `S/Log/CosmicLogSubsystem.h:122`. 사용자 정의 `UGameInstance`가 없다 |
| `FGalaxyState` | `S/Core/GalaxyState.h:694-776` | 최상위 18. `CurrentSaveVersion = 1`(700), `SaveVersion = 1`(706), 멤버 차례 `SaveVersion`·`Seed`·`Tick`·`RngState`·`TellRngState`·`GalaxyID`·`NextCivSerial`·`Systems`… , `Log`(764-766 "Capped (section 8.4)"), `Chronicle`(768-771), `Flags`(775). 머리 주석 4-13(저장 호환)·15-28(결정론, 19-21 '제외는 최상위 이름으로'). include는 `CoreMinimal`·`Misc/Guid.h`·`GameplayTagContainer.h`·`Core/CosmicTypes.h`(32-36) |
| 저장 상태 구조체의 꼴 | `S/Core/GalaxyState.h:42-43, 433-434, 525-526` | `USTRUCT(BlueprintType) struct COSMIC_ASCENSION_API F…` |
| 생애·부름 | `S/Core/GalaxyState.h:426, 442-478, 536-561` | `ELifeState`(426). `FLifeRecord`: `BornTick`·`DiedTick`·`BirthCivID`·`RuledCivIDs`·`PeakTier`. `FCallingState`: `LifeState`(539)·`Lives`(550)·`PersonalTier`(553)·`CompletedFeats`(561) |
| 그릇 | `S/Core/GalaxyState.h:336-356`, `S/Core/CosmicTypes.h:27` | `FPlayerEntityState`: `Role`(342)·`RuledCivID`(356). `EPlayerRole`은 `CosmicTypes.h:27` |
| 앎의 청원 둘 | `S/Core/GalaxyState.h:639, 644` | `PendingPetitionRow`·`PendingPetitionSystemID` — 예약 도우미가 빠뜨렸다(전체 해시와 `Sim.SavePetition`이 덮는다) |
| 규칙 행 | `S/Data/CosmicDefinitionRows.h:720` | `FCosmicRulesDef`. 앞선 선언의 꼴은 `S/Sim/CosmicEndings.h:10`·`S/Events/CosmicEventSubsystem.h:38` |
| 해시 | `S/Sim/GalaxySimulationSubsystem.cpp:1305-1349` | `NotWorldNames`(1310-1315), 반사 고리(1318), `RngState`는 실시간 스트림(1336-1341). 속성의 이름·순번은 해시에 들지 않는다. 0인 멤버도 든다 |
| `LoadState` | `S/Sim/GalaxySimulationSubsystem.cpp:1274-1293` | 방송 거절, `ensure(!bInTickStages)`, 상태 교체·색인·`Rng.Initialize`·`Accumulator = 0`·`bRunning`, 문이 열려 있으면 `OnGalaxyGenerated`. `TimeScale`은 그대로 |
| 틱의 꼬리 | `S/Sim/GalaxySimulationSubsystem.cpp:1351-1395` | `PetitionsAtHead` → `++Tick` → 단계 가드(1370) → `ResolvePendingRoleEffect`(1388) → `SyncRngState`(1390) → `OnSimTick`(1391-1394) |
| 델리게이트 | `S/Sim/GalaxySimulationSubsystem.h:18-19, 308, 312` | `FOnSimTick(int64)`·`FOnGalaxyGenerated(int32)`는 동적 멀티캐스트 → 핸들러는 `UFUNCTION()` |
| 문과 가드 | `S/Sim/GalaxySimulationSubsystem.h:88, 252-263, 304-305, 332, 494, 502` | `IsTickable`, 거절 목록 주석(252-260 — `UCosmicSaveSubsystem::LoadGalaxy`는 이미 256행에 있다), `IsBroadcasting`(261, public), `IsBroadcastEnabled`·`SetBroadcastEnabled`, `private:`(332), `TimeScale = 1.f`, `bInTickStages`(502, private) |
| 이관 고리 | `S/Log/CosmicLogSubsystem.cpp:185-226` | `Append`: static `ClaimTag`·`RecordTag`(187-188) → 항목 추가 → `while (Num > MaxLogEntries)` 머리(`Log[0]`, 가장 오래된 줄)를 S≥1·Claim·Record면 `Chronicle`로(208-216) → 말해진 줄은 `OnLogEntry`를 괄호 안에서 방송 |
| 로그 상한 | `S/Log/CosmicLogSubsystem.h:61` | `static constexpr int32 MaxLogEntries = 2000` — 테스트가 바꾸지 못한다 |
| 연대기 | `S/Log/CosmicChronicle.h:1-3, 28, 34, 37, 41, 50` / `.cpp:89-104, 106-115, 122-219` | 파일 규칙 '아무것도 쓰지 않는다', `bMore`(28), `MaxLifeLines` 40, `PeoplesOf`(37), `Admits(Entry, TConstArrayView<FName>)`(41), `BuildCapped`(50). 모음(145)이 `Admits`를 되풀이 규칙보다 먼저 본다. 넘침 표시는 202 |
| 노브 자리 | `S/Data/CosmicDefinitionRows.h:982-989` | `ProvisionalEndTick = 6000`(988), 구조체 닫기(989) |
| 데이터 | `Tools/data/DT_Rules.json:56`, `Tools/data/schema.json:39-43, 135-139` | 마지막 키 `ProvisionalEndTick`. `string_table.checked_prefixes`는 `Chronicle.` 하나 |
| 데이터 도장 | `Tools/data/generated.sha1`, `S/Tests/CosmicDataTests.cpp:1035-1080` | 도장에 `ST_CA.csv`·`schema.json`이 든다. `Data.SourcesInSync`가 지금의 SHA1과 견준다 → ST_CA·스키마를 고친 커밋은 `-Data` |
| 데이터 테스트 | `S/Tests/CosmicDataTests.cpp:316-342`, `Tools/test_data_schema.py:377-380, 636` | 열은 두 방향으로 잰다. 검사기 단위 테스트는 DT_Rules 열 이름을 적지 않는다 |
| 레이어 린트 | `Tools/check_layers.py:20-24, 54-56` | `CORE_LOGIC`에 Save, `FORBIDDEN_ENGINE`에 `Engine/World.h`, `BASELINE` 태그 글자 41(A4에서 38 → 41) |
| 게임 시작 | `S/World/CosmicGameMode.cpp:59-62, 96-117` | `ClockSeed()`. `GalaxyID`가 없을 때만 `NewGalaxy` + `-Speed=`(주석 108-110 '-Speed=N starts at that speed'), 그 뒤 `Super::StartPlay()`(117) |
| HUD | `S/World/CosmicGameMode.cpp:186-285, 287-298` | 카드 만들기(253-259) 직후 `FollowPetition(false)`(264), 연대기 위젯 만들기·접힘(274-279), 그 뒤 `FollowEnd(false)`(283). `EndPlay`는 묶음만 푼다 |
| 컨트롤러 스위치 | `S/World/CosmicGameMode.cpp:366-400, 418` | `-TimeUnit=`·`-AutoPause=`(389)·`-ProvisionalEnd=`(396). ini 쓰기는 `!GIsAutomationTesting`일 때만(418) |
| 컨트롤러 `BeginPlay`/`EndPlay` | `S/World/CosmicGameMode.cpp:527-554 / 556-572` | 묶기(542-547), `RunBeganRealSeconds`(549), `FollowEnd(false)`(552), `FollowOwnHazards`. `EndPlay`는 풀기만 |
| 핸들러 | `S/World/CosmicGameMode.cpp:574-585, 587-603` | `HandleSimTick` = `FollowSpeedGate` → `FollowEnd(true)` → 끝났으면 return → 위기 → 청원. `HandleGalaxyGenerated` = 게이트 → `ClearRunEnd` → 까닭·위기 지우기 → 초 → `FollowEnd(false)` → … |
| 끝의 래치 | `S/World/CosmicGameMode.cpp:605-649` | `ApplySpeed(0, RunEnd)`(621), 카드 접기(623-626), `CosmicEnd:` 줄(628-629 — `bArrived`와 상관없이 래치가 설 때마다), `Open(RunEnd, true)` |
| 새 판 | `S/World/CosmicGameMode.cpp:735-745`, `S/World/CosmicGameMode.h:261` | 방송 중 거절 → `NewGalaxy(ClockSeed(), 0, 0)` → `SetSpeed(1)`. 저장·지우기 없음. 선언 주석 261 |
| 카드 | `S/World/CosmicGameMode.cpp:1471-1495, 1497-1520, 1532` | `FollowPetition`은 끝난 판에서 곧 돌아온다(1473-1477). `AnswerPetition`은 끝난 판에서 `Refused`. 카드 닫기가 `SpeedBeforePause`로 되돌린다 |
| 배속 | `S/World/CosmicGameMode.cpp:1564-1590, 1619`, `S/World/CosmicGameMode.h:472, 478, 486, 492` | `ApplySpeed`가 지금 배속 > 0이면 `SpeedBeforePause`에 적는다(1574). Space는 `SpeedBeforePause`로(1619). 멤버 `SpeedBeforePause = 1.f`·`AutoPauseReason`·`bProvisionalEnd`·`RunBeganRealSeconds` |
| 클래스 | `S/World/CosmicGameMode.h:57, 75, 149` | `ACosmicGameMode : AGameModeBase`, `ACosmicHUD : AHUD`, `ACosmicPlayerController : APlayerController` |
| 자동 정지 열거 | `S/World/CosmicAutoPause.h:16-28`, `.cpp:50-66` | 평범한 enum class, 끝이 `RunEnd`. `WhyKeyOf`: `AutoPause.Why.*`·`Chronicle.Why` |
| 상단 바 | `S/UI/CosmicTopBarWidget.h:35, 95-101`, `.cpp:39, 118-132, 191-213, 218-226, 239-246, 290-296, 397, 487-494, 535` | `UCLASS(meta = (DisableNativeTick))`, `NativeTick` 재정의 없음. 연구 몫만 `Fill`·말줄임(123-132), 자동 정지 단추와 까닭 줄은 제 크기(199-213). `ReopenButton`의 꼴, `NativeConstruct`가 `Refresh`, 틱·새 은하마다 `Refresh`, 까닭 줄은 멈췄을 때 `WhyKeyOf` |
| 연대기 쪽의 '새 판' | `S/UI/CosmicChronicleWidget.cpp:93, 314-332, 362-381, 412-416` | 툴팁 `Chronicle.NewRunTip`, `Advance`가 `ConfirmLeft` 끝에 라벨을 되돌린다(314-332), 두 번 누름 → `StartNewRun`, 주석 370 'The run is not kept (item 4 has no save)', `NativeTick` → `Advance`(412-416) |
| ST_CA | `Tools/data/ST_CA.csv:107-112, 127-148` | `AutoPause.Why.*` 셋(11~20자), `Chronicle.*`(140 `NewRunTip` "새 은하에서 새 판을 연다. 이 판은 남지 않는다."). 148행이 끝 |
| 테스트 도우미 | `S/Tests/CosmicTestHelpers.h:66-93` | `AcquireSim`: 첫 컨트롤러의 끝 켬·잊기, `SetTimeScale(0)`, tell-all |
| 왕복 테스트 | `S/Tests/CosmicSimTests.cpp:274-300, 350-452, 1680-1844` | `FillEmptyContainers`(24 + TellRngState), `CountReservedMembersSet`, `SameReservedMembers`, `Sim.SaveRoundTrip`(기대 경고 1691-1692 — 수가 정확해야 한다, 거절 1795-1831이 `MakeSlotName` + `UGameplayStatics::LoadDataFromSlot`/`SaveDataToSlot`을 직접 부른다 1796·1800·1804·1825, 클래스 경로 찾기 1811-1821) |
| 해시 분류 | `S/Tests/CosmicSimTests.cpp:1617-1649` | 최상위 18을 하나씩 지워 두 해시를 본다(`Seen == 18`, 1649) |
| 재시작 | `S/Tests/CosmicSimTests.cpp:1846-1927` | 두 프로세스. 기록 `Saved/Automation/SaveRestart.txt`. 다른 빌드의 기록이면 `DeleteSave(RecordedGalaxyID)`(1912-1916) 뒤 첫 절반이 슬롯을 남긴다(1918). 지금 `Saved/SaveGames/`는 비어 있고 기록 파일도 없다(2026-10-23) |
| 다른 저장 테스트 | `S/Tests/CosmicKnowingTests.cpp:1653-1758`, `CosmicPetitionTests.cpp:521-556`, `CosmicSanctuaryTests.cpp:352-411` | `Sim.SightGrowsOnly`·`Sim.SavePetition`(대기 카드와 답의 왕복)·`Sim.SaveSanctuary`(Error 5) |
| 핸들러 가드 | `S/Tests/CosmicEventTests.cpp:37, 141-160, 1607, 1632, 1719, 1774`, `S/Tests/CosmicTestProbe.h:21, 33` | `FScopedProbe`(141-160 — `namespace CosmicEventTests`(37) 안이라 다른 테스트 파일에서는 보이지 않는다. A-5c가 `Tests/CosmicTestHelpers.h`로 옮긴다, §6.1)가 `OnLogEntry`·`OnEventFired`에 묶는 꼴, `UCosmicTestProbe::OnLog`(33). 1632는 `AddExpectedError(…LoadGalaxy…)`. `Events.RowAtomic`: `OnEventFired` 핸들러의 `LoadGalaxy` 거절 |
| 인물 테스트 | `S/Tests/CosmicHazardTests.cpp:2541-2575`, `Tools/data/DT_Events.json:311-341`, `S/Events/CosmicEventSubsystem.cpp:1185` | `Sim.NotablesFromEventsOnly`: 슬라이스 20,000틱, `Log ∨ Chronicle`에서 `NameNotable` 줄을 찾는다. 그 행은 `event_rebellion_rises`(Severity 1, 초점 = 그 백성) |
| 커맨드렛 | `S/Tests/CosmicBalanceCommandlet.cpp:285-295, 309-317, 396-415, 438, 448, 646-651, 690` | `SaveBytesOf`(복사 + `SaveGameToMemory`, 주석 285-286), CSV 열(`Expansion`이 끝), 스위치 `-Seed= -Ticks= -Systems= -Civs= -TellAll= -Answer= -Build= -Rules= -Out=`, 운반 객체 하나, 요약 줄(끝 `…save serialized %d bytes in %.1f ms; log %d, chronicle %d, told %lld`) |
| 러너 | `Tools/run_checks.ps1:47-52, 58-70, 76-117` | `-Data`(make_data_tables), smoke 거름 `Sim.Save`, 테스트 두 프로세스·스모크 모두 `-unattended`, `-Seed=` 없음, 커맨드렛 비교는 3,000틱 두 줄뿐 |
| 기준값 | `Tools/baselines/seed42.json` | 3,000틱 hash 2941370122 / world 1078735008, 정책 줄 hash 2361069549 / world 4271872285, 20,000틱 2410091205 / 885018087(기록 칸 `last_recorded_in_changelog` — 러너는 보지 않는다) |
| 맵 | `Config/DefaultEngine.ini:5, 8` | `GameDefaultMap=/Game/CA/Maps/L_Galaxy.L_Galaxy`, 게임 모드 `CosmicGameMode` |
| 일정 규칙 | `WORKPLAN.md:59, 65, 69, 73, 77`, `STATUS.md:20` | '저장 작업' xhigh·헤더 승인 + ultracode 리뷰(59), '설계 문서 변경' xhigh·재감사(65), 작업 유형 표의 제목 'Phase 표와 충돌하면 Phase 표가 우선'(69), '저장 포맷 → xhigh'(73), '저장 관련 버그는 xhigh'(77). STATUS '다음 3작업' 3은 '구현 high에 ultracode 끔'(헤더 결정 18) |

### 0.1 엔진 사실 (헤더 §1.2의 자리)
| 호출 | 선언 / 본문 | 기대는 점 |
|---|---|---|
| `UGameplayStatics::SaveGameToMemory(USaveGame*, TArray<uint8>&)` | `E/Runtime/Engine/Classes/Kismet/GameplayStatics.h:1134` / `Private/GameplayStatics.cpp:2371-2388` | 엔진 머리(첫 네 바이트 'GVAS' = `UE_SAVEGAME_FILE_TYPE_TAG` 0x53415647, `.cpp:87`) + 태그 속성. 객체가 있으면 늘 true |
| `SaveDataToSlot` / `LoadDataFromSlot` / `SaveGameToSlot` | `.h:1143` / `.h:1191` (`.cpp:2390-2401`, `2491-2505`) | 저장 층은 쓰지 않는다. 테스트가 '옛 꼴' 슬롯을 `SaveGameToSlot`으로 만든다 |
| `LoadGameFromMemory(const TArray<uint8>&)` | `.h:1182` / `.cpp:2457-2489` | 길이·체크섬 검사가 없다. 그래서 우리 머리가 먼저다 |
| `FGenericSaveGameSystem` | `E/Runtime/Engine/Public/SaveGameSystem.h:110` (목록 137-149, 쓰기 151-154, 읽기 156-159, 지움 163 — `IFileManager::Delete(경로, true, false, !bAttemptToUseUI)`, `DeleteGameInSlot`은 `bAttemptToUseUI` 거짓이라(`GameplayStatics.cpp:2452`) `Quiet` 참, 경로 169-172) | Win64가 쓰는 구현(`PlatformFeatures.cpp:13-17`). 경로 = `ProjectSavedDir()/SaveGames/<슬롯>.sav`. 목록은 그 폴더의 `*.sav`만(하위 폴더 아님). 읽기 전용 파일은 지우지 않는다 |
| `FFileHelper::LoadFileToArray` / `SaveArrayToFile` | `E/Runtime/Core/Private/Misc/FileHelper.cpp:42-72` / `738-751` | 읽기는 `CreateFileReader`가 실패하면 `FILEREAD_Silent`가 없을 때 `LogStreaming` Warning(47-53), 파일을 통째로 읽는다. 쓰기는 `Close` 뒤 오류를 알린다 |
| `FFileManagerGeneric::CreateFileWriterInternal` | `E/Runtime/Core/Private/HAL/FileManagerGeneric.cpp:95-128` | 열기에 실패하면 폴더를 만들고 다시 연다(`IPlatformFile`로 바로 쓸 때는 `CreateDirectoryTree`를 먼저 부른다) |
| `FWindowsPlatformFile::OpenWrite` / `Flush` / `MoveFile` | `E/Runtime/Core/Private/Windows/WindowsPlatformFile.cpp:1638-1652` / `933-936` / `1216-1219` | `bAppend` 거짓이면 `CREATE_ALWAYS`로 최종 경로를 바로 자른다(임시 파일 없음). `bAllowRead` 거짓이면 공유 깃발이 없다(테스트가 잠금을 흉내 낸다 — `bAppend` 참으로 열어 자르지 않는다). 손잡이의 `Flush`는 `FlushFileBuffers`. `MoveFileW`에 바꿔 넣기 깃발이 없다 |
| `FFileManagerGeneric::Move` | `FileManagerGeneric.cpp:299-323` | 대상을 **먼저 지우고** 옮긴다(0.5초 재시도 포함). 원자적이지 않다 |
| `FArchiveFileWriterGeneric::Close` / `Flush` | `FileManagerGeneric.cpp:958-967` / `1000-1006` | `Close`는 `FlushBuffer`뿐. 아카이브의 `Flush`는 버퍼에 남은 바이트가 있을 때만 손잡이를 내린다(1 MB 이상 한 번에 쓰면 남지 않는다 — 971-976) |
| `IFileManager::CreateFileReader` / `Delete` / `FindFiles` / `GetTimeStamp` / `FileSize` | `E/Runtime/Core/Public/HAL/FileManager.h:85, 107, 143, 218, 271` | 읽기(`FILEREAD_Silent`), 지움(`Delete(Filename, RequireExists = 0, EvenReadOnly = 0, Quiet = 0)` — 본문 `FileManagerGeneric.cpp:255-291`: 기본 인자는 없는 파일에도 참을 내고, 읽기 전용 파일을 지우지 못하면 `Quiet`가 아닐 때 `LogFileManager` Warning), 목록, UTC 시각, 크기(없으면 −1) |
| 동적 멀티캐스트의 부름 차례 | `E/Runtime/Core/Public/UObject/ScriptDelegates.h:1438-1447`(`ProcessDelegate`), `:1563-1567`(`AddUniqueInternal`), `:1600-1605`(`CompactInvocationList`) | 목록 사본을 앞에서부터 부른다 — 묶은 차례다. 정리는 `RemoveAllSwap`이라 풀린 항목이 있으면 차례가 바뀔 수 있다. 저장 Subsystem(`Initialize`)이 보통 컨트롤러(`BeginPlay`)보다 먼저 불린다 — 기대지 않는다(헤더 §4.5) |
| `IPlatformFile::OpenWrite` / `CreateDirectoryTree` / `SetReadOnly`, `IFileHandle::Write` / `Flush` | `E/Runtime/Core/Public/GenericPlatform/GenericPlatformFile.h:570, 927, 457` / `:158, 166` | `OpenWrite(Filename, bAppend = false, bAllowRead = false)`, `Flush(bFullFlush = false)` |
| `AutomationCommon::GetAnyGameWorld` | `E/Runtime/Engine/Public/Tests/AutomationCommon.h:190`(`ENGINE_API`) | 새 지연 명령 `FWaitForNewGameWorld`가 읽는다 |
| `FOodleDataCompression::Compress` / `Decompress` / `CompressedBufferSizeNeeded` | `E/Runtime/Core/Public/Compression/OodleDataCompression.h:180` / `264` / `152` (모두 `CORE_API`) | 대표 인코드 속도표 `:78-88`(Kraken2 = VeryFast 2.32:1·157.4 MB/s, Kraken3 = Fast 2.39:1·34.8 MB/s, Kraken4 = Normal 2.44:1·22.4 MB/s), 수준 값 `:93-103`, 압축기는 스트림에 기록되고 수준은 아니다(`:175`). 풀기는 `OodleLZ_FuzzSafe_Yes`(`Private/Compression/OodleDataCompression.cpp:378, 419`), 끝의 `check`(564). 실패는 0/false |
| `FCompression::CompressMemory(NAME_Oodle, …)`의 게임 안 길 | `E/Runtime/Core/Private/Compression/OodleDataCompressionFormat.cpp:274-290` → `OodleDataCompression.cpp:179-204` | 깃발 0 = `Default` = Kraken·Fast. 명령줄 설정은 `COMPRESS_ForPackaging` 길에만 든다(`OodleDataCompressionFormat.cpp:218`) |
| `FCrc::MemCrc32(const void*, int32, uint32 = 0)` | `E/Runtime/Core/Public/Misc/Crc.h:29-32` | `[[nodiscard]]` |
| `FGuid`의 `operator<<` / `FGuid::ParseExact` | `E/Runtime/Core/Private/Misc/Guid.cpp:637-640` / `:621`(`FChar::IsHexDigit` — 소문자도 받는다), `Public/Misc/Guid.h:420-422` | A·B·C·D 16바이트 |
| `FDateTime::UtcNow` / `GetTicks` / `FPlatformTime::Seconds` | `E/Runtime/Core/Public/Misc/DateTime.h:756` / `:398` / `Public/Windows/WindowsPlatformTime.h:23` | — |
| `FSubsystemCollectionBase::InitializeDependency<T>()` | `E/Runtime/Engine/Public/Subsystems/SubsystemCollection.h:34-43` | 저장 Subsystem이 시뮬보다 늦게 선다 |
| 종료 차례 | `E/Runtime/Engine/Private/GameEngine.cpp:1373` → `:1377` | `World->EndPlay(Quit)`(액터마다)가 `GameInstance->Shutdown()`보다 먼저다. Subsystem `Deinitialize` 차례는 TMap 순이라 보장이 없다(`Subsystems/SubsystemCollection.cpp:349-357`) |
| 맵 다시 열기 | `E/Runtime/Engine/Private/UnrealEngine.cpp:16238-16249` → `Actor.cpp:3311-3313` → `UnrealEngine.cpp:16259`; `:16640-16661`; `GameModeBase.cpp:973-978`; `PlayerController.cpp:1373-1388`; `WorldSettings.cpp:362-378` | 컨트롤러를 `DestroyActor` → `AActor::Destroyed` → `RouteEndPlay(Destroyed)`, 그 뒤 월드 `EndPlay(LevelTransition)`. 새 맵: `SpawnPlayActor`(컨트롤러, PostLogin → `InitializeHUDForPlayer` → `ClientSetHUD`가 HUD를 낳는다) → `World->BeginPlay` → `NotifyBeginPlay`가 `FActorIterator` 차례로 `DispatchBeginPlay` → 컨트롤러가 HUD보다 먼저 |
| 지연 명령 | `E/Runtime/Engine/Public/Tests/AutomationCommon.h:213, 239, 254`, `Private/Tests/AutomationCommon.cpp:795-803, 825-833, 863-877`; `UnrealEngine.cpp:15378-15410`; `GameEngine.cpp:1892-1893`; `E/Runtime/Core/Private/Misc/AutomationTest.cpp:684-707` | `FWaitLatentCommand`(시간), `FLoadGameMapCommand`(월드 문맥이 하나이고 `Game`이어야 한다 — `check`, `Open <맵>`), `FWaitForMapToLoadCommand`(`GetAnyGameWorld()`의 `AreActorsInitialized` ∧ `HasMatchStarted`만). `Open`은 `SetClientTravel`로 미루고 이동은 다음 엔진 틱의 `TickWorldTravel`. 지연 고리는 끝난 명령 뒤의 명령을 같은 프레임에 돌린다 → 기다림이 옛 월드로 끝날 수 있다 |
| 문의 재료 | `E/Runtime/Core/Public/CoreGlobals.h:200, 214, 220, 607`; `E/Runtime/Launch/Private/LaunchEngineLoop.cpp:2269-2273, 2296-2300`; `AutomationTest.cpp:651, 673, 1218, 1226`; `E/Runtime/Core/Private/Misc/App.cpp:274-280`; `E/Editor/UnrealEd/Private/PlayLevelNewProcess.cpp:399` | `GIsEditor`(에디터가 아닌 빌드에서는 `false` 매크로) — `-game`이면 거짓, 에디터면 참. `GIsAutomationTesting`은 `StartTest` → `PrepForAutomationTests`부터 `StopTest` → `ConcludeAutomationTests`까지. `FApp::IsUnattended` = `-unattended` ∨ `GIsAutomationTesting`(앞의 것은 static으로 한 번 읽는다). Standalone Game의 인자는 `-game -PIEVIACONSOLE …`(`-unattended` 없음) |
| `bMatchStarted` | `GameModeBase.cpp:204-207`, `GameStateBase.cpp:205-211` | `StartPlay` 안에서 선다 |

## 1. 슬롯 바이트 — `Save/CosmicSaveSlot.cpp` 본문 스케치

```cpp
namespace CosmicSaveSlotImpl
{
	// The 60 bytes before HeaderCrc, field by field (FArchive does not swap on Win64: little-endian).
	void WriteHead(TArray<uint8>& Out, const CosmicSaveSlot::FHeader& H)
	{
		TArray<uint8> Head;
		FMemoryWriter Writer(Head);
		uint32 Magic = CosmicSaveSlot::Magic;
		uint16 Form = H.Form;
		uint8 Codec = static_cast<uint8>(H.Codec);
		uint8 Reserved = 0;   // byte 7: written 0, never read
		FGuid ID = H.GalaxyID;
		int64 SavedAt = H.SavedAtUtcTicks, Tick = H.Tick, Raw = H.RawBytes, Packed = H.PackedBytes;
		uint32 PayloadCrc = H.PayloadCrc;
		Writer << Magic << Form << Codec << Reserved << ID << SavedAt << Tick << Raw << Packed << PayloadCrc;
		check(Head.Num() == CosmicSaveSlot::HeaderCrcBytes);
		uint32 HeaderCrc = FCrc::MemCrc32(Head.GetData(), Head.Num());
		Writer << HeaderCrc;
		FMemory::Memcpy(Out.GetData(), Head.GetData(), CosmicSaveSlot::HeaderBytes);
	}
}

bool CosmicSaveSlot::Encode(UCosmicSaveGame& Save, int64 SavedAtUtcTicks, TArray<uint8>& OutSlot, int64* OutRawBytes, ECodec Want)
{
	OutSlot.Reset();
	TArray<uint8> Raw;
	if (!UGameplayStatics::SaveGameToMemory(&Save, Raw) || Raw.IsEmpty() || Raw.Num() > MaxRawBytes)
	{
		return false;
	}
	if (OutRawBytes) { *OutRawBytes = Raw.Num(); }
	FHeader H;
	H.Form = FormVersion; H.GalaxyID = Save.State.GalaxyID; H.SavedAtUtcTicks = SavedAtUtcTicks;
	H.Tick = Save.State.Tick; H.RawBytes = Raw.Num();
	int64 Packed = 0;
	if (Want == ECodec::Oodle)
	{
		const int64 Bound = FOodleDataCompression::CompressedBufferSizeNeeded(Raw.Num());
		if (Bound > 0 && HeaderBytes + Bound < MAX_int32)
		{
			OutSlot.SetNumUninitialized(HeaderBytes + static_cast<int32>(Bound));
			Packed = FOodleDataCompression::Compress(OutSlot.GetData() + HeaderBytes, Bound, Raw.GetData(), Raw.Num(),
				FOodleDataCompression::ECompressor::Kraken, FOodleDataCompression::ECompressionLevel::VeryFast);
		}
	}
	if (Packed > 0)
	{
		H.Codec = ECodec::Oodle;
		OutSlot.SetNum(HeaderBytes + static_cast<int32>(Packed), EAllowShrinking::No);
	}
	else
	{
		H.Codec = ECodec::Stored;   // the codec gave nothing (or a test asked): the engine bytes as they are
		Packed = Raw.Num();
		OutSlot.SetNumUninitialized(HeaderBytes + Raw.Num());
		FMemory::Memcpy(OutSlot.GetData() + HeaderBytes, Raw.GetData(), Raw.Num());
	}
	H.PackedBytes = Packed;
	H.PayloadCrc = FCrc::MemCrc32(OutSlot.GetData() + HeaderBytes, static_cast<int32>(Packed));
	CosmicSaveSlotImpl::WriteHead(OutSlot, H);
	return true;
}
```

- **`ReadHeader(Head, FileBytes, Expected, Out)`**: `FileBytes < HeaderBytes` 또는 `Head.Num() < HeaderBytes` → `Short`. `FMemoryReaderView`로 앞 64바이트를 쓴 차례대로 읽는다(7번 바이트는 읽고 버린다). 첫 낱말 `EngineMagic` → `OldForm`, `≠ Magic` → `NotOurs`. `FCrc::MemCrc32(Head.GetData(), HeaderCrcBytes) != HeaderCrc` → `HeaderDamaged`(**꼴보다 먼저**). `Form > FormVersion` → `NewerForm`(이때 `Out`에는 봉투의 `GalaxyID`·`SavedAtUtcTicks`만 믿을 수 있다 — 이어하기의 차례가 그것을 쓴다). `Form == 0`, 코덱이 0·1이 아님, `RawBytes ∉ [1, MaxRawBytes]`, `PackedBytes <= 0`, Stored ∧ `RawBytes != PackedBytes` → `BadSizes`. `FileBytes != HeaderBytes + PackedBytes` → `Truncated`. `Expected && *Expected != GalaxyID` → `OtherGalaxy`.
- **`Check(Slot, …)`** = `ReadHeader(Slot, Slot.Num(), …)` → `FCrc::MemCrc32(Slot.GetData() + HeaderBytes, PackedBytes) != PayloadCrc` → `PayloadDamaged`. 길이 검사가 앞에 서므로 CRC는 버퍼 안만 읽는다.
- **`Unpack`** = `Check` → `OutRaw.SetNumUninitialized(RawBytes)` → Oodle면 `FOodleDataCompression::Decompress(OutRaw.GetData(), RawBytes, Slot.GetData() + HeaderBytes, PackedBytes)`가 false면 `CodecRefused`(`OutRaw` 비움), Stored면 복사.
- **`ParseSlotName`**: 길이 39 또는 43, 앞 7자 `Galaxy_`(대소문자 그대로), 43자면 끝 4자 `_bak`, 가운데 32자를 `FGuid::ParseExact(…, EGuidFormats::Digits, Out)`가 받는다(16진 검사는 그 안에 있다 — `Guid.cpp:621`).
- **`MaxSlotBytes`** = `HeaderBytes + FOodleDataCompression::CompressedBufferSizeNeeded(MaxRawBytes)`.
- **`NewestRun`**: 판마다 가장 늦은 `UtcTicks`를 모으고(주·백업, 머리 시각이든 파일 시각이든), 가장 늦은 판(같으면 `ToString(Digits)`가 큰 쪽)을 낸다. 비어 있으면 무효.
- **`NameOf`**: 열거자 이름 그대로(`TEXT("PayloadDamaged")` 등). **`IsDamage`**: 열거자를 이름으로 드는 `switch` — `Missing`·`Short`·`OldForm`·`NotOurs`·`HeaderDamaged`·`BadSizes`·`Truncated`·`OtherGalaxy`·`PayloadDamaged`·`CodecRefused`·`Unreadable`(열하나)이면 참, `Ok`·`ReadFailed`·`NewerForm`·`NewerSave`·`Sanctuary`·`Refused`는 거짓. 범위 비교(`Missing <= Read <= Unreadable`)는 쓰지 않는다 — 열거는 저장되지 않아 차례를 바꿔도 되고(헤더 §3.8의 주석), 그러려면 이 함수가 차례에 기대지 않아야 한다. 열거자를 더하면 이 `switch`에 넣을지 함께 정한다.

## 2. 저장 Subsystem — `Save/CosmicSaveSubsystem.cpp`

### 2.1 private 멤버
```cpp
private:
	UFUNCTION()
	void HandleSimTick(int64 Tick);

	// The one write (section 3.4): refusals, backup, slot, disk copy. bGamePath: the gate's paths - a failure there counts
	// for TakeFirstFailure, the retry wait and the kept cause; an explicit SaveCurrentGalaxy's does not.
	bool Write(ECosmicSaveCause Cause, bool bGamePath);
	// One slot, read and checked through the sanctuary; Out is the loaded object when Ok. The version and sanctuary lines are
	// logged for the slot only (bBackup false): a backup's reason is told by LoadRun's one line.
	ECosmicSlotRead TryRead(const FGuid& GalaxyID, bool bBackup, UCosmicSaveGame*& Out, CosmicSaveSlot::FHeader& OutHeader) const;
	// The file of a slot: Missing (no file), BadSizes (larger than a slot), ReadFailed (did not read twice), else Ok.
	ECosmicSlotRead ReadSlotFile(const FString& Path, TArray<uint8>& Out) const;
	// IPlatformFile::OpenWrite, Write, Flush(true) (FlushFileBuffers on Win64), close. False on any failure.
	bool WriteSlotFile(const FString& Path, TConstArrayView<uint8> Bytes) const;
	void SetMarkFromState(uint32 StateHash);
	UGalaxySimulationSubsystem* GetSim() const;

	FString Folder;                                       // "" (the game), "Automation/" (PrepareForTests) or a test's own
	FCosmicSavedMark Mark;                                // the disk copy
	TOptional<ECosmicSaveCause> Pending;                  // asked from a handler or a stage, or failed: the next due boundary writes it; any successful write clears it
	ECosmicContinue StartOutcome = ECosmicContinue::NotBegun;
	ECosmicContinue LastContinue = ECosmicContinue::NotBegun;
	FGuid HeldRunID;                                      // HoldStartRun: the new run of an Unreadable start (decision 6); invalid otherwise
	bool bCommandLineAllows = false;                      // CommandLineAllows(FCommandLine::Get()) in Initialize
	bool bBlockedForTests = false;
	bool bTestOverride = false;
	bool bContinueAtNextStart = false;
	bool bFailedOnce = false;
	bool bFailureTaken = false;
	double RetryNotBefore = 0.0;                          // FPlatformTime::Seconds; boundary writes wait for it after a failure
	double IntervalSeconds = AutosaveRealSeconds;
	double RetrySeconds = RetryRealSeconds;
	int64 LatestSavedAt = 0;                              // every SavedAtUtcTicks this process wrote or loaded: the next is later
	int32 SavesWritten = 0;
	TOptional<ECosmicSaveCause> LastCause;
```
`Mark`·`Pending`·`StartOutcome`·`HeldRunID`는 디스크와 프로세스의 기록이다(헤더 §4.5). 플레이어의 사실을 여기서 읽는 곳은 저장 층 밖에 없다.

### 2.2 `Initialize`·문
```cpp
void UCosmicSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UGalaxySimulationSubsystem* Sim = Collection.InitializeDependency<UGalaxySimulationSubsystem>();
	if (Sim) { Sim->OnSimTick.AddUniqueDynamic(this, &UCosmicSaveSubsystem::HandleSimTick); }
	bCommandLineAllows = CommandLineAllows(FCommandLine::Get());
	UE_LOG(LogCosmic, Log, TEXT("CosmicSave: the game's saving is %s by the command line"), bCommandLineAllows ? TEXT("allowed") : TEXT("off"));
}

bool UCosmicSaveSubsystem::CommandLineAllows(const TCHAR* CommandLine)
{
	int32 GivenSeed = 0;
	return !FParse::Param(CommandLine, TEXT("NoSave")) && !FParse::Param(CommandLine, TEXT("unattended"))
		&& !FParse::Value(CommandLine, TEXT("Seed="), GivenSeed);
}

const TCHAR* UCosmicSaveSubsystem::GateClosedBy(bool bAllows, bool bBlocked, bool bAutomation, bool bEditor, bool bOverride)
{
	if (bOverride) { return nullptr; }
	if (!bAllows) { return TEXT("command line"); }
	if (bBlocked) { return TEXT("tests"); }
	if (bAutomation) { return TEXT("automation"); }
	if (bEditor) { return TEXT("editor"); }
	return nullptr;
}

bool UCosmicSaveSubsystem::IsGameSavingOn() const
{
	// GIsEditor (CoreGlobals.h): true in the editor process, so play in editor; false under -game and Standalone Game.
	return GateClosedBy(bCommandLineAllows, bBlockedForTests, GIsAutomationTesting, GIsEditor, bTestOverride) == nullptr;
}
```
`Deinitialize`는 `OnSimTick.RemoveDynamic`만 한다(시뮬이 먼저 내려갔을 수 있어 `GetSim()`이 null이면 건너뛴다). `UWorld::IsPlayInEditor`는 쓰지 않는다(`Engine/World.h`를 레이어 린트가 막는다).

### 2.3 `Write`
```cpp
bool UCosmicSaveSubsystem::Write(ECosmicSaveCause Cause, bool bGamePath)
{
	UGalaxySimulationSubsystem* Sim = GetSim();
	if (!Sim) { return false; }
	if (Sim->IsBroadcasting())
	{
		UE_LOG(LogCosmic, Error, TEXT("SaveCurrentGalaxy called from a handler of a Core Logic delegate: a save waits for the tick boundary (RequestSave)"));
		return false;
	}
	if (Sim->IsInTickStages())
	{
		UE_LOG(LogCosmic, Error, TEXT("SaveCurrentGalaxy called inside the tick stages: save from OnSimTick or from input"));
		return false;
	}
	const FGuid ID = Sim->GetState().GalaxyID;
	if (!ID.IsValid()) { UE_LOG(LogCosmic, Warning, TEXT("SaveCurrentGalaxy: no galaxy to save")); return false; }

	auto Fail = [&]()
	{
		if (bGamePath)
		{
			bFailedOnce = true;
			RetryNotBefore = FPlatformTime::Seconds() + RetrySeconds;
			if (!Pending) { Pending = Cause; }   // the first boundary after RetryNotBefore writes it again (decision 12)
		}
		return false;
	};
	const double Start = FPlatformTime::Seconds();
	Sim->SyncRngState();
	UCosmicSaveGame* Carrier = NewObject<UCosmicSaveGame>(GetTransientPackage());
	Carrier->State = Sim->GetState();
	const int64 SavedAt = FMath::Max(FDateTime::UtcNow().GetTicks(), LatestSavedAt + 1);
	TArray<uint8> Slot;
	int64 RawBytes = 0;
	if (!CosmicSaveSlot::Encode(*Carrier, SavedAt, Slot, &RawBytes))
	{
		UE_LOG(LogCosmic, Warning, TEXT("CosmicSave: %s could not be packed"), *SlotNameOf(ID));
		return Fail();
	}
	const FString Main = SlotNameOf(ID), MainPath = SlotPathOf(ID), BackupPath = SlotPathOf(ID, true);
	if (IFileManager::Get().FileSize(*MainPath) >= 0)
	{
		TArray<uint8> Old;
		CosmicSaveSlot::FHeader OldHeader;
		ECosmicSlotRead OldRead = ReadSlotFile(MainPath, Old);
		if (OldRead == ECosmicSlotRead::Ok)
		{
			OldRead = CosmicSaveSlot::Check(Old, &ID, OldHeader);
		}
		if (OldRead == ECosmicSlotRead::Ok)
		{
			if (!WriteSlotFile(BackupPath, Old))
			{
				UE_LOG(LogCosmic, Warning, TEXT("CosmicSave: the backup of %s was not written: %s is kept as it was"), *Main, *Main);
				return Fail();
			}
		}
		else if (OldRead == ECosmicSlotRead::ReadFailed || OldRead == ECosmicSlotRead::NewerForm)
		{
			UE_LOG(LogCosmic, Warning, TEXT("CosmicSave: %s is %s: it is not replaced"), *Main, CosmicSaveSlot::NameOf(OldRead));
			return Fail();
		}
		else
		{
			UE_LOG(LogCosmic, Warning, TEXT("CosmicSave: %s is damaged (%s): not copied, the backup stays"), *Main, CosmicSaveSlot::NameOf(OldRead));
		}
	}
	if (!WriteSlotFile(MainPath, Slot))
	{
		UE_LOG(LogCosmic, Warning, TEXT("CosmicSave: %s was not written"), *Main);
		return Fail();
	}
	LatestSavedAt = SavedAt;
	++SavesWritten;
	LastCause = Cause;
	Pending.Reset();        // on any path: this write put the state on the disk, so nothing kept is owed
	RetryNotBefore = 0.0;   // and the disk took a write: the boundary does not wait
	const uint32 Hash = Sim->ComputeStateHash();
	SetMarkFromState(Hash);
	UE_LOG(LogCosmic, Display, TEXT("CosmicSave: wrote %s (%s) tick %lld, hash %u, %d of %lld bytes in %.1f ms"), *Main, CosmicSave::NameOf(Cause),
		Carrier->State.Tick, Hash, Slot.Num(), RawBytes, (FPlatformTime::Seconds() - Start) * 1000.0);
	return true;
}

bool UCosmicSaveSubsystem::WriteSlotFile(const FString& Path, TConstArrayView<uint8> Bytes) const
{
	IPlatformFile& File = FPlatformFileManager::Get().GetPlatformFile();
	File.CreateDirectoryTree(*FPaths::GetPath(Path));
	TUniquePtr<IFileHandle> Handle(File.OpenWrite(*Path));   // fails on a read-only file
	if (!Handle) { return false; }
	const bool bOk = Handle->Write(Bytes.GetData(), Bytes.Num()) && Handle->Flush(true);   // FlushFileBuffers on Win64
	Handle.Reset();
	return bOk;
}

ECosmicSlotRead UCosmicSaveSubsystem::ReadSlotFile(const FString& Path, TArray<uint8>& Out) const
{
	const int64 Size = IFileManager::Get().FileSize(*Path);
	if (Size < 0) { return ECosmicSlotRead::Missing; }
	if (Size > CosmicSaveSlot::MaxSlotBytes()) { return ECosmicSlotRead::BadSizes; }
	for (int32 Attempt = 0; Attempt < 2; ++Attempt)
	{
		if (FFileHelper::LoadFileToArray(Out, *Path, FILEREAD_Silent)) { return ECosmicSlotRead::Ok; }
	}
	Out.Reset();
	return ECosmicSlotRead::ReadFailed;
}
```
- `SaveCurrentGalaxy()` = `Write(Asked, false)`.
- `SetMarkFromState(Hash)` = `Mark = MarkOf(State, Rules); Mark.StateHash = Hash; Mark.RealSeconds = FPlatformTime::Seconds();`.
- **결정 3 대안 ㉮(내리기 없음)를 고르면**: `WriteSlotFile`이 `UGameplayStatics::SaveDataToSlot(Bytes, SlotNameOf(…), 0)`이 된다.
- **결정 9 대안 ㉮(지우기)를 고르면**: `RequestSave(NewRun)`의 쓰기가 성공한 뒤에만, 그 직전 디스크 사본의 `GalaxyID`(다른 판)의 주와 백업을 지운다. 지우는 판은 디스크 사본이 가리키는 판 하나뿐이다(찾기로 고르지 않는다).

### 2.4 경계 핸들러·부탁
```cpp
void UCosmicSaveSubsystem::HandleSimTick(int64 Tick)
{
	if (!IsGameSavingOn()) { Pending.Reset(); return; }
	const UGalaxySimulationSubsystem* Sim = GetSim();
	if (!Sim || !Sim->GetState().GalaxyID.IsValid()) { return; }
	const double Now = FPlatformTime::Seconds();
	if (Now < RetryNotBefore) { return; }        // after a failure; Pending and the due trigger wait
	TOptional<ECosmicSaveCause> Cause = Pending;
	if (!Cause) { Cause = DueAt(Mark, MarkOf(Sim->GetState(), Sim->GetRules()), Now, IntervalSeconds); }
	if (!Cause) { return; }
	Pending.Reset();
	Write(*Cause, true);                          // a failure keeps the cause in Pending again (Fail)
}

void UCosmicSaveSubsystem::RequestSave(ECosmicSaveCause Cause)
{
	UGalaxySimulationSubsystem* Sim = GetSim();
	if (!IsGameSavingOn() || !Sim || !Sim->GetState().GalaxyID.IsValid()) { return; }
	if (Sim->IsBroadcasting() || Sim->IsInTickStages())
	{
		if (!Pending) { Pending = Cause; }
		return;
	}
	const FGalaxyState& State = Sim->GetState();
	if (Cause == ECosmicSaveCause::Exit)
	{
		if (Mark.GalaxyID == State.GalaxyID && Mark.StateHash == Sim->ComputeStateHash())
		{
			UE_LOG(LogCosmic, Verbose, TEXT("CosmicSave: exit - the slot already holds this state"));
			return;
		}
		// Decision 6: the new run of an Unreadable start (HoldStartRun) is first written at a tick boundary, so a quit before
		// that leaves the run that did not read as the newest - the next start reads it again. That run only: a later new
		// run of the session is written.
		if (HeldRunID.IsValid() && State.GalaxyID == HeldRunID && Mark.GalaxyID != HeldRunID && State.Tick == 0)
		{
			UE_LOG(LogCosmic, Log, TEXT("CosmicSave: exit - the new run after an unreadable save is not written yet"));
			return;
		}
	}
	Write(Cause, true);
}
```
- `DueAt`: `Now.GalaxyID != Saved.GalaxyID` → `NewRun`; `Now.bEnded && !Saved.bEnded` → `Ended`; `Now.Feats != Saved.Feats` → `Ascended`; `Role`·`RuledCivID` 가운데 하나라도 다름 → `Role`; `Now.Tick != Saved.Tick && NowRealSeconds - Saved.RealSeconds >= IntervalSeconds` → `Interval`; 그 밖은 unset.
- `MarkOf`: `GalaxyID`, `Tick`, `Calling.CompletedFeats.Num()`, `Player.Role`, `Player.RuledCivID`, `CosmicEndings::ProvisionalEndOf(State, Rules) != None`. `StateHash`·`RealSeconds`는 0으로 둔다(쓰기가 채운다). 해시를 틱마다 셈하지 않는다(약 4 ms × 틱).
- `TakeFirstFailure`: `bFailedOnce && !bFailureTaken`이면 `bFailureTaken = true`로 두고 참.
- `HoldStartRun`: `HeldRunID = Sim ? Sim->GetState().GalaxyID : FGuid()`. 부르는 곳은 `StartPlay`의 `Unreadable` 가지 하나(§3.1)와 `Sim.SaveTriggers`다.
- `PrepareForTests`: `Folder = TEXT("Automation/")`, `bBlockedForTests = true`, `bTestOverride = false`, `bContinueAtNextStart = false`, `Pending.Reset()`, `IntervalSeconds = AutosaveRealSeconds`, `RetrySeconds = RetryRealSeconds`, `RetryNotBefore = 0`, `bFailedOnce = bFailureTaken = false`, `Mark = {}`, `LastContinue = NotBegun`, `HeldRunID.Invalidate()`.
- `SetFolderForTest(F)`: `Folder = F`(다음 `PrepareForTests`가 되돌린다). `SetTimingForTest(I, R)`: `IntervalSeconds = I; RetrySeconds = R; RetryNotBefore = 0` — 기다림을 지우므로 기다림을 보는 경계 앞에서는 부르지 않는다(§6.2 `Sim.SaveTriggers`).
- `SlotNameOf(ID, bBackup)` = `Folder + MakeSlotName(ID) + (bBackup ? TEXT("_bak") : TEXT(""))`. `SlotPathOf(ID, bBackup)` = `FPaths::ProjectSavedDir() / TEXT("SaveGames") / (SlotNameOf(ID, bBackup) + TEXT(".sav"))` — `FGenericSaveGameSystem::GetSaveGamePath`(`SaveGameSystem.h:169-172`)와 같은 계산.
- `DoesSaveExist` = 주 ∨ 백업의 `FileSize ≥ 0`. `DeleteSave` = 주와 백업마다 `FileSize(경로) ≥ 0`일 때만 `IFileManager::Get().Delete(*경로, /*RequireExists*/ true, /*EvenReadOnly*/ false, /*Quiet*/ true)`(엔진의 `FGenericSaveGameSystem::DeleteGame`과 같은 인자 — `SaveGameSystem.h:163`), 그 참이 하나라도 있으면 참. 기본 인자의 `Delete`는 없는 파일에도 참을 내고 읽기 전용에서 실패하면 Warning을 남기므로(`FileManagerGeneric.cpp:255-291`) 쓰지 않는다 — 기대 경고 수를 세는 테스트가 그 줄에 흔들리지 않는다.

### 2.5 로드·찾기·시작
```cpp
ECosmicSlotRead UCosmicSaveSubsystem::LoadRun(const FGuid& GalaxyID, bool& bOutFromBackup)
{
	bOutFromBackup = false;
	UGalaxySimulationSubsystem* Sim = GetSim();
	if (!Sim) { return ECosmicSlotRead::Refused; }
	if (Sim->IsBroadcasting()) { UE_LOG(LogCosmic, Error, TEXT("LoadGalaxy called from a handler of a Core Logic delegate: a handler may only read the state")); return ECosmicSlotRead::Refused; }
	if (Sim->IsInTickStages()) { UE_LOG(LogCosmic, Error, TEXT("LoadGalaxy called inside the tick stages: load from OnSimTick or from input")); return ECosmicSlotRead::Refused; }
	UCosmicSaveGame* Loaded = nullptr;
	CosmicSaveSlot::FHeader Header;
	const FString Main = SlotNameOf(GalaxyID);
	ECosmicSlotRead Read = TryRead(GalaxyID, false, Loaded, Header);
	if (CosmicSaveSlot::IsDamage(Read))
	{
		const ECosmicSlotRead FromBackup = TryRead(GalaxyID, true, Loaded, Header);
		if (FromBackup == ECosmicSlotRead::Ok)
		{
			UE_LOG(LogCosmic, Warning, TEXT("LoadGalaxy: slot %s: %s; read from its backup (tick %lld)"), *Main, CosmicSaveSlot::NameOf(Read), Header.Tick);
			bOutFromBackup = true;
			Read = ECosmicSlotRead::Ok;
		}
		else if (Read == ECosmicSlotRead::Missing && FromBackup == ECosmicSlotRead::Missing)
		{
			UE_LOG(LogCosmic, Warning, TEXT("LoadGalaxy: slot %s is missing"), *Main);
		}
		else
		{
			UE_LOG(LogCosmic, Warning, TEXT("LoadGalaxy: slot %s: %s; its backup: %s"), *Main, CosmicSaveSlot::NameOf(Read), CosmicSaveSlot::NameOf(FromBackup));
		}
	}
	else if (Read == ECosmicSlotRead::ReadFailed)
	{
		UE_LOG(LogCosmic, Warning, TEXT("LoadGalaxy: slot %s could not be read: the backup is not tried"), *Main);
	}
	else if (Read == ECosmicSlotRead::NewerForm)
	{
		UE_LOG(LogCosmic, Warning, TEXT("LoadGalaxy: slot %s has slot form %d, this build reads up to %d"), *Main, static_cast<int32>(Header.Form), static_cast<int32>(CosmicSaveSlot::FormVersion));
	}
	if (Read != ECosmicSlotRead::Ok) { return Read; }
	Sim->LoadState(Loaded->State);
	LatestSavedAt = FMath::Max(LatestSavedAt, Header.SavedAtUtcTicks);
	SetMarkFromState(Sim->ComputeStateHash());
	UE_LOG(LogCosmic, Display, TEXT("CosmicSave: loaded %s%s at tick %lld, hash %u"), *Main, bOutFromBackup ? TEXT(" from its backup") : TEXT(""), Loaded->State.Tick, Mark.StateHash);
	return ECosmicSlotRead::Ok;
}
```
- **`TryRead(ID, bBackup, …)`**: `ReadSlotFile(SlotPathOf(ID, bBackup))`가 `Missing`·`BadSizes`·`ReadFailed`면 그것 → `Unpack(…, &ID, …)` → `Cast<UCosmicSaveGame>(UGameplayStatics::LoadGameFromMemory(Raw))`이 null이면 `Unreadable` → `State.GalaxyID != ID`면 `OtherGalaxy` → 버전(지금 88행의 Warning 문구 그대로)이면 `NewerSave` → 성역(지금 97-104행 그대로, Error 문구 그대로)이면 `Sanctuary`. 이 두 줄은 주 슬롯(`bBackup` 거짓)을 읽을 때만 낸다. 백업에서는 줄 없이 까닭만 돌려준다.
- **주 슬롯의 `NewerSave`·`Sanctuary` 줄**은 `TryRead`가 내므로 `LoadRun`은 줄을 더하지 않는다(`Sim.SaveSanctuary`의 Error 5 그대로 — 다섯 모두 주 슬롯의 읽기다). 백업을 본 뒤에는 `its backup:` 줄 하나뿐이다 — 백업이 `NewerSave`·`Sanctuary`여도 `TryRead`가 둘째 줄을 내지 않는다(헤더 §3.5 '로드 하나에 Warning 한 줄').
- **`LoadGalaxy(ID)`** = `LoadRun(ID, b) == Ok`.
- **`ContinueNewest`**: 방송 중이면 Error 한 줄 + `NoSlot`. `IFileManager::Get().FindFiles(Files, *(FPaths::ProjectSavedDir() / TEXT("SaveGames/") + Folder), TEXT("*.sav"))` → 파일마다 `FPaths::GetBaseFilename` → `ParseSlotName` → 경로 = `SlotPathOf(ID, bBak)` → **머리만**: `Size = FileSize(경로)`, `TUniquePtr<FArchive> Reader(IFileManager::Get().CreateFileReader(*경로, FILEREAD_Silent))`가 있고 `Size >= 64`면 64바이트를 `Serialize` → `ReadHeader(Head, Size, &ID, H)`.
  - `Ok`·`Truncated`·`NewerForm` → `FSeen{ID, bBak, H.SavedAtUtcTicks, true}`.
  - `OldForm` → Log `CosmicContinue: <name> is in the slot form before item 5 - not a run to continue`, 후보에서 뺀다.
  - 그 밖(읽기 손잡이 없음·짧음·머리 상함 등) → `FSeen{ID, bBak, IFileManager::Get().GetTimeStamp(*경로).GetTicks(), false}`.
  - `Seen`이 비면 `NoSlot`. 아니면 `NewestRun` → `LoadRun` → `Ok`면 `Newest`/`Backup`, 아니면 `Unreadable`. `LatestSavedAt`을 본 머리의 가장 늦은 시각으로 올린다. 마지막에 `LastContinue = Outcome`과 Display `CosmicContinue: %s %s at tick %lld, hash %u (%d runs seen)`(열리지 않았으면 틱·해시 대신 `-`, 판의 수 = `Seen`의 서로 다른 GUID 수).
- **`BeginSession`**: 먼저 `HeldRunID.Invalidate()`. 문이 닫혀 있으면 `StartOutcome = LastContinue = Off`와 Log `CosmicContinue: Off (%s)`(`GateClosedBy`의 까닭), 열려 있으면 `StartOutcome = LastContinue = ContinueNewest()`. `TakeStartOutcome`은 돌려주고 `NotBegun`으로 둔다.

## 3. 게임 모드·컨트롤러·HUD·상단 바

### 3.1 `ACosmicGameMode::StartPlay`(96~117행)
```cpp
	UGalaxySimulationSubsystem* Sim = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGalaxySimulationSubsystem>() : nullptr;
	UCosmicSaveSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>() : nullptr;
	// Before Super::StartPlay: no handler is bound yet, so a continued run that ended stands ended through the two BeginPlay
	// calls (FollowEnd(false) in each - headers/A4 4.4 (5)); the load's OnGalaxyGenerated reaches nobody.
	if (Sim && (!Sim->GetState().GalaxyID.IsValid() || (Save && Save->ConsumeContinueAtStart())))
	{
		const ECosmicContinue Start = Save ? Save->BeginSession() : ECosmicContinue::Off;
		if (Start != ECosmicContinue::Newest && Start != ECosmicContinue::Backup)
		{
			int32 Seed = 0;
			if (!FParse::Value(FCommandLine::Get(), TEXT("Seed="), Seed))
			{
				Seed = CosmicControlImpl::ClockSeed();
			}
			Sim->NewGalaxy(Seed, 0, 0);
			if (Save && Start == ECosmicContinue::NoSlot)
			{
				// On disk from tick 0. While the gate is closed (Off) nothing is written.
				Save->RequestSave(ECosmicSaveCause::NewRun);
			}
			else if (Save && Start == ECosmicContinue::Unreadable)
			{
				// Decision 6: this run is first written at a tick boundary; a quit before that skips its exit save.
				Save->HoldStartRun();
			}
		}
		// -Speed= now applies to a continued run too (FollowStart then leaves it running).
		float Speed = 1.f;
		FParse::Value(FCommandLine::Get(), TEXT("Speed="), Speed);
		Sim->SetTimeScale(Speed);
	}
	Super::StartPlay();
```
include: `Save/CosmicSaveSubsystem.h`(World → Save, 규칙 1). 108-110행의 `-Speed=` 주석은 '새 은하와 이어 연 판의 시작 배속(이어 연 정지를 건너뛴다)'으로 고친다.

### 3.2 컨트롤러
- **`BeginPlay`**(527~554): `RunBeganRealSeconds` 옆에 `RunBeganTick = Sim->GetState().Tick`. `FollowEnd(false)`(552) 바로 뒤에 `FollowStart()`.
- **`FollowStart`**:
```cpp
void ACosmicPlayerController::FollowStart()
{
	UCosmicSaveSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCosmicSaveSubsystem>() : nullptr;
	const ECosmicContinue Outcome = Save ? Save->TakeStartOutcome() : ECosmicContinue::NotBegun;
	float GivenSpeed = 0.f;
	if (IsRunEnded() || FParse::Value(FCommandLine::Get(), TEXT("Speed="), GivenSpeed))
	{
		return;   // an ended run keeps its own line (Chronicle.Why); a development speed runs
	}
	switch (Outcome)
	{
	case ECosmicContinue::Newest:     ApplySpeed(0.f, ECosmicAutoPause::Continued); break;
	case ECosmicContinue::Backup:     ApplySpeed(0.f, ECosmicAutoPause::ContinuedFromBackup); break;
	case ECosmicContinue::Unreadable: ApplySpeed(0.f, ECosmicAutoPause::SaveUnreadable); break;
	default: break;
	}
}
```
- **`HandleSimTick`**: 끝났으면 돌아오기 전과 뒤 모두 끝에 `FollowSaveFailure()`(private): `if (!IsRunEnded() && Save && Save->TakeFirstFailure()) ApplySpeed(0.f, ECosmicAutoPause::SaveFailed);`. 끝난 판에서는 `TakeFirstFailure`를 부르지 않는다(다음 판의 알림으로 남는다). 저장 Subsystem의 핸들러와의 차례에 기대지 않으므로(보통은 저장 Subsystem이 먼저 묶여 먼저 불린다 — §0.1의 부름 차례), 같은 경계의 실패는 그 경계나 다음 경계에서 알려진다. 그래서 이 핸들러는 테스트 프로세스에서도 경계의 실패 깃발을 먼저 가져갈 수 있다 — `Sim.SaveTriggers`는 깃발을 경계 없는 입력으로 단언한다(§6.2).
- **⑤가 처음 걷는 길**: 이어 연 끝난 판에서 컨트롤러 `BeginPlay`의 `FollowEnd(false)`가 래치를 세우면, 그리고 이어 연 흐르던 판에서 `FollowStart`가 멈추면, `ApplySpeed` → `RefreshPanel` → `ShowRightSide`가 **`BeginPlay` 전의 HUD**(액터는 있고 위젯은 없다 — `PlayerController.cpp:1387`의 `ClientSetHUD`가 낳았다) 위에서 돈다. 지금은 `TopBar`·`SystemPanel`·`GalaxyPanel`·`Card`·연대기 쪽이 모두 null 검사를 지나 안전하다(`World/CosmicGameMode.cpp:305-358`, `FollowEnd` 605-649). 이 길의 함수에 위젯이 있다고 가정하는 줄을 더하지 않는다. `UI.ContinueEnded`·`UI.ContinueRunning`이 이 길을 지난다.
- **`HandleGalaxyGenerated`**(587~603): `RunBeganRealSeconds` 옆에 `RunBeganTick = Sim->GetState().Tick`.
- **`EndPlay`**(556~572): 맨 앞에 `if (EndPlayReason == EEndPlayReason::Quit) { if (UCosmicSaveSubsystem* Save = …) { Save->RequestSave(ECosmicSaveCause::Exit); } }`. 맵 다시 열기의 까닭은 `Destroyed`라 쓰지 않는다.
- **`AnswerPetition`**(1497~1520): `Refused` 검사 뒤, 패널 새로 그리기 앞에 `Save->RequestSave(ECosmicSaveCause::Answer); FollowSaveFailure();`.
- **`StartNewRun`**(735~745): `NewGalaxy` → `Save->RequestSave(ECosmicSaveCause::NewRun)` → `SetSpeed(1.f)` → `FollowSaveFailure()`. 선언 주석(`CosmicGameMode.h:261`)에 'then saves the new run (MVP item 5)'.
- **`CosmicEnd:` 줄**(628): 판이 이 세션에서 끝나면 `TEXT("CosmicEnd: %s at tick %lld; %.0f s since tick %lld, %.0f s of them stopped")`(인자에 `RunBeganTick`). 이미 끝난 판을 열어 래치가 서면(`!bArrived && (RunBeganTick < 0 || RunBeganTick == Tick)`) `TEXT("CosmicEnd: %s at tick %lld; opened ended (continued)")`. `UI.RunEnd`가 이 줄을 보지 않음을 구현 때 확인한다(보면 그 패턴을 고친다).
- **private**: `int64 RunBeganTick = -1;`, `void FollowSaveFailure();`.

### 3.3 자동 정지 열거와 키
`World/CosmicAutoPause.h`의 `RunEnd` 뒤에 `Continued`·`ContinuedFromBackup`·`SaveUnreadable`·`SaveFailed`(헤더 §5.4의 주석). `WhyKeyOf`에 `Save.Why.Continued`·`Save.Why.Backup`·`Save.Why.Unreadable`·`Save.Why.Failed`.

### 3.4 결정 2 대안 ㉯를 고르면 — 상단 바의 '새 판'
- `UCosmicTopBarWidget`: `NewRunButton`(`UPROPERTY(Transient) TObjectPtr<UCosmicActionButton>`), `NewRunAction(TEXT("newrun"))`, `TOptional<float> NewRunConfirmLeft`, public `GetNewRunButton()`·`PressNewRun()`·`AdvanceNewRun(float DeltaSeconds)`(테스트).
- **확인 창은 연대기 쪽과 같은 꼴**이어야 한다: 상단 바는 지금 `DisableNativeTick`이고 `NativeTick`이 없으며(`UI/CosmicTopBarWidget.h:35`), `Refresh`는 틱·새 은하·누름에서만 불린다. 단추는 시간이 멈춘 동안에만 서므로 틱이 오지 않는다. 그래서 메타를 지우고 `NativeTick` → `AdvanceNewRun`이 남은 시간을 줄여 다 되면 라벨을 `Chronicle.NewRun`으로 되돌린다(`CosmicChronicleWidget.cpp:314-332`의 `Advance`와 같다). 그렇지 않으면 3초 뒤에도 라벨이 '한 번 더 누르면 새 판이 열린다'인데 다음 누름은 첫 누름이 된다.
- 짓기: `ReopenButton`(191-197) 바로 뒤에 같은 꼴로 `Setup(NewRunAction, 0.f, CosmicUI::Label(TEXT("Chronicle.NewRun")))`, 툴팁 `Chronicle.NewRunTip`, 접힘.
- `Refresh`의 보임: `Controller && !bEnded && !(Current > 0.f) && (Reason == Continued || Reason == ContinuedFromBackup)`. 숨을 때 `NewRunConfirmLeft.Reset()`, 라벨을 `Chronicle.NewRun`으로 되돌린다.
- 누름: 창이 없으면 `NewRunConfirmLeft = CosmicChronicleScreen::ConfirmSeconds`(3), 라벨 `Chronicle.NewRunConfirm`. 창 안의 둘째 누름이면 `Controller->StartNewRun()`.
- 테스트: `UI.ContinueRunning`에 둘째 맵 다시 열기와 '새 판' 두 번 → 새 GalaxyID의 주 슬롯 있음·옛 슬롯 그대로, '한 번 누름 → `AdvanceNewRun(ConfirmSeconds + 0.1)` → 라벨이 되돌아옴, 다음 한 번은 새 판을 열지 않음'. 변이: 보임 조건에서 배속 0 빼기, 되돌림 빼기.
- 헤더 §5.3·§5.4·§5.5(툴팁 자리)와 손 확인 4('상단 바 새 판')를 함께 되살린다. 1280×720 창에서 까닭 줄과 단추가 상단 바 안에 드는지 본다.

## 4. 연대기 — `Log/CosmicLogSubsystem.cpp`·`Log/CosmicChronicle.cpp`

```cpp
	// UCosmicLogSubsystem::Append, in place of 206-216.
	// Over the cap the oldest entries leave. What the chronicle can show - CosmicChronicle::Admits over the peoples of every
	// life (MVP item 5, 16 #40 (7)) - moves to the chronicle; the rest is dropped (section 8.4). Then the chronicle's own cap.
	if (State.Log.Num() > MaxLogEntries)
	{
		const TArray<FName> Peoples = CosmicChronicle::PeoplesOfEveryLife(State.Calling);
		bool bMoved = false;
		while (State.Log.Num() > MaxLogEntries)
		{
			if (CosmicChronicle::Admits(State.Log[0], Peoples))
			{
				State.Chronicle.Add(State.Log[0]);
				bMoved = true;
			}
			State.Log.RemoveAt(0, 1, EAllowShrinking::No);
		}
		if (bMoved)
		{
			TrimChronicle(State.Chronicle, State.ChronicleEviction, Sim->GetRules().ChronicleCap);
		}
	}

// private static of UCosmicLogSubsystem (not a UFUNCTION). One stable pass: the rest keeps its order.
void UCosmicLogSubsystem::TrimChronicle(TArray<FCosmicLogEntry>& Chronicle, FChronicleEviction& Eviction, int32 Cap)
{
	if (Cap <= 0 || Chronicle.Num() <= Cap)
	{
		return;
	}
	int32 Excess = Chronicle.Num() - Cap;
	int32 Write = 0;
	for (int32 Read = 0; Read < Chronicle.Num(); ++Read)
	{
		if (Excess > 0 && CosmicChronicle::MayLetGo(Chronicle[Read]))
		{
			++Eviction.Count;
			Eviction.ThroughTick = FMath::Max(Eviction.ThroughTick, Chronicle[Read].Tick);
			--Excess;
			continue;
		}
		if (Write != Read)
		{
			Chronicle[Write] = MoveTemp(Chronicle[Read]);
		}
		++Write;
	}
	Chronicle.SetNum(Write, EAllowShrinking::No);
}
```
- `Append`의 static `ClaimTag`·`RecordTag`(187-188)는 쓰이지 않게 되므로 지운다 → 레이어 린트의 태그 글자 41 → 39(`Tools/check_layers.py:24`의 `BASELINE`과 주석을 A-5b에서).
- `PeoplesOfEveryLife`(A-5b): `Calling.Lives`를 차례로 돌며 `PeoplesOf(Life)`를 `AddUnique`.
- `MayLetGo`(A-5a — 커맨드렛의 `kept for good`이 먼저 읽는다): `Entry.Severity == 1 && Entry.Category != CallingTag() && != ClaimTag() && != RecordTag()`(`CosmicChronicleImpl`의 셋).
- `BuildCapped`의 (3b)는 (3)의 `if`가 닫힌 뒤, (4)의 `Kept.Sort` 앞:
```cpp
		// (3b) The saved chronicle's cap let older lines go (MVP item 5): a chapter whose range reaches them says so too.
		if (State.ChronicleEviction.Count > 0 && Chapter.FromTick <= State.ChronicleEviction.ThroughTick)
		{
			Chapter.bMore = true;
		}
```
  첫 장의 `FromTick`은 `int64` 최솟값이라 밀어낸 것이 있으면 첫 장에는 늘 선다. 상한은 가장 오래된 S1부터 내보내므로 첫 장의 S1이 먼저 나간다. 첫 장의 S1이 연대기에 하나도 없었던 판에서만 첫 장이 사실보다 많이 말한다(헤더 §6.4 '덜 말하지는 않는다').
- 주석 고침: `CosmicChronicle.h:2-3`('Build reads; the log subsystem is the one writer of the move and the cap'), `:28`(`bMore` — 'the cap left lines out, or the saved chronicle's cap let older lines go'), `GalaxyState.h:764-769`.

## 5. 커맨드렛 — `Tests/CosmicBalanceCommandlet.cpp`

```cpp
	// The slot bytes of the state as the game writes them (CosmicSaveSlot::Encode: SaveGameToMemory, Oodle, the header), the
	// engine bytes before packing, and the time of the copy, the serialization, the packing and the CRCs. No file write.
	int32 SaveBytesOf(UCosmicSaveGame* Save, const FGalaxyState& State, int64& OutRawBytes, double& OutMilliseconds)
	{
		TArray<uint8> Slot;
		const double Start = FPlatformTime::Seconds();
		Save->State = State;
		CosmicSaveSlot::Encode(*Save, 0, Slot, &OutRawBytes);
		OutMilliseconds = (FPlatformTime::Seconds() - Start) * 1000.0;
		return Slot.Num();
	}
```
- 285-286행의 주석('The file write of SaveGameToSlot is not in it')을 위 주석으로 바꾼다.
- CSV 머리(448): A-5a `…,SaveBytes,LogLen,ChronicleLen,SaveRawBytes,Expansion`, A-5b `…,ChronicleLen,SaveRawBytes,ChronicleLetGo,Expansion`. `SaveRawBytes`는 `SaveBytes`가 찍히는 표본에만(빈 칸 규칙 같음), `ChronicleLetGo`는 `State.ChronicleEviction.Count`.
- 요약 줄(646)의 끝: A-5a `…; save %d bytes (%lld before packing) in %.1f ms; log %d, chronicle %d (%d kept for good), told %lld`, A-5b `…; log %d, chronicle %d (%d kept for good), let go %d, told %lld` — `kept for good` = `Chronicle`에서 `!MayLetGo`인 줄 수. 압축 전 길이는 `int64`라 `%lld`(UE_LOG 꼴은 컴파일 때 검사된다).
- 측정 줄(A-5a, 기준값 비교 밖): 마지막 표본의 엔진 바이트를 `FOodleDataCompression::Compress`로 Kraken SuperFast·VeryFast·Fast 세 번 따로 압축해 `CosmicBalance pack: raw %lld; superfast %lld in %.1f ms; veryfast %lld in %.1f ms; fast %lld in %.1f ms`.
- 시각 0을 주므로 같은 상태는 같은 머리를 낸다(압축 바이트의 같음은 단언하지 않는다).

## 6. 테스트의 세부

### 6.1 도우미
- **`CosmicTests::AcquireSim`**(`Tests/CosmicTestHelpers.h:72-93`): `Sim->SetTimeScale(0)` 앞에 `if (UCosmicSaveSubsystem* Save = GameInstance->GetSubsystem<UCosmicSaveSubsystem>()) { Save->PrepareForTests(); }`. 주석에 '테스트의 슬롯은 `Saved/SaveGames/Automation/`에만, 게임 저장의 문은 닫힌다'.
- **`FillEmptyContainers`**(`CosmicSimTests.cpp:279`): `State.ChronicleEviction.Count = 3; State.ChronicleEviction.ThroughTick = 17;`. 나중 멤버의 값을 두는 곳은 이 함수 하나다(`Sim.TellStreamSeparate`(1623)와 `Sim.SaveRoundTrip`(1699)이 이미 부른다 — 둘째 설정 함수를 두지 않는다). 주석의 '25'는 v0.7의 셈 그대로 두고 '나중 멤버(MVP ⑤의 `ChronicleEviction`)도 여기서 둔다 — 비교는 `SameLaterMembers`'를 더한다.
- **새 `SameLaterMembers(A, B)`**(`CosmicSimTests.cpp`의 같은 namespace): `ChronicleEviction`만 필드별 비교. 청원 둘은 MVP ②의 멤버라 전체 해시와 `Sim.SavePetition`이 이미 덮는다. `Sim.SaveRoundTrip`만 부른다.
- **`FScopedProbe`를 옮긴다**(A-5c): 지금은 `Tests/CosmicEventTests.cpp`의 `namespace CosmicEventTests`(37행) 안(141-160행)이라 다른 테스트 파일에서는 보이지 않는다(unity 빌드로 우연히 보이는 데 기대지 않는다). `Tests/CosmicTestHelpers.h`의 `namespace CosmicTests`로 그대로 옮기고, 그 헤더에 `Events/CosmicEventSubsystem.h`·`Tests/CosmicTestProbe.h`를 더한다(`Log/CosmicLogSubsystem.h`는 이미 있다). `CosmicEventTests.cpp`는 제 namespace 안에 `using CosmicTests::FScopedProbe;` 한 줄을 두어 지금의 쓰임을 고치지 않는다. `Sim.SaveTriggers`는 `CosmicTests::FScopedProbe`를 쓴다.
- **슬롯 이름과 경로는 `Save->SlotNameOf(ID[, true])`·`Save->SlotPathOf(ID[, true])`로만 만든다**(`MakeSlotName`은 폴더가 없어 게임 폴더에 닿는다 — `SaveRestart`의 옛 슬롯 지움만 예외). 지금의 `Sim.SaveRoundTrip`(1796·1800·1804·1825)이 `MakeSlotName` + `UGameplayStatics::LoadDataFromSlot`/`SaveDataToSlot`을 직접 부르므로 A-5a에서 모두 바꾼다 — 그대로 두면 `Automation/`에 쓴 슬롯을 게임 폴더에서 찾다 실패하고, '다른 은하' 슬롯이 게임 폴더에 남는다.
- **슬롯 바이트 도우미**(GUID를 받는다): `CutSlot(Save, ID, bBackup, NewLength)` — `FFileHelper::LoadFileToArray` → `SetNum(NewLength)` → `SaveArrayToFile`, 모두 `SlotPathOf`. **바이트 뒤집기** `FlipByte(Bytes, At)`. **머리 다시 셈** `RewriteHeader(Bytes, Edit)`: 앞 60바이트를 고친 뒤 `HeaderCrc`를 다시 셈한다(코덱·크기·꼴 경우에만 — `RawBytes`를 속여 풀기에 닿는 슬롯은 만들지 않는다, 헤더 §11).
- **잠금 흉내**: `FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*Path, /*bAppend*/ true, /*bAllowRead*/ false)`를 쥔 채(자르지 않고 읽기 공유를 막는다) 부른다. 손잡이는 `ON_SCOPE_EXIT`에서 놓는다. 읽기 전용은 `SetReadOnly(*Path, true)`, 되돌림도 `ON_SCOPE_EXIT`.
- **새 지연 명령** `FWaitForNewGameWorld(TWeakObjectPtr<UWorld> Old, double TimeoutSeconds, FAutomationTestBase* Test)`(`Tests/` 안): `AutomationCommon::GetAnyGameWorld()`가 `Old`와 다른 월드이고, `AreActorsInitialized()`이고, `GetGameState()->HasMatchStarted()`이고, 첫 컨트롤러가 `ACosmicPlayerController`이며 그 `ACosmicHUD`에 연대기 위젯이 있으면 참. 제한 시간을 넘으면 `Test->AddError(…)` 뒤 참. 맵을 다시 여는 두 테스트는 `Old = GetAnyGameWorld()`를 잡고 `FLoadGameMapCommand(TEXT("/Game/CA/Maps/L_Galaxy"))` → `FWaitForNewGameWorld(Old, 10.0, this)` → 단언 명령(두 `BeginPlay`는 `LoadMap` 안에서 이미 돌았다). `FWaitForMapToLoadCommand`는 쓰지 않는다(옛 월드로 같은 프레임에 끝날 수 있다).
- 고정 GUID: `SaveFind`는 `{0x5A5E0001,1,1,1}`·`…0002`·`…0003`(A·B·C)과 미끼 `…0004`(D), `ContinueEnded`는 `…0010`, `ContinueRunning`은 `…0011`. 각 테스트는 시작과 끝에 `DeleteSave(제 GUID)`를 부르고, 심은 파일은 경로로 지운다(`IFileManager::Delete` — `DeleteSave`는 미끼 이름을 지우지 않는다). `SaveFind`는 제 폴더(`Automation/Find/`)의 `*.sav`를 시작과 끝에 모두 지운다.

### 6.2 테스트마다
| 테스트 | 단계와 값 | 변이(`Tools/mutations/`에 남긴다) |
|---|---|---|
| `Sim.SaveSlotForm` | 슬라이스(`SliceSystems`·`SliceCivs`) 시드 42 + `RunHeadless(1000)`, 운반 객체에 복사, `Encode(…, 123456789)`. 거절 경우: 0·63바이트 `Short`, 64바이트(머리만)·`−1`·`+1`(0 하나 덧붙임) `Truncated`, 엔진 꼴(`SaveGameToMemory` 그대로) `OldForm`, 첫 낱말 `'XXXX'` `NotOurs`, 꼴 2(CRC 다시 셈) `NewerForm`, 꼴 2(CRC 그대로) `HeaderDamaged`, 꼴 0(다시 셈) `BadSizes`, 머리 32번 바이트(`Tick`) 뒤집음 `HeaderDamaged`, 코덱 9·`RawBytes` 65 MB(다시 셈) `BadSizes`, 기대 GUID 다름 `OtherGalaxy`, 페이로드 `64 + Packed/2` 뒤집음 `PayloadDamaged`, Stored 슬롯을 `64 + (엔진 바이트에서 ANSI 'GalaxyID' 이름이 시작하는 자리 − 4)`(그 태그의 이름 길이 앞 = `TellRngState` 값의 끝)에서 자름 `Truncated`. `ParseSlotName`: `Galaxy_` + 32 대문자, 32 소문자, + `_bak`, 31자리, `Galaxy_<32>_tmp`, `Galaxy_<32> (1)`, `Galaxy_<32>-DESKTOP` | ① 길이 검사 빼기(`+1` 경우가 다른 까닭으로 바뀐다. 머리만·`−1` 경우는 페이로드 CRC가 버퍼 밖을 읽어 프로세스가 죽을 수 있어, 이 변이는 따로 돌린다) ② `ReadHeader`의 CRC 길이만 60 → 64(상수를 바꾸면 `WriteHead`의 `check`가 프로세스를 죽인다) ③ 꼴을 CRC보다 먼저(꼴 2·CRC 그대로 경우가 `NewerForm`) ④ 'GVAS'를 우리 것으로 ⑤ 페이로드 CRC 빼기 |
| `Sim.SaveBackup` | 슬라이스 시드 42 → 100틱 → 저장(백업 없음) → 100틱 → 저장(백업 머리 Tick 100, 주 200) → 주를 반으로 → `LoadRun` = Ok·백업에서·틱 100·해시 = 100틱 때의 해시(Warning 1: `read from its backup`) → (주가 잘린 채) 50틱 → 저장(Warning 1: `damaged … not copied`) → 백업 머리 Tick 여전히 100 → 다른 판의 온전한 주 바이트를 이 이름으로 → 백업에서(Warning 1) → 둘 다 자름 → 거절·해시 그대로(Warning 1: `its backup:`) → 주에 `SaveVersion = 2`인 상태를 `Encode`해 씀 → `NewerSave`(버전 Warning 1), `read from its backup` 줄 없음 → 주의 꼴을 2로(CRC 다시 셈) → `NewerForm`(Warning 1), 백업 줄 없음, 저장 false·주 바이트 그대로(Warning 1: `not replaced`) → 온전한 주로 되돌려 잠금 → `LoadRun` = `ReadFailed`(Warning 1), 저장 false·주 그대로(Warning 1), 놓음 → `_bak` 읽기 전용 → 저장 false(Warning 1: `backup of … was not written`), 주 바이트 그대로(앞뒤 비교), 되돌림 → `DeleteSave` → 둘 다 없음 | ① 검사 없이 백업으로 옮김 ② 상함에 백업을 안 봄 ③ `NewerSave`에도 백업으로 감 ④ 백업 실패 뒤에도 주를 씀 ⑤ `NewerForm`을 상함으로 ⑥ `ReadFailed`를 상함으로 |
| `Sim.SaveFind` | `SetFolderForTest("Automation/Find/")`, 폴더를 비움. 게임 폴더(`Saved/SaveGames/*.sav`)의 목록을 적어 둔다. 순수 `NewestRun`: `{A 주 5}`·`{A 백업 9}`·`{B 주 8}` → A / `{B 주 99, 파일 시각}`·`{A 주 5}` → B / 같은 시각 → GUID 큰 쪽. 디스크(시각은 지난 것 — 지금 − 30·20·10초): A·B·C를 `Encode`해 `SlotPathOf`에 씀 → `ContinueNewest` = `Newest`·C / C 백업을 지금 − 15초로 둔 뒤 C 주를 페이로드 가운데서 자름(머리 온전) → `Backup`·C / C 주·백업 페이로드 뒤집음 → `Unreadable`, 시뮬 그대로, C의 두 파일 그대로 / C의 두 파일을 0바이트로(파일 시각 = 지금, 가장 새것) → `Unreadable`(B를 열지 않는다) / C를 되돌리고 미끼: D의 온전한 머리(지금 + 9초)를 `Galaxy_<D>_tmp.sav`·`Galaxy_<D> (1).sav`에, `Galaxy_123.sav`·`Other.sav` → 여전히 `Newest`·C / 끝에 게임 폴더의 목록이 같다. 판의 절대 수는 단언하지 않는다 | ① 차례 뒤집기 ② 판의 시각이 백업의 머리를 보지 않음 ③ `Unreadable`에서 옛 판으로 내려감 ④ 이름 거름이 `_tmp`·`(1)`을 받음(D가 골라진다) |
| `Sim.SaveNeutral` | 슬라이스 시드 42를 두 번 2,000틱: 하나는 100틱마다 `SaveCurrentGalaxy`(20번), 하나는 없이. 끝의 전체·세계 해시·`RngState`·`TellRngState`가 같다. 첫 판의 저장 하나 직전·직후 `ComputeStateHash` 같다. 끝에 슬롯 지움 | ① 저장이 `DrawTell(2)`를 하나 뽑음 ② 저장이 로그 줄을 하나 씀 |
| `Log.ChronicleCap` | `WithRules(ChronicleCap = 10)`. 생애 하나(백성 `civ_0`). `State.Log`·`State.Chronicle`을 비운 뒤 **섞은 묶음 30줄을 먼저** 민다(틱 1~30 — S1 `civ_0` 24줄, 지울 수 없는 줄 6: S2·`Log.Calling`·`Log.Claim`·`Log.Record`), 그 뒤 S0 `Log.Civ` 채움 2,000줄을 민다 → 머리부터 떠나므로 떠나는 것은 정확히 묶음이고, 채움은 `Admits`가 받지 않아 연대기에 들지 않는다. 기대값은 떠난 묶음에서 테스트의 식으로 센다(`Log.CapAndChronicle`의 꼴): `Num() == 10`, 지울 수 없는 6줄 모두, 남은 S1 = 가장 새 4줄, `Count` = 20, `ThroughTick` = 나간 S1의 가장 늦은 틱, 차례 그대로. 상한 3: 같은 차례로 `Count` = 24, `Num()` = 6. 상한 0: `Count` 0, `Num()` = 30. 세계 해시: 같은 판의 상한 0과 10 | ① S1 대신 아무 Severity ② 가장 새것부터 ③ `ThroughTick`에 가장 이른 틱 ④ `>` 대신 `>=` ⑤ 0을 '모두 비움'으로 |
| `Sim.SaveTriggers` | `SetGameSavingForTest(true)`, `SetTimingForTest(1e9, 0)`, 방송 열림, 슬라이스 시드 42. 셈은 `GetSavesWritten()`의 차와 `GetLastCause()`. 업적은 `Calling.CompletedFeats.Add(feat_test)`, 역할은 `Player.Role = Wanderer`, 끝은 `State.Tick = ProvisionalEndTick − 1` 뒤 `StepOnce`. 핸들러는 `CosmicTests::FScopedProbe`(§6.1 — A-5c가 옮긴다)의 `OnLog`(`CosmicTestProbe.h:33`)에 람다를 둔다. **실패 묶음**(케이스 차례에 기대지 않는다. 산 컨트롤러가 `OnSimTick`에 묶여 있어 그 `HandleSimTick` → `FollowSaveFailure`가 경계의 실패 깃발을 같은 방송에서 가져갈 수 있으므로 — 헤더 §4.5 — 깃발은 경계 없는 입력으로 단언한다. `SetTimingForTest`는 `RetryNotBefore`를 0으로 두므로 기다림을 보는 경계 앞에서는 부르지 않는다): ⓐ `SetTimingForTest(1e9, 1e9)` → 주를 읽기 전용으로 → 입력 때의 `RequestSave(Answer)`(경계 없음) → 쓰기 0 · `TakeFirstFailure()` 참 → 다시 부르면 거짓 ⓑ 주를 되돌린 뒤 한 경계(`StepOnce`) → 0(60초 기다림) ⓒ `SetTimingForTest(1e9, 0)` → 한 경계 → 1, `GetLastCause() == Answer`(간격이 1e9라 남은 까닭만 쓸 수 있다) ⓓ 경계의 실패: `SetTimingForTest(0, 0)` → 주를 읽기 전용으로 → 한 경계 → 0(`Interval`이 남는다) → **파일을 되돌리기 전에** `SetTimingForTest(1e9, 0)` → 되돌린 뒤 한 경계 → 1, `GetLastCause() == Interval`. **결정 6의 건너뜀**: `NewGalaxy`(시드 7) → `HoldStartRun()` → `RequestSave(Exit)` → 0(Log `not written yet`) → `NewGalaxy`(시드 8, 다른 판) → `RequestSave(Exit)` → 1(`Exit` — 둘째 판의 슬롯도 끝에 지운다). 문의 순수 함수: `CommandLineAllows(TEXT("-NoSave"))`·`(TEXT("-unattended"))`·`(TEXT("-Seed=42"))` 거짓, `(TEXT("-game -windowed"))` 참 / `GateClosedBy`의 까닭마다(명령줄·테스트·자동화·에디터) 그 이름, 모두 풀림 null, 덮어쓰기 null. 문 닫힘: `SetGameSavingForTest(false)` 뒤 같은 것들을 다시 → 0. 끝에 `PrepareForTests()`와 슬롯 지움 | ① GalaxyID 규칙 빼기 ② 종료의 해시 비교 빼기 ③ 핸들러 안 저장 허용 ④ `GateClosedBy`의 까닭 하나 빼기(까닭마다 한 번) ⑤ 실패가 까닭을 버림(ⓒ·ⓓ가 잡는다 — 간격이 1e9라 `DueAt`이 대신 쓰지 못한다) ⑥ 60초 기다림(`Now < RetryNotBefore`) 빼기(ⓑ가 잡는다) ⑦ 종료 건너뜀이 GalaxyID를 보지 않음(세션 전체 — 둘째 `Exit`가 0이 된다) |
| `UI.ContinueEnded` | 월드 문맥이 하나이고 `Game`이 아니면 `AddError`로 끝(`FLoadGameMapCommand`의 `check`). 고정 GUID `…0010`의 상태 E: 슬라이스 시드 42, `Tick = ProvisionalEndTick`(UI.RunEnd ⑦의 끝난 사본과 같은 길), `PendingPetitionRow` 대기 → `SaveCurrentGalaxy` → 해시 H. `NewGalaxy(7, …)`(다른 은하). `SetGameSavingForTest(true)`, `ContinueAtNextStartForTest()`. 지연 명령(§6.1) → 단언(새 컨트롤러·HUD를 `GetFirstPlayerController`로): GalaxyID·해시 = H, 래치, 쪽 열림·다 드러남, 카드 없음, 배속 0·`RunEnd`, `GetLastContinue() == Newest` → `SetGameSavingForTest(false)`·`DeleteSave` | ① HUD `BeginPlay`의 `FollowEnd(false)`(283) 지우기 — 엔진의 차례에서 뒤에 오는 부름이다. 컨트롤러의 부름을 지우는 변이는 살아남을 것으로 적어 둔다(뒤의 부름이 쪽을 연다). ①이 살아남으면 차례가 엔진 소스와 다르다는 뜻이니 컨트롤러의 부름으로 다시 본다 ② 이어하기를 `Super::StartPlay()` 뒤의 방송 없는 로드로(`SetBroadcastEnabled(false)`로 감쌈) |
| `UI.ContinueRunning` | 고정 GUID `…0011`의 흐르던 상태(슬라이스, 40틱, 카드 `petition_wise_heart` 대기) → 저장 → H → `NewGalaxy(7, …)` → 같은 지연 차례 → 해시 = H·배속 0·까닭 `Continued`·카드 보임 → 주를 읽기 전용으로(`ON_SCOPE_EXIT`에서 되돌림) → `AnswerPetition(1)` → 쓰기 실패 → 배속 0·까닭 `SaveFailed`(한 번) → 되돌리고 `SetTimingForTest(1e9, 0)` → `StepOnce` 한 번(`Space` 길로 ×1에서 한 경계여도 된다) → `GetLastCause() == Answer`, 슬롯 머리의 `Tick` = 지금 틱, `GetSavesWritten()` +1 → Space 길(`:1619`) → ×1, 까닭 줄 사라짐 → 지움 | ① `FollowStart` 빼기 ② `AnswerPetition`의 `RequestSave` 빼기 ③ `FollowSaveFailure` 빼기 |
| `Log.CapAndChronicle` | 지금의 2,000 + 넘침 차례에 S1의 초점을 넷으로 섞는다: 그릇의 백성, AI 백성, 없음, `log_first_contact`(초점 = AI 백성). 기대 = `S ≥ 2 ∨ Calling ∨ Claim ∨ Record ∨ (S1 ∧ (자기 백성 ∨ 첫 접촉))`을 테스트 안의 식으로. 둘째 생애(손으로 `Lives.Add`, `BirthCivID` = AI 백성)를 둔 판에서 첫 생애 백성의 S1도 남는다. **참조 상태는 손으로 짓는다**(`MaxLogEntries`는 `static constexpr`라 상한을 풀 수 없다): 밀기 전의 `State.Log`를 사본으로 두고, `Push`마다 `State.Log.Last()`(민 줄은 늘 끝에 있다)를 모은 뒤 `Ref = State; Ref.Log = 처음 + 모은 것; Ref.Chronicle = {}` — `BuildCapped`는 순수라 `Ref.Log`가 2,000을 넘어도 된다. `BuildCapped(Ref, 40)`과 `BuildCapped(State, 40)`의 장별 줄과 `Candidates`가 같다 | ① 옛 규칙 `Severity >= 1` 되살리기 ② 마지막 생애의 백성만 ③ 첫 접촉 빠뜨리기 |
| `Log.ChronicleBuild` | 생애 둘(둘째 `BornTick` 500), 줄 없이 밀어냄 기록만 세운다: {1, 300} → 장 0만 `bMore`, {1, 500} → 둘 다, {0, 900} → 둘 다 아님 | ① `Count` 무시 ② `ToTick`과 비교 ③ `<=` 대신 `<` |
| `Sim.SaveRoundTrip` | `FillEmptyContainers`(1699 — 밀어냄 기록 {3, 17}도 둔다) 뒤 저장, 로드 뒤 `SameLaterMembers`. 0인 밀어냄 기록: 같은 상태를 {0,0}으로 `Encode` → `Unpack` → 엔진 바이트에 ANSI `ChronicleEviction`이 없다, {3,17}이면 있다(1811-1821의 찾기). 자르기(`CutSlot(Save, ID, false, …)`): 머리 안(30), 머리 끝(64), 페이로드 가운데, 1바이트 짧음 — 백업이 없어 거절, 해시 그대로. 옛 꼴: `UGameplayStatics::SaveGameToSlot(Carrier, Save->SlotNameOf(ID), 0)` → `OldForm`. 다른 은하: 온전한 바이트를 `Save->SlotPathOf(ForeignID)`에. 기대 경고: `LoadGalaxy: slot .*(is missing|: )` 꼴로 7(자르기 넷·옛 꼴·다른 은하·없음), 버전 줄 1 | ① `FChronicleEviction::ThroughTick`의 `UPROPERTY()` 빼기(필드별 비교가 잡고 해시는 못 잡는다) ② 밀어냄 기록의 초기값을 −1로 |
| `Sim.TellStreamSeparate` | `NotWorld`에 `ChronicleEviction`을 더하고 `Seen == 19` | ① `NotWorldNames`에서 빼기 ② `FillEmptyContainers`의 밀어냄 기록 줄 빼기 |
| `Sim.NotablesFromEventsOnly`(A-5b) | `bToldAtFirstTick &= Told(…) || (Record.FirstTick <= State.Log[0].Tick && !CosmicChronicle::PeoplesOfEveryLife(State.Calling).Contains(Civ.CivID))` — 같은 틱의 줄 여럿 가운데 일부만 떠났을 수 있어 `<=` | — |
| `Sim.SaveRestart`(A-5a) | 1912-1916의 '다른 빌드의 기록' 가지에 `UGameplayStatics::DeleteGameInSlot(UCosmicSaveSubsystem::MakeSlotName(RecordedGalaxyID), 0)`(게임 폴더의 옛 슬롯 — 일부러 `MakeSlotName`) | — |
| `Events.RowAtomic` | 1719행 옆: 핸들러 안에서 `SaveCurrentGalaxy()`가 false, `DoesSaveExist(Later.GalaxyID)` 거짓. `AddExpectedError(TEXT("SaveCurrentGalaxy called from a handler of a Core Logic delegate"), Contains, 1)` | ① 방송 거절 빼기 |
| `UI.AutoPause` | 까닭 고리(156)에 새 넷: `WhyKeyOf` = 각 키, 글이 ST_CA에 있다 | ① 키 하나 바꾸기 |

### 6.3 테스트 수
- A-5a: `Sim.SaveSlotForm`·`Sim.SaveBackup`·`Sim.SaveFind`·`Sim.SaveNeutral`(+4 → 110). 고침: `Sim.SaveRoundTrip`(새 감싸기·자르기·옛 꼴·`SlotNameOf`·경고 문구), `Events.RowAtomic`, `Sim.SaveRestart`(옛 슬롯 지움 한 줄 — 폴더는 `AcquireSim`이 바꾼다).
- A-5b: `Log.ChronicleCap`(+1 → 111). 고침: `Log.CapAndChronicle`, `Log.ChronicleBuild`, `Sim.TellStreamSeparate`, `Sim.SaveRoundTrip`(나중 멤버·0인 밀어냄 기록), `Sim.NotablesFromEventsOnly`.
- A-5c: `Sim.SaveTriggers`·`UI.ContinueEnded`·`UI.ContinueRunning`(+3 → **114**). 고침: `UI.AutoPause`. 옮김: `FScopedProbe`(`CosmicEventTests` → `Tests/CosmicTestHelpers.h`의 `CosmicTests`, §6.1 — 테스트 수와 동작은 그대로).
- 변이: 새·넓힌 단언에 모두 46개(표의 합 — `Sim.SaveTriggers`가 검증 뒤 5 → 7). 살아남은 변이는 CHANGELOG에 까닭과 함께 적는다(A3의 꼴 — `UI.ContinueEnded`의 컨트롤러 부름은 미리 적은 생존이다).

## 7. A-5a의 측정 (CHANGELOG에 적는다)
`run_checks.ps1`의 `$Editor`와 같은 실행 파일로, 에디터를 닫고:
```
-run=CosmicBalance -Seed=42 -Ticks=20000 -Out=a5_20k.csv            (전부 말하기, 6,000·10,000·20,000 표본)
-run=CosmicBalance -Seed=42 -Ticks=20000 -TellAll=0 -Out=a5_20k_scoped.csv
-run=CosmicBalance -Seed=42 -Ticks=3000 -TellAll=0 -Answer=1 -Build=1 -Out=a5_ruler.csv
-run=CosmicBalance -Seed=42 -Ticks=20000 -Systems=300 -Civs=48 -Out=a5_ea.csv
```
각 줄에 `-unattended -nosplash -stdout -FullStdOutLogOutput`. 적을 것: 표본마다 `SaveBytes`(슬롯)·`SaveRawBytes`·비율, 요약 줄의 ms·`kept for good`, `CosmicBalance pack:` 줄(수준 셋의 크기와 압축만의 ms). 게임의 쓰기와 내리기 시간은 손 확인 ⓐ의 `CosmicSave:` 줄(VS)에서 옮긴다. **멈춤 규칙(두 쪽)**: EA 20,000틱의 슬롯이 1 MB를 넘으면, 압축·직렬화·쓰기·내리기의 합이 100 ms 안에 머무는 동안만 수준을 올린다(VeryFast → Fast → Normal). 크기와 시간을 함께 맞출 수 없으면 멈추고 개발자에게 묻는다(헤더 '추가 이야기'의 비동기 쓰기). EA의 상태는 거르기 전이라 연대기가 더 길다 — 그래서 이 측정은 거른 뒤보다 크게 잰다(지울 수 없는 줄의 수는 같다). A-5b 뒤 같은 넷을 다시 돌려 `ChronicleLen`(거르기 뒤)과 `let go`를 적고, 세계 해시가 20,000틱에서 `885018087`인 것과 새 전체 해시(지금 `2410091205`)를 확인해 `seed42.json`의 기록 칸과 STATUS에 적는다.

## 8. 로그 줄 모음 (테스트의 기대 메시지)
| 자리 | 꼴 | 수준 |
|---|---|---|
| 쓰기 | `CosmicSave: wrote <slot> (<cause>) tick <n>, hash <h>, <packed> of <raw> bytes in <ms> ms` | Display |
| 백업 실패 | `CosmicSave: the backup of <slot> was not written: <slot> is kept as it was` | Warning |
| 덮지 않는 주 | `CosmicSave: <slot> is ReadFailed|NewerForm: it is not replaced` | Warning |
| 상한 주 | `CosmicSave: <slot> is damaged (<까닭>): not copied, the backup stays` | Warning |
| 쓰기 실패 | `CosmicSave: <slot> was not written` / `… could not be packed` | Warning |
| 핸들러·단계 안 저장 | `SaveCurrentGalaxy called from a handler of a Core Logic delegate: …` / `SaveCurrentGalaxy called inside the tick stages: …` | Error |
| 종료 건너뜀 | `CosmicSave: exit - the slot already holds this state` / `CosmicSave: exit - the new run after an unreadable save is not written yet` | Verbose / Log |
| 로드 | `CosmicSave: loaded <slot>[ from its backup] at tick <n>, hash <h>` | Display |
| 백업에서 | `LoadGalaxy: slot <slot>: <까닭>; read from its backup (tick <n>)` | Warning |
| 없음 | `LoadGalaxy: slot <slot> is missing` | Warning |
| 둘 다 상함 | `LoadGalaxy: slot <slot>: <까닭>; its backup: <까닭>` | Warning |
| 읽히지 않음 | `LoadGalaxy: slot <slot> could not be read: the backup is not tried` | Warning |
| 새 꼴 | `LoadGalaxy: slot <slot> has slot form <f>, this build reads up to <F>` | Warning |
| 버전·성역 | 지금 문구 그대로(`…has save version %d, this build reads up to %d` / `…does not end with a kept sanctuary…`) | Warning / Error |
| 핸들러·단계 안 로드 | `LoadGalaxy called from a handler…`(지금 그대로) / `LoadGalaxy called inside the tick stages: …` | Error |
| 이어하기 | `CosmicContinue: <outcome> <slot> at tick <n>, hash <h> (<k> runs seen)` | Display |
| 옛 꼴 슬롯 | `CosmicContinue: <slot> is in the slot form before item 5 - not a run to continue` | Log |
| 문 | `CosmicSave: the game's saving is allowed|off by the command line`(Initialize) / `CosmicContinue: Off (command line|tests|automation|editor)`(BeginSession) | Log |
| 끝 | `CosmicEnd: <까닭> at tick <틱>; <초> s since tick <n>, <초> s of them stopped` / `CosmicEnd: <까닭> at tick <틱>; opened ended (continued)` | Display |
| 측정 | `CosmicBalance pack: raw <n>; superfast <n> in <ms> ms; veryfast …; fast …` | Display |

## 9. 문서와 코드 주석 (커밋마다)
**A-5a — 같은 커밋에서 코드보다 먼저**
- `docs/20` §9.1 173-174행: 쓰는 함수 '`UGameplayStatics::SaveGameToSlot`로 끝난다' → '`SaveGameToMemory`의 바이트를 슬롯의 꼴(머리·압축·내린 쓰기 — 헤더 `A5_save_minimum.md` §3)로 쓴다'. 델타 규칙의 문장은 그대로(엔진 바이트가 같다).
- 코드 주석: `Core/GalaxyState.h:8`('SaveGameToSlot writes a delta' → 'the engine's save bytes (SaveGameToMemory) are a delta'), `Save/CosmicSaveGame.h:13-14`, `Tests/CosmicBalanceCommandlet.cpp:285-286`.

**A-5b — 같은 커밋에서 코드보다 먼저**
- `docs/20` §9.1: 새 필드와 델타에 안전한 까닭. §9.2 195행: 세계 해시 제외에 `ChronicleEviction`.
- `docs/21` 477행 최상위 19, `FChronicleEviction` 행, 479행(초기값 동결), 527·529행 이관 = `Admits`(모든 생애의 백성)·상한·밀어냄 기록.
- `CLAUDE.md` 규칙 6의 제외 목록. 코드 주석 `GalaxyState.h:19, 764-769`, `Tools/check_layers.py:24`의 `BASELINE` 주석(41 → 39).

**A-5d — 남은 문서**
- **`docs/20`**: §9.1 — 슬롯의 꼴 표, 쓰는 차례와 '좋은 사본 하나'(내리기), 상함만 백업으로(없음은 백업, 읽히지 않음·새 꼴은 거절), 트리거 표와 코드 자리, 이어하기(머리만 읽기·파일 시각의 대체값), 게임 저장의 문(`GIsEditor`), 테스트 폴더. '첫 자동 저장은 저장 작업 뒤다'(180행)를 현행으로. 176행에 '10분 = 실제 시간, 틱 경계에서'. 181행 끝에 '⑤ 뒤 끝난 MVP 판은 세이브에 남는다 — B6가 임시 끝을 바꿀 때 그 세이브를 무엇으로 열지 정한다(#117)'. §9.4 249행: 슬롯 바이트 예산과 A-5a의 측정값. 251행(해시 시간): '해시는 테스트·커맨드렛 전용' → '게임은 저장·로드·종료 비교 때 한 번씩 셈한다(약 4 ms, 300·48) — 틱마다는 셈하지 않는다'. 242·295행의 'SaveCurrentGalaxy는 막는다'를 현행으로. §6.3 77행: '④에는 저장이 없어 이 판은 남지 않는다' → '끝난 판은 저장에 남고 이어하기는 끝난 채 연다 — 새 판 뒤에는 새 판을 잇는다', 그 행의 `CosmicEnd:` 줄 인용을 새 꼴로(`since tick <n>`·`opened ended`). 같은 행의 '술어를 읽는 곳은 이 넷뿐이다'에 '+ 저장 층의 경계 비교(`UCosmicSaveSubsystem::MarkOf`, 읽기만)'를 더한다 — ⑤ 뒤에는 `MarkOf`가 `ProvisionalEndOf`를 읽어 '넷뿐'이 거짓이 된다. 부록: 핸들러 거절 목록에 저장을 더한다.
- **`docs/21`**: `UCosmicSaveSubsystem` 인터페이스 블록(지금 없다)과 `CosmicSaveSlot`, 노브, 1204행(핸들러 거절 목록에 `SaveCurrentGalaxy`·`LoadRun`·`ContinueNewest`), 1206행('읽는 곳은 플레이어 컨트롤러의 FollowEnd와 커맨드렛뿐'에 '+ 저장 층의 경계 비교, 읽기만'), 1215행(저장 직전의 `SyncRngState`), 1271행(자동 저장의 자리), 1432행의 'MVP ⑤ … 예정'을 지운다.
- **`docs/70` §8.4**: 161행 이관 규칙을 `Admits`로, 상한과 밀어냄 기록과 `Chronicle.More`. 167행의 '예정'을 지운다.
- **`docs/40`**: 26행('자동 저장(승천·역할 전환·10분·종료)은 틱 경계에서만' → 경계의 넷과 입력 때의 셋 — 답·새 판·종료), 테스트 표(새 8, 고친 8), 게임 저장의 문(`-NoSave`·`-unattended`·`-Seed=`·에디터 프로세스 — 에디터 안 플레이는 저장하지 않고 Play ▸ Standalone Game은 저장한다), 테스트 폴더 `Saved/SaveGames/Automation/`, 손 확인 ⓐ의 명령, CSV 열(`SaveBytes`의 새 뜻·`SaveRawBytes`·`ChronicleLetGo`)과 `CosmicBalance pack:` 줄, 200·201행 `SaveRoundTrip`·`SaveRestart` 문장, 439행의 `-Speed=N(새 은하의 시작 배속)` → '새 은하와 이어 연 판의 시작 배속(이어 연 정지를 건너뛴다)'과 `CosmicEnd:` 줄 인용.
- **`docs/45`**: §1.2 표의 엔진 경로(Oodle 속도표·CRC·저장 시스템·`IPlatformFile`의 쓰기와 `Flush`·읽기의 `FILEREAD_Silent`·지연 명령과 같은 프레임의 함정·`LoadMap`의 `Destroyed`/`LevelTransition`과 `BeginPlay`의 차례·`GIsEditor`·`GIsAutomationTesting`의 범위).
- **`docs/15`**: '이어하기'·'백업 슬롯'·'밀어냄 기록'(`FChronicleEviction`)·'디스크 사본'(`FCosmicSavedMark`)·'봉투'(슬롯 머리에서 모든 꼴이 옮기지 않는 자리) 행. **`docs/30`**: 다시 만든다.
- **`crisis_and_transcendents.md`** 138행: '영겁 모드도 이어하기도 없고' → '영겁 모드(끝 뒤로 계속하기)는 없다 — 재시작 뒤 이어하기는 끝난 판을 끝난 채 연다'.
- **`docs/50`**: A ⑤ 날짜와 링크, 19행 위험 칸('게임 코드의 저장 호출 0건·세이브 1MB 초과')을 닫는다.
- **`OPEN_QUESTIONS`**: #33·#70·#40 ⑦을 닫고 `DECISIONS_LOG`로 옮긴다. #9는 그대로. **#117에 한 줄**: '⑤ 뒤 끝난 MVP 판은 세이브에 남는다 — B6가 임시 끝을 바꾸거나 노브가 바뀔 때 그 세이브를 끝난 채 열지 흐르는 판으로 열지 정한다'. 새 행 '이관 범위와 즉위(Phase 4·4b 헤더 전)'(헤더 §6.5).
- **`CLAUDE.md`**: 규칙 5에 '핸들러 안의 저장은 거절되고 `RequestSave`만 남는다', 규칙 15의 '현재 코드'에 MVP ⑤ 한 줄. **`STATUS.md`**: '다음 3작업' 3의 effort는 헤더 결정 18의 권장(구현 high에 ultracode 끔)이면 그대로 두고, 대안(구현 xhigh)이면 '구현 xhigh'로 고친다, 미룬 손 확인 1의 '새 판부터 ×10으로 셋째 칸 → … → 새 판(두 번)'에 시작 조건('`Saved/SaveGames/`를 비우거나 `-NoSave`로 — ⑤ 뒤에는 마지막 판을 이어 연다'). **`docs/CHANGELOG.md`**(측정·해시·리뷰 통계), **`DECISIONS_LOG`**(단계 0의 답을 들은 뒤).
- **코드 주석**(코드와 함께 고치되 목록은 여기): **A-5c** — `Sim/CosmicEndings.h:29`의 'Read by FollowEnd and the commandlet' → 'Read by FollowEnd, the commandlet and the save layer's tick-boundary check (UCosmicSaveSubsystem::MarkOf), read only'(`MarkOf`가 `ProvisionalEndOf`를 읽는 커밋에서). 그 밖은 `Save/CosmicSaveSubsystem.h:11-14`, `Log/CosmicLogSubsystem.h:122`, `CosmicChronicle.h:2-3, 28`, `Sim/GalaxySimulationSubsystem.h:252-260`, `World/CosmicGameMode.cpp:108-110`(`-Speed=`), `World/CosmicGameMode.h:261`(`StartNewRun`이 새 판을 저장한다), `UI/CosmicChronicleWidget.cpp:370`('The run is not kept (item 4 has no save)' → 'The run's save stays; the continue opens the new run').
- A4 헤더는 승인된 기록이라 고치지 않는다.
