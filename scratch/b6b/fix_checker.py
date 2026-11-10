"""Apply the post-review checker's fixes (F1-F10, C1-C8) and move text to the notes for room. Usage: python -I fix_checker.py"""
import re
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


def cut(text, start, end_line_prefix):
    """Return (text without the span, span) - the span runs from start to the end of the first line that begins with end_line_prefix."""
    a = text.index(start)
    b = text.index("\n" + end_line_prefix, a - 1) + 1
    b = text.index("\n", b)
    return text[:a] + "\x01" + text[b:], text[a:b]


def to_notes(span):
    span = span.replace("노트 §", "\x00")
    span = re.sub(r"(?<!헤더 )§(\d)", r"헤더 §\1", span)
    return span.replace("\x00", "§")


# --- header: F5, F6, F7, F10
h = rep(h, "'택함'이 '선택함'도 막는다", "'택함을 받은'이 '선택함을 받은'도 막는다")
h = rep(h, "`[B-6b9]`가 둘 가운데 하나를 1,200 아래로 내릴 때만 묻는다", "`[B-6b9]`가 둘 다 1,200 아래로 내릴 때만 묻는다")
h = rep(h, "셋을 처음 움직인다 — 아무도 읽지 않던 `AIFourthRungCap` 2를 판정이 읽고, B6a가",
        "넷을 처음 움직인다 — 아무도 읽지 않던 `AIFourthRungCap` 2를 판정이 읽고, `AIUpperRungProgressMultiplier` 1을 `[B-6b7]`이 시작값 0.375로, B6a가")
h = rep(h, "r = 1이면 다스리는 백성이 먼저 넷째 칸에 서는 판이 0.16~0.26이다(노트 §2 ⓐ·ⓒ)",
        "r = 1이면 다스리는 백성이 먼저 넷째 칸에 서는 판이 0.16이다(c 1e-4·P 1,200 — 노트 §2 ⓐ·ⓒ)")
h = rep(h, "r = 1의 0.16~0.26은", "r = 1의 0.16은")

# --- C1: the law step never gives up a law for a law
h = rep(h, "내려놓을 칙령을 고를 때 법은 맨 뒤이고 (1)은 법을 놓지 않는다.",
        "내려놓을 칙령을 고를 때 법은 맨 뒤이고 (1)은 법을 놓지 않는다. 법칙 걸음의 `MakeRoomFor`는 법을 놓지 않는다 — 빈 칸이나 법이 아닌 칙령을 놓을 때만 제정하고, 칸이 노브보다 적거나 답 칙령이 칸을 쥔 동안에는 서지 않는다(법끼리 틱마다 돌아 제정 값·칙령 줄·결정 수를 내지 않게 — `DecisionsOf`가 제정과 철회를 센다, `World/CosmicRulerPolicy.cpp:869`).")
h = rep(h, "AI는 칙령으로 답하지 않아 끊기지 않는다 — 그 몫은 노트 §2 ⓒ·ⓓ의 '법의 끊김'(결정 10).",
        "AI는 칙령으로 답하지 않아 끊기지 않는다 — 그 몫은 노트 §2 ⓒ·ⓓ의 '법의 끊김'(결정 10). AI의 법칙 걸음은 유지비로 끊긴 법을 다음 틱에 다시 내린다 — 그 수는 `race:` 줄이 세고, 크면 값으로 고친다.")

# --- C2: the one grace per run is shared by every people
h = rep(h, "행이 나가면 성계·생애·기록 줄 무엇도 잃지 않는다.",
        "행이 나가면 성계·생애·기록 줄 무엇도 잃지 않는다. 은혜는 백성을 가리지 않는다(조건은 `FlagNotSet Flag.Grace.Spent` 하나 — `Tools/data/DT_Events.json:245`) — 셋째 칸 이상 AI가 열한 배쯤 자주 맞으므로 판의 은혜는 대개 AI의 절멸이 먼저 쓰고, 다스리는 백성의 절멸·초신성에는 남지 않을 수 있다(`race:` 줄이 쓴 백성과 틱을 센다).")
h = rep(h, "절멸은 넷째 칸 뒤에도 거듭 와 다스리는 넷째 칸 백성은 판의 나머지 동안 이 죽음 앞에 다시 선다(결정 4).",
        "절멸은 넷째 칸 뒤에도 거듭 와 다스리는 넷째 칸 백성은 판의 나머지 동안 이 죽음 앞에 다시 선다(결정 4); 은혜는 백성을 가리지 않아 대개 AI의 절멸이 판의 한 번을 먼저 쓴다(§2.2의 1) — 대안: 다스리는 백성의 절멸만 은혜를 묻는다(권하지 않는다 — 은혜가 그릇의 백성의 몫이 되어 그 행의 '누구의 공로로도 기록되지 않았다'와 어긋난다).")

# --- C3: the lapsed-friendship line names a hidden people
h = rep(h, "다스리는 백성이 주인일 때 `log_siege_lost`·`log_siege_held`·`log_siege_lifted`). 상대가 다스리는 백성보다 `IsFarAbove`면 이름 없는 변형(`…_far` 다섯 — §3.1)을 고른다;",
        "다스리는 백성이 주인일 때 `log_siege_lost`·`log_siege_held`·`log_siege_lifted`), 그리고 **우호의 소멸**(외교 단계의 유지비 — `:1989-1999`, `log_stance_lapsed`; 첫째 칸 백성의 벗이 넷째 칸에 오른 뒤). 상대가 다스리는 백성보다 `IsFarAbove`면 이름 없는 변형(`…_far` 여섯 — §3.1)을 고른다;")
h = rep(h, "연구·외교는 그대로.", "연구는 그대로, 외교는 ★ 우호 소멸 줄의 이름 없는 변형 `log_stance_lapsed_far`(§2.9 — 말하기 한 수 그대로)뿐.")
h = rep(h, "`log_siege_held_far`, `log_siege_lifted_far` | `DT_LogTemplates`, `[B-6b3]`(결정 19) | 이름 없는 변형(§2.9) — 상수 다섯,",
        "`log_siege_held_far`, `log_siege_lifted_far`, `log_stance_lapsed_far` | `DT_LogTemplates`, `[B-6b3]`(결정 19) | 이름 없는 변형(§2.9) — 상수 여섯,")
h = rep(h, "`log_*_far` 다섯은", "`log_*_far` 여섯은")
h = rep(h, "(`log_*_far` 다섯)", "(`log_*_far` 여섯)")
h = rep(h, "로그 행 여덟", "로그 행 아홉")
h = rep(h, "공성의 시작과 끝 다섯 줄의 이름 없는 변형 `…_far`(§3.1)을", "공성의 시작과 끝 다섯 줄과 우호 소멸 한 줄의 이름 없는 변형 `…_far`(§3.1)을")
h = rep(h, "`Log.FarSiegeNameless`", "`Log.FarNameless`")
n = rep(n, "`Log.FarSiegeNameless`: 셋 이상 위의 AI가 에워싸거나 그 공성이 끝나면 `…_far` 다섯(헤더 §3.1)",
        "`Log.FarNameless`: 셋 이상 위의 AI가 에워싸거나 그 공성이 끝나거나 그 우호가 유지비로 사라지면 `…_far` 여섯(헤더 §3.1)")
n = rep(n, "로그 행 여덟(`log_record_extinct`·`log_death_extinction`·`log_fallen_bound`·`log_*_far` 다섯)",
        "로그 행 아홉(`log_record_extinct`·`log_death_extinction`·`log_fallen_bound`·`log_*_far` 여섯)")
n = rep(n, "넷째 칸 이상 AI가 세 칸 아래의 다스리는 백성을 에워쌀 때 처음 생긴다(결정 19의 이름 없는 변형).",
        "넷째 칸 이상 AI가 세 칸 아래의 다스리는 백성을 에워쌀 때 처음 생긴다(결정 19의 이름 없는 변형). 외교 단계의 우호 소멸(`:1989-1999` — 유지비를 못 낸 우호의 `log_stance_lapsed`, `{OtherCiv}` = 벗의 이름)도 격차를 묻지 않는다 — 첫째 칸 다스리는 백성의 벗이 넷째 칸에 오른 뒤 처음 생긴다(헤더 리뷰의 고친 뒤 확인이 찾았다 — §4). 명령이 내는 줄(`Player/PlayerRoleSubsystem.cpp:943`·`:986`·`:1013`·`:1070`)은 만난 백성만 고른다.")

# --- C4: the per-hazard cap lives in FleetDelayOf
h = rep(h, "읽는 둘: `FleetDelayOf`(`Sim/CosmicConflict.cpp:366-405`)와 신정론 검사(`Tests/CosmicDataTests.cpp:2257`의 `GuardCap`을 행마다 — 규칙을 베끼지 않는다). 패널의 지연 표지(`UI/CosmicSystemPanelWidget.cpp:1084`)는 그대로.",
        "`GuardDelayTicksOf`(`Sim/CosmicConflict.cpp:395-405`)는 그대로 `FleetDelayCap`만 묻고 — 행 없이 부르는 패널의 지연 표지(`UI/CosmicSystemPanelWidget.cpp:1084`)가 그대로 읽는다 — `FleetDelayOf`(`:366-392`)가 `min(GuardDelayTicksOf(…), GuardDelayCapOf(Rules, Row.CountdownTicks))`를 낸다; 신정론 검사(`Tests/CosmicDataTests.cpp:2257`의 `GuardCap`을 행마다 — 규칙을 베끼지 않는다)도 `GuardDelayCapOf`를 읽는다.")
n = rep(n, "`GuardDelayCapOf`(위기 행마다; 수도 규칙 행은 0 — `FleetDelayOf`가 그 행에 0을 주는 것과 같게)",
        "`GuardDelayCapOf`(위기 행마다; 수도 규칙 행은 0 — `FleetDelayOf`가 그 행에 0을 주는 것과 같게; `GuardDelayTicksOf`는 그대로, `FleetDelayOf`가 둘의 작은 쪽을 낸다)")

# --- C5: a lost bound system locks only while another holds it
h = rep(h, "③ 묶은 성계를 잃으면 그 백성은 잠긴다. `race:` 줄이 묶은 자의 종류와 잠김을 센다(§5.1);",
        "③ 묶은 성계를 남이 쥔 동안 그 백성은 잠기고, 되찾으면 풀린다(완성 기록이 만든 이를 지녀 `HasCompleted`가 견준다 — `Sim/CosmicMegaprojects.cpp:158-172`; 그 백성이 지워지면 판 끝까지). `race:` 줄이 묶은 자의 종류·잠김·되찾음을 센다(§5.1);")
h = rep(h, "묶은 성계를 잃으면 아무도)", "묶은 성계를 남이 쥔 동안 아무도 — 묶은 백성이 되찾으면 풀린다)")
n = rep(n, "잠김(결박한 성계를 잃음 — 아무도 다섯째 칸에 닿지 못한다;", "잠김(결박한 성계를 남이 쥠 — 그동안 아무도 다섯째 칸에 닿지 못한다, 되찾은 틱도 센다;")

# --- C6: two reverence values, two reasons
h = rep(h, "결과 줄은 탓 없이 사실을 말한 뒤 그 백성의 사람들이 일을 맡는 것으로 끝난다(규칙 2). 닿는 것:",
        "결과 줄은 탓 없이 사실을 말한 뒤 그 백성의 사람들이 일을 맡는 것으로 끝난다(규칙 2). 증언의 +0.1은 그릇 자신이 한 일의 결과(설계 표의 수, B1의 꼴)이고 결정 22의 +0.2는 백성의 견딤을 그릇에게 돌리는 값이라 둘은 다르다. 닿는 것:")

# --- C7: the binding words reach the reason line
h = rep(h, "`binding_words`(다섯째 칸 행의 글·`Rung.` 키·`Start.BindHint`·결박의 완성 줄",
        "`binding_words`(다섯째 칸 행의 글·`Rung.` 키·`Start.BindHint`·`Chronicle.Note.`(까닭 줄)·결박의 완성 줄")
n = rep(n, "`Rung.` 키·`Start.BindHint`·`project_binding.CompleteTemplates`에 없음", "`Rung.` 키·`Start.BindHint`·`Chronicle.Note.`·`project_binding.CompleteTemplates`에 없음")

# --- C8: Rung.TreatyOf can never show
h = rep(h, "새 키 열다섯(`Rung.Treaty`·`Rung.TreatyOf`·`Rung.Laws`", "새 키 열넷(`Rung.Treaty`·`Rung.Laws`")
n = rep(n, r"| 새 `Rung.Treaty`·`Rung.TreatyOf` | '조약을 맺은 백성 {0}/{1}' · '{Treaty}\|hpp(을,를) 맺은 백성 {0}/{1}' |",
        "| 새 `Rung.Treaty` | '조약을 맺은 백성 {0}/{1}'(업적 행의 `CivHasTreaty`는 Tag를 싣지 않아 — 헤더 §3.3의 6′ — 조약 이름의 조각 `Rung.TreatyOf`는 들지 않는다) |")

# --- notes: F1, F2, F3, F4, F9, F10, C1, C2
n = rep(n, "#122의 격차 거절(결정 19)", "#122의 격차 거절(결정 41)")
n = rep(n, "(결정 22 — §2.12의 표)", "(결정 22 — §0.4의 표)")
n = rep(n, "판단 규칙을 B4의 모형 구간으로 바꾸고(DECISIONS_LOG — B6a 결정 12의 규칙을 대신한다) 약 140시드로 판단: 닫거나 모형 구간 밖이면 [개발자]에게",
        "판단 규칙을 B4의 모형 구간으로 바꾸고(DECISIONS_LOG — B6a 결정 12의 규칙을 대신한다) 시험 칸 약 140시드 — 기대 약 9~12건이라 30건 아래면 '재지 않음 — 잰 N'(30건에는 약 450시드, §2 ⓕ); 30건 이상이고 모형 구간 밖이면 [개발자]")
n = rep(n, "(§1.3의 첫 목록·§1.6·§1.7·§2.2)", "(§0.4·§0.5·§1.2·§1.3의 첫 목록·§1.6·§1.7·§2.1·§2.2·§2.3)")
n = rep(n, "결정론과 저장의 표(§0.4 — 헤더 §2.12에서)", "결정론과 저장의 표와 같은 틱의 차례(§0.4 — 헤더 §2.12·§2.11에서)")
n = rep(n, "옮을 기준값(§2.2 — 헤더 §5.4에서),", "옮을 기준값(§2.2 — 헤더 §5.4에서), 묶음 B의 적기만과 C에서 읽는 것(§2.3 — 헤더 §5.3에서),")
n = rep(n, "`:779-785`(`ChangeReverence`의 `Borders` 키 — 결정 22의 대안일 때만)", "`:779-785`(`ChangeReverence`(㉮)나 `ChangeRelation`(㉯)의 `Borders` 키 — 결정 22의 대안일 때만)")
n = rep(n, "(0.16~0.26 — 노출이 열한 배라서)", "(0.16 — c 1e-4·P 1,200의 줄; 노출이 열한 배라서)")
n = rep(n, "(1)은 법을 놓지 않는다(변이 — 법을 먼저 놓으면).",
        "(1)은 법을 놓지 않는다(변이 — 법을 먼저 놓으면); 각성의 답이 칸을 쥔 동안 두 틱에 걸쳐 법이 다른 법을 위해 놓이지 않는다(변이 — 법을 법의 빈자리로 놓게 하면).")
n = rep(n, "다스리는 백성의 첫 절멸 틱.", "다스리는 백성의 첫 절멸 틱; 은혜를 쓴 백성(다스리는 백성·AI)과 그 틱.")

# --- F8: room for the answers - the tick table and the bundle-B/C reading move to the notes
h, table = cut(h, "| 단계 | 지금 | B6b | 시뮬 스트림 |", "| 열째·끝의 기록 |")
h = rep(h, "\x01", "표(단계마다 지금 · B6b · 시뮬 스트림)는 노트 §0.4의 둘째 표다(그 글이 이 절의 본문이다). 요지: 경제 끝의 셈 둘; 분쟁·AI (a)의 공성 줄과 외교의 우호 소멸 줄에 이름 없는 변형; 위기 (a)의 셋째 칸 위 지연과 결박의 끝; (b) 절멸의 만료 — 마지막 성계의 문 → `LetPeopleGo` → `KillPlayer` → `DeposeRuler` → 기록 줄; (c) 은하 법칙의 갈고리 ①; (d) 시대 행; AI (a)의 법칙 거두지 않음 → 법칙 걸음 → 조약 걸음; (b)의 제동; 건넘이 절멸의 백성을 지움; 앎 끝의 `enc_witness`; 열째 단계의 절멸의 죽음. 새 시뮬 수는 없다.")
h, record = cut(h, "- **적기만**(B — 칸은 노트 §2.1):", "- **적기만**(B — 칸은 노트 §2.1):")
h = rep(h, "\x01", "- **적기만과 C에서 읽는 것**(묶음 B의 적기만과 그 두 문턱 — AI 단계 µs/틱 +50%·결박한 성계를 잃어 닫힌 판 R 60의 3판; 묶음 C의 #136·#142·#132·B1 지킴·#129·#154·'< 15%'·시계 아래의 끝 — 알리되 C의 문이 아니다): 글은 노트 §2.3이다(그 글이 이 절의 본문이다 — 데이터를 켜기 전에 이 절과 함께 고정한다).")
h, reading = cut(h, "**C에서 읽는 것 — 알리되 C의 문이 아니다**(묶음 C):", "- **시계 아래의 끝**:")
h = rep(h, "\n\n\x01\n\n", "\n\n")
section = ("## 2.3 묶음 B의 적기만과 C에서 읽는 것 (헤더 §5.3에서 — 그 글이 헤더 §5.3의 본문이다; 데이터를 켜기 전에 이 글 그대로 고정한다)\n\n"
           + to_notes(record) + "\n\n" + to_notes(reading) + "\n\n")
n = rep(n, "## 3. 측정 기록", section + "## 3. 측정 기록")
n = rep(n, "## 0.5 답과 앞 결정이 부딪는 곳",
        "**같은 틱의 차례**(헤더 §2.11의 표 — 그 글이 헤더 §2.11의 본문이다; ★ = B6b의 새 걸음):\n\n" + to_notes(table) + "\n\n## 0.5 답과 앞 결정이 부딪는 곳")
h = rep(h, "(적대 리뷰의 고침과 함께 표·목록·긴 본문을 노트 §0.4·§0.5·§1.2·§1.3·§1.6·§1.7·§2.2로 옮겼다)",
        "(적대 리뷰의 고침과 그 확인과 함께 표·목록·긴 본문을 노트 §0.4·§0.5·§1.2·§1.3·§1.6·§1.7·§2.1·§2.2·§2.3으로 옮겼다)")

# --- status line and the notes record
h = rep(h, "→ 리드의 고침(57건 모두 — 일부 16은 검증자의 고침대로, 기록은 노트 §4).",
        "→ 리드의 고침(57건 모두 — 일부 16은 검증자의 고침대로, 기록은 노트 §4) → 고친 뒤 확인(읽기만 — 남은 고침 열과 빠진 것 여덟을 리드가 들였다, 노트 §4).")
n = rep(n, "- **고친 뒤 확인**: (다음 확인이 채운다 — 고친 자리와 옮긴 글의 인용을 HEAD에서 다시 연다.)",
        "- **고친 뒤 확인**(2026-11-08, 읽기만): 57건의 자리를 다시 열어 맞음; 번호·[개발자] 열넷·리뷰 수·모형의 수가 서로 맞고, 기댄 코드 줄(`ECosmicStartBlock`의 차례·`CrownKeyOf`의 빠짐없는 switch·`ChoiceGateOf`의 `EnactEdict`·`Place`가 읽는 둘·`EndingClockAt`·`Events.CheckIsPure`·스키마의 열거 칸·쿨다운 키)이 HEAD에서 맞다. 남은 고침 열(번호·절 가리킴·#142 행·노트 머리의 '본문' 목록·'택함을 받은'·③의 '둘 다'·노브 넷·답의 자리·㉯의 열쇠·r = 1의 0.16)과 빠진 것 여덟을 리드가 들였다: ① 법칙 걸음의 `MakeRoomFor`는 법을 놓지 않는다(법끼리 틱마다 돌아 결정 밀도의 판단을 소음으로 지나가게 하던 것 — `Sim.RulerEdictPolicy`에 한 경우) ② 은혜는 백성을 가리지 않아 대개 AI의 절멸이 먼저 쓴다(결정 5의 닿는 것과 권하지 않는 대안) ③ 우호의 소멸 줄이 가려진 벗의 이름을 부르던 것 — 여섯째 이름 없는 변형 `log_stance_lapsed_far`(테스트 이름은 `Log.FarNameless`) ④ 지킴의 상한은 `FleetDelayOf`에서 — `GuardDelayTicksOf`와 패널은 그대로 ⑤ 묶은 성계는 남이 쥔 동안만 잠그고 되찾으면 풀린다(결정 14) ⑥ 증언의 +0.1과 시대의 +0.2가 다른 까닭(결정 20) ⑦ `binding_words`를 까닭 줄(`Chronicle.Note.`)에도 ⑧ 보일 수 없는 `Rung.TreatyOf`를 뺐다(새 키 열넷). 크기: 개발자의 답을 적을 자리로 헤더 §2.11의 표를 §0.4로, §5.3의 적기만과 'C에서 읽는 것'을 §2.3으로 옮겼다.")

open(HP, "w", encoding="utf-8", newline="\n").write(h)
open(NP, "w", encoding="utf-8", newline="\n").write(n)
print("header", len(h.encode("utf-8")), "notes", len(n.encode("utf-8")))
print("--- moved refs")
for name, span in (("table", to_notes(table)), ("record", to_notes(record)), ("reading", to_notes(reading))):
    print(name, sorted(set(re.findall(r"(?:헤더 )?§[0-9][0-9.]*′?", span))))
