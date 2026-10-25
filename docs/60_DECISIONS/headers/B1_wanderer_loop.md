# B1 헤더 초안 — 방랑자 최소 루프 (이동·스캔·조우 다섯 장·즉위)

상태: **승인 (2026-10-25, 개발자 "다 승인할테니 B작업을 끝까지 이어서 진행해줘" — §10의 결정 22개 전원 권장안).** 승인 전의 기록: 초안 (2026-10-25) — 조사 워크플로(넷이 나란히: 지금의 코드·설계 문서와 결정 기록·조우 카드의 내용과 서사 규칙·결정론과 저장과 테스트 — 사실은 모두 파일의 줄로). 적대 리뷰(ultracode 워크플로 — 네 관점: 코드의 사실과 될 일·결정론과 상태와 데이터의 규칙·플레이어의 경험과 화면과 서사·완결과 테스트와 문서, 관점마다 반박 검증 하나, 빠진 것을 찾는 비평 하나) 발견 82건, 남김 81(확인 50·일부 21·비평 10, 반박 1), 높음 2·중간 37·낮음 42 — 모두 본문에 넣었다. 리뷰가 바꾼 설계: 보기는 눈의 자리 모두에서 한 번에 거리 제한 없이 찾아 나간다(§2.1 — 처음 꼴로는 통치자의 기준값이 바뀌었다), 떠났다 돌아와 같은 카드를 거듭 받는 길을 막는다(§2.5), 주인 없는 성계의 카드(§2.5), 도착 카드의 고르기(§2.5), 왕좌 잠금은 떠날 때 모든 백성에게(§2.7), 옛 백성으로의 복귀(§2.7), 즉위한 첫 틱의 청원(§2.7), 즉위가 앎을 적는다(§2.7), 섭정 기록의 자리와 가드, 같은 틱의 반란 만료(§2.8), 섭정의 업적은 B1 밖(§2.8), 패널은 앎을 뷰로만 읽는다(§2.1·§4), 스키마의 새 시험(§3.5), 커밋 나눔(§8), 20,000틱 기준값을 러너가 본다(§5). 개발자 결정은 §10.

근거: `docs/50_ROADMAP.md` B1('방랑자 최소 루프(TravelTo·스캔·조우 5장·즉위)')과 B의 종료 조건, 옛 Phase 4의 'v0.7 추가'·'v0.8 추가'(`docs/50_ROADMAP.md:179-180`), `docs/00_VISION.md:24`·`:136`·`:174`, `docs/10_DESIGN/systems.md` §5.9(방랑자)·§5.10(역할 전환 — `:163` 양위, `:164` 즉위, `:165` 복귀, `:171` 잠금), `docs/10_DESIGN/crisis_and_transcendents.md:20-34`·`:195-197`, `docs/10_DESIGN/pillars.md:75-93`, 헤더 A2(`A2_petition_cards.md:18`·`:61`·`:123`·`:129`·`:142`)·A0(`:39`)·A4(`:27`)·A5(`:340`)·A6(`:18`), `docs/20_ARCHITECTURE.md` §7.4·§9.1·§9.2·부록 '불변 규칙 전문', `docs/21_DATA_MODEL_REFERENCE.md:263-271`·`:439-441`·`:1101`·`:1455-1477`, `OPEN_QUESTIONS` #11·#35·#53·#60·#67·#80·#81·#83·#87·#107·#118, `DECISIONS_LOG`(폐위 파이프라인 `:191`, 경외의 감쇠 `:188`, 섭정의 업적 `:182`, 반란 만료와 다른 원천 `:185`), `CLAUDE.md` 불변 규칙 2·3·4·5·6·7·8·10·11·13·14.

## 0. 범위와 전제

- **그릇**(플레이어의 몸 — `FPlayerEntityState`)이 통치자의 자리를 떠난 뒤의 짧은 고리를 마우스만으로 한 바퀴 돌 수 있게 한다: **떠난다(양위·폐위) → 레인을 따라 옮긴다 → 스캔한다 → 위기를 만난 백성의 카드에 답한다 → 경외가 쌓이거나 왕좌가 비면 그 백성의 왕좌를 맡는다(즉위), 또는 잠금이 풀린 뒤 옛 백성에게 돌아간다(복귀)**.
- **경외**(Reverence — 백성마다 플레이어를 우러르는 정도 0~1, `FCivilizationState::ReverenceToPlayer`)는 예배가 아니다(`systems.md:143`). 위기를 함께 넘은 백성에게서 오른다.
- **B1에 드는 것**: 명령 넷(`TravelTo`·`ScanCurrentSystem`·`BecomeRuler`·`Abdicate`), 여덟째 단계의 도착 규칙, 방랑자에게 카드를 내놓는 길과 답하는 길, 조우 카드 다섯 장, 왕좌 카드의 셋째 선택지(A2가 넘긴 것), 양위의 파이프라인과 섭정 기록, 옛 백성으로의 복귀(잠금만 본다), 연대기에 방랑의 줄, 패널·지도·상단 바의 방랑자 꼴, '첫 함선' 안내, 커맨드렛의 방랑자 정책과 기준값 한 줄, 러너의 20,000틱 기준값 비교.
- **B1 밖**(뒤의 블록 — §10 결정 22): 여러 레인을 잇는 경로, 수련(`Train`), 유물(`GrantRelic`), `PersonalPool`의 수입, 방랑자 업적과 `PersonalTier`의 오름, T3의 순간 이동 권능, 성역 경계 카드 '지경'(성역에 레인이 없다 — B8), 추방·재회 카드(충신 기록 — B5), 첫 접촉 카드(#24), 주장(Claim)의 방랑자 카드 `enc_witness`(T2 이상), 격차 2의 경외/공포 변형과 3 이상의 현상 카드, '수호신' 특성, 섭정의 업적 규칙·섭정 직접 선택·섭정 표류(`systems.md:163`), 떠나 있던 동안의 다이제스트, 사망·계승(B5).
- **바뀌지 않는 것**: 시드 42 기준값 넷(입력 없는 판과 통치자 정책 판의 3,000틱·20,000틱) — 그 판들에는 방랑자가 생기지 않는다(§1.7). 저장 필드는 하나도 더하지 않는다(`SaveVersion` 1 그대로).

## 1. 기대는 사실 (2026-10-25, HEAD `32aa9ff`)

### 1.1 그릇의 상태 — 예약된 필드

- `FPlayerEntityState`(`Core/GalaxyState.h:336-389`): `Role`(기본 Ruler), `CurrentSystemID`, `TravelTargetSystemID`('NAME_None = 이동 중 아님', `:350`), `TravelTicksRemaining`(0, `:353`), `RuledCivID`, `FormerCivID`, `Body` 1·`Mind` 1·`Causality` 0·`Exaltation` 0(`:366-377`), `PersonalPool`(아무도 쓰지도 읽지도 않는다, `:380`), `Relics`(읽기만), `StatusTags`.
- 이동 두 필드를 쓰는 게임 코드는 없다 — 테스트만 손으로 넣는다(`Tests/CosmicKnowingTests.cpp:587-588`·`:1707-1708`).
- `FCivilizationState`: `ReverenceToPlayer`(`:314-316` — `ChangeReverence` 효과와 폐위의 감쇠가 쓰고 `CivReverenceAtLeast`가 읽는다), `bIsPlayerRuled`, `ThroneLockedUntilTick`('즉위와 복귀는 Tick ≥ 이 값' — 폐위가 쓰고 **아무도 읽지 않는다**, `:321-323`).
- `FCallingState.PersonalTier`는 테스트 밖에서 쓰이지 않는다 — 실제 판의 방랑자는 늘 T1이다(`:553-554`).
- 기다리는 카드는 앎의 `PendingPetitionRow`·`PendingPetitionSystemID`(`:635-645`) 둘이고, 답·`Install`·`BecomeWanderer`가 비운다. `PendingPetitionSystemID`의 주석은 SettledSystem 행과 Civilization 행만 정의한다.
- 역할 효과 칸: `PendingEffect`(태그)·`PendingCause`(효과의 태그 — `Reign.*`·`Death.*`)·`PendingSourceRow`(`:564-577`).
- 생애 기록의 `LastChoiceRows`(그 생애의 마지막 답 셋, `:495-497`)와 재위 세 배열 `RuledCivIDs`·`ReignEndReasons`·`ReignTicks`(늘 같은 길이, `:466-473`·`:506-509`). 여덟째 단계가 다스리는 동안 `ReignTicks`의 끝을 한 틱씩 센다.

### 1.2 이동과 보기의 지금

- 여덟째 단계 `TickPlayerTravel`(`Sim/GalaxySimulationSubsystem.cpp:2143-2161`)은 남은 틱을 하나 줄이고 1 → 0이 되는 틱에 그릇을 목표로 옮긴다 — **레인·죽은 노드·성역·역할을 보지 않는다**.
- 아홉째 단계 `TickSight`(`:2163-2302`): 통치자의 눈은 그 백성의 소유 성계 전부(`FirstSightTick` 330부터), 방랑자의 눈은 선 성계 하나. **모든 눈의 자리를 한 번에 0단에 넣고** 레인을 따라 가까운 성계부터 찾아 나간다(같은 은하 행 안 — 거리 제한 없이 행 전체를, `:2208-2231`). 보는 거리(`SightLanesByTier[칸]`, T1 = 2) 안의 것을 찾은 차례로 `NoteKnown`하고(`:2236-2257`), 만남은 아는 성계 전부를 눈에서의 레인 수(제한 없는 값)로 가까운 순으로 `NoteMet`한다(`:2259-2301`). `KnownSystems`의 차례는 전체 해시에 든다. 헤더 주석이 스캔을 예약해 두었다(`.h:461-462`).
- `BecomeWanderer`는 선 성계와 그 1레인 이웃을 `NoteKnown`한다(`KnowAround`, `Player/PlayerRoleSubsystem.cpp:28-47`) — 그래서 **방랑자의 레인 이웃은 언제나 아는 성계다**. 파괴된 성계는 이웃의 레인 목록에서 빠진다.
- `CosmicHazards::Destroy`는 파괴된 성계에 **선** 그릇을 살아 있는 첫 이웃으로 옮기고, 그런 이웃이 없으면 그대로 둔다(`Sim/CosmicHazards.cpp:378-405`, #53 ② — `DECISIONS_LOG.md:191`). 이동 필드는 건드리지 않는다.

### 1.3 카드 장치 — 지금은 통치자만

- 청원은 `FEncounterDef`(`Data/CosmicDefinitionRows.h:381-429` — `Trigger` Lottery·Standing·Named, `Subject` Civilization·SettledSystem, 선택지 2~3, 판정 능력치·난이도·비용)를 쓰고, 행의 주석은 이미 '방랑자 조우와 청원이 함께 쓴다'고 적는다.
- `CosmicPetitions`의 모든 입구가 다스리는 백성(`RuledCivIndexOf`)을 거친다 — 내놓기·`StillStands`·답·위젯·통치자 정책(`Events/CosmicPetitions.cpp:132`·`:159`·`:250`, `Player/PlayerRoleSubsystem.cpp:363`, `UI/CosmicPetitionCardWidget.cpp:153`·`:203`, `World/CosmicRulerPolicy.cpp:18`). `Holds`는 백성이 없으면 거짓(`:115-118`). 자리에 관한 카드인지는 `Subject == SettledSystem` 다섯 곳이 묻는다(내놓기의 자리 기록, `StillStands`, 답의 결과 줄 초점, 사슬 카드, 위젯의 주제).
- 고르기는 파생 정수 `CosmicSeed::DeriveForName(Seed, CivID, Purpose::Petition = 10, Tick)` 하나로 `Pick = 정수/2^32 × 무게 합`(`:221-243`) — 시뮬 스트림을 뽑지 않는다. 서 있는 행(Standing)은 그 틱에 막 선 단위(행, 성계)만 온다(틱 첫머리의 `StandingHeld`와 견준다 — `:165-173`, `A2_petition_cards.md:53`). 주기 카드(Lottery)는 마지막 답 셋을 빼고, 비면 A8의 폴백으로 가장 최근의 Lottery 행 하나만 뺀다(`:184-216`) — 이 셈은 역할을 가리지 않는다.
- 답 `AnswerPetition`(`:348-448`)은 다스리는 백성이 없으면 거절하고, 비용을 그 백성의 비축에서 보고, 받아들인 답마다 시뮬 스트림을 정확히 한 번(0~5) 뽑는다. 결과 줄은 `Push`라 말하기 스트림을 뽑지 않지만, 뒤따르는 줄(위기를 끝낸 해결 줄, 새 만남의 줄)은 뽑는다(`Events/CosmicEventSubsystem.cpp:1269`·`:1370-1371`). A2가 약속한 `CosmicPetitions::Answer`는 없고 몸체가 함수 안에 있다.
- 카드 위젯은 다스리는 백성의 비축으로 낼 수 있는지 본다(`UI/CosmicPetitionCardWidget.cpp:207`) — 방랑자에게는 모든 선택지가 회색이 된다.
- 데이터: `DT_Encounters` 10행 모두 `petition_*`·`PlayerRoleIs Ruler`. 스키마는 모든 행에 `PlayerRoleIs`를 요구하지만 `has_type`은 종류만 보고 Key(역할)는 보지 않는다(`Tools/cosmic_data_schema.py:1090-1092`). 행 이름의 접두어를 보는 시험은 없다. 텍스트 칸은 `{Civ}`·`{System}`·`{Hazard}`·`{Rebel}`만.
- `PersonalPool`은 들어오는 길이 없다 — 방랑자 카드의 비용은 낼 수 없다. 판정 값 `CheckValueOf(Stability)`는 다스리는 백성의 안정도를 읽는다.
- `FHazardDef.WandererResolutions`(행 이름 목록, 예약 — `docs/21:717`)는 모두 비어 있고 시뮬 코드가 읽지 않는다.

### 1.4 경외·왕좌 잠금·폐위·섭정

- 폐위(`DeposeRuler`, `:177-221`): '몰락한 군주' 줄 → 섭정 성향 한 번 뽑기 → `BecomeWanderer(Reign.Deposed)` → **모든 백성**의 경외 × `ReverenceDecay`(0.5) → 옛 백성의 안정도 ≤ `PostRebellionStability`(0.15) → 옛 백성의 `ThroneLockedUntilTick = Tick + ThroneLockTicks(600)`. 그릇은 선 자리에 남는다.
- 역할 효과의 처리(`ResolvePendingRoleEffect`, `:2304-2319`)는 `Effect.DeposeRuler`만 다루고 원인 태그를 보지 않는다.
- 지금 경외를 올리는 것은 다스리는 백성에게 거는 청원 두 행(효과 셋 — `petition_plague` +0.1, `petition_exalt` +0.2·−0.05)뿐이다 — **다른 백성의 경외가 오르는 길이 없다**. 그래서 즉위 조건 '경외 ≥ 0.6'에 닿을 수 없다.
- 즉위 조건(`systems.md:164`): '대상 문명의 경외도 ≥ 0.6, 또는 안정도 < 0.2(지도자 공백). 그리고 `Tick ≥ ThroneLockedUntilTick`', '즉위는 만난 백성에게만'(`:169`). 복귀(`:165`): '옛 문명 존속 ∧ 잠금 경과' — 경외를 보지 않는다.
- **잠금이 하는 일**: 옛 백성의 잠금은 폐위 뒤 첫 약 50틱(안정도 0.15 → 0.2, MVP ③의 회복 0.6 아래 틱당 0.001) 동안 '지도자 공백' 길로 곧바로 다시 오르는 것을 막는 유일한 문이다(`systems.md:171`의 까닭). 그런데 같은 공백의 길이 **다른 백성에게는 잠금 없이 열려 있다** — 잠금이 옛 백성의 필드에만 적히기 때문이다.
- 섭정 기록: 태그 주석(`Config/DefaultGameplayTags.ini:93`)과 `docs/21:1457`은 `Notable.Regent`를 `BecomeWanderer`가 `Reign.Abdicated`(와 `Reign.Died`)로 만든다고 적는다(인물 훅 ① — 아직 코드에 없다). `NameNotable`에는 가드가 있다: 넷째 칸 이상·이름 없는 백성·`Trait.Mind.NoSelf`에는 기록이 없고, 같은 역할의 열린 기록을 먼저 닫고, 상한(`MaxNotablesPerCiv` 4)과 퇴출이 있다(`Events/CosmicEventSubsystem.cpp:278-368`).
- 반란 만료의 왕좌 교체(`ChangeThrone`, `Sim/GalaxySimulationSubsystem.cpp:345-387`)는 플레이어가 다스리는 백성이면 **그 반란이 적은 폐위가 기록된 효과일 때만** 왕좌를 바꾼다 — 다른 원천이 먼저 적었으면 왕좌는 그대로이고 열린 Rebel 기록도 그대로 남는다. 이 경우는 '그 원천이 처음 오는 작업'에 넘겨져 있다(`DECISIONS_LOG.md:185`).
- **섭정의 업적**: 지금의 판정은 플레이어가 다스리지 않는 모든 백성을 모든 Ruler 행으로 판정하고(`Ascension/AscensionSubsystem.cpp:58-63`·`:94`), 업적 세 행이 모두 Ruler 길이다(`Tools/data/DT_Feats.json:8`·`:30`·`:51`). '통치자 전용' 표지는 없다. `systems.md:163`은 다이슨 스웜·항성 안정화를 통치자 전용이라 하고, `DECISIONS_LOG.md:182`는 '섭정이 통치자 전용 업적을 하지 않는 규칙'을 자진 이탈과 함께 오기로 미뤘다.

### 1.5 방랑자에 닿는 길 — 거의 닿지 않는다

- 입력 없는 판의 첫 폐위는 MVP ⑧ 전 30판에서 가장 이른 것이 틱 6,090(`A4_provisional_end.md:42`), A-8b 뒤의 폐위 시드는 1·7·13·18·20·29(가장 이른 틱은 재지 않았다 — B-1d에서 함께 잰다), 시드 18은 10,406(`A8_economy_and_pace.md:78`) — MVP의 임시 끝(틱 6,000) 뒤다. 통치자 정책 판 30판은 6,000틱 전에 한 번도 폐위되지 않았다(`STATUS.md:12`).
- 이른 길은 왕좌 카드(`petition_keep_throne` — 반란이 50틱 안에 끝날 때 서는 행)의 첫 선택지가 판정에 지는 것 하나다. A2가 셋째 선택지 '명예로운 양위'를 B1에 넘겼다(`A2_petition_cards.md:18`·`:142`).
- 그러니 스스로 떠나는 길이 없으면 손 확인도, 테스터도 방랑자 고리에 거의 닿지 않는다.

### 1.6 표현 계층

- 패널은 '체화된 통치자가 아니면 명령이 없다'(`UI/CosmicSystemPanelWidget.h:80`), 모든 프로젝트 단추가 `StartProject`로 간다(`.cpp:545-553`). 테스트가 읽는 단추 목록은 `GetActionButtons` 하나다(`:112`, `Tests/CosmicPanelTests.cpp:256-265`·`:457`).
- 지도에는 그릇의 자리 표시가 없다(`World/`·`UI/`에서 `Player.CurrentSystemID`를 읽는 곳이 없다).
- 상단 바: 방랑자는 시계·보기·배속·자동 정지만(`UI/CosmicTopBarWidget.h:33-34`).
- 자동 정지: `OwnHazardsOf`는 방랑자에게 아무것도 내지 않는다(`World/CosmicAutoPause.cpp:16`). '내 위기' 기억 `OwnHazardsSeen`은 `TOptional`이고 새 은하에서 `Reset()`(아직 본 적 없음)으로 되돌아간다(`World/CosmicGameMode.cpp:771`). 카드의 정지(까닭 `Card`)는 역할을 보지 않는다(`:1700-1724`).
- 연대기의 거름 `Admits`(`Log/CosmicChronicle.cpp:106-115`)는 `Log.Calling`·`Log.Claim`·`Log.Record`·모든 등급 2와, 등급 1은 그 생애의 백성(태어난 백성·다스린 백성)에 초점을 둔 줄과 첫 접촉 줄만 받는다 — 방랑자가 남의 백성과 겪은 줄은 빠진다. 연대기 쪽의 생애 사실 줄은 `RuledCivIDs`의 끝 하나를 이름 댄다.

### 1.7 기준값 — 방랑자가 생기지 않는 판들 (2026-10-25 측정)

| 판 | 폐위 | 근거 |
|---|---|---|
| 입력 없는 판 3,000틱 | 없음 | `Saved/Logs/checks.log`의 `vessel deposed at -1` |
| 통치자 정책 판 3,000틱 | 없음 | `checks_ruler.log` 같은 줄. 온 카드는 `wise_heart`·`awakening`·`road`·`feast` — 왕좌 카드는 오지 않았다 |
| 입력 없는 판 20,000틱 | 없음 | A-8b 뒤 폐위된 시드에 42가 없다(`A8_economy_and_pace.md:72`) |
| 통치자 정책 판 20,000틱 | 없음 | 이 헤더를 위해 쟀다(2026-10-25): `deposed at -1`, 해시 3928640348 / 2194209278(MVP ⑨의 값과 같다), 온 카드 100장 가운데 왕좌 카드 0 |

러너는 3,000틱 둘만 견준다 — 20,000틱 둘은 `seed42.json`의 메모(입력 없는 판)와 헤더 A9(정책 판)에만 있다.

### 1.8 문서끼리 어긋난 것 (이 헤더가 고친다 — §7)

1. A0(`A0_sanctuary_first_life.md:39`)은 `TravelTo(sol)`을 '에러 한 줄 + 무동작'이라 하고, `systems.md:129`·`docs/21:1471`·#81은 '지경' 카드로 끝난다고 한다. 성역에는 레인이 없고(B8이 정한다) 이름으로만 보인다.
2. `systems.md:127`의 '`TravelTo`는 아는 성계만'은 여러 레인의 경로를 뜻하지만, 상태는 목표 하나와 카운트다운 하나뿐이고 여덟째 단계는 앎을 읽을 수 없다(규칙 2).
3. 불변 규칙 2는 앎을 읽는 Core를 '청원의 답 명령'(기다리는 카드)으로만 적는다 — `BecomeRuler`(만난 백성)는 새 독자다.
4. `docs/50:144`만 '조우의 생성'을 시뮬 스트림의 추첨 지점으로 적고 '골든 해시가 없어'도 낡았다(`docs/20:191`의 '조우 판정'은 답의 수라 맞다).
5. '도약'이 T3의 순간 이동 권능(`systems.md:127`)과 공백기의 시간선 도약(`docs/15_GLOSSARY.md:89`) 둘에 쓰인다.
6. `docs/00_VISION.md:24`의 '반란 성계의 즉위'와 `systems.md:164`의 '안정도 < 0.2(지도자 공백)'.
7. `pillars.md:75-76`은 T1 방랑자에게 '모성계 내 탐사·첫 함선'만, '레인 이동·조우'는 T2에 둔다 — `systems.md:127`에는 칸의 문이 없다. 폐위된 첫 생애는 T1이다.
8. `docs/40:39`와 `A5_save_minimum_notes.md:653`은 변이 패치를 `Tools/mutations/`에 둔다고 적지만 그 폴더는 없다.
9. `docs/20:272`과 `Core/GalaxyState.h:638`의 주석은 '틱 단계는 앎을 읽지 않는다'고 적지만, 지금도 앎 단계(`TickSight`가 아는 성계·만난 백성을 보고 새것만 적는다)와 틱 첫머리의 `StandingHeld`·앎 단계 끝의 `Present`(기다리는 카드 칸이 비었는지)가 읽는다 — 그 답은 앎에만 쓰인다.
10. '이동 중 조우 확률'(`docs/00_VISION.md:136`·`systems.md:127`)과 이 헤더의 '이동 중에는 카드가 없고, 도착한 틱의 도착 카드'.
11. `crisis_and_transcendents.md:22`(역병의 방랑자 응답 '유물 사용')·`:34`(`enc_rebel_parley` +0.2, 섭정 직접 선택)·`:195`('청원 카드는 통치자의 것 — 방랑자는 줄과 스캔으로 만난다')·`:197`(자진 이탈은 틱 밖)과 이 헤더.

## 2. 세계 함수와 명령

### 2.1 `Sim/CosmicWanderer.h/.cpp` (새 — 순수 함수 이름공간, 상태 없음, 추첨 없음)

`CosmicMegaprojects::SettleBlockOf`처럼 **한 창구**: 명령과 화면이 같은 답을 읽는다. 문은 앎을 직접 읽지 않고 앎의 답을 인자로 받는다 — 명령은 `State.Knowing`에서 채우고(입력의 정의), 패널은 `FCosmicKnownView`에서 채운다(표현 계층이 앎을 읽는 곳은 `KnownViewOf` 하나 그대로 — 불변 규칙 14).

```cpp
namespace CosmicWanderer
{
    // The first reason a TravelTo is refused, None = accepted. Not saved: the panel says the first reason in this order
    // (the vessel's three states first; the sanctuary is the first reason about the target).
    enum class ETravelBlock : uint8 { None, NotWanderer, Travelling, CardWaiting, Sanctuary, NotNeighbour };
    // The first reason a coronation or a return is refused.
    enum class ECrownBlock : uint8 { None, NotWanderer, Travelling, CardWaiting, NotStanding, NotSettled, Unmet, ThroneLocked,
                                     Lacking };

    // Travelling = TravelTargetSystemID != NAME_None (the field comment) - every gate, card and withdrawal reads this one test.
    bool IsTravelling(const FGalaxyState& State);

    // TravelTo's gate: an embodied wanderer, not travelling, no card waiting, the target not the sanctuary, a living lane
    // neighbour of the system the vessel stands in (lanes stay inside a galaxy row; a destroyed system leaves its
    // neighbours' lanes). Reads no knowing: a wanderer's lane neighbours are always known (KnowAround, sight reach >= 1).
    ETravelBlock TravelBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, FName TargetSystemID, bool bCardWaiting);

    // BecomeRuler's gate on CivID, the owner of the settled system the vessel stands in: an embodied wanderer, not
    // travelling, no card waiting, standing in one of CivID's settled systems, bMet, Tick >= CivID's ThroneLockedUntilTick,
    // and - unless CivID is the vessel's FormerCivID (a return: the lock alone, systems.md:165) - Reverence >=
    // CoronationReverence or Stability < CoronationStabilityBelow.
    ECrownBlock CrownBlockOf(const FGalaxyState& State, const FCosmicStateIndex& Index, const FCosmicRulesDef& Rules, FName CivID,
                             bool bMet, bool bCardWaiting);

    // One look outward from every place of an eye at once: all places on level 0 in the given order, then breadth first
    // over Lanes in array order, inside the galaxy row - the same walk as TickSight today (whether a look passes through
    // the sanctuary is #80, decided by the stage-5 header). Not cut at a reach: returns the order in which systems were
    // found and, per system index, the lanes from the eye (INDEX_NONE = not reached). The caller cuts at its reach
    // (NoteKnown) and orders the meetings by LanesFrom (NoteMet nearer first).
    struct FCosmicSightWalk { TArray<int32> Found; TArray<int32> LanesFrom; };
    FCosmicSightWalk SightWalkOf(const FGalaxyState& State, const FCosmicStateIndex& Index, TConstArrayView<int32> EyePlaces);
}
```

- **`SightWalkOf`로 옮기는 것은 비트 그대로여야 한다**: 통치자는 소유 성계 전부를 지금의 차례로, 방랑자는 선 성계 하나를 넘긴다. 동작을 바꾸지 않고 함수만 옮기는 커밋(§8의 `[B-1a]`)에서, 옮기기 전 HEAD로 시드 42·7의 (a) 통치자 2,000틱(`FirstSightTick` 330 뒤, 소유 성계가 둘 이상인 틱을 지난다) (b) 방랑자로 바꾼 그릇이 100틱마다 레인 이웃으로 옮기는 2,000틱을 돌려 `KnownSystems`·`MetCivs`(ID와 틱)의 CRC를 테스트 상수로 적어 두고, 옮긴 뒤 같은 값임을 단언한다(`Sim.SightWalkMoved`). 기준값 넷도 함께 그대로여야 한다.
- 거절 사유 열거형은 저장되지 않는다(차례는 화면의 말이 읽는다).
- 개발용 `CosmicSeeAll`에서는 뷰가 모든 백성에게 차례를 주므로 패널의 [왕좌를 맡는다]가 켜져도 명령이 `Unmet`으로 거절할 수 있다 — 그때 패널은 명령이 돌려준 사유를 보인다.

### 2.2 규칙 노브 여섯 (`FCosmicRulesDef` 끝에 덧붙임 — `Data/CosmicDefinitionRows.h`, `DT_Rules` 값·스키마의 `rules` 표·`test_data_schema.py`·설정 표와 함께)

| 노브 | 값 | 쓰는 곳 |
|---|---|---|
| `LaneTravelTicks` (int32, 1 이상) | 20 | `TravelTo` — 레인 하나를 지나는 틱(`systems.md:127`의 '기본 20'). 1 아래면 1로 읽는다(두 이동 필드는 늘 함께 서고 함께 빈다). 거리는 B8이 실제 하늘과 함께 다시 정한다(#83) |
| `ScanSightBonusLanes` (int32, 0 이상) | 1 | 스캔의 보는 거리 = `SightLanesByTier[PersonalTier] + 이 값`(`systems.md:131` — `docs/21:1101`의 자리는 끝으로 옮긴다) |
| `EncounterChancePerArrival` (float, 0~1) | 0.25 | 도착한 틱에 도착 카드가 올 몫(§2.5) |
| `CoronationReverence` (float) | 0.6 | 즉위의 경외 문턱 |
| `CoronationStabilityBelow` (float) | 0.2 | 즉위의 지도자 공백 문턱 |
| `AbdicationStabilityLoss` (float) | 0.1 | 양위가 옛 백성에게서 덜어 내는 안정도(§2.8) |

노브는 상태가 아니라 저장되지 않는다.

### 2.3 명령 (`UPlayerRoleSubsystem`, 틱 밖의 입력)

```cpp
// Out of the tick. One lane: TravelBlockOf must say None. Sets TravelTargetSystemID and TravelTicksRemaining =
// max(1, LaneTravelTicks). No draw, no line. Refused (state untouched) in a handler and on a block.
CosmicWanderer::ETravelBlock TravelTo(FName SystemID);

// Out of the tick. One look from the system the vessel stands in through SightWalkOf, cut at
// SightLanesByTier[PersonalTier] + ScanSightBonusLanes: NoteKnown in found order, then NoteMet nearer first - the
// knowing stage's two writes. No sim draw. Tells log_scan_wild once (tell stream) when the vessel stands in a system no
// people owns and the look learned at least one system. Returns how many systems it learned (0 = nothing new: the panel
// says so, no line). Refused for a ruler, while travelling, in a handler.
int32 ScanCurrentSystem();

// Out of the tick. Crowns the vessel over CivID when CrownBlockOf is None (bMet from Knowing.MetCivs - the input
// definition). A return when CivID is FormerCivID. No draw. Section 2.7.
CosmicWanderer::ECrownBlock BecomeRuler(FName CivID);

// Out of the tick. Records the abdication as the role effect Effect.DeposeRuler with cause Reign.Abdicated and source
// row log_throne_vacated through UCosmicEventSubsystem::Apply (the ruled people as the context); the tenth stage of the
// next tick runs the pipeline (section 2.8). No draw here. Refused unless an embodied ruler with no role effect pending,
// in a handler.
bool Abdicate();
```

- **앎을 읽는 곳(불변 규칙 2를 정확히)**: '명령은 입력의 정의로 앎을 읽을 수 있다(기다리는 카드·만난 백성). 틱 안에서 앎을 읽는 곳은 앎에 쓰는 곳뿐이다 — 앎 단계(`TickSight`가 아는 성계·만난 백성을 보고 새것만 적는다), 틱 첫머리의 `StandingHeld`와 앎 단계 끝의 카드 내놓기(기다리는 카드 칸이 비었는지). 그 답은 앎에만 쓰인다. 그 밖의 틱 단계와 모든 조건·효과는 앎을 읽지 않는다.' — `docs/20:272`, `CLAUDE.md` 규칙 2, `docs/21:439`, `GalaxyState.h:638`의 주석을 코드보다 먼저 이 문장으로 고친다. `TravelTo`는 앎을 읽지 않는다.
- **저장**: 넷 다 추첨하지 않는다 — 곧바로 저장하지 않는다(10분 자동 저장과 종료가 받는다). 즉위·복귀·양위로 역할이 바뀌면 다음 틱 경계의 `Role` 비교가 저장한다(`A5_save_minimum.md:340`).
- **핸들러 안의 거절**: 불변 규칙 5의 목록(`GalaxySimulationSubsystem.h:251-259`, `docs/20`)에 `TravelTo`·`ScanCurrentSystem`·`BecomeRuler`·`Abdicate`와 열째 단계의 `AbdicateRuler`를 더하고(방송 중이면 에러 한 줄 + 무동작, 추첨 없음), 가드는 이미 있으나 목록에 빠진 `AnswerPetition`·`SetResearchFocus`도 함께 적는다.
- **끝난 판**: 컨트롤러가 거절한다(MVP ④의 래치 — 지금의 명령과 같다).
- **이동 중에 다시 고르기**는 없다: 이동 중에는 `TravelTo`를 거절한다(한 레인 20틱).

### 2.4 여덟째 단계 — 도착의 규칙과 파괴

- 남은 틱이 1 → 0이 되는 틱에 **도착을 검사**한다: 목표가 그 틱에도 `CurrentSystemID`의 레인 이웃이면(파괴된 성계는 이웃에서 빠진다) 그릇을 옮긴다. 아니면 이동을 끝내고 그릇은 선 자리에 남는다 — 줄도 추첨도 없다. 어느 쪽이든 두 이동 필드를 비운다. 이 단계는 앎을 읽지 않는다.
- `TickPlayerTravel`은 '이 틱에 도착했다'를 지역 값으로 돌려준다(저장하지 않는다) — 아홉째 단계의 카드가 읽는다(§2.5). 파괴가 옮긴 것은 도착이 아니다.
- **출발지가 파괴되면**: 그릇은 이동 중에도 출발지에 선 것으로 센다(지금의 코드). `CosmicHazards::Destroy`가 그릇을 옮길 때 **두 이동 필드도 비운다**(#53 ②의 닫힌 규칙에 '이동 중'을 더한다 — `DECISIONS_LOG.md:188`이 넘긴 경우).
- **갇힌 방랑자**: 살아 있는 이웃이 없어 그 자리에 남은 그릇은 옮길 곳이 없다 — 받아들인다(드물다). 패널의 [이리로 간다]가 서지 않고, 상단 바의 툴팁이 '갈 레인이 없다'고 말한다(§10 결정 21).
- 통치자는 이동하지 않는다(`TravelTo`가 거절하므로 두 필드가 비어 있다). 즉위·복귀·`Install`·`BecomeWanderer`가 두 필드를 비운다.
- 테스트 `Sim.SightStage`는 레인 이웃이 아닌 목표를 손으로 넣고 도착을 기대한다(`CosmicKnowingTests.cpp:580-596`) — 레인 이웃으로 고친다(뜻은 그대로).

### 2.5 방랑자에게 카드를 내놓는 길 (아홉째 단계의 끝)

- **자리**: 지금의 `CosmicPetitions::Present` 자리(아홉째 단계의 끝, 체화된 동안만). 역할에 따라 갈린다 — 통치자는 지금 그대로, 방랑자는 아래. 카드 칸은 하나다(`Knowing.PendingPetitionRow`·`PendingPetitionSystemID`를 두 역할이 함께 쓴다 — 필드 이름은 저장의 일부라 바꾸지 않고, 주석과 문서의 '청원 카드'를 '기다리는 카드'로 고친다).
- **주제**: `ECosmicEventSubject`에 `VesselSystem`을 **끝에 덧붙인다** — 자리에 관한 카드다. 주제 성계 = 그릇이 선 성계(정착된 성계든 무주든), 그 성계의 주인 백성 = 카드가 말하는 백성(없으면 없음). 새 도움 함수 `CosmicPetitions::VesselSubjectOf(State, Index) → {SystemIndex, CivIndex(주인이 없으면 INDEX_NONE)}` 하나를 내놓기·`StillStands`·답·카드 위젯이 함께 쓴다. `Holds`는 `CivIndex`가 없어도 조건을 평가한다(조건의 뷰는 은하 행을 성계에서 읽고, 백성을 읽는 조건은 거짓). `Subject == SettledSystem`으로 자리를 묻는 다섯 곳(§1.3)은 `SettledSystem ∨ VesselSystem`으로 읽고, `VesselSystem`의 주제 성계는 주인과 정착을 묻지 않는다. `PendingPetitionSystemID`에는 그릇이 선 성계를 적는다(그 필드의 주석과 `CosmicPetitions.h:94`를 고친다).
- **이동 중에는 카드가 오지 않는다** — 도착한 틱부터 다시 본다.
- **서 있는 행(Standing)**: 그 틱에 막 선 `enc_` 행 — 틱 첫머리에 방랑자의 단위(행, 그릇이 선 성계)로 `StandingHeld`를 함께 잡아 두고 끝에서 견준다. 도착한 틱에는 틱이 시작할 때 선 성계와 앎 단계 끝에 선 성계가 달라 새 성계의 위기가 '막 선' 것이 된다. 서 있는 위기 곁에 머무는 동안은 다시 오지 않는다. 방랑자가 된 자리에 이미 선 위기도 카드가 되지 않는다 — 떠났다 돌아오면 온다(§10 결정 13).
- **같은 카드를 거듭 받지 않는다**: 방랑자에게는 그 생애의 **가장 최근에 답한 `enc_` 행**을 내놓지 않는다(서 있는 행·도착 카드 모두). 그래서 레인 하나를 오가도 같은 위기의 카드가 곧바로 다시 오지 않고, 다른 카드에 한 번 답한 뒤에는 다시 올 수 있다. 저장 필드 없이 `LastChoiceRows`를 읽는다. 통치자의 주기 카드가 빼는 '마지막 답 셋'과 A8의 폴백이 견주는 '가장 최근의 Lottery 행'은 **`petition_` 행만** 센다 — 즉위한 뒤 `enc_` 행이 통치자의 셈을 흐리지 않는다(배열은 하나 그대로).
- **도착 카드(Lottery)**: 서 있는 행이 오지 않았고 도착한 틱일 때만. 파생 정수 하나 `D = DeriveForName(Seed, 그릇이 선 SystemID, Purpose::Encounter = 11, Tick)`, `u = D / 2^32`. `u < EncounterChancePerArrival`일 때만 온다. 고르기는 다시 편 값 `u / EncounterChancePerArrival`(0 이상 1 미만) × 무게 합의 자리로 한다(행은 이름의 사전 차례 — 청원과 같다). `CosmicSeed::Purpose`에 `Encounter = 11`을 끝에 덧붙인다. 한 성계에 위기 칸이 하나이고 카드 1~4는 서로 다른 위기를 보므로 B1에서 서 있는 `enc_` 행이 둘 서는 일은 없다 — 서게 되면 같은 `D`로 무게 고르기를 한다.
- **격차**: 주인 백성이 있는 카드(1~4 — 글에 `{Civ}`가 없어도)는 그 백성과의 격차(백성의 칸 − `GetViewerTier()`, `NoteMet`과 같은 셈)가 1 이하일 때만 온다 — 세계 상태로 셈하고 앎을 읽지 않는다. 내놓기 전에 그 백성을 `NoteMet`한다(`docs/70:43`). 이것은 `pillars.md:93`의 격차 2 변형 규칙의 자리다 — 불변 규칙 14의 '격차 3 이상은 만나지 않는다'(세 곳을 함께 고치는 규칙)와 다른 문턱이라 그 목록에 넣지 않고, #107의 행에 'Core 술어가 생기면 방랑자 카드의 격차(1 이하)도 같은 칸 차 함수를 부른다'를 덧붙인다.
- **시뮬 스트림은 뽑지 않는다**: 카드를 내놓는 것은 앎에만 쓴다 — 답하지 않은 판의 세계 해시는 그대로다(`enc_` 행을 비운 판과 세계 해시가 같다 — §6).

### 2.6 답 — 하나의 답 명령 (`AnswerPetition`을 넓힌다)

- 이름은 그대로 두고(컨트롤러·자동 정지·저장 길이 이미 그 이름을 쓴다) 몸체도 명령에 둔다. 두 역할이 함께 쓰는 것 — 주제(`VesselSubjectOf`/`SubjectOf`), 가드, 판정 값 — 만 `CosmicPetitions`의 순수 함수로 둔다(A2가 약속한 `CosmicPetitions::Answer`는 만들지 않는다 — `A2_petition_cards.md:61`을 고친다).
- **누가 무엇에 답하는가**: 행의 `Subject`로 가른다 — `VesselSystem` 행은 방랑자만, 그 밖은 통치자만(스키마가 `enc_` ⇔ `VesselSystem`을 묶는다). 맞지 않으면 거절(추첨 0 — `Sim.PetitionAnswer`의 단언이 그대로 참).
- **방랑자의 답**: 비용은 보지 않는다(`enc_` 행의 비용은 스키마가 0으로 묶는다). 판정 값은 그릇의 능력치(`Stability` 판정은 스키마가 막는다). 시뮬 스트림 한 번(0~5) — 지금과 같은 자리, 같은 수. 문맥의 백성은 주제 성계의 주인(없으면 없음 — 백성에 거는 효과는 `Apply`가 이미 하지 않는다), 은하 행은 주제 성계의 것.
- **거둬짐(Withdrawn)**: 그릇이 주제 성계에 더 서 있지 않거나, 이동 중이거나, 행의 조건이 더 서 있지 않으면 — 한 번 뽑고 효과·줄 없이 거둔다(지금의 규칙과 같다).
- **결과의 줄**: `Log.Civ` 등급 1(`Push` — 말하기 스트림 없음), 초점 = 주인 백성(없으면 비움)과 주제 성계, `SourceRow` = 카드 행. 뒤따르는 줄(위기의 해결 줄, 새 만남의 줄)은 지금처럼 말하기 스트림으로.
- 연쇄(`NextEncounter`)는 B1의 다섯 장에 없다.

### 2.7 즉위와 복귀 (`BecomeRuler`) — 그리고 잠금

**잠금은 떠날 때 모든 백성에게 적는다**: 폐위와 양위 모두, 떠나는 틱에 모든 백성의 `ThroneLockedUntilTick = max(제 값, Tick + ThroneLockTicks)`. 저장 필드는 그대로이고, 옛 백성이 사라져도(붕괴 폐위 뒤 건넘 단계가 지운다) 잠금이 남는다. `CrownBlockOf`는 대상 백성의 값 하나만 읽는다. 그래서 떠난 뒤 600틱 동안은 어느 백성의 왕좌에도 오르지 못한다(§10 결정 7).

`BecomeRuler`가 쓰는 것, 순서대로, 추첨 없이:

1. 그 백성의 `bIsPlayerRuled = true`, 그릇의 `Role = Ruler`, `RuledCivID = CivID` (`FormerCivID`·`StatusTags`는 그대로 — 지난 일의 기록이다).
2. 생애 기록의 재위 세 배열에 한 칸씩(`RuledCivIDs` += CivID, `ReignEndReasons` += 빈 태그, `ReignTicks` += 0).
3. 그 백성의 열린 `Notable.Ruler`·`Notable.Regent` 기록을 닫는다(`LastTick = max(Tick, 1)` — 반란 만료의 왕좌 교체와 같은 꼴).
4. 두 이동 필드와 기다리는 카드, `Knowing.PendingContactCivID`를 비운다.
5. 그 백성의 소유 성계(정착 중 포함)를 소유 차례로 `NoteKnown`한다 — 줄도 추첨도 없다(역할 전환은 제 호출에서 앎을 적는다, `GalaxySimulationSubsystem.h:461-462`; 그 백성은 조건으로 이미 만났다). 보는 눈은 즉위한 때부터 통치자의 것이다.
6. 줄 하나(말하기 스트림): 즉위는 `log_crowned`(`Log.Calling`, 등급 2, 초점 = 그 백성 — 변형에 공백의 길도), 복귀는 `log_returned`(같은 꼴).
7. **`PeakTier`를 올리지 않는다**(#67 ①의 기본값 — 칸은 다스리는 동안의 업적으로만 오른다. 셋째 칸의 백성에 즉위해도 임시 끝이 곧바로 오지 않는다).
8. '수호신' 특성은 넣지 않는다 — B1의 그릇은 늘 T1이라 일어나지 않고, 그 이름은 불변 규칙 11과 다시 맞춰야 한다.

**즉위한 첫 틱의 청원**: 즉위는 틱 사이의 입력이라, 다음 틱의 첫머리가 새 백성의 서 있는 청원을 이미 '서 있던 것'으로 잡아 그 조건이 서 있는 동안 오지 않는다 — 지도자 공백의 길에서는 안정도 청원 둘(`petition_walls`·`petition_bread`)과 이미 선 위기의 청원, 반란이 50틱 안에 끝나면 왕좌 카드도 막힌다. 그래서 **재위의 첫 틱(`Lives.Last().ReignTicks.Last() == 0` — 여덟째 단계가 세기 전)에는 통치자의 틱 첫머리 단위를 비운 채 견준다**: 즉위하는 순간 이미 선 청원도 그 틱에 막 선 것으로 본다(저장 필드 없음, 앎에만 쓴다). 첫 생애의 틱 1도 같은 판별에 걸리지만 틱 0에는 어떤 청원의 조건도 서지 않으므로 기준값 넷이 그대로임을 러너가 보인다. 왕좌 카드의 둘째 선택지(판정 없이 반란을 끝낸다)가 있으므로 반란 50틱 안의 즉위도 함정이 아니다.

화면의 '내 위기' 기억은 역할이 바뀌면 `OwnHazardsSeen.Reset()`으로 '아직 본 적 없음'에 되돌린다(새 은하와 같은 꼴) — 빈 목록으로 두면(방랑하는 동안 이미 그렇다) 새 백성의 위기가 모두 새것이 되어 즉위 다음 틱에 정지와 카메라 이동이 겹친다.

### 2.8 양위 — 스스로 떠나기 (열째 단계의 파이프라인)

- **두 입구**: 명령 `Abdicate()`(패널 단추 — `PendingSourceRow = log_throne_vacated`)와 왕좌 카드의 셋째 선택지(효과 `DeposeRuler`, 태그 `Reign.Abdicated` — 판정·비용 없음, `PendingSourceRow = petition_keep_throne`). 둘 다 역할 효과를 **기록만** 하고 다음 틱의 열째 단계가 처리한다(불변 규칙 13; `crisis_and_transcendents.md:197`의 '틱 밖'은 '기록은 틱 밖, 처리는 다음 틱'으로 고친다).
- `ResolvePendingRoleEffect`가 원인으로 가른다: `Reign.Abdicated`면 `AbdicateRuler()`, 그 밖은 지금의 `DeposeRuler()`.
- **`AbdicateRuler()`의 차례**(`systems.md:163` — '섭정 직접 선택'은 뒤로, 지금은 폐위처럼 뽑는다):
  1. `log_throne_vacated`(새 행, `Log.Calling`, 등급 2, 초점 = 다스리던 백성) — 말하기 스트림.
  2. 섭정 성향을 시뮬 스트림에서 한 번 뽑는다(폐위와 같은 셈).
  3. `BecomeWanderer(섭정 성향, Reign.Abdicated)` — 그 안의 인물 훅 ①: 섭정의 이름 번호를 시뮬 스트림에서 **한 번, 가드보다 먼저** 뽑고(적용당 한 번 — 이름 뱅크가 비었거나 상한에 찼어도), `NameNotable`과 같은 가드(넷째 칸 이상·이름 없는 백성·NoSelf에는 기록 없음, 같은 역할의 열린 기록 닫기, 상한과 퇴출)로 `Notable.Regent` 기록(`FirstTick = Tick`, `SourceRow = log_throne_vacated`)을 연다. 그리고 지금처럼 `Status.Abdicated`, 재위를 닫는다, 기다리는 카드를 비운다, 이웃을 안다. (B5의 `Reign.Died`가 같은 훅을 쓴다.)
  4. 옛 백성의 안정도 −`AbdicationStabilityLoss`(0 아래로 가지 않는다), 모든 백성의 잠금(§2.7).
  5. **경외는 줄이지 않는다**(폐위와 다르다 — '다윗이 예루살렘을 떠난 형', `crisis_and_transcendents.md:34`).
- 그래서 열째 단계의 양위는 시뮬 스트림을 **정확히 두 번** 뽑는다(성향·이름).
- **반란 만료가 섭정을 만나면**: 반란 만료의 왕좌 교체(`ChangeThrone`)는 열린 `Notable.Ruler`와 함께 열린 `Notable.Regent`도 닫는다 — 양위 뒤 반란이 이기면 섭정이 아니라 반란의 우두머리가 다스린다.
- **같은 틱의 반란 만료 — 먼저 적힌 원천이 이긴다**(`DECISIONS_LOG.md:185`가 넘긴 경우): 다른 원천(양위의 명령·셋째 선택지, 왕좌 카드의 판정 실패)이 역할 효과를 먼저 적은 채로 다음 틱의 위기 단계에서 그 백성의 반란이 만료하면 — 왕좌는 반란 지도자에게 가지 않고(지금 코드), **열린 Rebel 기록을 닫으며**(`LastTick = max(Tick, 1)`), 만료 줄은 위기 행의 `ExpireTemplates` 대신 왕좌를 말하지 않는 새 로그 행 `log_rebellion_unseated`(가칭)를 쓴다. 그 뒤 열째 단계가 적힌 효과를 처리한다. 반란 잔여 1틱의 양위가 폐위의 경외 감쇠를 피하는 것은 받아들인다.
- **섭정의 업적은 B1에 넣지 않는다**(§10 결정 11): 섭정 아래의 옛 백성도 다른 AI 백성처럼 업적을 한다 — 다이슨 스웜을 마칠 수도 있다(`systems.md:163`과 다르다). 업적 행에 '통치자 전용' 표지(열 하나)가 생길 때 함께 정하고, `OPEN_QUESTIONS`에 그 행을 연다(새 번호).
- 통치자 정책(커맨드렛·해시 판)은 **떠나는 선택지를 고르지 않는다**: `ChoiceOf`의 돌아가며 고르기는 `OnSuccess`에 `DeposeRuler`를 지닌 선택지를 뺀 목록 위에서 돈다 — 지금의 행에는 그런 선택지가 없어 고르는 값이 비트 그대로다(왕좌 카드는 기준 판들에 오지도 않았다 — §1.7).

### 2.9 연대기

- `Admits`에 한 줄: `SourceRow`의 이름이 `enc_`로 시작하는 등급 1 줄을 받는다(대소문자를 가리지 않고 견준다 — `StartsWith(TEXT("enc_"), ESearchCase::IgnoreCase)`; 쿠킹 빌드의 `FName::ToString`은 대소문자를 지키지 않을 수 있다 — `docs/45`). 즉위·복귀·양위 줄은 `Log.Calling` 등급 2라 이미 받는다.
- 연대기 쪽의 생애 사실 줄은 다스린 백성을 차례대로 잇고('A → B'), 생애가 방랑자로 끝났으면 끝에 '방랑'(새 키 `Chronicle.Wandering`)을 붙인다.
- #118(즉위 전에 그 백성에 대해 빠진 줄)의 기본값 '빠진 채'를 지키고, 생애 가운데 즉위하는 테스트로 장을 확인한다.
- 이 바꿈은 `enc_` 줄이 있는 판의 연대기(전체 해시)만 바꾼다.

### 2.10 결정론과 저장 — 한눈에

| 입력·단계 | 시뮬 스트림 | 말하기 스트림 | 파생 정수 | 앎에 쓰기 | 저장 |
|---|---|---|---|---|---|
| `TravelTo` | — | — | — | — | 10분·종료 |
| 여덟째 단계의 도착 | — | — | — | — | — |
| 아홉째 단계의 방랑자 카드 | — | 새로 만날 때 만남 줄 1 | `Encounter` 11 (도착 카드의 몫과 고르기) | 기다리는 카드·`NoteMet` | — |
| 카드의 답 | 1 (0~5) | 결과 줄은 없음(`Push`); 위기를 끝내면 그 해결 줄, 새로 만나면 만남 줄 — 줄마다 1 | — | 카드를 비움 | 곧바로(`Answer`) |
| `ScanCurrentSystem` | — | `log_scan_wild` 1(줄이 설 때) + 새로 만난 백성마다 만남 줄 1 | — | `NoteKnown`·`NoteMet` | 10분·종료 |
| `BecomeRuler` | — | `log_crowned`·`log_returned` 1 | — | 카드·`PendingContactCivID` 비움, 소유 성계 `NoteKnown` | 다음 경계의 `Role` |
| `Abdicate` (기록) | — | — | — | — | — (다음 틱에 처리) |
| 열째 단계의 양위 | 2 (성향·이름) | `log_throne_vacated` 1 | — | 카드 비움·이웃 | 다음 경계의 `Role` |
| 위기 단계의 반란 만료(다른 원천이 먼저) | 지금과 같다 | `log_rebellion_unseated` | — | — | — |

- **저장 필드는 더하지 않는다** — 이동 두 필드·카드 칸·역할 효과 칸·`ThroneLockedUntilTick`·`ReverenceToPlayer`·`LastChoiceRows`·`ReignTicks`가 모두 이미 있다. `SaveVersion` 1 그대로. '도착했다'와 '아무것도 새로 알지 못했다'는 지역 값·반환값이다.
- 덧붙이는 열거자: `ECosmicEventSubject::VesselSystem`(데이터가 이름으로 쓴다 — 끝에, 스키마의 `enums`에도), `CosmicSeed::Purpose::Encounter = 11`(끝에), 거절 사유 둘(저장되지 않는다).
- 덧붙이는 태그는 없다(`Reign.Abdicated`·`Status.Abdicated`·`Notable.Regent`·`Log.Discovery`·`Log.Calling`이 이미 있다).
- 해시: 입력 없는 판·통치자 정책 판에는 방랑자가 없으므로 두 해시가 그대로다(즉위 첫 틱의 판별은 첫 생애의 틱 1에만 걸리고 그때 서는 청원이 없다 — 러너가 보인다). 카드가 세계를 바꾸지 않는다는 것은 `Sim.EncounterPresent`의 '`enc_` 행을 비워도 세계 해시가 같다'가 지킨다.
- **문서 먼저**: `docs/20`의 §7.4 틱 순서(여덟째 단계의 도착 검사, 아홉째 단계의 방랑자 카드, 재위 첫 틱의 청원), 추첨 지점 표(`:191`·`:195`·`:197` — 카드를 내놓는 것은 파생 정수, 답만 시뮬 스트림, 양위 두 번), 파생 정수 용도 표(`:220` — `Encounter` 11), 연대기의 거름(`:185`), 불변 규칙 2·4·5·13·14의 문장, `CLAUDE.md`의 같은 줄을 코드보다 먼저 고친다. 그리고 `docs/20` §9.2의 '앞으로 갈릴 수 있는 것'에 'B1부터 행 이름의 접두어가 판정에 든다(대소문자를 가리지 않고 견준다)'를 한 줄.

## 3. 데이터

### 3.1 조우 카드 다섯 장 (`DT_Encounters`, 행 이름은 저장에 남으므로 승인 뒤 고정 — 글은 구현 때 쓰고 감사한다)

공통: `PlayerRoleIs Wanderer`, `Subject VesselSystem`, 비용 0, 목소리 없음(`Body`만), 판정은 몸이나 정신 난이도 4(능력치 1이면 6에 3), 성공의 경외 +0.3(주인 백성에게 — 두 번의 성공이면 문턱 0.6; 값은 데이터라 §5의 측정으로 맞춘다), 실패는 효과 없이 줄 하나(벌이 아니라 그렇게 되었다는 사실). 모든 칸은 공용 칸이다 — 신앙 어휘가 없다(`DECISIONS_LOG.md:76`).

| # | 행 (가칭 이름) | 언제 | 선택지 → 효과 | 출처 |
|---|---|---|---|---|
| 1 | `enc_rebel_parley` (두 편 사이) | Standing — `PlayerInSystemWithHazard Hazard.Rebellion` | ① 두 편 사이를 오간다(정신 4) → 성공: `ResolveHazard Hazard.Rebellion`·경외 +0.3 / 실패: — ② 물러난다 → — | `crisis_and_transcendents.md:26`·`:34`(+0.2·사슬 — 여기서는 사슬 없이 +0.3) |
| 2 | `enc_plague_tend` (앓는 이들 곁) | Standing — `Hazard.Plague` | ① 앓는 이들 곁에 머문다(몸 4) → 성공: 안정도 +0.05·경외 +0.3 / 실패: — ② 지나간다 → — | 역병의 방랑자 응답은 '유물 사용'(`:22`) — 유물이 오기 전의 자리. 성공 줄은 앓는 이가 그로 인해 나았다고 쓰지 않는다 |
| 3 | `enc_awakening_talk` (깨어난 것과의 말) | Standing — `Hazard.AIAwakening` | ① 깨어난 것에게 말을 건다(정신 4) → 성공: `ResolveHazard Hazard.AIAwakening`·경외 +0.3 / 실패: — ② 물러난다 → — | '조우 대화'(`:23`) |
| 4 | `enc_star_watch` (흔들리는 별 곁에서) | Standing — `Hazard.PreSupernova` | ① 별의 떨림을 지켜보며 알린다(정신 4) → 성공: 안정도 +0.05·경외 +0.3 / 실패: — ② 떠난다 → — | '현장 개입 판정' — 별을 고치지는 않는다(안정화는 통치자의 대응 칸) |
| 5 | `enc_lane_weave` (휘는 별빛) | Lottery — 도착 카드 | ① 중력 닻을 버리고 작은 배로 빠져나간다(몸 4) → 성공: 몸 +1 / 실패: 줄만 ② 엔진을 높인다(판정 없음) → 효과 없음, 줄은 수칙대로 엔진이 듣지 않았음을 말한다('거미줄이 더 당겨졌다 — 한참 뒤에야 풀려났다' 꼴 — 엔진이 빠져나오게 했다고 쓰지 않는다) | 대도감의 '암흑을 직조하는 자들'과 수칙 4(`docs/90_LORE/cosmic_species_codex.md:336`, `theology_and_bestiary_notes.md:34`·`:106`) — 개체·둥지·상태 없음. 본문 첫 문장은 도착 앞의 일('{System}에 닿기 전 마지막 어둠에서 별빛이 휘었다' 꼴), 둘째 문장은 '…라고 기록은 적는다' 꼴의 성간 기록원 인용. 글의 슬롯은 `{System}`만(무주 성계에서도 온다) |

- 주장(`Hazard.Claim`)의 방랑자 카드 `enc_witness`는 T2 이상이라 B1에 없다. `FHazardDef.WandererResolutions`는 B1에서도 예약 그대로다 — 카드는 행의 조건(`PlayerInSystemWithHazard`)으로 오고 `ResolveHazard` 효과로 위기를 끝낸다.
- 1~4는 위기를 함께 넘은 백성의 경외를 올리는 B1의 유일한 원천이다(§1.4). 1과 3의 성공은 위기를 끝낸다 — 위기 자신의 해결 줄이 따로 말하므로 카드의 성공 줄은 한 일만 말한다(`DECISIONS_LOG.md:75`).
- 카드 5의 몸 +1은 카드 2의 판정을 쉽게 한다(몸 4면 늘 통과) — 의도한다(여행이 몸을 단련한다). 몸은 10에서 멈춘다. §5의 30판 표에 판 끝의 몸·정신 값을 함께 적는다.

### 3.2 왕좌 카드의 셋째 선택지 (`petition_keep_throne`)

- 셋째로 **끝에** 덧붙인다(로컬라이제이션 키는 위치): 화면의 글은 '왕좌를 내려놓고 떠난다'(가칭 — '명예로운 양위'는 설계 이름으로만 쓰고 화면에 내지 않는다: 선택에 옳고 그름의 이름을 붙이지 않는다, `docs/70:113`). 판정 없음, 비용 없음, `OnSuccess` = `DeposeRuler` 태그 `Reign.Abdicated`.
- 반란은 서 있는 채로 남는다 — 만료하면 반란의 우두머리가 다스리고 섭정 기록은 닫힌다(§2.8).

### 3.3 로그 행 다섯 (`DT_LogTemplates`)

| 행 | 태그·등급 | 언제 | 메모 |
|---|---|---|---|
| `log_scan_wild` | `Log.Discovery`, 1 | 무주 성계의 스캔이 새로 안 것이 있을 때 | 변형 셋, 하나는 '너무 깨끗한 광맥'의 서술자 줄(`systems.md:131` — 기록원의 인용은 줄에 넣지 않는다) |
| `log_crowned` | `Log.Calling`, 2 | 즉위 | 백성이 다스림을 맡긴다(받은 꼴 — '보좌'·기름·절·'기다리던 이' 없음). 변형에 공백의 길('다스릴 이를 잃은 {Civ}가 …' 꼴)도 |
| `log_returned` | `Log.Calling`, 2 | 복귀 | 옛 백성에게 돌아온다(받은 꼴) |
| `log_throne_vacated` | `Log.Calling`, 2 | 양위 | 행 이름은 `docs/21:1457`과 `FNotableRecord.SourceRow` 주석이 이미 쓴다 |
| `log_rebellion_unseated` | 반란 행의 태그·등급 | 다른 원천이 먼저 적은 채 반란이 만료할 때 | 왕좌를 말하지 않는다(§2.8) |

코드가 이름으로 부르는 행(로그 다섯)은 `Data/CosmicRowNames.h` 상수와 스키마의 `required_rows`로. 카드 행은 조건으로 오므로 이름으로 부르지 않는다.

### 3.4 스트링 테이블 (`ST_CA.csv` — 끝에)

명령 단추(이리로 간다·스캔·왕좌를 맡는다·돌아간다·왕좌를 내려놓는다)와 거절 사유(잠금은 하나 — '떠난 지 얼마 되지 않았다 · N주기', 결정 7로 걸리는 잠금은 늘 떠난 때의 것이다), 상단 바의 방랑 줄과 '갈 레인이 없다', 패널의 경외 줄, 방랑자 카드의 문구 둘(거둬졌을 때의 한 줄 — 청원의 낱말 '청'과 성스러운 질서의 낱말 '때'를 쓰지 않는다, 예: '이 일은 이미 지나갔다.'; 자동 정지의 까닭 '정지 — 카드가 왔다' — 기다리는 카드의 행 접두어로 키를 고른다), '첫 함선' 안내(`Hint.FirstShip`), '새로 보이는 것이 없었다', `Chronicle.Wandering`. 접두어 `Wander.`을 스키마의 검사 목록(`checked_prefixes`)에 더한다(`Petition.`도 함께).

### 3.5 스키마 (`Tools/data/schema.json`·`Tools/cosmic_data_schema.py`·`Tools/test_data_schema.py` — 새 규칙마다 깨지는 경우 하나)

- **닫힌 시험 목록에 셋을 더한다**(새 규칙 종류라 `docs/40:236`의 §12.3 세 물음을 함께 적는다): ① `row_prefix` — `when`에만, 행 이름이 그 접두어로 시작한다 ② `has_cond {type, key}` — 조건 목록에 그 종류와 그 Key를 가진 것이 있다(`has_type`은 종류만 본다) ③ `each_choice {when, then}` — 선택지마다 따로 거는 규칙.
- 규칙: `when row_prefix enc_` ⇒ `has_cond PlayerRoleIs Wanderer`·`Subject` = `VesselSystem`·`Choices.Cost` 0·`Choices.CheckStat` ∈ {Body, Mind, Causality, Exaltation, NoCheck}. `when row_prefix petition_` ⇒ `has_cond PlayerRoleIs Ruler`·`Subject` ∈ {Civilization, SettledSystem}. `Subject VesselSystem ∧ Trigger Lottery` ⇒ 글의 슬롯은 `{System}`만. `each_choice`: `OnSuccess`에 `DeposeRuler`가 있으면 `CheckStat` NoCheck이고 그 태그는 `Reign.Abdicated`. `DT_Events`: `Subject` ∈ {Civilization, SettledSystem}.
- `schema.enums.ECosmicEventSubject` 끝에 `VesselSystem`(`Data.Integrity`가 C++ 열거자와 견준다). 노브 여섯은 `rules` 표에(`LaneTravelTicks` min 1·정수).
- **구현 기록 (B-1b, 2026-10-25)**: 닫힌 시험 목록에는 ①·②만 더했다. 선택지마다의 두 규칙(`enc_`의 비용 0, '`OnSuccess`에 `DeposeRuler`면 `NoCheck`이고 `Reign.Abdicated`')은 `Data.Integrity`의 `ChoiceProblems`(C++)가 지킨다 — MVP ②의 선택지별 규칙(실패 줄은 판정이 있을 때만)과 같은 자리이고, 스키마의 규칙은 목록 단위로만 선다. 그래서 둘은 에디터 없는 검사(`check_data.py`)에는 보이지 않고 자동화 테스트에서 잡힌다(구현 리뷰가 찾은 차이, `DECISIONS_LOG`에 적는다).

## 4. 표현 계층

- **패널**(`UI/CosmicSystemPanelWidget`): 방랑자 단추 줄을 프로젝트 단추 목록(`ActionButtons`) 밖에 따로 두고 제 처리기로 보낸다(시험용 `GetWandererButtons`·`GetAbdicateButton` — 프로젝트 단추의 수·차례를 단언하는 기존 테스트가 그대로 참이다). 보이는 성계가 그릇이 선 성계면 [스캔], 그 성계가 백성의 정착된 성계면 [왕좌를 맡는다](옛 백성이면 [돌아간다]; 회색이면 `CrownBlockOf`의 사유 — 경외의 값과 문턱은 툴팁에만, 잠금은 남은 주기), 그릇의 레인 이웃이면 [이리로 간다 · 20주기](회색이면 `TravelBlockOf`의 사유). 문에 넘기는 앎의 답(만남·기다리는 카드)은 `FCosmicKnownView`에서. 만난 백성(`IsShown`)의 성계 패널에는 방랑자일 때 주인 줄 아래 '이 백성이 당신을 우러르는 정도 NN%'. 통치자는 수도의 패널에 [왕좌를 내려놓는다] — 3초 안 두 번 누름(되돌릴 수 없다), 툴팁에 무엇이 일어나는지와 '다시 왕좌에 앉기 전까지 칸이 오르지 않는다'.
- **지도**(`World/CosmicMapLook`): 그릇의 표시 — 방랑자일 때 선 성계에 작은 표지, 이동 중이면 목표로 가는 레인을 밝힌다. 그릴 값은 순수 함수(`VesselLookOf`)이고 앎을 쓰지 않는다(그릇이 선 성계와 그 이웃은 늘 아는 성계다).
- **상단 바**: 방랑자의 줄 '방랑 — {성계}' / '{성계}로 가는 중 · N주기', 툴팁에 몸·정신·인과·높아짐과 경외가 가장 높은 백성 하나, 갇혔으면 '갈 레인이 없다'.
- **카드 위젯**: 방랑자 카드는 언제나 낼 수 있고(비용 0), '6에 n'은 그릇의 능력치로, 목소리 없음. 주제 성계와 주인 백성은 `VesselSubjectOf`로 읽는다(답과 같은 함수).
- **자동 정지**: 방랑자 카드도 '카드' 까닭으로 멈춘다(문구는 §3.4). 역할이 바뀌면 `OwnHazardsSeen.Reset()`(§2.7).
- **'첫 함선' 안내**: A6의 첫 안내와 같은 꼴(화면의 안내 — 로그가 아니라 해시 밖). 수도 패널의 회색 한 줄 — 통치자이고 틱 510(×1의 8:30, `docs/00_VISION.md:174`) 이상 1,110 미만이며 판이 끝나지 않았을 때 수도 패널이 처음 보이면 선다(세션마다 한 번 — 안내 진행에 `bShipHinted` 하나, 저장 없음). 글 '첫 함선이 준비되었다. 왕좌를 내려놓고 떠날 수 있다.'(감사에서).
- **연대기 화면**: 생애 사실 줄의 'A → B · 방랑'(§2.9).
- **컨트롤러**: 명령 넷을 컨트롤러가 받아(끝난 판은 거절) 서브시스템에 넘긴다. 개발용 콘솔 명령 `CosmicTravel <성계>`·`CosmicScan`·`CosmicCrown`·`CosmicAbdicate`(같은 길 — 화면 찍기와 손 확인용).

## 5. 커맨드렛과 기준값

- **방랑자 정책** `World/CosmicWandererPolicy`(커맨드렛과 해시 판이 함께 쓴다 — MVP ⑨의 통치자 정책과 같은 꼴, 플레이어의 스크립트라 세계와 앎을 읽어도 된다): ① 기다리는 카드가 있으면 돌아가며 고른다(제 셈 `WandererAnswers`부터 — 비용 없음, 통치자 셈에 더하지 않는다) ② 그릇이 선 성계의 주인에게 `CrownBlockOf`가 None이면 즉위·복귀한다 ③ 도착마다 한 번 스캔한다 ④ 이동 중이 아니면 레인 이웃 가운데 위기가 선 첫 이웃(레인 배열 차례 — 방금 떠나온 성계는 빼고, 이웃이 그것 하나뿐이면 간다)으로, 없으면 (여행 수 mod 이웃 수)번째 이웃으로 간다 ⑤ 임시 끝을 읽은 뒤로는 입력하지 않는다(`bEnded`).
- **한 틱 경계의 차례**: `RunHeadless(1)` → 끝 읽기 → (그 틱이 `-Abdicate`면) `Abdicate()` → 통치자 정책(통치자일 때만 효과가 있다) → 방랑자 정책(방랑자일 때만: 답 → 즉위 → 스캔 → 이동). 즉위한 경계에서 통치자 정책은 다음 경계부터 움직인다.
- 커맨드렛 인자 `-Abdicate=<틱>`·`-Wander=1`. 요약에 새 줄 `CosmicBalance wanderer: left at T, crowned at T (CivID, tier, path reverence|vacuum|return), journeys N, scans N, cards N, answers N, reverence peak X, body B, mind M` — 통치자 줄은 고치지 않는다. 통치자 줄의 고침 셋: `CardsCome`은 통치자일 때만 센다, `vessel deposed at`은 `ReignEndReasons`의 끝이 `Reign.Deposed`일 때만(양위는 wanderer 줄의 `left at`), T2/T3 틱은 첫 재위의 백성만 본다 — 모두 기준 판들에서 값이 그대로다.
- **기준값**: `Tools/baselines/seed42.json`에 셋째 줄 `wanderer_summary_line` — 시드 42, 3,000틱, `-Answer=1 -Build=1 -Focus=1 -TellAll=0 -Abdicate=1200 -Wander=1`, 키는 해시 줄과 `CosmicBalance ruler:`·`CosmicBalance wanderer:` 두 줄을 ' | '로 잇는다. `run_checks.ps1`의 `Compare-Balance`에 `-Wanderer`를 더하고 `-UpdateBaseline`이 그 줄을 쓴다.
  - **구현 기록 (B-1d, 2026-10-25)**: 틱 수는 3,600이다 — 시드 42는 1,201에 떠나 3,001에 옛 백성에게 돌아오므로(복귀) 3,000틱 줄에는 즉위가 들지 않는다. 비교할 줄이 기준값 파일에 없으면 러너가 실패한다(전에는 그냥 지나갔다).
- **20,000틱 둘을 러너가 본다**: `seed42.json`에 `summary_line_20000`·`ruler_summary_line_20000`(정책 판 3928640348 / 2194209278을 여기에 처음 적는다)을 두고, `run_checks.ps1 -Tier full`에 두 균형 단계를 더한다 — B1의 긴 판에만 닿는 바꿈(연대기의 `enc_` 줄, 파괴의 이동 비우기, 열째 단계의 가르기)을 커밋마다 지킨다. 해시 판의 방랑자 판(`-CosmicHashWander`)과 패키지의 ⓓ는 마일스톤 B의 끝에서(그때가 ⓓ를 되풀이하는 때다 — `docs/20` §9.2).
- **닿을 기회를 먼저 잰다**(`[B-1d]`의 첫 일): 시드 1~30, 입력 없는 판, 틱 1,200~3,000에서 (a) T1~T2 백성의 위기 사례 수(종류별) (b) AI가 답할 때 각 위기가 서 있는 틱 (c) 안정도 < 0.2인 (백성, 틱) 표본 — 그 값으로 아래 목표를 확정한다.
- **커맨드렛으로 재는 목표**(잠정 — 위의 측정 뒤 확정): 30판(시드 1~30, `-Abdicate=1200 -Wander=1`, 6,000틱): 즉위·복귀 ≥ 24판, 떠난 뒤 즉위까지의 중앙값 ≤ 1,800틱(×10의 3분), 길(경외·공백·복귀)마다 센다. 모자라면 성공의 경외 값과 판정 난이도를 먼저 맞춘다(문턱 0.6·0.2와 잠금 600은 설계 값이라 마지막에). 표에 같은 백성에게서 받은 경외의 횟수 분포와 판 끝의 몸·정신을 함께 적는다.
  - **측정 기록 (B-1d, 2026-10-25)**: 닿을 기회(시드 1~30, 입력 없는 3,000틱, 틱 1,200~3,000) — T1~T2 백성에게 위기가 선 100틱 표본 638(판마다 약 21), 안정도 0.2 미만의 (백성, 표본) 783(30판 모두), 구간의 위기 줄 AI 각성 150·반란 109·역병 86·초신성 전조 62(반란 506건은 AI가 대응으로 끝낸다). 30판 — 판정 난이도 4: 즉위·복귀 23판, 중앙값 1,500틱(복귀 13·공백 9·경외 1); 서 있는 카드 넷의 난이도를 3으로(능력치 1이면 6에 4 → 6에 5): **25판, 중앙값 1,500틱**(최소 600·최대 3,700; 복귀 14·공백 9·경외 2), 판당 카드 4.4·답 4.4·이동 113, 경외 최고 평균 0.09, 판 끝의 몸 1~6·정신 1(30판 모두). 경외 길이 약한 까닭: 정책이 선택지를 돌아가며 고르니 답의 절반이 '물러난다'이고, 같은 백성에게 두 번 성공해야 0.6에 닿는다. 문턱과 잠금은 그대로 둔다.
- 기준값 넷(§1.7)은 비트 그대로여야 한다 — 3,000틱 둘은 구현의 첫 커밋부터, 20,000틱 둘은 그 단계를 더한 커밋부터 러너가 지킨다(그 전에는 손으로 한 번 돌려 견준다).

## 6. 테스트 (새 단언은 변이로 한 번씩 깨뜨린다 — 패치는 새 폴더 `Tools/mutations/B1/`에 남긴다)

| 테스트 | 단언 | 깨뜨릴 변이 |
|---|---|---|
| `Sim.SightWalkMoved` (새, 함수만 옮긴 증명) | 옮기기 전 HEAD에서 적어 둔 CRC 상수(§2.1 — 통치자 2,000틱·옮기는 방랑자 2,000틱, 시드 42·7)와 같다 | 찾아 나가는 출발점을 수도 하나로; 레인 수로 자른다 |
| `Sim.TravelTo` (새) | 사유마다 거절하고 상태·스트림·전체 해시가 그대로(이동 중도 카드도 없는 방랑자의 `TravelTo(sol)`은 `Sanctuary`); 받아들이면 `LaneTravelTicks`틱째에 도착(그 전 틱에는 아니다); `LaneTravelTicks` 0이면 다음 틱에 도착하고 두 필드가 함께 빈다; 가는 동안 목표가 파괴되면 이동이 끝나고 그릇은 남는다; 출발지가 파괴되면 옮겨지고 이동이 비워진다; 이동 중 시뮬 스트림을 뽑지 않는다; 방랑자의 레인 이웃은 모두 아는 성계다; 핸들러 안의 부름은 에러 한 줄(`AddExpectedError`)과 무동작 | 한 틱 일찍 도착; 이웃이 아닌 목표를 받는다; 파괴의 이동 비우기를 뺀다; 핸들러 가드를 뺀다 |
| `Sim.Scan` (새) | 보는 거리 + 1을 안다; 세계 해시·시뮬 스트림 그대로, 전체 해시는 바뀐다; 무주 성계에서 새로 안 것이 있을 때만 줄 하나; 통치자·이동 중·핸들러 거절; 두 번째 스캔은 0 | 보너스를 뺀다; 통치자에게 허락; 줄을 늘 말한다 |
| `Sim.EncounterPresent` (새 — `Sim.PetitionPresent`의 짝) | 서 있는 위기 곁에서 다시 오지 않는다; 도착한 틱에 새 성계의 위기 카드; 레인 하나를 오가도(그 사이 다른 답 없음) 막 답한 카드는 다시 오지 않고, 다른 카드에 답한 뒤에는 온다; 이동 중에는 없다; 격차 2의 백성 카드는 없다; 통치자 행은 방랑자에게 오지 않는다; 도착 카드의 유무가 `DeriveForName(42, 성계, 11, 틱)/2^32 < 몫`과 일치한다(정해진 성계·틱 몇에서 참·거짓 둘 다 — 상수); 몫 0이면 없고 1이면 늘 온다; 같은 무게의 도착 카드 두 행(테스트가 넣은 행)이 시드를 바꿔 가며 모두 온다; 무주 성계의 도착 카드와 그 답이 에러 줄 없이 몸 +1; `enc_` 행을 비워도 세계 해시가 같다; 시뮬 스트림이 그대로; 즉위 뒤 통치자의 주기 카드가 연이어 같지 않다 | `GetRng()`를 쓴다; 목적 10을 쓴다; 몫을 읽지 않는다; 다시 펴지 않고 `u`를 쓴다; 이동 중에도 낸다; 격차 검사를 뺀다; 기억을 빼지 않는다 |
| `Sim.EncounterAnswer` (새) | 받아들인 답마다 정확히 한 번 뽑는다; 판정 경계(정신 1 + 3 = 4 통과, + 2 실패); 경외는 주인 백성에게; `enc_awakening_talk` 성공이 그 성계의 위기를 끝낸다; 떠난 뒤의 답은 거둬진다; 방랑자의 `petition_` 답·통치자의 `enc_` 답은 거절(0번) | 비용 검사 앞에서 뽑는다; 다스리던 백성에게 경외; 거둬짐에서 효과를 낸다 |
| `Sim.Coronation` (새) | 사유마다 경계(경외 0.6 받음·0.599 거절, 안정도 0.199 받음·0.2 거절, 잠금의 틱과 그 전 틱); 붕괴로 폐위된 뒤 옛 백성이 지워진 다음 틱에도 다른 백성의 즉위가 600틱 동안 잠금; 옛 백성은 경외 0에도 잠금이 풀리면 복귀(그 전 틱은 거절); 만나지 않은 백성 거절; 재위 세 배열 길이가 같다; 열린 Ruler·Regent 기록이 닫힌다; 즉위 직후(틱 전) 그 백성의 소유 성계를 모두 안다; 추첨 없음; `PeakTier` 그대로; 안정도 0.15로 즉위 → 다음 틱에 `petition_bread`가 온다; 다음 경계에 `Role` 저장 까닭; 핸들러 거절 | `>`와 `>=`; 모든 백성의 잠금을 뺀다; 배열 하나를 빠뜨린다; `PeakTier`를 올린다; 재위 첫 틱 판별을 뺀다 |
| `Sim.Abdication` (새) | 두 입구 모두 다음 틱 열째 단계에서 같은 파이프라인을 탄다 — 열째 단계 안의 시뮬 추첨 2, `Notable.Regent` 1개(`SourceRow log_throne_vacated`), 경외 그대로, 안정도 −0.1, 모든 백성의 잠금 `Tick+600`, `Reign.Abdicated`·`Status.Abdicated`(카드 입구는 그 앞에 답의 추첨 1과 `LastChoiceRows`가 더해지고 곧바로 저장된다); 이름 뱅크가 빈 백성·상한에 찬 백성·넷째 칸 백성에서도 시뮬 스트림이 정확히 두 번; 정책이 셋째 선택지를 고르지 않는다; 반란 잔여 1틱에 `Abdicate` → 다음 틱: Rebel 기록 닫힘·왕좌는 반란에게 가지 않음·`log_rebellion_unseated`·Regent 하나; 셋째 선택지 뒤 반란이 만료하면 Regent가 닫히고 열린 기록은 Ruler 하나 | 경외를 줄인다; 섭정 기록을 뺀다; 정책 목록에서 빼지 않는다; Regent 닫기를 뺀다 |
| `Log.ChronicleWanderer` (새) | `enc_` 답 줄과 즉위·복귀·양위 줄이 연대기에 든다(접두어는 대소문자를 가리지 않는다); 생애 가운데 즉위한 장(#118); 사실 줄 'A → B · 방랑' | `enc_` 줄을 거른다 |
| `Sim.SaveTravel` (새 — `Sim.SavePetition`의 짝) | 이동 중·방랑자 카드 기다리는 중·양위가 기록된 채로 저장 → 로드 → 같은 입력: 저장 없는 판과 같은 전체 해시; `Sim.SaveRoundTrip`의 채움에 이동 두 필드·능력치·경외를 더한다 | 로드 뒤 `TravelTicksRemaining`을 0으로 |
| `Sim.WandererLoop` (새, `[B-1d]` — 정책과 같은 커밋) | 시드 42, `-Abdicate=1200`과 같은 정책을 테스트 안에서: 6,000틱 안에 즉위·복귀; 같은 입력이면 같은 두 해시(두 번); 에러 줄 없음 | 즉위의 문턱을 0.99로(복귀도 막는다) |
| `UI.WandererPanel` (새) | 세 단추가 맞는 성계에만, 회색의 사유 말; 경외 줄; 통치자의 [왕좌를 내려놓는다] 두 번 누름과 툴팁; 지도의 그릇 표지; 패널의 사유와 명령의 사유가 (`CosmicSeeAll` 없이) 같다 | [이리로 간다]를 이웃이 아닌 성계에도 보인다 |
| `UI.WandererCard` (새) | 방랑자 카드의 선택지가 낼 수 있고 '6에 n'이 그릇의 능력치; 서 있는 위기가 있는 백성에 즉위한 다음 틱에 '내 위기' 정지가 없다 | `Reset()` 대신 빈 목록 |
| `UI.FirstHints` (고침) | '첫 함선' 줄 — 틱 510 이상에서 수도 패널에 한 번; 이어 연 판이 틱 2,000이면 없다; 한 번 떠난 판에는 없다 | — |
| 고칠 테스트 | `Sim.SightStage`(레인 이웃으로), `Sim.PetitionAnswer`(방랑자의 `petition_` 답은 여전히 거절), `Sim.PetitionPresent`, `UI.AutoPause`, `UI.TopBarAndPanel`(방랑자의 패널에 프로젝트 단추가 없다는 뜻 그대로, 방랑자 단추 줄을 단언), `UI.Chronicle`, `Data.Integrity`(스키마의 새 규칙과 열거자), `Sim.HashRun`(정책 목록) | — |

## 7. 문서 (구현의 문서 커밋 — 끝에 재감사 워크플로)

- **코드보다 먼저**(`[B-1a]`): `docs/20`(§2.10의 '문서 먼저' 전부), `CLAUDE.md`(규칙 2·4·5·13·14의 같은 줄; 규칙 15의 '현재 코드'는 끝에 — 16 KB 안).
- **문서 커밋**: `docs/21`(새 명령·순수 함수·노브·열거자·`Purpose` 11, `:439`, `:1101`의 노브 자리, `:1457`의 섭정 기록 자리, `:1471`의 성역 문장), `docs/10_DESIGN/systems.md`(`:127` 한 레인씩·도착 카드, `:131`, `:163` 섭정 기록과 업적 — B1 밖, `:164`·`:165` 즉위와 복귀, `:171` 잠금은 떠날 때 모든 백성에게), `docs/10_DESIGN/pillars.md`(`:75-76` T1 방랑자도 레인 이동·조우), `docs/10_DESIGN/crisis_and_transcendents.md`(`:22`·`:34`·`:195`·`:197`), `docs/00_VISION.md`(`:18` 임시 끝은 생애의 `PeakTier` — 즉위로 오르지 않는다, `:24` 즉위의 두 길, `:136` 한 레인씩·도착 카드, `:174` '첫 함선'은 화면 안내), `docs/70`(방랑자 카드의 규칙과 화자, 연대기의 `enc_` 줄, `:43`), `docs/15`(즉위·복귀·양위·스캔·이동·기다리는 카드, '도약'의 뜻 가르기), `docs/40`(테스트 표·커맨드렛 인자·러너의 새 단계·`Tools/mutations/`, `:236`의 새 규칙 종류), `docs/45`(새로 확인한 엔진 사실이 있으면), `docs/50`(B1 끝·`:144`의 낡은 문장), `A0`(`:39`), `A2`(`:61` 답 함수), `Config/DefaultGameplayTags.ini:93`(섭정 기록의 자리), `OPEN_QUESTIONS`(#35 둘째 절반·#53 ② 이동 중·#67 ①을 닫는다; #81·#83·#87·#118을 고친다; #60 ②에 'T1 방랑자의 압력 항은 0 — 방랑자 칸이 오르는 블록에서'; #107에 방랑자 카드의 격차; #80은 그대로(단계 5); 섭정의 업적 규칙을 새 행으로), `DECISIONS_LOG`, `CHANGELOG`, `STATUS`.

## 8. 순서·검증·effort

데이터와 타입은 함께 들어와야 한다(스키마가 열거자를 C++와 견주고, 가져오기가 열거자를 요구한다). 커밋마다 러너 full 2회가 통과한다.

| 커밋 | 하는 일 | effort | 검증 |
|---|---|---|---|
| `[B-1a]` | 규칙 문서 먼저(`docs/20`·`CLAUDE.md`) + 동작을 바꾸지 않고 `SightWalkOf`만 옮기는 커밋(옮기기 전 HEAD에서 CRC 상수를 적고 `Sim.SightWalkMoved`) + 러너의 20,000틱 두 단계와 `seed42.json`의 두 키 | xhigh | 러너 full 두 번, 기준값 넷 그대로 |
| `[B-1b]` | 시뮬의 동작과 그 데이터를 한 커밋으로: 노브 여섯(멤버·`DT_Rules` 값·스키마)·열거자·`Purpose`·로그 다섯과 `required_rows`·상수·카드 다섯·셋째 선택지·스트링·스키마의 새 시험과 그 테스트·`CosmicWanderer`·명령 넷·여덟째 단계·파괴·방랑자 카드·답·양위와 섭정 기록·반란 만료의 두 규칙·잠금·즉위와 복귀·재위 첫 틱의 청원·연대기·통치자 정책의 목록과 셈 → Sim 테스트(카드에 기대는 단언은 테스트가 넣은 행으로) → **ultracode 글 감사**(A2의 다섯 관점: 서사 규칙·목소리·한국어와 틀 문법·글과 효과·신학) → 고친 글 | xhigh | 러너 full 두 번, 기준값 넷 그대로, 변이; **ultracode 구현 리뷰**(결정론·저장·틱 순서) |
| `[B-1c]` | 표현: 패널 단추 줄과 경외 줄·지도 표지·상단 바·카드 위젯·안내·연대기 사실 줄·컨트롤러·콘솔 | high | UI 테스트, 창 없이 찍은 화면(그릇 표지·세 단추·카드) |
| `[B-1d]` | 닿을 기회 측정 → 방랑자 정책(`World/CosmicWandererPolicy`)·커맨드렛·새 요약 줄·기준값 셋째 줄·러너 단계·`Sim.WandererLoop` → 30판 측정과 카드 값 맞추기 | high | 30판 표(§5 목표), 기준 셋째 줄 |
| `[B-1e]` | 문서(§7) | xhigh | **재감사 워크플로** |

## 9. 손 확인 (B1의 종료 조건 — 개발자, 콘솔 없이)

`Sim.WandererLoop`이 즉위하는 시드로 새 판 → (8:30 무렵 수도 패널의 '첫 함선' 줄) → 수도 패널의 [왕좌를 내려놓는다] → 지도의 그릇 표지 → 레인 이웃으로 두 번 이상 옮긴다 → [스캔] → 위기가 선 성계에서 카드에 답한다 → 경외나 지도자 공백으로 [왕좌를 맡는다](또는 잠금 뒤 옛 백성에게 [돌아간다]) — 모두 틱 6,000 전에, ×10에서. 즉위 뒤 상단 바와 패널이 통치자의 것으로 돌아오고 정지가 겹치지 않는지, 연대기에 양위·방랑·즉위가 있는지.

## 10. 개발자 결정 (권장안 — 1·4·6·7·9·12를 먼저 본다)

1. **이동은 명령 한 번에 레인 하나**(권장) — 저장 필드 없이, 여덟째 단계가 앎을 읽지 않는다. 대안: 먼 목표와 경로(필드 하나와 길 찾기 규칙이 는다). 나중에 덧붙여도 지금의 뜻이 그대로다.
2. **레인 하나 20틱**(`LaneTravelTicks`) — 거리에 따른 시간은 B8.
3. **스캔**: 보는 거리 +1 레인, 추첨·쿨다운 없음, 새로 안 것이 있을 때만 줄(무주 성계는 `log_scan_wild`).
4. **카드 다섯**(§3.1): 반란·역병·AI 각성·초신성 전조 곁의 넷과 도착 카드 하나. 비용 0, 판정 4, 성공 경외 +0.3(잠정 — 측정으로 맞춘다), 몸이 오르면 역병 카드가 쉬워지는 것은 의도.
5. **카드의 자리와 고르기**: 기다리는 카드 칸을 함께 쓴다, 주제 `VesselSystem`, 파생 정수 목적 11(몫의 시험과 다시 편 고르기), 도착 카드의 몫 0.25, 이동 중에는 카드가 없다, 격차 1 이하의 백성만, 가장 최근에 답한 `enc_` 행은 다시 내놓지 않는다, 통치자의 셈은 `petition_` 행만.
6. **즉위는 명령**(패널의 [왕좌를 맡는다])이지 카드가 아니다 — 조건은 `systems.md:164` 그대로 + 만난 백성 + 그 백성의 정착된 성계에 서 있다. **옛 백성에게는 복귀**([돌아간다] — 잠금만 본다, `systems.md:165`). 대안: 카드의 효과로(새 효과 열거자·역할 효과·뽑기 하나가 는다); 복귀를 B5로(그러면 양위한 플레이어가 집에 돌아갈 길이 즉위의 조건뿐이다).
7. **왕좌 잠금은 떠날 때 모든 백성에게 600틱**(필드 그대로 — 옛 백성이 사라져도 남는다). 대안: 지금처럼 옛 백성에게만(`systems.md:171`의 까닭은 그대로 맞다 — 다른 백성의 공백 길이 곧바로 열린다).
8. **즉위가 `PeakTier`를 올리지 않는다**(#67 ① 기본값).
9. **스스로 떠나기를 B1에**: 명령 [왕좌를 내려놓는다] + 왕좌 카드의 셋째 선택지('왕좌를 내려놓고 떠난다'). 섭정 기록(`BecomeWanderer`의 인물 훅 ① — 가드 그대로)·안정도 −0.1·경외 그대로·잠금 600. 정책은 고르지 않는다. 대안: 카드의 셋째 선택지만(손 확인이 반란을 기다려야 한다 — §1.5).
10. **같은 틱의 반란 만료는 먼저 적힌 원천이 이긴다**: 왕좌는 반란에게 가지 않고 Rebel 기록을 닫으며 만료 줄은 `log_rebellion_unseated`. 섭정 아래의 반란 만료는 Regent도 닫는다.
11. **섭정의 업적 규칙은 B1에 넣지 않는다** — 섭정 아래 옛 백성도 AI처럼 업적을 한다(새 열린 물음). 대안: `FAscensionFeatDef` 끝에 통치자 전용 표지를 더하고 섭정 아래에서 그 행을 판정·선택하지 않는다(T1 섭정 백성은 칸이 멈춘다 — 양위가 있는 판의 세계가 바뀐다).
12. **즉위한 첫 틱에는 이미 선 청원도 온다**(재위 첫 틱 판별 — 저장 필드 없음, 기준값 그대로). 대안: 오지 않는 것을 받아들인다(공백의 길로 오른 왕이 안정도 청원을 받지 못한다).
13. **방랑자가 된 자리에 이미 선 위기는 카드가 되지 않는다** — 떠났다 돌아오면 온다(의도, 저장 필드 없음).
14. **'첫 함선' 안내**를 수도 패널의 회색 줄로(로그가 아니다 — 기준값이 그대로).
15. **'지경'은 B8**: 성역에 레인이 생길 때. 그때까지 `TravelTo(sol)`은 사유 '성역'으로 거절(A0의 '에러 한 줄'은 `Install`·`BecomeWanderer`에만).
16. **격차 2의 경외/공포 변형과 3 이상의 현상 카드는 뒤로**.
17. **연대기**: `enc_` 답 줄을 받는다, 즉위·복귀·양위는 `Log.Calling` 등급 2, 사실 줄 'A → B · 방랑'.
18. **T1 방랑자도 레인을 옮기고 조우한다**(`pillars.md:75-76`을 고친다), **즉위의 둘째 길의 이름은 '지도자 공백(안정도 < 0.2)'**(`docs/00_VISION.md:24`를 고친다).
19. **측정과 기준값**: 커맨드렛 `-Abdicate=<틱> -Wander=1`, 기준값 셋째 줄, 러너가 20,000틱 둘도 견준다; 해시 판의 방랑자 판과 ⓓ는 마일스톤 B 끝에.
20. **B1의 종료 조건**: §9의 손 확인 + 30판 목표(즉위·복귀 ≥ 24판, 떠난 뒤 즉위까지 중앙값 ≤ 1,800틱 — 닿을 기회를 잰 뒤 확정).
21. **살아 있는 이웃이 없는 자리에 갇힌 방랑자는 받아들인다**(드물다 — 화면이 까닭을 말한다).
22. **B1 밖**(§0): 여러 레인 경로, 수련, 유물, `PersonalPool`, 방랑자 업적·`PersonalTier`, 순간 이동, '지경', 추방·재회 카드, 첫 접촉 카드, `enc_witness`, 격차 변형, '수호신', 섭정의 업적 규칙·직접 선택·표류, 다이제스트, 사망·계승.

## 추가 이야기와 보완

- '도약'은 T3의 순간 이동 권능과 공백기의 시간선 도약에 함께 쓰인다 — 권능 쪽 이름을 따로 정하자(용어집에 둘을 가르는 줄; 권능이 오는 블록에서).
- '수호신' 특성의 이름은 불변 규칙 11('신이 되었다' 금지, 받은 꼴만)과 맞지 않는다. 그 특성이 닿게 되는 때(방랑자 업적과 `PersonalTier`)에 게임 안의 이름을 다시 정하자.
- 설계 감사의 제안 D('칸 공명' — `PersonalTier`를 다스리는 백성의 칸 − 1로)는 B1/B5로 꼬리표가 붙어 있지만 채택되지 않았고, 들이면 통치자 정책 판의 기준값이 움직인다(그 판의 백성은 틱 2,396에 T3). B1에 넣지 않고 #1·#67·#96·#100과 함께 따로 정하자.
- `Tools/mutations/`가 문서에만 있었다 — B1에서 폴더를 만들고 첫 패치를 남긴다.
