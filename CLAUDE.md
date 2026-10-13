# Cosmic Ascension — Claude Code 작업 규칙

## 프로젝트

UE 5.8.3 (`C:\Program Files\Epic Games\UE_5.8`), C++ 게임 모듈 `Cosmic_Ascension`. 설계 기준 문서: `guideline.md` (섹션 번호로 참조. 결정이 바뀌면 guideline.md를 먼저 고친다)
1인 개발. 린 시뮬레이션. 3D 에셋·캐릭터 모델 참조 금지. 데이터는 DataTable(FTableRowBase), 식별자 FName, 분류 FGameplayTag, 표시 문자열 FText.
프로젝트 루트: `C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension` (OneDrive 안. 프로젝트를 옮기면 이 파일과 guideline.md §16 #8의 경로를 함께 갱신)
`.uproject`의 EngineAssociation은 GUID `{CCB28703-480F-382D-F8F7-E6A5E7850219}`로 5.8을 가리킨다. `"5.8"` 문자열로 바꾸지 않는다: 버전 문자열은 `%ProgramData%\Epic\UnrealEngineLauncher\LauncherInstalled.dat`의 UE_5.8 항목으로 해석되는데 이 PC의 파일에 그 항목이 없다. GUID는 사용자별 레지스트리 `HKCU\Software\Epic Games\Unreal Engine\Builds`에 있으므로, 다른 PC/계정에서는 .uproject 우클릭 "Switch Unreal Engine version"으로 다시 연결해야 한다.

## 아키텍처 불변 규칙

`Source/Cosmic_Ascension/Sim`, `Events`, `Ascension`, `Log`, `Player`, `Save` (Core Logic Layer 전체)는 UUserWidget/AActor를 include하지 않는다 (§6.1).
모든 런타임 상태는 `FGalaxyState` 안의 USTRUCT. UObject 그래프로 상태를 만들지 않는다 (§6.2, §7.2). `FGalaxyState`는 판의 상태다(고향 은하 상세 + 판 공통) — 코드 식별자의 Galaxy는 판을 가리키고 개명하지 않는다. 상태는 세 층이다: 세계 → 앎(`Knowing`) → 이야기(`Log`·`Chronicle`·`TellRngState`). 세계 코드는 앎을 `NoteMet`·`NoteKnown`으로 쓰기만 하고 읽지 않는다(그 밖에 앎에 닿는 곳은 이것뿐이다: `Install`이 출생 성계·출생 문명을 줄 없이 적고(첫 접촉의 기록 `FirstContactCivID`는 `NoteMet`만 적는다 — `Install`은 적지 않는다), 건넘 단계의 정리·역할 전환·플레이어의 답이 `PendingContactCivID`를 비우고, `Push`가 `FarLineUntilTick`을 적는다 — 어느 것도 세계 결과가 없다). 앎을 읽는 조건·효과는 없다 (§8.1). (v0.7 — `NoteMet`·`NoteKnown`, `Install`의 앎 기록, 이야기의 범위(먼 줄의 `FarLineUntilTick` 포함)는 Phase 2부터. `PendingContactCivID`를 비우는 줄은 건넘 단계의 정리 쪽이 Phase 2, 역할 전환·플레이어의 답 쪽이 첫 접촉 카드가 오는 Phase 4다)
난수의 출처는 셋이다: 시뮬 스트림 `UGalaxySimulationSubsystem::GetRng()`(세계 상태를 바꾸는 모든 추첨), 말하기 스트림 `DrawTell()`(`TellRngState` — `UCosmicLogSubsystem::PushVariants`의 문장 변형만. 부르는 함수는 그것 하나다), 파생 정수 `CosmicSeed::Derive`(`Core/CosmicSeed.h`, 상태 없음, 식·상수 동결). 서로 읽지 않는다. 추첨 지점은 무조건 뽑는다: 이벤트 슬롯은 슬롯당 정확히 1회를 행을 보기 전에 먼저 뽑고, `NameNotable`과 `Install`의 이름 추첨은 가드·뱅크보다 먼저 1회다 — 행 수·쿨다운·인물 이름 뱅크가 시뮬 난수열을 밀지 않는다. `FMath::Rand*` 금지 (§9.2). (v0.7 — `NewGalaxy`의 추첨 수는 아직 `DT_Civs`의 내용을 따른다: 가중 행이 있을 때만 아키타입, `arch_default`일 때만 윤리, `AIProfile`이 빈 행일 때만 프로필, 뱅크가 찼을 때만 모성계 이름·문명 이름. Phase 2 은하 생성 규칙 변경의 헤더 리뷰에서 §9.2 ⓐ에 맞추는 범위를 정한다 — 그때까지 `DT_Civs`의 `NameBank`·`HomeSystemNames`·`AIProfile`·`Weight`를 고치면 같은 시드의 은하가 달라진다)
틱 처리 순서(열 단계): 경제 → 연구 → 외교 → 분쟁 → 위기 → AI → 건넘 → 플레이어 이동 → 앎 → 역할 효과 해소. 변경 시 guideline.md §7.4 먼저 수정. 이벤트·업적·영압(영적 압력)은 제 단계가 없다: 이벤트 슬롯(`UCosmicEventSubsystem::EvaluateAndFire`)은 위기 단계의 끝, 업적 판정은 AI 단계의 끝, 틱당 영압 효과는 경제 단계의 수입과 비축 사이다(업적 판정은 Phase 2의 승천 판정 작업부터, 틱당 영압 효과는 영적 압력 작업부터). 발화한 행의 효과는 큐 없이 제자리에서 적용하고 행은 '효과 전부 → 줄 → `OnEventFired`'의 원자다. `Check`·`Apply`는 static이라 세계만 읽고 쓴다(줄·앎·방송에 닿지 못한다). `Civs`의 원소 수는 건넘 단계에서만 바뀌고 `Systems`는 줄지 않는다(파괴된 성계는 죽은 노드). (v0.7 — 건넘 단계의 정리와 앎 단계의 본문은 Phase 2부터)
소스 파일은 ASCII. `UMETA(DisplayName)` 영문. 한글은 DataTable/String Table (§10.3).
헤더는 모듈 루트 기준 경로로 include: `#include "Sim/GalaxySimulationSubsystem.h"`. (Build.cs가 `ModuleDirectory`를 PrivateIncludePaths에 추가해 둠. Public이 아닌 이유: 다른 모듈이 이 헤더를 쓰지 않고, UBT가 모듈 루트를 Public include로 두는 것을 허용하지 않음)
GameplayTag는 `Config/DefaultGameplayTags.ini`에 선언하고 코드에서 `FGameplayTag::RequestGameplayTag(FName)`로 얻는다. 태그는 `Domain.Sub.Leaf` 최대 3단계 (§10.4).
CommonUI 사용 금지 (UMG만, §6.3). Build.cs에 CommonUI를 추가하지 않는다.
`UE_LOG` 카테고리는 `LogCosmic` 하나 (`Cosmic_Ascension.h`, 개발자 로그). §8.1의 `Log.*` GameplayTag는 게임 내 서사 로그의 분류이며 별개다.
하나님(거룩한 질서)은 엔티티·화자·수치·엔딩 조건이 아니다 (§4.5). 1인칭 로그 화자는 타락 초월자만.
Core Logic Subsystem은 플레이어·생애에 종속된 멤버 변수를 갖지 않는다. 지속 업적 진척은 FGalaxyState 안에.
`FCivilizationState.Relations`의 키는 CivID 또는 TranscendentID. 항목은 처음 쓸 때 생기고 없는 키 = 0 = 중립이다 (§5.5). `Check(CivRelationAtLeast)`와 `Apply(ChangeRelation)`는 이 뜻으로 읽고 쓰며, 상대는 살아 있는 문명이나 판에 있는 초월자 인스턴스일 때만 풀린다 (§7.5). (v0.7 — 외교 단계와 `InitialRelations`는 Phase 4부터, 제거된 CivID 키의 정리는 Phase 2의 건넘 단계 정리부터, 인스턴스 없는 천상 행의 키는 영적 압력 작업부터)
`DeposeRuler`·`KillPlayer`는 위기 단계·분쟁 단계(통치 문명의 마지막 성계를 앗아 갈 때)·이벤트 효과·조우 해결에서 `UCosmicEventSubsystem::Apply`를 거쳐, Embodied일 때만 `PendingRoleEffect`로 기록되고(먼저 기록된 것 우선. 출생 보호는 적는 자리에서 가린다. 성계의 위기는 그 성계에 선 방랑자에게만 닿고 통치자는 통치 문명의 절멸로만 죽는다) 틱 경계의 '역할 효과 해소' 단계에서 처리된다(§7.4). `PendingEffect`에 담는 값은 ini에 선언한 태그 `Effect.KillPlayer`·`Effect.DeposeRuler`이고 빈 태그가 "없음"이다(§7.2). (v0.7 — 분쟁 단계의 기록은 그 단계가 본문을 얻는 Phase 4부터)
영혼·구원·멸망을 세는 필드·컬럼·업적은 없다. 영혼 경제의 유일한 크기는 초월자 영향권(성계 수)이다 (§4.2, §8.3).
`Realm`은 상태가 아니라 정의다: 초월자 소속은 `FTranscendentDef.Allegiance`(`Realm.Abyssal/Celestial/Independent`), `FTranscendentState`에 Realm 필드 없음. `TranscendentID` == `DT_Transcendents` 행 이름 (§4.2, §7.3). 초월자는 판에 교리마다 1체다(은하마다 따로 두지 않는다. 타락은 건너지 않는다) (§4.2).
심도는 상태가 아니라 정의다: `FTranscendentDef.Depth`(0~1) 컬럼 하나이고 영압의 초월자 항에만 (1 + Depth)로 곱한다. 문명·플레이어의 심도는 저장하지 않는다(파생·서술만). 칸(`EAscensionTier`)은 다섯 그대로다 — 칸을 대신하는 등급 타입, 차원 축·정렬 같은 수치, 현현의 열거형을 만들지 않는다 (§3.2 심도, §4.2 현현). (v0.7 — 코드는 Phase 2 영적 압력 작업부터)
서술자 카테고리 로그 템플릿에 판정 어휘(천국·지옥·구원받·멸망받·영혼·저주받·심판받) 금지 — 테스트 `Data.NoVerdictWords`. 1인칭(`Log.Transcendent`·`Log.Claim`)과 조우 인용문만 예외 (§8.3).
인물은 `FCivilizationState.Notables`의 기록이다(`FNotableRecord` 5필드 고정: Role·NameIndex·FirstTick·LastTick·SourceRow). 능력치·AI·관계 없음, 개인을 겨누는(ID를 받는) 조건/효과 없음(문명을 대상으로 기록 1개를 만드는 NameNotable과 역할 유무만 읽는 CivHasNotable이 전부). 능력치를 가진 개인은 그릇 하나(`FPlayerEntityState`) (§1.3, §7.2, §12.2).
사람의 수는 UI·로그·연대기에 숫자로 쓰지 않는다(`GetPopulationBand` 등급만). 숫자는 서술자 로그·연대기에서는 주기·비율만 쓰고, 자원·Stability 숫자는 UI에만 쓴다 (§1.3, §8.3). 천체의 수와 거리(별의 수·지름·광년)는 은하 패널·우주 화면의 UI 라벨과 게임 밖 '세계관과 출처' 화면에만 `DT_Galaxies`의 FText 그대로 쓰고(언제나 '약'·'추정') 서술자 로그·연대기·두 책·조우 문장에는 쓰지 않는다. 생명 성계·백성·아는 것의 수는 UI에도 숫자로 쓰지 않는다 (§8.3). (v0.7 — 코드는 Phase 2·3부터)
저장되는 필드의 기본 초기값(멤버 초기화식)은 세이브가 생긴 뒤 바꾸지 않는다(의미 변경 = 마이그레이션): 저장이 클래스 기본 객체 대비 델타라서 초기값과 같은 값은 기록되지 않는다. `FGalaxyState.SaveVersion`의 초기값은 리터럴 1 고정이고 현재 버전은 `FGalaxyState::CurrentSaveVersion`을 `NewGalaxy`와 마이그레이션이 찍는다 (§9.1).
`ComputeStateHash`는 FName·FGameplayTag를 소문자 문자열로 해시한다(`GetTypeHash`·이름 인덱스·구조체 원시 메모리 금지 — 프로세스마다 다르다). `TMap`은 `FNameLexicalLess`로 정렬한 키 순서로만 순회·해시한다. FText와 `GalaxyID`는 해시 밖이다. `ComputeWorldHash`는 거기서 `Knowing`·`Log`·`Chronicle`·`TellRngState`를 뺀 것이다 — 앎이 세계를 바꾸지 않음을 재는 자이고 쿠킹 빌드 비교도 이것으로 한다. 시뮬이 읽는 FText는 없다 (§9.2). (v0.7 — `CivName`이 빈 문명의 돌파 로그를 막는 게이트는 Phase 2 은하 생성 규칙 변경에서 지운다 — 그때까지 코드는 `CivName`이 비었는지를 읽는다)
데이터 컬럼의 Tier는 번호 1~5이고 `EAscensionTier` 값은 0~4다(`Tier1_Planetary == 0`). 비교는 `static_cast<int32>(Tier) + 1`, Tier로 인덱싱하는 배열은 `static_cast<int32>(Tier)` (§7.3).
정의 데이터는 `FCosmicDataTables`(`Data/CosmicDataTables.h`)가 고정 경로 `/Game/CA/Data/DT_*`에서 로드한다(설정 에셋 없음). 코드가 이름으로 찾는 행은 `Data/CosmicRowNames.h`의 `CosmicRows::` 상수로만 부른다. 에셋의 원본은 `Tools/data/*.json`·`ST_CA.csv`이고 `.uasset`은 `Tools/make_data_tables.py`의 산출물이다: 데이터는 JSON/CSV를 고치고 스크립트를 다시 돌린다(에디터에서 고친 행은 덮어써진다). 행 구조체를 바꾸면 JSON에 새 컬럼을 적고 다시 돌린다(JSON에 없는 컬럼은 스크립트가 실패로 알린다 — 행 바로 아래 컬럼만이다. 중첩 구조체에서 빠진 멤버는 그 구조체의 멤버가 모두 스칼라일 때만 잡히므로, 배열·구조체 멤버를 가진 중첩 구조체가 처음 나오는 `DT_Encounters`(`FEncounterChoice`)를 더하기 전에 스크립트의 검사를 중첩 구조체까지 넓힌다). 스크립트는 원본의 해시를 `Tools/data/generated.sha1`에 남기고, 테스트 `Data.SourcesInSync`가 원본만 고치고 에셋을 다시 만들지 않은 상태를 잡는다. 행 이름은 세이브에 남으므로(`SourceRow`·`ArchetypeRow`·`BirthArchetypeRow`·`GalaxyRow`) 배포 뒤에는 바꾸지 않는다 (§7.3, §7.4, §9.3, §10.1, §10.4). 정의 데이터의 참조·어휘 규칙은 `Tools/data/schema.json`에 적고 `make_data_tables.py`가 에셋을 만들 때 검사한다(`Data.Integrity`). 행 이름은 그 테이블의 접두어를 갖고(`DT_Traits`·`DT_Relics`·`DT_Dedications`의 접두어는 첫 행을 쓸 때 `schema.json`에 정한다) 테이블을 가로질러 유일하다 (§7.3, §10.4). `DT_Events`는 아직 에셋이 없다(첫 행을 쓸 때 로더·스크립트·JSON이 함께 온다 — 그때까지 테스트가 `SetTables`로 주입한다). `FGalaxyState.EventCooldownUntilTick`의 키는 `"<행>@<CivID>"`(`UCosmicEventSubsystem::CooldownKey`)이고 저장되므로 꼴을 바꾸지 않는다 — 행 이름에 '@'를 쓰지 않는다 (§7.2, §7.5). (v0.7 — 스키마와 `Data.Integrity`는 Phase 2의 스키마 검사기 작업부터)
DataTable 텍스트의 로컬라이제이션 키는 위치로 정해진다(행·컬럼·배열 인덱스). 번역을 시작한 뒤에는 `Templates`·`NameBank` 같은 배열은 끝에만 추가하고 중간 삽입·순서 변경·삭제를 하지 않는다. 같은 이유로 `DT_` 에셋을 지우고 다시 만들지 않는다(새 패키지는 네임스페이스가 바뀐다 — 스크립트는 기존 에셋을 제자리에서 다시 채운다) (§8.5).
서사 로그는 `UCosmicLogSubsystem::Push`·`PushVariants`만 `State.Log`에 쓴다(`PushRow`는 `PushVariants`의 포장이고 이벤트 행의 줄도 같은 길로 나간다). `Message`는 렌더 결과를 평탄화해 저장한다(`FText::AsCultureInvariant`). `PushVariants`는 호출당 말하기 스트림 추첨이 정확히 1회다: 직전 줄과 같은 문장이 나오면 다음 변형으로 넘기되 다시 뽑지 않는다(범위 밖 Severity 2를 대신하는 먼 줄을 낼 때만 2회). 줄이 쌓이는가는 이야기의 범위가 정한다: 범위 안은 쌓고, 범위 밖의 Severity 2·`Log.Claim`·`Log.Record`는 `bUntold`로 쌓고, 그 밖은 쌓지 않는다. 추첨 수·반환값·쿨다운 키는 줄이 말해졌는지에 좌우되지 않는다. `OnLogEntry`는 틱 단계 도중에도 동기로 발화하고 `bUntold` 항목에는 발화하지 않는다: 핸들러는 상태를 읽고 `SetTimeScale`을 부를 수 있지만 `NewGalaxy`·`LoadGalaxy`·`SaveCurrentGalaxy`·`StepOnce`·`RunHeadless`와 평가기(`EvaluateAndFire`·`FireRow`·`Apply`)는 부르지 않는다 — `OnEventFired`의 핸들러도 같다(상태를 갈아 끼우거나 틱을 돌리는 일은 틱 경계인 `OnSimTick`이나 입력에서) (§7.5, §8.1, §8.4, §9.4). (v0.7 — 이야기의 범위는 Phase 2부터)
게임 모듈은 모든 타깃에서 부동소수점 정밀 모드다(`Build.cs`의 `FPSemantics = FPSemanticsMode.Precise`. V7 기본값은 에디터 타깃만 정밀, 게임 타깃은 `/fp:fast`). 지우면 게임 바이너리와 테스트·커맨드렛의 해시가 갈릴 수 있다. 같은 이유로 해시에 들어가는 부동소수점 값은 이 모듈 안에서 컴파일되는 코드로만 계산한다: 엔진 모듈의 비인라인 함수는 타깃의 FP 모드를 따른다(예: `FLinearColor::MakeFromHSV8` 대신 시뮬 cpp의 `CivWheelColor`). 해시에 드는 값은 + − × ÷ √만 쓴다 — 삼각 함수를 비롯한 C 런타임 수학 함수(`FMath::Sin`·`Cos`·`Pow`·`Exp` 따위가 부르는 것)를 쓰지 않는다 (§9.2). (v0.7 — 성계 배치의 `FMath::Cos`·`FMath::Sin`은 Phase 2 은하 생성 규칙 변경에서 지운다. 그때까지 코드에 남아 있다)
시뮬의 일시정지는 `SetTimeScale(0)` 하나뿐이다: Subsystem은 엔진의 월드 없는 틱 패스(`TickObjects(nullptr, …)`)에서 돌아서 `SetGamePaused`와 시간 팽창이 닿지 않는다 (§7.4).
UI 라벨은 `FText::FromStringTable(…, EStringTableLoadingPolicy::FindOrFullyLoad)`로 읽는다. 기본 정책은 다른 비동기 로딩 중에 테이블을 비동기로 올려서, 그 사이에 만든 줄에 `<MISSING STRING TABLE ENTRY>`가 남는다 (§8.5).
은하: 상세로 시뮬되는 은하는 고향 은하(`DT_Rules.HomeGalaxyRow`, `gal_milkyway`) 하나다. 성계·문명의 `GalaxyRow`는 `NewGalaxy`가 찍는 `DT_Galaxies` 행 이름이고 NAME_None은 '아직 찍히지 않음'이다(고향으로 읽지 않는다. NAME_None이 뜻을 갖는 곳은 `OriginGalaxyRow` = 토박이뿐). 레인을 세는 모든 규칙은 같은 `GalaxyRow` 안이다 — 다리(`FGalaxyDef.BridgeTicks`)는 레인이 아니다 (§5.9, §7.2). (v0.7 — `DT_Galaxies`·`FGalaxyDef`·노브 `HomeGalaxyRow`와 도장은 Phase 2 은하 생성 규칙 변경부터)
ID: 성계는 `sol`·`sys_<n>`, 문명은 `civ_<n>`(패딩 없음, `FGalaxyState.NextCivSerial`, 재사용 없음). 'ID 사전순'은 언제나 `FNameLexicalLess`다 (§7.2). (v0.7 — 코드는 Phase 2 은하 생성 규칙 변경부터. 그때까지 `sys_%03d`·`civ_%02d`)
정착된 성계 = 소유이고 `ActiveMegaproject != project_settle`. 수입·`Law.Remnant`의 셈·Hazard 추첨·계승 후보·정착의 인접 판정이 이 술어를 쓴다. 무주 성계에는 Hazard와 주장이 서지 않는다 (§5.7). (v0.7 — 코드는 Phase 2부터)
낱말: '조우'는 조우 카드만. 백성을 처음 알아보는 일은 '만남', 그 판의 첫 만남은 '첫 접촉'. 사다리는 '칸'. 다리는 '문·관문·길'이 아니고, 건넘의 수단은 '방주'가 아니라 '건너는 배들'이다 (§15).

## 작업 방식

진행 방식(Phase별 effort·ultracode 사용 기준, 세션 시작 절차, 다음 작업)은 `WORKPLAN.md`(작업지시서)를 따른다. 프로젝트 기본 모델·effort는 `.claude/settings.json`(Opus 5.5, high).
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
:: -Script= 값은 Python 문자열로 읽힌다. 역슬래시 경로를 주면 \r, \U 따위가 이스케이프로 해석되어 실패하므로 슬래시 경로를 따로 둔다
set ROOTF=C:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension

:: 에디터 타깃 빌드 (에디터가 닫혀 있어야 함). Build.bat은 번들 .NET으로 UnrealBuildTool.dll을 실행하고 인자를 그대로 넘긴다
"%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" Cosmic_AscensionEditor Win64 Development -Project="%PROJECT%" -WaitMutex

:: 게임 타깃 빌드 (에디터가 열려 있어도 가능. 컴파일 검증용)
"%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" Cosmic_Ascension Win64 Development -Project="%PROJECT%" -WaitMutex

:: Visual Studio 프로젝트 파일 재생성 (소스 파일 추가/삭제 후. 이 PC는 .uproject 우클릭 메뉴가 등록되어 있지 않음)
"%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" -projectfiles -project="%PROJECT%" -game -rocket -progress

:: VS Code 프로젝트 파일 (.vscode/, Cosmic_Ascension.code-workspace 생성)
"%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" -projectfiles -project="%PROJECT%" -game -rocket -progress -VSCode -Automated

:: L_Galaxy 맵 생성 (에디터 닫힌 상태, 에디터 타깃 빌드 후. Tools/make_galaxy_level.py 참고)
"%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PROJECT%" -run=pythonscript -Script="%ROOTF%/Tools/make_galaxy_level.py" -unattended -nosplash -stdout -FullStdOutLogOutput -AbsLog="%ROOTF%/Saved/Logs/MakeGalaxyLevel.log"

:: DataTable·String Table 에셋 생성 (에디터 닫힌 상태, 에디터 타깃 빌드 후. Tools/data/*.json·ST_CA.csv -> Content/CA/Data. 로그 끝의 "written and verified" 줄 확인)
"%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PROJECT%" -run=pythonscript -Script="%ROOTF%/Tools/make_data_tables.py" -unattended -nosplash -stdout -FullStdOutLogOutput -AbsLog="%ROOTF%/Saved/Logs/MakeDataTables.log"

:: 밸런스 커맨드렛 (Saved/Balance/<Out>에 CSV, 100틱마다 문명별 1행. 로그의 "CosmicBalance: seed ... hash ... world ..." 줄 확인. world가 세계 해시)
"%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PROJECT%" -run=CosmicBalance -Seed=42 -Ticks=20000 -Systems=60 -Civs=8 -Out=balance.csv -unattended -nosplash -stdout -FullStdOutLogOutput -AbsLog="%ROOTF%/Saved/Logs/Balance.log"

:: 헤드리스 실행 스모크 (맵 로드 후 첫 프레임에 quit. 로그에서 LogCosmic 줄 확인)
"%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PROJECT%" -game -nullrhi -nosound -unattended -nosplash -stdout -FullStdOutLogOutput -ExecCmds="quit" -AbsLog="%ROOT%/Saved/Logs/GameSmoke.log"

:: 자동화 테스트 (이름에 Cosmic_Ascension이 포함된 테스트. 실패 시 종료 코드 255 = 로그의 "EXIT CODE: -1"을 uint8로 변환한 값. 매칭 테스트 0개여도 255이므로 0이 아닌지로만 판정)
"%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%PROJECT%" -game -nullrhi -nosound -unattended -nosplash -stdout -FullStdOutLogOutput -ExecCmds="Automation RunTests Cosmic_Ascension; Quit" -TestExit="Automation Test Queue Empty" -ReportExportPath="%ROOT%/Saved/Automation/Reports" -AbsLog="%ROOT%/Saved/Logs/AutomationRun.log"
```

```powershell
# PowerShell 5.1용. 변수는 반드시 $env: 로 정의한다. `--%` 뒤에서는 %VAR% 환경 변수만 치환되고 $VAR는 치환되지 않는다.
# Claude Code의 PowerShell 호출은 호출 간 상태가 유지되지 않으므로 $env: 정의와 명령을 같은 호출에 둔다.
# `--%` 뒤는 줄 끝까지 리터럴이라 그 뒤에 `;`로 다른 명령을 이어 쓸 수 없다. `set VAR=...`는 PowerShell에서 Set-Variable이라 실패한다.
$env:UE_ROOT='C:\Program Files\Epic Games\UE_5.8'; $env:ROOT='C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension'; $env:PROJECT="$env:ROOT\Cosmic_Ascension.uproject"; $env:ROOTF='C:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension'

# 에디터 타깃 빌드 (에디터 닫힌 상태)
& "$env:UE_ROOT\Engine\Build\BatchFiles\Build.bat" --% Cosmic_AscensionEditor Win64 Development -Project="%PROJECT%" -WaitMutex
# 게임 타깃 빌드 (컴파일 검증)
& "$env:UE_ROOT\Engine\Build\BatchFiles\Build.bat" --% Cosmic_Ascension Win64 Development -Project="%PROJECT%" -WaitMutex
# VS 프로젝트 파일 재생성
& "$env:UE_ROOT\Engine\Build\BatchFiles\Build.bat" --% -projectfiles -project="%PROJECT%" -game -rocket -progress
# VS Code 프로젝트 파일
& "$env:UE_ROOT\Engine\Build\BatchFiles\Build.bat" --% -projectfiles -project="%PROJECT%" -game -rocket -progress -VSCode -Automated
# L_Galaxy 맵 생성
& "$env:UE_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" --% "%PROJECT%" -run=pythonscript -Script="%ROOTF%/Tools/make_galaxy_level.py" -unattended -nosplash -stdout -FullStdOutLogOutput -AbsLog="%ROOTF%/Saved/Logs/MakeGalaxyLevel.log"
# DataTable·String Table 에셋 생성 (Tools/data -> Content/CA/Data)
& "$env:UE_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" --% "%PROJECT%" -run=pythonscript -Script="%ROOTF%/Tools/make_data_tables.py" -unattended -nosplash -stdout -FullStdOutLogOutput -AbsLog="%ROOTF%/Saved/Logs/MakeDataTables.log"
# 밸런스 커맨드렛
& "$env:UE_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" --% "%PROJECT%" -run=CosmicBalance -Seed=42 -Ticks=20000 -Systems=60 -Civs=8 -Out=balance.csv -unattended -nosplash -stdout -FullStdOutLogOutput -AbsLog="%ROOTF%/Saved/Logs/Balance.log"
# 헤드리스 스모크
& "$env:UE_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" --% "%PROJECT%" -game -nullrhi -nosound -unattended -nosplash -stdout -FullStdOutLogOutput -ExecCmds="quit" -AbsLog="%ROOT%/Saved/Logs/GameSmoke.log"
# 자동화 테스트
& "$env:UE_ROOT\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" --% "%PROJECT%" -game -nullrhi -nosound -unattended -nosplash -stdout -FullStdOutLogOutput -ExecCmds="Automation RunTests Cosmic_Ascension; Quit" -TestExit="Automation Test Queue Empty" -ReportExportPath="%ROOT%/Saved/Automation/Reports" -AbsLog="%ROOT%/Saved/Logs/AutomationRun.log"
```

빌드 전 확인: `UnrealEditor.exe`가 이 프로젝트를 열고 있으면 에디터 타깃 빌드는 `Unable to build while Live Coding is active`로 실패한다 (UBT `HotReload.cs`, 실행 파일 기준 뮤텍스 검사). 그때는 게임 타깃으로 컴파일만 검증하고, 에디터를 닫은 뒤 에디터 타깃을 빌드한다.
UBT 로그: `%LOCALAPPDATA%\UnrealBuildTool\Log.txt` (프로젝트 파일 생성은 `Log_GPF.txt`). 프로젝트 로그: `Saved\Logs\Cosmic_Ascension.log`.
PowerShell에서는 `& "<exe>" --% <나머지 인자>` 형태로 호출한다. `--%`가 없으면 `-Script="..."` 같은 따옴표 인자가 깨진다. `&&`는 PowerShell 5.1에서 동작하지 않는다.
`-ExecCmds` 값은 쉼표로 분리되므로 안에 쉼표를 넣지 않는다.
`--%` 없이 부르면 PowerShell이 `-Out=balance.csv` 같은 인자를 `-Out=balance`와 `.csv`로 쪼갠다.
자동화 테스트는 **빌드 뒤 두 번 연속** 돌린다. `Sim.SaveRestart`는 첫 프로세스가 저장하고 해시를 `Saved/Automation/SaveRestart.txt`에 적고, 다음 프로세스가 로드해 검증한다(바이너리나 `Content/CA/Data`의 DT 에셋이 바뀌면 기록을 새로 쓴다). 두 번째 실행 로그에 `Restart half verified`가 있어야 재시작 절반까지 검증된 것이다.
스모크에서 서사 로그 줄을 보려면 `-LogCmds="LogCosmic Verbose"`를 붙인다(`[0] 태양계. …` 줄이 `GameSmoke.log`에 찍힌다). 새 은하의 시드는 `-Seed=<n>`, 없으면 시계다.
화면 확인: `-nullrhi` 실행과 테스트는 패널이 실제로 그려지는지 보지 못한다. 창 없이 한 장 찍으려면 `UnrealEditor.exe "<uproject>" -game -RenderOffScreen -windowed -ResX=1280 -ResY=720 -nosound -unattended -nosplash -Seed=42 -ExecCmds="shot showui"`를 띄우고 `Saved/Screenshots/WindowsEditor/ScreenShot*.png`가 생기면 프로세스를 끝낸다(스스로 끝나지 않는다. 첫 프레임 직후의 화면이다).

**자동화 테스트 작성 규칙 (5.8)**
`IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClass, "Cosmic_Ascension.Sim.Determinism", EAutomationTestFlags::ClientContext | EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)`. `EAutomationTestFlags`는 enum class이고 마스크(`EAutomationTestFlags_ApplicationContextMask` 등)는 자유 상수다. Context 플래그 1개 이상 + Filter 플래그 정확히 1개가 static_assert로 강제된다. `-game`으로 돌리므로 `ClientContext`가 반드시 들어가야 한다. `-nullrhi`에서는 `NonNullRHI` 플래그 테스트가 제외된다.
공용 도우미는 `Tests/CosmicTestHelpers.h`(`CosmicTests::Flags`·`AcquireSim`): 테스트는 실행 중인 게임의 Subsystem을 멈춰 놓고(`SetTimeScale(0)`) 은하를 갈아 끼운다. 유니티 빌드라서 .cpp마다 익명 네임스페이스에 같은 이름을 두면 충돌한다 — 테스트 파일별 네임스페이스(`CosmicLogTests` 등)를 쓴다.
`AddExpectedMessage`/`AddExpectedError`의 Occurrences: 양수 = 정확히 그 횟수, 0 = 한 번 이상, 음수 = 횟수 무시(0번도 통과). 일부러 망가뜨린 세이브를 로드하는 테스트는 엔진의 `Failed loading tagged` 에러를 이것으로 예상 처리한다. 새 테스트는 한 번은 일부러 깨뜨려서(변이) 실패하는 것을 본 뒤 되돌린다. `ensure`는 `AddExpectedError`로 덮을 수 없다(제목·조건·메시지·콜스택이 줄마다 Error로 찍히고 프로세스당 한 번만 발화한다) — 테스트할 호출자 오류는 `UE_LOG(LogCosmic, Error, …)` 한 줄로 알린다(`DrawTell(Count < 1)`). `FProperty::ClearValue_InContainer`는 스칼라를 0으로, 구조체를 C++ 기본값으로 되돌린다(`Sim.TellStreamSeparate`의 최상위 멤버 표). 변이를 되돌릴 때는 파일의 수정 시각이 새로워져야 한다: PowerShell `Copy-Item`은 백업의 옛 시각을 유지해서 UBT가 다시 컴파일하지 않고 `Succeeded`를 낸다 — bash `cp`·Python으로 다시 쓰기·`touch`를 쓰고, 깨뜨렸던 테스트가 다시 통과하는지로 확인한다. `TestEqual`의 bool 꼴은 실패 메시지가 `Expected …`가 아니라 `…: The two values are not equal.`이다(로그를 걸러 볼 때). 핸들러 안에서 금지된 호출을 다시 해 보는 테스트는 한 번만 시도하게 한다(막히지 않는 구현에서는 무한 재귀로 프로세스가 죽고 뒤의 테스트가 돌지 않는다). 델리게이트를 받는 테스트는 `Tests/CosmicTestProbe.h`의 UCLASS를 쓴다(리플렉션 클래스는 `WITH_DEV_AUTOMATION_TESTS` 뒤에 둘 수 없다). `AddExpectedError`의 무늬는 기본이 정규식이다(`IsRegex = true`) — 괄호·마침표 같은 글자를 그대로 맞추려면 `AddExpectedErrorPlain`을 쓰거나 무늬에서 뺀다. 변이는 앞당긴 `return`으로 쓰지 않는다(도달 불가 코드가 에러다) — `if (false && …)`처럼 조건을 죽인다. 한 빌드에 변이 여럿을 묶을 때는 변이마다 다른 단언이 깨지게 묶고 실패 메시지로 가른다. 열거자 수는 리플렉션에서 읽는다(`StaticEnum<T>()->NumEnums() - 1`): 끝에 더한 열거자가 경우 없이 지나가지 않는다. 대조군은 테스트가 만든다 — 게임 데이터에 기대면 데이터가 찰 때 단언이 조용히 비어 버린다(`CivsWithoutNames`). 행을 주입하는 테스트는 조건을 가진 행도 넣는다(조건 없는 행만으로는 `CheckAll`이 늘 빈 배열을 받는다). 값이 그대로임을 보는 단언은 0이 아닌 값에서 본다(0은 곱해도 0이다 — `RowAtomic`의 수입). `TestTrue` 뒤의 배열 접근은 가드한다(단언은 멈추지 않고 `TArray`의 범위 검사는 프로세스를 끝낸다).

## 모듈 / UHT / 컴파일 규칙 (5.8 UBT 소스 확인)

헤더 파일 이름은 **타깃 전체에서 유일**해야 한다(UHT가 모듈을 가로질러 검사). `Types.h`, `Utils.h` 같은 이름 금지. 항상 도메인 접두어(`CosmicTypes.h`).
`.generated.h`는 헤더의 마지막 include. `#pragma once` 필수.
include는 모듈 루트 기준(`"Sim/GalaxySimulationSubsystem.h"`). `"Cosmic_Ascension/Sim/..."`도 해석되지만 쓰지 않는다. 서브폴더는 자동 include 경로가 아니므로 다른 폴더의 헤더를 파일명만으로 include하지 않는다.
BuildSettingsVersion.V7 + IncludeOrder Unreal5_8: C++20, `UE_LOG` 포맷 문자열 컴파일 타임 검증, 미정의 식별자/반환 타입/dangling/unreachable 경고가 **에러**, 전이 include 의존 금지(쓰는 헤더는 직접 include).
Live Coding은 .cpp 변경에만. UCLASS/USTRUCT/UENUM 추가·변경은 에디터 종료 후 전체 빌드.
UPROPERTY의 기본값 초기화식은 UHT가 `;`까지 건너뛴다(`EpicGames.UHT/Parsers/UhtPropertyParser.cs`의 `PropertyParsed`). `TArray<float> X = { 0.f, 1.f };`, `TMap<FName, float> M = { { TEXT("A"), 1.f } };` 같은 중괄호 초기화를 쓸 수 있다. Category 지정자는 엔진 모듈에서만 필수다.

## 검증된 엔진 헤더 경로 (5.8.3 로컬 소스 확인)

(Phase 0 조사 결과를 아래 표로 유지한다. 새 API를 쓰면 행을 추가한다.)

| 심볼 | include | 모듈 | 비고 |
|---|---|---|---|
| `USubsystem`, `UGameInstanceSubsystem` | `Subsystems/Subsystem.h`, `Subsystems/GameInstanceSubsystem.h` | Engine | `Engine/Public/Subsystems`에 있음. `Classes/Subsystems`는 5.8에 없음 |
| `FSubsystemCollectionBase` (InitializeDependency 등) | `Subsystems/SubsystemCollection.h` | Engine | Subsystem.h는 전방 선언만. `Collection.InitializeDependency<T>()`를 쓰면 이 헤더 필요 |
| `UGameInstance::GetSubsystem<T>()` | `Engine/GameInstance.h` | Engine | 5.8 include order에서 전이 include에 의존하지 말 것 |
| `FTickableGameObject`, `ETickableTickType` | `Tickable.h` | Engine | `Stats/Stats.h`를 포함하므로 `RETURN_QUICK_DECLARE_CYCLE_STAT`, `STATGROUP_Tickables`도 함께 옴 |
| `USaveGame` | `GameFramework/SaveGame.h` | Engine | `ULocalPlayerSaveGame`(버전 필드 내장)도 같은 헤더 |
| `UGameplayStatics::SaveGameToSlot/LoadGameFromSlot/DoesSaveGameExist/DeleteGameInSlot` | `Kismet/GameplayStatics.h` | Engine | `(const FString& SlotName, const int32 UserIndex)`. 비동기 `AsyncSaveGameToSlot`는 네이티브 델리게이트(ThreeParams). 저장은 태그드 프로퍼티이고 `ArIsSaveGame`을 켜지 않으므로 `SaveGame` 지정자 없이 모든 UPROPERTY가 대상이다. 단 클래스 기본 객체 대비 델타라서 초기값과 같은 직속 멤버는 기록되지 않는다(`TArray` 원소 구조체는 전부 기록). UENUM은 이름으로 저장되고 사라진 이름은 enum 최대값으로 로드된다. FName은 문자열로, FGameplayTag는 태그 이름으로 저장된다(ini에서 사라진 태그는 이름은 남지만 `RequestGameplayTag`가 빈 태그를 준다) |
| `FRandomStream` | `Math/RandomStream.h` | Core | 반드시 `Initialize(int32 Seed)`로 초기화. `Initialize(FName)`에 NAME_None을 주면 시간 기반 시드가 되어 결정론이 깨진다. `FRandRange`는 double(`FVector::FReal`) 반환, `GetFraction/FRand`는 float, `RandRange(int32,int32)`는 양끝 포함. 결과에 `%` 금지(하위 비트 품질 나쁨, 헤더 경고). 저장·복원: 상태는 `uint32 Seed` 하나라서 `GetCurrentSeed()`(int32)를 저장하고 `Initialize(int32)`로 되돌리면 스트림이 그대로 이어진다(`Reset()`·`GetInitialSeed()`는 쓰지 않는다). `RandHelper(int32 A)`는 [0, A)이고 A > 0이면 상태를 한 번 전진시킨다(`RandRange(0, A - 1)`과 같다). 저장된 값으로 지역 스트림을 열어(`FRandomStream(int32)`) 한 번 뽑고 `GetCurrentSeed()`를 되적으면 살아 있는 스트림과 같은 열이 이어진다(`DrawTell`). `GetFraction()`이 주는 값은 k × 2⁻²³이고 가장 큰 값은 1 − 2⁻²³이다 — 누적한 몫과 견줄 때 몫을 하나씩 나눠 줄이면 반올림으로 꼭대기에 아무 몫에도 들지 않는 수가 남는다(이벤트 슬롯은 수에 합을 곱한다) |
| `FAnsiStringView`, `StringCast<ANSICHAR>(const TCHAR*)` | `Containers/StringView.h`, `Containers/StringConv.h` | Core | `FAnsiStringView`는 값으로 넘긴다. `const ANSICHAR*` 생성자와 `begin()`·`end()`가 constexpr라서 문자열 리터럴을 받는 constexpr 함수와 `static_assert`에 쓸 수 있다(`Core/CosmicSeed.h`). 부호 없는 상수 넘침 경고(C4307)는 엔진이 꺼 둔다. `StringCast` 결과는 `.Get()`·`.Length()`. `FName::ToString()`은 에디터 타깃에서만 대소문자를 보존한다(`WITH_CASE_PRESERVING_NAME`) — 이름을 바이트로 해시하기 전에 소문자로 바꾼다 |
| `UEnum::NumEnums()`, `StaticEnum<T>()` | `UObject/Class.h` | CoreUObject | `NumEnums()`는 이름 수다. UHT가 만든 UENUM에는 끝에 `_MAX`가 더해져 있어 선언한 열거자 수는 `NumEnums() - 1`이다(MAX를 직접 선언한 열거형에는 더해지지 않으므로 그때도 MAX를 뺀 수가 `NumEnums() - 1`이다. `ContainsExistingMax()`는 두 경우 모두 참이라 둘을 가르지 못한다) |
| `FGuid` | `Misc/Guid.h` | Core | 5.8의 `CoreMinimal.h`는 이 헤더를 포함하지 않는다(전이로만 들어온다). 직접 include |
| `TSoftObjectPtr` | `UObject/SoftObjectPtr.h` | CoreUObject | 가리키는 클래스는 전방 선언으로 충분 |
| `FNameLexicalLess` | `UObject/NameTypes.h` | Core | 프로세스와 무관한 FName 정렬. `GetTypeHash(FName)`·`GetTypeHash(FGameplayTag)`·`FNameFastLess`는 이름 테이블 인덱스 기반이라 프로세스마다 다르다 — 해시·정렬 기준으로 쓰지 않는다 |
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
| `LoadObject<T>(nullptr, Path, {}, LOAD_NoWarn)` | `UObject/UObjectGlobals.h` | CoreUObject | 경로는 `/Game/CA/Data/DT_Rules.DT_Rules` 꼴. 없으면 nullptr이고 `LOAD_NoWarn`이면 엔진 경고도 없다(직접 `LogCosmic`으로 알린다). Subsystem `Initialize`에서 불러도 된다(-game, 커맨드렛 모두). 쿠킹에는 `Config/DefaultGame.ini`의 `+DirectoriesToAlwaysCook=(Path="/Game/CA/Data")`가 필요하다(코드 경로 로드는 참조로 잡히지 않는다) |
| `EStringTableLoadingPolicy`, `FStringTableEntry::GetPlaceholderSourceString()` | `Internationalization/StringTableCoreFwd.h`, `Internationalization/StringTableCore.h` | Core | 에셋 테이블 ID는 객체 경로(`/Game/CA/Data/ST_CA.ST_CA`). `FindOrLoad`(기본)는 `IsAsyncLoading()` 중이면 비동기 로드라서 그 사이 값이 자리표시 문자열(`<MISSING STRING TABLE ENTRY>`)이 된다. UI는 `FindOrFullyLoad` |
| `UWidgetTree::ConstructWidget<T>()`, `UUserWidget::WidgetTree`·`RebuildWidget`·`TakeWidget` | `Blueprint/WidgetTree.h`, `Blueprint/UserWidget.h` | UMG | WBP 없는 C++ 위젯은 `RebuildWidget`에서 `WidgetTree->RootWidget`이 없을 때 한 번 트리를 만들고 `Super::RebuildWidget()`을 부른다. `NativeConstruct`는 첫 `TakeWidget`(`AddToViewport`) 때, `NativeDestruct`는 Slate 위젯이 풀릴 때 온다. WBP가 없는 클래스는 `DisableNativeTick` 메타와 무관하게 틱한다(`UpdateCanTick`의 `!WidgetBPClass`). `CreateWidget<T>(Owner)`의 Owner는 PlayerController·GameInstance·World 등. 테스트는 `WidgetTree->ForEachWidget`으로 `UTextBlock::GetText()`를 읽는다 |
| `AGameModeBase`, `AHUD`, `APlayerController`, `FKey`/`EKeys` | `GameFramework/GameModeBase.h`, `GameFramework/HUD.h`, `GameFramework/PlayerController.h`, `InputCoreTypes.h` | Engine, InputCore | 맵을 건드리지 않고 게임 모드를 고르는 곳은 `Config/DefaultEngine.ini`의 `[/Script/EngineSettings.GameMapsSettings] GlobalDefaultGameMode`. 키는 `InputComponent->BindKey(FKey, IE_Pressed, this, &Class::Handler)`, 핸들러는 `void(FKey)` |
| `UGameMapsSettings` | `GameMapsSettings.h` | EngineSettings | `GameInstanceClass`(`TryLoadClass<UGameInstance>()`). 밸런스 커맨드렛이 읽는다. Build.cs Private 의존 |
| `UWorld::DestroyWorld`, `UEngine::DestroyWorldContext` | `Engine/World.h`, `Engine/Engine.h` | Engine | `InitializeStandalone`으로 만든 GameInstance를 치울 때 쓴다(아래 Phase 1 메모) |
| (에디터 Python) `unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(table, json, struct)`, `unreal.StringTableLibrary.import_table_from_csv_string`, `unreal.EditorAssetSubsystem` | — | 에디터 전용 | 확인한 사실은 `Tools/make_data_tables.py` 머리 주석에 있다. JSON은 행 객체 배열·행 키 `"Name"`, 단일 태그는 문자열, 태그 컨테이너는 export 텍스트 `(GameplayTags=((TagName="A.B")))`. 임포트는 행에 문제가 있어도 True를 돌려주므로 스크립트가 다시 내보내 비교한다. `unreal.EditorAssetLibrary`는 이 프로젝트가 켜지 않은 플러그인 소속이라 쓰지 않는다 |
| `UDataTable::AddRow(FName, const FTableRowBase&)`, `RowStruct` | `Engine/DataTable.h` | Engine | 런타임에서도 쓸 수 있다. 테스트가 `NewObject<UDataTable>(GetTransientPackage())`에 `RowStruct`를 넣고 행을 더해 `SetTables`로 주입한다(`Sim.GenerationRules`). 끝나면 원래 테이블을 되돌린다. `GetRowNames()`는 넣은 순서다 — 순서가 결과에 닿는 곳은 `FNameLexicalLess`로 정렬한 이름으로 돈다(`GetCivRowNames`·`GetEventRowNames`, 테스트 `Events.RowOrderInert`) |
| `DECLARE_DYNAMIC_MULTICAST_DELEGATE_*`의 인자, 본문 없는 멤버 선언 | `Delegates/DelegateCombinations.h` (CoreMinimal) | Core | 동적 델리게이트의 인자는 리플렉션 타입이어야 한다. 상태와 스트림의 참조를 든 C++ 구조체(평가 문맥 `FCosmicEvalContext`)는 넘기지 못하므로 값만 든 USTRUCT(`FCosmicEventNotice`)를 넘긴다. API 매크로가 붙은 UCLASS에 본문 없는 비-UFUNCTION 멤버 선언이 있어도 부르는 곳이 없으면 게임·에디터 타깃 모두 링크된다(헤더 승인 단계의 컴파일 확인). 참조 멤버를 든 구조체는 복사 생성은 되고 대입만 안 된다 — 복사를 막으려면 복사 생성자를 지우고 이동 생성자를 default로 둔다(값 반환은 그대로 된다) |
| `FPackageName::TryConvertLongPackageNameToFilename(const FString&, FString&, const FString& Ext)`, `GetAssetPackageExtension()` | `Misc/PackageName.h` | CoreUObject | `/Game/CA/Data/DT_Rules` → 디스크의 `.uasset` 경로. `Sim.SaveRestart`가 DT 에셋의 타임스탬프를 기록 키에 넣는다 |
| `FSHA1::HashBuffer(Data, Size)` → `FSHAHash::ToString()` | `Misc/SecureHash.h` | Core | 40자 16진. `Data.SourcesInSync`가 `Tools/data` 원본을 CR을 뺀 바이트로 해시한다(체크아웃의 줄바꿈 변환과 무관) |

**GameplayTags ini 규칙 (5.8 소스 확인)**
`Config/DefaultGameplayTags.ini`는 계층 레이어라서 배열 행은 반드시 `+GameplayTagList=(Tag="A.B",DevComment="...")`. `+` 없는 행은 배열을 통째로 덮어쓴다.
시스템별로 파일을 나누고 싶으면 `Config/Tags/<이름>.ini`에 섹션 `[/Script/GameplayTags.GameplayTagsList]`와 `+` 없는 `GameplayTagList=(...)` 행을 쓴다(자동 탐색). 거기서 `+`를 붙이면 키 이름의 일부로 취급되어 무시된다.
속성 이름은 `WarnOnInvalidTags`(b 접두어 없음). `ClearInvalidTags`는 5.5에서 deprecated → 쓰지 않는다. 태그 이름에 `"` `'` `,` 금지.

**Phase 1에서 틱 Subsystem을 만들 때 (5.8 확인 사항)**
`FTickableGameObject`를 UObject(Subsystem)와 함께 쓸 때는 헤더 주석대로: 생성자에서 `FTickableGameObject(ETickableTickType::Never)`, `Initialize()` 끝에서 `SetTickableTickType(ETickableTickType::Conditional)`, `Deinitialize()`에서 다시 `Never`. `IsAllowedToTick()`은 UE_DEPRECATED(5.5)이므로 오버라이드하지 않는다. `GetTickableGameObjectWorld()`를 오버라이드하지 않으면 월드 없는 패스(`UGameEngine::Tick`의 `TickObjects(nullptr, LEVELTICK_All, false, DeltaSeconds)`)에서 돌므로 엔진 일시정지·시간 팽창이 닿지 않는다(`IsTickableWhenPaused()`는 월드에 묶인 객체에만 의미가 있다).
`GetStatId()`는 `RETURN_QUICK_DECLARE_CYCLE_STAT(UGalaxySimulationSubsystem, STATGROUP_Tickables);`.
밸런스 커맨드렛(§9.3)은 UEditorEngine 아래에서 돌고 UGameInstance가 없다. `Main()`에서 `NewObject<UGameInstance>(GEngine, Class)` → `AddToRoot()` → `InitializeStandalone()`으로 GameInstance를 직접 만들어 Subsystem을 얻는다(`Tests/CosmicBalanceCommandlet.cpp`). `InitializeStandalone`은 월드 컨텍스트와 더미 월드를 만들고 `Shutdown()`은 둘을 치우지 않는다. 끝낼 때는 `UWorld* World = GameInstance->GetWorld()`를 잡아 두고 `Shutdown()` → `GEngine->DestroyWorldContext(World)` → `World->DestroyWorld(false)` → `RemoveFromRoot()` 순서로 치운다(엔진 `AutomationCommon.cpp`와 같은 순서). 빼면 종료 때 `WorldPartitionSubsystem … destroyed while still initialized` ensure가 뜬다.

## 렌더 / 플러그인 결정 (2026-10-10, Config/DefaultEngine.ini)

레이트레이싱 OFF + SkinCache OFF (한 묶음. RT만 켜고 SkinCache를 끄면 시작 시 Fatal). Lumen GI/반사 OFF, 메시 디스턴스 필드 OFF (읽기 전용 cvar, 메시 0개인 지금 결정). Substrate ON 유지 (5.8 신규 프로젝트 기본값. 나중에 끄면 에디터 재시작이 필요하고 Substrate 전용 노드로 만든 머티리얼이 깨지므로 머티리얼 작성 전인 지금 확정). 정적 라이팅 OFF, 가상 섀도맵 ON, DX12 SM6.
플러그인: ModelingToolsEditorMode 비활성, Niagara 명시 활성, CommonUI 비활성(기본값). PythonScriptPlugin은 헤드리스 에디터 스크립트용으로 활성. .uproject의 `TargetAllowList: ["Editor"]`는 Niagara.uplugin이 PythonScriptPlugin을 타깃 제한 없이 의존하기 때문에 게임 타깃에서 효력이 없다(게임 exe에는 `WITH_PYTHON=0` 스텁 모듈이 링크됨, 무해). 그래도 Niagara를 끌 경우를 위해 블록은 유지하고, DisablePlugins 등으로 게임 타깃에서 강제 제외하려 하지 않는다. 실제 Python 실행은 에디터 바이너리(UnrealEditor, UnrealEditor-Cmd)에서만 된다.
5.8 프로젝트 브라우저에는 레이트레이싱/스타터 콘텐츠 토글이 없고 RT·Lumen·Substrate를 무조건 켜서 쓴다. 끄는 방법은 ini 편집만이다.

## 현재 단계

Phase 0 (셋업) 종료 조건 달성 (2026-10-10): 에디터/게임 타깃 커맨드라인 빌드 성공, `Content/CA/Maps/L_Galaxy.umap` 생성(헤드리스 Python), `GameDefaultMap`/`EditorStartupMap`을 L_Galaxy로 지정, `-game -nullrhi` 스모크 실행에서 L_Galaxy 로드 + Subsystem 6개의 `LogCosmic` 초기화 로그 확인(`Saved/Logs/GameSmoke.log`, 에러 0).
설계 문서는 guideline.md **v0.6** (2026-10-11): v0.4(옛 §17 '부름과 계승'·§18 '통치자 시작·이중 계·세력군·기독교적 요소')와 v0.5(§17 '사다리 위의 두 영역·영혼의 행방·지구와 계시 양식' — 사다리 상한, 교리 4종·회귀 `Doctrine.Recurrence`·판당 활성 타락 ≤ 3, 영혼 경제 `Hazard.Claim`, 귀환 제안 `OfferReturn()`, 계시 양식 `RevelationMode`, 지구 `arch_terran` 첫 생애 고정, 닫힌 책 `Trait.Faith.Book`, 사후 문장 규칙·두 책 장)의 채택분 위에, v0.6에서 '요약 단위 원칙'(§1.3: 사람은 수치, 특출난 소수만 이름, 문명·영·육신도 요약, 표시는 규모 등급)을 명문화하고 인물 기록 `FNotableRecord`·규모 등급 `GetPopulationBand`를 본문에 반영했다. Phase 1 헤더에는 예약 필드(`FCallingState`/`FLifeRecord`/`ELifeState`, `GalaxyID`, `HomeSystemID`, `ThroneLockedUntilTick`, `StatusTags`, `Exaltation`, `MegaprojectCompletedTick`, `SealedTick`, `FGalaxyState.Flags`/`Chronicle`, `FCallingState.PendingRoleEffect`, `FCivilizationState.LowStabilityTicks`, `FCivilizationState.ArchetypeRow`, `FLifeRecord.SuccessionKind`/`bReturnOffered`, `FTranscendentDef.Allegiance`(상태에 `Realm` 없음), 방송 게이트, `Role` 기본값 Ruler, `DT_Civs`·`DT_Rules.PlayerArchetypeRow`, 그리고 v0.6의 `FNotableRecord`/`FCivilizationState.Notables`, `FLifeRecord.NameIndex`, `FCivArchetypeDef.PersonNameBank`, `EEffectType::NameNotable`, `EConditionType::CivHasNotable`, `EPopulationBand`, `DT_Rules.MaxNotablesPerCiv`/`PopulationBandThresholds`, `FHazardDef.OnResolveEffects`)가 포함된다 (§11 Phase 1, §7.2, §7.3, §9.1). 옛 '사도(Apostle)' Hazard 태그는 `Hazard.Cult`로 개명됐고 GameplayTag는 총 136개(v0.5 추가 36개, v0.6 추가 `Notable.*` 6개, §16 #21 확정으로 `Effect.KillPlayer`·`Effect.DeposeRuler` 2개 포함)가 `Config/DefaultGameplayTags.ini`에 선언되어 있다.
Git 저장소 초기화·커밋 완료(2026-10-11, `main`, LFS).
Phase 1 헤더 (2026-10-12, 완료): 헤더 4개 — `Core/CosmicTypes.h`(+`.cpp`, 공통 열거형·`FResourcePool`·`FEthicsAxes`), `Core/GalaxyState.h`(§7.2 런타임 상태 전체 + §8.1 `FCosmicLogEntry`), `Data/CosmicConditionTypes.h`(`EConditionType` 28·`EEffectType` 18과 조건·효과 구조체), `Data/CosmicDefinitionRows.h`(DataTable 행 구조체 9: 이벤트·Hazard·Feat·조우 선택지/조우·아키타입·초월자·로그 템플릿·규칙). §9.1·§11의 Phase 1 예약 필드는 전부 들어 있다(방송 게이트·`GetPopulationBand`·`ComputeStateHash` 선언은 Subsystem 헤더에 있다). 게임·에디터 타깃 빌드와 `-game -nullrhi` 스모크 통과. ultracode 헤더 리뷰 1회 완료(3관점 + 완전성 비평 51건 → 적대 검증 CONFIRMED 44, 수정 뒤 변경 범위 재감사 10건 → 8건 반영): 코드 변경은 `SaveVersion` 초기값 1 고정 하나이고 나머지는 직접 include 2개·주석 정정·guideline 문장 정정이다. 설계 결정 13건(§16 #19~#31) 가운데 #19~#22는 확정했고 나머지(#23~#31)는 미결이다. 헤더는 스케치대로다. 스케치와 다르게 둔 것: `Body`/`Mind`/`Causality`는 UPROPERTY 3개로 분리, `CosmicTypes.h`는 태그 헤더를 include하지 않음(쓰지 않으므로), 주석은 영문 ASCII, 구조체마다 `COSMIC_ASCENSION_API`, `FGalaxyState::CurrentSaveVersion` 상수, §7.3은 파일 2개로 분리.
Phase 1 종료 조건 달성 (2026-10-12, guideline §11 Phase 1의 상태 줄): 시뮬 코어(`Sim/GalaxySimulationSubsystem` — `NewGalaxy`, 고정 스텝 틱·배속·이월 상한, 틱 8단계 중 경제·연구·재위 누계 본문과 나머지 골격, 리플렉션 기반 `ComputeStateHash`), 첫 생애(`Player/PlayerRoleSubsystem::Install`), 저장·로드(`Save/CosmicSaveSubsystem`, 슬롯 `Galaxy_<GUID>`), 데이터 계층(`Data/CosmicDataTables`·`CosmicRowNames`, `Tools/make_data_tables.py`와 `Tools/data` 원본, `Content/CA/Data`의 `DT_Rules`·`DT_Civs`·`DT_LogTemplates`·`ST_CA`), 로그(`Log/CosmicLogSubsystem` — `Push`·`PushRow`, 2,000줄 상한과 연대기 이관), 표현 계층(`UI/CosmicTextPanelWidget`, `World/CosmicGameMode`·HUD·PlayerController: 0/1/2/3/4 = 정지/1/2/5/10배속, Space = 정지 토글), 밸런스 커맨드렛(`Tests/CosmicBalanceCommandlet`, `-run=CosmicBalance`). 자동화 테스트 12종(`Sim.Determinism`·`NoNaN`·`FirstLife`·`PopulationBand`·`FrameStep`·`GenerationRules`·`SaveRoundTrip`·`SaveRestart`, `Log.NoConsecutiveRepeat`·`CapAndChronicle`, `UI.PanelLabels`, `Data.SourcesInSync`)이 연속 두 프로세스에서 통과한다. ultracode 리뷰 3회(헤더, 시뮬 코어 41건 → CONFIRMED 38, 종료 리뷰 55건 → CONFIRMED 53. 시뮬 코어·종료 리뷰는 높음 0)의 확정분은 반영했고, 종료 리뷰의 수정분은 변경 범위 재감사(27건 → CONFIRMED 23, 높음 0)까지 거쳤다. 종료 리뷰에서 나온 설계 질문은 guideline §16 #37~#40에 미결로 두었다.
아직 확인하지 못한 것: 게임 바이너리(쿠킹된 빌드)에서 같은 시드가 같은 해시를 내는지(첫 패키징 때), 실행 중 패널이 틱마다 갱신되는 모습을 눈으로 보는 것(첫 프레임 스크린샷과 `UI.PanelLabels`의 델리게이트 검사까지만 했다). Phase 1 데이터의 한계: AI 문명은 표시 이름이 없어서 플레이어 문명만 돌파 로그를 낸다. `DT_Civs` 두 행의 `PersonNameBank`는 비어 있고 `RevelationTemplates`는 [0] 징조 한 줄뿐이다(이름 60개·계시 21줄은 Phase 4 콘텐츠, §11) — 그때까지 `Install()`은 호출마다 시뮬 스트림에서 정확히 1회 뽑은 수를 버리고(`NameIndex` 0. Phase 2 이벤트 평가기 작업부터의 고정 추첨이라 뱅크를 채워도 시뮬 난수열은 밀리지 않고 `NameIndex`만 달라진다), `RevelationTemplates`를 읽는 코드는 `Num() == 3`을 먼저 확인한다. guideline §16 미결: #1~#18, #23~#31, #33~#40.
설계 문서는 guideline.md **v0.7** (2026-10-12): 개발자의 세 방향(은하는 셀 수 없이 많다 / 문명이 은하를 넘나들고 생명이 사는 성계는 60개쯤 / 만난 뒤에야 디테일이 살아난다)을 채택했다. 결정한 것: 한 판 = 우주 시드 1개(저장은 판당 슬롯 1개, 이름은 그대로). 상세로 시뮬되는 은하는 고향 은하(우리은하) 하나이고 무대 은하 4개는 Phase 6a부터 요약으로 흐르며 나머지는 풍경이다(`DT_Galaxies` 6행). 한 은하의 네 층(배경 별·생명 성계·백성·넷째 칸 이상의 존재), VS 60·12와 EA 시작값 300·48, 보유 상한 `RealmCapByTier`, 정착 `project_settle`, 넷째 칸의 제동 `AIFourthRungCap`. 앎(`Knowing` — 부름의 것이고 세계는 쓰기만 한다), 보는 거리·만남·첫 접촉, 이야기의 범위와 `bUntold`, 난수의 출처 셋과 세계 해시, 틱 열 단계, 다리와 건넘(Phase 6a), 플레이어의 건넘은 Phase 6b 후보, 현실 수치 §3.4, 정의 스키마 `Tools/data/schema.json`. 위 불변 규칙에서 '(v0.7 — …)' 표지가 붙은 줄이 코드가 아직 뒤따르지 않은 곳이다('v0.7 예약' 커밋이 따라간 것은 아래 문단). 이 파일의 명령 블록은 오늘의 코드로 검증한 것이라 고치지 않았다(`-Systems`·`-Civs`의 기본값은 Phase 2 코드와 함께 바뀐다). guideline §16 미결: #1~#18, #23~#31, #33~#51(#36 ③⑤⑥과 #40 ②는 닫혔다).
'v0.7 예약' 종료 조건 달성 (2026-10-12, guideline §11 'v0.7 예약'의 상태 줄): 저장 필드 25개와 구조체 3개(`FKnownMark`·`FKnowingState`·`FTransitRecord`, `FGalaxyState.Knowing`·`Transits`·`NextCivSerial`·`TellRngState`, 성계·문명의 `GalaxyRow`·`OriginGalaxyRow`, 로그 항목의 `FocusCivID`·`bUntold`), `Core/CosmicSeed.h`(`Derive`와 용도 상수 8. 기준값은 헤더의 `static_assert`와 테스트가 함께 쥔다), 말하기 스트림(`DrawTell`. `PushRow`가 시뮬 스트림에서 빠져서 같은 시드의 세계가 이 커밋에서 한 번 달라졌다), `ComputeWorldHash`, 빈 `TickCrossings`·`TickSight`(틱은 열 단계), `Push`·`PushRow`의 `FocusCiv` 인자, 밸런스 커맨드렛의 두 해시 출력. `SaveVersion`은 1 그대로다. 자동화 테스트 14종(새 `Sim.SeedDerive`·`Sim.TellStreamSeparate`, 보강 `Log.NoConsecutiveRepeat`·`Log.CapAndChronicle`·`Sim.SaveRoundTrip`·`Sim.SaveRestart`)이 연속 두 프로세스에서 통과하고, 새 단언은 변이 7개로 한 번씩 깨뜨려 보았다. 헤더 리뷰 1회(3관점 + 적대 검증 + 완전성 비평: 유지 26·기각 2·비평 5, 높음 0)의 확정분을 반영했다. 구현 뒤 ultracode 리뷰 1회(말하기 스트림 분리·해시 제외·저장 호환·문서 대조 — 13건 가운데 유지 11·기각 2, 완전성 비평 4, 높음 0, 동작 결함 0)의 확정분도 반영했다: `Sim.TellStreamSeparate`의 마지막 블록을 `CivName` 게이트에 기대지 않게 고쳐 쓰고 변이 둘(`PushRow`의 스트림, `DrawTell`의 되적기)로 다시 깨뜨려 보았다. 아직 쓰이지 않는 것: `TellRngState`(`NewGalaxy`·`DrawTell`) 말고는 새 필드를 테스트만 채운다 — `GalaxyRow` 도장·`NextCivSerial`·`Knowing`의 기록·`bUntold`·`FocusCiv`를 넘기는 호출부는 Phase 2, `Transits`·`OriginGalaxyRow`는 Phase 6a다. `FTransitRecord.Kind`의 태그 `Transit.People`은 Phase 6a에 ini에 선언한다(그때까지 테스트는 선언된 태그를 대신 넣는다). `TickSight`의 두 계기(주기, 그릇이 움직인 틱)는 저장 필드도 Subsystem 멤버도 두지 않는다 — `SimulateTick`의 지역 값으로 넘긴다.
Phase 2 진행 중 (2026-10-13). 작업의 목록·순서·하는 일은 WORKPLAN.md §3의 표에, 리뷰 건수 같은 지난 기록은 guideline §11 Phase 2의 상태 줄에 있다. §16 #27·#37·#38·#39는 정했다(세 설계안 → 세 심사 → 종합 → 적대 리뷰).
- 끝난 작업 '상태 색인': `Sim/CosmicStateIndex`(ID → 인덱스 조회, `CosmicState::IsSettled`), `IsBroadcasting`.
- 끝난 작업 '이벤트 평가기': `Events/CosmicEventSubsystem`(static `Check`·`CheckAll`·`Apply`·`ApplyAll`·`CooldownKey`, 멤버 `EvaluateAndFire`·`FireRow`, `OnEventFired`. 평가 문맥은 `FCosmicEvalSubject`·`FCosmicEvalView`·`FCosmicEvalContext`), `UCosmicLogSubsystem::PushVariants`와 `FCosmicLine`, `FCosmicEventDef`의 `LogCategory`·`Trigger`·`Subject`·`SignRow`, `DT_Rules`의 `EventSlotTicks`·`EventChancePerSlot`, `FCosmicDataTables.Events`, `CosmicSeed` 용도 9 `EventSlot`과 `DeriveForName`, `MakeEvalContext`, 위기 단계 끝의 이벤트 슬롯, `Install`의 고정 추첨(같은 시드의 세계가 한 번 더 달라졌다).
- 이벤트 평가기의 리뷰: ultracode 리뷰 1회와 그 수정분의 재감사까지 끝났다. 리뷰가 고친 동작은 여섯이다 — 상대를 찾지 못하는 `ChangeRelation`·`CivRelationAtLeast`, 숫자만이 아닌 `CivStabilityBelowFor`의 Key, Tick 0에 닫는 인물 기록, 조정값마다 따로 내는 에러 줄, 합이 1을 넘는 Lottery 확률의 반올림 틈, `SignRow`의 줄이 나가는 조건. 조건을 쿨다운 키보다 먼저 보게 순서도 바꿨다. 나머지는 테스트 보강과 주석·문서 정정이다. 게임에는 `DT_Events` 행이 없어 같은 시드의 세계는 그대로다(커맨드렛의 두 해시 불변).
- 테스트: 자동화 테스트 25종(새 `Sim.StateIndex`, `Events.CheckIsPure`·`SlotDraws`·`RowOrderInert`·`LotteryShares`·`CooldownPerCiv`·`ConditionGate`·`RowAtomic`·`NotableDrawFixed`·`RoleEffectRules`, `Log.VariantsLedger`)이 연속 두 프로세스에서 통과한다. 새 단언은 변이(일부러 고장 낸 코드)로 한 번씩 깨뜨려 보았다 — 두 작업의 구현에서 15개, 리뷰 반영에서 41개이고, 고친 동작 여섯은 고치기 전 코드에서 새 단언이 실패하는 것을 먼저 보았다.
- 아직 없는 것(괄호는 그것을 넣을 작업): `DT_Events` 에셋과 행 — 게임에서는 이벤트 추첨만 돌고 나올 이벤트가 없다(첫 이벤트 데이터). `SpawnHazard`·`ResolveHazard`·`TransferSystem`의 구현(위기 처리 — 그때 이 헤더에 효과의 후속 처리 기록과 `FollowUpKnowing`·`FollowUpNotes`가 더해진다). `SystemPressureAtLeast`는 거짓이고 Lottery 행의 가중 인수는 1이다(영적 압력). 로그 줄을 볼 수 있는 Tier 범위 `MinViewerTier`~`MaxViewerTier`를 읽는 일(아는 범위와 로그). 비용과 빈도는 측정 전의 추정값이다(종료 리뷰의 커맨드렛).
- guideline §16 미결: #1~#18, #23~#26, #28~#31, #33~#36, #40~#59(#27·#37·#38·#39는 정했다. #57~#59는 초월자의 심도와 현현 — 2026-10-13에 채택한 개발자 초안의 남은 물음이고 기본값이 본문에 있다).

다음(작업의 이름과 하는 일은 WORKPLAN.md §3의 표): 영적 압력(`ComputeSpiritualPressure`, 초월자의 심도, 압력이 매 틱 거는 효과, 판에 나올 초월자 추첨. 헤더 전에 §16 #55 ③·#57~#59) → 스키마 검사기(그 전에 §16 #40 ⑥) → 은하 생성 규칙 변경 → 첫 이벤트 데이터(그 전에 §16 #23·#25·#29·#54) → 위기 처리(그 전에 §16 #52·#53·#56) → 승천 판정 → 정착과 문명 제거 → 아는 범위와 로그 → 종료 리뷰. 헤더가 새로 생기거나 바뀌는 작업은 헤더 먼저 제시 → 승인. effort xhigh, 구현은 단독, 서브시스템 하나가 끝날 때마다 ultracode 리뷰(WORKPLAN §2).
참고: §10.1의 빈 폴더(Core, Data, World, UI, Tests, Content/CA/*)는 Git에 올라가지 않는다. 첫 파일이 생길 때 자연히 생성되므로 `.gitkeep`을 넣지 않는다.
