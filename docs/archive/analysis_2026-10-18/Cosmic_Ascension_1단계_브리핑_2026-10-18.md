> **문서 성격**: 1단계(분석·제안) 브리핑. 저장소의 파일은 수정하지 않았다(분석 산출물은 저장소 밖 `_analysis_2026-10-18/`에 있다).
> **기준**: `rhwwl12-creator/Cosmic_Ascension` HEAD `75af02b`(2026-10-18, 99커밋). 클론 위치 `/home/mechtro/Desktop/bbb/Cosmic_Ascension`.
> **방법**: 읽기 에이전트 24개(guideline.md 10토막·v08_rulings 3토막·CLAUDE/WORKPLAN/대도감·코드 8모듈 전수) → 관점 분석 6개(정체성·코어루프 / 설정값·밸런스 / 문서 통폐합 / 아키텍처·가이드라인 / 로드맵·워크플로 / 드리프트·범위 회의론) → 핵심 주장 48건 적대 검증(확정 15·부분 확정 33·반박 0 — 부분 확정은 정정문을 반영) → 종합. 그 뒤 Claude(메인 세션)가 코드·git을 직접 열어 핵심 수치를 재확인하고 다듬었다.
> **읽는 법**: §0 한눈에 → §9 결정 목록만 읽어도 2단계 착수 여부를 정할 수 있다. 나머지는 2단계 실행 명세다.

---

# Cosmic Ascension — 1단계 브리핑: 전수 조사 분석·문서 통폐합 계획·설계 보완 제안

기준 2026-10-18 HEAD(75af02b, 99커밋). 수치는 저장소에서 재확인했고, 렌즈 간 의견이 갈린 곳은 근거가 강한 쪽을 택하고 반대 의견을 한 줄로 남겼다. 설계 어휘는 첫 등장에 식별자를 병기한다.

## 0. 한눈에

1. **진단**: 아키텍처(단일 상태 `FGalaxyState`, 열 단계 틱, 난수 3원천, 리플렉션 해시, 데이터 파이프라인, 테스트 76종)는 1인 프로젝트로는 드물게 단단하다. 반면 플레이어가 세계를 바꾸는 명령은 거대 프로젝트 시작(`StartMegaproject`, 정착 포함) **하나**이고, 판에는 끝·저장 호출·사망이 없다.
2. **전제 판정**: "아키텍처가 없다"는 틀리다. "설정값이 없다"는 절반만 맞다 — 값은 넘치나(DT_Rules 44노브·스키마 143컬럼) 18노브는 읽는 코드가 없고, 경제 상수 9개는 cpp에 박혀 있으며, 문서 스스로 '잠정값(Phase 5 확정)'이라 적는다.
3. **문제 ① 결정이 없다.** 첫 600틱의 플레이어 결정 약 1회(목표 15~20), 첫 위기가 10분 안에 설 확률 18.5%(2:30 안 5%), 말해진 줄 중앙값 9(목표 25~40). 청원·조우·칙령·연구 중점·자동 일시정지가 전부 Phase 4다.
4. **문제 ② 문서가 코드를 수십 배 앞선다.** 설계 1.7MB vs 코드 13K줄·데이터 42행. `v0.8 — 단계` 표지 162회·`가칭` 63회가 현재와 예정을 한 문장에 섞고, 같은 규칙이 3~4곳에 복제돼 있다(CLAUDE.md 불변 규칙 36줄 = guideline §14.3 바이트 동일).
5. **문제 ③ 세계가 고장 나 보인다.** 백성 59%가 안정도 0.3 미만, 위기의 62%가 반란(목표 25~40%), 주장(`Hazard.Claim`) 만료 18%(목표 40~60%) — 안정도 회복 규칙이 코드에 없다(§16 #68).
6. **권장 방향**: (a) 2단계 통폐합은 '새 설정값 작성'이 아니라 **현행/예정 분리·복제 제거·이력 분리**다. (b) 코드는 'v0.8 예약 ①(성역·첫 생애 행)'을 한 번에 치른 뒤 **Playable MVP(통치자 단일 모드·카드·자동 일시정지·저장·판의 끝)**를 실제 하늘·서막 전문보다 앞에 둔다. (c) 신학 레이어는 '선택지를 만들거나 규칙을 바꾸면 코어, 문장만 바꾸면 취향'으로 가른다.
7. **남은 거리("얼마나 남았어?")**: 현 속도(P0~P3 본체 9일) 기준 정리+MVP까지 약 20~30 작업일, 코어 시뮬 완성(Phase 4·4b)까지 추가 35~50 작업일. 단 텍스트(카드 35·이벤트 40)는 코드 속도로 나오지 않는다.

## 1. 현재 상태 분석

### 1.1 코드·구현 현황

| 모듈 | 상태 | 확인된 사실 |
|---|---|---|
| Core/Data | 완성 | `FGalaxyState` 단일 USTRUCT, 세계→앎(Knowing)→이야기 3층, `CosmicSeed::Derive` 골든값 고정. DT 로더 9종; DT_Encounters는 구조체만, Edicts·Traits·Relics·Dedications·Planets 없음 |
| Sim | 8/10단계 | `TickDiplomacy`/`TickConflict` 빈 함수(GalaxySimulationSubsystem.cpp:1585-1593). `ResearchFocus` 0.25 고정(:752). 엔딩 검사 없음(:1066). `SetTimeScale` clamp 10(:1036) |
| Events / Log | 완성 | 조건 30·효과 18 단일 평가기, `PushVariants` 한 길·범위·`bUntold`. `DurationTicks` 미읽음, KillPlayer는 기록만, `BuildChronicle` 없음 |
| Ascension / Player | 최소 | 통치자 경로 업적 2행 판정만. 명령은 `Install`·`DeposeRuler`·`BecomeWanderer`·`StartMegaproject`뿐(PlayerRoleSubsystem.h:38-76) |
| Save | 미배선 | 왕복·재시작 테스트 통과하나 게임 코드 호출 0건. 자동 저장·슬롯 열거·가드 없음 |
| UI/World | Phase 3 본체 | 13 C++ 위젯·지도·카메라. 꼬리 7(영적 시야)·8(앎 표시)·9(우주 화면)·12(×25·명령 사유) 미착수 |
| Tests/Tools | 단단 | 테스트 76종(Sim 49·Events 12·UI 7·Log 4·Feats 2·Data 2), 스키마 검사기+Python 테스트 69건, sha1 도장 |

**플레이어가 지금 할 수 있는 것**: 지도·성계 선택·카메라, 배속 0/1/2/5/10, 패널의 '시작 <프로젝트>'(다이슨·개혁·구호·초신성 안정화)와 '정착', 로그 피드, 승천 연출, 시계 단위 토글. 폐위되면 세계에 닿는 입력이 0이 된다.

### 1.2 콘텐츠·데이터 현황

| 테이블 | VS 목표(§12.1) | 현재 | 비고 |
|---|---|---|---|
| DT_Civs | 7 | 2 | v0.4 요약의 '11'은 VS 6+EA 5 합계 |
| DT_Hazards | 8(+방문 1) | 5 | Cult·Extinction·SubspaceErosion·Visitation 태그만; 아공간 침식의 때 #90 미결 |
| DT_Events | 40 | 11 | SignRow·MinTierToReveal 미사용 |
| DT_Encounters(조우 25+청원 10) | 35 | 0 | JSON·에셋·로더 없음, 스키마 columns null |
| DT_Feats / DT_Megaprojects | 4 / 7 | 2 / 5 | 스키마가 Path=Ruler만 허용; 방주·웜홀 없음 |
| DT_Edicts/Relics/Traits/Dedications/Planets | 9/8/— | 없음 | 구조체도 없음 |
| DT_Transcendents | 1 | 1 | Title·Lure·CounterfeitOf·Voice·Claim 빈 값 |
| LogTemplates/Galaxies/Rules/ST_CA/태그 | — | 10/6/44노브/37키/140 | 노브 18개 런타임 미읽음, 태그 약 100개 미사용 |

### 1.3 문서 현황

| 문서 | 분량 | 역할 | 실제 내용·문제 |
|---|---|---|---|
| guideline.md | 4,598줄 1.1MB | 설계 SSoT | 비전+명세+헤더 사본(§7)+테스트 카탈로그(§9.3 한 셀 16.7KB)+일지(§11 Phase 2 상태 줄 65KB)+미결·확정 24블록(§16 107KB). 1,000B 초과 줄 276개 |
| v08_rulings.md | 1,147줄 261KB | 단계 1 판정 | A~D 결정 근거, E는 적용 완료 패치(9aba0e9). 행 번호가 편집 전 기준 |
| CLAUDE.md | 279줄 118KB | 작업 규칙 | 불변 규칙 26KB(§14.3과 동일)+빌드 21KB+엔진 헤더 표 31KB+일지 34KB. Windows 경로 전제 |
| WORKPLAN.md | 216줄 140KB | effort 지시서 | §3·부록 A 약 110KB가 일지; guideline §11 상태 줄과 축자 80% 중복 |
| codex / code_artifact / directive | 48KB / 36KB / 7KB | 세계관 원천 / 옛 판 / 지시서 | 차용 고유명·'영혼' 어휘가 게임 규칙과 충돌; 옛 판은 v0.8 정경과 상충한 채 루트; 지시서는 삭제된 §17을 9회 가리킴 |

### 1.4 문서↔코드 드리프트 표

| # | 문서 | 저장소 | 종류 |
|---|---|---|---|
| 1 | 첫 생애 `arch_seeker`·`sys_0`·성역 `sol`·`Law.Hedge`(guideline:407,946) | `PlayerArchetypeRow=arch_terran`, `sol`=플레이어 모성계(Sim cpp:45), Law 4개 | 표지 있는 예정(단계 4·4b) |
| 2 | '항성 안정화', 첫 줄 '맡겨졌다'(:1076,:340) | '초신성 안정화', '당신은 이 행성의 통치자다' | 표지 있는 예정(단계 5·4b) |
| 3 | §5.5 외교·§5.6 분쟁 현재형 | 빈 함수, Fleet·Succession·Leap 노브 18개 미읽음 | **표지 없는 미구현** |
| 4 | §4.1 표 AI 각성 '생산 +150%/성계 반란', 역병 '인접 전파'(:625-626) | +30/+30 또는 −0.15, 이웃 효과 [] | 표에 축소형 표지 없음(:652에만) |
| 5 | §2.1 셋째 칸 ×25(:251) | `SetTimeScale` clamp 10, `Sim.FrameStep`이 상한 10 단언 | 설계가 Sim 상한을 모름 |
| 6 | §2.4 2:30 첫 Hazard(:340) | 150틱 안 확률 5%, 보장 기제 없음 | 기제 미명세 |
| 7 | '-TellAll은 Phase 5'(:3135) | 구현됨(커맨드렛 :231) | 낡은 문장(유일한 확정 수치 상충) |
| 8 | §10.1 WBP_*·M_Lane·L_MainMenu, §12.1 'Niagara 1', 헤더 'RevelationTemplates Exactly 3' | C++ 위젯·LineBatch·L_Galaxy, NS_Ascension 미룸, 스키마 [1,3] | 낡은 트리·표·거울 주석 |
| 9 | 승인 헤더 초안 `Saved/Design/phase3_*` | .gitignore 대상, 체크아웃에 없음 | 결정 전문이 저장소 밖 |

### 1.5 품질·아키텍처 평가

**강점**: 레이어 규칙 준수(Core Logic 여섯 폴더에 UI/Actor include 0건; Sim↔Events는 .cpp 수준 상호 의존이지 헤더 순환은 아님), 결정론 규율(행 보기 전 추첨·`FMath::Rand` 0건·FP Precise), 데이터 파이프라인 4겹 검증, 변이 테스트 관행, 한국어 템플릿 124문장의 고른 톤.

**위험**: ① 단일 모듈이라 레이어 규칙을 빌드가 강제하지 않고 린트도 없다. ② `GalaxySimulationSubsystem.cpp` 2,241줄·`NewGalaxy` 464줄, 핸들러 거절 가드 22곳 복붙, 태그 리터럴 59곳. ③ 쿠킹 빌드 해시 동일성 미검증. ④ 세이브 1MB 예산 초과(VS 1.8MB). ⑤ 증거 테스트(`Sim.FirstLife` 등)가 `sol`·`arch_terran`을 고정해 단계 4a/4b에서 깨질 예정. ⑥ 99커밋 중 32개가 'WORKPLAN 표에 해시 적기' 1~2줄 커밋.

### 1.6 사용자 전제에 대한 판정

| 전제 | 판정 | 근거 |
|---|---|---|
| 핵심 목적성 미확립 | 반만 맞음 | 피치·루프·엔딩 4종·첫 10분 타임라인은 분 단위로 적혀 있다(§1.2·§2.2·§2.4·§4.4). 다만 결정 장치가 0% 구현이라 '재미'가 측정된 적 없고, VS에는 T3→T4 행이 없어 '유일한 시계'(AI T5)가 돌지 않는다(:683) |
| 아키텍처 미확립 | 틀림 | §1.5 강점. 단 '상용 수준'은 과장(쿠킹 미검증·저장 미배선) |
| 설정값 미확립 | 절반 | 44노브 중 26개만 실효, 18개 예약, 설계가 적은 6개(Crossing*·Scenery 등)는 데이터에 없음, 경제 상수 9개 cpp. 소유권은 선언돼 있다(guideline→JSON→schema→ini) — 틈은 '검사되지 않는 거울'(주석·DevComment)과 예약 표지 부재 |

**과잉**: Phase 6a/6+ 항목(건넘·네 길·교리 4·모티프 30)에 수치까지 선행 — 단 본문 비중 약 7%이고 대부분 이미 VS 밖이라 자를 것은 적다. **공백**: 플레이어 입력(카드·칙령·슬라이더·여행), 사망→계승, 저장 호출, 엔딩, 안정도 회복, AI 프로필 행동.

## 2. 프로젝트 정체성과 코어 루프(제안)

### 2.1 한 문단 정의
너는 신이 아니다. 성역(태양계 `sol`) 밖 먼 별의 필멸자로 부르심(`FCallingState`)을 입어 한 백성(civ)을 맡고, 정착된 성계에 서는 카운트다운 위기(`Hazard`)에 답하며 존재의 격을 다섯 칸(Tier 1~5) 오른다. 그릇(vessel)은 쓰러지고 부름만 이어진다. 힘은 결말을 사지 못하지만 시간·피난·기록은 산다. 목표는 T5의 두 길(받는 길/찬탈의 길), 엔딩은 집단·단독 승천·다른 질서·침묵의 우주(§4.4), 실패는 붕괴·폐위→방랑과 사망→계승이며 GAME OVER는 없다. 판 끝에 네가 한 일이 두 책으로 펼쳐지되 아무도 너를 판정하지 않는다(§1.2:168·§4.4:781-792·§8.4).

### 2.2 코어 루프

| 단위 | 현 설계 | 제안(틱 기준, ×10 전제) |
|---|---|---|
| 30초(≈300틱) | 로그 누적→자동 일시정지→결정(:262) | 카드 1장 또는 단추 1개. Severity 2·자기 성계 위기·카드 도착에서 자동 일시정지 **기본 켬**. 피드백 = 결정이 낳은 Severity 1 줄+Stability 60틱 추세 |
| 10분(≈6,000틱) | 위기→업적 기회→승천(:276) | 위기 1~2개를 발생→답→만료/해결까지. 첫 위기·첫 청원은 **모든 백성 공통 규칙**(Standing 행+`GalaxyTickAtLeast`+`SpawnHazard`, 또는 Phase 4 헤더의 `Placed`)으로 보장 — 플레이어 전용 노브는 §16 #91과 충돌 |
| 1시간 | T1 30분·T2 60분(:328) | 한 칸 = 건설 업적 1 ∨ 위기 해결 업적 1. T2→T3에 건설 경로 둘째 행을 더해 '언제나 다음 할 일'이 있게 |
| 1판(MVP) | 4~6시간·T5·2~5대 | 통치 백성 T3 ∨ 틱 상한 → 연대기 최소형. 사망·계승·T4~5는 Stage B |

밀도 목표는 '1배속 20~40초'(:282)가 아니라 **'300~600틱마다 결정 1'**로 다시 적고, Phase 4 종료 조건 '30분 T1→T3'(:3958)에 배속(×10 권장)을 명시한다. 지금의 격차는 설계 결함이 아니라 문서가 '청원 카드가 채운다'(:284)고 적은 예정 콘텐츠의 부재다.

### 2.3 두 모드의 관계
첫 생애 통치자 고정, 같은 은하·이벤트 풀·`FEncounterDef`를 두 시점에서 소비(§5.10). 지금은 폐위 단방향만 있고 방랑자 입력이 0이라 차별점 (1) 모드 전환·(3) 죽음=계승은 체험 불가, (2) 승천=세계 사건은 절반. **MVP는 통치자 단일 모드**로 선언하고, 방랑자 최소 루프(TravelTo·스캔·조우 5장·BecomeRuler)는 Stage B 첫 블록으로 둔다.

### 2.4 신학 레이어의 처리 원칙
성경 장절 인용은 guideline 395회·rulings 399회, 게임 데이터 0회 — 데이터 문장은 '패턴만 빌린다'(§8.3 규칙 4)는 설계대로다. 기준: **선택지를 주거나 규칙을 바꾸면 코어, 문장만 바꾸면 취향**. 코어 = 성역 법·부름/그릇·타락 4교리의 위조 원리·판정 금지·두 책·신정론 예산·**고넬료 사슬**(선택지·특성 전이를 가진 체인)·의회/회중 두 목소리 청원. 뒤로 미룰 글 = 헌정명 뱅크 확장·성육신 고백 한 줄·서막 카드 **문안**·모티프 24종. 가장 큰 실은 영압(`ComputeSpiritualPressure`) 응답 고리가 '얼굴 없음'(trans_silence 문장 컬럼 전부 빈 값)+'카드 없음'인 것 — 구호 대응과 영향권 정착 차단은 이미 돈다. 설교조 리스크의 완화책(§8.3 사후 문장 ①~⑩·스키마 `verdict` 금지)은 유지한다.

## 3. 미확정 설정의 구체화

### 3.1 확정 기본값 표(현재 코드·데이터가 읽는 값)

| 이름 | 값 | 출처·위치 |
|---|---|---|
| SecondsPerTick / MaxTicksPerFrame / 배속 | 1.0 / 10 / {0,1,2,5,10} | Sim .h:491,494 — 코드 상수(법) |
| 판 규모 / 이벤트 슬롯 / Hazard 확률·카운트다운 | 성계 60·백성 12 / 10틱·0.02 / AI 각성 2e-4·역병 1e-4·초신성 5e-5(T2+), 150·200·300·200·300 | DT_Galaxies / DT_Rules / DT_Hazards |
| 프로젝트 비용×진척 / 업적 | 다이슨 E2M1×600·개혁 Inf1×100·구호 M1Inf1×150·정착 E1M2×300(T2)·안정화 E2M2×100(T2) / T1→T2 다이슨+물리 0.5, T2→T3 안정화 | DT_Megaprojects / DT_Feats |
| 영압 / 보유 상한 / 시야 / 통치 | {0,0,0.25,1,2}·Falloff 0.5·Drain 0.002 / {1,3,4,7,10} / {1,2,4,6,10}, FirstSightTick 330 / ThroneLockTicks 600(쓰기만)·PostRebellionStability 0.15·ReverenceDecay 0.5 | DT_Rules 노브 |
| 경제·연구 상수 9 | TierMultiplier {1,1.5,2.2,3,3}·StarClassEnergy·InsightPerPop 1.0·InfluencePerPop 0.5·보너스 3·ResearchInsightShare 0.5·ResearchPerInsight 0.003 | Sim cpp:83-100 constexpr('잠정') |
| 초기 Stability / 저안정 문턱 / 예약 노브 18 | 0.75 / 0.3 / Leap 3·Succession 6·Fleet 4·ImmutableLaws·Vigil·ReturnLure 2·ReturnedLives·LureAccept | GalaxyState.h:283, Sim cpp:103 / DT_Rules — 읽는 코드 0 |

### 3.2 공백 설정의 기준안 표

| 항목 | 현황 | 제안값/공식 | 근거 |
|---|---|---|---|
| 안정도 회복 | 상승 규칙 없음 | 수도에 위기 없을 때 0.6으로 +0.001/틱, 반란 해결 시 +0.1 | §16 #68. 반란 몫 62→25~40% |
| AI 구호 정책 | 주장이 서면 항상 구호 | AIProfile 분기: Conservative·Scholarly 구호, Expansionist·Militarist 방치 | #71. 주장 만료 18→40~60% |
| T2→T3 속도 | 전조 5e-5(3성계 평균 6,700틱) | 1e-4로 상향+건설 업적 둘째 행(정착 3 ∧ 공학 0.5) | #72. AI T3 중앙값 6,400→약 3,500틱 |
| 첫 10분 박자 | 트리거 미명세 | 모든 백성 공통 Standing 행(틱 150±50 첫 위기), 첫 청원 틱 30·420; 자동 일시정지를 Phase 4 첫 헤더에 배정 | §2.4:340 vs #91 긴장 해소 |
| Influence 병목 | 0.5/인구, 조약 항 없음 | InfluencePerPopulation 1.0 또는 구호 RequiredProgress 150→100; §5.1 '조약 수'에 Phase 4 표지 | 비축 0이면 구호가 정확히 300틱 끝에 닿는다 |
| 연구 돌파 효과 / Causality | 로그 줄뿐 / 영원히 0 | 물리 1.0 안정화 비용 ×0.75, 사회 문턱마다 칙령 슬롯 +1, 공학 문턱마다 RequiredProgress ×0.9, 초월 0.5에서 Causality 0.05×TierMultiplier/틱; `SetResearchFocus` 4프리셋 | §5.4 표·§5.1 '극소량' 구체화 |
| 배속 상한 | clamp = MaxTicksPerFrame | MaxTimeScale 25와 분리, `Sim.FrameStep` 단언 갱신 | 틱 30µs라 예산 안 |
| 판의 끝(MVP 임시) / 연대기 이관 | 없음 / Severity≥1 전부 → 세이브 1.8MB | 통치 백성 T3 ∨ 틱 ≥ 6,000 → 연대기 최소형(실패 끝은 #68·칙령 유지비 뒤) / #40 ⑦ 기본값+상한 노브 1,500 | 입력 없는 판 첫 3,600틱 폐위 0회 / 1MB 예산 |
| 외교·분쟁(Phase 4) | 빈 함수 | 관계 표류 ±0.001/틱(1레인 접촉), 조약 3종, P(소유 변동)=0.002×clamp((A−D)/(A+D),0,1), FleetPower 성장=Energy 지출 0.1/틱; 레인 이동 임시 20틱 | 기존 Fleet 노브 4개 그대로 |

### 3.3 설정 관리 원칙
- 소유권은 이미 선언돼 있다(guideline=설계 SSoT → `Tools/data/*.json`=데이터 원본 → `schema.json`=규칙 → ini=태그 단일 출처). 새로 할 일은 ① `Data.Integrity`에 **'읽힘/예약(since Phase N)' 검사**를 더해 미사용 노브 18개에 표지를 달고, ② 경제·연구 상수 9개를 DT_Rules로 올리며(`StarClassEnergy`는 단계 5까지 C++), ③ 거울 주석(헤더 '정확히 3', ini DevComment 수치)을 걷어 '어디를 보라'만 남긴다.
- 노브 표(`docs/30_CONFIG_REFERENCE.md`)는 DT_Rules.json+schema.json에서 생성하되 시점은 **'v0.8 예약 ①' 뒤**. 반대 의견(A2): 모든 콘텐츠 작업보다 앞 — 채택하지 않음.
- 코드 상수는 '법'으로 단언하는 값(틱 길이·로스터 상한 3·칸 5·저안정 문턱)에만 허용하고 주석에 `[law]`.

## 4. 게임성 발전 제안

| 우선 | 항목 | 내용 | 공수 |
|---|---|---|---|
| P0 | 자동 일시정지 | Severity 2·자기 성계 위기·카드 도착에서 `SetSpeed(0)`+포커스, 승천 오버레이의 배속 복원 패턴 재사용. 기본 켬 | 0.5~1일 |
| P0 | 청원 카드 10장+카드 위젯 | `FEncounterDef` 소비. 선행: schema DT_Encounters 컬럼+중첩 구조체 검사. 순서: 첫 접촉→파벌 2→'왕좌를 지키겠는가'→영압 체인 3→헤롯→'지혜로운 마음' | 4~5일 |
| P0 | 첫 10분 보장 / 세계 손질 #68·#71·#72 | §3.2 행대로; 커맨드렛 `ToldLines` 첫 600틱 25~40 검증 / '#68만·#71만' 판을 각각 잰 뒤 적용 | 1일 / 2~3일 |
| P0 | 판의 끝 최소형 / 저장 최소형 | §3.2 임시 규칙+`BuildChronicle` 최소형 → 새 판 / 틱 경계 자동 저장(승천·역할 전환·10분·종료)·이어하기 슬롯 열거·가드·백업 1 | 2~3일 / 2~3일 |
| P1 | 결과를 돌려주는 지표 4 | Stability 60틱 추세, '다음 칸까지' 툴팁, 명령 단추 거절 사유('멈춤 — 자원 부족'), 위기 줄 카메라 포커스 — 표현층이라 세계 해시 불변 | 2일 |
| P1 | 통치자 둘째 축 | `SetResearchFocus` 4프리셋+DT_Edicts 3행(철야·안식·기계 권리)과 유지비 — '첫 60분 폐위율 20~35%'의 원천 | 3일 |
| P1 | 초월자 얼굴 / 온보딩 3줄 | trans_silence Title·Voice 3·Claim 3·EventWeightModifiers, ClaimState 표식 / 단축키 툴팁·패널 안내·첫 카드 3장이 조작을 설명 | 2일 |
| P1 | 방랑자 최소 루프 | TravelTo(20틱/레인)·Scan·조우 5장·BecomeRuler(ThroneLock 읽기) | 4~5일 |
| P2 | 사망·계승·공백기 / 서막 문안·고넬료 본문·옛 땅의 기억 / 재미 측정 | KillPlayer 해소→FLifeRecord→RunHeadless(Leap)→계승 추첨 / 카드 UI 뒤 텍스트 묶음 / 첫 600틱 결정 ≥15, P(나쁜 결말\|충실) ≥0.25 — `-PlayerPolicy` 인자 선행 | Stage B / B·C / Phase 5 |

## 5. 문서 통폐합 계획

### 5.1 중복 목록

| # | 무엇 | 어디 ↔ 어디 | 처분 |
|---|---|---|---|
| 1 | 불변 규칙 36줄 26KB | CLAUDE.md:13-48 ≡ guideline:4196-4231(바이트 동일) | 한 원본(규칙 ID)+포인터 |
| 2 | Phase 2 작업 기록 / '다음' 문단 | guideline:3922(65KB) ↔ WORKPLAN §3 :141-159(축자 80%) ↔ 부록 A / CLAUDE.md:278 ≈ WORKPLAN:200 | CHANGELOG / STATUS 한 곳 |
| 3 | §16 새 행·확정 표 / 편집 지침 E | rulings D·E ↔ guideline:4467-4490·본문(적용 완료) | rulings 아카이브 |
| 4 | 성역 규칙 / 년 토글·arch_seeker 괄호 / 추첨·틱 순서·핸들러 금지 | guideline 여러 곳 ↔ CLAUDE ↔ rulings ↔ WORKPLAN ↔ §14.3 | 10_DESIGN·20_ARCHITECTURE 한 곳, 현행/예정 열 |
| 5 | 판정 어휘 7낱말 / Hazard 수치 | schema ↔ CLAUDE:32 ↔ guideline:3398 / ini DevComment ↔ §4.1 ↔ JSON | schema·JSON 원본, 주석 제거 |
| 6 | 대도감 두 판(약 70% 동일) / 빌드 명령 두 벌 / 설계 낱말 사전 | code_artifact ↔ codex / CLAUDE:63-101 ↔ :105-129 / CLAUDE:48 ↔ §15 ↔ rulings:22-33 | 옛 판 아카이브 / 러너 스크립트 / §15 코드 사전 하나 |

### 5.2 상충 목록과 채택안

| # | A | B | 채택 |
|---|---|---|---|
| 1 | '-TellAll은 Phase 5'(guideline:3135) | 구현됨(:3277) | 구현 |
| 2 | §4.1 표 AI 각성·역병 결과 | 데이터 축소형(:652) | 표를 '현행/EA 목표' 두 열로 |
| 3 | VS Hazard 8에 아공간 침식(§4.1·§12.1) | WORKPLAN·CLAUDE 'Phase 4·6+' | Phase 4 위기 헤더에서 확정(#90) — 2단계는 '미결' 표지만 |
| 4 | #95 '세 파일'(rulings:658) / 저장 '은하당'(:22,4590) | '네 파일'(guideline:4488) / '판당'(:3488) | guideline / 판당 |
| 5 | 연대기 이관 Severity 1·2 전부(§8.4=코드) / 첫 생애 sol·arch_terran(코드·데이터·테스트) | #40 ⑦ 읽는 것만 / sys_0·arch_seeker(v0.8 본문) | ⑦ 채택, 코드는 '예정' 표지 / 현행·목표 두 열 |
| 6 | §10.1·§10.2 WBP 레이아웃·M_Lane·L_MainMenu | C++ 위젯·LineBatch·L_Galaxy | 코드 기준으로 §10 재작성 |
| 7 | 사자가 책을 '전달'(directive:21,30) / 첫 생애 행 `arch_default`(directive·codex) | '가리키기만'(CLAUDE:23) / `arch_seeker`(guideline·CLAUDE) | guideline/CLAUDE |
| 8 | '부르심'=서막(:327) / Severity 2='선택의 결과'(:3336) | '부르심'=첫 구간(:253) / 은혜 행·첫 줄 Severity 2 | 용어집에서 하나로 / '선택 또는 부름에 닿은 전환점' |
| 9 | 'EA 12~18개월'(:3998); 테스트 '12종'·용도 상수 '8'·SaveRestart 1,000틱 | VS 8개월+6~12개월; 76·9·3,000 | 일정 열 삭제; 상충 아님(날짜 박힌 기록) — CHANGELOG로 |

### 5.3 새 문서 체계(규범 문서 합계 ≤ 200KB; 원본 대응은 5.4)

| 새 문서 | 목차 핵심 |
|---|---|
| CLAUDE.md(≤10KB) / STATUS.md(1쪽) | 현행 불변 규칙 20줄·작업 방식·러너 호출·포인터 / 현재 단계·다음 3작업·열린 결정 Top 5·기준 해시·테스트 수 |
| docs/00_VISION.md | 피치·코어 루프·3기둥·두 모드·신학적 토대 1문단·톤·포지셔닝·코드 사전(§15 4열: 인게임/표준 영문/식별자/상태) |
| docs/10_DESIGN/{time,economy,civ,crisis,transcendent,ascension,calling,knowing,text,balance}.md | 절마다 '현행 규칙 \| 목표(Phase) \| 미결(Q-nn)' 3열, 규칙 ID, 테스트 이름 |
| docs/20_ARCHITECTURE.md | 층·Subsystem·FGalaxyState·틱 열 단계·난수 셋·해시·저장·데이터 파이프라인(헤더 사본은 코드 링크로) |
| docs/30_CONFIG_REFERENCE.md(생성) | 노브·값·범위·읽는 코드·상태(읽힘/예약 Phase N) — DT_Rules.json+schema.json에서 Tools 생성기가 만든다 |
| docs/40_DEV_GUIDE.md / 45_ENGINE_NOTES.md | 컨벤션·세션 절차·effort 기준·헤더→리뷰→재감사·빌드/테스트(플랫폼 중립)·테스트 카탈로그(등록 76종) / UE 5.8 헤더 경로 표 |
| docs/50_ROADMAP.md | 마일스톤→작업→상태(미착수/최소형/끝+날짜)·종료 조건·선행 결정 표 하나, 번호 체계 하나, Cut List, 리스크 |
| docs/60_DECISIONS/ADR-nnnn.md + OPEN_QUESTIONS.md + headers/ | 결정·근거·대안·영향; 열린 물음 Q-nn(옛 #N 괄호 1회); 승인 헤더 초안(Saved/Design에서 복사) |
| docs/70_CONTENT_GUIDE.md | 템플릿 문법(슬롯 카탈로그 — {Project}{Star}·\|hpp 포함)·화자 3층·사후 문장 ①~⑩·이름의 거름·ST_CA 표 |
| docs/90_LORE/ | codex 원문·원형 장부(치환표)·신학 노트(규칙 ID별 성경 근거) |
| docs/CHANGELOG.md / docs/archive/ | 날짜·커밋·바뀐 규칙 ID·리뷰 통계·실측 세대 / guideline_v0.8 동결본·v08_rulings·directive·code_artifact(git mv) |

### 5.4 원본→새 문서 매핑표(요지)

| 원본 | 행 | → |
|---|---|---|
| guideline 머리·이력·§0 / §1~§3 | 1-600 | CHANGELOG / 00_VISION; §3.2 사다리·§3.4 하늘 → 10_DESIGN/ascension·time |
| §4 / §5 | 600-1460 | 10_DESIGN/crisis·transcendent·calling·economy·civ; 성경 근거 → 90_LORE |
| §6·§7 / §8 / §9 | 1460-3731 | 20_ARCHITECTURE(규칙만, 헤더 사본 삭제) / 70_CONTENT_GUIDE(§8.1 엔진 규칙 → 20) / 20_ARCHITECTURE(저장·결정론); 테스트 표 → 40_DEV_GUIDE(등록 76종만, 미래 17행 → ROADMAP); 목표·실측 → balance·CHANGELOG |
| §10 / §11~§13 / §14·§15 / §16 | 3731-4598 | 40_DEV_GUIDE(코드 기준 재작성) / 50_ROADMAP(상태 줄 제외) / 40_DEV_GUIDE·00_VISION 부록 / OPEN_QUESTIONS(열린 58행→Q-nn)·ADR(확정 24블록) |
| CLAUDE.md / WORKPLAN.md / rulings / directive / codex 2판 | 전체 | 5.7 / §0-§2 → 40, §3 표 → 50, 기록·부록 A → CHANGELOG / A·D → ADR, B.2·C → 90_LORE, E → archive / ADR 뒤 archive / 90_LORE·archive |

### 5.5 문체·용어 규칙
- 한 줄 1문장·300자 이내, 괄호 1단, 번호 목록. 규칙마다 안정 ID `RULE-<영역>-<이름>`와 상태 `[구현]`/`[예정 P4]`/`[미결 Q-nn]`. 괄호 표지 `(v0.8 — 단계 N부터)`는 열로 바꾸고, v0.7 이전에 미뤄진 규칙(계승·도약·귀환·함대·철야)에도 같은 표지를 소급한다.
- 2중 표기: 첫 등장 "인게임 낱말(코드 식별자)", 10/20/30/40은 식별자 우선, 00/70은 인게임 낱말 우선. 두 뜻 낱말 고정: '칸'=Tier(프로젝트 슬롯은 '건설/대응 슬롯'), '부르심'=첫 구간, '문턱'=T5 카드, '정착률'→'주장 만료율'.
- 근거(성경 장절·채팅 인용·리뷰 통계)는 규칙 문장에 두지 않고 90_LORE·ADR·CHANGELOG로. 수치(테스트 수·태그 수·해시)는 문서에 적지 않고 생성 표·코드를 가리킨다.
- 절 번호는 한 번도 옮겨진 적이 없고 '옛 §17'·'옛 #N' 관행이 매핑표를 대신해 왔다 — 이 관행(끝에 덧붙이기·번호 재사용 금지)을 규칙으로 적고, 코드 주석의 `section N`(비테스트 574회·Tests 73·Config 38)은 SECTION_MAP 뒤 마지막 단계에 규칙 ID로 치환한다.

### 5.6 이관 절차와 검증

| 단계 | 일 | 검증 |
|---|---|---|
| 0 준비 | docs/SECTION_MAP.md(§N→파일#ID), 규칙 ID 부여 스크립트, Saved/Design 헤더 초안을 개발자 PC에서 복사 | §참조 전수(guideline 3,026·코드 647·Config 38) 매핑 누락 0 |
| 1 이력 분리 | :20-92·§0·§11 상태 줄·§16 블록·CLAUDE 일지·WORKPLAN 기록/부록 A → CHANGELOG; 확정 24블록 → ADR | 원문 글자 수 합 = 이관 후 합 |
| 2 드리프트 동기화 | §1.4 표: 낡은 수치·표·트리를 코드 기준으로 먼저 고치고 예정 건은 '목표' 열로 — 분할 전에 해야 옛 값이 번지지 않는다 | 테스트 76종 2회·check_data 통과 |
| 3 CLAUDE 다이어트 | 5.7 | 세션 1회 실측 |
| 4 본문 분할 | §→00/10/20/40/70, 현행/예정/미결 3열 | `(§N)` 전부 링크 치환, 깨진 링크 0, 문서 린트(줄 300자·괄호 1단·표지 누락) |
| 5 코드 주석 / 6 아카이브 | `section N`→규칙 ID / 옛 파일 → docs/archive(git mv) | 빌드·테스트 2회 / 루트에 CLAUDE·STATUS·README만 |

단계마다 커밋 1개(4단계는 절마다), 문서 변경이 `generated.sha1`에 닿지 않음을 `python -I Tools/check_data.py`로 확인한다.

### 5.7 CLAUDE.md 축소안(118KB → 약 10KB)

| 절 | 현재 | 처분 |
|---|---|---|
| 프로젝트 | 1.6KB | 0.5KB; `C:\…` 경로 6줄 → `$UE_ROOT`·저장소 상대 |
| 아키텍처 불변 규칙 | 26.4KB(v0.8 표지 11·v0.7 5·가칭 4) | 현행 규칙만 20줄 ≈4KB; 예정은 ROADMAP, 상세는 20_ARCHITECTURE 규칙 ID |
| 작업 방식 / 모듈·UHT | 2.3KB | 유지 |
| 빌드/실행/테스트 | 21.2KB | 러너 호출 5줄 ≈2KB; 전문 → 40_DEV_GUIDE |
| 검증된 엔진 헤더 경로 / 렌더 결정 | 31.3KB / 2.0KB | 45_ENGINE_NOTES / ADR-0001 |
| 현재 단계 | 33.6KB | STATUS.md 10줄+CHANGELOG |

## 6. 개발 가이드라인 초안(검사할 수 있는 한 문장만)

- **코드 컨벤션**: 소스 ASCII·주석 영문, 한국어는 DataTable/ST_CA에만. 함수 150줄 초과 금지. 태그·ST_CA 키·행 이름 리터럴 금지 — `CosmicRows::`/`CosmicTags::`/`CosmicLabels::` 상수. 호출자 오류는 `UE_LOG(LogCosmic, Error)` 한 줄+무동작, `ensure`/`check` 금지(현 `Install`의 ensureMsgf는 위반).
- **모듈·의존**: `Core ← Data ← Sim ← {Events, Ascension, Log, Player, Save} ← {World, UI}`. 역방향 include 금지(`Tools/check_layers.py`). Sim↔Events의 .cpp 상호 호출은 '한 길' 설계로 허용하되 `FCosmicEvalContext`는 독립 헤더로 내린다.
- **상태·저장·결정론**: 런타임 상태는 FGalaxyState 안 USTRUCT만. 저장 필드 초기값 불변·열거자 끝에만·`SaveVersion` 인상은 마이그레이션+테스트 동반. 세계 추첨은 `GetRng()`만, 행 보기 전 고정 횟수, `FMath::Rand*` 0건. 해시 산술은 `+−×÷√`만. 시드 42 기준값은 `Tools/baselines/`에 파일로. 표현 상태는 Controller/ini.
- **데이터·설정**: 원본은 Tools/data, .uasset은 산출물. 컬럼·열거자 추가 = schema.json+test_data_schema 경계 케이스. 밸런스 수치는 DT_Rules 또는 행 컬럼, cpp constexpr는 `[law]`만. 노브는 읽는 코드가 있을 때만 추가, 예약은 `since: "Phase N"`.
- **테스트**: 필수 = 결정론·저장 왕복·데이터 무결성·세계 규칙. 새 단언은 변이로 한 번 깨뜨리고 `Tools/mutations/`에 패치를 남긴다. smoke/full 티어 분리(A4 제안)보다 '같은 시드 42의 20,000틱 판 합치기'와 단계 4a/4b/5 재측정 목록(FirstLife·PhaseTwoExit·GenerationRules·LayersInRange·Feats.Reachable·RealmCap)을 우선한다.
- **커밋·문서·세션**: 커밋 = 컴파일+테스트 2회, 제목 접두어 `[P3-05]`, 표에는 날짜만(해시 기록 커밋 폐지). 설계 변경은 10_DESIGN 먼저, 코드 둘째, STATUS 셋째. 헤더 승인→단독 구현→적대 리뷰→변경 범위 재감사 골격 유지하되 헤더 승인은 저장 필드·열거형·틱 단계·결정론 코드에만. ultracode 켬 = 세이브·해시 변경 / 틱 단계 신설 / 텍스트 ≥20문장 / 단계 종료 리뷰; Agent 도구 병렬 리뷰를 동등 수단으로 인정. 주 1회 '플레이만 하는 날'.
- **환경**: 경로 변수화(`$UE_ROOT`·`$PROJECT_ROOT`), `Tools/run_checks.(ps1|sh) -Tier smoke|full`(데이터 검사→레이어 린트→빌드→테스트 2회→nullrhi 스모크→커맨드렛 기준값 비교).

## 7. 로드맵

| 단계 | 작업 묶음 | 종료 조건 | 작업일 | 의존·리스크 |
|---|---|---|---|---|
| **0. 정리** | 손 확인(창에서 마우스만 T3·stat fps — 임시 빌드로 즉시), 문서 2단계(5.6), 러너·레이어 린트, 헤더 순환 해소 | Phase 3 종료 선언, 규범 문서 ≤200KB | 4~6 | 손 확인 1건이 꼬리·예약·Phase 4를 직렬로 막는다 |
| **A0. v0.8 예약 ①** | 4a 성역(sol 배열 끝·sys_0·Law.Hedge·IsSanctuary)+4b-lite 첫 생애 행 뼈대(arch_seeker·첫 줄·두 목소리) — 리베이스 한 번 | Sim.SanctuaryKept, 고칠 테스트 ~40곳 재측정, 개발 세이브 삭제 | 3~5 | ID·시드 기준값이 바뀌므로 MVP 작업보다 앞 |
| **A. Playable MVP** | §4 P0 여섯+P1 지표 4·온보딩+Win64 패키징 1회 | ① 새 판→저장→재시작→이어하기 ② 커맨드렛 30판 첫 600틱 말해진 줄 ≥20·결정 ≥15 ③ 마우스만 ×10으로 T3→연대기 화면 ④ 쿠킹 빌드 시드 42 해시 = 에디터 ⑤ 외부 1명 30분 "다음에 뭘 할지 알겠다" | 12~18 | DT_Encounters 스키마 선행; 폐위 '실패 끝'은 칙령 유지비 뒤 |
| **B. 코어 시뮬 완성** | B1 방랑자 루프 → B2 연구 중점·칙령 9 → B3 외교·분쟁 → B4 초월자 목소리·교리 3행·영압 카드·영적 시야 → B5 사망·계승·공백기·두 책(4b) → B6 T3→T5 업적·엔딩 4 → B7 AI 백성 5행·AIProfile 행동·고넬료 사슬 → B8 서막 문안·실제 하늘(단계 5, 늦어도 Phase 5 전) | §12.1 수량+§9.3 목표 ±30%+계승 1회 콘솔 없이 | 35~50 | 텍스트 100장+는 코드 속도가 아님; 실제 하늘은 모든 시드 기준값을 다시 바꿈 |
| **C. 폴리싱** | Niagara·튜토리얼·영어 로컬·'세계관과 출처'·옛 땅의 기억·Enhanced Input·UI 테마·Steam 페이지·테스터 5~10 | 첫 10분 이탈 <10%, '설교' ≤1/세션 | 20~30+대기 | 테스터 모집이 외부 의존 |

**당길 것 / 미룰 것**

| 항목 | 현 계획 | 판정 | 이유 |
|---|---|---|---|
| 저장 최소형 / #68·#71·#72 | 단계 6 앞 / Phase 4 헤더 | **MVP** | 호출자 0·1MB 초과 / 세계가 고장 나 보임 |
| 4a 성역+4b-lite | v0.8 예약 ① | **MVP 앞(A0)** | ID·기준값을 두 번 바꾸지 않기. 반대 의견(A6): 카드 화면 뒤 — 그러면 Phase 4 테스트까지 재기준 |
| 실제 하늘 ②·DT_Planets | 단계 5(8~11일) | **B8** | 플레이어 동사 0; 문서도 '늦어도 Phase 5 전' 허용. 라이선스 확인만 지금 |
| 서막 8장 / 고넬료 사슬 | Phase 4 첫 블록 / 단계 7 | **카드 UI는 MVP, 문안은 B8 / B7** | 단계 6 전엔 '지혜로운 마음'이 첫 청원 / 코어이나 방문 위기·사자 카드 뒤 |
| 꼬리 7 영적 시야 / 9 우주 화면 / 건넘 6a | Phase 3 꼬리 / EA 빌드 | **B4 / C / EA 이후 재심** | 9번은 종료 조건과 무관; 6a는 §12.3 (다) 스스로 VS 소비 불가 인정 |

일정 단위는 '주'가 아니라 **작업일(세션)**로 적는다 — §11의 P0~P3 계획 약 13~14주(주 25~30h 기준)가 9일에 끝났다.

## 8. 리스크와 Cut/Defer 제안

| 분류 | 리스크 | 완화 |
|---|---|---|
| 범위 | Phase 4가 276회 언급되는 체크리스트로 팽창; §16 열린 행 58 | B1~B8 블록마다 종료 조건; MVP 차단 미결만 Top 5 |
| 기술 | 기준값 리베이스 2회(A0, B8), 쿠킹 해시 미검증, ×25 clamp, Windows 전용 명령 | 리베이스는 두 번으로 고정; A 종료 조건 ④; 경로 변수화 |
| 밸런스 | 회복 규칙을 넣으면 입력 없는 판의 폐위가 사라질 수 있음 | '#68만' 판 선측정, 칙령 유지비와 함께 폐위율 20~35% 재조준 |
| 콘텐츠·IP / 시장 | 대도감 차용 고유명·'영혼' 어휘가 사람 검토에만 걸림 / 한국어 우선 텍스트 4X의 좁은 시장, 신학 프레임의 양방향 리뷰 | 원형 장부 `archetype_ledger.json`+스키마 `other_works`·`holy_and_myth` 낱말 경계 검사 / '미선언·미은폐' 유지, 영어 어휘 대응표(개역한글↔WEB)를 Phase 5 전에 |
| 번아웃 | 8일 99커밋, 리뷰→재감사 3중 루프, 문서가 코드보다 큼 | 1세션 1행, 해시 커밋 폐지, 문서 ≤200KB, MVP를 먼저 쥐어 보상 확보 |

**Cut(VS 표에서 명시적으로 제외)**: 네 길 중 3길·교리 3행·독립 초월자·모티프 24종(이미 Phase 6+), Niagara 승천 연출(C++ 오버레이로 대체 확정), 수동 저장 메뉴(아이언맨 유지). **Defer**: 건넘 6a(EA 이후 재심), 실제 하늘(B8), 서막 전문·헌정명 뱅크·옛 땅의 기억 화면(B8/C), DT_Planets·Tests 별도 모듈·Enhanced Input(C).

## 9. 2단계 착수 전 개발자가 확정할 결정

1. **문서 체계** — 권장: 5.3의 docs/ 체계+STATUS.md+CHANGELOG, 옛 문서는 `docs/archive`로 git mv(이력 보존과 SSoT 분리를 동시에).
2. **규칙 ID·상태 표지** — 권장: `RULE-<영역>-<이름>`+`[구현]/[예정 PN]/[미결 Q-nn]`, 절 번호는 끝에만 덧붙이는 관행을 규칙화(코드 주석 647회가 §를 가리킨다).
3. **CLAUDE.md 목표 크기** — 권장: ≤10KB, 불변 규칙은 현행 20줄만(세션마다 118KB 로드가 드리프트의 1차 원인).
4. **MVP 정의** — 권장: 통치자 단일 모드·첫 생애 한 번·×10 기준 30~60분·끝 = T3 ∨ 틱 6,000 → 연대기 최소형; 실패 끝은 칙령 유지비와 함께 B2(현 규칙으로 60분 안 폐위는 사실상 0).
5. **v0.8 예약 ①의 시점** — 권장: MVP 작업 전 A0에서 한 번(지시서 순서 유지). 대안: 카드 화면 뒤. 이유: ID·시드·테스트 고정물 ~40곳을 한 번만 바꾼다.
6. **실제 하늘(단계 5)의 시점** — 권장: Phase 4 첫 헤더 뒤, Phase 5 전(B8); 자료 라이선스 확인만 지금(플레이어 동사를 더하지 않으면서 모든 기준값을 바꾼다).
7. **첫 10분 보장 방식** — 권장: 모든 백성 공통 Standing 행+SpawnHazard(또는 `Placed`), 플레이어 전용 노브 없음(§16 #91과 완화 ⑥ '수치 이점 0').
8. **자동 일시정지 기본값** — 권장: 기본 켬(Severity 2·자기 성계 위기·카드), 설정 토글(×10에서 결정 자체가 불가).
9. **가칭 식별자 확정** — `arch_seeker`·`Law.Hedge`·`IsSanctuary`·`ReceiveCalling`·`enc_sanctuary_bound`·`event_word_arrived` 등. 권장: 4a/4b 헤더 전에 일괄 확정(세이브에 남아 배포 뒤 불변).
10. **밸런스 상수의 데이터 이동** — 권장: 경제·연구 상수 9개를 DT_Rules로(StarClassEnergy는 단계 5까지 C++), InfluencePerPopulation 1.0('잠정 코드 상수'를 Phase 5 전에 만질 수 있게).
11. **T2→T3 둘째 업적** — 권장: `feat_three_worlds`(정착 3 ∧ 공학 0.5)+전조 확률 1e-4(셋째 칸이 무작위 위기 하나에 묶이지 않게).
12. **연대기 이관 기준과 세이브 예산** — 권장: #40 ⑦ 기본값+Chronicle 상한 노브 1,500, 1MB 예산 유지(VS 1.8MB 실측).
13. **커밋 규약** — 권장: 표에는 날짜만, 제목 접두어 `[P3-05]`, 해시 기록 커밋 폐지(99커밋 중 32개가 1~2줄 해시 커밋).
14. **루트 md 셋과 code_artifact.md(§16 #95)** — 권장: codex는 `docs/90_LORE`로, code_artifact는 archive로, directive는 ADR 흡수 뒤 archive(옛 판이 v0.8 정경과 충돌한 채 루트에 있어 오참조 위험).
---

## 부록 A. 메인 세션이 직접 확인한 수치(에이전트 보고와 별개로 재측정)

| 항목 | 값 | 확인 방법 |
|---|---|---|
| guideline.md 크기 추이 | 69KB(10-10) → 176KB(10-11) → 579KB(10-12) → 1,099KB(10-18) — 8일 만에 16배 | `git show <commit>:guideline.md \| wc -c` |
| CLAUDE.md 크기 추이 | 18.6KB(10-10) → 48.7KB(10-12) → 118KB(10-18) | 같은 방법 |
| 가장 긴 줄 | guideline.md:3922 = 65,164B(Phase 2 상태 줄), :3582 = 16,722B(§9.3 테스트 표 한 셀); 2,000B 초과 줄 94개 | `awk length` |
| 커밋 구성 | 99커밋 중 '문서/guideline/docs:' 접두 53, "커밋 해시를 적는다" 32, '리뷰 반영/재감사 반영' 37; 하루 최대 22커밋(10-16) | `git log` |
| 파일별 누적 변경량 1위 | guideline.md 11,238줄(2위 CosmicHazardTests.cpp 2,997줄) | `git log --numstat` |
| 레이어 규칙 | Core·Data·Sim·Events·Ascension·Log·Player·Save에 UI/Actor/Widget/Pawn/HUD/PlayerController/GameMode include 0건 | grep |
| 비테스트 코드의 'Phase N' 주석 | Phase 4 26·6a 16·3 15·4b 9·2 4 — 코드가 스스로 '나중'을 75곳에 적어 둠 | grep |
| 코드 주석의 guideline 절 참조 | 'section N' 토큰 비테스트 574·Tests 73·Tools 8(+Config 38) — 절 번호 재편 시 전부 영향 | grep |
| GameplayTag | 140개(Trait 36·Revelation 12·Log 10·Hazard 9·Event 9·Star 7·Doctrine 7·Notable 6…) | `Config/DefaultGameplayTags.ini` |
| 데이터 행 | Events 11·Hazards 5·Megaprojects 5·Feats 2·Civs 2·Transcendents 1·LogTemplates 10·Galaxies 6·Rules 1(노브 44)·ST_CA 37키 | `Tools/data/*.json` |
| 빈 틱 단계 | `TickDiplomacy`·`TickConflict` 본문 = 주석 1줄 | GalaxySimulationSubsystem.cpp:1585-1593 |
| 플레이어 명령 | `SetResearchFocus`·`ResolveEncounter`·`EnactEdict`·`TravelTo`·`OfferReturn`·`IsSanctuary`·`Ending`·`AutoPause` 비테스트 코드 0건; `ResearchFocus`는 생성 시 0.25 고정(:752) 뒤 읽기만(:1554) | grep |
| 배속 상한 | `SetTimeScale`이 `MaxTicksPerFrame`(10)으로 clamp(:1036, .h:494) — 설계의 ×25 불가 | 코드 |
| 저장 호출 | `SaveCurrentGalaxy`/`LoadGalaxy`의 Tests·Save 밖 참조 = 주석 2줄뿐 | grep |
| 안정도 상승 규칙 | Sim 안에 Stability를 올리는 코드 없음(이벤트 효과 +0.03/+0.2뿐) | grep |
| DT_Encounters | schema.json에 struct `EncounterDef`만 있고 `columns: null`, JSON·에셋 없음 | schema.json |
| 성경 장절 패턴 | guideline 약 395회·rulings 약 399회 / 게임 데이터 0회('하나님' 29·'성경' 36·'창 N' 34·'행 N' 29회 등) | grep |

## 부록 B. 검증 통계와 산출물

- 적대 검증 48건: **확정 15**(A1-2·A1-3·A1-6·A2-1·A2-2·A2-3·A2-4·A2-6·A3-1·A4-3·A4-4·A5-4·A6-2·A6-3·A6-8), **부분 확정 33**(정정문 반영), **반박 0**. 본문에서 단정문으로 쓴 수치는 확정 또는 메인 세션 재측정 항목이다.
- 주요 정정(원 주장 → 정정): Sim↔Events '헤더 순환' → .cpp 수준 상호 의존이지 헤더 순환은 아님 / 'CLAUDE·guideline·WORKPLAN 3중 기록' → 축자 중복의 실체는 guideline:3922 ↔ WORKPLAN §3(80%)이고 CLAUDE.md '현재 단계'는 10-17에 이미 요약층으로 줄어듦 / '수치 상충 다수' → 규범 문장의 진짜 상충은 '-TellAll은 Phase 5' 하나, 나머지는 날짜가 박힌 기록 / 'KillPlayer가 기록만 남음' → 현 데이터·스키마상 KillPlayer가 기록되는 경로 자체가 없음(파이프라인은 Phase 4b) / '20,000틱 테스트 12건' → 7건(+LayersInRange) / '4a 성역을 MVP 뒤로'(A5·A6) → ID·시드 기준값·테스트 고정물 ~40곳을 두 번 바꾸므로 MVP 앞(A0)이 낫다는 쪽을 채택(결정 #5에서 개발자 확정).
- 산출물(저장소 밖): `/home/mechtro/Desktop/bbb/_analysis_2026-10-18/` — `lenses/lens_A1~A6.md`(관점 분석 전문·권장안·주장), `digests/digest_*.json`(읽기 24건의 구조화 요약: 결정·수치·공백·중복·상충·구현 상태), `verify/verified_claims.json`(48건 판정·정정문), `report_synthesized.md`(종합 초안 원문).
- 자원: 서브에이전트 토큰 1차 12.79M(세션 한도로 1.77M 손실) + 재개 8.26M = 약 21.1M, 에이전트 128회 실행(캐시 재사용 30건 포함), 도구 호출 약 2,050회, 벽시계 약 110분.
