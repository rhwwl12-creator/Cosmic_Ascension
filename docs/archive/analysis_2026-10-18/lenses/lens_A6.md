# A6 — 문서↔코드 드리프트·범위·실현 가능성 (회의적 감사)

**한 줄 결론**: 이 프로젝트의 병목은 "설정값이 없다"가 아니라 **설계 1.1MB가 코드 13K줄·데이터 42행을 수십 배 앞질러 달린 것**이다. 결정론·데이터 파이프라인·테스트(76종 등록 확인)는 1인 개발 기준으로 드물게 단단하지만, 플레이어가 내릴 결정은 아직 "프로젝트 시작·정착·배속" 셋뿐이고, 문서는 그 위에 성역·서막·실제 하늘·건넘·두 책까지 "v0.8 — 단계 N부터" 표지 162개를 얹어 두었다(`guideline.md` grep 결과).

## (a) 드리프트 표 — 문서가 말하는 것 vs 저장소에 있는 것

| # | 문서가 말하는 것 | 저장소의 실제 | 종류 |
|---|---|---|---|
| 1 | 아키타입 `DT_Civs` 11행(VS 6+EA 5) / VS 7행 (`guideline.md:26`, `:1272`, `:4039`) | 2행 `arch_terran`·`arch_default` (`Tools/data/DT_Civs.json`) | 수량 |
| 2 | Hazard 8종 (`guideline.md:4044`, §4.1 표) | 행 5 (`DT_Hazards.json`), ini 태그 9 (`Config/DefaultGameplayTags.ini`) — Cult·Extinction·SubspaceErosion·Visitation 행 없음 | 수량 |
| 3 | 업적 4(T1→T3 양 경로) (`guideline.md:4047`) | `DT_Feats.json` 2행, 스키마가 `Path=Ruler`만 허용 (`schema.json:538-545`) | 수량 |
| 4 | 조우 25 + 청원 10 = 35 (`guideline.md:4046`) | 0 — `DT_Encounters`는 `columns: null`, JSON·에셋 없음 (`schema.json:591`) | 수량 |
| 5 | 첫 생애 행 `arch_seeker`, 모성계 `sys_0`, 성역 `sol` (`guideline.md:407`, `CLAUDE.md:23`) | `DT_Rules.json:28 PlayerArchetypeRow="arch_terran"`, `sol`이 플레이어 모성계 (`Sim/GalaxySimulationSubsystem.cpp:45`) | 표지 있는 예정 |
| 6 | 다섯째 법 `Law.Hedge` (`guideline.md:946`, `:2703`) | ini에 `Law.*` 4개, `DT_Rules.json:26` 4개; `ImmutableLaws`는 코드가 읽지도 않음 | 표지 있는 예정 |
| 7 | 표시명 '항성 안정화' (`guideline.md:1076`, `CLAUDE.md:48`) | `DT_Megaprojects.json:52`·`DT_Feats.json:26` '초신성 안정화' | 데이터 |
| 8 | 첫 줄 "{HomeSystem}. 이 백성이 당신에게 맡겨졌다." (`guideline.md:340`) | `DT_LogTemplates.json:8` "당신은 이 행성의 통치자다." | 데이터 |
| 9 | GameplayTag 136개 (`CLAUDE.md:248`) | ini `+GameplayTagList` 140줄 | 낡은 수치 |
| 10 | `CosmicSeed` 용도 상수 8 (`CLAUDE.md:254`) | `Core/CosmicSeed.h:24-32` 9개(EventSlot=9) | 낡은 수치 |
| 11 | "인자 `-TellAll`은 Phase 5" (`guideline.md:3135`) | 커맨드렛이 이미 파싱 (`Tests/CosmicBalanceCommandlet.cpp:231`), `guideline.md:3277`도 그렇게 적음 | 문서 내부 모순 |
| 12 | '아직 없는 것'에 승천 연출(Phase 3) (`CLAUDE.md:262`) | 승천 오버레이 구현·테스트 `UI.AscensionOverlay` 존재 | 낡은 목록 |
| 13 | AI 각성 만료 "생산 +150% / 성계 반란" (`guideline.md:625`) | `DT_Hazards.json:11-36` Energy/Matter +30 또는 Stability −0.15 | 표 미갱신 |
| 14 | 역병 "인접 전파" (`guideline.md:626`) | `OnExpireNeighbourEffects: []` | 표 미갱신 |
| 15 | §5.5 외교·§5.6 분쟁을 현재형으로 서술 | `TickDiplomacy`/`TickConflict`가 주석 한 줄짜리 빈 함수 (`Sim/GalaxySimulationSubsystem.cpp:1585-1593`); Fleet·Succession·Leap 노브 18개 미사용 | 현재형 서술의 미구현 |
| 16 | EA 생명 성계 300 (`guideline.md:397`) | 같은 절 `MaxSystems` 150 액터 예산 (`:399`) — 해결책 없음 | 수치 긴장 |
| 17 | 주장 만료 18% (`CLAUDE.md:259`) | 같은 파일 "20%만 만료" (`CLAUDE.md:262`) | 기준값 불일치 |

요약: 17건 중 "표지 달린 예정"이 5건, **표지 없이 현재형으로 적힌 미구현·낡은 수치가 12건**이다. 문서가 스스로 '지금'과 '목표'를 가르지 못하는 것이 가장 큰 신뢰 문제다.

## (b) 과잉 설계 vs 진짜 공백 — 사용자 전제의 교정

| 사용자 전제 | 판정 | 근거 |
|---|---|---|
| "핵심 목적성이 확립되지 않았다" | **반만 맞음** | 한 줄 피치·3대 기둥·첫 10분 타임라인(`guideline.md:168`, `:340`)은 분 단위로 명확하다. 다만 '재미'의 실체인 결정 루프(카드·칙령·여행)가 0% 구현이라 **검증된 적이 없다** |
| "전체 아키텍처가 없다" | **틀림** | 3층 상태·열 단계 틱·난수 3원천·해시 규약은 코드·테스트·문서가 일치하는 몇 안 되는 영역이다 (`Core/GalaxyState.h:4-28`, 76 테스트) |
| "기본/상세 설정값이 없다" | **반대** | `DT_Rules` 46노브, `schema.json` 143컬럼·45규칙이 있다. 문제는 값의 부재가 아니라 (1) 18노브가 **읽히지 않고**, (2) 밸런스 핵심 12상수(연구 0.003/Insight, 수입 승수 등)가 **cpp에 박혀** 있으며, (3) 1.1MB 문서에서 값을 **찾을 수 없다**는 것 |

**과잉 설계(코드 전에 다시 바뀔 것)**: 건넘·요약 은하(§5.7, Phase 6a), 네 길·교리 4종·독립 초월자(§4.2·§4.5), 두 책·28줄 예산(§8.4), 모티프 30종(§8.6), 실제 하늘 투영·레인 평면 그래프(§3.4), 서막 카드 8장 문안 후보, EA Hazard 6종. 이것들은 플레이테스트 한 번 없이 수치까지 적혀 있어 Phase 4 첫 카드가 들어오는 순간 재작성 확률이 높다.

**진짜 공백**: ① 플레이어 입력(연구 슬라이더·칙령·조우·청원·여행) ② 사망→계승 루프(`Effect.KillPlayer`는 기록만 남음) ③ 저장/이어하기(게임 코드에서 `SaveCurrentGalaxy` 호출 0) ④ 안정도 회복 규칙(백성 59%가 0.3 미만, `guideline.md:4461`) ⑤ AI 프로필이 행동에 닿는 코드 ⑥ 세션 종료 조건(엔딩).

## (c) 일정 현실성

| 구간 | §11 계획 | 실제 | 비고 |
|---|---|---|---|
| P0~P3 (v0.7 예약 포함) | 1+2+3+4주 + 순증 ≈ 10~11주 | 2026-10-10 ~ 10-18, 99커밋 | 계획의 **약 10배 속도** |
| 커밋 구성 | — | 문서·리뷰 54 / 코드 44 | 절반이 문서 작업 |
| 남은 VS | v0.8 예약 11~15.5일 + P4 6주 + P4b 2주 + P5 4주 + 저장 (`guideline.md:3861-3863`, `:4002-4018`) | — | 합 약 3.5개월(계획) |

같은 속도면 코드 작업은 **3~5주**에 끝날 수 있다. 그러나 가속되지 않는 세 가지가 있다: (1) 한국어 콘텐츠 35카드·40이벤트·21계시줄의 저작과 톤 감사, (2) Phase 5 외부 테스터 5~10명 운영(4주는 달력 시간), (3) **개발자 손 확인** — Phase 3 종료 조건의 사람 몫이 10-17부터 대기 중이며(`WORKPLAN.md:200`) 꼬리 7·12·8·9와 v0.8 예약 전부가 그 뒤에 직렬로 서 있다. "EA 12~18개월"은 문서 안에서도 어긋난다(`guideline.md:3998` 8개월 + Phase 6+ 6~12개월 = 14~20개월). 현실적 전망: **Playable MVP 4~8주, 코어 시뮬 완성 3~4개월, EA는 범위를 절반으로 잘라야 12개월 안**.

## (d) 범위 팽창 징후와 Cut/Defer

| 항목 | 징후 | 제안 |
|---|---|---|
| 은하 여럿·건넘·요약 은하 | `Transits`·`BridgeTicks`·`SummaryAt` 전부 예약만, 플레이어 건넘은 "아무것도 예약하지 않음"(§16 #47) | **Defer to post-EA**. DT_Galaxies 5행은 패널 문안으로만 유지 |
| 실제 하늘 카탈로그·DT_Planets | 8~11일 추정, 모든 시드 기준값 교체, 라이선스 미확인, 모성계 G형 전제와 M형 76% 충돌(`:587`) | **Defer to Phase 5 이후**. 지금의 StarNameBank로 VS 완주 |
| 서막 8장·고넬료 사슬·옛 땅의 기억 | 카드 화면(Phase 4) 없이는 못 만듦; 문안 전부 '후보' | 서막은 **카드 3장 최소형**으로 Defer; 고넬료 사슬은 Phase 4 뒤 |
| 네 길·교리 4종·독립 초월자·후원 카드 | VS는 침묵 1체, 길 1종 | **Cut from VS**, EA 심사 |
| 모티프 24~30종·두 책 28줄 | 데이터 행 명세 없음 | VS 6모티프만, 두 책은 한 장 요약으로 |
| 성역 sol·Law.Hedge | ID 체계·테스트 고정물 약 40곳 교체 | 유지하되 **P4 카드 화면 뒤**로 순서 이동(지금 당기면 모든 기준값이 두 번 바뀜) |

## (e) 민감 요소의 제품 리스크

- **신학 프레임의 밀도**: `guideline.md`에 성경 장절 인용 약 395건, `v08_rulings.md` 약 399건. 설계 근거로는 일관되지만, 인월드 텍스트는 판정 어휘 7개를 스키마가 막고 "미선언·미은폐" 포지셔닝(`guideline.md:4129`)까지 정해 두어 제품 쪽 방어는 충분히 사고되어 있다. 남은 리스크는 **양방향 리뷰 폭격**(문서 스스로 인정)과 **한국어 개역한글 어휘의 번역 불가성**(영어 WEB 대응표 없음) — 로컬라이제이션 설정 자체가 ini에 없다.
- **실존 천체명**: 통용 지명 수준이라 저위험. 단 HYG·Gaia DR3는 "확인 전 보류"로 올바르게 적혀 있고(`:587`), '다이슨 스웜'은 통용 용어로 명시 허용(`:1264`)되어 실존 인물 규칙과 모순 없음.
- **대도감 톤**: `cosmic_species_codex.md`에 '칼라' 18회·'아콘' 3회·'오버마인드'·'아셰라'·'아폴리온', '영혼' 20회 — 타작품·성경 고유명과 금지어가 원천 문서에 그대로다. 원형 장부(치환표)가 아직 없어 **IP·금지어 누출은 사람 검토에만** 걸려 있다. `code_artifact.md`(인류 성역 밖 존재)는 정경과 정면 충돌하는 옛 판이 루트에 남아 에이전트 오참조 위험.

## (f) 문서 신뢰성 — 감사 기록 표본 3건

| 표본 | 기록 | 검증 결과 |
|---|---|---|
| `guideline.md:62` "55건 → CONFIRMED 53": "시뮬이 읽는 FText는 CivName의 유무 하나" | 당시 사실 | 지금은 `:3275`가 "시뮬이 읽는 FText는 없다"로 뒤집음; 코드도 `CivName`을 로그 인자로만 씀(`Sim/GalaxySimulationSubsystem.cpp:1108`). 기록이 **덮어쓰기 없이 누적**되어 옛 사실이 현재형으로 남음 |
| `CLAUDE.md:251`·`guideline.md:3887` "자동화 테스트 12종" → 이후 76종 | 수치 추이 | 등록 테스트 76개 확인, §9.3 표의 나머지 17행은 전부 미래 표지 — **테스트 수 기록은 신뢰 가능**. 단 "변이 N개로 깨뜨려 보았다"는 저장소에 산출물이 없어 **검증 불가** |
| `CLAUDE.md:248` "GameplayTag 총 136개 (당시)" | 괄호로 자기 정정 | 실제 140. 스스로 낡았다고 적으면서 숫자를 남기는 양식이 문서 전반의 패턴(같은 파일 `:259` 18% vs `:262` 20%) |

결론: 리뷰 건수·CONFIRMED 수는 **과정의 증거**이지 현재 상태의 증거가 아니다. 통폐합 시 이 기록들은 CHANGELOG로 보내고 본문에는 "지금 참인 것"만 남겨야 한다.

## 권장안
- [P0] **본문을 '현행(as-built)'과 '예정(roadmap)'으로 물리적으로 분리** — guideline.md 한 파일 안의 162개 'v0.8 — 단계 N' 표지와 표지 없는 미구현 현재형(외교·분쟁·BuildChronicle·자동 저장)을 가르기 위해, SSoT를 (1) DESIGN.md 현행 규칙 (2) ROADMAP.md 예정 설계 (3) CHANGELOG.md 리뷰·해시·변이 기록 세 파일로 쪼갠다. 규칙: 코드·데이터에 없는 것은 DESIGN.md에 쓰지 않는다.
- [P0] **드리프트 17건을 한 커밋으로 닫는 '문서 동기화 패스'** — 표 (a)의 낡은 수치(136→140, 용도 상수 8→9, -TellAll Phase 5, 승천 연출 목록, 18%/20%, §4.1 AI 각성·역병 표)를 먼저 고친다. 코드가 뒤따라야 하는 5건(arch_seeker·Law.Hedge·항성 안정화·첫 줄·sol)은 '예정' 표로 옮기고 본문에서 현재형 서술을 지운다. 이 패스 전에 새 설계를 더하지 않는다.
- [P0] **개발자 손 확인을 즉시 수행하고 Phase 3 종료를 선언** — 10-17부터 대기 중인 '창에서 마우스만으로 셋째 칸, stat fps 60'이 꼬리 7·12·8·9와 v0.8 예약 전체를 막고 있다(WORKPLAN.md:200). 임시 빌드(sol 모성계)로 확인하고 결과(fps 수치·막힌 조작)를 CHANGELOG에 한 줄로 적는다. 60fps 미달이어도 종료 조건을 '게임 스레드 ≤10ms'로 재정의해 넘어간다.
- [P0] **Playable MVP 정의를 '결정 루프 1개 + 세션 닫기'로 축소** — MVP = (1) 자동 일시정지 옵션 (2) 연구 중점 슬라이더 (3) 청원 카드 3장(첫 접촉·반란 50틱·헤롯) (4) 자동 저장·이어하기 (5) 엔딩 조건 1개(AI T3 선점 또는 N틱). 서막·성역·실제 하늘·사망 파이프라인은 MVP 밖. 이 다섯이 들어가야 '재미'가 처음으로 측정 가능해진다.
- [P1] **v0.8 예약(성역·실제 하늘) 순서를 Phase 4 카드 화면 뒤로 이동** — 지금 당기면 모든 시드 기준값·테스트 고정물 약 40곳(Sim.FirstLife 등)이 두 번 바뀐다. 성역은 카드 화면·DT_Encounters가 생긴 뒤 '지경' 카드와 함께 한 번에, 실제 하늘은 Phase 5 이후로 Defer. v08_rulings.md E장은 적용 완료 패치이므로 Docs/history로 아카이브한다.
- [P1] **읽히지 않는 DT_Rules 노브 18개에 '소비 Phase' 열을 달거나 제거** — Succession 6·Fleet 4·Leap 3·ReturnLure 2·ReturnedLivesToSeize·VigilClaimExtension·ImmutableLaws·LureAcceptByPolicy는 런타임 참조 0회다. schema.json에 'reads_from: Phase 4b' 같은 표지를 두고 Data.Integrity가 '읽는 코드 없는 노브'를 경고하게 한다. 반대로 cpp 상수 12개(연구 0.003·수입 승수·StarClassEnergy)는 DT_Rules로 올린다.
- [P1] **밸런스 회귀 테스트 3개를 커맨드렛 수치에 건다** — 주장 만료율·안정도 0.3 미만 비율·첫 600틱 말해진 줄 수를 Sim.BalanceBaseline으로 단언(허용 범위 ±30%). 지금은 §9.3 목표(40~60%, 25~40줄)와 실측(18%, 9줄)의 차이를 사람이 커맨드렛 로그에서 읽어야만 안다. #68 안정도 회복·#71 AI 구호 정책을 넣은 뒤 기준값을 갱신한다.
- [P1] **대도감 원형 장부(치환표)를 데이터 파일로 만들고 금지어 검사를 낱말 경계로 확장** — cosmic_species_codex.md의 타작품·성경 고유명(칼라·아콘·아셰라·아폴리온 등)과 '영혼' 20회가 게임 텍스트로 새지 않도록 Tools/data/archetype_ledger.json(원문 이름→표시명→행 이름)을 두고, schema words에 other_works·holy_and_myth 목록을 추가한다. code_artifact.md는 Docs/history로 옮겨 오참조를 막는다.
- [P1] **Cut List에 Phase 6 항목을 명시적으로 이동** — 건넘·요약 은하·둘째 상세 은하·네 길·교리 4종·독립 초월자·모티프 24종·실제 하늘 카탈로그·DT_Planets를 ROADMAP의 'EA 이후' 섹션으로 옮기고 §12.1 VS 표에서 뺀다. VS 표는 현재 데이터 행 수를 자동 생성(스크립트)하여 '목표 7행'과 '현재 2행'을 나란히 보이게 한다.
- [P2] **로컬라이제이션 설정과 영어 어휘 대응표를 지금 결정** — Config에 NativeGameCulture=ko와 Localization 타깃을 넣고, 개역한글 어휘(구별·부르심·날이 더해짐·남은 자·두 책)의 영어(WEB) 대응표를 ST_CA 옆에 둔다. 번역 시작 뒤 배열 끝에만 추가하는 규칙은 타깃이 있어야 의미가 있다. 스토어 선언 2문장과 FAQ 초안을 Docs/store.md로 분리한다.
- [P2] **한 줄 수 KB 문단을 번호 목록·표로 정규화하는 기계 패스** — guideline.md:3922(65KB 한 줄)·:13(1KB 셀)·§16 확정 블록 63KB를 CHANGELOG로 옮기고, 남는 본문은 줄당 300자 상한으로 쪼갠다. §번호 대신 안정 ID(RULE-SIM-003 식)를 달아 코드 주석의 §참조 70여 개가 재편 뒤에도 깨지지 않게 한다.
- [P2] **테스트 러너·변이 기록 스크립트를 저장소에 추가** — '빌드 뒤 두 번 연속 실행'과 변이 테스트 절차가 CLAUDE.md 산문에만 있고 산출물이 없어 '변이 N개' 기록을 검증할 수 없다. Tools/run_tests.py(두 프로세스 실행·SaveRestart 기록 검사·결과 요약)와 Tools/mutations/*.patch를 두어 감사 기록이 재현 가능해지게 한다. Linux 체크아웃에서도 돌도록 경로를 인자화한다.

## claims
- A6-1 [factual/high] guideline §12.1·변경 이력이 선언한 VS 수량(아키타입 7/11행·Hazard 8·업적 4·조우+청원 35)에 대해 현재 데이터는 아키타입 2행·Hazard 5행·업적 2행·조우/청원 0행이다(DT_Encounters는 스키마 columns null).  (근거: Tools/data/DT_Civs.json(2행), DT_Hazards.json(5행), DT_Feats.json(2행), Tools/data/schema.json:591; guideline.md:26, :1272, :4039, :4044, :4046, :4047)
- A6-2 [factual/high] v0.8의 핵심 결정(첫 생애 arch_seeker·모성계 sys_0·성역 sol·Law.Hedge·표시명 '항성 안정화'·첫 줄 '맡겨졌다')은 데이터·ini·코드 어디에도 없고, DT_Rules.PlayerArchetypeRow는 arch_terran, ImmutableLaws는 4개, log_first_life는 옛 문장이다.  (근거: Tools/data/DT_Rules.json:26,28; Config/DefaultGameplayTags.ini(Law.* 4개); Tools/data/DT_Megaprojects.json:52; Tools/data/DT_LogTemplates.json:8; Source/Cosmic_Ascension/Sim/GalaxySimulationSubsystem.cpp:45; guideline.md:407, :946, :1076, :340)
- A6-3 [factual/high] 문서가 현재형으로 서술하는 §5.5 외교·§5.6 분쟁은 TickDiplomacy/TickConflict 빈 함수이며, DT_Rules의 Fleet 4·Succession 6·Leap 3·ReturnLure/ReturnedLivesToSeize/VigilClaimExtension/ImmutableLaws/LureAcceptByPolicy 노브는 런타임 코드가 읽지 않는다.  (근거: Source/Cosmic_Ascension/Sim/GalaxySimulationSubsystem.cpp:1585-1593; Tools/data/DT_Rules.json:4-14, 22-26, 30-34; Source/Cosmic_Ascension/Data/CosmicDefinitionRows.h:664-792(참조 grep))
- A6-4 [factual/medium] CLAUDE.md와 guideline의 '현재 상태' 수치가 낡아 있다: GameplayTag 136(실제 140), CosmicSeed 용도 상수 8(실제 9), '-TellAll은 Phase 5'(이미 구현), '아직 없는 것'에 승천 연출(구현됨), 주장 만료 18% vs 20%.  (근거: CLAUDE.md:248, :254, :259, :262; guideline.md:3135 vs :3277; Config/DefaultGameplayTags.ini(140줄); Source/Cosmic_Ascension/Core/CosmicSeed.h:24-32; Source/Cosmic_Ascension/Tests/CosmicBalanceCommandlet.cpp:231)
- A6-5 [factual/medium] §11 일정은 P0~P3를 약 10~11주로 계획했으나 실제로는 2026-10-10~10-18의 9일·99커밋(문서성 54·코드성 44)에 끝났고, 남은 VS 계획(v0.8 예약 11~15.5일+P4 6주+P4b 2주+P5 4주)과 'EA 12~18개월'은 같은 문서의 '약 8개월 + 6~12개월'과도 어긋난다.  (근거: git log --date=short(99커밋, 10-10~10-18); guideline.md:3861-3863, :3998, :4002-4018; WORKPLAN.md:52-55, :115, :117)
- A6-6 [judgment/high] 사용자의 전제 '아키텍처·설정값이 확립되지 않았다'는 틀리다: 3층 상태·열 단계 틱·난수 3원천·해시 규약·DT_Rules 46노브·schema 143컬럼은 코드·테스트·문서가 일치하는 가장 단단한 층이다. 진짜 공백은 플레이어 입력(카드·칙령·슬라이더·여행), 사망→계승 루프, 저장/이어하기 호출, 엔딩 조건, 안정도 회복이며 '재미'는 한 번도 측정되지 않았다.  (근거: Source/Cosmic_Ascension/Core/GalaxyState.h:4-28; Tools/data/schema.json; Tests 76종 등록; Source/Cosmic_Ascension/Save/CosmicSaveSubsystem.h(게임 코드 호출 0); Source/Cosmic_Ascension/Player/PlayerRoleSubsystem.h:63-76; guideline.md:4461)
- A6-7 [judgment/medium] 1.1MB 설계의 절반 이상(건넘·요약 은하·네 길·교리 4종·독립 초월자·두 책 28줄·모티프 30종·실제 하늘 투영·서막 8장 문안)은 플레이테스트 없이 수치까지 적힌 과잉 설계로, Phase 4 첫 카드 이후 재작성될 가능성이 높아 EA 이후로 Defer하거나 VS에서 Cut해야 1인 개발 범위에 맞는다.  (근거: guideline.md:1112-1122(건넘), :964-969(길), :675(교리 4), :3419(두 책), :3452-3458(모티프), :587-593(실제 하늘), :796(서막); §16 #47(플레이어 건넘 미예약))
- A6-8 [factual/low] 감사 기록('N건 → CONFIRMED M건')은 과정의 증거일 뿐 현재 상태를 보증하지 않는다: guideline.md:62의 '시뮬이 읽는 FText는 CivName 하나'는 :3275에서 '없다'로 뒤집혔고, 테스트 수 기록(12→76)은 실제 등록 76개와 일치하지만 '변이 N개' 기록은 저장소에 산출물이 없어 검증 불가하다.  (근거: guideline.md:60, :62, :3275, :3887; CLAUDE.md:251; Source/Cosmic_Ascension/Tests/*.cpp(IMPLEMENT_SIMPLE_AUTOMATION_TEST 76); Tools/(러너·변이 스크립트 없음))
