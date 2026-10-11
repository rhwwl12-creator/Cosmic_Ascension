# Cosmic Ascension — 작업지시서 (WORKPLAN)

> 앞으로의 Claude Code 세션(모델: **Opus 5.5**)이 Phase·작업별로 **어떤 추론 강도(effort)**와 **어떤 협업 방식(단독 / ultracode 워크플로)**으로 일할지 정한 지시서다. 설계의 단일 기준은 `guideline.md`(SSoT), 작업 규칙은 `CLAUDE.md`, 이 파일은 **진행 방식**만 다룬다. 셋이 충돌하면 guideline.md → CLAUDE.md → 이 파일 순으로 우선한다. 작성 2026-10-11, Claude Code 문서 기준(아래 §1의 출처).

## 0. 세션 시작 절차 (매 세션 공통)

1. `git log --oneline -8`로 마지막 커밋을 확인하고, `CLAUDE.md`의 "현재 단계"와 `guideline.md` §16(미결)을 읽는다.
2. `/effort status`로 지금 세션의 effort를 확인하고, 이 파일 §2에서 현재 Phase의 행을 찾아 맞춘다(`/effort <level>`, 세션 한정은 슬라이더에서 `s`).
3. ultracode가 필요한 작업(§2 표의 "ultracode" 표기)이면 사용자가 프롬프트에 `ultracode`를 적거나 `/effort ultracode on`을 켠다. Claude는 지시서가 ultracode를 요구하는 작업에서 그것이 꺼져 있으면 **한 줄로 켜 달라고 요청**하고, 켜지기 전에는 단독으로 진행하지 않는다(리뷰 없이 넘어가지 않는다).
4. 작업 단위는 "컴파일되는 상태"(CLAUDE.md §10.5). 헤더만 추가하고 끝내지 않는다. 커밋 메시지는 한국어 요약 + `Co-Authored-By` 트레일러.
5. 세션 끝에는 `CLAUDE.md` "현재 단계"를 갱신하고, 결정이 바뀌었으면 guideline.md를 먼저 고친 뒤 코드를 고친다.

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
| 1 데이터 엔진 프로토타입 (2주) | `Core/CosmicTypes.h`, `Core/GalaxyState.h`, DataTable 행 구조체, `NewGalaxy`, 틱 8단계 골격, 저장/로드 왕복, 결정론 테스트 | **xhigh** | 헤더 설계는 단독으로 초안 → **ultracode 리뷰 1회**(§7 스케치·§9.1 예약 필드·§7.4 틱 순서 대조, 결정론·저장 호환 적대 검증) → 승인 후 .cpp 단독 | 모든 상위 Phase가 이 헤더에 묶인다. 예약 필드 누락은 SaveVersion 마이그레이션이 된다 |
| 2 이벤트·위기·승천 (3주) | 조건/효과 평가기, Hazard 8종 골격(`Hazard.Claim` 포함), `PendingRoleEffect`·역할 효과 해소, 은혜 이벤트, 로스터 추첨, 업적 평가, 테스트 | **high** | 서브시스템 하나 끝날 때마다 **ultracode 리뷰**(틱 순서·결정론·`Rng`만 사용·상태는 `FGalaxyState` 안·이벤트 큐 재진입) / 구현은 단독 | 시뮬 규칙의 정합성이 게임의 전부다. 재진입·순서 버그는 늦게 발견될수록 비싸다 |
| 3 은하 맵 뷰포트 (4주) | 성계 액터·머티리얼·카메라·HUD·로그 패널·영적 시야 파라미터(꼬리) | **medium** | 단독. 종료 조건 달성 뒤 **ultracode 리뷰 1회**(Core Logic이 UI/Actor를 include하지 않는지, Presentation만 파생인지) | 시각 작업은 사용자 피드백 루프가 더 중요하다. 아키텍처 경계만 기계적으로 지키면 된다 |
| 4 방랑자 vs 통치자 루프 (6주) | 영압 함수·수정자, 청원 카드, `Hazard.Claim` 해결 콘텐츠, 길 1종, 사자 카드, 계시 템플릿 21줄, 지구 행·책 이벤트, 로그 템플릿 | 코드 **high** / 템플릿·한국어 텍스트 **medium** | 코드는 단독 + 리뷰 1회. 텍스트는 단독으로 쓰고 **묶음마다 ultracode 감사**(§8.3 규칙: 두 문장·숫자 없음·화자 3층·사후 문장 규칙·Tier C 금지어·길/교리 어휘 분리) | 텍스트는 양이 많고 규칙이 많다. 쓰는 것은 한 사람 톤이 낫고, 검사는 여러 눈이 낫다 |
| 4b 부름 루프 (2주) | 사망 파이프라인, `RunHeadless` 공백기, 계승 추첨, `OfferReturn` 훅, 3화면, 두 책 장, 테스트 8종 | **high** | **ultracode 리뷰 필수**(저장 왕복·도약 결정론·`LeapTotalCap`·귀환 수락/거절 양 경로) | 플레이어가 죽는 순간의 버그는 판 전체를 잃게 한다 |
| 5 수직 슬라이스 폴리시 (4주) | 밸런스 커맨드렛·CSV·목표치, 튜토리얼 로그, 테스터 질문 운영, 린트 테스트(`Data.NoVerdictWords` 등) | **medium** | 커맨드렛 결과 분석·튜닝 제안은 **ultracode**(시드·정책 조합을 병렬로 돌려 비교) / 수정은 단독 | 밸런스는 탐색 폭이 중요하고 한 번에 여러 가설을 검증하는 편이 싸다 |
| 6+ 콘텐츠 확장·EA | 아키타입 EA 행, 교리 행(회귀·광신·동화), 독립 초월자, T5 문턱·후원 카드, 모티프 24종, 헌정명 뱅크 | **medium** | 콘텐츠 묶음마다 **ultracode 감사**(신학 정합·톤·반복·금지어) / 작성은 단독 | 양이 많고 민감한 영역(§8.3)이라 검사 비용이 작성 비용보다 싸다 |
| 설계 문서 변경 (모든 Phase) | guideline.md 규칙·결정 변경 | **high** | 한두 문장 수정은 단독. 섹션 단위 변경·새 제안 채택은 **ultracode 리뷰 → 적용 → 재감사**(2026-10-11의 v0.4·v0.5 절차) | SSoT의 모순은 코드 버그보다 오래 산다 |

### 작업 유형별 기본값 (Phase 표와 충돌하면 Phase 표가 우선)

| 작업 유형 | effort | 협업 |
|---|---|---|
| 헤더·구조체·열거형 설계, 저장 포맷, 결정론에 닿는 코드 | xhigh | 초안 단독 → ultracode 적대 리뷰 → 수정 |
| 단일 클래스 .cpp 구현, 테스트 작성 | high | 단독 |
| 한국어 템플릿·조우 카드·연대기 문장 | medium | 단독 작성 → 묶음 감사(ultracode) |
| 빌드·테스트 실행, 로그 확인, 기계적 리네임, ini 태그 추가 | low | 단독 |
| 버그 원인 조사(재현 가능) | high | 단독. 원인이 둘 이상 의심되면 ultracode로 가설 병렬 검증 |
| 엔진 API 확인(5.8 소스 대조) | medium | 단독(`C:\Program Files\Epic Games\UE_5.8\Engine\Source`에서 선언 헤더 확인) |

### ultracode를 켜지 않는 경우

- 수정 범위가 한 파일·한 함수인 작업, 사용자가 화면을 보며 왕복해야 하는 UI 조정, 빌드 오류 수정.
- 이미 리뷰를 통과한 코드를 같은 세션에서 변경 없이 다시 리뷰하는 일.

### ultracode를 켤 때의 표준 패턴

1. **적대 리뷰**(코드·헤더): 관점 3개(정합성 / 결정론·저장 / 가이드라인 대조)로 읽기 전용 감사 → 확정(CONFIRMED)된 발견만 수정 → 수정 뒤 재감사는 변경 범위만.
2. **텍스트 감사**(콘텐츠): 관점 3개(§8.3 작성 규칙 / 신학·톤 §4.5·§8.3 / 반복·슬롯·금지어 린트) → 문장 단위 교체안.
3. **밸런스 탐색**(Phase 5): 시드·정책 조합을 병렬 실행 → CSV 비교 → 노브 제안.
4. 워크플로의 읽기 전용 에이전트에게는 **스냅샷 파일**을 주고, 편집은 메인 세션 또는 단일 편집 에이전트가 한다(같은 파일 동시 편집 금지). 워크플로 스크립트의 템플릿 문자열 안에 백틱을 넣지 않는다(파싱 실패).

## 3. 다음 작업 (2026-10-11 기준)

1. guideline.md **v0.6**: '요약 단위 원칙'(사람·문명·영·육신은 전부 요약 단위; 인물은 기록, 규모 등급은 파생 표시) 반영 — 설계 문서 변경 행의 방식(ultracode 리뷰 → 적용 → 재감사).
2. **Phase 1 시작**: `/effort xhigh`(세션 한정 `s`). `Core/CosmicTypes.h` → `Core/GalaxyState.h` 초안(§7.1·§7.2, §9.1 예약 필드 전부) → `ultracode` 헤더 리뷰 1회 → .cpp와 `Tests/`(§9.3 Determinism·SaveRoundTrip) → 게임 타깃 빌드 → 커밋. 요청 예시는 guideline.md §14.4.
