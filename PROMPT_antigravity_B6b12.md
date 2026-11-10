# A. Antigravity — [B-6b12] 시작 문구 (이 PC, UE 있음)

너는 Cosmic Ascension(UE 5.8.3 C++, 1인 개발 게임) 저장소에서 B6b의 마지막 단계 `[B-6b12]`를 이어받는다. 이 PC(Windows 11, 엔진 `C:\Program Files\Epic Games\UE_5.8`)에서 일한다. 답과 보고는 한국어로, 쉬운 낱말로 한다.

1. 먼저 읽기(차례대로)
   - `C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension\Saved\Handoff\HANDOFF.md` — 맨 위 '상태 줄', §0 먼저 할 일, §2 일하는 방식, §3 반드시 지킬 것, §4 B6b 남은 단계(`[B-6b12]` 행), §7 교훈, §8 시작 확인.
   - 저장소의 `CLAUDE.md`(프로젝트 규칙 — 전부 지킨다), `STATUS.md`의 '지금'과 '다음 3작업'.
   - 지침서 `SCR\b6b12\brief_final.md`(같은 폴더에 `brief_working.md`가 있으면 그것이 앞선다), 도구 `lanes.py`·`plan.py`. `SCR` = `C:\Users\rhwwl\AppData\Local\Temp\claude\c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension\37de2919-0c44-45d8-94cd-4dcf0860263e\scratchpad`. 그 폴더가 없으면 `Saved\Handoff\scratch`를 같은 경로로 그대로 복사해 되돌린다(HANDOFF §1).
   - 헤더 `docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md` §7·§8과 노트 `..._notes.md` §5의 `[B-6b11]` 칸(바로 앞 단계의 결과·손 확인 시드·[개발자] 항목).
2. 시작 확인
   - `git log --oneline -5`의 맨 위가 `[B-6b11]`이어야 한다(아니면 멈추고 개발자에게 묻는다).
   - Claude Code 세션이 멈췄는지: `UnrealEditor*`·`UnrealBuildTool` 프로세스가 없고, python 명령 줄에 `bundleC`·`lanes`·`run_mutations`가 없어야 한다(HANDOFF §0).
   - `git --no-optional-locks diff --quiet HEAD`(내용 차이 없음). 평범한 `git status`는 색인을 다시 쓰고 줄끝만 다른 수백 줄을 보일 수 있다 — HANDOFF §7.
3. 할 일 — 지침서의 차례 그대로 `[B-6b12]`: 문서(헤더 §7) + 변이 레인 셋(`C:\CAL\l1`~`l3`, `lanes.py`) + `Sim.PhaseTwoExit`의 맞는 시드 찾기 + B6b 변이 전체 재실행(미뤄 둔 것 포함) + 재감사 + CHANGELOG·용어집. 죽은 패치 `b6a3a_rung_end_back`·`b5b_tenth_outside_guard`는 표시하거나 다시 뜬다.
4. 지킬 것
   - 커밋 = 컴파일되는 상태 + 러너 `powershell -ExecutionPolicy Bypass -File Tools\run_checks.ps1 -Tier full` **한 번** 통과(그 안의 두 판이 '테스트 2회 연속'). 변이는 마지막 러너 **앞**에. 실패가 하나라도 있으면 커밋하지 않는다. 러너를 돌리기 전에 환경 변수 `PROJECT_ROOT`를 비운다.
   - 커밋 제목은 한국어 요약에 `[B-6b12]` 접두어, 본문에 러너 결과와 기준값. 커밋한 뒤 `origin`에 push.
   - 개발자의 결정이 필요한 것은 지침서·노트의 권장안으로 진행한다(개발자, 2026-11-10). 성경 구절 대조와 손 확인은 개발자 몫이다.
   - main만 고친다. 원격의 `cloud/*`·`integrate/b7x`·`handoff/cloud`는 클라우드 Claude Code 세션의 것이라 건드리지 않는다.
   - 도중에 멈추면 HANDOFF §8의 `wip/<단계>` 규칙대로 하고, 끝나면 `STATUS.md`와 HANDOFF.md 맨 위 상태 줄을 고친다.
5. B6b가 끝나면(`[B-6b12]` 커밋 뒤): 다음은 HANDOFF §5.3 — B7x의 마지막 단계(이 PC가 필요하다). 클라우드 세션이 `origin/cloud/b7x-on-*` 브랜치를 올려 두었으면 그것을 받아 커밋마다 빌드·러너로 증명한다(HANDOFF §9). `origin/handoff/cloud`의 `CLOUD_STATUS.md`에 클라우드가 한 일이 적혀 있다.
