# 로드맵 (ROADMAP)

> 2026-10-18 재편. 앞부분은 개발자가 채택한 3단계 로드맵(1단계 브리핑 §7), 가운데는 지금 살아 있는 작업 표(옛 WORKPLAN §3), 뒤는 옛 guideline §11~§13 원문이다. 일정 단위는 '주'가 아니라 **작업일(세션)**이다 — 옛 §11의 주 단위 계획(P0~P3 약 13주)은 9일에 끝났으므로 §11의 기간 열은 쓰지 않는다. 번호 체계는 여기의 마일스톤(0·A0·A·B·C)과 작업 표의 번호 둘만 쓴다.

## 마일스톤

| 단계 | 작업 묶음 | 종료 조건(검증 가능) | 작업일 | 의존·리스크 |
|---|---|---|---|---|
| **0. 정리** | Phase 3 꼬리 7·12·8·9(끝, 2026-10-18~19) → 손 확인(개발자, 9번까지 든 빌드로) → Phase 3 종료 선언·종료 리뷰 10번 / 문서 재편(끝, 2026-10-19) / 러너·레이어 린트·문서 검사·설정 표 생성기(끝, 2026-10-19 — Windows 러너는 첫 실행에서 확인) / `FCosmicEvalContext` 독립 헤더(Windows 빌드 필요) / 시드 42 기준값 첫 측정(`run_checks.ps1 -UpdateBaseline`) | Phase 3 종료, 규범 문서 ≤ 200KB, 러너로 전체 검사 1회 | 3~5 | 손 확인 1건이 A0·A를 직렬로 막는다 — 60fps 미달이면 '게임 스레드 ≤ 10ms'로 재정의하고 넘어간다 |
| **A0. v0.8 예약 ①** (끝, 2026-10-19) | 4a 성역(`sol` 배열 끝·`sys_0`·`Law.Hedge`·`IsSanctuary`) + 4b-lite 첫 생애 행 뼈대(`arch_seeker`·첫 줄 "{HomeSystem}. 이 백성이 당신에게 맡겨졌다."·의회/회중 화자) — 가칭 식별자 일괄 확정, 헤더 한 번, 리베이스 한 번 | `Sim.SanctuaryKept`, 재측정 테스트 ~40곳, 개발 세이브 삭제, ultracode 리뷰 1회(저장·결정론) | 3~5 | ID·시드 기준값이 바뀌므로 MVP 작업보다 앞에 둔다(두 번 바꾸지 않기) |
| **A. Playable MVP** | ① 자동 일시정지(Severity 2·자기 성계 위기·카드, 기본 켬 — 끝, 2026-10-20) ② 청원 카드 10장 + 카드 위젯(`FEncounterDef` 소비; 선행 `DT_Encounters` 스키마·중첩 구조체 검사 — 끝, 2026-10-20: 헤더 `docs/60_DECISIONS/headers/A2_petition_cards.md`, 카드 글 묶음 감사는 남았다) ③ 첫 10분 보장(공통 첫 위기·첫 청원) + #68 안정도 회복·#71 AI 구호 정책·#72 전조 확률·T2→T3 둘째 업적(끝, 2026-10-22: 헤더 `docs/60_DECISIONS/headers/A3_first_ten_minutes.md`) ④ 임시 끝(T3 ∨ 틱 6,000) + 연대기 최소형(끝, 2026-10-23: 헤더 `docs/60_DECISIONS/headers/A4_provisional_end.md` — 판의 길이는 `docs/00_VISION.md` MVP 범위. 손 확인은 남았다) ⑤ 저장 최소형(틱 경계 자동 저장·입력의 저장·이어하기·백업 1·연대기 이관 #40 ⑦·압축과 검사하는 머리 #70 — 끝, 2026-10-23: 헤더 `docs/60_DECISIONS/headers/A5_save_minimum.md`, 구현 리뷰 반영. 손 확인 ⓐ는 남았다) ⑥ 남은 지표 2(Stability 60틱 추세·위기 발생/만료 줄의 카메라 포커스 — '다음 칸까지' 툴팁과 명령 단추의 거절 사유·'멈춤 — 자원 부족'은 Phase 3 12번 작업에서 끝남)·온보딩 3줄(끝, 2026-10-24: 헤더 `docs/60_DECISIONS/headers/A6_feedback_and_onboarding.md` — 위기 포커스는 자동 정지가 멈춘 새 위기에만, 온보딩 3줄은 화면의 안내 셋. 손 확인 ⑥은 남았다) ⑦ 연구 중점 프리셋(끝, 2026-10-24: 헤더 `docs/60_DECISIONS/headers/A7_research_focus.md` — 다스리는 성계의 패널에 단추 넷과 명령 하나, 판의 길이는 ⑧에서. 손 확인 ⑦은 남았다) ⑧ 경제 상수 → DT_Rules(판의 속도를 `-Focus=0`·`-Focus=1` 두 정책 판으로 함께 재고 둘 다에 목표를 둔다 — 손잡이는 MVP ④ 결정 1대로 업적의 조건이 먼저, MVP ⑦ 결정 9) ⑨ Win64 패키징 | ⓐ 새 판 → 저장 → 재시작 → 이어하기 ⓑ 커맨드렛 30판(`-TellAll=0 -Answer=1 -Build=1 -Focus=1`) 첫 600틱 말해진 줄 중앙값 ≥ 20·판의 끝까지의 결정 중앙값 ≥ 15(`decisions by end` — 끝 뒤의 입력은 세지 않는다, MVP ⑦ 결정 11. 2026-10-24에 12라 닿지 않는다 — ⑧이 판의 길이와 함께 본다. 그 전의 꼴 '첫 6,000틱 결정 ≥ 15'는 MVP ③ 결정 7) ⓒ 마우스만 ×10으로 T3 → 연대기 화면 ⓓ 쿠킹 빌드 시드 42 세계 해시 = 에디터 ⓔ 외부 1명이 설명 없이 30분 뒤 "다음에 뭘 할지 알겠다" | 12~18 | 폐위 '실패 끝'은 칙령 유지비(B2) 뒤에야 30~60분 안에 성립한다; 텍스트는 코드 속도로 나오지 않는다 |
| **B. 코어 시뮬 완성** | B1 방랑자 최소 루프(TravelTo·스캔·조우 5장·즉위) → B2 연구 중점·칙령 9 → B3 외교·분쟁(함대 노브 4개 소비) → B4 초월자 목소리·교리 3행·영압 카드 → B5 사망·계승·공백기·두 책(Phase 4b) → B6 T3→T5 업적·T5 문턱·엔딩 4 → B7 AI 백성 5행·AIProfile 행동·고넬료 사슬 → B8 서막 문안·실제 하늘(단계 5 — 늦어도 Phase 5 전) | 옛 §12.1 수량(아키타입 7·Hazard 8·조우 25+청원 10·업적 4·프로젝트 7·칙령 9) + 옛 §9.3 목표 ±30%(주장 만료 40~60%, 안정도 0.3 미만 백성 < 25%, 폐위율 20~35%) + 계승 1회 콘솔 없이 | 35~50 | B8 실제 하늘은 모든 시드 기준값을 다시 바꾼다(두 번째이자 마지막 리베이스) |
| **C. 폴리싱** | Niagara 3종·로그 튜토리얼·영어 로컬(Internationalization 설정부터)·'세계관과 출처'·'옛 땅의 기억' 화면·Enhanced Input·UI 테마/DPI·설정 화면·Steam 페이지·테스터 5~10 | 첫 10분 이탈 < 10%, '설교처럼 느낀 순간' ≤ 1회/세션, 테스터 과반 동의 2문항 | 20~30 + 테스터 대기 | 테스터 모집이 외부 의존 |

## 당길 것 / 미룰 것 (옛 계획 대비)

| 항목 | 옛 계획 | 판정 | 이유 |
|---|---|---|---|
| 저장 최소형(MVP ⑤에서 끝 — 2026-10-23: 틱 경계·입력의 저장, 시드 42 EA 20,000틱 슬롯 약 86 KB) / #68·#71·#72(MVP ③에서 끝 — 2026-10-22) | 단계 6 앞 / MVP ③ 헤더 | **A(MVP)** | (닫힘) / 세계가 고장 나 보임 |
| 4a 성역 + 4b-lite 첫 생애 행 | v0.8 예약 ① | **A0(MVP 앞)** | ID·기준값을 두 번 바꾸지 않는다 |
| 실제 하늘 ②·`DT_Planets` | 단계 5(8~11일) | **B8** | 플레이어 동사를 더하지 않으면서 모든 기준값을 바꾼다; 자료 라이선스(HYG·Gaia DR3) 확인만 지금 |
| 서막 8장 / 고넬료 사슬·옛 땅의 기억 | Phase 4 첫 블록 / 단계 7 | **카드 위젯은 A, 문안은 B8 / B7·C** | 단계 6 전에는 '지혜로운 마음'이 첫 청원(화자 의회)이라는 대체 경로가 설계에 있다 |
| 꼬리 7 영적 시야 / 12 ×25·명령 사유 / 8 앎의 표시 / 9 우주 화면 | Phase 3 꼬리 | **끝**(원격 커밋 94e8706·b8803ef·25104b2·25ab43b, 2026-10-18~19) | 다른 세션이 손 확인을 기다리지 않고 끝냈다 — 브리핑의 '미루기' 판정은 무효. 남은 것은 손 확인과 종료 리뷰 10번 |
| 건넘 6a·요약 은하 | EA 빌드 | **EA 이후 재심** | 플레이어는 건너지 않는다(#47); 추첨 지점 추가 비용 |

**Cut(VS 표에서 제외)**: 네 길 중 3길·교리 3행·독립 초월자·모티프 24종(이미 Phase 6+), Niagara 승천 연출(C++ 오버레이 확정), 수동 저장 메뉴(아이언맨 유지). **Defer**: 건넘 6a(EA 이후), 실제 하늘(B8), 서막 전문·헌정명 뱅크·옛 땅의 기억 화면(B8/C), `DT_Planets`·Tests 별도 모듈·Enhanced Input(C).

## 리스크

| 분류 | 리스크 | 완화 |
|---|---|---|
| 범위 | Phase 4가 체크리스트로 팽창(옛 문서에서 276회 언급), 열린 미결 58행 | B1~B8 블록마다 종료 조건; MVP를 막는 미결만 STATUS Top 5 |
| 기술 | 기준값 리베이스(A0·B8), 쿠킹 해시 미검증, Windows 전용 명령 | 리베이스는 두 번으로 고정; A 종료 조건 ⓓ; 경로 변수화 |
| 밸런스 | 회복 규칙을 넣으면 입력 없는 판의 폐위가 사라질 수 있음 | '#68만' 판 선측정(MVP ③에서 쟀다: 회복만이면 30판 가운데 0판, 구호 정책과 함께 8판 — 시드 7은 사라지고 18이 남았다), 칙령 유지비와 함께 폐위율 20~35% 재조준 |
| 콘텐츠·IP / 시장 | 대도감 차용 고유명·'영혼' 어휘가 사람 검토에만 걸림 / 한국어 우선 텍스트 4X의 좁은 시장, 신학 프레임의 양방향 리뷰 | 원형 장부 `archetype_ledger.json` + 스키마 낱말 경계 검사 / '미선언·미은폐' 유지, 개역한글↔영어 어휘 대응표를 Phase 5 전에 |
| 번아웃 | 8일 99커밋, 리뷰→재감사 3중 루프, 코드보다 큰 문서 | 1세션 1작업, 해시 커밋 폐지, 규범 문서 ≤ 200KB, MVP를 먼저 쥐어 보상 확보 |

---

# 현재 작업 표 (옛 WORKPLAN.md §3, 2026-10-18 기준 — 상태 열의 커밋 해시는 역사 기록이며 앞으로는 날짜만 적는다)

> 아래 표의 'v0.8 단계 0~8'은 위 마일스톤에 이렇게 대응한다: 단계 3(Phase 3 꼬리) → 0·A·B4·C / 단계 4 → A0 / 단계 5 → B8 / 저장 작업 → A / 단계 6·7 → B7·B8 / 단계 8 → C.

## 3. 다음 작업 (2026-10-18 기준)

1. guideline.md는 **v0.8**이다(2026-10-18 — 성역·성역 밖 찾는 백성의 첫 생애·서막과 부르심·시간 표시·실제 하늘·대도감 통합. 변경 이력의 'v0.8 변경 요약', 결정은 §16 'v0.8 확정'). v0.7(2026-10-12: 은하 여럿·네 층·앎·다리와 건넘·현실 수치 — 변경 이력의 'v0.7 변경 요약')은 그 앞이다. CLAUDE.md의 불변 규칙도 v0.7·v0.8로 바뀌었고, 코드가 아직 뒤따르지 않은 줄에는 '(v0.7 — …)'·'(v0.8 — 단계 N부터)' 표지가 붙어 있다. §16 미결은 #1~#18, #24·#26, #28~#31, #33~#36, #40~#51, #55, #59, #60, #64, #67~#73이다(#61(소모 노브)과 #31 ③은 Phase 2 종료 리뷰에서 닫혔고 #71~#73은 그 리뷰가 기본값과 함께 열었다 — 2026-10-17. #23의 정착 쪽·#61 ④·#63·#65·#66은 정착과 문명 제거 작업의 헤더에서 닫혔다 — 2026-10-16. #52·#53·#56과 #29 ⑤·#31 ①은 위기 처리 작업의 헤더에서, #25·#54·#62와 #29 ①~④·⑥·⑦은 그 전에 닫혔다 — 모두 2026-10-15. #27·#37·#38·#39와 #55 ③, 옛 #57·#58은 2026-10-13에 닫혔다. #36 ③⑤⑥과 #40 ②는 닫혔고 #3은 다시 썼다. #41~#51·#55·#59·#60·#64·#67~#73 가운데 개발자에게 묻는 물음은 기본값이 본문에 적용되어 있다 — #68의 바뀐 기본값과 #70~#73은 적힌 작업의 헤더에서 들어간다). v0.8(2026-10-18)은 #74~#95를 기본값과 함께 더했다(그 뒤 Phase 3 7번 작업이 #96~#98을, 그 리뷰가 #99를, 12번 작업이 #100~#104를, 8번 작업이 #105~#109를 기본값과 함께 더했다) — 기본값은 본문에 적용되어 있어 작업을 막지 않고, 행마다 적힌 '때'에 정한다. v0.8의 결정을 코드·데이터로 옮기는 단계 0~8은 아래 표다(번호는 이 파일 표의 작업 번호와 다르고, CLAUDE.md와 guideline의 '(v0.8 — 단계 N부터)' 표지가 이 번호다).

   | 단계 | 하는 일 | 언제 | effort | ultracode | 일정 |
   |---|---|---|---|---|---|
   | 0 | 정착 사유와 패널 단추의 클릭 버그(Phase 3 표 6번) | 끝(2026-10-17, `efe5c37`) | — | — | — |
   | 손 확인 | 마우스만의 플레이, 창에서 60fps(Phase 3 표 6번의 사람 몫) | 지금 — 개발자 | — | — | — |
   | 1 | v0.8 결정의 문서 반영(guideline·CLAUDE.md·이 파일)과 커밋 | 끝(2026-10-18, `9aba0e9`) | xhigh | 켬(리뷰 → 적용 → 변경 범위 재감사) | 1.5~2.5일 |
   | 2 | 시간 표시 토글(Phase 3 표 11번) | 끝(2026-10-18, `2f61b62`) | medium | 끔(단독) | 0.5~1일 |
   | 3 | Phase 3 꼬리 — 7번 영적 시야, 8번 앎의 표시(나선팔·'여기' 표지는 단계 5로, 은하 패널의 한 줄), 9번 우주 화면(표시 전용 은하 행 — 결정 11), 12번 셋째 칸의 ×25·명령 단추의 사유·'다음 칸까지', 10번 종료 리뷰 | 7·12·8·9번은 손 확인을 기다리지 않는다(2026-10-18 사용자의 '이어서 진행해주고' — 7·12·8·9번은 끝), 10번은 손 확인 뒤 | medium | 끔 — 10번 종료 리뷰만 켬 | v0.8 몫 1~2일(꼬리 본래 몫은 이미 일정에 있다) |
   | 4 | 'v0.8 예약' ① 성역과 첫 생애 행(4a·4b — 아래 'v0.8 예약' 항목) | Phase 3 종료 뒤, Phase 4 첫 헤더 전 | xhigh, 헤더 승인 | 리뷰 1회(저장·결정론) | 2.5~3.5일 + 리뷰 0.5~1일 |
   | 5 | 'v0.8 예약' ② 실제 하늘 | 단계 4 다음, Phase 4 첫 헤더 전(늦어도 Phase 5 전) | xhigh(지도 표현은 medium) | 리뷰(생성·결정론, 지도 표현은 끔) | 8~11일 |
   | 저장 작업 | §16 #33 ①②③·#70·#40 ⑦(§2의 행) | 단계 6 앞 | xhigh, 헤더 승인 | 리뷰 | 따로(기존 계획) |
   | 6 | Phase 4 첫 블록 '서막(부르심)'(아래 'Phase 4의 v0.8 몫' 항목) | Phase 4 | high | 텍스트 묶음마다 감사, 코드는 단독 + 리뷰 1회 | 1.5~2주 + 콘텐츠 |
   | 7 | Phase 4 외계 백성 VS 5행 + 첫 생애 행의 본문 + 고넬료 사슬·'옛 땅의 기억' + 헤롯 패턴 청원 + 대도감의 문장(§16 #64와 함께) | Phase 4 | high | 묶음마다 감사 | 2~3.5일 + 재측정 0.5일 |
   | 8 | Phase 5 '세계관과 출처' 최소형 | Phase 5 | medium | 텍스트 감사만 켬 | 1일 |

   - 합계 약 25~36 작업일(약 5~7주) — 단계 1~8의 수치를 더한 값이다(단계 0 제외, 저장 작업·서막 콘텐츠·Phase 3 꼬리 본래 몫은 따로). 자리: 단계 0~3은 Phase 3 안, 단계 4·5는 Phase 3과 Phase 4 사이의 새 블록 'v0.8 예약', 단계 6·7은 Phase 4, 단계 8은 Phase 5. 저장 작업은 단계 6 앞에 따로 선다. Phase 3의 종료는 v0.8의 새 방향을 기다리지 않는다.

5. **Phase 3 진행 중**(2026-10-17 — 4주 + 꼬리 5일 + v0.7 꼬리 5일 + v0.8 꼬리 1.5~3일): effort medium, ultracode 끔, 구현은 단독(§2의 Phase 3 행 — 시각 작업은 사용자 피드백 루프가 더 중요하다: 그림이 바뀌는 작업은 창 없이 찍은 화면 한 장을 보이고 의견을 받는다). 헤더가 새로 생기거나 바뀌는 작업은 헤더를 먼저 내고 승인을 받은 뒤 소스를 쓴다. 표현 계층(`World/`·`UI/`)만 새로 쓰고 Core Logic은 읽기만 한다(§6.1 — 명령은 Subsystem 함수 호출). 종료 조건(guideline §11 Phase 3): 마우스만으로 Phase 2의 플레이가 가능하고 성계 60에서 60fps를 지킨다. 아래 표의 작업으로 나눠 위에서 아래로 한다(11·12번은 v0.8로 더했고 할 자리는 표 아래의 '순서').

   | # | 작업 | 하는 일 | 먼저 정할 것 | 상태 |
   |---|---|---|---|---|
   | 1 | 성계와 레인 그리기 | 상태에서 지도를 만든다: 성계마다 액터 하나(엔진 기본 구 + `M_Star` — 항성 등급의 색·크기, 주인의 색, Hazard 경고), 성계마다 작은 이름표(이름과 건설·대응 칸의 진척), 레인(`ULineBatchComponent`), 성운 배경(`M_Nebula`), 고정 개관 카메라. 그릴 값은 상태만 읽는 순수 함수로 계산하고 새로 그리기는 프레임마다 많아야 한 번이다. 머티리얼은 스크립트가 만든다(`Tools/make_materials.py`). 테스트 `UI.GalaxyMap` | 헤더 초안의 결정 A~J | 끝(2026-10-17, e1776e3) |
   | 2 | 카메라와 선택 | 카메라 폰(스프링 암 — 궤도 회전·줌·더블 클릭 포커스), 클릭으로 성계 선택(라인 트레이스)과 선택 표시 | — | 끝(2026-10-17, fc46a21) |
   | 3 | 상단 바와 성계 정보 패널 | 상단 바(자원 다섯·틱·배속 단추), 성계 정보 패널(이름·항성 등급·주인·Hazard와 남은 틱·건설·대응 칸의 진척, 거대 프로젝트 시작 단추와 정착 단추 — 프로토타입 키 P를 대신한다, 상한에 닿은 통치자의 안내 한 줄 — §5.7). 텍스트 패널은 여기서 내린다 | §16 #40 ④(아키타입 이름을 보이면) | 끝(2026-10-17, 5f25d6c) |
   | 4 | 로그 피드 | `UListView` 피드(가상화, 표시 상한 500 — §9.4), 항목을 누르면 카메라가 그 줄의 성계로 간다, `bUntold` 항목은 건너뛴다 | — | 끝(2026-10-17, 5aa1a5d) |
   | 5 | 승천 연출 | `UAscensionSubsystem::OnTierAscended`를 듣는 연출: `NS_Ascension`(Niagara 3종 예산의 하나 — §6.5), 소리, `MythicNarrative` 오버레이(`WBP_AscensionOverlay` 몫의 C++ 위젯). 연출 동안 시뮬을 멈추는지 | §16 #40 ③(틱 안에서 거는 일시정지 — 이월분을 버리는지) | 끝(2026-10-17, ccfb2fc) |
   | 6 | 종료 조건 확인 | 마우스만으로 첫 생애를 둘째·셋째 칸까지 해 보는 확인(사용자에게 화면을 보인다), 성계 60에서 프레임 시간을 잰다(창 없이 찍는 실행에서 `stat unit`) | — | 기계 몫 끝(2026-10-17, 05512a4), 정착 사유와 단추 유지 끝(2026-10-17, efe5c37), 사람 몫 대기 |
   | 7 | 영적 시야 꼬리 | `M_Star`의 `ViewMode`·`Pressure`·`DominionColor`·`ClaimState`·`WarningColor`, 봉인 고리(선), 압력에 회색이 되는 반쪽 레인(`M_Lane`은 없다), `M_Nebula`의 `SpiritualIntensity`, 보는 쪽의 셋째 칸에서 열리는 보기 단추(§6.4), 성계 패널의 표식 말(툴팁과 한 줄 — 칭호 `DT_Transcendents.Title`)과 '{PopBand} 백성' 한 줄·규모 등급 라벨 ST_CA 6, 셋째 칸 연출 뒤의 "이제 보인다.". 리뷰로 고친 것(2026-10-18): 영적 시야의 주인 고리는 타락의 색상 띠를 피한다(그 띠의 고리는 제 회색), 표식은 빛을 더하지 않고 별을 짙은 보라로 물들인다, 패널의 표식 말은 상세 줄과 따로인 제 줄과 툴팁이고 주장의 말은 "…주장하는 곳", 칸의 연출은 `CosmicPreviewRung` | §16 #99(침묵의 호칭 — 손 확인 때 개발자에게) | 끝(2026-10-18, `94e8706`) |
   | 8 | 앎의 표시 꼬리 | (v0.7 꼬리) 모르는 성계는 배경 별의 제 자리 점이다(별을 숨기고 배경 별 인스턴스의 슬롯이 그 성계 — `M_Star`의 `Known` 대신, guideline §16 #105), 보이지 않는 주인의 색·이름·일을 가린다(패널은 '주인  다른 백성' — 12번 작업이 넘긴 정착 단추의 사유도 그대로 '이미 다른 백성이 산다', §16 #108), 레인은 두 끝이 아는 성계일 때만, 만난 순서의 12색(OKLab으로 잰 값), 배경 별 인스턴스(`M_Backdrop`, `DT_Rules.BackgroundStarCount` 20,000 — 나선팔·'여기' 표지는 'v0.8 예약' ②), 은하 패널(종류·별의 수·거리 FText·모든 은하의 한 줄·첫 선택까지의 안내 줄 — §16 #106), 생명 낱말, 선택·카메라·클릭은 아는 성계만, 개발용 `CosmicSeeAll`, 더블 클릭을 끝내는 뗌은 클릭이 아니다(9번 작업의 비평이 찾은 결함 — 수정 14), ST_CA 라벨 20, 테스트 `UI.KnownFilter`·`UI.Backdrop`·`UI.GalaxyPanel`, 배경 별을 넣은 뒤 프레임 시간을 다시 쟀다(GPU는 손 확인 때 창에서). 이미 끝나 있던 것: 죽은 별(1번 작업), 상한 안내 한 줄(정착 사유와 단추 유지, `efe5c37`), 피드의 `bUntold` 건너뛰기(4번 작업), '{PopBand} 백성'(7번 작업) | §16 #105~#109(손 확인 때 개발자에게 — #107은 Phase 4의 일) | 끝(2026-10-18, `25104b2`) |
   | 9 | 우주 화면 | (맨 끝, 종료 조건과 무관 — 밀리면 Phase 5) 같은 지도의 줌 단계: 풍경 은하 10,000 + 이름 있는 은하 20 — v0.7의 6과 새 표시 전용 14 — M87과 함께 표시 전용 15(§5.11, v0.8 결정 11) | §16 #110~#116 | 끝(2026-10-18, `25ab43b`) |
   | 10 | 종료 리뷰 | Phase 3 전체의 ultracode 리뷰 1회(§2 — Core Logic이 UI/Actor를 include하지 않는지, 표현 계층이 파생만 하는지, 앎의 표시, 프레임 예산)와 그 수정분의 재감사 | — | |
   | 11 | 시간 표시 토글 | 상단 바 시계를 눌러 '주기'↔'년'(보는 사람의 설정, 기본 '주기', 상태·해시 밖), 마우스를 올리면 두 단위, 피드 머리·패널 카운트다운·배속 단추 라벨이 따른다. 테스트 `UI.TimeUnitToggle` — v0.8 단계 2 | — | 끝(2026-10-18, `2f61b62`) |
   | 12 | 셋째 칸의 배속과 명령의 사유 | 승천 연출 뒤 ×25 단추, 거대 프로젝트 단추의 사유(칸(Tier)·비용·건설 칸이나 대응 칸 사용 중 — `CanStart` 곁의 사유 열거형, 헤더 초안 직접 승인. 비용은 툴팁의 값 줄로 읽었다 — guideline §16 #101), 값을 못 낸 칸의 '멈춤 — 자원 부족', 상단 바 툴팁 '다음 칸까지', 커맨드렛이 AI가 넓히지 못한 사유를 100틱마다 센다 — v0.8 단계 3의 몫. 손 확인에서 볼 것: 셋째 칸의 연출이 닫힌 뒤의 ×25(년 단위면 '25년/초'), 회색 프로젝트 단추의 짧은 사유와 툴팁(사유와 값의 줄), 회색 단추와 지금 배속 단추의 새 모양(거의 검은 면 위의 흰 글), 엔진 기본 툴팁의 글자 크기, 상단 바의 칸·연구 덩이에 마우스를 올린 '다음 칸까지', 셋째 칸의 년 단위에서 시계에 마우스를 올린 상단 바(연구 덩이가 '…'로 줄어든다), 값을 못 낸 칸의 '멈춤 — 자원 부족'(그 칸 줄이 두 줄로 접힌다 — §16 #103) | §16 #100~#104(손 확인 때 개발자에게) | 끝(2026-10-18, `b8803ef`) |

   - 순서의 까닭: 1번이 그리는 액터와 그릴 값을 2~4번이 쓴다(선택·패널·피드의 포커스). 꼬리는 원래 종료 조건 뒤였는데(guideline §11), 2026-10-18 사용자가 '이어서 진행해주고'라고 해서 셋째 칸의 꼬리(7번 영적 시야, 12번 배속과 명령의 사유)는 손 확인 전에 온다 — 손 확인이 셋째 칸까지 오르므로 그 꼬리가 든 가장 새 빌드로 보는 것이 낫다. 8·9번(앎의 표시, 우주 화면)도 손 확인을 기다리지 않는다(아래 '순서' — 같은 날 사용자). 8번이 든 빌드는 앎으로 거른 지도로 연다 — 첫 접촉(330틱 무렵, 1배속 5:30)까지 배경 별 사이에 모성계 하나(손 확인의 목록, 아래 '다음'). 손 확인은 언제 해도 되고 가장 새 빌드로, 10번 종료 리뷰 전에 한다. 종료 리뷰는 꼬리까지 끝난 뒤 한 번 돌려 꼬리도 함께 본다.
   - 순서: 11 → 7 → 12 → 8 → 9 → 6(사람 몫 — 손 확인, 가장 새 빌드로. 언제 해도 되고 10번 전에) → 10. 11·7·12·8은 끝났다. 7·12·8·9는 손 확인을 기다리지 않는다(2026-10-18 사용자의 '이어서 진행해주고').
   - 1번의 헤더 초안과 결정 A~J는 저장소 밖 `Saved/Design/phase3_map/header_draft.md`에 있다(앞 Phase의 설계 기록과 같은 자리).

6. **'v0.8 예약'**(Phase 3 종료 뒤, Phase 4 첫 헤더 전 — 11~15.5일): effort xhigh(② 지도 표현은 medium), 헤더 승인, ultracode 리뷰 — 단계 4·5. ①은 헤더를 한 번 승인받고 두 커밋(4a·4b)으로 내며 ultracode 리뷰 1회(저장·결정론), 2.5~3.5일 + 리뷰 0.5~1일이다. ②는 ultracode 리뷰(생성·결정론 — 지도 표현은 끔), 8~11일이다. 'v0.7 예약'처럼 두 Phase 사이의 블록이고(guideline §11 'v0.8 예약'), 저장 ID·생성 추첨·세계 기준값이 여기서 바뀐다. 종료 조건: ① 두 커밋의 증명과 `Sim.SanctuaryKept`가 연속 두 프로세스에서 통과하고, 첫 생애가 `sys_0`의 찾는 백성으로 시작한다. ② 관측 목록 60계 + 성역으로 판이 생성되고(시드마다 같은 지도, 백성·사건은 시드가 바꾼다), 새 기준값이 guideline §9.3과 CLAUDE.md에 적힌다.

   | # | 작업 | 하는 일 | 먼저 정할 것 | 상태 |
   |---|---|---|---|---|
   | ①-4a | 성역 | 다시 쓸 수 없는 다섯째 법(가칭 `Law.Hedge`)·성계 술어(가칭 `CosmicState::IsSanctuary`)·막는 자리(정착·넘김·주장·교단·초기 코어·모성계·출생·계승·즉위·파괴·영향권, 압력 0), 생성의 잎 노드와 ID(`sys_0` = 첫 생애의 모성계, `sol` = 성역 — guideline §7.4 11항), 정착 사유 '성역'(맨 앞 — 그 정착 단추에는 값 줄이 없고, `PeopleSettleBlockOf`에서는 영향권 아래·주인 있음 곁: 12번 작업의 `static_assert` 둘이 알린다)·성역의 패널·영향권 사유의 그림, 성역의 그림과 선택(8번 작업이 둔 자리: `FCosmicKnownView`의 둘째 비트(가칭 `ShownByName`)를 `ShownLookOf`·`LightSlots`·`PickCandidatesOf`·`SelectSystem`·성계 패널의 앎 문(`Refresh`)이 함께 읽는다, 성역에 닿는 레인을 정한다, `OwnerLineOf`의 첫 줄, `CosmicUI::NoLifeLineSystemID`를 성역의 술어로 — 그 뒤 `UI.Backdrop` §3·`UI.KnownFilter`의 '처음 아는 성계는 하나' 전제를 고친다), 영적 시야의 성역 경우(`CosmicMapLook::SpiritOf`의 첫 경우 — 색 없음·압력 0, `SpiritTipOf`의 갈래와 ST_CA `Sight.Sanctuary` "이곳에는 압력이 닿지 않는다." — 패널의 한 줄과 그 툴팁, guideline §6.4 '색'), 로드 때 주인 있는 성역 거절, 개발 세이브 지우기, `arch_terran.HomeSystemNames` 비우기(성역과 플레이어 모성계가 둘 다 '태양계'가 되는 중간 상태를 막는다). 증명: 커맨드렛 요약의 결정적인 칸이 같고 무주 성계 수만 +1, `Sim.SanctuaryKept` | §16 #80의 일부·#94 | 끝 2026-10-19 |
   | ①-4b | 첫 생애 행 | 가칭 `arch_seeker`의 뼈대(행 이름·태그·`Weight` 0·특성·제 해 이름·첫 줄), `DT_Rules.PlayerArchetypeRow`와 C++ 기본값, `arch_terran` 은퇴, 낱말 경계 금지어 검사(스키마의 낱말 목록 가칭 `other_works`·`holy_and_myth` — 이름 3단계 검사는 `DT_Transcendents`의 `Title`·`Lure`도 덮는다: guideline §4.2 '하나님의 성품 칭호를 그들에게 역으로 붙이지 않는다', 7번 작업의 수정 8. 침묵의 호칭은 §16 #99), 고칠 테스트(guideline §9.3). 증명: 새 기준값(guideline §9.3, CLAUDE.md, `Sim.PhaseTwoExit`의 증거 시드를 다시 고른다) | §16 #74 | 끝 2026-10-19(낱말 경계 금지어 검사는 빈 목록만 — 채우기는 단계 7. `PhaseTwoExit`는 그대로 통과해 시드를 고르지 않았다) |
   | ② | 실제 하늘 | 관측 목록 도구와 스냅숏(`Tools/data/sources/` README), 관측 목록 행 구조체와 `DT_Planets`, 투영과 레인(오프라인 평면 그래프, 손 고침 파일), 별 이름과 거름, `FHazardDef`의 등급별 표시 이름과 '항성 안정화'(상단 바 '다음 칸까지'의 '{위기}에 답할 때'도 `FCosmicNextRung::SystemIndex` 성계의 등급 이름을 읽는다 — 그 위기가 이미 서 있으면 '(시작 전)'으로 읽을지도 그때 본다, 12번 작업의 리뷰), 에너지 상수와 정규화, `LivingWorldStarShares` 은퇴, 첫 생애의 별 고르기(§16 #92), 성역의 실제 자리, 나선팔·'여기' 표지(Phase 3 표 8번에서 옮겨 온다), 배경 별과 광년 금(표현 계층 — `CosmicMapLook::BackdropPointsOf`를 갈아 끼운다(guideline §3.1 '배경 별의 그림'): 점의 모습 법칙이 바뀌면 모르는 성계의 슬롯(`BackdropSlotOf`·`BackdropLookOf`)도 같은 법칙), 환경 특성 `Trait.World.*`(계 행 — 문장은 Phase 4), 먼 표지(표시 전용 행), 스키마 규칙과 `Data.SourcesInSync` 도장 확장, 성역을 지나는 레인 셈(§16 #80). Phase 5 전에 모든 시드의 기준값이 한 번 바뀐다 | §16 #79·#80·#82·#92 | |

7. **Phase 4의 v0.8 몫**: Phase 4 첫 블록 '서막(부르심)'(단계 6) — 조우 카드 화면을 먼저, `DT_Encounters` 앞에 `make_data_tables` 검사를 중첩 구조체까지 넓힌다, 서막 카드, 부르심의 Core 함수(가칭 `ReceiveCalling`), `UI.PrologueGate`. 외계 백성(단계 7) — VS 5행 + 첫 생애 행의 본문 + 고넬료 사슬·'옛 땅의 기억' + 헤롯 패턴 청원 + 대도감의 문장(§16 #64와 함께). effort high, 텍스트 묶음마다 감사(ultracode), 코드는 단독 + 리뷰 1회. 저장 작업은 단계 6 앞에 따로 선다. 방랑자의 성역 경계 카드 '지경'(가칭 `enc_sanctuary_bound`)과 레인 이동 시간은 Phase 4의 `TravelTo` 헤더에서 정한다(§16 #81·#83).
8. Phase 1에서 미룬 확인: 게임 바이너리(쿠킹된 빌드)의 해시 일치는 첫 패키징 때 세계 해시로 비교한다(테스터와 시드를 나누기 전). 화면 확인은 CLAUDE.md의 오프스크린 한 장 찍는 법을 쓴다(패널을 고친 뒤에는 한 번 찍어 본다 — `-nullrhi` 테스트는 화면을 보지 못한다).
9. 개발자에게 묻는 물음 #41~#51은 기본값이 본문에 적용되어 있어 작업을 막지 않는다. 시점이 정해진 것은 #43 하나다(남은 자 법의 범위 — `Hazard.Cult` 만료 효과를 쓰는 Phase 4 전). #3의 "60인가 120인가"는 `DT_Galaxies` 고향 행의 값이라 답이 오면 행만 고친다(v0.8 — 단계 5부터는 60·300이 관측 목록의 반지름이라 120은 반지름을 넓혀야 한다: 단계 5 헤더에서 다시 본다, §16 #3).
10. **다음**: ① 9번 '우주 화면'의 리뷰와 커밋(메인 세션 — 구현은 2026-10-18 끝, 헤더 초안의 'As built'). ② 개발자의 Phase 3 손 확인 — 언제 해도 되고 10번 종료 리뷰 전에 한다. 가장 새 빌드(지금은 9번까지 든 빌드)로 창에서 마우스만으로 첫 생애를 둘째·셋째 칸까지, `stat fps`·`stat unit`으로 60fps(태양계에서 시작하는 임시 개발 빌드로 보고 마친다 — 배경 별의 GPU 몫은 그때 창에서 `show InstancedStaticMeshes`로 끄고 켜서 본다). 새 판은 앎으로 거른 지도로 연다(8번): 첫 접촉(330틱 무렵 — 1배속 5:30)까지 희미한 배경 별 사이에 모성계 하나이고 레인이 없다. 아무것도 고르지 않으면 오른쪽에 은하 패널과 그 아래 회색 안내 "별을 누르면 그 성계를 본다."(세션의 첫 선택 뒤 사라진다). 화면 중심에서 약 200픽셀 떨어진 별을 버튼을 약 150 ms 누른 채 더블 클릭해도 선택이 그대로다(수정 14). 시드 42의 첫 접촉에서 개관의 '태양계' 이름표가 바로 위 민타카의 고리에 얹혀 이웃의 이름처럼 읽히지 않는지(읽히면 뒤의 이름표 작업 — 붐비는 이름표 비켜 놓기나 별 옆 이름표). 상단 바의 시계를 누르면 '주기'와 '년'이 바뀐다(마우스를 올리면 둘 다). 셋째 칸에서 볼 것: 시계와 배속 사이에 나타나는 보기 단추, 오르는 틱에 영적으로 바뀌는 지도, 연출이 닫힌 뒤의 "이제 보인다.", 패널의 표식 줄, 연출이 닫힌 뒤 나타나는 ×25(년 단위면 '25년/초' — 그때 시계에 마우스를 올리면 상단 바의 연구 덩이가 '…'로 줄어든다). 언제든 볼 것(12번): 회색 프로젝트 단추의 짧은 사유와 툴팁(사유와 값의 줄), 회색 단추와 지금 배속 단추의 새 모양(거의 검은 면 위의 흰 글 — 받아들일지), 엔진 기본 툴팁의 글자 크기(작으면 패널 글꼴의 툴팁으로), 상단 바의 칸·연구 덩이에 마우스를 올린 '다음 칸까지', 값을 못 낸 칸의 '멈춤 — 자원 부족'(그 칸 줄이 두 줄로 접혀 단추를 한 줄 내린다 — §16 #103) — 개발자가 정할 것은 guideline §16 #96~#99(기본 보기, 보라의 세기와 봉인 고리, 침묵의 호칭)·#100~#104(×25의 문과 때, 비용, 같은 행만 '사용 중', '멈춤'의 문턱, 툴팁의 자리)·#105·#106·#108·#109(모르는 성계의 그림, 은하 패널의 자리와 안내·첫 5:30, 보이지 않는 주인의 말, 먼지와 팔레트 — 격차 규칙의 세 사본 #107은 Phase 4의 일)와 태양계가 단계 4까지 다른 성계처럼 그려진다는 것. 우주 화면(9번): 구역의 먼 끝을 지나 휠을 더 물러나 우주로 갔다가 돌아오기, 은하 하나를 누르기와 더블 클릭, 먼 왜소은하와 고향을 버튼을 약 150 ms 누른 채 더블 클릭하기, 은하의 초상에서 휠을 다가가면 머물고 고향 위에서는 들어가는지, 우주를 시간이 흐르는 동안 머물 곳으로 느끼는지, 고향이 우주의 가운데로 읽히는지(먼 끝은 고향을 가운데 두고, 고향만 둥글게 위에서 보인다 — 법칙으로는 M31·M101이 더 크지만 화면의 밝은 넓이는 고향이 크다: 밝기 0.42 위의 표본 519 대 370·296, 9번 작업의 리뷰 — §16 #111·#112), 피드와 은하 패널 밑에 비치는 은하 이름표(안드로메다 초상의 NGC 253, 가까운 끝의 화로자리)와 M87의 빛무리 안의 M104, 회색 안내 넷, 보는 방향의 새 성운, 우주의 GPU 몫(창에서) — §16 #110~#116(Q1~Q9). ③ 10번 종료 리뷰 — effort medium 세션에서 ultracode 켬. ④ 'v0.8 예약' ① 성역과 첫 생애 행 — effort xhigh, 헤더 승인, ultracode 리뷰 1회. ⑤ ② 실제 하늘 — effort xhigh(지도 표현은 medium), ultracode 리뷰. 사용자가 2026-10-17에 Phase 3을 알아서 진행하라고 한 것은 그대로다(헤더 초안은 직접 승인, ultracode가 필요한 곳이나 사용자의 손·결정이 필요한 곳에서 멈춘다).


---

# 옛 guideline §11 로드맵 · §12 범위 관리 · §13 리스크 (2026-10-19 이관 — §11의 기간 열과 상태 줄은 역사 기록이며 상태 줄은 docs/CHANGELOG.md로 옮겼다)

> 원문: guideline.md v0.8 §11~§13 (2026-10-19 docs/ 재편으로 이관, 내용 무변경 — 원격의 Phase 3 꼬리 작업 반영분 포함). 표지 `(v0.8 — 단계 N부터)`·`(v0.7 — …)`·`Phase N`이 붙은 문장은 **예정**(아직 코드에 없음), 표지 없는 문장은 **현행 규칙**이다. 열린 물음은 `docs/60_DECISIONS/OPEN_QUESTIONS.md`, 확정 기록은 `docs/60_DECISIONS/DECISIONS_LOG.md`. 절 번호(§N)는 원문 그대로 두었다 — 위치는 `docs/SECTION_MAP.md`.

## 11. 로드맵 (현실화)

초안의 "총 10주"는 프로토타입 기준으로는 맞지만 UE5 1인 개발의 실제 달력으로는 비현실적이다. 아래는 각 Phase에 **종료 조건(Exit Criteria)**을 두고, 셋업과 폴리시 단계를 추가한 것이다. 기간은 주당 25~30시간 투입 기준이다.

```

[P0 셋업] ─▶ [P1 데이터 엔진] ─▶ [v0.7 예약] ─▶ [P2 이벤트·승천] ─▶ [P3 은하 뷰포트] ─▶ [v0.8 예약] ─▶ [P4 이원 루프] ─▶ [P4b 부름 루프] ─▶ [P5 수직 슬라이스 폴리시] ─▶ [P6+ 콘텐츠 → EA]

1주 2주 1.5~2일 3주+8~9일 4주+5일(+v0.8 꼬리 1.5~3일) 11~15.5일 6주+1~1.5일 2주+0.5일 4주+1일 6~12개월

```

### Phase 0: 프로젝트 셋업 (1주)

UE 버전 고정, C++ 프로젝트 생성, 모듈 의존성(§6.6), 폴더 구조(§10.1), Git LFS, CLAUDE.md.
GameplayTags ini에 초기 태그 트리. 빈 Subsystem 6개가 컴파일되고 에디터가 뜨는 상태.
**종료 조건**: 커맨드라인 빌드 성공, 빈 L_Galaxy 맵 실행, 로그 1줄 출력.

### Phase 1: 데이터 엔진 프로토타입 (2주)

그래픽 없이 UMG 텍스트 패널 1개. 성계 5개를 코드로 생성. 1초마다 자원·연구가 쌓이고 로그가 흐른다.
포함: 고정 스텝 틱, 시간 배속, `FRandomStream`, 로그 템플릿 치환, **저장/로드 왕복**, 결정론 테스트, 밸런스 커맨드렛 골격.
헤더 예약(지금은 비용이 거의 0, 나중에 넣으면 SaveVersion 마이그레이션): `FCallingState/FLifeRecord/ELifeState`, `GalaxyID`, 방송 게이트, 판당 슬롯 1개 저장 모델, 해시에 `Calling` 포함(약 3시간, §4.4·§9.1). 필드 `HomeSystemID`·`ThroneLockedUntilTick`·`StatusTags`·`Exaltation`·`MegaprojectCompletedTick`·`SealedTick`·`FCivilizationState.ArchetypeRow`·`FLifeRecord.SuccessionKind`/`bReturnOffered`·`FGalaxyState.Flags/Chronicle`·`FCallingState.PendingRoleEffect`(PendingEffect/PendingCause/PendingSourceRow)·`FCivilizationState.LowStabilityTicks`·`FLifeRecord` 비문 필드(v0.4 감사분, §7.2), `Relations` 주석, `DT_Civs`·`FCivArchetypeDef`(계시 양식 4컬럼 + DedicationBanks 포함) 헤더, `FTranscendentDef`(`Allegiance`·`CounterfeitOf`·`Lure`·`Title`·`SpawnWeight`·`MaxInfluencedSystems`·`ClaimTemplates` — 상태에 `Realm` 없음), `FLogTemplateDef`, `FHazardDef.SourceDoctrine`, 열거형 끝에 `EEffectType::ChangePopulation`·`EConditionType::TranscendentDoctrinePresent`, `DT_Rules` 노브 9개(`FallenTranscendentCount`·`PlayerArchetypeRow`·`ClaimCountdown`·`ClaimAdjacencyLanes`·`VigilClaimExtension`·`SphereEventWeightStep`·`ReturnLureMaxLanes`·`ReturnedLivesToSeize`·`LureAcceptByPolicy`), `NewGalaxy`가 `PlayerArchetypeRow`(`arch_terran`)를 읽고 `Install(…, Ruler)`로 첫 생애 시작(로스터 추첨 코드는 Phase 2, §12.3), 해시에서 FText 제외, `Role` 기본값 Ruler(약 1일, §5.10·§7.2·§7.3·§9.1). v0.6 요약 단위 예약(약 2시간, §1.3·§9.1): `FNotableRecord`·`FCivilizationState.Notables`, `FLifeRecord.NameIndex`, `FCivArchetypeDef.PersonNameBank`, 열거형 끝에 `EEffectType::NameNotable`·`EConditionType::CivHasNotable`, `DT_Rules.MaxNotablesPerCiv`·`PopulationBandThresholds`, `FHazardDef.OnResolveEffects`(기본 빈 배열), `EPopulationBand`와 `GetPopulationBand` 선언, 해시에 `Notables`·`NameIndex` 포함. 헤더 리뷰 확정분(2026-10-12, §9.1·§16): `FLifeRecord.BirthArchetypeRow`(FName 1, 해시 포함). 종료 조건 변경 없음.
**종료 조건**: 1,000틱 실행 후 저장 → 재시작 → 로드 → 해시 동일. 로그에 같은 문장이 연속 2회 뜨지 않음. 새 은하에서 플레이어 문명이 `arch_terran`으로 생성되어 1개 성계(`sol`)의 통치자로 시작하고 로그 첫 줄이 태양계를 부른다("태양계. 당신은 이 행성의 통치자다.").

### v0.7 예약: Phase 2 첫 헤더 전 (1.5~2일)

guideline v0.7과 CLAUDE.md 수정이 먼저다. 그다음 한 커밋(컴파일되는 상태)으로 아래를 넣는다. 새 Phase가 아니라 Phase 번호를 받지 않는 한 블록이다.
헤더(저장되는 필드, §7.2·§9.1): `FKnownMark`, `FKnowingState`(멤버 6 — 첫 접촉의 기록 `FirstContactCivID` 포함)와 `FGalaxyState.Knowing`, `FTransitRecord`와 `FGalaxyState.Transits`, `NextCivSerial`, `TellRngState`, `FStarSystemState.GalaxyRow`, `FCivilizationState.GalaxyRow`·`OriginGalaxyRow`, `FCosmicLogEntry.FocusCivID`·`bUntold`.
코드: `Core/CosmicSeed.h`(`CosmicSeed::Derive`와 용도 상수 8), `DrawTell`과 `PushRow`의 말하기 스트림 전환, `ComputeWorldHash()`, 빈 `TickCrossings`·`TickSight`와 열 단계 주석(`bInTickStages`는 1~9단계), `Push`·`PushRow`의 `FocusCiv` 기본 인자(기존 호출부 무변경), 밸런스 커맨드렛의 로그 줄에 세계 해시 추가(§7.4, §7.5, §9.2, §9.3).
테스트: 새로 `Sim.SeedDerive`·`Sim.TellStreamSeparate`. 보강 `Log.NoConsecutiveRepeat`(탐침 3곳을 말하기 스트림으로, 주제 문명의 저장), `Log.CapAndChronicle`(`Push`가 주제 성계·주제 문명·행을 항목에 적는다), `Sim.SaveRoundTrip`(`TellRngState` 이어짐, 새 멤버 25개를 초기값 밖에 둔 채 저장하고 로드 뒤 필드별 비교), `Sim.SaveRestart`(기록에 Derive 값 하나)(§9.3).
행 이름·태그 이름만 지금 확정한다(정의 데이터 자체는 처음 읽는 Phase에 넣는다): `gal_milkyway`·`gal_andromeda`·`gal_triangulum`·`gal_lmc`·`gal_smc`·`gal_m87`, `project_settle`, `project_crossing`, `petition_first_contact`, `log_first_contact`·`log_contact`·`log_far_ascent`·`log_far_event`·`log_scan_wild`·`log_crossing_depart`·`log_crossing_arrive`·`log_crossing_turned`·`log_crossing_word`, `Galaxy.Kind.*` 4, `Transit.People`, ST_CA `Digest.Elsewhere`.
절차: 헤더 먼저 제시 → 승인 → 소스. 에디터를 닫고 두 타깃 빌드, 테스트 두 번, 새 테스트는 한 번 일부러 깨뜨린다. `SaveVersion`은 그대로다. 개발 세이브는 넘기지 않는다(§9.1).
**종료 조건**: 두 타깃 빌드 성공, 자동화 테스트 14종(기존 12 + 새 2)이 연속 두 프로세스에서 통과.

### Phase 2: 이벤트·위기·승천 (3주 + 5~6일 + 스키마 검사기 3일)

`UCosmicEventSubsystem` 조건/효과 평가기. Hazard 8종 중 5종(초신성, AI 각성, 역병, 반란, 주장 골격 `crisis_claim`). 업적 T1→T2, T2→T3 통치자 경로.
승천 연출(NS_Ascension + 사운드 + MythicNarrative 오버레이)은 첫 화면이 생기는 Phase 3로 옮겼다(2026-10-16, 거대 프로젝트와 승천 판정 작업의 헤더) — Phase 2에서는 승천 줄(`Log.Ascension`, Severity 2)과 패널의 Tier 한 줄이다.
추가: 효과 `KillPlayer`·`DeposeRuler`·`ChangePopulation`·`SetFlag`, 조건(`PlayerLifeIndexAtLeast`, `TranscendentAwareOfPlayer`, `SystemPressureAtLeast`, `CivRelationAtLeast`, `PlayerHasStatus`, `SystemInfluencedBy`, `TranscendentDoctrinePresent`, `FlagSet/FlagNotSet`, 지속 틱 조건), `ComputeSpiritualPressure`와 수정자 ①③④(§7.4), 초월자 행의 심도 컬럼 `Depth`와 압력 항의 `(1 + Depth)`(§3.2 심도 — 영적 압력 작업의 헤더에 함께 넣는다), `UCosmicLogSubsystem::PushVariants`, `FCosmicEventDef`의 `LogCategory`·`Trigger`·`Subject`·`SignRow`, `DT_Rules` 노브 `EventSlotTicks`·`EventChancePerSlot`, `CosmicSeed` 용도 `EventSlot`, 색인(`Sim/CosmicStateIndex.h` — 평가기보다 먼저), `MegaprojectCompletedTick`의 기록(`SealedTick`의 기록은 T4 봉인 업적 '결박'이 오는 Phase 6+와 함께 — 그 Phase의 목록에 적었다), `Death.*` 태그, `MinTierToReveal`, 은혜 이벤트 행(remnant는 이 Phase의 위기 처리 작업 — 마지막 성계의 문과 함께, rescue는 Phase 4b, jubilee는 Phase 4 — §4.5), 모티프 이벤트 행(요나는 이 Phase의 위기 처리 작업, 바벨·느부갓네살은 Phase 4 — 그 행이 기대는 장치 셋(탑 프로젝트, 단일 사상 칙령, 시한이 있는 강제 방랑과 `Trait.Legacy.Humbled`)은 VS 목록에 아직 없어 Phase 4 헤더에서 §12.3을 거쳐 넣거나, 있는 장치로 다시 쓰거나, Phase 6+로 미룬다, §8.6. 바벨 행은 새 문명을 만들지 않는다), 책 이벤트 3행("책이 닫혔다"/"책이 발견되었다"/낭독, §5.11 — 첫 이벤트 데이터 작업. 같은 작업이 모든 백성에게 도는 추첨 행 넷(광맥·에너지망·다툼·축제)도 쓴다), `DT_Transcendents` 최소 행 `trans_silence`(Doctrine·Allegiance·Tier·Depth·InitialInfluencedSystems·MaxInfluencedSystems·SpawnWeight·PressureEffectsPerTick만. Title·CounterfeitOf·Lure·VoiceTemplates·ClaimTemplates는 Phase 4) + `NewGalaxy` 로스터 추첨(침묵 고정, 약 20줄). 영혼 경제 골격(§4.2): `TickHazards` 만료 처리 끝의 `Hazard.Claim` 발생 규칙(약 80줄), 만료 = 영향권 편입, `Law.Remnant`, `MaxInfluencedSystems` 상한, 교리별 `PressureEffectsPerTick`과 영향권 가중 승수 `(1 + 0.1 × Num)` 상한 3, `FHazardDef.SourceDoctrine`, 카테고리 `Event.Crisis.Claim`, 로그 태그 `Log.Claim`·`Log.Record`. 테스트 `Sim.ClaimNeverTotal`·`Sim.SphereGrowthResponsive`·`Sim.FallenRosterPick`·`Sim.NoClaimsFromCelestial`(Celestial·Independent 행은 Phase 6+이므로 테스트 주입 행으로 검증)(§9.3). 약 2일 + 2시간 + 1일. 요약 단위(§1.3, 약 1일): `UCosmicEventSubsystem::Apply`의 `NameNotable` 가드·퇴출(약 30줄, §7.5), 코드 훅 3(`TickHazards` 반란 만료의 Rebel → Ruler·해결 시 닫기 — 위기 처리 작업 / `BecomeWanderer`의 Regent·`ReturnToFormerCiv`의 닫기 — 자진 이탈·복귀 명령과 함께 Phase 4로 옮겼다, 폐위에는 Regent가 없다 / `UAscensionSubsystem` AI T3의 Ascendant), `crisis_rebellion` 발생 전조 ② 행에 `NameNotable`(`Notable.Rebel`), `Notable.*` 태그 6은 ini에 선언 완료(v0.6) — RequestGameplayTag로 사용만, 테스트 `Sim.NotablesCapped`·`Sim.NotablesFromEventsOnly`, 그리고 `Sim.RealmPressureMonotonic`·`Sim.PressureDepthMonotonic`(영적 압력 작업과 함께)·`Sim.RevoltTelegraph`(위기 처리 작업과 함께). 시뮬 스트림의 추첨 지점은 테스터와 시드를 나누기 전(Phase 5 전)에만 더한다(§9.2): 생성, 위기의 발생과 만료 훅, 이벤트 슬롯, 인물 이름, AI 정착은 Phase 2에 한꺼번에 넣고, 분쟁 단계·조우의 생성과 판정·계승과 도약의 추첨은 그 본문이 오는 Phase 4·4b에 들어간다(그때 같은 시드의 역사가 한 번씩 더 바뀐다 — 골든 해시가 없어 깨지는 테스트는 없다). Phase 5 뒤의 예외는 6a의 착지 추첨 하나다(§5.7 건넘). 말하기 스트림은 여기 들지 않는다 — 로그 호출 지점이 뒤 Phase에서 늘어도 시뮬 난수열을 밀지 않는다. 인물 이름 뱅크(`PersonNameBank`)를 채우는 일도 시뮬 난수열을 밀지 않는다(§9.2 추첨 지점의 규칙. 문명 이름·모성계 이름의 뱅크도 은하 생성 규칙 변경부터 같다 — 그 헤더에서 생성의 다섯 추첨을 §9.2 ⓐ대로 무조건 뽑기로 정했다, §7.4. 그 커밋 전의 `NewGalaxy`는 뱅크에 값이 있을 때만 뽑는다).
위기 처리 작업에서 당긴 것(§16 #56, 2026-10-15): 폐위 파이프라인의 최소형(그 작업), 그리고 거대 프로젝트의 진척·완성(안정화 프로젝트 1행부터 — 위기 단계의 ⓐ 해결), 통치자 명령 `StartMegaproject` 하나, AI의 구호·개혁·안정화(거대 프로젝트와 승천 판정 작업 — Phase 4 목록에서 당겼다).

v0.7 추가(+5~6일, 스키마 검사기 +3일): 생성과 이 Phase의 단계들의 시뮬 추첨 지점이 바뀌는 일을 이 Phase에 한꺼번에 한다 — 같은 시드가 주는 은하는 이 Phase의 은하 생성 규칙 변경에서 한 번 바뀐다(§9.2).
v0.7 정의: `FGalaxyDef`, `DT_Galaxies` 6행(문구는 사실 확인의 안전 문안, §3.4), 로더·스크립트의 테이블 목록·`Sim.SaveRestart`의 스탬프 키(은하 행의 `CosmicRows` 상수는 두지 않는다 — 고향은 노브 `HomeGalaxyRow`가 부른다), `DT_Rules` 노브 8개(`HomeGalaxyRow`, `RealmCapByTier`, `SightLanesByTier`, `SightIntervalTicks`, `FirstSightTick`, `LivingWorldStarShares`, `FallenCoreMinLanesFromHome`, `FarLineCooldownTicks`), 태그 `Galaxy.Kind.*` 4(136 → 140)(§7.3).
v0.7 스키마 검사기('첫 이벤트 데이터' 작업 전, 3작업일 시간 상자): `Tools/data/schema.json`의 첫 조각, `Tools/make_data_tables.py`의 검사, 로드한 에셋을 보는 C++ 테스트 하나(`Data.Integrity`). 범위는 §9.3의 `Data.Integrity` 행(§7.3).
v0.7 은하 생성 규칙 변경(§7.4 `NewGalaxy` 주석의 10항): 수를 고향 행에서, 클램프 분리, `GalaxyRow` 도장, 패딩 없는 ID와 `NextCivSerial`, 원판 기각 추출과 삼각 함수 제거, `sol`의 자리, 항성 분포, 이웃 보장, 이름 비복원과 `NameBank`, 초기 코어의 자리.
v0.7 시뮬: 평가 문맥 `FCosmicEvalContext`의 `GalaxyRow`와 스트림(문맥의 `GalaxyRow`는 호출자가 평가 대상의 `GalaxyRow`를 넣는 값이라, 이 Phase의 첫 헤더부터 은하 생성 규칙 변경이 도장을 찍기 전까지는 NAME_None이 실린다 — 노브 `HomeGalaxyRow`와 도장은 은하 생성 규칙 변경과 함께 온다), 정착된 성계만 Hazard, `project_settle`과 AI 확장 규칙·보유 상한, 문명 배열의 정리(ⓞ)와 색인, 파괴된 성계의 죽은 노드(§16 #29 ⑤와 함께), `TickSight`·`NoteMet`·`NoteKnown`과 만남의 줄(`NoteMet`이 첫 접촉 때 `Knowing.FirstContactCivID`를 적는다 — §7.4), 통치 문명에 성계를 주는 자리의 `NoteKnown` 호출(정착의 시작·`TransferSystem` — 줄을 내기 전에, §8.1. 정복의 자리는 분쟁 단계가 본문을 얻는 Phase 4에), 이야기의 범위·`bUntold`('아는 범위와 로그' 작업의 커밋이 Phase 1의 텍스트 패널 `UCosmicTextPanelWidget`에도 `bUntold` 항목을 건너뛰는 한 줄을 넣는다 — 새 HUD 피드의 건너뛰기는 Phase 3 꼬리)·먼 줄 2행(간격 `FarLineCooldownTicks`는 `log_far_event`에만 — `log_far_ascent`는 매번 낸다)·`SetTellAll`(커맨드렛은 이 Phase부터 `SetTellAll(true)`로 돈다 — 종료 실측도 그 모드다. 인자 `-TellAll`은 종료 리뷰에서 들어왔다), `CivName` 게이트 삭제(`TickResearch`가 `FocusCiv`를 넘긴다), `FallenTranscendentCount`(코드 기본값 3, `DT_Rules.json`의 1은 VS의 데이터 값 — 둘 다 그대로 두고 로스터가 0~3으로 읽는다).
v0.7 테스트: 새 7종(`Sim.KnowingInert`·`Sim.SightGrowsOnly`·`Sim.FirstContactTimeline`·`Sim.LayersInRange`·`Log.TellScope`·`Sim.RealmCap`·`Sim.WildWorldsQuiet`)과 아는 범위와 로그 작업이 더한 둘(`Sim.SightStage`·`Sim.KnowingWriters`), `Sim.GenerationRules` 보강, `Data.Integrity` 첫 조각, 테스트 상수(§9.3).
v0.7 리뷰: '아는 범위와 로그' 작업(`TickSight`, 이야기의 범위, 세계 해시, `Sim.KnowingInert`)은 Phase 2의 방식(적대 리뷰)으로 리뷰한다.
v0.7 실측(종료 리뷰에서 했다, 2026-10-17): 임시 측정 테스트로 60·12(시드 1~29와 42)와 300·48을 재고 그 셈을 커맨드렛으로 옮겨(§9.3의 열한 컬럼·행별 줄 수·요약 줄 — 재감사가 커맨드렛만으로 30판을 다시 쟀다), §9.3의 잠정 시작값에 분모를 적고 곁에 Phase 2 기준값을 적었다('첫 600틱에 말해진 줄'의 목표는 스크립트 통치자가 오는 Phase 5에 맞춘다).
**종료 조건**: 입력 없이 돌려도 Hazard가 발생·만료·해결되는 로그가 나온다. 침묵 영향권에 인접하고 그 주인의 마지막 미편입 성계가 아닌(`Law.Remnant`) 정착된 성계의 위기가 만료되면, 영향권이 상한(`MaxInfluencedSystems`) 아래일 때 `Hazard.Claim`이 뜨고 만료 시 영향권이 1 커진다 — 입력 없는 20,000틱(시드 42, 60·12)에서 자연히 1회 이상. 입력 없는 판에서 `Sim.KnowingInert`가 통과하고, `Sim.FirstContactTimeline`이 통과한다(시드 1~50, 첫 접촉 줄이 늦어도 틱 340 — 세계 코드가 통치 문명을 상대로 `NoteMet`을 부르는 자리가 없고 게임의 행이 `TransferSystem`을 쓰지 않는 이 Phase에서는 틱 330~340, §7.4). 플레이어가 초신성 위기에 프로젝트로 대응해 T3로 승천하는 플레이가 콘솔 명령 없이 가능하다. 안정도를 방치하면 반란 → 폐위 → Role=Wanderer 전환과 '몰락한 군주' Severity 2 로그가 콘솔 명령 없이 발생한다. AI 문명의 반란 만료 시 그 문명의 열린 `Notable.Rebel` 기록이 `Notable.Ruler`로 바뀌고 `Sim.NotablesCapped`·`Sim.NotablesFromEventsOnly`가 통과한다(이름 렌더 `{Ruler}`와 남은 세 행(`arch_psionic`·`arch_covenant`·`arch_merchant`)의 이름 36개는 Phase 4. `arch_terran`·`arch_default`의 뱅크 24개는 위기 처리 작업에서 전조 ② 행과 함께 채웠다). 증거(종료 리뷰, 2026-10-17): 테스트 `Sim.PhaseTwoExit`(게임 데이터로 시드 42의 위기 발생·만료·해결과 그 줄, 주장과 영향권의 성장, 시드 7의 폐위와 '몰락한 군주' 줄 — 시드는 잰 것이고 규칙이 아니다), `Sim.KnowingInert`(20,000틱), `Sim.FirstContactTimeline`, `Sim.PlayerAscendsToT3`(손으로 놓은 초신성에 대응하는 길 — 초신성이 저절로 서는 것은 AI 판들이 보인다: 30판에 약 900번, 둘째 칸 백성마다), `Sim.RebellionThrone`·`Sim.NotablesCapped`·`Sim.NotablesFromEventsOnly`, 그리고 커맨드렛의 요약 줄.

### Phase 3: 은하 맵 뷰포트 (4주 + 꼬리 5일)

성계 60개 3D 노드, 레인, 카메라, 클릭 선택 → 우측 정보 패널. 소유 색/Hazard 경고/프로젝트 링 표시.
상단 바(자원 5종, 틱, 배속), 하단 로그 피드(ListView, 클릭 시 카메라 포커스). 승천 연출(`NS_Ascension` + 사운드 + `MythicNarrative` 오버레이 — `UAscensionSubsystem::OnTierAscended`를 듣는다. Phase 2에서 옮겼다). 거대 프로젝트 시작은 정보 패널의 버튼(Phase 2의 프로토타입 키 P를 대신한다).
**종료 조건**: 마우스만으로 Phase 2의 플레이가 가능. 성계 60에서 60fps 유지.
꼬리(7·12·8·9번은 손 확인을 기다리지 않는다 — 2026-10-18 사용자의 '이어서 진행해주고', 아래 상태 줄. 10번 종료 리뷰만 손 확인 뒤. 손 확인은 가장 새 빌드로 한다 — 그 빌드는 셋째 칸의 ×25, 회색 명령 단추와 그 사유·값의 툴팁, '다음 칸까지', '멈춤 — 자원 부족', 영적 시야를 보인다. 볼 것의 목록은 WORKPLAN §3의 '다음'): 영적 시야 토글 파라미터(`M_Star` `ViewMode`/`Pressure`/`DominionColor`/`WarningColor`, 레인은 `M_Lane` 대신 압력에 회색이 되는 반쪽 선, `M_Nebula`의 `SpiritualIntensity`, §6.4) + 주장 표식 `ClaimState` 파생 파라미터 1개(§4.2, Presentation만). M_Star·ACosmicStarSystemActor를 처음 만드는 시점에 넣는 것이 나중에 다시 여는 것보다 싸다. 약 2일. 같은 꼬리에 성계 패널(WBP 없는 C++ 위젯 `UCosmicSystemPanelWidget`)의 '{PopBand} 백성' 1줄(`GetPopulationBand`, §8.3 — 인구 숫자·비율 없음, 초상·수치·클릭 액션 없음). 약 1시간. (`GetPopulationBand` 구현은 Phase 1에 끝났다, §7.4) 규모 등급 라벨 ST_CA 6을 같은 꼬리에 포함.
v0.7 꼬리 추가(+5일 — 8·9번 작업. 처음에는 종료 조건 달성 뒤였고, 2026-10-18부터는 손 확인을 기다리지 않는다: 위 꼬리 줄): 모르는 성계는 배경 별 인스턴스의 제 자리 점(별을 숨긴다 — `M_Star`의 `Known` 대신, §16 #105), 보이지 않는 주인의 소유 색·이름·일 가림, 죽은 별 표시(1번 작업에 이미 있었다), 레인은 두 끝이 아는 성계일 때만, 배경 별 인스턴스(드로 1), 나선팔 그림과 '여기' 표지(v0.8 — 단계 5로 옮긴다: 실제 하늘(태양 근방의 관측된 계로 짓는 지도)의 자리), 은하 패널(종류·별의 수·거리 FText, 백성 줄 없음), 피드의 `bUntold` 건너뛰기(4번 작업에 이미 있었다), 만난 순서 12색 팔레트, 생명 낱말 한 줄과 상한 안내 한 줄(상한 안내는 단계 0의 정착 사유 `efe5c37`에 이미 있었다 — '{PopBand} 백성'은 7번 작업), ST_CA 라벨 약 25(8번 작업이 스물을 더했다 — §6.4). 성계 60개는 계획대로 전부 액터다(§9.4). 맨 끝 항목으로 우주 화면(같은 지도의 줌 단계: 풍경 은하 10,000 + 이름 있는 은하 20 — v0.7의 6과 새 표시 전용 14(M87과 함께 표시 전용 15). 끝, 2026-10-18) — Phase 3 종료 조건과 무관하고 밀리면 Phase 5로(§5.11). 테스트 `UI.KnownFilter`(§9.3). 배경 별을 넣은 뒤 60fps를 다시 잰다(성계 60 + 배경 별 — 8번 작업에서 창 없이 게임 스레드를 쟀다, §9.4. GPU는 손 확인 때 창에서).
v0.8 꼬리(+1.5~3일): 시간 표시 토글(단계 2 — 끝, 2026-10-18), 셋째 칸의 ×25·명령 단추의 사유·'다음 칸까지' 툴팁(12번 — 끝, 2026-10-18), 은하 패널의 한 줄(모든 은하에 같은 문장, 8번 작업 — 끝, 2026-10-18), 표시 전용 은하 행(9번 작업 — 끝, 2026-10-18).

### v0.8 예약: Phase 3 종료 뒤, Phase 4 첫 헤더 전 (11~15.5일)

v0.8의 단계 번호(WORKPLAN §3 — 본문의 '(v0.8 — 단계 N부터)'가 이 번호다): 0 정착 사유(끝남, `efe5c37`) → 1 문서 반영 → 2 시간 표시 토글 → 3 Phase 3 꼬리(7~10번 작업과 v0.8 몫) → 4 아래 ① → 5 아래 ② → 6 Phase 4 첫 블록 '서막(부르심)'(첫 틱 전의 카드 서막과 그 끝의 부르심) → 7 Phase 4의 외계 백성 묶음(아래 Phase 4의 'v0.8 추가') → 8 Phase 5 '세계관과 출처' 최소형.
① **성역과 첫 생애 행**(단계 4 — 헤더 한 번 승인, 커밋 둘. 성역은 다시 쓸 수 없는 법 하나로 소유·정착·주장·파괴에서 빠지는 태양계 노드 `sol`이고, 첫 생애 행은 플레이어의 첫 생애 백성을 만드는 `DT_Civs` 행이다): 4a 성역 — 다시 쓸 수 없는 다섯째 법(가칭 `Law.Hedge`)·성계 술어(가칭 `CosmicState::IsSanctuary`)·막는 자리(정착·넘김·주장·교단·초기 코어·모성계·출생·계승·즉위·파괴·영향권, 압력 0), 생성의 잎 노드와 ID(`sys_0` = 첫 생애의 모성계, `sol` = 성역 — §7.4 11항), 정착 사유 '성역'(맨 앞 — 그 정착 단추에는 값 줄이 없고, `PeopleSettleBlockOf`에서는 영향권 아래·주인 있음 곁: 12번 작업의 `static_assert` 둘이 알린다)·성역의 패널·영향권 사유의 그림, 성역의 그림과 선택(8번 작업이 둔 자리: `FCosmicKnownView`의 둘째 비트(가칭 `ShownByName`)를 `ShownLookOf`·`LightSlots`·`PickCandidatesOf`·`SelectSystem`·성계 패널의 앎 문(`Refresh`)이 함께 읽는다, 성역에 닿는 레인을 정한다, `OwnerLineOf`의 첫 줄, `CosmicUI::NoLifeLineSystemID`를 성역의 술어로), 영적 시야의 성역 경우(`CosmicMapLook::SpiritOf`의 첫 경우 — 색 없음·압력 0, `SpiritTipOf`의 갈래와 ST_CA `Sight.Sanctuary` "이곳에는 압력이 닿지 않는다." — 패널의 한 줄과 그 툴팁, §6.4 '색'), 로드 때 주인 있는 성역 거절, 개발 세이브 지우기, `arch_terran.HomeSystemNames` 비우기. 증명: 커맨드렛 요약의 결정적인 칸이 같고 무주 성계 수만 +1, `Sim.SanctuaryKept`. 4b 첫 생애 행 — 가칭 `arch_seeker`의 뼈대(행 이름·태그·`Weight` 0·특성·제 해 이름·첫 줄), `DT_Rules.PlayerArchetypeRow`와 C++ 기본값, `arch_terran` 은퇴, 낱말 경계 금지어 검사(스키마의 가칭 `other_works`·`holy_and_myth` — 이름 3단계 검사는 `DT_Transcendents`의 `Title`·`Lure`도 덮는다: §4.2 '하나님의 성품 칭호를 그들에게 역으로 붙이지 않는다'), 고칠 테스트(§9.3). 증명: 새 기준값(§9.3, CLAUDE.md, `Sim.PhaseTwoExit`의 증거 시드를 다시 고른다). effort xhigh, ultracode 리뷰 1회(저장·결정론). 2.5~3.5일 + 리뷰 0.5~1일.
② **실제 하늘**(단계 5 — 태양 근방의 관측된 계·행성·이름으로 짓는 지도): 관측 목록 도구와 스냅숏(`Tools/data/sources/` README), 관측 목록 행 구조체와 `DT_Planets`, 투영과 레인(오프라인 평면 그래프, 손 고침 파일), 별 이름과 거름, `FHazardDef`의 등급별 표시 이름과 '항성 안정화'(상단 바 '다음 칸까지'의 '{위기}에 답할 때'도 그 이름을 읽는다 — `FCosmicNextRung::SystemIndex` 성계의 등급. 그 위기가 백성의 정착된 성계에 이미 서 있으면 '(시작 전)'으로 읽을지도 그때 본다 — 12번 작업의 리뷰), 에너지 상수와 정규화, `LivingWorldStarShares` 은퇴, 첫 생애의 별 고르기(§16 #92), 성역의 실제 자리, 나선팔·'여기' 표지, 배경 별과 광년 금(표현 계층 — `CosmicMapLook::BackdropPointsOf`를 갈아 끼운다(§3.1 '배경 별의 그림'): 점의 모습 법칙이 바뀌면 모르는 성계의 슬롯(`BackdropSlotOf`)도 같은 법칙), 환경 특성 `Trait.World.*`(계 행 — 문장은 Phase 4), 먼 표지(표시 전용 행), 스키마 규칙과 `Data.SourcesInSync` 도장 확장, 성역을 지나는 레인 셈(§16 #80), 우주 화면의 손잡이(§6.4 '우주 화면' — 구역과 우주 사이의 줌 단계 '은하'(열거자와 멈춤 한 쌍), 단위·틀 `FrameOf`와 고향 그림을 실제 하늘의 것으로 갈아 끼우고, 은하 경도의 방향(남은하극 쪽에서 본 거울)을 물려받거나 §16 #111의 다른 길로). Phase 5 전에 모든 시드의 기준값이 한 번 바뀐다. effort xhigh(지도 표현은 medium), ultracode 리뷰(생성·결정론 — 지도 표현은 끔). 8~11일.
**종료 조건**: ① 두 커밋의 증명과 `Sim.SanctuaryKept`가 연속 두 프로세스에서 통과하고, 첫 생애가 `sys_0`의 찾는 백성(계시 양식이 '찾음'인 백성)으로 시작한다. ② 관측 목록 60계 + 성역으로 판이 생성되고(시드마다 같은 지도, 백성·사건은 시드가 바꾼다), 새 기준값이 §9.3과 CLAUDE.md에 적힌다.

### Phase 4: 방랑자 vs 통치자 루프 (6주 + 1~1.5일)

방랑자: 이동, 조우 카드 UI, 능력치 판정(Exaltation 포함), 유물, 수련, 경외도. 조우 25장(몰락한 군주 3~5장, 길 1종(`Path.Covenant`) 5장, 사자 카드 3장, 성역의 경계 '지경' 1장(v0.8) 포함).
통치자: 연구 중점 슬라이더, 외교 태세/조약, 칙령 9종(`edict_vigil`(주장 연장 포함), 안식 최소형, 낭독 `edict_reading` 포함), 거대 프로젝트 7종(개혁·구호 `project_relief` 포함 — 그 가운데 개혁·구호·안정화 행과 `StartMegaproject`, AI의 사용은 Phase 2의 거대 프로젝트와 승천 판정 작업으로 당겼다(§16 #56). Phase 4는 나머지 행(차원 방주 따위)과 통치자의 프로젝트 화면 — 시작 버튼은 Phase 3의 정보 패널이 프로토타입 키를 대신한다. 정착 `project_settle`은 Phase 2), 함대 배치, 청원 카드 9장(첫 접촉 `petition_first_contact` 1장 포함. `Trait.Society.Divided` 의회·회중 2화자 변형은 8장이고 첫 접촉 청원에는 없다 — 화자를 Trait로 덮어쓰는 분기 1개, §5.10. v0.8: 헤롯 패턴 청원을 더해 10장·2화자 변형 9장, §16 #93), 시대 명명 이벤트, 모티프 행 바벨·느부갓네살(Phase 2에서 옮겼다, §8.6 — 기대는 장치 셋은 이 목록에 아직 없어 헤더에서 정한다), §16 #29에서 정한 어휘(조건 `CivStabilityAtLeast`·`PlayerHasPower`, 효과 `GrantRelic`·`RemoveRelic`, Key 선택자 낱말 Ruled·빈 상대 — 처음 쓰는 행과 함께 끝에 더한다), 책을 되찾는 둘째 길(낭독 칙령이 낭독 주기마다 발견 행을 낸다, §5.8 — 정착과 문명 제거 작업의 헤더에서 정했다), 모든 백성의 추첨 행과 군체·기계 아키타입(§16 #64 — 기본값이면 조건 `CivLacksTrait`를 끝에 더하고 광맥·에너지망 문장을 고친다), `DT_Dedications` 최소형(Epithet 10행 + 의회·회중 헌정명), 생애의 최고 칸과 통치로 받은 권능(§16 #67 ①의 즉위·귀환 쪽 — 이미 높은 백성을 통치하게 된 생애 — 과 ②).
전환: 섭정 위임, 즉위, 복귀, 폐위 3화면(§8.4 재사용). `trans_silence` 행의 텍스트 컬럼 채움(`CounterfeitOf`·`Lure`·`VoiceTemplates`·`ClaimTemplates` — `Title`은 Phase 3 7번 작업이 채웠다: "꺼뜨리는 이"(그 낱말과 `feat_supernova_stabilized`의 동사 '붙들다'는 §16 #99). `Allegiance`·영향권은 Phase 2에 이미 있음), 영압의 수정자 ②(분쟁 단계 — 압력 함수와 수정자 ①③은 Phase 2 영적 압력 작업에, ④는 정착과 문명 제거 작업에 이미 있다), `Hazard.Visitation`, `Trait.Faith.*` 행(`Hazard.Claim` 발생 가중·카운트다운 보정 + `Log.Record` 푸시 포함). 남은 Hazard 3종(`Hazard.SubspaceErosion`·`Extinction`·`Cult`)과 그 R/W 해결 수단(방주 프로젝트, 피난 호위 체인, 침묵의 금지 칙령 + 청원 체인 — 절멸의 만료 훅은 통치자에게 `KillPlayer`를 걸고, 초신성의 붕괴처럼 통치 문명이 성계를 모두 잃었으면 `DeposeRuler`를 이어 적는다: 출생 보호가 가린 사망은 기록 자리를 차지하지 않아 폐위가 적히고, 그러지 않으면 건넘 단계의 정리가 그 문명을 계속 남겨 둔다, §7.4 ⓞ — 정착과 문명 제거 작업의 리뷰 수정분 재감사), `Hazard.Claim` 해결 콘텐츠(`project_relief`, `enc_witness`, `edict_vigil` 연장, 주장·기록·성장 로그 약 30줄). 계시 양식: VS 7행 × `RevelationTemplates` 3줄 = 21줄, `DT_LogTemplates`의 계시 틀 이름 행과 선택 행, `PickTemplateRow`, `{Revelation}`·`{Transcendent}` 슬롯(§5.11, §8.2). 말을 받은 백성: 고발 Book 변형 3줄, 첫 생애 백성의 `Faith.Bank.Nations`·`Civic`·(말이 온 뒤) `Hebrew` 최소형. 요약 단위(§1.3, 약 1일): 망명 변형 '충신이 몰래 배를 내준다'·`crisis_visitation` 행의 `OnResolveEffects`·'책이 발견되었다' 행에 `NameNotable`(`Notable.Loyalist`·`Hearer`), 역할 슬롯 6 해석과 ST_CA 집단 폴백 6, 다이제스트 항목 '왕좌가 바뀐 문명', `PersonNameBank` 5행 × 12 = 60 가운데 세 행 36개(`arch_terran`·`arch_default`의 24개는 Phase 2 위기 처리 작업, `arch_swarm`·`arch_machine` 비움. v0.8 — 단계 7에 첫 생애 행의 외계 이름 12가 `arch_terran`의 인간 이름 12를 대신한다), `{PopBand}` 템플릿 3~5(역병·절멸·붕괴·주장 인접, 애도로 끝남)(규모 등급 라벨 ST_CA 6은 Phase 3 꼬리), 재회 조우 '알아본다/숨겨준다' 변형에 조건 `CivHasNotable`(§5.10, §7.3, §8.2). 순증 약 1주 + 3일(콘텐츠 작성 포함).
Phase 2 종료 리뷰가 넘긴 것(안정도의 바닥·AI 구호의 정책·T2→T3의 속도는 MVP ③에서 끝났다 — 2026-10-22, `DECISIONS_LOG`): 안정도의 바닥과 AI 구호의 정책(§16 #68·#71 — AI와 칙령의 헤더에서. 기본값은 둘을 함께 넣는 것이고, 정하기 전에 #68만·#71만 넣은 커맨드렛 판을 하나씩 잰다 — 회복만으로 주장의 정착률이 떨어진다는 것은 짐작이다. 회복은 입력 없는 그릇의 저절로 된 폐위를 없앨 수 있다: 칙령의 유지비와 같은 커밋에 넣거나 `Sim.PhaseTwoExit`의 시드 7 블록을 다시 잰다), T2→T3의 속도(#72 — 이 Phase의 종료 조건과 함께), 남의 완성 기록(#73 — 첫 `TransferSystem` 행이나 분쟁 단계 전), 웅성거림 줄의 양(#68 뒤에 다시 센다 — 불만 100틱 안의 두 줄은 지킨다, `Sim.RevoltTelegraph`), 단계별 틱 시간(커맨드렛에 단계마다의 시간 — 외교·분쟁 단계가 본문을 얻을 때), 효과나 코드 훅만 놓는 위기의 생성 규칙(방문·징조 — §7.3 `SpawnRule`에 열거자 하나).
v0.7 추가(+1~1.5일): `petition_first_contact`(§16 #24의 태세 저장 위치를 먼저 확정)와 `PendingContactCivID` 채우기(통치자, 그 판의 첫 만남만 — `NoteMet`이 `Knowing.FirstContactCivID`를 적는 그 만남), 세계 코드의 `NoteMet` 호출(외교·분쟁·청원·조우가 통치 문명이나 선 성계를 상대로 삼는 자리마다)과 정복이 통치 문명에 성계를 주는 자리의 앎(`TransferSystem`의 후속 처리 기록을 `FollowUpKnowing`이 줄보다 먼저 따른다 — `NoteKnown` 다음 준 백성의 `NoteMet`), 명령의 전제(태세·조약·즉위는 만난 백성, `TravelTo`와 T3 도약은 아는 성계), `ScanCurrentSystem`과 `ScanSightBonusLanes`, `log_scan_wild`, `BuildWorldDigest`와 Interlude를 `Knowing` 기준으로, 여행 중에 출발지나 목적지가 파괴된 그릇(§16 #53 ②는 선 그릇만 정했다 — `TravelTo`의 헤더에서), 다른 원천(이름으로 부르는 행·조우의 해결)이 먼저 적은 폐위와 같은 틱의 반란 만료 줄(그 원천이 처음 오는 작업에서 — §7.4 인물 훅 ②), 희소 `Relations`(없는 키 = 0 = 중립을 먼저 고정, §5.5). `Data.Integrity`의 텍스트 규칙은 첫 텍스트 묶음 전에 넣는다. `Hazard.Cult` 만료 효과를 쓰기 전에 남은 자 법의 범위(§16 #43)를 확정한다. `Sim.FirstContactTimeline`에서 하한 330의 단언을 뗀다(세계 코드의 `NoteMet`이 통치 문명을 상대로 불리기 시작해 이웃을 더 일찍 만날 수 있다 — 상한 340의 단언은 그대로, §9.3). 검사 하나 추가: 스크립트 통치자 아래에서 플레이어 소유 성계에 주장이 선 시드의 비율(시드 1~50). 기준 비율은 이 검사의 첫 실측으로 정해 §9.3에 적는다(입력 없는 그릇은 한 성계라 Phase 2의 기준은 0이다). 통치자는 테스트 안에서 스크립트한다(`Sim.SphereGrowthResponsive`처럼 — 커맨드렛의 `-PlayerPolicy`는 Phase 5).
v0.8 추가: 첫 블록 '서막(부르심)'(단계 6 — 조우 카드 화면을 먼저 만들고, `DT_Encounters` 앞에 `make_data_tables` 검사를 중첩 구조체까지 넓힌다, 서막 카드 약 8장, 부르심의 Core 함수 가칭 `ReceiveCalling`, `UI.PrologueGate` — 1.5~2주 + 콘텐츠). 외계 백성 VS 5행 + 첫 생애 행의 본문 + 고넬료 사슬(방문 응답 → 사자의 전언 → "말이 도착했다"로 이어지는 사건 줄)·'옛 땅의 기억'(말이 도착한 뒤 그 백성에게 열리는 한 쪽짜리 화면) + 헤롯 패턴 청원(백성이 통치자를 높여 신이라 부르려 하는 청원, §5.10) + 대도감(개발자의 `cosmic_species_codex.md` — 성역 밖(태양계 바깥)의 세계관 원천, §5.11)의 문장(단계 7, §16 #64와 함께 — 2~3.5일 + 재측정 0.5일). 방랑자의 성역 경계 카드 '지경'과 레인 이동 시간(`TravelTo` 헤더, §16 #81·#83). 저장 작업(§16 #33 ①②③·#70·#40 ⑦)이 단계 6 앞에 따로 선다.
**종료 조건**: 새 게임에서 30분 동안 양쪽 모드를 각각 한 번 이상 경험하며 T1→T3 도달이 가능하다(몇 배속에서인지 이 Phase의 헤더가 적는다 — 1배속의 30분은 1,800틱이고 Phase 2의 AI는 T2에 약 1,330틱, T3에 수천 틱이 걸린다, §16 #72).

### Phase 4b: 부름 루프 (2주 + 0.5일)

KillPlayer 처리(기록된 사망이면 통치하던 문명의 `bIsPlayerRuled`를 풀어 건넘 단계의 정리가 성계 없는 그 문명을 놓게 한다 — §7.4 ⓞ, 정착과 문명 제거 작업의 리뷰. 출생 보호가 가린 절멸의 사망은 기록되지 않아 여기 오지 않는다 — 절멸의 만료 훅이 폐위를 이어 적는다, Phase 4) → `FLifeRecord` 기록 → 귀환 제안 훅 `OfferReturn()`(약 40줄. VS에는 회귀 행이 없어 실전에서는 뜨지 않으므로 테스트 주입 행으로만 검증, §4.4) → 공백기 도약 → 계승(`SuccessionKind` 기록) → 다이제스트 → 3화면 → 저장 트리거 → 빈 은하 가드('침묵의 우주') → 연대기 '두 책' 장(`BuildChronicle` 약 40줄, 에필로그 둘째 줄, §8.4) → 테스트 8개(`Calling.LeapDeterminism`, `Calling.HardReset`, `Calling.LeapResumeRoundTrip`, `Calling.NoPlayerDuringLeap`, `Calling.LureGate`, `Calling.ReturnLureBothPaths`, `Sim.SuccessionPossible`, `Sim.NoDanglingRefsAfterLeap`, §9.3) → 템플릿 65~70줄(§8.4 콘텐츠 예산) → `Install()`의 `FLifeRecord.NameIndex` 추첨 사용(`Called` 새 값·`Returned` 동일·이름 없는 문명은 `arch_default` 뱅크, §4.4)과 `Calling.ReturnLureBothPaths`·`Calling.HardReset`의 요약 단위 단언(§9.3, 약 2시간). 계승의 권능 재부여는 §16 #67 ②를 먼저 정하고, 묘비의 '최고 Tier'는 `FLifeRecord.PeakTier`를 읽는다(§16 #67 ①).
순증 7~9 작업일. §4.4가 원래 요구하던 사망 분기 약 4일을 대체한다. Phase 3·4의 코드는 변경하지 않고, 헤더는 Phase 1에서 예약했으므로 SaveVersion 마이그레이션이 없다. 양 모드가 소비한다(Extinction 통치자 사망(섭정 없음), 방랑자 사망, 계승 후 즉위).
v0.7 추가(+0.5일): `Calling.HardReset`에 '계승 전의 `Knowing`의 세 배열(`KnownSystems`·`MetCivs`·`MetGalaxies`)이 계승 뒤 배열의 앞부분과 같고 `FirstContactCivID`가 계승 전과 같다', `Calling.NoPlayerDuringLeap`에 '도약 중 `Knowing`의 세 배열과 `FirstContactCivID` 불변(`TickSight`·`NoteMet`·`NoteKnown` 0회)' 단언(`FarLineUntilTick`과 `PendingContactCivID`는 도약 사이에 바뀔 수 있어 단언에서 뺀다, §9.3). 다이제스트의 닫는 줄 ST_CA `Digest.Elsewhere`(§8.4). 계승 화면의 한 줄 '부름은 본 것을 잊지 않는다'(§4.4).
**종료 조건**: 방랑자 사망 → 연대기(두 책 장 포함) → 공백기 도약 → 계승이 콘솔 명령 없이 1회 이상 가능. 계승 후 §5.10 조건으로 즉위 가능. 테스트 8개 통과.

### Phase 5: 수직 슬라이스 폴리시 (4주 + 1일)

밸런스 커맨드렛으로 등급별 페이싱(§2.4)을 ±30% 안에 맞춘다. 커맨드렛 인자 `-DeathAt`·`-PlayerPolicy`(귀환 수락 확률 포함)·`-PlayerArchetype`·`-Fallen`, CSV 컬럼(부름·통치 + 영혼 경제·귀환·두 책), 테스트 `Data.NoVerdictWords`·`Data.PathDoctrineVocab`(`Sim.RevoltTelegraph`는 Phase 2의 위기 처리 작업과 함께, `Sim.RealmPressureMonotonic`은 영적 압력 작업과 함께 이미 온다), `Data.Integrity` 템플릿 린트와 v0.5·v0.6 추가 항목(요약 단위 린트: 사람 수 정규식·역할 슬롯과 `{PopBand}`의 두 책 금지·`PersonNameBank` 규칙·`NameNotable` 금지 행. v0.7에서 `Data.Integrity`의 시점을 당겼다 — 첫 조각은 Phase 2, 템플릿 린트를 비롯한 텍스트 규칙은 Phase 4 첫 텍스트 묶음 전이다: §9.3, 아래 'v0.7 추가'), CSV 요약 단위 컬럼 4, 부름·통치 밸런스 목표와 영혼 경제·요약 단위 시작 목표(§9.3). 약 3일 + 3시간.
온보딩: 첫 10분을 안내하는 로그 기반 튜토리얼(별도 UI 없음 — MVP ⑥의 화면 안내 셋(`docs/20` §6.3)과 따로. 타임라인은 §2.4, 첫 생애의 첫 10분 완화 7개는 §5.11).
외부 테스터 5~10명(비기독교 테스터 ≥ 절반 + 교회 공동체 1~2명). 피드백 수집 → 단일 개선 목록. 테스터 질문에 추가: "죽음이 공평했는가", "죽은 뒤에도 계속하고 싶었는가", "상위 존재 앞에서 무력했는가, 아니면 작아졌는가", "로그가 설교처럼 느껴진 순간이 있었는가", "반란이 올 것을 미리 알았는가" + 영혼 경제·귀환·첫 생애·두 책·문턱 질문 8개 + v0.8 질문 2개 + 요약 단위 질문 2개(§9.3). 지표: 첫 10분 이탈 < 10%, '설교처럼 느낀 순간' 세션당 ≤ 1회(§5.11 완화 ⑦). 이 반응으로 '관조 모드' 토글·노화 활성·폐위 후 위치(모성계/인접)·받는 길 두 변형 유지·타락 1인칭 로그 밀도를 결정한다(§16).
v0.7 추가(+1일): 커맨드렛 `-TellAll`과 CSV 컬럼 11개(§9.3). EA 규모(300·48)의 틱 비용·세이브 크기·생성 시간·커맨드렛 실행 시간(20,000틱)·해시 시간을 실측해 §16 #3을 확정한다(넘으면 200·30, §9.4). 보는 거리·`FirstSightTick`·`RealmCapByTier`·`LivingWorldStarShares`(v0.8 — 단계 5에서 은퇴한다: 항성 등급은 관측 목록을 따른다) 조정, §9.3 목표 확정. `Data.Integrity`: 동결 이름·끝에만 추가 검사를 첫 테스터 빌드 전에 넣는다. 업적·프로젝트가 그 칸의 보유 상한보다 많은 성계를 요구하지 않는지도 여기서 본다(텍스트 규칙은 Phase 4 첫 텍스트 묶음 전에 이미 들어가 있다). 테스터와 시드를 나누기 전에 패키징한 게임 타깃의 세계 해시를 비교한다(§9.2). 테스터 질문 4개 추가(§9.3): 지도가 비어 보였는가 / 처음 만난 백성을 기억하는가 / 먼 곳에서 태어난 그릇이 옛 지도를 아는 것이 어색했는가 / 은하 패널이 백과사전처럼 읽혔는가.
v0.8 추가(+1일): '세계관과 출처' 최소형(단계 8 — 성역의 문장(설계 기록 C.2), 출처·라이선스, '천문 수치는 관측이고 그 위의 생명·백성·사건은 허구', '우주 화면은 우리은하를 가운데 두고 그린 그림이다 — 방향은 은하 경도, 거리와 크기는 압축했다', '생물은 지어진 것'(대도감의 괴물은 신이나 악마가 아니라 피조물), 케플러·TESS가 찾은 행성 목록의 자료 줄 — 텍스트 감사).
**종료 조건**: 테스터 과반이 "다음에 뭘 할지 알겠다"와 "로그를 읽는 게 재미있다"에 동의.

### Phase 6+: 콘텐츠 확장과 Early Access (6~12개월)

**Phase 6a — 건넘 최소형 (Phase 6+의 첫 블록, 2~3주, EA 출시 빌드에 포함)**: `SummaryAt`과 임계 틱 조정(커맨드렛 CSV의 고향 분포에서), `TickCrossings`의 도착·출발 본문, `project_crossing`과 AI 규칙, AI의 T3→T4 판정과 `AIFourthRungCap`, `CrossingMinTier`·`CrossingCooldownTicks`·`CrossingSettleTicks`, 건넘 로그 4행, `MetGalaxies`와 은하 패널의 요약 낱말·건넘 주기 라벨(격 게이트 뒤 — 6a의 요약 낱말은 은하 패널의 글로만, 우주 화면의 색·고리·표지가 되지 않는다, §6.4 '우주 화면'), `PeopleStems` 32, 태그 `Transit.People`(140 → 141), EA 규모 데이터(고향 행 300·48)와 나머지 성계의 인스턴스 메시(§5.7, §7.4. 콘텐츠 약 1일은 §8.4). 착지 추첨은 '시뮬 추첨 지점은 테스터와 시드를 나누기 전(Phase 5 전)에만 더한다'의 명시적 예외다. 밀리면 버전 노트에 '같은 시드의 역사가 첫 도착 틱부터 달라진다'를 적는다. 테스트 4종: `Sim.SummaryIsPure`·`Sim.TransitArrives`·`Sim.HomeUnchangedBySummaries`·`Sim.ArrivalHasNoNotables`(§9.3). 일정을 잡을 때 §12.3의 둘째 물음(두 모드가 소비하는가)을 다시 묻는다.
**6a 종료 조건**: 시드 42의 한 판(18,000틱)에 `ResultCivID`가 찬 `Transits` 기록이 1개 이상, `Sim.HomeUnchangedBySummaries` 통과, 건넘과 도약이 연 '다른 질서' 엔딩 합계 15% 미만.
**Phase 6b — 플레이어의 건넘 (6a 다음 블록, 계획 후보)**: 전제는 도착할 때 요약에서 펼치는 둘째 상세 은하다. 구조의 선택과 착수 결정은 6a의 실측(틱 비용, 세이브 크기)을 가지고 §12.3으로 한다. 지금은 아무것도 예약하지 않는다(§16 #47).
이벤트 150+, 조우 100+, 특성 60, 업적 등급당 3~4(길별 T3→T4 변형 포함), 초월자 로스터(타락 교리 4행 전부 — 동화·광신·회귀 행 추가, 판당 활성 ≤ 3을 `NewGalaxy` 가중 추첨으로; 천상 사자 `trans_covenant` 1행(상태 인스턴스 없음); 독립 초월자 `trans_stillness`·`trans_release`; `arch_machine` T4 → 창발 동화는 동화가 없고 미봉인 수 < 3일 때만, 그때 `Transcendents`에 초월자 인스턴스 하나를 더하고 그 백성은 백성으로 남는다 — 기성 우주가 시뮬에서 자란다, §4.2), 엔딩(집단·단독 각 두 길, 다른 질서, 침묵의 우주; T5 문턱 `enc_threshold`, 받는 길 두 변형, 찬탈의 길 후원 카드 `enc_patron_*` 4장과 엔딩 변형 템플릿, §4.4), 영어 로컬.
아키타입 12행(EA 5행의 계시 템플릿 15줄 포함), 반란 결말 3종(쿠데타 `KillPlayer` 행/분열/분리. 새 문명을 만드는 것 — 분리, 바벨의 실제 분열, 바벨 연합의 '흩어짐' — 은 Phase 6a 이후이고 쓸 때 §12.3을 거쳐 다시 명세한다: 새 문명은 건넘 단계에서만 생긴다, §7.4), 길 3종 추가(해탈의 길 분별 보너스 포함), 이중 계 Hazard 6종(§4.1 EA 후보), 모티프 24종, 칙령(희년·안식·도피성·이삭 줍기), 헌정명 뱅크·`DT_Dedications`, `EpithetRow`, Nomad 덱, 첫 생애 아키타입 선택 UI(`arch_default`로 되돌리기, 첫 생애의 별 고르기 — 거기서도 태양계는 잠긴다, 성역의 법 §4.5 ①·§5.10), 기원 선택 카드(`InstallAt`), `Trait.Legacy.Delivered/Forsaken`(Forsaken은 Phase 6+ 행 작성 시 ini에 추가)과 2대 전용 조우, 종족 상한 컬럼, 통치자 암살 이벤트 행, 귀환 제안 `enc_recurrence_return` 실전(회귀 행과 함께), `event_zeal_sacrifice`, `enc_independent_question`, 노화 활성 판단(§16). 요약 단위 후보(§1.3, 각각 §12.3 '인물 3문' 재심사): `Notable.Councilor`·`Notable.Elder`(의회·회중의 얼굴, Stability 임계 파벌 청원이 만든다), 청원 Body의 `{Hearer}` 변형, AI 통치자 만남 때의 명명, 아키타입별 역할 호칭·규모 낱말(기계 '연산체', 군체 '입'), 규모 등급 변화 로그(Severity 1, 둘째 문장은 애도), 동화 영향권 안에서는 `NameNotable` 무시('너의 이름도 자리를 비워 두었다'의 메커닉 그림자, 가드 1줄). 인물의 별칭·칭호(`EpithetRow`를 사람에게)는 영구 금지(§8.3, §12.2). 각각 §12.3 재심사.
우주·층·앎 후보(결정 관문, 각각 §12.3 재심사): `DT_Bridges`로의 이전(§16 #51), 닿음(다리 건너의 `Hazard.Cult`, §16 #44), 항성 등급별 Hazard 가중과 팔을 따르는 배치(유리수 회전 — 모든 시드가 다시 바뀐다), 소유 문명 조건이 없는 '빈 하늘' 조우, 음절 이름 생성기·정착 이름·`NativeName`, '세계관과 출처' 화면의 현실 수치와 그 출처 줄(Phase 5 최소형 위에 — 성역의 문장·출처·라이선스 줄은 v0.8 단계 8에 먼저 든다).
대도감의 EA 후보(각각 §12.3 재심사): 아키타입 폭풍의 교향악단·잔불을 핥는 자들, 위기 미끼의 땅·변칙 개체(키메라)·먼 빛, 국경 배율(§16 #88·#89), 떠돌이 조우 덱의 그림.
EA 규모 300·48(§16 #3), 영겁 모드(스트레치). 봉인(T4 업적 '결박', §4.2)이 들어오면 `SealedTick`을 `bSealed`와 같은 자리에서 적는다(`TranscendentSealed` 조건의 SinceTick 쪽 — Phase 2 종료 리뷰의 재감사가 여기로 옮겼다).
Steam 페이지(포지셔닝 문구는 §13), '세계관과 출처' 화면의 완성형(최소형은 Phase 5 — v0.8 단계 8), 데모 빌드, EA 출시.

**현실 일정 요약**: 수직 슬라이스까지 약 6.5개월(Phase 4b 2주와 v0.7 순증 약 19~21 작업일 포함) — v0.8 순증 약 25~36 작업일(약 5~7주)을 더해 수직 슬라이스까지 약 8개월, EA까지 12~18개월. Phase 6a(건넘 최소형, 2~3주)는 EA 출시 빌드에 든다. 초안의 10주는 "Phase 1~4의 프로토타입 핵심 코드만" 기준으로 읽어야 한다.

**v0.7 순증 (새 Phase 없이)**: 새 Phase를 만들지 않는다. Phase 2 첫 헤더 전에 'v0.7 예약'(1.5~2일)을 두고, 생성·위기·이벤트 슬롯의 시뮬 추첨 지점이 바뀌는 일은 Phase 2 한 번에(+5~6일), 표시는 Phase 3 꼬리에(+5일), 청원·스캔·다이제스트·희소 관계는 Phase 4에(+1~1.5일), 단언과 닫는 줄은 Phase 4b에(+0.5일), 실측과 조정은 Phase 5에(+1일), 건넘은 Phase 6a(2~3주, EA 출시 빌드)에 둔다. 스키마 검사기는 Phase 2의 '첫 이벤트 데이터' 작업 전에 3작업일 시간 상자로 넣는다. 플레이어의 건넘은 Phase 6b 후보다. 콘텐츠 1.5~2일을 합쳐 수직 슬라이스 순증은 약 19~21 작업일이다. 정의 데이터(테이블·노브·태그)는 세이브에 남지 않으므로 처음 읽는 Phase에 넣고 행 이름만 지금 확정한다. 각 주에 플레이 가능한 상태를 유지한다(§13 번아웃 행).

| 구간 | v0.6 기간 | v0.7·v0.8 순증 | 비고 |
|---|---|---|---|
| v0.7 예약 (Phase 2 첫 헤더 전) | — | 1.5~2일 | 헤더 승인 절차와 리뷰 포함 |
| Phase 2 | 3주 | +5~6일, 스키마 검사기 +3일(시간 상자) | 생성·위기·이벤트 슬롯의 시뮬 추첨 지점은 전부 여기(분쟁·조우·계승은 Phase 4·4b) |
| Phase 3 | 4주 | 꼬리 +5일 | 7·12·8·9번은 손 확인을 기다리지 않는다(2026-10-18 사용자 — 11·7·12·8번 끝), 10번 종료 리뷰는 손 확인 뒤 |
| Phase 4 | 6주 | +1~1.5일 | |
| Phase 4b | 2주 | +0.5일 | |
| Phase 5 | 4주 | +1일 | |
| 콘텐츠 (Phase 2~4에 나뉜다) | — | +1.5~2일 | 금지어 확인과 사실 확인 대조 포함 |
| **수직 슬라이스 합계** | 22주(약 5.5개월) | **약 19~21 작업일** | **약 6.5개월** |
| v0.8 단계 1~3 (Phase 3 안) | — | 3~5.5일 | 문서 반영 1.5~2.5일, 시간 토글 0.5~1일, 꼬리의 v0.8 몫 1~2일 |
| v0.8 예약 (Phase 3과 4 사이) | — | 11~15.5일 | ① 성역과 첫 생애 행 3~4.5일(리뷰 포함), ② 실제 하늘 8~11일 |
| v0.8 Phase 4 몫 | — | 10~14일 | 서막 1.5~2주 + 외계 백성·고넬료 사슬 2~3.5일 + 재측정 |
| v0.8 Phase 5 몫 | — | 1일 | '세계관과 출처' 최소형 |
| **v0.8 순증 합계** | — | **약 25~36 작업일(5~7주)** | 저장 작업·서막 콘텐츠·Phase 3 꼬리 본래 몫은 따로 |
| Phase 6a (Phase 6+의 첫 블록, EA 출시 빌드에 포함) | — | 2~3주 + 콘텐츠 약 1일 | 건넘 최소형 |
| Phase 6b (6a 다음 블록) | — | 계획 후보 — 일정은 §12.3 심사 때 | 플레이어의 건넘과 둘째 상세 은하 |

---

## 12. 범위 관리: 수직 슬라이스와 미구현 목록

### 12.1 수직 슬라이스 정의 (Phase 5 종료 시점)

| 항목 | 수량/상태 |
|---|---|
| 무대 | 우리은하 1(상세, 고향 은하) + 이름 있는 은하 행 19(무대 4 — 안드로메다·삼각형자리·대마젤란·소마젤란 — 와 표시 전용 15 — M87과 Phase 3 9번 작업의 열넷. VS에서는 모두 보이기만 한다) + 풍경 은하 10,000점(그림). `DT_Galaxies` 20행 |
| 성계 | 60 (백성의 모성계 12 + 무주 생명 성계 48) + 성역 1(`sol` — 어느 백성의 소유도 되지 않는 태양계, §4.5. v0.8 — 단계 4부터). 단계 5부터 태양 근방 관측 목록의 계. 배경 별 20,000은 그림 |
| 백성(문명) | 12 (플레이어 포함, 모두 T1에서 시작). 첫 생애는 서막 뒤 첫 생애 행(가칭 `arch_seeker` — 성역 밖 찾는 백성, `sys_0`)의 T1 통치자(v0.8 — 단계 4b·6부터. 그 전의 코드는 `arch_terran`(지구, 태양계 `sol`)의 T1 통치자). 첫 생애 행은 플레이어의 첫 생애 백성을 만드는 `DT_Civs` 행, 성역 밖은 태양계 바깥(설계 낱말), 찾는 백성은 계시 양식이 '찾음'인 백성, 서막은 첫 틱 전의 카드다(§4.4). VS는 한 구역의 값이고 EA에서 행 값만 올린다(300·48) |
| 문명 아키타입 (`DT_Civs`) | 7 (첫 생애 행(가칭 `arch_seeker`) + `arch_default` + swarm·psionic·machine·covenant·merchant, §5.11). AI 백성 11은 `Weight` > 0인 6행의 가중 복원 추첨이고, 플레이어는 첫 생애 행(가칭 `arch_seeker`, `Weight` 0)이다(v0.8 — 단계 4b부터. 그 전의 데이터는 `arch_terran`) |
| 초월자 | 1 (침묵, 소속 `Realm.Abyssal`, `DT_Transcendents` 1행 `trans_silence`. 초기 코어는 첫 생애의 모성계(`sys_0` — v0.8 — 단계 4부터, 그 전은 `sol`)에서 3레인 이상 떨어진 AI 모성계 곁의 무주 성계. 활성 타락 상한 3과 가중 추첨은 EA) |
| 넷째 칸 이상의 존재 | VS 1체(침묵). 설계 값 3~7은 EA의 범위다(§4.2) |
| 보유 상한 / 보는 거리 / 첫 접촉 | 1/3/4/7/10(AI와 플레이어 같음) / 2/3/4/6/10레인(MVP ③ 전 1/2/4/6/10) / `FirstSightTick` 330(5:30. 첫 접촉은 늦어도 틱 340) |
| 도달 가능 등급 | T1 → T3 (양 경로) |
| Hazard | 8 (§4.1 표, `Hazard.Claim` 포함. 방문 `Hazard.Visitation`은 Phase 4에 별도 1종). 정착된 성계에만 선다. 아공간 침식이 VS에 드는 때는 §16 #90. 대도감(개발자의 `cosmic_species_codex.md` — 성역 밖의 세계관 원천, §5.11)의 얼굴(성식종·세월을 갉아먹는 아지랑이)은 문장일 뿐 위기 수를 늘리지 않는다 |
| 이벤트 | 40 (은혜 3·모티프 3·시대 명명·책 3 포함 — v0.8의 "말이 도착했다"(가칭 `event_word_arrived`)도 이 안이다) |
| 조우 | 25 (몰락한 군주 3~5, 길 1종(`Path.Covenant`) 5, 사자 3, 증언 `enc_witness` 1, 성역의 경계 '지경' 1, 레인 이동 조우 포함) + 청원 카드 10 (헤롯 패턴 청원 — 백성이 통치자를 높여 신이라 부르려 하는 시험, §5.10 — 포함. 의회·회중 2화자 변형은 9장, 첫 접촉 `petition_first_contact` 1 포함 — 이 카드는 변형 없음) = 35 (v0.8 — '지경'과 헤롯 패턴 청원은 Phase 4, §16 #93) |
| 업적 | 4 (T1→T2, T2→T3 각 2경로) |
| 거대 프로젝트 / 칙령 / 유물 | 7 (개혁·구호·정착 `project_settle` 포함) / 9 (철야·안식 최소형·낭독 포함) / 8 |
| 계시 양식 | VS 7행 × `RevelationTemplates` 3줄 = 21줄 + `DT_LogTemplates`의 계시 틀 이름 행(선택 행이 없을 때 호출자가 건네는 행, §8.2) (§5.11). v0.8 — 첫 생애 행의 3줄이 `arch_terran`의 것을 대신한다(수는 그대로) |
| 행성 | 정의 데이터 `DT_Planets`(VS의 계 약 61에 행성 약 120, 아는 성계에만 — v0.8 — 단계 5부터) |
| 서막 | 카드 약 8장(틱 밖, Phase 4 첫 블록 — v0.8 — 단계 6부터) |
| 대도감 | 원형 장부(종족마다 빌린 원형과 쓰지 않는 말을 적는 설계 문서의 표), 위기 얼굴·조우·소문·기록원 인용(성간 기록원의 기록 — 카드 안의 이름 없는 인용 목소리) 약 20줄(Phase 4), EA 후보는 §11 Phase 6+ |
| 인물 기록 (`FNotableRecord`, §7.2) | 문명당 ≤ 4(`MaxNotablesPerCiv`), 역할 6(`Notable.*`), 생성 훅 VS 6(데이터 행 4: 전조 ②(Rebel)·망명 충신 변형(Loyalist)·`crisis_visitation` 해결(Hearer)·책이 발견되었다(Hearer) + 코드 2: Regent·Ascendant — 반란 만료의 Rebel → Ruler는 역할 전환), 이름 뱅크 5행 × 12 = 60(`arch_swarm`·`arch_machine` 비움. v0.8 — 단계 7에 첫 생애 행의 외계 이름 12가 인간 이름 12를 대신한다). `NewGalaxy` 시드 0 |
| 규모 등급 (`GetPopulationBand`, §8.3) | 6(소수·수백만·수십억·수천억·수조·셀 수 없는), 저장 필드 0, 패널 1줄 + `{PopBand}` 템플릿 3~5 |
| 앎 | `Knowing`(아는 성계·만난 백성) 저장, 세계 해시로 불활성 확인(`Sim.KnowingInert`). 새 로그 행 5(`log_first_contact`·`log_contact`·`log_far_ascent`·`log_far_event`·`log_scan_wild`, 13줄), ST_CA `Digest.Elsewhere` |
| 새 테이블 / 태그 | `DT_Galaxies` 1 / 136 → 140(`Galaxy.Kind.*` 4) |
| 생애 전환 | 1회 이상 (사망 → 공백기 도약 → 계승이 콘솔 명령 없이 가능, Phase 4b). 연대기 생애별 장 + 두 책 장 |
| Niagara | 1 (NS_Ascension. 사망·영적 사건 전용 없음). 배경 별·풍경 은하는 Niagara가 아니다 |
| 저장/로드(판당 슬롯 1 — 이름은 `Galaxy_<GUID>` 그대로), 배속, 결정론 테스트 | 완료 |
| 시뮬 1틱 | 1 ms 미만(60·12) |
| 언어 | 한국어 |
| **VS가 전하지 않는 것** | 건넘(백성의 것도 플레이어의 것도 — 구조체와 빈 단계만 예약), 무대 은하의 요약, 백성 30~80, 넷째 칸 이상 3~7 |

### 12.2 명시적 미구현 목록 (Cut List)

아래는 **만들지 않는다**. 아이디어가 떠올라도 이 목록을 먼저 본다.

실시간 3D 함선 조작·전투. 방랑자 이동은 노드 간 이동이고, 분쟁은 수치 비교다.
행성 표면·식민지 건설 UI, 타일, 건물.
개별 유닛·함선 설계기.
수백 노드 기술 트리. 영역 4개 + 임계점 돌파만.
외계인·초월자 3D 캐릭터 모델. 일러스트 1장 + Niagara 오라.
멀티플레이어, 모딩 API(데이터 테이블 공개로 사실상 대체), 성우.
한국어·영어 외 언어 (EA 이후 판단).
영겁 모드(엔딩 후 샌드박스)는 EA 이후.
부름과 계승(§4.4): `USoulComponent`·`UCallingComponent` 등 Actor 결합 플레이어 상태. 행성·종족 엔티티와 DataAsset. 초월자 상태 FSM. 틱당 사망 확률·돌연사. 메타 프로그레션·스킬트리·판 간 메타 세이브. 사망 전용 Niagara·일러스트. 통치자 사망의 별도 메커니즘. 도약 중 수동 개입. 수동 저장·로드 메뉴(관조 모드 토글은 §16).
통치·이중 계·세력(§4.2, §4.5, §5.10, §5.11): `EPlayerRoleState`·`FPlanetaryGovernanceData`·Unrest/SpiritualFavor/영압 저장 필드·Dual-Realm Calculator 클래스·`UGalacticFactionDataAsset`·`EGalacticPhilosophy`. 천국·지옥·선계를 지도상의 영역으로. 영적·사후 영역의 두 번째 지도, 그리고 별도 레벨·별도 좌표계의 은하 사이 지도(우주 화면은 같은 지도의 줌 단계다 — 별도 상태가 아니다). 절대자 엔티티·화자·대화 NPC·'설득' 선택지. 영혼·기억 암시장, 영력 자원 바, 기도의 자원 지출, 신앙 게이지·개종 미터. 타락 교리 정의 5종 이상·판당 활성(미봉인) 타락 초월자 4체 이상(정의 4·활성 ≤ 3, §4.2) — 은하가 늘어도 같다. 영적 존재의 체력·전투·축귀 미니게임, 악마 유닛. 사망·영적 사건 전용 Niagara. 문명 내부 파벌 엔티티. 실존 교단·목회자·현대 설교자 이름. 길을 '종교 세력' 하나로 합치는 것. 인월드 성경 본문·장절 인용(§8.3).
영혼의 행방·두 영역·지구·계시(§3.2, §4.2, §4.4, §5.11, §8.3): 영혼 카운터·영혼 자원·구원/멸망 집계·비율·숨은 플래그·'구출한 영혼 수' 업적·영혼 해방 미션. 초월자별 `ClaimedSouls`·Harvest 스칼라(CSV 컬럼 포함). 개별 영혼·사후 세계 엔티티, 천국·지옥·음부의 내부 장면·방문 조우·심판 연출. 플레이어 윤회(회귀는 타락의 상품일 뿐), UI 표시명 '윤회'. 광신의 '부정 에너지' 바·추출 미니게임·고통 장면. `Realm.Heaven`·`Realm.Abyss` 태그, `EAscensionTier` T6·T0, 상태 구조체의 `Realm` 필드(소속은 정의 `FTranscendentDef.Allegiance`). `Realm.Celestial` 행의 영향권·주장·압력·상태 인스턴스. 시작 시 타락 4체 활성. 단일 '어둠의 군주' 엔티티·화자(공중의 권세 잡은 자를 인스턴스화하지 않는다). 길과 교리의 어휘 혼용, 해탈·무위를 타락의 속임수로 판정하는 문장. 지구의 현실 국가·종파·교단·도시·현대 인물 이름(천체의 통용 이름 — 별·은하·성좌 — 은 여기 들지 않는다), 지구 지표 지도·역사 이벤트, 지구 플롯 아머(v0.8: 성역의 법은 백성이 없는 성계 하나만 지킨다 — 첫 생애의 백성도 어느 백성도 플롯 아머가 없다, §4.5). 책 본문의 인월드 표시·인용·낭독 본문, 책의 Relic·연구·자원·능력치화, UI·서술자의 '성경' 낱말. 성육신의 인월드 역사 서술. 계승 화면의 같은 이름 재사용. 둘째 사망·불못을 집행하는 효과·엔딩. 회귀의 유혹 전용 화면·Niagara·일러스트. 독립 초월자의 Log 1인칭. 성계 단위 인구(`FStarSystemState.Population`). 우주론·종교 요소 토글('세속 모드'). 성역을 세력·영토·관할·순례지로 쓰는 것, 은하 관할 분류(모든 은하에 같은 한 줄만), '옛 땅의 기억'(말이 도착한 백성에게 열리는 한 쪽짜리 화면, §5.11)을 연표나 사건으로 쓰는 것, 플레이어 칭호(구원자·메시아·선택받은 자·빛의 아들·기름 받은 자), 서막의 칸·경험치·사망·1인칭·추첨, 그릇의 불사('날이 더해짐'은 불사가 아니다), 부르심의 승천 오버레이(v0.8).
요약 단위(§1.3, §7.2, §8.3): 인물의 능력치·특성·성품·초상·외모·나이·수명·호감·충성·관계·혼인·혈연·가계도·왕조·계승 서열. 인물 AI·의사결정·에이전트 틱. 인물 사망 롤·노화·인물 전용 `KillPlayer`류 효과. 인물 전용 이벤트·조우·퀘스트('X를 찾아라')·모집·영입·고용·장군/과학자/행정관 슬롯·인물이 수치에 주는 보너스. 인물과의 대화 NPC·호감 선택지. 인물 목록 패널·도감·초상 UI(표시는 로그 문장·카드 안의 이름·연대기 문장·다이제스트 1줄만). `FNotableRecord`의 6번째 필드(§12.3 '인물 3문' 통과 전). 플레이어 그릇을 `Notables`에 넣는 것. 사자·초월자·저자·교단 지도자를 인물로. 인물의 별칭·칭호·'대왕' 접미어(`EpithetRow`는 성계·시대만). 인물 간 상호작용, 전역 인물 ID, 인물 FText 저장. 성계 단위 인구(재확인)·도시·개별 함선·명 단위 인구 표시·인구 정수 저장·Population 수치의 UI·로그·연대기 표시(규모 등급만)·'남은 자'를 규모 등급 라벨로.
우주·층·앎(§3.1, §5.7, §5.9, §5.11, §5.12, §7.2, §8.1): 풍경 은하의 시뮬·역사 전개·지연 전개·방문·선택 패널(풍경은 그림이다). 상세로 도는 은하를 둘 이상 두는 것과 은하별 난수 스트림·은하별 시계·은하별 저장 슬롯·은하별 연대기(영구 금지가 아니다 — Phase 6b의 §12.3 심사 전까지). 별도 레벨·별도 좌표계의 은하 사이 지도, 은하 사이 항해·비행 장면(우주 화면은 같은 지도의 줌 단계다). 다리를 레인으로 저장하는 것, 레인 규칙이 은하를 넘는 것, 다리 머리 성계·다리 위의 전투나 봉쇄. 다리 위의 함선·함대 유닛, 건너는 배의 내부 관리, 항해 조작, 건넘 전용 연출·Niagara·일러스트, 건넘 중 사망·사고 롤·조우. VS와 6a의 플레이어 은하 건넘(6b 후보, §16 #47), 통치자 자신의 건넘, 계승이 은하를 넘는 것. 요약 은하의 T5와 거기서 서는 엔딩, 요약 은하에 저장 상태를 두는 것. 은하별 타락 로스터, 같은 교리의 은하별 복제, 은하 위의 상위 타락, 다섯째 교리, 타락이 건너는 새 편입 경로(주장·교단 밖), 은하별 영향권 집계 숫자·주장된 비율. 은하·은하단·은하 종류를 등급이나 사다리의 칸으로 삼는 것, 은하를 소속(Realm)·길·신앙 특성에 배정하는 것(선한 은하·타락한 은하, 천국·지옥·선계 은하). 다리를 '문'·'관문'·'길'로 부르는 것, 넷째 칸의 제동을 '자리'로 부르는 것, 어느 은하를 보좌·무저갱·'그 나라'의 자리로 읽히게 하는 것. 넷째 칸의 제동(`AIFourthRungCap`)을 플레이어의 업적 판정에 거는 것, 승천 정원제. 행성 노드·행성 엔티티, 괴수·원시 생명 개체와 생태 시뮬레이션, 무주 성계의 인구·생명 수치·채굴 미세 관리, 무주 성계의 Hazard·주장(초기 코어 밖의 무주 성계 편입). 정착 전용 화면·식민지 건설 UI(정착은 거대 프로젝트 행 하나다). 안개 타일, 탐사율·발견도 미터, '몇 개 중 몇 개' 표시, 모르는 성계를 셀 수 있는 점으로 따로 그리는 것, 정찰 유닛·관측소. 앎을 읽는 조건·효과(`EConditionType`에 앎 값 없음), 앎에 따라 시뮬 스트림을 뽑는 로그 호출, 앎의 판 간 이월, 생애마다 앎을 지우는 것, 문명별 앎(AI의 시야). 런타임 상태의 범용 사실 저장소·간선 저장소, 세계를 간선 테이블이나 트리플 저장소로 두는 것, 관계 종류를 데이터 행으로 두는 것, 믿음의 강도, 소문 시스템, 도감(코덱스) 패널. 첫 접촉이 만드는 인물 기록·도감·초상, 초월자와 교단 지도자에게 이름을 주는 것. 서술자 로그·연대기·두 책·조우 문장의 별의 수·광년·은하 수, UI의 생명 성계·백성·아는 것의 수 숫자, 드레이크 추정의 인월드 표시와 설정 화면, 별의 수 라벨과 '백성' 줄을 한 패널에 두는 것. 천체 이름에 든 인물을 인물로 호명하거나 인물 이름 뱅크에 쓰는 것, 지구의 실제 역사 연대표·지표. 책과 신앙 특성이 건넘을 따라 다른 은하로 가는 것, 성역 밖의 인간 백성·인간 이름 뱅크(둘째 `arch_terran` 금지를 넓힌다), 통치자의 보냄을 선교로 쓰는 것. 이름 음절 생성기(저작 뱅크 `PeopleStems`로 대신), 정착 때 성계 이름 다시 짓기, 숫자가 든 카탈로그 코드의 로그 슬롯. 항성 등급으로 초신성 전조를 막는 규칙(O·B 한정). 시작 은하·시작 규모를 고르는 선택지, 은하마다 시드를 입력받는 화면. 행성 노드·행성 상태·행성 명령(행성은 정의 데이터의 목록만), 성역을 시뮬하는 백성 행, 성역에 서는 그릇, 생태 시뮬·괴수 개체·키메라 개체, 국경 배율(VS), 다른 작품의 고유명과 신화의 신 이름, 허구 행성(잿빛 왕관의 넷 포함), 서기 연도·창조 연대·종말 연표, 별에 거는 기다림이나 순례(v0.8).

### 12.3 범위 추가 절차

새 기능을 넣고 싶으면 다음 세 질문에 모두 "예"여야 한다.

DataTable 행 추가로 구현되는가? (코드 변경이 필요하면 다음 Phase로)
두 모드 모두에서 소비되는가?
위기·업적·로그 중 하나와 연결되는가?

v0.5 채택분(사다리 상한·교리 4종·영혼 경제·귀환 제안·계시 양식·지구·두 책)의 통과 기록: Q1 — 교리·계시·지구·유혹·두 책은 DataTable 행으로 구현되므로 예. `Hazard.Claim` 발생 규칙과 로스터 추첨·템플릿 선택기·`OfferReturn()` 훅은 코드(약 200줄)라 '다음 Phase' 규칙대로 Phase 2·4b에 배정하고 Phase 1에는 헤더 예약만 둔다(§9.1, §11). Q2 — 통치자(조공·구호·철야·의회와 회중의 청원·성소·낭독)와 방랑자(유혹·후원·증언·독립자 조우·대면) 모두 소비하므로 예. Q3 — 주장은 Hazard 결과, 영향권 크기는 Hazard 가중치, 두 길은 Feat, 흔적은 연대기이므로 예.

v0.6 채택분(요약 단위 원칙·인물 기록·규모 등급)의 통과 기록: Q1 — 인물 생성은 `NameNotable` 효과를 실은 DataTable 행과 `PersonNameBank` 컬럼·ST_CA 라벨·폴백이므로 예. `FNotableRecord` 헤더·`Apply` 가드·코드 훅 3·슬롯 해석·`GetPopulationBand`(합계 약 100줄)는 코드라 '다음 Phase' 규칙대로 Phase 1 헤더 예약 + Phase 2·4 코드다(§9.1, §11). Q2 — 통치자(반란 청원의 `{Rebel}`, 정권 교체·섭정 로그, 패널 '백성' 1줄)와 방랑자(망명·재회 변형의 `{Loyalist}`, 취임 다이제스트의 `{Ruler}`) 모두 소비하므로 예. Q3 — 인물은 위기(반란·방문)와 업적(AI 문명 T3)의 부산물이고 쓰임은 로그·연대기이므로 예.

v0.7 채택분(은하와 층·앎·건넘·현실 수치)의 통과 기록: (가) 은하와 층(무대·풍경, 생명 성계·무주 성계, 보유 상한, 넷째 칸의 제동, 항성 분포): Q1 — 은하와 층의 수, 현실 수치 문구, 이름 풀, 상한과 분포는 `DT_Galaxies` 행과 `DT_Rules` 노브이므로 예. 은하 생성 규칙 변경(약 120줄)·정착된 성계만 Hazard·문명 배열 정리·은하 패널·우주 화면은 코드라 '다음 Phase' 규칙대로 저장 필드만 지금 예약하고 Phase 2·3에 둔다. 정착은 거대 프로젝트 행 `project_settle` 하나와 `TickCivAI`의 한 줄이다. Q2 — 통치자는 보유 상한 안에서 무주 성계에 정착하고 풍부도와 항성 등급을 수입으로 읽으며, 방랑자는 무주 성계를 지나고 스캔한다(`log_scan_wild`). 은하 패널과 생명 낱말은 두 모드가 같이 본다. 예. Q3 — 층은 Hazard 추첨의 범위(정착된 성계)와 주장의 조건(둘째 정착된 성계), AI 승천 판정(`AIFourthRungCap`)에 닿고 표시는 로그와 패널이다. 예. (나) 앎(보는 거리, 만남과 첫 접촉, 이야기의 범위): Q1 — 보는 거리 값·만남의 줄·먼 줄·청원은 노브와 행이므로 예. `TickSight`(약 60줄)·이야기의 범위(약 40줄)·세계 해시·표시 필터는 코드라 필드와 말하기 스트림만 지금 예약하고 Phase 2·3·4에 둔다. Q2 — 통치자는 소유 성계에서 보고 그 판의 첫 만남에 청원(`petition_first_contact`)을 받으며, 방랑자는 선 성계에서 보고 스캔으로 넓힌다 — 만남에서 방랑자의 몫은 줄과 스캔이다. 같은 `Knowing`을 두 모드가 쓰고 부름과 함께 넘어간다. 예. Q3 — 만남은 로그 줄이고 첫 접촉은 청원(태세 선택)이며, 범위는 어느 위기·업적 줄이 말해지는가를 정하고, 공백기 다이제스트와 Interlude의 '접촉'이 앎으로 정의된다. 위기·업적 조건은 앎을 읽지 않는다(세계 해시 테스트). 예. (다) 건넘: Q1 — `project_crossing`과 건넘 로그·`PeopleStems`는 행이므로 예. `TickCrossings`의 도착·출발과 `SummaryAt`(약 250줄)은 코드라 Phase 6a이고 지금은 `FTransitRecord`·`Transits`·`NextCivSerial` 예약뿐이다. Q2 — 통치자는 프로젝트로 보내고, 두 모드 모두 도착한 백성을 상대한다(외교 대상, 방문·조우 대상). 다만 VS에서는 어느 모드도 소비하지 못한다(도달 T3)는 점을 그대로 적는다 — 그래서 VS에 넣지 않았고, 6a에서 일정을 잡을 때 이 물음을 다시 묻는다. Q3 — 수단이 거대 프로젝트이고 결과가 로그이며 도착한 백성은 Hazard의 대상이 되고 경쟁 승천의 시계에 든다(`CrossingSettleTicks`). 예. (라) 현실 수치: Q1 — 전부 데이터(`DT_Galaxies` 문구, ST_CA, 노브 배열 1개)이므로 예. Q2 — 은하 패널은 두 모드 공용. 예. Q3 — 항성 등급이 Energy 수입에 닿고 초신성 문장이 `Hazard.PreSupernova`의 서술에 닿는다. 예. 인물 3문: 채택분 어느 것도 `FNotableRecord`를 건드리지 않는다 — 건너온 T4 백성은 인물 0, 만남은 인물을 만들지 않으며, 6번째 필드 요청이 없다(출신은 인물이 아니라 문명의 필드 `OriginGalaxyRow`다). 정의 스키마(`Tools/data/schema.json`)와 검사기는 게임 기능이 아니라 저작 도구라 세 질문의 대상이 아니다(런타임 구조체·필드·해시·세이브 변화 0). 다만 그 규칙 종류를 늘리는 일은 이 절차를 거친다. 통과 기록에 넣지 않는 것(채택하지 않음): 풍경 은하의 지연 전개, 은하별 로스터, 레인으로서의 다리, 세계 상태로서의 앎, 닿음, 상태 있는 요약, 정착 명명, 음절 이름 생성기, 런타임의 범용 사실·간선 저장소. 플레이어의 건넘과 둘째 상세 은하는 Phase 6b의 계획 후보이고 그때 이 절차를 거친다(지금은 예약 없음).

v0.8(성역·첫 생애·서막·실제 하늘·대도감)의 통과 기록: Q1 — 성역·관측 목록·서막 카드·'옛 땅의 기억'·대도감의 문장은 행과 데이터이고, 술어·도구·생성·연출·부르심의 Core 함수는 코드라 블록('v0.8 예약'·Phase 4)에 배정한다. Q2 — 서막은 두 모드의 첫 생애에 앞서고, 성역은 통치자의 정착과 방랑자의 방문이 모두 읽는다. Q3 — 정착 사유는 업적으로 가는 길을 보이고, 서막의 선택은 연대기의 첫 장이다. 대도감은 새 시스템이 아니다 — 행·문장·원형 장부다. 키메라·국경 배율·대도감의 EA 아키타입은 각각 이 절차를 다시 거친다.

**부칙 — 인물 3문**: 인물 기록(§7.2)을 건드리는 모든 기능에는 위 세 질문에 더해 아래 세 질문을 묻고, 하나라도 '예'면 거부한다. ① 인물에게 숫자를 주는가(능력치·확률 수정·가중치·보너스)? ② 인물 없이 같은 문장이 성립하지 않는가(기능이 인물에 의존하는가)? — 성립하면 인물은 슬롯일 뿐이라 통과, 성립하지 않으면 거부. ③ 인물이 다른 인물을 참조하는가(관계·왕조의 씨앗)? `FNotableRecord`의 6번째 필드는 이 세 질문을 전부 통과해야 한다(§7.2, §12.2). '능력치가 필요하다'는 요청은 그 자체로 영웅 시스템이라 기각 사유다(§13).

---

## 13. 리스크와 대응

| 리스크 | 징후 | 대응 |
|---|---|---|
| 범위 폭발 (4X + RPG) | Phase 종료 조건이 두 번 연속 미달 | §12 Cut List 재확인, 수직 슬라이스 항목 수 축소. 기능 추가는 §12.3 절차 |
| '스프레드시트 감상' 느낌 | 테스터가 로그를 읽지 않음 | 템플릿 변형 수 증가, Severity 2 빈도 조정, 자동 일시정지로 결정 유도, 승천 연출 강화 |
| 두 모드가 콘텐츠 비용을 2배로 | 조우와 이벤트가 따로 늘어남 | 같은 Hazard를 양 모드가 해결하는 구조 유지. 한쪽에만 있는 콘텐츠는 보류 |
| UE5 C++ 반복 속도 | 헤더 수정 → 전체 재빌드 반복 | 헤더 안정화 후 데이터로 작업. Live Coding은 .cpp만. 로직을 DataTable 쪽으로 밀기 |
| 저장/로드 후반 도입 | 상태가 Actor·위젯에 흩어짐 | Phase 1에서 왕복 테스트 통과를 종료 조건으로 고정 (이미 반영) |
| 밸런스 불가 (1인) | 플레이 1판 5시간씩 소요 | 헤드리스 커맨드렛 + CSV. 페이싱은 그래프로 맞춘다 |
| 텍스트 품질 편차 | 후반 이벤트 문장이 초반보다 성의 없음 | §8.3 작성 규칙 체크리스트. 10개 쓰고 다시 읽고 3개 버리기 |
| 번아웃 | 2주 이상 커밋 없음 | Phase를 더 작은 종료 조건으로 쪼갠다. 매주 '플레이 가능한 상태' 유지 |
| 하드 리셋 이탈 | 테스터가 2번째 사망 후 종료 | 공백기 다이제스트·세계 기억(Reverence ×0.5, `bAwareOfPlayer`)·사망 경고(치명 선택지 서술) 강화. 출생 보호 300틱과 사인 추적률 ≥ 90% 확인(§9.3). 그래도 안 되면 '관조 모드' 토글(§16) |
| 설교조 | 테스터가 "로그가 설교처럼 느껴졌다". 리뷰 폭격 양방향(선교로 느껴지면 비종교 유저, 가볍게 다루면 교회 커뮤니티) | §8.3 신성 표현 규칙 5줄 + 사후 문장 규칙 ①~⑩, 결과와 기록만. 포지셔닝 문구 '성경과 신화의 모티프에서 영감을 받은 관조적 SF 4X·승천 RPG'(Christian game 미선언, 미은폐) + Steam·게임 밖 선언 2문장 "신앙은 능력치가 아니며 게임은 어떤 종교의 진위도 판정하지 않습니다. 타락한 존재들의 주장은 그들의 주장입니다." + "당신은 아직 그 책을 만나지 못한 먼 별의 한 사람으로 시작한다." 그분의 이름은 스토어에 쓰지 않는다(이름은 개발자의 목소리인 '세계관과 출처' 화면에만, §4.5). 게임 밖 '세계관과 출처' 화면 1장(게임 메뉴에서 여는 개발자의 목소리 — 세계 안의 문장이 아니다. 어떤 본문을 어떤 방식으로 썼는지, 게임은 신학적 판정을 하지 않는다는 선언 + "이 게임은 누구의 영혼이 어디로 갔는지 말하지 않습니다. 그렇게 말하는 자가 있다면 그는 게임 속 인물이고, 그 말은 그의 주장입니다. 책은 펴지지 않습니다."). 한국어 FAQ 1문단("전도용인가요? — 아니요"). EA 포럼 고정 글 '세계관과 신앙 노트' 1편. 원칙은 '미선언·미은폐': 이름 없는 제단과 지켜지는 별이 첫 10분에 보이고 책이 나중에 오는데 스토어가 침묵하면 은폐로 읽히고, 'Christian game' 선언은 양쪽 폭격을 동시에 부른다. 우주론·종교 요소 토글(세속 모드)은 만들지 않는다 — 요소를 부끄러워한다는 신호이자 테스트 분기 비용이며, Tier 게이트(T1~2 징조만, T3부터 주장 표식, 유혹은 회귀가 있을 때만)가 이미 옵트인이다. 앎(보는 거리)은 영적 요소를 숨기는 토글이 아니다 — 격의 게이트와 독립이다(§3.2). 플랫폼 등급은 종교를 요소로 삼지 않는다 |
| 영혼 카운터 오독 | 테스터가 주장 로그·영향권·두 책 장을 "몇 명이 구원받고 멸망했는지"로 읽음. 리뷰에 '지옥 시뮬레이터' | 저장 스칼라 0·CSV 컬럼은 성계 수만(§4.2 영혼 경제). 숫자 린트("그 외에도 있었다", §8.4)와 `Data.NoVerdictWords`(§9.3). 서술자의 유일한 사후 문장 '기록되었다', 타락의 '내 것' 직후 서술자 침묵(§8.3). 테스터 질문 "게임이 누가 구원받는지 말했다고 느꼈는가" ≤ 1/10 |
| 종교적 타자 악역화 | 선계·불계·사이오닉이 '속은 자들'로 읽힘, 또는 회귀 교리가 특정 종교의 캐리커처로 읽힘 | 어휘 분리 린트 `Data.PathDoctrineVocab`(§4.5), 교리 비판은 길의 입으로만, 게임은 어느 응답이 받아들여졌는지 판정하지 않음(§4.5 원칙). 인월드 표시명은 '회귀'이고 '윤회'는 해탈의 길 텍스트에만. 해탈의 길은 회귀에 대한 분별 보너스를 받는다(통찰이 실제로 유용 = 존중). 종교·국가·도시·교단·인물·천사의 실존 고유명사 금지 유지(천체의 통용 이름은 여기 들지 않는다. 이름에 든 인물은 호명하지 않고 `PersonNameBank`에 쓰지 않는다) |
| 무력감 | 테스터가 "상위 존재 앞에서 작아진 게 아니라 무력했다" | 힘은 항상 무언가를 산다(시간·피난·기록, §5.6). 상위 존재는 숫자로 묘사되지 않는다. 영압 자체가 업적 기회(§4.2). AI 문명도 같은 날씨를 맞는다. 플레이어 성계 영압은 1,800틱 이후를 목표로 한다(초기 코어의 자리 규칙 §5.7, CSV 확인 §9.3) |
| 영웅 시스템 표류 | 인물 기록(§7.2)에 필드를 더하자는 요청(능력치·초상·호감·관계·수명), 인물 패널·도감 요청, 테스터 과반이 "그 사람을 더 알고 싶다". 영웅 시스템은 기능이 아니라 필드 하나씩 들어온다 — 능력치 1개 → 초상 → 호감 → 관계 → 왕조 | `FNotableRecord` 5필드 고정(6번째 필드는 §12.3 '인물 3문' 통과), §12.2 '요약 단위' 금지 단락, 슬롯 폴백으로 "모든 문장은 인물 없이도 성립"(§8.3). 테스터 질문 "이름 있는 인물을 한 명 기억하는가"의 정답은 '한 명'까지 — 여운이지 요구가 아니다(§9.3) |
| 규모 표류 (은하·성계·백성 수를 늘리자는 요청, 둘째 상세 은하를 앞당기자는 요청) | Phase 종료 조건이 두 번 연속 미달, 또는 `DT_Galaxies` 행 값이 아닌 코드 상수로 크기를 늘리는 변경이 나온다 | 값은 `DT_Galaxies` 행에서만 바꾸고 상세 은하는 하나다. 자르는 순서: 표시 전용 은하 행 열넷(9번 작업의 데이터 — 우주 화면보다 먼저) → 우주 화면 → 이름 있는 은하 행 5 → EA 규모 300을 200으로(앞의 둘은 버리는 것이 아니라 미루는 것이다 — 우주 화면은 Phase 5로, 이름 있는 은하 행 5는 Phase 6a로. 은하 행 5를 미루기 전에는 개발자에게 묻는다). 둘째 상세 은하는 Phase 6b의 §12.3 심사에서만 |
| 빈 지도 (앎 때문에 초반이 비어 보인다) | 첫 600틱에 말해진 줄 < 25, 첫 10분 이탈 ≥ 10%, 테스터가 '지도가 비어 보였다'고 답한다, CSV `FirstContactTick` > 340(또는 빈 값) | `FirstSightTick`·`SightLanesByTier[0]`·청원 밀도를 조정한다. T1의 빈자리는 청원 카드가 채운다(§2.2). 그래도 안 되면 2단 만남(먼저 이름 없는 '먼 등불'로 보고 뒤에 만나는 안)을 §12.3으로 재심사 |
| 앎의 누수 (보이는 것이 일어나는 것을 바꾼다) | `Sim.KnowingInert`·`Sim.TellStreamSeparate` 실패, 앎을 읽는 조건이나 '만났는가'를 묻는 카드 조건 요청, 로그 호출이 `GetRng()`를 쓴다 | 요청을 거부한다(앎을 읽는 것은 이야기의 범위·표시·명령의 전제·첫 접촉 카드 넷뿐이고, 세계는 `NoteMet`·`NoteKnown`으로 쓰기만 한다). `EConditionType`에 앎 값을 두지 않고 세계 해시 테스트를 Phase 2 종료 조건에 묶는다 |
| 요약과 상세의 불일치 (6a 이후) | 커맨드렛 비교에서 `SummaryAt`의 칸별 분포가 같은 크기의 상세 판과 크게 다르다, 건너온 백성의 수가 목표(한 판 1회 안팎)를 벗어난다 | 임계 틱을 고향 은하의 CSV 분포에서 다시 딴다. 화면에는 낱말만 낸다(수 없음). 둘째 상세 은하를 만들 때 불연속을 다시 본다 |
| 현실 수치의 오기·과신과 백과사전 톤 | 사실 확인과 다른 숫자가 문구에 들어간다(`Data.Integrity`의 '약·추정' 검사 실패), 테스터가 은하 패널을 '위키 같다'고 한다, 설계 값(백성 30~80)을 현실 추정으로 읽는 리뷰 | 문구는 §3.4의 안전 문안만 쓰고 정밀값은 인쇄 전에 다시 확인한다. 문구를 줄여 게임 밖 '세계관과 출처' 화면으로 옮긴다. '설계 값이다' 한 줄(§3.1, §3.4)을 유지한다 |
| 이름을 외우지 못함 (개발자 자신의 걱정) | CSV `MetCivs`(`ID`가 어느 생애 기록의 `BirthCivID`와 같은 항목을 빼고 센다, §7.4)가 VS 첫 생애에 6을 넘는다(EA는 한 생애에 12), 테스터가 '처음 만난 백성을 기억하는가'에 답하지 못한다, 같은 아키타입의 백성을 구별하지 못한다 | 보는 거리를 줄인다(`SightLanesByTier`). 색은 만난 순서로만 준다. 목소리의 구별은 절차 특성(§16 #36 ②)과 형용사 뱅크로 — 백성마다 필드를 더하지 않는다 |
| 영혼 경제가 닿지 않는다 (백성이 한 성계로 시작하고 무주 성계가 지도의 80%다) | CSV `FirstClaimTick`이 늦거나 비어 있다, 18,000틱의 영향권이 초기 코어 그대로다, 판당 플레이어 성계에 선 주장이 0에 가깝다, 플레이어 성계의 P가 처음 0을 넘는 틱이 판 끝에 가깝다 | 순서대로: `FallenCoreMinLanesFromHome`을 3에서 2로 → 주장 인접을 무주 성계를 건너 센다 → 둘째 코어. Phase 4 검사 '스크립트 통치자 아래 플레이어 소유 성계에 주장이 선 시드의 비율'로 확인한다(§9.3) |
| 신학 오독 — 성역(태양계 `sol` — 어느 백성의 소유도 되지 않는 다시 쓸 수 없는 법의 자리)이 '그분의 영토'로, 성역 밖(태양계 바깥 — 설계 낱말)이 '버려진 땅'으로, 부르심이 '신이 되기'로 읽힘 | 테스터 질문 두 개(§9.3)의 합격선을 넘음, 리뷰에 '이원론'·'신이 되는 게임' | 성역은 법이지 세력이 아니다(§4.5 ①, '성역 밖' 문단), 모든 은하의 한 줄, 받은 꼴의 낱말과 칭호 금지(§8.3), 성역의 별을 다른 별과 맞세우지 않는다 |
| 다른 작품의 IP — 대도감(개발자의 `cosmic_species_codex.md`)의 원문 이름(스타크래프트·40K·매스 이펙트·『프로젝트 헤일메리』와 겹치는 이름·그림) | 원문 이름이 게임 안 예문이나 데이터에 들어감 | 원형 장부(종족마다 빌린 원형과 쓰지 않는 말을 적는 표)와 이름의 거름(§5.11), 스키마 낱말 목록(단계 4), 원형만 빌린다 |
| 자료 라이선스 | 관측 목록의 출처·조건이 불명확함 | `Tools/data/sources/` README, HYG·Gaia DR3 직접은 보류, 감사 문구는 '세계관과 출처', Steam 출시 전 다시 확인 |
| 근방 하늘의 단조로움 — M형 위주의 경제, 판마다 같은 지도 | 에너지 격차, '지도가 늘 같다'는 반응 | 에너지 정규화(수치 이점 0), 시드가 AI 백성과 그 모성계·사건을 바꾼다(첫 생애의 별은 고정), 밸런스의 흩어짐은 백성·사건으로 잰다 |
| 서막(첫 틱 전의 카드)의 일자형 표류 — 카드가 늘거나 분기가 커짐 | 서막 카드 > 10장, 첫 10분 이탈 ≥ 10% | 카드 8장 고정·추첨 없음·모든 갈래가 머묾으로, 둘째 판부터 짧게, 이탈이 넘으면 다섯 장으로 |
| 해 표시를 두고 나올 물리 지적 | '1년에 몇 광년을 가느냐' 같은 지적 | 기본 '주기', '년'은 첫 기록부터 센 표시 단위, 실제 하늘(태양 근방 관측 목록의 지도) 뒤 배는 빛보다 느리고 다리만 빠르다(넷째 칸의 일), 수치는 '약'·'추정' |

---
