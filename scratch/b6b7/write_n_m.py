"""[B-6b7] start stage: insert the paragraph '[B-6b7]의 m' into N section 2 (after '[B-6b6]의 다시 셈', before the e-paragraph).
Writes LF, UTF-8 without BOM; refuses if the anchor is not found exactly once or the paragraph is already there."""
import sys

PATH = r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension\docs\60_DECISIONS\headers\B6b_upper_rungs_and_the_race_notes.md"
ANCHOR = "\n\n**ⓔ 지킴 지연의 상한과 W 다시 셈**"

PARA = """

**`[B-6b7]`의 m**(ⓓ — 법 `edict_wide_machine_accord`가 지닌 각성 몫 m, 결정 9; 2026-11-10; 셈은 `scratchpad\\b6b7\\m_awake.py`, 읽기만 — 묶음 A의 R 켬 30판 `Saved\\Balance\\b6b_bundleA\\`, P_bind 1,800·N 1,200·r 0.375는 위 '`[B-6b6]`의 다시 셈' 그대로).
- **입력(잰 값)**: 다스리는 백성이 넷째 칸에 닿은 판 n 30(30판 모두). k(넷째 칸 뒤 첫 표본에서 다스리는 백성의 성계 수) 중앙값 5(4가 7판, 5가 23판). a(그 백성의 아키타입이 각성에 곱하는 몫 — `DT_Civs`) 30판 모두 1.2(`arch_seeker`). R_L(사회 연구 1.0의 틱 − 넷째 칸, 0 아래는 0) 27판이 0(사회 1.0이 넷째 칸보다 먼저 섰다), 나머지 셋이 323·370·485(시드 30·12·27).
- **셈**: S = max(P_bind, R_L + (e^(λN) − 1)/λ), λ = 0.0002·a·k·m. 중앙값 S — m 0.5에서 1,800(가장 큰 판 2,107.7, 시드 27), m 0.375에서 1,800(가장 큰 판 1,985.9), m 0.25에서 1,800(가장 큰 판 1,875.7) — 세 값 모두 목표의 위 끝 3,600 안이고 결박 1,800이 계절을 정한다(k 5·a 1.2·m 0.5에서 끊김을 넣은 창 1,757 < 1,800). 참고로 고리가 없는 m 1이면 k 5에서 2,684·k 4에서 2,255다.
- **값(권장 그대로)**: **m = 0.5** — 사다리 {0.5, 0.375, 0.25}에서 중앙값 S ≤ 3,600인 가장 큰 값(n ≥ 5라 모형 k {4, 7}로 가지 않았다). m 1은 사다리에 없다 — 고리가 없으면 감사한 요약 '…그 일을 멈추고 사람이 직접 살핀다'가 거짓이 된다. 더 작은 m은 계절을 더 줄이지 않으면서(세 값 모두 1,800) 각성이 만료된 뒤의 주장, 곧 끝 전의 1인칭 줄(#136)을 제 몫만큼 더 줄이므로 가장 큰 값을 택한다.
- **#136의 바탕(법 행이 없는 `[B-6b6]` 묶음 A의 R 켬 30판, §0.1 #136의 방법 — Wilson 95%)**: 이 판들은 시계를 껐기에(`-Rules=ProvisionalEndTick:0`) 기록이 없다. 6,000 전에 1인칭 주장 줄이 말해진 판 **7/30 = 0.23 [0.12, 0.41]**(B6a 묶음 1의 7/30과 같다 — 문턱 10/30 아래); 20,000 안 어디서든 18/30 = 0.60 [0.42, 0.75]. 첫 줄의 틱으로 본 곡선: 6,000까지 7/30, 6,400 9/30, 7,000 11/30, 8,000 13/30, 10,000 15/30, 12,000 17/30, 18,000과 20,000 18/30. 이 커밋의 측정(묶음 A′ — 같은 30판을 법 행과 m을 넣고 다시 돌림)이 같은 방법으로 다시 읽어 견준다.
- **생각했으나 택하지 않은 것**: 결정 9의 다른 길 '남은 시간만 늘리고 각성의 발생 몫(①)은 두지 않는다' — 감사한 요약이 각성이 덜 일어남을 약속하므로 택하지 않았다."""


def main():
    data = open(PATH, "rb").read()
    text = data.decode("utf-8")
    if text.startswith("\ufeff") or "\r\n" in text:
        sys.exit("unexpected BOM or CRLF in N")
    if "**`[B-6b7]`의 m**" in text:
        sys.exit("the paragraph is already there")
    if text.count(ANCHOR) != 1:
        sys.exit("anchor found %d times" % text.count(ANCHOR))
    i = text.index(ANCHOR)
    new = text[:i] + PARA + text[i:]
    with open(PATH, "wb") as f:
        f.write(new.encode("utf-8"))
    print("inserted %d bytes" % (len(new.encode("utf-8")) - len(data)))


if __name__ == "__main__":
    main()
