"""Editor D: reduce docs/10_DESIGN/EXISTENCE_EVOLUTION_AND_ECOLOGY.md to the accepted form (answers 2026-11-10).
Usage: python -I -X utf8 reduce_design.py <in.md> <out.md>"""
import re, sys

src, dst = sys.argv[1], sys.argv[2]
raw = open(src, 'rb').read()
assert b'\r' not in raw and not raw.startswith(b'\xef\xbb\xbf')
text = raw.decode('utf-8')
lines = text.split('\n')


def idx(prefix):
    hits = [i for i, l in enumerate(lines) if l.startswith(prefix)]
    assert len(hits) == 1, (prefix, hits)
    return hits[0]


i3 = idx('## 3. ')
i33 = idx('### 3.3 ')
i4 = idx('## 4. ')
i43 = idx('### 4.3 ')

LINK = '../archive/EXISTENCE_EVOLUTION_original_2026-11-09.md'

head = f"""# B7x 설계의 받은 꼴 — 블록을 지닌 백성과 살아낸 시간

> **상태 (2026-11-10): 받은 꼴 — 아직 코드에 없다.** 개발자가 2026-11-10 B7x 헤더 `docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md` §10의 결정에 '권장대로' 답했고, 이 문서에는 그 답에 맞춘 받은 꼴만 남았다 — §3.3 겹침 규칙 열다섯, §4.3 갈림 카드 열두 장, §4.4 끝의 꼴, §5 이어서 일하는 차례. 표지가 없어도 이 문서의 문장은 아직 코드에 없는 예정이다(슬라이스 가는 `[B-7x1]`~`[B-7x5c]`, 나·다는 그 뒤 — 헤더 §8). 무엇을 어떤 꼴로 받는지는 헤더와 그 노트(예순 블록의 처분표)가 정한다 — 이 문서와 헤더가 어긋나면 헤더가 이긴다.
> **원안**: 2026-11-09 개발자가 쓴 원안(§1·§2·§3.1·§3.2·§4.1·§4.2와 처음 §5의 프롬프트 셋)은 [원안 동결본]({LINK})에 글자 그대로 있다 — 받지 않은 것을 담고 있고 고치지 않는다. 이 문서는 그 글을 줄 번호가 아니라 '원안 동결본 §2 축 5'처럼 절의 제목으로 부른다. 원안의 '존재'는 받은 꼴에서 늘 **백성**이고, 원안의 '5칸급'·`Scale`·'유목신'·영어 별칭·사람과 별의 수는 동결본에만 남고 게임에 내지 않는다.
> **잎 이름 (헤더 결정 3)**: 옛 이름 → 새 이름 — `UnseenLaw` → `KeptLaw`, `SilenceFeed` → `IdleFeed`, `PlanckMicro` → `FoldedSmall`, `DysonBrain` → `ShutSphere`, `KesslerTapestry` → `DustBand`, `AbyssalMaw` → `InwardHunger`, `StarShepherd` → `StarTow`, `HermitMonolith` → `ClosedPort`. 이 문서는 새 이름만 쓴다(동결본은 옛 이름 그대로다). 변이의 잎은 꼴의 이름을 받지 않는다 — 꼴 낱말(`WovenSwarm`·`LastHunger` 따위, §4.4)은 변이 없이도 부모를 지닌 모든 백성에 보인다.
> **리뷰와 답**: 2026-11-09 ultracode 적대 리뷰가 받은 꼴에서 고칠 것을 찾았고, 개발자의 답(2026-11-10)을 받은 것을 이 판에 넣었다 — 잎 이름(결정 3), 남긴 법의 좁힘(C10의 귀결 — 결정 18), 끝의 꼴이 붙는 장의 세 규칙과 '원안의 줄' 열의 삭제(§4.4 — 결정 13·21), 둘째 갈림 ②의 되풀이 이득(§4.3 — 결정 11 (나)), S14의 꼴(결정 15), 남은 기계와 탄생의 조건(S12 — 결정 16), '어디에나'를 표현으로도 받지 않음(S03·S15 — 결정 18). 카드의 글과 값의 남은 고침(C10의 제목과 ②의 윤리 이동 — 노트 §5.4 데2)은 `[B-7x4]`·`[B-7x5c]`의 몫이다. 어긋나면 헤더가 이긴다.

---

{lines[i3]}

원안의 §3.1(블록 조합의 예시)과 §3.2(반응형 상호작용 조립기)는 [원안 동결본]({LINK})에 있다 — 받은 꼴은 아래 §3.3이고, 상대의 블록에 맞춰 짓는 반응형 조우 카드 열넷은 B7의 AI 백성 행이 블록을 지닌 뒤에 쓴다(헤더 결정 11).
"""

sec4 = f"""{lines[i4]}

원안의 §4.1(시간 누적 벡터 여섯)과 §4.2(시간 기반 최종 결과 상태)는 [원안 동결본]({LINK})에 있다 — 받은 꼴은 백성의 시간 셈 여섯(헤더 §2.1·§2.2 — `FCivilizationState`에만 두고 생애를 건너지 않는다, 결정 4)과 아래 §4.3·§4.4다.
"""

body33 = '\n'.join(lines[i33:i4])          # §3.3 .. the '---' before §4
body43 = '\n'.join(lines[i43:])            # §4.3 .. end
out = head + '\n' + body33 + '\n' + sec4 + '\n' + body43

# ---- leaf renames (answer 3) ----
for old, new in [('UnseenLaw', 'KeptLaw'), ('SilenceFeed', 'IdleFeed'), ('PlanckMicro', 'FoldedSmall'),
                 ('DysonBrain', 'ShutSphere'), ('KesslerTapestry', 'DustBand'), ('AbyssalMaw', 'InwardHunger'),
                 ('StarShepherd', 'StarTow'), ('HermitMonolith', 'ClosedPort')]:
    # keep the one 'old -> new' record in the head block
    pre, rec, post = out.partition('> **잎 이름 (헤더 결정 3)**')
    rec_line, nl, rest = post.partition('\n')
    rest = re.sub(r'\b' + old + r'\b', new, rest)
    pre = re.sub(r'\b' + old + r'\b', new, pre)
    out = pre + rec + rec_line + nl + rest


def rep(old, new, count=1):
    global out
    n = out.count(old)
    assert n == count, (n, old[:80])
    out = out.replace(old, new)


# ---- §3.3 intro: K4 (answer 2) ----
rep("AI 백성은 씨앗과 변이 걸음, 그리고 방랑자의 카드 C12(주인 백성에 `Nomad`)로만 블록을 얻는다.",
    "AI 백성은 씨앗과 변이 걸음, 그리고 방랑자의 카드 C12(주인 백성에 `Nomad`)로만 블록을 얻는다. 업적 행의 조건에는 `Brick.` 태그를 두지 않는다(헤더 §3.3 규칙 7 — 결정 2, K4).")

# ---- S03: the fog body (answer 13 ①) ----
rep("안개의 몸(관측되면 굳는다)은 받지 않는다 — 얼굴은 S15.",
    "안개의 몸(관측되면 굳는다 — 원안 동결본 §2 축 1의 `QuantumCloud`)은 받지 않는다 — 남는 얼굴은 S15의 소문 한 줄뿐이다(헤더 결정 18).")

# ---- S11: star dimming switched off (answer 6) ----
rep("- 지키는 것: 몫만; 별 등급을 낮추지 않는다(결정 8).",
    "- 지키는 것: 몫만; 별 등급을 낮추지 않는다(헤더 결정 8 — 어두워짐 장치는 들지만 주기 0으로 꺼 둔다; 켤지는 B8의 실제 하늘 헤더가 정한다).")

# ---- S12: the leftover machines (answer 12 (3a)·(4)) ----
rep("띠는 C05와 변이 걸음 차례 2로 온다. 후보의 블록(남은 기계 둘)은 탄생 블록이라 같은 뜻의 가 블록으로 옮겼다.",
    "띠는 C05와 변이 걸음 차례 2로 온다. 원안의 후보 블록(남은 기계 둘 — 원안 동결본 §2 축 1의 `FossilAutomata`와 축 5의 `UndyingAutomata`)은 받지 않아 같은 뜻의 가 블록으로 옮겼다 — 잎 `Undying*`은 선언하지 않고, 남은 기계의 꼴은 절멸(`LetPeopleGo`)에서 태어나지 않으며 글이나 스캔 얼굴로만 둔다(헤더 결정 16 (3a)·(4)).")

# ---- S14: settle-driven form, no period knob (answer 11) ----
rep("`Unlink`·`Link`(창구의 규칙 그대로); 좌표는 둘째 걸음.",
    "`Unlink`·`Link`(창구의 규칙 그대로). 걸음은 정착이 끝나는 사건에 매여 주기 노브가 없다(헤더 결정 15 — 주기마다 옮기는 꼴은 고르지 않았다). 좌표는 창구가 받은 뒤 둘째 걸음이다(결정 15).")

# ---- S15: 'everywhere' not even in the presentation (answer 13 ①) ----
rep("- 지키는 것: 켠 판과 끈 판의 세계 해시가 같다(`Sim.KnowingInert`의 본).",
    "- 지키는 것: 켠 판과 끈 판의 세계 해시가 같다(`Sim.KnowingInert`의 본). '어디에나'는 표현으로도 받지 않는다 — 표현은 무주 성계 위에 그 백성을 그리지 않고, 남는 것은 이 소문 한 줄이며 그 줄은 제 별 곁에만 선다(헤더 결정 18).")

# ---- §4.3 common: measurement policy (answer 8/19) and the second-cross choice 2 (answer 8 (나)) ----
rep("카드마다 값·문·효과·다음 카드 없는 선택이 하나 있다. 답마다 시뮬 스트림 1. 측정 정책은 `Brick.` 태그를 주거나 걷는 선택과 `DeposeRuler`를 건너뛴다(`-Crossroads=1`에서만 고른다).",
    "둘째 갈림의 ②는 닫지 않는다 — 지난 세 답이 지나면 다시 오므로 ②에는 되풀이해 쌓이는 이득을 두지 않고 값과 작은 효과만 남긴다(`petition_road`·`petition_feast`의 꼴; 헤더 결정 11 (나), 데이터는 노트 §5.4 데1). 새 태그는 없다 — 블록 태그는 열일곱 그대로다. 카드마다 값·문·효과·다음 카드 없는 선택이 하나 있다. 답마다 시뮬 스트림 1. 측정 정책은 갈림 카드에서 값 없는 선택(③)만 고른다(헤더 결정 19 — 블록을 주거나 걷는 선택도, 블록 없는 값 있는 선택도 건너뛴다); 측정 전용 `-Crossroads=<k>`에서만 블록을 주거나 걷는 선택을 고르고, 그때만 C10 ①이 `DeposeRuler`를 지난다(헤더 §5).")

# ---- C09: decision 11 answered ----
rep("`CivTierAtLeast 2`(정착이 막힌 백성의 셋째 칸은 초신성 업적뿐이다 — **[개발자]**)",
    "`CivTierAtLeast 2`(정착이 막힌 백성의 셋째 칸은 초신성 업적뿐이다 — 헤더 결정 11로 받았다)")

# ---- C10: choice 2 without the stacking exaltation (answer 8 (나)) ----
rep("— `ChangeStability` +0.05, `ChangeReverence` +0.1, `PlayerStatDelta Exaltation` +1.",
    "— `ChangeStability` +0.05, `ChangeReverence` +0.1(높아짐은 주지 않는다 — 헤더 결정 11 (나)).")

# ---- C10 consequence: the narrowing (answer 13) ----
rep("- 귀결: ①의 읽는 자리(헤더 §2.4) — 다스리는 이가 없을 때(섭정·AI) 그 백성은 지닌 칙령을 하나도 무르지 않는다; 백성 범위의 칙령까지라 셋째 칸에서도 빈 선택이 아니다. 안정도를 깎는 칙령도 그대로 남는다",
    "- 귀결: ①의 읽는 자리(헤더 §2.4) — 다스리는 이가 없을 때(섭정·AI) 그 백성은 지닌 칙령을 무르지 않는다; 백성 범위의 칙령까지라 셋째 칸에서도 빈 선택이 아니다 — 다만 주장의 카운트다운에 더하는 칙령과 위기의 답인 칙령은 왕좌가 빈 뒤 여느 때처럼 거두고(K9 — B6b 결정 9가 은하 법칙에 그 고리를 금지한 선과 같다), 판정받는 행이 법칙을 물으면(넷째 칸 — `CosmicEdicts::AsksForLawsOf`) 백성 범위 칙령은 다른 AI처럼 거둔다(칙령 자리 셋이 법칙 셋의 자리다, B6b 결정 10·11; 헤더 결정 18). 남는 귀결은 하나다 — 안정도를 깎는 칙령도 그대로 남는다")
rep("묶음이 양위 뒤 그 백성의 안정도와 반란을 잰다(헤더 §5 판단 ⑧의 `-Crossroads` 칸 — 안정도 0.2 아래 첫 틱·반란·지워짐·`LawsHeldTicks`).",
    "이것이 설계가 노린 값이다. 묶음이 양위 뒤 그 백성의 안정도와 반란을 잰다(헤더 §5 판단 ⑧의 `-Crossroads` 칸 — 안정도 0.2 아래 첫 틱·반란·지워짐·`LawsHeldTicks`); 답이 그 수보다 먼저 왔으므로 수가 놀라우면 노브(`[B-7x5r]`)로 다루고, 그래도 안 되면 수와 함께 다시 묻는다.")
rep("②는 붙드는 선택 — 높아짐 +1; 내려온 쪽에 높아짐을 깎아 주지 않는다.",
    "②는 붙드는 선택이다 — 닫히지 않아 다시 오므로 높아짐을 주지 않는다(결정 11 (나)); 내려온 쪽(①)에도 높아짐을 깎아 주지 않는다.")

# ---- §4.4 (answer 9) ----
rep("§4.2의 '최종 결과 상태'는 이 꼴로 받는다. 판이 끝나면 연대기의 생애 장 사실 줄('다스린 백성 · 칸 · 주기') 끝에 **그 백성의 사는 모양을 가리키는 낱말 하나**가 붙는다. 표시할 때 `CosmicBricks::ShapeOf`가 백성의 블록과 시간 셈에서 셈하고 어디에도 저장하지 않는다.",
    "원안의 '최종 결과 상태'(원안 동결본 §4.2)는 이 꼴로 받는다(헤더 결정 13 ㉢ — 저장 없음). 판이 끝나면 연대기의 생애 장 사실 줄('다스린 백성 · 칸 · 주기') 끝에 **그 백성의 지금 살림을 가리키는 낱말 하나**가 붙는다. 표시할 때 `CosmicBricks::ShapeOf`가 백성의 블록과 시간 셈에서 셈하고 어디에도 저장하지 않는다 — `ReignShapes` 같은 필드는 두지 않고 새 R도 없다.\n\n"
    "- **어느 장에, 어느 백성의** (헤더 §4 첫 항과 같은 세 규칙):\n"
    "  - (a) 그 생애의 마지막 재위 백성(`Life.RuledCivIDs`의 끝)만 쓴다 — 다스린 적 없는 생애의 장에는 붙이지 않는다.\n"
    "  - (b) 그 백성이 `State.Civs`에 있을 때만 붙인다 — 지워진 백성에는 붙이지 않는다.\n"
    "  - (c) 같은 백성을 다스린 생애가 여럿이면 마지막 생애의 장에만 붙인다 — 시간 셈은 백성의 것이고 생애를 건너지 않는다(결정 4).\n"
    "  - (d) 낱말은 그 생애의 몫이 아니라 '그 백성의 지금 살림'으로 읽히게 쓴다. 셈은 통치가 끝난 뒤에도 섭정과 AI 아래에서 쌓이고 판의 끝 뒤에도 시뮬이 돌므로, 판의 끝 뒤 연대기를 다시 열면 낱말이 달라질 수 있다 — 이것을 받는다.")
rep("- **하지 않는 것**: 엔딩의 갈래나 화면 제목이 아니다(엔딩은 다섯 갈래 그대로, 끝마다 다른 것은 까닭 줄뿐이다).",
    "- **하지 않는 것**: 엔딩의 갈래나 화면 제목이 아니다(엔딩은 다섯 갈래 그대로, 끝마다 다른 것은 까닭 줄뿐이다 — 낱말은 발치가 아니라 사실 줄에 붙어 이 약속을 지킨다).")

# table: drop the last column
tbl_old_hdr = "| 답 | 언제 | 화면의 낱말 (초안) | §4.2 원안의 줄 |\n|---|---|---|---|\n"
assert out.count(tbl_old_hdr) == 1
start = out.index(tbl_old_hdr)
end = out.index('\n\n', start)
rows = out[start:end].split('\n')
new_rows = ['| 답 | 언제 | 화면의 낱말 (초안) |', '|---|---|---|']
for r in rows[2:]:
    cells = r.split(' | ')
    assert r.startswith('| ') and r.endswith(' |') and len(cells) == 4, r
    new_rows.append(' | '.join(cells[:3]) + ' |')
assert len(new_rows) == 14, len(new_rows)
out = out[:start] + '\n'.join(new_rows) + out[end:]

rep("원안의 이름(영어 별칭과 『…』의 문장)은 설계 문서에만 남는다.",
    "원안의 이름(영어 별칭과 『…』의 문장)은 원안 동결본에만 남는다.")

# ---- §5 (answers 1, 3, 14/20, 15, 21, 22) ----
rep("원안의 프롬프트 1은 이 문서의 §3.3·§4.3·§4.4와 헤더",
    "원안의 프롬프트 셋은 원안 동결본 §5에 있다. 프롬프트 1은 이 문서의 §3.3·§4.3·§4.4와 헤더")
rep("**다른 PC의 세션에 붙여 넣던 문구 `promptforother.txt`는 이 브랜치에서만 쓰였고 `[B-7x0]`에서 지운다(리뷰 F12).** 일의 차례는 헤더 §8의 표다(리뷰가 다시 정했다 — `[B-7x1]`·`[B-7x2]`를 하나로 합치고, 데이터는 측정의 건너뜀·끈 짝(`[B-7x4m]`)과 글(`[B-7x4]`) 뒤에 `[B-7x5s]`·`[B-7x5c]`로 갈라 켠다). 줄이면 이렇다.",
    "다른 PC의 세션에 붙여 넣던 문구 파일은 이 브랜치에서만 쓰였고 지웠다(리뷰 F12, 결정 21). 일의 차례는 헤더 §8의 표다 — B6b가 끝난 `main` 위로 다시 쌓고(결정 20), `[B-7x1]`·`[B-7x2]`를 하나로 합치며, 데이터는 측정의 건너뜀·끈 짝(`[B-7x4m]`)과 글(`[B-7x4]`) 뒤에 `[B-7x5s]`(씨앗·노브)·`[B-7x5c]`(카드·사건)로 갈라 켜고, 노브를 고치면 `[B-7x5r]`을 둔다. B6b의 값과 어긋나면 B7x의 노브가 먼저 물러선다(결정 1). 줄이면 이렇다.")
rep("B6b가 끝난 `main` 위로 옮기고, 커밋마다 컴파일 → 자동화 테스트 두 번 → 기준값의 차례로 다듬는다. 확인할 자리는 노트 §5.",
    "B6b가 끝난 `main` 위로 옮기고, 커밋마다 컴파일 → 자동화 테스트 두 번 → 기준값의 차례로 다듬는다. 잎 이름(결정 3)은 `[B-7x1]`이 main에 들기 전에 코드·데이터의 자리를 모두 고친다(노트 §5.4 코7). 확인할 자리는 노트 §5.")
rep("3. **지도를 고치는 창구와 품음·탄생**: 헤더 §2.8·§2.9의 명세와 [개발자] 결정 6·14·15·16·22의 답 뒤에.",
    "3. **지도를 고치는 창구와 품음·탄생**: 헤더 §2.8·§2.9의 명세대로 — 결정 6·14·15·16·22는 2026-11-10에 답을 받았다: 레인의 단일 창구는 슬라이스 나에서, `SystemDragger`는 S14의 꼴(주기 노브 없음), 태어난 백성은 새 `CivID`와 새 이름과 0인 시간 셈을 받는 새 백성이고 그 줄은 옛 백성이 살아남았다거나 돌아왔다고 말하지 않는다, 새 추첨은 가·나에 없고 다의 포자에 고정 수 하나다. 슬라이스 나·다의 헤더가 남은 물음(헤더 결정 14의 (iii)~(v), 결정 6 (ㄷ)의 탄생 요청의 자리)에 먼저 답한다.")

assert '답 대기' not in out and '답을 기다' not in out
assert '\r' not in out
open(dst, 'wb').write(out.encode('utf-8'))
print(len(out.encode('utf-8')))
