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

UPROPERTY() FName CivID; // 그 행을 여기서 마지막으로 완성한 백성(B3 — §16 #73, 헤더 B3 2.10절). 완성은 새 기록이든 있던 기록이든 Tick과 함께 이 칸을 그 백성으로 고쳐 쓴다(CosmicMegaprojects::Advance). NAME_None = B3 전의 세이브에서 온 기록 — 그 성계의 지금 주인의 것으로 읽고(HasCompleted), CosmicHazards::Transfer가 주인을 바꾸기 전에 옛 주인을 적는다(정복이 옛 기록을 정복자의 것으로 만들지 않게). 추가만이라 SaveVersion 1 그대로

};

// 짝 기록(B3 — 헤더 B3 2.1절): 한 백성이 다른 한 백성에 대해 지닌 태세와 조약. FCivilizationState.Ties[상대 CivID]의 값이다. 항목은 기본값(태세 없음 ∧ 조약 없음)이 아닐 때만 있다 — 쓰는 함수(CosmicDiplomacy::SetStanceOf·Sign·Break)가 기본값이 된 항목을 지운다(뜻이 같은 두 상태가 같은 해시를 갖게 — 정규형). 태세·조약은 태그라 열거자 차례의 함정이 없다

USTRUCT(BlueprintType)

struct FCosmicTie

{

GENERATED_BODY()

UPROPERTY(BlueprintReadOnly) FGameplayTag Stance; // 태세 — 이 백성이 그 상대를 대하는 자세: Stance.Friendly(우호) / Stance.Hostile(적대), 빈 태그 = 중립. 방향이 있다 — 상대가 이 백성을 대하는 태세는 상대 쪽의 다른 항목이다

UPROPERTY(BlueprintReadOnly) FGameplayTagContainer Treaties; // 조약 — 두 백성이 함께 맺는 약속: Treaty.Trade(무역) / Treaty.NonAggression(불가침). 두 백성에 같은 꼴로 있다 — 대칭은 쓰는 함수 하나(CosmicDiplomacy::Sign·Break)가 지킨다. 해시는 태그를 FNameLexicalLess 차례로 넣는다(저장된 차례가 아니라 — 끊었다 다시 맺은 짝도 같은 해시, 헤더 B3 11절 항목 2)

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

UPROPERTY(BlueprintReadOnly) TArray<FMegaprojectRecord> CompletedMegaprojects; // 행마다 마지막 완성(처음 완성한 순서). MegaprojectComplete는 Key 행의 기록이 있고 그 틱이 0보다 크고 평가 문맥의 SinceTick 이상이며 그 기록의 CivID가 문맥 백성일 때만 참이다(§7.5 — 플레이어 업적 판정만 SinceTick에 Calling.BornTick을 넣고 그 밖은 0이다. 업적 조건의 생애 상대화, §4.4. 남의 완성은 세지 않는다 — §16 #73, B3)

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

UPROPERTY() FName SourceRow; // 만든 행(이벤트·Hazard·조우 행. Ascendant 훅은 DT_Feats 행, 양위의 Regent는 DT_LogTemplates 행 log_throne_vacated — §7.5, B1). 테스트 Sim.NotablesFromEventsOnly의 근거 (§9.3)

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

UPROPERTY(BlueprintReadOnly) float FleetPower = 0.f; // 함대(B3 — 유닛이 아니라 백성의 수 하나). 늘 0 이상. 분쟁 단계가 자라게 하고(상한 CosmicConflict::FleetCapOf = FleetPerSettledByTier[칸] × 정착된 성계 수 — 첫째 칸은 0) 유지비·영향권 안의 소모·진 공격으로 줄인다(§7.4 TickConflict). B4부터는 카드의 답 하나도 줄인다 — 효과 `DelayHazard`가 서 있는 위기의 시간을 살 때 × `DT_Rules.ResistFleetFactor`(§7.3 효과 표). 그 함대가 무엇을 하는가는 아래 FleetTargetSystemID

UPROPERTY(BlueprintReadOnly) float Stability = 0.75f; // 0~1. 통치자의 유일한 지표. 불만도는 Hazard.Rebellion 카운트다운 (§5.10)

UPROPERTY(BlueprintReadOnly) int32 LowStabilityTicks = 0; // Stability < 0.3 연속 틱(문턱은 상수 하나 CosmicHazards::LowStabilityThreshold — Sim/CosmicHazards.h, B6a). TickEconomy 끝에서 갱신, CivStabilityBelowFor가 읽고 수도 패널의 반란 문턱 줄(CosmicHazards::RebellionWatchOf, B6a)이 보인다. 반란이 슬롯을 떠나면(해결·만료·넘김) 0으로 다시 센다 (§4.1)

UPROPERTY(BlueprintReadOnly) int32 FallenSphereTicks = 0; // 정착된 성계 가운데 하나라도 봉인되지 않은 타락 초월자(행의 Allegiance = Realm.Abyssal)의 영향권 안에 있는 연속 틱. 인접은 들지 않고, 행이 없는 인스턴스와 정착 중인 성계도 들지 않는다. TickEconomy 끝에서 LowStabilityTicks와 함께 갱신, CivFallenSphereTicksReached가 읽는다 (§5.11, §16 #25 ①)

UPROPERTY(BlueprintReadOnly) TArray<FName> OwnedSystems;

UPROPERTY(BlueprintReadOnly) TMap<FName, float> Relations; // Key = CivID 또는 TranscendentID -> -1~1 (§4.2). 별도 SpiritualFavor 미터 없음. 항목은 처음 쓸 때 생긴다. 없는 키 = 0 = 중립(§5.5). 방향이 있다 — 이 백성이 그 상대를 보는 눈이고, 상대가 이 백성을 보는 눈은 상대 쪽의 항목이다. 쓰는 곳(B3): 외교 단계의 표류(국경이 닿은 백성 쌍만, 값이 바뀔 때만 — 표류율 0이면 항목이 생기지 않는다), 정복과 배신이 곧 낮추는 값(CosmicDiplomacy::LowerRelation — 0보다 크고 유한한 양만), 효과 ChangeRelation. 국경이 닿지 않은 쌍의 저장값은 읽히지 않는다 — 보는 눈은 CosmicDiplomacy::ViewOf 하나(§7.4 외교의 규칙)

UPROPERTY(BlueprintReadOnly) TArray<FName> ActiveEdicts; // 그 백성이 지닌 칙령(B2, §5.8): DT_Edicts 행 이름. 원소는 겹치지 않고 제정의 차례이며, 철회는 남은 원소의 차례를 지킨다(CosmicEdicts::Revoke — RemoveSwap이 아니다: 차례가 해시에 들고 경제 단계가 그 차례로 곱한다). 경제 단계(효과·유지비)·위기 단계(발생 몫의 곱·카운트다운 더하기)·AI 단계(철회)가 이 차례로 걷는다 — 만료 때의 답만은 위기 행의 RulerResolutions 차례다(CosmicEdicts::HeldAnswerOf). 테이블에 없는 이름(다른 데이터로 만든 세이브)은 남되 아무 일도 하지 않는다 — 효과·유지비·슬롯이 없다. 통치자는 명령(EnactEdict·RevokeEdict, §7.5)과 카드의 답 효과 EnactEdict(B4 — 명령과 같은 문 CosmicEdicts::EnactBlockOf, §7.3 효과 표)로, AI는 철회로만 다룬다(CosmicEdicts::AIRevokesOf, §7.4). B2가 처음 쓰지만 처음부터 저장·해시되던 필드라 저장 필드는 늘지 않았다(SaveVersion 1)

UPROPERTY(BlueprintReadOnly) FGameplayTag AIProfile; // AI.Profile.Expansionist 등

UPROPERTY(BlueprintReadOnly) float ReverenceToPlayer = 0.f; // 0~1. 이 백성이 플레이어를 우러르는 정도 — 예배가 아니다. 즉위의 경외 길이 읽는다(DT_Rules.CoronationReverence 이상, CosmicWanderer::CrownBlockOf — B1). 오르내리는 길은 효과 ChangeReverence(청원 두 행, 방랑자 카드 넷의 성공 — 문맥 문명은 카드가 놓인 성계의 주인)와 폐위의 감쇠(모든 백성 × ReverenceDecay)다. 양위는 줄이지 않는다

UPROPERTY(BlueprintReadOnly) bool bIsPlayerRuled = false;

UPROPERTY(BlueprintReadOnly) int64 ThroneLockedUntilTick = 0; // 왕좌 잠금. 그릇이 왕좌를 떠나는 틱(폐위·양위)에 모든 백성에게 max(제 값, Tick + DT_Rules.ThroneLockTicks(600))을 적는다 — 옛 백성이 사라져도 다른 백성의 잠금은 남는다(B1). 즉위·복귀는 Tick >= 이 값일 때만(CrownBlockOf가 대상 백성의 값 하나를 읽는다, §5.10)

UPROPERTY(BlueprintReadOnly) TArray<FNotableRecord> Notables; // 역사에 흔적을 남긴 소수의 기록(§1.3). 상한 DT_Rules.MaxNotablesPerCiv(4). 역할당 열린 기록(LastTick==0) 1개. 능력치·AI·관계 없음, 개인을 겨누는(ID를 받는) 조건/효과 없음(문명을 대상으로 기록 1개를 만드는 NameNotable과 역할 유무만 읽는 CivHasNotable이 전부, §12.2). 플레이어 그릇은 넣지 않는다(FLifeRecord.NameIndex). NewGalaxy 시드 없음 — 인물을 만드는 사건(§7.3 NameNotable 행·코드 훅 3; Rebel만 Severity 1 예외)을 겪지 않은 문명은 비어 있다. 해시 포함 (§9.2)

// ---- B3(2026-10-26 [B-3a], 헤더 docs/60_DECISIONS/headers/B3_diplomacy_and_conflict.md 2.1절) 외교와 분쟁의 저장 필드 셋 — 구조체의 끝. 추가만이라 SaveVersion 1 그대로. 세계 상태라 Knowing에 두지 않는다(세계 해시에 든다) ----

UPROPERTY(BlueprintReadOnly) TMap<FName, FCosmicTie> Ties; // 짝 기록: 상대 CivID → 태세·조약(위 FCosmicTie). 없는 키 = 중립·조약 없음, 항목은 기본값이 아닐 때만 있다(정규형). 건넘 단계의 정리가 사라진 백성의 키를 모든 백성의 Ties에서 지운다(Relations를 지우는 같은 자리, CosmicDiplomacy::ForgetCiv — 추첨 없음). 맵의 차례로 걷지 않는다 — FNameLexicalLess 키 차례나 Civs + Find로(항목은 지워졌다 다시 생긴다). 생성은 비워 시작한다(태세는 생성이 정하지 않는다)

UPROPERTY(BlueprintReadOnly) FName FleetTargetSystemID; // 함대의 목표 성계: 제 정착된 성계 = 그 성계를 지킨다(지킴), 적대하는 이웃의 정착된 성계 = 그 성계를 에워싼다(공성), NAME_None = 제 성계들을 고루 지킨다. 공성은 이 둘에서 파생한다 — 성계 쪽 필드도 위기 칸도 쓰지 않는다(위기 칸을 쓰면 공성이 끝날 때마다 주장 규칙이 돈다, §16 #123). 목표를 쓰는 곳: 명령 AllocateFleet, AI 단계의 적대(§7.4 TickCivAI), 분쟁 단계의 점검과 끝, 즉위, CosmicHazards::Transfer·Destroy(그 성계를 노린 모든 목표를 푼다 — CosmicConflict::LiftTargetsOn)

UPROPERTY(BlueprintReadOnly) int64 FleetTargetSinceTick = 0; // 목표를 정한 틱(공성의 시작 — 공성은 이 값 + Max(1, DT_Rules.SiegeTicks)에 끝난다), 목표가 없으면 목표를 푼 틱(AI 백성의 쉼의 시작 — Tick ≥ 이 값 + SiegeRestTicks까지 적대를 열지 않는다; 통치자에게는 쉼이 없다). 0 = 한 번도 정하지 않았다. 같은 목표를 다시 고르면 바꾸지 않는다(공성 시간을 되감지 못하게)

};

// 그릇(육신). 계승 시 전부 초기화 (§4.4 3분할 표 '그릇' 열). PersonalTier·Powers·CompletedFeats는 부름(FCallingState)에 있다

USTRUCT(BlueprintType)

struct FPlayerEntityState

{

GENERATED_BODY()

UPROPERTY(BlueprintReadOnly) EPlayerRole Role = EPlayerRole::Ruler; // 첫 생애는 통치자 (§5.10). Succession.Called 취임은 항상 Wanderer(귀환 Succession.Returned은 사망 당시 Role)

UPROPERTY(BlueprintReadOnly) FName CurrentSystemID; // 그릇이 선 성계. 이동하는 동안에도 떠나온 성계다 — 여덟째 단계가 도착한 틱에 목표로 바꾼다(B1)

UPROPERTY(BlueprintReadOnly) FName TravelTargetSystemID; // NAME_None = 이동 중 아님(CosmicWanderer::IsTravelling — 모든 문·카드·거둬짐이 이 시험 하나를 읽는다). TravelTo가 레인 이웃 하나를 적고, 여덟째 단계(도착이든 이동만 끝나든)·BecomeRuler·BecomeWanderer·Install·그릇을 옮기는 파괴(CosmicHazards::Destroy)가 비운다(B1)

UPROPERTY(BlueprintReadOnly) int32 TravelTicksRemaining = 0; // 남은 틱. TravelTo가 CosmicWanderer::LaneTicksOf(DT_Rules.LaneTravelTicks, 1 아래는 1)를 적고 여덟째 단계가 하나씩 줄인다 — 두 이동 필드는 늘 함께 서고 함께 빈다(B1)

UPROPERTY(BlueprintReadOnly) FName RuledCivID; // Ruler일 때만

UPROPERTY(BlueprintReadOnly) FName FormerCivID; // 떠난 백성 — 양위·폐위·사망 모두 BecomeWanderer가 기록. 복귀(그 백성에 대한 BecomeRuler — CosmicWanderer::IsReturn: 경외·공백을 보지 않고 잠금만 본다, B1), 재회 조우의 CivHasNotable(Key = Former), NameNotable(Key = Former, Notable.Loyalist)이 읽는다. 즉위·복귀는 이 값을 지우지 않는다(지난 일의 기록) (§5.10, §7.3)

UPROPERTY(BlueprintReadOnly) int32 Body = 1, Mind = 1, Causality = 0; // 0~10. 출생 문명 Traits의 BodyMod/MindMod/CausalityMod로 초기화 (§5.9)

UPROPERTY(BlueprintReadOnly) int32 Exaltation = 0; // 0~10. 높아짐. 경배 수용 +1~2, 거부 -1, 귀환 수락 +2 / 거절 -1, T5 문턱 기다림 -1 (§5.9, §4.4). 경외/공포 분기(§3.2)·느부갓네살 트리거(§8.6)·받는 길 문턱 게이트(<= 3 열림, >= 7 닫힘)가 읽는다

UPROPERTY(BlueprintReadOnly) FResourcePool PersonalPool; // 아직 들어오는 길도 읽는 곳도 없다 — 방랑자 카드는 비용을 보지 않는다(B1). 수입과 쓰임은 B1 밖(뒤의 블록)

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

UPROPERTY(BlueprintReadOnly) bool bAwareOfPlayer = false; // 계승을 넘어 유지 (§4.4). 위기 단계의 주장 규칙 뒤에 초월자마다 한 번 쓴다(B6a — §7.4 TickHazards ⓑ)

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

UPROPERTY() int32 NameIndex = 0; // 그릇의 이름: 출생 문명 아키타입 PersonNameBank의 인덱스 (§4.4, §7.3). Install()이 GetRng()로 추첨 — Succession.Called는 새 인덱스(뱅크에 이름이 둘 이상이면 직전 생애의 이름과 원문 — FTextInspector의 source string, 표시 문화권과 상관없이 — 이 같은 첫 항목을 건너뛴다: 백성이 달라도 뱅크가 같을 수 있다, B5), Succession.Returned는 직전 생애와 같은 인덱스("같은 이름이 다시 적혔다"). 출생 문명이 이름 없는 문명이면 arch_default 뱅크. 슬롯 {Player}가 렌더. 해시 포함. 플레이어는 FCivilizationState.Notables에 들어가지 않는다

UPROPERTY() TArray<FName> RuledCivIDs; // 재위 3배열(RuledCivIDs·ReignEndReasons·ReignTicks)은 항상 같은 길이다. 즉위(Install의 Ruler, BecomeRuler — 옛 백성으로의 복귀도 같은 함수다, B1)가 세 배열에 한 칸씩 함께 더한다

UPROPERTY() TArray<FGameplayTag> ReignEndReasons; // RuledCivIDs와 같은 순서. Reign.Abdicated / Reign.Deposed / Reign.Died (§5.10). 열린 재위(지금 통치 중)는 빈 태그

UPROPERTY() EAscensionTier PeakTier = EAscensionTier::Tier1_Planetary; // 생애의 가장 높은 칸 — 출생 때의 개인 Tier, 통치하는 백성이 오르면 업적 판정이 올린다(§16 #67 ①). 즉위는 올리지 않는다 — 셋째 칸 백성의 왕좌에 올라도 그대로다(B1)

UPROPERTY() FGameplayTag CauseOfDeath; // Death.Hazard / Death.Encounter / Death.Age / Death.Transcendent

UPROPERTY() FName CauseSourceRow; // 사인을 만든 Hazard/조우/이벤트 행. 사인 추적률의 근거

UPROPERTY() TArray<FName> Feats;

UPROPERTY() TArray<FName> TranscendentsMet;

UPROPERTY() TArray<FName> LastChoiceRows; // 최근 3개 조우·청원 행 (비문). 답한 행을 끝에 붙이고 셋을 넘으면 맨 앞을 지운다(CosmicPetitions::Remember, MVP ②). 청원의 주기 슬롯은 먼저 이 행들을 뺀다(그러면 후보가 없을 때의 폴백은 아래 PetitionPeriodTicks — MVP ⑧). 그 셈과 폴백은 통치자의 청원만 센다 — 방랑자 카드(주제 VesselSystem)는 빼지 않는다. 방랑자에게는 이 배열에서 가장 최근의 방랑자 카드를 다시 내놓지 않는다(B1 — 저장 필드를 더하지 않고 이 배열을 읽는다). 비문이 이 배열을 읽는다(B5 — `[B-5c]`부터)

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

UPROPERTY() int64 BornTick = 0; // 현재 그릇의 출생 틱. 출생 보호(DT_Rules.BirthProtectionTicks)와 업적 조건 상대화의 기준

UPROPERTY() ELifeState LifeState = ELifeState::Embodied;

UPROPERTY() int64 PendingLeapTicks = 0; // 남은 도약 틱(B5): 죽음(UPlayerRoleSubsystem::Die)이 뽑은 폭을 넣고, 공백기에 열째 단계가 틱마다 하나 줄여 0이 되는 틱에 계승한다(§7.4 ResolvePendingRoleEffect). 공백기 도중의 세이브도 이 값으로 이어진다(§9.1). 틱 경계의 Interregnum ∧ 0 = 후보가 없어 계승하지 못했다(침묵 — 파생 술어이고 저장 필드가 아니다)

UPROPERTY() int64 LeapTicksTotal = 0; // 판의 도약 합계 — 폭을 뽑을 때 한 번 더한다. 다음 도약 폭 L(B5 — CosmicSuccession::LeapOf): 시뮬 스트림의 GetFraction() 한 수 f(0 아래·숫자 아님은 0, 1 이상은 1 바로 아래의 수 — 가장 긴 폭)로 Lo = max(0, LeapTicksMin), Hi = max(Lo, LeapTicksMax), L = Lo + min(⌊f × (Hi − Lo + 1)⌋, Hi − Lo), L = min(L, max(0, LeapTotalCap − 이 값)) — RandRange는 쓰지 않는다(범위가 0 이하면 뽑지 않아 노브 값이 수의 개수를 바꾼다, §9.2 ⓐ). 0이면 그 죽음의 틱에 곧 계승 (§4.4)

UPROPERTY() TArray<FLifeRecord> Lives;

// 부름 열(§4.4 3분할 표) — 그릇에서 그릇으로 맡겨지는 일과 그 기록이지, 몸을 건너 이어지는 플레이어의 정체가 아니다(B5 결정 12-가). 계승 시 PersonalTier는 T≤2 → Tier1_Planetary, T≥3 유지; Powers는 Way.*만 남기고 나머지는 Tier에서 다시 준다(B5 결정 8이 §16 #67 ②를 이렇게 닫았다 — 지금 권능을 주는 행이 없어 Way.*만 남는다); CompletedFeats는 오른 순서의 기록

UPROPERTY(BlueprintReadOnly) EAscensionTier PersonalTier = EAscensionTier::Tier1_Planetary;

UPROPERTY(BlueprintReadOnly) FGameplayTagContainer Powers; // Power.*와 Way.*. 계승 시 Way.*만 남는다(위 부름 열의 주석 — 나머지는 Tier에서 다시 준다)

UPROPERTY(BlueprintReadOnly) TArray<FName> CompletedFeats;

// PendingRoleEffect (§7.4 '역할 효과 해소'): 위기 단계·이벤트 효과·카드의 답·양위 명령(Abdicate — 원인 Reign.Abdicated, B1)은 아래 3필드에 기록만 한다(분쟁 단계는 적지 않는다 — 수도와 마지막 정착된 성계를 앗지 않는다, B3). 빈 PendingEffect = 없음 기록은 UCosmicEventSubsystem::Apply를 거치고 Embodied일 때만 한다(§7.5)

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

// 앎 — 부름이 알게 된 것의 기록 (§8.1 세 층: 세계 → 앎 → 이야기). 저장되고 전체 해시에 들지만 세계 상태가 아니다: 세계는 앎을 쓰기만 하고 읽지 않는다(전문과 세계 쪽의 예외 하나 — 건넘 단계의 정리 — 는 §8.1. 명령이 입력의 정의로 읽는 기다리는 카드·만난 백성과, 틱 안에서 앎에 쓰는 곳의 읽기는 docs/20 부록 '불변 규칙 전문'의 상태 문단). 배열은 알게 된 순서이고 줄지 않는다 — TMap 없음, 술어 태그 없음. 부름의 것이라 사망·폐위·양위·계승으로 지워지지 않는다 (§4.4. §9.1 v0.7 예약)

USTRUCT(BlueprintType)

struct FKnowingState

{

GENERATED_BODY()

UPROPERTY() TArray<FKnownMark> KnownSystems; // 아는 성계(ID = SystemID). 한번 알면 계속 안다 (§7.4 앎 단계, §8.1)

UPROPERTY() TArray<FKnownMark> MetCivs; // 만난 백성(ID = CivID). Install이 줄 없이 적는 출생 문명도 여기 든다 (§4.4, §7.4)

UPROPERTY() TArray<FKnownMark> MetGalaxies; // 만난 은하(ID = DT_Galaxies 행 이름). 사용은 Phase 6a (§7.4)

UPROPERTY() FName PendingContactCivID; // 대기 중인 첫 접촉 카드의 백성(B3 — 카드 행과 함께, 헤더 B3 2.9절). NAME_None = 대기 카드 없음. 적는 곳은 NoteMet 하나 — 그 판의 첫 만남을 적을 때, 몸을 입은 통치자이고 DT_Encounters에 petition_first_contact 행이 있으면. 비우는 곳: 건넘 단계의 정리(그 백성이 사라질 때), 역할 전환(Install·BecomeWanderer·BecomeRuler), 첫 접촉 카드 자신의 답(받아들인 답과 Withdrawn 모두). CosmicPetitions::Clear는 이 칸을 건드리지 않는다 — 다른 청원에 답해도 남는다. 건넘 단계의 정리가 비워도 이미 놓인 카드는 기다리고 그 답은 Withdrawn이다(§7.4 앎 단계의 '첫 접촉 카드'). 읽는 곳: 앎 단계 끝의 제시(기다리는 카드가 없을 때, 그 틱에 막 선 Standing 카드가 없으면 주기 슬롯보다 먼저 이 카드를 PendingPetitionRow에 놓는다 — 그 백성이 Civs에 있고 카드 행의 조건이 설 때만), 답 명령(그 카드의 상대 OtherID와 글의 {OtherCiv} — 입력의 정의), 표현 계층의 FCosmicKnownView.PendingContactCivID(KnownViewOf가 옮긴다 — 불변 규칙 14). 세계 결과는 없다 (§7.4 앎 단계, §8.1)

UPROPERTY() FName FirstContactCivID; // 첫 접촉의 기록. 초기값 NAME_None(동결) = 이 판에 아직 첫 접촉이 없다. NoteMet이 첫 접촉 때 한 번 적고 그 뒤로 바뀌지 않는다. Install은 적지 않는다. 사용은 Phase 2 (정의는 §7.4 앎 단계)

UPROPERTY() int64 FarLineUntilTick = 0; // log_far_event의 다음 허용 틱. log_far_ascent에는 걸리지 않는다 (§8.1)

UPROPERTY() FName PendingPetitionRow; // 플레이어의 답을 기다리는 카드(DT_Encounters 행) — 통치자의 청원(MVP ②)과 방랑자의 카드(B1)와 첫 접촉 카드(B3 — 놓이면 이 칸에 petition_first_contact, 상대는 위 PendingContactCivID)가 칸 하나를 함께 쓴다(필드 이름은 저장의 일부라 그대로 둔다). NAME_None(동결 초기값) = 카드 없음. 앎 단계의 끝(CosmicPetitions::Present)과 사슬을 잇는 답이 적고, 답·Install·BecomeWanderer·BecomeRuler가 비운다(다스리는 백성은 다스리는 동안 지워지지 않는다). 읽는 곳: 명령이 입력의 정의로(답, 그리고 TravelTo·BecomeRuler의 문 — 카드가 기다리는 동안 이동도 즉위도 없다), 틱 안에서는 앎에 쓰는 두 곳만(틱 첫머리의 StandingHeld와 앎 단계 끝의 Present — 칸이 비었는지, 그 답은 앎에만 쓰인다. 앎 단계 끝의 Present는 PendingContactCivID도 읽는다 — B3. 규칙은 docs/20 부록 '불변 규칙 전문'의 상태 문단) (§5.10)

UPROPERTY() FName PendingPetitionSystemID; // 그 카드가 다루는 성계: SettledSystem 행이면 다스리는 백성의 정착된 성계, VesselSystem 행(방랑자 카드, B1)이면 카드가 놓일 때 그릇이 선 성계(정착된 성계든 무주든), Civilization 행(백성 — 현재 성계는 수도)과 카드가 없을 때는 NAME_None

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

// 판이 어떻게 끝났는가 (B6a — 헤더 docs/60_DECISIONS/headers/B6a_endings_and_the_run_past_the_third_rung.md 2.1절, 결정 3·6-가): 기록 하나, FGalaxyState.Ending. 끝을 낸 틱이 열째 단계 뒤에 한 번 쓴다(§7.4 SimulateTick, CosmicEndings::EndingOf). 빈 Kind = 판이 계속된다. 끝은 이 필드 하나다 — 끝의 틱·종류를 다른 필드에 베끼지 않는다. 모든 초기값은 첫 세이브부터 동결(§9.1, 불변 규칙 7): 빈 기록은 쓰이지 않으므로(델타 저장) 이 필드 전의 세이브는 빈 기록으로 읽힌다(B6a 결정 2-다, SaveVersion은 1 그대로) — 셋째 칸으로 끝난 MVP 세이브도 흐르는 판으로 열린다(시계를 지난 세이브는 다음 틱에 그 틱의 Ending.Clock으로, 침묵에 머문 세이브는 Ending.NoCandidate로 끝난다). 세계 상태 — 두 해시에 든다(결정 3)

USTRUCT(BlueprintType)

struct FCosmicEndingRecord

{

GENERATED_BODY()

UPROPERTY() FGameplayTag Kind; // Ending.RuledPeople / Ending.Vessel / Ending.OtherPeople / Ending.NoCandidate / Ending.Clock (Config/DefaultGameplayTags.ini, CosmicTags::Ending* — 결정 6-가). 세이브에 남는 이름이라 바꾸지 않고, 사실의 이름일 뿐 화면 제목이 아니다(결정 6-나). 빈 태그(동결) = 끝 없음

UPROPERTY() int64 Tick = 0; // 기록을 쓴 틱. 래치의 CosmicEnd 줄과 연대기가 State.Tick이 아니라 이것을 끝의 틱으로 읽는다

UPROPERTY() FName CivID; // 다섯째 칸에 선 백성(Ending.RuledPeople·Ending.OtherPeople). 다른 종류는 NAME_None

UPROPERTY() bool bNoVessel = false; // 기록을 쓸 때 부름이 공백기에 있었다(그릇 없이 맞은 결말 — B5 결정 12-라). 다만 그릇이 그 백성이 다섯째 칸에 오른 틱에 죽은 끝은 아니다(헤더 B6a 2.2절 — 오름은 그릇이 있던 동안 일어났다)

};

// 판의 상태 = 고향 은하 상세 + 판 공통. 이름은 그대로 둔다 — 코드 식별자의 Galaxy(FGalaxyState·GalaxyID·NewGalaxy·슬롯 Galaxy_<GUID>·L_Galaxy)는 판을 가리키고 개명하지 않는다(§15). 고향 밖의 은하는 저장 상태가 없다(무대 은하의 요약은 순수 함수, 풍경은 그림 — §7.4, §5.11)

// - 고향 은하의 멤버: Systems·Civs — 원소마다 GalaxyRow를 가진다

// - 판 공통 멤버: 그 밖의 전부 — SaveVersion·Seed·Tick·RngState·TellRngState·GalaxyID·NextCivSerial·Transcendents·Player·Calling·Knowing·Transits·EventCooldownUntilTick·Log·Chronicle·ChronicleEviction(MVP ⑤)·Flags·Ending(B6a) — 최상위 20. 시계는 Tick 하나다(은하별 시계 없음)

// 저장: UCosmicSaveGame, 슬롯 이름, SaveVersion 초기값 1, CurrentSaveVersion은 그대로다. 새 저장 필드는 전부 기본값이 '없음'인 추가이고 그 초기값은 첫 세이브부터 동결된다(§9.1). 슬롯 신원(GalaxyID)과 스트림 값(RngState·TellRngState)은 FGalaxyState 최상위에만 둔다 — 해시 제외가 최상위 이름 기준이므로 중첩 구조체에 두지 않는다(§9.2). Sim.SaveRoundTrip의 FillEmptyContainers는 손으로 쓴 목록이므로 예약 커밋에서 새 컨테이너(Knowing의 세 배열과 Transits)를 채우는 줄을 더한다(§9.3 — B3의 Ties·함대 목표·완성의 주인도 그 채움과 SameLaterMembers에 들었다, 헤더 B3 11절 항목 1; B6a의 Ending도 같다)

// 은하 소속의 부호화는 하나다: NewGalaxy가 고향 행 이름(DT_Rules.HomeGalaxyRow, gal_milkyway)을 모든 성계와 문명의 GalaxyRow에 찍는다. 멤버 초기값 NAME_None은 동결이고 '아직 찍히지 않음'을 뜻한다 — NAME_None을 고향으로 읽는 규칙은 없다. NAME_None이 뜻을 갖는 곳은 OriginGalaxyRow(토박이)뿐이다. Transits의 FromGalaxyRow·ToGalaxyRow, Knowing.MetGalaxies, CosmicSeed::Derive의 이름, FCosmicEvalContext.GalaxyRow가 모두 같은 행 이름을 쓰므로 비교가 섞이지 않는다

// ID: SystemID·CivID·Relations와 Ties(B3)의 키·EventCooldownUntilTick의 키는 판 전체에서 유일하다. Phase 2의 은하 생성 규칙 변경부터 ID는 패딩 없는 번호를 쓴다 — 성계 sys_<n>(성역은 sol(배열 끝), 첫 생애의 모성계는 sys_0(배열 0번), 그 밖은 sys_<n>(배열 번호) — A0 4a부터. 그 전의 세이브는 플레이어 모성계가 sol), 문명 civ_<n>. CivID는 배열 인덱스가 아니라 NextCivSerial에서 받고 재사용하지 않는다. FName은 끝의 _숫자를 번호부로 가르므로 'ID 사전순'이라고 쓰는 모든 동률 기준은 FNameLexicalLess로 정의한다(패딩 없는 번호 이름은 번호순으로 놓인다 — sys_0은 다른 sys_*보다 앞이다)

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

UPROPERTY() TArray<FCivilizationState> Civs; // 고향 은하의 멤버. 원소 수는 건넘 단계에서만 바뀐다(§7.4). 문명을 제거할 때는 다른 모든 문명의 Relations와 Ties(짝 기록 — 태세·조약, B3)에서 그 CivID 키를 지우고(추첨 없음), Knowing.PendingContactCivID가 그 문명이면 비운다. Knowing.MetCivs·Knowing.FirstContactCivID·Transits·Lives는 그 ID를 그대로 둔다 — 조회가 실패하면 '사라진 것'이고 표시는 그것을 뺀다(Sim.NoDanglingRefsAfterLeap의 예외, §9.3). FirstContactCivID는 판의 기록이라 그 백성이 사라져도 비우지 않는다(비우면 다음 만남이 다시 첫 접촉이 된다, §7.4)

UPROPERTY() TArray<FTranscendentState> Transcendents;

UPROPERTY() FPlayerEntityState Player; // 그릇

UPROPERTY() FCallingState Calling; // 부름 (§4.4). ComputeStateHash에 포함

UPROPERTY() FKnowingState Knowing; // 앎 (§8.1). 상태 최상위에 둔다. 부름 열 — 계승에도 유지된다(§4.4). 전체 해시(ComputeStateHash) 포함, 세계 해시(ComputeWorldHash) 제외 (§9.1 v0.7 예약)

UPROPERTY() TArray<FTransitRecord> Transits; // 건넘의 기록, 생성 순 (§5.7). Phase 6a 전까지 빈 배열 (§9.1 v0.7 예약)

UPROPERTY() TMap<FName, int64> EventCooldownUntilTick; // 키 = UCosmicEventSubsystem::CooldownKey — "<DT_Events 행>@<CivID>"(슬롯 행의 문명별 쿨다운). 주장 줄에는 스팸 방지 키가 없다(B4 결정 6 — 편입이 드물고, 말하기는 세계를 바꾸지 않는다). 키는 저장되므로 꼴을 고정한다(행 이름에 '@'를 쓰지 않는다 — Data.Integrity). 이름으로 내는 행(Named)은 키를 쓰지 않는다. 끝난 항목은 600틱마다 지운다(끝난 항목과 없는 항목은 뜻이 같다 — 제거된 문명의 키도 그렇게 사라진다). 순회로 결과를 내지 않는다

UPROPERTY() TArray<FCosmicLogEntry> Log; // 상한 유지 (§8.4)

UPROPERTY() TArray<FCosmicLogEntry> Chronicle; // Log 상한(2,000)을 넘어 떠나는 줄 가운데 CosmicChronicle::Admits가 모든 생애의 백성으로 받는 줄만 이관(MVP ⑤ — #40 ⑦: 다른 백성의 S1·초점 없는 S1은 버린다. 다스리는 백성의 칙령 줄 셋 log_edict_enacted·log_edict_revoked·log_edict_lapsed도 받지 않는다 — B2). 옮긴 뒤 DT_Rules.ChronicleCap(1,500)을 넘으면 가장 오래된 Severity 1부터 밀어낸다(CosmicChronicle::MayLetGo — S2·Log.Calling·Log.Claim·Log.Record는 남는다). BuildChronicle 입력 — CosmicChronicle::Build는 이 배열 다음 Log를 읽는다(MVP ④: 게임 길이의 판에서는 이관이 일어나지 않는다). 생애별 장은 FLifeRecord.BornTick/DiedTick 범위 — BornTick(첫 장은 그 앞의 모든 틱도)부터 DiedTick, 없으면 다음 생애의 BornTick − 1, 마지막 장은 지금 틱까지 (§8.4)
UPROPERTY() FChronicleEviction ChronicleEviction; // MVP ⑤ — 연대기의 상한이 밀어낸 것: USTRUCT FChronicleEviction { UPROPERTY() int32 Count = 0; // 밀어낸 수, 늘기만 한다. 0(동결) = 밀어낸 것 없음 — '없음'의 유일한 시험  UPROPERTY() int64 ThroughTick = 0; // 밀어낸 줄 가운데 가장 늦은 틱(Count > 0일 때만 읽는다) }. 쓰는 곳은 UCosmicLogSubsystem 하나, 읽는 곳은 CosmicChronicle::BuildCapped(범위가 ThroughTick에 닿는 장은 bMore). 최상위라 세계 해시가 이름으로 뺀다(전체 해시 안). 두 초기값은 첫 세이브부터 동결 — 상한에 닿지 않은 판은 쓰지 않고, 그 전의 세이브는 '없음'으로 읽힌다
UPROPERTY() FGameplayTagContainer Flags; // SetFlag 효과의 저장소. Flag.Grace.Spent, 사자 예언 플래그 (§4.5). ComputeStateHash 포함

UPROPERTY() FCosmicEndingRecord Ending; // 판이 어떻게 끝났는가(B6a — 위 FCosmicEndingRecord, 헤더 B6a 2.1절). 빈 Kind = 판이 계속된다. 마지막 멤버. 세계 상태 — 전체 해시와 세계 해시 둘 다 든다(빼는 이름 목록은 그대로다, §7.4 ComputeWorldHash). 열째 단계 뒤에 한 번 쓰이고(§7.4 SimulateTick), 끝을 읽는 곳은 모두 이 기록을 읽는다(§7.4 '끝의 판정', §9.1)

};

```

### 7.3 정의 데이터 (Data/*.h)

```cpp

UENUM(BlueprintType)

enum class EConditionType : uint8

{

CivTierAtLeast, CivHasTrait, CivResourceAtLeast, CivStabilityBelow, CivEthicsAxisAbove, // CivTierAtLeast·PlayerTierAtLeast의 Value = Tier 번호 1~5 (열거형 값 + 1. EAscensionTier는 Tier1_Planetary == 0)

CivStabilityBelowFor, CivRelationAtLeast, // 지속 틱 조건(Value=임계 0.3 — 파라미터가 아니다: 카운터가 0.3 아래의 틱을 세고 코드는 Value를 읽지 않으며, 읽는 이를 위해 0.3을 적고 스키마는 그것만 받는다, §16 #25 ②. Key=틱 수 — 숫자만으로 된 양의 정수이고 아니면 거짓이다(§7.5; 읽는 곳은 CosmicConditions::TickCountOf 하나 — Data/CosmicConditionTypes.h, 평가기와 반란 문턱이 함께 읽는다, B6a). FCivilizationState.LowStabilityTicks 카운터를 읽는다, §4.1) / Key=OtherID(CivID 또는 TranscendentID)

CivReverenceAtLeast, // ReverenceToPlayer >= Value (§4.5 방문 SpawnConditions)

SystemHasHazard, SystemOwnedBy, MegaprojectComplete, // MegaprojectComplete의 Key = DT_Megaprojects 행(필수). 주제 성계의 완성 목록 CompletedMegaprojects에 그 행의 기록이 있고 그 틱이 0보다 크고 문맥의 SinceTick 이상이며 그 기록을 문맥 백성이 만들었을 때만 참(플레이어 업적 판정만 SinceTick에 Calling.BornTick을 넣고 그 밖은 0이다 — §7.5) (§4.4). 만든 백성은 기록의 CivID이고 그 칸이 빈 옛 기록은 성계의 지금 주인의 것으로 읽는다 — CosmicMegaprojects::HasCompleted(System, Row, SinceTick, CivID), 조건·업적 판정·AI의 Choices·패널(UI/CosmicNextRung)이 같은 함수다(§16 #73, B3)

SystemPressureAtLeast, SystemInfluencedBy, // 영압 P(S) >= Value (§4.2) / Key=TranscendentID (종속 판정)

PlayerTierAtLeast, PlayerInSystemWithHazard, PlayerStatAtLeast, PlayerHasRelic, PlayerRoleIs, // PlayerStatAtLeast Key: Body/Mind/Causality/Exaltation. PlayerRoleIs Ruler = 청원 카드, Wanderer = 방랑자 카드(B1 — 스키마의 시험 has_cond가 행 접두어 petition_·enc_와 묶는다). PlayerInSystemWithHazard = 그릇이 선 성계(이동 중이면 떠나온 성계)의 위기 — 방랑자 카드 넷의 조건

PlayerHasStatus, PlayerLifeIndexAtLeast, PlayerAgeAtLeast, // Tag=Status.* / 제 N대 이상 / 노화 ON일 때만 (§4.4)

GalaxyTickAtLeast, TranscendentSealed, TranscendentAwareOfPlayer, // GalaxyTickAtLeast는 판에 하나인 시계 Tick을 읽는다 — 은하별 시계는 없다 (§4.4) / TranscendentSealed는 bSealed이고 SealedTick >= 문맥의 SinceTick일 때만 참 (§4.4, §7.5)

FlagSet, FlagNotSet, // Tag = Flag.*. FGalaxyState.Flags를 읽는다 (§4.5)

TranscendentDoctrinePresent, // Tag = Doctrine.*. 미봉인 활성 초월자 중 그 교리가 있는가 (§4.2 로스터, §4.4 귀환 게이트)

CivHasNotable, // Tag = Notable.*. 문맥 문명에 그 역할의 열린 기록(LastTick == 0)이 있는가 (§7.2). Key = 문맥 문명 선택자(비우면 문맥 문명 — FCosmicEvalSubject.CivIndex, 호출자가 현재 성계의 소유 문명을 넣는다. Former = FormerCivID). 인물을 읽는 유일한 조건이고 조우 변형 선택에만 쓴다(재회 '알아본다/숨겨준다' = Notable.Loyalist, §5.10). 인물에 의존하는 카드·조건은 만들지 않는다 (§8.3)

CivFallenSphereTicksReached, // Key=틱 수(숫자만, CivStabilityBelowFor와 같은 읽기). 그 문명이 타락 초월자의 영향권에 머문 지 Key틱이 찬 슬롯 한 번만 참이다: Key <= FallenSphereTicks < Key + DT_Rules.EventSlotTicks(1 미만은 1로 읽는다) — 문명의 이벤트 슬롯은 EventSlotTicks틱에 한 번 돌고 카운터는 틱마다 1씩 늘므로, 머무는 동안 슬롯 하나만 그 창에 든다. 그래서 문명 주제(Civilization)의 슬롯 행에게 이 조건은 한 번 머무는 동안 많아야 한 슬롯에서 참이고, 머묾이 Key + EventSlotTicks − 1틱 이상이면 정확히 한 슬롯에서 참이다 — 다른 조건이 맞는 Standing 행은 그때 한 번 나고("책이 닫혔다", §5.11), Lottery 행은 그 슬롯의 추첨에 한 번 든다. 이름으로 부르는 행(창 안의 틱마다 참이다)과 성계 주제(SettledSystem) 행(쿨다운 없는 Standing 행은 성계마다 한 번씩 나고, 그 밖에는 주제와 상관없는 성계 하나에 난다)은 이 조건을 싣지 않는다(스키마). 이 보장은 머무는 동안 EventSlotTicks가 바뀌지 않는다고 본다 — 값이 바뀔 때(다른 규칙으로 불러온 세이브) 창 안이던 머묾은 두 번 나거나 나지 않을 수 있다. Tag(타락 교리 Doctrine.Silence·Assimilation·Zeal·Recurrence — 카운터가 타락의 영향권만 세므로)는 넣지 않아도 된다. 넣으면 판정하는 때에 그 문명의 정착된 성계 가운데 하나가 그 교리의 봉인되지 않은 초월자 영향권 안에 있어야 한다(카운터는 교리를 가리지 않는다). Value는 쓰지 않는다 (§16 #25 ①). 열거형은 끝에만 추가한다 (§9.1). 앎(Knowing)을 읽는 값은 두지 않는다 — 세계는 앎을 쓰기만 하고 읽지 않는다 (§8.1)

CivResearchAtLeast // 끝에 더한 것(Key = 연구 영역 Physics/Society/Engineering/Transcendence — 스키마의 낱말 목록 research, 영역이 아닌 Key는 거짓, Value 0~1: 문맥 문명의 Research[Key] >= Value — 거대 프로젝트와 승천 판정 작업의 T1→T2 업적 '물리 50%'). 정해 둔 추가분(§16 #29, 2026-10-15 — 처음 쓰는 행과 함께 끝에 더한다): CivStabilityAtLeast(Stability >= Value — Phase 4 시대 명명 ≥ 0.6, 느부갓네살 > 0.9는 그 행에서 정한다), PlayerHasPower(Tag = Power.* — Phase 4). Key 선택자 낱말(#29 ⑦): 비우면 문맥 문명(문명을 고르는 자리)·문맥의 상대 OtherID(상대를 고르는 자리), Former = FormerCivID, Ruled = 통치 문명(Embodied일 때만), SystemOwnedBy의 Key도 선택자다. 코드에는 지금 Former만 있고 나머지는 그 낱말을 처음 쓰는 행(Phase 4 청원 카드)과 함께 들어온다 — 그때까지 슬롯 행은 Player 조건·역할 효과·그릇 효과를 싣지 않는다(스키마)

SystemHazardTicksAtMost // 끝에 더한 것(MVP ②): Tag = Hazard.*(필수 — 비면 거짓), Value = 틱(1 이상의 정수). 주어 성계가 그 위기를 갖고 HazardTicksRemaining <= Value. 카운트다운은 틱마다 1씩 줄어 '잔여 50틱'을 기다리는 청원은 정확히 한 틱에 조건이 선다(§4.1 ③, §5.10 '왕좌를 지키겠는가')

CivSettledSystemsAtLeast // 끝에 더한 것(MVP ③): Value = 1 이상의 정수. 문맥 문명이 가진 정착된 성계(소유 ∧ CosmicState::IsSettled — 다른 백성이 가진 OwnedSystems 항목과 정착 중인 성계는 세지 않는다)가 Value 이상. 주어 성계는 읽지 않는다. 둘째 T2→T3 업적 feat_three_worlds(정착 3 ∧ 공학 1.0)의 조건이고(성계의 수라 정복한 성계도 센다 — 정복이 셋째 성계를 채우면 그 틱에 셋째 칸에 오른다, 헤더 B3 결정 11), 업적 행의 이 값은 그 칸의 보유 상한(RealmCapByTier[RequiredTier])을 넘지 않는다(Data.Integrity)
CivHasEdict // 끝에 더한 것(B4 — 헤더 `headers/B4_transcendent_voice_and_pressure_cards.md` 2.2절): Key = `DT_Edicts` 행. 문맥 백성의 `ActiveEdicts`가 그 행을 지니고 테이블이 그 행을 가질 때 참(`CosmicEdicts::HeldCountOf`와 같은 셈 — 테이블에 없는 이름은 거짓). 세계만 읽는다(`Check`는 static). 은혜 행(`Event.Grace.*`)의 조건에는 쓰지 않는다(스키마 — 은혜는 칙령으로 끌어오지 않는다). 지금 싣는 행: `event_claim_eased`(`edict_vigil` — B4 `[B-4d2]`)

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

KillPlayer, DeposeRuler, // 즉시 실행하지 않고 FCallingState.PendingRoleEffect에 기록만 한다. 틱 경계의 '역할 효과 해소' 단계에서 UPlayerRoleSubsystem의 사망·폐위 파이프라인으로 위임 (§4.4, §5.10, §7.4). Tag = Death.* / Reign.*(데이터의 KillPlayer는 Death.Hazard뿐이다 — 스키마, 서는 자리는 §7.3 FHazardDef.OnExpireEffects, B5 `[B-5d2]`). Phase 6+: InstallAt(기원 선택 카드)

ChangePopulation, // Value = 비율. 역병 미해결 −0.4 (§4.1), 희생 이벤트 event_zeal_sacrifice −0.05 (§4.2). 문명 Population에만 — 성계 단위 인구는 없다

NameNotable // Tag = Notable.*, Value 0. 문맥 문명(Key = 선택자, 비우면 문맥 문명 — 호출자가 현재 성계의 소유 문명을 넣는다, Former = FormerCivID)에 인물 기록 1개 (§1.3, §7.2). 가드는 UCosmicEventSubsystem::Apply (§7.5): Tier <= 3, PersonNameBank 비어 있지 않음, 같은 Role 열린 기록 닫기, 상한 MaxNotablesPerCiv 퇴출. 이름 추첨은 적용마다 정확히 1회, 제 가드보다 먼저다(틱당 원천은 이 효과를 걸지 않으므로 뽑지도 않는다, §7.5). VS에 이 효과를 싣는 행: crisis_rebellion 발생 전조 ②(Notable.Rebel — Severity 1 유일 예외, §4.1), 망명 변형 '충신이 몰래 배를 내준다'(Notable.Loyalist, Former), crisis_visitation 행의 OnResolveEffects(Notable.Hearer), '책이 발견되었다'(Notable.Hearer). 금지: enc_messenger_*·Event.Grace.*·Hazard.Cult 행 (Data.Integrity). 인물 기록을 만드는 효과는 이것 하나이며 대상은 개인이 아니라 문명이다. 열거형은 끝에만 추가한다 (§9.1)

FocusResearch, // 끝에 더한 것(MVP ②): Key = 연구 영역(문맥 문명의 ResearchFocus 키 — 스키마의 낱말 목록 research), Value = 그 몫(0 초과 1 이하, 스키마는 0.25~1). ResearchFocus[Key] = Value, 나머지 영역은 (1 − Value) / (영역 수 − 1)씩 — 합은 1이다(§5.4). 개국 청원 '지혜로운 마음'의 세 길 — 통치자의 연구 중점 명령(MVP ⑦)은 같은 규칙을 세계 함수 `CosmicResearch::Focus`로 쓴다(프리셋은 효과를 짓지 않는다). 틱당 원천은 걸지 않는다

SetStance // 끝에 더한 것(B3 — 헤더 B3 2.9절): 문맥 백성이 Key가 적은 백성(빈 Key = 문맥의 OtherID — 선택자 낱말은 풀지 않는다)을 대하는 태세. Tag = Stance.Friendly 또는 빈 태그(중립), Value 쓰지 않음. 명령과 같은 문 CosmicDiplomacy::StanceBlockOf(bMet = true — 세계 코드는 앎을 읽지 않는다)가 None일 때만 쓰고 거절은 무동작(false)이다(효과도 단일 창구를 지난다, 불변 규칙 13). 적대 태그는 스키마가 막는다(적대는 명령의 문으로만 — 코드가 받으면 다른 적대처럼 무역 조약을 끊는다, 값 없이). 카드의 답에만 선다(스키마의 효과 칸 card_only와 열 card — DT_Encounters의 OnSuccess·OnFailure, 그리고 첫 접촉 카드가 아닌 행에는 없다 — 행 시험 row_not). 틱당 원천이 아니다. 지금 싣는 행: petition_first_contact의 '손을 내민다'(Stance.Friendly)
EnactEdict // 끝에 더한 것(B4 — 헤더 B4 2.2절): Key = `DT_Edicts` 행. 문맥 백성(카드의 답 — 다스리는 백성)이 그 칙령을 내린다: 하나뿐인 문 `CosmicEdicts::EnactBlockOf`가 None일 때만 `CosmicEdicts::Enact`로 그 칙령의 `EnactCost`를 낸다(선택의 `Cost`는 비어야 한다 — 스키마). 거절은 무동작(false — 답의 문 `CosmicPetitions::ChoiceGateOf`가 수를 뽑기 전에 먼저 거절한다). 줄을 내지 않는다(카드의 결과 줄이 말한다). 카드의 답에만 선다(`card_only`, `petition_` 행의 판정 없는 선택의 `OnSuccess`만 — 스키마; 그 선택에서 문이 있는 효과로는 혼자이고 `OnSuccess`의 첫째이며 선택의 `Label`은 그 칙령의 `DisplayName`을 글자 그대로 품는다 — `Data.Integrity`). 틱당 원천이 아니다. 지금 싣는 행: 영압 카드 두 장(`petition_pressure_claim`의 `edict_vigil`, `petition_pressure_measured`의 `edict_against_silence` — B4 `[B-4d2]`)
DelayHazard // 끝에 더한 것(B4 — 헤더 B4 2.5절, #127은 이것으로 닫혔다 — `DECISIONS_LOG` 2026-10-27 B4): Tag = `Hazard.*`(필수), Value = 틱(1~100의 정수). 하나뿐인 문 `CosmicConflict::ResistBlockOf`(문맥 백성·문맥 성계·Tag)가 None일 때만 그 성계의 `HazardTicksRemaining`에 `CosmicConflict::ResistTicksOf(Value)`(정수 부분 — 1 아래·무한·수가 아님은 0이고 효과는 거절한다)를 더하고(`MAX_int32`에서 자른다) 그 백성의 `FleetPower`에 `DT_Rules.ResistFleetFactor`를 곱한다(`CosmicConflict::ResistShareOf` — [0, 1] 밖은 1). 격차를 읽지 않는다. 서 있는 위기의 남은 시간이 늘어나는 하나뿐인 길이다(칙령과 지킴은 놓일 때만). 수도 규칙 행의 태그와 어느 `SystemHazardTicksAtMost`가 읽는 태그는 안 된다(스키마 — 태그 칸의 `not_tag_of`·`not_read_by`). 카드의 답에만(`card_only`, `petition_` 행의 판정 없는 선택의 `OnSuccess`만 — 스키마; 그 선택에서 문이 있는 효과로는 혼자이고 첫째 — `Data.Integrity`), 틱당 원천이 아니다. 지금 싣는 행: 둘째 영압 카드 `petition_pressure_measured`(`Hazard.Claim`, 50 — B4 `[B-4d2]`)

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

UPROPERTY(EditAnywhere) FGameplayTag Category; // Event.Civ.Economy/Society, Event.Crisis.Supernova/Transcendent/Rebellion/Claim, Event.Grace.Rescue/Remnant/Jubilee (DefaultGameplayTags.ini의 Event.* 트리 9개와 동일하게 유지). 그 행의 가중 계열이다 — 초월자의 가중 수정자가 이 태그로 건다(특성 행의 가중은 Phase 4. 칙령 행은 이벤트 가중을 싣지 않는다 — B4의 철야의 가중은 칙령의 가중이 아니라 추첨 행 하나 event_claim_eased(Event.Crisis.Claim, 조건 CivHasEdict edict_vigil)로 섰다. 어느 초월자 행도 Event.Crisis.Claim에 무게를 싣지 않는다 — 스키마의 contains_none — 신정론의 몫을 지키려고, §7.3 정의 스키마의 `rules` 항목의 신정론 불변식). 로그 줄의 카테고리는 LogCategory다

UPROPERTY(EditAnywhere) FGameplayTag LogCategory; // Log.*. 이 행이 내는 로그 항목의 Category. 코드는 베끼기만 하고 Event.*에서 얻지 않는다. 허용 짝은 Tools/data/schema.json (§8.1)

UPROPERTY(EditAnywhere) ECosmicEventTrigger Trigger = ECosmicEventTrigger::Lottery; // Lottery(슬롯의 추첨, 문명·슬롯당 1행 이하) / Standing(조건과 쿨다운만 맞으면 슬롯에서 반드시 난다 — 주기 행, 추첨·가중 없음) / Named(슬롯이 보지 않는다. 코드가 FireRow로 낸다). 열거형은 끝에만 추가한다

UPROPERTY(EditAnywhere) ECosmicEventSubject Subject = ECosmicEventSubject::Civilization; // Civilization(문명에 한 번. 문맥 성계 = 정착된 수도. 줄의 주제는 문명만 — 수도를 모르는 만난 백성의 줄도 말해진다. 그래서 그 줄은 성계를 부르지 않는다, 스키마) / SettledSystem(정착된 성계마다, OwnedSystems 순서. 줄의 주제는 성계와 문명) / VesselSystem(끝에 더한 것 — B1: DT_Encounters 전용, 방랑자 카드의 주제 = 그릇이 선 성계(정착된 성계든 무주든)와 그 주인(없으면 없음). DT_Events 행은 쓰지 않는다 — 스키마). 열거형은 끝에만 추가한다

UPROPERTY(EditAnywhere) TArray<FCosmicCondition> Conditions; // AND

UPROPERTY(EditAnywhere) float BaseWeight = 1.f;

UPROPERTY(EditAnywhere) int32 CooldownTicks = 200;

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> Effects;

UPROPERTY(EditAnywhere) TArray<FText> LogTemplates; // Lottery·Standing 행은 3개 이상, Named 행은 1개 이상 (Data.Integrity). 슬롯은 {Civ} {System} {Hazard} 셋이고 Civilization 주제 행은 {Civ} {Hazard}만 부른다(줄의 주제 성계는 그 줄이 이름을 부르는 성계다 — 수도를 곁에 부르지 않는다, §8.1. 아는 범위와 로그 작업의 리뷰. 코드도 {HomeSystem}을 채우지 않는다 — 재감사. {Hazard} = 행이 발화하기 시작할 때 주제 성계가 쥔 Hazard의 이름 — 효과가 그것을 해결해도 그 이름이다. 위기 처리 작업. §8.2의 다른 슬롯은 이벤트 행의 줄에서 채워지지 않는다 — Data.Integrity가 막는다). 비면 줄을 내지 않는다(SignRow의 줄도). BaseWeight는 Lottery 행만 쓴다(1 = 자격 있는 단위에 대해 슬롯당 DT_Rules.EventChancePerSlot의 확률. Standing·Named 행은 0). CooldownTicks는 (행, 문명)마다다 — 키 "<행>@<CivID>", 0 = 쿨다운 없음, Named 행은 0

UPROPERTY(EditAnywhere) uint8 Severity = 0; // 0 일반, 1 주목, 2 신화적

UPROPERTY(EditAnywhere) uint8 MinTierToReveal = 0; // LogTemplates가 말해지는 가장 낮은 보는 쪽 Tier. 그 아래에는 SignRow가 있으면 그 줄이, 없으면 아무것도 말해지지 않는다(Severity 2·Log.Claim은 bUntold로 쌓인다, §8.1 — 이벤트 행은 Log.Record를 말하지 못한다: 그 짝이 스키마의 event_log_categories에 없다, B5 `[B-5d1]`). 초월자 이벤트만 템플릿 2벌 (§8.2). 값은 Tier 번호 1~5(3 = Tier 3, 열거형 값이 아니다), 0 = 누구에게나

UPROPERTY(EditAnywhere) FName SignRow; // DT_LogTemplates의 Log.Sign 행 — 하위 화자의 문장. 초월자 이벤트의 둘째 템플릿 벌은 이벤트 행이 아니라 여기에 있고 여러 행이 함께 쓴다. NAME_None = 그 Tier 아래에는 아무 줄도 없다. MinTierToReveal 2~5이고 LogTemplates가 있는 행에만 쓴다(스키마가 MinTierToReveal ≥ 2를 본다 — 1 이하면 그 아래에 보는 이가 없고, 줄이 없는 행은 징조도 내지 않는다). 가리키는 행의 Category는 Log.Sign이고 그 템플릿은 {Civ} {Hazard}만 부른다(스키마의 target 규칙 — Data.Integrity는 이것을 보지 않는다. 징조 줄은 이벤트 행의 줄의 초점을 그대로 쓰고(Civilization 행이면 주제 성계가 없다) 여러 행이 함께 쓰므로 성계를 부르지 않는다. 코드가 이름으로 부르는 징조 행 log_sign_claim(B4 — 주장의 1인칭 줄 앞의 징조, CosmicRows::LogSignClaim)은 이벤트 행이 가리키지 않아 이 규칙 밖이고 제 행 규칙으로 {System}·{Civ}를 부른다. LogCategory가 Log.Calling이고 MinTierToReveal이 2 이상인 행은 SignRow를 가져야 한다 — 창을 가진 부름의 줄은 징조와 짝이다, §8.1. 둘 다 아는 범위와 로그 작업의 재감사. LogCategory가 Log.Transcendent인 행은 MinTierToReveal ≥ 3과 SignRow를 지닌다 — B4가 앞으로의 행에 둔 문, 지금 그런 행은 없다). 징조 줄을 내는 길은 UCosmicLogSubsystem::PushWithSign 하나다(§7.5 — 징조가 먼저, 그다음 이 행의 줄)

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

UPROPERTY(EditAnywhere) int32 CountdownTicks = 300; // 1 미만은 1로 읽는다. 놓일 때 그 성계 주인이 지닌 칙령의 HazardCountdownBonus(이 태그)를 더하고(B2), 그 성계를 지키는 함대의 지연(CosmicConflict::FleetDelayOf — Capital 행 밖, B3)도 더한다 — 놓는 길 하나 CosmicHazards::Place에서만, 서 있는 Hazard의 카운트다운은 바뀌지 않는다 — 예외는 카드의 답 효과 DelayHazard 하나다(B4 — 플레이어가 함대로 한 번 사는 시간, §7.3 효과 표). Hazard.Claim도 제 행으로 센다(옛 DT_Rules.ClaimCountdown은 지웠다)

UPROPERTY(EditAnywhere) ECosmicHazardSpawn SpawnRule = ECosmicHazardSpawn::Draw; // Draw·Capital·ClaimRule. 효과나 코드 훅만 놓는 위기(방문·징조)는 Phase 4의 헤더가 열거자 하나를 더한다(예: Placed) — Draw 행의 확률은 0보다 크다(스키마, Phase 2 종료 리뷰의 재감사)

UPROPERTY(EditAnywhere) float SpawnChancePerTick = 0.0005f; // Draw 행만: 한 추첨에서의 몫 × 주인 아키타입의 HazardSpawnMultipliers[HazardTag](없으면 1) × 주인이 지닌 칙령의 HazardSpawnMultipliers 곱(CosmicEdicts::SpawnMultiplierOf — B2. 몫만 바뀌고 추첨 수는 그대로), 역병(Hazard.Plague) 행은 레인 이웃에 역병이 서 있는 정착된 성계에서 × DT_Rules.PlagueSpreadMultiplier(역병의 번짐 — B3, §7.4 위기 단계 ⓒ). 몫의 합이 1을 넘으면 수에 합을 곱한다(이벤트 슬롯과 같다). Capital·ClaimRule 행은 0(Data.Integrity). 0 아래나 숫자가 아닌 값은 0. 추첨은 정착된 성계만 돈다 — 무주 성계와 정착 중인 성계는 뽑지 않고 건너뛴다 (§5.7)

UPROPERTY(EditAnywhere) TArray<FCosmicCondition> SpawnConditions; // AND. 주제 = 그 성계와 주인. 단계가 틱마다 읽으므로 머묾 조건(CivFallenSphereTicksReached)은 싣지 않는다(Data.Integrity)

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> OnExpireEffects; // 주제 = 그 성계와 주인, SourceRow = 이 행. KillPlayer는 그 성계에 선 방랑자에게만 (§4.4 — CosmicHazards::VesselAtRiskOf: 성계의 일은 통치자를 죽이지 않는다). 예외(예정 — Hazard.Extinction의 행이 아직 없다, §16 #138): 플레이어가 통치하는 문명이 Hazard.Extinction의 만료로 성계를 모두 잃으면 위치와 무관하게 통치자도 사망한다 (§4.1, §4.4). 그 문명(플레이어가 통치하던 문명)은 그 틱이 아니라 다음 틱의 건넘 단계에서 제거된다 — 플레이어가 통치하지 않는 문명은 같은 틱의 건넘 단계에서 제거된다 (§7.4). KillPlayer는 위기 행의 이 칸에만 선다(스키마, 헤더 B5 §3.5 — `[B-5d2]`): 갈래·이웃·해결 칸(OnExpireBranchEffects·OnExpireNeighbourEffects·OnResolveEffects)에는 없고, HazardTag가 Hazard.Claim·Hazard.Visitation·Hazard.Cult인 행과 SourceDoctrine이 선 행(태그와 상관없이 — 타락은 생애에 닿지 않는다, `[B-5e]`)에는 없으며, Hazard.PreSupernova 행에는 제 죽음 줄 행이 오기 전까지 없다. 사인은 Death.Hazard뿐이다(Death.Transcendent는 데이터 어디에도 없다). 지금 싣는 행: crisis_ai_awakening 하나(결정 6-가 — 게임의 테이블에 든 죽음이 정확히 이것인지는 Data.Integrity, 그 죽음은 Calling.LethalSource가 본다). crisis_rebellion은 DeposeRuler (§4.1). 만료 효과(갈래 포함)는 TransferSystem과 ResolveHazard를 싣지 않는다 — 성계를 앗는 것은 문 뒤의 단계 코드뿐이고, 만료가 제 성계를 다시 해결하지 않는다(Data.Integrity)

UPROPERTY(EditAnywhere) float BranchChance = 0.f; // 만료의 결과 수가 이것보다 작으면 OnExpireEffects 대신 OnExpireBranchEffects (AI 각성의 50/50, §4.1). 0 = 갈래 없음

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> OnExpireBranchEffects;

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> OnExpireNeighbourEffects; // TransferSystem은 싣지 않는다(이웃의 Hazard를 해결하는 것은 된다 — 그 해결 효과도 TransferSystem을 싣지 않는다, OnResolveEffects). 만료 효과 뒤, 태그의 코드 훅 앞: 죽은 노드가 아닌 레인 이웃마다 그 Lanes 순서로 한 번(주제 = 그 이웃과 주인, SourceRow = 이 행). 초신성의 이웃 피해. 역병의 번짐은 이 칸이 아니다 — 역병은 거의 만료하지 않아 만료에 걸면 번짐이 일어나지 않는다. B3는 번짐을 발생 몫의 이웃 곱으로 넣었다(DT_Rules.PlagueSpreadMultiplier, §7.4 위기 단계 ⓒ — 추첨 수는 그대로). 그 번짐을 줄이는 격리령(edict_quarantine)은 B2의 칸 HazardSpawnMultipliers로 제 땅의 발생과 번져 오는 역병을 함께 줄인다

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> OnResolveEffects; // 해결 시 1회, 슬롯을 비운 뒤(주제 = 그 성계와 주인, SourceRow = 이 행). R·W·AI 해결 모두 같은 길(CosmicHazards::Resolve)이다. ResolveHazard는 싣지 않는다(해결이 놓은 것을 다시 해결하면 끝나지 않는다 — 코드도 깊이 4에서 멈추고 에러 한 줄, Data.Integrity). TransferSystem도 싣지 않는다(만료의 이웃 효과가 부르는 해결이 성계를 넘기면 문 뒤가 아닌 곳에서 성계를 앗는다 — §4.5, Data.Integrity). crisis_visitation: Trait.Faith.Visited 부여 + NameNotable(Notable.Hearer)

UPROPERTY(EditAnywhere) TArray<FName> RulerResolutions; // DT_Megaprojects·DT_Edicts·DT_Encounters(청원) 행 이름 — 두 갈래로 답한다. 여기 든 DT_Megaprojects 행이 그 Hazard의 대응 프로젝트다 — 대응 칸에서 돌고 완성되면 그 Hazard를 해결하며 Hazard와 함께 끝난다(§5.7, 거대 프로젝트와 승천 판정 작업부터. 어느 행이 대응 프로젝트인지는 FCosmicDataTables::IsHazardAnswer — 칙령 이름에도 참이지만 그 이름으로 부르는 곳이 없다). 여기 든 DT_Edicts 행(B2)은 만료 때 답한다: 만료하는 틱에 그 성계 주인이 그 칙령을 지니면 만료 대신 해결된다(CosmicEdicts::HeldAnswerOf — 이 배열의 차례로 지닌 첫 칙령, §7.4 위기 단계 ⓑ). DT_Encounters 이름은 Phase 4부터 읽는다(지금은 싣는 행도 읽는 코드도 없다). 지금의 짝: 반란 → project_reform, 초신성 전조 → project_stabilize, 주장·역병 → project_relief(역병은 MVP ⑧부터), AI 각성 → edict_machine_rights(B2). 스키마의 종류 rows(DT_Megaprojects·DT_Encounters·DT_Edicts — B2가 칙령 테이블을 더했다)

UPROPERTY(EditAnywhere) TArray<FName> WandererResolutions; // DT_Encounters 행 이름 — 예약. B1에서도 모든 행이 비어 있고 시뮬 코드가 읽지 않는다: 방랑자 카드는 행의 조건(PlayerInSystemWithHazard)으로 오고, 위기를 끝내는 카드는 성공 효과 ResolveHazard로 끝낸다(지금은 enc_rebel_parley·enc_awakening_talk 둘 — 역병과 초신성 전조의 카드는 끝내지 않는다)

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

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> AscensionCost; // 승천의 대가 (§3.2). KillPlayer는 어느 길(Path)의 행에도 싣지 않는다 — 죽음은 위기의 만료에서만 온다(스키마, B5 결정 6-가 — `[B-5e]`부터 방랑자 행도; 통치자 행의 나머지 규칙은 아래 GrantedPowers의 '업적 행 규칙')

UPROPERTY(EditAnywhere) FGameplayTagContainer GrantedPowers; // T4->T5 행은 Way.Received 또는 Way.Seized를 포함 (§3.2). Law.Humility: Way.Seized는 어떤 Feat도 떼지 못한다. Power.* 태그는 권능을 주는 첫 업적 행과 함께 ini에 추가한다(Phase 2의 두 행은 권능을 주지 않는다). 업적 행 규칙(스키마): RequiredTier에서 TargetTier까지 한 칸(T5에서 오르는 행 없음), CompletionConditions 1개 이상, 통치자 업적 행에는 그릇을 읽는 조건과 역할·그릇에 닿는 대가 없음(모든 백성을 판정한다 — 방랑자 행은 §16 #28) — 판정도 한 칸이 아니거나 조건이 없는 행은 에러 한 줄과 함께 판정하지 않는다

UPROPERTY(EditAnywhere) FGameplayTag DedicationBank; // 완성 시 그 문명이 붙이는 헌정명 뱅크 (§8.6. Phase 4: Epithet + 첫 생애 백성의 의회·회중 최소형, 뱅크 확장은 Phase 6+). DT_Edicts·거대 프로젝트 행에도 같은 컬럼(DT_Dedications와 함께 — FEdictDef·FMegaprojectDef에는 아직 없다)

UPROPERTY(EditAnywhere) TSoftObjectPtr<class UNiagaraSystem> FX;

UPROPERTY(EditAnywhere) TSoftObjectPtr<class USoundBase> Sound;

UPROPERTY(EditAnywhere) TArray<FText> LogTemplates; // 승천 줄(Log.Ascension, Severity 2 — AI 백성도), 오른 백성과 그 수도에 대해, SourceRow = 이 행. 슬롯 {Civ} {HomeSystem}. 3변형 이상 (거대 프로젝트와 승천 판정 작업)

};

// DT_Encounters

// 선택지의 판정 능력치(MVP ② A-2a — 정의 열거형, 저장되지 않는다, 끝에만 추가). 그릇의 능력치는 0~10, Stability는 다스리는 백성의 Stability(0~1)를 (int32)(Stability × 10)으로 읽는다(§16 #30 ①). Influence는 판정이 아니라 비용이다. 데이터는 빈 이름을 "None"으로 쓰므로 '판정 없음'은 NoCheck다. 방랑자 카드(enc_ 행)는 그릇의 능력치와 NoCheck만 쓴다 — 백성의 Stability는 방랑자의 것이 아니다(스키마, B1).

UENUM(BlueprintType) enum class ECosmicCheckStat : uint8 { NoCheck, Body, Mind, Causality, Exaltation, Stability };

USTRUCT(BlueprintType)

struct FEncounterChoice

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) FText Label;

UPROPERTY(EditAnywhere) ECosmicCheckStat CheckStat = ECosmicCheckStat::NoCheck; // 판정: 능력치 + 0~5 ≥ Difficulty (§5.9). 통치자 청원은 Stability도 (§5.10)

UPROPERTY(EditAnywhere) int32 Difficulty = 0; // >= 16 = 불가능한 선택지(영압 게이트, §4.2)

UPROPERTY(EditAnywhere) FResourcePool Cost; // 통치자의 청원은 다스리는 백성의 Stockpile에서 낸다(CosmicPetitions::CostOf — 0 아래·유한하지 않은 멤버는 0) (§5.10). 방랑자 카드는 비용을 보지 않는다 — 주제 VesselSystem 행의 비용은 0이어야 한다(C++ Data.Integrity의 선택지 검사, B1). PersonalPool에서 내는 길은 그 풀의 수입과 함께(B1 밖)

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> OnSuccess; // DeposeRuler를 싣는 선택지는 그릇이 스스로 떠나는 것이다 — 판정 없음(NoCheck)이고 태그는 Reign.Abdicated(C++ Data.Integrity의 선택지 검사, B1 — 왕좌 카드 petition_keep_throne의 셋째 선택지). 통치자 정책은 이런 선택지를 고르지 않는다(CosmicRulerPolicy::ChoiceOf). SetStance(B3)는 첫 접촉 카드의 답에만 선다(스키마 — 아래 FEncounterDef). 그 효과의 태세를 문이 거절하는 선택지(우호의 자리가 찼다 — CosmicPetitions::StanceBlockOfChoice)는 카드가 회색으로 두고 답이 거절한다(내지 못하는 비용과 같은 꼴, §7.4 앎 단계의 '첫 접촉 카드'). B4가 같은 꼴의 문을 둘 더했다 — EnactEdict(칙령을 내린다)와 DelayHazard(함대로 서 있는 위기의 시간을 산다)는 통치자의 청원(petition_ 행)의 판정 없는 선택지에만, 그 선택지의 OnSuccess 첫째에 선다 — 문이 있는 효과(SetStance·EnactEdict·DelayHazard)는 한 선택지에 하나뿐이다(§7.3 효과 표). 세 효과를 묻는 선택지의 문은 CosmicPetitions::ChoiceGateOf 하나다(§7.4 'B4' 줄 — 답·카드 창·정책이 같은 답을 읽는다). 문이 있는 선택지를 지닌 카드는 값도 문도 없는 선택지 하나를 지닌다 — 대기 칸이 하나라 모든 선택지가 막힌 카드는 그 뒤의 모든 카드를 막는다(스키마의 행 시험 none)

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> OnFailure; // OnSuccess처럼 죽음을 적지 않는다 — 카드는 KillPlayer를 싣지 않는다(스키마 — 죽음은 위기의 만료에서만 온다, §7.3 FHazardDef.OnExpireEffects; 카드의 치명은 §16 #151)

UPROPERTY(EditAnywhere) FText SuccessText;

UPROPERTY(EditAnywhere) FText FailureText;

UPROPERTY(EditAnywhere) FName NextEncounter; // 체인용. 첫 접촉 카드로는 잇지 않는다 — 답이 놓은 카드는 만난 상대가 없다(스키마의 대상 시험 row_not, B3)

};

USTRUCT(BlueprintType)

struct FEncounterDef : public FTableRowBase

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) FText Title; // 글 칸(Title·Body·VoiceBodies와 선택지의 Label·SuccessText·FailureText)의 슬롯은 {Civ} {System} {Hazard} {Rebel}(Civilization 행은 성계를 부르지 않는다), 그리고 첫 접촉 카드만 {OtherCiv}(상대 백성의 이름 — CosmicPetitions::ArgsOf, B3). 첫 접촉 카드 petition_first_contact(CosmicRows::PetitionFirstContact, 스키마의 required_rows)가 아닌 행은 {OtherCiv}도 SetStance도 갖지 않는다 — 답이 상대를 주는 카드가 그것뿐이다(스키마의 행 시험 row_not). 그 카드는 Named·Civilization이다(첫 만남이 이름으로 놓는다 — Standing·Lottery면 재위마다 스스로 온다). 통치자의 청원(petition_ 행)의 글 칸은 모두 판정 어휘에 더해 holy_words·soul_verbs·rescue_words의 낱말도 품지 않는다(B4 — 스키마의 행 규칙 lacks_words; 글의 규칙은 docs/70)

UPROPERTY(EditAnywhere) FText Body;

UPROPERTY(EditAnywhere) TArray<FCosmicCondition> Conditions;

UPROPERTY(EditAnywhere) float BaseWeight = 1.f;

UPROPERTY(EditAnywhere) TArray<FEncounterChoice> Choices; // 2~3. 로컬라이제이션 키가 위치라 끝에만 붙인다(§8.5)

UPROPERTY(EditAnywhere) TSoftObjectPtr<class UTexture2D> Illustration; // 선택

UPROPERTY(EditAnywhere) ECosmicEventTrigger Trigger = ECosmicEventTrigger::Named; // 앎 단계가 카드를 내놓는 때(§5.10): Standing = 조건이 막 선 틱, Lottery = 주기 슬롯의 가중 추첨, Named = 이름으로만(사슬의 다음 카드). 방랑자 카드(B1): Standing = 그릇이 선 성계에서 조건이 막 선 틱(도착한 틱이면 새 성계의 위기가 막 선 것이 된다), Lottery = 도착한 틱의 도착 카드(DT_Rules.EncounterChancePerArrival의 몫 — CosmicPetitions::Present)

UPROPERTY(EditAnywhere) ECosmicEventSubject Subject = ECosmicEventSubject::Civilization; // 다스리는 백성(Civilization — 수도) 또는 그 백성의 정착된 성계 하나(SettledSystem) — 통치자의 청원. VesselSystem = 그릇이 선 성계와 그 주인(무주면 없음) — 방랑자 카드(B1. 스키마가 enc_ 행을 여기에, petition_ 행을 앞의 둘에 묶고, 답 명령은 이 값으로 두 종류를 가른다 — CosmicPetitions::IsWandererCard)

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

UPROPERTY(EditAnywhere) TArray<FText> PersonNameBank; // 인물(FNotableRecord.NameIndex)과 그릇(FLifeRecord.NameIndex)의 이름 뱅크 (§1.3, §5.11). NoSelf가 아닌 행 12개(Data.Integrity 하한 10, §9.3) — Data.Integrity: 중복 없음, 성경 인물·실존 인물·천사·실존 종교 고유명사 금지, Data.NoVerdictWords 적용(Phase 5 — 지금은 스키마의 forbidden: verdict). arch_swarm·arch_machine은 비움 = 이름 없는 문명(NameNotable 무시 — Trait.Mind.NoSelf 보유 문명도 동일). 렌더 Bank[NameIndex % Bank.Num()] — 뱅크가 줄어도 구 세이브가 깨지지 않는다. arch_default의 뱅크는 그릇 이름의 마지막 자리라(뱅크가 빈 백성의 그릇이 그 뱅크로 이름을 받는다 — CosmicChronicle::VesselNameOf) 스키마가 10개 이상을 요구한다(B5 `[B-5d1]` — 빈 이름은 죽음·취임 줄의 주어를 지운다). 문명 NameBank({Star} 패턴)와 별개 컬럼

UPROPERTY(EditAnywhere) TMap<FGameplayTag, float> HazardSpawnMultipliers; // Hazard.* -> 배율 (선호 Hazard). 추첨(Draw) 행에만 닿는다 — 없는 키 = 1, 0 아래나 숫자가 아닌 값은 0 (§4.1)

UPROPERTY(EditAnywhere) TMap<FName, float> InitialRelations; // 다른 아키타입 행 -> 초기 관계. 읽는 곳이 아직 없다(두 행 모두 비었다 — 읽기는 AI 백성 다섯 행과 함께 B7. B3의 관계는 비어 시작해 표류로 움직인다)

UPROPERTY(EditAnywhere) float Weight = 1.f; // NewGalaxy AI 추첨 가중치. 0 = AI 추첨 제외 (첫 생애 행 arch_seeker — A0 4b부터, 그 전 arch_terran(은퇴): 플레이어 전용, DT_Rules.PlayerArchetypeRow에서만 참조. 스키마 규칙: arch_seeker는 Weight 0·Trait.Arch.Seeker·Revelation.Seeking)

UPROPERTY(EditAnywhere) FGameplayTag RevelationMode; // Revelation.* 계시 양식 (§5.11). 모든 행이 정확히 하나. 텍스트만 바꾸고 수치는 바꾸지 않는다. arch_default는 Revelation.Seeking 고정

UPROPERTY(EditAnywhere) TArray<FText> RevelationTemplates; // 정확히 3개(Data.Integrity): [0] 징조(Log.Sign) / [1] 전언(enc_messenger_* 간접 화법) / [2] 지나감·응답(Hazard.Visitation 해결·만료). 수동태·간접 화법만

UPROPERTY(EditAnywhere) TArray<FText> HomeSystemNames; // 모성계 표시명 뱅크. 첫 생애 행 = 그 백성이 제 해를 부르는 이름(숫자 없음 — A0 4b부터, MVP ⑧에서 {첫 해}를 {이른 해}로, 자리 채움 — 단계 7에서 다시 쓴다. A0 4a에서 arch_terran은 비웠다 — 성역과 이름이 겹치지 않게). 비면 FGalaxyDef.StarNameBank (§5.11)

UPROPERTY(EditAnywhere) FGameplayTag HomeStarClass; // 모성계 항성 등급 고정 (첫 생애 행 = Star.Class.G — v0.8 — 단계 5까지, 그 뒤 실제 별 등급 + 정규화). 빈 태그 = DT_Rules.LivingWorldStarShares의 분포에서 뽑는다 (§7.4)

UPROPERTY(EditAnywhere) TArray<FGameplayTag> DedicationBanks; // Faith.Bank.* (§8.6). 보통 1개. 첫 생애 행 = {Faith.Bank.Nations, Faith.Bank.Civic} — 말이 온 뒤 Trait.Faith.Book의 효과로 Hebrew를 더한다(첫 생애 행의 값은 A0 4b부터, 말이 온 뒤의 Hebrew는 단계 7부터. 그 전 arch_terran = {Hebrew, Civic}) — 청원한 파벌이 고른다 (§5.11). Feat 행의 DedicationBank(칙령·프로젝트 행의 같은 컬럼은 DT_Dedications와 함께 — Phase 4)는 '어떤 이름 종류'를, 이 컬럼은 '어느 뱅크'를 정한다

};

// DT_Transcendents (§4.2). 행 이름 == TranscendentID. 초월자는 판에 교리마다 1체다 — 은하마다 따로 두지 않고 타락은 건너지 않는다 (§4.2). VS 1행 trans_silence. EA: 동화·광신·회귀 행 중 2행이 NewGalaxy 가중 추첨으로 활성 + 천상 trans_covenant(상태 인스턴스 없음). Phase 6+: 독립 trans_stillness·trans_release

USTRUCT(BlueprintType)

struct FTranscendentDef : public FTableRowBase

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) FGameplayTag Doctrine; // Doctrine.* (고유. Data.Integrity)

UPROPERTY(EditAnywhere) FGameplayTag Allegiance; // 소속: Realm.Abyssal / Realm.Celestial / Realm.Independent. 정의에만 있고 상태(FTranscendentState)에는 없다 (§4.2). Data.Integrity: Allegiance ∈ Realm.*

UPROPERTY(EditAnywhere) FGameplayTag CounterfeitOf; // 모방의 원리 (§4.2): 이 교리가 흉내 내는 참된 것. Truth.Rest / Truth.Union / Truth.Worship / Truth.Renewal. Abyssal 행은 필수(스키마 — 뿌리 Truth., B4). trans_silence = Truth.Rest(Path.Stillness가 아니다 — 교리와 길의 낱말을 섞지 않는다). 읽는 코드는 없다(글의 기준)

UPROPERTY(EditAnywhere) FText Title; // 호칭(이름 아님). 로그 슬롯 {Transcendent}, 성계 패널의 표식 말(한 줄과 툴팁), 두 책 2부의 묶음 머리 '{Transcendent}의 말'(ST_CA Chronicle.ClaimedBy — CosmicChronicle::ClaimantTitleOf) (§8.2, §8.4). 스키마가 비지 않기를 요구한다(B5 `[B-5d1]` — 1인칭 줄이 제 화자의 머리 없이 다른 주장자의 머리 아래 이어지지 않게). 예: 꺼뜨리는 이

UPROPERTY(EditAnywhere) FText Lure; // 필멸자에게 주는 것 — 교리 표 '유혹' 열 (§4.2). 텍스트만 — 읽는 코드가 없다(B4가 trans_silence에 채웠다). 슬롯·숫자 없음, 낱말은 ClaimTemplates와 같은 1인칭 칸의 낱말 식(스키마 — 아래)

UPROPERTY(EditAnywhere) EAscensionTier Tier = EAscensionTier::Tier4_Transcendent; // 허용값 T3/T4. Abyssal T5 금지 (Data.Integrity, §4.2 위계)
UPROPERTY(EditAnywhere) float Depth = 0.5f; // 칸 안의 깊이 0~1 (§3.2 심도). 영압의 초월자 항에 (1 + Depth)를 곱한다(§4.2) — 코드는 0~1로 잘라 읽고 범위는 Data.Integrity가 본다. 정의 데이터라 저장·해시에 들지 않는다. 시작값: 침묵 1.0 / 회귀 0.7 / 광신 0.6 / 동화 0.5 (Phase 5 조정). 숫자가 아닌 값은 0으로 읽는다. Celestial 행은 인스턴스가 없어 압력에 들지 않는다. Independent 행은 로스터가 인스턴스를 만들지 않는다 — 독립 초월자의 인스턴스(Phase 6+)는 칸의 가중치 그대로 누르므로(§4.2 '격은 격이다') 그 행을 처음 쓸 때 (1 + Depth)를 `Realm.Abyssal`에만 곱하게 하거나 린트로 Depth 0을 요구한다. Phase 2

UPROPERTY(EditAnywhere) int32 InitialInfluencedSystems = 0; // 0 = 초기 코어 없음, 1 = 초기 코어 한 성계 — NewGalaxy가 §5.7의 규칙으로 자리를 고른다. 0~1 (Data.Integrity. 둘째 코어는 아직 없다)

UPROPERTY(EditAnywhere) int32 MaxInfluencedSystems = 12; // Hazard.Claim 발생 상한: InfluencedSystems.Num() >= 이 값이면 새 주장 없음 (§4.2). 개체당 판 전체의 합이다 — 은하가 늘어도 같다. Celestial 행은 0

UPROPERTY(EditAnywhere) float SpawnWeight = 1.f; // NewGalaxy 타락 로스터 가중 추첨 (침묵 고정, 동화 2 / 광신 3 / 회귀 2). Celestial·Independent 행은 0. 0 이하이거나 유한하지 않은 값(NaN·무한대)의 행은 후보가 아니다 (§4.2, §7.4)

UPROPERTY(EditAnywhere) TMap<FGameplayTag, float> EventWeightModifiers; // Event.* 잎 태그 -> 그 Category와 정확히 같은 Lottery 행의 가중 인수(부모 태그로 맞추지 않는다). 단위의 성계가 영향권 안이거나 1레인 인접일 때만 건다(§16 #55 ②). 초월자 하나의 인수 = min(3, 값 × (1 + DT_Rules.SphereEventWeightStep × InfluencedSystems.Num())) — 상한 3은 이 인수 전체에 걸고, 인수는 0 아래로 내려가지 않는다(값이나 노브가 음수여도, 숫자가 아니어도 0으로 읽는다). 여러 초월자는 Transcendents 배열 순서로 곱한다. 봉인된 초월자는 걸지 않는다 (§4.2 영혼 경제 '사용'). 어느 행도 Event.Crisis.Claim을 싣지 않는다(스키마의 contains_none, B4 — 철야의 추첨 행 event_claim_eased의 곱이 1이어야 신정론의 몫이 선다. 다른 범주에 실린 무게는 Data.Integrity의 신정론 불변식이 위쪽 끝으로 센다 — §7.3 정의 스키마의 `rules` 항목)

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> PressureEffectsPerTick; // 영압 효과 ①: 침묵 = 수입 감소, 동화 = 윤리 이동, 광신 = 관계 변동, 회귀 = 인구 손실 완화·Insight 감소·Transcendence 0 (§4.2 교리 표 '사용 방식'). 틱당 원천이라 양을 고치는 효과 일곱만 담는다 — 그 밖은 `Apply`가 돌려보내고 Data.Integrity가 행을 막는다(§7.5). 적용의 주제는 그 문명과 영향권 안의 첫 정착된 성계(OwnedSystems 순서), 상대(OtherID)와 SourceRow는 이 초월자다. Celestial 행은 비어 있다

UPROPERTY(EditAnywhere) TArray<FText> VoiceTemplates; // 1인칭 화자 템플릿 (Realm.Abyssal만 1인칭, §8.2). trans_covenant는 듣는 문명의 RevelationTemplates로 말한다 (§5.11). 읽는 코드가 아직 없어 모든 행에서 비어 있다(스키마 max_len 0, B4) — 알아챔의 줄과 함께 읽는 코드가 온다(B6, docs/50_ROADMAP.md)

UPROPERTY(EditAnywhere) TArray<FText> ClaimTemplates; // 주장의 1인칭 줄(B4 [B-4d1]부터 읽는다): 주장이 만료해 그 성계가 그 초월자의 영향권에 편입된 틱(위기 단계 ⓑ의 bJoined)에, 서술자의 만료 줄 뒤에 UCosmicLogSubsystem::PushWithSign이 징조 줄 log_sign_claim(보는 쪽 Tier 2 이하 — 셋째 칸 아래의 눈)을 먼저, 그다음 이 칸의 줄을 낸다 — Category Log.Claim, Severity 1, MinViewerTier 3(셋째 칸 아래의 판에서는 말하지 않은 채 쌓인다), 초점은 그 성계와 그 주인, SourceRow = 그 초월자의 TranscendentID(§7.4 '주장의 목소리'). 비면 줄도 추첨도 없다. 슬롯은 {System}만, 숫자 없음, 낱말은 1인칭 칸의 낱말 식(스키마 — forbidden 배열 verdict_without_soul(판정 어휘에서 '영혼'만 뺀 여섯)·holy_words·holy_and_myth·realm_names·rescue_words·revelation_mimicry는 Lure와 같은 배열이다. 그 위에 행 규칙 lacks_words로 타락 행은 2인칭 second_person을, 침묵의 행은 silence_words를 — Lure도 — 품지 않는다; 글의 규칙은 docs/70 §8.3). 둘째 문장은 남은 자 문장 (Law.Remnant). 데이터: trans_silence 셋

};

// DT_LogTemplates (§5.11 계시 양식, §8.2). 이벤트가 아닌 틱 로그(경제·연구·소유권 변동)와 방문·사자 템플릿. 만남의 줄(log_first_contact·log_contact), 먼 줄(log_far_ascent·log_far_event), 무주 성계 스캔(log_scan_wild — B1), 방랑자 고리의 줄(즉위 log_crowned·복귀 log_returned·양위 log_throne_vacated — Log.Calling 등급 2, 왕좌를 말하지 않는 반란 만료 log_rebellion_unseated — B1, `CosmicRows::` 상수와 스키마의 required_rows), 칙령의 줄(제정 log_edict_enacted·철회 log_edict_revoked·그침 log_edict_lapsed — B2: Log.Civ 등급 1, 다스리는 백성의 것만 — AI의 철회·그침은 말하지 않는다. 슬롯 {Civ}·{Edict}(칙령의 DisplayName — 이름은 SourceRow가 아니라 줄의 인자로 넘긴다). 연대기는 받지 않는다 — CosmicChronicle::Admits, §7.5), 외교와 분쟁의 줄 열하나(B3 — 헤더 B3 3.4절: 다스리는 백성이 당사자일 때만 내고 주제 백성은 늘 다스리는 백성, 상대는 인자 {OtherCiv} — AI끼리의 분쟁은 말하지 않는다. 쪽마다 다른 행이다. Log.Civ 등급 1: 태세 log_stance_changed({Stance})·우호의 그침 log_stance_lapsed — 이 둘은 연대기가 받지 않는다 —, 조약 log_treaty_signed·log_treaty_broken({Treaty}). Log.Conflict({System}): 명령이 연 공성 log_siege_begun과 문을 잃은 공성 log_siege_lifted는 등급 1, 다스리는 백성을 에워쌈 log_siege_on_us·차지함 log_siege_taken·잃음 log_siege_lost·지켜 냄 log_siege_held·서지 못함 log_siege_failed는 등급 2 — 자동 정지와 연대기. 줄은 상대를 먼저 만난 뒤 낸다(NoteMet). {Stance}·{Treaty}는 태그의 표시 이름 — CosmicDiplomacy::StanceNameOf·TreatyNameOf, ST_CA Stance.*·Treaty.*. 글은 holy_words도 품지 않는다 — 스키마), 주장과 함대의 줄 둘(B4 — 헤더 B4 3.6절, `CosmicRows::LogSignClaim`·`LogFleetWorn`과 스키마의 required_rows: 주장이 영향권에 편입된 틱의 징조 log_sign_claim(Log.Sign 등급 1, 슬롯 {System}·{Civ} — 코드가 이름으로 부르는 징조라 이벤트 행의 SignRow 규칙 밖이다. 그 줄의 SourceRow는 1인칭 줄과 같은 초월자 행, §7.3 ClaimTemplates)와 다스리는 백성의 함대가 상위 영향권 안에서 그 목표의 첫 분쟁 단계에 닳은 줄 log_fleet_worn(Log.Conflict 등급 1, {Civ}·{System} — 밤 지킴의 그림 vigil_image 없음, 스키마). 둘 다 MinTierToReveal 0), 부름과 두 책의 줄 다섯(B5 `[B-5d1]` — 헤더 B5 §3.3, `CosmicRows::LogDeathHazard`·`LogInauguration`·`LogInaugurationUnnamed`·`LogRecordDestroyed`·`LogRecordAnnexed`와 스키마의 required_rows; 모두 MinTierToReveal 0, 변형 셋, 서술자): 죽음 줄 log_death_hazard(Log.Calling 등급 2, {Player}·{Hazard} — 사인 Death.Hazard의 행; 사인마다 한 행이고 제 행이 없는 사인은 줄이 없다), 취임 줄 log_inauguration(Log.Calling 등급 2, {Player}·{Civ}·{System})과 그 둘째 행 log_inauguration_unnamed({Player}·{Civ} — 성계를 부르지 않는다: 그 틱 1인칭이 부른 성계에 선 새 그릇, docs/70 §8.3 사후 규칙 ⑤), 기록 줄 둘 log_record_destroyed·log_record_annexed(Log.Record 등급 1, {System}만 — 정착된 성계의 파괴와 주장의 편입, 결정 11-가). 로그는 {Player}를 스스로 풀지 않아 호출자가 그릇의 이름(CosmicChronicle::VesselNameOf)을 인자로 넘긴다. 스키마의 행 규칙: 범주가 Log.Record인 모든 행은 {System}만·숫자 없음·사후·생명책·부름·남은 자·기록의 목적어(날·이름·사람·죽은 이)의 낱말 없음; 죽음 행은 Log.Calling·{Player}·{Hazard}만과 사후·생명책·부름·'당신'·두 영역의 이름·죽음의 틀(위에서 내리는 행위자·희생·까닭)의 낱말 없음; 취임 행 둘은 Log.Calling·제 슬롯만과 사후·생명책·부름·'당신'·두 영역의 이름·돌아옴과 다시 남의 낱말 없음 — 목록은 아래 정의 스키마의 words), 건넘의 줄(log_crossing_depart·log_crossing_arrive·log_crossing_turned·log_crossing_word, Phase 6a)도 이 테이블의 행이다 (§5.7, §8.1). 행은 두 가지이고 Context가 가른다: Context가 빈 행은 **이름 행**이다 — PushRow가 행 이름으로 부르고 선택기는 보지 않는다(Phase 1의 다섯 행과 v0.7의 열 행이 모두 이것이다). Context가 Revelation.*인 행은 **선택 행**이다 — (Category, Context)마다 하나 이하(Data.Integrity). 선택기 UCosmicLogSubsystem::PickTemplateRow(Category, Context, FallbackRow)는 일치하는 선택 행의 이름을, 없으면 호출자가 넘긴 이름 행을 돌려준다. 폴백은 '빈 Context 행'이 아니라 호출자가 CosmicRows 상수로 넘기는 이름 행이다. 뽑지 않는다 — 변형 추첨은 그 이름으로 부르는 PushRow가 한다 (§9.2)

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

UPROPERTY(EditAnywhere) int32 BirthProtectionTicks = 301; // 출생 보호: Tick − BornTick이 이 값 미만이면 치명 효과를 무시한다(CosmicHazards::VesselAtRiskOf). 300이 아니라 301 — 첫 위기는 틱 300에 모든 수도에서 만료하고 그 틱은 첫 그릇의 보호 안이다(B5 `[B-5d2]` — crisis_and_transcendents.md §4.4 금지 목록의 '그 틱 포함'; 그 행이 죽이는 동안 Data.Integrity가 보호를 첫 위기의 만료와 견준다. C++ 기본값은 `[B-5e]`부터 같은 수)

UPROPERTY(EditAnywhere) int32 BaseLifespanTicks = 0; // 0 = 노화 OFF (VS). DT_Traits.LifespanTicksMod와 합산

UPROPERTY(EditAnywhere) float SuccessionHazardWeight = 2.f; // 예약 — B5는 이 노브와 아래 넷을 읽지 않는다(결정 23-가: 후보는 모두 같은 무게). 켜는 쪽이면: 2레인 안에 위기가 서 있는가(수가 아니라 있음)

UPROPERTY(EditAnywhere) float SuccessionCivTierWeight = 1.f; // 소유 문명 Tier >= 2

UPROPERTY(EditAnywhere) float SuccessionReverenceWeight = 2.f; // 소유 문명 Reverence > SuccessionReverenceThreshold

UPROPERTY(EditAnywhere) float SuccessionReverenceThreshold = 0.3f; // 위 가중치의 Reverence 임계

UPROPERTY(EditAnywhere) float SuccessionSameCivPenalty = 1.f; // 직전 생애와 같은 문명 감점

UPROPERTY(EditAnywhere) int32 SuccessionMinHazardTicks = 300; // 계승 후보에서 뺀다: 서 있는 위기의 남은 카운트다운이 이 값 미만인 성계 — 후보가 비면 먼저 풀리는 거름(CosmicSuccession::CandidatesOf, B5)

UPROPERTY(EditAnywhere) float ReverenceDecay = 0.5f; // 폐위 시 모든 백성의 Reverence 배율. 양위는 줄이지 않는다(B1). 죽음은 이 노브를 읽지 않고 모든 백성의 Reverence를 0으로 둔다(B5 결정 23-나)

UPROPERTY(EditAnywhere) float AIAscensionSpeedMultiplier = 1.f; // §4.3 난이도

UPROPERTY(EditAnywhere) float PostRebellionStability = 0.15f; // 폐위 직후 Stability 상한 (§5.10)

UPROPERTY(EditAnywhere) int32 ThroneLockTicks = 600; // 왕좌 잠금: 그릇이 왕좌를 떠나는 틱(폐위·양위, 통치자의 죽음 — B5)에 모든 백성의 ThroneLockedUntilTick을 Tick + 이 값 이상으로 올린다(B1 — 전에는 옛 백성만). 0 아래는 0으로 읽는다. B4([B-4d2]): 카드가 어느 위기의 시간을 사면(DelayHazard) 이 값은 그 위기의 가장 긴 머묾 이상이어야 한다(Data.Integrity의 신정론 불변식 — 지금 주장 550 ≤ 600): 새 통치가 서기 전에 그 주장이 끝나, 한 주장에 함대의 시간이 두 번 붙지 않는다(죽음·계승도 이 잠금을 지난다 — 통치자의 죽음이 잠그고 계승은 늘 방랑자다, B5; 헤더 B4 11절 항목 18)

UPROPERTY(EditAnywhere) TArray<float> PressureBySourceTier = {0.f, 0.f, 0.25f, 1.f, 2.f}; // 인덱스 = Tier 번호 − 1 = static_cast<int32>(EAscensionTier). 정확히 5개 (§4.2, Data.Integrity)

UPROPERTY(EditAnywhere) float PressureAdjacencyFalloff = 0.5f; // 1레인 인접

UPROPERTY(EditAnywhere) float SpiritualPressureStabilityDrain = 0.002f; // 틱당 × P

UPROPERTY(EditAnywhere) float PressureFleetAttrition = 0.01f; // 영향권 안의 소모(B3부터 읽는다 — 분쟁 단계): 목표 성계가 있는 백성의 FleetPower에서 틱마다 이 값 × P를 뺀다. P = UGalaxySimulationSubsystem::ComputeAttritionPressureIn(그 백성, 목표 성계) — 그 백성보다 높은 칸의 봉인되지 않은 초월자가 그 성계를 영향권 안에 둘 때의 그 항만(§7.4). 스키마 0 이상, 0 이하·유한하지 않으면 소모 없음

UPROPERTY(EditAnywhere) TArray<float> FleetEffectivenessByTierGap = {1.f, 0.5f, 0.f}; // 인덱스 = 격차(0, 1, >=2). 정확히 3개 (§5.6, Data.Integrity). B3부터 읽는다(CosmicConflict::EffectivePowerOf): 공격하는 쪽의 유효 전력(격차 = 지키는 주인이 공격하는 쪽보다 높은 칸 수, 아래로는 0 — 지키는 쪽은 제 성계에서 곱을 받지 않는다)과 지킴의 지연(주장은 그 주장자의 칸과의 격차, 그 밖의 위기는 0). 곱이 0 이상·유한이 아니면 0

UPROPERTY(EditAnywhere) float FleetDelayPerPower = 50.f; // 지킴의 지연(B3부터 읽는다 — 놓일 때만): 주인의 함대 목표가 그 성계일 때 새로 놓이는 위기의 카운트다운에 이 값 × 유효 전력 틱을 더한다(CosmicConflict::FleetDelayOf·GuardDelayTicksOf — 수도 규칙의 행(반란)은 빼고). 지킴은 서 있는 위기를 늦추지 않는다(함대가 서 있는 위기의 시간을 사는 길은 카드의 DelayHazard 하나 — B4, 아래 ResistFleetFactor). 스키마 0 이상

UPROPERTY(EditAnywhere) float FleetDelayCap = 100.f; // 위 지연의 상한 — 비교로 [0, Min(이 값, 100,000)] 안에 두고 int32로 자른다(음수·수가 아닌 값은 0 — 칙령의 CountdownBonusOf와 같은 자름). B4부터 신정론 불변식이 이 값을 위기의 가장 긴 머묾에 더한다(Data.Integrity — §7.3 정의 스키마의 `rules` 항목): 지킴의 두 노브나 카드의 DelayHazard 값을 올리면 철야의 추첨 행의 무게 상한을 같은 커밋에서 다시 셈한다. 스키마 0 이상

UPROPERTY(EditAnywhere) FGameplayTagContainer ImmutableLaws; // Law.Judgment, Law.Sabbath, Law.Humility, Law.Remnant, Law.Hedge(성역 — A0 4a부터. 스키마 규칙 contains_all이 다섯을 모두 요구한다) (§4.5)

UPROPERTY(EditAnywhere) int32 FallenTranscendentCount = 3; // 판당 활성(미봉인) 타락 초월자 수 — 은하가 늘어도 판 전체의 수다 (§4.2). VS 1(DT_Rules.json의 데이터 값이고 코드 기본값은 3이다 — 둘 다 그대로 둔다), EA 3. 0~3으로 읽는다: 밖의 값은 NewGalaxy가 에러 한 줄을 내고 잘라 읽는다(§4.2 '판당 활성 타락 ≤ 3'). 침묵 고정 + 나머지는 SpawnWeight 가중 추첨 (§4.2, §7.4 NewGalaxy)

UPROPERTY(EditAnywhere) FName PlayerArchetypeRow = TEXT("arch_seeker"); // 플레이어 첫 생애 문명의 DT_Civs 행 = 첫 생애 행 (§5.10, §5.11). Data.Integrity: 행 존재, 스키마 규칙: arch_seeker (A0 4b — C++ 기본값과 DT_Rules 값이 함께 바뀌었다. `CosmicRows::ArchSeeker`)

UPROPERTY(EditAnywhere) int32 ClaimAdjacencyLanes = 1; // 주장 발생 조건 (a): 타락 영향권에서 이 레인 수 이내. (옛 ClaimCountdown은 위기 처리 작업에서 지웠다 — 주장도 제 행의 CountdownTicks로 세고, 놓일 때 주인이 지닌 칙령의 HazardCountdownBonus를 더한다 — B2)

// (옛 VigilClaimExtension 100은 B2가 지웠다 — 주장 카운트다운의 연장은 칙령 행 edict_vigil(설계 이름 철야·금식, 표시 이름 '밤샘·절식')의 HazardCountdownBonus(Hazard.Claim +100)가 한다. 한 값이 두 곳에 있지 않게)

UPROPERTY(EditAnywhere) float SphereEventWeightStep = 0.1f; // 영향권 1성계당 교리 이벤트 가중 승수 증가분. 승수 상한 3 (§4.2 '사용')

UPROPERTY(EditAnywhere) int32 EventSlotTicks = 10; // 정착된 성계를 가진 문명마다 이 틱 수에 한 번 이벤트 슬롯 (§7.5). 1~20 (Data.Integrity — 20을 넘으면 반란 전의 웅성거림 두 줄이 보장되지 않는다, §4.1. 코드는 1 미만을 1로 읽는다). Phase 2

UPROPERTY(EditAnywhere) float EventChancePerSlot = 0.02f; // BaseWeight 1인 Lottery 행이 자격 있는 단위에 대해 슬롯당 발화할 확률. 잠정 시작값 — 한 백성에게 일어나는 일의 총량은 자격 있는 Lottery 행·단위의 수에 비례하므로 행 묶음이 들어올 때마다 다시 맞춘다(§16 #54). 0 이상(Data.Integrity. 코드는 0 미만과 수가 아닌 값을 0으로 읽는다). Phase 2

UPROPERTY(EditAnywhere) int32 ReturnLureMaxLanes = 2; // 귀환 제안 게이트: 사망 성계가 회귀 영향권에서 이 레인 수 이내 (§4.4 회귀의 유혹)

UPROPERTY(EditAnywhere) int32 ReturnedLivesToSeize = 3; // Succession.Returned 생애 수가 이 값에 이르면 GrantPower Way.Seized (Law.Humility로 영구)

UPROPERTY(EditAnywhere) TMap<FName, float> LureAcceptByPolicy = {{TEXT("Cautious"), 0.10f}, {TEXT("Reckless"), 0.60f}, {TEXT("Faithful"), 0.f}, {TEXT("Tribute"), 0.85f}}; // 커맨드렛 -PlayerPolicy별 귀환 제안 수락 확률 (§9.3) — 읽는 코드가 아직 없다(Phase 5): B4의 -PlayerPolicy는 영압 카드에만 답하고 Tribute를 거절한다

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

UPROPERTY(EditAnywhere) int32 SceneryGalaxyCount = 10000; // 풍경 은하의 점 수. 표현 계층만 읽는다(CosmicUniverseLook::SceneryCountOf — 0~50,000으로 읽는다, 스키마도 같은 경계) (§5.11, §6.4 '우주 화면'). Phase 3 9번 작업

UPROPERTY(EditAnywhere) int32 PetitionPeriodTicks = 300; // 청원의 주기 슬롯(MVP ②, §5.10 — 600에서 MVP ③이 300으로): Tick − Calling.BornTick이 이 값의 양의 배수인 틱에, 그 틱에 막 선 Standing 행이 없으면 앎 단계가 Lottery 행 하나를 내놓는다(생애의 LastChoiceRows에 든 행은 빼고, 그러면 후보가 없으면 그 가운데 가장 최근의 Lottery 행 하나만 뺀다 — MVP ⑧ A-8b, CosmicPetitions::Present. 그 셈은 통치자의 청원만 센다 — B1). 1 아래는 1로 읽는다(스키마는 1 이상)
// ---- MVP ③(2026-10-22, 헤더 docs/60_DECISIONS/headers/A3_first_ten_minutes.md) — 구조체의 끝에 다섯 ----
UPROPERTY(EditAnywhere) float StabilityRecoveryPerTick = 0.001f; // 안정도 회복(§4.1, §16 #68): 경제 단계에서 인구 회복 바로 뒤(수입 앞), StabilityRestingLevel 아래의 백성은 이만큼 × (1 + SocietyRecoveryBonus × 사회 연구 0~1, B2) 오르고 그 수준에서 멈춘다. 위는 그대로(카드·이벤트가 올린 몫은 남는다). 같은 틱의 LowStabilityTicks 셈에 든다. 0 이하·1 초과·숫자가 아니면 회복 없음. 시작값 — 영향권 곁의 소모(0.002 × P)보다 작게(0.002면 주장 정착률이 22%로 떨어진다 — 헤더 A3 §1)
UPROPERTY(EditAnywhere) float StabilityRestingLevel = 0.6f; // 회복이 멈추는 수준(§9.3 '자연 수렴 0.6'). 0~1 밖이거나 숫자가 아니면 회복 없음
UPROPERTY(EditAnywhere) FGameplayTagContainer ClaimLeaveProfiles; // AI 구호의 정책(§16 #71): AI 단계에서 이 프로필의 백성은 제 성계에 선 Hazard.Claim의 답을 시작하지 않는다. 다른 답과 다른 프로필(빈 프로필 포함)은 그대로. 통치자의 프로젝트 키는 읽지 않는다. 비면 모두 구호한다. 추첨 없음 — 프로필은 생성과 왕좌 교체가 이미 뽑았다. 데이터: AI.Profile.Expansionist, AI.Profile.Militarist
UPROPERTY(EditAnywhere) int32 FirstCrisisTick = 150; // 첫 위기(§2.4 첫 10분, §16 #91): Tick == 이 값인 틱의 위기 단계 발생 끝(추첨과 수도 규칙 뒤, 이벤트 슬롯 앞)에 Civs 배열 순서로 모든 백성의 수도(소유 ∧ 정착)에 FirstCrisisHazard의 Draw 행을 CosmicHazards::Place로 놓는다. 슬롯이 찬 수도는 그 위기를 지킨다(그것이 첫 위기다). 모든 백성 같은 규칙. 0 이하 = 없음. 추첨 없음
UPROPERTY(EditAnywhere) FGameplayTag FirstCrisisHazard; // DT_Hazards의 Draw 행의 태그(Data.Integrity). 다른 행의 태그는 에러 한 줄과 함께 거절, 그 행이 없는 테이블(테스트)은 아무것도 놓지 않는다. 비면 없음(C++ 기본값). 데이터: Hazard.AIAwakening
UPROPERTY(EditAnywhere) int32 ProvisionalEndTick = 6000; // MVP ④(2026-10-23, 헤더 docs/60_DECISIONS/headers/A4_provisional_end.md) 판의 시계 끝(CosmicEndings::EndingOf의 Ending.Clock): Tick이 이 값에 이른 틱이, 아직 끝이 없고 차례에서 시계 앞의 끝이 서지 않았으면 열째 단계 뒤에 기록 State.Ending을 적고, 그 틱 경계부터 표현 계층이 시간을 멈춘 채 두고 연대기를 연다 — 판의 마지막 틱이 이 틱이다. 0 이하 = 시계 끝 없음. 시뮬은 끝을 적되 여기서 멈추지 않는다(빨리 넘기기 RunLeap만 기록의 틱에서 선다; 커맨드렛은 알리기만 한다). 시작값 6,000 = ×10에서 쉬지 않고 10분, 시계로 끝나는 판만 늘린다. 상단 바의 시계 툴팁도 읽는다. 네 엔딩(B5·B6)과 함께 간다
UPROPERTY(EditAnywhere) int32 ChronicleCap = 1500; // MVP ⑤(A-5b, 헤더 docs/60_DECISIONS/headers/A5_save_minimum.md §6) 저장된 연대기의 상한(#40 ⑦): 로그에서 옮긴 뒤 이 수를 넘으면 가장 오래된 Severity 1부터 밀어낸다 — S2·Log.Calling·Log.Claim·Log.Record는 밀어내지 않아 넘을 수 있다. FGalaxyState.ChronicleEviction이 적고 장은 Chronicle.More를 단다. 0 이하 = 상한 없음. 이야기 층만 읽고 세계 해시는 움직이지 않는다. 표시 상한(CosmicChronicle::MaxLifeLines)이 아니다
UPROPERTY(EditAnywhere) float ResearchFocusShare = 0.55f; // MVP ⑦(2026-10-24, 헤더 docs/60_DECISIONS/headers/A7_research_focus.md) 통치자의 연구 중점이 그 분야에 주는 몫 — 나머지 분야는 남은 몫을 고루(CosmicResearch::Focus). 스키마 0.3..1. 고른 몫(1/4) 이하·1 초과·숫자 아님이면 SetResearchFocus가 분야를 에러 한 줄과 함께 거절한다(균형은 읽지 않는다). 시작 값 0.55 = 첫 청원 '지혜로운 마음'의 몫
// ---- MVP ⑧(2026-10-24, 헤더 docs/60_DECISIONS/headers/A8_economy_and_pace.md) 경제·연구의 노브 여덟 — 코드의 상수에서 옮겼다(StarClassEnergy만 단계 5까지 C++). 경제·연구 단계가 읽는다(§7.4) ----
UPROPERTY(EditAnywhere) TArray<float> TierMultiplier = { 1.0f, 1.5f, 2.2f, 3.0f, 3.0f }; // 칸(Tier1..5)의 수입 배수 — 에너지·물질에. 배열에 없는 칸·0 미만·NaN은 1로 읽는다(0은 0 — 테스트가 수입을 잠근다). 스키마: 다섯, 0 이상
UPROPERTY(EditAnywhere) float MaterialistEnergyBonus = 0.2f; // 물질 쪽 윤리(Spiritualism < 0) 1점당 에너지 보너스. 0 이하·NaN은 보너스 없음. 스키마 0 이상
UPROPERTY(EditAnywhere) float InsightPerPopulation = 1.0f; // 인구 1당 통찰(윤리 보너스 전, 칸 배수 없음). 0 이하·NaN은 0. 스키마 0 이상
UPROPERTY(EditAnywhere) float SpiritualistInsightBonus = 0.2f; // 영성 쪽 윤리 1점당 통찰 보너스. 0 이하·NaN은 보너스 없음
UPROPERTY(EditAnywhere) float InfluencePerPopulation = 1.0f; // 인구 1당 영향력(윤리 보너스 전, 칸 배수 없음). 0 이하·NaN은 0. 0.5에서 MVP ⑧ A-8b가 1.0으로(2026-10-18 결정 10 — 구호·개혁·청원의 값이 겹쳐 영향력에 굶던 일)
UPROPERTY(EditAnywhere) float OpenInfluenceBonus = 0.2f; // 개방 쪽 윤리 1점당 영향력 보너스. 0 이하·NaN은 보너스 없음
UPROPERTY(EditAnywhere) float ResearchInsightShare = 0.5f; // 연구 단계가 쓰는 통찰 수입의 몫(비축 한도 안). 0..1로 읽는다(NaN은 0). 스키마 0..1
UPROPERTY(EditAnywhere) float ResearchPerInsight = 0.003f; // 쓴 통찰 1당 연구 진척(분야의 몫을 곱하기 전). 0 이하·NaN은 진척 없음. 스키마 0 이상
// ---- B1(2026-10-25, 헤더 docs/60_DECISIONS/headers/B1_wanderer_loop.md 2.2절) 방랑자 고리의 노브 여섯. 정의라 저장되지 않는다 ----
UPROPERTY(EditAnywhere) int32 LaneTravelTicks = 20; // 레인 하나를 지나는 틱(TravelTo — §5.9 '레인당 20'). 1 아래는 1로 읽는다(CosmicWanderer::LaneTicksOf — 두 이동 필드는 늘 함께 서고 함께 빈다). 스키마 1 이상·정수. 거리에 따른 시간은 실제 하늘과 함께(B8, §16 #83)
UPROPERTY(EditAnywhere) int32 ScanSightBonusLanes = 1; // 스캔(ScanCurrentSystem)의 보는 거리 = SightLanesByTier[PersonalTier](없는 칸·1 아래는 1) + 이 값(0 아래는 0) (§4.4, §5.9). 스키마 0 이상·정수
UPROPERTY(EditAnywhere) float EncounterChancePerArrival = 0.25f; // 도착한 틱에 도착 카드(방랑자의 Lottery 행)가 올 몫: 도착의 파생 정수(CosmicSeed::Purpose::Encounter 11 — 이름은 그릇이 선 SystemID, Index는 틱) / 2^32가 이 값 아래일 때(CosmicPetitions::Present). 0~1로 잘라 읽고 숫자가 아니면 0. 스키마 0~1
UPROPERTY(EditAnywhere) float CoronationReverence = 0.6f; // 즉위의 경외 길: 그 백성의 ReverenceToPlayer가 이 값 이상(CosmicWanderer::CrownBlockOf — 노브나 경외가 숫자가 아니면 경외 길이 열리지 않는다; 지도자 공백 길은 따로 본다). 스키마 0~1
UPROPERTY(EditAnywhere) float CoronationStabilityBelow = 0.2f; // 즉위의 지도자 공백 길: 그 백성의 Stability가 이 값 미만. 스키마 0~1. 복귀(옛 백성)는 두 길을 보지 않고 잠금만 본다
UPROPERTY(EditAnywhere) float AbdicationStabilityLoss = 0.1f; // 양위(AbdicateRuler)가 떠나는 백성의 Stability에서 덜어 내는 값 — 0 아래로 가지 않는다. 0 이하·숫자 아님은 0, 1 위는 1로 읽는다. 스키마 0~1
// ---- B2(2026-10-25, 헤더 docs/60_DECISIONS/headers/B2_edicts_and_research.md 2.7절·3.3절) 칙령과 연구의 나머지의 노브 넷 — 지금 구조체의 끝. 정의라 저장되지 않는다. 장치 커밋([B-2a])은 세계를 옮기지 않는 값으로 넣고 밸런스 커밋([B-2c])이 데이터 값을 켰다 ----
UPROPERTY(EditAnywhere) FName StartingEdict; // 물려받은 칙령: NewGalaxy가 모든 백성의 ActiveEdicts에 이 DT_Edicts 행을 넣는다(첫 10분은 모든 백성 공통 규칙 — §7.4 NewGalaxy). 비거나 테이블에 없는 행 = 없음. 추첨·줄·제정의 값 없음. C++ 기본값은 빈 값, 데이터 edict_mobilization(전시 동원). 스키마: DT_Edicts의 행이거나 빈 값
UPROPERTY(EditAnywhere) TArray<float> EdictSlotSocietyMarks = { 0.5f, 1.f }; // 칙령 슬롯 = 1 + 그 백성의 사회 연구(Research["Society"], 없는 키는 0)가 닿은(≥) 문턱의 수(CosmicEdicts::SlotsOf) — 처음 1, 사회 0.5에 2, 1.0에 3. 수가 아닌 문턱은 닿지 않는다. 스키마 0~1·오름차순. AI도 같은 규칙이다(B2의 AI는 칙령을 지니지 않는다)
UPROPERTY(EditAnywhere) float SocietyRecoveryBonus = 0.f; // 사회 연구의 회복(§7.4 경제 단계): 안정도 회복 = StabilityRecoveryPerTick × (1 + 이 값 × 사회 연구(0~1로 읽는다)). 0보다 크고 유한할 때만 읽고 그 밖(0·음수·숫자 아님)은 0 — 그때 회복은 노브 그대로 비트까지 같다. C++ 기본값 0, 데이터 0.25(최대 회복 0.00125). Data.Integrity가 최대 회복을 물려받은 칙령의 순 안정도 소모 아래로 지킨다(CosmicEdicts::InheritedDrainOutrunsRecovery — 방치한 백성이 사회 1.0에서도 무너지게). 스키마 0~1
UPROPERTY(EditAnywhere) float AIEdictRevokeStability = 1.f; // AI 단계가 안정도 값이 있는 칙령(CosmicEdicts::HasStabilityCost)을 철회하는 문턱 — 그 백성의 Stability가 이 값 아래일 때. 그 밖의 칙령은 곧 철회한다: B2의 AI는 칙령을 지니지 않는다(AI의 칙령은 B7). 1 = 첫 AI 단계에서(물려받은 칙령은 AI에게 첫 틱의 경제 단계에만 걸린다). 숫자가 아니면 그 종류는 철회하지 않는다. C++ 기본값·데이터 1. 스키마 0~1
// ---- B3(2026-10-26 — 스물둘은 [B-3b]가 장치 값으로, 다스리는 백성을 향한 끝의 둘은 [B-3d2]가 더했고 밸런스 값은 [B-3d2]. 헤더 docs/60_DECISIONS/headers/B3_diplomacy_and_conflict.md 3.1절·11절 항목 12) 외교와 분쟁의 노브 스물넷 — 지금 구조체의 끝. 정의라 저장되지 않는다. C++ 기본값은 장치 값(외교·분쟁을 끈 값 — 표류 0·함대 0·번짐 1)이고 DT_Rules.json의 밸런스 값이 켠다([B-3d2]) — DT_Rules 행이 없는 판과 FCosmicRulesDef를 기본으로 만드는 테스트는 끈 채로 돈다. 수가 아닌 값은 읽는 자리에서 끈 값으로 읽는다 ----
UPROPERTY(EditAnywhere) float RelationDriftRate = 0.f; // 관계의 표류(외교 단계): 국경이 닿은 쌍의 R ← R + 이 값 × (목표 − R), [−1, 1]. (0, 1] 밖은 0 — 0이면 표류도 항목도 없다. 데이터 0.002(목표까지 남은 거리의 몫/틱 — 약 500틱의 시간 상수). 스키마 0~1
UPROPERTY(EditAnywhere) float RelationEthicsWeight = 0.6f; // 목표값의 윤리 항: −이 값 × 두 백성의 윤리 거리(CosmicDiplomacy::EthicsDistanceOf — 세 축 차이의 절댓값 합 ÷ 3, 0~2). 데이터 0.6. 스키마 0~1
UPROPERTY(EditAnywhere) TArray<float> RelationStanceTerms = { 0.4f, -0.4f }; // 목표값의 태세 항 — 그 상대가 이 백성을 대하는 태세: [0] 우호, [1] 적대(중립은 항 없음). 정확히 2개(Data.Integrity). 데이터 그대로. 스키마 −1~1
UPROPERTY(EditAnywhere) float RelationTreatyTerm = 0.2f; // 목표값의 조약 항: 두 백성이 맺은 조약 하나마다. 스키마 0~1
UPROPERTY(EditAnywhere) float ConquestRelationPenalty = 0.2f; // 정복한 백성을 보는 눈이 곧 이만큼 낮아진다 — 넘김 직후 정복한 백성과 국경이 닿은 모든 백성과 진 백성(CosmicDiplomacy::LowerRelation). 스키마 0~2
UPROPERTY(EditAnywhere) float BetrayalRelationPenalty = 0.3f; // 통치자가 조약을 끊거나(BreakTreaty) 무역 상대에게 적대를 걸면(SetStance — CosmicDiplomacy::TurnHostile) 그 상대가 다스리는 백성을 보는 눈이 곧 이만큼 낮아진다. AI가 적대를 열 때와 효과 SetStance는 값 없이 끊는다. 스키마 0~2
UPROPERTY(EditAnywhere) float FriendlyUpkeepInfluence = 0.2f; // 우호 하나의 틱당 영향력(외교 단계, 비축에서) — 내지 못한 우호는 중립으로 그친다. 0보다 크고 유한할 때만 읽는다. 스키마 0 이상
UPROPERTY(EditAnywhere) TArray<int32> FriendlySlotsByTier = { 1, 1, 2, 2, 3 }; // 우호의 자리: 한 백성이 우호를 걸 수 있는 상대의 수(CosmicDiplomacy::FriendlySlotsOf — 인덱스 = static_cast<int32>(EAscensionTier), 배열 밖 = 0, 0 아래는 0). 정확히 5개(Data.Integrity). 영향력이 묶이지 않아 유지비만으로는 '모든 이웃에게 우호'가 공짜가 된다 — 그래서 자리를 둔다(헤더 B3 결정 16). 스키마 0 이상·정수
UPROPERTY(EditAnywhere) float TreatyCostInfluence = 50.f; // 조약 제안의 값 — 받아들여질 때 한 번, 제안하는 쪽의 비축에서(CosmicDiplomacy::TreatyCostOf — 0보다 크고 유한할 때만, 그 밖은 0). 스키마 0 이상
UPROPERTY(EditAnywhere) float TreatyAcceptRelation = -0.1f; // 승낙의 문턱: 그 백성이 제안한 백성을 보는 눈(ViewOf) ≥ 이 값 − 제안하는 백성이 지닌 칙령의 TreatyAcceptBonus 합(CosmicDiplomacy::TreatyBarOf — 수가 아니면 0). 스키마 −1~1
UPROPERTY(EditAnywhere) float TradeInfluencePerTreaty = 0.2f; // 무역 조약 하나의 틱당 영향력 수입(경제 단계의 수입 계산 — 맺은 틱의 다음 틱부터). 0 이하는 0. 스키마 0 이상
UPROPERTY(EditAnywhere) TArray<float> FleetPerSettledByTier = { 0.f, 0.f, 0.f, 0.f, 0.f }; // 함대의 상한 = 이 값[칸] × 그 백성의 정착된 성계 수(CosmicConflict::FleetCapOf — 배열 밖·0 이하·수가 아님 = 0). 정확히 5개(Data.Integrity). 장치 값 0 = 아무도 함대가 없다. 데이터 {0, 1, 1, 1, 1} — 첫째 칸에는 함대가 없다(첫 10분에 공성이 없다). 스키마 0 이상
UPROPERTY(EditAnywhere) float FleetGrowthPerTick = 0.001f; // 유지비를 다 낸 틱의 자람(× 지닌 칙령의 FleetGrowthMultiplier 곱), 못 낸 틱에는 이만큼 준다. 스키마 0 이상
UPROPERTY(EditAnywhere) float FleetUpkeepEnergyPerPower = 0.5f; // 함대 1.0의 틱당 에너지 — 모두 내거나 아무것도 내지 않는다(칙령의 유지비와 같은 꼴). 스키마 0 이상
UPROPERTY(EditAnywhere) int32 SiegeTicks = 200; // 공성의 길이: FleetTargetSinceTick + Max(1, 이 값)에 이르렀거나 지난 틱의 분쟁 단계가 끝낸다(노브가 줄어든 세이브의 공성도 다음 분쟁 단계에서 끝난다). 스키마 1 이상·정수
UPROPERTY(EditAnywhere) int32 SiegeRestTicks = 600; // AI의 쉼: 목표를 푼 틱부터 이만큼 적대를 열지 않는다(0 아래는 0). 통치자에게는 쉼이 없다. 스키마 0 이상·정수
UPROPERTY(EditAnywhere) TArray<float> SiegeDefenceShares = { 1.f, 0.5f, 0.25f }; // 에워싸인 성계를 지키는 몫 — 주인의 FleetPower에 곱한다: [0] 주인의 목표가 그 성계, [1] 목표 없음, [2] 다른 성계. 정확히 3개(Data.Integrity). 스키마 0~1
UPROPERTY(EditAnywhere) float SiegeRepulsedFleetFactor = 0.5f; // 서지 못한 공성의 공격 함대에 곱한다(CosmicConflict::RepulsedShareOf — [0, 1] 밖은 0). 스키마 0~1
UPROPERTY(EditAnywhere) float AIHostileRelation = -0.3f; // AI가 적대를 여는 눈: 그 AI가 국경이 닿은 상대를 보는 저장된 관계 ≤ 이 값(§7.4 TickCivAI ③ — 다스리는 백성을 향해서는 AIHostileRelationTowardRuled). 스키마 −1~1
UPROPERTY(EditAnywhere) float AIHostilePowerRatio = 1.5f; // AI가 적대를 여는 전력의 비: 그 AI의 FleetPower(0보다 크다) ≥ 이 값 × 상대의 FleetPower — AI끼리. 데이터 0.79(1.5에서는 백성 사이의 넘김이 있는 판이 드물었다 — 까닭과 수는 헤더 B3 11절 항목 12와 DECISIONS_LOG). 스키마 0 이상
UPROPERTY(EditAnywhere) float AIHostilePacifismMax = 0.5f; // 평화 축(Ethics.Pacifism)이 이 값 이상인 AI는 적대를 열지 않는다. 스키마 −1~1
UPROPERTY(EditAnywhere) float PlagueSpreadMultiplier = 1.f; // 역병의 번짐(위기 단계 ⓒ): ⓒ의 시작에 레인 이웃에 역병이 서 있는 정착된 성계의 역병 행 몫에 곱한다. 1 = 번짐 없음(표를 만들지 않는다), 0 이상·유한이 아니면 1. 데이터 8. 추첨 수는 그대로. 스키마 0 이상
UPROPERTY(EditAnywhere) float AIHostileRelationTowardRuled = -0.3f; // 다스리는 백성을 향한 AI의 적대의 문만(헤더 B3 결정 21과 11절 항목 12 — 헤더의 노브 하나 대신 둘: 관계의 문과 전력의 문이 함께 막았다. AI끼리는 위 두 노브 그대로): 그 백성을 보는 눈 ≤ 이 값. 장치 값은 AIHostileRelation과 같다. 데이터 0.0. 스키마 −1~1
UPROPERTY(EditAnywhere) float AIHostilePowerRatioTowardRuled = 1.5f; // 같은 문의 전력비: 그 AI의 FleetPower ≥ 이 값 × 다스리는 백성의 FleetPower. 장치 값은 AIHostilePowerRatio의 장치 값과 같다. 데이터 0.6. 스키마 0 이상
UPROPERTY(EditAnywhere) float ResistFleetFactor = 1.f; // B4(헤더 B4 3.1절): 카드의 `DelayHazard`가 서 있는 위기의 시간을 살 때 남는 함대의 몫(`CosmicConflict::ResistShareOf`). 장치 값 1, 데이터 0.5(`[B-4d2]` — 잠정: 0.45와의 짝은 함대를 반으로 줄인 일이 받은 공성에 닿지 않아 가를 수 없었다, 헤더 B4 11절 항목 18). 스키마 0~1, 밖은 1로 읽는다(비교로 — 수가 아닌 값도 1)

// 아직 코드에 없는 노브(끝에 더한다 — 처음 읽는 작업과 함께)

UPROPERTY(EditAnywhere) int32 CrossingMinTier = 4; // 건넘의 최소 칸. 값은 Tier 번호 (§5.7 건넘). Phase 6a

UPROPERTY(EditAnywhere) int32 CrossingCooldownTicks = 4000; // 한 백성의 건넘 사이 간격 (§5.7 건넘). Phase 6a

UPROPERTY(EditAnywhere) int32 CrossingSettleTicks = 3000; // 건너온 백성의 T4→T5 판정 유예 (§5.7 건넘). Phase 6a

};

// DT_Dedications (§8.6). Phase 4: Epithet 10행 최소형 (§5.10 시대 명명, 로그 슬롯 {Epithet}) + 첫 생애 백성의 의회·회중 헌정명 최소형. Phase 6+: 헌정명 뱅크 행 (Bank 태그 Faith.Bank.Hebrew / Civic / Nations … + 트리거 종류 + 이름 FText 묶음). 구조는 뱅크 작성 시 확정. 로그 슬롯 {Dedication}

// DT_Megaprojects (§5.7): FMegaprojectDef : FTableRowBase { DisplayName, RequiredTier(EAscensionTier — 시작할 수 있는 주인의 칸), CostPerTick(FResourcePool — 낸 틱마다 전액. 0 아래이거나 숫자가 아닌 값은 0), RequiredProgress(낸 틱마다 1, AI 백성은 AIAscensionSpeedMultiplier. 1 미만이거나 숫자가 아니면 1 — CosmicMegaprojects::RequiredProgressOf, 패널도 같은 수), CompleteTemplates(완성 줄 — Log.Civ, 슬롯 {Civ} {System} {Project}) }. 옛 스케치의 TargetHazard는 Hazard 행의 RulerResolutions가 대신한다(그 목록에 든 행이 대응 프로젝트). DedicationBank는 DT_Dedications와 함께 Phase 4. 행(거대 프로젝트와 승천 판정 작업): project_dyson(다이슨 스웜, 건설 칸)·project_stabilize(항성 안정화)·project_reform(개혁)·project_relief(구호) — 뒤의 셋은 대응 칸, 값은 잠정이고 Phase 5에 맞춘다. 그리고 project_settle(정착 — 무주 성계를 제 것으로 삼는다. 대상 성계의 건설 칸에서 돌아 업적 프로젝트와 겹치지 않는다. Start는 거절하고 CosmicMegaprojects::Settle로만 시작하며, 진척·완성은 다른 건설 행과 같다 — 정착과 문명 제거 작업. RequiredProgress 600 — MVP ⑧ 전에는 300)·project_crossing(먼 항로 — 건넘의 수단. 이 구조체 그대로이고 수도의 건설 칸에서 돈다, Phase 6a) (§5.7) — DT_Relics·DT_Traits의 정의 구조체 스케치도 Phase 4 행 작성 시 추가 (§5.3, §5.7, §5.9). DT_Edicts는 B2에서 아래 구조체로 들어왔다(§5.8)

// DT_Edicts (§5.8, B2 — 헤더 docs/60_DECISIONS/headers/B2_edicts_and_research.md 2.1절). 칙령 = 통치자가 백성 전체에 거는 수정자이고 켜 두는 것이다 — 내리면(제정) 서 있고 철회하면 사라진다. 행 이름(접두어 edict_, 테이블을 가로질러 유일)은 FCivilizationState.ActiveEdicts에 저장되므로 배포 뒤 바꾸지 않는다. 어느 위기의 답인지는 칙령 행이 아니라 위기 행의 RulerResolutions가 말한다. 행을 읽는 추첨은 없다 — 모든 칸이 값·곱·합이다. 로더: FCosmicDataTables.Edicts(고정 경로 /Game/CA/Data/DT_Edicts — LoadFromFixedPaths가 읽는 열한 테이블의 끝, 없으면 LogCosmic 한 줄)·FindEdict(행 하나, 없으면 null — 경고하지 않는다)·GetEdictRowNames()(사전순 — 칙령 창이 그 차례로 늘어놓는다. 세계 규칙은 테이블을 훑지 않고 ActiveEdicts의 차례로 걷는다). 테이블이 없으면 아무 칙령도 내릴 수 없고 지닌 이름은 아무 일도 하지 않는다. 행 일곱(모두 조건 없음·제정 값 영향력 40, 여섯은 RequiredTier T1 — 값의 원본은 Tools/data/DT_Edicts.json): edict_free_trade(무역 자유화 — B3부터 조약 승낙의 문턱 −0.2)·edict_machine_rights(기계 권리 — AI 각성의 답)·edict_mobilization(전시 동원 — 물려받은 칙령, DT_Rules.StartingEdict. B3부터 함대의 자람 ×1.5)·edict_open_thought(사상 개방)·edict_vigil(밤샘·절식 — 설계 이름 철야·금식), 그리고 B3의 edict_quarantine(격리령 — 유지비 없음, 틱당 효과 모든 수입 ×0.9, HazardSpawnMultipliers {Hazard.Plague: 0.1} — 제 땅의 발생과 번져 오는 역병을 함께 줄인다, 이미 선 역병은 그대로; 헤더 B3 3.2절), 그리고 B4의 edict_against_silence(저녁 안부 — 설계 이름 침묵의 금지. RequiredTier T2, 유지비 없음, 틱당 효과 모든 수입 ×0.9, PressureDrainMultiplier 0.45 — 영압이 안정도를 깎는 몫을 줄인다. 안정도를 주지 않고 HazardCountdownBonus를 싣지 않는다; 헤더 B4 3.2절. 사전 차례로 맨 앞이라 칙령 창의 둘째 칸부터 맨 위에 선다). 예약 이름 셋은 docs/10_DESIGN/systems.md §5.8

USTRUCT(BlueprintType)

struct FEdictDef : public FTableRowBase

{

GENERATED_BODY()

UPROPERTY(EditAnywhere) FText DisplayName; // 칙령의 이름 — 칙령 창·성계 패널·툴팁과 칙령 줄의 슬롯 {Edict}. 스키마: 필수, 슬롯·숫자 없음, 낱말 목록 verdict·holy_words·soul_verbs·rescue_words(B4) 금지. 카드가 칙령을 내리는 선택지의 이름은 이 이름을 글자 그대로 품는다(Data.Integrity, B4 — 슬롯 {Edict}를 두지 않는다)

UPROPERTY(EditAnywhere) FText Summary; // 칙령 창의 한 줄 설명 — 낱말로만(수는 화면이 아래 칸의 값에서 만든다). 스키마는 DisplayName과 같다

UPROPERTY(EditAnywhere) EAscensionTier RequiredTier = EAscensionTier::Tier1_Planetary; // 이 칸부터 제정할 수 있다(백성의 Tier가 낮으면 거절 사유 Tier). 칙령 창은 칸이 모자란 행을 지닌 것이 아니면 세우지 않는다

UPROPERTY(EditAnywhere) TArray<FCosmicCondition> EnactConditions; // 제정의 조건(AND, 빈 배열은 참 — 거절 사유 Conditions). 주체는 그 백성과 그 수도, SourceRow = 이 행. B2의 다섯은 비었다(첫 쓰임은 B7의 낭독 — 책을 지닌 백성)

UPROPERTY(EditAnywhere) FResourcePool EnactCost; // 제정할 때 한 번, 비축에서 모두 내거나 내지 않는다(거절 사유 Cost). 멤버가 0 아래·숫자 아님·무한이면 0으로 읽는다(CosmicEdicts::CostOf). 철회는 값이 없고 다시 내리면 다시 낸다. 스키마 0 이상

UPROPERTY(EditAnywhere) FResourcePool UpkeepPerTick; // 서 있는 틱마다 경제 단계에서, 그 틱의 수입이 비축에 든 뒤 모두 낸다 — 다 내지 못한 칙령은 그 틱에 그친다(ActiveEdicts에서 빠진다. 그 틱의 효과는 이미 났다. 다스리는 백성이면 줄 log_edict_lapsed, AI는 줄 없이). 읽기는 EnactCost와 같다. 스키마 0 이상

UPROPERTY(EditAnywhere) TArray<FCosmicEffect> EffectsPerTick; // 틱당 원천(§7.4 경제 단계, §7.5 Apply): 수입 계산 뒤·영압 소모 앞에 ActiveEdicts 차례로, bPerTickSource로, 주체는 그 백성뿐(성계·OtherID 없음), SourceRow = 이 행. 양을 고치는 효과 일곱만(스키마 per_tick). DurationTicks는 0 — 칙령은 철회될 때까지 선다(스키마 no_duration, Data.Integrity). ChangeRelation은 Key로 상대를 적는다(스키마 keyed, Data.Integrity — 주체에 OtherID가 없다)

UPROPERTY(EditAnywhere) TMap<FGameplayTag, float> HazardSpawnMultipliers; // Hazard.* → 그 백성의 정착된 성계에서 그 Draw 행의 발생 몫에 곱하는 값 — 아키타입의 같은 이름 칸 곁에 곱하고 추첨 수는 그대로다. 지닌 칙령의 값을 ActiveEdicts 차례로 곱한다(CosmicEdicts::SpawnMultiplierOf). 0 아래·유한하지 않은 값은 0. 스키마 0 이상

UPROPERTY(EditAnywhere) TMap<FGameplayTag, int32> HazardCountdownBonus; // Hazard.* → 그 백성의 성계에 그 Hazard가 놓일 때 카운트다운에 더하는 틱(CosmicHazards::Place — 놓는 모든 길이 지난다). 0보다 큰 값만 int64로 더해 0~100,000으로 자른다(CosmicEdicts::CountdownBonusOf). 칙령은 서 있는 Hazard의 카운트다운을 바꾸지 않는다(서 있는 위기의 시간을 늘리는 길은 카드의 DelayHazard 하나 — B4). 스키마 0 이상·정수

UPROPERTY(EditAnywhere) float FleetGrowthMultiplier = 1.f; // B3: 지닌 백성의 함대가 자라는 몫에 곱한다 — 지닌 칙령의 값을 ActiveEdicts 차례로 곱한다(CosmicConflict::FleetGrowthMultiplierOf). 0보다 크고 유한할 때만 읽고 그 밖은 1. 스키마 0 이상과 행 규칙 gt 0(0이면 물려받은 칙령이 통치자의 함대를 첫 틱부터 0에 묶는다). 데이터: 전시 동원 1.5, 나머지 1

UPROPERTY(EditAnywhere) float TreatyAcceptBonus = 0.f; // B3: 지닌 백성이 청하는 조약의 승낙 문턱(DT_Rules.TreatyAcceptRelation)을 이만큼 낮춘다 — 지닌 칙령의 합(CosmicDiplomacy::TreatyAcceptBonusOf, 0보다 크고 유한한 값만). 스키마 0~1. 데이터: 무역 자유화 0.2, 나머지 0. 정의 데이터라 두 열 모두 저장 변화가 없다. 칙령 창의 툴팁이 두 열을 한 줄씩 보인다(곱 1·보너스 0이면 줄 없음)
UPROPERTY(EditAnywhere) float PressureDrainMultiplier = 1.f; // B4(헤더 B4 2.3절): 영압이 지닌 백성의 Stability를 깎는 몫에 곱한다 — 지닌 칙령의 곱(`CosmicEdicts::PressureDrainMultiplierOf`, `ActiveEdicts` 차례), 백성마다 한 번. 0보다 크고 유한할 때만 읽고 나머지는 1. 스키마 (0, 1]. 누르는 초월자의 틱당 효과에는 닿지 않는다. 정의 데이터라 저장 변화가 없다. 데이터: `edict_against_silence` 0.45, 나머지 행 1(`[B-4d2]`)

};

// DT_Traits 컬럼 추가분 (§5.9): int32 BodyMod, MindMod, CausalityMod, LifespanTicksMod. VS 신설 행 (§5.3, §5.11): Trait.Society.Divided, Trait.Faith.Book, Trait.Faith.BookShut, Trait.Biology.Shortlived. Trait.Faith.Remnant/Covenant/Exile 행에 Hazard.Claim 발생 가중·카운트다운 보정 (§4.2)

```

**정의 스키마**: 파일 하나 `Tools/data/schema.json`이 선언한다 — 행 이름·태그·텍스트를 담는 모든 컬럼이 무엇을 가리킬 수 있는지(대상 테이블 또는 태그의 뿌리), 조건·효과 열거자 55개(`EConditionType` 33 + `EEffectType` 22 — B4 뒤. B3 뒤에는 32 + 20, 거대 프로젝트와 승천 판정 작업 뒤에는 30 + 18이었다) 각각의 Tag·Key·Value 쓰임, 지금은 맨 이름으로만 있는 닫힌 낱말 목록. 저작 시점 전용이다: 런타임 구조체·필드·해시·세이브가 바뀌지 않는다. 검사는 `Tools/make_data_tables.py`가 에셋을 만들 때 하고, C++ 테스트 하나가 로드한 에셋을 다시 본다(§9.3 `Data.Integrity`). 규칙 종류는 닫힌 목록이고 새 종류는 §12.3을 거친다. Phase 2의 스키마 검사기 작업에서 만들었다(§11). 행 이름의 접두어와 테이블을 가로지른 유일성은 §10.4에 적는다.

**스키마의 꼴.** 스키마에는 행 구조체가 있는 테이블만 적는다(지금 열: `DT_Rules`·`DT_Civs`·`DT_LogTemplates`·`DT_Events`·`DT_Galaxies`·`DT_Transcendents`와, 위기 처리 작업에서 컬럼을 적은 `DT_Hazards`와, 거대 프로젝트와 승천 판정 작업에서 컬럼을 적은 `DT_Megaprojects`·`DT_Feats`와, MVP ②에서 컬럼을 적은 `DT_Encounters`(B1에서 `Subject`의 `VesselSystem`과 행 접두어 `enc_`의 규칙을 더했다. B3에서 `required_rows`의 `petition_first_contact`, 글 칸의 슬롯 `OtherCiv`, 선택지 효과 칸의 `card`, 첫 접촉 카드의 두 규칙 — 그 카드가 아닌 행은 `SetStance`도 `{OtherCiv}`도 갖지 않는다(`row_not`), 그 카드는 `Named`·`Civilization`이다(`row`) — 와 `NextEncounter`의 대상 시험 `row_not`을 더했다)와, B2에서 구조체와 함께 적은 `DT_Edicts`(struct `EdictDef`, 접두어 `edict_`, `must_exist`. B3에서 두 열 `FleetGrowthMultiplier`·`TreatyAcceptBonus`와 `when` 없는 행 규칙 하나 — `FleetGrowthMultiplier`는 0보다 크다 — 를 더했다). `DT_Relics` 같은 테이블은 구조체와 함께 온다).

- `tables`: 테이블마다 `struct`(행 구조체 이름에서 F를 뺀 것), 행 이름의 꼴(`prefixes` — 접두어, 또는 `names` — 그 이름만), `must_exist`(원본과 에셋이 반드시 있어야 하는가 — 에셋이 생긴 테이블은 켠다), `required_rows`(코드가 이름으로 부르는 행), `columns`, `rules`, `unique`, `pairs`.
- `columns`: 행 구조체의 **모든** 컬럼을 적는다 — 스키마에 없는 컬럼과 구조체에 없는 컬럼은 둘 다 문제다. 컬럼마다 종류가 하나다. 행이 아직 없는 테이블은 `null`이고 첫 행을 쓸 때 적는다(`null`인 테이블에 원본이나 에셋이 있으면 문제다).
- 컬럼의 종류(닫힌 목록 18)와 종류마다 쓸 수 있는 선택 항목(이것도 닫힌 목록이고, 검사기 머리의 `COLUMN_OPTIONS`가 쥔다): `pool`(자원 풀 `FResourcePool` — 낱말 목록 `resource`의 다섯 멤버를 빠짐없이, 다른 키 없이 수로 적는다. `min`. `Data.Integrity`가 그 목록을 구조체의 멤버에 맞춘다 — MVP ② A-2a), `structs`(구조체의 배열 — `columns`(원소의 멤버마다 지금의 종류와 선택 항목, `structs`는 다시 담지 않는다)·`min`·`len_in`. 원소는 `columns`와 정확히 같은 키를 갖는다: 빠진 멤버가 C++ 기본값으로 조용히 남지 않는다. 행 규칙은 하위 열을 `List.Column`으로 부르고 모든 원소에서 성립해야 참이다(빈 배열은 참). `DT_Encounters.Choices` — MVP ② A-2a), `plain`(검사하지 않는다 — bool·색·그 밖의 구조체. 선택 항목도 시험도 없다: 구조체에는 `is`·`in`이 늘 거짓이고 `set`이 늘 참이라 규칙을 조용히 끈다. bool 컬럼에 시험이 필요해지면 그때 종류를 더한다), `number`(`min`·`max`·`integer`), `numbers`(`len`·`ascending`·`ends_at`(마지막 칸의 값 — 누적 분포의 끝)·`min`·`max`·`integer`), `text`(`slots`·`no_digits`·`required`·`hedged`(수가 든 마디마다 이 낱말 가운데 하나를 제 낱말로 품는다 — '약'·'추정'·'논쟁 중'. 수는 어느 문자의 숫자든 되고, '개'·'광년' 앞의 한글 수 낱말(십·백·천·만·억·조 — '수억 개'·'십만 광년')도 수다. 'M87'·'NGC 205' 같은 목록 번호는 이름이라 수가 아니다. 마디는 ', '·' — '·'; '에서 나눈다(천 단위 쉼표는 뒤에 빈칸이 없어 마디를 나누지 않는다). 앞에 한글 음절이 붙으면 다른 낱말이고('요약'), 한 음절 낱말은 뒤에 붙어도 다른 낱말이다('약한' — '추정치'·'추정된'은 든다). §8.3은 수마다 붙이라고 하고, 검사기는 마디마다 본다)·`forbidden`(이 낱말 목록의 낱말을 품지 않는다 — 목록 이름 하나, 또는 B2부터 목록 이름의 배열: 칙령의 이름은 `["verdict", "holy_words", "soul_verbs"]`, 로그 틀은 B3부터 `["verdict", "soul_verbs", "holy_words"]`)), `texts`(`min`·`len_in`·`empty_or_min`·`unique`·`slots`·`no_digits`·`single_variant`·`hedged`·`forbidden`), `tag`(`root`·`optional`), `tags`(태그 컨테이너 — `root`), `tag_list`(태그 배열 — `root`), `tag_map`(키가 태그, 값은 수 — `root`·`min`·`max`·`integer`), `row`(`table`·`optional`·`target` — 가리키는 행이 맞아야 하는 시험. 행 규칙의 시험과 같은 목록이고 행 이름을 짚는 `row`·`row_not`도 든다 — B3: `DT_Encounters`의 `NextEncounter`는 `{"row_not": "petition_first_contact"}`, 사슬이 첫 접촉 카드로 이어지지 않는다), `row_map`(키가 행 이름, 값은 수 — `table`·`target`·`min`·`max`·`integer`), `word_map`(키가 `words`의 낱말, 값은 수 — `words`·`min`·`max`·`integer`), `enum`(`enum`), `conditions`, `effects`(`per_tick` — 틱당 원천의 효과 일곱만, §7.5 · `no_duration` — 어느 원소도 `DurationTicks`를 갖지 않는다 · `keyed` — 여기 적은 효과 종류는 Key를 적어야 한다. 뒤의 둘은 B2의 `DT_Edicts.EffectsPerTick`이 쓴다: 칙령은 철회까지 서 있고, 틱당 원천의 주체에는 상대 ID가 없어 `ChangeRelation`이 상대를 Key로 적는다 · `card` — 카드의 답이 적용하는 효과 칸이다: `card_only` 효과가 설 수 있는 유일한 자리, B3의 `DT_Encounters` 선택지 `OnSuccess`·`OnFailure`), `rows`(행 이름의 배열 — `tables`·`unique`. 원소마다 `tables`에 적은 테이블 가운데 하나의 행이고, `unique`면 한 배열에 두 번 쓰지 않는다. `DT_Hazards`의 해결 수단 두 컬럼 — 위기 처리 작업). 정수 프로퍼티의 컬럼에는 `integer`를 적는다(빠지면 `Data.Integrity`가 실패한다).
- `rules`: '`when`이 다 맞는 행은 `then`도 다 맞는다'. 시험은 닫힌 목록이다 — 컬럼 하나에 `is`·`in`·`gt`·`ge`·`lt`·`le`·`set`·`min_len`·`max_len`·`each_contains`·`lacks_type`(조건·효과 배열에 그 열거자가 없다)·`has_type`(그 열거자가 있다)·`has_cond`(그 열거자와 그 Key를 함께 가진 원소가 있다 — `has_type`이 가르지 못하는 `PlayerRoleIs Ruler`와 `Wanderer`, B1)·`slots_in`(텍스트의 슬롯이 그 목록 안에 있다)·`len_ge_col`(목록의 길이가 같은 행의 수 컬럼 이상)·`pairs_ge_col`(두 목록의 길이를 곱한 것이 같은 행의 수 컬럼 이상 — 짝으로 나눠 주는 이름 풀)·`contains_all`(태그 컨테이너·태그 배열이 그 태그를 모두 담는다)·`contains_none`(태그 컨테이너·태그 배열·태그 맵이 그 태그를 하나도 갖지 않는다 — B4: 어느 초월자 행도 `Event.Crisis.Claim`에 무게를 싣지 않는다)·`zero`(자원 풀의 모든 값이 0이다(true) 또는 무엇인가를 낸다(false) — B4: '값 없는 선택지')·`lacks_words`(텍스트가 그 낱말 목록의 낱말을 품지 않는다 — 한 칸의 몇 행에만 거는 목록, B4 `[B-4d1]`: 침묵의 행의 `silence_words`, 타락 행의 `second_person`, 청원의 글 칸, 철야의 추첨 행과 함대가 닳은 줄의 `vigil_image`)·`no_digits`(텍스트에 숫자가 없다(true) 또는 있다(false) — B5 `[B-5d1]`: 열의 `no_digits`는 모든 행에 걸리니 한 범주의 행에만 거는 꼴, `Log.Record` 행) 가운데 하나, 또는 행 이름을 짚는 `row`, 또는 그 이름이 아닌 모든 행을 고르는 `row_not`(B3 — `row`와 같은 꼴: 그 테이블이 가졌거나 `required_rows`에 든 행 이름 하나. 첫 접촉 카드가 아닌 행), 또는 `row_prefix`(행 이름이 주어진 접두어로 시작한다 — 접두어는 그 테이블의 `prefixes` 가운데 하나여야 하고 아니면 검사기가 문제로 알린다. 한 종류의 행을 고른다 — 지금은 `when`에서만 쓴다, B1: `petition_`·`enc_`. 부정의 꼴은 없다 — B4의 '`petition_` 행에서만'은 '`enc_` 행이면 없다'로 적고, 테스트가 `DT_Encounters`의 접두어 목록을 [`enc_`, `petition_`]로 박아 새 접두어가 그 규칙을 다시 묻게 한다), 또는 `none`(B4 — `{"none": [시험들]}`: 구조체 목록 하나의 하위 열(`List.Column`)을 짚는 시험들을, 목록의 어느 원소도 모두 지니지 않으면 참 — 선택지 하나를 통째로 읽는 규칙: 칙령을 내리거나 시간을 사는 선택지는 판정이 없고, 칙령을 내리는 선택지는 값이 없으며, 값도 문도 없는 선택지가 없는 카드는 문이 있는 효과를 싣지 않는다). 선택지마다 따로 거는 규칙 가운데 그 앞의 것(판정이 있는 선택지만 실패를 갖는다 — MVP ②, 방랑자 카드의 비용 0과 '`OnSuccess`에 `DeposeRuler`면 `NoCheck`이고 `Reign.Abdicated`' — B1)은 스키마의 시험이 아니라 C++ `Data.Integrity`가 본다(에디터 없는 검사에는 보이지 않는다). B4의 선택지 규칙 둘 — 문이 있는 효과(`SetStance`·`EnactEdict`·`DelayHazard`)는 한 선택지에 하나뿐이고 `OnSuccess`의 첫째다(`ChoiceGateOf`는 답 전의 상태를 읽으므로 앞선 효과가 수를 뽑은 뒤 문을 닫지 못하게), 칙령을 내리는 선택지의 `Label`은 그 칙령의 `DisplayName`을 글자 그대로 품는다 — 도 C++다. 두 테이블을 가로지르는 물려받은 칙령의 불변식(B2 — 최대 회복 `StabilityRecoveryPerTick` × (1 + `SocietyRecoveryBonus`)이 `StartingEdict` 행의 순 안정도 소모보다 작다, `CosmicEdicts::InheritedDrainOutrunsRecovery`)도 그렇고, B4의 **신정론 불변식**도 그렇다(헤더 B4 2.8절·11절 항목 7·18): 위기 행마다, 그 태그를 `ResolveHazard`로 걷는 은혜 아닌 `DT_Events` 추첨 행들의 슬롯당 몫의 합(`BaseWeight` × 영향권 곱의 위쪽 끝 — 그 범주에 무게를 싣는 초월자 행마다 min(3, 무게 × (1 + `SphereEventWeightStep` × 그 행이 가질 수 있는 가장 큰 영향권))을 1보다 클 때 곱한 것 — × `EventChancePerSlot`)으로 (1 − 몫)을 그 위기의 가장 긴 머묾(`CountdownTicks` + 모든 칙령 행의 그 태그 `HazardCountdownBonus` 양수의 합 + `FleetDelayCap` + 카드 행마다 그 태그를 겨누는 `DelayHazard`의 가장 큰 값)의 슬롯 수(⌈머묾 ÷ `EventSlotTicks`⌉)만큼 고리로 곱해(`Pow` 없음) 0.25 이상. `[B-4d2]`가 둘을 더했다 — 그 태그를 걷는 은혜 아닌 행이 추첨이 아니면(Standing·Named — 조건이 설 때마다 걷는다) 확실한 걷힘으로 세고, 카드가 그 위기의 시간을 사면 `DT_Rules.ThroneLockTicks`가 그 가장 긴 머묾 이상이어야 한다(새 통치가 같은 주장에 카드를 다시 데려오지 못하게 — 지금 550 ≤ 600). 그 셈이 카드 행마다 `DelayHazard`를 한 번으로 세므로 시간을 사는 카드의 꼴도 지킨다 — 추첨 행이 아니고, `NextEncounter`의 고리에 있지 않으며, 추첨 행이 이끌지 않고, 그 카드와 그것을 이끄는 행 가운데 Named가 아닌 행(뿌리)은 하나다. 지금의 값: 주장 300 + 100(밤샘·절식) + 100(지킴) + 50(둘째 영압 카드) = 550틱 = 55슬롯이라 철야의 추첨 행 `event_claim_eased`의 무게는 1.24까지이고 데이터는 1.2다. 시험마다 쓸 수 있는 컬럼의 종류가 정해져 있다(크기 비교는 `number`에만, `each_contains`는 `texts`에만, `is`·`in`은 값 하나짜리 컬럼에만 — 컨테이너에 쓴 `is`는 늘 거짓이라 규칙이 꺼진다. 이름 풀 시험 `len_ge_col`·`pairs_ge_col`은 목록·맵 컬럼에만 쓰고 짚는 수 컬럼은 `number`다 — 태그 컨테이너 `tags`는 export 텍스트라 길이가 없어 늘 거짓이므로 쓸 수 없다. `set`은 `number`·`enum`에 쓸 수 없다 — 늘 참이다. `plain`에는 어떤 시험도 쓸 수 없다). `unique`는 컬럼 묶음의 값이 테이블 안에서 겹치지 않음을, `pairs`는 두 컬럼의 허용 짝(이벤트 행의 `Category` → `LogCategory`, §8.1)을 적는다.
- `conditions`·`effects`: 열거자마다 `tag`(`root`·`optional`, 그리고 B4의 태그 거르기 둘 — `not_tag_of`(그 테이블에서 `when`의 시험을 지닌 행의 태그 열이 아니다: `DelayHazard`는 수도 규칙 행 — 반란 — 의 태그를 겨누지 않는다)·`not_read_by`(어느 테이블의 어느 행에서도 그 조건 열거자가 읽는 태그가 아니다: `DelayHazard`는 `SystemHazardTicksAtMost`가 기다리는 태그를 겨누지 않는다 — 그 카드가 두 번 서지 않게))·`key`(`word` — 낱말 목록, `row` — 행 이름과 그 행이 맞아야 하는 `target`, `digits` — 숫자만, `any` — 아무 이름. `optional`)·`value`(`min`·`max`·`integer`)를 적는다. 적지 않은 칸은 비어 있어야 한다 — 읽지 않는 칸에 쓴 값은 오타다. 효과에는 `per_tick_only`(틱당 원천만 거는 효과 — `MultiplyIncome`)와 `card_only`(카드의 답에만 서는 효과 — 그 효과를 실은 칸은 `card`여야 한다, B3의 `SetStance`: Tag는 `Stance.Friendly` 또는 빈 태그, Key는 아무 이름이거나 빈 값)를 더 적을 수 있고, `DurationTicks`는 틱당 원천의 효과에만 0보다 클 수 있다(`no_duration` 컬럼 — 칙령 — 은 0만). 원본의 조건·효과 객체는 구조체의 멤버를 모두 적는다(Type·Tag·Key·Value, 효과는 DurationTicks도) — 빠진 멤버와 없는 멤버는 문제다.
- 그 밖: `words`(닫힌 낱말 목록 — 키와 값이 쓰는 일곱 resource·axis·stat·role·civ_selector·policy·research와, `forbidden`·`lacks_words`가 부르는 목록 verdict·other_works·holy_and_myth(B4가 채웠다 — 성경·신화의 고유명 가운데 `Tools/data` 전체에서 0건인 이름만; 흔한 낱말 속에 드는 이름은 글 감사의 손 목록으로)·holy_words(성령·은혜·기름부음·성결 — B2)·soul_verbs(영혼 경제의 동사 '거두다'의 꼴 — B2), 그리고 B4의 일곱 verdict_without_soul(판정 어휘에서 '영혼'만 뺀 여섯 — 1인칭 칸의 예외를 '영혼' 하나로 좁힌다; 단위 테스트가 verdict와 견준다)·realm_names·rescue_words(구원·중보)·revelation_mimicry·silence_words(잠잠·고요·쉼·안식)·vigil_image(밤·철야·금식·절식·기도)·second_person, 그리고 B5 `[B-5c]`의 둘 reader_address('당신')·reader_question_ends('?'·'는가' — 아래 `epitaph_leaves_out`), 그리고 B5 `[B-5d1]`(글 감사)의 일곱 — afterlife_words(죽은 이의 행방을 말하는 관용구와 사후의 낱말)·book_of_life_words(생명책의 낱말)·called_away_words(부름을 죽음으로 읽히게 하는 말)·remnant_words('남은 자' — 택하심의 낱말이라 기록 행에만)·death_frame_words(죽음의 행위자·희생·까닭 — 죽음 행에만)·rebirth_words(돌아옴·다시 남 — 취임 행 둘과 `Succession.`에만)·record_object_words(날·이름·사람·죽은 이를 기록의 목적어로 — 기록 행에만). 글자 그대로 맞추므로 짧은 낱말이 다른 낱말 속에서도 걸린다 — 그래서 이 일곱은 새 자리(아래 접두어와 `DT_LogTemplates`의 기록·죽음·취임 행 규칙)에만 걸고 열 전체에는 걸지 않는다(헤더 B5 §3.5). 1인칭 칸(`ClaimTemplates`·`Lure`)의 `forbidden`은 한 배열이고 단위 테스트가 두 칸이 같은지 견준다 — 낱말과 글의 규칙은 docs/70 §8.3), `string_table`(String Table의 키 접두어마다 품지 않는 낱말 목록과 숫자 금지 — `checked_prefixes`. B2가 `Edict.`·`Hint.Edict`·`Resource.`를, B3가 `Stance.`·`Treaty.`·`Diplomacy.`·`Siege.`·`Guard.`를 더했다 — 판정 어휘·`holy_words`·`soul_verbs`와 숫자 금지. B4가 `Petition.Resist`·`Petition.Edict`(카드의 함대·칙령 선택지 아래 줄과 사유 — `Guard.`·`Edict.`와 같은 규칙)를 더하고 `Petition.`·`Edict.`에 `rescue_words`를 걸었다. B5 `[B-5d1]`이 죽음과 계승의 쪽 `Epitaph.`·`Reign.`·`Digest.`·`Succession.`(판정 어휘·`soul_verbs`·`holy_words`·`rescue_words`와 사후·생명책·부름의 셋, 숫자 금지 — `Epitaph.`·`Digest.`·`Succession.`은 `reader_address`도, `Digest.`는 `silence_words`도, `Succession.`은 `rebirth_words`도)을 더하고, 두 책이 든 `Chronicle.`에 `soul_verbs`·`rescue_words`와 그 셋을, `Chronicle.NoVessel`에 그것과 `silence_words`를 걸었다), `enums`(컬럼이 쓰는 열거형의 열거자), `per_tick_effects`(틱당 원천의 효과 일곱), `required_tags`(ini에 선언되어 있어야 하는 태그 — §9.3 v0.6 항목의 `Notable.*` 역할 태그 여섯과 B3의 태세·조약 태그 넷 `Stance.Friendly`·`Stance.Hostile`·`Treaty.Trade`·`Treaty.NonAggression` — 코드가 `Core/CosmicTags.h`에서 이름으로 부른다), `row_name_sources`(코드가 이름으로 부르는 행을 적어 둔 헤더 `Data/CosmicRowNames.h` — 검사기와 `Data.Integrity`가 그 이름들을 `required_rows`와 양쪽으로 맞춘다. 주석으로 막은 상수는 세지 않는다), `single_variant_rows`, `epitaph_leaves_out`(B5 `[B-5c]` — 비문이 죽은 그릇의 마지막 답 가운데 제목을 보이지 않는 `DT_Encounters` 행: 코드의 `CosmicRows::EpitaphLeavesOut`와 같은 여섯 — 읽는 이에게 말하거나 그릇을 높이는 둘 `petition_keep_throne`·`petition_exalt`와, 죽은 그릇의 이름 아래서 추모로 읽히는 넷 `petition_wise_heart`·`petition_feast`·`enc_plague_tend`·`enc_star_watch`(`[B-5d1]`의 글 감사 — 덕·기념일·희생의 '곁', 헤더 B5 §3.6의 1; 제목은 제 카드에서 그대로다) — 이고 저마다 그 테이블의 `required_rows`다 — 검사기가 보고, 목록이 없어도 문제다. 카드 제목이 읽는 이에게 말하면 — `reader_address`의 낱말을 품거나 `reader_question_ends`의 낱말로 끝나면 — 그 행은 이 목록에 있어야 한다: 죽음과 취임의 쪽은 3인칭이다, 헤더 B5 결정 12-다; `test_data_schema.py`가 깨지는 경우를 든다), `event_log_categories`(`Event.Crisis.Claim`은 `Log.Crisis`만 — B5 `[B-5d1]`이 `Log.Record`를 뺐다: 그 생산처는 위기 단계의 기록 행 둘뿐이다, 결정 11-가), `row_name_pattern`, `tag_sources`(태그를 선언하는 ini).
- 숫자의 범위는 이 문서가 정한 것만 적는다. 범위가 없는 노브는 `number`만이고 '유한한 수'인지만 본다.

**도구.** `Tools/cosmic_data_schema.py`가 검사기다 — 에디터 없이 도는 Python이고 `Tools/data`의 원본(`schema.json`, `DT_*.json` 전부, `ST_*.csv`)을 본다. 검사는 아래 셋을 차례로 한다.

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

// 물려받은 칙령(B2 — 헤더 B2 2.7절): 백성을 만드는 자리(CivID·`GalaxyRow`·수도를 적고 연구의 몫을 1/4씩 놓는 곳 — 아키타입 추첨 앞)에서 `DT_Rules.StartingEdict`가 `DT_Edicts`에 있는 행이면 그 백성의 `ActiveEdicts`에 넣는다 — 모든 백성(첫 생애의 백성도 AI도) 같은 규칙, 추첨·줄·제정의 값 없음. 그래서 추첨 수가 바뀌지 않는다. 노브가 비거나 행이 없으면 아무도 지니지 않는다. AI는 첫 AI 단계에서 철회한다(`AIEdictRevokeStability` 1 — §7.4 AI 단계)

// 현실에서 가져온 것 가운데 해시에 드는 것은 셋뿐이다: 항성 등급(5), `sys_0`의 고정 자리와 성계 좌표(4)(A0 4a부터 성역의 고정 자리 — 1.15 × 원판 반지름 — 도 해시에 든다) (§8.3, §9.2. v0.8 — 단계 5 뒤에는 관측 목록의 좌표·등급이 해시에 든다 — 오프라인 도구가 계산한 값이라 C++에 삼각 함수가 없다). 추첨의 순서(은하 생성 규칙 변경의 헤더에서 고정했다 — 같은 종류의 추첨은 묶어서 뽑는다, §9.2 ⓑ): ① 생명 성계마다 배열 순서로 — 좌표(`sys_0`은 뽑지 않는다. 그 밖은 시도마다 2회), 항성 등급 1회, 자원 풍부도 1회(등급과 풍부도는 `sys_0`도 뽑는다. 고정값이 있는 모성계는 뽑은 수를 버린다) ② 모성계의 자리 — 첫 AI 백성은 `sys_0`의 1레인 이웃에서 1회(7항), 나머지 AI 백성은 저마다 남은 성계에서 1회 ③ AI 백성마다 CivID 순서로 — 아키타입 1회, 윤리 3회(평화·영성·개방), 프로필 1회 ④ 이름 — 백성마다(플레이어 먼저) 모성계 이름 1회와 백성 이름 1회, 이어서 무주 성계마다 배열 순서로 카탈로그 이름 1회 ⑤ 타락 로스터(자리 − 1회) ⑥ 초기 코어(자리마다 1회) ⑦ 첫 생애의 그릇 이름 1회(`Install`)(성역 `sol`은 ⑥과 ⑦ 사이에서 추첨 없이 붙는다 — 11항. 그래서 추첨 수는 A0 4a 전과 같다). 아키타입·윤리·프로필·모성계 이름·백성 이름의 다섯 추첨은 §9.2 ⓐ대로 무조건 뽑는다 — 가중 행이 없어도, 아키타입이 `arch_default`가 아니어도, `AIProfile`이 차 있어도, 뱅크가 비어도 뽑고 버린다. 그래서 생성의 추첨 수는 시드와 두 수(성계·백성)와 로스터 노브만 따르고 `DT_Civs`·`DT_Galaxies`·`DT_Transcendents`의 내용에 좌우되지 않는다 — 행을 더하거나 뱅크를 채워도 지형과 모성계의 자리는 그대로다. `Sim.LayersInRange`가 결과를 본다: 생명 성계·백성 수 = 고향 행 값(성역은 따로 센다 — 배열 끝의 하나, 원판 밖), 등급 분포의 허용 오차(성역의 고정 등급은 세지 않는다), 첫 생애의 모성계 `sys_0`의 자리와 등급, 이름 중복 없음, 모든 레인이 같은 `GalaxyRow`, CivID 유일, 모든 소유 성계의 `GalaxyRow`가 주인과 같다 (§9.3)

UFUNCTION(BlueprintCallable) void SetTimeScale(float Scale); // 0,1,2,5,10, 25(셋째 칸부터 — 어느 배속이 열리는가는 표현 계층의 문이고 시뮬은 0..MaxTimeScale로만 자른다: 0 이하이거나 숫자가 아니면 0, 25 위는 25. FMath::Clamp는 NaN을 위쪽 끝으로 읽으므로 비교로). 0 = 일시정지이며 시뮬의 유일한 일시정지다 — 이 Subsystem은 엔진의 null-world 패스에서 틱하므로 엔진 일시정지(`SetGamePaused`)와 시간 팽창이 닿지 않는다. UI는 SetTimeScale(0)으로만 멈춘다

UFUNCTION(BlueprintPure) float GetTimeScale() const { return TimeScale; } // 패널의 배속 표시와 테스트의 복원용

UFUNCTION(BlueprintCallable) void StepOnce(); // 디버그/테스트. OnLogEntry·OnEventFired·OnTierAscended 핸들러 안(IsBroadcasting)에서 불리면 에러 한 줄을 내고 틱을 돌리지 않는다 — RunHeadless도 같고, 상태를 갈아 끼우는 NewGalaxy·LoadState(와 UCosmicSaveSubsystem::LoadGalaxy)도 같은 자리에서 에러 한 줄을 내고 아무것도 하지 않는다(§9.4). 저장의 SaveCurrentGalaxy·LoadGalaxy(LoadRun)는 그 자리와 틱 단계 안(IsInTickStages)에서, ContinueNewest는 그 자리에서 거절되고, 게임의 저장 요청 RequestSave는 다음 틱 경계로 미뤄진다(MVP ⑤). 틱 단계 안(IsInTickStages — B5가 열째 단계까지 넓혔다, 아래)과 OnSimTick 알림 안(IsSendingSimTick, 아래)에서도 StepOnce·RunHeadless·RunLeap은 에러 한 줄을 내고 틱을 돌리지 않는다(B5 — 셋의 문은 비공개 RefusesTicks(Caller) 하나다): OnSimTick을 내는 동안의 깊이 SimTickDepth(저장하지 않는 멤버 — IsBroadcasting은 OnSimTick을 세지 않는다)를 본다. OnSimTick 핸들러의 정지·NewGalaxy·LoadState는 그대로 된다

// 끝의 판정(MVP ④, 2026-10-23; B6a — Sim/CosmicEndings.h, 순수 함수 namespace): 끝의 판정은 FCosmicEndingRecord CosmicEndings::EndingOf(const FGalaxyState&, const FCosmicRulesDef&)이다 — 기록과 상관없이 지금 끝이 서 있는가를 답하는 순수 함수(추첨·쓰기 없음; Tick·Calling·Civs·노브만 읽고 앎·로그·연대기·말하기 스트림은 읽지 않는다). 생애가 없으면 빈 기록, 같은 틱의 차례는 Ending.RuledPeople > Ending.Vessel > Ending.OtherPeople > Ending.NoCandidate(침묵 — CosmicSuccession::IsSilent: 공백기 ∧ PendingLeapTicks ≤ 0) > Ending.Clock(ProvisionalEndTick > 0 ∧ Tick ≥ 그 값)이고 앞의 것이 서면 뒤는 보지 않는다(조건과 CivID·bNoVessel의 식은 헤더 B6a 2.2절). 셋째·넷째 칸은 끝이 아니다(셋째 칸의 끝 EndRung은 B6a에서 빠졌다). 그것을 부르는 곳은 기록을 쓰는 한 자리(아래 SimulateTick)와 테스트, 커맨드렛의 end check뿐이고, 끝을 읽는 곳(플레이어 컨트롤러의 래치 FollowEnd, 저장 층의 경계 비교 UCosmicSaveSubsystem::MarkOf, 측정의 두 정책, 커맨드렛의 end:·ending: 줄, 연대기 쪽, RunLeap의 멈춤과 AdvanceLeap의 둘째 부름)은 FGalaxyState.Ending을 읽는다 — 단계·명령·RunHeadless는 읽지 않는다. 옛 열거 ECosmicProvisionalEnd { None, Rung, Clock }과 ProvisionalEndOf는 B6a에서 지워졌다. const TCHAR* NameOf(const FGameplayTag& Kind) = 태그의 끝 낱말 "RuledPeople"·"Vessel"·"OtherPeople"·"NoCandidate"·"Clock"(빈 태그와 다른 태그는 "None" — 커맨드렛과 CosmicEnd 로그 줄)
// 연구 중점(MVP ⑦, 2026-10-24 — Sim/CosmicResearch.h, 순수 함수 namespace, 상태·추첨 없음): enum class ECosmicFocusShape : uint8 { Even, Focused, Mixed } — UENUM이 아니다. bool Focus(TMap<FName, float>& ResearchFocus, FName Field, float Share) — Field = Share, 다른 키 = (1 − Share) / (키 수 − 1)(효과 FocusResearch가 쓰던 식과 float 차례 그대로); 키 아님·0 이하·1 초과·NaN·키 둘 미만은 거짓·바뀜 없음. bool Even(TMap&) — 키마다 1 / 키 수, 빈 지도는 거짓. ECosmicFocusShape ShapeOf(const TMap&, FName* OutField = nullptr) — 정확한 비교: 키 둘 이상이 모두 같으면 Even(옛 테스트의 모두 0 지도도), 하나가 홀로 높고 나머지가 서로 같으면 Focused(OutField = 그 키), 그 밖·키 둘 미만·NaN은 Mixed. 키마다 그 키만 보고 쓰고 답은 지도의 차례에 좌우되지 않는다. 효과·명령·패널·상단 바·커맨드렛이 함께 읽는다. 분야 목록은 연구 단계의 ResearchFields(Sim/GalaxySimulationSubsystem.cpp) 그대로. B2의 연구의 몫(§16 #119 ③): FName TranscendenceField() = "Transcendence"(연구 단계의 마지막 분야와 같은 이름 — 목록은 옮기지 않는다, Sim.ResearchShares가 생성된 백성의 키로 그 같음을 본다) · constexpr int32 MaxShareWeight = 20(눈금 하나 = 몫의 약 0.05) · bool SharesOf(TMap<FName, float>& ResearchFocus, const TMap<FName, int32>& Weights) — Weights는 지도의 키 가운데 초월을 뺀 모든 키를 담고 그 밖의 키·초월의 키는 없으며 값은 0~MaxShareWeight, 합은 0보다 크다(아니면 거짓·바뀜 없음). 초월의 몫 T는 그대로이고 다른 키마다 (1 − T) × (float)무게 ÷ (float)합(합은 int32로 — 답이 지도의 차례에 좌우되지 않는다), 무게 0이면 0을 쓰고 키를 지우지 않는다. 슬라이더가 만든 몫은 ShapeOf가 Mixed로 읽을 수 있다 — 그래서 패널의 '지금' 단추와 머리 줄의 낱말은 ShapeOf가 아니라 프리셋(Focus·Even이 만들 지도)과의 같음으로 가른다(표현 계층, 헤더 B2 2.8절)
UFUNCTION(BlueprintCallable) void RunHeadless(int64 Ticks); // 테스트/밸런스용 (§4.4 — 빨리 넘기기 RunLeap(아래, B5)은 같은 꼴의 고리를 제가 돈다). 방송 게이트 OFF — 그 안의 틱은 OnSimTick을 내지 않는다. 끝에서 서지 않는다 — 시뮬은 끝을 기록하되 멈추지 않는다(위 '끝의 판정'): 기록은 SimulateTick의 열째 단계 뒤에 쓰이고(아래), 기록이 선 뒤에도 SimulateTick·StepOnce·RunHeadless는 틱을 돌린다 — 커맨드렛의 20,000틱 기준값과 테스트의 긴 판, 끝 뒤 측정(-PastEnd)이 끝 뒤로 돌고, 기록을 읽는 단계가 없어 끝 뒤의 세계는 기록이 없을 때와 같으므로 한 틱씩 돈 판과 긴 도약은 같은 세계로 남는다. 끝 뒤의 명령은 컨트롤러의 래치가 거절한다. 끝의 틱에서 서는 것은 RunLeap 하나다(아래)

UFUNCTION(BlueprintCallable) void RunLeap(int64 Ticks); // (B5 — 헤더 B5 2.6절) 빨리 넘기기: 공백기의 남은 도약을 한 번에 돌리는 시뮬의 입구 하나. Ticks만큼 RunHeadless와 같은 꼴의 고리(방송 게이트 OFF)를 제가 돈다 — 부른 때 State.Ending이 비어 있었으면 기록이 적힌 틱에서 고리를 멈추고(그 틱은 돌고 그 뒤 틱은 돌지 않는다), 부른 때 이미 기록이 있으면 멈추지 않는다(끝 뒤의 넘김 — 끝 스위치가 꺼진 세션; B6a, 헤더 B6a 2.6절). 그 뒤, 알림 문이 열려 있으면 단계 밖에서 OnSimTick(State.Tick)을 한 번 낸다(그 알림도 SimTickDepth를 세고, 그동안 IsSendingLeapBoundary가 참이다). Ticks가 0 이하면 틱도 경계도 없다(같은 틱을 두 번 알리지 않는다). 그 경계 하나가 자동 저장(Ended 또는 Succession, §9.1)·컨트롤러의 끝 래치(FollowEnd)·화면의 다시 그림(피드도 — 이 경계에서는 넘긴 틱 수와 상관없이 상태에서 다시 짓는다: 한 틱짜리 도약의 틱도 줄을 알리지 않았다. 다른 경계에서는 받은 틱이 마지막으로 본 틱 + 1이 아닐 때)을 맡고 명시 저장은 두지 않는다. 세계는 한 틱씩 돈 것과 같다. 부르는 자리는 컨트롤러의 위젯 없는 함수 하나(ACosmicPlayerController::AdvanceLeap — 틱 수는 PendingLeapTicks이고 시계로 미리 자르지 않는다: RunLeap이 끝의 기록이 적힌 틱에서 서고, 끝 스위치가 켜져 있으면 그 경계가 래치한다. 끝 스위치가 꺼진 세션에서는 그 부름이 틱을 나아가게 했고 부른 때 비어 있던 기록이 이제 서 있으면 RunLeap(PendingLeapTicks)을 한 번 더 부른다 — 기록은 도약을 많아야 한 번 멈추므로 PendingLeapTicks로 되풀이하지 않는다. 남은 도약이 0이면(침묵 — 끝 스위치가 켜져 있으면 침묵은 그 틱의 끝이라 래치가 이미 섰다) 아무것도 돌리지 않고 시간을 보통의 놀이로 돌려준다 — 정지 전의 배속(없으면 ×1)으로, 자동 정지의 까닭은 비운다 — 그리고 거짓이다. 공백기가 아니거나 끝난 판이거나 시뮬이 넘김을 거절했어도 거짓이고 아무것도 돌리지 않는다)이고, 테스트도 그것을 부른다(`Calling.LeapDeterminism`·`Sim.SaveLeapResume`·`UI.LeapBoundary` — `Tests/CosmicLeapTests.cpp`; 기록의 틱에서 서는 멈춤은 `Sim.RunLeapStopsAtEnding`). 핸들러 안·틱 단계 안·OnSimTick 알림 안에서는 거절한다(위 StepOnce)

const FGalaxyState& GetState() const { return State; } // C++ 위젯 베이스가 const 참조로 읽는다(§10.2). BlueprintPure로 두지 않는다 — FGalaxyState의 멤버는 Blueprint에 보이지 않고 노드 평가마다 상태 전체가 복사된다. WBP에 필요한 값은 좁은 접근자로 낸다

FGalaxyState& GetMutableState() { return State; }

FRandomStream& GetRng() { return Rng; }

void SyncRngState(); // State.RngState = Rng.GetCurrentSeed(). NewGalaxy와 SimulateTick의 끝에서 자동으로 부른다. 틱 밖에서 GetRng()를 쓴 호출자(조우 판정 등)와 저장 직전의 저장 Subsystem은 직접 부른다. ComputeStateHash는 살아 있는 Rng를 읽는다

uint32 DrawTell(int32 Count); // 말하기 스트림 (§9.2). State.TellRngState로 지역 스트림을 열어 0 이상 Count 미만의 수를 한 번 뽑고 상태를 되적는다 — 살아 있는 스트림 객체를 두지 않는다(GetTellRng 같은 getter는 없다). PushVariants의 문장 변형 추첨만 쓰고(부르는 함수는 그것 하나다) 어떤 규칙도 이 값을 읽지 않는다. 저장 값이 항상 최신이라 해시는 TellRngState를 그대로 읽는다. Count < 1은 호출자 오류다 — 에러 로그 한 줄, 추첨 없음, 0 반환. 핸들러 안(IsBroadcasting)에서 불려도 같다(Phase 2 종료 리뷰). v0.7 예약 커밋

uint32 ComputeWorldHash() const; // 세계 해시 (§8.1, §9.2). ComputeStateHash에서 Knowing·Log·Chronicle·ChronicleEviction(MVP ⑤)·TellRngState를 뺀 것(최상위 멤버 이름 기준 — 엔딩 기록 Ending(B6a)은 세계 상태라 빼지 않는다). 앎이 세계를 바꾸지 않음을 잰다(Sim.KnowingInert). 쿠킹 빌드는 이것과 ComputeStateHash 둘로 견준다(같은 말하기 모드 — MVP ⑨). v0.7 예약 커밋

bool NoteMet(FName CivID, FName SeenAtSystemID = NAME_None); // 앎에 쓰는 함수 (§8.1). 격차 규칙을 적용하고 Knowing.MetCivs에 없을 때만 더하며, 새로 적을 때 만남의 줄을 낸다(말하기 스트림만. 주제는 그 백성과 SeenAtSystemID). 새로 만났으면 true. 그릇이 없으면(공백기) 아무것도 하지 않고, 핸들러 안에서는 에러 한 줄로 거절한다. 그때 Knowing.FirstContactCivID가 NAME_None이면 그 만남이 첫 접촉이고 같은 호출이 그 칸에 만난 백성의 ID를 적는다(아래 '앎 단계'). 그 첫 접촉에서 몸을 입은 통치자이고 DT_Encounters에 petition_first_contact 행이 있으면 같은 호출이 Knowing.PendingContactCivID에도 그 백성을 적는다(첫 접촉 카드의 대기 칸 — B3, 카드 행과 함께. 행이 없는 테이블에서는 적지 않는다). 시뮬 추첨 0. Phase 2

bool NoteKnown(FName SystemID); // Knowing.KnownSystems에 같은 방식으로 더한다. 줄은 내지 않는다. 새로 적었으면 true. 시뮬 추첨 0. 가드는 NoteMet과 같다. Phase 2

int32 LookFrom(TConstArrayView<int32> EyePlaces, int32 Reach, bool bSees); // 눈 하나의 보기 — 앎 단계의 두 쓰기(B1에서 TickSight 안에서 레인을 따라 너비 우선으로 찾아 나가던 일을 함수로 떼어 냈다, 동작 그대로 — 테스트 Sim.SightWalkMoved): 눈의 자리 모두에서 한 번에 찾아 나가고(CosmicState::SightWalkOf), bSees이면 Reach 레인 안의 성계를 찾은 차례로 NoteKnown, 이어서 아는 성계 전부의 주인을 가까운 순(눈에서의 레인 수 — 보는 거리의 상한 없이, 동률은 CivID)으로 NoteMet. 세계를 읽고 앎과 이야기만 쓴다, 시뮬 추첨 0. 새로 안 성계의 수를 돌려준다. 부르는 곳은 TickSight와 방랑자의 스캔(UPlayerRoleSubsystem::ScanCurrentSystem, 틱 밖) 둘이다

EAscensionTier GetViewerTier() const; // 보는 쪽 Tier(§8.1): 그릇이 다스리는 백성이 있으면 그 백성의 Tier, 없으면(방랑자·공백기) Calling.PersonalTier. NoteMet의 격차 규칙과 이야기의 범위가 읽는다 — 세계는 읽지 않는다

const FCosmicRulesDef& GetRules() const { return Rules; } // DT_Rules 단일 행(행 이름 rules). 테이블이나 행이 없으면 C++ 기본값

FCosmicStageClock& GetStageClock() { return StageClock; } // 단계의 시계(B3 — 측정 전용, 헤더 B3 5절): struct FCosmicStageClock { enum class ESiegeEnd : uint8 { Taken, Held, Lifted }; struct FSiegeEnd { ESiegeEnd Kind = ESiegeEnd::Lifted; FName Attacker; FName Owner; FName SystemID; }; bool bOn = false; uint64 Cycles[6] = {}; int64 Ticks = 0; TArray<FSiegeEnd> SiegeEnds; } — USTRUCT가 아니다. 부르는 쪽(밸런스 커맨드렛)이 bOn을 켜면 SimulateTick이 외교·분쟁·위기·AI 네 단계의 FPlatformTime::Cycles64 차를 [0]~[3]에, B4부터 경제 단계와 앎 단계 끝의 카드 제시를 [4]·[5]에 더하고(연구 단계는 재지 않는다) Ticks를 센다. 켜져 있는 동안 분쟁 단계는 끝낸 공성(차지함·지켜 냄·문을 잃어 풀림 — 공격한 백성·끝나기 전의 주인·성계)도 SiegeEnds에 적는다(커맨드렛의 conflict 줄이 읽고 비운다). 저장·해시 밖이고 어떤 규칙도 읽지 않으며 차례도 추첨도 바꾸지 않는다 — 멤버라도 플레이어·생애 종속 상태가 아니다(불변 규칙 2)

const FCosmicDataTables& GetTables() const { return Tables; } // 정의 테이블(§7.3, Data/CosmicDataTables.h). Initialize가 고정 경로 /Game/CA/Data/DT_*에서 로드한다. 설정 에셋은 두지 않는다

void SetTables(const FCosmicDataTables& InTables); // 테이블을 바꾸고 규칙 행과 고향 은하 행을 다시 읽는다. 테스트가 일시 테이블을 주입한다

const FGalaxyDef& GetHomeGalaxy() const; // DT_Galaxies의 고향 행(DT_Rules.HomeGalaxyRow). 테이블이나 행이 없으면 C++ 기본값이고, 행이 빠진 테이블은 에러 한 줄

FCosmicEvalContext MakeEvalContext(); // 살아 있는 상태·시뮬 스트림·색인·테이블·규칙을 평가 문맥으로 묶는다(§7.5). 효과를 적용하거나 행을 이름으로 내는 단계 코드가 여기서 만든다(이벤트 슬롯은 상태와 스트림을 건네받아 스스로 만든다). 틱 밖 호출자는 끝에 SyncRngState()를 부른다

const FCosmicStateIndex& GetStateIndex() const; // ID → 배열 인덱스(Sim/CosmicStateIndex.h). 저장하지 않고 해시하지 않으며 조회만 한다(순회하지 않는다). NewGalaxy·LoadState·건넘 단계가 다시 만든다. 조회는 그 인덱스의 원소가 정말 그 ID인지 확인하고, 낡았으면 에러 한 줄과 INDEX_NONE이다. 같은 파일의 CosmicState::IsSettled가 '정착된 성계'의 술어 하나다(§5.7)

bool IsBroadcasting() const; // OnLogEntry·OnEventFired·OnTierAscended를 내는 동안 참. 그 핸들러에서 부른 EvaluateAndFire·FireRow는 에러 한 줄을 남기고 아무 일도 하지 않는다. MakeEvalContext는 에러 한 줄을 남기되 문맥은 돌려준다(참조를 든 값이라 빈 문맥이 없다). static인 Apply·ApplyAll은 방송 셈에 닿지 못해 핸들러 규칙으로만 금한다(§9.4). 핸들러 안에서 거절되는 호출의 다른 목록(Push·PushVariants·DrawTell을 비롯해)은 헤더 주석과 CLAUDE.md 불변 규칙에 있다
bool IsInTickStages() const; // 열 단계와 그 끝의 SyncRngState가 도는 동안 참(SimulateTick 안 — MVP ⑤; B5가 열째 단계 '역할 효과 해소'와 SyncRngState까지 넓혔다). 새 깃발은 두지 않았다: 열째 단계가 틱을 돌리지 않게 되어(공백기는 카운트다운, 아래 역할 효과 해소) 옛 도약을 위해 밖에 둔 까닭이 사라졌고, 열째 단계가 Install과 줄(폐위·양위·죽음·취임의 줄)을 쓰는 동안의 저장도 같은 문을 지난다. StepOnce·RunHeadless·RunLeap은 그 안에서 에러 한 줄로 거절하고, SimulateTick의 ensure는 뒷받침으로 남는다. 저장 층은 틱 경계와 입력에서만 쓰고 읽으므로 그 안에서는 핸들러 안처럼 거절한다(§16 #33 ①)
bool IsSendingSimTick() const; // OnSimTick을 내는 동안 참(B5 — 깊이 SimTickDepth > 0, 저장하지 않는다. 틱 경계라 Core Logic 델리게이트의 IsBroadcasting과 따로다): 그 핸들러는 시간을 멈추고 상태를 갈아 끼울 수 있지만(NewGalaxy·LoadState) 틱은 돌리지 못한다 — StepOnce·RunHeadless·RunLeap이 거절한다
bool IsSendingLeapBoundary() const; // RunLeap이 제 경계의 OnSimTick을 내는 동안 참(B5 — 비공개 bSendingLeapBoundary, 저장하지 않는다. 그 알림 안에서는 IsSendingSimTick도 참이다): 그 경계 뒤의 틱은 알림 문이 닫혀 표현 계층에 아무것도 알리지 않았으므로, 알린 것을 쥐는 화면(로그 피드)은 도약의 길이와 상관없이 상태에서 다시 짓는다

// 영압 (§4.2). 파생값, 저장 안 함. Presentation의 영적 시야(§6.4)도 이 함수를 읽는다

UFUNCTION(BlueprintPure) float ComputeSpiritualPressure(FName SystemID, FName& OutDominantSourceID) const; // Phase 2 영적 압력 작업. 본체는 static FCosmicPressure ComputePressureIn(State, Index, Tables, Rules, SystemIndex)이고 이 멤버는 살아 있는 상태로 그것을 불러 P와 지배 원천의 ID만 돌려준다 — Check는 static을 부르고, 경제 단계는 같은 본체를 제 영향권 소속 표로 돈다(§9.4 — 두 길이 같은 값을 내는지는 `Sim.PressurePerCivilization`이 본다). FCosmicPressure = { P, 지배 원천의 종류(초월자·문명·방랑자), 그 ID, 그 Tier }: 안정도 소모가 '지배 원천의 Tier가 더 높을 때만'이라 종류와 Tier를 함께 돌려준다(심도는 싣지 않는다 — 행에서 읽는다). 항은 셋이다. 봉인되지 않은 초월자마다 PressureBySourceTier[Tier] × (1 + Depth) × 닿는 정도(Depth는 행의 값을 0~1로 잘라 읽는다, §3.2 심도. 닿는 정도는 static SphereReachIn이 말한다 — 영향권 안 1, 1레인 인접 PressureAdjacencyFalloff, 그 밖 0. 행이 없는 인스턴스는 0) / 그 성계를 소유한 문명의 PressureBySourceTier[Tier](AI든 플레이어가 통치하든 같다 — 통치하는 그릇은 제 항을 따로 더하지 않는다) / 방랑자인 그릇의 PressureBySourceTier[PersonalTier](그릇이 선 성계에, Calling.LifeState가 Embodied일 때만 — 공백기에는 그릇이 없다). 문명 항과 방랑자 항에는 심도가 없다. 죽은 노드는 원천이 아니다(위기 처리 작업부터). 성역(`CosmicState::IsSanctuary`)에는 어느 원천도 닿지 않는다 — 본체(두 길이 함께 쓰는 `PressureWithReach`)가 원천을 더하기 전에 0을 돌려준다(`Law.Hedge`, A0 4a). 지배 원천은 가장 큰 항의 원천이고 동률은 Transcendents 배열의 앞쪽, 그다음 문명, 그다음 방랑자다(방랑자의 ID는 NAME_None — 그릇에는 ID가 없다). 수정자 ①(경제 단계의 틱당 원천)·③(조건)은 이 작업, ④(정착 — AI와 통치자 명령)는 정착과 문명 제거 작업이고 이 함수가 아니라 영향권 소속을 읽는다(§4.2), ②(분쟁 — 격차 곱·영향권 안의 소모·지킴의 지연)는 B3이고 소모는 이 함수가 아니라 아래 ComputeAttritionPressureIn을 읽는다

static FCosmicPressure ComputeAttritionPressureIn(const FGalaxyState&, const FCosmicStateIndex&, const FCosmicDataTables&, const FCosmicRulesDef&, int32 CivIndex, int32 SystemIndex); // 소모의 압력(B3 — 헤더 B3 2.3절, ComputePressureIn 곁의 공개 static): 백성 CivIndex의 함대가 SystemIndex에서 만나는 압력 = 봉인되지 않은 초월자 가운데 행의 칸이 그 백성보다 높고 그 성계를 영향권 안(Inside)에 둔 것들의 항만 — 주인의 칸 항·방랑자 항·이웃(Adjacent) 항은 없다. 본체는 따로지만(PressureWithReach를 거치지 않는 제 고리) 같은 항의 함수(TranscendentTermOf — 닿는 정도 Inside)와 AddPressureTerm을 쓰므로 그 항들은 ComputePressureIn과 비트까지 같다. 범위 밖의 백성·성계와 성역은 0의 결과. 읽는 곳: 분쟁 단계의 소모, AI가 노릴 성계를 고르는 자리(P가 0인 성계만), 성계 패널의 소모 툴팁, 커맨드렛의 측정과 테스트(Sim.Fleet) — ComputePressureIn은 주인·방랑자를 원천으로 세므로 이 자리들이 쓰지 않는다

static float ComputeDrainPressureOf(const FGalaxyState&, const FCosmicStateIndex&, const FCosmicDataTables&, const FCosmicRulesDef&, int32 CivIndex); // 경제 단계가 그 백성의 Stability를 깎는 P(B4 — 헤더 B4 2.3절, 공개 static): 그 백성이 가진 정착된 성계(주인이 그 백성인 OwnedSystems) 가운데 지배 원천의 칸이 그 백성보다 높은 성계의 ComputePressureIn P의 최댓값, 없으면 0(범위 밖의 백성도 0). 틱 경계에서 읽으면 다음 틱 경제 단계의 P다 — 단계는 같은 닿음의 제 표를 만들어 쓰고(§9.4의 빠른 길), Sim.PressureDrainEdict가 둘이 같은 값을 내는지 본다. 측정(커맨드렛의 spirit 줄과 CSV의 DrainPressure)과 표현이 규칙의 사본을 갖지 않게 둔다

static float DrainLossOf(const FCosmicRulesDef&, const FCosmicDataTables&, const FCivilizationState& Civ, float Pressure); // 그 P에서 경제 단계가 한 틱에 깎는 Stability(B4): DT_Rules.SpiritualPressureStabilityDrain(0 아래·수가 아님은 0) × Pressure × 지닌 칙령의 곱(CosmicEdicts::PressureDrainMultiplierOf). 단계가 이 함수를 부른다(0에서 끊기 전의 값) — 커맨드렛의 spirit 줄도 이것 하나를 읽는다

// 규모 등급 (§1.3, §8.3). 파생값, 저장 안 함, 해시 밖. 기준 = 문명 Tier(T1 Billions — 지구 수십억, T2 HundredBillions, T3 Trillions, T4~5 Countless — 셈이 뜻을 잃는 곳), 보정 = Population(DT_Rules.PopulationBandThresholds: < [1] 한 단계 아래, < [0] Few, > [2] 한 단계 위, 상한 Countless). Presentation의 문명·성계 패널 '백성' 1줄과 UCosmicLogSubsystem의 {PopBand} 슬롯이 읽는다. Population 숫자·비율은 어디에도 내지 않는다(커맨드렛 CSV만 예외)

UFUNCTION(BlueprintPure) EPopulationBand GetPopulationBand(FName CivID) const;

// 전역 방송 게이트 (§6.1). 모든 Core Logic Subsystem의 Broadcast가 이 값을 본다. RunHeadless 중 false — 빨리 넘기기 RunLeap(B5)도 같은 꼴의 고리라 그 안의 틱을 알리지 않고 끝에 경계 하나만 낸다. 공백기의 틱 자체는 게이트를 닫지 않는다(보통의 틱이다)

bool IsBroadcastEnabled() const { return bBroadcastEnabled; }

void SetBroadcastEnabled(bool bEnabled) { bBroadcastEnabled = bEnabled; }

UPROPERTY(BlueprintAssignable) FOnSimTick OnSimTick;

UPROPERTY(BlueprintAssignable) FOnGalaxyGenerated OnGalaxyGenerated;

// 저장/로드에서 호출

void LoadState(const FGalaxyState& In);

uint32 ComputeStateHash() const; // 결정론 테스트용. 전부(앎·이야기 포함)를 해시한다 — 세계만은 ComputeWorldHash(). FName·숫자·태그만 — 모든 FText 제외 (§9.2). Calling(SuccessionKind·bReturnOffered·FLifeRecord.NameIndex·BirthArchetypeRow 포함)과 FCivilizationState.Notables(Role·NameIndex·FirstTick·LastTick·SourceRow) 포함. 반사로 모든 UPROPERTY를 걷는다 — 배열은 원소 수부터, 맵은 항목 수부터 키를 FNameLexicalLess 차례로(Ties처럼 값이 구조체여도), 태그 컨테이너는 명시 태그만 FNameLexicalLess 차례로(B3부터 — 저장된 차례가 아니라: 조약을 끊었다 다시 맺은 짝이 같은 짝과 같은 해시를 갖게. 특성·플래그·권능·상태의 컨테이너도 같다). 그래서 기본값인 새 필드 하나도 두 해시를 옮긴다. 규모 등급은 파생값이라 해시 밖

private:

void SimulateTick(); // 고정 순서(열 단계): ① 경제 → ② 연구 → ③ 외교 → ④ 분쟁 → ⑤ 위기 → ⑥ AI → ⑦ 건넘(TickCrossings) → ⑧ 플레이어 이동(Embodied이고 죽음이 기록되지 않았을 때만 — 문은 아래 '공백기') → ⑨ 앎(TickSight, ⑧과 같은 문) → ⑩ 역할 효과 해소(10번째·마지막 — 그 뒤의 SyncRngState와 함께 단계 가드 안, B5). (B6a, 헤더 B6a 2.3절) ⑩ 뒤·SyncRngState 앞, 같은 단계 가드 안에서 끝의 기록: State.Ending.Kind가 비었을 때만 CosmicEndings::EndingOf(위 '끝의 판정')를 부르고 그 답의 Kind가 서면 State.Ending에 그대로 적는다 — 단계가 아니고, 판에 한 번(첫 끝만 — 기록이 선 뒤에는 부르지도 않는다), 추첨 없음(세 난수 출처 어느 것도 뽑지 않는다), 줄 없음, 앎에 쓰지 않음. 틱 0(NewGalaxy가 만든 상태)에는 쓰지 않는다. State.Tick을 먼저 올린 뒤 단계를 돈다 — 첫 시뮬 틱은 1이고 틱 0은 NewGalaxy가 만든 상태다(틱 단계에서 생기는 사건은 틱 0을 갖지 않는다. 틱 밖 플레이어 입력은 Tick 0에도 일어날 수 있다 — §16 #35)

// 기다리는 카드의 자리(MVP ②, B1): ++Tick 전에 틱 첫머리의 단위 CosmicPetitions::StandingHeld(…, bTravelAtOrigin = true)를 지역 값으로 잡아 두고(이동 중인 방랑자는 떠나온 성계에 선 것으로 센다. 재위의 첫 틱이면 비운다 — IsFirstTickOfReign; 계승이 연 생애의 첫 틱도 — IsFirstTickOfLife, B5), ⑧이 돌려준 '이 틱에 도착했다'와 함께 ⑨의 끝에서 Present(…, 그 단위, bArrived)를 부른다. 방랑자 카드가 놓이면 그 성계의 주인을 NoteMet한다(주제 성계와 함께 — 격차 규칙은 NoteMet의 것). 저장도 멤버 상태도 아니다. 규칙은 docs/20 부록 '불변 규칙 전문'의 틱 순서 문단

// 위기 단계(TickHazards): Hazard 만료 효과 적용. TickHazards(Hazard 만료)·UCosmicEventSubsystem::Apply(이벤트 효과)·틱 밖 플레이어 입력(카드의 답 AnswerPetition의 효과, 양위 명령 Abdicate — B1)은 KillPlayer·DeposeRuler를 실행하지 않고 FCallingState.PendingRoleEffect(PendingEffect = 태그 Effect.KillPlayer 또는 Effect.DeposeRuler / PendingCause / PendingSourceRow, 틱당 최대 1건, 먼저 기록된 것 우선)에 기록만 한다 — 기록하는 자리는 하나다: 데이터 효과든 단계 코드든 UCosmicEventSubsystem::Apply를 거친다. 분쟁 단계(TickConflict)는 적지 않는다 — 수도와 마지막 정착된 성계를 앗지 않는다(B3). Calling.LifeState가 Embodied일 때만 적는다(공백기의 틱은 역할 효과를 적지 않는다 — 공백기의 Player는 계승의 Install 전까지 죽은 그릇을 품는다. 공백기는 B5). 출생 보호(생애 첫 BirthProtectionTicks)는 적는 자리에서 가린다 — 가려진 KillPlayer는 칸을 차지하지 않아 같은 틱의 뒤따르는 DeposeRuler가 그대로 적힌다. KillPlayer가 그릇에 닿는가는 세계 함수 CosmicHazards::VesselAtRiskOf 하나가 답한다(B5 — Apply와 커맨드렛의 측정이 함께 읽고, 출생 보호도 그 함수가 본다, 아래 세계 함수). 닿는 경우는 둘뿐이다: 문맥에 성계가 있으면 그 성계에 선 방랑자에게만(성계에 일어난 일은 통치자를 죽이지 않는다 — 통치자의 실패는 폐위다, §4.4), 문맥에 성계가 없으면 문맥 문명이 통치 문명일 때(그 문명의 절멸)나 문맥에 문명이 없을 때만. event_grace_rescue는 Apply가 아니라 열째 단계가 사망 파이프라인 앞에서 FireRow로 낸다(B5 밖 — §16 #147)

// 역할 효과 해소(10번째·마지막 단계, ResolvePendingRoleEffect): PendingRoleEffect가 있으면 UPlayerRoleSubsystem의 사망·폐위·양위 파이프라인(§4.4, §5.10 — 양위는 B1, 사망은 B5)을 실행한다. 공백기에는(B5) 이 단계가 카운트다운을 맡는다 — 틱마다 남은 도약을 하나 줄이고 0이 되는 틱에 계승한다(차례는 아래 ResolvePendingRoleEffect). 이 단계는 틱을 돌리지 않는다 — 공백기를 한 번에 넘기는 일은 틱 밖 입력의 RunLeap(위 — 컨트롤러의 AdvanceLeap이 부른다)이다. (B6a) 이 단계 바로 뒤에 끝의 기록이 선다(위 SimulateTick) — 이 틱의 죽음·계승·폐위가 끝난 뒤라 LifeState와 PendingLeapTicks(침묵과 bNoVessel)가 그 틱의 답이고, 업적과 칸은 AI 단계 끝에 이미 정해졌다. 틱 경계의 자동 저장(§9.1 — UCosmicSaveSubsystem의 OnSimTick 핸들러)은 이 단계와 SyncRngState가 끝난 뒤의 OnSimTick에서 찍는다. 입력 때의 저장(받아들인 답·새 판·종료)은 틱 밖이다

// 공백기(Calling.LifeState == Interregnum — B5): 공백기의 틱은 보통의 틱이다 — 경제부터 건넘까지 그대로 돌고, TickPlayerTravel(⑧)·TickSight(⑨)·플레이어 업적 평가·기다리는 카드의 제시는 Embodied일 때만이라 건너뛴다(이 배제는 지금 코드에 있다); 열째 단계는 카운트다운이다(위) (§4.4). 그릇이 없어 앎이 늘지 않는다 — NoteMet·NoteKnown도 불리지 않는다. 죽음이 기록된 틱도 같다(B5): SimulateTick은 ⑧·⑨(⑨ 끝의 카드 제시와 방랑자 카드의 NoteMet까지)를 LifeState == Embodied ∧ Calling.PendingEffect != Effect.KillPlayer로 가른다 — 열째 단계가 집행할 죽음이 적힌 그릇은 그 틱에 움직이지 않고(그 틱에 끝날 여정도 도착하지 않는다 — 떠나온 성계가 죽은 자리다) 보지 않으며 카드를 받지 않는다. 폐위의 기록(Effect.DeposeRuler)은 이 문을 닫지 않는다

// 이벤트·업적·영압의 자리: 열 단계와 SimulateTick의 호출 열 개는 그대로이고 네 단계 안의 순서를 고정한다. ① 경제 = 문명마다(Civs 배열 순서) 인구 회복(`PopulationRegrowthPerTick`) → 안정도 회복(`StabilityRecoveryPerTick` × (1 + `SocietyRecoveryBonus` × 사회 연구)·`StabilityRestingLevel`, MVP ③·B2) → 수입 계산(정착된 성계만 — 정착 중인 성계는 수입이 없다. 에너지 = 별 등급 합 × 칸 배수 × (1 + 물질 보너스 × 물질 쪽 윤리), 물질 = 풍부도 합 × 칸 배수, 통찰·영향력 = 인구 × 인구당 값 × (1 + 영성·개방 보너스 × 그 윤리) — 칸 배수는 통찰·영향력에 걸리지 않는다, 노브는 DT_Rules, MVP ⑧) → 틱당 원천(먼저 지닌 칙령의 `EffectsPerTick` — `ActiveEdicts` 차례, 주체는 그 백성뿐이고 성계·`OtherID` 없음, B2; 그다음 영압의 Stability 소모(`SpiritualPressureStabilityDrain` × 가장 큰 P × 지닌 칙령의 곱 `CosmicEdicts::PressureDrainMultiplierOf` — 백성마다 한 번, 곱이 1이면 비트 그대로, B4; 그 식은 `DrainLossOf` 하나이고 가장 큰 P의 규칙은 `ComputeDrainPressureOf`가 공개로 읽는다)와 PressureEffectsPerTick(칙령의 곱이 닿지 않는다). Phase 4부터 특성 행. 틱당 원천은 양을 고치는 효과만 건다 — 이 단계에는 추첨도 역할 효과의 기록도 없다, §7.5 Apply) → 비축 가산 → 칙령의 유지비(`ActiveEdicts`의 사본 차례로 `UpkeepPerTick`을 비축에서 모두 내거나, 내지 못한 칙령은 그 틱에 그친다 — 다스리는 백성이면 줄 `log_edict_lapsed` 하나: 이 단계의 유일한 줄이고 말하기 스트림만 쓴다, B2). 모든 문명을 다 돈 뒤 LowStabilityTicks와 FallenSphereTicks를 따로 한 바퀴 센다(FallenSphereTicks의 영향권 소속은 이 단계 첫머리에 만든 표를 그대로 읽는다). ② 연구 = 통찰 수입 × ResearchInsightShare(비축 한도 안)를 써서 분야마다 그 × 몫 × ResearchPerInsight만큼 오르고(MVP ⑧ — 노브), 돌파마다 줄을 내고, Society 돌파에는 그 줄 다음에 event_book_found를 이름으로 낸다(행의 조건이 책이 닫힌 백성을 고른다, §5.11 — 이름으로 부르는 행은 그 자리의 단계가 낸다). ⑤ 위기 = 거대 프로젝트와 해결 → 카운트다운과 만료 → 발생 → 이벤트 슬롯(UCosmicEventSubsystem::EvaluateAndFire). ⑥ AI = AI·섭정의 결정(백성마다 거대 프로젝트의 시작, 이어서 정착과 그 추첨, 이어서 칙령의 철회 — `CosmicEdicts::AIRevokesOf`, 추첨 없음, B2, 이어서 외교 — `TickAIDiplomacy`, 추첨 없음, B3) → 업적 판정(UAscensionSubsystem::EvaluateFeats, 틱당 정확히 1회). 귀결 셋: 그 틱의 LowStabilityTicks 셈에 드는 Stability 변화는 안정도 회복과 틱당 원천뿐이고 그 밖의 변화(이벤트·만료·해결·AI·틱 사이 입력)는 다음 틱의 셈에 처음 잡힌다 / 한 틱에 역할 효과를 적는 순서는 틱 사이 입력 → ② 연구의 이름 행(Civs 배열 순서) → ⑤ⓐ 대응 프로젝트의 완성이 건 해결 효과(Systems 배열 순서) → ⑤ 만료(Systems 배열 순서) → ⑤ 발생의 전조 행 → ⑤ 이벤트 슬롯(Civs 배열 순서) → ⑥ 업적의 대가(플레이어가 통치하는 백성 먼저, 그다음 Civs 배열 순서 — 지금 스키마는 업적의 대가에 역할 효과를 싣지 않는다)이다 / 위기 단계의 카운트다운 뒤에 놓인 Hazard(ⓑ 만료의 효과·주장 규칙, ⓒ 추첨·수도, ⓓ 이벤트 슬롯)는 그 틱에 카운트다운이 줄지 않는다 — 카운트다운을 먼저 줄이고 만료를 모은 뒤에 놓인다. 그보다 앞에서 놓인 것(② 연구의 이름 행, '거대 프로젝트와 승천 판정' 작업부터 ⓐ 해결의 효과 — ④ 분쟁은 Hazard를 놓지 않는다, B3)은 그 틱의 ⓑ에서 한 번 준다. 발화한 행의 효과는 큐 없이 그 자리에서 배열 순서로 적용한다(§7.5)
// 주장의 목소리와 소모의 줄(B4 — `[B-4d1]`부터): 위기 단계 만료의 주장 편입(`bJoined`)은 서술자의 만료 줄 → 기록 줄(B5 `[B-5d1]` — 아래 TickHazards ⓑ) → 징조 줄(셋째 칸 아래의 눈, `UCosmicLogSubsystem::PushWithSign`) → 1인칭 줄(그 초월자 행의 `ClaimTemplates`, 셋째 칸 이상의 눈) 차례로 말하기 스트림만 쓴다(`JoinClaimedSphere`가 편입시킨 초월자의 자리를 돌려준다 — `[B-4b]`). 분쟁 단계의 영향권 안 소모는 다스리는 백성의 함대가 그 목표의 첫 분쟁 단계에 닳은 틱에 줄 하나(`log_fleet_worn` — 말하기 스트림만). 세계 맵에 주장 줄의 키를 쓰지 않는다(결정 6)

// 외교와 분쟁의 자리(B3 — 헤더 docs/60_DECISIONS/headers/B3_diplomacy_and_conflict.md 2.5절~2.8절): ① 경제의 수입 계산에서 영향력에 지닌 무역 조약마다 `TradeInfluencePerTreaty`를 더한다(맺은 틱의 다음 틱부터). ③ 외교·④ 분쟁의 단계 안 차례는 아래 TickDiplomacy·TickConflict. ⑤ 위기의 두 고리 — 놓일 때의 함대 지연과 발생 몫의 역병 번짐 — 는 아래 TickHazards(추첨 수 그대로). ⑥ AI는 칙령의 철회 뒤에 외교를 한다(아래 TickCivAI). 이 가운데 추첨은 분쟁 단계의 공성의 끝(끝에 이른 공성마다 1회)뿐이다

void TickEconomy();

void TickResearch();

void TickDiplomacy(); // 외교 단계(셋째, B3 — 식은 헤더 docs/60_DECISIONS/headers/B3_diplomacy_and_conflict.md 2.2절·2.5절). 추첨 없음. 차례(고정): ⓐ 우호의 유지비 — 백성마다(Civs 차례) Ties에서 태세가 우호인 키를 사전 차례(FNameLexicalLess)로 모아, 자리(FriendlySlotsOf — FriendlySlotsByTier[칸])보다 많으면 뒤의 키부터 먼저 그치게 하고(칸은 내려가지 않지만 노브가 바뀐 세이브), 남은 우호마다 그 차례로 FriendlyUpkeepInfluence를 비축에서 낸다 — 내지 못한 우호는 중립으로 그친다(0보다 크고 유한한 유지비만 읽는다). 그친 우호마다 다스리는 백성이면 줄 log_stance_lapsed(이 단계의 유일한 줄, 말하기 스트림만), AI는 줄 없음 ⓑ 관계의 표류 — 백성 X마다(Civs 차례) 그 백성의 국경(두 백성의 정착된 성계가 같은 GalaxyRow에서 1레인 — CosmicDiplomacy::BordersOf, 지역 배열 하나를 X마다 다시 쓴다. 표류는 정착된 성계를 바꾸지 않아 단계 시작에 만든 국경표 BorderTableOf와 같은 답이다 — 헤더 B3 11절 항목 12의 속도 고침)을 걸으며 국경이 닿은 상대 Y마다(Civs 차례) X의 Relations[Y]를 목표값 CosmicDiplomacy::TargetRelationOf(X, Y)(윤리 거리·Y가 X를 대하는 태세·두 백성의 조약 수만 읽는다 — 관계를 읽지 않아 한 쌍의 쓰기가 다른 쌍에 닿지 않는다) 쪽으로 RelationDriftRate만큼 옮기고 [−1, 1]로 자른다(표류율은 (0, 1] 밖이면 0 — 그때 표류는 돌지 않는다). 값이 바뀌지 않으면 항목을 만들지 않는다(표류율 0이면 아무 항목도 생기지 않는다). 국경이 닿지 않은 쌍과 초월자 키는 움직이지 않는다. 소리 없는 표류는 만남을 낳지 않는다(§8.1). 무역 조약의 영향력은 경제 단계의 수입에서 더한다(위 '외교와 분쟁의 자리')

void TickConflict(); // 분쟁 단계(넷째, B3 — 식과 사유의 차례는 헤더 B3 2.3절·2.6절). 함대는 유닛이 아니라 백성의 수 FleetPower와 목표 성계 하나(FleetTargetSystemID·FleetTargetSinceTick)다 — 목표가 제 정착된 성계면 지킴, 남의 정착된 성계면 공성, 없으면 고루 지킨다. 차례(고정): ⓐ 함대 — 백성마다(Civs 차례) 에너지 유지비(FleetUpkeepEnergyPerPower × FleetPower)를 모두 내면 FleetGrowthPerTick × 지닌 칙령의 FleetGrowthMultiplier 곱만큼 자라고, 못 내면 아무것도 내지 않고 FleetGrowthPerTick만큼 준다. 상한 CosmicConflict::FleetCapOf(FleetPerSettledByTier[칸] × 정착된 성계 수 — 첫째 칸 0)로 자르고, 목표 성계가 그 백성보다 높은 칸의 봉인되지 않은 초월자의 영향권 안이면 소모 PressureFleetAttrition × P(P = ComputeAttritionPressureIn — 그 초월자들의 항만)를 뺀다. FleetPower는 0 아래로 내려가지 않는다(비교로). 상한도 함대도 없는 백성은 건너뛴다(데이터를 끈 판의 모든 백성, 첫째 칸의 백성) ⓑ 끝에 이른 공성 — 목표의 주인이 다른 살아 있는 백성이고 Tick ≥ FleetTargetSinceTick + Max(1, SiegeTicks)인 공성을 공격하는 쪽의 Civs 차례로 모으고(문을 보지 않는다), 모은 공성마다 시뮬 수 1회를 전부 먼저 뽑는다(§9.2 ⓑ — 뽑는 수는 목표·시작 틱·SiegeTicks만 따른다) ⓒ 목표의 점검 — 목표가 있는 백성마다 제 성계면 CosmicConflict::GuardBlockOf, 남의 성계면 SiegeBlockOf가 None이 아니면 목표를 풀고 FleetTargetSinceTick = Tick(쉼의 시작). 모은 공성이었으면 그 수는 버린다 ⓓ 끝의 적용 — 남은 모은 공성마다 적용 직전에 SiegeBlockOf를 다시 묻고(앞의 넘김이 뒤의 문을 바꿀 수 있다 — None이 아니면 ⓒ처럼 풀린다, 함대 손실 없음), None이면 CosmicConflict::Takes(수, SiegeOddsOf) — 수 < 몫(동률은 지키는 쪽) — 일 때 넘어간다: MakeEvalContext의 문맥으로 CosmicHazards::Transfer, 곧 FollowUpKnowing(줄 전에), 그리고 정복한 백성과 국경이 닿은 모든 백성(진 백성 포함 — 넘김 직후에 묻는다)이 그 백성을 보는 눈 −ConquestRelationPenalty(문을 지난 넘김을 Transfer가 거절하면 에러 한 줄과 함께 지켜 낸 것으로 읽는다 — 문이 Transfer의 거절을 모두 앞서 묻으므로 놀이에서는 닿지 않는다). 지켜 내면 공격한 함대 × SiegeRepulsedFleetFactor(CosmicConflict::RepulsedShareOf). 어느 쪽이든 공격한 쪽의 태세를 중립으로 되돌리고(휴전) 목표를 풀며 FleetTargetSinceTick = Tick(쉼의 시작 — 아래 TickCivAI). 줄은 다스리는 백성이 당사자일 때만, 상대를 먼저 NoteMet한 뒤 낸다(풀림 log_siege_lifted Severity 1, 끝은 쪽마다 다른 행 Severity 2 — 행은 헤더 B3 3.4절). 목표의 정리는 단계 밖에서도 한다: CosmicHazards::Destroy·Transfer가 그 성계를 함대 목표로 둔 모든 백성의 목표를 풀고 FleetTargetSinceTick = Tick(줄 없음) — 그래서 이 단계가 만나는 목표는 늘 주인이 있다. 분쟁은 수도와 마지막 정착된 성계를 앗지 않는다(SiegeBlockOf의 Capital·LastSettled, Transfer의 거절) — 역할 효과를 적지 않고 백성을 없애지 않는다. 한 성계에는 공성 하나(Besieged)라 한 틱에 한 번만 넘어가고, 넘어간 성계는 같은 틱의 위기 발생 추첨에 새 주인으로 든다(수는 그대로). 함대가 0이면(데이터를 끈 판) 공성이 서지 않아 뽑지 않는다. 측정을 켠 판(FCosmicStageClock.bOn)이면 끝낸 공성(ⓒ·ⓓ의 풀림과 ⓓ의 넘김·지킴)을 그 시계의 SiegeEnds에 적는다 — 세계에 닿지 않는다

// 세계 함수(공개, static이 아닌 이름공간 — 상태·색인·테이블을 받고 줄을 내지 않는다): CosmicHazards::Place·Resolve·Transfer·MoveCapital·Destroy·EmptyHazardSlot·LetSettlementsGo, 물음 ClaimantOf·IsLastUnclaimed·IsLastSettledSystem·IsDeadNode·VesselAtRiskOf(B5)·KillsAVesselOf(B5)·ClaimRuleHoldsOf(B6a — 주장 규칙의 결정적인 셋, TryPlaceClaim과 JoinClaimedSphere가 부른다)·ClaimMayFollowOf(B6a — 화면의 답, 만료 앞의 문까지)·RebellionWatchOf(B6a — FCosmicRebellionWatch; 상수 LowStabilityThreshold는 경제 단계가 센다) / CosmicMegaprojects::Start·StartBlockOf·CanStart·Choices·Advance·HasCompleted·RequiredProgressOf·CostPerTickOf·CanPay·RealmCapOf·IsSettling·MaySettle·CanSettle·SettleBlockOf·SettleTargets·PeopleSettleBlockOf·Settle, 읽기만 하는 ReadSlot·PaidNothing(패널의 '멈춤'과 커맨드렛 — 어느 단계도 부르지 않는다) / CosmicState::IsSettled·OwnsAnySystem·PickAIProfile·LaneDistancesFrom·SightWalkOf(B1) / CosmicWanderer::IsTravelling·IsEmbodiedWanderer·PlaceIndexOf·StandingIndexOf·LaneTicksOf·TravelBlockOf·IsReturn·CrownBlockOf(방랑자의 문 — B1, 아래) / CosmicEdicts::SlotsOf·HeldCountOf·EnactBlockOf·HeldAnswerOf·AnswersStanding·IsAnswerEdict·SpawnMultiplierOf·CountdownBonusOf·HasStabilityCost·StabilityDrainOf·CostOf·TryPay·Enact·Revoke·AIRevokesOf·InheritedDrainOutrunsRecovery(칙령의 규칙 — B2, 아래) / CosmicDiplomacy::TieOf·StanceOf·HasTreaty·TreatyCountOf·TradeTreatyCountOf·FriendlyCountOf·FriendlySlotsOf·SetStanceOf·Sign·Break·ForgetCiv·Borders·BordersOf·BorderTableOf·EthicsDistanceOf·TargetRelationOf·ViewOf·ViewBandOf·TreatyAcceptBonusOf·TreatyBarOf·TreatyCostOf·StanceBlockOf·TreatyBlockOf·TurnHostile·LowerRelation·StanceNameOf·TreatyNameOf(외교의 규칙 — B3, 아래) / CosmicConflict::SettledCountOf·OwnedCountOf·FleetCapOf·FleetGrowthMultiplierOf·EffectivePowerOf·AttackGapOf·BesiegerOf·IsBesieging·SiegeBlockOf·GuardBlockOf·SiegeNameOf·GuardDelayTicksOf·RepulsedShareOf·SiegeOddsOf·Takes·IsSiegeDue·SiegeEndTickOf·OddsBandOf·FleetDelayOf·LiftTargetsOn·AIAimOf(분쟁의 규칙 — B3, AIAimOf는 B5, 아래 — 소모의 압력은 이 이름공간이 아니라 위 ComputeAttritionPressureIn) / CosmicSuccession::CandidatesOf·PickOf·LeapOf(계승의 단일 창구 — B5, 아래) / CosmicState::IsSanctuary·IsFirstHome과 상수 CosmicIDs::Sanctuary·FirstHome(ID만 보는 순수 함수 — Core/CosmicIDs.h, §4.5 ①. `Law.Hedge`를 지키는 자리: `SettleBlockOf`의 첫 사유 `Sanctuary`·`SettleTargets`·`Place`·`Transfer`·`Destroy`·압력 본체·`Install`·`BecomeWanderer`·`TravelBlockOf`의 사유 `Sanctuary`와 여덟째 단계의 도착(B1)·`SiegeBlockOf`·`GuardBlockOf`의 첫 사유 `Sanctuary`(B3)·`LoadGalaxy`) (Sim/CosmicHazards.h·CosmicMegaprojects.h·CosmicStateIndex.h·CosmicWanderer.h·CosmicEdicts.h·CosmicDiplomacy.h·CosmicConflict.h·CosmicSuccession.h(B5) — 새 물음을 만들기 전에 여기서 찾는다. 종료 리뷰)
// B4(헤더 B4 2.3절·2.4절·11절 항목 16·18): `CosmicEdicts::PressureDrainMultiplierOf(Civ, Tables)`(칙령의 소모 곱 — 지닌 칙령의 `PressureDrainMultiplier`를 `ActiveEdicts` 차례로 곱하고, 0보다 크고 유한하지 않은 값은 1로 읽는다), `CosmicConflict::ResistBlockOf(State, Index, Tables, Rules, int32 CivIndex, int32 SystemIndex, const FGameplayTag& Tag)`(함대로 시간을 사는 문 하나 — 효과 `DelayHazard`와 카드 선택의 문이 이것만 읽는다. `enum class ECosmicResistBlock : uint8 { None, NoHazard, NotOwn, Capital, NoFleet, Spent }` — UENUM이 아니다: `NoHazard`(그런 성계가 없거나 그 성계에 그 태그의 위기가 없다 — 조건처럼 `MatchesTag`, 빈 태그도) → `NotOwn`(그런 백성이 없거나 그 백성의 성계가 아니다) → `Capital`(그 위기 행이 수도 규칙 행 — 반란; 태그 이름을 읽지 않는다) → `NoFleet`(함대 상한 0 — 첫째 칸, `GuardBlockOf`의 `NoFleet`과 같은 뜻) → `Spent`(상한은 있으나 함대 0 — 공성·소모·못 낸 유지비·앞선 저항). 격차를 묻지 않는다)·`ResistShareOf(Rules)`(남는 함대의 몫 — `ResistFleetFactor`가 [0, 1]이면 그 값, 아니면 1, 비교로)·`int64 ResistTicksOf(float Value)`(더할 틱 — 정수 부분, 2^31에서 자르고, 1 아래·무한·수가 아님은 0이라 효과가 거절한다; 효과와 카드 창의 아래 줄이 같은 수를 읽는다 — `[B-4c]`)·`int32 FirstFleetRungOf(Rules, EAscensionTier From)`(From 이상에서 `FleetPerSettledByTier`가 함대를 주는 첫 칸 — `FleetCapOf`와 같은 시험(0보다 크고 유한), `EAscensionTier` 인덱스, 없으면 INDEX_NONE; 함대의 짧은 말 `CosmicUI::FleetSuffix`가 읽어 UI가 규칙을 베끼지 않는다 — `[B-4d2]`), `CosmicPetitions::ChoiceGateOf(State, Index, Tables, Rules, int32 CivIndex, int32 SystemIndex, FName OtherCivID, const FEncounterChoice&)`(카드 선택의 문 하나 — 규칙 13. 돌려주는 `struct FCosmicChoiceGate { ECosmicChoiceGateKind Kind; ECosmicStanceBlock Stance; ECosmicEdictBlock Edict; ECosmicResistBlock Resist; int32 EffectAt = INDEX_NONE; bool IsOpen() const }` — `enum class ECosmicChoiceGateKind : uint8 { None, Stance, Edict, Resist }`(저장되지 않는다), 그 꼴의 사유 하나, `EffectAt` = 거절한 효과의 `OnSuccess` 자리(카드 창이 그 칙령 행과 위기 태그를 읽는다 — `[B-4c]`), 받으면 None과 INDEX_NONE. 선택의 `OnSuccess`를 차례로 보아 첫 거절: `SetStance`는 `StanceBlockOfChoice`의 규칙(`Same`은 받아들인다), `EnactEdict`는 Key의 `EnactBlockOf`(`Held`도 거절 — 아무것도 바꾸지 않을 답이 수를 뽑고 '내렸다'고 말하지 않게), `DelayHazard`는 Tag의 `ResistBlockOf`. 문이 있는 효과가 없는 선택은 늘 열려 있다. 답(`AnswerPetition` — 서 있는 통치자 카드에 수를 뽑기 전에, 에러 한 줄 'the choice's gate refuses choice N')·정책(`CosmicRulerPolicy::ChoiceOf` — 규칙과 테이블을 함께 받을 때)·카드 창(회색과 사유 — 아래 §7.5 '카드 창의 문')이 이것 하나를 읽는다), `UGalaxySimulationSubsystem::ComputeDrainPressureOf`·`DrainLossOf`(위 ComputeAttritionPressureIn 곁)

// B5(헤더 B5 2.4절·5.1절 — docs/60_DECISIONS/headers/B5_death_succession_two_books.md): `CosmicHazards::VesselAtRiskOf(const FPlayerEntityState& Player, const FCallingState& Calling, const FCosmicRulesDef& Rules, int64 Tick, FName SystemID, FName CivID)`(KillPlayer가 그 주어에서 그릇에 닿는가 — Apply가 기록하는 규칙에서 '슬롯이 비었나'만 뺀 것: Calling.LifeState가 Embodied일 때만; 성계 주어(SystemID가 None이 아님)는 그 성계에 선 방랑자에게 — 성계에 일어난 일은 통치자를 죽이지 않는다; 성계 없이 CivID만 있으면 그 백성의 통치자에게(그 백성의 절멸); 둘 다 없으면 어디서나. Tick − Calling.BornTick < BirthProtectionTicks면 닿지 않는다. Apply의 KillPlayer와 커맨드렛의 '죽음 기회'가 이것을 읽는다 — 규칙을 베끼지 않는다. 판정은 옮기기 전과 비트 그대로), `CosmicConflict::AIAimOf(State, Index, Tables, Rules, int32 CivIndex, bool bLiftHeld = false)`(AI 단계의 겨눔 — 아래 TickCivAI의 '이어서 외교' ③의 적대의 문과 표적 고르기를 그대로 옮긴 순수 함수, 추첨·쓰기 없음. 돌려주는 `struct FCosmicAIAim { int32 Other = INDEX_NONE; int32 System = INDEX_NONE; }` — 적대를 열 백성과 공성할 그 성계의 인덱스, 겨누지 않으면 둘 다 INDEX_NONE. AI 단계는 bLiftHeld = false로 묻고 그 답대로 적대를 열어 공성한다(비트 그대로). bLiftHeld = true는 측정 전용 — SiegeBlockOf의 두 사유 Capital·LastSettled만 연다: 커맨드렛이 틱 경계의 상태로 두 번 물어, 연 답만 다스리는 백성의 성계를 겨누는 (공격자, 다스리는 백성)을 '수도·마지막 성계만으로 막힌 경우'로 센다 — §16 #126), `CosmicHazards::KillsAVesselOf(const FCosmicDataTables& Tables, const FGameplayTag& Tag)`(B5: 그 태그의 DT_Hazards 행이 만료 효과 — OnExpireEffects나 갈래 쪽 OnExpireBranchEffects — 에 KillPlayer를 싣는가; 행이 없는 태그는 거짓. 계승 후보의 거름이 읽고, 치명 위기의 경고·정지(B5 `[B-5c]` — 만료의 틱에 묻는 `CosmicAutoPause::LethalPlaceOf`)도 같은 답을 읽는다), `CosmicSuccession::CandidatesOf(State, Index, Tables, Rules, int32 DeathSystemIndex)`(B5 — Sim/CosmicSuccession.h, 상태·추첨 없음, 규칙 13의 단일 창구: `struct FCosmicSuccessionCandidates { TArray<int32> Systems; int32 Relaxed = 0; }` — 계승 후보(Systems 배열 순서의 인덱스)와 푼 거름의 수(0 없음, 1 위기, 2 위기와 영향권)를 돌려준다. 후보 = DeathSystemIndex의 성계(Succeed가 죽은 그릇의 자리 Player.CurrentSystemID로 찾는다)와 같은 GalaxyRow의 정착된 성계(CosmicState::IsSettled — 성역은 주인이 없어 저절로 빠진다)에서 그릇이 죽은 성계와 치명 위기(KillsAVesselOf)가 선 성계를 뺀 것 — 이 두 거름은 풀지 않는다. 거름 둘 — 봉인되지 않은 타락 초월자(Allegiance Realm.Abyssal)의 영향권 안(InfluencedSystems — SphereReachIn의 Inside와 같은 시험), 남은 카운트다운이 SuccessionMinHazardTicks 미만인 위기 — 은 후보가 비면 위기 → 영향권 차례로 풀고, 그래도 비면 후보 0(침묵). DeathSystemIndex가 상태에 없으면 후보 0. 무게는 모두 같다(계승 가중 노브 다섯은 예약 — 읽지 않는다). 세계만 읽고 앎을 읽지 않는다 — 규칙 2), `CosmicSuccession::PickOf(float Number, int32 Count)`(같은 무게의 후보 Count 가운데 수가 고르는 차례 — min(⌊Number × Count⌋, Count − 1); 0 아래·숫자 아님은 첫째, 1 이상은 마지막(1 바로 아래의 수로 읽는다), Count ≤ 0이면 INDEX_NONE), `CosmicSuccession::LeapOf(const FCosmicRulesDef& Rules, int64 LeapTicksTotal, float Number)`(죽음의 도약 폭 — 식은 §7.2 LeapTicksTotal; 노브가 무엇이든 수는 그 하나다)

// 한 번의 보기(B1, Sim/CosmicStateIndex.h): struct CosmicState::FCosmicSightWalk { TArray<int32> Found; // 찾은 차례의 성계 인덱스 — 눈의 자리가 먼저, 준 차례로  TArray<int32> LanesFrom; // State.Systems의 원소마다 가장 가까운 자리에서의 레인 수(자리는 0, 닿지 않으면 INDEX_NONE) } · CosmicState::FCosmicSightWalk CosmicState::SightWalkOf(const FGalaxyState&, const FCosmicStateIndex&, TConstArrayView<int32> EyePlaces) — 눈의 자리 모두를 준 차례로 0단에 놓고(범위 밖이나 두 번 준 인덱스는 건너뛴다) Lanes 배열 차례로 너비 우선. 레인은 떠나는 성계와 같은 GalaxyRow의 성계로만 따라가고 상태에 없는 SystemID로 가는 레인은 건너뛴다. 보는 거리로 자르지 않는다 — 부르는 쪽(앎 단계·스캔, LookFrom)이 제 거리로 자르고 LanesFrom으로 만남의 차례를 정한다. 보기가 성역을 지나는가는 §16 #80(단계 5 — 지금 성역에는 레인이 없다). 세계만 읽고 추첨 없이 한 번 돈다

// 방랑자의 문(B1, Sim/CosmicWanderer.h — 순수 함수 이름공간, 상태·줄·추첨 없음. 명령과 패널이 같은 답을 읽는 한 창구. 문이 쓰는 앎(만났는가·카드가 기다리는가)은 인자로 받는다: 명령은 State.Knowing에서(입력의 정의), 패널은 FCosmicKnownView에서(표현 계층이 앎을 읽는 곳은 KnownViewOf 하나 — 불변 규칙 14))
// - enum class ETravelBlock : uint8 { None, NotWanderer, Travelling, CardWaiting, Sanctuary, NotNeighbour, Handler }  TravelTo의 첫 거절 사유(None = 받는다). 저장되지 않는다 — 패널이 이 차례로 첫 사유를 말한다(그릇의 세 상태가 먼저, 대상에 관한 사유 가운데 성역이 먼저). Handler = 핸들러 안의 부름(에러 한 줄)
// - enum class ECrownBlock : uint8 { None, NotWanderer, Travelling, CardWaiting, NotStanding, NotSettled, Unmet, ThroneLocked, Lacking, Handler }  즉위·복귀의 첫 거절 사유. NotStanding = 그릇이 그 백성의 성계에 서 있지 않다, NotSettled = 서 있는 성계가 정착 중이다, Lacking = 경외도 지도자 공백도 아니다(즉위만 — 복귀는 둘 다 보지 않는다). 저장되지 않는다
// - bool IsTravelling(const FGalaxyState&)  Player.TravelTargetSystemID != NAME_None — 모든 문·카드·거둬짐이 이 시험 하나를 읽는다
// - bool IsEmbodiedWanderer(const FGalaxyState&)  몸을 입은(Embodied) 방랑자
// - int32 PlaceIndexOf(const FGalaxyState&, const FCosmicStateIndex&)  몸을 입은 방랑자가 선 것으로 세는 성계의 인덱스 — 이동 중이면 떠나온 성계(Player.CurrentSystemID). 통치자·공백기·상태에 없는 ID는 INDEX_NONE
// - int32 StandingIndexOf(const FGalaxyState&, const FCosmicStateIndex&)  이동 중이 아닌 몸을 입은 방랑자가 선 성계의 인덱스, 그 밖은 INDEX_NONE
// - int32 LaneTicksOf(const FCosmicRulesDef&)  레인 하나의 틱 = DT_Rules.LaneTravelTicks, 1 아래는 1
// - ETravelBlock TravelBlockOf(const FGalaxyState&, const FCosmicStateIndex&, FName TargetSystemID, bool bCardWaiting)  TravelTo의 문: 몸을 입은 방랑자, 이동 중 아님, 기다리는 카드 없음, 대상이 성역 아님, 선 성계의 살아 있는 레인 이웃(그 Lanes에 있고 같은 GalaxyRow이고 죽은 노드가 아니며 선 성계 자신이 아니다). 앎을 읽지 않는다 — 방랑자의 레인 이웃은 늘 아는 성계다(BecomeWanderer의 새 눈, 그리고 모든 보기가 한 레인에 닿는다)
// - bool IsReturn(const FGalaxyState&, FName CivID)  CivID가 그릇이 떠나온 백성(Player.FormerCivID)인가 — 즉위가 복귀인가
// - ECrownBlock CrownBlockOf(const FGalaxyState&, const FCosmicStateIndex&, const FCosmicRulesDef&, FName CivID, bool bMet, bool bCardWaiting)  BecomeRuler의 문(CivID = 그릇이 선 성계의 주인): 몸을 입은 방랑자, 이동 중 아님, 기다리는 카드 없음, 그 백성의 정착된 성계에 서 있다, bMet, Tick >= 그 백성의 ThroneLockedUntilTick(떠날 때 모든 백성에게 적힌다), 그리고 복귀가 아니면 ReverenceToPlayer >= DT_Rules.CoronationReverence 또는 Stability < CoronationStabilityBelow(숫자가 아닌 값은 어느 쪽도 통과하지 않는다). 설계는 §5.10(즉위·복귀·잠금), 결정의 근거는 헤더 B1의 2.7절
// 칙령의 규칙(B2, Sim/CosmicEdicts.h — 헤더 docs/60_DECISIONS/headers/B2_edicts_and_research.md 2.2절. 순수 함수 이름공간, 상태·줄·추첨 없음. 명령·경제·위기·AI 단계·칙령 창·성계 패널·상단 바·커맨드렛의 정책이 같은 답을 읽는 한 창구. 해시에 드는 값은 비교와 + − ×뿐이고, 곱과 합의 차례는 저장되고 해시되는 ActiveEdicts의 차례다):
// - enum class ECosmicEdictBlock : uint8 { None, NotRuler, NoRow, Held, Tier, Conditions, Slots, Cost, Handler }  제정의 첫 거절 사유. UENUM이 아니다 — 저장·해시·데이터 칸이 아니다. EnactBlockOf는 NoRow부터 Cost까지를 이 차례로 묻고 첫 사유를 낸다: NoRow = DT_Edicts의 행이 아니다(빈 이름·범위 밖의 백성 인덱스도), Held = 이미 지닌다, Tier = 백성의 칸 < RequiredTier, Conditions = EnactConditions가 서지 않는다(주체는 그 백성과 그 수도), Slots = HeldCountOf ≥ SlotsOf, Cost = 비축이 EnactCost를 다 내지 못한다. NotRuler(몸을 입은 통치자의 다스리는 백성이 없다)와 Handler(핸들러 안의 부름 — 에러 한 줄)는 명령 EnactEdict만 낸다
// - int32 SlotsOf(Civ, Rules)  1 + DT_Rules.EdictSlotSocietyMarks 가운데 그 백성의 사회 연구(Research.FindRef("Society"))가 닿은(≥) 문턱의 수. 수가 아닌 문턱은 닿지 않는다
// - int32 HeldCountOf(Civ, Tables)  지닌 칙령 가운데 테이블에 있는 것의 수 — 없는 이름은 효과·유지비·슬롯이 없다
// - ECosmicEdictBlock EnactBlockOf(State, Index, Tables, Rules, CivIndex, Row)  위의 차례. 칙령 창의 회색 단추와 그 사유도 이 함수 하나를 읽는다
// - FName HeldAnswerOf(Civ, Hazard, Tables)  위기 행의 RulerResolutions 차례로, 그 백성이 지닌 첫 DT_Edicts 행(없으면 NAME_None — 프로젝트 이름은 건너뛴다). 만료 때의 해결(§7.4 위기 단계 ⓑ)과 패널의 칙령 줄이 읽는다
// - bool AnswersStanding(State, Index, Tables, CivIndex, Row)  Row가 그 백성의 정착된 성계에 지금 선 위기 행의 RulerResolutions에 있는가(정책·화면) · bool IsAnswerEdict(Tables, Row)  어느 위기 행의 RulerResolutions에 든 칙령 행인가
// - float SpawnMultiplierOf(Civ, Tables, Tag)  지닌 칙령의 HazardSpawnMultipliers[Tag]를 ActiveEdicts 차례로 곱한 값(없으면 1. 0 아래·유한하지 않은 값은 0 — 아키타입 곱의 가드와 같은 비교) · int32 CountdownBonusOf(Civ, Tables, Tag)  0보다 큰 HazardCountdownBonus[Tag]만 int64로 더해 0~100,000으로 자른 값(부동소수에서 정수로 바꾸는 일이 없다)
// - bool HasStabilityCost(Row)  EffectsPerTick에 0 아래의 ChangeStability가 있는가(AI의 철회·정책·첫 안내가 읽는다) · float StabilityDrainOf(Row)  유한한 ChangeStability 값 전부의 합의 음수(값과 되돌림이 함께 있으면 서로 덜어진다) — 순 소모
// - FResourcePool CostOf(Cost)  0보다 크고 유한한 멤버만, 그 밖은 0 · bool TryPay(Stockpile, Cost)  CostOf로 읽은 값을 모두 낼 수 있으면 내고 참, 아니면 바뀜 없이 거짓
// - void Enact(Civ, Row, Def)  EnactCost를 내고 ActiveEdicts 끝에 더한다 — 문(EnactBlockOf)은 부르는 쪽이 먼저 본다 · bool Revoke(Civ, Row)  차례를 지키는 제거(RemoveAt — RemoveSwap이 아니다), 지니지 않았으면 거짓. 명령·경제 단계의 그침·AI 단계가 함께 쓴다
// - TArray<FName> AIRevokesOf(Civ, Tables, Rules)  AI 단계가 철회할 행들, ActiveEdicts 차례로: 안정도 값이 있는 칙령은 Stability < AIEdictRevokeStability일 때(노브가 수가 아니면 그 종류는 철회하지 않는다), 그 밖의 칙령(위기의 답·edict_vigil)은 곧. 테이블에 없는 이름은 그대로 둔다
// - bool InheritedDrainOutrunsRecovery(Rules, Tables)  물려받은 칙령의 불변식: StartingEdict가 비면 참(볼 것이 없다), 행이 없거나 순 소모(StabilityDrainOf)가 0 이하면 거짓(실패 끝이 없다), 그 밖은 StabilityRecoveryPerTick × (1 + SocietyRecoveryBonus) < 순 소모(노브는 경제 단계가 읽는 대로 읽는다). Data.Integrity가 게임 데이터에 부른다
// 외교의 규칙(B3, Sim/CosmicDiplomacy.h — 헤더 docs/60_DECISIONS/headers/B3_diplomacy_and_conflict.md 2.2절. 순수 함수 이름공간, 상태·줄·추첨 없음. 명령·외교와 AI 단계·효과 SetStance·칙령 창의 [백성] 탭·커맨드렛의 정책이 같은 답을 읽는 한 창구 — 불변 규칙 13. 만났는가는 부르는 쪽의 인자 bMet이다: 세계는 앎을 읽지 않는다. 해시에 드는 값은 비교와 + − × ÷와 절댓값뿐):
// - enum class ECosmicStanceBlock : uint8 { None, NotRuler, NoCiv, Self, NotMet, Same, FriendlySlots, Tier, NonAggression, Handler }  태세의 첫 거절 사유. UENUM이 아니다 — 저장·해시·데이터 칸이 아니다. StanceBlockOf는 NoCiv부터 NonAggression까지를 이 차례로 묻는다: NoCiv = 살아 있는 백성이 아니다(받지 않는 태그 — 태세 셋 밖 — 도 NoCiv, 헤더 B3 11절 항목 3), Self, NotMet(bMet이 거짓), Same(이미 그 태세), FriendlySlots(우호 — 자리가 찼다), Tier(적대 — 둘째 칸 아래: 첫 10분에 전쟁이 없다), NonAggression(적대 — 불가침을 먼저 끊어야). NotRuler와 Handler(에러 한 줄)는 명령만 낸다
// - enum class ECosmicTreatyBlock : uint8 { None, NotRuler, NoCiv, Self, NotMet, Held, Tier, Hostile, Refused, Cost, Handler }  조약의 첫 거절 사유(TreatyBlockOf의 차례): NoCiv(조약 둘 밖의 태그도), Self, NotMet, Held(이미 맺었다), Tier(제안하는 쪽이 둘째 칸 아래), Hostile(어느 쪽이든 상대에게 적대), Refused(그 백성이 제안한 백성을 보는 눈 ViewOf < TreatyBarOf), Cost(비축의 영향력 < TreatyCostOf). NotRuler·Handler는 명령만
// - const FCosmicTie* TieOf(const FCivilizationState&, FName OtherID)  없으면 nullptr(중립·조약 없음) · FGameplayTag StanceOf(Civ, OtherID)  없으면 빈 태그 · bool HasTreaty(Civ, OtherID, FGameplayTag Treaty) · int32 TreatyCountOf(Civ, OtherID) · int32 TradeTreatyCountOf(Civ)  모든 상대와의 무역 조약 수(경제 단계의 수입) · int32 FriendlyCountOf(Civ) · int32 FriendlySlotsOf(Civ, const FCosmicRulesDef&)  FriendlySlotsByTier[칸](배열 밖 0, 0 아래 0)
// - void SetStanceOf(FCivilizationState&, FName OtherID, FGameplayTag Stance) · void Sign(FGalaxyState&, int32 A, int32 B, FGameplayTag Treaty) · bool Break(FGalaxyState&, int32 A, int32 B, FGameplayTag Treaty)(맺지 않았으면 거짓) · void ForgetCiv(FGalaxyState&, FName CivID)  쓰는 함수 넷 — 정규형을 지키고(기본값이 된 항목을 지운다) 조약은 두 백성에 함께 쓴다(대칭의 하나뿐인 길). ForgetCiv는 건넘 단계의 정리가 모든 백성의 Ties에서 그 키를 지운다
// - bool Borders(const FGalaxyState&, const FCosmicStateIndex&, int32 A, int32 B)  국경: A의 정착된 성계 하나와 B의 정착된 성계 하나가 같은 GalaxyRow에서 1레인(CosmicState::IsSettled — 불변 규칙 13의 인접 술어. 성역은 무주라 빠진다) · void BordersOf(State, Index, int32 A, TArray<bool>& Out)  모든 B에 한 번에 — Out[B](State.Civs의 원소마다, A 자신은 거짓): A의 정착된 성계를 한 번 걷는다(외교 단계의 표류와 AI 단계의 적대의 문, 헤더 B3 11절 항목 12) · TArray<TArray<int32>> BorderTableOf(State, Index)  백성마다(Civs 차례) 국경이 닿은 백성들(Civs 차례)
// - float EthicsDistanceOf(const FEthicsAxes&, const FEthicsAxes&)  (|Δ평화| + |Δ영성| + |Δ개방|) ÷ 3, 0~2 · float TargetRelationOf(State, Rules, int32 Viewer, int32 Other)  표류의 목표값 = −RelationEthicsWeight × 거리 + (Other가 Viewer를 대하는 태세의 항 RelationStanceTerms[우호·적대]) + RelationTreatyTerm × 두 백성의 조약 수(수가 아닌 노브는 0). 관계를 읽지 않는다
// - float ViewOf(State, Index, Rules, int32 Viewer, int32 Other)  보는 눈 하나: 두 백성의 국경이 닿았으면 저장된 Relations[Other], 아니면 태세의 항을 뺀 목표값(윤리의 항 + 조약의 항) — 국경 밖 쌍은 표류하지 않으므로 기본값 0이나 곧바로 읽는 태세의 항이 문턱을 넘지 못하게 한다(우호를 걸었다 놓아 조약을 사는 길을 막는다). 조약의 승낙(TreatyBlockOf의 Refused), [백성] 탭의 낱말 띠와 툴팁, 외교 정책, 커맨드렛의 측정이 이것을 읽고, AI의 적대의 문은 국경이 닿은 상대만 보므로 저장된 관계를 그대로 읽는다(같은 값) · int32 ViewBandOf(float View)  낱말 띠 다섯 — 0 적의(≤ −0.5) · 1 냉담(≤ −0.2) · 2 무심(< 0.2) · 3 호의(< 0.5) · 4 신뢰, 수가 아니면 2. 경계는 상수(노브가 아니다)
// - float TreatyAcceptBonusOf(Civ, const FCosmicDataTables&)  지닌 칙령의 TreatyAcceptBonus 합(0보다 크고 유한한 값만) · float TreatyBarOf(Civ, Tables, Rules)  승낙의 문턱 = TreatyAcceptRelation(수가 아니면 0) − TreatyAcceptBonusOf — TreatyBlockOf가 묻고 [백성] 탭이 말하는 같은 산술 · float TreatyCostOf(Rules)  TreatyCostInfluence(0보다 크고 유한할 때만, 그 밖은 0)
// - ECosmicStanceBlock StanceBlockOf(State, Index, Rules, int32 CivIndex, FName OtherID, FGameplayTag Stance, bool bMet) · ECosmicTreatyBlock TreatyBlockOf(State, Index, Tables, Rules, int32 CivIndex, FName OtherID, FGameplayTag Treaty, bool bMet)  위의 차례. 명령·효과·창·정책이 이 둘만 읽는다
// - bool TurnHostile(FGalaxyState&, int32 CivIndex, int32 Other, float Penalty)  적대를 걸고 그 쌍의 무역 조약을 두 쪽에서 끊는다 — 끊었으면 Other가 CivIndex를 보는 눈을 Penalty만큼 곧 낮추고 참(문은 부르는 쪽이 먼저 본다). 적대가 서는 자리 셋이 이 하나를 지난다: 통치자의 SetStance(BetrayalRelationPenalty), AI가 적대를 여는 자리와 효과 SetStance(0 — 값 없음). 적대 중인 쌍은 TreatyBlockOf의 Hostile로 다시 맺지 못한다
// - void LowerRelation(FCivilizationState& Viewer, FName OtherID, float Amount)  Viewer의 눈을 곧 Amount만큼 낮춘다([−1, 1] — 0보다 크고 유한한 양만, 아니면 항목도 쓰지 않는다): 정복의 눈(ConquestRelationPenalty)과 배신의 눈(BetrayalRelationPenalty) · FText StanceNameOf(FGameplayTag)·TreatyNameOf(FGameplayTag)  ST_CA Stance.Friendly·Stance.Hostile·Stance.Neutral(빈 태그)·Treaty.Trade·Treaty.NonAggression — 줄의 슬롯 {Stance}·{Treaty}
// - 코드가 이름으로 부르는 태그(태세 둘·조약 둘·Hazard.Plague — B3; Log.Claim — B4; Effect.KillPlayer·Reign.Died·Realm.Abyssal — B5 `[B-5b]`; B5 `[B-5c]`의 열 — 다이제스트가 읽지 않는 로그 범주 Log.Calling·Log.Record, 왕좌 항목의 자리 Notable.Ruler·Notable.Regent, 왕좌를 앗는 위기 Hazard.Rebellion, 비문의 사인 Death.Hazard·Death.Encounter·Death.Age, 재위 끝 쪽의 까닭 Reign.Abdicated·Reign.Deposed; 엔딩 기록의 종류 Ending.RuledPeople·Ending.Vessel·Ending.OtherPeople·Ending.NoCandidate·Ending.Clock — B6a `[B-6a2]`, 지금은 테스트만 부른다)는 Core/CosmicTags.h의 함수 하나씩이다(CosmicTags::StanceFriendly()… — RequestGameplayTag(FName), 불변 규칙 9. 상수 헤더라 check_layers.py의 태그 글자 셈에 들지 않는다)
// 분쟁의 규칙(B3, Sim/CosmicConflict.h — 헤더 B3 2.3절. 순수 함수 이름공간, 상태·줄·추첨 없음. 분쟁·AI 단계·명령 AllocateFleet·CosmicHazards::Place·성계 패널·커맨드렛의 정책이 같은 답을 읽는 한 창구. 함대는 유닛이 아니라 FleetPower 하나와 목표 성계 하나다):
// - enum class ECosmicSiegeBlock : uint8 { None, NotRuler, Sanctuary, NoSystem, Dead, Unowned, Own, NotSettled, Capital, LastSettled, NotAdjacent, Besieged, RealmCap, NoFleet, NonAggression, NotHostile, Handler }  공성의 첫 거절 사유 — 구조의 사유가 먼저, 태세가 마지막이라 첫 사유가 길을 바로 알린다('먼저 적대해야 한다'는 다른 길이 모두 열렸을 때만). UENUM이 아니다. SiegeBlockOf의 차례: Sanctuary(불변 규칙 13의 첫 거절) → NoSystem(성계나 백성이 없다) → Dead → Unowned → Own(제 성계 — 그것은 지킴) → NotSettled(정착 중인 성계) → Capital(주인의 수도 — 분쟁은 앗지 않는다) → LastSettled(주인의 마지막 정착된 성계 — CosmicHazards::IsLastSettledSystem) → NotAdjacent(이 백성의 정착된 성계에서 같은 GalaxyRow로 1레인이 아니다) → Besieged(다른 백성의 공성이 이미 서 있다) → RealmCap(소유 성계 수 ≥ RealmCapOf(칸) — 정복도 보유 상한을 센다, §16 #42) → NoFleet(그 주인을 상대로 한 유효 전력이 0 — 두 칸 이상 높은 주인도) → NonAggression → NotHostile(그 주인을 적대하지 않는다 — bAssumeHostile이면 묻지 않는다). NotRuler·Handler는 명령만. 노릴 성계는 늘 제 정착된 성계의 레인 이웃이라 첫 눈 뒤 아는 성계다 — 명령은 아는 성계를 묻지 않는다
// - enum class ECosmicGuardBlock : uint8 { None, Sanctuary, NoSystem, Dead, NotOwn, NotSettled, NoFleet }  지킴의 첫 거절 사유(GuardBlockOf의 차례). NoFleet = 함대의 상한이 0(첫째 칸에는 지킬 함대가 없다 — 쓸모없는 입력이 세계 필드를 바꾸지 않게) · ECosmicSiegeBlock SiegeNameOf(ECosmicGuardBlock)  AllocateFleet이 지킴의 사유를 공성의 이름으로 답한다(NotOwn은 NoSystem)
// - int32 SettledCountOf(State, Index, int32 CivIndex) · int32 OwnedCountOf(State, Index, int32 CivIndex)  정착된 성계의 수 / 정착 중인 것까지의 소유 수(보유 상한이 읽는 수) — OwnedSystems 가운데 주인이 그 백성인 항목만
// - float FleetCapOf(State, Index, int32 CivIndex, Rules)  FleetPerSettledByTier[칸] × SettledCountOf(배열 밖·0 이하·수가 아님 = 0). 성계를 잃은 백성의 함대를 줄이고 같은 칸의 큰 백성과 작은 백성을 가른다 · float FleetGrowthMultiplierOf(Civ, Tables)  지닌 칙령의 FleetGrowthMultiplier를 ActiveEdicts 차례로 곱한 값(0보다 크고 유한하지 않은 곱은 1)
// - float EffectivePowerOf(float Power, int32 TierGap, Rules)  Power × FleetEffectivenessByTierGap[Clamp(격차, 0, 2)] — 격차는 상대가 더 높은 만큼만(아래로는 0). 0보다 크지 않은 Power, 0 이상·유한이 아닌 곱은 0. 공격하는 쪽에만 걸린다 — 지키는 쪽은 제 성계에서 받지 않는다(앞선 칸이 늘 이기는 눈덩이를 막는다) · int32 AttackGapOf(State, Index, int32 Attacker, int32 SystemIndex)  그 성계의 주인이 Attacker보다 높은 칸 수(아니면 0)
// - int32 BesiegerOf(State, Index, int32 SystemIndex)  그 성계를 에워싼 백성(목표가 그 성계이고 주인이 아닌 Civs 차례의 첫 백성), 없으면 INDEX_NONE · bool IsBesieging(State, Index, int32 CivIndex)  그 백성의 목표가 다른 백성의 성계인가
// - ECosmicSiegeBlock SiegeBlockOf(State, Index, Rules, int32 Attacker, int32 SystemIndex, bool bAssumeHostile = false) · ECosmicGuardBlock GuardBlockOf(State, Index, Rules, int32 CivIndex, int32 SystemIndex)  위의 차례. 목표 없음(NAME_None)은 늘 받는다(명령) — 문을 묻지 않는다
// - float SiegeOddsOf(State, Index, Rules, int32 Attacker, int32 SystemIndex)  공격하는 쪽의 몫: A = 그 주인을 상대로 한 유효 전력, D = 주인의 FleetPower × SiegeDefenceShares[주인의 목표가 그 성계 0 · 목표 없음 1 · 다른 성계 2]. A ≤ 0(또는 수가 아님)이면 0, D ≤ 0이면 1, 그 밖에 A ÷ (A + D) · bool Takes(float Number, float Odds)  Number < Odds(동률은 지키는 쪽) · int32 OddsBandOf(float OursOdds)  화면의 띠 — 2 우세(≥ 0.6) · 1 팽팽 · 0 열세(≤ 0.4), 상수. 화면은 늘 다스리는 백성 쪽에서 본다(지킬 때는 1 − 몫)
// - bool IsSiegeDue(Civ, int64 Tick, Rules)  Tick ≥ FleetTargetSinceTick + Max(1, SiegeTicks) — 이르렀거나 지났다(노브가 줄어든 세이브의 공성도 다음 분쟁 단계에서 끝난다) · int64 SiegeEndTickOf(Civ, Rules)  그 끝의 틱(패널의 남은 시간)
// - int32 FleetDelayOf(State, Index, Tables, Rules, int32 SystemIndex, const FHazardDef& Row)  놓일 때의 지연(위기 단계 — CosmicHazards::Place가 슬롯을 채우기 전에 읽는다): 그 성계 주인의 함대 목표가 그 성계이고 Row가 Capital 규칙의 행(반란)이 아닐 때만 — 제 백성의 불만을 힘으로 미루지 않는다 —, GuardDelayTicksOf(주인, 격차)(주장은 그 주장자의 칸과의 격차 — CosmicHazards::ClaimantOf, 그 밖의 위기는 0), 그 밖은 0 · int32 GuardDelayTicksOf(const FCivilizationState& Owner, int32 Gap, Rules)  FleetDelayPerPower × EffectivePowerOf(Owner.FleetPower, Gap)을 비교로 [0, Min(FleetDelayCap, 100000)] 안에 둔 뒤 int32로(음수·수가 아님 = 0) — FleetDelayOf의 산술이고 패널의 지연 줄(격차 0)이 같은 함수다
// - float RepulsedShareOf(Rules)  서지 못한 공격 함대가 남기는 몫 = SiegeRepulsedFleetFactor([0, 1] 밖은 0 — 분쟁 단계와 [함대를 보낸다]의 툴팁) · void LiftTargetsOn(FGalaxyState&, FName SystemID)  그 성계를 목표로 둔 모든 백성의 목표를 비우고 FleetTargetSinceTick = Tick(CosmicHazards::Transfer와 Destroy가 부른다 — 줄 없음)
void TickHazards(); // 발생 추첨은 정착된 성계만 돈다 — 무주 성계와 정착 중인 성계는 뽑지 않고 건너뛴다(추첨 수는 소유를 따르고 시드에 대해 고정이다). Hazard.Claim도 정착된 성계에만 선다 (§5.7). Hazard.Extinction의 만료는 그 문명의 성계를 놓게 할 뿐 Civs 배열을 줄이지 않는다 — 제거는 그 틱의 건넘 단계의 정리가 한다(플레이어가 통치하는 문명만 다음 틱의 정리로 미룬다 — 아래 '건넘 단계'). 만료 효과, Hazard.Rebellion 지속 틱 조건 (§4.1). KillPlayer·DeposeRuler는 PendingRoleEffect 기록만. 만료 처리 끝에 Hazard.Claim 발생 규칙(결정론: 타락 영향권 인접 — 조공은 주인 블록 없음(#137), Law.Remnant, MaxInfluencedSystems 상한)과 Claim 만료 = InfluencedSystems 편입 (§4.2 영혼 경제, 틱 순서 불변) 단계 안의 순서(고정): ⓐ 거대 프로젝트와 해결(AdvanceMegaprojects) — 모든 성계의 대응 칸(Systems 배열 순서), 그다음 모든 성계의 건설 칸(같은 순서)을 내고 진척시키고, 완성된 프로젝트를 기록하고 칸을 비우고, 그 성계의 Hazard 행이 든 프로젝트면 그 Hazard를 해결한다(답이 짓는 일보다 먼저 비축을 쓴다 — 정착과 문명 제거 작업의 리뷰. 값을 다 치르지 못한 대응 칸이 기다리는 자원을 그 값 아래로 남길 같은 백성의 건설 칸은 그 틱에 내지 않는다 — 종료 리뷰와 그 재감사). 카운트다운보다 먼저라 마지막 틱에 완성된 해결 수단도 성계를 구한다. 프로젝트 칸 하나가 '세계 → 받은 성계의 앎 → 완성 줄 → 해결 줄'이다(§5.7). 행이 더는 들고 있지 않은 대응 프로젝트는 결과 없이 끝나고, 정착 중인 성계의 건설 칸(`project_settle`)도 다른 건설 행처럼 돈다(그 행이 테이블에 없으면 칸이 그대로 남는다 — 정착과 문명 제거 작업). ⓐ는 제 추첨이 없다 — 해결이 거는 효과(`OnResolveEffects`)와 그 뒤의 행은 Apply대로 뽑는다(`NameNotable` 1회 따위) ⓑ 카운트다운과 만료 — 두 번에 나눠 한다: 모든 카운트다운을 줄이며 0이 된 성계를 Systems 배열 순서로 모으고, 그다음 모은 순서대로 만료시킨다. 모은 만료마다 시뮬 추첨 2회(결과의 수, 주장의 수)를 만료를 돌기 전에 전부 먼저 뽑는다(행이 없는 태그도 뽑는다). 만료 하나는 '세계 → 받은 성계의 앎 → 만료 줄 → 뒤따르는 줄'이다(원인이 먼저 읽힌다. 받은 성계의 앎은 세계 쪽의 코드 훅보다도 먼저라 그 만남의 줄은 슬롯에 아직 만료하는 태그가 있을 때 나간다 — 재감사): 앞선 만료가 그 슬롯을 비웠거나 바꿨으면(같은 태그로 다시 놓았어도 — 새로 놓인 것은 카운트가 1 이상이다) 건너뛴다 → 행이 없는 태그는 결과도 줄도 없이 비운다(해결처럼 — 반란이면 지도자와 불만의 셈도 끝난다) → 주인이 그 행의 `RulerResolutions`에 든 칙령을 지니면(`CosmicEdicts::HeldAnswerOf`) 만료 대신 해결한다 — `CosmicHazards::Resolve`(해결 효과와 해결 줄), FollowUpKnowing, FollowUpNotes. 뽑아 둔 두 수는 버리고 주장 규칙은 돌지 않는다(B2) → 성계를 앗아 가는 만료(초신성 전조)가 주인의 마지막 정착된 성계에 닿으면(정착 중인 성계는 셈하지 않는다 — 그 정착은 성계와 함께 놓인다, §5.7) 먼저 마지막 성계의 문을 묻는다(아래 건넘 단계, §4.5 — 문이 열리면 그 만료는 줄 없이 슬롯만 비운다) → OnExpireEffects 또는 갈래(UCosmicEventSubsystem::ApplyAll) → 이웃 효과 → 받은 성계의 앎(FollowUpKnowing — 효과가 통치 문명에 넘긴 성계를 알고 준 백성을 만난다. 아래의 은혜 줄과 만료 줄보다 먼저다 — 아는 범위와 로그 작업과 그 리뷰) → 태그의 코드 훅(파괴·왕좌 교체·편입. 효과가 그 성계를 주인의 마지막 정착된 성계로 만들었으면 — 데이터는 그럴 수 없다: 만료 효과도, 이웃 효과가 부르는 해결 효과도 TransferSystem을 싣지 않는다, Data.Integrity — 파괴 앞에서 문을 다시 묻고 에러 한 줄. 그때 지켜진 성계는 만료 효과와 이웃 효과를 이미 받았다: 그 효과가 통치 문명에 넘긴 성계는 은혜 줄보다 먼저 알고, 그 효과가 남긴 Hazard의 줄은 은혜 줄 다음에 나간다) → 슬롯 비움 → 이 만료가 비운 슬롯에 Hazard.Claim 발생 규칙((B6a, 헤더 B6a 2.11절) 그 규칙이 주장을 놓은 바로 뒤의 알아챔 플래그: 그 성계가 몸을 입은 그릇이 다스리는 백성의 정착된 성계이고 그 백성이 셋째 칸 이상이며 Calling.PendingEffect가 비었으면, 주장자 초월자의 bAwareOfPlayer가 거짓일 때 참으로 쓴다 — 초월자마다 한 번이고 계승을 넘어 남는다. 세계만 읽는다(Calling.LifeState·PendingEffect·Player.Role·RuledCivID·그 성계의 주인·그 백성의 Tier — 앎을 읽지 않는다), 추첨 없음. TryPlaceClaim은 플래그를 세운 주장자의 번호를 돌려준다) → 만료 줄(주장이 아무것도 편입하지 못했으면 해결 줄) → 기록 줄(B5 `[B-5d1]`, 결정 11 — 돌이킬 수 없는 잃음 하나에 한 줄: 정착되어 있던 성계가 파괴되어 죽은 노드가 되었으면 log_record_destroyed, 주장이 편입했으면 log_record_annexed; Log.Record 등급 1, {System}만, 초점은 그 성계와 그것을 잃은 주인, 범위와 상관없이 bUntold, 말하기 1수·시뮬 0수. 건넘 단계의 정리는 기록하지 않는다 — 그 백성의 마지막 성계를 앗은 만료가 이미 적었다) → 편입의 목소리(징조 → 1인칭, B4 — 위 '주장의 목소리와 소모의 줄') → 뒤따르는 줄(새로 선 Hazard의 발생 줄과 전조 행, 해결 줄) → 알아챔 줄((B6a — `[B-6a5]`부터) 이 만료의 주장 규칙이 플래그를 세웠고 그 쓰기 전에 어느 초월자도 알아채지 않았을 때만 — 판에 한 줄. 그 만료의 마지막 줄이고, 그 초월자 행의 AwarenessTemplates에서 말하기 스트림 한 수(시뮬 0수), Log.Transcendent·Severity 1, 주제 백성은 다스리는 백성·주제 성계 없음, 보는 칸 바닥 셋째) ⓒ 발생 — 정착된 성계마다 슬롯이 비었든 찼든 시뮬 추첨 정확히 1회를 Systems 배열 순서로 먼저 전부 뽑고(행의 몫 = (Chance × 아키타입 곱) × 주인이 지닌 칙령의 `HazardSpawnMultipliers` 곱 — 몫만 바뀌고 추첨 수는 그대로, B2. 역병(`Hazard.Plague`) 행의 몫은 그 성계가 '레인 이웃에 역병이 서 있는 정착된 성계'의 표에 들면 × `PlagueSpreadMultiplier` — 역병의 번짐, B3. 표는 ⓒ의 시작에 한 번 만들어 이 고리에서 방금 놓인 역병을 읽지 않는다. 추첨 수는 그대로), 그다음 그 수가 정한 발생을 같은 순서로 놓는다(찬 슬롯과 정착 중인 성계에는 서지 않는다). 그다음 결정론 발생(반란)을 Civs 배열 순서로 본다. 그다음 첫 위기(MVP ③ — `Tick == DT_Rules.FirstCrisisTick`인 틱에만, Civs 배열 순서로 모든 백성의 수도에 `FirstCrisisHazard`의 Draw 행을 놓는다. 찬 슬롯은 건너뛴다. 추첨 없음) ⓓ 이벤트 슬롯 — UCosmicEventSubsystem::EvaluateAndFire. Hazard를 슬롯에 놓는 길은 하나다(추첨·결정론 발생·SpawnHazard 효과·인접 전파가 같은 함수를 거친다 — 카운트다운은 `Max(1, CountdownTicks)` + 그 성계 주인이 지닌 칙령의 `HazardCountdownBonus` 합(`CosmicEdicts::CountdownBonusOf` — 0~100,000, B2) + 함대의 지연(`CosmicConflict::FleetDelayOf` — 주인의 함대 목표가 그 성계일 때만, 수도 규칙의 행(반란)은 빼고, 0~Min(`FleetDelayCap`, 100,000), B3)을 int64로 더해 `MAX_int32`에서 자른 값이다. 놓일 때만 붙고 서 있는 Hazard의 카운트다운은 바뀌지 않는다 — 함대를 옮겨도 그대로다. 예외는 카드의 답 효과 DelayHazard 하나다 — B4, §7.3 효과 표). 그 길이 발생 줄과 그 Hazard의 발생 전조 행(crisis_rebellion의 전조 ②)을 낸다 — 효과가 놓은 반란도 전조 ②를 받는다. 효과가 세계를 바꾼 뒤 따라야 하는 줄은 문맥의 후속 처리 기록(FCosmicEvalContext.Notes — 세계 함수 CosmicHazards::Place·Resolve가 적는다)과 멤버 FollowUpNotes가 잇는다(위기 처리 작업). 받은 성계의 기록은 FollowUpKnowing이 줄보다 먼저 따른다(아는 범위와 로그 작업, §7.5)

// 인물 훅 ② (§1.3, §7.2, §7.5): crisis_rebellion 발생 처리가 전조 ② 행을 직접 Fire하고 그 효과 NameNotable(Notable.Rebel)이 만든다(데이터). 만료 처리에서 먼저 그 문명의 열린 Notable.Ruler·Notable.Regent 기록을 Apply 가드 ③과 같은 규칙으로 닫고(LastTick = max(Tick, 1), §7.5 — 왕좌가 바뀌었다. 양위 뒤에 반란이 이기면 섭정이 아니라 반란의 우두머리가 다스린다 — B1), 그다음 열린 Notable.Rebel 기록의 Role을 Notable.Ruler로 바꾼다(AI 문명·플레이어 옛 문명 동일, '정권 교체' 로그 {Ruler}). 플레이어 문명이면 그 만료의 효과를 적용한 직후 PendingEffect가 Effect.DeposeRuler이고 PendingSourceRow가 crisis_rebellion일 때만 전환한다(이 만료가 적은 DeposeRuler가 기록된 것일 때). 그때 그 행의 만료 줄('새 다스리는 이가 섰다')이 나간다. 다른 원천이 역할 효과를 먼저 적었으면(양위의 명령과 왕좌 카드의 셋째 선택지, 왕좌 카드의 판정 실패 — B1. 통치자에게 닿는 사망을 적는 행은 없다 — 게임의 사망 하나(AI 각성의 만료, B5 `[B-5d2]`)는 성계 주어라 그 성계에 선 방랑자에게만 닿는다) 먼저 적힌 원천이 이긴다: 왕좌는 반란에게 가지 않고, 열린 Notable.Rebel 기록을 닫으며(LastTick = max(Tick, 1)), 만료 줄은 위기 행의 ExpireTemplates 대신 왕좌를 말하지 않는 이름 행 log_rebellion_unseated(위기 행의 Severity, 말하기 추첨 1회 — 수는 같다)다. 그 뒤 열째 단계가 적힌 효과를 처리한다(B1, `CosmicRows::LogRebellionUnseated`). AI 문명이면 새 AIProfile을 만료의 결과 수가 고른다(CosmicState::PickAIProfile — 네 프로필 균등). 해결 처리에서 열린 Rebel 기록을 닫는다(LastTick = max(Tick, 1)). 넘김(TransferSystem)으로 비는 반란과 행이 없는 태그의 만료도 해결처럼 닫는다. 반란이 슬롯을 떠나면(만료·해결·넘김) 그 문명의 LowStabilityTicks = 0. Hazard.Extinction의 만료로 성계를 모두 잃은 문명은 그 틱의 건넘 단계(정리)에서 제거되고(플레이어가 통치하던 문명만 다음 틱) 명부도 함께 사라진다 — 애도 문장이 통치자 이름을 한 번 부른다. 인물 훅 ③은 UAscensionSubsystem의 AI 간이 판정(T3 도달 → Notable.Ascendant, §7.5), 훅 ①은 UPlayerRoleSubsystem::BecomeWanderer(Regent, §7.5). 인물 추첨의 난수는 문맥이 건넨 시뮬 스트림이다(§7.5)

void TickCivAI(); // AI 백성(플레이어가 통치하지 않는 백성)마다 Civs 배열 순서로: CosmicMegaprojects::Choices(SinceTick 0)를 모두 시작한다 — 대응 프로젝트와 업적의 건설 프로젝트, 추첨 없음(§4.3, §5.7, 거대 프로젝트와 승천 판정 작업). 다만 그 백성의 AIProfile이 DT_Rules.ClaimLeaveProfiles에 들면 Hazard.Claim이 선 성계의 답(구호)은 시작하지 않는다(MVP ③, §16 #71 — Choices 자체는 그대로라 통치자의 프로젝트 키는 이 노브를 받지 않는다). 이어서 확장은 정착(project_settle, §5.7 — 정착과 문명 제거 작업)이다: MaySettle(정착된 성계가 있고, 소유 성계 수 < RealmCapOf(Tier) — §5.12, 진행 중인 정착이 없다)이면 시뮬 스트림에서 정확히 1회 뽑고(대상을 보기 전에 — 대상이 없으면 그 수를 버린다, §9.2), SettleTargets(Systems 배열 순서 — 1레인·같은 GalaxyRow의 무주 성계, 봉인되지 않은 초월자의 영향권 안이면 Tier >= 3, §4.2 ④. 성역은 대상이 아니다)에서 그 수로 하나를 골라 Settle한다. 시작할 수 없는 백성은 뽑지 않는다. 소유 성계가 0인 문명은 프로젝트도 정착도 하지 않는다(정착된 성계가 없다). 이어서 칙령의 철회(B2 — 헤더 B2 2.6절): 그 백성이 지닌 칙령 가운데 CosmicEdicts::AIRevokesOf가 고른 행을 ActiveEdicts 차례로 철회한다 — 안정도 값이 있는 칙령은 Stability < DT_Rules.AIEdictRevokeStability(1 — 곧, 첫 AI 단계에서)일 때, 그 밖의 칙령은 곧. 추첨·줄 없음. B2의 AI는 칙령을 내리지 않는다(까닭은 헤더 B2 2.6절 — AI의 칙령 사용은 B7). 섭정 아래의 옛 백성도 여기 든다(통치자가 남긴 칙령과 즉위·복귀 때의 뜻은 `docs/10_DESIGN/systems.md` §5.8 'AI의 칙령'). 이어서 외교(B3 — 문의 전부는 헤더 B3 2.8절. 추첨 없음, 프로필과 앎을 읽지 않는다): ① 우호를 놓고 제 성계를 목표로 둔 지킴을 물린다(FleetTargetSinceTick = Tick — AI는 우호도 지킴도 쓰지 않는다. 섭정이 물려받은 것도. 통치자가 남긴 공성은 ②의 규칙대로 이어 간다) ② 함대 목표가 그 상대의 성계가 아닌 적대를 중립으로 푼다 — AI의 적대는 공성과 한 몸이다 ③ 적대가 없고 쉬는 중이 아니며(목표 없음 ∧ Tick ≥ FleetTargetSinceTick + SiegeRestTicks) 함대가 있고(FleetPower > 0) 평화 축 < AIHostilePacifismMax이면 적대의 문 — 국경이 닿고(그 자리에서 묻는다 — CosmicDiplomacy::BordersOf로 그 AI마다 한 번) 불가침이 없고, 그 백성이 상대를 보는 눈 ≤ AIHostileRelation(국경이 닿은 상대라 ViewOf는 저장된 관계다), 그 백성의 FleetPower ≥ AIHostilePowerRatio × 상대의 FleetPower — 상대가 다스리는 백성이면 두 문턱은 AIHostileRelationTowardRuled·AIHostilePowerRatioTowardRuled(헤더 B3 11절 항목 12) —, 다른 어느 AI도 그 상대에게 적대하지 않고(몰매 막기 — 태세로 본다) 다스리는 백성이 그 상대를 에워싸고 있지 않으며, 그 상대의 성계 가운데 SiegeBlockOf(…, bAssumeHostile = true)가 None이고 ComputeAttritionPressureIn의 P가 0인 것이 있다(소모가 공성이 끝나기 전에 함대를 비우는 곳은 노리지 않는다) — 을 지나는 Civs 차례의 첫 상대에게 적대를 연다. 보유 상한에 닿은 AI는 상대를 훑지 않는다(SiegeBlockOf의 RealmCap이 모든 성계에서 서므로 같은 답 — 속도만, 헤더 B3 11절 항목 12). 이 문과 고르기(③)는 세계 함수 CosmicConflict::AIAimOf(…, bLiftHeld = false) 하나다(B5 — 판정은 비트 그대로 옮겼다; 위 세계 함수). 열면 그 쌍의 무역 조약을 값 없이 끊고(CosmicDiplomacy::TurnHostile) 그 첫 성계(Systems 차례)를 함대 목표로 정해(FleetTargetSinceTick = Tick) 공성이 곧 선다 — 그 상대가 다스리는 백성이면 먼저 그 AI를 NoteMet하고, 무역 조약이 끊겼으면 줄 log_treaty_broken, 그다음 log_siege_on_us(Severity 2). AI는 조약을 제안하지 않고 불가침을 끊지 않는다(무역 조약은 적대가 서면 끊긴다 — AI의 조약은 B7). 단계의 끝에서 UAscensionSubsystem::EvaluateFeats를 틱당 한 번 부른다(§7.5)

void TickAIDiplomacy(int32 CivIndex); // 위 TickCivAI의 '이어서 외교'(B3) — TickCivAI가 AI 백성마다 칙령의 철회 바로 뒤에 부른다. 추첨 없음, 프로필과 앎을 읽지 않는다(다스리는 백성을 그 AI가 노릴 때의 NoteMet은 앎에 쓰기만 한다)

void TickCrossings(); // 건넘 단계(7번째). Civs의 원소 수가 바뀌는 유일한 자리: ⓞ 정리(소유 성계가 0인 문명 제거) → ⓐ 도착 → ⓑ 출발. 정착과 문명 제거 작업부터 ⓞ, ⓐⓑ는 Phase 6a (아래 '건넘 단계')

bool TickPlayerTravel(); // 여덟째 단계(Embodied일 때만 — 문은 위 '공백기', B1): 이동의 남은 틱을 하나 줄이고, 0이 되는 틱에 목표가 그 틱에도 선 성계의 살아 있는 레인 이웃(그 Lanes에 있고 같은 GalaxyRow, 죽은 노드·성역이 아니다 — 가는 길에 파괴된 성계는 레인에서 빠진다)이면 그릇을 옮기고, 아니면 이동만 끝낸다. 어느 쪽이든 두 이동 필드를 함께 비운다(남은 틱 없이 목표만 있는 상태도 비운다). 줄·추첨 없음, 앎을 읽지 않는다. '이 틱에 도착했다'를 돌려준다 — SimulateTick의 지역 값이고 앎 단계 끝의 도착 카드가 읽는다(파괴가 그릇을 옮긴 것은 도착이 아니다). 끝에서 Role == Ruler면 현재 생애의 열린 재위 ReignTicks를 1 올린다 (§7.2). 공백기 중에는 이 단계 전체를 건너뛴다

void TickSight(bool bVesselMoved); // 앎 단계(9번째, Embodied일 때만 — 문은 위 '공백기'). 세계를 읽고 Knowing과 이야기만 쓴다 — 시뮬 스트림 추첨 0회. SightIntervalTicks마다, 그리고 그릇이 움직인 틱(bVesselMoved — 틱 첫머리의 CurrentSystemID와 이 단계 직전의 값이 다르다: 여행의 도착도, 파괴가 그릇을 옮긴 것도. SimulateTick의 지역 값)에 한 번 더 돈다. 공백기 중에는 건너뛴다. 본문은 아는 범위와 로그 작업 (아래 '앎 단계'). 한 번의 보기는 LookFrom이다(B1에서 옮겼다 — 동작 그대로)

void ResolvePendingRoleEffect(); // 역할 효과 해소(단계 가드 안 — B5, 헤더 B5 2.2절)의 차례 (§4.4, §5.10): (0) LifeState == Interregnum이면(공백기에는 기록이 생기지 않는다 — Apply가 거절) PendingLeapTicks > 0일 때 하나 줄이고 0이 되면 UPlayerRoleSubsystem::Succeed, 그 밖(침묵)은 아무것도 하지 않는다(B6a — 이 단계 뒤의 끝의 기록이 침묵을 Ending.NoCandidate로 적는다 — 위 SimulateTick) — 끝 (1) Calling.PendingEffect가 Effect.DeposeRuler면 원인으로 가른다 — PendingCause가 Reign.Abdicated면 UPlayerRoleSubsystem::AbdicateRuler(양위 파이프라인, B1), 그 밖은 DeposeRuler(폐위 파이프라인) (2) Effect.KillPlayer면 UPlayerRoleSubsystem::Die(사망 파이프라인, B5) (3) 기록을 비운다. 이 단계는 틱을 돌리지 않는다(빨리 넘기기는 틱 밖 입력의 RunLeap). Die: (통치자인 그릇이 성역에 서 있으면 — 어느 규칙도 만들지 않는 상태, Install이 막는다 — 무엇을 바꾸거나 뽑기 전에 에러 한 줄로 거절한다) 생애를 닫는다(DiedTick = Tick, CauseOfDeath = PendingCause, CauseSourceRow = PendingSourceRow) → 죽음 줄(`[B-5d1]` — 바꾸는 일보다 먼저: 그릇은 아직 몸을 입었고(통치자면 아직 다스린다) 죽음의 수는 아직 뽑지 않았다, 폐위가 몰락을 먼저 말하는 꼴. 사인의 행 — Death.Hazard는 CosmicRows::LogDeathHazard, 제 행이 없는 사인(측정의 Death.Age)은 줄도 말하기 수도 없다; Log.Calling 등급 2, 초점 없음, {Player}는 닫힌 생애의 이름(CosmicChronicle::VesselNameOf), {Hazard}는 CauseSourceRow 위기 행의 DisplayName(표에 없으면 빈 글); PushRow — 말하기 1수) → 통치자였다면 왕좌를 놓는다(섭정 성향 1수 → UPlayerRoleSubsystem::ReleaseThrone(비공개 — BecomeWanderer의 앞 절반: Reign.Died, 섭정 이름 1수 — 원천 행은 PendingSourceRow, 새 눈 없음) → 모든 백성의 왕좌 잠금(Tick + ThroneLockTicks, 제 값보다 이르지 않게)) → 모든 백성의 ReverenceToPlayer = 0(계승 추첨보다 먼저) → 카드 대기 칸·첫 접촉 대기·여정을 비운다 → LifeState = Interregnum, 도약 폭 1수(CosmicSuccession::LeapOf — §7.2 LeapTicksTotal의 식) → LeapTicksTotal += L, PendingLeapTicks = L → L == 0이면 그 자리에서 Succeed. Succeed: 계승 추첨 1수(후보를 보기 전에) → 후보 = CosmicSuccession::CandidatesOf(죽은 그릇이 선 Player.CurrentSystemID의 성계로 — 위 세계 함수; 고르기는 CosmicSuccession::PickOf: 그 수 f로 min(⌊f × N⌋, N − 1)번째); 없거나 고른 후보가 성역이면(후보는 주인이 있어 성역일 수 없다 — 그렇다고 하는 상태는 부름을 건드리기 전에 에러 한 줄) 이름 자리의 수 1을 뽑고 버린 뒤 공백기에 머문다(침묵 — PendingLeapTicks 0, 다시 뽑지 않는다) → PersonalTier가 Tier 2 이하면 Tier 1(Install이 PeakTier = PersonalTier를 읽으므로 그 전에), Powers는 Way.*만 남긴다 → Install(고른 성계, 그 주인, Wanderer — 이름 1수) → 몸을 입었으면 취임 줄(`[B-5d1]` — CosmicRows::LogInauguration: Log.Calling 등급 2, {Player} 새 생애의 이름, {Civ}·{System} 출생 백성과 성계, 초점도 그 둘; 그 틱의 로그가 출생 성계에 대한 타락의 1인칭(Log.Claim)을 이미 가지면 — UCosmicLogSubsystem::HasClaimLineThisTick — 그 자리에 둘째 행 LogInaugurationUnnamed({Player}·{Civ}, 초점 성계 없음 — docs/70 §8.3 사후 규칙 ⑤). 어느 쪽이든 말하기 1수이고 새 생애의 장을 연다 — CosmicChronicle::OpensCalledChapter; 침묵과 거절된 Install에는 줄이 없다). 계승 지점은 후보가 있든 없든 2수다(§9.2)

UPROPERTY() FGalaxyState State;

UPROPERTY(Transient) FCosmicDataTables Tables; // 정의 테이블. UDataTable 참조를 쥐므로 UPROPERTY

FCosmicRulesDef Rules; // SetTables가 다시 읽는 DT_Rules 행의 사본(행이 없으면 C++ 기본값)

FGalaxyDef HomeGalaxy; // SetTables가 다시 읽는 고향 은하 행의 사본(행이 없으면 C++ 기본값)

FRandomStream Rng;

FCosmicStageClock StageClock; // 단계의 시계(B3 — 측정 전용, 위 GetStageClock). 저장·해시 밖

float Accumulator = 0.f;

float TimeScale = 1.f;

bool bRunning = false;

bool bBroadcastEnabled = true;

bool bInTickStages = false; // 열 단계와 그 끝의 SyncRngState가 도는 동안 참(B5가 열째 단계 '역할 효과 해소'까지 넓혔다 — 위 IsInTickStages). 이때 NewGalaxy·LoadState·SimulateTick이 불리면 ensure하고, StepOnce·RunHeadless·RunLeap은 에러 한 줄로 거절한다 (§9.4)

int32 SimTickDepth = 0; // OnSimTick을 내는 깊이(B5 — IsSendingSimTick = SimTickDepth > 0; SimulateTick의 경계와 RunLeap의 경계가 센다). 저장하지 않는다

bool bSendingLeapBoundary = false; // RunLeap이 제 경계의 OnSimTick을 내는 동안 참(B5 — IsSendingLeapBoundary). 저장하지 않는다

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

- ⓞ **정리**: 소유 성계가 0인 문명을 배열 순서로 제거한다(파생 기준이라 표지를 저장하지 않는다). 제거할 때 다른 모든 문명의 `Relations`와 `Ties`(짝 기록 — 태세·조약, B3 — 사라진 백성과의 조약이 다음 틱의 무역 영향력에 남지 않는다)에서 그 CivID 키를 지우고(추첨 없음), `Knowing.PendingContactCivID`가 그 문명이면 비운다(그 밖에 그 ID를 가리키는 기록은 그대로 둔다 — 조회가 실패하면 사라진 것이다, §7.2). 그 틱에 플레이어가 통치하는 문명은 남겨 두고, 역할 효과 해소가 플레이어를 떼어 낸 다음 틱의 정리에서 제거한다 — 남겨 두는 일은 그 틱에 기록된 폐위(`Effect.DeposeRuler`)나 사망(`Effect.KillPlayer` — B5)에 기댄다. 기록된 폐위가 없는 경우는 셋이다: ① 먼저 적힌 사망(`Effect.KillPlayer`)이 폐위를 밀어낸다 — B5의 사망 파이프라인(`UPlayerRoleSubsystem::Die`)이 기록된 사망을 처리하며 왕좌를 놓아 `bIsPlayerRuled`를 풀고, 그 틱의 건넘은 기록된 사망도 기록된 폐위처럼 받아 ③의 에러 줄을 내지 않는다(§11). ② 출생 보호가 절멸의 사망을 가린다 — 가려진 사망은 기록 자리를 차지하지 않으므로(위 역할 효과 규칙) `Hazard.Extinction`의 만료 훅이 초신성의 붕괴처럼 그 뒤에 폐위를 적는다(그 행이 오는 블록의 몫 — §16 #138, §11; 사망 파이프라인은 가려진 사망을 받지 못한다). ③ 정리 자신이 통치 문명의 남은 정착을 놓아 그 문명을 비운다 — 놀이에서는 닿지 않는다(마지막 정착된 성계는 문 뒤의 단계 코드만 앗고, 파괴가 정착을 함께 놓은 뒤 붕괴를 적는다). 닿으면(손으로 만든 상태) 그 문명이 계속 남으므로 에러 한 줄을 낸다(`TickCrossings` — `LetSettlementsGo`는 놓은 것이 있는지를 돌려준다). 남겨 둔 문명은 다음 틱에 연구하지 않는다(연구 단계는 성계가 없는 문명을 건너뛴다 — 돌파 줄과 책의 발견 행이 나지 않는다). 제거에는 줄이 없다 — 마지막 성계를 앗은 만료 줄(과 플레이어가 통치하던 문명이면 '몰락한 군주' 줄)이 이미 말했다. '소유 성계'는 `OwnedSystems`에 있고 주인이 그 문명인 성계다(`CosmicState::OwnsAnySystem` — 붕괴의 검사도 같은 술어를 쓴다). 정리는 먼저 정착된 성계가 없는 문명의 남은 정착을 무주로 놓는다(`CosmicHazards::LetSettlementsGo` — 놀이에서는 파괴가 이미 놓았다, §5.7) — 그래서 정착 중인 성계만 가진 문명은 남지 않는다. 통치 문명의 마지막 정착된 성계를 앗아 가는 단계(위기의 만료 — 분쟁은 앗지 않는다, B3)가 그 자리에서 남은 정착을 함께 놓고(`LetSettlementsGo` — 파괴처럼) `PendingRoleEffect`를 적는다(붕괴는 폐위 파이프라인, `Hazard.Extinction`이면 사망 — §4.4, 그 행은 아직 없다. 정리가 정착을 놓는 것은 그물일 뿐이고 붕괴를 적지 않는다 — 위 ③). 그 자리들은 앗기 **전에** 마지막 성계의 문(`GraceKeepsLastSystem` — 위기 처리 작업의 헤더에서 선언했다)을 먼저 묻는다: 그 성계가 주인의 마지막 정착된 성계이면(`IsLastSettledSystem` — 정착 중인 성계는 셈하지 않는다. 그 정착은 함께 놓인다, §5.7) `event_grace_remnant`를 `FireRow`로 내 보고, 행이 나가면 상실 전체를 건너뛴다 — 만료 효과(`OnExpireEffects`)를 적용하지 않고, Hazard 슬롯을 결과 없이 비우고, 역할 효과를 적지 않고, 그 슬롯에 주장을 세우지 않는다(§4.5). 행이 나가지 않으면 상실을 그대로 진행한다. 데이터 효과 `TransferSystem`은 어느 백성의 마지막 정착된 성계도 — 정착이 남아 있어도 — 옮기거나 놓지 못한다(그런 효과는 건너뛴다) — 백성이 소유 성계 0에 닿는 길은 문을 거치는 단계 코드뿐이다. 만료의 효과는 문을 물은 뒤에 걸리므로 그 효과가 주인의 다른 성계를 놓을 수 없게 스키마가 막는다(만료 효과와, 이웃 효과가 부르는 해결 효과에 `TransferSystem` 없음 — `Data.Integrity`). 그래도 그렇게 되면 코드가 파괴 앞에서 문을 다시 묻고 에러 한 줄을 낸다 — 그때 지켜진 성계는 만료 효과와 이웃 효과를 이미 받았다. 위기 단계의 `Hazard.Extinction` 만료는 그 문명의 성계를 놓게 할 뿐 배열을 줄이지 않는다.
- ⓐ **도착**: `ArriveTick == Tick`인 `Transits` 기록을 배열 순서로 처리한다(착지 추첨은 시뮬 스트림, §5.7 건넘).
- ⓑ **출발**: 이번 틱에 확정된 출발을 `Transits` 끝에 더한다(고향발은 `Civs` 배열 순서, 요약 은하발은 행 이름 `FNameLexicalLess` 순). 도착을 먼저 처리해 넷째 칸의 셈을 확정한 뒤 출발을 판정한다.

제거·추가 뒤에는 저장하지 않는 색인(ID → 인덱스)을 다시 만든다(색인을 다시 만드는 곳은 `NewGalaxy`·`LoadState`·정리·도착 넷이다). 건넘 단계 밖의 단계는 `Civs`에 원소를 더하거나 빼지 않는다. 문명을 만들려는 효과(반란의 분리, 바벨의 실제 분열, 바벨 연합의 '흩어짐')도 요청만 적고, 그 틱의 이 단계가 백성을 만든다 — CivID는 `NextCivSerial`에서 받고, `GalaxyRow` 도장을 찍고, 이름은 아키타입의 뱅크에서 `CosmicSeed::Derive`로 고른다. 그런 효과는 Phase 6a 이후이고 쓸 때 §12.3을 거쳐 다시 명세한다(VS에는 문명을 만드는 효과가 없다). `Systems`는 판 도중에 원소 수가 바뀌지 않는다(파괴된 성계는 죽은 노드로 남는다, §7.2). 소유 성계가 0인 문명은 그 틱의 AI 단계에서 건너뛴다. `TickCrossings`는 정착과 문명 제거 작업부터 ⓞ만 하고, ⓐⓑ는 Phase 6a까지 빈 본문이다.

**넷째 칸의 셈**: 넷째 칸의 제동(§5.12)이 무엇을 세고 어디서 막는가의 정의는 여기에 한 번만 적는다. 상한은 `FGalaxyDef.AIFourthRungCap`이고, 0이면 제동이 없어 아래의 통과 규칙 셋이 모두 통과다.

- T4Civs(G) = `GalaxyRow`가 G이고 Tier가 T4 이상인 문명의 수(플레이어가 통치하는 문명 포함 — §4.2의 설계 범위 '넷째 칸 이상 3~7'은 그 백성을 세지 않는다. 두 셈은 다른 셈이다).
- Inbound(G) = `Transits` 기록 가운데 `ToGalaxyRow == G`이고 아직 착지하지 않은 것의 수 — 업적 판정(AI 단계의 마지막)은 건넘 단계보다 먼저 돌므로 `ArriveTick ≥ Tick`으로 세고, 건넘 단계 안의 셈(착지·출발)은 `ArriveTick > Tick`으로 센다(처리 중인 기록은 세지 않는다).
- AI 문명의 T3→T4는 T4Civs(G) + Inbound(G) < 상한일 때만 통과한다. `Civs` 배열 순서로 먼저 판정된 문명이 통과한다.
- 건너온 백성의 착지(ⓐ)는 T4Civs + Inbound(처리 중인 기록은 세지 않는다) < 상한일 때만 통과한다.
- 고향으로 향하는 요약 은하발 출발(ⓑ)은 예정된 틱에 T4Civs + Inbound < 상한일 때만 기록되고, 건너뛴 칸은 다시 시도하지 않는다(k는 `Transits`의 기록 수가 아니라 `Tick`에서 찾는다).

**앎 단계(`TickSight`) — 보는 거리와 만남**: 앎 단계는 세계를 읽고 `Knowing`과 이야기만 쓰며 시뮬 스트림 추첨은 0회다. 가드(`bInTickStages`) 안에서 줄을 낼 수 있으므로 `OnLogEntry` 핸들러가 동기로 불린다 — 핸들러는 상태를 바꾸지 않는다(§9.4). 첫 접촉 카드도 핸들러가 띄우지 않는다: 앎 단계 끝의 제시가 기다리는 카드 칸에 놓고 화면은 틱 경계에 그 칸을 읽는다(B3 — 아래 '첫 접촉 카드'). `Install`(역할 효과 해소 단계 안 — 단계 가드 안, B5)은 그 틱에 출생 성계와 출생 문명을 `Knowing`에 적는다(난수 없음. 방랑자로 취임하거나 방랑자로 바뀌는 틱에는 선 성계의 1레인 이웃도 적는다 — §4.4). 즉위(`BecomeRuler`, 틱 밖 — B1)는 그 백성의 소유 성계를 `OwnedSystems` 차례로 줄 없이 적는다(그 백성은 즉위의 조건으로 이미 만났다). 방랑자의 스캔(`ScanCurrentSystem`, 틱 밖 — B1)은 같은 보기(`LookFrom`)를 선 성계 하나에서 보는 거리 + `DT_Rules.ScanSightBonusLanes`로 한 번 돈다.

보는 거리의 단위는 레인 수다(본문과 ST_CA 라벨은 '보는 거리' — '시야'는 §6.4의 '영적 시야'와 겹친다. 노브 이름은 그대로). `DT_Rules.SightLanesByTier = {2, 3, 4, 6, 10}`(5칸 — MVP ③ 전 {1, 2, 4, 6, 10}), `SightIntervalTicks` 10, `FirstSightTick` 330 — 모두 앎 쪽 노브이고 세계와 생성 규칙은 읽지 않는다.

앎 단계는 `SightIntervalTicks`마다 돌고(1 미만은 1로 읽는다 — 스키마가 1 이상을 지킨다), 그릇의 `CurrentSystemID`가 바뀐 틱(틱 첫머리의 값과 이 단계 직전의 값이 다르다 — 여행의 도착도, 초신성의 파괴가 그릇을 옮긴 것도)에는 주기와 무관하게 한 번 더 돈다. 보는 거리는 `SightLanesByTier[Tier]`이고 없는 칸이나 1 미만은 1로 읽는다(에러 줄 없이 — 범위는 스키마가 지킨다). 눈(§4.4)에서 같은 은하의 레인을 따라 너비 우선으로 보는 거리 안의 성계를 `NoteKnown`으로 `KnownSystems`에 더한다(이번 갱신의 임시 집합은 이미 아는 성계를 거르기만 한다. 출발점은 `OwnedSystems` 배열 순서 — 눈의 자리 모두를 한 번에 0단에 놓는다, 이웃은 `Lanes` 배열 순서 — 더해지는 순서가 해시에 든다. 정수만 쓰고 임시 색인 위에서 돈다. 레인을 따라 찾아 나가는 일은 `CosmicState::SightWalkOf`, 보기 한 번은 `LookFrom` — B1). `FirstSightTick`은 통치자가 소유 성계에서 보는 것만 늦춘다: 그 전의 통치자는 출생 성계(와 그 사이에 제 백성이 얻은 성계 — `NoteKnown`, §8.1)만 안다. 판의 틱 기준이라 첫 생애의 초반에만 작용하고, 방랑자의 눈은 늦추지 않는다.

- **만남**: 갱신 때마다 아는 성계의 현재 주인을 다시 본다. 주인이 아직 만난 백성이 아니면 `NoteMet`으로 만난다 — 방문이 아니라 알아봄이다(행성에 묶인 두 백성은 배가 아니라 신호로 만난다. 첫 함선은 8:30의 박자다). 격차 규칙이 먼저다: 상대의 Tier가 보는 쪽보다 3 이상 높으면 성계는 아는 성계가 되지만 그 주인은 만난 백성이 되지 않는다(§3.2). 한 번의 갱신에서 여럿을 만나면 가까운 순서(눈에서의 레인 수, 동률은 CivID `FNameLexicalLess`)로 더한다 — 거리는 눈의 자리(통치자는 소유 성계 전부, 방랑자는 선 성계)에서 같은 은하의 레인을 보는 거리의 상한 없이 잰다(예전에 알게 된 먼 성계도 잰다). 한 백성의 거리는 그 백성이 가진 아는 성계 가운데 가장 가까운 것이다. `FirstSightTick` 전의 통치자도 눈의 자리는 있다 — 새 성계를 적지 않을 뿐 만남은 같은 순서로 본다. 격차 규칙에 막힌 백성은 만나지 않은 채 남고 갱신마다 다시 본다(줄도 카드도 없다 — 이름 없는 현상 줄은 그 줄을 내는 Phase 4 자리의 것). 만남의 줄의 주제는 그 백성과 그 백성을 알아본 성계다(아는 범위와 로그 작업의 헤더).
- **낱말과 줄**: 만남은 각각의 만남이고, 첫 접촉은 그 판의 첫 만남이다(§2.4의 5:30 박자). '그 판의 첫 만남'은 저장된 필드 하나로 정의한다(이 정의는 여기 한 곳에만 쓰고 다른 절은 가리킨다): `FKnowingState`의 멤버 `FName FirstContactCivID` — 초기값 NAME_None은 '이 판에 아직 첫 접촉이 없다'이고, 구조체와 함께 'v0.7 예약' 커밋에 들어가며, `Knowing`의 일부라 전체 해시에 들고 세계 해시 밖이다. **첫 접촉**은 `FirstContactCivID`가 NAME_None인 동안 `NoteMet`이 `MetCivs`에 새 항목을 적는 만남이고, 같은 호출이 그 칸에 만난 백성의 ID를 적는다. `Install`은 이 칸을 결코 적지 않는다 — `Install`은 생애마다 출생 문명을 `MetCivs`에 줄 없이 적으므로 `MetCivs`가 비었는지나 그 길이로는 첫 만남을 가릴 수 없고, 그래서 필드로 가린다. 한번 적힌 칸은 바뀌지 않는다: 계승에도 남고(§4.4) 그 백성이 사라져도 비우지 않는다(§7.2) — 첫 접촉은 판에 한 번이다. 첫 접촉은 `log_first_contact`(Severity 1) 한 줄, 그 뒤의 만남은 `log_contact`(Severity 1, 3변형) 한 줄이다 — `log_first_contact`와 카드 `petition_first_contact`는 그 한 번의 만남에만 속한다. `MetCivs`에 새 백성이 적히는 자리(`NoteMet`)가 그 줄을 낸다(말하기 스트림만 쓴다). `Install`이 적는 출생 문명은 만남이 아니다. 만난 백성을 세는 곳 — CSV `MetCivs`와 §9.3의 목표(VS 첫 생애 2~6, EA 한 생애 12 이하) — 은 `ID`가 `Calling.Lives`의 어느 생애 기록(지금 생애 포함)의 `BirthCivID`와 같은 항목을 빼고 센다. CSV `FirstContactTick`은 `ID`가 `FirstContactCivID`와 같은 `MetCivs` 항목의 `Tick`이고, `FirstContactCivID`가 NAME_None인 동안은 빈 값이다.
- **첫 접촉 카드**(B3 — 카드 행과 함께, 헤더 B3 2.9절과 11절 항목 11): 첫 접촉(위의 정의)에, 몸을 입은 통치자이고 `DT_Encounters`에 `petition_first_contact` 행(`CosmicRows::PetitionFirstContact`)이 있으면 `NoteMet`이 `Knowing.PendingContactCivID`에 그 백성을 적는다(행이 없는 테이블에서는 적지 않는다 — 테스트가 지운 행).
  - **제시의 차례**: 앎 단계 끝의 제시(`CosmicPetitions::Present`)는 기다리는 카드가 없을 때만 돈다. 그 틱에 조건이 막 선 Standing 카드가 있으면 그것을 먼저 놓고(그 카드는 한 틱에만 오므로 첫 접촉 카드가 앞서면 다시 오지 않는다), 없으면 첫 접촉 카드를, 그다음 주기 슬롯을 본다. 첫 접촉 카드는 그 백성이 `Civs`에 있고 행의 조건(`Holds` — 지금은 `PlayerRoleIs Ruler`)이 설 때만 놓이고 — 서지 않으면 대기 칸이 남는다 —, 놓이면 카드 칸에 `PendingPetitionRow` = 그 행, `PendingPetitionSystemID` = NAME_None(주제 Civilization)을 적는다. 추첨·쿨다운·주기·`LastChoiceRows`의 거르기를 쓰지 않는다. 제 대기 칸이 있어 다른 카드 뒤에서 한 장 기다릴 수 있는 카드는 이것뿐이다 — 다른 청원에 답해도(`CosmicPetitions::Clear`는 대기 칸을 건드리지 않는다) 그 뒤 앎 단계 끝에 온다. 저장되고 로드 뒤 다시 보인다. 상시 카드(B4의 영압 카드 첫 장 petition_pressure_claim도)에는 이런 대기 칸이 없다 — 조건이 선 틱에 다른 카드가 칸을 쥐면 그 카드는 오지 않는다. B4는 상시 카드의 대기 버퍼를 넣지 않았다(헤더 B4 결정 8 — 첫 측정은 잰 수가 30건에 못 미쳤고, `[B-4d2]`의 합친 몫 6.9%는 문턱 10% 아래였다; 철야를 지니는 놀이에서 주장 카드가 칸을 잃는 몫 13%는 그 카드의 무게를 4로 올려 6.7%로 줄였다 — 헤더 B4 11절 항목 1·18, OPEN_QUESTIONS #135). 저장 필드는 그대로다.
  - **선택 둘**(판정 없음·비용 0·화자 없음): 손을 내민다(그 백성을 우호로 — 효과 `SetStance Stance.Friendly`, 우호의 자리 하나와 틱당 영향력을 쓴다. 선택지 아래 줄 `Petition.FriendlyUpkeep`이 그 값과 자리를 말한다) / 지켜본다(중립 — 아무것도 바꾸지 않는다). 첫째 칸의 우호 자리는 하나다.
  - **문이 거절하는 선택지**: 그 선택지의 태세를 문이 거절하면(우호의 자리가 찼다 — `CosmicPetitions::StanceBlockOfChoice`가 `CosmicDiplomacy::StanceBlockOf`의 첫 사유를 낸다, `Same`은 거절이 아니다) 카드가 그 선택지를 내지 못하는 비용처럼 회색으로 두고(툴팁은 [백성] 탭의 사유), 답은 에러 한 줄과 함께 거절한다 — 뽑지 않고, 카드와 대기 칸은 그대로 기다린다. 화면·답·정책이 이 한 함수를 읽는다 — B4부터는 카드 선택의 문 CosmicPetitions::ChoiceGateOf의 태세 갈래로(칙령·함대의 시간의 갈래와 함께, §7.4 'B4' 줄). 효과의 문(`Apply`의 `SetStance`)도 그대로 있다 — 답이 먼저 거절하지 않으면 수를 뽑고 우호를 건 것처럼 결과 줄을 내면서 아무것도 바꾸지 않는다(헤더 B3 11절 항목 11).
  - **답**: 답 명령은 그 칸의 백성을 문맥의 상대(`OtherID`)와 글의 `{OtherCiv}`로 읽고(`CosmicPetitions::ArgsOf`의 `OtherCivID`·`bNamesOther`), 받아들인 답마다 시뮬 수 하나(있는 규칙), 끝에서 — 받아들인 답이든 Withdrawn이든 — 그 칸을 비운다. 다른 카드의 문맥에는 `OtherID`가 없다.
  - **상대가 사라지면**: 카드가 기다리는 동안 그 백성이 없어지면 건넘 단계의 정리가 대기 칸을 비우고(놓인 카드는 그대로 기다린다) 답은 Withdrawn이다(`StillStands`가 그 백성을 찾지 못한다 — 수는 뽑힌 뒤다). 그동안 — 로드 뒤 다시 그린 카드에서도 — 글의 `{OtherCiv}`는 빈 이름이 아니라 ST_CA `Petition.OtherGone`('그 백성')이고, 어느 선택지도 회색이 아니다(태세의 문은 카드가 서 있을 때만 본다 — 답과 같다).
  - **역할이 바뀌면 비운다**: `Install`·`BecomeWanderer`·즉위(`BecomeRuler`)가 대기 칸을 비운다. 그 뒤의 만남은 줄만 내고, 뒤에 만난 백성에 대한 태세는 외교 명령(`SetStance`)으로 정한다. 데이터 규칙(스키마): 그 행은 `Named`·`Civilization`이고, 그 행이 아닌 카드는 `SetStance`도 `{OtherCiv}`도 갖지 않으며(행 시험 `row_not`), 사슬(`NextEncounter`)도 그 카드로 이어지지 않는다(대상 시험 `row_not`).
- **5:30의 보장**: §2.4의 '5:30 첫 접촉'은 생성 규칙이 보장한다 — `NewGalaxy`가 `sys_0`의 1레인 이웃 가운데 하나를 첫 AI 백성의 모성계로 뽑는다(시뮬 추첨 1회, 보는 거리 노브를 읽지 않는다. `CivCount`가 1이면 보장하지 않는다). 이웃의 아키타입·윤리는 무작위라(태세는 생성이 정하지 않는다 — 짝 기록은 비어 시작한다, B3) 보호도 수치 이점도 아니다. 보장하는 것은 상한이다: 첫 접촉은 늦어도 틱 `FirstSightTick + SightIntervalTicks`(340)까지 일어난다(아래 테스트).
- **만난 은하**(Phase 6a): 앎 단계가 `Transits`와 `MetCivs`에서 갱신한다(추첨 없음. 건넘 단계는 `MetGalaxies`를 쓰지 않는다) — 도착 기록의 `ResultCivID`가 만난 백성이면 그 `FromGalaxyRow`가, 출발 기록의 `SourceCivID`가 출발 때 이미 만난 백성이었으면(만난 틱 ≤ `DepartTick`) 그 `ToGalaxyRow`가 만난 은하다.

데이터 검사: `SightLanesByTier`의 모든 칸 ≥ 1, `SightIntervalTicks` ≥ 1, `FirstSightTick`·`FarLineCooldownTicks` ≥ 0(스키마 — 아는 범위와 로그 작업의 리뷰). 테스트 `Sim.FirstContactTimeline`: 시드 1~50(입력 없는 판)에서 첫 접촉이 늦어도 틱 `FirstSightTick + SightIntervalTicks`(340)까지 일어난다. 단언은 상한이다 — 세계 코드가 통치 문명을 상대로 `NoteMet`을 부르는 자리(분쟁의 줄은 B3에 들었다 — 공성의 줄과 다스리는 백성을 AI가 에워싸는 줄 전에; 청원은 Phase 4, §8.1)가 첫째 칸에서도 서면 이웃이 틱 330보다 먼저 만난 백성이 될 수 있다(B3의 분쟁은 함대가 없는 첫째 칸에서 서지 않는다). 데이터가 지금도 열 수 있는 길이 셋 있다 — 효과가 통치 문명에 성계를 넘기면 `FollowUpKnowing`이 준 백성을 만나고, 통치자가 아는 성계가 효과로 다른 백성에게 넘어가면 다음 갱신이 그 백성을 만나며(`FirstSightTick` 전에도 — 그 전의 눈은 성계를 더하지 않지만 아는 성계의 주인은 만난다), `FirstSightTick` 전에 폐위되면 방랑자의 눈은 늦춰지지 않아 새 눈이 아는 이웃의 주인을 다음 갱신이 만난다(폐위에는 출생 보호가 없다 — 재감사). 게임의 행은 `TransferSystem`을 쓰지 않고, 게임의 반란 행은 틱 500 전에 만료하지 않는다(발생이 틱 300부터이고 카운트다운이 200틱이다). 그런 호출과 행이 없는 Phase 2·3에서는 첫 접촉이 틱 330~340(`FirstSightTick`부터 `SightIntervalTicks` 안)이고, 테스트는 그 Phase 동안 하한 330도 함께 단언한다(B3에서도 그대로 둔다 — 세계 코드의 `NoteMet`(공성의 줄)은 함대가 없는 첫째 칸에서 일어나지 않는다). Phase 2 종료 조건의 첫 접촉 확인은 이 테스트가 한다(스모크 실행은 첫 프레임에 끝나므로 볼 수 없다). 값은 CSV `KnownSystems`·`MetCivs`·`FirstContactTick`으로 맞춘다. 목표(§9.3): VS에서 첫 생애에 만난 백성 2~6, EA에서 한 생애에 12 이하 — 60성계 그래프에서 T3의 4레인이 지도 대부분을 덮을 수 있다.

### 7.5 승천·이벤트·로그 Subsystem 인터페이스 요약

```cpp

// UCosmicEventSubsystem

// - 두 층이다. static 층(Check·CheckAll·Apply·ApplyAll)은 this가 없어 문맥이 건넨 상태·색인·테이블·규칙·스트림만 닿는다 — GetRng()·로그·NoteMet·NoteKnown·델리게이트에 닿을 길이 없고, 떼어 낸 상태 사본에서도 돈다. 멤버 층(EvaluateAndFire·FireRow, 그리고 TellHazardLine·FollowUpKnowing·FollowUpNotes)은 살아 있는 상태에서만 돌고 static 층이 못 하는 세 가지를 한다: 앎을 적고(FollowUpKnowing — NoteKnown·NoteMet), 줄을 내고, OnEventFired를 낸다. Subsystem에는 멤버 변수도 캐시도 없다

// - 행은 원자다: (슬롯 행이면 쿨다운 키) → 효과 전부(배열 순서, 그 자리에서) → 행의 줄 → OnEventFired. 효과 도중에는 줄도 델리게이트도 나가지 않으므로 핸들러는 반쯤 적용된 행을 보지 못한다. 큐는 없다. 위기 처리 작업부터는 '효과 전부 → 행의 줄 → 뒤따르는 줄(효과가 놓은 Hazard의 발생 줄과 전조 행, 해결한 Hazard의 해결 줄) → OnEventFired'다 — 원인이 먼저 읽힌다. 그 작업이 문맥에 후속 처리 기록(FCosmicEvalContext.Notes)과 멤버 FollowUpNotes를 더했다. 아는 범위와 로그 작업부터는 효과 전부와 행의 줄 사이에 받은 성계의 앎이 든다: 효과로 통치 문명이 받은 성계는 어느 줄보다도 먼저 아는 성계가 되고, 그 성계를 준 백성은 만난 백성이 된다(§8.1, FollowUpKnowing — 준 백성은 그 리뷰가 더했다. 그 만남의 줄이 행의 줄보다 먼저 나갈 수 있다). SpawnHazard·ResolveHazard·TransferSystem은 세계 함수 CosmicHazards::Place·Resolve·Transfer를 거친다(틱당 원천은 셋 다 줄 없이 돌려보낸다)

// - int32 EvaluateAndFire(FGalaxyState&, FRandomStream&)  매 틱, 위기 단계의 맨 끝에서 부른다. 이벤트 슬롯을 돈다. 정착된 성계를 가진 문명은 DT_Rules.EventSlotTicks(10)틱에 한 번 슬롯을 받는다 — Tick mod EventSlotTicks가 CosmicSeed::DeriveForName(Seed, CivID, EventSlot) mod EventSlotTicks와 같은 틱이다(백성마다 고정된 위상. 배열 인덱스에 기대지 않아 정리가 다른 백성의 박자를 밀지 않는다. EventSlotTicks가 1 미만이면 1로, EventChancePerSlot이 0 미만이면 0으로 읽는다 — 범위를 벗어난 노브마다 에러 한 줄). 그 틱에 슬롯을 받는 문명마다 Civs 배열 순서로 건네받은 스트림에서 정확히 1회씩 먼저 전부 뽑는다 — 이 함수 자신의 추첨 수는 시드·틱·소유·CivID만 따르고 DT_Events의 내용·쿨다운·가중치·공백기에 좌우되지 않는다(테이블이 없어도 뽑는다). 그다음 문명마다 그 순서로, 제 차례의 상태를 보고: ① Standing 행 — 조건이 참이고 쿨다운이 끝난 (행, 단위)를(조건을 먼저 보고, 쿨다운 키는 조건을 통과한 단위가 있는 행에서만 한 번 만든다 — §9.4의 예산이 이 순서를 전제한다. Lottery 행도 같다) 행 이름 순(FNameLexicalLess. 한 행의 단위는 OwnedSystems 순서)으로 모아 하나씩 낸다. 발화 직전에 셋을 다시 본다 — 단위가 아직 그 문명의 정착된 성계인가, 조건, 쿨다운. 쿨다운이 (행, 문명)마다라서 쿨다운이 있는 SettledSystem 행은 슬롯당 한 단위(통과한 첫 성계)에만 나고 CooldownTicks 0인 행은 모든 단위에 난다(추첨·가중 없음) ② Lottery 행 — 조건과 쿨다운을 통과한 (행, 단위)를 같은 순서로 놓고, 후보의 몫 = BaseWeight × 가중 인수 × DT_Rules.EventChancePerSlot(몫의 합이 1을 넘으면 함께 줄인다 — 몫을 하나씩 나누지 않고 뽑아 둔 수에 합을 곱해 견준다. 그래야 반올림 때문에 어느 후보에도 떨어지지 않는 수가 남지 않는다), 뽑아 둔 수가 떨어진 자리의 후보 하나를 낸다(문명·슬롯당 최대 1행, 어느 자리에도 떨어지지 않으면 없음). 행의 단위는 Subject 컬럼이다: Civilization = 문명에 한 번(문맥 성계 = 정착된 수도), SettledSystem = 그 문명의 정착된 성계마다(OwnedSystems 순서). Named 행은 슬롯이 보지 않는다. 쿨다운은 (행, 문명)마다다 — 키 "<행>@<CivID>". 600틱마다 끝난 키를 지운다(끝난 키와 없는 키는 뜻이 같다). 반환값은 발화한 행 수. 백성 하나의 빈도는 백성 수와 무관하다: 60·12에서 틱당 슬롯 약 1.2개, 300·48에서 약 4.8개

// - bool FireRow(FName RowName, const FCosmicEvalContext&)  행 하나를 이름으로 낸다(Named 행: 반란 전조 ②, 은혜 행, 연구 단계가 Society 돌파마다 내는 event_book_found — 책이 고넬료 사슬로 늦게 온 판의 문턱은 §16 #78). 행의 Conditions가 통과할 때만 내고(없는 행이거나 조건이 거짓이면 아무 일도 없이 false), 쿨다운 키를 읽지도 적지도 않으며 제 추첨이 없다. 제 문맥을 새로 만든다 — 호출자의 문맥에서는 주제만 받는다(SourceRow는 그 행, bPerTickSource는 꺼짐). 호출자의 문맥은 바뀌지 않는다

// - static bool Check(const FCosmicCondition&, const FCosmicEvalView&) / static bool CheckAll(TConstArrayView<FCosmicCondition>, const FCosmicEvalView&)  조건 1개 / 배열(AND, 빈 배열은 참) 평가 (업적/조우/청원도 같은 함수 사용). 뷰에는 스트림이 없다 — 뽑을 수 없고, 쓰지 않는다. 주제가 없는 조건(문명 없는 Civ 조건, 성계 없는 System 조건)과 풀 수 없는 Key의 조건은 거짓이다. 상대를 고르는 자리(CivRelationAtLeast·ChangeRelation — Key가 비면 문맥의 OtherID)는 살아 있는 문명의 CivID, 판에 있는 초월자 인스턴스의 TranscendentID, DT_Transcendents의 천상(Realm.Celestial) 행 이름 가운데 하나일 때만 풀린다(천상 행은 상태 인스턴스가 없어서(§4.2) 행 이름으로 풀린다. 타락·독립 행은 판에 인스턴스가 있을 때만 상대다 — 로스터에 뽑히지 않은 타락의 키가 세이브에 남지 않는다). CivStabilityBelowFor·CivFallenSphereTicksReached의 Key는 숫자만으로 된 양의 정수(9자리까지)일 때만 풀린다. Player 조건은 Calling.LifeState가 Embodied가 아니면 전부 거짓이다 — 그릇을 고치는 효과와 역할 효과의 Embodied 검사(Apply)와 함께 이것이 '공백기 중 플레이어 제외'의 전부다. MegaprojectComplete/TranscendentSealed는 문맥의 SinceTick 기준 + 0 센티널(CompletedTick > 0, bSealed): 플레이어 업적 판정만 SinceTick에 Calling.BornTick을 넣고 이벤트·Hazard·AI 판정은 0(언제든)이다 — 세계의 판정이 그릇의 출생 틱에 기대지 않는다 (§4.4)

// - static bool Apply(const FCosmicEffect&, FCosmicEvalContext&) / static void ApplyAll(TConstArrayView<FCosmicEffect>, FCosmicEvalContext&)  효과 1개 / 배열. 세계 상태만 바꾼다: 줄을 내지 않고, 앎을 적지 않고, 행을 내지 않고, 방송하지 않으며, Civs·Systems의 원소 수를 바꾸지 않는다. KillPlayer/DeposeRuler는 즉시 실행하지 않고 FCallingState.PendingRoleEffect에 기록만 한다(틱당 최대 1건, 먼저 기록된 것 우선) — Embodied일 때만. KillPlayer는 문맥에 성계가 있으면 그 성계에 선 방랑자에게만, 성계가 없으면 문맥 문명이 통치 문명이거나 문맥에 문명이 없을 때만, 그리고 출생 보호 밖일 때만 적는다(가려진 KillPlayer는 칸을 차지하지 않는다). DeposeRuler는 문맥 문명이 통치 문명일 때만. 파이프라인은 틱 경계의 '역할 효과 해소' 단계(§7.4)가 UPlayerRoleSubsystem에서 시작한다. PlayerStatDelta·GrantPower는 그릇과 부름을 고치므로 Embodied일 때만 적용한다. MultiplyIncome은 문맥의 bPerTickSource가 참일 때만 적용한다(경제 단계의 틱당 원천). Value는 0 아래를 0으로 읽고(수입이 음수가 되어 비축이 0 아래로 내려가지 않는다) Key가 있든 없든 같은 곱 하나다(수입 × Value). 양을 더하거나 곱하는 효과 여덟(AddResource·MultiplyIncome·ShiftEthics·ChangeStability·ChangeRelation·ChangeReverence·ChangePopulation·PlayerStatDelta)은 Value가 유한한 수가 아니면(NaN·무한대) false다 — 숫자가 아닌 값이 한계값으로 잘리거나 상태에 적히지 않는다(범위는 Data.Integrity, 가드는 코드). 거꾸로 틱당 원천이 거는 효과는 양을 고치는 일곱뿐이다 — AddResource·MultiplyIncome·ShiftEthics·ChangeStability·ChangeRelation·ChangeReverence·ChangePopulation. 그 밖의 효과(NameNotable, KillPlayer·DeposeRuler, PlayerStatDelta·GrantPower, AddTrait·RemoveTrait·SetFlag, SpawnHazard·ResolveHazard·TransferSystem, FocusResearch, SetStance — B3, EnactEdict·DelayHazard — B4)는 bPerTickSource가 참이면 false다(NameNotable은 추첨도 하지 않는다): 경제 단계는 추첨하지 않고, 역할 효과를 적지 않고, 그릇을 고치지 않고, 한 번 서는 표지를 틱마다 세우지 않는다. 열거형 끝에 더하는 효과는 이 목록에 넣기 전까지 틱당 원천에서 돌려보낸다. DurationTicks는 읽지 않는다 — 세는 기록을 두지 않는다(Data.Integrity가 0보다 큰 값을 틱당 원천에만 허락하고, 칙령의 EffectsPerTick에는 그것도 막는다 — B2). TransferSystem은 어느 백성의 마지막 정착된 성계도 옮기지 못한다(정착 중인 성계는 셈하지 않는다). 풀 수 없는 Key의 효과는 건너뛴다(§16 #29) — 상대가 풀리지 않는 ChangeRelation은 Relations에 항목을 만들지 않는다(없는 백성·없는 초월자의 키가 세이브에 남지 않는다). SetStance(B3)는 상대(Key, 빈 Key = 문맥의 OtherID)에게 명령과 같은 문 CosmicDiplomacy::StanceBlockOf(bMet = true)를 묻고 None일 때만 Ties를 쓴다 — 살아 있는 백성이 아닌 상대·받지 않는 태그는 NoCiv, 자리가 찬 우호는 FriendlySlots로 false·바뀜 없음(효과도 단일 창구를 지난다, 불변 규칙 13). 적대 태그면 무역 조약을 값 없이 끊는다(CosmicDiplomacy::TurnHostile — 스키마가 그 태그를 데이터에서 막는다). 카드의 답 가운데 문맥에 상대를 넣는 것은 첫 접촉 카드뿐이다(§7.4 앎 단계의 '첫 접촉 카드'). EnactEdict·DelayHazard(B4)도 같은 꼴이다 — 문맥 백성에게 명령과 같은 문(CosmicEdicts::EnactBlockOf — 그다음 CosmicEdicts::Enact가 EnactCost를 낸다 / 문맥 성계에 CosmicConflict::ResistBlockOf — 그다음 남은 시간 + ResistTicksOf, 함대 × ResistShareOf)이 None일 때만 쓰고, 거절은 false·바뀜 없음(답의 문 ChoiceGateOf가 수를 뽑기 전에 먼저 거절하므로 놀이에서는 닿지 않는다). 줄은 내지 않는다(카드의 결과 줄이 말한다 — log_edict_enacted를 되풀이하지 않는다). 반환값은 효과가 적용되었는가다 — false면 FGalaxyState는 그대로이고(NameNotable의 추첨 1회는 이미 뽑혔다 — 틱당 원천이 돌려보낸 때는 뽑지 않았다) 호출자는 다음 효과로 넘어간다

// - 평가 문맥은 FCosmicEvalContext(상태·스트림·색인·테이블·규칙 + 주제 + bPerTickSource)와 그 읽기 전용 쪽 FCosmicEvalView다(평범한 C++ 구조체이고 USTRUCT가 아니다 — 상태와 스트림의 참조를 든다. 문맥은 복사할 수 없다). 주제(FCosmicEvalSubject) = CivIndex·SystemIndex(Civs·Systems의 인덱스, 없으면 INDEX_NONE)·OtherID(상대 — CivID 또는 TranscendentID)·GalaxyRow·SourceRow·SinceTick. 문맥은 처음부터 GalaxyRow와 스트림을 갖는다 (§7.2, §9.2). GalaxyRow에는 호출자가 평가 대상인 문명·성계의 GalaxyRow를 넣는다(Phase 2 은하 생성 규칙 변경이 도장을 찍기 전에는 NAME_None = '아직 찍히지 않음'이 실린다). 효과를 적용하거나 행을 이름으로 내는 단계 코드는 UGalaxySimulationSubsystem::MakeEvalContext로 문맥을 만든다(EvaluateAndFire는 상태와 스트림을 건네받아 제 발화의 문맥을 스스로 만든다). Check·Apply·EvaluateAndFire는 문맥이 건넨 상태와 스트림만 쓰고 GetRng()를 직접 부르지 않는다. 앎(Knowing)은 읽지 않는다 (§8.1)

// - Apply의 NameNotable 가드(약 30줄, Phase 2, §1.3·§7.2): ① 문맥 문명 Tier <= 3(T4 집단 승천은 개인성을 지운다, §3.2) ② PersonNameBank가 비어 있지 않고 문명 Traits에 Trait.Mind.NoSelf가 없음(둘 중 하나라도 걸리면 무시) ③ 같은 Role의 열린 기록이 있으면 LastTick = max(Tick, 1)로 닫고 새로 만든다(0은 '열린 기록'이라 Tick 0에 닫아도 닫힌 것으로 읽히게 한다 — §16 #35의 기본값) ④ Notables.Num() >= DT_Rules.MaxNotablesPerCiv면 닫힌 기록 중 LastTick 최소를 퇴출, 닫힌 것이 없으면 Ruler·Regent가 아닌 열린 기록 중 FirstTick 최소를 퇴출, 그래도 없으면 생성을 건너뛴다(로그는 그대로 뜬다 — 명부는 짧고 역사는 길다). NameIndex = 문맥의 스트림에서 적용마다 정확히 1회, 이 가드들보다 먼저 뽑은 수로 현재 배열에 없는 뱅크 인덱스 가운데 하나를 고른다 — 기록을 만들지 않으면 그 수를 버린다(행의 시뮬 추첨 수가 이름 뱅크·Tier·상한에 좌우되지 않는다. PersonNameBank를 뒤 Phase에 채워도 시뮬 난수열이 밀리지 않는다 — §16 #40 ⑥. 같은 이름의 재등장은 바퀴의 표지라 데이터로 피한다). 퇴출은 연대기를 지우지 않는다(Message는 Push 시점에 렌더되어 저장됨). 닫힘은 이 훅들(즉위·복귀, 반란의 만료와 해결 — B1부터 반란 만료는 섭정 기록도 닫는다)과 문명 제거(소유 성계가 0이 된 문명을 건넘 단계의 정리가 지운다, §7.4)만 — 틱당 사망 롤·노화 없음(§4.4 원칙 4)

// - FOnCosmicEvent OnEventFired(const FCosmicEventNotice&)  값만 든 USTRUCT(EventRow·Category·Severity·SystemID·CivID·Tick). 동적 델리게이트의 인자는 리플렉션 타입이어야 하고 핸들러에 상태와 스트림을 건네지 않는다. 행이 다 적용되고 말해진 뒤에 낸다. 핸들러 규칙은 OnLogEntry와 같고(§9.4) EvaluateAndFire·FireRow·Apply도 부르지 않는다 — 방송 도중에 부른 EvaluateAndFire·FireRow는 에러 한 줄을 남기고 아무 일도 하지 않는다(Apply는 static이라 막지 못한다 — 규칙으로만 금한다)

// UAscensionSubsystem

// - void EvaluateFeats(FGalaxyState&, FRandomStream&)  틱당 정확히 1회, AI 단계의 맨 끝에서만 부른다(§7.4). 그 틱의 해결·만료·이벤트·AI 행동이 모두 지난 자리라 '주요 이벤트 후'는 자리로 지켜진다(따로 부르지 않는다). 순서: 플레이어가 통치하는 백성 먼저(Embodied이고 통치자일 때만. 조건은 SinceTick = Calling.BornTick으로 본다), 그다음 Civs 배열 순서로 그 밖의 백성(SinceTick 0. 정착된 성계가 없는 백성은 건너뛴다. 섭정 아래의 옛 백성도 여기 든다 — 업적 행에 통치자 전용 표지가 없다. 섭정의 업적 규칙은 B1 밖). 한 백성에 대해: Path가 Ruler이고 RequiredTier가 그 백성의 Tier인 DT_Feats 행을 행 이름 사전순으로 보고(한 칸 위가 아니거나 조건이 없는 행은 판정하지 않고 에러 한 줄을 낸 뒤 다음 행으로 간다 — FAscensionFeatDef::IsOneRungWithDeed. 보는 행의 거르개는 FAscensionFeatDef::IsJudgedRulerRowAt 하나다(Path·RequiredTier·IsOneRungWithDeed — AI의 선택과 상단 바의 '다음 칸까지'도 같은 행만 본다, Phase 3 12번 작업)), CompletionConditions가 그 백성과 제가 가진 정착된 성계 하나(OwnedSystems 순서 — 목록에 있어도 주인이 다른 성계는 세지 않는다)를 주제로 전부 성립하는 첫 행 하나로 오른다 — 한 틱에 한 칸. 오를 때의 순서: Tier = TargetTier → AscensionCost(ApplyAll, SourceRow = 업적 행) → 플레이어면 CompletedFeats·생애의 Feats와 PeakTier(오른 칸이 더 높을 때만, §16 #67 ①)·GrantedPowers → AI 백성이 Tier 3에 오르면 Notable.Ascendant 기록(아래 인물 훅 ③) / Tier 4 이상이면 열린 인물 기록 닫기(아래) → 받은 성계의 앎(FollowUpKnowing — 대가가 통치 문명에 넘긴 성계를 알고 준 백성, 곧 승천한 백성을 만난다. 그 만남이 승천 줄보다 먼저다) → 승천 줄(행의 LogTemplates, Log.Ascension, Severity 2) → 뒤따르는 줄(FollowUpNotes) → 플레이어면 OnTierAscended. 방랑자 경로(개인 Tier)의 판정은 첫 방랑자 업적 행과 함께 Phase 4다. 핸들러 안에서 불리거나 살아 있는 상태가 아닌 상태를 받으면 에러 한 줄을 내고 아무것도 하지 않는다. 틱 사이 입력이 채운 조건은 다음 틱의 판정이 잡는다

// - FOnTierAscended OnTierAscended(EAscensionTier NewTier, FName FeatRow, EPlayerRole Path)  플레이어의 승천에만 낸다(AI 백성의 승천은 줄뿐이다). 방송 게이트와 방송 중 표시를 거친다

// - 승천 시: 대가 적용 → 권능 부여(T5는 Way.Received/Seized 포함) → 받은 성계의 앎(FollowUpKnowing — 대가가 통치 문명에 넘긴 성계를 알고 준 백성, 곧 승천한 백성을 만난다. 그 만남이 승천 줄보다 먼저다 — 재감사) → 로그 Severity 2 → 뒤따르는 줄 → FX/사운드 요청 (Presentation이 구독 — 연출은 Phase 3)

// - 계승의 권능은 이 서브시스템이 아니라 UPlayerRoleSubsystem::Succeed가 다룬다(B5 — §7.4 ResolvePendingRoleEffect): Way.*만 남기고 나머지는 Tier에서 다시 준다 — 지금 칸이 주는 권능의 행이 없어 Way.*만 남는다(§4.4. B5 결정 8이 §16 #67 ②를 닫았다)

// - 인물 훅 ③ (§7.2): AI 문명이 간이 판정(§4.3)으로 Tier 3에 도달하면 그 문명에 Notable.Ascendant('먼저 건넌 자') 기록 1개 — SourceRow = Feat 행, Apply와 같은 가드(Tier <= 3·PersonNameBank·상한). T4 이상 도달은 만들지 않는다. 능력치 없음 이름 추첨은 T3에 도달한 AI 문명마다 정확히 1회, 가드보다 먼저다(Apply의 NameNotable과 같은 규칙)

// - 문명이 Tier 4에 도달하면(AI·플레이어 통치 문명 동일) 그 문명의 열린 인물 기록을 전부 닫는다(LastTick = Tick) — 집단 승천은 개인성을 지운다(§3.2). 닫힌 기록은 상한 규칙대로 남는다. 새 생성은 Apply 가드 ①이 막는다

// - AI 간이 판정의 T3→T4에는 넷째 칸의 제동(FGalaxyDef.AIFourthRungCap)이 걸린다 — 셈의 정의는 §7.4 건넘 단계이고 EA부터 유효하다(VS에는 T3→T4 판정 데이터가 없다). 플레이어의 업적 판정은 이 값을 읽지 않는다 (§5.12). 건너온 백성의 T4→T5 판정은 도착 뒤 DT_Rules.CrossingSettleTicks가 지나야 평가한다 (§5.7 건넘, Phase 6a)

// UCosmicLogSubsystem

// - 로그 모드는 두지 않는다(옛 계획의 ELogMode { Normal, Interlude }·SetMode()는 B5에 없다 — 헤더 B5 2.7절·2.12절): 공백기의 틱도 보통의 이야기 범위를 그대로 따르고, 보는 칸은 개인 칸(PersonalTier — 몸을 입은 통치자가 아니면 늘 그렇다)이다. 공백기 동안 바뀐 것을 보이는 쪽은 로그의 모드가 아니라 표현 계층의 순수 함수 World/CosmicDigest(아래 '다이제스트' — B5 `[B-5c]`)다 — 로그에 모드를 두면 격차 규칙을 넷째 자리에 베끼게 된다(규칙 14) (§4.4, §8.1)

// - void SetTellAll(bool)  저장하지 않는 모드 표지. 켜면 범위를 묻지 않고 전부 쌓는다(bUntold도 먼 줄도 없다 — Log.Record만은 이 모드에서도 bUntold로 쌓는다, B5 `[B-5d1]`). 게임 기본은 꺼짐. CosmicTests::AcquireSim이 켜고(그 모드를 끄는 테스트는 끝날 때 되돌린다 — AcquireSim은 포인터만 돌려준다), 커맨드렛은 아는 범위와 로그 작업부터 SetTellAll(true)로 돈다(인자 `-TellAll=0|1`은 Phase 2 종료 리뷰에서 구현됨 — 2026-10-18 정정). 어느 모드든 세계 해시는 같다 (§8.1)

// - void Push(FGameplayTag Category, uint8 Severity, const FText& Template, const FFormatNamedArguments&, FName FocusSystem, FName SourceRow, FName FocusCiv = NAME_None)  FocusSystem·FocusCiv = 주제 성계·주제 문명. 이야기의 범위가 Push하는 순간의 세계 상태와 앎으로 판정한다: 범위 안은 쌓고, 범위 밖의 Severity 2·Log.Claim은 bUntold로 쌓고, 그 밖은 쌓지 않는다 — Log.Record는 범위와 말하기 모드와 상관없이 늘 bUntold로 쌓고 방송하지 않으며 먼 줄도 붙이지 않는다(B5 결정 11-나 — 두 책만 읽는다) (§8.1). FocusCiv는 끝에 붙는 기본 인자라 기존 호출부는 그대로다(인자는 v0.7 예약 커밋, 범위는 아는 범위와 로그 작업부터). SourceRow = 그 줄을 낸 행(이벤트·Hazard·조우·Feat·로그 템플릿 행), Tick = State.Tick. 렌더한 결과를 평탄화해 저장한다: Message = FText::AsCultureInvariant(FText::Format(Template, Args).ToString()). 포맷 이력과 ST_CA 키가 세이브에 남지 않고, 쌓인 줄은 Push 시점의 언어로 고정된다 (§8.1, §8.5). 핸들러 안(IsBroadcasting)에서 불리면 에러 한 줄을 내고 아무것도 쌓지 않는다(Phase 2 종료 리뷰)

// - bool PushVariants(const FCosmicLine&, TConstArrayView<FText> Variants, const FFormatNamedArguments&)  호출자가 가진 변형 배열에서 한 줄을 낸다 — "말하기 추첨 1회, 렌더 결과가 마지막으로 말해진 줄과 같으면(bUntold 항목은 건너뛴다, §8.1) 다시 뽑지 않고 다음 변형" 규칙의 유일한 자리이고 DrawTell을 부르는 유일한 함수다. 변형이 1개여도 1회 뽑고, 배열이 비거나 핸들러 안(IsBroadcasting — 에러 한 줄)이면 추첨 없이 false, 그 밖에는 true(범위와 무관). FCosmicLine = Category·Severity·FocusSystem·FocusCiv·SourceRow·보는 쪽 Tier의 창(MinViewerTier·MaxViewerTier, 0 = 그쪽 끝 없음. 창은 아는 범위와 로그 작업부터 읽는다). 이벤트 행의 LogTemplates와 SignRow, Hazard의 줄, 초월자 행의 ClaimTemplates(B4 — VoiceTemplates는 읽는 코드가 아직 없다)가 모두 이 길로 나간다. B5 `[B-5d1]`: Log.Record 줄은 변형을 뽑은 뒤 범위를 묻지 않고 bUntold로 쌓는다(위 Push — 말하기 1수는 그대로). 둘째 주제(§8.1)는 Phase 4에 FCosmicLine의 필드로 더한다(호출부는 그대로)

// - bool PushRow(FName RowName, uint8 Severity, const FFormatNamedArguments&, FName FocusSystem, FName FocusCiv = NAME_None)  DT_LogTemplates의 이름 행 하나를 낸다: 행을 찾아 PushVariants(행의 Category·Templates, SourceRow = RowName, MinViewerTier = 행의 MinTierToReveal)를 부르는 얇은 포장이다 — 추첨·연속 중복·반환값의 규칙은 PushVariants의 것 그대로다(먼 줄을 내는 호출만 2회, §9.2). 로그 항목 수는 범위가 정한다(§8.1). 시뮬 스트림은 뽑지 않는다. 행이 없거나 템플릿이 비면 추첨 없이 false. 한 판에 한 번만 뜨는 고정 문장 행은 변형 1개를 허용한다(목록은 Tools/data/schema.json의 single_variant_rows: log_first_life·log_first_contact)

// - bool PushWithSign(const FCosmicLine& Line, TConstArrayView<FText> Variants, FName SignRow, const FFormatNamedArguments&)  아래 화자의 문장을 앞에 둔 줄(B4 — 헤더 B4 2.6절, §8.2): Variants가 비지 않았고 Line.MinViewerTier > 1(그 창 아래에 보는 이가 있다)이고 SignRow가 DT_LogTemplates의 행이면, 먼저 그 행의 줄 하나 — 그 행의 Category, Severity는 Min(Line.Severity, 1), Line의 초점과 SourceRow, 보는 쪽 Tier의 창 [0, Line.MinViewerTier − 1] — 를, 그다음 Variants에서 Line을 낸다. 둘 다 PushVariants를 거친다(말하기 추첨 각 1회 — 징조가 있으면 한 번 부를 때 PushVariants를 두 번). 징조가 먼저다: 위 화자가 '내 것'이라 말한 뒤 같은 자리를 두고 서술자 쪽 줄이 서지 않게. 테이블에 없는 SignRow는 에러 한 줄과 Line만. 반환은 Line의 PushVariants가 낸 값이고, Variants가 비면 아무것도 말하지 않고(추첨 없음) false, 핸들러 안이면 에러 한 줄과 false. 징조의 하나뿐인 집: 이벤트 행(FireResolved — 행의 SignRow, 그 행은 {Civ}·{Hazard}만 부른다, 스키마)과 주장의 1인칭 줄(위기 단계 만료 — CosmicRows::LogSignClaim, 줄의 인자로 {System}·{Civ}를 부를 수 있다). 시뮬 스트림을 뽑지 않는다

// - FName PickTemplateRow(FGameplayTag Category, FGameplayTag Context, FName FallbackRow) const  (Phase 4) (Category, Context)가 일치하는 선택 행의 이름, 없으면 호출자가 CosmicRows 상수로 넘긴 이름 행 FallbackRow. 뽑지 않고 줄도 내지 않는다 — 호출자가 그 이름으로 PushRow를 부르므로 변형 규칙이 선택 행에도 그대로 선다 (§5.11 계시 양식). 슬롯 {Revelation} {Transcendent}는 슬롯 해석이 푼다 (§8.2)

// - 슬롯 해석(§8.2, 약 20줄, Phase 4): 역할 슬롯 {Ruler} {Regent} {Rebel} {Loyalist} {Hearer} {Ascendant} = 호출자가 넘긴 문맥 문명의 그 역할 열린 기록 이름(PersonNameBank[NameIndex % Num]), 없으면 ST_CA 집단 폴백(다스리는 이·섭정·반란군·한 사람·들은 이·그들 중 하나) — 모든 문장은 인물 없이도 성립한다 (§8.3). {PopBand} = GetPopulationBand(CivID)의 ST_CA 라벨 6 중 하나. {Player} = FLifeRecord.NameIndex(로그는 아직 스스로 풀지 않는다 — B5의 죽음·취임 줄은 호출자가 CosmicChronicle::VesselNameOf의 이름을 인자로 넘긴다). Log.Record·Log.Claim·Log.Transcendent 템플릿과 FTranscendentDef.ClaimTemplates·VoiceTemplates에는 역할 슬롯·{PopBand} 금지 (인물은 두 책에 구조적으로 들어가지 못한다, §8.4). 지키는 자리는 C++ Data.Integrity가 아니라 파이썬 스키마의 칸 슬롯 목록이다(B4 헤더 3.8절): ClaimTemplates는 {System}만, VoiceTemplates는 비어 있어야 하고(max_len 0), 이벤트 행의 LogTemplates는 {Civ}·{System}·{Hazard}만이라 Log.Transcendent의 이벤트 줄에도 서지 않는다(Log.Record는 이벤트 행이 말하지 못한다 — B5 `[B-5d1]`, §7.3 event_log_categories). DT_LogTemplates의 Templates 칸은 범주를 가리지 않고 역할 슬롯을 받으므로, 그 세 범주의 로그 행을 처음 쓰는 작업이 행 규칙(slots_in)을 함께 더한다 — Log.Record는 B5 `[B-5d1]`의 기록 행 둘이 처음이고 그 범주의 모든 행이 {System}만이다(§7.3 DT_LogTemplates); Log.Claim·Log.Transcendent의 로그 행은 아직 없다. 인물은 화자가 아니다: 1인칭은 타락 전용 유지

// - FOnLogEntry OnLogEntry(const FCosmicLogEntry&)  방송 게이트가 닫히면 쌓이기만 하고 방송되지 않는다(나중에 몰아 보내지 않는다). bUntold 항목에는 발화하지 않는다 (§8.1)

// - bool HasClaimLineThisTick(FName SystemID) const  (B5 `[B-5d1]` — docs/70 §8.3 사후 규칙 ⑤): 이 틱의 로그가 그 성계에 초점을 둔 타락의 1인칭(Log.Claim — 말해졌든 bUntold든)을 이미 가졌는가. State.Log를 가장 새 항목부터 이 틱의 항목까지 거슬러 읽고, 그다음 State.Chronicle을 같은 꼴로 읽는다(한 틱에 상한보다 많은 줄이 오면 오랜 줄이 그리로 옮겨진다). 그 뒤에는 그 틱 그 성계를 부르는 서술자 줄이 서지 않으므로 취임 줄이 새 생애의 출생 성계로 묻고(거름이 마지막까지 풀린 계승은 그 틱 위기 단계에 주장이 편입한 성계를 고를 수 있다 — CosmicSuccession::CandidatesOf) 참이면 성계를 부르지 않는 둘째 행을 말한다(§7.4 ResolvePendingRoleEffect의 Succeed). 이야기 층이 제 줄에 묻는 물음이다 — 세계는 로그를 읽지 않는다. 읽기만 하고 뽑지 않아 핸들러도 부를 수 있다. None은 false

// - TArray<FCosmicChronicleChapter> BuildChronicle() const  (MVP ④, 2026-10-23 — 헤더 docs/60_DECISIONS/headers/A4_provisional_end.md §4) 지금 틱까지의 연대기 = CosmicChronicle::Build(산 상태). 읽기만 하고 뽑지 않아 핸들러도 부를 수 있다. 장 배열이다(장과 넘침 표지는 평면 배열에 실리지 않는다; 두 책 장은 이 배열에 없다 — 쪽이 CosmicChronicle::BuildTwoBooks를 따로 부른다, 아래 '두 책 장')
// - CosmicChronicle(Log/CosmicChronicle.h, MVP ④ — 순수 함수 namespace, USTRUCT 아님, 저장·해시·추첨·쓰기 없음, 앎을 읽지 않는다 §16 #49): FCosmicChronicleLine{ Entry(저장된 항목의 사본 — Message는 이미 평탄화), bOwn(그 생애의 줄 — Log.Calling, 그 백성의 Severity 2, 들인 Severity 1; 다른 백성의 Severity 2는 false — 흐리게) } · FCosmicChronicleChapter{ LifeAt(Calling.Lives의 자리), FromTick, ToTick(§7.2 Chronicle의 범위), Lines(보이는 줄, 시간순), Candidates(되풀이 ① 뒤·상한 전의 줄 수 — 커맨드렛의 'of'), bMore(상한이 줄을 뺐다 — 장 끝에 ST_CA Chronicle.More, 수 없음), Before·After(B5 — 아래 '그릇이 없던 동안') } · MaxLifeLines = 40(표시 상한 — 상태에서는 아무것도 떠나지 않는다) · PeoplesOf(Life) = BirthCivID 다음 RuledCivIDs(None·중복 없이) · Admits(Entry, Peoples) = 닫힌 #31 ②의 앞 절반(MVP ④ — #31은 B5 결정 10-나가 닫았다, `DECISIONS_LOG`; 8필드만: Log.Calling·Log.Claim·Log.Record·모든 Severity 2, FocusCivID가 Peoples에 든 Severity 1, SourceRow가 log_first_contact인 Severity 1, SourceRow가 enc_로 시작하는 Severity 1(방랑자 카드의 답 줄 — 대소문자를 가리지 않고 견준다, B1). 다만 SourceRow가 칙령의 줄 셋(CosmicRows::LogEdictEnacted·LogEdictRevoked·LogEdictLapsed — 다스리는 백성의 Severity 1)과 태세의 줄 둘(LogStanceChanged·LogStanceLapsed — B3)인 줄은 받지 않는다(B2 — 통치자의 장부라 장을 차지하지 않게. 조약과 공성의 줄은 받는다 — 생애의 이야기다) — bUntold와 행 표를 읽지 않는다. MVP ⑤의 이관 거르기 #40 ⑦도 모든 생애의 백성으로 부른다 — PeoplesOfEveryLife(Calling) = 생애 차례의 PeoplesOf를 겹침 없이) · MayLetGo(Entry) = Severity 정확히 1이고 Log.Calling·Log.Claim·Log.Record가 아님(상한이 밀어낼 수 있는 줄) · Build(State) = 보이는 생애마다 장 하나(B5 — 아래 ShownLivesOf; 생애가 없으면 없다), docs/70 §8.4의 ①~④(정수 + − × ÷만, 앎으로 줄을 세우지 않는다) = BuildCapped(State, MaxLifeLines) · BuildCapped(State, Cap) = 같은 일을 주어진 상한으로(0 아래는 0 — 테스트와 커맨드렛의 측정) · VesselNameOf(Life, Tables) = BirthArchetypeRow의(풀리지 않으면 arch_default의) PersonNameBank[NameIndex % Num](Install의 규칙), 둘 다 없으면 빈 FText('제 N대'만)
// - 그릇이 없던 동안(B5 `[B-5c]` — 헤더 B5 §2.7·§4, 결정 14: 사이의 쪽을 세우지 않는다): struct FCosmicChronicleNoVessel { int64 FromTick = 0(죽음 다음 틱); int64 ToTick = -1(구간의 마지막 틱 — 다음 생애의 BornTick(그 틱은 그 생애의 장을 여는 줄 앞까지), After면 State.Tick; FromTick보다 작으면 구간 없음 — 도약 0); TArray<FCosmicChronicleLine> Lines(보이는 줄, 시간순 — 장의 몸처럼 골라 깎는다); int32 Candidates(첫 되풀이 규칙 뒤·상한 전의 줄 수); bool bMore(상한이 줄을 뺐거나 저장된 연대기의 상한이 구간의 옛 줄을 밀어냈다); bool HasSpan() const(ToTick ≥ FromTick) } · FCosmicChronicleChapter.Before(부른 생애의 머리 — 그 생애로 이어진 그릇 없는 구간; 첫 생애는 없다)·After(마지막 장의 꼬리 — 뒤에 보이는 생애가 없을 때 죽음 뒤의 구간) · MaxNoVesselLines = 12(구간의 표시 상한 — `[B-5c]`의 초안이고 결정 22의 손 확인이 맞춘다; 생애의 이야기가 아니라 두 생애 사이의 세계 소식이다)
// - int32 CosmicChronicle::ShownLivesOf(const FGalaxyState&)(B5 `[B-5c]`: 장을 세우는 생애의 수 — State.Tick에 난 부른 생애(첫 생애가 아닌)는 빼고, 그 밖에는 Lives.Num(): 연대기는 판이 끝난 자리에서 읽히고 끝 틱이 세운 그릇은 보인 적이 없다 — 끝이 취임 쪽을 이긴다, 헤더 §2.6. Log 층은 컨트롤러의 끝 래치를 모르므로 State.Tick이 그 자리를 맡는다 — 계승의 틱에 연 개발용 미리보기도 그 생애를 숨긴다)
// - bool CosmicChronicle::OpensCalledChapter(const FCosmicLogEntry& Entry, const FLifeRecord& Called)(B5 `[B-5c]`·`[B-5d1]`: Entry가 Called — 죽음 뒤에 부른 생애 — 의 장을 여는가: Called의 BornTick 틱의 줄 가운데 SourceRow가 CosmicRows::LogInauguration이거나 그 둘째 행 LogInaugurationUnnamed인 줄(Succeed가 Install 뒤에 말한다 — 한 틱에 계승은 하나라 그 줄도 하나)만 true. 그 틱의 그 앞 줄은 그릇 없는 구간(도약 > 0)이거나 그 틱에 죽은 생애의 장 끝(도약 0 — 그 죽음 줄도)이며 그 줄부터가 Called의 것이다. BornTick 틱에 그런 줄이 없는 부른 생애(취임 줄 전의 세이브)는 그 틱 뒤에 열린다 — 도약 > 0이면 BornTick 틱 전부가 구간의 것이고 도약 0이면 그 틱에 죽은 생애의 장이 갖는다. 이 자름의 규칙 하나를 BuildCapped(다음 장의 Before와 몸, 도약 0의 죽은 장의 끝, 끝 틱에 난 생애가 보이지 않을 때 마지막 장의 After — 취임 줄은 Chronicle.NoVessel 아래에 서지 않는다)와 CosmicDigest::DigestOver(구간의 끝)가 함께 읽어 연대기와 다이제스트가 같은 줄을 읽는다)
// - bool CosmicChronicle::IsUnseenLifeLine(const FGalaxyState&, const FCosmicLogEntry& Entry, bool bRunEnded)(B5 `[B-5d1]` — 헤더 B5 §4: 플레이어가 본 적 없는 생애의 줄인가 — bRunEnded(표현의 끝 래치 ACosmicPlayerController::IsRunEnded — 래치는 끝의 기록 State.Ending을 읽되 끝 스위치를 따른다) ∧ ShownLivesOf < Lives.Num()(끝 틱이 세운 부른 생애) ∧ OpensCalledChapter(Entry, Lives.Last())(그 생애의 취임 줄 — 어느 행이든, 그 BornTick 틱에). 끝난 자리를 읽는 화면이 그 그릇을 숨기듯(IsVesselShown) 그 줄도 숨긴다 — 로그 피드가 건너뛰고(아래 '그릇을 그리는가'의 로그 피드), 연대기의 구간은 그 줄 앞에서 멈춘다. 읽기만 하고, 판이 이어지는 동안은 false)
// - Build·BuildCapped의 그릇 없는 구간(B5 `[B-5c]`): 죽은 생애의 몸은 그 DiedTick의 줄로 닫고, 그 뒤부터 다음 생애의 몸 앞까지가 구간이다 — 몸과 같은 네 걸음을 보이는 생애의 백성으로(모든 생애의 백성에서 보이지 않는 생애 — 끝 틱에 난 생애 — 의 것만 빠진다; Log.Claim·Log.Record는 몸처럼 뺀다 — 죽음 바로 뒤의 '내 것'이 없게), 상한은 min(Cap, MaxNoVesselLines); 다음 장의 Before에, 뒤에 보이는 생애가 없으면 마지막 장의 After에 싣는다(끝 틱에 난 생애가 있으면 그 생애가 열 자리까지). 앞 생애가 죽지 않은 생애(지금 코드에는 그런 생애가 서는 길이 없다)는 BornTick 틱의 첫 줄부터다. 쪽은 구간의 줄을 장 머리(마지막 장이면 끝)에 ST_CA Chronicle.NoVessel 아래 싣고, 줄이 없는 구간은 머리도 세우지 않는다(UCosmicChronicleWidget — docs/70 §8.4)
// - 두 책 장(B5 `[B-5d1]` — 헤더 B5 §4, 결정 10·11; 글과 쪽의 규칙은 docs/70 §8.4 '두 책 장'): 생애 장들 뒤·에필로그의 고정 줄 앞의 장 하나 — 판의 것이라 모든 생애와 구간을 함께 읽고, 보일 때 짓고 저장하지 않는다. struct FCosmicClaimedGroup { FName TranscendentID(줄들의 SourceRow — 1인칭 줄의 SourceRow는 그 초월자 행이다, B4; None은 그 이름이 없는 줄의 묶음); TArray<FCosmicChronicleLine> Lines(보이는 줄, 시간순 — bOwn true: 장 전체가 한 불투명도); int32 Candidates(상한 전의 그 주장자의 줄 수) } — 초월자마다 묶고 백성마다 묶지 않는다(백성의 묶음은 그 백성의 버려진 이들로 읽힌다) · struct FCosmicTwoBooks { TArray<FCosmicChronicleLine> Recorded(1부, 시간순 — MaxBookItems까지); int32 RecordedCandidates(Log.Record 항목 전부); bool bRecordedMore(1부를 Chronicle.More로 닫는다 — 상한이 줄을 뺐거나, 2부에 항목이 있는데 1부가 비었다: 기록 없는 잃음, 곧 `[B-5d1]` 전의 세이브 — 맨 머리로 두지 않고 말한다); TArray<FCosmicClaimedGroup> Claimed(2부 — 묶음은 첫 줄의 차례로, 묶음 안은 시간순); int32 ClaimedCandidates(Log.Claim 항목 전부); bool bClaimedMore(상한이 줄이나 묶음을 뺐다); bool IsEmpty() const(두 부 다 항목이 없다 — 장 없음); int32 NumRows() const(1부의 머리·줄·'더', 2부가 있으면 그 머리·묶음마다 머리와 줄·'더'; MaxBookLines 이하, 장이 없으면 0 — Title이 없는 주장자의 머리는 쪽이 빼므로 쪽은 그만큼 적다) } · MaxBookItems = 12(부마다 항목) · MaxBookLines = 28(장 전체 — 모든 머리와 '더'를 센다; 둘 다 표시 상한 — 상태에서는 아무것도 떠나지 않는다) · FCosmicTwoBooks BuildTwoBooks(const FGalaxyState&)(State.Chronicle 다음 State.Log의 Log.Record·Log.Claim 두 범주만 — 죽음·취임·징조(Log.Sign은 범주로 갈린다)·그릇의 줄은 들지 않는다; bUntold와 Knowing을 읽지 않고(§16 #49) 되풀이 규칙이 없다 — 줄 하나가 잃음 하나다. 2부를 먼저 나눈다: Log.Claim을 SourceRow마다 묶고, 전부가 MaxBookItems 줄 안이고 1부가 남긴 행에 들면 전부, 아니면 bClaimedMore와 그 '더' 한 행을 두고 두 상한이 허락하는 동안 묶음마다 한 줄씩 차례로 나눠 준다(묶음의 첫 줄은 머리 값도 친다 — 들 수 있는 모든 주장자가 어느 주장자의 둘째 줄보다 먼저 선다), 묶음마다 처음 ⌈K/2⌉과 마지막 ⌊K/2⌋. 1부는 Log.Record 전부이고, MaxBookItems를 넘으면 자리를 먼저 2부가 보이는 주장 줄마다 그 기록 — 같은 FocusSystemID·Tick: ExpireHazard가 기록 줄을 그 1인칭 바로 앞에 말한다 — 에 주어(1부가 2부를 품는다, 결정 10-가: 보이는 주장이 1부가 뺀 곳 위에 서지 않는다; 2부는 많아야 MaxBookItems 줄이라 늘 든다), 남은 자리를 나머지 기록의 처음 ⌈K/2⌉과 마지막 ⌊K/2⌋에 주고 시간순으로 싣는다 — 기록이 없는 주장(`[B-5d1]` 전의 세이브)은 자리를 받지 않는다) · FText ClaimantTitleOf(FName TranscendentID, const FCosmicDataTables&)(묶음의 머리에 넣는 그 DT_Transcendents 행의 Title — 호칭이지 이름이 아니다, B4; 행이나 Title이 없으면 빈 글이고 쪽은 그 묶음의 줄을 머리 없이 보인다 — 게임의 Title은 스키마가 요구한다). 쪽(UCosmicChronicleWidget): 생애 장들 뒤·고정 줄 앞에 1부의 머리 ST_CA Chronicle.BookRecorded를 생애 장 머리의 꼴로(크기 26·금빛·같은 여백 — 장 하나이지 마지막 생애 장의 부분이 아니다), 그 아래 2부의 머리 Chronicle.BookClaimed(회색 18)와 주장자마다 Chronicle.ClaimedBy(회색 14 — '{Transcendent}의 말'), 넘침은 Chronicle.More; 줄은 장의 줄 꼴(16, 주기 스탬프, 불투명도 1)이고 한 색(장의 Severity 1 색 — 1인칭 줄도 보라가 아니다: 보라는 피드의 것이다); 1부는 장이 서면 늘 서고 2부는 줄이 있을 때만, 두 부 다 비면 장이 없고 제 닫는 줄이 없다(결정 10-다 — 에필로그 둘째 줄 '다른 책은 펴지지 않았다.' Chronicle.Unopened 한 번). 테스트가 읽는 GetLineColors()(줄의 글 색, 차례대로) · GetRowLooks()(GetPageTexts의 행마다 CosmicChronicleScreen::FRowLook { int32 Size; FLinearColor Color; FMargin Gap; operator== } — 글의 크기·색과 행의 여백). 연대기와 두 책은 이야기의 범위로 거르지 않는다(bUntold 줄 포함, §8.4). 연속 중복 규칙은 마지막으로 말해진 줄과만 견주므로 bUntold 항목이 같은 문장의 말해진 줄 바로 앞에 쌓이거나(예: [B(bUntold), B]) bUntold 항목을 사이에 두고 같은 문장이 이어질 수 있다 — 연대기는 바로 앞에 실은 줄과 같은 문장을 싣지 않는다(표시 규칙이고 상태는 없다, 아는 범위와 로그 작업의 리뷰). 사람·성계의 수를 렌더하지 않고(주기·비율은 허용) 인물이 없다 — 인물은 생애별 장의 이미 렌더된 Severity 2·1 Message 안에서만 등장하고 이 함수들에 인물 코드 0

// - 다이제스트(B5 `[B-5c]` — 헤더 B5 §2.7·§4; 글은 `[B-5c]`의 초안이고 `[B-5d1]`의 글 감사를 지났다 — 스키마의 `Digest.` 규칙, §7.3) (§8.4): 로그 서브시스템의 함수가 아니라 표현 계층의 순수 함수 이름공간 World/CosmicDigest다 — 앎을 CosmicMapLook::KnownViewOf 하나로 읽어야 하므로(규칙 14: 아는 성계는 뷰의 IsKnown, 백성은 그 뷰가 보이는 백성(ShownOrderOf)으로 답하는 것만 — 격차 규칙을 넷째 자리에 베끼지 않는다). 저장 필드 없이 보일 때 셈하고, 아무것도 쓰지 않고 어느 스트림도 뽑지 않아(문장 변형은 파생 정수 — 아래 VariantOf) 같은 상태는 같은 글이다. 입력은 구간의 줄(State.Chronicle 다음 State.Log)과 지금의 세계다. Log.Claim·Log.Record·Log.Calling 줄은 읽지 않는다 — 세계의 소식이고 서술자가 말한다(죽음 뒤 처음 들리는 목소리가 '내 것'이 아니게 — 결정 14). 계승에만 쓴다: 폐위·양위는 도약이 없어 다이제스트가 없고(재위 끝 쪽 — 아래 '죽음과 계승의 쪽'), B1의 복귀는 한 줄 그대로다(떠나 있던 동안의 다이제스트는 §16 #152 — 구간을 받는 DigestOver가 그 자리다). 한계(받아들임): 긴 도약에서 로그의 2,000 상한이 밀어내고 연대기로 옮기지 않은 줄은 보지 못한다
// - enum class ECosmicDigestItem : uint8 { LostPeople, Rose, Lost, Annexed, Owner, Throne, ThroneUnnamed }(World/CosmicDigest.h — 저장되지 않는다; 차례는 상한의 등급이다. VS에 재료가 있는 것만 — 봉인·구한 성계는 없다): LostPeople = 구간에 무너진 아는 성계(지금 CosmicHazards::IsDeadNode)의 백성 — 구간의 마지막 위기·프로젝트 줄이 부른 그 성계의 주인 — 을 상태가 더는 갖지 않는다(그 이름은 풀리지 않으니 곳으로 말한다) · Rose = 보이는 백성에 대한 구간의 승천 줄(DT_Feats 행의 줄) — 몇 칸이든 항목 하나라 말에 칸의 수가 없고, 화면에서 '도약'이라 쓰지 않는다(공백기의 낱말) · Lost = 구간에 무너진 아는 성계(그 백성은 지도에 남는다) · Annexed = 구간에 ClaimRule 위기 행의 줄이 있었고 지금 초월자의 영향권이 쥔 아는 산 성계(서술자 항목 — 1인칭도 주장자의 이름도 없다) · Owner = 지금 주인이 보이는 백성(OwnerShownOf Shown)인 아는 산 성계의 주인이 구간의 첫 위기·프로젝트 줄이 부른 주인(보이는 백성이거나 사라진 백성)과 다르다 — 그런 줄이 앞에 없는 바뀜은 보지 못한다(공성의 줄은 다스리는 쪽에만 말해져 공백기에는 없다 — AI끼리의 정복은 §16 #153)
// - ECosmicDigestItem::Throne = 보이는 백성의 자리가 구간에 바뀌었고 그 열린 Notable.Ruler 기록의 이름으로 말한다. 표지는 둘뿐이고 저마다 바뀜 없이는 서지 않는다: 구간에 닫힌 Notable.Ruler·Notable.Regent 기록(왕좌를 앗은 반란, 그 자리에 이름 붙은 다스리는 이), 그리고 반란이 다스리는 이로 만든 반란자 — SourceRow가 반란의 발생 행인 Notable.Ruler 기록이고 그 틱은 그 기록의 FirstTick 뒤 그 백성에 대한 첫 반란 줄(행은 태그 Hazard.Rebellion으로 찾는다 — 행 이름 리터럴 없음), State.Log가 발생의 말해진 줄을 아직 들 때만(처음 왕좌를 앗은 반란자는 닫는 기록이 없다). 해결된 반란은 다스리는 이를 만들지 않고, 두 표지가 다 남지 않은 자리 바뀜은 말하지 않는다 — 없는 바뀜을 말하지 않는다(한계: 칸이 내려가지 않고 SetTellAll이 판 도중에 바뀌지 않는다는 데 기댄다; 발생 줄이 로그를 떠났거나 범위 밖이었던 바뀜과 넷째 칸 이상의 백성은 말하지 않는다) · ThroneUnnamed = 같은데 말할 이름이 없다(열린 Notable.Ruler 기록이나 이름 뱅크가 없다)
// - struct FCosmicDigestItem { ECosmicDigestItem Kind; FName SubjectID(변형의 이름 — LostPeople·Lost·Annexed·Owner는 SystemID, Rose·Throne·ThroneUnnamed는 CivID); FName CivID(줄이 부르는 백성 — Owner는 지금 주인, Rose와 왕좌 항목은 그 백성; LostPeople·Lost·Annexed는 None); int64 Tick(말하는 틱 — 변형의 번호이자 쪽의 차례: 성계 항목은 그 성계에 대한 구간의 마지막 줄(Annexed는 마지막 주장 줄), Rose는 마지막 승천 줄, 왕좌 항목은 마지막 자리 바뀜); int32 Variant(0 ~ VariantCount − 1 — VariantOf); FText Line } · struct FCosmicDigest { bool bValid(닫힌 생애 뒤 구간이 있거나 도약 0이다 — 닫힌 생애가 아닌 자리와 죽음의 틱 자체(아직 아무것도 지나지 않았다)는 false); int64 AfterTick; int64 ThroughTick(구간 (AfterTick, ThroughTick] — 닫힌 생애의 DiedTick, 그리고 다음 생애의 BornTick(그 틱은 그 생애의 장을 여는 줄 — 취임 줄 — 앞까지: CosmicChronicle::OpensCalledChapter, 연대기와 같은 자름; 그 틱에 취임 줄이 없으면 그 틱 전부) 또는 공백기가 이어지면(끝이 공백기 안) State.Tick); bool bNoLeap(다음 생애가 죽음의 틱에 섰다 — 도약 0, LeapTotalCap이 찼다: 한 줄); TArray<FCosmicDigestItem> Items(MaxItems까지, 틱 차례 — 같은 틱이면 등급, 그다음 성계가 백성보다 앞, 배열 차례); bool bMore(상한이 항목을 뺐다); bool bElsewhere(구간에 앎 밖에서 바뀐 것이 있다: 먼 줄(log_far_event·log_far_ascent), 뷰가 보이지 않는 주제의 줄 — 모르는 성계(계승의 Install이 알게 한 것도), 상태에 있지만 보이지 않는 백성, 아는 성계를 부르지 않는 사라진 백성 —, 보이지 않는 백성에게로·에게서 주인이 바뀐 아는 성계(Owner 항목 없음), 보이지 않는 백성의 자리 바뀜. 쪽은 항목 뒤에만 보인다) }
// - namespace CosmicDigest: MaxItems = 5(머리·항목 다섯·상한 줄·닫는 줄 — 5~8줄의 예산) · MaxThrones = 2(왕좌 항목은 가장 늦은 둘까지 — MaxItems 안에서 다른 항목과 겨룬다) · VariantCount = 3(ST_CA <키>.1 ~ .3) · 고정 키 HeadKey "Digest.Head"({0} = 구간의 틱 수, UI 숫자)·QuietKey "Digest.Quiet"·ElsewhereKey "Digest.Elsewhere"·NoLeapKey "Digest.NoLeap"·MoreKey "Chronicle.More"(연대기의 말 — 수 없음) · const TCHAR* KeyOf(ECosmicDigestItem Kind, int32 Variant)(Digest.<Kind>.<Variant + 1>, 범위 밖의 변형은 가까운 쪽으로; 슬롯 — LostPeople·Lost·Annexed {System}, Owner {System}·{Civ}, Rose·ThroneUnnamed {Civ}, Throne {Civ}·{Ruler}) · int32 VariantOf(int32 Seed, FName SubjectID, int64 Tick)(CosmicSeed::DeriveForName(Seed, SubjectID, Purpose::Digest 12, (uint32)Tick) % VariantCount — 말하기 스트림도 줄의 추첨도 시뮬 스트림도 아니다, §9.2 ③) · int32 LastClosedLifeOf(const FGalaxyState&)(가장 최근에 닫힌 생애의 자리 — 공백기면 마지막 생애, 계승이 뒤따랐으면 그 앞 생애; 닫힌 생애가 없으면(DiedTick 0 — 첫 생애, 그리고 폐위·양위는 생애를 닫지 않는다) INDEX_NONE)
// - FCosmicDigest CosmicDigest::DigestOf(const UGalaxySimulationSubsystem&, int32 ClosedLifeIndex, bool bSeeAll = false)(그 생애 뒤 구간의 다이제스트 — 산 상태·색인·테이블과 지금의 보는 쪽 Tier(GetViewerTier — 지난 칸은 남지 않는다). 다음 생애가 있으면 앎은 그 생애 전의 것(KnownViewOf를 그 BornTick에서 자른다 — 계승의 Install이 그 틱에 적은 출생 성계·출생 백성·방랑자의 이웃은 그릇이 없던 동안의 앎이 아니어서 항목 없이 bElsewhere로 간다)이고 그 틱은 장을 여는 줄 앞까지 읽는다. bSeeAll = 개발용 CosmicSeeAll(컨트롤러의 IsSeeingAll — KnownViewOf가 읽는 대로)) · FCosmicDigest DigestOver(const UGalaxySimulationSubsystem&, int64 AfterTick, int64 ThroughTick, bool bSeeAll = false)(주어진 구간의 같은 셈 — 다음 생애 없이: ThroughTick의 모든 줄과 지금의 앎. 다른 구간의 다이제스트를 다시 쓰는 자리(귀환 — §16 #152). ThroughTick ≤ AfterTick이면 무효) · FCosmicDigest DigestOver(Sim, AfterTick, ThroughTick, bool bSeeAll, const FLifeRecord* Next)(같은데 Next(닫힌 생애 뒤에 부른 생애, ThroughTick = 그 BornTick)가 있으면 그 틱을 장을 여는 줄 앞까지 읽고 앎을 그 BornTick에서 자른다; null이면 위의 꼴 — DigestOf가 이것을 부른다)
// - TArray<FText> CosmicDigest::LinesOf(const FCosmicDigest&)(쪽의 줄: 무효면 없음, 도약 0이면 Digest.NoLeap 하나, 항목이 없으면 머리와 Digest.Quiet 둘만 — 닫는 줄을 붙이지 않는다('세계는 흘렀다.'는 어느 길에서도 참이고 아무 일도 없었다고 말하지 않는다 — 줄을 남기지 않은 바뀜은 다이제스트가 보지 못한다), 그 밖에는 머리 → 항목의 줄 → 상한이 뺐으면 Chronicle.More → 앎 밖의 바뀜이 있으면 Digest.Elsewhere) · TArray<FText> LinesOf(Sim, int32 ClosedLifeIndex, bool bSeeAll = false)(DigestOf 다음 LinesOf — 쪽이 부르는 꼴)

// UGalaxySimulationSubsystem — 인물 훅 ② (§7.4 TickHazards 주석): crisis_rebellion 만료로 왕좌가 바뀌면 열린 Ruler·Regent 닫기 + Rebel → Ruler, 먼저 적힌 다른 원천이 재위를 끝냈으면 Rebel 닫기(B1), 해결 시 Rebel 닫기

// UPlayerRoleSubsystem

// - void BecomeWanderer(FGameplayTag RegentProfile, FGameplayTag ReignEndReason)  통치자 → 방랑자(§5.10): Role = Wanderer, FormerCivID = 다스리던 백성, RuledCivID 비움, 그 백성은 RegentProfile로 AI가 다스린다. ReignEndReason = Reign.Abdicated / Reign.Deposed / Reign.Died. StatusTags는 함수 안에서 파생(Abdicated → Status.Abdicated, Deposed → Status.FallenMonarch, Died → 없음). 열린 재위를 그 사유로 닫는다. 역할이 바뀌면 보는 눈이 바뀐다: 방랑자가 되는 틱에 선 성계와 그 1레인 이웃(같은 GalaxyRow, Lanes 차례)을 NoteKnown으로 적는다(추첨 없음, 줄 없음 — 그 주인을 만나는 것은 다음 앎 단계, §4.4). 기다리는 카드와 Knowing.PendingContactCivID, 두 이동 필드를 비운다 (§8.1). 그릇이 다스리지 않으면 아무것도 하지 않고, 핸들러 안과 성역에 선 그릇(어느 규칙도 만들지 않는 상태 — Install이 막는다)은 에러 한 줄로 거절한다. 왕좌를 놓는 앞 절반(새 눈을 뺀 모두 — 백성·역할·상태 태그·재위·카드·여정·섭정 훅)은 비공개 ReleaseThrone(RegentProfile, ReignEndReason, RegentSourceRow)이고 Die가 Reign.Died로 함께 부른다(B5); 새 눈(선 성계와 이웃의 NoteKnown)은 이 함수의 몫이다 — 죽음은 둘러보지 않는다

// - 인물 훅 ① (§7.2): BecomeWanderer는 ReignEndReason이 Reign.Abdicated일 때(B1) 그 문명에 Notable.Regent 기록 1개를 NameNotable의 규칙으로 연다 — UCosmicEventSubsystem::Apply를 거쳐 이름의 수 1회를 가드보다 먼저 뽑고(Tier <= 3·이름 뱅크·Trait.Mind.NoSelf·같은 역할의 열린 기록 닫기·상한과 퇴출 — 기록이 서지 않아도 1회), SourceRow = log_throne_vacated(`CosmicRows::LogThroneVacated` — 그 줄은 양위 파이프라인이 같은 틱에 먼저 Push한다). 섭정 프로필은 그대로 AI.Profile.* 태그이고 인물은 이름만 더한다. Reign.Died도 같은 훅을 쓴다(B5 — 훅은 BecomeWanderer와 Die가 함께 부르는 비공개 ReleaseThrone에 있고, Die가 넘기는 SourceRow는 KillPlayer의 SourceRow = PendingSourceRow). Reign.Deposed에는 만들지 않는다 — 왕좌에는 Rebel이 앉는다(§4.1). 열린 Regent 기록이 닫히는 곳: 즉위·복귀(BecomeRuler)와 반란 만료의 왕좌 교체(§7.4 인물 훅 ②)

// - CosmicWanderer::ECrownBlock BecomeRuler(FName CivID)  즉위와 복귀(B1 — 옛 백성에 대한 즉위가 복귀다, 따로 된 함수는 없다), 틱 밖 입력. CosmicWanderer::CrownBlockOf가 None일 때만(만났는가는 Knowing.MetCivs에서, 카드가 기다리는가는 Knowing에서 — 입력의 정의). 쓰는 것, 차례대로: 그 백성의 bIsPlayerRuled, Role = Ruler·RuledCivID → 재위 세 배열에 한 칸씩 → 그 백성의 열린 Notable.Ruler·Notable.Regent 기록을 닫는다(LastTick = max(Tick, 1) — 플레이어가 앉은 왕좌에는 다른 이름이 남지 않는다, 플레이어는 Notables에 없음, §7.2) → 두 이동 필드·기다리는 카드·Knowing.PendingContactCivID를 비운다 → 그 백성의 함대 목표가 다른 백성의 성계면 푼다(FleetTargetSinceTick = Tick)·그 백성의 적대를 모두 중립으로 되돌린다(B3 — 새 통치자는 모르는 성계를 두고 보이지 않는 공성이나 바꿀 수 없는 적대를 물려받지 않는다. 조약과 지킴은 그대로) → 그 백성의 소유 성계를 OwnedSystems 차례로 NoteKnown(줄 없음) → log_crowned, 옛 백성(FormerCivID)이면 log_returned(Log.Calling, Severity 2, 초점은 그릇이 선 성계와 그 백성). FormerCivID·StatusTags·생애의 PeakTier는 그대로. 추첨 없음 — 다음 틱 경계의 Role 비교가 저장한다(MVP ⑤). 거절은 첫 사유를 돌려주고 아무것도 바꾸지 않는다. 핸들러 안은 에러 한 줄과 Handler

// - 즉위·복귀는 Tick >= ThroneLockedUntilTick 검사 (§5.10) — 잠금은 그릇이 왕좌를 떠나는 틱에 모든 백성에게 적힌다(DeposeRuler·AbdicateRuler, B1; 통치자의 죽음 Die, B5)

// - bool Abdicate()  통치자의 양위(B1), 틱 밖 입력: Effect.DeposeRuler를 원인 Reign.Abdicated, SourceRow log_throne_vacated로 UCosmicEventSubsystem::Apply를 거쳐 기록만 한다(문맥은 다스리는 백성 — 먼저 기록된 효과가 이긴다). 다음 틱의 열째 단계가 AbdicateRuler를 부른다. 추첨 없음. 몸을 입은 통치자이고 기록된 역할 효과가 없을 때만 참이고, 아니면 거짓·바뀜 없음. 핸들러 안은 에러 한 줄. 왕좌 카드(petition_keep_throne)의 셋째 선택지도 같은 효과를 적는다(그때 SourceRow는 그 카드 행)

// - void AbdicateRuler()  양위 파이프라인 — 열째 단계가 Effect.DeposeRuler의 원인 Reign.Abdicated에 부른다(B1): log_throne_vacated(Severity 2, 다스리던 백성) → 섭정 성향을 시뮬 스트림에서 1회(CosmicState::PickAIProfile) → BecomeWanderer(그 성향, Reign.Abdicated — 인물 훅 ①이 섭정 이름을 1회 뽑는다) → 떠나는 백성의 Stability −DT_Rules.AbdicationStabilityLoss(0 아래로 가지 않는다) → 모든 백성의 왕좌 잠금(max(제 값, Tick + ThroneLockTicks)). 경외는 줄이지 않는다(폐위와 다르다). 시뮬 스트림은 언제나 정확히 2회. 그릇이 다스리지 않으면 아무것도 하지 않고, 핸들러 안은 에러 한 줄

// - 사망 파이프라인(B5, 헤더 B5 2.2절 — 차례 전문은 §7.4 ResolvePendingRoleEffect, 호출자는 그것만) (§4.4): void Die() — 열째 단계가 기록된 Effect.KillPlayer를 받아 부른다(생애를 닫고 공백기에 든다 — 시뮬 수는 방랑자의 죽음 1, 통치자의 죽음 3; 생애를 닫은 바로 뒤 죽음 줄 하나(`[B-5d1]`) — 말하기 1수, 차례는 §7.4). void Succeed() — 카운트다운이 끝나는 틱(폭이 0이면 Die 안에서 죽음의 틱)에 계승 추첨 1수 뒤 후보(CosmicSuccession::CandidatesOf)에, 개인 칸을 낮추고(셋째 칸 아래는 첫째로 — Install이 PeakTier로 읽기 전에) 권능은 Way.*만 남긴 뒤 Install(…, Wanderer)을 부른다(SuccessionKind = Succession.Called; 후보가 없으면 한 수를 버리고 공백기에 머문다; Install 뒤 취임 줄 하나(`[B-5d1]`) — 두 행 가운데 하나, 말하기 1수, §7.4). Die는 몸을 입은 부름에서만, Succeed는 공백기에서만 돈다(그 밖은 아무것도 하지 않는다); 핸들러 안은 에러 한 줄. 도약은 틱을 돌리지 않는다 — 한 번에 넘기는 것은 표현 계층의 입력(ACosmicPlayerController::AdvanceLeap)이 부르는 RunLeap(§7.4). 귀환 제안(OfferReturn)은 이 파이프라인에 없다(아래 — §16 #146)

// - bool OfferReturn()  귀환 제안 훅 (B5 밖 — §16 #146; §4.4 회귀의 유혹, 약 40줄). 게이트: Doctrine.Recurrence 초월자 존재 ∧ 미봉인 ∧ (bAwareOfPlayer ∨ 사망 성계가 영향권 ReturnLureMaxLanes 이내), 생애당 1회(bReturnOffered). enc_recurrence_return 1장을 WBP_EncounterCard로 제시. 수락 시 도약 0(PendingLeapTicks를 두지 않는다)·Install(사망 성계, 직전 생애의 BirthCivID, 사망 당시 Role) — 귀환은 '같은 몸'의 약속이므로 BirthCivID·BirthArchetypeRow(이름 뱅크)·NameIndex를 직전 생애와 같게 유지한다(사망 성계의 소유 문명이 달라도 뱅크는 바뀌지 않는다) — (SuccessionKind = Succession.Returned; 통치 문명의 소유 성계가 하나 이상이면 Ruler로 복위·섭정 해임·ThroneLockedUntilTick 미적용, 아니면 Wanderer)·Exaltation +2·Status.Returned·옛 모성계 편입(그 성계가 출생 문명의 마지막 미편입 성계면 편입되지 않는다 — Law.Remnant, §16 #43), ReturnedLivesToSeize회면 GrantPower Way.Seized. 거절 시 Exaltation −1, Log.Calling "제안이 있었다. 거절되었다." 후 파이프라인 계속. VS에는 회귀가 없어 뜨지 않는다 — 훅·테스트 주입 행·실전 모두 회귀 교리 행이 오는 블록의 일이다(§16 #146. 수락의 'ThroneLockedUntilTick 미적용'은 통치가 끝나는 모든 길의 왕좌 잠금(B5 결정 4가 #143을 닫은 구조)과 부딪는다)

// - 틱 경계에서 DeposeRuler()  폐위 파이프라인 (§5.10) — 열째 단계가 Effect.DeposeRuler(원인이 Reign.Abdicated가 아닐 때)에 부른다: log_fallen_monarch(Severity 2) → 섭정 성향을 시뮬 스트림에서 1회 → BecomeWanderer(그 성향, Reign.Deposed — 방랑자가 되는 틱에 선 성계와 1레인 이웃을 NoteKnown, §4.4) → 모든 백성의 ReverenceToPlayer × DT_Rules.ReverenceDecay → 다스리던 백성의 Stability ≤ PostRebellionStability → 모든 백성의 왕좌 잠금(max(제 값, Tick + ThroneLockTicks) — B1, 전에는 옛 백성만). 그릇은 선 자리에 남는다. 화면은 재위 끝 쪽 하나다(B5 `[B-5c]` — 3단도 다이제스트도 아니다, 결정 16; 표현 계층 — 아래 '죽음과 계승의 쪽'). 망명 조우는 §16 #148. 그릇이 다스리지 않으면 아무것도 하지 않고, 핸들러 안은 에러 한 줄

// - Install(FName SystemID, FName CivID, EPlayerRole Role)  첫 생애(NewGalaxy, Ruler)와 계승(Wanderer)이 같은 함수. 은하 인자는 받지 않는다 — 계승은 은하를 넘지 않는다(§4.4. 시그니처의 미결은 §16 #34 그대로). 그 틱에 출생 성계와 출생 문명을 Knowing에 적는다(추첨 없음, 줄 없음 — 만남이 아니므로 NoteMet을 거치지 않고 FirstContactCivID도 적지 않는다). Role이 Wanderer면 선 성계의 1레인 이웃도 NoteKnown으로 적는다 (§4.4). 기다리는 카드(CosmicPetitions::Clear)와 첫 접촉 카드의 대기 칸(Knowing.PendingContactCivID — B3)을 비운다 — 지난 생애의 카드는 새 생애의 것이 아니다. 그릇의 이름 FLifeRecord.NameIndex는 Install마다 시뮬 스트림에서 정확히 1회, 뱅크를 보기 전에 뽑은 수로 고른다 — 쓸 뱅크가 비었으면 그 수를 버리고 NameIndex는 0이다(NameNotable과 같은 규칙이라 뱅크를 뒤에 채워도 난수열이 밀리지 않는다, §9.2)(§4.4): Succession.Installed·Called = 새 인덱스(첫 생애가 아니고 뱅크에 이름이 둘 이상이면, 직전 생애의 이름과 원문 — FTextInspector의 source string, 표시 문화권과 상관없이 — 이 같은 그 뱅크의 첫 항목을 건너뛴다: 두 아키타입이 뱅크를 나눌 수 있다, B5 결정 8), Succession.Returned = 직전 생애와 같은 인덱스, 뱅크가 비면(이름 없는 문명) arch_default 뱅크. 플레이어는 Notables에 넣지 않는다. FLifeRecord.BirthArchetypeRow에 출생 문명의 ArchetypeRow를, PeakTier에 Calling.PersonalTier를 적고(계승은 그 전에 개인 칸을 낮춘다 — Succeed), Role이 Ruler면 재위 3배열에 한 칸을 더한다(RuledCivIDs = CivID, ReignEndReasons = 빈 태그, ReignTicks = 0, §7.2). 성역(`sol`)은 받지 않는다 — 에러 한 줄로 거절하고, 아무것도 바꾸지 않으며 뽑지 않는다(A0 4a).

// - CosmicWanderer::ETravelBlock TravelTo(FName SystemID)  방랑자의 이동(B1), 틱 밖 입력 — 명령 한 번에 레인 하나: CosmicWanderer::TravelBlockOf가 None일 때만(카드가 기다리는가는 Knowing에서 — 입력의 정의) Player.TravelTargetSystemID = SystemID, TravelTicksRemaining = CosmicWanderer::LaneTicksOf. 그릇은 여덟째 단계가 옮긴다(§7.4 TickPlayerTravel). 추첨·줄·제 저장 없음. 앎에서 읽는 것은 기다리는 카드 하나다(입력의 정의) — 아는 성계는 읽지 않는다. 거절은 첫 사유를 돌려주고 아무것도 바꾸지 않는다 — 이동 중에 다시 고르지 않는다(Travelling). 핸들러 안은 에러 한 줄과 Handler. 다리는 TravelTo의 대상이 아니다 (§5.9). TravelTo(sol)은 사유 Sanctuary로 거절한다 — 성역에는 레인이 없다. 성역의 경계 카드 '지경'과, 경로가 성역을 거쳐 가는가는 성역에 레인이 생길 때(B8, §16 #80·#81). 여러 레인을 잇는 경로·T3 도약(아는 성계만 받는다 — 거절된 입력은 아무 상태도 바꾸지 않는다, §8.1)은 B1 밖

// - int32 ScanCurrentSystem()  방랑자의 스캔(B1, §5.9), 틱 밖 입력: 이동 중이 아닌 몸을 입은 방랑자가 선 성계에서 보기 한 번(UGalaxySimulationSubsystem::LookFrom) — 보는 거리 = SightLanesByTier[PersonalTier](없는 칸·1 아래는 1) + DT_Rules.ScanSightBonusLanes(0 아래는 0). NoteKnown을 찾은 차례로, 이어서 NoteMet을 가까운 순으로 — 앎과 이야기만, 시뮬 추첨 없음. 그 성계에 주인이 없고 새로 안 성계가 있으면 log_scan_wild(Severity 1, 말하기 스트림) 한 줄(§5.7). 새로 안 성계의 수를 돌려준다(0 = 새것 없음 — 줄 없음, 화면이 그렇게 말한다). 통치자·이동 중·공백기는 INDEX_NONE, 핸들러 안은 에러 한 줄과 INDEX_NONE. 쿨다운 없음 — 같은 자리의 두 번째 스캔은 0

// - Train(FName Stat)  수련 — B1 밖(뒤의 블록)

// - 통치자 명령: SetResearchFocus, SetResearchShares(B2), EnactEdict·RevokeEdict(B2), StartMegaproject, SetStance·ProposeTreaty·BreakTreaty·AllocateFleet(B3 — 아래 '외교와 분쟁')  만나지 않은 백성에게는 태세·조약·즉위를 걸 수 없다 (§8.1). StartMegaproject의 대상에는 정착(project_settle)이 든다 — 새 명령이 아니다 (§5.7). bool StartMegaproject(FName SystemID, FName RowName)은 거대 프로젝트와 승천 판정 작업부터: Embodied인 통치자가 통치 문명의 성계에, CosmicMegaprojects::Start(AI와 같은 규칙)로 시작한다 — project_settle만은 SystemID가 정착할 무주 성계이고 CosmicMegaprojects::Settle(AI의 정착 규칙, 추첨 없음)로 시작한다(정착과 문명 제거 작업). 틱 사이 입력이라 다음 틱의 위기 단계 ⓐ에서 처음 나아간다. 핸들러 안에서는 에러 한 줄. 프로토타입 키(P)의 StartNextMegaproject는 통치 문명에 CosmicMegaprojects::Choices의 첫 것(SinceTick = Calling.BornTick)을, 그것이 없으면 SettleTargets의 첫 대상에 정착을 시작한다(Phase 3의 패널이 대신한다)
// - 연구 중점(MVP ⑦, 2026-10-24 — 헤더 docs/60_DECISIONS/headers/A7_research_focus.md): bool SetResearchFocus(FName Field) — 틱 밖 입력. Field = 다스리는 백성의 ResearchFocus의 키는 DT_Rules.ResearchFocusShare를(CosmicResearch::Focus — 효과 FocusResearch의 규칙), NAME_None은 균형(CosmicResearch::Even). Embodied 통치자에게 상태에 있는 다스리는 백성이 있을 때만. 거짓·바뀜 없음: 키 아님, 몫이 그대로(같은 중점 — 결정이 아니다), 분야인데 노브가 고를 수 없는 값(고른 몫 이하·1 초과·NaN — 에러 한 줄), 핸들러 안(에러 한 줄). 추첨·줄·저장 없음. 초월도 받는다(패널의 단추는 균형과 셋). 컨트롤러의 ACosmicPlayerController::SetResearchFocus가 끝난 판을 거절하고 받으면 HUD를 다시 그린다
// - 연구의 몫(B2, 2026-10-25 — 헤더 docs/60_DECISIONS/headers/B2_edicts_and_research.md 2.3·2.8절, §16 #119 ③): bool SetResearchShares(const TMap<FName, int32>& Weights) — 틱 밖 입력. 몸을 입은 통치자에게 상태에 있는 다스리는 백성이 있을 때만, 그 백성의 ResearchFocus를 CosmicResearch::SharesOf로 고친다(물리·사회·공학의 무게 0~20, 초월의 몫은 그대로 — §7.4 연구 중점). 거짓·바뀜 없음: 무게가 맞지 않는다(SharesOf가 거절), 몫이 그대로(같은 지도 — OrderIndependentCompareEqual), 통치자가 아니다, 핸들러 안(에러 한 줄). 추첨·줄·제 저장 없음(SetResearchFocus와 같은 자리 — 다음 경계의 정해진 까닭에 함께 저장된다)
// - 칙령(B2 — 헤더 B2 2.3절): CosmicEdicts::ECosmicEdictBlock EnactEdict(FName Row) — 틱 밖 입력. 핸들러 안이면 Handler(에러 한 줄), 몸을 입은 통치자의 다스리는 백성이 없으면 NotRuler, 그다음 CosmicEdicts::EnactBlockOf(그 백성)의 첫 사유 — 어느 것이든 바뀜 없음. None이면 EnactCost를 모두 내고 ActiveEdicts 끝에 더하며(CosmicEdicts::Enact) 줄 log_edict_enacted(Severity 1, 말하기 스트림 1회)를 낸다. bool RevokeEdict(FName Row) — 지닌 행을 차례를 지켜 뺀다(CosmicEdicts::Revoke). 값은 없고 다시 내리면 제정의 값을 다시 낸다. 줄 log_edict_revoked(테이블에 없는 이름은 줄 없이 빠진다 — 부를 이름이 없다). 거짓·바뀜 없음: 통치자가 아니다, 지니지 않은 행, 핸들러 안(에러 한 줄). 둘 다 시뮬 스트림을 뽑지 않고 제 저장이 없다. 세 명령은 핸들러 가드의 목록(Sim/GalaxySimulationSubsystem.h)에 든다
// - 외교와 분쟁(B3 — 헤더 B3 2.4절): 틱 밖 입력 넷. 공통 — 핸들러 안이면 에러 한 줄과 Handler(BreakTreaty는 거짓), 몸을 입은 통치자의 다스리는 백성이 없으면 NotRuler(BreakTreaty는 거짓), 거절은 첫 사유를 돌려주고 아무것도 바꾸지 않는다. 만났는가는 SetStance·ProposeTreaty만 Knowing.MetCivs에서 읽는다(입력의 정의 — 불변 규칙 2) — BreakTreaty는 지닌 조약만 묻고, AllocateFleet은 앎을 읽지 않는다(목록과 까닭은 docs/20 부록 '불변 규칙 전문'의 상태 문단). 시뮬 스트림을 뽑지 않고 줄은 말하기 스트림만, 제 저장이 없다(다음 경계의 정해진 까닭에 함께 저장된다 — EnactEdict와 같은 자리). 넷 다 핸들러 가드의 목록에 든다
//   · CosmicDiplomacy::ECosmicStanceBlock SetStance(FName CivID, FGameplayTag Stance)  Stance = Stance.Friendly·Stance.Hostile·빈 태그(중립). CosmicDiplomacy::StanceBlockOf가 None이면 쓴다 — 적대는 CosmicDiplomacy::TurnHostile(그 쌍의 무역 조약을 끊고, 끊었으면 그 상대의 눈 −BetrayalRelationPenalty). 우호는 한 번의 값이 없고 서 있는 동안 자리 하나와 틱당 영향력을 쓴다(외교 단계). 그 상대를 에워싸던 공성은 적대를 풀면 다음 분쟁 단계의 점검이 푼다(SiegeBlockOf의 NotHostile — 명령은 목표를 건드리지 않는다). 줄 log_stance_changed(Severity 1 — 연대기는 받지 않는다), 무역 조약이 끊겼으면 log_treaty_broken
//   · CosmicDiplomacy::ECosmicTreatyBlock ProposeTreaty(FName CivID, FGameplayTag Treaty)  Treaty = Treaty.Trade·Treaty.NonAggression. 승낙은 곧 판정한다(추첨 없음): TreatyBlockOf가 None이면 TreatyCostInfluence를 내고 CosmicDiplomacy::Sign(두 쪽에 함께), 줄 log_treaty_signed(Severity 1 — 연대기가 받는다). 거절(Refused 포함)은 줄 없이 사유로만 돌아온다(화면의 툴팁)
//   · bool BreakTreaty(FName CivID, FGameplayTag Treaty)  지닌 조약만: CosmicDiplomacy::Break(두 쪽에서), 그 상대가 다스리는 백성을 보는 눈 −BetrayalRelationPenalty(곧), 줄 log_treaty_broken. 상대가 없거나 자신이거나 지니지 않은 조약이면 거짓
//   · CosmicConflict::ECosmicSiegeBlock AllocateFleet(FName SystemID)  NAME_None = 함대를 물린다(목표가 있었으면 비우고 FleetTargetSinceTick = Tick — 늘 받는다). 제 성계면 CosmicConflict::GuardBlockOf(사유는 SiegeNameOf로 공성의 이름), 남의 성계면 SiegeBlockOf. None이면 FleetTargetSystemID·FleetTargetSinceTick = Tick — 같은 목표를 다시 고르면 아무것도 바꾸지 않는다(공성 시간을 되감지 못하게). 공성을 시작하면 줄 log_siege_begun(Severity 1 — 플레이어가 방금 한 일이라 자동 정지하지 않는다, 연대기는 받는다)
//   · AI의 외교는 명령이 아니다 — AI 단계(§7.4 TickCivAI)가 세계에 직접 쓴다. AI는 조약을 제안하지 않고 통치자의 제안에 TreatyBlockOf로 답할 뿐이다(B7)
// - 컨트롤러(World/CosmicGameMode.h, B2): ACosmicPlayerController::EnactEdict·RevokeEdict·SetResearchShares가 끝난 판을 거절하고 명령으로 넘긴다. 받아들인 명령은 HUD를 곧 다시 그린다(멈춘 시뮬은 틱을 보내지 않는다. 칙령도 연구도 지도에 없어 지도는 다시 그리지 않는다) — 칙령의 두 명령은 그 앞에 첫 칙령 안내(Hint.Edict — 물려받은 칙령이 안정도를 깎고 있음을 알리는 화면의 안내, 세션마다 한 번)의 상태를 한 번 갱신한다(FollowEdictHint — CosmicUI::StepEdictHint). ToggleEdictWindow(상단 바의 칙령 단추)·OpenEdictWindow(Tab)(패널의 [직접 나눈다] — 연구의 몫 쪽)·NoteEdictWindowOpened(창이 열리면 그 세션의 첫 칙령 안내가 끝난다). 개발용 콘솔(UFUNCTION(Exec) — 화면 찍기와 손 확인): CosmicEdict <행>(지니면 철회, 아니면 제정 — 창과 같은 길, 거절은 사유와 함께 한 줄; 인자 없으면 행마다 문의 답을 로그에), CosmicShares <물리> <사회> <공학>([이대로 나눈다]와 같은 길), CosmicEdictWindow <0|1|2|3>(닫기·칙령 쪽·연구의 몫 쪽·[백성] 쪽 — 3은 B3). 칙령 창과 화면의 규칙과 문구는 헤더 B2 4절. B3(헤더 B3 4절·11절 항목 10): bool SetStance(FName CivID, FGameplayTag Stance)·ProposeTreaty(CivID, Treaty)·BreakTreaty(CivID, Treaty)·AllocateFleet(FName SystemID) — 칙령 창의 [백성] 탭과 성계 패널의 함대 단추가 부른다. 끝난 판은 거절하고 받아들인 명령은 HUD와 지도를 곧 다시 그린다. 개발용 콘솔 CosmicStance <CivID> <friendly|neutral|hostile>·CosmicTreaty <CivID> <trade|nonaggression> [break]·CosmicFleet <SystemID|none> — 단추와 같은 길, 인자가 없으면 문의 답을 한 줄씩 로그에, 거절은 사유와 함께 한 줄

// - 청원(MVP ②, 2026-10-20 — 헤더 docs/60_DECISIONS/headers/A2_petition_cards.md): FCosmicPetitionAnswer AnswerPetition(int32 ChoiceIndex) — 대기 카드에 답한다(틱 밖 입력). 통치자의 청원은 몸을 입은 통치자가, 방랑자 카드(CosmicPetitions::IsWandererCard — Subject VesselSystem, B1)는 몸을 입은 방랑자가 답한다. 거절(대기 카드 없음·다른 역할의 카드는 에러 줄 없이, 표에 없는 행·번호 밖 선택지·비축 부족·선택지의 문이 거절하는 선택지(CosmicPetitions::ChoiceGateOf — 태세·칙령·함대의 시간, §7.4 'B4' 줄; B3에는 첫 접촉 카드의 태세의 문 StanceBlockOfChoice뿐이었다. 통치자 카드가 아직 서 있을 때만 묻고, 에러 줄은 'the choice's gate refuses choice N'. 서 있지 않은 카드는 아래처럼 뽑고 거둔다)는 에러 한 줄, 방송 중)은 뽑지 않고, 받아들인 답은 시뮬 스트림에서 판정의 수(0~5)를 먼저 1회 뽑는다 → 카드가 더 서 있지 않으면 거둔다(Withdrawn — 비용·효과·줄 없음) → 비용(다스리는 백성의 비축) → 판정(CosmicPetitions::Passes: NoCheck이면 성공, 아니면 능력치 + 수 ≥ Difficulty, Stability는 (int32)(Stability × 10 + 0.0001)) → OnSuccess|OnFailure를 제자리에 적용 → FollowUpKnowing → 결과 줄 Push(Log.Civ, Severity 1, 말하기 스트림 추첨 없음) → FollowUpNotes → LastChoiceRows → 카드를 거두고 NextEncounter가 있으면 같은 주어로 다음 카드 — 그 행의 조건(`CosmicPetitions::Holds`)이 설 때만, 추첨 없음(B4의 둘째 영압 카드는 그 사이 주장이 끝났으면 오지 않는다 — `Sim.PressureChain`) → SyncRngState. 반환 { Outcome(Refused·Withdrawn·Succeeded·Failed), Roll, Line } — 상태가 아니다. 델리게이트는 없다(화면은 틱 경계에 대기 카드를 읽는다). 첫 접촉 카드(CosmicRows::PetitionFirstContact, B3)의 답은 Knowing.PendingContactCivID의 백성을 효과 문맥의 OtherID와 글의 {OtherCiv}로 쓰고(ArgsOf(…, OtherCivID, bNamesOther = true)), 받아들인 답이든 Withdrawn이든 끝에서 그 칸을 비운다 — 다른 카드의 문맥에는 OtherID가 없다(§7.4 앎 단계의 '첫 접촉 카드'). 순수 규칙은 Events/CosmicPetitions.h(RuledCivIndexOf·IsWandererCard·IsAboutPlace·VesselSubjectOf·WithinWandererGap·IsFirstTickOfReign·IsFirstTickOfLife(B5)·SubjectOf·Holds·StandingHeld·Present·StillStands·CheckValueOf·Passes·PassingRollsOf·CostOf·ArgsOf·StanceBlockOfChoice·ChoiceGateOf(B4 — §7.4)·Clear·Remember — 아래). 카드에 KillPlayer는 싣지 않는다(스키마 — 카드의 치명은 B5 밖, §16 #151), DeposeRuler는 PendingRoleEffect에 기록만, 처리는 다음 틱의 열째 단계(§7.4)
// - 방랑자 카드의 답(B1 — 같은 함수, 같은 차례, 같은 1회 추첨): 비용을 보지 않는다(주제 VesselSystem 행의 비용 0은 C++ Data.Integrity의 선택지 검사가 지킨다 — 스키마의 시험이 아니다). 판정 값은 그릇의 능력치(CheckValueOf — Stability 판정은 스키마가 막는다). 주제와 효과의 문맥은 VesselSubjectOf — 그릇이 선 성계와 그 주인(무주면 없음 — 백성에 거는 효과는 Apply가 하지 않는다), 은하 행은 주인의 것, 없으면 그 성계의 것. 그래서 경외(ChangeReverence)는 카드가 놓인 성계의 백성에게 오른다. 거둬짐: 그릇이 그 성계에 더 서 있지 않거나 이동 중이거나 행의 조건이 더 서지 않으면(StillStands). 결과 줄은 같은 꼴(Log.Civ, Severity 1, Push — 초점은 그 성계와 주인). 사슬(NextEncounter)은 역할을 넘지 않는다 — 방랑자 카드의 다음은 방랑자 카드뿐(B1의 다섯 장에는 사슬이 없다). FOnEncounterPresented는 두지 않는다 — 대기 카드는 앎에 저장되고 화면이 틱 경계에 읽는다
// - CosmicPetitions의 방랑자 쪽(Events/CosmicPetitions.h, B1 — 상태·색인·테이블의 함수, 줄·추첨 없음): struct FCosmicVesselSubject { int32 SystemIndex = INDEX_NONE; int32 CivIndex = INDEX_NONE; // 무주 성계면 INDEX_NONE } · bool IsWandererCard(const FEncounterDef&)(Subject == VesselSystem) · bool IsAboutPlace(const FEncounterDef&)(SettledSystem ∨ VesselSystem — 줄이 그 성계에 관한 것이고 기다리는 카드가 그 성계를 쥔다) · FCosmicVesselSubject VesselSubjectOf(State, Index, bool bTravelAtOrigin = false)(이동 중이 아닌 몸을 입은 방랑자가 선 성계와 그 주인. 통치자·이동 중·공백기는 둘 다 INDEX_NONE — bTravelAtOrigin이면 이동 중인 그릇을 떠나온 성계에 선 것으로 센다: 틱 첫머리, CosmicWanderer::PlaceIndexOf) · bool WithinWandererGap(State, CivIndex)(주인 없음, 또는 부르심의 PersonalTier보다 많아야 한 칸 위의 백성 — 세계의 칸만 읽고 앎을 읽지 않는다. 격차 2의 경외/공포 변형은 B1 밖) · bool IsFirstTickOfReign(State)(몸을 입은 통치자의 열린 재위가 아직 한 틱도 세지 않았다 — Lives.Last().ReignTicks.Last() == 0; 앎 단계가 그 틱의 청원을 빈 첫머리와 견준다) · bool IsFirstTickOfLife(State)(B5: 틱 머리에서 몸을 입은 방랑자이고 Lives.Num() > 1이며 Tick == Calling.BornTick — 계승이 연 생애의 첫 틱; 앎 단계가 그 틱의 방랑자 카드를 빈 첫머리와 견준다. 앎에만 쓰고 시뮬 수는 없다) · StandingHeld(State, Index, Tables, Rules, bool bTravelAtOrigin = false)(방랑자: 선 성계의 방랑자 Standing 카드 가운데 그 주인이 격차 안인 단위; 카드가 기다리는 동안·공백기는 비어 있다 — 이동 중인 그릇은 bTravelAtOrigin일 때만 떠나온 성계에서 센다) · Present(State, Index, Tables, Rules, HeldAtHead, bool bArrived = false)(방랑자: 파생 정수 하나 u = DeriveForName(Seed, 그릇이 선 SystemID, Purpose::Encounter 11, Tick) / 2^32 — 시뮬 스트림을 뽑지 않는다. 첫머리에 없던 Standing 단위가 있으면 그 가운데 u로 무게 고르기, 없으면 도착한 틱(bArrived)에만 u < EncounterChancePerArrival일 때 조건이 선 방랑자 Lottery 행 가운데 다시 편 u / 몫으로 무게 고르기. 그 생애에 가장 최근에 답한 방랑자 카드는 다시 내놓지 않고, 격차 밖 백성의 Standing 카드는 내놓지 않는다 — 도착 카드는 자리만 말해 격차를 보지 않는다. Knowing의 두 칸만 쓴다) · StillStands(방랑자 카드: 이동 중이 아닌 방랑자가 카드의 성계에 서 있고 조건이 선다). 규칙의 전문은 docs/20 부록의 틱 순서 문단
// - CosmicPetitions의 첫 접촉 쪽(Events/CosmicPetitions.h, B3 — 헤더 B3 2.9절·11절 항목 11. 줄·추첨 없음): Present(통치자: 그 틱에 막 선 Standing 단위가 없을 때, 주기 슬롯 앞에 — Knowing.PendingContactCivID의 백성이 Civs에 있고 CosmicRows::PetitionFirstContact 행이 있고 방랑자 카드가 아니며 Holds가 서면 그 카드를 놓는다. 추첨·주기·LastChoiceRows의 거르기 없음) · StillStands(첫 접촉 카드는 PendingContactCivID의 백성이 Civs에 있어야 한다 — 없으면 Withdrawn) · FFormatNamedArguments ArgsOf(const FGalaxyState&, const FCosmicDataTables&, int32 CivIndex, int32 SystemIndex, FName OtherCivID = NAME_None, bool bNamesOther = false)({Civ}·{System}·{Hazard}·{Rebel}에 더해 {OtherCiv} = OtherCivID 백성의 이름. bNamesOther(첫 접촉 카드)이면 찾지 못한 백성 — 사라졌거나 대기 칸이 비었다 — 은 ST_CA Petition.OtherGone, 아니면 빈 글. 답·카드 창이 같은 함수 — 컨트롤러는 첫 접촉 카드일 때만 상대를 카드 창에 넘긴다) · CosmicDiplomacy::ECosmicStanceBlock StanceBlockOfChoice(const FGalaxyState&, const FCosmicStateIndex&, const FCosmicRulesDef&, int32 CivIndex, FName OtherCivID, const FEncounterChoice&)(선택지의 OnSuccess에 든 빈 Key의 SetStance마다 CosmicDiplomacy::StanceBlockOf(bMet = true — 효과가 묻는 그대로)의 첫 거절 — 받거나 이미 그 태세(Same)거나 상대의 이름이 비었으면(NAME_None) None이고, 이름이 Civs에 없으면 StanceBlockOf의 NoCiv다. 카드가 회색으로 두고, 답이 거절하고, 정책이 건너뛴다 — 카드와 답은 카드가 서 있을 때만 묻고(그때 상대는 Civs에 있다), 사라진 백성의 대기 칸은 건넘 단계의 정리가 비우므로 정책도 NoCiv를 만나지 않는다) · void Clear(FGalaxyState&)(카드 칸 둘만 — PendingPetitionRow·PendingPetitionSystemID. Knowing.PendingContactCivID는 건드리지 않는다: 답마다 불리므로 다른 청원 뒤에 기다리는 첫 접촉 카드가 남는다)

// - ReceiveCalling(가칭, v0.8 — 단계 6, 헤더 승인): 부르심에서 서막(첫 틱 전의 카드, §4.4)의 결과(`Exaltation`, 연구 중점, 상처의 상태 태그 하나, 선택의 기록, 거절 횟수)를 상태에 적는다. 칸·`PeakTier`를 적지 않는다. 핸들러 안에서 불리면 에러 한 줄(CLAUDE.md의 목록에 든다). 선택의 기록이 저장 필드가 되면 초기값 규칙과 해시 포함 여부를 헤더에 적는다.

// UCosmicSaveSubsystem (Save/CosmicSaveSubsystem.h, MVP ⑤ 2026-10-23 — 헤더 docs/60_DECISIONS/headers/A5_save_minimum.md, §9.1. Core Logic: UI·World를 include하지 않는다)

// - 슬롯 이름: MakeSlotName(GalaxyID) = Galaxy_<GUID 32자리> · SlotNameOf(ID, bBackup) = 폴더 + 이름(+ _bak) · SlotPathOf(ID, bBackup) = ProjectSavedDir()/SaveGames/<SlotNameOf>.sav(엔진의 저장 시스템과 같은 폴더·확장자). 판당 슬롯 하나와 백업 하나, 수동 저장·로드 메뉴 없음.

// - 명시 호출(테스트·도구 — 게임 저장의 문을 보지 않는다): bool SaveCurrentGalaxy()(까닭 Asked) · ECosmicSlotRead LoadRun(ID, bOutFromBackup) / bool LoadGalaxy(ID) · ECosmicContinue ContinueNewest() · DoesSaveExist(ID)(슬롯이나 백업) · DeleteSave(ID)(둘 다). 쓰기와 로드는 핸들러 안(IsBroadcasting)과 틱 단계 안(IsInTickStages)에서 에러 한 줄로 거절되고(Refused — 아무것도 읽지 않는다), ContinueNewest는 핸들러 안에서 NoSlot. 쓰는 차례: 검사를 통과한 슬롯을 _bak으로 → 내림(IPlatformFile Flush) → 슬롯 → 내림. 머리의 쓴 시각은 이 프로세스가 쓰거나 읽은 어느 머리의 시각보다 늦다(LatestSavedAt — 시계가 뒤로 가도, 실패한 쓰기의 시각도 센다). 읽히지 않는 슬롯(ReadFailed — 0.25초 뒤 한 번 더)·새 꼴의 슬롯은 덮지 않고, 상한 슬롯은 옮기지 않으며, CosmicSave::KeepsSanctuary가 거짓인 상태는 쓰지 않는다. 쓰기 직전 UGalaxySimulationSubsystem::SyncRngState. 로드는 상함(CosmicSaveSlot::IsDamage — 없음 포함)일 때만 백업을 열고, 거절된 로드는 시뮬을 건드리지 않는다. ContinueNewest: 폴더의 *.sav마다 머리 64바이트만 읽어 NewestRun(머리의 쓴 시각 — Ok·Truncated·NewerForm의 머리, 엔진의 옛 꼴(OldForm — MVP ⑤ 전의 GVAS 슬롯)은 판으로 세지 않고 지나치며, 그 밖은 파일 시각)을 그 슬롯이나 백업에서 연다 — Newest·Backup·Unreadable(더 옛 판으로 내려가지 않는다)·NoSlot. 아무것도 지우지 않는다.

// - 게임의 저장(문 IsGameSavingOn = GateClosedBy(CommandLineAllows(-NoSave·-unattended·-Seed= 가 없다), PrepareForTests, GIsAutomationTesting, GIsEditor, 테스트의 덮어쓰기) == null): BeginSession()(ACosmicGameMode::StartPlay가 Super::StartPlay 앞에서 한 번 — 해시 판의 프로세스에서는 부르지 않는다, MVP ⑨ — 문이 닫혔으면 Off, 열렸으면 ContinueNewest) · TakeStartOutcome()(컨트롤러의 BeginPlay가 한 번 가져간다 — FollowStart) · GetLastContinue() · HoldStartRun()(Unreadable 시작의 새 판 — 그 판 하나만 첫 경계가 쓰고, 쓰이기 전 틱 0의 종료 저장은 건너뛴다, 헤더 결정 6) · RequestSave(ECosmicSaveCause)(입력 때 곧바로 쓴다 — 실패 뒤의 입력도; Exit는 디스크 사본과 같은 해시면 쓰지 않는다; 핸들러·단계 안이면 까닭을 남겨 다음 경계가 쓰고, 쓰기가 실패하면 그 까닭이 남아 RetryRealSeconds 뒤의 경계가 쓴다) · TakeFirstFailure()(게임 인스턴스마다 한 번, 아직 바로잡히지 않은 실패만 참 — 컨트롤러가 시간을 멈춘다). 틱 경계: OnSimTick 핸들러(Initialize가 묶는다)가 DueAt(디스크 사본, MarkOf(State, Rules), 실제 초, AutosaveRealSeconds 600)으로 까닭을 고른다 — NewRun > Ended > Ascended > Death > Succession(B5) > Role > Interval(마지막 쓰기나 로드부터 실제 600초 ∧ 틱이 움직임). 실패 뒤에는 RetryRealSeconds 60초를 기다린다(Ended는 기다리지 않는다). 성공한 쓰기는 어느 길이든 남은 까닭과 기다림을 지운다. Deinitialize는 쓰지 않는다(종료 저장은 컨트롤러의 창 닫기 OnCloseRequested와 EndPlay(Quit)).

// - enum class ECosmicSaveCause : uint8 { Asked, NewRun, Ascended, Role, Ended, Interval, Answer, Exit, Death, Succession }(Death·Succession은 B5 — 공백기에 든 경계와 새 생애가 선 경계) · enum class ECosmicContinue : uint8 { NotBegun, Off, NoSlot, Newest, Backup, Unreadable } — UENUM이 아니다(파일·상태·해시 밖, 로그 줄과 테스트만). struct FCosmicSavedMark { GalaxyID, Tick = -1, StateHash, Feats, Role, RuledCivID, bEnded, Lives, bInterregnum, RealSeconds }(Lives = Calling.Lives.Num()·bInterregnum = LifeState == Interregnum — B5; bEnded = State.Ending.Kind가 섰다 — B6a, 까닭 Ended의 차례는 그대로) — 디스크 사본: 마지막으로 쓰거나 읽은 슬롯이 든 것의 캐시(쓰기와 로드가 통째로 바꾼다. 잃으면 한 번 더 쓸 뿐 — 상태가 갖지 않은 사실은 없다, 규칙 2의 생애 종속 상태가 아니다). static DueAt·MarkOf·CommandLineAllows·GateClosedBy(순수 — Sim.SaveTriggers). namespace CosmicSave { NameOf(까닭·이어하기), bool KeepsSanctuary(const FGalaxyState&) — 마지막 성계가 주인·위기·프로젝트 없는 성역(§4.5 ①): 로드는 거절(Sanctuary)하고 쓰기는 쓰지 않는다 }.

// - 테스트: PrepareForTests()(CosmicTests::AcquireSim — 폴더 Automation/, 문 닫음, 덮어쓰기 끔, 시간·실패 되돌림) · SetFolderForTest · SetGameSavingForTest(bOn) · ContinueAtNextStartForTest()(다음 StartPlay가 지금 은하 위로도 잇는다 — 지도 다시 열기) · SetTimingForTest(Interval, Retry) · GetSavesWritten · GetLastCause.

// 해시 판(World/CosmicHashRun.h, MVP ⑨ 2026-10-24 — 헤더 docs/60_DECISIONS/headers/A9_win64_packaging.md §2, §9.2. World 층: Sim·Log·Player를 부르는 상태 없는 함수 — 새 은하를 만들어 돌리므로 지금의 판을 바꾸고, 로그의 말하기 모드는 끝나면 되돌린다)

// - namespace CosmicHashRun: MaxTicks 100000 · enum class ESwitch : uint8 { Absent, Bad, Ticks } · FSwitch { Kind, Ticks, EveryKind, Every, bPolicy, Text, EveryText, IsGood() } · FSwitch SwitchOf(CommandLine)(-CosmicHashRun=·-CosmicHashEvery= — 1~6자리 ASCII 숫자·MaxTicks 이하를 다음 공백까지 글로 읽는다, -CosmicHashPolicy) · enum class EOutcome : uint8 { Ran, MissingData } · FResult { Outcome, Seed, Ticks, bPolicy, StateHash, WorldHash, Checkpoints, Missing } · FResult Run(Sim, Log, Role, Seed, Switch)(idle: 전부 말하기 → NewGalaxy(Seed, 0, 0) → RunHeadless를 Every씩; 정책: 범위 → NewGalaxy → 틱마다 RunHeadless(1)과 CosmicRulerPolicy::Act(커맨드렛의 -Answer=1 -Build=1 -Focus=1 -Edict=1 -Diplomacy=1 -TellAll=0과 같은 정책 — -Edict=1은 B2부터, -Diplomacy=1(평화의 외교 정책)은 B3부터: 정책 값 { true, true, true, true, NAME_None, 1, Round, 0, false } — B4의 세 값은 영압 카드를 돌아가며 답하고 지킴·거르기가 없는 기준값의 꼴); 로그의 모드를 되돌린다; 열한 테이블(B2의 DT_Edicts까지)·규칙 행·고향 은하 행이 없으면 NewGalaxy 전에 MissingData) · FString LineOf(Result)('CosmicHash: seed S, T ticks, hash H, world W[, policy]' / 'CosmicHash: refused - missing <이름>').

// - ACosmicGameMode::StartPlay의 맨 앞: 'CosmicBuild: version <ProjectVersion>, commit <ProjectDir()/BuildInfo.txt의 commit=|->, cooked|editor' 한 줄, 그리고 세 스위치 가운데 하나라도 있으면 해시 판 — 거절(Error: no -CosmicHashRun= · bad -CosmicHashRun=<글> · bad -CosmicHashEvery=<글> · no -Seed= · the game's saving is on · no simulation) 또는 체크포인트(Display)와 LineOf(Ran은 Display, MissingData의 'refused - missing <이름>'은 Error), 그리고 RequestExitWithStatus(false, 0) — BeginSession도 Super::StartPlay도 부르지 않는다.

// 통치자 정책(World/CosmicRulerPolicy.h, MVP ⑨ — 밸런스 커맨드렛의 -Answer·-Build·-Focus를 그대로 옮겼다; B2가 -Edict·-EdictRow를, B3가 -Diplomacy를, B4가 측정 전용 -PlayerPolicy·-Guard·-LeaveNearSphere를 더했다; 커맨드렛과 해시 판이 함께 쓴다): FCosmicRulerPolicy { bAnswer, bBuild, bFocus, bEdict, FName EdictRow(빈 슬롯에 세워 둘 행, None = 없음), int32 Diplomacy(B3 — 0 끔 · 1 평화의 정책 · 2 공격의 정책, 2는 측정 전용이라 어느 기준값 판도 돌지 않는다), ECosmicPressurePolicy Pressure = Round(B4 — 영압 카드에 답하는 꼴. enum class ECosmicPressurePolicy : uint8 { Round, Cautious, Reckless, Faithful } — 저장되지 않는다; Round는 다른 카드처럼 돌아가며, Cautious는 늘 지켜봄(값·판정·효과·다음 카드가 없는 선택지), Reckless는 분별(다음 카드를 여는 선택지) → 함대의 시간 → 지켜봄, Faithful은 철야(VigilRowOf를 내리는 EnactEdict) → 분별 → 다른 칙령 → 지켜봄. 어느 기준값 판도 Round만 돈다), int32 Guard = 0(B4 — 1이면 함대가 ActDiplomacy ①의 지킬 성계를 지킨다: 에워싸인 제 성계, 없으면 GuardTargetOf — 외교가 꺼졌어도, 판의 끝 뒤에도), bool bLeaveNearSphere = false(B4 — 영압 정책 아래에서만: 주장자가 닿을 다스리는 정착 성계의 깨어남·역병(Draw 행이고 초신성 전조가 아닌 위기)에 프로젝트도 답 칙령도 내지 않는다 — 그 만료가 주장을 데려오게, 헤더 B4 5절 첫 측정 ③), bool bPastEnd = false(B5 — 측정 전용 -PastEnd=1, 자동화 테스트가 든 빌드에서만: 끝 뒤에도 연구 중점·칙령·외교의 입력을 이어 간다. 끝은 그대로 센다 — bEnded(끝의 기록이 섰다 — B6a)와 그 '끝 전' 셈은 끝에서 선다) — 만드는 곳마다 열 값을 모두 쓴다(집합 초기화는 빠진 값을 조용히 false·0으로 둔다) } · FCosmicRulerPolicyCounts { CardsCome(통치자의 카드만 — 방랑자 카드는 세지 않는다, B1), Answers, Started, FocusChanges, EdictsEnacted, EdictsRevoked(받아들인 칙령 명령, B2), VigilEnacted(규칙 ③이 내린 edict_vigil — 규칙 ④가 철회한다), StanceChanges·TreatiesSigned·FleetTargetChanges(받아들인 외교 명령 — 태세·맺은 조약·함대 목표의 바꿈, B3), TreatiesProposedBorder·TreatiesRefusedBorder·TreatiesProposedOther·TreatiesRefusedOther와 TreatyLooks(조약을 본 셈 — 국경이 닿은 백성에게 받아들여진 제안, (백성, 조약) 쌍마다 처음 본 것이 거절인 수, 국경 밖 백성은 처음 본 답만(정책은 그들에게 청하지 않는다). 기록 키는 (다스리는 백성, 상대, 조약) — 다른 백성을 다스리는 둘째 치세는 다시 세고 같은 백성으로의 복귀는 다시 세지 않는다. 둘째 칸 전의 Tier 거절은 본 것이 아니다), bEnded, TArray<FName> CardEdicts(B4 — 영압 카드의 받아들인 답이 내린 칙령, 헤더 B4 결정 9: 다스리는 백성의 정착된 성계에 주장이 서 있는 동안 정책은 그것을 철회하지 않는다), 그리고 B4의 측정 셈(커맨드렛의 pressure 줄 — 정책의 놀이는 바꾸지 않는다): PressureCards·PressureCardsByEnd·FirstPressureCardTick(−1 = 없음)·ClaimCards·MeasuredCards·PressureCardsByTier[3] — 다스리는 백성의 칸: 첫째·둘째·셋째 이상, PressureAnswers[5] — 답의 꼴: 간구·분별·지켜봄·칙령·저항, PressureAnswersByEnd·PressureGreyed(카드가 왔을 때 회색인 선택지의 사유별 — 비축의 것, 문의 꼴과 사유)·CardEnactments(카드의 답이 내린 행별)·CardEdictsRevoked(스크립트가 철회한 카드의 칙령)·DiscernInsightMin(분별을 받아들인 때의 가장 적은 통찰, −1 = 없음) } · int32 ChoiceOf(State, Index, Row, AnswersSoFar, FName RowName = NAME_None, int32 Diplomacy = 0, const FCosmicRulesDef* Rules = nullptr, const FCosmicDataTables* Tables = nullptr, ECosmicPressurePolicy Pressure = Round)(B4가 끝의 둘을 기본 인자로 더했다 — B3의 테스트 직접 부름은 그대로. 비축으로 낼 수 있는 첫 선택지를 답한 수부터 돌아가며 — 성공이 재위의 끝을 적는 선택지는 빼고 돈다: 스크립트는 스스로 왕좌를 떠나지 않는다, B1. 지금의 행에서 그런 선택지는 왕좌 카드의 셋째 하나이고, 기준 판들에는 왕좌 카드가 오지 않아 고르기가 그대로다. Rules를 주면 첫 접촉 카드의 상대에게 태세의 문이 거절하는 선택지를 낼 수 없는 것처럼 건너뛰고(CosmicPetitions::StanceBlockOfChoice — 답이 거절하는 선택지), Diplomacy가 0이 아니면 첫 접촉 카드(RowName = CosmicRows::PetitionFirstContact)에는 답한 수와 상관없이 성공이 우호를 거는 첫 선택지('손을 내민다')를 고른다 — 돌아가며 고르면 판마다 두 선택이 섞인다, B3. Rules와 Tables를 함께 주면 카드의 성계(SubjectOf — Knowing.PendingPetitionSystemID)에서 선택지의 문 CosmicPetitions::ChoiceGateOf가 거절하는 선택지를 모두 건너뛰고(B4 — 칙령·함대의 시간도), Tables와 Round 아닌 Pressure를 주면 영압 카드(칙령을 내리거나 위기의 시간을 사는 선택지를 지닌 카드 — 행 이름이 아니라 효과로 알아본다)에는 그 정책의 꼴 차례에서 답이 받는 첫 선택지를 고른다 — 없으면 돌아가며) · void Act(Policy, Sim, Role, CardBefore, Counts)(틱 경계: 끝의 기록 읽기 → 온 카드 → 답(8번까지) → 프로젝트 키(8번까지 — 영압 정책 아래에서는 ProjectChoicesOf가 거른 목록의 첫 것을 StartMegaproject로 짓고, 비면 첫 정착 대상의 정착, B4 — 공격의 정책은 마지막 보유 자리를 정복에 남긴다: 프로젝트의 고를 것이 없고 소유 수 + 1 ≥ 보유 상한이면 정착을 시작하지 않는다, 상한에 닿은 백성은 공성을 시작하지 못하므로 — B3) → 끝나지 않았으면 다음 칸의 연구로 중점 → 칙령 ActEdicts → 외교 ActDiplomacy). void ActEdicts(Policy, Sim, Role, Counts)(B2 — bEdict이고 판이 끝나지 않았고 몸을 입은 통치자일 때, 다스리는 백성에게 명령 EnactEdict·RevokeEdict로만 — 받아들인 명령마다 셈 하나. 규칙 다섯을 이 차례로: ① 안정도 0.5 아래면 안정도 값이 있는 칙령을 철회 ② 그 백성의 정착된 성계에 선 위기(Systems 차례)마다, 지닌 칙령이 답하지 않으면 그 위기 행의 RulerResolutions에서 첫 DT_Edicts 행을 내린다 ③ 그 백성의 LowStabilityTicks가 0보다 크고 수도에 반란(Capital 행의 위기)이 서 있지 않으면 철야의 행 — Capital 행의 태그에 0보다 큰 카운트다운 더하기를 가진 사전순 첫 칙령 행, 지금 edict_vigil — 을 지니지 않았을 때 내린다 ④ 답할 위기가 없는 답인 칙령, 그리고 ③이 내린 edict_vigil은 안정도 0.6 이상이고 반란이 서 있지 않을 때 철회 — EdictRow는 철회하지 않는다 ⑤ 빈 슬롯이 있으면 EdictRow를 내린다 — 안정도 값이 있는 행이면 안정도 0.6 이상일 때만. ②·③은 슬롯이 차 있으면 서 있는 위기에 답하지 않는 칙령 하나를 먼저 철회해 자리를 만든다(EdictRow는 다른 것이 없을 때만) — 그 행의 사유가 Slots 하나이고 비축이 그 제정의 값을 낼 수 있을 때만이라 거절될 제정을 위해 칙령을 버리지 않는다. 한 경계에서 내린 행을 철회하지 않고 철회한 행을 내리지 않는다. B4: 영압 정책 아래 ②는 ProjectChoicesOf와 같은 거르기로 답 칙령을 내리지 않는다(주장의 답, bLeaveNearSphere면 깨어남·역병의 답). 다스리는 백성의 정착된 성계에 주장이 서 있는 동안(ClaimStandsOn) 자리를 만드는 철회와 ④의 철야 규칙은 어느 정책에서든 CardEdicts를 건드리지 않고(④의 할 일 없는 답 칙령의 철회는 막지 않는다 — 지금 카드는 답 칙령을 내리지 않는다), 영압 정책 아래에서는 누가 내렸든 철야(VigilRowOf)도 건드리지 않는다 — 카드가 내린 철야는 주장이 끝난 뒤 ④의 '위험이 지났다'(안정도 0.6 이상·반란 없음)로 철회한다) · void ActDiplomacy(Policy, Sim, Role, Counts)(B3 — 헤더 B3 5절과 11절 항목 11·12. Diplomacy가 0이 아니고 판이 끝나지 않았고 몸을 입은 통치자일 때(Guard가 1이면 외교가 꺼졌거나 판이 끝났어도 ①은 돈다 — B4; ②~④는 그대로 Diplomacy ≠ 0 ∧ 끝 전), 명령 SetStance·ProposeTreaty·AllocateFleet로만 — 받아들인 명령마다 셈 하나, 한 경계에 명령 하나까지, 이 차례로: ① 지킴이 먼저(공성은 지금 성계를 위협하고 태세와 조약은 한 경계를 기다릴 수 있다) — 그 백성이 아무도 에워싸지 않는 동안, 제 성계를 노린 공성이 서 있으면(Systems 차례의 첫 성계) 그 성계로 함대를 보내고, 공성이 없는데 제 성계를 지키고 있으면 물린다 — Guard가 1이면 물리는 대신 GuardTargetOf의 성계로 옮기고, 이미 그 성계를 지키면 아무것도 하지 않고 다음 단계로 간다(B4) ② 우호 — 첫 접촉 카드가 기다리는 동안은 누구에게도 걸지 않고(그 첫 태세는 카드가 정한다 — 정책이 자리를 먼저 채우면 [손을 내민다]가 막힌다), 영향력의 수입이 지닌 칙령의 유지비와 우호 하나를 더한 유지비를 덮을 때만(덮지 못하면 우호가 틱마다 끊기고 경계마다 다시 걸린다), 국경이 닿고 태세가 없는 만난 백성 가운데 Civs 차례의 첫 백성에게 문이 받으면 ③ 조약 — 공격의 정책은 청하지 않는다(불가침은 에워쌀 이웃과의 공성의 문을 닫고, 무역은 공성마다 배신으로 열린다). 만난 백성을 Civs 차례로 걸으며 불가침, 그다음 무역의 문을 묻고 — 처음 본 답을 센다 — 국경이 닿은 백성에게 TreatyBlockOf가 None인 첫 조약을 청한다 ④ 공격의 정책만 — 그 백성이 에워싸는 중이 아니고 제 성계를 노린 공성을 지키고 있지 않을 때: 적대하는 백성의 첫 성계(Systems 차례, SiegeBlockOf가 받는 것)로 함대를 보내고, 그런 성계가 없는 적대는 중립으로 되돌리며(AI의 규칙과 같은 꼴 — 공성 없는 적대를 쌓지 않는다), 그다음 그런 성계(bAssumeHostile)가 있는 국경이 닿은 만난 백성 가운데 첫 백성에게 적대를 건다. 평화의 정책(1)은 적대하지 않는다. 만난 백성은 앎에서 읽는다 — 정책은 플레이어의 스크립트다) · int32 DecisionsOf(Counts)(답 + 시작한 프로젝트 + 몫의 바꿈 + 칙령의 제정과 철회 + 외교 명령(태세·조약·함대 목표 — B3) — 커맨드렛의 통치자 줄이 이 하나를 읽는다, B2. B4의 새 값은 그 줄의 모양을 바꾸지 않고 pressure 줄의 머리에만 찍는다). B4의 함수(헤더 B4 2.9절·11절 항목 8~10): bool PressurePolicyOf(Word, Out)(-PlayerPolicy의 낱말 Round·Cautious·Reckless·Faithful, 대소문자 무관 — 다른 낱말은 거짓이고 Tribute도 거짓이라 커맨드렛이 판을 돌리지 않는다: B4에는 조공이 없다) · const TCHAR* PressurePolicyName(Pressure) · bool ClaimStandsOn(State, Tables, CivIndex)(그 백성의 정착된 성계에 주장 규칙 행(ClaimRule)의 위기가 서 있는가) · FName GuardTargetOf(State, Index, Tables, Rules, CivIndex)(에워싸인 제 성계가 없을 때 -Guard=1이 지킬 성계: OwnedSystems 차례의 정착된 성계 가운데 그 백성보다 높은 칸의 봉인되지 않은 초월자 영향권에서 1레인인 첫 성계(주장이 서는 자리), 없으면 그 안의 첫 성계(함대가 닳는 자리), 없으면 수도 — 지킴의 문이 수도를 거절하면(함대 없음 — 첫째 칸) NAME_None) · TArray<TPair<int32, FName>> ProjectChoicesOf(Policy, State, Index, Tables, Rules, CivIndex)(스크립트가 시작할 수 있는 프로젝트 — Round는 CosmicMegaprojects::Choices 그대로, 영압 정책은 주장의 답(구호 — 신정론의 셈은 일이 정하지 않은 주장이다, 헤더 B4 2.8절)을 빼고, bLeaveNearSphere면 주장자가 닿을 정착 성계의 깨어남·역병의 답도 뺀다) · FName VigilRowOf(Tables)(ActEdicts ③의 철야 — 수도 규칙 행의 태그에 0보다 큰 카운트다운 더하기를 지닌 사전순 첫 DT_Edicts 행, 지금 edict_vigil; 영압 정책과 커맨드렛의 spirit 줄이 같은 행을 읽는다).

// 방랑자 정책(World/CosmicWandererPolicy.h, B1 — 밸런스 커맨드렛의 -Abdicate=<틱>·-Wander=1과 테스트가 함께 쓴다. 플레이어의 스크립트라 화면처럼 세계와 앎을 읽는다): FCosmicWandererPolicy { int64 AbdicateAt(0 이하 = 떠나지 않는다), bool bWander, bool bPastEnd(B5 — 측정 전용 -PastEnd=1, 자동화 테스트가 든 빌드에서만: 끝 뒤에도 양위·카드의 답·즉위·스캔·이동을 이어 간다. bEnded는 이 값 아래에서도 끝을 그대로 세고, 입력의 문만 bEnded ∧ ¬bPastEnd로 닫힌다 — 통치자 정책과 같은 꼴) } · FCosmicWandererPolicyCounts { LeftAt·CrownedAt(처음 떠난 틱과 처음 다시 왕좌에 오른 틱, -1 = 아직), CrownedCivID, CrownedTier, CrownedPath("reverence"·"vacuum"·"return" — 경외와 공백이 함께 서면 경외), Journeys, Scans, Cards, Answers(거둬진 카드는 세지 않는다), ReverencePeak(걷는 동안 어느 백성이든 가장 높았던 경외), LastLeft, ScannedAt, bEnded, FailedByRow(B5 [B-5a] — 실패한 답의 카드 행별 수), LastCrownedAt·LastCrownedPath(B5 — 마지막 즉위와 그 길) } · void CosmicWandererPolicy::AbdicateIfDue(Policy, Sim, Role, Counts)(AbdicateAt 틱의 경계에서 통치자 정책보다 먼저 Abdicate — 끝의 기록을 먼저 읽고 그 뒤에는 입력하지 않는다) · void CosmicWandererPolicy::Act(Policy, Sim, Role, CardBefore, Counts)(통치자 정책 다음: 떠남·온 카드·경외를 세고, bWander이고 판이 끝나지 않았으면 ① 기다리는 카드에 제 답 수부터 돌아가며 답한다(방랑자는 비용이 없다) ② 선 성계의 주인에게 CrownBlockOf(만남은 앎에서)가 None이면 즉위·복귀 ③ 도착마다 한 번 스캔 ④ 이동 중이 아니면 TravelTo가 받는 레인 이웃 가운데 위기가 선 첫 이웃(Lanes 차례 — 방금 떠나온 성계는 그것만 열려 있을 때만), 없으면 (여행 수 mod 열린 이웃 수)번째로 간다). 한 경계의 차례: RunHeadless(1) → 끝 읽기 → 양위 → 통치자 정책 → 방랑자 정책. 커맨드렛의 요약 줄과 기준값은 docs/40.

// 방랑자의 화면(B1 — 표현 계층의 인터페이스, 상태·해시·추첨 밖. 화면의 규칙과 문구는 헤더 B1의 4절):
// - FCosmicKnownView(World/CosmicMapLook.h)에 TBitArray<> Met와 bool IsMet(int32 CivIndex) — 백성 인덱스마다 Knowing.MetCivs에 있는가. 패널이 즉위의 문(CrownBlockOf)의 bMet을 여기서 읽는다(표현 계층이 앎을 읽는 곳은 KnownViewOf 하나 그대로 — 불변 규칙 14). 개발용 CosmicSeeAll에서도 같다 — 그 도구는 모든 백성을 보이지만 아무도 만나지 않는다. 같은 IsMet을 칙령 창의 [백성] 탭이 읽는다(행은 IsMet ∧ ShownOrderOf가 INDEX_NONE이 아닌 백성만, 만난 차례 — B3)
// - FCosmicKnownView.PendingContactCivID(B3): Knowing.PendingContactCivID의 사본(KnownViewOf가 PendingPetitionRow처럼 옮긴다 — CosmicSeeAll에서도 같다). 카드 창(UCosmicPetitionCardWidget::ShowCard(Row, SystemID, OtherCivID))은 기다리는 카드가 첫 접촉 카드일 때만 이 백성을 받아 {OtherCiv}를 풀고(ArgsOf, bNamesOther), '손을 내민다' 아래에 우호의 유지비와 자리의 줄(Petition.FriendlyUpkeep — 수는 UI에만)을 더하며, 문이 거절하는 선택지는 회색으로 두고 툴팁에 [백성] 탭의 사유(UCosmicEdictWidget::StanceReasonOf)를 보인다. [백성] 탭과 성계 패널의 함대 줄의 규칙과 문구는 docs/20 §6.3과 헤더 B3 4절·11절 항목 10
// - 카드 창의 문(B4 — 헤더 B4 4절·11절 항목 16·18. 화면의 규칙과 문구는 docs/20 §6.3): 카드가 서 있는 동안 선택지마다 CosmicPetitions::ChoiceGateOf(답이 읽는 그 문, §7.4 'B4' 줄)를 묻고, 막힌 선택지는 회색에 툴팁의 첫 사유(비축이 모자라면 Petition.Short가 먼저 — 답이 비축을 먼저 묻는다). 사유는 꼴마다: 태세 UCosmicEdictWidget::StanceReasonOf, 칙령 UCosmicEdictWidget::ReasonOf(거절한 효과의 칙령 행 — FCosmicChoiceGate::EffectAt으로 찾는다), 함대 static FText UCosmicPetitionCardWidget::ResistReasonOf(ECosmicResistBlock, const FText& Hazard)(NoHazard·Capital은 Petition.Resist.Block.NoHazard·.Capital이 거절한 효과의 위기 이름을 부르고, NoFleet은 Guard.Block.NoFleet, Spent는 패널의 같은 단추 낱말 Siege.Block.NoFleet, None·NotOwn은 빈 글 — 서 있지 않은 카드는 그 사유를 보이지 않는다). 칙령을 내리는 선택지의 툴팁에는 그 뒤에 칙령 창의 말 UCosmicEdictWidget::TipOf를 둔다(열린 선택지는 그 말만). 단추 이름 뒤의 짧은 말: 칙령의 거절은 UCosmicEdictWidget::SuffixOf, 함대 상한 0은 CosmicUI::FleetSuffix(Rules, Tier)(UI/CosmicUILabels.h — Guard.Suffix.NoFleet '(칸 {0}부터)', 수는 CosmicConflict::FirstFleetRungOf; 없으면 빈 글 — 패널의 [함대를 여기에]도 같은 함수, [B-4d2]). 이미 지닌 칙령을 내리는 선택지(Held로 회색)는 이름 앞에 [백성] 탭의 지닌 표 Edict.Mark('●')를 단다([B-4d2] — 짧은 말이 없어 고장처럼 보이지 않게). 선택지 아래 줄: 칙령은 Petition.EdictLine(제정의 값 — UCosmicEdictWidget::CostWordsOf, 값이 없으면 Petition.EdictCostNone — 과 칙령의 자리), 함대의 시간은 Petition.ResistLine(함대 지금 → 뒤 — ResistShareOf, 그 위기의 이름, 더할 틱 — ResistTicksOf). 칙령 창에서 철회한 칙령은 기다리는 카드의 회색 선택지를 곧 연다(컨트롤러의 HUD 갱신이 카드 창의 선택지를 다시 그린다). 수는 UI에만이고 앎에 쓰지 않는다
// - struct FCosmicVesselLook { int32 FromIndex = INDEX_NONE; // 그릇이 선 성계(이동 중이면 떠나온 성계), INDEX_NONE = 표지 없음(통치자·몸이 없는 그릇)  int32 ToIndex = INDEX_NONE; // 서 있으면 INDEX_NONE  float Share = 0.f; // 레인을 간 몫 0~1 } · FCosmicVesselLook CosmicMapLook::VesselLookOf(const FGalaxyState&, const FCosmicStateIndex&, const FCosmicRulesDef&) — Share = 1 − TravelTicksRemaining / LaneTicksOf, 0~1로. 앎을 읽지 않는다(방랑자가 선 성계와 그 레인 이웃은 늘 아는 성계다). 지도가 그 별 둘레에 금색 마름모를, 이동 중이면 그 레인을 밝히고 작은 마름모를 그린다 — B5 `[B-5c]`부터 컨트롤러가 그릇을 보일 때만(ACosmicPlayerController::IsVesselShown, 아래 '죽음과 계승의 쪽')
// - ECosmicAutoPause::Encounter(World/CosmicAutoPause.h, 끝에 덧붙임 — 저장되지 않는다): 방랑자 카드가 와서 시간이 멈춘 까닭('정지 — 카드가 왔다' — 청원의 Card와 따로)
// - ECosmicAutoPause::Death(B5 — 끝에 덧붙임, 저장되지 않는다): 생애가 닫힌 경계에 컨트롤러가 자동 정지의 선택과 상관없이 시간을 멈춘 까닭('정지 — 한 생애가 닫혔다' — AutoPause.Why.Death). 그 뒤의 쪽은 아래 '죽음과 계승의 쪽'(B5 `[B-5c]`)
// - ACosmicPlayerController(World/CosmicGameMode.h): bool TravelTo(FName SystemID) · int32 ScanHere()(새로 안 성계의 수, 0 = 새것 없음, INDEX_NONE = 거절) · bool TakeThrone(FName CivID)(그릇이 선 성계의 주인 — 복귀도 같은 길) · bool Abdicate() — UPlayerRoleSubsystem의 명령으로 넘긴다. 끝난 판은 거절. B5: static bool LifeClosedAt(const FGalaxyState&, int64 Tick)(그 틱에 생애가 닫혔다 — 닫힌 마지막 생애의 DiedTick == Tick: 공백기면 Lives의 끝, 도약 0의 계승이면 그 앞 생애. LifeState로 묻지 않아 도약 0의 죽음도 잡는다) · 틱 경계의 FollowDeath(그 술어가 서면 ApplySpeed(0, ECosmicAutoPause::Death)와 GetDeathTick — 끝의 래치 FollowEnd보다 먼저; `[B-5c]`부터 비문을 연다 — 아래 '죽음과 계승의 쪽') · bool AdvanceLeap()(빨리 넘기기의 입력 — 차례와 거짓의 경우는 §7.4 RunLeap: 공백기가 아니거나 끝난 판이거나 돌릴 틱이 없거나 시뮬이 거절하면 거짓이고 아무것도 돌리지 않는다; 시계로 미리 자르지 않고, 끝 스위치가 꺼진 세션에서 기록이 넘김을 멈추면 한 번 더 넘긴다; 남은 도약이 0인 침묵(끝 스위치가 꺼진 세션 — 켜져 있으면 침묵은 그 틱의 끝이다)은 시간을 정지 전의 배속으로 돌려준다 — `[B-5c]`부터 쪽이 서 있으면 쪽이 접힌 뒤) 받아들인 명령은 HUD를 다시 그리고 지도에 다시 그리기를 청한다(멈춘 시뮬은 틱을 보내지 않는다). 즉위는 새 백성의 서 있는 위기를 곧바로 기억하고(FollowOwnHazards — 다음 틱에 막 선 위기는 여전히 시간을 멈춘다) 기다리는 카드를 따른다. 개발용 콘솔(UFUNCTION(Exec) — 화면 찍기와 손 확인): CosmicTravel <성계>(인자 없으면 선 성계의 레인마다 문의 답을 로그에), CosmicScan, CosmicCrown(선 성계의 백성), CosmicAbdicate(한 번 — 두 번 누름은 화면의 것). 단추와 같은 길이고 거절은 사유와 함께 한 줄

// 죽음과 계승의 쪽(B5 `[B-5c]` — 표현 계층의 인터페이스, 상태·해시·저장·추첨 밖. 화면의 흐름은 docs/20 §6.3 '죽음과 계승의 쪽'과 헤더 B5 §4·§2.6·§2.10, 쪽의 글은 docs/70 §8.4):
// - enum class ECosmicCallingPage : uint8 { None, Epitaph, Digest, Inauguration, ReignEnd }(UI/CosmicCallingPagesWidget.h — 저장되지 않는다, 두 해시 밖) · struct FCosmicAnswerTitle { FName Row(카드의 DT_Encounters 행 — FLifeRecord.LastChoiceRows); FText Title(제목, 슬롯은 그 답의 줄로 푼다); bool bOfTheDeath(가장 새 답이고 그 카드가 생애를 닫은 바로 그 위기의 카드다 — 상태가 아직 그곳을 말하면 그 위기가 닫은 성계에서: 비문이 사인 줄과 떨어진 자리에 둔다, 헤더 §3.6의 1 — 희생의 읽힘이 없게) }
// - namespace CosmicCallingPages: bool IsCardOfHazard(const FEncounterDef& Card, const FHazardDef& Hazard)(카드의 조건 하나가 그 위기의 태그를 부른다 — enc_star_watch의 PlayerInSystemWithHazard Hazard.PreSupernova, petition_unstable_star의 SystemHasHazard) · TArray<FCosmicAnswerTitle> LastAnswersOf(const FGalaxyState&, const FCosmicDataTables&, int32 LifeAt)(Lives[LifeAt]의 LastChoiceRows — 마지막 셋, 오랜 것 먼저 — 의 제목을 시간순으로: 행마다 그 답의 줄 — 그 생애 안(BornTick ~ DiedTick, 열린 생애는 State.Tick까지)의 State.Chronicle·State.Log에서 SourceRow가 그 카드 행인 줄(UPlayerRoleSubsystem::AnswerPetition의 결과 줄), 가장 새 답부터 거슬러 줄마다 한 번 — 과 짝짓고 {System}은 그 줄의 FocusSystemID, {Civ}는 FocusCivID(상태가 아직 가진 백성)로 푼다. 그 줄로 풀 수 없는 슬롯({OtherCiv}·{Hazard}·{Rebel}, 줄이 없는 슬롯, 상태가 더는 갖지 않는 성계·백성)의 제목, 테이블에 없는 행, 읽는 이에게 말하거나 그릇을 높이거나 죽은 그릇의 이름 아래서 추모로 읽히는 카드(CosmicRows::EpitaphLeavesOut = { PetitionKeepThrone, PetitionExalt, PetitionWiseHeart, PetitionFeast, EncPlagueTend, EncStarWatch } — Data/CosmicRowNames.h, 스키마의 epitaph_leaves_out과 같다 §7.3; 결정 12-다: 죽음을 건너 '당신'은 없다, §4: 죽음에는 아무것도 오르지 않는다; 뒤의 넷은 `[B-5d1]`의 글 감사 — 덕·기념일·희생의 '곁', 헤더 §3.6의 1. 제목은 제 카드에서 그대로다)는 뺀다. bOfTheDeath는 Death.Hazard로 닫힌 생애의 가장 새 답에만. 상태와 테이블만 읽고 앎은 읽지 않는다)
// - UCosmicCallingPagesWidget(UUserWidget, WBP 없는 C++ — ACosmicHUD가 Z 15에 세운다: 카드(5)·승천 연출(10) 위, 연대기(20) 아래; ACosmicHUD::GetCallingPages()): void OpenEpitaph(int32 LifeAt, bool bSkippable, bool bAtOnce)(제목 '제 N대 · <그릇 이름>' — 3인칭, 결정 12-다; 사인과 원천 — 위기의 이름, 카드가 원천인 사인(Death.Encounter)은 그 카드가 속한 위기의 이름(IsCardOfHazard) 또는 원천 없음이고 카드의 제목은 사인 줄에 서지 않는다; 일을 맡은 주기(Epitaph.Lived — `[B-5d1]`의 글) DiedTick − BornTick; 가장 높은 칸; 업적; 마지막 답 — 죽음의 답은 다른 답 아래 떨어져서; 별칭 없음. bSkippable = '남은 쪽 넘기기'(둘째 죽음부터), bAtOnce = 막과 드러냄 없이 다 보인다(이어 연 판)) · void OpenDigest()(CosmicDigest::LinesOf — DigestOf(Sim, LastClosedLifeOf, 컨트롤러의 IsSeeingAll) — 의 줄마다 한 행: 머리가 제목, 도약 0의 한 줄은 그대로; 막은 비문에서 이어 선다) · void OpenInauguration(int32 LifeAt)('제 N대', 새 그릇의 사실 줄 — 이름·Chronicle.BornAmong('{Civ}의 사람' — `[B-5d1]`의 글)의 출생 백성·성계·몸·정신 — 과 고정 줄 Succession.Kept) · void OpenReignEnd(int32 LifeAt)(Reign.Title, 마지막 재위의 백성과 다스린 주기 ReignTicks, 까닭 Reign.End.Abdicated·Deposed·Other — Reign.* 키만: Epitaph. 키·'제 N대'·죽음의 낱말·일을 맡은 주기가 없다) · void Close() · bool IsOpen() const · ECosmicCallingPage GetPage() const · int32 GetLifeAt() const(비문·다이제스트는 닫힌 생애, 취임은 새 생애, 재위 끝은 걷는 생애; 접히면 INDEX_NONE) · bool IsRevealing() const(줄이 아직 드러나는 중 — 단추가 기다린다) · void Advance(float DeltaSeconds)(막과 드러냄 — 연대기 화면의 VeilSeconds·LineSeconds·RevealCapSeconds) · void PressVeil()(막의 누름 대신 — 줄을 다 보이고 단추를 세운다) · void PressNext()('다음'·'지도 보기' — 컨트롤러의 NextCallingPage; 드러나는 동안은 아무것도 않는다) · void PressSkip()(비문의 '남은 쪽 넘기기' — SkipCallingPages; 드러나는 동안과 서지 않았을 때는 아무것도 않는다) · 테스트가 읽는 GetPageTexts·GetPageKeys(쪽의 말이 온 ST_CA 키 전부, 단추 포함)·GetApartRow·GetCauseRow·GetFactsEndRow·GetAnswers·GetNextButton·GetSkipButton. 쪽 위의 누름은 지도의 클릭이 아니다. 상태를 쓰지 않고 앎을 읽지 않는다(다이제스트는 제 함수가 KnownViewOf로 읽는다)
// - ACosmicPlayerController(B5 `[B-5c]`): void NextCallingPage()(서 있는 쪽의 '다음' — 비문: 도약을 돌리고(AdvanceLeap — 그 안의 끝은 제 틱에서 래치한다; 넘김이 침묵(서 있는 후보 없음)으로 끝나면 그 틱이 끝이다 — Ending.NoCandidate) 그 경계 뒤 다이제스트를 연다, 판이 이미 끝났으면(끝 틱의 죽음) 연대기; 다이제스트: 새 생애가 서 있으면 취임 쪽, 판이 끝났으면 연대기(공백기 안의 끝, 계승의 틱의 끝 — 취임 쪽 없음), 침묵이면(끝 스위치가 꺼진 세션 — 래치가 없다) 쪽을 접고 시간을 돌려준다(AdvanceLeap); 취임: 쪽을 접고 카메라가 새 그릇의 성계로 가며(SelectAndFocus — 보는 이의 고름이 아니다) 시간은 멈춘 채 ApplySpeed(0, Succession); 재위 끝: 접고 멈춘 채(그 까닭 그대로). 연대기는 그 기다림 없이 곧 연다 — 쪽이 기다림의 자리에 섰다. 입력이다 — 핸들러 안이나 쪽이 없으면 아무것도 않는다) · void SkipCallingPages()(비문의 '남은 쪽 넘기기', 둘째 죽음부터: 도약('다음'과 같이)과 남은 쪽 전부를 한 번에 — 그다음 취임 쪽의 닫힘, 판이 끝났으면 연대기(끝 스위치가 켜져 있으면 침묵도 그 틱의 끝 — Ending.NoCandidate), 끝 스위치가 꺼진 세션의 침묵이면 시간을 돌려준다; 다른 쪽이나 핸들러 안에서는 아무것도 않는다) · bool IsCallingPageOpen() const · static bool IsCallingPageOpenFor(const UUserWidget&)(위젯을 가진 컨트롤러의 답 — 없으면 false). 쪽이 서 있는 동안: 상단 바의 배속 단추가 회색, 숫자 키·Space는 아무것도 않고(PressSpeedKey), 배속은 서지 않으며(ApplySpeed — 양의 배속과 까닭 없는 그냥 정지는 쪽의 정지와 그 까닭을 그대로 둔다: 승천 연출·카드·침묵의 되돌림이 쪽을 기다린다), 휠은 카메라를 움직이지 않고(HandleWheel), 연대기의 기다림(AdvanceRunEnd)은 세지 않는다
// - ACosmicPlayerController — 그릇을 그리는가(B5 `[B-5c]`): bool IsVesselShown() const(화면이 그릇을 그리는가 — 쪽이 서 있으면 거짓(다이제스트와 취임 쪽은 취임이 소개할 새 그릇 위에, 재위 끝 쪽은 그것이 시작하는 걸음 위에 선다), 끝난 판에서 끝 틱에 난 그릇(CosmicChronicle::ShownLivesOf < Lives.Num() — 플레이어가 본 적이 없다)도 거짓. 공백기·통치자의 규칙과 방랑자 자신의 검사는 화면마다 그대로. 읽기만 한다) · static bool IsVesselShownFor(const UUserWidget&)(위젯을 가진 컨트롤러의 답 — 없으면 true: 테스트의 제 위젯). 읽는 곳 — 상단 바의 방랑자 줄과 치명 경고, 지도의 그릇 표지와 레인(월드의 첫 플레이어 컨트롤러), 성계 패널의 방랑자 줄(경외·스캔 없음)·방랑자 명령·치명 경고·[이동]의 경고 · void RefreshVesselScreens()(그 화면들을 다시 그린다 — HUD의 바와 패널(ACosmicHUD::RefreshPanel), 다음 프레임의 지도(RequestRefresh): 쪽이 접히면 CloseCallingPages가 부른다 — 멈춘 시뮬은 틱을 보내지 않는다) · FSimpleMulticastDelegate OnRunEndChanged(B5 `[B-5d1]` — 끝의 래치가 바뀔 때마다 바뀐 뒤에 한 번: FollowEnd가 까닭을 래치할 때와 ClearRunEnd가 래치된 끝을 잊을 때. 틱이 다시 그리지 않는 것을 그 래치로 보이는 화면이 듣는다 — 로그 피드가 끝 틱이 세운 그릇의 취임 줄을 숨기고 다시 보인다; 핸들러는 읽기만 한다 — 경계의 핸들러 안에서 돌 수 있다) · 로그 피드(UCosmicLogFeedWidget, `[B-5d1]`): 줄 하나의 규칙 Shows(Entry) = bUntold가 아니고 CosmicChronicle::IsUnseenLifeLine(상태, Entry, ACosmicPlayerController::IsRunEndedFor(위젯))이 아니다 — 다시 짓기(Refresh)와 새 줄(OnLogEntry)이 같은 규칙; NativeConstruct가 가진 컨트롤러의 OnRunEndChanged에 Refresh를 묶고 NativeDestruct가 푼다
// - ACosmicPlayerController — 이어하기·휠·경계(B5 `[B-5c]`): void FollowInterregnumOnOpen()(공백기에서 열리는 판, 또는 계승의 틱에서 — 생애가 닫힌 뒤 틱이 돌지 않은 채(넘김의 경계 저장, 도약 0의 죽음, 다이제스트나 취임 쪽 동안의 종료) — 열리는 판은 마지막으로 닫힌 생애의 비문을 다 드러난 채 다시 열고 멈춘다: 흐르던 시간과 까닭 없이 멈춘 시간은 Death로, 제 까닭이 있는 정지(이어 연 판의 Continued)는 그대로. 계승 뒤면 그 '다음'이 넘길 도약을 찾지 못하고 다이제스트와 취임 쪽으로 간다 — 이미 본 쪽을 다시 보일 수 있다(받아들임). 끝난 판·쪽이 서 있을 때·HUD의 쪽이 없을 때는 아무것도 않는다; 컨트롤러와 HUD의 BeginPlay가 FollowEnd(false) 뒤에 — 둘째가 연다 —, HandleGalaxyGenerated(로드)가 제 것 뒤에 부른다) · void HandleWheel(FKey)(이제 public — 테스트가 휠 대신 부른다; 연대기나 쪽이 서 있으면 아무것도 않는다 — 막이 휠을 받고 그 아래 카메라는 움직이지 않는다). private: bool FollowDeath(int64 Tick)(그 틱에 생애가 닫혔으면 참 — ApplySpeed(0, Death)와 함께 카드를 접고, 떠 있는 승천 연출을 되돌림 없이 접고(FoldSilently) 비문을 연다; 끝의 래치 FollowEnd보다 먼저) · void FollowReign(bool bArrived)(경계마다 몸을 입은 통치자였는가와 생애 수를 적는다; bArrived(생애가 닫히지 않은 경계)에서 지난번에 몸을 입은 통치자로 적은 같은 생애가 지금 걷고 마지막 재위가 까닭과 함께 닫혔으면(양위·폐위 — 열째 단계) 연출을 같은 꼴로 접고 재위 끝 쪽을 열며 자동 정지와 상관없이 ApplySpeed(0, ReignEnd); bArrived가 거짓(BeginPlay·새 은하·로드·틱 사이의 즉위)이면 적기만 한다. 통치자의 죽음은 비문이 받는다) · void CloseCallingPages()(쪽을 접고 RefreshVesselScreens — ClearRunEnd도 쪽을 접는다) · void FinishCallingPages()(쪽을 접고, 끝난 판이면 연대기를 곧 연다) · void CloseInauguration()(취임 쪽을 접고 카메라와 Succession)
// - void UCosmicAscensionOverlayWidget::FoldSilently()(B5 `[B-5c]`): 되돌림·드러냄·종소리 없이 곧 접는다 — 멈추기 전 배속을 잊고 울리던 종소리를 끊고(Chime->ResetAudio) 서 있는 드러냄도 끝낸다(셋째 칸의 드러냄은 다시 오지 않는다 — 받아들임). 죽음의 쪽과 재위 끝 쪽이 그 위에 열릴 때(FollowDeath·FollowReign — 죽음과 재위 끝은 승천 연출을 입지 않고, 쪽이 제 까닭으로 시간을 멈춘다). 연출도 드러냄도 서 있지 않으면 아무것도 않는다
// - FCosmicKnownView CosmicMapLook::KnownViewOf(const FGalaxyState&, const FCosmicStateIndex&, EAscensionTier ViewerTier, bool bSeeAll, int64 CutTick)(B5 `[B-5c]`, 헤더 §2.7): CutTick 전에 부름이 알던 뷰 — Knowing의 표지 가운데 Tick < CutTick인 것만 센다(아는 성계, 만난 백성과 만난 차례의 자리 — 표지는 시간순으로 덧붙으므로 센 표지는 자르지 않은 뷰의 제자리를 지킨다, Calling.HardReset). 보는 쪽 Tier는 받은 것(지난 칸은 남지 않는다 — 부르는 쪽의 지금 칸); 성역의 이름·bSeeAll·기다리는 카드(그 표지에는 틱이 없다)는 자르지 않은 뷰와 같다. 다이제스트가 다음 생애의 BornTick으로 읽는다 — 계승의 Install이 그 틱에 적은 출생 성계·출생 백성·방랑자의 이웃은 그릇이 없던 동안의 앎이 아니다. 여전히 앎을 읽는 하나다(불변 규칙 14): 네 인자 꼴은 자르지 않은 이것(CutTick = TNumericLimits<int64>::Max())이고 예전과 똑같이 답한다
// - 치명 위기의 경고와 정지(B5 `[B-5c]` — 헤더 §2.8·§4): ECosmicAutoPause 끝에 셋을 덧붙였다(저장되지 않는다) — LethalHazard(몸을 입은 방랑자가 서 있는 것으로 치는 성계에 치명 위기가 새로 섰다: 방랑자 자신의 위기 정지이고 자동 정지라 토글을 따른다 — 다스리는 성계의 것은 OwnHazard; '정지 — 서 있는 성계의 위기가 만료하면 쓰러질 수 있다' AutoPause.Why.Lethal — `[B-5d1]`의 글, 까닭은 docs/20 §6.3), Succession(취임 쪽이 닫혔다 — 카메라가 새 그릇의 성계로 갔고 시간이 플레이어를 기다린다; '정지 — 새 그릇에게 일이 맡겨졌다' Succession.Stopped), ReignEnd(한 생애 안에서 재위가 끝났다 — 양위·폐위의 쪽이 멈춘 지도 위에 열렸다; '정지 — 다스림이 끝났다' Reign.Stopped). 뒤의 둘은 Death처럼 자동 정지가 아니다 — 토글과 상관없이 선다
// - int32 CosmicAutoPause::LethalPlaceOf(const FGalaxyState&, const FCosmicStateIndex&, const FCosmicDataTables&, const FCosmicRulesDef&)(몸을 입은 방랑자가 서 있는 것으로 치는 성계(CosmicWanderer::PlaceIndexOf — 이동 중이면 떠나온 성계: 떠난 그릇은 닿기 전까지 거기 선다. B5 `[B-5r]`부터 이동 중에는 만료의 틱이 도착의 틱 State.Tick + Player.TravelTicksRemaining 이하일 때만 — 한 틱 안에서 위기 단계가 이동 단계보다 먼저다; 도착 뒤에 만료하는 위기는 두고 떠난다)에 서 있는 위기가 만료에 그릇을 죽이고(CosmicHazards::KillsAVesselOf) 그 죽음이 그때 이 그릇에 닿으면 — Apply가 KillPlayer를 적는 규칙 CosmicHazards::VesselAtRiskOf(출생 보호 포함)를 만료의 틱 State.Tick + max(1, HazardTicksRemaining)에 묻는다(위기 단계는 다음 틱에 먼저 세고 0에서 만료한다) — 그 성계의 자리, 아니면 INDEX_NONE(통치자, 공백기, 그런 위기 없음, 보호 안에서 만료하는 위기, 이동 중이면 도착 뒤에 만료하는 위기). 서 있으면 아래 LethalStayOf(그 성계, State.Tick)가 None이 아닌 곳과 같다. 성계 패널(위기 줄 아래 Wander.Lethal, 모든 [이동]의 툴팁 Wander.TravelLethalTip — 지금 떠나도 닿기 전에 만료하면(LethalStayOf(그 성계, State.Tick) == NoWayOut — 이동 중이면 늘) Wander.TravelLethalLate, `[B-5r]`)·상단 바(LethalLineOf)·카드 창(UCosmicPetitionCardWidget::LethalLineOf, `[B-5r]`)·정지(FollowLethalHazard)가 이 한 답을 읽는다 — 규칙을 베끼지 않는다)
// - enum class ECosmicLethalStay : uint8 { None, CanLeave, NoWayOut }(World/CosmicAutoPause.h, B5 `[B-5r]` — 저장되지 않는다, 두 해시 밖): 그릇이 와서 설 성계의 치명 위기가 그 그릇에게 무엇인가 — None(경고 없음: 그곳에 만료에 그릇을 죽이는 위기가 없다, 그릇이 서기 전에 만료한다, 그때 그 죽음이 그릇에 닿지 않는다 — 출생 보호), CanLeave(그릇이 선 뒤 레인 하나의 시간보다 늦게 만료한다 — 곧 다시 떠나면 그 전에 다른 곳에 닿는다), NoWayOut(레인 하나의 시간 안에 만료한다 — 다시 떠난 그릇도 닿기 전까지는 거기 서 있어(헤더 B5 §2.8) 떠남이 나갈 길이 아니다)
// - ECosmicLethalStay CosmicAutoPause::LethalStayOf(const FGalaxyState& State, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, int32 SystemIndex, int64 StandsFromTick)(B5 `[B-5r]`): SystemIndex에 StandsFromTick의 끝부터 설 그릇에게 — StandsFromTick은 [이리로 간다]의 도착(State.Tick + CosmicWanderer::LaneTicksOf; 이동 중이면 State.Tick + Player.TravelTicksRemaining), 양위의 걸음(State.Tick + 1 — 다음 틱의 열째 단계), 지금 선 곳(State.Tick) — 그곳의 위기가 만료에 그릇을 죽이고(KillsAVesselOf) 그릇이 온 뒤에도 서 있으며(만료의 틱 > StandsFromTick — 한 틱 안에서 위기 단계가 이동 단계와 열째 단계보다 먼저다) 그 죽음이 그때 그릇에 닿으면(그 성계에 방랑자로 둔 그릇의 사본으로 만료의 틱의 VesselAtRiskOf — 출생 보호 포함) 만료의 틱 ≤ StandsFromTick + LaneTicksOf면 NoWayOut, 더 늦으면 CanLeave; 아니면 None. 만료의 틱은 LethalPlaceOf와 같은 State.Tick + max(1, HazardTicksRemaining)(파일 안 도우미 CosmicAutoPauseImpl::ExpiryTickOf). [이리로 간다]의 도착으로 재면 남은 틱 R ≤ 레인은 None, 레인 < R ≤ 레인 둘은 NoWayOut, R > 레인 둘은 CanLeave. 시간만 잰다 — 살아 있는 레인이 나가는지는 묻지 않는다(그런 레인이 없는 그릇 — B1 결정 21 — 에게는 상단 바의 Wander.Bar.Stuck이 경고 곁에서 말한다). 세계만 읽는다(앎·추첨·쓰기 없음); 상태에 없는 인덱스와 공백기는 None. 읽는 곳 — 서 있는 그릇의 LethalPlaceOf, 도착할 곳과 떠날 곳의 [이리로 간다], 이동 중의 상단 바, [왕좌를 내려놓는다]와 첫 함선의 안내, 카드 창(아래 '경고를 읽는 화면'·'카드 창')
// - void ACosmicPlayerController::FollowLethalHazard()(그 자리와 위기를 지난번과 견준다: 새로 선 치명 위기 — 새 위기, 다른 위기, 그릇의 자리가 위기가 선 곳으로 옴(도착, 위기가 선 성계에서 왕좌를 놓음) — 면 자동 정지(LethalHazard)를 청하고, 그것이 시간을 멈추면 그 성계를 고르고 카메라를 보낸다(SelectAndFocus — 보는 이의 고름이 아니다, FollowOwnHazards처럼). 컨트롤러의 시작·새 은하·로드 뒤의 첫 부름은 적기만 하고(LethalSeen), 공백기와 통치자는 아무것도 적지 않는다. FollowOwnHazards가 끝에 부르고 테스트는 틱 대신 부른다. IsVesselShown은 읽지 않는다 — 쪽 아래와 끝난 판에서는 시간이 이미 멈춰 있어 적기만 한다. `[B-5r]`부터 도착 뒤에 만료하는 위기의 성계를 떠나면 적은 자리를 잊는다 — 그 성계로 돌아와 닿으면 다시 멈춘다)
// - static FText UCosmicTopBarWidget::LethalLineOf(const FGalaxyState&, const FCosmicStateIndex&, const FCosmicDataTables&, const FCosmicRulesDef&)(LethalPlaceOf가 답하면 위기의 이름으로 Wander.Bar.Lethal, 이동 중이면 Wander.Bar.LethalTravelling('떠난 성계의 …은 닿기 전에 만료한다' — `[B-5r]`부터 떠난 성계가 도착 전에 만료할 때만); 그 답이 없고 이동 중이면(`[B-5r]`) 가는 성계의 LethalStayOf(State.Tick + TravelTicksRemaining) — CanLeave면 Wander.Bar.Ahead, NoWayOut면 Wander.Bar.AheadTrap(떠난 성계가 먼저 만료하면 그것이 먼저다) — 약속하지 않는 말; 아니면 빈 글. 바는 방랑자 줄 뒤에 그릇이 보이는 동안 보인다 — const UTextBlock* GetLethalText() const). 상단 바의 끝 예고 Chronicle.Until은 몸을 입은 모든 방랑자에게다(폐위·양위 뒤, 그리고 다스린 적 없이 부른 그릇 — 통치자와 공백기에는 없다); 공백기에는 바에 그릇의 줄(다스리는 백성·방랑자 줄)이 없다
// - 경고를 읽는 화면(B5 `[B-5r]` — 표현 계층, 상태·해시·저장·추첨 밖; `[B-5d2]` 리뷰의 play-1, `[B-5r]` 리뷰의 logic-2): 성계 패널 — 도착할 곳의 [이리로 간다]는 툴팁의 제 말(과 떠날 곳의 경고) 뒤에 LethalStayOf(그 성계, State.Tick + LaneTicksOf)의 띠를 Wander.TravelAheadTip(CanLeave)·Wander.TravelAheadTrapTip(NoWayOut)으로 말하고, 회색의 사유가 제 짧은 말을 두지 않으면 라벨 뒤에 Wander.Suffix.Lethal('(쓰러질 수 있다)'); 경고이지 문이 아니다(TravelBlockOf는 그대로). 이동 중에는 없다(여기서 떠나는 여정이 없다). [왕좌를 내려놓는다]는 LethalStayOf(Player.CurrentSystemID의 성계, State.Tick + 1)의 띠를 툴팁의 제 말(기록된 양위면 Wander.Block.Leaving, 기다리는 폐위면 Wander.Block.Deposed) 뒤에 Wander.AbdicateLethalTip·Wander.AbdicateTrapTip({System}·{Hazard} — '다스림이 끝나도 그릇은 … 그대로 선다': 양위와 폐위 둘 다에 참인 말)으로, 기다리는 역할 효과가 없으면 라벨 뒤에 Wander.Suffix.Lethal — 컨트롤러가 그릇을 보이고 끝나지 않은 판에서만. 첫 함선의 안내: CosmicUI::ShowsShipHint(UI/CosmicUILabels.h)의 일곱째 인자 bRebellionStands를 넓혔다 — 패널이 bCapitalRebellion || 그 [왕좌를 내려놓는다]의 띠 != None을 넘긴다: 수도에 반란이 서 있거나 내려온 그릇이 걸어 나갈 곳에 치명 위기가 서 있으면 줄이 서지 않고(선 그 보기에서도 사라진다) 세션의 한 번도 쓰지 않는다
// - 카드 창(UCosmicPetitionCardWidget, B5 `[B-5r]` — `[B-5d2]` 리뷰의 play-2, `[B-5r]` 리뷰의 logic-1): static FText LethalLineOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules, FName CardSystemID)(LethalPlaceOf가 카드의 성계이면 위기의 이름으로 — LethalStayOf(그 성계, State.Tick)가 NoWayOut이면 Wander.CardLethalTrap(어느 답도 그릇을 옮기지 않고, 레인 하나의 시간 안에 만료해 답한 뒤 떠나도 늦다), 아니면 Wander.CardLethal; 다른 성계·통치자·공백기·아무도 죽이지 않는 위기·보호 안의 만료는 빈 글. 부르는 쪽이 방랑자의 카드에만 묻는다; 남은 시간은 말하지 않는다 — 곁의 패널이 남은 주기를 보인다) · const UTextBlock* GetLethalText() const(본문 아래, 선택지와 결과 위의 줄 — 글이 없으면 접힌다). private ShowLethalLine()이 상태에서 다시 짓는다: Refresh가 결과 아래에서도 부르고(답은 HUD의 다시 그리기 — ACosmicPlayerController::AnswerPetition — 로 여기 온다: 위기를 끝낸 답이 줄을 거둔다), ShowCard의 거둠과 PressClose가 접는다
// - UCosmicActionButton(UI/CosmicActionButton.h, B5 `[B-5r]` — 리뷰 layout-1): void SetWrapsLabel(bool bInWrapsLabel) · bool GetWrapsLabel() const(기본 꺼짐) — 켜면 라벨이 단추의 폭에서 여러 줄로 감기고 가운데 맞춘다(UTextBlock의 SetAutoWrapText와 SetJustification(Center), 라벨의 단추 슬롯은 HAlign_Fill — 기본 HAlign_Center면 글이 제 폭만 받아 라벨이 바뀔 때마다 더 좁게 감긴다). 호출 뒤에 지은 트리에도 선다(RebuildWidget이 private ApplyWrapsLabel을 부른다). 폭이 정해진 열의 단추에만 켠다 — 성계 패널의 SyncButtons 단추들과 [왕좌를 내려놓는다], 카드의 선택지; 상단 바의 단추(Auto 슬롯)는 한 줄 그대로다(Slate는 지난 배치의 폭에서 감으므로 자라는 라벨이 감긴 채 남는다)
// - static bool UCosmicSystemPanelWidget::IsLockedBeforeLife(const FGalaxyState&, const FCivilizationState& Owner, const FCosmicRulesDef&)(Owner의 왕좌 잠금이 그릇의 생애보다 늦지 않게 시작했고 — Calling.BornTick ≥ ThroneLockedUntilTick − DT_Rules.ThroneLockTicks(0 이상) — 그 생애가 왕좌에 앉은 적이 없다(RuledCivIDs 비었음): 앞 죽음의 잠금 속에 난 부른 그릇이라 즉위의 사유를 Wander.Block.Locked('떠난 지 얼마 되지 않아' — 그 그릇에게 거짓이다) 대신 Wander.Block.LockedNew로 보인다(짧은 말은 그대로). 잠기지 않았으면 false. 둘째 조건은 틱 0의 판에서 난 틱에 폐위된 첫 생애가 새 그릇의 말을 받지 않게 한다)

// CosmicSaveSlot (Save/CosmicSaveSlot.h, MVP ⑤ — 바이트의 순수 함수, 파일을 읽거나 쓰지 않는다): Magic 'CASV'(파일 차례), FormVersion 1, HeaderBytes 64(머리 CRC가 앞 60을 덮는다), MaxRawBytes 64 MiB, 봉투의 자리 FormAt 4·GalaxyIDAt 8·SavedAtAt 24·HeaderCrcAt 60(모든 꼴에 고정), enum class ECodec : uint8 { Stored = 0, Oodle = 1 }(파일에 든다 — 끝에만 추가). FHeader { Form, Codec, GalaxyID, SavedAtUtcTicks, Tick, RawBytes, PackedBytes, PayloadCrc, HeaderCrc } · FSeen { GalaxyID, bBackup, UtcTicks, bTimeFromHeader }. Encode(Save, SavedAt, OutSlot, OutRawBytes, Want)(SaveGameToMemory → Oodle Kraken·VeryFast, 코덱이 0을 내면 Stored) · ReadHeader(머리, 파일 길이, 기대 ID) · Check(+ 페이로드 CRC) · Unpack(+ 코덱 — 엔진의 읽개는 길이와 CRC를 지난 바이트만 본다, #70) · ParseSlotName · MaxSlotBytes · NewestRun(판의 시각 = 그 슬롯들 가운데 늦은 것, 같으면 GUID 글자가 늦은 것). enum class ECosmicSlotRead : uint8 { Ok, Missing, Short, OldForm, NotOurs, HeaderDamaged, BadSizes, Truncated, OtherGalaxy, PayloadDamaged, CodecRefused, Unreadable, ReadFailed, NewerForm, NewerSave, Sanctuary, Refused } — Missing~Unreadable이 상함(IsDamage가 하나씩 이름으로 — 백업을 연다), 나머지는 백업 없이 거절. 저장되지 않는다(로그 글).

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
