# Cosmic Ascension — Claude Code 작업 규칙

## 프로젝트

UE 5.8.3 (`C:\Program Files\Epic Games\UE_5.8`), C++ 게임 모듈 `Cosmic_Ascension`. 설계 기준 문서: `guideline.md` (섹션 번호로 참조. 결정이 바뀌면 guideline.md를 먼저 고친다)
1인 개발. 린 시뮬레이션. 3D 에셋·캐릭터 모델 참조 금지. 데이터는 DataTable(FTableRowBase), 식별자 FName, 분류 FGameplayTag, 표시 문자열 FText.
프로젝트 루트: `C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension` (OneDrive 안. 프로젝트를 옮기면 이 파일과 guideline.md §14.3의 경로를 함께 갱신)
`.uproject`의 EngineAssociation은 GUID `{CCB28703-480F-382D-F8F7-E6A5E7850219}`로 5.8을 가리킨다. `"5.8"` 문자열로 바꾸지 않는다: 버전 문자열은 `%ProgramData%\Epic\UnrealEngineLauncher\LauncherInstalled.dat`의 UE_5.8 항목으로 해석되는데 이 PC의 파일에 그 항목이 없다. GUID는 사용자별 레지스트리 `HKCU\Software\Epic Games\Unreal Engine\Builds`에 있으므로, 다른 PC/계정에서는 .uproject 우클릭 "Switch Unreal Engine version"으로 다시 연결해야 한다.

## 아키텍처 불변 규칙

`Source/Cosmic_Ascension/Sim`, `Events`, `Ascension`, `Log`, `Player`, `Save` (Core Logic Layer 전체)는 UUserWidget/AActor를 include하지 않는다 (§6.1).
모든 런타임 상태는 `FGalaxyState` 안의 USTRUCT. UObject 그래프로 상태를 만들지 않는다 (§6.2, §7.2).
난수는 `UGalaxySimulationSubsystem::GetRng()` 의 FRandomStream만. `FMath::Rand*` 금지 (§9.2).
틱 처리 순서: 경제 → 연구 → 외교 → 분쟁 → 위기 → AI → 플레이어 이동. 변경 시 guideline.md §7.4 먼저 수정.
소스 파일은 ASCII. `UMETA(DisplayName)` 영문. 한글은 DataTable/String Table (§10.3).
헤더는 모듈 루트 기준 경로로 include: `#include "Sim/GalaxySimulationSubsystem.h"`. (Build.cs가 `ModuleDirectory`를 PrivateIncludePaths에 추가해 둠. Public이 아닌 이유: 다른 모듈이 이 헤더를 쓰지 않고, UBT가 모듈 루트를 Public include로 두는 것을 허용하지 않음)
GameplayTag는 `Config/DefaultGameplayTags.ini`에 선언하고 코드에서 `FGameplayTag::RequestGameplayTag(FName)`로 얻는다. 태그는 `Domain.Sub.Leaf` 최대 3단계 (§10.4).
CommonUI 사용 금지 (UMG만, §6.3). Build.cs에 CommonUI를 추가하지 않는다.
`UE_LOG` 카테고리는 `LogCosmic` 하나 (`Cosmic_Ascension.h`, 개발자 로그). §8.1의 `Log.*` GameplayTag는 게임 내 서사 로그의 분류이며 별개다.

## 작업 방식

클래스 하나씩. 헤더 먼저 제시하고 승인 후 소스 (§14.1).
시뮬 코드는 `Tests/` 의 자동화 테스트와 함께 작성 (§9.3, §14.2).
UE 5.8은 모델 학습 데이터보다 새 버전이다. 낯선 엔진 API는 반드시 로컬 엔진 소스에서 선언 헤더를 확인한 뒤 사용한다: `C:\Program Files\Epic Games\UE_5.8\Engine\Source` (UnrealBuildTool C# 소스도 `Engine\Source\Programs\UnrealBuildTool`에 있음).
새 UCLASS/USTRUCT/UENUM 추가나 헤더 변경 = 에디터를 닫고 전체 빌드. Live Coding(에디터에서 Ctrl+Alt+F11)은 .cpp 본문 변경에만 쓴다.
커밋 단위는 "컴파일되는 상태". 헤더만 추가하고 커밋하지 않는다 (§10.5).

## 빌드 / 실행 / 테스트 (Windows, 5.8.3 소스로 검증한 명령)

아래 첫 블록은 cmd.exe 구문(`set`, `::`, `%VAR%`)이다. Claude Code의 셸은 Windows PowerShell 5.1이므로 실제 실행은 두 번째 PowerShell 블록을 쓴다.

```bat
set UE_ROOT=C:\Program Files\Epic Games\UE_5.8
set ROOT=C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension
set PROJECT=%ROOT%\Cosmic_Ascension.uproject

:: 에디터 타깃 빌드 (에디터가 닫혀 있어야 함). Build.bat은 번들 .NET으로 UnrealBuildTool.dll을 실행하고 인자를 그대로 넘긴다
"%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" Cosmic_AscensionEditor Win64 Development -Project="%PROJECT%" -WaitMutex

:: 게임 타깃 빌드 (에디터가 열려 있어도 가능. 컴파일 검증용)
"%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" Cosmic_Ascension Win64 Development -Project="%PROJECT%" -WaitMutex

:: Visual Studio 프로젝트 파일 재생성 (소스 파일 추가/삭제 후. 이 PC는 .uproject 우클릭 메뉴가 등록되어 있지 않음)
"%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" -projectfiles -project="%PROJECT%" -game -rocket -progress

:: VS Code 프로젝트 파일 (.vscode/, Cosmic_Ascension.code-workspace 생성)
"%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" -projectfiles -project="%PROJECT%" -game -rocket -progress -VSCode -Automated

:: L_Galaxy 맵 생성 (에디터 닫힌 상태, 에디터 타깃 빌드 후. Tools/make_galaxy_level.py 참고)
"%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PROJECT%" -run=pythonscript -Script="%ROOT%/Tools/make_galaxy_level.py" -unattended -nosplash -stdout -FullStdOutLogOutput -AbsLog="%ROOT%/Saved/Logs/MakeGalaxyLevel.log"

:: 헤드리스 실행 스모크 (맵 로드 후 첫 프레임에 quit. 로그에서 LogCosmic 줄 확인)
"%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PROJECT%" -game -nullrhi -nosound -unattended -nosplash -stdout -FullStdOutLogOutput -ExecCmds="quit" -AbsLog="%ROOT%/Saved/Logs/GameSmoke.log"

:: 자동화 테스트 (이름에 Cosmic_Ascension이 포함된 테스트. 실패 시 종료 코드 255 = 로그의 "EXIT CODE: -1"을 uint8로 변환한 값. 매칭 테스트 0개여도 255이므로 0이 아닌지로만 판정)
"%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PROJECT%" -game -nullrhi -nosound -unattended -nosplash -stdout -FullStdOutLogOutput -ExecCmds="Automation RunTests Cosmic_Ascension; Quit" -TestExit="Automation Test Queue Empty" -ReportExportPath="%ROOT%/Saved/Automation/Reports" -AbsLog="%ROOT%/Saved/Logs/AutomationRun.log"
```

```powershell
# PowerShell 5.1용. 변수는 반드시 $env: 로 정의한다. `--%` 뒤에서는 %VAR% 환경 변수만 치환되고 $VAR는 치환되지 않는다.
# Claude Code의 PowerShell 호출은 호출 간 상태가 유지되지 않으므로 $env: 정의와 명령을 같은 호출에 둔다.
# `--%` 뒤는 줄 끝까지 리터럴이라 그 뒤에 `;`로 다른 명령을 이어 쓸 수 없다. `set VAR=...`는 PowerShell에서 Set-Variable이라 실패한다.
$env:UE_ROOT='C:\Program Files\Epic Games\UE_5.8'; $env:ROOT='C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension'; $env:PROJECT="$env:ROOT\Cosmic_Ascension.uproject"

# 에디터 타깃 빌드 (에디터 닫힌 상태)
& "$env:UE_ROOT\Engine\Build\BatchFiles\Build.bat" --% Cosmic_AscensionEditor Win64 Development -Project="%PROJECT%" -WaitMutex
# 게임 타깃 빌드 (컴파일 검증)
& "$env:UE_ROOT\Engine\Build\BatchFiles\Build.bat" --% Cosmic_Ascension Win64 Development -Project="%PROJECT%" -WaitMutex
# VS 프로젝트 파일 재생성
& "$env:UE_ROOT\Engine\Build\BatchFiles\Build.bat" --% -projectfiles -project="%PROJECT%" -game -rocket -progress
# VS Code 프로젝트 파일
& "$env:UE_ROOT\Engine\Build\BatchFiles\Build.bat" --% -projectfiles -project="%PROJECT%" -game -rocket -progress -VSCode -Automated
# L_Galaxy 맵 생성
& "$env:UE_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" --% "%PROJECT%" -run=pythonscript -Script="%ROOT%/Tools/make_galaxy_level.py" -unattended -nosplash -stdout -FullStdOutLogOutput -AbsLog="%ROOT%/Saved/Logs/MakeGalaxyLevel.log"
# 헤드리스 스모크
& "$env:UE_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" --% "%PROJECT%" -game -nullrhi -nosound -unattended -nosplash -stdout -FullStdOutLogOutput -ExecCmds="quit" -AbsLog="%ROOT%/Saved/Logs/GameSmoke.log"
# 자동화 테스트
& "$env:UE_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" --% "%PROJECT%" -game -nullrhi -nosound -unattended -nosplash -stdout -FullStdOutLogOutput -ExecCmds="Automation RunTests Cosmic_Ascension; Quit" -TestExit="Automation Test Queue Empty" -ReportExportPath="%ROOT%/Saved/Automation/Reports" -AbsLog="%ROOT%/Saved/Logs/AutomationRun.log"
```

빌드 전 확인: `UnrealEditor.exe`가 이 프로젝트를 열고 있으면 에디터 타깃 빌드는 `Unable to build while Live Coding is active`로 실패한다 (UBT `HotReload.cs`, 실행 파일 기준 뮤텍스 검사). 그때는 게임 타깃으로 컴파일만 검증하고, 에디터를 닫은 뒤 에디터 타깃을 빌드한다.
UBT 로그: `%LOCALAPPDATA%\UnrealBuildTool\Log.txt` (프로젝트 파일 생성은 `Log_GPF.txt`). 프로젝트 로그: `Saved\Logs\Cosmic_Ascension.log`.
PowerShell에서는 `& "<exe>" --% <나머지 인자>` 형태로 호출한다. `--%`가 없으면 `-Script="..."` 같은 따옴표 인자가 깨진다. `&&`는 PowerShell 5.1에서 동작하지 않는다.
`-ExecCmds` 값은 쉼표로 분리되므로 안에 쉼표를 넣지 않는다.

**자동화 테스트 작성 규칙 (5.8)**
`IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClass, "Cosmic_Ascension.Sim.Determinism", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)`. `EAutomationTestFlags`는 enum class이고 마스크(`EAutomationTestFlags_ApplicationContextMask` 등)는 자유 상수다. Context 플래그 1개 이상 + Filter 플래그 정확히 1개가 static_assert로 강제된다. `-game`으로 돌리므로 `ClientContext`가 반드시 들어가야 한다. `-nullrhi`에서는 `NonNullRHI` 플래그 테스트가 제외된다.

## 모듈 / UHT / 컴파일 규칙 (5.8 UBT 소스 확인)

헤더 파일 이름은 **타깃 전체에서 유일**해야 한다(UHT가 모듈을 가로질러 검사). `Types.h`, `Utils.h` 같은 이름 금지. 항상 도메인 접두어(`CosmicTypes.h`).
`.generated.h`는 헤더의 마지막 include. `#pragma once` 필수.
include는 모듈 루트 기준(`"Sim/GalaxySimulationSubsystem.h"`). `"Cosmic_Ascension/Sim/..."`도 해석되지만 쓰지 않는다. 서브폴더는 자동 include 경로가 아니므로 다른 폴더의 헤더를 파일명만으로 include하지 않는다.
BuildSettingsVersion.V7 + IncludeOrder Unreal5_8: C++20, `UE_LOG` 포맷 문자열 컴파일 타임 검증, 미정의 식별자/반환 타입/dangling/unreachable 경고가 **에러**, 전이 include 의존 금지(쓰는 헤더는 직접 include).
Live Coding은 .cpp 변경에만. UCLASS/USTRUCT/UENUM 추가·변경은 에디터 종료 후 전체 빌드.

## 검증된 엔진 헤더 경로 (5.8.3 로컬 소스 확인)

(Phase 0 조사 결과를 아래 표로 유지한다. 새 API를 쓰면 행을 추가한다.)

| 심볼 | include | 모듈 | 비고 |
|---|---|---|---|
| `USubsystem`, `UGameInstanceSubsystem` | `Subsystems/Subsystem.h`, `Subsystems/GameInstanceSubsystem.h` | Engine | `Engine/Public/Subsystems`에 있음. `Classes/Subsystems`는 5.8에 없음 |
| `FSubsystemCollectionBase` (InitializeDependency 등) | `Subsystems/SubsystemCollection.h` | Engine | Subsystem.h는 전방 선언만. `Collection.InitializeDependency<T>()`를 쓰면 이 헤더 필요 |
| `UGameInstance::GetSubsystem<T>()` | `Engine/GameInstance.h` | Engine | 5.8 include order에서 전이 include에 의존하지 말 것 |
| `FTickableGameObject`, `ETickableTickType` | `Tickable.h` | Engine | `Stats/Stats.h`를 포함하므로 `RETURN_QUICK_DECLARE_CYCLE_STAT`, `STATGROUP_Tickables`도 함께 옴 |
| `USaveGame` | `GameFramework/SaveGame.h` | Engine | `ULocalPlayerSaveGame`(버전 필드 내장)도 같은 헤더 |
| `UGameplayStatics::SaveGameToSlot/LoadGameFromSlot/DoesSaveGameExist/DeleteGameInSlot` | `Kismet/GameplayStatics.h` | Engine | `(const FString& SlotName, const int32 UserIndex)`. 비동기 `AsyncSaveGameToSlot`는 네이티브 델리게이트(ThreeParams) |
| `FRandomStream` | `Math/RandomStream.h` | Core | 반드시 `Initialize(int32 Seed)`로 초기화. `Initialize(FName)`에 NAME_None을 주면 시간 기반 시드가 되어 결정론이 깨진다. `FRandRange`는 double(`FVector::FReal`) 반환, `GetFraction/FRand`는 float, `RandRange(int32,int32)`는 양끝 포함. 결과에 `%` 금지(하위 비트 품질 나쁨, 헤더 경고) |
| `FTableRowBase`, `UDataTable::FindRow<T>/GetAllRows<T>/ForeachRow<T>/GetRowNames` | `Engine/DataTable.h` | Engine | `FindRow<T>(RowName, TEXT("Ctx"))` |
| `DECLARE_LOG_CATEGORY_EXTERN` / `DEFINE_LOG_CATEGORY` | `Logging/LogMacros.h` (실제 정의는 `Logging/LogCategory.h`) | Core | `UE_LOG` 포맷은 반드시 `TEXT("...")` (static_assert). 구조화 로그 `UE_LOGFMT`는 `Logging/StructuredLog.h` |
| `IMPLEMENT_PRIMARY_GAME_MODULE` | `Modules/ModuleManager.h` | Core | 3번째 인자는 무시됨(UE_PROJECT_NAME 사용) |
| `UUserWidget` | `Blueprint/UserWidget.h` | UMG | C++ 파생 위젯(과 그 자식 WBP)은 기본적으로 매 프레임 NativeTick이 돈다(`DisableNativeTick` 메타는 UUserWidget 자신에만 적용). 틱이 필요 없는 베이스 클래스는 `meta=(DisableNativeTick)`를 다시 선언. UI 갱신은 Subsystem 델리게이트로 |
| `UListView`, `IUserObjectListEntry` | `Components/ListView.h`, `Blueprint/IUserObjectListEntry.h` | UMG | 항목은 UObject, 엔트리 위젯은 IUserObjectListEntry 구현 |
| `UNiagaraSystem` | `NiagaraSystem.h` | Niagara | 플러그인 `Classes/` 폴더라 하위 경로 없이 include |
| `UNiagaraFunctionLibrary::SpawnSystemAtLocation` | `NiagaraFunctionLibrary.h` | Niagara | |
| `FText::Format`, `FFormatNamedArguments`, `FText::FromStringTable` | `Internationalization/Text.h` | Core | `FFormatNamedArguments`는 `TSortedMap<FString, FFormatArgumentValue>` |
| `LOCTEXT` / `NSLOCTEXT` / `INVTEXT` | `Internationalization/Internationalization.h` | Core | 각 .cpp에서 `#define LOCTEXT_NAMESPACE` 후 사용, 끝에 `#undef` |
| `FStringTableRegistry`, `LOCTABLE(ID, KEY)` | `Internationalization/StringTableRegistry.h` | Core | `UStringTable` 에셋 클래스는 Engine 모듈 `Internationalization/StringTable.h` |
| `FGameplayTag`, `FGameplayTagContainer`, `FGameplayTag::RequestGameplayTag(const FName&, bool ErrorIfNotFound=true)` | `GameplayTagContainer.h` | GameplayTags | 엔진 런타임 모듈(플러그인 아님). `Classes/` 폴더라 하위 경로 없음 |
| `UE_DECLARE_GAMEPLAY_TAG_EXTERN` / `UE_DEFINE_GAMEPLAY_TAG_COMMENT` | `NativeGameplayTags.h` | GameplayTags | 참고용. 현재 규칙상 사용하지 않는다: 태그 선언은 `Config/DefaultGameplayTags.ini` 단일 출처(§10.1), 코드는 `RequestGameplayTag`만. 바꾸려면 guideline.md §10.1을 먼저 고친다 |
| `UCommandlet` | `Commandlets/Commandlet.h` | Engine | `-run=<이름>`은 `<이름>Commandlet` 클래스를 찾는다. 커맨드렛 프로세스에는 UGameInstance가 없으므로 Subsystem을 쓰려면 §9.3의 부트스트랩이 필요하다 |

**GameplayTags ini 규칙 (5.8 소스 확인)**
`Config/DefaultGameplayTags.ini`는 계층 레이어라서 배열 행은 반드시 `+GameplayTagList=(Tag="A.B",DevComment="...")`. `+` 없는 행은 배열을 통째로 덮어쓴다.
시스템별로 파일을 나누고 싶으면 `Config/Tags/<이름>.ini`에 섹션 `[/Script/GameplayTags.GameplayTagsList]`와 `+` 없는 `GameplayTagList=(...)` 행을 쓴다(자동 탐색). 거기서 `+`를 붙이면 키 이름의 일부로 취급되어 무시된다.
속성 이름은 `WarnOnInvalidTags`(b 접두어 없음). `ClearInvalidTags`는 5.5에서 deprecated → 쓰지 않는다. 태그 이름에 `"` `'` `,` 금지.

**Phase 1에서 틱 Subsystem을 만들 때 (5.8 확인 사항)**
`FTickableGameObject`를 UObject(Subsystem)와 함께 쓸 때는 헤더 주석대로: 생성자에서 `FTickableGameObject(ETickableTickType::Never)`, `Initialize()` 끝에서 `SetTickableTickType(ETickableTickType::Conditional)`, `Deinitialize()`에서 다시 `Never`. `IsAllowedToTick()`은 UE_DEPRECATED(5.5)이므로 오버라이드하지 않는다. 일시정지 중에도 돌려야 하면 `IsTickableWhenPaused()` 오버라이드.
`GetStatId()`는 `RETURN_QUICK_DECLARE_CYCLE_STAT(UGalaxySimulationSubsystem, STATGROUP_Tickables);`.
밸런스 커맨드렛(§9.3)은 UEditorEngine 아래에서 돌고 UGameInstance가 없다. `Main()`에서 `NewObject<UGameInstance>(GEngine, Class)` → `AddToRoot()` → `InitializeStandalone()`으로 GameInstance를 직접 만들어 Subsystem을 얻는다. 구현 전에 `Engine/GameInstance.h`의 `InitializeStandalone` 시그니처를 확인한다.

## 렌더 / 플러그인 결정 (2026-10-10, Config/DefaultEngine.ini)

레이트레이싱 OFF + SkinCache OFF (한 묶음. RT만 켜고 SkinCache를 끄면 시작 시 Fatal). Lumen GI/반사 OFF, 메시 디스턴스 필드 OFF (읽기 전용 cvar, 메시 0개인 지금 결정). Substrate ON 유지 (5.8 신규 프로젝트 기본값. 나중에 끄면 에디터 재시작이 필요하고 Substrate 전용 노드로 만든 머티리얼이 깨지므로 머티리얼 작성 전인 지금 확정). 정적 라이팅 OFF, 가상 섀도맵 ON, DX12 SM6.
플러그인: ModelingToolsEditorMode 비활성, Niagara 명시 활성, CommonUI 비활성(기본값). PythonScriptPlugin은 헤드리스 에디터 스크립트용으로 활성. .uproject의 `TargetAllowList: ["Editor"]`는 Niagara.uplugin이 PythonScriptPlugin을 타깃 제한 없이 의존하기 때문에 게임 타깃에서 효력이 없다(게임 exe에는 `WITH_PYTHON=0` 스텁 모듈이 링크됨, 무해). 그래도 Niagara를 끌 경우를 위해 블록은 유지하고, DisablePlugins 등으로 게임 타깃에서 강제 제외하려 하지 않는다. 실제 Python 실행은 에디터 바이너리(UnrealEditor, UnrealEditor-Cmd)에서만 된다.
5.8 프로젝트 브라우저에는 레이트레이싱/스타터 콘텐츠 토글이 없고 RT·Lumen·Substrate를 무조건 켜서 쓴다. 끄는 방법은 ini 편집만이다.

## 현재 단계

Phase 0 (셋업) 종료 조건 달성 (2026-10-10): 에디터/게임 타깃 커맨드라인 빌드 성공, `Content/CA/Maps/L_Galaxy.umap` 생성(헤드리스 Python), `GameDefaultMap`/`EditorStartupMap`을 L_Galaxy로 지정, `-game -nullrhi` 스모크 실행에서 L_Galaxy 로드 + Subsystem 6개의 `LogCosmic` 초기화 로그 확인(`Saved/Logs/GameSmoke.log`, 에러 0).
남음: Git 초기화와 첫 커밋 (Git/Git LFS 설치 필요. `.gitignore`/`.gitattributes`는 준비됨). 그 다음은 Phase 1 (§11): `Core/CosmicTypes.h`, `Core/GalaxyState.h`부터 §14.4의 요청 예시 순서로.
참고: §10.1의 빈 폴더(Core, Data, World, UI, Tests, Content/CA/*)는 Git에 올라가지 않는다. 첫 파일이 생길 때 자연히 생성되므로 `.gitkeep`을 넣지 않는다.
