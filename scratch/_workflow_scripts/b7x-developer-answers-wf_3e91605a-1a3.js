export const meta = {
  name: 'b7x-developer-answers',
  description: 'Apply the developer answers (all recommended) to the B7x header, notes and design doc on the design branch worktree, archive the original texts, prepare the leaf-rename script, then check adversarially and fix',
  phases: [
    { title: 'Apply', detail: 'design doc + archive, header, notes editors on disjoint files; rename-script prep on an export' },
    { title: 'Check', detail: 'four lenses over the applied answers' },
    { title: 'Verify', detail: 'one refuter per finding' },
    { title: 'Fix', detail: 'apply surviving findings, run doc checks, write the commit message' },
  ],
}

const WT = 'C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad/b7x/wt'
const OUT = 'C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad/b7x/answers'
const MAIN = 'C:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension'
const H = 'docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md'
const N = 'docs/60_DECISIONS/headers/B7_composable_existence_and_evolution_notes.md'
const D = 'docs/10_DESIGN/EXISTENCE_EVOLUTION_AND_ECOLOGY.md'
const ARCH = 'docs/archive/EXISTENCE_EVOLUTION_original_2026-11-09.md'
const ARCH_HTML = 'docs/archive/OVERVIEW_as_of_B6a.html'

const CONTEXT = `
You work on the B7x design branch of the Cosmic Ascension UE 5.8 project (solo developer, Korean docs), inside its own git worktree:
  WT = ${WT}
  (branch design/b7x-composable-existence, HEAD b57449c, already pushed). Scratch output folder: OUT = ${OUT} (create it if missing).
The branch was written on a PC without the engine: the B7x header ${H}, its notes ${N}, the design doc ${D}, never-compiled code and a data draft.
On 2026-11-09 an ultracode adversarial review (63 findings) was applied as b57449c; every decision that needed the developer was written in header section 10 as a bullet '(리뷰 반영 — 개발자 답 대기 …)' with the review's recommendation, alternatives and consequences.
On 2026-11-10 the developer answered '권장대로 진행해줘' to the lead's 12 questions: every decision takes the recommended option exactly as the lead put it to them. The ANSWER TABLE below is binding.

HARD RULES
- Work only inside WT and OUT. Never edit, stage, check out or run any git write command in MAIN = ${MAIN} (another workflow is building and measuring there right now). Reading MAIN files is allowed to check current main facts (main HEAD is 220129d [B-6b7]; block B6b is still running, so main's living docs change again before the B7x re-stack).
- Never run Unreal builds, the editor, Tools/run_checks.ps1 or anything that launches UnrealEditor. Light Python checks are fine, e.g. from WT: python -I -X utf8 Tools/check_docs.py
- In WT edit docs only (the files your task names). Do not edit Source/, Config/, Tools/, Content/ in WT: code and data renames happen at the re-stack, when the code can be built.
- Files are UTF-8 without BOM with LF line endings - keep them so (verify with python: the bytes contain no b'\\r'). Use exact, unique replacements; never rewrite a whole file from memory.
- Write plain Korean in the docs' existing style: native words, few loan words, no coined shorthand; name things by what they do.
- Project narrative rules apply to every sentence you add: the holy order (God) is never an entity, speaker, number, lever or ending condition; first-person log speakers are only the fallen transcendents; no verdict words (천국·지옥·구원받·멸망받·영혼·저주받·심판받) in narrator text; no counts of people as numbers; invented names are Korean descriptive names (no names from other works, myths, Bible proper names or real people); no saviour/messiah titles for the player and never 'became god'. Never add a Bible verse.
- docs/90_LORE/cosmic_species_codex.md is read-only. Do not touch docs/OVERVIEW.html in MAIN (an untracked file there, different from the branch file docs/OVERVIEW_as_of_B6a.html).
- The header must stay under 150,000 bytes (122,952 now).
- Cross-reference rule: after the design doc is split (answer 15), line references into the design doc ('설계서 :NNN', 'branch EXISTENCE...:NNN') break. Refer to the accepted design doc by section and rule/card IDs (§3.3 S14, §4.3 C10, §4.4, §5) and to the original by the archive file ${ARCH} plus its section heading (e.g. '원안 동결본 §2 축 5'), never by line number. To see what an old line reference pointed at, read the pre-split file with: git -C "${WT}" show b57449c:${D}

ANSWER TABLE (binding - the developer's answers, 2026-11-10)
A1. 결정 1 B7x의 자리 - 권장: 확장 블록 B7x (가는 B6b 뒤·B7 본 블록 앞, 나는 B8의 레인 결정과 함께, 다는 6a와 함께). 귀결을 받았다: B6b 값과 어긋나면 B7x의 노브가 먼저 물러선다 - B6b의 r·c·N·P_bind·C는 건드리지 않는다; 한 번 고쳐도 어긋나면 그 수와 함께 다시 묻는다. 데이터 커밋은 [B-7x5s]·[B-7x5c](와 필요할 때 [B-7x5r]).
A2. 결정 2 Brick. 뿌리 - 권장 + DT_Feats 스키마 규칙 7(업적 행의 조건에 Brick. 태그를 두지 않는다 - K4).
A3. 결정 3 잎 이름 - 리뷰가 고친 목록 전부, 선택 항목까지(개발자가 받았다): UnseenLaw→KeptLaw, SilenceFeed→IdleFeed(노브 BrickQuietInsightPerTick→BrickIdleInsightPerTick, 함수 IsQuietOf→IsIdleOf도), PlanckMicro→FoldedSmall, DysonBrain→ShutSphere, KesslerTapestry→DustBand, AbyssalMaw→InwardHunger, StarShepherd→StarTow, HermitMonolith→ClosedPort. 결정 3에 문장 하나를 더한다: '변이의 잎은 꼴의 이름을 받지 않는다 - 꼴 낱말은 변이 없이도 부모를 지닌 모든 백성에 보인다'. 이름은 세이브에 남으므로 [B-7x1]이 main에 들기 전에 코드·데이터의 자리를 모두 고친다(노트 §5.4 코7 - 리베이스 때; 준비한 스크립트 ${OUT}/rename/rename_leaves.py). 문서(헤더·노트·받은 설계서)의 현행 문장은 이 커밋에서 새 이름으로 고친다; 옛 이름은 '옛 이름 → 새 이름'의 기록과 동결본·§11의 지난 기록에만 남는다.
A4. 결정 4 - 권장: 시간 셈은 FCivilizationState에만; 결정 13의 답(㉢)이 4를 지킨다.
A5. 결정 6 HostCivID - 권장: [B-7x1]에 예약, 이름 HostCivID, 초기값 NAME_None, 뜻이 언다; 읽는 규칙은 슬라이스 다. (ㄱ) 건넘 단계가 지운 백성의 이름은 같은 걸음에서 모든 성계의 HostCivID에서도 비운다(추첨 없음 - PendingContactCivID를 비우는 꼴); (ㄴ) 탄생한 백성도 NextCivSerial에서 CivID를 받는다(지운 이름은 다시 쓰이지 않는다); (ㄷ) 리드의 판단(개발자가 뒤집을 수 있다고 적는다): 탄생 요청 필드는 지금 예약하지 않는다 - 꼴을 정할 명세(슬라이스 다)가 없다; 슬라이스 다의 헤더는 먼저 요청을 한 틱 안에서 끝내는 꼴(필드 없음)을 찾고, 필드가 들면 그때 두 해시가 한 번 더 옮는다.
A6. 결정 8 - 권장: 별 등급의 어두워짐 장치는 넣되 주기 0으로 꺼 둔다(켤지는 B8의 실제 하늘 헤더); [B-7x2]에 DimCapitalStar 거절 갈래 셋(남의 수도·정착 중·인덱스 없음)의 단언과 변이 하나(노트 §5.4 시6).
A7. 결정 9·23 - [개발자]로 올리지 않는다; R11·R13으로 적어 받는다(이미 적힌 그대로).
A8. 결정 11 - 권장: 수직 슬라이스의 줄에 청원 열하나와 조우 하나(갈림 열둘), 수량 줄은 'B6b 끝의 수 + 청원 11·조우 1'; 둘째 갈림의 ②는 (나) - 되풀이해 쌓이는 이득을 뺀다(적어도 petition_cross_law_stays ②의 PlayerStatDelta Exaltation +1; 데이터는 노트 §5.4 데1); 새 태그 없음 - 태그 열일곱 그대로. 측정 정책은 갈림 카드에서 ③만 고른다(결정 19).
A9. 결정 13 - ㉢(저장 없음): (a) 그 생애의 마지막 재위 백성(Life.RuledCivIDs의 끝)만 쓰고 다스린 적 없는 생애에는 붙이지 않는다; (b) 그 백성이 State.Civs에 있을 때만 붙인다; (c) 같은 백성을 다스린 생애가 여럿이면 마지막 생애의 장에만 붙인다; (d) 낱말은 '그 백성의 지금 살림'으로 읽히게 쓰고 판의 끝 뒤 다시 열면 달라질 수 있음을 받는다. 새 R 없음, '끝마다 다른 것은 까닭 줄뿐'의 약속을 지킨다. ReignShapes 필드는 들지 않는다. 세 규칙을 헤더 §4 첫 항과 설계서 §4.4에 적고, 손 확인 5를 ㉢에 맞게 고치고, [B-7x3]의 UI.Chronicle에 지워진 백성·다스린 적 없는 생애·두 백성을 다스린 생애의 경우(노트 §5.4 시9). 설계서 §4.4 표의 '§4.2 원안의 줄' 열을 지운다.
A10. 결정 14 - 권장: 레인의 단일 창구는 슬라이스 나에서 받는다; 모순 고침(고립 금지는 살아 있는 성계의 Link·Unlink에만, 파괴로 생기는 고립은 받는다 - §2.8에 이미 고쳤다); 슬라이스 나의 헤더가 (iii)~(v)에 먼저 답한다; CLAUDE.md 규칙 13의 낱말은 슬라이스 나에서.
A11. 결정 15 - ㉡ 설계서 S14의 꼴: project_settle이 끝난 틱에 수도의 레인 하나를 새 터의 이웃으로 옮긴다(걸음이 사건에 매여 주기 노브가 필요 없다). 고르지 않은 쪽(헤더 §2.8의 '주기마다 딸린 성계의 레인 하나를 수도 쪽 이웃으로')의 문장을 이 커밋에서 고친다. 좌표의 이동은 여전히 창구가 받은 뒤 둘째 걸음(권장 그대로).
A12. 결정 16 - 권장 + 조건: (1) 태어난 백성은 새 FCivilizationState다 - 새 CivID(NextCivSerial), 이름 뱅크의 새 이름, TimeVectors 0; 부모의 ArchetypeRow·Traits(탄생 블록 하나만 빼고)·관계·칙령·HostCivID는 물려받지 않는다; (2) 그 줄은 옛 백성이 살아남았다·죽지 않았다·돌아왔다·다시 태어났다고 말하지 않는다 - rebirth_words·afterlife_words·'불사'·'유령'을 스키마 규칙 하나로 막는다(슬라이스 다); (3) (3a) FossilAutomata는 LetPeopleGo(절멸)에서 태어나지 않고 글이나 스캔 얼굴로만 둔다; (4) 잎 Undying*은 선언하지 않는다. 원안의 불사 분열·유령 제국은 받지 않는다.
A13. 결정 18 - 권장 + 리뷰의 좁힘: KeptLaw 백성이라도 (a) 주장의 카운트다운에 더하는 칙령과 위기의 답인 칙령은 왕좌가 빈 뒤 여느 AI처럼 거두고(K9 - B6b 결정 9가 은하 법칙에 그 고리를 금지한 선과 같다), (b) 판정받는 행이 법칙을 물으면(넷째 칸 - CosmicEdicts::AsksForLawsOf, main Source/Cosmic_Ascension/Sim/CosmicEdicts.cpp:242) 백성 범위 칙령을 다른 AI처럼 거둔다(칙령 자리 셋이 법칙 셋의 자리다, B6b 결정 10·11). 헤더 §10 결정 18에 리뷰가 미리 써 둔 문장을 §2.4 그 행의 끝, 설계서 §4.3 C10의 귀결, 노트 §1.1의 KeptLaw 행에 넣고, §0의 R12를 이 좁힘으로 닫는다. 남는 귀결은 하나 - 전시 동원을 쥔 채 양위한 백성이 섭정 아래에서 안정도를 잃는 것(설계가 노린 값). ①도 받는다: 노트 §1.1의 QuantumCloud 행을 리뷰가 적은 글로('세계는 늘 확정 - 정착된 성계가 레인으로 닿거나 그릇이 서면 드러난다; 어디에나는 받지 않는다 - 표현도 무주 성계 위에 그 백성을 그리지 않는다; 남는 것은 S15의 소문 한 줄이고 그 줄은 제 별 곁에만'), 마지막 열은 '받지 않음'('(㉡은 뒤 블록)'을 지운다); 결정 18에 ''어디에나'는 표현으로도 받지 않는다(S15의 소문 한 줄뿐)'를 더한다. 답은 -Crossroads의 수(§5 ⑧)보다 먼저 왔다 - ⑧이 놀라운 값을 내면 노브([B-7x5r])로 다루고, 그래도 안 되면 수와 함께 다시 묻는다.
A14. 결정 20 - 권장: B6b가 끝난 main 위로 §8의 차례로 다시 쌓는다([B-7x1]과 [B-7x2]는 하나로, [B-7x5a]는 [B-7x5s]·[B-7x5c]로 갈라 다시 적고, 데이터는 [B-7x4m]·[B-7x4] 뒤); 충돌 해소는 §8.1.
A15. 결정 21(개발자의 글 - 답을 받았다): (i) 설계서의 원안 = 9be5b3e의 ${D} 글자 그대로를 ${ARCH}에 둔다(맨 위에 동결 표지 한 덩이만 더한다: 2026-11-09 개발자 원안 9be5b3e 글자 그대로, 받지 않은 것을 포함한다, 받은 꼴은 ${D}와 B7x 헤더, 고치지 않는다); ${D}에는 머리의 상태 표지, 동결본으로 가는 링크 한 줄, 받은 꼴(§3.3, §4.3, '원안의 줄' 열을 뺀 §4.4, §5)만 남긴다 - 남는 절은 번호와 제목을 그대로 둔다; (ii) docs/OVERVIEW_as_of_B6a.html은 ${ARCH_HTML}로 옮기고(git mv) 맨 위에 보이는 동결 표지(받지 않은 원안을 포함한다 - 현행 설계가 아니다; 현행은 docs/)를 둔다; (iii) promptforother.txt는 지운다(git rm - 안내문이 받은 뒤 지워도 된다고 했다). CLAUDE.md의 docs/archive 행은 main의 [B-7x0]에서 늘리지 않게 고쳐 쓴다(예: '옛 판·원안의 동결본(고치지 않는다 - 목록은 docs/SECTION_MAP.md)') - 쓸 글과 SECTION_MAP의 두 줄을 노트 §5.4 [B-7x0]에 적는다(CLAUDE.md·SECTION_MAP은 이 브랜치에서 고치지 않는다).
A16. 결정 22 - 권장: 가·나는 추첨 0, 다의 포자에 고정 수 하나(Phase 5 전에만 더할 수 있다); 결정 16의 조건 (1)·(2)를 먼저 적었으므로 이제 정해졌다.
A17. 결정 5·7·10·12·17·19·23과 결정 9(국경 조건)는 적힌 권장 그대로다.

HOW TO MARK ANSWERS IN HEADER §10: each '(리뷰 반영 — 개발자 답 대기 …)' bullet becomes '(답 2026-11-10 — 권장 …)' stating the final choice in one or two sentences, with the accepted conditions folded into the decided text and the alternatives kept short (a phrase each). Keep the decision title lines. No '답 대기' may remain anywhere in the header, notes or accepted design doc except inside the section 11 record of b57449c (history). Where the review wrote '이 결정을 기다린다/답이 오면' in body sections, rewrite to the decided form.
`

const EDIT_SCHEMA = {
  type: 'object',
  properties: {
    files_changed: { type: 'array', items: { type: 'string' } },
    applied: { type: 'array', items: { type: 'string' } },
    open_issues: { type: 'array', items: { type: 'string' } },
    notes_for_others: { type: 'array', items: { type: 'string' } },
    bytes_after: { type: 'string' },
  },
  required: ['files_changed', 'applied', 'open_issues', 'notes_for_others', 'bytes_after'],
}

const RENAME_SCHEMA = {
  type: 'object',
  properties: {
    script: { type: 'string' },
    mapping: { type: 'array', items: { type: 'string' } },
    sites_count: { type: 'integer' },
    site_list_file: { type: 'string' },
    checks_pristine: { type: 'string' },
    checks_renamed: { type: 'string' },
    collisions: { type: 'array', items: { type: 'string' } },
    ambiguities: { type: 'array', items: { type: 'string' } },
    doc_sites_for_editors: { type: 'array', items: { type: 'string' } },
  },
  required: ['script', 'mapping', 'sites_count', 'site_list_file', 'checks_pristine', 'checks_renamed', 'collisions', 'ambiguities', 'doc_sites_for_editors'],
}

const FINDINGS_SCHEMA = {
  type: 'object',
  properties: {
    findings: {
      type: 'array',
      items: {
        type: 'object',
        properties: {
          id: { type: 'string' },
          severity: { type: 'string', enum: ['high', 'medium', 'low'] },
          file: { type: 'string' },
          location: { type: 'string' },
          claim: { type: 'string' },
          evidence: { type: 'string' },
          fix: { type: 'string' },
        },
        required: ['id', 'severity', 'file', 'location', 'claim', 'evidence', 'fix'],
      },
    },
  },
  required: ['findings'],
}

const VERDICT_SCHEMA = {
  type: 'object',
  properties: {
    verdict: { type: 'string', enum: ['confirmed', 'partial', 'refuted'] },
    reason: { type: 'string' },
    corrected_fix: { type: 'string' },
  },
  required: ['verdict', 'reason', 'corrected_fix'],
}

const FIX_SCHEMA = {
  type: 'object',
  properties: {
    applied: { type: 'array', items: { type: 'string' } },
    skipped: { type: 'array', items: { type: 'string' } },
    check_docs: { type: 'string' },
    header_bytes: { type: 'integer' },
    notes_bytes: { type: 'integer' },
    design_bytes: { type: 'integer' },
    cr_free: { type: 'boolean' },
    leftover_wait_markers: { type: 'integer' },
    git_status: { type: 'string' },
    commit_message_file: { type: 'string' },
    summary_for_lead: { type: 'string' },
  },
  required: ['applied', 'skipped', 'check_docs', 'header_bytes', 'notes_bytes', 'design_bytes', 'cr_free', 'leftover_wait_markers', 'git_status', 'commit_message_file', 'summary_for_lead'],
}

phase('Apply')
log('Editors on disjoint files: design doc + archive, header, notes; rename script prepared on an export (WT untouched by it).')

const editD = () => agent(`${CONTEXT}

YOUR TASK (editor D - you own ONLY: ${D}, the new file ${ARCH}, the move of docs/OVERVIEW_as_of_B6a.html to ${ARCH_HTML}, and the removal of promptforother.txt; do not touch the header or notes - other editors own them right now).
1. Archive the original: write git -C "${WT}" show 9be5b3e:${D} byte for byte into WT/${ARCH} (check the bytes: LF, no BOM; compare its sha1 to the blob before adding the label). Look at how the existing frozen copies in WT/docs/archive begin (guideline_v0.8_2026-10-18.md, code_artifact.md) and add only one short frozen-label block at the very top (a blockquote of two or three lines: '동결본 - 2026-11-09 개발자 원안(9be5b3e) 글자 그대로. 받지 않은 것(되는 끝·불사 분열·유령 제국·사람의 수 등)을 포함한다 - 현행 설계가 아니다. 받은 꼴은 ${D}와 B7x 헤더(docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md). 고치지 않는다.' - word it in the docs' plain style; keep it free of rule-11 words). Nothing else in that file changes.
2. Move the HTML: git -C "${WT}" mv docs/OVERVIEW_as_of_B6a.html ${ARCH_HTML}; then insert at the very top of <body> a visible frozen banner (a small styled div; Korean text: '동결본 - B6a 때의 개관(2026-11-09). 받지 않은 원안(B7x 절의 여러 표현 포함)을 담고 있다 - 현행 설계가 아니다. 현행은 docs/의 문서와 B7x 헤더.') and an HTML comment with the same meaning just before it. Keep every other byte. The working file may have CRLF on disk (git normalizes text=auto) - edit bytes with Python, preserving whatever line ending the file has.
3. git -C "${WT}" rm promptforother.txt (explicit path; never use rm on unresolved paths).
4. Reduce ${D} to the accepted form (answer A15 (i)): keep the head status label but rewrite it to the decided state (answers received 2026-11-10; the original lives in ${ARCH}); add one link line to the frozen original; remove the bodies of §1, §2 (all five axes), §3.1, §3.2, §4.1, §4.2 and the original title's English subtitle; keep §3.3, §4.3, §4.4 and §5 with their section numbers and headings unchanged so outside references stay valid (keep the '## 3.' and '## 4.' parent headings, with one sentence each saying the original subsections are in the frozen copy). Give the file a plain Korean title that says it is the accepted form of the B7x design (keep the file name).
5. Apply the answers inside the kept sections: leaf names (A3) everywhere in current sentences (C05 '티끌 띠' is DustBand, S11 '안으로 돌아선 식욕' is InwardHunger, C06 '도읍의 별에 끄는 줄' is StarTow; also IdleFeed/KeptLaw/FoldedSmall/ShutSphere/ClosedPort) - keep shape (꼴) names such as WovenSwarm/LastHunger as they are; C10's consequence gets the A13 narrowing sentence (copy the prepared sentence from header §10 decision 18 - read it with: git -C "${WT}" show b57449c:${H}); §4.4 per A9 (the three rules + (d), drop the '§4.2 원안의 줄' column, keep the promise that only the reason line differs between endings); S14 per A11 (it is already S14's own form - make sure nothing in the kept text still describes the periodic form, and that S14 says no period knob); any birth / FossilAutomata / undying text in the kept sections per A12 (3a); the second-cross ② per A8 (나) if §4.3 describes their gains; QuantumCloud/'everywhere' per A13 ① (only S15's one rumour line, beside its own star). References from the kept sections to removed sections must point to ${ARCH} by section heading, or to the header section that now defines the thing (six time vectors: header §2.1·§2.2).
6. Verify: no b'\\r' in the md files you wrote; python -I -X utf8 Tools/check_docs.py from WT has 0 errors; grep the kept design doc for the old leaf names (UnseenLaw, SilenceFeed, QuietFeed, PlanckMicro, DysonBrain, KesslerTapestry, AbyssalMaw, StarShepherd, HermitMonolith) - only an explicit 'old -> new' record may remain; grep for '답 대기'.
Return: files_changed, applied (one line per answer you applied, with section), open_issues (anything you could not decide), notes_for_others (exact things the header or notes editor must change to stay consistent with your file, e.g. a section that moved or a sentence they quote), bytes_after (bytes of ${D} and ${ARCH}).`, { label: 'apply:design-doc+archive', phase: 'Apply', schema: EDIT_SCHEMA, effort: 'high' })

const editH = () => agent(`${CONTEXT}

YOUR TASK (editor H - you own ONLY the header ${H}; other editors are changing the notes, the design doc and the archive right now, so do not edit those, and read the design doc only through git show b57449c:...).
Apply the ANSWER TABLE to the header:
- §10: every decision answered per 'HOW TO MARK ANSWERS'. Decision 3 gets the final eight renames and the added sentence; decision 6 states (ㄱ)(ㄴ) as decided and (ㄷ) as the lead's call that the developer may overturn; decision 13 states ㉢ with (a)-(d); decision 15 states ㉡ (S14 form); decision 16 states (1)(2)(3a)(4); decision 18 states the narrowing and ①; decision 21 states the archive handling; decision 9 and 23 say they stay non-developer (R11/R13). Shorten the now-answered alternatives; do not lose any accepted condition.
- Body sections that carried pending wording or depend on an answer: §0 (the R table - close R12 by the narrowing; R8's quantity line 'B6b 끝의 수 + 청원 11·조우 1'; anything saying '답을 기다린다'); §1.4 if it mentions pending items; §2.1 (HostCivID frozen meaning; no ReignShapes; (ㄷ) not reserved); §2.4 (rename the leaves; KeptLaw row ends with the A13 sentence; IdleFeed knob/function names); §2.6, §2.7, §2.8 (A11: replace the periodic SystemDragger form with S14's settle-tick form, no period knob; keep the isolation fix), §2.9 (A12 conditions incl. 3a and no Undying*, A16 draws, A5 (ㄱ)(ㄴ)), §2.10, §2.11 (counts: tags 17, knobs, fields - check they still hold), §3.1, §3.3, §3.4 (BrickIdleInsightPerTick; no dragger period knob), §3.5, §4 (first item = A9's three rules + (d)), §5 (⑧ - the answer came before the numbers; what happens if ⑧ is surprising, per A13), §7 (the [B-7x0] list: the archive handling is done on the branch; CLAUDE.md archive row rewrite and SECTION_MAP lines are for main's [B-7x0]; promptforother.txt already removed), §8 (table row 1: answers received - what remains is the post-answer fixes and the main-side approval docs; row 2: no ReignShapes, no birth-request field; the leaf names come in with [B-7x1] using the prepared rename script ${OUT}/rename/rename_leaves.py), §8.1 if names appear, §9 (hand check 5 per ㉢; any leaf names), §11 (ADD one new entry at the end for this change: '개발자 답 반영(2026-11-10 - 전부 권장)' - what was decided in one dense paragraph and what changed in which files, including the archive moves; leave the earlier entries as history), and '추가 이야기와 보완' (the KeptLaw bullet now states the decided narrowing).
- Rename the leaves in every current sentence of the header (keep an explicit 'old -> new' record in decision 3 and history lines in §11). Replace design-doc line references with section/ID references, and references to removed original text with the archive file ${ARCH} by section heading.
- Check: grep the header for the nine old names (UnseenLaw, SilenceFeed, QuietFeed, PlanckMicro, DysonBrain, KesslerTapestry, AbyssalMaw, StarShepherd, HermitMonolith), for '답 대기', for 'Quiet' near Idle (only Revelation.Quiet facts may remain), for ':NNN' references into EXISTENCE_EVOLUTION; bytes < 150,000; no b'\\r'.
Return: files_changed, applied (one line per answer and per body section changed), open_issues, notes_for_others (exact things the notes editor or design-doc editor must mirror), bytes_after.`, { label: 'apply:header', phase: 'Apply', schema: EDIT_SCHEMA, effort: 'high' })

const editN = () => agent(`${CONTEXT}

YOUR TASK (editor N - you own ONLY the notes ${N}; other editors are changing the header, the design doc and the archive right now, so do not edit those, and read the header and the design doc only through git show b57449c:...).
Apply the ANSWER TABLE to the notes:
- §1 (the sixty-brick disposition table, five axes): rename the leaves (A3) in current text; the KeptLaw row (§1.1) gets the A13 narrowing sentence; the QuantumCloud row gets A13 ①'s text and its last column '받지 않음' ('(㉡은 뒤 블록)' removed); rows for FossilAutomata, Undying*, births, SystemDragger and the second-cross gains follow A12 (3a, no Undying*), A11 (S14 settle-tick form) and A8 (나); rows that cite the original design text cite ${ARCH} by section heading instead of design-doc line numbers.
- §2 (conflicts and handling) and §3 (draft OPEN_QUESTIONS rows): update to the answers; add a draft row if an answer leaves a question for a later slice that is not yet written down (e.g. A5 (ㄷ) - the birth request must first try to finish inside one tick; if a field is needed the two hashes move again; slice 나's (iii)-(v) from A10) - keep the placeholder numbering scheme the section already uses ('새N').
- §4 is history (the text once pasted to the other PC) - leave it, but if it still says something is pending, add a one-line note that the answers came 2026-11-10.
- §5.3 and §5.4 (rebase hand-offs by target commit): [B-7x0] - add the CLAUDE.md archive-row rewrite text (it must not grow the row; e.g. '옛 판·원안의 동결본(고치지 않는다 - 목록은 docs/SECTION_MAP.md)'), the two SECTION_MAP lines for ${ARCH} and ${ARCH_HTML}, and say the archive moves and promptforother.txt removal were done on the branch; [B-7x1] - 코7: the complete rename list (all eight + IdleFeed's knob and function) to be applied with the prepared script ${OUT}/rename/rename_leaves.py before the field commit lands, no ReignShapes, no birth-request field, HostCivID with (ㄱ)(ㄴ); [B-7x2] - 코1: the KeptLaw narrowing in code (release claim-hook and crisis-answer edicts like any AI after the throne empties; release people-scope edicts when the judged row asks for laws via CosmicEdicts::AsksForLawsOf) with its tests and one mutation each, decision 8's three assertions + one mutation (시6); [B-7x3] - 시9: UI.Chronicle cases per ㉢ (removed people, life that never ruled, life that ruled two peoples, two lives that ruled the same people); [B-7x5c] - 데1: option (나) (drop the repeating gains of the second-cross ②, at least petition_cross_law_stays ② PlayerStatDelta Exaltation +1); slice 다 - 16's conditions as a schema rule (rebirth_words, afterlife_words, 불사, 유령) and the HostCivID clearing test.
- Rename the leaves in every current sentence of the notes (keep explicit 'old -> new' records). Remove or rewrite every '답 대기' / '답이 오면' wording to the decided form (§5.5 is the review record - keep its history but mark answered items as answered).
- Check: grep the notes for the nine old names (UnseenLaw, SilenceFeed, QuietFeed, PlanckMicro, DysonBrain, KesslerTapestry, AbyssalMaw, StarShepherd, HermitMonolith), '답 대기', design-doc ':NNN' references; no b'\\r'.
Return: files_changed, applied (one line per answer and per section changed), open_issues, notes_for_others (exact things the header or design-doc editor must mirror), bytes_after.`, { label: 'apply:notes', phase: 'Apply', schema: EDIT_SCHEMA, effort: 'high' })

const renamePrep = () => agent(`${CONTEXT}

YOUR TASK (rename prep - you must NOT modify WT at all; work only in ${OUT}/rename/).
Goal: a tested, idempotent script that applies decision 3's leaf renames (A3) to the code, data, config, schema and tests of a tree, to be run at the re-stack (on main after B6b plus the re-stacked branch commits) before [B-7x1] lands.
1. Export two copies of the branch tree at b57449c: git -C "${WT}" archive --format=tar b57449c | tar -x -C ${OUT}/rename/pristine (and the same into ${OUT}/rename/renamed). LFS files are pointer files - fine.
2. In the pristine copy, find every occurrence of each old leaf name and of identifiers built from it, case-sensitive and in any form: gameplay tags (Brick.<Axis>.<Leaf> in Config/DefaultGameplayTags.ini including DevComment text such as 'leaf rename to QuietFeed proposed'), C++ identifiers in Source/ (accessors, functions like IsQuietOf, knobs like BrickQuietInsightPerTick, comments), Tools/data/*.json (DT_Rules knob names, BrickSeedTags, card/event conditions and effects), Tools/data/schema.json and Tools/cosmic_data_schema.py, Tools/test_data_schema.py, Tests/, ST_CA.csv. Old names: UnseenLaw, SilenceFeed (plus Quiet* identifiers that belong to SilenceFeed - but NEVER Revelation.Quiet or other Quiet words unrelated to this leaf), PlanckMicro, DysonBrain, KesslerTapestry, AbyssalMaw, StarShepherd, HermitMonolith. New names: KeptLaw, IdleFeed (BrickIdleInsightPerTick, IsIdleOf), FoldedSmall, ShutSphere, DustBand, InwardHunger, StarTow, ClosedPort. Shape (꼴) keys such as WovenSwarm, LastHunger, SwarmTech, StarEater must not change (see Sim/CosmicExistenceBricks.h in the branch). Build an exact token-level mapping (list each derived identifier explicitly).
3. Check collisions: each new name and each derived new identifier must not already exist in the branch tree or in MAIN (read-only grep of ${MAIN}/Source, Config, Tools).
4. Write ${OUT}/rename/rename_leaves.py: usage 'python -I -X utf8 rename_leaves.py <tree-root> [--dry-run]'; edits only Source/, Config/, Tools/ (and Content-free text files); never touches docs/ (docs are edited by hand), docs/archive/ or binary files; preserves each file's line endings and encoding byte for byte apart from the replaced tokens; prints per-file counts; is idempotent (a second run reports 0 changes); exits 1 if any old token remains outside an allow-list it prints. If renaming a knob means a generated reference (Tools/make_config_reference.py output) must be regenerated, say so and make the script print the follow-up command rather than doing hidden work.
5. Run it on the renamed copy, run it again (expect 0), then run the light Python checks on BOTH copies and compare: python -I -X utf8 Tools/check_data.py, Tools/test_data_schema.py, Tools/check_layers.py --strict, Tools/make_config_reference.py --check, Tools/check_docs.py (run each from that copy's root; some may already fail on the pristine branch copy - the point is that the rename adds no new failure; if make_config_reference --check fails only because a knob was renamed, regenerate it in the renamed copy and re-check, and record that the script must be followed by that regeneration).
6. Write the full site list (file:line old -> new) to ${OUT}/rename/sites.txt.
Return: script path, mapping (one 'old -> new' per item incl. derived identifiers), sites_count, site_list_file, checks_pristine and checks_renamed (exit codes + key lines), collisions, ambiguities (anything that needs the lead's choice), doc_sites_for_editors (old-name occurrences under docs/ in the branch, file:line - for the doc editors' cross-check; do not edit them).`, { label: 'apply:rename-script', phase: 'Apply', schema: RENAME_SCHEMA, effort: 'high' })

const applied = await parallel([editD, editH, editN, renamePrep])
const [resD, resH, resN, resR] = applied
log(`Apply done: D ${resD ? 'ok' : 'FAILED'}, H ${resH ? 'ok' : 'FAILED'}, N ${resN ? 'ok' : 'FAILED'}, rename ${resR ? 'ok' : 'FAILED'}`)

const applySummary = JSON.stringify({ design_doc_editor: resD, header_editor: resH, notes_editor: resN, rename_prep: resR }, null, 1)

const LENSES = [
  {
    key: 'answers',
    prompt: `LENS 1 - answer fidelity and cross-file consistency. Check that every row of the ANSWER TABLE is applied exactly (no option drifted: ClosedPort included; (나) not (가); ㉢ not ㉠/㉣; S14 settle-tick form not the periodic one; 3a; archive both; (ㄷ) not reserved and marked as the lead's call), that no accepted condition was lost, that no '답 대기'/'답이 오면' wording remains outside the §11 history of b57449c, and that the header, notes and accepted design doc say the same thing everywhere (names, counts - 17 tags, 17 knobs, 12 cards, 9 event rows -, commit order, R table, hand checks). Check every cross-reference into the design doc and the archive resolves (section headings exist; no stale ':NNN' line numbers into EXISTENCE_EVOLUTION; links to ${ARCH} and ${ARCH_HTML} are spelled right).`,
  },
  {
    key: 'narrative',
    prompt: `LENS 2 - narrative, theology, plain words and the frozen copies. Check every sentence added or changed in this change (git -C "${WT}" diff b57449c -- docs/ and the new archive file's label) against the project narrative rules in HARD RULES (holy order never an entity/speaker/number/lever/ending; first person only for the fallen; verdict words; no people counts as numbers; descriptive Korean names, none from other works/myths/Bible/real people; no saviour titles or 'became god'; no new verse) and the plain-words rule (no coined shorthand, no needless loan words). Check the eight new leaf names against the same naming rule (e.g. that 'StarTow', 'ClosedPort', 'DustBand' are descriptive and carry no other work's symbol) and that none collides in meaning with an existing tag word (read ${MAIN}/Config/DefaultGameplayTags.ini read-only; Revelation.Quiet is reserved vocabulary). Check the archive: ${ARCH} body is byte-identical to git show 9be5b3e:${D} apart from the top label; the HTML banner is visible and says it is not the current design; the labels themselves follow rule 11. Check A12 (2): the accepted texts nowhere let a born people be the old one surviving/returning/reborn.`,
  },
  {
    key: 'determinism',
    prompt: `LENS 3 - determinism, save, rule windows and the re-stack plan. Check against main facts (read-only ${MAIN}: docs/20_ARCHITECTURE.md, docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md decisions 9-11, Source/Cosmic_Ascension/Sim/CosmicEdicts.cpp AsksForLawsOf, Sim/GalaxySimulationSubsystem.cpp crossing cleanup) that: the KeptLaw narrowing as written is implementable without a new saved field or draw and keeps K9 (no claim newly released or delayed) and the law slots (three slots = three laws); ㉢ needs no saved field and the four presentation rules read only existing state (Life.RuledCivIDs, State.Civs) through the knowing-view rule (CLAUDE.md rule 14) where the chronicle reads; (ㄷ) not reserving the field is stated with its hash consequence; S14's settle-tick lane move is a world change that needs the lane single window of slice 나 (rule 13) and is described as slice 나 work, with no period knob left in §3.4 or the knob count; 3a keeps LetPeopleGo untouched; §8's table rows and §8.1 still match (row 2 no ReignShapes/no birth field; the rename script runs before [B-7x1]). Also review the rename-prep result below (mapping completeness - every derived identifier, shape keys untouched, Revelation.Quiet untouched, idempotence, allow-list, the config-reference follow-up) by reading ${OUT}/rename/rename_leaves.py and ${OUT}/rename/sites.txt, and check the notes' 코7 hand-off matches the script's mapping.`,
  },
  {
    key: 'critic',
    prompt: `LENS 4 - completeness critic. Ask what is missing: an answer applied in one file but not mirrored in another; a consequence the review listed under a decision that the decided text dropped (e.g. decision 1's 'ask again with the numbers', decision 18's ⑧ timing, decision 21's [B-7x0] main-side steps, decision 3's 'only before [B-7x1]'); a slice-나/다 question that now has no home (header §2.8/§2.9, notes §3); the §11 entry for this change missing a file or a decision; any reference to promptforother.txt or docs/OVERVIEW_as_of_B6a.html that still points at the old place; STATUS-style claims that main will need at [B-7x0] but that nobody wrote down in notes §5.4. Report only concrete gaps with the exact place to add the fix.`,
  },
]

const COMMON_CHECK = (lens) => `${CONTEXT}

You are a reviewer. The editors have just applied the developer's answers in WT (uncommitted; compare with git -C "${WT}" diff b57449c and git -C "${WT}" status). Editors' and rename-prep reports:
${applySummary}

${lens.prompt}

Report only real defects with evidence (quote the text and give file + section or line). Severity: high = an answer applied wrongly or a rule broken; medium = inconsistency or broken reference; low = wording. Each finding needs an exact fix (the replacement text or the precise action). Do not edit any file.`

phase('Check')
const verified = await pipeline(
  LENSES,
  (lens) => agent(COMMON_CHECK(lens), { label: `check:${lens.key}`, phase: 'Check', schema: FINDINGS_SCHEMA, effort: 'xhigh' }),
  (res, lens) => {
    const findings = (res && res.findings) || []
    log(`lens ${lens.key}: ${findings.length} findings`)
    return parallel(findings.map((f) => () =>
      agent(`${CONTEXT}

You are a skeptical verifier. Try to REFUTE this review finding about the applied answers in WT (uncommitted; see git -C "${WT}" diff b57449c). Read the actual files. If the finding is wrong, say refuted. If it is right but its fix is wrong or incomplete, say partial and give the corrected fix. If right, confirmed (repeat or sharpen the fix).
Finding (${lens.key}): ${JSON.stringify(f)}`, { label: `verify:${lens.key}:${f.id}`, phase: 'Verify', schema: VERDICT_SCHEMA, effort: 'high' })
        .then((v) => ({ lens: lens.key, finding: f, verdict: v }))))
  },
)

const all = verified.filter(Boolean).flat().filter(Boolean)
const kept = all.filter((x) => x.verdict && x.verdict.verdict !== 'refuted')
const refuted = all.filter((x) => x.verdict && x.verdict.verdict === 'refuted')
log(`findings: ${all.length}, kept ${kept.length}, refuted ${refuted.length}`)

phase('Fix')
const fix = await agent(`${CONTEXT}

YOUR TASK (fixer - you may edit ${H}, ${N}, ${D}, ${ARCH} (label only - never its body) and ${ARCH_HTML} (banner only) in WT; nothing else; do not commit, do not stage beyond what git mv/rm already did).
Editors' reports:
${applySummary}

Apply every kept finding below (use corrected_fix when the verdict is partial). If two findings conflict, choose the one that follows the ANSWER TABLE and say so. Then:
1. Run python -I -X utf8 Tools/check_docs.py from WT (0 errors required).
2. Check bytes: header < 150,000; no b'\\r' in the three md files and the archive md; no BOM.
3. Grep header, notes and the accepted design doc for '답 대기' (allowed only inside the §11 record of b57449c) and for the nine old leaf names (allowed only in explicit 'old -> new' records and §11 history) - report the counts.
4. Update the header §11 entry for this change so it states the review of the answers (lenses four: findings N, kept K, refuted R) and the checks.
5. Write the commit message to ${OUT}/commit_b7x_answers.txt (UTF-8, LF): a Korean title starting with '[B-7x] 개발자 답 반영(2026-11-10, 전부 권장) — ' followed by the main decided items in a short comma list (잎 이름 여덟, 남긴 법의 좁힘, 끝의 꼴 ㉢, 둘째 갈림 ②의 (나), 끄는 블록은 정착의 틱, 탄생의 조건, 원안·OVERVIEW 동결본, promptforother.txt 지움 - adjust to what was really done); a blank line; a body of short '- ' bullets in plain Korean (what was decided, what files changed, the archive moves, the rename script prepared in the scratchpad and run on an export with its check results, the review numbers, that no code/data/config changed and nothing was built); a blank line; then exactly: Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>
6. Return git -C "${WT}" status --short as git_status.

KEPT FINDINGS:
${JSON.stringify(kept.map((x) => ({ lens: x.lens, id: x.finding.id, severity: x.finding.severity, file: x.finding.file, location: x.finding.location, claim: x.finding.claim, fix: x.finding.fix, verdict: x.verdict.verdict, corrected_fix: x.verdict.corrected_fix })), null, 1)}

REFUTED (do not apply; listed so you know): ${JSON.stringify(refuted.map((x) => ({ id: x.finding.id, claim: x.finding.claim, reason: x.verdict.reason })))}`, { label: 'fix:apply-findings', phase: 'Fix', schema: FIX_SCHEMA, effort: 'high' })

return {
  apply: { design: resD, header: resH, notes: resN, rename: resR },
  findings_total: all.length,
  kept: kept.map((x) => ({ lens: x.lens, id: x.finding.id, severity: x.finding.severity, claim: x.finding.claim, verdict: x.verdict.verdict })),
  refuted: refuted.map((x) => ({ lens: x.lens, id: x.finding.id, claim: x.finding.claim, reason: x.verdict.reason })),
  fix,
}
