# B. 클라우드 Claude Code — GitHub만 연결된 세션의 시작 문구

이 저장소(Cosmic Ascension, UE 5.8 C++ 게임)를 GitHub만 연결된 클라우드에서 이어 작업한다. 여기에는 UE·Windows가 없다. 답과 보고는 한국어로, 쉬운 낱말로 한다.

1. 받기
   - `git fetch origin handoff/cloud integrate/b7x main`
   - `git worktree add ../handoff origin/handoff/cloud` → `../handoff/HANDOFF.md`를 읽는다: **§9**(클라우드에서 되는 일·안 되는 일·규칙)를 먼저, 그다음 §2·§3·§5. 본문의 `SCR`은 `../handoff/scratch`다. (LFS 오류가 나면 `GIT_LFS_SKIP_SMUDGE=1`로 — 에셋은 쓰지 않는다.)
   - 저장소의 `CLAUDE.md`·`STATUS.md`도 읽는다.
2. 할 일(HANDOFF §9, 이 차례로)
   1. **B7x 다시 쌓기**: `origin/integrate/b7x`(main `8b6c5a7` 위의 B7x 다섯 커밋)를 그때의 `origin/main` 위로 다시 쌓는다. 충돌은 손으로(main 쪽 변경은 §9에 있다). 커밋마다 `bash Tools/run_checks.sh` 통과, `Tools/test_data_schema.py`의 `walked` 핀은 그 테스트가 말하는 수로, `[B-7x1]`·`[B-7x4]` 커밋 본문의 낡은 '다시 뜬다' 문장을 고치고, 줄끝은 LF. 결과를 `cloud/b7x-on-<main 짧은 sha>`로 push한다(`integrate/b7x`는 덮지 않는다). PC 쪽 main이 그 뒤 더 앞서면(`[B-6b12]`) 다시 한 번 쌓는다.
   2. **B7x 승인 커밋의 살아 있는 문서 초안**(브랜치 `cloud/docs-b7x0`): HANDOFF §5의 '`[B-7x0]` 커밋에 살아 있는 문서' — `STATUS.md`, `DECISIONS_LOG.md`의 B7x 절, `OPEN_QUESTIONS.md`의 새 행(번호는 `[B-6b12]` 뒤에 정하므로 자리표 '새1'~'새11'), `docs/50`의 B 행과 수량, `CLAUDE.md`의 `docs/archive/` 행(바이트가 늘지 않게).
   3. **다음 블록 헤더의 조사·초안**(브랜치 `cloud/docs-<블록>-header`): `docs/50_ROADMAP.md`가 B6b·B7x 다음에 두는 블록. 시작하기 전에 무엇을 쓸지 개발자에게 한 줄로 알린다.
3. 이미 정해진 것: 개발자의 글 확인 셋(C10 제목 '손대지 않은 칙령', 꼴 낱말 여섯, C12 제목 '집을 싣는 배')은 개발자가 2026-11-10 '권장대로'로 받았다 — `scratch/b7x/stage3/texts/lead_rulings.md` 그대로 쓴다. 그 밖에 개발자의 결정이 필요한 것도 권장안으로 진행한다.
4. 지킬 것
   - main에는 커밋·push하지 않는다(main은 PC에서 러너를 통과한 커밋만). `cloud/*` 브랜치에만 push한다.
   - 빌드·자동화 테스트·러너·커맨드렛·`make_data_tables`·측정·변이는 여기서 못 한다 — 필요한 것은 PC 몫으로 적는다. `Tools/data/*.json`·`Tools/cosmic_data_schema.py`·`Tools/data/schema.json`을 고친 커밋은 브랜치에만 둔다(에셋은 PC에서 다시 만든다).
   - `docs/90_LORE/cosmic_species_codex.md`와 `docs/OVERVIEW.html`은 건드리지 않는다. 신학·서사 규칙은 `CLAUDE.md` 규칙 11.
5. 멈출 때: `handoff/cloud` 브랜치에 `CLOUD_STATUS.md`(한 일, 브랜치마다 든 것, PC가 빌드·러너로 증명할 것)를 커밋해 push한다 — PC 세션이 그것을 읽고 받는다.
