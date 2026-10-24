# MVP ⑧ 헤더 초안 — 경제·연구 상수 8개 → DT_Rules · 판의 속도와 결정 밀도(ⓑ) · 역병 대응 연결

상태: **승인 (2026-10-24, 개발자 — §8의 결정 8개 전원 권장안)**. **A-8a**(노브 이전·해시 불변) → **A-8b**(밸런스·판의 속도·주기 청원 폴백·역병 대응·기준값 갱신) → **A-8c**(문서) 순서로 구현한다. 승인 전의 기록: 초안 (2026-10-24, 다른 세션).

근거: `docs/50_ROADMAP.md` A ⑧과 종료 조건 ⓑ, `STATUS.md` '2026-10-18 개발자 결정'(경제·연구 상수 → `DT_Rules`, `StarClassEnergy`는 단계 5(B8)까지 C++, `InfluencePerPopulation` 0.5 → 1.0) 및 '다음 작업 2'(이름 둘 고르기), `docs/60_DECISIONS/headers/A4_provisional_end.md` §10 결정 1, `docs/60_DECISIONS/headers/A7_research_focus.md` §9 결정 9·11, `docs/10_DESIGN/DESIGN_AUDIT_AND_IMPROVEMENT.md` 자료 3·4(1-C 역병 대응 연결, 주기 청원 고갈 진단), `docs/40_DEV_GUIDE.md` §9.3, `docs/21_DATA_MODEL_REFERENCE.md` `FCosmicRulesDef`·§7, `CLAUDE.md` 불변 규칙 3·4·7·8·13.

---

## 0. 범위와 전제

- 네 가지를 한다.
  1. **경제·연구 상수 8개 → `DT_Rules` (`FCosmicRulesDef`)**: `Sim/GalaxySimulationSubsystem.cpp`의 `namespace Economy`에 남아 있던 상수 9개 가운데 단계 5(B8, 실제 하늘)까지 C++에 두기로 한 `StarClassEnergy`를 뺀 **8개**(`TierMultiplier`, `MaterialistEnergyBonus`, `InsightPerPopulation`, `SpiritualistInsightBonus`, `InfluencePerPopulation`, `OpenInfluenceBonus`, `ResearchInsightShare`, `ResearchPerInsight`)를 `FCosmicRulesDef` 끝에 옮긴다. `CosmicBalanceCommandlet`의 `-Rules=Name:Value;…`가 `FCosmicRulesDef::StaticStruct()`를 리플렉션으로 읽으므로, 8개 모두 코드 재빌드 없이 커맨드렛에서 바로 실험할 수 있게 된다.
  2. **`InfluencePerPopulation` 0.5 → 1.0** (2026-10-18 결정 10): 인구당 영향력 기본 수입을 두 배로 올려 구호(`project_relief`, 틱당 물질 1·영향력 1)와 개혁(`project_reform`, 틱당 영향력 1), 청원 선택지 비용이 겹칠 때 생기던 영향력 병목과 정착 굶주림(`SettlingStalled`)을 푼다.
  3. **판의 속도와 목표 ⓑ(`decisions by end ≥ 15`) 함께 맞추기** (MVP ④ 결정 1 · MVP ⑦ 결정 9):
     - **원인 진단 1 (주기 청원 영구 불발)**: 왜 `-Focus=0`(T3 중앙값 4,425틱)과 `-Focus=1`(T3 중앙값 1,958틱) 두 정책 판 모두 끝까지의 결정 중앙값이 **12**에서 멈추었는가? `FLifeRecord::RememberedChoices = 3`(`LastChoiceRows` 3칸, 비문용)과 `DT_Encounters.json`의 주기(`Lottery`) 카드 수(**2장**: `petition_road`, `petition_feast`)가 비둘기집 원리로 충돌했기 때문이다. 틱 300과 600에 주기 카드 두 장에 답하면 `LastChoiceRows`가 `[awakening, road, feast]`가 되고, T2에서 상시 카드 `petition_exalt`에 답해도 `[road, feast, exalt]`가 되어 두 `Lottery` 행이 모두 3칸 창 안에 갇힌다. 그 결과 **틱 900부터 판이 끝날 때까지 모든 주기 슬롯(900·1200·1500·1800·2100…)이 후보 0장으로 불발**되고 있었다.
     - **원인 진단 2 (`-Focus=1`과 `-Focus=0`의 속도 격차)**: `-Focus=0`(틱 30에 사회 0.55를 고른 뒤 중점을 바꾸지 않는 판)은 물리·공학 지분이 0.15라 연구가 병목(T2 2,203틱, T3 4,425틱)인 반면, `-Focus=1`(물리 0.55 → 공학 0.55)은 연구 지분이 3.67배 빨라 T1에서는 다이슨(`RequiredProgress` 600), T2에서는 정착 둘(`project_settle.RequiredProgress` 300 × 2 = 600)과 공학 1.0이 거의 동시에 끝나 **1,958틱(×10 약 3.3분)**에 T3에 닿는다.
     - **해법**: ① `CosmicPetitions::Present`에서 `LastChoiceRows` 셋을 모두 뺀 `Lottery` 후보가 비면, **`LastChoiceRows` 안의 가장 최근 `Lottery` 행 하나만** 후보에서 빼도록 폴백을 둔다(수직 슬라이스 10장 구성을 깨지 않고 같은 주기 카드의 연속 등장을 막으면서 틱 900 뒤에도 주기 카드가 번갈아 나오게 함). ② `project_settle.RequiredProgress`를 **300 → 600**(`project_dyson`과 같은 값)으로 올려, `-Focus=0`의 T3 시각(연구 병목 약 4,425틱, 6,000틱 안 26/30판)은 건드리지 않으면서 `-Focus=1`의 T3 시각만 **약 2,560틱(×10 약 4.3분)**으로 늦추고 중점 전환을 4회 → 5회로 늘린다.
  4. **설계 감사(`DESIGN_AUDIT_AND_IMPROVEMENT.md`) MVP 범위 반영 및 가칭 이름 둘 정리**:
     - `Tools/data/DT_Hazards.json`의 `crisis_plague.RulerResolutions`에 `["project_relief"]`를 넣는다(`DESIGN_AUDIT_AND_IMPROVEMENT.md` 자료 4 1-C — 역병 발생 시 구호 사업으로 대응 가능).
     - `STATUS.md` 다음 작업 2의 이름 둘(`Tools/data/DT_Civs.json` `arch_seeker`의 셋째 이름 `'첫 불의 의회'`와 모성계 이름 `'첫 해'`)을 함께 확정한다.
- **새 저장 필드·새 열거형 값·새 추첨 지점은 없다.** `SaveVersion` 1 그대로.
- 커밋 셋:
  - **A-8a**: 경제·연구 노브 8개를 **지금 C++ 값 그대로**(`InfluencePerPopulation = 0.5f`) `FCosmicRulesDef`·`DT_Rules.json`·`schema.json`으로 옮기고 `Sim.EconomyRules` 자동화 테스트와 `test_data_schema.py` 음수 케이스를 더한다. **세계 해시·전체 해시·시드 42 기준값이 그대로**인지 확인한다(`-UpdateBaseline` 없음).
  - **A-8b**: 승인된 밸런스·데이터 값(`InfluencePerPopulation` 1.0, `project_settle.RequiredProgress` 600, 주기 청원 폴백, `crisis_plague.RulerResolutions: ["project_relief"]`, `arch_seeker` 이름 둘)을 반영하고 30판(`-Focus=0`, `-Focus=1`, 입력 없는 20,000틱) 측정과 `-UpdateBaseline`을 수행한다.
  - **A-8c**: 문서(`docs/20`, `docs/21`, `docs/30`, `docs/40`, `docs/50`, `docs/10_DESIGN/systems.md`, `docs/10_DESIGN/crisis_and_transcendents.md`, `OPEN_QUESTIONS.md`, `DECISIONS_LOG.md`, `STATUS.md`, `CHANGELOG.md`, `CLAUDE.md`)를 갱신한다.

---

## 1. 기대는 사실과 진단

### 1.1 지금 두 정책 판의 비교 (`CosmicBalance`, 30판 = 시드 1~29와 42, 6,000틱, `-TellAll=0 -Answer=1 -Build=1`)

| 정책 | T2 중앙값 | T3 중앙값 (닿은 판) | 끝난 판 (T3 / 폐위) | 600틱 결정 (중앙값, 범위) | 끝까지 결정 (중앙값, 범위) | 중점 바꿈 (중앙값, 범위) | 첫 600틱 말해진 줄 |
|---|---|---|---|---|---|---|---|
| **`-Focus=0`** (지금) | 2,203 | 4,425 (26/30) | 30 (26 / 4) | 5 (4~6) | **12 (8~22)** | 0 | 24.5 (14~44) |
| **`-Focus=1`** (지금) | 1,165 | 1,958 (30/30) | 30 (30 / 0) | 7 (6~8) | **12 (10~20)** | 4 (4~4) | 24.5 (14~44) |

### 1.2 왜 두 판 모두 끝까지의 결정 중앙값이 12인가

1. **주기 청원의 비둘기집 교착 (`RememberedChoices = 3` vs `Lottery` 2장)**:
   - `CosmicPetitions::Remember`는 답한 모든 청원(`Standing`·`Lottery` 구분 없이)을 `FLifeRecord.LastChoiceRows` 끝에 붙이고 3개를 넘으면 맨 앞을 지운다.
   - `CosmicPetitions::Present`의 둘째 패스는 `Ruler.LastChoiceRows.Contains(RowName)`인 모든 `Lottery` 행을 후보에서 뺀다.
   - 지금 `DT_Encounters.json`의 10장 가운데 `Lottery` 행은 `petition_road`(새 길)와 `petition_feast`(쉼의 날) **두 장뿐**이다.
   - 따라서 틱 30(`petition_wise_heart`), 틱 150(`petition_awakening`), 틱 300(`petition_road`), 틱 600(`petition_feast`)을 지나면 `LastChoiceRows`는 `[awakening, road, feast]`가 되어 **틱 900 주기 슬롯에서 두 `Lottery` 행이 모두 제외**된다.
   - 그 뒤 T2에 올라 상시 카드 `petition_exalt`에 답해도 `LastChoiceRows`는 `[road, feast, exalt]`가 되어 **여전히 두 `Lottery` 행이 모두 3칸 창 안에 남는다**. 상시 카드 두 장이 연달아 끼어들지 않는 한 **틱 900부터 판 끝(1,958틱이든 4,425틱이든)까지 주기 청원이 단 한 장도 오지 않는다**.
   - 그 결과 안정도가 0.5 위로 유지되는 대다수 판에서 통치자가 받는 청원은 판 전체를 통틀어 **5장**(`wise_heart`, `awakening`, `road`, `feast`, `exalt`)뿐이고, 여기에 사업 3개(`dyson`, `settle` × 2)와 중점 전환 4회를 더한 `5 + 3 + 4 = 12`가 그대로 중앙값이 되었다.

2. **연구 지분 비율(3.67배)과 정착 시간(`300 × 2 = 600`틱)**:
   - `TickEconomy`가 매 틱 `Income.Insight`를 더하고 `TickResearch`가 `Stockpile.Insight * ResearchInsightShare`를 빼므로, 청원이 통찰을 쓰지 않는 정상 상태에서 매 틱 연구로 들어가는 통찰(`ResearchInsight`)은 `ResearchInsightShare`와 무관하게 `Income.Insight`에 수렴한다.
   - 따라서 `ResearchPerInsight`(0.003)를 낮춰 `-Focus=1`을 늦추려 하면 `-Focus=0`(지분 0.15)은 그 3.67배만큼 더 늦어져 6,000틱 창 밖으로 밀려난다(예: `ResearchPerInsight`를 0.002로 낮추면 `-Focus=0`의 공학 1.0은 약 6,640틱이 되어 6,000틱 안에 T3에 닿지 못한다).
   - 반면 **T2 정착 사업(`project_settle.RequiredProgress`)**은 `-Focus=0`에서는 여유 시간(T2 2,203틱 뒤 공학 1.0까지 2,222틱이 걸리므로 정착 둘이 600틱이든 1,200틱이든 그 안에 일찌감치 끝남) 안에 들어 있어 **`-Focus=0`의 T3 시각을 전혀 늦추지 않으면서 `-Focus=1`과 AI의 T2→T3 시간만 정확히 늘린다**.
   - 또한 `project_settle.RequiredProgress`를 300 → **600**으로 올리면, `-Focus=1`에서 두 번째 정착이 끝나기 전에 공학 1.0이 먼저 차올라 `PolicyFocusOf`가 **다섯 번째 중점 전환(`Engineering → Even`)**을 수행하므로 결정 수가 1회 더 늘어난다.

---

## 2. 경제·연구 노브 8개 (`FCosmicRulesDef` — `Data/CosmicDefinitionRows.h` 끝)

`ResearchFocusShare` 바로 뒤에 아래 8개 필드를 순서대로 선언한다(`StarClassEnergy`는 `Sim/GalaxySimulationSubsystem.cpp`에 그대로 둔다).

```cpp
	// Economy income multipliers by ascension tier (Tier1..Tier5, section 5.1, 16 decision 10). Index out of range,
	// below 0, or not a number reads as 1.0 (0.0 stays 0.0).
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<float> TierMultiplier = { 1.0f, 1.5f, 2.2f, 3.0f, 3.0f };

	// Bonus to Energy income per point of Materialism (-Spiritualism > 0, section 5.1). 0 or below, or not a number:
	// no bonus.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaterialistEnergyBonus = 0.2f;

	// Base Insight income per unit of Population before ethics and tier multipliers (section 5.1). 0 or below, or not
	// a number: 0.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InsightPerPopulation = 1.0f;

	// Bonus to Insight income per point of Spiritualism (Spiritualism > 0, section 5.1). 0 or below, or not a number:
	// no bonus.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpiritualistInsightBonus = 0.2f;

	// Base Influence income per unit of Population before ethics and tier multipliers (section 5.1, 16 decision 10).
	// 0 or below, or not a number: 0.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InfluencePerPopulation = 0.5f;

	// Bonus to Influence income per point of Openness (Openness > 0, section 5.1). 0 or below, or not a number:
	// no bonus.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float OpenInfluenceBonus = 0.2f;

	// Fraction of Stockpile.Insight converted into research each tick (section 5.4). Clamped to [0, 1]; not a number
	// reads as 0.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ResearchInsightShare = 0.5f;

	// Research progress gained per unit of Insight spent in TickResearch (section 5.4). 0 or below, or not a number:
	// no progress.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ResearchPerInsight = 0.003f;
```

- **A-8a 기본값**: C++ 생성자 기본값과 `DT_Rules.json` 모두 `InfluencePerPopulation = 0.5f`로 두어 기존 C++ 상수와 바이트 단위로 같은 부동소수점 연산을 수행한다(세계·전체 해시 불변).
- **A-8b 밸런스 반영**: `Data/CosmicDefinitionRows.h` 기본값과 `Tools/data/DT_Rules.json`의 `InfluencePerPopulation`을 `1.0f`(`1.0`)로 올린다.
- **런타임 방어 (`TickEconomy` · `TickResearch`)**:
  - `TierMultiplier`: `TierIdx`가 배열 범위 밖이거나 값이 `< 0.f` 또는 `NaN`(`!(Value >= 0.f)`)이면 `1.0f`로 읽는다(`0.0f`는 그대로 `0.0f` — 테스트에서 특정 칸의 수입을 0으로 잠글 수 있다).
  - `MaterialistEnergyBonus`, `InsightPerPopulation`, `SpiritualistInsightBonus`, `InfluencePerPopulation`, `OpenInfluenceBonus`, `ResearchPerInsight`: `Value > 0.f ? Value : 0.f`로 읽는다(음수·`NaN`은 `0.f`).
  - `ResearchInsightShare`: `Value > 0.f ? (Value < 1.f ? Value : 1.f) : 0.f`로 읽는다(음수·`NaN`은 `0.f`, `1.f` 초과는 `1.f`).
- **스키마 (`Tools/data/schema.json`의 `DT_Rules.columns`)**:
  - `"TierMultiplier": {"kind": "numbers", "len": 5, "min": 0}`
  - `"MaterialistEnergyBonus": {"kind": "number", "min": 0}`
  - `"InsightPerPopulation": {"kind": "number", "min": 0}`
  - `"SpiritualistInsightBonus": {"kind": "number", "min": 0}`
  - `"InfluencePerPopulation": {"kind": "number", "min": 0}`
  - `"OpenInfluenceBonus": {"kind": "number", "min": 0}`
  - `"ResearchInsightShare": {"kind": "number", "min": 0, "max": 1}`
  - `"ResearchPerInsight": {"kind": "number", "min": 0}`

---

## 3. 주기 청원 폴백과 판의 속도 (`CosmicPetitions::Present` · `DT_Megaprojects`)

### 3.1 주기 청원 슬롯의 비둘기집 교착 해소 (`Events/CosmicPetitions.cpp`)

`CosmicPetitions::Present`의 둘째 패스(`Lottery` 후보 수집)를 아래와 같이 정리한다:

1. **1차 수집 (지금과 같음)**: 조건(`Qualifies`)이 서고 `BaseWeight > 0.f`인 `Lottery` 행 가운데 `Ruler.LastChoiceRows`에 **없는** 행을 모은다.
2. **폴백 수집 (1차 후보가 비었을 때만)**: 조건이 서는 `Lottery` 행이 테이블에 2~3장뿐이라 `LastChoiceRows`(길이 3) 안에 모두 들어간 경우, `Ruler.LastChoiceRows`를 뒤에서부터 읽어 **가장 최근에 답한 `Lottery` 행 하나(`LastPeriodic`)만** 제외하고 나머지 적격 `Lottery` 행을 후보로 모은다.
   - 테이블에 적격 `Lottery` 행이 **1장뿐**이고 이미 답했다면 `LastPeriodic`이 곧 그 행이므로 폴백 후보도 비어 연속 재등장을 막는다.
   - 테이블에 적격 `Lottery` 행이 **2장**(`petition_road`, `petition_feast`)이면 틱 300(`road`) → 틱 600(`feast`) 뒤 틱 900에는 가장 최근 `Lottery`인 `feast`만 제외되어 `road`가 나오고, 틱 1200에는 `feast`가 나와 **같은 주기 카드가 연달아 나오지 않으면서 매 주기 슬롯마다 끊기지 않고 제시**된다(사이에 `Standing` 카드 `petition_exalt`가 끼어도 가장 최근 `Lottery`는 여전히 직전 주기 카드이므로 순서가 흔들리지 않는다).
   - 추후 단계에서 `Lottery` 행이 4장 이상으로 늘어나면 1차 수집이 비지 않으므로 자동으로 최근 3장을 모두 피한다.
   - `Sim.PetitionPresent`의 `{ LotA, LotB }` 단언은 이 규칙에 맞춰 "`LastChoiceRows = { LotA, LotB }`이면 직전 `Lottery`인 `LotB`를 피하고 `LotA`를 고른다(단일 `Lottery` 행만 있을 때는 아무것도 내놓지 않는다)"로 갱신한다.

### 3.2 정착 사업 진척 요구량 (`Tools/data/DT_Megaprojects.json`)

- `project_settle.RequiredProgress`: **300.0 → 600.0** (`project_dyson`과 같은 600.0).
  - `InfluencePerPopulation`이 0.5 → 1.0으로 오르면 구호(`project_relief`)가 막히지 않고 빠르게 끝나 정착 슬롯의 물질 경합도 줄어든다.
  - `project_settle.RequiredProgress`를 600.0으로 두면:
    - **`-Focus=1` 판**: T2(약 1,165틱) 뒤 두 성계를 차례로 정착하는 데 최소 1,200틱이 걸려 T3 도달이 **1,958틱 → 약 2,560틱**으로 늦춰진다. 그사이 틱 1,950 무렵 공학 1.0이 먼저 차올라 `Engineering → Even` 다섯째 중점 전환이 일어나고, 틱 900·1200·1500·1800·2100·2400 주기 청원이 정상 제시되어 **끝까지의 결정 중앙값이 12 → 18~20(최소 ≥ 15)**으로 올라 종료 조건 ⓑ를 넉넉히 넘긴다.
    - **`-Focus=0` 판**: T2(약 2,203틱) 뒤 두 성계 정착(약 1,200틱, 틱 3,400 무렵 완료)이 공학 1.0 연구 완료(약 4,425틱) 안에 들어오므로 **T3 중앙값(약 4,425틱)과 6,000틱 안 완주 판 수(26/30)가 그대로 유지**된다.
    - **입력 없는 20,000틱 세계 판**: AI의 T2→T3 최소 정착 시간이 600틱 → 1,200틱으로 늘어나 AI T3 중앙값이 2,700틱 → **약 3,300틱**으로 이동한다(A3 헤더 §1.1의 5가 지적했던 '정착 둘이 600틱 만에 끝나는 현상' 완화).

---

## 4. 설계 감사(`DESIGN_AUDIT_AND_IMPROVEMENT.md`) 연계 및 가칭 이름 둘

### 4.1 `crisis_plague` 대응 사업 연결 (`Tools/data/DT_Hazards.json`)

- `DESIGN_AUDIT_AND_IMPROVEMENT.md` 자료 3(리스크 C-1) 및 자료 4(개선안 1-C):
  - 지금 `crisis_plague`(성간 역병, 카운트다운 200틱)는 `RulerResolutions: []`라, 청원 `petition_plague`에서 겸비의 선택지(`"앓는 이들 곁에 선다"`, `Spiritualism +0.05`, `Reverence +5`)를 고른 뒤 성계 패널에서 구호 사업을 시작할 길이 막혀 있었다.
  - `project_relief`(구호, `RequiredTier: Tier1_Planetary`, `RequiredProgress: 150.0`, 비용 물질 1·영향력 1)는 대사(`"{Civ}의 구호선이 {System}에 닿았다."`, `"{Civ}|hpp(은,는) {System}의 밤을 함께 지켰다."`)와 완료 시 위기 해소(`ResolveTemplates`: `"{System}의 역병이 물러갔다."`)가 이미 모두 갖추어져 있고, `RequiredProgress` 150틱이 역병 카운트다운 200틱 안에 들어온다.
  - 따라서 `crisis_plague.RulerResolutions`를 `[]` → **`["project_relief"]`**로 바꾼다.
  - **AI 영향 확인**: `GalaxySimulationSubsystem.cpp`의 `ClaimLeaveProfiles` 필터는 `Hazard.Claim`만 걸러내므로(`Crisis == ClaimTag`), 역병이 돈 성계를 가진 모든 백성(AI 포함)과 통치자(`-Build=1`)가 역병에 `project_relief`로 대응할 수 있다(`InfluencePerPopulation = 1.0`과 맞물려 영향력 부족 없이 150틱에 해소).
- **그 밖의 설계 감사 항목(B1~B7 배정 유지)**:
  - 상시(`Standing`) 청원 1칸 유예 버퍼(`DeferredStandingRow`)는 저장 필드 1개가 추가되므로 로드맵에 적힌 대로 **B4(청원 나머지 3장과 함께)**에서 헤더와 함께 다룬다.
  - 사회(`Society`) 연구의 안정도 회복 보너스는 **B2(칙령 5종과 함께)**, 주장(`Claim`) 예고 줄과 반란 임계 표시는 **B4/B6**에서 다룬다.

### 4.2 `arch_seeker` 이름 둘 고르기 (`STATUS.md` 다음 작업 2 — `Tools/data/DT_Civs.json`)

`A0` 텍스트 감사에서 개발자 결정으로 남겨 둔 두 가칭(`DECISIONS_LOG.md` 2026-10-19 기록):
1. `arch_seeker.NameBank[2]`: 지금 **`"첫 불의 의회"`**
   - 문제: 뽑히면 의회 목소리 본문(`"의회가 첫 불의 의회의 일손을…"`)에서 '의회'가 겹친다.
   - 권장 대안: **`"첫 불의 모임"`** (또는 `"새벽 불의 동맹"`). `NameBank`의 앞선 두 이름(`"갈라진 집"`, `"찾는 자들의 연맹"`)과 결이 맞고 '의회/회중' 화자 낱말과 겹치지 않는다.
2. `arch_seeker.HomeSystemNames[0]`: 지금 **`"첫 해"`**
   - 문제: 조사와 붙을 때 `"첫 해에 역병이 돌기 시작했다"`가 시간 명사 '첫해(first year)'로 읽힌다.
   - 권장 대안: **`"이른 해"`** (또는 `"밝은 해"`). 고유한 모성계 이름으로만 읽히고 받침 유무에 따른 조사(`이른 해의`, `이른 해에서`, `이른 해가`)도 자연스럽다.
- 같은 위치의 문자열만 바꾸므로(위치 키 기반 번역/테이블) **시뮬레이션 추첨과 세계 해시는 바뀌지 않는다**.

---

## 5. 테스트 (`Sim.EconomyRules` 외 — 새 단언은 변이로 한 번씩 깨뜨린다)

| 테스트 | 단언 |
|---|---|
| `Sim.EconomyRules` (새 — `Tests/CosmicSimTests.cpp`) | ① 기본 `DT_Rules` 행이 기존 C++ 상수와 같은 값(8개)을 가진다 ② `TierMultiplier`를 `[2, 3, 4, 5, 6]`으로 바꾸면 칸별 에너지·물질·통찰·영향력 수입이 정확히 그 배수를 따른다(범위 밖·음수·NaN은 1.0, 0.0은 0.0) ③ `MaterialistEnergyBonus`·`SpiritualistInsightBonus`·`OpenInfluenceBonus`를 바꾼 행에서 윤리 양극의 보너스가 노브 값을 따르고 음수·NaN은 0이다 ④ `InsightPerPopulation`·`InfluencePerPopulation`을 바꾼 행에서 인구 비례 수입이 노브 값을 따른다 ⑤ `ResearchInsightShare`(0·0.25·1.0·범위 밖 클램프)와 `ResearchPerInsight`(배수 및 0·음수·NaN 방어)가 `TickResearch`의 통찰 차감과 연구 진행에 정확히 반영된다 |
| `Sim.PetitionPresent` (갱신 — `Tests/CosmicPetitionTests.cpp`) | `LastChoiceRows = { LotA, LotB }`일 때 두 `Lottery` 행이 모두 3칸 창 안에 있어도 직전 `Lottery`인 `LotB`만 피하고 `LotA`를 다시 내놓는다. 테이블에 적격 `Lottery` 행이 하나뿐(`LotA`)이고 `LastChoiceRows`에 있으면 아무것도 내놓지 않는다 |
| `test_data_schema.py` (넓힘) | `DT_Rules`의 새 노브 8개에 대한 타입·길이(`TierMultiplier` 길이 5)·범위(`min: 0`, `ResearchInsightShare` `max: 1`) 음수 케이스 |
| 기준값 및 기존 테스트 확인 | **A-8a**: `run_checks.ps1 -Tier full -Data`가 `-UpdateBaseline` 없이 그대로 통과(해시 불변). **A-8b**: `Sim.PhaseTwoExit`·`Sim.PetitionSoak`·`Sim.WorldMend` 등 전체 통과 확인 후 `run_checks.ps1 -Tier full -UpdateBaseline`으로 `Tools/baselines/seed42.json` 갱신 |

---

## 6. 문서 (A-8c)

- `docs/20_ARCHITECTURE.md`: 경제·연구 단계 설명에 8개 노브가 `DT_Rules`에서 읽힌다는 점 반영.
- `docs/21_DATA_MODEL_REFERENCE.md`: `FCosmicRulesDef`의 새 노브 8개와 기본값(`InfluencePerPopulation = 1.0f`), `CosmicPetitions::Present`의 주기 슬롯 폴백 규칙, `project_settle.RequiredProgress = 600.0`, `crisis_plague.RulerResolutions = ["project_relief"]` 반영.
- `docs/30_CONFIG_REFERENCE.md`: `python Tools/make_config_reference.py`로 재생성(`--check` 통과).
- `docs/40_DEV_GUIDE.md`: §1.1('경제 상수 C++ 하드코딩' 줄을 `StarClassEnergy`만 단계 5까지 C++에 남고 8개는 `DT_Rules`로 이전됨으로 갱신), §9.3(30판 측정치 갱신).
- `docs/50_ROADMAP.md`: A ⑧ 완료 표시 및 종료 조건 ⓑ 달성 수치 기록.
- `docs/10_DESIGN/systems.md` · `docs/10_DESIGN/crisis_and_transcendents.md` · `docs/10_DESIGN/DESIGN_AUDIT_AND_IMPROVEMENT.md`: 역병 대응 사업 연결(1-C) 및 주기 청원 폴백 반영.
- `docs/60_DECISIONS/DECISIONS_LOG.md` · `STATUS.md` · `CHANGELOG.md` · `CLAUDE.md` 갱신(`DECISIONS_LOG.md`의 커밋되지 않은 바깥 수정에 대한 결정도 함께 반영).

---

## 7. 순서와 검증

1. **A-8a (노브 8개 이전, 해시 불변)**:
   - `CosmicDefinitionRows.h`, `GalaxySimulationSubsystem.cpp`, `DT_Rules.json`(`InfluencePerPopulation: 0.5`), `schema.json`, `test_data_schema.py`, `CosmicSimTests.cpp`(`Sim.EconomyRules`).
   - `Commandlet -Import` → `run_checks.ps1 -Tier full -Data` (`-UpdateBaseline` 없이 시드 42 해시 그대로 통과).
2. **A-8b (밸런스·속도·주기 청원 폴백·역병 대응·이름 둘)**:
   - `InfluencePerPopulation` 0.5 → 1.0, `project_settle.RequiredProgress` 300 → 600, `CosmicPetitions::Present` 폴백 + `Sim.PetitionPresent`, `crisis_plague.RulerResolutions: ["project_relief"]`, `DT_Civs.json` 이름 둘.
   - `Commandlet -Import` → `CosmicBalance` 30판 측정(`-Focus=0`, `-Focus=1`, 입력 없는 20,000틱) → 이 헤더 §1에 실측치 기록 → `run_checks.ps1 -Tier full -Data -UpdateBaseline`.
3. **A-8c (문서)**:
   - `make_config_reference.py` 및 에디터 없는 점검 5종(`check_data.py`, `test_data_schema.py`, `check_layers.py`, `make_config_reference.py --check`, `check_docs.py`) + `run_checks.ps1 -Tier quick` 확인.

---

## 8. 개발자 결정 (권장안)

1. **경제·연구 노브 8개 (`FCosmicRulesDef` 끝, §2)**: `TierMultiplier` · `MaterialistEnergyBonus` · `InsightPerPopulation` · `SpiritualistInsightBonus` · `InfluencePerPopulation` · `OpenInfluenceBonus` · `ResearchInsightShare` · `ResearchPerInsight` 8개를 `DT_Rules`로 옮기고 `StarClassEnergy`는 단계 5(B8)까지 C++에 둔다. **A-8a**에서 기존 값 그대로 옮겨 해시 불변을 먼저 증명한다.
2. **`InfluencePerPopulation` (§2)**: **A-8b**에서 `0.5 → 1.0`으로 올린다(2026-10-18 결정 10). 나머지 7개 노브는 지금 값을 유지한다.
3. **주기 청원 비둘기집 교착 폴백 (§3.1)**: `CosmicPetitions::Present`에서 `LastChoiceRows`(3칸)를 모두 뺀 `Lottery` 후보가 비면, **`LastChoiceRows` 안의 가장 최근 `Lottery` 행 하나만** 빼서 2장 풀(`petition_road`, `petition_feast`)에서도 직전 주기 카드와 겹치지 않고 틱 900 뒤 매 주기 슬롯마다 번갈아 나오게 한다.
   - *대안*: 코드는 그대로 두고 `DT_Encounters.json`에 새 주기(`Lottery`) 카드 2장을 추가해 `Lottery` 풀을 4장으로 늘린다(단, 수직 슬라이스 10장 구성이 12장으로 늘어남).
4. **판의 속도와 정착 진척 요구량 (§3.2)**: `DT_Megaprojects.json`의 `project_settle.RequiredProgress`를 **`300.0 → 600.0`**(`project_dyson`과 동일)으로 올려, `-Focus=0`의 T3 시각(약 4,425틱)은 유지하면서 `-Focus=1`의 T3 시각을 1,958틱 → 약 2,560틱으로 늦추고 중점 전환을 5회로 늘려 목표 ⓑ(`decisions by end ≥ 15`)를 달성한다.
5. **성간 역병(`crisis_plague`) 대응 사업 연결 (§4.1)**: `DT_Hazards.json`의 `crisis_plague.RulerResolutions`를 `[] → ["project_relief"]`로 바꾼다(`DESIGN_AUDIT_AND_IMPROVEMENT.md` 자료 4 1-C).
6. **`arch_seeker`의 셋째 백성 이름 (§4.2, `STATUS.md` 다음 작업 2)**: `DT_Civs.json` `arch_seeker.NameBank[2]`의 `"첫 불의 의회"`(의회 화자 목소리와 중복)를 **`"첫 불의 모임"`**으로 바꾼다.
7. **`arch_seeker`의 모성계 이름 (§4.2, `STATUS.md` 다음 작업 2)**: `DT_Civs.json` `arch_seeker.HomeSystemNames[0]`의 `"첫 해"`('첫해'와 동음이의)를 **`"이른 해"`**로 바꾼다.
8. **`docs/60_DECISIONS/DECISIONS_LOG.md`의 커밋되지 않은 바깥 수정 (`STATUS.md` 다음 작업 2)**: 워킹 트리에 남아 있던 바깥 수정(빈 첫 줄 및 지워진 2026-10-19 A0 항목)은 **`git checkout -- docs/60_DECISIONS/DECISIONS_LOG.md`로 되돌려 2026-10-19 A0 기록을 보존**한 뒤 A-8 기록을 덧붙인다.
