# STATUS — 현재 단계

갱신: 2026-10-19 (Windows 도구 검증 및 FCosmicEvalContext 헤더 분리 완료; 시드 42 3,000틱 기준값 기록). 세션 시작에 읽고 세션 끝에 이 파일만 갱신한다. 일지·리뷰 통계는 `docs/CHANGELOG.md`에.

## 지금

- **Phase 3(은하 맵 뷰포트) — 작업은 모두 끝, 종료 절차만 남음.** 본체(1~5번·6번 기계 몫·정착 사유·11번 시간 표시 토글)에 이어 꼬리 7번 영적 시야·12번 셋째 칸의 ×25와 명령의 사유·8번 앎의 표시·9번 우주 화면이 2026-10-18~19에 끝났다(원격 커밋 `94e8706`·`b8803ef`·`25104b2`·`25ab43b`, 기록은 `docs/CHANGELOG.md`). 남은 것: 종료 조건의 사람 몫 **손 확인(개발자: 9번까지 든 가장 새 빌드로, 창에서 마우스만으로 셋째 칸, `stat fps`·`stat unit` 60fps — 배경 별의 GPU 몫은 `show InstancedStaticMeshes`로 끄고 켜서 본다)** → 10번 종료 리뷰(ultracode 1회: Core Logic이 UI/Actor를 include하지 않는지, 표현 계층이 앎을 `KnownViewOf` 한 곳에서만 읽는지).
- **설계 문서**: 2026-10-18~19 `docs/`로 재편(1단계 전수 조사 → 2단계 통폐합; 원격 반영분까지 다시 이관). 설계 결정은 `docs/60_DECISIONS/DECISIONS_LOG.md`, 열린 물음은 `OPEN_QUESTIONS.md`(#번호는 옛 guideline §16 그대로, 원격이 더한 #96~#116 포함).
- **코드 사실**: 자동화 테스트 89종 통과, `Sim/CosmicEvalContext.h` 분리 완료로 `check_layers.py` 0 error/0 warning 달성. DT_Rules 노브 46(읽힘 28·예약 18), `DT_Galaxies` 20행, 배속 상한 `MaxTimeScale` 25.
- **기준값**: 커맨드렛 시드 42(성계 60·백성 12, 3000틱) hash 2438129736, world 1171528496 (`Tools/baselines/seed42.json` 기록 완료). `run_checks.ps1 -Tier smoke` 통과.

## 다음 3작업 (`docs/50_ROADMAP.md` 0 → A0)

1. **손 확인**(개발자) → 결과(fps·막힌 조작)를 CHANGELOG에 한 줄 → 10번 종료 리뷰 → Phase 3 종료 선언.
2. **A0 'v0.8 예약 ①'**: 성역 `sol`(배열 끝)·`sys_0`·`Law.Hedge`·`IsSanctuary`(앎 뷰의 둘째 비트 `ShownByName` 포함) + 첫 생애 행 뼈대 `arch_seeker`(행 이름·태그·첫 줄·두 목소리). 가칭 식별자 일괄 확정 → 헤더 승인(`docs/60_DECISIONS/headers/A0_sanctuary_first_life.md` 결정 6개) → 구현 → 재측정 테스트 → ultracode 리뷰 1회(저장·결정론). effort xhigh.
3. **Playable MVP(A)**: 자동 일시정지 → 청원 카드 10장+카드 위젯 → 첫 10분 보장·#68/#71/#72 → 임시 엔딩·연대기 최소형 → 저장 최소형 → 지표·온보딩 → Win64 패키징.

## 열린 결정 Top 5 (`docs/60_DECISIONS/OPEN_QUESTIONS.md`)

1. A0 헤더 초안 승인 — `docs/60_DECISIONS/headers/A0_sanctuary_first_life.md`(식별자 확정 표 + 결정 6개: `Core/CosmicIDs.h`, 레인 없는 성역, 자리, `arch_terran` 삭제, 이름 후보, 금지어 목록 시점). 승인 뒤 Windows에서 4a → 4b 구현.
2. #68 안정도 회복·#71 AI 구호 정책·#72 전조 확률의 기본값 — MVP '세계 손질' 전('#68만'·'#71만' 판을 커맨드렛으로 먼저 잰다).
3. 첫 10분 보장 방식 — 모든 백성 공통 Standing 위기(+`GalaxyTickAtLeast`+`SpawnHazard`) vs `Placed` 열거자(카드 헤더 전).
4. MVP 임시 엔딩(통치 백성 T3 ∨ 틱 6,000 → 연대기 최소형)과 연대기 이관 기준(#40 ⑦ + 상한 노브 1,500).
5. 격차 규칙을 Core의 술어 하나로 모으는 때(#107, Phase 4)와 아공간 침식 Hazard의 때(#90).

## 2026-10-18 개발자 결정 (1단계 브리핑 §9 — "권장대로")

docs/ 재편과 옛 문서 archive · 규칙 문장의 상태 표지(예정/현행) · CLAUDE.md ≤10KB대 · MVP = 통치자 단일 모드, ×10 기준 30~60분, 끝 = T3 ∨ 틱 6,000 · v0.8 예약 ①은 MVP 앞(A0)에서 한 번 · 실제 하늘(단계 5)은 Phase 4 뒤·Phase 5 전(B8), 자료 라이선스 확인만 지금 · 첫 10분은 모든 백성 공통 규칙 · 자동 일시정지 기본 켬 + 토글 · 가칭 식별자는 A0 헤더 전 일괄 확정 · 경제·연구 상수 9개 → DT_Rules(StarClassEnergy는 단계 5까지 C++), InfluencePerPopulation 1.0 · T2→T3 둘째 업적 + 전조 확률 1e-4 · 연대기 이관 #40 ⑦ + 상한 노브 · 커밋 규약(표에는 날짜, 접두어, 해시 커밋 폐지) · codex → docs/90_LORE, code_artifact → archive, directive·rulings → 기록 흡수 뒤 archive.
