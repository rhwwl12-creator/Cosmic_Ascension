"""[B-6b8] I1: the data docs - docs/21 section 7.3 (the witness lists, Rules W and E) and the markers that name the two
rows (docs/10_DESIGN, docs/20, docs/21), present tense now that the rows are in data. Exact single replacements, LF kept.
usage: i1_docs.py <project root>"""
import sys

root = sys.argv[1]
EDITS = {
    "docs/21_DATA_MODEL_REFERENCE.md": [
        ("방주 행의 `row` 규칙은 이름에 rescue_words·title_words·holy_and_myth·self_rescue_words를, 완성 줄에 self_rescue_words·holy_and_myth를 막는다; holy_and_myth의 끝에 '노아'가 든다. ",
         "방주 행의 `row` 규칙은 이름에 rescue_words·title_words·holy_and_myth·self_rescue_words를, 완성 줄에 self_rescue_words·holy_and_myth를 막는다; holy_and_myth의 끝에 '노아'가 든다. "
         "B6b `[B-6b8]`의 증언 낱말 셋(결정 20, 헤더 §2.8, 노트 §1.6 규칙 7) — witness_words(증언·기록함·전함·말함과, 떠남·지나감·외면·도망·부끄러움의 물러섬; "
         "줄어든 과거꼴 '기록했'·'떠났'·'지나갔' 같은 것은 따로 적는다 — 글자 그대로 찾으므로 '기록하'는 '기록했다'를, '떠나'는 '떠났다'를 찾지 못한다)·sign_voice_words(징조의 목소리)·"
         "witness_record_words('기록') — 는 `DT_Encounters`의 행 규칙 하나에만 걸린다: `when`이 접두어 `enc_`와 `Hazard.Claim`을 이름한 조건(`has_cond` — 주장 곁의 방랑자 카드, 지금 `enc_witness`)이고, "
         "witness_words는 글 칸 여섯(`Title`·`Body`·`VoiceBodies`·`Choices.Label`·`Choices.SuccessText`·`Choices.FailureText`)에, sign_voice_words는 `Body`에, "
         "witness_record_words는 선택지의 성공 줄 밖 다섯 칸에 건다('기록'은 성공 줄에만 — 칸 시험은 선택지를 가르지 못하므로 '물러난다'의 성공 줄은 글 감사가 지킨다; 더 긴 낱말 속의 헛걸림은 이 좁은 범위에서 받아들인다). "
         "같은 커밋의 `DT_Events` 행 규칙은 `CivPressureEnduredTicksReached`를 묻는 행에 조건 `SystemOwnedBy` `Ruled`와 효과 `ChangeReverence` `Borders`를 함께 요구한다(결정 22의 꼴 — `Ruled`가 없으면 AI 백성의 견딤마다 이웃의 경외가 오른다). "
         "단위 테스트 `test_the_witness_words`가 세 목록을 그대로 고정하고 두 규칙을 픽스처와 게임의 두 행에서 깬다. "),
        ("문명 주제 행의 성계는 수도라 '다스리는 백성이고 수도가 정착됐다'가 된다. 처음 싣는 행은 event_era_endured(`[B-6b8]`).",
         "문명 주제 행의 성계는 수도라 '다스리는 백성이고 수도가 정착됐다'가 된다. 처음 실은 행은 event_era_endured(`[B-6b8]` — 견딘 압력을 묻는 행에는 스키마가 Ruled를 요구한다, §7.3 '스키마의 꼴')."),
    ],
    "docs/20_ARCHITECTURE.md": [
        ("시대 행과 `enc_witness`(B6b — `[B-6b8]`부터)는 데이터다", "시대 행과 `enc_witness`(B6b `[B-6b8]`)는 데이터다"),
        ("방랑자의 증언 카드 `enc_witness`는 데이터라(`[B-6b8]`부터)", "방랑자의 증언 카드 `enc_witness`는 데이터라(`[B-6b8]`)"),
    ],
    "docs/10_DESIGN/crisis_and_transcendents.md": [
        ("주장을 끝내지도 늦추지도 않는다; B6b `[B-6b8]`부터, §4.2 '방랑자:')", "주장을 끝내지도 늦추지도 않는다; B6b `[B-6b8]`, §4.2 '방랑자:')"),
        ("방랑자 Mind 판정의 분별은 첫째 칸 방랑자의 증언 카드가 맡는다(아래 '방랑자:' 문단 — B6b `[B-6b8]`부터).",
         "방랑자 Mind 판정의 분별은 첫째 칸 방랑자의 증언 카드가 맡는다(아래 '방랑자:' 문단 — B6b `[B-6b8]`)."),
        ("주장 곁의 서 있는 카드 `enc_witness`(B6b `[B-6b8]`부터)는", "주장 곁의 서 있는 카드 `enc_witness`('그늘의 가장자리' — B6b `[B-6b8]`)는"),
        ("[증언]은 정신 판정 3이고(설계의 낱말 — 화면의 라벨은 몸이 하는 일의 이름으로 글 감사가 정한다)",
         "[증언]은 정신 판정 3이고(설계의 낱말 — 화면의 라벨은 몸이 하는 일의 이름으로 글 감사가 정한 '그늘의 가장자리를 재어 둔다'이고, 카드의 글은 증언·기록함·말함·떠남을 쓰지 않는다 — 스키마의 증언 낱말, `docs/21` §7.3)"),
        ("셈·조건·선택자·열쇠는 지금 코드, 행은 `[B-6b8]`부터;", "셈·조건·선택자·열쇠는 지금 코드, 행은 `[B-6b8]`의 데이터;"),
        ("한 줄 \"{Civ}의 백성이 이 시대를 '…의 시대'라 부르기 시작했다.\"를 낸다 — 변형마다 제 시대가 있고 `{Epithet}`이 없다.",
         "한 줄 \"{Civ}의 백성이 이 시대를 '…의 시대'라 부르기 시작했다.\"를 낸다 — 변형마다 제 시대가 있고(물레·장독·느린 수레) `{Epithet}`이 없다. 스키마가 이 꼴(`Ruled`와 `Borders`)과 시대의 낱말을 지킨다(`docs/21` §7.3)."),
        ("방랑자의 증언(첫째 칸의 카드 — B6b `[B-6b8]`부터)", "방랑자의 증언(첫째 칸의 카드 `enc_witness` — B6b `[B-6b8]`)"),
    ],
    "docs/10_DESIGN/pillars.md": [
        ("주장 곁의 증언 카드 `enc_witness`는 B6b `[B-6b8]`부터, §4.2 '방랑자:'", "주장 곁의 증언 카드 `enc_witness`는 B6b `[B-6b8]`, §4.2 '방랑자:'"),
    ],
    "docs/10_DESIGN/systems.md": [
        ("**지금의 방랑자 카드 다섯**(B1, 2026-10-25):", "**지금의 방랑자 카드 여섯**(B1, 2026-10-25; 주장 곁의 증언 카드는 B6b `[B-6b8]`, 2026-11-10):"),
        ("— 위기마다의 뜻은 §4.1 표의 W 열.",
         "— 위기마다의 뜻은 §4.1 표의 W 열. 주장 곁의 서 있는 카드 '그늘의 가장자리'(`enc_witness` — 첫째 칸 방랑자의 증언, §4.2 '방랑자:')도 주장이 선 성계에 그릇이 설 때 오지만 결과가 다르다: 정신 판정 3에 성공하면 그 성계 주인의 경외가 0.1 오르고(위 넷의 0.3이 아니다) 주장을 끝내지도 늦추지도 않으며, 실패와 '물러난다'는 효과가 없다; 카드의 글은 증언·기록함·말함·떠남을 쓰지 않는다(스키마의 증언 낱말 — `docs/21` §7.3)."),
        ("B1 밖(아직 코드에 없다): 주장의 증언 카드 `enc_witness`(첫째 칸 방랑자의 카드 — B6b `[B-6b8]`부터, §4.2 '방랑자:'), 성역의 경계 카드 '지경'(B8)",
         "B1 밖(아직 코드에 없다): 성역의 경계 카드 '지경'(B8)"),
    ],
}
for rel, edits in EDITS.items():
    path = root + "/" + rel
    data = open(path, "rb").read()
    assert b"\r" not in data, rel
    text = data.decode("utf-8")
    for old, new in edits:
        assert text.count(old) == 1, (rel, text.count(old), old[:60])
        text = text.replace(old, new)
    open(path, "wb").write(text.encode("utf-8"))
    print("edited", rel, len(edits))
