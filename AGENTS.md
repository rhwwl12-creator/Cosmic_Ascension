# Cosmic Ascension — 다른 에이전트(Antigravity 등)용 안내

이 저장소의 작업 규칙 원본은 CLAUDE.md다. 이 파일은 그것을 가리키기만 하고 규칙을 옮겨 적지 않는다. Claude Code는 CLAUDE.md가 있으면 이 파일을 읽지 않는다.

## 먼저
- CLAUDE.md(아래에 끼워 넣었다) → STATUS.md의 '지금'·'다음 3작업' 차례로 읽고 일을 시작한다. CLAUDE.md와 이 파일이 다르게 말하면 CLAUDE.md가 이긴다.
- 보고는 한국어로 하고 '추가 이야기와 보완'으로 끝낸다.
- main에 직접 커밋·푸시하지 않고, 주 작업 사본의 브랜치를 바꾸지 않는다. 브랜치가 필요하면 `Saved/AgentWorktrees/` 아래에 git worktree를 따로 꺼낸다.
- UE 빌드·자동화 테스트·커맨드렛·측정·에셋 재생성은 Claude Code 세션과 개발 PC 러너(`Tools/run_checks.ps1`)의 몫이다. 여기서는 편집기 없는 검사(`Tools/run_checks.sh`, 파이썬 검사 다섯)까지만 돌리고, 나머지는 '러너 몫'으로 갈라 적는다.

## 역할 스킬 (부를 때만 쓴다 — 원본은 `.claude/skills/`)
- `/cosmic-gatekeeper` 문지기: 브랜치·커밋에 편집기 없이 되는 검사를 돌리고 보고한다. 읽기 전용.
- `/cosmic-design-scout` 설계 조사원: 아이디어를 규칙에 부딪혀 보고 헤더 초안을 낸다.
- `/cosmic-decision-digest` 결정·손 확인 길잡이: 개발자가 답할 결정과 밀린 손 확인을 모은다. 읽기 전용.
- `/cosmic-lead` 총괄: 요청 하나를 위 셋에게 나눠 맡기고 결과를 하나로 정리한다.

## 작업 규칙 (CLAUDE.md 원문)

@[CLAUDE.md](CLAUDE.md)
