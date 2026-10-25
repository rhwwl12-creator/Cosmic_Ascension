# B3 헤더 — 외교와 분쟁 (관계의 표류·태세·조약 둘·함대·공성·첫 접촉 카드·격리령)

상태: **승인 (2026-10-25, 개발자의 상시 승인 — "다 승인할테니 B작업을 끝까지 이어서 진행해줘", B의 권장안은 승인으로 친다 — §10의 결정 22개 전원 권장안).** 승인 전의 기록: 초안 둘째 판 (2026-10-25) — 조사 워크플로(다섯 관점이 나란히: 설계 문서가 말하는 외교·분쟁·코드가 꽂을 자리·밸런스의 수와 목표·B2가 넘긴 것과 서사·화면·다른 게임의 유닛 없는 외교·분쟁 꼴, 그리고 빠진 것을 찾는 비평 하나 — 사실은 모두 파일의 줄로). 첫 판의 적대 리뷰(ultracode 워크플로 — 네 관점: 코드의 사실과 될 일·결정론과 상태와 데이터의 규칙·플레이어의 경험과 화면과 서사·밸런스와 측정, 관점마다 반박 검증 하나, 빠진 것을 찾는 비평 하나): 발견 72건 — 네 관점 51(확인 40·일부 10·반박 1)과 비평 21, 높음 11·중간 32·낮음 29 — 반박 하나를 빼고 모두 본문에 넣었다(일부는 검증이 고친 꼴로). 리뷰가 바꾼 설계: **#73의 쓰기는 필드만의 커밋으로**(완성은 데이터를 꺼도 일어난다), **첫 접촉 카드의 길은 카드 행과 한 커밋에**(코드가 이름으로 부르는 행은 데이터에 있어야 한다), **AI의 적대는 공성과 한 몸**(노릴 성계가 없으면 적대도 없다 — 끝나지 않는 적대와 목표 없는 적대·몰매를 막는다), **공성이 끝나면 쉼**(공격한 쪽의 휴전 — 새 필드 없이 목표의 틱으로), **함대의 상한은 정착된 성계의 수 × 칸의 곱**(칸만 보면 후반이 얼고 통치자가 늘 앞선다), **우호의 자리**(우호가 공짜인 지배 전략을 막는다), **첫 접촉 카드의 선택은 둘**(셋째의 효과를 아무 규칙도 읽지 않았다), **공성의 줄은 쪽마다 다른 행**(공격·방어를 한 틀로 쓸 수 없다), 사유의 차례(구조의 사유가 먼저), 끝 판정은 '이르렀거나 지났다', 소모는 상위 초월자만, 낱말 '거두다'를 쓰지 않는다(함대는 '물린다'), 측정은 짝 차이로·방랑자는 60판으로. 둘째 판의 확인(셋이 나란히 — 발견마다의 반영 둘과 새로 읽기 하나): 36건(반영의 빈자리 19와 새 결함 17)을 모두 넣었다 — `CosmicPetitions::Clear`는 첫 접촉의 대기 칸을 비우지 않는다(답마다 불린다), `Destroy`·`Transfer`가 그 성계를 노린 함대 목표를 푼다, 보는 눈 `ViewOf` 하나(국경 밖 쌍은 태세의 항을 뺀 목표값), 적대는 무역 조약을 끊는다, 섭정은 지킴을 물린다, 소모의 압력은 공개 함수 하나, 우호의 자리 {1, 1, 2, 2, 3}, 물려받은 전시 동원의 함대 곱을 짝으로 잰다, 정책 값은 정수 하나(0·1·2), 첫 접촉 카드의 길은 구현 리뷰를 받는다. 개발자 결정은 §10.

근거: `docs/50_ROADMAP.md:12`(B3 '외교·분쟁(함대 노브 4개 소비)'와 B의 종료 조건), `docs/10_DESIGN/systems.md` §5.5(`:52-58`)·§5.6(`:60-66`)·§5.8(B2가 넘긴 셋 — 격리령·전시 동원의 함대 효과·무역 자유화의 조약 효과), `docs/10_DESIGN/crisis_and_transcendents.md:22`(역병의 번짐)·`:98`·`:104`·`:106`(영압 ②와 무력 저항)·`:125`(AI가 하는 일), `docs/10_DESIGN/pillars.md:76`(T2의 무역 조약), `docs/00_VISION.md:118-119`·`:168`·`:175`(첫 10분의 첫 접촉), `docs/21_DATA_MODEL_REFERENCE.md:1347`·`:1458-1462`·`:1558`, `docs/20_ARCHITECTURE.md:198`·`:273`·`:282`·`:286-287`, `docs/70_CONTENT_GUIDE.md:43`·`:45`·`:61`·`:144`·`:166`, `OPEN_QUESTIONS` #24·#29 ⑧·#31 ②·#35·#42·#73·#107·#123·#124, `DECISIONS_LOG`(B2 2026-10-25·#53 ①·A2 결정 ①), 헤더 A2(첫 접촉 카드를 미룬 까닭)·B1(`:12`·결정 22)·B2(§0·§11), `CLAUDE.md` 불변 규칙 2·3·4·5·6·7·8·11·13·14. 조사와 리뷰의 원문은 세션 스크래치패드 `b3/research_*.md`·`b3/review_header.md`(저장소 밖) — 이 헤더가 그 사실을 옮겨 적는다.

## 0. 범위와 전제

- **관계**(백성이 다른 백성을 보는 눈, −1~+1 — `FCivilizationState.Relations`)는 방향이 있다: A가 B를 보는 값과 B가 A를 보는 값은 다른 항목이다(저장 그대로 — `Core/GalaxyState.h:302-304`). 화면이 먼저 보이는 값과 조약의 승낙이 읽는 값은 '그들이 우리를 보는 눈'이다.
- **태세**(한 백성이 다른 백성을 대하는 자세 — 우호·중립·적대)는 백성마다 상대마다 저장한다. 없는 항목 = 중립. **조약**(두 백성이 함께 맺는 약속 — 무역·불가침)은 두 백성에 같은 칸으로 저장하고 쓰는 함수가 하나다. **함대**는 유닛이 아니라 백성의 수 하나(`FleetPower`)와 **목표 성계** 하나다. 목표가 제 정착된 성계면 그 성계를 **지킨다**, 적대하는 이웃의 정착된 성계면 그 성계를 **에워싼다**(공성). 공성은 정해진 틱 뒤에 한 번 뽑아 끝난다. AI의 적대는 공성과 한 몸이다 — 노릴 성계가 있을 때만 서고, 공성이 끝나면 풀리고 한동안 쉰다.
- **B3에 드는 것**(§2·§3): 저장 필드 넷(관계의 짝 기록·함대 목표·목표의 틱·완성 기록의 주인), 외교 단계의 본문(관계의 표류, 우호의 유지비), 분쟁 단계의 본문(함대의 자람과 유지비, 영향권 안의 소모, 목표의 점검, 공성의 끝), 위기 단계의 두 고리(함대가 지키는 성계에 새로 서는 위기의 남은 시간, 역병의 번짐), AI 단계의 외교 규칙(적대와 공성, 쉼 — 추첨 없음), 명령 넷(`SetStance`·`ProposeTreaty`·`BreakTreaty`·`AllocateFleet`), 첫 접촉 카드 `petition_first_contact`(A2·B1이 #24 때문에 미룬 것), 효과 열거자 하나(`SetStance`), 격리령 `edict_quarantine`(여섯째 칙령), 전시 동원의 함대 효과와 무역 자유화의 조약 효과(B2가 넘긴 것), #73(업적은 제 백성의 완성만 센다), 함대 노브 넷의 소비, 로그 행 열하나, 칙령 창의 셋째 탭 [백성]과 패널의 함대 줄, 커맨드렛의 외교 정책과 측정, 기준값 다시 적기.
- **B3 밖**(뒤의 블록 — §10 결정 22): 동맹·전력 공유·참전(B7 — 몰매의 엔진), AI끼리의 조약·AI가 통치자에게 하는 조약 제안 카드·AI가 조약을 끊는 길·프로필을 읽는 외교·AI의 지킴(B7 — AIProfile 행동), **수도와 마지막 정착된 성계의 정복**·정복 폐위·분쟁의 백성 제거(B5에서 사망·계승과 함께 다시 묻는다 — 또는 하지 않는다), 서 있는 위기를 함대가 늦추는 일과 그 누계 필드(B4 — 교단·영압 카드의 무력 저항과 함께), 희년(#29 ⑧ — B6 또는 EA), 전쟁의 피로(안정도의 틱당 소모 — 측정 뒤), 평화 성향의 흐름(B7 — AI의 칙령과 함께), `InitialRelations` 읽기(B7 — AI 백성 5행), 무역 조약 상대로의 역병 번짐(B7 — `arch_merchant`), 물리 연구의 함대 승수(B6), 영향권 안 소모의 줄('승무원들이 이유 없이 기도를 시작했다' — B4의 영압 카드와 함께), AI끼리 분쟁의 줄, 지도의 공성 표지, #107(격차 술어 — 둘째 백성의 격차가 실제로 생기는 B6), 헌정명 '샬롬 협정'(C).
- **바뀌는 것**: 세계가 바뀐다 — 관계가 처음으로 움직이고(국경이 닿은 쌍), 함대가 자라고, AI끼리 공성이 서고 성계가 넘어간다(지금은 0 — 세계에 백성 사이의 넘김이 없다). 시드 42 기준값 다섯은 모두 다시 적는다(§8: 필드만의 커밋 `[B-3a]`에서 해시만 옮기고, 장치 커밋 `[B-3b]`가 비트 그대로임을 보이고, 밸런스 커밋 `[B-3d]`에서 데이터를 켜고 재어 다시 적는다). **B 마일스톤에서 저장 필드를 더하는 첫 블록이다** — 추가만이라 `SaveVersion` 1 그대로.
- **지키는 것**: 분쟁은 **수도와 그 백성의 마지막 정착된 성계를 앗지 않는다**(`CosmicHazards::Transfer`의 거절 그대로 — `Sim/CosmicHazards.cpp:236`). 그래서 첫째 칸의 백성(성계 하나 — 보유 상한 1)은 정복되지 않고, 판에 한 번인 남은 자의 은혜(`DECISIONS_LOG` #53 ①)를 AI끼리의 전쟁이 써 버리지 않으며, 분쟁으로 백성이 사라지지 않는다. 첫째 칸에는 함대가 없다 — 첫 10분(틱 600 — 모두 T1)의 외교는 첫 접촉 카드 하나다.
- **판의 끝**: MVP 판은 생애가 셋째 칸에 닿으면 끝난다(`CosmicEndings::ProvisionalEndOf` — 정책 판 중앙값 T2 1,170·T3 2,370). 둘째 정착된 성계가 생기는 틱(약 1,774)부터 끝까지 통치자가 분쟁을 만나는 창은 약 600틱이다(공성 200틱으로 둘~셋). 정복이 셋째 정착된 성계를 채우면(공학 1.0과 함께) 그 틱에 셋째 칸에 올라 판이 끝난다(결정 11).

## 1. 기대는 사실 (2026-10-25, HEAD `0e29f1c`)

### 1.1 외교·분쟁 — 지금 코드에 있는 것

- 두 단계의 몸은 비었다: `TickDiplomacy`(`Sim/GalaxySimulationSubsystem.cpp:1729-1732`)·`TickConflict`(`:1734-1737`) — 주석 한 줄씩, 부르는 자리 `:1414-1415`(연구 뒤·위기 앞, 단계 가드 안).
- `FCivilizationState.FleetPower`(`Core/GalaxyState.h:279-280`, `float = 0`)는 아무도 쓰지도 읽지도 않는다. `Relations`(`:302-304`, `TMap<FName, float>` — 키는 CivID 또는 TranscendentID, 없는 키 = 0 = 중립)는 효과 `ChangeRelation` 하나가 문맥 백성 쪽만 쓰고(`Events/CosmicEventSubsystem.cpp:741-753`) 조건 `CivRelationAtLeast`(`:457-462`)가 읽는데, **게임 데이터의 어느 행도 그 둘이나 `TransferSystem`을 싣지 않는다** — 실제 판의 `Relations`는 늘 비었다.
- 태세·조약·함대 배치의 저장 자리가 없다(#24 열림 — `OPEN_QUESTIONS.md:29`). 명령 이름 `SetStance`·`ProposeTreaty`·`AllocateFleet`은 `docs/21:1558`에만 있다.
- 함대 노브 넷(`PressureFleetAttrition` 0.01, `FleetEffectivenessByTierGap` {1, 0.5, 0}, `FleetDelayPerPower` 50, `FleetDelayCap` 100 — `Data/CosmicDefinitionRows.h:864-877`, `Tools/data/DT_Rules.json:22-25`)는 읽는 코드가 없다(`Tools/check_layers.py:33`의 '읽는 코드 없는 노브' 17에 든다).
- `InitialRelations`(`CosmicDefinitionRows.h:543-545`)는 정의에만 있고 읽는 곳이 없다(두 행 모두 `{}`).
- 소유를 바꾸는 길은 `CosmicHazards::Transfer` 하나(`Sim/CosmicHazards.cpp:223-284`): 성역·죽은 노드·같은 주인·**주인의 마지막 정착된 성계**를 거절하고, 수도면 `MoveCapital`(안정도 −0.3), 두 프로젝트 칸을 비우며(그래서 정착 중인 성계는 넘어가는 순간 정착된 성계가 된다 — `:255`), `Draw` 위기와 주장은 성계를 따라 새 주인에게 간다, 다른 백성의 함대 목표는 건드리지 않는다. 받는 쪽이 다스리는 백성이면 그 성계를 알게 하는 일은 남긴 `SystemReceived` 쪽지를 `FollowUpKnowing`이 따라 한다(`Sim/CosmicHazards.h:80-86`, `Events/CosmicEventSubsystem.cpp:1272-1322` — 만료는 줄 전에 따른다, `GalaxySimulationSubsystem.cpp:1994-1998`). 지금 부르는 곳은 정착(`Sim/CosmicMegaprojects.cpp:565`)과 데이터에 없는 효과뿐이다 — **세계에 백성 사이의 넘김은 0이다**(입력 없는 60판 × 20,000틱).
- 완성 기록: 완성(`CosmicMegaprojects`의 완성 — `.cpp:594-605`)은 새 기록을 더하거나 있던 기록의 `Tick`만 고친다. 정착도 같은 길이라 데이터를 꺼도 모든 판에서 기록이 적힌다. 기록은 성계에 남아 넘김을 따라간다.
- 앎: 세계는 앎을 읽지 않는다(`docs/70_CONTENT_GUIDE.md:43`, `Sim.KnowingInert`). '만난 백성에게만'은 명령의 입력 정의로만 읽힌다(규칙 2 — 꼴은 `CosmicWanderer::CrownBlockOf(..., bool bMet, ...)`, `Sim/CosmicWanderer.h:76-84`). `NoteMet`은 `PendingContactCivID`를 적지 않는다(`Sim/GalaxySimulationSubsystem.h:206-208` 'the card is Phase 4'). 첫 접촉은 모든 판에서 틱 330(`FirstSightTick`)이고, 첫 AI 백성의 모성계는 `sys_0`의 레인 이웃이라(`.cpp:748`) **첫 생애 백성은 틱 0부터 이웃 하나와 국경이 닿는다**. 레인 이웃은 첫 눈(330) 뒤 늘 아는 성계다(보는 거리 [2, 3, 4, 6, 10]).
- 기다리는 카드 칸은 하나다(`Knowing.PendingPetitionRow` — 청원과 방랑자 카드, `Core/GalaxyState.h:642-649`). 답 `AnswerPetition`은 받아들인 답마다 시뮬 수 하나를 뽑고(`Player/PlayerRoleSubsystem.cpp:427-428`) 문맥의 `OtherID`를 채우지 않는다. 카드의 글 인자는 `CosmicPetitions::ArgsOf`(`Events/CosmicPetitions.cpp:476-500` — Civ·System·Hazard·Rebel)가 만들고, 카드 창은 `FCosmicKnownView`의 `PendingPetitionRow`·`PendingPetitionSystemID`만 받는다(`World/CosmicMapLook.h:144-147`). `DT_Encounters`의 글 칸이 받는 슬롯도 그 넷뿐이다(`Tools/data/schema.json:678-697`, 주제 Civilization이면 Civ·Hazard·Rebel — `:757-762`).
- 해시: 반사로 모든 UPROPERTY를 걷고 배열은 원소 수부터, 맵은 항목 수부터(`FName` 키만 — 값은 구조체도 된다) `FNameLexicalLess` 차례로 넣는다(`Sim/GalaxySimulationSubsystem.cpp:483-545`). **기본값인 새 필드 하나도 두 해시를 옮긴다.**
- 압력: `ComputePressureIn`(`.cpp:1221-1228`)의 P는 모든 원천의 합이다 — 초월자 항(이웃 항 포함)·그 성계 주인의 칸 항(`:282-288`)·그 자리에 선 방랑자의 항(`:290-294`). 거를 수 있는 본체 `PressureWithReach`·`PressureWeight`는 그 파일의 익명 이름공간에 있다(`:35-551`).
- `Place`의 카운트다운: Min<int64>(Max(1, `CountdownTicks`) + `CosmicEdicts::CountdownBonusOf`, `MAX_int32`)(`Sim/CosmicHazards.cpp:165-168`) — 칙령의 더하기 자체는 [0, 100000]으로 잘린 `int64` 합이다(`Sim/CosmicEdicts.cpp:148-160`).

### 1.2 세계의 모양 — 잰 값 (B2 뒤의 세계, 입력 없는 60판 × 20,000틱과 통치자 정책 30판 — 원천은 로컬 `Saved/Balance/bal_f_*.csv`; `[B-3b]` 뒤 데이터를 끈 판으로 같은 표를 다시 잰다)

| 무엇 | 값 |
|---|---|
| AI의 영향력 | 틱 1,000에 중앙값 1,000 · 3,000에 3,000 · 6,000에 5,956 · 20,000에 19,389 — 수입 약 1/틱 가운데 쓰는 몫 약 12%(개혁·구호). **영향력은 묶이지 않는다**; T2 뒤에는 에너지·물질도 묶이지 않는다(AI 에너지 중앙값 틱 6,000에 27,283). **묶이는 것은 안정도뿐**이다. 통치자 정책 판의 비축은 틱 300에 244 · 1,200에 1,351 · 2,400에 2,695 |
| 주장 | 발생 519, 편입 177(34.1%), 만료 181(34.9%) — **편입한 177건 모두 답이 돌지 않았고 주인의 영향력은 답한 주인과 같다**(7,153 대 7,062): 편입을 정하는 것은 영향력이 아니라 프로필(`ClaimLeaveProfiles` — Expansionist·Militarist)이다. 그래서 '외교의 영향력 쓰임이 편입률을 올린다'는 기대(`docs/50:12`·`STATUS.md` Top 5)는 서지 않는다 |
| 만남 | 첫 접촉 틱 330(90판 모두). 만난 백성 중앙값: 입력 없는 판 틱 600에 4·2,000에 6, 통치자 정책 판 2,000에 10·3,000에 11(모든 백성) |
| 국경이 닿은 쌍(구조 모형 — 측정 아님) | 모성계만 5쌍(틱 0) → 보유 상한 3(T2) 14쌍 → 상한 4(T3) 17쌍(66쌍 가운데). 대부분 틱 1,500~3,000 사이에 생긴다 |
| 칸 | 모든 백성의 행 가운데 T2 이상: 틱 2,000에 96%, 6,000에 T3 91%, 20,000에 T3 99%(T4 업적 없음 — AI의 끝은 T3) |
| 공성할 수 있는 성계(수도 아닌 정착된 성계)가 처음 생기는 틱 | 어느 백성이 처음 정착된 성계 둘을 갖는 틱 중앙값 1,782 |
| 보유 상한 | 틱 6,000 뒤 AI 표본의 66.5%가 상한에, 22.5%가 이웃에 막혀 상한 아래(갈 데 없음) |
| 윤리 | AI는 축마다 [−1, 1] 고른 추첨(`.cpp:823-865`), 첫 생애 백성(`arch_seeker`)은 {0, 0, 0.2}(`Tools/data/DT_Civs.json:3-6`) — 두 AI의 축별 차이 절댓값 합의 중앙값 약 2(0~6). 아래 §3.1의 식(−0.6 × 윤리 거리)으로 AI가 다른 AI를 보는 목표는 중앙값 −0.39(−0.3 이하 71%, −0.5 이하 26%), AI가 통치자를 보는 목표는 중앙값 −0.30(−0.3 이하 51%, −0.5 이하 3% — 가운데의 윤리라 덜 모질다) |
| 역병 | 판당 약 71건 발생, 만료 몫 0%(B2 뒤); 반란은 판당 약 240(모든 위기의 43.9%) |
| B1 방랑자(B2 뒤) | 60판 46/60 = 77%(시드 1~30 24/30·31~60 22/30), 즉위까지 중앙값 1,470 |
| 틱 시간 | 시드 42 홀로 평균 25.2 µs, p99 51 µs |

### 1.3 B2가 넘긴 것

- 격리령 `edict_quarantine`(행 없음 — 이름만 예약)과 역병의 번짐: 역병 행 `crisis_plague`의 만료 이웃 효과는 비었다(`Tools/data/DT_Hazards.json:115`). **역병은 거의 만료하지 않는다**(B2 뒤 만료 몫 0% — 시드 42 20,000틱 발생 82·만료 0) — 번짐을 만료에 걸면 판에 한 번도 일어나지 않아 격리령이 다시 빈 칙령이 된다.
- 전시 동원의 함대 효과·무역 자유화의 조약 효과: 이름만 있고 정의가 어디에도 없다(`systems.md` §5.8, B2 헤더 §0). AI는 첫 AI 단계에서 모든 칙령을 철회하므로(`AIEdictRevokeStability` 1.0) 칙령의 효과는 B7까지 다스리는 백성만 받는다 — **함대의 기본 원천은 칙령 밖에 있어야 한다** — 단 물려받은 칙령(전시 동원)에 실은 효과는 통치자의 기본값이 된다(§3.2).
- 칙령 창의 묶음(#124 — 'B3 헤더 전, 첫 새 칙령 행과 함께').
- 청원 '역병'의 의회 목소리 '성문을 닫아 번지는 것을 막자'(`DT_Encounters.json:762`)는 번짐이 없어 지금 거짓이다 — B3가 번짐을 넣으면 참이 된다.

### 1.4 열린 물음 가운데 B3가 닫거나 옮길 것

#24(태세·조약·배치의 저장 — §2.1에서 닫는다), #42(정복과 보유 상한 — 정복도 센다로 닫는다, 결정 6), #73(남의 완성 기록 — 제 백성의 완성만, 결정 11), #31 ②(두 백성의 줄 — 둘째 백성은 인자로만; B3는 다스리는 백성이 당사자인 줄만 낸다), #35(틱 0의 입력 — 새 명령 넷에도 '틱 0에 들어온 입력은 첫 틱 전에 든다'), #124(칙령 창의 묶음 — 행 여섯은 한 목록으로 든다, 묶음은 '칙령 9'의 B7로), #29 ⑧(희년의 걸림 '외교 단계 전'을 B6으로 옮긴다), #107(B6으로 옮긴다).

### 1.5 문서끼리·문서와 코드가 어긋난 것 (이 헤더가 고친다 — §7)

- `systems.md:18` 영향력 '인구 × 개방 축 × **조약 수**' — 곱이면 조약 0개의 백성은 영향력 0이다 → '+ 무역 조약마다'(§3.1).
- `systems.md:44`(사회가 '조약 종류'를 연다) ↔ `pillars.md:76`(T2의 무역 조약) → 조약은 둘째 칸부터(결정 4); 사회 연구의 줄에서 '조약 종류'를 뺀다.
- `systems.md:54` '스칼라 하나' ↔ 저장은 방향이 있는 둘 → 방향을 적는다.
- `systems.md:58` 국경 = '소유 성계'가 1레인 ↔ 세계의 인접 판정은 '정착된 성계'(CLAUDE.md 규칙 13) → 정착된 성계로.
- `systems.md:62-63` '비율로 배치'·'틱마다 전력 비교 + 난수' → 목표 하나·공성 + 끝에서 한 번(결정 1·2).
- `systems.md:64` 본보기 줄('카시오페아 연합이 … 이양됩니다.')은 높임 현재·행정 낱말·신화 이름·실제 별 이름 — 게임의 문체로 새로 쓴다.
- `systems.md:66`·`docs/21:1347` '승수·소모 2줄'·'함대 투입당 카운트다운 +50' ↔ B2의 불변식 '서 있는 위기의 카운트다운은 바뀌지 않는다'(`Sim/CosmicHazards.h:54-56`) → 함대도 **놓일 때만**(결정 8), 단계의 크기를 바로 적는다.
- `docs/21:1347`·`docs/20:287` '분쟁이 통치 문명의 마지막 성계를 앗으면 폐위 기록' ↔ `Transfer`의 거절 → B3는 앗지 않는다(결정 3).
- `docs/21:1459` '이웃의 태세는 무작위' — 태세 필드도 생성 추첨도 없다 → '이웃의 아키타입·윤리는 무작위'로.
- `crisis_and_transcendents.md:22`·`CosmicDefinitionRows.h:180-185` 번짐 = 만료 결과 ↔ 역병 만료 0% → 발생 몫의 이웃 곱(결정 13).
- `crisis_and_transcendents.md:34`(함대가 교단을 진압)·`:25`(지연만) → 지연만(B4가 교단과 함께).
- `crisis_and_transcendents.md:72`(분쟁의 인구 손실 ×0.5) — 분쟁은 인구를 줄이지 않는다(소유만).
- `crisis_and_transcendents.md:125` 'AI가 스스로 하는 일은 넷' → 여섯(외교의 적대·공성).
- `docs/50:180`·`:255` 청원 10장에 첫 접촉을 넣어 셈 ↔ MVP ②의 10장에 없음 → 첫 접촉을 더해 11장.
- `docs/21:1458`·`docs/00_VISION.md:175` 첫 접촉 카드의 선택 셋 → 둘(결정 12).
- A2 헤더 규칙 5(카드는 줄을 세우지 않는다) ↔ `docs/70:61`(첫 접촉 카드는 기다린다) → 첫 접촉만 제 칸이 있어 한 장 기다린다(§2.9).
- `docs/70:61` '헤드리스 판은 답하지 않는다' ↔ 정책 판(`-Answer`)이 답한다 → '입력 없는 판은 답하지 않는다'.
- `Install`이 `PendingContactCivID`를 비우지 않는다(`PlayerRoleSubsystem.cpp:108`) ↔ A2 헤더·`docs/70:61` '역할이 바뀌면 비운다' → B3가 `Install`에 한 줄을 더한다(§2.9). `CosmicPetitions::Clear`(`Events/CosmicPetitions.cpp:502-506`)는 그 칸을 그대로 둔다(기다리는 첫 접촉 카드를 지킨다).
- `docs/20:198`의 '분쟁·조우·계승의 지점은 Phase 4·4b에' — 조우의 지점은 B1이 넣었다; 분쟁의 지점은 B3가 넣는다.
- `docs/50:12`·`STATUS.md` Top 5·`DECISIONS_LOG` B2 '주장 편입률은 AI의 영향력 쓰임(B3 외교)과 함께 다시 잰다' → 편입을 정하는 것은 프로필이다(§1.2) — 측정 항목으로만 남긴다(결정 19).
- `docs/70:166`·`crisis_and_transcendents.md:116` '가져갔다'는 타락의 입에만 — 그것을 지키는 기계 검사(`Data.Integrity` 린트)가 있다고 적었으나 낱말 목록이 없다 → 공성의 줄의 글 규칙에 적고(§3.4) 검사는 글 감사가 본다('거두었다'는 `soul_verbs`가 막는다).

## 2. 저장 필드·세계 함수·명령·단계

### 2.1 저장 필드 (`[B-3a]` — 헤더와 완성 기록의 쓰기 한 줄만)

| 자리 | 필드 | 뜻 |
|---|---|---|
| `FCivilizationState` 끝 | `TMap<FName, FCosmicTie> Ties` | 상대 CivID → 짝 기록. 없는 키 = 중립·조약 없음. **정규형**: 항목은 기본값(태세 없음 ∧ 조약 없음)이 아닐 때만 있다 — 쓰는 함수가 기본값이 되면 항목을 지운다(뜻이 같은 두 상태가 다른 해시를 갖지 않게). 건넘 단계의 정리가 지운 백성의 키를 모든 백성의 `Ties`에서도 지운다(`Relations`를 지우는 같은 자리 `.cpp:2256` — 추첨 없음) |
| 새 USTRUCT `FCosmicTie`(`Core/GalaxyState.h`) | `FGameplayTag Stance`, `FGameplayTagContainer Treaties` | 태세: `Stance.Friendly`·`Stance.Hostile`, 빈 태그 = 중립(이 백성이 그 상대를 대하는 자세 — 방향이 있다). 조약: `Treaty.Trade`·`Treaty.NonAggression`(두 백성에 같은 꼴로 — 대칭은 쓰는 함수 하나가 지킨다). 태그라 열거자 차례의 함정이 없고 동맹(B7)은 태그 하나를 더한다 |
| `FCivilizationState` 끝 | `FName FleetTargetSystemID`, `int64 FleetTargetSinceTick = 0` | 함대의 목표 성계(없음 = 제 성계들을 고루 지킨다)와 목표를 정하거나 풀어 준 틱. 목표가 있으면 공성·지킴의 시작, 목표가 없으면 **쉼의 시작**(AI는 공성이 끝난 뒤 `SiegeRestTicks` 동안 적대를 열지 않는다 — §2.8; 통치자에게는 쉼이 없다). 공성은 이 둘에서 파생한다 — 성계 쪽 필드 없음, **위기 칸을 쓰지 않는다**(쓰면 공성이 끝날 때마다 주장 규칙이 돈다 — 주장은 만료가 비운 칸에서만 선다, #123) |
| `FMegaprojectRecord` 끝 | `FName CivID` | 그 성계에서 그 행을 마지막으로 완성한 백성(#73). 완성은 새 기록이든 있던 기록이든 `Tick`과 함께 `CivID`를 그 백성으로 고쳐 쓴다 — **`[B-3a]`가 이 쓰기를 넣는다**(데이터를 꺼도 완성은 일어나므로 장치 커밋에 넣으면 그 커밋의 해시가 옮는다; `[B-3a]`에서는 읽는 코드가 없고 백성 사이의 넘김이 없어 기록의 `CivID`는 늘 지금 주인이라 CSV의 결정적 칸이 그대로다). 옛 세이브의 기록은 `NAME_None` — §2.10 |

- 태그 선언(`Config/DefaultGameplayTags.ini`): `Stance.Friendly`·`Stance.Hostile`, `Treaty.Trade`·`Treaty.NonAggression`(DevComment와 함께).
- 해시: 새 필드는 반사로 곧 든다. `FleetPower`는 이미 있다. `SaveVersion` 1 그대로(추가만, 배열 원소 구조체 안이라 델타 저장의 초기값 함정이 없다).
- 세계 상태라 `Knowing` 안에 두지 않는다(세계 해시 밖이 되면 안 된다).

### 2.2 `Sim/CosmicDiplomacy.h/.cpp` (새 — 순수 함수 이름공간, 상태 없음, 추첨 없음)

- 읽기: `const FCosmicTie* TieOf(const FCivilizationState&, FName OtherID)`, `FGameplayTag StanceOf(...)`(없으면 빈 태그), `bool HasTreaty(..., FGameplayTag Treaty)`, `int32 TreatyCountOf(...)`, `int32 FriendlyCountOf(const FCivilizationState&)`.
- 쓰기(정규형을 지킨다): `void SetStanceOf(FCivilizationState&, FName OtherID, FGameplayTag Stance)`, `void Sign(FGalaxyState&, int32 A, int32 B, FGameplayTag Treaty)`·`bool Break(FGalaxyState&, int32 A, int32 B, FGameplayTag Treaty)`(두 백성을 함께 — 대칭의 하나뿐인 길), `void ForgetCiv(FGalaxyState&, FName CivID)`(정리 — `Ties`에서 그 키를 지운다).
- 국경: `bool Borders(const FGalaxyState&, const FCosmicStateIndex&, int32 A, int32 B)` — 두 백성의 **정착된 성계**(규칙 13의 술어 `CosmicState::IsSettled`)가 같은 `GalaxyRow`에서 1레인. 성역은 무주라 저절로 빠진다. 외교 단계는 단계 시작에 한 번 만든 국경표(지역 값)를 쓴다. 분쟁 단계의 정복의 눈은 그 넘김 직후 `Borders`를 그 자리에서 묻고(진 백성은 늘 포함), AI 단계의 '국경이 닿고'도 그 자리에서 묻는다(분쟁 단계의 넘김 뒤 세계).
- 윤리 거리: `float EthicsDistanceOf(const FEthicsAxes&, const FEthicsAxes&)` = (|Δ평화| + |Δ영성| + |Δ개방|) ÷ 3 — `Abs`만(규칙 6).
- 목표값: `float TargetRelationOf(State, Rules, int32 Viewer, int32 Other)` = −`RelationEthicsWeight` × 거리 + (그 상대가 이 백성을 대하는 태세의 항 `RelationStanceTerms`[우호·적대]) + `RelationTreatyTerm` × 두 백성의 조약 수. + − ×만. 관계를 읽지 않는다.
- 보는 눈: `float ViewOf(State, Index, Rules, int32 Viewer, int32 Other)` = 두 백성의 국경이 닿았으면 저장된 `Relations[Other]`, 아니면 태세의 항을 뺀 목표값(−`RelationEthicsWeight` × 거리 + 조약의 항 — 그 쌍은 표류하지 않아, 기본값 0이나 곧바로 읽는 태세의 항이 문턱을 넘는 길 — 우호를 한 경계 걸었다 놓아 조약을 사는 길 — 을 막는다; 태세는 표류로만 눈을 옮긴다). 승낙(`TreatyBlockOf`의 `Refused`), [백성] 탭의 낱말 띠와 툴팁, 외교 정책, 측정의 relation mean, AI의 적대의 문이 이 하나를 읽는다. 국경 밖 쌍에는 `BreakTreaty`·정복의 눈이 남지 않는다(목표값이 덮는다 — 툴팁이 '국경이 닿지 않아 그들의 눈은 윤리와 조약의 몫 그대로다'를 말한다).
- 낱말 띠(화면): `int32 ViewBandOf(float Relation)` — 다섯(적의 ≤ −0.5 · 냉담 ≤ −0.2 · 무심 < 0.2 · 호의 < 0.5 · 신뢰).
- 우호의 자리: `int32 FriendlySlotsOf(const FCivilizationState&, const FCosmicRulesDef&)` = `FriendlySlotsByTier`[칸](배열 밖 = 0).
- 거절의 첫 사유(명령·창·정책·효과가 같은 함수를 읽는다 — `EnactBlockOf`의 꼴, 저장되지 않는 `enum class`; 규칙 13의 단일 창구):
  - `ECosmicStanceBlock StanceBlockOf(State, Index, Rules, int32 CivIndex, FName OtherID, FGameplayTag Stance, bool bMet)` — `NoCiv`(살아 있는 백성이 아니다) → `Self` → `NotMet` → `Same` → (우호) `FriendlySlots`(자리가 찼다) → (적대) `Tier`(둘째 칸부터 — 첫 10분에 전쟁이 없다) → (적대) `NonAggression`(불가침을 먼저 끊어야). 명령만 더 낸다: `NotRuler`·`Handler`.
  - `ECosmicTreatyBlock TreatyBlockOf(State, Index, Tables, Rules, int32 CivIndex, FName OtherID, FGameplayTag Treaty, bool bMet)` — `NoCiv` → `Self` → `NotMet` → `Held` → `Tier`(제안하는 쪽이 둘째 칸 아래) → `Hostile`(어느 쪽이든 적대) → `Refused`(`ViewOf`(그 백성, 제안한 백성) < `TreatyAcceptRelation` − 제안하는 백성이 지닌 칙령의 `TreatyAcceptBonus` 합) → `Cost`(`TreatyCostInfluence`).
- **적대는 무역 조약을 끊는다**: 적대가 서는 두 자리 — 통치자의 `SetStance`(적대)와 AI가 적대를 여는 자리(§2.8 3.) — 는 그 쌍의 무역 조약을 `CosmicDiplomacy::Break`로 두 쪽에서 함께 끊는다(통치자가 걸었으면 그 상대의 눈 −`BetrayalRelationPenalty`, AI가 열었으면 값 없음; 다스리는 백성이 당사자면 줄 `log_treaty_broken`). 적대 중인 쌍은 `TreatyBlockOf`의 `Hostile`로 다시 맺지 못한다(불가침은 적대를 막으므로 함께 설 일이 없다).
- 줄의 낱말: 태세·조약의 표시 이름은 스트링 테이블(`Stance.*`·`Treaty.*` — 태그의 짧은 이름을 키로).

### 2.3 `Sim/CosmicConflict.h/.cpp` (새 — 순수 함수 이름공간, 상태 없음, 추첨 없음)

- **`FleetPower`는 늘 0 이상이다** — 자람·유지비·소모·진 공격의 곱이 모두 0에서 멈춘다(비교로 — 규칙 6).
- 함대의 상한: `float FleetCapOf(const FGalaxyState&, const FCosmicStateIndex&, int32 CivIndex, const FCosmicRulesDef&)` = `FleetPerSettledByTier`[칸] × 그 백성의 정착된 성계 수(배열 밖·수가 아님 = 0). 첫째 칸은 0 — **첫째 칸에는 함대가 없다**(첫 10분에 공성이 없다). 정복이 성계를 늘리면 상한도 늘지만 보유 상한(정복도 센다 — 결정 6)이 그 끝을 막는다. 이 꼴은 성계를 잃은 백성의 함대를 줄이고 같은 칸의 큰 백성과 작은 백성을 가르며, 칸이 앞선 통치자가 늘 강하지는 않게 한다. 후반이 어는 것은 풀지 않는다 — 보유 상한에 닿은 AI(틱 6,000 뒤 66.5%)는 모두 같은 상한(T3 4)이고 `RealmCap`이 그들의 공성을 막으며, 공성할 수 있는 상한 아래의 AI(22.5%)는 성계가 적어 함대도 작아 상한에 닿은 이웃에게 전력 비의 문을 지나지 못한다(측정 1의 시간 분포가 본다).
- 유효 전력: `float EffectivePowerOf(float Power, int32 TierGap, const FCosmicRulesDef&)` = Power × `FleetEffectivenessByTierGap`[Clamp(격차, 0, 2)] — 격차는 '상대가 더 높은 만큼'만(아래로는 0). **공격하는 쪽에만 걸린다** — 지키는 쪽은 제 성계에서 받지 않는다(앞선 칸이 늘 이기는 눈덩이를 막는다). 위기를 상대할 때(지연)는 그 위기의 원천의 칸: 주장은 그 타락 초월자의 칸, 그 밖의 위기(역병·각성·전조)는 격차 0.
- 공성의 문: `ECosmicSiegeBlock SiegeBlockOf(State, Index, Rules, int32 Attacker, int32 SystemIndex, bool bAssumeHostile = false)` — `Sanctuary`(규칙 13의 첫 거절) → `NoSystem`·`Dead` → `Unowned`(무주) → `Own`(제 성계 — 그것은 지키기) → `NotSettled`(정착 중인 성계는 노리지 않는다) → `Capital` → `LastSettled`(`CosmicHazards::IsLastSettledSystem`) → `NotAdjacent`(이 백성의 정착된 성계에서 1레인이 아니다) → `Besieged`(다른 백성의 공성이 이미 서 있다) → `RealmCap`(이 백성의 소유 수가 `RealmCapOf` 이상 — **정복도 보유 상한을 센다**, #42) → `NoFleet`(유효 전력 0) → `NonAggression` → `NotHostile`(이 백성이 그 주인을 대하는 태세가 적대가 아니다 — `bAssumeHostile`이면 묻지 않는다: AI가 적대를 열기 전에 노릴 성계가 있는지 묻는 자리). 구조의 사유가 먼저라 화면의 첫 사유가 길을 바로 알린다('먼저 적대해야 한다'는 다른 길이 모두 열렸을 때만). 명령만 더 낸다: `NotRuler`·`Handler`. 노릴 성계는 늘 제 정착된 성계의 레인 이웃이라 첫 눈 뒤 아는 성계다 — 명령은 아는 성계를 묻지 않는다(규칙 2의 '명령이 읽는 앎'을 늘리지 않는다).
- 지킴의 문: `ECosmicGuardBlock GuardBlockOf(State, Index, Rules, int32 CivIndex, int32 SystemIndex)` — `Sanctuary` → `NoSystem`·`Dead` → `NotOwn` → `NotSettled` → `NoFleet`(상한 0 — 첫째 칸은 지킬 함대가 없다: 쓸모없는 입력이 세계 필드를 바꾸지 않게). 목표 없음(`NAME_None`)은 늘 받는다.
- 공성의 몫: `float SiegeOddsOf(State, Index, Rules, int32 Attacker, int32 SystemIndex)` — A = 공격하는 쪽의 유효 전력, D = 지키는 주인의 `FleetPower` × 지키는 몫(그 주인의 목표가 이 성계면 `SiegeDefenceShares`[0] 1, 목표가 없으면 [1] 0.5, 다른 성계면 [2] 0.25). A ≤ 0(또는 수가 아님)이면 0, D ≤ 0 ∧ A > 0이면 1, 그 밖에 A ÷ (A + D). `bool Takes(float Number, float Odds)` = Number < Odds(동률은 지키는 쪽). 화면의 띠는 **늘 다스리는 백성 쪽에서** 본다 — 우리가 에워쌀 때는 몫, 우리가 지킬 때는 1 − 몫: 우세 ≥ 0.6 · 팽팽 · 열세 ≤ 0.4. 툴팁도 같은 쪽의 몫.
- 공성의 끝: `Tick` ≥ `FleetTargetSinceTick` + Max(1, `SiegeTicks`) — 이 틱에 이르렀거나 이미 지난 공성(노브가 줄어든 세이브의 공성도 다음 분쟁 단계에서 끝난다).
- 지연: `int32 FleetDelayOf(State, Index, Tables, Rules, int32 SystemIndex, const FHazardDef& Row)` — 그 성계의 주인의 함대 목표가 그 성계일 때만, `Capital` 규칙의 행(반란)이 아닐 때만(제 백성의 불만을 힘으로 미루지 않는다 — 반란에는 밤샘·절식이 있다), `FleetDelayPerPower` × 유효 전력을 비교로 [0, Min(`FleetDelayCap`, 100000)] 안에 둔 뒤 `int32`로(음수·수가 아닌 값은 0 — 칙령의 `CountdownBonusOf`와 같은 자름).
- 소모의 압력은 이 이름공간에 두지 않는다: `static FCosmicPressure UGalaxySimulationSubsystem::ComputeAttritionPressureIn(State, Index, Tables, Rules, int32 CivIndex, int32 SystemIndex)`(공개 static — `ComputePressureIn` 곁; 본체는 익명 이름공간의 `PressureWithReach` 거른 변형): 봉인되지 않은 초월자 가운데 **칸이 그 백성보다 높고 그 성계를 영향권 안(Inside)에 둔 것들의 항만** — 주인의 칸 항·방랑자 항·이웃(Adjacent) 항 없음. 분쟁 단계의 소모, AI의 노릴 성계, 패널의 소모 툴팁, `Sim.Fleet`가 이 하나를 읽는다(`ComputePressureIn`은 주인·방랑자를 원천으로 세므로 쓰지 않는다).

### 2.4 명령 (`UPlayerRoleSubsystem`, 틱 밖의 입력 — 추첨 없음, 줄은 말하기 스트림만)

- `ECosmicStanceBlock SetStance(FName CivID, FGameplayTag Stance)` — 몸을 입은 통치자의 다스리는 백성이 그 상대를 대하는 태세를 바꾼다. 우호는 한 번의 값이 없고 서 있는 동안 영향력 유지비와 우호의 자리 하나를 쓴다(§2.5), 적대는 값이 없다(그 대신 그 상대가 우리를 보는 눈이 표류로 내려가고, 그 상대와의 무역 조약을 끊는다 — 툴팁이 미리 말한다). 줄 `log_stance_changed`(Severity 1 — 연대기는 받지 않는다: 칙령의 줄과 같은 장부).
- `ECosmicTreatyBlock ProposeTreaty(FName CivID, FGameplayTag Treaty)` — 승낙은 곧 판정한다(추첨 없음 — `systems.md:56`): `TreatyBlockOf`가 None이면 값을 내고 `CosmicDiplomacy::Sign`. 줄 `log_treaty_signed`(Severity 1 — 연대기가 받는다: 다스리는 백성이 주제). 거절은 줄 없이 사유로만 돌아온다(화면의 툴팁).
- `bool BreakTreaty(FName CivID, FGameplayTag Treaty)` — 지닌 조약만. 그 상대가 우리를 보는 눈 −`BetrayalRelationPenalty`(곧). 줄 `log_treaty_broken`(Severity 1).
- `ECosmicSiegeBlock AllocateFleet(FName SystemID)` — `NAME_None` = 함대를 물린다(고루 지킨다). 제 성계면 `GuardBlockOf`, 남의 성계면 `SiegeBlockOf`. 받아들이면 `FleetTargetSystemID`·`FleetTargetSinceTick = State.Tick`(같은 목표를 다시 고르면 틱을 바꾸지 않는다 — 켜고 끄며 공성 시간을 되감지 못하게). 공성을 시작하면 줄 `log_siege_begun`(Severity 1 — 플레이어가 방금 한 일이라 자동 정지하지 않는다, 연대기는 받는다; 상대는 적대의 문에서 이미 만난 백성이다).
- 공통: 핸들러 안이면 에러 한 줄과 `Handler`(가드 목록 `GalaxySimulationSubsystem.h:261-272`와 `docs/20`의 '추첨 없는 틱 밖 입력' 목록에 넷을 더한다), 컨트롤러는 끝난 판에서 거절(`IsRunEnded`), 저장은 다음 경계의 정해진 까닭(`EnactEdict`와 같다). 개발용 콘솔 `CosmicStance <CivID> <friendly|neutral|hostile>`·`CosmicTreaty <CivID> <trade|nonaggression> [break]`·`CosmicFleet <SystemID|none>`(인자 없으면 사유 한 줄씩).
- 만난 백성: 명령은 `State.Knowing.MetCivs`를 입력의 정의로 읽는다(규칙 2 — '만난 백성'은 이미 그 목록에 있다). 표현은 `FCosmicKnownView::IsMet`(규칙 14).

### 2.5 외교 단계 (셋째 — `TickDiplomacy`)

차례(단계 시작에 국경표를 한 번 만든다):
1. **우호의 유지비**: 백성마다(`Civs` 차례), 짝 기록을 키의 사전 차례로 걸으며 태세가 우호인 항목마다 `FriendlyUpkeepInfluence`를 비축에서 — 내지 못하면 그 태세가 중립으로 그친다(다스리는 백성이면 줄 `log_stance_lapsed`, AI는 줄 없음). 우호의 수가 자리보다 많으면(칸이 내려가지는 않지만 노브가 바뀐 세이브) 키의 사전 차례로 뒤의 것부터 그친다. AI는 우호를 지니지 않으므로(§2.8) 사실상 통치자만 낸다.
2. **관계의 표류**: 백성 X마다, 국경이 닿은 상대 Y마다(`Civs` 차례): R ← R + `RelationDriftRate` × (`TargetRelationOf`(X, Y) − R), [−1, 1]로 자른다. **바뀌지 않으면 항목을 만들지 않는다**(표류율 0이면 아무 항목도 생기지 않는다 — 장치 커밋의 비트). 국경이 닿지 않은 쌍은 움직이지 않는다(있는 값이 그대로 남는다). 한 쌍의 쓰기는 다른 쌍의 읽기에 닿지 않는다(목표값은 태세·조약·윤리만 읽는다).
- 초월자 키(`Relations[TranscendentID]`)는 표류하지 않는다(국경이 없다 — `systems.md:58`의 예외 그대로).
- 무역 조약의 영향력은 경제 단계의 수입 자리에서 더한다(첫째 단계 — `IncomePerTick`가 다시 만들어지는 곳): 영향력 += `TradeInfluencePerTreaty` × 무역 조약 수. 맺은 틱의 다음 틱부터 센다.
- 이 단계에 추첨은 없다. 줄은 말하기 스트림만. 소리 없는 표류는 만남을 낳지 않는다.

### 2.6 분쟁 단계 (넷째 — `TickConflict`)

목표의 정리(단계 밖의 두 곳): `CosmicHazards::Destroy`와 `CosmicHazards::Transfer`(모든 길 — 정착의 넘김은 무주에서 오므로 무동작)는 그 성계를 함대 목표로 둔 모든 백성의 목표를 풀고 `FleetTargetSinceTick = Tick`(줄 없음 — 넘어감·파괴의 줄이 말한다). 그래서 분쟁 단계가 만나는 목표는 늘 주인이 있다: 주인이 그 백성이면 지킴, 다른 백성이면 공성이다.

차례:
1. **함대**(백성마다 `Civs` 차례): 유지비 = `FleetUpkeepEnergyPerPower` × `FleetPower`를 비축의 에너지에서 — 다 내면 자람(못 내면 아무것도 내지 않는다 — B2 칙령 유지비의 '모두 내거나'와 같은 꼴): `FleetPower` ← Min(상한, `FleetPower` + `FleetGrowthPerTick` × 지닌 칙령의 `FleetGrowthMultiplier` 곱), 못 내면 자라지 않고 Max(0, `FleetPower` − `FleetGrowthPerTick`). 상한보다 크면(성계를 잃었다) 상한으로. **영향권 안의 소모**: 목표 성계가 있고 그 Pf(§2.3)가 0보다 크면 `FleetPower` ← Max(0, `FleetPower` − `PressureFleetAttrition` × Pf)(Pf = `ComputeAttritionPressureIn`(그 백성, 목표 성계).P — 목표가 있는 백성만, 틱당 12번 이하).
2. **끝나는 공성을 모으고 먼저 뽑는다**: 목표 성계의 주인이 다른 살아 있는 백성이고 공성의 끝(§2.3)에 이른 백성을 공격하는 쪽의 `Civs` 차례로 모으고(목표의 주인과 시작 틱과 `SiegeTicks`만 본다 — 문을 보지 않는다), **모은 공성마다 시뮬 수 하나를 모두 먼저 뽑는다**(규칙 ⓑ). 뽑는 수는 목표와 시작 틱(세계 상태)과 `SiegeTicks`(위기의 `CountdownTicks`와 같은 '때'의 값)만 따른다 — `docs/20:198`에 새 예외를 더하지 않는다.
3. **목표의 점검**: 목표가 있는 백성마다 — 주인이 그 백성이면 `GuardBlockOf`, 다른 백성이면 `SiegeBlockOf`; None이 아니면 목표를 풀고 `FleetTargetSinceTick = Tick`(쉼의 시작). 공성을 풀 때만, 다스리는 백성이 당사자면 줄 `log_siege_lifted`(Severity 1). 모은 공성이면 그 수는 버린다(만료의 답 칙령과 같은 꼴 — `.cpp:1953-1961`).
4. **끝의 적용**: 남은 모은 공성을 차례로, **적용 직전에 `SiegeBlockOf`를 다시 묻는다**(앞의 넘김이 뒤의 문을 바꿀 수 있다) — None이 아니면 풀린 것으로(함대 손실 없음, 수는 버린다, 3.과 같은 줄); None이면 `Takes`(수, `SiegeOddsOf`):
   - **넘어간다**: `MakeEvalContext`의 문맥으로 `CosmicHazards::Transfer` → 곧 `Events->FollowUpKnowing(Ctx)`(받은 쪽이 그릇의 백성이면 그 성계를 알고 준 백성을 만난다; 진 쪽이 다스리는 백성이면 상대를 `NoteMet` — 둘 다 줄 전에). (`Transfer`는 거절 검사 뒤, 주인을 바꾸기 전에 그 성계의 `CivID`가 `NAME_None`인 완성 기록에 옛 주인을 적고 그 성계를 노린 모든 목표를 푼다 — §2.10·위의 정리; `Transfer` 안의 한 곳이라 모든 넘김이 같다.) 정복한 백성과 국경이 닿은 모든 백성(진 백성 포함 — 넘김 직후 `Borders`를 묻는다)이 그 백성을 보는 눈 −`ConquestRelationPenalty`.
   - **지킨다**: 공격한 쪽 `FleetPower` × `SiegeRepulsedFleetFactor`(0.5).
   - 둘 다: 공격한 쪽의 태세를 중립으로 되돌리고(휴전) 목표를 풀며 `FleetTargetSinceTick = Tick`(쉼의 시작 — AI는 `SiegeRestTicks` 동안 적대를 다시 열지 않는다, §2.8). 다스리는 백성이 당사자면 상대를 먼저 `NoteMet`하고 쪽마다의 줄(§3.4 — `log_siege_taken`·`log_siege_lost`·`log_siege_held`·`log_siege_failed`, Severity 2).
- 한 틱에 한 성계는 한 번만 넘어간다(한 성계에 공성 하나 — `Besieged`).
- 추첨 지점: '그 틱에 끝에 이른 공성마다 1개'(일어난 일마다 그 수만큼 — 위기 만료와 같은 꼴, `docs/20:198`). 데이터를 끈 판(함대 0 — 공성이 서지 않는다)에는 뽑지 않는다.
- 분쟁은 위기 단계 앞이다: 넘어간 성계는 같은 틱의 위기 발생 추첨에 새 주인으로 든다(정착된 성계마다 1개 — 수는 그대로).

### 2.7 위기 단계의 두 고리 (다섯째)

1. **함대의 지연**(놓일 때만): `CosmicHazards::Place`의 카운트다운 = Min<int64>(Max(1, `CountdownTicks`) + `CountdownBonusOf` + `FleetDelayOf`, `MAX_int32`) — 지금 `Place`의 꼴에 항 하나만 더한다(두 덧셈항은 각각 [0, 100000]이라 1 아래로 내려가지 않는다). **서 있는 위기의 카운트다운은 바뀌지 않는다**(B2 불변식 그대로 — 함대를 옮겨도 서 있는 위기는 그대로; 화면이 '새로 서는 위기에만'을 말한다). 서 있는 위기를 함대가 늦추는 일은 B4(교단·영압 카드의 무력 저항)와 함께 다시 묻는다.
2. **역병의 번짐**: 발생 ⓒ의 시작에 '레인 이웃에 역병이 서 있는 정착된 성계'의 표를 한 번 만든다(그 고리 안에서 방금 놓인 역병을 읽지 않는다 — 차례의 치우침이 없게). 역병 행의 발생 몫 = `Chance` × 아키타입 곱 × 칙령 곱 × (표에 들면 `PlagueSpreadMultiplier`). **추첨 수는 그대로**(성계마다 1개 — 몫만 바뀐다). 격리령은 B2의 칸 `HazardSpawnMultipliers {Hazard.Plague: 0.1}`로 제 땅의 발생과 번져 오는 역병을 함께 줄인다 — '미리 세우는 수정자'(이미 선 역병은 그대로, `Edict.TipNewOnly`와 같은 꼴). 무역 조약 상대로의 번짐은 B7.

### 2.8 AI 단계의 외교 규칙 (여섯째 — 추첨 없음, 프로필을 읽지 않는다)

**AI의 적대는 공성과 한 몸이다** — 노릴 성계가 있을 때만 적대하고, 공성이 끝나거나 풀리면 적대도 풀린다. 플레이어가 다스리지 않는 백성 X마다(섭정 아래의 옛 백성도), 프로젝트·정착·칙령 철회 **뒤에**:
1. **우호와 지킴을 놓는다**: AI는 우호를 지니지 않고 지킴을 쓰지 않는다(유지비와 자리를 쓰는 장부는 통치자의 것 — 섭정은 통치자가 남긴 우호를 놓고, 제 성계를 목표로 둔 지킴을 물린다: `FleetTargetSinceTick = Tick`, 쉼의 시작; 통치자가 남긴 공성은 2.의 규칙대로 이어 간다; B2의 칙령과 같은 자리).
2. **적대를 푼다**: X가 적대하는 Y마다 — X의 함대 목표가 Y의 성계가 아니면(공성이 끝났거나 풀렸거나, 통치자가 남긴 목표 없는 적대) 중립으로. 적대가 둘 이상이면(섭정이 물려받은 경우) 목표의 주인이 아닌 것을 모두 푼다.
3. **적대를 연다**(X에게 적대가 없고 쉬는 중이 아닐 때만 — 목표가 없고 `Tick` ≥ `FleetTargetSinceTick` + `SiegeRestTicks`): `Civs` 차례의 첫 Y — 국경이 닿고(그 자리에서 묻는다) ∧ 불가침이 없고 ∧ `ViewOf`(X, Y) ≤ `AIHostileRelation` ∧ X의 `FleetPower` ≥ `AIHostilePowerRatio` × Y의 `FleetPower` ∧ X의 평화 축 < `AIHostilePacifismMax` ∧ **다른 어느 AI도 Y에게 적대하지 않는다**(몰매 막기 — 태세로 본다) ∧ 다스리는 백성이 Y를 에워싸고 있지 않다 ∧ **Y의 성계 가운데 `SiegeBlockOf`(X, 그 성계, bAssumeHostile = true)가 None이고 `ComputeAttritionPressureIn`(X, 그 성계).P가 0인 성계가 있다**(소모가 공성이 끝나기 전에 함대를 비우는 곳은 노리지 않는다 — 노릴 성계가 없는 백성, 수도뿐인 첫째 칸 백성에게는 적대를 열지 않는다). 적대를 열면 그 쌍의 무역 조약을 끊고(§2.2) 그 첫 성계(`Systems` 차례)를 목표로 정하고 `FleetTargetSinceTick = Tick` — 공성이 곧 선다. Y가 다스리는 백성이면 상대를 먼저 `NoteMet`하고 줄 `log_siege_on_us`(Severity 2 — 자동 정지·연대기).
- AI는 조약을 제안하지 않고(통치자의 제안에 `TreatyBlockOf`로 답할 뿐 — B7), 불가침을 끊지 않는다(통치자가 끊기 전까지 선다 — 평화의 길; 무역은 적대가 서면 끊긴다), 지킴을 쓰지 않는다(B7 — 프로필과 함께).
- 차례의 치우침: `Civs` 앞의 AI가 먼저 고른다(결정적 — 고르게는 아니다, 받아들인다).
- 즉위: 방랑자가 AI 백성에 오르면 그 백성의 조약을 받는다. 함대 목표가 남의 성계면 즉위가 풀고(`FleetTargetSinceTick = Tick`) 적대는 모두 중립으로 되돌린다(명령의 세계 쓰기 — 추첨·앎 읽기 없음): 새 통치자는 모르는 성계를 두고 보이지 않는 공성이나 바꿀 수 없는 적대를 물려받지 않는다.

### 2.9 첫 접촉 카드 (`petition_first_contact`, 아홉째 단계의 끝 — **카드 행과 한 커밋**, `[B-3d]`)

- 코드가 이름으로 부르는 행은 `Data/CosmicRowNames.h` 상수로만 부르고(규칙 8), 스키마는 그 상수와 `required_rows`를 양쪽으로 견주며 `required_rows`의 행이 데이터에 있기를 요구한다(`Tools/cosmic_data_schema.py:803-817`·`:1188-1190`). 그래서 카드의 길 전부 — `CosmicRows::PetitionFirstContact`·`required_rows`·카드 행·`NoteMet`의 대기 칸 쓰기·`Present`의 먼저 내기·답의 `OtherID`·`ArgsOf`의 `{OtherCiv}`·뷰의 칸·`Install`과 카드의 답의 비움·선택지 아래 우호의 줄·`Sim.FirstContactCard` — 는 카드 행과 한 커밋에 들고, 그 커밋에서 구현 리뷰를 받는다(앎 쓰기·대기 칸의 차례·답의 추첨 — `[B-3b]`의 관점 그대로). 카드가 옮기는 것: 모든 통치자 판의 전체 해시(틱 330의 대기 칸 — 앎), 정책·방랑자 판의 세계(답 하나의 추첨).
- `NoteMet`이 그 판의 첫 만남을 적을 때(`FirstContactCivID`), 몸을 입은 통치자이고 행이 있으면 `Knowing.PendingContactCivID`에 그 백성을 적는다(행이 없는 테이블 — 테스트가 지운 행 — 에서는 적지 않는다).
- 제시: 앎 단계 끝의 `CosmicPetitions::Present`가 **기다리는 청원이 없을 때 첫 접촉을 먼저** 낸다(제 칸이 있어 한 장 기다릴 수 있다 — 첫 접촉만의 예외; 그 밖의 카드는 지금처럼 줄을 세우지 않는다). 추첨·쿨다운·평가 슬롯을 쓰지 않는다. 저장되고 로드 뒤 다시 낸다. `PendingContactCivID`가 비었거나 그 백성이 없으면 카드는 서 있지 않다(`StillStands` — 답은 Withdrawn). 역할이 바뀌면 비운다 — `BecomeWanderer`·즉위는 이미 비우고(`PlayerRoleSubsystem.cpp:296`·`:746`), `Install`에 같은 한 줄을 더한다. `CosmicPetitions::Clear`는 그 칸을 건드리지 않는다(답마다 불리므로 — `:432`·`:479` — 비우게 하면 다른 청원에 답하는 순간 기다리던 첫 접촉 카드가 사라진다); 첫 접촉 카드 자신의 답(받아들인 답·Withdrawn 모두)이 끝에서 그 칸을 직접 비운다.
- 글 인자: `CosmicPetitions::ArgsOf`가 인자 `FName OtherCivID`를 받아 `{OtherCiv}`(그 백성의 이름)를 채운다 — 답(`AnswerPetition`)은 `Knowing.PendingContactCivID`를 넘기고, 카드 창은 `FCosmicKnownView`의 새 값 `PendingContactCivID`(`KnownViewOf`가 `PendingPetitionRow`처럼 옮긴다 — 규칙 14)를 넘긴다.
- 답: 그 카드의 답은 문맥의 `OtherID` = `PendingContactCivID`(명령이 기다리는 카드를 입력의 정의로 읽는다 — 규칙 2). 받아들인 답마다 시뮬 수 하나(있는 규칙). 답한 뒤 `PendingContactCivID`를 비운다.
- 선택 **둘**(판정 없음·한 번 내는 값 없음·화자 없음 — 첫 10분 완화 ①③④): **손을 내민다** → 그 백성을 우호로(효과 `SetStance Stance.Friendly` — 우호의 자리 하나와 틱당 영향력을 쓴다; 선택지 아래 줄이 그것을 말한다 — §4); **지켜본다** → 중립(아무것도 바꾸지 않는다 — 자리를 남긴다). 첫째 칸의 우호 자리는 하나라 둘은 '이 이웃에게 하나뿐인 손을 내미는가'의 선택이다. 첫 판의 셋째 선택 '응답하지 않는다'(우리 쪽 눈 −0.1)는 지웠다 — 다스리는 동안 그 값을 읽는 규칙이 없어 화면에서 '지켜본다'와 같았다. 적대는 카드가 아니라 [백성] 탭에서, 둘째 칸부터.
- 새 효과 열거자 `SetStance`(`EEffectType` 끝 — Tag = `Stance.Friendly` 또는 빈 태그(중립), Key = 상대 선택자(빈 Key = 문맥의 `OtherID`)). `Apply`는 `CosmicDiplomacy::StanceBlockOf`(bMet = true — 세계 코드는 앎을 읽지 않는다)가 None일 때만 쓴다(거절은 무동작 — 효과도 단일 창구를 지난다). 틱당 원천이 아니다(`IsPerTickEffect`에 들지 않는다). 스키마는 `Stance.Friendly`·빈 태그만 받는다(적대는 명령의 문으로만). 스키마·`test_data_schema.py`에 함께.
- `Sim.FirstContactTimeline`의 하한 330 단언: 세계 코드의 `NoteMet`(공성의 줄)은 함대가 없는 첫째 칸에서는 일어나지 않으므로 B3에서는 그대로 둔다.

### 2.10 #73 — 업적은 제 백성의 완성만 센다

- 쓰기(`[B-3a]`): 완성이 새 기록이든 있던 기록이든 `Tick`과 함께 `CivID`를 그 백성으로 고쳐 쓴다(한 성계 한 행에 기록 하나 — 마지막에 완성한 백성).
- 읽기(`[B-3b]`): `CosmicMegaprojects::HasCompleted`가 백성을 받는다 — 기록의 `CivID`가 그 백성일 때만 참. 조건 `MegaprojectComplete`·업적 판정·패널(`UI/CosmicNextRung`)이 같은 함수.
- 옛 기록(`NAME_None` — B3 전 세이브에서만 생긴다: B3 뒤의 모든 완성은 주인을 적는다): `HasCompleted`는 그 성계의 지금 주인의 것으로 읽고, `Transfer`가 주인을 바꾸기 전에 그 성계의 `NAME_None` 기록에 옛 주인을 적는다(정복이 옛 기록을 정복자의 것으로 만들지 않게 — 한 곳, 추첨 없음; 데이터를 끈 판에는 백성 사이의 넘김이 없어 무동작).
- `feat_three_worlds`(정착된 성계 셋 ∧ 공학 1.0)는 성계의 수라 정복한 성계도 센다(정복은 셋째 성계를 채우는 길이다 — 정착 600틱 없이). 그래서 정착된 성계 둘과 공학 1.0을 가진 둘째 칸 통치자가 성계 하나를 정복하면 그 틱에 셋째 칸에 오르고 판이 끝난다(결정 11 — 측정한다; 업적의 줄이 정복 뒤에도 참인지는 글 감사가 본다). 둘째 칸의 보유 상한은 3이라 셋째 정착이 돌고 있으면 공성은 `RealmCap`으로 막힌다 — 정복의 창은 정확히 둘을 가졌을 때다.

### 2.11 결정론과 저장 — 한눈에

| 입력·단계 | 시뮬 스트림 | 말하기 스트림 | 앎에 쓰기 | 저장 |
|---|---|---|---|---|
| `SetStance`·`ProposeTreaty`·`BreakTreaty` | — | 줄 1 | — | 다음 경계의 정해진 까닭 |
| `AllocateFleet` | — | 공성을 시작하면 줄 1 | — | 같다 |
| 외교 단계(유지비·표류) | — | 그침의 줄(통치자) | — | — |
| 분쟁 단계(함대·점검) | — | 풀림의 줄(통치자 당사자) | 풀림의 줄 전에 상대 `NoteMet` | — |
| 분쟁 단계(공성의 끝) | **끝에 이른 공성마다 1(먼저 모두)** | 다스리는 백성이 당사자면 줄 1 | 상대 `NoteMet`, 받은 성계 `FollowUpKnowing` | — |
| 위기 단계(지연·번짐) | 그대로(몫만) | — | — | — |
| AI 단계(적대·공성) | — | 다스리는 백성을 노리면 줄 1 | 상대 `NoteMet` | — |
| 첫 접촉 카드 제시 | — | — | 대기 칸 | — |
| 첫 접촉 카드 답 | 답마다 1(있는 규칙) | 결과 줄 | 대기 칸 비움 | 답의 저장 |

- **새 저장 필드는 넷**(§2.1), 새 UENUM 없음(태세·조약은 태그), 새 조건 열거자 없음, 새 효과 열거자 하나(`SetStance`). 거절 사유 넷은 저장되지 않는다.
- 세계 코드의 `NoteMet`은 다스리는 백성이 당사자인 **말해지는** 일에만, 줄 전에(`docs/70:43`). 소리 없는 표류는 만남을 낳지 않는다(첫 이웃은 틱 0부터 국경이 닿아 있다 — 표류를 '관계 변화'로 읽으면 첫 접촉이 틱 1로 당겨진다).
- 해시: `[B-3a]`(필드와 완성의 쓰기)에서 다섯 기준값의 해시만 옮고 커맨드렛 CSV의 결정적 칸(시간·세이브 크기 밖)과 요약 줄의 해시 밖 수는 비트 그대로. `[B-3a]`의 증명은 CSV의 결정적 칸(시간 `TickMicros`와 세이브 크기 `SaveBytes`·`SaveRawBytes`를 뺀 칸 — A0 4a의 꼴; 세이브 크기는 새 필드와 완성의 주인만큼 는다)으로 한다. `[B-3b]`(장치, 데이터 끔 — 표류율 0·함대 곱 0·번짐 1, 격리령 행 없음, 카드의 길 없음)는 `[B-3a]`와 비트 그대로: 함대가 0이라 공성·지연·소모가 없고, 표류가 항목을 만들지 않으며, 백성 사이의 넘김이 없어 `Transfer`의 옛 기록 쓰기와 #73의 읽기가 같은 답을 낸다. `[B-3d]`가 켜고 다시 적는다.
- **문서 먼저**(`[B-3a]`): `docs/21` §7.4(외교·분쟁 단계의 차례와 추첨 지점, 위기 단계의 두 고리, AI 단계의 일, 건넘 정리의 `Ties`, `NoteMet`의 대기 칸)·`:433`(`PendingContactCivID` — '사용은 Phase 4'를 지금의 쓰기·비우기·읽기로)·`:513`(`Civs` — 제거할 때 `Ties`의 키도 지운다), `docs/20`(`:198` 추첨 지점·`:273` 앎의 목록 — `NoteMet`이 첫 만남에 `PendingContactCivID`를 적고(통치자, 행이 있을 때), `Install`·첫 접촉 카드의 답이 그 칸을 비우며, 외교 명령(`SetStance`·`ProposeTreaty`)은 만난 백성을, 답 명령은 첫 접촉 카드의 대기 칸을 입력의 정의로 읽고, 앎 단계 끝의 제시가 기다리는 청원이 없을 때 그 칸을 읽는다; 끝의 v0.7 표지를 지운다·`:274` 부록 규칙 4·13 — `SiegeBlockOf`·`GuardBlockOf`·`StanceBlockOf`·`TreatyBlockOf`가 단일 창구), `docs/70`(`:45` ③④·`:61`의 '입력 없는 판'), `CLAUDE.md` 규칙 4·13의 같은 줄.

## 3. 데이터

### 3.1 규칙 노브 (`FCosmicRulesDef` 끝에 덧붙임 — 값·스키마의 `rules` 표·`test_data_schema.py`·설정 표와 함께)

C++ 초기값(`FCosmicRulesDef`)은 **장치 값**과 같다(B2의 `SocietyRecoveryBonus = 0`과 같은 꼴 — `DT_Rules` 행이 없는 판(`SetTables`의 되돌이)과 `FCosmicRulesDef`를 기본으로 만드는 테스트가 C++ 값으로 돈다; 커맨드렛의 `-Rules=`는 게임의 `DT_Rules` 행에서 시작해 이름 붙은 칸만 바꾸므로 `[B-3d]` 뒤에는 밸런스 값에서 출발한다); `[B-3d]`는 JSON만 바꾼다.

| 노브 | 장치 값 | 밸런스 값(잠정) | 뜻 |
|---|---|---|---|
| `RelationDriftRate` | 0 | 0.002 | 표류의 걸음(목표까지 남은 거리의 몫/틱 — 약 500틱의 시간 상수) |
| `RelationEthicsWeight` | 0.6 | 0.6 | 윤리 거리(0~2)의 곱 — AI끼리 고른 윤리면 목표의 중앙값 약 −0.4 |
| `RelationStanceTerms` | {0.4, −0.4} | {0.4, −0.4} | 그 상대가 이 백성을 대하는 태세 — 우호·적대 |
| `RelationTreatyTerm` | 0.2 | 0.2 | 조약 하나마다 |
| `ConquestRelationPenalty` | 0.2 | 0.2 | 정복한 백성을 보는 이웃들의 눈(곧) |
| `BetrayalRelationPenalty` | 0.3 | 0.3 | 조약을 끊긴 쪽이 끊은 쪽을 보는 눈(곧) |
| `FriendlyUpkeepInfluence` | 0.2 | 0.2 | 우호 하나의 틱당 영향력 |
| `FriendlySlotsByTier` | {1, 1, 2, 2, 3} | {1, 1, 2, 2, 3} | 칸마다 우호의 자리 — 첫 생애 백성의 윤리가 가운데라 우호를 건 이웃의 목표는 −0.24 이상(적대의 문 −0.3 위)이다: 자리 하나가 이웃 하나와의 평화이고, 둘째 칸의 자리는 분쟁 창의 국경 이웃(평균 약 2.3) 가운데 하나만 덮는다 |
| `TreatyCostInfluence` | 50 | 50 | 조약 제안의 값(한 번) |
| `TreatyAcceptRelation` | −0.1 | −0.1 | 승낙: `ViewOf`(그 백성, 제안한 백성) ≥ 이 값 — 우호를 건 중앙값 이웃(목표 약 +0.1)은 첫 접촉 뒤 약 100틱에 닿아 둘째 칸에는 이미 넘는다(첫 조약을 정하는 것은 문턱이 아니라 칸의 문이다); 우호 없는 중앙값 이웃(목표 −0.3)은 둘째 칸에 약 −0.27이라 무역 자유화(−0.3)를 내려야 받는다. 목표값 이상인 문턱에는 영원히 닿지 않는다 |
| `TradeInfluencePerTreaty` | 0.2 | 0.2 | 무역 조약 하나의 틱당 영향력 수입 |
| `FleetPerSettledByTier` | {0, 0, 0, 0, 0} | {0, 1, 1, 1, 1} | 칸마다 정착된 성계 하나의 함대 상한 — 장치 값이 0이라 아무도 함대가 없다 |
| `FleetGrowthPerTick` | 0.001 | 0.001 | 자람 |
| `FleetUpkeepEnergyPerPower` | 0.5 | 0.5 | 함대 1.0의 틱당 에너지 |
| `SiegeTicks` | 200 | 200 | 공성의 길이(최소 1) |
| `SiegeRestTicks` | 600 | 600 | AI가 공성이 끝난 뒤 쉬는 틱 |
| `SiegeDefenceShares` | {1, 0.5, 0.25} | {1, 0.5, 0.25} | 지키는 몫 — 목표가 그 성계·목표 없음·다른 성계 |
| `SiegeRepulsedFleetFactor` | 0.5 | 0.5 | 진 공격의 함대에 곱한다 |
| `AIHostileRelation` | −0.3 | −0.3 | AI가 적대를 여는 눈 |
| `AIHostilePowerRatio` | 1.5 | 1.5 | AI가 적대를 여는 전력의 비 |
| `AIHostilePacifismMax` | 0.5 | 0.5 | 평화 축이 이 값 이상인 AI는 적대를 열지 않는다 |
| `PlagueSpreadMultiplier` | 1 | 8 | 이웃에 역병이 선 성계의 역병 몫의 곱(1 = 번짐 없음; 모형에서 10은 역병 ×1.5, 20은 ×2.5, 40은 ×8.7 — 20 근처에 절벽이 있다; 5~10에서 맞춘다) |

- 함대 노브 넷이 읽힌다: `FleetEffectivenessByTierGap`(공격의 유효 전력·지연의 유효 전력), `PressureFleetAttrition`(영향권 안의 소모), `FleetDelayPerPower`·`FleetDelayCap`(놓일 때의 지연 — 스키마에 최소 0을 더한다). `check_layers.py`의 '읽는 코드 없는 노브'가 17 → 13(기준선을 날짜 메모와 함께 내린다).
- 상수(노브가 아니다 — 코드 주석과 `docs/21`): 낱말 띠의 경계, 화면 띠의 경계(0.6·0.4).
- 노브를 더하고 값을 바꾸는 커밋마다 `python -I Tools/make_config_reference.py`로 `docs/30`을 다시 만든다.
- 데이터 무결성(`Data.Integrity`): `RelationStanceTerms`는 정확히 2, `FleetPerSettledByTier`·`FriendlySlotsByTier`는 정확히 5, `SiegeDefenceShares`는 정확히 3, 범위는 스키마.

### 3.2 칙령 (`Tools/data/DT_Edicts.json` — 행 이름은 저장에 남으므로 승인 뒤 고정; 값은 잠정)

- **격리령** `edict_quarantine`(여섯째 — 칸 1, 조건 없음): 제정 영향력 40, 유지비 없음, 틱당 효과 모든 수입 ×0.9(항구를 닫는다), `HazardSpawnMultipliers {Hazard.Plague: 0.1}`. 글은 사람의 조치로만(닫힘·기다림 — '역병이 넘어갔다'는 쓰지 않는다: 은혜의 모티프, `docs/70:127`), 청원 '역병'의 '성문을 닫는다'와 겹치지 않게(카드 = 한 번의 답, 칙령 = 서 있는 법). 신앙 어휘 아님.
- `FEdictDef` 끝에 열 둘(정의 데이터 — 저장 변화 0): `float FleetGrowthMultiplier = 1`(함대 자람의 곱 — 지닌 칙령마다 곱한다; 0보다 크게) — **전시 동원 1.5**(B2가 넘긴 함대 효과. 전시 동원은 물려받은 칙령(`DT_Rules.StartingEdict`)이라 통치자는 틱 0부터 지니고 AI는 첫 AI 단계에서 철회한다 — 자람 ×1.5는 통치자의 기본값이 되어 AI가 통치자에게 여는 전력의 문(×`AIHostilePowerRatio`)과 지킴의 몫을 함께 옮긴다; 값은 측정 2를 1.0과 1.5의 짝으로 잰 뒤 정한다); `float TreatyAcceptBonus = 0`(조약 승낙 문턱을 낮춘다 — 지닌 칙령의 합; 0 이상) — **무역 자유화 0.2**(B2가 넘긴 조약 효과).
- 칙령 창의 툴팁은 새 두 열을 위기 고리의 두 칸처럼 한 줄씩 보인다(`Edict.Effect.FleetGrowth` '함대의 자람 ×{0}'·`Edict.Effect.TreatyAccept` '조약을 받아들이는 문턱 −{0}' — 수는 UI에만).
- 전시 동원·무역 자유화의 한 줄 설명은 '어느 때에나 참인 효과의 말'(B2)이라 새 효과를 담게 고친다(같은 자리 — 키·해시 그대로).
- 평화 성향의 흐름(B2가 넘긴 것)은 B7로 다시 미룬다 — B2의 까닭(모든 백성이 물려받는 칙령이 윤리를 옮긴다)이 그대로다.

### 3.3 카드 (`DT_Encounters` — 청원 11장째)

- `petition_first_contact`: `Named`, 화자 없음(두 목소리 변형 없음 — `docs/50:180`), 판정 없음, 비용 없음, 선택 둘(§2.9). 결과 줄은 `log_first_contact`('{Civ}의 신호가 처음으로 닿았다. 하늘은 더 이상 비어 있지 않다.')를 되풀이하지 않는다. Tier A 어휘, 영적 낱말 없음(첫 10분 완화 ①③). 카드의 제목·몸·선택지·결과는 상대 백성을 `{OtherCiv}`로 부른다.
- 상수 `CosmicRows::PetitionFirstContact`(`Data/CosmicRowNames.h`)와 스키마의 `required_rows` — 카드 행과 한 커밋(§2.9).

### 3.4 로그 행 (`DT_LogTemplates` — 변형 셋씩; 다스리는 백성이 당사자인 줄만 — AI끼리의 분쟁은 말하지 않는다)

| 행 | 태그·등급 | 언제 | 연대기 |
|---|---|---|---|
| `log_stance_changed` | `Log.Civ`, 1 | 통치자가 태세를 바꿈(`{OtherCiv}`·`{Stance}`) | 받지 않는다(칙령의 줄과 같은 장부) |
| `log_stance_lapsed` | `Log.Civ`, 1 | 우호의 유지비를 내지 못해 그침 | 받지 않는다 |
| `log_treaty_signed`·`log_treaty_broken` | `Log.Civ`, 1 | 조약 맺음·끊음(`{OtherCiv}`·`{Treaty}`) | 받는다 |
| `log_siege_begun` | `Log.Conflict`, 1 | 다스리는 백성이 명령으로 에워쌈(`{System}`·`{OtherCiv}`) — 자동 정지하지 않는다 | 받는다 |
| `log_siege_on_us` | `Log.Conflict`, 2 | 다른 백성이 다스리는 백성의 성계를 에워쌈 | 받는다 |
| `log_siege_taken` | `Log.Conflict`, 2 | 다스리는 백성이 차지함 | 받는다 |
| `log_siege_lost` | `Log.Conflict`, 2 | 다스리는 백성의 성계가 넘어감 | 받는다 |
| `log_siege_held` | `Log.Conflict`, 2 | 다스리는 백성이 지켜 냄 | 받는다 |
| `log_siege_failed` | `Log.Conflict`, 2 | 다스리는 백성의 공성이 서지 못함 | 받는다 |
| `log_siege_lifted` | `Log.Conflict`, 1 | 다스리는 백성이 당사자인 공성이 문을 잃어 풀림(글은 쪽을 말하지 않는다) | 받는다 |

- 주제 백성(`FocusCivID`)은 늘 다스리는 백성이고 상대는 인자 `{OtherCiv}`(#31 ②의 기본값 — 항목에 둘째 백성을 남기지 않는다). 다스리는 백성이 당사자인 줄은 상대를 먼저 만나므로 둘째 백성이 보이지 않는 백성일 일이 없다(격차 ≤ 2 — B3의 AI 최고 칸은 T3). 쪽마다 행이 달라 '공격한 쪽을 능동의 주어로, 진 쪽의 애도로 끝내기'를 한 틀이 두 쪽에 쓰지 않는다.
- 새 슬롯 `{Stance}`·`{Treaty}`(태그의 표시 이름 — 스트링 테이블에서), `{OtherCiv}`는 로그 틀의 슬롯 목록에 이미 있다(`schema.json:240`) — 줄의 인자로 처음 채운다.
- 상수 `CosmicRows::Log…` **열하나**(표의 행 전부)와 `required_rows`. `CosmicChronicle::Admits`가 `log_stance_changed`·`log_stance_lapsed`를 받지 않는다(칙령 셋 곁).
- **글의 규칙**(`docs/70` §8.3에 '외교와 분쟁의 줄' 문단으로): 정복은 백성이 한 일로 능동태로(수동태 '넘겨졌다'는 판결로 읽힌다); 승리의 기쁨을 쓰지 않는다 — 넘어간 성계의 줄도 진 쪽의 애도로 끝낸다; 진 백성을 악이나 벌로 쓰지 않는다; 승리를 은혜·응답으로 쓰지 않는다; '가져갔다'는 타락의 입에만(`docs/70:166` — 서술자는 '차지했다'); 사람·배의 수 없음(규칙 11 — '세 척' 같은 낱말 수도); '구원'은 군사 낱말로도 쓰지 않는다('구원군' 따위); 칭호 없음('정복자'); 평화 축은 덕의 눈금이 아니다('의로운 백성' 꼴 없음); 이웃의 믿음은 '그들이 조상이라 부르는 것' 꼴; 함대·공성·적대에 '거두다'를 쓰지 않는다(영혼 경제의 동사 — 함대는 보내고·물리고·지키며, 공성은 서고·풀린다). 판정 어휘·`soul_verbs`·`holy_words`는 스키마가 막는다(로그 틀에 `holy_words`를 더한다 — §3.6).

### 3.5 스트링 테이블 (`ST_CA.csv` — 끝에)

[백성] 탭의 이름(짧게 '백성')·빈 목록의 말('아직 만난 백성이 없다.'), 낱말 띠 다섯(`Diplomacy.View.*`), 태세 셋과 조약 둘의 이름(`Stance.*`·`Treaty.*`), 단추([우호]·[중립]·[적대]·[무역]·[불가침]·맺은 조약은 [무역을 끊는다]·[불가침을 끊는다] — 청한다는 말·값·문턱·사유는 툴팁), 사유(`Diplomacy.Block.*`·`Siege.Block.*`·`Guard.Block.*`), 툴팁의 틀(관계의 항 — 윤리·태세·조약; 승낙의 문턱; 값; 우호의 자리와 유지비), 함대 줄('함대 {0} · 상한 {1}' — 수는 UI에만)·목표 줄('함대가 지킨다'·'함대가 에워싸고 있다 — 남은 시간 {0}'·우리 쪽 몫의 띠 셋)·[함대를 여기에]·[함대를 보낸다]·[함대를 물린다], 위기 패널의 '함대가 지키는 성계에 새로 서는 위기는 남은 시간이 는다'와 그 툴팁('반란과 두 칸 이상 높은 원천의 주장은 늦추지 않는다'), 이름 모를 공성의 줄('이름 모를 함대가 에워싸고 있다'), 첫 접촉 카드의 우호 줄 `Petition.FriendlyUpkeep`('우호를 지키는 동안 영향력이 {1}마다 {0} 든다 — 우호의 자리 {2}/{3}. [백성] 탭에서 중립으로 바꿀 수 있다.' — 수는 UI에만), 칙령 툴팁의 두 줄(§3.2), 상단 바 칙령 단추 툴팁의 '만난 백성과의 태세·조약'. 함대의 키는 `Siege.`·`Guard.` 아래에 둔다. 접두어 `Diplomacy.`·`Siege.`·`Guard.`·`Stance.`·`Treaty.`를 스키마의 검사 목록(`checked_prefixes` — 판정 어휘·`holy_words`·`soul_verbs`·숫자 금지)에 더한다.

### 3.6 스키마 (`Tools/data/schema.json`·`Tools/cosmic_data_schema.py`·`Tools/test_data_schema.py` — 새 규칙마다 깨지는 경우 하나)

- 노브 스물둘의 범위와 길이(`SiegeTicks` 최소 1, `FleetDelayPerPower`·`FleetDelayCap` 최소 0), `FEdictDef`의 새 열 둘, 효과 `SetStance`(Tag = `Stance.Friendly` 또는 빈 태그, 틱당 원천 아님 — `per_tick` 칸에서 막는다; 청원 카드의 효과에만), `required_rows`의 새 상수, 로그 행의 새 슬롯, ST_CA 접두어 다섯.
- `DT_Encounters`의 글 칸(Title·Body·VoiceBodies·Choices.Label·SuccessText·FailureText)의 `slots`와 주제 Civilization의 `slots_in`에 `OtherCiv`를 더하고, 새 규칙 '선택지의 효과에 `SetStance`가 없으면 그 여섯 칸의 `slots_in`은 `OtherCiv`를 뺀 목록'을 둔다(상대에 대한 효과를 지닌 카드만 상대를 부른다 — 지금 검사기의 행 시험에는 '이 행 밖'이 없어, 열 시험의 닫힌 목록에 효과 종류의 부정형 하나를 더한다; B1이 `has_cond`를 더한 꼴). `test_data_schema.py`에 깨지는 경우 하나(`SetStance` 없는 카드의 `{OtherCiv}`).
- `DT_LogTemplates.Templates`의 `forbidden`에 `holy_words`를 더한다(지금 행에서 걸리는 낱말 0).
- 태그 목록(`Stance.*`·`Treaty.*`)은 `DefaultGameplayTags.ini`와 `Data.Integrity`가 견준다.

## 4. 표현 계층

- **칙령 창의 셋째 탭 [백성]**(`UI/CosmicEdictWidget` — 새 창을 만들지 않는다; 상단 바 단추 '칙령 n/m'은 그대로, 툴팁에 '만난 백성과의 태세·조약'을 더한다). 창의 안쪽 너비는 312단위(340 − 좌우 14)다 — 탭 줄([칙령][연구][백성][닫기] — 탭 이름을 짧게)과 행이 그 안에 들어야 한다. 머리 줄 '함대 {전력} · 상한 {상한}'과 지금 목표 한 줄([함대를 물린다]), 우호의 자리 '우호 {n}/{m}'. 만난 백성마다(만난 차례 — `FCosmicKnownView::ShownOrderOf`, 규칙 14) **세 줄의 행**: ① 이름 · '그들이 우리를 보는 눈'의 낱말 띠(툴팁에 항 — 윤리·태세·조약과 수) · 그들의 태세가 적대면 그 낱말 ② 우리의 태세 단추 셋(지금의 것은 흐리게, 회색이면 `StanceBlockOf`의 첫 사유) ③ 조약 단추 둘(회색이면 `TreatyBlockOf`의 사유, 툴팁에 값·승낙의 문턱) — 그 백성과의 공성이 서 있으면 한 줄 더. 한 백성 약 90단위, 680을 넘으면 굴린다(B2의 규칙). 초월자는 목록에 없다(관계를 수나 태세로 보이면 '신의 호감 미터'가 된다 — `crisis_and_transcendents.md:104`).
- **성계 패널**: 다스리는 백성의 정착된 성계 — '함대가 지킨다' 또는 [함대를 여기에](회색 사유 `GuardBlockOf` — 첫째 칸은 '아직 함대가 없다'); 그 성계에 공성이 서 있으면 '{OtherCiv}의 함대가 에워싸고 있다 — 남은 시간 · 우리 쪽 우세/팽팽/열세'(에워싼 백성이 보이는 백성 — `FCosmicKnownView::ShownOrderOf`가 `INDEX_NONE`이 아닐 때만 이름으로, 아니면 '이름 모를 함대가 에워싸고 있다': 즉위가 물려받은, 그 백성을 노리는 공성 — 그 백성은 다음 시야나 공성의 끝에서 만난다). 다른 백성의 정착된 성계(보이는 주인 — 규칙 14) — [함대를 보낸다](회색이면 `SiegeBlockOf`의 첫 사유 — 구조의 사유가 먼저, 다른 길이 모두 열렸을 때만 '먼저 적대해야 한다'), 다스리는 백성이 에워싸고 있으면 공성 줄(다른 백성끼리의 공성은 줄도 띠도 없다 — 결정 18; 공격한 백성을 만나지 않았을 수 있다; [함대를 보낸다]의 회색 사유 `Besieged`가 공성이 서 있다는 것만 알린다). 위기 줄 아래 '함대가 지키는 성계에 새로 서는 위기는 남은 시간이 는다'(지킬 때만, 그리고 `FleetDelayPerPower` × 그 백성의 `FleetPower`가 1 이상일 때만 — 툴팁에 '반란과 두 칸 이상 높은 원천의 주장은 늦추지 않는다'; 영향권 안이면 소모의 툴팁 — `ComputeAttritionPressureIn`).
- 첫 접촉 카드 창: 효과 `SetStance Stance.Friendly`를 지닌 선택지 아래 줄(비용·판정의 줄)에 `Petition.FriendlyUpkeep`을 더한다(`[B-3d]` — 카드의 길과 함께).
- 컨트롤러: 명령 넷(끝난 판은 거절), 창의 탭 열기, 개발용 콘솔 셋.
- 첫 10분: 첫 접촉 카드 하나. [백성] 탭은 첫 만남 뒤에 행이 생긴다; 안내 줄은 더하지 않는다(카드가 첫 태세를 고르게 한다 — 손 확인이 보고 정한다).
- 지도의 공성 표지는 B3 밖(패널 줄과 Severity 2 줄·자동 정지로 읽힌다).

## 5. 커맨드렛과 측정

- **외교 정책**(`World/CosmicRulerPolicy` — `int32 Diplomacy`(0 끔 · 1 평화 · 2 공격 — `EdictRow` 뒤 끝에; 집합 초기화 일곱 곳은 0 또는 1), 인자 `-Diplomacy=0|1|2`): `Act`의 끝(칙령 뒤), 판이 끝나지 않았을 때, 한 경계에 하나: ① 국경이 닿은 만난 백성에게(`Civs` 차례) 우호를 건다(자리가 남을 때) ② 둘째 칸부터, 국경이 닿은 만난 백성 가운데 `TreatyBlockOf`가 None인 첫 백성에게 불가침, 그다음 무역을 청한다 ③ 다스리는 백성의 성계를 노리는 공성이 서 있으면 함대 목표를 그 성계로(지킴), 공성이 끝나면 물린다 ④ 적대하지 않는다(평화의 정책). 첫 접촉 카드는 `-Answer`의 답 고리가 답한다 — `ChoiceOf`는 답한 수의 나머지로 선택을 돌리므로(`World/CosmicRulerPolicy.cpp`) 그대로 두면 판마다 두 선택이 섞인다: `Diplomacy`가 0이 아닌 판에서는 `ChoiceOf`가 `CosmicRows::PetitionFirstContact`에만 첫 선택(손을 내민다)을 고른다(그 답도 `Answers`에 센다; 0은 지금 차례 그대로). 결정의 셈에 외교의 바꿈(태세·조약·함대 목표)을 더한다. 측정 전용 **공격 정책** `-Diplomacy=2`(기준값 아님): 둘째 칸부터, 국경이 닿은 만난 백성 가운데 공성할 성계가 있는 첫 백성에게 적대하고 함대를 보낸다. 집합 초기화 일곱 곳(`Tests/CosmicBalanceCommandlet.cpp`, `World/CosmicHashRun.cpp`, `Tests/CosmicHashRunTests.cpp`, `Tests/CosmicKnowingTests.cpp`, `Tests/CosmicWandererTests.cpp`, `Tests/CosmicEdictTests.cpp` 두 곳)에 새 값.
- **새 요약 줄**(기준값의 키 밖, 언제나): `CosmicBalance diplomacy: border pairs a/b/c/d (1000/3000/6000/end), friendly F hostile H, treaties trade T nonaggression N (signed S broken B), relation mean M; ruler stance changes X, treaties proposed to borderers P1 refused R1, to others P2 refused R2; first contact card at T; ruler window: second settled at A, first siege target at B, end at C` · `CosmicBalance conflict: sieges begun N (on ruled R by end Re), taken T (from ruled L by end Le), held H, lifted X; hostilities opened O (by tick bands 0-3000/3000-6000/6000-20000); AIs hostile toward one people max K, sieges on one people max S; taken by top conqueror M, lost by one people max Q; fleet median [..] at 3000/6000; delay bought per tag [..]; attrition A, AI sieges into a higher sphere Z; feat_three_worlds by conquest F3; plague placed P (spread-placed Ps), quarantine held Qh; ruled severity-2 conflict lines V (by end Ve, max Vm)`. CSV 끝에 열 `FleetPower`·`Hostile`·`Treaties`·`BorderPeoples`. **단계마다의 시간**(외교·분쟁·위기·AI의 평균 µs — `docs/50:182`).
- **기준값**: 통치자 정책의 기준 판(3,000·20,000틱·방랑자 판)은 밸런스 커밋부터 `-Diplomacy=1` — 러너의 세 정책 인자 배열(`Tools/run_checks.ps1:119`·`:124`·`:130`)과 머리 주석, 해시 판의 고정 정책(`World/CosmicHashRun.cpp:123`)에 함께(둘이 함께 바뀌지 않으면 '해시 판 대 커맨드렛' 단계가 깨진다). 통치자 줄에 'diplomacy N'(정책 값)과 'diplomacy changes N'(B2의 'edict 1'·'edict changes N' 꼴). 다섯 줄을 `[B-3a]`(해시만)와 `[B-3d]`(데이터)에서 다시 적고, 옛 해시와 인자의 바뀜은 `seed42.json`의 `note`에.
- **판정의 잣대**: 데이터를 끈 같은 시드의 판과 **짝 차이**로 본다(B2 결정 17 — 60판 짝 차이가 표준오차 안이면 '나빠지지 않음'). 몫만 보는 가드는 분모가 바뀌는 B3에서 쓰지 않는다 — 판당 수를 함께 적는다.
- **잴 것**(장치 커밋 뒤 데이터를 끈 판을 같은 표로 먼저 잰다 — 국경 쌍의 수는 그때 처음 잰다):
  1. **세계**(입력 없는 20,000틱, 시드 1~60): 백성 사이의 넘김 — 판마다 중앙값 3~10, 2/3 이상의 판에서 ≥ 1(지금 0), 시간 분포(0~3,000·3,000~6,000·6,000~20,000 — 6,000 뒤가 0이면 '후반이 언다'를 결정으로 받거나 고친다); 눈덩이 — 한 백성이 얻은 넘김의 최대 ≤ 3, 가장 많이 얻은 백성의 몫 ≤ 50%, 한 백성이 잃은 성계의 최대 ≤ 2; 몰매 — 한 백성에게 동시에 적대한 AI의 최대 1, 한 백성을 동시에 에워싼 백성의 최대 1; 살아 있는 백성 12(설계의 확인 — 목표가 아니다); 끝의 안정도 0.3 미만 — 짝 차이 +2점 이하이거나 그 표준오차 안; 반란 — 판당 발생 수의 짝 차이 ≤ +5%, 역병을 뺀 위기 가운데 반란의 몫 ≤ 지금(50.5% — 14,387/28,475) + 2; 주장 — 발생 수 −20% 안·편입률을 적는다; AI T2·T3 중앙값 1,400·2,700의 +20% 안; 역병 — 판당 발생 ≤ 지금(약 71)의 1.5배, 번져 선 몫 20~40%, 만료 몫; 무역 자유화 행(`-EdictRow=edict_free_trade`)의 다스리는 백성 역병 수를 따로 적는다(그 행은 역병 ×2라 번짐의 실효 곱이 16이다); 영향권으로 들어간 AI 공성 0; 단계 시간 — 외교 + 분쟁 < 틱의 10%; 국경 쌍(1,000·3,000·6,000·끝 — 모형 5·14·17과 견줌).
  2. **통치자 정책**(`-Answer=1 -Build=1 -Focus=1 -Edict=1 -Diplomacy=1 -TellAll=0`, 6,000틱, 시드 1~30 — 셈은 **판의 끝까지**(정책의 `bEnded` 전)와 6,000틱 전체를 따로): 판의 끝까지 결정 ≥ 15·첫 600틱 말해진 줄 ≥ 20(지금 20.5·28); 첫 접촉 카드의 틱 ≤ 600; T2·T3 +20% 안; 폐위 ≤ 2/30; 끝 전에 AI가 다스리는 백성을 에워싼 판의 몫(목표 10~30%는 `-Diplomacy=0`의 같은 판에 건다 — `-Diplomacy=1`은 적기만 한다: 우호와 불가침이 닫은 문은 평화의 길이 낸 결과다), 잃은 성계의 수, 그 둘을 전시 동원의 `FleetGrowthMultiplier` 1.0과 1.5로 짝 지어(물려받은 칙령의 함대 곱이 통치자의 기본값이다); 다스리는 백성이 받은 `Log.Conflict` Severity 2 줄의 수 — 판의 끝까지와 6,000틱 전체(판마다 중앙값·최대, 잠정 목표 6,000틱에 최대 8)와 생애 장 40줄 가운데 그 줄의 몫; 첫 접촉에서 손을 내민 이웃과 둘째 칸 뒤 500틱 안에 불가침을 맺은 판의 몫 ≥ 1/2(그 답이 실제로 우호를 건 판만 센다); 통치자의 분쟁 창(A·B·C — B < C인 판의 몫 ≥ 2/3).
  3. **방치**(입력 없는 6,000틱 30판): 폐위 ≥ 27/30, 첫 반란 849·폐위 1,050의 ±20%.
  4. **칙령 행마다**(B2의 표 + 격리령): 한 행이 T2·T3를 모두 이기고 잃는 것이 없으면 다시 맞춘다. 전시 동원의 함대 효과는 평화의 정책에서도 보인다(측정 2의 짝) — 공격 쪽의 차이는 6에서.
  5. **B1 방랑자**(`-Abdicate=1200 -Wander=1 -Edict=1 -Diplomacy=1`, **시드 1~60**): 즉위·복귀 ≥ 44/60(지금 46/60 = 77% — B2가 받은 잡음 ±5점의 아래 끝), 즉위까지 중앙값 ≤ 1,800. 30판 24/30은 쓰지 않는다(77%에서 24/30 이상일 확률은 약 0.45라 흐름만 옮겨도 반은 떨어진다). 44/60 아래면 길별 수(복귀·경외·공백)를 B2의 13·3·8과 견주어 B3 장치가 줄인 길을 찾는다; 그런 길이 없으면 잡음으로 받는다(B2 결정 17의 꼴). `-Diplomacy=1`을 뺀 같은 60판도 함께 적는다. **밸런스 커밋에서 먼저 잰다.**
  6. **공격 정책**(`-Diplomacy=2`, 6,000틱, 시드 1~30, 칙령 행은 전시 동원·기계 권리): 통치자의 분쟁 창, 공성·넘김·지킴, 정복으로 채운 `feat_three_worlds`의 수와 T3가 당겨진 틱(±20%), 전시 동원 행과 기계 권리 행의 넘김 차.
- **맞추는 차례**: 적대의 문턱(`AIHostileRelation`·`AIHostilePowerRatio`), `RelationEthicsWeight`, `FleetPerSettledByTier`, `FleetGrowthPerTick`, `PlagueSpreadMultiplier`, `SiegeDefenceShares`·`SiegeRepulsedFleetFactor`를 `-Rules=`로 한 표에(데이터를 고치지 않고). 칙령의 두 열(전시 동원 1.5·무역 자유화 0.2)과 격리령 값은 `DT_Edicts`를 고치고 `make_data_tables.py`로 다시 만든 뒤 `-EdictRow` 표로 잰다.
- 해시 판의 방랑자 판과 패키지의 ⓓ는 마일스톤 B의 끝에서(B1·B2와 같다).

## 6. 테스트 (새 단언은 변이로 한 번씩 깨뜨린다 — 패치는 `Tools/mutations/B3/`, README에 한 줄씩)

| 테스트 | 단언 | 깨뜨릴 변이 |
|---|---|---|
| `Sim.SaveTies` (새, `[B-3a]`) | 짝 기록·함대 목표·완성의 주인을 채운 판의 저장 → 로드 → 같은 전체 해시; `Sim.SaveRoundTrip`의 채움에 새 필드; 완성이 `CivID`를 적는다 — 새 기록에, 그리고 `CivID`를 미리 `NAME_None`과 다른 백성으로 심어 둔 있던 기록을 다시 완성하면 지금 주인으로 바뀐다(넘김이 없는 `[B-3a]`에서 이 심기 없이는 변이가 살아남는다) | 로드 뒤 비운다; 있던 기록에서 `CivID`를 고치지 않는다 |
| `Sim.DiplomacyRules` (새) | `Borders`(정착된 성계만·같은 은하 행·성역 빠짐), 윤리 거리(상수), 목표값(항마다 상수), 정규형(중립 ∧ 조약 없음이면 항목이 없다), `Sign`·`Break`가 두 백성을 함께 쓴다, `ForgetCiv`, 사유의 차례(상태를 하나씩 막으며 첫 사유가 바뀐다 — 우호의 자리 포함), 국경 밖 쌍의 띠와 승낙이 같은 값(`ViewOf` — 태세의 항을 뺀 목표값, 우호를 걸어도 그대로), 낱말 띠의 경계, 적대가 서면 무역 조약이 두 쪽에서 함께 사라진다 | 한쪽만 쓴다; 기본 항목을 남긴다; 사유 차례를 바꾼다; 국경 밖도 저장된 값으로; 적대가 무역을 끊지 않는다 |
| `Sim.DiplomacyStage` (새) | 표류는 국경이 닿은 쌍만·목표 쪽으로·[−1, 1]; 표류율 0이면 항목이 생기지 않는다(전체 해시 그대로); 우호의 유지비와 그침(통치자는 줄, AI는 없음)·자리보다 많은 우호의 그침; 무역 영향력이 다음 틱의 수입에; 초월자 키는 움직이지 않는다; 사라진 백성의 짝 기록이 남지 않는다(다음 틱의 무역 영향력 0); 추첨 없음 | 국경 밖도 표류; 0에서도 항목을 만든다; 유지비를 그치지 않는다; 정리에서 `Ties`를 지우지 않는다 |
| `Sim.DiplomacyCommands` (새) | 명령 넷의 사유마다 거절하고 상태·스트림·해시가 그대로(만나지 않음·우호의 자리·불가침 중의 적대·둘째 칸 전의 적대와 조약·값·문턱, 첫째 칸의 지킴, 핸들러, 방랑자); 받아들이면 값을 내고 줄 하나; `BreakTreaty`의 눈 −0.3; 무역 자유화가 승낙 문턱을 낮춘다; 같은 목표를 다시 골라도 틱이 그대로 | 만남을 묻지 않는다; 승낙에 칙령 보너스를 빼먹는다 |
| `Sim.Fleet` (새) | 상한(칸의 곱 × 정착된 성계 수)·자람·유지비(못 내면 준다)·전시 동원의 곱·영향권 안의 소모(상위 원천만 — 주인의 칸·방랑자·이웃 항은 아니다)·첫째 칸 0; 지키는 성계가 영향권 안이어도 함대는 0 아래로 내려가지 않는다 | 상한에 칸만 쓴다; 주인의 칸을 원천으로 센다; `Max(0, …)`를 뺀다 |
| `Sim.Siege` (새) | `SiegeBlockOf`의 사유 차례(성역 첫째·수도·마지막 성계·인접·공성·상한·함대·불가침·적대의 차례; `bAssumeHostile`); 끝에 이른 공성마다 수 하나를 먼저 모두 뽑는다 — 끝 틱에 문을 잃은 공성도 수를 쓴다(스트림이 공성 수만큼 나아간다); 적용 직전에 문을 다시 묻는다(앞의 넘김이 뒤의 인접을 앗으면 풀림); 몫 A ÷ (A + D)와 지키는 몫 셋, `Takes(0.5, 0.5) == false`; 파괴된 성계를 지키던·에워싸던 목표가 그 틱에 풀리고 다음 분쟁 단계는 그것으로 뽑지 않는다; 못 낸 유지비는 비축을 건드리지 않는다; 넘어감(`Transfer`·`FollowUpKnowing`·휴전·쉼·이웃의 눈·**지키던 옛 주인의 목표가 비고 공성이 되지 않는다**)·지킴(공격 함대 × 0.5); 점검이 문을 잃은 목표를 푼다(제 성계는 `GuardBlockOf`); 한 성계에 공성 하나; `SiegeTicks`를 줄여 로드한 공성이 다음 틱에 수 하나로 끝난다; 데이터를 끈 판에는 뽑지 않는다 | 끝 틱에 문을 잃은 공성은 뽑지 않는다; 틱에 한 번만 뽑는다; 다시 묻지 않는다; 진 쪽의 목표를 남긴다(`Transfer`가 목표를 풀지 않는다); `Destroy`가 목표를 풀지 않는다; `==`로 끝을 본다; `<=`로 동률을 공격 쪽에 |
| `Sim.FleetDelay` (새) | 지키는 성계에 놓이는 위기의 카운트다운 + 지연(상한·0 아래 없음); 반란(수도 규칙)은 아니다; 서 있는 위기는 그대로; 주장은 타락의 칸 격차로 줄어든다; `Place`의 위쪽 자름은 `MAX_int32` 그대로 | 서 있는 위기를 늘린다; 반란에도 |
| `Sim.PlagueSpread` (새) | 이웃에 역병이 선 성계의 몫 × 곱; 단계 시작의 표(같은 틱에 놓인 역병을 읽지 않는다); 추첨 수 그대로; 격리령의 0.1 | 고리 안에서 읽는다; 곱을 빼먹는다 |
| `Sim.DiplomacyAI` (새) | AI는 우호를 놓는다(섭정도), 섭정이 물려받은 지킴을 물린다; 적대의 문(눈·전력 비·평화 축·불가침·몰매(다른 AI의 적대)·노릴 성계·영향권 밖·쉼)과 적대를 열면 곧 공성; 목표가 아니게 된 적대를 푼다(공성이 끝난 뒤·지킨 뒤); 쉼 동안 다시 열지 않는다; 첫째 칸 이웃(수도뿐)에게는 열지 않는다; 다스리는 백성은 건너뛴다; 즉위가 남의 목표와 적대를 지운다; 추첨 없음; 앎을 읽지 않는다(`Sim.KnowingInert`가 그대로) | 몰매 막기를 뺀다; 노릴 성계를 묻지 않는다; 지킨 뒤에도 적대를 남긴다; 쉼을 뺀다; 지킴을 남긴다 |
| `Sim.FirstContactCard` (새, `[B-3d]`) | 행이 있으면 첫 만남에 대기 칸, 행을 지운 테이블에서는 그대로; 기다리는 청원이 없을 때 먼저; 답의 `OtherID`; 카드 글의 `{OtherCiv}`가 상대의 이름으로 풀린다; 선택 둘의 결과(우호와 자리·중립); 답마다 1; 역할이 바뀌면 비운다(`Install` 포함)·다른 청원에 답해도(`Clear`) 대기 칸이 남아 그 뒤 앎 단계 끝에 첫 접촉 카드가 온다·상대가 사라지면 Withdrawn; 손을 내민다의 줄에 우호의 유지비와 자리가 보인다; 저장·로드 뒤 다시; 효과 `SetStance`는 `StanceBlockOf`를 지난다 | `OtherID`를 비운다; `Install`에서 비우지 않는다; `Clear`가 대기 칸도 비운다; 효과가 문을 건너뛴다; 우호의 줄을 빼먹는다 |
| `Sim.OwnCompletions` (새, #73) | 남의 완성으로 업적이 서지 않는다; 정복한 백성이 다시 완성하면 그 백성의 것; 옛 기록(`NAME_None`)은 지금 주인의 것이고 정복으로 정복자의 것이 되지 않는다(`Transfer`가 옛 주인을 적는다); 정복한 성계는 `feat_three_worlds`의 셋에 든다 | `CivID`를 묻지 않는다; `Transfer`의 옛 주인 쓰기를 뺀다 |
| `Log.ConflictLines` (새) | 공격·방어의 쪽마다 다른 행이 나고, 명령이 낸 줄(`log_siege_begun`)은 자동 정지하지 않는다(`CosmicAutoPause::StopsFor`가 거짓); 태세의 줄 둘이 연대기에 들지 않고 조약·공성의 줄은 든다 | 한 행을 두 쪽에 쓴다; 거름을 뺀다 |
| `UI.PeoplesTab` (새) | 탭의 행이 만난 백성만(차례 `ShownOrder`); 세 줄의 행의 바라는 너비 ≤ 312; 띠와 툴팁; 그들의 적대가 행에 보인다; 단추의 사유가 명령과 같다; 누르면 명령 하나; 초월자 없음; 패널의 함대 단추와 공성 줄(보이는 주인만), 지키는 성계의 띠는 1 − 몫, 다른 백성끼리의 공성은 패널에 줄이 없다, 만나지 않은 백성의 공성 줄은 이름을 보이지 않는다 | 만나지 않은 백성도; 사유를 다른 함수로; 띠의 뒤집기를 뺀다; 띠가 저장값을 읽는다; 제3자 공성에도 줄을 세운다 |
| 정책(`Sim.RulerDiplomacyPolicy`, 새) | 우호 → 불가침 → 무역의 차례·한 경계에 하나; 공성을 받으면 지키고 끝나면 물린다; `-Diplomacy=1`은 적대하지 않고 `-Diplomacy=2`는 한다; 첫 접촉 카드는 답한 수와 상관없이 첫 선택; 끝 뒤에는 바꾸지 않는다 | 한 경계에 둘; 적대한다; 첫 접촉의 예외를 뺀다 |
| `Sim.HashRun` (고침) | 정책의 새 값 | 새 값을 뺀다 |
| 고칠 테스트 | `Data.Integrity`(노브 길이·태그·새 열·`SetStance`·카드 행의 슬롯), `Sim.SaveRoundTrip`, `Sim.FirstContactTimeline`(그대로임을 확인), `Sim.KnowingInert`(그대로), `Events.RoleEffectRules`(`SetStance`의 상대 규칙), `UI.EdictRules`(새 두 줄), `UI.EdictWindow`(셋째 탭), `UI.TopBarAndPanel`(단추 툴팁). **밸런스 커밋**: 게임의 규칙을 복사해 보드를 까는 테스트(약 170곳 — `Board`는 백성 1·2를 레인 이웃에 둔다)에 공용 도우미 `CosmicTests::WithoutDiplomacy(const FCosmicDataTables&)`(표류율 0·함대 곱 0·번짐 1에 더해 `petition_first_contact` 행을 뺀 테이블 — `WithRules`처럼)를 `[B-3b]`에 넣고, 안정도·수입·소유를 상수로 보는 테스트가 쓴다. B3 전의 세계를 상수로 지키는 `Sim.SightWalkMoved`도 이 도우미를 쓴다(상수는 그대로). 세계가 옮기는 증거 테스트(`Sim.PhaseTwoExit`·`Sim.FirstLife`·`Feats.Reachable`·`Sim.WandererLoop`·`Sim.FirstCrisis`·`Sim.StartingEdict`의 게임 데이터 판)는 다시 재어 시드와 틱을 옮긴다 | — |

## 7. 문서 (구현의 문서 커밋 — 끝에 재감사 워크플로)

- **코드보다 먼저**(`[B-3a]`): §2.11의 '문서 먼저' 전부(`docs/21` §7.4·`docs/20`·`docs/70`·`CLAUDE.md` 규칙 4·13의 같은 줄 — `CLAUDE.md`는 LF 바이트 + 줄 수가 16,374로 남은 것이 10이다: 다른 줄을 줄여 값을 낸다).
- **문서 커밋**: `docs/21`(`FCosmicTie`·새 필드 넷·`CosmicDiplomacy`·`CosmicConflict`·명령 넷과 사유·노브·`SetStance` 효과·`HasCompleted`의 백성·건넘 정리의 `Ties`·즉위의 풀기·`:1347`·`:1458-1462`·`:1558`), `systems.md`(§5.5·§5.6을 현행으로 — 방향 있는 관계·태세와 우호의 자리·조약 둘과 그 문·함대의 상한·목표·공성·쉼·지연은 놓일 때만; `:18` 영향력의 식·`:44` 사회의 줄·`:58` 국경의 술어·`:64` 본보기 줄; §5.8에 격리령과 두 열), `crisis_and_transcendents.md`(`:22` 번짐·격리령, `:25`·`:34` 교단과 함대, `:72` 분쟁의 인구, `:116` '가져갔다'의 기계 검사는 없다('거두었다'는 `soul_verbs`가 막고 '가져갔다'는 글 감사가 본다), `:104`·`:106` 영압 ②의 소모·격차·지연 — B3가 넣은 것과 B4로 남은 것, `:125` AI의 일 여섯), `pillars.md`(`:76` T2의 조약과 적대), `docs/00_VISION.md`(`:118-119` 통치자의 결정, `:175` 첫 10분의 첫 접촉 카드 — 선택 둘), `docs/15`(관계·태세·우호의 자리·조약·함대·공성·지킴·쉼·국경·만남과 첫 접촉 카드·격리령), `docs/20`(§6.3 칙령 창의 [백성] 탭과 패널의 함대 줄, `:198`·`:273`·`:274`·`:282`·`:286-287`), `docs/40`(테스트 표·`-Diplomacy=1/2`·새 줄·단계 시간·짝 차이의 잣대·`Tools/mutations/B3/`), `docs/50`(B3 끝, `:12`의 편입률 문장, `:144` 분쟁의 추첨, `:180`·`:255` 청원 11장, B5·B6·B7로 넘긴 것), `docs/70`(§8.3 '외교와 분쟁의 줄', `:43` 소리 없는 표류는 만남을 낳지 않는다, `:45`·`:61` 첫 접촉 카드의 차례와 '입력 없는 판', `:166` '가져갔다', §8.4 연대기의 거름), `OPEN_QUESTIONS`(#24·#42·#73·#35·#124 닫기, #29 ⑧·#107 옮기기, #31 ② 기본값 확정, 새 행 — 수도·마지막 성계의 정복을 B5에서 다시 물을지, 서 있는 위기의 함대 지연 B4, 동맹 B7, AI가 통치자를 거의 노리지 않는다면 그 문), `DECISIONS_LOG`, `CHANGELOG`, `STATUS`, 코드 주석(`GalaxySimulationSubsystem.h:206-208` 'the card is Phase 4', `CosmicDefinitionRows.h:180-185` 번짐, `:864-877` 함대 노브, `GalaxyState.h:279-280` `FleetPower`·`:629-631` `PendingContactCivID`, `CosmicHazards.h:54-56` 지연), `DefaultGameplayTags.ini`의 `Log.Conflict` DevComment.

## 8. 순서·검증·effort

| 커밋 | 하는 일 | effort | 검증 |
|---|---|---|---|
| `[B-3a]` | 규칙 문서 먼저(`docs/21` §7.4·`docs/20`·`docs/70`·`CLAUDE.md`) + **저장 필드**(`FCosmicTie`·`Ties`·`FleetTargetSystemID`·`FleetTargetSinceTick`·`FMegaprojectRecord.CivID`)와 태그 넷 + 완성이 `CivID`를 적는 한 줄(새 기록과 다시 완성한 기록 모두 — 읽는 코드 없음), `Sim.SaveTies` | xhigh | 러너 full 두 번; 기준값 다섯과 해시 판은 **해시만** 바뀐다 — 커맨드렛 CSV의 결정적 칸(시간 `TickMicros`와 세이브 크기 `SaveBytes`·`SaveRawBytes`를 뺀 칸 — A0 4a의 증명 꼴; 세이브 크기는 새 필드와 완성의 주인만큼 는다)과 요약 줄의 해시 밖 수가 비트 그대로임을 보이고 `-UpdateBaseline`, 새 해시는 커밋 메시지에; **ultracode 구현 리뷰**(저장·해시·정규형) |
| `[B-3b]` | 장치 전부, 데이터 끔: `CosmicDiplomacy`·`CosmicConflict`·명령 넷·외교·분쟁 단계·위기의 두 고리·AI 규칙·즉위의 풀기·건넘 정리의 `Ties`·`SetStance` 효과(문을 지난다)·#73의 읽기와 `Transfer`의 옛 주인 쓰기·노브(장치 값 — C++ 초기값도)·칙령의 두 열·로그 행과 슬롯·줄의 낱말 스트링(`ST_CA` 끝에 `Stance.*`·`Treaty.*` — 줄의 슬롯이 읽는다)·`Destroy`·`Transfer`의 목표 정리·`ComputeAttritionPressureIn`·스키마와 그 테스트·`WithoutDiplomacy` 도우미 → Sim 테스트(테스트가 넣은 행과 노브로) → **ultracode 구현 리뷰**(결정론·추첨 지점·저장·틱 순서·앎). 첫 접촉 카드의 길은 넣지 않는다(§2.9) | xhigh | 러너 full 두 번, `[B-3a]` 대비 다섯 기준값 비트 그대로, 변이, `docs/30` 다시 생성 |
| `[B-3c]` | 표현: [백성] 탭·패널의 함대 단추와 공성 줄·칙령 툴팁의 두 줄·컨트롤러·콘솔·나머지 스트링 | high | UI 테스트, 창 없이 찍은 화면(탭·세 줄의 행·패널·공성 줄), **ultracode 화면·문구 리뷰** |
| `[B-3d]` | 데이터 켜기(노브 밸런스 값·격리령 행·두 열의 값·글)와 **첫 접촉 카드의 길 전부와 그 행**(§2.9), **글 감사**(ultracode — 첫 접촉 카드·로그 행 열하나·격리령·두 칙령의 설명·정복 뒤의 `feat_three_worlds` 줄), 외교 정책(`-Diplomacy=1/2`)과 커맨드렛의 줄·단계 시간, 데이터를 끈 판의 견줄 표(방랑자 60판을 먼저) → 한 표의 맞추기 → §5의 표 → 증거 테스트의 시드 옮기기 → `-UpdateBaseline` | high(카드의 길은 xhigh) | §5의 표, 다섯 기준값의 새 줄, **첫 접촉 카드 길의 ultracode 구현 리뷰**(앎 쓰기·대기 칸의 차례·답의 추첨·역할 전환의 비움 — `[B-3b]`의 관점 그대로), **ultracode 밸런스 리뷰** |
| `[B-3e]` | 문서(§7) | xhigh | **재감사 워크플로** |

## 9. 손 확인 (B3의 종료 조건 — 개발자, 콘솔 없이)

먼저 측정 2(시드 1~30)의 diplomacy 줄에서 통치자 창(A·B·C)을 본다 — 시드 42의 B가 C보다 앞서면 42로, 아니면 B < C인 가장 작은 시드로 하고, 게임을 `-Seed=<그 시드>`로 띄워 첫 판으로 둔다(새 판 단추와 `-Seed=` 없는 시작은 시계의 시드다; `-Seed=`는 저장도 끈다; 창은 정책의 판에서 잰 어림이다). 판 → ×1 5:30 무렵 첫 접촉 카드(자동 정지) — 두 선택지와 우호의 값(선택지 아래 줄)이 읽히는지, '손을 내민다'를 고른다 → 상단 바 '칙령' 단추 → [백성] 탭에 그 백성 한 행(띠·태세·조약 단추, 조약이 회색이고 사유가 '둘째 칸부터'인지, 우호 1/1 — 둘째 칸에도 자리는 하나다) → 둘째 칸 뒤 [불가침]을 눌러 맺는다(거절되면 툴팁의 사유를 읽고, 무역 자유화를 내린 뒤 다시 청한다) → 정착된 성계가 둘이 된 뒤(셋째 정착을 시작하지 않는다 — 보유 상한이 공성을 막는다; 공학 1.0 전에 — 정복이 셋째 칸과 판의 끝을 당긴다) 다른 이웃에게 [적대] → 그 이웃의 수도 아닌 정착된 성계를 골라 [함대를 보낸다](회색이면 첫 사유가 길을 알리는지) → 공성 줄(자동 정지하지 않는다)과 패널의 남은 시간·우리 쪽 띠 → 끝(차지함 또는 서지 못함의 줄 — 자동 정지). 다른 판: 외교는 하지 않고(칙령·청원·건설은 한다 — 방치하면 약 1,050에 폐위된다) ×10 — AI가 우리를 에워싸는 줄이 오면 제 성계에 [함대를 여기에]로 지켜 본다(끝 전에 오지 않을 수도 있다 — 측정 2의 몫). 탭의 글·툴팁의 수가 읽히는지, 창 안쪽 312단위에서 탭 줄과 세 줄의 행이 잘리지 않는지, 만난 백성이 여덟 이상일 때 굴리는지(해상도는 상관없다 — 화면은 높이 약 1,080단위).

## 10. 개발자 결정 (권장안 — 1·2·3·6·7·10·11·12·13을 먼저 본다)

1. **분쟁의 꼴은 공성**(권장): 함대 목표에서 파생하는 카운트다운(200틱) + 끝에서 한 번 뽑기(몫 A ÷ (A + D), 동률은 지키는 쪽). 대안: 문서대로 '틱마다 전력 비교 + 난수'(추첨이 적대 쌍 × 틱만큼 늘고, 언제·왜 잃었는지 읽을 수 없다).
2. **함대 배치는 목표 성계 하나**(권장 — 제 성계 = 지킴, 적대한 이웃의 성계 = 공성, 없음 = 고루 지킴). 대안: 성계별 비율(`systems.md:62` — 저장 맵과 슬라이더).
3. **수도와 마지막 정착된 성계는 분쟁이 앗지 않는다**(권장 — `Transfer`의 거절 그대로; 첫째 칸은 정복되지 않고, 은혜를 AI 전쟁이 쓰지 않고, 백성이 사라지지 않는다). 대안: `docs/21:1347`대로 마지막 성계 → 폐위(정복 폐위의 까닭 태그, 백성 제거 — B5와 얽힌다).
4. **조약은 둘(무역·불가침), 둘째 칸부터**(권장 — `pillars.md:76`과 맞춤; 승낙은 그들의 눈 ≥ −0.1, 국경 밖 쌍은 목표값으로). 대안: 셋(동맹까지 — 몰매의 엔진), 또는 첫째 칸부터.
5. **태세·조약은 짝 기록 하나**(권장 — 백성마다 상대 → `FCosmicTie{Stance 태그, Treaties 태그들}`, 정규형, 조약의 대칭은 함수 하나가, 건넘 정리가 지운다). 대안: 쌍의 최상위 배열(Cut List의 '범용 간선 저장소'를 닮는다), 관계 값의 문턱에서 파생(통치자의 결정이 남지 않는다).
6. **정복도 보유 상한을 센다**(권장 — #42를 닫는다; 상한에 닿은 백성은 공성을 시작하지 못한다 — 갈 데 없는 AI(22.5%)가 정복의 동기). 대안: #42대로 면제(눈덩이).
7. **함대의 상한은 칸의 곱 × 정착된 성계 수**(권장 — `FleetPerSettledByTier` {0, 1, 1, 1, 1}: 첫째 칸 0, 자람과 에너지 유지비(못 내면 아무것도 내지 않고 준다), 전시 동원은 자람의 곱 1.5 — 물려받은 칙령이라 통치자의 기본값이 된다; 측정 2가 1.0과 1.5를 짝으로 잰다). 대안: 칸만(성계를 잃어도 함대가 줄지 않고, 칸이 앞선 통치자가 늘 강하다; 후반이 어는 것은 두 꼴이 같다 — 측정 1), 물리 연구의 곱(B6).
8. **함대의 지연은 놓일 때만**(권장 — 지키는 성계에 새로 서는 위기에 + Min(100, 50 × 유효 전력), 반란은 빼고; B2 불변식 그대로, 저장 0). 대안: 서 있는 위기를 늘림(누계 필드·불변식 고침·`SystemHazardTicksAtMost` 카드가 두 번 설 수 있다 — B4로).
9. **격차 곱은 공격하는 쪽과 위기의 상대에만**(권장 — 지키는 쪽은 제 성계에서 받지 않는다). 대안: 문서대로 양쪽(앞선 칸이 늘 이긴다).
10. **AI의 적대는 공성과 한 몸이고, 공성이 끝나면 쉰다**(권장 — 적대의 문: 눈 ≤ −0.3 ∧ 전력 ≥ 1.5배 ∧ 평화 축 < 0.5 ∧ 불가침 없음 ∧ 다른 AI의 적대 없음 ∧ 노릴 성계가 있음(영향권 밖) ∧ 쉬는 중 아님; 열면 곧 공성, 공성이 끝나거나 풀리면 적대도 풀리고 600틱 쉰다 — 새 필드 없이 목표의 틱으로; AI는 우호·조약 제안·조약 끊기·지킴을 쓰지 않는다; 프로필을 읽지 않는다). 대안: 첫 판의 꼴(눈이 −0.2를 넘어야 풀린다 — 목표값이 문턱 아래라 거의 풀리지 않고, 목표 없는 적대가 몰매 막기를 비켜 간다), 프로필이 문턱을 옮김(B7과 겹침).
11. **#73을 닫는다 — 업적은 제 백성의 완성만**(권장 — `FMegaprojectRecord.CivID`, 다시 완성하면 고쳐 쓴다, 옛 기록은 지금 주인의 것이고 `Transfer`가 넘기기 전에 옛 주인을 적는다). 정복한 성계는 `feat_three_worlds`의 셋에 든다 — 정복으로 셋째 칸에 오르면 판이 그 틱에 끝난다(측정 6이 보고, 정복이 셋째 칸의 지배적인 길이 되면 그때 정의를 바꾼다). 대안: `project_settle`의 제 완성 기록이 있는 성계만 센다.
12. **첫 접촉 카드를 B3에 — 선택 둘**(권장 — 손을 내민다 → 우호(첫째 칸의 하나뿐인 자리와 유지비), 지켜본다 → 중립; 적대는 카드에 없다; 기다리는 청원이 없을 때 먼저; 카드의 길은 카드 행과 한 커밋). 대안: 선택 셋(셋째 '응답하지 않는다'에 그들의 눈 −0.1을 쓰는 효과 — 상대 쪽을 쓰는 선택자가 든다), 줄만(`docs/21:1458`의 결정 하나가 사라진다).
13. **역병의 번짐은 발생 몫의 이웃 곱**(권장 — 추첨 수 그대로, 서 있는 동안 번진다, 곱 8 잠정; 격리령은 B2의 칸으로 0.1). 대안: 문서대로 만료의 이웃 효과(역병이 만료하지 않아 판에 한 번도 일어나지 않는다), 발생 때의 새 추첨.
14. **격리령**: 칸 1·조건 없음·영향력 40·모든 수입 ×0.9·역병 몫 ×0.1(권장). **전시 동원의 함대 효과 = 자람 ×1.5**, **무역 자유화의 조약 효과 = 승낙 문턱 −0.2**(권장 — `FEdictDef`의 새 열 둘, 저장 0).
15. **관계의 표류**: 국경이 닿은 쌍만, 목표 = −0.6 × 윤리 거리 + 상대 태세의 항 ±0.4 + 조약 0.2씩, 걸음 0.002(권장). 국경 = 정착된 성계의 1레인(규칙 13의 술어 — `systems.md:58`의 '소유'를 고친다). 소리 없는 표류는 만남을 낳지 않는다.
16. **우호의 자리**(권장 — `FriendlySlotsByTier` {1, 1, 2, 2, 3}; 유지비 0.2/틱): 영향력이 묶이지 않아 유지비만으로는 '모든 이웃에게 우호'가 공짜인 지배 전략이 된다 — 첫 생애 백성의 윤리가 가운데라 우호를 건 이웃은 적대의 문(−0.3)에 끝내 닿지 않으므로 자리 하나가 이웃 하나와의 영구한 평화다; 둘째 칸에도 하나라 분쟁 창의 국경 이웃 약 2.3 가운데 누구에게 손을 내밀지를 고르게 한다. 대안: {1, 2, 3, 4, 5}(둘째 칸에서 국경 이웃을 거의 다 덮는다), 자리 없이 유지비만(첫 판).
17. **영향력의 쓰임**: 우호의 유지비 0.2/틱·조약의 값 50·무역 조약의 수입 0.2/틱(권장 — `systems.md:18`의 '× 조약 수'를 '+ 무역 조약마다'로). 영향력을 묶는 것은 B3의 목표가 아니다(결정 19).
18. **줄은 다스리는 백성이 당사자인 것만, 쪽마다 행**(권장 — 다른 백성이 우리를 에워쌈·차지함·잃음·지켜 냄·서지 못함은 Severity 2(자동 정지·연대기), 우리의 명령이 연 공성과 풀림과 조약은 1(연대기), 태세는 1(연대기에서 뺀다); 상대는 인자 `{OtherCiv}` — #31 ② 기본값; AI끼리의 분쟁은 말하지 않는다 — 지도의 주인 색만). 대안: AI끼리의 줄도(둘째 백성의 범위 규칙 #107이 든다).
19. **측정의 목표**(§5): 넘김 중앙값 3~10·2/3 판 ≥ 1·시간 분포, 눈덩이와 몰매의 셈, 안정도·반란은 데이터를 끈 판과의 짝 차이로, 역병 ≤ 1.5배, 결정 ≥ 15, 첫 접촉 카드 ≤ 600틱, 방랑자 ≥ 44/60(60판), AI가 끝 전에 통치자를 에워싼 판 10~30%. **'외교가 주장의 편입률을 올린다'는 기대를 지운다**(편입은 프로필이 정한다 — 측정 항목으로만).
20. **화면**: 칙령 창의 셋째 탭 [백성](세 줄의 행, 상단 바 단추를 늘리지 않는다)과 성계 패널의 함대 단추·공성 줄(띠는 우리 쪽); 지도의 공성 표지는 뒤로(권장). 대안: 새 '외교' 창과 상단 바 단추.
21. **AI가 통치자를 거의 노리지 않으면**(측정 2의 `-Diplomacy=0` 판이 10% 아래): 통치자를 향한 적대의 문만 따로 낮추는 노브(가칭 `AIHostilePowerRatioTowardRuled` 1.0)를 `[B-3d]`에서 더할지 그때 정한다(권장 — 먼저 잰다; 우호·불가침이 닫은 문은 이 노브로 열리지 않는다 — 설계대로). 대안: 받아들이고 '전쟁은 통치자가 고를 때만 온다'로 적는다.
22. **B3 밖**(§0): 동맹·AI 조약·AI의 조약 끊기·프로필 외교·AI의 지킴(B7), 수도·마지막 성계의 정복(B5에서 다시), 서 있는 위기의 지연(B4), 희년(B6/EA), 전쟁의 피로(측정 뒤), 평화 성향의 흐름(B7), `InitialRelations`(B7), 조약 상대로의 번짐(B7), 물리의 함대 승수(B6), 영향권 소모의 줄(B4), AI끼리의 줄과 지도의 공성 표지, #107(B6), 샬롬 협정(C).

## 11. 구현 기록 (헤더와 다르게 하거나 더한 것 — 커밋마다 더한다; 까닭과 수는 `DECISIONS_LOG`)

1. `[B-3a]`: 새 테스트 `Sim.SaveTies`는 따로 두지 않고 그 단언을 셋에 나눠 넣었다 — 저장·로드의 같음은 `Sim.SaveRoundTrip`(채움과 `SameLaterMembers`), 해시가 새 필드를 보는 것은 `Sim.Determinism`, 완성이 `CivID`를 적는 것(새 기록·다른 백성으로 심은 기록·옛 기록)은 `Sim.MegaprojectProgress`.
2. `[B-3a]`: 해시는 태그 컨테이너의 태그를 `FNameLexicalLess` 차례로 넣는다(저장된 차례가 아니라 — `AddTag`가 끝에 더하므로 조약을 끊었다 다시 맺은 짝이 같은 짝과 다른 해시를 가졌다; 구현 리뷰). 모든 태그 컨테이너(특성·플래그·권능·상태)의 해시가 함께 옮았다 — 필드를 더하는 같은 커밋이라 기준값을 한 번만 다시 적는다.
3. `[B-3b]`: 태세·조약의 문은 받지 않는 태그(태세 셋·조약 둘 밖의 태그)를 `NoCiv`로 거절한다 — 사유 목록을 늘리지 않고, 명령과 효과가 같은 문을 지나므로 세계에 다른 태그가 들지 않는다.
4. `[B-3b]`: 효과 `SetStance`가 카드의 답에만 서는 것은 스키마의 두 칸으로 지킨다 — 효과의 `card_only`와 열의 `card`(`DT_Encounters`의 `OnSuccess`·`OnFailure`). 칙령의 `FleetGrowthMultiplier`는 `DT_Edicts`의 행 규칙(`gt 0`)이 0보다 크게 지키고 코드는 0 이하·수가 아닌 값을 1로 읽는다. 태세·조약의 태그 넷은 `required_tags`에 들어 `Data.Integrity`와 `check_data.py`가 ini와 견준다.
5. `[B-3b]`: 화면과 정책이 읽을 작은 함수를 더했다 — `CosmicConflict`의 `SettledCountOf`·`OwnedCountOf`·`AttackGapOf`·`BesiegerOf`·`IsBesieging`·`SiegeEndTickOf`·`OddsBandOf`·`FleetGrowthMultiplierOf`·`LiftTargetsOn`, `CosmicDiplomacy`의 `BorderTableOf`·`TradeTreatyCountOf`·`TreatyAcceptBonusOf`·`TurnHostile`·`LowerRelation`·`StanceNameOf`·`TreatyNameOf`. 코드가 이름으로 부르는 태그는 `Core/CosmicTags.h`(상수 헤더)에 있다.
6. `[B-3b]`: `HasCompleted`의 백성이 빈 이름이면 그 성계의 지금 주인으로 읽는다(백성이 아니라 성계를 묻는 자리 — 커맨드렛의 답의 셈과 테스트).
7. `[B-3b]`: 구현 리뷰(ultracode — 결정론·세계 규칙·앎과 명령·테스트의 네 관점에 관점마다 반박 검증, 빠진 것을 찾는 비평 하나)가 더한 것: **`Transfer`가 다스리는 백성이 잃는 성계에 선 통치자의 그릇을 그 백성의 수도로 옮긴다**(`Destroy`가 옮기는 것과 같은 꼴 — 정복이 백성 사이의 첫 넘김이라 전에는 없던 상태: 남의 영토에 선 통치자, 그 자리에서 이는 폐위의 줄과 방랑의 첫 눈). 점검이 공성을 풀 때의 줄도 상대를 먼저 만난다(§2.11 표의 칸을 고쳤다 — §3.4의 '줄은 상대를 먼저 만난다' 그대로). `MegaprojectComplete`의 주석들(#73).
8. `[B-3b]`: 헤더 §6의 Sim 테스트 아홉과 `Log.ConflictLines`는 한 파일(`Tests/CosmicDiplomacyTests.cpp`)에 있다. 보드는 데이터를 끈 규칙과 시야 없음에서 시작해(케이스가 켜는 것만 켜진다 — `[B-3d]`가 데이터를 켜도 그대로) 앎에 쓰는 길(공성의 줄 전 `NoteMet`, 넘김 뒤 `FollowUpKnowing`, AI가 연 공성의 줄 전 `NoteMet`)을 만나지 않은 상대로 본다. AI와 분쟁의 세계가 앎을 읽지 않는 것은 `Sim.KnowingInert`를 기다리지 않고 `Sim.DiplomacyAI` 안의 짝 판(모두 안다 대 아무것도 모른다 — 적대가 열리고 공성이 끝나는 15틱, 세계 해시와 시뮬 스트림이 같다)이 본다. `Events.RoleEffectRules`가 `SetStance`의 줄(틱당 원천은 거절, 문을 지나 쓰고 거절은 무동작)을 받았다.
9. `[B-3b]`: §3.6 가운데 `DT_Encounters`의 `{OtherCiv}` 슬롯과 '`SetStance` 없는 카드는 `{OtherCiv}`를 부르지 않는다' 규칙·그 깨지는 경우는 `ArgsOf`의 `{OtherCiv}`와 함께 `[B-3d]`로(그 전에 슬롯을 열면 채우는 이 없는 슬롯을 카드가 부를 수 있다 — 지금의 스키마가 더 엄하다), ST_CA 접두어 `Diplomacy.`·`Siege.`·`Guard.`는 그 키들과 함께 `[B-3c]`로 간다(`Stance.`·`Treaty.`는 줄의 낱말과 함께 들었다).
10. `[B-3c]`: 화면·문구 리뷰(ultracode — 화면 코드·문구·테스트의 세 관점에 관점마다 검증, 빠진 것을 찾는 비평 하나; 발견 47건, 반박 1)와 그 반영을 다시 본 확인 리뷰(ultracode — 반영·코드·문구·테스트의 네 관점에 발견마다 반박 검증; 발견 25건, 남은 21)를 거쳐 헤더와 다르게 하거나 더한 것:
    - **탭 줄**: [칙령][연구][백성][닫기] — 연구의 몫 탭의 이름을 '연구'로 줄였다(탭 줄이 312단위에 꼭 맞는다). `docs/20`·`docs/15`의 '[연구의 몫]'은 `[B-3e]`가 고친다.
    - **머리 줄**: 함대도 상한도 없으면 `Guard.NoFleet`, 목표가 없고 함대나 상한이 있으면 목표 줄은 `Guard.Even`('함대는 모든 성계를 고루 지킨다'). 함대의 툴팁은 자람과 상한의 규칙과 크기 하나의 유지비(합이 아니라 율 — 합은 틱마다 움직인다), 우호 자리의 툴팁은 다음 자리가 열리는 칸(`UCosmicEdictWidget::NextFriendlySlotRungOf`, 다 열렸으면 `Diplomacy.SlotsAll`), [함대를 물린다]는 지킴과 공성을 나눠 말한다(공성은 '다시 보내면 처음부터').
    - **행**: 그들의 적대는 이름 줄이 아니라 제 줄에 선다(긴 이름 + 띠 + '우리를 적대한다'가 한 줄에 들지 않는다 — 행은 최대 이름·적대·태세·조약·공성의 다섯 줄). 지금의 태세는 회색에 더해 `Edict.Mark`('●')를 이름 앞에 단다(거절된 태세와 같은 꼴이었다); 지금의 우호는 툴팁에 유지비를 그대로 둔다. 맺은 조약의 단추는 **[무역을 끊는다]·[불가침을 깬다]** — 낱말마다 그 목적어에 맞는 동사(헤더의 [불가침을 끊는다]와 함께면 두 단추가 한 줄에 299단위로 스크롤 막대 곁 296을 넘었다; 지금 가장 넓은 행 282; 줄 `log_treaty_broken`의 '깨졌다'), 깨는 툴팁은 '조약이 깨지면'. 행의 공성 줄은 쪽마다 `Siege.Row.Ours`·`Siege.Row.Theirs`. 행은 `FCosmicKnownView::IsMet` ∧ `ShownOrderOf` — '만난 백성마다'(모두 보기에서도 만나지 않은 백성은 행이 없다).
    - **툴팁이 말하는 값과 뒤따름**: 적대는 무역 조약이 있으면 배신의 눈 하락을, 중립·우호는 우리 함대가 그 백성을 에워싸고 있으면 공성이 풀림을(`Siege.Lifts`), 조약을 깨는 단추는 국경이 닿으면 눈 −0.3, 닿지 않으면 조약의 몫(저장된 눈을 읽지 않는 쌍 — `ViewOf`)을 말한다.
    - **보는 눈의 수**: 헤더 §4는 띠의 툴팁에 '항과 수'를 두었으나 **지금의 눈의 수는 조약 단추의 툴팁 한 곳에서만** 말한다 — 표류하는 수라 툴팁에서 십분의 일까지(열린 툴팁이 거의 닫히지 않는다), 아래로 자르되 닿은 문턱 아래로는 읽히지 않는다(문턱이 십분의 일이 아니어도 — 그 문턱의 수 그대로). 띠의 툴팁은 항만(국경이 닿으면 목표·윤리·태세·조약, 아니면 윤리·조약과 `Diplomacy.TipFar` — 명령이나 느린 윤리로만 움직인다). 첫 판은 띠 곁에서 0 쪽으로, 문턱 곁에서 아래로 잘라 한 행이 같은 눈을 두 수로 읽었다(확인 리뷰) — 띠의 경계와 문턱을 함께 지키는 하나의 십분의 일은 없다. 항의 수는 '−0'·'+0'을 쓰지 않고(0.005 이하는 0), 태세의 항은 목표에서 빼지 않고 노브에서 읽는다. 문턱과 값은 문과 같은 함수 `CosmicDiplomacy::TreatyBarOf`·`TreatyCostOf`(`TreatyBlockOf`가 그것을 부르도록 옮겼다 — 같은 산술).
    - **공성 줄의 남은 시간**: `{Time}`은 길이(`DurationLabel` — '남은 시간 200주기')다; `TimeLeftLabel`은 '남은 시간 남은 주기 200'으로 두 번 말했다.
    - **패널의 함대**: 줄과 단추는 상세 줄 아래가 아니라 **정착 단추의 사유 줄 아래의 한 블록**(`GetFleetText`·`GetFleetButtons` — 프로젝트 단추 목록은 프로젝트만 그대로; 정착의 사유 줄은 정착 단추 바로 아래로 옮겼다 — 정착 단추가 보이는 패널에는 그 사이에 다른 것이 서지 않아 보이는 꼴은 그대로). 누름이 단추를 지우고 줄을 그 자리에 들이므로 아래의 것이 커서 밑으로 거의 움직이지 않는다. 회색 단추는 이름 뒤에 짧은 사유(`Guard.Suffix.NoFleet`·`Siege.Suffix.*`), 함대가 있는데 `NoFleet`이면 주인이 너무 높은 것(`Siege.Block.Above` '두 칸 이상 높아' — 칸 격차의 효율 0, `Guard.DelayTip`의 꼴). [함대를 보낸다]의 툴팁은 실패하면 함대가 그 몫만 남는 것(`CosmicConflict::RepulsedShareOf` — 분쟁 단계와 같은 함수)·어느 쪽이든 태세가 중립으로 돌아가는 것·차지하면 그 백성과 국경이 닿은 백성들의 눈이 낮아지는 것, 그리고 누를 수 있을 때만 우리 쪽 띠(`Siege.SendOdds`); 두 단추 모두 다른 목표에서 함대를 옮기면 그 목표를 말한다(`Guard.Moves`·`Siege.Lifts`). 지킴의 툴팁에 '그동안 다른 성계는 덜 지킨다'. 지연의 줄은 헤더의 'FleetDelayPerPower × FleetPower ≥ 1'이 아니라 **지킴의 지연 그 자체가 한 틱 이상일 때**(`CosmicConflict::GuardDelayTicksOf` — `FleetDelayOf`의 산술을 나눈 함수, 격차 0: 효율과 상한까지 같다). 소모의 줄은 '이곳을 누르는 압력에 …'(영향권이라는 설계어를 쓰지 않는다). 패널이 보이는 지킴의 사유는 명령과 같은 사상으로 공성의 이름(`CosmicConflict::SiegeNameOf`).
    - **두 번 누름**: `UCosmicActionButton`은 Slate 단추의 더블클릭을 스스로 받는다(`SWidget::SetOnMouseDoubleClick` — `SButton`이 받지 않은 더블클릭을 누름으로 받아, 제자리에서 바뀌는 단추([무역]→[무역을 끊는다], [내린다]→[철회한다])가 제 명령을 곧 되돌렸다). 더블클릭의 간격은 시스템의 설정을 따르고, 다른 단추를 사이에 둔 빠른 누름은 막지 않는다(첫 판의 0.5초 시계는 둘 다 틀렸다 — 확인 리뷰). 코드의 `Press`는 막지 않는다.
    - 창은 [백성] 쪽이 보일 때만 그 쪽을 다시 그린다(행마다 국경과 눈을 읽는다). 콘솔 `CosmicFleet`은 지킴의 사유도 공성의 이름으로 적는다. 상단 바 칙령 툴팁의 줄은 '칙령 창의 [백성]'.
    - **테스트**: §6의 `UI.EdictRules`(새 두 줄)는 시뮬의 테이블이 드는 `UI.PeoplesTab`에 두었다(없는 경우 — 곱 1·보너스 0이면 두 줄이 없다 — 와 함께), `UI.TopBarAndPanel`(단추 툴팁)의 단언(상단 바 칙령 툴팁의 `Edict.Bar.Peoples`)도 `UI.PeoplesTab`에 — `CosmicPanelTests.cpp`는 그대로다(함대 단추가 제 상자라 프로젝트 단추의 목록이 바뀌지 않는다). `UI.EdictWindow`가 셋째 탭을 누른다. 보드는 화면이 읽는 노브를 스스로 놓고(`[B-3d]`의 값과 상관없이), 칙령 하나(문턱 보너스 0.15 — 십분의 일이 아닌 문턱)를 둔다. 공성의 남은 시간은 다른 두 값(150·100)으로 박는다. 끝난 판(명령 넷의 거절·창·패널의 회색 함대 단추), 만났으나 세 칸 위인 주인(행 없음·이름 없는 공성 줄·[함대를 보낸다] 없음), 더블클릭(`SButton`의 더블클릭이 누름이 되지 않는다), 지연의 문턱(0.019·0.02). 너비는 탭 줄 ≤ 312, 가장 넓은 행(긴 이름·적대·두 조약을 깨는 단추) ≤ 296(스크롤 막대 곁).
    - 받지 않은 발견: 첫째 칸의 회색 단추 벽(헤더 §4의 선택 — 손 확인이 본다), `Guard.DelayTip`의 '원천'(헤더가 승인한 문구 — 고치려면 헤더의 문구 결정), '우리'라는 화자(헤더의 문구 — `[B-3e]`에서 `docs/70`에 'UI의 우리 = 다스리는 백성'을 적을지), 소모를 누르기 전에 알리기(헤더가 목표의 줄에 둔다), 소모의 툴팁 줄의 UI 테스트(상위 초월자의 영향권을 까는 보드 — 소모의 압력은 `Sim.Fleet`이 본다).

## 추가 이야기와 보완

- 커밋 다섯(필드만·장치·화면·밸런스·문서)은 B1·B2보다 하나 많다 — 필드를 더하는 커밋에서 '기준값 비트 그대로'를 지킬 수 없어, 해시만 옮기는 증명(CSV의 결정적 칸)을 먼저 따로 세우는 값이다. 데이터를 꺼도 도는 쓰기(완성 기록)는 그 커밋으로, 코드가 이름으로 부르는 행이 드는 길(첫 접촉 카드)은 데이터 커밋으로 옮겼다.
- 평화의 길은 우호(자리 하나)를 먼저 걸고 그 이웃의 눈이 −0.1에 닿기를 기다린 뒤 불가침을 맺는 것이다 — 우호를 건 중앙값 이웃은 둘째 칸 전에 이미 닿는다(값은 측정 2에서 확인). 불가침은 AI가 끊지 않으므로 맺은 이웃과는 전쟁이 없다(값은 조약 50과 [끊는다]의 눈 −0.3). AI가 조약을 끊는 길은 B7의 프로필과 함께 다시 묻자.
- 느부갓네살 모티프('FleetPower 1위 ∧ Stability > 0.9 …' — `docs/70:201`)의 문이 B3에서 처음 열린다(함대가 뜻을 얻는다). 모티프 자체는 그 블록의 일이지만, 함대 1위를 화면에 순위로 보이지 않는 편이 그 시험의 결과 맞다(B3는 순위를 보이지 않는다).
- 첫 접촉 카드의 선택은 첫 10분의 결정 수(지금 정책 판 600틱 11)를 하나 더한다 — `docs/00_VISION.md:175`의 '결정 15~20'에 가까워진다.
- 첫 생애 백성의 윤리가 가운데라 AI의 눈이 통치자에게 덜 모질다(목표 중앙값 −0.30 — AI끼리는 −0.39) — 적대의 문 −0.3은 AI끼리 쌍의 71%, 통치자를 보는 쌍의 51%를 연다(−0.5였으면 3%로 통치자는 거의 노려지지 않는다). 그 위에 전력 비·몰매·쉼·노릴 성계가 문을 좁힌다. 측정 2와 결정 21이 그것이 '전쟁이 오지 않는 판'이 되는지 본다.
