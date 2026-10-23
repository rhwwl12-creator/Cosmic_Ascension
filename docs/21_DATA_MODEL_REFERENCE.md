# 데이터 모델·C++ 설계안 사본 (옛 §7)

> **코드가 원본이다.** 이 문서는 guideline.md v0.8 §7(공통 타입·런타임 상태·정의 데이터·시뮬레이션 Subsystem·인터페이스 요약)을 2026-10-19에 그대로 옮긴 사본이다. `Source/Cosmic_Ascension/Core`·`Data`·`Sim`·`Events`·`Log`·`Player`·`Save`의 헤더와 다르면 코드가 이긴다. 설계 결정(규칙)의 출처로만 읽고, 필드 목록·시그니처는 헤더를 본다. 예정 표지와 §번호의 뜻은 다른 문서와 같다(`docs/SECTION_MAP.md`).

## 7. 데이터 모델 및 C++ 설계안

**규칙**: 정의(Def)는 `FTableRowBase` 파생 USTRUCT로 DataTable에, 상태(State)는 일반 USTRUCT로 `FGalaxyState` 안에. 식별자는 `FName`, 분류·특성은 `FGameplayTag`. 사용자에게 보이는 문자열은 `FText`.

아래 코드는 설계 스케치다. 실제 구현 시 Claude Code에 "이 스케치를 기준으로 컴파일되는 헤더/소스 작성"을 요청한다.

### 7.1 공통 타입 (Core/CosmicTypes.h)

```cpp

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

#include "CosmicTypes.generated.h"

UENUM(BlueprintType)

enum class EAscensionTier : uint8

{

Tier1_Planetary UMETA(DisplayName = "Tier 1 - Planetary"),

Tier2_Stellar UMETA(DisplayName = "Tier 2 - Stellar"),

Tier3_Causal UMETA(DisplayName = "Tier 3 - Causal"),

Tier4_Transcendent UMETA(DisplayName = "Tier 4 - Transcendent"),

Tier5_EternalOrder UMETA(DisplayName = "Tier 5 - Eternal Order")

};

UENUM(BlueprintType)

enum class EPlayerRole : uint8 { Ruler, Wanderer };

UENUM(BlueprintType)

enum class ECosmicResource : uint8 { Energy, Matter, Insight, Influence, Causality };

// 규모 등급 (§1.3 요약 단위 원칙, §8.3). 파생값 — 저장하지 않고 UGalaxySimulationSubsystem::GetPopulationBand(§7.4)가 Tier 기준 + Population 보정으로 낸다. ST_CA 라벨 6: 소수·수백만·수십억·수천억·수조·셀 수 없는. '남은 자'는 등급이 아니다

UENUM(BlueprintType)

enum class EPopulationBand : uint8 { Few, Millions, Billions, HundredBillions, Trillions, Countless };

USTRUCT(BlueprintType)

struct FResourcePool

{

GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite) float Energy = 0.f;

UPROPERTY(EditAnywhere, BlueprintReadWrite) float Matter = 0.f;

UPROPERTY(EditAnywhere, BlueprintReadWrite) float Insight = 0.f;

UPROPERTY(EditAnywhere, BlueprintReadWrite) float Influence = 0.f;

UPROPERTY(EditAnywhere, BlueprintReadWrite) float Causality = 0.f; // Tier 3+

float Get(ECosmicResource R) const;

void Add(ECosmicResource R, float Amount);

};

USTRUCT(BlueprintType)

struct FEthicsAxes

{

GENERATED_BODY()

UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin=-1, ClampMax=1)) float Pacifism = 0.f; // -1 군사 ~ +1 평화

UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin=-1, ClampMax=1)) float Spiritualism = 0.f; // -1 물질 ~ +1 영성

UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin=-1, ClampMax=1)) float Openness = 0.f; // -1 폐쇄 ~ +1 개방

};

```

### 7.2 런타임 상태 (Core/GalaxyState.h)

```cpp

USTRUCT(BlueprintType)

struct FMegaprojectRecord // 한 성계에서 한 DT_Megaprojects 행의 마지막 완성 (§5.7). 성계가 주인을 바꾸거나 죽은 노드가 되어도 남는다. MegaprojectComplete가 읽는다

{

GENERATED_BODY()

UPROPERTY() FName Row;

UPROPERTY() int64 Tick = 0; // 그 행이 여기서 마지막으로 완성된 틱. 0보다 크다 — 틱 0에는 틱 단계가 돌지 않는다

};

// 성계 = 시뮬 노드. 은하의 네 층 가운데 생명 성계다(생명이 있거나 백성이 사는 별, §3.1). 배경 별과 풍경 은하는 그림이라 상태가 없다 (§5.11)

USTRUCT(BlueprintType)

struct FStarSystemState

{

GENERATED_BODY()

UPROPERTY(BlueprintReadOnly) FName SystemID; // 판 전체에서 유일하다. 성역(태양계, 다시 쓸 수 없는 법 — §4.5 ①)은 sol(배열 끝), 첫 생애의 모성계는 sys_0(배열 0번), 그 밖은 sys_<n>(배열 번호. 패딩 없음 — Phase 2 은하 생성 규칙 변경부터. 옛 형식은 sys_%03d) — A0 4a(2026-10-19)부터. 그 전의 세이브는 플레이어 모성계가 sol이고 로드에서 거절된다. ID 규칙은 아래 FGalaxyState 머리 주석

UPROPERTY(BlueprintReadOnly) FName GalaxyRow; // 이 성계가 속한 은하의 DT_Galaxies 행 이름. 초기값 NAME_None(동결) = '아직 찍히지 않음' — NewGalaxy가 고향 행 이름(gal_milkyway)을 찍는다. NAME_None을 고향으로 읽는 규칙은 없다 (§9.1 v0.7 예약. 도장은 Phase 2 은하 생성 규칙 변경부터)

UPROPERTY(BlueprintReadOnly) FText DisplayName;

UPROPERTY(BlueprintReadOnly) FVector Position = FVector::ZeroVector; // 뷰포트 배치용

UPROPERTY(BlueprintReadOnly) TArray<FName> Lanes; // 인접 성계. 같은 GalaxyRow의 성계만 들어간다(다리는 레인이 아니다, §5.9)

UPROPERTY(BlueprintReadOnly) FGameplayTag StarClass; // Star.Class.G 등. 빈 태그 = 파괴된 성계(죽은 노드)의 표지(§16 #29 ⑤ — 위기 처리 작업에서 정했다)

UPROPERTY(BlueprintReadOnly) float ResourceRichness = 1.f;

UPROPERTY(BlueprintReadOnly) FName OwnerCivID; // NAME_None = 무주 성계 (§3.1, §5.7)

UPROPERTY(BlueprintReadOnly) FGameplayTag ActiveHazard; // 없으면 빈 태그. 놓는 길은 CosmicHazards::Place 하나다 — 정착된 성계의 빈 슬롯에만 (§4.1)

UPROPERTY(BlueprintReadOnly) int32 HazardTicksRemaining = 0;

UPROPERTY(BlueprintReadOnly) FName ActiveMegaproject; // 건설 칸: Hazard에 답하지 않는 프로젝트(다이슨 스웜, 정착, 먼 항로). 정착된 성계 = 소유이고 이 값이 project_settle이 아닌 성계 (§5.7)

UPROPERTY(BlueprintReadOnly) float MegaprojectProgress = 0.f;

UPROPERTY(BlueprintReadOnly) int64 MegaprojectCompletedTick = 0; // 어느 행이든 가장 최근 완성의 틱. 0 = 아직 없음. 조건은 이 값이 아니라 CompletedMegaprojects를 읽는다 (§5.7)

UPROPERTY(BlueprintReadOnly) FName ResponseMegaproject; // 대응 칸: 이 성계의 Hazard 행이 RulerResolutions에 든 프로젝트. Hazard와 함께 살고 진다 — Hazard 슬롯을 비우는 모든 길이 이 칸도 비운다(CosmicHazards::EmptyHazardSlot). NAME_None = 없음 (§5.7, §16 #23)

UPROPERTY(BlueprintReadOnly) float ResponseProgress = 0.f;

UPROPERTY(BlueprintReadOnly) TArray<FMegaprojectRecord> CompletedMegaprojects; // 행마다 마지막 완성(처음 완성한 순서). MegaprojectComplete는 Key 행의 기록이 있고 그 틱이 0보다 크고 평가 문맥의 SinceTick 이상일 때만 참이다(§7.5 — 플레이어 업적 판정만 SinceTick에 Calling.BornTick을 넣고 그 밖은 0이다. 업적 조건의 생애 상대화, §4.4)

// (v0.8 — 단계 5: 관측 목록의 행 이름을 담는 필드 하나 — 이름은 단계 5 헤더. ID는 바꾸지 않는다)

// Phase 6: FName EpithetRow; 연대기가 붙이는 성계의 두 번째 이름 (§5.11)

};

// 인물 기록 (§1.3 요약 단위 원칙). FName·정수·태그만, 5필드 고정. 사건이 만들고(효과 EEffectType::NameNotable을 실은 행 + 코드 훅 3, §7.3·§7.4·§7.5) 사건이 닫는다. 능력치·AI·관계 없음, 개인을 겨누는(ID를 받는) 조건/효과 없음(문명을 대상으로 기록 1개를 만드는 NameNotable과 역할 유무만 읽는 CivHasNotable이 전부) (§12.2). 플레이어 그릇은 이 구조체가 아니다(FLifeRecord.NameIndex). 인물을 만드는 행은 Severity 2 사건 또는 Severity 2 직후의 카드다 — crisis_rebellion 전조 ②(Severity 1)만 예외(§4.1)

USTRUCT(BlueprintType)

struct FNotableRecord

{

GENERATED_BODY()

UPROPERTY() FGameplayTag Role; // Notable.Ruler / Regent / Rebel / Loyalist / Hearer / Ascendant (ini 선언, 2단계). 기능어이지 칭호가 아니다 — 별칭·칭호는 사람에게 붙지 않는다 (§5.11, §8.3)

UPROPERTY() int32 NameIndex = 0; // 소유 문명 아키타입 PersonNameBank의 인덱스 (§7.3). FText 저장 금지, 표시 시점에 Bank[NameIndex % Bank.Num()]으로 렌더

UPROPERTY() int64 FirstTick = 0; // 등장 틱 = 만든 사건의 틱

UPROPERTY() int64 LastTick = 0; // 퇴장 틱. 0 = 열린 기록. 등장·퇴장 틱이며 생몰이 아니다 — 게임은 인물의 죽음·행방을 말하지 않는다 (§8.3 사후 문장 규칙 ⑥)

UPROPERTY() FName SourceRow; // 만든 행(이벤트·Hazard·조우 행. Ascendant 훅은 DT_Feats 행, 자진 이탈 Regent는 DT_LogTemplates 행 log_throne_vacated — §7.5). 테스트 Sim.NotablesFromEventsOnly의 근거 (§9.3)

// 6번째 필드는 §12.3 '인물 3문'을 통과해야 한다

};

USTRUCT(BlueprintType)

struct FCivilizationState

{

GENERATED_BODY()

UPROPERTY(BlueprintReadOnly) FName CivID; // 판 전체에서 유일하다. civ_<n>(패딩 없음) — Phase 2 은하 생성 규칙 변경부터 배열 인덱스가 아니라 FGalaxyState.NextCivSerial에서 받고 재사용하지 않는다(옛 형식은 civ_%02d)

UPROPERTY(BlueprintReadOnly) FName GalaxyRow; // 이 문명이 속한 은하의 DT_Galaxies 행 이름. 초기값 NAME_None(동결) = '아직 찍히지 않음' — NewGalaxy가 고향 행 이름을 찍는다. 소유 성계의 GalaxyRow와 언제나 같다 (§9.1 v0.7 예약. 도장은 Phase 2 은하 생성 규칙 변경부터)

UPROPERTY(BlueprintReadOnly) FName OriginGalaxyRow; // 건너온 백성만 값을 가진다(§5.7 건넘). NAME_None = 토박이 — NAME_None이 뜻을 갖는 곳은 이 필드뿐이다 (§9.1 v0.7 예약. 기록은 Phase 6a)

UPROPERTY(BlueprintReadOnly) FName HomeSystemID; // 수도. '행성' = 모성계 1개. Hazard.Rebellion의 대상 성계 (§5.10)

UPROPERTY(BlueprintReadOnly) FName ArchetypeRow; // DT_Civs 행 이름 (§5.11). 이름·형용사·헌정명 뱅크, 계시 양식 RevelationMode, 청원 화자 분기가 읽는다. 없으면 arch_default 뱅크로 폴백 (§9.1 Phase 1 예약)

UPROPERTY(BlueprintReadOnly) FText CivName;

UPROPERTY(BlueprintReadOnly) FLinearColor Color = FLinearColor::White;

UPROPERTY(BlueprintReadOnly) EAscensionTier Tier = EAscensionTier::Tier1_Planetary;

UPROPERTY(BlueprintReadOnly) FEthicsAxes Ethics;

UPROPERTY(BlueprintReadOnly) FGameplayTagContainer Traits;

UPROPERTY(BlueprintReadOnly) FResourcePool Stockpile;

UPROPERTY(BlueprintReadOnly) FResourcePool IncomePerTick; // 캐시, 매 틱 재계산

UPROPERTY(BlueprintReadOnly) float Population = 1.f; // 추상 비율(1.0 = 그 Tier의 전형). 문명 단위 유지(성계 단위 인구 없음, §12.2). 영혼이 아니고 머릿수가 아니다 (§1.3, §8.3). ChangePopulation 효과가 비율로 바꾼다. 표시는 GetPopulationBand 등급만 — UI·로그·연대기에 숫자 없음

UPROPERTY(BlueprintReadOnly) TMap<FName, float> Research; // "Physics" 등 -> 0~1

UPROPERTY(BlueprintReadOnly) TMap<FName, float> ResearchFocus; // 합계 1.0

UPROPERTY(BlueprintReadOnly) float FleetPower = 0.f;

UPROPERTY(BlueprintReadOnly) float Stability = 0.75f; // 0~1. 통치자의 유일한 지표. 불만도는 Hazard.Rebellion 카운트다운 (§5.10)

UPROPERTY(BlueprintReadOnly) int32 LowStabilityTicks = 0; // Stability < 0.3 연속 틱. TickEconomy 끝에서 갱신, CivStabilityBelowFor가 읽는다. 반란이 슬롯을 떠나면(해결·만료·넘김) 0으로 다시 센다 (§4.1)

UPROPERTY(BlueprintReadOnly) int32 FallenSphereTicks = 0; // 정착된 성계 가운데 하나라도 봉인되지 않은 타락 초월자(행의 Allegiance = Realm.Abyssal)의 영향권 안에 있는 연속 틱. 인접은 들지 않고, 행이 없는 인스턴스와 정착 중인 성계도 들지 않는다. TickEconomy 끝에서 LowStabilityTicks와 함께 갱신, CivFallenSphereTicksReached가 읽는다 (§5.11, §16 #25 ①)

UPROPERTY(BlueprintReadOnly) TArray<FName> OwnedSystems;

UPROPERTY(BlueprintReadOnly) TMap<FName, float> Relations; // Key = CivID 또는 TranscendentID -> -1~1 (§4.2). 별도 SpiritualFavor 미터 없음. 항목은 처음 쓸 때 생긴다. 없는 키 = 0 = 중립(§5.5)

UPROPERTY(BlueprintReadOnly) TArray<FName> ActiveEdicts;

UPROPERTY(BlueprintReadOnly) FGameplayTag AIProfile; // AI.Profile.Expansionist 등

UPROPERTY(BlueprintReadOnly) float ReverenceToPlayer = 0.f; // 0~1, 방랑자→군주 전환 조건

UPROPERTY(BlueprintReadOnly) bool bIsPlayerRuled = false;

UPROPERTY(BlueprintReadOnly) int64 ThroneLockedUntilTick = 0; // 폐위 후 Tick + 600. 즉위·복귀 조건 Tick >= 이 값 (§5.10)

UPROPERTY(BlueprintReadOnly) TArray<FNotableRecord> Notables; // 역사에 흔적을 남긴 소수의 기록(§1.3). 상한 DT_Rules.MaxNotablesPerCiv(4). 역할당 열린 기록(LastTick==0) 1개. 능력치·AI·관계 없음, 개인을 겨누는(ID를 받는) 조건/효과 없음(문명을 대상으로 기록 1개를 만드는 NameNotable과 역할 유무만 읽는 CivHasNotable이 전부, §12.2). 플레이어 그릇은 넣지 않는다(FLifeRecord.NameIndex). NewGalaxy 시드 없음 — 인물을 만드는 사건(§7.3 NameNotable 행·코드 훅 3; Rebel만 Severity 1 예외)을 겪지 않은 문명은 비어 있다. 해시 포함 (§9.2)

};

// 그릇(육신). 계승 시 전부 초기화 (§4.4 3분할 표 '그릇' 열). PersonalTier·Powers·CompletedFeats는 부름(FCallingState)에 있다

USTRUCT(BlueprintType)

struct FPlayerEntityState

{

GENERATED_BODY()

UPROPERTY(BlueprintReadOnly) EPlayerRole Role = EPlayerRole::Ruler; // 첫 생애는 통치자 (§5.10). Succession.Called 취임은 항상 Wanderer(귀환 Succession.Returned은 사망 당시 Role)

UPROPERTY(BlueprintReadOnly) FName CurrentSystemID;

UPROPERTY(BlueprintReadOnly) FName TravelTargetSystemID;

UPROPERTY(BlueprintReadOnly) int32 TravelTicksRemaining = 0;

UPROPERTY(BlueprintReadOnly) FName RuledCivID; // Ruler일 때만

UPROPERTY(BlueprintReadOnly) FName FormerCivID; // 떠난 제국 — 자진 이탈·폐위·사망 모두 BecomeWanderer가 기록. 복귀(ReturnToFormerCiv), 재회 조우의 CivHasNotable(Key = Former), NameNotable(Key = Former, Notable.Loyalist)이 읽는다 (§5.10, §7.3)

UPROPERTY(BlueprintReadOnly) int32 Body = 1, Mind = 1, Causality = 0; // 0~10. 출생 문명 Traits의 BodyMod/MindMod/CausalityMod로 초기화 (§5.9)

UPROPERTY(BlueprintReadOnly) int32 Exaltation = 0; // 0~10. 높아짐. 경배 수용 +1~2, 거부 -1, 귀환 수락 +2 / 거절 -1, T5 문턱 기다림 -1 (§5.9, §4.4). 경외/공포 분기(§3.2)·느부갓네살 트리거(§8.6)·받는 길 문턱 게이트(<= 3 열림, >= 7 닫힘)가 읽는다

UPROPERTY(BlueprintReadOnly) FResourcePool PersonalPool;

UPROPERTY(BlueprintReadOnly) TArray<FName> Relics; // 최대 3

UPROPERTY(BlueprintReadOnly) FGameplayTagContainer StatusTags; // Status.FallenMonarch / Status.Abdicated / Status.Returned(귀환 수락, §4.4). 조건 PlayerHasStatus (§5.10)

};

USTRUCT(BlueprintType)

struct FTranscendentState

{

GENERATED_BODY()

UPROPERTY(BlueprintReadOnly) FName TranscendentID; // == DT_Transcendents 행 이름 (의미 고정, §4.2). 소속 Realm.*은 상태가 아니라 정의 FTranscendentDef.Allegiance에 있다 — 이 구조체에 Realm 필드를 두지 않는다. 초월자는 판에 교리마다 1체다(은하마다 따로 두지 않는다. 타락은 건너지 않는다, §4.2)

UPROPERTY(BlueprintReadOnly) FGameplayTag Doctrine; // Doctrine.Silence / Assimilation / Zeal / Recurrence / Stillness / Release(독립, Phase 6+). 상태에 저장되므로 이름은 지금 확정 (§9.1)

UPROPERTY(BlueprintReadOnly) TArray<FName> InfluencedSystems; // 영향권. Hazard.Claim 만료가 성계를 여기에 편입한다 (§4.2 영혼 경제). 상한 FTranscendentDef.MaxInfluencedSystems(개체당 판 전체의 합 — 은하가 늘어도 같다, §4.2). Realm.Celestial 행은 상태 인스턴스를 만들지 않는다

UPROPERTY(BlueprintReadOnly) bool bAwareOfPlayer = false; // 생애를 넘어 유지. "같은 부름의 냄새가 난다" (§4.4)

UPROPERTY(BlueprintReadOnly) bool bSealed = false;

UPROPERTY(BlueprintReadOnly) int64 SealedTick = 0; // 봉인 틱. TranscendentSealed 조건은 bSealed이고 이 값이 평가 문맥의 SinceTick 이상일 때만 참이다(§7.5 — 플레이어 업적 판정만 Calling.BornTick, 그 밖은 0. §4.4)

};

// 부름과 계승 (§4.4). 필드는 FName과 숫자만. 문장은 표시 시점에 템플릿으로 렌더한다

UENUM(BlueprintType)

enum class ELifeState : uint8 { Embodied, Interregnum };

USTRUCT(BlueprintType)

struct FLifeRecord

{

GENERATED_BODY()

UPROPERTY() int32 LifeIndex = 0;

UPROPERTY() int64 BornTick = 0;

UPROPERTY() int64 DiedTick = 0; // TicksLived = DiedTick - BornTick

UPROPERTY() FName BirthSystemID;

UPROPERTY() FName BirthCivID; // 출생 은하 필드는 더하지 않는다 — 계승은 은하를 넘지 않고(§4.4) 나중 추가는 호환이다(§9.1)

UPROPERTY() FName BirthArchetypeRow; // 출생 문명의 DT_Civs 행. Install()이 그 문명의 ArchetypeRow를 복사한다 — 출생 문명이 제거되어도(소유 성계 0 — 건넘 단계의 정리, §7.4) {Player}의 이름 뱅크가 풀린다(§8.2). 귀환(Succession.Returned)은 직전 생애 값을 유지. 해시 포함

UPROPERTY() int32 NameIndex = 0; // 그릇의 이름: 출생 문명 아키타입 PersonNameBank의 인덱스 (§4.4, §7.3). Install()이 GetRng()로 추첨 — Succession.Called는 새 인덱스(같은 문명이면 직전 생애와 다른 값), Succession.Returned는 직전 생애와 같은 인덱스("같은 이름이 다시 적혔다"). 출생 문명이 이름 없는 문명이면 arch_default 뱅크. 슬롯 {Player}가 렌더. 해시 포함. 플레이어는 FCivilizationState.Notables에 들어가지 않는다

UPROPERTY() TArray<FName> RuledCivIDs; // 재위 3배열(RuledCivIDs·ReignEndReasons·ReignTicks)은 항상 같은 길이다. 즉위(Install의 Ruler·BecomeRuler·ReturnToFormerCiv)가 세 배열에 한 칸씩 함께 더한다

UPROPERTY() TArray<FGameplayTag> ReignEndReasons; // RuledCivIDs와 같은 순서. Reign.Abdicated / Reign.Deposed / Reign.Died (§5.10). 열린 재위(지금 통치 중)는 빈 태그

UPROPERTY() EAscensionTier PeakTier = EAscensionTier::Tier1_Planetary; // 생애의 가장 높은 칸 — 출생 때의 개인 Tier, 통치하는 백성이 오르면 업적 판정이 올린다(§16 #67 ①)

UPROPERTY() FGameplayTag CauseOfDeath; // Death.Hazard / Death.Encounter / Death.Age / Death.Transcendent

UPROPERTY() FName CauseSourceRow; // 사인을 만든 Hazard/조우/이벤트 행. 사인 추적률의 근거

UPROPERTY() TArray<FName> Feats;

UPROPERTY() TArray<FName> TranscendentsMet;

UPROPERTY() TArray<FName> LastChoiceRows; // 최근 3개 조우·청원 행 (비문). 답한 행을 끝에 붙이고 셋을 넘으면 맨 앞을 지운다(CosmicPetitions::Remember, MVP ②). 청원의 주기 슬롯은 이 행들을 다시 내놓지 않는다. 비문이 읽는 것은 Phase 4b

UPROPERTY() FName EpithetRow; // 시대 명명 (§5.10)

UPROPERTY() TArray<FName> MegaprojectsCompleted;

UPROPERTY() TArray<int64> ReignTicks; // RuledCivIDs와 같은 순서. 통치 중 매 틱 1씩 느는 누계(TickPlayerTravel 끝, §7.4) — 시대 명명 600틱·Trait.Legacy.Enduring 1,800틱이 읽는다 (§5.10)

UPROPERTY() FGameplayTag SuccessionKind; // 이 생애가 시작된 방식: Succession.Installed(첫 생애) / Succession.Called(계승 추첨) / Succession.Returned(귀환 제안 수락, §4.4 회귀의 유혹). 해시 포함

UPROPERTY() bool bReturnOffered = false; // 이 생애의 끝에 귀환 제안이 제시되었는가 (생애당 1회 게이트, §4.4). 해시 포함

};

USTRUCT(BlueprintType)

struct FCallingState

{

GENERATED_BODY()

UPROPERTY() int32 LifeIndex = 1; // 제 N대

UPROPERTY() int64 BornTick = 0; // 현재 그릇의 출생 틱. 출생 보호 300틱과 업적 조건 상대화의 기준

UPROPERTY() ELifeState LifeState = ELifeState::Embodied;

UPROPERTY() int64 PendingLeapTicks = 0; // 공백기 중 저장·로드 재개용

UPROPERTY() int64 LeapTicksTotal = 0; // 판 합계. 다음 도약 폭 = clamp(Rng.RandRange(LeapTicksMin, LeapTicksMax), 0, LeapTotalCap - 이 값). 0이면 즉시 계승 (§4.4)

UPROPERTY() TArray<FLifeRecord> Lives;

// 부름 열(§4.4 3분할 표). 계승 시 PersonalTier는 T≤2 → Tier1_Planetary, T≥3 유지; Powers는 Tier에서 재부여(통치로 받은 권능은 §16 #67 ②); CompletedFeats는 오른 순서의 기록

UPROPERTY(BlueprintReadOnly) EAscensionTier PersonalTier = EAscensionTier::Tier1_Planetary;

UPROPERTY(BlueprintReadOnly) FGameplayTagContainer Powers; // 사망 시 Tier에서 재부여 (§4.4. 통치자 업적이 준 권능은 §16 #67 ②)

UPROPERTY(BlueprintReadOnly) TArray<FName> CompletedFeats;

// PendingRoleEffect (§7.4 '역할 효과 해소'): 위기 단계·분쟁 단계(통치 문명의 마지막 성계를 앗아 갈 때)·이벤트 효과·조우 해결은 아래 3필드에 기록만 한다. 빈 PendingEffect = 없음 기록은 UCosmicEventSubsystem::Apply를 거치고 Embodied일 때만 한다(§7.5)

UPROPERTY() FGameplayTag PendingEffect; // 태그 Effect.KillPlayer 또는 Effect.DeposeRuler(ini에 선언. EEffectType::KillPlayer·DeposeRuler와 1:1), 틱당 최대 1건, 먼저 기록된 것 우선

UPROPERTY() FGameplayTag PendingCause; // Death.* 또는 Reign.*

UPROPERTY() FName PendingSourceRow;

};

// 알게 된 것 하나와 그 틱. 병렬 배열 대신 쓴다. 해시 포함 (§9.1 v0.7 예약)

USTRUCT(BlueprintType)

struct FKnownMark

{

GENERATED_BODY()

UPROPERTY() FName ID; // SystemID, CivID 또는 DT_Galaxies 행 이름

UPROPERTY() int64 Tick = 0; // 알게 된 틱

};

// 앎 — 부름이 알게 된 것의 기록 (§8.1 세 층: 세계 → 앎 → 이야기). 저장되고 전체 해시에 들지만 세계 상태가 아니다: 세계는 앎을 쓰기만 하고 읽지 않는다(전문과 예외 하나는 §8.1). 배열은 알게 된 순서이고 줄지 않는다 — TMap 없음, 술어 태그 없음. 부름의 것이라 사망·폐위·자진 이탈·계승으로 지워지지 않는다 (§4.4. §9.1 v0.7 예약)

USTRUCT(BlueprintType)

struct FKnowingState

{

GENERATED_BODY()

UPROPERTY() TArray<FKnownMark> KnownSystems; // 아는 성계(ID = SystemID). 한번 알면 계속 안다 (§7.4 앎 단계, §8.1)

UPROPERTY() TArray<FKnownMark> MetCivs; // 만난 백성(ID = CivID). Install이 줄 없이 적는 출생 문명도 여기 든다 (§4.4, §7.4)

UPROPERTY() TArray<FKnownMark> MetGalaxies; // 만난 은하(ID = DT_Galaxies 행 이름). 사용은 Phase 6a (§7.4)

UPROPERTY() FName PendingContactCivID; // 대기 중인 첫 접촉 카드의 백성. NAME_None = 대기 카드 없음. 사용은 Phase 4 (§8.1)

UPROPERTY() FName FirstContactCivID; // 첫 접촉의 기록. 초기값 NAME_None(동결) = 이 판에 아직 첫 접촉이 없다. NoteMet이 첫 접촉 때 한 번 적고 그 뒤로 바뀌지 않는다. Install은 적지 않는다. 사용은 Phase 2 (정의는 §7.4 앎 단계)

UPROPERTY() int64 FarLineUntilTick = 0; // log_far_event의 다음 허용 틱. log_far_ascent에는 걸리지 않는다 (§8.1)

UPROPERTY() FName PendingPetitionRow; // 플레이어의 답을 기다리는 청원 카드(DT_Encounters 행, MVP ②). NAME_None(동결 초기값) = 카드 없음. 앎 단계의 끝(CosmicPetitions::Present)과 사슬을 잇는 답이 적고, 답·Install·BecomeWanderer가 비운다(다스리는 백성은 다스리는 동안 지워지지 않는다). 틱 단계는 읽지 않고 답 명령만 읽는다 — 입력의 정의 (§5.10)

UPROPERTY() FName PendingPetitionSystemID; // 그 카드가 다루는 성계: SettledSystem 행이면 다스리는 백성의 정착된 성계, Civilization 행(백성 — 현재 성계는 수도)과 카드가 없을 때는 NAME_None

};

// 건넘 한 번의 기록 (§5.7 건넘). 건너는 것은 유닛이 아니라 기록이다. FText 없음. 지우지 않는다 — 배열 순서가 해시에 든다. 본문은 Phase 6a이고 그 전까지 FGalaxyState.Transits는 빈 배열이다 (§9.1 v0.7 예약)

USTRUCT(BlueprintType)

struct FTransitRecord

{

GENERATED_BODY()

UPROPERTY() FGameplayTag Kind; // Transit.People (태그의 ini 선언은 Phase 6a)

UPROPERTY() FName FromGalaxyRow; // 출발 은하의 DT_Galaxies 행 이름

UPROPERTY() FName ToGalaxyRow; // 목적지 은하의 DT_Galaxies 행 이름

UPROPERTY() int64 DepartTick = 0;

UPROPERTY() int64 ArriveTick = 0; // DepartTick + BridgeTicks (§5.9 다리)

UPROPERTY() FName SourceCivID; // 보낸 백성. 요약 은하에서 오는 건넘은 빈 값이고, 제거된 문명을 가리킬 수 있다 (§5.7, §7.4)

UPROPERTY() FName ArchetypeRow; // 건너간 무리의 DT_Civs 행 (§5.7)

UPROPERTY() FName ResultCivID; // 도착해 생긴 문명의 CivID. 빈 값 = 건너는 중이거나 내리지 못함

};

// 판의 상태 = 고향 은하 상세 + 판 공통. 이름은 그대로 둔다 — 코드 식별자의 Galaxy(FGalaxyState·GalaxyID·NewGalaxy·슬롯 Galaxy_<GUID>·L_Galaxy)는 판을 가리키고 개명하지 않는다(§15). 고향 밖의 은하는 저장 상태가 없다(무대 은하의 요약은 순수 함수, 풍경은 그림 — §7.4, §5.11)

// - 고향 은하의 멤버: Systems·Civs — 원소마다 GalaxyRow를 가진다

// - 판 공통 멤버: 그 밖의 전부 — SaveVersion·Seed·Tick·RngState·TellRngState·GalaxyID·NextCivSerial·Transcendents·Player·Calling·Knowing·Transits·EventCooldownUntilTick·Log·Chronicle·Flags. 시계는 Tick 하나다(은하별 시계 없음)

// 저장: UCosmicSaveGame, 슬롯 이름, SaveVersion 초기값 1, CurrentSaveVersion은 그대로다. 새 저장 필드는 전부 기본값이 '없음'인 추가이고 그 초기값은 첫 세이브부터 동결된다(§9.1). 슬롯 신원(GalaxyID)과 스트림 값(RngState·TellRngState)은 FGalaxyState 최상위에만 둔다 — 해시 제외가 최상위 이름 기준이므로 중첩 구조체에 두지 않는다(§9.2). Sim.SaveRoundTrip의 FillEmptyContainers는 손으로 쓴 목록이므로 예약 커밋에서 새 컨테이너(Knowing의 세 배열과 Transits)를 채우는 줄을 더한다(§9.3)

// 은하 소속의 부호화는 하나다: NewGalaxy가 고향 행 이름(DT_Rules.HomeGalaxyRow, gal_milkyway)을 모든 성계와 문명의 GalaxyRow에 찍는다. 멤버 초기값 NAME_None은 동결이고 '아직 찍히지 않음'을 뜻한다 — NAME_None을 고향으로 읽는 규칙은 없다. NAME_None이 뜻을 갖는 곳은 OriginGalaxyRow(토박이)뿐이다. Transits의 FromGalaxyRow·ToGalaxyRow, Knowing.MetGalaxies, CosmicSeed::Derive의 이름, FCosmicEvalContext.GalaxyRow가 모두 같은 행 이름을 쓰므로 비교가 섞이지 않는다

// ID: SystemID·CivID·Relations의 키·EventCooldownUntilTick의 키는 판 전체에서 유일하다. Phase 2의 은하 생성 규칙 변경부터 ID는 패딩 없는 번호를 쓴다 — 성계 sys_<n>(성역은 sol(배열 끝), 첫 생애의 모성계는 sys_0(배열 0번), 그 밖은 sys_<n>(배열 번호) — A0 4a부터. 그 전의 세이브는 플레이어 모성계가 sol), 문명 civ_<n>. CivID는 배열 인덱스가 아니라 NextCivSerial에서 받고 재사용하지 않는다. FName은 끝의 _숫자를 번호부로 가르므로 'ID 사전순'이라고 쓰는 모든 동률 기준은 FNameLexicalLess로 정의한다(패딩 없는 번호 이름은 번호순으로 놓인다 — sys_0은 다른 sys_*보다 앞이다)

// 레인을 세는 모든 규칙은 같은 GalaxyRow 안이다(§5.9 다리). Phase 2의 평가 문맥 FCosmicEvalContext는 처음부터 GalaxyRow와 스트림을 갖고, Check·Apply·EvaluateAndFire는 문맥이 건넨 상태와 스트림만 쓴다(§7.5, §9.2)

// 상세로 도는 은하를 둘 이상 두는 일(은하별 상태와 스트림)은 Phase 6b의 후보이고 지금은 아무것도 예약하지 않는다(§16 #47). 그 비용을 뒤로 미루는 근거는 'VS의 사다리는 T3에서 끝난다'이다

USTRUCT(BlueprintType)

struct FGalaxyState

{

GENERATED_BODY()

UPROPERTY() int32 SaveVersion = 1; // 초기값 1 고정 = 버전 태그가 없는 세이브가 읽히는 버전. 현재 버전은 static constexpr int32 CurrentSaveVersion을 NewGalaxy와 마이그레이션이 찍는다. 초기값을 그 상수에 묶지 않는다 (§9.1)

UPROPERTY() int32 Seed = 0; // 우주 시드. 판에 하나이고 플레이어가 넣는 시드는 이것뿐이다 — 은하별 시드는 저장하지 않는다(CosmicSeed::Derive로 파생, §9.2)

UPROPERTY() int64 Tick = 0;

UPROPERTY() int32 RngState = 0; // 시뮬 스트림(GetRng())의 FRandomStream 복원용 (§9.2)

UPROPERTY() int32 TellRngState = 0; // 말하기 스트림의 상태(§9.2). 초기값 0은 동결이고 NewGalaxy가 (int32)CosmicSeed::Derive(Seed, "", Tell, 0)을 찍는다. 살아 있는 스트림 객체가 없어 저장 값이 항상 최신이다 — DrawTell이 이 값으로 지역 스트림을 열어 한 번 뽑고 되적는다. 세계 해시 제외 (§9.1 v0.7 예약)

UPROPERTY() FGuid GalaxyID; // 판당 저장 슬롯 1개의 키(판의 신원) (§9.1)

UPROPERTY() int32 NextCivSerial = 0; // Phase 2 은하 생성 규칙 변경부터 모든 CivID(civ_<n>)를 여기서 받는다. 재사용 없음 (§9.1 v0.7 예약)

UPROPERTY() TArray<FStarSystemState> Systems; // 고향 은하의 멤버. 판 도중에 줄지 않는다 — 파괴된 성계는 지우지 않고 죽은 노드로 남긴다(주인 없음, 레인 없음. 표지는 StarClass 빈 태그 — §16 #29 ⑤, 위기 처리 작업에서 정했다. 파괴는 레인을 양쪽에서 끊고 모든 영향권에서 뺀다). 죽은 노드는 생명 성계가 아니어서 생명 성계를 훑는 모든 규칙이 건너뛰고, 아는 성계가 파괴되면 희미한 죽은 별 표시로 남는다 (§6.4)

UPROPERTY() TArray<FCivilizationState> Civs; // 고향 은하의 멤버. 원소 수는 건넘 단계에서만 바뀐다(§7.4). 문명을 제거할 때는 다른 모든 문명의 Relations에서 그 CivID 키를 지우고, Knowing.PendingContactCivID가 그 문명이면 비운다. Knowing.MetCivs·Knowing.FirstContactCivID·Transits·Lives는 그 ID를 그대로 둔다 — 조회가 실패하면 '사라진 것'이고 표시는 그것을 뺀다(Sim.NoDanglingRefsAfterLeap의 예외, §9.3). FirstContactCivID는 판의 기록이라 그 백성이 사라져도 비우지 않는다(비우면 다음 만남이 다시 첫 접촉이 된다, §7.4)

UPROPERTY() TArray<FTranscendentState> Transcendents;

UPROPERTY() FPlayerEntityState Player; // 그릇

UPROPERTY() FCallingState Calling; // 부름 (§4.4). ComputeStateHash에 포함

UPROPERTY() FKnowingState Knowing; // 앎 (§8.1). 상태 최상위에 둔다. 부름 열 — 계승에도 유지된다(§4.4). 전체 해시(ComputeStateHash) 포함, 세계 해시(ComputeWorldHash) 제외 (§9.1 v0.7 예약)

UPROPERTY() TArray<FTransitRecord> Transits; // 건넘의 기록, 생성 순 (§5.7). Phase 6a 전까지 빈 배열 (§9.1 v0.7 예약)

UPROPERTY() TMap<FName, int64> EventCooldownUntilTick; // 키 = UCosmicEventSubsystem::CooldownKey — "<DT_Events 행>@<CivID>"(슬롯 행의 문명별 쿨다운), 또는 주장 줄의 스팸 방지 키 log_claim_<row>(§4.2). 키는 저장되므로 꼴을 고정한다(행 이름에 '@'를 쓰지 않는다 — Data.Integrity). 이름으로 내는 행(Named)은 키를 쓰지 않는다. 끝난 항목은 600틱마다 지운다(끝난 항목과 없는 항목은 뜻이 같다 — 제거된 문명의 키도 그렇게 사라진다). 순회로 결과를 내지 않는다

UPROPERTY() TArray<FCosmicLogEntry> Log; // 상한 유지 (§8.4)

UPROPERTY() TArray<FCosmicLogEntry> Chronicle; // Log 상한(2,000) 초과 시 Severity 1·2 이관. BuildChronicle 입력 — CosmicChronicle::Build는 이 배열 다음 Log를 읽는다(MVP ④: 게임 길이의 판에서는 이관이 일어나지 않는다). 생애별 장은 FLifeRecord.BornTick/DiedTick 범위 — BornTick(첫 장은 그 앞의 모든 틱도)부터 DiedTick, 없으면 다음 생애의 BornTick − 1, 마지막 장은 지금 틱까지 (§8.4)
// (B6 — 예정) 엔딩의 저장 기록은 그 헤더가 정한다(스케치는 §16 #117). MVP의 임시 끝은 저장하지 않는다 — Tick과 모든 생애의 PeakTier에서 다시 읽힌다 (§9.1)

UPROPERTY() FGameplayTagContainer Flags; // SetFlag 효과의 저장소. Flag.Grace.Spent, 사자 예언 플래그 (§4.5). ComputeStateHash 포함

};

```

### 7.3 정의 데이터 (Data/*.h)

```cpp

UENUM(BlueprintType)

enum class EConditionType : uint8

{

CivTierAtLeast, CivHasTrait, CivResourceAtLeast, CivStabilityBelow, CivEthicsAxisAbove, // CivTierAtLeast·PlayerTierAtLeast의 Value = Tier 번호 1~5 (열거형 값 + 1. EAscensionTier는 Tier1_Planetary == 0)

CivStabilityBelowFor, CivRelationAtLeast, // 지속 틱 조건(Value=임계 0.3 — 파라미터가 아니다: 카운터가 0.3 아래의 틱을 세고 코드는 Value를 읽지 않으며, 읽는 이를 위해 0.3을 적고 스키마는 그것만 받는다, §16 #25 ②. Key=틱 수 — 숫자만으로 된 양의 정수이고 아니면 거짓이다(§7.5). FCivilizationState.LowStabilityTicks 카운터를 읽는다, §4.1) / Key=OtherID(CivID 또는 TranscendentID)

CivReverenceAtLeast, // ReverenceToPlayer >= Value (§4.5 방문 SpawnConditions)

SystemHasHazard, SystemOwnedBy, MegaprojectComplete, // MegaprojectComplete의 Key = DT_Megaprojects 행(필수). 주제 성계의 완성 목록 CompletedMegaprojects에 그 행의 기록이 있고 그 틱이 0보다 크고 문맥의 SinceTick 이상일 때만 참(플레이어 업적 판정만 SinceTick에 Calling.BornTick을 넣고 그 밖은 0이다 — §7.5) (§4.4)

SystemPressureAtLeast, SystemInfluencedBy, // 영압 P(S) >= Value (§4.2) / Key=TranscendentID (종속 판정)

PlayerTierAtLeast, PlayerInSystemWithHazard, PlayerStatAtLeast, PlayerHasRelic, PlayerRoleIs, // PlayerStatAtLeast Key: Body/Mind/Causality/Exaltation. PlayerRoleIs Ruler = 청원 카드

PlayerHasStatus, PlayerLifeIndexAtLeast, PlayerAgeAtLeast, // Tag=Status.* / 제 N대 이상 / 노화 ON일 때만 (§4.4)

GalaxyTickAtLeast, TranscendentSealed, TranscendentAwareOfPlayer, // GalaxyTickAtLeast는 판에 하나인 시계 Tick을 읽는다 — 은하별 시계는 없다 (§4.4) / TranscendentSealed는 bSealed이고 SealedTick >= 문맥의 SinceTick일 때만 참 (§4.4, §7.5)

FlagSet, FlagNotSet, // Tag = Flag.*. FGalaxyState.Flags를 읽는다 (§4.5)

TranscendentDoctrinePresent, // Tag = Doctrine.*. 미봉인 활성 초월자 중 그 교리가 있는가 (§4.2 로스터, §4.4 귀환 게이트)

CivHasNotable, // Tag = Notable.*. 문맥 문명에 그 역할의 열린 기록(LastTick == 0)이 있는가 (§7.2). Key = 문맥 문명 선택자(비우면 문맥 문명 — FCosmicEvalSubject.CivIndex, 호출자가 현재 성계의 소유 문명을 넣는다. Former = FormerCivID). 인물을 읽는 유일한 조건이고 조우 변형 선택에만 쓴다(재회 '알아본다/숨겨준다' = Notable.Loyalist, §5.10). 인물에 의존하는 카드·조건은 만들지 않는다 (§8.3)

CivFallenSphereTicksReached, // Key=틱 수(숫자만, CivStabilityBelowFor와 같은 읽기). 그 문명이 타락 초월자의 영향권에 머문 지 Key틱이 찬 슬롯 한 번만 참이다: Key <= FallenSphereTicks < Key + DT_Rules.EventSlotTicks(1 미만은 1로 읽는다) — 문명의 이벤트 슬롯은 EventSlotTicks틱에 한 번 돌고 카운터는 틱마다 1씩 늘므로, 머무는 동안 슬롯 하나만 그 창에 든다. 그래서 문명 주제(Civilization)의 슬롯 행에게 이 조건은 한 번 머무는 동안 많아야 한 슬롯에서 참이고, 머묾이 Key + EventSlotTicks − 1틱 이상이면 정확히 한 슬롯에서 참이다 — 다른 조건이 맞는 Standing 행은 그때 한 번 나고("책이 닫혔다", §5.11), Lottery 행은 그 슬롯의 추첨에 한 번 든다. 이름으로 부르는 행(창 안의 틱마다 참이다)과 성계 주제(SettledSystem) 행(쿨다운 없는 Standing 행은 성계마다 한 번씩 나고, 그 밖에는 주제와 상관없는 성계 하나에 난다)은 이 조건을 싣지 않는다(스키마). 이 보장은 머무는 동안 EventSlotTicks가 바뀌지 않는다고 본다 — 값이 바뀔 때(다른 규칙으로 불러온 세이브) 창 안이던 머묾은 두 번 나거나 나지 않을 수 있다. Tag(타락 교리 Doctrine.Silence·Assimilation·Zeal·Recurrence — 카운터가 타락의 영향권만 세므로)는 넣지 않아도 된다. 넣으면 판정하는 때에 그 문명의 정착된 성계 가운데 하나가 그 교리의 봉인되지 않은 초월자 영향권 안에 있어야 한다(카운터는 교리를 가리지 않는다). Value는 쓰지 않는다 (§16 #25 ①). 열거형은 끝에만 추가한다 (§9.1). 앎(Knowing)을 읽는 값은 두지 않는다 — 세계는 앎을 쓰기만 하고 읽지 않는다 (§8.1)

CivResearchAtLeast // 끝에 더한 것(Key = 연구 영역 Physics/Society/Engineering/Transcendence — 스키마의 낱말 목록 research, 영역이 아닌 Key는 거짓, Value 0~1: 문맥 문명의 Research[Key] >= Value — 거대 프로젝트와 승천 판정 작업의 T1→T2 업적 '물리 50%'). 정해 둔 추가분(§16 #29, 2026-10-15 — 처음 쓰는 행과 함께 끝에 더한다): CivStabilityAtLeast(Stability >= Value — Phase 4 시대 명명 ≥ 0.6, 느부갓네살 > 0.9는 그 행에서 정한다), PlayerHasPower(Tag = Power.* — Phase 4). Key 선택자 낱말(#29 ⑦): 비우면 문맥 문명(문명을 고르는 자리)·문맥의 상대 OtherID(상대를 고르는 자리), Former = FormerCivID, Ruled = 통치 문명(Embodied일 때만), SystemOwnedBy의 Key도 선택자다. 코드에는 지금 Former만 있고 나머지는 그 낱말을 처음 쓰는 행(Phase 4 청원 카드)과 함께 들어온다 — 그때까지 슬롯 행은 Player 조건·역할 효과·그릇 효과를 싣지 않는다(스키마)

SystemHazardTicksAtMost // 끝에 더한 것(MVP ②): Tag = Hazard.*(필수 — 비면 거짓), Value = 틱(1 이상의 정수). 주어 성계가 그 위기를 갖고 HazardTicksRemaining <= Value. 카운트다운은 틱마다 1씩 줄어 '잔여 50틱'을 기다리는 청원은 정확히 한 틱에 조건이 선다(§4.1 ③, §5.10 '왕좌를 지키겠는가')

CivSettledSystemsAtLeast // 끝에 더한 것(MVP ③): Value = 1 이상의 정수. 문맥 문명이 가진 정착된 성계(소유 ∧ CosmicState::IsSettled — 다른 백성이 가진 OwnedSystems 항목과 정착 중인 성계는 세지 않는다)가 Value 이상. 주어 성계는 읽지 않는다. 둘째 T2→T3 업적 feat_three_worlds(정착 3 ∧ 공학 1.0)의 조건이고, 업적 행의 이 값은 그 칸의 보유 상한(RealmCapByTier[RequiredTier])을 넘지 않는다(Data.Integrity)

};

USTRUCT(BlueprintType)

struct FCosmicCondition

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) EConditionType Type = EConditionType::GalaxyTickAtLeast;

UPROPERTY(EditAnywhere) FGameplayTag Tag; // 특성/Hazard/교리 등

UPROPERTY(EditAnywhere) FName Key; // 연구 영역, 자원 이름, 능력치 이름 등

UPROPERTY(EditAnywhere) float Value = 0.f; // 임계값

};

UENUM(BlueprintType)

enum class EEffectType : uint8

{

AddResource, MultiplyIncome, ShiftEthics, AddTrait, RemoveTrait, ChangeStability, ChangeRelation,

SpawnHazard, ResolveHazard, TransferSystem, ChangeReverence, GrantPower, PlayerStatDelta, SetFlag, // SetFlag은 FGalaxyState.Flags에 Tag(Flag.*)를 기록 (§4.5). 위기의 셋(위기 처리 작업, §4.1·§7.5): SpawnHazard = Tag의 행을 주제 성계에 놓는다(주장 행은 놓지 못한다 — 주장 규칙이 Law.Remnant를 지킨다) / ResolveHazard = 주제 성계의 Hazard를 해결한다(Tag가 있으면 그 아래 태그일 때만) / TransferSystem = Key의 문명에게 넘긴다(빈 Key = 무주로 놓기. 어느 백성의 마지막 정착된 성계도 넘기거나 놓지 못한다 — §16 #53 ①. 정착 중인 성계는 셈하지 않는다 — §5.7, 정착과 문명 제거 작업의 리뷰). 셋 다 Sim/CosmicHazards의 세계 함수 하나를 거친다

KillPlayer, DeposeRuler, // 즉시 실행하지 않고 FCallingState.PendingRoleEffect에 기록만 한다. 틱 경계의 '역할 효과 해소' 단계에서 UPlayerRoleSubsystem의 사망·폐위 파이프라인으로 위임 (§4.4, §5.10, §7.4). Tag = Death.* / Reign.*. Phase 6+: InstallAt(기원 선택 카드)

ChangePopulation, // Value = 비율. 역병 미해결 −0.4 (§4.1), 희생 이벤트 event_zeal_sacrifice −0.05 (§4.2). 문명 Population에만 — 성계 단위 인구는 없다

NameNotable // Tag = Notable.*, Value 0. 문맥 문명(Key = 선택자, 비우면 문맥 문명 — 호출자가 현재 성계의 소유 문명을 넣는다, Former = FormerCivID)에 인물 기록 1개 (§1.3, §7.2). 가드는 UCosmicEventSubsystem::Apply (§7.5): Tier <= 3, PersonNameBank 비어 있지 않음, 같은 Role 열린 기록 닫기, 상한 MaxNotablesPerCiv 퇴출. 이름 추첨은 적용마다 정확히 1회, 제 가드보다 먼저다(틱당 원천은 이 효과를 걸지 않으므로 뽑지도 않는다, §7.5). VS에 이 효과를 싣는 행: crisis_rebellion 발생 전조 ②(Notable.Rebel — Severity 1 유일 예외, §4.1), 망명 변형 '충신이 몰래 배를 내준다'(Notable.Loyalist, Former), crisis_visitation 행의 OnResolveEffects(Notable.Hearer), '책이 발견되었다'(Notable.Hearer). 금지: enc_messenger_*·Event.Grace.*·Hazard.Cult 행 (Data.Integrity). 인물 기록을 만드는 효과는 이것 하나이며 대상은 개인이 아니라 문명이다. 열거형은 끝에만 추가한다 (§9.1)

FocusResearch // 끝에 더한 것(MVP ②): Key = 연구 영역(문맥 문명의 ResearchFocus 키 — 스키마의 낱말 목록 research), Value = 그 몫(0 초과 1 이하, 스키마는 0.25~1). ResearchFocus[Key] = Value, 나머지 영역은 (1 − Value) / (영역 수 − 1)씩 — 합은 1이다(§5.4). 개국 청원 '지혜로운 마음'의 세 길과 연구 중점 프리셋(MVP ⑦). 틱당 원천은 걸지 않는다

// Phase 6+ 예약(끝에 추가): MultiplyResearch, PopulationLossMultiplier, StabilityFloor (§4.2 회귀·광신 행). 셋 다 틱당 효과다 — 더할 때 Apply의 틱당 원천 목록(§7.5)에도 넣는다

// 정해 둔 추가분(§16 #29, 2026-10-15): GrantRelic·RemoveRelic(Key = DT_Relics 행 — Phase 4 유물과 함께 끝에 더한다). 성계를 무주로 놓는 일(바벨 모티프, §8.6)은 새 효과가 아니라 TransferSystem의 빈 Key다(위기 처리 작업에서 TransferSystem과 함께 들어갔다, #29 ⑥)

};

USTRUCT(BlueprintType)

struct FCosmicEffect

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) EEffectType Type = EEffectType::AddResource;

UPROPERTY(EditAnywhere) FGameplayTag Tag;

UPROPERTY(EditAnywhere) FName Key;

UPROPERTY(EditAnywhere) float Value = 0.f;

UPROPERTY(EditAnywhere) int32 DurationTicks = 0; // 0 = 즉시/영구

};

// DT_Events

USTRUCT(BlueprintType)

struct FCosmicEventDef : public FTableRowBase

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) FGameplayTag Category; // Event.Civ.Economy/Society, Event.Crisis.Supernova/Transcendent/Rebellion/Claim, Event.Grace.Rescue/Remnant/Jubilee (DefaultGameplayTags.ini의 Event.* 트리 9개와 동일하게 유지). 그 행의 가중 계열이다 — 초월자·특성·칙령의 가중 수정자가 이 태그로 건다. 로그 줄의 카테고리는 LogCategory다

UPROPERTY(EditAnywhere) FGameplayTag LogCategory; // Log.*. 이 행이 내는 로그 항목의 Category. 코드는 베끼기만 하고 Event.*에서 얻지 않는다. 허용 짝은 Tools/data/schema.json (§8.1)

UPROPERTY(EditAnywhere) ECosmicEventTrigger Trigger = ECosmicEventTrigger::Lottery; // Lottery(슬롯의 추첨, 문명·슬롯당 1행 이하) / Standing(조건과 쿨다운만 맞으면 슬롯에서 반드시 난다 — 주기 행, 추첨·가중 없음) / Named(슬롯이 보지 않는다. 코드가 FireRow로 낸다). 열거형은 끝에만 추가한다

UPROPERTY(EditAnywhere) ECosmicEventSubject Subject = ECosmicEventSubject::Civilization; // Civilization(문명에 한 번. 문맥 성계 = 정착된 수도. 줄의 주제는 문명만 — 수도를 모르는 만난 백성의 줄도 말해진다. 그래서 그 줄은 성계를 부르지 않는다, 스키마) / SettledSystem(정착된 성계마다, OwnedSystems 순서. 줄의 주제는 성계와 문명). 열거형은 끝에만 추가한다

UPROPERTY(EditAnywhere) TArray<FCosmicCondition> Conditions; // AND

UPROPERTY(EditAnywhere) float BaseWeight = 1.f;

UPROPERTY(EditAnywhere) int32 CooldownTicks = 200;

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> Effects;

UPROPERTY(EditAnywhere) TArray<FText> LogTemplates; // Lottery·Standing 행은 3개 이상, Named 행은 1개 이상 (Data.Integrity). 슬롯은 {Civ} {System} {Hazard} 셋이고 Civilization 주제 행은 {Civ} {Hazard}만 부른다(줄의 주제 성계는 그 줄이 이름을 부르는 성계다 — 수도를 곁에 부르지 않는다, §8.1. 아는 범위와 로그 작업의 리뷰. 코드도 {HomeSystem}을 채우지 않는다 — 재감사. {Hazard} = 행이 발화하기 시작할 때 주제 성계가 쥔 Hazard의 이름 — 효과가 그것을 해결해도 그 이름이다. 위기 처리 작업. §8.2의 다른 슬롯은 이벤트 행의 줄에서 채워지지 않는다 — Data.Integrity가 막는다). 비면 줄을 내지 않는다(SignRow의 줄도). BaseWeight는 Lottery 행만 쓴다(1 = 자격 있는 단위에 대해 슬롯당 DT_Rules.EventChancePerSlot의 확률. Standing·Named 행은 0). CooldownTicks는 (행, 문명)마다다 — 키 "<행>@<CivID>", 0 = 쿨다운 없음, Named 행은 0

UPROPERTY(EditAnywhere) uint8 Severity = 0; // 0 일반, 1 주목, 2 신화적

UPROPERTY(EditAnywhere) uint8 MinTierToReveal = 0; // LogTemplates가 말해지는 가장 낮은 보는 쪽 Tier. 그 아래에는 SignRow가 있으면 그 줄이, 없으면 아무것도 말해지지 않는다(Severity 2·Log.Claim·Log.Record는 bUntold로 쌓인다, §8.1). 초월자 이벤트만 템플릿 2벌 (§8.2). 값은 Tier 번호 1~5(3 = Tier 3, 열거형 값이 아니다), 0 = 누구에게나

UPROPERTY(EditAnywhere) FName SignRow; // DT_LogTemplates의 Log.Sign 행 — 하위 화자의 문장. 초월자 이벤트의 둘째 템플릿 벌은 이벤트 행이 아니라 여기에 있고 여러 행이 함께 쓴다. NAME_None = 그 Tier 아래에는 아무 줄도 없다. MinTierToReveal 2~5이고 LogTemplates가 있는 행에만 쓴다(Data.Integrity — 1 이하면 그 아래에 보는 이가 없고, 줄이 없는 행은 징조도 내지 않는다). 가리키는 행의 Category는 Log.Sign이고 그 템플릿은 {Civ} {Hazard}만 부른다(Data.Integrity — 징조 줄은 이벤트 행의 줄의 초점을 그대로 쓰고(Civilization 행이면 주제 성계가 없다) 여러 행이 함께 쓰므로 성계를 부르지 않는다. LogCategory가 Log.Calling이고 MinTierToReveal이 2 이상인 행은 SignRow를 가져야 한다 — 창을 가진 부름의 줄은 징조와 짝이다, §8.1. 둘 다 아는 범위와 로그 작업의 재감사)

};

// DT_Hazards — Hazard가 놓이는 길(위기 처리 작업, §4.1)

UENUM() enum class ECosmicHazardSpawn : uint8 { Draw, Capital, ClaimRule }; // Draw = 정착된 성계마다 틱당 시뮬 추첨 1회를 조건이 맞는 Draw 행들의 확률에 놓는다 / Capital = 추첨 없이 조건이 맞는 백성의 수도에 / ClaimRule = 만료 뒤의 주장 규칙만(SpawnHazard 효과는 놓지 못한다)

USTRUCT(BlueprintType)

struct FHazardDef : public FTableRowBase

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) FGameplayTag HazardTag; // 행마다 다르다(Data.Integrity) — 성계는 태그를 쥐고 코드는 태그로 행을 찾는다 (§16 #31 ①)

UPROPERTY(EditAnywhere) FGameplayTag SourceDoctrine; // 선택. Doctrine.*. Hazard.Cult/Possession/Famine 행이 T4 교리에 귀속되어 로그 화자·압력 효과를 교리에서 가져온다 — 'T4 교리 → T3 대리 Hazard → T1~2 교단' (§4.2 위계). 빈 태그 = 교리 무관

UPROPERTY(EditAnywhere) FText DisplayName; // UI와 슬롯 {Hazard} (v0.8 — 단계 5: 별 등급마다의 표시 이름 — 열을 더하거나 같은 기제, 헤더가 정한다. 결정 10)

UPROPERTY(EditAnywhere) int32 CountdownTicks = 300; // 1 미만은 1로 읽는다. Hazard.Claim도 제 행으로 센다(옛 DT_Rules.ClaimCountdown은 지웠다)

UPROPERTY(EditAnywhere) ECosmicHazardSpawn SpawnRule = ECosmicHazardSpawn::Draw; // Draw·Capital·ClaimRule. 효과나 코드 훅만 놓는 위기(방문·징조)는 Phase 4의 헤더가 열거자 하나를 더한다(예: Placed) — Draw 행의 확률은 0보다 크다(스키마, Phase 2 종료 리뷰의 재감사)

UPROPERTY(EditAnywhere) float SpawnChancePerTick = 0.0005f; // Draw 행만: 한 추첨에서의 몫 × 주인 아키타입의 HazardSpawnMultipliers[HazardTag](없으면 1). 몫의 합이 1을 넘으면 수에 합을 곱한다(이벤트 슬롯과 같다). Capital·ClaimRule 행은 0(Data.Integrity). 0 아래나 숫자가 아닌 값은 0. 추첨은 정착된 성계만 돈다 — 무주 성계와 정착 중인 성계는 뽑지 않고 건너뛴다 (§5.7)

UPROPERTY(EditAnywhere) TArray<FCosmicCondition> SpawnConditions; // AND. 주제 = 그 성계와 주인. 단계가 틱마다 읽으므로 머묾 조건(CivFallenSphereTicksReached)은 싣지 않는다(Data.Integrity)

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> OnExpireEffects; // 주제 = 그 성계와 주인, SourceRow = 이 행. KillPlayer는 그 성계에 선 방랑자에게만 (§4.4). 예외: 플레이어가 통치하는 문명이 Hazard.Extinction의 만료로 성계를 모두 잃으면 위치와 무관하게 통치자도 사망한다 (§4.1, §4.4). 그 문명(플레이어가 통치하던 문명)은 그 틱이 아니라 다음 틱의 건넘 단계에서 제거된다 — 플레이어가 통치하지 않는 문명은 같은 틱의 건넘 단계에서 제거된다 (§7.4). Phase 4b의 사망 파이프라인 전에는 KillPlayer를 싣는 행이 없다(Data.Integrity). crisis_rebellion은 DeposeRuler (§4.1). 만료 효과(갈래 포함)는 TransferSystem과 ResolveHazard를 싣지 않는다 — 성계를 앗는 것은 문 뒤의 단계 코드뿐이고, 만료가 제 성계를 다시 해결하지 않는다(Data.Integrity)

UPROPERTY(EditAnywhere) float BranchChance = 0.f; // 만료의 결과 수가 이것보다 작으면 OnExpireEffects 대신 OnExpireBranchEffects (AI 각성의 50/50, §4.1). 0 = 갈래 없음

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> OnExpireBranchEffects;

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> OnExpireNeighbourEffects; // TransferSystem은 싣지 않는다(이웃의 Hazard를 해결하는 것은 된다 — 그 해결 효과도 TransferSystem을 싣지 않는다, OnResolveEffects). 만료 효과 뒤, 태그의 코드 훅 앞: 죽은 노드가 아닌 레인 이웃마다 그 Lanes 순서로 한 번(주제 = 그 이웃과 주인, SourceRow = 이 행). 초신성의 이웃 피해. 역병의 전파는 해결 수단인 격리와 함께 Phase 4

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> OnResolveEffects; // 해결 시 1회, 슬롯을 비운 뒤(주제 = 그 성계와 주인, SourceRow = 이 행). R·W·AI 해결 모두 같은 길(CosmicHazards::Resolve)이다. ResolveHazard는 싣지 않는다(해결이 놓은 것을 다시 해결하면 끝나지 않는다 — 코드도 깊이 4에서 멈추고 에러 한 줄, Data.Integrity). TransferSystem도 싣지 않는다(만료의 이웃 효과가 부르는 해결이 성계를 넘기면 문 뒤가 아닌 곳에서 성계를 앗는다 — §4.5, Data.Integrity). crisis_visitation: Trait.Faith.Visited 부여 + NameNotable(Notable.Hearer)

UPROPERTY(EditAnywhere) TArray<FName> RulerResolutions; // DT_Megaprojects·DT_Edicts·DT_Encounters(청원) 행 이름. 하나라도 완료되면 그 Hazard를 해결한다. 여기 든 DT_Megaprojects 행이 그 Hazard의 대응 프로젝트다 — 대응 칸에서 돈다(§5.7, 거대 프로젝트와 승천 판정 작업부터). 스키마의 종류 rows(DT_Megaprojects·DT_Encounters — 칙령 테이블은 그 구조체와 함께)

UPROPERTY(EditAnywhere) TArray<FName> WandererResolutions; // DT_Encounters 행 이름 (Phase 4)

UPROPERTY(EditAnywhere) FGameplayTag LogCategory; // 아래 세 줄의 Log.*(Log.Crisis 아래 — Data.Integrity)

UPROPERTY(EditAnywhere) uint8 Severity = 1;

UPROPERTY(EditAnywhere) TArray<FText> OutbreakTemplates; // Hazard 자신의 줄 세 벌(UCosmicEventSubsystem::TellHazardLine): 발생·만료·해결. 그 성계와 주인에 관한 줄, SourceRow = 이 행, 슬롯 {Civ} {System} {Hazard}(수도를 곁에 부르지 않는다 — Data.Integrity, 아는 범위와 로그 작업의 리뷰). 비면 그 순간은 제 줄이 없다(반란의 발생은 전조 ② 행이 말한다)

UPROPERTY(EditAnywhere) TArray<FText> ExpireTemplates;

UPROPERTY(EditAnywhere) TArray<FText> ResolveTemplates; // 주장이 아무것도 편입하지 못한 만료도 이 줄로 말한다

UPROPERTY(EditAnywhere) FName OutbreakRow; // 발생 줄 다음에 FireRow로 내는 DT_Events의 Named 행이고 SettledSystem 주제다(주제 = 그 성계와 주인 — 그 줄이 Hazard 자신의 줄과 같은 성계로 판정되어 시작과 끝이 함께 말해지거나 함께 숨는다, Data.Integrity, 아는 범위와 로그 작업의 재감사). crisis_rebellion = event_rebellion_rises(전조 ②). NAME_None = 없음. 그 행은 ResolveHazard를 싣지 않고(되풀이되는 발생 행도 깊이 4에서 멈추고 에러 한 줄), 교단 행에는 발생 행이 없다(지도자에게 이름이 없다, §4.1 — Data.Integrity)

};

// DT_Feats

USTRUCT(BlueprintType)

struct FAscensionFeatDef : public FTableRowBase

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) FText Title;

UPROPERTY(EditAnywhere) FText MythicNarrative; // 승천 연출 중 표시되는 서사

UPROPERTY(EditAnywhere) EAscensionTier RequiredTier = EAscensionTier::Tier1_Planetary;

UPROPERTY(EditAnywhere) EAscensionTier TargetTier = EAscensionTier::Tier2_Stellar;

UPROPERTY(EditAnywhere) EPlayerRole Path = EPlayerRole::Ruler;

UPROPERTY(EditAnywhere) TArray<FCosmicCondition> CompletionConditions;

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> AscensionCost; // 승천의 대가 (§3.2)

UPROPERTY(EditAnywhere) FGameplayTagContainer GrantedPowers; // T4->T5 행은 Way.Received 또는 Way.Seized를 포함 (§3.2). Law.Humility: Way.Seized는 어떤 Feat도 떼지 못한다. Power.* 태그는 권능을 주는 첫 업적 행과 함께 ini에 추가한다(Phase 2의 두 행은 권능을 주지 않는다). 업적 행 규칙(스키마): RequiredTier에서 TargetTier까지 한 칸(T5에서 오르는 행 없음), CompletionConditions 1개 이상, 통치자 업적 행에는 그릇을 읽는 조건과 역할·그릇에 닿는 대가 없음(모든 백성을 판정한다 — 방랑자 행은 §16 #28) — 판정도 한 칸이 아니거나 조건이 없는 행은 에러 한 줄과 함께 판정하지 않는다

UPROPERTY(EditAnywhere) FGameplayTag DedicationBank; // 완성 시 그 문명이 붙이는 헌정명 뱅크 (§8.6. Phase 4: Epithet + 첫 생애 백성의 의회·회중 최소형, 뱅크 확장은 Phase 6+). DT_Edicts·거대 프로젝트 행에도 같은 컬럼

UPROPERTY(EditAnywhere) TSoftObjectPtr<class UNiagaraSystem> FX;

UPROPERTY(EditAnywhere) TSoftObjectPtr<class USoundBase> Sound;

UPROPERTY(EditAnywhere) TArray<FText> LogTemplates; // 승천 줄(Log.Ascension, Severity 2 — AI 백성도), 오른 백성과 그 수도에 대해, SourceRow = 이 행. 슬롯 {Civ} {HomeSystem}. 3변형 이상 (거대 프로젝트와 승천 판정 작업)

};

// DT_Encounters

// 선택지의 판정 능력치(MVP ② A-2a — 정의 열거형, 저장되지 않는다, 끝에만 추가). 그릇의 능력치는 0~10, Stability는 다스리는 백성의 Stability(0~1)를 (int32)(Stability × 10)으로 읽는다(§16 #30 ①). Influence는 판정이 아니라 비용이다. 데이터는 빈 이름을 "None"으로 쓰므로 '판정 없음'은 NoCheck다.

UENUM(BlueprintType) enum class ECosmicCheckStat : uint8 { NoCheck, Body, Mind, Causality, Exaltation, Stability };

USTRUCT(BlueprintType)

struct FEncounterChoice

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) FText Label;

UPROPERTY(EditAnywhere) ECosmicCheckStat CheckStat = ECosmicCheckStat::NoCheck; // 판정: 능력치 + 0~5 ≥ Difficulty (§5.9). 통치자 청원은 Stability도 (§5.10)

UPROPERTY(EditAnywhere) int32 Difficulty = 0; // >= 16 = 불가능한 선택지(영압 게이트, §4.2)

UPROPERTY(EditAnywhere) FResourcePool Cost; // Role에 따라 PersonalPool 또는 문명 Stockpile에서 차감 (§5.10)

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> OnSuccess;

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> OnFailure; // KillPlayer 효과의 데이터 발생 지점 중 하나 (§4.4)

UPROPERTY(EditAnywhere) FText SuccessText;

UPROPERTY(EditAnywhere) FText FailureText;

UPROPERTY(EditAnywhere) FName NextEncounter; // 체인용

};

USTRUCT(BlueprintType)

struct FEncounterDef : public FTableRowBase

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) FText Title;

UPROPERTY(EditAnywhere) FText Body;

UPROPERTY(EditAnywhere) TArray<FCosmicCondition> Conditions;

UPROPERTY(EditAnywhere) float BaseWeight = 1.f;

UPROPERTY(EditAnywhere) TArray<FEncounterChoice> Choices; // 2~3. 로컬라이제이션 키가 위치라 끝에만 붙인다(§8.5)

UPROPERTY(EditAnywhere) TSoftObjectPtr<class UTexture2D> Illustration; // 선택

UPROPERTY(EditAnywhere) ECosmicEventTrigger Trigger = ECosmicEventTrigger::Named; // 앎 단계가 카드를 내놓는 때(§5.10): Standing = 조건이 막 선 틱, Lottery = 주기 슬롯의 가중 추첨, Named = 이름으로만(사슬의 다음 카드)

UPROPERTY(EditAnywhere) ECosmicEventSubject Subject = ECosmicEventSubject::Civilization; // 다스리는 백성(수도) 또는 그 백성의 정착된 성계 하나

UPROPERTY(EditAnywhere) TArray<FText> VoiceBodies; // 비었거나 둘: [0] 의회, [1] 회중의 장로들. 있으면 Body는 비우고 화면이 다스리는 백성의 Spiritualism(> 0 = 장로들)으로 고른다 — 상태가 아니다

};

// DT_Civs (§5.11). 문명 아키타입. DataAsset 아님

USTRUCT(BlueprintType)

struct FCivArchetypeDef : public FTableRowBase

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) FText DisplayName;

UPROPERTY(EditAnywhere) FGameplayTagContainer Traits; // 아키타입 태그 포함. CivHasTrait로 거른다

UPROPERTY(EditAnywhere) FEthicsAxes EthicsPreset;

UPROPERTY(EditAnywhere) FGameplayTag AIProfile; // AI.Profile.*

UPROPERTY(EditAnywhere) FLinearColor Color = FLinearColor::White;

UPROPERTY(EditAnywhere) TArray<FText> NameBank; // {Star} 패턴 (§5.11)

UPROPERTY(EditAnywhere) TArray<FText> AdjectiveBank; // {CivAdj} 3개, 또는 비움(윤리 축의 형용사만 쓴다 — arch_default). Data.Integrity: 0개 또는 3개

UPROPERTY(EditAnywhere) TArray<FText> PersonNameBank; // 인물(FNotableRecord.NameIndex)과 그릇(FLifeRecord.NameIndex)의 이름 뱅크 (§1.3, §5.11). NoSelf가 아닌 행 12개(Data.Integrity 하한 10, §9.3) — Data.Integrity: 중복 없음, 성경 인물·실존 인물·천사·실존 종교 고유명사 금지, Data.NoVerdictWords 적용(Phase 5 — 지금은 스키마의 forbidden: verdict). arch_swarm·arch_machine은 비움 = 이름 없는 문명(NameNotable 무시 — Trait.Mind.NoSelf 보유 문명도 동일). 렌더 Bank[NameIndex % Bank.Num()] — 뱅크가 줄어도 구 세이브가 깨지지 않는다. 문명 NameBank({Star} 패턴)와 별개 컬럼

UPROPERTY(EditAnywhere) TMap<FGameplayTag, float> HazardSpawnMultipliers; // Hazard.* -> 배율 (선호 Hazard). 추첨(Draw) 행에만 닿는다 — 없는 키 = 1, 0 아래나 숫자가 아닌 값은 0 (§4.1)

UPROPERTY(EditAnywhere) TMap<FName, float> InitialRelations; // 다른 아키타입 행 -> 초기 관계

UPROPERTY(EditAnywhere) float Weight = 1.f; // NewGalaxy AI 추첨 가중치. 0 = AI 추첨 제외 (첫 생애 행 arch_seeker — A0 4b부터, 그 전 arch_terran(은퇴): 플레이어 전용, DT_Rules.PlayerArchetypeRow에서만 참조. 스키마 규칙: arch_seeker는 Weight 0·Trait.Arch.Seeker·Revelation.Seeking)

UPROPERTY(EditAnywhere) FGameplayTag RevelationMode; // Revelation.* 계시 양식 (§5.11). 모든 행이 정확히 하나. 텍스트만 바꾸고 수치는 바꾸지 않는다. arch_default는 Revelation.Seeking 고정

UPROPERTY(EditAnywhere) TArray<FText> RevelationTemplates; // 정확히 3개(Data.Integrity): [0] 징조(Log.Sign) / [1] 전언(enc_messenger_* 간접 화법) / [2] 지나감·응답(Hazard.Visitation 해결·만료). 수동태·간접 화법만

UPROPERTY(EditAnywhere) TArray<FText> HomeSystemNames; // 모성계 표시명 뱅크. 첫 생애 행 = 그 백성이 제 해를 부르는 이름(숫자 없음 — A0 4b부터 {첫 해}, 자리 채움 — 단계 7에서 다시 쓴다. A0 4a에서 arch_terran은 비웠다 — 성역과 이름이 겹치지 않게). 비면 FGalaxyDef.StarNameBank (§5.11)

UPROPERTY(EditAnywhere) FGameplayTag HomeStarClass; // 모성계 항성 등급 고정 (첫 생애 행 = Star.Class.G — v0.8 — 단계 5까지, 그 뒤 실제 별 등급 + 정규화). 빈 태그 = DT_Rules.LivingWorldStarShares의 분포에서 뽑는다 (§7.4)

UPROPERTY(EditAnywhere) TArray<FGameplayTag> DedicationBanks; // Faith.Bank.* (§8.6). 보통 1개. 첫 생애 행 = {Faith.Bank.Nations, Faith.Bank.Civic} — 말이 온 뒤 Trait.Faith.Book의 효과로 Hebrew를 더한다(첫 생애 행의 값은 A0 4b부터, 말이 온 뒤의 Hebrew는 단계 7부터. 그 전 arch_terran = {Hebrew, Civic}) — 청원한 파벌이 고른다 (§5.11). Feat·칙령·프로젝트 행의 DedicationBank는 '어떤 이름 종류'를, 이 컬럼은 '어느 뱅크'를 정한다

};

// DT_Transcendents (§4.2). 행 이름 == TranscendentID. 초월자는 판에 교리마다 1체다 — 은하마다 따로 두지 않고 타락은 건너지 않는다 (§4.2). VS 1행 trans_silence. EA: 동화·광신·회귀 행 중 2행이 NewGalaxy 가중 추첨으로 활성 + 천상 trans_covenant(상태 인스턴스 없음). Phase 6+: 독립 trans_stillness·trans_release

USTRUCT(BlueprintType)

struct FTranscendentDef : public FTableRowBase

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) FGameplayTag Doctrine; // Doctrine.* (고유. Data.Integrity)

UPROPERTY(EditAnywhere) FGameplayTag Allegiance; // 소속: Realm.Abyssal / Realm.Celestial / Realm.Independent. 정의에만 있고 상태(FTranscendentState)에는 없다 (§4.2). Data.Integrity: Allegiance ∈ Realm.*

UPROPERTY(EditAnywhere) FGameplayTag CounterfeitOf; // 모방의 원리 (§4.2): 이 교리가 흉내 내는 참된 것. Truth.Rest / Truth.Union / Truth.Worship / Truth.Renewal. Abyssal 행은 필수

UPROPERTY(EditAnywhere) FText Title; // 호칭(이름 아님). 로그 슬롯 {Transcendent}, 성계 패널의 표식 말(한 줄과 툴팁), 두 책 장 묶음 제목 (§8.2, §8.4). 예: 꺼뜨리는 이

UPROPERTY(EditAnywhere) FText Lure; // 필멸자에게 주는 것 — 교리 표 '유혹' 열 (§4.2). 텍스트만

UPROPERTY(EditAnywhere) EAscensionTier Tier = EAscensionTier::Tier4_Transcendent; // 허용값 T3/T4. Abyssal T5 금지 (Data.Integrity, §4.2 위계)
UPROPERTY(EditAnywhere) float Depth = 0.5f; // 칸 안의 깊이 0~1 (§3.2 심도). 영압의 초월자 항에 (1 + Depth)를 곱한다(§4.2) — 코드는 0~1로 잘라 읽고 범위는 Data.Integrity가 본다. 정의 데이터라 저장·해시에 들지 않는다. 시작값: 침묵 1.0 / 회귀 0.7 / 광신 0.6 / 동화 0.5 (Phase 5 조정). 숫자가 아닌 값은 0으로 읽는다. Celestial 행은 인스턴스가 없어 압력에 들지 않는다. Independent 행은 로스터가 인스턴스를 만들지 않는다 — 독립 초월자의 인스턴스(Phase 6+)는 칸의 가중치 그대로 누르므로(§4.2 '격은 격이다') 그 행을 처음 쓸 때 (1 + Depth)를 `Realm.Abyssal`에만 곱하게 하거나 린트로 Depth 0을 요구한다. Phase 2

UPROPERTY(EditAnywhere) int32 InitialInfluencedSystems = 0; // 0 = 초기 코어 없음, 1 = 초기 코어 한 성계 — NewGalaxy가 §5.7의 규칙으로 자리를 고른다. 0~1 (Data.Integrity. 둘째 코어는 아직 없다)

UPROPERTY(EditAnywhere) int32 MaxInfluencedSystems = 12; // Hazard.Claim 발생 상한: InfluencedSystems.Num() >= 이 값이면 새 주장 없음 (§4.2). 개체당 판 전체의 합이다 — 은하가 늘어도 같다. Celestial 행은 0

UPROPERTY(EditAnywhere) float SpawnWeight = 1.f; // NewGalaxy 타락 로스터 가중 추첨 (침묵 고정, 동화 2 / 광신 3 / 회귀 2). Celestial·Independent 행은 0. 0 이하이거나 유한하지 않은 값(NaN·무한대)의 행은 후보가 아니다 (§4.2, §7.4)

UPROPERTY(EditAnywhere) TMap<FGameplayTag, float> EventWeightModifiers; // Event.* 잎 태그 -> 그 Category와 정확히 같은 Lottery 행의 가중 인수(부모 태그로 맞추지 않는다). 단위의 성계가 영향권 안이거나 1레인 인접일 때만 건다(§16 #55 ②). 초월자 하나의 인수 = min(3, 값 × (1 + DT_Rules.SphereEventWeightStep × InfluencedSystems.Num())) — 상한 3은 이 인수 전체에 걸고, 인수는 0 아래로 내려가지 않는다(값이나 노브가 음수여도, 숫자가 아니어도 0으로 읽는다). 여러 초월자는 Transcendents 배열 순서로 곱한다. 봉인된 초월자는 걸지 않는다 (§4.2 영혼 경제 '사용')

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> PressureEffectsPerTick; // 영압 효과 ①: 침묵 = 수입 감소, 동화 = 윤리 이동, 광신 = 관계 변동, 회귀 = 인구 손실 완화·Insight 감소·Transcendence 0 (§4.2 교리 표 '사용 방식'). 틱당 원천이라 양을 고치는 효과 일곱만 담는다 — 그 밖은 `Apply`가 돌려보내고 Data.Integrity가 행을 막는다(§7.5). 적용의 주제는 그 문명과 영향권 안의 첫 정착된 성계(OwnedSystems 순서), 상대(OtherID)와 SourceRow는 이 초월자다. Celestial 행은 비어 있다

UPROPERTY(EditAnywhere) TArray<FText> VoiceTemplates; // 1인칭 화자 템플릿 (Realm.Abyssal만 1인칭, §8.2). trans_covenant는 듣는 문명의 RevelationTemplates로 말한다 (§5.11)

UPROPERTY(EditAnywhere) TArray<FText> ClaimTemplates; // Hazard.Claim 만료 시 1인칭 Log.Claim (Severity 1, MinTierToReveal 3, §4.2). 둘째 문장은 남은 자 문장 (Law.Remnant)

};

// DT_LogTemplates (§5.11 계시 양식, §8.2). 이벤트가 아닌 틱 로그(경제·연구·소유권 변동)와 방문·사자 템플릿. 만남의 줄(log_first_contact·log_contact), 먼 줄(log_far_ascent·log_far_event), 무주 성계 스캔(log_scan_wild, Phase 4), 건넘의 줄(log_crossing_depart·log_crossing_arrive·log_crossing_turned·log_crossing_word, Phase 6a)도 이 테이블의 행이다 (§5.7, §8.1). 행은 두 가지이고 Context가 가른다: Context가 빈 행은 **이름 행**이다 — PushRow가 행 이름으로 부르고 선택기는 보지 않는다(Phase 1의 다섯 행과 v0.7의 열 행이 모두 이것이다). Context가 Revelation.*인 행은 **선택 행**이다 — (Category, Context)마다 하나 이하(Data.Integrity). 선택기 UCosmicLogSubsystem::PickTemplateRow(Category, Context, FallbackRow)는 일치하는 선택 행의 이름을, 없으면 호출자가 넘긴 이름 행을 돌려준다. 폴백은 '빈 Context 행'이 아니라 호출자가 CosmicRows 상수로 넘기는 이름 행이다. 뽑지 않는다 — 변형 추첨은 그 이름으로 부르는 PushRow가 한다 (§9.2)

USTRUCT(BlueprintType)

struct FLogTemplateDef : public FTableRowBase

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) FGameplayTag Category; // Log.* (§8.1)

UPROPERTY(EditAnywhere) FGameplayTag Context; // 빈 태그 = 이름 행(PushRow). Revelation.* = 그 계시 양식의 선택 행(PickTemplateRow). 텍스트만 바꾼다

UPROPERTY(EditAnywhere) uint8 MinTierToReveal = 0; // 이 행의 줄이 말해지는 가장 낮은 보는 쪽 Tier(§8.1 보는 쪽 Tier의 창의 아래 끝 — PushRow가 FCosmicLine.MinViewerTier로 넘긴다). 하위 화자의 문장은 이 행이 아니라 이벤트 행의 SignRow가 가리키는 Log.Sign 행에 있다. 값은 Tier 번호 1~5, 0 = 누구에게나

UPROPERTY(EditAnywhere) TArray<FText> Templates; // 3개 이상(한 판에 한 번만 뜨는 고정 문장 행은 1개 허용 — 목록은 Tools/data/schema.json의 single_variant_rows: log_first_life·log_first_contact). 두 문장, 사실 → 함의 (§8.3). 서술자 카테고리에는 판정 어휘 금지 (Data.NoVerdictWords — Phase 5. 지금은 스키마의 forbidden: verdict)

};

// DT_Galaxies (§5.11). 은하 정의. 행 이름이 은하의 식별자이고 세이브에 닿으므로(GalaxyRow·OriginGalaxyRow·Transits) 배포 뒤 바꾸지 않는다. 20행: gal_milkyway(고향 은하 = DT_Rules.HomeGalaxyRow)·gal_andromeda·gal_triangulum·gal_lmc·gal_smc·gal_m87과 새 표시 전용 14행(Phase 3 9번 작업 — M87과 함께 표시 전용 15, §5.11의 표). 시뮬이 읽는 컬럼과 읽지 않는 표시 컬럼의 규칙은 §5.11에 있다(읽는 쪽은 아래 컬럼 주석의 '시뮬'·'표시'다). 컬럼의 형은 아래 그대로이고(은하 생성 규칙 변경의 헤더에서 확정), 테이블이나 고향 행이 없을 때 쓰는 C++ 기본값(멤버 초기화식)은 VS의 고향 값이다 — `LivingWorlds` 60·`Peoples` 12·`AIFourthRungCap` 2·`HomeRadiusShare` 0.53, 나머지는 0과 빈 값. 고향이 아닌 행은 네 시뮬 값에 0을 적는다. Phase 2

USTRUCT(BlueprintType)

struct FGalaxyDef : public FTableRowBase

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) FText DisplayName; // 은하의 통용 이름. 표시

UPROPERTY(EditAnywhere) FGameplayTag Kind; // Galaxy.Kind.Dwarf / Spiral / GrandSpiral / Elliptical. 은하 종류. 정의 전용이고 상태에 저장하지 않는다. 표시

UPROPERTY(EditAnywhere) int32 LivingWorlds = 60; // 생명 성계 수 = NewGalaxy의 성계 수 (§3.1). 고향 행: VS 60, EA 시작값 300. 시뮬(고향 행만)

UPROPERTY(EditAnywhere) int32 Peoples = 12; // 백성 수(플레이어 포함) = NewGalaxy의 문명 수. 고향 행: VS 12, EA 시작값 48. 시뮬(고향 행만)

UPROPERTY(EditAnywhere) int32 AIFourthRungCap = 2; // 넷째 칸의 제동 (§5.12. 셈의 정의는 §7.4 건넘 단계). 고향 행 2, 0 = 제동 없음. AI 간이 판정 전용이고 Phase 6a부터 유효하다 — 플레이어의 업적 판정은 읽지 않는다. 시뮬(고향 행만)

UPROPERTY(EditAnywhere) float HomeRadiusShare = 0.53f; // 첫 생애의 모성계 sys_0의 자리(임시 — v0.8 단계 5가 관측 좌표로 바꾼다. A0 4a 전에는 sol의 자리) = 값 × 원판 반지름, 각도 0 (§7.4). 고향 행 0.53. 0~1로 읽는다(밖의 값과 숫자가 아닌 값은 C++ 기본값으로 읽고 에러 한 줄). 시뮬(고향 행만)

UPROPERTY(EditAnywhere) int32 RadiusLy = 0; // 광년. 고향은 '지름 약 10만 광년'의 절반(우주 화면의 단위), 다른 행은 ½ × D25 × 중심 거리. 우주 화면의 그림 크기(CosmicUniverseLook::SpriteShareOf — 43,500 광년에 대한 몫의 제곱근)와 단계 5의 광년 눈금. 0이면 종류의 크기. 표시 — 시뮬은 읽지 않고 인쇄하지 않는다

UPROPERTY(EditAnywhere) int32 BridgeTicks = 0; // 다리 (§5.9): 고향 은하에서 건너는 틱. 0 = 다리 없음. 고향 행은 0, 음수 금지 (Data.Integrity). 레인이 아니고 상태가 없다. 표시, Phase 6a부터 건넘 단계가 읽는다

UPROPERTY(EditAnywhere) FText StarCountText; // 별의 수 문안('약'·'추정' 포함, §8.3). 빈 값 허용 — 비면 패널이 그 줄을 내지 않는다. 표시

UPROPERTY(EditAnywhere) FText DistanceText; // 거리 문안('약' 포함). 고향 행은 태양의 자리 문안. 표시

UPROPERTY(EditAnywhere) FText FeatureText; // 특징 문안. 빈 값 허용. 표시

UPROPERTY(EditAnywhere) FVector2D MapPosition = FVector2D::ZeroVector; // 우주 화면(같은 지도의 줌 단계)에서의 자리: 고향 은하 반지름 단위, 고향의 중심에서, 지도의 X·Y. 거리 1 + ln(d / 50,000 광년), 방향은 은하 경도가 카메라에서 시계 방향(X = −r cos l, Y = −r sin l), 겹치면 손으로 옮긴다(출처와 손 고침은 Tools/data/sources/galaxies.md). 고향은 무엇을 들든 가운데, 다른 행은 (0,0)·NaN·1단위 안·20단위 밖이면 그리지 않는다(CosmicUniverseLook::HasPlace). 표시, 해시 밖

UPROPERTY(EditAnywhere) TArray<FText> StarNameBank; // 백성 모성계의 고유명. 고향 행 60칸, 비복원 (§5.11). 시뮬(고향 행, 생성) (v0.8 — 단계 5에서 은퇴하거나 6a의 건너온 백성용으로 남긴다. 남길 때 '알골'을 뺀다 — '란'은 지금 뱅크에 없다)

UPROPERTY(EditAnywhere) TArray<FText> CatalogPrefixes; // 무주 성계 이름의 앞말(그리스 문자). 고향 행 24칸. 시뮬(고향 행, 생성)

UPROPERTY(EditAnywhere) TArray<FText> CatalogStems; // 무주 성계 이름의 뒷말(별자리). 고향 행 30칸 — 앞말과의 조합 720, 비복원. 시뮬(고향 행, 생성)

UPROPERTY(EditAnywhere) TArray<FText> PeopleStems; // 그 은하에서 건너온 백성의 이름 줄기. 다리가 있는 4행에 8칸씩. Phase 6a의 도착 처리가 읽는다

};

// DT_Rules (§10.1). 단일 행. 튠 값과 다시 쓸 수 없는 법

USTRUCT(BlueprintType)

struct FCosmicRulesDef : public FTableRowBase

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) int32 LeapTicksMin = 800; // 공백기 도약 (§4.4)

UPROPERTY(EditAnywhere) int32 LeapTicksMax = 1600;

UPROPERTY(EditAnywhere) int32 LeapTotalCap = 4500; // 판 합계 상한 = 18,000의 25%

UPROPERTY(EditAnywhere) int32 BirthProtectionTicks = 300; // 출생 보호. 치명 효과 무시

UPROPERTY(EditAnywhere) int32 BaseLifespanTicks = 0; // 0 = 노화 OFF (VS). DT_Traits.LifespanTicksMod와 합산

UPROPERTY(EditAnywhere) float SuccessionHazardWeight = 2.f; // 계승 추첨: 2레인 안 활성 Hazard당

UPROPERTY(EditAnywhere) float SuccessionCivTierWeight = 1.f; // 소유 문명 Tier >= 2

UPROPERTY(EditAnywhere) float SuccessionReverenceWeight = 2.f; // 소유 문명 Reverence > SuccessionReverenceThreshold

UPROPERTY(EditAnywhere) float SuccessionReverenceThreshold = 0.3f; // 위 가중치의 Reverence 임계

UPROPERTY(EditAnywhere) float SuccessionSameCivPenalty = 1.f; // 직전 생애와 같은 문명 감점

UPROPERTY(EditAnywhere) int32 SuccessionMinHazardTicks = 300; // 제외: Hazard 카운트다운이 이 값 미만인 성계

UPROPERTY(EditAnywhere) float ReverenceDecay = 0.5f; // 사망·폐위 시 Reverence 배율

UPROPERTY(EditAnywhere) float AIAscensionSpeedMultiplier = 1.f; // §4.3 난이도

UPROPERTY(EditAnywhere) float PostRebellionStability = 0.15f; // 폐위 직후 Stability 상한 (§5.10)

UPROPERTY(EditAnywhere) int32 ThroneLockTicks = 600; // 왕좌 잠금

UPROPERTY(EditAnywhere) TArray<float> PressureBySourceTier = {0.f, 0.f, 0.25f, 1.f, 2.f}; // 인덱스 = Tier 번호 − 1 = static_cast<int32>(EAscensionTier). 정확히 5개 (§4.2, Data.Integrity)

UPROPERTY(EditAnywhere) float PressureAdjacencyFalloff = 0.5f; // 1레인 인접

UPROPERTY(EditAnywhere) float SpiritualPressureStabilityDrain = 0.002f; // 틱당 × P

UPROPERTY(EditAnywhere) float PressureFleetAttrition = 0.01f; // 틱당 × P

UPROPERTY(EditAnywhere) TArray<float> FleetEffectivenessByTierGap = {1.f, 0.5f, 0.f}; // 인덱스 = 격차(0, 1, >=2). 정확히 3개 (§5.6, Data.Integrity)

UPROPERTY(EditAnywhere) float FleetDelayPerPower = 50.f; // 함대 투입 1.0당 Hazard 카운트다운 +틱

UPROPERTY(EditAnywhere) float FleetDelayCap = 100.f;

UPROPERTY(EditAnywhere) FGameplayTagContainer ImmutableLaws; // Law.Judgment, Law.Sabbath, Law.Humility, Law.Remnant, Law.Hedge(성역 — A0 4a부터. 스키마 규칙 contains_all이 다섯을 모두 요구한다) (§4.5)

UPROPERTY(EditAnywhere) int32 FallenTranscendentCount = 3; // 판당 활성(미봉인) 타락 초월자 수 — 은하가 늘어도 판 전체의 수다 (§4.2). VS 1(DT_Rules.json의 데이터 값이고 코드 기본값은 3이다 — 둘 다 그대로 둔다), EA 3. 0~3으로 읽는다: 밖의 값은 NewGalaxy가 에러 한 줄을 내고 잘라 읽는다(§4.2 '판당 활성 타락 ≤ 3'). 침묵 고정 + 나머지는 SpawnWeight 가중 추첨 (§4.2, §7.4 NewGalaxy)

UPROPERTY(EditAnywhere) FName PlayerArchetypeRow = TEXT("arch_seeker"); // 플레이어 첫 생애 문명의 DT_Civs 행 = 첫 생애 행 (§5.10, §5.11). Data.Integrity: 행 존재, 스키마 규칙: arch_seeker (A0 4b — C++ 기본값과 DT_Rules 값이 함께 바뀌었다. `CosmicRows::ArchSeeker`)

UPROPERTY(EditAnywhere) int32 ClaimAdjacencyLanes = 1; // 주장 발생 조건 (a): 타락 영향권에서 이 레인 수 이내. (옛 ClaimCountdown은 위기 처리 작업에서 지웠다 — 주장도 제 행의 CountdownTicks로 센다)

UPROPERTY(EditAnywhere) int32 VigilClaimExtension = 100; // edict_vigil 활성 중 Hazard.Claim 카운트다운 연장 (§5.8)

UPROPERTY(EditAnywhere) float SphereEventWeightStep = 0.1f; // 영향권 1성계당 교리 이벤트 가중 승수 증가분. 승수 상한 3 (§4.2 '사용')

UPROPERTY(EditAnywhere) int32 EventSlotTicks = 10; // 정착된 성계를 가진 문명마다 이 틱 수에 한 번 이벤트 슬롯 (§7.5). 1~20 (Data.Integrity — 20을 넘으면 반란 전의 웅성거림 두 줄이 보장되지 않는다, §4.1. 코드는 1 미만을 1로 읽는다). Phase 2

UPROPERTY(EditAnywhere) float EventChancePerSlot = 0.02f; // BaseWeight 1인 Lottery 행이 자격 있는 단위에 대해 슬롯당 발화할 확률. 잠정 시작값 — 한 백성에게 일어나는 일의 총량은 자격 있는 Lottery 행·단위의 수에 비례하므로 행 묶음이 들어올 때마다 다시 맞춘다(§16 #54). 0 이상(Data.Integrity. 코드는 0 미만과 수가 아닌 값을 0으로 읽는다). Phase 2

UPROPERTY(EditAnywhere) int32 ReturnLureMaxLanes = 2; // 귀환 제안 게이트: 사망 성계가 회귀 영향권에서 이 레인 수 이내 (§4.4 회귀의 유혹)

UPROPERTY(EditAnywhere) int32 ReturnedLivesToSeize = 3; // Succession.Returned 생애 수가 이 값에 이르면 GrantPower Way.Seized (Law.Humility로 영구)

UPROPERTY(EditAnywhere) TMap<FName, float> LureAcceptByPolicy = {{TEXT("Cautious"), 0.10f}, {TEXT("Reckless"), 0.60f}, {TEXT("Faithful"), 0.f}, {TEXT("Tribute"), 0.85f}}; // 커맨드렛 -PlayerPolicy별 귀환 제안 수락 확률 (§9.3)

UPROPERTY(EditAnywhere) int32 MaxNotablesPerCiv = 4; // 문명당 인물 기록 상한 (§1.3, §7.2). Data.Integrity: 3~5. 초과 시 Apply 퇴출 규칙 (§7.5). 폭주를 막는 것은 상한이 아니라 '역할당 열린 기록 1개'이고, 상한은 닫힌 기록의 보존 길이를 정한다

UPROPERTY(EditAnywhere) TArray<float> PopulationBandThresholds = {0.15f, 0.5f, 2.0f}; // 규모 등급 보정 (§8.3, GetPopulationBand §7.4): Population < [1]이면 Tier 기준 등급에서 한 단계 아래, < [0]이면 Few, > [2]이면 한 단계 위(상한 Countless). 오름차순 (Data.Integrity). 값은 여기 한 곳에만 둔다
UPROPERTY(EditAnywhere) float PopulationRegrowthPerTick = 0.0002f; // 인구 회복 (§4.1, §7.4 경제 단계 — 정착과 문명 제거 작업): 틱마다 수입을 셈하기 전에, Population이 1.0 아래인 백성은 이만큼 오르고 1.0에서 멈춘다(1.0 위는 그대로). 0 이하이거나 숫자가 아니면 회복 없음. 시작값 — 역병 하나의 손실 0.4가 2,000틱에 돌아온다(Phase 5에 맞춘다)

// 은하·층·앎·건넘의 노브(v0.7 추가분). 전부 시작값이다. 정의 데이터라 세이브에 남지 않으므로 처음 읽는 Phase에 넣는다 (§11)

UPROPERTY(EditAnywhere) FName HomeGalaxyRow = TEXT("gal_milkyway"); // 고향 은하의 DT_Galaxies 행 — 상세로 시뮬되는 유일한 은하 (§5.11, §9.2). NewGalaxy가 모든 성계·문명의 GalaxyRow에 이 이름을 찍는다. 배포 뒤 바꾸지 않는다. Data.Integrity: 행 존재. Phase 2

UPROPERTY(EditAnywhere) TArray<int32> RealmCapByTier = {1, 3, 4, 7, 10}; // 보유 상한 (§5.12): 정착은 소유 성계 수가 이 값 미만일 때만 된다(CosmicMegaprojects::RealmCapOf — 배열에 그 칸이 없으면 0). 인덱스 = static_cast<int32>(EAscensionTier). 정확히 5개, 오름차순 (Data.Integrity). 법이 아니라 노브이고 AI 문명과 플레이어가 통치하는 문명에 같다. Phase 2

UPROPERTY(EditAnywhere) TArray<int32> SightLanesByTier = {2, 3, 4, 6, 10}; // ({1, 2, 4, 6, 10}에서 MVP ③이 첫 두 칸을 넓혔다 — 첫 600틱의 말해진 줄, 헤더 A3 §7) 보는 거리(레인 수, §7.4 앎 단계). 인덱스 = static_cast<int32>(EAscensionTier). 정확히 5개, 모든 칸 >= 1 (Data.Integrity). 앎 쪽 노브 — 세계와 생성 규칙은 읽지 않는다. Phase 2

UPROPERTY(EditAnywhere) int32 SightIntervalTicks = 10; // 앎 단계의 주기 (§7.4). Phase 2

UPROPERTY(EditAnywhere) int32 FirstSightTick = 330; // 통치자가 소유 성계에서 보기 시작하는 틱(1배속 5:30). 방랑자의 눈은 늦추지 않는다 (§7.4). Phase 2

UPROPERTY(EditAnywhere) TArray<float> LivingWorldStarShares = {0.01f, 0.05f, 0.14f, 0.31f, 0.57f, 0.82f, 1.0f}; // 생명 성계의 항성 등급 분포. 누적 7칸 O~M(= O 1 · B 4 · A 9 · F 17 · G 26 · K 25 · M 18%), 오름차순·끝 1.0 (Data.Integrity). FRand 한 번과 비교한다 (§7.4). 플레이를 위한 설계 값이다 — 현실의 분광형 비율이 아니다 (§3.4). Phase 2 (v0.8 — 단계 5에서 은퇴)

UPROPERTY(EditAnywhere) int32 FallenCoreMinLanesFromHome = 3; // 초기 코어의 자리 규칙: 첫 생애의 모성계(sys_0)에서 이 레인 수 이상 떨어진 곳 (§5.7). 첫 10분 완화 ③의 기제 (§5.11). Phase 2

UPROPERTY(EditAnywhere) int32 FarLineCooldownTicks = 300; // log_far_event 사이의 최소 간격. log_far_ascent에는 걸리지 않는다 — 매번 낸다 (§8.1). Phase 2

UPROPERTY(EditAnywhere) int32 BackgroundStarCount = 20000; // 배경 별의 점 수. 표현 계층만 읽는다 — 0~100,000으로 읽는다(`CosmicMapLook::BackdropCountOf`, 스키마도 같은 경계) (§3.1, §6.4). Phase 3 8번 작업에서 코드에 들어왔다

UPROPERTY(EditAnywhere) int32 PetitionPeriodTicks = 300; // 청원의 주기 슬롯(MVP ②, §5.10 — 600에서 MVP ③이 300으로): Tick − Calling.BornTick이 이 값의 양의 배수인 틱에, 그 틱에 막 선 Standing 행이 없으면 앎 단계가 Lottery 행 하나를 내놓는다. 1 아래는 1로 읽는다(스키마는 1 이상)
// ---- MVP ③(2026-10-22, 헤더 docs/60_DECISIONS/headers/A3_first_ten_minutes.md) — 구조체의 끝에 다섯 ----
UPROPERTY(EditAnywhere) float StabilityRecoveryPerTick = 0.001f; // 안정도 회복(§4.1, §16 #68): 경제 단계에서 인구 회복 바로 뒤(수입 앞), StabilityRestingLevel 아래의 백성은 이만큼 오르고 그 수준에서 멈춘다. 위는 그대로(카드·이벤트가 올린 몫은 남는다). 같은 틱의 LowStabilityTicks 셈에 든다. 0 이하·1 초과·숫자가 아니면 회복 없음. 시작값 — 영향권 곁의 소모(0.002 × P)보다 작게(0.002면 주장 정착률이 22%로 떨어진다 — 헤더 A3 §1)
UPROPERTY(EditAnywhere) float StabilityRestingLevel = 0.6f; // 회복이 멈추는 수준(§9.3 '자연 수렴 0.6'). 0~1 밖이거나 숫자가 아니면 회복 없음
UPROPERTY(EditAnywhere) FGameplayTagContainer ClaimLeaveProfiles; // AI 구호의 정책(§16 #71): AI 단계에서 이 프로필의 백성은 제 성계에 선 Hazard.Claim의 답을 시작하지 않는다. 다른 답과 다른 프로필(빈 프로필 포함)은 그대로. 통치자의 프로젝트 키는 읽지 않는다. 비면 모두 구호한다. 추첨 없음 — 프로필은 생성과 왕좌 교체가 이미 뽑았다. 데이터: AI.Profile.Expansionist, AI.Profile.Militarist
UPROPERTY(EditAnywhere) int32 FirstCrisisTick = 150; // 첫 위기(§2.4 첫 10분, §16 #91): Tick == 이 값인 틱의 위기 단계 발생 끝(추첨과 수도 규칙 뒤, 이벤트 슬롯 앞)에 Civs 배열 순서로 모든 백성의 수도(소유 ∧ 정착)에 FirstCrisisHazard의 Draw 행을 CosmicHazards::Place로 놓는다. 슬롯이 찬 수도는 그 위기를 지킨다(그것이 첫 위기다). 모든 백성 같은 규칙. 0 이하 = 없음. 추첨 없음
UPROPERTY(EditAnywhere) FGameplayTag FirstCrisisHazard; // DT_Hazards의 Draw 행의 태그(Data.Integrity). 다른 행의 태그는 에러 한 줄과 함께 거절, 그 행이 없는 테이블(테스트)은 아무것도 놓지 않는다. 비면 없음(C++ 기본값). 데이터: Hazard.AIAwakening
UPROPERTY(EditAnywhere) int32 ProvisionalEndTick = 6000; // MVP ④(2026-10-23, 헤더 docs/60_DECISIONS/headers/A4_provisional_end.md) 임시 끝의 시계(CosmicEndings::ProvisionalEndOf): 다스리는 백성이 먼저 셋째 칸에 오르지 않았다면 Tick이 이 값에 이른 틱 경계부터 표현 계층이 시간을 멈추고 연대기를 연다 — 판의 마지막 틱이 이 틱이다. 0 이하 = 시계 끝 없음. 시뮬은 여기서 멈추지 않는다(커맨드렛은 알리기만 한다). 시작값 6,000 = ×10에서 쉬지 않고 10분, 시계로 끝나는 판만 늘린다. 상단 바의 시계 툴팁도 읽는다. 네 엔딩(B5·B6)과 함께 간다. 구조체의 맨 끝 컬럼

UPROPERTY(EditAnywhere) int32 SceneryGalaxyCount = 10000; // 풍경 은하의 점 수. 표현 계층만 읽는다(CosmicUniverseLook::SceneryCountOf — 0~50,000으로 읽는다, 스키마도 같은 경계) (§5.11, §6.4 '우주 화면'). Phase 3 9번 작업

UPROPERTY(EditAnywhere) int32 ScanSightBonusLanes = 1; // 스캔이 그 자리에서 보는 거리에 더하는 레인 수 (§4.4). Phase 4

UPROPERTY(EditAnywhere) int32 CrossingMinTier = 4; // 건넘의 최소 칸. 값은 Tier 번호 (§5.7 건넘). Phase 6a

UPROPERTY(EditAnywhere) int32 CrossingCooldownTicks = 4000; // 한 백성의 건넘 사이 간격 (§5.7 건넘). Phase 6a

UPROPERTY(EditAnywhere) int32 CrossingSettleTicks = 3000; // 건너온 백성의 T4→T5 판정 유예 (§5.7 건넘). Phase 6a

};

// DT_Dedications (§8.6). Phase 4: Epithet 10행 최소형 (§5.10 시대 명명, 로그 슬롯 {Epithet}) + 첫 생애 백성의 의회·회중 헌정명 최소형. Phase 6+: 헌정명 뱅크 행 (Bank 태그 Faith.Bank.Hebrew / Civic / Nations … + 트리거 종류 + 이름 FText 묶음). 구조는 뱅크 작성 시 확정. 로그 슬롯 {Dedication}

// DT_Megaprojects (§5.7): FMegaprojectDef : FTableRowBase { DisplayName, RequiredTier(EAscensionTier — 시작할 수 있는 주인의 칸), CostPerTick(FResourcePool — 낸 틱마다 전액. 0 아래이거나 숫자가 아닌 값은 0), RequiredProgress(낸 틱마다 1, AI 백성은 AIAscensionSpeedMultiplier. 1 미만이거나 숫자가 아니면 1 — CosmicMegaprojects::RequiredProgressOf, 패널도 같은 수), CompleteTemplates(완성 줄 — Log.Civ, 슬롯 {Civ} {System} {Project}) }. 옛 스케치의 TargetHazard는 Hazard 행의 RulerResolutions가 대신한다(그 목록에 든 행이 대응 프로젝트). DedicationBank는 DT_Dedications와 함께 Phase 4. 행(거대 프로젝트와 승천 판정 작업): project_dyson(다이슨 스웜, 건설 칸)·project_stabilize(항성 안정화)·project_reform(개혁)·project_relief(구호) — 뒤의 셋은 대응 칸, 값은 잠정이고 Phase 5에 맞춘다. 그리고 project_settle(정착 — 무주 성계를 제 것으로 삼는다. 대상 성계의 건설 칸에서 돌아 업적 프로젝트와 겹치지 않는다. Start는 거절하고 CosmicMegaprojects::Settle로만 시작하며, 진척·완성은 다른 건설 행과 같다 — 정착과 문명 제거 작업)·project_crossing(먼 항로 — 건넘의 수단. 이 구조체 그대로이고 수도의 건설 칸에서 돈다, Phase 6a) (§5.7) — DT_Edicts·DT_Relics·DT_Traits의 정의 구조체 스케치도 Phase 4 행 작성 시 추가 (§5.3, §5.7, §5.8, §5.9)

// DT_Traits 컬럼 추가분 (§5.9): int32 BodyMod, MindMod, CausalityMod, LifespanTicksMod. VS 신설 행 (§5.3, §5.11): Trait.Society.Divided, Trait.Faith.Book, Trait.Faith.BookShut, Trait.Biology.Shortlived. Trait.Faith.Remnant/Covenant/Exile 행에 Hazard.Claim 발생 가중·카운트다운 보정 (§4.2)

```

**정의 스키마**: 파일 하나 `Tools/data/schema.json`이 선언한다 — 행 이름·태그·텍스트를 담는 모든 컬럼이 무엇을 가리킬 수 있는지(대상 테이블 또는 태그의 뿌리), 조건·효과 열거자 48개(`EConditionType` 30 + `EEffectType` 18 — 거대 프로젝트와 승천 판정 작업 뒤) 각각의 Tag·Key·Value 쓰임, 지금은 맨 이름으로만 있는 닫힌 낱말 목록. 저작 시점 전용이다: 런타임 구조체·필드·해시·세이브가 바뀌지 않는다. 검사는 `Tools/make_data_tables.py`가 에셋을 만들 때 하고, C++ 테스트 하나가 로드한 에셋을 다시 본다(§9.3 `Data.Integrity`). 규칙 종류는 닫힌 목록이고 새 종류는 §12.3을 거친다. Phase 2의 스키마 검사기 작업에서 만들었다(§11). 행 이름의 접두어와 테이블을 가로지른 유일성은 §10.4에 적는다.

**스키마의 꼴.** 스키마에는 행 구조체가 있는 테이블만 적는다(지금 열: `DT_Rules`·`DT_Civs`·`DT_LogTemplates`·`DT_Events`·`DT_Galaxies`·`DT_Transcendents`와, 위기 처리 작업에서 컬럼을 적은 `DT_Hazards`와, 거대 프로젝트와 승천 판정 작업에서 컬럼을 적은 `DT_Megaprojects`·`DT_Feats`와, 컬럼을 아직 적지 않은 `DT_Encounters`. `DT_Edicts` 같은 테이블은 구조체와 함께 온다).

- `tables`: 테이블마다 `struct`(행 구조체 이름에서 F를 뺀 것), 행 이름의 꼴(`prefixes` — 접두어, 또는 `names` — 그 이름만), `must_exist`(원본과 에셋이 반드시 있어야 하는가 — 에셋이 생긴 테이블은 켠다), `required_rows`(코드가 이름으로 부르는 행), `columns`, `rules`, `unique`, `pairs`.
- `columns`: 행 구조체의 **모든** 컬럼을 적는다 — 스키마에 없는 컬럼과 구조체에 없는 컬럼은 둘 다 문제다. 컬럼마다 종류가 하나다. 행이 아직 없는 테이블은 `null`이고 첫 행을 쓸 때 적는다(`null`인 테이블에 원본이나 에셋이 있으면 문제다).
- 컬럼의 종류(닫힌 목록 18)와 종류마다 쓸 수 있는 선택 항목(이것도 닫힌 목록이고, 검사기 머리의 `COLUMN_OPTIONS`가 쥔다): `pool`(자원 풀 `FResourcePool` — 낱말 목록 `resource`의 다섯 멤버를 빠짐없이, 다른 키 없이 수로 적는다. `min`. `Data.Integrity`가 그 목록을 구조체의 멤버에 맞춘다 — MVP ② A-2a), `structs`(구조체의 배열 — `columns`(원소의 멤버마다 지금의 종류와 선택 항목, `structs`는 다시 담지 않는다)·`min`·`len_in`. 원소는 `columns`와 정확히 같은 키를 갖는다: 빠진 멤버가 C++ 기본값으로 조용히 남지 않는다. 행 규칙은 하위 열을 `List.Column`으로 부르고 모든 원소에서 성립해야 참이다(빈 배열은 참). `DT_Encounters.Choices` — MVP ② A-2a), `plain`(검사하지 않는다 — bool·색·그 밖의 구조체. 선택 항목도 시험도 없다: 구조체에는 `is`·`in`이 늘 거짓이고 `set`이 늘 참이라 규칙을 조용히 끈다. bool 컬럼에 시험이 필요해지면 그때 종류를 더한다), `number`(`min`·`max`·`integer`), `numbers`(`len`·`ascending`·`ends_at`(마지막 칸의 값 — 누적 분포의 끝)·`min`·`max`·`integer`), `text`(`slots`·`no_digits`·`required`·`hedged`(수가 든 마디마다 이 낱말 가운데 하나를 제 낱말로 품는다 — '약'·'추정'·'논쟁 중'. 수는 어느 문자의 숫자든 되고, '개'·'광년' 앞의 한글 수 낱말(십·백·천·만·억·조 — '수억 개'·'십만 광년')도 수다. 'M87'·'NGC 205' 같은 목록 번호는 이름이라 수가 아니다. 마디는 ', '·' — '·'; '에서 나눈다(천 단위 쉼표는 뒤에 빈칸이 없어 마디를 나누지 않는다). 앞에 한글 음절이 붙으면 다른 낱말이고('요약'), 한 음절 낱말은 뒤에 붙어도 다른 낱말이다('약한' — '추정치'·'추정된'은 든다). §8.3은 수마다 붙이라고 하고, 검사기는 마디마다 본다)·`forbidden`(이 낱말을 품지 않는다)), `texts`(`min`·`len_in`·`empty_or_min`·`unique`·`slots`·`no_digits`·`single_variant`·`hedged`·`forbidden`), `tag`(`root`·`optional`), `tags`(태그 컨테이너 — `root`), `tag_list`(태그 배열 — `root`), `tag_map`(키가 태그, 값은 수 — `root`·`min`·`max`·`integer`), `row`(`table`·`optional`·`target` — 가리키는 행이 맞아야 하는 시험), `row_map`(키가 행 이름, 값은 수 — `table`·`target`·`min`·`max`·`integer`), `word_map`(키가 `words`의 낱말, 값은 수 — `words`·`min`·`max`·`integer`), `enum`(`enum`), `conditions`, `effects`(`per_tick` — 틱당 원천의 효과 일곱만, §7.5), `rows`(행 이름의 배열 — `tables`·`unique`. 원소마다 `tables`에 적은 테이블 가운데 하나의 행이고, `unique`면 한 배열에 두 번 쓰지 않는다. `DT_Hazards`의 해결 수단 두 컬럼 — 위기 처리 작업). 정수 프로퍼티의 컬럼에는 `integer`를 적는다(빠지면 `Data.Integrity`가 실패한다).
- `rules`: '`when`이 다 맞는 행은 `then`도 다 맞는다'. 시험은 닫힌 목록이다 — 컬럼 하나에 `is`·`in`·`gt`·`ge`·`lt`·`le`·`set`·`min_len`·`max_len`·`each_contains`·`lacks_type`(조건·효과 배열에 그 열거자가 없다)·`slots_in`(텍스트의 슬롯이 그 목록 안에 있다)·`len_ge_col`(목록의 길이가 같은 행의 수 컬럼 이상)·`pairs_ge_col`(두 목록의 길이를 곱한 것이 같은 행의 수 컬럼 이상 — 짝으로 나눠 주는 이름 풀) 가운데 하나, 또는 행 이름을 짚는 `row`. 시험마다 쓸 수 있는 컬럼의 종류가 정해져 있다(크기 비교는 `number`에만, `each_contains`는 `texts`에만, `is`·`in`은 값 하나짜리 컬럼에만 — 컨테이너에 쓴 `is`는 늘 거짓이라 규칙이 꺼진다. 이름 풀 시험 `len_ge_col`·`pairs_ge_col`은 목록·맵 컬럼에만 쓰고 짚는 수 컬럼은 `number`다 — 태그 컨테이너 `tags`는 export 텍스트라 길이가 없어 늘 거짓이므로 쓸 수 없다. `set`은 `number`·`enum`에 쓸 수 없다 — 늘 참이다. `plain`에는 어떤 시험도 쓸 수 없다). `unique`는 컬럼 묶음의 값이 테이블 안에서 겹치지 않음을, `pairs`는 두 컬럼의 허용 짝(이벤트 행의 `Category` → `LogCategory`, §8.1)을 적는다.
- `conditions`·`effects`: 열거자마다 `tag`(`root`·`optional`)·`key`(`word` — 낱말 목록, `row` — 행 이름과 그 행이 맞아야 하는 `target`, `digits` — 숫자만, `any` — 아무 이름. `optional`)·`value`(`min`·`max`·`integer`)를 적는다. 적지 않은 칸은 비어 있어야 한다 — 읽지 않는 칸에 쓴 값은 오타다. 효과에는 `per_tick_only`(틱당 원천만 거는 효과 — `MultiplyIncome`)를 더 적을 수 있고, `DurationTicks`는 틱당 원천의 효과에만 0보다 클 수 있다. 원본의 조건·효과 객체는 구조체의 멤버를 모두 적는다(Type·Tag·Key·Value, 효과는 DurationTicks도) — 빠진 멤버와 없는 멤버는 문제다.
- 그 밖: `words`(닫힌 낱말 목록 일곱 — resource·axis·stat·role·civ_selector·policy·research), `enums`(컬럼이 쓰는 열거형의 열거자), `per_tick_effects`(틱당 원천의 효과 일곱), `required_tags`(ini에 선언되어 있어야 하는 태그 — 지금은 §9.3 v0.6 항목의 `Notable.*` 역할 태그 여섯), `row_name_sources`(코드가 이름으로 부르는 행을 적어 둔 헤더 `Data/CosmicRowNames.h` — 검사기와 `Data.Integrity`가 그 이름들을 `required_rows`와 양쪽으로 맞춘다. 주석으로 막은 상수는 세지 않는다), `single_variant_rows`, `event_log_categories`, `row_name_pattern`, `tag_sources`(태그를 선언하는 ini).
- 숫자의 범위는 이 문서가 정한 것만 적는다. 범위가 없는 노브는 `number`만이고 '유한한 수'인지만 본다.

**도구.** `Tools/cosmic_data_schema.py`가 검사기다 — 에디터 없이 도는 Python이고 `Tools/data`의 원본(`schema.json`, `DT_*.json` 전부, `ST_*.csv`)을 본다. 세 걸음이다.

- 읽기: JSON 문법 오류, 한 객체에 두 번 적힌 키(뒤의 것이 앞의 것을 지운다), NaN·Infinity는 파일 이름과 함께 문제 줄이다. 읽기에서 문제가 나면 거기서 멈춘다.
- 스키마 자신: 모르는 종류·시험, 아무도 읽지 않는 키(철자가 틀린 선택 항목, 다른 종류의 선택 항목), 타입이 틀린 값(`"min": "3"`, 문자열로 적은 목록), 스키마에 없는 테이블·낱말 목록·열거형, 규칙이 짚는 없는 이름(선언하지 않은 컬럼, ini에 없는 태그, 없는 열거자, 그 테이블에 없고 `required_rows`에도 없는 행), 그 컬럼의 종류에 쓸 수 없는 시험(숫자·열거형 컬럼의 `set`처럼 늘 참인 것도), `CosmicRowNames.h`와 어긋난 `required_rows`. 틀린 철자나 틀린 타입 하나가 검사를 조용히 끄지 못하게 하려는 것이다(`target`의 시험은 가리키는 테이블의 컬럼으로 본다). 스키마에 문제가 하나라도 있으면 데이터는 보지 않는다 — 스키마를 먼저 고친다.
- 데이터: 스키마의 규칙 전부. 단일 태그는 문자열로만 적는다(객체 꼴은 문제 — 규칙과 `unique`가 한 가지 꼴만 읽는다). 태그 컨테이너는 export 텍스트 꼴 그대로여야 한다(따옴표가 빠지거나 철자가 틀린 항목은 임포터가 조용히 버린다). 텍스트의 슬롯은 엔진이 읽는 대로 읽는다(여는 중괄호부터 다음 닫는 중괄호까지가 이름, 백틱은 탈출 — 짝 없는 중괄호는 문제다). 태그는 ini에서 엔진이 읽는 줄만 센다(`DefaultGameplayTags.ini`는 섹션 `[/Script/GameplayTags.GameplayTagsSettings]` 아래의 `+` 줄, `Config/Tags` 아래(하위 폴더까지)의 ini는 섹션 `[/Script/GameplayTags.GameplayTagsList]` 아래의 `+` 없는 줄 — 반대 꼴이나 다른 섹션 아래의 줄은 문제 줄). String Table 원본은 꼴만 본다(머리줄, 칸 수, 빈 문장, 빈 키와 겹친 키).

`Tools/make_data_tables.py`는 원본을 한 번만 읽는다 — 읽은 그 내용을 검사하고, 임포트하고, 도장에 해시한다. 에셋에 손대기 전에 검사기로 원본을 보고 이어서 검사기의 단위 테스트를 돌린다 — 어느 쪽이든 문제가 하나라도 있으면 멈춘다(원본이 먼저인 까닭: 단위 테스트는 프로젝트의 데이터에서 출발하므로 데이터가 틀리면 함께 실패하고, 무엇이 틀렸는지 말하는 줄이 그 뒤에 묻힌다. 도장도 남지 않는다. 폴더의 테이블 원본과 스크립트가 만드는 테이블, 스키마의 `struct`가 서로 맞는지도 여기서 본다). 테이블은 전부 채워서 견준 뒤에 한꺼번에 저장한다(뒤 테이블의 실패가 앞 테이블만 저장된 상태를 남기지 않는다). 도장에는 원본과 `schema.json`과 검사기 파일이 든다: 검사기를 고치면 에셋을 다시 만들어야 `Data.SourcesInSync`가 통과하고, 그때 단위 테스트가 돈다. `python Tools/check_data.py`는 같은 검사를 에디터 없이 돌린다(1초 안쪽 — JSON을 고친 뒤 먼저 돌린다). `python Tools/test_data_schema.py`는 검사기의 단위 테스트다: 검사마다 틀린 입력 하나를 넣어 문제 줄이 나오는지 보고, 프로젝트 스키마의 `rules`·`unique`·허용 짝·`target`·`slots` 하나하나에 그것을 깨는 입력이 있는지도 본다(깨는 입력이 없는 규칙은 테스트가 실패한다 — 규칙을 더하면 깨는 입력도 더한다).

**누가 무엇을 보는가.** 원본은 Python이 전부 본다. C++ 테스트 `Data.Integrity`(§9.3)는 Python이 볼 수 없는 것(리플렉션, 코드의 목록)과, 원본 없이 에셋만 달라졌을 때 실행 중에 조용히 깨지는 것을 본다.

| 보는 것 | Python (원본 JSON) | C++ `Data.Integrity` |
|---|---|---|
| 스키마가 코드와 같은가 — 행 구조체의 모든 컬럼과 그 종류(정수 프로퍼티에는 `integer`), 조건·효과·`enums`의 열거자, `per_tick_effects`와 `Apply`의 목록, 낱말 목록(자원·역할·윤리 축·정책은 코드의 목록과 양쪽으로, 능력치·문명 선택자·연구 영역은 평가기가 그 낱말을 푸는지로), 열거형 컬럼의 `enum`이 그 프로퍼티의 열거형인가, `required_tags`, `required_rows`와 `CosmicRowNames.h`(양쪽으로), 로더(`FCosmicDataTables`)가 든 테이블이 모두 스키마에 있고 로드된 테이블에는 `must_exist`가 켜져 있는가 | 원본 행의 컬럼이 스키마와 같은지, `required_tags`, `required_rows`와 `CosmicRowNames.h` | ○ (이름은 대소문자까지 견준다 — 컬럼 이름과 테이블 키도. 능력치·문명 선택자·연구 영역의 낱말만 평가기가 읽는 대로 대소문자 없이 본다. `CosmicRowNames.h`는 C++ 테스트도 읽는다: 헤더만 고치면 원본이 그대로라 Python 검사기가 돌지 않는다) |
| 행 이름 — 꼴, 접두어, 코드가 부르는 행, 테이블을 가로지른 유일성 | ○ | ○ (로드한 에셋) |
| 행 참조, 태그(ini에 선언되었는가·뿌리 — 조건·효과 안의 태그는 선언만), 숫자의 범위, 숫자 배열의 길이·오름차순·끝 값, 텍스트 배열의 개수, `word_map`의 키 | ○ | ○ (로드한 에셋) |
| 텍스트의 슬롯(과 그 뒤의 조사 수식어 — `\|hpp` 하나, 짝 하나, 받침 있는 꼴이 먼저)과 숫자, `rules`·`unique`·`pairs`·`target`, 조건·효과의 Tag·Key·Value 쓰임 | ○ | — |

표의 마지막 줄을 C++ 쪽이 다시 보지 않는 까닭: 에셋은 원본에서만 만들고(`Data.SourcesInSync`가 원본이 바뀌었는데 에셋을 다시 만들지 않은 상태를 잡는다) 에디터에서 손으로 고친 행은 스크립트를 다시 돌리면 덮어써진다. 그 사이에 손으로 고친 에셋은 이름·참조·태그·숫자까지만 잡힌다. 태그가 ini에 '선언'되었는지(자식이 있어 암시만 된 부모가 아닌지)는 에디터 바이너리만 안다 — 테스트는 에디터 바이너리에서 돈다. ini는 도장에 들지 않는다(태그를 하나 더할 때마다 에셋을 다시 만들게 되지 않도록) — 그래서 에셋 안의 태그가 지금의 ini에 있는지는 C++ 쪽이 매번 다시 본다.

**관측 목록과 행성**(v0.8 — 단계 5부터): 관측 목록 행 구조체와 `DT_Planets`(계 행·이름·종류 태그·크기·궤도·온화 여부·발견 해 — 상태·해시·저장 ID 없음). 구조체 이름과 열, `FGalaxyDef`의 이름 풀(`StarNameBank`·`CatalogPrefixes`·`CatalogStems`)의 은퇴 여부는 단계 5 헤더. 패널에서는 접힌 목록으로 아는 성계에만 보이고 수치는 '약'이다 — '백성' 줄과 같은 묶음에 두지 않는다. 발견 해는 게임 밖 '세계관과 출처' 화면에만 쓴다(게임 안에 서기 연도가 들어가면 게임의 '지금'이 정해진다, §12.2). 행성 그림은 도식이다 — 같은 지도의 줌 단계가 아니다. 문구는 안전 문안('조석 고정' 같은 말)만 쓰고 지어낸 행성은 넣지 않는다(v0.8 결정 8).

### 7.4 시뮬레이션 Subsystem (Sim/GalaxySimulationSubsystem.h)

```cpp

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSimTick, int64, Tick);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGalaxyGenerated, int32, Seed);

UCLASS()

class UGalaxySimulationSubsystem : public UGameInstanceSubsystem, public FTickableGameObject

{

GENERATED_BODY()

public:

// UE 5.8 Tickable.h 규칙: UObject와 함께 쓸 때는 Never로 생성하고 Initialize 끝에서 켠다. Deinitialize에서 다시 Never.

UGalaxySimulationSubsystem() : FTickableGameObject(ETickableTickType::Never) {}

virtual void Initialize(FSubsystemCollectionBase& Collection) override; // 끝에서 SetTickableTickType(ETickableTickType::Conditional)

virtual void Deinitialize() override; // 처음에 SetTickableTickType(ETickableTickType::Never)

// FTickableGameObject (IsAllowedToTick은 5.5에서 deprecated. 오버라이드하지 않는다)

virtual void Tick(float DeltaTime) override;

virtual bool IsTickable() const override { return bRunning && TimeScale > 0.f; }

virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UGalaxySimulationSubsystem, STATGROUP_Tickables); }

UFUNCTION(BlueprintCallable) void NewGalaxy(int32 Seed, int32 SystemCount, int32 CivCount); // 플레이어 문명 = DT_Rules.PlayerArchetypeRow(첫 생애 행 arch_seeker — A0 4b부터, 그 전 arch_terran) → Install(…, Ruler), 각 문명의 ArchetypeRow 기록 (§5.10). 타락 로스터(Phase 2 영적 압력 작업, §4.2): FallenTranscendentCount가 1 이상이고 DT_Transcendents에 Realm.Abyssal 행 trans_silence가 있으면 침묵을 고정으로 넣는다. 이어서 시뮬 스트림에서 정확히 (0~3으로 읽은 FallenTranscendentCount − 1)번 뽑아, 아직 들지 않은 Realm.Abyssal 행(SpawnWeight가 유한하고 0보다 큰 것, 교리가 겹치지 않는 것) 가운데 하나씩 SpawnWeight 가중으로 고른다 — 행 이름 사전순으로 놓고 고르며, 후보가 없으면 뽑은 수를 버린다(추첨 수는 노브만 따르고 테이블을 따르지 않는다, §9.2 ⓐ). Celestial·Independent 행은 상태 인스턴스를 만들지 않는다. 영향권은 아래 9항의 초기 코어로 시작한다 — 행의 `InitialInfluencedSystems`가 1이면 성계 하나, 0이면 빈 영향권이다(§5.7). 생성 규칙: AI 문명의 아키타입은 `Weight` > 0인 `DT_Civs` 행을 행 이름 사전순(`FNameLexicalLess`)으로 놓고 가중 추첨한다(복원 추첨 — 같은 아키타입이 여러 번 나올 수 있다). `arch_default` 문명은 윤리 3축을 축마다 [-1, 1] 균등 추첨하고 그 외 행은 `EthicsPreset` 그대로다. `AIProfile`이 빈 행의 AI 문명은 프로필 4개 중 추첨한다(윤리 가중 공식은 §16 #36). 플레이어 모성계는 `ResourceRichness` 1.0·`HomeStarClass`·`HomeSystemNames`를 쓴다(§5.11). 문명 이름과 성계 표시명은 뱅크에서 뽑아 평탄화해 저장한다(§8.5). 아래 1~10항은 Phase 2의 은하 생성 규칙 변경(`43a7b94`)에서 한 번에 넣었다 — 시뮬 추첨 지점이 바뀌어 같은 시드가 주는 은하가 그때 한 번 달라졌다(§9.2). 11항은 v0.8이다(단계 4 — 추첨 없이 붙어 추첨 수가 바뀌지 않는다)

// 1. 수: `SystemCount`·`CivCount`가 0이면 고향 행(`DT_Rules.HomeGalaxyRow` → `DT_Galaxies`)의 `LivingWorlds`·`Peoples`를 쓴다 — 게임 모드·테스트·커맨드렛은 60과 12의 사본을 갖지 않는다(§3.1). `DT_Galaxies`나 고향 행이 없으면 `FGalaxyDef`의 C++ 기본값을 쓴다(`DT_Rules`와 같은 방식이다 — `LogCosmic`으로 알린다. 기본값은 VS의 고향 값 60·12다, §7.3). 수 인자는 0 이하면 고향 행을 읽고, 성계는 1 이상으로, 백성은 1 이상 성계 수 이하로 맞춘다. `MaxSystems` 150 클램프를 시뮬에서 뗀다(시뮬은 150을 알지 못한다 — 150은 표현 계층의 액터 예산이고 넘었을 때의 경고도 액터를 만드는 표현 계층이 낸다, §9.4)

// 2. 도장: 모든 성계와 문명의 `GalaxyRow`에 고향 행 이름을 찍는다(§7.2). `TellRngState = (int32)CosmicSeed::Derive(Seed, "", Tell, 0)`(이 값은 'v0.7 예약' 커밋부터 찍는다)

// 3. ID: 첫 생애의 모성계는 `sys_0`(배열 0번 — `CosmicIDs::FirstHome`), 성역은 `sol`(11항 — `CosmicIDs::Sanctuary`, `Core/CosmicIDs.h`)(A0 4a부터. 그 전의 코드는 플레이어 모성계가 `sol`), 그 밖의 성계는 `sys_<n>`, 문명은 `NextCivSerial`에서 받은 `civ_<n>`(패딩 없음, §7.2)

// 4. 좌표: 정사각형에서 뽑아 원판 밖이면 버리는 기각 추출(추첨 수는 가변이지만 시드에 대해 고정이다. 성계 하나에 시도는 16번까지이고, 16번째 점도 원판 밖이면 그 점의 좌표를 절반으로 줄여 쓴다, §9.2). 삼각 함수를 쓰지 않는다(코드의 `FMath::Cos`·`FMath::Sin`은 이 변경에서 지웠다). `sys_0`은 고정 자리 — `HomeRadiusShare`(0.53) × 원판 반지름, 각도 0(임시 — v0.8 단계 5가 관측 좌표로 바꾼다, 첫 생애의 별은 §16 #92. A0 4a 전의 코드는 `sol`의 자리). 원판 반지름 = 1000 × √(`SystemCount`) — 성역은 세지 않는다. 자리일 뿐 수치 이점·보호는 없다. 생명 성계는 팔을 따르지 않는다(고른 원판) — 나선팔·막대·광년 눈금·배경 별·풍경 은하는 표현 계층이 그린다(해시 밖)

// 5. 항성 등급: `DT_Rules.LivingWorldStarShares`(누적 7칸, O~M)를 `FRand` 한 번과 비교한다. 이 분포는 플레이를 위한 설계 값이고 G·K 쪽으로 기울였다 — 현실의 분광형 비율도 생명 가능성의 추정도 아니다(§3.4). 첫 생애의 모성계(`sys_0`)는 `HomeStarClass`(`Star.Class.G`)(v0.8 — 단계 5에서 관측 목록의 실제 등급으로 바뀐다. 첫 생애 행의 `HomeStarClass` G는 그때까지)

// 6. 레인: 같은 `GalaxyRow`의 성계끼리만 잇는다(§5.9 다리)

// 7. 이웃 보장: `sys_0`의 1레인 이웃 가운데 하나를 첫 AI 백성의 모성계로 뽑는다(시뮬 추첨 1회, 보는 거리 노브를 읽지 않는다. `CivCount`가 1이면 없다) — §2.4의 5:30 첫 접촉을 지킨다(아래 '앎 단계')

// 8. 이름: 백성의 모성계는 `StarNameBank`에서, 무주 성계는 `CatalogPrefixes` × `CatalogStems`에서 비복원(부분 셔플)으로 받는다. AI 백성 이름은 아키타입 `NameBank`의 {Star} 패턴. 전부 생성 때 평탄화한다(§5.11). 아키타입의 `HomeSystemNames`가 차 있으면 모성계 이름은 거기서 받는다(같은 추첨 한 번을 그 뱅크에 쓴다 — 첫 생애 행의 제 해 이름(v0.8 — 단계 4b부터, 그 전 `arch_terran`의 '태양계'). AI가 뽑히는 행은 이 컬럼을 비운다, `Data.Integrity`). 카탈로그 이름은 앞말·빈칸·뒷말을 이은 것이다. 풀이 비었거나 다 쓰였으면 이름은 빈 값이다(풀의 크기는 `Data.Integrity`가 본다) (v0.8 — 단계 5: 관측 목록의 이름. `{Star}`와 로그는 그 백성이 부르는 이름 — `Derive`, 숫자 없음, §16 #79)

// 9. 로스터와 초기 코어: 타락 로스터 추첨(현행)에 이어 초기 코어의 자리를 고른다(§5.7의 규칙. 로스터 자리마다 시뮬 추첨 1회 — 0~3으로 읽은 `FallenTranscendentCount`회). 초기 코어의 거리는 `sys_0`에서 잰다 — 성역은 후보가 아니다(11항이 코어를 고른 뒤에 붙인다)

// 10. 첫 생애: `Install`이 첫 생애의 모성계 `sys_0`과 플레이어 문명을 `Knowing`에 적는다. `Install`은 성역을 거절한다(에러 한 줄, 추첨 없음)

// 11. 성역(A0 4a부터, 임시 — v0.8 단계 5가 실제 자리와 레인을 정한다): 초기 코어를 고른 뒤, `Install` 전에 배열 끝에 추첨 없이 `sol`을 붙인다 — `GalaxyRow`는 고향 행, 이름은 ST_CA `System.Sanctuary`('태양계')를 평탄화해 고정(카탈로그 이름 추첨에 끼지 않는다), 레인 없음(지도는 성역 없이 이어진다 — A0 결정 2), 자리는 원판 밖 고정 자리(1.15 × 원판 반지름, 각도 0 — 추첨도 `Derive`도 쓰지 않는다, A0 결정 3), `StarClass` `Star.Class.G`, `ResourceRichness` 0. 주인·위기·프로젝트·주장·코어 없음. 그래서 `State.Systems`는 `SystemCount` + 1개다. 추첨 수가 바뀌지 않는다 — 증명은 커맨드렛(시드 42, 20,000틱)의 결정적인 칸이 모든 행에서 같고 무주 성계 수(`WildSystems`)만 +1(2026-10-19 실측, 행 CSV도 같다). 성역을 지나는 레인 셈(주장의 인접·압력의 감쇠·시야·계승 가중은 지난다, 그릇의 이동은 거쳐 가지 않는다 — 기본값)은 §16 #80, 단계 5

// 현실에서 가져온 것 가운데 해시에 드는 것은 셋뿐이다: 항성 등급(5), `sys_0`의 고정 자리와 성계 좌표(4)(A0 4a부터 성역의 고정 자리 — 1.15 × 원판 반지름 — 도 해시에 든다) (§8.3, §9.2. v0.8 — 단계 5 뒤에는 관측 목록의 좌표·등급이 해시에 든다 — 오프라인 도구가 계산한 값이라 C++에 삼각 함수가 없다). 추첨의 순서(은하 생성 규칙 변경의 헤더에서 고정했다 — 같은 종류의 추첨은 묶어서 뽑는다, §9.2 ⓑ): ① 생명 성계마다 배열 순서로 — 좌표(`sys_0`은 뽑지 않는다. 그 밖은 시도마다 2회), 항성 등급 1회, 자원 풍부도 1회(등급과 풍부도는 `sys_0`도 뽑는다. 고정값이 있는 모성계는 뽑은 수를 버린다) ② 모성계의 자리 — 첫 AI 백성은 `sys_0`의 1레인 이웃에서 1회(7항), 나머지 AI 백성은 저마다 남은 성계에서 1회 ③ AI 백성마다 CivID 순서로 — 아키타입 1회, 윤리 3회(평화·영성·개방), 프로필 1회 ④ 이름 — 백성마다(플레이어 먼저) 모성계 이름 1회와 백성 이름 1회, 이어서 무주 성계마다 배열 순서로 카탈로그 이름 1회 ⑤ 타락 로스터(자리 − 1회) ⑥ 초기 코어(자리마다 1회) ⑦ 첫 생애의 그릇 이름 1회(`Install`)(성역 `sol`은 ⑥과 ⑦ 사이에서 추첨 없이 붙는다 — 11항. 그래서 추첨 수는 A0 4a 전과 같다). 아키타입·윤리·프로필·모성계 이름·백성 이름의 다섯 추첨은 §9.2 ⓐ대로 무조건 뽑는다 — 가중 행이 없어도, 아키타입이 `arch_default`가 아니어도, `AIProfile`이 차 있어도, 뱅크가 비어도 뽑고 버린다. 그래서 생성의 추첨 수는 시드와 두 수(성계·백성)와 로스터 노브만 따르고 `DT_Civs`·`DT_Galaxies`·`DT_Transcendents`의 내용에 좌우되지 않는다 — 행을 더하거나 뱅크를 채워도 지형과 모성계의 자리는 그대로다. `Sim.LayersInRange`가 결과를 본다: 생명 성계·백성 수 = 고향 행 값(성역은 따로 센다 — 배열 끝의 하나, 원판 밖), 등급 분포의 허용 오차(성역의 고정 등급은 세지 않는다), 첫 생애의 모성계 `sys_0`의 자리와 등급, 이름 중복 없음, 모든 레인이 같은 `GalaxyRow`, CivID 유일, 모든 소유 성계의 `GalaxyRow`가 주인과 같다 (§9.3)

UFUNCTION(BlueprintCallable) void SetTimeScale(float Scale); // 0,1,2,5,10, 25(셋째 칸부터 — 어느 배속이 열리는가는 표현 계층의 문이고 시뮬은 0..MaxTimeScale로만 자른다: 0 이하이거나 숫자가 아니면 0, 25 위는 25. FMath::Clamp는 NaN을 위쪽 끝으로 읽으므로 비교로). 0 = 일시정지이며 시뮬의 유일한 일시정지다 — 이 Subsystem은 엔진의 null-world 패스에서 틱하므로 엔진 일시정지(`SetGamePaused`)와 시간 팽창이 닿지 않는다. UI는 SetTimeScale(0)으로만 멈춘다

UFUNCTION(BlueprintPure) float GetTimeScale() const { return TimeScale; } // 패널의 배속 표시와 테스트의 복원용

UFUNCTION(BlueprintCallable) void StepOnce(); // 디버그/테스트. OnLogEntry·OnEventFired·OnTierAscended 핸들러 안(IsBroadcasting)에서 불리면 에러 한 줄을 내고 틱을 돌리지 않는다 — RunHeadless도 같고, 상태를 갈아 끼우는 NewGalaxy·LoadState(와 UCosmicSaveSubsystem::LoadGalaxy)도 같은 자리에서 에러 한 줄을 내고 아무것도 하지 않는다(§9.4)

// 끝의 판정(MVP ④, 2026-10-23 — Sim/CosmicEndings.h, 순수 함수 namespace): enum class ECosmicProvisionalEnd : uint8 { None, Rung, Clock } — UENUM이 아니다(저장·해시·데이터 칸이 아니라 끝 추가 규칙에 묶이지 않는다. 네 엔딩과 함께 간다). CosmicEndings::EndRung = Tier3_Causal(C++ 상수, 노브가 아니다). ECosmicProvisionalEnd ProvisionalEndOf(const FGalaxyState&, const FCosmicRulesDef&) — Calling.Lives가 비면 None, 어느 생애든 PeakTier ≥ EndRung면 Rung, 아니면 ProvisionalEndTick > 0 ∧ Tick ≥ 그 값이면 Clock(같은 틱은 Rung). Tick·Calling.Lives·노브만 읽는다(Civs·앎·로그·연대기는 읽지 않는다). 두 입력이 줄지 않아 한 번 선 끝은 판 내내 서고 로드도 같은 답을 낸다. 읽는 곳은 플레이어 컨트롤러의 FollowEnd와 커맨드렛뿐 — 단계·명령·RunHeadless는 읽지 않는다. const TCHAR* NameOf(ECosmicProvisionalEnd) = "None"·"Rung"·"Clock"(로그 줄)
UFUNCTION(BlueprintCallable) void RunHeadless(int64 Ticks); // 테스트/밸런스/공백기 도약용 (§4.4). 방송 게이트 OFF, 매 틱 엔딩 조건 검사(B5·B6 — 네 엔딩과 함께. MVP의 임시 끝은 들지 않는다 — 그 끝은 판을 멈추지 않는다, 아래 '끝의 판정'. 엔딩이 오면 끝난 판은 그 뒤의 RunHeadless·StepOnce가 아무것도 시뮬하지 않는다 — 한 틱씩 돌린 판이 긴 도약과 같은 세계로 남도록: 커맨드렛과 Sim.PhaseTwoExit가 한 틱씩 돈다)

const FGalaxyState& GetState() const { return State; } // C++ 위젯 베이스가 const 참조로 읽는다(§10.2). BlueprintPure로 두지 않는다 — FGalaxyState의 멤버는 Blueprint에 보이지 않고 노드 평가마다 상태 전체가 복사된다. WBP에 필요한 값은 좁은 접근자로 낸다

FGalaxyState& GetMutableState() { return State; }

FRandomStream& GetRng() { return Rng; }

void SyncRngState(); // State.RngState = Rng.GetCurrentSeed(). NewGalaxy와 SimulateTick의 끝에서 자동으로 부른다. 틱 밖에서 GetRng()를 쓴 호출자(조우 판정 등)와 저장 직전의 저장 Subsystem은 직접 부른다. ComputeStateHash는 살아 있는 Rng를 읽는다

uint32 DrawTell(int32 Count); // 말하기 스트림 (§9.2). State.TellRngState로 지역 스트림을 열어 0 이상 Count 미만의 수를 한 번 뽑고 상태를 되적는다 — 살아 있는 스트림 객체를 두지 않는다(GetTellRng 같은 getter는 없다). PushVariants의 문장 변형 추첨만 쓰고(부르는 함수는 그것 하나다) 어떤 규칙도 이 값을 읽지 않는다. 저장 값이 항상 최신이라 해시는 TellRngState를 그대로 읽는다. Count < 1은 호출자 오류다 — 에러 로그 한 줄, 추첨 없음, 0 반환. 핸들러 안(IsBroadcasting)에서 불려도 같다(Phase 2 종료 리뷰). v0.7 예약 커밋

uint32 ComputeWorldHash() const; // 세계 해시 (§8.1, §9.2). ComputeStateHash에서 Knowing·Log·Chronicle·TellRngState를 뺀 것(최상위 멤버 이름 기준). 앎이 세계를 바꾸지 않음을 잰다(Sim.KnowingInert). 쿠킹 빌드 비교도 이 값으로 한다. v0.7 예약 커밋

bool NoteMet(FName CivID, FName SeenAtSystemID = NAME_None); // 앎에 쓰는 함수 (§8.1). 격차 규칙을 적용하고 Knowing.MetCivs에 없을 때만 더하며, 새로 적을 때 만남의 줄을 낸다(말하기 스트림만. 주제는 그 백성과 SeenAtSystemID). 새로 만났으면 true. 그릇이 없으면(공백기) 아무것도 하지 않고, 핸들러 안에서는 에러 한 줄로 거절한다. 그때 Knowing.FirstContactCivID가 NAME_None이면 그 만남이 첫 접촉이고 같은 호출이 그 칸에 만난 백성의 ID를 적는다(아래 '앎 단계'). 시뮬 추첨 0. Phase 2

bool NoteKnown(FName SystemID); // Knowing.KnownSystems에 같은 방식으로 더한다. 줄은 내지 않는다. 새로 적었으면 true. 시뮬 추첨 0. 가드는 NoteMet과 같다. Phase 2

EAscensionTier GetViewerTier() const; // 보는 쪽 Tier(§8.1): 그릇이 다스리는 백성이 있으면 그 백성의 Tier, 없으면(방랑자·공백기) Calling.PersonalTier. NoteMet의 격차 규칙과 이야기의 범위가 읽는다 — 세계는 읽지 않는다

const FCosmicRulesDef& GetRules() const { return Rules; } // DT_Rules 단일 행(행 이름 rules). 테이블이나 행이 없으면 C++ 기본값

const FCosmicDataTables& GetTables() const { return Tables; } // 정의 테이블(§7.3, Data/CosmicDataTables.h). Initialize가 고정 경로 /Game/CA/Data/DT_*에서 로드한다. 설정 에셋은 두지 않는다

void SetTables(const FCosmicDataTables& InTables); // 테이블을 바꾸고 규칙 행과 고향 은하 행을 다시 읽는다. 테스트가 일시 테이블을 주입한다

const FGalaxyDef& GetHomeGalaxy() const; // DT_Galaxies의 고향 행(DT_Rules.HomeGalaxyRow). 테이블이나 행이 없으면 C++ 기본값이고, 행이 빠진 테이블은 에러 한 줄

FCosmicEvalContext MakeEvalContext(); // 살아 있는 상태·시뮬 스트림·색인·테이블·규칙을 평가 문맥으로 묶는다(§7.5). 효과를 적용하거나 행을 이름으로 내는 단계 코드가 여기서 만든다(이벤트 슬롯은 상태와 스트림을 건네받아 스스로 만든다). 틱 밖 호출자는 끝에 SyncRngState()를 부른다

const FCosmicStateIndex& GetStateIndex() const; // ID → 배열 인덱스(Sim/CosmicStateIndex.h). 저장하지 않고 해시하지 않으며 조회만 한다(순회하지 않는다). NewGalaxy·LoadState·건넘 단계가 다시 만든다. 조회는 그 인덱스의 원소가 정말 그 ID인지 확인하고, 낡았으면 에러 한 줄과 INDEX_NONE이다. 같은 파일의 CosmicState::IsSettled가 '정착된 성계'의 술어 하나다(§5.7)

bool IsBroadcasting() const; // OnLogEntry·OnEventFired·OnTierAscended를 내는 동안 참. 그 핸들러에서 부른 EvaluateAndFire·FireRow는 에러 한 줄을 남기고 아무 일도 하지 않는다. MakeEvalContext는 에러 한 줄을 남기되 문맥은 돌려준다(참조를 든 값이라 빈 문맥이 없다). static인 Apply·ApplyAll은 방송 셈에 닿지 못해 핸들러 규칙으로만 금한다(§9.4). 핸들러 안에서 거절되는 호출의 다른 목록(Push·PushVariants·DrawTell을 비롯해)은 헤더 주석과 CLAUDE.md 불변 규칙에 있다

// 영압 (§4.2). 파생값, 저장 안 함. Presentation의 영적 시야(§6.4)도 이 함수를 읽는다

UFUNCTION(BlueprintPure) float ComputeSpiritualPressure(FName SystemID, FName& OutDominantSourceID) const; // Phase 2 영적 압력 작업. 본체는 static FCosmicPressure ComputePressureIn(State, Index, Tables, Rules, SystemIndex)이고 이 멤버는 살아 있는 상태로 그것을 불러 P와 지배 원천의 ID만 돌려준다 — Check는 static을 부르고, 경제 단계는 같은 본체를 제 영향권 소속 표로 돈다(§9.4 — 두 길이 같은 값을 내는지는 `Sim.PressurePerCivilization`이 본다). FCosmicPressure = { P, 지배 원천의 종류(초월자·문명·방랑자), 그 ID, 그 Tier }: 안정도 소모가 '지배 원천의 Tier가 더 높을 때만'이라 종류와 Tier를 함께 돌려준다(심도는 싣지 않는다 — 행에서 읽는다). 항은 셋이다. 봉인되지 않은 초월자마다 PressureBySourceTier[Tier] × (1 + Depth) × 닿는 정도(Depth는 행의 값을 0~1로 잘라 읽는다, §3.2 심도. 닿는 정도는 static SphereReachIn이 말한다 — 영향권 안 1, 1레인 인접 PressureAdjacencyFalloff, 그 밖 0. 행이 없는 인스턴스는 0) / 그 성계를 소유한 문명의 PressureBySourceTier[Tier](AI든 플레이어가 통치하든 같다 — 통치하는 그릇은 제 항을 따로 더하지 않는다) / 방랑자인 그릇의 PressureBySourceTier[PersonalTier](그릇이 선 성계에, Calling.LifeState가 Embodied일 때만 — 공백기에는 그릇이 없다). 문명 항과 방랑자 항에는 심도가 없다. 죽은 노드는 원천이 아니다(위기 처리 작업부터). 성역(`CosmicState::IsSanctuary`)에는 어느 원천도 닿지 않는다 — 본체(두 길이 함께 쓰는 `PressureWithReach`)가 원천을 더하기 전에 0을 돌려준다(`Law.Hedge`, A0 4a). 지배 원천은 가장 큰 항의 원천이고 동률은 Transcendents 배열의 앞쪽, 그다음 문명, 그다음 방랑자다(방랑자의 ID는 NAME_None — 그릇에는 ID가 없다). 수정자 ①(경제 단계의 틱당 원천)·③(조건)은 이 작업, ④(정착 — AI와 통치자 명령)는 정착과 문명 제거 작업이고 이 함수가 아니라 영향권 소속을 읽는다(§4.2), ②는 분쟁 단계의 본문과 함께 Phase 4

// 규모 등급 (§1.3, §8.3). 파생값, 저장 안 함, 해시 밖. 기준 = 문명 Tier(T1 Billions — 지구 수십억, T2 HundredBillions, T3 Trillions, T4~5 Countless — 셈이 뜻을 잃는 곳), 보정 = Population(DT_Rules.PopulationBandThresholds: < [1] 한 단계 아래, < [0] Few, > [2] 한 단계 위, 상한 Countless). Presentation의 문명·성계 패널 '백성' 1줄과 UCosmicLogSubsystem의 {PopBand} 슬롯이 읽는다. Population 숫자·비율은 어디에도 내지 않는다(커맨드렛 CSV만 예외)

UFUNCTION(BlueprintPure) EPopulationBand GetPopulationBand(FName CivID) const;

// 전역 방송 게이트 (§6.1). 모든 Core Logic Subsystem의 Broadcast가 이 값을 본다. RunHeadless와 공백기 도약 중 false

bool IsBroadcastEnabled() const { return bBroadcastEnabled; }

void SetBroadcastEnabled(bool bEnabled) { bBroadcastEnabled = bEnabled; }

UPROPERTY(BlueprintAssignable) FOnSimTick OnSimTick;

UPROPERTY(BlueprintAssignable) FOnGalaxyGenerated OnGalaxyGenerated;

// 저장/로드에서 호출

void LoadState(const FGalaxyState& In);

uint32 ComputeStateHash() const; // 결정론 테스트용. 전부(앎·이야기 포함)를 해시한다 — 세계만은 ComputeWorldHash(). FName·숫자·태그만 — 모든 FText 제외 (§9.2). Calling(SuccessionKind·bReturnOffered·FLifeRecord.NameIndex·BirthArchetypeRow 포함)과 FCivilizationState.Notables(Role·NameIndex·FirstTick·LastTick·SourceRow) 포함. 규모 등급은 파생값이라 해시 밖

private:

void SimulateTick(); // 고정 순서(열 단계): ① 경제 → ② 연구 → ③ 외교 → ④ 분쟁 → ⑤ 위기 → ⑥ AI → ⑦ 건넘(TickCrossings) → ⑧ 플레이어 이동(Embodied일 때만) → ⑨ 앎(TickSight, Embodied일 때만) → ⑩ 역할 효과 해소(가드 밖, 10번째·마지막). State.Tick을 먼저 올린 뒤 단계를 돈다 — 첫 시뮬 틱은 1이고 틱 0은 NewGalaxy가 만든 상태다(틱 단계에서 생기는 사건은 틱 0을 갖지 않는다. 틱 밖 플레이어 입력은 Tick 0에도 일어날 수 있다 — §16 #35)

// 위기 단계(TickHazards): Hazard 만료 효과 적용. TickHazards(Hazard 만료)·TickConflict(분쟁 단계 — 통치 문명의 마지막 성계를 앗아 갈 때 DeposeRuler)·UCosmicEventSubsystem::Apply(이벤트 효과)·ResolveEncounterChoice(조우 OnFailure, 틱 밖 플레이어 입력)는 KillPlayer·DeposeRuler를 실행하지 않고 FCallingState.PendingRoleEffect(PendingEffect = 태그 Effect.KillPlayer 또는 Effect.DeposeRuler / PendingCause / PendingSourceRow, 틱당 최대 1건, 먼저 기록된 것 우선)에 기록만 한다 — 기록하는 자리는 하나다: 데이터 효과든 단계 코드든 UCosmicEventSubsystem::Apply를 거친다. Calling.LifeState가 Embodied일 때만 적는다(공백기의 틱은 역할 효과를 적지 않는다 — 도약은 바깥 틱의 열째 단계 안에서 돌고 그릇은 Install 전까지 옛 값이다). 출생 보호(생애 첫 BirthProtectionTicks)는 적는 자리에서 가린다 — 가려진 KillPlayer는 칸을 차지하지 않아 같은 틱의 뒤따르는 DeposeRuler가 그대로 적힌다. KillPlayer가 그릇에 닿는 경우는 둘뿐이다: 문맥에 성계가 있으면 그 성계에 선 방랑자에게만(성계에 일어난 일은 통치자를 죽이지 않는다 — 통치자의 실패는 폐위다, §4.4), 문맥에 성계가 없으면 문맥 문명이 통치 문명일 때(그 문명의 절멸)나 문맥에 문명이 없을 때만. event_grace_rescue는 Apply가 아니라 열째 단계가 사망 파이프라인 앞에서 FireRow로 낸다(Phase 4b)

// 역할 효과 해소(10번째·마지막 단계, ResolvePendingRoleEffect): PendingRoleEffect가 있으면 UPlayerRoleSubsystem의 사망·폐위 파이프라인(§4.4, §5.10)을 실행한다. 그 안의 RunHeadless 도약은 틱 경계에서 시작하므로 재진입이 없다. 자동 저장(§9.1)은 이 단계가 끝난 틱 경계에서만 찍는다

// 공백기 도약 중(Calling.LifeState == Interregnum): TickPlayerTravel(⑧)·TickSight(⑨)·플레이어 업적 평가·조우 생성을 건너뛰고 매 틱 엔딩 조건을 검사한다 (§4.4). 그릇이 없어 앎이 늘지 않는다 — NoteMet·NoteKnown도 불리지 않는다

// 이벤트·업적·영압의 자리: 열 단계와 SimulateTick의 호출 열 개는 그대로이고 네 단계 안의 순서를 고정한다. ① 경제 = 문명마다(Civs 배열 순서) 인구 회복(`PopulationRegrowthPerTick`) → 안정도 회복(`StabilityRecoveryPerTick`·`StabilityRestingLevel`, MVP ③) → 수입 계산(정착된 성계만 — 정착 중인 성계는 수입이 없다) → 틱당 원천(영압의 Stability 소모와 PressureEffectsPerTick. Phase 4부터 칙령·특성 행. 틱당 원천은 양을 고치는 효과만 건다 — 이 단계에는 추첨도 역할 효과의 기록도 없다, §7.5 Apply) → 비축 가산. 모든 문명을 다 돈 뒤 LowStabilityTicks와 FallenSphereTicks를 따로 한 바퀴 센다(FallenSphereTicks의 영향권 소속은 이 단계 첫머리에 만든 표를 그대로 읽는다). ② 연구 = 돌파마다 줄을 내고, Society 돌파에는 그 줄 다음에 event_book_found를 이름으로 낸다(행의 조건이 책이 닫힌 백성을 고른다, §5.11 — 이름으로 부르는 행은 그 자리의 단계가 낸다). ⑤ 위기 = 거대 프로젝트와 해결 → 카운트다운과 만료 → 발생 → 이벤트 슬롯(UCosmicEventSubsystem::EvaluateAndFire). ⑥ AI = AI·섭정의 결정(백성마다 거대 프로젝트의 시작, 이어서 정착과 그 추첨) → 업적 판정(UAscensionSubsystem::EvaluateFeats, 틱당 정확히 1회). 귀결 셋: 그 틱의 LowStabilityTicks 셈에 드는 Stability 변화는 안정도 회복과 틱당 원천뿐이고 그 밖의 변화(이벤트·만료·해결·AI·틱 사이 입력)는 다음 틱의 셈에 처음 잡힌다 / 한 틱에 역할 효과를 적는 순서는 틱 사이 입력 → ② 연구의 이름 행(Civs 배열 순서) → ④ → ⑤ⓐ 대응 프로젝트의 완성이 건 해결 효과(Systems 배열 순서) → ⑤ 만료(Systems 배열 순서) → ⑤ 발생의 전조 행 → ⑤ 이벤트 슬롯(Civs 배열 순서) → ⑥ 업적의 대가(플레이어가 통치하는 백성 먼저, 그다음 Civs 배열 순서 — 지금 스키마는 업적의 대가에 역할 효과를 싣지 않는다)이다 / 위기 단계의 카운트다운 뒤에 놓인 Hazard(ⓑ 만료의 효과·주장 규칙, ⓒ 추첨·수도, ⓓ 이벤트 슬롯)는 그 틱에 카운트다운이 줄지 않는다 — 카운트다운을 먼저 줄이고 만료를 모은 뒤에 놓인다. 그보다 앞에서 놓인 것(② 연구의 이름 행, Phase 4의 ④ 분쟁, '거대 프로젝트와 승천 판정' 작업부터 ⓐ 해결의 효과)은 그 틱의 ⓑ에서 한 번 준다. 발화한 행의 효과는 큐 없이 그 자리에서 배열 순서로 적용한다(§7.5)

void TickEconomy();

void TickResearch();

void TickDiplomacy();

void TickConflict(); // 유효 전력 = FleetPower × FleetEffectivenessByTierGap, 상위 영향권 소모, 함대 투입당 카운트다운 +50 (§5.6). 승수·소모 2줄. 통치 문명의 마지막 성계를 앗아 가면 PendingRoleEffect(DeposeRuler) 기록만(§4.4)

// 세계 함수(공개, static이 아닌 이름공간 — 상태·색인·테이블을 받고 줄을 내지 않는다): CosmicHazards::Place·Resolve·Transfer·MoveCapital·Destroy·EmptyHazardSlot·LetSettlementsGo, 물음 ClaimantOf·IsLastUnclaimed·IsLastSettledSystem·IsDeadNode / CosmicMegaprojects::Start·StartBlockOf·CanStart·Choices·Advance·HasCompleted·RequiredProgressOf·CostPerTickOf·CanPay·RealmCapOf·IsSettling·MaySettle·CanSettle·SettleBlockOf·SettleTargets·PeopleSettleBlockOf·Settle, 읽기만 하는 ReadSlot·PaidNothing(패널의 '멈춤'과 커맨드렛 — 어느 단계도 부르지 않는다) / CosmicState::IsSettled·OwnsAnySystem·PickAIProfile·LaneDistancesFrom / CosmicState::IsSanctuary·IsFirstHome과 상수 CosmicIDs::Sanctuary·FirstHome(ID만 보는 순수 함수 — Core/CosmicIDs.h, §4.5 ①. `Law.Hedge`를 지키는 자리: `SettleBlockOf`의 첫 사유 `Sanctuary`·`SettleTargets`·`Place`·`Transfer`·`Destroy`·압력 본체·`Install`·`BecomeWanderer`·`LoadGalaxy`) (Sim/CosmicHazards.h·CosmicMegaprojects.h·CosmicStateIndex.h — 새 물음을 만들기 전에 여기서 찾는다. 종료 리뷰)
void TickHazards(); // 발생 추첨은 정착된 성계만 돈다 — 무주 성계와 정착 중인 성계는 뽑지 않고 건너뛴다(추첨 수는 소유를 따르고 시드에 대해 고정이다). Hazard.Claim도 정착된 성계에만 선다 (§5.7). Hazard.Extinction의 만료는 그 문명의 성계를 놓게 할 뿐 Civs 배열을 줄이지 않는다 — 제거는 그 틱의 건넘 단계의 정리가 한다(플레이어가 통치하는 문명만 다음 틱의 정리로 미룬다 — 아래 '건넘 단계'). 만료 효과, Hazard.Rebellion 지속 틱 조건 (§4.1). KillPlayer·DeposeRuler는 PendingRoleEffect 기록만. 만료 처리 끝에 Hazard.Claim 발생 규칙(결정론: 타락 영향권 인접 — 조공은 Phase 4, Law.Remnant, MaxInfluencedSystems 상한)과 Claim 만료 = InfluencedSystems 편입 (§4.2 영혼 경제, 틱 순서 불변) 단계 안의 순서(고정): ⓐ 거대 프로젝트와 해결(AdvanceMegaprojects) — 모든 성계의 대응 칸(Systems 배열 순서), 그다음 모든 성계의 건설 칸(같은 순서)을 내고 진척시키고, 완성된 프로젝트를 기록하고 칸을 비우고, 그 성계의 Hazard 행이 든 프로젝트면 그 Hazard를 해결한다(답이 짓는 일보다 먼저 비축을 쓴다 — 정착과 문명 제거 작업의 리뷰. 값을 다 치르지 못한 대응 칸이 기다리는 자원을 그 값 아래로 남길 같은 백성의 건설 칸은 그 틱에 내지 않는다 — 종료 리뷰와 그 재감사). 카운트다운보다 먼저라 마지막 틱에 완성된 해결 수단도 성계를 구한다. 프로젝트 칸 하나가 '세계 → 받은 성계의 앎 → 완성 줄 → 해결 줄'이다(§5.7). 행이 더는 들고 있지 않은 대응 프로젝트는 결과 없이 끝나고, 정착 중인 성계의 건설 칸(`project_settle`)도 다른 건설 행처럼 돈다(그 행이 테이블에 없으면 칸이 그대로 남는다 — 정착과 문명 제거 작업). ⓐ는 제 추첨이 없다 — 해결이 거는 효과(`OnResolveEffects`)와 그 뒤의 행은 Apply대로 뽑는다(`NameNotable` 1회 따위) ⓑ 카운트다운과 만료 — 두 번에 나눠 한다: 모든 카운트다운을 줄이며 0이 된 성계를 Systems 배열 순서로 모으고, 그다음 모은 순서대로 만료시킨다. 모은 만료마다 시뮬 추첨 2회(결과의 수, 주장의 수)를 만료를 돌기 전에 전부 먼저 뽑는다(행이 없는 태그도 뽑는다). 만료 하나는 '세계 → 받은 성계의 앎 → 만료 줄 → 뒤따르는 줄'이다(원인이 먼저 읽힌다. 받은 성계의 앎은 세계 쪽의 코드 훅보다도 먼저라 그 만남의 줄은 슬롯에 아직 만료하는 태그가 있을 때 나간다 — 재감사): 앞선 만료가 그 슬롯을 비웠거나 바꿨으면(같은 태그로 다시 놓았어도 — 새로 놓인 것은 카운트가 1 이상이다) 건너뛴다 → 행이 없는 태그는 결과도 줄도 없이 비운다(해결처럼 — 반란이면 지도자와 불만의 셈도 끝난다) → 성계를 앗아 가는 만료(초신성 전조)가 주인의 마지막 정착된 성계에 닿으면(정착 중인 성계는 셈하지 않는다 — 그 정착은 성계와 함께 놓인다, §5.7) 먼저 마지막 성계의 문을 묻는다(아래 건넘 단계, §4.5 — 문이 열리면 그 만료는 줄 없이 슬롯만 비운다) → OnExpireEffects 또는 갈래(UCosmicEventSubsystem::ApplyAll) → 이웃 효과 → 받은 성계의 앎(FollowUpKnowing — 효과가 통치 문명에 넘긴 성계를 알고 준 백성을 만난다. 아래의 은혜 줄과 만료 줄보다 먼저다 — 아는 범위와 로그 작업과 그 리뷰) → 태그의 코드 훅(파괴·왕좌 교체·편입. 효과가 그 성계를 주인의 마지막 정착된 성계로 만들었으면 — 데이터는 그럴 수 없다: 만료 효과도, 이웃 효과가 부르는 해결 효과도 TransferSystem을 싣지 않는다, Data.Integrity — 파괴 앞에서 문을 다시 묻고 에러 한 줄. 그때 지켜진 성계는 만료 효과와 이웃 효과를 이미 받았다: 그 효과가 통치 문명에 넘긴 성계는 은혜 줄보다 먼저 알고, 그 효과가 남긴 Hazard의 줄은 은혜 줄 다음에 나간다) → 슬롯 비움 → 이 만료가 비운 슬롯에 Hazard.Claim 발생 규칙 → 만료 줄(주장이 아무것도 편입하지 못했으면 해결 줄) → 뒤따르는 줄(새로 선 Hazard의 발생 줄과 전조 행, 해결 줄) ⓒ 발생 — 정착된 성계마다 슬롯이 비었든 찼든 시뮬 추첨 정확히 1회를 Systems 배열 순서로 먼저 전부 뽑고, 그다음 그 수가 정한 발생을 같은 순서로 놓는다(찬 슬롯과 정착 중인 성계에는 서지 않는다). 그다음 결정론 발생(반란)을 Civs 배열 순서로 본다. 그다음 첫 위기(MVP ③ — `Tick == DT_Rules.FirstCrisisTick`인 틱에만, Civs 배열 순서로 모든 백성의 수도에 `FirstCrisisHazard`의 Draw 행을 놓는다. 찬 슬롯은 건너뛴다. 추첨 없음) ⓓ 이벤트 슬롯 — UCosmicEventSubsystem::EvaluateAndFire. Hazard를 슬롯에 놓는 길은 하나다(추첨·결정론 발생·SpawnHazard 효과·인접 전파가 같은 함수를 거친다). 그 길이 발생 줄과 그 Hazard의 발생 전조 행(crisis_rebellion의 전조 ②)을 낸다 — 효과가 놓은 반란도 전조 ②를 받는다. 효과가 세계를 바꾼 뒤 따라야 하는 줄은 문맥의 후속 처리 기록(FCosmicEvalContext.Notes — 세계 함수 CosmicHazards::Place·Resolve가 적는다)과 멤버 FollowUpNotes가 잇는다(위기 처리 작업). 받은 성계의 기록은 FollowUpKnowing이 줄보다 먼저 따른다(아는 범위와 로그 작업, §7.5)

// 인물 훅 ② (§1.3, §7.2, §7.5): crisis_rebellion 발생 처리가 전조 ② 행을 직접 Fire하고 그 효과 NameNotable(Notable.Rebel)이 만든다(데이터). 만료 처리에서 먼저 그 문명의 열린 Notable.Ruler 기록이 있으면 Apply 가드 ③과 같은 규칙으로 닫고(LastTick = max(Tick, 1), §7.5 — 왕좌가 바뀌었다), 그다음 열린 Notable.Rebel 기록의 Role을 Notable.Ruler로 바꾼다(AI 문명·플레이어 옛 문명 동일, '정권 교체' 로그 {Ruler}). 플레이어 문명이면 그 만료의 효과를 적용한 직후 PendingEffect가 Effect.DeposeRuler이고 PendingSourceRow가 crisis_rebellion일 때만 전환한다(다른 역할 효과가 먼저 기록되어 이 DeposeRuler가 버려지면 전환도 하지 않는다 — 다른 행이 적은 DeposeRuler로는 전환하지 않는다). 그때도 지금은 그 행의 만료 줄('새 다스리는 이가 섰다')이 나간다. 왕좌가 그대로 남는 것은 사망이 먼저 적혔을 때뿐이라 '반란이 지나갔다' 줄은 Phase 4b의 사망 파이프라인과 함께 더한다. 다른 원천(이름으로 부르는 행, Phase 4부터 조우의 해결)이 먼저 적은 폐위는 그릇을 어차피 틱 경계에서 떨어뜨린다 — 그때 만료 줄은 반란이 왕좌를 차지했다고 말하고 기록은 반란 지도자를 반란 지도자로 둔다. 그 줄은 그런 원천이 처음 오는 작업에서 정한다(§11 Phase 4). AI 문명이면 새 AIProfile을 만료의 결과 수가 고른다(CosmicState::PickAIProfile — 네 프로필 균등). 해결 처리에서 열린 Rebel 기록을 닫는다(LastTick = max(Tick, 1)). 넘김(TransferSystem)으로 비는 반란과 행이 없는 태그의 만료도 해결처럼 닫는다. 반란이 슬롯을 떠나면(만료·해결·넘김) 그 문명의 LowStabilityTicks = 0. Hazard.Extinction의 만료로 성계를 모두 잃은 문명은 그 틱의 건넘 단계(정리)에서 제거되고(플레이어가 통치하던 문명만 다음 틱) 명부도 함께 사라진다 — 애도 문장이 통치자 이름을 한 번 부른다. 인물 훅 ③은 UAscensionSubsystem의 AI 간이 판정(T3 도달 → Notable.Ascendant, §7.5), 훅 ①은 UPlayerRoleSubsystem::BecomeWanderer(Regent, §7.5). 인물 추첨의 난수는 문맥이 건넨 시뮬 스트림이다(§7.5)

void TickCivAI(); // AI 백성(플레이어가 통치하지 않는 백성)마다 Civs 배열 순서로: CosmicMegaprojects::Choices(SinceTick 0)를 모두 시작한다 — 대응 프로젝트와 업적의 건설 프로젝트, 추첨 없음(§4.3, §5.7, 거대 프로젝트와 승천 판정 작업). 다만 그 백성의 AIProfile이 DT_Rules.ClaimLeaveProfiles에 들면 Hazard.Claim이 선 성계의 답(구호)은 시작하지 않는다(MVP ③, §16 #71 — Choices 자체는 그대로라 통치자의 프로젝트 키는 이 노브를 받지 않는다). 이어서 확장은 정착(project_settle, §5.7 — 정착과 문명 제거 작업)이다: MaySettle(정착된 성계가 있고, 소유 성계 수 < RealmCapOf(Tier) — §5.12, 진행 중인 정착이 없다)이면 시뮬 스트림에서 정확히 1회 뽑고(대상을 보기 전에 — 대상이 없으면 그 수를 버린다, §9.2), SettleTargets(Systems 배열 순서 — 1레인·같은 GalaxyRow의 무주 성계, 봉인되지 않은 초월자의 영향권 안이면 Tier >= 3, §4.2 ④. 성역은 대상이 아니다)에서 그 수로 하나를 골라 Settle한다. 시작할 수 없는 백성은 뽑지 않는다. 소유 성계가 0인 문명은 어느 것도 하지 않는다(정착된 성계가 없다). 단계의 끝에서 UAscensionSubsystem::EvaluateFeats를 틱당 한 번 부른다(§7.5)

void TickCrossings(); // 건넘 단계(7번째). Civs의 원소 수가 바뀌는 유일한 자리: ⓞ 정리(소유 성계가 0인 문명 제거) → ⓐ 도착 → ⓑ 출발. 정착과 문명 제거 작업부터 ⓞ, ⓐⓑ는 Phase 6a (아래 '건넘 단계')

void TickPlayerTravel(); // 끝에서 Role == Ruler면 현재 생애의 열린 재위 ReignTicks를 1 올린다 (§7.2). 공백기 중에는 이 단계 전체를 건너뛴다

void TickSight(bool bVesselMoved); // 앎 단계(9번째, Embodied일 때만). 세계를 읽고 Knowing과 이야기만 쓴다 — 시뮬 스트림 추첨 0회. SightIntervalTicks마다, 그리고 그릇이 움직인 틱(bVesselMoved — 틱 첫머리의 CurrentSystemID와 이 단계 직전의 값이 다르다: 여행의 도착도, 파괴가 그릇을 옮긴 것도. SimulateTick의 지역 값)에 한 번 더 돈다. 공백기 중에는 건너뛴다. 본문은 아는 범위와 로그 작업 (아래 '앎 단계')

void ResolvePendingRoleEffect(); // 역할 효과 해소. Calling.PendingEffect가 Effect.DeposeRuler면 UPlayerRoleSubsystem::DeposeRuler(폐위 파이프라인)를 부르고 기록을 비운다. Effect.KillPlayer는 Phase 4b의 사망 파이프라인까지 기록된 채 남는다 (§4.4, §5.10)

UPROPERTY() FGalaxyState State;

UPROPERTY(Transient) FCosmicDataTables Tables; // 정의 테이블. UDataTable 참조를 쥐므로 UPROPERTY

FCosmicRulesDef Rules; // SetTables가 다시 읽는 DT_Rules 행의 사본(행이 없으면 C++ 기본값)

FGalaxyDef HomeGalaxy; // SetTables가 다시 읽는 고향 은하 행의 사본(행이 없으면 C++ 기본값)

FRandomStream Rng;

float Accumulator = 0.f;

float TimeScale = 1.f;

bool bRunning = false;

bool bBroadcastEnabled = true;

bool bInTickStages = false; // 1~9단계가 도는 동안 참(역할 효과 해소는 밖). 이때 NewGalaxy·LoadState·SimulateTick이 불리면 ensure (§9.4)

static constexpr int32 MaxTicksPerFrame = 10; // 프레임당 처리 상한이자 이월 상한 (§9.4)

static constexpr float MaxTimeScale = 25.f; // (공개) 배속 상한 — 셋째 칸의 ×25(§2.1). 프레임 예산이 아니다: MaxTicksPerFrame은 10 그대로 (§9.4)

static constexpr float SecondsPerTick = 1.0f;

};

```

**상세와 요약**: 상세로 도는 것은 고향 은하뿐이다. 무대 은하는 저장 상태 없이 순수 함수 `SummaryAt(GalaxyRow, Tick)`로 '전체 흐름에 맞춰' 흐른다(본문은 Phase 6a이고 VS에는 없다).

- **입력**은 `Seed`, 행 이름, `Tick`, `DT_Rules` 노브, 미봉인 타락의 로스터(`TranscendentID`·`bSealed`)뿐이다. `Knowing`은 결코 읽지 않는다.
- **출력**은 칸별 백성 수 5개와 '닿아 있는 교리' 태그다. 이 판에 없거나 봉인된 교리는 어디에도 닿아 있다고 나오지 않는다. 요약의 백성은 T5에 이르지 않는다 — 요약은 결말을 만들지 않는다.
- **산술**은 정수만 쓴다(임계 틱을 `AIAscensionSpeedMultiplier`로 나누는 곳은 모듈 안에서 나누고 한 번만 버린다 — 배율이 크면 AI가 빠르다, §5.7). 시뮬 스트림도 말하기 스트림도 뽑지 않고, 틱당 비용이 없다(볼 때와 건넘 단계가 출발을 판정할 때만 계산한다).
- `SummaryAt`은 고향으로 오는 출발을 정하므로 **시뮬 규칙**이다: `Sim/`에 두고 §9.2의 수학 규칙을 따르며, 바꾸면 모든 시드의 역사가 바뀐다.
- **출발 일정**은 저장 상태 없이 정의한다. 은하 G의 k번째 출발 틱은 `CosmicSeed::Derive(Seed, G, Depart, k)`에서 나오는 순수 함수이고, 건넘 단계는 `Tick`이 그 값과 같은 틱에만 판정한다. k는 `Transits`의 기록 수가 아니라 `Tick`에서 찾는다. 그 틱에 기록을 더할지는 고향 은하의 넷째 칸의 셈이 정한다(통과 규칙과 '건너뛴 칸은 다시 시도하지 않는다'의 정의는 아래 '건넘 단계'에 한 번만 있다). 기록이 곧 기억이라 저장을 사이에 둬도 한 번만 일어난다. 그 건넘의 아키타입은 `Weight` > 0인 `DT_Civs` 행 가운데 `CosmicSeed::Derive(Seed, G, Archetype, k)`로 고른다.
- **한계**를 그대로 적는다: 고향이 요약 은하로 보낸 건넘은 `Transits` 기록과 로그만 남기고 요약 값을 바꾸지 않는다.
- **표시**: 요약 값은 화면에 낱말로만 나오고(수 없음, §8.3) 그 낱말도 격의 규칙을 먼저 통과한다(T1~T2에게 은하 패널이 타락의 영향권이나 넷째 칸의 존재를 알려 주지 않는다). 만난 은하가 아니면 종류와 현실 수치 문구만 보인다(§6.4).
- 임계 틱은 커맨드렛 CSV의 고향 은하 분포에서 딴다(Phase 6a의 밸런스 작업이고 그 2~3주에 든다).

개발자의 문장 '대략적으로만 표시하고 설정해서'는 이렇게 읽는다: 만나지 않은 은하는 상태까지 대략(요약)이고, 고향 은하의 만나지 않은 백성은 상태는 온전하되 표시만 대략(앎)이다.

**건넘 단계(`TickCrossings`)**: 문명 배열 `Civs`의 원소 수가 바뀌는 유일한 자리다.

- ⓞ **정리**: 소유 성계가 0인 문명을 배열 순서로 제거한다(파생 기준이라 표지를 저장하지 않는다). 제거할 때 다른 모든 문명의 `Relations`에서 그 CivID 키를 지우고, `Knowing.PendingContactCivID`가 그 문명이면 비운다(그 밖에 그 ID를 가리키는 기록은 그대로 둔다 — 조회가 실패하면 사라진 것이다, §7.2). 그 틱에 플레이어가 통치하는 문명은 남겨 두고, 역할 효과 해소가 플레이어를 떼어 낸 다음 틱의 정리에서 제거한다 — 남겨 두는 일은 그 틱에 기록된 폐위(`Effect.DeposeRuler`)에 기댄다. 기록된 폐위가 없으면 아무도 떼어 내지 않는데, 그런 경우는 셋이다: ① 먼저 적힌 사망(`Effect.KillPlayer`)이 폐위를 밀어낸다 — Phase 4b의 사망 파이프라인이 기록된 사망을 처리하며 `bIsPlayerRuled`를 푼다(§11). ② 출생 보호가 절멸의 사망을 가린다 — 가려진 사망은 기록 자리를 차지하지 않으므로(위 역할 효과 규칙) `Hazard.Extinction`의 만료 훅이 초신성의 붕괴처럼 그 뒤에 폐위를 적는다(그 행이 오는 Phase 4의 몫, §11 — 사망 파이프라인은 가려진 사망을 받지 못한다). ③ 정리 자신이 통치 문명의 남은 정착을 놓아 그 문명을 비운다 — 놀이에서는 닿지 않는다(마지막 정착된 성계는 문 뒤의 단계 코드만 앗고, 파괴가 정착을 함께 놓은 뒤 붕괴를 적는다). 닿으면(손으로 만든 상태) 그 문명이 계속 남으므로 에러 한 줄을 낸다(`TickCrossings` — `LetSettlementsGo`는 놓은 것이 있는지를 돌려준다). 남겨 둔 문명은 다음 틱에 연구하지 않는다(연구 단계는 성계가 없는 문명을 건너뛴다 — 돌파 줄과 책의 발견 행이 나지 않는다). 제거에는 줄이 없다 — 마지막 성계를 앗은 만료 줄(과 플레이어가 통치하던 문명이면 '몰락한 군주' 줄)이 이미 말했다. '소유 성계'는 `OwnedSystems`에 있고 주인이 그 문명인 성계다(`CosmicState::OwnsAnySystem` — 붕괴의 검사도 같은 술어를 쓴다). 정리는 먼저 정착된 성계가 없는 문명의 남은 정착을 무주로 놓는다(`CosmicHazards::LetSettlementsGo` — 놀이에서는 파괴가 이미 놓았다, §5.7) — 그래서 정착 중인 성계만 가진 문명은 남지 않는다. 통치 문명의 마지막 정착된 성계를 앗아 가는 단계(분쟁의 소유권 변동, 위기의 만료)가 그 자리에서 남은 정착을 함께 놓고(`LetSettlementsGo` — 파괴처럼) `PendingRoleEffect`를 적는다(붕괴는 폐위 파이프라인, `Hazard.Extinction`이면 사망 — 현행 §4.4. 정리가 정착을 놓는 것은 그물일 뿐이고 붕괴를 적지 않는다 — 위 ③). 그 자리들은 앗기 **전에** 마지막 성계의 문(`GraceKeepsLastSystem` — 위기 처리 작업의 헤더에서 선언했다)을 먼저 묻는다: 그 성계가 주인의 마지막 정착된 성계이면(`IsLastSettledSystem` — 정착 중인 성계는 셈하지 않는다. 그 정착은 함께 놓인다, §5.7) `event_grace_remnant`를 `FireRow`로 내 보고, 행이 나가면 상실 전체를 건너뛴다 — 만료 효과(`OnExpireEffects`)를 적용하지 않고, Hazard 슬롯을 결과 없이 비우고, 역할 효과를 적지 않고, 그 슬롯에 주장을 세우지 않는다(§4.5). 행이 나가지 않으면 상실을 그대로 진행한다. 데이터 효과 `TransferSystem`은 어느 백성의 마지막 정착된 성계도 — 정착이 남아 있어도 — 옮기거나 놓지 못한다(그런 효과는 건너뛴다) — 백성이 소유 성계 0에 닿는 길은 문을 거치는 단계 코드뿐이다. 만료의 효과는 문을 물은 뒤에 걸리므로 그 효과가 주인의 다른 성계를 놓을 수 없게 스키마가 막는다(만료 효과와, 이웃 효과가 부르는 해결 효과에 `TransferSystem` 없음 — `Data.Integrity`). 그래도 그렇게 되면 코드가 파괴 앞에서 문을 다시 묻고 에러 한 줄을 낸다 — 그때 지켜진 성계는 만료 효과와 이웃 효과를 이미 받았다. 위기 단계의 `Hazard.Extinction` 만료는 그 문명의 성계를 놓게 할 뿐 배열을 줄이지 않는다.
- ⓐ **도착**: `ArriveTick == Tick`인 `Transits` 기록을 배열 순서로 처리한다(착지 추첨은 시뮬 스트림, §5.7 건넘).
- ⓑ **출발**: 이번 틱에 확정된 출발을 `Transits` 끝에 더한다(고향발은 `Civs` 배열 순서, 요약 은하발은 행 이름 `FNameLexicalLess` 순). 도착을 먼저 처리해 넷째 칸의 셈을 확정한 뒤 출발을 판정한다.

제거·추가 뒤에는 저장하지 않는 색인(ID → 인덱스)을 다시 만든다(색인을 다시 만드는 곳은 `NewGalaxy`·`LoadState`·정리·도착 넷이다). 건넘 단계 밖의 단계는 `Civs`에 원소를 더하거나 빼지 않는다. 문명을 만들려는 효과(반란의 분리, 바벨의 실제 분열, 바벨 연합의 '흩어짐')도 요청만 적고, 그 틱의 이 단계가 백성을 만든다 — CivID는 `NextCivSerial`에서 받고, `GalaxyRow` 도장을 찍고, 이름은 아키타입의 뱅크에서 `CosmicSeed::Derive`로 고른다. 그런 효과는 Phase 6a 이후이고 쓸 때 §12.3을 거쳐 다시 명세한다(VS에는 문명을 만드는 효과가 없다). `Systems`는 판 도중에 원소 수가 바뀌지 않는다(파괴된 성계는 죽은 노드로 남는다, §7.2). 소유 성계가 0인 문명은 그 틱의 AI 단계에서 건너뛴다. `TickCrossings`는 정착과 문명 제거 작업부터 ⓞ만 하고, ⓐⓑ는 Phase 6a까지 빈 본문이다.

**넷째 칸의 셈**: 넷째 칸의 제동(§5.12)이 무엇을 세고 어디서 막는가의 정의는 여기에 한 번만 적는다. 상한은 `FGalaxyDef.AIFourthRungCap`이고, 0이면 제동이 없어 아래의 통과 규칙 셋이 모두 통과다.

- T4Civs(G) = `GalaxyRow`가 G이고 Tier가 T4 이상인 문명의 수(플레이어가 통치하는 문명 포함 — §4.2의 설계 범위 '넷째 칸 이상 3~7'은 그 백성을 세지 않는다. 두 셈은 다른 셈이다).
- Inbound(G) = `Transits` 기록 가운데 `ToGalaxyRow == G`이고 아직 착지하지 않은 것의 수 — 업적 판정(AI 단계의 마지막)은 건넘 단계보다 먼저 돌므로 `ArriveTick ≥ Tick`으로 세고, 건넘 단계 안의 셈(착지·출발)은 `ArriveTick > Tick`으로 센다(처리 중인 기록은 세지 않는다).
- AI 문명의 T3→T4는 T4Civs(G) + Inbound(G) < 상한일 때만 통과한다. `Civs` 배열 순서로 먼저 판정된 문명이 통과한다.
- 건너온 백성의 착지(ⓐ)는 T4Civs + Inbound(처리 중인 기록은 세지 않는다) < 상한일 때만 통과한다.
- 고향으로 향하는 요약 은하발 출발(ⓑ)은 예정된 틱에 T4Civs + Inbound < 상한일 때만 기록되고, 건너뛴 칸은 다시 시도하지 않는다(k는 `Transits`의 기록 수가 아니라 `Tick`에서 찾는다).

**앎 단계(`TickSight`) — 보는 거리와 만남**: 앎 단계는 세계를 읽고 `Knowing`과 이야기만 쓰며 시뮬 스트림 추첨은 0회다. 가드(`bInTickStages`) 안에서 줄을 낼 수 있으므로 `OnLogEntry` 핸들러가 동기로 불린다 — 첫 접촉 카드는 상태를 바꾸지 않는 핸들러가 띄운다(§9.4). `Install`(역할 효과 해소 단계 안, 가드 밖)은 그 틱에 출생 성계와 출생 문명을 `Knowing`에 적는다(난수 없음. 방랑자로 취임하거나 방랑자로 바뀌는 틱에는 선 성계의 1레인 이웃도 적는다 — §4.4).

보는 거리의 단위는 레인 수다(본문과 ST_CA 라벨은 '보는 거리' — '시야'는 §6.4의 '영적 시야'와 겹친다. 노브 이름은 그대로). `DT_Rules.SightLanesByTier = {2, 3, 4, 6, 10}`(5칸 — MVP ③ 전 {1, 2, 4, 6, 10}), `SightIntervalTicks` 10, `FirstSightTick` 330 — 모두 앎 쪽 노브이고 세계와 생성 규칙은 읽지 않는다.

앎 단계는 `SightIntervalTicks`마다 돌고(1 미만은 1로 읽는다 — 스키마가 1 이상을 지킨다), 그릇의 `CurrentSystemID`가 바뀐 틱(틱 첫머리의 값과 이 단계 직전의 값이 다르다 — 여행의 도착도, 초신성의 파괴가 그릇을 옮긴 것도)에는 주기와 무관하게 한 번 더 돈다. 보는 거리는 `SightLanesByTier[Tier]`이고 없는 칸이나 1 미만은 1로 읽는다(에러 줄 없이 — 범위는 스키마가 지킨다). 눈(§4.4)에서 같은 은하의 레인을 따라 너비 우선으로 보는 거리 안의 성계를 `NoteKnown`으로 `KnownSystems`에 더한다(이번 갱신의 임시 집합은 이미 아는 성계를 거르기만 한다. 출발점은 `OwnedSystems` 배열 순서, 이웃은 `Lanes` 배열 순서 — 더해지는 순서가 해시에 든다. 정수만 쓰고 임시 색인 위에서 돈다). `FirstSightTick`은 통치자가 소유 성계에서 보는 것만 늦춘다: 그 전의 통치자는 출생 성계(와 그 사이에 제 백성이 얻은 성계 — `NoteKnown`, §8.1)만 안다. 판의 틱 기준이라 첫 생애의 초반에만 작용하고, 방랑자의 눈은 늦추지 않는다.

- **만남**: 갱신 때마다 아는 성계의 현재 주인을 다시 본다. 주인이 아직 만난 백성이 아니면 `NoteMet`으로 만난다 — 방문이 아니라 알아봄이다(행성에 묶인 두 백성은 배가 아니라 신호로 만난다. 첫 함선은 8:30의 박자다). 격차 규칙이 먼저다: 상대의 Tier가 보는 쪽보다 3 이상 높으면 성계는 아는 성계가 되지만 그 주인은 만난 백성이 되지 않는다(§3.2). 한 번의 갱신에서 여럿을 만나면 가까운 순서(눈에서의 레인 수, 동률은 CivID `FNameLexicalLess`)로 더한다 — 거리는 눈의 자리(통치자는 소유 성계 전부, 방랑자는 선 성계)에서 같은 은하의 레인을 보는 거리의 상한 없이 잰다(예전에 알게 된 먼 성계도 잰다). 한 백성의 거리는 그 백성이 가진 아는 성계 가운데 가장 가까운 것이다. `FirstSightTick` 전의 통치자도 눈의 자리는 있다 — 새 성계를 적지 않을 뿐 만남은 같은 순서로 본다. 격차 규칙에 막힌 백성은 만나지 않은 채 남고 갱신마다 다시 본다(줄도 카드도 없다 — 이름 없는 현상 줄은 그 줄을 내는 Phase 4 자리의 것). 만남의 줄의 주제는 그 백성과 그 백성을 알아본 성계다(아는 범위와 로그 작업의 헤더).
- **낱말과 줄**: 만남은 각각의 만남이고, 첫 접촉은 그 판의 첫 만남이다(§2.4의 5:30 박자). '그 판의 첫 만남'은 저장된 필드 하나로 정의한다(이 정의는 여기 한 곳에만 쓰고 다른 절은 가리킨다): `FKnowingState`의 멤버 `FName FirstContactCivID` — 초기값 NAME_None은 '이 판에 아직 첫 접촉이 없다'이고, 구조체와 함께 'v0.7 예약' 커밋에 들어가며, `Knowing`의 일부라 전체 해시에 들고 세계 해시 밖이다. **첫 접촉**은 `FirstContactCivID`가 NAME_None인 동안 `NoteMet`이 `MetCivs`에 새 항목을 적는 만남이고, 같은 호출이 그 칸에 만난 백성의 ID를 적는다. `Install`은 이 칸을 결코 적지 않는다 — `Install`은 생애마다 출생 문명을 `MetCivs`에 줄 없이 적으므로 `MetCivs`가 비었는지나 그 길이로는 첫 만남을 가릴 수 없고, 그래서 필드로 가린다. 한번 적힌 칸은 바뀌지 않는다: 계승에도 남고(§4.4) 그 백성이 사라져도 비우지 않는다(§7.2) — 첫 접촉은 판에 한 번이다. 첫 접촉은 `log_first_contact`(Severity 1) 한 줄, 그 뒤의 만남은 `log_contact`(Severity 1, 3변형) 한 줄이다 — `log_first_contact`와 카드 `petition_first_contact`는 그 한 번의 만남에만 속한다. `MetCivs`에 새 백성이 적히는 자리(`NoteMet`)가 그 줄을 낸다(말하기 스트림만 쓴다). `Install`이 적는 출생 문명은 만남이 아니다. 만난 백성을 세는 곳 — CSV `MetCivs`와 §9.3의 목표(VS 첫 생애 2~6, EA 한 생애 12 이하) — 은 `ID`가 `Calling.Lives`의 어느 생애 기록(지금 생애 포함)의 `BirthCivID`와 같은 항목을 빼고 센다. CSV `FirstContactTick`은 `ID`가 `FirstContactCivID`와 같은 `MetCivs` 항목의 `Tick`이고, `FirstContactCivID`가 NAME_None인 동안은 빈 값이다.
- **첫 접촉 카드**: 첫 접촉(위의 정의)에만, 통치자일 때만 `Knowing.PendingContactCivID`에 올려 청원 `petition_first_contact`(판정 없음, 비용 0: 손을 내민다 / 지켜본다 / 응답하지 않는다 → 그 백성에 대한 태세)를 낸다. 그 뒤의 만남은 줄만 내고, 뒤에 만난 백성에 대한 태세는 보통의 외교 목록에서 정한다. 청원은 태세의 저장 위치(§16 #24)가 정해지는 Phase 4부터이고 Phase 2·3에는 줄만 나온다.
- **5:30의 보장**: §2.4의 '5:30 첫 접촉'은 생성 규칙이 보장한다 — `NewGalaxy`가 `sys_0`의 1레인 이웃 가운데 하나를 첫 AI 백성의 모성계로 뽑는다(시뮬 추첨 1회, 보는 거리 노브를 읽지 않는다. `CivCount`가 1이면 보장하지 않는다). 이웃의 아키타입·윤리·태세는 무작위라 보호도 수치 이점도 아니다. 보장하는 것은 상한이다: 첫 접촉은 늦어도 틱 `FirstSightTick + SightIntervalTicks`(340)까지 일어난다(아래 테스트).
- **만난 은하**(Phase 6a): 앎 단계가 `Transits`와 `MetCivs`에서 갱신한다(추첨 없음. 건넘 단계는 `MetGalaxies`를 쓰지 않는다) — 도착 기록의 `ResultCivID`가 만난 백성이면 그 `FromGalaxyRow`가, 출발 기록의 `SourceCivID`가 출발 때 이미 만난 백성이었으면(만난 틱 ≤ `DepartTick`) 그 `ToGalaxyRow`가 만난 은하다.

데이터 검사: `SightLanesByTier`의 모든 칸 ≥ 1, `SightIntervalTicks` ≥ 1, `FirstSightTick`·`FarLineCooldownTicks` ≥ 0(스키마 — 아는 범위와 로그 작업의 리뷰). 테스트 `Sim.FirstContactTimeline`: 시드 1~50(입력 없는 판)에서 첫 접촉이 늦어도 틱 `FirstSightTick + SightIntervalTicks`(340)까지 일어난다. 단언은 상한이다 — 세계 코드가 통치 문명을 상대로 `NoteMet`을 부르는 자리(외교·분쟁·청원, §8.1)가 본문을 얻는 Phase 4부터는 이웃이 틱 330보다 먼저 만난 백성이 될 수 있다. 데이터가 지금도 열 수 있는 길이 셋 있다 — 효과가 통치 문명에 성계를 넘기면 `FollowUpKnowing`이 준 백성을 만나고, 통치자가 아는 성계가 효과로 다른 백성에게 넘어가면 다음 갱신이 그 백성을 만나며(`FirstSightTick` 전에도 — 그 전의 눈은 성계를 더하지 않지만 아는 성계의 주인은 만난다), `FirstSightTick` 전에 폐위되면 방랑자의 눈은 늦춰지지 않아 새 눈이 아는 이웃의 주인을 다음 갱신이 만난다(폐위에는 출생 보호가 없다 — 재감사). 게임의 행은 `TransferSystem`을 쓰지 않고, 게임의 반란 행은 틱 500 전에 만료하지 않는다(발생이 틱 300부터이고 카운트다운이 200틱이다). 그런 호출과 행이 없는 Phase 2·3에서는 첫 접촉이 틱 330~340(`FirstSightTick`부터 `SightIntervalTicks` 안)이고, 테스트는 그 Phase 동안 하한 330도 함께 단언한다(Phase 4에 하한을 뗀다). Phase 2 종료 조건의 첫 접촉 확인은 이 테스트가 한다(스모크 실행은 첫 프레임에 끝나므로 볼 수 없다). 값은 CSV `KnownSystems`·`MetCivs`·`FirstContactTick`으로 맞춘다. 목표(§9.3): VS에서 첫 생애에 만난 백성 2~6, EA에서 한 생애에 12 이하 — 60성계 그래프에서 T3의 4레인이 지도 대부분을 덮을 수 있다.

### 7.5 승천·이벤트·로그 Subsystem 인터페이스 요약

```cpp

// UCosmicEventSubsystem

// - 두 층이다. static 층(Check·CheckAll·Apply·ApplyAll)은 this가 없어 문맥이 건넨 상태·색인·테이블·규칙·스트림만 닿는다 — GetRng()·로그·NoteMet·NoteKnown·델리게이트에 닿을 길이 없고, 떼어 낸 상태 사본에서도 돈다. 멤버 층(EvaluateAndFire·FireRow, 그리고 TellHazardLine·FollowUpKnowing·FollowUpNotes)은 살아 있는 상태에서만 돌고 static 층이 못 하는 세 가지를 한다: 앎을 적고(FollowUpKnowing — NoteKnown·NoteMet), 줄을 내고, OnEventFired를 낸다. Subsystem에는 멤버 변수도 캐시도 없다

// - 행은 원자다: (슬롯 행이면 쿨다운 키) → 효과 전부(배열 순서, 그 자리에서) → 행의 줄 → OnEventFired. 효과 도중에는 줄도 델리게이트도 나가지 않으므로 핸들러는 반쯤 적용된 행을 보지 못한다. 큐는 없다. 위기 처리 작업부터는 '효과 전부 → 행의 줄 → 뒤따르는 줄(효과가 놓은 Hazard의 발생 줄과 전조 행, 해결한 Hazard의 해결 줄) → OnEventFired'다 — 원인이 먼저 읽힌다. 그 작업이 문맥에 후속 처리 기록(FCosmicEvalContext.Notes)과 멤버 FollowUpNotes를 더했다. 아는 범위와 로그 작업부터는 효과 전부와 행의 줄 사이에 받은 성계의 앎이 든다: 효과로 통치 문명이 받은 성계는 어느 줄보다도 먼저 아는 성계가 되고, 그 성계를 준 백성은 만난 백성이 된다(§8.1, FollowUpKnowing — 준 백성은 그 리뷰가 더했다. 그 만남의 줄이 행의 줄보다 먼저 나갈 수 있다). SpawnHazard·ResolveHazard·TransferSystem은 세계 함수 CosmicHazards::Place·Resolve·Transfer를 거친다(틱당 원천은 셋 다 줄 없이 돌려보낸다)

// - int32 EvaluateAndFire(FGalaxyState&, FRandomStream&)  매 틱, 위기 단계의 맨 끝에서 부른다. 이벤트 슬롯을 돈다. 정착된 성계를 가진 문명은 DT_Rules.EventSlotTicks(10)틱에 한 번 슬롯을 받는다 — Tick mod EventSlotTicks가 CosmicSeed::DeriveForName(Seed, CivID, EventSlot) mod EventSlotTicks와 같은 틱이다(백성마다 고정된 위상. 배열 인덱스에 기대지 않아 정리가 다른 백성의 박자를 밀지 않는다. EventSlotTicks가 1 미만이면 1로, EventChancePerSlot이 0 미만이면 0으로 읽는다 — 범위를 벗어난 노브마다 에러 한 줄). 그 틱에 슬롯을 받는 문명마다 Civs 배열 순서로 건네받은 스트림에서 정확히 1회씩 먼저 전부 뽑는다 — 이 함수 자신의 추첨 수는 시드·틱·소유·CivID만 따르고 DT_Events의 내용·쿨다운·가중치·공백기에 좌우되지 않는다(테이블이 없어도 뽑는다). 그다음 문명마다 그 순서로, 제 차례의 상태를 보고: ① Standing 행 — 조건이 참이고 쿨다운이 끝난 (행, 단위)를(조건을 먼저 보고, 쿨다운 키는 조건을 통과한 단위가 있는 행에서만 한 번 만든다 — §9.4의 예산이 이 순서를 전제한다. Lottery 행도 같다) 행 이름 순(FNameLexicalLess. 한 행의 단위는 OwnedSystems 순서)으로 모아 하나씩 낸다. 발화 직전에 셋을 다시 본다 — 단위가 아직 그 문명의 정착된 성계인가, 조건, 쿨다운. 쿨다운이 (행, 문명)마다라서 쿨다운이 있는 SettledSystem 행은 슬롯당 한 단위(통과한 첫 성계)에만 나고 CooldownTicks 0인 행은 모든 단위에 난다(추첨·가중 없음) ② Lottery 행 — 조건과 쿨다운을 통과한 (행, 단위)를 같은 순서로 놓고, 후보의 몫 = BaseWeight × 가중 인수 × DT_Rules.EventChancePerSlot(몫의 합이 1을 넘으면 함께 줄인다 — 몫을 하나씩 나누지 않고 뽑아 둔 수에 합을 곱해 견준다. 그래야 반올림 때문에 어느 후보에도 떨어지지 않는 수가 남지 않는다), 뽑아 둔 수가 떨어진 자리의 후보 하나를 낸다(문명·슬롯당 최대 1행, 어느 자리에도 떨어지지 않으면 없음). 행의 단위는 Subject 컬럼이다: Civilization = 문명에 한 번(문맥 성계 = 정착된 수도), SettledSystem = 그 문명의 정착된 성계마다(OwnedSystems 순서). Named 행은 슬롯이 보지 않는다. 쿨다운은 (행, 문명)마다다 — 키 "<행>@<CivID>". 600틱마다 끝난 키를 지운다(끝난 키와 없는 키는 뜻이 같다). 반환값은 발화한 행 수. 백성 하나의 빈도는 백성 수와 무관하다: 60·12에서 틱당 슬롯 약 1.2개, 300·48에서 약 4.8개

// - bool FireRow(FName RowName, const FCosmicEvalContext&)  행 하나를 이름으로 낸다(Named 행: 반란 전조 ②, 은혜 행, 연구 단계가 Society 돌파마다 내는 event_book_found — 책이 고넬료 사슬로 늦게 온 판의 문턱은 §16 #78). 행의 Conditions가 통과할 때만 내고(없는 행이거나 조건이 거짓이면 아무 일도 없이 false), 쿨다운 키를 읽지도 적지도 않으며 제 추첨이 없다. 제 문맥을 새로 만든다 — 호출자의 문맥에서는 주제만 받는다(SourceRow는 그 행, bPerTickSource는 꺼짐). 호출자의 문맥은 바뀌지 않는다

// - static bool Check(const FCosmicCondition&, const FCosmicEvalView&) / static bool CheckAll(TConstArrayView<FCosmicCondition>, const FCosmicEvalView&)  조건 1개 / 배열(AND, 빈 배열은 참) 평가 (업적/조우/청원도 같은 함수 사용). 뷰에는 스트림이 없다 — 뽑을 수 없고, 쓰지 않는다. 주제가 없는 조건(문명 없는 Civ 조건, 성계 없는 System 조건)과 풀 수 없는 Key의 조건은 거짓이다. 상대를 고르는 자리(CivRelationAtLeast·ChangeRelation — Key가 비면 문맥의 OtherID)는 살아 있는 문명의 CivID, 판에 있는 초월자 인스턴스의 TranscendentID, DT_Transcendents의 천상(Realm.Celestial) 행 이름 가운데 하나일 때만 풀린다(천상 행은 상태 인스턴스가 없어서(§4.2) 행 이름으로 풀린다. 타락·독립 행은 판에 인스턴스가 있을 때만 상대다 — 로스터에 뽑히지 않은 타락의 키가 세이브에 남지 않는다). CivStabilityBelowFor·CivFallenSphereTicksReached의 Key는 숫자만으로 된 양의 정수(9자리까지)일 때만 풀린다. Player 조건은 Calling.LifeState가 Embodied가 아니면 전부 거짓이다 — 그릇을 고치는 효과와 역할 효과의 Embodied 검사(Apply)와 함께 이것이 '공백기 중 플레이어 제외'의 전부다. MegaprojectComplete/TranscendentSealed는 문맥의 SinceTick 기준 + 0 센티널(CompletedTick > 0, bSealed): 플레이어 업적 판정만 SinceTick에 Calling.BornTick을 넣고 이벤트·Hazard·AI 판정은 0(언제든)이다 — 세계의 판정이 그릇의 출생 틱에 기대지 않는다 (§4.4)

// - static bool Apply(const FCosmicEffect&, FCosmicEvalContext&) / static void ApplyAll(TConstArrayView<FCosmicEffect>, FCosmicEvalContext&)  효과 1개 / 배열. 세계 상태만 바꾼다: 줄을 내지 않고, 앎을 적지 않고, 행을 내지 않고, 방송하지 않으며, Civs·Systems의 원소 수를 바꾸지 않는다. KillPlayer/DeposeRuler는 즉시 실행하지 않고 FCallingState.PendingRoleEffect에 기록만 한다(틱당 최대 1건, 먼저 기록된 것 우선) — Embodied일 때만. KillPlayer는 문맥에 성계가 있으면 그 성계에 선 방랑자에게만, 성계가 없으면 문맥 문명이 통치 문명이거나 문맥에 문명이 없을 때만, 그리고 출생 보호 밖일 때만 적는다(가려진 KillPlayer는 칸을 차지하지 않는다). DeposeRuler는 문맥 문명이 통치 문명일 때만. 파이프라인은 틱 경계의 '역할 효과 해소' 단계(§7.4)가 UPlayerRoleSubsystem에서 시작한다. PlayerStatDelta·GrantPower는 그릇과 부름을 고치므로 Embodied일 때만 적용한다. MultiplyIncome은 문맥의 bPerTickSource가 참일 때만 적용한다(경제 단계의 틱당 원천). Value는 0 아래를 0으로 읽고(수입이 음수가 되어 비축이 0 아래로 내려가지 않는다) Key가 있든 없든 같은 곱 하나다(수입 × Value). 양을 더하거나 곱하는 효과 여덟(AddResource·MultiplyIncome·ShiftEthics·ChangeStability·ChangeRelation·ChangeReverence·ChangePopulation·PlayerStatDelta)은 Value가 유한한 수가 아니면(NaN·무한대) false다 — 숫자가 아닌 값이 한계값으로 잘리거나 상태에 적히지 않는다(범위는 Data.Integrity, 가드는 코드). 거꾸로 틱당 원천이 거는 효과는 양을 고치는 일곱뿐이다 — AddResource·MultiplyIncome·ShiftEthics·ChangeStability·ChangeRelation·ChangeReverence·ChangePopulation. 그 밖의 효과(NameNotable, KillPlayer·DeposeRuler, PlayerStatDelta·GrantPower, AddTrait·RemoveTrait·SetFlag, SpawnHazard·ResolveHazard·TransferSystem)는 bPerTickSource가 참이면 false다(NameNotable은 추첨도 하지 않는다): 경제 단계는 추첨하지 않고, 역할 효과를 적지 않고, 그릇을 고치지 않고, 한 번 서는 표지를 틱마다 세우지 않는다. 열거형 끝에 더하는 효과는 이 목록에 넣기 전까지 틱당 원천에서 돌려보낸다. DurationTicks는 읽지 않는다 — 세는 기록을 두지 않는다(Data.Integrity가 0보다 큰 값을 틱당 원천에만 허락한다). TransferSystem은 어느 백성의 마지막 정착된 성계도 옮기지 못한다(정착 중인 성계는 셈하지 않는다). 풀 수 없는 Key의 효과는 건너뛴다(§16 #29) — 상대가 풀리지 않는 ChangeRelation은 Relations에 항목을 만들지 않는다(없는 백성·없는 초월자의 키가 세이브에 남지 않는다). 반환값은 효과가 적용되었는가다 — false면 FGalaxyState는 그대로이고(NameNotable의 추첨 1회는 이미 뽑혔다 — 틱당 원천이 돌려보낸 때는 뽑지 않았다) 호출자는 다음 효과로 넘어간다

// - 평가 문맥은 FCosmicEvalContext(상태·스트림·색인·테이블·규칙 + 주제 + bPerTickSource)와 그 읽기 전용 쪽 FCosmicEvalView다(평범한 C++ 구조체이고 USTRUCT가 아니다 — 상태와 스트림의 참조를 든다. 문맥은 복사할 수 없다). 주제(FCosmicEvalSubject) = CivIndex·SystemIndex(Civs·Systems의 인덱스, 없으면 INDEX_NONE)·OtherID(상대 — CivID 또는 TranscendentID)·GalaxyRow·SourceRow·SinceTick. 문맥은 처음부터 GalaxyRow와 스트림을 갖는다 (§7.2, §9.2). GalaxyRow에는 호출자가 평가 대상인 문명·성계의 GalaxyRow를 넣는다(Phase 2 은하 생성 규칙 변경이 도장을 찍기 전에는 NAME_None = '아직 찍히지 않음'이 실린다). 효과를 적용하거나 행을 이름으로 내는 단계 코드는 UGalaxySimulationSubsystem::MakeEvalContext로 문맥을 만든다(EvaluateAndFire는 상태와 스트림을 건네받아 제 발화의 문맥을 스스로 만든다). Check·Apply·EvaluateAndFire는 문맥이 건넨 상태와 스트림만 쓰고 GetRng()를 직접 부르지 않는다. 앎(Knowing)은 읽지 않는다 (§8.1)

// - Apply의 NameNotable 가드(약 30줄, Phase 2, §1.3·§7.2): ① 문맥 문명 Tier <= 3(T4 집단 승천은 개인성을 지운다, §3.2) ② PersonNameBank가 비어 있지 않고 문명 Traits에 Trait.Mind.NoSelf가 없음(둘 중 하나라도 걸리면 무시) ③ 같은 Role의 열린 기록이 있으면 LastTick = max(Tick, 1)로 닫고 새로 만든다(0은 '열린 기록'이라 Tick 0에 닫아도 닫힌 것으로 읽히게 한다 — §16 #35의 기본값) ④ Notables.Num() >= DT_Rules.MaxNotablesPerCiv면 닫힌 기록 중 LastTick 최소를 퇴출, 닫힌 것이 없으면 Ruler·Regent가 아닌 열린 기록 중 FirstTick 최소를 퇴출, 그래도 없으면 생성을 건너뛴다(로그는 그대로 뜬다 — 명부는 짧고 역사는 길다). NameIndex = 문맥의 스트림에서 적용마다 정확히 1회, 이 가드들보다 먼저 뽑은 수로 현재 배열에 없는 뱅크 인덱스 가운데 하나를 고른다 — 기록을 만들지 않으면 그 수를 버린다(행의 시뮬 추첨 수가 이름 뱅크·Tier·상한에 좌우되지 않는다. PersonNameBank를 뒤 Phase에 채워도 시뮬 난수열이 밀리지 않는다 — §16 #40 ⑥. 같은 이름의 재등장은 바퀴의 표지라 데이터로 피한다). 퇴출은 연대기를 지우지 않는다(Message는 Push 시점에 렌더되어 저장됨). 닫힘은 이 훅들(역할 교체·복귀·반란 해결)과 문명 제거(소유 성계가 0이 된 문명을 건넘 단계의 정리가 지운다, §7.4)만 — 틱당 사망 롤·노화 없음(§4.4 원칙 4)

// - FOnCosmicEvent OnEventFired(const FCosmicEventNotice&)  값만 든 USTRUCT(EventRow·Category·Severity·SystemID·CivID·Tick). 동적 델리게이트의 인자는 리플렉션 타입이어야 하고 핸들러에 상태와 스트림을 건네지 않는다. 행이 다 적용되고 말해진 뒤에 낸다. 핸들러 규칙은 OnLogEntry와 같고(§9.4) EvaluateAndFire·FireRow·Apply도 부르지 않는다 — 방송 도중에 부른 EvaluateAndFire·FireRow는 에러 한 줄을 남기고 아무 일도 하지 않는다(Apply는 static이라 막지 못한다 — 규칙으로만 금한다)

// UAscensionSubsystem

// - void EvaluateFeats(FGalaxyState&, FRandomStream&)  틱당 정확히 1회, AI 단계의 맨 끝에서만 부른다(§7.4). 그 틱의 해결·만료·이벤트·AI 행동이 모두 지난 자리라 '주요 이벤트 후'는 자리로 지켜진다(따로 부르지 않는다). 순서: 플레이어가 통치하는 백성 먼저(Embodied이고 통치자일 때만. 조건은 SinceTick = Calling.BornTick으로 본다), 그다음 Civs 배열 순서로 그 밖의 백성(SinceTick 0. 정착된 성계가 없는 백성은 건너뛴다). 한 백성에 대해: Path가 Ruler이고 RequiredTier가 그 백성의 Tier인 DT_Feats 행을 행 이름 사전순으로 보고(한 칸 위가 아니거나 조건이 없는 행은 판정하지 않고 에러 한 줄을 낸 뒤 다음 행으로 간다 — FAscensionFeatDef::IsOneRungWithDeed. 보는 행의 거르개는 FAscensionFeatDef::IsJudgedRulerRowAt 하나다(Path·RequiredTier·IsOneRungWithDeed — AI의 선택과 상단 바의 '다음 칸까지'도 같은 행만 본다, Phase 3 12번 작업)), CompletionConditions가 그 백성과 제가 가진 정착된 성계 하나(OwnedSystems 순서 — 목록에 있어도 주인이 다른 성계는 세지 않는다)를 주제로 전부 성립하는 첫 행 하나로 오른다 — 한 틱에 한 칸. 오를 때의 순서: Tier = TargetTier → AscensionCost(ApplyAll, SourceRow = 업적 행) → 플레이어면 CompletedFeats·생애의 Feats와 PeakTier(오른 칸이 더 높을 때만, §16 #67 ①)·GrantedPowers → AI 백성이 Tier 3에 오르면 Notable.Ascendant 기록(아래 인물 훅 ③) / Tier 4 이상이면 열린 인물 기록 닫기(아래) → 받은 성계의 앎(FollowUpKnowing — 대가가 통치 문명에 넘긴 성계를 알고 준 백성, 곧 승천한 백성을 만난다. 그 만남이 승천 줄보다 먼저다) → 승천 줄(행의 LogTemplates, Log.Ascension, Severity 2) → 뒤따르는 줄(FollowUpNotes) → 플레이어면 OnTierAscended. 방랑자 경로(개인 Tier)의 판정은 첫 방랑자 업적 행과 함께 Phase 4다. 핸들러 안에서 불리거나 살아 있는 상태가 아닌 상태를 받으면 에러 한 줄을 내고 아무것도 하지 않는다. 틱 사이 입력이 채운 조건은 다음 틱의 판정이 잡는다

// - FOnTierAscended OnTierAscended(EAscensionTier NewTier, FName FeatRow, EPlayerRole Path)  플레이어의 승천에만 낸다(AI 백성의 승천은 줄뿐이다). 방송 게이트와 방송 중 표시를 거친다

// - 승천 시: 대가 적용 → 권능 부여(T5는 Way.Received/Seized 포함) → 받은 성계의 앎(FollowUpKnowing — 대가가 통치 문명에 넘긴 성계를 알고 준 백성, 곧 승천한 백성을 만난다. 그 만남이 승천 줄보다 먼저다 — 재감사) → 로그 Severity 2 → 뒤따르는 줄 → FX/사운드 요청 (Presentation이 구독 — 연출은 Phase 3)

// - 계승 시 Tier에서 Powers 재부여 (§4.4. 통치로 받은 권능은 §16 #67 ②)

// - 인물 훅 ③ (§7.2): AI 문명이 간이 판정(§4.3)으로 Tier 3에 도달하면 그 문명에 Notable.Ascendant('먼저 건넌 자') 기록 1개 — SourceRow = Feat 행, Apply와 같은 가드(Tier <= 3·PersonNameBank·상한). T4 이상 도달은 만들지 않는다. 능력치 없음 이름 추첨은 T3에 도달한 AI 문명마다 정확히 1회, 가드보다 먼저다(Apply의 NameNotable과 같은 규칙)

// - 문명이 Tier 4에 도달하면(AI·플레이어 통치 문명 동일) 그 문명의 열린 인물 기록을 전부 닫는다(LastTick = Tick) — 집단 승천은 개인성을 지운다(§3.2). 닫힌 기록은 상한 규칙대로 남는다. 새 생성은 Apply 가드 ①이 막는다

// - AI 간이 판정의 T3→T4에는 넷째 칸의 제동(FGalaxyDef.AIFourthRungCap)이 걸린다 — 셈의 정의는 §7.4 건넘 단계이고 EA부터 유효하다(VS에는 T3→T4 판정 데이터가 없다). 플레이어의 업적 판정은 이 값을 읽지 않는다 (§5.12). 건너온 백성의 T4→T5 판정은 도착 뒤 DT_Rules.CrossingSettleTicks가 지나야 평가한다 (§5.7 건넘, Phase 6a)

// UCosmicLogSubsystem

// - enum class ELogMode : uint8 { Normal, Interlude }  SetMode() (Phase 4b). Interlude: Severity 0 폐기, 1은 범위 안(Knowing의 만난 백성과 아는 성계)만, 2 보존(범위 밖이면 bUntold) (§4.4, §8.1)

// - void SetTellAll(bool)  저장하지 않는 모드 표지. 켜면 범위를 묻지 않고 전부 쌓는다. 게임 기본은 꺼짐. CosmicTests::AcquireSim이 켜고(그 모드를 끄는 테스트는 끝날 때 되돌린다 — AcquireSim은 포인터만 돌려준다), 커맨드렛은 아는 범위와 로그 작업부터 SetTellAll(true)로 돈다(인자 `-TellAll=0|1`은 Phase 2 종료 리뷰에서 구현됨 — 2026-10-18 정정). 어느 모드든 세계 해시는 같다 (§8.1)

// - void Push(FGameplayTag Category, uint8 Severity, const FText& Template, const FFormatNamedArguments&, FName FocusSystem, FName SourceRow, FName FocusCiv = NAME_None)  FocusSystem·FocusCiv = 주제 성계·주제 문명. 이야기의 범위가 Push하는 순간의 세계 상태와 앎으로 판정한다: 범위 안은 쌓고, 범위 밖의 Severity 2·Log.Claim·Log.Record는 bUntold로 쌓고, 그 밖은 쌓지 않는다 (§8.1). FocusCiv는 끝에 붙는 기본 인자라 기존 호출부는 그대로다(인자는 v0.7 예약 커밋, 범위는 아는 범위와 로그 작업부터). SourceRow = 그 줄을 낸 행(이벤트·Hazard·조우·Feat·로그 템플릿 행), Tick = State.Tick. 렌더한 결과를 평탄화해 저장한다: Message = FText::AsCultureInvariant(FText::Format(Template, Args).ToString()). 포맷 이력과 ST_CA 키가 세이브에 남지 않고, 쌓인 줄은 Push 시점의 언어로 고정된다 (§8.1, §8.5). 핸들러 안(IsBroadcasting)에서 불리면 에러 한 줄을 내고 아무것도 쌓지 않는다(Phase 2 종료 리뷰)

// - bool PushVariants(const FCosmicLine&, TConstArrayView<FText> Variants, const FFormatNamedArguments&)  호출자가 가진 변형 배열에서 한 줄을 낸다 — "말하기 추첨 1회, 렌더 결과가 마지막으로 말해진 줄과 같으면(bUntold 항목은 건너뛴다, §8.1) 다시 뽑지 않고 다음 변형" 규칙의 유일한 자리이고 DrawTell을 부르는 유일한 함수다. 변형이 1개여도 1회 뽑고, 배열이 비거나 핸들러 안(IsBroadcasting — 에러 한 줄)이면 추첨 없이 false, 그 밖에는 true(범위와 무관). FCosmicLine = Category·Severity·FocusSystem·FocusCiv·SourceRow·보는 쪽 Tier의 창(MinViewerTier·MaxViewerTier, 0 = 그쪽 끝 없음. 창은 아는 범위와 로그 작업부터 읽는다). 이벤트 행의 LogTemplates와 SignRow, Hazard의 줄, 초월자 행의 VoiceTemplates·ClaimTemplates가 모두 이 길로 나간다. 둘째 주제(§8.1)는 Phase 4에 FCosmicLine의 필드로 더한다(호출부는 그대로)

// - bool PushRow(FName RowName, uint8 Severity, const FFormatNamedArguments&, FName FocusSystem, FName FocusCiv = NAME_None)  DT_LogTemplates의 이름 행 하나를 낸다: 행을 찾아 PushVariants(행의 Category·Templates, SourceRow = RowName, MinViewerTier = 행의 MinTierToReveal)를 부르는 얇은 포장이다 — 추첨·연속 중복·반환값의 규칙은 PushVariants의 것 그대로다(먼 줄을 내는 호출만 2회, §9.2). 로그 항목 수는 범위가 정한다(§8.1). 시뮬 스트림은 뽑지 않는다. 행이 없거나 템플릿이 비면 추첨 없이 false. 한 판에 한 번만 뜨는 고정 문장 행은 변형 1개를 허용한다(목록은 Tools/data/schema.json의 single_variant_rows: log_first_life·log_first_contact)

// - FName PickTemplateRow(FGameplayTag Category, FGameplayTag Context, FName FallbackRow) const  (Phase 4) (Category, Context)가 일치하는 선택 행의 이름, 없으면 호출자가 CosmicRows 상수로 넘긴 이름 행 FallbackRow. 뽑지 않고 줄도 내지 않는다 — 호출자가 그 이름으로 PushRow를 부르므로 변형 규칙이 선택 행에도 그대로 선다 (§5.11 계시 양식). 슬롯 {Revelation} {Transcendent}는 슬롯 해석이 푼다 (§8.2)

// - 슬롯 해석(§8.2, 약 20줄, Phase 4): 역할 슬롯 {Ruler} {Regent} {Rebel} {Loyalist} {Hearer} {Ascendant} = 호출자가 넘긴 문맥 문명의 그 역할 열린 기록 이름(PersonNameBank[NameIndex % Num]), 없으면 ST_CA 집단 폴백(다스리는 이·섭정·반란군·한 사람·들은 이·그들 중 하나) — 모든 문장은 인물 없이도 성립한다 (§8.3). {PopBand} = GetPopulationBand(CivID)의 ST_CA 라벨 6 중 하나. {Player} = FLifeRecord.NameIndex. Log.Record·Log.Claim·Log.Transcendent 템플릿과 FTranscendentDef.ClaimTemplates·VoiceTemplates에는 역할 슬롯·{PopBand} 금지 (Data.Integrity — 인물은 두 책에 구조적으로 들어가지 못한다, §8.4). 인물은 화자가 아니다: 1인칭은 타락 전용 유지

// - FOnLogEntry OnLogEntry(const FCosmicLogEntry&)  방송 게이트가 닫히면 쌓이기만 하고 방송되지 않는다(나중에 몰아 보내지 않는다). bUntold 항목에는 발화하지 않는다 (§8.1)

// - TArray<FCosmicChronicleChapter> BuildChronicle() const  (MVP ④, 2026-10-23 — 헤더 docs/60_DECISIONS/headers/A4_provisional_end.md §4) 지금 틱까지의 연대기 = CosmicChronicle::Build(산 상태). 읽기만 하고 뽑지 않아 핸들러도 부를 수 있다. 장 배열이다(장과 넘침 표지는 평면 배열에 실리지 않는다)
// - CosmicChronicle(Log/CosmicChronicle.h, MVP ④ — 순수 함수 namespace, USTRUCT 아님, 저장·해시·추첨·쓰기 없음, 앎을 읽지 않는다 §16 #49): FCosmicChronicleLine{ Entry(저장된 항목의 사본 — Message는 이미 평탄화), bOwn(그 생애의 줄 — Log.Calling, 그 백성의 Severity 2, 들인 Severity 1; 다른 백성의 Severity 2는 false — 흐리게) } · FCosmicChronicleChapter{ LifeAt(Calling.Lives의 자리), FromTick, ToTick(§7.2 Chronicle의 범위), Lines(보이는 줄, 시간순), Candidates(되풀이 ① 뒤·상한 전의 줄 수 — 커맨드렛의 'of'), bMore(상한이 줄을 뺐다 — 장 끝에 ST_CA Chronicle.More, 수 없음) } · MaxLifeLines = 40(표시 상한 — 상태에서는 아무것도 떠나지 않는다) · PeoplesOf(Life) = BirthCivID 다음 RuledCivIDs(None·중복 없이) · Admits(Entry, Peoples) = §16 #31 ②의 앞 절반(8필드만: Log.Calling·Log.Claim·Log.Record·모든 Severity 2, FocusCivID가 Peoples에 든 Severity 1, SourceRow가 log_first_contact인 Severity 1 — bUntold와 행 표를 읽지 않는다. MVP ⑤의 이관 거르기 #40 ⑦도 모든 생애의 백성으로 부른다 — 예정) · Build(State) = 생애마다 장 하나(생애가 없으면 없다), docs/70 §8.4의 네 걸음(정수 + − × ÷만, 앎으로 줄을 세우지 않는다) = BuildCapped(State, MaxLifeLines) · BuildCapped(State, Cap) = 같은 일을 주어진 상한으로(0 아래는 0 — 테스트와 커맨드렛의 측정) · VesselNameOf(Life, Tables) = BirthArchetypeRow의(풀리지 않으면 arch_default의) PersonNameBank[NameIndex % Num](Install의 규칙), 둘 다 없으면 빈 FText('제 N대'만)
// - (Phase 4b) 같은 장들 둘레에 두 책 장·비문·다이제스트를 더한다 — 그때의 집계: Severity 2 + Admits가 들인 Severity 1 + Log.Claim·Log.Record 전부를 엔딩용으로 집계(bUntold 줄 포함 — 연대기와 두 책은 이야기의 범위로 거르지 않는다, §8.4. 연속 중복 규칙은 마지막으로 말해진 줄과만 견주므로 bUntold 항목이 같은 문장의 말해진 줄 바로 앞에 쌓이거나(예: [B(bUntold), B]) bUntold 항목을 사이에 두고 같은 문장이 이어질 수 있다 — 연대기와 두 책은 바로 앞에 실은 줄과 같은 문장을 싣지 않는다. 표시 규칙이고 상태는 없다, 아는 범위와 로그 작업의 리뷰). 생애(대)별 장 나누기 + '두 책' 장(기록된 이름들 / 주장된 이름들, ≤ 28줄, 사람·성계의 수 렌더 없음 — 주기·비율은 허용, 인물 없음) + 에필로그 둘째 줄 "다른 책은 펴지지 않았다." (§8.4, 약 40줄). 인물은 생애별 장의 이미 렌더된 Severity 2·1 Message 안에서만 등장한다 — 이 함수에 인물 코드 0

// - FWorldDigest BuildWorldDigest(int64 FromTick, int64 ToTick) const  (Phase 4b) 상태 diff 5~8줄. 복귀·폐위·계승 공용 (§8.4). Knowing의 만난 백성과 아는 성계만 비교하고, 그 밖에서 바뀐 것이 있으면 끝에 닫는 한 줄 Digest.Elsewhere(ST_CA)를 둔다. 항목 유형에 '왕좌가 바뀐 문명: {Civ}의 {Ruler}'(구간 (FromTick, ToTick] 안에 crisis_rebellion 만료 로그('정권 교체', Log.Crisis, SourceRow = crisis_rebellion)가 있는 문명의 열린 Notable.Ruler 이름 — FirstTick은 반란 발생 틱이라 기준으로 쓰지 않는다. 최대 2줄, 5~8줄 예산 안에서 기존 항목과 경쟁)

// UGalaxySimulationSubsystem — 인물 훅 ② (§7.4 TickHazards 주석): crisis_rebellion 만료 시 열린 Ruler 닫기 + Rebel → Ruler, 해결 시 Rebel 닫기

// UPlayerRoleSubsystem

// - BecomeRuler(FName CivID) / BecomeWanderer(FGameplayTag RegentProfile, FGameplayTag ReignEndReason) / ReturnToFormerCiv()  (BecomeRuler·ReturnToFormerCiv와 자진 이탈은 Phase 4·4b — 지금은 폐위의 BecomeWanderer만) ReignEndReason = Reign.Abdicated / Reign.Deposed / Reign.Died. StatusTags는 함수 안에서 파생(Abdicated → Status.Abdicated, Deposed → Status.FallenMonarch, Died → 없음). 역할이 바뀌면 보는 눈이 바뀐다: BecomeWanderer는 방랑자가 되는 틱에 선 성계의 1레인 이웃을 NoteKnown으로 적고(추첨 없음, 줄 없음 — §4.4), 역할 전환은 Knowing.PendingContactCivID를 비운다 (§8.1)

// - 인물 훅 ① (§7.2): BecomeWanderer는 ReignEndReason이 Reign.Abdicated 또는 Reign.Died일 때만 그 문명에 Notable.Regent 기록 1개(섭정 프로필은 그대로 AI.Profile.* 태그, 인물은 이름만 더한다. SourceRow = 전환 사유의 행: 사망·쿠데타는 KillPlayer의 SourceRow, 자진 이탈은 행이 없으므로 Severity 2 '왕좌를 비우다' 로그의 DT_LogTemplates 행 이름(log_throne_vacated)을 SourceRow로 적고 그 로그를 같은 틱·같은 SourceRow로 Push한다). Reign.Deposed에는 만들지 않는다 — 왕좌에는 Rebel이 앉아 있다(§4.1). ReturnToFormerCiv는 열린 Regent 기록을 닫는다(LastTick = Tick). Apply와 같은 가드

// - BecomeRuler·ReturnToFormerCiv는 그 문명의 열린 Notable.Ruler·Notable.Regent 기록을 닫는다(LastTick = Tick) — 플레이어가 앉은 왕좌에는 다른 이름이 남지 않는다(플레이어는 Notables에 없음, §7.2)

// - 즉위·복귀는 Tick >= ThroneLockedUntilTick 검사 (§5.10)

// - 틱 경계에서 KillPlayer(FGameplayTag Cause, FName SourceRow) → FLifeRecord 기록 → OfferReturn() → BeginInterlude() → RunHeadless(LeapTicks) → Succeed()  사망 파이프라인 (§4.4). Succeed()가 계승 추첨 후 Install()을 부른다(SuccessionKind = Succession.Called). 호출자는 §7.4 ResolvePendingRoleEffect만

// - bool OfferReturn()  귀환 제안 훅 (§4.4 회귀의 유혹, 약 40줄). 게이트: Doctrine.Recurrence 초월자 존재 ∧ 미봉인 ∧ (bAwareOfPlayer ∨ 사망 성계가 영향권 ReturnLureMaxLanes 이내), 생애당 1회(bReturnOffered). enc_recurrence_return 1장을 WBP_EncounterCard로 제시. 수락 시 LeapTicks 0·Install(사망 성계, 직전 생애의 BirthCivID, 사망 당시 Role) — 귀환은 '같은 몸'의 약속이므로 BirthCivID·BirthArchetypeRow(이름 뱅크)·NameIndex를 직전 생애와 같게 유지한다(사망 성계의 소유 문명이 달라도 뱅크는 바뀌지 않는다) — (SuccessionKind = Succession.Returned; 통치 문명의 소유 성계가 하나 이상이면 Ruler로 복위·섭정 해임·ThroneLockedUntilTick 미적용, 아니면 Wanderer)·Exaltation +2·Status.Returned·옛 모성계 편입(그 성계가 출생 문명의 마지막 미편입 성계면 편입되지 않는다 — Law.Remnant, §16 #43), ReturnedLivesToSeize회면 GrantPower Way.Seized. 거절 시 Exaltation −1, Log.Calling "제안이 있었다. 거절되었다." 후 파이프라인 계속. VS에는 회귀가 없어 뜨지 않는다(Phase 4b 훅 + 테스트 주입 행, 6+ 실전)

// - 틱 경계에서 DeposeRuler()  폐위 파이프라인 (§5.10). 사망 파이프라인과 같은 순서 재사용. 방랑자가 되는 틱에 선 성계의 1레인 이웃을 NoteKnown으로 적는다 (§4.4)

// - Install(FName SystemID, FName CivID, EPlayerRole Role)  첫 생애(NewGalaxy, Ruler)와 계승(Wanderer)이 같은 함수. 은하 인자는 받지 않는다 — 계승은 은하를 넘지 않는다(§4.4. 시그니처의 미결은 §16 #34 그대로). 그 틱에 출생 성계와 출생 문명을 Knowing에 적는다(추첨 없음, 줄 없음 — 만남이 아니므로 NoteMet을 거치지 않고 FirstContactCivID도 적지 않는다). Role이 Wanderer면 선 성계의 1레인 이웃도 NoteKnown으로 적는다 (§4.4). 그릇의 이름 FLifeRecord.NameIndex는 Install마다 시뮬 스트림에서 정확히 1회, 뱅크를 보기 전에 뽑은 수로 고른다 — 쓸 뱅크가 비었으면 그 수를 버리고 NameIndex는 0이다(NameNotable과 같은 규칙이라 뱅크를 뒤에 채워도 난수열이 밀리지 않는다, §9.2)(§4.4): Succession.Installed·Called = 새 인덱스(직전 생애와 같은 문명이면 직전과 다른 값), Succession.Returned = 직전 생애와 같은 인덱스, 뱅크가 비면(이름 없는 문명) arch_default 뱅크. 플레이어는 Notables에 넣지 않는다. FLifeRecord.BirthArchetypeRow에 출생 문명의 ArchetypeRow를 적고, Role이 Ruler면 재위 3배열에 한 칸을 더한다(RuledCivIDs = CivID, ReignEndReasons = 빈 태그, ReignTicks = 0, §7.2). 성역(`sol`)은 받지 않는다 — 에러 한 줄로 거절하고, 아무것도 바꾸지 않으며 뽑지 않는다(A0 4a).

// - TravelTo(FName SystemID) / ScanCurrentSystem() / Train(FName Stat)  TravelTo와 T3 도약은 아는 성계만 받는다 — 거절된 입력은 아무 상태도 바꾸지 않는다 (§8.1). 다리는 TravelTo의 대상이 아니다 (§5.9). ScanCurrentSystem은 그 자리에서 보는 거리를 DT_Rules.ScanSightBonusLanes(1)만큼 넓혀 한 번 본다 (§4.4). 무주 성계의 스캔 서술은 log_scan_wild (§5.7). 성역으로의 이동은 경계 카드 '지경'으로 끝나고 `CurrentSystemID`는 바뀌지 않는다. 경로는 성역을 거쳐 가지 않는다(v0.8 — Phase 4 `TravelTo` 헤더, §16 #80·#81).

// - 통치자 명령: SetResearchFocus, SetStance, ProposeTreaty, EnactEdict, StartMegaproject, AllocateFleet  만나지 않은 백성에게는 태세·조약·즉위를 걸 수 없다 (§8.1). StartMegaproject의 대상에는 정착(project_settle)이 든다 — 새 명령이 아니다 (§5.7). bool StartMegaproject(FName SystemID, FName RowName)은 거대 프로젝트와 승천 판정 작업부터: Embodied인 통치자가 통치 문명의 성계에, CosmicMegaprojects::Start(AI와 같은 규칙)로 시작한다 — project_settle만은 SystemID가 정착할 무주 성계이고 CosmicMegaprojects::Settle(AI의 정착 규칙, 추첨 없음)로 시작한다(정착과 문명 제거 작업). 틱 사이 입력이라 다음 틱의 위기 단계 ⓐ에서 처음 나아간다. 핸들러 안에서는 에러 한 줄. 프로토타입 키(P)의 StartNextMegaproject는 통치 문명에 CosmicMegaprojects::Choices의 첫 것(SinceTick = Calling.BornTick)을, 그것이 없으면 SettleTargets의 첫 대상에 정착을 시작한다(Phase 3의 패널이 대신한다)

// - 청원(MVP ②, 2026-10-20 — 헤더 docs/60_DECISIONS/headers/A2_petition_cards.md): FCosmicPetitionAnswer AnswerPetition(int32 ChoiceIndex) — 대기 카드에 답한다(틱 밖 입력). 거절(대기 카드·통치자 없음, 표에 없는 행·번호 밖 선택지·비축 부족은 에러 한 줄, 방송 중)은 뽑지 않고, 받아들인 답은 시뮬 스트림에서 판정의 수(0~5)를 먼저 1회 뽑는다 → 카드가 더 서 있지 않으면 거둔다(Withdrawn — 비용·효과·줄 없음) → 비용(다스리는 백성의 비축) → 판정(CosmicPetitions::Passes: NoCheck이면 성공, 아니면 능력치 + 수 ≥ Difficulty, Stability는 (int32)(Stability × 10 + 0.0001)) → OnSuccess|OnFailure를 제자리에 적용 → FollowUpKnowing → 결과 줄 Push(Log.Civ, Severity 1, 말하기 스트림 추첨 없음) → FollowUpNotes → LastChoiceRows → 카드를 거두고 NextEncounter가 있으면 같은 주어로 다음 카드(추첨 없음) → SyncRngState. 반환 { Outcome(Refused·Withdrawn·Succeeded·Failed), Roll, Line } — 상태가 아니다. 델리게이트는 없다(화면은 틱 경계에 대기 카드를 읽는다). 순수 규칙은 Events/CosmicPetitions.h(RuledCivIndexOf·SubjectOf·Holds·StandingHeld·Present·StillStands·CheckValueOf·Passes·PassingRollsOf·CostOf·ArgsOf·Clear·Remember). 카드에 KillPlayer는 싣지 않는다(스키마 — Phase 4b), DeposeRuler는 PendingRoleEffect에 기록만, 처리는 다음 틱의 열째 단계(§7.4)
// - 조우(B1): 방랑자의 조우 카드도 같은 FEncounterDef와 답 함수를 쓴다(그때 이름을 넓히거나 조건으로 가른다). FOnEncounterPresented는 두지 않는다 — 대기 카드는 앎에 저장되고 화면이 틱 경계에 읽는다

// - ReceiveCalling(가칭, v0.8 — 단계 6, 헤더 승인): 부르심에서 서막(첫 틱 전의 카드, §4.4)의 결과(`Exaltation`, 연구 중점, 상처의 상태 태그 하나, 선택의 기록, 거절 횟수)를 상태에 적는다. 칸·`PeakTier`를 적지 않는다. 핸들러 안에서 불리면 에러 한 줄(CLAUDE.md의 목록에 든다). 선택의 기록이 저장 필드가 되면 초기값 규칙과 해시 포함 여부를 헤더에 적는다.

// 공통: 모든 Broadcast 앞에 UGalaxySimulationSubsystem::IsBroadcastEnabled() 검사 (§6.1). 플레이어·생애 종속 멤버 변수 금지, 지속 진척은 FGalaxyState에

```

### 7.6 초안 코드와의 차이

| 초안 | v0.2 | 이유 |
|---|---|---|
| `TArray<FString> ActiveTraits` | `FGameplayTagContainer Traits` | 오타 방지, 계층 질의, 에디터 검증. 초안 자체의 "FName 위주" 원칙과도 일치 |
| `FCivilizationData` 하나 | `FCivilizationState`(런타임) + `DT_Traits` 등 정의 | 저장 대상과 읽기 전용 데이터 분리 |
| `FAscensionFeat`에 `FeatID` 멤버 | 행 이름이 ID. 구조체는 `FTableRowBase` 파생 | DataTable 관례 |
| 조건이 없음 | `FCosmicCondition` 배열 | 업적·이벤트·조우가 같은 조건 평가기를 공유 |
| 한글 DisplayName | 영문 DisplayName | §10.3 |

---
