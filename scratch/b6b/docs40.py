import sys
sys.stdout.reconfigure(encoding='utf-8')
p='docs/40_DEV_GUIDE.md'
s=open(p,'rb').read().decode('utf-8')
def rep(old,new):
    global s
    assert s.count(old)==1, old
    s=s.replace(old,new)
# Feats.Evaluate
rep("넷째 칸에 둘이 선 판이라 제동이 있으면 셋째 칸 백성이 오르지 못한다)을 박았다 |",
    "넷째 칸에 둘이 선 판이라 제동이 있으면 셋째 칸 백성이 오르지 못한다)을 박았다. B6b `[B-6b2c]`: 넷째→다섯째 칸 행 `[MegaprojectComplete project_binding]`은 결박의 완성 기록을 읽어 그 일을 한 백성만 올린다 — 봉인된 타락이 있어도 기록 없는 넷째 칸 백성은 그대로다(변이 `b6b2c_binding_read_as_sealed` — 그 행을 `TranscendentSealed`로 읽으면 다른 백성도 오른다) |")
# Sim.StartBlockReasons
rep("에서 사유가 None인 곳이 Phase 2 규칙을 옮긴 `ReferenceCanStart`가 받는 곳과 같고 `CanStart`와도 같다(대조군: 판마다 받는 것과 거절하는 것이 하나 이상) |",
    "에서 사유가 None인 곳이 Phase 2 규칙을 옮긴 `ReferenceCanStart`가 받는 곳과 같고 `CanStart`와도 같다(대조군: 판마다 받는 것과 거절하는 것이 하나 이상). B6b `[B-6b2c]`: 결박 행(넷째 칸)과 게임의 타락 행을 얹은 판에서 묶일 타락이 남으면 사유 없음, 없으면 `NoFallen`(결박에만 — 다른 건설은 그대로), 칸이 모자라면 `NoFallen`보다 `RowTier`가 먼저(변이 `b6b2c_nofallen_before_rowtier`), 짓는 중인 결박도 대상이 없으면 `Running`보다 `NoFallen`이 먼저(변이 `b6b2c_nofallen_after_slot`); 판 둘(묶일 타락이 있는 판·묶인 판)과 게임의 은하의 칸 4·묶인 칸 4를 더해 판 열셋 — `ReferenceCanStart`도 결박의 거절을 함께 옮긴다 |")
# Sim.FleetDelay
rep("지연을 더한 카운트다운도 `int32`의 끝에서 잘린다; 0 아래의 노브는 아무것도 늦추지도 빼지도 않는다 |",
    "지연을 더한 카운트다운도 `int32`의 끝에서 잘린다; 0 아래의 노브는 아무것도 늦추지도 빼지도 않는다. B6b `[B-6b2c]`(헤더 §2.10, #132): 행의 상한 `CosmicConflict::GuardDelayCapOf` — 게임의 몫 0이면 `FleetDelayCap`(100, 지금 값), 몫 0.25면 역병 200의 50(함대 3이 100이 아니라 50을 더한다 — 변이 `b6b2c_guard_cap_ignored`), 0.5면 150의 75·200의 100, 수가 아니거나 0 아래의 몫은 몫 없음, 카운트다운 0은 0 |")
# UI.StartReasons
rep("명령 단추의 사유(Phase 3 12번 작업, `Tests/CosmicCommandReasonTests.cpp` — 테스트의 건설·답 행 여섯과 정착 행의 사본, 게임의 위기 행과 테스트의 `crisis_test_storm`): 조용한 모성계는 건설 행을 사전순으로 보이고 답과 정착 행은 단추가 없다(`GetShownStartBlock`이 `Count`). 시작할 수 있는 건설은 켜지고, 칸 위의 행은 회색 단추에",
    "명령 단추의 사유(Phase 3 12번 작업, `Tests/CosmicCommandReasonTests.cpp` — 테스트의 건설·답 행 여섯과 정착 행의 사본, 게임의 위기 행과 테스트의 `crisis_test_storm`): 조용한 모성계는 건설 행을 사전순으로 보이고 답과 정착 행은 단추가 없다(`GetShownStartBlock`이 `Count`). B6b `[B-6b2c]`(헤더 §4): 건설 행의 단추는 그 칸이 다음 칸일 때부터 보인다 — 첫째 칸 모성계에는 셋째 칸의 `project_test_high` 단추가 없고(변이 `b6b2c_build_button_any_rung`), 둘째 칸 모성계에서 회색으로 선다; 사유 `NoFallen`도 말이 있다. 시작할 수 있는 건설은 켜지고, 칸 위의 행은 (둘째 칸 모성계에서) 회색 단추에")
# Data.Integrity
rep("| `Cosmic_Ascension.Data.Integrity` | B5 `[B-5d2]`:",
    "| `Cosmic_Ascension.Data.Integrity` | B6b `[B-6b2c]`(헤더 §2.2·§2.5·§2.6·§2.10, 노트 §2 ⓑ): 방주의 여유 — 다스리는 이를 죽이는 위기 행(`KillsTheRulerOf`)마다 답인 `DT_Megaprojects` 행이 AI 백성에게 카운트다운 안에 끝난다(`RequiredProgressOf` ÷ `AIAscensionSpeedMultiplier` < `CountdownTicks` — 대응 칸에는 지연이 걸리지 않는다); 게임에는 절멸 행이 `[B-6b6]`까지 없어 테스트의 절멸 250·방주 150을 얹은 사본을 게임의 규칙으로도 본다(변이 `b6b2c_data_ark_margin_half` — 배율 0.5면 300틱), 배율 0.5·방주 250은 잡히고 249·수 아닌 배율(1로 읽는다)은 지난다; `project_binding`을 답으로 드는 위기 행이 없다(대응 칸의 완성은 봉인하지 않는다 — 변이 `b6b2c_data_binding_answers`). 신정론 불변식의 지킴은 행마다 `CosmicConflict::GuardDelayCapOf`를 읽는다(몫 0이면 `FleetDelayCap` — 지금 값). B5 `[B-5d2]`:")
# New rows after Sim.ExtinctionExpiry
rows = """| `Cosmic_Ascension.Sim.ArkAnswers` | (B6b `[B-6b2c]`, `Tests/CosmicUpperRungTests.cpp` — 헤더 §2.6, 노트 §2 ⓑ; 테스트의 절멸 행과 방주 행(셋째 칸, 진척 4), AI 배율 1·지연 0.25) 셋째 칸 AI 백성이 수도에 선 절멸에 방주를 AI 단계에서 대응 칸에 시작하고, 대응 칸은 AI 배율로만 나아간다 — 셋째 칸 행이어도 셋째 칸 위의 지연은 건설 칸의 것이다(변이 `b6b2c_delay_on_response`); 넷째 낸 틱에 끝나 절멸을 풀고(만료 줄 없이 해결 줄), 기록은 그 틱·그 백성이며 백성은 세 성계를 지닌다 |
| `Cosmic_Ascension.Sim.UpperRungDelay` | (B6b `[B-6b2c]`, 같은 파일 — 헤더 §2.6·§3.4, 결정 17; AI 배율 2) 셋째 칸 위의 지연 `AIUpperRungProgressMultiplier`는 다스리지 않는 백성의 건설 칸에서 셋째 칸 이상 행(결박 포함)에만 곱한다 — 0.25에서 0.5, 둘째 칸 행은 2(변이 `b6b2c_delay_second_rung`), 다스리는 백성은 1, 노브 1이면 AI 배율 그대로; 0과 수 아닌 노브는 에러 한 줄씩과 1(변이 `b6b2c_delay_bad_knob_as_is`) |
| `Cosmic_Ascension.Sim.FourthRungBrake` | (B6b `[B-6b2c]`, 같은 파일 — 헤더 §2.6, 결정 16, `docs/21` §7.4 '넷째 칸의 셈'의 경우마다; 테스트의 업적 행 — 셋째 칸은 늘 서는 조건, 넷째 칸은 손으로 놓은 기록 — 과 고향 행의 상한을 바꾼 게임의 은하 행 사본) 상한 2에서 다스리는 백성(넷째 칸)을 세어 `Civs`의 앞 백성이 오르고 다음은 막힌다(변이 `b6b2c_brake_skips_ruled`·`b6b2c_brake_order_reversed`·`b6b2c_brake_counted_once_per_tick`), 줄은 오른 백성의 승천 줄 하나, 막힌 틱과 상한 0의 틱은 같은 수를 뽑는다; 다스리는 백성과 넷째→다섯째는 막지 않는다(변이 `b6b2c_brake_ruled_too`·`b6b2c_brake_fifth_too`); 상한 0과 은하 행이 없는 표는 제동이 없다(변이 `b6b2c_brake_cap_zero_brakes`·`b6b2c_brake_missing_row_closes`); 넷째 칸 백성이 성계를 모두 놓으면 그 틱의 AI 단계는 아직 그 백성을 세고 건넘 단계가 지운 다음 틱에 막힌 백성이 오른다(변이 `b6b2c_brake_counts_third_rung`); 이 틱에 닿는 건넘 기록은 아직 들어오는 중이라 세고 닿은 다음 틱에는 세지 않는다(변이 `b6b2c_brake_counts_landed_transits`) |
| `Cosmic_Ascension.Sim.Binding` | (B6b `[B-6b2c]`, 같은 파일 — 헤더 §2.5, 결정 12~15·32; 테스트의 결박 행(넷째 칸, 진척 4)·게임의 타락 행과 그 독립 사본) (1) `CosmicSeal::BindTargetOf`는 `Transcendents` 차례로 처음인 봉인되지 않은 타락 — 독립 인스턴스·행 없는 인스턴스·봉인된 인스턴스를 지나고 없으면 `INDEX_NONE`(변이 `b6b2c_bind_target_any_realm`·`b6b2c_rebind_moves_sealed_tick`) (2) 끝나는 결박은 기록(만든 백성)을 적고 대상의 `bSealed`·`SealedTick`(그 틱)을 적는다(변이 `b6b2c_binding_does_not_bind`) — 독립 인스턴스는 그대로, 영향권은 남는다, 한 틱 값을 낸다; 완성 줄 바로 뒤에 `log_fallen_bound`(그 성계의 이름, 완성 줄의 초점, Severity 0 — 변이 `b6b2c_bound_line_first`); 그 뒤 그 타락의 주장자가 없다(`ClaimantOf`); 같은 틱에 다른 건설이 끝난 판과 같은 수를 뽑는다(변이 `b6b2c_binding_draws`) (3) 대상이 이미 묶이면 다음 결박은 결과 없이 끝난다 — 빈 칸·진척 0·기록·값·줄 없음, `SealedTick`은 첫 결박의 틱 (4) 한 틱에 두 결박이 끝나면 `Systems` 차례의 앞이 묶고 뒤는 같은 패스에서 결과 없이 끝나 그 백성의 `MegaprojectComplete project_binding`이 거짓, 묶임의 줄은 하나(변이 `b6b2c_target_once_per_pass`) |
"""
anchor = "| `Cosmic_Ascension.Log.AttritionLine` |"
assert s.count(anchor)==1
s=s.replace(anchor, rows + anchor)
open(p,'wb').write(s.encode('utf-8'))
print('ok')
