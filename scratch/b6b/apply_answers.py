"""Write the developer's fourteen answers (2026-11-08) into the B6b header and notes: thirteen recommended, decision 22 alternative A
(the design's neighbour reverence +0.2). Usage: python -I apply_answers.py"""
import sys

sys.stdout.reconfigure(encoding="utf-8")
D = r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension\docs\60_DECISIONS\headers"
HP = D + r"\B6b_upper_rungs_and_the_race.md"
NP = D + r"\B6b_upper_rungs_and_the_race_notes.md"
h = open(HP, encoding="utf-8").read()
n = open(NP, encoding="utf-8").read()


def rep(text, old, new, count=1):
    found = text.count(old)
    if found != count:
        raise SystemExit("expected %d, found %d: %s" % (count, found, old[:80]))
    return text.replace(old, new)


A = "결정 22의 답 ㉮"

# --- status line and the answers paragraph
h = rep(h, "상태: **초안 (2026-11-08) — 적대 리뷰를 고친 판, [개발자]의 답 전.**",
        "상태: **승인 (2026-11-08)** — [개발자] 열넷의 답: 결정 22는 대안 ㉮(설계 그대로 이웃 경외 +0.2), 나머지 열셋은 권장.")
h = rep(h, "남은 길: **[개발자]** 열넷(결정 1·2·3·5·9·14·20·21·22·32·35·38·41·42)의 답 → 승인.",
        "→ 개발자의 답 열넷(2026-11-08 — §10의 머리). 남은 길: §8의 차례로 `[B-6b1]`부터.")
h = rep(h, "**승인 때 기다리는 답 — [개발자] 열넷**: 1·2·3·5·9·14·20·21·22·32·35·38·41·42(41·42는 적대 리뷰가 19에서 가르고 넘겨받은 것에서 더했다). 나머지 스물여덟은 권장안이 곧 승인이다(마일스톤 B의 상시 승인). 답이 대안을 고르면 그 결정이 가리키는 절의 본문을 고친 뒤 승인한다.",
        "**[개발자] 열넷의 답**(2026-11-08): 1·2·3·5·9·14·20·21·32·35·38·41·42는 권장, 22는 대안 ㉮ — 그 본문(§2.7·§3.3의 3)을 답에 맞췄다(41·42는 적대 리뷰가 19에서 가르고 넘겨받은 것에서 더했다). 나머지 스물여덟은 권장안이 곧 승인이다(마일스톤 B의 상시 승인).")
h = rep(h, "22. **[개발자] 견딘 영압의 시대(#156)** — 권장:",
        "22. **[개발자] 견딘 영압의 시대(#156)** — **답(2026-11-08): 장치는 권장대로, 이웃 경외는 대안 ㉮** — 행의 효과는 설계 그대로 `ChangeReverence` Key `Borders` +0.2(아래의 '남기지 않는다'와 그 까닭은 답 전의 기록이다). 권장:")
h = rep(h, "기록은 그 답을 처음 들이는 커밋에(A1~A4·#1은 `[B-6b1]`,", "기록은 그 답을 처음 들이는 커밋에(A1~A4·#1·[개발자] 열넷의 답은 `[B-6b1]`,")

# --- decision 22, alternative A: the Borders key and the era row's effect
h = rep(h, "조건·선택자 `[B-6b2]`(열쇠는 결정 22의 대안일 때)", "조건·선택자·열쇠 `Borders` `[B-6b2]`(%s)" % A)
h = rep(h, "- **열쇠 `Borders`**(결정 22의 대안일 때만 — 권장의 행에는 효과가 없다): ㉮ `ChangeReverence`의 Key `Borders`",
        "- **열쇠 `Borders`**(%s): `ChangeReverence`의 Key `Borders`" % A)
h = rep(h, " ㉯면 같은 낱말을 `ChangeRelation`의 Key로(그릇이 아니라 백성 사이의 관계 — 방향은 그 답이 오면 정한다).", "")
h = rep(h, "효과 없음(결정 22의 권장 — 대안 ㉮ `ChangeReverence Borders +0.2`, ㉯ `ChangeRelation Borders +0.2`)",
        "효과 `ChangeReverence Borders +0.2`(%s — 설계 그대로)" % A)
h = rep(h, "설계의 경외 +0.2를 남기는가(메시아 신호 9·10 — `theology_and_bestiary_notes.md:243-244`)는 결정 22가 묻는다 — 권장은 줄만.",
        "설계의 경외 +0.2가 남으므로(%s) 줄은 그 수를 말하지 않고 그릇을 우러르는 말이 없다(메시아 신호 9·10 — `theology_and_bestiary_notes.md:243-244`; 경외는 예배가 아니다 — `systems.md:195`)." % A)
h = rep(h, "(`Ruled`는 Key 낱말; `Borders`는 결정 22의 대안일 때만)", "(`Ruled`·`Borders`는 Key 낱말 — `Borders`는 %s)" % A)
h = rep(h, "3. **`Borders` 키**(결정 22의 대안일 때만 — 권장이면 이 규칙과 목록 `words.reverence_reach`는 들지 않는다): 그 효과의 Key는 비거나 `Borders`;",
        "3. **`Borders` 키**(%s): `ChangeReverence`의 Key는 비거나 `Borders`;" % A)
h = rep(h, "`Ruled`(`Borders`는 결정 22의 대안일 때)", "`Ruled`·`Borders`(%s)" % A)
h = rep(h, "#156의 국경 경외(결정 22가 대안 ㉮일 때)", "#156의 국경 경외(%s)" % A)
h = rep(h, "4. **견딘 영압의 시대**(#156): 저장 필드·조건·상시 이벤트 행 하나씩. 결정 22.",
        "4. **견딘 영압의 시대**(#156): 저장 필드·조건·상시 이벤트 행 하나씩, 설계 그대로 이웃 경외 +0.2. 결정 22(답 ㉮).")

n = rep(n, "(권장은 효과 없이 줄만 — `Borders` 키는 이웃 경외를 남기는 대안일 때)", "(%s — 설계 그대로 이웃 경외 +0.2, `Borders` 키)" % A)
n = rep(n, "| 결정 22([개발자] — 이웃 경외를 남기는가) |", "| 결정 22(답 ㉮) |")
n = rep(n, "(증언의 +0.1; 시대 행은 결정 22의 대안 ㉮일 때만)", "(증언의 +0.1; 시대 행의 국경 +0.2 — %s)" % A)
n = rep(n, "`:779-785`(`ChangeReverence`(㉮)나 `ChangeRelation`(㉯)의 `Borders` 키 — 결정 22의 대안일 때만)", "`:779-785`(`ChangeReverence`의 `Borders` 키 — %s)" % A)
n = rep(n, "효과 키 `Borders`(결정 22의 대안일 때)", "효과 키 `Borders`(%s)" % A)
n = rep(n, "`ChangeReverence`의 `Borders` 키(결정 22의 대안일 때만)", "`ChangeReverence`의 `Borders` 키(%s)" % A)
n = rep(n, "`Borders`는 국경의 백성마다 한 번(결정 22의 대안일 때만).", "`Borders`는 국경의 백성마다 한 번(변이 — 한 백성에 두 번 더하면).")
n = rep(n, "설계의 +0.2를 남기면(결정 22의 대안 ㉮) 그 수는", "설계의 +0.2가 남으므로(%s) 그 수는" % A)
n = rep(n, "시대 행이 서는 판만(결정 22의 권장이면 효과 없는 줄이라 전체 해시만)", "시대 행이 서는 판만(국경의 경외가 세계라 두 해시 — %s)" % A)
n = rep(n, "결정 22가 대안 ㉮일 때 #156의 국경 경외", "#156의 국경 경외(%s)" % A)
n = rep(n, "- **고친 뒤 확인**(2026-11-08, 읽기만):",
        "- **개발자의 답**(2026-11-08): [개발자] 열넷 가운데 열셋은 권장, 결정 22는 대안 ㉮(설계 그대로 이웃 경외 +0.2 — 리뷰가 뒤집은 권장을 개발자가 설계 쪽으로 되돌렸다). 헤더의 열쇠 `Borders`·스키마 규칙 3·`Sim.EraSelector`의 경우·기준값 표의 `[B-6b8]` 칸·B1 지킴의 미리 이름한 길·§0.4의 표를 그 답에 맞췄다.\n- **고친 뒤 확인**(2026-11-08, 읽기만):")

leftover = [s for s in ("결정 22의 대안", "결정 22가 대안", "결정 22의 권장", "reverence_reach") if s in h or s in n]
open(HP, "w", encoding="utf-8", newline="\n").write(h)
open(NP, "w", encoding="utf-8", newline="\n").write(n)
print("header", len(h.encode("utf-8")), "notes", len(n.encode("utf-8")), "leftover", leftover)
