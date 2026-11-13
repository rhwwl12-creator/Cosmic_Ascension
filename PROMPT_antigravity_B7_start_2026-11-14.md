# Antigravity 넘김 글 — B7x 끝, B7 헤더 들임 뒤 (2026-11-14, Claude Code 세션 15a1d0fe)

이 글은 이 PC에서 일을 이어받는 Antigravity 세션에 드린다. 설계의 단일 기준은 `docs/`, 지금 상태는 `STATUS.md`와 `git log`다. 이 글과 다르면 그쪽이 이긴다.

`Saved\Handoff\HANDOFF.md`는 이 글보다 낡았다. 그 파일에서는 §2·§3·§7의 규칙만 읽는다. 상태 줄·§0·§1의 작업 사본 목록·§4~§9는 따르지 않는다. 거기 나오는 통합 작업 사본 `SCR\b7x\int`, `integrate/b7x`, `cloud/*`, B6b의 남은 단계, '#168부터'는 모두 끝났거나 지워졌다.

## 0. 시작 전에 (차례)

1. **개발자에게 한 줄로 확인한다.** 원격 가지 `tools/agent-setup`(main에 들지 않은 제안, 2026-11-10)의 `AGENTS.md`는 Antigravity에게 'main에 직접 커밋·푸시하지 않는다, UE 빌드·측정은 Claude Code의 몫'이라고 적는다. 이 글은 그 반대(main 커밋, 빌드·측정)를 맡긴다. 그래서 먼저 묻는다: 'Antigravity가 main에 커밋·푸시하고 UE 빌드·측정을 돌려도 되나요?' 답이 '아니오'면 이 글의 §2는 읽기와 문서 초안만 한다.
2. **Claude Code 세션이 멈췄는지 확인한다.** 두 세션이 한 작업 사본을 함께 고치면 트리와 측정이 망가진다.
   - `Get-Process UnrealEditor*,UnrealBuildTool,dotnet -ErrorAction SilentlyContinue`가 비어 있어야 한다. UBT는 `dotnet`으로 돈다.
   - `Get-CimInstance Win32_Process | Where-Object { $_.ProcessId -ne $PID -and $_.CommandLine -match 'lanes\.py|plan\.py|apply_check|b7x5_bundle|bundle[ABC]\.py|run_checks\.ps1|make_data_tables' } | Select-Object ProcessId,Name,CommandLine`가 비어 있어야 한다. 러너는 `powershell.exe`로 돈다.
   - 지난 15분 안에 바뀐 파일이 없어야 한다: `Get-ChildItem 'C:\CAL','<저장소>\Saved\Balance' -Recurse -File | Where-Object LastWriteTime -gt (Get-Date).AddMinutes(-15)`.
   - 그래도 모르면 개발자에게 'Claude Code 세션을 닫았나요?'를 묻는다.
3. **읽는 차례**:
   - `CLAUDE.md`(프로젝트 규칙 — 전부 지킨다) → `WORKPLAN.md` §2의 B 행 → `STATUS.md`의 '지금'·'다음 3작업' → 이 글
   - → `Saved\Handoff\HANDOFF.md` §2·§3·§7(규칙과 교훈만)
   - → B7 헤더 `docs/60_DECISIONS/headers/B7_ai_peoples_hands.md` §5.1·§5.3(J7·J8)·§8 → B3 헤더 §1.2(B7-0 첫째 수의 셈)
   - → B7x 노트 `docs/60_DECISIONS/headers/B7_composable_existence_and_evolution_notes.md` §5.6
4. **상태 확인**(§5의 명령): main 끝이 `[B-7xz]` `18a0cfa`이고, `git status --short`가 `?? docs/OVERVIEW.html` 하나여야 한다. 그리고 `git ls-files --eol | Select-String 'w/crlf'`가 비어 있어야 한다. 비어 있지 않으면 §3 '줄끝'을 먼저 한다.

## 1. 끝난 것 (2026-11-12~14)

- **main**: `[B-6b12]` `29e2ecd` 위에 B7x 여덟 커밋, B7 헤더 승인 문서, 글 고침 하나가 들었다. 모두 원격에 올렸다.
  - B7x 데이터를 켜기 전: `[B-7x0]` `bf6ef85` 승인 문서 → `[B-7x1]` `0f03dad` 필드와 장치 → `[B-7x4m]` `878e9bd` 측정 → `[B-7x3]` `2de9cf4` 표현 → `[B-7x4]` `8b7c2de` 글.
  - `[B-7x5s]` `2bd7bab`: 씨앗 블록 여덟과 노브 열여섯의 시작값, 변이 문턱 1,200. 세계가 옮았다(AI 백성만).
  - `[B-7x5c]` `fa17af9`: 갈림 카드 열둘, 블록 사건 아홉, `arch_seeker`의 `Brick.Form.Society`. 카드 문턱은 잰 값이다. 세계가 옮았다.
  - `[B-7x5r]` `62eb013`: 판단 ④의 셈을 비트 그대로 줄인 성능 커밋.
  - `[B-7h0]` `56e7d82`: B7 헤더(AI 백성의 손) 승인 문서.
  - `[B-7xz]` `18a0cfa`: 낡은 글 셋을 고쳤다(README의 B7x 수, B7x 헤더 §9의 카드 문턱, STATUS의 넘김 글 자리).
- **증명**:
  - 코드·데이터 커밋(`[B-7x1]`~`[B-7x5r]`)은 커밋마다 빌드하고 러너 `-Tier full`을 통과했다. 테스트 수는 `[B-7x1]` 238, `[B-7x4m]`~`[B-7x5s]` 240, `[B-7x5c]`·`[B-7x5r]` 242이고, 모두 × 두 판이다.
  - 문서만의 커밋(`[B-7x0]`·`[B-7h0]`·`[B-7xz]`)은 파이썬 검사로 증명했다.
  - `-UpdateBaseline`은 세 커밋에서 썼다. `[B-7x1]`은 두 해시만 옮았고, `[B-7x5s]`·`[B-7x5c]`는 다섯 줄과 해시 판 둘이 모두 옮았다. 지금 값은 `fa17af9`의 메시지와 `Tools/baselines/seed42.json`에 있다(`[B-7x5r]`은 비트 그대로).
  - main을 B7x 끝으로 옮긴 뒤 main에서 러너 full을 한 번 더 통과했다(242 × 두 판).
- **변이**: B7x 변이는 모두 걸렸다 — 가지 끝 98/98, `[B-7x5s]` 8/8, `[B-7x5c]` 51/51, `[B-7x5r]` 23/23. `Tools/mutations/B7x/`에는 149개가 있다.
- **B7x 데이터 커밋의 판단**(B7x 헤더 §5, 기록은 노트 §5.6, 셈법은 리드 판정 L1~L10):
  - ⑥(B6b 경주를 지키는가)·①·②·③·③′: 통과.
  - ④(시간 +5% 안): 어긋남. `[B-7x5r]` 뒤에도 +7~+19%이고, 남은 몫은 세계 차이다 → 개발자 물음 **#186**(기본값 '받아들인다').
  - ⑤(갈림 카드가 주기 칸을 차지): 어긋남(결정 수 −17) → **#184**(기본값 '받아들인다').
  - ⑧의 남긴 법은 **#178**에 수를 더했다(기본값: 좁힘은 받았다; 지렛대인 `law_stays`의 조건은 B7과 함께 정한다).
  - L8의 셈(`Consumed`·`Isolated`가 일찍 멈춤)은 **#185**(기본값: 변이 문턱 1,200과 아래 끝 900을 둔다).
  - 넷 모두 기본값으로 막지 않는다. 개발자가 뒤집을 수 있다.
- **브랜치 정리**(개발자 요청 2026-11-09 '적용 다 했으면 남은 브랜치는 다 없애고 메인에 병합'):
  - 원격에는 `main`·`handoff/cloud`·`tools/agent-setup`만 남았다.
  - `handoff/cloud`는 넘김 기록이다. 덧붙이기만 하고 고쳐 쓰지 않는다. `tools/agent-setup`은 이 세션의 것이 아니라 손대지 않았다(위 §0 1).
  - 로컬 브랜치는 `main` 하나, 작업 사본은 저장소 하나다.
  - 지우기 전의 브랜치는 모두 로컬 묶음 파일 `Saved\Handoff\branches_before_cleanup_2026-11-14.bundle`에 남겼다(원격에는 없다). 든 ref는 `git bundle list-heads <파일>`로 본다. 꺼낼 때는 `git fetch <파일> <브랜치>:<새 이름>`을 쓴다. `design/existence-evolution`만은 `refs/remotes/origin/design/existence-evolution:refs/heads/<새 이름>`이다.

## 2. 다음 일 (차례 — 무게는 B7 헤더 §8 표의 effort 칸)

1. **`[B-7m0]` B7-0 다시 잼** — medium. B7 헤더 §5.1. 세계(비트)는 그대로다.
   - **코드 하나가 들 수 있다.** 다섯째 수(섭정 아래 옛 백성의 칙령 보유 틱)는 지금 `bricks:` 줄에 없다. 그 줄의 kept law 칸은 사람·때·안정도 0.2 아래·반란·지워짐·`LawsHeldTicks`뿐이다(`Tests/CosmicBalanceCommandlet.cpp` 4,500행 언저리). 그래서 헤더 §5.1 5대로 커맨드렛에 측정 전용 줄 하나를 더한다. 그러면 빌드와 러너 full이 한 번 든다(다섯 줄과 해시 판 그대로).
   - **하네스는 새로 쓴다.**
     - 본은 B6b의 `Saved\Handoff\scratch\b6b11\bundleC.py`다(셈과 바쁨 검사 `_busy_others` — UnrealEditor-Cmd가 스스로 띄우는 `dotnet … -Mode=ValidatePlatforms`는 바쁨이 아니다). 자체 시험은 `bundleC_selftest.py`이고, 그것이 불러 쓰는 `..\b6b8\bundleB.py`·`..\b6b6\bundleA.py`도 `Saved\Handoff\scratch\`에 있다.
     - bundleC는 [B-6b11] 전용이라(인자와 끈 쪽 `-WithoutB6bData=1`) 그대로 쓰지 않는다. 짝의 끈 쪽은 `-WithoutB7xData=1`이다.
     - §5.1의 명령에는 `-Rules`가 없다. B7x 묶음(`b7x5_bundle.py` — 모든 판에 `-Rules=ProvisionalEndTick:0`)의 판과 섞지 않는다.
   - **수마다**:
     - ① 1~4번: 입력 없는 20,000틱, 시드 1~60, 켬과 끔. `C:\CAL\b7x5m\c5\inputless_*`는 같은 비트의 빌드지만 `-Rules=ProvisionalEndTick:0`으로 돌았다. 다시 쓰려면 먼저 두 시드를 `-Rules` 없이 돌려 `csv_cmp.py`로 결정적 칸이 같은지 본다.
     - ② 5번: R + `-Crossroads=1`로 30판 × 9,000. c5x의 xr1에서 30/30판에 kept law가 섰다. 헤더 §5.1의 `-Abdicate=1200`은 빼고, 그 까닭(카드가 셋째 칸의 통치자에게만 오므로 1,200에 양위하면 오지 않는다)을 헤더 §11에 적는다.
     - ③ 6번(틱 시간): **다른 일이 하나도 돌지 않는 기계에서 직렬로, 켬·끔을 번갈아** 잰다(헤더 §5.1 6, 방법은 B6b 노트의 `[B-6br]`). 하네스의 `serial` 단계가 쓸 만하다: `python -I -X utf8 Saved\Handoff\scratch_15a1d0fe\b7x5_prep\harness\b7x5\b7x5_bundle.py serial c5 --folder C:\CAL\b7_0\serial --project "<저장소>\Cosmic_Ascension.uproject" --baseline-rev 8b7c2de --prev-baseline-rev 2bd7bab`. 이번 세션에서 파이썬 검사와 CPU를 나눈 직렬은 같은 인자 두 판이 13%까지 벌어졌고, 같은 세계의 `=1` 줄이 조용한 판보다 15~25% 느렸다.
     - ④ J7(헤더 §5.3): 늦은 계승(4,000 뒤)의 다이제스트 5~8줄은 bundleC `r6_154`의 셈으로 방랑자·입력 없는 칸에서 잰다. 복귀 다이제스트 2줄 이상은 '재지 않음 — `hands:` 줄은 `[B-7a2]`'로 적는다.
     - ⑤ J8: `-Wander=1`(다른 정책 인자 없이) 60판 × 6,000, 켠 쪽 하나. 셈은 bundleC의 R4다. `-WithoutB7Data=3`은 `[B-7a2]`에서야 생긴다.
   - **결과와 커밋**: 판은 `C:\CAL\b7_0\`(OneDrive 밖)에 두고, 요약은 새 폴더 `Saved\Balance\b7_0\`에 둔다(있는 보관 폴더는 덮지 않는다). §1.2의 `<<B7-0:1~6>>`를 채운 뒤 `git grep -n "<<B7-0"`가 0이어야 한다. 헤더 §11에 한 줄, STATUS를 고치고 커밋 `[B-7m0]`.
2. **B7x 손 확인의 준비** — B7x 헤더 §9. 리드의 몫이며 **이번 세션은 하지 못했다**. **`[B-7a1]` 전에** 한다. `[B-7a3]`부터 세계가 옮아 c5의 시드·틱이 맞지 않게 되기 때문이다.
   - 빌드는 지금 main(코드는 `62eb013` 그대로)이다.
   - 명령(R의 꼴): `UnrealEditor-Cmd.exe "<저장소>\Cosmic_Ascension.uproject" -run=CosmicBalance -Seed=<시드> -Ticks=<틱 이상> -TellAll=0 -Answer=1 -Build=1 -Focus=1 -Edict=1 -Diplomacy=1 -SaveAt=<틱> -unattended -nosplash`. ⑥은 `-Crossroads=1`을 더한다. `-Rules`는 쓰지 않는다(`-SaveAt`이 거절한다).
   - 틱은 카드가 오는 틱 − 10이다(④는 그 답 뒤도 된다). 시드와 틱은 `C:\CAL\b7x5m\c5`·`c5x`·`c5w` 로그의 `bricks:` 줄(`crossroads came … [틱:행 …]`)에서 고른다. 묶음은 시계 없이(`-Rules=ProvisionalEndTick:0`) 돌았고 세이브 판은 시계 9,000으로 돈다. 그래서 같은 세계는 9,000 앞뿐이니 9,000 아래에서 고른다.
   - ⑥(`petition_cross_law_stays`, `Endured` 433)은 `c5x\xr1_<시드>`에서 고른다(예 `xr1_1`: 3,000).
   - ④(`petition_cross_isolated`, `Isolated` 900·`Society`)는 R 판과 `-Crossroads` 판에 한 번도 오지 않았다(#185). 온 판은 방랑자 시드 18 하나다(`c5\wanderer_18.log`, 첫 3,900). 그 판의 인자(R + `-Abdicate=1200 -Wander=1`, `-Crossroads` 없이)로 `-Seed=18 -SaveAt=3890`을 만들거나, 고를 수 없다고 적는다.
   - `-SaveAt`은 게임의 슬롯에 쓴다. 세이브마다 곧바로 `.sav`를 새 폴더 `Saved\HandCheck\B7x_<항목>_seed<시드>_<틱>\`로 옮긴다. `B6a_*`·`B6b_*` 폴더는 건드리지 않는다.
   - 고른 시드·틱·명령 줄·기대 틱을 B7x 노트 §5에 적는다. 문서만이면 파이썬 검사로 커밋한다.
3. **B7 코드** — `[B-7a1]`(필드와 쓰는 곳 — `docs/20`·`docs/21` §7.4를 코드보다 먼저)부터 B7 헤더 §8 표의 차례대로. 무게는 `[B-7a1]`·`[B-7a2]`·`[B-7b1]`·`[B-7b2]`·`[B-7z]` xhigh, 나머지 high.
   - 헤더에 적힌 일은 '권장대로'가 승인이다.
   - 헤더에 없는 저장 필드·열거자·틱 단계·추첨 지점·해시 규칙은 헤더의 고침을 먼저 개발자에게 보이고 승인 뒤 소스로 간다(틱 순서·추첨·저장이 바뀌면 `docs/20`을 먼저).
   - 미리 적은 판단(헤더 §5.3)이 고침 한 번 뒤에도 어긋나면 [개발자].
   - 헤더 §8에 ultracode 리뷰가 적힌 커밋(`[B-7a1]`·`[B-7a3]`·`[B-7b1]`·`[B-7b3]`·`[B-7t]`·`[B-7z]`) 앞에서는 멈추고 개발자에게 묻는다: 'Claude Code에서 돌릴까요, 여기서 서로 따로 읽는 리뷰 셋과 반박 하나로 대신할까요?'
4. **개발자 몫** — 막지 않는다.
   - 손 확인: B1~B6b와 B7x §9(세이브는 위 2).
   - 물음 #184·#185·#186·#178의 답, L8(변이 문턱 1,200)의 확인.
   - 이야기 행: 이야기 초안 파일 `cosmic_ascension_story_drafts.md`가 이 PC에 없어 하지 못했다. 개발자에게 받으면 main에서 새 가지로 한다(옛 `cloud/content-rungs`는 지웠다).
5. **하네스 고침** — B7 이후의 다시 잼 전에 한다. 고칠 곳은 `Saved\Handoff\scratch_15a1d0fe\b7x5_prep\harness\b7x5\b7x5_bundle.py`의 판단 읽기 넷이다(노트 §5.6 '하네스의 읽기와 다른 자리').
   - ⑤의 끈 쪽 창을 C(9,000)에서 자른다.
   - ③′의 증언 카드가 30건 미만이면 '재지 않음'으로 적는다.
   - ⑦의 주장 수는 `_claims.csv`로 판의 끝 전 전체를 센다.
   - ⑧의 빈 항목을 채운다: 초신성 업적, #129와 S09의 셈(#130은 `[B-7h0]`이 닫았으니 셈만 적는다), 인구 바닥 틱.

## 3. 일하는 방식 (지난 세션의 교훈 포함)

- **낱말 풀이**:
  - 무게: Claude Code의 생각 깊이 표지다(medium 보통·high 깊게·xhigh 가장 깊게).
  - 리드: 앞 세션의 주 세션. L1~L10은 그 판정이고 B7x 노트 §5.6에 있다.
  - 판: 커맨드렛 한 번.
  - 칸: 정책 묶음이다.
    - R = `-Answer=1 -Build=1 -Focus=1 -Edict=1 -Diplomacy=1 -TellAll=0`인 통치자 판.
    - 느린 R(slowR) = R에 `-Focus=0`.
    - 방랑자 = R에 `-Abdicate=1200 -Wander=1`.
    - 입력 없음 = 정책 인자 없음.
  - 짝: 같은 빌드·같은 시드에서 데이터를 끈 판(`-WithoutB7xData=1`; `=2`는 카드·사건·Society만 끈다).
  - c5·c5w·c5x: `b7x5_bundle.py`의 측정 단계 이름이다. c5x의 xr1·xr2는 R + `-Crossroads=1`·`2`.
  - 판단 ①~⑧·③′: B7x 헤더 §5. J1~J12: B7 헤더 §5.3.
  - '세계가 옮았다': 기준값 다섯 줄과 CSV가 바뀌었다. '비트 그대로': 기준값과 해시가 그대로다.
  - '다시 뜬다': 서지 않게 된 변이 패치를 새 이름으로 다시 만든다.
- **보고는 한국어로**, 쉬운 낱말로 한다. 보고 끝은 '추가 이야기와 보완'이고, 맨 끝 줄에 다음 작업과 무게를 적는다.
- **커밋**:
  - Source·Tools/data·Config가 바뀐 커밋은 빌드와 러너 full 한 번(그 안의 두 판)을 통과해야 한다. 문서만의 커밋은 파이썬 검사 다섯(check_docs 0)으로 증명한다.
  - 제목은 한국어 요약에 작업 접두어를 붙인다(`[B-7m0]` 등). 본문은 `- ` 항목으로 쓴다. 끝에 `Co-Authored-By:` 트레일러를 실제로 쓴 도구와 모델의 줄로 둔다.
  - 차례: 커밋(아직 푸시하지 않는다) → 그 커밋에서 변이 → 결과를 `git commit --amend`로 넣는다 → 푸시. 푸시한 커밋은 고쳐 쓰지 않는다(B7 헤더 §8). 강제 푸시는 쓰지 않는다.
- **건드리지 않는 것**:
  - `docs/OVERVIEW.html`(추적하지 않는 로컬 파일)과 `docs/90_LORE/cosmic_species_codex.md`. 절대 건드리거나 스테이징하지 않는다.
  - 그 밖에 `docs/archive/`(동결본), `Saved\Balance\`와 `Saved\HandCheck\`의 있는 폴더, 원격 `tools/agent-setup`. `handoff/cloud`는 덧붙이기만 한다.
- **스테이징**: `git add -A`나 `git add .`는 쓰지 않는다. HANDOFF §2의 PowerShell 명령을 쓴다:
  ```powershell
  $l = @(git -c core.quotepath=false diff --name-only) + @(git -c core.quotepath=false ls-files --others --exclude-standard) | Where-Object { $_ -ne 'docs/OVERVIEW.html' }
  [IO.File]::WriteAllLines("$PWD\Saved\stage.txt", $l)
  git add --pathspec-from-file=Saved/stage.txt
  ```
  그 뒤 `git diff --cached --name-only`에 `docs/OVERVIEW.html`이나 뜻밖의 새 파일이 없는지 본다.
- **러너**는 PowerShell에서, 에디터를 닫고 돌린다: `powershell -ExecutionPolicy Bypass -File Tools\run_checks.ps1 -Tier full [-Data] [-UpdateBaseline]`.
  - `-Data`는 `Tools/data/`나 `Tools/cosmic_data_schema.py`를 고친 커밋에 붙인다.
  - 시작에 `Remove-Item Env:PROJECT_ROOT -ErrorAction SilentlyContinue`를 한다. 작업 사본은 저장소 하나뿐이다.
  - `-UpdateBaseline`은 `Tools/baselines/seed42.json`을 PowerShell 꼴(CRLF·`\u0027`)로 다시 쓴다. 저장소 뿌리에서 `python -I -X utf8 Saved\Handoff\scratch\b6b8\fix_seed42.py`(인자 없이)로 json indent 4·LF 꼴로 되돌린다.
- **파이썬 검사 다섯**(에디터 없이, 저장소 뿌리에서): `python -I -X utf8 Tools/check_data.py`, `Tools/test_data_schema.py`, `Tools/check_layers.py --strict`, `Tools/make_config_reference.py --check`, `Tools/check_docs.py`. `Tools/run_checks.sh`는 `python3`를 불러 이 PC에서 돌지 않으니 하나씩 부른다.
- **줄끝**: 인덱스와 커밋은 LF다. 작업 트리는 아닐 수 있다.
  - 까닭: `.gitattributes`의 `* text=auto`에 `core.eol` 기본값(native = CRLF)이 겹쳐 git이 쓰는 파일이 CRLF로 나온다. 이 저장소의 `core.autocrlf`는 `false`다.
  - 이번 세션 끝에 main 작업 트리를 LF로 되돌렸다(`w/crlf` 0).
  - git이 파일을 쓰는 명령(checkout·reset·apply·rebase·cherry-pick·stash·merge·pull)에는 `-c core.eol=lf`를 붙인다. 잊었으면 `python -I -X utf8 Saved\Handoff\scratch\b7x\tools\lf_norm.py "<저장소>"`로 되돌린다. 그 뒤 ` M`이 수백 줄 보이면 stat 소음이다(`git diff --numstat`이 `0 0`). `git add -u`로 지운다.
  - 커밋 전에 `lf_norm.py "<저장소>" --check`가 0이어야 한다. CR 검사는 커밋할 바이트로 한다(`git show :<파일>`의 `\r` 수가 0). CRLF 트리에서 작업 파일의 `\r`를 세면 손대지 않은 파일에서도 걸린다.
  - 변이 패치와 `git apply --check`도 LF 작업 트리에서만 맞는 수를 낸다.
- **크기 한도**(커밋된 LF 바이트로 잰다 — `git cat-file -s HEAD:<파일>`, 스테이징 뒤에는 `git cat-file -s :<파일>`):
  - `CLAUDE.md` ≤ 16,384. 지금 16,308이니 줄을 늘리지 않는다.
  - B7x 헤더 < 150,000. 지금 149,673이라 §11에 한 줄을 더하려면 그만큼 노트로 옮긴다. §11은 커밋마다 400바이트 아래 한 줄이다.
  - B7 헤더 101,207 — 여유가 있다. §11은 커밋마다 한 줄이고, 넘치면 그 헤더의 노트로 옮긴다.
- **열린 물음 번호**: 마지막이 #186이다. 새 물음은 #187부터.
- **변이 패치**: 새 단언은 변이로 한 번 깨뜨린다. 도구는 `Saved\Handoff\scratch_15a1d0fe\b7x_mut\`에 있다.
  1. 계획: `python -I -X utf8 plan.py --root "<저장소>" --rev HEAD --main-folder B7 --out C:\CAL\mut\<커밋>\plan.json`. B7x 행이면 `--main-folder B7x`. B7 패치는 `Tools/mutations/B7/`에 둔다(B7 헤더 §6).
  2. 실행: `python -I -X utf8 lanes.py --plan C:\CAL\mut\<커밋>\plan.json --root "<저장소>" --out C:\CAL\mut\<커밋> --only <태그 파일(한 줄에 하나)> "--lane-flags=-WaitMutex"`.
  - 레인(`C:\CAL\l1`·`l2`)은 이번 정리로 지웠다. 첫 판은 `--reuse-lanes` 없이 돌려 레인을 만들고, 그 뒤부터 붙인다.
  - `--lane-flags`를 빼면 기본이 `-WaitMutex -DisableUnity`다. 항목마다 모듈 전체를 다시 빌드해 약 17분씩 걸리니 빼지 않는다.
  - 다른 줄을 고쳐 옛 패치가 서지 않게 되면, 옛 행에 'no longer applies since [커밋] - taken again as …'을 달고 새 이름으로 다시 뜬다.
  - 커밋마다 모든 패치를 커밋한 바이트로 견준 수를 적는다. 폴더마다 `apply_check.py --root "<저장소>" --rev HEAD --wt C:\CAL\ac_<폴더> --out C:\CAL\ac_<폴더>.txt --folder <폴더>`를 돌려 OK 줄을 더한다. 지금은 1,277개 가운데 1,084개가 선다.
- **heredoc 안의 파이썬**은 백슬래시와 `\n`을 망가뜨린다. 긴 편집은 파일로 쓴 스크립트로 한다.
- **뜻이 바뀌는 테스트**: B7x가 주제가 아닌 테스트가 데이터 커밋으로 깨지면, 먼저 까닭을 찾는다. 그다음 가장 바깥에 `CosmicTests::WithoutB7xData`를 둔다(L7·L9·L10의 꼴). B7의 데이터 커밋(`[B-7a3]`부터)에서는 `[B-7a2]`가 더하는 `-WithoutB7Data` 짝을 같은 꼴로 쓴다.
- **ultracode**는 Claude Code의 다중 에이전트 워크플로다. 이번 세션이 쓴 스크립트는 기록용으로 `Saved\Handoff\scratch_15a1d0fe\workflow_scripts\`에 있고, Antigravity에서는 돌지 않는다.
  - 쓴 꼴은 이렇다: 읽기만 하는 적대 리뷰(렌즈와 반박자), 파일 무리로 나눈 고침과 무리마다의 확인자, 밸런스 리뷰.
  - 측정이나 빌드가 도는 동안 리뷰 에이전트에게 UE·빌드·git 쓰기를 금지했다.

## 4. 도구와 자료의 자리

- 저장소: `C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension`. 원격 `origin`은 GitHub 비공개 저장소이고, 푸시는 Git Credential Manager가 한다. 토큰을 화면에 내지 않는다.
- 엔진: `C:\Program Files\Epic Games\UE_5.8`.
- **이번 세션의 도구 백업**: `Saved\Handoff\scratch_15a1d0fe\`.
  - `b7x5_prep\`: B7x 측정 하네스 `harness\b7x5\b7x5_bundle.py`와 지침서 `brief_b7x5.md`.
    - 하네스는 `jobs`·`run`·`read`·`serial` × `ref`·`pre`·`s5`·`c5`·`c5w`·`c5x`를 받고, 새 시드 칸 `s5n`·`c5n`·`c5wn`도 받는다(L2).
    - 같은 폴더의 `harness\b6b8\bundleB.py`를 import한다.
    - **주의**: 기본 `PROJECT`(96행)는 지운 정션 `C:\Users\rhwwl\AppData\Local\Temp\b7xi`를 가리킨다. 늘 `--project "<저장소>\Cosmic_Ascension.uproject"`를 붙인다.
  - `b7x_mut\`: 변이 도구 `lanes.py`·`plan.py`·`apply_check.py`.
  - `b7x5m_summaries\`: 측정 요약과 리뷰 결과.
  - `q5c.ps1`: 에셋·에디터 빌드·테스트 필터 한 판. `$Root`는 `$env:PROJECT_ROOT`, 없으면 저장소다.
  - `csv_cmp.py <폴더A> <폴더B>`: `TickMicros`·`SaveBytes`·`SaveRawBytes`를 뺀 결정적 칸을 비교한다.
- 옛 세션의 스크래치 백업은 `Saved\Handoff\scratch\`다(`b6b8\fix_seed42.py`·`bundleB.py`, `b6b11\bundleC.py`, `b7x\tools\lf_norm.py`).
- 측정 판의 원본은 `C:\CAL\b7x5m\`(ref·pre·s5·c5·c5w·c5x·직렬 폴더)이고 OneDrive 밖이다. B7x 손 확인의 시드를 고른 뒤에는 지워도 된다.
- 넘김 기록: 이 글, `Saved\Handoff\HANDOFF.md`의 상태 줄, `git show origin/handoff/cloud:CLOUD_STATUS.md`.

## 5. 시작 확인 명령

```powershell
git log --oneline -8
git status --short
git ls-files --eol | Select-String 'w/crlf' | Measure-Object
git branch -a
git worktree list
python -I -X utf8 Tools/check_docs.py
```

정상이면 다음과 같다.
- main 끝이 `18a0cfa` `[B-7xz]`(그 아래 `56e7d82` `[B-7h0]`)다.
- `git status --short`는 `?? docs/OVERVIEW.html` 하나다.
- `w/crlf`의 Count가 0이다.
- 원격 브랜치는 `main`·`handoff/cloud`·`tools/agent-setup`뿐이다.
- 작업 사본은 저장소 하나다.
- `check_docs`의 마지막 줄이 `check_docs: 47 files, 0 error(s)`다(NOTE 줄은 많다).
