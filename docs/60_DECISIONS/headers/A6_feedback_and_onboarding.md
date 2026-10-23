# MVP ⑥ 헤더 초안 — 결과를 돌려주는 지표 둘(안정도 추세·위기 포커스)과 첫 안내 세 줄

상태: **승인 (2026-10-24, 개발자 "다음 진행해줘" — §9의 결정 8개 전원 권장안) — 구현 끝(2026-10-24, `[A-6a]`·`[A-6b]` — `git log --grep 'A-6'`; 헤더와 다르게 하거나 더한 것은 `DECISIONS_LOG` 2026-10-24). 손 확인 ⑥은 남았다. 승인 전의 기록: 초안(2026-10-23) — 적대 리뷰(ultracode 워크플로 — 네 관점: 코드의 사실과 될 일·문서 규칙·플레이어의 경험·완결과 테스트, 관점마다 적대 검증 하나) 발견 49건, 남김 48(확인 36·일부 12, 반박 1), 높음 1·중간 22·낮음 25 — 겹치는 것을 묶어 모두 본문에 넣었다.** 개발자 결정은 §9.

근거: `docs/50_ROADMAP.md` A ⑥('Stability 60틱 추세·위기 발생/만료 줄의 카메라 포커스… 온보딩 3줄')와 종료 조건 ⓔ('외부 1명이 설명 없이 30분 뒤 "다음에 뭘 할지 알겠다"'), `docs/00_VISION.md:15`(30초 고리가 돌려주는 것 — '그 결정이 낳은 Severity 1 줄 1개, Stability 60틱 추세, 다음 칸까지 진척')·`:32`(MVP 범위), `docs/10_DESIGN/systems.md:157`(HUD — 'Stability 바 + 60틱 이동평균 추세(▲▼)'), 출처는 2026-10-18 1단계 브리핑(`docs/archive/analysis_2026-10-18/…브리핑_2026-10-18.md:149, 153, 155`)과 렌즈 A1(`…/lenses/lens_A1.md:55, 90, 94` — '조작 안내 세 줄: 상단 바 툴팁에 단축키(0~4·Space), 성계 패널 첫 표시 때 별을 누르면… 한 줄, 첫 카드 3장의 본문이 연구·위기·정착을 설명하게 쓴다'), `docs/20_ARCHITECTURE.md` §6.3·§6.4, `docs/70_CONTENT_GUIDE.md` §8.2·§8.3, `docs/40_DEV_GUIDE.md` §2(표현 상태는 Controller/ini).

## 0. 범위와 전제

**⑥이 하는 일 셋 — 모두 표현 계층**
1. **안정도 추세**: 상단 바의 '안정도 NN%' 옆에 ▲·▼와 툴팁 — 지금 안정도가 지난 60주기 평균보다 높은가 낮은가.
2. **위기 포커스**: 다스리는 백성의 성계에 위기가 새로 서서 자동 정지가 시간을 멈출 때, 그 성계를 고르고 카메라를 보낸다(패널이 위기와 남은 주기를 보인다). 피드의 위기 줄을 누르면 그 성계로 가는 길은 이미 있다(§1.2).
3. **첫 안내 세 줄**: 배속 단추의 툴팁(숫자 키·Space), 이미 있는 '별을 누르면 그 성계를 본다.'(`Galaxy.Hint`), 다스리는 성계의 패널에 한 줄 — 모두 화면의 안내이고 로그가 아니다. 출처의 첫째·둘째를 따르고, 셋째(첫 카드 셋의 본문)는 카드 글 묶음 감사가 따르는 일이라 패널 한 줄로 바꾼다(결정 5).

**그대로인 것**: `FGalaxyState`·두 해시·기준값 셋(`Tools/baselines/seed42.json`)·`SaveVersion` 1·최상위 멤버 19·추첨 수·틱 단계·저장 슬롯. 화면의 상태(표본·안내의 진행)는 위젯과 컨트롤러가 든다 — 저장하지 않고, 판의 상태가 아니며, 해시 밖이다. 증명은 구현 커밋의 `run_checks.ps1 -Tier full -Data`가 `-UpdateBaseline` 없이 통과하는 것이다.

**헤더를 먼저 쓰는 까닭**: 저장 필드는 없지만 셋 다 플레이어가 겪는 동작을 바꾸고(카메라가 저절로 움직인다), '온보딩 3줄'이 두 뜻으로 읽힌다 — 출처는 화면 안내(둘)와 카드 본문(하나)이고, 로드맵 193행·C '로그 튜토리얼'·`WORKPLAN.md` Phase 5는 로그 줄이다(결정 5).

**⑥ 밖**: 안정도 '바'(상단 바에 자리가 없다 — 결정 2), 윤리 삼각형과 인접 초월자 입장(Phase 4), 로그 튜토리얼(Phase 5 C), 연구 슬라이더 안내(⑦ 연구 중점 프리셋과 함께), '첫 함선이 준비되었다'(방랑자 B1), 첫 카드 본문의 다시 쓰기(카드 글 묶음 감사), **반란 문턱(0.3)의 예고** — 안정도가 30% 아래로 가도 화살표는 그 문턱을 말하지 않는다. 반란 자체가 카운트다운이다(§5.10).

**함께 고치는 낡은 문장**: `docs/20_ARCHITECTURE.md:72`의 '(위기의 자동 일시정지는 아직 없다, §2.1)' — MVP ①이 거짓으로 만들었다. `SelectAndFocus`의 주석('아는 성계' → `IsShown` — 성역은 이름으로 보인다).

**작업 트리의 바깥 수정**: `docs/60_DECISIONS/DECISIONS_LOG.md`의 커밋하지 않은 바깥 수정(빈 첫 줄, 지워진 2026-10-19 A0 항목 13줄)은 개발자의 답 전까지 그대로 두고, ⑥의 기록도 ⑤처럼 HEAD 위에 인덱스로 올린다(`git add` 하지 않는다 — 그 13줄이 커밋으로 사라지지 않게).

## 1. 기대는 사실 (2026-10-23, HEAD `f955039`)

### 1.1 상단 바 (`UI/CosmicTopBarWidget.h/.cpp`)
- 안정도는 글자뿐이다: `StatusText` 하나에 `Panel.Stability`('안정도') + `CosmicUI::Percent(Civ->Stability)`(소수 없음 — `FText::AsPercent`)와 자원 다섯(`.cpp:413-419`). 색·표지·툴팁이 없고 `Refresh()`가 매번 다시 짓는다.
- 줄의 차례(`RebuildWidget`, `.cpp:88-216`): 칸 단추 → `StatusText`(14pt, 오른쪽 32) → 연구 덩이(유일하게 줄어드는 칸 — `Fill`, 말줄임) → 시계 → 끝의 예고 → 시야 → 배속 단추 여섯 → 연대기 → 자동 정지 단추 → 까닭 줄. 높이 40, 가로 전체.
- 다시 그림: `NativeConstruct`가 `OnSimTick`·`OnGalaxyGenerated`에 묶여 `Refresh()`를 부르고(`.cpp:221-247`), HUD의 `RefreshPanel`도 부른다(배속·명령·답 뒤). `OnSimTick`은 틱마다 한 번(한 프레임에 많아야 10), `RunHeadless` 동안은 오지 않는다.
- 툴팁의 두 꼴: 한 번 정하는 `SetToolTipText`(시계·시야·자동 정지·×25), 살아 있는 묶음 `ToolTipTextDelegate`(칸과 연구의 public `GetNextRungTip` — Slate 위젯이 서기 전에 묶는다, `.h:86-93`; `Tests/CosmicThirdRungTests.cpp:758-761`이 '틱이 툴팁 객체를 지킨다'를 본다).
- 배속 단추: `CosmicUI::SpeedSteps`(0·1·2·5·10·25)마다 하나, 지금은 ×25만 툴팁(`Speed.ThirdRungTip`, `.cpp:180`)이고 `UI.ThirdRungSpeed`가 그것을 단언한다(`Tests/CosmicThirdRungTests.cpp:273` — ×25의 툴팁이 정확히 그 글, `:275` — ×10은 툴팁이 없다). 숫자 키 0~5가 단계의 차례이고 Space가 멈춤과 이어가기다(`World/CosmicGameMode.cpp:1709-1743`).
- 표현 계층 어디에도 틱마다의 기록이 없다(`LowStabilityTicks`는 0.3 아래의 연속 틱 수일 뿐이다). 위젯은 게임 시작(이어하기 포함)에 새로 난다.

### 1.2 피드·위기 줄·카메라
- 줄을 누르면 `OpenItem` → `SelectAndFocus(FocusSystemID, 없으면 FocusCivID의 수도)`(`UI/CosmicLogFeedWidget.cpp:240-280`). `SelectAndFocus`(`World/CosmicGameMode.cpp:1234-1255`)는 None·지도에 별이 없는 성계·`SelectSystem`이 거절하는 성계(`IsShown`이 아닌 성계)를 거절하고, 받으면 카메라의 팔을 3,000 이내로 줄여 미끄러지게 하고(`FocusOn`, `CosmicCameraPawn.cpp:113-120` — 우주 화면에서도 구역으로 돌아온다) `FollowZoomLevel`을 부른다.
- **안내의 기록 둘이 거기서 선다**: `SelectSystem`이 `Hints.bPickedSystem`(`:1038-1041` — '별을 누르면'이 끝난다), `FollowZoomLevel`이 우주에서 구역으로 오면 `Hints.bLeftUniverse`(`:1116-1134` — `Galaxy.HintHome`이 끝난다; `PlayerTick`도 프레임마다 부른다, `:1372`). 보는 사람이 하지 않은 이동을 적지 않는 길은 이미 있다 — `HandleMapRebuilt`(`:1270-1277`)는 `LastLevel`만 고치고 알린다.
- 위기의 시작·해결·만료 줄은 `TellHazardLine`(`Events/CosmicEventSubsystem.cpp:1239-1278`)이 `FocusSystem`·`FocusCiv`를 달아 말한다(시작 `:1357-1376`, 해결 `:1378-1380`, 만료 `Sim/GalaxySimulationSubsystem.cpp:1933-1935`). 다섯 위기 행이 모두 `Log.Crisis`·Severity 1이고, 다스리는 백성의 성계의 줄은 늘 범위 안이라 피드에 서며 **누르면 이미 그 성계로 간다**(`UI.LogFeed`가 `OpenItem`을 본다).
- 저절로 움직이는 카메라는 없다: `FollowOwnHazards`(`:881-895`)는 `RequestAutoPause(OwnHazard)`만 부르고, `CosmicAutoPause::HasNewHazard`(`World/CosmicAutoPause.cpp:37-48`)는 참·거짓만 낸다. 위기의 끝은 따라가지 않는다. 틱 경계의 차례(`HandleSimTick`, `:664-677`): `FollowSpeedGate` → `FollowEnd(true)` → (끝났으면 돌아간다) → `FollowOwnHazards` → `FollowPetition(true)` → `FollowSaveFailure`. `OnSimTick`은 Core 알림의 괄호 밖이라 거기서 고르고 카메라를 움직여도 된다(규칙 5).
- `RequestAutoPause`(`:921-929`)는 자동 정지가 꺼졌거나 시간이 이미 멈췄으면 아무것도 하지 않는다(지금은 `void`).
- 누르는 중의 입력: 왼쪽 단추를 누른 채(`bLeftDown`) 움직이지 않으면 놓을 때 클릭이 된다 — 더블 클릭은 그 놓음을 삼킨다(`bSwallowRelease`).

### 1.3 안내
- `FCosmicHintProgress`(`UI/CosmicUILabels.h:33-43` — `bPickedSystem`·`bReachedUniverse`·`bPickedGalaxy`·`bLeftUniverse`)는 저장하지 않고 세션 동안 새 판을 넘어 남는다. `CosmicUI::HintKeyOf`(`:261-268`)가 은하 패널의 회색 한 줄(`Galaxy.Hint`·`HintZoomOut`·`HintPickGalaxy`·`HintHome`)을 고른다. 은하 패널은 성계 패널이 서 있는 동안 숨는다. `Galaxy.Hint`의 문구와 존속은 열린 물음 #106(손 확인에서 뺄 수 있다)이다.
- 성계 패널의 명령 단추는 다스리는 백성의 정착된 성계에만 있고(`docs/20` §6.3 67행), 정착 단추는 다른 살아 있는 아는 성계에 있다. 끝난 판에서는 단추가 회색이고 그 아래 줄이 '이 판은 여기까지다 — 명령을 받지 않는다.'다.
- ST_CA의 글 검사는 접두어로 한다(`schema.json` `string_table.checked_prefixes` — 지금 `Chronicle.`·`Save.`, `startswith`로 맞춘다).

### 1.4 첫 10분이 지금 도는 꼴 (×1 기준 초 = 틱)
틱 0 `log_first_life`(Severity 2, `Log.Calling`) · 30 첫 청원 '지혜로운 마음'(자동 정지) · 150 첫 위기(AI 각성, 모든 백성의 수도 — 다스리는 수도에 서면 자동 정지; 같은 틱의 '깨어난 기계' 카드는 기다리는 카드가 없을 때만) · 300 그 만료와 첫 추첨 카드 · 330 첫 눈과 첫 접촉. MVP는 ×10으로 놀아 이 열 줄이 1분 안에 지나간다 — 안내는 틱이 아니라 일(첫 고름·첫 명령)에 묶는다(§4).

## 2. 안정도 추세

### 2.1 정의 (결정 1)
- **보이는 정수 퍼센트로 잰다**: `now` = 지금 안정도의 보이는 정수 %(`CosmicUI::WholePercent` — `Percent`와 같은 반올림, 한 함수를 함께 쓴다), `avg` = 최근 60개 틱 경계 표본의 평균의 보이는 정수 %. **d = now − avg**. d ≥ 1이면 ▲, d ≤ −1이면 ▼, 그 밖은 표지 없음. 화면의 숫자가 1 움직이지 않으면 화살표도 서지 않고, 부동소수의 경계 오차가 없다.
- **왜 평균과 견주나**(60틱 앞의 한 점과 견주지 않고): 한 점과 견주면 계단 하나가 60틱 동안 그대로 남았다가 한꺼번에 사라진다 — 떨어진 뒤 회복하는 동안 숫자가 오르는데 ▼가 60틱 내내 서 있다가 아무 일 없는 틱에 ▲로 뒤집힌다. 평균과 견주면 계단은 60틱에 걸쳐 고르게 옅어지고, 회복이 숫자를 올리는 동안 곧 ▲로 넘어간다. 회복(0.6 아래 틱당 +0.001)만 있는 판에서 d는 약 +3이라 ▲가 선다.
- **아직 모를 때**: 표본이 60개가 안 되면 표지가 없고 툴팁이 기다림을 말한다(§2.4).
- **멈춘 동안의 답**: `now`는 틱 경계의 표본이 아니라 그릴 때의 안정도다 — 시간이 멈춘 채 카드에 답해 안정도가 오르면 HUD의 `RefreshPanel`이 다시 그리는 그 자리에서 ▲가 선다(30초 고리의 되돌림 — 결정마다의 결과가 곧바로 보인다). 표본(평균)은 틱 경계에서만 쌓인다.

### 2.2 표본 (상단 바가 든다)
- 상단 바가 `OnSimTick`마다 다스리는 백성의 안정도를 하나씩 쌓는다(최근 60개). 표본마다 그 틱과 그 백성의 ID를 함께 본다.
- **비우는 때**: `OnGalaxyGenerated`(새 판·게임 안의 로드), 위젯이 새로 설 때(게임 시작·이어하기), 다스리는 백성이 바뀔 때(폐위·즉위 — 표본의 백성 ID가 다르면), 다스리는 백성이 없을 때(방랑자·공백기 — 표지 없음), 틱이 이어지지 않을 때(지난 표본의 틱 + 1이 아니면 — 헤드리스 도약 뒤 따위).
- 판의 상태가 아니다: 저장하지 않고, 이어 연 판은 60틱이 흐른 뒤에야 표지가 선다.

### 2.3 순수 함수 (테스트의 자리)
`UI/CosmicUILabels.h`에 더한다(새 헤더를 만들면 이름이 타깃에서 유일해야 한다):
```cpp
enum class ECosmicTrend : uint8 { Unknown, Up, Down, Flat };   // not a UENUM - a screen value, never saved or hashed
namespace CosmicUI
{
	inline constexpr int32 StabilityTrendTicks = 60;   // docs/10_DESIGN/systems.md:157 - a screen rule, not a knob
	// The shown whole percent of a ratio: the rounding Percent uses (one function for both).
	int32 WholePercent(float Ratio);
	// Samples: the tick-boundary values, oldest first, consecutive ticks of one people. Fewer than StabilityTrendTicks: Unknown.
	// Now: the value as it is drawn (a paused answer counts at once). Up when WholePercent(Now) - WholePercent(mean) >= 1.
	ECosmicTrend StabilityTrendOf(TConstArrayView<float> Samples, float Now);
	int32 StabilityPointsOf(TConstArrayView<float> Samples, float Now);   // that difference in whole points (0 when Unknown)
	// '60 cycles' or '60 years' in the clock's unit (ST_CA Time.Cycles / Time.Year).
	FText DurationLabel(int64 Ticks, ECosmicTimeUnit Unit);
}
```

### 2.4 꼴 (결정 2)
- **안정도는 제 덩이로 뗀다**: 지금의 `StatusText`(안정도 + 자원 다섯)를 둘로 나눈다 — 안정도 덩이(`StabilityText` '안정도 72%'와 그 뒤 고정 폭 칸의 `TrendText` ▲·▼)와 자원 덩이(`StatusText`, 툴팁 없음). 툴팁은 안정도 덩이에만 묶는다 — 자원 숫자 위에 안정도 툴팁이 뜨지 않고, 화살표가 서고 사라져도 자원 숫자가 옆으로 밀리지 않는다. 연구 덩이가 그만큼 줄어든다.
- 표지는 글자다(새 색 없음, 상단 바 글자색 그대로): ST_CA `Panel.TrendUp` '▲'·`Panel.TrendDown` '▼'(소스는 ASCII — 표지는 데이터에 둔다; ▲ U+25B2·▼ U+25BC는 Roboto에 없지만 한국어를 그리는 대체 글꼴 `Engine/Content/Slate/Fonts/DroidSansFallback.ttf`에 있다 — 손 확인 ⑥이 보고, 안 보이면 데이터 두 줄을 '오름'·'내림'으로 바꾼다).
- **툴팁**(살아 있는 묶음 — `GetNextRungTip`처럼 Slate 위젯이 서기 전에 안정도 덩이의 `ToolTipTextDelegate`에 public `GetStabilityTip`을 묶는다). 글(ST_CA `Panel.` — 검사 접두어 밖, 숫자는 `{…}`에만):
  - 오름 `Panel.StabilityUp` '안정도가 지난 {0} 평균보다 {1}%p 높다.' · 내림 `Panel.StabilityDown` '안정도가 지난 {0} 평균보다 {1}%p 낮다.' · 평평 `Panel.StabilityFlat` '안정도가 지난 {0} 평균과 거의 같다.' — {0} = `DurationLabel(60, 시계 단위)`('60주기' / '60년'), {1} = 차이의 절댓값(정수 — 부호는 '높다·낮다'가 말한다, 부호 문자를 쓰지 않는다).
  - 모름 `Panel.StabilityWait` '안정도의 추세는 {0} 뒤에 보인다.' — {0} = 아직 모자란 틱의 길이(`DurationLabel(60 − 표본 수)`; 조사를 받지 않는 꼴이라 '주기'·'년' 어느 쪽도 맞다).
  - 안정도 숫자는 UI에만 쓴다는 규칙(`docs/70` §8.3) 안이다. `DurationLabel`의 주기 꼴은 새 키 `Time.Cycles` '{0}주기'(`Time.Year` '{0}년' 곁).

## 3. 위기 포커스

### 3.1 새 동작 (결정 3)
- 틱 경계에서 `FollowOwnHazards`가 다스리는 백성의 성계에 **새** 위기를 찾고 `RequestAutoPause(OwnHazard)`가 **실제로 시간을 멈췄을 때만**(`bool`을 돌려주게 바꾼다) 컨트롤러가 그 성계를 고르고 카메라를 보낸다. 성계 패널이 위기와 남은 주기를 보인다 — 첫 위기(×1 2:30, ×10 15초)의 '카운트다운 UI 학습'(`docs/00_VISION.md:174`)이 여기서 선다. 카메라는 피드의 누름처럼 팔 3,000 이내로 다가간다.
- 자동 정지가 꺼져 있거나 시간이 이미 멈춰 있으면(카드·연출·다른 정지) 움직이지 않는다 — 흐르는 화면을 빼앗지 않고, 멈춤 하나에 움직임 하나다. 자동 정지 단추의 툴팁에 한 마디를 더한다(`AutoPause.Tip` — '…청원이 오면 시간이 멈춘다. 다스리는 성계의 위기에서는 그 성계를 보인다. 누르면 켜고 끈다.').
- **어느 성계**: 한 경계에 새 위기가 둘 이상이면 `State.Systems`의 배열 차례로 첫 성계(결정적 — `TMap` 차례에 기대지 않는다). 새 순수 함수 `CosmicAutoPause::FirstNewHazardSystem(Before, Now, State)`(없으면 `NAME_None`).
- **보는 사람의 기록을 쓰지 않는다**: 저절로 고른 것은 보는 사람이 고른 것이 아니다 — `SelectAndFocus(SystemID, bool bByViewer = true)`·`SelectSystem(SystemID, bool bByViewer = true)`로 넓히고 이 길은 `false`를 준다. `false`면 `Hints.bPickedSystem`을 세우지 않고, 카메라를 보낸 뒤 `FollowZoomLevel` 대신 `HandleMapRebuilt`처럼 `LastLevel`만 고쳐 알린다(`bLeftUniverse`를 세우지 않는다 — 다음 프레임의 `FollowZoomLevel`도 `LastLevel`이 같아 적지 않는다).
- **누르는 중이면**: 왼쪽 단추를 누른 채 움직이지 않은 동안(`bLeftDown && !bPanning`) 저절로 골랐으면 그 놓음을 삼킨다(`bSwallowRelease` — 더블 클릭과 같은 막음). 놓음이 빈 곳의 클릭이 되어 방금 고른 위기를 지우지 않게.
- **같은 틱의 카드**: 틱 150처럼 위기와 카드가 함께 오면 경계의 차례가 위기(`FollowOwnHazards`) → 카드(`FollowPetition`)라, 위기가 시간을 멈추고 성계가 골라지며 카메라는 카드 뒤에서 다가간다. 카드를 닫으면 패널과 그 별이 이미 거기 있다(까닭 줄은 '다스리는 성계에 위기가 섰다'; 카드를 닫아도 시간은 그 까닭으로 멈춘 채다). 카메라가 다가가는 모습을 카드 뒤에서 놓치는 것은 받아들인다 — 손 확인 ⑥이 본다(결정 3의 대안 ㉰: 카드가 닫힐 때까지 미끄러짐만 미룬다).
- 끝난 판에서는 `FollowOwnHazards`가 돌지 않는다. 우주 화면에서는 구역으로 돌아온다(피드의 누름과 같다).

### 3.2 끝과 해결 (결정 4)
- 위기의 만료·해결 줄은 저절로 카메라를 움직이지 않는다 — 자동 정지도 서지 않고(`docs/20` §6.3), 플레이어는 그 성계의 남은 주기를 패널에서 보아 왔다. 피드의 그 줄을 누르면 간다(이미 있다).

## 4. 첫 안내 세 줄 (결정 5·6·7)

### 4.1 화면의 안내이고 로그가 아니다
- 출처(브리핑 155행·렌즈 A1 94행)의 첫째·둘째 — 단축키 툴팁과 '별을 누르면' — 를 따르고, 셋째(첫 카드 셋의 본문이 연구·위기·정착을 말하게)는 카드 글 묶음 감사가 따르는 일이라 다스리는 성계의 패널 한 줄로 바꾼다. 로그 줄로 쓰면 저장되는 상태가 되어 전체 해시·`ruler_summary_line`의 '말해진 줄'이 바뀌고, 쓰는 자리가 Core라 헤더·기준값 갱신이 따르며, 로그에는 플레이어를 가르치는 목소리의 규칙이 없다(서술자는 3인칭 — `docs/70` §8.2 77행; 2인칭은 `log_first_life`·`log_fallen_monarch`의 '당신'과 연대기의 '네가'가 다음 묶음 감사의 물음이다, MVP ④ 결정 14). 로그 튜토리얼은 Phase 5 C에 그대로 남는다.
- 셋:
  1. **숫자 키** — 배속 단추마다 툴팁 `Hint.SpeedKey` '숫자 키 {0}'(멈춤 단추는 `Hint.PauseKey` '숫자 키 {0} · Space로 멈추거나 이어간다.'; ×25 단추는 그 줄 다음 줄에 지금의 `Speed.ThirdRungTip`). 늘 서 있다 — 한 번 보면 사라지는 줄이 아니다. 끝난 판에서도 그대로다(단추가 회색이고 키가 아무것도 하지 않을 뿐이다).
  2. **별을 누르면** — 이미 있다(`Galaxy.Hint` '별을 누르면 그 성계를 본다.', 보는 사람의 첫 고름까지 — 자동 포커스는 세지 않는다, §3.1). 열린 물음 #106이 이 줄을 지키는 동안 셋 가운데 하나로 센다 — 손 확인에서 빼거나 고치면 ⑥은 그 결정을 따른다.
  3. **다스리는 성계의 패널** — `Hint.Commands` '단추를 누르면 이 성계에서 짓기 시작한다. 다음 칸까지 남은 일은 상단 바의 칸에 마우스를 올리면 보인다.' — 성계 패널 맨 아래 회색 한 줄. **보이는 조건**은 패널이 보이는 것으로 정한다(누가 골랐는지가 아니다 — 자동 포커스로 고른 위기의 패널에도 선다): 보이는 패널에 통치자의 켜진 프로젝트 시작 단추가 하나 이상 있고(정착 중인 성계·회색 단추만 있는 끝난 판·방랑자는 아니다), 그 세션에서 프로젝트를 아직 한 번도 시작하지 않았다. 순수 술어 `CosmicUI::ShowsCommandHint(bPanelHasEnabledProject, bRunEnded, Hints)`. **끝나는 때**: 컨트롤러의 `StartProject`가 성공할 때 `Hints.bStartedProject`를 세운다(패널의 단추든 개발용 키든).
- 글의 규칙: 새 접두어 `Hint.`과 `Galaxy.Hint`를 `string_table.checked_prefixes`에 더한다(판정 어휘와 `{…}` 밖의 숫자를 막는다 — 키 번호는 `{0}`으로 넣는다; `Galaxy.Hint`는 그 아래 넷을 함께 덮는다). `Tools/test_data_schema.py`에 두 접두어의 경우를 함께 적는다(`docs/40` §4). 영적 층(영압·초월자)을 설명하지 않는다(첫 10분 완화 ③ — `systems.md:254`). 명령형 대신 '…하면 …한다'의 평서(지금의 안내 꼴). '칸'은 Tier의 뜻으로만 쓴다(`docs/15`).

### 4.2 기억 (결정 6)
- 세션 동안만(지금의 `Galaxy.Hint`와 같다) — 저장하지 않고 보는 사람의 설정 파일에도 쓰지 않는다. 끝나는 것은 패널의 두 줄뿐이다(`Galaxy.Hint`는 보는 사람의 첫 고름, `Hint.Commands`는 첫 프로젝트 시작에서). 숫자 키의 툴팁은 늘 있다. 새 세션은 두 줄을 다시 보인다.

## 5. 선언 (바뀐 꼴)
- `UI/CosmicUILabels.h`: `ECosmicTrend`, `StabilityTrendTicks`, `WholePercent`(그리고 `Percent`가 그것을 쓴다), `StabilityTrendOf`, `StabilityPointsOf`, `DurationLabel`, `ShowsCommandHint`, `FCosmicHintProgress::bStartedProject`.
- `UI/CosmicTopBarWidget.h`: 안정도 덩이 `UTextBlock* StabilityText`·`UTextBlock* TrendText`(고정 폭 칸)와 그것을 담는 덩이(툴팁의 자리); private `TArray<float> StabilitySamples`(60까지), `int64 LastSampleTick = -1`, `FName SampledCivID`; public `UFUNCTION() FText GetStabilityTip() const`(`GetNextRungTip` 곁), 테스트용 `ECosmicTrend GetStabilityTrend() const`·`const UTextBlock* GetStabilityText() const`·`const UTextBlock* GetTrendText() const`. `HandleSimTick`이 표본을 쌓고, `HandleGalaxyGenerated`가 비운다.
- `World/CosmicAutoPause.h`: `COSMIC_ASCENSION_API FName FirstNewHazardSystem(const TMap<FName, FGameplayTag>& Before, const TMap<FName, FGameplayTag>& Now, const FGalaxyState& State)`.
- `World/CosmicGameMode.h`: `bool RequestAutoPause(ECosmicAutoPause Reason)`(멈췄으면 참), `bool SelectAndFocus(FName SystemID, bool bByViewer = true)`, `void SelectSystem(FName SystemID, bool bByViewer = true)`. `StartProject`의 성공이 `Hints.bStartedProject`를 세운다.
- `UI/CosmicSystemPanelWidget`: 맨 아래 회색 한 줄(`ShowsCommandHint`가 참일 때만).
- `Tools/data/ST_CA.csv` 끝에 `Panel.TrendUp`·`Panel.TrendDown`·`Panel.StabilityUp`·`Panel.StabilityDown`·`Panel.StabilityFlat`·`Panel.StabilityWait`·`Time.Cycles`·`Hint.SpeedKey`·`Hint.PauseKey`·`Hint.Commands`, 그리고 `AutoPause.Tip`의 문구 고침. `schema.json`의 접두어 `Hint.`·`Galaxy.Hint`. ST_CA와 schema.json이 도장(`generated.sha1`)에 들어 `-Data`로 에셋을 다시 만든다.

## 6. 테스트 (새 단언은 변이로 한 번씩 깨뜨린다)
- **새 `UI.StabilityTrend`**: 순수 — 표본 59개면 Unknown; 표본 60개가 모두 0.50일 때 now 0.51이면 Up(51 − 50), 0.504면 Flat(50 − 50), 0.49면 Down — 기대값은 `WholePercent`의 반올림으로 적는다; 떨어진 뒤 회복(표본이 0.60에서 0.50으로 한 번 떨어지고 틱당 +0.001) — 몇째 틱에 ▼가 사라지고 ▲가 서는지 식대로; 오래된 것부터의 차례; `DurationLabel` 두 단위. 산 상단 바 — 새 은하 바로 뒤(틱 전) 표지가 없고 기다림 툴팁; `StepOnce` 60번 뒤 안정도를 손으로 올려 `Refresh`하면(틱 없이 — 멈춘 채의 답) ▲와 '…높다.'; 내리면 ▼; 같으면 표지 없음(평평); 다스리는 백성을 바꾸면 빈다; 틱을 건너뛰면 빈다; 틱이 안정도 덩이의 Slate 툴팁 객체를 지킨다(변이: `Refresh`에서 `SetToolTipText`); 자원 덩이에는 툴팁이 없다; 자리 표시가 없다.
- **`UI.AutoPause` 넓힘(셋째 절)**: 준비 — 새 은하마다 지도의 `RefreshNow`(다시 지은 지도가 고름을 비우고 구역을 잡는다), 안내 진행을 `SetHintProgress(FCosmicHintProgress())`로 비우고 끝에 되돌린다, 끝에 고름을 비운다. 경우 — 자동 정지 켬·×1에서 다스리는 성계의 새 위기 → 그 성계가 골라지고 카메라 목표가 그 성계의 자리이며 `bPickedSystem`은 거짓 그대로; 자동 정지 끔 → 고름이 그대로; 이미 멈춤 → 그대로; 새 위기 둘 → 배열 차례의 첫째; 위기의 끝 → 그대로; 우주 화면에서 → 단계가 구역이 되고 `bLeftUniverse`는 그대로(다음 `FollowZoomLevel` 뒤에도); 누른 채 → 놓음이 고름을 지우지 않는다; **같은 경계의 위기와 카드**(대기 카드 행을 함께 두고 `StepOnce` 하나) → 그 성계가 골라지고 까닭이 OwnHazard이며 카드가 뜬다(변이: `FollowOwnHazards`와 `FollowPetition`의 차례를 바꾼다). 순수 `FirstNewHazardSystem` 넷(없음·하나·둘의 배열 차례·같은 위기).
- **`UI.ThirdRungSpeed` 넓힘**: ×25의 툴팁 = `Hint.SpeedKey`(5) 다음 줄 `Speed.ThirdRungTip`, ×10의 툴팁 = `Hint.SpeedKey`(4)('×10은 툴팁이 없다'를 갈음한다).
- **새 `UI.FirstHints`**: 배속 단추 여섯 모두 툴팁이 있고 자리 표시가 없으며 키를 누른 뒤에도 그대로다; 다스리는 정착된 성계를 고르면 패널의 안내 줄이 서고, 다른 백성의 성계·정착 중인 다스리는 성계·끝난 판에는 없으며, 프로젝트를 한 번 시작하면 사라지고 새 은하에도 다시 서지 않는다(세션); 순수 `ShowsCommandHint`의 경우들.
- **검사기 단위 테스트**: `Hint.`·`Galaxy.Hint` 접두어의 경우(판정 어휘 하나와 `{…}` 밖의 숫자 하나 — 변이로 접두어를 빼면 깨진다).
- 114 → 116종(새 2·넓힘 2). 기준값 그대로.

## 7. 문서 (구현의 문서 커밋)
`docs/20` §6.3(67행 상단 바의 안정도 덩이와 추세·툴팁, 68행 은하 패널 — `Galaxy.Hint`는 보는 사람의 첫 고름까지, 72행의 낡은 문장, 76행 자동 일시정지 문단에 포커스, 안내 셋)·§6.4(101행 선택·카메라·클릭 — 누르지 않은 고름과 카메라 이동) · `docs/40` 테스트 표(새 2·넓힘 2 — 226행 `UI.ThirdRungSpeed`, `UI.AutoPause`)와 §4 접두어 · `systems.md:157`(MVP의 꼴 — '바' 대신 %와 ▲▼) · `docs/00_VISION.md:32`('남은 둘'을 지운다) · `docs/50` A ⑥과 193행('로그 기반 튜토리얼 — MVP ⑥의 화면 안내 셋과 따로') · `docs/15`(추세·안내) · OPEN_QUESTIONS #106(⑥이 `Galaxy.Hint`를 셋 가운데 하나로 센다) · STATUS · CHANGELOG · DECISIONS_LOG(인덱스로).

## 8. 순서·검증·effort
1. 헤더 리뷰(끝 — 위 상태 줄) → 개발자 승인.
2. **[A-6a] 코드와 테스트**(high · ultracode 끔): 이 헤더 파일과 §2~§5, 테스트 넷, ST_CA·스키마 → 먼저 빌드하고 `cycle.ps1 -Data -NoBuild` → 고친 뒤 `run_checks.ps1 -Tier full -Data`(기준값 그대로) → 변이 → 커밋·푸시.
3. **[A-6b] 문서**(xhigh · 단독, 끝에 재감사 1회 — ultracode 워크플로).
4. **손 확인 ⑥**(개발자 — 저장의 손 확인 ⓐ와 함께 해도 된다): 1280×720 창에서 ▲·▼가 그려지고 안정도 덩이와 연구 덩이가 들어가는지, 첫 위기(×1 2:30, ×10 15초)에 카메라가 카드 뒤에서 그 성계로 가는 것이 괜찮은지, 안내 줄이 패널에 들어가는지, 화살표가 반란 앞에서 오해를 낳지 않는지.

## 9. 개발자 결정 (권장안 — 1·3·5를 먼저 본다)
1. **추세의 정의**. **권장**: 지금 안정도(그릴 때의 값)의 보이는 % − 최근 60개 틱 경계 표본 평균의 보이는 %, ±1 이상이면 ▲▼, 표본 60개 전에는 표지 없음 — 멈춘 채의 답이 곧바로 보이고, 계단은 60틱에 걸쳐 옅어진다. 대안 ㉮: 60틱 앞의 한 점과 견준다(이동평균의 기울기 — 계단이 60틱 뒤 한꺼번에 사라지고 회복 중에 ▼가 남는다). 대안 ㉯: 이번 60틱 평균과 그 앞 60틱 평균(표본 120 — 더 매끄럽고 두 배 늦다).
2. **추세의 꼴**. **권장**: '안정도 72% ▲' — 안정도를 제 덩이로 떼고, 글자 하나(데이터의 ▲▼), 새 색 없음, 툴팁에 차이의 크기. 대안: 짧은 막대(`systems.md:157`의 '바') — 상단 바에 자리가 없어 연구 덩이가 더 줄어든다.
3. **자동 포커스의 조건**. **권장**: 다스리는 성계의 새 위기가 자동 정지로 시간을 멈춘 바로 그때만(멈춤 하나에 움직임 하나), 같은 틱의 카드 뒤에서도 그대로. 대안 ㉮: 자동 정지와 상관없이 늘(×10에서 흐르는 화면이 움직인다). 대안 ㉯: 하지 않는다 — 피드의 누름만(지금 그대로). 대안 ㉰: 권장과 같되 같은 틱에 카드가 뜨면 미끄러짐만 카드가 닫힐 때로 미룬다.
4. **끝과 해결의 포커스**. **권장**: 저절로 움직이지 않는다(누르면 간다). 대안: 만료에도 간다(멈춤이 없어 흐르는 화면이 움직인다).
5. **'온보딩 3줄'의 뜻**. **권장**: 화면의 안내 셋 — 숫자 키 툴팁·'별을 누르면'(있다)·다스리는 성계의 패널 한 줄(**출처의 셋째 '첫 카드 셋의 본문'을 바꾼다**) — 상태·해시·기준값 그대로. 대안 ㉮: 로그 줄 셋(Phase 5의 로그 튜토리얼을 당긴다 — 저장되는 줄, 전체 해시와 '말해진 줄'이 바뀌고 Core 헤더가 따른다, 목소리 규칙이 없다). 대안 ㉯: 출처 그대로 — 셋째는 첫 카드 셋의 본문을 연구·위기·정착을 말하게 다시 쓴다(카드 글 묶음 감사가 따른다).
6. **안내의 기억**. **권장**: 세션 동안(지금의 `Galaxy.Hint`처럼). 대안: 보는 사람의 설정(`GameUserSettings` — 한 번 본 컴퓨터에서는 다시 안 보인다).
7. **글의 검사**. **권장**: 새 접두어 `Hint.`과 `Galaxy.Hint`를 스키마의 검사에 넣는다. 대안: 지금처럼 검사 밖의 키로.
8. **effort**. **권장**: A-6a high에 ultracode 끔(표현 계층 — 저장·결정론 밖), 구현 뒤 따로 리뷰하지 않고 A-6b 문서의 재감사(ultracode)가 코드와의 일치도 본다. 대안: A-6a 뒤 구현 리뷰 1회.

## 추가 이야기와 보완
- 리뷰가 바꾼 큰 것: 추세를 부동소수의 띠(±0.01)에서 **보이는 정수 %**로(0.51 − 0.50이 float에서 0.01보다 작아 화살표가 서지 않는 경계), 60틱 앞의 한 점에서 **60틱 평균**으로(계단이 한꺼번에 사라지는 일), 표본만이 아니라 **그릴 때의 값**으로(멈춘 채의 답이 그 자리에서 보인다 — 리뷰가 높음으로 올린 것), 안정도를 **제 덩이**로(자원 위의 툴팁·밀리는 숫자), 자동 포커스가 **`bLeftUniverse`도 적지 않게**·**누르는 중의 놓음을 삼키게**, 셋째 안내의 **문구와 보이는 조건**(다스리는 수도에는 정착 단추가 없다 — '넓힌다'를 뺐다), `UI.ThirdRungSpeed`가 깨지는 것과 그 넓힘, 툴팁 글의 조사와 부호.
