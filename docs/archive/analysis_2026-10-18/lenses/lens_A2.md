## A2 — 설정값·밸런스 규칙 분석 (시뮬레이션 시스템 설계자 관점)

### 0. 전제 바로잡기
"기본/상세 설정값이 확립되지 않았다"는 절반만 맞다. **값은 있다.** `Tools/data/DT_Rules.json` 단일 행에 노브 44개, 정의 테이블 5종의 행 컬럼(위기 5·프로젝트 5·업적 2·초월자 1·아키타입 2), 코드 상수 약 20개(`Sim/GalaxySimulationSubsystem.cpp:33-106`), 그리고 `schema.json`과 테스트 76개가 그 값을 지킨다. 실제 문제는 셋이다. ① 44노브 중 **18개를 읽는 런타임 코드가 없다**(계승 6·함대 4·귀환 3·도약 3·ImmutableLaws·Vigil — grep 결과 Tests와 행 구조체 헤더 밖 참조 0, 도약 3개는 `Core/GalaxyState.h:545` 주석뿐). ② 플레이어 체감을 정하는 값(첫 10분 타임라인, 결정 빈도, T1→T3 30분)은 노브에서 도출되지 않고 산문(guideline:282, 340, 3958)에만 있다. ③ 같은 값이 guideline·ini DevComment·행 구조체 주석·JSON 네 곳에 적혀 이미 어긋났다(§5 표).

### 1. (a) 확정 설정값 통합 목록
분류 기호: **체** 체감(페이싱·난이도) / **밸** 밸런스 / **기** 기술(결정론·성능·구조) / **예** 예약(읽는 코드 없음).

| 영역 | 이름 | 값 | 읽는 곳 | 분류 |
|---|---|---|---|---|
| 시간 | SecondsPerTick / MaxTicksPerFrame / 배속 | 1.0초 / 10 / {0,1,2,5,10} | 코드 `Sim/…Subsystem.h:491,494`, `UI/CosmicTopBarWidget.cpp:29` | 체·기 |
| 시간 | 판 길이·도약 상한 | 18,000틱 / 4,500 | 문서 guideline:252만 / DT_Rules(미읽힘) | 체·예 |
| 경제 | TierMultiplier / StarClassEnergy(O→M) | {1,1.5,2.2,3,3} / {2,1.7,1.4,1.2,1,0.8,0.6} | 코드 cpp:86-90 | 밸 |
| 경제 | Insight·Influence per Population / 윤리 보너스 | 1.0·0.5 / +0.2 | 코드 cpp:93-97 | 밸 |
| 연구 | 지출 비율 / 진척률 / 돌파 문턱 | 수입×0.5 / 0.003 / 0.25·0.5·0.75·1.0 | 코드 cpp:98-99, 42 | 밸 |
| 이벤트 | EventSlotTicks / EventChancePerSlot | 10 / 0.02 | DT_Rules | 체 |
| 위기 | CountdownTicks | AI각성 150·역병 200·초신성 300·반란 200·주장 300 | 행 컬럼 DT_Hazards | 체 |
| 위기 | SpawnChancePerTick | 0.0002 / 0.0001 / 5e-5(T2+) | 행 컬럼 DT_Hazards | 밸 |
| 위기 | 반란 조건 / 수도 상실 | Stability<0.3 100틱 ∧ tick≥300 / −0.3 | 행 컬럼 / 코드 `CosmicHazards.cpp:18` | 밸 |
| 프로젝트 | CostPerTick × RequiredProgress | 다이슨 E2M1×600·개혁 Inf1×100·구호 M1Inf1×150·정착 E1M2×300(T2)·안정화 E2M2×100(T2) | 행 컬럼 DT_Megaprojects | 밸 |
| 승천 | 업적 조건 | T1→T2 다이슨+물리0.5 / T2→T3 안정화 | 행 컬럼 DT_Feats | 체 |
| 영압 | PressureBySourceTier / Falloff / Drain | {0,0,0.25,1,2} / 0.5 / 0.002 | DT_Rules | 밸 |
| 영혼경제 | ClaimAdjacencyLanes / Depth / MaxInfluenced | 1 / 1.0 / 12 | DT_Rules / 행 컬럼 trans_silence | 밸 |
| 정착 | RealmCapByTier / 영향권 해제 칸 | {1,3,4,7,10} / T3 | DT_Rules / 코드 `CosmicMegaprojects.h:98` | 밸 |
| 앎 | SightLanesByTier / Interval / FirstSightTick | {1,2,4,6,10} / 10 / 330 | DT_Rules | 체 |
| 통치 | ThroneLockTicks / PostRebellionStability / ReverenceDecay | 600 / 0.15 / 0.5 | DT_Rules(ThroneLock은 쓰기만) | 체·밸 |
| 인구 | Regrowth / BandThresholds / 초기 Stability | 0.0002 / {0.15,0.5,2.0} / 0.75 | DT_Rules / 코드 `GalaxyState.h:268` 초기값 | 밸 |
| 로스터 | FallenTranscendentCount / CoreMinLanes | JSON 1 (C++ 기본 3) / 3 | DT_Rules | 밸 |
| 규모 | LivingWorlds / Peoples / 상한 로스터 | 60 / 12 / 3 | DT_Galaxies 고향 행 / 코드 cpp:106 | 체·기 |
| 계승·함대·귀환 | Leap 3·Succession 6·Fleet 4·ReturnLure 2·LureAccept·Vigil·ImmutableLaws | 800~1600 등 | DT_Rules **미읽힘 18개** | 예 |
| 표시 | TimeUnit(주기/년) / 렌더 cvar | Cycle / FXAA 등 | ini GameUserSettings / DefaultEngine.ini | 기 |

**DT_Rules 44노브 분류 요약**: 체감 11(슬롯·시야·출생보호·왕좌잠금·AI 배율·첫 생애 행·노화 0) / 밸런스 14(영압 3·보유상한·인구 3·주장 2·로스터 2·폐위 2·항성 분포) / 구조 1(HomeGalaxyRow) / 예약 18. 즉 **설계자가 지금 만질 수 있는 실효 노브는 26개**이고, 그중 플레이어 '결정'에 닿는 것은 없다(결정은 거대 프로젝트 시작 하나 — `Player/PlayerRoleSubsystem.h:71`).

### 2. (b) 비어 있거나 문서에만 있는 규칙과 기준안
현행 수치를 **그대로 계산**해 보면 뼈대 페이싱은 이미 서 있다: 인구 1.0·중점 0.25에서 물리 0.5까지 1,333틱, 다이슨은 G형 T1 에너지 수입 1.0/틱에 비용 2/틱이라 1,200틱 → **T2 ≈ 1,333틱(22분)**. 문제는 그 사이와 그 뒤다.

| 항목 | 현황 | 기준안(수치·근거) |
|---|---|---|
| 자원 5종 수입 | 코드 상수. Influence 0.5/틱이 병목(구호 Inf1×150틱 ↔ 주장 300틱, 개혁 Inf1×100 ↔ 반란 200틱 — 비축 0이면 정확히 카운트다운 끝에 닿는다; Phase 2 실측 '편입 57건 주인 Influence 평균 0.5'와 일치) | `InfluencePerPopulation` 1.0으로 상향(또는 구호 RequiredProgress 100). 상수 9개를 DT_Rules 행으로 이동. 초기 비축 E60/M30/Inf20(첫 10분 Dyson 즉시 착수). Causality: T3 ∧ Transcendence≥0.5에서 0.05×TierMultiplier/틱(문서 §5.1:996 '극소량' 구체화). §5.1:995 '조약 수' 항은 Phase 4까지 문서에서 삭제 |
| 연구 돌파 효과 | 문턱마다 로그 줄뿐(cpp:1560-1579), 중점 명령 없음 | `SetResearchFocus` 명령(4값) 추가. 효과표: 물리 1.0 안정화 비용 ×0.75 / 사회 0.25·0.5·0.75 칙령 슬롯 1·2·3 / 공학 문턱마다 RequiredProgress ×0.9 / 초월 0.5 Causality 해금 |
| 외교 태세·조약 | 빈 함수 | Phase 4: Relations 표류 ±0.001/틱(1레인 접촉, 윤리 거리 기준), 무역 Matter +10% 쌍방, 불가침 분쟁 금지, 동맹은 EA |
| 분쟁 추상화 | 빈 함수, 노브 4개 미사용 | Phase 4: 적대 접촉 성계에서 틱당 소유 변동 P = 0.002×clamp((A−D)/(A+D),0,1), FleetPower 성장 = Energy 지출 0.1/틱, 격차 승수·지연 노브는 현행값 유지 |
| 위기 빈도·첫 10분 | §2.4:340 '2:30 첫 Hazard(AI각성, 모성계)'. 실제 0.0002×1.2=0.00024/틱 → 150틱 안 발생 확률 ≈3.5%, 평균 대기 ≈4,170틱 | **디렉터 노브** 신설: `FirstHazardTick 150`·`FirstHazardRow crisis_ai_awakening`(SpawnRule `Placed` 추가, §7.3 예약). 기본 확률 AI각성 0.0005·역병 0.0002·초신성 1e-4(T2+, §16 #72 기본값). 적응형: 직전 600틱 결정 수 < 10이면 Draw 몫 ×1.5(상한 ×3) |
| 승천 Tier별 | T1→T2·T2→T3 2행. T2→T3는 초신성이 저절로 서야 열려 성계 3개 평균 6,700틱(#72) | T2→T3 둘째 행 `feat_three_worlds`: 정착된 성계 3 ∧ 공학 0.5(결정론 경로). T3→T4: 방주 프로젝트 ∧ 초월 0.75(코어 완성 단계). T4→T5: 법 3개 1,800틱 유지 ∧ PersonalTier≥3(폴리싱) |
| 계승·공백기 | 노브 9개 예약, 파이프라인 없음 | 값 유지(800~1600, 상한 4,500 = 25%). MVP는 폐위→방랑자만; 사망·계승은 코어 완성 단계 |
| 안정도 회복 | Sim에 상승 규칙 없음(cpp:1495-1498 소모만). 실측 백성 59%가 0.3 미만, 반란이 위기의 62%, 반란→개혁→반란 200틱 고리 | #68 기본값 구현: 수도에 위기 없을 때 0.6을 향해 +0.001/틱, `crisis_rebellion.OnResolveEffects` ChangeStability +0.1. 목표 반란 몫 25~40% |
| 영압 | 공식·노브 구현됨 | 유지. 회복 규칙과 함께 재측정(P=2.25 영향권 안 순손실 −0.0035/틱) |
| 시간·배속·판 길이 | ×25 설계(guideline:251)인데 `SetTimeScale`이 10으로 clamp(cpp:1036) | 배속 상한을 `MaxTicksPerFrame`에서 분리(25), 프레임 예산 10ms 검사. MVP 목표: 혼합 배속으로 T1→T3 45~60분 |
| 은하 규모 | 60·12 데이터, EA 300·48 | VS 유지. EA는 Phase 5 실측 뒤 |
| AI 행동 | AIProfile 저장만, 읽는 코드 0(`CosmicStateIndex.cpp:102-111` 추첨만) | #71 기본값: Conservative·Scholarly 구호, Expansionist·Militarist 방치; Scholarly 중점 초월 0.4; 정착 대상은 풍부도 최대(균등 추첨 대신) |

### 3. (c) 설정 관리 방식 제안
- **DT_Rules(단일 행)**: 리빌드 없이 밸런스 패스가 바꾸는 값 — 경제·연구 상수 9개를 여기로 옮긴다. 노브 이름은 `<대상><양><단위>`(`ClaimCountdownTicks`, `…ByTier`는 길이 5 고정).
- **행 컬럼(DT_*)**: 콘텐츠 단위의 값(카운트다운·비용·확률). 교차 테이블 검사(`HazardSpawnMultipliers` 키, `CostPerTick ≥ 0`)를 스키마에 추가.
- **코드 상수**: 세이브·해시·테스트가 '법'으로 단언하는 값만(틱 길이, 로스터 상한 3, LowStabilityThreshold 0.3, 레인 4).
- **ini**: 보는 사람의 설정(TimeUnit)과 엔진 cvar만. DevComment에 수치를 적지 않는다(현재 Cult 350·Erosion 400·Extinction 250·Visitation 120이 ini 주석에만 산다).
- **Config Reference 문서 1개**: `이름 | VS값 | EA값 | 단위 | 읽는 곳(파일:함수) | 분류 | 상태(읽힘/예약 Phase N) | 출처 §`. 예약 노브는 schema에 `reader: pending` 표지를 두고, Python 검사기가 `Source/` grep으로 '읽힘' 표지와 대조한다.

### 4. (d) 밸런스 검증 수단
현황: `CosmicBalance` 커맨드렛은 인자 6개(Seed/Ticks/Systems/Civs/TellAll/Out, `CosmicBalanceCommandlet.cpp:227-232`)와 CSV 23컬럼(문명 12 + 판 11), 요약 4줄을 낸다. §9.3이 요구하는 `-PlayerPolicy`·`-DeathAt`·`-Fallen`·`-PlayerArchetype`와 Phase 5 컬럼 34개는 없다. 목표 대비 실측(guideline:3682-3692): 반란 몫 62%(목표 25~40), 주장 정착률 18%(40~60), 첫 600틱 말해진 줄 9(25~40), 입력 없는 폐위 13%(10~15 ✓).
**MVP 최소 지표 세트(8개)**: ① 플레이어 T2·T3 도달 틱 ② 첫 600틱 결정 수 ③ 첫 Hazard 틱(자기 성계) ④ Stability<0.3 백성 비율 ⑤ 반란 몫 ⑥ 주장 정착률 ⑦ 첫 600틱 말해진 줄 ⑧ 폐위된 판 비율. 전제는 `-PlayerPolicy=AI`(플레이어 백성을 AI 규칙으로 돌림 — `bIsPlayerRuled` 해제 한 줄)이고, 시드 1~30 요약 줄을 골든 파일과 diff하는 스크립트 하나면 회귀가 잡힌다. 테스터 질문 21개는 Phase 5 그대로.

### 5. (e) 수치 불일치 표
| 항목 | A | B | 비고 |
|---|---|---|---|
| ×25 배속 | guideline:251 T3부터 ×25 | `SetTimeScale` clamp 10(cpp:1036, h:494) | UI만 고치면 조용히 10으로 깎임 |
| 첫 Hazard 2:30 | guideline:340 | 발생 확률로 3.5% | Placed 규칙 없음 |
| VS 수량 | §12.1: 아키타입 7·Hazard 8·이벤트 40·조우+청원 35·업적 4·프로젝트 7·칙령 9·유물 8 | 데이터: 2·5·11·0·2·5·0·0 | 표에 '목표/현재' 열 필요 |
| AI각성 만료 | §4.1:625·ini '생산+150%/성계 반란' | DT_Hazards +30/+30 또는 −0.15 | 셋 중 하나로 통일 |
| Influence 공식 | §5.1:995 '×조약 수' | cpp:1431 조약 항 없음 | Phase 4까지 문서 수정 |
| 타락 로스터 | C++ 기본 3(`CosmicDefinitionRows.h:763`) | JSON 1 | 테이블 부재 시 세계가 달라짐 |
| 첫 줄·표시명 | guideline '맡겨졌다'·'항성 안정화' | DT_LogTemplates '통치자다'·DT_Megaprojects '초신성 안정화' | v0.8 단계 4b·5 표지 |
| T1→T3 시간 | §2.4 T1 30분+T2 60분 / Phase 4 종료 '30분' | #72 실측 T3 중앙값 6,400틱(107분@1x) | 배속 미기재 |
| 스키마 상한 | §7.3 RevelationTemplates '정확히 3' | schema len_in [1,3], 데이터 1줄 | 헤더 주석 낡음 |
| 노브 수 | digest·§7.3 '46' | JSON 44 | 본 보고 기준 44 |

**결론**: 설정값은 '없는' 것이 아니라 **소유권과 소비자가 불명확**하다. MVP 전에 할 일은 노브 추가가 아니라 (1) 읽히지 않는 18개에 표지를 달고, (2) 경제·연구 상수를 데이터로 올리며, (3) 안정도 회복·첫 Hazard 디렉터·연구 중점 명령 세 가지로 '결정 빈도'를 만든 뒤, (4) 커맨드렛 8지표로 그것을 재는 것이다.

## 권장안
- [P0] **Config Reference 문서 1개 신설 + '읽힘/예약' 검사** — DT_Rules 44노브·행 컬럼·코드 상수·ini를 한 표(이름|VS값|EA값|단위|읽는 곳 파일:함수|분류|상태|출처 §)로 모은다. schema.json에 노브별 `reader: pending|<file>` 메타를 두고 Python 검사기가 Source/ grep으로 대조해, 현재 읽히지 않는 18개(Succession 6·Fleet 4·Leap 3·ReturnLure 2·LureAccept·Vigil·ImmutableLaws)를 '예약 Phase N'으로 표시한다. ini DevComment의 수치(Cult 350 등)는 이 표로 옮기고 주석에서 뺀다.
- [P0] **경제·연구 상수 9개를 DT_Rules로 이동하고 Influence 병목 해소** — GalaxySimulationSubsystem.cpp:86-99의 TierMultiplier·StarClassEnergy·InsightPerPopulation·InfluencePerPopulation·보너스 3·ResearchInsightShare·ResearchPerInsight를 DT_Rules 컬럼으로 올린다(Data.Integrity·schema 갱신 포함). 동시에 InfluencePerPopulation을 0.5→1.0으로 올리거나 project_relief RequiredProgress를 150→100으로 내려, 비축 0일 때 구호·개혁이 카운트다운(300·200틱) 끝에 겨우 닿는 구조를 푼다. 커맨드렛으로 주장 정착률 40~60% 재확인.
- [P0] **안정도 회복 규칙(§16 #68 기본값) 구현** — 경제 단계에 '수도에 위기가 없을 때 Stability를 0.6을 향해 +0.001/틱' 노브(StabilityRecoveryPerTick, StabilityRestTarget)를 추가하고 crisis_rebellion.OnResolveEffects에 ChangeStability +0.1을 넣는다. 반란→개혁→반란 200틱 고리와 '백성 59%가 0.3 미만'을 끊는 것이 목적. #68만 넣은 판·#71만 넣은 판을 각각 재어 반란 몫 25~40%를 확인한다.
- [P0] **첫 10분 디렉터: Placed 발생 규칙 + 자동 일시정지** — ECosmicHazardSpawn::Placed를 추가하고 DT_Rules에 FirstHazardTick(150)·FirstHazardRow(crisis_ai_awakening)를 두어 guideline §2.4의 '2:30 첫 Hazard, 모성계'를 발생 확률(현재 150틱 안 3.5%)이 아니라 규칙으로 보장한다. Severity 2 줄과 자기 성계의 위기 발생에 자동 일시정지 옵션(기본 ON, 보는 사람 설정)을 붙여 '로그 누적→정지→결정' 코어 루프를 실제로 돌린다.
- [P0] **연구 중점 명령과 돌파 효과 표** — UPlayerRoleSubsystem::SetResearchFocus(4값, 합 1) 명령과 상단 바 슬라이더를 더해 통치자의 둘째 결정 축을 연다(현재 ResearchFocus 0.25 고정). 돌파 효과를 데이터로: 물리 1.0 → 안정화 비용 ×0.75, 사회 0.25/0.5/0.75 → 칙령 슬롯 1/2/3, 공학 문턱마다 RequiredProgress ×0.9, 초월 0.5 → T3에서 Causality 0.05×TierMultiplier/틱 해금.
- [P0] **배속 상한을 프레임 상한에서 분리하고 ×25 구현** — SetTimeScale의 clamp가 MaxTicksPerFrame(10)이라 설계된 셋째 칸 ×25가 조용히 10으로 깎인다. SpeedMax(25)와 MaxTicksPerFrame(25, 틱 30µs라 10ms 예산 안)을 나누고 Sim.FrameStep 단언·상단 바 Speeds·키 핸들러 세 곳을 함께 고친다. MVP 목표 플레이 시간(혼합 배속 T1→T3 45~60분)을 Phase 4 종료 조건에 배속과 함께 적는다.
- [P1] **T2→T3 둘째 업적과 초신성 전조 확률 상향(§16 #72)** — feat_three_worlds(정착된 성계 3 ∧ 공학 0.5 → T3)를 추가해 저절로 서는 위기에 매이지 않는 결정론 경로를 둔다. 동시에 crisis_pre_supernova의 SpawnChancePerTick을 T2+ 백성에게 5e-5→1e-4로 올려 평균 대기 6,700→3,300틱(성계 3개 기준). AI T3 중앙값이 6,400틱에서 3,500틱 안팎으로 내려오는지 커맨드렛으로 확인.
- [P1] **AI 프로필을 실제로 읽는 행동 규칙(§16 #71)** — AIProfile은 생성·반란·폐위에서 추첨되지만 읽는 코드가 없다. CosmicMegaprojects::Choices에서 Conservative·Scholarly는 구호·개혁을 시작하고 Expansionist·Militarist는 주장을 방치(추첨 지점 추가 없음), Scholarly는 ResearchFocus 초월 0.4, Expansionist는 정착 대상을 균등 추첨 대신 풍부도 최대로 고른다. 주장 정착률 목표 40~60%의 주 레버.
- [P1] **밸런스 커맨드렛 MVP 지표 8개와 골든 요약 diff** — -PlayerPolicy=AI(플레이어 백성을 AI 규칙으로 돌리는 한 줄)를 먼저 넣고 CSV에 PlayerT2Tick·PlayerT3Tick·DecisionsPer600·FirstOwnHazardTick·StabilityBelow03Share·RevoltShare·ClaimSettleRate·DepositionShare 8컬럼을 더한다. 시드 1~30의 summary 줄을 Saved/Balance/golden.txt와 비교하는 Python 스크립트를 Tools/에 두어 밸런스 회귀를 빌드 없이 잡는다. Phase 5의 34컬럼·-DeathAt는 그 뒤.
- [P1] **칙령 최소 3종(DT_Edicts)과 유지비로 '첫 60분 폐위율' 측정 가능하게** — DT_Edicts 행 구조체·JSON·스키마를 만들고 철야(edict_vigil: 수입 −20%, Stability +0.1, 주장 +100틱)·안식(수입 ×6/7)·기계 권리(AI각성 해결, Energy 수입 −10%) 3행만 넣는다. 칙령 유지비가 있어야 guideline §9.3의 '방치 시 폐위 100%, 플레이 시 20~35%' 목표가 재어진다. 슬롯 수는 사회 돌파와 연결.
- [P1] **연대기 이관 기준 축소로 세이브 1MB 예산 복귀** — Log 2,000줄 초과 시 Severity≥1 전부를 Chronicle로 옮기는 현행 규칙(CosmicLogSubsystem.cpp:205)을 §16 #40 ⑦ 기본값(Severity 2·Log.Claim·Log.Record·통치 백성 관련 Severity 1만)으로 좁히고 Chronicle 상한(예: 1,500)을 DT_Rules 노브로 둔다. 전부 쌓는 모드 VS 1.8MB·EA 4.2MB가 예산 안으로 들어오는지 커맨드렛 SaveBytes로 확인.
- [P2] **외교·분쟁 기준 공식을 데이터 행으로 먼저 적기** — TickDiplomacy·TickConflict는 빈 함수다. 구현 전에 수식을 확정해 DT_Rules/행 컬럼으로 적는다: 관계 표류 ±0.001/틱(1레인 접촉, 윤리 거리), 조약 3종 효과(무역 Matter +10% 쌍방·불가침 분쟁 금지·동맹 EA), 분쟁 P(소유 변동)=0.002×clamp((A−D)/(A+D),0,1), FleetPower 성장=Energy 지출 0.1/틱. 격차 승수·지연 노브 4개는 현행값 유지.
- [P2] **사망·계승 파이프라인에서 예약 노브 9개 소비** — LeapTicksMin/Max/Cap·Succession 6노브는 값을 유지하되 코어 시뮬 완성 단계에서 KillPlayer 해소→FLifeRecord→RunHeadless(Leap)→계승 추첨 순으로 읽게 한다. 그 전까지 MVP의 실패 상태는 폐위→방랑자뿐임을 로드맵에 명시하고, ThroneLockTicks를 읽는 BecomeRuler(즉위) 명령을 먼저 넣어 왕좌 잠금 600틱이 효과를 갖게 한다.
- [P2] **드리프트 정리: C++ 기본값·문서 표·ini 주석 통일** — FallenTranscendentCount C++ 기본값 3을 JSON과 같은 1로(또는 '폴백=EA값' 명시), §4.1 표의 AI각성 결과를 데이터(+30/+30 또는 −0.15)로, §5.1 Influence '조약 수' 항에 Phase 4 표지, CosmicDefinitionRows.h의 RevelationTemplates '정확히 3' 주석을 스키마 [1,3]에 맞춤, '46노브' 표기를 44로. 각 수치는 Config Reference 한 곳만 원본으로 두고 나머지는 참조.

## claims
- A2-1 [factual/high] DT_Rules.json의 노브는 44개이며(‘46’이 아님), 그중 18개(LeapTicksMin/Max/TotalCap, Succession* 6, PressureFleetAttrition, FleetEffectivenessByTierGap, FleetDelayPerPower, FleetDelayCap, ImmutableLaws, VigilClaimExtension, ReturnLureMaxLanes, ReturnedLivesToSeize, LureAcceptByPolicy)는 Tests/와 행 구조체 헤더를 제외한 런타임 코드에서 참조가 없다(Leap 3개는 Core/GalaxyState.h:545 주석뿐).  (근거: Tools/data/DT_Rules.json (키 수 세기); grep -rl 각 노브명 Source/Cosmic_Ascension --include=*.cpp,*.h | grep -v Tests/ -v CosmicDefinitionRows.h; Source/Cosmic_Ascension/Core/GalaxyState.h:545)
- A2-2 [factual/medium] 경제·연구 밸런스 상수 9개(TierMultiplier, StarClassEnergy, MaterialistEnergyBonus, InsightPerPopulation, SpiritualistInsightBonus, InfluencePerPopulation, OpenInfluenceBonus, ResearchInsightShare, ResearchPerInsight)는 DT_Rules가 아니라 cpp 상수이고, guideline §5.1의 Influence 공식 ‘인구 × 개방 축 × 조약 수’와 달리 코드에는 조약 항이 없다.  (근거: Source/Cosmic_Ascension/Sim/GalaxySimulationSubsystem.cpp:83-100 (namespace Economy), :1431 (Income.Influence); guideline.md:995)
- A2-3 [factual/high] guideline §2.4는 첫 10분 타임라인에서 2:30(틱 150)에 첫 Hazard(AIAwakening)가 모성계에 선다고 적지만, 데이터의 발생 규칙(Draw, SpawnChancePerTick 0.0002 × arch_terran 배율 1.2)으로는 150틱 안 발생 확률이 약 3.5%(평균 대기 약 4,170틱)이고 지정 시각에 놓는 SpawnRule(Placed)은 코드에 없다.  (근거: guideline.md:340; Tools/data/DT_Hazards.json (crisis_ai_awakening SpawnChancePerTick 0.0002); Tools/data/DT_Civs.json (arch_terran HazardSpawnMultipliers Hazard.AIAwakening 1.2); Source/Cosmic_Ascension/Sim/GalaxySimulationSubsystem.cpp:1656-1686; Source/Cosmic_Ascension/Data/CosmicDefinitionRows.h:106-119 (ECosmicHazardSpawn Draw/Capital/ClaimRule))
- A2-4 [factual/medium] guideline §2.1은 셋째 칸(T3)부터 ×25 배속을 설계하지만 UGalaxySimulationSubsystem::SetTimeScale은 MaxTicksPerFrame(10)으로 clamp하므로 UI에 ×25 단추를 더해도 10으로 깎인다.  (근거: guideline.md:251; Source/Cosmic_Ascension/Sim/GalaxySimulationSubsystem.cpp:1034-1043; Source/Cosmic_Ascension/Sim/GalaxySimulationSubsystem.h:494)
- A2-5 [factual/medium] guideline §12.1의 수직 슬라이스 수량(아키타입 7·Hazard 8·이벤트 40·조우 25+청원 10·업적 4·거대 프로젝트 7·칙령 9·유물 8)에 대해 현재 Tools/data는 아키타입 2·Hazard 5·이벤트 11·조우/청원 0(테이블 없음)·업적 2·프로젝트 5·칙령 0·유물 0이며, 표에는 ‘목표’와 ‘현재’의 구분이 없다.  (근거: guideline.md:4034-4062; Tools/data/DT_Civs.json, DT_Hazards.json, DT_Events.json, DT_Feats.json, DT_Megaprojects.json (행 수); Tools/data/ 디렉터리에 DT_Encounters·DT_Edicts·DT_Relics 없음)
- A2-6 [factual/high] 시뮬 코드에는 Stability를 올리는 규칙이 없고(경제 단계는 영압 소모만, 상승은 이벤트 효과 +0.03/+0.2뿐), 그 결과 Phase 2 실측에서 백성 59%가 Stability 0.3 미만이며 놓인 위기의 62%가 반란이다(목표 25~40%). §16 #68이 ‘지금은 그것을 만드는 규칙이 없다’고 인정한다.  (근거: Source/Cosmic_Ascension/Sim/GalaxySimulationSubsystem.cpp:1495-1498, :1526 (grep Stability — 감소뿐); Tools/data/DT_Events.json (event_civ_festival +0.03, event_book_found +0.2); guideline.md:3682, :4461)
- A2-7 [judgment/high] 현행 44노브로는 플레이어 체감 페이싱(첫 600틱 결정 15~20회, 20~40초마다 결정 1회, 첫 Hazard 2:30)을 만들 수 없다 — 지금 플레이어 명령은 StartMegaproject 하나이고 페이싱은 노브가 아니라 디렉터 규칙(Placed 위기·자동 일시정지·적응형 빈도)과 콘텐츠(청원·칙령·연구 중점)에 달려 있으므로, MVP 전에 ‘디렉터 노브’ 묶음(FirstHazardTick 등)과 명령 축 2개(연구 중점·칙령)를 먼저 두어야 한다.  (근거: guideline.md:282, :340; Source/Cosmic_Ascension/Player/PlayerRoleSubsystem.h:38-76 (명령은 Install/DeposeRuler/BecomeWanderer/StartMegaproject/StartNextMegaproject뿐); Source/Cosmic_Ascension/Sim/GalaxySimulationSubsystem.cpp:752 (ResearchFocus 0.25 고정))
- A2-8 [judgment/medium] 설정 관리의 핵심 문제는 값의 부재가 아니라 소유권 불명이다 — 같은 수치가 guideline 표·ini DevComment·행 구조체 헤더 주석·JSON 네 곳에 적혀 이미 어긋났으므로(AI각성 만료 결과, RevelationTemplates ‘정확히 3’ vs [1,3], FallenTranscendentCount C++ 3 vs JSON 1), ‘Config Reference 1표 + 읽힘/예약 검사’로 원본을 한 곳에 고정하는 것이 로드맵의 어떤 콘텐츠 작업보다 앞서야 한다.  (근거: guideline.md:625 vs Tools/data/DT_Hazards.json (crisis_ai_awakening); Source/Cosmic_Ascension/Data/CosmicDefinitionRows.h:441-444 vs Tools/data/schema.json (RevelationTemplates len_in); Source/Cosmic_Ascension/Data/CosmicDefinitionRows.h:763 vs Tools/data/DT_Rules.json (FallenTranscendentCount); Config/DefaultGameplayTags.ini:16-24 (행 없는 위기의 카운트다운이 주석에만))
