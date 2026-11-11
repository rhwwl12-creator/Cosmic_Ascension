# 넘김 글 — [B-6b12] 마무리와 B7x 다시 쌓기 (2026-11-12 07:15 기준, Claude Code 세션 15a1d0fe)

이 글은 이 PC에서 같은 일을 이어받는 다른 세션(Claude Code 또는 Antigravity)을 위한 것이다. 쉬운 한국어로 보고한다. 프로젝트 규칙은 `CLAUDE.md`가 이긴다.

## 0. 먼저 — 두 세션이 한 트리를 동시에 고치지 않는다

이 글을 쓴 세션은 아직 일하는 중일 수 있다. 시작하기 전에 개발자에게 'Claude Code 세션(15a1d0fe)을 멈췄나요?'를 묻고, 아래도 본다.
- 프로세스: `Get-Process UnrealEditor*,UnrealBuildTool,dotnet -ErrorAction SilentlyContinue`가 비어 있어야 한다.
- B7x 통합 작업 사본 `SCR_OLD\b7x\int`(아래 경로)에 `git rebase`가 걸려 있는지: `git -C <그 경로> status`. 'rebase in progress'면 앞 세션의 다시 쌓기가 도중에 멈춘 것이다(§4).
- main 작업 사본의 마지막 15분 안에 바뀐 파일이 있으면 앞 세션이 아직 쓰고 있는 것이다.

## 1. 경로

- 저장소(main): `C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension` — HEAD `467cfdb` `[B-6b11]`. `[B-6b12]`는 **아직 커밋 전이다** — 작업 사본에만 있다.
- 엔진: `C:\Program Files\Epic Games\UE_5.8`.
- `SCR_OLD`(B6b 지침서·하네스·변이 레인 기록) = `C:\Users\rhwwl\AppData\Local\Temp\claude\c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension\37de2919-0c44-45d8-94cd-4dcf0860263e\scratchpad`
  - `[B-6b12]` 지침서: `SCR_OLD\b6b12\brief_final.md`(§3 세이브와 러너, §5.7 변이 정리, §6.5 커밋 메시지의 꼴, §7.1 차례).
  - 변이 레인 기록: `SCR_OLD\b6b12\lanes_run\`(431/431 KILLED).
- `SCR_NEW`(이 세션의 작업 파일) = `C:\Users\rhwwl\AppData\Local\Temp\claude\96227757d97b02e7\15a1d0fe-7848-4df0-9fc2-e33990085063\scratchpad`
  - `reaudit\` — 재감사 결과(`find_*.json`·`verify_*.json`·`critic.json`, 적용 기록 `applied.json`·`failed.json`·`critic_applied.json`, 적용 도구 `apply_fixes.py`).
  - `snap_before_fixes\` — 재감사 고침을 넣기 전 파일의 사본(되돌릴 때).
  - `runner_full_1.txt`(러너 출력), `hc_saves.ps1`·`hc_compare.py`(손 확인 세이브), `status_b6b12.py`·`fill_b6b12.py`(STATUS와 자리표 채움).
- 넘김 글 원본: `Saved\Handoff\HANDOFF.md`(B7x 차례는 §5.3~§5.5), 클라우드 쪽 상태: `git show origin/handoff/cloud:CLOUD_STATUS.md`.

## 2. 끝난 것 (작업 사본에 들어 있음)

1. **Antigravity의 Phase A·B·C**(코드·테스트·에셋·패치 37개, 변이 431/431 KILLED, 문서)는 그대로 받았다. 작업 사본의 코드 diff는 변이 레인이 쓴 `SCR_OLD\b6b12\phaseA.diff`와 같다.
2. **STATUS.md 갱신**: 날짜 2026-11-12, B6b '끝' 문단, 코드 사실·기준값의 `[B-6b12]` 줄, 다음 3작업(1 손 확인 B1~B6b, 2 B7x 통합과 B7 헤더 들이기, 3 앞선 손 확인과 외부 테스터), Top 5(1번을 'B6b가 B7에 넘긴 것'으로, 닫힌 #107·#121·#122 정리).
3. **러너 full 한 번 통과**(05:55~06:02, `-Data` 없이): 테스트 227 × 두 판 = 454 성공·실패 0, 스모크 통과, 기준값 다섯 줄과 해시 판 둘이 `seed42.json`(`[B-6b10]`의 값) 그대로.
   - **주의**: 이 러너 뒤에 재감사 고침이 들어갔고 그 가운데 `Source/Cosmic_Ascension/Tests/CosmicKnowingTests.cpp`의 주석 한 줄이 있다 → 커밋 전에 러너 full을 **한 번 더** 돌린다(약 7분).
4. **손 확인 세이브 넷을 이 빌드로 다시 만들었다**(지침서 §3 — 고친 옛 줄 둘이 세이브의 로그에 실리므로).
   - 명령은 노트 §5 `[B-6b11]`의 꼴 그대로다: `-run=CosmicBalance -Ticks=9000 -Seed=<s> -SaveAt=<t> -TellAll=0 -Answer=1 -Build=1 -Focus=1 -Edict=1 -Diplomacy=1`. 처음 받은 넘김 글의 명령은 정책 인자(`-Answer=1 …`)가 빠져 있어 다른 판이 나온다 — 쓰지 않는다.
   - 결과: 끝 해시·세계 해시가 `[B-6b11]`과 넷 모두 같다(① 4087035544/3212530736, ② 2424659768/2411572982, ③ 2429773939/1203789176, ⑥(i)은 ②와 같은 판). 요약 줄·`_rows.csv`·`_claims.csv`·CSV의 세계 칸도 같고, 다른 것은 시간 칸과 세이브 크기뿐이다(`SaveRawBytes` +4~20바이트).
   - 새 세이브는 `Saved\HandCheck\B6b_*\`에 하나씩 있고, 옛 세이브는 `Saved\HandCheck\_old_b6b11\`로 옮겼다. 로그·CSV는 `Saved\Balance\b6b12_handcheck\`.
5. **재감사**(워크플로 두 번, 에이전트 9 + 5):
   - 첫 재감사: 범위 넷의 발견 131(확인 108·일부 21·반박 2), 검증자의 추가 22, 비평 24. 고침 158을 넣었다. 넣지 않은 17은 다른 고침이 같은 자리를 이미 고친 겹침(16)과 이미 들어 있던 것(1)이다.
   - 바로잡은 주요 사실: CHANGELOG의 B6b 일지가 커밋마다 테스트 수를 '223종'으로 똑같이 적었다(실제 210·214·215·219·222·223…). 리뷰 수와 `[B-6br]` 시간도 틀렸다. 변이는 '다시 뜸 37·낡은 패치 38'이 아니라 **다시 뜸 35(+새 변이 둘 = `b6b12_*` 37개)·B6b가 낡게 만든 앞 블록 패치 42**다. 레인은 셋이 아니라 **둘**(`C:\CAL\l1`·`l2`)이고, `[B-6b12]` 날짜는 **2026-11-12**다. 지침서 밖의 `Sim.PhaseTwoExit` 시드 변경(시드 18 `WithoutB6bData` → 게임 데이터 전체의 시드 1)은 이제 모든 기록에 있다.
   - 둘째 워크플로(`wf_0d9c1be3-b15`)가 돌던 중이다: 고친 문서의 확인 넷(읽기만)과 B7x 다시 쌓기 하나(§4).

## 3. `[B-6b12]`에 남은 일 (차례)

1. 둘째 워크플로의 '고침 확인' 결과를 넣는다(결과는 그 워크플로의 `journal.jsonl` — 경로 `C:\Users\rhwwl\.claude\projects\c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension\15a1d0fe-7848-4df0-9fc2-e33990085063\subagents\workflows\wf_0d9c1be3-b15\`).
2. 재감사의 수를 채운다: `STATUS.md` 7행의 자리표 `<REAUDIT>`. 노트 §5 `[B-6b12]`의 '재감사' 줄과 `docs/CHANGELOG.md` `[B-6b12]` 줄의 '5개 영역 … 0 error / 불일치 0'은 사실이 아니다 — 위 §2-5의 수로 바꾼다.
3. 검사: `python -I Tools/check_docs.py` 0 error, 다섯 파이썬 검사, `CLAUDE.md` ≤ 16,384바이트(LF), 바뀐 파일 모두 LF(`git diff --ignore-cr-at-eol --stat`가 `git diff --stat`와 같다), 헤더 B6b < 149,000바이트, `git grep "<REAUDIT>\|<RUNNER>\|<HC_SAME>"` 0줄.
4. 러너 full 한 번 더(`$env:PROJECT_ROOT = $null; powershell -ExecutionPolicy Bypass -File Tools\run_checks.ps1 -Tier full` — `-Data`·`-UpdateBaseline` 없이).
5. 스테이징: 바뀐 추적 파일 전부와 새 패치 `Tools/mutations/B6b/b6b12_*.patch` 37개. **`docs/OVERVIEW.html`(추적하지 않음)과 `docs/90_LORE/cosmic_species_codex.md`는 절대 넣지 않는다** — `git add docs/`처럼 폴더째 넣지 말고 파일을 고른다. `handoff_B7_next/`는 `.git/info/exclude`에 있다.
6. 커밋 메시지: 지침서 §6.5의 꼴. 처음 받은 넘김 글의 메시지 초안은 수가 틀렸다(37/38, 레인 셋) — 위 §2-5의 바른 수를 쓴다. 끝에 `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
7. `git push origin main`.
8. 패키지 ⓓ: 에디터를 닫고 `powershell -ExecutionPolicy Bypass -File Tools\package.ps1`(깨끗한 트리 필요 — 추적하지 않는 `docs/OVERVIEW.html`이 막는지 본다), 종료 코드 0과 쿠킹 빌드·에디터 해시가 같은지.
9. `Saved\Handoff\HANDOFF.md` 맨 위 상태 줄을 고치고, `origin/handoff/cloud`의 HANDOFF를 바꿀 때는 그 가지의 다른 파일(`CLOUD_STATUS.md` 등)을 지우지 않게 그 가지 위에 커밋을 하나 더한다(덮어 만들지 않는다).

## 4. B7x 다시 쌓기 (2단계) — 미리 시작해 둔 것

- `[B-6b12]` 작업 사본의 그 시점 트리를 임시 커밋 `e1716a6`(로컬 가지 `tmp/b6b12-provisional` — main에 올리지 않는다)으로 만들고, 그 위로 `origin/cloud/b7x-on-rebase`의 다섯 커밋을 다시 쌓는 중이다.
  - 작업 사본: `SCR_OLD\b7x\int`. 빌드할 때는 꼭 정션 `C:\Users\rhwwl\AppData\Local\Temp\b7xi`로 부른다(경로가 길어 UBT가 거부한다).
  - 로컬 가지: `tmp/b7x-restack-pre`.
  - 07:11 기준으로 `[B-7x0]`·`[B-7x1]`·`[B-7x4m]`·`[B-7x3]`이 올라갔고, `[B-7x4]`를 푸는 중이었다. 충돌 자리: `Tools/mutations/README.md`, `docs/21`, `docs/40`, `Tests/CosmicChronicleScreenTests.cpp` — 두 쪽 뜻을 모두 살린다.
- `[B-6b12]`가 실제로 커밋되면: 실제 커밋 위로 한 번 더 옮긴다(`git rebase --onto <새 main> tmp/b6b12-provisional tmp/b7x-restack-pre` — 문서만 다르면 충돌이 적다). 그 결과를 **새 가지 `cloud/b7x-on-<새 main sha>`**에 둔다. `cloud/b7x-on-rebase`는 덮지 않는다 — `cloud/docs-b7-header`가 그 위에 있다.
- 그다음(HANDOFF §5.3):
  - 커밋마다 체크아웃한 채 빌드와 러너 full을 돌린다. 데이터가 바뀐 커밋은 `-Data`. `[B-7x1]`은 새 저장 필드로 두 해시만 옮기므로 그 커밋에서만 `-UpdateBaseline`을 한 번 하고, 다섯 판 CSV가 앞 커밋과 같은지 본다. 테스트는 238종이 기대값이다.
  - B7x의 C++ 변이 `Tools/mutations/B7x/`는 아직 한 번도 죽여 보지 않았다.
  - `[B-7x0]`에 `cloud/docs-b7x0` `453f7f0`(살아 있는 문서 넷)을 접는다. 새2~새11은 `[B-6b12]`의 마지막 물음 #168 다음 번호(#169~)로 바꾸고, `git grep -n "새[0-9]" -- STATUS.md docs/50_ROADMAP.md docs/60_DECISIONS/OPEN_QUESTIONS.md docs/60_DECISIONS/DECISIONS_LOG.md`가 0줄이어야 한다.
- main에 합치는 것은 `[B-7x5s]`·`[B-7x5c]`(데이터)까지 든 뒤다(HANDOFF §5.5). B7 헤더(`cloud/docs-b7-header`, 2026-11-12 승인)는 B7x 뒤에 들인다.

## 5. 지킬 것

- 커밋 = 컴파일되는 상태 + 테스트 두 판 통과. 저장 필드·틱·결정론에 닿는 코드는 헤더 먼저.
- 기준값이 옮으면 멈추고 까닭부터 찾는다(`[B-6b12]`는 비트 그대로여야 한다).
- 보고는 '추가 이야기와 보완'으로 끝내고, 맨 끝에 다음 작업과 그 무게(effort·ultracode)를 적는다.

## 6. 보충 (2026-11-12 07:20, 세션 cb0d1d99가 읽기만 하고 적음 — 이 세션 15a1d0fe의 할 일에 더했다)

1. handoff/cloud는 지금 5067152다. §3의 9에서 그 가지를 고칠 때는 git fetch 뒤 5067152 위에 커밋을 더하고(덮어 만들지 않는다), CLOUD_STATUS.md도 함께 고친다 — [B-6b12] 커밋 sha, 새 가지 cloud/b7x-on-<sha>, 커밋마다 증명한 것(빌드·러너·해시·변이). HANDOFF.md 상태 줄의 'wip/b6b12에 보존됨'은 사실이 아니었다(그 가지는 없다) — 상태 줄을 고칠 때 바로잡는다.
2. B7x 다시 쌓기의 변이 대조: b6b12_ 패치 37개와 다시 뜬 패치들이 B7x 다섯 커밋 위에서 서는지 커밋마다 git apply --check. 새로 서지 않게 된 것은 b7x<N>_ 새 이름으로 다시 뜨고 README에 행과 B7x 머리 문단의 비교 한 문장. 끝에서 서지 않는 B7x 패치 셋(b7x2_data_era_no_ruled_past_checker·b7x2_data_extinction_expiry_effect·b7x4_schema_era_rule_dropped)은 건너뛴다.
3. 다시 쌓는 커밋마다 Tools/test_data_schema.py의 walked 핀을 다시 센다. LF — git이 쓰는 명령에 -c core.eol=lf, 끝에 SCR_OLD\b7x\tools\lf_norm.py <사본> --check. 커밋 본문에 '새 main <sha> 위로 다시 쌓음 — 푼 충돌·walked 핀·변이 대조'.
4. Tools/data를 고친 커밋([B-7x1]·[B-7x3]·[B-7x4])은 러너 -Data가 다시 만든 에셋과 generated.sha1을 그 커밋에 넣는다. [B-7x1]은 -UpdateBaseline 뒤 SCR_OLD\b6b8\fix_seed42.py(인자 없이)로 seed42.json의 꼴을 되돌려 그 커밋에 넣는다. 커밋을 고치면 뒤 커밋들을 그 위로 하나씩 다시 쌓고 같은 증명을 이어 간다.
5. [B-7x0]에 cloud/docs-b7x0 453f7f0을 접을 때: 새2~새11 → #168 다음 번호(확인하고 쓴다). 453f7f0 기준 18곳(STATUS 7·39째 줄, OPEN_QUESTIONS 128~137째 줄 행 머리와 130·131째 줄 안, DECISIONS_LOG 17·20째 줄). 글로 고칠 둘: DECISIONS_LOG 9째 줄 '새1, 닫힘' → '노트 §3의 첫 행, 닫힘', 20째 줄 '노트 §3의 새2~새11' → '노트 §3의 표'. 증명: 살아 있는 문서 넷의 git grep "새[0-9]" 0줄, check_docs 0, CLAUDE.md ≤ 16,384. 헤더(4곳)·노트(15곳)의 새N은 그대로.
6. [B-7x5s]·[B-7x5c]: SCR_OLD\b7x\port의 port_b7x5s.py·port_b7x5c.py를 먼저 --dry-run. port_b7x5c.py 뒤 walked 핀을 다시 센다. 판단 ⓪~⑧(⑥은 B6b 경주 다시 잼)은 B7x 헤더 §5대로.
7. B7 헤더는 B7x가 main에 든 뒤 [B-7h0]으로: 17c34c2와 8afc9b5만 옮기고 1bacd61은 건너뛴다. 새12~새16 → B7x 번호 다음. 헤더의 줄 번호는 그때의 main에 맞춘다. B7 코드와 B7-0 측정은 그 뒤.
8. cloud/b7x-on-<sha>를 푸시한 뒤 로컬 임시 가지 tmp/b6b12-provisional·tmp/b7x-restack-pre를 지운다. 남은 정리는 HANDOFF §5.5.
