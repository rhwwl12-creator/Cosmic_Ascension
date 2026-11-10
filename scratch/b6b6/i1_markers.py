"""I1: the brief's section 5 markers that the data rows make false (each by exact text, once).
usage: i1_markers.py <project root>"""
import os, sys
root = sys.argv[1]
edits = {
    'docs/10_DESIGN/crisis_and_transcendents.md': [
        ("(그 위기의 대응 프로젝트 — B6b `[B-6b6]`부터)", "(그 위기의 대응 프로젝트 — B6b `[B-6b6]`)"),
        ("(B6b — 만료의 코드는 지금 코드, 행 `crisis_species_extinction`과 방주는 `[B-6b6]`부터; ",
         "(B6b — 만료의 코드와 행 `crisis_species_extinction`·방주 `project_ark`가 지금 코드와 데이터다(두 행은 `[B-6b6]`); "),
        ("(B6b 헤더 결정 40 — T3→T4 업적 행은 `[B-6b6]`부터이고 그 전에는 침묵 1체다)",
         "(B6b 헤더 결정 40 — T3→T4 업적 행 `feat_passage_by_ark`는 `[B-6b6]`에 들었고 그 전에는 침묵 1체였다)"),
        ("(§4.1 '절멸 위기' — `[B-6b6]`부터)", "(§4.1 '절멸 위기' — `[B-6b6]`)"),
        ("(B6b — 묻는 행은 `[B-6b6]`·`[B-6b7]`부터; 추첨·줄 없음",
         "(B6b — 조약을 묻는 행은 `[B-6b6]`에 들었고 은하 법칙을 묻는 행은 `[B-6b7]`부터; 추첨·줄 없음"),
        ("그 끝이 실제로 닿는 길은 넷째·다섯째 칸의 업적 행이 오는 B6b다 — `[B-6b6]`·`[B-6b7]`부터)",
         "그 끝이 실제로 닿는 길은 넷째·다섯째 칸의 업적 행이 오는 B6b다 — 넷째 칸 행은 `[B-6b6]`에 들었고 다섯째 칸 행은 `[B-6b7]`부터)"),
        ("절멸 위기의 만료다(B6b — 그 행은 `[B-6b6]`부터; 죽음의 둘째 출처",
         "절멸 위기의 만료다(B6b — 그 행 `crisis_species_extinction`은 `[B-6b6]`, `Data.Integrity`가 그 행 하나뿐임을 쥔다; 죽음의 둘째 출처"),
        ("(위 '사망'의 코드 한 곳 — 그 행 `crisis_species_extinction`은 B6b `[B-6b6]`부터;",
         "(위 '사망'의 코드 한 곳 — 그 행 `crisis_species_extinction`은 B6b `[B-6b6]`;"),
        ("지금 마지막 정착된 성계를 앗아 가는 길은 초신성 전조의 파괴 훅 하나다(B6b: 절멸의 만료 훅도 지금 코드이고 성계 수와 상관없이 문을 묻는다 — 그 행은 `[B-6b6]`부터, 위 절멸 위기 문단)",
         "지금 마지막 정착된 성계를 앗아 가는 길은 초신성 전조의 파괴 훅과 절멸의 만료 훅 둘이다(B6b: 절멸의 만료 훅은 성계 수와 상관없이 문을 묻는다 — 그 행은 `[B-6b6]`, 위 절멸 위기 문단)"),
    ],
    'docs/10_DESIGN/pillars.md': [
        ("(`feat_passage_by_ark` — B6b `[B-6b6]`부터, 아래)", "(`feat_passage_by_ark` — B6b `[B-6b6]`, 아래)"),
        ("T3→T4·T4→T5 하나씩(B6b — `[B-6b6]`·`[B-6b7]`부터)", "T3→T4·T4→T5 하나씩(B6b — T3→T4는 `[B-6b6]`에 들었고 T4→T5는 `[B-6b7]`부터)"),
    ],
    'docs/10_DESIGN/systems.md': [
        ("(B6b — 묻는 행은 넷째 칸으로 오르는 업적 행 `[B-6b6]`부터;", "(B6b — 묻는 행은 넷째 칸으로 오르는 업적 행 `feat_passage_by_ark`(`[B-6b6]`);"),
        ("방주 `project_ark` — B6b `[B-6b6]`부터, §4.1", "방주 `project_ark` — B6b `[B-6b6]`, §4.1"),
        ("절멸 위기의 만료 하나(B6b — 그 행은 `[B-6b6]`부터)", "절멸 위기의 만료 하나(B6b — 그 행은 `[B-6b6]`)"),
        ("오를 T3→T4 행은 `[B-6b6]`부터.", "오를 T3→T4 행 `feat_passage_by_ark`는 `[B-6b6]`."),
    ],
    'docs/15_GLOSSARY.md': [
        ("그 제동이 막을 셋째→넷째 칸 행은 `[B-6b6]`부터", "그 제동이 막는 셋째→넷째 칸 행 `feat_passage_by_ark`는 `[B-6b6]`"),
    ],
    'docs/20_ARCHITECTURE.md': [
        ("판정받는 행이 없는 칸(지금 게임 데이터에서는 셋째 칸부터 — 셋째 → 넷째 행은 `[B-6b6]`부터)",
         "판정받는 행이 없는 칸(지금 게임 데이터에서는 넷째 칸부터 — 셋째 → 넷째 행은 `[B-6b6]`에 들었고 넷째 → 다섯째 행은 `[B-6b7]`부터)"),
        ("지금의 게임에서 세 칸 위의 만난 백성은 나오지 않는다(T3→T4 행이 없어 백성은 셋째 칸까지다 — 보는 칸이 개인 칸인 방랑자·공백기(B5)에도 격차는 많아야 둘). 넷째 칸 백성이 생기는 셋째 → 넷째 업적 행(`feat_passage_by_ark`, B6b `[B-6b6]`)부터 나오고, 그때 방랑자·공백기의 보는 칸(개인 칸 — 늘 첫째)에서는 넷째 칸 이상의 백성이 모두 가려진다(다스리는 동안에는 다스리는 백성보다 세 칸 이상 위의 백성).",
         "세 칸 위의 만난 백성은 넷째 칸 백성이 생기는 셋째 → 넷째 업적 행(`feat_passage_by_ark`, B6b `[B-6b6]`)부터 나온다(그 전의 게임에서는 백성이 셋째 칸까지라 보는 칸이 개인 칸인 방랑자·공백기(B5)에도 격차는 많아야 둘이었다). 방랑자·공백기의 보는 칸(개인 칸 — 늘 첫째)에서는 넷째 칸 이상의 백성이 모두 가려진다(다스리는 동안에는 다스리는 백성보다 세 칸 이상 위의 백성)."),
        ("절멸 위기의 만료(B6b — 그 행은 `[B-6b6]`부터)는", "절멸 위기의 만료(B6b — 그 행은 `[B-6b6]`)는"),
        ("(B6b — 데이터 행이 켜지는 `[B-6b6]`부터):", "(B6b — 데이터 행이 켜진 `[B-6b6]`부터):"),
        ("`Hazard.Extinction`)의 만료(B6b — 행은 `[B-6b6]`부터)는", "`Hazard.Extinction`)의 만료(B6b — 행은 `[B-6b6]`)는"),
        ("(두 단계의 코드는 그대로 — 그 절멸의 행은 `[B-6b6]`부터)", "(두 단계의 코드는 그대로 — 그 절멸의 행은 `[B-6b6]`)"),
    ],
    'docs/21_DATA_MODEL_REFERENCE.md': [
        ("위기 행은 `[B-6b6]`부터", "위기 행은 `[B-6b6]`"),
        ("데이터에 T3→T4 판정 행이 아직 없다 — 행은 `[B-6b6]`)", "T3→T4 판정 행은 `feat_passage_by_ark`다 — `[B-6b6]`)"),
        ("밸런스 커맨드렛의 `race:` 줄(B6b `[B-6b3b]` — 측정: 절멸이 놓인 수·방주·만료·그것이 죽인 통치자의 셈)이 읽고",
         "밸런스 커맨드렛의 `race:` 줄(B6b `[B-6b3b]` — 측정: 절멸이 놓인 수·방주·만료·그것이 죽인 통치자의 셈)과 `Data.Integrity`의 코드 죽음 핀(B6b `[B-6b6]`, 결정 5 — 게임 행 가운데 참인 것은 `crisis_species_extinction` 하나이고 그 만료 효과 세 칸은 비고 갈래 확률은 0이다)이 읽고"),
    ],
    'docs/40_DEV_GUIDE.md': [
        ("; 게임에는 절멸 행이 `[B-6b6]`까지 없어 테스트의 절멸 250·방주 150을 얹은 사본을 게임의 규칙으로도 본다(변이 `b6b2c_data_ark_margin_half` — 배율 0.5면 300틱)",
         "; 게임의 절멸 행과 방주(`[B-6b6]`)를 그대로 보고, 테스트의 절멸 250·방주 150을 얹은 사본으로 가장자리를 게임의 규칙으로도 본다(변이 `b6b2c_data_ark_margin_half` — 배율 0.5면 300틱)"),
        ("지금 데이터에서는 `project_binding` 하나만 뺀다(`[B-6b6]`이 제 행과 함께 고칠 일부러 둔 덫 — 변이 `b6b3b_without_b6b_keeps_binding`)",
         "지금 데이터에서는 `[B-6b6]`의 세 행(`feat_passage_by_ark`·`project_ark`·`crisis_species_extinction`)과 `project_binding`을 뺀다(`[B-6b7]`·`[B-6b8]`이 제 행과 함께 고칠 덫 — 변이 `b6b3b_without_b6b_keeps_binding`)"),
        ("`KillPlayersOf`가 비는 것은 `WithoutDeath` 제 핀) |",
         "`KillPlayersOf`가 비는 것은 `WithoutDeath` 제 핀). B6b `[B-6b6]`(결정 5): 코드의 죽음 하나 — 게임의 위기 행 가운데 `KillsTheRulerOf`가 참인 것은 정확히 `crisis_species_extinction`이고 그 행의 만료 효과 세 칸은 비고 `BranchChance`는 0이다(다른 죽음의 출처는 새 결정이다 — 변이 `b6b6_kills_ruler_not_extinction`, 에셋을 다시 짓는 `b6b6_data_extinction_expiry_effect`) |"),
        ("절멸 만료의 통치자 죽음은 코드라 남는다 — B6b 결정 5, 그 행은 `[B-6b6]`부터)", "절멸 만료의 통치자 죽음은 코드라 남는다 — B6b 결정 5, 그 행은 `[B-6b6]`)"),
    ],
}
multi = {('docs/21_DATA_MODEL_REFERENCE.md', "행은 `[B-6b6]`부터"): "행은 `[B-6b6]`"}
for rel, pairs in edits.items():
    p = os.path.join(root, rel)
    s = open(p, 'rb').read().decode('utf-8')
    assert '\r' not in s, rel
    for old, new in pairs:
        assert s.count(old) == 1, (rel, s.count(old), old[:60])
        s = s.replace(old, new)
    for (mrel, old), new in multi.items():
        if mrel == rel:
            n = s.count(old)
            print(rel, 'replace-all', n, old)
            s = s.replace(old, new)
    open(p, 'wb').write(s.encode('utf-8'))
    print('edited', rel, len(pairs))
