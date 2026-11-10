# B7x 데이터 포트 스크립트 확인 (2026-11-10)

두 스크립트(`port_b7x5s.py`·`port_b7x5c.py`)를 db9bf9f를 내보낸 스크래치 사본에서 돌렸다. INT는 건드리지 않았다(`git archive`만 썼다). 커밋·빌드·측정은 하지 않았다. 확인 중에 실패한 것이 없어 스크립트를 고치지 않았고, 그래서 try2도 만들지 않았다.

경로 줄임: `B7X` = `C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad/b7x`, `PORT` = `B7X/port`.

## 1. 사본 셋

| 사본 | 만든 법 | 쓰임 |
|---|---|---|
| `PORT/try1` | 지시 그대로 `git -C B7X/int archive --format=tar db9bf9f \| tar -x -C PORT/try1` | 이 PC에서는 **모든 텍스트 파일이 CRLF**로 나온다(시스템 gitconfig의 `core.autocrlf=true`와 `.gitattributes`의 `* text=auto`). 실제 작업 트리는 LF다(저장소 `.git/config`의 `core.autocrlf=false`). 그래서 CRLF를 지키는지 보는 데 썼다 |
| `PORT/try1lf` | `git -C B7X/int -c core.autocrlf=false -c core.eol=lf archive --format=tar db9bf9f \| tar -x -C PORT/try1lf` | 실제 트리와 같은 LF 사본. 검사를 모두 여기서 돌렸다 |
| `PORT/try0` | try1lf와 같은 법, 포트 없음 | db9bf9f 그대로. 견줄 기준 |

## 2. 결과

### 2.1 두 스크립트를 두 번씩 (try1lf와 try1 모두)

| 실행 | 결과 |
|---|---|
| `port_b7x5s.py` 1회 | 노브 16개 값을 바꾸고 `BrickStarEaterDimPeriodTicks`는 0 그대로. `BrickSeedTags`에 씨앗 8개. `test_existence_vocabulary`의 핀을 시작값 핀으로 바꾸고, docs/30을 다시 만들었다(`Brick` 줄 16개 바뀜). exit 0 |
| `port_b7x5c.py` 1회 | 38개를 바꿨다. 카드 12행, 사건 9행, `arch_seeker`의 `Society`, 규칙 8개(then-test 77), 테스트 쪽 4가지 추가와 기존 테스트 4곳 손봄. exit 0 |
| `port_b7x5s.py` 2회 | `Nothing to change ... (0 changes)`, exit 0 |
| `port_b7x5c.py` 2회 | `changes (0):`, exit 0 |
| CRLF 사본 | CRLF가 그대로 남았다. CRLF를 LF로 바꾸면 try1lf와 바이트까지 같다. 하나 다른 것: docs/30은 트리의 `make_config_reference.py`가 LF로 다시 쓴다. 실제 트리는 LF라서 문제없다 |

로그: `PORT/try1_logs/try1lf_5s_run1.txt`, `try1lf_5s_run2.txt`, `try1lf_5c_run1.txt`, `try1lf_5c_run2.txt`(같은 이름의 `try1_*`는 CRLF 사본).

### 2.2 try1lf에서 돌린 검사

| 검사 | 결과 |
|---|---|
| `python -I -X utf8 Tools/check_data.py` | `0 problem(s)` |
| `python -I -X utf8 Tools/test_data_schema.py` (핀 고치기 전) | 102개 중 실패 1개. `test_every_rule_of_the_schema_can_fail`이 `465 != 388`로 실패했고, 다른 실패는 없다 |
| walked 핀을 465로 놓은 뒤 두 번 | 두 번 모두 `Ran 102 tests ... OK`(db9bf9f에서는 100개, 포트가 2개 더한다) |
| `Tools/check_layers.py --strict` | `179 files, 0 error(s), 0 warning(s)` |
| `Tools/make_config_reference.py --check` | `up to date`. 노브 값이 docs/30에 나오므로 **docs/30을 다시 만들었다**. `port_b7x5s.py`가 직접 다시 만들고, 바뀐 줄은 `Brick` 16줄이다 |
| `Tools/check_docs.py` | `46 files, 0 error(s)`. 경로 길이 문제는 없었다. NOTE도 db9bf9f와 같은 230+10개다. 출력에 보이는 10줄이 다른 것은 집합을 도는 차례가 실행마다 달라서다 |
| CRLF 사본(try1): check_data · 핀 465 뒤 test_data_schema 1회 · config check | 0건 · 102 OK · up to date |

walked 핀은 스크래치 도우미 `PORT/set_pin_scratch.py`로 사본에만 놓았다. 주석 줄도 함께 넣었다: `# B7x [B-7x5c]: 388 + 77 (rule 4: 2 x 34; rule 5: 3; the three named brick lines: 6) = 465.`

### 2.3 db9bf9f와 견준 차이

바뀐 파일은 일곱이다. 더하거나 지운 파일은 없다. 일곱 모두 BOM이 없고 CR이 없으며 LF로 끝난다.

| 파일 | 바이트 | 줄 |
|---|---|---|
| `Tools/data/DT_Encounters.json` | 39944 → 82829 | +1770 |
| `Tools/data/DT_Events.json` | 10956 → 21730 | +414 |
| `Tools/data/DT_Civs.json` | 1906 → 1939 | ±1 |
| `Tools/data/DT_Rules.json` | 4326 → 4637 | ±16 |
| `Tools/data/schema.json` | 113079 → 123811 | +116 |
| `Tools/test_data_schema.py` | 393750 → 407197 | +123, 핀 포함 |
| `docs/30_CONFIG_REFERENCE.md` | 14965 → 15032 | ±16 |

포트 스크립트와 따로 짠 검사기 `PORT/verify_port.py`(포트 스크립트를 import하지 않는다)로 확인했다. 결과는 `verify_port: 0 problem(s)`이고 로그는 `PORT/try1_logs/verify_lf.txt`다. 확인한 것:
- **행의 차례**: 새 행은 표 끝에 aa09543의 차례대로 붙었다(카드 12, 사건 9). 그 앞의 행은 db9bf9f와 같고, 원래 바이트도 앞부분에 그대로 남았다.
- **한국어 글**: 21행의 글 칸(Title·Body·VoiceBodies·Choices의 Label·SuccessText·FailureText·LogTemplates)이 감사 블록(`audited_texts.md` 1·2절의 json)과 같다. 리드 판정으로 바뀐 것은 두 칸이다. C12 Title은 '집을 싣는 배'이고, C12 ②의 SuccessText는 aa09543의 '…{Civ}의 일꾼들이 당신의 손을 기억했다.'다. C10 Title은 감사 블록대로 '손대지 않은 칙령'이다. 새 행의 한국어 문자열은 모두 감사 블록이나 aa09543 둘 중 하나에서 왔다.
- **효과**: 효과 칸은 모두 aa09543에 잎 이름만 바꾼 것과 같다. 다른 것은 판정 JSON의 ② 넷뿐이다. C05는 `ChangeStability +0.05`·기울기 +0.05, C07은 `+0.04`·기울기 −0.05, C10은 `ChangeStability +0.05` 하나, C11은 `Insight 120`·`−0.02`·기울기 +0.05다. 기울기 부호는 aa09543과 같다. 감사 6.1이 뺐던 실패의 기울기는 남았다(예: C01 ① 실패는 `−0.06`·기울기 −0.05). C12 ②의 `ChangeReverence +0.05`도 남았다. 값·판정·조건·Trigger·무게도 aa09543과 같다.
- **DT_Civs**: 바뀐 것은 `arch_seeker`의 Traits에 `Brick.Form.Society`가 붙은 것뿐이다. aa09543의 Traits와 같다.
- **DT_Rules**: Brick 노브 17개의 값과 타입이 aa09543(옛 `BrickQuietInsightPerTick` → `BrickIdleInsightPerTick`)과 같다. 다른 키와 키 순서는 db9bf9f 그대로다.
- **schema.json**:
  - 이전 규칙과 표의 다른 키, 최상위 키(words·prefixes)는 그대로다.
  - DT_Encounters에 규칙 5개, then-test 71개가 붙었다. 모두 aa09543과 when/then이 같고, 규칙 4의 then은 칸·종류별로 17 태그를 새 이름으로 정렬했다.
  - 닫힘 규칙의 why만 aa09543과 다르다. 노트 5.4 데1이 정한 새 why를 글자 그대로 썼다.
  - DT_Events에 붙은 3개는 감사 7.3의 이름 규칙 셋과 같다(then-test 6).
- **꼴**: `DT_Encounters`·`DT_Events`는 db9bf9f처럼 탭 들여쓰기의 표준 dump(`json.dumps(indent="\t", ensure_ascii=False)` + LF)와 바이트까지 같다. schema.json은 주변 규칙과 같은 꼴이다(한 줄 when/then, 탭). 테스트 파일에서 더한 줄에는 탭이나 끝 공백이 없다.
- **옛 잎 이름**: `Source/`·`Config/`·`Tools/`에 옛 이름 9개(`QuietInsight` 포함)가 하나도 없다. 같은 사본에 `rename_leaves.py --dry-run`을 돌리면 `changes: 0 in 0 files`, `allow list (0)`이고 `try1lf_rename.txt`에 남았다. 옛 이름은 docs의 다섯 파일(헤더·노트·설계서·동결본 둘)에만 있다. 이것은 손으로 고치거나 기록으로 남기는 자리다.

검사기도 변이 넷으로 깨 보았다. SuccessText에 공백 하나 넣기, C01 ① 실패의 기울기 빼기, C12 제목을 '조선소의 갈림'으로 되돌리기, 사건 행에 `HermitMonolith` 되돌리기다. 넷 모두 PROBLEM을 냈다.

### 2.4 덤: 지금 다시 쌓는 중인 트리에서 dry-run

INT의 `b5b9e11`은 `8b6c5a7` 위의 `[B-7x1+2]`다. 이것을 LF로 내보내 두 스크립트를 `--dry-run`으로 돌렸다. 사본은 지웠고 로그는 `PORT/try1_logs/restack_5s_dry.txt`·`restack_5c_dry.txt`에 있다.
- `port_b7x5s.py`는 노브 16개를 바꾸고, 핀을 바꾸고, docs/30을 다시 만들겠다고 냈다(exit 0). B6b의 `8b6c5a7`까지 main이 Brick 노브를 건드리지 않았다는 뜻이다.
- `port_b7x5c.py`는 `test_crossroads_words ... place it by hand`로 거절했다(exit 1). 그 트리에는 아직 `[B-7x4]`가 다시 쌓이지 않았으니 맞는 거절이다. 헤더 8절의 차례로는 7번째 줄이 4번째 줄 뒤에 온다.

## 3. B6b가 끝난 뒤 다시 쌓는 세션이 돌릴 명령

차례는 헤더 8절의 차례 표와 같다. 에디터는 닫는다. `TREE` = 다시 쌓은 작업 트리(INT 워크트리나 main 저장소)다. 아래는 Git Bash 명령이고, 러너만 PowerShell이다.

### 3.0 다시 쌓기(데이터 커밋 전)
1. B6b의 마지막 커밋이 main에 든 뒤, 헤더 8.1의 충돌 해소대로 `[B-7x0]` → `[B-7x1+2]` → `[B-7x4m]` → `[B-7x3]` → `[B-7x4]`를 final main 위로 다시 쌓는다(각각 빌드와 러너를 돌린 뒤 커밋한다. 그 커밋들에는 B7x 데이터가 없다).
2. 스크립트가 고치지 않는 사본인지 확인한다: `git -C TREE status --short`가 비어 있어야 한다.
3. 소스 대조에 필요한 것이 이 PC에 있는지 본다: `git -C B7X/int cat-file -e aa09543 && ls B7X/stage3/texts/audited_texts.md B7X/stage3/texts/lead_rulings.md`. 없으면 `port_b7x5c.py`에 `--no-source-check`를 준다. aa09543은 `design/b7x-composable-existence`(로컬과 origin)에 있다.

### 3.1 `[B-7x5s]` 씨앗과 노브 — 데이터만, C++ 바뀜 없음, 제 커밋
```bash
python -I -X utf8 PORT/port_b7x5s.py TREE --dry-run      # 바뀔 것 16개 + 핀 + docs/30을 읽는다. 셋째 값 거절이 나면 멈추고 손으로 정한다
python -I -X utf8 PORT/port_b7x5s.py TREE
python -I -X utf8 PORT/port_b7x5s.py TREE                # 0 changes
python -I -X utf8 B7X/answers/rename/rename_leaves.py TREE   # changes: 0 in 0 files
cd TREE
grep -rnE "UnseenLaw|SilenceFeed|PlanckMicro|DysonBrain|KesslerTapestry|AbyssalMaw|StarShepherd|HermitMonolith|QuietInsight" Source Config Tools   # 0줄
python -I -X utf8 Tools/check_data.py                    # 0 problem(s)
python -I -X utf8 Tools/test_data_schema.py              # OK (walked 핀은 이 커밋에서 바뀌지 않는다)
python -I -X utf8 Tools/make_config_reference.py --check # up to date
python -I -X utf8 Tools/check_layers.py --strict
python -I -X utf8 Tools/check_docs.py
```
```powershell
powershell -ExecutionPolicy Bypass -File Tools\run_checks.ps1 -Tier full -Data   # 두 번. 에셋과 generated.sha1을 다시 만들고, 빌드·자동화 테스트·커맨드렛까지 돈다
```
같은 커밋에 넣을 것(스크립트가 하지 않는다) — 헤더 8절 차례 표의 6번째 줄:
- 세계가 옮는다(AI만). 깨질 수 있는 확인 ⓐ~ⓒ(=1), 값보다 먼저 재는 것과 문턱의 규칙, 판단 ⑥(B6b 다시 잼)을 한다.
- 6절의 뜻이 바뀌는 테스트를 `WithoutB7xData` 아래로 옮긴다.
- 변이 패치를 커밋 전후에 `git apply --check`로 견준다.
- 기준값을 다시 적는다.

그 뒤 `[B-7x5s]`를 커밋한다.

### 3.2 `[B-7x5c]` 카드·사건·Society — 따로 커밋
```bash
python -I -X utf8 PORT/port_b7x5c.py TREE --dry-run      # 바뀔 것 38개. 거절이 나면 그 자리를 손으로 맞춘다
python -I -X utf8 PORT/port_b7x5c.py TREE
python -I -X utf8 PORT/port_b7x5c.py TREE                # changes (0)
python -I -X utf8 B7X/answers/rename/rename_leaves.py TREE   # changes: 0 in 0 files
cd TREE
python -I -X utf8 Tools/test_data_schema.py              # walked 핀 하나만 실패한다: "<N> != <옛 값>"
```
walked 핀을 테스트가 다시 센 N으로 손으로 놓는다. db9bf9f 위에서는 388 → 465(+77)였다. B6b가 then-test를 더했으면 옛 값과 N이 함께 옮고, 차이는 여전히 77이어야 한다. 주석 줄은 `# B7x [B-7x5c]: <old> + 77 (rule 4: 2 x 34; rule 5: 3; the three named brick lines: 6) = <N>.`이다.
```bash
python -I -X utf8 Tools/test_data_schema.py              # 두 번 OK
python -I -X utf8 Tools/check_data.py
python -I -X utf8 Tools/check_layers.py --strict
python -I -X utf8 Tools/make_config_reference.py --check
python -I -X utf8 Tools/check_docs.py
python -I -X utf8 PORT/verify_port.py <TREE를 port_b7x5s 뒤·port_b7x5c 전에 내보낸 사본> TREE   # 기준이 db9bf9f가 아니면 DT_Rules 줄은 무시하거나, 기준을 [B-7x5s] 커밋으로 둔다
```
```powershell
powershell -ExecutionPolicy Bypass -File Tools\run_checks.ps1 -Tier full -Data   # 두 번
```
같은 커밋에 손으로 넣을 것:
- **문서**:
  - 설계서 4.3: C05·C07·C10·C11 ② 줄과 효과, C10·C12 제목, C12의 줄들.
  - 노트 5.4 데1·데2.
  - 헤더 11절: C10 제목이 개발자 답에서 벗어난 것('거두'는 `soul_verbs`).
- **테스트와 판단**:
  - 6절의 `[B-7x5c]` 테스트(`Sim.CrossroadsPresent`·`Sim.CrossroadsAnswer`·정책의 건너뜀·`Sim.KnowingInert`의 경우, C++ — 빌드가 든다)와 뜻이 바뀌는 테스트.
  - 판단 ⓪~⑤·③′·⑦·⑧과 ⑥ 다시, 그리고 변이 패치 견줌과 기준값.
  - ultracode 밸런스 리뷰.

## 4. 열린 점

1. `git archive`를 지시 그대로 쓰면 이 PC에서는 CRLF 사본이 나온다. 다시 쌓는 세션에서 사본을 만든다면 `-c core.autocrlf=false -c core.eol=lf`를 붙인다. 스크립트는 둘 다 CRLF도 지키지만, docs/30은 트리의 생성기가 LF로 쓴다.
2. 확인은 모두 db9bf9f에서 했다. final main 위에서는 두 스크립트가 정확한 글을 찾는 자리에서 거절할 수 있다. `port_b7x5s.py`는 `[B-7x2]`의 무동작 핀 블록을, `port_b7x5c.py`는 `test_crossroads_words`·`rule_setups`·`test_tables` 네 곳과 표 끝을 찾는다. 다시 쌓은 `[B-7x1+2]`(b5b9e11)에서는 `port_b7x5s.py`의 dry-run이 통과했다.
3. 두 스크립트와 `rename_leaves.py`는 이 세션의 임시 폴더에 있다. 다시 쌓는 세션이 다른 PC나 정리된 폴더라면 `PORT/*.py`와 `B7X/answers/rename/rename_leaves.py`를 먼저 옮겨 둔다. 그러면 `--no-source-check`가 필요하다.
4. 헤더 8.1에 적힌 붙일 자리는 `enc_witness` 뒤, `event_era_endured` 뒤, Rule W 뒤다. 스크립트는 '표의 끝'에 붙인다. final main의 표 끝에 다른 행이 더 들었다면(`[B-6b9]`~`[B-6b12]`) 그 행들 뒤에 붙는다. 이 차이는 가중 추첨의 차례에 닿지 않는다(차례는 행 이름 정렬이다 — 헤더 3.1). 다만 원하면 손으로 옮긴다.
