# Cosmic Ascension — Claude Code 작업 규칙

## 프로젝트

UE 5.8.3, C++ 게임 모듈 `Cosmic_Ascension`. 1인 개발, 린 시뮬레이션: 3D 에셋·캐릭터 모델 참조 금지, 데이터는 DataTable(FTableRowBase), 식별자 FName, 분류 FGameplayTag, 표시 문자열 FText.
개발 PC(Windows): 프로젝트 루트 `C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension`(OneDrive 안 — 옮기면 이 줄과 docs/40_DEV_GUIDE.md의 경로를 함께 갱신), 엔진 `C:\Program Files\Epic Games\UE_5.8`. `.uproject`의 EngineAssociation은 GUID `{CCB28703-480F-382D-F8F7-E6A5E7850219}`이고 `"5.8"` 문자열로 바꾸지 않는다(이유와 다른 PC에서의 재연결은 docs/45_ENGINE_NOTES.md).
세계관 원천은 `docs/90_LORE/cosmic_species_codex.md`(개발자의 대도감 — 게임 데이터가 아니고 고치지 않는다).

## 문서 지도 (2026-10-18 재편 — 설계의 단일 기준은 docs/)

| 파일 | 내용 |
|---|---|
| `STATUS.md` | 현재 단계·다음 3작업·열린 결정 Top 5. 세션 시작에 읽고 끝에 갱신한다 |
| `docs/00_VISION.md` | 비전·정체성·코어 루프·MVP 범위 |
| `docs/10_DESIGN/` | 시스템 설계(pillars·crisis_and_transcendents·systems) |
| `docs/20_ARCHITECTURE.md` | 모듈 층·상태 모델·틱·난수·해시·저장 + 불변 규칙 전문 |
| `docs/21_DATA_MODEL_REFERENCE.md` | 타입·상태·정의 행·Subsystem 인터페이스 사본 — **코드가 원본**, 어긋나면 코드가 이긴다 |
| `docs/40_DEV_GUIDE.md` | 코드 규약·데이터/설정 관리·테스트·빌드/실행 명령 전문·협업 규칙 |
| `docs/45_ENGINE_NOTES.md` | UE 5.8.3 소스로 검증한 엔진 헤더 경로·UBT 사실 |
| `docs/50_ROADMAP.md` | 마일스톤(정리 → A0 → Playable MVP → 코어 시뮬 → 폴리싱)·현재 작업 표·Cut List·리스크 |
| `docs/60_DECISIONS/` | `OPEN_QUESTIONS.md`(미결 #번호 그대로), `DECISIONS_LOG.md`(확정 기록·ADR) |
| `docs/70_CONTENT_GUIDE.md` | 로그·템플릿 문법·서사 규칙·로컬라이제이션 |
| `docs/15_GLOSSARY.md` | 용어집(인게임 낱말 ↔ 코드 식별자) |
| `docs/90_LORE/` | 대도감 원문·신학 조율·대도감 배정표 |
| `docs/CHANGELOG.md` | 변경 이력·Phase 상태 기록·작업 일지·리뷰 통계(본문에 두지 않는다) |
| `docs/SECTION_MAP.md` | 옛 `guideline.md §N` → 새 문서 위치. 코드 주석의 `guideline.md section N`은 이 표로 푼다 |
| `docs/archive/` | `guideline_v0.8_2026-10-18.md`·`v08_rulings.md`·`v08_agent_directive.md`·`code_artifact.md` 동결본(고치지 않는다) |
| `WORKPLAN.md` | 세션 절차·effort·ultracode 기준(Phase별) |

문장 표지: `(v0.8 — 단계 N부터)`·`(v0.7 — …)`·`Phase N` 표지가 붙은 문장은 **아직 코드에 없는 예정**이고, 표지 없는 문장이 현행 규칙이다. 설계가 바뀌면 docs의 해당 문서 → 코드 → STATUS.md 순서로 고친다. 같은 문장을 두 문서에 적지 않고 링크한다.

## 아키텍처 불변 규칙 (현행 요약 — 전문·근거는 docs/20_ARCHITECTURE.md '불변 규칙 전문')

1. **레이어**: `Sim`·`Events`·`Ascension`·`Log`·`Player`·`Save`(Core Logic)와 `Core`·`Data`는 UUserWidget/AActor·`UI/`·`World/` 헤더를 include하지 않는다. 의존 방향 `Core ← Data ← Sim ← {Events, Ascension, Log, Player, Save} ← {World, UI}`.
2. **상태**: 모든 런타임 상태는 `FGalaxyState` 안의 USTRUCT(UObject 그래프 금지). Subsystem 멤버에 플레이어·생애 종속 상태를 두지 않는다. 상태는 세 층 — 세계 → 앎(`Knowing`) → 이야기(`Log`·`Chronicle`·`TellRngState`). 세계 코드는 앎을 `NoteMet`·`NoteKnown`으로 쓰기만 하고 읽지 않는다(명령은 입력의 정의로 기다리는 카드·만난 백성을 읽는다; 틱 안의 읽기는 앎에 쓰는 곳뿐).
3. **난수 출처 셋**: 시뮬 스트림 `UGalaxySimulationSubsystem::GetRng()`(세계를 바꾸는 모든 추첨), 말하기 스트림 `DrawTell()`(`PushVariants`의 문장 변형만), 파생 정수 `CosmicSeed::Derive`(상태 없음, 식·상수 동결). 서로 읽지 않는다. 추첨 지점은 행·뱅크·후보를 보기 전에 정해진 횟수를 무조건 뽑는다(테이블 내용이 추첨 수를 바꾸지 않는다). `FMath::Rand*` 금지.
4. **틱 열 단계**: 경제 → 연구 → 외교 → 분쟁 → 위기 → AI → 건넘 → 플레이어 이동 → 앎 → 역할 효과 해소. 이벤트 슬롯은 위기 단계 끝, 업적 판정은 AI 단계 끝, 기다리는 카드(청원·방랑자 조우·첫 접촉)의 제시는 앎 단계 끝(앎에만 쓰고 시뮬 스트림을 뽑지 않는다 — `Purpose::Petition`·`Encounter`; 답은 틱 밖 입력이고 받아들인 답마다 1회 뽑는다), 안정도 회복은 경제 단계의 인구 회복 바로 뒤, 첫 위기(`FirstCrisisTick`)는 위기 단계 발생의 끝(수도 규칙 뒤, 이벤트 슬롯 앞 — 추첨 없음), 경제·외교·분쟁 단계 안(B2·B3)과 B6b의 걸음(셈 둘·결박 완성·절멸 만료·AI 걸음 둘·제동 — 추첨 없음; 견딘 셈 밖은 `[B-6b2]`부터)은 docs/20 전문. 발화한 행은 '효과 전부 → 받은 성계의 앎 → 행의 줄 → 뒤따르는 줄 → `OnEventFired`'의 원자. `Check`·`Apply`는 static(세계만 읽고 쓴다). `Civs`의 원소 수는 건넘 단계에서만 바뀌고 `Systems`는 줄지 않는다. 열째 단계는 죽음도 처리한다(B5): 공백기에는 틱마다 남은 도약을 하나 줄이고 0인 틱에 계승한다 — 빨리 넘기기는 표현이 입력 시점에 부르는 `RunLeap`(끝에 경계 하나). 판의 끝(B6a)은 열째 단계 뒤 한 번 적힌다(`Ending` — 추첨 없음, 먼저 온 끝만); 시뮬은 끝 뒤에도 돌고 `RunLeap`만 그 틱에서 선다.
5. **핸들러**: `OnLogEntry`·`OnEventFired`·`OnTierAscended` 안에서는 상태 교체·틱 실행·평가기·명령·앎·로그 쓰기를 하지 않는다 — 그런 호출은 에러 한 줄 + 무동작(저장도 — `RequestSave`만 다음 틱 경계로 미뤄진다). 일시정지는 `SetTimeScale(0)` 하나(정지는 이월 시간도 버린다).
6. **해시**: `ComputeStateHash`는 FName·태그를 소문자 문자열로, `TMap`은 `FNameLexicalLess` 정렬 순서로. FText·`GalaxyID` 제외. `ComputeWorldHash` = 거기서 앎·로그·연대기(와 그 밀어냄 기록)·말하기 스트림 제외(앎이 세계를 바꾸지 않음을 재는 자). 해시에 드는 값은 모듈 안 코드의 + − × ÷ √만(Core Logic의 `FMath`·CRT 부름은 `check_layers.py` 규칙 3b의 목록만; `Build.cs`의 FP Precise 유지).
7. **저장**: 저장 필드의 기본 초기값은 세이브가 생긴 뒤 바꾸지 않는다(델타 저장). `SaveVersion` 초기값 리터럴 1, 현재 버전은 `FGalaxyState::CurrentSaveVersion`. 열거자는 끝에만 추가. 행 이름은 세이브에 남으므로(`SourceRow`·`ArchetypeRow`·`GalaxyRow` 등) 배포 뒤 바꾸지 않는다.
8. **데이터**: 원본은 `Tools/data/*.json`·`ST_CA.csv`, `.uasset`은 `Tools/make_data_tables.py`의 산출물(에디터에서 고친 행은 덮어써진다). 규칙은 `Tools/data/schema.json`, 검사는 `python -I Tools/check_data.py`와 테스트 `Data.Integrity`·`Data.SourcesInSync`(도장 `generated.sha1`). 컬럼·조건/효과 열거자·`CosmicRows::` 상수를 더하면 스키마와 `Tools/test_data_schema.py`에 함께 적는다. 코드가 이름으로 부르는 행은 `Data/CosmicRowNames.h` 상수로만. 행 이름은 테이블 접두어를 갖고 테이블을 가로질러 유일, `@` 금지. 로컬라이제이션 키는 위치 — 배열은 끝에만 추가, `DT_` 에셋을 지우고 다시 만들지 않는다.
9. **태그·Tier**: GameplayTag는 `Config/DefaultGameplayTags.ini`에 선언, `Domain.Sub.Leaf` 최대 3단계, `RequestGameplayTag(FName)`로 얻는다. 데이터 컬럼의 Tier는 1~5, `EAscensionTier` 값은 0~4(`Tier1_Planetary == 0`).
10. **소스**: ASCII만(주석 영문), 한국어는 DataTable/String Table에만. include는 모듈 루트 기준 `"Sim/GalaxySimulationSubsystem.h"`. `UE_LOG` 카테고리는 `LogCosmic` 하나. CommonUI 금지(UMG만). UI 라벨은 `FText::FromStringTable(…, FindOrFullyLoad)`.
11. **서사**: 거룩한 질서(하나님)는 엔티티·화자·수치·엔딩 조건이 아니다. 1인칭 로그 화자는 타락 초월자만. 서술자 문장에 판정 어휘(천국·지옥·구원받·멸망받·영혼·저주받·심판받) 금지(스키마 `words.verdict` — `Tools/test_data_schema.py`가 이 괄호의 일곱 낱말을 그대로 읽는다). 사람의 수는 어디에도 숫자로 쓰지 않는다(규모 등급만); 서술의 숫자는 주기·비율뿐, 자원·Stability 숫자는 UI에만; 천체의 수와 거리는 UI 라벨에만('약'·'추정'). 지어낸 이름은 한국어 묘사 이름(다른 작품·신화·성경 고유명·실존 인물 금지 — '이름의 거름'). 플레이어에게 구원자·메시아 등의 칭호와 '신이 되었다' 금지 — 받은 꼴만.
12. **요약 단위**: 인물은 `FNotableRecord`(5필드) 기록이고 능력치·AI·관계가 없다; 능력치를 가진 개인은 그릇 하나(`FPlayerEntityState`). 영혼·구원·멸망을 세는 필드·컬럼·업적은 없다(영혼 경제의 유일한 크기는 초월자 영향권). `Realm`·`Depth`는 정의(`FTranscendentDef`)이지 상태가 아니다. 칸(`EAscensionTier`)은 다섯 그대로(T0·T6·대체 등급 타입 없음). 초월자는 판에 교리마다 1체.
13. **세계 규칙의 단일 창구**: 정착된 성계 = 소유 ∧ `ActiveMegaproject != project_settle`(수입·남은 자 법·Hazard 추첨·인접 판정·국경이 이 술어를 쓴다). 무주 성계에는 Hazard·주장이 서지 않는다. 성역 `sol`(배열 끝, `Law.Hedge`, 술어 `CosmicState::IsSanctuary` — `Core/CosmicIDs.h`)은 소유·정착·주장·파괴·압력·그릇에서 빠진다(거절 자리는 docs/21 §7.4 '세계 함수'). 정착 가능 여부는 `CosmicMegaprojects::SettleBlockOf` 하나(첫 사유 `Sanctuary`), Hazard 배치는 `CosmicHazards::Place` 하나, 칙령 제정은 `CosmicEdicts::EnactBlockOf` 하나, 공성·지킴은 `CosmicConflict::SiegeBlockOf`·`GuardBlockOf`, 태세·조약은 `CosmicDiplomacy::StanceBlockOf`·`TreatyBlockOf`(B3), 카드 선택은 `CosmicPetitions::ChoiceGateOf`, 카드의 함대 시간은 `CosmicConflict::ResistBlockOf`이고 효과 `EnactEdict`는 `EnactBlockOf`를 지난다(B4), `KillPlayer`의 닿음은 `CosmicHazards::VesselAtRiskOf`(B5), 계승 후보는 `CosmicSuccession::CandidatesOf`(B5), 절멸 `CosmicHazards::LetPeopleGo`·결박 `CosmicSeal::BindTargetOf`·제동 `CosmicAscent::FourthRungOpenOf`·지킴 상한 `GuardDelayCapOf`(B6b — `[B-6b2]`부터), 엔딩·침묵은 `CosmicEndings::EndingOf`·`CosmicSuccession::IsSilent`(B6a), 주장 규칙은 `CosmicHazards::ClaimRuleHoldsOf`(화면의 답은 `ClaimMayFollowOf`)(B6a). `Relations`·`Ties`의 없는 키 = 중립. `DeposeRuler`·`KillPlayer`는 `Apply`를 거쳐 `PendingRoleEffect`로 기록되고 열째 단계에서 처리된다(폐위·양위·죽음).
14. **표현 계층의 앎 읽기**: 앎을 읽는 곳은 `CosmicMapLook::KnownViewOf` 하나(`FCosmicKnownView`) — 지도·성계 패널·선택·클릭·피드가 같은 답(`IsKnown`·`IsShownByName`·`IsShown`·`ShownOrderOf`·`OwnerShownOf`)을 읽고 앎에 쓰지 않는다. 그리기·고르기·선택의 문은 `IsShown`(= `IsKnown` ∨ `IsShownByName` — 성역은 이름과 별만), 주인을 알리는 답은 `IsKnown`만. 격차 규칙은 `UCosmicLogSubsystem::Tell`·`NoteMet`·`CosmicMapLook::IsShownPeople` 세 곳 — 하나를 고치면 셋을 함께 고친다(`[B-6b3]`부터 Core 술어 하나). 우주 화면은 같은 지도의 줌 단계(`World/CosmicZoomLevels`·`World/CosmicUniverseLook`; `DT_Galaxies` 20행) — 수·다리·영적 표지가 없다.
15. **예정(아직 코드에 없음 — 바꾸기 전에 docs/50_ROADMAP.md·OPEN_QUESTIONS 확인)**: 성역의 레인·실제 자리·실제 하늘·`DT_Planets`(단계 5), 서막·`ReceiveCalling`(단계 6), 첫 생애 행의 본문·이름 뱅크·금지어 목록(단계 7). **현재 코드**(A0~B6a — 블록마다 무엇이 코드에 들었는지, `sys_0`·`sol`의 자리 포함): docs/50_ROADMAP.md '현재 코드와 예정'. 헤더 `docs/60_DECISIONS/headers/`.

## 작업 방식

- 세션 시작: `git log --oneline -8` → `STATUS.md` → `WORKPLAN.md` §2의 현재 Phase 행에서 effort·ultracode 확인(`/effort status`). 세션 끝: `STATUS.md` 갱신(일지·리뷰 통계는 `docs/CHANGELOG.md`).
- 한 번에 클래스 하나. 저장 필드·열거형·틱 단계·결정론에 닿는 코드는 **헤더 먼저 제시 → 승인 → 소스**. UI·데이터 행·버그 수정은 바로 구현한다.
- 시뮬 코드는 `Tests/`의 자동화 테스트와 함께 쓴다. 새 단언은 변이로 한 번 깨뜨려 본다. 틱 순서·추첨 지점·저장 필드를 바꾸면 docs/20_ARCHITECTURE.md를 먼저 고친다.
- UE 5.8은 학습 데이터보다 새 버전이다. 낯선 엔진 API는 반드시 로컬 엔진 소스에서 선언 헤더를 확인한다(검증된 경로 목록: docs/45_ENGINE_NOTES.md).
- 새 UCLASS/USTRUCT/UENUM·헤더 변경 = 에디터를 닫고 전체 빌드. Live Coding(Ctrl+Alt+F11)은 .cpp 본문 변경에만.
- 커밋 = 컴파일되는 상태 + 자동화 테스트 2회 연속 통과. 제목은 한국어 요약에 작업 접두어(예 `[P3-12]`), 본문 끝에 `Co-Authored-By` 트레일러. 작업 표에는 날짜만 적고 '표에 커밋 해시를 적는' 별도 커밋은 만들지 않는다(`git log --grep '[P3-12]'`로 찾는다).
- 제안·리뷰·보고는 '추가 이야기와 보완'으로 끝낸다. 설계를 바꾸지 않는 작은 보완(툴팁·문구·표시)은 목록에 두지 않고 그 작업의 커밋에 바로 넣는다. 결정이 바뀌면 `docs/60_DECISIONS/DECISIONS_LOG.md`에 날짜·근거를 적고 OPEN_QUESTIONS의 행을 지운다.

## 빌드 / 실행 / 테스트 (요약 — 명령 전문과 PowerShell 5.1 주의는 docs/40_DEV_GUIDE.md '빌드·실행·테스트')

Windows, 에디터를 닫은 상태에서. 순서: 데이터 검사(`python -I Tools/check_data.py`, 에디터 불필요) → (JSON을 고쳤으면) `make_data_tables.py`로 에셋 재생성 → 에디터·게임 타깃 빌드(`Build.bat`) → 자동화 테스트 `Cosmic_Ascension.*`를 **두 번 연속**(`-nullrhi -unattended`) → `-game -nullrhi` 스모크 → 커맨드렛 `-run=CosmicBalance -Seed=42 -Ticks=3000` 기준값 비교 → 해시 판(`-CosmicHashRun=3000`) = 커맨드렛. 패키지와 ⓓ: `Tools\package.ps1`(에디터를 닫고). 러너: Windows `powershell -ExecutionPolicy Bypass -File Tools\run_checks.ps1 -Tier smoke|full`(위 순서 전부, `-Data`로 에셋 재생성), 에디터 없이 `Tools/run_checks.sh`(앞의 파이썬 검사 다섯 — 목록은 docs/40). 커밋 전에 하나는 돌린다.

## 모듈 / UHT / 컴파일 규칙 (5.8 UBT 소스 확인)

헤더 파일 이름은 **타깃 전체에서 유일**해야 한다(UHT가 모듈을 가로질러 검사). `Types.h`, `Utils.h` 같은 이름 금지. 항상 도메인 접두어(`CosmicTypes.h`).
`.generated.h`는 헤더의 마지막 include. `#pragma once` 필수.
include는 모듈 루트 기준(`"Sim/GalaxySimulationSubsystem.h"`). `"Cosmic_Ascension/Sim/..."`도 해석되지만 쓰지 않는다. 서브폴더는 자동 include 경로가 아니므로 다른 폴더의 헤더를 파일명만으로 include하지 않는다.
BuildSettingsVersion.V7 + IncludeOrder Unreal5_8: C++20, `UE_LOG` 포맷 문자열 컴파일 타임 검증, 미정의 식별자/반환 타입/dangling/unreachable 경고가 **에러**, 전이 include 의존 금지(쓰는 헤더는 직접 include).
UPROPERTY의 기본값 초기화식은 UHT가 `;`까지 건너뛴다(`EpicGames.UHT/Parsers/UhtPropertyParser.cs`의 `PropertyParsed`). `TArray<float> X = { 0.f, 1.f };`, `TMap<FName, float> M = { { TEXT("A"), 1.f } };` 같은 중괄호 초기화를 쓸 수 있다. Category 지정자는 엔진 모듈에서만 필수다.

## 현재 단계

`STATUS.md`를 본다. 지난 기록은 `docs/CHANGELOG.md`.
