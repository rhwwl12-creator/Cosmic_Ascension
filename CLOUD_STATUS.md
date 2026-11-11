# CLOUD_STATUS — GitHub 쪽 일의 넘김 (넷째 세션, 2026-11-12)

> 이 파일이 클라우드 쪽 일의 지금 상태다. PC 세션은 `git fetch origin` 뒤 `git show origin/handoff/cloud:CLOUD_STATUS.md`로 읽는다. 앞 세션들의 기록은 `git show origin/cloud/docs-b7x0:docs/drafts/CLOUD_STATUS.md`(셋째 세션까지).
> 넷째 세션은 클라우드가 아니라 **이 PC의 Claude Code 세션**이 클라우드 시작 문구대로 git과 파이썬만으로 했다. UE·빌드·러너·커맨드렛은 돌리지 않았다 — 같은 시각 main 작업 사본에서 `[B-6b12]`의 변이 레인(`lanes.py`)이 돌고 있었고, 그 작업 사본과 `C:\CAL\l1`·`l2`는 건드리지 않았다.

## 바탕

- main = `467cfdb` `[B-6b11]`(`[B-6b12]`는 아직 커밋 전 — main 작업 사본에만 있다).
- 개발자의 답(2026-11-11 밤, 이 세션의 물음): 7 피난 = B6b 결정 21 그대로; 1 = 이음·조공도 B7 헤더 안에서 정한다(총괄 세션의 해석과 다름); A~G = 모두 권장대로. 글은 `Saved/Handoff/b7_next/`(저장소 밖, 원본은 저장소 뿌리의 추적하지 않는 `handoff_B7_next/`)와 아래 B7 헤더 초안의 머리.

## 한 일

1. **B7x 다시 쌓기 → `origin/cloud/b7x-on-rebase`** (끝 `84997fe`; `integrate/b7x`는 덮지 않았다)
   - `origin/integrate/b7x`(`279b353`, main `8b6c5a7` 위 다섯 커밋)를 `origin/main` `467cfdb` 위로: `9d3ecf7` `[B-7x0]` → `ea05044` `[B-7x1]` → `b4a1aaa` `[B-7x4m]` → `6ae3663` `[B-7x3]` → `84997fe` `[B-7x4]`.
   - 손으로 푼 충돌은 두 파일뿐: `Tools/mutations/README.md`(머리 문단의 데이터 패치 목록 — main의 `b6b10_data_clock_six_thousand`를 지킴; 표 끝 — main의 `b6br_`·`b6b10_` 행 뒤에 B7x 행)와 `docs/40_DEV_GUIDE.md`(`Data.Integrity` 행 — B7x 문장을 얹고 main의 `Data.SortedNamesFollowTable`·`Data.HazardAnswerAnyRow` 행을 그 아래; `UI.RunEnd` 행 — main의 '시계 − 3·시계(9,000)'; `UI.Chronicle` 행 — B7x 문장). 나머지는 저절로 합쳐졌다(`CosmicDataTables.h`·`CosmicDefinitionRows.h`·`CosmicTestHelpers.h`·`DT_Rules.json`·`docs/20`·`21`·`30`). 끝 트리는 `git merge-tree origin/main origin/integrate/b7x`와 그 두 파일만 다르다.
   - 커밋마다 파이썬 검사 다섯(`check_data`·`test_data_schema`·`check_layers --strict`·`make_config_reference --check`·`check_docs`) 통과, 커밋된 바이트에 CR 0. `walked` 핀은 손대지 않았다 — 테스트가 그대로 통과(372·372·372·402).
   - 변이 패치를 커밋마다 `git apply --check`로 견줬다: main의 새 열셋(`b6br_` 열·`b6b10_` 셋)은 B7x 커밋 모두에서 선다; B7x 쪽에서 새로 서지 않게 된 것 없음 — 다시 뜬 것 없음. README의 B7x 머리와 `docs/40`의 변이 문장 끝에 이 비교 한 문장을 더했다(`[B-7x4]`). `B7x/`는 92개.
   - 커밋 본문: 다섯 모두 '467cfdb 위로 다시 쌓음' 항목, `[B-7x1]`·`[B-7x4m]`·`[B-7x4]`의 낡은 '다시 뜬다' 문장과 패치 수를 지금 사실로 고쳤다.
2. **`[B-7x0]` 살아 있는 문서 → `origin/cloud/docs-b7x0`** (`453f7f0`, main `467cfdb` 위)
   - 셋째 세션이 패치로만 넘긴 일을 커밋으로 다시 했다: `OPEN_QUESTIONS.md`(#36 ②·#88 끝 문장, 표 끝에 새2~새11 — 노트 §3 글자 그대로), `DECISIONS_LOG.md`(B7x 절을 B6b 절 위에), `docs/50_ROADMAP.md`(B 행 작업 묶음·종료 조건 수량·§12.1 조우 행), `STATUS.md`('지금' 맨 위 항목, Top 5 1번 끝). `CLAUDE.md`는 `8435c9f` 그대로(16,308 LF 바이트).
   - 초안 둘의 `[B-7x5c](데이터)` 글자를 띄어 `check_docs`의 링크 오류 셋을 없앴다 — 이제 `check_docs: 45 files, 0 error(s)`, 검사 다섯 통과.
   - 번호: 아직 자리표 '새2'~'새11'. `[B-6b12]` 작업 사본의 마지막 물음이 #168(판의 길이와 비전의 호)이라 #169~#178이 될 것으로 보인다(확인은 `[B-6b12]` 커밋 뒤).

## PC가 할 것 (빌드·러너로 증명)

1. `[B-6b12]` 커밋 뒤: `cloud/b7x-on-rebase`를 그 main 위로 한 번 더 쌓는다(충돌 자리는 위와 같은 둘이 다시 날 수 있다 — README의 표 끝과 머리 목록, `docs/40`의 테스트 표). 그다음 HANDOFF §5.3의 차례 — 커밋마다 체크아웃한 채 빌드·러너 full(`[B-7x1]`은 새 저장 필드로 두 해시만 옮긴다 → 그 커밋의 하나뿐인 `-UpdateBaseline`, 다섯 판 CSV가 앞 커밋과 같음), `-Data`(ST_CA·DT_LogTemplates·schema·DT_Rules가 바뀌어 `generated.sha1`이 낡았다), C++ 변이(`Tools/mutations/B7x/`의 `Source/` 패치는 아직 한 번도 죽여 보지 않았다), 헤더 §11의 `[B-7x1]` 한 줄.
2. `cloud/docs-b7x0`의 `453f7f0`을 `[B-7x0]`에 접어 넣는다 — `OPEN_QUESTIONS`·`DECISIONS_LOG`·`docs/50`·`STATUS`는 `[B-6b12]`의 고침과 충돌할 수 있다(손으로). 새N을 실제 번호로, `git grep -n "새[0-9]"`가 `docs/drafts/` 밖에서 0, 그 뒤 `docs/drafts/`를 지운다.
3. 병합 뒤 원격 `cloud/*`·`integrate/b7x`·`handoff/cloud`를 지운다(HANDOFF §5.5).

## 이 PC에서 본 것 (알림)

- HANDOFF.md 상태 줄(2026-11-11 23:25)은 '`[B-6b12]` 작업이 `wip/b6b12` 브랜치에 보존됨'이라 적지만 그 브랜치는 없다(`git branch --list "wip/*"` 빈 답). 변경은 main 작업 사본에만 있다 — 그 세션이 멈추기 전에 커밋해야 한다.
- 저장소 뿌리의 추적하지 않는 `handoff_B7_next/`를 `.git/info/exclude`에 넣었다(`[B-6b12]`의 '추적하지 않는 파일 모두 스테이징' 명령이 집어 가지 않게). 사본은 `Saved/Handoff/b7_next/`.

3. **B7 헤더 초안 → `origin/cloud/docs-b7-header`** (`17c34c2`, 바탕 `cloud/b7x-on-rebase` `84997fe`)
   - 새 파일 `docs/60_DECISIONS/headers/B7_ai_peoples_hands.md`(95,610바이트) — 상태 '초안 — 승인 전'. 슬라이스 B7a 성향과 칙령·B7b 외교와 전쟁(#126을 연다 — 천도, 은혜는 다스리는 백성만)·B7d 이음·B7e 조공(칙령 행으로); B7c와 갈라짐은 따로 헤더.
   - 조사 셋(코드·문서를 따로 읽음) → 초안 → **ultracode 적대 리뷰**(관점 넷 — 설계와 범위·결정론과 저장·신학과 글·측정과 판단, 관점마다 반박자 하나; 발견 47 — 확인 28·일부 17·반박 2) → 남은 45건 모두 고침(헤더 §11). 리뷰 원문: 이 브랜치의 `b7_header_review.md`.
   - **개발자가 정할 것 넷**(헤더 §10): 14 다스리는 백성의 마지막 정착된 성계를 정복할 수 있나(은혜가 쓰인 뒤면 폐위 — B3의 '정복으로 폐위되지 않는다'를 뒤집음; 권장 = 연다), 22 구출 은혜를 역할을 묻지 않고 그릇의 죽을 자리에(지금 닿는 것은 방랑자의 죽음 — 신학; 권장 = 설계대로), 24 조공을 영압 카드가 아닌 칙령 행으로(설계 바꿈 — 빠지는 것: 특성 '굴복한'·초월자 관계 +0.2·위기를 걷음; 권장 = 칙령), 29 언약 공동체 행이 처음부터 `Trait.Faith.Remnant`를 지니나(권장 = 설계대로).
   - B7-0의 수는 `<<B7-0:…>>` 자리표 — PC가 B7x를 main에 다시 쌓은 뒤 잰다. 코드 없음. 파이썬 검사 다섯 통과.

## PC가 할 것 — B7 (B7x가 main에 든 뒤)

1. 개발자의 답(위 넷) → 헤더 승인 커밋(헤더 §7 — `DECISIONS_LOG`의 B7 절, `OPEN_QUESTIONS` 닫음·새 행, `STATUS`, `docs/50`; 줄 번호를 그때의 main에 맞춤).
2. `[B-7m0]` B7-0 측정(헤더 §5.1 — 여섯 수, 입력 없는 20,000 60판 짝 등).
3. 그다음 헤더 §8의 커밋 차례(`[B-7a1]` 필드와 쓰는 곳 → …).
