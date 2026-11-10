# [B-6b8] records stage: the H section 11 line and the DECISIONS_LOG line (placeholders <<B:NAME>>).
import sys

ROOT = r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension"
H = ROOT + r"\docs\60_DECISIONS\headers\B6b_upper_rungs_and_the_race.md"
D = ROOT + r"\docs\60_DECISIONS\DECISIONS_LOG.md"

# Filled, the line must stay under 400 bytes: the skeleton without the four values is 357 bytes with its newline,
# so J1 + J2 + J3 + C together may take at most 42 bytes (e.g. '45', '27%', '2,524·93%', '12,000' = 20 bytes).
H_LINE = ("- `[B-6b8]`(2026-11-10): 데이터 ③ 곁 행 `event_era_endured`·`enc_witness`와 묶음 B의 판단"
          "(① <<B:H_J1>>/60·② <<B:H_J2>>·③ <<B:H_J3>>·C <<B:H_C>>). 다르게: 짝의 다리는 min(첫 셋째 칸+1, 시대 줄, 증언 카드)"
          "(노트 ⓗ는 첫 셋째 칸 — CSV가 앎을 읽는다); 물러남의 셈·증언 첫 틱; 강제 죽음은 `-Wander` 없이. 세부는 노트 §5.")

D_LINE = ("- `[B-6b8]`(2026-11-10 — 결정 20~22의 데이터 ③, 헤더 §2.7·§2.8·§5.3·§8): 곁 행 둘을 켜고 묶음 B로 미리 적은 판단 넷을 읽는다. "
 "**행**(글은 B6b 노트 §1.5 그대로): `DT_Events`의 `event_era_endured`(견딘 영압의 시대 — 상시 행, 다스리는 백성(`SystemOwnedBy Ruled`)의 견딘 셈 600에서 한 번, "
 "국경을 맞댄 백성마다 경외 +0.2(`Borders` — 결정 22의 답 ㉮), 줄 셋 물레·장독·느린 수레의 시대, 분류 `Event.Civ.Society`; 12 → 13행)와 "
 "`DT_Encounters`의 `enc_witness`('그늘의 가장자리' — 주장이 선 성계에 선 방랑자의 상시 카드, '그늘의 가장자리를 재어 둔다'의 정신 판정 3이 이기면 주인 백성의 경외 +0.1, "
 "'물러난다'는 판정도 효과도 없다, 주장을 끝내지도 늦추지도 않는다 — 결정 20·21; 18 → 19행). "
 "**스키마**: 증언 낱말 셋(`witness_words`·`sign_voice_words`·`witness_record_words`)을 주장 곁 방랑자 카드의 행 규칙 하나에 — 징조는 보이거나 재어질 뿐 증언·기록·말함·떠남이 없다(노트 §1.6 규칙 7); "
 "결정 22의 꼴(`Ruled`와 `Borders`)을 `DT_Events`의 행 규칙 하나에(검사기 단위 테스트 95 → 96, 걷는 수 316 → 330). "
 "**측정**: 방랑자 정책의 판정 없는 답의 셈(`NoCheckByRow`)과 race 줄의 증언 첫 틱·답의 갈래(판정을 거친 답·실패·판정 없는 답·답 전에 사라진 카드); 방주 카운터는 넣지 않았다. "
 "**묶음 B**(<<B:B_PROCS>> 프로세스, 두 번 실패 <<B:B_FAILED_TWICE>>, 검사 줄 0, 짝 <<B:B_PAIRS>> — 다리는 칸마다 min(첫 셋째 칸 + 1, 첫 시대 줄, 첫 증언 카드), 노트 ⓗ에서 벗어남; 6 작업자 <<B:B_MINUTES>>분): "
 "① J1 <<B:J1>>/60 ② J2 <<B:J2>>(가름 <<B:SPLIT>>) ③ 길이 <<B:J3_LEN>>·밀도 <<B:J3_DENSITY>> ④ C₀ <<B:C0>>·C <<B:C>>·(i) <<B:J4_I>>·(ii) <<B:J4_II>>; "
 "#156 시대 줄 <<B:D_156>>, #141 증언 <<B:D_141>>; 잠김 <<B:B_LOCKED>>; 넘김 <<B:HANDOFF>>. "
 "**기준값**: 다섯 줄과 해시 판 둘 비트 그대로(시드 42의 다섯 판에 시대 줄·증언 카드 0 — 1,072의 폐위 뒤 시대 행이 서지 않고 방랑자 판에 주장이 없다); 같은 빌드의 `-WithoutB6bData=1` 다섯 판은 `[B-6b5]`의 줄 그대로. "
 "**테스트**: 223 → 225(`Sim.EraEndured`·`Sim.WitnessCard` — 새 파일 `Tests/CosmicSideRowTests.cpp`), `Data.Integrity`의 걸림줄은 열 행; 고친 테스트 없음. "
 "**변이**: 새 `b6b8_` 열일곱(다시 뜬 셋 포함)과 다시 돌린 옛 129 — 탐침 둘 밖 144 모두 깨짐(살아남음 0); 일의 차례를 바꿔 묶음 B 앞에서 돌렸다(리드의 결정). "
 "리뷰: 발견 17(확인 10·일부 7·반박 0 — 모두 들였다). 구절은 더하지 않았다. 세부는 B6b 노트 §3 '묶음 B'·§5 `[B-6b8]`.")

def insert_after(path, prefix, new_line):
    raw = open(path, "rb").read()
    assert b"\r" not in raw
    lines = raw.decode("utf-8").split("\n")
    assert not any(l.startswith(new_line[:14]) for l in lines), "already there"
    hits = [i for i, l in enumerate(lines) if l.startswith(prefix)]
    assert len(hits) == 1, (path, hits)
    lines.insert(hits[0] + 1, new_line)
    return "\n".join(lines).encode("utf-8")

h = insert_after(H, "- `[B-6b7]`(2026-11-10): 데이터 ② — 법칙 셋", H_LINE)
d = insert_after(D, "- `[B-6b7]`(2026-11-10 — 결정 9~15의 데이터 ②", D_LINE)
print("H", len(h), "D", len(d))
skeleton = H_LINE
for k in ("<<B:H_J1>>", "<<B:H_J2>>", "<<B:H_J3>>", "<<B:H_C>>"):
    skeleton = skeleton.replace(k, "")
print("H line skeleton bytes with newline", len((skeleton + "\n").encode("utf-8")))
if "--write" in sys.argv:
    open(H, "wb").write(h)
    open(D, "wb").write(d)
    print("written")
