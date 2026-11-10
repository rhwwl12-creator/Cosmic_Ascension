# B7x 헤더 초안과 브랜치 — ultracode 적대 리뷰 종합 (2026-11-09)

- **대상**: `origin/design/b7x-composable-existence`(tip `0bffb8e`, 바탕 `cef8cc7` [B-6b3b]). 엔진 없는 PC에서 2026-11-09에 만들었다.
  - 문서: 헤더 `docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md`, 노트 `…_notes.md`, 설계서 `docs/10_DESIGN/EXISTENCE_EVOLUTION_AND_ECOLOGY.md`(9be5b3e·4555c89·0bffb8e).
  - 빌드 전 커밋: `[B-7x1]` 01eef6f(필드), `[B-7x2]` 492c4f0 + `[B-7x2r]` 5969bd5 + `[B-7x2c]` 85f297c(장치, 컴파일한 적 없음), `[B-7x5a]` aa09543(데이터 초안 — 이대로 켜지 않는다).
- **견준 main**: 리뷰가 도는 동안 `[B-6b5]`가 `10205b6`로 커밋·푸시되었다(과제가 적은 529cab9의 다음). 통합 렌즈는 10205b6에 견주었다. 남은 B6b는 `[B-6b6]`~`[B-6b12]`다(main B6b 헤더 §8, 브리프 `scratchpad/b6b6`·`b6b7`·`b6b8`·`b6b8b`).
- **렌즈 여섯**: 설계·규칙(F), 결정론·저장·해시(DET), 신학·서사(T), 측정·밸런스(M), 빌드(build), 통합(I). 발견마다 독립 검증이 붙었다(confirmed / partial / refuted). 렌즈별 원본: `scratchpad/b7x/review_design.json`·`review_determinism.json`·`review_theology.json`·`review_measurement.json`·`review_build.json`·`review_integration.json`.
- **줄 표기**: `헤더 :N`·`노트 :N`·`설계서 :N`은 브랜치 0bffb8e의 줄. `branch 파일:N`은 0bffb8e, `main 파일:N`은 달리 적지 않으면 10205b6(일부 렌즈는 cef8cc7·529cab9에서 읽었고 그 경우 적는다). 코드 경로는 `Source/Cosmic_Ascension/` 아래를 줄였다.

---

## 1. 요약

### 1.1 렌즈별 수

| 렌즈 | 발견 | confirmed | partial | refuted |
|---|---|---|---|---|
| 설계·규칙 (F1~F12) | 12 | 7 | 5 | 0 |
| 결정론·저장·해시 (DET-1~9) | 9 | 4 | 5 | 0 |
| 신학·서사 (T1~T10) | 10 | 4 | 6 | 0 |
| 측정·밸런스 (M1~M14, M5b) | 15 | 7 | 8 | 0 |
| 빌드 (build-1~5) | 5 | 1 | 3 | 1 |
| 통합 (I1~I12) | 12 | 9 | 3 | 0 |
| **합** | **63** | **32** | **30** | **1** |

남긴 발견은 62다(confirmed 32, partial 30). 렌즈를 가로질러 겹치는 것이 많아서, 고칠 것은 §3에서 41묶음으로 모았다. 처음 매긴 무게로 high는 F1·T1·M1·M2·build-1·I1·I2다. I1은 검증에서 일부가 반박되어 무게가 내려갔다.

### 1.2 승인 판정

**이대로는 승인할 수 없다. 다만 조건부로 승인할 수 있다.** 설계를 다시 하라는 발견은 없다. 남긴 62건은 모두 좁은 고침이다. 문장을 하나 더하거나, 갈래 하나를 거르거나, 측정 줄을 하나 더하는 정도다. 빌드 렌즈는 확실한 컴파일 오류를 찾지 못했다. 블록 없는 백성이 지나는 옛 길은 열두 자리 모두 글자 그대로였다(DET 요약; build 요약). 그래도 승인 전에 막아야 할 것이 여섯 있다.

1. **결정 18의 귀결이 코드에서 K9를 깬다.** 남긴 법(`Brick.Form.UnseenLaw`)을 지닌 백성은 칙령을 하나도 거두지 않는다. 그래서 주장 고리 칙령 둘(`edict_vigil` 주장 +100, `edict_machine_rights` 각성 만료 → 해결)을 판 끝까지 지닌다(F1). 칙령 자리도 막아서 B6b의 법칙 셋에 닿지 못한다(I2·M11). 헤더 :112는 안정도 하나만 적는다.
2. **§5 측정이 B6b 경주를 다시 재지 않는다.** 씨앗 블록이 생성 때부터 모든 AI 백성을 바꾸는데도 그렇다(M1·I4·F3). 깨질 수 있는 확인도 없다(M2).
3. **시험 보드가 게임 데이터를 물려받는다.** 그래서 `[B-7x5a]`를 켜면 B7x 자신의 테스트 셋과 있던 테스트 하나가 깨진다(build-1·DET-3·I7).
4. **잎 이름은 세이브에 남는데, 권장 이름 셋이 틀렸다.** `QuietFeed`는 계시의 낱말이고, `WovenSwarm`과 `LastHunger`는 꼴 이름과 겹친다. 빠진 이름도 하나 있다(`StarShepherd`)(T2·F10).
5. **끝의 꼴 낱말이 어느 백성의 어느 때인지 정해지지 않았다.** 앞 생애의 장에 뒤 생애의 선택이 찍힌다(F9·DET-5·T3·I12 — 결정 13).
6. **브랜치가 받지 않은 원안을 표지 없이 main으로 들고 간다.** `docs/OVERVIEW_as_of_B6a.html`(T1)과 `promptforother.txt`(F12)가 그렇다. 충돌 목록과 해소 규칙도 틀렸다(I1).

여섯 모두 `[B-7x0]`의 글 고침과 개발자의 답(3·13·18, 그리고 20의 차례)으로 닫힌다. 코드 고침은 리베이스 때 `[B-7x2]`에 든다.

### 1.3 K1~K10이 코드에서 서는가

| K | 판정 | 근거 |
|---|---|---|
| K1 세계는 앎을 읽지 않는다 | 선다 | 새 술어·조건·걸음은 세계만 읽는다(F 요약; build 요약) |
| K2 틱 열 단계 | 선다 | 경제 끝의 누적 셈, AI 끝·업적 앞의 변이 걸음이 docs/20 차례에 맞다(DET 요약) |
| K3 추첨 수가 먼저 정해진다 | 선다 | AI 정착은 대상이 없어도 `GetFraction`을 뽑는다(branch `Sim/GalaxySimulationSubsystem.cpp:2943-2952`). 씨앗은 파생 정수(용도 13)다. 단 docs/20의 문장은 고쳐야 한다(DET-1) |
| K4 칸은 업적 행으로만 | **데이터의 우연으로만 선다** | `CivHasTrait` 뿌리에 `Brick.`이 들었는데, DT_Feats 스키마에 막는 규칙이 없다(F6) |
| K5 초월자·교리 그대로 | 선다 | — |
| K6 누구도 법·언약·신이 되지 않는다 | 코드는 선다. **글은 깬다** | OVERVIEW html(T1), 설계서 원안(T5), DevComment·주석(T6·F11) |
| K7 저장 규칙 | 선다 | 끝에 더했고 기본값이 얼었다. `HostCivID`를 비우는 규칙만 빠졌다(DET-9) |
| K8 절대 면역 없음 | 선다 | 다만 신정론 불변식이 기생 덤을 세지 않는다(DET-4·M10·build-2) |
| K9 `Hazard.Claim`은 풀리지도 늦춰지지도 않는다 | **깨진다** | UnseenLaw의 `AIRevokesOf` 첫 물음(branch `Sim/CosmicEdicts.cpp:347-352`; F1). 간접 경로(기생 덤·미시 곱)는 적고 재야 한다(F2·M9·I5) |
| K10 시간 셈은 백성의 것 | 저장에서는 선다. **표시에서 흔들린다** | 생애 장에 백성의 지금 셈을 붙인다(T3·DET-5) |

### 1.4 R1~R10은 좁고, 적히고, 시험되는가

- **좁다.** 리뷰가 넓다고 본 R은 없다.
- **적히지 않았다.** §0 :11은 "고칠 문장을 적는다"고 약속했다. 그런데 :17의 R 목록은 이름뿐이다. 문장이 붙은 것은 R1(systems.md:381)·R8·R9 정도다(F5).
- **시험되지 않았다.** 지키는 시험의 이름이 붙은 R이 없다. R10의 함수는 거절 갈래가 시험되지 않았다(build-4).
- **빠진 R이 셋 있다.** #88(결정 9), B2 §2.6 'AI는 곧 철회한다'(UnseenLaw), #36 ②(결정 23)이다(F4). 씨앗 블록에 대해서는 docs/20 규칙 3 ①의 문장도 고쳐야 한다(DET-1).

### 1.5 가장 큰 다섯

1. **남긴 법이 주장 고리와 법칙 자리를 쥔다**(F1·I2·M11, high). 결정 18에 닿는다. 고침은 §3의 코1이다.
2. **씨앗 블록이 B6b 경주를 옮기는데 다시 재지 않는다**(M1·I4·F3·M8, high). 끈 짝도 깨질 수 있는 확인이 아니다(M2·I9).
3. **시험 보드가 게임 값을 물려받는다**(build-1·DET-3·I7, high). `[B-7x5a]`는 테스트를 깬 채로는 커밋할 수 없다.
4. **잎 이름과 꼴 이름**(T2·F10, 결정 3). 머지 뒤에는 고칠 수 없다.
5. **받지 않은 원안이 main으로 든다**(T1·T5·F12). 충돌 목록도 틀렸다(I1·I11).

---

## 2. 개발자에게 물을 것 ([개발자] 결정 열넷)

결정마다 차례로 적는다. 헤더의 권장, 리뷰가 바꾸거나 날카롭게 한 것, 그리고 답하기 전에 보아야 할 귀결이다.

### 결정 1 — B7x의 자리
- **헤더 권장**(헤더 :258): 확장 블록 B7x로 둔다. 슬라이스 가는 B6b가 끝난 뒤, B7 본 블록 앞에 둔다. 나는 B8의 레인 결정과 함께, 다는 6a와 함께 둔다.
- **리뷰**: 권장은 그대로 둔다. 다만 슬라이스 가가 B6b 바로 뒤에 들면 씨앗 블록 여덟이 B6b가 막 맞춘 경주를 옮긴다(M1·I4·F3). 그래서 `[B-7x5]`를 둘로 가르고(`[B-7x5s]` 씨앗·노브, `[B-7x5c]` 카드·사건·Society — M8), B6b의 판단을 다시 세우는 ⑥을 더한다. 노브를 고치면 `[B-7x5r]`을 둔다.
- **보아야 할 귀결**: 어긋나면 **B7x의 노브가 먼저 물러선다.** B6b의 r·c·N·P_bind·C는 건드리지 않는다. 이 규칙을 받아야 한다(M1의 질문은 B6b 결정 35와 이 헤더 결정 23 사이에 있다). 한 번 고쳐도 어긋나면 그 수와 함께 다시 묻는다.

### 결정 2 — `Brick.` 뿌리와 판정의 번복 (R1)
- **헤더 권장**(:259): 새 뿌리 `Brick.`을 두고, 스키마의 뿌리 네 곳을 목록으로 넓힌다. systems.md:381의 판정은 '읽는 곳이 있는 블록은 선언한다'로 고친다.
- **리뷰**: 권장은 그대로 둔다. 다만 받으면 `Brick.`이 `CivHasTrait`를 지나 **업적 행의 조건에도 들 수 있다.** 그러면 K4가 지금 데이터의 우연에 기댄다. 그래서 DT_Feats 스키마 규칙 하나를 함께 넣는다(F6 → 데4). 같은 블록이 #88과 #36 ②도 연다. 그 둘은 R11과 R13으로 따로 적는다(F4).
- **귀결**: 스키마 뿌리 줄은 main에서 이미 옮겨졌다(:272·:1196·:1237-1238 → 10205b6에서는 :291·:1245·:1286-1287 — I10).

### 결정 3 — 잎 이름의 거름
- **헤더 권장**(:260): `UnseenLaw` → `KeptLaw`, `SilenceFeed` → `QuietFeed`, `PlanckMicro` → `FoldedSmall`, `DysonBrain` → `ShutSphere`, `KesslerTapestry` → `WovenSwarm`, `AbyssalMaw` → `LastHunger`.
- **리뷰가 바꾼 것**:
  - `SilenceFeed` → **`IdleFeed`**(`QuietFeed`가 아니다). 'Quiet'은 main의 참된 계시의 낱말이다(`Config/DefaultGameplayTags.ini:201` `Revelation.Quiet` — 'Vocabulary kept apart from Doctrine.Silence'). 노브 `BrickQuietInsightPerTick` → `BrickIdleInsightPerTick`, 함수 `IsQuietOf` → `IsIdleOf`도 함께 바꾼다(T2).
  - `KesslerTapestry` → **`DustBand`**(설계서 C05 '티끌 띠'), `AbyssalMaw` → **`InwardHunger`**(S11 '안으로 돌아선 식욕'). `WovenSwarm`과 `LastHunger`는 이미 `SwarmTech`·`StarEater`의 꼴 이름이다(branch `Sim/CosmicExistenceBricks.h:204-207`). 같은 이름을 쓰면 세이브에 남는 한 이름이 두 사실을 가리킨다(F10).
  - **더한다**: `StarShepherd` → **`StarTow`**(카드 그림 '도읍의 별에 끄는 줄', 설계서 :400). '목자'는 §3.5가 금한 낱말이고(:200), '선한 목자'는 main `Tools/data/schema.json:51` revelation_mimicry에 있다(T2).
  - **선택**: `HermitMonolith` → `ClosedPort`. 까닭은 'Monolith'(다른 작품의 상징물) 하나뿐이다. 이름을 `ShutHouse`로 하면 꼴 키와 겹친다(T2 검증).
  - 그대로 둔다: `KeptLaw`·`FoldedSmall`·`ShutSphere`. 태그와 꼴이 같은 것을 가리켜 해가 없다(F10).
  - 결정 3에 한 문장을 더한다: "변이의 잎은 꼴의 이름을 받지 않는다 — 꼴 낱말은 변이 없이도 부모를 지닌 모든 백성에 보인다".
- **귀결**: 이름은 세이브에 남는다(헤더 :165). **`[B-7x1]`이 main에 들기 전에만** 고칠 수 있다. 함께 고칠 곳은 ini :176 'QuietFeed proposed', :184, 스키마 규칙 4의 나열, 테스트 메시지, 설계서 :111·:397·:400·:417·:462다.

### 결정 4 — 시간 셈의 주인은 백성 (K10)
- **헤더 권장**(:261): `FCivilizationState`에만 둔다.
- **리뷰**: 권장은 그대로 둔다. 다만 결정 13의 표시가 이 결정을 화면에서 무른다. 표시할 때 백성의 지금 셈을 앞 생애의 장에 붙이면 '몸을 건너는 장부'가 화면에 다시 선다(T3 ①·DET-5).
- **귀결**: 13의 답이 4를 지켜야 한다(아래 13).

### 결정 6 — `HostCivID`를 지금 예약 (R3)
- **헤더 권장**(:263): `[B-7x1]`에 함께 넣는다(해시를 한 번만 옮긴다). 이름은 `HostCivID`다.
- **리뷰**: 권장은 그대로 둔다. 리뷰가 날카롭게 한 것은 셋이다.
  - (ㄱ) 품는 백성이 건넘 단계에서 지워질 때 모든 성계의 `HostCivID`를 비우는 한 문장이 필요하다. `PendingContactCivID`를 비우는 꼴로, 추첨은 없다(DET-9). 근거는 branch `Sim/GalaxySimulationSubsystem.cpp:3131-3134`다.
  - (ㄴ) 탄생한 백성도 `CivID`를 `NextCivSerial`에서 받는다. 지운 이름은 다시 쓰이지 않는다(DET-9 검증; main `:866-872`).
  - (ㄷ) 슬라이스 다의 '탄생 요청'이 틱 경계를 건너면 저장 필드가 하나 더 든다. 그 필드를 `[B-7x1]`에 함께 예약할지, 해시가 또 옮는다고 적을지 정한다(F8 (vi)).
- **귀결**: 승인 때 뜻과 초기값(`NAME_None`)이 언다. 읽는 규칙은 슬라이스 다에서 온다.

### 결정 8 — 별 등급의 어두워짐 (R10)
- **헤더 권장**(:265): 장치는 넣고 주기 0으로 꺼 둔다. 켤지는 B8 헤더가 정한다.
- **리뷰**: 권장은 그대로 둔다. 다만 꺼 두어도 장치 커밋에 단언은 든다. `DimCapitalStar`의 거절 갈래 셋(남의 수도, 정착 중, 인덱스 없음)이 시험되지 않는다. 그래서 가드를 빼는 변이가 시험 모음 전체를 통과한다(build-4 → 시6). 죽은 노드와 성역 갈래는 결과를 바꾸지 않아 시험할 수 없다(검증). R10에는 고칠 문장도 붙인다(F5).
- **귀결**: 받으면 `[B-7x2]`에 단언 셋과 변이 하나가 든다.

### 결정 11 — 카드와 수량 (R8)
- **헤더 권장**(:268): 수직 슬라이스의 줄에 청원 열하나와 조우 하나를 더한다. 갈림 카드는 열둘이 되고, 줄은 '조우 26 + 청원 24'가 된다.
- **리뷰가 날카롭게 한 것**:
  - **둘째 갈림 여섯 장의 ②가 닫히지 않는다**(F7). 값을 치르고 이득(안정도·인구·윤리·경외)을 받는데 카드를 닫는 표지가 없다. 그래서 지난 세 답이 지나면 다시 오고, C10 ②는 높아짐 +1을 되풀이해 쌓는다. 설계서 :353 '되풀이되는 맞바꿈이 되지 않게'와 schema.json:1273의 why와 어긋난다. 답은 둘 가운데 하나다.
    - (가) ②에 `SetFlag Flag.Cross.<행>`을 주고 행 조건에 `FlagNotSet`을 둔다. **`FGalaxyState.Flags`는 판 전체라**(branch `Core/GalaxyState.h:981`) 계승한 다른 백성과 다른 생애에도 그 카드가 닫힌다. ini 태그는 17 → 23이 된다.
    - (나) ②에서 되풀이 이득을 뺀다. 적어도 `petition_cross_law_stays` ②의 `PlayerStatDelta Exaltation +1`은 뺀다. 결정 없이 가능하다.
    - **리뷰 권장은 (나)다.** 새 태그가 없고, 판 전체를 닫는 문제가 없다.
  - **정책 문제**: 측정 정책은 닫히지 않는 ②를 두 답마다 한 번꼴로 되풀이해 고른다. 사람은 하지 않는 일이다(M7). 그래서 정책은 '갈림 카드에서는 ③만' 고른다. 첫 갈림 넷은 건너뜀 아래에서 계속 다시 와서 결정 수를 부풀린다(M4).
  - `enc_cross_drift_fleet`은 위기 조건이 없는 첫 방랑자 상시 카드다. 그래서 `[B-6b8]`의 `enc_witness` 전제('늘 뽑힌다')를 깨고 도착 카드를 밀어낸다(I3 → 판단 ③′).
  - C10의 제목 '비워 둔 왕좌'와 ① 결과 줄 '왕좌는 비어 있었다'는 돌아올 왕의 틀이다. C10 ②의 `ShiftEthics Spiritualism +0.05`는 통치자 높임을 영성으로 센다(T8).
- **귀결**: (가)를 고르면 판 전체 Flag의 범위를 받아야 한다. (나)를 고르면 ②가 값만 남는 선택이 된다.

### 결정 13 — 끝의 꼴은 사실 줄의 낱말 하나
- **헤더 권장**(:270, §4 :204): 연대기 생애 장의 사실 줄 끝에 '그 백성의 꼴 낱말'을 붙인다. 표시할 때 `ShapeOf`로 셈하고 저장하지 않는다.
- **리뷰가 찾은 것**(F9·DET-5·T3·I12): 이 문장에는 네 가지가 빠졌다.
  - ① 한 생애가 여러 백성을 다스렸을 때('A → B', main `UI/CosmicChronicleWidget.cpp:274-283`) 어느 백성인지 정하지 않았다.
  - ② 지워진 백성(`State.Civs`에 없음)에는 `ShapeOf`를 부를 수 없다. `CivNameOf`는 이미 이름을 비운다(main `:60-65`).
  - ③ 셈은 통치가 끝난 뒤에도 섭정과 AI 아래에서 계속 쌓인다. 그래서 앞 생애의 장에 뒤 생애의 선택(예: C10의 남긴 법)이 찍히고, 같은 백성을 다스린 생애마다 같은 낱말이 붙는다(K10과 결정 4가 막으려던 것).
  - ④ 판의 끝 뒤에도 시뮬은 돌고 연대기를 다시 열 수 있다(main `World/CosmicGameMode.cpp:1345-1355`). 그래서 같은 장의 낱말이 다시 열 때마다 바뀔 수 있다.
- **고를 꼴 셋**:
  - **㉢(리뷰 권장 — 저장 없음)**: (a) 그 생애의 마지막 재위 백성(`Life.RuledCivIDs`의 끝)만 쓴다. 다스린 적 없는 생애는 붙이지 않는다. (b) 그 백성이 `State.Civs`에 있을 때만 붙인다. (c) 같은 백성을 다스린 생애가 여럿이면 마지막 생애의 장에만 붙인다. (d) 낱말은 '그 백성의 지금 살림'으로 읽히게 쓰고, 판의 끝 뒤 다시 열면 달라질 수 있음을 받는다. 새 R이 없고, '끝마다 다른 것은 까닭 줄뿐'이라는 약속(main `UI/CosmicChronicleWidget.h:52-53`)도 지킨다.
  - **㉠(정확한 기록)**: 통치가 끝날 때(폐위·양위·죽음과 판의 끝) 꼴을 `FLifeRecord`에 적는다. 끝에 더하는 `TArray<uint8> ReignShapes`(`RuledCivIDs`와 같은 길이, 초기값 빈 배열)다. 새 R과 헤더 먼저가 든다. `[B-7x1]`에 함께 넣으면 해시는 한 번만 옮는다.
  - **㉣(T3)**: 사실 줄에는 두지 않고, 백성이 주어인 자리(성계 패널의 백성 줄 §4 :205)에만 둔다. 칙령 창은 판의 끝 뒤에 열리지 않는다(main `UI/CosmicEdictWidget.h:81`). 그래서 끝 쪽에서는 꼴이 사라진다. 끝 쪽에 두려면 '<백성>의 살림: …' 줄을 두어야 하고, 그러면 '까닭 줄만 다르다'는 약속을 고쳐야 한다.
- **함께 고칠 것**: 헤더 :296의 마지막 마디 '그 판은 다른 질서의 끝으로 적힐 수 있다'를 지운다(T3). 꼴 낱말 `Endured` '견디어 남은 살림'은 공로로 얻은 남은 자로 읽힌다 → '눌림 속에 이어 간 살림'(T7, `[B-7x4]`의 글 감사 몫).
- **귀결**: 셋 가운데 무엇을 고르든 '어느 백성 / 없는 백성 / 여러 생애'의 세 규칙을 §4에 적는다. `[B-7x3]`의 `UI.Chronicle`에는 지워진 백성, 다스린 적 없는 생애, 두 백성을 다스린 생애의 경우를 더한다.

### 결정 14 — 레인의 단일 창구 (R4)
- **헤더 권장**(:271): 슬라이스 나에서 받는다(§2.8).
- **리뷰**: 권장은 그대로 둔다. 다만 **지금 승인할 문안에 모순이 하나 있다.**
  - 창구 규칙 '끊어서 어느 성계도 고립시키지 않는다'(:141)와 '파괴의 끊음도 이 창구로 옮긴다'가 부딪는다. 파괴는 죽은 노드의 레인을 모두 끊는다(branch `Sim/CosmicHazards.cpp:445-454`). 그래서 문안을 이렇게 고친다: '고립 금지는 살아 있는 성계의 Link/Unlink에만 걸고, 파괴의 끊음으로 이웃이 고립되는 것은 받는다'(F8 (i)).
  - K9: 레인이 바뀌면 `ClaimantOf`가 읽는 `ClaimAdjacencyLanes`(branch `Sim/CosmicHazards.h:138-142`)가 바뀐다. 레인 걸음 뒤 주장이 풀리거나 늦춰지지 않는다는 시험을 슬라이스 헤더에 둔다.
  - 모르는 성계로 잇는 Link가 `NoteKnown`을 쓰는지(규칙 2)를 정한다.
  - 노트와 설계서가 슬라이스 나에 둔 블록(VoidWeb·BeyondReach·WebReachLanes·OrbitShaper·Labyrinth)의 목록을 §2.8에 적는다(F8).
  - CLAUDE.md 규칙 13의 낱말은 슬라이스 나에서 넣는다. 노트 :179는 틀렸다(F12).
- **귀결**: 모순을 고치지 않고 승인하면 슬라이스 나의 헤더가 지금 문장을 번복해야 한다.

### 결정 15 — 좌표의 이동 (R5)
- **헤더 권장**(:272): 창구가 받은 뒤 둘째 걸음으로 간다.
- **리뷰**: `SystemDragger`가 두 꼴로 적혀 있다. 하나를 고른다(F8 (ii)).
  - 헤더 :142: 주기마다 딸린 성계의 레인 하나를 수도 쪽 이웃으로 옮긴다.
  - 설계서 S14 :281: `project_settle`이 끝난 틱에 수도의 레인 하나를 새 터의 이웃으로 옮긴다.
  - 리뷰는 어느 쪽도 강하게 권하지 않는다. S14의 꼴은 걸음이 사건에 매여 주기 노브가 필요 없고 측정하기 쉽다.
- **귀결**: 고르지 않은 쪽의 문장을 같은 커밋에서 고친다.

### 결정 16 — 판 도중의 탄생 (R6)
- **헤더 권장**(:273): 슬라이스 다에서 건넘 단계의 요청으로 받는다(§2.9).
- **리뷰가 더한 받는 조건**(T4):
  - (1) 태어난 백성은 새 `FCivilizationState`다. 새 `CivID`, 이름 뱅크의 새 이름, `TimeVectors` 0을 받는다. 부모의 `ArchetypeRow`·`Traits`(탄생 블록 하나만 빼고)·관계·칙령·`HostCivID`는 물려받지 않는다.
  - (2) 그 줄은 옛 백성이 살아남았다·죽지 않았다·돌아왔다·다시 태어났다고 말하지 않는다. rebirth_words·afterlife_words·'불사'·'유령'을 스키마 규칙 하나로 막는다.
  - (3) 고를 것: (3a) `FossilAutomata`는 `LetPeopleGo`(절멸)에서 태어나지 않고 글이나 스캔 얼굴로만 둔다. (3b) 거기서 태어나도 새 백성이고, 그 틱에 `LetPeopleGo`가 놓은 성계를 받지 않는다.
  - (4) 잎 `Undying*`은 선언하지 않는다.
- **F8 (vi)가 슬라이스 다 헤더에 물을 것**: 요청이 어디서 생기는가(틱을 건너면 저장 필드), CivID·이름(규칙 11)·아키타입 행·관계·`TimeVectors` 0(K10), `HostCivID` 정리, 품는 쪽 수입의 몫. 수입의 몫은 규칙 13 '정착된 성계' 술어를 굽히므로 R14로 둔다.
- **R6의 문장**: docs/20 main:289 'Civs의 원소 수는 건넘 단계에서만 바뀌고'는 그대로 선다. 굽는 것은 코드 사실 `:873-874` 하나다(F5 검증). 문장이 굽지 않으면 R에서 빼서 '새 꼴'(㉡)로 옮긴다.
- **귀결**: 조건 없이 승인하면 원안의 불사 분열과 유령 제국(설계서 :47·:88·:121)이 열린 채다. B6b가 지은 '마지막 성계의 문 → `LetPeopleGo`'(main B6b 헤더 :106·:422) 뒤에 같은 백성이 기계로 이어 사는 길도 열린다.

### 결정 18 — 받지 않는 다섯 (그리고 남긴 법의 귀결)
- **헤더 권장**(:275): §1.3 그대로 둔다. 앎을 읽는 붕괴는 세계의 사실로 발동하고 표현에서만 가린다. 거꾸로 흐르는 시간은 받지 않는다. 변이는 칸을 주지 않는다. 면역은 감쇠와 문의 사유로 받는다. '되는' 끝은 '남긴' 끝으로 받는다.
- **리뷰가 찾은 '남긴 법'의 귀결**(F1·I2·M11). 헤더 :112, 설계서 C10 :443, promptforother.txt:39-40은 셋 가운데 첫째만 적는다.
  1. **안정도 — 헤더가 적은 것.** C10 ①로 양위한 옛 백성은 섭정·AI 아래에서 물려받은 `edict_mobilization`을 판 끝까지 지닌다. 이 칙령은 안정도를 깎고, 유지비가 0이라 만료하지도 않는다. 안정도가 내려가 반란과 지워짐 쪽으로 간다. 리뷰가 고친 뒤에도 이 귀결은 **남는다**. 이것이 설계가 노린 '값'이다.
  2. **주장(K9) — 헤더가 적지 않은 것.** 같은 백성이 `edict_vigil`(`Hazard.Claim` +100, 유지비 0)과 `edict_machine_rights`(AI 각성의 `RulerResolutions` — 만료를 해결로 바꾸어 주장 규칙을 건너뛴다)도 판 끝까지 지닌다(branch `Tools/data/DT_Edicts.json:96`, `DT_Hazards.json:46`, `Sim/GalaxySimulationSubsystem.cpp:2542-2551`). 그 백성의 성계에서는 주장이 영영 100틱 늦고, 각성 만료 뒤의 주장은 서지 않는다. W의 수(철야 100은 이미 W 안 — main B6b 헤더 :221)는 그대로다. 그러나 '늦춤이 영원'과 '해결로 바뀌어 주장이 없음'은 K9가 바로 금하는 것이다. B6b 결정 9는 은하 법칙에서 이 고리를 금했다. 헤더 :112는 '백성 범위의 칙령까지 넓힌다'면서 그 선을 놓쳤다.
  3. **법칙 셋(다섯째 칸) — 헤더가 적지 않은 것.** 칙령 자리는 많아야 셋이다(main `Sim/CosmicEdicts.cpp:26-35`, `DT_Rules.json:75`). 법칙 셋은 모든 자리를 쓴다(`GalaxyLawsToHold` 3; main B6b 헤더 :143). 백성 범위 칙령이 하나라도 남으면 AI 법칙 걸음은 많아야 둘을 세우고, `LawsHeldTicks`는 0에 머문다. 그래서 법을 묻는 넷째 → 다섯째 행이 그 백성에게 닫힌다. 유지비 있는 칙령(machine_rights 0.2, open_thought 0.1)이 만료하면 풀리지만, mobilization·free_trade·vigil·quarantine·against_silence는 만료하지 않는다(M11 검증).
- **리뷰 권장 — 좁힘**(코1, 상시 승인 안). 남긴 법 백성이라도 (a) 주장 카운트다운에 더하거나 위기의 답인 칙령은 여느 AI처럼 거두고(B6b 결정 9와 같은 선), (b) 그 백성의 행이 법칙을 물으면(`CosmicEdicts::AsksForLawsOf`, main `:242`) 여느 AI처럼 거둔다(B6b 결정 10·11 — 칙령 자리 셋이 법칙 셋의 자리다). 그러면 귀결 2·3은 사라지고 1만 남는다.
- **대안**: '모든 칙령을 지닌다'를 고르면 K9를 굽히는 새 R(R11급)을 적고 W를 다시 센다. Data.Integrity 신정론 불변식에 그 경우를 더한다(main `Tests/CosmicDataTests.cpp:2218-2293`). 다섯째 칸이 그 백성에게 닫힌다는 것도 적는다.
- **결정 18이 함께 받을 다른 것**:
  - ①(관측 전에는 어디에나): 노트 :20 "'어디에나'는 표현만"을 "'어디에나'는 받지 않는다 — 표현도 무주 성계 위에 그 백성을 그리지 않는다; 남는 것은 S15의 소문 한 줄이고 그 줄은 '제 별 곁에만'으로 닫는다"로 고친다. 결정 18 문장에 "'어디에나'는 표현으로도 받지 않는다"를 더한다(T10).
  - 간접 경로를 적는다: 기생 덤이 이웃의 각성·역병·반란 만료와 그 뒤 주장 규칙을 늦춘다. 미시 곱이 각성을 줄여 그 뒤 주장도 준다(F2 — 문2 참조).
- **귀결 요약**: 권장대로 답하면, 양위한 백성은 안정도를 깎는 칙령을 지닌 채 섭정 아래에서 무너질 수 있다. 이것이 받는 값이다. 주장과 법칙의 문은 여느 백성과 같다. `-Crossroads` 칸(M7)이 안정도 0.2 아래 첫 틱·반란·지워짐·`LawsHeldTicks`를 수로 보인다. **수를 보고 답하려면 이 칸이 답보다 먼저다.**

### 결정 20 — 빌드 전 코드를 받는 길
- **헤더 권장**(:277): B6b가 끝난 main 위로 리베이스한다. `[B-7x1]`부터 커밋마다 다듬는다. 헤더는 '충돌은 노브의 끝·스키마·ST_CA.csv에서 난다'고 적었다.
- **리뷰가 바꾼 것**:
  - **충돌 목록이 양쪽으로 틀렸다**(I1). 헤더가 이름한 FCosmicRulesDef·EConditionType·ECosmicSettleBlock·CosmicSeed·ini의 끝은 충돌하지 않는다. 실제 충돌은 이렇다.
    - `[B-7x2]`: ST_CA.csv의 끝, schema.json의 `words`, test_data_schema.py의 `walked` 핀.
    - `[B-7x5]`: [B-6b8] 뒤의 DT_Events.json·DT_Encounters.json 끝, DT_Encounters rules의 끝(Rule W 뒤), rule_setups, `walked`.
  - 'main 쪽을 앞에 두고 양쪽을 다 살린다'를 `words`에 그대로 하면 깨진다. 쉼표가 빠져 JSON이 깨지거나, 쉼표를 넣어도 check_data가 '두 번 적힌 키'로 멈춘다(main `Tools/cosmic_data_schema.py:300-316`). 어느 쪽이든 브랜치의 8낱말 `holy_passive`는 [B-6b5]의 13낱말을 지우는 옛 값이다. 해소: main 줄을 그대로 두고 `time_vectors` 한 줄만 `words`의 끝에 더한다.
  - **차례**(I11): 브랜치 차례는 코드가 헤더보다 먼저이고, 데이터 초안이 표현·글·측정 스위치보다 먼저다. aa09543은 커밋 메시지대로 `Data.SourcesInSync`를 깬다. 그래서 그대로 리베이스할 수 없다. §4의 차례로 다시 쌓는다.
  - **`[B-7x5a]`는 제 커밋으로 남기지 않는다**: `[B-7x5s]`와 `[B-7x5c]`로 가른다(M8·build-1).
  - **변이 패치 일곱이 멈춘다**(I6). 다섯은 다시 뜨고, 둘은 `time_vectors`를 `words` 끝에 두면 산다.
  - **브랜치 최상위의 두 파일을 main에 들이지 않는다**: `promptforother.txt`(F12)와 `docs/OVERVIEW_as_of_B6a.html`(T1).
  - 반박 하나: `UI/CosmicSystemPanelWidget.cpp`는 깨끗이 합쳐진다(build-3 refuted). 다만 static_assert 12는 B6b가 정착 사유를 더했는지 보고 다시 읽는다.
- **귀결**: 권장 길(B6b 뒤 리베이스)은 그대로다. 충돌 문구, 해소 규칙, 커밋 차례가 바뀐다.

### 결정 22 — 새 추첨 지점 (R9)
- **헤더 권장**(:279): 가·나는 0, 다의 포자에 고정 수 하나(Phase 5 전에만).
- **리뷰**: 권장은 그대로 둔다. 다만 **결정 16의 조건 (1)·(2)를 먼저 적은 뒤에** 정한다(T4). 씨앗 블록은 추첨이 아니라 파생 정수라 이 결정 밖이다. docs/20과 `CosmicSeed.h`의 문장만 고친다(DET-1 검증 → 문16).
- **귀결**: 추첨 장부에 줄이 하나 는다. 포자가 '죽으며 흩어져 다시 자라는 같은 백성'이 되지 않게 하는 조건이 그 줄의 전제다.

### [개발자]로 올릴지 물을 것 (헤더에서는 [개발자]가 아니다)
- **결정 9**(조건 셋 — 국경): 개발자가 승인한 #88의 기본값 'VS에 두지 않는다'(main `OPEN_QUESTIONS.md:63`)를 이 둘만큼 연다. 리뷰는 R11로 적기를 권한다. 격상할지는 개발자가 정한다(F4).
- **결정 23**(씨앗 블록): 모든 AI 백성의 세계를 바꾸고(M1), #36 ② 'DT_Traits와 함께'(main `:33`)를 앞당긴다. R13으로 적고, '닿는 것'에 'B6b 경주의 입력과 AI의 셋째 칸 틱'을 더한다(F3·F4). 격상할지는 개발자가 정한다. DET-1 검증은 격상이 필요 없다고 본다.
- **결정 21**(설계서): `[B-7x0]`에서 원안(§1~§3.2, §4.1~§4.2)을 `docs/archive/EXISTENCE_EVOLUTION_original_2026-11-09.md`로 동결할지(T5), OVERVIEW html을 뺄지 다시 쓸지(T1)를 정한다. 둘 다 개발자의 글이라 확인이 든다.

---

## 3. 고칠 것 (confirmed·partial — partial은 검증자가 고친 안을 따른다)

표기: **[커밋]**은 그 고침이 드는 커밋이고, **[대기 N]**은 개발자의 답 N을 기다린다. 묶음 이름의 앞글자는 문(문서 글)·코(코드)·데(데이터·스키마)·시(테스트·측정 장치)다.

### 3.1 헤더·노트·설계서 글

**문1. R 목록을 표로 바꾸고 R11~R13을 더한다** (F4·F5·DET-1) — [B-7x0]
- 헤더 §0 :17을 표 하나로 바꾼다: `| R | 지금 문장(파일:줄, main) | 새 문장 | 넓이(무엇에만) | 지키는 시험(이름, 또는 '슬라이스 나/다 헤더에서') |`.
- 굽는 문장이 없는 것은 R에서 빼서 '새 꼴'(㉡)로 옮긴다. R2(누적 셈을 금하는 규칙이 없다)가 그렇다. R6은 코드 사실 `Sim/GalaxySimulationSubsystem.cpp:873-874`(헤더 :29)만 굽는다.
- 더할 행:
  - R11 #88: `BorderingCivHasTrait`, 기생의 두 국경 항, 표류 목표의 블록 항까지만이다. 다른 국경 배율은 여전히 EA다. 결정 9에 단다.
  - R12 B2 §2.6 'AI는 곧 철회한다'(main B2 헤더 :13): UnseenLaw 백성만이고, 코1의 좁힘과 함께다. 결정 18에 단다.
  - R13 #36 ②: 씨앗 블록 하나를 파생 정수로 고르고, 노브가 비면 꺼진다. 결정 23에 단다.
  - (슬라이스 다) R14: 품는 쪽의 수입 몫(규칙 13).
- 노트 §2(:96-117)에 R2·R7·R8·R9와 R11~R13의 행을 더한다.
- `[B-7x0]` 문서 목록(§7 :227)에 OPEN_QUESTIONS #88·#36 행의 고침과 B2 헤더 §2.6의 B7x 표지를 더한다.
- [대기: 결정 9·23을 [개발자]로 올릴지 — 표 고침 자체는 기다리지 않는다]

**문2. §2.4의 행 셋** — [B-7x0]
- UnseenLaw(:112) 끝에 더한다 (F1·I2·M11): "— 다만 주장의 카운트다운에 더하는 칙령과 위기의 답인 칙령은 왕좌가 빈 뒤 여느 때처럼 거두고(K9 — B6b 결정 9가 은하 법칙에 그 고리를 금지한 선과 같다), 판정받는 행이 법칙을 물으면(넷째 칸 — `CosmicEdicts::AsksForLawsOf`) 백성 범위 칙령은 다른 AI처럼 거둔다(칙령 자리 셋이 법칙 셋의 자리다, B6b 결정 10·11)". 같은 한 줄을 설계서 C10 귀결(:443)과 노트 §1.1 UnseenLaw 행에 넣는다. [대기 18]
- CrisisParasite(:106) 끝에 더한다 (F2·build-2): "이웃의 각성·역병·반란에 붙은 덤은 그 만료와 그 뒤의 주장 규칙을 늦춘다 — 철야의 `Hazard.Rebellion` +200과 같은 결이다; 주장의 카운트다운은 빼므로 W는 그대로다; 신정론의 4분의 1(가장 긴 머묾)이 이 덤을 센다(Data.Integrity) — 주장은 빠지므로 `ThroneLockTicks`의 한계는 그대로다".
- Inward(:108)를 고친다 (M9·I5·F2): "미시 쪽은 위기가 설 몫 곱(`BrickInwardHazardSpawnMultiplier` — 0은 안 된다; 주장·절멸·초신성 전조는 빼고 — 기생과 같은 제외)". 끝에 "각성의 몫이 줄면 그 만료 뒤의 주장도 준다 — `[B-7x5]`에서 #136과 함께 읽는다; 한 성계 안쪽 백성과 그 뒤 AI 손에 든 백성은 다시 정착할 수 없어 `Law.Remnant`에 남는다"를 더한다.

**문3. §5를 다시 쓴다** (M1·M2·M3·M4·M5·M5b·M6·M7·M8·M12·M14·I3·I4·I9·I10·F2·F3·T8c) — [B-7x0]

헤더 :212-216을 아래 글로 바꾼다. 문턱과 판단은 데이터를 켜기 전에 이 글 그대로 쓴다.

- **끈 짝**: `-WithoutB7xData=1`은 모두 끈다(카드 열둘·`event_brick_*` 아홉·`arch_seeker`의 `Brick.Form.Society`를 빼고, 노브 열일곱을 C++ 초기값으로, `BrickSeedTags`를 빈 값으로). `=2`는 카드·사건·Society만 끄고 노브와 씨앗은 켠 채 둔다. 사건 행은 씨앗을 받은 AI에서도 서므로 `=2`는 사건과 카드를 함께 잰다. 도우미 `CosmicTests::WithoutB7xData`는 측정 스위치이면서 **시험 보드의 무동작 쪽**이다(`WithoutB6bData`가 main `Tests/CosmicEndingTests.cpp:158`에서 하는 꼴). `-SaveAt` 거절 목록(main `Tests/CosmicBalanceCommandlet.cpp:1513-1517`의 조건과 Error 글)에 `-WithoutB7xData`를 넣는다. `-Crossroads`는 입력의 꼴이라 넣지 않는다.
- **깨질 수 있는 확인**(데이터를 켜는 커밋마다 — B6b `[B-6b6]` ⓐ~ⓒ의 꼴):
  - ⓐ 그 빌드에서 러너의 다섯 인자 + `-WithoutB7xData=1`이 앞 커밋의 다섯 줄을 글자 그대로(`wrote` 꼬리 빼고) 낸다. 묶음의 끈 쪽 CSV·`_rows.csv`·`_claims.csv`는 앞 커밋 빌드의 같은 시드·인자 판과 `TickMicros`·`SaveBytes`·`SaveRawBytes`를 빼고 같다. 켠 쪽과의 '첫 셋째 칸까지 같음'은 쓰지 않는다(씨앗이 생성 때 든다).
  - ⓑ 스위치 대조: 끈 로그마다 `CosmicBalance: tables without B7x's data - rows taken out [<카드 열둘> <사건 아홉>], Brick.Form.Society off arch_seeker, 17 knobs inert` 한 줄이 있고, 켠 로그에는 없다. 끈 쪽 `bricks:`의 AI 씨앗·갈림 카드·변이·`event_brick_*` 발화가 모두 0이고, 켠 쪽 합에서는 넷 모두 0보다 크다.
  - ⓒ 모든 로그의 `end check`·`race check`가 0이다.
- **정책**: 갈림 카드에서는 값 없는 선택(③)만 고른다. 블록을 주거나 걷는 선택도, 블록 없는 값 있는 선택(둘째 갈림의 ② 여섯, `enc_cross_drift_fleet` ②)도 건너뛴다.
  - `-Crossroads=<k>`(측정 전용)는 갈림 카드에서 `Brick.` 태그를 주거나 걷는 선택 가운데 값을 낼 수 있는 k번째(1부터)를 고른다. 없으면 ③이다.
  - 이 스위치가 있을 때만 `petition_cross_law_stays` ①이 `DeposeRuler`의 거름(main `World/CosmicRulerPolicy.cpp:270-281`)을 지난다(`-Abdicate`의 꼴).
  - 방랑자 정책의 건너뜀은 [B-6b8]의 `Pick = Counts.Answers % Row->Choices.Num()` 위에 짓는다.
- **새 줄 `bricks:`**(러너가 견주는 세 줄 밖)에 담을 것:
  - 판마다, 다스리는 백성과 AI 백성마다(지닌 씨앗 블록과 함께) 여섯 셈을 틱 1,200·2,400·C·끝에 낸다.
  - 온 갈림 카드의 수와 틱, 갈림 카드 답의 틱(틱:수), 고른 꼴, `petition_walls`·`petition_bread`가 온 수.
  - 행별 발화 수. 측정 쪽이 `OnEventFired`를 센다 — 세계를 쓰지 않고, `-TellAll`과 무관하다.
  - 코드 규칙의 셈: 기생 덤이 붙은 놓임 수, 기생 통찰이 든 틱, 삼키는·뒤엉킨 항이 0이 아닌 짝 수, AI 변이 수.
- **값보다 먼저 재는 것**:
  - 빌드: 씨앗과 노브만 켠 빌드로 잰다(카드·사건 행·Society 없이 — 커밋하지 않는 데이터 사본). 칸은 R 30·입력 없는 30·방랑자 30(시드 1~30, 20,000틱, `-Rules=ProvisionalEndTick:0`)이다.
  - **문턱의 규칙**(재기 전에 이 글 그대로):
    - 첫 갈림 카드의 문턱: R 30에서 그 셈의 틱 2,400 값의 40번째 백분위수를 100 단위로 내린다(아래 끝 900).
    - 둘째 갈림: 다스리는 셋째 칸 틱 + 1,200에서 같은 셈의 40번째 백분위수다.
    - 변이 문턱(노브 하나 `BrickMutationThresholdTicks`): 세 중앙값 가운데 가장 작은 것을 100 단위로 내린다. 각 중앙값은 부모 블록을 씨앗으로 지닌 AI 백성의 해당 셈의 틱 C 값이다(Nomad → Drifted, SwarmTech → min(Built, Linked), StarEater → Consumed).
    - 방랑자 카드: 방랑자 30(`-Abdicate=1200 -Wander=1`)에서 그릇이 다른 백성의 정착된 성계에 처음 선 틱마다 그 주인의 `Drifted`를 읽는다. 그 중앙값을 100 단위로 내린다(아래 끝 900).
    - C는 `[B-6b10]`이 정한 시계다(6,000 고정이 아니다 — I10).
- **미리 적은 판단**(30건 미만은 '재지 않음 — 잰 N'):
  - ⓪ 겹침 규칙마다 재는 칸을 적는다. S04·S06·S07·S11·S12는 R 30·입력 없는 30에서, S01·S05·S08은 `-Crossroads` 칸에서만 잰다. 칸 밖의 규칙은 '칸 밖'으로 적고 조건을 고치는 근거로 쓰지 않는다. 셈은 `_rows.csv`가 아니라 `bricks:`의 행별 발화와 코드 규칙의 셈으로 한다(M6).
  - ① 통치자 정책 30판에서 판의 끝 전에 갈림 카드가 한 장 이상 온 판이 20/30 이상이다(그대로).
  - ② 첫 갈림 카드의 틱 중앙값이 1,200~3,000 안이다(그대로).
  - ③ R 30·방랑자 30에서, 종류마다 30판을 합친 몫의 켠−끈 차이가 +2%p 이하다(95% Wilson 구간을 함께 적는다). 입력 없는 칸은 쓰지 않는다(M5).
    - 주장 카드: `TriggerClaimTaken ÷ TriggerClaim`(`spirit:` 줄). 따로 적고, 잃은 판마다 그 틱에 칸을 쥔 카드를 `pressure:`에서 적는다.
    - 잰 카드 `petition_pressure_measured`(Named 카드): `TriggerPressureTaken ÷ TriggerPressure`.
    - 안정도 청원 `petition_walls`·`petition_bread`: `StandingLost[row] ÷ (온 수 + StandingLost[row])`.
  - ③′ 방랑자 60 × 6,000(R + `-Abdicate=1200 -Wander=1`)과 끈 짝에서 `enc_witness`의 제시 수와 도착 카드의 수를 본다. 끈 짝 대비 줄어든 몫이 2%p 안이어야 한다. `enc_cross_drift_fleet`이 판마다 온 수도 적는다. 넘으면 그 카드의 `BaseWeight`를 0.25로 낮추거나 `CivTierAtLeast 2`를 더한다(I3).
  - ④ 러너의 다섯 인자를 켠 쪽과 끈 쪽(+`-WithoutB7xData=1`)으로 손으로 하나씩, 두 번씩 돌린다. `stages:`의 경제·AI µs와 틱 평균의 켠−끈 차이(두 번의 중앙값)를 본다. 틱 평균 +5% 안이면 통과다. 넘으면 기생의 국경 걸음과 레인 폭의 BFS를 먼저 본다. 병렬 묶음의 `TickMicros`로는 판단하지 않는다(M5b).
  - ⑤ R 30에서 판의 끝(기록의 틱, 없으면 C)까지의 결정 가운데 갈림 카드의 답을 뺀 수를 본다. 판마다 켠−끈 차이의 중앙값이 −1 이상이어야 한다. 또 다스리는 셋째 칸부터 첫 기록과 C 가운데 이른 것까지, 600틱 창마다 갈림 답을 뺀 결정 수의 판별 중앙값이 끈 쪽에서 1 이상이고 켠 쪽에서 1 미만인 판이 3/30 이하여야 한다(M4).
  - ⑥ **B6b의 판단을 다시 세운다**(B6b 헤더 §5.3 — 짝은 같은 시드의 `-WithoutB7xData=1`).
    - 칸: 묶음 B의 칸(R 60·느린 R 60·방랑자 60·입력 없는 60 × 20,000, 시드 1~60, `-Rules=ProvisionalEndTick:0`).
    - 판단: ①(40/60 이상), ②(다른 질서 15~35%, (가)·(나)·(다) 가름 그대로), ③(넷째 → 다섯째 중앙값 3,600 이하), 시계 규칙 ⓔ의 (i)·(ii)를 `[B-6b10]`의 C로 본다(C를 다시 셈하지 않는다). 첫 AI 셋째 칸 중앙값은 끈 짝의 ±10% 안이고, '다스리는 셋째 칸 − 첫 AI 셋째 칸'의 부호가 그대로여야 한다.
    - 적기만 할 것: 다스리는 둘째·셋째 칸 틱, 첫 AI 셋째·넷째 칸 틱, 제동이 막은 수, 절멸(다스리는·AI), AI 단계 µs, 묶음 C의 'C에서 읽는 것'(#136·#142·#132·B1 지킴·#129·#154·15% 미만).
    - 어긋나면 B7x의 노브를 먼저 고친다(씨앗 묶음에서 연구·통찰·에너지 블록을 빼거나 그 곱·더함을 낮춘다). B6b의 r·c·N·P_bind·C는 건드리지 않는다. 한 번 고쳐도 어긋나면 그 수와 함께 [개발자].
  - ⑦ 판의 끝 전 주장 수(끈 짝 대비 — S04의 약속, 설계서 :213)와 각성 지연 가운데 기생 덤의 몫(#132 띠 곁에)을 적는다(F2).
  - ⑧ 적기만: 스스로 사라진 백성 수와 초신성 업적 수(S01·S06), #129·#130(S09), 인구 바닥 틱(S12).
    - `-Crossroads` 칸(R + `=1`·`=2` 각 30, 짝은 같은 시드의 R)에서는 다음을 적는다: 안으로 접힌 둘의 셋째 칸 도달 몫과 틱, 양위 뒤 남긴 법 백성의 안정도 0.2 아래 첫 틱·반란·지워짐·`LawsHeldTicks`.
    - C11 ①을 고른 판의 틱 C 안정도와 받은 공성 수가 ②를 고른 판보다 낫지 않은지 본다(T8 (c)). 넘으면 그때 `ChangeStability +0.08`을 낮춘다.
  - **고침의 규칙**(M14): 판단이 어긋나 값을 고치면, 탐침은 시드 1~30(커밋하지 않는 사본)으로 하고 고친 값의 판단은 새 시드 31~60으로 한 번 한다. 탐침의 판은 판단에 쓰지 않는다.
    - ①이 못 미치면 문턱을 내린다.
    - ②가 1,200 아래면 문턱을 올리고, 3,000 위면 내린다(100 단위).
    - ③이 넘으면 갈림 카드의 문턱을 올리거나 `BaseWeight`를 0.5로 낮춘다(어느 쪽인지 수와 함께 적는다).
    - ④가 넘으면 경제 단계의 셈을 먼저 잰다.
    - ⑤가 줄면 [개발자].
    - 그래도 어긋나면 그 수와 함께 [개발자](③은 #158과 함께).
- 함께 고칠 곳: §2.6 :133과 §3.4 :196의 '시작값' 문장이 이 문턱의 규칙을 가리키게 한다. 결정 19(:276)는 '갈림 카드에서는 ③만'으로 고친다.

**문4. §6에 '뜻이 바뀌는 테스트' 표와 보드 규칙** (I7·DET-3·build-1·I3·I6) — [B-7x0]
- `[B-7x5]` 줄에 넣을 표: `Sim.SightWalkMoved`, EndingTables의 테스트들, `Sim.SuccessionPossible`, `Sim.WandererLoop`, `Sim.PhaseTwoExit`, `Feats.Reachable`, `Sim.WitnessCard`, `UI.Chronicle`, `Sim.SeedBricks`.
- 규칙: "B7x가 주제가 아니면 `CosmicTests::WithoutB7xData`를 가장 바깥에 둔다 — 시드와 기대값으로 흡수하지 않는다. `WithoutB6bData`가 이미 바깥에 있으면 그 바깥에 둔다(둘은 겹치지 않는 행과 노브를 빼므로 차례가 결과를 바꾸지 않는다)."
- `Sim.WandererLoop`과 `Sim.PhaseTwoExit`는 b6b8 §2.4의 선례대로 먼저 까닭을 찾는다. B7x가 까닭이 아닐 때만 끈다. B1 지킴이 바뀌면 [개발자] 발견으로 적는다.
- `[B-7x2]` 줄에 더한다: "보드는 노브를 무동작으로, 특성은 블록 없이 깐다 — 게임 값에 기대지 않는다".
- `[B-7x1]`·`[B-7x2]`·`[B-7x5]`에 더한다: "`git apply --check`로 모든 패치를 커밋 전후에 견준다. 서지 않게 된 패치마다 같은 변이를 `b7x<N>_` 이름으로 다시 뜨고, README의 옛 줄에 `(no longer applies since [B-7x2] - taken again as …)`를 단다(main `Tools/mutations/README.md:165`의 꼴)".

**문5. §8 표** (M8·M1·M12·F12·I9·I11·DET-2) — [B-7x0]
- `[B-7x0]` 행에 'promptforother.txt를 지운다'를 더한다. 노트 §4(:131-133)를 "다른 PC의 문구는 브랜치에서만 쓰였고 [B-7x0]에서 지웠다 — 차례는 헤더 §8"로 고친다.
- `[B-7x1]`·`[B-7x2]` 검증 칸을 고친다: "다섯 판의 CSV·`_rows.csv`·`_claims.csv`(TickMicros·SaveBytes·SaveRawBytes 빼고)와 `ending:`·`race:` 줄이 앞 커밋과 글자 그대로". `[B-7x2]`에는 선택 확인을 하나 더한다: 해셔가 `TimeVectors`·`HostCivID`를 건너뛰게 한 로컬 탐침 빌드로 다섯 판과 해시 판 둘을 돌려, 리베이스한 main의 `Tools/baselines/seed42.json`과 두 해시가 비트 그대로인지 본다. 탐침을 뺀 뒤 `-UpdateBaseline`을 돌리고, 커밋 메시지에 한 줄 적는다. 탐침은 `Tools/mutations` 아래에 두지 않는다(DET-2).
- `[B-7x4]` 뒤에 행을 더한다: `[B-7x4m]` | **측정**: 정책의 건너뜀(통치자·방랑자)·`-WithoutB7xData=1`·`=2`와 도우미·`-Crossroads=<k>`·새 줄 `bricks:` | xhigh | 에디터를 닫고 전체 빌드(`Tests/CosmicTestHelpers.h`·`World/CosmicRulerPolicy.h`); 러너 full 두 번; 비트 그대로(`-TellAll=0` 판 포함); `-SaveAt` 거절 목록; 변이(건너뜀을 뺌 — 테스트 안에서 지은 벤치 카드로; 끈 스위치가 씨앗·노브를 남김). §5 :215의 '(`[B-7x2]`에 새 줄 `bricks:`를 더한 것)'은 '(`[B-7x4m]`의 빌드)'로 고친다. §6의 '정책의 건너뜀'은 이 행으로 옮기되, `[B-7x5c]`에 실제 카드로 한 번 더 남긴다.
- `[B-7x5]` 행을 둘로 가른다:
  - `[B-7x5s]` 노브 열일곱의 값과 `BrickSeedTags`(AI만 바뀐다) | high | 러너 full `-Data` 두 번; 깨질 수 있는 확인; 값보다 먼저 재는 것; ⑥.
  - `[B-7x5c]` `Brick.Form.Society`·카드 열둘·사건 아홉 | high | 깨질 수 있는 확인; 판단 ⓪~⑤·③′·⑦·⑧; ⑥ 다시.
  - 행 하나를 더한다: `[B-7x5r]` | 노브를 바꾸면 제 커밋 | high | 미리 적은 판단; STATUS의 기준값 사실을 그 커밋이 고친다.
- '이 브랜치에 이미 있는 것'(:244)의 충돌 문구는 §4.2의 목록으로 바꾼다(I1). 차례는 §4.1로 바꾼다(I11).

**문6. §4 끝의 꼴 규칙** (F9·DET-5·T3·I12) — [B-7x0] [대기 13]
- 결정 13의 답을 §4 첫 항(:204)과 설계서 §4.4(:469)에 적는다(§2의 ㉢/㉠/㉣).
- 손 확인 5(:253)를 답에 맞게 고친다. ㉣이면 '패널과 [백성] 탭의 낱말이 칭호나 엔딩 이름으로 읽히지 않는가'로 고친다.
- 헤더 :296의 마지막 마디 '그 판은 다른 질서의 끝으로 적힐 수 있다'를 지운다.
- 설계서 §4.4 표에서 '§4.2 원안의 줄' 열을 지운다(T3·T5).

**문7. §3.2와 §3.5 — 꼴의 낱말과 금지어의 목록** (T7·I8·T9·T8a) — [B-7x0] 문장 / [B-7x4] 값
- §3.2 :181과 설계서 :487·:344: `Endured` '견디어 남은 살림' → '눌림 속에 이어 간 살림'. '오래 버틴 살림'은 쓰지 않는다. '버틴'은 main `schema.json:73` desolation_words('공로로서의 견딤')에 있다.
- §3.2 :181의 'Chronicle.과 같은 낱말 규칙'을 고친다: "`Shape.*`는 [B-6b5] 뒤의 Chronicle. 목록(rung_grasp_words·title_words·binding_words 포함)에 remnant_words·desolation_words를 더하고, 이 접두어에만 거는 새 목록 shape_merit_words `["견디","남은"]`를 건다". 견딘 목록을 테스트에 핀으로 박는다. '견디'를 desolation_words 자체에 더하지 않는다(그 목록은 다른 행 규칙에도 걸린다 — main `schema.json:410`·`:418`·`:595`·`:820-822`).
- §3.5 :200의 '낱말 목록은 schema.json의 것이 기준이다'를 목록 이름으로 바꾼다: silence_words·rung_grasp_words·title_words와 새 crossroads_words `['침묵','울타리','언약','방주','목자','순례','성소','불멸','하나가 되']`. 문장도 하나 더한다: '양위 뒤의 줄은 기다림·돌아옴·빈 자리의 그림을 두지 않는다'.

**문8. §3.3 규칙 목록** (F6·DET-4·M10) — [B-7x0]
- 규칙 7을 더한다: 'K4 — 업적 행의 완성 조건은 블록 태그를 묻지 않는다(`CivHasTrait`·`CivLacksTrait`의 `Brick.*`, `BorderingCivHasTrait`)'.
- :188 '새 블록 태그는 두 규칙에 함께 더한다'를 '세 규칙'으로 고친다.
- 규칙 6(:190)에 한 줄 더한다: '기생의 덤은 신정론 불변식의 가장 긴 머묾에 한 번 든다'.

**문9. §2.8·§2.9 끝에 '슬라이스 헤더가 답할 물음'** (F8·DET-9·T4) — [B-7x0] [대기 14·15·16]
- §2.8:
  - (i) 파괴의 끊음을 '고립 금지'의 예외로 적는다(§2의 14).
  - (ii) `SystemDragger`의 꼴을 하나로 맞춘다(§2의 15).
  - (iii) §2.10 표에 '(슬라이스 나)' 행 하나를 더한다.
  - (iv) VoidWeb/OmniLinked의 BeyondReach와 WebReachLanes, OrbitShaper Link, Labyrinth를 적는다. 아니면 노트 §1의 그 자리를 B7이나 글로만으로 옮긴다.
  - (v) K9: 레인 걸음마다 `ClaimantOf`·영향권·주장 규칙을 다시 읽고, 레인 걸음이 주장을 풀거나 늦추지 않는다는 시험을 둔다. 모르는 성계로 잇는 Link가 `NoteKnown`을 쓰는지 정한다.
- §2.9:
  - MirrorProjection과 S13의 세 문(일방 `TreatyBlockOf` HostWorld, 품긴 쪽의 정착 문, NonAggression)을 적는다.
  - 탄생 요청의 자리를 적는다. 틱 경계를 건널 때만 저장 필드가 든다.
  - CivID 규칙(`NextCivSerial`), 이름(규칙 11), `ArchetypeRow`, 관계, `TimeVectors` 0을 적는다.
  - 품음에 한 문장을 더한다: '건넘 단계가 지운 백성의 이름은 같은 걸음에서 모든 성계의 `HostCivID`에서도 비운다(추첨 없음 — `PendingContactCivID`를 비우는 `Sim/GalaxySimulationSubsystem.cpp:3131-3134`의 꼴); 품는 쪽만 남고 제 성계가 없는 백성이 건넘에서 지워지는지는 슬라이스 다의 명세에서 정한다'.
  - T4의 받는 조건 (1)·(2)·(4)와 (3a)/(3b)를 적는다. 수입 몫은 R14다.
- 노트 §5.3에 슬라이스 다의 테스트 한 줄을 예약한다: 품는 백성이 지워지면 `HostCivID`가 `NAME_None`이 된다.

**문10. 결정 3의 권장** (T2·F10) — [B-7x0] [대기 3]
- §2의 3대로 바꾼다. ini :176의 'QuietFeed proposed' 문구, 설계서 :111·:397·:400·:417·:462도 함께 고친다.

**문11. 결정 18 ①과 노트 처분표** (T10) — [B-7x0] [대기 18]
- 노트 :20 QuantumCloud의 받은 꼴을 고친다: "세계는 늘 확정 — 정착된 성계가 레인으로 닿거나 그릇이 서면 드러난다; '어디에나'는 받지 않는다 — 표현도 무주 성계 위에 그 백성을 그리지 않는다; 남는 것은 S15의 소문 한 줄이고 그 줄은 '제 별 곁에만'으로 닫는다".
- 마지막 열은 '받지 않음'으로 두고 '(㉡은 뒤 블록)'은 지운다.
- 결정 18(:275)에 "'어디에나'는 표현으로도 받지 않는다(S15의 소문 한 줄뿐)"를 더한다.

**문12. 설계서 원안의 동결과 OVERVIEW html** (T5·T1) — [B-7x0] [대기: 개발자의 글이라 확인]
- 설계서의 원안(§1~§3.2, §4.1~§4.2)을 글자 그대로 `docs/archive/EXISTENCE_EVOLUTION_original_2026-11-09.md`로 옮긴다. `docs/10_DESIGN`에는 받은 꼴만 남긴다(§3.3, §4.3, '원안의 줄' 열을 뺀 §4.4, §5)와 링크 한 줄.
- **CLAUDE.md에 줄을 더하지 않는다**(16KB 예산 — 헤더 §7 :226 '36바이트만 남았다'). 있는 `docs/archive` 행을 늘리지 않게 고쳐 쓰거나('옛 판·원안의 동결본(고치지 않는다)'), `docs/SECTION_MAP.md`에만 적는다.
- `docs/OVERVIEW_as_of_B6a.html`(branch :670-726의 B7x 절 — '항성 유목신'·'살아있는 방주-세계'·'즉시 현현'·'언약 자체가 되어'·'언약의 시간'·'불멸'·'헌정')은 둘 가운데 하나로 처리한다. ㉮ 브랜치에서 뺀다. ㉯ 받은 꼴로만 다시 쓰고 설계서 :10과 같은 상태 표지를 둔다.
- 헤더 §7(:224-228)의 머리 주장 '이 브랜치는 살아 있는 문서를 건드리지 않았다'를 고치고, 이 파일을 `[B-7x9]` 재감사 목록에 더한다.

**문13. docs/20과 `CosmicSeed.h`의 문장 — 씨앗 블록** (DET-1) — [B-7x1] (문서 먼저)
- docs/20 :204 ①의 '생성' 뒤에 '(AI 백성의 씨앗 블록은 ③ — B7x)'를 넣는다.
- :208 ③ 목록의 '건너온 백성의 이름과 아키타입' 뒤에 'AI 백성의 씨앗 블록(B7x, 용도 13 Brick — 이름 CivID, 번호 0, 태그는 이름 차례; 생성 때 세계의 사실을 정한다, `BrickSeedTags`가 비면 없음)'을 넣는다.
- :220 '시뮬 스트림 Initialize(Seed)에서 생성된다' 뒤에 '(AI 백성의 씨앗 블록만 파생 정수 — B7x)'를 넣는다.
- 헤더 §7(:226)의 '[B-7x1]과 함께'에 '파생 정수 용도 13(docs/20 ①의 생성과 ③ 목록)'을 더한다. 노트 :179와 맞춘다.

**문14. 낡은 사실** (I10·build-5·F12) — [B-7x0]
- 테스트 수: '223 → 232'(:33·:221, 노트 :179)를 '[B-6b12] 끝의 STATUS.md 수 + 9'로 바꾼다. [B-6b8]이 `Sim.EraEndured`·`Sim.WitnessCard`를 더한다.
- 열린 물음 번호: '#167부터'(:45)를 'OPEN_QUESTIONS의 마지막 번호 다음부터'로 바꾼다.
- 스키마 줄 번호를 B6b가 끝난 main에서 다시 적는다(10205b6에서는 :291·:1245·:1286-1287).
- `seed42.json:13`은 [B-6b6]이 고쳐 쓴다. [B-6b1]의 꼴은 DECISIONS_LOG/CHANGELOG의 줄로 인용한다.
- §5의 6,000은 C로 바꾼다(문3).
- 노트 §5.2 4(:165): '연구의 몫이 셋으로 고르고'를 '연구의 몫이 넷으로 고르고(ResearchFields 넷 — Physics·Society·Engineering·Transcendence, 각 1/4; GalaxySimulationSubsystem.cpp:46·880)'로 바꾼다.
- 노트 §5.3 7(:179): 'CLAUDE.md 규칙 13'을 '(CLAUDE.md 규칙 13은 슬라이스 나 — 헤더 §7)'로 바꾼다.

**문15. 결정 23의 '닿는 것'** (F3) — [B-7x0]
- :280에 'B6b 경주의 입력과 AI의 셋째 칸 틱(⑥이 다시 잰다)'을 더한다. R13의 번호도 단다(문1).

**문16. §9 손 확인** (M13·F7) — [B-7x0]
- §9 머리에 적는다: "세이브는 데이터를 켠 마지막 커밋 뒤 `-SaveAt`으로 만든다(R + `-Seed=<시드> -SaveAt=<틱>`; 갈림을 이미 고른 세계가 필요한 ④·⑥은 `-Crossroads=1`도). 시드와 틱은 §5의 30판에서 골라 노트 §5에 명령 줄·기대 틱과 함께 적고, 마지막 코드 커밋의 빌드로 다시 만든다(B6b §9 2의 꼴). ④는 `petition_cross_isolated`(Isolated 1,500·`Brick.Form.Society`)가 오는 틱 − 10 또는 그 답 뒤, ⑥은 `petition_cross_law_stays`(셋째 칸·Endured 2,000)가 오는 틱 − 10이다."
- ①을 고친다: "시드 42 보통 판의 첫 10분(틱 600)의 결정·말해진 줄이 커맨드렛 `ruler:` 줄의 decisions by 600·told by 600(켠 쪽의 시드 42 값과 30판 중앙값을 노트에 적어 둔다) 언저리이고, 갈림 카드·`event_brick_*` 줄이 그 안에 오지 않는가".
- 3(:251)의 '다시 같은 카드가 오지 않는가'를 F7의 답에 맞춘다.

### 3.2 코드 (리베이스 때 — `[B-7x1]`·`[B-7x2]`는 하나로 합쳐 다시 쌓는다, §4)

**코1. `CosmicEdicts::AIRevokesOf`의 UnseenLaw 갈래를 좁힌다** (F1·I2·M11·F4·T6·F11) — [B-7x2] [대기 18 — 권장안은 상시 승인 안]

branch `Sim/CosmicEdicts.cpp:344-366`을 아래로 바꾼다(main `Sim/CosmicEdicts.h:104` `AsksForLawsOf`, `Data/CosmicDataTables.h:131` `IsHazardAnswer`, `Data/CosmicDefinitionRows.h:426` `TMap<FGameplayTag, int32> HazardCountdownBonus`가 있다).

```cpp
TArray<FName> CosmicEdicts::AIRevokesOf(const FCivilizationState& Civ, const FCosmicDataTables& Tables, const FCosmicRulesDef& Rules)
{
	TArray<FName> Revokes;
	// A people that keeps the edicts it holds when no one rules it (B7x [B-7x2], Brick.Form.UnseenLaw) keeps them - except an
	// edict that adds to the claim's countdown or answers a crisis (K9, the line B6b decision 9 drew for galaxy laws), and
	// except while its row asks for galaxy laws (the slots are the laws' places, B6b decisions 10-11).
	const bool bKeepsEdicts = Civ.Traits.HasTagExact(CosmicTags::BrickFormUnseenLaw()) && !AsksForLawsOf(Civ, Tables);
	for (const FName Row : Civ.ActiveEdicts)
	{
		const FEdictDef* Def = Tables.FindEdict(Row);
		// A galaxy law stays (B6b decision 11): the AI's law step enacts the laws its row asks for, and keeps them.
		if (!Def || Def->Scope == ECosmicEdictScope::HomeGalaxy)
		{
			continue;
		}
		if (bKeepsEdicts)
		{
			if (Def->HazardCountdownBonus.Contains(CosmicTags::HazardClaim()) || Tables.IsHazardAnswer(Row))
			{
				Revokes.Add(Row);
			}
			continue;
		}
		if (!HasStabilityCost(*Def) || Civ.Stability < Rules.AIEdictRevokeStability)
		{
			Revokes.Add(Row);
		}
	}
	return Revokes;
}
```
- 같은 커밋에서 할 것:
  - branch `Sim/GalaxySimulationSubsystem.cpp:2954-2956`의 TickCivAI 주석('The AI holds no edict of the people's scope')에 '- except a people that carries Brick.Form.UnseenLaw (B7x), which keeps what CosmicEdicts::AIRevokesOf leaves it'를 단다.
  - `CosmicEdicts.h`의 `AIRevokesOf` 선언 주석에도 같은 예외를 적는다.
- 대안(개발자가 '모든 칙령'을 고르면): 코드는 그대로 두고 R11급의 새 R을 적는다. 신정론 불변식에 그 경우를 더하고 W를 다시 센다.

**코2. 미시 곱에서 주장·절멸·전조를 뺀다** (M9·I5) — [B-7x2]
- branch `Sim/CosmicExistenceBricks.cpp:395`: `if (Owner.Traits.HasTagExact(CosmicTags::BrickSpacePlanckMicro()) && !IsSparedHazard(Hazard))`.
- `Sim/CosmicExistenceBricks.h:111-114` 문서 주석에 'never the claim, the extinction or the supernova precursor'를 더한다. 별의 위기 곱(`BrickStellarCrisisSpawnMultiplier`)은 설계대로 둔다.
- [대기: 개발자가 절멸에도 감쇠를 원하면 — B6b 결정 2~5와 경주 ② (나)에 닿는다]

**코3. 기생 덤의 상한을 읽는 자리 하나** (DET-4·M10·build-2) — [B-7x2]
- `Sim/CosmicExistenceBricks.h`의 다른 `CosmicBricks` 선언과 같은 꼴로 `int32 ParasiteBonusBoundOf(const FCosmicRulesDef& Rules, const FGameplayTag& Hazard)`를 둔다. 태그가 유효하지 않거나 `IsSparedHazard(Hazard)`이면 0을, 아니면 `FMath::Clamp(Rules.BrickParasiteCountdownBonusTicks, 0, 100000)`을 답한다.
- `ParasiteCountdownBonusOf`(branch `:337-339`)는 그것을 쓴다. 한 위기에 한 번이고, 기생 수만큼 더하지 않는다.
- **스키마 max(branch `schema.json:247` 100000)와 지킴 상한으로 묶는 것은 하지 않는다**(F2 검증 — 칙령 덤 열도 상한이 없고, #132는 함대 몫의 띠다).

**코4. 표류 목표의 블록 항을 자르지 않는다** (DET-6) — [B-7x2]
- branch `Sim/CosmicDiplomacy.cpp:288-295`의 자르기를 `if (BrickTerm != 0.f) { Target += BrickTerm; }`로 바꾼다.
- .cpp 주석(:288-289)은 'a term added like the stance term; the drift stage clamps the relation'으로 고친다. `CosmicDiplomacy.h:109`에서는 '- when it is not 0 the sum is clamped to [-1, 1]'를 지운다.
- `GalaxySimulationSubsystem.h:501` 'within [-1, 1]'은 표류 결과를 말하므로 그대로 둔다.

**코5. `IsBrick`의 대소문자** (DET-7) — `IsBrick`이 든 커밋
- branch `Core/CosmicTags.h:305`: `return Tag.IsValid() && Tag.GetTagName().ToString().StartsWith(TEXT("Brick."), ESearchCase::IgnoreCase);`.
- 프로젝트의 다른 접두어 검사와 같은 꼴이다(main `Player/PlayerRoleSubsystem.cpp:547`, `Log/CosmicChronicle.cpp:230`). 계층으로 묻는 길도 엔진에서 된다(UE_5.8 `GameplayTagsManager.cpp:1541-1556`·`:2409-2413`). 그 길로 가면 `Tag.MatchesTag(Root) && Tag != Root`로 쓴다.

**코6. 주석과 DevComment** (DET-8·T6·F11·DET-1) — 각각의 커밋(비트 그대로)
- branch `Data/CosmicDefinitionRows.h:1411-1412`를 바꾼다: '...and with these values a people that carries a share brick lives as before too - but not one folded inward (Brick.Space.HermitMonolith / PlanckMicro: the settle gate's Inward, CosmicMegaprojects) or one that keeps the unseen law (Brick.Form.UnseenLaw: CosmicEdicts::AIRevokesOf); those act on the tag alone.'
- branch `Config/DefaultGameplayTags.ini:173`: 'Form brick: a people whose held edicts outlive an empty throne - no regent or AI of it revokes one (CosmicEdicts::AIRevokesOf), save those that touch the claim or answer a crisis. Never a people that becomes law, never the holy order (K6). Read by AIRevokesOf, the end shape (KeptLaw) and conditions (B7x). Saved name, never renamed.' 결정 3의 이름으로 바꾼다.
- `:172`: 'Form brick: a people that shut its star inside a sphere and turns its heat to reckoning. …'(읽는 곳은 지금대로).
- 테스트 메시지 branch `Tests/CosmicExistenceBricksTests.cpp:658`·`:1039`: 'The shut sphere round its star …'.
- branch `Core/CosmicSeed.h:3-6` 머리 목록에 ', the seed brick of an AI people at generation (B7x - a saved trait the sim stream does not draw; nothing while DT_Rules.BrickSeedTags is empty)'를 더한다.
- ini는 `generated.sha1`에 들지 않는다(main `Tools/make_data_tables.py:267`).

**코7. 잎·노브·함수 이름** (T2·F10) — [B-7x1](태그·접근자)·[B-7x2](노브·함수) [대기 3]
- 결정 3의 답대로 바꾼다. ini, `CosmicTags.h` 접근자, 스키마 규칙 4의 나열, `FCosmicRulesDef`·`DT_Rules.json`·docs/30의 노브 이름, `IsQuietOf`, 테스트(:576-591·:923·:960)를 함께 바꾼다.

**코8. 측정 장치** (M2·M4·M5·M6·M7·M8·M12·I7·I3) — 새 커밋 [B-7x4m] (§3.1 문3·문5의 명세)
- 통치자·방랑자 정책의 건너뜀('갈림 카드에서는 ③만'), `-Crossroads=<k>`와 `law_stays` ①의 `DeposeRuler` 통과, `-WithoutB7xData=1`·`=2`, `CosmicTests::WithoutB7xData`, `bricks:` 줄(측정 쪽 `OnEventFired` 셈, 사람마다·틱마다의 셈, 갈림 답 틱, walls/bread의 온 수), `-SaveAt` 거절 목록.
- 도우미의 트립와이어: `Data.Integrity`에서 뺀 집합(카드 열둘·사건 아홉·Society·노브 열일곱의 초기값·빈 `BrickSeedTags`)을 정확히 핀으로 박는다.

### 3.3 데이터·스키마

**데1. 둘째 갈림 ②를 닫는다** (F7·M7) — [B-7x5c] [대기 11 — (가)를 고를 때만]
- (나, 리뷰 권장): `petition_cross_dust_band`·`_star_line`·`_between_stars`·`_into_dust`·`_law_stays`·`_seam`의 ②에서 되풀이 이득을 뺀다. 적어도 `_law_stays` ②의 `PlayerStatDelta Exaltation +1`은 뺀다. `petition_road`/`feast`처럼 값과 작은 효과만 남긴다.
- (가): ②에 `SetFlag Flag.Cross.<행>`을 주고 행 조건에 `FlagNotSet Flag.Cross.<행>`을 둔다(branch `Data/CosmicConditionTypes.h:62-63`·`:168`에 이미 있다). 태그 여섯을 ini에 DevComment와 함께 넣고, 헤더 §2.11 :164 '열일곱'을 23으로 고친다.
- 어느 쪽이든 schema.json:1273-1275 규칙의 `then`은 넓히지 않는다(검사기에 OR 꼴이 없고 필요도 없다). `why`만 바꾼다: 'a crossroads petition closes on its branch: the first by taking Brick.Form.Society, the second by the tag its first choice gives or (choice 2) by Flag.Cross.<row>; a dismissed card (no cost, no effect) comes again after the last three answers'. (나)이면 마지막 마디를 'the second by the tag its first choice gives; its choice 2 carries no repeatable gain'으로 쓴다.

**데2. C10·C11의 글과 값** (T8·T7) — [B-7x4](노트의 초안 감사) → [B-7x5c](행에 옮김)
- C10 제목: branch `DT_Encounters.json:3048`과 설계서 :444의 '비워 둔 왕좌' → '왕좌 없이 서는 칙령'.
- C10 ① SuccessText(:3113)를 바꾼다: '{Civ}|hpp(은,는) 정해 둔 칙령을 손대지 않은 채 두었다. 다스리던 이는 자리에서 내려왔다.'
- C10 ②의 `ShiftEthics Spiritualism +0.05`(:3150-3156)를 뺀다. 설계서 :438·:441에 '다스리는 이의 이름을 높이는 일은 윤리 축을 옮기지 않는다(petition_exalt)'를 적는다.
- 목소리의 '견딘 날들 내내'(:3185-3186) → '눌린 날들 내내'로, `petition_cross_into_dust`의 '견딘 날이 길었다' → '떠도는 날이 길었다'로 바꾼다.
- C11 ①의 값(`ChangeStability +0.08`)은 지금 고치지 않는다. ⑧의 측정 뒤에 정한다.

**데3. 스키마 규칙** — 각 커밋
- **K4 업적 규칙** (F6) [B-7x2]: DT_Feats rules에 하나를 더한다. why는 'K4 - a rung comes from a feat's deed, never from a brick or a mutation (B7x header K4)'다. `lacks_type` `BorderingCivHasTrait`, 그리고 열일곱 `Brick.` 태그마다 `lacks_cond` {CivHasTrait, Brick.X}와 {CivLacksTrait, Brick.X}를 둔다(규칙 4의 나열 꼴). `CivTimeVectorAtLeast` 금지는 K4 글 밖이라 넣지 않는다. `AscensionCost`의 `AddTrait`/`RemoveTrait Brick.*` 금지는 선택이다.
- **`Settle.Block.Inward`** (I8) [B-7x2]: `string_table.checked_prefixes`의 `Settle.Block.RealmCapLast` 뒤에 `"Settle.Block.Inward": {"forbidden": ["verdict","holy_words","rescue_words","rung_grasp_words","title_words"], "no_digits": true}`를 둔다.
- **갈림 낱말 규칙** (T9) [B-7x4]: `crossroads_words`를 한 번 적는다. Conditions에 `CivTimeVectorAtLeast` 또는 `BorderingCivHasTrait`를 지닌 행에 건다. DT_Encounters의 갈림 열둘은 Title·Body·VoiceBodies·Choices.Label·SuccessText·FailureText, DT_Events의 `event_brick_*` 아홉, DT_LogTemplates의 `log_brick_scan_spread`가 대상이다. 그 칸들에 silence_words·rung_grasp_words·title_words·crossroads_words의 낱말이 들지 않게 한다. rung_grasp_words와 title_words는 [B-6b5](10205b6)에서 생겼으므로 리베이스 뒤에 넣는다.
- **`Shape.`** (T7·I8) [B-7x3]/[B-7x4]: 문7의 목록과 shape_merit_words.
- **탄생 행의 낱말 규칙** (T4) [슬라이스 다]: rebirth_words·afterlife_words·'불사'·'유령'.

**데4. `[B-7x5a]`를 가른다** (M8·build-1·I11) — 리베이스 때 [대기 20]
- aa09543은 커밋으로 옮기지 않는다. 내용을 `[B-7x5s]`(노브 값·`BrickSeedTags`·변이 노브)와 `[B-7x5c]`(`arch_seeker`의 Society·카드 열둘·사건 아홉)로 나눠 다시 적는다.
- `[B-7x5s]` 전까지 데이터의 노브는 무동작이고 `BrickSeedTags`는 비어 있다(build-1의 대안).

**데5. `enc_cross_drift_fleet`** (I3) — [B-7x5c], 조건부
- ③′가 넘으면 `BaseWeight` 0.25로 낮추거나 `CivTierAtLeast 2`를 더한다.

### 3.4 테스트

**시1. 보드는 노브를 무동작으로, 특성은 블록 없이 깐다** (DET-3·build-1) — [B-7x2]
- `Tests/CosmicTestHelpers.h`에 둘을 둔다.
  - `WithInertBricks(Rules)`: Rules를 베끼고, 수 노브 열여섯을 기본 생성한 `FCosmicRulesDef`의 값으로 되돌리고, `BrickSeedTags.Reset()`을 부른다.
  - `StripBricks(State)`: 각 `Civ.Traits`에서 `CosmicTags::IsBrick` 태그를 걷는다. 아키타입 태그는 둔다. 0번 백성의 `Brick.Form.Society`도 걷힌다.
- branch `Tests/CosmicExistenceBricksTests.cpp:310`은 `WithInertBricks(WithoutStartingEdict(*Game))`, `NewGalaxy(42, 12, 4)` 바로 뒤에 `StripBricks(State)`를 둔다.
- 게임 규칙을 읽는 공유 시드 42 보드에도 같은 둘을 둔다. main `Tests/CosmicDiplomacyTests.cpp`의 LayBoard(:132)부터다. :389 `Sim.DiplomacyRules`는 −0.12 → 0.13으로 깨질 것이다. 그다음 데이터를 다시 만든 에셋으로 모음 전체를 한 번 돌려 나머지를 찾는다.

**시2. `Sim.SeedBricks`** (DET-3·build-1·I7) — [B-7x2]
- Pass 0은 `WithInertBricks(*Game)`로 돈다.
- Pass 1은 그 사본에 `Seeding.BrickSeedTags = Rules.BrickSeedTags`를 준다.
- 기대값은 백성마다 '아키타입 행 `Traits`의 `Brick.*` 수 + (Pass 1이고 CivIndex > 0이면 1)'이다. AI 아키타입 `arch_default`에는 블록이 없다.

**시3. 신정론 불변식이 기생 덤을 센다** (DET-4·M10·build-2) — [B-7x2], **[B-6b8b] 뒤**
- `Tests/CosmicDataTests.cpp` TheodicyProblems의 칙령 고리 뒤에 `Longest += CosmicBricks::ParasiteBonusBoundOf(Rules, Tag);`를 둔다. `#include "Sim/CosmicExistenceBricks.h"`를 더한다.
- 주석: '…and the crisis parasite's placement bonus (CosmicBricks::ParasiteCountdownBonusOf — once a hazard, never for the claim, the extinction or the supernova precursor, B7x); the ThroneLockTicks bound is unchanged while only claim cards delay.'
- Expect 경우: main 데이터에서 역병(카운트다운 200 + 지킴 상한 100)에 덤 400이면 70슬롯이고, 0.98^70 ≈ 0.243 < 0.25라 문제여야 한다. 덤 30은 통과한다.
- B6b 끝의 데이터로 다시 센다. [B-6b7]의 법칙 덤 50이 들면 역병의 가장 긴 머묾이 350이 된다(`scratchpad/b6b7/brief_final.md:182` 'Plague's longest stand = countdown 200 + guard cap 100 + 50 = 350'). 그러면 깨지는 문턱이 덤 약 330~340으로 내려간다. `scratchpad/b6b8b/w_recount.py`에 같은 항을 더해 확인한다.
- 변이: `Tools/mutations/B7x/`에 덤 400 → crisis_plague에서 `Data.Integrity`가 깨지는 패치를 둔다.

**시4. 남긴 법의 칙령** (F1·I2·M11) — [B-7x2]
- `Sim.MutationStep`(branch `:987-998`)을 넓히거나, 새 `Sim.KeptLawEdicts`를 둔다.
  - (a) UnseenLaw AI 백성은 `edict_vigil`과 `edict_machine_rights`를 거두고 `edict_mobilization`은 지닌다.
  - (b) UnseenLaw AI 백성이 Society 1.0 이상(자리 셋)이고 넷째 칸이며 `CivLawsHeldTicksAtLeast`를 지닌 행 아래에 있다. 안정도는 `AIEdictRevokeStability`(1.0) 아래이고, 유지비 없는 백성 칙령 하나와 넉넉한 비축을 지닌다. AI 단계 뒤 법칙 셋을 지니고 `HoldsGalaxyLawsOf`가 참이다. 대조로, 좁힘을 빼면 법칙이 둘에서 멈추고 `LawsHeldTicks`가 0이다.
- 변이: `b7x2_unseen_law_keeps_claim_edicts`와 `b7x2_unseen_law_keeps_slots`.

**시5. 미시 곱의 제외** (M9·I5) — [B-7x2]
- `Sim.BrickShares`에 경우 하나를 더한다: PlanckMicro 주인의 `Hazard.Extinction`·`Hazard.PreSupernova` 몫이 블록 없는 주인과 같다. 변이 `b7x2_inward_damps_extinction`.

**시6. `DimCapitalStar`의 거절** (build-4) — [B-7x2]
- branch `Tests/CosmicExistenceBricksTests.cpp:709`('one class') **뒤에** 넣는다. :708 바로 뒤에 넣으면 :709가 깨진다.
- 차례:
  - `StarClass = StarClassAt(4)`로 되돌린다.
  - 정착 중: `ActiveMegaproject = CosmicRows::ProjectSettle` → TestFalse('not while the capital is being settled'), 다시 `NAME_None`.
  - 남의 수도: `Civs[1].HomeSystemID = Systems[6].SystemID` → TestFalse('not a capital another people owns'), 되돌림.
  - 사람 없음: `DimCapitalStar(*Board, Index, INDEX_NONE)` → TestFalse.
  - TestTrue('(the class stayed)')로 등급이 그대로인지 본다.
- 변이: 주인·정착 가드를 뺀다.

**시7. 표류 목표** (DET-6) — [B-7x2]
- branch `:678`은 `Before - 0.4f`를 Clamp 없이(1e-6) 기대한다. `:692-693` '[-1, 1] 안'은 지우거나 바꾼다.
- 기본 목표가 −1 아래인 보드를 하나 둔다(윤리 거리 2 + 적대 태세). 삼키는 항이 목표를 항만큼 정확히 내리는지 본다.

**시8. 스키마 검사기 테스트** (F6·I8·T9·T7) — 각 커밋
- `Tools/test_data_schema.py`에 깨지는 경우를 하나씩 둔다: 업적 행에 `CivHasTrait Brick.Mutation.AbyssalMaw`(이름은 결정 3 뒤의 것), `Settle.Block.Inward`에 금지어, `crossroads_words` 목록마다 하나, `shape_merit_words` 하나. `Settle.Block.Inward`와 `Shape.`의 금지 목록은 핀으로 박는다.

**시9. 연대기** (F9·DET-5·I12·T3) — [B-7x3] [대기 13]
- `UI.Chronicle`에 지워진 백성, 다스린 적 없는 생애, 두 백성을 다스린 생애의 경우를 더한다.

**시10. 갈림 카드** (F7) — [B-7x5c]
- `Sim.CrossroadsPresent`에 '② 뒤 그 카드가 다시 오지 않는다'((가)) 또는 '② 뒤 되풀이 이득이 없다'((나))를 넣는다. `Sim.KnowingInert`에 갈림 카드의 경우를 넣는다.

**시11. 뜻이 바뀌는 테스트** (I7·I3) — [B-7x5s]·[B-7x5c]
- 문4의 표대로 돌린다. `Sim.WitnessCard`([B-6b8], 게임 조우를 씀)를 다시 돌리고, 깨지면 가장 바깥에 `WithoutB7xData`를 둔다. B6b 노트 §5에 '증언 카드는 더는 유일한 방랑자 상시 카드가 아니다'를 적는다.

**시12. 멈춘 변이 패치 다시 뜨기** (I6) — [B-7x2]
- 다시 뜰 다섯: `B3/b3b_no_trade_income`, `B6b/b6b2a_data_laws_knob_four`, `b6b2a_endured_window_inclusive`, `b6b2a_laws_counted_before_upkeep`, `b6b2c_delay_second_rung`.
- `time_vectors`를 `words` 끝에 두면 `b6b5_schema_holy_passive_unextended`와 `b6b5_schema_title_without_messiah`는 산다(검증).
- [B-6b6]~[B-6b8]의 새 패치(Rule W, 시대·증언 행, ark_words)는 리베이스 뒤 apply-before/after로 다시 견준다.

---

## 4. 리베이스 계획

### 4.1 차례 (B6b가 끝난 main — `[B-6b12]` 뒤, 지금 10205b6 + [B-6b6]~[B-6b12])

| 차례 | 커밋 | 무엇으로 짓는가 | 증명할 것 |
|---|---|---|---|
| 0 | (리베이스 전) | `git merge-tree`로 B6b가 끝난 main과 브랜치를 견준다(읽기만). §4.2 목록을 다시 확인한다 | 충돌 자리가 §4.2와 같은가 |
| 1 | `[B-7x0]` 승인 | 4555c89의 헤더, 9be5b3e의 설계서, 0bffb8e의 문서에 §3.1 문1~문16과 개발자의 답을 반영한다. `promptforother.txt`는 뺀다. OVERVIEW html은 빼거나 다시 쓴다(T1). 설계서 원안은 동결본으로 옮긴다(T5). OPEN_QUESTIONS #88·#36·새 행, B2 §2.6 표지, systems.md:381, docs/50, DECISIONS_LOG, STATUS | `python -I Tools/check_docs.py` 0, CLAUDE.md 크기, 코드 없음 |
| 2 | `[B-7x1]` 필드 | 01eef6f에 결정 3의 이름(태그·접근자), 결정 13 ㉠이면 `ReignShapes`, 결정 6 (ㄷ)이면 탄생 요청 필드를 더한다. docs/20(저장 문단, 문13의 씨앗 문장)·docs/21을 먼저 고친다 | 에디터를 닫고 전체 빌드. 러너 full 두 번. **두 해시가 옮는다**(새 해시를 커밋 메시지에). CSV·`_rows.csv`·`_claims.csv`와 `ending:`·`race:`가 앞 커밋과 글자 그대로. `Sim.SaveTimeVectors`·`Sim.HashCoversTimeVectors`, 옛 세이브. apply 전후. `-UpdateBaseline` |
| 3 | `[B-7x2]` 장치 | 492c4f0 + 5969bd5 + 85f297c를 하나로 합친다. §4.2 해소, 코1~코7, 데3(K4·Inward 키), 시1~시7·시12, `Sim.DiplomacyRules` 등 보드 고침. 2와 하나로 합쳐 해시를 한 번만 옮겨도 된다(헤더 §0) | 러너 full 두 번. **두 해시가 한 번 더 옮는다**(누적 셈). CSV 결정적 칸 그대로. 선택: 해셔 탐침으로 두 해시 비트 그대로(DET-2). 새 단언마다 변이 한 번. apply 전후로 멈춘 패치를 다시 뜬다. `docs/30`. ultracode 구현 리뷰 |
| 4 | `[B-7x4m]` 측정 | 코8 | **비트 그대로**(다섯 줄·해시 판 둘, `-TellAll=0` 포함). 변이(건너뜀을 뺌 — 벤치 카드, 끈 스위치가 씨앗·노브를 남김). 그다음 '값보다 먼저 재는 것'을 커밋하지 않는 데이터 사본으로 잰다 |
| 5 | `[B-7x3]` 표현 | 결정 13의 답, 스캔 줄의 얼굴, 외교 창 툴팁(표류 목표의 새 항). 패널의 꼴 낱말 | 비트 그대로. 시9. 화면·문구 리뷰 |
| 6 | `[B-7x4]` 글 | 노트에 둔 카드·사건 행의 감사한 초안(데2). ST_CA 키, 데3의 낱말 규칙(갈림·`Shape.`), shape_merit_words | 세계 해시 그대로(행은 아직 켜지 않는다). ultracode 글 감사 |
| 7 | `[B-7x5s]` 씨앗·노브 | aa09543의 DT_Rules 몫(데4) | **세계가 옮는다(AI만).** 깨질 수 있는 확인 ⓐ~ⓒ(=1). ⑥ B6b 다시 잼. 끈 짝 =1이 6의 다섯 줄을 글자 그대로. 시11. 러너 full `-Data` 두 번 |
| 8 | `[B-7x5c]` 카드·사건·Society | aa09543의 DT_Encounters·DT_Events·DT_Civs 몫, 데1·데2·데5. 6의 감사한 글을 글자 그대로 | **세계가 옮는다.** 깨질 수 있는 확인(=1과 =2). 판단 ⓪~⑤·③′·⑦·⑧, ⑥ 다시. 시10·시11. ultracode 밸런스 리뷰 |
| 9 | `[B-7x5r]` | 판단이 노브를 부를 때만 | 고침의 규칙(새 시드 31~60). STATUS의 기준값 사실 |
| 10~ | `[B-7x6]`~`[B-7x9]` | 슬라이스 나·다, 문서 | [개발자] 14·15·16·22의 답 뒤 |

### 4.2 예상 충돌과 해소 (main을 앞에 둔다)

리뷰 때(10205b6) `git merge-tree cef8cc7 10205b6 0bffb8e`로 확인한 것과, B6b 브리프에서 읽은 앞으로의 것을 함께 적는다.

- **`[B-7x2]` — 지금 충돌**
  1. `Tools/data/ST_CA.csv` 끝: main의 키(Rung.*·[B-6b4]·[B-6b5]의 키) 뒤에 `"Settle.Block.Inward"`를 둔다. [B-6b7]의 `Edict.TipWide`는 제자리 고침이라 끝과 떨어져 있다.
  2. `Tools/data/schema.json`의 `words`: **main의 줄은 모두 그대로 둔다**([B-6b5]의 13낱말 `holy_passive`와 아홉 목록, [B-6b6]의 `ark_words`·'노아' 등). 브랜치의 옛 `holy_passive`·`record_object_words` 줄은 버리고, `time_vectors` 한 줄만 `words`의 끝에 더한다. 두 벌을 두면 JSON이 깨지거나 check_data가 '두 번 적힌 키'로 멈춘다(main `Tools/cosmic_data_schema.py:300-316`). `words` 끝에 두면 [B-6b5]의 두 스키마 변이 패치가 산다.
  3. `Tools/test_data_schema.py`의 `walked` 핀: 더하지 말고 스키마에서 다시 센다. 10205b6 기준으로는 [B-7x2] 311, [B-7x2r] 312, [B-7x5] 383이고, B6b가 끝난 main의 수에서 다시 센다([B-6b6] +11, [B-6b8]도 더한다).
- **`[B-7x2]` — 저절로 합쳐지되 다시 읽을 것**: `UI/CosmicSystemPanelWidget.cpp`(main의 `ReasonOf(…, Tier)`·`SettleReason = Reason;` 곁의 static_assert 12와 Inward 힌트 — B6b가 정착 사유를 더했으면 13), `Tests/CosmicPanelTests.cpp`, `DT_Events.json`.
- **`[B-7x2]` — B6b가 앞으로 고칠 곳 곁**: `Tests/CosmicDataTests.cpp` — [B-6b7] §2.1과 [B-6b8b]가 TheodicyProblems(main 지금 :2477-2568)를 고친다. 브랜치는 :1790 뿌리 거울만 고치므로 겹치지 않는다. 시3의 기생 항은 [B-6b8b] 뒤의 글에 얹는다. `Data/CosmicDefinitionRows.h` — [B-6b8b]는 :992 주석만, B7x 노브는 `GalaxyLawsToHold`(:1407) 뒤다. `DT_Rules.json` — [B-6b7]은 :17(r), [B-6b8b]는 :27(지킴 몫)을 제자리에서 고치고, B7x는 끝에 더한다. 충돌은 예상하지 않지만 0단계에서 확인한다.
- **`[B-7x1]`**: `Config/DefaultGameplayTags.ini` — [B-6b6]은 :20만, [B-6b7]의 ini 고침은 0단계에서 확인한다. B7x 태그 열일곱은 다른 자리에 있다. 스키마 뿌리 세 곳은 줄이 옮았을 뿐(I10)이고 충돌하지 않는다. FCosmicRulesDef·EConditionType·ECosmicSettleBlock·CosmicSeed의 끝은 [B-6b5] 뒤의 B6b가 더하지 않는다(I1).
- **`[B-7x5s]`·`[B-7x5c]`**: `DT_Events.json` 끝([B-6b8]의 `event_era_endured` 뒤에 `event_brick_*` 아홉), `DT_Encounters.json` 끝(`enc_witness` 뒤에 갈림 열둘), `schema.json`의 DT_Encounters rules 끝([B-6b8]의 Rule W 뒤에 B7x의 다섯 규칙, branch `:1180-1281`), `rule_setups`와 `fixture_tables`([B-6b6]·[B-6b8]이 더한 것 뒤에), `walked`. `DT_Rules.json` 끝의 노브 값, `DT_Civs.json` `arch_seeker`.
- **문서**: B7x 문서는 `[B-6b12]`가 쓴 STATUS·DECISIONS_LOG·OPEN_QUESTIONS 위에 새로 쓴다. 문서의 충돌은 없지만 번호와 수는 그 끝의 값을 쓴다(I10).
- **반박**: `UI/CosmicSystemPanelWidget.cpp`가 충돌로 멈춘다는 주장은 틀렸다(build-3 — git merge-file 0 충돌).

### 4.3 B6b와의 의미 상호작용과 다시 볼 것

1. **남긴 법 대 AI 법칙 걸음**(I2·M11; B6b 결정 10·11, [B-6b2]의 `AIRevokesOf` 은하 법칙 예외): 시4 (b), `-Crossroads` 칸의 `LawsHeldTicks`.
2. **남긴 법 대 주장 고리**(F1; B6b 결정 9, W): 시4 (a). W 1.2445가 그대로인지(`Data.Integrity`).
3. **미시 곱 대 [B-6b6]의 절멸·초신성 전조**(M9·I5; B6b 결정 2~5, 경주 ② (나)): 시5. 묶음에서 다스리는 백성의 절멸 수.
4. **기생 덤 대 [B-6b8b]의 지킴 몫 0.5와 신정론 불변식**(DET-4·M10·build-2·F2): 시3. B6b가 끝난 데이터로 `w_recount.py`를 다시 돌린다(역병 350 + 덤 30 → 0.98^38 ≈ 0.46). 판단 ⑦의 기생 몫.
5. **씨앗 블록 대 B6b 경주**(M1·I4·F3; #161 띠, ①·②·③, 시계 ⓔ, C): 판단 ⑥. 첫 AI 셋째·넷째 칸 틱. Society 연구가 칙령 자리를 앞당겨 AI 법칙 걸음이 일찍 서는지도 본다(I4). 관계 항이 AI 조약 걸음과 `feat_passage_by_ark`(CivHasTreaty)에 닿는지도 본다.
6. **`enc_cross_drift_fleet` 대 [B-6b8]의 `enc_witness`·도착 카드**(I3; b6b8 brief :198-200의 전제, `T_B` 다리 :398): 판단 ③′, 시11.
7. **게임 데이터를 쓰는 테스트**(I7·DET-3·build-1): `Sim.SightWalkMoved`(CRC), `Sim.WitnessCard`, EndingTables, `Sim.SuccessionPossible`, `Sim.WandererLoop`, `Sim.PhaseTwoExit`, `Feats.Reachable`. `WithoutB6bData`와 쌓는 차례도 본다.
8. **변이 패치**(I6): 일곱이 멈춘다(§3.4 시12). [B-6b6]~[B-6b8]의 새 패치도 apply 전후로 다시 견준다.
9. **방랑자 정책**([B-6b8]의 `Pick = Counts.Answers % Row->Choices.Num()`): 건너뜀을 그 위에 짓는다. `NoCheckByRow`는 실제로 답한 선택으로 센다.
10. **낱말 목록**([B-6b5]의 rung_grasp_words·title_words·binding_words, [B-6b6]의 ark_words): 갈림 열둘·사건 아홉의 글, `Settle.Block.Inward`, `Shape.`이 그 목록에 걸리는지 본다(T9·I8). 리뷰 스캔에서 '견딘'(desolation_words)과 '왕좌'·'다스리'(dawn_words)가 걸렸다.
11. **시계 C**([B-6b10]): §5의 틱 C, 판단 ⑥의 ⓔ. 6,000을 쓰는 모든 문장.
12. **정착 사유의 열거자 수**: B6b가 정착 사유를 더했으면 `Inward`의 자리와 static_assert 셋(branch `Sim/CosmicMegaprojects.cpp:569`, `UI/CosmicSystemPanelWidget.cpp:1065`, `Tests/CosmicBalanceCommandlet.cpp:1098`)이 13이 된다. I1은 [B-6b5]~[B-6b8b]가 더하지 않는다고 본다. 0단계에서 확인한다.
13. **열린 물음과 테스트 수**([B-6b12]): #번호와 STATUS 수(I10).
14. **절멸과 탄생**(T4; B6b의 `LetPeopleGo`, 남은 자의 문): 슬라이스 다의 결정 16 (3a)/(3b).

### 4.4 커밋마다 증명할 것 (요약)

- **해시가 옮는 커밋**(`[B-7x1]`, `[B-7x2]`, 합치면 하나): 두 해시만 옮는다. 다섯 판의 CSV·`_rows.csv`·`_claims.csv`(TickMicros·SaveBytes·SaveRawBytes 빼고)와 `ending:`·`race:`가 앞 커밋과 글자 그대로다. 선택으로 해셔 탐침의 비트 그대로를 본다. 새 해시를 커밋 메시지에 적고 `-UpdateBaseline`을 돌린다.
- **비트 그대로의 커밋**(`[B-7x4m]`, `[B-7x3]`, `[B-7x4]`): 다섯 줄과 해시 판 둘이 비트 그대로다(`-TellAll=0` 포함). `[B-7x4]`는 세계 해시가 그대로다.
- **세계가 옮는 커밋**(`[B-7x5s]`, `[B-7x5c]`, `[B-7x5r]`): 끈 짝(=1, `[B-7x5c]`는 =2도)이 앞 커밋의 다섯 줄을 글자 그대로 낸다. 스위치 대조, `end check`·`race check` 0, 미리 적은 판단, B6b 판단 ⑥.
- **모든 커밋**: 러너 full 두 번(데이터 커밋은 `-Data`), 헤더를 고치면 에디터를 닫고 전체 빌드, 새 단언마다 변이 한 번, apply 전후 견줌.

---

## 5. 반박된 것

### 5.1 통째로 반박된 발견
- **build-3**: '`UI/CosmicSystemPanelWidget.cpp`가 충돌로 멈추는데 인계 목록에서 빠졌다' — 틀렸다. base cef8cc7 / ours 10205b6 / theirs 0bffb8e의 3방향 병합이 0 충돌이다. main은 옛 :1061을, 브랜치는 :1063-1066을 고치고, 그 사이의 :1062가 두 쪽 모두 그대로다. 합친 글도 맞다(`ECosmicSettleBlock`은 Count 앞 11개라 12가 옳다).

### 5.2 남긴 발견 안에서 반박된 주장 (한 줄씩)
- F1: '주장이 100틱 늦춰져 W가 는다' — 철야 100은 이미 W 안이다(main B6b 헤더 :221). 깨지는 것은 '영원한 늦춤'과 '해결로 바뀐 만료'다.
- F2: 기생 덤을 `GuardDelayCapOf`로 묶고 스키마 max를 300으로 — #132는 함대 몫의 띠이지 합의 상한이 아니다. 칙령 덤 열에도 상한이 없다. 간접 경로(철야의 반란 +200, 격리의 역병 ×0.1)는 main에도 있어 K9 위반이 아니다.
- F6: `CivTimeVectorAtLeast`도 업적에서 금지 — K4의 글(블록·변이·카드) 밖이다.
- F7: 스키마 `then`을 'CivLacksTrait 또는 FlagNotSet'으로 넓히기 — 검사기에 OR 꼴이 없고 필요하지도 않다.
- F8 (6): '탄생 요청은 열째 단계에서 생긴다' — 헤더에 없는 전제다. 물어야 할 것은 자리다.
- F11: DevComment 낱말이 규칙 11을 깬다 — DevComment는 화면 글이 아니다. 일관성의 문제일 뿐이다(T6의 고침은 남는다).
- DET-1: '새 종류의 예외라 결정 22로 올린다' — docs/20 ③에 이미 생성 때 세계를 정하는 파생 정수(건너온 백성의 이름·아키타입)가 있다. 문장만 고치면 된다.
- DET-2: '[B-6b1]은 해셔 탐침으로 증명했다' — 그 패치는 `Sim.HashCoversNewCounters`가 잡는 변이였다. [B-6b1]은 CSV와 `_rows`로 증명했다(무게는 low).
- DET-5: '연대기의 기록이 흔들린다'(과장) — 첫 열림에서는 끝의 꼴과 맞다. 결함은 정하지 않은 백성과 다시 열 때의 바뀜이다.
- DET-7: '편집기 판과 패키지 게임이 지금 갈린다' — 데이터 검사가 대소문자를 막는다(`Tools/cosmic_data_schema.py:679`·`:1012`). 잠재 위험일 뿐이다.
- DET-9: '지운 CivID를 탄생이 다시 쓴다' — `NextCivSerial`은 단조라 재사용이 없다. 지움은 Relations만이 아니라 Ties·`PendingContactCivID`도 걷는다.
- T2: HermitMonolith의 'hermit'은 systems.md:385 ⑨에 없고, 프로젝트가 이미 쓴다(main ini:206). 제안한 `ShutHouse`는 꼴 키와 겹친다.
- T3 ③: 'C10 → 다른 질서 → 법을 남긴 살림' 사슬은 해석이다. ②는 이미 손 확인 5에 있다. 제안한 칙령 창 자리는 판의 끝 뒤에 열리지 않는다.
- T4 (3): FossilAutomata를 절멸에서 반드시 막는다 — (1)·(2)로 신학상 족하다. 선택지로 둔다.
- T5: 'CLAUDE.md의 표지 없는 문장이 현행 규칙이 된다' — 설계서 :10의 표지가 이 파일에서 그것을 명시적으로 덮는다. CLAUDE.md에 줄을 더하는 고침은 크기 예산을 넘는다.
- T7: '오래 버틴 살림' — '버틴'은 이미 desolation_words에 있다.
- T8 (c): C11 ①의 안정도를 지금 낮춘다 — 동화의 유혹보다 싸다는 근거가 없다. 측정으로 정한다.
- M3: 변이 문턱 셋 — 노브가 하나다.
- M5: `petition_pressure_measured`를 'standing cards lost'로 센다 — Named 카드라 그 셈에 들지 않는다.
- M7: S04·S12를 `-Crossroads` 칸에서 — 표준 칸에서 선다.
- M11: 기대값 '법 0' — 자리 셋에 칙령 하나면 법칙 둘은 선다. `LawsHeldTicks`만 0이다.
- M12: `-WithoutB7xData=2`는 헤더에 정의가 없다(M8이 정의하면 쓴다). `-Crossroads`는 입력의 꼴이라 `-SaveAt` 거절 목록에 넣지 않는다.
- M13: `ruler:` 줄의 cards는 판 전체 수다. 틱 600의 수는 decisions by 600·told by 600이다.
- M14: 헤더 글에 '③은 M5'처럼 리뷰 번호를 쓰지 않는다.
- build-1: '인계가 WithoutB7xData를 나중 측정에만 둔다' — promptforother 4d(:60-62)는 데이터 켜기 전에 넣는다고 했다. aa09543도 통과한다고 내세운 적이 없다(커밋 메시지가 SourcesInSync 실패를 적는다). 실제 결함은 시험 보드와의 매김이 없다는 것이다.
- build-2: `ThroneLockTicks`의 한계도 과소 — `DelayHazard`는 주장에만 있고 기생은 주장을 빼므로 아니다.
- build-4: 죽은 노드·성역 갈래의 시험 — 결과를 바꾸지 않아 시험할 수 없다.
- I1: 'JSON은 조용히 뒤 키를 쓰고 check_data는 0이다' — 이 프로젝트의 `parse_json`은 object_pairs_hook으로 '두 번 적힌 키'를 알리고 멈춘다.
- I9: '[B-7x1] 뒤 B6b의 끈 짝 약속이 깨진다' — 그 약속은 '장치가 그대로인 동안'의 조건부이고, B6b의 ⓐ는 B6b 커밋에서 이미 잰다.
- I12: 'B6b의 절멸이 지워진 백성을 흔하게 한다' — 셋째 칸 이상과 답 없는 만료가 든다. 결함은 빈도가 아니라 적히지 않은 규칙이다.
- I5(근거 하나): '각성 몫을 줄이는 것이 새 지렛대다' — B6b 은하 법칙 고리 ①이 이미 각성을 포함한 Draw 몫을 줄인다(main B6b 헤더 :146). 측정 항목일 뿐이다.

---

## 추가 이야기와 보완

- **좁힘 하나로 R이 하나 준다.** 코1의 '주장 고리 칙령은 거둔다'는 B6b 결정 9가 은하 법칙에 그은 선을 백성 범위 칙령에 그대로 옮긴다. 결정 18의 답과 함께 내면 K9를 굽히는 새 R이 들지 않는다. R12(B2 §2.6)만 남는다.
- **해시는 한 번만 옮기는 편이 싸다.** 결정 13이 ㉠(`ReignShapes`)이거나 결정 6 (ㄷ)이 탄생 요청 필드를 부르면, 그 필드를 `[B-7x1]`에 함께 넣는다. 그리고 `[B-7x1]`·`[B-7x2]`를 한 커밋으로 합쳐 두 해시를 한 번만 옮긴다(헤더 §0이 이미 허락한다).
- **측정 줄은 값보다 먼저다.** `[B-7x4m]`의 `bricks:` 줄과 끈 짝 두 단을 데이터 커밋보다 앞에 두면, '값보다 먼저 재는 것'과 판단 ⑥이 같은 빌드에서 잴 수 있다. B6b의 `scratchpad/b6b8/bundleB.py`를 B7x 묶음 곁에 베껴 끈 스위치, 기대 줄, 짝 검사 꼴을 인자로 받게 하면 하네스를 새로 쓰지 않아도 된다. 저장소에 들이는 것은 선택이다.
- **둘째 갈림은 (나)로 닫는 편이 싸다.** 되풀이 이득을 빼면 태그 여섯이 늘지 않고, 판 전체 Flag의 범위 문제도 없다. 결정 11에 묻지 않아도 된다.
- **이름은 지금만 고칠 수 있다.** 결정 3의 답은 `[B-7x1]`이 main에 들기 전에만 쓸모가 있다. 답을 받으면 ini·접근자·스키마 나열·노브·테스트를 한 번에 고친다.
- **이 리뷰가 고치지 않은 것.** 브랜치의 어떤 파일도 고치지 않았다. 리뷰 동안 main의 작업 트리도 읽지 않았다. 고침은 모두 `[B-7x0]`과 리베이스 뒤의 커밋 몫이다.
