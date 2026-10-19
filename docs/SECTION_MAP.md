# SECTION MAP — 옛 guideline.md v0.8 §번호 → 새 문서

> 코드 주석의 `guideline.md section N`·문서의 `§N`은 이 표로 푼다. 원문(동결본)은 `docs/archive/guideline_v0.8_2026-10-18.md`(2026-10-19 원격 반영분까지 든 마지막 판)이고 행 번호는 그 파일 기준이다. 절 번호는 재사용하지 않는다(새 절은 새 문서에 쓴다).

| § | 제목 | 원문 행 | 새 위치 |
|---|---|---|---|
| §0 | 초안(v0.1) 검토 결과 및 변경 요약 | 128 | CHANGELOG.md |
| §1 | 프로젝트 비전 및 핵심 철학 | 160 | 00_VISION.md |
| §1.1 | 한 줄 피치 | 164 | 00_VISION.md |
| §1.2 | 게임 개요 | 172 | 00_VISION.md |
| §1.3 | 1인 개발 최적화 철학 (Lean Simulation) | 187 | 00_VISION.md |
| §1.4 | 참조 게임과 차용 요소 | 220 | 00_VISION.md |
| §2 | 플레이어 경험: 코어 루프 | 242 | 00_VISION.md |
| §2.1 | 시간 모델 | 246 | 00_VISION.md |
| §2.2 | 통치자(Ruler) 10분 루프 | 256 | 00_VISION.md |
| §2.3 | 방랑자(Wanderer) 10분 루프 | 287 | 00_VISION.md |
| §2.4 | 한 판의 호(Arc) | 321 | 00_VISION.md |
| §3 | 핵심 게임플레이 3대 기둥 | 348 | 10_DESIGN/pillars.md |
| §3.1 | Pillar 1: 추상화된 문명 및 경제 시뮬레이션 (Data Engine) | 380 | 10_DESIGN/pillars.md |
| §3.2 | Pillar 2: 존재 등급(Ascension Tier)과 초월자 시스템 | 422 | 10_DESIGN/pillars.md |
| §3.3 | Pillar 3: 이원화된 플레이어 경험 (Dual Role Loop) | 491 | 10_DESIGN/pillars.md |
| §3.4 | 현실에서 가져온 것 | 535 | 10_DESIGN/pillars.md |
| §4 | 긴장과 위기: 샌드박스를 '게임'으로 만드는 장치 | 612 | 10_DESIGN/crisis_and_transcendents.md |
| §4.1 | 위기 생성기 (Crisis Generator) | 620 | 10_DESIGN/crisis_and_transcendents.md |
| §4.2 | 초월자 (타락·천상·독립) | 683 | 10_DESIGN/crisis_and_transcendents.md |
| §4.3 | 경쟁 승천 문명과 시계 | 778 | 10_DESIGN/crisis_and_transcendents.md |
| §4.4 | 실패 상태와 엔딩 | 787 | 10_DESIGN/crisis_and_transcendents.md |
| §4.5 | 거룩한 질서 (Divine Order) | 934 | 10_DESIGN/crisis_and_transcendents.md |
| §5 | 게임 시스템 상세 (린 설계) | 990 | 10_DESIGN/systems.md |
| §5.1 | 자원 (5종) | 998 | 10_DESIGN/systems.md |
| §5.2 | 문명 윤리 3축 | 1016 | 10_DESIGN/systems.md |
| §5.3 | 특성 (Traits) | 1033 | 10_DESIGN/systems.md |
| §5.4 | 연구 (4개 영역, 트리 없음) | 1042 | 10_DESIGN/systems.md |
| §5.5 | 외교 (태세 + 조약 3종) | 1059 | 10_DESIGN/systems.md |
| §5.6 | 분쟁 (완전 추상화) | 1070 | 10_DESIGN/systems.md |
| §5.7 | 거대 프로젝트 (Megaprojects) | 1083 | 10_DESIGN/systems.md |
| §5.8 | 칙령 (Edicts) | 1137 | 10_DESIGN/systems.md |
| §5.9 | 방랑자 시스템 | 1147 | 10_DESIGN/systems.md |
| §5.10 | 역할 전환 규칙 | 1198 | 10_DESIGN/systems.md |
| §5.11 | 문명 아키타입 (DT_Civs) | 1244 | 10_DESIGN/systems.md |
| §5.12 | 층을 지키는 두 규칙 | 1468 | 10_DESIGN/systems.md |
| §6 | UE5 기술 아키텍처 | 1488 | 20_ARCHITECTURE.md |
| §6.1 | 모듈 계층 구조 | 1492 | 20_ARCHITECTURE.md |
| §6.2 | Subsystem 선택 근거 | 1545 | 20_ARCHITECTURE.md |
| §6.3 | UI 기술 선택: UMG 우선 | 1555 | 20_ARCHITECTURE.md |
| §6.4 | 3D 뷰포트 (최소) | 1576 | 20_ARCHITECTURE.md |
| §6.5 | Niagara 예산 (3종) | 1625 | 20_ARCHITECTURE.md |
| §6.6 | 모듈 의존성 (Build.cs) | 1642 | 20_ARCHITECTURE.md |
| §7 | 데이터 모델 및 C++ 설계안 | 1672 | 21_DATA_MODEL_REFERENCE.md |
| §7.1 | 공통 타입 (Core/CosmicTypes.h) | 1684 | 21_DATA_MODEL_REFERENCE.md |
| §7.2 | 런타임 상태 (Core/GalaxyState.h) | 1788 | 21_DATA_MODEL_REFERENCE.md |
| §7.3 | 정의 데이터 (Data/*.h) | 2252 | 21_DATA_MODEL_REFERENCE.md |
| §7.4 | 시뮬레이션 Subsystem (Sim/GalaxySimulationSubsystem.h) | 2873 | 21_DATA_MODEL_REFERENCE.md |
| §7.5 | 승천·이벤트·로그 Subsystem 인터페이스 요약 | 3141 | 21_DATA_MODEL_REFERENCE.md |
| §7.6 | 초안 코드와의 차이 | 3249 | 21_DATA_MODEL_REFERENCE.md |
| §8 | 프로시저럴 텍스트 / 로그 시스템 | 3267 | 70_CONTENT_GUIDE.md |
| §8.1 | 로그 항목 | 3275 | 70_CONTENT_GUIDE.md |
| §8.2 | 템플릿 문법 | 3339 | 70_CONTENT_GUIDE.md |
| §8.3 | 서사 가이드 (작성 규칙) | 3386 | 70_CONTENT_GUIDE.md |
| §8.4 | 상한과 연대기 | 3461 | 70_CONTENT_GUIDE.md |
| §8.5 | 로컬라이제이션 | 3486 | 70_CONTENT_GUIDE.md |
| §8.6 | 헌정명과 모티프 뱅크 | 3495 | 70_CONTENT_GUIDE.md |
| §9 | 저장, 결정론, 테스트, 밸런스 | 3529 | 20_ARCHITECTURE.md |
| §9.1 | 저장/로드 | 3537 | 20_ARCHITECTURE.md |
| §9.2 | 결정론 | 3552 | 20_ARCHITECTURE.md |
| §9.3 | 테스트와 1인 밸런싱 | 3606 | 40_DEV_GUIDE.md (옛 §9.3) |
| §9.4 | 성능 예산 | 3770 | 20_ARCHITECTURE.md |
| §10 | 프로젝트 구조 및 코딩 규약 | 3799 | 40_DEV_GUIDE.md |
| §10.1 | 폴더 구조 | 3803 | 40_DEV_GUIDE.md |
| §10.2 | C++ / Blueprint 분담 | 3875 | 40_DEV_GUIDE.md |
| §10.3 | 소스 텍스트 규칙 | 3888 | 40_DEV_GUIDE.md |
| §10.4 | 명명 | 3897 | 40_DEV_GUIDE.md |
| §10.5 | 버전 관리 | 3907 | 40_DEV_GUIDE.md |
| §11 | 로드맵 (현실화) | 3919 | 50_ROADMAP.md (상태 줄은 CHANGELOG.md) |
| §12 | 범위 관리: 수직 슬라이스와 미구현 목록 | 4094 | 50_ROADMAP.md |
| §12.1 | 수직 슬라이스 정의 (Phase 5 종료 시점) | 4098 | 50_ROADMAP.md |
| §12.2 | 명시적 미구현 목록 (Cut List) | 4134 | 50_ROADMAP.md |
| §12.3 | 범위 추가 절차 | 4157 | 50_ROADMAP.md |
| §13 | 리스크와 대응 | 4182 | 50_ROADMAP.md |
| §14 | Claude Code 협업 규칙 | 4222 | 40_DEV_GUIDE.md |
| §14.1 | 요청 단위 | 4226 | 40_DEV_GUIDE.md |
| §14.2 | 검증 | 4236 | 40_DEV_GUIDE.md |
| §14.3 | CLAUDE.md 템플릿 (프로젝트 루트) | 4245 | (§14.3 CLAUDE.md 템플릿 — 중복이라 삭제; 불변 규칙 전문은 20_ARCHITECTURE.md 부록) |
| §14.4 | 좋은 요청 예시 | 4323 | 40_DEV_GUIDE.md |
| §15 | 용어집 | 4339 | 15_GLOSSARY.md |
| §16 | 미결 사항 (결정 필요) | 4478 | 60_DECISIONS/OPEN_QUESTIONS.md |

## 옛 CLAUDE.md 절 → 새 위치

| 절 | 새 위치 |
|---|---|
| 프로젝트 | CLAUDE.md(요약), docs/45_ENGINE_NOTES.md(EngineAssociation) |
| 아키텍처 불변 규칙 | CLAUDE.md(항목 요약), docs/20_ARCHITECTURE.md 부록(전문) |
| 작업 방식 | CLAUDE.md |
| 빌드 / 실행 / 테스트 | CLAUDE.md(요약), docs/40_DEV_GUIDE.md(전문) |
| 모듈 / UHT / 컴파일 규칙 | CLAUDE.md |
| 검증된 엔진 헤더 경로 | docs/45_ENGINE_NOTES.md |
| 렌더 / 플러그인 결정 | docs/60_DECISIONS/DECISIONS_LOG.md ADR-0001 |
| 현재 단계 | STATUS.md(현재), docs/CHANGELOG.md(기록) |

## 옛 WORKPLAN.md·루트 md → 새 위치

| 원본 | 새 위치 |
|---|---|
| WORKPLAN §0~§2 | WORKPLAN.md(유지) |
| WORKPLAN §3 작업 표(v0.8 단계·Phase 3·v0.8 예약·다음) | docs/50_ROADMAP.md |
| WORKPLAN §3 작업 기록·부록 A | docs/CHANGELOG.md |
| v08_rulings.md A·D·E | docs/archive/v08_rulings.md (결정은 DECISIONS_LOG의 v0.8 확정 블록에 이미 흡수됨) |
| v08_rulings.md B·C | docs/90_LORE/theology_and_bestiary_notes.md |
| v08_agent_directive.md | docs/archive/ (결정 15·13·손 확인은 DECISIONS_LOG v0.8 확정 블록) |
| code_artifact.md | docs/archive/ (대도감 옛 판 — 원천 아님) |
| cosmic_species_codex.md | docs/90_LORE/cosmic_species_codex.md |
