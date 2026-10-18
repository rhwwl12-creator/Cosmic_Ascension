# 용어집 (옛 §15)

> 원문: guideline.md v0.8 §15 (2026-10-18 docs/ 재편으로 이관, 내용 무변경). 표지 `(v0.8 — 단계 N부터)`·`(v0.7 — …)`·`Phase N`이 붙은 문장은 **예정**(아직 코드에 없음), 표지 없는 문장은 **현행 규칙**이다. 열린 물음은 `docs/60_DECISIONS/OPEN_QUESTIONS.md`, 확정 기록은 `docs/60_DECISIONS/DECISIONS_LOG.md`. 절 번호(§N)는 원문 그대로 두었다 — 위치는 `docs/SECTION_MAP.md`.

※ 같은 낱말의 두 뜻은 이렇게 고정한다: '칸' = Tier(거대 프로젝트의 자리는 '건설/대응 슬롯'), '부르심' = 서막 끝의 소명 사건, '문턱' = T3(칸 이름) / `enc_threshold`(T5 카드), '정착률'은 '주장 만료율'로 바꿔 부른다.

## 15. 용어집

| 한국어 | 영어 | 코드 식별자 | 설명 |
|---|---|---|---|
| 존재 등급 | Ascension Tier | `EAscensionTier` | 5단계. 문명과 개인이 각각 가짐. T1~2 보이는 것들 / T3 문턱 / T4~5 보이지 않는 것들 |
| 보이는 것들 / 보이지 않는 것들 (보임) | Visible / Invisible realms (골 1:16) | `EAscensionTier` T1~2 / T3(문턱) / T4~5 (열거형 변경 없음). §3.2 표의 열 이름 '보임' | 두 계는 장소가 아니라 격 (§3.2). `Realm.*` 소속 태그와 별개 — 계 태그 = 소속(Allegiance), Tier 의미론 = 보임 |
| 사다리의 상한 / 보좌 / 무저갱 | Ladder cap / Throne / Abyss | 코드 식별자 없음. `EAscensionTier`에 T6·T0 없음, 두 영역의 태그 없음(§3.2, §12.2) | 여섯째 칸은 없다. 보좌와 무저갱은 사다리 밖의 두 판결. 서술자·UI 사용 금지, 화자 등급별 이름은 §8.2 표 (§3.2) |
| 신화적 업적 | Mythic Feat | `FAscensionFeatDef`, `DT_Feats` | 승천의 유일한 조건 |
| 승천 | Ascension | `UAscensionSubsystem::OnTierAscended` | 등급 상승 사건 |
| 두 길 / 높아짐 | Two Ways / Exaltation | `Way.Received` / `Way.Seized`, `FPlayerEntityState.Exaltation`(0~10) | Tier 5의 받는 길과 찬탈의 길 (§3.2). 경배 수용이 올리는 4번째 개인 능력치 (§5.9) |
| 위기 | Crisis / Hazard | `FHazardDef`, `Hazard.*` 태그, `FHazardDef.SourceDoctrine`(선택, 교리 귀속) | 카운트다운이 있는 성계 상태. `Hazard.Visitation`만 '놓친 기회', `Hazard.Claim`은 영혼 경제 (§4.1, §4.2) |
| 반란 / 폐위 / 왕좌 잠금 | Rebellion / Deposition / Throne lock | `Hazard.Rebellion`, `EEffectType::DeposeRuler`, `ThroneLockedUntilTick` | 통치자의 실패 상태 (§4.1, §5.10) |
| 몰락한 군주 | Fallen Monarch | `Status.FallenMonarch`, Severity 2 '몰락한 군주' | 폐위·붕괴로 모성계에 남는 방랑자. 전용 조우 3~5장 (§4.4, §5.10) |
| 초월자 | Transcendent | `FTranscendentState`(`TranscendentID` == `DT_Transcendents` 행 이름), `Doctrine.*`, `DT_Transcendents` | Tier 4 AI 존재. 타락(침묵/동화/광신/회귀 — 정의 4, 판당 활성 ≤ 3)·천상·독립. 판에 교리마다 1체다(은하마다 따로 두지 않는다. 타락은 건너지 않는다) (§4.2) |
| 소속 (계 태그) | Allegiance | `FTranscendentDef.Allegiance`: `Realm.Abyssal/Celestial/Independent`. 정의에만 있고 상태에 없음 | 타락 / 천상 / 독립. 계 태그 = 소속, Tier 의미론 = 보임 (§3.2, §4.2) |
| 모방의 원리 | Counterfeit principle | `FTranscendentDef.CounterfeitOf`(`Truth.Rest/Union/Worship/Renewal`) | 타락은 만들지 못하고 흉내 낸다. 모든 교리는 참된 것 하나의 위조품 (§4.2) |
| 회귀 (교리) | Recurrence | `Doctrine.Recurrence`, 호칭 바퀴·돌이키는 자·다시 보내는 이, `FTranscendentDef.Title` | 4번째 타락 교리, 부활의 위조·재사용. 행은 Phase 6+. '윤회'는 해탈의 길 텍스트에만 (§4.2) |
| 귀환 제안 | Return lure | `enc_recurrence_return`, `UPlayerRoleSubsystem::OfferReturn()`, `DT_Rules.ReturnLureMaxLanes/ReturnedLivesToSeize/LureAcceptByPolicy` | 사망 파이프라인의 선택적 분기. 수락해도 게임 오버 아님 (§4.4) |
| 돌아온 자 | Returned | `Status.Returned`, `Succession.Returned`, `FLifeRecord.SuccessionKind/bReturnOffered` | 귀환 수락 생애. 3회면 `Way.Seized` 영구 (§4.4) |
| 계승 종류 | Succession kind | `Succession.Installed/Called/Returned` | 첫 생애 / 계승 추첨 / 귀환 수락 (§4.4) |
| 주장 | Claim | `Hazard.Claim`(`crisis_claim`), `Log.Claim`(1인칭), `FTranscendentDef.ClaimTemplates/MaxInfluencedSystems`, `DT_Rules.ClaimAdjacencyLanes/VigilClaimExtension/SphereEventWeightStep` | 영혼 경제의 유일한 형태. 만료 = 영향권 편입, 저장 필드 0 (§4.2) |
| 기록됨 | Recorded | `Log.Record`(무주어 수동태) | 서술자의 유일한 사후 문장 "기록되었다" (§4.2, §8.3) |
| 남은 자 법 | Law of the remnant | `Law.Remnant` | 한 문명의 마지막 미편입 성계는 영향권에 넣지 못한다(주장·교단 만료·조공·귀환 수락 — 기본값, §16 #43. 셈은 정착된 성계로 한다) (§4.2, §4.5) |
| 두 책 | Two Books | `BuildChronicle` 장: 기록된 이름들 / 주장된 이름들, 슬롯 `{Transcendent}` | '다른 책'은 펴지지 않는다. 사람·성계의 수 렌더 없음(주기·비율 허용), 인물 없음 — 역할 슬롯·`{PopBand}` 금지 (§8.4) |
| 영혼의 행방 | Fate of souls | 식별자 없음. 테스트 `Data.NoVerdictWords`(Phase 5 — 그때까지 스키마의 낱말 목록 `verdict`) | 세 목소리 규칙(고백·주장·수동태). 게임은 누구의 영혼이 어디로 갔는지 말하지 않는다 (§8.3) |
| 요약 단위 | Summary unit | 식별자 없음(원칙). 관련: `Population`(float), `FNotableRecord`, `FTranscendentState`, `FPlayerEntityState`, `GetPopulationBand` | 사람·문명·영·육신은 전부 집계 또는 기록이다. 능력치를 가진 개인은 플레이어의 그릇 하나. 이름은 사건이 준다 (§1.3) |
| 백성 | Peoples | `FCivilizationState.Population`, 슬롯 `{PopBand}` | 문명 단위 집계. 영혼이 아니고 개인이 아니다. 서술자가 사람을 부르는 집합 명사(백성·이들·집) (§7.2, §8.3) |
| 규모 등급 | Population band | `EPopulationBand`, `GetPopulationBand()`(파생, 저장 안 함), `DT_Rules.PopulationBandThresholds`, ST_CA 라벨 6, 슬롯 `{PopBand}` | Tier 기준 + Population 보정. 소수·수백만·수십억·수천억·수조·셀 수 없는. UI·로그·연대기에 사람 수 없음. '남은 자'는 등급이 아니다. 머릿수의 조사가 아니며 별의 수와 견주는 값이 아니다 (§8.3) |
| 인물 | Notable | `FNotableRecord`(Role·NameIndex·FirstTick·LastTick·SourceRow), `FCivilizationState.Notables`, `Notable.*` 태그 6, `EEffectType::NameNotable`, `EConditionType::CivHasNotable`, `DT_Rules.MaxNotablesPerCiv`, 역할 슬롯 6 | 사건이 이름을 준 소수의 기록. 엔티티·에이전트 아님, 5필드 고정, 문명당 ≤ 4, 등장·퇴장 틱(생몰 아님). 두 책 1부 '기록된 이름들'(백성·성계·주기)과 다르다 — 인물은 어느 책에도 없다 (§1.3, §7.2, §8.4, §12.2) |
| 들은 자 | Hearer | `Notable.Hearer` | 방문 응답·책 발견이 이름 붙이는 인물. 사자는 이름이 없고 들은 자만 이름을 가진다. 간접 화법만, 내용은 전하지 않는다 (§4.5, §5.11) |
| 그릇의 이름 | Vessel name | `FLifeRecord.NameIndex`, 슬롯 `{Player}` | `Install()`이 출생 문명 `PersonNameBank`(비면 `arch_default` 뱅크)에서 뽑는다. 계승마다 새 인덱스, 귀환 수락은 같은 인덱스. 플레이어는 `Notables`에 들어가지 않는다 (§4.4) |
| 인물 이름 뱅크 | Person name bank | `FCivArchetypeDef.PersonNameBank` | 아키타입별 12개(Data.Integrity 하한 10, §9.3). `Trait.Mind.NoSelf` 문명(뱅크 비움)과 T4 이상 문명(가드)은 인물을 만들지 않는다. 성경 인물·실존 인물·천사 이름 금지. 두 번째 이름은 사람에게 붙지 않는다 (§5.11, §8.3) |
| 이름 없는 문명 | Nameless civilization | `PersonNameBank` 빈 행(`arch_swarm`, `arch_machine`) | NoSelf 문명은 인물이 없다. 가장 강하고 가장 눈먼 문명의 비극이 데이터 한 칸으로. 만나기 전의 백성에는 이 말을 쓰지 않는다(그쪽은 만나지 않은 백성) (§5.11) |
| 계시 양식 | Revelation mode | `FCivArchetypeDef.RevelationMode`(`Revelation.*`)·`RevelationTemplates`, `FLogTemplateDef`, `PickTemplateRow`, 슬롯 `{Revelation}` | 한 저자의 말이 각 백성에게 닿는 형식. 텍스트만 바꾼다 (§5.11) |
| 첫 생애 행 / 찾는 백성 | First-life row / Seeking people | `DT_Rules.PlayerArchetypeRow`(가칭 `arch_seeker`, `Trait.Arch.Seeker`), `Revelation.Seeking` | 플레이어의 첫 생애 백성 — 성역 밖 찾는 백성, 두 목소리(의회·회중). v0.8 — 단계 4b부터(그 전 `arch_terran` 지구) (§5.11). 찾는 백성은 계시 양식이 '찾음'인 백성이다 — AI의 `arch_default`도 그렇다 |
| 닫힌 책 | Closed book | `Trait.Faith.Book` / `BookShut`, `edict_reading`, 책 이벤트 3종 | 말이 도착한 백성의 특성. 사물·제도·사건이지 본문이 아니다 (§5.11) |
| 갈라진 문명 | Divided society | `Trait.Society.Divided`: 의회(`Faith.Bank.Civic`) / 회중의 장로들(말이 오기 전 `Faith.Bank.Nations`, 온 뒤 `Hebrew`) | 청원 화자 두 파벌 고정 (§5.10, §5.11) |
| 열방의 호칭 | Names of the nations | `Faith.Bank.Nations` | 이방인이 쓴 호칭만. `Trait.Faith.Seeking` 문명의 헌정명 뱅크 (§4.5, §8.6). 첫 생애 백성의 회중이 말이 오기 전 쓴다 |
| 문턱 / 후원 | Threshold / Patronage | `enc_threshold`(T5, 통과 선택지 없음), `enc_patron_*`(찬탈의 길, 활성 교리마다 1장) | 문은 장소. 받는 길 두 변형 / 후원 결말은 '내가 올라가리라'의 텍스트 변형 (§4.4) |
| 위계 | Hierarchy (엡 6:12) | `FHazardDef.SourceDoctrine` | 정사 = T4 교리 / 권세 = 교단 / 주관자 = 조공 문명 / 악의 영 = 영압. 새 엔티티 0 (§4.2) |
| 영압 (영적 압력) | Spiritual pressure | `ComputeSpiritualPressure()`(파생, 저장 안 함) | Tier 격차의 파생 함수 + 수정자 4곳 (§4.2) |
| 심도 | Depth | `FTranscendentDef.Depth`(0~1, 정의 데이터) | 칸 안의 깊이. 초월자는 행의 컬럼, 문명과 플레이어는 파생·서술만(저장 없음). 영압의 초월자 항에 (1 + Depth). 칸을 넘지 못한다 (§3.2, §4.2) |
| 현현 | Manifestation | 코드 식별자 없음 | 초월자와 거룩한 질서가 규칙을 비트는 방식 — 거룩한 질서의 것은 은혜, 타락의 것은 유혹과 침식. 기존 Hazard·효과·특성·은혜 이벤트의 데이터다 (§4.2, §8.3) |
| 영적 시야 | Spiritual sight | `M_Star` `ViewMode`/`Pressure`/`DominionColor`, PlayerTier >= 3 해금 | 영적·사후 영역의 두 번째 지도 없음(우주 화면은 같은 지도의 줌 단계). Phase 3 꼬리 (§6.4) |
| 영력 | Spiritual power | `Causality`(서사 이름만, 자원 아님) | 바람·숨·빛·음성으로만 묘사 (§5.1, §8.3) |
| 거룩한 질서 | Divine Order | 엔티티 없음. `ImmutableLaws`, `Hazard.Visitation`, `Trait.Faith.*`, 수동태 | 저자는 등장인물이 아니다 (§4.5) |
| 은혜 이벤트 | Grace event | `event_grace_rescue/remnant/jubilee`, `Event.Grace.*`, `Law.Sabbath`, `Flag.Grace.Spent` | 조건으로 유도 불가, 판당 1회, 거절 가능 (§4.5) |
| 사자 | Messenger | `enc_messenger_*` | 이름 없음, 숭배 거절, 간접 화법 (§4.5). 가리키기만 하고 책을 건네지 않는다 |
| 남은 자 | Remnant | `Trait.Faith.Remnant`, `event_grace_remnant` | 어느 문명에도 뜨는 신앙 특성 (§4.5) |
| 길 | Path | `Path.Stillness/Release/Axiom/Covenant`, `Power.Path.*` | 한 저자에 대한 여러 응답. 세력도 초월자도 아닌 조우 덱 데이터. 전용 엔딩 없음. 네 은유 바람·숨·빛·음성 (§4.5) |
| 통치자 | Ruler | `EPlayerRole::Ruler` | 부르심을 입어 구별되고 날이 더해진 자 — 한 백성을 맡은 자의 모드. 첫 생애는 서막 뒤 고정 |
| 방랑자 | Wanderer | `EPlayerRole::Wanderer` | 단독 개체 모드. `Succession.Called` 취임은 항상 Wanderer(귀환 `Succession.Returned`은 사망 당시 Role) |
| 모성계 | Home system | `FCivilizationState.HomeSystemID`, 슬롯 `{HomeSystem}` | '행성' = 모성계 1개 (§5.10) |
| 청원 카드 | Petition card | `FEncounterDef` + 조건 `PlayerRoleIs Ruler` | 통치자 모드의 결정 주기를 채우는 카드 (§5.10) |
| 섭정 | Regent | `AI.Profile.*` 태그, 인물 기록 `Notable.Regent`(`BecomeWanderer` 훅 ①, `ReturnToFormerCiv`가 닫음) | 통치자 이탈·사망 시 문명을 맡는 AI 프로필. 폐위(`Reign.Deposed`)에는 섭정 기록이 없다 (§5.10, §7.5) |
| 경외도 | Reverence | `FCivilizationState::ReverenceToPlayer` | 방랑자 → 통치자 전환 조건. 예배가 아니다. 사망·폐위 시 ×0.5 |
| 헌정명 / 시대 명명 | Dedication / Era naming | `DT_Dedications`, `{Dedication}`, `{Epithet}` | 사건 뒤에 문명이 붙이는 이름 (§8.6, §5.10) |
| 조우 | Encounter | `FEncounterDef`, `DT_Encounters` | 방랑자 텍스트 선택 카드 (통치자 모드에서는 청원). 백성을 처음 알아보는 일은 조우가 아니라 만남이다 |
| 유물 | Relic | `DT_Relics` | 방랑자 소지품, 슬롯 3 |
| 거대 프로젝트 | Megaproject | `FStarSystemState::ActiveMegaproject`(건설 칸)·`ResponseMegaproject`(대응 칸 — Hazard와 함께 산다), `CompletedMegaprojects`(행별 완성 목록), `MegaprojectCompletedTick`, `DT_Megaprojects`, `project_settle`(정착)·`project_crossing`(건넘, 6a) | 통치자 경로 업적의 실체. 프로젝트의 자리는 늘 '건설 칸'·'대응 칸'이고 둘을 함께 이르면 '프로젝트 칸'이다 — '칸'만 쓰면 사다리의 칸이다 |
| 칙령 | Edict | `DT_Edicts` | 제국/고향 은하 범위 수정자 |
| 특성 | Trait | `Trait.*` 태그, `DT_Traits` | 문명 고유 성격. `DT_Traits` 컬럼 `BodyMod/MindMod/CausalityMod/LifespanTicksMod` |
| 문명 아키타입 | Civ Archetype | `FCivArchetypeDef`, `DT_Civs` | 문명 생성 템플릿 행 (§5.11) |
| 윤리 축 | Ethics Axes | `FEthicsAxes` | 평화/영성/개방 3축 |
| 인과 | Causality | `ECosmicResource::Causality` | Tier 3 해금 자원. 개인 능력치 이름과 동일 |
| 틱 / 주기 | Tick / Cycle | `FGalaxyState::Tick` | 고정 스텝 시뮬 단위. 시계는 판에 하나다. UI는 1주기를 1년으로 보일 수 있다(토글, 첫 기록부터) — 서술·연대기는 주기 |
| 레인 | Lane | `FStarSystemState::Lanes` | 성계 간 이동 경로. 같은 은하 안의 것만 — 다리는 레인이 아니다 |
| 연대기 | Chronicle | `UCosmicLogSubsystem::BuildChronicle` | 엔딩용 로그 재편집. 생애(대)별 장 |
| 부름 | Calling | `FCallingState`, `FGalaxyState::Calling`, `Log.Calling` | 플레이어의 정체. 그릇을 넘어 이어진다 (§4.4) |
| 그릇 (육신) | Vessel | `FPlayerEntityState` | 사망 시 전부 초기화되는 쪽. `PersonalTier`·`Powers`·`CompletedFeats`는 `FCallingState`에 유지 (§4.4 3분할 표) |
| 생애 / 대(代) | Life | `FLifeRecord`, `LifeIndex`, `BornTick`, `Lives` | 판 안의 장. "제 N대" |
| 사망 / 사인 | Death / Cause of Death | `EEffectType::KillPlayer`, `Death.*` 태그, `CauseSourceRow` | SourceRow가 있는 효과만 |
| 시간선 도약 | Timeline Leap | `LeapTicks`, `RunHeadless` | 공백기 동안 은하가 혼자 흘러가는 구간 |
| 공백기 | Interregnum | `ELifeState::Interregnum`, `ELogMode::Interlude`, `BeginInterlude()`, `BuildWorldDigest` | 부름이 쉬는 사이 |
| 계승 / 취임 | Succession / Installation | `Succeed()`, `Install(SystemID, CivID, Role)` | 계승 추첨과 새 그릇의 시작. 첫 생애도 `Install()` |
| 재위 종료 사유 / 상태 | Reign end / Status | `Reign.Abdicated/Deposed/Died`, `Status.FallenMonarch/Abdicated` | 왜 방랑자가 되었나의 기록 (§5.10) |
| 생존 주기 | Cycles lived | `TicksLived = DiedTick - BornTick` | 비문의 숫자 |
| 수직 슬라이스 | Vertical Slice | - | §12.1 정의 |
| 판 | Run | 식별자 없음(낱말). 코드 식별자의 Galaxy — `FGalaxyState`·`GalaxyID`·`NewGalaxy`·슬롯 `Galaxy_<GUID>`·`L_Galaxy` — 가 가리키는 것 | 우주 시드 1개 = 부름 1회 = 저장 슬롯 1개. 은하는 여럿이고 판은 하나다 (§9.2) |
| 고향 은하 | Home galaxy | `DT_Rules.HomeGalaxyRow`(`gal_milkyway`), 성계·문명의 `GalaxyRow` | 매 틱 상세로 시뮬되는 은하 하나(우리은하). `NewGalaxy`가 모든 성계·문명의 `GalaxyRow`에 이 행 이름을 찍는다. NAME_None은 '아직 찍히지 않음'이다 (§5.11, §7.2) |
| 무대 은하 | Stage galaxy | `DT_Galaxies` 행 가운데 `BridgeTicks` > 0인 것(안드로메다·삼각형자리·대마젤란·소마젤란) | 이름과 다리가 있는 은하. 6a부터 저장 상태 없이 요약으로 흐른다(본문의 '요약 은하'는 요약으로 흐르는 무대 은하를 가리킨다). VS에서는 표시 전용 (§5.11, §7.4) |
| 풍경 은하 | Scenery galaxy | `DT_Rules.SceneryGalaxyCount`, `Derive`의 용도 Scenery | 셀 수 없이 많은 나머지 은하. 표현 계층이 그리기만 한다 — 상태·역사·방문이 없다 (§5.11) |
| 요약 | Summary | `SummaryAt(GalaxyRow, Tick)` (6a) | 무대 은하의 대략적인 상태. 순수 함수의 값(칸별 백성 수와 닿아 있는 교리 태그)이고 화면에는 낱말로만 나온다 (§7.4) |
| 배경 별 | Backdrop star | `DT_Rules.BackgroundStarCount`, `Derive`의 용도 Backdrop | 은하의 모양을 그리는 점. 그림뿐이고 상태·해시가 없다 (§3.1) |
| 생명 성계 / 무주 성계 | Living world / Wild world | `FStarSystemState`, `OwnerCivID == NAME_None` | 성계 = 시뮬 노드 = 생명이 있거나 백성이 사는 별. 주인이 없는 것이 무주 성계: Hazard·주장의 대상이 아니고 정착의 대상이다 (§3.1, §5.7) |
| 정착 / 정착된 성계 | Settlement / Settled system | `project_settle`, 술어 '소유이고 `ActiveMegaproject != project_settle`' | 정착 = 자기의 정착된 성계에서 1레인인 무주 성계를 거대 프로젝트로 제 것으로 삼는 일(§3.2 표 T2의 '성계 확장'). 정착된 성계만 수입·Hazard·계승 후보·남은 자의 셈·다음 정착의 발판이 되고 마지막 성계의 문이 묻는 성계가 된다(계승 후보의 예외 — 정착된 성계가 하나도 없을 때의 마지막 완화 — 는 정착과 문명 제거 작업의 리뷰부터 비어 있다, §4.4) (§5.7) |
| 죽은 노드 | Dead node | `StarClass` 빈 태그(§16 #29 ⑤) | 파괴된 성계. `Systems`에서 지우지 않는다 — 주인도 레인도 없고 생명 성계가 아니다 (§7.2) |
| 보유 상한 | Realm cap | `DT_Rules.RealmCapByTier` | 문명이 정착으로 가질 수 있는 성계 수의 칸별 상한(1/3/4/7/10). 법이 아니라 노브이고 AI와 플레이어에 같다 (§5.12) |
| 넷째 칸 이상의 존재 / 넷째 칸의 제동 | Beings of the fourth rung and above / Fourth-rung brake | `FGalaxyDef.AIFourthRungCap` | 한 은하에 3~7로 설계한 소수: 미봉인 Tier 4 타락 + 독립 초월자 + T4 이상 AI 문명. 천상 사자와 플레이어는 세지 않는다. 상주하는 T5는 없다. 제동은 AI 간이 판정 전용이고 '자리'라 부르지 않는다 (§4.2, §5.12. 넷째 칸의 셈의 정의는 §7.4 건넘 단계) |
| 앎 | Knowing | `FKnowingState`, `FGalaxyState.Knowing`, `NoteMet`·`NoteKnown` | 부름이 알게 된 것의 기록. 저장되지만 세계의 규칙은 읽지 않는다(쓰기만 한다). Insight의 표시명 '지식'과 다른 말이고 자원이 아니다 (§4.4, §8.1) |
| 아는 성계 | Known system | `Knowing.KnownSystems` | 한 번 알게 된 성계 — 출생 성계, 보는 거리 안에 든 성계, 제 백성이 받은 성계, 방랑자의 눈이 새로 서며 적은 이웃. 한번 알면 계속 안다 (§4.4, §7.4, §8.1) |
| 보는 거리 | Sight (reach) | `DT_Rules.SightLanesByTier`·`SightIntervalTicks`·`FirstSightTick`·`ScanSightBonusLanes` | 지금 그릇이 닿는 범위(레인 수). 파생이고 저장하지 않는다. §3.2의 '보임'(격의 의미론)·§6.4의 '영적 시야'(보기 전환)와 다른 말 (§7.4) |
| 만남 / 첫 접촉 | Meeting / First contact | `log_contact` / `log_first_contact`, `petition_first_contact`, `Knowing.FirstContactCivID`, `Knowing.PendingContactCivID` | 만남 = 한 백성을 처음 알아보는 일(각각). 첫 접촉 = 그 판의 첫 만남(§2.4 타임라인 5:30의 박자) — `Knowing.FirstContactCivID`가 NAME_None일 때 `NoteMet`이 적는 만남이고, 같은 호출이 그 칸에 만난 백성의 ID를 적는다(`Install`은 적지 않는다. 판에 한 번이고 계승에도 남는다 — 정의 §7.4). 방문이 아니라 알아봄(신호)이다. 청원 카드는 첫 접촉에만, 통치자에게만 (§7.4) |
| 만난 백성 / 만나지 않은 백성 | Met people / Unmet people | `Knowing.MetCivs` | 알아본 백성 / 아직 그렇지 않은 백성(지도·목록·외교에 없다). 개발자의 '조우한 뒤'는 본문의 '만난 뒤'다. '조우'는 조우 카드만 뜻한다 (§6.4, §8.1) |
| 만난 은하 | Met galaxy | `Knowing.MetGalaxies` (6a) | 만난 백성과 건넘을 주고받은 은하. 그 전에는 종류와 현실 수치 문구만 보인다 (§7.4) |
| 이야기의 범위 | Scope of telling | `Push`·`PushRow`의 `FocusSystem`·`FocusCiv`, `FCosmicLogEntry.bUntold`, `SetTellAll`, `log_far_ascent`·`log_far_event` | 줄을 쌓을지 정하는 규칙. 범위 밖의 Severity 2·주장·기록은 `bUntold`로 쌓이고 그 밖은 쌓이지 않는다 (§8.1) |
| 보는 쪽 Tier | Viewer tier | `UGalaxySimulationSubsystem::GetViewerTier` | 부름이 세상을 보는 칸 — 통치하는 동안은 통치 문명의 Tier, 그 밖(방랑자, 공백기)은 개인 Tier. 격차 규칙과 줄의 창이 읽는다 (§7.4, §8.1) |
| 격차 규칙 | Gap rule | `NoteMet`, 이야기의 범위 | 보는 쪽보다 세 칸 이상 위의 백성은 만나지 못하고 그 백성의 줄도 말해지지 않는다. 아래 칸은 막지 않는다 (§3.2, §8.1) |
| 먼 줄 | Far line | `log_far_event`·`log_far_ascent`, `Knowing.FarLineUntilTick` | 범위 밖 주제의 Severity 2 줄 대신 말해지는 이름 없는 한 줄 — 시계는 읽힌다. 셋째 칸 이상의 승천은 매번, 그 밖은 많아야 300틱(`FarLineCooldownTicks`)에 한 번 (§8.1) |
| 전부 쌓는 모드 | Tell-all mode | `UCosmicLogSubsystem::SetTellAll` | 범위를 묻지 않고 모든 줄을 쌓는 실행의 모드(저장하지 않는다). 테스트와 커맨드렛만 켜고 게임은 끈다. 어느 모드든 세계 해시는 같다 (§8.1) |
| 방랑자의 눈 | Wanderer's eye | `TickSight`, `KnowAround`(`BecomeWanderer`·방랑자 `Install`) | 방랑자가 선 성계에서 개인 Tier의 거리로 보는 눈. 방랑자가 되는 틱에는 선 성계와 같은 은하 행의 1레인 이웃을 곧바로 안다(만남은 다음 갱신) (§4.4, §7.4) |
| 말하기 스트림 | Tell stream | `FGalaxyState.TellRngState`, `DrawTell` | 문장 변형만 뽑는 난수 출처. 어떤 규칙도 읽지 않는다 (§9.2) |
| 파생 정수 | Derived value | `CosmicSeed::Derive`·`DeriveForName`(`Core/CosmicSeed.h`), 용도 상수 9 | 시드에서 상태 없이 나오는 값. 식과 상수는 동결 (§9.2) |
| 세계 해시 | World hash | `ComputeWorldHash()` | 앎·로그·연대기·말하기 스트림을 뺀 해시 — 앎이 세계를 바꾸지 않음을 재는 자 (§9.2) |
| 다리 | Bridge | `FGalaxyDef.BridgeTicks` | 고향 은하와 무대 은하 사이의 건넘 틱. 레인이 아니고 상태가 없다. '문'·'관문'·'길'이라 부르지 않는다 (§5.9) |
| 건넘 / 건너는 배들 | Crossing / the crossing ships | `project_crossing`, `FTransitRecord`, `FGalaxyState.Transits`, `Transit.People` | T4 이상의 백성이 다른 은하에 무리를 보내는 일. 건너는 것은 유닛이 아니라 기록이다. 방주 = 구출, 건너는 배들 = 건넘 (§5.7) |
| 건너온 백성 | Arrived people | `FCivilizationState.OriginGalaxyRow`, `FTransitRecord.ResultCivID` | 도착해 고향 은하의 무주 성계 하나에 선 새 문명(Tier 4, 인물 없음) (§5.7) |
| 먼저 건넌 자 (겹침 주의) | Ascendant | `Notable.Ascendant` | AI 문명이 T3에 이를 때의 인물 기록. 여기서 '건넌'은 문턱(T3)을 넘은 것이고 은하의 건넘(`project_crossing`)과 무관하다 (§4.3) |
| 정착률 (겹침 주의) | Claim settle rate | CSV `ClaimsSettled` / `ClaimsSpawned` | §9.3의 낱말. 주장이 만료로 확정된 비율이며 정착(`project_settle`)과 무관하다 |
| 길 덱의 '첫 만남' · '문명을 만나면' (겹침 주의) | Path opening card / wanderer encounter | 길 덱의 첫 카드(§4.5), 루스드라 카드(§5.9) | 조우 카드의 일이다. 만난 백성(`Knowing.MetCivs`)·첫 접촉과 다른 말이다 |
| 서막 (부르심) | Prologue | 식별자 없음(표현 계층의 상태 — 저장하지 않는다) | 첫 틱 전의 카드. 틱 밖이고 칸이 아니다 — 시뮬은 `SetTimeScale(0)`이고 배속 입력이 없다 (§2.4, §4.4. v0.8 — 단계 6부터) |
| 부르심 | Calling (event) | `UPlayerRoleSubsystem::ReceiveCalling`(가칭) | 서막 끝의 소명 — 서막의 결과를 상태에 적고 칸을 주지 않는다. 구원의 사건이 아니다. 플레이어 자신을 이르는 '부름'과 다른 말 (§4.4, §7.5. v0.8 — 단계 6부터) |
| 받은 높음 | Received height | 식별자 없음 | 구별·날이 더함·눈이 열림. 칸 밖의 일이다. 게임 안 낱말은 받은 꼴(구별되었다·부르심을 입었다·날이 더해졌다·눈이 열렸다·깨닫게 되었다)만 (§4.4, §8.3) |
| 날이 더함 | Days added | `DT_Rules.BaseLifespanTicks` 0(노화 0) | 부름이 머문 그릇은 날이 더해진다. 불사가 아니다 — 출처 있는 죽음은 그대로 온다 (§4.4, §16 #10) |
| 성역 | Sanctuary | `sol`, 가칭 `Law.Hedge`·`CosmicState::IsSanctuary` | 태양계 노드. 다시 쓸 수 없는 법 하나로 소유·정착·주장·파괴에서 빠지고 압력은 0이다. 세력·색·화자가 아니다 (§4.5 ①, §7.4. v0.8 — 단계 4부터) |
| 성역 밖 | Beyond the sanctuary | 식별자 없음(설계 낱말) | 태양계 바깥 전부. 게임 안에는 이름이 없다 — 대도감의 '외곽 심연'은 쓰지 않는다 (§4.5 '성역 밖') |
| 지경 | Bound | `enc_sanctuary_bound`(가칭) | 방랑자의 성역 경계 카드 — 한 장으로 끝나고 그릇은 떠난 성계로 돌아온다. 방문 위기의 '신을 벗으라'와 다르다 (§5.9, §16 #81. v0.8 — Phase 4부터) |
| 고넬료 사슬 | Cornelius chain | `Hazard.Visitation` → `Trait.Faith.Visited` → `enc_messenger_*` → `event_word_arrived`(가칭) → `Trait.Faith.Book` | 방문 → 사자 → "말이 도착했다". 사자는 가리키기만 한다 (§5.11. v0.8 — Phase 4 단계 7부터) |
| 옛 땅의 기억 | Memory of the old land | 식별자 없음(화면) | 말이 도착한 백성에게 열리는 한 쪽 — 옛 땅(성역)의 이야기를 두 목소리(회중의 고백·의회의 기록)가 받아 적는다. 연표가 아니다 (§5.11, §16 #77. v0.8 — 단계 7부터) |
| 해 (UI 표시) | Year (display) | 보는 사람의 설정(상태·해시 밖) | 1주기 = 1년, '첫 기록부터' 센다. 기본은 '주기'이고 서술·연대기는 주기다 (§2.1, §3.4, §16 #82. v0.8 단계 2) |
| 행성 | Planet | `DT_Planets`(정의 데이터) | 성계 패널의 접힌 목록. 노드·상태·해시·저장 ID가 없고 아는 성계에만 보인다. 모성계 줄의 '행성'(통치하는 모성계 하나)과 다른 말 (§3.1, §7.3. v0.8 — 단계 5부터) |
| 관측 목록 | Observed catalogue | `Tools/data/sources/`(스냅숏과 README) | 태양 근방의 관측된 계·행성·별 이름의 출처. 관측인 것은 별·행성·거리·등급이고 그 위의 생명·백성·사건은 설정이다 (§3.4, §10.1. v0.8 — 단계 5부터) |
| 대도감 | Bestiary | `cosmic_species_codex.md`(저장소 루트, 게임 데이터 아님) | 개발자의 작가 노트 — 성역 밖의 세계관 원천. 원문은 고치지 않고 원형만 빌린다 (§5.11 '대도감') |
| 원형 장부 | Archetype ledger | 식별자 없음(설계 문서의 표) | 종족마다 빌린 원형·빌린 곳·쓰지 않는 말·게임에 보일 꼴을 적는 표. 금지어 목록의 씨앗 (§5.11) |
| 성간 기록원의 기록 | Archive quote | 식별자 없음(카드 안의 인용) | 대도감의 서문·생존 수칙이 게임에 들어올 때의 이름 없는 인용 목소리 — 조우·스캔·청원 카드 안에서만("…라고 기록은 적는다"). 로그 화자 층이 아니다 (§5.11, §8.2) |
| 항성 안정화 | Star stabilization | `project_stabilize`·`feat_supernova_stabilized`(행 이름 그대로) | 초신성 전조 위기의 대응 프로젝트와 칸 2→3 업적의 표시명. 위기 문구는 별 등급마다(M·K·G 항성 폭풍, O·B·백색왜성 쌍성 초신성) (§3.2, §4.1. v0.8 — 단계 5부터, 그 전의 데이터 표시명은 '초신성 안정화') |

---
