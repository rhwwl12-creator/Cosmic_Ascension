# [B-6b8] CHANGELOG work-log line - kept here, not written into docs/CHANGELOG.md

Why here: docs/CHANGELOG.md has no B6b section yet (its newest section is "2026-10-31~11-01 — B6a"), and the
precedent of this block is that the B6b work-log lines wait for [B-6b12] (scratchpad/b6b6/brief_final.md:419,
scratchpad/b6b7/brief_final.md:616 "CHANGELOG waits for [B-6b12]", scratchpad/b6b8b/brief_final.md:292,
B6a's lines were written by its document commit [B-6a7]). [B-6b1]..[B-6b7] wrote none. A lone [B-6b8] line under a
new section would break that order; the lead decides. If it goes in now, it belongs at the top, under a new heading
"## 2026-11-08~ — B6b 넷째·다섯째 칸과 경주" (the [B-6b12] document commit then adds the other lines above/below it).

The line (same placeholders as N section 3 / section 5):

- `[B-6b8]` 데이터 ③ 곁 행(헤더 §2.7·§2.8·§5.3·§8, 결정 20~22; 기준값 다섯과 해시 판 둘 비트 그대로): `DT_Events.event_era_endured`(견딘 영압의 시대 — 다스리는 백성의 견딘 셈 600에서 한 번, 국경을 맞댄 백성마다 경외 +0.2, 줄 셋; 12 → 13행)와 `DT_Encounters.enc_witness`('그늘의 가장자리' — 주장 곁 방랑자 카드, 정신 판정 3이 이기면 주인 백성의 경외 +0.1, '물러난다'는 효과 없음; 18 → 19행), 스키마의 증언 낱말 셋과 규칙 W·E(검사기 단위 테스트 95 → 96, 걷는 수 316 → 330), 방랑자 정책의 판정 없는 답의 셈과 race 줄의 증언 첫 틱·답의 갈래, 새 테스트 `Sim.EraEndured`·`Sim.WitnessCard`(새 파일 `Tests/CosmicSideRowTests.cpp` — 223 → 225종), `Data.Integrity`의 걸림줄 열 행. 묶음 B(556 프로세스, 짝 240/240 통과 — 다리는 칸마다 min(첫 셋째 칸 + 1, 첫 시대 줄, 첫 증언 카드)): ① 60/60 ② 14/60 = 0.233 [0.144, 0.354] ③ 2,209·46/46 = 1.000 [0.923, 1.000] ④ C 9,000; 넘김 판단 넷 모두 통과 — `[B-6b9]`에 넘길 깨진 판단 없음; [개발자]에게 느린 R의 ②(31/60, 구간이 통째로 35% 위)를 ④의 받아들임(C 9,000)과 함께; `[B-6b9]` 전에 AI 단계 µs가 기준을 넘은 까닭(한가한 기계에서 직렬 판 다시 재기부터); `enc_witness`가 300판 0 — 닿는 길은 `[B-6b9]`의 B′나 `[B-6b12]`; E가 모형의 짧은 꼴 — 노트 §0.2에 #168을 열었다. 리뷰: 발견 17(확인 10·일부 7·반박 0), 기록 위의 확인 리뷰 발견 6(모두 맞음 — 기록만). 변이: 새 `b6b8_` 열일곱과 옛 129 — 탐침 둘 밖 144 모두 깨짐(묶음 B 앞에서 — 리드가 바꾼 차례).
