# B7x 헤더 — 조합형 존재와 시간이 남긴 꼴 (블록 태그·백성의 시간 셈 여섯·규칙 모듈 `CosmicBricks`·조건 셋·겹침 규칙 열다섯·갈림 카드 열두 장·끝의 꼴 표지·레인의 단일 창구·품음과 판 도중의 탄생)

상태: **초안 (2026-11-09) — 승인 전.** 만든 길: 조사 워크플로(문서 리더 스물하나·코드 리더 일곱) → 구상 패널(제안 여섯·비평 셋) → 초안 → 적대 리뷰와 고침 → ultracode 적대 리뷰(2026-11-09, 렌즈 여섯 — 발견 63, 남긴 것 62)와 그 가운데 개발자의 답을 기다리지 않는 글의 고침(결과는 §11; 답을 기다리는 것은 §10에 '(리뷰 반영 — 개발자 답 대기)'로, 리베이스 때 고칠 코드·데이터·테스트는 노트 §5.4). 개발자의 말 둘(2026-11-09)을 따랐다 — "불변 규칙과 부딪히는 자리는 좀 유연하게 변경시켜가면서 진행", "실질적인 빌드와 자동화 테스트는 다른 PC에서 — 할 수 있는 수준까지만". 그래서 이 브랜치(`design/b7x-composable-existence`)에는 헤더와 함께 **빌드하지 않은 코드**(슬라이스 가의 `[B-7x1]`·`[B-7x2]`)가 쌓여 있다 — 컴파일·자동화 테스트·기준값은 다른 PC의 몫이다(§8·§11). 승인은 §10의 **[개발자]** 결정 열넷(1·2·3·4·6·8·11·13·14·15·16·18·20·22)의 답이다("권장대로"면 그대로 간다) — 그리고 결정 9·23을 [개발자]로 올릴지와 결정 21의 두 글(설계서 원안·`docs/OVERVIEW_as_of_B6a.html`)을 어떻게 둘지. 짝 노트 `B7_composable_existence_and_evolution_notes.md` — §1 예순 블록의 처분표, §2 부딪는 곳과 처리, §3 새 열린 물음의 초안, §4 다른 PC의 문구(브랜치에서만 — `[B-7x0]`이 지운다), §5 빌드 전 코드의 확인 목록과 리뷰가 넘긴 고침(§5.4)·리뷰의 기록(§5.5).

근거: 설계서 `docs/10_DESIGN/EXISTENCE_EVOLUTION_AND_ECOLOGY.md`(브랜치 `design/existence-evolution`의 `9be5b3e` — 이 헤더가 §3.3·§4.3·§4.4와 §5를 더하고 고쳤다), `docs/50_ROADMAP.md:12`(B7·B8의 칸)·`:314-318`(범위 추가 절차), `docs/10_DESIGN/systems.md:381`(섭식은 장부의 칸일 뿐이라는 판정), `docs/20_ARCHITECTURE.md`의 불변 규칙 전문, `CLAUDE.md` 불변 규칙 2·3·4·6·7·8·9·11·12·13. 코드의 사실은 모두 `main`의 `cef8cc7`(`[B-6b3b]`)에서 읽었다 — ultracode 리뷰(2026-11-09)가 `10205b6`(`[B-6b5]`)에 견주어 다시 읽었다: 그 사이 `Source`의 `Sim`·`Core`·`Events`·`Log`·`Data`와 `Config`는 바뀌지 않아 그 줄 번호는 그대로이고, 바뀐 것(스키마의 줄·`OPEN_QUESTIONS.md`의 끝)은 고쳐 적었다. B6b가 끝난 `main`의 수와 번호는 리베이스 때 다시 적는다(§8). 코드는 `Source/Cosmic_Ascension/` 아래를 줄여 `Sim/…`로, 문서는 `docs/20:197`처럼 적는다.

## 0. 범위와 전제

- **이 헤더의 낱말.** 설계서의 '존재'는 여기서 늘 **백성**(`FCivilizationState`)이다 — 개체·괴수·초월자가 아니다. **블록**은 백성의 `Traits`에 든 `Brick.<축>.<잎>` 태그, **시간 셈**은 백성이 살아낸 틱을 여섯 갈래로 센 것(설계서의 시간 벡터), **꼴**은 블록 몇 개가 모여 이룬 백성의 사는 모양, **겹침 규칙**은 블록 둘셋이 함께 있을 때 서는 규칙(설계서 §3.3), **갈림 카드**는 시간 셈이 문턱을 넘을 때 오는 청원·조우 카드(설계서 §4.3), **끝의 꼴 표지**는 판의 끝에 연대기에 붙는 저장하지 않는 낱말이다. 설계서의 '유목신'·'5칸급'·영어 별칭은 설계 문서의 이름일 뿐 화면에 내지 않는다.
- **받는 꼴 한 문장.** 블록은 백성의 특성 태그로, 살아낸 시간은 백성의 틱 셈 여섯으로, 현상은 있는 테이블의 데이터 행과 규칙 모듈 하나로, 갈림은 지어 둔 카드로, 끝의 꼴은 저장하지 않는 표시 한 낱말로 받는다.
- **개발자의 말을 읽은 방식.** 부딪는 곳마다 셋 가운데 하나를 골랐다 — ㉠ 규칙을 좁게 고쳐 받는다(고칠 문장을 적는다 — 아래 '고쳐 받는 규칙'), ㉡ 기능을 같은 뜻의 다른 꼴로 옮긴다(규칙은 그대로), ㉢ 받지 않는다(왜인지 적는다). ㉢은 아래 '지키는 것' 열 가지에 닿는 곳뿐이다 — 굽히면 다른 PC의 해시·테스트가 통째로 깨지거나 게임의 정체가 바뀌는 자리다.
- **B7x에 드는 것**(슬라이스 셋):
  1. **슬라이스 가 — 코어**: 블록 태그 열일곱과 스키마의 뿌리(결정 2·3), 백성의 시간 셈 여섯(결정 4·5), 품는 백성의 칸 예약(결정 6), 규칙 모듈 `CosmicBricks`(결정 7·8·12), 조건 셋(결정 9), AI 백성의 씨앗 블록(결정 23), 갈림 카드 열두 장과 겹침 규칙의 행(결정 10·11), 꼴의 낱말(결정 13). 커밋 `[B-7x1]`~`[B-7x5c]`(차례는 §8).
  2. **슬라이스 나 — 지도를 고치는 창구**: 레인을 잇고 끊는 단일 창구 `CosmicLanes`와 그 위의 견인·접힘·초연결·궤도 제동(결정 14·15·17). 커밋 `[B-7x6]`.
  3. **슬라이스 다 — 품음과 탄생**: `HostCivID`를 읽는 규칙, 건넘 단계에서 받는 백성의 탄생(결정 6·16·22). 커밋 `[B-7x7]`·`[B-7x8]`.
- **B7x 밖.** AI 백성 다섯 행·`AIProfile`의 행동·교단·안식과 낭독 칙령은 로드맵의 B7 그대로다(본능 축 여덟 가운데 여섯은 그 블록의 낱말이 된다 — 노트 §1). 서막과 실제 하늘은 B8 그대로다. 타락 초월자와 교리 넷, 다섯 칸, 엔딩 다섯 갈래는 손대지 않는다.
- **고쳐 받는 규칙**(㉠ — 저마다 결정 하나; 리뷰 F5·F4·DET-1이 표로 바꾸고 R11~R14를 더했다). '지금 문장'은 `main` `10205b6`의 줄이다. 굽는 문장이 없고 코드의 사실만 굽는 것은 그렇게 적었다(R6의 꼴). 굽는 문장도 코드의 사실도 없는 것은 R에서 빼서 ㉡로 옮겼다(R2). 번호는 다른 곳이 부르므로 그대로 둔다.

| R | 지금 문장 | 새 문장 | 넓이(무엇에만) | 지키는 시험 | 결정 |
|---|---|---|---|---|---|
| R1 | `docs/10_DESIGN/systems.md:381` '새 태그·열·축을 두지 않는다(`Trait.Diet.*` 없음 — 읽는 곳이 없는 태그는 ini에 넣지 않는다)'; 스키마의 뿌리 `"Trait."`(`Tools/data/schema.json:291`·`:1245`·`:1286-1287`) | '읽는 곳이 있는 블록은 `Brick.` 뿌리 아래 선언한다 — 읽는 곳이 없는 태그는 ini에 넣지 않는다(그대로)'; 뿌리는 `["Trait.", "Brick."]` | 블록 태그 열일곱, 저마다 읽는 자리가 있다 | `Data.Integrity`(뿌리 목록의 C++ 거울), `test_data_schema.py`(깨짐 `Trick.Form.X`), §3.3 규칙 7(K4) | 2 |
| R2 | (R에서 뺐다 — ㉡) 누적 셈을 금하는 문장이 없다. 연속 셈 넷(`Core/GalaxyState.h:315-339`)은 선례일 뿐이다 | — | — | `Sim.TimeVectorsCount`·`Sim.HashCoversTimeVectors` | 5 |
| R3 | 적힌 금지는 없다 — 성계의 주인은 `OwnerCivID` 하나다(그대로) | `docs/21` §7.2: '`HostCivID` — 품는 백성의 이름, 예약(슬라이스 다가 읽는다); 품는 백성이 건넘 단계에서 지워지면 비운다' | 성계마다 이름 하나, 초기값 `NAME_None`, 읽는 규칙은 슬라이스 다 | `Sim.SaveTimeVectors`(옛 세이브 — 지운 수 = 성계 수)·`Sim.HashCoversTimeVectors`; 비움은 슬라이스 다의 테스트(노트 §5.4) | 6 |
| R4 | `CLAUDE.md` 규칙 13의 창구 목록(레인의 창구가 없다); 생성 뒤 레인을 쓰는 코드는 파괴 하나(`Sim/CosmicHazards.cpp:442-450`) — 적힌 금지는 없다 | 규칙 13의 목록에 `CosmicLanes::Link`·`Unlink`·`CanLinkOf`(슬라이스 나 — 다른 줄을 줄이며 넣는다, §7); 파괴의 끊음도 그 창구로 | 같은 `GalaxyRow`, 성역은 첫 거절, 대칭, 레인 상한, 살아 있는 성계의 고립 금지(파괴의 끊음은 예외 — §2.8), 한 틱 한 걸음 | 슬라이스 나 헤더(레인 걸음 뒤 주장이 풀리거나 늦춰지지 않는다 — K9) | 14 |
| R5 | 적힌 문장이 없다 — 코드의 사실: 좌표를 읽는 곳은 표현뿐(`World/CosmicGalaxyMapActor.cpp:355`·`:822-823`), 지도는 `OnGalaxyGenerated`에서만 짓는다 | 슬라이스 나 헤더가 적는다(좌표는 레인 다음의 둘째 걸음) | 창구가 받은 뒤 | 슬라이스 나 헤더 | 15 |
| R6 | 코드의 사실 `Sim/GalaxySimulationSubsystem.cpp:873-874`(생성이 백성을 만드는 하나뿐인 자리). `docs/20_ARCHITECTURE.md:289` '`Civs`의 원소 수는 건넘 단계에서만 바뀌고'는 그대로 선다 | '건넘 단계가 탄생의 요청을 받는 곳 하나 — 새 `CivID`는 `NextCivSerial`에서' | 슬라이스 다, 결정 16의 받는 조건 | 슬라이스 다 헤더 | 16 |
| R7 | 코드의 사실 — 레인 시간은 규칙 행 하나만 읽는다(`Sim/CosmicWanderer.cpp:35-38`; `docs/21_DATA_MODEL_REFERENCE.md:1160`) | '`LaneTicksOf`가 맥락(지나는 성계의 주인)을 받는다 — 최소 1틱' | B8 헤더의 '거리에 따른 레인 시간'과 한 번에 | B8·슬라이스 나 헤더 | 17 |
| R8 | `docs/50_ROADMAP.md:12` '옛 §12.1 수량(… 조우 25+청원 13 …)' — `[B-6b12]`가 고친다 | 'B6b 끝의 수 + 청원 11·조우 1'(갈림 열둘) | 수직 슬라이스의 줄 하나 | 수를 세는 테스트는 없다 — `[B-7x5c]`의 스위치 대조 줄(카드 열둘)과 `Data.Integrity`의 도우미 핀(§5) | 11 |
| R9 | `docs/20:210`의 추첨 지점 목록; `docs/20:218` '시뮬 추첨 지점은 테스터와 시드를 나누기 전에만 더한다'(§11 — 지킨다) | 목록에 포자의 고정 수 하나(슬라이스 다) | Phase 5 전, 고정 수 하나 | 슬라이스 다 헤더(추첨 장부) | 22 |
| R10 | 적힌 문장이 없다 — 코드의 사실: 생성 뒤 `StarClass`를 쓰는 곳은 파괴 하나(빈 태그 = 죽은 노드 — `docs/21:561`, `Sim/CosmicHazards.cpp:451`) | '수도의 별을 일곱 등급 안에서 주기마다 한 단 낮춘다(`DimCapitalStar` — 주기 0이면 꺼짐)' | 남의 수도·정착 중·인덱스 없음은 거절, 죽은 노드를 만들지 않는다 | `Sim.BrickShares`의 (5)와 거절 셋(노트 §5.4 시6) | 8 |
| R11 | `docs/60_DECISIONS/OPEN_QUESTIONS.md:63` #88 '국경 배율 — VS에 두지 않는다 … EA 후보: 아키타입 행의 새 열(이웃에게 거는 배율)' | #88 끝에 'B7x가 VS에 연 것: 조건 `BorderingCivHasTrait`, 기생 블록의 두 국경 항(이웃에 새로 서는 위기의 덤·이웃 위기의 통찰), 표류 목표의 블록 항 — 다른 국경 배율은 여전히 EA' | 그 넷만 | `Events.BrickConditions`, `Sim.BrickShares`(기생·눈의 항), §3.3 규칙 3 | 9 |
| R12 | B2 헤더 §2.6 `docs/60_DECISIONS/headers/B2_edicts_and_research.md:125` 'AI는 칙령을 내리지 않고 지니지 않는다'·`:129` '섭정 아래의 옛 백성도 AI다 — 통치자가 내린 칙령을 첫 AI 단계에서 철회한다'; 코드 주석 `Sim/GalaxySimulationSubsystem.cpp:2931` | '— 다만 `Brick.Form.UnseenLaw`를 지닌 백성은 지닌 백성 범위 칙령을 남긴다(B7x)' — 그 좁힘의 문장은 결정 18의 답(§10) | `UnseenLaw` 백성만 | `Sim.KeptLawEdicts`(또는 넓힌 `Sim.MutationStep` — 노트 §5.4 시4)와 변이 둘 | 18 |
| R13 | `OPEN_QUESTIONS.md:33` #36 ② '절차 특성(…)의 알고리즘 — `DT_Traits`와 함께'; 규칙 3 — `docs/20:204` ① '시뮬 스트림: 저장되는 세계 상태를 바꾸는 모든 추첨 — 생성, …'·`:208` ③ 목록·`:220` '고향 은하는 시뮬 스트림 `Initialize(Seed)`에서 생성된다' | #36 ② 끝에 'B7x가 가장 작은 꼴을 앞당겼다 — AI 백성마다 씨앗 블록 하나(파생 정수)'; `docs/20`의 세 문장은 §7(`[B-7x1]`) | AI 백성마다 하나, 파생 정수 용도 13, `BrickSeedTags`가 비면 없음, 첫 생애의 백성은 받지 않는다 | `Sim.SeedBricks`(같은 시드 같은 블록·스트림의 자리 같음·노브가 비면 은하가 그대로), `Sim.SeedDerive`(용도 13), 변이 '씨앗이 시뮬 스트림에서 뽑음' | 23 |
| R14 | `CLAUDE.md` 규칙 13 '정착된 성계 = 소유 ∧ `ActiveMegaproject != project_settle`(수입·…이 이 술어를 쓴다)' | 품긴 성계 수입의 몫이 품는 쪽에 간다 — 술어는 그대로 주인만 읽는다 | 슬라이스 다 | 슬라이스 다 헤더 | 6·16 |
- **지키는 것**(㉢의 까닭 — 굽히지 않는다): K1 세계는 앎·로그·연대기·기다리는 카드를 읽지 않는다(`Sim.KnowingInert`와 세계 해시가 재는 것). K2 틱 열 단계와 시계 하나. K3 추첨 수는 테이블과 블록을 보기 전에 정해진다. K4 칸은 업적 행으로만 오른다 — 블록·변이·카드는 칸을 올리지도 내리지도 않는다. K5 타락 초월자와 교리는 그대로다(블록은 백성만 말한다). K6 거룩한 질서는 엔티티·블록·엔딩이 아니고, 누구도 법·언약·울타리·신이 '되지' 않는다; 금지어와 사람의 수. K7 저장 규칙(더하기만, 얼린 기본값, `SaveVersion` 1, 열거자는 끝에). K8 절대 면역은 없다(문의 사유와 유한한 감쇠는 된다). K9 `Hazard.Claim`은 새로 풀리지도 늦춰지지도 않는다(신정론 불변식 — 뜻과 범위는 바로 아래 항). K10 정체는 몸을 건너지 않는다 — 시간 셈은 백성의 것이다.
  - **K9의 범위**: 블록 항이 주장의 카운트다운과 주장 규칙의 문에 직접 닿지 않고, 만료를 해결로 바꾸거나 판 끝까지 늦추는 길(영원한 늦춤)이 없다는 뜻이다. 다만 `Brick.Form.UnseenLaw`가 `edict_vigil`(`Hazard.Claim` +100)·`edict_machine_rights`를 판 끝까지 지니는 길은 지금 브랜치에서 이 줄을 깬다 — 좁힘은 §10 결정 18(리뷰 반영 — 개발자 답 대기). 다른 위기의 만료가 늦거나 그 위기가 덜 서서 그 뒤의 주장 규칙이 늦게 서거나 덜 서는 간접 길은 main에도 있다(철야의 `Hazard.Rebellion` 덤 +200 — main `Tools/data/DT_Edicts.json:96`, 격리가 역병이 설 몫에 곱하는 0.1 — `:113`). 그래서 기생 덤(§2.4 `CrisisParasite` 행)은 이 줄을 굽히지 않고, 미시 곱이 각성을 줄여 그 뒤의 주장이 주는 길(§2.4 안으로 접힌 행)도 마찬가지다. 기생 덤은 신정론 불변식의 가장 긴 머묾에 위기마다 한 번 든다(§3.3 규칙 6 — 리베이스 때 노트 §5.4 시3; 리뷰 F2·DET-4·M10).
- **바뀌는 것.** `[B-7x1]`은 두 해시를 모든 기준값 줄에서 한 번 옮긴다(해시는 리플렉션으로 모든 `UPROPERTY`를 걷는다 — `Sim/GalaxySimulationSubsystem.cpp:521-530`; `[B-6b1]`의 꼴 — main B6b 헤더 `:44`; 그 기준값의 `note`(`Tools/baselines/seed42.json:13`)는 `[B-6b6]`이 고쳐 쓴다). 세계는 그대로다 — 증명은 커맨드렛 CSV의 결정적 칸이 같다는 것이다. 설계서 §5의 '기준값이 흔들리지 않게'는 이룰 수 없는 조건이라 그렇게 고쳐 읽는다. `[B-7x2]`는 누적 셈을 켜므로 두 해시를 한 번 더 옮긴다 — 셈은 블록 없이도 모든 백성에서 첫 틱부터 쌓이고(첫 한 성계·물려받은 전시 동원) 해시가 그 값을 걷는다(`Sim.HashCoversTimeVectors`). 세계는 그대로다 — 셈을 읽는 행과 걸음이 아직 없어 CSV의 결정적 칸이 같다(`[B-6b1]`의 꼴; 두 커밋을 하나로 합쳐 기준값을 한 번만 다시 적어도 된다). `[B-7x5s]`(노브와 씨앗 — AI만)와 `[B-7x5c]`(카드·사건·`Society`)가 데이터를 켜며 세계를 옮긴다(§5·§8).

## 1. 기대는 사실 (2026-11-09, HEAD `cef8cc7` — 리뷰가 `10205b6`에 견주었다)

### 1.1 지금 코드

- **특성은 태그일 뿐이다.** `FCivilizationState::Traits`는 `FGameplayTagContainer`이고(`Core/GalaxyState.h:280-282`) 생성 때 아키타입에서 베껴진다(`Sim/GalaxySimulationSubsystem.cpp:938`). 읽는 곳은 조건 `CivHasTrait`(`Events/CosmicEventSubsystem.cpp:431-432`)와 인물 이름의 거름(`:285`) 둘, 쓰는 곳은 효과 `AddTrait`·`RemoveTrait`(`:751-765`)뿐이다 — **틱의 어느 단계도 특성을 읽지 않는다.** `DT_Traits`는 표도 행 구조체도 없다(`Data/CosmicDefinitionRows.h:4`, `Data/CosmicDataTables.cpp:36-46`).
- **뿌리는 스키마에만 있다.** 코드의 평가기는 태그의 뿌리를 묻지 않고, `Tools/data/schema.json:291`·`:1245`·`:1286-1287`(`10205b6`; `cef8cc7`에서는 `:272`·`:1196`·`:1237-1238`)이 `Trait.`으로 묶는다(C++ 거울 `Tests/CosmicDataTests.cpp:1790`). 뿌리는 목록일 수 있다(`:897`의 꼴 — `10205b6`). ini의 태그는 150개, `Trait.*` 36개, `Brick.*` 없음(`Config/DefaultGameplayTags.ini:126-165`).
- **있는 셈 넷은 모두 연속 셈이다.** `LowStabilityTicks`·`FallenSphereTicks`·`LawsHeldTicks`·`PressureEnduredTicks`는 경제 단계의 둘째 고리에서 '서면 +1, 아니면 0'으로 센다(`Core/GalaxyState.h:315-339`, `Sim/GalaxySimulationSubsystem.cpp:1928-1935` — 추첨도 앎 읽기도 없다). 끊기지 않는 누적 셈의 선례는 `FLifeRecord::ReignTicks`뿐이다.
- **해시와 저장.** 두 해시는 한 리플렉션 걸음을 나눠 쓴다. 세계 해시가 빼는 것은 최상위의 다섯 이름뿐이라(`Sim/GalaxySimulationSubsystem.cpp:1537-1573`) 백성과 성계 안의 것은 모두 세계다. 태그 컨테이너는 정렬된 꼴로 해시된다. 배열 원소의 구조체는 통째로 적히고, 이름표가 붙어 있어 새 멤버가 없는 옛 세이브는 초기값으로 읽힌다(`Tests/CosmicUpperRungTests.cpp:147-158`).
- **지도.** 생성 뒤 레인을 쓰는 곳은 파괴 하나(`Sim/CosmicHazards.cpp:442-450`), 좌표를 읽는 곳은 표현 계층뿐(`World/CosmicGalaxyMapActor.cpp:355`·`:822-823`), 레인 시간은 규칙 행만 읽는다(`Sim/CosmicWanderer.cpp:35-38`). 성계 없는 백성은 건넘 단계에서 지워지고(`Sim/GalaxySimulationSubsystem.cpp:3073-3084`) 생성 뒤 백성을 만드는 코드는 없다(`:873-874`가 하나뿐인 자리).
- **이미 있는 보호.** 한 성계만 정착한 백성은 주장을 받지 않고(`Law.Remnant` — `Sim/CosmicHazards.cpp:549-587`) 분쟁은 수도와 마지막 정착된 성계를 앗지 않는다(`Sim/CosmicConflict.cpp:229-236`). '한 별에 닫아 건 백성'은 새 면역 없이 이미 그렇다.
- **카드.** 대기 칸은 하나, 상시 카드는 칸이 차 있으면 사라지고 즉위 때마다 다시 온다(`Events/CosmicPetitions.cpp:230-233`·`:277-280`; `Sim/GalaxySimulationSubsystem.cpp:1596-1600`). 카드의 조건은 `SinceTick` 0으로 읽힌다(`Events/CosmicPetitions.cpp:217-221`). 답은 행의 이름만 남긴다 — 고른 갈래는 효과가 써야 남는다. 측정 정책은 모든 카드에 차례로 답하고 통치자 정책이 빼는 것은 `DeposeRuler`를 지닌 선택뿐이다(`World/CosmicRulerPolicy.cpp:272-280`).
- **끝의 쪽.** 연대기 쪽은 고정 줄 셋으로 끝나고 발치의 까닭 줄은 표시할 때 셈한다(`UI/CosmicChronicleWidget.cpp:115-160`·`:340-343`). 두 책은 `Log.Record`와 `Log.Claim`만 받는다(`Log/CosmicChronicle.cpp:416-429`).
- **검사.** 에디터 없이 도는 파이썬 검사 다섯은 이 HEAD에서 모두 통과한다. 설계서가 이름한 `Tools/audit_content.py`와 `FPlayerCalling`은 없다(글의 검사는 스키마와 `check_data.py`·`test_data_schema.py`, 부름의 구조체는 `FCallingState`). 자동화 테스트는 223종이다(`STATUS.md:26` — `10205b6`에서도 같다; `[B-6b8]`이 `Sim.EraEndured`·`Sim.WitnessCard`를 더하므로 B7x의 수는 `[B-6b12]` 끝의 수에서 센다 — §6).

### 1.2 잰 값 (앞 블록들의 기록에서)

시계 6,000(마지막 값은 `[B-6b10]`), 둘째 칸 중앙값 약 1,170·셋째 칸 약 2,370, 통치자의 주기 카드 칸 300틱, 판의 끝까지 결정 28~60. 빈 자리는 틱 30~150·셋째 칸 뒤의 계절·방랑자의 삶이고, 붐비는 자리는 둘째·셋째 칸의 틱과 틱 650~2,000, 역할이 바뀐 첫 틱이다. 넷째 칸 위의 잰 값은 아직 없다(B6b의 묶음 A·B·C 전) — 그래서 이 헤더의 모든 문턱은 **시작값**이다. 이 절의 수는 B6a 끝의 것이다 — B6b의 묶음 B·C가 수를 내면 그 수에서 시작하고, 시계는 `[B-6b10]`이 정한 C다(리뷰 I4·I10).

### 1.3 설계서와 부딪는 곳 (표는 노트 §1·§2)

예순 블록 가운데 열일곱을 슬라이스 가에서 읽고, 본능 여섯은 B7의 낱말로, 열넷은 슬라이스 나·다로 넘기며, 나머지는 글과 얼굴로만 쓰거나 다른 꼴로 옮겼다. 적힌 그대로는 받지 않는 것은 다섯 가지다 — 앎을 읽는 붕괴(K1), 거꾸로 흐르는 시간과 미리 적용되는 결과(K2), 칸을 주는 변이(K4), 절대 면역(K8), 법·언약·울타리가 '되는' 끝(K6). 저마다 옮긴 꼴이 노트 §2에 있다.

### 1.4 열린 물음 가운데 닿는 것

#36 ②(절차 특성의 알고리즘 — 블록이 그 답의 절반이다)·#88(국경 배율)·#89·#91(키메라와 성식종은 위기의 얼굴)·#90(인과율 수입 — 블록은 인과율을 내지 않는다)·#126·#128(B7의 물음 — 슬라이스 다의 품음은 #128의 뒤)·#144(값없는 카드)·#158(대기 칸). 새 물음은 `OPEN_QUESTIONS.md`의 마지막 번호 다음부터다(`10205b6`의 끝은 #163이고 B6b 헤더가 #164를 잡았다 — 번호는 `[B-6b12]` 뒤의 승인 커밋이 매긴다; 리뷰 I10) — 초안은 노트 §3.

## 2. 장치

### 2.1 저장 필드 — 시간 셈 여섯과 품는 백성의 칸 (`[B-7x1]` — 필드가 먼저인 커밋; 결정 4·5·6)

```cpp
// Core/GalaxyState.h — a new USTRUCT before FCivilizationState, and one member next to the four streak counters.
// Cumulative ticks, never reset: unlike LowStabilityTicks and its three siblings these do not start over.
// Counted at the end of the economy stage (B7x header section 2.2). No draw, no knowing read.
USTRUCT(BlueprintType)
struct FCosmicTimeVectors
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly) int32 Built = 0;     // a megaproject other than the settlement was in progress
	UPROPERTY(BlueprintReadOnly) int32 Consumed = 0;  // a siege was under way, or the inherited extraction edict was held
	UPROPERTY(BlueprintReadOnly) int32 Drifted = 0;   // a settlement was in progress
	UPROPERTY(BlueprintReadOnly) int32 Linked = 0;    // a treaty or a friendly stance stood
	UPROPERTY(BlueprintReadOnly) int32 Isolated = 0;  // one settled system, no treaty, no stance
	UPROPERTY(BlueprintReadOnly) int32 Endured = 0;   // pressure or a law was borne, or a hazard stood on its ground
};
// FCivilizationState:  FCosmicTimeVectors TimeVectors;
// FStarSystemState:    FName HostCivID;   // reserved: the people that hosts this system's owner (slice "da")
```

- **셈인 까닭.** 갈림 카드의 문턱과 끝의 꼴이 '살아낸 시간'을 물어서다. 있는 기록(완성 기록·조약·연속 셈)으로는 지나간 시간의 양을 되살릴 수 없다. 여섯을 한 구조체에 묶어 한 번에 더한다.
- **얼린 기본값 0·`NAME_None`**(규칙 7): 둘 다 배열 원소의 멤버라 새 세이브에는 통째로 적히고 옛 세이브는 초기값으로 읽힌다. `SaveVersion` 1 그대로, 이주 없음.
- **두 해시에 든다.** 백성과 성계 안의 것은 모두 세계의 사실이다. 그래서 `[B-7x1]`이 기준값 다섯 줄과 해시 판 둘을 한 번 옮긴다 — 새 필드를 한 커밋에 모으는 까닭이다(`HostCivID`를 지금 예약하는 까닭도 같다 — 결정 6).
- **주인은 백성이다**(결정 4, K10). 그릇과 부름에는 두지 않는다. 방랑자의 '떠돈 시간'이 필요한 카드는 있는 칸에서 셈한다(산 틱 − `ReignTicks`).
- **테스트의 덫.** 옛 세이브를 흉내 내는 도우미 `Forget`은 날바이트 검색이다 — `Endured`는 `PressureEnduredTicks`의 부분 문자열이라 안쪽 이름을 지우면 다른 셈이 깨진다. 바깥 이름 `TimeVectors`를 지운다.

### 2.2 셈의 술어 (`[B-7x2]` — 경제 단계 끝의 새 걸음 ★; 결정 5)

있는 셈의 고리(`Sim/GalaxySimulationSubsystem.cpp:1928-1935`) 바로 뒤에서 `CosmicBricks::CountTimeVectors`가 백성마다 제 `OwnedSystems`를 한 번 걷는다. 틱마다 셈마다 많아야 +1, 되돌림 없음, 추첨 없음, 앎 읽기 없음. 한 틱이 여러 셈에 들 수 있다.

| 셈 | 그 틱에 서는 세계의 사실 (시작값 — 결정 5) |
|---|---|
| `Built` 지은 시간 | 제 성계 어디선가 정착이 아닌 거대 프로젝트가 진행 중이다(건설 칸이든 대응 칸이든) |
| `Consumed` 삼킨 시간 | 함대가 다른 백성의 성계를 겨누고 있다(공성 중), 또는 물려받은 칙령(`DT_Rules.StartingEdict` — 전시 동원)을 쥐고 있다 |
| `Drifted` 떠돈 시간 | 제 성계 어디선가 정착(`project_settle`)이 진행 중이다 |
| `Linked` 이은 시간 | 조약이 하나라도 있거나 우호의 태세가 서 있다 |
| `Isolated` 홀로 선 시간 | 정착된 성계가 꼭 하나이고 조약도 중립 아닌 태세도 없다 |
| `Endured` 견딘 시간 | 그 틱에 `PressureEnduredTicks`나 `LawsHeldTicks`가 섰거나, 제 정착된 성계에 위기가 서 있다 |

### 2.3 조건 셋 (`[B-7x2]` — `EConditionType`의 끝, `CivPressureEnduredTicksReached` 뒤; 결정 9)

- `CivTimeVectorAtLeast` — `Key` = 셈의 낱말(`Built`·`Consumed`·`Drifted`·`Linked`·`Isolated`·`Endured`), `Value` = 틱. 맥락 백성의 셈이 그 이상이면 참. 창이 없는 '이상' 꼴이다 — 누적 셈은 멈출 수 있어 한 슬롯짜리 '닿음' 창을 쓰면 줄곧 발화한다(`CivPressureEnduredTicksReached`의 창은 연속 셈을 전제한다).
- `CivLacksTrait` — `Tag`. 맥락 백성이 그 태그를 지니지 않으면 참(맥락 백성이 없거나 태그가 비면 거짓 — `CivHasTrait`와 같은 결). 카드와 행이 한 번만 서게 하는 문이다(그 선택이 그 태그를 준다). 어휘의 첫 부정이다(`FlagNotSet`을 빼면).
- `BorderingCivHasTrait` — `Tag`. 맥락 백성과 국경이 닿은 백성 가운데 하나가 그 태그를 지니면 참. `CosmicDiplomacy::BordersOf`를 한 번 걷는다 — 국경은 캐시가 없어서(`Sim/CosmicDiplomacy.cpp:70-98`) 스키마가 위기의 `SpawnConditions`와 정착된 성계 주제의 행에는 막는다.
- 새 효과는 없다. 블록을 주고 걷는 것은 있는 `AddTrait`·`RemoveTrait`다(카드에서 합법 — 청원에서는 다스리는 백성, 방랑자 카드에서는 그 성계의 주인에게 닿는다).

### 2.4 블록의 몫과 노브 (`[B-7x2]` — 규칙 모듈 `Sim/CosmicExistenceBricks.h/.cpp`, 이름공간 `CosmicBricks`; 결정 7·8)

특성 행(`DT_Traits`)은 아직 없다 — 그 표는 로드맵 B7의 몫이다. 슬라이스 가는 **노브로** 읽는다: `FCosmicRulesDef`의 끝에 더한 열일곱 노브의 C++ 값은 무동작(곱 1·더함 0·주기 0·빈 묶음)이라 장치 커밋에서 블록의 몫은 어느 백성도 바꾸지 않고(두 해시를 옮기는 것은 누적 셈뿐이다 — §0; 안으로 접힌 둘의 정착 거절과 남긴 법은 노브 없이 태그로 일하지만, 장치 커밋에는 그 태그를 지닌 백성이 없어 세계가 그대로다 — 리뷰 DET-8), `[B-7x5s]`가 JSON의 값을 켠다. 읽는 자리는 블록마다 하나, `HasTagExact`로 가려 태그 없는 백성은 옛 길을 글자 그대로 간다.

| 블록 | 읽는 규칙 (노브) | 받은 꼴의 까닭 |
|---|---|---|
| `Brick.Diet.DarkMatter` | 별 등급·풍부도에 매이지 않는 에너지(`BrickDarkMatterEnergyPerTick`) | 원안의 '정착 없이 여과'는 땅 없는 백성이라 못 받는다 — 닻 성계 하나를 쥔 채 별에 매이지 않는 수입으로 |
| `Brick.Diet.StarEater` | 에너지 수입 곱(`BrickStarEaterEnergyMultiplier`), 다른 백성이 그를 보는 눈의 표류 목표에 더하는 항(`BrickStarEaterRelationTarget` — 값은 음수), 수도 별의 등급을 주기마다 한 단 낮춤(`BrickStarEaterDimPeriodTicks` — 0이면 꺼짐; 결정 8) | 여덟째 등급과 파괴는 없다 — 일곱 등급 안에서 가장 어두운 데서 멈춘다. 눈은 틱마다 남의 상태를 쓰지 않고 외교 단계의 목표에 항 하나로 든다 |
| `Brick.Diet.SilenceFeed` | 일 없는 틱의 통찰(`BrickQuietInsightPerTick`) — 제 정착된 성계에 위기·사업·공성이 없는 틱 | '사건 없는 틱'을 세계의 술어로; 화면의 이름에 침묵·고요를 쓰지 않는다 |
| `Brick.Diet.DistanceTension` | 제 정착된 성계 사이의 가장 먼 레인 수 × `BrickSpanEnergyPerLane` | 좌표의 거리가 아니라 레인 수(`CosmicState::LaneDistancesFrom`) |
| `Brick.Diet.CrisisParasite` | 그 백성과 국경이 닿은 백성의 성계에 **새로 서는** 위기의 놓일 때 덤(`BrickParasiteCountdownBonusTicks`), 그 이웃들의 정착된 성계에 서 있는 위기(주장·절멸·전조 빼고) 하나마다 틱당 통찰(`BrickParasiteInsightPerHazard`) | 서 있는 카운트다운은 건드리지 않는다; 주장·절멸·초신성 전조는 뺀다(K9, 은하 법칙의 갈고리와 같은 제외). 이웃의 각성·역병·반란에 붙은 덤은 그 만료와 그 뒤의 주장 규칙을 늦춘다 — 철야의 `Hazard.Rebellion` +200과 같은 결이다; 주장의 카운트다운은 빼므로 W(B6b의 신정론 한계 — 주장의 가장 긴 머묾 550틱(카운트다운 300 + 지킴 상한 100 + `edict_vigil` 100 + 카드 50) = 55슬롯 동안 철야 추첨 행 `event_claim_eased`가 오지 않을 몫이 0.25 이상이게 하는 그 행 무게의 상한 1.2445; B6b 노트 §2 ⓔ — `10205b6` `:622`, B6b 헤더 §2.10 `:221`)는 그대로다; 신정론의 4분의 1(가장 긴 머묾)이 이 덤을 센다(`Data.Integrity` — §3.3 규칙 6) — 주장은 빠지므로 `ThroneLockTicks`의 한계는 그대로다(리뷰 F2·DET-4·M10) |
| `Brick.Space.Nomad` | 정착의 진척 곱(`BrickNomadSettleProgressMultiplier`) | 수도 없는 백성은 못 받는다 — 빨리 넓히는 백성으로 |
| `Brick.Space.HermitMonolith`·`Brick.Space.PlanckMicro` | 정착의 문이 새 정착을 거절한다(§2.5), 연구 곱(`BrickInwardResearchMultiplier`), 미시 쪽은 위기가 설 몫 곱(`BrickInwardHazardSpawnMultiplier` — 0은 안 된다; 주장·절멸·초신성 전조는 빼고 — 기생과 같은 제외, 리뷰 M9·I5) | 침공 무효·완전 면역은 없다(K8) — 한 성계 백성의 있는 보호로 족하다; 빠른 안쪽 시간은 곱으로(K2). 각성의 몫이 줄면 그 만료 뒤의 주장도 준다 — `[B-7x5s]`·`[B-7x5c]`에서 #136과 함께 읽는다; 한 성계 안쪽 백성과 그 뒤 AI 손에 든 백성은 다시 정착할 수 없어 `Law.Remnant`에 남는다(리뷰 F2) |
| `Brick.Instinct.SwarmTech` | 연구 곱(`BrickSwarmResearchMultiplier`) | — |
| `Brick.Instinct.HiveMerge` | 다른 백성이 그를 보는 눈의 표류 목표에 더하는 항(`BrickMergeRelationTarget` — 값은 양수) | 동화 교리의 위조와 같은 결이라 축하하지 않는다 — 고르는 선택은 그릇의 높아짐을 올린다(높아짐을 읽는 문은 아직 예정이라 지금은 적히기만 한다) |
| `Brick.Form.DysonBrain`·`Brick.Diet.StarEater` | 제 정착된 성계에 별의 위기(`Hazard.PreSupernova`)가 설 몫의 곱(`BrickStellarCrisisSpawnMultiplier` — 둘을 지녀도 한 번, 0은 안 된다) | 닫아 건 별과 삼키는 별은 제 별이 값을 잰다 — 설계서 §3.3의 S01·S06·S11이 함께 쓰는 상한 |
| `Brick.Form.UnseenLaw` | 다스리는 이가 없을 때(섭정·AI) 그 백성은 지닌 칙령을 하나도 거두지 않는다(`CosmicEdicts::AIRevokesOf`의 첫 물음) | 은하 범위의 법칙은 이미 누구에게나 그렇다(B6b 결정 11) — 이 블록은 그것을 백성 범위의 칙령까지 넓힌다: 왕좌가 빈 뒤에도 남는 법 — 안정도를 깎는 칙령(물려받은 전시 동원)도 남는다(설계서 §4.3 C10). **리뷰가 귀결 둘을 더 찾았다** — 주장 고리 칙령(`edict_vigil`·`edict_machine_rights`)을 판 끝까지 지녀 K9를 깨고, 칙령 자리를 막아 법칙 셋에 닿지 못한다(F1·I2·M11). 좁힘의 권장과 대안은 §10 결정 18(리뷰 반영 — 개발자 답 대기) |
| `Brick.Form.Society`·`LivingNebula`, `Brick.Mutation.*` 셋 | 조건이 읽고, `LivingNebula`는 끝의 꼴도, 변이 셋은 AI의 변이 걸음도(지닌 변이를 건너뛴다) 읽는다 — `Society`와 변이 셋은 끝의 꼴이 읽지 않는다(부모 블록의 꼴을 입는다) — 수를 바꾸는 것은 그 표지를 묻는 겹침 규칙의 행이다(설계서 §3.3) | 꼴의 표지. `StarShepherd`는 슬라이스 가에서 낱말만 연다(별을 끄는 일은 슬라이스 나) |

이 브랜치의 코드에서 **잇지 않은 것은 하나** — 별 등급의 어두워짐이다(순수 함수 `DimmerStarClassOf`·`StarDimDueOf`·`DimCapitalStar`만 있다; 결정 8의 답 뒤에 잇는다). 나머지는 모두 이어져 있다 — 자리는 §11과 노트 §5.

**씨앗 블록**(결정 23): AI 백성은 B7의 아키타입 행이 오기 전에는 블록을 지닐 길이 없다 — 그러면 두 백성이 맞닿아야 서는 겹침 규칙이 판에서 보이지 않는다. 그래서 생성 때, 아키타입이 `Brick.` 태그를 지니지 않은 AI 백성마다(첫 생애가 다스리는 백성은 받지 않는다) 노브 `BrickSeedTags`(태그 묶음 — 비어 있으면 꺼짐)에서 **파생 정수**로 하나를 고른다(`CosmicSeed`의 용도 13 `Brick`을 끝에 더한다; 태그는 이름 차례). 시뮬 스트림은 뽑지 않는다 — 노브가 빈 장치 커밋에서는 은하가 비트 그대로다. 절차 특성(#36 ②)의 가장 작은 꼴이다(R13 — `docs/20` 규칙 3 ①·③의 문장도 고친다, §7).

### 2.5 정착의 문에 사유 하나 (`[B-7x2]`; 결정 7)

안으로 접힌 백성(`HermitMonolith`·`PlanckMicro`)은 새 정착을 시작하지 못한다. 있는 단일 창구(`SettleBlockOf`·`PeopleSettleBlockOf`·`SettleTargets`)가 답하고, 사유 열거의 끝에 `ECosmicSettleBlock::Inward` 하나를 더한다(묻는 차례는 '정착 중' 바로 뒤) — 명령·AI·패널이 같은 답을 읽는다. AI의 정착 추첨은 그대로 뽑는다(`MaySettle`은 바꾸지 않았다 — 추첨 수가 블록을 따르지 않는다). 새 창구는 없다. 화면의 사유는 `Settle.Block.Inward` '안으로 접힌 백성은 더 넓히지 않는다.'

### 2.6 AI의 변이 걸음 (`[B-7x2]` — AI 단계 끝, 업적 판정 앞의 새 걸음 ★; 결정 12)

다스리지 않는 백성만, `BrickEvolutionPeriodTicks`마다(0이면 꺼짐). `CosmicBricks::MutationDueOf`가 시간 셈의 문턱(`BrickMutationThresholdTicks`)과 지닌 블록에서 정해진 차례로 첫 변이 태그를 답하고 걸음이 그 태그를 더한다. 추첨 없음, 줄 없음. **다스리는 백성은 저절로 바뀌지 않는다** — 갈림 카드로만 바뀐다.

| 차례 | 지닌 블록과 셈 | 얻는 태그 |
|---|---|---|
| 1 | `Brick.Space.Nomad`, `Drifted` ≥ 문턱 | `Brick.Mutation.StarShepherd` |
| 2 | `Brick.Instinct.SwarmTech`, `Built` ≥ 문턱이고 `Linked` ≥ 문턱 | `Brick.Mutation.KesslerTapestry` |
| 3 | `Brick.Diet.StarEater`, `Consumed` ≥ 문턱 | `Brick.Mutation.AbyssalMaw` |

문턱은 셋이 함께 쓰는 노브 하나다(`BrickMutationThresholdTicks`). 값은 §5의 '문턱의 규칙'이 재기 전에 적은 글대로 정한다 — 세 중앙값 가운데 가장 작은 것(리뷰 M3).

### 2.7 끝의 꼴 (`[B-7x2]`의 순수 함수, `[B-7x3]`의 표시; 결정 13)

`CosmicBricks::ShapeOf(const FCivilizationState&)` — 저장하지 않는다, 엔딩의 갈래가 아니다, 두 책에 들지 않는다. 꼴의 태그가 있으면 그 꼴, 없으면 가장 큰 셈(같으면 `Built`·`Consumed`·`Drifted`·`Linked`·`Isolated`·`Endured`의 차례)으로 넷 가운데 하나. 태그의 차례(앞이 이긴다): `UnseenLaw` → `KeptLaw` · `PlanckMicro` → `FoldedSmall` · `HermitMonolith`나 `DysonBrain` → `ShutHouse` · `LivingNebula`나 `DarkMatter` → `SpreadBody` · `Nomad` → `StarHerd` · `SwarmTech` → `WovenSwarm` · `StarEater` → `LastHunger` · `HiveMerge` → `Entwined`. 열두 답의 화면 낱말은 §3.2.

### 2.8 슬라이스 나 — 레인의 단일 창구 (`[B-7x6]`; 결정 14·15·17 — 명세만, 이 브랜치에 코드 없음)

- **창구 `Sim/CosmicLanes`**: `Link(State, A, B)`·`Unlink(State, A, B)`·`CanLinkOf(...)`. 규칙: 같은 `GalaxyRow`, 성역 `sol`은 첫 거절, 늘 양쪽에 대칭, 한 성계의 레인은 상한까지(생성의 `MaxLanesPerSystem` 4 — 노브로), 살아 있는 성계 사이의 `Link`·`Unlink`는 어느 성계도 고립시키지 않는다, 한 틱에 한 걸음, 이름 차례. 파괴의 끊음도 이 창구로 옮기되 고립 금지의 예외다 — 파괴는 죽은 노드의 레인을 모두 끊고(`Sim/CosmicHazards.cpp:442-450`), 그 노드만 잇던 이웃이 고립되는 것은 받는다(리뷰 F8 (i) — 결정 없이 고친 모순).
- **읽는 블록**: `Brick.Space.SystemDragger`(주기마다 딸린 성계의 레인 하나를 수도 쪽 이웃으로 옮긴다 — 설계서 S14는 `project_settle`이 끝난 틱에 수도의 레인 하나를 새 터 쪽으로 옮기는 다른 꼴이다; 하나를 고르는 것도 좌표도 결정 15), `Brick.Space.FoldedTopology`(제 성계 둘을 레인 하나로 잇는다 — 0틱이 아니라 레인 하나의 시간), `Brick.Space.OmniLinked`(제 성계들이 지킴의 몫에서 서로 이웃으로 셈된다), `Brick.Diet.OrbitBrake`(그 백성의 성계를 지나는 레인 시간에 덤 — `LaneTicksOf`가 맥락을 받는다, 최소 1틱).
- **깨지는 가정을 먼저 고친다**: 표현은 `OnGalaxyGenerated`에서만 별과 레인을 짓는다 — 레인의 바뀜은 그 신호가 아니라 지도의 더티 표시로 다시 그린다. '레인 이웃은 언제나 아는 성계'라는 가정과 치명 경고의 레인 시간(`LethalPlaceOf`)을 창구의 걸음 뒤에 다시 본다. 실제 하늘(B8)의 오프라인 그래프는 **처음** 그래프가 된다.
- **슬라이스 나 헤더가 답할 물음**(리뷰 F8): §10 결정 14·15의 '(리뷰 반영 — 개발자 답 대기)'.

### 2.9 슬라이스 다 — 품음과 탄생 (`[B-7x7]`·`[B-7x8]`; 결정 6·16·22 — 명세만)

- **품음**: 주인은 그대로 품긴 백성이다. `HostCivID`가 품는 백성을 가리키면 품긴 성계 수입의 몫이 품는 쪽에 가고, 그 성계가 에워싸일 때 품는 쪽의 함대가 지키는 몫에 든다. 정착된 성계의 술어·위기·주장은 그대로 주인만 읽는다. 맺고 끊는 문은 조약의 문이다(#128 뒤).
- **탄생**: 포자(`Brick.Space.NovaSpore`)와 남은 기계(`Brick.Form.FossilAutomata`)는 '요청'을 남기고 건넘 단계가 받아 백성을 만든다 — `Civs`의 수는 지금처럼 일곱째 단계에서만 바뀐다. 6a의 '백성을 더하는 첫 코드'를 여기로 당긴다. 포자의 흩어짐은 고정된 수의 추첨이다(결정 22).
- **슬라이스 다 헤더가 답할 물음과 받는 조건**(리뷰 F8·DET-9·T4): §10 결정 6·16·22의 '(리뷰 반영 — 개발자 답 대기)'.

### 2.10 같은 틱의 차례 (★ = B7x의 새 걸음)

| 단계 | 지금 | B7x | 시뮬 스트림 |
|---|---|---|---|
| 경제 | 수입 → 틱당 원천 → 안정 → 연속 셈 넷 | 수입의 자리에 블록의 몫, 연속 셈 뒤에 ★ 누적 셈 여섯 | 없음 |
| 위기 | 발생의 몫 → 놓기 | 몫에 안쪽 블록의 곱, 놓을 때 기생 블록의 덤 | 그대로 |
| AI | 선택·정착·칙령 철회·법칙·조약 → 업적 판정 | 업적 판정 앞에 ★ 변이 걸음(주기) | 없음 |
| 건넘 | 성계 없는 백성을 지운다 | (슬라이스 다) ★ 탄생의 요청을 받는다 | 슬라이스 다에서 고정 수 |
| 앎 | 카드의 제시 | 갈림 카드도 같은 길로 | 없음(답마다 1) |

### 2.11 결정론과 저장 — 한눈에

- **새 시뮬 추첨 지점: 슬라이스 가·나 0, 슬라이스 다 1**(포자 — 결정 22). **새 파생 정수 용도 1**(13번 `Brick` — 씨앗 블록; 끝에 더한다; 규칙 3의 문장은 R13·§7).
- **해시**: 새 필드 일곱(셈 여섯·`HostCivID`)이 두 해시에 든다. 태그는 `Traits`에 들어 이미 해시된다.
- **끝에 더한다**: `EConditionType` 셋, 정착 거절 사유 하나, `FCosmicRulesDef` 노브 열일곱, ini 태그 열일곱, `CosmicSeed`의 용도 하나. 새 효과 0.
- **마지막 이름**: 태그와 행의 이름은 세이브에 남는다 — 잎 이름의 고침(결정 3)은 머지 전에만 할 수 있다.
- **헤더 먼저**: 필드와 셈, 열거자 셋, 표의 ★ 둘, 정착 사유, 스키마의 뿌리 목록, (슬라이스 나·다) 레인의 창구와 탄생.

## 3. 데이터

### 3.1 새 행과 그 이름 (이름은 세이브에 남는다 — 본문은 설계서 §3.3·§4.3)

| 행 | 표·커밋 | 자리와 까닭 |
|---|---|---|
| `petition_cross_*` 열하나, `enc_cross_*` 하나 | `DT_Encounters`, `[B-7x5c]` | 갈림 카드 열두 장(설계서 §4.3). 통치자의 열한 장은 `Lottery` — 상시 카드는 칸이 차면 사라지고 즉위마다 다시 온다; 첫 갈림은 꼴을 고르는 선택도 반걸음도 `Brick.Form.Society`를 걷어 내어 닫히고(네 장이 함께), 둘째 갈림은 ①이 준 태그(`CivLacksTrait`)나 걷힌 부모 블록으로 닫힌다 — ②는 태그를 주지 않아 닫지 않으므로 되풀이해 쌓이는 이득을 두지 않는다(값과 작은 효과만 — `petition_road`·`petition_feast`의 꼴; 리뷰 F7의 (나), 데이터는 `[B-7x5c]`; 닫는 표지 (가)는 결정 11의 대안). 방랑자의 한 장은 `Standing`이다 — 도착 카드(`Lottery`)는 자리만 말하고 격차의 문을 지나지 않아서(`Events/CosmicPetitions.cpp:285`) 남의 백성을 말하는 카드는 상시 카드여야 한다. 이름은 `petition_c…`가 있는 `petition_e…` 앞에 정렬되어 가중 추첨의 차례가 바뀐다 — 그래서 켜는 커밋이 기준값을 옮긴다 |
| `event_brick_*` 아홉 | `DT_Events`, `[B-7x5c]` | 겹침 규칙 가운데 슬라이스 가에서 사건 행으로 서는 것(설계서 §3.3): `shut_star`·`heat_at_door`·`drift_plenty`·`emptied_trail`·`ember_wake`·`own_hour`·`fold_gleaning`·`inward_appetite`·`works_run`. 어느 행도 블록을 주지 않는다(AI 백성은 씨앗과 변이 걸음, 방랑자 카드 `enc_cross_*`의 `Nomad`로만 얻는다). `event_brick_left_behind`는 B7 |
| `log_brick_scan_spread` | `DT_LogTemplates`, `[B-7x3]` | 스캔 줄의 얼굴 하나(S15) — 코드가 부르는 줄이라 `CosmicRows` 상수와 `required_rows`를 함께 |
| `arch_seeker`의 `Traits`에 `Brick.Form.Society` | `DT_Civs`, `[B-7x5c]` | 갈림 카드의 출발 표지 |

### 3.2 스트링 테이블 (`ST_CA.csv` — 새 키는 끝에만; 초안, 글 감사가 정한다)

꼴의 낱말 열둘(`Shape.*` — 새 접두어, 낱말 규칙은 아래; 연대기의 사실 줄과 성계 패널의 백성 줄이 함께 쓴다 — 어디에 붙는지는 결정 13): `BuiltOrLinked` '짓고 이은 살림', `ConsumedOrDrifted` '넓히고 삼킨 살림', `Isolated` '홀로 선 살림', `Endured` '눌림 속에 이어 간 살림'(리뷰 T7 — '견디어 남은'은 남음을 공로로 읽힌다), `WovenSwarm` '촘촘히 엮은 일손의 살림', `StarHerd` '머물지 않는 살림', `SpreadBody` '별들 사이에 번진 살림', `FoldedSmall` '티끌 속으로 접힌 살림', `ShutHouse` '한 별에 닫아 건 살림', `KeptLaw` '법을 남긴 살림', `LastHunger` '끝없이 삼키는 살림', `Entwined` '이웃과 뒤엉킨 살림'. 정착 거절의 사유 한 줄(패널 접두어 — 있는 사유의 꼴; 키 `Settle.Block.Inward`는 `string_table.checked_prefixes`에 금지 목록 verdict·holy_words·rescue_words·rung_grasp_words·title_words와 숫자 금지로 든다 — 리뷰 I8, `[B-7x2]`). '살림'은 백성의 사는 모양을 가리키는 낱말이다(업적 '여러 별의 살림'의 결) — 사람의 칭호가 아니다.

**`Shape.*`의 낱말 규칙**(리뷰 T7·I8): `[B-6b5]` 뒤의 `Chronicle.` 목록(rung_grasp_words·title_words·binding_words 포함)에 remnant_words·desolation_words를 더하고, 이 접두어에만 거는 새 목록 `shape_merit_words` `["견디", "남은"]`을 건다 — 걸린 목록은 테스트에 핀으로 박는다. '견디'를 desolation_words에 더하지 않는다(그 목록은 다른 행 규칙에도 걸린다 — main `Tools/data/schema.json:410`·`:418`·`:595`·`:820-822`). '오래 버틴 살림'도 쓰지 않는다('버틴'은 desolation_words — `:73`).

### 3.3 스키마 (`Tools/data/schema.json`·`Tools/cosmic_data_schema.py`·`Tools/test_data_schema.py` — 규칙마다 깨지는 경우 하나)

1. 특성의 뿌리는 `["Trait.", "Brick."]`(네 곳; 깨짐: `Trick.Form.X`).
2. `CivTimeVectorAtLeast`의 `Key`는 여섯 낱말 가운데 하나(깨짐: `Scale`), `Value`는 1 이상의 정수.
3. `BorderingCivHasTrait`는 위기의 `SpawnConditions`와 정착된 성계 주제의 행에 서지 않는다(깨짐: 그 조건을 지닌 위기 행).
4. `Brick.` 태그를 주거나 걷는 효과는 선택의 실패 쪽에 서지 않는다(깨짐: 실패 효과의 `AddTrait Brick.…`); 그런 효과를 지닌 카드는 값도 효과도 다음 카드도 없는 선택 하나를 지닌다(깨짐: 세 선택이 모두 블록을 주는 카드). 검사기의 조건 시험이 태그의 뿌리가 아니라 태그 하나씩만 맞추어서 두 규칙이 열일곱 태그를 늘어놓는다 — **새 블록 태그는 세 규칙(이 둘과 규칙 7)에 함께 더한다.**
5. 갈림 카드는 이름이 아니라 조건으로 가려낸다(검사기의 접두어 시험은 표의 접두어만 받는다): `CivTimeVectorAtLeast`를 지닌 청원 행은 `Lottery`이고(깨짐: `Standing`), `CivHasTrait Brick.Form.Society`가 없으면 `CivLacksTrait`를 지닌다(깨짐: 둘 다 없는 행); 그 조건을 지닌 `enc_` 행은 `Standing`이다(깨짐: `Lottery`). 규칙의 까닭 글은 둘째 갈림의 닫힘을 바르게 적는다 — ①이 준 태그로 닫히고 ②에는 되풀이 이득이 없다(리뷰 F7 — `[B-7x5c]`).
6. 블록의 노브: 진척·연구·위기 몫의 곱은 0보다 크고(삼키는 블록의 에너지 곱만 0 이상) 위기 몫의 곱은 0이 아니다(면역 금지 — K8; 깨짐: 0), 주기는 0 이상, `BrickSeedTags`의 태그는 `Brick.` 아래다. 정착을 막는 둘(`HermitMonolith`·`PlanckMicro`)이 씨앗에 들지 않는 것은 아직 스키마 규칙이 아니다 — 단위 테스트(`test_existence_vocabulary`)가 지금 값만 본다. 기생의 덤은 신정론 불변식의 가장 긴 머묾에 한 번 든다(`Data.Integrity` — 위기마다 한 번, 기생 수만큼 더하지 않는다; 리뷰 DET-4·M10·build-2).
7. K4 — 업적 행(`DT_Feats`)의 완성 조건은 블록 태그를 묻지 않는다: `CivHasTrait`·`CivLacksTrait`의 `Brick.*`(열일곱 태그 — 규칙 4의 나열 꼴), `BorderingCivHasTrait`(깨짐: 업적 행에 `CivHasTrait Brick.Mutation.<변이>`). `CivTimeVectorAtLeast`는 K4의 글(블록·변이·카드) 밖이라 막지 않는다(리뷰 F6).

### 3.4 노브 (`DT_Rules` — 열일곱, 모두 C++ 무동작 값; 값은 `[B-7x5s]`·`[B-7x6]`)

`BrickStellarCrisisSpawnMultiplier` 1 · `BrickDarkMatterEnergyPerTick` 0 · `BrickStarEaterEnergyMultiplier` 1 · `BrickStarEaterRelationTarget` 0 · `BrickStarEaterDimPeriodTicks` 0 · `BrickQuietInsightPerTick` 0 · `BrickSpanEnergyPerLane` 0 · `BrickParasiteCountdownBonusTicks` 0 · `BrickParasiteInsightPerHazard` 0 · `BrickNomadSettleProgressMultiplier` 1 · `BrickInwardResearchMultiplier` 1 · `BrickInwardHazardSpawnMultiplier` 1 · `BrickSwarmResearchMultiplier` 1 · `BrickMergeRelationTarget` 0 · `BrickEvolutionPeriodTicks` 0 · `BrickMutationThresholdTicks` 0 · `BrickSeedTags` 빈 묶음. 곱은 이진수로 떨어지는 값만(1.25·1.5·0.75 …). 노브를 더한 커밋이 `docs/30`을 다시 만든다.

데이터 초안 `[B-7x5a]`가 `DT_Rules.json`에 적은 **시작값**(재기 전의 값이다 — 문턱은 §5의 '문턱의 규칙'이, 나머지는 §5의 판단과 '고침의 규칙'이 정한다; 커밋은 `[B-7x5s]`): 별에 매이지 않는 에너지 0.5, 삼키는 곱 1.5와 눈의 항 −0.25, 일 없는 틱의 통찰 0.25, 레인 하나에 0.125, 기생의 덤 30틱과 통찰 0.25, 정착 진척 1.5, 안쪽의 연구 1.5와 위기 몫 0.5, 엮은 일손의 연구 1.25, 뒤엉킨 눈의 항 +0.25, 변이 걸음 100틱마다·문턱 1,500, 별의 위기 몫 2, 어두워짐 0(꺼짐), 씨앗 블록 여덟(`DarkMatter`·`StarEater`·`SilenceFeed`·`DistanceTension`·`CrisisParasite`·`Nomad`·`SwarmTech`·`HiveMerge`).

### 3.5 글의 규칙 (초안 — `[B-7x4]`의 글 감사가 정한다; 모든 줄에 규칙 11)

블록·시간 셈·꼴의 설계 이름은 화면에 내지 않는다 — 선택은 하는 일로 부르고 꼴은 '살림'의 낱말로만 말한다. 두 문장 이내, 숫자 없음, 사람의 수 없음. 백성의 바뀜은 한 일의 사실과 그 뒤에 서 있게 된 자리로 적는다. 청원의 두 목소리는 서로 다른 선택을 청하고 어느 쪽도 옳다고 적지 않는다. 쓰지 않는 낱말: 구원·중보, 침묵·고요·잠잠·쉼·안식, 울타리·언약·방주, 목자·순례·성소, 불멸·영원·영겁, '신이 되'·'하나가 되'·일체화. 양위 뒤의 줄은 기다림·돌아옴·빈 자리의 그림을 두지 않는다(리뷰 T8). 타락의 교리와 닮은 갈래(뒤엉킴·일 없는 날)는 좋은 끝으로 적지 않는다. 낱말 목록의 기준은 `schema.json`의 이름 붙은 목록이다 — silence_words·rung_grasp_words·title_words와 새 `crossroads_words` `['침묵','울타리','언약','방주','목자','순례','성소','불멸','하나가 되']`; 갈림 카드 열둘·`event_brick_*` 아홉·`log_brick_scan_spread`의 글 칸에 건다(리뷰 T9 — `[B-7x4]`; rung_grasp_words·title_words는 `[B-6b5]`의 것이라 리베이스 뒤).

## 4. 표현 계층 (`[B-7x3]` — 결정 13; 이 브랜치에 코드 없음)

- **끝의 꼴 표지**: 연대기의 생애 장 사실 줄('<다스린 백성> · 칸 N · N주기') 끝에 그 백성의 꼴 낱말 하나를 덧붙인다(`Chronicle.Wandering`의 꼴 — UI 줄이라 숫자 곁에 선다). 표시할 때 `ShapeOf`로 셈하고 저장하지 않는다. 판의 끝 쪽의 고정 줄 셋·까닭 줄·두 책은 그대로다 — '끝마다 다른 것은 까닭 줄뿐'이라는 약속(`UI/CosmicChronicleWidget.h:52-53`)을 깨지 않으려고 발치가 아니라 사실 줄에 둔다. 셈이 모두 0이고 꼴의 태그가 없는 백성에는 아무것도 붙이지 않는다 — `ShapeOf`는 그때도 `BuiltOrLinked`를 답하므로 표시가 먼저 묻는다. `UI.Chronicle`은 1,500틱을 돌린 보드라 셈이 0이 아니다 — `[B-7x3]`이 그 사실 줄 단언에 꼴 낱말을 더해 고친다. **리뷰가 찾은 빈칸 넷**(여러 백성을 다스린 생애·지워진 백성·통치가 끝난 뒤에도 쌓이는 셈·판의 끝 뒤 다시 열기)과 고를 꼴 셋은 §10 결정 13(리뷰 반영 — 개발자 답 대기).
- **백성 줄의 꼴 낱말**: 성계 패널의 백성 줄(보이는 주인일 때만)에 같은 낱말을 한 번 더 쓴다 — 꼴이 숨은 곱으로만 일하면 '보이지 않는 판정'이 된다. 갈림 카드의 값 줄은 있는 길 그대로 값과 판정을 보인다.
- **정착의 거절**: 있는 사유의 자리에 한 줄 — `[B-7x2]`가 이미 넣었다(`UI/CosmicSystemPanelWidget.cpp`의 사유 키와 값 줄을 숨기는 갈래, `ST_CA`의 `Settle.Block.Inward`).
- **카드 창**: 바뀌지 않는다(선택의 조건은 스키마에 없다 — 블록에 따라 달라지는 것은 행의 조건이다).
- (슬라이스 나) 지도는 레인의 바뀜을 더티 표시로 다시 그리고, 좌표가 옮으면 별의 자리를 옮긴다.

## 5. 커맨드렛과 측정 (2026-11-09 ultracode 리뷰가 다시 썼다 — 문턱과 판단은 데이터를 켜기 전에 이 글 그대로 쓴다)

- **끈 짝**: `-WithoutB7xData=1`은 모두 끈다 — 카드 열둘·`event_brick_*` 아홉을 빼고, `arch_seeker`의 `Brick.Form.Society`를 빼고, 노브 열일곱을 C++ 초기값으로, `BrickSeedTags`를 빈 값으로 둔다(노브만이 아니라 태그의 출처 — 카드·씨앗·Society — 를 모두 뺀다). `=2`는 카드·사건·Society만 끄고 노브와 씨앗은 켠 채 둔다 — 사건 행은 씨앗을 받은 AI에서도 서므로 `=2`는 사건과 카드를 함께 잰다.
  - 도우미 `CosmicTests::WithoutB7xData`는 측정 스위치이면서 시험 보드의 무동작 쪽이다(`WithoutB6bData`가 main `Tests/CosmicEndingTests.cpp:158`에서 하는 꼴). `Data.Integrity`가 뺀 집합(카드 열둘·사건 아홉·Society·노브 열일곱의 초기값·빈 `BrickSeedTags`)을 정확히 핀으로 박는다.
  - `-SaveAt`의 거절 목록(main `Tests/CosmicBalanceCommandlet.cpp:1513-1517`의 조건과 Error 글)에 `-WithoutB7xData`를 넣는다. `-Crossroads`는 입력의 꼴이라 넣지 않는다.
- **깨질 수 있는 확인**(데이터를 켜는 커밋마다 — B6b `[B-6b6]` ⓐ~ⓒ의 꼴):
  - ⓐ 그 빌드에서 러너의 다섯 인자 + `-WithoutB7xData=1`이 앞 커밋의 다섯 줄을 글자 그대로(`wrote` 꼬리 빼고) 낸다. 묶음의 끈 쪽 CSV·`_rows.csv`·`_claims.csv`는 앞 커밋 빌드의 같은 시드·인자 판과 `TickMicros`·`SaveBytes`·`SaveRawBytes`를 빼고 같다. 켠 쪽과의 '첫 셋째 칸까지 같음'은 쓰지 않는다 — 씨앗이 생성 때 든다.
  - ⓑ 스위치 대조: 끈 로그마다 `CosmicBalance: tables without B7x's data - rows taken out [<카드 열둘> <사건 아홉>], Brick.Form.Society off arch_seeker, 17 knobs inert` 한 줄이 있고 켠 로그에는 없다. 끈 쪽 `bricks:`의 AI 씨앗·갈림 카드·변이·`event_brick_*` 발화가 모두 0이고, 켠 쪽 합에서는 넷 모두 0보다 크다.
  - ⓒ 모든 로그의 `end check`·`race check`가 0이다.
- **정책**(결정 19): 통치자·방랑자 정책은 갈림 카드에서 값 없는 선택(③)만 고른다 — 블록을 주거나 걷는 선택도, 블록 없는 값 있는 선택(둘째 갈림의 ② 여섯, `enc_cross_drift_fleet`의 ②)도 건너뛴다. 기준값의 판이 되돌릴 수 없는 갈림을 눈 감고 고르지 않고, 닫히지 않는 카드의 값을 되풀이해 걷지 않게 한다.
  - `-Crossroads=<k>`(측정 전용)는 갈림 카드에서 `Brick.` 태그를 주거나 걷는 선택 가운데 값을 낼 수 있는 k번째(1부터)를 고르고, 없으면 ③이다. 이 스위치가 있을 때만 `petition_cross_law_stays`의 ①이 `DeposeRuler`의 거름(main `World/CosmicRulerPolicy.cpp:270-281`)을 지난다(`-Abdicate`의 꼴).
  - 방랑자 정책의 건너뜀은 `[B-6b8]`의 `Pick = Counts.Answers % Row->Choices.Num()` 위에 짓고, `NoCheckByRow`는 실제로 답한 선택으로 센다.
  - **이 정책과 끈 짝 스위치·`bricks:` 줄은 아직 코드에 없다 — 데이터를 켜는 커밋 앞의 `[B-7x4m]`이 넣는다(§8).**
- **새 줄 `bricks:`**(러너가 견주는 세 줄 밖):
  - 판마다, 다스리는 백성과 AI 백성마다(지닌 씨앗 블록과 함께) 여섯 셈을 틱 1,200·2,400·C·끝에 낸다. C는 `[B-6b10]`이 정한 시계다(6,000 고정이 아니다).
  - 온 갈림 카드의 수와 틱, 갈림 카드 답의 틱(틱:수), 고른 꼴, `petition_walls`·`petition_bread`가 온 수, 대기 칸을 잃은 상시 카드의 수.
  - 행별 발화 수 — 측정 쪽이 `OnEventFired`를 센다(세계를 쓰지 않고 `-TellAll`과 무관하다).
  - 코드 규칙의 셈: 기생 덤이 붙은 놓임 수, 기생 통찰이 든 틱, 삼키는·뒤엉킨 항이 0이 아닌 짝 수, AI 변이 수.
- **판의 꼴**(B6b 헤더 §5와 같다 — main `docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md:325`): R = 통치자 정책 판 `-Answer=1 -Build=1 -Focus=1 -Edict=1 -Diplomacy=1 -TellAll=0`, 느린 R = R의 `-Focus=0`, 방랑자 = R + `-Abdicate=1200 -Wander=1`, 입력 없는 판 = 인자 없이; 'R 30'은 그 꼴의 30판이다(짝은 같은 시드의 `-WithoutB7xData=1`). 이 R은 §0의 규칙 번호 R1~R14와 다르다.
- **값보다 먼저 재는 것**: 씨앗과 노브만 켠 빌드(카드·사건 행·Society 없이 — 커밋하지 않는 데이터 사본)로 R 30·입력 없는 30·방랑자 30(시드 1~30, 20,000틱, `-Rules=ProvisionalEndTick:0`)의 `bricks:` 줄을 잰다. 물려받은 전시 동원과 첫 한 성계 때문에 '삼킨 시간'과 '홀로 선 시간'은 기본값으로도 쌓인다 — 문턱은 그 바닥 위에 선다.
- **문턱의 규칙**(재기 전에 이 글 그대로):
  - 첫 갈림 카드: R 30에서 그 셈의 틱 2,400 값의 40번째 백분위수를 100 단위로 내린다(아래 끝 900).
  - 둘째 갈림: 다스리는 셋째 칸 틱 + 1,200에서 같은 셈의 40번째 백분위수.
  - 변이 문턱(노브 하나 `BrickMutationThresholdTicks`): 세 중앙값 가운데 가장 작은 것을 100 단위로 내린다. 각 중앙값은 부모 블록을 씨앗으로 지닌 AI 백성의 해당 셈의 틱 C 값이다(`Nomad` → `Drifted`, `SwarmTech` → min(`Built`, `Linked`), `StarEater` → `Consumed`).
  - 방랑자 카드: 방랑자 30(`-Abdicate=1200 -Wander=1`)에서 그릇이 다른 백성의 정착된 성계에 처음 선 틱마다 그 주인의 `Drifted`를 읽고, 그 중앙값을 100 단위로 내린다(아래 끝 900).
- **미리 적은 판단**(30건 미만은 '재지 않음 — 잰 N'):
  - ⓪ 겹침 규칙마다 재는 칸을 적는다. S04·S06·S07·S11·S12는 R 30·입력 없는 30에서, S01·S05·S08은 `-Crossroads` 칸에서만 잰다. 칸 밖의 규칙은 '칸 밖'으로 적고 조건을 고치는 근거로 쓰지 않는다. 셈은 `_rows.csv`가 아니라 `bricks:`의 행별 발화와 코드 규칙의 셈으로 한다.
  - ① 통치자 정책 30판에서 판의 끝 전에 갈림 카드가 한 장 이상 온 판이 20/30 이상이다(B4의 교훈: 드문 조건의 내용은 보이지 않았다).
  - ② 첫 갈림 카드의 틱 중앙값이 1,200~3,000 안이다.
  - ③ R 30·방랑자 30에서, 종류마다 30판을 합친 몫의 켠−끈 차이가 +2%p 이하다(95% Wilson 구간을 함께 적는다). 입력 없는 칸은 쓰지 않는다 — 답을 하지 않아 대기 칸이 판 끝까지 첫 카드로 막혀 있다.
    - 주장 카드: `TriggerClaimTaken ÷ TriggerClaim`(`spirit:` 줄) — 따로 적고, 잃은 판마다 그 틱에 칸을 쥔 카드를 `pressure:`에서 적는다.
    - 잰 카드 `petition_pressure_measured`(Named 카드): `TriggerPressureTaken ÷ TriggerPressure`.
    - 안정도 청원 `petition_walls`·`petition_bread`: `StandingLost[row] ÷ (온 수 + StandingLost[row])`.
  - ③′ 방랑자 60 × 6,000(R + `-Abdicate=1200 -Wander=1`)과 끈 짝에서 `enc_witness`의 제시 수와 도착 카드의 수를 본다 — 끈 짝 대비 줄어든 몫이 2%p 안이어야 한다. `enc_cross_drift_fleet`이 판마다 온 수도 적는다. 넘으면 그 카드의 `BaseWeight`를 0.25로 낮추거나 `CivTierAtLeast 2`를 더한다.
  - ④ 러너의 다섯 인자를 켠 쪽과 끈 쪽(`-WithoutB7xData=1`)으로 손으로 하나씩, 두 번씩 돌려 `stages:`의 경제·AI µs와 틱 평균의 켠−끈 차이(두 번의 중앙값)를 본다. 틱 평균 +5% 안이면 통과다. 넘으면 기생의 국경 걸음과 레인 폭의 BFS를 먼저 본다. 병렬 묶음의 `TickMicros`로는 판단하지 않는다.
  - ⑤ R 30에서 판의 끝(기록의 틱, 없으면 C)까지의 결정 가운데 갈림 카드의 답을 뺀 수 — 판마다 켠−끈 차이의 중앙값이 −1 이상이다. 또 다스리는 셋째 칸부터 첫 기록과 C 가운데 이른 것까지, 600틱 창마다 갈림 답을 뺀 결정 수의 판별 중앙값이 끈 쪽에서 1 이상이고 켠 쪽에서 1 미만인 판이 3/30 이하다.
  - ⑥ **B6b의 판단을 다시 세운다**(B6b 헤더 §5.3 — main `docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md:336-354`; 짝은 같은 시드의 `-WithoutB7xData=1`).
    - 칸: 묶음 B의 칸(R 60·느린 R 60·방랑자 60·입력 없는 60 × 20,000, 시드 1~60, `-Rules=ProvisionalEndTick:0`).
    - 판단: ①(40/60 이상), ②(다른 질서 15~35%, (가)·(나)·(다) 가름 그대로), ③(넷째 → 다섯째 중앙값 3,600 이하), 시계 규칙 ⓔ의 (i)·(ii)를 `[B-6b10]`의 C로 본다(C를 다시 셈하지 않는다). 첫 AI 셋째 칸 중앙값은 끈 짝의 ±10% 안이고, '다스리는 셋째 칸 − 첫 AI 셋째 칸'의 부호가 그대로여야 한다.
    - 적기만 할 것: 다스리는 둘째·셋째 칸 틱, 첫 AI 셋째·넷째 칸 틱, 제동이 막은 수, 절멸(다스리는·AI), AI 단계 µs, 묶음 C의 'C에서 읽는 것'(#136·#142·#132·B1 지킴·#129·#154·15% 미만). Society 연구가 칙령 자리를 앞당겨 AI 법칙 걸음이 일찍 서는지, 관계 항이 AI 조약 걸음과 `feat_passage_by_ark`(`CivHasTreaty`)에 닿는지도 적는다.
    - 어긋나면 B7x의 노브를 먼저 고친다(씨앗 묶음에서 연구·통찰·에너지 블록을 빼거나 그 곱·더함을 낮춘다). B6b의 r·c·N·P_bind·C(B6b의 경주 값 — r = `AIUpperRungProgressMultiplier`, 셋째 칸 위 AI 건설 배율로 시작값 0.375(B6b 결정 17, main B6b 헤더 `:434`); c = 절멸 위기 `crisis_species_extinction`의 확률로 시작값 1e-4(결정 4, `:421`); P_bind = `project_binding`의 진척으로 시작값 1,800(결정 12, `:429`); N = `feat_laws_and_binding`이 묻는 은하 법칙을 지닌 틱(`CivLawsHeldTicksAtLeast`)으로 시작값 1,200(결정 15, `:432`); C는 `[B-6b10]`이 정한 시계)는 건드리지 않는다. 한 번 고쳐도 어긋나면 그 수와 함께 [개발자](결정 1).
  - ⑦ 판의 끝 전 주장 수(끈 짝 대비 — 설계서 S04의 약속)와 각성 지연 가운데 기생 덤의 몫(#132 띠 곁에)을 적는다.
  - ⑧ 적기만: 스스로 사라진 백성 수와 초신성 업적 수(S01·S06), #129·#130(S09), 인구 바닥 틱(S12).
    - `-Crossroads` 칸(R + `=1`·`=2` 각 30, 시드 1~30, 20,000틱, 짝은 같은 시드의 R)에서: 안으로 접힌 둘의 셋째 칸 도달 몫과 틱, 양위 뒤 남긴 법 백성의 안정도 0.2 아래 첫 틱·반란·지워짐·`LawsHeldTicks`, S05 판의 후반 순이익.
    - C11 ①을 고른 판의 틱 C 안정도와 받은 공성 수가 ②를 고른 판보다 낫지 않은지 본다. 나으면 그때 C11 ①의 `ChangeStability +0.08`을 낮춘다.
- **고침의 규칙**: 판단이 어긋나 값을 고치면, 탐침은 시드 1~30(커밋하지 않는 사본)으로 하고 고친 값의 판단은 새 시드 31~60으로 한 번 한다. 탐침의 판은 판단에 쓰지 않는다.
  - ①이 못 미치면 문턱을 내린다. ②가 1,200 아래면 문턱을 올리고, 3,000 위면 내린다(100 단위).
  - ③이 넘으면 갈림 카드의 문턱을 올리거나 `BaseWeight`를 0.5로 낮춘다(어느 쪽인지 수와 함께 적는다).
  - ④가 넘으면 경제 단계의 셈을 먼저 잰다. ⑤가 줄면 [개발자].
  - 그래도 어긋나면 그 수와 함께 [개발자](③은 #158과 함께).
- **하네스**: B6b의 묶음 B 스크립트(`[B-6b8]` 세션이 쓴 것 — 저장소 밖에 있다; 저장소에 들이면 그 경로를 여기에 적는다; 묶음 B가 `[B-6b8]`의 일인 근거는 main B6b 헤더 `:394`)를 B7x 묶음 곁에 베껴 끈 스위치·기대 줄·짝 검사의 꼴을 인자로 받게 하면 하네스를 새로 쓰지 않는다(저장소에 들이는 것은 선택).

## 6. 테스트 (새 단언은 변이로 한 번씩 깨뜨린다 — 패치는 `Tools/mutations/B7x/`, 다른 PC에서)

- **`[B-7x1]`** — `Sim.SaveTimeVectors`·`Sim.HashCoversTimeVectors`(파일 `Tests/CosmicExistenceBricksTests.cpp`).
- **`[B-7x2]`** — `Sim.TimeVectorsCount`(술어마다 그 사실이 설 때만 +1, 대조 백성은 그대로, 스트림의 자리 같음)·`Sim.BrickShares`(태그 지닌 백성만 달라지고 무동작 값에서는 아무도 달라지지 않는다)·`Sim.BrickSettleGate`·`Sim.MutationStep`·`Sim.ShapeOf`·`Sim.SeedBricks`(같은 시드는 같은 블록, 노브가 비면 은하가 그대로, 스트림의 자리 같음)·`Events.BrickConditions`, 넓힌 것은 `Events.CheckIsPure`·`Data.Integrity`(뿌리 목록)·`Sim.SeedDerive`(용도 13)·`Sim.SaveRoundTrip`의 도우미. 자동화 테스트는 `[B-6b12]` 끝의 `STATUS.md` 수 + 9(새 `Sim.KeptLawEdicts`를 두면 + 10 — 리뷰 I10).
  - **리뷰가 더한 단언**(노트 §5.4의 넘김 — 리베이스 때): 보드의 무동작 깔기(시1), `Sim.SeedBricks`의 Pass 0·1 기대값(시2), 신정론 불변식의 기생 덤(시3 — `[B-6b8b]` 뒤), 남긴 법의 칙령(시4 — 결정 18 대기), 미시 곱의 제외(시5), `DimCapitalStar`의 거절 셋(시6), 표류 목표의 블록 항(시7), 스키마 검사기의 깨짐(시8).
  - **보드 규칙**(리뷰 DET-3·build-1): 보드는 노브를 무동작으로, 특성은 블록 없이 깐다(`CosmicTests::WithInertBricks`·`StripBricks`) — 게임 값에 기대지 않는다. 게임 규칙을 읽는 공유 시드 42 보드(main `Tests/CosmicDiplomacyTests.cpp:132`의 `LayBoard`부터)에도 같다.
- **`[B-7x4m]`** — 정책의 건너뜀(테스트 안에서 지은 벤치 카드로), 끈 스위치와 도우미의 핀(`Data.Integrity`), `-SaveAt`의 거절 목록.
- **`[B-7x5c]`** — `Sim.CrossroadsPresent`(문턱·한 번만·대기 칸, 둘째 갈림의 ② 뒤 되풀이 이득이 없다), `Sim.CrossroadsAnswer`(블록이 맥락 백성에 닿는다), 정책의 건너뜀을 실제 카드로 한 번 더, `Sim.KnowingInert`에 갈림 카드의 경우.
- **뜻이 바뀌는 테스트**(`[B-7x5s]`·`[B-7x5c]` — 씨앗이 생성 때부터 모든 AI 백성을 바꾼다; 리뷰 I7·DET-3·build-1·I3): `Sim.SightWalkMoved`, EndingTables의 테스트들, `Sim.SuccessionPossible`, `Sim.WandererLoop`, `Sim.PhaseTwoExit`, `Feats.Reachable`, `Sim.WitnessCard`, `UI.Chronicle`, `Sim.SeedBricks`.
  - 규칙: B7x가 주제가 아니면 `CosmicTests::WithoutB7xData`를 가장 바깥에 둔다 — 시드와 기대값으로 흡수하지 않는다. `WithoutB6bData`가 이미 바깥에 있으면 그 바깥에 둔다(둘은 겹치지 않는 행과 노브를 빼므로 차례가 결과를 바꾸지 않는다).
  - `Sim.WandererLoop`과 `Sim.PhaseTwoExit`는 `[B-6b8]` 브리프 §2.4의 선례대로 먼저 까닭을 찾고, B7x가 까닭이 아닐 때만 끈다. B1 지킴이 바뀌면 [개발자] 발견으로 적는다.
  - `Sim.WitnessCard`(`[B-6b8]` — 게임 조우를 쓴다)가 깨지면 가장 바깥에 `WithoutB7xData`를 두고, B6b 노트 §5에 '증언 카드는 더는 유일한 방랑자 상시 카드가 아니다'를 적는다.
- **변이 패치의 견줌**(`[B-7x1]`·`[B-7x2]`·`[B-7x5s]`·`[B-7x5c]` — 리뷰 I6): `git apply --check`로 모든 패치를 커밋 전후에 견준다. 서지 않게 된 패치마다 같은 변이를 `b7x<N>_` 이름으로 다시 뜨고, README의 옛 줄에 `(no longer applies since [B-7x2] - taken again as …)`를 단다(main `Tools/mutations/README.md:165`의 꼴). 리베이스로 멈출 일곱은 노트 §5.4 시12.

## 7. 문서 (B6b가 끝난 뒤 — 이 브랜치는 살아 있는 규칙 문서(`docs/20`·`docs/21`·`docs/50`·`docs/60`의 표·`STATUS.md`·`CLAUDE.md`)를 건드리지 않았다. 다만 설계서와 `docs/OVERVIEW_as_of_B6a.html`을 더했고(9be5b3e) 최상위에 `promptforother.txt`를 두었다 — 셋의 처리는 아래 `[B-7x0]`; 리뷰 T1·F12)

- **`[B-7x1]`과 함께, 코드 앞에**: `docs/20`의 저장 문단(새 필드 일곱·두 해시가 옮는다)과 틱 순서(경제 끝의 누적 셈, AI 끝의 변이 걸음), `docs/21` §7.2·§7.3·§7.4. 파생 정수 용도 13의 문장 셋(R13 — 리뷰 DET-1; `main` `10205b6`의 줄):
  - `docs/20:204` ①의 '생성' 뒤에 '(AI 백성의 씨앗 블록은 ③ — B7x)'.
  - `:208` ③ 목록의 '건너온 백성의 이름과 아키타입' 뒤에 'AI 백성의 씨앗 블록(B7x, 용도 13 `Brick` — 이름 CivID, 번호 0, 태그는 이름 차례; 생성 때 세계의 사실을 정한다, `BrickSeedTags`가 비면 없음)'.
  - `:220` '시뮬 스트림 `Initialize(Seed)`에서 생성된다' 뒤에 '(AI 백성의 씨앗 블록만 파생 정수 — B7x)'.
- **승인 커밋 `[B-7x0]`**: `docs/10_DESIGN/systems.md:381`의 판정에 번복의 표지(R1 — 결정 2), `docs/50_ROADMAP.md`의 B 행에 B7x의 칸과 §12.1의 수량(R8 — 결정 11), `docs/60_DECISIONS/DECISIONS_LOG.md`의 B7x 절, `OPEN_QUESTIONS.md`의 #88(R11)·#36 ②(R13) 행의 고침과 새 행(노트 §3), B2 헤더 §2.6(`docs/60_DECISIONS/headers/B2_edicts_and_research.md:125`·`:129`)에 B7x 표지(R12), `STATUS.md`. 브랜치 최상위의 `promptforother.txt`는 지운다(리뷰 F12). 설계서 원안과 `docs/OVERVIEW_as_of_B6a.html`의 처리는 결정 21(리뷰 반영 — 개발자 답 대기).
  - `CLAUDE.md`는 36바이트만 남았다 — 줄을 더하지 않는다. 규칙 13의 창구 목록에 더할 낱말은 슬라이스 나에서 다른 줄을 줄이며 넣는다(R4). 설계서 원안의 동결본을 두면(결정 21) 있는 `docs/archive` 행을 늘리지 않게 고쳐 쓰거나('옛 판·원안의 동결본(고치지 않는다)') `docs/SECTION_MAP.md`에만 적는다. 규칙 3의 괄호에 씨앗 블록을 넣을 자리가 없으면 `docs/20` 전문에 맡긴다.
- **`[B-7x9]`**: 설계서를 받은 꼴로 다시 쓴다(지금은 머리의 상태 표지와 새 절만 더했다), `docs/15_GLOSSARY.md`·`docs/70`. `docs/OVERVIEW_as_of_B6a.html`을 남기면(결정 21 ㉯) 재감사 목록에 든다.

## 8. 순서·검증·effort (`WORKPLAN.md:61`의 B 행; B6b가 끝난 `main` — `[B-6b12]` 뒤 — 위로 다시 쌓는다; 2026-11-09 리뷰가 다시 정했다)

브랜치의 차례(코드가 헤더보다 먼저, 데이터 초안이 표현·글·측정 스위치보다 먼저)는 그대로 리베이스할 수 없다 — aa09543은 커밋 메시지대로 `Data.SourcesInSync`를 깨고, 시험 보드가 게임 값을 물려받아 테스트가 깨진다(리뷰 I11·build-1). 아래 차례로 다시 쌓는다. **`[B-7x1]`과 `[B-7x2]`는 하나로 합쳐 다시 쌓는다** — 두 해시를 한 번만 옮긴다(§0이 허락한다); 나눠 두면 아래 2·3의 검증을 따로 한다. 데이터는 측정의 건너뜀·끈 짝·`bricks:` 줄(`[B-7x4m]`)과 글(`[B-7x4]`) 뒤에만 켠다.

| 차례 | 커밋 | 무엇으로 짓는가 | effort | 증명할 것 |
|---|---|---|---|---|
| 0 | (리베이스 전) | `git merge-tree`로 B6b가 끝난 main과 브랜치를 견준다(읽기만) | — | 충돌 자리가 §8.1과 같은가 |
| 1 | `[B-7x0]` 승인 | 4555c89의 헤더, 9be5b3e의 설계서, 0bffb8e의 문서 + 리뷰의 글 고침(이 판)과 개발자의 답. `promptforother.txt`는 뺀다. OVERVIEW html과 설계서 원안은 결정 21의 답대로. §7의 승인 문서 | xhigh | `python -I Tools/check_docs.py` 0, `CLAUDE.md` 크기, 코드 없음. ultracode 적대 리뷰는 2026-11-09에 했다(§11) — 남은 것은 [개발자] 답 뒤의 고침 |
| 2 | `[B-7x1]` 필드 | 01eef6f + 결정 3의 이름(태그·접근자), 결정 13 ㉠이면 `ReignShapes`, 결정 6 (ㄷ)이면 탄생 요청 필드. `docs/20`(저장 문단·씨앗 문장 — §7)·`docs/21`을 먼저 | xhigh | 에디터를 닫고 전체 빌드; 러너 full 두 번; **두 해시가 옮는다**(새 해시를 커밋 메시지에); 다섯 판의 CSV·`_rows.csv`·`_claims.csv`(`TickMicros`·`SaveBytes`·`SaveRawBytes` 빼고)와 `ending:`·`race:` 줄이 앞 커밋과 글자 그대로; `Sim.SaveTimeVectors`·`Sim.HashCoversTimeVectors`, 옛 세이브; 변이 패치 apply 전후; 에셋과 `generated.sha1`; `-UpdateBaseline` |
| 3 | `[B-7x2]` 장치 | 492c4f0 + 5969bd5 + 85f297c를 하나로. §8.1의 해소, 노트 §5.4의 `[B-7x2]` 넘김(코드·스키마·테스트), 보드 고침(`Sim.DiplomacyRules` 등) | xhigh | 러너 full 두 번; **두 해시가 한 번 더 옮는다**(누적 셈); CSV의 결정적 칸과 `ending:`·`race:`가 그대로; 선택(리뷰 DET-2): 해셔가 `TimeVectors`·`HostCivID`를 건너뛰게 한 로컬 탐침 빌드로 다섯 판과 해시 판 둘을 돌려 리베이스한 main의 `Tools/baselines/seed42.json`과 두 해시가 비트 그대로인지 본 뒤 탐침을 빼고 `-UpdateBaseline`(커밋 메시지에 한 줄; 탐침은 `Tools/mutations` 아래에 두지 않는다); 새 단언마다 변이 한 번; apply 전후로 멈춘 패치를 다시 뜬다; `docs/30`; ultracode 구현 리뷰 |
| 4 | `[B-7x4m]` 측정 | 정책의 건너뜀(통치자·방랑자)·`-WithoutB7xData=1`·`=2`와 도우미·`-Crossroads=<k>`·새 줄 `bricks:`·`-SaveAt`의 거절 목록(§5) | xhigh | 에디터를 닫고 전체 빌드(`Tests/CosmicTestHelpers.h`·`World/CosmicRulerPolicy.h`); 러너 full 두 번; **비트 그대로**(다섯 줄·해시 판 둘, `-TellAll=0` 판 포함 — 데이터가 없어 건너뛸 선택이 없다); 변이(건너뜀을 뺌 — 벤치 카드로; 끈 스위치가 씨앗·노브를 남김). 그다음 '값보다 먼저 재는 것'을 커밋하지 않는 데이터 사본으로 잰다 |
| 5 | `[B-7x3]` 표현 | 결정 13의 답, 스캔 줄의 얼굴, 외교 창 툴팁(표류 목표의 새 항), 패널의 꼴 낱말 | high | 비트 그대로; `UI.Chronicle`의 경우(노트 §5.4 시9); 화면·문구 리뷰 |
| 6 | `[B-7x4]` 글 | 노트에 둔 카드·사건 행의 감사한 초안(노트 §5.4 데2), `ST_CA` 키, 낱말 규칙(갈림·`Shape.`)과 `shape_merit_words`; ini의 DevComment도 감사한다 | high | 세계 해시 그대로(행은 아직 켜지 않는다); ultracode 글 감사 |
| 7 | `[B-7x5s]` 씨앗·노브 | aa09543의 `DT_Rules` 몫(노브 열일곱의 값·`BrickSeedTags`·변이 노브) | high | **세계가 옮는다(AI만)**; 깨질 수 있는 확인 ⓐ~ⓒ(=1); 값보다 먼저 재는 것과 문턱의 규칙; 판단 ⑥(B6b 다시 잼); 뜻이 바뀌는 테스트(§6); 러너 full `-Data` 두 번 |
| 8 | `[B-7x5c]` 카드·사건·Society | aa09543의 `DT_Encounters`·`DT_Events`·`DT_Civs` 몫 + 노트 §5.4의 데1·데2·데5; 6의 감사한 글을 글자 그대로 | high | **세계가 옮는다**; 깨질 수 있는 확인(=1과 =2); 판단 ⓪~⑤·③′·⑦·⑧, ⑥ 다시; §6의 `[B-7x5c]` 테스트와 뜻이 바뀌는 테스트; ultracode 밸런스 리뷰 |
| 9 | `[B-7x5r]` | 판단이 노브를 부를 때만, 제 커밋 | high | 고침의 규칙(새 시드 31~60); 미리 적은 판단; `STATUS.md`의 기준값 사실(ruled T3·cards·decisions)을 이 커밋이 고친다 |
| 10~ | `[B-7x6]`·`[B-7x7]`·`[B-7x8]`·`[B-7x9]` | 슬라이스 나(레인의 창구와 그 블록, 지도)·다(품음, 탄생)·문서 | xhigh | [개발자] 14·15·16·22(와 6)의 답 뒤; B8 헤더·6a와 맞춤; `[B-7x9]`는 재감사 워크플로 |

- **이 브랜치에 이미 있는 것**: `[B-7x1]`·`[B-7x2]`(+`[B-7x2r]`·`[B-7x2c]`)의 코드(빌드 전)와 `[B-7x5a]`의 데이터 초안. `[B-7x5a]`는 제 커밋으로 옮기지 않고 `[B-7x5s]`·`[B-7x5c]`로 갈라 다시 적는다 — 그 전까지 데이터의 노브는 무동작이고 `BrickSeedTags`는 비어 있다.
- 해시를 옮기는 새 길과 데이터 켜기는 한 커밋에 두지 않는다.
- **커밋마다**: 러너 full 두 번(데이터 커밋은 `-Data`), 헤더를 고치면 에디터를 닫고 전체 빌드, 새 단언마다 변이 한 번, 변이 패치 apply 전후 견줌(§6).

### 8.1 예상 충돌과 해소 (main을 앞에 둔다 — 리뷰가 `git merge-tree cef8cc7 10205b6 0bffb8e`로 확인한 것과 B6b 브리프에서 읽은 앞으로의 것; 리뷰 I1)

브랜치가 처음 적은 목록('충돌은 노브의 끝·스키마·`ST_CA.csv`에서 난다', 'main 쪽을 앞에 두고 양쪽을 다 살린다')은 양쪽으로 틀렸다 — 이름한 `FCosmicRulesDef`·`EConditionType`·`ECosmicSettleBlock`·`CosmicSeed`·ini의 끝은 충돌하지 않고, `words`에 '양쪽을 다 살린다'를 그대로 하면 깨진다.

- **`[B-7x2]` — 지금 충돌**:
  1. `Tools/data/ST_CA.csv`의 끝: main의 키(`Rung.*`·`[B-6b4]`·`[B-6b5]`의 키) 뒤에 `"Settle.Block.Inward"`. `[B-6b7]`의 `Edict.TipWide`는 제자리 고침이라 끝과 떨어져 있다.
  2. `Tools/data/schema.json`의 `words`: **main의 줄은 모두 그대로 두고**(`[B-6b5]`의 13낱말 `holy_passive`와 아홉 목록, `[B-6b6]`의 `ark_words`·'노아' 등) 브랜치의 옛 `holy_passive`·`record_object_words` 줄은 버리며, **`time_vectors` 한 줄만 `words`의 끝에 더한다**. 두 벌을 두면 JSON이 깨지거나 `check_data`가 '두 번 적힌 키'로 멈춘다(main `Tools/cosmic_data_schema.py:300-316`). `words`의 끝에 두면 `[B-6b5]`의 두 스키마 변이 패치가 산다.
  3. `Tools/test_data_schema.py`의 `walked` 핀: 더하지 말고 스키마에서 다시 센다(10205b6 기준으로는 `[B-7x2]` 311, `[B-7x2r]` 312, `[B-7x5]` 383 — B6b가 끝난 main의 수에서 다시 센다; `[B-6b6]` +11, `[B-6b8]`도 더한다).
- **`[B-7x2]` — 저절로 합쳐지되 다시 읽을 것**: `UI/CosmicSystemPanelWidget.cpp`(main의 `ReasonOf(…, Tier)`·`SettleReason = Reason;` 곁의 static_assert 12와 `Inward` 힌트 — B6b가 정착 사유를 더했으면 13; 이 파일이 충돌로 멈춘다는 주장은 반박되었다 — 3방향 병합 0 충돌), `Tests/CosmicPanelTests.cpp`, `DT_Events.json`.
- **`[B-7x2]` — B6b가 앞으로 고칠 곳 곁**: `Tests/CosmicDataTests.cpp` — `[B-6b7]` §2.1과 `[B-6b8b]`가 `TheodicyProblems`(main 지금 `:2477-2568`)를 고친다; 브랜치는 `:1790`의 뿌리 거울만 고쳐 겹치지 않고, 기생 항은 `[B-6b8b]` 뒤의 글에 얹는다. `Data/CosmicDefinitionRows.h` — `[B-6b8b]`는 `:992` 주석만, B7x 노브는 `GalaxyLawsToHold`(`:1407`) 뒤다. `DT_Rules.json` — `[B-6b7]`은 `:17`(r), `[B-6b8b]`는 `:27`(지킴 몫)을 제자리에서 고치고 B7x는 끝에 더한다. 충돌은 예상하지 않지만 0단계에서 확인한다.
- **`[B-7x1]`**: `Config/DefaultGameplayTags.ini` — `[B-6b6]`은 `:20`만, `[B-6b7]`의 ini 고침은 0단계에서 확인한다; B7x 태그 열일곱은 다른 자리다. 스키마 뿌리 세 곳은 줄이 옮았을 뿐이다. `FCosmicRulesDef`·`EConditionType`·`ECosmicSettleBlock`·`CosmicSeed`의 끝은 `[B-6b5]` 뒤의 B6b가 더하지 않아 충돌하지 않는다. 정착 사유가 B6b에서 늘었으면 `Inward`의 자리와 static_assert 셋(branch `Sim/CosmicMegaprojects.cpp:569`, `UI/CosmicSystemPanelWidget.cpp:1065`, `Tests/CosmicBalanceCommandlet.cpp:1098`)이 13이 된다 — 0단계에서 확인한다.
- **`[B-7x5s]`·`[B-7x5c]`**: `DT_Events.json`의 끝(`[B-6b8]`의 `event_era_endured` 뒤에 `event_brick_*` 아홉), `DT_Encounters.json`의 끝(`enc_witness` 뒤에 갈림 열둘), `schema.json`의 DT_Encounters rules 끝(`[B-6b8]`의 Rule W 뒤에 B7x의 다섯 규칙 — branch `:1180-1281`), `rule_setups`와 `fixture_tables`(`[B-6b6]`·`[B-6b8]`이 더한 것 뒤에), `walked`. `DT_Rules.json` 끝의 노브 값, `DT_Civs.json`의 `arch_seeker`.
- **문서**: B7x 문서는 `[B-6b12]`가 쓴 `STATUS`·`DECISIONS_LOG`·`OPEN_QUESTIONS` 위에 새로 쓴다. 충돌은 없지만 번호와 수는 그 끝의 값을 쓴다.
- **B6b와 뜻이 닿는 곳**(다시 볼 열넷)과 변이 패치의 멈춤은 노트 §5.5.

## 9. 손 확인 (개발자, 콘솔 없이 — `[B-7x5c]` 뒤)

세이브는 데이터를 켠 마지막 커밋 뒤 `-SaveAt`으로 만든다(R + `-Seed=<시드> -SaveAt=<틱>`; 갈림을 이미 고른 세계가 필요한 ④·⑥은 `-Crossroads=1`도). 시드와 틱은 §5의 30판에서 골라 노트 §5에 명령 줄·기대 틱과 함께 적고, 마지막 코드 커밋의 빌드로 다시 만든다(B6b §9 2의 꼴). ④는 `petition_cross_isolated`(`Isolated` 1,500·`Brick.Form.Society`)가 오는 틱 − 10 또는 그 답 뒤, ⑥은 `petition_cross_law_stays`(셋째 칸·`Endured` 2,000)가 오는 틱 − 10이다(리뷰 M13).

1. 시드 42 보통 판의 첫 10분(틱 600)의 결정·말해진 줄이 커맨드렛 `ruler:` 줄의 decisions by 600·told by 600(켠 쪽의 시드 42 값과 30판 중앙값을 노트에 적어 둔다) 언저리이고, 갈림 카드·`event_brick_*` 줄이 그 안에 오지 않는가.
2. 전시 동원을 쥔 채 놀아 '삼킨 시간'의 갈림 카드가 오는가, 두 목소리가 서로 다른 선택을 청하는가, 값도 문도 없는 선택이 있는가.
3. 갈림을 고른 뒤 그 백성의 수입·연구·정착이 달라진 것이 패널의 수로 읽히는가, 첫 갈림과 둘째 갈림의 ①은 다시 오지 않고, 둘째 갈림의 ②를 고른 카드가 다시 와도 되풀이해 쌓이는 이득이 없는가(리뷰 F7).
4. 안으로 접는 갈림 뒤 정착 단추가 사유와 함께 흐린가.
5. 판의 끝 쪽의 생애 장 사실 줄에 꼴의 낱말이 붙고 칭호나 엔딩의 이름으로 읽히지 않는가 — 설교·승리·'신이 됨'으로 읽히면 낱말을 뺀다. (결정 13의 답에 맞게 고친다 — ㉣이면 '패널과 [백성] 탭의 낱말이 칭호나 엔딩 이름으로 읽히지 않는가'.)
6. 왕좌를 내려놓는 갈림 뒤 옛 백성이 섭정 아래에서 그 꼴을 지닌 채 가는가.

## 10. 결정 (권장안 먼저, 대안과 그 까닭; **[개발자]**는 상시 승인 밖)

1. **[개발자] B7x의 자리** — 권장: 로드맵의 B7·B8은 그대로 두고 이 헤더는 확장 블록 **B7x**다. 슬라이스 가는 B6b가 끝난 뒤·B7 본 블록 앞, 나는 B8의 레인 결정과 함께, 다는 6a와 함께. 까닭: B7은 이미 물려받은 물음 스물 남짓과 ±30% 목표 셋의 다시 잼을 지고 있어 한 블록에 섞으면 측정이 흐려진다; 저장 필드는 마지막 리베이스(B8) 전에 드는 편이 싸다. 대안: ㉮ B7 안에 넣는다 ㉯ 통째로 Phase 6+ — 필드의 해시 옮김이 테스터 뒤로 간다. 닿는 것: `docs/50_ROADMAP.md:12`. *왜 개발자인가*: 블록의 차례와 이름이다.
    - **(리뷰 반영 — 개발자 답 대기)** 권장은 그대로다. 다만 슬라이스 가가 B6b 바로 뒤에 들면 씨앗 블록 여덟이 B6b가 막 맞춘 경주를 옮긴다(M1·I4·F3). 그래서 데이터 커밋을 `[B-7x5s]`(씨앗·노브)와 `[B-7x5c]`(카드·사건·Society)로 가르고 B6b의 판단을 다시 세우는 ⑥을 더했다(§5·§8 — 이 고침은 결정을 기다리지 않는다). 노브를 고치면 `[B-7x5r]`을 둔다.
    - **보아야 할 귀결**: 어긋나면 **B7x의 노브가 먼저 물러선다** — B6b의 r·c·N·P_bind·C(풀이는 §5 판단 ⑥의 끝 항)는 건드리지 않는다. 이 규칙을 받아야 한다(B6b 결정 35와 이 헤더 결정 23 사이). 한 번 고쳐도 어긋나면 그 수와 함께 다시 묻는다.
2. **[개발자] `Brick.` 뿌리와 판정의 번복** (R1) — 권장: 새 뿌리 `Brick.`을 두고 스키마의 뿌리 네 곳을 목록으로 넓힌다; "`Trait.Diet.*` 없음 … 생태 시뮬은 만들지 않는다"(`systems.md:381`)는 '읽는 곳이 있는 블록은 선언한다'로 고친다. '읽는 곳이 없는 태그는 ini에 넣지 않는다'는 지킨다 — 그래서 예순이 아니라 열일곱이다. 대안: ㉮ `Trait.` 아래에 둔다(스키마 무변경) — 요청과 설계서의 이름과 갈린다. *왜 개발자인가*: 서 있는 판정을 뒤집는다.
    - **(리뷰 반영 — 개발자 답 대기)** 권장은 그대로다. 받으면 `Brick.`이 `CivHasTrait`를 지나 업적 행의 조건에도 들 수 있어 K4가 데이터의 우연에 기댄다 — 그래서 `DT_Feats`의 스키마 규칙 하나를 함께 넣는다(§3.3 규칙 7 — 결정을 기다리지 않고 적었다; 스키마의 고침은 노트 §5.4 데3). 같은 블록이 #88과 #36 ②도 연다 — R11·R13으로 따로 적었다(§0). 스키마의 뿌리 줄은 main에서 옮았다(`cef8cc7`의 `:272`·`:1196`·`:1237-1238` → `10205b6`의 `:291`·`:1245`·`:1286-1287` — I10).
3. **[개발자] 잎 이름의 거름** — 권장: 머지 전에 여섯을 묘사 이름으로 고친다(이름은 세이브에 남는다): `UnseenLaw` → `KeptLaw`, `SilenceFeed` → `QuietFeed`, `PlanckMicro` → `FoldedSmall`, `DysonBrain` → `ShutSphere`, `KesslerTapestry` → `WovenSwarm`, `AbyssalMaw` → `LastHunger`. 까닭: 실존 인물의 이름·교리의 낱말·`Realm.Abyssal`과의 겹침. 지금 코드는 요청의 이름 그대로 썼다. 대안: 그대로 — `project_dyson`의 선례. *왜 개발자인가*: 요청이 쓴 이름이다.
    - **(리뷰 반영 — 개발자 답 대기)** 리뷰가 권장을 바꿨다(T2·F10):
      - `SilenceFeed` → **`IdleFeed`**(`QuietFeed`가 아니다 — 'Quiet'은 main의 참된 계시의 낱말이다: `Config/DefaultGameplayTags.ini:201` `Revelation.Quiet`, 'Vocabulary kept apart from Doctrine.Silence'). 노브 `BrickQuietInsightPerTick` → `BrickIdleInsightPerTick`, 함수 `IsQuietOf` → `IsIdleOf`도 함께 바꾼다.
      - `KesslerTapestry` → **`DustBand`**(설계서 C05 '티끌 띠'), `AbyssalMaw` → **`InwardHunger`**(S11 '안으로 돌아선 식욕'). `WovenSwarm`과 `LastHunger`는 이미 `SwarmTech`·`StarEater`의 꼴 이름이다(branch `Sim/CosmicExistenceBricks.h:204-207`) — 같은 이름을 쓰면 세이브에 남는 한 이름이 두 사실을 가리킨다.
      - **더한다**: `StarShepherd` → **`StarTow`**(카드 그림 '도읍의 별에 끄는 줄', 설계서 C06). '목자'는 §3.5가 금한 낱말이고, '선한 목자'는 main `Tools/data/schema.json:51`의 revelation_mimicry에 있다.
      - **선택**: `HermitMonolith` → `ClosedPort`(까닭은 'Monolith' — 다른 작품의 상징물 — 하나뿐이다; `ShutHouse`로 하면 꼴 키와 겹친다).
      - 그대로 둔다: `KeptLaw`·`FoldedSmall`·`ShutSphere`(태그와 꼴이 같은 것을 가리켜 해가 없다).
      - 결정 3에 한 문장을 더한다: '변이의 잎은 꼴의 이름을 받지 않는다 — 꼴 낱말은 변이 없이도 부모를 지닌 모든 백성에 보인다'.
      - **귀결**: 이름은 세이브에 남는다(§2.11) — **`[B-7x1]`이 main에 들기 전에만** 고칠 수 있다. 함께 고칠 곳: ini의 'leaf rename to QuietFeed proposed'(branch `:176`)와 `:184`, 스키마 규칙 4의 나열, 테스트 메시지, 이 헤더 §2.4·§2.6·§3.4, 노트 §1, 설계서 `:111`·`:397`·`:400`·`:417`·`:462`(코드·데이터는 노트 §5.4 코7).
4. **[개발자] 시간 셈의 주인은 백성** (K10) — 권장: `FCivilizationState`에만. 까닭: 부름은 직분이고 몸을 건너는 것은 없다는 B5의 판정(계승에 가중을 두지 않은 까닭과 같다). 대안: ㉮ `FCallingState`에도(요청의 글) — 생애를 건너는 공로의 장부가 된다. *왜 개발자인가*: 요청과 다르다.
    - **(리뷰 반영 — 개발자 답 대기)** 권장은 그대로다. 다만 결정 13의 표시가 이 결정을 화면에서 무를 수 있다 — 표시할 때 백성의 지금 셈을 앞 생애의 장에 붙이면 '몸을 건너는 장부'가 화면에 다시 선다(T3 ①·DET-5). 13의 답이 4를 지켜야 한다.
5. **시간 셈의 술어 여섯**(권장): §2.2. 누적이고 되돌리지 않는다(㉡ 새 꼴 — 굽는 문장이 없어 R에서 뺐다, §0의 R2). 대안: ㉮ 있는 기록에서 파생만 — 지나간 시간을 되살릴 수 없다 ㉯ '삼킨 시간'은 공성만 센다 — 물려받은 전시 동원을 쥔 것만으로는 쌓이지 않게(지금 술어로는 다스리는 백성이 첫 틱부터 쌓고 AI는 칙령을 거두어 공성 때만 쌓는다; 묶음이 분포를 잰 뒤 가른다).
6. **[개발자] `HostCivID`를 지금 예약** (R3) — 권장: `[B-7x1]`에 함께 넣는다(해시를 한 번만 옮긴다), 이름은 `HostCivID`(`OwnerCivID`의 결). 한 성계 한 주인은 그대로다. 대안: ㉮ 슬라이스 다에서(해시가 또 옮는다) ㉯ 요청의 이름 `HostBeingID`. *왜 개발자인가*: 읽는 곳이 아직 없는 저장 필드다.
    - **(리뷰 반영 — 개발자 답 대기)** 권장은 그대로다. 리뷰가 날카롭게 한 셋(DET-9·F8):
      - (ㄱ) 품는 백성이 건넘 단계에서 지워질 때 모든 성계의 `HostCivID`를 비우는 한 문장: '건넘 단계가 지운 백성의 이름은 같은 걸음에서 모든 성계의 `HostCivID`에서도 비운다(추첨 없음 — `PendingContactCivID`를 비우는 branch `Sim/GalaxySimulationSubsystem.cpp:3131-3134`의 꼴); 품는 쪽만 남고 제 성계가 없는 백성이 건넘에서 지워지는지는 슬라이스 다의 명세가 정한다'. 시험은 슬라이스 다(노트 §5.4).
      - (ㄴ) 탄생한 백성도 `CivID`를 `NextCivSerial`에서 받는다 — 지운 이름은 다시 쓰이지 않는다(main `Sim/GalaxySimulationSubsystem.cpp:868-874`).
      - (ㄷ) 슬라이스 다의 '탄생 요청'이 틱 경계를 건너면 저장 필드가 하나 더 든다 — 그 필드를 `[B-7x1]`에 함께 예약할지, 해시가 또 옮는다고 적을지 정한다.
      - **귀결**: 승인 때 뜻과 초기값(`NAME_None`)이 언다. 읽는 규칙은 슬라이스 다에서 온다.
7. **블록의 몫은 노브로, 표는 B7에서**(권장): §2.4. 대안: 지금 `DT_Traits`를 짓는다 — 표·적재·에셋·스키마가 한꺼번에 들고 B7의 '특성 행'과 겹친다.
8. **[개발자] 별 등급의 어두워짐** (R10) — 권장: 장치는 넣되 주기 0으로 꺼 둔다; 켤지는 실제 하늘(B8)의 헤더가 정한다(관측된 별을 어둡게 해도 되는가). 일곱 등급 안에서만, 파괴 없음. 대안: 넣지 않는다 — 수입 곱만으로. *왜 개발자인가*: 대도감 판정(성식종은 위기의 얼굴)과 실제 하늘에 닿는다.
    - **(리뷰 반영 — 개발자 답 대기)** 권장은 그대로다. 다만 꺼 두어도 장치 커밋에 단언은 든다 — `DimCapitalStar`의 거절 갈래 셋(남의 수도·정착 중·인덱스 없음)이 시험되지 않아 가드를 빼는 변이가 시험 모음 전체를 통과한다(build-4). 받으면 `[B-7x2]`에 단언 셋과 변이 하나가 든다(노트 §5.4 시6). 죽은 노드와 성역 갈래는 결과를 바꾸지 않아 시험할 수 없다. R10의 문장은 §0의 표.
9. **조건 셋**(권장): §2.3. 대안: 벡터마다 열거자 여섯 — 어휘가 붓는다. `BorderingCivHasTrait`와 기생 블록의 이웃 덤은 '국경의 장치'다 — 국경 배율을 두지 않기로 한 판정(#88)을 이 둘만큼 좁게 연다(R11 — §0의 표).
    - **(리뷰 반영 — 개발자 답 대기 — [개발자]로 올릴지)** 이 결정은 개발자가 승인한 #88의 기본값 'VS에 두지 않는다'(main `OPEN_QUESTIONS.md:63`)를 조건 하나·기생의 두 국경 항·표류 목표의 블록 항만큼 연다. 리뷰는 R11로 적기를 권했고 그렇게 적었다(§0). 이 결정을 [개발자]로 올릴지는 개발자가 정한다(F4).
10. **갈림 카드는 `Lottery`와 `CivLacksTrait`**(권장): 상시 카드는 칸이 차면 사라지고 즉위마다 다시 온다. 대안: 대기 버퍼 — B4가 거절했다.
11. **[개발자] 카드와 수량** (R8) — 권장: 수직 슬라이스의 줄에 청원 열하나와 조우 하나를 더한다(갈림 열둘 — '조우 26 + 청원 24'). 요청 3의 반응형 카드 열넷은 B7의 AI 백성 행이 블록을 지닌 뒤에(그 전에는 만날 상대가 없다). 되돌림 카드(설계서 §4.3의 C09)만 둘째 칸에서 오는 것도 이 답에 든다. 대안: 38 안에서 다른 카드를 뺀다. *왜 개발자인가*: 수량은 B의 종료 조건이다.
    - **(리뷰 반영 — 개발자 답 대기)** 리뷰가 날카롭게 한 것:
      - **둘째 갈림 여섯 장의 ②가 닫히지 않는다**(F7) — 값을 치르고 이득(안정도·인구·윤리·경외)을 받는데 닫는 표지가 없어 지난 세 답이 지나면 다시 오고, C10 ②는 높아짐 +1을 되풀이해 쌓는다(설계서 C01의 '되풀이되는 맞바꿈이 되지 않게'와 branch `Tools/data/schema.json:1273`의 why와 어긋난다). (가) ②에 `SetFlag Flag.Cross.<행>`을 주고 행 조건에 `FlagNotSet`을 둔다 — `FGalaxyState.Flags`는 판 전체라(branch `Core/GalaxyState.h:981`) 계승한 다른 백성과 다른 생애에도 그 카드가 닫히고, ini 태그는 17 → 23이 된다. (나) ②에서 되풀이 이득을 뺀다 — 적어도 `petition_cross_law_stays` ②의 `PlayerStatDelta Exaltation +1`. **리뷰 권장은 (나)**다 — 새 태그가 없고 판 전체를 닫는 문제가 없으며 결정 없이 갈 수 있어 §3.1·§3.3·§9에 (나)로 적었다(데이터는 노트 §5.4 데1). (가)를 고르면 §2.11의 '열일곱'을 23으로 고친다.
      - 측정 정책은 갈림 카드에서 ③만 고른다(결정 19를 고쳤다) — 닫히지 않는 ②를 두 답마다 되풀이해 고르는 일은 사람은 하지 않는다(M7), 첫 갈림 넷은 건너뜀 아래에서 다시 와 결정 수를 부풀린다(M4).
      - `enc_cross_drift_fleet`은 위기 조건이 없는 첫 방랑자 상시 카드다 — `[B-6b8]`의 `enc_witness` 전제('늘 뽑힌다')를 깨고 도착 카드를 밀어낸다(I3 → §5 판단 ③′).
      - C10의 제목 '비워 둔 왕좌'와 ① 결과 줄 '왕좌는 비어 있었다'는 돌아올 왕의 틀이고, C10 ②의 `ShiftEthics Spiritualism +0.05`는 통치자 높임을 영성으로 센다(T8 — 고침은 `[B-7x4]`, 노트 §5.4 데2).
      - **귀결**: (가)를 고르면 판 전체 Flag의 범위를 받는다. (나)를 고르면 ②가 값만 남는 선택이 된다. 수량 줄(R8)은 'B6b 끝의 수 + 청원 11·조우 1'로 적는다(§0).
12. **AI의 변이 걸음**(권장): §2.6 — 다스리는 백성은 카드로만. 대안: 사건 행으로 — 누적 셈에는 한 슬롯 창이 맞지 않는다.
13. **[개발자] 끝의 꼴은 사실 줄의 낱말 하나** — 권장: §4. 요청 3의 '두 책 마지막 장의 결과 비문 열둘'은 이 꼴로 받는다. 까닭: 엔딩에는 화면 제목이 없고, 두 책은 기록과 주장만 받으며, '비문'은 죽음의 쪽의 이름이고, 사람에게는 칭호를 붙이지 않는다. 대안: ㉮ 판의 끝 쪽 발치에 한 줄(약속 '까닭 줄만 다르다'를 고친다) ㉯ 넣지 않는다. *왜 개발자인가*: 요청의 꼴과 다르다.
    - **(리뷰 반영 — 개발자 답 대기)** 이 문장에는 네 가지가 빠졌다(F9·DET-5·T3·I12): ① 한 생애가 여러 백성을 다스렸을 때('A → B' — main `UI/CosmicChronicleWidget.cpp:274-283`) 어느 백성인가; ② 지워진 백성(`State.Civs`에 없음)에는 `ShapeOf`를 부를 수 없다(`CivNameOf`는 이미 이름을 비운다 — main `:60-65`); ③ 셈은 통치가 끝난 뒤에도 섭정과 AI 아래에서 쌓여 앞 생애의 장에 뒤 생애의 선택(예: C10의 남긴 법)이 찍히고 같은 백성을 다스린 생애마다 같은 낱말이 붙는다(K10과 결정 4가 막으려던 것); ④ 판의 끝 뒤에도 시뮬은 돌고 연대기를 다시 열 수 있어(main `World/CosmicGameMode.cpp:1345-1355`) 같은 장의 낱말이 바뀔 수 있다. 고를 꼴 셋(표지는 리뷰가 붙인 것이다 — §0의 처리 ㉠·㉡·㉢과 뜻이 다르다: ㉢ = 저장 없이 표시만 좁힌다, ㉠ = 통치가 끝날 때 저장한다, ㉣ = 백성이 주어인 자리에만 둔다; 처음 대안 ㉮·㉯는 그대로 둔다):
      - **㉢(리뷰 권장 — 저장 없음)**: (a) 그 생애의 마지막 재위 백성(`Life.RuledCivIDs`의 끝)만 쓰고 다스린 적 없는 생애에는 붙이지 않는다; (b) 그 백성이 `State.Civs`에 있을 때만 붙인다; (c) 같은 백성을 다스린 생애가 여럿이면 마지막 생애의 장에만 붙인다; (d) 낱말은 '그 백성의 지금 살림'으로 읽히게 쓰고 판의 끝 뒤 다시 열면 달라질 수 있음을 받는다. 새 R이 없고 '끝마다 다른 것은 까닭 줄뿐'이라는 약속(main `UI/CosmicChronicleWidget.h:52-53`)도 지킨다.
      - **㉠(정확한 기록)**: 통치가 끝날 때(폐위·양위·죽음과 판의 끝) 꼴을 `FLifeRecord`의 끝에 더한 `TArray<uint8> ReignShapes`(`RuledCivIDs`와 같은 길이, 초기값 빈 배열)에 적는다 — 새 R과 헤더 먼저가 들고, `[B-7x1]`에 함께 넣으면 해시는 한 번만 옮는다.
      - **㉣(T3)**: 사실 줄에는 두지 않고 백성이 주어인 자리(성계 패널의 백성 줄 — §4)에만 둔다. 칙령 창은 판의 끝 뒤에 열리지 않아(main `UI/CosmicEdictWidget.h:81`) 끝 쪽에서는 꼴이 사라진다; 끝 쪽에 두려면 '<백성>의 살림: …' 줄을 두고 '까닭 줄만 다르다'는 약속을 고친다.
      - **함께 고칠 것**: 무엇을 고르든 '어느 백성 / 없는 백성 / 여러 생애'의 세 규칙을 §4 첫 항과 설계서 §4.4에 적고, 손 확인 5를 답에 맞게 고치며, `[B-7x3]`의 `UI.Chronicle`에 지워진 백성·다스린 적 없는 생애·두 백성을 다스린 생애의 경우를 더한다(노트 §5.4 시9). 설계서 §4.4 표의 '§4.2 원안의 줄' 열은 지운다(T3·T5 — 결정 21의 동결과 함께). 끝의 '추가 이야기'의 '그 판은 다른 질서의 끝으로 적힐 수 있다'와 꼴 낱말 `Endured`의 공로 읽힘(T7)은 어느 답에도 드는 고침이라 이 판에서 고쳤다.
14. **[개발자] 레인의 단일 창구** (R4) — 권장: 슬라이스 나에서 받는다(§2.8). 까닭: 파괴가 이미 레인을 끊고 있고, 웜홀 게이트가 레인 더하기를 설계에 두었다. 대안: Phase 6+로 — 견인·접힘이 오래 글로만 남는다. *왜 개발자인가*: 규칙 13의 창구와 B8의 그래프에 닿는다.
    - **(리뷰 반영 — 개발자 답 대기)** 권장은 그대로다. 지금 문안의 모순 하나 — '끊어서 어느 성계도 고립시키지 않는다'와 '파괴의 끊음도 이 창구로'(파괴는 죽은 노드의 레인을 모두 끊는다) — 는 결정 없이 §2.8에서 고쳤다: 고립 금지는 살아 있는 성계의 `Link`·`Unlink`에만 걸고, 파괴의 끊음으로 이웃이 고립되는 것은 받는다(F8 (i)). 슬라이스 나 헤더가 답할 물음(F8):
      - (iii) §2.10 표에 슬라이스 나의 걸음 행(단계와 같은 틱의 차례).
      - (iv) 노트와 설계서가 슬라이스 나에 둔 블록 — `VoidWeb`·`OmniLinked`의 `BeyondReach`(정착·공성의 새 사유)와 `WebReachLanes`(S10), `OrbitShaper`의 `Link`, `Labyrinth`(방랑자 이동의 거절 사유) — 을 §2.8에 적거나, 노트 §1의 그 자리를 B7이나 글로만으로 옮긴다.
      - (v) K9 — 레인이 바뀌면 `ClaimantOf`가 읽는 `ClaimAdjacencyLanes`(branch `Sim/CosmicHazards.h:138-142`)와 영향권이 바뀐다: 레인 걸음마다 `ClaimantOf`·영향권·주장 규칙을 같은 틱에 다시 읽는 차례와, 레인 걸음이 주장을 풀거나 늦추지 않는다는 시험을 슬라이스 헤더에 둔다. 모르는 성계로 잇는 `Link`가 `NoteKnown`을 쓰는지(규칙 2)도 정한다.
      - `CLAUDE.md` 규칙 13의 낱말은 슬라이스 나에서 넣는다(노트 §5.3의 7을 고쳤다 — F12).
      - **귀결**: 답한 뒤 슬라이스 나의 헤더가 이 물음들에 먼저 답한다.
15. **[개발자] 좌표의 이동** (R5) — 권장: 창구가 받은 뒤 둘째 걸음으로 — 먼저 레인만 옮기고(지도에는 길어진 레인으로 보인다), 표현이 따라오면 좌표를 옮긴다. 대안: 영영 옮기지 않는다. *왜 개발자인가*: 지도의 모든 그림이 좌표에서 나온다.
    - **(리뷰 반영 — 개발자 답 대기)** `SystemDragger`가 두 꼴로 적혀 있다(F8 (ii)): 헤더 §2.8 — 주기마다 딸린 성계의 레인 하나를 수도 쪽 이웃으로 옮긴다; 설계서 S14 — `project_settle`이 끝난 틱에 수도의 레인 하나를 새 터의 이웃으로 옮긴다. 리뷰는 어느 쪽도 강하게 권하지 않는다 — S14의 꼴은 걸음이 사건에 매여 주기 노브가 필요 없고 재기 쉽다. **귀결**: 고르지 않은 쪽의 문장을 같은 커밋에서 고친다.
16. **[개발자] 판 도중의 탄생** (R6) — 권장: 슬라이스 다에서 건넘 단계의 요청으로(§2.9). 대안: 6a까지 미룬다. *왜 개발자인가*: 문명 배열에 원소를 더하는 첫 코드다.
    - **(리뷰 반영 — 개발자 답 대기)** 리뷰가 더한 받는 조건(T4):
      - (1) 태어난 백성은 새 `FCivilizationState`다 — 새 `CivID`(`NextCivSerial`), 이름 뱅크의 새 이름(규칙 11), `TimeVectors` 0을 받는다. 부모의 `ArchetypeRow`·`Traits`(탄생 블록 하나만 빼고)·관계·칙령·`HostCivID`는 물려받지 않는다.
      - (2) 그 줄은 옛 백성이 살아남았다·죽지 않았다·돌아왔다·다시 태어났다고 말하지 않는다 — rebirth_words·afterlife_words·'불사'·'유령'을 스키마 규칙 하나로 막는다.
      - (3) 고를 것: (3a) `FossilAutomata`는 `LetPeopleGo`(절멸)에서 태어나지 않고 글이나 스캔 얼굴로만 둔다; (3b) 거기서 태어나도 새 백성이고, 그 틱에 `LetPeopleGo`가 놓은 성계를 받지 않는다.
      - (4) 잎 `Undying*`은 선언하지 않는다.
      - 슬라이스 다 헤더가 물을 것(F8 (vi)): 요청이 어디서 생기는가(틱을 건너면 저장 필드 — 결정 6 (ㄷ)), `CivID`·이름·아키타입 행·관계·`TimeVectors` 0(K10), `HostCivID` 정리(결정 6 (ㄱ)), 품는 쪽 수입의 몫(R14), `MirrorProjection`과 S13의 세 문(일방 `TreatyBlockOf` `HostWorld`, 품긴 쪽의 정착 문, `NonAggression`). R6은 코드의 사실 하나만 굽는다(§0).
      - **귀결**: 조건 없이 승인하면 원안의 불사 분열과 유령 제국(설계서 `:47`·`:88`·`:121`)이 열린 채다. B6b가 지은 '마지막 성계의 문 → `LetPeopleGo`'(main B6b 헤더 `:106`·`:422`) 뒤에 같은 백성이 기계로 이어 사는 길도 열린다.
17. **레인 시간의 수정**(권장, R7): `LaneTicksOf`가 맥락을 받는 것은 B8 헤더의 '거리에 따른 레인 시간'과 한 번에.
18. **[개발자] 받지 않는 다섯** — 권장: §1.3 그대로. 앎을 읽는 붕괴는 세계의 사실로 발동하고 표현에서만 가린다; 거꾸로 흐르는 시간은 받지 않는다; 변이는 칸을 주지 않는다; 면역은 감쇠와 문의 사유로; '되는' 끝은 '남긴' 끝으로. *왜 개발자인가*: 설계서의 머릿기능들이다.
    - **(리뷰 반영 — 개발자 답 대기)** 리뷰가 '남긴 법'(`UnseenLaw`)의 귀결 셋을 적었다(F1·I2·M11 — §2.4와 설계서 C10은 첫째만 적었다):
      1. **안정도**(적힌 것): C10 ①로 양위한 옛 백성은 섭정·AI 아래에서 물려받은 `edict_mobilization`을 판 끝까지 지닌다 — 안정도를 깎고, 유지비가 0이라 만료하지 않아 반란과 지워짐 쪽으로 간다. 좁혀도 남는다 — 이것이 설계가 노린 값이다.
      2. **주장(K9)**: 같은 백성이 `edict_vigil`(`Hazard.Claim` +100, 유지비 0)과 `edict_machine_rights`(AI 각성의 `RulerResolutions` — 만료를 해결로 바꾸어 주장 규칙을 건너뛴다)도 판 끝까지 지닌다(branch `Tools/data/DT_Edicts.json:96`, `DT_Hazards.json:46`, `Sim/GalaxySimulationSubsystem.cpp:2542-2551`). W의 수는 그대로지만(철야 100은 이미 W 안 — main B6b 헤더 `:221`) '영원한 늦춤'과 '해결로 바뀐 만료'는 K9가 바로 금하는 것이다 — B6b 결정 9가 은하 법칙에서 이 고리를 금했다.
      3. **법칙 셋(다섯째 칸)**: 칙령 자리는 많아야 셋이고(main `Sim/CosmicEdicts.cpp:26-35`) 법칙 셋은 모든 자리를 쓴다(main B6b 헤더 `:143`). 백성 범위 칙령이 하나라도 남으면 AI 법칙 걸음은 많아야 둘을 세우고 `LawsHeldTicks`가 0에 머물러, 법을 묻는 넷째 → 다섯째 행이 그 백성에게 닫힌다(유지비 있는 칙령은 만료하면 풀리지만 mobilization·free_trade·vigil·quarantine·against_silence는 만료하지 않는다).
      - **리뷰 권장 — 좁힘**(상시 승인 안; 코드는 노트 §5.4 코1): 남긴 법 백성이라도 (a) 주장 카운트다운에 더하거나 위기의 답인 칙령은 여느 AI처럼 거두고(B6b 결정 9와 같은 선), (b) 그 백성의 행이 법칙을 물으면(`CosmicEdicts::AsksForLawsOf`, main `:242`) 백성 범위 칙령을 여느 AI처럼 거둔다(B6b 결정 10·11 — 칙령 자리 셋이 법칙 셋의 자리다). 그러면 귀결 2·3은 사라지고 1만 남는다. 답이 오면 §2.4 행 끝에 넣을 문장: '— 다만 주장의 카운트다운에 더하는 칙령과 위기의 답인 칙령은 왕좌가 빈 뒤 여느 때처럼 거두고(K9 — B6b 결정 9가 은하 법칙에 그 고리를 금지한 선과 같다), 판정받는 행이 법칙을 물으면(넷째 칸 — `CosmicEdicts::AsksForLawsOf`) 백성 범위 칙령은 다른 AI처럼 거둔다(칙령 자리 셋이 법칙 셋의 자리다, B6b 결정 10·11)'. 같은 줄을 설계서 C10의 귀결과 노트 §1.1의 `UnseenLaw` 행에 넣고, R12(§0)의 새 문장도 이 좁힘으로 닫는다.
      - **대안**: '모든 칙령을 지닌다'를 고르면 K9를 굽히는 새 R을 적고 W를 다시 세며, `Data.Integrity`의 신정론 불변식(main `Tests/CosmicDataTests.cpp:2477-2568`의 `TheodicyProblems` — 리뷰 보고서가 적은 `:2218-2293`은 그 함수가 아니다)에 그 경우를 더하고, 다섯째 칸이 그 백성에게 닫힌다는 것도 적는다.
      - **함께 받을 것**: ①(관측 전에는 어디에나) — 노트 §1.1의 `QuantumCloud` 행을 '세계는 늘 확정 — 정착된 성계가 레인으로 닿거나 그릇이 서면 드러난다; '어디에나'는 받지 않는다 — 표현도 무주 성계 위에 그 백성을 그리지 않는다; 남는 것은 S15의 소문 한 줄이고 그 줄은 '제 별 곁에만'으로 닫는다'로, 마지막 열은 '받지 않음'으로('(㉡은 뒤 블록)'을 지운다) 고치고, 이 결정에 ''어디에나'는 표현으로도 받지 않는다(S15의 소문 한 줄뿐)'를 더한다(T10). 간접 경로 — 기생 덤이 이웃의 각성·역병·반란 만료와 그 뒤 주장 규칙을 늦추고, 미시 곱이 각성을 줄여 그 뒤 주장도 준다 — 는 §2.4에 적었다(F2).
      - **귀결**: 권장대로 답하면, 양위한 백성은 안정도를 깎는 칙령을 지닌 채 섭정 아래에서 무너질 수 있다 — 이것이 받는 값이고, 주장과 법칙의 문은 여느 백성과 같다. `-Crossroads` 칸(§5 ⑧)이 안정도 0.2 아래 첫 틱·반란·지워짐·`LawsHeldTicks`를 수로 보인다 — **수를 보고 답하려면 이 칸(`[B-7x4m]` 뒤)이 답보다 먼저다.**
19. **측정 정책의 건너뜀**(권장): §5 — 갈림 카드에서는 값 없는 선택(③)만 고른다(블록 없는 값 있는 선택도 건너뛴다 — 리뷰 M7·M4로 고쳤다). 대안: 블록을 주거나 걷는 선택만 건너뛴다(처음 글) — 닫히지 않는 ②를 두 답마다 되풀이해 고른다.
20. **[개발자] 빌드 전 코드를 받는 길** — 권장: B6b가 끝난 `main` 위로 리베이스하고 `[B-7x1]`부터 커밋마다 빌드·테스트·기준값의 차례로 다듬는다(충돌과 해소는 §8.1 — 리뷰가 바로잡았다). 대안: 헤더만 받고 코드는 다시 쓴다. *왜 개발자인가*: '헤더 먼저 → 승인 → 소스'의 차례를 이 브랜치가 앞질렀다.
    - **(리뷰 반영 — 개발자 답 대기)** 권장 길(B6b 뒤 리베이스)은 그대로다. 바뀐 것은 충돌 문구·해소 규칙·커밋 차례다(I1·I11·M8·build-1·I6·F12·T1):
      - 충돌 목록은 양쪽으로 틀렸다 — 이름한 `FCosmicRulesDef`·`EConditionType`·`ECosmicSettleBlock`·`CosmicSeed`·ini의 끝은 충돌하지 않고, 실제 충돌은 §8.1이다. 'main 쪽을 앞에 두고 양쪽을 다 살린다'를 `words`에 그대로 하면 깨진다 — 해소는 main 줄을 그대로 두고 `time_vectors` 한 줄만 `words`의 끝에 더하는 것이다.
      - 차례: 브랜치의 차례를 버리고 §8의 차례로 다시 쌓는다 — `[B-7x1]`과 `[B-7x2]`는 하나로 합치고, 데이터는 `[B-7x4m]`·`[B-7x4]` 뒤에 켠다.
      - `[B-7x5a]`(aa09543)는 제 커밋으로 남기지 않고 `[B-7x5s]`·`[B-7x5c]`로 가른다(M8·build-1) — 그대로 켜면 시험 보드가 게임 값을 물려받아 B7x 자신의 테스트 셋과 있던 테스트 하나가 깨진다.
      - 변이 패치 일곱이 멈춘다(I6) — 다섯은 다시 뜨고, 둘은 `time_vectors`를 `words` 끝에 두면 산다(노트 §5.4 시12).
      - 브랜치 최상위의 두 파일 — `promptforother.txt`(F12)와 `docs/OVERVIEW_as_of_B6a.html`(T1, 결정 21) — 을 main에 들이지 않는다.
      - 반박 하나: `UI/CosmicSystemPanelWidget.cpp`는 깨끗이 합쳐진다(build-3) — static_assert 12만 B6b가 정착 사유를 더했는지 보고 다시 읽는다.
      - **귀결**: 권장 길은 그대로이고 충돌 문구·해소 규칙·차례가 바뀐다 — 이 판의 §8은 이미 그 꼴로 적었다.
21. **설계서의 고침**(권장): 머리에 상태 표지, §3.3·§4.3·§4.4를 더하고 §5의 문구를 받은 꼴로 바꿨다. 통째로 다시 쓰는 것은 `[B-7x9]`.
    - **(리뷰 반영 — 개발자 답 대기 — 개발자의 글이라 확인이 든다)** 둘을 정한다(T5·T1):
      - **설계서의 원안**(§1~§3.2, §4.1~§4.2): 리뷰 권장은 `[B-7x0]`에서 글자 그대로 `docs/archive/EXISTENCE_EVOLUTION_original_2026-11-09.md`로 옮겨 얼리고(고치지 않는 동결본 — `guideline_v0.8`의 꼴), `docs/10_DESIGN`에는 받은 꼴(§3.3, §4.3, '원안의 줄' 열을 뺀 §4.4, §5)과 동결본으로 가는 링크 한 줄만 남기는 것이다. 까닭: 설계의 단일 기준인 `docs/10_DESIGN`에 원안의 '되는' 끝·사람의 수·금지된 낱말이 그대로 있고, 지키는 것은 머리의 표지 하나다(그 표지가 이 파일에서 '표지 없는 문장은 현행 규칙'을 덮으므로 규칙 위반은 아니다). `CLAUDE.md`에는 줄을 더하지 않는다(16KB 예산 — §7). 대안: 지금처럼 표지만 두고 `[B-7x9]`에서 다시 쓴다.
      - **`docs/OVERVIEW_as_of_B6a.html`**(branch `:670-726`의 B7x 절 — '항성 유목신'·'살아있는 방주-세계'·'즉시 현현'·'언약 자체가 되어'·'언약의 시간'·'불멸'·'헌정'): ㉮ 브랜치에서 뺀다, 또는 ㉯ 받은 꼴로만 다시 쓰고 설계서 머리와 같은 상태 표지를 둔다 — 어느 쪽이든 받지 않은 원안을 표지 없이 main으로 들고 가지 않는다(K6·규칙 11). ㉯면 `[B-7x9]`의 재감사 목록에 든다(§7).
      - **귀결**: 답이 오기 전에는 `[B-7x0]`을 커밋하지 않는다 — 그대로 두면 두 파일이 main으로 든다.
22. **[개발자] 새 추첨 지점** (R9) — 권장: 가·나는 0, 다의 포자에 고정 수 하나(Phase 5 전에만 더할 수 있다). *왜 개발자인가*: 추첨 장부에 줄이 는다.
    - **(리뷰 반영 — 개발자 답 대기)** 권장은 그대로다. 다만 결정 16의 조건 (1)·(2)를 먼저 적은 뒤에 정한다(T4) — 포자가 '죽으며 흩어져 다시 자라는 같은 백성'이 되지 않게 하는 조건이 추첨 장부의 그 줄의 전제다. 씨앗 블록은 추첨이 아니라 파생 정수라 이 결정 밖이다(R13과 §7의 문장만 — DET-1).
23. **AI 백성의 씨앗 블록**(권장): §2.4 — 생성 때 파생 정수로 블록 하나, 노브가 비면 꺼짐. 까닭: B7의 아키타입 행 전에도 판마다 이웃이 달라지고, 두 백성의 겹침 규칙이 측정에 잡힌다. 대안: B7의 행을 기다린다 — 그때까지 겹침 규칙의 절반이 판에서 서지 않는다. 닿는 것: #36 ②(절차 특성 — R13), 파생 정수 용도 13(`docs/20` 규칙 3의 문장 — §7), B6b 경주의 입력과 AI의 셋째 칸 틱(§5 판단 ⑥이 다시 잰다 — 리뷰 F3·M1).
    - **(리뷰 반영 — 개발자 답 대기 — [개발자]로 올릴지)** 씨앗은 모든 AI 백성의 세계를 바꾸고(M1) #36 ② '`DT_Traits`와 함께'(main `OPEN_QUESTIONS.md:33`)를 앞당긴다. 리뷰는 R13으로 적기를 권했고 그렇게 적었다(§0). 이 결정을 [개발자]로 올릴지는 개발자가 정한다(F3·F4) — 추첨 쪽(DET-1)은 격상이 필요 없다고 본다(`docs/20` ③에 이미 생성 때 세계를 정하는 파생 정수 — 건너온 백성의 이름·아키타입 — 가 있다).

## 11. 구현 기록 (헤더와 다르게 하거나 더한 것 — 커밋마다 더한다)

이 브랜치의 커밋은 모두 **빌드 전**이다 — 엔진이 없는 PC에서 썼고 파이썬 검사 다섯만 통과했다(컴파일·자동화 테스트·에셋·기준값은 다른 PC에서). 이어진 자리와 확인 목록은 노트 §5.

- `[B-7x1]` — 필드 먼저: `FCosmicTimeVectors`와 `TimeVectors`, `HostCivID`, 태그 열일곱과 접근자, 스키마의 뿌리 목록, 테스트 둘. 헤더와 다른 것 없음. '문서 먼저'(`docs/20`·`docs/21`)는 하지 않았다 — B6b가 고치는 문서라 §7대로 다른 PC에서 넣는다.
- `[B-7x2]` — 장치: `Sim/CosmicExistenceBricks`, 조건 셋, 노브 열여섯, 정착 사유, 변이 걸음, 씨앗 블록, 끝의 꼴. 헤더에 더하거나 달리한 것: 기생 블록의 제외(주장·절멸·전조)는 코드가 지킨다(은하 법칙의 제외는 스키마 규칙뿐이었다); 0보다 커야 하는 곱은 스키마 규칙이 지키고 코드는 0 이하를 1로 읽는다; 별 등급의 차례가 `Core/CosmicTags.h`에 한 벌 더 생겼다; 외교 창의 툴팁(`UI/CosmicEdictWidget.cpp`)은 표류 목표의 새 항을 아직 보이지 않는다(`[B-7x3]`).
- `[B-7x2r]` — 리뷰의 고침(읽어서 한 리뷰 한 번 — 확실한 컴파일 오류는 찾지 못했고, 블록 없는 백성의 옛 길이 이어진 열 자리에서 글자 그대로임을 읽어 확인했다): 별 등급의 차례를 `Core/CosmicTags.h` 한 벌로 줄였다(생성·별의 에너지·등급 몫의 검사가 그것을 읽는다 — 같은 태그, 같은 차례); 끝의 꼴 `SpreadBody`가 `DarkMatter`에서도 선다; 열일곱째 노브 `BrickStellarCrisisSpawnMultiplier`와 그 읽는 자리(`HazardSpawnShareOf`가 위기의 태그를 받는다).
- `[B-7x5a]` — 데이터 초안(에셋·측정 전): 갈림 카드 열두 장과 사건 행 아홉이 `Tools/data`에 들었고, 노브 열일곱의 시작값과 씨앗 블록 여덟이 `DT_Rules.json`에, `Brick.Form.Society`가 `arch_seeker`에 들었다. 헤더와 달리한 것: 스키마 규칙 4·5는 다섯 규칙으로 갈라 조건으로 가려낸다(§3.3을 그 꼴로 고쳤다); 목소리가 청한 선택에 관례의 `ShiftEthics Spiritualism` ±0.05가 든다; 설계서가 주지 않은 결과 줄·실패 줄·사건 행의 변형과 수는 이 커밋이 초안으로 썼다(`[B-7x4]`의 감사 몫); `event_brick_ember_wake`는 주제가 백성이라 수도의 전조만 보고 되풀이 간격 300을 지닌다. **이 데이터는 측정 정책의 건너뜀과 끈 짝 스위치가 들어오기 전에는 켜진 채로 재지 않는다.**
- `[B-7x2c]` — 주석의 절 번호(빌드 전): 코드 주석·태그의 설명 글(`DevComment`)·스키마의 까닭 글과 그 시험이 부르던 작업 틀의 번호(E1~E6)를 이 헤더의 절 번호로 고쳤다(E1 → §2.1·§2.2, E2 → §2.3, E3 → §2.4·§2.5, E4 → §2.6, E5 → §2.7, E6 → §6). 주석과 글 쉰여덟 줄뿐이다 — 코드의 낱말은 하나도 바뀌지 않았다.
- **이 초안의 리뷰**: 리뷰어 하나가 헤더·노트·설계서의 새 절을 코드와 규칙에 견주어 한 번 읽고 지적 열아홉을 냈다 — 모두 고쳤다. 가장 큰 것은 `[B-7x2]`가 두 해시를 한 번 더 옮긴다는 것(§0·§8 — 처음에는 '비트 그대로'라고 적었다), 다음은 남긴 법이 안정도를 깎는 칙령도 지킨다는 귀결(§2.4, 설계서 §4.3 C10 — 결정 18에 답하기 전에 볼 것)이다. `WORKPLAN.md`의 ultracode 적대 리뷰는 그때 하지 않았다 — 2026-11-09에 돌렸다(아래).
- **아직 코드에 없는 것**: 표현(`[B-7x3]` — 꼴의 낱말을 사실 줄과 패널에, 스캔 줄의 얼굴, 외교 창 툴팁의 새 항), 측정 정책의 건너뜀과 끈 짝 스위치·새 줄 `bricks:`(§5 — '값보다 먼저 재는 것'은 이 줄을 먼저 넣은 빌드로 잰다), 스키마 규칙 §3.3의 6 가운데 씨앗 묶음의 거름(정착을 막는 둘), 별 등급의 어두워짐을 잇는 일(결정 8), 슬라이스 나·다.
- **ultracode 적대 리뷰와 결정 없는 글의 고침(2026-11-09 — 코드·데이터·테스트는 고치지 않았다)**: 렌즈 여섯이 발견 63을 냈다 — confirmed 32, partial 30, refuted 1(build-3). 렌즈별(발견·confirmed·partial·refuted): 설계·규칙 F 12·7·5·0, 결정론·저장·해시 DET 9·4·5·0, 신학·서사 T 10·4·6·0, 측정·밸런스 M 15·7·8·0, 빌드 build 5·1·3·1, 통합 I 12·9·3·0. 판정은 '이대로는 승인할 수 없고 조건부로 승인할 수 있다' — 설계를 다시 하라는 발견은 없고, 확실한 컴파일 오류도 없으며, 블록 없는 백성의 옛 길은 열두 자리 모두 글자 그대로였다. 승인 전에 막을 여섯은 남긴 법의 귀결(K9·법칙 자리), B6b 경주를 다시 재지 않는 측정, 게임 값을 물려받는 시험 보드, 잎 이름, 끝의 꼴의 백성과 때, 받지 않은 원안과 틀린 충돌 목록이다.
  - **이 판이 고친 것**(답을 기다리지 않는 글 — 고친 자리마다 발견 번호를 괄호에 적었다): §0의 R 표(R1~R14 — R2는 ㉡로), 낡은 사실(스키마 줄·테스트 수·열린 물음 번호·기준값 줄·`[B-7x5]`의 갈림), §2.4의 기생·미시 행과 무동작 문장, §2.6·§3.4의 문턱 문장, §2.8의 모순 하나, §3.1·§3.3의 둘째 갈림 ②((나))·K4 규칙 7·기생의 신정론, §3.2의 `Endured` 낱말과 `Shape.`·`Settle.Block.Inward`의 낱말 규칙, §3.5의 목록 이름과 양위 뒤의 줄, §5 다시 씀(끈 짝 두 단·깨질 수 있는 확인·정책 ③만·`bricks:`·문턱의 규칙·판단 ⓪~⑧·고침의 규칙), §6의 보드 규칙·뜻이 바뀌는 테스트·변이 견줌, §7의 머리·`docs/20`의 씨앗 문장·승인 문서, §8의 차례(`[B-7x4m]`·`[B-7x5s]`·`[B-7x5c]`·`[B-7x5r]`)와 충돌 목록(§8.1), §9의 세이브·①·3, 결정 5·9·19·23의 문장, 끝의 '추가 이야기'의 한 마디; 노트 §2·§3·§4·§5.2·§5.3; 설계서의 상태 표지·'잰다'의 자리·`Endured` 낱말·§5의 문구.
  - **기다리는 것**: [개발자] 1·2·3·4·6·8·11·13·14·15·16·18·20·22의 답, 결정 9·23을 [개발자]로 올릴지, 결정 21의 두 글(설계서 원안·OVERVIEW html) — §10의 '(리뷰 반영 — 개발자 답 대기)'에 리뷰의 권장·대안·귀결을 적었다. 리베이스 때 고칠 코드·데이터·테스트(리뷰 보고서 §3.2~§3.4)는 노트 §5.4에 대상 커밋마다, 리뷰의 기록(반박된 것·B6b와 닿는 곳)은 노트 §5.5.

## 추가 이야기와 보완

- **비전과의 거리.** 설계서가 그린 '별을 끌고 다니는 존재'는 슬라이스 가에서는 '빨리 넓히는 백성'일 뿐이다 — 글자 그대로의 견인은 슬라이스 나의 창구 뒤에 온다. '무형의 법'은 누가 법이 되는 이야기가 아니라, 왕좌가 빈 뒤에도 그 백성이 통치자가 세운 칙령을 하나도 거두지 않는다는 사실이다(은하의 법칙은 B6b에서 이미 그렇고, 이 블록이 백성 범위의 칙령까지 넓힌다 — 주장 고리와 법칙의 자리를 빼는 좁힘이 결정 18에 걸려 있다).
- **보이는가가 먼저다.** B4의 교훈대로 드문 조건의 내용은 보이지 않는다. §5의 판단 ①이 못 미치면 문턱을 내리는 것이 값을 고치는 첫 손이다.
- **대기 칸.** 통치자의 주기 카드는 지금 둘뿐이다 — 문턱을 넘은 갈림 카드가 주기 칸(300틱)을 차지하는 몫은 §5의 ③이 잰다. 물리친 갈림은 지난 세 답이 지나면 다시 온다(값 없는 선택은 효과를 지닐 수 없어 닫는 표지를 달지 못한다) — 무게는 1에서 시작한다. 둘째 갈림의 ②도 닫지 않으므로 되풀이해 쌓이는 이득을 두지 않는다(§3.1).
- **안으로 접는 값.** 정착을 막는 두 꼴은 '여러 별의 살림' 업적(정착 셋)의 길을 스스로 닫는다 — 셋째 칸은 다른 업적으로만 간다. 갈림의 값으로 두고, 그 꼴을 고른 판의 셋째 칸 도달을 묶음이 잰다. 하나뿐인 되돌림(설계서 §4.3의 C09 — 닫아 건 살림을 다시 연다)은 그래서 둘째 칸부터 온다 — 다른 둘째 갈림은 셋째 칸부터다.
- **삼키는 살림의 문.** 다스리는 백성이 `Brick.Diet.StarEater`를 얻는 길은 '삼킨 시간'의 첫 갈림의 둘째 선택 하나다; AI 백성은 씨앗 블록으로 얻는다.
- **본능 축.** 여섯은 B7의 `AIProfile`이 읽을 낱말로 넘겼다 — 그 헤더가 이 처분표(노트 §1)를 받아 쓰면 된다.
- **리뷰 뒤.** 승인을 막는 여섯은 모두 `[B-7x0]`의 글 고침과 개발자의 답(3·13·18, 그리고 20의 차례)으로 닫힌다 — 글 고침은 이 판에서 했고 답은 §10에 모았다. 수를 보고 답할 수 있는 것은 결정 18 하나다(`-Crossroads` 칸 — `[B-7x4m]` 뒤). 이름(결정 3)은 `[B-7x1]`이 main에 들기 전에만 고칠 수 있으니 먼저 답을 받는 편이 싸다.
