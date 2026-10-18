# 아키텍처: 모듈 층·Subsystem·상태·틱·난수·해시·저장·성능 (옛 §6·§9.1·§9.2·§9.4 + 불변 규칙 전문)

> 원문: guideline.md v0.8 §6, §9.1, §9.2, §9.4 및 옛 CLAUDE.md 불변 규칙 (2026-10-18 docs/ 재편으로 이관, 내용 무변경). 표지 `(v0.8 — 단계 N부터)`·`(v0.7 — …)`·`Phase N`이 붙은 문장은 **예정**(아직 코드에 없음), 표지 없는 문장은 **현행 규칙**이다. 열린 물음은 `docs/60_DECISIONS/OPEN_QUESTIONS.md`, 확정 기록은 `docs/60_DECISIONS/DECISIONS_LOG.md`. 절 번호(§N)는 원문 그대로 두었다 — 위치는 `docs/SECTION_MAP.md`.

## 6. UE5 기술 아키텍처

### 6.1 모듈 계층 구조

```

[ Presentation Layer (UI & Visuals) ]

├── UMG 위젯 (은하 지도 HUD, 성계 정보 패널, 은하 패널, 로그 피드, 조우 카드, 상단 자원/시간 바)

├── Minimal 3D Viewport (성계 노드 액터, 레인 라인, 성운 배경 셰이더, 배경 별·풍경 은하(그림뿐))

└── Niagara FX (승천 연출, 차원 균열, 초월자 오라) — 3종만

▲ (델리게이트 구독만. UI는 시뮬 상태를 읽기만 하고 명령은 Subsystem 함수 호출)

[ Core Logic Layer (GameInstance Subsystems) ]

├── UGalaxySimulationSubsystem 성계/문명 고정 스텝 틱, 경제·연구·외교·분쟁·위기·AI·건넘·앎 (§7.4)·건넘·앎

├── UCosmicEventSubsystem 조건 평가, 가중치 추첨, 효과 적용, 쿨다운

├── UAscensionSubsystem 업적 조건 감시, 등급 상승, 권능 부여, 승천 연출 트리거

├── UCosmicLogSubsystem 템플릿 치환(PickTemplateRow: 계시 양식 Context, 역할 슬롯·{PopBand} 해석과 집단 폴백), 로그 큐, 이야기의 범위(§8.1), 연대기 집계(생애별 장 + 두 책 장)

├── UPlayerRoleSubsystem 통치자/방랑자 전환, 이동, 조우·청원 진행, 명령 API, 사망·폐위·공백기·계승 파이프라인, 귀환 제안 훅 OfferReturn (§4.4, §5.10)

└── UCosmicSaveSubsystem FGalaxyState ↔ USaveGame 직렬화, 판당 슬롯 1개 (§9.1)

▲

[ Data Layer ]

├── 정의(Definition, 읽기 전용, DataTable): DT_Traits, DT_Events, DT_Feats, DT_Encounters, DT_Relics, DT_Edicts, DT_Megaprojects, DT_Hazards, DT_LogTemplates, DT_Civs, DT_Transcendents, DT_Dedications, DT_Galaxies, DT_Rules

├── 상태(State, 런타임, 저장 대상, USTRUCT): FGalaxyState { Systems, Civs, Transcendents, Player, Calling, Knowing, Transits, Log, Chronicle, Flags, RngState, TellRngState, NextCivSerial, GalaxyID }

└── 문자열: String Table ST_CA (UI), DataTable의 FText 컬럼 (서사)

```

**원칙**

시뮬레이션 코드는 UI와 월드(Actor)를 모른다. Core Logic Layer 폴더(`Sim/`, `Events/`, `Ascension/`, `Log/`, `Player/`, `Save/`) 안의 코드가 `UUserWidget`이나 `AActor`를 include하면 설계 위반이다. 이 규칙은 직접 include와 그 타입의 사용에 대한 것이다 — `Save/`가 `SaveGameToSlot` 계열(§9.1) 때문에 include하는 `Kismet/GameplayStatics.h`가 끌고 오는 Actor 헤더는 허용되는 전이 경우다.
UI는 Subsystem의 델리게이트를 구독하고 상태를 읽는다. 상태를 직접 수정하지 않는다.
뷰포트의 성계 액터는 상태의 '시각 표현'일 뿐이다. 액터가 없어도 시뮬은 돌아야 한다 (헤드리스 테스트의 전제).
모든 Core Logic Subsystem의 델리게이트 Broadcast는 전역 방송 게이트(`UGalaxySimulationSubsystem::IsBroadcastEnabled()`)를 본다. 공백기 도약(§4.4)과 헤드리스 실행 중에는 게이트가 닫힌다.
Core Logic Subsystem은 플레이어·생애에 종속된 멤버 변수를 갖지 않는다. 지속 업적 진척(`CompletedMegaprojects`, `SealedTick` 등)은 `FGalaxyState` 안에 둔다(§4.4).

### 6.2 Subsystem 선택 근거

모두 `UGameInstanceSubsystem`. 레벨 전환(메뉴 ↔ 은하 맵)에도 상태가 유지된다.
틱은 `FTickableGameObject`를 함께 상속해 받는다. 월드 타이머에 의존하지 않는다.
상태는 Subsystem이 들고 있는 `FGalaxyState` 구조체 하나에 모은다(판의 상태 = 고향 은하 상세 + 판 공통, §7.2). UObject 그래프로 만들지 않는다. 저장이 단순해지고 결정론 검증(해시)이 가능하다.
판 상태는 세 층(세계 → 앎 → 이야기)이고 의존은 한 방향이다: 세계는 앎을 `NoteMet`·`NoteKnown`으로 쓰기만 하고 읽지 않는다(전문과 예외 하나는 §8.1).

### 6.3 UI 기술 선택: UMG 우선

초안은 CommonUI였다. CommonUI의 강점(입력 라우팅, 플랫폼별 액션 바, 패드 포커스)은 PC 싱글 텍스트 게임에서는 거의 쓰이지 않고, 학습 비용과 셋업 비용이 있다.
**결정: UMG + C++ 위젯 베이스 클래스**로 시작한다. 스팀덱/패드 지원을 목표에 넣는 시점에 CommonUI 전환을 재검토한다 (§16).
로그 피드는 `UListView`로 가상화한다. 수천 줄을 Text Block으로 쌓지 않는다.
화면의 패널(Phase 3 3번 작업, WBP 없는 C++ 위젯): 상단 바 `UCosmicTopBarWidget`(통치하는 백성의 이름·칸·안정도, 자원 다섯은 정수, 연구 넷은 비율, 주기, 배속 단추 다섯 — 정지·×1·×2·×5·×10, 지금 배속은 흐리게), 오른쪽의 성계 정보 패널 `UCosmicSystemPanelWidget`(선택한 성계가 있을 때만: 이름, 항성 등급, 주인 — 또는 '주인 없음'·'파괴된 성계' —, 위기와 남은 주기, 건설·대응 칸의 진척/필요 진척, 그리고 통치자의 명령). 명령 단추는 통치하는 백성의 정착된 성계에서 `CosmicMegaprojects::CanStart`가 받는 행마다 하나(행 이름 사전순)와, 통치하는 백성의 것이 아닌 살아 있는 성계마다 정착 단추 하나다(`SettleBlockOf`가 거절하면 회색 단추 + 사유 툴팁 + 아래 한 줄, §5.12) — 눌림은 컨트롤러(`StartProject`)를 거쳐 `UPlayerRoleSubsystem::StartMegaproject`로 가고, 컨트롤러는 HUD와 지도를 바로 다시 그린다(멈춘 시뮬은 틱을 보내지 않는다). 프로토타입 키 P는 이 단추로 대신하고 뺐다. 그릇이 육신을 가진 통치자가 아니면 단추가 없다(방랑자의 패널은 Phase 4). 단추 목록은 행동의 목록이 바뀔 때만 다시 짓고, 그렇지 않으면 단추마다 제자리에서 이름·켜짐·툴팁을 고친다(툴팁은 문구가 바뀔 때만 — `SetToolTipText`는 새 Slate 툴팁 객체를 만들어 열린 것을 닫는다): 클릭은 누름과 뗌에 걸쳐 있어서 그 사이에 단추를 다시 지으면 클릭이 버려지고, 시뮬은 1초에 열 번까지 틱한다(v0.8의 단계 0에서 고친 버그 — 그 전에는 틱마다 다시 지었다). 꺼진 단추는 코드의 `Press`도 받지 않는다. 아키타입 이름은 보이지 않는다(백성의 이름 `CivName`만 — §16 #40 ④를 기본값대로 닫았다: `FCivArchetypeDef.DisplayName`은 개발자용 라벨이다). 단추는 키보드 포커스를 받지 않는다(눌린 단추가 포커스를 쥐면 Space가 정지 대신 그 단추를 누른다). 패널의 상자를 누른 것은 지도의 클릭이 아니다(위젯이 그 누름을 처리한다 — 처리되지 않은 누름은 뷰포트로 올라간다). 로그 피드(4번 작업) `UCosmicLogFeedWidget`: 왼쪽 아래의 `UListView`(가상화), 말해진 항목만(§8.1), 마지막 500줄(§9.4 — 넘치면 가장 오래된 줄이 빠진다), "[주기] 문장", Severity 색(0 회색·1 흰색·2 금색, 초월자의 1인칭 `Log.Transcendent`과 주장 `Log.Claim`은 보라 — §8.2의 넷), 끝을 보고 있었으면 새 줄을 따라간다. 줄을 누르면 그 줄의 성계를 — 없으면 그 줄의 백성의 수도를 — 고르고 카메라가 간다(별을 더블 클릭한 것과 같다, 컨트롤러의 `SelectAndFocus`). 에디터 바이너리의 리스트 뷰는 블루프린트 항목 클래스만 받으므로(`UListViewBase`, WITH_EDITOR) 빈 위젯 블루프린트 `/Game/CA/UI/WBP_LogEntry`(부모 C++ `UCosmicLogEntryWidget`)를 `Tools/make_widget_blueprints.py`가 만든다(트리는 C++이 짓는다). Phase 1 텍스트 패널은 지웠다. 승천 연출(5번 작업, §6.5) `UCosmicAscensionOverlayWidget`: `UAscensionSubsystem::OnTierAscended`(플레이어의 승천)를 듣고 화면 전체에 약 5초(실제 시간 — 0.8초에 밝아지고 4.2초부터 어두워진다) 보인 뒤 닫히고, 누르면 바로 닫힌다. 보이는 동안 움직이던 시뮬을 멈추고(컨트롤러의 `SetSpeed(0)` — 핸들러는 `SetTimeScale`을 부를 수 있다, §9.4) 닫힐 때 그 배속을 돌려준다. 그 사이 플레이어가 배속을 바꿨으면 그대로 둔다. 보이는 동안의 둘째 승천은 글을 바꾸고 5초를 다시 센다. 개발용 콘솔 명령 `CosmicPreviewAscension <초>`는 그만큼 뒤에 연출을 보인다(창 없이 찍는 화면에 쓴다).
**성역의 패널**(v0.8 — 단계 4부터. 성역은 태양계 `sol` — 다시 쓸 수 없는 법 하나로 소유·정착·주장·파괴에서 빠지는 곳이다, §4.5 ①): 이름 '태양계 · 성역', 주인 줄 "성역 — 어느 백성의 소유도 되지 않는다", 생명 낱말 대신 "이곳은 지켜지고 있다.", 백성마다 부르는 이름 줄은 세계 사실을 읽는다(§8.2 '성역의 이름'), 명령 단추 없음, 정착 단추의 사유는 성역 사유가 맨 앞. 행성 목록에는 '지구'라는 낱말이 선다(단계 5).
**시계 토글**(v0.8 단계 2, 2026-10-18 — WORKPLAN §3 Phase 3 표 11번): 상단 바 시계를 눌러 '주기 1,204'↔'1,204년'(툴팁 "첫 기록부터 센 해 · 1주기 = 1년" 아래 "누르면 주기와 해가 바뀐다." — 납작한 단추라 누를 수 있음을 툴팁이 알린다), 마우스를 올리면 두 단위를 함께 '주기 1,204 · 1,204년(첫 기록부터)'. 피드 머리와 패널 카운트다운('120년 남음')도 따르고, 년 모드의 배속 단추는 '1년/초 … 10년/초'. 보는 사람의 설정이라 상태·해시 밖이다. 기본은 '주기'다(§16 #82). 단위는 플레이어 컨트롤러의 표현 상태(`ECosmicTimeUnit`)이고 보는 사람의 `GameUserSettings` ini에 남는다 — 자동화 테스트가 도는 동안에는 쓰지 않고, 테스트는 단위를 메모리에서 정해 개발자의 설정을 답으로 읽지 않는다. 라벨 함수는 단위를 인자로 받는다(`CosmicUI::ClockLabel`·`TimeLeftLabel`·`StampLabel`·`SpeedLabel`).
**셋째 칸의 ×25와 명령의 사유**(v0.8 — Phase 3 꼬리부터): 승천 연출 뒤 상단 바에 ×25 단추 하나. 거대 프로젝트 단추도 칸(Tier)·비용·건설 칸이나 대응 칸 사용 중을 회색 단추와 툴팁으로, 값을 못 낸 칸에 '멈춤 — 자원 부족'(`CanStart` 곁의 사유 열거형 — 헤더 승인). 상단 바 툴팁 '다음 칸까지: 다이슨 스웜(완성) · 물리 50%(34%)' — 다음 업적 행의 조건을 읽는 표현 계층의 순수 함수.
**'옛 땅의 기억'**(v0.8 — Phase 4 단계 7부터): 말이 도착한 백성에게 열리는 한 쪽(§5.11). 화면은 그 사건이 남긴 세계 사실을 읽는다 — 앎을 읽지 않는다(§16 #77).
**서막과 부르심의 화면**(v0.8 — 단계 6부터): 서막은 표현 계층의 상태다 — 저장하지 않고, 시뮬은 `SetTimeScale(0)`(일시정지 규칙 그대로)이며 배속 입력이 없다. 카드는 Phase 4의 조우 카드 화면을 먼저 만들어 쓴다. 부르심에는 승천 오버레이를 쓰지 않는다 — 카메라가 출생 성계에서 넓은 하늘로 물러나고 상단 바·피드·패널이 켜진 뒤 ×1이다. 서막의 결과는 부르심의 Core 함수(가칭 `ReceiveCalling`, §7.5)가 상태에 적는다(§4.4).

### 6.4 3D 뷰포트 (최소)

`ACosmicStarSystemActor`: 스피어 메시 1개 + 이미시브 머티리얼(항성 색/크기 파라미터) + 선택 콜리전. 상세 액터는 150개 이하다(아는 성계 가운데 화면에 든 것. VS는 생명 성계 60개를 전부 액터로 그리고 `Known`으로 가른다. EA의 300은 6a에 나머지를 인스턴스 메시로 옮긴다). 배경 별과 풍경 은하는 인스턴스 또는 하늘 머티리얼로 드로 각 1이다(선택 없음, Niagara 3종 예산 밖, 새 Niagara 없음).
레인: `ULineBatchComponent` 또는 얇은 스플라인 메시. 수직 슬라이스에서는 라인 배치로 충분.
카메라: 스프링 암을 가진 폰 `ACosmicCameraPawn`(Phase 3 2번 작업 — 게임 모드가 만들어 조종한다. `L_Galaxy`에 PlayerStart가 없어 기본 폰으로 두지 않는다). 오른쪽 드래그로 궤도 회전(위아래 −85°~−20°), 휠로 줌(한 칸 ×0.85, 1,500부터 원반 반지름의 3배까지), 왼쪽 드래그로 이동(지난 프레임에 커서 밑에 있던 원반의 점이 지금 커서 밑에 오도록 — 6픽셀 넘게 움직인 누름은 클릭이 아니다), 별을 더블 클릭하면 그 별로 다가간다(거리 3,000). 조작은 목표값을 정하고 폰이 실제 시간으로 그쪽에 미끄러진다(시뮬의 정지와 무관 — 이동만 바로 옮긴다). 새 은하와 로드는 원반 전체를 보는 시점으로 바로 간다(맵 액터의 `OnRebuilt`). 클릭은 라인 트레이스가 아니라 화면 공간에서 고른다: 커서에서 24픽셀 또는 그 별이 화면에 그려진 반지름 안에 있는 별 가운데 가장 가까운 것(`CosmicMapPick::NearestOnScreen` — 개관 거리에서 별은 약 10픽셀이라 구에 대는 트레이스는 정확히 찍어야 했다. 별의 충돌은 껐다). 빈 곳을 누르면 선택이 풀린다. 선택은 표현 상태다: 플레이어 컨트롤러가 들고(`SelectSystem`·`OnSelectionChanged` — 3번 작업의 정보 패널이 듣는다) `FGalaxyState` 밖이라 저장·해시와 무관하며, 지도는 선택한 별을 주인 고리 바깥의 흰 고리로 보인다. 멀리서(카메라 거리 9,000 초과)는 주인이 있거나 위기가 선 성계와 선택한 성계의 이름표만 보인다(`CosmicMapLook::ShowsLabel`). 개발용 콘솔 명령 `CosmicSelect <성계ID>`는 더블 클릭처럼 고르고 다가간다(창 없이 찍는 화면에 쓴다). 줌 단계는 성계(서막)·구역·은하·우주다(v0.8 — 우주는 Phase 3 9번 작업, 성계(서막)는 단계 6부터. 지금은 휠 줌 하나다). 실제 하늘(단계 5) 뒤 지도는 태양 중심 방위 등거리 투영이다 — 방향 = 은하 경도, 태양까지의 거리 = 실제 3D 거리. 좌표와 레인(교차 없는 평면 그래프, 차수 ≤ 4, 손 고침 파일, 태양을 빼도 이어진다)은 오프라인 도구가 미리 계산하므로 C++에 삼각 함수가 없다(§9.2). 실제 하늘 뒤에는 행성이 알려지지 않은 유명한 별(시리우스·프로키온·알타이르·베가·포말하우트 등)을 고를 수 없는 배경 점으로 두고, 태양 둘레에 10·20·30광년 금을 숫자 없이 그린다 — 출처가 깨끗한 공개 목록에서 오고, 표현 계층만이라 해시 밖이다(v0.8 — 단계 5부터).
배경: 스카이스피어에 성운 노이즈 머티리얼 1장. 배경 별과 풍경 은하는 위 `ACosmicStarSystemActor` 문장의 규칙대로 따로 그린다(드로 각 1).
성계 상태 표시(Phase 3 1번 작업에서 정했다): 항성 등급의 색·크기와 Hazard 경고(별 전체의 맥동)는 `M_Star`의 파라미터, 소유 문명 색은 별을 두르는 선의 고리(정착 중이면 끊긴 고리 — 구 위의 테두리는 개관 거리에서 몇 픽셀이라 읽히지 않았다), 이름과 건설·대응 칸의 진척은 성계마다 작은 화면 공간 위젯(`UCosmicSystemLabelWidget`)이다. 지도는 맵 액터 `ACosmicGalaxyMapActor` 하나가 그린다: `ACosmicGameMode::StartPlay`가 만들고(`L_Galaxy`는 고치지 않는다), `OnGalaxyGenerated`(새 은하와 로드)에 성계 액터를 다시 짓고, `OnSimTick`에는 표시만 해 두어 다시 그리기는 프레임마다 많아야 한 번이다(10배속에서도 그리는 비용이 틱 수를 따라 늘지 않는다). 그릴 값은 상태만 읽는 순수 함수 `CosmicMapLook::LookOf`·`LanesOf`가 계산한다(테스트 `UI.GalaxyMap`이 그리지 않고 본다). 선의 두께는 월드 단위라 카메라 거리에 비례하게 다시 잡는다(2번 작업 — 화면에서 약 1.2픽셀. 고정 두께는 멀리서 점선으로 끊기고 가까이서 13픽셀이 되었다. 거리가 5% 넘게 바뀌거나 먼 시점의 경계를 넘을 때만 다시 그린다). 머티리얼 `M_Star`·`M_Nebula`는 `Tools/make_materials.py`가 만든다(데이터 테이블처럼 생성물이다 — 에디터에서 고치면 덮어써진다. 성계 액터와 맵 액터는 생성자의 `FObjectFinder`가 아니라 `BeginPlay`의 `LoadObject`로 읽는다: 생성자 탐색은 머티리얼을 프로세스 내내 루트에 묶어 스크립트가 제자리에서 다시 채우지 못한다. 쿠킹은 `DirectoriesToAlwaysCook`의 `/Game/CA/Materials`). 앎의 표시(아래)는 8번 작업까지 아직 없어서 지금은 모든 성계를 그린다. 성역의 그림(v0.8 — 단계 4부터): 주인 고리 없음, 색 없음, 이름표 '태양계 · 성역'.

**앎의 표시**: 표현 계층은 `FGalaxyState.Knowing`(아는 성계·만난 백성·만난 은하 — §7.2, §8.1)을 읽어 무엇을 그릴지 가른다. 표시일 뿐이라 저장·해시와 무관하다(Phase 3 꼬리, §11).

- 모르는 성계는 배경 별과 구별되지 않게 그린다(같은 점 — 따로 셀 수 없다. 이름·소유 색·Hazard 없음).
- 아는 성계는 이름·항성 등급·Hazard·생명 낱말(§5.7)을 보인다. 소유 색과 백성의 이름은 그 주인이 만난 백성일 때만 보인다 — 만나지 않은 백성이 가진 아는 성계에는 소유 색도 백성 이름도 없다. 파괴된 아는 성계는 희미한 죽은 별 표시로 남는다(§7.2).
- 만나지 않은 백성은 지도·목록·외교에 없다. 만난 백성은 이름·색·형용사·'백성: {PopBand}'·Tier가 켜진다. 만났으나 사라진 백성은 표시에서 빠진다.
- 레인은 두 끝이 모두 아는 성계일 때만 그린다.
- 새 색을 쓰지 않고 밝기로만 가른다(머티리얼 파라미터 `Known`). 문명의 지도 색은 표현 계층이 `Knowing.MetCivs`의 순서대로 12색 팔레트에서 준다(열세 번째로 만난 백성부터는 첫 색에서 다시 돈다. EA 규모에서는 Phase 6a에 다시 본다) — 소유 색이라 Severity 4색 상한과 별개이고, 상태의 `Color`는 그대로다(§16 #36 ⑥ 닫음).
- 열리지 않은 은하(만난 은하가 아닌 것)는 종류와 현실 수치 문구만 보인다(은하 패널, §8.3).
- 초월자는 만남의 대상이 아니고 그 뒤에도 호칭만 가진다(§3.2). 첫 접촉 뒤에도 교단 지도자와 초월자는 이름을 얻지 않는다.
- 태양(성역)은 언제나 보인다 — 이름만. 앎에 쓰지 않는 표시 규칙이다(v0.8 — 단계 4부터).
- 행성 목록은 아는 성계에만 보인다(v0.8 — 단계 5부터).
- 은하 패널에는 모든 은하에 같은 한 줄 "이 은하의 별들도 하나도 빠짐없이 이름대로 불렸다." — 관할 분류·소속 값 없음(v0.8 — Phase 3 8번 작업부터).
- 영적 시야 토글(아래)은 보이는 것의 보기 방식만 바꾼다 — 격과 앎을 둘 다 통과한 것만 보인다(§3.2).

테스트 `UI.KnownFilter`(§9.3): 패널·피드에 만나지 않은 백성의 이름이 없다.

**영적 시야 토글**: 영적·사후 영역의 두 번째 지도는 없다. 우주 화면은 같은 지도의 줌 단계이지 별도 지도가 아니다. `M_Star` 파라미터 3개(`ViewMode`, `Pressure` 0~1 이미시브 맥동, `DominionColor` 교리별 색/성소 백색 — 성역(`sol`)은 경우 하나: 색 없음(성소의 백색도 아니다), 툴팁 "이곳에는 압력이 닿지 않는다."(법이 압력 0을 정해 참이다. v0.8 — 단계 4에서 Phase 3 7번 작업의 `DominionColor`에 경우 하나). 보라는 셋째 칸 영적 시야의 타락에만) + `Sealed` 링 + 주장 표식 `ClaimState` 1개(0 없음 / 1 주장 중: `ActiveHazard == Hazard.Claim`, 교리색 맥동 / 2 편입: `∈ InfluencedSystems`, 교리색 고정 / 3 결박: 봉인된 초월자의 영향권, Sealed 링 재사용 — 전부 파생, 새 위젯 없음, §4.2 영혼 경제), `M_Lane` 고압 구간 채도 저하, `M_Nebula`에 `SpiritualIntensity` 스칼라 1개. Pressure는 Presentation 측이 `ComputeSpiritualPressure`(§4.2)를 읽어 파생한다(Sim 폴더 무변경, §6.1). 성계 패널 툴팁은 호칭(`FTranscendentDef.Title`)만, 숫자·비율 없음. 토글은 PlayerTier ≥ 3에서 해금 — 그 전에는 물리 뷰의 이상 현상 아이콘만. T3 승천 연출 직후 "이제 보인다"가 첫 보상. 파라미터는 M_Star·ACosmicStarSystemActor를 처음 만드는 Phase 3에 넣는다(종료 조건 달성 뒤 꼬리, 약 2일. §11). 색은 Severity 3색(회/백/금) + 영적 계 보라 1색, 상한 4색(§8.2).

### 6.5 Niagara 예산 (3종)

| 시스템 | 트리거 | 비고 |
|---|---|---|
| NS_Ascension | 등급 상승 | 화면 중앙, 3~5초, 사운드 + 서사 텍스트 동반 |
| NS_Rift | `Hazard.SubspaceErosion` 발생/봉합 | 성계 노드 위치 |
| NS_TranscendentAura | 초월자 영향권 성계 가운데 아는 성계(§6.4 '앎의 표시' — 모르는 성계에는 어떤 이펙트도 그리지 않는다. `NS_Rift`도 같다). 영적 뷰(§6.4)에서는 그 전부, 물리 뷰에서는 플레이어 T4 이상일 때만 | 루프, 저비용 |

사망·폐위·영적 사건 전용 Niagara와 일러스트는 만들지 않는다. 사망·폐위·계승 화면(§8.4)은 `WBP_AscensionOverlay`와 `NS_Ascension`의 색 파라미터를 재사용한다.
`NS_Ascension`은 미뤘다(Phase 3 5번 작업, 2026-10-17): 이 프로젝트가 에셋을 만드는 헤드리스 스크립트(머티리얼·데이터·위젯 블루프린트)로는 Niagara 시스템을 짓지 못하고, 이 절은 예산(많아야 셋)이지 첫 연출의 요건이 아니다. 그 자리는 C++ 오버레이 `UCosmicAscensionOverlayWidget`(`WBP_AscensionOverlay`의 몫)이 메운다 — 어두운 막, 화면 가운데의 금빛 빛무리(UI 머티리얼 `M_AscensionGlow`, `Tools/make_materials.py`), 업적의 `Title`·새 칸·`MythicNarrative`, 코드로 합성한 짧은 종소리(`CosmicAscensionSound::MakeChime`, `USoundWaveProcedural` — 녹음된 소리는 Phase 5의 소리 작업에서). Niagara 시스템은 아티스트 작업이나 Niagara를 짓는 스크립트가 생길 때 온다.

### 6.6 모듈 의존성 (Build.cs)

```csharp

PublicDependencyModuleNames.AddRange(new string[] {

"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",

"UMG", "Slate", "SlateCore",

"GameplayTags", "Niagara"

});

PrivateDependencyModuleNames.AddRange(new string[] { "EngineSettings", "Json" }); // EngineSettings: UGameMapsSettings — 밸런스 커맨드렛의 GameInstance 생성 (§9.3). Json: 테스트 Data.Integrity가 Tools/data/schema.json을 읽는다 (§7.3, §9.3) — 테스트 전용이고 Core Logic은 JSON을 읽지 않는다

// 에디터가 아닌 타깃에는 FPSemantics = FPSemanticsMode.Precise (§9.2)

// CommonUI는 넣지 않는다 (§6.3). 필요 시 재검토.

```

---


---

## 9. 저장, 결정론, 테스트, 밸런스

초안에 없던 장이지만 Phase 1부터 들어가야 한다. 나중에 붙이면 전체 리팩터링이 된다.

### 9.1 저장/로드

`UCosmicSaveGame : USaveGame { UPROPERTY() FGalaxyState State; }` 하나.
상태가 전부 USTRUCT이므로 `UGameplayStatics::SaveGameToSlot`로 끝난다. Actor나 위젯 상태는 저장하지 않는다. 로드 후 Presentation이 상태를 보고 다시 그린다.
`SaveVersion`을 두고, 구조체 필드 추가 시 기본값으로 호환한다. 필드 삭제·의미 변경은 버전 올리고 마이그레이션 함수를 쓴다. 저장된 필드의 기본 초기값(멤버 초기화식)도 세이브가 생긴 뒤에는 바꾸지 않는다(의미 변경으로 취급한다): `SaveGameToSlot`은 클래스 기본 객체 대비 델타로 쓰므로 `FGalaxyState`·`Player`·`Calling`의 직속 멤버(와 그 안에 직접 든 구조체의 멤버)는 초기값과 같으면 기록되지 않고, 로드하는 빌드의 초기값으로 읽힌다(`TArray` 원소 구조체는 전부 기록된다). 같은 이유로 `SaveVersion`의 초기값은 리터럴 1로 고정하고(= 버전 태그가 없는 세이브가 읽히는 버전) 현재 버전 `FGalaxyState::CurrentSaveVersion`은 `NewGalaxy`와 마이그레이션이 찍는다 — 초기값을 그 상수에 묶으면 버전이 한 번도 기록되지 않아 첫 버전 인상 뒤 구 세이브가 새 버전으로 읽히고 마이그레이션이 돌지 않는다(UE 5.8.3 소스 확인: `GameplayStatics.cpp` `SaveGameToMemory`, `Class.cpp` `SerializeTaggedProperties`). Phase 1의 개발 세이브와, v0.7 예약 커밋부터 Phase 2 은하 생성 규칙 변경 사이에 만든 세이브는 넘기지 않는다(그런 세이브의 `Knowing`에는 출생 문명이 없어 첫 갱신이 제 백성을 첫 접촉으로 만날 것이다 — 고치는 코드를 두지 않는다, 아는 범위와 로그 작업의 리뷰) — 배포된 세이브가 없고 `SaveVersion`은 1 그대로이며, `Sim.SaveRestart`는 바이너리가 바뀌면 기록을 새로 쓴다.
**슬롯 정책: 판당 슬롯 1개(아이언맨).** 슬롯 이름은 `FGalaxyState.GalaxyID`에서 만든다(`Galaxy_<GUID>` — 이름은 그대로 두고 판의 신원으로 읽는다). 수동 저장·로드 메뉴는 없다. 사망 직전 세이브를 로드할 수 있으면 하드 리셋이 선택 사항이 되어 부름과 계승(§4.4) 전체가 무의미해진다. 결정론(§9.2) 덕에 비용은 정책 수준이다. "관조 모드" 토글(수동 저장 허용)은 Phase 5 테스터 반응 뒤 결정한다(§16).
**자동 저장 트리거**: 승천 직후, 역할 전환 직후(자진 이탈·즉위·복귀·폐위), **사망 직후**(공백기 진입 시, `PendingLeapTicks` 포함), **계승(취임) 직후**, 10분마다, 종료 시(강제 종료 롤백 방지). 모든 자동 저장은 틱 경계('역할 효과 해소' 단계 뒤, §7.4)에서만 찍는다. 사망과 계승 사이(공백기 도약 중)에는 저장하지 않는다 — 진입 시 저장한 `PendingLeapTicks`로 재개한다(`Calling.LeapResumeRoundTrip` 테스트, §9.3). 귀환 제안(§4.4)의 선택 해결 뒤에도 자동 저장한다.
**Phase 1 헤더 예약(v0.5 추가분)** — 추가는 호환, 의미 변경은 마이그레이션: `FCivilizationState.ArchetypeRow`(없으면 구 세이브의 모든 문명이 `arch_default` 뱅크로 떨어진다) / `FLifeRecord.SuccessionKind`·`bReturnOffered`(해시 포함) / `TranscendentID` == `DT_Transcendents` 행 이름 / `Realm`은 상태에 두지 않음(`FTranscendentDef.Allegiance` — 상태에 두면 개명마다 리다이렉트가 필요해진다) / `Doctrine.*` 이름은 상태에 남으므로 지금 확정(`Doctrine.Recurrence` 포함) / 열거형은 끝에만 추가(UE 아카이브는 UENUM을 이름으로 직렬화하므로 추가는 안전, 재배열·개명은 마이그레이션 — `EEffectType::ChangePopulation`, `EConditionType::TranscendentDoctrinePresent`) / `Population`은 문명 단위 유지 / `ComputeStateHash`에서 FText 제외(§9.2). 저장 필드 순증: FName 1 + 태그 1 + bool 1. `SaveVersion` 변경 없음.
**Phase 1 헤더 예약(v0.6 추가분, §1.3 요약 단위 원칙)**: `FNotableRecord`(Role·NameIndex·FirstTick·LastTick·SourceRow, 5필드 고정)·`FCivilizationState.Notables` / `FLifeRecord.NameIndex`(해시 포함) / `FCivArchetypeDef.PersonNameBank` / 열거형 끝에 `EEffectType::NameNotable`·`EConditionType::CivHasNotable` / `DT_Rules` 노브 2개(`MaxNotablesPerCiv`·`PopulationBandThresholds`) / `EPopulationBand`(파생, 저장 없음) / `FHazardDef.OnResolveEffects`(기본 빈 배열, §7.3). 전부 기본값 추가라 구 세이브는 빈 `Notables`·`NameIndex` 0으로 열리고 다음 사건부터 채워진다. 저장 필드 순증: 배열 1 + int32 1. `SaveVersion` 변경 없음. 헤더 리뷰 확정분(2026-10-12, §16): `FLifeRecord.BirthArchetypeRow`(FName 1, 해시 포함). `SaveVersion` 변경 없음.
**Phase 2 전 헤더 예약(v0.7 추가분, §3.1 네 층·§5.7 건넘·§8.1 앎)** — 추가는 호환: `FKnownMark`(`ID`·`Tick`)·`FKnowingState`(`KnownSystems`·`MetCivs`·`MetGalaxies`·`PendingContactCivID`·`FirstContactCivID`·`FarLineUntilTick` — `FirstContactCivID`의 초기값 NAME_None = 이 판에 아직 첫 접촉이 없다. `NoteMet`이 첫 접촉 때 한 번 적는다, §7.4)·`FGalaxyState.Knowing` / `FTransitRecord`(`Kind`·`FromGalaxyRow`·`ToGalaxyRow`·`DepartTick`·`ArriveTick`·`SourceCivID`·`ArchetypeRow`·`ResultCivID`)·`FGalaxyState.Transits` / `FGalaxyState.NextCivSerial`(int32)·`TellRngState`(int32, 초기값 0 동결) / `FStarSystemState.GalaxyRow`·`FCivilizationState.GalaxyRow`(초기값 NAME_None = 아직 찍히지 않음. `NewGalaxy`가 고향 행 이름을 찍는다)·`FCivilizationState.OriginGalaxyRow`(NAME_None = 토박이) / `FCosmicLogEntry.FocusCivID`·`bUntold`. 슬롯 신원과 스트림 값은 `FGalaxyState` 최상위에만 둔다. 열거형 추가 없음(`ELifeState::Crossing`·앎을 읽는 `EConditionType` 값은 넣지 않는다). 전부 기본값 추가라 구 세이브는 형식으로는 열리지만(빈 `Knowing`·`Transits`) 개발 세이브는 넘기지 않는다(이 단락의 끝 문장). 저장 필드 순증: 최상위 4(구조체 1 + 배열 1 + int32 2), 성계 FName 1, 문명 FName 2, 로그 항목 FName 1 + bool 1. 구조체 `FKnowingState`의 멤버는 6이다(배열 3 + FName 2 + int64 1 — FName은 `PendingContactCivID`와 `FirstContactCivID`). `SaveVersion` 변경 없음. 정의 데이터(`DT_Galaxies`·노브·태그)는 세이브에 남지 않으므로 처음 읽는 Phase에 넣고 행 이름만 지금 확정한다(§11). 개발 세이브는 이 커밋과 Phase 2 은하 생성 규칙 변경을 넘기지 않는다.
**v0.8 저장**: `SaveVersion` 1 그대로다(배포된 세이브 없음). v0.8의 단계 4(§11 'v0.8 예약' ①)에서 개발 세이브를 지운다 — 성역(태양계 `sol` — 다시 쓸 수 없는 법 하나로 소유·정착·주장·파괴에서 빠지는 곳, §4.5)의 ID와 첫 생애의 모성계가 바뀌기 때문이고, 지운 날을 이 문단에 적는다. 로드 때 주인 있는 성역(`sol`의 `OwnerCivID`가 비지 않음)을 에러 한 줄로 거절한다(4a). 서막(첫 틱 전의 카드, §4.4)은 저장하지 않는다(표현 계층) — 서막 도중 저장이 필요해지면 `ELifeState` 끝에 `Prologue`를 더한다(열거형 끝 추가는 호환). 첫 자동 저장은 저장 작업(§16 #33·#70·#40 ⑦) 뒤다. 부르심의 Core 함수(가칭 `ReceiveCalling`, §7.5)가 적는 '선택의 기록'이 저장 필드가 되면 초기값 규칙과 해시 포함 여부를 그 헤더에 적는다(v0.8 — 단계 4·6).

### 9.2 결정론

**난수의 출처 셋**: 난수의 출처는 셋뿐이다 — ①②는 `UGalaxySimulationSubsystem`이 주고 ③은 `Core/CosmicSeed.h`의 순수 함수다.

① **시뮬 스트림** `GetRng()`(= `RngState`): 저장되는 세계 상태를 바꾸는 모든 추첨 — 생성, 틱 단계, 건넘의 착지, 계승과 도약, 조우 판정, 인물과 그릇의 이름.

② **말하기 스트림**(= `TellRngState`, int32 하나): `UCosmicLogSubsystem::PushVariants`의 문장 변형 추첨만 — `DrawTell`을 부르는 함수는 이것 하나다(`PushRow`는 그 포장이고 `PickTemplateRow`는 뽑지 않는다). 살아 있는 스트림 객체를 두지 않는다 — `uint32 DrawTell(int32 Count)`가 `TellRngState`로 지역 스트림을 열어 한 번 뽑고 상태를 되적는다. 어떤 규칙도 ②를 읽지 않고, 말하기는 ①을 뽑지 않는다. `PushVariants`는 줄이 쌓이든 아니든 호출당 ②에서 정확히 1회 뽑는다(거절된 호출 — 빈 변형, 핸들러 안 — 은 뽑지 않는다. 마지막으로 말해진 줄과 같으면 — `bUntold` 항목은 건너뛴다 — 다음 변형으로 넘기고 다시 뽑지 않는다). 예외는 하나다: 범위 밖의 Severity 2 줄을 대신하는 먼 줄(§8.1)은 안에서 부르는 둘째 `PushRow`이고 제 몫의 추첨을 한 번 한다 — 그 호출은 2회다. `PushRow`의 반환값은 범위에 좌우되지 않는다(행이 없거나 템플릿이 비면 추첨 없이 false, 그 밖에는 true).

③ **파생 정수** `CosmicSeed::Derive`(아래 '시드와 파생'): 상태 없는 순수 함수 — 요약 은하의 값과 출발 일정, 건너온 백성의 이름과 아키타입, 성계의 생명 낱말, 표현 계층의 그림(배경 별·풍경 은하), 문명의 이벤트 슬롯 위상(§7.5).

②③은 ①을 뽑지 않고 ①에 씨를 주지 않는다. ③의 값이 시뮬 규칙의 일정을 정하는 곳은 둘이다 — 요약 은하발 출발 일정(Depart, §7.4)과 문명의 이벤트 슬롯 위상(EventSlot, §7.5): ①의 고정 추첨이 어느 틱에 놓이는지를 정할 뿐이고, 둘 다 Seed와 이름만의 함수라 앎·이야기·표현에 좌우되지 않는다. `Check`·`Apply`·`EvaluateAndFire`는 `GetRng()`를 직접 부르지 않고 문맥이 건넨 스트림을 쓴다(은하별 스트림은 둘째 상세 은하를 만들 때 이 절을 고쳐 더한다 — 지금은 두지 않는다). `FMath::Rand*`를 시뮬 코드에서 쓰면 설계 위반이고 추첨은 틱 순서의 고정 지점에서만 한다 — 둘 다 그대로다. 추첨 지점의 규칙 셋: ⓐ 지점은 무조건 뽑는다 — 후보가 없어도, 테이블이 비어도 뽑고 버린다(`RandHelper(0)`은 뽑지 않으므로 후보 수로 호출을 가르지 않는다) ⓑ 같은 처리 안의 추첨은 묶어서 먼저 뽑는다 — 이벤트 슬롯은 그 틱의 수를 전부 뽑은 뒤 행을 보고, 위기의 발생은 성계마다의 수를 전부 뽑은 뒤 Hazard를 놓는다 ⓒ `NameNotable`·`Notable.Ascendant`·그릇(`Install`)의 이름은 적용마다 정확히 1회, 가드와 뱅크보다 먼저다(틱당 원천은 `NameNotable`을 걸지 않는다 — 걸리지 않은 효과는 적용이 아니고 추첨도 없다, §7.5). 그래서 추첨 지점마다 뽑는 수는 시드·틱·소유와 세계에 일어난 일만 따르고 `DT_Events`의 행 수·조건·가중치·쿨다운과 이름 뱅크·Tier·상한에 좌우되지 않는다(이름 뱅크·Tier·상한은 `NameNotable`·`Notable.Ascendant`·`Install`의 가드를 두고 하는 말이다 — AI 단계의 정착 추첨은 정착할 수 있는가(`MaySettle`)를 따르므로 백성의 정착된 성계·소유 성계 수·진행 중 정착과 그 Tier의 `RealmCapByTier`를 읽는다. 정착 행·대상·지형은 읽지 않는다) — 발화한 행이 `NameNotable`을 실었으면 그 적용마다 1회가 더해지고(행의 효과 목록이 정한다), 만료·폐위처럼 일어난 일마다 뽑는 지점은 그 일이 일어난 수만큼 뽑는다. 그 일이 언제 몇 번 일어나는지는 세계를 따르고 세계는 테이블의 값(카운트다운·확률·조건·Stability)을 따르므로, 테이블의 값을 바꾸면 같은 시드의 역사도 바뀐다 — 바뀌지 않는 것은 한 지점이 한 번에 뽑는 수다. `Check`가 받는 뷰에는 스트림이 없다(§7.5). Phase 2 뒤의 틱 안 추첨 지점: 위기 단계의 만료(모은 만료마다 2회 — 결과의 수와 주장의 수. Hazard의 종류와 행의 유무에 좌우되지 않는다), 발생(정착된 성계마다 1회 — 슬롯이 찼든 비었든), 역할 효과 해소 단계의 폐위(섭정 프로필 1회), 이벤트 슬롯(슬롯을 받는 문명마다 1회), `NameNotable`(적용마다 1회), AI 단계의 정착 추첨(정착을 시작할 수 있는 AI 백성마다 1회, 대상을 보기 전에 — 대상이 없어도, 정착 행이 없어도. 정착과 문명 제거 작업)과 업적 판정의 인물 이름 — 분쟁·조우·계승의 지점은 Phase 4·4b에 더한다(§11).

해시: `RngState`는 지금처럼 살아 있는 스트림 값으로 대체하고, `TellRngState`는 저장 값(항상 최신)을 그대로 읽는다. `TellRngState`의 멤버 초기값 0은 동결이고 `NewGalaxy`가 값을 찍는다. `ComputeStateHash()`는 전부를, `ComputeWorldHash()`는 `Knowing`·`Log`·`Chronicle`·`TellRngState`를 뺀 것을 해시한다(§8.1).

'v0.7 예약' 커밋(§11)에서 함께 하는 일: `PushRow`를 말하기 스트림으로 옮긴다. `Log.NoConsecutiveRepeat`의 스트림 탐침 3곳을 `TellRngState`로 옮긴다('호출당 1회' 단언 유지). `Sim.SaveRoundTrip`에 `TellRngState`가 이어지는지의 단언을 더한다. 새 테스트 `Sim.TellStreamSeparate`(`PushRow` 전후 `GetRng().GetCurrentSeed()` 불변, `TellRngState`만 전진).

**v0.8과 추첨**: 서막은 추첨이 없다(바꾸려면 조우 판정과 같은 입력 열의 자격 — ①). 성역은 추첨 없이 배열 끝에 붙는다(단계 4 — 수가 바뀌지 않는다). 실제 하늘(단계 5 — 태양 근방 관측 목록의 계와 행성으로 짓는 지도)은 생성 추첨 목록을 바꾸고 Phase 5 전에 모든 시드를 한 번 바꾼다(새 기준값) — '시뮬 추첨 지점은 테스터와 시드를 나누기 전에만 더한다'(§11)를 지킨다. 시간 표시는 시뮬 스트림을 쓰지 않는다. 그 백성이 제 별을 부르는 이름은 `CosmicSeed::Derive`다(단계 5 — 시뮬 추첨 없음).

**시드와 파생**: 한 판 = 우주 시드 1개 = 부름 1회 = 4~6시간, 기대 2~5대(길이는 그대로다). 플레이어가 넣는 시드는 `FGalaxyState.Seed` 하나뿐이다(필드, `-Seed=`, `NewGalaxy(Seed, SystemCount, CivCount)` 시그니처 그대로). 은하마다 시드를 받지 않고, 시작 은하나 시작 규모를 고르는 화면은 없다 — 첫 생애는 우리은하의 성역 밖(태양계 바깥) 한 별이다(`sys_0` — v0.8 — 단계 4부터. 그 전의 코드는 태양계 `sol`). 고향 은하(`DT_Rules.HomeGalaxyRow`, 기본 `gal_milkyway`)는 시뮬 스트림 `Initialize(Seed)`에서 생성된다. 그 밖의 모든 파생값은 `CosmicSeed::Derive`(`Core/CosmicSeed.h`)에서만 나온다. 은하는 언제나 `DT_Galaxies` 행 이름으로 가리키고 은하별 시드는 저장하지 않는다(파생) — 행을 더하거나 순서를 바꿔도 다른 은하의 값이 움직이지 않는다. 은하 행 이름과 `HomeGalaxyRow`는 세이브에 닿는 다른 행 이름처럼 배포 뒤 바꾸지 않는다(§10.4). 시계는 `Tick` 하나다(은하별 시계 없음).

`CosmicSeed::Derive`는 모듈 안의 인라인 함수 하나이고 부호 없는 32비트 산술만 쓴다(넘침은 그대로 감긴다. `GetTypeHash`·엔진 CRC·부동소수점 금지). 식과 상수는 여기 적힌 그대로 동결한다:

```
uint32 Derive(int32 Seed, FAnsiStringView LowerAsciiName, uint32 Purpose, uint32 Index = 0)
  h = 0x811C9DC5
  for each byte b of LowerAsciiName:  h ^= b;  h *= 0x01000193        // FNV-1a 32
  h ^= (uint32)Seed;   h *= 0x9E3779B1
  h ^= Purpose;        h *= 0x85EBCA6B
  h ^= Index;          h *= 0xC2B2AE35
  h ^= h >> 16;  h *= 0x85EBCA6B;  h ^= h >> 13;  h *= 0xC2B2AE35;  h ^= h >> 16
  return h
```

용도는 고정 상수이고 번호를 다시 매기지 않는다: Tell 1(말하기 스트림의 시작값), Summary 2(요약 은하), Name 3(건너온 백성의 이름), Life 4(성계의 생명 낱말), Backdrop 5·Scenery 6(표현 계층의 그림), Depart 7(요약 은하발 출발 일정), Archetype 8(요약 은하발 건넘의 아키타입), EventSlot 9(문명의 이벤트 슬롯 위상 — 이름은 CivID, §7.5). 새 용도는 끝에 더한다(안전하다). 인자를 더하는 일은 기준값이 생긴 뒤에는 안전하지 않으므로 `Index`를 처음부터 둔다. Tell의 이름은 빈 문자열이고, 그 밖의 이름은 행 이름이나 ID를 소문자 ASCII로 쓴 것이다. 파생 값으로 스트림을 열 때는 `FRandomStream::Initialize((int32)Value)`를 쓴다. `TellRngState`는 `NewGalaxy`가 `(int32)Derive(Seed, "", Tell, 0)`으로 찍는다.

기준값(엔진 밖에서 계산했다. 테스트 `Sim.SeedDerive`가 상수로 갖는다 — 이 프로젝트의 첫 골든 값):

| Seed | 이름 | 용도 | Index | 값 |
|---|---|---|---|---|
| 42 | (빈 문자열) | Tell 1 | 0 | 272114134 (0x103821D6) |
| 42 | gal_andromeda | Summary 2 | 0 | 569404729 (0x21F06D39) |
| 7 | sol | Life 4 | 3 | 2833998181 (0xA8EB6165) |
| -1 | gal_lmc | Depart 7 | 2 | 668976756 (0x27DFC674) |
| 0 | (빈 문자열) | Tell 1 | 0 | 4196651879 (0xFA23D367) |

`Sim.SaveRestart`의 기록에 Derive 값 하나를 더해 두 프로세스에서 같은지 본다(두 절반이 같은 바이너리라 구조상 같다 — 값을 쥐는 것은 `Sim.SeedDerive`와 헤더의 `static_assert`다). v0.7에서 같은 시드가 주는 은하는 달라진다(골든 해시가 없어 깨지는 테스트는 없다). 이 변화는 Phase 2의 은하 생성 규칙 변경 한 번으로 묶고 테스터와 시드를 나누기 전(Phase 5 전)에 끝낸다.

틱 내부 처리 순서는 고정(§7.4 주석). `TMap` 순회로 결과가 달라지는 코드를 쓰지 않는다. 순서가 중요한 곳은 `TArray`를 정렬해서 쓴다. `TMap`은 한 판 안에서는 순서가 결정적으로 보여도, 원소를 지운 뒤 새로 넣으면 실행 중에는 빈 자리를 재사용하고 로드 뒤에는 끝에 붙어 저장·로드 전후의 순회 순서가 달라진다 — 키를 `FNameLexicalLess`로 정렬해 쓰거나 `Civs`·`Transcendents` 배열을 돌며 `Find`한다.
같은 시드 + 같은 플레이어 입력 시퀀스 → 같은 `ComputeStateHash()`. 이것이 테스트의 기준이다. 해시에는 `Calling`(생애 인덱스·상태·기록)이 포함된다. 해시는 FName·숫자·태그만 포함하고 모든 FText(`Log`·`Chronicle`의 `Message`, `DisplayName`, `CivName` 등)를 제외한다 — 로그 항목은 Category·Severity·Tick·SourceRow·FocusSystemID·FocusCivID·bUntold만(§4.4의 FText 저장 원칙과 같은 이유. 로컬라이제이션만 바뀌어도 Determinism 테스트가 깨지는 것을 막는다). `FLifeRecord.SuccessionKind`·`bReturnOffered`·`BirthArchetypeRow`는 포함한다. `FCivilizationState.Notables`(Role·NameIndex·FirstTick·LastTick·SourceRow — 정수·태그만)와 `FLifeRecord.NameIndex`도 포함한다. 규모 등급(`GetPopulationBand`)은 파생값이라 해시에 없다. 사망 파이프라인·계승 추첨·공백기 도약의 추첨도 전부 시뮬 스트림 `GetRng()`를 통한다(§4.4. 그 줄들의 문장 변형은 말하기 스트림이다). 인물의 이름 추첨(`NameNotable` 가드·`Install()`)도 시뮬 스트림만 쓰며(`Apply`는 문맥이 건넨 스트림으로, 적용마다 1회 제 가드보다 먼저), 틱 순서 안의 고정 지점(`UCosmicEventSubsystem::Apply`, `TickHazards` 만료 처리, `UAscensionSubsystem` 판정, 역할 효과 해소 단계의 `Install`) 또는 조우·청원 해결(틱 밖 플레이어 입력 — 판정 난수와 같은 자격으로 입력 시퀀스의 일부)에서만 일어난다. 같은 시드와 같은 입력 시퀀스면 같은 명부. `Notables`는 `TArray`이고 퇴출 선택은 (LastTick, FirstTick, 배열 순서) 키라 `TMap` 순회 없음. **해시의 표준형**: `FName`과 `FGameplayTag`는 소문자로 바꾼 문자열로 해시한다 — `GetTypeHash(FName)`·`GetTypeHash(FGameplayTag)`·이름 인덱스·구조체 원시 메모리는 프로세스마다 값이 달라, 한 프로세스 안의 `Sim.Determinism`·`Sim.SaveRoundTrip`은 통과해도 "저장 → 재시작 → 로드 → 해시 동일"(§11 Phase 1 종료 조건)은 깨진다. `TMap`은 `FNameLexicalLess` 키 순서로, `FGameplayTagContainer`는 명시 태그만(파생 부모 태그 제외) 해시한다. `FGalaxyState.GalaxyID`는 저장 슬롯의 신원이라 해시에서 제외한다 — `NewGalaxy`가 `FGuid::NewGuid()`로 만들고(`GetRng()`를 쓰지 않는다) 같은 시드의 두 판도 값이 다르다. 시뮬이 읽는 FText는 없다 — 모든 AI 문명은 생성 때 이름을 받고(§5.11), 돌파 줄이 말해지는가는 이야기의 범위가 정한다(§8.1). `ComputeWorldHash()`는 `ComputeStateHash()`에서 `Knowing`·`Log`·`Chronicle`·`TellRngState`를 뺀 해시다(최상위 멤버 이름 기준) — 앎이 세계를 바꾸지 않음을 재는 자이고(`Sim.KnowingInert`, §8.1) 쿠킹 빌드 비교도 이것으로 한다. `Knowing`은 전체 해시에 들고 세계 해시에는 들지 않는다.
결정론의 보상: 버그 재현, 리플레이, 헤드리스 밸런스 측정이 모두 가능해진다. 테스트와 밸런스 커맨드렛(에디터 바이너리)이 게임 바이너리와 같은 시드에서 같은 해시를 내도록, 게임 모듈은 모든 타깃에서 부동소수점을 정밀 모드로 컴파일한다(`Build.cs`의 `FPSemantics = FPSemanticsMode.Precise`. V7 기본값은 에디터 타깃만 정밀이고 게임 타깃은 `/fp:fast`다). 해시에 들어가는 부동소수점 값은 이 모듈 안에서 컴파일되는 코드(헤더 인라인 포함)로만 계산한다 — 엔진 모듈의 비인라인 함수(예: `FLinearColor::MakeFromHSV8`)는 타깃의 부동소수점 모드를 따르므로 쓰지 않는다(AI 문명 색은 모듈 안의 같은 산술로 낸다). 해시에 드는 값은 + − × ÷ √만 쓴다(`sqrtf`는 정확히 반올림된다). 삼각 함수를 비롯한 다른 C 런타임 수학 함수는 쓰지 않는다 — 성계 배치는 원판 안 기각 추출이다(§7.4). 기각 추출은 성계 하나에 16번까지 시도하고, 16번째 점도 원판 밖이면 그 점의 좌표를 절반으로 줄여 쓴다(정사각형 안의 점은 절반으로 줄이면 언제나 원판 안이다. 16번 내리 빗나갈 확률은 성계 하나에 약 2 × 10⁻¹¹이다 — 상한은 루프를 묶으려는 것이다). 해시에 드는 산술은 모듈 안 코드로만 한다(`FMath::Pow`·비인라인 `FVector` 도우미 금지). 코드의 `FMath::Cos`·`FMath::Sin`은 Phase 2 은하 생성 규칙 변경 커밋에서 한 번에 지웠다. `SummaryAt`은 시뮬 규칙이다(§7.4) — 이 수학 규칙을 따른다. 현실 수치(§3.4)와 닿는 것 가운데 해시에 드는 것은 셋뿐이다(§7.4): 항성 등급(`DT_Rules.LivingWorldStarShares` — 설계 값), `sys_0`의 고정 자리(`FGalaxyDef.HomeRadiusShare` — v0.8 — 단계 4부터. 그 전의 코드는 `sol`), 성계 좌표(v0.8 — 단계 5 뒤에는 관측 목록의 좌표·등급이 해시에 든다 — 오프라인 도구가 계산한 값). 나선팔·막대·광년 눈금·배경 별·풍경 은하는 표현 계층이 그린다(해시 밖). 게임 바이너리에서의 해시 일치는 첫 패키징 때 확인한다 — 테스터와 시드를 나누기 전에 패키징한 게임 타깃에서 한 번, 비교는 세계 해시로 한다.


---

### 9.4 성능 예산

| 항목 | 예산 |
|---|---|
| 시뮬 1틱 (상세 은하 전체: VS 60·12, EA 시작값 300·48) | < 1 ms. Phase 2 종료 실측(2026-10-17, 에디터 바이너리 Development, Ryzen 7 9700X, 입력 없는 판, 전부 쌓는 모드 — 시드 42, 종료 리뷰 수정 전의 세계, 임시 측정): 평균 VS 36 µs·EA 150 µs, p99 VS 0.12 ms·EA 0.44 ms, 가장 긴 틱 2.3 ms(EA에서 한 번). 재감사 뒤의 커맨드렛(VS, 시드 1~29와 42): 평균 30 µs, 판마다의 p99 가운데 가장 큰 값 74 µs, 가장 긴 틱 2.9 ms(30판에 한 번 — 단계별 시간은 아직 재지 않아 그 틱이 어디서 길었는지 모른다, Phase 4 인계). 패널이 듣는 게임의 틱은 이보다 길다 |
| 10x 배속 | 프레임당 최대 10틱 처리, 초과 시 틱을 다음 프레임으로 이월하되 이월은 최대 10틱분까지 — 그 이상 밀린 시간은 버린다(긴 멈춤 뒤 몰아치기 방지). 일시정지·새 은하·로드가 프레임 도중에 걸리면 그 프레임의 남은 틱은 돌지 않는다. 새 은하와 로드는 틱 경계(`OnSimTick` 핸들러)나 입력에서만 건다 — 틱 단계 도중에 동기로 발화하는 `OnLogEntry`·`OnEventFired`·`OnTierAscended` 핸들러에서 상태를 갈아 끼우거나 틱을 돌리지(`StepOnce`·`RunHeadless`) 않는다(코드가 막는다: `NewGalaxy`·`LoadState`·`LoadGalaxy`·`StepOnce`·`RunHeadless`는 그 핸들러 안에서 불리면 에러 한 줄을 내고 아무것도 하지 않는다. 틱 단계 안의 ensure는 뒷받침으로 남는다. 그 자리의 `SaveCurrentGalaxy`는 §16 #33 ①과 함께 막는다) (렌더 60fps 유지). 프레임당 시뮬 10 ms 이하. 상한은 틱 수로 두고 시간 기준으로 바꾸지 않는다(`Sim.FrameStep`) |
| 앎 갱신 (`TickSight`) | < 0.1 ms (10틱마다). 실측(300·48, 종료 리뷰의 임시 측정 — 가장 나쁜 꼴은 앎을 손으로 채운 판이라 커맨드렛이 재지 못한다): 첫 칸 그릇(아는 성계 9) 약 17 µs, 가장 나쁜 꼴(통치 문명 Tier 4, 성계 300을 다 알고 백성 48을 다 만났다) 약 30 µs |
| 이벤트 슬롯 (`EvaluateAndFire`, 매 틱) | VS < 0.02 ms, EA 평균 < 0.1 ms · 가장 찬 틱 < 0.3 ms (Phase 2 종료 실측, 게이트를 닫고 범위를 켠 게임의 꼴: VS 평균 4.6 µs·가장 찬 틱 37 µs, EA 평균 17 µs·p99 38 µs·가장 찬 틱 0.21 ms. 전부 쌓는 모드에서 패널이 들을 때는 줄을 쓰고 패널을 다시 그리는 값이 더해져 EA p99 0.36 ms·최대 1.4 ms였다. 행 목록과 쿨다운 키를 틱마다 다시 만드는 값은 Phase 4의 행이 늘 때 다시 잰다. 같은 임시 측정에서 업적 판정(`EvaluateFeats`)은 300·48 평균 25 µs·p99 70 µs) |
| 영압 패스 (경제 단계, 매 틱) | EA < 0.1 ms (추정). 틱마다 영향권 소속 표와 초월자 행 목록을 한 번 만들어 쓰고, 인접은 성계의 레인을 그 표에 비춰 읽는다 — 성계마다 공개 함수를 부르거나 행을 다시 찾지 않고, 누르는 초월자가 없는 틱에는 레인을 풀지 않는다 |
| 쿨다운 맵 | 백성당 살아 있는 키 약 10, 많아야 30 — 48 백성에서 20~65 KB로 짐작했다. Phase 2 종료 실측: 백성당 약 3~4(20,000틱 끝, 12 백성 40·48 백성 149) — 수 KB |
| 새 은하 생성 (성계 300) | < 50 ms (2026-10-14 실측 약 0.5 ms — 300·48, 에디터 바이너리 Development, `Sim.GenerationRules`의 안내 줄. 종료 리뷰의 임시 측정은 `NewGalaxy` 전체를 300·48에서 약 1.3 ms로 쟀다 — 다른 계기, 로그와 첫 생애가 든다) |
| 공백기 도약 1,600틱 | ≤ 1.6초 |
| 세이브 크기 / 동기 저장 | ≤ 1 MB / ≤ 100 ms (§16 #40 ⑦의 실측으로 확정. 백성 48 기준). Phase 2 종료 실측: 로그 항목 하나가 약 600바이트(태그드 속성) — 범위를 켠 판의 20,000틱 끝 VS 0.4~1.07 MB·EA 1.23 MB(로그가 반쯤 찼다), 전부 쌓는 모드 VS 1.8 MB·EA 4.2 MB(연대기에 상한이 없다). 상태의 직렬화(`SaveGameToMemory` — 상태의 복사와 파일 쓰기는 빠진다, 임시 측정) 27~62 ms, 재감사 뒤의 커맨드렛(복사와 직렬화, 전부 쌓는 모드, VS 30판) 평균 20 ms·1.7 MB. 커맨드렛의 `SaveBytes`는 #70의 머리와 압축이 오면 그 감싸기를 지나 잰다. 지금 크기로는 넘는다 — #40 ⑦의 기본값(연대기에는 `BuildChronicle`이 읽는 것만, #70의 머리 안에서 압축)으로 맞춘다 |
| 커맨드렛 실행 시간 (20,000틱, 300·48) | Phase 2 종료 실측: 틱만 약 3초(전부 쌓는 모드), 60·12는 약 0.7초. 1,000틱마다의 세이브 바이트와 CSV를 더한 커맨드렛 전체는 Phase 5에 기입 |
| 해시 시간 (300·48) | Phase 2 종료 실측: 전체 해시 약 4 ms(로그 2,000·연대기 3,922), 60·12는 약 1.8 ms (해시는 테스트·커맨드렛 전용이라 프레임 예산 밖이다) |
| 로그 UI | `UListView` 가상화, 표시 상한 500. 로그 상한 2,000과 다이제스트 5~8줄도 그대로 — 이야기의 범위가 유입을 묶는다(§8.1) |
| 성계 액터 | ≤ 150개(아는 성계 가운데 화면에 든 것의 상세 표현), 개별 Draw Call 허용. VS는 생명 성계 60개를 전부 액터로 그리고 머티리얼 파라미터 `Known`(밝기)으로 가른다. EA(300)에서는 나머지를 인스턴스 메시로 옮긴다(Phase 6a) |
| 배경 별 20,000 / 풍경 은하 5,000 | 인스턴스 또는 하늘 머티리얼로 드로 각 1. 선택 없음, Niagara 3종 예산 밖, 새 Niagara 없음 |

EA 수치는 Phase 2 종료 리뷰에서 처음 쟀다(위기·AI 단계는 본문이 있고 외교·분쟁 단계는 아직 비었다 — Phase 4에 다시 잰다). Phase 5 실측에서 넘으면 EA 값을 200·30으로 내린다. 성계·문명 찾기는 저장하지 않는 색인(ID → 인덱스)으로 한다(순회 금지, §7.4).

---


---

# 부록: 아키텍처 불변 규칙 전문 (옛 CLAUDE.md, v0.8 — 2026-10-18 이관, 내용 무변경)

> `CLAUDE.md`의 14항목 요약이 이 전문의 색인이다. 전문의 괄호 `(v0.7 — …)`·`(v0.8 — 단계 N부터)`는 예정 표지다.

`Source/Cosmic_Ascension/Sim`, `Events`, `Ascension`, `Log`, `Player`, `Save` (Core Logic Layer 전체)는 UUserWidget/AActor를 include하지 않는다 (§6.1).
모든 런타임 상태는 `FGalaxyState` 안의 USTRUCT. UObject 그래프로 상태를 만들지 않는다 (§6.2, §7.2). `FGalaxyState`는 판의 상태다(고향 은하 상세 + 판 공통) — 코드 식별자의 Galaxy는 판을 가리키고 개명하지 않는다. 상태는 세 층이다: 세계 → 앎(`Knowing`) → 이야기(`Log`·`Chronicle`·`TellRngState`). 세계 코드는 앎을 `NoteMet`·`NoteKnown`으로 쓰기만 하고 읽지 않는다(그 밖에 앎에 닿는 곳은 이것뿐이다: `Install`이 출생 성계·출생 문명을 줄 없이 적고(첫 접촉의 기록 `FirstContactCivID`는 `NoteMet`만 적는다 — `Install`은 적지 않는다), 건넘 단계의 정리·역할 전환·플레이어의 답이 `PendingContactCivID`를 비우고, 말하기(`Push`·`PushVariants`의 범위 판정)가 `FarLineUntilTick`을 적는다 — 어느 것도 세계 결과가 없다). 앎을 읽는 조건·효과는 없다 (§8.1). (v0.7 — `PendingContactCivID`를 비우는 줄 가운데 플레이어의 답 쪽은 첫 접촉 카드가 오는 Phase 4다)
난수의 출처는 셋이다: 시뮬 스트림 `UGalaxySimulationSubsystem::GetRng()`(세계 상태를 바꾸는 모든 추첨), 말하기 스트림 `DrawTell()`(`TellRngState` — `UCosmicLogSubsystem::PushVariants`의 문장 변형만. 부르는 함수는 그것 하나다), 파생 정수 `CosmicSeed::Derive`(`Core/CosmicSeed.h`, 상태 없음, 식·상수 동결). 서로 읽지 않는다. 추첨 지점은 무조건 뽑는다: 이벤트 슬롯은 슬롯당 정확히 1회를 행을 보기 전에 먼저 뽑고, `NameNotable`과 `Install`의 이름 추첨은 가드·뱅크보다 먼저 1회이고(틱당 원천은 `NameNotable`을 걸지 않는다 — 추첨도 없다), 위기 단계는 정착된 성계마다 발생의 수 1회(슬롯이 찼든 비었든)와 모은 만료마다 2회(결과·주장)를 각각 행을 보기 전에 전부 먼저 뽑고, 열째 단계의 폐위는 섭정의 수 1회를 뽑고, AI 단계의 정착은 정착을 시작할 수 있는 AI 백성마다 1회를 대상을 보기 전에 뽑고(시작할 수 없는 백성은 뽑지 않는다), `NewGalaxy`의 타락 로스터는 침묵 다음 자리마다 1회(0~3으로 읽은 `FallenTranscendentCount` − 1회)를 후보를 보기 전에 뽑고, 생성의 나머지 추첨(성계의 자리·등급·풍부도, 모성계, 아키타입·윤리·프로필, 이름, 로스터 자리마다의 초기 코어)도 쓰지 않을 때조차 정해진 순서로 뽑는다(`NewGalaxy`의 머리 주석) — 한 지점이 한 번에 뽑는 수는 행 수·쿨다운·이름 뱅크·`DT_Civs`·`DT_Galaxies`·`DT_Transcendents`·`DT_Hazards`·`DT_Megaprojects`의 내용에 좌우되지 않는다(수 인자 0일 때 읽는 고향 행의 `LivingWorlds`·`Peoples`만 예외 — 그 둘이 성계와 백성의 수다). 만료·폐위·인물 이름처럼 일어난 일마다 뽑는 지점은 그 일의 수만큼 뽑고, 그 일이 언제 몇 번 일어나는지는 세계를 따르므로 테이블의 값(카운트다운·확률·조건)이 바꾼다 (§9.2). `FMath::Rand*` 금지 (§9.2). 서막은 추첨이 없다(v0.8 — 단계 6. 바꾸려면 조우 판정과 같은 입력 열의 자격). 성역은 추첨 없이 배열 끝에 붙는다(단계 4). 실제 하늘(단계 5)은 생성 추첨 목록을 바꾸고 Phase 5 전에 모든 시드를 한 번 바꾼다(새 기준값). 시간 표시는 시뮬 스트림을 쓰지 않는다.
틱 처리 순서(열 단계): 경제 → 연구 → 외교 → 분쟁 → 위기 → AI → 건넘 → 플레이어 이동 → 앎 → 역할 효과 해소. 변경 시 guideline.md §7.4 먼저 수정. 이벤트·업적·영압(영적 압력)은 제 단계가 없다: 이벤트 슬롯(`UCosmicEventSubsystem::EvaluateAndFire`)은 위기 단계의 끝, 업적 판정(`UAscensionSubsystem::EvaluateFeats`)은 AI 단계의 끝, 틱당 영압 효과는 경제 단계의 수입과 비축 사이다. 틱당 원천(`bPerTickSource`)은 양을 고치는 효과 일곱만 건다 — 경제 단계는 추첨하지 않고, 역할 효과를 적지 않고, 그릇을 고치지 않는다(§7.5). 발화한 행의 효과는 큐 없이 제자리에서 적용하고 행은 '효과 전부 → 받은 성계의 앎(`FollowUpKnowing` — 통치 문명이 받은 성계를 알고 준 백성을 만난다) → 행의 줄 → 뒤따르는 줄(효과가 놓은 Hazard의 발생 줄과 그 `OutbreakRow` 행, 해결 줄) → `OnEventFired`'의 원자다. `Check`·`Apply`는 static이라 세계만 읽고 쓴다(줄·앎·방송에 닿지 못한다). `Civs`의 원소 수는 건넘 단계에서만 바뀌고 `Systems`는 줄지 않는다(파괴된 성계는 죽은 노드).
소스 파일은 ASCII. `UMETA(DisplayName)` 영문. 한글은 DataTable/String Table (§10.3).
헤더는 모듈 루트 기준 경로로 include: `#include "Sim/GalaxySimulationSubsystem.h"`. (Build.cs가 `ModuleDirectory`를 PrivateIncludePaths에 추가해 둠. Public이 아닌 이유: 다른 모듈이 이 헤더를 쓰지 않고, UBT가 모듈 루트를 Public include로 두는 것을 허용하지 않음)
GameplayTag는 `Config/DefaultGameplayTags.ini`에 선언하고 코드에서 `FGameplayTag::RequestGameplayTag(FName)`로 얻는다. 태그는 `Domain.Sub.Leaf` 최대 3단계 (§10.4).
CommonUI 사용 금지 (UMG만, §6.3). Build.cs에 CommonUI를 추가하지 않는다.
`UE_LOG` 카테고리는 `LogCosmic` 하나 (`Cosmic_Ascension.h`, 개발자 로그). §8.1의 `Log.*` GameplayTag는 게임 내 서사 로그의 분류이며 별개다.
하나님(거룩한 질서)은 엔티티·화자·수치·엔딩 조건이 아니다 (§4.5). 1인칭 로그 화자는 타락 초월자만.
태양계(`sol`)는 다시 쓸 수 없는 법의 성역이다(§4.5 ①, 가칭 `Law.Hedge`·`CosmicState::IsSanctuary`): 소유·세력·색·수치·관계 키가 아니고, 정착·넘김·주장·교단·초기 코어·모성계·출생·계승·즉위·파괴·영향권의 대상이 아니며 압력은 0이다. 그릇은 `sol`에 서지 않는다(방랑자는 경계 카드 '지경'까지). 게임 안에서 성역을 가리키는 문구는 이름 없는 수동태와 화자의 목소리뿐이다 — 백성마다의 이름 줄(말을 받은 회중은 '하늘의 하나님이 지키시는 땅'), 말이 온 뒤의 기록원 인용, 셋째 칸부터 타락 1인칭의 '울타리의 증언'(이름 없이 '위')이다(§8.2 '성역의 이름'). 그분의 이름은 게임 밖 '세계관과 출처' 화면에만. 성역의 별을 첫 생애의 해나 타락의 별과 한 줄·한 카드·한 장면에 맞세우지 않는다. (v0.8 — 단계 4부터. 그 전의 코드는 `sol`이 플레이어 모성계다)
첫 생애는 성역 밖 찾는 백성(계시 양식이 '찾음' `Revelation.Seeking`인 백성. 첫 생애 행은 `DT_Rules.PlayerArchetypeRow` = 가칭 `arch_seeker`, `Weight` 0, 두 목소리 — 의회·회중)의 한 필멸자다. 성역 밖에 인간 백성·인간 이름 뱅크는 없고, '지구'라는 낱말은 성역 패널의 행성 목록과 게임 밖 화면에만 쓴다(백성의 목소리는 '옛 땅'). 책은 고넬료 사슬(방문 응답 → 사자의 전언 → '말이 도착했다' → `Trait.Faith.Book`)로 나중에 오고, 사자는 가리키기만 하고 책을 건네지 않는다. 성육신의 한 줄은 '옛 땅의 기억'(말이 도착한 뒤 열리는 한 쪽짜리 화면)의 회중 쪽 고백으로만 선다(연대기·두 책·조우 카드·다른 백성의 입에는 없다). (v0.8 — 단계 4b·7부터. 그 전의 데이터는 `arch_terran`)
통치자는 '부르심을 입어 구별되고 날이 더해진 자 — 한 백성을 맡았다'이다(§4.4). 서막은 칸이 아니고(T0 없음) 부르심은 칸·`PeakTier`를 주지 않는다. 플레이어에게 구원자·메시아·선택받은 자·빛의 아들·기름 받은 자라는 칭호, '신이 되었다'·'초월자가 되었다', 명사 '깨달음'(해탈의 길 카드의 자기 어휘는 예외 — §16 #85)을 게임 안에 쓰지 않는다 — 받은 꼴(구별되었다·부르심을 입었다·날이 더해졌다·눈이 열렸다·깨닫게 되었다)만. 부르심은 소명이지 구원이 아니다.
Core Logic Subsystem은 플레이어·생애에 종속된 멤버 변수를 갖지 않는다. 지속 업적 진척은 FGalaxyState 안에.
`FCivilizationState.Relations`의 키는 CivID 또는 TranscendentID. 항목은 처음 쓸 때 생기고 없는 키 = 0 = 중립이다 (§5.5). `Check(CivRelationAtLeast)`와 `Apply(ChangeRelation)`는 이 뜻으로 읽고 쓰며, 상대는 살아 있는 문명, 판에 있는 초월자 인스턴스, `DT_Transcendents`의 천상(`Realm.Celestial`) 행 이름 가운데 하나일 때만 풀린다 — 천상 행은 인스턴스가 없어 행 이름으로 풀리고, 타락·독립 행은 판에 인스턴스가 있을 때만 상대다 (§7.5). (v0.7 — 외교 단계와 `InitialRelations`는 Phase 4부터)
`DeposeRuler`·`KillPlayer`는 위기 단계·분쟁 단계(통치 문명의 마지막 성계를 앗아 갈 때)·이벤트 효과·조우 해결에서 `UCosmicEventSubsystem::Apply`를 거쳐, Embodied일 때만 `PendingRoleEffect`로 기록되고(먼저 기록된 것 우선. 출생 보호는 적는 자리에서 가린다. 성계의 위기는 그 성계에 선 방랑자에게만 닿고 통치자는 통치 문명의 절멸로만 죽는다) 틱 경계의 '역할 효과 해소' 단계에서 처리된다(§7.4). `PendingEffect`에 담는 값은 ini에 선언한 태그 `Effect.KillPlayer`·`Effect.DeposeRuler`이고 빈 태그가 "없음"이다(§7.2). 열째 단계는 지금 폐위만 처리하고 `Effect.KillPlayer`는 Phase 4b의 사망 파이프라인까지 기록된 채 남는다(그때까지 사망을 싣는 행은 없다 — `Data.Integrity`). (v0.7 — 분쟁 단계의 기록은 그 단계가 본문을 얻는 Phase 4부터)
영혼·구원·멸망을 세는 필드·컬럼·업적은 없다. 영혼 경제의 유일한 크기는 초월자 영향권(성계 수)이다 (§4.2, §8.3).
`Realm`은 상태가 아니라 정의다: 초월자 소속은 `FTranscendentDef.Allegiance`(`Realm.Abyssal/Celestial/Independent`), `FTranscendentState`에 Realm 필드 없음. `TranscendentID` == `DT_Transcendents` 행 이름 (§4.2, §7.3). 초월자는 판에 교리마다 1체다(은하마다 따로 두지 않는다. 타락은 건너지 않는다) (§4.2).
심도는 상태가 아니라 정의다: `FTranscendentDef.Depth`(0~1) 컬럼 하나이고 영압의 초월자 항에만 (1 + Depth)로 곱한다. 문명·플레이어의 심도는 저장하지 않는다(파생·서술만). 칸(`EAscensionTier`)은 다섯 그대로다 — 칸을 대신하는 등급 타입, 차원 축·정렬 같은 수치, 현현의 열거형을 만들지 않는다 (§3.2 심도, §4.2 현현). 서막은 칸이 아니다(T0 없음, v0.8). 대도감의 에너지 등급은 칸을 대신하는 등급 타입이 아니다(§3.2).
서술자 카테고리 로그 템플릿에 판정 어휘(천국·지옥·구원받·멸망받·영혼·저주받·심판받) 금지 — 지금은 스키마가 서술자 문장의 컬럼과 그 슬롯을 채우는 이름, 인물 이름에서 막고(낱말 목록 `verdict`를 부르는 컬럼의 `forbidden`), 테스트 `Data.NoVerdictWords`는 Phase 5에 온다. 1인칭(`Log.Transcendent`·`Log.Claim`)과 조우 인용문, Chronicle 머리 문장만 예외 (§8.3). 같은 낱말 목록 장치로 다른 작품의 고유명과 신화·성경 고유명사(가칭 `other_works`·`holy_and_myth`)를 낱말 경계·조사 떼기로 막고 이름 3단계 검사를 ST_CA까지 넓힌다(v0.8 — 단계 4. 스키마와 검사기 단위 테스트의 몫 — C++ `Data.Integrity`는 `forbidden`을 읽지 않는다). 대도감의 성간 기록원 인용(카드 안의 이름 없는 인용 목소리)도 판정 어휘·'영혼'을 쓰지 않는다(예외 목록에 들지 않는다).
게임에 보이는 지어낸 이름은 한국어 묘사 이름이다 — 다른 작품의 고유명·조어, 신화의 신, 성경 고유명사(인물·장소·피조물·우상·천사), 실존 인물, 허구에 실제 목록 번호를 쓰지 않고, 교회 낱말을 괴물·세력에 쓰지 않는다(§5.11 '이름의 거름' ①~⑫). 대도감의 원문 이름은 게임에 보이는 글에 쓰지 않는다 — 원형 장부(종족마다 빌린 원형과 쓰지 않는 말을 적는 설계 문서의 표)와 설계 문서의 풀이에만 둔다(§5.11 '대도감').
인물은 `FCivilizationState.Notables`의 기록이다(`FNotableRecord` 5필드 고정: Role·NameIndex·FirstTick·LastTick·SourceRow). 능력치·AI·관계 없음, 개인을 겨누는(ID를 받는) 조건/효과 없음(문명을 대상으로 기록 1개를 만드는 NameNotable과 역할 유무만 읽는 CivHasNotable이 전부). 능력치를 가진 개인은 그릇 하나(`FPlayerEntityState`) (§1.3, §7.2, §12.2).
사람의 수는 UI·로그·연대기에 숫자로 쓰지 않는다(`GetPopulationBand` 등급만). 숫자는 서술자 로그·연대기에서는 주기·비율만 쓰고, 자원·Stability 숫자는 UI에만 쓴다 (§1.3, §8.3). 천체의 수와 거리(별의 수·지름·광년)는 은하 패널·우주 화면의 UI 라벨과 게임 밖 '세계관과 출처' 화면에만 `DT_Galaxies`의 FText 그대로 쓰고(언제나 '약'·'추정') 서술자 로그·연대기·두 책·조우 문장에는 쓰지 않는다. 생명 성계·백성·아는 것의 수는 UI에도 숫자로 쓰지 않는다 (§8.3). (v0.7 — UI 라벨과 화면은 Phase 3부터. `DT_Galaxies` 문안의 '약'·'추정'은 스키마가 지킨다) 성계 패널의 행성 목록(`DT_Planets`, '약')과 태양에서의 거리도 UI 라벨에만(v0.8 — 단계 5). 천체 이름의 숫자(관측 목록의 표기)는 수가 아니다 — UI 라벨에만 쓰고 로그와 백성 이름은 그 백성이 부르는 이름(숫자 없음). UI의 '년'은 표시 단위(1주기 = 1년, 첫 기록부터, 보는 사람의 설정 — 상태·해시 밖)이고 서술·연대기는 주기다.
저장되는 필드의 기본 초기값(멤버 초기화식)은 세이브가 생긴 뒤 바꾸지 않는다(의미 변경 = 마이그레이션): 저장이 클래스 기본 객체 대비 델타라서 초기값과 같은 값은 기록되지 않는다. `FGalaxyState.SaveVersion`의 초기값은 리터럴 1 고정이고 현재 버전은 `FGalaxyState::CurrentSaveVersion`을 `NewGalaxy`와 마이그레이션이 찍는다 (§9.1).
`ComputeStateHash`는 FName·FGameplayTag를 소문자 문자열로 해시한다(`GetTypeHash`·이름 인덱스·구조체 원시 메모리 금지 — 프로세스마다 다르다). `TMap`은 `FNameLexicalLess`로 정렬한 키 순서로만 순회·해시한다. FText와 `GalaxyID`는 해시 밖이다. `ComputeWorldHash`는 거기서 `Knowing`·`Log`·`Chronicle`·`TellRngState`를 뺀 것이다 — 앎이 세계를 바꾸지 않음을 재는 자이고 쿠킹 빌드 비교도 이것으로 한다. 시뮬이 읽는 FText는 없다 (§9.2).
데이터 컬럼의 Tier는 번호 1~5이고 `EAscensionTier` 값은 0~4다(`Tier1_Planetary == 0`). 비교는 `static_cast<int32>(Tier) + 1`, Tier로 인덱싱하는 배열은 `static_cast<int32>(Tier)` (§7.3).
정의 데이터는 `FCosmicDataTables`(`Data/CosmicDataTables.h`)가 고정 경로 `/Game/CA/Data/DT_*`에서 로드한다(설정 에셋 없음). 코드가 이름으로 찾는 행은 `Data/CosmicRowNames.h`의 `CosmicRows::` 상수로만 부른다. 에셋의 원본은 `Tools/data/*.json`·`ST_CA.csv`이고 `.uasset`은 `Tools/make_data_tables.py`의 산출물이다: 데이터는 JSON/CSV를 고치고 스크립트를 다시 돌린다(에디터에서 고친 행은 덮어써진다). 행 구조체를 바꾸면 JSON에 새 컬럼을 적고 다시 돌린다(JSON에 없는 컬럼은 스크립트가 실패로 알린다 — 행 바로 아래 컬럼만이다. 중첩 구조체에서 빠진 멤버는 그 구조체의 멤버가 모두 스칼라일 때만 잡히므로, 배열·구조체 멤버를 가진 중첩 구조체가 처음 나오는 `DT_Encounters`(`FEncounterChoice`)를 더하기 전에 스크립트의 검사를 중첩 구조체까지 넓힌다). 스크립트는 원본의 해시를 `Tools/data/generated.sha1`에 남기고, 테스트 `Data.SourcesInSync`가 원본만 고치고 에셋을 다시 만들지 않은 상태를 잡는다. 행 이름은 세이브에 남으므로(`SourceRow`·`ArchetypeRow`·`BirthArchetypeRow`·`GalaxyRow`) 배포 뒤에는 바꾸지 않는다 (§7.3, §7.4, §9.3, §10.1, §10.4). 정의 데이터의 참조·어휘 규칙은 `Tools/data/schema.json`에 적는다. 검사기 `Tools/cosmic_data_schema.py`가 원본 JSON을 보고(`make_data_tables.py`가 에셋에 손대기 전에 돌린다 — 문제가 있으면 에셋을 만들지 않는다. 에디터 없이는 `python Tools/check_data.py`), 테스트 `Data.Integrity`가 스키마를 코드(행 구조체의 컬럼·열거자·틱당 효과 목록)와 로드한 에셋에 맞춰 본다. 행 구조체에 컬럼을 더하거나 조건·효과 열거자를 더하면 `schema.json`에 그 줄을 적는다(빠지면 `Data.Integrity`가 실패한다). `CosmicRows::` 상수를 더하면 그 테이블의 `required_rows`에 적고, 테이블에 에셋이 생기면 `must_exist`를 켠다(앞의 것은 검사기와 `Data.Integrity`가, 뒤의 것은 `Data.Integrity`가 요구한다). 플레이어를 읽는 조건이나 역할·그릇에 닿는 효과의 열거자를 더하면 스키마의 슬롯 행 규칙과 통치자 업적 행 규칙에 모두 적는다(검사기의 단위 테스트가 요구한다 — 새 효과는 그 두 규칙과 테스트의 세계 효과 목록 가운데 한쪽에 든다). 스키마에 규칙을 더하면 `Tools/test_data_schema.py`에 그것을 깨는 입력을 더한다(숫자의 경계를 거는 규칙은 경계 양쪽의 입력을 손으로 넣는다 — 규칙을 걷는 테스트는 규칙의 피연산자를 따라 움직여 넓힌 규칙을 잡지 못한다). 검사기나 스키마의 꼴을 고치면 `python Tools/test_data_schema.py`를 돌리고 에셋을 다시 만든다(도장에 검사기 파일이 든다). 행 이름은 그 테이블의 접두어를 갖고(`DT_Traits`·`DT_Relics`·`DT_Dedications`의 접두어는 첫 행을 쓸 때 `schema.json`에 정한다) 테이블을 가로질러 유일하다 (§7.3, §10.4). `DT_Events`의 에셋은 첫 이벤트 데이터 작업에서 생겼다(책 세 행과 모든 백성에게 도는 추첨 행 넷). 이름으로 부르는 행(Named)은 코드가 그 자리의 단계에서 `FireRow`로 내고 행 이름은 `CosmicRows::` 상수다 — `event_book_found`는 연구 단계가 Society 돌파마다, `event_grace_remnant`는 마지막 성계의 문이 부른다. 예외 하나: Hazard 행의 컬럼 `OutbreakRow`가 이름을 쥔 행은 그 Hazard가 놓인 뒤 `FollowUpNotes`가 낸다(`event_rebellion_rises` — 이름은 데이터이고, 컬럼의 target이 Named임을 지킨다). `FGalaxyState.EventCooldownUntilTick`의 키는 `"<행>@<CivID>"`(`UCosmicEventSubsystem::CooldownKey`)이고 저장되므로 꼴을 바꾸지 않는다 — 행 이름에 '@'를 쓰지 않는다 (§7.2, §7.5).
DataTable 텍스트의 로컬라이제이션 키는 위치로 정해진다(행·컬럼·배열 인덱스). 번역을 시작한 뒤에는 `Templates`·`NameBank` 같은 배열은 끝에만 추가하고 중간 삽입·순서 변경·삭제를 하지 않는다. 같은 이유로 `DT_` 에셋을 지우고 다시 만들지 않는다(새 패키지는 네임스페이스가 바뀐다 — 스크립트는 기존 에셋을 제자리에서 다시 채운다) (§8.5).
서사 로그는 `UCosmicLogSubsystem::Push`·`PushVariants`만 `State.Log`에 쓴다(`PushRow`는 `PushVariants`의 포장이고 이벤트 행의 줄도 같은 길로 나간다). `Message`는 렌더 결과를 평탄화해 저장한다(`FText::AsCultureInvariant`). `PushVariants`는 호출당 말하기 스트림 추첨이 정확히 1회다(거절된 호출 — 빈 변형, 핸들러 안 — 은 뽑지 않는다): 직전 줄과 같은 문장이 나오면 다음 변형으로 넘기되 다시 뽑지 않는다(범위 밖 Severity 2를 대신하는 먼 줄을 낼 때만 2회). 줄이 쌓이는가는 이야기의 범위가 정한다: 범위 안은 쌓고, 범위 밖의 Severity 2·`Log.Claim`·`Log.Record`는 `bUntold`로 쌓고, 그 밖은 쌓지 않는다. 추첨 수·반환값·쿨다운 키는 줄이 말해졌는지에 좌우되지 않는다. `OnLogEntry`는 틱 단계 도중에도 동기로 발화하고 `bUntold` 항목에는 발화하지 않는다: 핸들러는 상태를 읽고 `SetTimeScale`을 부를 수 있지만 `NewGalaxy`·`LoadGalaxy`·`LoadState`·`SaveCurrentGalaxy`·`StepOnce`·`RunHeadless`와 평가기(`EvaluateAndFire`·`FireRow`·`Apply`)는 부르지 않는다(`NewGalaxy`·`LoadGalaxy`·`LoadState`·`StepOnce`·`RunHeadless`·`EvaluateAndFire`·`FireRow`·`TellHazardLine`·`FollowUpNotes`·`FollowUpKnowing`·`EvaluateFeats`, 앎에 쓰는 `NoteMet`·`NoteKnown`, 로그에 쓰는 `Push`·`PushVariants`(따라서 `PushRow`)와 말하기 스트림의 `DrawTell`, 역할 전환의 `Install`·`DeposeRuler`·`BecomeWanderer`, 부르심의 `ReceiveCalling`(가칭, v0.8 — 단계 6)과 통치자 명령 `StartMegaproject`·`StartNextMegaproject`는 핸들러 안에서 불리면 에러 한 줄을 내고 아무것도 하지 않는다. `SaveCurrentGalaxy`는 §16 #33 ①과 함께 막는다) — `OnEventFired`·`OnTierAscended`의 핸들러도 같다(상태를 갈아 끼우거나 틱을 돌리는 일은 틱 경계인 `OnSimTick`이나 입력에서) (§7.5, §8.1, §8.4, §9.4).
게임 모듈은 모든 타깃에서 부동소수점 정밀 모드다(`Build.cs`의 `FPSemantics = FPSemanticsMode.Precise`. V7 기본값은 에디터 타깃만 정밀, 게임 타깃은 `/fp:fast`). 지우면 게임 바이너리와 테스트·커맨드렛의 해시가 갈릴 수 있다. 같은 이유로 해시에 들어가는 부동소수점 값은 이 모듈 안에서 컴파일되는 코드로만 계산한다: 엔진 모듈의 비인라인 함수는 타깃의 FP 모드를 따른다(예: `FLinearColor::MakeFromHSV8` 대신 시뮬 cpp의 `CivWheelColor`). 해시에 드는 값은 + − × ÷ √만 쓴다 — 삼각 함수를 비롯한 C 런타임 수학 함수(`FMath::Sin`·`Cos`·`Pow`·`Exp` 따위가 부르는 것)를 쓰지 않는다 (§9.2).
시뮬의 일시정지는 `SetTimeScale(0)` 하나뿐이다: Subsystem은 엔진의 월드 없는 틱 패스(`TickObjects(nullptr, …)`)에서 돌아서 `SetGamePaused`와 시간 팽창이 닿지 않는다. 정지는 이월된 시간도 버린다(§16 #40 ③ — 다시 움직이면 남은 틱을 한 프레임에 몰아 돌지 않는다) (§7.4). 서막도 `SetTimeScale(0)`이고 배속 입력이 없다(v0.8 — 단계 6).
UI 라벨은 `FText::FromStringTable(…, EStringTableLoadingPolicy::FindOrFullyLoad)`로 읽는다. 기본 정책은 다른 비동기 로딩 중에 테이블을 비동기로 올려서, 그 사이에 만든 줄에 `<MISSING STRING TABLE ENTRY>`가 남는다 (§8.5).
은하: 상세로 시뮬되는 은하는 고향 은하(`DT_Rules.HomeGalaxyRow`, `gal_milkyway`) 하나다. 성계·문명의 `GalaxyRow`는 `NewGalaxy`가 찍는 `DT_Galaxies` 행 이름이고 NAME_None은 '아직 찍히지 않음'이다(고향으로 읽지 않는다. NAME_None이 뜻을 갖는 곳은 `OriginGalaxyRow` = 토박이뿐). 레인을 세는 모든 규칙은 같은 `GalaxyRow` 안이다 — 다리(`FGalaxyDef.BridgeTicks`)는 레인이 아니다 (§5.9, §7.2).
ID: 성계는 `sol`(성역 — 배열 끝)·`sys_0`(첫 생애의 모성계 — 배열 0번)·`sys_<n>`(배열 번호)이고 모두 영구다(v0.8 — 단계 4부터. 그 전의 코드는 `sol`이 플레이어 모성계), 문명은 `civ_<n>`(패딩 없음, `FGalaxyState.NextCivSerial`, 재사용 없음). 'ID 사전순'은 언제나 `FNameLexicalLess`다(`sys_0`이 다른 `sys_*`보다 앞) (§7.2).
정착된 성계 = 소유이고 `ActiveMegaproject != project_settle`. 수입·`Law.Remnant`의 셈·Hazard 추첨·계승 후보·정착의 인접 판정·마지막 성계의 문이 이 술어를 쓴다. 무주 성계에는 Hazard와 주장이 서지 않는다. 정착 중인 성계는 정착된 성계 없이 남지 않는다 — 마지막 정착된 성계를 잃으면 함께 무주로 놓인다(`CosmicHazards::LetSettlementsGo`) (§5.7). 정착할 수 있는가는 `CosmicMegaprojects::SettleBlockOf` 하나가 답한다 — 거절의 첫 사유(주인 있음 → 죽은 노드 → 레인 → 행 없음 → 행의 칸 → 보유 상한 → 진행 중 정착 → 영향권)를 돌려주고 `CanSettle`은 그 답이 None인 것이다. 명령·AI의 대상·패널의 정착 단추(거절되면 회색 단추와 사유)가 같은 답을 읽는다 (§5.7, §5.12). (v0.8 — 단계 4부터 성역 사유가 사유 순서의 맨 앞이다) Hazard를 슬롯에 놓는 길은 `CosmicHazards::Place` 하나다(추첨·수도·주장 규칙·`SpawnHazard` 효과가 모두 거친다 — 해결·넘기기·파괴도 `Sim/CosmicHazards`의 함수 하나씩이다, §4.1). (v0.7 — 계승 후보는 Phase 4b부터) 성역은 정착·넘김·초기 코어·모성계·`Install`의 대상이 아니다(v0.8 — 단계 4부터).
낱말: '조우'는 조우 카드만. 백성을 처음 알아보는 일은 '만남', 그 판의 첫 만남은 '첫 접촉'. 사다리는 '칸'. 다리는 '문·관문·길'이 아니고, 건넘의 수단은 '방주'가 아니라 '건너는 배들'이다 (§15). '성역'은 태양계, 그 바깥은 '성역 밖'이다(설계 낱말 — 게임 안에는 이름이 없고 '외곽 심연'은 쓰지 않는다). 말이 온 뒤 백성의 목소리는 태양계를 '옛 땅'이라 부른다. 방랑자의 성역 카드는 '지경'이다(방문 위기의 '신을 벗으라'와 다르다). `project_stabilize`·`feat_supernova_stabilized`의 표시명은 '항성 안정화'(v0.8 — 단계 5부터).
