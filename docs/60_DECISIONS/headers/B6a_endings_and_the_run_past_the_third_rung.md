# B6a 헤더 — 끝의 장치와 셋째 칸 뒤의 판 (엔딩 기록·열째 단계 뒤의 한 번 쓰기·끝 뒤에도 도는 시뮬·셋째 칸 끝을 뗌·침묵과 그릇 없이 맞은 결말·끝난 MVP 세이브·알아챔·`petition_exalt`·주장의 경고와 반란 문턱·셋째 칸 뒤 계절의 재측정)

상태: **승인 (2026-10-31 — [개발자] 일곱 묶음(1·2·6·7·17-나·20·21)은 개발자가 모두 답했다: 1·2-가·2-나·2-다는 먼저, 남은 여섯은 "전부 권장대로"; 나머지 열넷과 17-가는 권장안이 곧 승인이다; 대안을 고른 항목이 없어 §2~§9의 본문은 그대로다).** 만든 길: 조사 워크플로(여섯 관점과 어긋난 곳을 가른 비평 하나 — 사실 F1~F18) → 개발자의 답 셋(범위의 나눔·B6a의 시계·끝난 MVP 세이브 — 2026-10-31) → 초안 → ultracode 적대 리뷰(관점 넷 — 발견 61, 검증: 확인 47·일부 13·반박 1) → 리드의 고침(확인·일부 60건을 검증자의 고침대로, 반박 1은 들이지 않음) → 고친 뒤 확인의 비평(열일곱 — 사실 다섯·알아챔의 문 하나·커밋 검증 넷·손 확인 하나·테스트 다섯·작은 보완 하나) → 둘째 확인(고침 77건이 든 자리와 인용 25줄을 HEAD에서 다시 열었다 — 남은 결함 여덟, 이 판; 기록은 모두 노트 §4) → 승인. 짝 노트 `B6a_endings_and_the_run_past_the_third_rung_notes.md` — 지금 코드의 줄마다 근거(노트 §0), 넘겨받은 것과 열린 물음의 표(§0.2·§0.3), 문서의 어긋남(§1), 측정의 셈(§2); 첫 측정과 구현 기록은 커밋마다 노트 §3·§5에 더한다. **B6b**(넷째·다섯째 칸의 업적 행과 AI, 두 엔딩이 실제로 닿는 블록)는 이 헤더가 끝난 뒤 따로 헤더를 쓴다(§0 'B6a 밖').

근거: `docs/50_ROADMAP.md:12`(B 행의 B6 칸 — 'T3→T5 업적·T5 문턱·엔딩 4'와 그 칸에 적힌 B3·B4·B5가 넘긴 것)와 그 행이 기대는 채택 문장 'B6 T3→T5 업적·엔딩 4'(`docs/archive/analysis_2026-10-18/Cosmic_Ascension_1단계_브리핑_2026-10-18.md:259` — 'T5 문턱'은 그 문장에 없다, 결정 1), `docs/00_VISION.md:18`·`:20`·`:32`, `STATUS.md:30`(다음 작업 2)·`:43`(2026-10-18 결정 '끝 = T3 ∨ 틱 6,000'), `WORKPLAN.md:61`(B 행)·`:65`(6a)·`:66`(6+ 콘텐츠), `docs/20_ARCHITECTURE.md:89`·`:195`·`:285`, `docs/21_DATA_MODEL_REFERENCE.md:1324`·`:1326`, `crisis_and_transcendents.md:99`·`:134`·`:145-151`, `OPEN_QUESTIONS`(노트 §0.2), 헤더 A2(`:148`·`:194`)·A4 노트(`:440-463` — 기록의 스케치)·B4(`:88-92`·`:336`)·B5(`:185-187`·결정 9·12-라·15·21·22 — `:390`·`:407`·`:417`·`:423`·`:424`), `CLAUDE.md` 불변 규칙 1~4·6·7·9·11~15. 조사 원문은 저장소 밖(세션의 스크래치패드)이라 근거로 인용하지 않는다 — 사실은 모두 HEAD `b10be48`의 줄로 단다. 경로: 코드는 `Source/Cosmic_Ascension/` 아래를 줄여 `Sim/…`로, 문서는 `docs/20:89`처럼, 설계 본문은 파일 이름으로, 앞 헤더는 'B5 헤더 `:417`'로 적는다.

## 0. 범위와 전제

- **이 헤더의 낱말**(새 낱말을 만들지 않고 설계의 낱말을 풀어 둔다):
  - **판의 끝**: 시간이 멈추고 연대기 쪽이 열리는 일. 지금은 MVP의 **임시 끝** — 다스리는 백성이 셋째 칸에 오르거나(어느 생애든 `PeakTier` ≥ T3) 틱 6,000(`Sim/CosmicEndings.h:23-30`, `Tools/data/DT_Rules.json:55`). **셋째 칸 끝**은 그 앞쪽, **시계**는 뒤쪽(노브 `ProvisionalEndTick`)이다.
  - **엔딩**: 설계의 네 결말 — 집단 승천(다스리는 백성이 다섯째 칸), 단독 승천(그릇의 개인 칸이 다섯째 칸), 다른 질서(다스려지지 않는 백성이 먼저 다섯째 칸), 침묵의 우주(그릇이 설 곳이 없음 — `crisis_and_transcendents.md:145-148`). 이 이름들은 설계 문서의 이름이고 화면 글이 아니다(결정 6-나).
  - **엔딩 기록**: 판이 어떻게 끝났는지를 적는 저장 필드 하나(`FCosmicEndingRecord` — §2.1). 시계의 끝도 이 기록에 적는다. **빈 기록 = 판이 계속된다.**
  - **끝 뒤**: 기록이 적힌 다음의 틱들. 게임은 그 틱을 돌리지 않지만(컨트롤러가 시간을 멈춘다) 시뮬은 거절하지 않는다 — 커맨드렛과 테스트의 긴 판은 끝 뒤로 돈다(§2.5).
  - **셋째 칸 뒤의 계절**: 다스리는 백성이 셋째 칸에 오른 뒤부터 판의 끝까지의 틱. 지금은 0이다(셋째 칸이 끝이라). B6a부터 시계까지다(스크립트 통치자로 약 3,600틱 — §1.2).
  - **그릇 없이 맞은 결말**: 공백기(그릇이 죽고 다음 그릇이 서기 전) 안에 판이 끝나는 일. B5 결정 12-라가 옛 이름 '부름이 지켜본 결말'을 이렇게 바꿨다(B5 헤더 `:407`). 기록의 칸 `bNoVessel`이 적는다.
  - **침묵**: 계승할 후보가 없어 공백기에 머문 상태(B5 — `Sim/CosmicSuccession.h:28-38`). 설계의 '침묵의 우주'가 이것이다(결정 6-다). 화면에는 '침묵'을 내지 않는다(#150).
  - **알아챔**: 타락 초월자가 플레이어의 부르심을 알아채는 일(`FTranscendentState.bAwareOfPlayer` — `Core/GalaxyState.h:465-467`)과 그때 타락이 처음 플레이어에게 2인칭으로 하는 1인칭 줄(`docs/70:168` 끝 — 'B6의 알아챔 줄이 2인칭을 쓴다').
  - **첫 측정**·**짝**·**한 번만 센다**: B5의 낱말 그대로(B5 헤더 `:15`·§5.2) — 데이터를 켜기 전에 커맨드렛의 새 칸으로 재고, 같은 시드의 두 판을 견주고, 같은 세계의 사건은 한 번만 센다.
- **B6a에 드는 것**(하나의 목록 — `docs/50:12`의 B6 칸이 이것과 B6b의 목록을 옮겨 적고, `STATUS.md`는 이 절을 가리킨다):
  1. **엔딩 기록**: 저장 필드 하나(`FCosmicEndingRecord` — 종류·틱·백성·그릇 없음)와 종류 태그 다섯. 필드만의 커밋. 결정 3·6-가.
  2. **끝의 장치**: 엔딩 술어의 한 집(`Sim/CosmicEndings`), 열째 단계 뒤 한 번 쓰기(추첨 없음), 같은 틱의 차례, 끝 뒤에도 도는 시뮬, 기록이 적힌 틱에서 서는 `RunLeap`, 기록을 읽는 래치·저장 층·두 정책·커맨드렛·연대기·상단 바. 결정 4·5.
  3. **셋째 칸 끝을 뗀다**(`CosmicEndings::EndRung`을 지운다) — 판은 시계 6,000까지 간다. 장치와 가른 커밋이다(`[B-6a3a]` — 결정 15). 결정 2-가·2-나.
  4. **침묵이 엔딩이 되고**, 공백기 안의 끝이 **그릇 없이 맞은 결말**로 적힌다(#117·#150). 결정 8.
  5. **끝난 MVP 세이브는 흐르는 판으로 연다**(`SaveVersion` 1 그대로). 결정 2-다.
  6. **알아챔**(#140·#136): 플래그를 처음 쓰는 코드(초월자마다 한 번), 그 줄(판에 한 번 — §2.11), `VoiceTemplates`를 두 칸으로 가른 것 가운데 알아챔의 칸. 결정 7.
  7. **`petition_exalt`를 바로잡는다**(A2가 뜻한 둘째 칸의 틱에 오게)와 그 결함을 막는 스키마 규칙 하나. 결정 9.
  8. **표현**: 연대기의 까닭 줄과 시험판 알림, 상단 바의 규칙 줄과 예고, 그릇 없이 맞은 결말의 쪽(결정 13); 주장의 경고 줄과 반란 문턱의 표시(결정 14).
  9. **셋째 칸 뒤 계절의 재측정**(#132·#135·#136·#140·#142·#144·#145 — #129는 재지 않음, §5.2)과 **B6b의 경주를 위한 입력**(AI가 처음 셋째 칸에 오른 정확한 틱, 분야마다 연구가 1.0에 닿는 틱). 결정 11·12.
  10. **첫 테스트 커밋**: `Sim.NotablesFromEventsOnly`를 게임 데이터로 되돌린다. 결정 10.
  11. **글 감사**: 까닭 줄·규칙 줄·그릇 없이 맞은 결말·알아챔·주장의 경고·반란 문턱, 그리고 B6a에서 처음 판 안에 서는 `petition_exalt`의 글(§3.1·§8). 결정 6·7·16.
  12. **문서와 규칙 파일**. 결정 19·20·21.
- **B6a 밖**(어디로 가는지 — 넘기는 일마다 `OPEN_QUESTIONS`에 새 행, 번호는 #155부터; 지금 마지막이 #154다 — `OPEN_QUESTIONS.md:122`; 표는 노트 §0.2·§0.3):
  - **B6b**(이 헤더 다음, 따로 헤더 — 결정 1): 넷째·다섯째 칸의 통치자 업적 행과 VS 정의의 수량(새 #160 — `docs/50:268`·`:270`·`:274`, `pillars.md:108`), 셋째 칸 위의 AI — 넷째 칸의 제동 `AIFourthRungCap`(`Data/CosmicDefinitionRows.h:759` — 지금 아무도 읽지 않는다)과 셋째 칸 위만의 AI 지연, '다른 백성이 먼저'의 몫의 띠(새 #161), #1(통치자 T4→T5의 '개인 칸 ≥ 3'과 칸 공명 — 그 행의 걸림을 'B6b 헤더'로), #141(방랑자의 영압 선택 — #1의 칸 공명과 함께; 공명이 들면 방랑자의 칸이 오르는 블록이다, B5 결정 9의 다시 물음), 칙령 범위의 넓힘(T4·T5 — `systems.md:166`·`:169`; #160과 — T4→T5 행의 꼴이 정한다), 영압이라는 업적 기회(별칭·이웃의 경외 — 새 #156, 업적 행과 함께 — `crisis_and_transcendents.md:115`의 'B6, 업적과 함께'; 별칭은 사람의 칭호가 아니라 백성이 붙이는 시대 이름의 꼴 — `docs/70:184`), 보이지 않는 백성이 낸 엔딩의 글(새 #162 — §1.1의 '먼 줄'), #107(격차 규칙의 Core 술어 — `CLAUDE.md` 규칙 14), #121·#122·#129·#154, 넷째·다섯째 칸의 연출(다섯째는 금빛 없이)과 다음 칸 툴팁(`UI/CosmicNextRung.h:58`), 경주의 묶음과 '도약이 일으킨 다른 질서 < 15%'(`docs/40:460`), **시계의 마지막 값**(#2 — 결정 2-나).
  - **Phase 6+**: T5 문턱·두 길·후원 카드·봉인과 그 주인 필드·단독 승천의 길(단독 승천은 B6a에서 기록 종류와 까닭 줄만 — 결정 1), 받는 길의 두 변형('함께 앉았다'는 #14로), 그 묶음의 신학 판정(새 #163 — 문의 '안에서'·'기다린다'의 되풀이·'왕 노릇'·'질서의 화신'·찬탈의 제목·후원 결말의 이름·'법은 너의 것이 아니었다'·단독 승천의 까닭 줄).
  - **B7**: 희년(#29 ⑧의 걸림을 'B7 또는 EA'로), 물리 연구의 함대 승수(새 #155 — #131·#145·#129와 한 커밋), 역할이 바뀐 첫 틱의 카드(`CosmicPetitions::IsFirstTickOfLife` 넓히기 — 새 #158, #141의 '난 틱 가까이의 즉위' 몫과 함께), `ResistFleetFactor`의 바닥(#145).
  - **B8**: 울타리의 증언(타락의 둘째 1인칭 — 성역의 레인과 함께, 새 #159).
  - **Phase 5**([개발자] 결정 17-나 — 승인된 B6 칸에서 빼는 일이다): 위기 청원의 낮은 선택(새 #157 — 들인다면 해결이 아니라 시간으로, 그리고 `Data.Integrity`의 첫 위기 검사 `FirstCrisisProblems`를 그 유예까지 넓힌다 — `Tests/CosmicDataTests.cpp:2454`).
- **바뀌는 것**: 저장 필드 하나와 태그 다섯(§2.1 — `SaveVersion` 1 그대로); 셋째 칸이 판을 끝내지 않으므로 **통치자·방랑자 정책 판의 세계**가 옮고(`[B-6a3a]`), 기록이 시계를 적으므로 6,000에 닿는 판의 두 해시가 옮는다(`[B-6a3b]` — §5.3); 알아챔의 플래그와 `petition_exalt`가 통치자 판의 세계를 옮긴다. **입력 없는 세계(정책 없는 판)의 잰 수는 그대로다**(B6a에는 넷째 칸으로 오르는 행이 없다) — 다만 첫 통치자가 셋째 칸까지 왕좌를 지킨 시드는 알아챔 플래그(세계 해시)를, 다스리는 백성이 둘째 칸에 선 시드는 `[B-6a6]`부터 `petition_exalt`의 대기 칸(앎 — 전체 해시)을 얻을 수 있다(시드 42의 입력 없는 그릇은 1,072에 폐위된다 — `Tools/baselines/seed42.json:13`).
- **지키는 것**: 거룩한 질서는 엔티티·화자·수치·엔딩 조건이 아니다(`CLAUDE.md` 규칙 11) — 엔딩 술어는 세계의 사실(칸·생애·공백기·틱)만 읽고, 까닭 줄은 판의 규칙으로만 말한다(A4 헤더 결정의 꼴 — '…에 이르렀다 — 이 판은 여기까지다', `Tools/data/ST_CA.csv:133-135`); 1인칭은 타락만(`docs/70:124`) — 알아챔은 그 1인칭이 2인칭으로 말을 거는 한 줄의 예외다(`docs/70:168`); 플레이어에게 칭호나 '신이 되었다'가 없다 — 엔딩의 화면 제목을 두지 않는다(결정 6-나); 칸은 다섯 그대로(규칙 12 — 여섯째 자리·문 너머 없음); 영혼·구원·멸망을 세는 칸이 없다 — 엔딩 기록은 판의 사실 넷(종류·틱·백성·그릇 없음)뿐이다.
- **판의 끝과 B6a가 보이는 곳**: 통치자 정책 판은 셋째 칸(중앙값 2,369 — B4 헤더 `:89`) 뒤 시계까지 약 3,600틱을 더 논다. 이 계절에 영압 카드·1인칭 주장 줄·알아챔·지킴·신정론의 띠가 처음 판 안에 든다(B4가 '끝 뒤'에만 잰 것 — `OPEN_QUESTIONS.md:105`·`:111`). 집단 승천·단독 승천·다른 질서·침묵은 B6a의 데이터로 닿지 않는다 — 테스트로만 선다(§6). 그래서 B6a의 손 확인은 시계까지의 판과 공백기 안의 끝, 끝난 옛 세이브를 본다(§9).

## 1. 기대는 사실 (2026-10-31, HEAD `b10be48`)

### 1.1 지금 코드 (줄마다의 근거는 노트 §0)

- **끝은 순수 함수 하나다**: `CosmicEndings::ProvisionalEndOf`가 생애가 없으면 None, 어느 생애든 `PeakTier` ≥ `EndRung`(C++ 상수 T3)면 Rung, 아니면 `ProvisionalEndTick` > 0 ∧ `Tick` ≥ 그 값이면 Clock(같은 틱은 칸이 이긴다 — `Sim/CosmicEndings.h:23-30`, `.cpp:8-27`). 열거 `ECosmicProvisionalEnd`는 UENUM이 아니다(`.h:13-19`). 머리 주석은 이 파일을 끝의 조건의 한 집이라 하고 'The four endings (B5, B6) come here too'라 적는다(`.h:1-3`).
- **시뮬은 끝을 모른다**(`docs/20:89`): 그 함수를 읽는 곳은 컨트롤러의 래치, 저장 층의 경계 비교, 두 측정 정책, 커맨드렛, 연대기의 까닭 줄뿐이다(자리마다의 줄은 노트 §0.1′). 래치·두 정책은 끝 뒤에 멈춘다 — 정책은 중점·칙령·외교를 그친다(`-PastEnd=1`이면 이어 간다 — `Tests/CosmicBalanceCommandlet.cpp:1058-1067`).
- **B6를 기다리는 약속과 자리표시**: 헤더의 'Not yet: the per-tick ending-condition check … a run that has ended simulates nothing on a later RunHeadless or StepOnce'(`Sim/GalaxySimulationSubsystem.h:213-216`), 같은 약속의 사본(`docs/21:1326`), `RunHeadless` 고리 안의 자리표시 주석(`Sim/GalaxySimulationSubsystem.cpp:1178`). '네 엔딩의 저장 기록은 B6 헤더가 정한다'(`docs/20:195`).
- **틱의 끝**: 열째 단계 `ResolvePendingRoleEffect()`와 `SyncRngState()`가 단계 가드 안에서 차례로 돌고(`Sim/GalaxySimulationSubsystem.cpp:1575-1578`) 그 뒤 경계의 `OnSimTick`이 난다(`:1581-1586`). 업적 판정은 AI 단계의 끝이다(`:2731`) — 다스리는 백성을 먼저, 그다음 다른 백성을 `Civs` 순서로 판정한다(`Ascension/AscensionSubsystem.cpp:45-64`). 생애의 `PeakTier`는 그릇이 다스리는 백성이 오를 때만 오른다(`:145-158`).
- **`RunLeap`은 `RunHeadless` 뒤 경계 하나**(`Sim/GalaxySimulationSubsystem.cpp:1152-1166`)이고 끝을 모른다. 그래서 컨트롤러가 **시계만** 미리 셈해 도약을 자른다(`AdvanceLeap` — 줄은 노트 §0.1′). 앞에 끝이 없으면 침묵에서 시간을 보통의 놀이로 돌려준다; 앞에 끝이 있으면 침묵의 도약은 그 끝까지 한 번 더 달린다(`LeapOfThePages`·`IsSilent`).
- **임시 끝 스위치 `-ProvisionalEnd=0`**은 컨트롤러의 세션 스위치다(`World/CosmicGameMode.cpp:543-549`) — 꺼지면 래치가 아무것도 보지 않는다(`:1190`).
- **저장**: 새 필드는 기본값이면 세이브에 쓰이지 않고 로드하는 빌드의 초기값으로 읽힌다(델타 저장 — `Core/GalaxyState.h:4-11`). `CurrentSaveVersion` 1, 초기값 리터럴 1(`:793`·`:799`). 이주 함수의 자리는 주석뿐이다(`Save/CosmicSaveSubsystem.cpp:504`). `FGalaxyState`의 마지막 필드는 `Flags`다(`Core/GalaxyState.h:871-873`).
- **해시**: 두 해시는 `FGalaxyState`의 모든 속성을 반사로 걷고(`Sim/GalaxySimulationSubsystem.cpp:448-457`) 빼는 것은 최상위 이름으로만 뺀다(`:1450-1461` — 두 해시에서 `GalaxyID`, 세계 해시에서 `Knowing`·`Log`·`Chronicle`·`ChronicleEviction`·`TellRngState`). **그래서 새 필드는 기본값이어도 다섯 기준값의 두 해시를 모두 옮긴다.**
- **침묵의 범위**: 후보는 죽은 자리와 같은 은하 행에 있는 **모든 백성의** 정착된 성계에서 죽은 자리와 치명 위기의 성계를 뺀 것이다(`Sim/CosmicSuccession.cpp:42-53`) — 플레이어의 백성만이 아니다; 성역은 주인이 없어 저절로 빠진다. 설계는 '상세 은하의 소유 성계가 0일 때 사망'이라 적는다(`crisis_and_transcendents.md:145`). 지금 그 상태는 테스트로만 선다(`LeaveNoCandidate` — 노트 §0.5′).
- **`petition_exalt`는 오지 않는다**: 조건이 `PlayerRoleIs Ruler` ∧ `PlayerTierAtLeast 2`(`Tools/data/DT_Encounters.json:961-976`)이고, 그 조건은 통치자에게도 **개인 칸**을 읽는다(값은 칸 번호 1~5). 개인 칸을 쓰는 곳은 계승의 내림 하나다. 그래서 실제 판의 개인 칸은 늘 첫째이고 이 카드는 서지 않는다. A2가 뜻한 때는 '둘째 칸에 오른 틱'이다(A2 헤더 `:148`·`:194`). `Exaltation`을 움직이는 데이터는 이 카드 하나이고 그 값은 그릇마다 0으로 돌아간다(줄은 노트 §0.4′).
- **알아챔**: `bAwareOfPlayer`는 아무도 쓰지 않는다(`Core/GalaxyState.h:465-467` — 주석 'Survives succession'); 그것을 읽는 조건을 쓰는 데이터 행도 없다. `VoiceTemplates` 한 배열이 알아챔과 울타리의 증언 둘을 품도록 적혀 있고 스키마가 비워 둔다. 말하기 도우미는 변형 배열에서 하나를 말하기 스트림 한 수로 고른다 — **한 배열에 때가 다른 두 말을 두면 알아챔의 순간에 울타리의 증언이 뽑힐 수 있다.** 지금 1인칭의 유일한 자리는 주장의 편입이다(보는 칸 셋째부터; 줄은 노트 §0.6′).
- **먼 줄**: 범위 밖 백성이 셋째 칸 이상으로 오르면 이름 없는 먼 줄 `log_far_ascent`가 선다(`Log/CosmicLogSubsystem.cpp:241-244`; 규칙 '시계는 읽힌다' — `docs/70:55`). 격차 규칙의 세 사본 가운데 로그의 것은 `:209`다 — 넷째 칸 백성이 생기는 B6b가 처음 그 격차를 일하게 한다(#107).
- **영적 시야와 ×25**는 `>=` 비교라(`World/CosmicMapLook.cpp:201-204`) 셋째 칸 뒤에도 열린 채다 — 고칠 것이 없다.

### 1.2 잰 값 (B5 끝의 세계 — 표는 노트 §0.1)

표와 원천은 노트 §0.1. 이 헤더가 기대는 수만: 통치자 평화 정책 30판의 셋째 칸 중앙값 2,369(그래서 스크립트 통치자의 셋째 칸 뒤 계절은 약 3,600틱), 6,000 안에 주장이 선 판 18/30(첫 주장 중앙값 3,328), 끝 전 1인칭 줄 0; 시드 42 통치자의 셋째 칸 2,375 — 방랑자 판은 3,121에 `civ_7`에 즉위하고 그 뒤 다스린 백성의 셋째 칸이 6,000 전에 판을 끝냈다(`Tools/baselines/seed42.json:17`·`:18`; 그 틱은 `[B-6a2]`의 CSV에서 다시 읽어 노트 §5에 적는다).

### 1.3 B3·B4·B5가 넘긴 것 (표는 노트 §0.3)

B6 칸에 적힌 열여섯 남짓(`docs/50:12`)을 B6a·B6b·B7·B8·Phase 5·Phase 6+로 나눈 표. B6a가 반드시 하는 것은 `Sim.NotablesFromEventsOnly`(결정 10), 알아챔(결정 7), 침묵의 글과 기록(결정 6·8), 그릇 없이 맞은 결말의 칸(결정 3), 셋째 칸 뒤 계절의 재측정(결정 12), 주장의 경고와 반란 문턱(결정 14 — 넘길 수 있으나 셋째 칸 뒤 놀이가 그것이 쓸모 있는 때라 싸다).

### 1.4 열린 물음 가운데 B6a가 닫거나 고치거나 옮길 것 (표는 노트 §0.2)

닫음: #117(기록 — 결정 3~5)·#150(침묵의 글과 이름 — 결정 6·8)·#140(알아챔 — 결정 7). 잰 뒤 닫거나 수와 함께 옮김: #132·#135·#136·#142·#144·#145(결정 12의 미리 적은 판단). 걸림만 고침: #1·#107·#121·#122·#129·#154(B6b), #141(B6b/B7), #29(B7), #14(Phase 6+), #2(B6b — 시계의 마지막 값), #13(손 확인 ①). 새 행 #155~#163.

### 1.5 문서끼리·문서와 코드가 어긋난 것 (표는 노트 §1)

열하나 — 표는 노트 §1(B6의 정의의 'T5 문턱', 임시 끝의 '시뮬은 끝을 모른다'와 B6의 약속, 침묵의 조건 둘, `petition_exalt`의 칸, `bWatched`, `VoiceTemplates`의 두 말, 낡은 주석 셋과 '불멸', #154의 'B7', 알아챔의 '같은 부름'). 고침은 각 결정과 §7.

## 2. 장치

### 2.1 저장 필드 — 엔딩 기록 하나 (`[B-6a2]` — 필드만의 커밋)

```cpp
// Core/GalaxyState.h — USTRUCT, FGalaxyState의 끝(Flags 뒤)에 멤버 하나: UPROPERTY() FCosmicEndingRecord Ending;
// How the run ended (B6a header section 2.1). Written once, after the tenth stage, by the tick whose end it is
// (UGalaxySimulationSubsystem - CosmicEndings::EndingOf). Frozen defaults: an empty Kind = the run goes on, so a save
// made before the field loads as a run that goes on (SaveVersion stays 1 - decision 2-da). Part of both hashes.
USTRUCT()
struct FCosmicEndingRecord
{
	GENERATED_BODY()
	UPROPERTY() FGameplayTag Kind;      // Ending.* (decision 6-ga); empty = none
	UPROPERTY() int64 Tick = 0;         // the tick that wrote it
	UPROPERTY() FName CivID;            // the people at the fifth rung (Ending.RuledPeople, Ending.OtherPeople); none otherwise
	UPROPERTY() bool bNoVessel = false; // the calling was in the interregnum when it was written (B5 decision 12-ra)
};
```

- **종류는 태그다**(저장 UENUM이 아니다 — 열거자 순서가 영구가 되지 않는다; A4 노트 `:462`의 까닭). 태그 다섯(이름은 결정 6-가 — 권장안): `Ending.RuledPeople`(설계의 집단 승천)·`Ending.Vessel`(단독 승천 — B6a에서는 기록 종류만)·`Ending.OtherPeople`(다른 질서)·`Ending.NoCandidate`(침묵의 우주)·`Ending.Clock`(시계). 두 단계(규칙 9의 셋 안). `Config/DefaultGameplayTags.ini`에 선언하고 `Core/CosmicTags.h`(`:10` — 코드가 이름으로 부르는 태그의 한 집)에 함수 다섯을 둔다. **태그의 설명(DevComment)은 사실의 낱말로만** — 예 `Ending.NoCandidate` = "No settled system in the death's galaxy row where a vessel can stand (CosmicSuccession::CandidatesOf empty). Never a screen title (B6a decision 6-na)."; 다섯 모두 설계 제목(영겁의 문명·질서를 맡다·다른 질서·침묵의 우주)과 성경 구절을 싣지 않는다(이웃 `Way.*`의 설명이 엔딩 제목을 싣는 꼴 — `Config/DefaultGameplayTags.ini:47-48` — 을 따르지 않는다; 그 둘은 `[B-6a7]`이 고친다 — §7).
- **이름을 이때 정하는 까닭**: 저장 필드와 태그 이름은 세이브에 이름으로 남아 뒤에 바꾸면 그 칸을 잃는다(`Core/GalaxyState.h:12`, 규칙 7). B6b가 집단 승천과 다른 질서를 실제로 닿게 할 때 필드를 다시 바꾸지 않도록 다섯 종류를 지금 다 정한다.
- **두 해시에 든다**(결정 3): 엔딩은 세계의 사실이다(무엇이 판을 끝냈는가). 해시의 빼는 목록(`Sim/GalaxySimulationSubsystem.cpp:1450-1461`)은 그대로라 규칙 6의 문장도 그대로다. 그래서 이 커밋은 **다섯 기준값 줄의 두 해시와 해시 판 둘을 모두** 옮긴다 — 세계는 그대로임을 커맨드렛 CSV의 결정적 칸이 같음으로 보인다(B3 `[B-3a]`의 꼴 — `STATUS.md:25`).
- **얼린 기본값**: 빈 `Kind`·`Tick` 0·`CivID` 없음·`bNoVessel` 거짓 — 세이브가 생긴 뒤 바꾸지 않는다(규칙 7). 끝은 이 필드 하나다 — 끝의 틱·종류를 다른 필드에 베끼지 않는다(셋째 칸에 오른 틱 같은 칸도 두지 않는다 — '문턱의 계절'을 들이지 않으므로, 결정 2).
- `Tick`은 그 끝을 낸 틱이다 — 연대기의 '{0}주기가 흘렀다'와 `CosmicEnd` 줄이 `State.Tick`이 아니라 이것을 읽는다(끝 뒤로 돈 상태나 이어 연 세이브에서도 끝의 틱을 말한다). **한계**: 연대기의 장은 그대로 `State.Tick`까지 잰다(`CosmicChronicle::ShownLivesOf`·장과 그 뒤의 끝 — `Log/CosmicChronicle.cpp:250`·`:334`·`:364-371`). 끝 뒤로 돈 상태는 개발 스위치 `-ProvisionalEnd=0`의 세션과 테스트에서만 생기므로 그런 세이브는 장을 그 세이브의 틱까지 보인다 — 장을 기록의 틱에서 자르는 고침은 그런 상태가 놀이의 길에 들 때 한다.

### 2.2 엔딩 술어 — 한 집 (`Sim/CosmicEndings` — 순수 함수, `[B-6a3b]`)

`FCosmicEndingRecord CosmicEndings::EndingOf(const FGalaxyState& State, const FCosmicRulesDef& Rules)` — **지금 끝이 서 있는가**를 기록과 상관없이 답한다(기록을 쓰는 자리는 §2.3). 읽는 것: `Tick`, `Calling`(`Lives`의 `PeakTier`·`RuledCivIDs`·`DiedTick`, `LifeState`, `PendingLeapTicks`, `PersonalTier`), `Civs`(`Tier`·`CivID`), 노브 `ProvisionalEndTick`. **앎·로그·연대기·말하기 스트림은 읽지 않는다**(규칙 2; 지금 함수의 꼴 — `Sim/CosmicEndings.h:26-29`). 추첨 없음, 쓰기 없음.

차례(앞의 것이 서면 뒤는 보지 않는다 — 결정 5):

| 차례 | 종류 | 조건 | `CivID` |
|---|---|---|---|
| — | 없음 | `Calling.Lives`가 비었다(지금 꼴 — `.cpp:10-14`) | — |
| 1 | `Ending.RuledPeople` | 어느 생애의 `PeakTier` == T5 — `PeakTier`는 그릇이 설 때(`Install`)의 개인 칸(`Player/PlayerRoleSubsystem.cpp:198` — 셋째 칸 이상이면 계승을 건너 남는다, `:530-534`)과 그릇이 다스리는 동안 그 백성이 업적으로 오른 칸(`Ascension/AscensionSubsystem.cpp:145-158`) 가운데 큰 것이다; B6a에는 개인 칸을 올리는 코드가 없어 앞의 것은 늘 첫째라 이 행은 다스리는 백성의 오름만 센다(개인 칸을 올리는 블록 — #1·Phase 6+ — 이 행을 개인 칸과 가른다). 오르기만 하고 폐위와 같은 틱이어도 센다(업적은 열째 단계보다 먼저 판정된다 — `Sim/CosmicEndings.h:17`의 지금 규칙) | 그 생애의 `RuledCivIDs`를 끝에서부터 보아 다섯째 칸에 선 첫 백성(지금 까닭 줄이 백성을 찾는 꼴 — `UI/CosmicChronicleWidget.cpp:126-142`); 없으면 비움 |
| 2 | `Ending.Vessel` | `LifeState == Embodied` ∧ `PersonalTier` == T5 — **B6a에는 개인 칸을 올리는 코드가 없어 닿지 않는다**(`Player/PlayerRoleSubsystem.cpp:534`만 쓴다). 길(Path)은 묻지 않는다 — 단독 승천의 길이 오는 Phase 6+가 술어만 고친다(필드는 그대로) | 비움 |
| 3 | `Ending.OtherPeople` | 다섯째 칸에 선 백성이 하나라도 있다(1이 서지 않았으므로 그 칸은 그릇이 올린 것이 아니다 — 섭정 아래의 옛 백성(#121)과 이미 다섯째 칸인 백성에 즉위한 경우도 여기다) | `Civs` 순서로 첫 그 백성 |
| 4 | `Ending.NoCandidate` | `CosmicSuccession::IsSilent(State.Calling)` = `LifeState == Interregnum` ∧ `PendingLeapTicks` ≤ 0 — 열째 단계가 계승할 후보를 찾지 못하고 공백기에 머문 상태(B5 헤더 `:77` — 틱 경계에서 '후보가 없어 계승하지 못했다'와 같다). 침묵의 술어는 Sim에 하나다 — 컨트롤러의 `CosmicCallingFlowImpl::IsSilent`(`World/CosmicGameMode.cpp:892-895`, 부르는 곳 `:903`·`:998`·`:1034`)도 이것을 부른다(규칙 13 — 결정 20) | 비움 |
| 5 | `Ending.Clock` | `ProvisionalEndTick` > 0 ∧ `Tick` ≥ 그 값(지금 꼴 그대로 — `.cpp:22-25`) | 비움 |

`Tick` = `State.Tick`; `bNoVessel` = (`LifeState == Interregnum`) ∧ ¬(`Kind` ∈ {`Ending.RuledPeople`, `Ending.OtherPeople`} ∧ `Calling.Lives.Last().DiedTick == State.Tick`) — 다섯째 칸의 오름(AI 단계 끝 — `Sim/GalaxySimulationSubsystem.cpp:2731`)은 같은 틱 열째 단계의 죽음(`:1577`)보다 먼저, 그릇이 있던 동안 일어났다(`FLifeRecord.DiedTick` — `Core/GalaxyState.h:500`). 시계·침묵은 그 틱의 끝에 공백기면 참이다.

- **셋째 칸은 이제 끝이 아니다**: `EndRung`(`Sim/CosmicEndings.h:23-24`)과 `Rung` 가지는 `[B-6a3a]`가, `ECosmicProvisionalEnd`(`:13-19`)와 `ProvisionalEndOf`는 읽는 곳이 기록으로 옮는 `[B-6a3c]`가 지운다(그 사이 `[B-6a3b]`에서는 읽는 곳이 시계만 답하는 옛 함수를 읽고 새 함수는 기록을 쓰는 자리만 부른다 — 결정 15). 열거자 `Rung`은 `[B-6a3c]`까지 남는다(`ProvisionalEndOf`가 내지 않을 뿐 — `NameOf`의 가지(`Sim/CosmicEndings.cpp:33-34`)와 `Tests/CosmicChronicleTests.cpp:157`은 `[B-6a3c]`가 열거와 함께 지운다). 이름 `ProvisionalEndOf`는 남기지 않는다(새 이름 `EndingOf` — 테스트와 커맨드렛이 옛 뜻으로 부르는 자리를 컴파일러가 찾게; 컴파일러가 못 찾는 자리 — 손으로 놓은 틱으로 끝을 세우는 테스트 — 는 §2.7의 목록).
- `const TCHAR* NameOf(const FGameplayTag& Kind)` — 태그의 끝 낱말(`RuledPeople`…`Clock`; 빈 태그는 `None`). 커맨드렛 줄과 `CosmicEnd` 줄이 읽는다(시계 끝의 줄 'CosmicEnd: Clock at tick …'은 글자 그대로다 — 손 확인의 잣대가 바뀌지 않는다).
- **규칙 13의 단일 창구**에 '엔딩은 `CosmicEndings::EndingOf`, 침묵은 `CosmicSuccession::IsSilent`'를 더한다(결정 20). 화면·정책·저장 층은 이 함수를 부르지 않고 **기록**을 읽는다(§2.7) — 함수를 부르는 곳은 기록을 쓰는 한 자리와 테스트, 커맨드렛의 `end check`(§5.1)뿐이다.
- **2(그릇의 개인 칸)를 3보다 먼저 두는 까닭**: 그릇의 것이 같은 틱의 다른 백성보다 먼저다(1과 같은 결 — 결정 5). 닿지 않는 칸이라 지금 값은 없다.

### 2.3 쓰는 자리 — 열째 단계 뒤 한 번 (`[B-6a3b]`; `docs/20:285`와 `docs/21` §7.4를 `[B-6a2]`가 먼저 고친다)

`SimulateTick`의 단계 가드 안, `ResolvePendingRoleEffect()` 다음·`SyncRngState()` 앞(`Sim/GalaxySimulationSubsystem.cpp:1577-1578` 사이):

```
(열째 단계 뒤) State.Ending.Kind가 비었으면: R = CosmicEndings::EndingOf(State, Rules); R.Kind가 서면 State.Ending = R.
```

- **왜 이 자리인가**: 열째 단계가 이 틱의 죽음·계승·폐위를 끝낸 뒤라 `LifeState`·`PendingLeapTicks`(침묵과 `bNoVessel`)가 그 틱의 답이고, 업적(AI 단계 끝 — `:2731`)과 칸은 이미 정해졌다. 단계 가드 안이라 그동안 들어오는 저장·틱 요청은 이미 있는 문이 거절하거나 미룬다(B5 헤더 `:116`).
- **한 번만 쓴다**: 기록이 선 뒤에는 `EndingOf`를 부르지도 않는다 — 먼저 온 끝이 판의 끝이다(다른 백성이 끝 뒤에 사라지거나 역할이 바뀌어도 답이 뒤집히지 않는다 — 순수 함수만으로 두면 끝 뒤로 도는 측정에서 먼저 온 끝을 잃는다).
- **추첨 없음, 줄 없음**: 시뮬 스트림·말하기 스트림·파생 정수 어느 것도 뽑지 않고(규칙 3) 로그에 줄을 내지 않는다 — 끝을 말하는 것은 연대기 쪽의 까닭 줄(표현)이다(§4). 다섯째 칸에 오른 일 자체는 이미 있는 승천 줄(`Log.Ascension` — `Ascension/AscensionSubsystem.cpp:184-196`)이 말한다.
- **틱 0에는 쓰지 않는다**(생성된 상태 — `NewGalaxy`는 단계를 돌지 않는다). 시험판에서 시계를 지난 상태를 손으로 놓은 테스트는 다음 틱에 기록을 본다.

### 2.4 같은 틱의 차례 (결정 5)

집단 승천 > 단독 승천 > 다른 질서 > 침묵 > 시계(§2.2의 표). 다스리는 백성이 먼저 판정되므로(`Ascension/AscensionSubsystem.cpp:45-64`) 다스리는 백성과 다른 백성이 같은 틱에 다섯째 칸에 서면 집단 승천이다 — 지금의 '칸이 시계를 이긴다'(`Sim/CosmicEndings.h:26`)와 같은 결이다. 다른 질서의 `CivID`는 `Civs` 순서로 처음인 백성이다.

### 2.5 끝 뒤의 시뮬 — 거절하지 않는다 (`[B-6a3b]`; `RunLeap`의 계약은 `[B-6a3c]`)

- `SimulateTick`·`StepOnce`·`RunHeadless`는 기록이 선 뒤에도 틱을 돌린다. `Sim/GalaxySimulationSubsystem.h:213-216`의 약속('a run that has ended simulates nothing…')과 그 사본(`docs/21:1326`)을 '시뮬은 끝을 기록하되 멈추지 않는다 — `RunLeap`만 그 틱에서 선다(§2.6)'로 다시 적고, 자리표시 주석(`.cpp:1178`)을 지운다. 코드가 원본이다(규칙 표 `docs/21`의 머리).
- **까닭**: 커맨드렛의 20,000틱 기준값 둘과 테스트의 긴 판(`Feats.Reachable`의 20,000틱 — `Tests/CosmicAscensionTests.cpp:1582`), B5의 끝 뒤 측정(`-PastEnd`)이 끝 뒤로 돈다; 거절하면 측정마다 끝을 넘는 스위치가 든다. 한 틱씩 돈 판과 긴 도약이 같은 세계라는 B5의 약속(B5 헤더 `:115`)도 그대로다 — 끝이 시뮬을 바꾸지 않으므로.
- **명령은 지금처럼 컨트롤러가 거절한다**(래치 — `World/CosmicGameMode.cpp:2158`~`:2515`의 `IsRunEnded()` 문들). Core 명령은 끝을 모른다(`docs/20:89`의 지금 규칙 그대로).
- **기록을 읽는 단계는 없다** — 끝 뒤의 세계는 기록이 없을 때와 같다(테스트 `Sim.EndingInert` — §6).

### 2.6 빨리 넘기기 — `RunLeap`이 끝의 틱에서 선다 (`[B-6a3c]`)

- `RunLeap(n)`: 부른 때 기록이 비어 있었으면 **기록이 적힌 틱에서 고리를 멈추고** 경계 하나를 낸다(B5의 경계 하나 — `Sim/GalaxySimulationSubsystem.cpp:1158-1165`). 몸은 `RunHeadless`를 부르지 않고 같은 꼴의 고리를 제가 돈다(알림 문 닫힘 — `:1174`의 꼴). 부른 때 이미 기록이 있으면 멈추지 않는다(끝 뒤의 넘김 — 스위치가 꺼진 세션).
- **까닭**: 다른 질서처럼 틱으로 미리 알 수 없는 끝이 공백기 안에 오면 컨트롤러가 미리 잘라 둘 수 없다. 설계는 '공백기 안에 일어나면 빨리 넘기기가 그 끝에서 멈춘다'(`crisis_and_transcendents.md:148`). 그 멈춤을 시계에도 쓰면 컨트롤러의 셈(`World/CosmicGameMode.cpp:863-869`)이 필요 없다.
- **`AdvanceLeap`**(컨트롤러 — `World/CosmicGameMode.cpp:854-880`): n = `PendingLeapTicks`(시계로 자르지 않는다). 스위치가 켜져 있으면 `RunLeap`이 끝에서 멈추고 그 경계가 래치를 건다(지금의 6,000 자르기와 같은 틱 — B5 결정 15의 쪽 차례 '비문 → 거기까지의 다이제스트 → 연대기'가 그대로다). **스위치가 꺼져 있으면** 래치가 서지 않으므로, 첫 `RunLeap`이 틱을 나아가게 했고 부른 때 비어 있던 기록이 이제 서 있으면 `RunLeap(PendingLeapTicks)`를 **한 번 더** 부른다(기록은 도약을 많아야 한 번 멈추므로 두 부름이면 다 넘는다; `PendingLeapTicks`로 되풀이하지 않는다 — 시뮬이 거절한 도약은 아무것도 돌지 않아 고리가 멈추지 않는다, `Sim/GalaxySimulationSubsystem.cpp:1154`·`World/CosmicGameMode.cpp:877-879`). 스위치는 세션 동안 끝을 보지 않는다는 지금의 뜻 그대로(`:1190`).
- **침묵**은 이제 그 틱의 엔딩이다(§2.8): 스위치가 켜져 있으면 계승의 틱에 기록이 서고 래치가 걸린다 — `LeapOfThePages`의 '앞의 끝까지 한 번 더'(`:901-907`)는 스위치가 꺼진 세션에서만 그 둘째 부름이 시간을 돌려준다(`:870-876` 그대로).
- 벽시계: 도약은 많아야 1,600틱이라 지금과 같다(B5 헤더 `:161`).

### 2.7 기록을 읽는 곳 (`[B-6a3c]` — 서명과 읽기; 새 글과 화면의 규칙은 `[B-6a4]`)

| 자리 | 지금 | B6a |
|---|---|---|
| 컨트롤러 래치 `FollowEnd` | `ProvisionalEndOf`(`World/CosmicGameMode.cpp:1197`), 멤버 `ECosmicProvisionalEnd RunEnd`(`World/CosmicGameMode.h:707`) | 기록의 `Kind`를 래치한다 — 멤버는 `FGameplayTag RunEnd`, `GetRunEnd()`는 태그, `IsRunEnded()` = 태그가 섰다(`:285-286`). `CosmicEnd:` 줄은 `NameOf(Kind)`와 기록의 `Tick`. 스위치·두 `BeginPlay`·로드·`ClearRunEnd`의 길은 그대로(`docs/20:89`의 래치 넷) |
| 저장 층 `MarkOf` | `bEnded` = 그 함수 ≠ None(`Save/CosmicSaveSubsystem.cpp:132`) | `bEnded` = 기록이 섰다. 까닭 `Ended`의 차례 그대로(`:144-146`) |
| 두 측정 정책 | 그 함수(`World/CosmicRulerPolicy.cpp:392`, `World/CosmicWandererPolicy.cpp:19`·`:31`) | 기록이 섰다. `-PastEnd=1` 그대로 |
| 커맨드렛 | `end:` 줄의 `RungTick`·`ClockTick`·`FirstEnd`(`Tests/CosmicBalanceCommandlet.cpp:1338-1352`·`:2442`), `end check`(`:2484-2490` — 셋째 칸이 박혀 있다) | `RungTick`과 옛 `end check`는 `[B-6a3a]`가 지우고, 새 `end check`는 `[B-6a3b]`, `end:`가 기록을 읽는 것과 새 줄 `ending:`은 `[B-6a3c]`(§5.1) |
| 연대기 쪽 | `Open(ECosmicProvisionalEnd, bAtOnce)`·`NoteOf`·`ShownCause`(`UI/CosmicChronicleWidget.h:62`·`:94`·`:113`; `.cpp:111-145`·`:334-361`·`:465` — `NoteOf`의 칸 가지는 `EndRung`을 읽는다 `:130`·`:137`), 부르는 곳 `World/CosmicGameMode.cpp:1064`·`:1235`·`:1266`·`:1332`·`:1370`(개발용 미리 보기) | **`[B-6a3c]`**(열거와 함께 — 컴파일되는 커밋): `Open(const FCosmicEndingRecord&, bool)`·`NoteOf(const FCosmicEndingRecord&)`·`ShownCause`는 기록, 미리 보기(`:1370`)는 빈 기록. 줄은 잠시 지금 키로 — `Ending.Clock`은 `Chronicle.Note.Clock`(기록의 `Tick`), 다른 종류는 기록의 `CivID`로 `Chronicle.Note.Rung`, 없으면 `RungNoCiv`(테스트로만 닿는다). `NoteOf`의 칸 가지는 `[B-6a3a]`가 `EndRung`과 함께 뗀다. 새 글·보이는 백성의 문·그릇 없음의 줄·시계에만의 시험판 알림은 `[B-6a4]`(§4) |
| 래치를 읽는 화면 | 칙령 창·피드·성계 패널·상단 바(`UI/CosmicEdictWidget.cpp:575`, `UI/CosmicLogFeedWidget.cpp:220`, `UI/CosmicSystemPanelWidget.cpp:850`, `UI/CosmicTopBarWidget.cpp:775`·`:830`) | 그대로(래치를 읽는다) |

**고치는 테스트 열세 파일**(노트 §0.9′ — §6): 셋째 칸으로 끝을 세우는 테스트는 `[B-6a3a]`가 시계로, 열거를 쓰거나 손으로 놓은 틱(`Tick = ProvisionalEndTick` 뒤 틱 없이 `FollowEnd(false)`)에 래치하는 테스트는 `[B-6a3c]`가 도우미 `CosmicTests::EndRunAt`(§6)로 고친다 — 뒤의 것은 열거를 부르지 않아 컴파일러가 찾지 못하고 기록이 틱 안에서만 쓰여(§2.3) 런타임에 깨진다(`Tests/CosmicDiplomacyUITests.cpp:809`, `Tests/CosmicEdictUITests.cpp:612`·`:635`, `Tests/CosmicFeedbackTests.cpp:363`, `Tests/CosmicChronicleScreenTests.cpp:297-301`과 `UI.Chronicle`의 `:678-679`; 기록 없이 6,000에 놓은 상태를 저장해 로드 뒤 래치를 단언하는 `UI.ContinueEnded` — `Tests/CosmicSaveSessionTests.cpp:411`·`:432`, 결정 2-다로 그 상태는 흐르는 판으로 열린다).

### 2.8 침묵이 엔딩이 된다 (`[B-6a3b]` 기록, `[B-6a3c]` 래치, `[B-6a4]` 쪽, `[B-6a5]` 글)

- 조건은 B5의 **후보 없음**이다(§2.2의 4 — 계승의 단일 창구 `CosmicSuccession::CandidatesOf`가 정한 상태를 그대로 읽는다; 규칙 13). 설계의 '소유 성계 0에서 사망'(`crisis_and_transcendents.md:145`, 같은 조건의 둘째 사본 `:195`)은 두 겹으로 다르다 — 후보는 모든 백성의 정착된 성계이고, 죽은 자리와 치명 위기의 성계는 소유여도 빠진다. 조건은 결정 6-다가 `[B-6a2]` 전에 정한다(태그 `Ending.NoCandidate`가 그 조건을 세이브에 얼린다 — 설계 문장도 그 전에, §7); '`Doctrine.Silence`와 일치하는 4번째 엔딩'의 문장은 6-마가 고친다.
- 그 상태는 늘 공백기라 기록의 `bNoVessel`이 참이다. 계승의 틱(도약의 끝, 도약 0이면 죽음의 틱)에 서고 그 틱이 끝이다 — 그 뒤 시계까지 기다리지 않는다(지금은 시계가 받는다 — B5 헤더 `:187`).
- B6a의 데이터로는 닿지 않는다: 분쟁은 수도와 마지막 정착된 성계를 앗지 않고(#126 — B7), 첫 백성 상실은 남은 자의 은혜가 막는다(B5 헤더 `:187`). 테스트로만 세운다(`LeaveNoCandidate` — `Tests/CosmicCallingPagesTests.cpp:221`). 측정은 '0 기대 — 재지 않음'으로 미리 적는다(§5.2).

### 2.9 그릇 없이 맞은 결말 (`bNoVessel` — #117, B5 결정 12-라)

- 참이 되는 끝: 침묵(늘), 공백기 안의 시계(지금 끝 전 죽음의 약 넷에 하나 — `OPEN_QUESTIONS.md:20`의 #10), 공백기 안의 다른 질서(B6b가 닿게 한다). 다섯째 칸에 오른 틱에 그 그릇이 죽은 끝은 **아니다**(§2.2의 `bNoVessel` — 오름은 그릇이 있던 동안 일어났다; 그것을 '그릇이 없던 동안'으로 적으면 거짓이고, 비문 뒤에 백성의 다섯째 칸이 이어 서면 '그들을 위해 죽었다'로 읽힌다 — `docs/70:209`, 메시아 신호 2; 글 감사의 눈 ③).
- 쪽의 차례는 B5 결정 15 그대로 — 비문 → 거기까지의 다이제스트 → 연대기(B5 헤더 `:417`). 연대기의 까닭 줄 아래에 한 줄을 더한다(새 키 `Chronicle.Note.NoVessel` — §3.3). '부름이 지켜본'·'쉼'·'고요'를 쓰지 않는다(공백기에는 눈이 없다 — B5 헤더 `:407`; `silence_words` — `Tools/data/schema.json:46`).
- 기록이 이미 그 사실을 지니므로 화면은 래치 때의 `LifeState`를 다시 읽지 않는다.

### 2.10 끝난 MVP 세이브 — 흐르는 판으로 연다 (결정 2-다)

- 그 세이브에는 `Ending`이 없다(그 빌드에 필드가 없었다) — 델타 로드가 빈 기록을 준다(`Core/GalaxyState.h:8-11`). 그래서 **셋째 칸으로 끝난 판**(어느 생애든 `PeakTier` T3)은 흐르는 판으로 열리고 시계까지 이어 놀 수 있다; **시계로 끝난 판**(`Tick` ≥ 6,000)은 다음 틱에 시계의 기록을 그 틱으로 적고 끝난다(예: 6,201). 침묵에 머문 B5의 세이브도 다음 틱에 `Ending.NoCandidate`로 끝난다.
- 이주 함수를 두지 않는다 — 그런 세이브는 개발자와 외부 한 사람의 시험 세이브뿐이다(`STATUS.md:31`의 3). 이어 연 판이 끝나지 않았으면 컨트롤러는 멈춘 채 세운다(지금 꼴 — `World/CosmicGameMode.h:358-359`의 이어하기 규칙; 바꾸지 않는다).
- **B6a의 `Ending.Clock` 기록은 마지막이다**(결정 2-나): B6b가 시계를 옮겨도 6,000에 끝난 B6a 세이브는 끝난 채 열린다(시험 세이브뿐이다 — 2-다의 까닭). 다시 열지는 B6b가 정하고, 다시 열면 `SaveVersion` 2와 새 `ProvisionalEndTick`보다 앞선 `Tick`의 `Ending.Clock`을 비우는 이주가 든다(`Save/CosmicSaveSubsystem.cpp:504`). 끝 뒤로 돈 세이브(개발 스위치의 세션)의 연대기 장은 그 세이브의 틱까지다(§2.1의 한계).

### 2.11 알아챔 — 플래그를 처음 쓴다 (`[B-6a3a]` 플래그, `[B-6a5]` 줄 — 결정 7 [개발자])

결정 7-가·나의 답(권장대로):

- **때**: 그릇이 몸을 입고 다스리는 백성이 **셋째 칸 이상인 동안 그 백성의 정착된 성계에 처음 주장이 서는 틱**. 플래그의 자리: 위기 단계의 주장 규칙이 주장을 놓은 바로 뒤(`TryPlaceClaim` — `Sim/GalaxySimulationSubsystem.cpp:306-332`의 `Place` `:331` 뒤). 그 주장의 주장자(`ClaimantOf`가 고른 초월자 — `:318`)의 `bAwareOfPlayer`가 거짓이면 참으로 쓰고, `TryPlaceClaim`은 플래그를 세운 주장자의 번호를 돌려준다(줄의 자리는 아래 '줄' — 같은 만료 처리의 끝이다).
- **세계만 읽는다**: 조건은 `Calling.LifeState`·`Calling.PendingEffect`·`Player.Role`·`Player.RuledCivID`·그 성계의 주인·그 백성의 `Tier`뿐이다 — '다스리는 백성의 눈에 든다'를 앎으로 판정하면 세계 코드가 앎을 읽게 된다(규칙 2). 추첨 없음. **`PendingEffect`가 비어야 한다**: 이 틱에 앞서 폐위·양위·죽음이 기록된 그릇(반란 만료의 폐위, 각성 만료의 `KillPlayer`, 초신성 붕괴의 폐위 — `:2495-2503`, 양위 명령)은 열째 단계에서 떠나므로(`:3053-3072`) 알아채이지 않는다 — 판에 하나뿐인 줄이 떠나는 그릇에게 가고 플래그가 뒤의 그릇을 막는 일을 줄인다(B5가 죽음이 기록된 그릇의 이동·시야를 막는 꼴 — `:1556`). **거르지 못하는 것**: 같은 틱의 뒤에 기록되는 떠남 — 같은 만료 차례의 뒤 만료(반란·각성)와 위기 단계 끝의 이벤트 슬롯(`DeposeRuler`·`KillPlayer` 효과 — 규칙 4)은 줄이 이미 말해진 뒤에 기록된다. 그런 판은 커맨드렛이 센다(§5.1 — 알아챔 틱에 그릇이 떠났는가); 0이 아니면 `[B-6a5]`의 글 감사가 그 읽힘을 보고, 자리를 옮길지는 그 수와 함께 B6b 헤더로.
- **초월자마다 한 번, 줄은 판에 한 번**: 플래그는 세계 상태라 계승을 넘어 남고(`Core/GalaxyState.h:465-467`) 그 초월자에게는 다시 서지 않는다. 플래그는 초월자마다이고(판에 타락이 셋까지 — `MaxFallenRoster` `Sim/GalaxySimulationSubsystem.cpp:108`) **줄은 그 쓰기 전에 어느 초월자도 알아채지 않았을 때만** 말한다(쓰기 전에 `!State.Transcendents.ContainsByPredicate(…bAwareOfPlayer)`를 본다) — 2인칭은 판에 한 줄이다(§3.6의 4). B6a의 데이터는 판에 타락을 하나만 세우므로(`FallenTranscendentCount` 1 — `Tools/data/DT_Rules.json:27`; 타락 행도 하나 — `Tools/data/DT_Transcendents.json:3`) 둘이 같다. 플래그를 읽는 데이터 행은 B6a에도 없다(조건 `TranscendentAwareOfPlayer` — '이벤트 가중치가 바뀐다'(`crisis_and_transcendents.md:99`)는 그 행이 오는 블록의 일).
- **셋째 칸의 틱에 두지 않는 까닭**: 그 틱에는 승천 줄 → 연출 → 드러냄 '이제 보인다.'가 잇는다(`UI/CosmicAscensionOverlayWidget.h:42-44` — 받은 눈뜸이 업적의 상과 떨어져 서도록 한 자리). 그 곁에 타락의 첫 2인칭이 서면 열린 눈이 타락이 열어 준 것으로 읽힌다(뱀의 약속을 이룬 것처럼 보이는 '눈이 열림' — 창 3:5의 겹침, 이미 피하기로 적은 자리 `docs/90_LORE/theology_and_bestiary_notes.md:247`). 주장이 선 틱은 타락이 제 손을 그 백성에게 뻗은 때라 그 말이 그 일에 붙는다.
- **같은 틱의 카드**: 다스리는 성계에 주장이 서면 그 틱의 앎 단계 끝에 영압 카드가 올 수 있다(B4 — 상시 카드). 줄(위기 단계) → 카드(앎 단계)의 차례다. 카드의 두 목소리가 타락의 말에 대꾸하는 것으로 읽히지 않는지 글 감사가 본다(§8).
- **줄**(`[B-6a5]`): 플래그는 `Place` 뒤에 쓰고, 줄은 그 만료 처리의 끝 — `Events->FollowUpNotes(Ctx)`(새 주장의 발생 줄) 뒤(`Sim/GalaxySimulationSubsystem.cpp:2576`) — 서술자의 만료 줄(`:2537`)·기록 줄(`:2544-2551`)·편입의 징조와 1인칭(`:2556-2575`)·주장 발생 줄(`Place`는 알림만 남기고 `FollowUpNotes`가 말한다 — `Sim/CosmicHazards.cpp:173`, `Events/CosmicEventSubsystem.cpp:1369-1371`) 다음에 낸다. 차례는 만료 → 기록 → (징조·1인칭) → 주장 발생 → 알아챔(`docs/70:77`의 차례를 잇는다 — 타락의 말이 그것이 대꾸하는 일을 서술자가 말하기 전에 서지 않는다; 현현 규칙 ① `docs/70:173` — 타락은 1인칭이거나 그 결과의 수동태일 뿐 만료와 주장을 부른 자로 서지 않는다). 만료 처리는 `TryPlaceClaim`이 돌려준 번호를 지녀 끝에서 말한다. `Log.Transcendent`·**Severity 1**(B4의 1인칭 꼴 — `Log.Claim` Severity 1 `:2565`; 자동 정지가 없다 — `CosmicAutoPause::StopsFor`는 Severity 2부터, `World/CosmicAutoPause.cpp:24`, 그 정지의 사유 '정지 — 큰 일이 기록되었다' `Tools/data/ST_CA.csv:110`가 타락의 말에 붙지 않는다; 보라는 범주가 준다 — `UI/CosmicLogFeedWidget.cpp:64-68`)·주제 백성은 다스리는 백성·주제 성계 없음으로 한 줄(말하기 스트림 한 수 — `Log/CosmicLogSubsystem.h:100-106`). 보는 칸 바닥 셋째(`MinViewerTier` 3 — 주장 1인칭의 꼴 `Sim/GalaxySimulationSubsystem.cpp:2569`); 그 틱의 보는 칸은 다스리는 백성의 칸(`:1250-1261`)이라 늘 말해진다. 변형은 새 정의 칸 `AwarenessTemplates`에서 읽는다(§3.2).
- **책**: 생애 장과 두 책에 싣지 않는다 — 생애 장은 `Log.Claim`·`Log.Record`를 빼는 자리(`Log/CosmicChronicle.h:159`)에 `Log.Transcendent`를 더하고, 두 책은 그 범주를 읽지 않는다(2부 '주장된 것들'에 서면 주장이 아닌 말이 주장으로 읽힌다). 행위의 책에 타락의 2인칭이 서지 않게 하는 것이다. 피드에서만 들린다.

### 2.12 결정론과 저장 — 한눈에

| 입력·자리 | 시뮬 스트림 | 말하기 스트림 | 앎 | 저장 |
|---|---|---|---|---|
| 기록 쓰기(열째 단계 뒤) | 없음 | 없음 | 없음 | 필드 하나(첫 끝에 한 번) |
| 끝 뒤의 틱 | 보통의 틱 | 보통 | 보통 | 보통 |
| `RunLeap`의 멈춤 | 한 틱씩 돈 것과 같다 | 같다 | 같다 | 끝의 경계 하나 — 까닭 `Ended` |
| 알아챔 플래그(위기 단계, 초월자마다 한 번)와 줄(그 만료의 끝, 판에 한 번) | 없음 | `[B-6a5]`부터 줄마다 한 수 | 없음 | 이미 있는 필드 |
| `petition_exalt`(데이터) | 받아들인 답마다 1(지금 규칙) | 카드 줄 | 대기 칸 | — |
| 주장의 경고·반란 문턱(표현) | — | — | — | — |

- 새 저장 필드 하나(`FGalaxyState::Ending`), 새 USTRUCT 하나, 새 태그 다섯, 새 정의 칸 하나(`FTranscendentDef.AwarenessTemplates` — 저장 아님), 새 노브 0, 새 UENUM 0, 지우는 C++ 열거 하나(`ECosmicProvisionalEnd` — 저장·해시·데이터 밖, `Sim/CosmicEndings.h:13`), 새 파생 정수 용도 0, `SaveVersion` 1 그대로.
- 커밋마다 옮는 해시: §5.3.

## 3. 데이터

### 3.1 `petition_exalt` 바로잡기 (`[B-6a6]` — 결정 9)

- 조건 `PlayerTierAtLeast 2`를 `CivTierAtLeast 2`로 바꾼다(`Tools/data/DT_Encounters.json:971-976`). 통치자에게 오는 상시 카드는 다스리는 백성을 주제 백성으로 판정하므로(`Events/CosmicPetitions.cpp:153-160` `RuledCivIndexOf`) 이 조건은 다스리는 백성의 칸을 읽는다(`Events/CosmicEventSubsystem.cpp:433-434`). 상시 카드는 조건이 막 선 틱에 한 번 온다(`Events/CosmicPetitions.cpp:341-343`) — 그래서 **다스리는 백성이 둘째 칸에 오른 틱**, 그리고 둘째 칸 이상의 백성에 즉위한 재위의 첫 틱(빈 머리 — B1)에 온다. 둘째 칸의 틱은 A2가 뜻한 때다(A2 헤더 `:148`); 즉위의 첫 틱은 B1 뒤의 새 자리다(`Events/CosmicPetitions.h:131-137` — 즉위했을 때 이미 서 있던 청원은 막 선 것으로 온다). 이 카드의 글(제목·두 목소리·세 결과 줄 — `Tools/data/DT_Encounters.json:961-1074`)은 B6a에서 처음 판 안에 선다 — 둘째 칸의 연출(금빛·종소리·업적 제목과 '별 하나의 빛을 두른 백성은…' — `UI/CosmicAscensionOverlayWidget.h:36-45`, `Tools/data/DT_Feats.json:4-5`) 바로 뒤에 '그들이 당신을 높이려 한다'·'당신을 신이라 부르면…'이 오는 차례를 글 감사가 본다(§8의 ①; 글을 고치면 `[B-6a5]` — 데이터 켜기 전).
- 선택지와 값은 바꾸지 않는다(`Exaltation` −1/+2, 안정도 +0.1, 경외 ±). `Exaltation`을 판정하는 곳은 B6a에도 없다(T5 문지기는 Phase 6+ — 결정 1); 보이는 곳은 방랑자 툴팁의 '높아짐 N'(`UI/CosmicTopBarWidget.cpp:458`)이다 — 받아들이고(+2) 양위하거나 폐위된 같은 그릇에게 이 값이 처음 0이 아니게 보인다(그릇은 `Install`에서만 새로 선다 — `Player/PlayerRoleSubsystem.cpp:181`; 손 확인 ⑤).
- 스키마 규칙 하나를 더한다(`Tools/data/schema.json` `DT_Encounters`, 깨지는 경우는 옛 행 — `Tools/test_data_schema.py`): **`PlayerRoleIs Ruler`를 지닌 행은 `PlayerTierAtLeast` 2 이상을 싣지 못한다** — 통치자의 개인 칸은 오르지 않는다(§1.1). 개인 칸을 올리는 블록(#1·#141)이 오면 그 블록이 이 규칙을 푼다(규칙의 설명 줄에 적는다).
- 통치자 기준값이 옮는다(둘째 칸 1,175 뒤 — §5.3). 첫 10분(틱 600까지)은 그대로다(B4의 첫 10분 문 'told by 600' — B4 헤더 `:361`).

### 3.2 알아챔의 칸 (`[B-6a5]` 글 — 결정 7-라)

- `FTranscendentDef`에 새 칸 `AwarenessTemplates`(`TArray<FText>`, 정의 — 저장 아님)를 더한다. `VoiceTemplates`는 울타리의 증언 하나만의 칸으로 남긴다(B8 — 새 #159; 스키마 `max_len 0` 그대로 `Tools/data/schema.json:605`). 주석(`Data/CosmicDefinitionRows.h:679-683`)을 둘로 가른다. 한 배열에 두 말을 두면 위치 키(규칙 8)와 끝에만 덧붙이는 규칙 때문에 나중에 가를 수도 없다.
- 침묵 행(`Tools/data/DT_Transcendents.json:3`)에 변형을 채운다(초안 셋 — 글 감사가 정한다). 스키마: 1인칭 칸의 공유 식(`:562`의 여섯 목록)에 `rebirth_words`(`:56`)·`called_away_words`와 새 목록 `words.awareness_words`(`같은`·`똑같`·`다시`·`알아보`·`알아본`·`알아봤`·`알아볼`)를 더한다 — `rebirth_words`의 '다시 '는 뒤 칸이 있어 '다시는'·'다시금'을, '알아보았'은 과거 꼴만이라 '알아본다'·'알아봤다'·'알아볼'을 지나 보내고 '같은'은 어느 목록에도 없다. 침묵 교리의 낱말 규칙(`Tools/data/schema.json:612-614` — 지금 `ClaimTemplates`·`Lure`만)에 `{"col": "AwarenessTemplates", "lacks_words": "silence_words"}`를 더한다(같은 입이다 — `docs/70:162`). `second_person`은 이 칸에만 허락한다(`docs/70:168` 끝). 슬롯 없음, 숫자 없음. 칸을 더하는 일이므로 스키마와 `Tools/test_data_schema.py`에 함께 적는다(규칙 8 — 규칙마다 깨지는 경우 하나: '같은 …'·'…알아본다'·'다시는 …'·'…고요…').
- 초안(B4 헤더 `:336`의 '같은 부름의 냄새가 난다. 너는 그것을 모른다.'에서): '같은'·'다시'·'알아보'는 쓰지 않는다 — 부름은 정체가 아니라 맡김이고(B5 결정 12-가) '같은'은 앞의 것을 전제해 새 그릇에게 '너를 다시 안다'로 읽힌다. **알아챔 줄은 그릇이 무엇을 지녔는지(맡겨진 것·부름·택함) 말하지 않고 위에서 맡긴 이를 가리키지 않는다 — 타락이 알아채는 것은 제 손이 닿은 곳을 다스리는 이다**('맡겨진'은 거룩한 질서의 수동이라 위의 맡긴 이를 타락의 입에 들이고 — 타락은 위를 '위·그 자리·그곳·아래'로만 가리킨다, `docs/70:162` — 맡김을 그 사람이 지닌 것으로 만든다 — B5 결정 12-가). 앞 판의 초안 '맡겨진 것의 냄새가 난다. 너는 그것을 모른다.'는 바꾼다. 후보(`[B-6a5]`의 글 감사가 정한다): '네가 다스리는 곳에도 나는 닿는다. 다 닿지는 못한다.' — 그 틱에 참이고(다스리는 성계에 주장이 섰다) 둘째 문장은 남은 자의 양보다(`docs/70:163`); 위 목록과 침묵의 낱말이 없다. 외 둘('냄새' 같은 낱말이 다시 들면 성도의 향으로 읽히지 않는지 — 메시아 신호의 눈).

### 3.3 스트링 테이블 (`ST_CA.csv` — 새 키는 끝에만; `[B-6a4]` 초안, `[B-6a5]` 감사)

| 키 | 지금(`Tools/data/ST_CA.csv`) | B6a(초안 — 글 감사가 정한다) |
|---|---|---|
| `Chronicle.Note.Rung` | `:133` '{Civ}…셋째 칸에 이르렀다 — 이 판은 여기까지다.' | '{Civ}\|hpp(이,가) 다섯째 칸에 이르렀다 — 이 판은 여기까지다.' — 집단 승천과 다른 질서가 **같은 줄**이다(비조소 — `crisis_and_transcendents.md:151` 끝의 '차이는 기록된 사건의 내용뿐') |
| `Chronicle.Note.RungNoCiv` | `:134` | '한 백성이 다섯째 칸에 이르렀다 — 이 판은 여기까지다.' — 그 백성이 사라졌거나 보는 이에게 보이지 않을 때(§4) |
| `Chronicle.Note.Clock` | `:135` | 그대로 — `{0}`은 기록의 `Tick` |
| `Chronicle.Provisional` | `:136` '시험판의 끝이다…' | 그대로 — **시계의 끝에만** 보인다(시계가 임시인 동안 참이다; 다섯째 칸·침묵의 끝은 시험판의 끝이 아니다) |
| `Chronicle.Rule` | `:145` '…셋째 칸에 오르거나 {0}에 이르면 멈춘다.' | '이 판은 {0}에 이르면 멈춘다.' — B6a에서 판 안에 실제로 오는 끝은 시계뿐이다(결정 13); 다섯째 칸의 문장은 그것이 닿는 B6b가 더한다 |
| `Chronicle.RuleRung` | `:146` | 읽지 않는다(노브가 0 이하면 규칙 줄이 없다 — 시계가 없으면 이 판에 오는 끝이 없다). 키는 B6b가 다섯째 칸의 문장으로 다시 쓴다 |
| `Chronicle.Soon`·`Chronicle.Until` | `:147-148` | 그대로(시계에만 — 결정 13) |
| 새 `Chronicle.Note.Vessel` | — | '{Player}\|hpp(은,는) 다섯째 칸에 서 있게 되었다 — 이 판은 여기까지다.'(그릇의 이름으로 된 3인칭 — B5 결정 12-다의 꼴; 사람이 칸에 '이르는' 능동은 스스로 붙잡는 구문이라 쓰지 않고 그 일 뒤에 서 있게 된 자리로 적는다 — `docs/70:129`, 규칙 11 '받은 꼴만'; 거룩한 질서의 수동이 없어 §3.6의 1도 지킨다; 닿지 않는 줄이라 감사만, 단독 승천의 신학과 함께 #163이 다시 본다) |
| 새 `Chronicle.Note.NoCandidate` | — | '그릇이 설 수 있는 별이 남지 않았다 — 이 판은 여기까지다.'(B5 헤더 `:187`의 초안 — 그 틱에 참; 성역을 빼지 않으며 사람이 남지 않았다고 쓰지 않는다 — #150의 조건) |
| 새 `Chronicle.Note.NoVessel` | — | '그릇이 없던 동안 이 판이 멈추었다.'(B5 결정 12-라의 이름 '그릇 없이 맞은 결말'을 화면에 제목으로 세우지 않는다 — 사실 한 줄; '끝이 … 왔다'는 끝을 오는 주어로 세우는 종말의 관용구라 쓰지 않고 판의 규칙 꼴 '멈춘다'로 — A4의 판정, `Tools/data/ST_CA.csv:145`; 시계·침묵 모두에 참) |
| 새 `Panel.ClaimMayFollow` | — | '이 위기가 풀리지 않고 만료하면 이 성계에 주장이 설 수 있다.'(§4 — 두려움이 아니라 결정의 꼴) |
| 새 `Panel.RebellionWatch` | — | '안정도 {0} 아래로 {1}주기째 — {2}주기가 이어지면 반란이 설 수 있다.'(UI 수만 — 규칙 11) |

### 3.4 스키마 (`Tools/data/schema.json`·`Tools/cosmic_data_schema.py`·`Tools/test_data_schema.py` — 새 규칙마다 깨지는 경우 하나)

- 문자열 접두어 `Chronicle.Note.`(새 줄): 지금의 `Chronicle.` 목록(`:62`)에 `holy_words`·`realm_names`·`silence_words`·`revelation_mimicry`를 더한다(이름·'고요' 없이). 접두어 `Chronicle.` 전체(까닭 줄·`Rule`·`RuleRung`·`Soon`·`Until`과 그 밖 — 맞는 접두어는 모두 걸린다, `Tools/cosmic_data_schema.py:411-412`)에는 새 목록 `words.holy_passive`(`맡겨졌`·`맡겨진`·`허락되`·`허락된`·`때가 찼`·`때가 차`·`때가 왔`·`끝이 왔`)를 더한다 — 거룩한 질서의 수동과 끝의 관용구; 앞의 넷에는 수동이 없어 §3.6의 1이 감사에만 기대던 자리다(지금 `Chronicle.` 줄 스물아홉에 0건 — '펼쳐졌다'는 들지 않는다, `Tools/data/ST_CA.csv:130`). 깨지는 경우 하나(`Tools/test_data_schema.py`). `afterlife_words`에 '승천'이 있어(`:51`) 설계 이름 '집단 승천'·'단독 승천'은 화면 글에 쓸 수 없다 — 그대로 둔다(결정 6-나).
- `Panel.` 두 새 키: `verdict`·`holy_words`·`silence_words`, 숫자는 UI 수(슬롯)만.
- `AwarenessTemplates`(§3.2 — 공유 식·`rebirth_words`·`called_away_words`·`awareness_words`, 침묵 교리의 `silence_words`), `VoiceTemplates`는 그대로 `max_len 0`.
- `DT_Encounters`의 통치자 행 규칙(§3.1).
- 엔딩 태그 다섯은 데이터 칸이 아니다(코드만 쓴다) — 스키마의 태그 목록에 들지 않는다.

### 3.5 노브

새 노브 없음. `ProvisionalEndTick` 6,000 그대로(`Tools/data/DT_Rules.json:55`, 범위 `Tools/data/schema.json:180`; 결정 2-나). 이름도 그대로 둔다 — 마지막 값을 B6b가 정할 때까지 그 시계는 임시다. 노브의 주석(`Data/CosmicDefinitionRows.h:1097-1101` — '…unless the ruled people reached the third rung', 'Nothing in the simulation stops at it')은 `[B-6a3a]`가 셋째 칸을, `[B-6a3c]`가 함수 이름(`ProvisionalEndOf` → `EndingOf`)과 '기록'을 고친다. **`docs/30`**(생성 문서)은 주석을 싣지 않고 노브마다 **읽는 코드**를 싣는다(`Tools/make_config_reference.py:20-28` — 테스트와 주석 밖): `[B-6a3c]`가 컨트롤러의 시계 자르기(`World/CosmicGameMode.cpp:864` — 그 파일의 하나뿐인 읽기)를 지우면 `ProvisionalEndTick`의 읽는 코드가 셋에서 둘이 되어(`docs/30_CONFIG_REFERENCE.md:58`) 러너의 첫 문서 검사(`make_config_reference.py --check` — `Tools/run_checks.ps1:45`)가 깨진다 — `[B-6a3c]`가 `python -I Tools/make_config_reference.py`로 다시 만든다(`[B-6a3a]`에서는 바뀌지 않는다).

### 3.6 글의 규칙 (초안 — 글 감사가 정한다)

1. 까닭 줄은 **판의 규칙**으로만 — '…에 이르렀다 — 이 판은 여기까지다.'(백성이 주어일 때; 그릇이 주어면 '…에 서 있게 되었다' — §3.3) 거룩한 질서의 수동('맡겨졌다'·'허락되었다'·'때가 찼다')을 까닭 줄에 쓰지 않는다 — 까닭에 묶으면 거룩한 질서가 끝의 조건이 된다(규칙 11; A4의 판정 — B5 헤더 `:187`의 꼴). 스키마의 `holy_passive`가 `Chronicle.` 전체에서 지킨다(§3.4).
2. 모든 끝은 같은 쪽·같은 색·같은 고정 줄이다(`UI/CosmicChronicleWidget.cpp`의 고정 줄 셋 — 무엇이 끝을 냈든 같다). 패배 화면·경고·훈계 없음. 다른 백성의 다섯째 칸을 '패배'로, 그릇의 죽음을 그 끝의 까닭으로 말하지 않는다.
3. '침묵'·'고요'·'쉼'을 끝의 글에 쓰지 않는다(#150; `silence_words`).
4. 알아챔의 줄은 판에 한 번(플래그는 초월자마다 — §2.11), 2인칭은 그 줄만, 반쪽 진실이지 거짓이 아니다(`docs/70:162`의 1인칭 규칙 — 제 이름·호칭 없음, 이길 수 없음을 안다); 그릇이 지닌 것(맡겨진 것·부름·택함)을 말하지 않는다(§3.2); 서술자의 만료·주장 줄 뒤에 선다(§2.11).
5. 주장의 경고와 반란 문턱은 결정을 돕는 사실이다 — '위험하다'·'곧 무너진다' 같은 두려움의 낱말 대신 조건과 결과.

## 4. 표현 계층 (`[B-6a4]`)

- **연대기 쪽**: 서명은 `[B-6a3c]`가 기록으로 바꾼다(§2.7 — `UCosmicChronicleWidget::Open(const FCosmicEndingRecord& End, bool bAtOnce)`; 빈 기록 = 개발용 미리 보기 — 까닭·알림·'새 판' 없음). 이 커밋은 줄을 고른다: `NoteOf`가 종류마다 §3.3의 줄을 고르고, `bNoVessel`이면 그 아래 `Chronicle.Note.NoVessel`. 시험판 알림은 `Ending.Clock`일 때만(`UI/CosmicChronicleWidget.cpp:350-357`을 고친다).
- **까닭 줄의 백성 이름**: 집단 승천은 다스리던 백성이라 늘 부른다(만났다). 다른 질서는 **보는 이에게 보이는 백성일 때만** 이름을 부르고 아니면 `RungNoCiv` — 앎은 `CosmicMapLook::KnownViewOf`의 답(`World/CosmicMapLook.h:322` — `IsShownPeople`, `:316`)으로만 읽는다(규칙 14 — 격차 규칙을 넷째 자리에 베끼지 않는다). B6a에서는 테스트로만 닿는다; 보이지 않는 백성의 엔딩 글 전체는 B6b(#162).
- **엔딩의 화면 제목은 없다**(결정 6-나) — 쪽의 머리는 지금처럼 생애 장과 고정 줄이고, 다른 것은 까닭 줄 하나다.
- **상단 바**: 시계 툴팁의 규칙 줄은 시계만(§3.3 — `UI/CosmicTopBarWidget.cpp:533-544`); 노브가 0 이하면 규칙 줄 없음. 끝 예고(`:870-885`)는 그대로 — 끝나기 600주기 전부터 모두에게 'Soon', 몸을 입은 방랑자에게 'Until'. 통치자도 이제 시계까지 가지만 통치자의 끝은 툴팁의 규칙 줄이 미리 말한다(바꾸지 않는다 — 손 확인 ①이 본다).
- **그릇 없이 맞은 결말**: 쪽의 차례는 §2.9.
- **주장의 경고 줄**(결정 14): 다스리는 백성의 정착된 성계에 위기가 서 있고 그 위기가 답 없이 만료하면 주장이 설 수 있는 자리면 성계 패널의 위기 줄 아래와 그 위기의 툴팁에 `Panel.ClaimMayFollow`. '설 수 있는가'는 순수 함수 둘(시뮬 코드라 xhigh — §8): ① 가칭 `CosmicHazards::ClaimRuleHoldsOf(State, Index, Tables, Rules, SystemIndex)` — 주장 규칙의 결정적인 셋(주장자가 있다·그 영향권이 차지 않았다·그 성계가 주인의 마지막 미편입 정착 성계가 아니다 — `Sim/GalaxySimulationSubsystem.cpp:318-323`)을 옮긴 것이고 **시뮬의 `TryPlaceClaim`도 이것을 부른다**(비트 그대로가 증거; 화면이 규칙을 베끼지 않는다 — B4·B5의 교훈). ② 가칭 `CosmicHazards::ClaimMayFollowOf(같은 인자)` — 화면이 읽는 답: 만료 앞에서 이미 정해진 문을 먼저 본다 — 주인이 그 위기의 답인 칙령을 지니면 만료가 해결이 되어 주장 규칙을 돌지 않고(`CosmicEdicts::HeldAnswerOf` — 만료가 읽는 같은 함수, `:2402-2411`), 선 위기가 `Hazard.PreSupernova`면 은혜가 마지막 성계를 지키거나(`:2424-2434`) 파괴가 정착을 지워 규칙이 거절하며(`:314`), `Hazard.Claim`이면 그 만료가 성계를 영향권에 들이고 `ClaimantOf`는 성계를 쥔 영향권의 초월자를 고르지 않는다(`Sim/CosmicHazards.h:115-116` — 편입이 서지 않으면 같은 세 문이 규칙도 막는다, `:334-352`; 타락이 하나인 B6a에서 정확하고 — `FallenTranscendentCount` 1, `Tools/data/DT_Rules.json:27` — 그 값이 늘면 그 블록이 이 가지를 다시 본다) — 셋이면 거짓, 아니면 ①의 답. 규칙 13의 단일 창구(결정 20)에서 ①은 게임과 화면이 나누는 주장 규칙의 한 집(`VesselAtRiskOf`의 꼴), ②는 화면이 읽는 답의 한 집(만료의 차례 `Sim/GalaxySimulationSubsystem.cpp:2402-2434`를 따르므로 `UI.ClaimMayFollow`의 정해 둔 답이 지킨다). 신앙의 수(`ClaimNumber` — `:324-329`, 지금 늘 지난다)는 화면이 알 수 없으므로 '설 수 있다'로 쓴다.
- **반란 문턱의 표시**(결정 14): 다스리는 백성의 수도 패널에 `Panel.RebellionWatch` — 안정도가 문턱 아래로 이어진 틱(`LowStabilityTicks` — `Core/GalaxyState.h:315-319`)과 반란 행이 기다리는 틱 수를 순수 함수 하나(가칭 `CosmicHazards::RebellionWatchOf(State, Tables, CivIndex)`)로 읽는다. **안정도의 문턱 `{0}`은 시뮬이 세는 상수 하나다**: 지금 파일 안의 `LowStabilityThreshold` 0.3(`Sim/GalaxySimulationSubsystem.cpp:104-105`, 세는 곳 `:1818`)을 Sim 헤더(가칭 `CosmicHazards::LowStabilityThreshold`)로 옮겨 경제 단계와 이 함수가 함께 읽는다 — 조건 `CivStabilityBelowFor`는 값을 읽지 않고 키(틱 수)만 읽으며(`Events/CosmicEventSubsystem.cpp:454-459`) 행의 0.3은 스키마가 상수와 같게 묶을 뿐이다(`Tools/data/schema.json:1064`). 틱 수 `{2}`와 `GalaxyTickAtLeast`는 행(`Hazard.Rebellion`을 태그로 찾는다 — `Tools/data/DT_Hazards.json:203-215`)에서 읽는다(행을 바꾸면 화면이 따라온다). 문턱 아래일 때만 서고, 수도에 `Hazard.Rebellion`이 이미 서 있으면 내지 않는다(위기 줄이 말한다 — 세는 칸은 반란이 선 뒤에도 늘어 '{1}주기째'가 '{2}'를 넘는다). 화면 코드에 값을 적지 않는다.
- 셋째 칸의 문(영적 시야·×25)은 고칠 것이 없다(§1.1). 판이 길어지면 ×25에서 새 위기의 300주기가 12초다(#96·#100 ③) — 손 확인 ①.

## 5. 커맨드렛과 측정

### 5.1 새 줄과 인자

- **새 줄 `CosmicBalance ending:`**(`[B-6a3c]`) — 러너가 글자로 견주는 세 줄(`seed`·`ruler:`·`wanderer:` — `Tools/run_checks.ps1:115`·`:119`·`:125`) 밖이다(B4 `pressure`·B5 `calling`의 꼴). 칸(기록·셋째 칸 뒤 계절의 길이와 결정·AI가 처음 셋째 칸에 오른 정확한 틱·분야마다 연구 1.0의 틱·알아챔·끝 전의 1인칭 주장 줄과 알아챔 줄·셋째 칸 뒤 다스리는 성계의 주장·#132·#135·#142·#145의 셈·반란 문턱의 시드 고르기·영압 카드의 비축·`petition_exalt`)의 목록은 노트 §2.1. 셈은 규칙을 베끼지 않고 게임 코드가 읽는 함수와 기록을 읽는다. `pressure:` 줄(`:2468`)의 '(by end …)' 칸은 이제 시계 전을 뜻한다 — B4의 '끝 뒤' 수는 새 '셋째 칸 뒤' 가름과 견준다.
- **`end check`**(`Tests/CosmicBalanceCommandlet.cpp:2484-2490`): 셋째 칸이 박힌 지금의 비교는 `[B-6a3a]`가 지우고, `[B-6a3b]`가 새 뜻으로 세운다 — 경계마다 `EndingOf`를 부르고, 그것이 처음 선 경계에서 기록의 네 칸(종류·틱·`CivID`·`bNoVessel`)이 같아야 한다(열째 단계 앞에 쓴 결함은 주로 `bNoVessel`·종류에서 보인다). 어긋나면 `UE_LOG(LogCosmic, Error, …)`와 `Result = 1` — 지금의 'A measure, not a gate'(`:2484-2486`)와 달리 정당한 예외가 없는 불변식이라 깨질 수 있는 확인이다(엔진이 오류 줄을 끝 코드로 바꾸고 러너의 `Step`이 그 코드에 멈춘다 — `Tools/run_checks.ps1:37`).
- `end:` 줄(`:2442`)은 기록을 읽게 고친다(첫 종류·틱 — `[B-6a3c]`; 칸 끝 칸 'rung at'은 `RungTick`과 함께 `[B-6a3a]`가 지운다). B5 `calling:` 줄의 '(by a new people %d)'(`EndByNewPeople` — `:1349`, 줄 `:2424`)는 셋째 칸 끝과 함께 뜻을 잃으므로 칸을 뗀다(`[B-6a3a]`; 백성이 다스리던 백성인가는 `ending:`이 싣는다 — 같은 글자 아래 다른 사실을 싣지 않는다; `docs/40`의 커맨드렛 절에 적는다).
- **`-ExaltAsBefore=1`**(`[B-6a6]` — 측정 전용, 자동화 테스트가 든 빌드에서만; `-WithoutDeath`의 문과 같은 꼴 `:1107-1118`; 해시 판·러너 인자에는 넣지 않는다): `petition_exalt`의 조건을 옛 `PlayerTierAtLeast 2`로 되돌린 테이블(테스트 도우미 `CosmicTests::WithExaltAsBefore` — `WithoutDeath`의 꼴 `Tests/CosmicTestHelpers.h:251`). 데이터 커밋의 짝이다.
- B6b가 쓸 측정 전용 스위치(`AIFourthRungCap`을 덮는 것, 셋째 칸 위 행을 뺀 테이블)는 B6b가 짓는다 — `-Rules`는 `DT_Galaxies`를 덮지 못한다.
- `-SaveAt`의 문(게임과 같은 꼴의 판에서만 — `:1070`)은 그대로다; `-ExaltAsBefore`도 그 문의 '아닌' 목록에 더한다.

### 5.2 묶음과 목표

B5의 일하는 법 그대로(B5 헤더 §5.2): 결과는 **30건 이상일 때만 판단**하고 모자라면 '재지 않음 — 잰 N'; 몫에는 95% 구간(Wilson); 판 단위의 수는 짝 차이; 판의 끝까지와 끝 뒤를 따로; 원인은 `-Rules` 탐침 뒤에만; **미리 셀 수 있는 목표는 데이터를 켜기 전에 셈한다**(묶음 0에서 미리 세고 묶음 1에서 같은 규칙으로 판단한다; 기대 N이 30 미만이면 '재지 않음 예상 — 잰 N'을 미리 적는다); **같은 세계는 한 번만 센다**(어디까지가 같은 세계인지 — 노트 §2 '한 번만 센다'); **모든 묶음의 로그에 `end check` 줄 0**(§5.1).

| # | 묶음 | 커밋 | 정하는 것 / 미리 적은 판단 |
|---|---|---|---|
| 0 | **첫 측정**(장치, 데이터는 B5 그대로): 통치자 R(`-Answer=1 -Build=1 -Focus=1 -Edict=1 -Diplomacy=1 -TellAll=0`) 30 × 6,000(시드 1~30); 느린 통치자 R의 `-Focus=0` 30 × 6,000(1~30 — B2 뒤 처음); 방랑자 R + `-Abdicate=1200 -Wander=1` 60 × 6,000(1~60); **시험 칸** — B4의 시험 묶음 그대로(`DECISIONS_LOG.md:28`: R + `-Rules=FallenCoreMinLanesFromHome:1 -LeaveNearSphere=1`, 20,000틱) × `-PlayerPolicy=Faithful·Reckless·Cautious` × `-Guard=0·1`, 시드 1~60(360판 — 정책의 중점·칙령·외교는 시계 6,000에서 멈추고 카드의 답과 지킴은 이어 간다(`World/CosmicRulerPolicy.cpp:493`·`:518`·`:672`·`:729` — B4의 끝 뒤와 같은 꼴); 셈은 셋째 칸 뒤·끝 전(게임의 계절)과 끝 뒤를 가른다 — B4는 끝 = 셋째 칸에서 갈랐다); 입력 없는 60 × 6,000(1~60 — 경주의 입력만; 세계는 B5 그대로라 다른 칸은 다시 세지 않는다). **참고 칸**(판단 없음 — B6b의 시계 입력): R + `-Rules=ProvisionalEndTick:18000` 30 × 18,000 | `[B-6a3c]` | ⓐ 계절의 길이(R·느린 R — 중앙값과 사분위); ⓑ **계절의 결정 밀도**(깨질 수 있는 확인): 다스리는 셋째 칸 틱과 기록의 틱 사이의 결정을 600틱마다 센 수의 판 중앙값 ≥ 1(비전의 '300~600틱마다 결정 1' — `docs/00_VISION.md:20`; R과 느린 R 따로; 모자라면 그 수와 가장 긴 틈을 결정 2-라의 잣대와 함께 [개발자]에게 다시 묻는다 — B6b 헤더 전, 노브는 B6a에서 바꾸지 않는다)과 그 계절의 결정 없는 가장 긴 틈(적기만); '끝까지의 결정 중앙값'은 적기만 한다(셋째 칸까지 이미 28 — B4 헤더 `:89`; 끝이 늦어지면 줄지 않아 깨질 수 없다; 결정 2-라의 문장이 읽는다); **#136** 끝 전에 1인칭 주장 줄(`Log.Claim` — §5.1의 셈; 알아챔 줄은 #140으로 따로)이 말해진 R 판의 몫 — ≥ 10/30이면 닫는다('계절이 목소리를 들인다'), 모자라면 그 수와 함께 B6b(시계의 값)로; **#140** 알아챔이 끝 전에 선 판의 수(판에 한 번이라 빈도 목표 없음 — 적기만); **#135** 주장 카드의 조건이 처음 선 경우(한 번 세기) ≥ 30건이면 칸을 잃은 몫 ≥ 10%일 때 그 수와 함께 B6b·B7 헤더로(버퍼는 앎의 저장 필드 둘 — 헤더 먼저), < 10%면 닫는다; 30건 미만이면 '재지 않음 — 잰 N'으로 열어 둔다; **#142** 시험 칸 × 지킴의 401~550 띠 ≥ 30건이면 `Faithful`의 나쁜 결말 몫의 95% 아래 끝이 0.25 이상이면 닫고, 아래면 [개발자]에게 그 수와 함께 묻는다(B6b 헤더 전) — 노브는 B6a에서 바꾸지 않는다; 기대 N은 B4의 23건(같은 칸 60 × 20,000 — `OPEN_QUESTIONS.md:111`)에서 계절의 몫(약 3,600 / 17,600)으로 약 5, 판 전체로 약 23 — 둘 다 30 미만이라 **'재지 않음 예상 — 잰 N'**; **#132** 셋째 칸 뒤·끝 전 깨어남 지연의 몫(띠 10~50%)과 영향권 안 소모 — ≥ 30건에서 띠 밖이면 노브 바꿈을 그 수와 함께 B6b로(바꾸는 커밋은 같은 커밋에서 철야 무게 상한을 다시 센다 — `Tests/CosmicDataTests.cpp:2234`); **#144** 카드가 온 틱의 비축 — 적고 B7로; **#145** 저항 뒤 함대가 다 자라기 전에 다스리는 백성이 받은 공성(§5.1의 새 셈) ≥ 30건이면 `-Rules=FallenCoreMinLanesFromHome:1;ResistFleetFactor:0.45` 짝(커맨드렛은 `-Rules` 하나를 `;`로 잇는다 — `Tests/CosmicBalanceCommandlet.cpp:943`)을 돌려 편입 몫을 적고 값은 B7 헤더로 — 기대 N은 B4의 0건(`OPEN_QUESTIONS.md:113`)이라 **'재지 않음 예상'**; **경주의 입력**(B6b의 미리 셈): 첫 AI 셋째 칸 대 다스리는 셋째 칸의 앞섬 분포(정확한 틱), 분야마다 1.0 틱; **`petition_exalt`의 미리 셈**: 칸마다 바닥과 다툼의 두 수(정의는 노트 §2 — 묶음 1의 ⓓ가 견준다) |
| 1 | **데이터 켬**(`petition_exalt`): 묶음 0의 R·느린 R·방랑자·시험 칸과 시드 + 같은 빌드의 `-ExaltAsBefore=1` 짝 | `[B-6a6]` | **깨질 수 있는 확인**: ⓐ 입력 없는 세 줄(3,000·20,000·해시 판) — 세계 해시는 움직일 수 없다(정책 없는 판은 답하지 않는다); 전체 해시는 그 카드가 놓이지 않을 때만 그대로다(놓이면 앎의 대기 칸이 든다 — `Sim/GalaxySimulationSubsystem.cpp:1564`) — 시드 42의 입력 없는 그릇은 1,072에 폐위된다(`seed42.json:13`); 그 전에 그 백성이 둘째 칸에 서지 않았으면 비트 그대로, 어긋나면 그 틱을 탐침한다; ⓑ 짝마다 둘 — (i) 켠 쪽의 `petition_exalt`가 온 틱 = 끈 쪽의 `RuledT2Tick`(`Tests/CosmicBalanceCommandlet.cpp:2204`) 또는 둘째 칸 이상 백성의 재위 첫 틱, (ii) 그 틱보다 앞선 모든 CSV 행이 두 쪽에서 같다 — 처음 갈린 행까지의 거리를 적는다(CSV는 100틱 표본이고 — `:54` — 답이 움직이는 `Exaltation`·경외는 CSV 칸이 아니라 처음 갈린 행은 카드보다 늦을 수 있다); 어긋나면 그 시드를 적고 그 틱을 탐침한다; ⓒ `-ExaltAsBefore=1`이 바로 앞 커밋(`[B-6a5]`)의 다섯 기준값 줄을 글자 그대로 다시 낸다; ⓓ 칸마다 바닥 ≤ 켠 쪽에 카드가 온 판의 수 ≤ 바닥 + 다툼(묶음 0의 두 수 — 다툼의 판은 뽑기가 고르므로 같음으로 견주면 잘못 없이 깨진다; 상시 카드는 조건이 처음 선 틱에만 오고 칸이 차면 그 머묾 동안 오지 않는다 — `Events/CosmicPetitions.cpp:341-343`). **판단**(묶음 0의 규칙 그대로 — 이 묶음이 B6a의 최종 수): 위 #135·#136·#142·#132·#144·#145; 방랑자 즉위·복귀 ≥ 44/60·떠난 뒤 중앙값 ≤ 1,800(B1의 지킴 — B5 결정 6의 ②); **적기만**: 카드의 답, 짝의 셋째 칸 틱·반란·폐위의 차이 |
| 2 | 기준값의 짝(시드 42 다섯 판) | 커밋마다 | §5.3 |

- **재지 않음 — 미리 적는다**: #129(B6a는 입력 없는 세계를 바꾸지 않는다 — 넘김의 3/4 경계는 보유 상한 7·10이 쓰이는 B6b에서 다시 잰다; 그 행의 '임시 끝이 바뀔 때'는 정책 판의 끝만 옮긴 B6a에는 걸리지 않는다); 집단 승천·다른 질서·단독 승천·침묵(테스트로만 — 0 기대); '도약이 일으킨 다른 질서 < 15%'(`docs/40:460` — B6b); 단계 시간(장치는 틱마다 술어 하나 — 기록이 선 뒤에는 부르지 않는다; 재지 않음 — 구조).
- 셈의 세부(구간·한 번 세기·띠의 경계)는 노트 §2.

### 5.3 옮을 기준값 (`Tools/baselines/seed42.json`)

| 기준값 | `[B-6a1]` | `[B-6a2]` 필드 | `[B-6a3a]` 셋째 칸 뒤 | `[B-6a3b]` 기록 쓰기 | `[B-6a3c]` 장치 | `[B-6a4]` 표현 | `[B-6a5]` 글 | `[B-6a6]` 데이터 |
|---|---|---|---|---|---|---|---|---|
| 입력 없는 3,000 | 그대로 | 두 해시 | 그대로 | 그대로(끝 없음 — 2,933의 죽음 뒤 공백기, `seed42.json:13`) | 그대로 | 그대로 | 그대로 | 그대로 |
| 입력 없는 20,000 | 그대로 | 두 해시 | 그대로 | 두 해시 — 기록만(6,000의 시계; 세계는 그대로) | 그대로 | 그대로 | 그대로 | 그대로 |
| 통치자 3,000 | 그대로 | 두 해시 | 세계 — 정책이 2,375 뒤로 이어 간다; 줄의 'by end'가 바뀐다(끝이 3,000 안에 없다); 알아챔 플래그(주장이 3,000 전에 다스리는 성계에 서면) | 그대로 | 그대로 | 그대로 | 전체 해시(알아챔 줄이 3,000 전에 말해질 때만) | 세계(둘째 칸 1,175의 카드) |
| 통치자 20,000 | 그대로 | 두 해시 | 세계(2,375~6,000) | 두 해시 — 기록(6,000) | 그대로 | 그대로 | 전체 해시('told' 칸) | 세계 |
| 방랑자 6,000 | 그대로 | 두 해시 | 세계(그 뒤 다스린 백성의 셋째 칸이 끝이 아니다 — §1.2) | 두 해시 — 기록(6,000) | 그대로 | 그대로 | 전체 해시(말해지면) | 세계(1,175의 카드는 1,200의 양위 전) |
| 해시 판 둘 | 그대로 | 둘 다 | 입력 없는 판 그대로, 정책 판은 통치자 3,000과 함께 | 그대로(3,000틱) | 그대로 | 그대로 | 정책 판은 통치자 3,000과 함께 | 같다 |

- **`[B-6a3a]`의 깨질 수 있는 확인 넷**(결정 15): ⓐ 입력 없는 3,000·20,000과 입력 없는 해시 판이 비트 그대로; ⓑ 다섯 판 모두 `-PastEnd=1`을 준 CSV의 결정적 칸이 `[B-6a2]`의 `-PastEnd=1` 판과 판 전체에서 같다(세계를 바꾸는 길은 정책이 끝을 읽는 자리뿐이라는 증거 — 알아챔 플래그는 CSV 칸이 아니다); ⓒ 통치자 3,000·통치자 20,000·방랑자 6,000(스위치 없이)의 CSV가 `[B-6a2]`의 같은 판 `-PastEnd=1` CSV와 틱 < 6,000의 모든 행에서 같다(셋째 칸 끝이 사라졌고 두 정책의 끝 읽기가 그 밖을 바꾸지 않는다는 증거 — 6,000의 행은 정책이 표본 앞의 경계에서 끝을 읽어 갈릴 수 있다, `Tests/CosmicBalanceCommandlet.cpp:1671-1672` 대 `:2213`); ⓓ **옮김의 귀속**(탐침 — 커밋하지 않는다): 이 커밋은 정책 판의 해시를 두 까닭(셋째 칸 끝을 뗌·알아챔 플래그)으로 옮기고 플래그는 CSV 칸이 아니어서 ⓑ가 플래그를 보지 못한다 — 플래그를 쓰는 한 줄을 끈 같은 빌드에서 ⓑ의 `-PastEnd=1` 다섯 판의 **두 해시**가 `[B-6a2]`의 `-PastEnd=1` 판과 비트 그대로임을 보인다(그러면 켠 빌드의 해시 차이는 플래그뿐이다; 탐침 패치는 변이 패치처럼 `Tools/mutations/B6a/`에 두고 README에 한 줄).
- **`[B-6a3b]`의 확인**: 다섯 판의 CSV 결정적 칸이 `[B-6a3a]`와 판 전체에서 같고, 3,000틱 줄 둘과 해시 판 둘은 비트 그대로 — 옮는 것은 6,000에 닿는 세 판의 두 해시뿐이다(기록; B3 `[B-3a]`의 꼴).
- **`[B-6a3c]`의 확인**: 다섯 기준값 줄과 해시 판 둘이 **비트 그대로**(`WORKPLAN.md:61`의 '장치(기준값 비트 그대로)' — 읽는 곳이 시계만 답하던 옛 함수에서 시계의 기록으로 옮아도 같은 경계에 같은 답이고, `RunLeap`·`AdvanceLeap`·침묵의 래치는 게임에만 있다 — 커맨드렛과 해시 판은 `RunHeadless`로만 걷는다 — `RunHeadless(1)` `Tests/CosmicBalanceCommandlet.cpp:1330`·`World/CosmicHashRun.cpp:128`, 정책 없는 해시 판은 `RunHeadless(Step)` `:141`). 짝이 어긋나면 그 틱을 탐침한다.
- `[B-6a5]`의 세계 해시는 다섯 모두 그대로여야 한다(말하기만 — B4 `[B-4d1]`·B5 `[B-5d1]`의 문).
- 옮은 줄은 `-UpdateBaseline`로 다시 적고 `note`에 까닭을 적는다(B5 `[B-5d2]`의 꼴). 쿠킹한 빌드의 해시 비교(ⓓ — `Tools/package.ps1`)는 마일스톤 B의 끝에 한 번(`STATUS.md:25`).

## 6. 테스트 (새 단언은 변이로 한 번씩 깨뜨린다 — 패치는 `Tools/mutations/B6a/`, README에 한 줄씩; 깨끗한 판이 먼저 통과하는지 본 뒤에)

저장 왕복은 러너의 smoke 필터에 들도록 `Sim.Save` 아래에 둔다(B5 헤더 §6의 꼴).

- `Sim.NotablesFromEventsOnly`(`[B-6a1]` — 고침, `Tests/CosmicHazardTests.cpp:2983-3033`): 죽음을 뺀 테이블(`:2999`)을 걷어 **게임 데이터**로 돈다. 허용은 보수적으로 — 기록의 백성이 **생애의 백성이 된 첫 생애의 `BornTick`보다 앞선** Severity 1 줄이고 그 틱이 로그에 남은 가장 이른 틱 이하면 떠났을 수 있다(계승이 그 백성을 뒤늦게 생애의 백성으로 만든 경우 — #118의 받아들인 잃음; 출생 백성은 `FLifeRecord.BirthCivID`·`BornTick`, 다스린 백성은 그 생애의 `BornTick`으로 자른다 — 즉위 틱은 저장되지 않는다). 변이: 지금의 허용(끝의 `PeoplesOfEveryLife` — `:3016-3026`)으로 되돌리면 깨진다; 허용을 빼면 깨진다. `docs/40:284`의 행을 같은 커밋에서 고친다.
- `Sim.SaveEndingRecord`(`[B-6a2]`): 선 기록의 왕복(종류·틱·백성·그릇 없음); `FCosmicEndingRecord()`의 기본값 넷이 얼린 값이다(변이 — 기본값 하나를 바꾸면 깨진다); 빈 기록의 상태는 왕복 뒤 빈 기록이고 해시가 같다.
- `Sim.HashCoversEnding`(`[B-6a2]`): 기록을 세우면 두 해시가 모두 바뀐다(변이 — 세계 해시의 빼는 목록에 `Ending`을 넣으면 깨진다).
- `Sim.EndingOf`(`[B-6a3b]` — `Sim.ProvisionalEnd`(`Tests/CosmicChronicleTests.cpp:93`)의 경우는 옛 함수와 함께 `[B-6a3c]`에서 이리 옮긴다): 종류 다섯과 없음; 차례(변이 — 다른 질서를 집단 승천 앞에 두면 깨진다); `CivID`의 규칙; `bNoVessel`; **셋째·넷째 칸의 `PeakTier`는 끝이 아니다**; 앎·로그·연대기·말하기 스트림을 바꿔도 같은 답(변이 — `MetCivs`를 읽으면 깨진다); 노브 0 이하면 시계 없음.
- `Sim.EndingRecordOnce`(`[B-6a3b]`): 술어가 처음 선 틱에 열째 단계 뒤 한 번 — 시계의 틱에 죽음이 적힌 판은 `bNoVessel` 참(변이 — 열째 단계 앞에 쓰면 깨진다); 다섯째 칸에 오른 틱에 그릇이 죽은 판은 `bNoVessel` 거짓(변이 — 예외를 빼면 깨진다); 뒤에 다른 술어가 서도 덮어쓰지 않는다(변이 — 덮어쓰면 깨진다); 시뮬 스트림이 움직이지 않는다(같은 시드에서 노브 6,000과 0의 판이 `RngState`까지 같다 — 변이: 쓰기에서 한 수를 뽑으면 깨진다).
- `Sim.EndingInert`(`[B-6a3b]`): 같은 시드의 시계 6,000 판과 시계 0 판이 끝 뒤 1,000틱에서 기록만 다르다(두 번째의 기록을 첫째와 같게 놓으면 세계 해시가 같다 — 변이: 어느 단계가 기록을 읽어 무언가를 건너뛰면 깨진다).
- `Sim.RunLeapStopsAtEnding`(`[B-6a3c]`): 공백기 안의 시계와 다른 질서(테스트에서 백성의 칸을 다섯째로 놓는다)에서 `RunLeap`이 기록의 틱에서 서고 경계가 하나; 기록이 있은 뒤의 `RunLeap`·`RunHeadless`·`StepOnce`는 틱을 돌린다(변이 — `RunHeadless`가 끝에서 서면 깨진다).
- `Calling.LeapDeterminism`(고침, `[B-6a3c]` — `Tests/CosmicLeapTests.cpp:219`): 스위치를 끄고 5,500에 죽인 판에서 `AdvanceLeap`이 6,000에서 멈춘 `RunLeap`을 한 번 더 불러 이어 가고, 마지막 틱의 전체 해시가 한 틱씩 돈 판과 같다(§2.5의 약속이 멈춤을 건너서도 선다 — 변이: 둘째 부름을 빼면 남은 도약이 남아 깨진다).
- `UI.LeapBoundary`(고침, `[B-6a3c]` — `docs/40:253`, `Tests/CosmicLeapTests.cpp:467`): 시계의 멈춤이 기록에서 온다; 스위치가 꺼진 세션은 남은 도약을 다 넘기고 래치하지 않는다; 침묵의 판은 스위치가 켜져 있으면 그 틱에 래치하고, 꺼져 있으면 지금처럼 시간을 돌려주고 래치하지 않는다(B5의 단언 그대로 — `:462`); 시뮬이 거절한 도약(핸들러 안)은 아무것도 돌지 않고 `AdvanceLeap`이 거짓을 돌려 되풀이하지 않는다(§2.6).
- `UI.Epitaph`의 (11)(고침 — 뜻이 바뀐다, `[B-6a3c]`; `Tests/CosmicCallingPagesTests.cpp:836-865`): 지금은 '침묵의 도약은 끝까지 달린다'(`:853` — `Tick == EndTick`, 래치 `Clock`, 다이제스트는 끝까지, 까닭 줄 `Chronicle.Note.Clock` `:861-862`)를 단언한다. B6a에서 침묵은 계승의 틱(`EndTick − 100 + 40`)의 엔딩이다 — 그 틱에서 `Ending.NoCandidate`로 래치하고 다이제스트는 그 틱까지; 까닭 줄은 `[B-6a3c]`에서 §2.7의 임시 키(`RungNoCiv`)를, `[B-6a4]`에서 `Chronicle.Note.NoCandidate`와 그 아래 `Chronicle.Note.NoVessel`을 단언한다(`UI.Chronicle`과 함께). 꺼진 스위치의 쪽(`:866-885`)은 그대로다(§2.6 — 둘째 `RunLeap`은 남은 도약 0이라 아무것도 돌지 않고 `LeapOfThePages`가 시간을 돌려준다).
- `UI.RunEnd`(고침, `[B-6a3c]` — `Tests/CosmicChronicleScreenTests.cpp:144`)·`Sim.SaveTriggers`(고침): 래치·`MarkOf`·까닭 `Ended`가 기록을 읽는다; 기록이 선 상태를 로드하면 래치한다; 스위치를 끄면 래치하지 않는다.
- `Sim.SilenceEnds`(`[B-6a3b]` 기록, 래치는 `UI.LeapBoundary`): `LeaveNoCandidate`의 판이 계승의 틱에 `Ending.NoCandidate`·`bNoVessel` 참; 그 뒤 계승을 다시 시도하지 않는다(B5 그대로).
- `Sim.SaveOldEndFlows`(`[B-6a3c]` — 결정 2-다; smoke 필터 `Sim.Save`에 들도록 이 이름 — `Tools/run_checks.ps1:59`): 기록 없이 `PeakTier` T3인 상태를 로드하면 흐르고(로드 직후 래치 없음, 다음 틱에 기록 없음) 6,000에 시계로 끝난다(상태를 시계 몇 틱 앞에 놓는다 — smoke 필터의 시간); 기록 없이 `Tick` 6,200인 상태는 6,201에 시계로 끝난다.
- `Sim.AwarenessOnce`(`[B-6a3a]` — 결정 7의 답대로): 셋째 칸 이상의 다스리는 백성의 정착 성계에 처음 주장이 선 틱에 그 주장자의 플래그가 선다; 둘째 칸의 백성·방랑자·셋째 칸의 틱만으로는 서지 않는다(변이 — 칸의 문을 빼면 둘째 칸에서 서서 깨진다); 같은 틱에 앞서 폐위를 기록한 반란 만료와 죽음을 기록한 각성 만료가 놓은 주장은 플래그를 세우지 않는다(변이 — `PendingEffect`의 문을 빼면 둘 다 깨진다); 타락 행 둘의 표에서 둘째 초월자의 첫 주장은 제 플래그를 세운다; 계승을 넘어 남고 다시 쓰지 않는다; 앎을 바꿔도 같은 답; 시뮬 스트림 그대로.
- `UI.Chronicle`(고침, `[B-6a4]`): 종류마다의 까닭 줄과 그릇 없음의 줄, 시험판 알림은 시계에만, 다른 질서의 백성은 보일 때만 이름(변이 — 늘 부르면 깨진다), 규칙 줄은 시계만.
- `UI.ClaimMayFollow`(`[B-6a4]`): 경우마다 **정해 둔 답**과 시뮬의 결과를 함께 본다 — 주인의 마지막 미편입 정착 성계: 거짓이고 만료 뒤 `Hazard.Claim`이 없다(`Law.Remnant`); 영향권이 찬 주장자: 거짓; 닿는 주장자 없음: 거짓; 주장자가 닿는 보통의 정착 성계: 참이고 주장이 선다; 주인이 답인 칙령을 지닌 위기·`Hazard.PreSupernova`·`Hazard.Claim`: 거짓(만료 뒤 새 주장 없음). 변이: `ClaimRuleHoldsOf`에서 마지막 미편입의 검사를 빼면 그 경우가 깨진다(시뮬과 화면이 함께 바뀌어도 정해 둔 답이 깨진다); `ClaimMayFollowOf`에서 칙령의 문을 빼면 칙령의 경우가 깨진다.
- `UI.RebellionWatch`(`[B-6a4]`): 틱 수는 행에서 읽는다(키를 50으로 바꾼 테스트 표에서 50 — 변이: 100을 적으면 깨진다); 안정도의 문턱은 시뮬이 세는 상수 하나를 읽는다(§4).
- `Log.AwarenessVoice`(`[B-6a5]`): 플래그가 선 틱에 한 줄, `Log.Transcendent`, 말하기 스트림 한 수·시뮬 스트림 0, 생애 장과 두 책에 없다(변이 — 범주를 `Log.Claim`으로 하면 두 책에 서서 깨진다); **그 만료의 마지막 줄이다** — 만료 줄·기록 줄·주장 발생 줄 뒤(변이 — `Place`의 자리에서 말하면 깨진다); `CosmicAutoPause::StopsFor`가 거짓(변이 — Severity 2면 깨진다); 계승 뒤 다시 말해지지 않는다; 타락 행 둘의 표에서 둘째 초월자가 알아채도 둘째 줄이 없다(변이 — 줄의 문을 그 초월자의 플래그로 하면 깨진다).
- `Sim.ExaltArrives`(`[B-6a6]`): 게임 데이터에서 `petition_exalt`가 다스리는 백성의 둘째 칸 틱에 대기 칸에 선다(변이 — 조건을 되돌리면 깨진다); 둘째 칸 이상 백성의 왕좌에 오른 재위의 첫 틱에도 선다(B1의 빈 머리); 스키마의 통치자 행 규칙은 `Tools/test_data_schema.py`.
- **고치는 테스트**(끝을 읽거나 손으로 세우는 열세 파일 — 노트 §0.9′): `[B-6a3a]`은 셋째 칸으로 끝을 세우던 테스트를 고치고(셋째 칸에서는 연출이 서고 판이 이어진다 — 끝은 테스트 노브의 시계로), `[B-6a3c]`은 열거·래치를 읽거나 틱 없이 끝을 세우던 테스트를 도우미 하나 `CosmicTests::EndRunAt(FGalaxyState&, FGameplayTag Kind, int64 Tick)`(`Tests/CosmicTestHelpers.h` — `State.Ending`을 쓴다)로 고친다 — 각 테스트는 `ClearRunEnd()`·`Tick = WasTick` 곁에서 `State.Ending = FCosmicEndingRecord()`로 되돌린다. 커밋마다의 파일·줄과 고치는 꼴(`UI.RunEnd`의 (6)·`UI.Chronicle`·`UI.ContinueEnded` 포함)은 노트 §0.9′의 '고치는 자리'. `Feats.Reachable`(`Tests/CosmicAscensionTests.cpp:1582` — 'AI 셋째 칸 ≥ 1')은 그대로.

## 7. 문서

**코드보다 먼저**(`[B-6a2]` — '틱 순서·저장 필드를 바꾸면 docs/20을 먼저'):
- `docs/20:285`(틱 처리 순서 — 열째 단계 뒤 끝의 기록: 한 번·추첨 없음·첫 끝만; 위기 단계 만료의 새 세계 쓰기와 새 말하기 자리: 주장 규칙이 다스리는 셋째 칸 이상 백성의 성계에 주장을 놓으면 그 주장자의 알아챔 플래그(초월자마다 한 번, `PendingEffect`가 빈 그릇만), 그 만료의 끝 '… → 뒤따르는 줄 → 알아챔 줄(판에 한 번, 말하기 한 수)')·`:195`('임시 끝에는 저장 필드가 없다' → '끝의 기록' — 필드·얼린 기본값·두 해시·`SaveVersion` 1·옛 세이브는 흐른다)·`:89`의 규칙 문장 둘('시뮬은 끝을 모른다' → '시뮬은 끝을 적되 멈추지 않는다 — `RunLeap`만 선다'; '칸이 이긴다' → 같은 틱의 차례) — 단락 전체의 고침은 문서 커밋.
- `docs/21` §7.4 — `:1324`(끝의 판정 → `EndingOf`와 기록)·`:1326`(`RunHeadless`의 약속)과 `RunLeap`의 사본(그 아래 줄)·`:1401`(`SimulateTick`의 열 단계 뒤)·`:1407`(역할 효과 해소 뒤의 기록)·`:1480`(`TickHazards`의 만료 차례 '… 만료 줄 → 기록 줄 → 징조·1인칭 → 뒤따르는 줄' 끝에 알아챔 줄, 주장 규칙 뒤의 플래그); `FGalaxyState` 사본에 `Ending`.
- `crisis_and_transcendents.md:149`('엔딩 이후'의 MVP 임시 끝 문장 가운데 장치 문장만 — 저장 필드 없음·시뮬은 모름·이어하기는 끝난 채) · `:145`·`:195`(침묵의 우주의 **조건만** — '상세 은하의 소유 성계가 0일 때' → 계승할 후보가 없음; 결정 6-다의 답대로 — 태그 `Ending.NoCandidate`가 그 조건을 세이브에 얼리므로) · `:99`(알아챔의 때와 '타락은 제 손이 닿은 곳을 다스리는 이를 알아챈다' — 결정 7-가·7-다의 답대로, 늦어도 `[B-6a3a]` 전).
- `CLAUDE.md` 규칙 4·13과 규칙 15의 예정(결정 20의 답대로): `[B-6a3]`의 코드를 말하는 문장에는 '(B6a — `[B-6a3]`부터)' 표지를 달고 `[B-6a3c]`에서 뗀다(B5 결정 21의 꼴; 규칙 13의 `ClaimRuleHoldsOf`(화면의 답 `ClaimMayFollowOf`)는 `[B-6a4]`부터).

**장치의 세 커밋**(`[B-6a3a]`~`[B-6a3c]` — 주석은 그것이 말하는 코드를 바꾸는 커밋에; 헤더 주석은 그 커밋의 전체 빌드에 든다): 커밋마다 고칠 주석의 파일·줄은 노트 §1.1. 그 가운데 `[B-6a3c]`에 드는 문서 둘 — **`docs/30` 다시 만듦**(§3.5 — 시계 노브의 읽는 코드가 둘)과 `docs/40`의 커맨드렛 절에 `ending:` 줄 한 단락(`calling:`에서 뗀 칸과 함께 — §5.1).

**표현과 한 커밋**(`[B-6a4]`): `UI/CosmicTopBarWidget.h:70-71`(규칙 줄 — `Chronicle.RuleRung`)·`:85-86`(예고), 옮긴 `LowStabilityThreshold`의 주석.

**문서 커밋**(`[B-6a7]` — 끝에 재감사 워크플로): `docs/20:89`의 '임시 끝' 단락과 `docs/21`의 나머지 사본, 설계(`crisis_and_transcendents.md` — 결정 6-나·6-마·7-다의 자리, `systems.md` — B6b·B7로 넘긴 표지), `docs/00_VISION.md`(결정 1·2-라), `docs/50`(B6 칸을 B6a·B6b로 — 결정 1·17-나·21), `WORKPLAN.md:61`(결정 15의 한 줄)·`:65`, `docs/15_GLOSSARY.md`, `docs/70`(까닭 줄·알아챔 줄·경고의 글의 규칙), `docs/40`, `DESIGN_AUDIT_AND_IMPROVEMENT.md`(결정 2-가·9), `OPEN_QUESTIONS`(노트 §0.2)·`DECISIONS_LOG`·`docs/CHANGELOG.md`·`STATUS.md`, `CLAUDE.md` 규칙 15의 'A0~B5' → 'A0~B6a'(결정 20), 코드 주석의 나머지(헤더 파일이라 에디터를 닫고 전체 빌드). 줄마다의 자리와 고칠 말은 노트 §1.2.

## 8. 순서·검증·effort (`WORKPLAN.md:61`의 B 행)

| 커밋 | 하는 일 | effort | 검증 |
|---|---|---|---|
| (헤더) | 이 파일과 노트(만든 길은 상태 줄) | xhigh | `python -I Tools/check_docs.py` 0; 150,000바이트 안; **ultracode 적대 리뷰**와 고친 뒤 확인 — 일꾼에게 줄 범위를 나눠 준다(§0~§2 / §3~§6 / §7~§10) |
| `[B-6a1]` | 첫 테스트: `Sim.NotablesFromEventsOnly`를 게임 데이터로(결정 10), `docs/40:284` | high(테스트 하나의 허용 범위 — ultracode 끔, 리뷰 한 번) | 러너 full 두 번; 다섯 기준값과 해시 판 비트 그대로; 변이 둘 |
| `[B-6a2]` | **필드만** — `FCosmicEndingRecord`·`FGalaxyState::Ending`·태그 다섯·`CosmicTags` 다섯(이름은 결정 6-가의 답); 문서 먼저(§7의 첫 목록 — `CLAUDE.md` 규칙 4·13·15의 예정 포함, 결정 20); `Sim.SaveEndingRecord`·`Sim.HashCoversEnding` | xhigh | 에디터를 닫고 전체 빌드; 러너 full 두 번; **다섯 줄의 두 해시와 해시 판 둘이 옮는다** — 커맨드렛 CSV의 결정적 칸이 다섯 판 모두 같다(B3 `[B-3a]`의 꼴); 새 해시를 커밋 메시지에; **ultracode 리뷰**(저장 호환 — 얼린 기본값·델타 로드·태그 이름 / 해시 — 두 해시에 드는가 / 문서 먼저 감사 — docs/20·docs/21·설계·`CLAUDE.md`를 이 헤더에 견준다) + 고친 뒤 확인 |
| `[B-6a3a]` | **셋째 칸 뒤의 판**(결정 2-가·15): `EndRung`·`Rung` 가지를 지움(`ProvisionalEndOf`는 시계만 답한다; 연대기 `NoteOf`의 칸 가지, 커맨드렛의 `RungTick`·옛 `end check`·`calling:`의 '새 백성' 칸), 시계 노브의 툴팁, 알아챔 플래그(결정 7-가·나의 답 — §2.11); `Sim.AwarenessOnce`; 셋째 칸으로 끝을 세우던 테스트를 시계로(§6) | xhigh | 에디터를 닫고 전체 빌드; 러너 full 두 번; **깨질 수 있는 확인 ⓐ~ⓓ**(§5.3 — ⓓ는 탐침); 옮은 기준값(정책 판 셋과 정책 해시 판)을 커밋 메시지에 |
| `[B-6a3b]` | **기록 쓰기**: `EndingOf`(§2.2)·열째 단계 뒤 한 번(§2.3)·같은 틱의 차례·`CosmicSuccession::IsSilent`(컨트롤러의 사본이 이것을 부른다)·새 `end check`(§5.1)·`RunHeadless`의 약속 고쳐 적기(§2.5); `Sim.EndingOf`·`Sim.EndingRecordOnce`·`Sim.EndingInert`·`Sim.SilenceEnds` | xhigh | 에디터를 닫고 전체 빌드(`Sim/CosmicEndings.h`·`Sim/CosmicSuccession.h`의 새 선언); 러너 full 두 번; **CSV의 결정적 칸이 다섯 판 모두 `[B-6a3a]`와 같고 옮는 것은 6,000에 닿는 세 판의 두 해시뿐**(§5.3); 새 해시를 커밋 메시지에 |
| `[B-6a3c]` | **장치**: 읽는 곳을 기록으로(래치·`MarkOf`·두 정책·커맨드렛의 `end:`·연대기의 서명 — §2.7), `RunLeap`의 멈춤·`AdvanceLeap`(시계의 자르기를 지우고 스위치가 꺼지면 한 번 더 — §2.6)·침묵의 래치·끝난 세이브, `ProvisionalEndOf`·`ECosmicProvisionalEnd`를 지움, 커맨드렛의 `ending:`; §6의 `[B-6a3c]` 테스트와 고치는 테스트(도우미 `CosmicTests::EndRunAt`); §7의 장치 주석; 묶음 0 | xhigh | 에디터를 닫고 전체 빌드; `docs/30` 다시 만듦(§3.5 — 러너의 문서 검사); 러너 full 두 번; **다섯 기준값과 해시 판 둘이 비트 그대로**(§5.3); 묶음 0의 표는 노트 §3, 미리 셈한 판단은 §11; **ultracode 구현 리뷰** — 세 커밋의 차이를 이 커밋 전에 함께 본다(고침의 자리: 기준값을 옮기지 않는 고침은 이 커밋에 들고 그 확인을 다시 돈다; **기준값을 옮기는 고침** — 알아챔 플래그의 문, 기록을 쓰는 자리, `EndingOf`의 답 — 은 이 커밋에 넣지 않고 그 앞의 따로 된 커밋 `[B-6a3r]`로(옮은 줄과 까닭을 커밋 메시지에, 확인은 `[B-6a3b]`의 꼴 — CSV의 결정적 칸과 옮은 해시의 까닭) — 그래야 이 커밋의 비트 그대로가 장치의 증거로 남는다; 앞의 두 커밋은 이미 푸시되었으므로 고쳐 쓰지 않는다): 결정론 — 추첨 0·쓰는 자리·한 번 / 끝 뒤의 시뮬과 `RunLeap`의 멈춤·스위치가 꺼진 세션의 둘째 부름·침묵의 판·공백기 안의 끝 / 저장 — 옛 세이브·이어하기·까닭 `Ended` / 래치와 쪽의 차례 / 알아챔이 앎을 읽지 않는가·`PendingEffect`의 문·초월자마다 한 번 / 기록을 읽는 단계가 없는가 / **측정** — `ending:` 줄의 셈(한 번 세기·셋째 칸 뒤와 끝 전의 가름·6,000 안에 닿지 않은 셋째 칸과 연구 1.0은 따로 센다·`end check`)과 묶음 0의 미리 셈 + 고친 뒤 확인 |
| `[B-6a4]` | **표현**: 연대기의 까닭 줄·그릇 없음의 줄·시험판 알림(서명은 `[B-6a3c]`), 상단 바의 규칙 줄, 주장의 경고 줄과 반란 문턱(함수 셋 — `ClaimRuleHoldsOf`(시뮬의 `TryPlaceClaim`이 부른다)·`ClaimMayFollowOf`·`RebellionWatchOf`, Sim 헤더로 옮긴 `LowStabilityThreshold` — §4), 스트링 초안 | high — **주장 규칙을 옮기는 코드(`ClaimRuleHoldsOf`와 그것을 부르는 `TryPlaceClaim`, 상수의 옮김)는 xhigh**(위기 단계의 시뮬 코드 — B 행의 '장치(결정론)'; B5가 `VesselAtRiskOf`·`AIAimOf`를 옮긴 `[B-5a]`의 꼴) | 에디터를 닫고 전체 빌드(`Sim/CosmicHazards.h`의 새 함수 셋과 옮긴 상수); 러너 full `-Data` 두 번(새 스트링 키 — `Data.SourcesInSync`); **비트 그대로**(주장 규칙을 함수로 옮긴 증거); UI 테스트; 창 없이 찍은 화면(종류마다의 연대기 쪽, 그릇 없이 맞은 결말의 쪽, 패널의 두 줄); **ultracode 구현 리뷰**(옮긴 판정이 비트 그대로인가 — 같은 판정·추첨 수 그대로)와 **ultracode 화면·문구 리뷰** + 확인 |
| `[B-6a5]` | **글과 목소리**: 알아챔의 줄과 `AwarenessTemplates`(정의 칸·스키마), 까닭 줄·규칙 줄·그릇 없음·패널 두 줄의 글, 스키마의 접두어 규칙과 새 목록 둘(§3.2·§3.4), `Log.AwarenessVoice` — **B6a 모든 글의 ultracode 글 감사** | high(알아챔 줄을 말하는 위기 단계의 코드는 xhigh — 틱 안에서 말하기 스트림을 뽑는 시뮬 코드라 B 행의 '장치(결정론)'다) | **세계 해시 다섯 그대로**(옮는 것은 알아챔 줄이 말해진 판의 전체 해시뿐); **ultracode 구현 리뷰**(그 코드 — 말하기 스트림만·세계와 앎에 쓰지 않음·판에 한 번·만료의 마지막 줄·Severity 1·핸들러) + 고친 뒤 확인; 에디터를 닫고 전체 빌드(`FTranscendentDef`의 새 UPROPERTY); 러너 full `-Data` 두 번; 변이 |
| `[B-6a6]` | **데이터 켜기와 밸런스**: `petition_exalt`의 조건, 스키마의 통치자 행 규칙, `-ExaltAsBefore`와 `CosmicTests::WithExaltAsBefore`(도우미가 먼저 — 짝의 한쪽), 묶음 1 → (판단이 노브 바꿈을 부르면 그 노브는 B6a에서 바꾸지 않고 수와 함께 넘긴다 — §5.2) → 증거 시드와 `-SaveAt` 세이브(§9) → `-UpdateBaseline` | high | 에디터를 닫고 전체 빌드(`Tests/CosmicTestHelpers.h`의 새 도우미); 러너 full `-Data` 두 번; **깨질 수 있는 확인 ⓐ~ⓓ**(묶음 1); 옮은 기준값을 커밋 메시지에; **ultracode 밸런스 리뷰**(목표·방법·놀이·위험 + 비평) |
| `[B-6ar]` | 리뷰의 고침이 크면 그 고침만(B5 `[B-5r]`의 꼴) | 고침의 성격대로 | 그 고침의 검증 |
| `[B-6a7]` | **문서**(§7의 문서 커밋)·`CLAUDE.md` 규칙 15의 가리킴·`WORKPLAN.md`의 두 줄·코드 주석의 나머지 | xhigh | 에디터를 닫고 전체 빌드; 러너 full 두 번(비트 그대로); **재감사 워크플로**(파일 묶음마다 편집 → 감사 셋 → 겹치지 않는 고침 → 확인 → 비평) |

- **`[B-6a3]`은 세 커밋이다**(`[B-6a3a]`·`[B-6a3b]`·`[B-6a3c]` — 결정 15); 이 헤더에서 꼬리 없는 `[B-6a3]`은 셋을 함께 가리킨다. 구현 리뷰의 고침이 기준값을 옮기면 `[B-6a3c]` 앞에 넷째 커밋 `[B-6a3r]`이 선다(위 표의 `[B-6a3c]` 행).
- **글 감사의 눈**(`[B-6a5]` — 발견마다 반박 검증, 빠진 것을 찾는 비평 하나): ① 신학과 화자 층 — 까닭 줄이 거룩한 질서를 끝의 조건·행위자로 만들지 않는가, 알아챔이 환생('같은 부름을 다시 안다')·심판·메시아 신호로 읽히지 않는가, 그 줄이 같은 틱의 영압 카드와 엮여 두 목소리를 타락의 꾐으로 만들지 않는가, **`petition_exalt`의 글(제목·두 목소리·세 결과 줄)이 처음 실제로 서는 두 자리** — 둘째 칸 연출(금빛·종소리·업적 제목) 바로 뒤, 둘째 칸 이상 백성의 즉위 첫 틱 — 에서 메시아 신호 10·11(경배받는 통치자·얻은 승천처럼 보이는 부르심 — `docs/90_LORE/theology_and_bestiary_notes.md:244-245`)이 겹쳐 읽히지 않는가 ② 한국어 교회 말투 ③ 모든 길 — 그릇 없이 맞은 결말의 쪽, 끝난 옛 세이브, 시계로 끝난 공백기, 보이지 않는 백성의 이름, 한 그릇의 비문과 그 백성의 다섯째 칸이 같은 틱에 잇는 쪽(`docs/70:209`의 '그들을 위해 죽었다'의 읽힘 — §2.9) ④ 첫 독자 — 끝이 패배·성적표로, 경고가 공포로 읽히지 않는가. 성경은 설계의 근거로만, 대조 도구로 확인한 구절만 설계 문서에 넣는다(`docs/90_LORE/theology_and_bestiary_notes.md:259`).
- **일하는 법**: B4 헤더 `:425`·B5 §8의 목록 그대로(도우미는 한 항목으로 먼저; 고친 뒤 확인; 코드가 부르는 행은 그 코드와 한 커밋; 해시를 옮기는 길과 데이터 켜기는 나눈다; 워크플로 일꾼은 `CLAUDE.md`를 디스크에서 읽는다).
- **크기**: 이 헤더는 150,000바이트(UTF-8) 안에 두고 같은 말을 두 절에 적지 않는다 — 결정은 절을 가리킨다. 긴 잰 값과 커진 §11은 노트로 뗀다.

## 9. 손 확인 (B6a의 종료 조건 — 개발자, 콘솔 없이; 결정 18)

1. **B1~B5의 손 확인을 먼저 권한다**(`STATUS.md:29`의 1 — 문으로 걸지 않는다; 오지 않으면 B6a의 것과 한 빌드로 함께 하고 결함은 블록마다 가른다 — B5 결정 22의 꼴).
2. **세이브는 `-SaveAt`으로 만든다**(게임과 같은 꼴의 판 — R + `-Seed=<시드> -SaveAt=<틱>`; 시드와 틱은 묶음 1에서 골라 노트 §5에 명령 줄·기대 틱과 함께 적는다). 에디터 `-game`의 '이어하기'로 연다(`Saved\SaveGames`에는 하나씩). **증거 세이브는 마지막 코드 커밋(`[B-6ar]` 또는 `[B-6a6]`)의 빌드로 다시 만든다** — 측정 때의 사본은 옛 글을 지닌다(B5의 교훈 — `STATUS.md:29`); ④의 옛 세이브만 예외다.
3. **목록**: ① **셋째 칸 바로 앞**(다스리는 셋째 칸 − 50) → 셋째 칸 → 연출 → '이제 보인다' → 시계까지 논다: 영적 시야와 ×25(#96·#100 ③ — ×25에서 새 위기의 300주기가 12초), 영압 카드와 1인칭 주장 줄(B4 손 확인 3 — #13 '지옥 시뮬레이터'로 읽히는가), 끝 예고 'Soon'과 툴팁의 규칙 줄, 시계의 끝 쪽(시험판 알림이 선다); ② **알아챔**: 알아챔이 끝 전에 선 시드의 그 틱 − 30 — 그 줄이 어떻게 읽히는가, 같은 틱의 카드와 엮이지 않는가; ③ **공백기 안의 끝**: B5의 시드 60 **입력 없는 판** `-Seed=60 -Ticks=6000 -TellAll=0 -SaveAt=5130`(정책 인자 없이 — 위 2의 R이 아니다; B5 노트 §5의 ⓒ — 1,022 폐위·5,143 죽음, `STATUS.md:29`; 입력 없는 세계는 B6a에서 그대로라 같은 틱이고, 기다리는 카드에 답하지 않는다) → 비문 → 다이제스트 → 연대기의 까닭 줄 '6000주기…'와 그 아래 '그릇 없이' 줄; ④ **끝난 옛 세이브**: B5 빌드에서 셋째 칸으로 끝난 세이브(개발자의 MVP 세이브가 있으면 그것)를 B6a 빌드로 연다 — 멈춘 채 흐르는 판으로 서고, 놀면 시계에서 끝난다; 그 세이브는 `[B-6a3a]` **전에** HEAD `b10be48`의 빌드로 만든다(그 뒤에는 B5 빌드가 없다): R `-Seed=42 -Ticks=2400 -SaveAt=2376`(통치자 판의 칸 끝 2,375 바로 뒤)의 슬롯을 `Saved\SaveGames` 밖의 이름 붙인 폴더로 옮겨 두고 노트 §5에 적는다(델타 저장이라 B6a의 `-SaveAt`으로 만든, 셋째 칸 뒤·빈 기록의 세이브도 같은 꼴이다 — 개발자의 MVP 세이브가 없을 때의 둘째 길); ⑤ **`petition_exalt`**: 시드 42 `-SaveAt=1150` → 1,175 무렵 둘째 칸의 연출(금빛·종소리)이 닫힌 뒤 카드(자동 정지)와 세 선택 — 연출에서 카드로 가는 차례가 어떻게 읽히는가; 받아들인 뒤 양위하면(방랑자 판의 꼴 — 1,200) 같은 그릇의 툴팁 '높아짐 2'가 몸·정신 곁에서 키울 능력치로 읽히는가(§3.1); ⑥ **주장의 경고와 반란 문턱**: 경고는 ①의 판에서 다스리는 성계에 위기가 섰을 때; 반란 문턱은 ①의 시드에서 드물므로(통치자 판의 안정도 최저 중앙값 0.640 — 노트 §0.1) 묶음 1에서 끝 전에 다스리는 수도의 `LowStabilityTicks`가 처음 0을 넘은 시드(§5.1의 칸)를 골라 그 틱 − 30의 따로 된 `-SaveAt` 세이브로 본다; ⑦ 침묵은 손으로 닿지 않는다 — '없음'으로 적고 `[B-6a4]`의 창 없이 찍은 화면으로 본다; ⑧ 시드 42의 보통 판에서 첫 10분이 그대로인가; ⑨ **벽시계**: 세이브 없이 시드 42의 새 판을 ×10으로 시계까지 한 번 — `CosmicEnd` 줄 전체(첫 틱부터의 초와 멈춘 초 — `World/CosmicGameMode.cpp:1211-1221`)를 노트 §5에 적는다(결정 2-라가 읽는다; ①은 셋째 칸 − 50에서 열어 그 줄이 판 전체를 재지 않는다).

## 10. 결정 (권장안 먼저, 대안과 그 까닭; **[개발자]**는 상시 승인 밖)

**답**(2026-10-31): [개발자] 일곱 묶음이 모두 왔다 — 1·2-가·2-나·2-다는 먼저, 2-라·6·7·17-나·20·21은 '전부 권장대로'. 커밋 앞에서 기다리는 답은 없다 — 답에 닿는 문서는 §7의 차례대로 고친다(6-다·7-가·7-다·20의 문장은 `[B-6a2]`의 문서 먼저, 늦어도 `[B-6a3a]` 전; 나머지는 `[B-6a7]`).

1. **[개발자] B6의 범위와 나눔** (답: 개발자 2026-10-31 — ㉮ B6a / B6b): B6a는 끝의 장치와 셋째 칸 뒤의 판(§0의 '드는 것'), B6b는 넷째·다섯째 칸의 업적 행과 AI(제동 포함), 집단 승천과 다른 질서가 실제로 닿는 길(§0의 'B6a 밖'). T5 문턱·두 길·후원·봉인·단독 승천의 길은 Phase 6+(단독 승천은 B6a에서 기록 종류와 까닭 줄만). `docs/50:12`의 'T5 문턱'은 채택 문장(브리핑 `:259` 'T3→T5 업적·엔딩 4')으로 되돌린다(문서 재편이 더한 말이다). 대안이던 것: ㉮′ B6a → B7 → B6b(AI가 칙령·조약을 갖춘 뒤 설계 표에 가까운 행 — 엔딩이 한 블록 늦다), ㉯ 줄인 한 블록(결정 25~30개로 헤더가 150KB를 넘고 셋째 칸 뒤 계절의 재측정과 넷째 칸의 경주가 한 데이터 커밋에 섞인다), ㉰ 설계 표 전부(B7보다 크다). *왜 개발자인가*: 승인된 계획의 차례·범위와 비전의 '1판 = T5 두 길'(`docs/00_VISION.md:18`)을 바꾼다.
2. **[개발자] 판의 끝과 길이**:
    - **2-가 셋째 칸 끝을 지운다** (답: 결정 1 ㉮에 든다 — 개발자 2026-10-31; 예정의 실행): 'T3 ∨ 6,000'은 MVP의 끝이고(`STATUS.md:43`) 그것을 엔딩으로 바꾸는 일은 이미 예정되어 있었다(`docs/20:195`, `Sim/CosmicEndings.h:2-3`, `docs/21:1326`) — 앞의 답을 뒤집는 것이 아니라 예정의 실행이다. '문턱의 계절 +300'(`DESIGN_AUDIT_AND_IMPROVEMENT.md:335`)은 받지 않는다 — 300틱은 주장·1인칭에 닿지 않고(R의 첫 주장 중앙값 3,328 — B4 헤더 `:90`) 오른 틱의 저장 칸이 들며 그 전제의 `petition_exalt`는 셋째 칸의 카드가 아니다(§1.1).
    - **2-나 시계는 B6a에서 6,000 그대로** (답: 개발자 2026-10-31): 스크립트 통치자에게 셋째 칸 뒤 약 3,600틱의 계절. 마지막 값은 B6b의 경주를 잰 뒤 정한다(#2). B6a의 `Ending.Clock` 기록은 마지막이다 — B6b가 시계를 옮겨도 6,000에 끝난 B6a 세이브는 끝난 채 열리고, 다시 열지는 B6b가 정한다(그러면 `SaveVersion` 2와 이주 — §2.10). 대안이던 것: 지금 18,000(`docs/00_VISION.md:103` — 넷째 칸이 끼어들지 않은 계절과 섞여 B6b의 측정이 흐려진다), +N의 계절.
    - **2-다 끝난 MVP 세이브는 흐르는 판으로 연다** (답: 개발자 2026-10-31): §2.10 — `SaveVersion` 1 그대로, 이주 없음. 대안이던 것: `SaveVersion` 2와 이주 한 번(`Save/CosmicSaveSubsystem.cpp:504`의 자리 — v1 세이브의 임시 끝을 기록으로 옮긴다), 끝난 채 열기(기록 없이 v1과 B6a 세이브를 가를 수 없어 이주가 든다).
    - **2-라 [개발자] 비전의 결정 밀도 잣대와 MVP 벽시계 문장** (답: 권장대로, 개발자 2026-10-31 — **잣대는 '판의 끝까지 결정 중앙값 ≥ 15' 그대로 두고 그 끝을 'B6a부터 시계 6,000(마지막 값은 B6b)'으로 고쳐 적으며, '×10 30~60분'은 MVP(첫 생애에서 셋째 칸까지)의 기록으로 남기고 B6a 판의 벽시계는 손 확인 ⑨(세이브 없이 시계까지 한 판)의 `CosmicEnd` 줄로 잰 뒤 B6b가 시계와 함께 새 잣대를 정한다; 잣대의 깨질 수 있는 확인은 '계절의 결정 밀도'다 — §5.2의 ⓑ**): 판이 6,000까지 가면 ×10에서 쉬지 않는 틱만 10분이고 카드·정지가 더해진다 — 30~60분은 셋째 칸 끝(약 2,370틱)을 두고 정한 값이다(`docs/00_VISION.md:20`·`:32`). 대안이던 것: ㉮ 지금 '×10 45~90분' 같은 새 값을 적는다(잰 것 없이); ㉯ 벽시계 잣대를 B6b까지 지운다(그동안 손 확인의 잣대가 없다). *왜 개발자인가*: 비전 문장이고 2026-10-18의 MVP 결정(`STATUS.md:43`)의 '×10 30~60분'을 옮긴다.
3. **엔딩 기록 — 저장 필드 하나, 두 해시, 필드만의 커밋**(권장): §2.1. 대안 ㉮ 두 해시에서 뺀다 — 규칙 6의 문장과 빼는 목록이 바뀌고(`CLAUDE.md` 규칙 → [개발자]) 엔딩이 세계의 사실이 아니게 된다; ㉯ 순수 함수만(필드 없음 — 기준값 비트 그대로): 끝 뒤로 도는 측정에서 다른 질서의 답이 역할 바뀜·백성의 소멸로 뒤집혀 먼저 온 끝을 잃고, 끝의 틱을 시계 노브가 바뀌면 다시 셈해 끝난 판이 다시 열린다; ㉰ 기록을 `Calling` 안에 — 판의 사실이지 부름의 것이 아니다.
4. **쓰는 자리와 끝 뒤의 시뮬**(권장): §2.3·§2.5·§2.6 — 열째 단계 뒤 한 번, 추첨 없음; 시뮬은 거절하지 않는다; `RunLeap`만 그 틱에서 선다; 명령은 컨트롤러가 거절한다. 대안 ㉮ 약속대로 끝난 판에서 `RunHeadless`·`StepOnce`가 거절하고 측정에 스위치 — 측정마다 스위치가 들고, 20,000틱 기준값 둘의 뜻이 바뀐다; ㉯ 경계의 순수 함수로 판정하고 컨트롤러만 래치(필드 없음) — 결정 3의 ㉯와 같다; ㉰ 업적 판정 안(AI 단계 끝)에서 쓴다 — 열째 단계의 침묵과 `bNoVessel`을 보지 못한다.
5. **같은 틱의 차례와 백성·그릇 없음**(권장): §2.2의 표·§2.4. 대안: 다른 질서를 집단 승천 앞에 — '그릇의 일이 같은 틱의 남의 일보다 앞선다'는 지금의 '칸이 시계를 이긴다'와 어긋나고 판정 순서(다스리는 백성 먼저)와 반대다.
6. **[개발자] 엔딩의 이름과 글**:
    - **6-가 저장 태그와 필드의 이름** (답: 권장대로, 개발자 2026-10-31 — **사실의 낱말** — `Ending.RuledPeople`·`Ending.Vessel`·`Ending.OtherPeople`·`Ending.NoCandidate`·`Ending.Clock`, 필드 `FCosmicEndingRecord { Kind, Tick, CivID, bNoVessel }`, 멤버 `FGalaxyState::Ending`): 세이브에 남는 이름이라 처음 넣을 때 정한다. A4 스케치(`A4_provisional_end_notes.md:449-458`)의 `Ending.SilentUniverse`는 타락 교리의 낱말을 영구 식별자로 박고(#150 — '침묵'은 타락이 이겼다는 말), `OtherOrder`의 '질서'는 거룩한 질서의 낱말이며(B5 글 감사가 채굴 AI 각성의 만료 줄에서 막은 말 — `docs/70:209`의 결), `bWatched`('부름이 지켜본')는 B5 결정 12-라가 버린 그림이다. 침묵을 `Ending.NoVessel`로 부르면 칸 `bNoVessel`과 뜻이 겹친다. 대안이던 것: ㉮ A4 스케치 그대로; ㉯ 시계를 기록에 두지 않는다 — 시계 끝이 결정 3의 ㉯의 약점을 그대로 지닌다.
    - **6-나 엔딩의 화면 제목을 두지 않는다** (답: 권장대로, 개발자 2026-10-31): 설계 이름('영겁의 문명'·'맡겨진 영겁'·'질서를 맡다'·'다른 질서'·'침묵의 우주' — `crisis_and_transcendents.md:145-148`)은 설계 문서의 이름으로만 남는다. 비조소 규칙('모든 엔딩은 같은 서식·같은 색·같은 첫 줄 … 차이는 기록된 사건의 내용뿐' — `:151` 끝)과 규칙 11('받은 꼴만')에 맞고, '승천'은 `Chronicle.` 키가 막는다(`Tools/data/schema.json:51`·`:62`). '영겁의 문명'은 피조물의 '영원'으로 읽혀 고정 지평 줄('손으로 짓지 않은 나라' — `ST_CA.csv:132`)과 맞부딪고, '질서를 맡다'는 플레이어가 주어인 능동이다. 대안이던 것: ㉮ 받은 꼴로 다시 쓴 제목; ㉯ 설계 제목 그대로.
    - **6-다 침묵의 우주의 조건** (답: 권장대로, 개발자 2026-10-31 — **계승할 후보가 없음** — §2.8; 6-가의 태그 `Ending.NoCandidate`가 이 조건을 세이브에 얼리고 `[B-6a3b]`가 짓는다): 설계의 '상세 은하의 소유 성계가 0일 때'(`crisis_and_transcendents.md:145`, 둘째 사본 `:195`)를 B5의 후보 없음으로 고친다 — 후보는 모든 백성의 정착된 성계이고 죽은 자리와 치명 위기의 성계는 소유여도 빠진다(`Sim/CosmicSuccession.cpp:42-53`). 설계 문장은 `[B-6a2]`의 문서 먼저에서 고친다(설계 → 코드의 차례 — §7). 대안이던 것: 소유 성계 0을 따로 판정한다 — 계승의 단일 창구 밖에 둘째 조건이 서고, 후보가 없어 공백기에 머문 판이 그 조건이 설 때까지 그릇 없이 흐른다.
    - **6-라 까닭 줄과 규칙 줄의 꼴** (답: 권장대로, 개발자 2026-10-31): §3.3·§3.6 — 판의 규칙, 거룩한 질서의 수동 없음, 집단 승천과 다른 질서는 같은 줄, B6a의 규칙 줄은 시계만. 대안이던 것: 다섯째 칸의 규칙을 지금 툴팁에(닿지 않는 끝을 약속처럼 읽힌다 — 결정 13).
    - **6-마 설계 문장 둘을 고친다** (답: 권장대로, 개발자 2026-10-31): '당신이 아닌 누군가가 우주를 정의했다'(`crisis_and_transcendents.md:134`)를 세계의 사실 꼴('그릇이 다스리지 않는 백성이 다섯째 칸에 섰다' — 초안; 섭정 아래의 옛 백성(§2.2의 3)에도, 공백기에도 참이고 '당신'과 경주의 낱말이 없다)로 — 피조물에게 저자의 일을 준다(`pillars.md:89`의 '저자에게 대칭되는 상대는 없다'); '`Doctrine.Silence`와 일치하는 4번째 엔딩'(`:145`)을 지운다(남은 자 법이 마지막 미편입 성계를 지켜 주장만으로는 은하가 비지 않는다 — 그 상태를 만드는 것은 파괴다); '그릇 없이 맞은 결말'의 '가칭'을 뗀다(`:148`). 대안이던 것: 그대로 — 이원론의 읽힘이 설계에 남는다.
    - *왜 개발자인가*: 비전의 낱말(`docs/00_VISION.md:7`·`:172` — 영겁)·설계 표의 이름과 문장, 신학 규칙 11.
7. **[개발자] 알아챔**(#140·#136):
    - **7-가 때** (답: 권장대로, 개발자 2026-10-31 — **셋째 칸 이상의 다스리는 백성의 정착 성계에 처음 주장이 서는 틱** — §2.11): 셋째 칸의 틱(B4 초안 — B4 헤더 `:559`, `crisis_and_transcendents.md:99`)이 아니다 — 드러냄 '이제 보인다' 곁에서 타락의 첫 2인칭이 열린 눈을 타락이 준 것으로 읽히게 한다. '다스리는 백성의 눈에 영향권이 든다'처럼 앎으로 정하면 세계 코드가 앎을 읽는다(규칙 2). 대안이던 것: ㉮ B4 초안대로 셋째 칸의 틱(업적 판정의 플레이어 가지 — `Ascension/AscensionSubsystem.cpp:145-158`; 늘 서고 통치자 판 모두의 세계 해시가 2,370 무렵에 옮는다); ㉯ 셋째 칸 뒤 첫 편입(그 틱에는 이미 1인칭 주장 줄이 선다 — B4의 '편입마다 1인칭 하나'와 겹친다).
    - **7-나 플래그와 판에 한 번** (답: 권장대로, 개발자 2026-10-31 — **플래그를 쓴다** — 세계의 사실, 초월자마다 한 번, 계승을 넘어 남고 뒤의 그릇에게 다시 말하지 않는다; **줄은 판에 한 번** — 그 쓰기 전에 어느 초월자도 알아채지 않았을 때만 말한다, §2.11; 이 틱에 떠나는 그릇(`PendingEffect`)은 알아채이지 않는다): 그 플래그가 '한 번'의 지킴이다. 대안이던 것: ㉮ 말하기만(플래그 없이 — 세계 해시 그대로): 판에 한 번을 지킬 자리가 따로 든다 — 로그로 대신하면 연대기 상한(`ChronicleCap` 1,500)에서 그 보증이 깨지고, `Knowing`의 새 bool이면 저장 필드다(헤더 먼저); ㉯ 계승이 플래그를 끈다 — 설계 문장 셋(`crisis_and_transcendents.md:99` 등)과 코드 주석을 고치고 새 그릇에게 다시 말한다(환생의 읽힘).
    - **7-다 글** (답: 권장대로, 개발자 2026-10-31): '같은'·'다시'·'알아보'를 쓰지 않는다(§3.2 — B5 결정 12-가·12-나, `rebirth_words`·새 `awareness_words`); 그릇이 지닌 것(맡겨진 것·부름·택함)을 말하지 않고 위의 맡긴 이를 가리키지 않는다(§3.2 — 앞 판의 '맡겨진 것의 냄새'를 바꾼다); 2인칭은 이 줄만; 설계의 '초월자는 부름을 알아본다 — 같은 부름의 냄새'(`:99`·`:174`)를 **'타락은 제 손이 닿은 곳을 다스리는 이를 알아챈다'**로 고친다(답의 문장 — '맡겨진 이'는 거룩한 질서의 수동을 설계의 타락 문장에 들인다). 대안이던 것: B4 초안 그대로 — 새 그릇에게 '같은'이 '너를 다시 안다'로 읽힌다.
    - **7-라 칸과 책** (답: 권장대로, 개발자 2026-10-31): `AwarenessTemplates`를 새 칸으로 가르고 `VoiceTemplates`는 울타리의 증언(B8)에 남긴다; 범주 `Log.Transcendent`, 생애 장과 두 책에 싣지 않는다(§2.11). 대안이던 것: ㉮ 한 배열 그대로(말하기 추첨이 때를 섞는다); ㉯ 생애 장에 싣는다(행위의 책에 타락의 2인칭).
    - *왜 개발자인가*: 개발자가 승인한 B4의 초안(B4 헤더 `:336`)과 설계 문장(`:99`), 타락이 처음 플레이어를 2인칭으로 부르는 신학의 자리(`docs/70:168`).
8. **침묵과 그릇 없이 맞은 결말의 장치**(권장): §2.8·§2.9 — 조건은 B5의 후보 없음, 그 틱이 끝, `bNoVessel`, 쪽은 B5 결정 15의 차례, 화면 제목 없음. 대안 ㉮ 침묵은 지금처럼 시계가 받는다 — 판이 그 틱부터 시계까지 그릇 없이 흐른다(18,000이면 10,000틱 넘게); ㉯ 침묵의 기록을 #126이 열리는 B7로 — 기록의 종류는 지금 정해야 하므로(결정 6-가) 장치만 미루는 것이라 이득이 없다.
9. **`petition_exalt` 바로잡기**(권장): §3.1 — `CivTierAtLeast 2`, 값 그대로, 스키마 규칙 하나. 대안 ㉮ `PlayerTierAtLeast`가 몸을 입은 통치자에게 다스리는 백성의 칸을 읽게 한다 — 조건 하나의 뜻이 역할마다 달라지고 방랑자 카드의 격차 규칙(`Events/CosmicPetitions.cpp:176-180`)과 갈린다; ㉯ 개인 칸을 백성의 칸에 맞춰 올린다(칸 공명 — B5 결정 9가 미룬 것, #1 — B6b 이상) — 둘 가운데 하나만 고른다(공명이면 이 카드가 셋째 칸에 서기 시작한다); ㉰ 설계 감사의 안 — `CivTierAtLeast 3`(셋째 칸의 카드 — `DESIGN_AUDIT_AND_IMPROVEMENT.md:369`의 열린 점검 항목): A2가 뜻한 것은 둘째 칸이고(A2 헤더 `:148`·`:194`) 셋째 칸의 틱은 승천 줄 → 연출 → 드러냄이 이미 붐빈다(§2.11 — 알아챔도 그 틱을 피한다); 감사의 전제('셋째 칸의 클라이맥스')는 판이 셋째 칸에서 끝나던 때의 것이다.
10. **`Sim.NotablesFromEventsOnly`를 게임 데이터로**(권장): §6 — 생애의 `BornTick`으로 자르는 보수적 허용. 대안 ㉮ 도우미 하나(가칭 `CosmicChronicle::PeoplesOfLivesBornBy(Tick)`) — 다스린 백성의 즉위 틱이 저장되지 않아 더 정확해지지 않는다(`ReignTicks`는 재위의 길이라 — `Core/GalaxyState.h:519`·`:564` — 죽음이나 지금에서 거꾸로 셀 수 있는 것은 그 생애의 마지막 재위뿐이다); ㉯ 죽음을 뺀 테이블 그대로 두고 계승을 건너는 작은 판을 더한다 — 게임 데이터의 인물 기록을 보는 테스트가 여전히 없다.
11. **커맨드렛과 측정 인자**(권장): §5.1 — 새 줄 `ending:`, `end check`의 새 뜻, `-ExaltAsBefore`. 대안: `ruler:` 줄에 덧붙인다 — 세계가 그대로여도 러너가 견주는 글자가 옮는다.
12. **측정**(권장): §5.2 — 묶음 0·1과 미리 적은 판단, B6a에서는 노브를 바꾸지 않고 수를 넘긴다(B6b가 세계를 다시 바꾸므로 두 번 재지 않게), #129는 재지 않음. 대안 ㉮ 판단이 부르면 B6a 안에서 노브를 바꾼다(지킴·버퍼) — B6b의 넷째 칸이 그 세계를 다시 옮겨 같은 노브를 두 번 잰다; ㉯ 묶음을 B6b와 합친다 — 셋째 칸 뒤 계절의 효과와 넷째 칸의 효과를 짝으로 가를 수 없다.
13. **표현**(권장): §4 — 연대기는 기록을 받는다, 시험판 알림은 시계에만, B6a의 규칙 줄은 시계만, 예고 그대로, `-ProvisionalEnd=0`은 세션 동안 끝을 보지 않는 스위치 그대로(그 세션의 `AdvanceLeap`은 도약을 다 넘긴다). 대안 ㉮ 규칙 줄에 다섯째 칸을 지금 적는다 — B6a의 데이터로는 오지 않는 끝을 약속처럼 읽힌다(다음 칸 툴팁도 '셋째 칸에서 오르는 행이 없다' — `UI/CosmicNextRung.h:58`); ㉯ 시험판 알림을 모든 끝에 — 다섯째 칸·침묵의 끝은 시험판의 끝이 아니다.
14. **주장의 경고 줄과 반란 문턱의 표시**(권장 — B6a의 화면 커밋): §4 — 순수 함수 셋(시뮬의 주장 규칙이 `ClaimRuleHoldsOf`를 부르고, 화면의 `ClaimMayFollowOf`는 만료 앞의 문 — 답인 칙령·초신성·주장 — 까지 본다; 반란 문턱의 안정도는 시뮬이 세는 상수 하나). 셋째 칸 뒤의 계절이 다스리는 성계의 주장을 보통의 일로 만드는 때라 쓸모의 때다(A8 헤더 `:183`이 'B4/B6'으로 넘겼다). 대안: Phase 5의 UX로(새 #) — 계절이 열리는 블록에서 경고 없이 주장이 선다.
15. **셋째 칸 끝을 떼는 일과 기록을 쓰는 일을 장치에서 가른다 — `[B-6a3]`은 세 커밋**(권장): §5.3·§8 — `[B-6a3a]` 셋째 칸 뒤의 판(`EndRung`과 `Rung` 가지를 지우고 알아챔 플래그를 더한다 — 정책 판 셋과 정책 해시 판의 세계가 옮고 깨질 수 있는 확인 ⓐ~ⓓ가 '세계를 옮기는 길은 정책이 끝을 읽는 자리와 알아챔 플래그뿐'임을 보인다), `[B-6a3b]` 기록 쓰기(`EndingOf`와 열째 단계 뒤의 한 번 — 6,000에 닿는 세 판의 두 해시만 옮고 CSV의 결정적 칸은 다섯 판 모두 `[B-6a3a]`와 같다; B3 `[B-3a]`의 꼴), `[B-6a3c]` 장치(읽는 곳을 기록으로, `RunLeap`의 멈춤·`AdvanceLeap`·침묵의 래치, `ECosmicProvisionalEnd`를 지움 — 다섯 기준값과 해시 판 둘이 **비트 그대로**, `WORKPLAN.md:61`의 '장치(기준값 비트 그대로)'를 지킨다). 지금 셋째 칸은 저장되지 않는 순수 함수 안에만 있고(`Sim/CosmicEndings.cpp:15-21`; 열거는 UENUM이 아니다 — `.h:13-19`) 기록의 다섯 종류에 셋째 칸이 없어, 기록보다 **먼저** 떼면 임시 태그도 비계도 들지 않는다 — 피할 수 없는 옮김은 기록 쓰기 하나뿐이다. 값: 몇 테스트 파일을 두 번 고친다(`[B-6a3a]`의 셋째 칸, `[B-6a3c]`의 래치 — §6). `[B-6a7]`이 `WORKPLAN.md:61`의 B 행에 이 꼴을 한 줄로 적는다(§7). 구현 리뷰는 `[B-6a3c]` 전에 세 커밋을 함께 보고, 앞 두 커밋의 코드를 고쳐 기준값을 옮기는 고침은 `[B-6a3c]`가 아니라 그 앞의 `[B-6a3r]`에 든다(§8 — 푸시된 커밋은 고쳐 쓰지 않는다). 대안 ㉮ 한 커밋(앞 판의 꼴) — 확인이 기준값의 옮김을 하나로 묶어 장치의 결함이 옮김 속에 숨고, 장치 커밋이 B 행의 '비트 그대로'를 어긴다; ㉯ 기록을 쓰되 해시에서 뺀다 — 결정 3의 ㉮.
16. **커밋 순서와 과정**(권장): §8의 표 그대로 — 테스트 → 필드만(문서 먼저) → 셋째 칸 뒤 → 기록 쓰기 → 장치 → 표현 → 글과 목소리(ultracode 글 감사) → 데이터 켜기와 밸런스 → (고침) → 문서. 대안 ㉮ 알아챔의 플래그를 글 커밋에 — 글 커밋의 세계 해시가 옮아 '말하기만'의 문(B4 `[B-4d1]`·B5 `[B-5d1]`)이 깨진다; ㉯ `petition_exalt`를 장치 커밋에 — 해시를 옮기는 길과 데이터 켜기가 한 커밋에 섞인다.
17. **넘길 것과 새 열린 물음**:
    - **17-가 마일스톤 B 안에서의 넘김** (권장): §0의 'B6a 밖' — 새 행 #155(물리 연구의 함대 승수 → B7)·#156(영압이라는 업적 기회 → B6b, 업적 행과 함께 — `crisis_and_transcendents.md:115`의 'B6, 업적과 함께'; 별칭의 꼴은 노트로)·#158(역할이 바뀐 첫 틱의 카드 → B7)·#159(울타리의 증언 → B8)·#160(넷째·다섯째 칸 업적 행의 꼴과 VS 정의, 칙령 범위의 넓힘 → B6b)·#161(셋째 칸 위의 AI와 공정성의 띠 → B6b)·#162(보이지 않는 백성이 낸 엔딩의 글 → B6b)·#163(T5 문턱 묶음의 신학 판정 → Phase 6+ 헤더 — 개발자의 답이 Phase 6+로 보낸 묶음); 걸림만 고치는 행은 노트 §0.2. 대안: 희년·물리 승수를 B6a에 — B6a의 주제와 닿지 않고 B7의 AI 조약·함대 커밋과 두 번 잰다.
    - **17-나 [개발자] 위기 청원의 낮은 선택을 마일스톤 B 밖(Phase 5)으로** (답: 권장대로, 개발자 2026-10-31 — **Phase 5의 첫 60분으로 — 새 #157**): 들인다면 해결이 아니라 함대 없는 시간이고 첫 위기 검사(`Tests/CosmicDataTests.cpp:2454`)를 그 유예까지 넓혀야 해 첫 60분의 손질과 한 덩이다; B6a·B6b의 주제(끝·높은 칸)와 닿지 않는다. 대안이던 것: ㉮ B6b에(업적 행과 함께 — 닿는 곳이 없다); ㉯ B7에(위기·함대의 블록). *왜 개발자인가*: 승인된 `docs/50:12`의 B6 칸에 적힌 항목을 마일스톤 B에서 뺀다(결정 1의 까닭 — 승인된 계획의 범위). `[B-6a7]`이 `docs/50:12`를 답대로 고친다.
18. **손 확인의 꼴**(권장 — B5 결정 22의 답을 그대로 따른다): §9 — 문으로 걸지 않고, `-SaveAt` 세이브를 '이어하기'로 연다. 대안: 보통의 판을 손으로 셋째 칸까지(×10 4분 남짓)와 시계까지 — 알아챔·공백기 안의 끝을 볼 판을 손으로 고를 수 없다.
19. **문서**(권장): §7. 대안: 설계 문장 고침을 B6b와 한 번에 — 그 사이 설계가 B6a의 코드와 어긋난다(침묵의 조건·시뮬이 끝을 모른다는 문장).
20. **[개발자] `CLAUDE.md` 규칙 4·13·15** (답: 권장대로, 개발자 2026-10-31 — **`[B-6a2]`의 문서 먼저에서, 규칙 15의 '현재 코드' 가리킴만 `[B-6a7]`에서** — B5 결정 21의 꼴): 규칙 4(`CLAUDE.md:37`)의 끝에 '판의 끝(B6a)은 열째 단계 뒤 한 번 적힌다(`Ending` — 추첨 없음, 먼저 온 끝만); 시뮬은 끝 뒤에도 돌고 `RunLeap`만 그 틱에서 선다'(초안 약 +230바이트); 규칙 13(`:46`)의 단일 창구 목록에 '엔딩은 `CosmicEndings::EndingOf`, 침묵은 `CosmicSuccession::IsSilent`, 주장 규칙은 `CosmicHazards::ClaimRuleHoldsOf`(화면의 답은 `ClaimMayFollowOf`)(B6a)'(약 +175 — 초안 169바이트; 셋째는 `[B-6a4]`부터, §7의 표지); 규칙 15(`:48`) 'A0~B5' → 'A0~B6a'. 규칙 6(`:39`)은 바뀌지 않는다(기록은 두 해시에 들고 빼는 목록이 그대로다); 규칙 14(#107)는 B6b. 크기: 지금 15,880바이트(`Tools/check_docs.py:42`의 16,384까지 504) — 이 고침 뒤 약 100 남는다(504 − 230 − 175 − 규칙 15의 1); 모자라면 규칙 4의 B2·B3 괄호를 `docs/20`으로 옮긴다(B4 결정 20의 꼴). 대안이던 것: ㉮ 셋 다 `[B-6a7]`에서 — 워크플로 일꾼이 `[B-6a3]`~`[B-6a6]`의 리뷰에서 낡은 규칙(끝이 없는 열 단계)을 읽는다; ㉯ 규칙 4는 그대로 두고 `docs/20`만 — 모든 세션이 읽는 규칙 파일이 틱의 끝을 빠뜨린다. *왜 개발자인가*: 모든 세션이 읽는 규칙 파일의 문장이다(B4 결정 20·B5 결정 21과 같은 까닭).
21. **[개발자] 마일스톤 B의 종료 조건** (답: 권장대로, 개발자 2026-10-31 — **B 끝 칸에 '엔딩 1회 콘솔 없이'를 더하되 그 엔딩은 B6b가 닿게 하는 둘(집단 승천·다른 질서) 가운데 하나로; 옛 §12.1의 수량(업적 4·도달 가능 등급 T1→T3·넷째 칸 이상 1체 — `docs/50:268`·`:270`·`:274`)은 B6b 헤더가 업적 행의 꼴과 함께 고친다; B6a는 끝 칸에 '셋째 칸 뒤의 계절을 시계까지 콘솔 없이 한 번'만 더한다**): 지금 종료 조건은 MVP의 VS 정의에서 와 엔딩을 요구하지 않는다(`docs/50:12` 끝 칸). 대안이던 것: ㉮ 그대로 — 마일스톤 B가 끝나도 엔딩을 사람이 본 적이 없다; ㉯ §12.1의 수량을 지금 고친다 — 업적 행의 꼴(B6b) 전에 수만 정한다. *왜 개발자인가*: 승인된 마일스톤의 종료 조건과 VS 정의를 바꾼다.

**B6b 헤더로 넘기는 결정**(B6a의 답이 아니다 — B6b 헤더가 묻는다): 통치자 T4→T5의 '개인 칸 ≥ 3'과 칸 공명(#1 — 그 요구를 업적 행에서 뺄지, 공명을 넣을지 — B5 결정 9가 미룬 것), 넷째·다섯째 칸 업적 행의 꼴(지금 장치로 된 단순한 행 대 설계 표 — #160), 셋째 칸 위의 AI(넷째 칸 제동 `AIFourthRungCap`의 꼴과 값 — B6b에 드는 것은 결정 1의 답이 정했다; 셋째 칸 위만의 지연·공정성의 띠 — #161, 섭정 아래 옛 백성의 다섯째 칸 #121), 칙령 범위의 넓힘(#160과), 방랑자의 영압 선택(#141 — #1의 칸 공명과 함께), 영압이라는 업적 기회의 꼴(#156). **Phase 6+로**: T5 문턱 묶음의 신학 판정(#163 — '함께 앉았다'는 #14로).

## 11. 구현 기록 (헤더와 다르게 하거나 더한 것 — 커밋마다 더한다; 까닭과 수는 `DECISIONS_LOG`)

- **`[B-6a1]`**(2026-10-31): `Sim.NotablesFromEventsOnly`를 게임 데이터로 되돌리고, 시드 42의 20,000틱에서는 반란 기록이 로그를 떠나기 전에 `MaxNotablesPerCiv`(5)로 먼저 밀려나므로 끝에 5성계 판의 #118 경계(로그를 떠난 뒤의 계승)를 함께 두어 두 변이가 모두 깨지게 했다. 세부는 노트 §5.

- **`[B-6a2]`**(2026-10-31): 필드만 — `FCosmicEndingRecord`·`FGalaxyState::Ending`(최상위 멤버 20)·태그 다섯·`CosmicTags` 다섯과 문서 먼저(§7의 첫 목록). 헤더와 다르게 하거나 더한 것 셋: ① 손 확인 ④의 옛 세이브(§9의 3 ④)는 HEAD `b10be48`이 아니라 `[B-6a1]`(`62404f0` — 테스트만 바꾼 커밋이라 게임 코드는 `b10be48`과 같다)의 빌드로, 이 커밋의 코드를 고치기 전에 만들었다; ② ultracode 리뷰가 찾은 낡은 현재 문장(열린 물음 #117의 '저장 필드가 없다', 용어집 '임시 끝', `docs/40`의 `Sim.TellStreamSeparate` 멤버 수, STATUS의 기준값·테스트 수)과 `docs/20` 불변 규칙 전문의 규칙 13을 이 커밋에서 고쳤다(§7의 첫 목록 밖 — 같은 커밋이 그 문장을 거짓으로 만들었다); ③ `[B-6a3a]`의 깨질 수 있는 확인 ⓑ~ⓓ가 견줄 `-PastEnd=1` 다섯 판을 이 빌드로 만들어 두었다(§5.3 — 그 커밋 뒤에는 이 빌드가 없다). 세부는 노트 §5.

- **`[B-6a3a]`**(2026-10-31): 셋째 칸 뒤의 판 — `ProvisionalEndOf`는 시계만 답하고(`EndRung`과 `Rung` 가지를 지움; 열거자 `Rung`과 `NameOf`의 그 가지는 §2.2대로 `[B-6a3c]`까지 남는다), 알아챔 플래그를 처음 쓴다(§2.11). 헤더와 다르게 하거나 더한 것 여덟: ① `TryPlaceClaim`은 플래그를 세운 주장자의 번호를 돌려줄 뿐 아니라 `CosmicHazards::Place`의 결과를 보고, 놓지 못했으면 플래그를 쓰지 않는다(전에는 그 결과를 버렸다); 만료 처리는 그 번호를 `[B-6a5]`의 줄을 위해 받아 둔다; ② 연대기 `NoteOf`의 칸 가지는 백성을 찾지 않고 늘 `Chronicle.Note.RungNoCiv`로 답한다(`Rung`이 열거에 남는 동안의 자리 — 백성을 부르는 줄은 `[B-6a3c]`가 기록의 `CivID`로 되살린다, §2.7); ③ 확인 ⓓ: 켠 빌드의 `-PastEnd=1` 다섯 판의 두 해시가 이미 `[B-6a2]`의 `-PastEnd=1` 판과 비트 그대로다(정책이 도는 동안 다스리는 백성의 성계에 주장이 서지 않는다 — 'claims on ruled 0'); 탐침 빌드는 주장이 서는 하나뿐인 판 — 스위치 없는 통치자 20,000틱, 입력이 시계 6,000에서 멈춘 뒤 셋째 칸 백성의 성계에 주장 둘 — 에 돌려 그 판의 옮김을 둘로 갈랐다(플래그 없이 1072463830 / 4098835384, 켠 빌드 1219932898 / 330974340 — 그 판의 옮김은 셋째 칸 끝을 뗀 것과 끝 뒤의 알아챔 플래그 둘이다); ④ 알려진 사이의 어긋남 — 상단 바 시계 툴팁의 규칙 줄(`UI/CosmicTopBarWidget`의 `ClockTipOf`가 보이는 ST_CA `Chronicle.Rule`, 노브 0이면 `Chronicle.RuleRung`)은 아직 '셋째 칸에 오르면 멈춘다'고 말한다 — 규칙 줄을 고치는 `[B-6a4]`(§4)까지 남는 거짓이고 그 커밋이 고친다; ⑤ `Sim.AwarenessOnce`의 떠남 두 경우는 §6의 반란·각성 만료 대신 대역으로 세웠다 — 폐위는 `DeposeRuler`를 만료 효과로 지닌 테스트 역병이 같은 만료에서 주장 앞에 기록하고, 죽음은 틱 전에 손으로 기록한다(성계의 `KillPlayer`는 그 성계에 선 방랑자에게만 닿아 — `CosmicHazards::VesselAtRiskOf` — 통치자에게 같은 만료의 죽음은 생기지 않는다); 같은 `PendingEffect`의 문을 닫는다; ⑥ 리뷰가 더한 두 경우 — 넷째 칸 백성의 성계(셋째 칸 '이상', 변이 `b6a3a_aware_rung_exact`)와 다스리지 않는 셋째 칸 백성의 성계(주인의 문, 변이 `b6a3a_aware_no_owner_gate`); ⑦ `docs/20`의 임시 끝 단락 머리에 '`[B-6a3a]`부터 셋째 칸은 판을 끝내지 않는다'는 표지 한 문장(같은 커밋이 그 단락의 칸 끝 문장을 거짓으로 만들었다 — 단락 전체는 `[B-6a7]`); ⑧ `TryPlaceClaim`의 돌려준 번호와 '이미 알아챘으면 쓰지 않는다' 가지는 지금 아무 상태도 바꾸지 않아 이 커밋의 테스트로는 깨지지 않는다 — 그 계약 위에 줄을 세우는 `[B-6a5]`의 `Log.AwarenessVoice`가 쌍둥이(미리 알아챈 타락 — 줄 없음)와 타락 둘(둘째가 알아채도 둘째 줄 없음)을 단언하고 그 가지를 떼는 변이를 함께 둔다. 세부는 노트 §5.

- **`[B-6a3b]`**(2026-10-31): 기록 쓰기 — `CosmicEndings::EndingOf`(§2.2의 표 그대로)·`NameOf(태그)`, `CosmicSuccession::IsSilent`(컨트롤러의 `CosmicCallingFlowImpl::IsSilent`가 부른다), `SimulateTick`의 열째 단계 뒤 한 번 쓰기(§2.3), `RunHeadless`의 약속 고쳐 적기(§2.5), 커맨드렛의 새 `end check`(§5.1). 헤더와 다르게 하거나 더한 것 넷: ① `end check`는 §5.1의 '처음 선 경계에서 네 칸이 같다'에 더해 '그 전에는 기록이 비어 있다'와 '그 뒤에는 기록이 바뀌지 않는다'도 본다 — 어긋나면 에러 줄과 끝 코드 1, 통과하면 아무것도 적지 않는다(셋 다 같은 '기록을 쓴 자리와 경계 사이에 상태가 바뀌지 않는다'에 기댄다); ② `Sim.ProvisionalEnd`의 '노브가 세계를 바꾸지 않는다'(시계 200 대 0)는 이제 시계의 틱이 기록을 쓰므로 기록을 떼어 두고 견주며 시계 판의 기록(`Ending.Clock`·200)을 단언한다(그 경우는 `[B-6a3c]`에서 `Sim.EndingOf`로 옮는다); ③ 테스트의 산 판(`Sim.EndingRecordOnce`·`Sim.EndingInert`)은 `KillPlayer`를 뺀 테이블로 돈다 — 시험이 기록한 죽음만 나게(자연의 죽음과 시계 전의 침묵이 두 기록을 같게 만들지 않게); ④ `docs/21`의 '지금은 아무도 쓰지 않는다' 둘(`bAwareOfPlayer`·`Ending`)은 이 커밋과 앞 커밋으로 거짓이 되어 지웠다(`[B-6a3a]`·`[B-6a3b]` 표지는 결정 20대로 `[B-6a3c]`에서 뗀다). 세부는 노트 §5.

- **`[B-6a3c]`**(2026-11-01): 장치 — 끝을 읽는 곳(컨트롤러 래치 `FollowEnd`·저장 층 `MarkOf`·두 정책·커맨드렛 `end:`·연대기의 서명)을 기록으로 옮기고, `RunLeap`이 기록이 적힌 틱에서 서며(부른 때 기록이 비었을 때만), `AdvanceLeap`은 시계로 자르지 않고 스위치가 꺼지면 한 번 더 부른다; `ProvisionalEndOf`·`ECosmicProvisionalEnd`·열거의 `NameOf`를 지웠다; 커맨드렛의 `ending:` 줄; 도우미 `CosmicTests::EndRunAt`; `Sim.ProvisionalEnd`는 `Sim.EndingOf`로 옮겨 지웠다; `docs/30` 다시 만듦; `[B-6a3*]` 표지를 뗐다(결정 20). **다섯 기준값과 해시 판 둘은 비트 그대로.** 헤더와 다르게 하거나 더한 것 넷: ① 세 장치 커밋의 공동 구현 리뷰에서 기준값을 옮기는 고침이 나오지 않아 `[B-6a3r]`은 없다; ② 리뷰가 더한 테스트 — 침묵에 머문 옛 세이브(다음 틱에 `Ending.NoCandidate`), 기록을 지난 세이브의 `CosmicEnd` 줄이 기록의 틱을 말함(`Sim.SaveOldEndFlows`의 (3)·(4)), 두 정책이 시계가 아니라 기록을 읽음(`Sim.WandererPolicy`·`Sim.RulerDiplomacyPolicy` — 시계 없는 기록은 멈추고 기록 없는 시계는 멈추지 않는다); ③ `ending:` 줄에 리뷰가 더한 칸 넷 — `petition_exalt`의 미리 셈(틱:꼴:행), #135의 열쇠(틱:성계:단계:꼴 — 묶음이 (시드, 틱, 성계)로 한 번 센다), #132 다스리는 백성의 영향권 안 소모(단계별), #144 비축의 단계별 가름; ④ 변이 `b6a3c_advanceleap_clock_cut_back`(시계 자르기를 되살림)은 같은 꼴이라 지웠다 — 스위치가 켜지면 `RunLeap`이 시계 이하의 첫 기록에서 이미 서므로 자르기가 아무것도 바꾸지 않는다. 묶음 0(570판)의 미리 셈한 판단(노트 §3): ⓑ 계절의 결정 밀도 지킴(R·느린 R 모든 판 ≥ 1, 중앙값 4); #136 9/30 — 닫지 않음(B6b로); #135 159건 중 잃음 1.3% — 닫음(묶음 1에서 다시); #142 '재지 않음 — 잰 20'; #132 계절의 지연 몫 32~35% — 띠 안; #145 '재지 않음 — 잰 0'; #140 끝 전 0. 세부는 노트 §5.

(헤더 리뷰의 기록은 노트 §4, 커밋마다의 세부는 노트 §5.)

## 추가 이야기와 보완

- **장치의 구현 리뷰가 먼저 볼 넷**: ① 기록 쓰기가 열째 단계 **뒤**이고 단계 가드 안인가(같은 틱의 죽음 → `bNoVessel`; 침묵의 틱), ② 스위치가 꺼진 세션의 `AdvanceLeap`이 끝에서 멈춘 `RunLeap`을 한 번 더 부르는가(공백기에 그릇 없이 남지 않게 — 되풀이하지 않고), ③ 알아챔의 조건이 앎을 한 줄도 읽지 않는가(그리고 이 틱에 떠나는 그릇을 거르는가), ④ 세 장치 커밋의 확인이 저마다 제 옮김만 보이는가(`[B-6a3c]`의 비트 그대로가 장치의 증거다).
- **청원 행이 설 수 있는가의 정적 검사**: `petition_exalt`는 `[A-2b]` 뒤로 서지 않았는데 아무 검사도 알리지 않았다. §3.1의 스키마 규칙은 그 꼴 하나(통치자 행의 개인 칸 조건)만 막는다. 조건의 칸 척도(1~5)와 개인 칸·백성의 칸을 섞은 행을 더 넓게 찾는 검사는 데이터 행만 보는 작은 일이라 `[B-6a6]`의 리뷰가 더 필요하다고 보면 그 커밋에 넣는다(설계를 바꾸지 않는 보완).
- **B6b에 넘길 생각**: B6a의 묶음 0이 재는 '첫 AI 셋째 칸의 정확한 틱'과 '연구 1.0의 틱'이 B6b의 경주를 미리 셈하는 입력이다 — B6b 헤더는 그 수로 넷째·다섯째 칸 행의 진척과 AI 지연을 정한다. 넷째 칸 백성은 넷째 칸 타락의 안정도 소모를 받지 않으므로(`Sim/GalaxySimulationSubsystem.cpp:1335` — 지배 원천의 칸이 그 백성보다 높을 때만) 다스리는 백성이 넷째 칸에 서는 순간 B4의 소모 놀이가 끝난다 — B6b의 세계 수가 움직이는 까닭으로 미리 적어 둔다.
- 이 헤더가 든 성경 구절은 창 3:5 하나이고 장·절만이다 — 피하는 겹침으로 이미 적힌 자리(`docs/90_LORE/theology_and_bestiary_notes.md:247`)를 가리킬 뿐 설계에 새 근거로 넣지 않는다(대조 목록 `:259`에 더하는 일은 없다).
