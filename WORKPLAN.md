# Cosmic Ascension — 작업지시서 (WORKPLAN)

> 앞으로의 Claude Code 세션(모델: **Opus 5.5**)이 Phase·작업별로 **어떤 추론 강도(effort)**와 **어떤 협업 방식(단독 / ultracode 워크플로)**으로 일할지 정한 지시서다. 설계의 단일 기준은 `guideline.md`(SSoT), 작업 규칙은 `CLAUDE.md`, 이 파일은 **진행 방식**만 다룬다. 셋이 충돌하면 guideline.md → CLAUDE.md → 이 파일 순으로 우선한다. 작성 2026-10-11, Claude Code 문서 기준(아래 §1의 출처).

> 2026-10-18 문서 재편: 설계의 단일 기준은 `docs/`다. 이 파일의 `guideline.md §N` 참조는 `docs/SECTION_MAP.md`로 푼다(원문 동결본 `docs/archive/guideline_v0.8_2026-10-18.md`). 현재 단계·다음 작업은 `STATUS.md`, 마일스톤·작업 표는 `docs/50_ROADMAP.md`, 지난 작업 기록은 `docs/CHANGELOG.md`.

## 0. 세션 시작 절차 (매 세션 공통)

1. `git log --oneline -8`로 마지막 커밋을 확인하고, `STATUS.md`와 `docs/60_DECISIONS/OPEN_QUESTIONS.md`(미결)를 읽는다.
2. `/effort status`로 지금 세션의 effort를 확인하고, 이 파일 §2에서 현재 Phase의 행을 찾아 맞춘다(`/effort <level>`, 세션 한정은 슬라이더에서 `s`).
3. ultracode가 필요한 작업(§2 표의 "ultracode" 표기)이면 사용자가 프롬프트에 `ultracode`를 적거나 `/effort ultracode on`을 켠다. Claude는 지시서가 ultracode를 요구하는 작업에서 그것이 꺼져 있으면 **한 줄로 켜 달라고 요청**하고, 켜지기 전에는 단독으로 진행하지 않는다(리뷰 없이 넘어가지 않는다). 반대 방향도 알린다: Claude는 작업이 바뀔 때마다(브리핑과 보고의 끝에) **다음 작업의 effort와 ultracode 켬/끔을 한 줄로** 적는다. 예: "다음: 로그 Subsystem 구현 — effort xhigh, ultracode 끔(단독). 작업이 끝나면 리뷰 때 켜 달라고 요청한다." ultracode가 세션에 켜져 있으면 Claude는 실질 작업마다 워크플로를 띄우므로, 단독 구간에서는 꺼 두고 리뷰·감사 턴에만 프롬프트에 `ultracode`를 적는 쪽이 싸다(2026-10-12 사용자 피드백).
4. 작업 단위는 "컴파일되는 상태"(guideline.md §10.5). 헤더만 추가하고 끝내지 않는다. 커밋 메시지는 한국어 요약 + 작업 접두어(예 `[P3-12]`) + `Co-Authored-By` 트레일러. 작업 표에는 날짜만 적고 "표에 커밋 해시를 적는" 별도 커밋은 만들지 않는다(`git log --grep`으로 찾는다).
5. 세션 끝에는 `STATUS.md`를 갱신하고(일지·리뷰 통계는 `docs/CHANGELOG.md`), 결정이 바뀌었으면 `docs/`의 해당 설계 문서를 먼저 고친 뒤 코드를 고친다.
6. 제안·리뷰·보고는 '추가 아이디어와 보완'으로 끝낸다(v0.8). 그 가운데 작업 범위 안의 작은 보완(툴팁 한 줄, 문구 하나, 표시 하나처럼 설계를 바꾸지 않는 것)은 목록에만 두지 않고 바로 구현해 그 작업의 커밋에 넣는다(2026-10-18 개발자). 설계를 바꾸거나 개발자의 결정이 필요한 것만 물음으로 남긴다. 결정이 바뀌면 guideline §16의 확정 기록에 근거 열(구절·절)을 둔다. 단계가 바뀔 때마다 effort와 ultracode 켬/끔을 함께 말한다.

## 1. 설정 방법 (Claude Code 공식 문서로 확인한 사실, 2026-10-11)

**모델**
- 세션 중 전환: `/model opus` (새 세션 기본값으로도 저장. `/model`만 치면 선택기, `s`는 세션 한정). 실행 시: `claude --model opus`. 프로젝트 고정: `.claude/settings.json`의 `"model"` 키 — 이 저장소에는 `"claude-opus-5-5"`(정확한 ID)로 넣어 두었다. `opus` 별칭은 최신 Opus를 가리키므로 버전이 바뀔 수 있다. Opus 5.5는 Claude Code v2.1.280 이상.
- CLAUDE.md로는 모델·effort를 고정할 수 없다(문서에 그런 기능이 없음). 고정은 설정 파일 또는 플래그로만.

**effort(추론 강도)**
- 값: `low` / `medium` / `high` / `xhigh` / `max`. Opus 5.5는 다섯 값 모두 지원하며 **기본값은 medium**이다.
- 우선순위: 명시(환경 변수 `CLAUDE_CODE_EFFORT_LEVEL`, `claude --effort <level>`, `/effort <level>`) > 저장된 설정(`modelSettings`의 모델별 값 또는 `effortLevel`) > 모델 기본값.
- `/effort <level>`은 그 모델의 기본값으로 사용자 설정에 저장된다(세션 한정은 슬라이더에서 `s`). `max`는 세션 한정(환경 변수로만 지속). `/effort status`로 현재 값 확인, `/effort auto`로 저장값 해제. 프롬프트에 `ultrathink`를 적으면 그 턴만 더 깊이 생각한다(세션 effort는 그대로).
- 프로젝트 기본값: 이 저장소의 `.claude/settings.json`에 `"effortLevel": "high"`를 두었다(프로젝트 설정의 `effortLevel`은 Opus 5.5에도 적용된다. 사용자 설정의 최상위 `effortLevel`은 Opus 5.5에 적용되지 않으니 주의). 허용값은 low~xhigh(`max`는 설정 파일에 넣을 수 없다).
- 서브에이전트: Agent 도구의 `effort` 인자(v2.1.292+)와 Workflow 스크립트 `agent(prompt, { effort })`(세션에 번들된 workflow-authoring 참조에 명시)로 호출 단위 지정. `.claude/agents/<이름>.md` 프런트매터의 `effort`/`model`도 가능하지만 호출 시 인자가 그보다 우선한다. `CLAUDE_CODE_EFFORT_LEVEL`은 서브에이전트 effort까지 전부 덮어쓴다.

**ultracode(다중 에이전트 워크플로)**
- 뜻: 실질적인 작업마다 Claude가 Workflow 도구로 여러 서브에이전트를 설계·실행하는 모드. **effort는 바꾸지 않는다**(세션 effort 그대로).
- 켜기: 프롬프트에 `ultracode`를 적는다(그 턴만. `-p`·예약 작업·외부 웹훅 페이로드에서는 무시) / `/effort ultracode on`(세션, v2.1.284+. `/effort ultracode off`로 해제) / `claude --effort ultracode`(xhigh + ultracode로 시작, v2.1.203+) / 설정 파일 `"ultracode": true`(지속. 단독으로는 effort가 medium이므로 `effortLevel`을 함께 둔다). `/ultracode` 명령은 없다.
- 전제: 동적 워크플로가 켜져 있고(기본 ON, Pro 플랜은 `/config`의 "Dynamic workflows"에서 켬) 모델이 xhigh를 지원할 것. 끄는 길: `/config` 토글, `"disableWorkflows": true`, `CLAUDE_CODE_DISABLE_WORKFLOWS=1`.
- 규모: `/config`의 "Dynamic workflow size" 또는 설정 `"workflowSizeGuideline"`(unrestricted / small <5 / medium <10 / large <50, 기본 medium). 권고치이며 상한은 아니다. 동시 실행 상한은 기본 16(`CLAUDE_CODE_WORKFLOW_MAX_CONCURRENT_AGENTS`).
- 이 저장소에는 `"ultracode": true`를 **넣지 않았다**. 리뷰·감사·탐색 작업에서만 켜는 것이 아래 §2의 방침이다.

**이 저장소의 `.claude/settings.json`** (팀 공유 설정. 개인 취향은 `.claude/settings.local.json`에):
```json
{
  "model": "claude-opus-5-5",
  "effortLevel": "high",
  "workflowSizeGuideline": "medium"
}
```

**확인하지 못한 것**(사실로 적지 않는다): 스킬 프런트매터의 `effort`가 활성 중 세션 레벨을 덮는지, `/model`이 저장하는 설정 파일 위치. 출처: code.claude.com/docs/en/ 의 model-config, workflows, settings-reference, settings, commands, cli-reference, env-vars, sub-agents, vs-code.

## 2. Phase별 지시 (guideline.md §11 기준)

"effort"는 그 Phase에서 메인 세션에 맞출 값(`/effort`)이고, 괄호는 Phase 안에서 달리 적용할 작업이다. "ultracode"는 Workflow 도구로 여러 에이전트를 돌리는 방식이며 **표기된 작업에서만** 켠다(그 외는 단독).

| Phase | 작업 | effort | 협업 방식 | 이유 |
|---|---|---|---|---|
| 0 셋업 | 완료(2026-10-10). 재실행 시 빌드·스모크 확인만 | low | 단독 | 기계적 검증 |
| 1 데이터 엔진 프로토타입 (2주) | `Core/CosmicTypes.h`, `Core/GalaxyState.h`, DataTable 행 구조체, `NewGalaxy`, 틱 열 단계 골격(Phase 1은 여덟 단계였고 건넘·앎 두 단계는 'v0.7 예약' 커밋이 빈 본문으로 더한다), 저장/로드 왕복, 결정론 테스트 | **xhigh** | 헤더 설계는 단독으로 초안 → **ultracode 리뷰 1회**(§7 스케치·§9.1 예약 필드·§7.4 틱 순서 대조, 결정론·저장 호환 적대 검증) → 승인 후 .cpp 단독 | 모든 상위 Phase가 이 헤더에 묶인다. 예약 필드 누락은 SaveVersion 마이그레이션이 된다 |
| v0.7 예약 (Phase 2 첫 헤더 전, 1.5~2일) | 저장 필드 헤더(`Knowing`·`Transits`·`NextCivSerial`·`TellRngState`·`GalaxyRow`·`OriginGalaxyRow`·`FocusCivID`·`bUntold`), `Core/CosmicSeed.h`, 말하기 스트림, `ComputeWorldHash()`, 빈 `TickCrossings`·`TickSight`, 테스트 2종 추가와 4종 보강 | **xhigh** | 단독(헤더 먼저 제시 → 승인 → 소스) → **ultracode 리뷰 1회**(저장 호환·해시 제외 규칙·말하기 스트림 분리 적대 검증) | 저장되는 필드와 해시 규칙이 이 커밋에서 굳는다. 빠뜨린 필드는 마이그레이션이 되고, 잘못 든 해시와 섞인 스트림은 뒤의 모든 Phase의 결정론 테스트를 흔든다 |
| 2 이벤트·위기·승천 (3주 + 5~6일 + 스키마 검사기 3일) | 조건/효과 평가기, Hazard 8종 골격(`Hazard.Claim` 포함 — Phase 2에는 5종: 초신성 전조·AI 각성·역병·반란·주장. 절멸·교단은 Phase 4, 아공간 침식은 6+), `PendingRoleEffect`·역할 효과 해소, 은혜 이벤트, 로스터 추첨, 업적 평가, 테스트 | **xhigh** | 서브시스템 하나 끝날 때마다 **ultracode 리뷰**(틱 순서·결정론·난수의 출처 셋(시뮬 스트림·말하기 스트림·파생 정수)만 사용·상태는 `FGalaxyState` 안·재진입) / 구현은 단독 | 시뮬 규칙의 정합성이 게임의 전부다. 재진입·순서 버그는 늦게 발견될수록 비싸다. 틱 열 단계 순서·재진입 방지·난수의 출처 셋만 쓰는 결정론·주장 발생 규칙·로스터 추첨은 하나 틀리면 테스트 전체가 흔들려 헤더(Phase 1)만큼 되돌리기 비용이 크다 |
| 3 은하 맵 뷰포트 (4주 + 꼬리 5일) | 성계 액터·머티리얼·카메라·HUD·로그 패널·영적 시야 파라미터(꼬리), v0.8 꼬리(시간 표시 토글, 셋째 칸의 ×25·명령 사유·'다음 칸까지') | **medium** | 단독. 종료 조건 달성 뒤 **ultracode 리뷰 1회**(Core Logic이 UI/Actor를 include하지 않는지, Presentation만 파생인지) | 시각 작업은 사용자 피드백 루프가 더 중요하다. 아키텍처 경계만 기계적으로 지키면 된다 |
| v0.8 예약 (Phase 3 종료 뒤, Phase 4 첫 헤더 전, 11~15.5일) | ① 성역과 첫 생애 행(4a·4b, 헤더 한 번·커밋 둘) ② 실제 하늘(관측 목록 도구·`DT_Planets`·투영과 레인·별 이름·등급별 문구·에너지 정규화) | **xhigh**(② 지도 표현은 medium) | 헤더 승인. ①은 ultracode 리뷰 1회(저장·결정론), ②는 ultracode 리뷰(생성·결정론, 지도 표현은 끔) | 저장 ID·생성 추첨·세계 기준값이 바뀐다 |
| 저장 작업 (Phase 4 첫 블록 전) | §16 #33 ①②③·#70·#40 ⑦ | **xhigh** | 헤더 승인 + ultracode 리뷰 | 자동 저장 트리거와 첫 패키징 전 |
| A Playable MVP(①~⑨, 2026-10-20~24) | MVP 항목마다(②~⑨) 헤더(`docs/60_DECISIONS/headers/A<N>_*.md`) → 구현 → 문서 | 헤더 **xhigh**, 구현 high(⑨의 결정론·빌드 코드는 xhigh; ⑤는 헤더 결정 18로 high), 문서 xhigh | 헤더는 조사 → 초안 → **ultracode 적대 리뷰** → 승인; 결정론·저장·빌드의 구현은 구현 리뷰(⑨는 **ultracode**, ⑤는 서로 독립인 리뷰 넷 — ultracode 끔); 문서는 **재감사 워크플로**. 패키징(⑨)의 빌드·쿠킹 실행 자체는 medium(`Tools/package.ps1`) | 플레이 가능한 한 판을 묶는 항목들이라 하나가 틀리면 종료 조건이 무너진다. 빌드·쿠킹은 기계적이지만 그 판정(ⓓ)은 결정론이다 |
| B 코어 시뮬 완성(B1~B8, 2026-10-25~) | 블록마다 헤더(`docs/60_DECISIONS/headers/B<N>_*.md`) → (저장 필드를 더하는 블록은 필드만의 커밋 — 해시만 옮긴다, B3) → 장치(기준값 비트 그대로) → 표현 → 밸런스(데이터를 켜고 재어 기준값을 다시 적는다; 해시를 옮기는 새 길과 데이터 켜기는 커밋을 나눈다 — B3의 `[B-3d1]`·`[B-3d2]`) → 문서 | 헤더 **xhigh**, 필드·장치(결정론·저장) xhigh, 표현·밸런스 high, 문서 xhigh | 헤더는 조사 → 초안 → **ultracode 적대 리뷰** → 승인; 장치는 **ultracode** 글 감사와 구현 리뷰, 표현은 **ultracode** 화면·문구 리뷰, 밸런스는 **ultracode** 밸런스 리뷰(측정의 방법과 수), 문서는 **재감사 워크플로**(B1·B2·B3가 이렇게 했다) | 세계와 기준값을 옮기는 블록이라 A와 같은 급이다 |
| 4 방랑자 vs 통치자 루프 (6주 + 1~1.5일) | 영압 함수·수정자, 청원 카드, `Hazard.Claim` 해결 콘텐츠, 길 1종, 사자 카드, 계시 템플릿 21줄, 지구 행(v0.8 — 단계 4b부터 첫 생애 행)·책 이벤트, 로그 템플릿. v0.8: 첫 블록 '서막(부르심)'(조우 카드 화면 먼저, 가칭 `ReceiveCalling`, `UI.PrologueGate`), 외계 백성 VS 5행·첫 생애 행 본문·고넬료 사슬·'옛 땅의 기억'·헤롯 패턴 청원·대도감 문장 | **high**(코드·텍스트 공통) | 코드는 단독 + 리뷰 1회. 텍스트는 단독으로 쓰고 **묶음마다 ultracode 감사**(§8.3 규칙: 두 문장·숫자 없음·화자 3층·사후 문장 규칙·Tier C 금지어·길/교리 어휘 분리). v0.8 몫: 텍스트 묶음마다 감사(ultracode), 코드는 단독 + 리뷰 1회 | 텍스트는 양이 많고 규칙이 많다. 쓰는 것은 한 사람 톤이 낫고, 검사는 여러 눈이 낫다. 텍스트도 §8.3 규칙(두 문장·숫자 없음·화자 3층·사후 문장·금지어·어휘 분리)을 작성 중에 지켜야 하므로 medium으로는 부족하고, Phase 전체를 high로 통일하면 세션 중 `/effort`를 오갈 일이 없다 |
| 4b 부름 루프 (2주 + 0.5일) | 사망 파이프라인, `RunHeadless` 공백기, 계승 추첨, `OfferReturn` 훅, 3화면, 두 책 장, 테스트 8종 | **xhigh** | **ultracode 리뷰 필수**(저장 왕복·도약 결정론·`LeapTotalCap`·귀환 수락/거절 양 경로) | 플레이어가 죽는 순간의 버그는 판 전체를 잃게 한다. 사망 파이프라인·공백기·저장 왕복·귀환 양 경로가 얽혀 Phase 2와 같은 급이다 |
| 5 수직 슬라이스 폴리시 (4주 + 1일) | 밸런스 커맨드렛·CSV·목표치, 튜토리얼 로그, 테스터 질문 운영, 린트 테스트(`Data.NoVerdictWords` 등). v0.8: '세계관과 출처' 최소형(medium, 텍스트 감사만 ultracode) | **medium** | 커맨드렛 결과 분석·튜닝 제안은 **ultracode**(시드·정책 조합을 병렬로 돌려 비교) / 수정은 단독 | 밸런스는 탐색 폭이 중요하고 한 번에 여러 가설을 검증하는 편이 싸다 |
| 6a 건넘 최소형 (Phase 6+의 첫 블록, 2~3주) | `SummaryAt`, `TickCrossings`의 도착·출발, `project_crossing`과 AI 규칙, AI의 T3→T4 판정과 `AIFourthRungCap`, 건넘 로그 4행, `MetGalaxies`, EA 규모 데이터, 테스트 4종 | **xhigh** | 구현은 단독 → 끝난 뒤 **ultracode 리뷰**(착지 추첨, 날아가는 중인 기록을 둔 저장 왕복, 세계 해시) | 시뮬 추첨 지점이 Phase 5 뒤에 늘어나는 하나뿐인 곳이고 문명 배열에 원소를 더하는 첫 코드다. 아래 6+ 콘텐츠 행의 high가 아니라 Phase 2와 같은 급으로 다룬다 |
| 6+ 콘텐츠 확장·EA | 아키타입 EA 행, 교리 행(회귀·광신·동화), 독립 초월자, T5 문턱·후원 카드, 모티프 24종, 헌정명 뱅크 | **high** | 콘텐츠 묶음마다 **ultracode 감사**(신학 정합·톤·반복·금지어) / 작성은 단독 | 양이 많고 민감한 영역(§8.3)이라 검사 비용이 작성 비용보다 싸다. 교리 행·독립 초월자·후원 카드는 신학 민감도가 가장 높아 Phase 4 텍스트와 같은 high로 쓴다 |
| 설계 문서 변경 (모든 Phase) | guideline.md 규칙·결정 변경 | **xhigh** | 한두 문장 수정은 단독. 섹션 단위 변경·새 제안 채택은 **ultracode 리뷰 → 적용 → 재감사**(2026-10-11의 v0.4·v0.5 절차) | SSoT의 모순은 코드 버그보다 오래 산다. 3,000줄 문서의 교차 참조를 놓치면 재감사에서 30건씩 다시 걸린다(2026-10-11에 두 번 겪었다) |

v0.8로 더한 줄의 낱말(결정은 guideline §16 'v0.8 확정'): **칸**은 존재의 사다리 다섯 단(Tier)이다 — 거대 프로젝트의 '건설 칸·대응 칸'은 성계의 프로젝트 자리라 다른 말이다. **성역**은 태양계 노드 `sol` — 다시 쓸 수 없는 법 하나로 소유·정착·주장·파괴에서 빠지는 곳이고(v0.8 — 단계 4부터. 그 전의 코드는 `sol`이 플레이어 모성계다), 그 바깥 전부가 **성역 밖**이다(설계 문서의 낱말 — 게임 안에는 이름이 없다). **첫 생애 행**은 플레이어의 첫 생애 백성을 만드는 `DT_Civs` 행(가칭 `arch_seeker` — 단계 4b부터. 그 전의 데이터는 `arch_terran`)이고, 그 백성은 계시 양식(그 백성에게 말이 닿는 꼴)이 '찾음'(`Revelation.Seeking`)인 **찾는 백성**이다. **서막(부르심)**은 첫 틱 전의 카드 서막이다 — 성역 밖 먼 별의 필멸의 한 사람이 부르심을 입고 한 백성을 맡는다(서막은 칸이 아니다). **고넬료 사슬**은 방문 응답 → 사자의 전언 → "말이 도착했다"로 이어지는 사건 줄이고, **'옛 땅의 기억'**은 말이 도착한 뒤 열리는 한 쪽짜리 화면이다(옛 땅 — 성역 — 의 이야기를 그 백성의 두 목소리, 회중과 의회가 받아 적은 것). **헤롯 패턴 청원**은 백성이 통치자를 신이라 부르려 하는 청원 카드이고, **대도감**은 개발자의 `cosmic_species_codex.md`(성역 밖의 세계관 원천, 작가 노트 — 고치지 않는다)다.

### 작업 유형별 기본값 (Phase 표와 충돌하면 Phase 표가 우선)

| 작업 유형 | effort | 협업 |
|---|---|---|
| 헤더·구조체·열거형 설계, 저장 포맷, 결정론에 닿는 코드 | xhigh | 초안 단독 → ultracode 적대 리뷰 → 수정 |
| 단일 클래스 .cpp 구현, 테스트 작성 | high | 단독 |
| 한국어 템플릿·조우 카드·연대기 문장 | high | 단독 작성 → 묶음 감사(ultracode) |
| 빌드·테스트 실행, 로그 확인, 기계적 리네임, ini 태그 추가 | low | 단독 |
| 버그 원인 조사(재현 가능) | high (해시 불일치·저장 왕복 실패 등 결정론·저장 관련은 xhigh — 원인이 한 필드 누락일 때가 많다) | 단독. 원인이 둘 이상 의심되면 ultracode로 가설 병렬 검증 |
| 엔진 API 확인(5.8 소스 대조) | medium | 단독(`C:\Program Files\Epic Games\UE_5.8\Engine\Source`에서 선언 헤더 확인) |

### effort 세 단 요약과 세션 시작 시 설정

- **코드와 문서는 xhigh**(Phase 1·'v0.7 예약'·2·'v0.8 예약'·저장 작업·4b·6a, 설계 문서 변경, 결정론·저장 관련 버그), **콘텐츠 작성은 high**(Phase 4·6+ 텍스트, 일반 .cpp·테스트·버그 조사), **시각·기계 작업은 medium 이하**(Phase 3·5, 빌드·테스트 실행, 엔진 API 확인).
- 프로젝트 설정 파일의 기본값은 `high`다. xhigh가 필요한 Phase(1·2·4b)와 'v0.7 예약'·'v0.8 예약'·6a 블록, 저장 작업, 설계 문서 변경 세션은 **시작할 때 `/effort xhigh`를 명시**한다(세션 한정은 슬라이더에서 `s`). 설정 파일 기본값을 xhigh로 올리지 않는다 — 모든 작업이 비싸진다.
- `max`는 세션 한정이고 비용 대비 효과가 불확실하므로 메인 세션에는 쓰지 않는다. Phase 1 헤더 리뷰 워크플로의 **적대 검증 에이전트에만** `agent(prompt, { effort: 'max' })`로 준다.

### ultracode를 켜지 않는 경우

- 수정 범위가 한 파일·한 함수인 작업, 사용자가 화면을 보며 왕복해야 하는 UI 조정, 빌드 오류 수정.
- 이미 리뷰를 통과한 코드를 같은 세션에서 변경 없이 다시 리뷰하는 일.

### ultracode를 켤 때의 표준 패턴

1. **적대 리뷰**(코드·헤더): 관점 3개(정합성 / 결정론·저장 / 가이드라인 대조)로 읽기 전용 감사 → 확정(CONFIRMED)된 발견만 수정 → 수정 뒤 재감사는 변경 범위만.
2. **텍스트 감사**(콘텐츠): 관점 3개(§8.3 작성 규칙 / 신학·톤 §4.5·§8.3 / 반복·슬롯·금지어 린트) → 문장 단위 교체안.
3. **밸런스 탐색**(Phase 5): 시드·정책 조합을 병렬 실행 → CSV 비교 → 노브 제안.
4. 워크플로의 읽기 전용 에이전트에게는 **스냅샷 파일**을 주고, 편집은 메인 세션 또는 단일 편집 에이전트가 한다(같은 파일 동시 편집 금지). 워크플로 스크립트의 템플릿 문자열 안에 백틱을 넣지 않는다(파싱 실패).


## 3. 다음 작업

`STATUS.md`(현재 단계·다음 3작업·열린 결정 Top 5)와 `docs/50_ROADMAP.md`(마일스톤 0 → A0 → A Playable MVP → B → C, 현재 작업 표)를 본다. 옛 §3의 작업 표는 ROADMAP으로, 작업 기록과 부록 A는 `docs/CHANGELOG.md`로 옮겼다(2026-10-18).
