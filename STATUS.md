# STATUS — 현재 단계

갱신: 2026-10-18 (문서 재편 커밋). 세션 시작에 읽고 세션 끝에 이 파일만 갱신한다. 일지·리뷰 통계는 `docs/CHANGELOG.md`에.

## 지금

- **Phase 3(은하 맵 뷰포트) 진행 중** — 본체(성계와 레인 그리기·카메라와 선택·상단 바와 성계 패널·로그 피드·승천 연출, 6번의 기계 몫, 정착 사유, 11번 시간 표시 토글)는 끝. 종료 조건의 사람 몫 **손 확인(개발자: 창에서 마우스만으로 셋째 칸, `stat fps` 60)** 대기. 꼬리 7(영적 시야)·8(앎의 표시)·9(우주 화면)·12(×25·명령 사유)와 종료 리뷰 10번 미착수.
- **설계 문서**: 2026-10-18 `docs/`로 재편(1단계 전수 조사 → 2단계 통폐합). 설계 결정은 v0.8까지 `docs/60_DECISIONS/DECISIONS_LOG.md`, 열린 물음은 `OPEN_QUESTIONS.md`(#번호는 옛 guideline §16 그대로).
- **코드 사실(HEAD 기준)**: 자동화 테스트 76종, GameplayTag 140, DT_Rules 노브 44(런타임이 읽는 것 26 — 계승 6·도약 3·함대 4·귀환 3·ImmutableLaws·Vigil은 예약), 데이터 행 42(이벤트 11·위기 5·프로젝트 5·업적 2·아키타입 2·초월자 1·로그 템플릿 10·은하 6). 플레이어 명령은 `StartMegaproject`(정착 포함) 하나. `TickDiplomacy`·`TickConflict` 빈 본문, 엔딩·자동 일시정지·자동 저장(게임 코드 호출) 없음, `SetTimeScale`은 10으로 clamp.
- **기준값**: 커맨드렛 시드 42(성계 60·백성 12) 세계 해시와 Phase 2 실측(반란 62%·주장 만료 18%·첫 600틱 말해진 줄 중앙값 9)은 `docs/CHANGELOG.md`의 Phase 2 종료 리뷰 기록. 쿠킹 빌드의 해시 일치는 미확인.

## 다음 3작업 (`docs/50_ROADMAP.md` 0 → A0)

1. **손 확인**(개발자) → 결과(fps·막힌 조작)를 CHANGELOG에 한 줄 → Phase 3 종료 선언 → 종료 리뷰 10번(ultracode 1회, Core Logic이 UI/Actor를 include하지 않는지).
2. **정리**: `Tools/run_checks`(데이터 검사 → 빌드 → 테스트 2회 → 스모크 → 커맨드렛 기준값 비교)·`Tools/check_layers.py`(include 방향·태그 리터럴·`FMath::Rand` grep), `FCosmicEvalContext`를 `Sim/CosmicEvalContext.h`로 분리, 시드 42 기준값을 `Tools/baselines/`에 파일로. effort medium, 단독.
3. **A0 'v0.8 예약 ①'**: 성역 `sol`(배열 끝)·`sys_0`·`Law.Hedge`·`IsSanctuary` + 첫 생애 행 뼈대 `arch_seeker`(행 이름·태그·첫 줄·두 목소리). 가칭 식별자 일괄 확정 → 헤더 승인 → 구현 → 재측정 테스트 ~40곳 → ultracode 리뷰 1회(저장·결정론). effort xhigh. 그 뒤 **Playable MVP(A)**: 자동 일시정지 → 청원 카드 10장+카드 위젯 → 첫 10분 보장·#68/#71/#72 → 임시 엔딩·연대기 최소형 → 저장 최소형 → 지표 4·온보딩 → Win64 패키징.

## 열린 결정 Top 5 (`docs/60_DECISIONS/OPEN_QUESTIONS.md`)

1. 가칭 식별자 확정 — `arch_seeker`·`Trait.Arch.Seeker`·`Law.Hedge`·`IsSanctuary`·`ReceiveCalling`·`enc_sanctuary_bound`·`event_word_arrived`(A0 헤더 전; 세이브에 남아 배포 뒤 불변).
2. #68 안정도 회복·#71 AI 구호 정책·#72 전조 확률의 기본값 — MVP '세계 손질' 전('#68만'·'#71만' 판을 커맨드렛으로 먼저 잰다).
3. 첫 10분 보장 방식 — 모든 백성 공통 Standing 위기(+`GalaxyTickAtLeast`+`SpawnHazard`) vs `Placed` 열거자(카드 헤더 전).
4. MVP 임시 엔딩(통치 백성 T3 ∨ 틱 6,000 → 연대기 최소형)과 연대기 이관 기준(#40 ⑦ + 상한 노브 1,500).
5. 아공간 침식 Hazard의 때(#90)와 T2→T3 둘째 업적 `feat_three_worlds`(정착 3 ∧ 공학 0.5)의 채택.

## 2026-10-18 개발자 결정 (1단계 브리핑 §9 — "권장대로")

docs/ 재편과 옛 문서 archive · 규칙 문장의 상태 표지(예정/현행) · CLAUDE.md ≤10KB · MVP = 통치자 단일 모드, ×10 기준 30~60분, 끝 = T3 ∨ 틱 6,000 · v0.8 예약 ①은 MVP 앞(A0)에서 한 번 · 실제 하늘(단계 5)은 Phase 4 뒤·Phase 5 전(B8), 자료 라이선스 확인만 지금 · 첫 10분은 모든 백성 공통 규칙 · 자동 일시정지 기본 켬 + 토글 · 가칭 식별자는 A0 헤더 전 일괄 확정 · 경제·연구 상수 9개 → DT_Rules(StarClassEnergy는 단계 5까지 C++), InfluencePerPopulation 1.0 · T2→T3 둘째 업적 + 전조 확률 1e-4 · 연대기 이관 #40 ⑦ + 상한 노브 · 커밋 규약(표에는 날짜, 접두어, 해시 커밋 폐지) · codex → docs/90_LORE, code_artifact → archive, directive·rulings → 기록 흡수 뒤 archive.
