import os, re, sys

D = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'draft')
OUT = sys.argv[1]
NOTES_OUT = sys.argv[2]

def rd(n):
    return open(os.path.join(D, n), encoding='utf-8').read()

def rep(text, old, new, count=1, name=''):
    n = text.count(old)
    if n != count:
        raise SystemExit(f'[{name}] expected {count} got {n}: {old[:80]!r}')
    return text.replace(old, new)

def cut_between(text, start, end, new, name=''):
    i = text.find(start)
    j = text.find(end, i)
    if i < 0 or j < 0:
        raise SystemExit(f'[{name}] markers not found: {start[:40]!r} / {end[:40]!r}')
    return text[:i] + new + text[j + len(end):]

# ---------------------------------------------------------------- h0_1
h0 = rd('h0_1.md')
h0 = rep(h0, "`FleetDelayCountdownShare` — 장치 커밋에서는 지금의 상한을 내는 값)",
         "`FleetDelayCountdownShare` 0 — 장치 커밋에서는 지금의 상한 그대로)", name='h0')
h0 = rep(h0, "**주인 없음 그대로**: 조공(#137 — #156의 '조공 없이'는 그때까지 빈 조건).",
         "**주인 없음 그대로**: 조공(#137 — #156의 '조공 없이'는 그때까지 빈 조건).\n"
         "  - **열린 [개발자] 물음 그대로**: 넷째 칸 백성을 떠난 그릇의 보는 칸(#96·#100·#108 — 보는 칸에 그 생애의 바닥을 두는 안은 시야의 규칙 `docs/20:127`을 바꾼다; '추가 이야기와 보완').", name='h0')
h0 = rep(h0, "- **답과 승인된 결정이 부딪는 곳**(열셋 — 풀이는 각 결정):",
         "- **답과 승인된 결정이 부딪는 곳**(열셋 — 결정들은 이 번호를 '§0 부딪는 곳 N'으로 부른다; 풀이는 각 결정):", name='h0')
h0 = rep(h0, "`LawsHeldTicks`·`PressureEnduredTicks` — 필드만의 커밋 `[B-6b1]`, `SaveVersion` 1 그대로)",
         "`LawsHeldTicks`·`PressureEnduredTicks` — 필드와 문서가 먼저인 커밋 `[B-6b1]`, `SaveVersion` 1 그대로 — §2.1)", name='h0')
h0 = rep(h0, "묶인 초월자를 멈춘 것으로 읽는 곳은 이미 열둘쯤(압력·소모·주장자·알아챔·정착 거절·계승 거름·지도 고리 …)이라",
         "묶인 초월자를 멈춘 것으로 읽는 곳은 이미 여럿(압력·소모·주장자·알아챔·정착 거절·계승 거름·지도 고리 … — 노트 §0.4′)이라", name='h0')
h0 = rep(h0, "VS 정의의 수량(#160), `DECISIONS_LOG`, `CLAUDE.md` 규칙 4·13·14·15. 결정 37~40.",
         "VS 정의의 수량(#160), `DECISIONS_LOG`, `CLAUDE.md` 규칙 4·13·14·15. 결정 38~40(커밋 나눔은 결정 37·§8).", name='h0')

# ---------------------------------------------------------------- h2a
h2a = rd('h2a.md')
h2a = rep(h2a, "- **기준값**: 다섯 줄의 두 해시와 해시 판 둘이 옮고 CSV의 결정적 칸은 `[B-6a7]`과 같다(`[B-3a]`·`[B-6a2]`의 꼴); 새 해시는 커밋 메시지에. 테스트 `Sim.SaveLawsHeld`(왕복, 칸 없는 세이브는 0)·`Sim.HashCoversNewCounters`(하나하나가 두 해시를 옮긴다) — `UPROPERTY`를 뗀 변이가 깬다.",
          "- **기준값과 테스트**: §5.4의 `[B-6b1]` 행, §6의 `[B-6b1]` 항목.", name='h2a')
h2a = rep(h2a, "(D표의 '셈과 함께'를 이렇게 읽는다)", "(§8)", name='h2a')
h2a = rep(h2a, "- **이름은 최종이고 `crisis_pre_supernova` 뒤로 정렬된다**(예 `crisis_species_extinction`): 발생 몫은 행 이름 순서로 쌓이므로(`Data/CosmicDataTables.cpp:143-147`, `Sim/GalaxySimulationSubsystem.cpp:2297-2362`) 앞 `Draw` 행 셋의 몫이 제자리이고 절멸이 놓이기 전까지 세계가 같다.",
          "- **이름 `crisis_species_extinction`**(마지막 이름 — `crisis_pre_supernova` 뒤로 정렬되는 까닭은 §3.1): 절멸이 처음 놓이기 전까지 세계가 같다.", name='h2a')
h2a = rep(h2a, "(`pillars.md:91`, Phase 6+)", "(`pillars.md:106`, Phase 6+)", name='h2a')
h2a = rep(h2a, "6. **기록 줄**: `RecordRow` = 새 `Log.Record` 행(이름·슬롯은 §3), 만료 줄 바로 뒤",
          "6. **기록 줄**: 새 `Log.Record` 행 `log_record_extinct`(§3.1), 만료 줄 바로 뒤", name='h2a')
h2a = rep(h2a, "를 훅·통치자 경고와 자동 정지(§4)·새 테스트(§6)가 읽는다.",
          "를 훅·패널의 통치자 경고(§4 — 새 자동 정지는 없다)·새 테스트(§6)가 읽는다.", name='h2a')
h2a = rep(h2a, "`CosmicTests::WithoutDeath`의 주석은 '데이터의 죽음만'(절멸의 끔은 `-WithoutB6bData` — §5), `Death.Hazard`의 설명",
          "`CosmicTests::WithoutDeath`의 주석(§5.1), `Death.Hazard`의 설명", name='h2a')
h2a = rep(h2a, "`DeathRowOf`가 `KillsTheRulerOf`인 행의 죽음에 제 행(§3)을 고른다",
          "`DeathRowOf`가 `KillsTheRulerOf`인 행의 죽음에 제 행 `log_death_extinction`(§3.1)을 고른다", name='h2a')
h2a = rep(h2a, "칙령 철회(`Sim/GalaxySimulationSubsystem.cpp:2820-2823`)", "칙령 철회(`Sim/GalaxySimulationSubsystem.cpp:2818-2823`)", name='h2a')
h2a = rep(h2a, "`GrantedPowers` 비움, 이름 최종(규칙 7).", "`GrantedPowers` 비움, 이름 `feat_passage_by_ark`(§3.1 — 규칙 7).", name='h2a')
h2a = rep(h2a, "- **B7에 남는 것**: 프로필의 외교·통치자에게 청하는 카드·조약 깨기·AI의 교역. 고칠 문서: `crisis_and_transcendents.md:134`의 '여섯뿐'(법칙 걸음과 함께 여덟)·`systems.md:65`·`docs/50`의 B7 칸·규칙 4(결정 38).",
          "- **B7에 남는 것**: 프로필의 외교·통치자에게 청하는 카드·조약 깨기·AI의 교역(고칠 문서는 §7).", name='h2a')

# ---------------------------------------------------------------- h2b
h2b = rd('h2b.md')
h2b = rep(h2b, "- **셈**: 경제 단계 끝 `LowStabilityTicks`·`FallenSphereTicks`의 고리",
          "- **셈**(`[B-6b2]` — §2.1): 경제 단계 끝 `LowStabilityTicks`·`FallenSphereTicks`의 고리", name='h2b')
h2b = cut_between(h2b, "- **줄**: 지금의 완성 줄(", "백성을 그 일의 주인으로 쓰지 않는다.",
    "- **줄**: 지금의 완성 줄(`:2763-2777` — 백성의 일을 3인칭 사실로) 뒤에 묶임의 줄 `log_fallen_bound`(§3.1) 하나 — 말하기 스트림 한 수; 글은 행위자 없는 수동이고 백성을 그 일의 주인으로 쓰지 않는다(§3.5의 4, 결정 32).", name='h2b')
h2b = cut_between(h2b, "- **봉인이 바꾸는 것 — 이미 있는 읽는 곳 열둘**", "`:406-409`만 예외).",
    "- **봉인이 바꾸는 것 — 이미 있는 읽는 곳**(그 자리의 새 코드 없음; 줄마다 노트 §0.4′의 표): 압력의 항·소모·틱당 효과·함대 소모가 없어지고 `FallenSphereTicks`가 0으로; `ClaimantOf`가 새 주장을 내지 않고 선 주장은 주장자가 없어 만료의 편입·알아챔·1인칭·지킴의 격차가 따라 멎는다; 이벤트 가중은 1(추첨 수 그대로); 교리를 묻는 조건이 거짓; 영향권 안 정착의 거절·계승의 거름·정책의 닿음이 영향권을 보지 않는다; 지도는 `Bound`의 회색 고리, 패널은 `Sight.Bound`; 커맨드렛의 셈에서 빠진다. 영향권은 지우지 않는다 — 그것이 '동결'이다(남은 자 법의 `InFallenSphere` `Sim/CosmicHazards.cpp:65-77`은 봉인을 묻지 않는다; 초신성이 죽은 노드를 빼는 `:406-409`만 예외).", name='h2b')
h2b = rep(h2b, "기록이 선 성계에서 선다. 이름은 처음 넣을 때 마지막이다(규칙 7).",
          "기록이 선 성계에서 선다. 이름 `feat_laws_and_binding`(§3.1 — 규칙 7).", name='h2b')

# ---------------------------------------------------------------- h2c
h2c = rd('h2c.md')
h2c = rep(h2c, "### 2.8 방랑자의 증언 (#141 — 결정 20 [개발자]·21;", "### 2.8 방랑자의 증언 (#141 — 결정 20·21 [개발자];", name='h2c')
h2c = rep(h2c, "- **행**(이름 §3): Standing", "- **행** `event_era_endured`(§3.1): Standing", name='h2c')
h2c = rep(h2c, "`log_siege_lifted`; 합성이 빠뜨린 셋)", "`log_siege_lifted`)", name='h2c')
h2c = rep(h2c, "(`…_far` 다섯 — §3)", "(`…_far` 다섯 — §3.1)", name='h2c')
h2c = rep(h2c, "W ≤ 1.24·왕좌 잠금 600", "W ≤ 1.2445·왕좌 잠금 600", name='h2c')
h2c = rep(h2c, "#142는 §5(결정 24).", "#142는 §5.3(결정 24).", name='h2c')

# ---------------------------------------------------------------- h3_4
h34 = rd('h3_4.md')
h34 = rep(h34, "합이 1 아래인 한 세계는 절멸이 처음 놓인 틱부터만 갈린다. `crisis_extinction`이면 역병·전조 앞이라 셋째 칸 이상 주인의 성계마다 데이터를 켠 틱부터 몫이 밀린다. 태그 설명",
          "합이 1 아래인 한 세계는 절멸이 처음 놓인 틱부터만 갈린다(앞으로 정렬되는 이름은 결정 4의 대안). 태그 설명", name='h34')
h34 = rep(h34, "설계의 이름(`crisis:27`, `pillars.md:74`, `docs/50:274`)", "설계의 이름(`crisis:27`, `pillars.md:76`, `docs/50:274`)", name='h34')
h34 = rep(h34, "| `log_siege_on_us_far`, `log_treaty_broken_far` | `[B-6b3]`(결정 19) | 이름 없는 변형 — 상수 둘, `required_rows`. 넷째 칸 전에는 고르지 않는다 |",
          "| `log_siege_on_us_far`, `log_treaty_broken_far`, `log_siege_lost_far`, `log_siege_held_far`, `log_siege_lifted_far` | `DT_LogTemplates`, `[B-6b3]`(결정 19) | 이름 없는 변형(§2.9) — 상수 다섯, `required_rows`. 넷째 칸 전에는 고르지 않는다 |", name='h34')
h34 = rep(h34, "`Ruled`를 받는 조건은 `[B-6b2]`가 평가기에서 정한다(§2.7 — 지금 `SystemOwnedBy`는 `{}`, `:1080`).",
          "`Ruled`를 받는 조건은 `SystemOwnedBy` 하나다(§2.7 — 그 Key 규칙 `:1080`을 넓힌다).", name='h34')
h34 = rep(h34, "8. **로그 행 다섯**: `log_*_far`는 `{OtherCiv}` 없음,", "8. **새 로그 행**: `log_*_far` 다섯은 `{OtherCiv}` 없음,", name='h34')
h34 = rep(h34, "`Data.Integrity`가 '≤ `HomeGalaxy` 행 수 ∧ ≤ 자리의 끝(1 + `EdictSlotSocietyMarks`의 수 = 3)'을 본다 — 넘으면 넷째→다섯째 행이 닿지 않는다(변이 하나).",
          "`Data.Integrity`가 '≤ 자리의 끝(1 + `EdictSlotSocietyMarks`의 수 = 3)'을, `CivLawsHeldTicksAtLeast`를 싣는 행이 있으면 '≤ `HomeGalaxy` 행 수'도 본다(행이 없는 `[B-6b2]`~`[B-6b6]`에서 깨지지 않게) — 넘으면 넷째→다섯째 행이 닿지 않는다(변이 하나).", name='h34')
h34 = rep(h34, "(`UI/CosmicCallingPagesWidget.cpp:406`) — 초안 '맡은 백성이 선 가장 높은 칸'은 다스리지 않은 생애와 오르지 않은 백성에 즉위한 생애에 거짓이라 고쳤다.",
          "(`UI/CosmicCallingPagesWidget.cpp:406`; 결정 30).", name='h34')

# ---------------------------------------------------------------- h5_6_8_9
h5 = rd('h5_6_8_9.md')
h5 = rep(h5, "시간·크기는 노트 §2(B6a 꼴로 A 약 10분·B 약 22분·C 약 25분).",
         "시간은 B6a 꼴의 어림으로 A 약 10분·B 약 22분·C 약 25분(잰 값은 노트 §3).", name='h5')
h5 = rep(h5, "- **#132**(결정 23): 시험 칸 — 각성 위기의 지연 몫 ≤ 50%(띠 10~50%; B6a 64.2%).",
         "- **#132**(결정 23): 시험 칸 — 각성 위기의 지연 몫 ≤ 50%(띠 10~50%; B6a 64.2%) — 몫 0.5에서는 위 끝이 구성상 50%라 깨질 수 있는 시험이 아니라 확인이다(노트 §2 ⓔ).", name='h5')
h5 = rep(h5, "`Log.FarSiegeNameless`: 셋 이상 위의 AI가 에워싸면 `log_siege_on_us_far`·`log_treaty_broken_far`, 말하기 한 수",
         "`Log.FarSiegeNameless`: 셋 이상 위의 AI가 에워싸거나 그 공성이 끝나면 `…_far` 다섯(§3.1), 말하기 한 수", name='h5')
h5 = rep(h5, "법칙 행은 행마다 한 번(C11).", "법칙 행은 행마다 한 번(§0 부딪는 곳 11).", name='h5')
h5 = rep(h5, "**필드와 문서 먼저**: `LawsHeldTicks`(셈의 자리 — `HomeGalaxy` 읽기는 `Scope` 칸과 함께 `[B-6b2]`, 넷째 칸 백성이 없어 어느 쪽이든 0)·",
         "**필드와 문서 먼저**: `LawsHeldTicks`(필드만 — 셈은 `[B-6b2]`, §2.1)·", name='h5')
h5 = rep(h5, "**장치 — 세계**: 열거자 셋·", "**장치 — 세계**: 열거자 셋·법칙 틱의 셈·", name='h5')
h5 = rep(h5, "개발자, 콘솔 없이; 결정 37, B6a 결정 18·21의 꼴)", "개발자, 콘솔 없이; B6a 결정 18·21의 꼴)", name='h5')

# ---------------------------------------------------------------- h7_10_11
h7 = rd('h7_10_11.md')
h7 = rep(h7, "`pillars.md:74`·`:108`(공명의 가리킴 — A3)", "`pillars.md:76`(`enc_witness`의 T2+)·`:108`(공명의 가리킴 — A3)", name='h7')
h7 = rep(h7, "은 `[B-6b4]`; 연대기 위젯 헤더의 낡은 주석은 `[B-6b5]`(결정 29).",
         "은 `[B-6b4]`, 연대기 위젯 헤더의 낡은 주석(결정 29)도 같은 커밋.", name='h7')
h7 = cut_between(h7, "**답**(2026-11-08 — 이 헤더 전에 온 넷):", "그 결정이 가리키는 절의 본문을 고친 뒤 승인한다.",
    "**답**(2026-11-08 — A1~A4의 요지는 §0): A1은 이 헤더의 범위 전부, A2는 결정 2~15, A3은 결정 15·20, A4는 결정 35와 `[B-6b10]`이 받는다. 네 답이 앞 결정과 부딪는 자리와 그 풀이는 §0, 기록은 결정 39. **승인 때 기다리는 답 — [개발자] 열둘**: 1·2·3·5·9·14·20·21·22·32·35·38. 나머지 스물여덟은 권장안이 곧 승인이다(마일스톤 B의 상시 승인). 답이 대안을 고르면 그 결정이 가리키는 절의 본문을 고친 뒤 승인한다.", name='h7')

NEW = {}
NEW[1] = ("1. **[개발자] 셋째 칸 위의 AI가 설계 표의 행을 만나는 길** — 권장: 모든 백성이 같은 행을 읽고, AI에게는 그 행이 물을 때만 서는 좁은 걸음 둘을 B7에서 당겨 온다 — **조약 걸음**(§2.3 — 국경의 다스리지 않는 백성에게 불가침, 세계의 문 `TreatyBlockOf`를 `bMet = true`로)과 **법칙 걸음**(결정 11); 방주와 결박은 지금의 `Choices`가 짓는다. 추첨·줄 없음. "
          "까닭: 설계의 간이 판정은 같은 행에 `SinceTick` 0과 제동을 더한 것이고(`crisis_and_transcendents.md:134`, `systems.md:411`) A2는 행을 '적힌 그대로' 묻는다; 걸음은 행이 묻기 전에 아무것도 하지 않아 장치 커밋이 비트 그대로다; `SetStance`가 이미 세계의 문을 '만남은 참'으로 부른다(`Data/CosmicConditionTypes.h:153`). "
          "대안: ㉮ B7을 B6b 앞으로(B6a 결정 1의 ㉮′ — 엔딩이 한 블록 늦다); ㉯ AI만의 행(새 칸 `Judged`) — '같은 행·같은 조건'(`pillars.md:108`)을 고쳐 쓴다; ㉰ B7까지 셋째 칸 위의 AI 없음 — 다른 질서·#161·'< 15%'·제동을 재지 못한다; ㉱ 방주가 다 지어질 때 조약을 적는다 — 행의 '외교 조약'이 방주의 덤이 된다. "
          "닿는 것: AI끼리의 불가침이 그 짝의 공성을 닫는다(#129 — 노트 §2 ⓖ); 그릇이 맺은 조약이 상대 AI의 다음 칸에도 드는 짝은 툴팁 한 줄(§4); 고칠 문서는 §7. 본문 §2.3·§2.4. *왜 개발자인가*: 승인된 B7의 범위를 줄이고 설계의 AI 규칙을 바꾼다(§0 부딪는 곳 2).")
NEW[4] = ("4. **절멸 위기의 행**(권장): `crisis_species_extinction`(마지막 이름 — `crisis_pre_supernova` 뒤로 정렬되어 앞 `Draw` 행 셋의 조각이 제자리다, §3.1), `Draw`, 발생 조건 `CivTierAtLeast 3`(주인), 카운트다운 250, 확률 c(시작값 1e-4 — 노트 §2 ⓐ); `OnExpireEffects`·`OnResolveEffects`는 빈다(언약은 Phase 4); 화면 이름 '절멸 위기'와 발생·만료·해결의 줄(결정 33). "
          "대안: 역병 앞으로 정렬되는 이름(예 `crisis_extinction`) — 데이터를 켠 틱부터 셋째 칸 이상의 모든 주인에게 조각이 밀린다. 본문 §2.2·§3.1.")
NEW[5] = ("5. **[개발자] 절멸의 만료 훅과 통치자의 둘째 죽음 원천** — 권장: `ExpireHazard`의 태그 코드 안에서 마지막 성계의 문(남은 자의 은혜) → 새 단일 창구 `CosmicHazards::LetPeopleGo`(주인의 성계를 모두 놓는다) → `KillPlayer`(`Death.Hazard`, 성계 없는 주제 — `VesselAtRiskOf`가 이미 그 백성의 통치자에게 닿게 적혀 있다, `Sim/CosmicHazards.h:212-220`) → `DeposeRuler`(초신성 붕괴의 꼴) → 기록 줄; 차례와 까닭은 §2.2. 새 추첨 없음. "
          "까닭: 설계가 VS의 하나뿐인 통치자 죽음으로 이것을 적었다(`crisis_and_transcendents.md:24`·`:170`, `docs/50:198`, `docs/21:756`). "
          "대안: 데이터의 `OnExpireEffects`에 `KillPlayer` — 성계 주제는 거기 선 방랑자에게만 닿고 `KillsAVesselOf`가 그 성계를 계승 후보에서 뺀다. "
          "닿는 것: 데이터의 죽음 핀(`Tests/CosmicDataTests.cpp:2418-2430`)은 그대로, 새 테스트가 코드의 죽음을 핀으로 박는다(§6 `[B-6b6]`); `-WithoutDeath`는 이 죽음을 빼지 않는다(§5.1); 신학 — 죽음의 줄은 통치자가 백성을 '위해 남았다'로 읽히지 않고 애도만(메시아 신호 2 — `theology_and_bestiary_notes.md:236`; §3.5의 3). 본문 §2.2·§2.11. *왜 개발자인가*: B5 결정 6-가가 죽음 원천을 하나로 열었고 핀이 '또 다른 원천은 새 결정'이라 한다(§0 부딪는 곳 5).")
NEW[9] = ("9. **[개발자] 은하 법칙이 무엇이고 넓힘이 무엇을 하는가** — 권장 ㉮: `DT_Edicts`의 넷째 칸 행 셋 이상과 새 정의 칸 `Scope`(`ECosmicEdictScope {People, HomeGalaxy}` — 저장되지 않는다); `HomeGalaxy` 행은 위기 고리 ①(발생 몫 ≤ 1)·②(카운트다운 ≥ 0)만 지닌 백성의 고향 은하의 모든 정착된 성계에 닿게 하고(성계마다 행마다 한 번), 나머지 효과·값·만료의 답(③)은 지닌 백성의 것; 주장·절멸·초신성 전조에는 고리가 없다(§2.4). "
          "까닭: '청지기'·'법 수호'에 맞고 '법칙 왜곡'의 강제가 아니다(`crisis_and_transcendents.md:245`); 추첨 수 그대로; W 그대로(§0 부딪는 곳 11); AI가 칙령을 지니지 않는 까닭(`systems.md:162`)이 그대로 선다. "
          "대안: ㉯ 모든 효과가 모두에게 — 강제이고 B2~B4의 균형을 다시 맞추며 `edict_free_trade`가 역병을 두 번 센다; ㉰ 이름만(넷째 칸 이상에서 지닌 칙령 셋) — 칸이 말하는 넓힘을 짓지 않는다. 본문 §2.4·§3.3의 4. *왜 개발자인가*: 설계에 없는 장치(넓힘의 뜻)를 정하고 다른 백성의 세계에 닿는다.")
NEW[12] = ("12. **결박이 일어나는 길**(권장 ㉮): 건설 프로젝트 행 `project_binding`(넷째 칸, 인과율 없음, 진척 P_bind — 시작값 1,800, 노트 §2 ⓒ), 단일 창구 `CosmicSeal::BindTargetOf`와 시작 거절 `NoFallen`; 완성 훅이 `bSealed`와 `SealedTick`을 한 자리에서 쓴다(`docs/50:231`); 업적은 `MegaprojectComplete project_binding`을 읽어 기록의 `CivID`가 '묶은 백성만'과 '이 생애'를 함께 지킨다 — **새 저장 필드 없음**, `TranscendentSealed`는 행이 쓰지 않는다(§2.5). "
           "까닭: 봉인은 판 안에서 하는 일이다(`crisis_and_transcendents.md:121`·`:128`); 봉인된 타락을 이미 비키는 읽는 곳들(노트 §0.4′)이 쓰기 하나로 영향권을 얼린다. "
           "대안: ㉯ `FTranscendentState.SealedByCivID`와 백성 수준의 조건 — 결박한 성계를 잃어도 남는다(필드 하나 — B6a가 이름한 '주인 필드'; 묶음 B 뒤 다시 본다, 결정 14); ㉰ 봉인을 오름의 값으로 — 세계의 결과가 보이지 않아 `:121`·`:128`과 어긋난다; ㉱ `TranscendentSealed`를 조건으로 — `SinceTick` 0인 AI가 누군가 묶은 뒤 절반을 거저 얻는다(#73). 본문 §2.5.")
NEW[19] = ("19. **보는 것과 부르는 이름**(#107·#122·#121·이름이 새는 줄·#129·#154, 권장): #107 — Core 술어(`Core/CosmicKnowing.h`의 칸의 차와 '세 칸 이상')를 `Tell`·`NoteMet`·`IsShownPeople`이, 칸의 차를 `WithinWandererGap`(≤ 1)이 부른다(비트 그대로, `-TellAll=0` 해시 증명); #122 — 즉위·복귀의 문에 `PersonalTier`에 견준 거절 `FarAbove`(복귀도 같은 문을 지나므로 — `Sim/CosmicWanderer.cpp:74-113`, `IsReturn` `:108` — 넷째 칸 백성을 떠난 그릇은 돌아가지 못한다: 결정 14의 잠김과 같은 쪽, '추가 이야기와 보완'); "
           "이름이 새는 줄 — 공성의 시작과 끝 다섯 줄의 이름 없는 변형 `…_far`(§3.1)을 세계의 칸 차로 고른다(말하기 한 수 그대로); #121 — 지금대로(섭정의 백성은 AI로 오르고 그 다섯째 칸은 다른 질서); #129·#154 — 다시 재고 적기만. "
           "대안: 격차 규칙의 사본 셋을 테스트로 묶어 둔 채 — 넷째 칸 백성이 처음 생기는 블록이라 한 곳이 어긋나면 이름이 샌다. 본문 §2.9.")
NEW[22] = ("22. **[개발자] 견딘 영압의 시대(#156)** — 권장: 저장되는 셈 `PressureEnduredTicks`(소모를 받았고 수도가 봉인되지 않은 영향권 밖인 틱의 연속; `MoveCapital`이 0으로), 한 슬롯 창의 조건 `CivPressureEnduredTicksReached` 600(`systems.md:233`의 시대 주기), `SystemOwnedBy`의 선택자 `Ruled`(`docs/21:619`가 정한 낱말), `ChangeReverence`의 키 `Borders`, 상시 `DT_Events` 행 하나와 세 변형(변형마다 제 시대, `{Epithet}` 없음); **`EpithetRow`를 쓰지 않는다**(`docs/70:192` — 시대 이름은 사람에게 붙지 않는다). 본문 §2.1·§2.7. "
           "개발자에게 묻는 것: '이웃의 경외 +0.2'를 남기는가 — 백성의 견딤을 그릇에게 돌려 메시아 신호 9·10에 가깝다; 권장은 설계대로 남기되 줄은 백성을 말하고 수는 방랑자 패널에만. '조공 없이'는 #137까지 빈다. "
           "대안: 업적 행 — AI가 행을 나눠 쓰고(#156 ③) 별칭이 사람에게 붙는다(①). *왜 개발자인가*: 신학의 자리(경외가 누구에게 가는가)와 보상의 꼴(§0 부딪는 곳 12).")
NEW[30] = ("30. **비문의 칸**(권장): `Epitaph.Peak`을 백성의 일로 고친다 — '다스리는 동안 백성이 이른 가장 높은 칸 {0}', 값이 첫째 칸이면 줄을 내지 않는다(§4). "
           "대안: '맡은 백성이 선 가장 높은 칸 {0}' — 다스리지 않은 생애와 오르지 않은 백성에 즉위한 생애에 거짓이다; 그대로 — 개인 칸으로 읽혀 늘 첫째인 그릇이 넷째·다섯째를 지닌 것처럼 보인다. 본문 §3.2·§4.")
NEW[34] = ("34. **측정 장치**(권장): 러너가 견주지 않는 새 줄 `CosmicBalance race:`(칸은 §5.1·노트 §2.1)와 주장마다의 줄(#142), 깨질 수 있는 `race check`, 측정 전용 스위치 `-WithoutB6bData=1`(끈 짝 — 앞 커밋을 글자 그대로 되낸다)·`-FourthRungCap=<n>`(둘 다 `-SaveAt`이 거절); 노브와 시계는 `-Rules`로. "
           "대안: `ruler:` 줄에 덧붙인다 — 세계가 그대로여도 러너가 견주는 글자가 옮는다(B6a 결정 11). 본문 §5.1.")
NEW[35] = ("35. **[개발자] 미리 적은 목표와 시계의 규칙**(A4·#2·#161) — 권장: §5.3의 판단 넷과 시계의 규칙을 데이터를 켜기 전에 그 글 그대로 고정한다 — 묶음 B가 다섯째 칸에 닿음(R 60의 ≥ 40/60)·#161의 띠(다른 질서 15~35%)·넷째 칸의 계절(1,200~3,600틱)을 판단하고, 시계 C는 느린 통치자 칸의 첫 다섯째 칸 끝의 80번째 백분위를 1,000으로 올려 6,000~18,000에 가둔다(받아들임이 어긋나면 값을 적지 않고 개발자에게). C에서 읽고 알리되 C의 문이 아닌 것: #136·#142·#132·B1 지킴·#129·#154·'< 15%'. C는 6,000에 끝난 B6a 세이브를 다시 열지 않는다. "
           "까닭: 마일스톤 B의 끝은 사람이 엔딩을 보는 일이고(B6a 결정 21) 사람의 걸음은 `-Focus=0`에 가깝다고 놓는다(MVP ⑧ — 중점을 쓰지 않는 판의 셋째 칸 4,425, `DECISIONS_LOG.md:139`). "
           "대안: ㉮ R 60의 80번째 백분위(통치자·정책 관점의 안) — 정책은 사람보다 빨라 사람이 끝을 못 볼 수 있다; ㉯ 지금 18,000(`docs/00_VISION.md:103`) — 잰 것 없이. "
           "닿는 것: 6,000을 박은 테스트와 `docs/30`(§6); 모형은 C ≈ 6,000~10,000을 내다본다(노트 §2 ⓒ — '추가 이야기와 보완'). 본문 §5.3·`[B-6b10]`. *왜 개발자인가*: 판의 길이와 공정성의 띠는 놀이의 느낌이고 비전의 수다(§0 부딪는 곳 8).")
NEW[2] = ("2. **[개발자] 누구의 절멸인가** — 권장: **그 백성 자신의 절멸** — 방주는 그 위기의 답(대응 프로젝트)이고 업적은 방주의 완성 기록과 조약 하나를 읽는다; '이주'는 방주에 실려 위기를 건넌 것으로 말하고 상태는 옮기지 않는다(`Civs` 그대로). "
          "까닭: 대응 프로젝트의 문장이 방주를 Hazard의 답으로 적는다(`systems.md:91` '뒤에 방주', `crisis_and_transcendents.md:24` 'R: 방주 프로젝트'); AI는 `Choices`가 답을 먼저 짓는 길로 저절로 답한다; 백성을 가로지르는 프로젝트도 새 저장 필드도 없다; 뒤의 언약(`crisis_and_transcendents.md:255` '다시는')이 그 위기를 지나온 백성에게 맞는다. "
          "대안: 조약 짝의 절멸을 구한다(절멸·방주 관점의 권장) — 백성을 가로지르는 창·새 발생 조건·앎의 격차를 넘는 `{OtherCiv}`의 이름이 들고, '위기 종족 구출'은 받는 길이 더하는 것이다(`pillars.md:106` — Phase 6+). 본문 §2.2·§2.3. *왜 개발자인가*: 설계 표 한 칸(`pillars.md:105`)의 뜻을 정한다.")
NEW[14] = ("14. **[개발자] 결박하는 백성 하나의 경주와 그 잠김** — 권장: 받아들이고 잰다 — VS의 타락이 하나라(`FallenTranscendentCount` 1) 처음 묶은 백성만 그 판에 다섯째 칸에 설 수 있고, 잠김 둘(묶은 뒤 다섯째 칸 전에 그릇이 떠나면 그 백성은 AI로만 올라 다른 질서; 묶은 성계를 잃으면 아무도)은 §2.5, `race:` 줄이 센다(§5.1). "
           "까닭: '묶은 백성이 다섯째 칸에 선다'는 읽히는 결승선이고, AI 쪽의 손잡이는 결정 17이다. "
           "대안: ㉮ 나눈 봉인(`SinceTick` 뒤의 어느 봉인이든) — 결정 12의 ㉱와 같다; ㉯ 성계 손실에 결정 12의 ㉯; ㉰ 다스리는 백성만 묶는다(AI 관점의 안) — AI의 다섯째 칸이 막혀 다른 질서가 서지 않는다. 본문 §2.5·§5.3. *왜 개발자인가*: 한 판의 승부(누가 끝을 가져가는가)를 정하는 놀이의 느낌이다(§0 부딪는 곳 10).")
NEW[18] = ("18. **넷째 칸이 그저 주는 것**(권장 — 모두 받고 잰다; 목록은 §2.6): '원천이 위에 있을 때만'의 거름을 바꾸는 일은 세계 코드라 제 헤더 줄이 든다; 결박은 침묵 둘레의 첫째~셋째 칸 백성에게 주는 것이다(넷째 칸의 묶은 이는 이미 소모를 받지 않는다). "
           "대안: 거름을 고친다 — B4 소모 놀이의 규칙을 다시 쓴다. 본문 §2.6·§5.")
NEW[23] = ("23. **지킴의 지연 상한(#132)**(권장): 새 노브 `FleetDelayCountdownShare` 0.5를 순수 함수 하나 `CosmicConflict::GuardDelayCapOf`에 두고 `FleetDelayOf`와 `TheodicyProblems`가 함께 읽는다 — 각성만 75로 내려가고 W·550틱의 버팀·판의 경우는 그대로(§2.10). 영향권 안 소모의 몫은 '생기지 않는다'로 닫고(넷째 칸 이상에서 없고 AI는 지키지 않는다) B7로. "
           "대안: `FleetDelayCap` 값만 낮춘다 — 주장·초신성의 지킴도 함께 줄어든다. 본문 §2.10·`[B-6b8b]`.")

NEW[10] = ("10. **법칙을 지킨 틱의 상태**(권장): 새 저장 필드 `FCivilizationState.LawsHeldTicks`(int32 = 0), DT_Rules 노브 `GalaxyLawsToHold`(3), 조건 `CivLawsHeldTicksAtLeast`(Key = N틱); 경제 단계 끝에서 넷째 칸 이상이고 `HomeGalaxy` 행을 노브 이상 지니면 +1, 아니면 0; 생애에 맞춘 읽기는 새 필드 없이 `min(LawsHeldTicks, Tick − SinceTick) ≥ N`(§2.4). "
           "까닭: int 하나, 셈의 선례(`FallenSphereTicks`), `ActiveEdicts`의 세 쓰는 곳과 맞출 일이 없고 옛 세이브에 넷째 칸 백성이 없다. "
           "대안: 칙령마다 제정 틱 배열 — `Enact`·`Revoke`·`NewGalaxy`가 맞춰 지녀야 하고(나란한 배열을 피하는 주석 — `Core/GalaxyState.h:642`) 옛 세이브의 칙령에 틱이 없다. 본문 §2.1·§2.4.")
NEW[20] = ("20. **[개발자] 첫째 칸 방랑자의 증언(#141)** — 권장: `enc_witness`를 첫째 칸 방랑자에게 — 주장이 선 성계에서 `PlayerTierAtLeast` 없이; [증언] 정신 3이면 그 성계 주인에게 경외 +0.1, 물러섬('외면한다' — `docs/70:139`의 규칙 1)은 −0.1; `ResolveHazard`·`DelayHazard`를 싣지 않고(신정론의 셈 그대로) 새 스키마 규칙이 그것을 지킨다; 셋 위의 주장자를 이름하지 않는 징조의 목소리(§2.8). 대면·정신의 닻은 방랑자의 칸이 오르는 블록으로(#141을 가른다 — 새 #164). "
           "까닭: A3 아래 개인 칸은 늘 첫째라 설계의 T2+·T3+ 카드는 죽은 글이다. "
           "대안: ㉮ 설계의 T2+ 그대로 — B6b에서 서지 않는다; ㉯ 물러섬을 효과 없는 '물러난다'로(B1의 꼴) — 설계의 −0.1을 버린다. "
           "닿는 것: `pillars.md:76`·`crisis_and_transcendents.md:27`의 T2+, B1 지킴(결정 36). 본문 §2.8. *왜 개발자인가*: 설계 표의 칸 문을 버리고 승인된 #141을 가른다(§0 부딪는 곳 3).")
NEW[26] = ("26. **다섯째 칸의 연출 — 금빛 없이**(권장): 장막·흰 제목·'칸 5'·서사·멈춤은 남기고 빛무리·금빛 제목·종소리는 뺀다; 순수 UI 도우미 하나를 미리보기·테스트·게임이 같이 읽고, 피드의 다섯째 줄은 Severity 2 그대로(§4). "
           "까닭: '자기 높임은 타락의 원형이고 주어지는 높임이 참 상승이다'(`pillars.md:91`), T5의 영적 사건은 수동태 전용(`docs/70:79`), 메시아 신호 11이 금빛 빛무리·종소리를 '칸이 오르는 그림'으로 이름한다(`theology_and_bestiary_notes.md:245`). "
           "대안: 연출 없음(신호 11이 부르심에 준 처방 '오버레이 없이'의 꼴) — 쪽이 서기 전 두 칸의 끝이 같아 보이고 문장의 '연출'이 사라진다. 본문 §4.")
NEW[32] = ("32. **[개발자] 결박의 글과 고리**(#163의 봉인 몫과 #98 ②) — 권장: 백성의 일은 3인칭의 사실로, 결박 자체는 행위자 없는 수동 '…|hpp(이,가) 묶였다'(`Sight.Bound`와 같은 결); \"{Civ}이 봉인했다\"·승리·조롱(`docs/70:185`의 ⑨ — 결박이지 파멸이 아니다)·타락의 목소리 줄이 없다(`VoiceTemplates`는 B8에 남는다); 고리는 회색, 끊기지 않은 Severity 0 그대로(#98 ②를 닫는다 — 까닭은 §4); 헌정명 '결박'은 `DT_Dedications`(Phase 4)를 기다린다. "
           "까닭: 열쇠는 둘 다 한 손에 있다(`pillars.md:89`) — 백성이 한 것은 프로젝트를 지은 일이고, 묶임은 행위자 없이 말한다. "
           "대안: 능동 '봉인했다' — 피조물에게 열쇠를 준다. 본문 §2.5·§3.5·§4. *왜 개발자인가*: 거룩한 질서의 자리(규칙 11)와 #163의 한 몫을 Phase 6+ 전에 닫는다.")

lines = h7.split('\n')
for i, ln in enumerate(lines):
    m = re.match(r'^(\d+)\. ', ln)
    if m and int(m.group(1)) in NEW and '**' in ln[:8]:
        lines[i] = NEW.pop(int(m.group(1)))
if NEW:
    raise SystemExit(f'decisions not replaced: {sorted(NEW)}')
h7 = '\n'.join(lines)

h7 = rep(h7, "(권장): 조건 `[MegaprojectComplete project_ark, CivHasTreaty 1]`", "(권장): `feat_passage_by_ark`, 조건 `[MegaprojectComplete project_ark, CivHasTreaty 1]`", name='h7')
h7 = rep(h7, "(`pillars.md:93` — 추첨 없음), 이름은 넣을 때 마지막(규칙 7);", "(`pillars.md:93` — 추첨 없음);", name='h7')
h7 = rep(h7, "15. **넷째→다섯째 칸 업적 행**(권장): 조건 `[CivLawsHeldTicksAtLeast N, MegaprojectComplete project_binding]`; 그릇의 조건 없음(A3 — `schema.json:818`의 'why'를 고쳐 적는다); `AscensionCost`·`GrantedPowers`는 빈다; 이름은 넣을 때 마지막.",
         "15. **넷째→다섯째 칸 업적 행**(권장): `feat_laws_and_binding`, 조건 `[CivLawsHeldTicksAtLeast N, MegaprojectComplete project_binding]`(N 시작값 1,200 — 노트 §2 ⓒ·ⓓ); 그릇의 조건 없음(A3 — `schema.json:818`의 'why'를 고쳐 적는다); `AscensionCost`·`GrantedPowers`는 빈다.", name='h7')
h7 = rep(h7, "`CosmicAscent::FourthRungOpenOf(State, GalaxyRow)`를 `JudgePeople`이 플레이어가 아닌 T3→T4에서만 묻는다; 추첨·필드 없음.",
         "`CosmicAscent::FourthRungOpenOf(State, Tables, GalaxyRow)`를 `JudgePeople`이 플레이어가 아닌 T3→T4에서만 묻는다; 추첨·필드 없음. 모형으로는 제동이 누가 먼저 묶는지를 바꾸지 않고 넷째 칸에 선 AI의 수만 바꾼다(노트 §2 ⓒ) — 띠의 손잡이는 결정 17이다.", name='h7')
h7 = rep(h7, "띠의 주 손잡이다.", "띠의 주 손잡이다(시작값 0.375 — 노트 §2 ⓒ).", name='h7')
h7 = rep(h7, "(C6).", "(§0 부딪는 곳 6).", name='h7')
h7 = rep(h7, "다음 칸이 다섯째일 때 `Diplomacy.SlotsNext`·`Settle.Block.RealmCap`의 말을 고친다.",
         "다음 칸이 다섯째일 때 `Diplomacy.SlotsNext`·`Settle.Block.RealmCap` 대신 `Diplomacy.SlotsAll`·`Settle.Block.RealmCapLast`(§4).", name='h7')

# closing section
i = h7.index('## 추가 이야기와 보완')
old_tail = h7[i:]
def bullet(start):
    for ln in old_tail.split('\n'):
        if ln.startswith(start):
            return ln
    raise SystemExit('bullet not found: ' + start)
tail = ['## 추가 이야기와 보완', '',
    "- **결정 13·14는 묶음 B 뒤에 함께 다시 본다**: 결박한 성계를 잃어 잠기는 판이 잦으면 가장 작은 고침은 `SealedByCivID`(결정 12 ㉯)이고, 설계가 이미 내다본 '주인 필드'다(새 열린 물음 #165의 안 — 노트 §0.2).",
    bullet('- **넷째 칸 백성을 떠난 그릇의 돌아감**'),
    bullet('- **벽시계의 새 잣대**'),
    "- **판의 길이가 비전보다 짧을 공산**: 모형은 첫 다섯째 칸의 끝을 스크립트 통치자로 약 4,000~7,500틱에 두고 시계 규칙이 C ≈ 6,000~10,000(시작값에서 8,000)을 낼 것으로 본다(노트 §2 ⓒ·ⓓ) — 비전의 18,000(`docs/00_VISION.md:103`)과 '초월 ~90분'보다 짧다. 묶음 B가 그렇게 보이면 노트 §0.2의 후보 행을 열고, 값은 규칙대로 둔다.",
    "- **건설 단추가 서는 칸**(§4 '묶음'): 지금은 칸이 모자란 건설 행도 회색 단추로 서서 넷째 칸의 결박이 첫째 칸의 패널에 선다 — §4는 '그 칸이 다음 칸일 때부터'로 바꾼다. 결정 밖의 표현 고침이라 `[B-6b4]`의 화면 리뷰가 다시 본다.",
    bullet('- **넷째 칸의 [저녁 안부를 내린다]**'),
    bullet('- **정책은 끝 뒤에도 답하고 짓는다**'),
    '']
h7 = h7[:i] + '\n'.join(tail)

parts = [h0, h2a, h2b, h2c, h34, h5, h7]
parts = [p.strip('\n') for p in parts]
# §6 and §8 live in h5; §7 lives in h7 -> reorder: h5 holds 5,6,8,9 ; h7 holds 7,10,11
# split h5 at '## 8.' and h7 at '## 10.'
h5p = parts[5]; h7p = parts[6]
k = h5p.index('## 8. ')
h5_56, h5_89 = h5p[:k].rstrip('\n'), h5p[k:]
k = h7p.index('## 10. ')
h7_7, h7_1011 = h7p[:k].rstrip('\n'), h7p[k:]
body = '\n\n'.join(parts[:5] + [h5_56, h7_7, h5_89.rstrip('\n'), h7_1011])
body = body.replace('`crisis:', '`crisis_and_transcendents.md:')

# ---------------------------------------------------------------- post: shrink and de-duplicate
def take_between(text, start, end, name=''):
    i = text.find(start)
    j = text.find(end, i)
    if i < 0 or j < 0:
        raise SystemExit(f'[{name}] markers not found: {start[:40]!r} / {end[:40]!r}')
    return text[:i], text[i:j + len(end)], text[j + len(end):]

CONFLICTS = """- **답과 승인된 결정이 부딪는 곳**(열셋 — 결정들은 이 번호를 '§0 부딪는 곳 N'으로 부른다; 풀이는 각 결정):
  1. A2 ↔ B6a 결정 1(결박과 '그 주인 필드'를 Phase 6+로 — B6a 헤더 `:34`·`:387`, `docs/50:231`, #98 ②) → 결박만 B6b로, 주인 필드 없이; B6a 헤더는 고치지 않는다. 결정 12·39.
  2. A2 ↔ AI의 규칙(조약도 칙령도 하지 않는다 — `crisis_and_transcendents.md:134`, `systems.md:162`; B7 칸의 몫) → 행이 부를 때만 하는 좁은 걸음 둘. 결정 1·11.
  3. A3 ↔ #141의 주인('#1의 칸 공명이 들면 B6b' — `crisis_and_transcendents.md:115`) → 첫째 칸 방랑자가 가질 것만 짓고 #141을 가른다. 결정 20.
  4. A3 ↔ 낡은 글(`schema.json:818`, #1의 기본값, `pillars.md:108`의 공명 가리킴) → #1을 닫고 글만 고친다. 결정 15·39.
  5. 절멸 ↔ B5 결정 6-가('다른 죽음의 출처는 새 결정' — `Tests/CosmicDataTests.cpp:2425`) → 코드의 죽음, 데이터 핀 그대로, 새 테스트. 결정 5.
  6. A2 ↔ #138의 기본값(피난을 영압 카드의 선택으로) → 문서의 대안을 받고 닫는다. 결정 21.
  7. A2 ↔ 로드맵의 Phase 4 목록('절멸·교단' — `docs/50:197-198`) → 절멸의 행·훅·방주만 B6b로. 결정 3, §7.
  8. A4 ↔ 6,000을 못 박은 테스트와 B6a 결정 21(콘솔 없는 엔딩) → 시계 커밋이 테스트를 고치고, 규칙이 시계를 사람의 빠르기에 가까운 칸에 묶는다. 결정 35.
  9. A1 ↔ B6a 결정 1의 크기 경고(150KB) → 표와 인용은 노트로. 결정 37, §8.
  10. '같은 행' + VS의 타락 하나 ↔ #73(남의 완성은 이 백성의 것이 아니다 — `docs/21:113`) → 결박의 완성 기록으로 그 일을 한 백성만 센다. 결정 12·14.
  11. 칙령 범위의 넓힘 ↔ 신정론 불변식(칙령 행마다 카운트다운 덤을 한 번 센다) → 법칙은 주장에 닿지 않고 성계마다 행마다 한 번. 결정 9.
  12. #156 ↔ 그 주의 셋(`OPEN_QUESTIONS.md:120`) → 이벤트 행, 다스리는 백성만, `EpithetRow`를 쓰지 않는다. 결정 22.
  13. 'Tier 4에서는 직접 대면 조우가 열린다'(`crisis_and_transcendents.md:99`) → 주인 블록의 표지. 결정 20, §7.
"""
a, _, c = take_between(body, "- **답과 승인된 결정이 부딪는 곳**", "결정 20, §7.\n", name='conf')
body = a + CONFLICTS + c

KEEP = """- **지키는 것**(`CLAUDE.md` 규칙 11·12 — 글의 규칙은 §3.5): 거룩한 질서는 엔티티·화자·수치·엔딩 조건이 아니다 — 결박은 행위자 없는 수동('…|hpp(이,가) 묶였다' — `Tools/data/ST_CA.csv:52`의 `Sight.Bound`과 같은 꼴)으로만 말하고 '누가 봉인했다'·승리·조롱이 없으며, 결박 프로젝트를 완성한 백성의 일은 3인칭의 사실로만 말한다(결정 32). 다섯째 칸의 오름은 '얻은 오름'의 그림(금빛 빛무리·종소리 — 메시아 신호 11)을 쓰지 않고 칭호·'신이 되었다'·'영겁'·'왕 노릇'이 글에 오지 않는다(결정 26·33 — T5의 이름은 #163). 절멸의 만료로 죽는 통치자는 백성을 위해 '남은' 희생으로 읽히지 않는다(결정 5). 사람의 수는 없다 — 절멸은 '백성이 사라졌다'는 사실이다. 영혼·구원·멸망을 세는 칸이 없다 — 새 저장 필드는 틱의 수 둘뿐. 칸은 다섯 그대로, 초월자는 판에 교리마다 하나(VS는 `trans_silence` 하나 — 그래서 결박은 판에 한 번, 결정 14).
"""
a, _, c = take_between(body, "- **지키는 것**: 거룩한 질서는", "결박은 판에 한 번, 결정 14).\n", name='keep')
body = a + KEEP + c

S11 = """### 1.1 지금 코드 (줄마다의 근거는 노트 §0)

- **판정과 행**(노트 §0.5′): AI 단계의 끝에 틱마다 한 번 — 다스리는 백성을 먼저(그릇이 몸을 입은 통치자일 때, `BornTick`부터), 그다음 다스려지지 않는 백성을 `Civs` 순서로 본다; 정착된 성계 하나 위에서 조건이 모두 서는 첫 행이 이기고, 뽑지 않는다. 판정·AI의 건설·다음 칸 툴팁이 거름 하나 `IsJudgedRulerRowAt`(`Data/CosmicDefinitionRows.h:298`)을 읽는다. `DT_Feats`는 통치자 행 셋(T1→T2 하나·T2→T3 둘) — 셋째 칸에서 오르는 행이 없고, 방랑자 행은 스키마가 막는다.
- **끝**(노트 §0.5′): `EndingOf`가 집단 승천 → 그릇 → 다른 질서(`Civs`의 첫 다섯째 칸) → 침묵 → 시계 차례로 묻고 기록은 열째 단계 뒤 한 번 적힌다. 판 안에 닿는 것은 침묵과 시계 6,000뿐; 섭정 아래나 공백기의 다섯째 칸은 다른 질서다(#121); 그릇의 끝은 A3로 닿지 않는다.
- **제동과 AI 단계**(노트 §0.5′·§0.6′): `AIFourthRungCap` 2(고향 행만)를 읽는 코드가 없다(셈은 `docs/21:1574-1580` — 다스리는 백성도 센다). AI 단계는 (a) 건설 고르기(추첨 없음) → 정착 한 수 → 칙령 철회 → 외교, (b) 판정. AI는 조약을 제안하지도 끊지도 않고 칙령을 지니지 않는다. `AIAscensionSpeedMultiplier` 1.0은 모든 칸에 같다.
- **건설**(노트 §0.1′): 다섯 행, 모두 둘째 칸 이하 — 방주도 결박도 없다. `Choices`는 위기의 답을 먼저, 그다음 수도의 빈 건설 칸에 판정할 행이 부르는 첫 건설을 시작한다. 완성 기록이 백성과 틱을 적어 `MegaprojectComplete`는 그 백성의 일만 센다(#73).
- **위기**(노트 §0.0): `DT_Hazards` 다섯. 발생은 정착된 성계마다 한 수를 먼저 뽑고 조건이 서는 `Draw` 행의 몫을 행 이름 순서로 더한다 — 새 `Draw` 행은 추첨 수를 바꾸지 않는다. 절멸은 태그뿐이고, 만료의 마지막 성계 문(남은 자의 은혜)은 초신성에만 선다.
- **백성을 놓음과 통치자의 죽음**(노트 §0.0): 한 백성의 성계를 모두 놓게 하는 함수가 없다(`Transfer`는 마지막 정착된 성계를 거절한다); 건넘 단계가 성계 없는 백성을 지운다. `VesselAtRiskOf`는 성계 없는 주제의 죽음이 그 백성의 통치자에게 닿게 이미 적혀 있고('its extinction' — `Sim/CosmicHazards.h:212-220`), 초신성의 붕괴가 같은 꼴로 `DeposeRuler`를 건다. `Data.Integrity`는 데이터의 죽음이 AI 각성의 만료 하나임을 지킨다(`Tests/CosmicDataTests.cpp:2418-2430`).
- **조약과 칙령**(노트 §0.2′·§0.3′): `FCosmicTie`는 짝에 대칭이고 맺은 틱이 없으며 조약을 읽는 조건이 없다. 세계의 문 `TreatyBlockOf`는 `NotRuler`를 내지 않아(명령의 껍질 — `Player/PlayerRoleSubsystem.cpp:970`) AI의 걸음이 같은 문을 지날 수 있다. `ActiveEdicts`는 틱 없는 이름 배열이고, 칸은 많아야 셋이라 법칙 셋이면 칸이 다 찬다; 범위 칸이 없고 행 일곱은 첫째·둘째 칸이다.
- **결박**(노트 §0.4′): `bSealed`·`SealedTick`을 쓰는 코드도 효과도 없다. 묶인 초월자를 멈춘 것으로 읽는 곳은 이미 여럿(압력·소모·주장자·알아챔·정착 거절·계승 거름·지도 고리 …)이라 한 번 쓰면 영향권이 얼고 새 주장이 그친다. `TranscendentSealed`는 누가 묶었는지 적지 않는다.
- **넷째 칸이 거저 얻는 것**(노트 §0.6′): 소모·영압 효과·함대 소모는 '그 백성보다 높은' 출처만 걸어 넷째 칸 `trans_silence`가 넷째 칸 백성에게 닿지 않는다; 영역 상한 7(정착 추첨이 돌아온다)·수입 ×3·공성이 다시 열린다; 주장은 주인의 칸을 보지 않는다.
- **격차와 새는 이름**(노트 §0.7′): '세 칸 위는 보이지 않는다'가 세 곳에 따로 있다 — `Tell`·`NoteMet`·`IsShownPeople`. 방랑자와 공백기의 보는 칸은 개인 칸(늘 첫째)이라 넷째 칸 이상 백성은 모두 가려진다. 공성 줄과 조약 깨짐 줄은 `NoteMet`의 거절을 버리고 상대의 이름을 넣는다. 즉위의 문은 만남만 묻는다(#122).
- **#156과 #141의 재료**: `EpithetRow`는 쓰는 곳이 없다. 따를 꼴은 카운터 `FallenSphereTicks`·한 칸 조건 `CivFallenSphereTicksReached`·상시 행 `event_book_closed`. 선택자 `Ruled`는 정해졌으나(`docs/21:619`) 코드에 없다. `enc_witness`는 없고 방랑자 카드의 격차는 '한 칸 위까지'다; 신정론 불변식은 `DelayHazard` 50짜리 둘째 카드에 깨진다(§2.8).
- **표현**(노트 §0.8′): 오름의 연출은 칸마다 같고(금빛 빛무리·금빛 제목·종소리 …) 다스리는 백성의 오름에만 선다 — 다른 질서의 다섯째 칸에는 연출이 없다. 낡은 약속: `Chronicle.RuleRung`(셋째 칸에 멈춘다)·`Chronicle.Until`·`Epitaph.Peak`, 먼 줄의 변형 ①·③, 툴팁 주석 'the game has none from the third'.
- **저장·커맨드렛·규칙 파일**(노트 §0.10′): `CurrentSaveVersion` 1, 델타 저장 — 새 필드는 옛 세이브에서 초기값으로 읽힌다. 커맨드렛에는 넷째·다섯째 칸·제동·절멸·조약·법칙·결박의 칸이 없고 `-Rules`는 `DT_Rules`만 닿는다. `CLAUDE.md`는 16,224 B — `Tools/check_docs.py:42`의 16 × 1024까지 160 B 남았다(결정 38).

"""
a, _, c = take_between(body, "### 1.1 지금 코드 (줄마다의 근거는 노트 §0)", "160 B 남았다(결정 38).\n\n", name='s11')
body = a + S11 + c

RACE = """- **새 줄 `CosmicBalance race:`** — 러너가 글자로 견주는 세 줄(`seed`·`ruler:`·`wanderer:` — `Tools/run_checks.ps1:87-98`) 밖이다(B6a `ending:`의 꼴 — `Tests/CosmicBalanceCommandlet.cpp:2942`). 셈은 규칙을 베끼지 않고 게임 코드의 창구를 읽는다 — 제동은 `CosmicAscent::FourthRungOpenOf`, 업적 조건은 거르개 `IsJudgedRulerRowAt`, 보임은 `CosmicMapLook::KnownViewOf`. 칸은 노트 §2.1: 첫 AI와 다스리는 백성(어느 재위든)이 칸마다 선 틱, 끝의 종류마다 술어가 처음 선 틱(**기록과 따로** — 기록은 먼저 온 끝 하나라 시계를 바꿔 다시 세려면 따로 있어야 한다), 넷째 칸 이상의 수와 제동의 거절, 절멸·방주·AI 조약·법칙, 결박(묶은 자의 종류와 잠김 — 결정 14), **600틱 창마다의 결정 수를 판 끝까지**(지금 `ending:`의 계절 결정은 기록이 없으면 −1 — 시계를 판 밖에서 세는 입력), #162의 보임, #156·#141의 줄."""
a, _, c = take_between(body, "- **새 줄 `CosmicBalance race:`**", "#141 `enc_witness`가 온 수와 답.", name='race')
body = a + RACE + c

a, moved, c = take_between(body, "**데이터를 켜면 뜻이 바뀌는 테스트**:", "(`Tools/run_checks.ps1:45`).", name='tests')
body = a + ("**데이터를 켜면 뜻이 바뀌는 테스트와 시계에 박힌 테스트**: 표는 노트 §1.3 — 데이터 커밋(`[B-6b6]`~`[B-6b8]`)마다 그 표의 테스트를 돌려 보고 고치며, 시계 커밋 `[B-6b10]`은 `Sim.EndingOf`·`UI.RunEnd`를 C로 옮기고 `Calling.LeapDeterminism`에는 시계 6,000을 제 값으로 두며 `Sim.SaveOldEndFlows`(6,000의 시계 기록을 지닌 세이브는 C 아래서도 끝난 채 연다 — A4)를 더한다.") + c

DOCS = """**문서 커밋**(`[B-6b12]` — 끝에 재감사 워크플로; 줄마다의 자리와 고칠 말은 노트 §1.2): `docs/50`의 B 행 B6b 칸(`:12`)·Phase 4 목록(`:197-198` — 절멸은 B6b로)·§12.1의 수량(결정 40); `WORKPLAN.md:56`·`:61`; 설계의 나머지(`crisis_and_transcendents.md:99`의 주인 블록 표지·`:115`(#141을 가름)·`:27`(`enc_witness`의 T2+ — 결정 20), `pillars.md:76`·`:108`(공명의 가리킴 — A3)); `docs/15_GLOSSARY.md`; `docs/70`(결박의 글 ⑨·시대 줄·다섯째 칸의 글); `docs/40:480`('< 15%'의 셈); `OPEN_QUESTIONS`(노트 §0.2)·`DECISIONS_LOG`(결정 39)·`docs/CHANGELOG.md`·`STATUS.md`; `CLAUDE.md` 규칙 15의 'A0~B6a' → 'A0~B6b'(봉인을 예정에서 뺀다); 코드 주석의 나머지(헤더 파일이면 에디터를 닫고 전체 빌드)."""
a, _, c = take_between(body, "**문서 커밋**(`[B-6b12]`", "줄마다의 자리와 고칠 말은 노트 §1.2.", name='docs')
body = a + DOCS + c

body = rep(body, bullet('- **정책은 끝 뒤에도 답하고 짓는다**'),
    "- **정책은 끝 뒤에도 답하고 짓는다**: 통치자 정책의 카드 답과 사업은 기록 뒤에도 이어 가고 중점·칙령·외교만 선다(노트 §1의 어긋남 5) — 시계 6,000의 20,000틱 통치자 기준값에서 다스리는 백성은 끝 뒤에 방주와 결박을 짓되 새 조약·법칙은 얻지 못한다. `[B-6b6]`·`[B-6b7]`이 옮길 줄의 까닭으로 `docs/40` 커맨드렛 절에 미리 적는다(묶음은 시계를 끄므로 받지 않는다).", name='tail')
body = rep(body, bullet('- **넷째 칸의 [저녁 안부를 내린다]**'),
    "- **넷째 칸의 [저녁 안부를 내린다]**: 영압 카드는 다스리는 성계에 주장이 서면 칸과 무관하게 오는데, 그 칙령 `edict_against_silence`가 사는 것은 소모 ×0.45뿐이고(수입 ×0.9를 치른다) 넷째 칸 백성은 소모를 받지 않는다; 게다가 법칙 셋이 칙령 칸을 모두 쓴다. 그 선택을 선택의 문(`CosmicPetitions::ChoiceGateOf` — 정책의 답도 읽는 규칙 13의 창구)의 새 사유로 회색으로 할지는 `[B-6b2]`에서 리드가 정하고, 넣으면 비트 그대로를 확인한다.", name='tail')

body = cut_between(body, " 글은 애도로 끝나고(`docs/70:193`) 백성과 그릇이 한 끝을 나눌 뿐", "용광로 그림 없음.", " 글의 규칙은 §3.5의 3.", name='p')
body = rep(body, "- '방주'는 구출의 낱말이고(`systems.md:128`) 이 행은 그 백성 자신의 구출이다 — '새 땅으로 옮겼다'가 아니다.\n", "", name='p')
body = cut_between(body, "- **글**: 백성의 일만 — '견뎠기에'의 까닭", "남기면 수는 방랑자 패널에만.",
    "- **글**: §3.5의 6. 경외 +0.2가 견딤을 그릇의 몫으로 돌리는가(메시아 신호 9·10 — `theology_and_bestiary_notes.md:243-244`)는 결정 22가 묻는다.", name='p')
body = cut_between(body, "- **목소리**: `WithinWandererGap`(≤ 1 — `Events/CosmicPetitions.cpp:176-180`) 그대로라", "'방주' 없음.",
    "- **목소리**: `WithinWandererGap`(≤ 1 — `Events/CosmicPetitions.cpp:176-180`) 그대로라 첫째·둘째 칸 백성의 땅에서만 온다; 주장자(넷째)는 세 칸 위라 부르지 않는다(글은 §3.5의 7).", name='p')
body = cut_between(body, "- **먼 줄(#162)**: `log_far_ascent`(", "글만이라 비트 동일.",
    "- **먼 줄(#162)**: `log_far_ascent`의 세 변형을 어느 칸에도 참인 글로(§4의 #162, 결정 29) — 글만이라 비트 동일.", name='p')
body = rep(body, " 0.5면 상한 각성 75·역병 100·전조 100·주장 100; W ≤ 1.2445와 550틱 그대로.", " 상한과 W는 §2.10.", name='p')
body = cut_between(body, "- **`AIUpperRungProgressMultiplier`** float 1.0(결정 17)", "#161 띠의 손잡이.",
    "- **`AIUpperRungProgressMultiplier`** float 1.0(결정 17 — 무엇에 곱하는지는 §2.6): 범위는 `AIAscensionSpeedMultiplier`의 꼴(`:137`·`:240`), 값은 이진수로 떨어지게(0.5·0.75·0.375). 읽는 곳 `CosmicMegaprojects::Advance` 하나 — #161 띠의 손잡이.", name='p')
body = rep(body, "3. **절멸의 줄**(위기 행 세 칸·기록 줄·죽음의 줄): 사람의 수와 규모 등급 없음,", "3. **절멸의 줄**(위기 행 세 칸·기록 줄·죽음의 줄): 사람의 수와 규모 등급·구출의 낱말·용광로의 그림 없음,", name='p')

body = cut_between(body, "- **누구의 절멸인가**(결정 2 [개발자]): 그 백성 자신의 것", "(`pillars.md:106`, Phase 6+).",
    "- **누구의 절멸인가**(결정 2 [개발자]): 그 백성 자신의 것 — '이주'는 상태가 옮기지 않고(`Civs` 그대로) 글이 '방주로 위기를 건넜다'로 말한다.", name='p2')
body = cut_between(body, "- **아공간 균열은 들이지 않는다**(결정 3 [개발자])", "(#90은 Phase 4).",
    "- **아공간 침식은 들이지 않는다**(결정 3 [개발자] — #90은 Phase 4 위기 헤더에 그대로).", name='p2')
body = cut_between(body, "- **까닭**: '청지기'·'법 수호'(`pillars.md:79`·`:91`)에 맞고", "법은 답이 아니다.\n", "", name='p2')
body = rep(body, "까닭: '청지기'·'법 수호'에 맞고 '법칙 왜곡'의 강제가 아니다", "까닭: '청지기'·'법 수호'(`pillars.md:79`·`:91`)에 맞고 '법칙 왜곡'의 강제가 아니다", name='p2')
body = cut_between(body, "- **까닭**: int 하나, 이미 있는 셈의 꼴", "0이 맞다.\n", "", name='p2')
body = rep(body, "- **해시**: 새 필드 둘은 두 해시(`[B-6b1]`이 다섯 줄과 해시 판 둘을 옮기고 CSV의 결정적 칸은 `[B-6a7]`과 같다 — `[B-3a]`·`[B-6a2]`의 꼴);",
    "- **해시**: 새 필드 둘은 두 해시(§2.1·§5.4);", name='p2')

FIX = [
    ("대안: 큰 진척 — 늦춰진 AI가 지워진다. 본문 §2.3·§3.", "대안: 큰 진척 — 늦춰진 AI가 지워진다. 본문 §2.3·§3.1."),
    ("대안: 특성을 지우는 값 — 특성 행은 Phase 4. 본문 §2.3·§3.", "대안: 특성을 지우는 값 — 특성 행은 Phase 4. 본문 §2.3·§3.1."),
    ("대안: 값을 단다 — 판이 그 틱에 끝나 보이지 않는다. 본문 §2.5·§3.", "대안: 값을 단다 — 판이 그 틱에 끝나 보이지 않는다. 본문 §2.5·§3.1."),
    ("셋째 칸에서 멈춘다는 거짓 약속이 남는다. 본문 §4·§3.", "셋째 칸에서 멈춘다는 거짓 약속이 남는다. 본문 §4·§3.2."),
    ("저장된 글을 다시 짓는 일이다. 본문 §2.9·§3.", "저장된 글을 다시 짓는 일이다. 본문 §4·§2.9."),
    ("다음 행이 같은 낱말을 다시 들인다. 본문 §3.", "다음 행이 같은 낱말을 다시 들인다. 본문 §3.3·§3.5."),
    ("`RequiredTier` T4 행 셋 이상(§3)과", "`RequiredTier` T4 행 셋 이상(§3.1)과"),
    ("- **스키마**(§3): `Home", "- **스키마**(§3.3의 4): `Home"),
    ("새 업적 행의 대가는 뽑지 않는다(§3).", "새 업적 행의 대가는 뽑지 않는다(결정 8)."),
    ("처음 이름이 마지막이다(§3, 규칙 7).", "처음 이름이 마지막이다(§3.1, 규칙 7)."),
    ("다음 절멸을 기다린다 — `race:` 줄이 센다(§5).", "다음 절멸을 기다린다 — `race:` 줄이 센다(§5.1)."),
    ("그 횟수는 `race:` 줄이 센다(§5).", "그 횟수는 `race:` 줄이 센다(§5.1)."),
    ("`race:` 줄이 묶은 자의 종류와 세 잠김을 센다(§5);", "`race:` 줄이 묶은 자의 종류와 잠김을 센다(§5.1);"),
    ("B1 지킴의 기제(§5).", "B1 지킴의 기제(§5.3)."),
    ("3,000 줄과 해시 판 둘은 그대로(§5).", "3,000 줄과 해시 판 둘은 그대로(§5.4)."),
    ("(`Tools/test_data_schema.py` — §3).", "(`Tools/test_data_schema.py` — §3.3)."),
    ("§9.1(`:184`~)에 저장 필드 둘의 한 단락, §9.2(`:199`~)의", "`docs/20` §9.1(`:184`~)에 저장 필드 둘의 한 단락, 그 §9.2(`:199`~)의"),
]
FIX += [
    ("남을 위해 진 흉(메시아 신호 2)", "남을 위한 희생(메시아 신호 2)"),
    ("절멸 행·`project_ark`·업적 행·기록 줄, 결정 5의 테스트", "절멸 행·`project_ark`·업적 행, 결정 5의 테스트"),
    ("`KillsTheRulerOf`·스키마 규칙 |", "`KillsTheRulerOf`·로그 행 셋(§3.1)·스키마 규칙 |"),
    ("Sim/CosmicHazards.h:210-217", "Sim/CosmicHazards.h:212-220"),
]
for o, nw in FIX:
    body = rep(body, o, nw, name='fix')

a, MOVED_ST, c = take_between(body, "| 키 | B6b(초안 — 글 감사가 정한다) |", "'지금의 칸이 다스릴 수 있는 성계 수에 닿았다.' |", name='st')
body = a + ("고치는 키 셋 — `Chronicle.RuleRung`(`:146`, 지금 읽지 않음 — 두 끝에 같은 다섯째 칸의 규칙, B6a 결정 6-마)·`Chronicle.Until`(`:148` — '늦어도')·`Epitaph.Peak`(`:406` — 결정 30) — 과 새 키 열다섯(`Rung.Treaty`·`Rung.TreatyOf`·`Rung.Laws`·`Rung.LawsLeft`·`Rung.LawsReset`·`Rung.NoFallen`·`Start.Block.NoFallen`·`Start.Suffix.NoFallen`·`Start.BindHint`·`Panel.ExtinctionFollows`·`Panel.RulerLethal`·`Edict.Wide`·`Edict.TipWide`·`Diplomacy.TreatyRungTip`·`Settle.Block.RealmCapLast`)의 초안은 노트 §1.4의 표다. 쓰는 자리는 §4. `Start.Block.NoFallen`·`Start.Suffix.NoFallen`은 열거와 같은 `[B-6b2]`에 든다(패널의 `static_assert` 표 `UI/CosmicSystemPanelWidget.cpp:86-119`).") + c

body = body.replace("[외면한다](설계의 도주;", "[외면한다] (설계의 도주;")
body = body.replace("'일체화'](`docs/70:129`", "'일체화'] (`docs/70:129`")
body = body.replace("'기름 받은'](신호 9", "'기름 받은'] (신호 9")
body = body.rstrip('\n') + '\n'
MOVED_TESTS = moved
open(OUT, 'w', encoding='utf-8', newline='\n').write(body)

# ---------------------------------------------------------------- notes
nt = rd('notes.md')
nt = rep(nt, "| 커맨드렛 `T4PlusCount` | `Tests/CosmicBalanceCommandlet.cpp:949-967` (`:962`) | 그 타락이 셈에서 빠진다 |",
         "| 커맨드렛 — 정책 판의 영향권 닿음과 `T4PlusCount` | `Tests/CosmicBalanceCommandlet.cpp:620-628` (`:625`) · `:949-967` (`:962`) | 영향권을 보지 않는다 · 그 타락이 셈에서 빠진다 |", name='nt')
nt = rep(nt, "느린 R의 다스리는 셋째 칸은 6,000 − 1,616 ≈ **4,384**",
         "R의 다스리는 셋째 칸은 6,000 − 3,634.5 ≈ **2,366**, 느린 R은 6,000 − 1,616 ≈ **4,384**", name='nt')
nt = rep(nt, "| 136 | `:103` | 시계 C에서 읽는다 — ≥ 10/30이면 닫고, 아니면 수와 함께 B7로 | 결정 35 — 묶음 C |",
         "| 136 | `:103` | 시계 C에서 읽는다 — R 60에서 ≥ 1/3이면 닫고, 아니면 수와 함께 열어 둔다(헤더 §5.3) | 결정 35 — 묶음 C |", name='nt')
nt = rep(nt, "'Counted last, …' 단락에 둘을 더한다)", "'Counted last, …' 단락에 견딘 틱을 더한다 — 법칙 틱은 `[B-6b2]`)", name='nt')
nt = rep(nt, "- `[B-6b2]`(세계의 장치): `Data/CosmicConditionTypes.h:91-92`",
         "- `[B-6b2]`(세계의 장치): 경제 단계 끝 셈 주석에 법칙 틱(헤더 §2.1); `Data/CosmicConditionTypes.h:91-92`", name='nt')
nt = rep(nt, "자동 정지가 읽을 새 답 `KillsTheRulerOf`가 든다 — §0.0)", "패널의 경고가 읽을 새 답 `KillsTheRulerOf`가 든다 — 헤더 §2.2·§4)", name='nt')
nt = rep(nt, "비용은 정착 행처럼 셋째 칸 수입(×2.2)에 맞춰 헤더 §3이 정한다(인과율 없음 — 수입 0).",
         "비용은 `[B-6b6]`의 행이 셋째 칸 수입(×2.2)에 맞춰 정한다(인과율 없음 — 수입 0; 헤더 §3.3의 7).", name='nt')
nt = rep(nt, "(B4 헤더 `:197` —", "(B4 헤더 `:362` —", name='nt')
nt = rep(nt, "지평은 시계 C에서 자른다(헤더 §5 묶음 C).", "지평은 20,000까지 돌리고 주장마다의 줄을 C에서 밖에서 자른다(헤더 §5.2 묶음 C).", name='nt')
nt = rep(nt, "장치 커밋의 주석·문서 커밋의 목록(§1.1·§1.2 — 헤더 §7에서),",
         "장치 커밋의 주석·문서 커밋의 목록(§1.1·§1.2 — 헤더 §7에서), 데이터와 시계가 뜻을 바꾸는 테스트(§1.3 — 헤더 §6에서),", name='nt')
MOVED_TESTS = MOVED_TESTS.replace("§11에", "헤더 §11에")
sec13 = ("## 1.3 데이터와 시계가 뜻을 바꾸는 테스트 (헤더 §6에서 — 커밋마다 돌려 보고 고친다; 깨지면 시드를 바꾸기 전에 까닭부터)\n\n"
         + MOVED_TESTS.replace("**데이터를 켜면 뜻이 바뀌는 테스트**:", "**데이터를 켜면 뜻이 바뀌는 테스트**:", 1).strip('\n') + "\n\n")
sec14 = ("## 1.4 스트링 테이블의 초안 (헤더 §3.2에서 — 줄은 `Tools/data/ST_CA.csv`의 것, 새 키는 끝(`:447` 뒤)에만; `[B-6b5]`의 글 감사가 정한다)\n\n"
         + MOVED_ST.strip('\n') + "\n\n")
nt = rep(nt, "## 2. 미리 셈 (헤더 §5.2에서)", sec13 + sec14 + "## 2. 미리 셈 (헤더 §5.2에서)", name='nt')
nt = rep(nt, "데이터와 시계가 뜻을 바꾸는 테스트(§1.3 — 헤더 §6에서),", "데이터와 시계가 뜻을 바꾸는 테스트(§1.3 — 헤더 §6에서), 스트링 테이블의 초안(§1.4 — 헤더 §3.2에서),", name='nt')
nt = rep(nt, "10/30은 6,340 바로 뒤에 넘는다(시드 18 6,331, 시드 1 6,340)", "10/30에는 6,331(시드 18)에, 11/30에는 6,340(시드 1)에 닿는다", name='nt')
nt = rep(nt, "T4Civs(기록·끝·판의 최대 — '≤ 상한'의 깨질 수 있는 확인)", "T4Civs(기록·끝·판의 최대 — 적기만; 깨질 수 있는 확인 `race check`의 꼴은 헤더 §5.1 — '≤ 상한'만으로는 불변식이 아니다)", name='nt')
nt = rep(nt, "`[B-6b6]`('AI 넷째 칸 ≥ 1 ∧ T4Civs ≤ 상한')", "`[B-6b6]`('AI 넷째 칸 ≥ 1' — 상한의 확인은 헤더 §5.1의 `race check`)", name='nt')
nt = rep(nt, "| B6a 칸·결정 13 |", "| B6a 칸·B6a 결정 13 |", name='nt')
nt = nt.replace("](`:", "] (`:")
nt = nt.rstrip('\n') + '\n'
open(NOTES_OUT, 'w', encoding='utf-8', newline='\n').write(nt)
print('header bytes', len(body.encode('utf-8')), 'notes bytes', len(nt.encode('utf-8')))
