# Cosmic Ascension 인수인계 — B6b 마무리와 B7x 통합 (Claude Code 세션 → 다른 세션)

> **상태 줄**(커밋할 때마다, 그리고 멈출 때 고친다): 2026-11-14 07:30 — 이 세션(Claude Code 15a1d0fe)의 범위를 마쳤다. main = `18a0cfa` `[B-7xz]`(푸시) — 그 아래 `56e7d82` `[B-7h0]` — B7x 여덟 커밋(`[B-7x0]`~`[B-7x5r]`, 끝 `62eb013`)과 B7 헤더 승인 문서. 남은 브랜치는 지웠다(원격은 `main`·`handoff/cloud`·`tools/agent-setup`만; 지우기 전 브랜치는 `Saved\Handoff\branches_before_cleanup_2026-11-14.bundle`). **다음 세션은 `Saved\Handoff\PROMPT_antigravity_B7_start_2026-11-14.md`부터 읽는다** — 이 파일의 §4·§5(B6b와 B7x 통합)는 끝난 일의 기록이다.

이 파일은 저장소 밖(`Saved/`는 git이 무시한다)의 운영 메모다. 설계의 단일 기준은 `docs/`, 지금 상태는 `STATUS.md`와 `git log`다 — 이 파일과 다르면 그쪽이 이긴다.

> **2026-11-14 — 이 파일의 §4(B6b)·§5(B7x 통합)·§6은 끝난 일이다.** 다음 일(B7-0 다시 잼·B7 코드·B7x 손 확인의 준비)과 지금의 규칙·도구는 `Saved\Handoff\PROMPT_antigravity_B7_start_2026-11-14.md`에 있다. §1~§3의 규칙 가운데 그 글과 다른 것(통합 작업 사본·`integrate/b7x`·`SCR\b7x\int` — 이제 없다)은 그 글이 이긴다.

## 0. 먼저 할 일 (차례)

**클라우드(GitHub만, UE 없음)에서 이 파일을 열었으면 §9부터 읽는다** — 아래 1번의 프로세스 확인은 이 PC에서만 뜻이 있다.

1. **Claude Code 세션이 멈췄는지 확인한다.** 두 세션이 한 작업 사본을 동시에 고치면 트리와 측정이 망가진다.
   - 프로세스: `Get-Process UnrealEditor*,UnrealBuildTool,dotnet -ErrorAction SilentlyContinue`가 비어 있어야 한다. python은 명령 줄로 가른다: `Get-CimInstance Win32_Process -Filter "Name='python.exe'" | Select-Object CommandLine` — `bundleB`·`bundleC`·`lanes`·`run_mutations`·`run_py`가 보이면 아직 돈다(이 도구 자신의 언어 서버 같은 것은 상관없다).
   - 파일: 지난 15분 안에 바뀐 파일이 없어야 한다 — `Get-ChildItem '<SCR>','<저장소>\Saved\Balance' -Recurse -File | Where-Object LastWriteTime -gt (Get-Date).AddMinutes(-15)`(쉬는 단계에서는 프로세스가 없어도 곧 다시 쓴다).
   - 그래도 모르면 이것만은 개발자에게 한 줄로 묻는다: 'Claude Code 세션을 닫았나요?'
   - 변이 패치가 걸린 채 멈췄을 수 있다 — §8 끝의 '되돌리기'.
2. 읽기: `CLAUDE.md`(프로젝트 규칙 — 전부 지킨다) → `STATUS.md`의 '지금'과 '다음 3작업' → 이 파일 → B6b 헤더 §8과 노트 §5(아래 §4).
3. **할 일 고르기**:
   - B6b가 남았으면 §4가 주 작업이다. 그 사이 B7x는 §5.2의 차례로 **통합 작업 사본에서만**, B6b가 빌드·측정하지 않을 때만(리뷰·기록 단계) 나란히 한다: 첫 컴파일·테스트 → 3단계 준비 `[B-7x4m]`·`[B-7x3]`·`[B-7x4]`(데이터를 켜지 않는 커밋만). B7x 데이터(`[B-7x5s]`·`[B-7x5c]`)는 B6b가 끝난 뒤에만.
   - B6b가 끝났으면(`[B-6b12]` 커밋) §5.3, 둘 다 끝났으면 §5.5.
4. 멈출 때(이 세션이 끝날 때): `STATUS.md`를 고치고(`CLAUDE.md` '세션 끝'), 이 파일 맨 위 상태 줄을 그 시점으로 고친다.

## 1. 프로젝트와 경로

- 저장소: `C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension` (원격 `origin` = GitHub `rhwwl12-creator/Cosmic_Ascension`, 비공개; 푸시는 Git Credential Manager가 한다 — 토큰을 화면에 내지 않는다).
- 엔진: `C:\Program Files\Epic Games\UE_5.8` (UE 5.8.3). `.uproject`의 EngineAssociation GUID는 바꾸지 않는다.
- **`SCR`**(스크래치 원본) = `C:\Users\rhwwl\AppData\Local\Temp\claude\c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension\37de2919-0c44-45d8-94cd-4dcf0860263e\scratchpad`. 단계마다의 지침서(`brief_final.md`)와 측정 도구(파이썬)가 여기 있고, 도구 몇은 자기 경로를 절대 경로로 안다.
- **스크래치 백업** = `Saved\Handoff\scratch\` — 같은 상대 경로로 지침서·도구·리드 판정을 복사해 두었다(`b6b6\bundleA.py` 포함 — `bundleB.py`·`bundleC.py`·`lanes.py`가 그것을 불러 쓴다). Temp가 지워졌으면 이 폴더를 `SCR`로 그대로 되돌려 놓는다(같은 경로라야 절대 경로가 맞는다). 되돌린 뒤 `python -I -X utf8 <SCR>\b6b8\bundleB.py --dry-run --t-death 5000 --r 0.375`가 명령 목록을 찍는지 먼저 본다. 백업에 없는 것: 묶음 B의 CSV(`SCR\b6b8\bundleB\`, 약 330 MB — `[B-6b11]`의 `bundleC.py`가 읽는다; 없으면 그 지침서의 대안 줄을 따른다)와 큰 로그. `_workflow_scripts\`는 Claude Code가 단계를 돌린 차례의 기록(참고용 — 다른 도구에서는 돌지 않는다).
- 측정 보관: `Saved\Balance\` — `b6b_bundleA`·`b6b_bundleB` 같은 보관 폴더는 덮어쓰지 않는다.
- 작업 사본(worktree): main(저장소), `SCR\b7x\int`(브랜치 `integrate/b7x` — B7x 통합), `SCR\b7x\wt`(브랜치 `design/b7x-composable-existence` — 동결, 고치지 않는다), `[B-6b12]` 뒤에는 `C:\CAL\l1`~`l3`(변이 레인)이 생긴다. `git worktree list`로 본다.
  - Temp가 지워져 `SCR\b7x\int`가 없어졌으면: 커밋은 저장소 `.git`의 `integrate/b7x`에 남는다. `git worktree prune` 뒤 `git -c core.eol=lf worktree add "<SCR>\b7x\int" integrate/b7x`로 다시 붙인다(커밋하지 않은 변경은 사라진다).

## 2. 일하는 방식 (개발자와의 약속)

- **한국어로 보고한다.** 쉬운 낱말로, 지어낸 줄임말 없이, 일은 하는 일로 부른다. 보고·제안·리뷰는 '추가 이야기와 보완'으로 끝내고, 맨 끝 한 줄에 다음 작업과 그 무게(아래 §4의 high·xhigh)를 적는다.
- **'권장대로'가 기본 승인이다 — 이미 승인된 헤더(B6b, B7x)에 적힌 일에만.** 개발자에게 묻고 답을 기다리는 것은 셋이다.
  1. 헤더와 지침서의 **[개발자]** 표지(지침서의 '멈춤 — 커밋하지 않음' 문 포함: `b6b9` §1의 판정 표, `b6b10`의 ⓔ, `b6b11`의 R8과 '헤더 먼저').
  2. 승인된 헤더에 없는 저장 필드·열거자·틱 단계·추첨 지점·해시 규칙 — `CLAUDE.md`대로 **헤더(또는 헤더의 고침)를 먼저 써서 보이고 → 승인 → 소스**. 이 세션은 스스로 승인하지 않는다. 틱 순서·추첨 지점·저장 필드가 바뀌면 `docs/20`을 먼저 고친다.
  3. 미리 적은 판단이 고침 한 번 뒤에도 어긋날 때(B6b `[B-6b9]`의 B′, B7x `[B-7x5r]`).

  나머지 권장안과 작은 보완(툴팁·문구·표시)은 묻지 않고 그 작업의 커밋에 바로 넣는다. 물음을 닫을 때는 `docs/60_DECISIONS/DECISIONS_LOG.md`에 날짜·근거를 적고 `OPEN_QUESTIONS.md`의 행을 지운다.
- **커밋**: 컴파일되고 자동화 테스트가 두 번 연속 통과한 상태만. 제목은 한국어 요약에 작업 접두어(`[B-6b8b]` 등 — `git log`의 앞 커밋 꼴), 본문은 `- ` 항목, 끝에 `Co-Authored-By:` 트레일러.
  - **주의**: `SCR\b6b8\commit_b6b8.txt`와 지침서들(`b6b8b`~`b6b12`의 `brief_final.md`)의 커밋 틀 끝 줄 `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`은 Claude Code 세션의 줄이다. 메시지 사본에서 **이 세션이 실제로 쓴 도구·모델의 줄로 바꾼다**(지침서 파일은 고치지 않는다).
  - main의 커밋은 커밋마다 푸시한다. **`integrate/b7x`는 원격에 올리지 않는다** — 리베이스로 커밋을 고쳐 쓰기 때문이다(B6b 헤더 §8 '푸시한 커밋은 고쳐 쓰지 않는다'). 강제 푸시는 쓰지 않는다.
- **빠른 차례**(근거: B6b 노트 §5 `[B-6b3b]`의 '일의 속도'): 러너 `-Tier full`은 커밋마다 **한 번** — 그 안의 두 판(`run_checks.ps1`의 pass 1·2)이 `CLAUDE.md`의 '테스트 2회 연속'이고, B6b·B7x 헤더 §8 표의 '러너 full 두 번'도 그 두 판으로 읽는다. 커밋 메시지에는 '러너 full 한 번(그 안에서 N개를 연속 두 판 — 성공 2N, 실패 0)'으로 적는다. 실패가 하나라도 있으면 커밋하지 않는다. 변이는 그 커밋의 새 단언과 그 커밋이 닿은 옛 패치만; B6b 전체 변이 재실행은 `[B-6b12]`에서 한 번.
- **러너**: PowerShell에서, 에디터를 닫고: `powershell -ExecutionPolicy Bypass -File Tools\run_checks.ps1 -Tier full [-Data] [-UpdateBaseline]`. bash로 부르면 백슬래시가 사라진다. 명령 전문과 PowerShell 5.1 주의는 `docs/40_DEV_GUIDE.md`의 '빌드·실행·테스트'.
  - `-Data`는 `Tools/data/`의 어느 파일(JSON·`schema.json`·`ST_CA.csv`)이나 `Tools/cosmic_data_schema.py`를 고친 커밋에 붙인다 — `generated.sha1`이 그 모두에 도장을 찍는다(`Data.SourcesInSync`).
  - 러너와 측정 도구는 `$env:PROJECT_ROOT`가 있으면 그것을 뿌리로 쓴다. 작업 사본을 바꿀 때마다 `Remove-Item Env:PROJECT_ROOT -ErrorAction SilentlyContinue`. 통합 작업 사본에서는 늘 그 사본의 `Tools\run_checks.ps1`을 부른다.
  - `-UpdateBaseline`은 `Tools/baselines/seed42.json`을 PowerShell 꼴(CRLF·정렬·`\u0027`)로 다시 쓴다 — 저장소 뿌리에서 `python -I -X utf8 <SCR>\b6b8\fix_seed42.py`(**인자 없이** — 꼴만 되돌린다; `--from-on`·`--final`은 `b6b8_on`을 박아 두어 다른 단계에서 쓰지 않는다)로 커밋된 꼴(json indent 4, LF)로 되돌린다.
- **파이썬 검사 다섯**(에디터 없이, 저장소 뿌리에서): `python -I -X utf8 Tools/check_data.py`, `Tools/test_data_schema.py`, `Tools/check_layers.py --strict`, `Tools/make_config_reference.py --check`, `Tools/check_docs.py`.
- **스테이징**: `docs/OVERVIEW.html`(추적하지 않는 로컬 파일)은 **절대 건드리거나 커밋하지 않는다**. `git status`의 'M' 수백 줄 가운데 `git diff --numstat`이 `0 0`인 것은 stat 소음이다.
  - Git Bash: `{ git -c core.quotepath=false diff --name-only; git -c core.quotepath=false ls-files --others --exclude-standard; } | grep -v '^docs/OVERVIEW.html$' | git add --pathspec-from-file=-`
  - PowerShell 5.1: `$l = @(git -c core.quotepath=false diff --name-only) + @(git -c core.quotepath=false ls-files --others --exclude-standard) | Where-Object { $_ -ne 'docs/OVERVIEW.html' }; [IO.File]::WriteAllLines("$PWD\Saved\stage.txt", $l); git add --pathspec-from-file=Saved/stage.txt` (`>`·`Out-File`은 UTF-16을 써서 쓰지 않는다).
  - 그 뒤 `git diff --cached --name-only`에 `docs/OVERVIEW.html`과 뜻밖의 새 파일이 없는지 본다.
- **줄끝**: 저장소의 파일은 LF다. git·`git apply`·PowerShell이 CRLF를 쓰는 일이 있다 — 커밋 전에 바뀐 파일에 CR이 없는지 본다. 통합 작업 사본에서는 git이 파일을 쓰는 명령에 `-c core.eol=lf`를 붙이고 `python -I -X utf8 <SCR>\b7x\tools\lf_norm.py <작업 사본> [--check]`(작업 사본이 먼저, `--check`가 뒤).
- **크기 한도**(`check_docs`가 보지 않으니 커밋 전에 손으로 잰다 — `(Get-Item <파일>).Length`): `CLAUDE.md` ≤ 16,384 LF 바이트(지금 36바이트 남음 — 줄을 늘리지 않는다), B6b 헤더 < 149,000(150,000 한도; 지금 약 148,700 — `[B-6b8b]`부터는 §11에 한 줄 더할 때마다 같은 바이트를 노트로 옮긴다), 헤더 §11은 커밋마다 400바이트 아래 한 줄.
- **헤더 §11 줄**: 커밋마다 한 줄. `[B-6b8]`의 줄은 이미 있고 `<<B:…>>` 자리만 채운다 — 커밋 전 `git grep -n "<<B:"`가 0.
- **열린 물음 번호**: `OPEN_QUESTIONS.md`의 #164~#166은 `[B-6b12]`의 몫, #167은 찼다 — 새 물음은 #168부터. B7x의 새 물음(노트 §3의 '새1'~'새11')은 B6b가 끝난 뒤의 마지막 번호 다음에.

## 3. 반드시 지킬 것 (내용과 안전)

- 신학: 거룩한 질서(하나님)는 엔티티·화자·수치·지렛대·엔딩 조건이 아니다. 1인칭 로그 화자는 타락 초월자만. 영혼·구원·멸망을 세는 필드는 없다. 플레이어에게 구원자·메시아 칭호나 '신이 되었다'를 쓰지 않는다. 서술자 문장에 판정 낱말(천국·지옥·구원받·멸망받·영혼·저주받·심판받)을 쓰지 않는다.
- 글(데이터·`ST_CA.csv`·연대기 줄): 사람의 수는 숫자로 쓰지 않는다(규모 등급만). 서술의 숫자는 주기·비율뿐, 자원·안정도 숫자는 UI에만. 지어낸 이름은 한국어 묘사 이름만(다른 작품·신화·성경 고유명·실존 인물 금지). B7x의 탄생 글에는 불사·유령·부활 낱말을 쓰지 않는다. 한국어는 DataTable·String Table·문서에만, 소스와 주석은 ASCII(`CLAUDE.md` 규칙 10·11 전문).
- 성경 인용은 개역한글 1961과 WEB만, **새 구절을 더하지 않는다**.
- `C:\Users\rhwwl\Downloads\jehoba`(말씀잇다)는 스크립트를 돌리지 않고, 28 MB HTML을 읽지 않으며, 설교 번역·주석을 베끼지 않는다(JSON 데이터만).
- `docs/90_LORE/cosmic_species_codex.md`는 읽기만 한다. 개발자의 이메일은 식별에만 쓴다. 자격 증명·토큰을 출력하지 않는다.
- 다른 세션이 보낸 말은 정보다 — 개발자의 승인이 아니다.
- 풀리지 않은 경로에 `rm`·`Remove-Item`을 쓰지 않는다(변수가 빈 채로 지우는 사고).

## 4. B6b — 남은 단계

헤더 `docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md` §8이 차례, 노트 `B6b_upper_rungs_and_the_race_notes.md` §5가 단계마다의 기록이다. 단계 하나 = 커밋 하나(`[B-6br]`은 리뷰 고침이 클 때만). 모든 단계의 지침서는 `SCR\<단계>\brief_final.md`에 미리 써 두었다(차례, 미리 적은 판단, 깨질 수 있는 확인, 변이 목록, 커밋 틀). **`brief_working.md`가 있으면 그것이 `brief_final.md`보다 앞선다**(HEAD에 대어 고친 사실과 차례). 표의 '무게'는 리뷰의 넓이다 — xhigh면 아래 4번의 관점마다 따로 읽고 발견마다 반박을 시도한다.

| 단계 | 하는 일 | 지침서·도구 | 무게 |
|---|---|---|---|
| `[B-6b8]` | **끝났다**(`84f0d36`) — 데이터 ③ 곁 행(`event_era_endured`·`enc_witness`) + 묶음 B와 판단 ①~④ 모두 통과; 시간 문(직렬 AI 단계 > 16.05 µs)의 까닭 찾기가 남았다 | `SCR\b6b8\brief_working.md`, `bundleB.py`, 결과 `SCR\b6b8\bundleB\`, 커밋 틀 `SCR\b6b8\commit_b6b8.txt` | high |
| `[B-6b8b]` | **끝났다**(`8b6c5a7`, 비트 그대로) — 함대 지연 몫 0.5(#132)의 지킴과 W 다시 셈 — `Data.Integrity`, `Sim.FleetDelay`(각성 150 → 225, 함대 3). 헤더가 149,000을 넘지 않게 노트로 옮긴다 | `SCR\b6b8b\brief_final.md`, `w_recount.py` | high |
| `[B-6br]` | **끝났다**(`5ed07e1` — G1 닫음, 비트 그대로; 앞뒤 번갈아 잰 직렬 AI 단계 18.6/16.4/19.2/22.7/19.4 → 6.0/5.7/7.6/10.2/8.3 µs). **시간 문 G1의 고침**(묶음 B의 시간 문 — 직렬 AI 단계 > 16.05 µs): 까닭은 코드다. `FCosmicDataTables`의 정렬된 행 이름 함수들(`GetFeatRowNames` 등)이 부를 때마다 이름을 새로 모아 정렬한다(AI 단계의 41~55%). 결과를 바꾸지 않는 인스턴스 기억 + `IsHazardAnswer`가 할당 없이 걷기 — 다섯 줄·해시 판 둘·CSV가 비트 그대로이고 AI 단계가 7~13 µs(시제품). 헤더(`Data/CosmicDataTables.h`)가 바뀌므로 지침서의 헤더 글을 먼저 노트에 적는다 | 증거 `SCR\b6b\timing\judgement.md`·`runs.md`·`prototype.diff`, 지침서 `SCR\b6br\brief_final.md`(쓰는 중) | high |
| `[B-6b9]` | **판단이 부를 때만**: 값만 고치고 B′. 묶음 B의 판단(노트 §5 `[B-6b8]` 칸의 ⑧ 읽기)이 모두 통과면 커밋 없이 건너뛴다 | `SCR\b6b9\brief_final.md`(§1.1~1.5의 글만 얼렸다 — `readings.sha1`; `brief_final.md`·`readings.sha1`은 고치지 않는다; 도구 출력 `bundleBprime\`·`probe_rows.jsonl`은 이 폴더에 생긴다), 하네스 `SCR\b6b9\bundleB.py --classify/--bprime` — **먼저** `SCR\b6b8\fix\handoff.md` §4대로 `[B-6b8]`이 하네스에 넣은 셈 고침 M2~M5(`b6b8\fix\bundleB.before.py` ↔ `b6b8\bundleB.py`)를 이 사본에 같은 꼴로 옮기고, `bundleB_selftest.py`와 `--check-only <SCR>\b6b8\bundleB --summary-dir <새 폴더>`로 묶음 B의 판단이 그대로 나오는지 본다 | high |
| `[B-6b10]` | **끝났다**(`4448705` — C = 9,000; 3,000의 두 줄·해시 판 둘·통치자 20,000은 비트 그대로, 입력 없는 20,000과 방랑자 6,000은 기록만으로 옮았다; 테스트 227종). 시계: 미리 적은 규칙으로 C(묶음 B가 정한 C = 9,000 — 갈래 M), 박힌 테스트, `docs/30`, 6,000의 언급; 지침서는 `10205b6`에서 썼으므로 부모를 `[B-6br]`로 다시 읽는다; 헤더가 149,000에 19바이트만 남아 §11 줄만큼 노트로 옮긴다 | `SCR\b6b10\brief_final.md`, `csv_cmp.py --clock` | high |
| `[B-6b11]` | **끝났다**(`467cfdb` — 묶음 C 620판 PASS — #136 18/60 열어 둠·#142 재지 않음(잰 4)·B1 잡음(L 7/G 4, p 0.5488)·[개발자] 항목 없음, 손 확인 세이브 넷(Saved\HandCheck\B6b_*), 비트 그대로). 다시 잼(묶음 C) — 노브를 바꾸면 제 커밋 | `SCR\b6b11\brief_final.md`(작업 메모 `SCR\b6b11rev\`), `bundleC.py`(다른 UE 프로세스가 돌면 멈춘다) | high |
| `[B-6b12]` | 문서(헤더 §7) + 레인 셋(`lanes.py`, `C:\CAL\l1`~`l3`) + `Sim.PhaseTwoExit`의 맞는 시드 찾기(지금은 `WithoutB6bData` 표로 돈다) + B6b 변이 전체 재실행(미뤄 둔 108 + 102) + 재감사; 성경 구절 대조는 개발자 몫 | `SCR\b6b12\brief_final.md`, `lanes.py`, `plan.py` | xhigh |

**다음 단계 고르기**: 마지막 `[B-6b*]` 커밋이 끝난 단계다. 다음은 `STATUS.md` '다음 3작업'의 **2번 'B6b 구현'**에서 '끝' 목록 다음에 적힌 단계다(1번은 개발자의 손 확인이라 이 세션의 일이 아니다). `[B-6b9]`는 커밋 없이 건너뛸 수 있어 `[B-6b8b]` 다음이 곧 `[B-6b10]`일 수 있다 — 노트 §5 `[B-6b8]` 칸의 판단 결과로 정한다.

**한 단계의 차례**(Claude Code가 돌린 꼴 — 도구가 달라도 같은 차례로 한다):
1. 시작: 지침서의 사실을 HEAD에 대어 보고 달라진 것을 `brief_working.md`에 적는다; 도구의 자체 시험(`*_selftest.py`)을 돌린다.
2. 구현: 데이터(`Tools/data/*.json`·`ST_CA.csv` — 에셋은 러너 `-Data`가 다시 짓는다; 새 `DT_Rules` 칸·열거자는 **빌드 먼저**, 그다음 `-Data`)·스키마·C++ 테스트·문서·변이 패치(`Tools/mutations/B6b/` + `Tools/mutations/README.md` 줄).
3. 빌드와 탐침: 에디터를 닫고 전체 빌드, 러너 `-Tier smoke [-Data] [-UpdateBaseline]`, 시드 42 다섯 판을 `Saved\Balance\<단계>_on`에(지침서의 명령), 지침서의 깨질 수 있는 확인 ⓐ~ⓒ.
4. 리뷰: 지침서·헤더가 이름한 관점(데이터와 글·신학, 결정론과 기준값, 테스트와 변이, 측정)으로 따로 읽고, 발견마다 반박을 시도한 뒤 고친다(데이터·측정·시뮬이 바뀌면 3을 다시). 헤더나 `WORKPLAN.md`가 ultracode(여러 에이전트) 리뷰를 요구하는데 이 도구로 돌릴 수 없으면, 관점마다 새 대화나 새 패스로 따로 읽는 리뷰를 하고 기록에는 'ultracode'가 아니라 '관점 N의 단독 리뷰(ultracode 대신)'라고 적는다 — 최종 보고에도 한 줄로 알린다.
5. 변이(에디터를 닫고, **변이를 걸 작업 사본의 뿌리에서** — 도구가 지금 폴더를 프로젝트로 삼는다): C++·에셋 패치는 `python -I -X utf8 <SCR>\b6b\run_mutations2.py Tools/mutations/B6b/<이름>.patch=Cosmic_Ascension.<테스트> …`(필터는 README 행). 파이썬 패치는 B6b 폴더만 `python -I <SCR>\b6b\run_py_mutations.py <이름>.patch …`, 다른 폴더(B3·B4·B7x 등)는 `python -I -X utf8 <SCR>\b6b8\fix\run_py_paths.py <뿌리> <저장소 밖 빈 작업 폴더> kill @<목록 파일>`(목록은 `rerun_set.txt`의 `=python:` 줄 꼴). 살아남으면 **테스트를 고친다(패치가 아니라)**. `run_mutations2.py`는 2026-11-10부터 C++ 패치를 되돌린 뒤 데이터 패치 앞에서 깨끗이 다시 짓는다(그 전에는 되돌린 변이 DLL 위에서 데이터 패치가 돌 수 있었다 — `[B-6b8b]`가 찾았다). 멈췄으면 `<SCR>\b6b\restore_tree.py <스냅숏>`; 끝나면 `Rebuild.bat`로 깨끗이 다시 짓고 바뀐 파일에 CR이 없는지 본다. **변이는 마지막 러너와 묶음보다 먼저** — 잰 빌드와 커밋하는 빌드가 같아야 한다.
6. 마지막 러너: `-Tier full [-Data] [-UpdateBaseline]` 한 번 → `fix_seed42.py`(인자 없이) → 기준값이 탐침의 다섯 판과 같은지 `python -I -X utf8 <SCR>\b6b8\check_base.py . Saved\Balance\<단계>_on`(저장소 뿌리에서, 마지막 줄 ALL SAME).
7. 묶음(있으면): 지침서의 명령 그대로, 보관 폴더는 새 이름으로.
8. 기록: 노트 §5의 그 단계 칸, 헤더 §11 한 줄, `DECISIONS_LOG.md`, 닿은 `docs/`(20·21·30·40·70), `Tools/mutations/README.md`, `STATUS.md`('지금'·'다음 3작업'·코드 사실·기준값), 파이썬 검사 다섯, 크기 한도.
9. 커밋·푸시. 이 파일의 상태 줄도 고친다.

**`[B-6b8]`을 이어 받을 때**(Claude Code가 커밋 전에 멈췄다면): `SCR\b6b8\`의 `final_full.txt`·`final_build_sha1.txt`(마지막 러너), `run_b\bundleB_out.txt`(묶음 진행 — 끝이 `bundle B: PASS`면 묶음은 끝났다), `bundleB\summary.json`·`summary.md`(묶음의 수), `records_draft\placeholders.md`(기록의 `<<B:…>>` 자리마다 넣을 값과 출처), `fix\handoff.md`(리뷰 고침의 넘김)를 본다. 남은 일: ① `<<B:…>>`를 다 채운다(`git grep -n "<<B:"`와 `commit_b6b8.txt` 안이 0); ② 창 없는 화면 한 장(세이브는 백업하고 되돌린다); ③ 기록을 한 번 더 읽어 수가 `summary.json`과 같은지 본다; ④ `STATUS.md`; ⑤ `commit_b6b8.txt`의 사본에서 트레일러를 이 세션의 것으로 바꿔 커밋·푸시.
- 묶음 B가 끝나지 않은 채 멈췄으면 하네스는 이어 돌지 않는다: 에디터·UBT가 없는지 보고 → `python -I -X utf8 <SCR>\b6b8\run_b\check_guard.py`로 빌드 sha1이 `final_build_sha1.txt`와 같은지 본다 → 멈춘 `bundleB\`를 `bundleB_stopped\`로 옮기고(보관 폴더 `Saved\Balance\b6b_bundleB`가 생겼으면 지우지 말고 이름을 바꾼다) → 저장소 뿌리에서 `python -I -X utf8 <SCR>\b6b8\bundleB.py --t-death 5000 --r 0.375 --folder <SCR>\b6b8\bundleB --serial-logs <SCR>\b6b8\run_b\serial_logs --a-sec-per-20k 12.8 --w-all-cells --workers 4`(540판, 약 40분; `--w-all-cells`는 빼지 않는다; 다른 빌드·커맨드렛을 같이 돌리지 않는다). 다시 읽기만 할 때는 `--check-only <SCR>\b6b8\bundleB --w-all-cells … --summary-dir <새 폴더>` — `--summary-dir` 없이 보관 폴더를 읽지 않는다(요약을 덮어쓴다).

## 5. B7x — 조합형 존재

이 절의 '헤더'·'노트'는 B7x 것이다: `SCR\b7x\int\docs\60_DECISIONS\headers\B7_composable_existence_and_evolution.md`와 `…_notes.md`(파일 이름이 `B7_`다 — main에는 B6b 뒤 `[B-7x0]`이 들어올 때까지 없다). B7x 단계에는 B6b처럼 미리 쓴 지침서가 없다 — 단계마다 헤더 §8의 '증명할 것', §5의 판단 ⓪~⑧, §6의 테스트, 노트 §5.4의 넘김을 읽고 `SCR\b7x\<단계>\brief_working.md`에 차례를 먼저 적은 뒤 §4의 '한 단계의 차례'를 따른다.

### 5.1 무엇이 정해졌나

- 다른 PC(엔진 없음)에서 만든 브랜치 `design/b7x-composable-existence`(바탕 `cef8cc7`)에 헤더·노트·설계서·빌드 전 코드·데이터 초안이 있었다. ultracode 적대 리뷰(발견 63)를 반영했고, **개발자가 2026-11-10 결정 전부를 '권장대로' 답했다**(헤더 §10). 답을 반영한 끝은 `79f2583`(푸시됨) — **이 브랜치는 동결한다; B7x 문서는 이제 `integrate/b7x`에서만 고친다.**
- 받은 답의 요점: 잎 이름 여덟(`KeptLaw`·`IdleFeed`·`FoldedSmall`·`ShutSphere`·`DustBand`·`InwardHunger`·`StarTow`·`ClosedPort`, 노브 `BrickIdleInsightPerTick`, 함수 `IsIdleOf`); 남긴 법(`KeptLaw`)은 좁힌다(주장·위기에 닿는 칙령과 법칙을 묻는 행의 백성 칙령은 여느 AI처럼 거둔다); 끝의 꼴 낱말은 저장 없이 마지막 재위 백성이 살아 있을 때 그 생애의 장에만; 둘째 갈림 ②에 되풀이 이득 없음; 끌어가는 블록은 정착이 끝난 틱에; 판 도중 탄생은 늘 새 백성(불사·유령·부활 낱말 금지, 절멸에서 태어나지 않음); 원안 설계서와 `OVERVIEW_as_of_B6a.html`은 `docs/archive`의 동결본; `promptforother.txt`는 지웠다. 리드의 판단 하나: 탄생 요청 필드는 지금 예약하지 않는다(개발자가 뒤집을 수 있다).
- **순서 규칙**(헤더 §8): B6b가 끝난 main 위로 다시 쌓는다. main에는 B6b가 끝나기 전에 B7x 커밋을 올리지 않는다. B7x 노브와 B6b 값이 어긋나면 **B7x 노브가 먼저 물러선다**(B6b의 r·c·N·P_bind·C는 B7x 때문에 바꾸지 않는다).

### 5.2 통합 브랜치 `integrate/b7x` (작업 사본 `SCR\b7x\int`; 빌드 전)

`git -C <SCR>\b7x\int log --oneline 8b6c5a7..HEAD`로 본다(main을 더 앞으로 다시 쌓았으면 `main..HEAD`). 2026-11-10 12:40 기준 — main `8b6c5a7` 위의 다섯 커밋(헤더 §8의 마지막 꼴):

| 커밋 | 내용 |
|---|---|
| `[B-7x0]` | 승인 문서 — 헤더·노트·받은 설계서·동결본 둘, `systems.md`의 번복 표지, B2 헤더 표지, `SECTION_MAP` 세 줄(살아 있는 문서는 아직 — §5.3) |
| `[B-7x1]` | 필드와 장치(시간 셈 여섯·`HostCivID`·`Brick.` 태그 열일곱·`CosmicBricks`)·잎 이름 여덟·리뷰 넘김과 그 고침을 하나로 — 두 해시를 옮긴다(새 저장 필드; 세계는 그대로) |
| `[B-7x4m]` | 측정 — 정책의 건너뜀·`-WithoutB7xData`·`-Crossroads`·`bricks:` 줄·`-SaveAt` 거절·도우미와 걸림줄 |
| `[B-7x3]` | 표현 — 끝의 꼴 낱말(결정 13 ㉢)·패널의 백성 줄·스캔 얼굴·외교 툴팁의 블록 항·`Shape.*` 키와 낱말 규칙 |
| `[B-7x4]` | 글 — 갈림 낱말 규칙·DevComment 열일곱·헤더와 설계서의 글 |

- 끝(`[B-7x4]`)에서: 빌드 경고 0, 테스트 238종 × 두 판 실패 0, 다섯 판의 CSV가 main `8b6c5a7`과 같고(시간 칸 빼고) 요약 줄이 해시를 가리면 같다. 새 해시 쌍(세계는 그대로, 새 저장 필드 때문): 3,000 2172948422 / 1868952007, 통치자 3,000 1023765488 / 3511480669, 방랑자 6,000 3662401041 / 309340336, 20,000 195851862 / 1941613769, 통치자 20,000 3035997636 / 3729142648 — main의 세계가 B6b 뒤에 옮으면 다시 잰다.
- 커밋마다 파이썬 검사 다섯이 통과했다(`walked` 330 / 372 / 372 / 372 / 402). 커밋마다의 빌드·테스트 증명은 B6b 뒤 마지막 다시 쌓기에서.
- 뒷일(2026-11-10 13:55, 각 커밋에 접어 넣음): README의 낡은 수, main `[B-6b8]`의 새 시험 판 `CosmicSideRowTests.cpp`에 블록 도우미(`[B-7x1]`), 신정론 `TheodicyProblems`의 기생 항과 변이 `b7x2_parasite_bonus_400`(`[B-7x1]` — 역병의 가장 긴 머묾 350, 덤 330까지 통과·331부터 깨짐), B7x 위에서 멈추는 main의 `b6b8_` 패치 여덟을 `b7x2_`·`b7x4m_`·`b7x4_` 이름으로 다시 뜸.
- 백업 브랜치: `integrate/b7x-on-220129d`(옛 아홉 커밋), `tmp/b7x-plain-rebase1`(증명용) — 병합 뒤 지운다.
- B7x 데이터 커밋(`[B-7x5s]`·`[B-7x5c]`)은 이 위에 B6b 뒤 이식 스크립트로 든다.

- **커밋되지 않은 변경이 있으면** 앞 세션의 일이 도중에 멈춘 것이다. 지우지도 stash하지도 않는다: `git -C <SCR>\b7x\int diff > <SCR>\b7x\stopped_unstaged.diff`, `git -C <SCR>\b7x\int diff --cached > <SCR>\b7x\stopped_staged.diff`로 남기고, `SCR\b7x\stage2\`·`stage3\`의 가장 최근 파일과 노트 §5.4로 남은 일을 정한다.
- **B6b가 끝나기 전에 여기서 해도 되는 것**: (1) 첫 컴파일·테스트, (2) `[B-7x4m]` 측정(정책의 건너뜀·`-WithoutB7xData=1/2`·`-Crossroads=<k>`·`bricks:` 줄·`-SaveAt` 거절 목록 — 헤더 §5), `[B-7x3]` 표현(연대기의 꼴 낱말 규칙 (a)~(d), 패널의 백성 줄, 스캔 얼굴 `log_brick_scan_spread`, 외교 창 툴팁), `[B-7x4]` 글(감사한 글 `<SCR>\b7x\stage3\texts\audited_texts.md` + **리드 판정 `<SCR>\b7x\stage3\texts\lead_rulings.md`가 앞선다**: 목소리 기울기는 main 관례대로 남기고, C10 제목은 '손대지 않은 칙령' — '거두'가 `soul_verbs`라서)의 코드·글 커밋. 시3(`TheodicyProblems`)은 `[B-6b8b]`가 main에 든 뒤.
- **데이터 이식 스크립트**(B6b 뒤에 쓴다): `SCR\b7x\port\port_b7x5s.py <작업 사본>`(노브 값·씨앗 블록 — `[B-7x5s]`), `port_b7x5c.py <작업 사본>`(갈림 카드 열둘·사건 행 아홉·Society·스키마 규칙과 시험 — `[B-7x5c]`). 감사한 글과 리드 판정을 이미 담았고 두 번 돌려도 같다; 확인 기록 `SCR\b7x\port\port_check.md`(따로 쓴 검사기 `verify_port.py`가 0건). 주의: ① 실제 LF 작업 사본에서 돌린다(`git archive`로 내보내면 이 PC에서는 CRLF가 된다 — 내보낼 때는 `-c core.autocrlf=false -c core.eol=lf`); ② 스크립트는 정해진 글 자리(테스트 블록 등)를 찾아 쓰므로 리베이스 뒤 그 글이 바뀌었으면 거절한다 — 먼저 `--dry-run`; ③ `port_b7x5c.py` 뒤 `Tools/test_data_schema.py`의 `walked` 핀을 테스트를 돌려 다시 센다(`db9bf9f`에서는 388 → 465, 차이 +77); ④ 이 세션의 임시 폴더가 없으면 백업의 `b7x\port\*.py`와 `b7x\answers\rename\rename_leaves.py`를 쓰고 `port_b7x5c.py --no-source-check`.
- **B6b가 끝나기 전에 하지 않는 것**: `[B-7x5s]`·`[B-7x5c]` 데이터(브랜치 `aa09543`의 몫)를 커밋하거나 사본으로 켜서 재는 일, 헤더 §8의 '값보다 먼저 재는 것'. 셋 모두 `[B-6b12]`가 main에 든 뒤, 끝난 main 위로 리베이스한 `[B-7x4m]` 빌드에서 시작한다.
- **첫 컴파일·테스트 — 끝났다(2026-11-10 08:40, `1333de4`)**: 다른 PC에서 엔진 없이 쓴 코드가 **고칠 것 없이** 에디터·게임 타깃 모두 경고 0으로 컴파일됐고(바뀐 37개 파일을 따로 컴파일해도 통과), 러너 `-Tier full -Data -UpdateBaseline`이 통과했다(테스트 234종 × 두 판 실패 0 — 223 + B7x 11). 데이터를 끈 다섯 판은 해시만 옮고(새 저장 필드) 요약 줄·CSV(`TickMicros`·`SaveBytes`·`SaveRawBytes` 빼고)·`race:`·`ending:` 줄이 `220129d`와 글자 그대로였다. 생성물(`seed42.json`·`generated.sha1`·에셋)은 되돌렸다 — 이것은 증명이 아니라 미리 본 것이고, 증명은 §5.3에서 끝난 main 위로 다시 쌓은 뒤 한다. 기록은 `SCR\b7x\compile1\`.
- **통합 작업 사본을 빌드할 때 꼭 알 것**:
  - 경로가 길어 UBT가 거부한다(260자). **정션 `C:\Users\rhwwl\AppData\Local\Temp\b7xi` → `SCR\b7x\int`**를 만들어 두었다 — 빌드·러너·에디터는 늘 이 정션 경로로 부른다(예: `Remove-Item Env:PROJECT_ROOT -ErrorAction SilentlyContinue; & 'C:\Users\rhwwl\AppData\Local\Temp\b7xi\Tools\run_checks.ps1' -Tier full -Data`). 긴 경로로 한 번이라도 빌드하면 `Intermediate`가 긴 경로를 기억해 계속 멈춘다 — 그때는 그 작업 사본의 `Intermediate\`를 지운다(git이 무시하는 폴더). 정션이 사라졌으면 `cmd /c mklink /J C:\Users\rhwwl\AppData\Local\Temp\b7xi <SCR>\b7x\int`.
  - main 쪽에서 에디터나 `-game`이 떠 있으면 Live Coding 가드가 다른 작업 사본의 에디터 빌드를 막는다. 통합 작업 사본에는 `Saved\UnrealBuildTool\BuildConfiguration.xml`(git이 무시)에 `bAllowHotReloadFromIDE=false`를 적어 두었다; 그래도 막히면 `Build.bat`에 `-NoHotReloadFromIDE`.
  - 에셋은 이미 LFS에서 꺼냈다(`git lfs checkout`). 새 작업 사본이면 그것부터, 그리고 **빌드 먼저, 그다음 `-Data`**(`make_data_tables`는 게임 모듈의 행 구조체를 쓴다).
  - B6b가 측정하는 동안에는 빌드하지 않는다(`bundleA`·`bundleK`·`bundleC`·`lanes` 같은 하네스는 다른 UE 프로세스가 돌면 시작하지 않는다; 메모리 31 GB).
  - 미리 보는 빌드의 생성물(`seed42.json`·`generated.sha1`·에셋)은 커밋하지 않는다 — `git -C <작업 사본> -c core.eol=lf checkout -- <파일>`. 고침은 소스만 커밋한다.

### 5.3 B6b가 끝난 뒤의 차례 (헤더 §8의 표)

1. 읽기만: `git merge-tree`로 끝난 main과 `integrate/b7x`를 견주어 충돌 자리를 본다(예상: `ST_CA.csv` 끝, 스키마 `words`·규칙의 끝, `DT_Events`·`DT_Encounters` 끝, `rule_setups`·`fixture_tables`, `walked` 핀 다시 셈, `[B-6b8b]`가 고친 `TheodicyProblems`, `docs/20`·`21`, 멈추는 변이 패치 — 다시 뜰 때는 옛 패치를 그 이름으로 고쳐 쓰지 않고 `b7x…_` 새 이름으로 뜬다).
2. `integrate/b7x`를 끝난 main 위로 리베이스하고 헤더 §8의 커밋 차례로 정리한다: `[B-7x0]` → `[B-7x1]`(필드+장치+이름+넘김+고침을 하나로 — 두 해시를 한 번만 옮긴다) → `[B-7x4m]` → `[B-7x3]` → `[B-7x4]` → `[B-7x5s]` → `[B-7x5c]` → (판단이 부르면) `[B-7x5r]`.
   - **`[B-7x0]` 커밋에 살아 있는 문서를 함께 넣는다**(헤더 §7 '승인 커밋'과 노트 §5.4 `[B-7x0]` 6의 글): `STATUS.md`, `DECISIONS_LOG.md`의 B7x 절, `OPEN_QUESTIONS.md`의 #36 ②·#88 고침과 새 행(번호는 `[B-6b12]`가 남긴 마지막 번호 다음), `docs/50`의 B 행과 수량('B6b 끝의 수 + 청원 11·조우 1'), `CLAUDE.md`의 `docs/archive/` 행(바이트가 늘지 않게). 코드는 넣지 않고 `check_docs` 0과 `CLAUDE.md` 크기로 증명한다.
   - 헤더 §11: 합칠 때 `[B-7x1+2]`의 빌드 전 줄들을 `[B-7x1]` 한 줄로 고쳐 쓰고(새 두 해시·러너 결과·변이 수), 절 머리의 '모두 빌드 전이다'도 고친다. 그 뒤 커밋마다 한 줄.
3. **빌드 전 커밋은 그대로 main에 오르지 않는다.** 다시 쌓은 커밋 하나하나를 그 커밋을 체크아웃한 채로 빌드하고 러너 full을 통과시킨 뒤에만 다음으로 간다(끝만 시험하고 빨리 감지 않는다). 커밋마다: 헤더 §8의 '증명할 것'(해시만 옮는 `[B-7x1]`은 다섯 판 CSV가 앞 커밋과 같음 + 그 커밋의 하나뿐인 `-UpdateBaseline`), 새 단언의 변이(`Tools/mutations/B7x/`를 실제로 죽여 본다), 헤더 §11 한 줄. `[B-7x5s]`·`[B-7x5c]`는 헤더 §5의 미리 적은 판단 ⓪~⑧(⑥은 B6b 경주의 다시 잼)과 밸런스 리뷰.

### 5.4 손 확인 (개발자 몫 — 막지 않는다)

B1~B6a(B6a ①③⑨은 빌드 `5455e56`이 든다 — `[B-6b12]` 지침서가 `C:\CAL\b6a` 작업 사본을 말한다), B6b 헤더 §9, B7x 헤더 §9(`[B-7x5c]` 뒤).

### 5.5 병합과 브랜치 정리 (개발자 요청 2026-11-09: '적용 다 했으면 남은 브랜치들은 다 없애고 메인에 병합')

'적용'은 `[B-7x5c]`(필요하면 `[B-7x5r]`)까지 main에 든 것이다 — 슬라이스 나·다(레인 창구, 탄생)는 명세만 있는 뒤 블록(B8·6a)이라 이번 병합에 들지 않는다.

1. main을 다시 쌓은 `integrate/b7x`로 빨리 감기(fast-forward)하고 푸시한다.
2. 지우기 전에 본다: `git diff --name-status cef8cc7 79f2583`과 `git diff --name-status 9be5b3e~1 9be5b3e`의 각 경로가 main에 있는가. 없으면 일부러 옮기거나 지운 것(`promptforother.txt` 삭제, 설계서 원안·`OVERVIEW_as_of_B6a.html`은 `docs/archive/`)인지 대조하고 결과를 마지막 커밋 본문이나 보고에 한 줄로 적는다.
3. `git push origin --delete design/b7x-composable-existence design/existence-evolution`, 로컬 브랜치 `design/b7x-composable-existence`·`integrate/b7x` 삭제, `git worktree remove`로 `SCR\b7x\wt`·`SCR\b7x\int`(그리고 남았다면 `C:\CAL\l1`~`l3`·`C:\CAL\b6a` — 지우기 전에 B6b 노트 §5에 그 줄이 있는지 본다), `git worktree prune`.

## 6. 측정·값의 지금 (자세한 것은 `STATUS.md`의 '기준값'과 B6b 노트 §2·§3)

- 값: r 0.375, P_bind 1,800, N 1,200, 각성 몫 0.5, T_DEATH 5,000, 지킴 몫 0.5, 시계 C 9,000.
- 시드 42 기준값 다섯 줄은 `Tools/baselines/seed42.json` — `[B-6b10]` 뒤: 3,000 2557850423 / 2906170550, 통치자 3,000 1189870756 / 3359656751, 방랑자 6,000 2158489212 / 1214038630, 입력 없는 20,000 4192287602 / 2387995655(기록 OtherPeople 7,950 civ_8), 통치자 20,000 1375149537 / 510604211(기록 RuledPeople 5,899); 게임의 해시 판 둘은 3,000 줄과 같다. `-WithoutB6bData=1`이면 다섯 줄 모두 `[B-6b5]`의 값.
- 자동화 테스트 227종·검사기 단위 테스트 96·스키마가 걷는 수 330(`4448705` 기준) — B6b 끝까지 늘 수 있다.

## 7. 지난 교훈 (같은 실수를 하지 않는다)

- `git apply`·PowerShell이 CRLF를 쓴다 — 커밋 전 CR 확인.
- `UnrealEditor-Cmd`는 시작마다 UBT `-Mode=ValidatePlatforms`(SDK 조회)를 스스로 띄운다 — 판마다 바쁨을 검사하는 도구는 그 `dotnet` 줄을 빼야 한다(`[B-6b11]` 첫 묶음이 324/620에서 멈춘 까닭; `SCR\b6b11\bundleC.py` `_busy_others`; `lanes.py`에도 같은 예외가 든다).
- main 작업 사본의 `git status`가 많은 파일을 ` M`으로 보이는데 `git diff --quiet`가 0이면 줄끝만 다른 낡은 색인 정보다(2026-11-10 15:40에 229개 — 파일은 LF, 색인에 적힌 크기는 CRLF). 내용 차이가 없음을 그 명령으로 확인한 뒤 `git add -u`로 색인만 새로 고친다(커밋할 것은 생기지 않는다).
- `make_data_tables`는 검사기 테스트를 먼저 돌린다; 새 `DT_Rules` 칸·열거자는 빌드 먼저, 그다음 `-Data`.
- `-Tier smoke`는 테스트 전부를 돌리지 않는다(어떤 테스트는 full에서만 깨졌다).
- 게임 데이터가 바뀌어 옛 변이 패치가 살아남으면 테스트를 고친다, 패치가 아니라.
- 측정 하네스의 `--check-only`는 `--summary-dir` 없이 보관 폴더의 요약을 덮어쓸 수 있다.
- 감사·리뷰 지시에 헤더보다 넓은 '구속 규칙'을 덧붙이면 그대로 적용된다 — 헤더의 받은 글만 쓴다.
- 커맨드렛 `.h`의 클래스 주석은 UHT 문자열 하나(MSVC 16,380바이트 한계) — 자세한 것은 `docs/40`.

## 8. 시작 확인 (지금 상태를 알아내는 명령)

```
git -C "C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension" log --oneline -12
git -C "C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension" -c core.quotepath=false diff --numstat
   (0 0이 아닌 줄만이 진짜 변경)
git -C "C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension" status --short --untracked-files=all
git -C "C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension" worktree list
git -C "<SCR>\b7x\int" log --oneline 8b6c5a7..HEAD     (main 위로 다시 쌓은 뒤에는 main..HEAD)
git -C "<SCR>\b7x\int" status --short
```

- 다음 단계는 §4의 '다음 단계 고르기'로 정한다.
- **`wip/<단계>` 브랜치가 있으면**(`git branch --list "wip/*"`) Claude Code 세션이 단계를 도중에 멈추며 그 일을 커밋해 둔 것이다 — 개발자가 '멈출 땐 다 커밋'을 요청했고, 러너를 통과하지 않은 일은 main에 올리지 않으므로 그 브랜치에 둔다. main은 마지막 커밋 그대로다. 이어 받을 때: `git switch wip/<단계>`가 아니라 main에서 `git -c core.eol=lf cherry-pick --no-commit wip/<단계>`로 작업 사본에 펼치고(CR 확인), 그 단계의 차례를 이어 간다; 단계를 커밋한 뒤 `git branch -D wip/<단계>`. 남은 일은 `STATUS.md`의 '지금'과 이 파일 상태 줄에 적혀 있다.
- main에 진짜 변경이 있는데 커밋이 없다면 그 단계가 도중에 멈춘 것이다. 그 단계의 `SCR\<단계>\`에서 가장 최근 파일(`fix\handoff.md`, `final_full.txt`, `mut\` 등)과 `brief_working.md`를 보고 어디까지 했는지 정한다.
- **되돌리기**: 변이 패치가 걸린 채 멈췄으면(소스에 변이가 남음) 그 단계의 스냅숏으로 `python -I -X utf8 <SCR>\b6b\restore_tree.py <스냅숏 폴더>`(저장소 뿌리에서), 스냅숏이 없으면 그 패치만 거꾸로(`git apply -R <patch>`) — 그 뒤 CR 확인과 다시 빌드.

## 9. 클라우드 세션 (GitHub만 — UE·Windows·이 PC의 임시 폴더 없음)

2026-11-10 15:50에 개발자가 'PC 없이 클라우드에서 GitHub만 연결해 진행'을 물었다. 그때를 위해 GitHub에 둘을 올렸다:

- `origin/handoff/cloud` — 이 파일(`HANDOFF.md`)과 `scratch/`(= 본문의 `SCR`, `Saved\Handoff\scratch`의 사본: 지침서·하네스·이식 스크립트·감사한 글·리드 판정). main과 이어지지 않는 고아 브랜치라 병합하지 않는다. 받기: `git fetch origin handoff/cloud` → `git worktree add ../handoff origin/handoff/cloud`(또는 `git show origin/handoff/cloud:HANDOFF.md`). 본문의 Windows 경로(`C:\...`, 정션 `b7xi`, `C:\CAL\...`)와 `SCR\b6b8\bundleB\`의 CSV는 이 PC에만 있다.
- `origin/integrate/b7x` — B7x 통합 브랜치(main `8b6c5a7` 위 다섯 커밋, 끝 `279b353`; §5 표).

**클라우드에서 되는 일**(UE 없이 — 검사는 `bash Tools/run_checks.sh`, 파이썬 3의 검사 다섯):

1. **B7x를 main(`4448705`) 위로 다시 쌓기** — `integrate/b7x`를 받아 `git rebase origin/main`(이 PC의 rerere 기록은 없다 — 충돌은 손으로; main 쪽 변경: `[B-6br]`의 `Data/CosmicDataTables.h/.cpp`·`Tests/CosmicTableNamesTests.cpp`·`Tools/run_checks.ps1`, `[B-6b10]`의 `DT_Rules`·`EndingClockAt`을 `Tests/CosmicTestHelpers.h`의 `CosmicTests`로 옮김·`Sim.SaveOldEndFlows`·`docs/30`과 B6b 헤더·노트). 커밋마다 `run_checks.sh` 통과, `Tools/test_data_schema.py`의 `walked` 핀은 그 테스트가 말하는 수로 다시 센다, `[B-7x1]`·`[B-7x4]` 커밋 본문의 낡은 '다시 뜬다' 문장을 고친다, 줄끝 LF. 결과는 **`cloud/b7x-on-4448705`** 브랜치로 푸시한다(`integrate/b7x`는 덮지 않는다). C++는 컴파일해 보지 못하므로 이 PC 세션이 받아 빌드·테스트 두 판·`[B-7x1]` 해시 쌍 다시 재기로 증명한 뒤 `integrate/b7x`로 옮긴다.
2. **문서 초안**(브랜치 `cloud/docs-<이름>`): `[B-7x0]`의 살아 있는 문서(§5 — `STATUS`·`DECISIONS_LOG`의 B7x 절·`OPEN_QUESTIONS` 새 행; 번호는 `[B-6b12]` 뒤에 정하므로 자리표 '새1'~'새11'로), 다음 블록 헤더의 조사·초안.
3. **개발자의 글 확인 — 끝났다**(개발자 2026-11-10 '권장대로'): `SCR\b7x\stage3\texts\lead_rulings.md` §2 — C10 제목 '손대지 않은 칙령'(받은 답 '거두지 않은 칙령'은 `soul_verbs`의 '거두'에 걸린다), 꼴 낱말 여섯, C12 제목 '집을 싣는 배'. 답이 바뀌면 `port_b7x5c.py`의 글 자리를 고친다.

**시작 문구**: 클라우드 세션은 `PROMPT_cloud_claude_code.md`, 이 PC의 `[B-6b12]`(Antigravity)는 `PROMPT_antigravity_B6b12.md` — 둘 다 이 파일 곁(`Saved\Handoff\`와 `origin/handoff/cloud`)에 있다. 클라우드 세션은 멈출 때 `handoff/cloud`에 `CLOUD_STATUS.md`를 커밋해 둔다 — 이 PC의 세션은 시작할 때 `git fetch origin` 뒤 `git show origin/handoff/cloud:CLOUD_STATUS.md`로 읽는다.

**클라우드에서 안 되는 일**(이 PC의 UE 5.8이 있어야): 빌드·자동화 테스트·러너 `run_checks.ps1`·커맨드렛·`make_data_tables`(에셋과 `generated.sha1`)·해시와 시간 측정·묶음 C·레인·변이·손 확인 세이브. 그래서 `[B-6b11]`(게임 620판)·`[B-6b12]`(레인·`Sim.PhaseTwoExit` 시드 찾기·변이 전체 재실행)와 B7x 데이터 커밋 `[B-7x5s]`·`[B-7x5c]`(B6b 뒤, 에셋을 다시 만든다)는 이 PC에서 한다.

**규칙**: 클라우드에서 main에 커밋·푸시하지 않는다(main은 러너를 통과한 커밋만 — §2). `Tools/data/*.json`·`Tools/cosmic_data_schema.py`·`schema.json`을 고친 커밋은 `Data.SourcesInSync`가 깨지므로 브랜치에만 두고 이 PC에서 `-Data`로 마무리한다. 두 세션이 함께 돌면 이 PC 세션은 main의 B6b만, 클라우드는 `cloud/*` 브랜치만 고친다(같은 브랜치를 둘이 고치지 않는다). 병합 뒤 `integrate/b7x`·`handoff/cloud`·`cloud/*`의 원격 브랜치도 지운다(§5 마지막 차례에 더한다).
