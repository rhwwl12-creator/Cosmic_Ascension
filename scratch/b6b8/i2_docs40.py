# I2 of [B-6b8]: the docs/40 edits (race-line item 13, the two new test rows, the tripwire text, the switch paragraph).
import sys

PATH = sys.argv[1]
with open(PATH, "rb") as f:
    raw = f.read()
assert b"\r\n" not in raw, "CRLF in docs/40"
text = raw.decode("utf-8")

EDITS = [
    # 1. Race line item 13: the witness part's new layout.
    ("⑬ 시대의 줄(`event_era_endured` — 판·기록까지·첫 틱)과 증언 카드(`enc_witness` — 방랑자 정책이 행마다 센 온 수·기록까지·성공·실패·물러남; 성공은 받아들인 답 − 실패) ⑭",
     "⑬ 시대의 줄(`event_era_endured` — 판·기록까지·첫 틱)과 증언 카드(`enc_witness` — 방랑자 정책이 행마다 센 온 수, 첫 카드가 온 경계(`first at` — `[B-6b8]`, 짝의 다리 W; 정책은 그릇이 걷는 동안 `-Wander`와 상관없이 카드를 세므로 답이 없는 판에서도 선다), 기록까지, 받아들인 답(`answered`)과 그 갈래 — 판정을 거친 답(`by the check` = 받아들인 답 − 실패 − 판정 없는 답)·실패·판정 없는 답(`without a check` — 물러남: 정책이 고른 선택지의 `CheckStat`이 `NoCheck`, `FCosmicWandererPolicyCounts::NoCheckByRow`)·답하기 전에 사라진 카드(`gone before the answer` — 정책의 `WithdrawnByRow`, '물러난다' 선택지가 아니다); `[B-6b8]` 전의 꼴 `succeeded/failed/withdrawn`의 `succeeded`는 판정을 지난 답과 물러남을 섞었다) ⑭"),
    # 2. The Data.Integrity row: the tripwire on the ten.
    ("`CosmicTests::WithoutB6bData`의 핀 — 지금 데이터에서는 `[B-6b6]`·`[B-6b7]`의 일곱 행(`feat_passage_by_ark`·`feat_laws_and_binding`·`project_ark`·`crisis_species_extinction`·`edict_wide_machine_accord`·`edict_wide_quarantine`·`edict_wide_warning`)과 `project_binding`을 뺀다(`[B-6b8]`이 제 행과 함께 고칠 덫 — `b6b7_without_b6b_keeps_laws`·`b6b7_without_b6b_keeps_fifth_feat`도 — 변이 `b6b3b_without_b6b_keeps_binding`)",
     "`CosmicTests::WithoutB6bData`의 핀 — `[B-6b8]`부터 지금 데이터가 B6b의 열 행을 모두 지녀 도우미는 정확히 그 열을 제 차례로 뺀다(`[B-6b6]`의 `feat_passage_by_ark`·`project_ark`·`project_binding`·`crisis_species_extinction`, `[B-6b7]`의 `feat_laws_and_binding`과 은하 법칙 셋, `[B-6b8]`의 `event_era_endured`·`enc_witness` — 열 가운데 하나가 데이터를 떠나거나 도우미의 이름이 제 행을 못 찾으면 깨지는 덫; 변이 `b6b3b_without_b6b_keeps_binding`·`b6b7_without_b6b_keeps_laws`·`b6b7_without_b6b_keeps_fifth_feat`)"),
    # 3. The switch paragraph: the rows taken out today.
    ("— 지금 데이터에서는 `feat_passage_by_ark`·`project_ark`·`project_binding`·`crisis_species_extinction` 넷, 이 차례로)",
     "— `[B-6b8]`부터 지금 데이터에서는 열 모두, 이 차례로)"),
]

ROW_ERA = ("| `Cosmic_Ascension.Sim.EraEndured` | (B6b `[B-6b8]`, `Tests/CosmicSideRowTests.cpp` — 헤더 §2.7, #156, 결정 22의 답 ㉮; 게임의 `event_era_endured`만 든 `DT_Events`, "
           "테스트가 짠 지형과 테스트의 타락 초월자 행, 소모 노브 0.001 — 영향권이 다스리는 백성과 AI 백성 하나를 수도 밖의 성계로 깎는다) 게임 데이터의 시대 행: 다스리는 백성의 셈을 590에 두면 "
           "599에는 줄이 없고, 조건의 창(슬롯 10틱이면 600~609)을 지나며 그 백성에 관한 줄이 꼭 하나다; 국경을 맞댄 백성마다 행의 0.2를 0에서 얻고 그 백성 자신과 국경 밖의 백성은 얻지 않는다"
           "(변이 `b6b8_data_era_borders_0_3` — 행의 값 0.3, 에셋을 다시 만드는 데이터 패치); 같은 소모 아래 같은 창을 지난 AI 백성은 줄이 없다(`Ruled`); 창 뒤에 같은 견딤은 다시 서지 않으며; "
           "행이 없는 같은 판과 시뮬 스트림이 같다(줄은 말하기 스트림만 뽑는다) |")
ROW_WITNESS = ("| `Cosmic_Ascension.Sim.WitnessCard` | (B6b `[B-6b8]`, 같은 파일 — 헤더 §2.8, #141, 결정 20·21; 게임의 `DT_Encounters` 그대로, 테스트의 불씨 행(놓은 다음 틱에 만료)과 주장 행(300틱), "
               "타락 영향권은 성계 1 — 백성 1의 성계 2를 주장한다, 도착 카드 몫 0) 게임 데이터의 증언 카드: 첫째 칸의 방랑자가 선 성계에 주장이 내려앉으면(불씨의 만료 — 도착 없이, 틱 머리에 서 있지 않던 단위) "
               "그 틱에 `enc_witness`가 오고, 한 칸 위 백성의 주장된 성계에 도착해도 온다; 두 칸 위 백성의 땅에서는 오지 않는다; 답한 뒤 한 레인 갔다 돌아와도 곧바로 다시 오지 않는다(생애의 가장 최근 답); "
               "모든 눈에서 통과하는 Mind(3)로 판정 선택지를 고르면 주인의 경외가 행의 0.1만큼 오르고(변이 `b6b8_data_witness_reverence_0_3` — 행의 값 0.3, 데이터 패치) 주장과 그 카운트다운은 그대로다; "
               "물러남은 아무 경외도 올리지 않는다; 받아들인 답마다 시뮬 스트림의 수 하나; 방랑자 정책이 두 경계에서 증언 카드에 차례로 답하면(판정, 그다음 물러남) 행마다 카드 둘·답 둘·판정 없는 답 하나를 센다"
               "(`NoCheckByRow` — 변이 `b6b8_wanderer_nocheck_not_by_row`: 모든 답을 판정 없는 답으로 센다) |")

for old, new in EDITS:
    n = text.count(old)
    assert n == 1, ("count %d for: %s" % (n, old[:60]))
    text = text.replace(old, new)

anchor = "| `Cosmic_Ascension.Sim.EraSelector` |"
lines = text.split("\n")
idx = [i for i, l in enumerate(lines) if l.startswith(anchor)]
assert len(idx) == 1, idx
lines[idx[0] + 1:idx[0] + 1] = [ROW_ERA, ROW_WITNESS]
text = "\n".join(lines)
assert all(ord(c) != 0xFEFF for c in text[:1])
with open(PATH, "wb") as f:
    f.write(text.encode("utf-8"))
print("ok")
