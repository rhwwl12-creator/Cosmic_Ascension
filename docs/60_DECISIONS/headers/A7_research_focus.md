# MVP ⑦ 헤더 초안 — 연구 중점 프리셋

상태: **승인 (2026-10-24, 개발자 "다음작업들 계속 이어서 진행해줘" — §9의 결정 11개 전원 권장안). 승인 전의 기록: 초안 (2026-10-24) — 적대 리뷰(ultracode 워크플로 — 네 관점: 코드의 사실과 될 일·결정론과 상태와 데이터의 규칙·플레이어의 경험과 화면·완결과 테스트와 문서, 관점마다 적대 검증 하나) 발견 68건, 남김 68(확인 46·일부 22, 반박 0), 높음 0·중간 14·낮음 54 — 겹치는 것을 묶어 모두 본문에 넣었다. 리뷰가 바꾼 설계: 분야 목록을 옮기지 않는다(§2.1), 노브의 실행 규칙(§2.3), 안내 줄 아래의 덩이와 머리 줄의 '지금'(§3), 사회가 MVP에서 아무것도 하지 않는다는 사실과 결정 1, 결정의 셈은 판의 끝까지(§1.6·§4), 기준값을 두 걸음으로(§8).** 개발자 결정은 §9.

근거: `docs/50_ROADMAP.md` A ⑦('연구 중점 프리셋')·B2('연구 중점·칙령 9'), `docs/00_VISION.md:15`(30초 고리 — '단추 1개(거대 프로젝트·정착·연구 중점·칙령)')·`:20`(결정 밀도와 MVP의 잣대)·`:32`(MVP 범위)·`:118`, `docs/10_DESIGN/systems.md` §5.4('통치자의 유일한 조작은 4개 영역 간 중점 비율 슬라이더'), 헤더 A2 §4.3('`FocusResearch` … MVP ⑦ 연구 중점 프리셋이 같은 함수를 단추로 부른다')·A3 §6('연구 중점 프리셋(MVP ⑦)이 오면 그 바꿈도 결정으로 센다')·A4 결정 1('모자라면 노브가 아니라 업적 문턱을 다시 정한다')·A6 §0('연구 슬라이더 안내(⑦과 함께)'), 출처는 2026-10-18 렌즈 A1(`docs/archive/analysis_2026-10-18/lenses/lens_A1.md:91` — 'SetResearchFocus를 4프리셋(지혜/부/안전/균형) 단추로')와 브리핑(`…브리핑_2026-10-18.md:154`), `docs/21_DATA_MODEL_REFERENCE.md:615`·`:1462`, `docs/20_ARCHITECTURE.md` §6.3·§9.1·§9.2·부록 '불변 규칙 전문', `CLAUDE.md` 불변 규칙 2·3·4·5·6·7·8·10·11.

## 0. 범위와 전제

**⑦이 하는 일 — 통치자의 둘째 손잡이**: 지금 통치자가 세계에 닿는 길은 프로젝트·정착 단추와 청원의 답뿐이고, 연구의 몫(`FCivilizationState.ResearchFocus`)을 바꾸는 것은 첫 청원 '지혜로운 마음'의 답 한 번뿐이다. ⑦은 그 몫을 아무 때나 고르는 단추 넷(균형·물리·사회·공학)을 다스리는 성계의 패널에 두고, 그 뒤의 명령 하나와 세계 함수 하나를 더한다.

**세계에 닿는다 — 그래서 헤더를 먼저 쓴다**: `ResearchFocus`는 저장되고 두 해시에 드는 세계 상태다. 명령은 틱 밖의 플레이어 입력으로 그 값을 바꾸고, 다음 틱의 연구 단계(규칙 4의 둘째)가 그 몫으로 쓴다. **새 저장 필드·열거자·틱 단계·추첨 지점은 없다**(`SaveVersion` 1, 최상위 멤버 19 그대로). 입력이 없는 판의 세계는 그대로다 — 증명은 `run_checks`의 idle 3,000틱 줄(`summary_line`)이 `-UpdateBaseline` 없이 통과하고, 손으로 돌린 `-run=CosmicBalance -Seed=42 -Ticks=20000`이 기록의 3448516153 / 885018087과 같은 것이다. 효과 본문을 옮긴 것이 청원의 길을 비트 하나 바꾸지 않았다는 증명은 옛 정책 판(`-Focus` 없음)의 `ruler_summary_line`이 그대로인 것이다(§8 걸음 1). 그 뒤 정책 줄만 결정 7로 한 번 바뀐다.

**그대로인 것**: AI는 중점을 바꾸지 않는다(B2) — AI 백성은 생성의 균형을 지니고, 플레이어가 다스리지 않게 된 백성은 통치자가 둔 몫을 그대로 지닌다(결정 6). 연구의 셈(`Spent × 몫 × 0.003`, `Spent = min(비축, 수입 × 0.5)`), 돌파점 넷(0.25·0.5·0.75·1.0), 첫 청원의 세 길(데이터 그대로 — 그 답이 곧 프리셋 하나와 같은 몫이 된다, 결정 2), 분야 목록의 자리(`ResearchFields` — §2.1).

**⑦ 밖**: 슬라이더(네 몫을 손으로 — B2, `systems.md` §5.4의 본 꼴), 초월 중점(MVP의 어떤 규칙도 초월 연구를 읽지 않는다 — §1.3), 사회 연구의 쓸모(지금은 아무것도 열지 않는다 — §1.3, 결정 1), AI의 중점 정책(B2), 칙령(B2), 중점을 바꾸는 값·기다림(결정 4), 판의 속도(⑧ — 결정 9).

**작업 트리의 바깥 수정**: `docs/60_DECISIONS/DECISIONS_LOG.md`의 커밋하지 않은 바깥 수정(빈 첫 줄, 지워진 2026-10-19 A0 항목 13줄)은 개발자의 답 전까지 그대로 두고, ⑦의 기록도 ⑤·⑥처럼 HEAD 위에 인덱스로 올린다.

## 1. 기대는 사실 (2026-10-24, HEAD `b953d36`)

### 1.1 연구의 몫과 셈
- 네 분야는 `Sim/GalaxySimulationSubsystem.cpp:37`의 파일 안 배열 `ResearchFields`(Physics·Society·Engineering·Transcendence — 연구 단계와 돌파 줄 `CosmicRows::LogBreakthrough[]`의 차례, 둘의 길이를 `static_assert`가 묶고 `SocietyField`가 그 둘째를 부른다)다. 생성이 백성마다 `Research`를 0, `ResearchFocus`를 1/4씩으로 넣는다(`:760-765`). 상단 바(`UI/CosmicTopBarWidget.cpp:537-541`)와 다음 칸 표시(`UI/CosmicNextRung.cpp`의 `ResearchLabelKey`)는 제 낱말 표를 따로 든다.
- 연구 단계(`TickResearch`, `:1575-1627`): 소유 성계가 없는 백성은 건너뛰고, `Spent = max(0, min(Stockpile.Insight, IncomePerTick.Insight × 0.5))`를 비축에서 덜어 분야마다 `Research += Spent × ResearchFocus[분야] × 0.003`(1.0에서 멈춤). 돌파점을 넘을 때마다 돌파 줄(다스리는 백성이면 Severity 1), 사회의 돌파 뒤에는 책의 행(`event_book_found`)을 부른다. **셈이 몫에 대해 선형이다** — 몫을 자주 바꾸어도 남는 것은 몫의 시간 평균뿐이다(결정 4의 근거).
- 몫을 바꾸는 유일한 길은 효과 `FocusResearch`(`Events/CosmicEventSubsystem.cpp:877-890`): 문맥의 백성(`Civ` — 없을 수 있다)의 Key 분야가 Value(0 초과 1 이하)를, 나머지가 `(1 − Value) / (키 수 − 1)`씩 — 백성이 없거나, 분야가 아닌 Key·범위 밖 Value·키 하나면 거짓, 아무것도 바꾸지 않는다. 데이터에서 그 효과를 싣는 행은 '지혜로운 마음'(`petition_wise_heart`, 틱 30부터) 하나다: 지혜를 구한다 → 사회 0.55, 부를 구한다 → 공학 0.55, 안전을 구한다 → 물리 0.55. 그 거절과 몫은 `Sim.PetitionAnswer`(`Tests/CosmicPetitionTests.cpp:502-516`), 합 1은 `Sim.PetitionSoak`, 틱당 출처의 거절은 `Events.RoleEffectRules`(`Tests/CosmicEventTests.cpp:2549`)가 본다.
- 명령 `SetResearchFocus`는 `docs/21:1462`의 목록에만 있고 코드에 없다. `docs/21:615`와 `Data/CosmicConditionTypes.h:140-142`는 프리셋이 효과 `FocusResearch`를 쓴다고 적는다(결정 5가 그 문장을 고친다 — §7).

### 1.2 연구를 읽는 곳 (MVP)
- 업적 두 행: `feat_dyson_swarm`(T1 → T2 — `project_dyson` 완성 ∧ 물리 0.5), `feat_three_worlds`(T2 → T3 — 정착 3 ∧ 공학 1.0). `feat_supernova_stabilized`(T2 → T3)는 연구를 읽지 않는다.
- 다음 칸 표시(`UI/CosmicNextRung`)의 연구 부분('물리 50%(23%)' — 채우면 '물리 50%(50%)'), 상단 바의 연구 넷 덩이(`UI/CosmicTopBarWidget.cpp:150-159, 537-541` — 한 글 블록, `Fill`·말줄임 — 유일하게 줄어드는 덩이, 툴팁은 칸 덩이와 같은 '다음 칸까지' `GetNextRungTip`).
- 기본 몫 0.25에서 물리 0.5는 약 1,330틱, 공학 1.0은 약 2,660틱이다(A3 §1의 측정 — 수입이 틱당 약 1). 0.55면 약 600·1,210틱, 0.15면 약 2,220·4,430틱.

### 1.3 초월과 사회는 아무것도 열지 않는다
- 초월: `CivResearchAtLeast`의 데이터 행은 위 둘뿐이고(물리·공학), 초월의 돌파는 줄만 낸다(`systems.md` §5.4의 'Causality 해금'은 아직 코드에 없다).
- **사회**: 돌파마다 `event_book_found`를 부르지만 그 행의 조건은 `CivHasTrait Trait.Faith.BookShut`이고 MVP의 어떤 아키타입 행(`DT_Civs`)도 `Trait.Faith.*`를 지니지 않는다 — 행이 서지 않는다. `systems.md` §5.4의 사회 해금(조약·안정도 회복 속도·칙령 슬롯)은 모두 Phase 4다. 그래서 첫 청원의 첫 길 '지혜를 구한다'(커맨드렛 `-Answer=1`의 답이기도 하다)는 지금 아무것도 열지 않고 물리를 늦춘다(§1.6) — 결정 1.

### 1.4 명령과 패널의 지금 꼴
- 통치자의 명령은 프로젝트의 `UPlayerRoleSubsystem::StartMegaproject`(`Player/PlayerRoleSubsystem.h:67-75`)와 그 프로토타입 키 `StartNextMegaproject`(`:77-80`), 그리고 청원의 답 `AnswerPetition`(`:97`)이다. 프로젝트의 명령: 핸들러 안이면 에러 한 줄과 거짓(`IsBroadcasting`), Embodied ∧ Ruler ∧ 다스리는 백성이 있을 때만, 추첨 없음, 틱 사이의 입력(다음 틱부터). 컨트롤러의 `StartProject`(`World/CosmicGameMode.cpp:1670-1693`)가 끝난 판을 거절하고(`IsRunEnded`), 받으면 HUD와 지도를 바로 다시 그린다. 저장을 바로 부르는 입력은 추첨이 있는 청원의 답 하나다(`RequestSave(Answer)`, `:1638-1642`). 프로젝트 시작은 다음에 저장이 서는 틱 경계(새 판·끝·승천·역할·실제 600초 간격 — `docs/20` §9.1)나 종료 저장이 싣는다.
- 성계 패널(`UI/CosmicSystemPanelWidget.cpp`, 너비 340 — 안쪽 여백 16을 빼면 308, 높이 460): 이름 → 상세 줄 → 표식의 말 → 명령 단추 상자(세로 — 다스리는 백성의 정착된 성계에는 프로젝트 단추, 다른 살아 있는 아는 성계에는 정착 단추, `:633-675`) → 사유 줄 → 안내 줄 `Hint.Commands`(`:186-197` — 첫 프로젝트를 시작할 때까지). 단추는 `UCosmicActionButton`(`OnAction(FName Action, float Value)`, 키보드 포커스를 받지 않는다, 꺼진 단추는 `Press`도 받지 않는다)이고 목록은 행동의 목록이 바뀔 때만 다시 짓는다(`:682-716` — 그 사이의 클릭을 버리지 않으려고). 끝난 판에서는 모든 단추가 회색이고 이름 뒤에 '(멈춘 판)'(`Chronicle.Suffix`), 툴팁이 `Chronicle.Block`이다.
- 상단 바의 '지금 배속' 단추는 꺼짐으로 흐리게 그린다(`UI/CosmicTopBarWidget.cpp:595` — 이미 그것이라 누를 일이 없다). 패널에서는 같은 흐림이 '막힘(사유와 함께)'을 뜻한다 — ⑦의 '지금 중점'은 머리 줄이 말한다(§3.2).

### 1.5 커맨드렛과 기준값
- 정책 인자 `-Answer`·`-Build`(`Tests/CosmicBalanceCommandlet.cpp:438-452`, 문서 주석은 `Tests/CosmicBalanceCommandlet.h`), 틱 뒤의 입력 순서 = 끝의 판정(`:526-553` — 아무것도 멈추지 않는다) → 답 → 프로젝트(`:555-578`), 결정 = 받아들인 답 + 시작한 프로젝트(`:653` — 600·6,000틱에 적는다, 끝 뒤의 입력도 센다), 일곱째 줄 `CosmicBalance ruler:`(`:715-717`). `run_checks.ps1`(두 등급 모두)이 시드 42 · 3,000틱 · `-Answer=1 -Build=1 -TellAll=0` 판의 그 줄을 `seed42.json`의 `ruler_summary_line`과 글자로 견준다. `-UpdateBaseline`은 견주기를 모두 끄고 그 단계들의 칸을 모두 다시 쓴다(한 칸만 고치는 길이 없다).

### 1.6 측정 (실험 스위치 — 커밋하지 않는다)
30판(시드 1~29와 42), 6,000틱, `-Answer=1 -Build=1 -TellAll=0`. 실험 스위치 `-Focus=`·`-FocusShare=`(커맨드렛이 그 백성의 `ResearchFocus`를 직접 쓴다 — 측정 뒤 `git checkout`으로 지웠다). 정책 1 = §4의 '다음 칸의 연구'(T1에서 물리 0.5 전까지 물리, T2에서 공학 1.0 전까지 공학, 그 밖에는 균형), 정책 2 = 같되 균형으로 돌아가지 않는다.

| 판 | 통치 백성 T2 (중앙값, 범위) | T3 (중앙값, 범위, 닿은 판) | 결정 — 6,000틱까지 (중앙값, 범위) | 결정 — 판의 끝까지 | 중점의 바꿈 |
|---|---|---|---|---|---|
| 지금(중점 입력 없음) | 2,203 (2,201~2,587) | 4,425 (2,954~4,819, 26/30) | 19 (8~37) | **12** (8~26) | 0 |
| 정책 1 · 몫 0.55 | **1,165** (1,125~1,195) | **1,958** (1,406~2,273, 30/30) | 28 (14~73) — 끝 뒤의 바꿈을 막으면 27 | **12** (11~16) | 4 (끝 앞) |
| 정책 2 · 몫 0.55 | 1,165 | 2,060 (1,406~2,384, 30/30) | 25 (16~57) | — | 3 |
| 정책 1 · 몫 0.4 | 1,165 | 2,208 (1,406~2,567, 30/30) | 28 (18~55) | — | 5 |
| 정책 1 · 몫 0.7 | 1,165 | 1,804 (1,406~2,082, 30/30) | 31.5 (16~55) | — | 5 |

'판의 끝까지'는 첫 끝(T3 ∨ 6,000)의 틱 경계에서 센 결정이다 — 게임은 끝 뒤의 입력을 받지 않는다(그 측정의 정책은 끝 뒤에 중점을 바꾸지 않는다). '6,000틱까지'는 끝 뒤의 답·프로젝트도 센다(지금의 커맨드렛 그대로).

읽은 것:
- **지금의 T2(2,203)는 물리가 묶는다**: 커맨드렛의 답은 '지혜로운 마음'에서 늘 첫째(지혜 → 사회 0.55)를 고르므로 물리가 0.15로 0.5에 약 2,200틱이 걸린다. 물리에 기울면(0.4 이상 어느 몫이든) T2는 다이슨 스웜이 묶는 약 1,165틱이다 — 그래서 T2는 몫에 좌우되지 않는다. 사람이 청원에서 '안전'(물리)을 고르면 지금도 T2는 약 1,165다.
- **T3는 공학이 묶는다**: '여러 별의 살림'(정착 3 ∧ 공학 1.0)이 T2 뒤 약 800~1,000틱에 선다. 몫은 T3를 1,804~2,208 사이에서만 움직인다 — 몫보다 '중점을 다시 겨눌 수 있는가'가 크다.
- **판이 짧아진다**: 주의 깊은 통치자의 칸 끝은 약 4,400틱에서 약 2,000틱으로 당겨진다 — ×10이면 틱만 약 200초, A4의 어림('결정마다 1분')으로 멈춘 시간까지 약 12~15분(A4의 어림으로 지금은 20분 안팎). '×10 기준 30~60분'(A4 결정 1)이 더 멀어진다 — 결정 9.
- **결정의 셈**: 판의 끝까지 센 결정은 두 판 모두 중앙값 12다 — 중점을 쓰는 판은 바꿈 넷을 더하지만 판이 약 2,000틱 일찍 끝나 답과 프로젝트가 그만큼 준다. **'6,000틱까지 19'는 끝 뒤의 답·프로젝트를 센 수였다**(지금의 커맨드렛이 끝 뒤에도 입력한다): 게임이 받는 판으로 읽으면 목표 ⓑ('6,000틱 결정 ≥ 15')는 지금도 닿지 않는다 — 결정 11. 6,000틱 셈의 19 → 28도 바꿈 다섯(하나는 끝 뒤)과 더 이른 칸이 연 답·프로젝트 약 넷의 합이다.

## 2. 세계 함수와 명령

### 2.1 `Sim/CosmicResearch.h/.cpp` (새 — 순수 함수 namespace, 상태 없음, 추첨 없음)

```cpp
// Cosmic Ascension - Sim layer. Design: docs/10_DESIGN/systems.md section 5.4; headers/A7_research_focus.md (MVP item 7).
namespace CosmicResearch
{
	// Field takes Share and every other key of the map the rest evenly, (1 - Share) / (keys - 1) - the same expression in
	// the same float order as the FocusResearch effect before item 7, so the petition's answer writes the same bits. False
	// and nothing changed when Field is not a key, Share is not above 0 and at most 1 (NaN included), or the map has fewer
	// than two keys. Each key is set from that key alone: no result depends on the map's iteration order (GalaxyState.h).
	COSMIC_ASCENSION_API bool Focus(TMap<FName, float>& ResearchFocus, FName Field, float Share);

	// Every key 1 / keys (the generation's shares: 1 / 4 is an exact float). False and nothing changed for an empty map.
	COSMIC_ASCENSION_API bool Even(TMap<FName, float>& ResearchFocus);

	// What a map is, for the panel, the top bar and the commandlet. Even: two keys or more and every share equal (the
	// all-zero bench maps of old tests too). Focused (OutField = that key): one key strictly above every other key and the
	// others all equal to each other. Mixed: anything else - fewer than two keys, or no such single key (no rule writes such
	// a map yet; the slider of B2 will). Exact float comparisons: the writers above give the equal shares the very same
	// float. Order-free: the answer is a strict unique maximum or none.
	enum class ECosmicFocusShape : uint8 { Even, Focused, Mixed };
	COSMIC_ASCENSION_API ECosmicFocusShape ShapeOf(const TMap<FName, float>& ResearchFocus, FName* OutField = nullptr);
}
```

- **분야 목록은 옮기지 않는다**: `ResearchFields`·`static_assert`·`SocietyField`는 `GalaxySimulationSubsystem.cpp`에 그대로 둔다(내보낸 배열은 컴파일 시간의 길이 검사를 잃고, 다른 번역 단위의 정적 초기화 순서를 부른다). 새 함수들은 지도의 키만 본다 — 분야인지는 '지도의 키인가'다(효과의 규칙 그대로). 생성은 지금처럼 키를 넣고 1/4을 쓴다(바꾸지 않는다).
- `FocusResearch` 효과의 본문은 `return Civ != nullptr && CosmicResearch::Focus(Civ->ResearchFocus, Effect.Key, Effect.Value);`가 된다(거절의 조건과 몫의 식이 지금과 같다 — `Sim.PetitionAnswer`·`Sim.PetitionSoak`·`Events.RoleEffectRules` 그대로).
- 레이어: `Sim/` 안이고 `Core`만 읽는다(`Events`·`Player`·UI·커맨드렛이 부른다 — 의존 방향 그대로).

### 2.2 규칙 노브 하나 (`FCosmicRulesDef`, `Data/CosmicDefinitionRows.h` 끝)

```cpp
// The share a ruler's research focus gives its field (MVP item 7, docs/10_DESIGN/systems.md section 5.4): the field takes
// it and the other fields the rest evenly. The schema keeps it 0.3..1; a value not above 1/4 or above 1 (NaN included)
// cannot focus, and UPlayerRoleSubsystem::SetResearchFocus refuses a field with one error line (the even focus never reads
// it). Start value 0.55 - the opening petition's choices (petition_wise_heart), so its answer is one of the presets.
UPROPERTY(EditAnywhere) float ResearchFocusShare = 0.55f;
```

- 스키마(`Tools/data/schema.json`의 노브 범위 0.3..1)·`Tools/test_data_schema.py`(깨는 입력 0.29와 1.01)·`Data.Integrity`·설정 표(`docs/30_CONFIG_REFERENCE.md` — `make_config_reference.py`)를 함께 고친다. 새 컬럼이므로 **빌드 먼저, 그다음 `make_data_tables`**(`-Data`).
- 청원의 0.55와 노브는 데이터의 두 자리다. 같게 두는 것이 권장이다(결정 2 — 다르게 두면 생기는 엇갈림도 거기).

### 2.3 명령 (`UPlayerRoleSubsystem`, `Player/PlayerRoleSubsystem.h`)

```cpp
/**
 * The ruler's research focus (section 5.4, 7.5; MVP item 7), between ticks: Field = a key of the ruled people's
 * ResearchFocus takes DT_Rules.ResearchFocusShare and the others the rest evenly (CosmicResearch::Focus - the rule of the
 * FocusResearch effect); NAME_None = every key even (CosmicResearch::Even). The next tick's research stage spends by it.
 * Only while Embodied and Ruler with a ruled people that is in the state. False and nothing changed: for a field that is
 * not a key; when the shares would not change (the focus asked is already the people's - not a decision); with one error
 * line for a field while ResearchFocusShare cannot focus (not above 1/4, above 1, NaN - a data fault the schema keeps
 * out); with one error line inside a handler of a Core Logic delegate. No draw, no line, no save of its own (as for
 * StartMegaproject: the next tick boundary that writes, or the exit save, keeps it).
 */
bool SetResearchFocus(FName Field);
```

- '바뀌지 않으면 거짓': 같은 단추를 두 번 눌러도 결정은 하나다(커맨드렛의 셈·`Sim.ResearchFocus`). 비교는 쓰기 전후의 지도 그대로(`OrderIndependentCompareEqual` — 정확한 float).
- 초월(Transcendence)도 명령은 받는다(효과와 같은 규칙 하나 — 키이면 받는다). 단추를 두지 않는 것은 패널의 일이다(결정 1).
- 컨트롤러: `ACosmicPlayerController::SetResearchFocus(FName Field)`(새 public) — `IsRunEnded`면 거절, 받으면 HUD의 `RefreshPanel`(멈춘 시뮬은 틱을 보내지 않는다 — 패널의 '지금 중점'과 상단 바의 툴팁이 바로 바뀐다). 지도는 다시 그리지 않는다(연구는 지도에 없다). 저장 요청 없음.
- 개발용 Exec `CosmicFocus Physics|Society|Engineering|Transcendence|Even` — 컨트롤러의 `SetResearchFocus`를 거친다(끝난 판은 거절), 모르는 낱말이나 거절은 `LogCosmic` 한 줄. 클릭과 같은 만큼만 세계를 바꾼다. `docs/40`의 창 없이 찍는 화면 줄(`-ExecCmds="CosmicSelect sys_0; CosmicFocus Physics; …"`)에 적는다.
- 규칙 5: 핸들러 안의 명령은 에러 한 줄 + 무동작(다른 명령과 같은 문장 — `docs/20` 부록 '불변 규칙 전문'의 핸들러 목록에 더한다).

### 2.4 결정론과 저장
- 추첨 지점이 늘지 않는다 — 명령은 시뮬 스트림을 뽑지 않고(`docs/20` §9.2 '틱 밖의 지점'에 들지 않는다 — 그 목록은 추첨하는 입력이다. 추첨 없는 입력이 하나 더 있다고 그 절에 한 줄), 말하기 스트림·파생 정수도 쓰지 않으며, 앎·로그에 쓰지 않는다.
- 입력은 세계를 바꾼다: 같은 시드·같은 틱의 같은 입력이면 같은 두 해시, 입력이 다르면 세계 해시가 다르다(`ResearchFocus`는 세계 해시에 든다 — 앎이 아니다. `TMap`은 해시가 키 사전순으로 읽는다, 규칙 6).
- 저장: 새 필드 없음. 바뀐 몫은 다음에 저장이 서는 틱 경계(새 판·끝·승천·역할·실제 600초 간격 — `docs/20` §9.1)나 종료 저장(상태 해시가 다르므로 쓴다)에 실린다. 멈춘 동안에는 경계가 없어 종료 저장만 싣는다. 그 사이(길게는 실제 10분)의 강제 종료는 그 바꿈을 다른 모든 것과 함께 잃는다 — 추첨이 없어 '다시 굴리기'가 아니고, 다시 누르면 된다.

## 3. 패널의 덩이와 상단 바 (표현 계층)

### 3.1 자리와 꼴 (결정 3)
- **다스리는 백성의 정착된 성계의 패널**(프로젝트 단추가 서는 같은 문 — `bRules ∧ !bDead ∧ 주인 = 다스리는 백성 ∧ IsSettled`)의 맨 아래에 한 덩이: 차례는 명령 단추 상자 → 사유 줄 → 안내 줄 `Hint.Commands` → **중점 덩이**(위 여백 8 — 안내 줄이 프로젝트 단추 바로 아래에 남아 중점의 단추를 말하는 것으로 읽히지 않는다). 덩이 = 머리 줄(13pt, §3.2) + 가로 한 줄의 단추 넷 **[균형] [물리] [사회] [공학]**(차례 고정 — 분야는 상단 바의 연구 덩이와 같은 차례). 308픽셀에 넷이면 단추마다 약 75픽셀, 글은 두 글자다.
- 중점은 백성 전체의 것이라 그 백성의 어느 정착된 성계에서 눌러도 같은 몫이 바뀐다 — 머리 줄이 '이 성계'라 하지 않는다(툴팁이 '백성의 연구'라 말한다).
- 단추는 따로 짓는 넷이다(명령 단추의 목록과 섞지 않는다 — 패널이 한 번 지어 두고 제자리에서 켜짐·툴팁만 고친다). 처리기도 따로다: `void HandleFocus(FName Field, float Value)`(Value는 쓰지 않는다 — `OnAction`의 꼴), [균형]의 행동은 `NAME_None`, 분야 단추는 그 분야의 FName.
- 높이: 덩이는 약 55 단위다. 가장 붐비는 수도(T2의 위기와 그 대응 단추·영적 시야의 표식 줄·안내 줄)가 460 안에 드는지를 창 없이 찍은 화면(A-7b)과 손 확인 ⑦이 본다. 넘치면 패널을 높인다(`SystemPanelSize` — 표현 계층).

### 3.2 '지금 중점'·머리 줄·툴팁
- **머리 줄이 지금을 말한다**: `Focus.Heading` '연구 중점 — {0}'({0} = `ShapeOf`가 Focused면 그 분야의 낱말(`Research.*` — 초월도), Even이면 `Focus.Even` '균형', Mixed면 `Focus.Mixed` '여러 갈래'). 머리 줄의 툴팁 `Focus.HeadingTip` — '백성의 배움이 어디로 기울지 정한다. 의회의 첫 청에 답한 것도 여기서 바꿀 수 있다. 다음 칸에 드는 연구는 상단 바의 칸에 마우스를 올리면 보인다.'
- 단추: `ShapeOf`가 Even이면 [균형]이, Focused면 그 분야의 단추가 꺼진다(흐림 — 이미 그것이라 누를 일이 없다. 머리 줄이 까닭을 말한다), 나머지는 켜진다. Mixed나 초월에 기운 몫(명령으로만 생긴다)이면 넷 다 켜진다. 첫 청원의 답(0.55)은 Focused라 그 분야가 흐려진다.
- 툴팁(ST_CA — 숫자는 슬롯이고 글에 '%'를 적지 않는다: 슬롯은 `CosmicUI::Percent(몫)` — 0.1499999…은 '15%'다. 시계 단위 낱말을 쓰지 않는다 — 패널의 다른 시간 낱말은 주기/년을 따르지만 이 글에는 때가 없다):
  - {0} = 네 몫을 상단 바의 차례로: '물리 55% · 사회 15% · 공학 15% · 초월 15%'(`CosmicUI::FocusSharesOf` — 노브로 지은 지도를 읽는다).
  - 켜진 분야 단추 `Focus.TipField` — '누르면 백성의 연구가 이렇게 나뉜다: {0}. 쌓인 연구는 그대로다.'
  - 켜진 [균형] `Focus.TipEven` — '누르면 백성의 연구가 넷에 고루 나뉜다: {0}. 쌓인 연구는 그대로다.'
  - 꺼진 '지금' 단추 `Focus.TipNow` — '지금의 연구 중점이다: {0}.'(분야) · `Focus.TipNowEven` — '지금은 연구가 넷에 고루 나뉜다: {0}.'([균형])
  - **다음 칸의 줄**: 분야 단추의 툴팁 아래에, 다스리는 백성의 다음 칸 통치자 업적 행이 그 분야의 `CivResearchAtLeast`를 가지면 `Focus.TipNeeded` '다음 칸의 업적이 이 연구를 본다.' 한 줄(T1의 [물리], T2의 [공학] — 사회와 초월에는 서지 않는다. 칸이 바뀔 때만 바뀌므로 툴팁 객체가 틱마다 새로 나지 않는다). 읽는 함수는 `CosmicNextRung::ResearchFieldsOf`(새 — 다음 칸 행들의 연구 조건, 분야와 채움; 커맨드렛의 정책이 같은 함수를 읽는다 — §4).
  - 끝난 판: 넷 다 꺼지고 툴팁 `Chronicle.Block`, 이름 뒤 꼬리(`Chronicle.Suffix`)는 달지 않는다(75픽셀에 들지 않는다 — 머리 줄이 '연구 중점 — 물리'로 남아 멈춘 지도에서도 지금 중점이 읽힌다).
- 단추 글: [균형] `Focus.Even` '균형'(출처의 낱말 — '고르게'는 '고르다'의 꼴로 읽힐 수 있다), 분야는 기존 `Research.Physics`·`Research.Society`·`Research.Engineering`(상단 바와 같은 낱말 — 누른 분야의 %가 상단 바에서 빨리 오른다).
- 새 키 아홉: `Focus.Heading`·`Focus.HeadingTip`·`Focus.Even`·`Focus.Mixed`·`Focus.TipField`·`Focus.TipEven`·`Focus.TipNow`·`Focus.TipNowEven`·`Focus.TipNeeded`. ST_CA 접두어 `Focus.`를 스키마의 검사에 더한다(`string_table.checked_prefixes` — 판정 어휘와 `{…}` 밖의 숫자).
- 글은 바뀔 때만 준다(`SetToolTipText`는 새 툴팁을 만든다 — 열린 툴팁이 닫히지 않게, 기존 관례). 몫은 입력으로만 바뀌므로 툴팁은 누름과 칸의 바뀜에만 새로 난다.

### 3.3 안내 (결정 10)
A6 §0이 ⑦로 넘긴 '연구 슬라이더 안내'는 머리 줄의 지금·머리 줄의 툴팁·분야 단추의 '다음 칸의 줄'이다 — 새 회색 안내 줄은 두지 않는다(대안은 결정 10).

### 3.4 상단 바
- 연구 덩이의 글은 그대로다(유일하게 줄어드는 덩이라 글을 더하면 년 단위의 T3에서 공학까지 잘린다).
- 연구 덩이에 **제 툴팁**을 묶는다: 첫 줄 `Focus.BarLine` '연구 중점 — {0}: {1}'(머리 줄과 같은 {0}, 네 몫), 빈 줄, 그다음 지금의 '다음 칸까지'(`GetNextRungTip`의 글). 새 `UFUNCTION() FText GetResearchTip() const`를 `Super::RebuildWidget()` 앞에서 묶는다(`docs/45`의 묶인 툴팁 — 툴팁 객체가 틱마다 새로 나지 않는다). 칸 덩이의 툴팁은 그대로 `GetNextRungTip`이다. 다른 성계를 고른 뒤에도 지금 중점이 상단 바에서 읽힌다. 키는 열이 된다(위 아홉 + `Focus.BarLine`).

## 4. 커맨드렛과 기준값 (결정 7)

- 인자 `-Focus=0|1`(기본 0): 1이면 정책의 입력 끝(답 → 프로젝트 → **중점**)에 '다음 칸의 연구' — 판이 끝나지 않았고(`CosmicEndings::ProvisionalEndOf`가 None — 커맨드렛의 `FirstEnd`) Embodied 통치자에게 다스리는 백성이 있을 때만: `CosmicNextRung::ResearchFieldsOf`의 아직 채우지 못한 첫 조건(행 이름 사전순, 조건의 차례)의 분야로 `SetResearchFocus`, 그런 조건이 없으면 `SetResearchFocus(NAME_None)`(균형). 받아들인 바꿈(참)만 센다.
- 결정 = 받아들인 답 + 시작한 프로젝트 + **받아들인 중점의 바꿈**(A3 §6의 약속). `CosmicBalance ruler:` 줄에 `focus N`(인자)·`focus changes N`·**`decisions by end N`**(첫 끝의 틱 경계에서 센 결정 — 끝이 없으면 −1)을 더한다. 목표 ⓑ의 판단은 결정 11.
- `Tests/CosmicBalanceCommandlet.h`의 문서 주석(줄과 정책 인자의 목록)에 `-Focus`와 새 칸을 적는다.
- `run_checks.ps1`의 정책 판에 `-Focus=1`을 더해 명령의 결정론을 빌드를 넘어 본다 → `ruler_summary_line`이 한 번 바뀐다(§8 걸음 1의 두 단계). idle 3,000틱 줄은 그대로다(입력이 없으면 명령은 없다). 바꾸는 문장: `run_checks.ps1`의 머리 주석(3~4행)과 정책 단계의 주석(101~102행), `seed42.json`의 `note`, `docs/40` §9.3의 정책 판 명령 줄(341행)과 기준값 줄.

## 5. 선언 (바뀐 꼴)

| 자리 | 바뀜 |
|---|---|
| `Sim/CosmicResearch.h/.cpp` | 새 파일 — §2.1(분야 목록 없음) |
| `Data/CosmicDefinitionRows.h` | `FCosmicRulesDef::ResearchFocusShare` 끝에 하나 |
| `Player/PlayerRoleSubsystem.h` | `SetResearchFocus(FName)` |
| `World/CosmicGameMode.h` | `ACosmicPlayerController::SetResearchFocus(FName)`(public), 개발용 Exec `CosmicFocus` |
| `UI/CosmicSystemPanelWidget.h` | 덩이(`FocusHeading`·`FocusButtons`(넷)), `HandleFocus(FName, float)`, 테스트의 getter |
| `UI/CosmicTopBarWidget.h` | `GetResearchTip()`(UFUNCTION) |
| `UI/CosmicNextRung.h` | `ResearchFieldsOf`(다음 칸 행들의 연구 조건 — 분야·몫·채움) |
| `UI/CosmicUILabels.h` | `FocusSharesOf`(네 몫의 글), 분야 → 낱말 키(`CosmicNextRung.cpp`의 `ResearchLabelKey`를 옮겨 패널·상단 바·다음 칸 표시가 함께 읽는다) |
| `Events/CosmicEventSubsystem.cpp` | `FocusResearch`의 본문이 `CosmicResearch::Focus`를 부른다(행동 그대로) |
| `Data/CosmicConditionTypes.h:140-142` | 주석 — '개국 청원의 길; 통치자의 중점 명령은 같은 규칙을 `CosmicResearch::Focus`로 쓴다' |
| `Tests/CosmicBalanceCommandlet.h/.cpp`·`Tools/run_checks.ps1`·`Tools/baselines/seed42.json` | §4 |
| `Tools/data/ST_CA.csv`·`schema.json`·`DT_Rules.json` | `Focus.*` 열, 접두어, 노브 |

## 6. 테스트 (새 단언은 변이로 한 번씩 깨뜨린다)

| 테스트 | 단언 |
|---|---|
| `Sim.ResearchFocus`(새, `Tests/CosmicResearchFocusTests.cpp`) | ① 순수 함수: `Focus`의 몫(분야 = 0.55, 나머지 = `(1.f - 0.55f) / 3.f`와 `==`)과 거절(키 아님·0·1.01·NaN·키 하나), `Even`(빈 지도 거절), `ShapeOf` — 균형·물리에 기움·청원의 사회 0.55·{0.3,0.3,0.3,0.1}은 Mixed(하나가 낮은 꼴 — '다르다'와 '높다'를 가르는 변이)·빈 지도와 키 하나는 Mixed·모두 0은 Even·사회 1 나머지 0은 Focused ② 효과와 명령이 비트까지 같은 지도를 낸다(같은 분야·노브 몫) ③ 명령: 물리 → 노브 몫, None → 균형, 같은 중점 다시 → 거짓·그대로, 키 아님 → 거짓, 방랑자·Embodied 아님·상태에 없는 `RuledCivID` → 거짓, 핸들러 안 → 에러 한 줄·거짓 ④ 추첨 없음(`GetRng` 그대로)·말하기 스트림·앎·로그 그대로(멤버를 직접 견준다) ⑤ 다음 틱의 연구가 그 몫으로 쌓인다(물리의 늘음 : 사회의 늘음 = 노브 : 나머지) ⑥ 같은 틱의 같은 입력 두 판 = 같은 두 해시, 입력 없는 판과는 세계 해시가 다르다 ⑦ 노브 0.25·0.2·0·1.01·NaN(테스트 규칙 행)이면 분야 명령이 에러 한 줄과 함께 거절되고 None은 받아들여진다 ⑧ 저장의 왕복: 물리에 기운 뒤 쓰고, 다른 시드의 새 판, 읽기 — 몫이 정확히 같고 `ShapeOf`가 Focused(물리), 두 해시가 쓰기 전과 같으며 물리 명령이 거짓이다(슬롯은 끝에 지운다) |
| `UI.ResearchFocus`(새) | 다스리는 수도의 패널에 머리 줄과 단추 넷(차례·글 — [균형][물리][사회][공학]), 덩이가 안내 줄 아래(자식의 차례), 처음은 [균형]이 꺼지고 머리 줄이 '연구 중점 — 균형'·툴팁이 `TipNowEven`, 컨트롤러의 `SetResearchFocus(물리)`(HUD의 패널 — `Refresh`를 따로 부르지 않는다)로 몫이 바뀌고 [물리]가 꺼지며 [균형]이 켜진다(멈춘 채로 — 배속 0 그대로), 툴팁의 '55%'·'15%'가 한 번씩, 규칙 행의 노브 0.7이면 '70%'·'10%', T1의 [물리]와 T2의 [공학]에만 `TipNeeded`, 초월에 기운 지도와 Mixed 지도는 넷 다 켜짐과 머리 줄의 낱말, 청원의 답(사회 0.55) 뒤에 [사회]가 꺼지고 머리 줄이 '사회', 단추 넷의 객체가 `Refresh`와 방송을 켠 `StepOnce` 뒤에도 같고 바뀌지 않은 툴팁의 Slate 객체도 같다, 다른 백성의 성계·정착 중인 성계·파괴된 성계·방랑자에는 덩이가 없다, 상단 바 연구 덩이의 툴팁 첫 줄이 지금 중점 |
| `UI.RunEnd`(넓힘, `Tests/CosmicChronicleScreenTests.cpp`) | ③ 잠금에 중점의 단추 넷(꺼짐·`Chronicle.Block`·`Press` 무동작·머리 줄은 남음)과 컨트롤러의 `SetResearchFocus` 거절 |
| `UI.ThirdRungSpeed`(넓힘, `Tests/CosmicThirdRungTests.cpp:738-761`) | 연구 덩이의 툴팁이 `GetResearchTip`(첫 줄 중점, 그 뒤 '다음 칸까지')이고 틱이 그 툴팁 객체를 지킨다 |
| `Data.Integrity`·`test_data_schema.py` | 노브 범위(깨는 입력 0.29·1.01), `Focus.` 접두어의 검사 |
| 커맨드렛(`run_checks`) | 옛 정책 판의 줄 그대로(걸음 1의 앞), 그다음 `-Focus=1` 정책 판의 새 `ruler_summary_line` |
| 그대로 | `Sim.PetitionAnswer`(거절과 0.55·0.15)·`Sim.PetitionSoak`(합 1)·`Events.RoleEffectRules`(틱당 출처의 거절), idle 3,000틱 줄 |

옛 테스트들이 손으로 짓는 지도(모두 0, 사회 1)는 `ShapeOf`로 Even·Focused다 — 그 테스트들은 패널의 중점 덩이를 읽지 않는다.

## 7. 문서 (구현의 문서 커밋)

`docs/10_DESIGN/systems.md` §5.4(MVP의 꼴 — 프리셋 넷, 슬라이더는 B2, 사회·초월은 아직 아무것도 열지 않는다), `docs/20` §6.3(성계 패널의 덩이 — 77행의 '성계 패널 맨 아래의 회색 한 줄'은 '명령 단추 아래의 회색 한 줄'로, 상단 바 연구 덩이의 툴팁)·§9.2(추첨 없는 틱 밖 입력이 하나 더)·부록 '불변 규칙 전문'(핸들러 목록), `docs/21`(명령·`CosmicResearch`·노브, 615행의 `FocusResearch` 사용처 — '개국 청원의 세 길; 통치자의 중점 명령은 같은 세계 함수 `CosmicResearch::Focus`로 쓴다(프리셋은 효과를 짓지 않는다)'), `docs/30`(설정 표), `docs/40`(테스트 표, §9.3의 `-Focus`와 결정의 셈과 `decisions by end`, 341행의 정책 판 명령 줄, 기준값 줄, `CosmicFocus`), `docs/15`(연구 중점 / 균형 — Research focus / Even, ST_CA `Focus.`, '위기 포커스'와 다른 말), `docs/50` A ⑦(과 결정 11이 ⓑ를 바꾸면 ⓑ), `docs/00_VISION.md:20`(결정 11이 잣대를 바꾸면)·`:32`, OPEN_QUESTIONS(새 물음 하나 — 초월 중점·사회의 쓸모·슬라이더·AI의 중점은 B2·Phase 4에서, 폐위 뒤 섭정의 중점은 기본 '그대로'·대안 '균형으로'는 Phase 4b에서; #104는 안내 줄의 낱말이 그대로라 바뀌지 않는다), `DECISIONS_LOG`(HEAD 위에), `STATUS.md`, `CHANGELOG`, `CLAUDE.md` 규칙 15의 '현재 코드'에 ⑦ 한 마디(크기 한도 안에서), `Data/CosmicConditionTypes.h:140-142`의 주석.

## 8. 순서·검증·effort

헤더는 xhigh(지금), 리뷰는 ultracode 워크플로(끝났다 — 위 상태 줄), 구현은 high에 ultracode 끔, 문서의 재감사는 ultracode 워크플로.

1. 승인 → (실험 스위치는 측정 뒤 지웠다 — §1.6) → **A-7a** 세계 함수·노브·명령·커맨드렛, 두 단계로:
   - ⓐ `CosmicResearch`·효과의 본문·노브·명령·`Sim.ResearchFocus`만(커맨드렛과 `run_checks`는 그대로): 빌드 → `-Data` → `run_checks.ps1 -Tier full` — idle 줄과 **옛 정책 줄(해시 2307434165, 세계 4271872285, 옛 꼴)이 그대로** 통과해야 한다(효과 본문의 옮김이 비트 하나 바꾸지 않았다는 증명). 손으로 20,000틱(3448516153 / 885018087).
   - ⓑ 커맨드렛의 `-Focus`와 새 칸, `run_checks`의 `-Focus=1`: `-Tier full`(정책 단계만 실패하고 idle 단계는 통과해야 한다) → `-UpdateBaseline`으로 한 번 → `git diff Tools/baselines/seed42.json`이 `ruler_summary_line`·`verified_at`·`note`만 바꾸는지 → `-UpdateBaseline` 없이 `-Tier full`을 한 번 더(두 번 연속 통과). 두 판의 줄을 커밋 본문에 적는다. 변이.
2. **A-7b** 패널의 덩이·컨트롤러·상단 바의 툴팁·ST_CA: 테스트, 변이, 창 없이 찍은 패널 두 장(1280×720 — 처음의 수도, 가장 붐비는 수도).
3. **A-7c** 문서와 재감사.
4. **손 확인 ⑦**(개발자): 1280×720에서 덩이가 패널 460 안에 드는지(가장 붐비는 수도), 꺼진 '지금' 단추가 '고른 것'으로 읽히고 '막힌 것'으로 읽히지 않는지(머리 줄과 함께), [균형]이 '고루 나눈다'로 읽히는지, 안내 줄이 중점의 단추를 말하는 것으로 읽히지 않는지, ×10으로 물리를 눌렀을 때 상단 바의 물리 %가 빨라지는 것이 보이는지.

## 9. 개발자 결정 (권장안 — 1·9·11을 먼저 본다)

1. **프리셋 넷과 사회**. **권장**: 균형·물리·사회·공학 — 물리·사회·공학은 첫 청원의 세 길(안전·지혜·부)이고, 단추 글은 상단 바의 연구 낱말과 같다. 초월은 두지 않는다. **사회는 MVP에서 아무것도 열지 않는다**(§1.3) — 넷은 청원의 길이라서 사회를 남기고, 툴팁은 사회에 아무것도 약속하지 않는다('다음 칸의 줄'이 서지 않는다). 바깥 사람(종료 조건 ⓔ)은 [사회]를 눌러 판이 느려질 수 있다 — '다음 칸의 줄'과 상단 바의 툴팁이 물리·공학을 가리킨다. 대안 ㉮ 셋(균형·물리·공학) — 청원의 '지혜'가 패널에서 고를 수 없는 몫이 된다(머리 줄은 '사회'를 말한다) ㉯ 사회의 작은 쓸모를 당긴다(예: A3가 Phase 4로 미룬 '사회 연구가 안정도 회복을 올린다' — 세계와 기준값 셋이 바뀌는 밸런스 일, ⑧과 함께) ㉰ 청원의 낱말(안전·지혜·부·균형) — 상단 바의 낱말과 갈린다.
2. **몫**. **권장**: 노브 `ResearchFocusShare` 0.55(첫 청원과 같다 — 그 답이 곧 프리셋 하나). 몫은 T3를 1,804(0.7)~2,208(0.4) 사이에서만 움직이고 T2는 움직이지 않는다(§1.6) — 판의 길이를 지키는 손잡이로 쓰기에는 약하다. ⑦ 뒤에는 **첫 청원이 처음의 프리셋만 정한다** — 누름 한 번이 그 답을 바꾼다(지금 판에서 가장 큰 선택이 기본값이 된다 — 머리 줄의 툴팁이 '첫 청에 답한 것도 여기서 바꿀 수 있다'고 말한다). 노브를 0.55가 아닌 값으로 두면: 청원의 답이 같은 분야로 기운 다른 몫이 되어 패널은 그 분야를 '지금'으로 꺼 두는데(꼴로 본다), 명령은 그 분야를 노브 몫으로 다시 쓸 수 있어 커맨드렛은 그 바꿈을 센다 — 그때는 ㉮ `Data.Integrity`가 `DT_Encounters`의 모든 `FocusResearch` Value를 노브와 같게 묶거나 ㉯ 패널이 '노브로 지은 지도와 정확히 같을 때만' 끈다. 0.55면 둘 다 필요 없다. 대안: 0.4(완만)·0.7(가파름) — 그때는 위 ㉮나 ㉯를 함께 고른다.
3. **자리**. **권장**: 다스리는 백성의 정착된 성계의 패널 맨 아래(안내 줄 아래)의 덩이 하나 — 머리 줄이 지금을 말하고, 가로 단추 넷의 지금 것은 꺼진다. 상단 바의 연구 덩이에 제 툴팁(지금 중점과 다음 칸). 대안 ㉮ 상단 바의 연구 덩이를 단추로(누르면 아래로 펼쳐지는 넷 — 상단 바에 자리가 없고 메뉴는 포커스를 쥔다) ㉯ 은하 패널(선택이 없을 때만 보인다 — 은하의 패널이지 백성의 것이 아니다) ㉰ '지금' 단추를 켜진 얼굴에 금빛 글로(`UCosmicActionButton`에 고름의 꼴을 더한다 — 흐림이 '막힘'으로 읽히면 손 확인 뒤에).
4. **값과 기다림 없음**. **권장**: 바꿈은 공짜이고 다음 주기부터 — 연구가 몫에 선형이라 자주 바꾸어도 얻는 것이 없고(툴팁이 '쌓인 연구는 그대로다'라 말한다), 기다림은 저장 필드를 부른다. 줄도 없다(로그는 세계의 일이고, 단추를 오가며 줄이 쌓이지 않게). 대안: 바꿈마다 기다림(저장 필드 `ResearchFocusChangedTick` — 헤더·델타 저장) 또는 통찰 값.
5. **한 규칙**. **권장**: 새 `Sim/CosmicResearch`(`Focus`·`Even`·`ShapeOf` — 지도의 키만 본다)를 효과·명령·패널·상단 바·커맨드렛이 함께 읽고, 명령 `SetResearchFocus(FName)`(None = 균형)은 초월도 받는다. 분야 목록은 옮기지 않는다. 대안: 명령이 효과를 지어 `Apply`를 부른다(문맥을 짓는 일이 명령에 들고, 균형은 'Key 하나에 0.25'라는 꾀가 된다).
6. **AI 그대로**. **권장**: AI는 생성의 균형(B2), 폐위된 백성의 섭정은 통치자가 둔 몫을 그대로 지닌다(세계 상태다 — 지금도 청원의 0.55를 지닌다). 대안: 폐위 때 균형으로 되돌린다(폐위 파이프라인에 한 걸음 — 새 열린 물음에 적고 Phase 4b에서 본다).
7. **커맨드렛과 기준값**. **권장**: `-Focus=1` '다음 칸의 연구' 정책(끝 뒤·다스리는 백성 없음에는 입력 없음), 결정에 중점의 바꿈과 `decisions by end`를 넣고, `run_checks`의 정책 판에 `-Focus=1`을 더해 `ruler_summary_line`을 한 번 바꾼다(§8 걸음 1의 두 단계 — idle 줄 그대로). 대안: `run_checks`는 `-Focus` 없이 두고 명령의 결정론은 `Sim.ResearchFocus` ⑥만 본다(빌드를 넘는 비교가 없다 — 그래도 ruler 줄의 새 칸 때문에 줄은 한 번 바뀐다).
8. **effort**. 구현 high, ultracode 끔(작은 면 — 세계 함수 하나·명령 하나·패널 덩이 하나·툴팁 하나). 문서의 재감사는 ultracode 워크플로.
9. **판의 길이는 ⑧에서**. **권장**: ⑦은 세계의 상수(연구 속도·업적의 조건·다이슨 스웜의 값)를 바꾸지 않는다. ⑧(경제 상수 → DT_Rules)이 판의 속도를 다시 맞출 때 **`-Focus=0`과 `-Focus=1` 두 정책 판을 함께 재고 둘 다에 목표를 둔다**(예: 중점을 쓰는 판의 T3 중앙값 4,000~4,500, 쓰지 않는 판도 6,000 전에 T3에 닿는 판 ⅔ 이상 — ⑧의 헤더가 정한다). 손잡이는 A4 결정 1대로 업적의 조건(문턱)이 먼저이고, ⑧이 경제 상수까지 넓히면 `DECISIONS_LOG`에 적고 `docs/50` ⑧의 범위에 더한다. 까닭: 중점을 쓰는 판만 4,400으로 늦추면 쓰지 않는 판은 같은 비로 6,000을 넘어 대부분 칸 끝에 닿지 못한다 — 중점의 단추를 찾는 것이 종료 조건 ⓒ의 문이 된다. ⑦과 ⑧ 사이의 빌드에서는 주의 깊은 판이 약 2,000틱(×10 멈춤까지 약 12~15분)에 끝난다. 대안 ㉮ ⑦에서 업적의 연구 조건을 올린다(예: 물리 0.75 — 세계와 기준값 셋이 바뀌고, ⑧이 다시 잰다) ㉯ 프리셋에 바꿈의 기다림을 둔다(결정 4의 대안 — 판의 길이를 거의 바꾸지 못한다: T3는 '한 번 다시 겨누는 것'에 묶인다, 정책 2가 정책 1과 100틱 차이).
10. **안내**. **권장**: 새 회색 줄 없이 머리 줄의 지금·머리 줄의 툴팁·'다음 칸의 줄'(§3.2). 대안: 덩이 아래 회색 한 줄 `Hint.Focus`('누른 쪽으로 백성의 배움이 기운다. 다음 칸에 드는 연구는 상단 바의 칸에 마우스를 올리면 보인다.') — 첫 바꿈까지(세션 동안, `FCosmicHintProgress::bChangedFocus`를 컨트롤러의 `SetResearchFocus`가 세운다, MVP ⑥과 같은 꼴). 패널 높이(§3.1)가 그 줄까지 들 때만.
11. **목표 ⓑ의 잣대**. **권장**: ⓑ의 결정을 **판의 끝까지**(`decisions by end`) 세고, 잣대 판을 `-TellAll=0 -Answer=1 -Build=1 -Focus=1`로 고친다(`docs/50` ⓑ와 `docs/00_VISION.md:20`). 그 잣대로 지금은 중앙값 12라 ⓑ가 닿지 않는다 — 판을 늘이는 ⑧(결정 9)이 ⓑ를 함께 본다(판이 약 4,400틱이면 청원 주기 300으로 답이 약 일곱 더 온다 — 어림). ⑦은 ⓑ를 닫지 않는다. 대안 ㉮ ⓑ를 지금 꼴(6,000틱까지, 끝 뒤 포함)로 둔다 — 놀지 않은 틱의 답을 센다 ㉯ ⓑ의 문턱을 판의 길이로 나눈 밀도로 바꾼다(예: '300~600틱마다 결정 1' — 비전의 말 그대로. 지금 중점을 쓰는 판은 약 163틱마다 1이다).

## 추가 이야기와 보완

- 측정에서 본 것: 지금의 커맨드렛 기준 판(중점 입력 없음)은 '청원에서 지혜를 고른 통치자'의 판이라 T2가 물리에 묶여 있었다 — 사람이 '안전'을 고르면 T2는 이미 약 1,165다. ⑧이 두 정책 판을 함께 재는 것(결정 9)은 이 쏠림도 함께 드러낸다.
- 정책 1이 정책 2보다 T3가 약 100틱 이르다: 물리 0.5를 채운 뒤 T2 전까지 균형(공학 0.25)으로 돌아가는 것이 공학을 먼저 쌓는다. '다음 칸의 줄'은 T1에서 [물리]에만 서므로 이것까지 가르치지는 않는다 — 다음 칸 표시가 '물리 50%(50%)'를 보이면 사람도 찾을 수 있다.
- B2의 슬라이더가 오면 `ShapeOf`의 Mixed가 처음 생기고, 패널의 단추 넷은 슬라이더 위의 '빠른 길'로 남는다 — 명령은 `SetResearchFocus(TMap)` 겹이름을 더한다.
- 초월 중점은 'Causality 해금'이 코드에 올 때(Phase 4~5) 다섯째 단추로 함께 본다 — 신학 조율(`docs/90_LORE/`)에서 초월 연구의 뜻(타락 초월자의 길과 닮지 않게)을 먼저 정한다.
- 사회가 Phase 4에 쓸모를 얻으면(칙령 슬롯·안정도 회복) [사회]의 '다음 칸의 줄'과 같은 꼴로 '이 연구가 여는 것' 한 줄을 툴팁에 더한다.
