# B6b 헤더 — 넷째·다섯째 칸과 경주 (절멸 위기와 방주·조약·은하 법칙·결박·넷째 칸의 제동과 셋째 칸 위의 지연·견딘 영압의 시대·방랑자의 증언·격차 술어·다섯째는 금빛 없이·미리 적은 규칙으로 정하는 시계)

상태: **승인 (2026-11-08)** — [개발자] 열넷의 답: 결정 22는 대안 ㉮(설계 그대로 이웃 경외 +0.2), 나머지 열셋은 권장. 만든 길: 조사 워크플로 ①(이 세션의 여섯 지도와 그 종합, 같은 HEAD를 따로 읽은 다른 세션의 여덟 보고와 그 종합) → 개발자의 답 넷(A1~A4, 2026-11-08 — §0) → 조사 워크플로 ②(답 안에서 다시 본 여섯 관점과 그 종합 — 사실 마흔넷·결정 마흔·충돌 열셋·커밋 표) → 이 초안 → ultracode 적대 리뷰(관점 다섯 — 발견 57, 검증: 확인 41·일부 16·반박 0) → 리드의 고침(57건 모두 — 일부 16은 검증자의 고침대로, 기록은 노트 §4) → 고친 뒤 확인(읽기만 — 남은 고침 열과 빠진 것 여덟을 리드가 들였다, 노트 §4). → 개발자의 답 열넷(2026-11-08 — §10의 머리). 남은 길: §8의 차례로 `[B-6b1]`부터. 짝 노트 `B6b_upper_rungs_and_the_race_notes.md` — 지금 코드의 줄마다 근거(노트 §0), 잰 값(§0.1), 열린 물음과 넘겨받은 것(§0.2·§0.3), 문서의 어긋남(§1), 미리 셈(§2), 리뷰의 기록(§4); 측정과 구현 기록은 커밋마다 노트 §3·§5에 더한다.

근거: `docs/50_ROADMAP.md:12`(B 행의 B6b 칸과 'B6a가 수와 함께 넘긴 것', 끝 칸의 '엔딩 1회 콘솔 없이'), B6a 헤더 §0 'B6a 밖'(`:33`·`:34`)·결정 1·2-나·2-다·21(`:387`·`:390`·`:391`·`:424`)·'B6b 헤더로 넘기는 결정'(`:426`), `pillars.md:105`·`:106`·`:108`, `crisis_and_transcendents.md:24`·`:115`·`:121`·`:134`·`:170`, `systems.md:169`, `docs/21:1574-1580`, `docs/50:231`·`:292`, `STATUS.md:31`, `WORKPLAN.md:61`·`:65`, `OPEN_QUESTIONS`(노트 §0.2), `CLAUDE.md` 불변 규칙 1~4·6~9·11~15. 조사 원문은 저장소 밖(세션의 스크래치패드)이라 인용하지 않는다 — 사실은 모두 HEAD `5455e56`의 줄로 단다. 경로: 코드는 `Source/Cosmic_Ascension/` 아래를 줄여 `Sim/…`로, 문서는 `docs/20:89`처럼, 설계 본문은 파일 이름으로, 앞 헤더는 'B6a 헤더 `:387`'로 적는다.

## 0. 범위와 전제

- **이 헤더의 낱말**(새 낱말을 만들지 않고 설계의 낱말을 풀어 둔다):
  - **경주**: 다스리는 백성과 다스려지지 않는 백성 가운데 누가 먼저 다섯째 칸에 서는가. 먼저 선 쪽이 판의 끝을 적는다 — 집단 승천(`Ending.RuledPeople`) 또는 다른 질서(`Ending.OtherPeople`). 화면에는 '경주'를 내지 않는다(B6a 결정 6-마).
  - **절멸 위기**: `Hazard.Extinction` — 250틱, 만료가 그 백성의 성계를 모두 놓게 하는 위기(`crisis_and_transcendents.md:24`). 지금은 태그뿐이다(`:30`).
  - **방주**: 절멸 위기의 답인 대응 프로젝트(결정 6). '방주'는 구출의 예약어라 이 행에만 쓴다.
  - **은하 법칙**: 범위가 고향 은하로 넓어진 칙령(`systems.md:169`). 화면에서는 '칙령'이다(결정 9).
  - **결박**: 타락 초월자 하나를 묶는 일 — 설계의 'T4 봉인 Feat = 결박: 그 초월자의 영향권 동결, 신규 주장 0'(`crisis_and_transcendents.md:121`). 여기서는 넷째 칸의 건설 프로젝트이고 그 완성이 `bSealed`·`SealedTick`을 처음 쓴다(결정 12). 글에서는 행위자 없는 수동('묶였다')뿐이다.
  - **제동**: AI 백성의 T3→T4를 고향 은하의 넷째 칸 이상 백성 수로 막는 상한 `AIFourthRungCap`(`docs/21:1574-1580`). '자리'라 부르지 않는다.
  - **셋째 칸 위의 지연**: AI의 셋째 칸 위 건설에만 곱하는 진척 배율(결정 17).
  - **견딘 영압의 시대**: #156 — 다스리는 백성이 수도를 잃지 않고 타락의 영압을 오래 견디면 그 백성이 한 시대의 이름을 부르기 시작하는 줄. 사람의 칭호가 아니다(`docs/70:192`).
  - **미리 적은 규칙**: 경주를 재기 전에 이 헤더가 적어 두는 시계의 셈법(A4·결정 35). **짝**·**첫 측정**·**한 번만 센다**는 B5·B6a의 낱말 그대로.
- **개발자의 답 넷**(2026-11-08 — 이 헤더는 그 안에서 쓴다; 아래는 리드가 옮긴 요지이고 원문은 날짜와 함께 `DECISIONS_LOG`의 B6b 절에 적는다, 결정 39):
  - **A1 범위** — 고른 답 '칸에 적힌 그대로 전부': `docs/50:12`의 B6b 칸에 적힌 것 전부를 **한 블록**으로 한다(물은 대안 — 장치 없는 #156·칙령 범위의 넓힘·#141을 떼어 줄이기, B6b1(업적 행·AI·시계)·B6b2(칸 공명·#141·#156)로 나누기 — 은 받지 않았다).
  - **A2 행** — 고른 답 '설계 표 그대로'(물은 대안 — 칸마다 거대 사업 하나와 보유 성계 조건의 단순한 행): 넷째·다섯째 칸의 통치자 행은 **설계 업적 표에 적힌 그대로** — T3→T4 '절멸 위기 종족을 차원 방주로 이주(프로젝트 + 외교 조약)', T4→T5 '은하 법칙 3개를 N틱 이상 유지 + 타락 초월자 1체 봉인'. 그 행이 봉인을 부르므로 결박이 B6b에 든다; T5 문턱과 두 길이 Phase 6+ 그대로라는 것은 질문에 없던 리드의 읽기다(설계 표의 T4→T5 칸은 두 길의 덧붙임 — 받는 길의 통치권 위임·위기 종족 구출, 찬탈의 '봉인 대신 인과율 흡수' — 을 적지만, 길은 다섯째 칸에 선 뒤의 문이 가르고 업적 행은 `Way.*`를 주지 않아(`pillars.md:91`) 다섯째 칸 전에는 어느 길도 없다 — 그래서 B6b는 길 없는 기본 행만 짓는다. '봉인 대신'은 결정 14의 잠김을 풀 설계의 길이라 이 읽기는 결정 14와 함께 [개발자]가 확인한다; 결정 39가 `DECISIONS_LOG`에 적는다).
  - **A3 칸** — 고른 답 '요구를 떼고 칸은 따로(권장)'(물은 대안 — 칸 공명을 들인다, 요구를 남기고 AI는 따로 읽기): 통치자 T4→T5의 '개인 칸 ≥ 3'을 버린다(#1). 백성의 칸과 그릇의 개인 칸은 따로 가고 칸 공명은 없다; 스키마의 금지(`Tools/data/schema.json:817-833`·`:896-900`)와 `-ExaltAsBefore`는 뜻이 그대로다.
  - **A4 시계** — 고른 답 '재고 나서 정함(권장)'(물은 대안 — 6,000 그대로, 지금 18,000): 경주를 먼저 재고, 시계의 값은 **미리 적은 규칙**으로 정해 그 값만의 커밋에 넣는다. 6,000에 끝난 B6a 세이브는 끝난 채로 둔다(`SaveVersion` 1 그대로, 이주 없음).
- **B6b에 드는 것**(하나의 목록 — `docs/50:12`의 B6b 칸과 `STATUS.md`는 이 절을 가리킨다):
  1. **넷째 칸으로 오르는 행**: 절멸 위기 행과 그 만료 훅(백성의 성계를 모두 놓게 하고, 통치 중이면 통치자가 죽는다 — 죽음의 둘째 출처), 방주, 조약을 읽는 조건, T3→T4 업적 행. 결정 2·4~8.
  2. **다섯째 칸으로 오르는 행**: 은하 법칙(칙령 범위의 넓힘, #160 — 넷째 칸 칙령 행 셋 이상과 범위 칸), 법칙을 지킨 틱의 저장 필드, 결박 프로젝트, T4→T5 업적 행. 결정 9~15.
  3. **셋째 칸 위의 AI**: 행이 부를 때만 하는 좁은 걸음 둘(조약·법칙 — B7에서 떼어 온다)과 기존 `Choices`로 짓는 방주·결박; 제동과 셋째 칸 위만의 지연(#161); 넷째 칸이 거저 얻는 것은 받아들이고 잰다. 결정 1·11·16~18.
  4. **견딘 영압의 시대**(#156): 저장 필드·조건·상시 이벤트 행 하나씩, 설계 그대로 이웃 경외 +0.2. 결정 22(답 ㉮).
  5. **방랑자의 증언**(첫째 칸 방랑자의 카드 `enc_witness` — #141을 가른다); 피난(#138)은 절멸 위기의 답으로 닫는다. 결정 20·21.
  6. **보는 것과 부르는 이름**: 격차 규칙의 Core 술어(#107), 즉위의 문의 격차(#122), 이름이 새는 줄, 섭정 아래 옛 백성(#121 — 지금대로). 결정 19·41.
  7. **지킴의 지연 상한**(#132)과 **신정론 예산의 판단 규칙**(#142). 결정 23·24.
  8. **표현**: 넷째 칸의 연출, 다섯째는 금빛 없이, 규칙 줄과 경고, 다음 칸 툴팁(#104), 먼 줄과 까닭 줄(#162), 비문, [백성] 탭의 '칸 N'. 결정 25~31.
  9. **글**: 새 행 모두의 글, 결박의 글과 고리(#98 ②; #163의 결박 몫), 새 낱말 목록과 스키마 규칙, ultracode 글 감사(`docs/70:129`). 결정 32·33.
  10. **측정**: `race:` 줄과 시험 스위치, 묶음 A·B·C, 미리 적은 판단(#161·'도약이 일으킨 다른 질서 < 15%'·#136·#129·#154·B1 지킴). 결정 34·36.
  11. **시계의 마지막 값**(#2): 미리 적은 규칙으로, 그 값만의 커밋 — 그리고 벽시계의 새 잣대(B6a 결정 2-라, `docs/00_VISION.md:32`). 결정 35·42.
  12. **문서와 규칙 파일**: VS 정의의 수량(#160), `DECISIONS_LOG`, `CLAUDE.md` 규칙 4·13·14·15. 결정 38~40(커밋 나눔은 결정 37·§8).
- **B6b 밖**(어디로 — 표는 노트 §0.2·§0.3):
  - **Phase 4**: 아공간 침식(#90 그대로 — 결정 3 [개발자]), 언약 특성의 재발 면역(`crisis_and_transcendents.md:255`), 교단, 방랑자 업적 행(`schema.json:785-787`)과 #141에서 가른 대면·정신의 닻·절멸의 W 칸 '피난 호위 조우 체인'(#138을 닫으며 — #141의 피난 호위와 같은 것; 방랑자의 칸이 오르는 블록, #164), 헌사 '결박'(`DT_Dedications`).
  - **Phase 6+**: T5 문턱·두 길·받는 길의 덧붙임(통치권 위임·위기 종족 구출)·찬탈의 인과율 흡수·후원 카드·단독 승천의 길, #163의 나머지, 넷째 칸 물리 화면의 오라.
  - **B7**: 프로필이 정하는 외교(AI끼리·AI가 제안하고 끊는 조약), AI의 칙령 일반, 영향권 안 함대 소모(#132의 둘째 몫 — 결정 23), #155·#158. **B8**: 울타리의 증언(#159). **주인 없음 그대로**: 조공(#137 — #156의 '조공 없이'는 그때까지 빈 조건).
  - **열린 [개발자] 물음 그대로**: 넷째 칸 백성을 떠난 그릇의 보는 칸(#96·#100·#108 — 보는 칸에 그 생애의 바닥을 두는 안은 시야의 규칙 `docs/20:127`을 바꾼다; '추가 이야기와 보완').
- **답과 승인된 결정이 부딪는 곳**(열셋 — 결정들은 이 번호를 '§0 부딪는 곳 N'으로 부른다; 목록과 풀이는 노트 §0.5): 1 A2 ↔ B6a 결정 1(결박), 2 A2 ↔ AI의 규칙, 3 A3 ↔ #141의 주인, 4 A3 ↔ 낡은 글, 5 절멸 ↔ B5 결정 6-가, 6 A2 ↔ #138, 7 A2 ↔ 로드맵의 Phase 4, 8 A4 ↔ 6,000을 박은 테스트, 9 A1 ↔ 크기, 10 결박 ↔ #73, 11 넓힘 ↔ 신정론, 12 #156 ↔ 그 주의 셋, 13 넷째 칸의 대면 조우.
- **노브**: B6b는 노브를 셋 더하고(`GalaxyLawsToHold` 3, `AIUpperRungProgressMultiplier` 1, `FleetDelayCountdownShare` 0 — 장치 커밋에서는 지금의 상한 그대로) 넷을 처음 움직인다 — 아무도 읽지 않던 `AIFourthRungCap` 2를 판정이 읽고, `AIUpperRungProgressMultiplier` 1을 `[B-6b7]`이 시작값 0.375로, B6a가 넘긴 지킴의 상한(#132 — B6a 결정 12)을 `[B-6b8b]`가 몫 0.5로, `ProvisionalEndTick` 6,000을 `[B-6b10]`이 규칙의 값으로 옮긴다. 그 밖의 값은 판단이 부를 때만 값만의 커밋(`[B-6b9]`)에서, 다시 잰 뒤의 노브는 따로의 커밋(`[B-6b11]`)에서 바꾼다.
- **바뀌는 것**: 저장 필드 둘(`FCivilizationState`의 `LawsHeldTicks`·`PressureEnduredTicks` — 필드와 문서가 먼저인 커밋 `[B-6b1]`, `SaveVersion` 1 그대로 — §2.1)이 반사로 두 해시에 들어 다섯 줄과 해시 판 둘의 두 해시를 모두 옮기고, 커맨드렛 CSV의 결정적 칸은 `[B-6a7]`과 같다(`[B-3a]`·`[B-6a2]`의 꼴). 장치·표현·글의 커밋은 비트 그대로다. **입력 없는 세계가 처음 옮는다**: B6a까지는 그 잰 수가 그대로였으나(B6a 헤더 `:38`), `[B-6b6]`의 절멸 행이 셋째 칸 이상 백성의 성계에서 발생 몫을 얻고 방주와 조약 걸음이 AI를 넷째 칸으로 올리므로 다섯 줄과 해시 판 둘이 모두 옮는다(시드 42의 첫 AI 셋째 칸은 3,000 줄 안). 정책 없는 판에도 절멸로 사라지는 백성·넷째 칸 AI와 제동·다른 질서의 끝이 처음 서고, 정책 판에는 절멸의 만료로 죽는 통치자와 집단 승천이 처음 든다. 시계 커밋은 규칙의 값에 닿는 줄만 옮긴다 — 3,000 줄과 해시 판 둘은 그대로(§5.4).
- **지키는 것**(`CLAUDE.md` 규칙 11·12 — 글의 규칙은 §3.5): 거룩한 질서는 엔티티·화자·수치·엔딩 조건이 아니다 — 결박은 행위자 없는 수동('…|hpp(이,가) 묶였다' — `Tools/data/ST_CA.csv:52`의 `Sight.Bound`과 같은 꼴)으로만 말하고 '누가 봉인했다'·승리·조롱이 없으며, 결박 프로젝트를 완성한 백성의 일은 3인칭의 사실로만 말한다(결정 32). 다섯째 칸의 오름은 '얻은 오름'의 그림(금빛 빛무리·종소리 — 메시아 신호 11)을 쓰지 않고 칭호·'신이 되었다'·'영겁'·'왕 노릇'이 글에 오지 않는다(결정 26·33 — T5의 이름은 #163). 절멸의 만료로 죽는 통치자는 백성을 위해 '남은' 희생으로 읽히지 않는다(결정 5). 사람의 수는 없다 — 절멸은 '백성이 사라졌다'는 사실이다. 영혼·구원·멸망을 세는 칸이 없다 — 새 저장 필드는 틱의 수 둘뿐. 칸은 다섯 그대로, 초월자는 판에 교리마다 하나(VS는 `trans_silence` 하나 — 그래서 결박은 판에 한 번, 결정 14).
- **판의 끝과 B6b가 보이는 곳**: 집단 승천과 다른 질서가 처음 판 안에 닿는다. 마일스톤 B의 '엔딩 1회 콘솔 없이'(B6a 결정 21)는 `[B-6b10]` 뒤의 손 확인이 본다(§9); B6a의 손 확인 ①·③·⑨는 시계 6,000과 `[B-6a6]`의 세이브에 기대므로 `[B-6b6]` 전에 B6a의 빌드로 한다.

## 1. 기대는 사실 (2026-11-08, HEAD `5455e56`)

### 1.1 지금 코드

지금 코드의 요약(판정과 행·끝·제동과 AI 단계·건설·위기·백성을 놓음과 통치자의 죽음·조약과 칙령·결박·넷째 칸이 거저 얻는 것·격차와 새는 이름·#156과 #141의 재료·표현·저장과 규칙 파일)과 줄마다의 근거는 노트 §0(HEAD `5455e56`).

### 1.2 잰 값 (B6a 끝의 세계 — 표는 노트 §0.1)

표와 원천은 노트 §0.1(B6a 노트 §3 묶음 1의 960판). 이 헤더가 기대는 수만(`docs/50:12`): 첫 AI 셋째 칸 중앙값 2,174.5, 다스리는 셋째 칸 − 첫 AI 셋째 칸 중앙값 +92틱(경주의 입력); 셋째 칸 뒤 계절은 통치자 정책 판 3,634.5·`-Focus=0` 1,616·방랑자 2,959틱; 끝 전 1인칭 주장 줄 7/30(#136 문 10/30 아래); 지킴이 AI 각성에 더한 지연의 몫 64.2%(띠 10~50% 밖 — #132); 셋째 칸 뒤 신정론 예산은 잰 13(30건 문 아래 — #142); B1 지킴 40/60, 중앙값 1,350(잡음으로 받음); 끝 전 알아챔 0/120.

### 1.3 B6a가 넘긴 것 (표는 노트 §0.3)

`docs/50:12` B6b 칸의 'B6a가 수와 함께 넘긴 것'과 B6a 헤더 `:426`의 결정 목록(B3~B5가 넘긴 것은 B6a가 이미 갈랐다). B6b가 반드시 하는 것: #132의 노브(결정 23), B1 지킴의 짝 판단(결정 36), #136·#142의 시계 뒤 판단(결정 35·24).

### 1.4 열린 물음 가운데 B6b가 닫거나 가르거나 옮길 것 (표는 노트 §0.2)

닫음: #1(A3)·#2(결정 35)·#98 ②(결정 32)·#107·#121·#122(결정 41)·#132(결정 23)·#138(결정 21)·#156(결정 22)·#160(결정 9·40)·#161(결정 16·17·35)·#162(결정 29). 가름: #141(증언·물러남은 B6b, 대면·정신의 닻은 방랑자의 칸이 오르는 블록 — 결정 20). 잰 뒤 닫거나 수와 함께 옮김: #129·#136·#142·#154(결정 24·35). 걸림만 고침: #90(Phase 4 그대로 — 결정 3)·#104(결정 28)·#163(결박의 몫만 — 결정 32). 새 행은 #164부터.

### 1.5 문서끼리·문서와 코드가 어긋난 것 (표는 노트 §1)

셋으로 나눈 표(지금 어긋남 1~21·B6b가 거짓으로 만드는 문장 22~43·앞 헤더의 낡은 줄)는 노트 §1 — 그 머리의 요약도 거기. 고침은 각 결정과 §7.

## 2. 장치

### 2.1 저장 필드 — 셈 둘 (`[B-6b1]` — 필드와 문서가 먼저인 커밋; 결정 10·22)

```cpp
// Core/GalaxyState.h — FCivilizationState, FallenSphereTicks(:326) 바로 뒤에 멤버 둘 (같은 꼴의 셈 LowStabilityTicks :319와 한 무리).
// Consecutive ticks at the end of whose economy stage the people stood on the fourth rung or above and held at least
// DT_Rules.GalaxyLawsToHold edicts of home-galaxy scope (B6b header section 2.4). Read by CivLawsHeldTicksAtLeast within
// the life being judged. 0 below the fourth rung.
UPROPERTY(BlueprintReadOnly)
int32 LawsHeldTicks = 0;

// Consecutive ticks at the end of whose economy stage the people took the drain while its capital lay in no unsealed
// fallen sphere (B6b header section 2.7, #156); MoveCapital starts it over. Read by CivPressureEnduredTicksReached.
UPROPERTY(BlueprintReadOnly)
int32 PressureEnduredTicks = 0;
```

- **셈인 까닭**: 두 행이 묻는 것은 잇단 틱의 수이고 같은 꼴의 셈이 이미 둘 있다(경제 단계 끝 — `Sim/GalaxySimulationSubsystem.cpp:1881-1889`). 칙령마다의 제정 틱·수도를 잃은 틱·봉인한 백성(B6a 헤더 `:34`의 '주인 필드')·`EpithetRow`의 쓰기는 두지 않는다(결정 10·12·22).
- **얼린 기본값 0**(규칙 7): `FCivilizationState`는 `Civs` 배열의 원소라 새 세이브에는 통째로 적히고 그 칸이 없는 옛 세이브는 초기값으로 읽힌다(`Core/GalaxyState.h:5-11`). 법칙 틱의 0은 정확하다(넷째 칸의 백성이 든 세이브는 없다). 견딘 틱은 로드에서 새로 센다 — 그 전부터 견디던 백성은 창을 다시 기다린다(시험 세이브뿐 — B6a 헤더 §2.10의 까닭; 받아들인다).
- **두 해시에 든다**: 해시는 리플렉션으로 모든 `UPROPERTY`를 걷고(`Sim/GalaxySimulationSubsystem.cpp:1505-1520`) 둘은 세계의 사실이라 빼는 목록(`:1509-1515`)과 규칙 6이 그대로다. 백성마다 세고, 앎을 읽지 않고, 뽑지 않는다.
- **`SaveVersion` 1 그대로**(`Core/GalaxyState.h:828`·`:834`; 기본값이 있는 필드는 호환 — `:5-6`). 이주 없음. 6,000에 끝난 B6a 세이브는 끝난 채 열린다(A4 — 시계는 `[B-6b10]`).
- **`[B-6b1]`의 몫**(결정 37): 두 필드, **견딘 틱의 셈과 `MoveCapital`의 되돌림**(첫 칸부터 바로 세므로 해시를 옮기는 이 커밋에 들어야 장치 커밋이 비트 그대로다), 먼저 고치는 문서(`docs/20`·`docs/21` §7.2(`FCivilizationState` 사본의 두 필드 — `:249` 곁)·§7.3·§7.4·§7.5(`:1616` — `SinceTick`을 읽는 조건에 `CivLawsHeldTicksAtLeast`: `Min(LawsHeldTicks, Tick − SinceTick)`, 플레이어 업적 판정만 `BornTick`, AI 0)·`CLAUDE.md`의 옮김 — 결정 38). **법칙 틱의 셈은 `[B-6b2]`** — 읽는 범위 칸과 노브 `GalaxyLawsToHold`가 거기서 오고, 넷째 칸의 백성이 없는 동안 0이라 비트 그대로다(§8).
- **기준값과 테스트**: §5.4의 `[B-6b1]` 행, §6의 `[B-6b1]` 항목.

### 2.2 절멸 위기와 그 만료·통치자의 죽음 (`[B-6b2]` 장치, `[B-6b6]` 행 — 결정 2~5)

**행**(결정 4): `DT_Hazards`의 새 `Draw` 행 — `Hazard.Extinction`, '절멸 위기', 카운트다운 250(`crisis_and_transcendents.md:24`), `SpawnConditions` = `CivTierAtLeast 3`(주인), 확률 c는 노트 §2의 미리 셈. 둘째 칸의 짝(`crisis_pre_supernova` → 답 `project_stabilize` → `feat_supernova_stabilized`)과 같은 꼴이다.
- **이름 `crisis_species_extinction`**(마지막 이름 — `crisis_pre_supernova` 뒤로 정렬되는 까닭은 §3.1): 절멸이 처음 놓이기 전까지 세계가 같다.
- `OnExpireEffects`·`OnResolveEffects`는 비운다(놓음과 죽음은 아래 코드 — 데이터의 `TransferSystem`은 만료에 서지 못한다, `docs/21:756`; 언약의 면역 '다시는'은 Phase 4). `RulerResolutions` = `[project_ark]`(§2.3); 방랑자의 답(피난 호위 조우 체인)은 방랑자의 칸이 오르는 블록(#164 — 결정 21).
- **누구의 절멸인가**(결정 2 [개발자]): 그 백성 자신의 것 — '이주'는 상태가 옮기지 않고(`Civs` 그대로) 글이 '방주로 위기를 건넜다'로 말한다.
- **아공간 침식은 들이지 않는다**(결정 3 [개발자] — #90은 Phase 4 위기 헤더에 그대로).

**만료 훅**(결정 5 [개발자] — `ExpireHazard`의 태그 코드, `docs/50:198`의 문장 그대로):
1. **마지막 성계의 문이 먼저**: 절멸은 늘 마지막 정착된 성계까지 가져가므로 `bTakesSystem`(`:2493`)이 이 태그도 덮고, 성계 수와 상관없이 남은 자의 은혜(`event_grace_remnant` — 판에 한 번, 마지막 성계를 잃기 직전에 나서 그 상실 전체를 막는 행, `crisis_and_transcendents.md:261`)를 묻는다 — `GraceKeepsLastSystem`의 첫 물음(`:2681`)을 이 태그에는 건너뛴다. 행이 나가면 성계·생애·기록 줄 무엇도 잃지 않고, 그 백성의 다른 절멸도 결과 없이 끝난다(은혜는 성계 하나가 아니라 백성의 절멸에 답한다 — 같은 패스의 둘째 절멸이 은혜 바로 뒤에 그 백성을 앗지 않게, `[B-6b2b]`의 구현 리뷰; 그 절멸에 짓던 방주도 그 칸과 함께 끝나고 낸 값은 돌아오지 않는다 — 다스리는 백성의 넷째 칸은 다음 절멸을 기다린다). 은혜는 백성을 가리지 않는다(조건은 `FlagNotSet Flag.Grace.Spent` 하나 — `Tools/data/DT_Events.json:245`) — 셋째 칸 이상 AI가 열한 배쯤 자주 맞으므로 판의 은혜는 대개 AI의 절멸이 먼저 쓰고, 다스리는 백성의 절멸·초신성에는 남지 않을 수 있다(`race:` 줄이 쓴 백성과 틱을 센다). 은혜 행의 변형 둘은 마지막 성계 하나의 길에서 쓰였다 — `[B-6b5]`가 두 길에서 모두 참인 글로 고친다(변형 수 그대로, 비트 그대로; `docs/70:217`): 둘째 '무너질 것이 무너지지 않았다. {Civ}의 백성이 남겨졌다.', 셋째 '{Civ}의 땅 {System}|hpp(이,가) 지켜졌다. 그것은 누구의 공로로도 기록되지 않았다.'
2. 효과(비었다)와 `FollowUpKnowing`은 지금 차례.
3. **`CosmicHazards::LetPeopleGo(Ctx, OwnerIndex)`** — 새 단일 창구(규칙 13), 문 뒤의 단계 코드만 부른다: 그 백성의 모든 성계를 아무에게도 주지 않는다 — 주인 없음, 두 프로젝트 칸과 위기 칸 비움(다른 성계의 위기는 결과 없이 끝나고, 같은 패스의 그 만료는 첫 물음 `:2443-2446`에서 돌아간다 — 두 수는 이미 뽑혔다), 함대 목표 걷음(`CosmicConflict::LiftTargetsOn`), 완성 기록은 남김. `Transfer`는 마지막 정착된 성계를 거절하고(`Sim/CosmicHazards.h:78-94`) `LetSettlementsGo`는 그것이 없을 때만 움직인다(`:44-52`). 줄·알림·추첨 없음.
4. **`KillPlayer`**(`Death.Hazard`, `SourceRow` = 그 행)를 `Ctx.SystemIndex = INDEX_NONE`·`CivIndex` = 주인으로 `Apply`에 — `VesselAtRiskOf`가 이미 '성계 없는 주제는 그 백성의 통치자에게 닿는다(그 절멸)'이고(`Sim/CosmicHazards.h:212-220`) 출생 보호(태어난 뒤 301틱의 무시)도 거기서 가린다.
5. **이어 `DeposeRuler`**(`Reign.Deposed`) — 초신성 붕괴의 꼴(`:2560-2572`). 먼저 적힌 것이 이긴다: 출생 보호가 가린 죽음은 자리를 차지하지 않아 폐위가 적힌다 — 없으면 건넘 단계가 다스릴 것 없는 그 백성을 늘 남겨 둔다(`docs/50:198`).
6. **기록 줄**: 새 `Log.Record` 행 `log_record_extinct`(§3.1), 만료 줄 바로 뒤(`:2609-2621` — B5 결정 11의 꼴, 말하기 한 수).
7. 그 뒤는 그대로 — `TryPlaceClaim`은 정착되지 않은 성계라 아무것도 놓지 않는다(수는 이미 뽑혔다).

- **사라짐**: 건넘 단계가 지운다 — AI 백성은 그 틱, 다스리는 백성은 다음 틱(`Sim/GalaxySimulationSubsystem.cpp:2902-2959`). 그 사이 AI 단계는 정착 수를 뽑지 않는다(`MaySettle`은 정착된 성계를 묻는다 — `Sim/CosmicMegaprojects.cpp:391-402`). `ForgetCiv`가 짝의 조약도 지운다(§2.3).
- **새 추첨 지점 없음**: 만료의 두 수(`:2252-2257`) 그대로. 열째 단계의 통치자 죽음은 지금의 수 셋(섭정 프로필·이름·도약 — `Player/PlayerRoleSubsystem.h:91`) — 게임에서 통치자가 죽는 것은 처음이다(지금까지는 `-DeathAt`과 테스트).
- **둘째 죽음의 출처**(결정 5): 설계가 정한 예외 하나(`crisis_and_transcendents.md:142`·`:170`). B5의 데이터 핀(`Tests/CosmicDataTests.cpp:2418-2427` — 데이터의 `KillPlayer`만 걷는다)은 그대로, 그 핀이 요구한 `DECISIONS_LOG` 줄을 적는다. 새 단일 답 `CosmicHazards::KillsTheRulerOf(Tag)`(지금 `Hazard.Extinction` 하나)를 훅·패널의 통치자 경고(§4 — 새 자동 정지는 없다)·새 테스트(§6)가 읽는다. `KillsAVesselOf`(`:222-228`)는 그대로(새 그릇은 방랑자로 서서 닿지 않는다). 함께: 스테이지 시계의 기록(`:2486-2490`)은 이 태그에서 훅의 주제로 묻고, `CosmicTests::WithoutDeath`의 주석(§5.1), `Death.Hazard`의 설명(`Config/DefaultGameplayTags.ini:67`)을 고친다.
- **죽음 줄과 신학**: `log_death_hazard`(`Player/PlayerRoleSubsystem.cpp:83-90`)의 세 변형('…별은 한 사람을 잃었다'·'빈자리 하나'·'한동안 조용했다')은 한 백성 전체가 그친 틱에 참이 아니다(변형은 어느 길로 왔든 참 — `docs/70:217`). 권장: 사인은 `Death.Hazard` 그대로 두고(새 저장 태그 없음 — 비문은 이미 위기 행을 읽는다, `UI/CosmicCallingPagesWidget.cpp:194`) `DeathRowOf`가 `KillsTheRulerOf`인 행의 죽음에 제 행 `log_death_extinction`(§3.1)을 고른다 — 말하기 한 수 그대로. 글의 규칙은 §3.5의 3.

### 2.3 방주와 조약·AI의 조약 걸음 (`[B-6b2]` 장치, `[B-6b6]` 행 — 결정 1·6~8)

**방주**(결정 6): 새 행 `project_ark`(이름 최종), 절멸 행의 `RulerResolutions`에만 들어 대응 칸에서 돈다 — 절멸이 선 동안만 시작하고 위기와 함께 끝나며, 완성되면 그 위기를 해소하고 기록이 그 성계에 `CivID` = 주인으로 남는다(`Data/CosmicDefinitionRows.h:198-203`, `Sim/CosmicMegaprojects.cpp:599-628`). `RequiredTier` T3, `RequiredProgress`는 relief의 150에서(율 1로 250 안에, 값을 못 낸 틱의 여유를 두고 — 노트 §2), 인과율 비용 없음, `CompleteTemplates` 셋 이상.
- AI는 `Choices`가 답을 먼저 골라(`:311-331`) 스스로 짓고, 측정의 통치자 정책도 같은 함수로 답한다(`World/CosmicRulerPolicy.cpp:233-238`). **셋째 칸 위의 AI 지연(결정 17)은 건설 칸에만 걸려 방주에 닿지 않는다** — 닿으면 늦춘 AI가 지워진다. 답에도 걸리는 `AIAscensionSpeedMultiplier`(`:596`)에는 `Data.Integrity`가 '방주의 진행 ÷ 그 값 < 절멸의 카운트다운'을 붙든다.
- 기록이 성계에 남으므로 넷째 칸 전에 그 성계를 잃으면(편입·공성) 다음 절멸을 기다린다 — `race:` 줄이 센다(§5.1).

**조약 조건**(결정 7): `EConditionType::CivHasTreaty`를 `CivHasEdict`(`Data/CosmicConditionTypes.h:91`) 뒤에 덧붙인다(새 열거자 셋의 차례는 §2.12). Tag = `Treaty.*`, 비면 어느 조약이든; Value = 그 조약을 함께 지닌 백성의 수, 1부터의 정수; 문맥 백성만 읽는다(`CivSettledSystemsAtLeast`의 꼴). 새 순수 함수 `CosmicDiplomacy::TreatyPartnersOf(Civ, Treaty)`(`HasTreaty`·`TreatyCountOf` 곁 — `Sim/CosmicDiplomacy.h:62-68`). 스키마·`Tools/test_data_schema.py`·`docs/21` §7.3.
- **생애에 상대적이지 않다**: 조약에는 맺은 틱이 없다(`Core/GalaxyState.h:112-125`). 앞 대의 것을 받는 구멍(`crisis_and_transcendents.md:225`)은 방주의 반쪽(이번 생애의 기록)이 막는다.
- 잃는 길은 통치자의 깨기, 짝의 사라짐(`ForgetCiv` — 절멸도), 적대가 깨는 교역뿐(`Sim/CosmicDiplomacy.h:133-136`).
- 조건으로 두는 까닭: '+ 외교 조약'이 판정과 다음 칸 툴팁(§4)에 보인다.

**넷째 칸 행**(결정 8): `[MegaprojectComplete project_ark, CivHasTreaty 1]`(Tag 없음), `AscensionCost` = `ShiftEthics` Openness −0.1(`pillars.md:93` — 추첨 없음), `GrantedPowers` 비움, 이름 `feat_passage_by_ark`(§3.1 — 규칙 7). 공격 정책은 조약을 청하지 않아(`World/CosmicRulerPolicy.cpp:762-764`) 그 칸은 넷째 칸에 닿지 않는다 — 측정이 따로 적는다.

**AI의 조약 걸음**(결정 1 [개발자]):
- **자리**: AI 단계 (a), 다스리지 않는 백성마다 `Civs` 차례로, 칙령 철회(`Sim/GalaxySimulationSubsystem.cpp:2818-2823`)와 법칙 걸음(§2.4) 뒤·`TickAIDiplomacy`(`:2825`) 앞 — 같은 틱의 공성 겨냥(`AIAimOf`는 불가침의 짝을 건너뛴다, `Sim/CosmicConflict.cpp:519`)이 그 조약을 본다. 두 걸음은 다른 칸의 행이 묻는 것이라 한 백성의 한 틱에 함께 서지 않는다.
- **때**: 그 백성의 칸에서 판정되는 행(`IsJudgedRulerRowAt` — `Data/CosmicDefinitionRows.h:298`)의 `CivHasTreaty`가 서지 않을 때. 방주를 기다리지 않는다.
- **하는 일**: 한 틱에 하나 — `Civs` 차례로 다스리지 않고, 국경을 맞대고(`CosmicDiplomacy::BordersOf`), `TreatyBlockOf(…, Treaty.NonAggression, bMet = true)`가 `None`인 첫 백성에게 불가침을 청한다. 순수 문은 `NotRuler`를 내지 않고(명령의 것 — `Player/PlayerRoleSubsystem.cpp:970`) met의 참은 `SetStance`의 꼴이다(`Data/CosmicConditionTypes.h:153` — 세계는 앎을 읽지 않는다). 받아들임·적대·비용 50 영향력은 문이 묻고, 명령처럼 치르고 `Sign`한다(`:979-983`) — 줄·추첨·앎 쓰기 없음.
- **불가침인 까닭**: 교역은 틱마다 영향력을 더해(`Sim/GalaxySimulationSubsystem.cpp:1743-1748`) AI의 경제를 바꾸고 적대가 깬다. 스키마는 `DT_Feats`의 `CivHasTreaty`에 Tag를 두지 않는다(걸음의 불가침이 늘 채운다 — §3.3의 6′).
- **다스리는 백성은 짝이 아니다**: 그 동의는 통치자의 명령이고 AI가 청하는 카드는 B7이다(`systems.md:65`). 거꾸로 그릇이 AI와 맺은 조약은 그 AI의 넷째 칸도 돕는다 — 조약 툴팁에 한 줄(§4). 섭정 아래의 옛 백성은 통치자의 조약을 지닌 채(`TickAIDiplomacy`는 우호만 놓는다 — `:2834-2858`) AI로 오른다(#121).
- **장치 커밋에서 잠잔다**: 묻는 행이 없으면 아무것도 하지 않아 `[B-6b2]`가 비트 그대로다. 행이 켜지면 AI끼리의 불가침이 그 짝의 공성을 닫고 관계 표류의 목표를 올린다(`Sim/CosmicDiplomacy.cpp:103`) — 묶음이 잰다(#129).
- **B7에 남는 것**: 프로필의 외교·통치자에게 청하는 카드·조약 깨기·AI의 교역(고칠 문서는 §7).

### 2.4 은하 법칙 — 칙령 범위의 넓힘·법칙을 지킨 틱·AI의 법칙 걸음 (결정 9·10·11; `[B-6b1]` 필드, `[B-6b2]` 장치, `[B-6b7]` 행)

'은하 법칙'은 범위가 고향 은하로 넓어진 칙령이다(`systems.md:169`; 넓힘은 넷째 칸부터 — `pillars.md:78`). `Law.*` 다섯은 늘 서 있어 '셋을 지킨다'의 뜻이 될 수 없다. 넓힘이 무엇을 하는지는 어느 문서에도 없어 여기서 정한다(결정 9 [개발자]).

- **행과 칸**: `DT_Edicts`에 `RequiredTier` T4 행 셋 이상(§3.1)과 끝에 더하는 정의 칸 `Scope`(새 UENUM `ECosmicEdictScope { People, HomeGalaxy }` — 저장 아님; 기본 `People` — 지금 일곱 행은 `People`을 적을 뿐 뜻이 그대로, §3.3의 4). 칙령 칸은 Society 표지 {0.5, 1.0}으로 많아야 셋이라(`Sim/CosmicEdicts.cpp:25-34`) **법 셋은 모든 칸을 쓴다** — 백성 범위의 칙령을 내려놓아야 하고, Society 1.0에 못 이른 백성은 셋을 지닐 수 없다.
- **넓힘이 하는 일 — 보호하는 위기 고리 둘만**: `HomeGalaxy` 행의 ①(발생 몫의 곱)과 ②(놓일 때의 카운트다운 더하기)(`systems.md:153-154`)가 같은 `GalaxyRow`의 모든 정착된 성계에 닿는다. 나머지 칸(틱당 효과·유지비·제정 비용·함대·조약·영압 소모)과 ③(만료 때의 답)은 지닌 백성의 것 그대로다.
- **행마다 한 번(합집합)**: 성계의 ①·②는 주인의 `ActiveEdicts`(두 범위, 그 차례) 뒤에 같은 `GalaxyRow`의 다른 백성이 지닌 `HomeGalaxy` 행을 `Civs` 순서·각자의 차례로 잇고 이미 든 행 이름은 건너뛴 목록을 걷는다 — ①은 그 차례의 곱, ②는 양수의 합(지금 꼴 — `Sim/CosmicEdicts.cpp:133-160`). `SpawnMultiplierOf`·`CountdownBonusOf`가 `State`·색인·성계를 받고, 부르는 곳 셋(발생 몫 `Sim/GalaxySimulationSubsystem.cpp:2329`, `CosmicHazards::Place` `Sim/CosmicHazards.cpp:174`, 커맨드렛 `Tests/CosmicBalanceCommandlet.cpp:1947`)이 따라온다. `HomeGalaxy` 행이 없으면 목록도 곱의 차례도 지금과 같아 **비트 그대로**다.
- **스키마**(§3.3의 4): `HomeGalaxy` 행은 ① ≤ 1·② ≥ 0만, 어느 Hazard의 답도 아니고, `Hazard.Claim`(신정론)·`Hazard.Extinction`·`Hazard.PreSupernova`(남의 업적을 늦추지 않게 — 방주는 T3→T4, 안정화는 `feat_supernova_stabilized`의 일)에 고리가 없다.
- **신정론 불변식**(`Tests/CosmicDataTests.cpp:2218-2232`; 칙령의 합 `:2287-2293`)은 모든 칙령 행의 더하기를 태그마다 한 번 센다. 합집합은 한 성계에서 한 행을 한 번만 세어 그 합이 여전히 위의 끝이고, 법에 주장 고리가 없어 W는 그대로다. 행을 켜는 커밋이 다시 돌린다.
- 화면의 낱말은 '칙령'이다('은하 법칙'은 설계의 낱말). 다섯째 칸이 판을 끝내므로 그 칸의 범위는 묻지 않는다.

**법칙을 지킨 틱**(결정 10 — 필드는 §2.1):
- `FCivilizationState.LawsHeldTicks`(int32 = 0, 저장), 노브 `DT_Rules.GalaxyLawsToHold`(3), 조건 `CivLawsHeldTicksAtLeast`(`EConditionType` 끝 — Key = N틱, `TickCountOf`로 읽고 Value는 읽지 않는다).
- **셈**(`[B-6b2]` — §2.1): 경제 단계 끝 `LowStabilityTicks`·`FallenSphereTicks`의 고리(`Sim/GalaxySimulationSubsystem.cpp:1881-1889` — 유지비와 그 소멸 `:1856-1878` 뒤)에서 백성마다(AI 포함): Tier ≥ T4 ∧ 지닌 `HomeGalaxy` 행(표가 지닌 것만) ≥ 노브이면 +1, 아니면 0 — 지금에서 끝나는 연속한 틱의 수.
- **생애 상대의 읽기**: 참 ⇔ `Min(LawsHeldTicks, Tick − SinceTick)` ≥ N. 생애는 `BornTick`에 서고(`Player/PlayerRoleSubsystem.cpp:190`) 그 틱의 경제 단계는 이미 지났으므로 이 생애 안의 경제 단계는 `Tick − BornTick`개이고, 셈이 지금에서 끝나는 연속이라 작은 쪽이 '이 생애 안에서 지킨 틱'이다(판정은 같은 틱 AI 단계 끝이라 그 틱의 셈이 든다). AI는 `SinceTick` 0. 새 필드 없이 생애 상대화(`crisis_and_transcendents.md:225`)를 지킨다.

**AI의 법칙 걸음**(결정 11 — AI가 스스로 하는 일 `crisis_and_transcendents.md:134`에 하나를 더한다, 결정 1):
- AI 단계 (a)(`Sim/GalaxySimulationSubsystem.cpp:2785-2826`), 칙령 철회(`:2818-2823`) 바로 뒤·조약 걸음(§2.3) 앞: Tier ≥ T4이고 판정되는 행(`IsJudgedRulerRowAt`)이 `CivLawsHeldTicksAtLeast`를 지닌 AI 백성은 `HomeGalaxy` 행을 표 순서로 걸어 `EnactBlockOf`(`Sim/CosmicEdicts.cpp:46-85`)가 None인 것마다 `Enact`한다. 추첨·줄 없음.
- `AIRevokesOf`(`Sim/CosmicEdicts.cpp:247`)는 `HomeGalaxy` 행을 철회하지 않는다. 섭정 아래·공백기의 백성도 AI라 법과 셈이 그릇의 죽음을 넘어 이어진다(#121 — 그 다섯째 칸은 다른 질서). 그 백성에 다시 즉위한 그릇은 `BornTick`부터 N을 다시 기다린다.
- **측정의 통치자 정책**(`World/CosmicRulerPolicy.cpp:509` `ActEdicts`, `bEdict` 아래 — 러너는 그대로): (4)와 (5) 사이의 새 걸음 — 다스리는 백성이 Tier ≥ T4이고 그 행이 법을 물으면 지니지 않은 `HomeGalaxy` 행을 표 순서로 `MakeRoomFor` → `Enact`. 내려놓을 칙령을 고를 때 법은 맨 뒤이고 (1)은 법을 놓지 않는다. 법칙 걸음의 `MakeRoomFor`는 법을 놓지 않는다 — 빈 칸이나 법이 아닌 칙령을 놓을 때만 제정하고, 칸이 노브보다 적거나 답 칙령이 칸을 쥔 동안에는 서지 않는다(법끼리 틱마다 돌아 제정 값·칙령 줄·결정 수를 내지 않게 — `DecisionsOf`가 제정과 철회를 센다, `World/CosmicRulerPolicy.cpp:869`). (2)의 답은 지금처럼 먼저다 — 각성의 답은 칙령 하나뿐이라(`edict_machine_rights`) 셋을 지킨 백성이 각성에 답하면 법 하나를 놓고 셈이 0에서 다시 선다. 그 횟수는 `race:` 줄이 센다(§5.1). 철야(3)도 같은 길로 법을 놓고(`MakeRoomFor`), 법칙 걸음은 (3)이 철야를 다시 내릴 동안과 안정도 0.5 아래인 동안 그 칸을 되찾지 않는다(답 칙령과 같은 까닭 — 둘이 경계마다 바뀌지 않게; 법은 불만이 더는 세어지지 않고 안정도가 0.5 이상이면 철야의 칸을 되찾거나 (4)가 놓은 빈 칸을 받는다 — `[B-6b2a]`); 영압 카드의 `EnactEdict`는 칸이 차면 `ChoiceGateOf`가 거절해 법을 끊지 않는다. AI는 칙령으로 답하지 않아 끊기지 않는다 — 그 몫은 노트 §2 ⓒ·ⓓ의 '법의 끊김'(결정 10). AI의 법칙 걸음은 유지비로 끊긴 법을 다음 틱에 다시 내린다 — 그 수는 `race:` 줄이 세고, 크면 값으로 고친다.

### 2.5 결박(봉인) — `project_binding` (결정 12·13·14·15; `[B-6b2]` 장치와 행의 시작값, `[B-6b7]` 값)

상태는 있고(`bSealed`·`SealedTick` — `Core/GalaxyState.h:472-478`) 쓰는 곳만 없다. 봉인은 놀이 중의 행위다('T4 봉인 Feat = 결박: 그 초월자의 영향권 동결, 신규 주장 0' — `crisis_and_transcendents.md:121`; 일곱 답의 하나 — `:128`). B6a 헤더 `:34`가 Phase 6+로 보낸 '봉인과 그 주인 필드' 가운데 봉인만 A2로 온다(결정 39).

- **행**: 건설 행 `project_binding`(`RequiredTier` T4, 높은 비용, Causality 없음; `RequiredProgress`는 노트 §2의 미리 셈), 상수 `CosmicRows::ProjectBinding`. 어느 Hazard의 답도 아니다. 행은 `[B-6b2]`부터 있다(코드가 부르는 행 — `check_data`의 `required_rows`): 판정되는 행이 부르지 않으면 `Choices`가 짓지 않고(T4 아래 백성뿐) 비트 그대로다; 패널은 같은 커밋의 '다음 칸일 때부터' 규칙(§4)으로 첫째~둘째 칸 집에 단추를 내지 않는다(`Tests/CosmicPanelTests.cpp:257`). 값은 시작값, 마지막 값은 `[B-6b7]`.
- **단일 창구**(규칙 13 — `Sim/CosmicSeal.h`, 순수, 추첨 없음):
  - `int32 CosmicSeal::BindTargetOf(const FGalaxyState&, const FCosmicDataTables&)` — `Transcendents` 순서로 처음인, 행이 `Realm.Abyssal`(`Core/CosmicTags.h:78`)이고 봉인되지 않은 인스턴스; 없으면 `INDEX_NONE`. VS의 타락은 하나다(`Tools/data/DT_Rules.json:27`).
  - 시작 거절의 새 까닭 `ECosmicStartBlock::NoFallen`(`RowTier`와 `Answering` 사이 — 열거의 차례가 `StartBlockOf`가 묻는 차례다, `Sim/CosmicMegaprojects.h:80-84`; 저장 아님 — 그 차례를 견주는 코드는 없다): `StartBlockOf`(`Sim/CosmicMegaprojects.cpp:213`)가 `RowTier` 뒤·칸 앞에서, `project_binding`에 대상이 없을 때. 패널의 키 표 둘(`UI/CosmicSystemPanelWidget.cpp:86-119`)에도 같은 자리로 하나씩 들고 `static_assert`가 수를 지킨다(`:101`·`:119`). AI·정책의 `Choices`와 통치자의 명령이 같은 문을 지난다.
- **진행과 끝**(`Advance`의 건설 칸 — `Sim/CosmicMegaprojects.cpp:630-658`): `project_binding`의 건설 칸은 `Advance`가 그 칸을 부를 때마다 값을 치르기(`:647`) 전에 `CosmicSeal::BindTargetOf`를 묻는다 — 같은 틱 같은 패스에서 `Systems` 차례로 앞선 성계의 결박이 방금 대상을 묶었어도 마찬가지다(대상을 패스나 틱 앞에서 한 번만 읽지 않는다). 대상이 없으면 값을 치르지 않고 결과 없이 끝난다(빈 칸·진행 0 — 완성 기록·줄·생애의 목록 없음). 다 차면 완성(`:599-628`)이 기록(`CivID` = 주인)을 적은 뒤 `CosmicSeal::Bind`가 대상의 `bSealed = true`·`SealedTick = Tick`을 **한 자리에서** 적는다(`docs/50:231`) — 묻기와 묶기 사이에 다른 쓰기가 없어 `Bind`는 실패하지 않는다. 위기 단계 (a)(`Sim/GalaxySimulationSubsystem.cpp:2239`) 안이라 틱 순서는 그대로이고, 같은 틱 (b)의 주장 만료는 주장자를 찾지 못한다. 그 틱의 경제 단계는 이미 돌아 압력과 소모는 다음 틱부터 멎는다.
- **줄**: 지금의 완성 줄(`:2763-2777` — 백성의 일을 3인칭 사실로) 뒤에 묶임의 줄 `log_fallen_bound`(§3.1) 하나 — 말하기 스트림 한 수; 글은 행위자 없는 수동이고 백성을 그 일의 주인으로 쓰지 않는다(§3.5의 4, 결정 32).
- **봉인이 바꾸는 것 — 이미 있는 읽는 곳**(그 자리의 새 코드 없음; 줄마다 노트 §0.4′의 표): 압력의 항·소모·틱당 효과·함대 소모가 없어지고 `FallenSphereTicks`가 0으로; `ClaimantOf`가 새 주장을 내지 않고 선 주장은 주장자가 없어 만료의 편입·알아챔·1인칭·지킴의 격차가 따라 멎는다; 이벤트 가중은 1(추첨 수 그대로); 교리를 묻는 조건이 거짓; 영향권 안 정착의 거절·계승의 거름·정책의 닿음이 영향권을 보지 않는다; 지도는 `Bound`의 회색 고리, 패널은 `Sight.Bound`; 커맨드렛의 셈에서 빠진다. 영향권은 지우지 않는다 — 그것이 '동결'이다(남은 자 법의 `InFallenSphere` `Sim/CosmicHazards.cpp:65-77`은 봉인을 묻지 않는다; 초신성이 죽은 노드를 빼는 `:406-409`만 예외).
- **봉인한 자만·생애 상대**(결정 12): T4→T5 행은 `MegaprojectComplete project_binding`을 읽는다 — 기록의 `CivID`가 판정되는 백성이고(#73 — `Events/CosmicEventSubsystem.cpp:463-466`) 그 틱이 `SinceTick` 이상일 때만 참이다. 그래서 **새 저장 필드가 없다**. `TranscendentSealed`는 어휘에 남되 행이 쓰지 않는다(`SinceTick` 0인 AI에게는 누가 묶든 영원히 참 — #73에 어긋난다).
- **닿음**(결정 13): 자기의 정착된 어느 성계에서나. AI는 `Choices`가 수도의 빈 건설 칸에 짓고(`Sim/CosmicMegaprojects.cpp:334-363`), 정책도 같은 `Choices`를 쓴다(`World/CosmicRulerPolicy.cpp:236`). 코어는 어느 AI 모성계에 1레인으로 닿고 `sys_0`에서 3레인 이상 떨어져 생기므로(`systems.md:113`) 거리 규칙은 경주를 지리에 넘긴다.
- **한 손의 경주와 잠김**(결정 14 [개발자] — 받아들이고 잰다): ① 타락이 하나라 처음 묶은 백성만 그 판에서 다섯째 칸에 설 수 있다(그 뒤 `NoFallen`). ② 묶은 뒤 다섯째 칸 전에 그릇이 죽거나 양위하면 새 생애는 그 기록을 읽지 못한다(기록의 틱 < `BornTick`) — 그 백성은 AI로만 오르고 끝은 다른 질서다. ③ 묶은 성계를 남이 쥔 동안 그 백성은 잠기고, 되찾으면 풀린다(완성 기록이 만든 이를 지녀 `HasCompleted`가 견준다 — `Sim/CosmicMegaprojects.cpp:158-172`; 그 백성이 지워지면 판 끝까지). `race:` 줄이 묶은 자의 종류·잠김·되찾음을 센다(§5.1); 잠김이 잦으면 가장 작은 고침은 주인 필드 `SealedByCivID`(결정 12 ㉯)다.
- **T4→T5 행**(결정 15): `[CivLawsHeldTicksAtLeast N, MegaprojectComplete project_binding]`, 빈 `AscensionCost`·`GrantedPowers`, 그릇 조건 없음(A3 — `Tools/data/schema.json:818`의 까닭 글을 고친다). 업적은 한 정착된 성계에서 모든 조건이 설 때라(`Ascension/AscensionSubsystem.cpp:90-124`) 기록이 선 성계에서 선다. 이름 `feat_laws_and_binding`(§3.1 — 규칙 7).

### 2.6 넷째 칸의 제동과 셋째 칸 위의 지연 (결정 16·17·18; `[B-6b2]`)

- **제동**(결정 16): `docs/21:1574-1580` 그대로, 값 2(`Tools/data/DT_Galaxies.json:8` 고향 행). 순수 창구 `bool CosmicAscent::FourthRungOpenOf(const FGalaxyState&, const FCosmicDataTables&, FName GalaxyRow)`(`Sim/CosmicAscent.h` — 규칙 13): 상한은 그 행의 `AIFourthRungCap`(`Data/CosmicDefinitionRows.h:768-772`; 행이 없거나 0 이하 = 제동 없음). T4Civs = 그 `GalaxyRow`의 Tier ≥ T4 백성 수(다스리는 백성 포함), Inbound = `Transits` 가운데 `ToGalaxyRow` = G ∧ `ArriveTick ≥ Tick`(6a까지 비어 0 — `Core/GalaxyState.h:884`; 6a가 고치지 않게 지금 센다). 열림 ⇔ T4Civs + Inbound < 상한.
  - 묻는 곳은 `JudgePeople`(`Ascension/AscensionSubsystem.cpp:67`) 하나 — `!bPlayer` ∧ Tier == T3일 때 행을 보기 전에; 닫혔으면 그 틱에 오르지 않는다. 판정은 다스리는 백성 먼저, 그다음 `Civs` 순서이고(`:44-64`) `Ascend`가 Tier를 곧 바꾸므로 '먼저 판정된 백성이 먼저 지난다'와 '같은 틱에 앞서 오른 백성을 센다'가 저절로 선다. 그릇의 백성과 T4→T5는 막지 않는다(`systems.md:413`). 추첨·필드·줄 없음; 본문·UI·로그에서 '자리'라 부르지 않는다(`systems.md:413`).
  - 막힌 AI는 방주 기록을 지닌 채(`SinceTick` 0) 셈이 상한 아래로 내려가는 틱에 지난다 — 절멸이 넷째 칸 백성을 지워 그 셈을 줄일 수 있다(§2.2). '넷째 칸 이상의 존재'(`crisis_and_transcendents.md:65`)는 봉인되지 않은 타락 하나와 제동이 들인 AI 많아야 둘이다(다스리는 백성은 세지 않는다; 제동 없이 넷째 칸에 오른 다스리는 백성이 섭정 아래 AI가 되면 그 생애마다 하나씩 더할 수 있다 — §5.1, 결정 40).
- **셋째 칸 위의 지연**(결정 17): 노브 `DT_Rules.AIUpperRungProgressMultiplier`(float, 기본 1 — 비트 그대로). `Advance`의 **건설 칸에서만** 다스리지 않는 백성의 속도(`Sim/CosmicMegaprojects.cpp:596`)에 곱하고, 행의 `RequiredTier` ≥ T3일 때만 — B6b에서는 `project_binding` 하나다. 대응 칸(방주 등)에는 닿지 않는다: 늦춰진 방주는 250틱을 넘겨 그 AI를 지운다. 값은 `AIAscensionSpeedMultiplier`처럼 비교로 읽고(0 이하·수 아님 → 오류 한 줄, 1 — `Sim/GalaxySimulationSubsystem.cpp:2704-2710`의 꼴) 이진수로 정확한 값만 쓴다(`Sim/CosmicMegaprojects.h:252-254`). #161 띠의 주 다이얼이다(결정 35).
- **넷째 칸이 공짜로 주는 것**(결정 18 — 다 받아들이고 잰다): 넷째 칸인 침묵(`trans_silence`)에게서 소모·틱당 효과·함대 소모가 오지 않는다(거름마다 '백성보다 위의 근원'을 묻는다 — `Sim/GalaxySimulationSubsystem.cpp:1356`·`:1390`·`:1809`·`:1839`) — 그래서 결박은 묶은 자가 아니라 침묵 둘레의 첫째~셋째 칸 백성에게 준다. 영토 상한 7(`Tools/data/DT_Rules.json:40`)이라 AI의 정착 추첨이 돌아오고(`Sim/CosmicMegaprojects.cpp:391-402`) 공성이 다시 열린다(`Sim/CosmicConflict.cpp:248-251`). 수입 ×3(`Tools/data/DT_Rules.json:58`), 셋째 칸 이하를 온 힘으로 치는 칸 차(`Sim/CosmicConflict.cpp:55-58`). 주장은 주인의 칸을 묻지 않아(`Sim/CosmicHazards.cpp:529`) 넷째 칸 백성의 성계에도 선다 — 그 답이 결박이다. 이 거름들을 바꾸는 것은 세계 코드라 B6b는 손대지 않는다.

### 2.7 견딘 영압의 시대 (#156 — 결정 22 [개발자]; 셈 `[B-6b1]`, 조건·선택자·열쇠 `Borders` `[B-6b2]`(결정 22의 답 ㉮), 글 `[B-6b5]`, 행 `[B-6b8]`)

설계: '조공·수도 상실 없이 견디면 Epithet + 이웃 Reverence +0.2'(`crisis_and_transcendents.md:115`). 업적 행이 아니라 이벤트 행 하나다 — 업적 행은 칸을 올리고 AI도 같은 행으로 판정된다(#156 ③).

- **셈**(필드 §2.1): 경제 단계 끝 `LowStabilityTicks`·`FallenSphereTicks`의 고리(`Sim/GalaxySimulationSubsystem.cpp:1884-1889`)에서 모든 백성에 — 이 틱에 영압이 안정도를 깎았고(`Loss > 0` — `:1831-1835`) 수도가 봉인되지 않은 타락의 영향권 밖이면(`FallenSphereTicks`의 영향권 — `:1681`; 안이면 설계의 '종속') +1, 아니면 0. `CosmicHazards::MoveCapital`(`Sim/CosmicHazards.cpp:314`)이 0으로 돌려 '수도 상실 없이'를 새 필드 없이 읽는다. 추첨·앎 읽기 없음.
- **조건** `CivPressureEnduredTicksReached`: `CivFallenSphereTicksReached`(`Data/CosmicConditionTypes.h:67-76`)의 한 슬롯 창 그대로, Named·SettledSystem 행 금지(스키마 `:466`·`:471`·`:732`의 꼴). Key 600 — 설계의 시대 주기(`systems.md:233`).
- **선택자 `Ruled`**: `SystemOwnedBy`의 Key 낱말(`docs/21:619`에서 정했고 코드에 없다 — `Events/CosmicEventSubsystem.cpp:455-461`) = 그릇이 몸을 입고 다스리는 동안의 `Player.RuledCivID`, 그 밖에는 거짓. 문명 주제 행의 성계는 수도라(`:198`·`:387`) '다스리는 백성이고 수도가 정착됐다'가 된다 — **`SystemOwnedBy`가 싣는다**(평가기에서 확인; 깨지는 경우는 AI 백성). 스키마 `civ_selector`(`schema.json:35`)에 낱말을, 슬롯 행 규칙의 까닭 글(`:421`)을 고친다.
- **열쇠 `Borders`**(결정 22의 답 ㉮): `ChangeReverence`의 Key `Borders` = 국경을 맞댄 백성마다(`CosmicDiplomacy::BordersOf` — `Sim/CosmicDiplomacy.h:94`, `Civs` 순서) 더한다(0~1로 자른다); 틱마다 도는 원천(`schema.json:1139`)에는 금지.
- **행** `event_era_endured`(§3.1): Standing·Civilization·`Log.Civ`·Severity 1, `[SystemOwnedBy Ruled, CivPressureEnduredTicksReached 600]`, 효과 `ChangeReverence Borders +0.2`(결정 22의 답 ㉮ — 설계 그대로), 변형 셋 "{Civ}의 백성이 이 시대를 '…의 시대'라 부르기 시작했다." — 변형마다 제 시대, `{Epithet}` 없음. 견딤 하나에 한 번(`event_book_closed` — `Tools/data/DT_Events.json:2-49`의 꼴).
- **쓰지 않는다**: `FLifeRecord.EpithetRow`(`Core/GalaxyState.h:557-559`) — 시대 이름은 사람에게 붙지 않는다(`docs/70:192`, #156 ①). 첫째~셋째 칸에서 결박 전까지만 닿는다(넷째 칸 백성은 깎이지 않는다). '조공 없이'는 #137까지 빈다.
- **글**: §3.5의 6. 설계의 경외 +0.2가 남으므로(결정 22의 답 ㉮) 줄은 그 수를 말하지 않고 그릇을 우러르는 말이 없다(메시아 신호 9·10 — `theology_and_bestiary_notes.md:243-244`; 경외는 예배가 아니다 — `systems.md:195`).

### 2.8 방랑자의 증언 (#141 — 결정 20·21 [개발자]; 글 `[B-6b5]`, 행 `[B-6b8]`)

A3로 개인 칸은 첫째에 머문다(`Player/PlayerRoleSubsystem.cpp:530-535`). 설계의 다섯 선택(`crisis_and_transcendents.md:115`) 가운데 첫째 칸이 가질 둘만 짓는다.

- **`enc_witness`**(설계 이름 — `crisis_and_transcendents.md:27`, `pillars.md:76`; B1의 서 있는 위기 카드 꼴 `Tools/data/DT_Encounters.json:1228`): `PlayerRoleIs Wanderer` + `PlayerInSystemWithHazard Hazard.Claim`, `PlayerTierAtLeast` 없음. [증언](설계 낱말 — 화면의 라벨은 §3.5의 7) Mind 3 — 이기면 그 성계 주인에게 `ChangeReverence +0.1`, 지면 없음(설계가 미룬 분별 — `crisis_and_transcendents.md:112`). [물러난다](설계의 도주 — `docs/70:139` 규칙 1의 라벨; 그릇이 떠났다고 쓰지 않는다) 효과 없음(B1의 꼴 — 규칙 2; 설계의 −0.1은 결정 20의 대안).
- **싣지 않는다**: `ResolveHazard`·`DelayHazard` — '해결되면 아무 일도 없다'(`crisis_and_transcendents.md:121`)대로, B1 헤더 `:289`를 바로잡는다; 50틱 카드가 하나 더면 신정론 검사(`Tests/CosmicDataTests.cpp:2294-2308`)가 0.976^60 ≈ 0.233 < 0.25로 깨진다. **새 스키마 규칙**(§3.3의 8′): `Hazard.Claim`을 조건에 싣는 카드는 `ResolveHazard`를 싣지 않고 주장의 `DelayHazard`는 `petition_pressure_measured`의 것뿐(지금 데이터가 지킨다).
- **목소리**: `WithinWandererGap`(≤ 1 — `Events/CosmicPetitions.cpp:176-180`) 그대로라 첫째·둘째 칸 백성의 땅에서만 온다; 주장자(넷째)는 세 칸 위라 부르지 않는다(글은 §3.5의 7).
- **결정론**: 코드·필드·열거자 없음, 받아들인 답마다 1수. 정책의 고름(`World/CosmicWandererPolicy.cpp:61` — `Answers % Choices`)이 한 장만큼 밀린다 — B1 지킴의 기제(§5.3).
- **옮겨 간다**: 대면(셋째 칸 이상)과 정신의 닻(스키마는 `Path` Ruler만 — `schema.json:785-787`)은 방랑자의 칸이 오르는 블록(Phase 4)으로 — #141을 가른다. 피난은 절멸의 답(#138 닫음 — 결정 21). `crisis_and_transcendents.md:99`의 '넷째 칸의 직접 대면'에 그 블록을 적는다(§7).

### 2.9 보는 것과 부르는 이름 — 격차 술어·즉위의 문·이름 새는 줄 (#107·#122·#121·#162 — 결정 19·41; `[B-6b3]`, 글 `[B-6b5]`)

넷째 칸 백성이 생기면 격차 규칙(보는 쪽보다 세 칸 이상 위는 보이지 않는다)이 처음 선다. 방랑자·공백기의 보는 칸은 개인 칸이라(`Sim/GalaxySimulationSubsystem.cpp:1305-1316`) 늘 첫째다 — 넷째 칸 이상은 모두 가려진다.

- **술어 하나(#107)**: 새 `Core/CosmicKnowing.h`(순수) — `RungsAbove(Of, Viewer)`와 `IsFarAbove`(≥ 3; `World/CosmicMapLook.h:304-311`의 `ShownGap`이 옮겨 온다). 부르는 넷: `Tell`(`Log/CosmicLogSubsystem.cpp:209`), `NoteMet`(`Sim/GalaxySimulationSubsystem.cpp:1256-1260`), `IsShownPeople`(`World/CosmicMapLook.cpp:378-382`), `WithinWandererGap`(≤ 1). 비트 동일 — `-TellAll=0`의 정책 판과 해시 판이 증명; 대조 테스트에 둘째 백성의 격차.
- **즉위의 문(#122 — 결정 41 [개발자], 권장)**: `ECrownBlock`(저장 아님 — `Sim/CosmicWanderer.h:32-49`) 끝에 `FarAbove`, `Unmet`(`Sim/CosmicWanderer.cpp:99-102`) 바로 뒤에서 `IsFarAbove(Civ.Tier, Calling.PersonalTier)` — 즉위·복귀 모두, 세계만 읽는다. 없으면 지도가 가린 넷째 칸 백성에 즉위해 `Ending.RuledPeople`을 얻는다. A3로 개인 칸은 첫째라 이 문은 넷째 칸 이상 어느 백성에도 즉위·복귀를 막는다(결정 41이 묻는 것). 패널: `CrownKeyOf`(`UI/CosmicSystemPanelWidget.cpp:194-215`)는 `FarAbove`를 단추 없는 무리(`NotStanding`·`NotSettled` 곁)에 둔다 — 가려진 백성에게 '너무 높다'는 회색 단추는 그 백성을 드러낸다; 새 키 없음.
- **이름 새는 줄**: 다스리는 백성이 당사자인 공성 줄은 상대를 `NoteMet`에 보내고 거절을 보지 않은 채 `{OtherCiv}`로 부른다 — AI의 공성 시작(`:2887-2898` — `log_siege_on_us`·`log_treaty_broken`)과 **공성의 끝**(`TellSiege` `:2044-2057` — 다스리는 백성이 주인일 때 `log_siege_lost`·`log_siege_held`·`log_siege_lifted`), 그리고 **우호의 소멸**(외교 단계의 유지비 — `:1989-1999`, `log_stance_lapsed`; 첫째 칸 백성의 벗이 넷째 칸에 오른 뒤). 상대가 다스리는 백성보다 `IsFarAbove`면 이름 없는 변형(`…_far` 여섯 — §3.1)을 고른다; 칸은 세계의 값이고 어느 쪽이든 말하기 한 수다. 공격자 쪽 줄은 닿지 않는다(두 칸 위 주인은 에워쌀 수 없다 — `Sim/CosmicConflict.cpp:252-255`). 행은 `[B-6b3]`에 초안으로(고르는 판이 없어 비트 동일).
- **#121 그대로**: 섭정 아래 옛 백성은 AI로 올라 그 다섯째 칸은 `Ending.OtherPeople`; 넷째 칸에 선 옛 백성으로는 돌아가지 못한다. 까닭 줄은 그 백성을 부른다(§4).
- **먼 줄(#162)**: `log_far_ascent`의 세 변형을 어느 칸에도 참인 글로(§4의 #162, 결정 29) — 글만이라 비트 동일.
- **다시 잼(#129·#154 — 적기만, `[B-6b11]`)**: #129는 상한 7·10과 6,000 뒤의 끝에서 0.75/0.79의 짝. #154 — 넷째 칸 이상의 오름은 첫째 칸의 눈에 가려(`World/CosmicDigest.cpp:397`) 늦은 '오른' 항목은 셋째 칸까지에서만 온다고 미리 적는다.

### 2.10 지킴의 지연 상한 (#132 — 결정 23; 함수 `[B-6b2]`(몫 꺼짐), 값 `[B-6b8b]`)

- 노브 `FleetDelayCountdownShare`: C++ 기본 0 = 몫 없음(지금 셈), `[B-6b8b]`에서 0.5; 범위 [0, 1]은 스키마와 그 테스트.
- `CosmicConflict::GuardDelayCapOf(Rules, CountdownTicks)` = min(`FleetDelayCap`, 몫 × 행의 `CountdownTicks`) — 몫이 0 이하·수 아님이면 앞의 것만; 곱과 비교뿐(규칙 6). `GuardDelayTicksOf`(`Sim/CosmicConflict.cpp:395-405`)는 그대로 `FleetDelayCap`만 묻고 — 행 없이 부르는 패널의 지연 표지(`UI/CosmicSystemPanelWidget.cpp:1084`)가 그대로 읽는다 — `FleetDelayOf`(`:366-392`)가 `min(GuardDelayTicksOf(…), GuardDelayCapOf(Rules, Row.CountdownTicks))`를 낸다; 신정론 검사(`Tests/CosmicDataTests.cpp:2257`의 `GuardCap`을 행마다 — 규칙을 베끼지 않는다)도 `GuardDelayCapOf`를 읽는다.
- 0.5의 상한: 깨어남 75(150의 50% — 띠 안), 역병·초신성 전조·주장 100 그대로, 절멸 100(250의 40%). **W 다시 셈(같은 커밋)**: 300 + 철야 100 + 100 + 카드 50 = 550 그대로 — W ≤ 1.2445·왕좌 잠금 600·판의 경우(549/550 — `:2606-2641`) 그대로.
- `Sim.FleetDelay`(`Tests/CosmicDiplomacyTests.cpp:1405`)에 깨어남(함대 3: 150 → 225)을 더한다. 입력 없는 판은 그대로(AI는 지키지 않는다). 소모의 절반은 '생기지 않음'으로 닫는다(넷째 칸 위에서 서지 않는다 — `Sim/GalaxySimulationSubsystem.cpp:1356`; B7로). #142는 §5.3(결정 24).

### 2.11 같은 틱의 차례 (★ = B6b의 새 걸음; `docs/20`·`docs/21` §7.4를 `[B-6b1]`이 먼저 고친다)

표(단계마다 지금 · B6b · 시뮬 스트림)는 노트 §0.4의 둘째 표다(그 글이 이 절의 본문이다). 요지: 경제 끝의 셈 둘; 분쟁·AI (a)의 공성 줄과 외교의 우호 소멸 줄에 이름 없는 변형; 위기 (a)의 셋째 칸 위 지연과 결박의 끝; (b) 절멸의 만료 — 마지막 성계의 문 → `LetPeopleGo` → `KillPlayer` → `DeposeRuler` → 기록 줄; (c) 은하 법칙의 갈고리 ①; (d) 시대 행; AI (a)의 법칙 거두지 않음 → 법칙 걸음 → 조약 걸음; (b)의 제동; 건넘이 절멸의 백성을 지움; 앎 끝의 `enc_witness`; 열째 단계의 절멸의 죽음. 새 시뮬 수는 없다.

갈고리 ②(놓일 때의 카운트다운 더하기 — §2.4)와 `GuardDelayCapOf`(§2.10 — `FleetDelayOf` 안)는 `CosmicHazards::Place` 안에서 읽힌다 — 놓기가 서는 모든 자리: (b) 만료 뒤의 주장 규칙, (c) 발생·수도 규칙·첫 위기, (d) 슬롯 행의 `SpawnHazard`, 틱 밖 카드 답의 `SpawnHazard`. 어느 자리든 추첨 수는 그대로다. 연구는 그대로, 외교는 ★ 우호 소멸 줄의 이름 없는 변형 `log_stance_lapsed_far`(§2.9 — 말하기 한 수 그대로)뿐. 결박이 (a)라 같은 틱의 주장 규칙과 판정은 묶인 초월자를 이미 본다.

### 2.12 결정론과 저장 — 한눈에

표(입력·자리마다 시뮬 스트림·말하기 스트림·앎·저장)는 노트 §0.4.

- **새 시뮬 추첨 지점 0.** 세계를 따라 수가 바뀌는 곳: 발생(상한 7·10으로 성계가 는다), 넷째 칸 AI의 정착, 절멸의 만료, 죽음과 계승. 새 업적 행의 대가는 뽑지 않는다(결정 8). 새 파생 정수 용도 0.
- **해시**: 새 필드 둘은 두 해시(§2.1·§5.4); 결박·경외·법칙·조약은 세계라 두 해시; 새 로그 행은 전체 해시만; 글만은 비트 동일. 빼는 목록 그대로 — 규칙 6의 문장 그대로.
- **끝에 더한다**: `EConditionType` 셋(`CivHasTreaty`·`CivLawsHeldTicksAtLeast`·`CivPressureEnduredTicksReached` — `CivHasEdict` 뒤); 정의 UENUM `ECosmicEdictScope`; 저장 아닌 `FarAbove`·부분 종류 `Treaty`·`Laws`(`NoFallen`도 저장 아님 — 차례는 `StartBlockOf`가 묻는 차례라 `RowTier` 뒤, §2.5). 새 효과 종류·태그 0(`Ruled`·`Borders`는 Key 낱말 — `Borders`는 결정 22의 답 ㉮).
- **마지막 이름**: 새 행은 모두 세이브에 남아 처음 이름이 마지막이다(§3.1, 규칙 7).
- **헤더 먼저**: 두 필드와 셈, 열거자 셋과 `Scope` 열, 표의 ★, `bSealed`의 첫 쓰기, 코드의 죽음(결정 5), Core 술어(결정 38). `SaveVersion` 1 그대로 — 옛 세이브의 셈은 0에서 서고 6,000에 끝난 B6a 세이브는 끝난 채다(A4).

## 3. 데이터

### 3.1 새 행과 그 이름 (이름은 세이브에 남는다 — 규칙 7; 이름의 차례가 걷는 차례다)

| 행 | 표·커밋 | 이름이 서는 자리와 그 까닭 |
|---|---|---|
| `crisis_species_extinction` | `DT_Hazards`, `[B-6b6]`(결정 4) | 추첨 위기의 몫은 이름 차례로 쌓인다(`Data/CosmicDataTables.cpp:143-148`, `Sim/GalaxySimulationSubsystem.cpp:2317-2365`). 표의 끝이라 옛 몫이 제자리이고, 합이 1 아래인 한 세계는 절멸이 처음 놓인 틱부터만 갈린다(앞으로 정렬되는 이름은 결정 4의 대안). 태그 설명(`Config/DefaultGameplayTags.ini:20`)을 고친다 |
| `project_ark` | `DT_Megaprojects`, `[B-6b6]`(결정 6) | 절멸의 답(대응 칸). 프로젝트 표를 걷는 세계 규칙은 없고 패널만 이름 차례로 단추를 놓는다(`UI/CosmicSystemPanelWidget.cpp:1017-1021`) — 답은 그 위기가 선 성계에만. 코드가 부르지 않아 상수 없음 |
| `project_binding` | `DT_Megaprojects`, `[B-6b2]`(결정 12 — 코드가 부르는 행이라 그 코드와 한 커밋, `required_rows`에; 값은 노트 §2 ⓒ의 시작값, 마지막 값은 `[B-6b7]`) | 완성의 고리와 `NoFallen`이 부른다 — `CosmicRows::ProjectBinding`; 패널은 이름 차례로 단추를 놓되 '다음 칸일 때부터'(§4)로 거른다 |
| `feat_passage_by_ark`(셋째→넷째), `feat_laws_and_binding`(넷째→다섯째) | `DT_Feats`, `[B-6b6]`·`[B-6b7]`(결정 8·15) | 판정·`Choices`·툴팁은 이름 차례로 걷되(`Data/CosmicDataTables.cpp:150-155`) 칸으로 거른다(셋째부터 칸마다 한 행). 둘 다 `feat_dyson_swarm` 뒤라 첫 행이 첫째→둘째 행으로 남는다 — `UI.AscensionOverlay`(`Tests/CosmicAscensionOverlayTests.cpp:69-71`)와 `CosmicPreviewRung`의 대체(`World/CosmicGameMode.cpp:1793`)가 그렇게 읽는다 |
| `edict_wide_machine_accord`, `edict_wide_quarantine`, `edict_wide_warning` | `DT_Edicts`, `[B-6b7]`(결정 9·11) | `edict_wide_`는 지금의 일곱과 예약 둘(`edict_reading`·`edict_sabbath`) 뒤라 칙령 창(`UI/CosmicEdictWidget.cpp:689-698`)의 끝에 모인다. AI의 법칙 걸음이 이 차례로 내리므로 `ActiveEdicts`와 고리를 곱하는 차례가 이름으로 정해진다(`Data/CosmicDataTables.h:119-120`의 주석을 고친다). 고리는 행마다 하나(각성의 몫·역병의 몫·두 위기의 남은 시간 +), 값은 `[B-6b7]` — 각성의 몫을 낮추면 첫 넷째 칸 뒤 고향 은하의 주장도 몫만큼 준다(결정 9 — `[B-6b7]`에서 #136과 함께 본다; 남은 시간만의 꼴이 대안) |
| `event_era_endured` | `DT_Events`, `[B-6b8]`(결정 22) | 슬롯은 이름 차례로 본다(`Data/CosmicDataTables.h:102-103`) — 상시 행 가운데 책의 둘 뒤, 웅성거림 앞; 그 조건들은 경외를 읽지 않아 말하기 수의 차례만 정해진다 |
| `enc_witness` | `DT_Encounters`, `[B-6b8]`(결정 20) | 설계의 이름(`crisis_and_transcendents.md:27`, `pillars.md:76`, `docs/50:274`). 한 성계에 위기는 하나라 위기 카드 다섯은 함께 서지 않는다(`Events/CosmicPetitions.h:161`) |
| `log_record_extinct`, `log_death_extinction`, `log_fallen_bound` | `DT_LogTemplates`, 고리와 함께 `[B-6b2]`(글 `[B-6b5]`) | 절멸 만료의 기록 줄(결정 5), 그 죽음의 줄(§2.2 — `DeathRowOf`, `Player/PlayerRoleSubsystem.cpp:87-90`), 묶임의 줄(§2.5). 상수 셋(`Data/CosmicRowNames.h:91-103`의 꼴), `required_rows`(`Tools/data/schema.json:304`); 행이 없는 동안 밀리지 않는다 |
| `log_siege_on_us_far`, `log_treaty_broken_far`, `log_siege_lost_far`, `log_siege_held_far`, `log_siege_lifted_far`, `log_stance_lapsed_far` | `DT_LogTemplates`, `[B-6b3]`(결정 19) | 이름 없는 변형(§2.9) — 상수 여섯, `required_rows`. 넷째 칸 전에는 고르지 않는다 |

`DT_Dedications`·Covenant·`Hazard.SubspaceErosion`의 행은 없다(결정 3).

### 3.2 스트링 테이블 (`ST_CA.csv` — 새 키는 끝(`:447` 뒤)에만; 초안은 들이는 커밋, 감사는 `[B-6b5]`)

고치는 키 셋 — `Chronicle.RuleRung`(`:146`, 지금 읽지 않음 — 두 끝에 같은 다섯째 칸의 규칙, B6a 결정 6-마)·`Chronicle.Until`(`:148` — '늦어도')·`Epitaph.Peak`(`:406` — 결정 30) — 과 새 키 열넷(`Rung.Treaty`·`Rung.Laws`·`Rung.LawsLeft`·`Rung.LawsReset`·`Rung.NoFallen`·`Start.Block.NoFallen`·`Start.Suffix.NoFallen`·`Start.BindHint`·`Panel.ExtinctionFollows`·`Panel.RulerLethal`·`Edict.Wide`·`Edict.TipWide`·`Diplomacy.TreatyRungTip`·`Settle.Block.RealmCapLast`)의 초안은 노트 §1.4의 표다. 쓰는 자리는 §4. `Start.Block.NoFallen`·`Start.Suffix.NoFallen`은 열거와 같은 `[B-6b2]`에 든다(패널의 `static_assert` 표 `UI/CosmicSystemPanelWidget.cpp:86-119`).

### 3.3 스키마 (`Tools/data/schema.json`·`Tools/cosmic_data_schema.py`·`Tools/test_data_schema.py` — 규칙마다 깨지는 경우 하나)

`[B-6b2]`(열거·칸과 함께):
1. **조건 셋**(`:1099`의 꼴): `CivHasTreaty` — Tag 뿌리 `Treaty.`(없어도 된다), Value 1 이상 정수(깨짐: Value 0, Tag `Stance.Friendly`); `CivLawsHeldTicksAtLeast` — Key 숫자(깨짐: `N`); `CivPressureEnduredTicksReached` — Key 숫자, Named·SettledSystem 이벤트 행과 위기의 `SpawnConditions`에 싣지 못한다(`CivFallenSphereTicksReached`의 `:466`·`:471`·`:732`를 베낀다; 깨짐: Named 행).
2. **`civ_selector`에 `Ruled`**(`:35`): `NameNotable`·`CivHasNotable`의 Key(`:1124`·`:1098`)는 `Former`만 받게 따로 묶는다(깨짐: `NameNotable` Key `Ruled`). `Ruled`를 받는 조건은 `SystemOwnedBy` 하나다(§2.7 — 그 Key 규칙 `:1080`을 넓힌다).
3. **`Borders` 키**(결정 22의 답 ㉮): `ChangeReverence`의 Key는 비거나 `Borders`; 틱마다 내는 칸(칙령의 `EffectsPerTick` `:1050`)은 `Borders`를 싣지 못한다(깨짐: 칙령의 틱당 `Borders`).
4. **`DT_Edicts.Scope`**(새 정의 UENUM `ECosmicEdictScope` — `People`·`HomeGalaxy`; `schema.json`의 `enums`에 `"ECosmicEdictScope": ["People", "HomeGalaxy"]`(`Data.Integrity`가 반사의 UENUM과 견준다 — `Tests/CosmicDataTests.cpp:421-430`), 열거 칸에는 빈 값이 없어(`Tools/cosmic_data_schema.py:1188-1191`·`:1366-1369`) 지금 일곱 행에 `"Scope": "People"`을 적는다 — C++ 기본과 같아 비트 그대로): `HomeGalaxy`면 넷째 칸의 행(깨짐: 셋째), 몫의 곱 1 이하(깨짐: 1.5), 두 지도에 `Hazard.Claim`·`Hazard.Extinction`·`Hazard.PreSupernova` 키 없음(깨짐: 주장의 +100 — W가 그대로인 까닭, 결정 9), 어느 `RulerResolutions`에도 없음(깨짐: 답인 법칙 — 표를 가로지르는 검사는 못 하면 `Data.Integrity`).
5. **절멸 행**: 만료의 효과 세 칸이 비고 `BranchChance` 0 — 잃음은 코드의 고리(§2.2; 깨짐: `OnExpireEffects`의 `KillPlayer`); `RulerResolutions`는 `DT_Megaprojects` 행만(깨짐: 칙령 답).
6. **`DT_Feats`**: 통치자 행은 `TranscendentSealed`를 싣지 않는다(결정 12 ㉱ — `SinceTick` 0인 AI에게 남의 묶음이 공짜, #73; 깨짐: 넷째→다섯째 행). 그릇 규칙의 'why'(`:818`) '…decided with that row (#1)'을 '칸은 서로 따로다 — 어느 통치자 행도 그릇을 읽지 않는다(#1 닫음, A3)'로(규칙은 그대로).
6′. **`DT_Feats`의 `CivHasTreaty`는 Tag를 싣지 않는다**(AI의 걸음은 불가침만 맺는다 — §2.3; 깨짐: Tag `Treaty.Trade`).
7. **프로젝트의 `Causality` 값은 0**(수입이 없다 — #90; 깨짐: 인과 1).
8. **새 로그 행**: `log_*_far` 여섯은 `{OtherCiv}` 없음, `log_record_extinct`는 `Log.Record`의 규칙(`:349`), `log_death_extinction`은 죽음 줄의 규칙(`:354-357`)을 함께 받는다; `log_fallen_bound`는 `slots_in` ["System"](백성·지은 것의 이름 없음 — §3.5의 4).
8′. **`DT_Encounters`**: 조건에 `Hazard.Claim`을 싣는 행은 `ResolveHazard`를 싣지 않고, 주장의 `DelayHazard`는 `petition_pressure_measured`만(§2.8 — 신정론의 셈; 지금 데이터가 지킨다; 깨짐: 주장 카드 선택에 `DelayHazard` 50).

`[B-6b5]`(낱말 — 결정 33):
9. **새 목록 둘**: `words.rung_grasp_words` = ['영겁','왕 노릇','질서의 화신','초월자가 되','신이 되','일체화'] (`docs/70:129`, B6a 결정 6-나), `words.title_words`(열한 — 선택받은·택함·기름 부은·구세주·메시야의 무리; 낱말은 노트 §1.6, 신호 9 — `docs/90_LORE/theology_and_bestiary_notes.md:243`). 검사는 글자 조각이라 '신이 되'가 '자신이 되'·'당신이 되'도, '택하신'이 '선택하신'도, '택함을 받은'이 '선택함을 받은'도 막는다 — 받아들이고 글을 고친다(지금 데이터·`ST_CA.csv`에 0건, 전수).
10. **`DT_Feats`의 글 칸 셋**(지금 `verdict`만 — `:771-772`·`:782`): `holy_words`·`realm_names`·`rescue_words`·`afterlife_words`·`holy_and_myth`와 새 둘; `TargetTier Tier5_EternalOrder` 행은 행 규칙으로 `holy_passive`도(그 줄이 끝의 쪽 바로 앞 — B6a 헤더 §3.6의 1). 옛 세 행은 지난다(전수).
11. 새 둘을 `DT_Megaprojects.CompleteTemplates`(`:763`)·`DT_Hazards`와 `DT_Events`의 줄·`DT_LogTemplates.Templates`·ST_CA `Chronicle.`·`Epitaph.`에도 건다.
12. **검사하는 접두어 둘**(지금 `:64-89`에 없다): `Rung.`(`verdict`·`holy_words`·`soul_verbs`·`realm_names`·`holy_passive`·새 둘)과 `Sight.`(`verdict`·`holy_words`·`realm_names`·새 둘), 둘 다 숫자 없음. 검사 밖 접두어의 새 키(`Start.`·`Settle.`·`Panel.`)는 키마다 항목(`:88-89`의 꼴).

13. **신학 리뷰의 새 목록 다섯**(목록마다 깨지는 경우 하나 — 낱말·깨지는 경우는 노트 §1.6): `binding_words`(다섯째 칸 행의 글·`Rung.` 키·`Start.BindHint`·`Chronicle.Note.`(까닭 줄)·결박의 완성 줄 — 묶임은 이야기의 줄 둘만), `triumph_words`(`log_fallen_bound`와 그 완성 줄), `stay_words`(`death_frame_words`와 함께 절멸 행의 세 칸·`log_record_extinct`·`log_death_extinction`), `self_rescue_words`(방주의 완성 줄·절멸의 해결 줄·`feat_passage_by_ark`), `dawn_words`(`event_era_endured`).
14. **`project_binding`의 `DisplayName`**에 '탑'·'둘레'·'사슬'·'열쇠' 없음(§3.5의 4; 깨짐: '둘레 탑').

깨지는 경우는 목록·자리마다 하나(업적 제목 '영겁의 …', 서사 '선택받은 백성'·'구세주'·'메시야', `Rung.` 키의 '하나님', 다섯째 행의 '때가 찼다').

### 3.4 노브 (`DT_Rules`; 노브를 더하는 커밋이 `docs/30`을 `python -I Tools/make_config_reference.py`로 다시 만든다)

- **`GalaxyLawsToHold`** int 3(결정 10) — 법칙을 지킨 틱이 세이는 고향 은하의 칙령 수; 1 이상 정수. 읽는 곳: 경제 단계의 셈(§2.4의 순수 함수), 툴팁의 `Laws` 조각, 통치자 정책의 법칙 걸음. `Data.Integrity`가 '≤ 자리의 끝(1 + `EdictSlotSocietyMarks`의 수 = 3)'을, `CivLawsHeldTicksAtLeast`를 싣는 행이 있으면 '≤ `HomeGalaxy` 행 수'도 본다(행이 없는 `[B-6b2]`~`[B-6b6]`에서 깨지지 않게) — 넘으면 넷째→다섯째 행이 닿지 않는다(변이 하나).
- **`AIUpperRungProgressMultiplier`** float 1.0(결정 17 — 무엇에 곱하는지는 §2.6): 범위는 `AIAscensionSpeedMultiplier`의 꼴(`:137`·`:240`), 값은 이진수로 떨어지게(0.25·0.3125·0.375·0.4375·0.5 — §5.3 ②의 사다리). 읽는 곳 `CosmicMegaprojects::Advance` 하나 — #161 띠의 손잡이.
- **`FleetDelayCountdownShare`** float(결정 23) — `[B-6b2]`에 0(끔: 0 이하 = 몫의 상한 없음, 비트 그대로), `[B-6b8b]`에 0.5; 범위 0~1. 읽는 곳 `CosmicConflict::GuardDelayCapOf` 하나(`FleetDelayOf`와 신정론 불변식이 부른다). 상한과 W는 §2.10.
- 있던 `AIFourthRungCap` 2(`DT_Galaxies` 고향 행)가 처음 읽힌다 — 주석 `Data/CosmicDefinitionRows.h:768-772`(결정 16).

### 3.5 글의 규칙 (초안 — `[B-6b5]`의 글 감사가 정한다; 모든 줄에 규칙 11)

일곱 규칙(1 업적의 줄 · 2 넷째 행과 방주 · 3 절멸의 줄 · 4 묶음의 줄 · 5 법칙 · 6 시대의 줄 · 7 `enc_witness`)의 글은 노트 §1.6 — '§3.5의 N'은 그 번호다. 헤더에 남는 신학의 선(§0 '지키는 것', 목록은 §3.3의 9·13·14): 묶임은 행위자 없는 수동으로 이야기의 줄 둘에서만 말하고 조건·약속·승리의 말이 아니다(그늘 가드 포함); 절멸은 애도로 끝나고 희생·벌의 그림이 없다; 방주는 구한 이를 세우지 않는다; 시대의 이름은 사람·재위·새벽에 붙지 않는다; 증언의 라벨은 몸이 하는 일이고 물러남은 부끄럽게 하지 않는다(권장은 효과 없음 — 결정 20); 성경 구절은 대조한 것만(§8 글 감사의 눈).

## 4. 표현 계층 (`[B-6b4]` — 결정 25~32; #162 ②·③은 `[B-6b3]`, 건설 단추의 '다음 칸일 때부터'는 `[B-6b2]`; 모두 비트 그대로, 테스트는 §6)

줄마다의 자리·초안 글·까닭은 노트 §1.7(그 글이 이 절의 본문이다). 요지:
- **넷째 칸의 연출**(결정 25): 둘째·셋째와 같고, ×25는 영적 시야를 여는 연출 동안만 숨긴다; 다스리는 넷째 칸 성계의 맥동이 사라진다(`docs/20`에 적는다).
- **다섯째 칸은 금빛 없이**(결정 26): 순수 UI 함수 `LookOf` 하나를 미리 보기·테스트·게임이 읽는다 — 막·흰 제목·'칸 5'·서사·정지는 남고 빛무리·금빛 제목·종소리는 없다; 피드의 다섯째 줄은 Severity 2 그대로, 색은 금빛 그대로(흰색은 대안 — 표현만이라 비트 그대로); 다른 백성의 다섯째 칸에는 연출이 없다; `CosmicPreviewRung 5`는 `[B-6b7]`부터 새 행을 보인다.
- **규칙 줄과 경고**(결정 27): `ClockTipOf`가 넷째 칸에서 판정되는 행이 있으면 시계와 상관없이 `Chronicle.RuleRung`; 다음 칸이 다섯째일 때 `Diplomacy.SlotsAll`·`Settle.Block.RealmCapLast`.
- **다음 칸 툴팁**(결정 28): 부품 `Treaty`·`Laws`를 끝에, 방주는 '{0}에 답할 때', 결박은 `Project`(묶일 이가 없으면 `Rung.NoFallen`), 조약 단추의 `Diplomacy.TreatyRungTip`; `UI.NextRung`의 '다음 칸 없음'은 다섯째로.
- **#162**(결정 29): ① 먼 줄 세 변형을 어느 칸에도 참인 글로(`[B-6b5]`) ② 까닭 줄의 '늘 부르는' 예외를 `PeoplesOfEveryLife`로 ③ 머리 주석(②·③은 `[B-6b3]`).
- **묘비의 칸**(결정 30): `Epitaph.Peak`을 백성의 일로, 값이 첫째 칸이면 줄을 내지 않는다. **경주의 칸**(결정 31): [백성] 탭의 '칸 N'(`IsKnown`만 — 규칙 14).
- **패널의 줄**(결정 4·5·12): 절멸의 `Panel.ExtinctionFollows`·`Panel.RulerLethal`(`KillsTheRulerOf` — 새 정지 없음), 주장의 경고 `ClaimMayFollowOf`는 절멸에 늘 거짓(`[B-6b2]`); 방주는 지금의 답 단추; 건설 단추는 그 칸이 다음 칸일 때부터(`[B-6b2]`), 회색 사유 `NoFallen`·툴팁 `Start.BindHint`; 칙령 창의 `Edict.Wide`·`Edict.TipWide`.
- **봉인 고리**(결정 32 — #98 ② 닫음): 회색(Severity 0), 끊기지 않은 고리 그대로; 화면에 '인'·'봉인'이 없다; `docs/20:134`를 고친다.

## 5. 커맨드렛과 측정

### 5.1 새 줄 `race:`와 측정 전용 스위치 (`[B-6b3]` — 결정 34)

- **새 줄 `CosmicBalance race:`** — 러너가 글자로 견주는 세 줄(`Tools/run_checks.ps1:87-98`) 밖이다(B6a `ending:`의 꼴 — `Tests/CosmicBalanceCommandlet.cpp:2942`). 셈은 규칙을 베끼지 않고 게임 코드의 창구를 읽는다(`CosmicAscent::FourthRungOpenOf`·`IsJudgedRulerRowAt`·`CosmicMapLook::KnownViewOf`). 칸은 노트 §2.1 — 끝의 종류마다 술어가 처음 선 틱은 **기록과 따로**(기록은 먼저 온 끝 하나라 시계를 바꿔 다시 세는 입력), 결정은 **결정이 든 경계의 열(틱:수)을 판 끝까지**(창은 밖에서; 지금 `ending:`의 계절 결정은 기록이 없으면 −1).
- **주장마다 한 줄**(#142 — 결정 24): `<Out>_claims.csv`에 놓인 틱·끝난 틱·끝의 종류·카드 때의 남은 시간·지킴·결박 뒤에 끝남(`<Out>_rows.csv`의 꼴 — `:1378`).
- **`race check`**(깨질 수 있는 확인 — `end check` `:1600-1622`의 꼴): 넷째 칸에 처음 선 AI가 있는 경계마다 **앞 경계의 넷째 칸 이상 수 + 이 틱에 넷째 칸에 선 수(다스리는 백성 포함 — 판정이 먼저 본다) ≤ 고향 행의 상한**(상한 0이면 묻지 않는다; `Civs`는 건넘 단계에서만 줄고 칸은 내려가지 않아 앞 경계의 수가 판정 때의 수다). '넷째 칸 이상 수 ≤ 상한'만으로는 불변식이 아니다 — 다스리는 백성은 제동 없이 상한을 넘겨 오르고(`docs/21:1574-1580`) 섭정의 백성은 AI가 된다. 어긋나면 `UE_LOG(LogCosmic, Error, …)`와 `Result = 1`.
- **`-WithoutB6bData=1`**(측정 전용 — 자동화 테스트가 든 빌드에서만, 문은 `:1282-1308`의 꼴): `CosmicTests::WithoutB6bData`(`Tests/CosmicTestHelpers.h` — `WithoutDeath` `:268`의 꼴)가 B6b의 행을 모두 빼고(셋째 칸 위 업적 행 둘·`project_ark`·`project_binding`·절멸 위기 행·법칙 행 셋·시대 행·`enc_witness`) B6b의 노브를 끈 값으로 둔다(`AIUpperRungProgressMultiplier` 1·`FleetDelayCountdownShare` 0). 시계는 건드리지 않는다. **데이터를 켜는 모든 커밋의 짝** — 장치가 그대로인 동안 끈 쪽은 `[B-6b5]`의 다섯 줄을 글자 그대로 낸다.
- **`-FourthRungCap=<n>`**(같은 문): 고향 행의 `AIFourthRungCap`을 덮는다(`CosmicTests::WithGalaxies` — `Tests/CosmicTestHelpers.h:346`; `-Rules`는 `DT_Rules`만 — `:1101-1128`). 지연과 시계는 `-Rules`로.
- 두 스위치는 `-SaveAt`의 '아닌' 목록과 거절 줄(`:1241-1245`·`:2973`)에 더하고, 해시 판과 러너 인자에는 넣지 않는다.
- **`CosmicTests::WithoutDeath`는 데이터의 `KillPlayer`만 걷는다** — 절멸의 죽음은 코드라 남는다(결정 5). 그 도우미로 '죽음 없는 세계'를 뜻한 테스트(노트 §1.3 — `EndingTables`·`Sim.SightWalkMoved`·`Sim.SuccessionPossible`)는 `WithoutB6bData`를 함께 건다; 도우미 주석과 `docs/40`의 그 스위치 문장에 한 줄.
- 작은 고침(러너 밖): 영압 카드의 칸별 셈을 다섯 칸으로(`World/CosmicRulerPolicy.cpp:372`의 클램프 0~2). `docs/40:476`('엔딩의 줄') 곁에 이 절의 한 단락.

### 5.2 묶음 (결정 34~36)

B6a의 일하는 법 그대로(B6a 헤더 §5.2): **30건 이상일 때만 판단**, 모자라면 '재지 않음 — 잰 N'; 몫에는 95% 구간(Wilson); 판 단위의 수는 짝 차이; 원인은 탐침 뒤에만; 미리 셀 수 있는 것은 데이터 전에 노트 §2에; 같은 세계는 한 번만; 모든 로그에 `end check`·`race check` 줄 0. **A·B는 시계 없이**(`-Rules=ProvisionalEndTick:0` — A4): 기록은 세계를 바꾸지 않고(`Sim.EndingInert`) 정책은 기록 뒤에만 서므로, 시계 없는 판 하나가 어느 C의 끝도 준다(C 이하의 첫 다섯째 칸·침묵, 아니면 C의 시계). 꼴: R = `-Answer=1 -Build=1 -Focus=1 -Edict=1 -Diplomacy=1 -TellAll=0`, 느린 R = R의 `-Focus=0`, 방랑자 = R + `-Abdicate=1200 -Wander=1`, 입력 없는 판 = 인자 없이, 짝 = 같은 빌드·시드의 `-WithoutB6bData=1`. 시간은 B6a 꼴의 어림으로 A 약 10분·B 약 22분·C 약 30분(잰 값은 노트 §3).

| 묶음 | 커밋 | 칸(시드·지평) | 정하는 것 |
|---|---|---|---|
| **A** | `[B-6b6]` | R 30 × 20,000(1~30), 입력 없는 30 × 20,000(1~30), 방랑자 60 × 6,000(1~60), 각각 짝 | **깨질 수 있는 확인**: 짝마다 첫 셋째 칸 틱 이하의 CSV 행이 같다. **②의 첫 값**: 넷째 칸 틱의 분포, 제동이 막은 수, 절멸(방주가 250 안에 끝난 몫·지움·통치자의 죽음), AI 조약 — 이것으로 결박을 마지막 다리로 둔 경주를 노트 §2에서 다시 셈해 N·P_bind·r과 강제 죽음의 틱 t를 `[B-6b7]` 전에 적는다. **적기만**: AI 단계 µs/틱(B6a 10.4~10.7), B1 지킴(①만의 몫 — 귀속) |
| **B** | `[B-6b8]` — A4의 '장치 빌드'(장치와 데이터 모두, 시계 전) | R 60·느린 R 60·방랑자 60·입력 없는 60 × 20,000(1~60) 넷 모두 짝; **강제 죽음** R 60 + `-DeathAt=<t>`(t = A의 다스리는 넷째 칸 중앙값, 100으로 올림; 짝은 같은 시드의 R) | §5.3의 ①~④ |
| B′ | `[B-6b9]`(판단이 부를 때만) | B의 칸, 시드 61~120 | 같은 판단 — 한 번 고쳐도 어긋나면 그 수와 함께 [개발자] |
| **C** | `[B-6b11]`(시계 C의 빌드, 게임의 꼴) | R 60 × C; 느린 R 60 × C; 강제 죽음 R 60 × C; 방랑자 60 × C와 짝; 입력 없는 60 × 20,000과 짝; **시험 칸** — B4의 시험 묶음(R + `-Rules=FallenCoreMinLanesFromHome:1 -LeaveNearSphere=1` — `DECISIONS_LOG.md:68`)의 `-PlayerPolicy=Faithful -Guard=1` 140 × 20,000(1~140) | §5.3의 'C에서 읽는 것' |

묶음 B는 `[B-6b8b]` 전의 빌드다 — 지킴의 노브는 정책이 지킨 성계의 각성 위기에만 닿아(R은 `-Guard=0` — `World/CosmicRulerPolicy.cpp:686-720`) 경주를 거의 움직이지 않지만, C에서 읽는 것은 마지막 빌드의 묶음 C가 다시 돈다.

### 5.3 미리 적은 판단과 시계의 규칙 (결정 35 [개발자] — 데이터를 켜기 전에 이 글 그대로 고정한다)

**묶음 B의 판단**(어긋나면 `[B-6b9]` — 탐침은 `-Rules`(r)·`-FourthRungCap`, 행의 값(c·P_ark·P_bind·N)은 커밋하지 않은 데이터 사본으로; 탐침의 판은 판단에 쓰지 않는다 — 탐침은 묶음 B의 시드 1~60으로 하고 B′의 판단과 시계 규칙 ⓐ(B′일 때)는 새 시드 61~120으로 한다(같은 시드의 B′는 결정론으로 탐침의 판 그 자체다)):

| # | 판단 | 칸 | 규칙 | 어긋나면 |
|---|---|---|---|---|
| ① | 다섯째 칸에 닿는다 | R 60 | 기록이 집단 승천이나 다른 질서이고 그 틱이 20,000 이하인 판(노트 §2 ⓗ) ≥ 40/60 | 늦은 다리부터 — 다스리는 넷째 칸이 늦으면 c를 올리고, 넷째 → 다섯째가 길면 N·P_bind를 내린다 |
| ② | '다른 백성이 먼저'의 띠(#161) | R 60의 다섯째 칸 끝(≥ 30) | 다른 질서 15~35% — 분모는 ①의 판, 분자는 기록이 다른 질서이고 그 백성을 어느 생애도 다스리지 않은 판(다스린 적 있는 백성의 다른 질서 — `ending:`의 '어느 생애가 다스린 백성' — 는 분모에 두고 따로 적는다). 점이 띠 밖이면 어긋남; 95% Wilson 구간은 함께 적는다(n = 60에서 구간이 통째로 띠 밖인 것은 ≤ 3/60·≥ 29/60). 다른 질서의 판은 (가) 다른 백성이 결박을 마친 틱에 다스리는 백성이 넷째 칸 이상(r이 닿는 경주)과 (나) 다스리는 백성이 절멸로 지워졌거나 넷째 칸에 닿지 못함(c·P_ark의 몫)으로 갈라 적고, r을 옮기는 판단은 (가)의 몫으로만 한다. 느린 R 60은 같은 셈으로 몫과 구간을 적어 결정 35의 답대로 다룬다(권장: 알린다 — 구간이 통째로 35% 위면(모형 63% — 노트 §2 ⓒ) 시계 규칙의 받아들임과 함께 [개발자]에게 보인다, 시계의 까닭이 사람의 걸음을 느린 R에 두므로; 위 끝을 두기로 하면 그 값은 개발자가 정한다) | 35% 위면 r을 한 칸 내리고 15% 아래면 한 칸 올려 B′로 한 번 — 사다리는 이진수로 정확한 1/16 걸음 0.25·0.3125·0.375·0.4375·0.5(시작값 0.375; 모형으로 R의 다른 질서 9%·17%·26%·34%·43% — 노트 §2 ⓒ); B′의 점도 밖이거나 사다리 밖이 들면 그 수와 함께 [개발자] |
| ③ | 넷째 칸의 계절 | ①의 판 가운데 다스리는 백성이 다섯째 칸에 선 판(≥ 30) | 넷째 → 다섯째 틱의 중앙값 ≤ 3,600(아래 끝 1,200은 구성상 지켜진다 — 계절 ≥ max(P_bind, N); `[B-6b9]`가 둘 다 1,200 아래로 내릴 때만 묻는다); 그 계절 600틱 창마다 결정 — 판마다(창마다 결정 수의) 중앙값 ≥ 1인 판 ≥ 90%(B6a 묶음 0·1의 ⓑ는 모든 판으로 판단했다 — 30/30·28/28; 창은 노트 §2.1) | 길이는 N·P_bind; 결정 밀도는 값으로 고치지 않는다 — [개발자] |
| ④ | 시계 C | 느린 R 60 | 아래 규칙 | 규칙이 정한다 |

**시계의 규칙**(`[B-6b10]`이 이 글을 `DECISIONS_LOG`에 그대로 옮긴다):

> ⓐ **칸**: 느린 통치자 판(R의 `-Focus=0`) 시드 1~60(값을 고쳐 B′면 61~120), 시계 없이, 지평 20,000, 묶음 B(값을 고쳤으면 B′)의 빌드. ⓑ 판마다 **E** = 첫 다섯째 칸 끝(집단 승천이나 다른 질서)의 틱 — 시계가 없으므로 그 종류의 기록의 틱; 20,000 안에 다섯째 칸이 없거나 침묵이 먼저 선 판은 E = ∞. ⓒ **C₀** = E의 80번째 백분위수 — 60판을 올림차순으로 놓은 48번째 값(가장 가까운 순위)을 1,000의 배수로 올림(∞면 ∞). ⓓ **C = min(18,000, max(6,000, C₀))**. ⓔ **받아들임** — 둘 다: (i) E ≤ C인 판 ≥ 30/60; (ii) 다스리는 셋째 칸의 틱부터 그 판의 첫 기록(침묵 포함)과 C 가운데 이른 쪽까지 600틱의 온 창마다 결정 수의 판마다 중앙값 ≥ 1인 판 ≥ 90%(셋째 칸에 닿지 않은 판은 빠진다; 셈은 노트 §2.1). 하나라도 어긋나면 값을 적지 않고 그 수와 함께 [개발자]에게 묻는다. ⓕ C는 6,000에 끝난 B6a 세이브를 다시 열지 않는다 — 끝은 기록이 정한다(`SaveVersion` 1 그대로, 이주 없음 — A4).

- **C = 6,000이면**(ⓓ의 바닥 — 모형의 아래 끝이다): `[B-6b10]`은 노브를 옮기지 않는다 — 다섯 줄·해시 판 둘·6,000에 박힌 테스트(`Sim.EndingOf` `Tests/CosmicEndingTests.cpp:621`·`UI.RunEnd`)는 그대로이고, 커밋은 규칙과 잰 수를 `DECISIONS_LOG`에 옮기고 '잠정' 문장(`Data/CosmicDefinitionRows.h:1108`의 머리 줄·`:1114`, `docs/30`, 6,000을 말하는 문서의 표지 — 노트 §1.2)과 `Sim.SaveOldEndFlows`의 더한 경우(C와 상관없이 — §6)만 든다. §5.4(노트 §2.2)의 `[B-6b10]` 행과 §6의 시계 테스트 고침은 C > 6,000일 때만이다.
- (i)은 C가 18,000에서 잘릴 때만 문다(C₀ ≤ 18,000이면 48판이 이미 안이다). 18,000은 비전의 한 판(`docs/00_VISION.md:103`); 바닥 6,000은 B6a가 잰 계절보다 짧아지지 않게 하고 시계를 따르는 테스트의 문(C > 0 — §6)을 지킨다.
- **느린 칸인 까닭**: 마일스톤 B의 끝은 사람이 엔딩을 보는 일이고(B6a 결정 21) 사람의 걸음은 스크립트보다 느린 쪽에 가깝다고 놓는다 — 다스리는 셋째 칸은 R 약 2,366·느린 R 약 4,384(B6a 묶음 1 — 노트 §0.1), 비전의 호로는 약 5,400(`docs/00_VISION.md` §2.4), 중점을 쓰지 않는 판 4,425(`DECISIONS_LOG.md:139`).
- **적기만과 C에서 읽는 것**(묶음 B의 적기만과 그 두 문턱 — AI 단계 µs/틱 +50%·결박한 성계를 잃어 닫힌 판 R 60의 3판; 묶음 C의 #136·#142·#132·B1 지킴·#129·#154·'< 15%'·시계 아래의 끝 — 알리되 C의 문이 아니다): 글은 노트 §2.3이다(그 글이 이 절의 본문이다 — 데이터를 켜기 전에 이 절과 함께 고정한다).

### 5.4 옮을 기준값 (`Tools/baselines/seed42.json`)

커밋마다 옮을 기준값과 그 깨질 수 있는 확인은 노트 §2.2의 표(C = 6,000일 때의 `[B-6b10]`은 §5.3). `[B-6b11]`·`[B-6b12]`은 옮기지 않는다(노브를 바꾸면 그 커밋이 옮긴다). 옮은 줄은 `-UpdateBaseline`로 다시 적고 `note`에 까닭과 새 해시(B6a §5.3의 꼴); 쿠킹한 빌드의 해시 비교는 마일스톤 B의 끝에 한 번.

## 6. 테스트 (새 단언은 변이로 한 번씩 깨뜨린다 — 패치는 `Tools/mutations/B6b/b6b<N>_<이름>.patch`, README에 한 줄씩; 깨끗한 판이 먼저 통과하는지 본 뒤에)

저장 왕복은 smoke 필터에 들도록 `Sim.Save` 아래(`Tools/run_checks.ps1:59`). 장치의 테스트는 테스트 표에서 돈다(게임 데이터에는 `project_binding` 말고 아직 행이 없다).

커밋마다의 단언과 그 변이는 노트 §1.3의 첫 목록(그 글이 이 절의 본문이다) — 이름만:
- **`[B-6b1]`** — `Sim.SaveLawsHeld`·`Sim.HashCoversNewCounters`·`Sim.PressureEndured`.
- **`[B-6b2]`** — `Sim.ExtinctionExpiry`·`Sim.ArkAnswers`·`Sim.UpperRungDelay`·`Sim.FourthRungBrake`·`Sim.LawsHeld`·`Sim.GalaxyLaws`·`Sim.AIRowSteps`·`Sim.EdictAI`(고침)·`Sim.Binding`(같은 틱의 두 결박 포함)·`Feats.Evaluate`(더함)·`Sim.EraSelector`·`Sim.FleetDelay`(고침)·`Data.Integrity`(신정론·법의 수·방주의 여유)·`Events.CheckIsPure`(고침 — 반사로 열거자를 센다)·`Sim.StartBlockReasons`·`UI.StartReasons`(고침)·`UI.TopBarAndPanel`·`Calling.DeathPipeline`·`UI.ClaimMayFollow`·`Sim.RulerEdictPolicy`, 스키마 규칙마다 깨지는 경우.
- **`[B-6b3]`** — `Knowing.FarAbove`·`Sim.CrownFarAbove`·`Log.FarNameless`·`UI.Chronicle`(고침)·`UI.WandererPanel`.
- **`[B-6b4]`** — `UI.AscensionOverlay`(고침)·`UI.Chronicle`·`UI.RunEnd`·`UI.NextRung`·`UI.PeoplesTab`(고침)·`UI.TopBarAndPanel`·`UI.StartReasons`·`UI.Epitaph`(고침)·`UI.ThirdRungSpeed`·`UI.EdictWindow`.
- **`[B-6b5]`** — 변형 수(커밋 메시지의 한 줄)·새 낱말 목록과 접두어마다 깨지는 경우. **`[B-6b6]`** — `Data.Integrity`(코드의 죽음의 핀 — 결정 5).

**데이터를 켜면 뜻이 바뀌는 테스트와 시계에 박힌 테스트**: 표는 노트 §1.3 — 데이터 커밋(`[B-6b6]`~`[B-6b8]`)마다 그 표의 테스트를 돌려 보고 고치며, 시계 커밋 `[B-6b10]`은(C > 6,000일 때 — C = 6,000이면 §5.3) `Sim.EndingOf`·`UI.RunEnd`를 C로 옮기고 `Calling.LeapDeterminism`에는 시계 6,000을 제 값으로 둔다. `Sim.SaveOldEndFlows`(있는 테스트에 한 경우를 더함 — C와 상관없이): 시계 6,000의 기록을 지닌 세이브를, 시계를 7,000으로 덮은 규칙(`Tests/CosmicEndingTests.cpp:138`의 `EndingClockAt`을 `CosmicTests` 도우미로 옮겨 쓴다)으로 열어도 끝난 채다(변이 — 래치가 노브와 견주면 깨진다 — A4).

## 7. 문서

자리와 고칠 말은 노트 §1.1(장치 커밋의 주석)·§1.2(이 절의 원문과 문서 커밋의 목록). 차례:
- **코드보다 먼저**(`[B-6b1]` — 필드와 한 커밋; '틱 순서·저장 필드를 바꾸면 docs/20을 먼저'): `docs/20:286`(먼저 `CLAUDE.md` 규칙 4의 B2·B3 괄호를 이 전문으로 옮기고 새 자리를 적는다 — 두 셈·결박의 완성·절멸 만료의 차례·`Place`의 덤인 은하 칙령의 갈고리·AI의 두 걸음과 제동; 결정 38)·§9.1·§9.2(새 시뮬 추첨 자리 없음); `docs/21` §7.2·§7.3·§7.4·§7.5; 설계(`crisis_and_transcendents.md:134`·`:24`·`:30`·`:121`, `systems.md:65`·`:162`·`:166`·`:169`, `docs/50:231`); `CLAUDE.md` 규칙 4·13·14의 예정 표지(`[B-6b3]`에서 뗀다).
- **장치의 두 커밋**(`[B-6b2]`·`[B-6b3]` — 주석은 그것이 말하는 코드를 바꾸는 커밋에). **`docs/30` 다시 만듦**(러너의 문서 검사 — 생성기는 노브의 값과 노브를 이름으로 읽는 시험 밖 소스 파일을 싣는다, `Tools/make_config_reference.py:20-37`): DT_Rules의 값이나 노브를 이름으로 읽는 소스 파일을 바꾸는 모든 커밋 — `[B-6b2]`(새 노브 셋), `[B-6b4]`(툴팁이 `GalaxyLawsToHold`를 이름으로 읽으면), `[B-6b7]`(r의 첫 값), `[B-6b8b]`·`[B-6b9]`·`[B-6b10]`. `docs/40`: 커맨드렛 절에 `race:` 줄과 두 스위치(`[B-6b3]`).
- **표현과 글**: `UI/CosmicNextRung.h:56-57`·`docs/20:75`·#104의 '첫 행만'은 `[B-6b4]`; 연대기 위젯 헤더의 낡은 주석(결정 29)은 그 코드와 `[B-6b3]`.
- **시계**(`[B-6b10]`): `docs/30`, 6,000을 말하는 현행 문장(`docs/20:89`, `docs/50:12`, `docs/00_VISION.md:20`·`:32`) — 결정 35(C = 6,000이면 §5.3).
- **문서 커밋**(`[B-6b12]` — 끝에 재감사 워크플로): `docs/50`(B6b 칸, Phase 4 목록 `:197-198` — 절멸의 행·훅·방주만 B6b로, 방랑자의 '피난 호위 조우 체인'은 그 항목에 남긴다(#164), §12.1의 수량 — 결정 40), `WORKPLAN.md:56`·`:61`, 설계의 나머지, `docs/15`·`docs/70`·`docs/40:480`, `OPEN_QUESTIONS`·`DECISIONS_LOG`(결정 39)·`docs/CHANGELOG.md`·`STATUS.md`, `CLAUDE.md` 규칙 15 'A0~B6b'; 코드 주석의 나머지(헤더 파일이면 에디터를 닫고 전체 빌드).

## 8. 순서·검증·effort (`WORKPLAN.md:61`의 B 행)

| 커밋 | 하는 일 | effort | 검증 |
|---|---|---|---|
| (헤더) | 이 파일과 노트 | xhigh | `python -I Tools/check_docs.py` 0; 150,000바이트 안; **ultracode 적대 리뷰** — 관점 다섯(설계·결정론·신학·측정·빌드 — 노트 §4)과 고친 뒤 확인 |
| `[B-6b1]` | **필드와 문서 먼저**: `LawsHeldTicks`(필드만 — 셈은 `[B-6b2]`, §2.1)·`PressureEnduredTicks`(셈과 `MoveCapital`의 되돌림 — 곧 센다); `docs/20`(틱 순서·셈·훅·AI 걸음·추첨 장부)·`docs/21` §7.2·§7.3·§7.4·§7.5; `CLAUDE.md` 규칙 옮김(결정 38) | xhigh | 에디터를 닫고 전체 빌드; 러너 full 두 번; §5.4(두 해시가 옮고 CSV는 같다), 새 해시를 커밋 메시지에; **ultracode 리뷰**(저장 호환·해시·문서 먼저 감사) |
| `[B-6b2]` | **장치 — 세계**: 열거자 셋·법칙 틱의 셈·`ECosmicEdictScope`와 `Scope` 칸·`NoFallen`·`LetPeopleGo`와 절멸의 훅·결박의 창구와 완성 훅·은하 칙령의 셈·`FourthRungOpenOf`·AI의 두 걸음·`AIRevokesOf`의 예외·지연 노브 1·`GuardDelayCapOf`(몫 0)·`Ruled`·`Borders`(결정 22의 답 ㉮)·`KillsTheRulerOf`·로그 행 셋과 `project_binding` 행(시작값 — `required_rows`, §3.1)·건설 단추의 '다음 칸일 때부터'(§4)·스키마 규칙 | xhigh | 에디터를 닫고 전체 빌드; 러너 full `-Data` 두 번; **비트 그대로**; `docs/30`; **ultracode 구현 리뷰**(틱 순서·추첨·단일 창구·제동) |
| `[B-6b3]` | **장치 — 앎과 측정**: Core 격차 술어와 부르는 넷, 즉위의 `FarAbove`, 이름 없는 줄, 까닭 줄의 예외, §5.1 전부 | xhigh | 에디터를 닫고 전체 빌드(`Core/CosmicKnowing.h`·`ECrownBlock`·`Tests/CosmicTestHelpers.h`·`World/CosmicRulerPolicy.h`); 러너 full `-Data` 두 번(`log_*_far` 여섯); 비트 그대로(`-TellAll=0` 포함); 변이; **ultracode 구현 리뷰**(규칙 14 — 앎을 읽는 자리) |
| `[B-6b4]` | **표현**(결정 25~32) | high | 러너 full `-Data` 두 번; 비트 그대로; UI 테스트; `docs/30`(툴팁이 노브를 이름으로 읽으면); 창 없이 찍은 화면 — 다섯째 칸의 꼴만(행의 글은 `[B-6b7]` 전까지 첫 행의 것), 새 행이 서는 화면은 `[B-6b8]` 뒤 손 확인 ⑥과 함께; **ultracode 화면·문구 리뷰** |
| `[B-6b5]` | **글과 목소리**: 먼 줄 셋, 새 ST_CA 키, 새 행 모두의 감사한 초안 — 데이터 행(절멸·방주·업적 둘·법칙 셋·시대·`enc_witness`)의 글은 노트 §1.5에 감사한 그대로 두고 그 행을 켜는 커밋이 글자 그대로 옮긴다(새 낱말 목록은 그 커밋의 `check_data`가 다시 본다); 이 커밋에 드는 것은 비트 그대로인 글(`[B-6b2]`·`[B-6b3]`부터 있는 결박의 행과 로그 행 아홉 포함)과 새 낱말 목록뿐; **ultracode 글 감사**(결정 33) | high | 러너 full `-Data` 두 번; 변형 수 그대로; 비트 그대로 |
| `[B-6b6]` | **데이터 ① 셋째 → 넷째**: 절멸 행·`project_ark`·업적 행, 결정 5의 테스트 → 묶음 A | high | 러너 full `-Data` 두 번; §5.4 ⓐ~ⓒ; **ultracode 밸런스 리뷰** |
| `[B-6b7]` | **데이터 ② 넷째 → 다섯째**: 법칙 행 셋·업적 행·`project_binding`의 값, N·P_bind·r의 첫 값 | high | 러너 full `-Data` 두 번; §5.4; `race check` 0; `docs/30`(r의 첫 값) |
| `[B-6b8]` | **데이터 ③ 곁 행**: 시대 행·`enc_witness` → 묶음 B와 §5.3의 판단 | high | 미리 적은 판단 |
| `[B-6b8b]` | #132 몫 0.5와 W를 다시 셈 — 한 커밋 | high | `Data.Integrity`; `Sim.FleetDelay`에 각성(150 → 225, 함대 3) |
| `[B-6b9]` | **값만**(판단이 부를 때) → B′ | high | 같은 판단 |
| `[B-6b10]` | **시계**: §5.3의 규칙으로 C, 박힌 테스트, `docs/30`, 6,000의 언급(노트 §1.2) | high | §5.4(3,000 두 줄·해시 판 둘 비트 그대로); C = 6,000이면 §5.3 |
| `[B-6b11]` | **다시 잼(묶음 C)** — 노브를 바꾸면 제 커밋 | high | 미리 적은 규칙으로 알린다 |
| `[B-6br]` | 리뷰의 고침이 크면 그 고침만 | 고침대로 | 그 고침의 검증 |
| `[B-6b12]` | **문서**(§7) | xhigh | 러너 full 두 번(비트 그대로); **재감사 워크플로** |

- **모든 커밋의 바닥 검증**: `Tools/data`의 JSON이나 `ST_CA.csv`를 고치는 커밋(`[B-6b2]`~`[B-6b10]` 가운데 데이터에 닿는 모두)은 러너 full `-Data` 두 번(`Data.SourcesInSync` — `Tests/CosmicDataTests.cpp:1101-1116`, `Tools/run_checks.ps1:47-52`), 헤더를 고치는 커밋은 에디터를 닫고 전체 빌드 — 표의 검증 칸은 그 위에 더한 것만 적는다(B6a §8의 꼴).
- **해시를 옮기는 새 길과 데이터 켜기는 한 커밋에 두지 않는다**(`WORKPLAN.md:61`; B3 `[B-3d1]`·`[B-3d2]`): 새 길은 필드 `[B-6b1]`·지킴의 노브 `[B-6b8b]`·시계 `[B-6b10]`, 데이터는 `[B-6b6]`~`[B-6b8]`, 값은 `[B-6b9]`. `[B-6b2]`~`[B-6b5]`의 비트 그대로가 '행이 없으면 아무것도 하지 않는다'의 증거다. 데이터를 셋으로 나누는 까닭: 묶음 A의 수가 ②의 값을 정하고, B1 지킴의 짝이 옮김을 ①과 ③에 나눠 줄 수 있다.
- **구현 리뷰의 고침이 기준값을 옮기면** 장치 커밋에 넣지 않는다 — 행 없이 세계가 움직였다는 뜻이라 까닭부터 찾고, 고침은 `[B-6br]`에 옮은 줄·까닭과 함께(B6a `[B-6a3r]`의 꼴). 푸시한 커밋은 고쳐 쓰지 않는다.
- **글 감사의 눈**(`[B-6b5]` — 발견마다 반박 검증, 빠진 것을 찾는 비평 하나): ① 신학 — 결박이 행위자 없는 수동('…|hpp(이,가) 묶였다')이고 승리·조롱이 없는가, 그늘 가드(`DECISIONS_LOG.md:79` — 결박의 줄과 그 앞 완성 줄)를 지키는가, 고친 은혜 행 `event_grace_remnant`의 변형이 두 길에서 참인가(§2.2), 절멸의 죽음이 남을 위한 희생(메시아 신호 2)으로 읽히지 않는가, 다섯째 칸이 얻은 승천(신호 11)으로, 시대의 줄이 그릇의 칭호·경배(신호 9·10)로 읽히지 않는가(`docs/90_LORE/theology_and_bestiary_notes.md:236-245`); '대조 필요'인 구절(계 20:1-3·막 3:27·계 7:3·벧전 3:20-21·행 4:20·요 3:11·계 1:19·요 1:41, `docs/70:185` ⑨의 계 20:2-3)은 설계 문서에 넣기 전에 대조 도구로 확인해 목록(`theology_and_bestiary_notes.md:259`)에 더하거나 뺀다(B5 헤더 `:450`의 규칙) ② 한국어 교회 말투 ③ 모든 길 — 보이지 않는 백성의 다섯째 칸, 섭정 아래의 다섯째 칸, 그릇 없는 결말, 결박이 막힌 판 ④ 첫 독자 — 절멸의 경고가 공포로, 경주가 성적표로 읽히지 않는가.
- **일하는 법**: B6a §8 그대로(도우미가 먼저 — `WithoutB6bData`는 `[B-6b3]`; 코드가 부르는 행은 그 코드와 한 커밋). **크기**: 150,000바이트 안 — 승인 때 147,000 아래(적대 리뷰의 고침과 그 확인과 함께 표·목록·긴 본문을 노트 §0.4·§0.5·§1.2·§1.3·§1.6·§1.7·§2.1·§2.2·§2.3으로 옮겼다); §11은 커밋마다 한 줄(400바이트 아래 — 세부는 노트 §5); 149,000을 넘기는 커밋은 같은 커밋에서 그만큼을 노트로 뗀다(B6a의 헤더는 구현 동안 11,450바이트 늘었다).

## 9. 손 확인 (B6b의 종료 조건과 마일스톤 B의 끝 — 개발자, 콘솔 없이; B6a 결정 18·21의 꼴)

1. **B6a의 ①·③·⑨를 먼저**(B6a 헤더 §9): 셋은 시계 6,000과 `[B-6a6]`의 증거 세이브(`Saved\HandCheck\B6a_*`)에 기댄다 — **B6a의 빌드(HEAD `5455e56`)로, 늦어도 `[B-6b6]` 전에**(그때까지 세계와 시계는 같고 `[B-6b4]`·`[B-6b5]`의 글만 다르다). B1~B6a의 나머지도 같은 빌드로(`STATUS.md:30`). 블록을 막지 않는다.
2. **B6b의 세이브는 `[B-6b10]` 뒤 `-SaveAt`으로**(게임의 꼴 — R + `-Seed=<시드> -SaveAt=<틱>`, `-Rules` 없이). 시드와 틱은 묶음 C에서 골라 노트 §5에 명령 줄·기대 틱과 함께 적고, **마지막 코드 커밋의 빌드로 다시 만든다**. 에디터 `-game`의 '이어하기'로 연다.
3. **목록**: ① **집단 승천 — 마일스톤 B의 '엔딩 1회 콘솔 없이'**(B6a 결정 21): 다스리는 백성이 다섯째 칸에 서는 시드의 다스리는 넷째 칸 − 50 → 법칙과 범위 줄, 툴팁의 조약·법칙 조각, 결박 사업과 그 줄·회색 고리, 금빛·종소리 없는 다섯째 칸의 연출 → 끝 쪽 → 연대기의 까닭 줄; 다섯째 칸이 쥔 오름(금빛·종소리·'신이 되었다')으로도, 거룩한 질서가 내린 상('맡겨졌다'·'허락되었다'·'때가 찼다')으로도 읽히지 않고 그 일 뒤에 백성이 서 있게 된 자리로만 읽히는가(신호 11, B6a §3.6의 1), 피드 다섯째 줄의 색(결정 26의 대안), 결박 줄이 행위자 없는 수동으로 읽히는가. ② **다른 질서**: AI의 다섯째 칸이 먼저인 시드의 그 틱 − 30 → 고친 먼 줄, 연출 없는 끝이 갑작스럽지 않은가, 까닭 줄이 보일 때만 이름을 부르는가(#162). ③ **절멸의 경고**: 다스리는 성계에 절멸이 서는 틱 − 10 → 패널의 치명 경고, 방주의 '{0}에 답할 때'; 만료까지 가는 시드가 있으면 죽음의 줄이 애도뿐인가(신호 2). ④ **넷째 칸 연출**(시야가 열리는 동안 ×25를 숨긴다 — 결정 25)과 [백성] 탭의 '칸 N'. ⑤ **벽시계**: 세이브 없이 시드 42의 새 판을 ×10으로 끝까지 한 번 — `CosmicEnd` 줄 전체를 노트 §5에(B6a ⑨의 꼴; C의 실제 길이 — 결정 42의 잣대); 다섯째 칸에 닿으면 그것도 마일스톤의 엔딩이다. ⑥ 시대의 줄(#156)·`enc_witness`(#141): 묶음 C에서 C 전에 선 시드가 있으면 그 틱 − 30, 없으면 '없음'과 `[B-6b8]` 뒤 창 없이 찍은 화면.

## 10. 결정 (권장안 먼저, 대안과 그 까닭; **[개발자]**는 상시 승인 밖)

**답**(2026-11-08 — A1~A4의 요지는 §0): A1은 이 헤더의 범위 전부, A2는 결정 2~15, A3은 결정 15·20, A4는 결정 35와 `[B-6b10]`이 받는다. 네 답이 앞 결정과 부딪는 자리와 그 풀이는 §0, 기록은 결정 39. **[개발자] 열넷의 답**(2026-11-08): 1·2·3·5·9·14·20·21·32·35·38·41·42는 권장, 22는 대안 ㉮ — 그 본문(§2.7·§3.3의 3)을 답에 맞췄다(41·42는 적대 리뷰가 19에서 가르고 넘겨받은 것에서 더했다). 나머지 스물여덟은 권장안이 곧 승인이다(마일스톤 B의 상시 승인).

1. **[개발자] 셋째 칸 위의 AI가 설계 표의 행을 만나는 길** — 권장: 모든 백성이 같은 행을 읽고, AI에게는 그 행이 물을 때만 서는 좁은 걸음 둘을 B7에서 당겨 온다 — **조약 걸음**(§2.3 — 국경의 다스리지 않는 백성에게 불가침, 세계의 문 `TreatyBlockOf`를 `bMet = true`로)과 **법칙 걸음**(결정 11); 방주와 결박은 지금의 `Choices`가 짓는다. 추첨·줄 없음. 까닭: 설계의 간이 판정은 같은 행에 `SinceTick` 0과 제동을 더한 것이고(`crisis_and_transcendents.md:134`, `systems.md:411`) A2는 행을 '적힌 그대로' 묻는다; 걸음은 행이 묻기 전에 아무것도 하지 않아 장치 커밋이 비트 그대로다; `SetStance`가 이미 세계의 문을 '만남은 참'으로 부른다(`Data/CosmicConditionTypes.h:153`). 대안: ㉮ B7을 B6b 앞으로(B6a 결정 1의 ㉮′ — 엔딩이 한 블록 늦다); ㉯ AI만의 행(새 칸 `Judged`) — '같은 행·같은 조건'(`pillars.md:108`)을 고쳐 쓴다; ㉰ B7까지 셋째 칸 위의 AI 없음 — 다른 질서·#161·'< 15%'·제동을 재지 못한다; ㉱ 방주가 다 지어질 때 조약을 적는다 — 행의 '외교 조약'이 방주의 덤이 된다. 닿는 것: AI끼리의 불가침이 그 짝의 공성을 닫는다(#129 — 노트 §2 ⓖ); 그릇이 맺은 조약이 상대 AI의 다음 칸에도 드는 짝은 툴팁 한 줄(§4); 고칠 문서는 §7. 본문 §2.3·§2.4. *왜 개발자인가*: 승인된 B7의 범위를 줄이고 설계의 AI 규칙을 바꾼다(§0 부딪는 곳 2).
2. **[개발자] 누구의 절멸인가** — 권장: **그 백성 자신의 절멸** — 방주는 그 위기의 답(대응 프로젝트)이고 업적은 방주의 완성 기록과 조약 하나를 읽는다; '이주'는 방주에 실려 위기를 건넌 것으로 말하고 상태는 옮기지 않는다(`Civs` 그대로). 까닭: 대응 프로젝트의 문장이 방주를 Hazard의 답으로 적는다(`systems.md:91` '뒤에 방주', `crisis_and_transcendents.md:24` 'R: 방주 프로젝트'); AI는 `Choices`가 답을 먼저 짓는 길로 저절로 답한다; 백성을 가로지르는 프로젝트도 새 저장 필드도 없다; 뒤의 언약(`crisis_and_transcendents.md:255` '다시는')이 그 위기를 지나온 백성에게 맞는다. 대안: 조약 짝의 절멸을 구한다(절멸·방주 관점의 권장) — 백성을 가로지르는 창·새 발생 조건·앎의 격차를 넘는 `{OtherCiv}`의 이름이 들고, '위기 종족 구출'은 받는 길이 더하는 것이다(`pillars.md:106` — Phase 6+). 본문 §2.2·§2.3. 닿는 것(권장의 값): 다스리는 백성의 넷째 칸은 제 성계에 절멸이 서기를 기다리는 일이고, 방주를 못 치르면 통치자가 죽는다(결정 5) — 대안에서는 짝이 맞을 때 오르고 제 통치자는 걸지 않는다(AI 열한과의 노출 차와 r = 1의 0.16은 어느 읽기에서도 같은 1 대 11의 경주라 결정 17에 둔다). *왜 개발자인가*: 설계 표 한 칸(`pillars.md:105`)의 뜻을 정한다.
3. **[개발자] 아공간 침식은 B6b 밖** (#90은 Phase 4 위기 헤더에 그대로) — 권장: 들이지 않는다. 까닭: 통치자 칸은 방주만 말하고 침식은 방랑자 칸의 '단독 봉합'이다(`pillars.md:105`); 그 소비자인 방랑자 업적은 Phase 4이고 스키마가 막는다(`schema.json:785-787`); 답이 인과율을 쓰는데 인과율 수입이 0이다; '레인 영구 차단'은 국경·정착·이동·시야·함대의 노림을 다시 쓴다. 대안: 행과 차단만 들이고 답 없이 — 답 없는 치명 위기는 신정론 예산 밖이다. 닿는 것: §0의 'B6b 밖'. *왜 개발자인가*: A2가 인용한 위기 칸이 이 태그를 이름한다 — 좁힘은 개발자의 확인이다.
4. **절멸 위기의 행**(권장): `crisis_species_extinction`(마지막 이름 — `crisis_pre_supernova` 뒤로 정렬되어 앞 `Draw` 행 셋의 조각이 제자리다, §3.1), `Draw`, 발생 조건 `CivTierAtLeast 3`(주인), 카운트다운 250, 확률 c(시작값 1e-4 — 노트 §2 ⓐ); `OnExpireEffects`·`OnResolveEffects`는 빈다(언약은 Phase 4); 화면 이름 '절멸 위기'와 발생·만료·해결의 줄(결정 33). 대안: 역병 앞으로 정렬되는 이름(예 `crisis_extinction`) — 데이터를 켠 틱부터 셋째 칸 이상의 모든 주인에게 조각이 밀린다. 닿는 것: 발생 조건에 위 끝이 없어(칸의 위 끝을 묻는 조건도, 조건의 부정도 없다 — `Data/CosmicConditionTypes.h`) 절멸은 넷째·다섯째 칸 백성도 거듭 친다 — 넷째 칸 상한 7까지 정착한 다스리는 백성은 c 1e-4에서 약 1,400틱마다(성계 넷이면 약 2,500) 방주를 다시 지어야 하고 못 치르면 통치자가 죽으며(결정 5), 결박한 백성이 지워지면 판의 다섯째 칸이 닫힌다(결정 14). 위 끝을 두려면 새 열거자가 든다(대안). 본문 §2.2·§3.1.
5. **[개발자] 절멸의 만료 훅과 통치자의 둘째 죽음 원천** — 권장: `ExpireHazard`의 태그 코드 안에서 마지막 성계의 문(남은 자의 은혜) → 새 단일 창구 `CosmicHazards::LetPeopleGo`(주인의 성계를 모두 놓는다) → `KillPlayer`(`Death.Hazard`, 성계 없는 주제 — `VesselAtRiskOf`가 이미 그 백성의 통치자에게 닿게 적혀 있다, `Sim/CosmicHazards.h:212-220`) → `DeposeRuler`(초신성 붕괴의 꼴) → 기록 줄; 차례와 까닭은 §2.2. 새 추첨 없음. 까닭: 설계가 VS의 하나뿐인 통치자 죽음으로 이것을 적었다(`crisis_and_transcendents.md:24`·`:170`, `docs/50:198`, `docs/21:756`). 대안: 데이터의 `OnExpireEffects`에 `KillPlayer` — 성계 주제는 거기 선 방랑자에게만 닿고 `KillsAVesselOf`가 그 성계를 계승 후보에서 뺀다. 닿는 것: 데이터의 죽음 핀(`Tests/CosmicDataTests.cpp:2418-2430`)은 그대로, 새 테스트가 코드의 죽음을 핀으로 박는다(§6 `[B-6b6]`); `-WithoutDeath`는 이 죽음을 빼지 않는다(§5.1); 신학 — 죽음의 줄은 통치자가 백성을 '위해 남았다'로 읽히지 않고 애도만(메시아 신호 2 — `theology_and_bestiary_notes.md:236`; §3.5의 3); 절멸은 넷째 칸 뒤에도 거듭 와 다스리는 넷째 칸 백성은 판의 나머지 동안 이 죽음 앞에 다시 선다(결정 4); 은혜는 백성을 가리지 않아 대개 AI의 절멸이 판의 한 번을 먼저 쓴다(§2.2의 1) — 대안: 다스리는 백성의 절멸만 은혜를 묻는다(권하지 않는다 — 은혜가 그릇의 백성의 몫이 되어 그 행의 '누구의 공로로도 기록되지 않았다'와 어긋난다). 본문 §2.2·§2.11. *왜 개발자인가*: B5 결정 6-가가 죽음 원천을 하나로 열었고 핀이 '또 다른 원천은 새 결정'이라 한다(§0 부딪는 곳 5).
6. **방주의 행**(권장): `project_ark`, 대응 프로젝트, `RequiredTier` T3, `RequiredProgress`는 진척 1로 250을 넉넉히 이기는 값(구호와 같은 150에서 시작 — 못 치른 틱의 여유는 노트 §2), 인과율 비용 없음, `CompleteTemplates` 셋 이상; **AI 지연을 받지 않는다**(결정 17). 대안: 큰 진척 — 늦춰진 AI가 지워진다. 본문 §2.3·§3.1.
7. **조약의 조건**(권장): 새 `EConditionType::CivHasTreaty`(Tag = `Treaty.*`, 비우면 어느 조약이든; Value = 짝의 수, 1 이상의 정수; `CosmicDiplomacy::HasTreaty`·`TreatyCountOf`를 읽는다), `CivHasEdict` 뒤에 붙이고 스키마와 `test_data_schema.py`에 함께. 대안: 조건 없이 문이 조약을 진다 — 툴팁에서 '+ 외교 조약'이 사라진다. 본문 §2.3.
8. **셋째→넷째 칸 업적 행**(권장): `feat_passage_by_ark`, 조건 `[MegaprojectComplete project_ark, CivHasTreaty 1]`, `AscensionCost`는 설계의 값 `ShiftEthics` 개방 −0.1(`pillars.md:93` — 추첨 없음); 인물 기록이 넷째 칸에서 닫히는 일은 이미 있다. 대안: 특성을 지우는 값 — 특성 행은 Phase 4. 본문 §2.3·§3.1.
9. **[개발자] 은하 법칙이 무엇이고 넓힘이 무엇을 하는가** — 권장 ㉮: `DT_Edicts`의 넷째 칸 행 셋 이상과 새 정의 칸 `Scope`(`ECosmicEdictScope {People, HomeGalaxy}` — 저장되지 않는다); `HomeGalaxy` 행은 위기 고리 ①(발생 몫 ≤ 1)·②(카운트다운 ≥ 0)만 지닌 백성의 고향 은하의 모든 정착된 성계에 닿게 하고(성계마다 행마다 한 번), 나머지 효과·값·만료의 답(③)은 지닌 백성의 것; 주장·절멸·초신성 전조에는 고리가 없다(§2.4). 까닭: '청지기'·'법 수호'(`pillars.md:79`·`:91`)에 맞고 '법칙 왜곡'의 강제가 아니다(`crisis_and_transcendents.md:245`); 추첨 수 그대로; W 그대로(§0 부딪는 곳 11); ③(만료의 답)은 넓히지 않아 `systems.md:162`의 까닭(답인 칙령이 만료를 해결로 바꿔 주장이 거의 서지 않는다)은 그대로지만, 고향 은하 칙령의 ① 각성 몫은 첫 넷째 칸 지닌 이 뒤 은하 전체의 각성과 그 만료 뒤 주장을 몫만큼 줄인다(§3.1) — 값은 `[B-6b7]`에서 #136과 함께 보고, 남은 시간만의 꼴(①을 각성에 걸지 않음)이 대안이다. 대안: ㉯ 모든 효과가 모두에게 — 강제이고 B2~B4의 균형을 다시 맞추며 `edict_free_trade`가 역병을 두 번 센다; ㉰ 이름만(넷째 칸 이상에서 지닌 칙령 셋) — 칸이 말하는 넓힘을 짓지 않는다. 본문 §2.4·§3.3의 4. *왜 개발자인가*: 설계에 없는 장치(넓힘의 뜻)를 정하고 다른 백성의 세계에 닿는다.
10. **법칙을 지킨 틱의 상태**(권장): 새 저장 필드 `FCivilizationState.LawsHeldTicks`(int32 = 0), DT_Rules 노브 `GalaxyLawsToHold`(3), 조건 `CivLawsHeldTicksAtLeast`(Key = N틱); 경제 단계 끝에서 넷째 칸 이상이고 `HomeGalaxy` 행을 노브 이상 지니면 +1, 아니면 0; 생애에 맞춘 읽기는 새 필드 없이 `min(LawsHeldTicks, Tick − SinceTick) ≥ N`(§2.4). 까닭: int 하나, 셈의 선례(`FallenSphereTicks`), `ActiveEdicts`의 세 쓰는 곳과 맞출 일이 없고 옛 세이브에 넷째 칸 백성이 없다. 대안: 칙령마다 제정 틱 배열 — `Enact`·`Revoke`·`NewGalaxy`가 맞춰 지녀야 하고(나란한 배열을 피하는 주석 — `Core/GalaxyState.h:642`) 옛 세이브의 칙령에 틱이 없다. 닿는 것: 법 셋이 칸을 모두 쓰므로 넷째 칸의 다스리는 백성이 칙령으로 답할 때마다 N을 처음부터 센다(AI는 끊기지 않는다 — §2.4) — 경주의 승자는 결박이 정하므로 이것은 넷째 칸 계절과 시계를 늘린다; N과 `edict_wide_machine_accord`의 각성 몫은 이 항을 넣은 노트 §2 ⓓ로 함께 정한다. 본문 §2.1·§2.4.
11. **AI의 법칙 걸음**(권장): AI 단계 (a)에서, 판정받는 행이 법칙을 묻는 넷째 칸 이상의 AI가 `HomeGalaxy` 행을 표의 순서대로 `EnactBlockOf`를 지나 제정한다; `AIRevokesOf`는 `HomeGalaxy` 행을 철회하지 않는다; 추첨·줄 없음. 섭정의 백성은 법칙을 지닌다(그래서 #121과 셈이 그릇의 죽음을 건너 이어진다). 통치자 정책은 지금의 `bEdict` 아래 법칙 걸음 하나(백성 범위 칙령을 먼저 철회하고 제정) — 러너는 바뀌지 않는다. 대안: 정해 둔 DT_Rules 목록에서 — 행이 둘째 목록을 낳는다. 본문 §2.4.
12. **결박이 일어나는 길**(권장 ㉮): 건설 프로젝트 행 `project_binding`(넷째 칸, 인과율 없음, 진척 P_bind — 시작값 1,800, 노트 §2 ⓒ), 단일 창구 `CosmicSeal::BindTargetOf`와 시작 거절 `NoFallen`; 완성 훅이 `bSealed`와 `SealedTick`을 한 자리에서 쓴다(`docs/50:231`); 업적은 `MegaprojectComplete project_binding`을 읽어 기록의 `CivID`가 '묶은 백성만'과 '이 생애'를 함께 지킨다 — **새 저장 필드 없음**, `TranscendentSealed`는 행이 쓰지 않는다(§2.5). 까닭: 봉인은 판 안에서 하는 일이다(`crisis_and_transcendents.md:121`·`:128`); 봉인된 타락을 이미 비키는 읽는 곳들(노트 §0.4′)이 쓰기 하나로 영향권을 얼린다. 대안: ㉯ `FTranscendentState.SealedByCivID`와 백성 수준의 조건 — 결박한 성계를 잃어도 남는다(필드 하나 — B6a가 이름한 '주인 필드'; 묶음 B 뒤 다시 본다, 결정 14); ㉰ 봉인을 오름의 값으로 — 세계의 결과가 보이지 않아 `:121`·`:128`과 어긋난다; ㉱ `TranscendentSealed`를 조건으로 — `SinceTick` 0인 AI가 누군가 묶은 뒤 절반을 거저 얻는다(#73). 본문 §2.5.
13. **결박의 자리**(권장): 자기 정착된 성계 어디서나(거리 규칙 없음). 까닭: 행이 자리를 말하지 않는다; 코어는 AI 모성계 곁, `sys_0`에서 3레인 이상(`systems.md:113`)이라 자리 규칙은 경주를 지리에 넘긴다; 마일스톤 B의 끝이 닿아야 한다. 대안: 영향권 안이나 그 곁(결박·법칙 관점의 권장) — 공간의 목표는 낫지만 정책의 정착 논리가 들고 닿지 않는 판이 생길 수 있다. 본문 §2.5.
14. **[개발자] 결박하는 백성 하나의 경주와 그 잠김** — 권장: 받아들이고 잰다 — VS의 타락이 하나라(`FallenTranscendentCount` 1) 처음 묶은 백성만 그 판에 다섯째 칸에 설 수 있고, 잠김 둘(묶은 뒤 다섯째 칸 전에 그릇이 떠나면 그 백성은 AI로만 올라 다른 질서; 묶은 성계를 남이 쥔 동안 아무도 — 묶은 백성이 되찾으면 풀린다)은 §2.5, `race:` 줄이 센다(§5.1). 까닭: '묶은 백성이 다섯째 칸에 선다'는 읽히는 결승선이고, AI 쪽의 손잡이는 결정 17이다. 대안: ㉮ 나눈 봉인(`SinceTick` 뒤의 어느 봉인이든) — 결정 12의 ㉱와 같다; ㉯ 성계 손실에 결정 12의 ㉯; ㉰ 다스리는 백성만 묶는다(AI 관점의 안) — AI의 다섯째 칸이 막혀 다른 질서가 서지 않는다. 본문 §2.5·§5.3. *왜 개발자인가*: 한 판의 승부(누가 끝을 가져가는가)를 정하는 놀이의 느낌이다(§0 부딪는 곳 10). 함께 확인하는 것: A2의 읽기 — 설계 표의 T4→T5 칸이 적는 찬탈의 '봉인 대신 인과율 흡수'는 이 잠김을 풀 설계의 길이지만 Phase 6+의 문 뒤라 B6b에 들이지 않는다(§0 A2).
15. **넷째→다섯째 칸 업적 행**(권장): `feat_laws_and_binding`, 조건 `[CivLawsHeldTicksAtLeast N, MegaprojectComplete project_binding]`(N 시작값 1,200 — 노트 §2 ⓒ·ⓓ); 그릇의 조건 없음(A3 — `schema.json:818`의 'why'를 고쳐 적는다); `AscensionCost`·`GrantedPowers`는 빈다. 대안: 값을 단다 — 판이 그 틱에 끝나 보이지 않는다. 본문 §2.5·§3.1.
16. **넷째 칸의 제동**(권장): `docs/21:1574-1580` 그대로, 값 2(고향 행). 순수 창구 하나 `CosmicAscent::FourthRungOpenOf(State, Tables, GalaxyRow)`를 `JudgePeople`이 플레이어가 아닌 T3→T4에서만 묻는다; 추첨·필드 없음. 모형으로는 제동이 누가 먼저 묶는지를 바꾸지 않고 넷째 칸에 선 AI의 수만 바꾼다(노트 §2 ⓒ) — 띠의 손잡이는 결정 17이다. 막힌 AI는 방주 기록을 지닌 채 셈이 상한 아래로 내려가는 틱에 지난다 — 넷째 칸 백성의 절멸도 그 셈을 줄인다. 대안: 다스리는 백성을 세지 않는다 — 정의의 문장을 고친다. 본문 §2.6.
17. **셋째 칸 위의 AI 지연**(권장): DT_Rules 실수 하나 `AIUpperRungProgressMultiplier`(기본 1, 이진수로 정확한 값만)가 다스려지지 않는 백성의 진척에 곱해진다 — **건설 칸에서만, `RequiredTier` ≥ T3인 행에만**(지금은 `project_binding` 하나; 답은 늘 빠진다 — `Advance`가 칸을 안다, `Sim/CosmicMegaprojects.cpp:580`·`:596`). 띠의 주 손잡이다(시작값 0.375 — 노트 §2 ⓒ; 고칠 때는 1/16 걸음의 사다리 — §5.3 ②). r이 1보다 작아야 하는 까닭: 셋째 칸 AI 열한이 저마다 절멸을 기다려 r = 1이면 다스리는 백성이 먼저 넷째 칸에 서는 판이 0.16이다(c 1e-4·P 1,200 — 노트 §2 ⓐ·ⓒ). 대안: 칸에 머문 시간 — `Ascend`마다 저장된 틱이 들어 모든 판의 세계 해시가 옮는다. 본문 §2.6.
18. **넷째 칸이 그저 주는 것**(권장 — 모두 받고 잰다; 목록은 §2.6): '원천이 위에 있을 때만'의 거름을 바꾸는 일은 세계 코드라 제 헤더 줄이 든다; 결박은 침묵 둘레의 첫째~셋째 칸 백성에게 주는 것이다(넷째 칸의 묶은 이는 이미 소모를 받지 않는다). 대안: 거름을 고친다 — B4 소모 놀이의 규칙을 다시 쓴다. 본문 §2.6·§5.
19. **보는 것과 부르는 이름**(#107·#121·이름이 새는 줄·#129·#154, 권장 — #122는 결정 41): #107 — Core 술어(`Core/CosmicKnowing.h`의 칸의 차와 '세 칸 이상')를 `Tell`·`NoteMet`·`IsShownPeople`이, 칸의 차를 `WithinWandererGap`(≤ 1)이 부른다(비트 그대로, `-TellAll=0` 해시 증명); #122 — 즉위·복귀의 문은 결정 41([개발자]); 이름이 새는 줄 — 공성의 시작과 끝 다섯 줄과 우호 소멸 한 줄의 이름 없는 변형 `…_far`(§3.1)을 세계의 칸 차로 고른다(말하기 한 수 그대로); #121 — 지금대로(섭정의 백성은 AI로 오르고 그 다섯째 칸은 다른 질서); #129·#154 — 다시 재고 적기만. 대안: 격차 규칙의 사본 셋을 테스트로 묶어 둔 채 — 넷째 칸 백성이 처음 생기는 블록이라 한 곳이 어긋나면 이름이 샌다. 본문 §2.9.
20. **[개발자] 첫째 칸 방랑자의 증언(#141)** — 권장: `enc_witness`를 첫째 칸 방랑자에게 — 주장이 선 성계에서 `PlayerTierAtLeast` 없이; [증언](화면의 라벨은 몸이 하는 일의 이름 — '본 것'·말하다·증언·기록하다 없음, §3.5의 7) 정신 3이면 그 성계 주인에게 경외 +0.1, 물러섬은 라벨 '물러난다'(`docs/70:139`의 규칙 1)에 효과 없음(B1의 꼴 — 규칙 2; '외면'은 그 선택에 그른 쪽의 표지를 붙인다); `ResolveHazard`·`DelayHazard`를 싣지 않고(신정론의 셈 그대로) 새 스키마 규칙이 그것을 지킨다; 셋 위의 주장자를 이름하지 않는 징조의 목소리(§2.8). 대면·정신의 닻은 방랑자의 칸이 오르는 블록으로(#141을 가른다 — 새 #164). 까닭: A3 아래 개인 칸은 늘 첫째라 설계의 T2+·T3+ 카드는 죽은 글이다. 대안: ㉮ 설계의 T2+ 그대로 — B6b에서 서지 않는다; ㉯ 설계의 도주 −0.1을 남긴다 — 라벨은 도덕의 돌아섬을 이름하지 않는 다른 말(글 감사가 정한다; '외면'·'지나간다'·'떠난다'도, B1에서 효과 없는 선택의 라벨인 '물러난다'도 아니다)이고 결과 줄은 탓 없이 사실을 말한 뒤 그 백성의 사람들이 일을 맡는 것으로 끝난다(규칙 2). 증언의 +0.1은 그릇 자신이 한 일의 결과(설계 표의 수, B1의 꼴)이고 결정 22의 +0.2는 백성의 견딤을 그릇에게 돌리는 값이라 둘은 다르다. 닿는 것: `pillars.md:76`·`crisis_and_transcendents.md:27`의 T2+, B1 지킴(결정 36). 본문 §2.8. *왜 개발자인가*: 설계 표의 칸 문을 버리고 승인된 #141을 가른다(§0 부딪는 곳 3).
21. **[개발자] 피난(#138)** — 권장: 열린 물음의 적힌 대안을 받는다 — 피난은 절멸의 답(방주)이고 호위 체인은 그 W 칸이다; 영압 카드의 선택이 아니다; #138을 닫는다. 까닭: 주장에는 피난할 사람이 없다; 셋째 카드가 들지 않고 신정론의 셈이 그대로다. 대안: 기본값대로 영압의 다섯째 선택 — 셋째 카드와 W의 다시 셈이 든다. 호위 체인은 방랑자의 칸이 오르는 블록(#164)으로. 본문 §2.2·§0. *왜 개발자인가*: #138의 기본값을 뒤집는다(§0 부딪는 곳 6).
22. **[개발자] 견딘 영압의 시대(#156)** — **답(2026-11-08): 장치는 권장대로, 이웃 경외는 대안 ㉮** — 행의 효과는 설계 그대로 `ChangeReverence` Key `Borders` +0.2(아래의 '남기지 않는다'와 그 까닭은 답 전의 기록이다). 권장: 저장되는 셈 `PressureEnduredTicks`(소모를 받았고 수도가 봉인되지 않은 영향권 밖인 틱의 연속; `MoveCapital`이 0으로), 한 슬롯 창의 조건 `CivPressureEnduredTicksReached` 600(`systems.md:233`의 시대 주기), `SystemOwnedBy`의 선택자 `Ruled`(`docs/21:619`가 정한 낱말), 상시 `DT_Events` 행 하나와 세 변형(변형마다 제 시대, `{Epithet}` 없음) — 효과 없이 줄만; **`EpithetRow`를 쓰지 않는다**(`docs/70:192` — 시대 이름은 사람에게 붙지 않는다). 본문 §2.1·§2.7. 개발자에게 묻는 것: 설계의 '이웃 Reverence +0.2'(`crisis_and_transcendents.md:115`)를 남기는가. 권장: 남기지 않는다 — 시대의 줄만 둔다. 경외(`ReverenceToPlayer`)는 그 백성이 그릇을 우러르는 수이고(`systems.md:195`, `ST_CA.csv:201` '당신을 가장 우러르는 백성') 즉위의 문이 읽어(`Sim/CosmicWanderer.cpp:108` — `CoronationReverence` 0.6) 백성의 견딤이 그릇의 왕좌로 바뀐다(신호 9·10에 가깝다); 같은 +0.2는 헤롯 청원의 '유혹이 내미는 값'이고(`systems.md:195`), 설계의 시대 명명은 '충실한 통치의 보상은 숫자가 아니라 로그다'(`systems.md:233`). 대안 ㉮ 설계 그대로 이웃의 경외 +0.2(`ChangeReverence` Key `Borders`) — 신호 10의 걸림을 받아들인다(경외는 예배가 아니다 — `systems.md:195`); ㉯ 이웃 백성과 그 백성 사이의 관계 +0.2(`ChangeRelation` — 그릇이 아니라 백성에게; 키 `Borders`는 그 효과로). ㉮·㉯면 열쇠 `Borders`와 그 규칙·테스트가 든다(§2.7·§3.3의 3·§6). '조공 없이'는 #137까지 빈다. 꼴의 대안: 업적 행 — AI가 행을 나눠 쓰고(#156 ③) 별칭이 사람에게 붙는다(①). *왜 개발자인가*: 신학의 자리(경외가 누구에게 가는가)와 보상의 꼴(§0 부딪는 곳 12).
23. **지킴의 지연 상한(#132)**(권장): 새 노브 `FleetDelayCountdownShare` 0.5를 순수 함수 하나 `CosmicConflict::GuardDelayCapOf`에 두고 `FleetDelayOf`와 `TheodicyProblems`가 함께 읽는다 — 각성만 75로 내려가고 W·550틱의 버팀·판의 경우는 그대로(§2.10). 영향권 안 소모의 몫은 '생기지 않는다'로 닫고(넷째 칸 이상에서 없고 AI는 지키지 않는다) B7로. 대안: `FleetDelayCap` 값만 낮춘다 — 주장·초신성의 지킴도 함께 줄어든다. 본문 §2.10·`[B-6b8b]`.
24. **신정론 예산의 판단(#142)**(권장): B4의 모형 구간 규칙으로 시험 칸(`Faithful` × `-Guard=1`, 시드 약 140, 시계 C(묶음 C의 게임 꼴) — C 뒤는 밖에서 자른다, 데이터 켬; 결박 전·C 전의 몫으로 30건에는 약 450시드라 모자라면 '재지 않음 — 잰 N')에서 판단하고 바닥 0.25는 불변식에 남긴다; `DECISIONS_LOG`에 B6a 결정 12의 규칙(리드의 것)을 대신한다고 적는다. 대안: n = 30의 Wilson 하한 — 0.25에 닿으려면 n ≈ 230이 든다(힘이 모자란다). 본문 §5.3.
25. **넷째 칸의 연출**(권장): 둘째·셋째 칸과 같다; 시야가 열리는 연출 동안만 ×25를 숨긴다; 넷째 칸 물리 보기의 빛은 미룬다. 대안: 넷째 칸만의 그림 — 셋째 칸과 같은 드러냄 위에 덧칠이다. 본문 §4.
26. **다섯째 칸의 연출 — 금빛 없이**(권장): 장막·흰 제목·'칸 5'·서사·멈춤은 남기고 빛무리·금빛 제목·종소리는 뺀다; 순수 UI 도우미 하나를 미리보기·테스트·게임이 같이 읽고, 피드의 다섯째 줄은 Severity 2 그대로이고 색은 금빛 그대로(색은 표현만이라 비트와 상관없다 — 흰색은 대안, 손 확인 ①이 본다; §4). 까닭: '자기 높임은 타락의 원형'(`pillars.md:91` — 같은 문장의 '주어지는 높임'은 Phase 6+ 문턱의 받는 길의 말이라 B6b의 다섯째 칸에 걸지 않는다, #163), 메시아 신호 11이 금빛 빛무리·종소리를 '칸이 오르는 그림'으로 이름한다(`theology_and_bestiary_notes.md:245`); `docs/70:79`의 '수동태 전용'은 다섯째 칸의 글에서 행위자 없는 '서 있게 되었다'로 지키고 거룩한 수동은 쓰지 않는다(§3.3의 10, B6a §3.6의 1 — 걸면 거룩한 질서가 끝의 조건이 된다). 대안: 연출 없음(신호 11이 부르심에 준 처방 '오버레이 없이'의 꼴) — 쪽이 서기 전 두 칸의 끝이 같아 보이고 문장의 '연출'이 사라진다. 본문 §4.
27. **규칙 줄과 경고**(권장): `Chronicle.RuleRung`을 고쳐 쓰고(초안 '이 판은 한 백성이 다섯째 칸에 이르면 멈춘다.') 표에 판정받는 T4→T5 행이 있으면 시계가 있든 없든 보인다; `Chronicle.Until`은 조건부로(초안 '이 판은 늦어도 {0}에 멈춘다.'); 다음 칸이 다섯째일 때 `Diplomacy.SlotsNext`·`Settle.Block.RealmCap` 대신 `Diplomacy.SlotsAll`·`Settle.Block.RealmCapLast`(§4). 대안: 그대로 — 셋째 칸에서 멈춘다는 거짓 약속이 남는다. 본문 §4·§3.2.
28. **다음 칸 툴팁**(권장): UI 부품 종류 `Treaty`·`Laws`를 끝에 더한다(지닌 것/드는 것과 남은 주기 — 수는 화면에만); 방주는 '{0}에 답할 때', 결박은 프로젝트 부품; 조약의 짝(결정 1)을 한 줄로; 낡은 주석·`docs/20:75`·#104의 말은 §7. 대안: 첫 행만 — 두 길의 행을 숨긴다. 본문 §4.
29. **보이지 않는 백성이 낸 끝의 글(#162)**(권장): 먼 줄의 세 변형을 어느 칸에서나 참이게 고쳐 쓴다(글만 — 비트 그대로; '경주는 쉬지 않고 이어진다'·'누가 먼저 닿을지는 아무도 모른다'는 오름이 판을 끝내면 거짓); 까닭 줄의 '늘 이름한다' 예외를 `PeoplesOfEveryLife`로 넓힌다; 연대기 위젯 헤더의 낡은 주석. 본문 줄의 알리지 않은 Severity ≥ 2 줄(이름이 이미 박힌 채 저장)은 그대로 두고 노트에 적는다(#49의 땅). 대안: 연대기 본문도 거른다 — 저장된 글을 다시 짓는 일이다. 본문 §4·§2.9.
30. **비문의 칸**(권장): `Epitaph.Peak`을 백성의 일로 고친다 — '다스리는 동안 백성이 이른 가장 높은 칸 {0}', 값이 첫째 칸이면 줄을 내지 않는다(§4). 대안: '맡은 백성이 선 가장 높은 칸 {0}' — 다스리지 않은 생애와 오르지 않은 백성에 즉위한 생애에 거짓이다; 그대로 — 개인 칸으로 읽혀 늘 첫째인 그릇이 넷째·다섯째를 지닌 것처럼 보인다. 본문 §3.2·§4.
31. **경주를 보이기**(권장): [백성] 탭에서 보이는(`IsKnown`) 백성의 '칸 N' — UI의 수, 비트 그대로; 그 밖에는 줄로만 읽힌다. 대안: 줄로만(`docs/70:55`) — 다른 질서가 진짜 끝이 된 블록에서 경주가 보이지 않는다. 본문 §4.
32. **[개발자] 결박의 글과 고리**(#163의 봉인 몫과 #98 ②) — 권장: 백성의 일은 3인칭의 사실로, 결박 자체는 행위자 없는 수동 '…|hpp(이,가) 묶였다'(`Sight.Bound`와 같은 결); "{Civ}이 봉인했다"·승리·조롱(`docs/70:185`의 ⑨ — 결박이지 파멸이 아니다)·타락의 목소리 줄이 없다(`VoiceTemplates`는 B8에 남는다); 고리는 회색, 끊기지 않은 Severity 0 그대로(#98 ②를 닫는다 — 까닭은 §4); 헌정명 '결박'은 `DT_Dedications`(Phase 4)를 기다린다. 까닭: 열쇠는 둘 다 한 손에 있다(`pillars.md:89`) — 백성이 한 것은 프로젝트를 지은 일이고, 묶임은 행위자 없이 말한다. 대안: 능동 '봉인했다' — 피조물에게 열쇠를 준다. 본문 §2.5·§3.5·§4. *왜 개발자인가*: 거룩한 질서의 자리(규칙 11)와 #163의 한 몫을 Phase 6+ 전에 닫는다.
33. **글과 스키마**(권장): 새 목록 — 다섯째 칸의 이름과 스스로 쥠의 말 ['영겁','왕 노릇','질서의 화신','초월자가 되','신이 되','일체화'], 칭호 `title_words`(열한 — §3.3의 9), 신학 리뷰의 다섯 `binding_words`·`triumph_words`·`stay_words`·`self_rescue_words`·`dawn_words`(§3.3의 13)와 결박의 화면 이름 규칙(14); DT_Feats 글은 `holy_words`·`realm_names`·`rescue_words`·`afterlife_words`·`holy_and_myth`를 받고 T5 행은 `holy_passive`도; 검사 접두어 `Rung.`·`Sight.`; 규칙마다 깨지는 경우 하나; ultracode 글 감사가 옛 업적 행 셋·`Digest.Rose`·모든 새 행과 키를 본다; '방주'는 방주에만. 대안: 감사만으로 — 다음 행이 같은 낱말을 다시 들인다. 본문 §3.3·§3.5.
34. **측정 장치**(권장): 러너가 견주지 않는 새 줄 `CosmicBalance race:`(칸은 §5.1·노트 §2.1)와 주장마다의 줄(#142), 깨질 수 있는 `race check`, 측정 전용 스위치 `-WithoutB6bData=1`(끈 짝 — 앞 커밋을 글자 그대로 되낸다)·`-FourthRungCap=<n>`(둘 다 `-SaveAt`이 거절); 노브와 시계는 `-Rules`로. 대안: `ruler:` 줄에 덧붙인다 — 세계가 그대로여도 러너가 견주는 글자가 옮는다(B6a 결정 11). 본문 §5.1.
35. **[개발자] 미리 적은 목표와 시계의 규칙**(A4·#2·#161) — 권장: §5.3의 판단 넷과 시계의 규칙을 데이터를 켜기 전에 그 글 그대로 고정한다 — 묶음 B가 다섯째 칸에 닿음(R 60의 ≥ 40/60)·#161의 띠(다른 질서 15~35% — 점으로 판단, n = 60의 표집 폭은 약 ±11점; 구간은 함께 적는다)·넷째 칸의 계절(중앙값 ≤ 3,600틱)을 판단하고, 시계 C는 느린 통치자 칸의 첫 다섯째 칸 끝의 80번째 백분위를 1,000으로 올려 6,000~18,000에 가둔다(받아들임이 어긋나면 값을 적지 않고 개발자에게). C에서 읽고 알리되 C의 문이 아닌 것: #136·#142·#132·B1 지킴·#129·#154·'< 15%'. C는 6,000에 끝난 B6a 세이브를 다시 열지 않는다. 띠를 R에서만 재면, 시계 규칙이 사람으로 놓은 느린 R에서는 모형으로 다른 질서가 약 63%다(노트 §2 ⓒ — c 1e-4·P 1,800·r 0.375의 '느린 R: 먼저 결박 0.37'); r 하나로 두 칸을 함께 띠에 넣는 값은 모형에 없다(r 0.25면 R 9%·느린 R 24%; 가장 가까운 칸은 c 1e-4·P 1,200·r 0.25의 R 21%·느린 R 51%) — 사람 걸음의 판은 대개 다른 질서로 끝나고 띠 15~35%는 스크립트 R의 것이다. 개발자에게 묻는 것: 띠가 지킬 판은 스크립트(R)인가 사람(느린 R)인가, 둘 다라면 느린 R의 위 끝(권장: R의 띠로 판단하고 느린 R은 몫과 구간을 시계 규칙의 받아들임과 함께 알린다 — §5.3 ②; 둘을 함께 승인한다). 까닭: 마일스톤 B의 끝은 사람이 엔딩을 보는 일이고(B6a 결정 21) 사람의 걸음은 `-Focus=0`에 가깝다고 놓는다(MVP ⑧ — 중점을 쓰지 않는 판의 셋째 칸 4,425, `DECISIONS_LOG.md:139`). 대안: ㉮ R 60의 80번째 백분위(통치자·정책 관점의 안) — 정책은 사람보다 빨라 사람이 끝을 못 볼 수 있다; ㉯ 지금 18,000(`docs/00_VISION.md:103`) — 잰 것 없이. 닿는 것: 6,000을 박은 테스트와 `docs/30`(§6); 모형은 C ≈ 6,000~10,000을 내다본다(노트 §2 ⓒ — '추가 이야기와 보완'). 본문 §5.3·`[B-6b10]`. *왜 개발자인가*: 판의 길이와 공정성의 띠는 놀이의 느낌이고 비전의 수다(§0 부딪는 곳 8).
36. **B1 지킴의 다시 판단**(권장): 지평 6,000에 고정; 같은 바이너리의 켬과 끔(`-WithoutB6bData`) 짝, B6a와의 다리는 CSV의 결정적 칸; 미리 이름한 길 — #122의 격차, 옛 백성을 지우는 절멸, `enc_witness`가 정책의 답 차례를 바꿈, #156의 국경 경외(결정 22의 답 ㉮). 절대 문 44는 두 쪽 모두 적기만, 판단은 짝의 부호 검정 p < 0.05(§5.3). 대안: `[B-6a7]` 빌드와 견준다 — 바이너리가 달라 까닭을 가를 수 없다. 본문 §5.3.
37. **커밋 나눔**(권장): §8의 표 — 해시를 옮기는 길은 데이터 켜기와 같은 커밋에 들지 않고 장치 커밋마다 비트 그대로; 데이터 켜기는 셋(넷째 칸 → 다섯째 칸 → 곁 행)과 노브 하나(`[B-6b8b]`); 시계는 제 커밋(A4). 대안: 데이터를 한 커밋에 — 묶음 A의 미리 셈이 다섯째 칸 행의 값을 정하지 못하고 B1 지킴의 짝이 까닭을 가르지 못한다. 본문 §8.
38. **[개발자] `CLAUDE.md` 규칙 4·13·14·15**(남은 160바이트 — `CLAUDE.md` 16,224바이트, `Tools/check_docs.py:42`의 16 × 1024) — 권장: 먼저 규칙 4의 B2·B3 괄호를 `docs/20`으로 옮기고(B6a 결정 20의 꼴), 그다음 규칙 4에 두 셈·결박과 절멸의 훅·AI의 두 걸음, 규칙 13에 `LetPeopleGo`·`BindTargetOf`·`FourthRungOpenOf`·은하 칙령의 셈·`GuardDelayCapOf`, 규칙 14에 Core 격차 술어(사본 셋의 문장을 바꾼다), 규칙 15에 'A0~B6b'와 봉인을 예정에서 뺌; 표지는 §7. 대안: ㉮ 모두 `[B-6b12]`에서 — 워크플로 일꾼이 리뷰에서 낡은 규칙을 읽는다; ㉯ `docs/20`만 — 모든 세션이 읽는 규칙 파일이 새 틱 자리를 빠뜨린다. 본문 §7. *왜 개발자인가*: 모든 세션이 읽는 규칙 파일의 문장이다(B4 결정 20·B5 결정 21·B6a 결정 20과 같은 까닭).
39. **`DECISIONS_LOG`의 기록**(권장): A1~A4와 날짜; B6a 결정 1의 봉인 문구를 뒤집음(결박은 B6b, T5 문턱·두 길·#163의 나머지는 Phase 6+; B6a의 동결 헤더는 고치지 않는다); #1 닫음(따로 가는 두 칸, 공명 없음); 새 죽음 원천(결정 5); 같은 행과 AI의 걸음(결정 1); #142의 규칙(결정 24); #138·#98 ② 닫음. 기록은 그 답을 처음 들이는 커밋에(A1~A4·#1·[개발자] 열넷의 답은 `[B-6b1]`, 죽음은 `[B-6b2]`, #142는 `[B-6b11]`), `[B-6b12]`가 모은다. 대안: 문서 커밋에 한꺼번에 — 그 사이 커밋의 리뷰가 까닭을 찾지 못한다. 본문 §7.
40. **VS 정의의 수량**(`docs/50` §12.1, #160, 권장): 업적 4 → 5(통치자 행 — T1→T2 하나·T2→T3 둘·T3→T4 하나·T4→T5 하나; 옛 4의 '각 2경로'는 Phase 4의 방랑자 길을 센 수라 뺀다 — 노트 §1의 12); 도달 가능 등급 T1 → T5(통치자 길); 칙령 9 → 12(법칙 셋); 거대 프로젝트 7 → 8(설계의 일곱에 결박을 더한다 — 웜홀 게이트는 아직이라 B6b 뒤 데이터는 7행); Hazard에 절멸(8 안 — 아공간 침식은 #90); 조우 25(`enc_witness` 포함 — 이미 수에 든다); 넷째 칸 이상의 존재 — 봉인되지 않은 타락 하나와 제동이 들인 AI 많아야 둘(다스리는 백성은 세지 않는다 — `crisis_and_transcendents.md:65`; 제동 없이 넷째 칸에 오른 다스리는 백성이 섭정 아래 AI가 되면 그 생애마다 하나씩 더할 수 있다). 대안: B6a 결정 21의 옛 수 그대로 — 마일스톤 B의 종료 조건이 닿은 칸과 어긋난다. 본문 §7.
41. **[개발자] 즉위·복귀의 문의 격차(#122)**(적대 리뷰가 결정 19에서 갈랐다) — 권장: `FarAbove`(`PersonalTier`와 셋 이상)를 즉위와 복귀 모두에(`ECrownBlock` 끝 — `Unmet` 바로 뒤, 세계만 읽는다; 복귀도 같은 문을 지난다 — `Sim/CosmicWanderer.cpp:74-113`, `IsReturn` `:108`). 까닭: 없으면 지도가 가린 넷째 칸 이상 백성에 즉위해 `Ending.RuledPeople`을 얻는다(§2.9). *왜 개발자인가*: 복귀까지 막으면 넷째 칸 이상 백성을 떠난 그릇은 그 백성의 끝을 다시 갖지 못하고(그 백성은 섭정 아래 AI로 올라 다른 질서) 첫째 칸 방랑자(A3)는 넷째 칸 이상 어느 백성에도 서지 못한다 — 누가 끝을 가져가는가(결정 14와 같은 까닭). 문은 `PersonalTier`를 읽으므로 #96·#100이 보는 칸에 바닥을 두면 보이는 옛 백성을 문이 거절하게 된다 — 그 답이 오면 문을 보는 칸에 맞출지 다시 묻는다. 대안: 즉위에만 `FarAbove`, 복귀(`IsReturn`)는 지금대로. 닿는 것: B1 지킴의 미리 이름한 길(결정 36), '추가 이야기와 보완'. 본문 §2.9·`[B-6b3]`.
42. **[개발자] 벽시계의 잣대**(B6a 결정 2-라 — 'B6b가 시계와 함께 새 잣대를 정한다', `docs/00_VISION.md:32`; 적대 리뷰가 '추가 이야기와 보완'에서 옮겼다) — 권장: 손 확인 ⑤의 `CosmicEnd` 줄과 묶음 C의 C로 '×10 a~b분'을 적는다; 값은 손 확인 뒤 `[B-6b12]`가 `docs/00_VISION.md`에 적는다. 까닭: 결정 35는 틱과 결정 밀도만 정하고, 벽시계는 사람이 재는 수다(B6a 결정 2-라). 대안: 틱만 적고 벽시계는 Phase 5의 측정으로 — B6a가 넘긴 일을 다시 넘긴다. *왜 개발자인가*: 비전 문장의 수다. 본문 §9의 ⑤·`[B-6b12]`.

## 11. 구현 기록 (헤더와 다르게 하거나 더한 것 — 커밋마다 더한다; 까닭과 수는 `DECISIONS_LOG`)

- `[B-6b1]`(2026-11-08): 필드 둘·견딘 영압의 셈·문서 먼저. 다르게 한 것: 해시의 변이는 해시기가 두 칸을 건너뛰는 꼴(빼는 목록은 최상위 멤버만 이름한다); `docs/50:231`(봉인)은 노트 §1.2의 `[B-6b12]`가 아니라 §7대로 이 커밋에 표지와 함께; 규칙 13은 크기 때문에 은하 칙령의 셈을 싣지 않는다(`[B-6b3]`에 자리가 나면). 세부는 노트 §5.
- `[B-6b2a]`(2026-11-08): 칙령·법칙·AI의 두 걸음(`[B-6b2]`을 셋으로 나눈 첫째). 더한 것: 규칙 8′(`lacks_cond`), 통치자 행의 선택자 규칙, 법칙 걸음이 철야의 칸을 0.5 아래에서도 쥐지 않음(§2.4), 커맨드렛 지연의 남는 오차. §2.4의 '다음 틱에'는 '문이 받는 첫 AI 단계(그친 틱일 수 있다)'. 세부는 노트 §5.
- `[B-6b2b]`(2026-11-09): 절멸의 만료 훅(`[B-6b2]`을 나눈 둘째 — 은혜의 문·`LetPeopleGo`·`KillsTheRulerOf`·로그 행 둘). 더한 것: `LetPeopleGo`가 `Transfer`처럼 반란 기록을 닫고 백성 없는 완성 기록을 적는다; 은혜가 백성의 다른 절멸도 끝낸다(§2.2 1); 규칙 5의 시험 `rows_of`. `ini:20`은 §3.1대로 `[B-6b6]`. 세부는 노트 §5.
- `[B-6b2c]`(2026-11-09): 결박·제동·지연·지킴 상한(`[B-6b2]`을 나눈 셋째 — 나눔 끝). 더한 것: 검사기의 `Pool.Member`(규칙 7을 적으려고), `Data.Integrity`의 '결박은 어느 위기의 답도 아니다', `log_fallen_bound`의 범주 `Log.Civ`(헤더가 정하지 않음), `Bind`가 이미 봉인된 인스턴스를 거절. 세부는 노트 §5.
- `[B-6b3a]`(2026-11-09): 앎(`[B-6b3]`을 나눈 첫째; 측정은 `[B-6b3b]`) — 격차 술어·즉위의 문·이름 없는 줄 여섯·까닭 줄. 더한 것: 연대기가 `log_stance_lapsed_far`도 받지 않음, 변형 행이 없으면 줄 없음, 규칙 8은 행마다, 패널이 만난 먼 백성을 문 앞에서 거름. 세부는 노트 §5.

## 추가 이야기와 보완

- **결정 13·14는 묶음 B 뒤에 함께 다시 본다**: 결박한 성계를 잃어 잠기는 판이 R 60에서 3판 이상이면 가장 작은 고침은 `SealedByCivID`(결정 12 ㉯)이고, 설계가 이미 내다본 '주인 필드'다(새 열린 물음 #165의 안 — 노트 §0.2).
- **넷째 칸 백성을 떠난 그릇의 돌아감**: 결정 41의 `FarAbove`(권장)는 돌아감에도 선다 — 양위나 폐위로 넷째 칸 백성을 떠난 첫째 칸 방랑자는 옛 백성을 보지도 맡지도 못한다. 보는 칸에 그 생애가 다스리며 오른 칸의 바닥을 두는 길(다른 세션의 안)은 #96·#100의 [개발자] 물음이라 B6b에 들이지 않는다; 묶음 B가 그 판의 몫을 적고 #96에 넷째 칸의 경우를 한 줄 더한다.
- **판의 길이가 비전보다 짧을 공산**: 모형은 첫 다섯째 칸의 끝을 스크립트 통치자로 약 4,000~7,500틱에 두고 시계 규칙이 C ≈ 6,000~10,000(시작값에서 8,000)을 낼 것으로 본다(노트 §2 ⓒ·ⓓ) — 비전의 18,000(`docs/00_VISION.md:103`)과 '초월 ~90분'보다 짧다. 묶음 B가 그렇게 보이면 노트 §0.2의 후보 행을 열고, 값은 규칙대로 둔다.
- **건설 단추가 서는 칸**(§4 '묶음'): 지금은 칸이 모자란 건설 행도 회색 단추로 서서 넷째 칸의 결박이 첫째 칸의 패널에 선다 — §4는 '그 칸이 다음 칸일 때부터'로 바꾼다 — 결박의 행이 드는 `[B-6b2]`에서(행과 거름이 한 커밋). 결정 밖의 표현 고침이라 `[B-6b2]`의 구현 리뷰와 `[B-6b4]`의 화면 리뷰가 다시 본다.
- **넷째 칸의 [저녁 안부를 내린다]**: 영압 카드는 다스리는 성계에 주장이 서면 칸과 무관하게 오는데, 그 칙령 `edict_against_silence`가 사는 것은 소모 ×0.45뿐이고(수입 ×0.9를 치른다) 넷째 칸 백성은 소모를 받지 않는다; 게다가 법칙 셋이 칙령 칸을 모두 쓴다. 그 선택을 선택의 문(`CosmicPetitions::ChoiceGateOf` — 정책의 답도 읽는 규칙 13의 창구)의 새 사유로 회색으로 할지는 `[B-6b2]`에서 리드가 정하고, 넣으면 비트 그대로를 확인한다.
- **정책은 끝 뒤에도 답하고 짓는다**: 통치자 정책의 카드 답과 사업은 기록 뒤에도 이어 가고 중점·칙령·외교만 선다(노트 §1의 어긋남 5) — 시계 6,000의 20,000틱 통치자 기준값에서 다스리는 백성은 끝 뒤에 방주와 결박을 짓되 새 조약·법칙은 얻지 못한다. `[B-6b6]`·`[B-6b7]`이 옮길 줄의 까닭으로 `docs/40` 커맨드렛 절에 미리 적는다(묶음은 시계를 끄므로 받지 않는다).
