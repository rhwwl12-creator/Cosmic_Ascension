# STATUS — 현재 단계

갱신: 2026-10-19 (A0 'v0.8 예약 ①' 끝 — 성역 4a·첫 생애 행 4b·ultracode 리뷰 1회). 세션 시작에 읽고 세션 끝에 이 파일만 갱신한다. 일지·리뷰 통계는 `docs/CHANGELOG.md`에.

## 지금

- **A0 'v0.8 예약 ①' — 끝(2026-10-19, `git log --grep 'A0-4'`).** 4a 성역: `sol`은 배열 끝의 레인 없는 노드(1.15 × 원판 반지름·각도 0), 첫 생애의 모성계는 `sys_0`, 다섯째 법 `Law.Hedge`, 거절 자리(`SettleBlockOf` 첫 사유·`SettleTargets`·`Place`·`Transfer`·`Destroy`·압력 0·`Install`·`BecomeWanderer`·`LoadGalaxy`), 표현은 `FCosmicKnownView::ShownByName`·`IsShown`. 4b 첫 생애 행 `arch_seeker`(찾는 백성, 첫 줄 "{HomeSystem}. 이 백성이 당신에게 맡겨졌다."). 증명: 4a 뒤 커맨드렛 20,000틱 CSV의 결정적인 칸이 A0 전과 같고 무주 성계만 +1(추첨 수 불변). 리뷰: 5건 중 1건 확인(설정 표 검사가 실패하지 못하던 4b의 도구 실수 — 고침), 4건 반박.
- **Phase 3**: 작업은 모두 끝. 기계 몫 종료 리뷰(규칙 1·14)는 통과했다(개발자 보고). 남은 것은 **손 확인(개발자: 가장 새 빌드로, 창에서 마우스만으로 셋째 칸, `stat fps`·`stat unit` 60fps — 배경 별의 GPU 몫은 `show InstancedStaticMeshes`로 끄고 켜서 본다; 성역 '태양계'가 원반 가장자리 — 첫 생애 모성계 쪽 바깥 — 에 이름과 별로만 보이고 고를 수 있는지도)** → CHANGELOG 한 줄 → Phase 3 종료 선언.
- **코드 사실**: 자동화 테스트 92종 통과(새 셋: `Sim.SanctuaryKept`·`Sim.SanctuaryRefusals`·`Sim.SaveSanctuary`). `State.Systems`는 생명 성계 + 성역(VS 61 — 테스트는 `CosmicTests::LivingSystems`). DT_Rules 노브 46(읽힘 28·예약 18), `ImmutableLaws` 5, `DT_Galaxies` 20행, 배속 상한 25.
- **기준값**: 커맨드렛 시드 42, 3,000틱 hash 2998664607, world 2405589164 (`Tools/baselines/seed42.json`; 20,000틱 hash 215423113, world 3260616143). `run_checks.ps1 -Tier full` 통과. 개발 세이브는 지웠다 — A0 전의 세이브는 로드 문이 거절한다.

## 다음 3작업 (`docs/50_ROADMAP.md` 0 → A)

1. **손 확인**(개발자) → 결과(fps·막힌 조작·성역의 보임)를 CHANGELOG에 한 줄 → Phase 3 종료 선언.
2. **Playable MVP(A) ①②**: 자동 일시정지(Severity 2·자기 성계 위기·카드, 기본 켬 + 토글) → `DT_Encounters` 스키마·중첩 구조체 검사 → 청원 카드 10장 + 카드 위젯. effort high(헤더·저장에 닿는 부분은 xhigh), ultracode 끔(단독) — 카드 텍스트 묶음 감사 때 켠다.
3. **MVP ③**: 첫 10분 보장 + #68·#71·#72의 기본값(먼저 커맨드렛으로 '#68만'·'#71만' 판을 잰다) + T2→T3 둘째 업적.

## 열린 결정 Top 4 (`docs/60_DECISIONS/OPEN_QUESTIONS.md`)

1. #68 안정도 회복·#71 AI 구호 정책·#72 전조 확률의 기본값 — MVP '세계 손질' 전('#68만'·'#71만' 판을 커맨드렛으로 먼저 잰다).
2. 첫 10분 보장 방식 — 모든 백성 공통 Standing 위기(+`GalaxyTickAtLeast`+`SpawnHazard`) vs `Placed` 열거자(카드 헤더 전).
3. MVP 임시 엔딩(통치 백성 T3 ∨ 틱 6,000 → 연대기 최소형)과 연대기 이관 기준(#40 ⑦ + 상한 노브 1,500).
4. 격차 규칙을 Core의 술어 하나로 모으는 때(#107, Phase 4)와 아공간 침식 Hazard의 때(#90).

## 2026-10-18 개발자 결정 (1단계 브리핑 §9 — "권장대로")

docs/ 재편과 옛 문서 archive · 규칙 문장의 상태 표지(예정/현행) · CLAUDE.md ≤10KB대 · MVP = 통치자 단일 모드, ×10 기준 30~60분, 끝 = T3 ∨ 틱 6,000 · v0.8 예약 ①은 MVP 앞(A0)에서 한 번 · 실제 하늘(단계 5)은 Phase 4 뒤·Phase 5 전(B8), 자료 라이선스 확인만 지금 · 첫 10분은 모든 백성 공통 규칙 · 자동 일시정지 기본 켬 + 토글 · 가칭 식별자는 A0 헤더 전 일괄 확정 · 경제·연구 상수 9개 → DT_Rules(StarClassEnergy는 단계 5까지 C++), InfluencePerPopulation 1.0 · T2→T3 둘째 업적 + 전조 확률 1e-4 · 연대기 이관 #40 ⑦ + 상한 노브 · 커밋 규약(표에는 날짜, 접두어, 해시 커밋 폐지) · codex → docs/90_LORE, code_artifact → archive, directive·rulings → 기록 흡수 뒤 archive.
