"""[B-6b6] final stage: complete the notes' section 5 block, add the header section 11 line and the DECISIONS_LOG line.
usage (project root): python -I -X utf8 final_notes.py"""
import re

N = 'docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race_notes.md'
H = 'docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md'
D = 'docs/60_DECISIONS/DECISIONS_LOG.md'


def load(p):
    b = open(p, 'rb').read()
    assert b'\r' not in b, p
    return b.decode('utf-8')


def save(p, t):
    open(p, 'wb').write(t.encode('utf-8'))


def once(t, old, new):
    assert t.count(old) == 1, old[:80]
    return t.replace(old, new)


n = load(N)

COMMENT = ("<!-- [B-6b6] fix stage: the rest of this block (나눔, 코드, 데이터, 스키마, the mutation counts, 기준값, items 1-6 and "
           "8-11) is the final stage's; it removes this comment. -->\n")
HEAD4 = (
    "- **나눔**: 없음(헤더 §8의 `[B-6b6]` 한 커밋 그대로). 설계 워크플로가 쓴 지침서(저장소 밖 — 이 노트는 그 절 번호를 쓰지 않는다)를 "
    "구현 일꾼 둘이 나눠 맡았다 — 데이터(행 셋·태그 설명·스키마와 검사기 테스트·파이썬 변이)와 C++(행 이름 머리의 주석·테스트·C++ 변이 파일). "
    "이어 측정(빌드 판·ⓐ ⓑ ⓒ·묶음 A·다시 셈) → 관점 다섯의 리뷰 → 반박 검증 → 고침 일꾼 → 마지막 단계(러너 `-Tier full -Data -UpdateBaseline` "
    "한 번·변이·기록).\n"
    "- **코드**: Core Logic과 표현 계층의 소스는 그대로다 — `Source/`에서 바뀐 것은 `Tests/`뿐이다: `Tests/CosmicDataTests.cpp`(결정 5의 코드 "
    "죽음 핀·`WithoutB6bData` 걸림줄·방주 여유의 주석 — 아래 「테스트」), `Tests/CosmicDeathTests.cpp`(아래 ⑥), `Tests/CosmicAscensionTests.cpp`"
    "(아래 ⑦), `Tests/CosmicSimTests.cpp`(아래 「테스트」), 머리 `Tests/CosmicB6bRowNames.h`의 주석(아래 ②), 주석만 고친 "
    "`Tests/CosmicTestHelpers.h`·`Tests/CosmicUpperRungTests.cpp`·`Tests/CosmicWarningLineTests.cpp`·`Tests/CosmicHazardTests.cpp`. 머리가 "
    "바뀌어 두 타깃을 모두 다시 지었다. 그 밖에 `Config/DefaultGameplayTags.ini`의 `Hazard.Extinction` 설명(ASCII — 행·답·은혜의 문·"
    "`LetPeopleGo`·죽음과 폐위·건넘 단계의 지움)과 `Tools/run_checks.ps1`의 방랑자 주석.\n"
    "- **데이터**: 행 셋을 표의 끝에 — `DT_Hazards`의 `crisis_species_extinction`(`Hazard.Extinction`, `Draw`, 틱당 c 1e-4, 250틱, 조건 "
    "`CivTierAtLeast` 3, 효과 없음, `RulerResolutions` `[project_ark]`, 방랑자의 답 없음 — 5 → 6행; 새 몫은 추첨 몫의 끝에 붙어 앞 행들의 "
    "추첨이 그대로다), `DT_Megaprojects`의 `project_ark`(셋째 칸, 진척 150, 에너지·물질 3씩 — 아래 ③; 6 → 7행), `DT_Feats`의 "
    "`feat_passage_by_ark`(셋째 → 넷째 칸, 통치자 길, 조건 `MegaprojectComplete project_ark`·`CivHasTreaty` 1, 값 `ShiftEthics Openness` "
    "−0.1 — 3 → 4행). 글은 §1.5에서 글자 그대로 옮겼다(리뷰가 칸마다 견주었다 — 어긋남 0; '방주'는 방주 행과 넷째 칸 업적 행에만). "
    "`make_data_tables` 뒤 바뀐 에셋은 `DT_Hazards`·`DT_Megaprojects`·`DT_Feats`의 `.uasset`과 도장 `generated.sha1`뿐이다(`ST_CA` 그대로).\n"
    "- **스키마**: 새 목록 `ark_words`('방주' — 결정 33)와 `holy_and_myth` 끝의 '노아'; 여섯 열의 `forbidden`에 `ark_words` — `DT_Hazards`의 "
    "이름(문자열 `\"verdict\"`가 배열이 됐다)과 발생·만료·해결 줄, `DT_Events.LogTemplates`, `DT_LogTemplates.Templates`; 행 규칙 셋 — 방주의 "
    "행 규칙(`row` `project_ark`: 이름에 `rescue_words`·`title_words`·`holy_and_myth`·`self_rescue_words`, 완성 줄에 `self_rescue_words`·"
    "`holy_and_myth`가 없다 — 테스트 여섯, 아래 ④)과 `row_not` 둘(`project_ark` 밖의 프로젝트의 이름·완성 줄, `feat_passage_by_ark` 밖의 업적의 "
    "제목·서사·줄에 `ark_words`가 없다). 검사기 테스트: `fixture_tables`에 `project_ark`와 셋째 → 넷째 칸의 `feat_passage_by_ark`, "
    "`rule_setups`에 새 규칙 셋, 걷는 수 305 → 316, 새 메서드 `test_the_ark_and_its_word`(목록의 핀, 새 자리마다 깨는 경우, '…방주로 백성을 "
    "건져 냈다'·'노아의 배'·두 `row_not`의 경우, 두 예외 행의 통과, 원본 전체에서 '방주'가 두 행 밖에 서지 않음과 `DT_Encounters`의 Body·"
    "`ST_CA.csv`에 넣은 두 실패 경우 — 아래 ⑤), 그 열들의 `forbidden`을 쥔 정확한 핀(`assertEqual`)을 새 목록으로. 검사기 단위 테스트 "
    "94 → 95개, `check_data` 0.\n")
n = once(n, COMMENT, HEAD4)

n = once(n, "- **테스트**(고침 단계까지): `Sim.Determinism`(",
         "- **테스트**(자동화 테스트 223개 그대로 — 새 테스트 없이 단언을 넓혔다; 마지막 러너의 두 판 446 Success·실패 0): `Data.Integrity` — "
         "결정 5의 코드 죽음 핀(게임의 위기 행 가운데 `KillsTheRulerOf`가 참인 것이 정확히 `crisis_species_extinction`이고 그 만료의 효과 세 칸이 "
         "비고 `BranchChance` 0; `KillPlayersOf`의 핀 그대로), `WithoutB6bData`의 걸림줄(빼는 행 넷 — `feat_passage_by_ark`·`project_ark`·"
         "`project_binding`·`crisis_species_extinction`, 도우미의 `Note` 차례; 열 행의 사본 검사 그대로), 방주의 여유는 이제 게임 행에서도 "
         "문다(150 / 1.0 < 250 — 주석만 고쳤다). §1.3 표의 테스트: `Sim.EndingRecordOnce`·`Sim.EndingInert`·`Sim.SightWalkMoved`·"
         "`Sim.SuccessionPossible`은 그대로 지났다(`WithoutB6bData`가 맨 바깥 — 새는 곳 없음); `Sim.NotablesFromEventsOnly`·`Sim.WandererLoop`"
         "(시드 4)·`Sim.RealmCap`·`Sim.CivAIProjects`·`Sim.NotablesCapped`·`Sim.PhaseTwoExit`(시드 18)·`Sim.StartBlockReasons`(훑기에 "
         "`project_ark`가 든다)·`UI.TopBarAndPanel`·`UI.NextRung`의 9b도 기대값과 시드를 바꾸지 않고 지났다; 셋째 칸 이상 백성을 게임의 업적·"
         "위기로 돌리는 그 밖의 테스트에 `WithoutB6bData`를 더할 일은 없었다. `Sim.Determinism`(")

n = once(n, "- **변이**(고침 단계가 더한 것 — 셈과 결과는 마지막 단계): ",
         "- **변이**(모두 깨졌다): 새 `b6b6_` 열넷 — 지침서의 열둘(C++ 둘 `kills_ruler_not_extinction`·`brake_always_shut`, 에셋을 다시 짓는 "
         "데이터 하나 `data_extinction_expiry_effect`, 파이썬만의 아홉 — 스키마 여섯·데이터 셋)과 B4의 둘을 다시 뜬 것(`b6b6_schema_myth_takes_baro`·"
         "`b6b6_schema_verdict_without_soul_has_soul` — '노아'가 `holy_and_myth` 줄의 맥락을 옮겼다; 옛 행에 '(no longer applies since "
         "[B-6b6] …)'); 제자리에서 다시 만든 `[B-6b5]`의 열은 아래 ⑬. `b6b6_data_extinction_expiry_effect`는 지침서의 꼴(스키마 규칙 5를 "
         "늦추고 행에 효과 하나)로는 에셋에 닿지 못했다 — `make_data_tables`가 쓰기 전에 검사기 단위 테스트도 돌리므로(규칙 5의 깨는 경우 셋과 "
         "걷는 수) 에셋이 그대로였고 `Data.Integrity`가 지났다(마지막 단계의 첫 판 — 변이가 아니라 패치의 결함); 패치가 그 테스트의 세 경우를 빼고 "
         "걷는 수를 312로 두게 고쳐 다시 돌렸다 — 에셋이 쓰였고 새 절이 깼다(README 행도 고쳤다). 다시 돌림: 파이썬 89 판(패치와 테스트의 짝 — "
         "고침 단계가 이 바이트에서, 89/89)과 C++·에셋 62 패치(새 셋과 옛 쉰아홉; 62/62 깨짐) — 옛 것은 이 커밋이 고친 파일"
         "(`Tests/CosmicTestHelpers.h`·`Tests/CosmicDataTests.cpp`·데이터 셋)의 패치, 이 커밋이 몸을 고친 테스트(`Data.Integrity`·"
         "`Calling.DeathPipeline`·`Feats.Reachable`·`Sim.Determinism`)를 깨는 패치, 고침 단계가 더한 열다섯이다. 다시 돌림 목록(아래 문단)의 "
         "'고친 테스트 파일의 모든 테스트' 가운데 C++ 102 패치는 돌리지 않았다(일의 속도 — 같은 바이트의 같은 검증; `[B-6b12]`의 B6b 전체 재실행에 "
         "든다): 주석만 바뀐 `Tests/CosmicUpperRungTests.cpp`의 64(판들이 `DT_Hazards`·`DT_Megaprojects`를 제 표로 바꾸고 `DT_Feats`를 비워 "
         "게임의 새 행이 닿지 않는다 — 지침서가 이름한 것과 다르다)와 `Tests/CosmicWarningLineTests.cpp`의 13(`UI.ClaimMayFollow` 아홉 포함 — "
         "제 표가 게임의 위기 표를 바꾼다), `Tests/CosmicDeathTests.cpp`의 `Calling.DeathPipeline` 밖 25(조용한 사본은 추첨 몫이 0이라 절멸이 "
         "서지 않고 테스트 코드도 그대로다). 마지막 단계의 `apply_after`(아래 「기록」 앞)에서 `[H'']` 대비 '서다 → 서지 않다'는 다시 뜬 둘과 제자리의 "
         "열뿐이다. 고침 단계의 기록: ")

BASE = (
    "- **기준값**(옮았다 — B6b에서 처음으로 입력 없는 세계가 옮는다): 3,000틱 2816687688 / 3300228750 → 2557850423 / 2906170550, 통치자 "
    "3,000 3561771757 / 2462331303 → 1189870756 / 3359656751, 방랑자 6,000 2665013209 / 2229239127 → 271068805 / 309834531, 20,000틱 "
    "2359424635 / 726895141 → 3168555397 / 1640870674, 통치자 20,000 1611658162 / 29194274 → 4053448003 / 129517356; 해시 판 둘(게임 "
    "프로세스)이 커맨드렛의 3,000틱 줄과 같다(2557850423 / 2906170550, 1189870756 / 3359656751). 마지막 러너 `-Tier full -Data "
    "-UpdateBaseline` 한 번(그 안의 두 판이 '테스트 2회 연속' — 223개씩)이 다섯 줄을 다시 적었고 측정 단계의 `Saved\\Balance\\b6b6_on\\` "
    "다섯과 글자 그대로 같다; 그 뒤 `Tools/baselines/seed42.json`을 HEAD의 꼴(들여쓰기 4·LF·BOM 없음)로 되돌리고 "
    "`last_recorded_in_changelog`를 손으로 적었다. ⓐ 같은 빌드에서 러너의 인자 그대로 + `-WithoutB6bData=1`의 다섯 판이 `[B-6b5]`의 다섯 "
    "줄을 글자 그대로 냈다(`wrote …` 꼬리를 뺀 열쇠; 다섯 로그 모두 `rows taken out [feat_passage_by_ark project_ark project_binding "
    "crisis_species_extinction]` 한 줄). ⓑ 다섯 on 판의 T(첫 셋째 칸 — 첫 AI 셋째 칸 1,521, 방랑자 판 1,558)가 시작 상태의 base 판과 같고, "
    "Tick ≤ T의 CSV·`_rows.csv` 행이 `TickMicros`·`SaveBytes`·`SaveRawBytes` 밖의 모든 칸에서 같다(주장마다의 CSV는 틱 칸이 없어 놓인 틱으로 "
    "본다 — on 쪽에만 있는 주장은 모두 T 뒤). ⓒ 측정의 255 프로세스와 마지막 러너의 다섯 판·해시 판 둘 모두 `CosmicBalance end check`·"
    "`race check` 줄 0(`race check` 0은 아래 ⑨). 판마다 먼저 움직인 것: 다섯 판 모두 AI 조약 걸음(첫 조약 입력 없음 1,901·통치자와 방랑자 "
    "1,935; 처음 다른 CSV 표본 2,000은 두 백성의 `Treaties`와 청한 백성의 `Influence` −50만 다르다). 절멸: 3,000틱 입력 없음 — AI에 하나"
    "(`_rows.csv`의 첫 절멸 행 표본 3,000); 통치자 3,000 — 없음; 방랑자 6,000 — `ExtinctionOnRuledFirst` −1(다스리는 백성에 없음, AI에 15; "
    "말한 첫 절멸 행 표본 3,100 — `-TellAll=0`이라 둘은 말하지 않았고 그 판의 첫 놓임은 재지 않음); 입력 없음 20,000 — AI에 90(첫 행 표본 "
    "3,000); 통치자 20,000 — `ExtinctionOnRuledFirst` 5,018(다스리는 백성에 11, AI에 89; `_rows.csv`의 3,600은 첫 AI 놓임의 표본). 통치자 "
    "3,000의 정책 줄: 카드 16 → 17·답 16 → 17·시작 4 → 5·칙령 바꿈 4 → 5·외교 바꿈 15 → 17·결정 44 → 49(둘째 칸 1,175·셋째 칸 2,375 그대로); "
    "방랑자 6,000: 즉위 없음(3,121의 `civ_7` 즉위가 사라졌다 — 거절 0), 여정 96 → 219. `[B-6b7]`에 넘김: §3 '묶음 A'의 마지막 줄(다섯 판의 "
    "`race:` 둘째 칸 — 지평 안의 AI 넷째 칸).\n")
n = once(n, "- **균형 관점의 판정**(헤더 §4.6의 물음", BASE + "- **균형 관점의 판정**(헤더 §4.6의 물음")

ITEMS1 = (
    "- **헤더와 다르게 하거나 더한 것**: ① 첫 갈림 — 헤더 §3.1 표의 '절멸이 처음 놓인 틱부터만'은 추첨 표만의 말이다: 세계는 첫 셋째 칸 T + 1부터 "
    "갈릴 수 있고(위기 단계에 놓인 절멸이나 AI 단계의 조약 걸음 — 판정받는 행이 `CivHasTreaty`를 물으면 선다; 먼저 오는 것), 기준값 다섯 판 모두 "
    "조약 걸음이 먼저였다(위 「기준값」; 묶음 A의 쌍에서는 R 24/30·입력 없음 27/30·방랑자 49/60 — §3 '묶음 A'). T 이하는 비트 그대로다(ⓑ). "
    "헤더 §11에 한 마디. ② `CosmicRows::` 상수를 두지 않았다 — 세 행을 이름으로 부르는 게임 코드가 없고(만료 훅은 태그를, 업적과 방주는 표를 "
    "걷는다) 상수는 검사기의 `required_rows`를 부른다; 이름은 `Tests/CosmicB6bRowNames.h`에 남고 그 머리 주석이 '데이터 커밋이 이름을 옮긴다'를 "
    "'이름으로 부르는 코드의 커밋이 옮긴다'로 고쳤다. ③ 방주의 비용 에너지·물질 3씩(헤더에 값이 없다) — `project_stabilize`의 2/2에 칸 배수 "
    "×2.2/×1.5를 곱한 2.93을 올림; 인과 0(수입 0), 영향 0(조약 걸음의 50과 겨루지 않는다); 균형 관점이 3/3을 유지했다(위). ④ 방주의 행 "
    "규칙에 `[B-6b5]`가 넘긴 넷(완성 줄의 `self_rescue_words`, 이름의 `rescue_words`·`title_words`·`holy_and_myth`) 밖의 둘 — 이름의 "
    "`self_rescue_words`와 완성 줄의 `holy_and_myth`. ⑤ '방주'의 지킴을 둘로 나눴다 — `check_data`의 단단한 문(여섯 열의 `forbidden`과 "
    "`row_not` 둘 — 에셋을 쓰기 전에 거절한다)과 검사기 단위 테스트의 원본 전체 걸음(두 행 밖의 모든 DT 칸과 `ST_CA.csv` — 깨지면 러너가 "
    "실패를 적고 1로 끝나 커밋을 막는다; 그 변이 `b6b6_data_ark_word_in_card`는 'test_data_schema.py만'). ⑥ `Calling.DeathPipeline`의 "
    "조용한 사본(`QuietHazards`)이 게임의 위기 행을 모두 베끼므로 게임의 `crisis_species_extinction`과 테스트의 절멸 행이 한 태그에 둘로 설 "
    "뻔했다 — 테스트 행을 넣기 전에 `RemoveRow`로 뺀다(지침서가 미리 고친 것; `docs/40`의 그 행). ⑦ `Feats.Reachable`")
n = once(n, "- **헤더와 다르게 하거나 더한 것**(고침 단계까지 — ①~⑥·⑧~⑪은 마지막 단계): ⑦ `Feats.Reachable`", ITEMS1)

ITEMS2 = (
    " ⑧ 이제 거짓이 된 '`[B-6b6]`부터'의 표지를 '들었다'로 고쳤다 — `docs/10_DESIGN`의 세 문서와 `docs/15`·`docs/20`·`docs/21`·`docs/40`·"
    "`docs/70`(행이 코드에 든 자리만; `[B-6b7]`·`[B-6b8]`의 표지는 그대로); `docs/21`의 `KillsTheRulerOf`를 읽는 곳과 `docs/40`의 "
    "`Data.Integrity` 행에 코드 죽음의 핀을 더했다. ⑨ `race check` 0은 경계마다 넷째 칸 수를 묻는 배선에 기댄다 — 이 커밋은 그 배선을 "
    "시험하지 않는다(커맨드렛의 상한을 하나 낮춘 한 번의 시험은 `[B-6b7]` — 위 `[B-6b3b]`의 「헤더와 다르게」; `-FourthRungCap`은 제동도 함께 낮춰 시험이 되지 "
    "못한다). ⑩ 은혜의 두 길(위 `[B-6b2b]`의 ④) — 판단: 지금 길(은혜가 그 백성의 다른 절멸 슬롯을 결과 없이 비우고 짓던 방주를 무른다)을 둔다. 묶음 A의 "
    "은혜는 120판에 둘뿐이고 둘 다 AI가 공성으로 넘겨받은 성계의 절멸이라(§3 '묶음 A', `OPEN_QUESTIONS` #167) 슬롯 둘 이상의 몫은 재지 않음"
    "(잰 2); 모형의 P(250 안에 같은 백성에 둘째 절멸) ≈ 7%. 다른 길(같은 백성에 둘째 절멸을 놓지 않음)은 추첨 몫을 바꾸므로 이 커밋의 일이 "
    "아니다 — `[B-6b9]` 또는 [개발자]. 다스리는 백성의 절멸이나 초신성 전조에 쓰인 은혜 0. ⑪ 묶음의 '재지 않음': 치르지 못한 틱과 '방주가 선 채 "
    "만료'(그 값을 내는 출력이 없다) — 비용은 카운트다운 안의 완성 몫(1.000)과 비축으로 판단했다; 측정 전용 셈(테스트 모듈, 비트 그대로, "
    "`Sim.RaceMeasure`의 한 경우)은 원하면 `[B-6b7]`·`[B-6b8]`이 묶음 B 전에 더한다. ⑫")
n = once(n, " ⑫ `[B-6b5]`가 넘긴 손 읽기 셋", ITEMS2 + " `[B-6b5]`가 넘긴 손 읽기 셋")

# Item 14 at the end of the block's last bullet (the line that holds item 13).
m = re.search(r"(⑬ `\[B-6b5\]`의 패치 열[^\n]*)\n", n)
assert m
n = n[:m.end(1)] + (
    " ⑭ 고침 단계가 연 열린 물음의 번호를 #164에서 #167로 옮겼다 — #164~#166은 §0.2가 `[B-6b12]`의 문서 커밋에 잡아 둔 번호다(#164 '방랑자의 "
    "대면·정신의 닻·피난 호위'는 `docs/10_DESIGN/crisis_and_transcendents.md`의 절멸 행과 헤더 §8이 이미 부른다).") + n[m.end(1):]
save(N, n)

h = load(H)
LINE = ("- `[B-6b6]`(2026-11-09): 데이터 ① — 절멸 행(c 1e-4)·`project_ark`(에너지·물질 3)·`feat_passage_by_ark`, 결정 5의 코드 죽음 핀. "
        "다르게: 세계는 첫 셋째 칸 +1부터 갈릴 수 있다(절멸 또는 AI 조약 걸음 — §3.1은 추첨 표만); 이름은 `CosmicB6bRows`에; '방주' 지킴·'노아'; "
        "`Feats.Reachable`은 넷째 칸. 세부는 노트 §5.\n")
assert len(LINE.encode('utf-8')) < 400, len(LINE.encode('utf-8'))
m = re.search(r"- `\[B-6b5\]`\(2026-11-09\): 글과 목소리[^\n]*\n", h)
h = h[:m.end()] + LINE + h[m.end():]
save(H, h)
print('H line bytes', len(LINE.encode('utf-8')), 'H bytes', len(h.encode('utf-8')))

d = load(D)
DL = (
    "- `[B-6b6]`(2026-11-09 — 결정 2~8의 데이터 ①, 헤더 §3.1·§5.2·§8): 셋째 → 넷째 칸의 행 셋을 켠다 — B6b에서 처음으로 입력 없는 세계가 "
    "옮는다. **행**: `crisis_species_extinction`(`DT_Hazards`, `Hazard.Extinction`, `Draw`, 틱당 c 1e-4 — 셋째 칸 이상 백성의 정착된 성계, "
    "250틱, 효과 없음, 답 `project_ark`; 새 몫은 추첨 몫의 끝에 붙어 앞 행들의 추첨이 그대로다), `project_ark`(`DT_Megaprojects`, 셋째 칸, 진척 "
    "150, 에너지·물질 3씩 — `project_stabilize`의 2/2에 칸 배수 ×2.2/×1.5; 인과·영향 0), `feat_passage_by_ark`(`DT_Feats`, 셋째 → 넷째 칸, "
    "통치자 길, 조건 방주 완성·조약 하나, 값 개방성 −0.1) — 글은 B6b 노트 §1.5 그대로. c 1e-4는 헤더 결정 4·노트 §2 ⓐ의 값이고 `[B-6b9]`까지 "
    "둔다(묶음 A에서 c는 문으로만 작동했다 — 다스리는 백성의 셋째 → 넷째 칸 시간은 거의 모두 첫 절멸을 기다리는 시간이다; 고를 길 셋은 노트 §5). "
    "행 이름 상수는 두지 않는다(이름으로 부르는 코드가 없다 — `Tests/CosmicB6bRowNames.h`에 남는다). **결정 5의 코드 죽음 핀**(위 `[B-6b2b]`의 "
    "요구를 닫는다): `Data.Integrity`가 게임의 위기 행 가운데 `KillsTheRulerOf`가 참인 것이 정확히 `crisis_species_extinction`이고 그 만료의 "
    "효과 세 칸이 비고 `BranchChance`가 0임을 쥔다. **'방주'의 지킴**(결정 33): 새 목록 `ark_words`, `holy_and_myth`의 '노아', 방주의 행 규칙"
    "(테스트 여섯 — `[B-6b5]`가 넘긴 넷과 둘 더), `row_not` 둘, 여섯 열의 `forbidden`, 그리고 검사기 단위 테스트가 원본 전체에서 '방주'를 두 행 "
    "밖에 서지 못하게 한다. **묶음 A**(240판 + ⓐ·base·on 15 = 255 프로세스, 실패 0, end check·race check 줄 0, 스위치 대조와 쌍 검사 "
    "120/120): R 30판에서 첫 AI 넷째 칸 30/30 중앙값 3,281.5, 다스리는 넷째 칸 30/30 중앙값 4,983.5, 첫 AI가 먼저 24/30 = 0.80 [0.63, 0.91]; "
    "카운트다운 안의 방주 완성 1.000(세 칸 4,948/4,948 — 비용 3/3 유지), 만료된 다스리는 백성의 절멸·다스리는 이의 죽음·절멸로 지워진 백성 0; "
    "AI 조약 걸음이 맺은 조약 R 95·입력 없음 173·방랑자 293; 공성은 켬에서 늘었다(넷째 칸 AI의 상한 7 — `[B-6b9]`에). **다시 셈**(노트 §2의 "
    "`[B-6b6]`의 다시 셈 — `[B-6b7]`이 데이터로 넣는다): r 0.375(다른 질서의 몫 직접 다시 뽑기 9/30 = 0.30, 몬테카를로 0.287 — 띠 15~35% 안), "
    "P_bind 1,800·N 1,200 그대로, t = 5,000(n 30, 중앙값 4,983.5). **은혜의 두 길**: 지금 길을 둔다 — 은혜 2/120판, 둘 다 AI가 공성으로 "
    "넘겨받은 성계의 절멸(새 열린 물음 #167); 다른 길은 추첨 몫을 바꾸어 `[B-6b9]` 또는 [개발자]. **기준값**: 3,000틱 2816687688 / 3300228750 "
    "→ 2557850423 / 2906170550, 통치자 3,000 3561771757 / 2462331303 → 1189870756 / 3359656751, 방랑자 6,000 2665013209 / 2229239127 → "
    "271068805 / 309834531(시드 42의 3,121 즉위가 사라졌다), 20,000틱 2359424635 / 726895141 → 3168555397 / 1640870674, 통치자 20,000 "
    "1611658162 / 29194274 → 4053448003 / 129517356; 해시 판 둘이 커맨드렛과 같다; 세계는 첫 셋째 칸(1,521·방랑자 1,558) + 1부터 갈릴 수 있고 "
    "다섯 판 모두 AI 조약 걸음이 먼저였다(1,901·1,935) — T 이하 비트 그대로, `-WithoutB6bData=1`의 짝은 `[B-6b5]`의 다섯 줄 그대로. "
    "`Feats.Reachable`은 넷째 칸으로 올렸다. 테스트 223개 그대로(두 판 446 Success), 검사기 단위 테스트 95개. 변이: 새 열둘과 다시 뜬 둘, "
    "다시 돌림 파이썬 89 판·C++과 에셋 62 패치 — 모두 깨졌다. 리뷰: 발견 25(확인 12·일부 13 — 모두 들였다). 구절은 더하지 않았다. 세부는 B6b "
    "노트 §3 '묶음 A'·§5 `[B-6b6]`.\n")
m = re.search(r"- `\[B-6b5\]`\(2026-11-09 — 결정 33의 글과 목소리[^\n]*\n", d)
d = d[:m.end()] + DL + d[m.end():]
save(D, d)
print('done')
