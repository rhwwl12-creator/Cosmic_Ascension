# B7x 노트 — 조합형 존재와 시간이 남긴 꼴 (헤더 `B7_composable_existence_and_evolution.md`의 짝)

헤더의 표와 긴 본문을 여기에 둔다. 사실의 기준은 `main`의 `cef8cc7`(ultracode 리뷰가 `10205b6`에 견주었다 — 헤더 머리), 설계서는 `docs/10_DESIGN/EXISTENCE_EVOLUTION_AND_ECOLOGY.md`다. '자리'의 낱말: **가**(슬라이스 가 — 이 블록에서 읽는다)·**나**(레인의 창구)·**다**(품음과 탄생)·**B7**(AI 백성과 AI의 손)·**B8**(실제 하늘)·**글**(얼굴과 문장만 — 태그를 선언하지 않는다)·**받지 않음**. '처리'의 낱말: ㉠ 규칙을 고쳐 받는다·㉡ 꼴을 옮겨 받는다·㉢ 받지 않는다(헤더 §0).

## 1. 예순 블록의 처분표

### 1.1 형태 (`Brick.Form.*` — 14)

| 블록 | 원안이 바라는 것 | 부딪는 곳 | 받은 꼴 | 자리 |
|---|---|---|---|---|
| `Vessel` | 한 몸의 개체, `Scale`, 반란 없음 | 개체는 없다(요약 단위) — '그릇'은 플레이어 한 사람의 낱말 | 인물이 없는 한 몸의 백성(`Trait.Mind.NoSelf`의 거름), 반란의 몫 감쇠; 잎 이름을 고친다 | B7 |
| `Society` | 필멸의 사회 | — | 갈림 카드의 출발 표지 | 가 |
| `Crust` | 물질 +100%, 공성 면역에 가까움 | 면역(K8) | 물질 수입의 곱, 면역 없음 | B7 |
| `Shroud` | 별에서 직접 에너지, '2~3칸 권능' | 칸(K4) | 에너지 수입의 곱 | B7 |
| `VoidWeb` | 레인에 걸친 몸, 통행 지배 | 레인에는 상태가 없다 | 있는 조우 `enc_lane_weave`의 얼굴; 레인 시간은 나 | 글·나 |
| `LivingNebula` | 세계가 된 몸, 품음 | `Scale` 4칸, 두 주인 | 번진 살림의 표지; 품음은 `HostCivID`로 | 가·다 |
| `Computronium` | 안정 고정, 위기를 통찰로 | 고정은 면역 | 안정 회복과 통찰의 곱 | B7 |
| `DysonBrain` | 통찰과 인과의 폭발 | 인과율 수입은 0(#90), 이름 | 닫아 건 살림의 표지, 제 별의 위기 몫 곱(`BrickStellarCrisisSpawnMultiplier`); 통찰은 S01의 줄 행 | 가 |
| `EventHorizon` | 파괴 불가, 기억 박제 | 면역(K8), 죽은 이의 기억을 재는 미터 | 죽은 별 곁 한 성계 백성의 얼굴 | 글 |
| `QuantumCloud` | 관측 전에는 어디에나 | 세계가 앎을 읽는다(K1) | 세계는 늘 확정 — 정착된 성계가 레인으로 닿거나 그릇이 서면 드러나고, '어디에나'는 표현만 | 받지 않음(㉡은 뒤 블록) |
| `AcousticString` | 레인을 현으로, 윤리를 홀림 | 레인 상태, 노래와 홀림은 침묵 교리의 표지 | 항로의 떨림이라는 얼굴 | 글 |
| `ReverseTime` | 미래에서 과거로, 다섯째 칸 | 시계 하나(K2), 칸(K4), 예언 | 정해진 걸음으로 규모가 줄어드는 늙은 백성과 기록의 인용 | 받지 않음(얼굴만) |
| `FossilAutomata` | 절멸 뒤 남은 기계의 나라 | 절멸의 만료에 새 추첨 없음, 백성의 탄생 | 건넘 단계의 요청으로 | 다 |
| `UnseenLaw` | 무형의 법·신앙이 됨 | 누구도 법이 '되지' 않는다(K6) | 왕좌가 빈 뒤에도 지닌 칙령을 하나도 거두지 않는 백성의 표지('남긴 법' — 은하의 법칙은 이미 누구나 지킨다) | 가 |

### 1.2 섭식 (`Brick.Diet.*` — 14)

| 블록 | 원안이 바라는 것 | 부딪는 곳 | 받은 꼴 | 자리 |
|---|---|---|---|---|
| `Biosphere` | 기본 | 읽는 곳이 없다 | 선언하지 않는다 | 글 |
| `MantleStrip` | 폭식 뒤 풍부도를 깎음 | 풍부도는 수입의 계수일 뿐 | 물질 수입의 곱과 안정의 내림 | B7 |
| `StarEater` | 별 등급 강등, 흑색 잔해 | 등급은 일곱, 빈 등급은 죽은 노드, 실제 하늘 | 에너지의 곱·이웃 눈의 내림·제 별의 위기 몫 곱·(꺼 둔) 한 단씩의 어두워짐 | 가 |
| `DarkMatter` | 정착 없이 여과 | 땅 없는 백성은 지워진다 | 별에 매이지 않는 에너지 | 가 |
| `OrbitBrake` | 레인 길이를 흔듦 | 레인 시간은 규칙 행 하나 | 그 백성의 성계를 지나는 레인 시간의 덤 | 나·B8 |
| `BioHarvest` | 충돌로 특성을 흡수 | 틱당 원천은 수량 효과만 | 공성의 끝에 한 번 서는 효과 | B7 |
| `ChronoEntropy` | 버려진 선택지를 먹음 | 카드는 앎에 산다(K1) | 답 없이 만료한 위기라는 세계의 사실; 얼굴은 아공간 침식(#90) | 글 |
| `Resonance` | 경외와 기도를 먹음 | 경배를 받는 존재는 타락한다(K6), 신앙의 미터 | 조약의 수에서 통찰; 경배를 받는 선택은 그릇의 높아짐 | B7 |
| `SilenceFeed` | 사건 없는 틱을 먹음 | 사건은 세계에 기록이 없다, 침묵의 낱말 | 일 없는 틱(위기·사업·공성 없음)의 통찰; 잎 이름을 고친다 | 가 |
| `ForgottenNames` | 무명 로그를 먹음 | 세계가 로그를 읽는다(K1), 죽은 이름의 미터 | 죽은 노드의 수라는 세계의 사실, 글은 애도로 | 글·B7 |
| `DistanceTension` | 멀수록 산출 | 좌표의 거리는 규칙이 읽지 않는다 | 제 성계 사이 레인 수의 폭 | 가 |
| `CrisisParasite` | 위기의 카운트다운을 먹음 | 서 있는 카운트다운·주장(K9) | 이웃에 새로 서는 위기의 놓일 때 덤, 주장·절멸·전조 제외 | 가 |
| `DeadStarLight` | 죽은 별만 | 죽은 노드에는 아무것도 없다 | 가장 어두운 제 별에서의 곱 | B7 |
| `BorderFriction` | 정반대 윤리의 국경 | 국경 배율은 두지 않기로 했다(#88) | 윤리의 거리가 먼 이웃과 닿을 때의 곱 | B7 |

### 1.3 공간 (`Brick.Space.*` — 12)

| 블록 | 원안이 바라는 것 | 부딪는 곳 | 받은 꼴 | 자리 |
|---|---|---|---|---|
| `Settler` | 기본 | 읽는 곳이 없다 | 선언하지 않는다 | 글 |
| `Nomad` | 수도 없는 유영 | 성계 없는 백성은 지워진다 | 빨리 넓히는 백성 | 가 |
| `SystemDragger` | 좌표 이동과 레인 재연결 | 레인·좌표는 생성 뒤 그대로 | 레인의 창구 위에서 — 먼저 레인, 다음에 좌표 | 나 |
| `OrbitShaper` | 인공 레인 | 같음 | 창구의 `Link`; 설계의 웜홀 게이트 행과 한 길 | 나 |
| `HermitMonolith` | 한 성계, 침공 무효, 빠른 안쪽 시간 | 면역(K8), 시계(K2) | 정착의 거절과 연구의 곱; 보호는 있는 규칙으로 족하다 | 가 |
| `OmniLinked` | 거리 0 | 0틱은 없다 | 제 성계들이 지킴에서 서로 이웃 | 나 |
| `HostWorld` | 다른 백성을 품음 | 한 성계 한 주인 | `HostCivID` — 주인은 품긴 쪽 | 다 |
| `FoldedTopology` | 양 끝을 포갬 | 레인 | 제 성계 둘 사이의 레인 하나 | 나 |
| `PlanckMicro` | 무주처럼 보임, 완전 면역 | 세계의 소유는 진실(K1), 면역(K8) | 정착의 거절·위기 몫의 감쇠·연구의 곱; 주인 표시는 그대로 | 가 |
| `Labyrinth` | 빠져나가지 못하는 영역 | 함대는 유닛이 아니다 | 방랑자의 이동 거절 사유 하나 | 나 |
| `MirrorProjection` | 원본을 따라 피어남 | 백성의 탄생 | 탄생의 요청 | 다 |
| `NovaSpore` | 죽으며 흩어짐 | 탄생, 추첨 수 | 고정 수의 추첨과 정착의 문 | 다 |

### 1.4 본능 (`Brick.Instinct.*` — 8)

| 블록 | 받은 꼴 | 자리 |
|---|---|---|
| `Aloof` | 적대와 조약의 걸음을 서지 않는 AI | B7 |
| `ApexDevour` | 겨눔의 문턱이 낮은 AI | B7 |
| `Symbiotic` | 조약의 걸음이 먼저인 AI — '교정'은 없다 | B7 |
| `HiveMerge` | 이웃의 눈을 올리는 표지; 동화의 위조와 같은 결이라 축하하지 않는다 | 가 |
| `SwarmTech` | 연구의 곱 — 촘촘히 엮은 일손 | 가 |
| `LawRadiant` | 은하 범위의 칙령을 먼저 세우는 AI; '거룩한 안식과 울타리'는 뺀다(K6) | B7 |
| `Warden` | 지킴과 `project_binding`을 먼저 하는 AI; 묶는 것은 `CosmicSeal`뿐이고 울타리는 없다 | B7 |
| `Collector` | 얼굴만 — 성계째 끌고 가지 않는다; B7의 `ReverseTime` 행이 `Requires`로 읽는다(S02) | 글·B7 |

### 1.5 변이 (`Brick.Mutation.*` — 12)

| 블록 | 받은 꼴 | 자리 |
|---|---|---|
| `StarShepherd` | 머물지 않는 살림의 둘째 표지; 죽은 별을 다시 켜지 않는다 | 가 |
| `WorldBecome` | `LivingNebula`에 합친다; 백성의 자연 발생은 다 | 가·다 |
| `KesslerTapestry` | 엮은 일손의 둘째 표지; 칸의 격상과 레인은 없다 | 가 |
| `ApexSingularity` | `HiveMerge`의 유혹에 합친다 — 따로 두지 않는다 | 받지 않음 |
| `ChronoInversion` | 결과를 미리 적용 — 시계 하나(K2) | 받지 않음 |
| `PlanckAscension` | `PlanckMicro`에 합친다 | 가 |
| `GravityHologram` | 절멸의 순간 다시 태어남 — 만료 길에 새 추첨 없음 | 받지 않음 |
| `LivingCovenant` | `UnseenLaw`의 받은 꼴에 합친다 | 가 |
| `SilentGardener` | 영원한 평화의 정원 — 침묵 교리의 위조 | 받지 않음 |
| `AbyssalMaw` | 삼키는 살림의 둘째 표지 | 가 |
| `UndyingAutomata` | `FossilAutomata`와 같다 | 다 |
| `BridgeOfSighs` | `FoldedTopology`와 같다 | 나 |

슬라이스 가가 선언하는 태그는 열일곱이다: 형태 넷(`Society`·`LivingNebula`·`DysonBrain`·`UnseenLaw`), 섭식 다섯(`DarkMatter`·`StarEater`·`SilenceFeed`·`DistanceTension`·`CrisisParasite`), 공간 셋(`Nomad`·`HermitMonolith`·`PlanckMicro`), 본능 둘(`SwarmTech`·`HiveMerge`), 변이 셋(`StarShepherd`·`KesslerTapestry`·`AbyssalMaw`).

## 2. 부딪는 곳과 처리

| # | 설계서의 것 | 닿는 규칙 | 처리 | 받은 꼴 |
|---|---|---|---|---|
| 1 | 여덟 아키타입과 네 초월자를 해체 | 초월자는 교리마다 한 체, 아키타입 행의 이름은 세이브에 남는다 | ㉡ | 블록은 있는 행 위의 태그다 — 아무 행도 사라지지 않는다 |
| 2 | 틱 엔진이 블록의 조합을 읽는다 | 틱 열 단계 | ㉡ | 있는 단계 안의 걸음 둘(★)과 읽는 자리 하나씩 |
| 3 | `Brick.*` 태그 예순 | 섭식 태그 없음·생태 시뮬 없음, 읽는 곳 없는 태그 금지 | ㉠ | 읽는 곳이 있는 열일곱부터(결정 2) |
| 4 | 새 필드가 기준값을 흔들지 않게 | 해시는 모든 필드를 걷는다 | ㉡ | 필드(`[B-7x1]`)와 셈(`[B-7x2]`)에서 한 번씩 옮기고, 매번 CSV로 세계가 같음을 보인다 |
| 5 | 시간 벡터를 부름에도 | 정체는 몸을 건너지 않는다 | ㉢→결정 4 | 백성에만 |
| 6 | 좌표 이동·레인 재연결 | 생성 뒤 그대로 | ㉠ | 레인의 단일 창구(결정 14·15) |
| 7 | 품는 존재 | 한 성계 한 주인 | ㉠ | `HostCivID` — 주인은 그대로(결정 6) |
| 8 | 포자·남은 기계·자연 발생 | 백성의 수는 건넘 단계에서만 | ㉠ | 요청을 건넘 단계가 받는다(결정 16) |
| 9 | 침공 무효·완전 면역·파괴 불가 | 플롯 아머는 없다 | ㉡ | 문의 사유와 유한한 감쇠 |
| 10 | 관측하면 붕괴 | 세계는 앎을 읽지 않는다 | ㉢ | 세계의 사실로 드러나고 표현만 가린다 |
| 11 | 역류하는 시간·미리 적용 | 시계 하나 | ㉢ | 얼굴과 기록의 인용만 |
| 12 | 칸을 주는 변이·다섯째 칸의 존재 | 칸은 업적 행으로만 | ㉢ | 변이는 태그일 뿐 |
| 13 | 경외와 기도를 먹음 | 경배를 받는 존재는 타락한다 | ㉡ | 조약의 수; 경배의 선택은 높아짐 |
| 14 | 법·언약·울타리·우주 상수가 됨 | 거룩한 질서는 엔티티가 아니다 | ㉢→㉡ | '남긴 법' — 왕좌가 빈 뒤에도 거두어지지 않는 칙령 |
| 15 | 결박하고 울타리를 치는 수호자 | 묶는 것은 `CosmicSeal`뿐 | ㉡ | 지킴과 결박 사업을 먼저 하는 AI |
| 16 | 실시간으로 조립되는 선택지 | 카드는 지어 둔 행 | ㉡ | 행의 조건이 블록을 읽는다 |
| 17 | 최종 결과 상태의 제목과 두 책의 비문 | 엔딩에는 제목이 없다, 두 책은 기록과 주장만 | ㉡ | 사실 줄의 낱말 하나(결정 13) |
| 18 | 사람·별의 수, 영어 별칭, 유목신·구원·영겁 | 규칙 11과 낱말 목록 | ㉡ | 설계 문서의 이름으로만 |
| 19 | 위기의 카운트다운을 먹고 해결 | 서 있는 위기는 `DelayHazard`만, 주장은 풀리지 않는다 | ㉡ | 놓일 때의 덤, 주장·절멸·전조 제외 |
| 20 | 별 등급 강등과 흑색 잔해 | 등급 일곱, 실제 하늘 | ㉠ | 일곱 안에서 한 단씩, 꺼 둔 채(결정 8) |
| 21 | 끊기지 않는 누적 셈(R2) | 누적 셈을 금하는 문장은 없다 — 연속 셈 넷은 선례일 뿐 | ㉡ | 새 꼴의 셈(결정 5) — R에서 뺐다 |
| 22 | 레인 시간을 백성마다 고침(R7) | 레인 시간은 규칙 행 하나(`Sim/CosmicWanderer.cpp:35-38` — 코드의 사실) | ㉠ | `LaneTicksOf`가 맥락을 받는다, B8 헤더와 한 번에(결정 17) |
| 23 | 반응형·갈림 카드(R8) | 수직 슬라이스의 수량(`docs/50_ROADMAP.md:12`) | ㉠ | 'B6b 끝의 수 + 청원 11·조우 1'(결정 11) |
| 24 | 포자의 흩어짐(R9) | 추첨 지점은 테스터와 시드를 나누기 전에만(`docs/20:218`) | ㉠ | 고정 수 하나, Phase 5 전(결정 22) |
| 25 | 이웃의 블록을 읽는 조건과 기생의 이웃 항(R11) | 국경 배율은 VS에 두지 않는다(#88) | ㉠ | 조건 하나·기생의 두 국경 항·표류 목표의 블록 항만 VS — 다른 국경 배율은 EA(결정 9) |
| 26 | 왕좌가 빈 뒤에도 남는 칙령(R12) | AI는 칙령을 지니지 않고 곧 철회한다(B2 헤더 §2.6 `:125`·`:129`) | ㉠ | `UnseenLaw` 백성만 — 주장 고리·위기의 답·법칙을 묻는 행 아래는 빼는 좁힘이 결정 18에 걸려 있다 |
| 27 | AI 백성의 씨앗 블록(R13) | 절차 특성은 `DT_Traits`와 함께(#36 ②), 생성은 시뮬 스트림(규칙 3 ①) | ㉠ | 파생 정수 용도 13, 노브가 비면 꺼짐(결정 23) |
| 28 | 품는 쪽에 가는 수입(R14) | 정착된 성계의 술어(규칙 13) | ㉠ | 슬라이스 다 헤더(결정 6·16) |

R 번호의 지금 문장·새 문장·넓이·지키는 시험은 헤더 §0의 표다(리뷰 F5·F4가 더하라고 한 행 — 21~28).

## 3. 새 열린 물음의 초안 (`OPEN_QUESTIONS.md`에 승인 커밋이 더한다 — 번호는 그때의 마지막 번호 다음부터 매긴다: `10205b6`의 끝은 #163이고 B6b 헤더가 #164를 잡았다; 아래 '새1'~'새8'은 자리표다 — 리뷰 I10)

| # | 결정 사항 | 권장안 | 영향 |
|---|---|---|---|
| 새1 | 블록의 뿌리와 '섭식 태그 없음' 판정(B7x 헤더 결정 2·3) | 기본값: `Brick.` 뿌리, 읽는 곳이 있는 것만 선언, 잎 이름은 머지 전에 고친다(리뷰가 권장 이름을 바꿨다 — 헤더 결정 3), 업적 행은 블록을 묻지 않는다(K4 — 헤더 §3.3 규칙 7). 대안: `Trait.` 아래 | §5.3·§5.11 |
| 새2 | 시간 셈의 주인과 술어(결정 4·5) | 기본값: 백성에만, 누적(㉡ — R에서 뺐다), 술어 여섯은 시작값; 끝의 꼴의 표시가 앞 생애의 장에 지금 셈을 붙이지 않게(결정 13). 대안: 부름에도 | §7.2 |
| 새3 | 갈림 카드와 대기 칸(결정 10·11) | 기본값: `Lottery`와 `CivLacksTrait`, 둘째 갈림의 ②는 되풀이 이득 없이(리뷰 F7의 (나)), 수량 줄은 'B6b 끝의 수 + 청원 11·조우 1' — 묶음에서 대기 칸 잃음(헤더 §5 ③)과 방랑자 갈림 카드가 증언·도착 카드를 밀어내는 몫(③′)을 잰다(#158과 함께) | §12.1 |
| 새4 | 레인의 창구와 실제 하늘(결정 14·15·17) | 기본값: 창구는 받고(고립 금지는 살아 있는 성계의 걸음에만 — 파괴의 끊음은 예외) 좌표는 둘째 걸음, 레인 시간은 B8 헤더와 한 번에; 레인 걸음이 주장을 풀거나 늦추지 않는다는 시험(K9); `SystemDragger`의 꼴 하나 | §6.4·§9.2 |
| 새5 | 판 도중의 탄생과 6a(결정 16·22) | 기본값: 건넘 단계의 요청, 포자에 고정 수의 추첨 하나 — 태어난 것은 새 백성(새 `CivID`·새 이름·셈 0, 부모의 행·특성·관계·칙령을 물려받지 않는다), 그 줄은 살아남음·돌아옴·다시 태어남을 말하지 않는다(헤더 결정 16의 받는 조건 — 리뷰 T4) | §7.4 |
| 새6 | 끝의 꼴 표지의 자리와 글(결정 13) | 기본값(리뷰 권장 ㉢ — 결정 13의 꼴 표지로, §0의 처리 ㉢과 뜻이 다르다; 헤더 §10 결정 13): 그 생애의 마지막 재위 백성이 `Civs`에 있을 때만, 같은 백성의 여러 생애는 마지막 장에만, '지금 살림'으로 읽히게 — 손 확인에서 칭호나 승리로 읽히면 뺀다(#163과 함께). 대안: ㉠ 통치가 끝날 때 저장(`ReignShapes`)·㉣ 백성이 주어인 자리에만 | §8.4 |
| 새7 | 본능 축과 `AIProfile`(노트 §1.4) | 기본값: B7 헤더가 여섯을 프로필의 낱말로 받는다 | §5.11 |
| 새8 | B7x의 값과 B6b의 경주(결정 1·23, 헤더 §5 판단 ⑥ — 리뷰 M1·I4) | 기본값: 어긋나면 B7x의 노브가 먼저 물러선다(씨앗 묶음에서 연구·통찰·에너지 블록을 빼거나 곱·더함을 낮춘다) — B6b의 r·c·N·P_bind·C(풀이는 헤더 §5 판단 ⑥의 끝 항)는 건드리지 않는다; 한 번 고쳐도 어긋나면 그 수와 함께 [개발자] | §9.3 |

**있는 행의 고침**(승인 커밋 — 헤더 §0의 R11·R13, 리뷰 F4):

| # | 지금(`10205b6`) | 끝에 더할 것 |
|---|---|---|
| 36 | ② '절차 특성(§3.1 "1~2개"·"3~5개")의 알고리즘 — `DT_Traits`와 함께' (`OPEN_QUESTIONS.md:33`) | 'B7x(헤더 결정 23 — R13)가 가장 작은 꼴을 앞당겼다: AI 백성마다 씨앗 블록 하나(파생 정수 용도 13, 노브가 비면 없음); 나머지는 `DT_Traits`와 함께' |
| 88 | '국경 배율 (EA 심사) — VS에 두지 않는다 …' (`:63`) | 'B7x(헤더 결정 9 — R11)가 VS에 연 것: 조건 `BorderingCivHasTrait`, 기생 블록의 두 국경 항, 표류 목표의 블록 항 — 다른 국경 배율은 여전히 EA' |

B2 헤더 §2.6(`docs/60_DECISIONS/headers/B2_edicts_and_research.md:125`·`:129`)에는 '(B7x — `Brick.Form.UnseenLaw`를 지닌 백성은 예외, B7x 헤더 결정 18·R12)'의 표지를 단다.

## 4. 다른 PC 세션에 붙여 넣던 문구

다른 PC의 문구는 브랜치 최상위의 `promptforother.txt`에만 있고 브랜치에서만 쓰인다 — `[B-7x0]`에서 지운다(리뷰 F12; 그 파일 스스로 '받은 뒤에는 지워도 된다'고 적었다). 차례는 헤더 §8이다. 그 문구의 충돌 목록(§4a)과 해소 규칙('양쪽을 다 살리고 main 쪽을 앞에 둔다')은 틀렸다(리뷰 I1) — 헤더 §8.1을 따른다.

## 5. 빌드 전 코드의 확인 목록

이 브랜치의 코드 커밋은 엔진이 없는 PC에서 읽어서만 썼다. 파이썬 검사 다섯(`check_layers`·`check_data`·`test_data_schema`·`make_config_reference --check`·`check_docs`)은 통과했고, 컴파일·자동화 테스트·에셋·기준값은 한 번도 돌리지 않았다. 경로는 `Source/Cosmic_Ascension/` 아래다.

### 5.1 이어진 자리 (`[B-7x2]`)

| 무엇 | 어디 |
|---|---|
| 누적 셈 여섯 — 연속 셈의 고리 바로 뒤 | `Sim/GalaxySimulationSubsystem.cpp`의 경제 단계 끝(`CosmicBricks::CountTimeVectors`) |
| 블록의 수입 몫(별에 매이지 않는 에너지·삼키는 블록의 곱·레인 폭·일 없는 틱·기생의 통찰) | 같은 파일, 틱당 수입을 적기 전(`AddIncomeShares`) |
| 연구의 곱(엮은 일손·안으로 접힌 둘) | 같은 파일의 연구 단계(`ResearchMultiplierOf`) |
| 미시 블록의 위기 몫 | 같은 파일의 위기 발생 몫(`HazardSpawnShareOf`) |
| AI의 변이 걸음 — 업적 판정 앞 | 같은 파일의 AI 단계(`MutateAIPeoples`) |
| 씨앗 블록 — 아키타입의 특성을 베낀 바로 뒤 | 같은 파일의 `NewGalaxy`(`SeedBrickOf`) |
| 유목 블록의 정착 진척 | `Sim/CosmicMegaprojects.cpp`의 `Advance`(`SettleProgressOf`) |
| 정착의 문 — `Inward` | `Sim/CosmicMegaprojects.cpp`의 `SettleBlockOf`·`SettleTargets`·`PeopleSettleBlockOf` |
| 기생 블록의 놓일 때 덤 | `Sim/CosmicHazards.cpp`의 `Place`(`ParasiteCountdownBonusOf`) |
| 삼키는·뒤엉킨 블록의 눈 — 표류 목표의 항 | `Sim/CosmicDiplomacy.cpp`의 `TargetRelationOf`(`RelationTargetTermOf`) |
| 남긴 법 — 칙령을 거두지 않는다 | `Sim/CosmicEdicts.cpp`의 `AIRevokesOf` |
| 조건 셋 | `Events/CosmicEventSubsystem.cpp`의 `Check` |

잇지 않은 것: 별 등급의 어두워짐(함수만 — 결정 8). 별 등급의 차례는 `[B-7x2r]`이 `Core/CosmicTags.h`(`StarClassAt`·`StarClassCount`) 한 벌로 줄였다 — 은하 생성·별의 에너지·등급 몫의 검사가 그것을 읽는다. 열일곱째 노브 `BrickStellarCrisisSpawnMultiplier`는 미시 블록의 위기 몫과 같은 자리(`HazardSpawnShareOf`)에서 별의 위기에만 곱한다.

읽어서 한 리뷰(`[B-7x2r]`)의 결론: 확실한 컴파일 오류는 찾지 못했다; 블록 태그가 없는 백성은 이어진 열 자리 모두에서 옛 셈을 옛 차례 그대로 간다(곱 1이나 더함 0을 옛 길에 끼워 넣지 않았다 — 갈래로 갈랐다). 그래도 한 번도 컴파일하지 않은 코드다.

### 5.2 먼저 볼 자리 (컴파일과 동작 — 있을 법한 차례)

1. **가려지는 이름**(빌드가 오류로 본다): `Tests/CosmicExistenceBricksTests.cpp`의 람다(`Sim.BrickShares`·`Sim.TimeVectorsCount`), `Sim/CosmicMegaprojects.cpp`의 `Advance` 안 `Step`.
2. **`TestEqual`의 오버로드**: int32·uint32·허용 오차 있는 float·FName에 쓴다 — 없으면 `TestTrue(a == b)`로.
3. **옛 세이브 테스트의 셈**: `Sim.SaveTimeVectors`가 바깥 이름 `TimeVectors`를 지우고 지운 수가 백성 수 이상임을 단언한다(구조체 형 이름에도 그 글자가 들 수 있어서); `HostCivID`는 성계 수와 꼭 같음을 단언한다.
4. **보드의 가정**: `Sim.BrickSettleGate`는 첫 틱에 백성 1만 성계 4에 정착할 수 있다고 본다(백성 2는 제 정착에 묶여 있고 백성 3은 보유 상한이다 — `bSettled[0]`을 본다); `Sim.MutationStep`은 `NewGalaxy` 뒤 첫 백성이 다스리는 백성이라고 본다(단언한다); `Sim.BrickShares`는 연구의 몫이 넷으로 고르고(`ResearchFields` 넷 — Physics·Society·Engineering·Transcendence, 각 1/4; `Sim/GalaxySimulationSubsystem.cpp:46`·`:880` — 리뷰 build-5) 물리가 0에서 시작한다고 본다('두 배'의 단언은 그래야 선다), 그리고 무동작 노브의 20틱이 비트 그대로라고 본다.
5. **패널 테스트**: `Settle.Block.Inward`의 글은 스트링 테이블 에셋을 다시 만들기 전에는 비어 있다 — 사유마다 글이 있는지 보는 테스트가 그때까지 실패한다.
6. **`BrickSeedTags`**: `DT_Rules`의 태그 묶음 칸(`ClaimLeaveProfiles`의 꼴) — `Data.Integrity`의 칸·형 검사가 받는지.
7. **외교 창의 툴팁**(`UI/CosmicEdictWidget.cpp`)은 표류 목표의 항을 손으로 늘어놓는다 — 새 항이 0이 아니게 되면(데이터 켜기) 보이지 않는다.
8. **절 번호**: 코드 주석·태그의 설명 글·`Tools/data/schema.json`의 까닭 글이 부르던 'B7x section E1~E6'(작업 틀의 번호)은 `[B-7x2c]`가 헤더의 절 번호로 고쳤다(E1 → §2.1·§2.2, E2 → §2.3, E3 → §2.4·§2.5, E4 → §2.6, E5 → §2.7, E6 → §6) — 남은 것이 있으면 같은 대응으로 고친다(주석과 글만).
9. **시험 보드가 게임 값을 물려받는다**(리뷰 build-1·DET-3·I7): `LayBoard`(`Tests/CosmicExistenceBricksTests.cpp:310`)와 `Sim.SeedBricks`의 Pass 0(`:1127`)이 `DT_Rules`·`DT_Civs`를 읽는다 — `[B-7x5a]`의 값을 켜면 B7x의 테스트 셋과 있던 테스트 하나(`Sim.DiplomacyRules`)가 깨진다. 고침은 §5.4 시1·시2.

### 5.3 다른 PC에서 할 일 (차례 — 리뷰 뒤의 차례는 헤더 §8: `[B-7x1]`·`[B-7x2]`를 하나로 합치고, 데이터는 `[B-7x4m]`·`[B-7x4]` 뒤)

1. 에디터를 닫고 전체 빌드(새 `USTRUCT`, 열거자, 헤더).
2. `Tools/make_data_tables.py`로 에셋과 `Tools/data/generated.sha1`을 다시 만든다(`DT_Rules`의 새 칸, `ST_CA`의 새 키 — 그때까지 `Data.SourcesInSync`가 낡았다).
3. 자동화 테스트 `Cosmic_Ascension.*` 두 번.
4. `[B-7x1]`은 다섯 줄의 두 해시를 옮긴다 — CSV가 `TickMicros`·`SaveBytes`·`SaveRawBytes`를 빼고 같음을 본 뒤 `Tools\run_checks.ps1 -Tier full -UpdateBaseline`. `_rows.csv`·`_claims.csv`와 `ending:`·`race:` 줄도 같아야 한다(리뷰 I9 — 헤더 §8).
5. `[B-7x2]`는 누적 셈이 쌓이므로 두 해시를 한 번 더 옮긴다 — CSV가 `TickMicros`·`SaveBytes`·`SaveRawBytes`를 빼고 `[B-7x1]`의 것과 같음을 본 뒤 다시 `-UpdateBaseline`(두 커밋을 하나로 합쳐 한 번만 적는다 — 헤더 §8). 선택: 해셔 탐침의 비트 그대로(헤더 §8 차례 3 — 리뷰 DET-2).
6. 변이 패치(`Tools/mutations/B7x/`)와 README의 줄: 초기값 1, 해시가 `TimeVectors`를 건너뜀, 셈이 되돌아감, `Inward`를 묻지 않음, 기생이 주장에 닿음, 남긴 법의 첫 물음을 뺌, 씨앗이 시뮬 스트림에서 뽑음. 리뷰가 더한 변이는 §5.4(시3·시4·시5·시6·시7과 `[B-7x4m]`의 둘).
7. 헤더 §7의 문서: `docs/20`(규칙 4의 새 걸음, 저장 필드, 용도 13 — 규칙 3의 세 문장)·`docs/21`·`docs/40`의 테스트 표·`STATUS.md`(`[B-6b12]` 끝의 수 + 9 — 리뷰 I10)·`DECISIONS_LOG`. `CLAUDE.md` 규칙 13은 슬라이스 나의 몫이다(헤더 §7 — 리뷰 F12).

### 5.4 리뷰가 넘긴 고침 (리베이스 때 — 대상 커밋마다)

ultracode 리뷰(2026-11-09) 보고서 §3.2~§3.4의 고침이다 — 이 판(`[B-7x0]` 전의 글 고침)은 코드·데이터·테스트를 고치지 않았다(이 브랜치는 빌드할 수 없고, 고친 코드는 리베이스한 커밋이 컴파일·테스트로 증명해야 한다). 묶음 이름은 리뷰 보고서의 것이다 — 앞글자는 문(문서 글)·코(코드)·데(데이터와 스키마)·시(테스트와 측정 장치 — 자동화 테스트·변이·러너)이고, 번호는 보고서 §3의 차례이며 아래 항목의 굵은 이름(**코1** 등)이 같은 번호다. 괄호는 발견 번호, '대기 N'은 헤더 §10 결정 N의 답을 기다린다는 뜻이다. partial 발견은 검증자가 고친 안을 적었다. 커밋의 차례는 헤더 §8, 줄은 달리 적지 않으면 branch `0bffb8e`의 것(경로는 `Source/Cosmic_Ascension/` 아래)이다.

#### `[B-7x0]` 승인 커밋 — 이 판이 고칠 수 없는 살아 있는 문서

1. `OPEN_QUESTIONS.md`의 #36·#88 행 고침과 새 행(이 노트 §3), B2 헤더 §2.6의 B7x 표지, `docs/10_DESIGN/systems.md:381`의 번복 표지, `docs/50`의 B 행과 수량, `DECISIONS_LOG`의 B7x 절, `STATUS.md`(F4·F5 — R1·R8·R11·R12·R13). 증명: `check_docs` 0.
2. `promptforother.txt`를 지운다(F12). 증명: 파일이 없고 노트 §4·설계서 §5가 그것을 부르지 않는다.
3. `docs/OVERVIEW_as_of_B6a.html` — ㉮ 뺀다 / ㉯ 받은 꼴로 다시 쓴다(T1) — **대기 21**.
4. 설계서 원안의 동결본(`docs/archive/EXISTENCE_EVOLUTION_original_2026-11-09.md`)과 §4.4의 '§4.2 원안의 줄' 열(T5·T3) — **대기 21**(열은 13과 함께).
5. 결정 3·13·18의 답을 헤더·노트·설계서의 글에 — 헤더 §10의 문장 그대로(T2·F10, F9·DET-5·T3·I12, F1·I2·M11·T10) — **대기 3·13·18**. 18이 권장이면 §2.4 행·설계서 C10의 귀결·노트 §1.1 `UnseenLaw` 행에 좁힘의 한 줄, 노트 §1.1 `QuantumCloud` 행의 고침(T10).

#### `[B-7x1]` 필드 (헤더 §8 — `[B-7x2]`와 하나로 합친다)

1. 문13(DET-1): `docs/20:204`·`:208`·`:220`의 씨앗 문장 — 헤더 §7의 글 그대로, 코드 앞에. 증명: `check_docs` 0.
2. 코7의 태그·접근자(T2·F10) — **대기 3**: 결정 3의 이름으로 ini(`Config/DefaultGameplayTags.ini:172-184`)와 `Core/CosmicTags.h`의 접근자, 스키마 규칙 4의 나열을 바꾼다. 증명: `Sim.SaveTimeVectors`·`Data.Integrity`; ini는 `generated.sha1`에 들지 않는다(main `Tools/make_data_tables.py:267`).
3. 결정 13 ㉠(결정 13의 꼴 표지 — 통치가 끝날 때 저장한다)이면 `FLifeRecord`의 끝에 `TArray<uint8> ReignShapes`(`RuledCivIDs`와 같은 길이, 초기값 빈 배열 — 규칙 7)(DET-5) — **대기 13**. 증명: 두 해시가 이 커밋에서만 옮고 옛 세이브가 빈 배열로 읽힌다.
4. 결정 6 (ㄷ)이면 탄생 요청 필드(F8) — **대기 6·16**. 증명: 3과 같다.
5. 증명의 꼴(I9): 다섯 판의 CSV·`_rows.csv`·`_claims.csv`(`TickMicros`·`SaveBytes`·`SaveRawBytes` 빼고)와 `ending:`·`race:` 줄이 앞 커밋과 글자 그대로.

#### `[B-7x2]` 장치 (492c4f0 + 5969bd5 + 85f297c를 하나로)

- **코1**(F1·I2·M11·F4·T6·F11) — **대기 18**(권장안은 상시 승인 안): `Sim/CosmicEdicts.cpp:344-367`의 `AIRevokesOf`를 아래로 바꾼다. main에 `Sim/CosmicEdicts.h:104` `AsksForLawsOf`, `Data/CosmicDataTables.h:131` `IsHazardAnswer`, `Data/CosmicDefinitionRows.h:426` `TMap<FGameplayTag, int32> HazardCountdownBonus`가 있다.

```cpp
TArray<FName> CosmicEdicts::AIRevokesOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules)
{
	TArray<FName> Revokes;
	// A people that keeps the edicts it holds when no one rules it (B7x [B-7x2], Brick.Form.UnseenLaw) keeps them - except an
	// edict that adds to the claim's countdown or answers a crisis (K9, the line B6b decision 9 drew for galaxy laws), and
	// except while its row asks for galaxy laws (the slots are the laws' places, B6b decisions 10-11).
	const bool bKeepsEdicts = Civ.Traits.HasTagExact(CosmicTags::BrickFormUnseenLaw()) && !AsksForLawsOf(Civ, Tables);
	for (const FName Row : Civ.ActiveEdicts)
	{
		const FEdictDef* Def = Tables.FindEdict(Row);
		// A galaxy law stays (B6b decision 11): the AI's law step enacts the laws its row asks for, and keeps them.
		if (!Def || Def->Scope == ECosmicEdictScope::HomeGalaxy)
		{
			continue;
		}
		if (bKeepsEdicts)
		{
			if (Def->HazardCountdownBonus.Contains(CosmicTags::HazardClaim()) || Tables.IsHazardAnswer(Row))
			{
				Revokes.Add(Row);
			}
			continue;
		}
		if (!HasStabilityCost(*Def) || Civ.Stability < Rules.AIEdictRevokeStability)
		{
			Revokes.Add(Row);
		}
	}
	return Revokes;
}
```

  - 같은 커밋: `Sim/GalaxySimulationSubsystem.cpp:2954-2956`의 TickCivAI 주석('The AI holds no edict of the people's scope') 끝에 '- except a people that carries Brick.Form.UnseenLaw (B7x), which keeps what CosmicEdicts::AIRevokesOf leaves it'; `CosmicEdicts.h`의 `AIRevokesOf` 선언 주석에 같은 예외. 시험: 시4. 대안('모든 칙령'): 코드는 그대로, K9를 굽히는 새 R, 신정론 불변식에 그 경우, W 다시 셈.
- **코2**(M9·I5): `Sim/CosmicExistenceBricks.cpp:395` → `if (Owner.Traits.HasTagExact(CosmicTags::BrickSpacePlanckMicro()) && !IsSparedHazard(Hazard))`; `Sim/CosmicExistenceBricks.h:111-114`의 문서 주석에 'never the claim, the extinction or the supernova precursor'. 별의 위기 곱(`BrickStellarCrisisSpawnMultiplier`)은 설계대로 둔다. 시험: 시5. (절멸에도 감쇠를 원하면 [개발자] — B6b 결정 2~5와 경주 ② (나)에 닿는다.)
- **코3**(DET-4·M10·build-2): `Sim/CosmicExistenceBricks.h`에 다른 `CosmicBricks` 선언과 같은 꼴로 `int32 ParasiteBonusBoundOf(const FCosmicRulesDef& Rules, const FGameplayTag& Hazard)` — 태그가 유효하지 않거나 `IsSparedHazard(Hazard)`면 0, 아니면 `FMath::Clamp(Rules.BrickParasiteCountdownBonusTicks, 0, 100000)`. `ParasiteCountdownBonusOf`(`:337-339`)가 그것을 쓴다 — 한 위기에 한 번, 기생 수만큼 더하지 않는다. 스키마 max(branch `Tools/data/schema.json:247` 100000)와 지킴 상한으로 묶는 안은 하지 않는다(F2의 그 부분은 반박 — 칙령 덤 열도 상한이 없고 #132는 함대 몫의 띠다). 시험: 시3.
- **코4**(DET-6): `Sim/CosmicDiplomacy.cpp:288-295`의 자르기 → `if (BrickTerm != 0.f) { Target += BrickTerm; }`; 주석(`:288-289`)은 'a term added like the stance term; the drift stage clamps the relation'; `CosmicDiplomacy.h:109`에서 '- when it is not 0 the sum is clamped to [-1, 1]'를 지운다. `GalaxySimulationSubsystem.h:501`의 'within [-1, 1]'은 표류 결과를 말하므로 그대로. 시험: 시7.
- **코5**(DET-7): `Core/CosmicTags.h:305` → `return Tag.IsValid() && Tag.GetTagName().ToString().StartsWith(TEXT("Brick."), ESearchCase::IgnoreCase);`(main `Player/PlayerRoleSubsystem.cpp:547`·`Log/CosmicChronicle.cpp:230`의 꼴; 계층으로 묻는 길 `Tag.MatchesTag(Root) && Tag != Root`도 엔진에서 된다 — UE_5.8 `GameplayTagsManager.cpp:1541-1556`·`:2409-2413`). 새 단언은 없다 — 데이터 검사가 이미 대소문자를 막는다(main `Tools/cosmic_data_schema.py:679`·`:1012`); `Sim.SeedBricks`가 그대로 서는지 본다.
- **코6**(DET-8·T6·F11·DET-1) — 주석과 글뿐, 비트 그대로: `Data/CosmicDefinitionRows.h:1411-1412` → '...and with these values a people that carries a share brick lives as before too - but not one folded inward (Brick.Space.HermitMonolith / PlanckMicro: the settle gate's Inward, CosmicMegaprojects) or one that keeps the unseen law (Brick.Form.UnseenLaw: CosmicEdicts::AIRevokesOf); those act on the tag alone.'; ini `:173` → 'Form brick: a people whose held edicts outlive an empty throne - no regent or AI of it revokes one (CosmicEdicts::AIRevokesOf), save those that touch the claim or answer a crisis. Never a people that becomes law, never the holy order (K6). Read by AIRevokesOf, the end shape (KeptLaw) and conditions (B7x). Saved name, never renamed.'(이름은 결정 3, 'save those…'는 결정 18의 답에 맞춘다); ini `:172` → 'Form brick: a people that shut its star inside a sphere and turns its heat to reckoning. …'(읽는 곳은 지금대로); 테스트 메시지 `Tests/CosmicExistenceBricksTests.cpp:658`·`:1039` → 'The shut sphere round its star …'; `Core/CosmicSeed.h:3-6`의 머리 목록에 ', the seed brick of an AI people at generation (B7x - a saved trait the sim stream does not draw; nothing while DT_Rules.BrickSeedTags is empty)'.
- **코7**의 노브·함수(T2·F10) — **대기 3**: `FCosmicRulesDef`·`DT_Rules.json`·`docs/30`의 노브 이름, `IsQuietOf`, 테스트(`:576-591`·`:923`·`:960`).
- **데3 — K4 업적 규칙**(F6): `DT_Feats` rules에 하나 — why 'K4 - a rung comes from a feat's deed, never from a brick or a mutation (B7x header K4)'; `lacks_type` `BorderingCivHasTrait`, 열일곱 `Brick.` 태그마다 `lacks_cond` {`CivHasTrait`, Brick.X}와 {`CivLacksTrait`, Brick.X}(규칙 4의 나열 꼴). `CivTimeVectorAtLeast` 금지는 K4의 글 밖이라 넣지 않는다; `AscensionCost`의 `AddTrait`/`RemoveTrait Brick.*` 금지는 선택. 시험: 시8.
- **데3 — `Settle.Block.Inward`**(I8): `string_table.checked_prefixes`의 `Settle.Block.RealmCapLast` 뒤에 `"Settle.Block.Inward": {"forbidden": ["verdict","holy_words","rescue_words","rung_grasp_words","title_words"], "no_digits": true}`. 시험: 시8.
- **시1**(DET-3·build-1): `Tests/CosmicTestHelpers.h`에 `WithInertBricks(Rules)`(Rules를 베끼고 수 노브 열여섯을 기본 생성한 `FCosmicRulesDef`의 값으로 되돌리고 `BrickSeedTags.Reset()`)와 `StripBricks(State)`(각 `Civ.Traits`에서 `CosmicTags::IsBrick` 태그를 걷는다; 아키타입 태그는 둔다; 0번 백성의 `Brick.Form.Society`도 걷힌다). `Tests/CosmicExistenceBricksTests.cpp:310`은 `WithInertBricks(WithoutStartingEdict(*Game))`, `NewGalaxy(42, 12, 4)` 바로 뒤에 `StripBricks(State)`. 게임 규칙을 읽는 공유 시드 42 보드(main `Tests/CosmicDiplomacyTests.cpp:132`의 `LayBoard`부터)에도 같은 둘 — `:389`의 `Sim.DiplomacyRules`는 −0.12 → 0.13으로 깨질 것이다; 그다음 데이터를 다시 만든 에셋으로 모음 전체를 한 번 돌려 나머지를 찾는다.
- **시2**(DET-3·build-1·I7): `Sim.SeedBricks`의 Pass 0은 `WithInertBricks(*Game)`로, Pass 1은 그 사본에 `Seeding.BrickSeedTags = Rules.BrickSeedTags`를 준다. 기대값은 백성마다 '아키타입 행 `Traits`의 `Brick.*` 수 + (Pass 1이고 CivIndex > 0이면 1)'(AI 아키타입 `arch_default`에는 블록이 없다).
- **시3**(DET-4·M10·build-2) — **`[B-6b8b]` 뒤**: `Tests/CosmicDataTests.cpp` `TheodicyProblems`의 칙령 고리 뒤에 `Longest += CosmicBricks::ParasiteBonusBoundOf(Rules, Tag);`와 `#include "Sim/CosmicExistenceBricks.h"`. 주석: '…and the crisis parasite's placement bonus (CosmicBricks::ParasiteCountdownBonusOf - once a hazard, never for the claim, the extinction or the supernova precursor, B7x); the ThroneLockTicks bound is unchanged while only claim cards delay.' Expect: main 데이터에서 역병(카운트다운 200 + 지킴 상한 100)에 덤 400이면 70슬롯, 0.98^70 ≈ 0.243 < 0.25라 문제여야 하고, 덤 30은 통과한다. B6b 끝의 데이터로 다시 센다 — `[B-6b7]`의 법칙 덤 50이 들면 역병의 가장 긴 머묾이 350이 되어 깨지는 문턱이 덤 약 330~340으로 내려간다(B6b 노트 §2 ⓔ '은하 법칙의 덤'(`10205b6` `:625`)의 덤 상한 — 역병의 (300 + B)가 68슬롯 이하이려면 B ≤ 380 — 에서 `[B-6b7]` 법 행의 덤 50을 빼면 약 330; `[B-6b7]`이 커밋한 `Tools/data/DT_Edicts.json`의 값과 `TheodicyProblems`(`Tests/CosmicDataTests.cpp`)로 다시 센다). 변이: `Tools/mutations/B7x/`에 덤 400 → `crisis_plague`에서 `Data.Integrity`가 깨지는 패치.
- **시4**(F1·I2·M11) — **대기 18**: `Sim.MutationStep`(`:987-998`)을 넓히거나 새 `Sim.KeptLawEdicts`. (a) UnseenLaw AI 백성은 `edict_vigil`과 `edict_machine_rights`를 거두고 `edict_mobilization`은 지닌다. (b) UnseenLaw AI 백성(Society 1.0 이상 — 자리 셋, 넷째 칸, `CivLawsHeldTicksAtLeast`를 지닌 행 아래, 안정도 `AIEdictRevokeStability`(1.0) 아래, 유지비 없는 백성 칙령 하나와 넉넉한 비축)이 AI 단계 뒤 법칙 셋을 지니고 `HoldsGalaxyLawsOf`가 참이다; 대조로 좁힘을 빼면 법칙이 둘에서 멈추고 `LawsHeldTicks`가 0이다. 변이 `b7x2_unseen_law_keeps_claim_edicts`·`b7x2_unseen_law_keeps_slots`.
- **시5**(M9·I5): `Sim.BrickShares`에 경우 하나 — PlanckMicro 주인의 `Hazard.Extinction`·`Hazard.PreSupernova` 몫이 블록 없는 주인과 같다. 변이 `b7x2_inward_damps_extinction`.
- **시6**(build-4) — 결정 8이 '넣지 않는다'면 함수와 함께 빠진다: `Tests/CosmicExistenceBricksTests.cpp:709`('one class') **뒤에**(`:708` 바로 뒤면 `:709`가 깨진다) — `StarClass = StarClassAt(4)`로 되돌린 뒤, 정착 중(`ActiveMegaproject = CosmicRows::ProjectSettle` → TestFalse 'not while the capital is being settled', 다시 `NAME_None`), 남의 수도(`Civs[1].HomeSystemID = Systems[6].SystemID` → TestFalse 'not a capital another people owns', 되돌림), 사람 없음(`DimCapitalStar(*Board, Index, INDEX_NONE)` → TestFalse), TestTrue('(the class stayed)'). 변이: 주인·정착 가드를 뺀다.
- **시7**(DET-6): `:678`은 `Before - 0.4f`를 Clamp 없이(1e-6) 기대하고, `:692-693`의 '[-1, 1] 안'은 지우거나 바꾼다. 기본 목표가 −1 아래인 보드(윤리 거리 2 + 적대 태세)에서 삼키는 항이 목표를 항만큼 정확히 내리는지 본다.
- **시8의 `[B-7x2]` 몫**(F6·I8): `Tools/test_data_schema.py`에 깨지는 경우 — 업적 행에 `CivHasTrait Brick.Mutation.<변이>`(이름은 결정 3 뒤의 것), `Settle.Block.Inward`에 금지어; `Settle.Block.Inward`의 금지 목록은 핀으로 박는다.
- **시12**(I6): 다시 뜰 다섯 — `B3/b3b_no_trade_income`, `B6b/b6b2a_data_laws_knob_four`, `b6b2a_endured_window_inclusive`, `b6b2a_laws_counted_before_upkeep`, `b6b2c_delay_second_rung`. `time_vectors`를 `words`의 끝에 두면 `b6b5_schema_holy_passive_unextended`와 `b6b5_schema_title_without_messiah`는 산다. `[B-6b6]`~`[B-6b8]`의 새 패치(Rule W, 시대·증언 행, ark_words)는 리베이스 뒤 apply 전후로 다시 견준다.
- **충돌의 해소**(I1): 헤더 §8.1 — `ST_CA.csv` 끝, `words`(`time_vectors` 한 줄만 끝에), `walked` 다시 셈.

#### `[B-7x4m]` 측정 (새 커밋)

- **코8**(M2·M4·M5·M6·M7·M8·M12·I7·I3): 통치자·방랑자 정책의 건너뜀('갈림 카드에서는 ③만'), `-Crossroads=<k>`와 `petition_cross_law_stays` ①의 `DeposeRuler` 통과, `-WithoutB7xData=1`·`=2`, `CosmicTests::WithoutB7xData`, `bricks:` 줄(측정 쪽 `OnEventFired` 셈, 사람마다·틱마다의 셈, 갈림 답의 틱, `petition_walls`·`petition_bread`의 온 수), `-SaveAt`의 거절 목록 — 명세는 헤더 §5. 도우미의 트립와이어: `Data.Integrity`가 뺀 집합(카드 열둘·사건 아홉·Society·노브 열일곱의 초기값·빈 `BrickSeedTags`)을 정확히 핀으로 박는다. 증명: 비트 그대로(다섯 줄·해시 판 둘, `-TellAll=0` 판 포함); 변이 둘 — 건너뜀을 뺌(테스트 안에서 지은 벤치 카드로), 끈 스위치가 씨앗·노브를 남김.

#### `[B-7x3]` 표현

- **시9**(F9·DET-5·I12·T3) — **대기 13**: `UI.Chronicle`에 지워진 백성·다스린 적 없는 생애·두 백성을 다스린 생애의 경우.
- **데3 — `Shape.`**(T7·I8) — 키가 처음 드는 커밋(`[B-7x3]`)이나 `[B-7x4]`: 헤더 §3.2의 목록과 `shape_merit_words` `["견디", "남은"]`; 시8에 `shape_merit_words`의 깨짐 하나와 걸린 목록의 핀.

#### `[B-7x4]` 글

- **데2**(T8·T7) — 노트의 초안 감사, 행에 옮기는 것은 `[B-7x5c]`: C10의 제목(branch `Tools/data/DT_Encounters.json:3048`, 설계서 C10) '비워 둔 왕좌' → '거두지 않은 칙령'; C10 ①의 SuccessText(`:3113`) → '{Civ}|hpp(은,는) 정해 둔 칙령을 손대지 않은 채 두었다. 칙령을 정한 이는 자리에서 내려왔다.'(둘 다 main `Tools/data/schema.json:72` dawn_words의 '왕좌'·'다스리'를 피한다 — ⑩, 헤더 §3.5의 빈 자리 규칙; 보고서 문안 '왕좌 없이 서는 칙령'·'다스리던 이는'을 고쳤다); C10 ②의 `ShiftEthics Spiritualism +0.05`(`:3150-3156`)를 빼고 설계서 C10의 목소리 줄에 '다스리는 이의 이름을 높이는 일은 윤리 축을 옮기지 않는다(`petition_exalt`)'(설계 글이다 — 게임 글로 옮기면 '다스리'를 같은 까닭으로 바꾼다); 목소리의 '견딘 날들 내내'(`:3185-3186`) → '눌린 날들 내내', `petition_cross_into_dust`의 '견딘 날이 길었다' → '떠도는 날이 길었다'(설계서 C08도). C11 ①의 값(`ChangeStability +0.08`)은 지금 고치지 않는다 — 헤더 §5 ⑧ 뒤에 정한다. 증명: 세계 해시 그대로, ultracode 글 감사.
- **데3 — 갈림 낱말 규칙**(T9) — `[B-6b5]` 뒤: `crossroads_words`를 한 번 적고, Conditions에 `CivTimeVectorAtLeast` 또는 `BorderingCivHasTrait`를 지닌 행에 건다 — DT_Encounters의 갈림 열둘(Title·Body·VoiceBodies·Choices.Label·SuccessText·FailureText), DT_Events의 `event_brick_*` 아홉, DT_LogTemplates의 `log_brick_scan_spread`; 걸 목록은 silence_words·rung_grasp_words·title_words·crossroads_words. 시8: 목록마다 깨짐 하나.
- F11: ini의 DevComment도 글 감사의 대상이다.

#### `[B-7x5s]` 씨앗·노브

- **데4**(M8·build-1·I11) — **대기 20**(권장 차례 그대로면 이대로): aa09543의 `DT_Rules` 몫(노브 열일곱의 값·`BrickSeedTags`·변이 노브)만 옮긴다. 그 전까지 데이터의 노브는 무동작, `BrickSeedTags`는 비어 있다.
- **시11의 `[B-7x5s]` 몫**(I7·I3): 헤더 §6의 '뜻이 바뀌는 테스트'를 돌리고 규칙대로 둔다.
- 증명: 깨질 수 있는 확인 ⓐ~ⓒ(=1), 값보다 먼저 재는 것과 문턱의 규칙, 판단 ⑥.

#### `[B-7x5c]` 카드·사건·Society

- **데1**(F7·M7) — (나)가 기본, (가)는 **대기 11**: (나) `petition_cross_dust_band`·`_star_line`·`_between_stars`·`_into_dust`·`_law_stays`·`_seam`의 ②에서 되풀이 이득을 뺀다 — 적어도 `_law_stays` ②의 `PlayerStatDelta Exaltation +1`; `petition_road`·`petition_feast`처럼 값과 작은 효과만 남긴다. (가) ②에 `SetFlag Flag.Cross.<행>`, 행 조건에 `FlagNotSet Flag.Cross.<행>`(branch `Data/CosmicConditionTypes.h:62-63`·`:168`에 이미 있다), ini에 태그 여섯(DevComment와 함께), 헤더 §2.11의 '열일곱' → 23. 어느 쪽이든 `schema.json:1273-1275` 규칙의 `then`은 넓히지 않고(검사기에 OR 꼴이 없고 필요도 없다) `why`만 바꾼다 — (나): 'a crossroads petition closes on its branch: the first by taking Brick.Form.Society, the second by the tag its first choice gives; its choice 2 carries no repeatable gain; a dismissed card (no cost, no effect) comes again after the last three answers', (가): '… the second by the tag its first choice gives or (choice 2) by Flag.Cross.<row>; …'. 설계서 C05~C11의 ② 줄도 같은 커밋에서 고친다.
- **데2**를 행에 옮긴다(`[B-7x4]`의 감사한 글을 글자 그대로).
- **데5**(I3) — 조건부: 판단 ③′가 넘으면 `enc_cross_drift_fleet`의 `BaseWeight`를 0.25로 낮추거나 `CivTierAtLeast 2`를 더한다.
- **시10**(F7): `Sim.CrossroadsPresent`에 '② 뒤 되풀이 이득이 없다'((나)) 또는 '② 뒤 그 카드가 다시 오지 않는다'((가)); `Sim.KnowingInert`에 갈림 카드의 경우.
- **시11의 `[B-7x5c]` 몫**(I7·I3): `Sim.WitnessCard`(`[B-6b8]` — 게임 조우를 쓴다)를 다시 돌리고, 깨지면 가장 바깥에 `WithoutB7xData`; B6b 노트 §5에 '증언 카드는 더는 유일한 방랑자 상시 카드가 아니다'.
- 증명: 깨질 수 있는 확인(=1과 =2), 판단 ⓪~⑤·③′·⑦·⑧, ⑥ 다시.

#### `[B-7x5r]` — 판단이 노브를 부를 때만

- 고침의 규칙(헤더 §5 — 탐침은 시드 1~30, 판단은 새 시드 31~60). `STATUS.md`의 기준값 사실을 이 커밋이 고친다(M1).

#### 슬라이스 다 `[B-7x7]`·`[B-7x8]`

- DET-9(헤더 결정 6 (ㄱ)): 품는 백성이 지워지면 모든 성계의 `HostCivID`가 `NAME_None`이 된다 — 테스트 한 줄을 예약한다.
- **데3 — 탄생 행의 낱말 규칙**(T4) — **대기 16**: rebirth_words·afterlife_words·'불사'·'유령'.

**넘김의 수**(대상 커밋마다): `[B-7x0]` 5(대기 3 — 21·21·3/13/18), `[B-7x1]` 5(대기 3 — 3·13·6/16), `[B-7x2]` 19(코1~코7·데3 둘·시1~시8의 몫·시12·충돌; 대기 3 — 코1·시4는 18, 코7은 3), `[B-7x4m]` 1, `[B-7x3]` 2(대기 1 — 13), `[B-7x4]` 3, `[B-7x5s]` 2(대기 1 — 20), `[B-7x5c]` 5(대기 1 — 11의 (가)), `[B-7x5r]` 1(조건부), 슬라이스 다 2(대기 1 — 16). 모두 45.

### 5.5 리뷰의 기록 (ultracode 적대 리뷰, 2026-11-09)

- **대상**: `origin/design/b7x-composable-existence`(tip `0bffb8e`, 바탕 `cef8cc7`). 견준 main은 리뷰 동안 커밋된 `10205b6`(`[B-6b5]`). 남은 B6b는 `[B-6b6]`~`[B-6b12]`다.
- **렌즈별 수**:

| 렌즈 | 발견 | confirmed | partial | refuted |
|---|---|---|---|---|
| 설계·규칙 (F1~F12) | 12 | 7 | 5 | 0 |
| 결정론·저장·해시 (DET-1~9) | 9 | 4 | 5 | 0 |
| 신학·서사 (T1~T10) | 10 | 4 | 6 | 0 |
| 측정·밸런스 (M1~M14, M5b) | 15 | 7 | 8 | 0 |
| 빌드 (build-1~5) | 5 | 1 | 3 | 1 |
| 통합 (I1~I12) | 12 | 9 | 3 | 0 |
| **합** | **63** | **32** | **30** | **1** |

- **판정**: 이대로는 승인할 수 없고 조건부로 승인할 수 있다. 처음 매긴 무게로 high는 F1·T1·M1·M2·build-1·I1·I2(I1은 검증에서 일부가 반박되어 내려갔다). 남긴 62건은 모두 좁은 고침이고, 고칠 것은 41묶음(문1~문16·코1~코8·데1~데5·시1~시12)으로 모였다.
- **K1~K10이 코드에서 서는가**: K1·K2·K5는 선다. K3은 서되 `docs/20` 규칙 3의 문장을 고친다(DET-1 → R13). K4는 데이터의 우연으로만 선다(→ 헤더 §3.3 규칙 7). K6은 코드는 서고 글(OVERVIEW html·설계서 원안·DevComment)이 깬다. K7은 서되 `HostCivID`를 비우는 규칙이 빠졌다(결정 6 (ㄱ)). K8은 서되 신정론 불변식이 기생 덤을 세지 않는다(시3). K9는 깨진다 — UnseenLaw의 `AIRevokesOf`(코1, 결정 18); 간접 경로는 적고 잰다(헤더 §2.4·§5 ⑦). K10은 저장에서 서고 표시에서 흔들린다(결정 13).
- **R은 좁고, 적히고, 시험되는가**: 넓다고 본 R은 없다. 적히지 않았고 시험되지 않았다 — 헤더 §0의 표가 고쳤다. 빠진 R 셋(#88·B2 §2.6·#36 ②)은 R11~R13으로 더했고, 슬라이스 다의 수입 몫을 R14로 두었다.
- **통째로 반박된 것**: build-3 — `UI/CosmicSystemPanelWidget.cpp`가 충돌로 멈춘다는 주장(3방향 병합 0 충돌, `ECosmicSettleBlock`은 Count 앞 11개라 12가 옳다).
- **남긴 발견 안에서 반박된 주장**(한 줄씩):
  - F1의 'W가 는다'(철야 100은 이미 W 안).
  - F2의 지킴 상한·스키마 max 300(#132는 함대 몫의 띠이지 합의 상한이 아니다 — 칙령 덤 열에도 상한이 없다), 그리고 간접 경로가 K9를 깬다는 것(철야의 반란 +200·격리의 역병 ×0.1은 main에도 있다 — 헤더 §0 K9의 범위).
  - F6의 `CivTimeVectorAtLeast` 금지(K4의 글 밖).
  - F7의 `then` 넓힘(검사기에 OR 꼴이 없다).
  - F8 (6)의 '요청은 열째 단계에서'(헤더에 없는 전제 — 물을 것은 자리).
  - F11의 'DevComment가 규칙 11을 깬다'(화면 글이 아니다 — T6의 고침은 남는다).
  - DET-1의 '결정 22로 올린다'(③에 생성 때의 파생 정수가 이미 있다).
  - DET-2의 '[B-6b1]은 해셔 탐침으로 증명했다'(CSV와 `_rows`로 했다).
  - DET-5의 '기록이 흔들린다'(첫 열림은 맞다 — 결함은 정하지 않은 백성과 다시 열 때).
  - DET-7의 '지금 갈린다'(데이터 검사가 막는다 — 잠재 위험).
  - DET-9의 'CivID 재사용'(`NextCivSerial`은 단조).
  - T2의 'hermit'(이미 쓰는 낱말 — `ShutHouse`는 꼴 키와 겹친다).
  - T3 ③의 사슬(해석).
  - T4 (3)의 반드시 막기(선택지로 둔다).
  - T5의 'CLAUDE.md의 표지 없는 문장'(설계서 머리의 표지가 덮는다 — CLAUDE.md에 줄을 더하면 크기 예산을 넘는다).
  - T7의 '오래 버틴 살림'('버틴'은 desolation_words).
  - T8 (c)의 지금 낮춤(측정으로 정한다).
  - M3의 문턱 셋(노브는 하나).
  - M5의 Named 카드를 'standing cards lost'로 셈(그 셈에 들지 않는다).
  - M7의 S04·S12를 `-Crossroads` 칸에서(표준 칸에서 선다).
  - M11의 '법 0'(법칙 둘은 선다 — `LawsHeldTicks`만 0).
  - M12의 `-Crossroads`를 `-SaveAt` 거절 목록에(입력의 꼴).
  - M13의 cards by 600(판 전체 수다 — decisions by 600·told by 600).
  - M14의 리뷰 번호를 헤더 글에.
  - build-1의 '인계가 WithoutB7xData를 나중 측정에만'(promptforother 4d는 데이터 전에 넣는다 — 결함은 시험 보드).
  - build-2의 `ThroneLockTicks`(기생은 주장을 뺀다).
  - build-4의 죽은 노드·성역 갈래(결과를 바꾸지 않아 시험할 수 없다).
  - I1의 'JSON이 조용히 뒤 키를 쓴다'(`parse_json`이 두 번 적힌 키로 멈춘다).
  - I9의 'B6b의 끈 짝 약속이 깨진다'(조건부 약속).
  - I12의 '절멸이 지워진 백성을 흔하게 한다'(결함은 빈도가 아니라 적히지 않은 규칙).
  - I5의 '새 지렛대'(B6b 은하 법칙 ①이 이미 Draw 몫을 줄인다 — 측정 항목일 뿐).
- **B6b와 뜻이 닿는 곳**(리베이스 뒤 다시 볼 열넷): ① 남긴 법 대 AI 법칙 걸음(시4 (b), `-Crossroads` 칸의 `LawsHeldTicks`); ② 남긴 법 대 주장 고리(시4 (a), W 1.2445(풀이는 헤더 §2.4 `CrisisParasite` 행) 그대로인지); ③ 미시 곱 대 `[B-6b6]`의 절멸·초신성 전조(시5, 다스리는 백성의 절멸 수); ④ 기생 덤 대 `[B-6b8b]`의 지킴 몫 0.5와 신정론 불변식(시3, B6b 노트 §2 ⓔ의 셈 — 역병 350 + 덤 30 → 0.98^38 ≈ 0.46, 판단 ⑦); ⑤ 씨앗 블록 대 B6b 경주(판단 ⑥, 첫 AI 셋째·넷째 칸 틱, Society 연구가 AI 법칙 걸음을 앞당기는지, 관계 항이 AI 조약 걸음과 `feat_passage_by_ark`에 닿는지); ⑥ `enc_cross_drift_fleet` 대 `[B-6b8]`의 `enc_witness`·도착 카드(③′, 시11); ⑦ 게임 데이터를 쓰는 테스트와 `WithoutB6bData`와 쌓는 차례(헤더 §6); ⑧ 변이 패치 일곱의 멈춤(시12)과 `[B-6b6]`~`[B-6b8]`의 새 패치; ⑨ 방랑자 정책 — 건너뜀은 `[B-6b8]`의 `Pick` 위에, `NoCheckByRow`는 실제로 답한 선택으로; ⑩ 낱말 목록(`[B-6b5]`의 rung_grasp_words·title_words·binding_words, `[B-6b6]`의 ark_words) — 리뷰 스캔에서 '견딘'(desolation_words)과 '왕좌'·'다스리'(dawn_words)가 걸렸다; ⑪ 시계 C(`[B-6b10]`) — 6,000을 쓰는 모든 문장; ⑫ 정착 사유의 열거자 수(static_assert 셋); ⑬ 열린 물음 번호와 테스트 수(`[B-6b12]`); ⑭ 절멸과 탄생(결정 16 (3a)/(3b)).
