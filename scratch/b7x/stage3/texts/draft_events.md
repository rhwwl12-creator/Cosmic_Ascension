# [B-7x4] 글 — 사건 행 아홉·꼴 낱말 열둘·정착 거절 한 줄·스캔 얼굴·블록 태그 DevComment 열일곱 (초안, 2026-11-10)

출발: 데이터 초안 `aa09543`(`Tools/data/DT_Events.json`의 `event_brick_*` 아홉, `ST_CA.csv`의 `Settle.Block.Inward`), 헤더 `79f2583` §3.1·§3.2·§3.5·§4(결정 13 ㉢ (d))·§10, 노트 §5.4 `[B-7x2]` 코6·`[B-7x3]`·`[B-7x4]`, 설계서 §3.3 S01~S15·§4.4, 통합 트리 `int/Config/DefaultGameplayTags.ini:170-186`(읽기만 — 사본 `stage3/texts/src/tags_int_snapshot.ini`).

검사 기준(모든 줄): main `schema.json`의 `DT_Events.LogTemplates` 금지 목록(verdict·holy_words·soul_verbs·rescue_words·rung_grasp_words·title_words·ark_words) + 헤더 §3.5가 거는 silence_words·crossroads_words; `DT_LogTemplates.Templates`의 같은 목록; `Shape.`은 `Chronicle.` 목록 + remnant_words·desolation_words·shape_merit_words `["견디","남은"]`; `Settle.Block.Inward`는 verdict·holy_words·rescue_words·rung_grasp_words·title_words, 숫자 없음. 아래 글은 모두 이 목록을 지난다(스크립트로 확인 — 끝의 '검사 기록'). 주제 `Civilization` 행의 슬롯은 `{Civ}`·`{Hazard}`뿐이고(main 규칙 'the subject system of a line…') 슬롯 행은 변형 셋 이상이다.

잎 이름은 새 이름이다(결정 3 — `rename_leaves.py`가 고칠 자리; 화면에는 어느 이름도 나오지 않는다).

---

## 사건 행 (`DT_Events`, `[B-7x5c]`가 옮긴다)

### event_brick_shut_star — S01 (덮은 별에 건 살림)

- 조건: `CivHasTrait Brick.Space.ClosedPort` ∧ `CivHasTrait Brick.Form.ShutSphere` (Lottery, `BaseWeight` 1, `CooldownTicks` 600, Severity 0, `Event.Civ.Economy`/`Log.Civ`, 주제 Civilization)
- 효과: `AddResource Insight +20`

```json
{
  "LogTemplates": [
    "{Civ}의 별을 감싼 구 안에서 셈이 끊이지 않고 돈다고 한다. 그들의 배는 새 터를 찾아 나서지 않는다.",
    "{Civ}의 닫힌 구에서 새 셈법이 흘러나왔다. 그 셈을 낸 이들은 구 바깥을 내다보지 않았다.",
    "{Civ}의 학자들이 별의 열로 돌린 셈을 장부에 옮겼다. 그 장부는 바깥으로 나가지 않았다."
  ]
}
```

- why: 바꾼 칸 없음 — 셋 모두 규칙을 지나 글자 그대로(조건의 잎 `HermitMonolith`→`ClosedPort`, `DysonBrain`→`ShutSphere`는 이름 고침뿐).

### event_brick_heat_at_door — S04 (문간에 든 재앙 곁의 이웃)

- 조건: `BorderingCivHasTrait Brick.Diet.CrisisParasite` ∧ `CivTimeVectorAtLeast Endured 1200` (Lottery, 1, 600, Severity 0, `Event.Civ.Society`/`Log.Civ`)
- 효과: `ChangeStability −0.03`

```json
{
  "LogTemplates": [
    "{Civ}의 국경 너머에서 재앙의 열을 핥는 이들이 서성인다고 한다. 오래 눌려 온 집들은 그들을 내쫓지 않지만, 거리는 술렁인다.",
    "{Civ}의 국경 장터에 재앙 곁에서 사는 이웃의 배들이 드나들었다. 오래 눌려 온 이들은 그 배를 보며 말을 낮추었다.",
    "{Civ}의 변경에서 이웃이 재앙의 자리를 찾아다닌다는 말이 돌았다. 그 말이 도는 동안 거리의 목소리가 날카로워졌다."
  ]
}
```

- why [0]: '오래 견딘 집들은 그들을 내쫓지 않는다.' → '오래 눌려 온 집들은 그들을 내쫓지 않지만, 거리는 술렁인다.' — '견딘'은 desolation_words(main `schema.json` — 견딤을 공로로 읽는 낱말, 리뷰 T7)이고 시간 셈 `Endured`의 설계 이름('견딘 시간' — 헤더 §3.5: 시간 셈의 설계 이름은 화면에 내지 않는다); 결정이 카드 C10의 '견딘 날들 내내' → '눌린 날들 내내'로 고친 것과 같은 낱말로 맞췄다. 둘째 절은 이 행의 효과(안정도 내림)를 사실로 적었다 — 처음 글은 내쫓지 않는다는 말만 있어 백성이 그 뒤 선 자리가 없었다(§3.5 '한 일의 사실과 그 뒤에 서 있게 된 자리').
- why [1]: '오래 견딘 이들은' → '오래 눌려 온 이들은' — 같은 까닭.
- why [2]: 그대로.

### event_brick_drift_plenty — S05 (지나온 자리를 비우는 떠도는 일손 — 앞쪽)

- 조건: `CivHasTrait Brick.Instinct.SwarmTech` ∧ `CivHasTrait Brick.Space.Nomad` (Lottery, 1, 600, Severity 1, `Event.Civ.Economy`/`Log.Civ`)
- 효과: `AddResource Matter +30`, `ChangePopulation +0.02`

```json
{
  "LogTemplates": [
    "{Civ}의 떠도는 일손이 닿는 별마다 광맥을 열었다. 배 위의 창고가 차고 식구가 늘었다.",
    "{Civ}의 작은 일꾼들이 새로 닿은 자리에서 쓸 것을 캐 올렸다. 배마다 식구가 늘었다.",
    "{Civ}의 배들이 지나는 항로마다 캐 올린 것이 쌓였다. 배 위의 살림이 넉넉해지고 집이 불었다."
  ]
}
```

- why: 바꾼 칸 없음 — 사람의 수는 '식구가 늘었다'의 규모뿐(숫자 없음).

### event_brick_emptied_trail — S05 (뒤쪽)

- 조건: `CivHasTrait Brick.Instinct.SwarmTech` ∧ `CivHasTrait Brick.Space.Nomad` ∧ `CivTimeVectorAtLeast Drifted 1800` (Lottery, 1, 600, Severity 1, `Event.Civ.Economy`/`Log.Civ`)
- 효과: `ChangeStability −0.03`, `ChangePopulation −0.03`, `AddResource Matter −40`

```json
{
  "LogTemplates": [
    "{Civ}의 배들이 지나온 자리에서는 더는 아무것도 올라오지 않았다. 갱도 어귀의 집들이 비어 갔다.",
    "{Civ}의 일꾼들이 캐던 자리마다 바닥이 드러났다. 창고가 줄고 배 위의 식구도 줄었다.",
    "{Civ}의 항로 뒤편에 파헤친 별들이 비어 남았다. 다음 터를 찾는 동안 배 위의 살림이 줄고 다툼이 잦아졌다."
  ]
}
```

- why: 바꾼 칸 없음 — 벌로 서술하지 않는다(설계서 S06의 지킴과 같은 결)는 셋이 지킨다.

### event_brick_ember_wake — S06 (불이 잦아들기를 기다리는 이웃)

- 조건: `CivHasTrait Brick.Diet.StarEater` ∧ `SystemHasHazard Hazard.PreSupernova` (Standing, `BaseWeight` 0, `CooldownTicks` 300, Severity 0, `Event.Civ.Society`/`Log.Civ` — 주제가 백성이라 수도 별의 전조만 본다)
- 효과: `ChangeStability −0.02`

```json
{
  "LogTemplates": [
    "{Civ}의 해에 검은 벌집 무늬가 번졌다. 국경 너머에는 그 불이 잦아들기를 기다리는 이들이 있었다.",
    "{Civ}의 화로가 끌어 쓰던 별이 고르지 않게 떨렸다. 화로 곁의 사람들이 불을 줄이자는 말을 주고받았다.",
    "{Civ}의 도읍 하늘에서 별빛이 얼룩졌다. 그 별의 불을 끌어 쓰던 집들이 하늘을 자주 살폈다."
  ]
}
```

- why: 바꾼 칸 없음 — 벌로 서술하지 않고(S06), 별의 위기를 백성의 탓으로 잇지 않는다.

### event_brick_own_hour — S07 (제 시각을 지키는 이웃)

- 조건: `BorderingCivHasTrait Brick.Instinct.HiveMerge` ∧ `CivTimeVectorAtLeast Isolated 1500` (Lottery, 1, 600, Severity 0, `Event.Civ.Society`/`Log.Civ`)
- 효과: `ChangeStability −0.03`, `AddResource Insight +20`

```json
{
  "LogTemplates": [
    "{Civ}의 국경 너머에서는 모두가 같은 시각에 장을 연다고들 했다. {Civ}|hpp(은,는) 제 시각에 문을 열고 닫았다.",
    "{Civ}의 변경 장터에 이웃의 한결같은 목소리가 들려왔다. {Civ}의 학자들은 술렁이는 거리에서 제 셈을 따로 적었다.",
    "{Civ}의 국경 너머 이웃이 같은 답으로만 말한다는 소문이 돌았다. {Civ}|hpp(은,는) 술렁이면서도 제 셈법을 지켰다."
  ]
}
```

- why: 바꾼 칸 없음 — 뒤엉킴('같은 시각'·'같은 답')을 좋은 끝으로 적지 않고, 홀로 선 쪽도 옳다고 적지 않는다(값은 술렁임으로 치른다). '하나가 되'·일체화 없음.

### event_brick_fold_gleaning — S08 (티끌 곁의 나루)

- 조건: `CivHasTrait Brick.Space.FoldedSmall` ∧ `BorderingCivHasTrait Brick.Space.Nomad` (Lottery, 1, 600, Severity 0, `Event.Civ.Economy`/`Log.Civ`)
- 효과: `AddResource Insight +25`

```json
{
  "LogTemplates": [
    "{Civ}의 별 곁에는 떠도는 살림이 자리를 펴고 있다. {Civ}|hpp(은,는) 그들이 떨군 것을 주워 맞춘다.",
    "{Civ}의 국경 너머 항로에는 떠도는 배들이 오간다. {Civ}의 학자들은 그 배들이 남긴 것에서 새 셈을 찾는다.",
    "{Civ}|hpp(은,는) 이웃의 떠도는 배들이 버린 부스러기를 모은다. 작은 눈금 속에서 그것들이 새 앎으로 맞추어진다."
  ]
}
```

- why [0]: 그대로.
- why [1]: '…오갔다. …새 셈을 찾았다.' → '…오간다. …새 셈을 찾는다.' — 설계서 S08 '변형은 상태 문장'(맞닿음이 이어지는 동안 되풀이되는 행) — 초안은 [0]만 상태 문장이었다. 낱말은 그대로.
- why [2]: '…모았다. …맞추어졌다.' → '…모은다. …맞추어진다.' — 같은 까닭.

### event_brick_inward_appetite — S11 (안으로 돌아선 식욕)

- 조건: `CivHasTrait Brick.Mutation.InwardHunger` ∧ `CivTimeVectorAtLeast Isolated 1500` (Lottery, 1, 600, Severity 0, `Event.Civ.Economy`/`Log.Civ`)
- 효과: `AddResource Energy +30`

```json
{
  "LogTemplates": [
    "{Civ}|hpp(은,는) 제 별의 불을 안으로 끌어 먹는다고 한다. 그 별을 바라보는 이웃들은 말을 아꼈다.",
    "{Civ}의 화로가 제 별 깊은 곳의 불을 끌어올렸다. 그들의 창고에 에너지가 찼다.",
    "{Civ}의 화로가 바깥의 것 없이 제 별의 불만으로 타올랐다. 화로지기들은 그 별을 오래 들여다보았다."
  ]
}
```

- why: 바꾼 칸 없음(조건의 잎 `AbyssalMaw`→`InwardHunger`는 이름 고침뿐). 자원 이름 '에너지'는 main의 결(`event_civ_grid_failure`)이고 숫자는 없다.

### event_brick_works_run — S12 (돌보는 손이 줄어도 도는 일손)

- 조건: `CivHasTrait Brick.Instinct.SwarmTech` ∧ `CivHasTrait Brick.Mutation.DustBand` ∧ `CivTimeVectorAtLeast Built 1500` (Lottery, 1, 600, Severity 0, `Event.Civ.Economy`/`Log.Civ`)
- 효과: `AddResource Energy +25`, `AddResource Matter +25`, `ChangePopulation −0.02`

```json
{
  "LogTemplates": [
    "{Civ}의 기관들은 돌보는 손이 줄어든 뒤에도 돈다고 한다. 띠를 이룬 일꾼들이 서로를 고쳐 가며 일을 잇는다.",
    "{Civ}의 별 둘레의 띠가 사람 손 없이 에너지와 물질을 실어 왔다. 공방을 지키던 집들 가운데 빈 집이 늘었다.",
    "{Civ}의 일꾼 띠가 스스로를 고치며 창고를 채웠다. 그 곁에서 손을 놀리던 사람은 줄어 갔다."
  ]
}
```

- why: 바꾼 칸 없음 — 결정 16 (3a)를 지킨다: 기계가 사람을 대신해 '이어 산다'·'살아남았다'고 말하지 않고, 손이 준 사실과 기관이 도는 사실만 나란히 적는다.

---

## 스캔 얼굴 (`DT_LogTemplates`, `[B-7x3]`)

### log_brick_scan_spread — S15 (엷게 퍼져 있다는 소문)

- 행: `Category` `Log.Discovery`, `Context` 비움, `MinTierToReveal` 0 — `log_scan_wild`와 같은 자리. 코드(`UPlayerRoleSubsystem::ScanCurrentSystem`)가 무주 성계를 스캔해 새로 안 것이 있을 때 `log_scan_wild` 대신 이 행을 고른다 — 스캔한 성계의 레인 이웃에 `Brick.Form.LivingNebula`와 `Brick.Diet.DarkMatter`를 함께 지닌 백성의 정착된 성계가 있으면. 슬롯은 `{System}`(스캔한 무주 성계)뿐 — 그 백성의 이름은 앎의 문(`IsKnown`)을 지나지 않았으므로 부르지 않는다.
- 효과: 없음(표현 계층만 — 세계 해시가 켠 판과 끈 판에서 같다).

```json
{
  "Templates": [
    "{System} 너머 이웃한 별 곁의 암흑 구간에 무엇인가 엷게 퍼져 있다고 한다. 가까이서 본 이들은 그것이 그 별 곁에만 있었다고 했다.",
    "당신은 {System}에서 이웃한 별 쪽을 오래 살폈다. 그 별 곁에만 무엇인가 엷게 번져 있다는 말이 떠돌았고, {System}의 하늘에는 그런 것이 없었다.",
    "{System}에서 바라본 이웃한 별 곁이 엷게 흐려 있었다고들 한다. 그 흐림은 그 별 곁을 넘어 번지지 않았다고 했다."
  ]
}
```

- why [0](설계서 S15 초안 '{System} 둘레의 암흑 구간에 무엇인가 엷게 퍼져 있다고 한다. 가까이서 본 이들은 그것이 제 별 곁에만 있었다고 했다.'에서): '{System} 둘레의' → '{System} 너머 이웃한 별 곁의' — `{System}`은 스캔한 **무주** 성계다; 초안은 퍼진 것을 그 무주 성계의 둘레에 두어, 결정 18 리뷰 고침 ①('표현은 무주 성계 위에 그 백성을 그리지 않는다; 그 줄은 제 별 곁에만')을 줄 안에서 어겼다. '제 별 곁에만' → '그 별 곁에만' — '제'가 '본 이들'의 별로도 읽혀 자리가 흐렸다.
- why [1]·[2]: 새 변형 — `DT_LogTemplates.Templates`는 변형 셋 이상이다(`single_variant_rows`는 `log_first_life`·`log_first_contact`뿐). S15의 '소문 한 줄'은 스캔 한 번에 한 줄이라는 뜻으로 읽었다 — 세 변형 모두 한 줄의 소문이고, 셋 다 퍼진 것을 이웃한 별 곁에만 두며 '어디에나'의 그림이 없다. [1]은 `log_scan_wild`의 화자('당신은 {System}…')를 따랐다.

---

## 스트링 키 (`ST_CA.csv` — 새 키는 끝에만, 꼴은 `"Key","SourceString"`, LF)

```csv
"Settle.Block.Inward","안으로 접힌 백성은 더 넓히지 않는다."
"Shape.BuiltOrLinked","짓고 이은 살림"
"Shape.ConsumedOrDrifted","넓히고 삼킨 살림"
"Shape.Isolated","홀로 선 살림"
"Shape.Endured","눌림 속에 이어 간 살림"
"Shape.WovenSwarm","촘촘히 엮은 일손의 살림"
"Shape.StarHerd","머물지 않는 살림"
"Shape.SpreadBody","별들 사이에 번진 살림"
"Shape.FoldedSmall","티끌 속으로 접힌 살림"
"Shape.ShutHouse","한 별에 닫아 건 살림"
"Shape.KeptLaw","법을 남긴 살림"
"Shape.LastHunger","별의 불을 끌어 쓰는 살림"
"Shape.Entwined","이웃과 뒤엉킨 살림"
```

(`Settle.Block.Inward`는 `aa09543`에 이미 `ST_CA.csv:450`으로 들어 있다 — 리베이스 때 main의 끝으로 옮기고 글은 그대로. `Shape.*`는 `[B-7x3]`이 처음 드는 키라 그 뒤에 붙인다.)

### Settle.Block.Inward — 헤더 §2.5 (정착 거절의 사유)

- 쓰는 곳: 성계 패널의 정착 거절 줄(`ECosmicSettleBlock::Inward` — `Brick.Space.ClosedPort`·`Brick.Space.FoldedSmall`).
- why: 그대로 — 금지 목록 다섯과 숫자 금지를 지나고, 설계 이름(`ClosedPort`·`FoldedSmall`·`InwardHunger`)을 내지 않으며, 거절을 벌이나 판정으로 적지 않는다. 다른 `Settle.Block.*`처럼 '정착'을 말하지 않지만 줄의 자리(정착 단추의 사유)가 그것을 말한다.

### Shape.* — 설계서 §4.4·헤더 §3.2 (결정 13 ㉢ (d))

모든 꼴 낱말은 '그 백성의 지금 살림'이다 — 칭호·판정·공로·엔딩 이름이 아니다; '되었다'가 없고, 변이의 잎은 꼴을 받지 않는다.

- `Shape.BuiltOrLinked` '짓고 이은 살림' — 그대로.
- `Shape.ConsumedOrDrifted` '넓히고 삼킨 살림' — 그대로(공성과 정착의 사실).
- `Shape.Isolated` '홀로 선 살림' — 그대로.
- `Shape.Endured` '눌림 속에 이어 간 살림' — 그대로(리뷰 T7의 고친 꼴; '견디'·'남은'·'버틴' 없음).
- `Shape.WovenSwarm` '촘촘히 엮은 일손의 살림' — 그대로.
- `Shape.StarHerd` '머물지 않는 살림' — 그대로.
- `Shape.SpreadBody` '별들 사이에 번진 살림' — 그대로('어디에나'가 아니라 별들 사이).
- `Shape.FoldedSmall` '티끌 속으로 접힌 살림' — 그대로.
- `Shape.ShutHouse` '한 별에 닫아 건 살림' — 그대로(desolation의 '문이 닫'·'닫힌 문'을 지난다).
- `Shape.KeptLaw` '법을 남긴 살림' — 그대로. 결정 18의 '되는 끝은 남긴 끝으로'의 꼴이고 무엇을 얼마나 남겼는지 약속하지 않는다(결정 18의 좁힘을 넘지 않는다). '칙령을 거두지 않는 살림' 같은 사실 꼴은 쓰지 않는다 — '거두'는 soul_verbs라 `Chronicle.` 목록에 걸리고, 좁힘(주장·위기의 답 칙령은 거둔다)보다 많이 약속한다.
- `Shape.LastHunger` '끝없이 삼키는 살림' → '별의 불을 끌어 쓰는 살림' — '끝없이'는 사실이 아니라 탐욕의 판정으로 읽히고(결정 13 ㉢ (d) 'never a verdict', 설계서 §4.4 '낱말은 사실만 말한다') 끝이 없다는 주장은 블록의 몫(유한한 곱, 일곱 등급 안에서 멈추는 어두워짐 — K8·결정 8)과 어긋난다. 새 낱말은 그 블록을 주는 선택(C02 ② '제 별의 불을 화로에 끌어들인다')과 사건 줄(`event_brick_ember_wake`의 '화로가 끌어 쓰던 별')의 낱말이다. 걸린 목록을 모두 지난다. (리드가 초안을 고르면 '끝없이 삼키는 살림'도 목록은 지난다 — 열린 점 2.)
- `Shape.Entwined` '이웃과 뒤엉킨 살림' — 그대로(좋은 끝으로 적지 않는다; '하나가 되' 없음).

---

## 블록 태그 DevComment 열일곱 (`Config/DefaultGameplayTags.ini` — 통합 트리 `:170-186`을 바꿀 글; ASCII만)

```ini
+GameplayTagList=(Tag="Brick.Form.Society",DevComment="Form brick: a people of mortal bodies living as an ordinary society - the start mark of the crossroads cards (arch_seeker carries it; the first crossroads takes it away with its first or its second choice). Read by conditions only, never by the end shape (B7x header 2.4, 3.1). Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Form.LivingNebula",DevComment="Form brick: a people whose life is spread thin between the stars like a nebula - it still owns only the systems it settled, and no unowned system is drawn as its (B7x decision 18, S15). Read by the end shape (SpreadBody), the scan face log_brick_scan_spread together with Brick.Diet.DarkMatter (S15) and conditions (B7x). Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Form.ShutSphere",DevComment="Form brick: a people that shut its star inside a sphere and turns its heat to reckoning - the shell closed around its own star; not the fallen sphere (InSphere/FallenSphereTicks) nor Trait.Faith.BookShut. Read by the end shape (ShutHouse), DT_Rules.BrickStellarCrisisSpawnMultiplier on its settled systems and conditions (B7x). Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Form.KeptLaw",DevComment="Form brick: a people whose held edicts outlive an empty throne - with no one ruling it, its regent and AI keep its people-scope edicts (CosmicEdicts::AIRevokesOf), except an edict that adds to the claim's countdown or answers a crisis, and all of them while its row asks for galaxy laws (CosmicEdicts::AsksForLawsOf): those fall to the rule every AI uses (B7x decision 18, K9). It keeps edicts; it never becomes a law, and it is never the holy order (K6). Read by AIRevokesOf, the end shape (KeptLaw) and conditions (B7x). Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Diet.DarkMatter",DevComment="Diet brick: feeds on dark matter - DT_Rules.BrickDarkMatterEnergyPerTick flat Energy a tick, tied to no star class or abundance (B7x header 2.4). Also read by the end shape (SpreadBody), the scan face log_brick_scan_spread together with Brick.Form.LivingNebula (S15) and conditions. Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Diet.StarEater",DevComment="Diet brick: draws its star's fire into its furnaces - DT_Rules.BrickStarEaterEnergyMultiplier on its Energy, BrickStarEaterRelationTarget as a term of the drift target of every view of it, BrickStellarCrisisSpawnMultiplier on its settled systems, BrickStarEaterDimPeriodTicks (0 = off, B7x decision 8; never past the darkest of the seven classes, never a destroyed star) (B7x header 2.4, R10). Also read by the end shape (LastHunger), the AI mutation step (InwardHunger) and conditions. Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Diet.IdleFeed",DevComment="Diet brick: feeds on idle ticks - DT_Rules.BrickIdleInsightPerTick Insight on a tick with no hazard, project or siege on its settled systems (CosmicBricks::IsIdleOf, B7x header 2.4). Never named silence or rest on screen and kept apart from Revelation.Quiet (B7x decision 3); idle days resemble a fallen doctrine and are never told as a good end (header 3.5). Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Diet.DistanceTension",DevComment="Diet brick: feeds on its own span - DT_Rules.BrickSpanEnergyPerLane Energy per lane of the widest lane distance between two of its settled systems (lanes, never coordinates; B7x header 2.4). Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Diet.CrisisParasite",DevComment="Diet brick: lives off its neighbours' crises - DT_Rules.BrickParasiteCountdownBonusTicks on a hazard newly placed on a bordering people's system, once a hazard however many parasites border it (CosmicBricks::ParasiteBonusBoundOf, counted in the theodicy bound of Data.Integrity), and BrickParasiteInsightPerHazard for each hazard standing on its neighbours' settled systems - never the claim, the extinction or the supernova precursor (K9; B7x header 2.4). Comes from the seed only. Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Space.Nomad",DevComment="Space brick: a wandering people - DT_Rules.BrickNomadSettleProgressMultiplier on its settlements (B7x header 2.4); it keeps a capital and its systems like any people. Also read by the end shape (StarHerd), the AI mutation step (StarTow) and conditions; enc_cross_drift_fleet gives it to the owner of the wanderer's system. Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Space.ClosedPort",DevComment="Space brick: a people folded inward into one sealed house - starts no further settlement (settle reason Inward), DT_Rules.BrickInwardResearchMultiplier; no immunity - its system keeps only the protection every people has (K8; B7x header 2.4, 2.5). Inward here is not Brick.Mutation.InwardHunger. Also read by the end shape (ShutHouse) and conditions. Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Space.FoldedSmall",DevComment="Space brick: a people folded into the very small - starts no further settlement (settle reason Inward), DT_Rules.BrickInwardResearchMultiplier, BrickInwardHazardSpawnMultiplier on its systems (never 0, never the claim, the extinction or the supernova precursor - K8, K9); the map still shows it as the owner (K1) (B7x header 2.4, 2.5). Also read by the end shape (FoldedSmall) and conditions. Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Instinct.SwarmTech",DevComment="Instinct brick: a swarm that learns together - DT_Rules.BrickSwarmResearchMultiplier (B7x header 2.4). Also read by the end shape (WovenSwarm), the AI mutation step (DustBand) and conditions. Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Instinct.HiveMerge",DevComment="Instinct brick: a people that draws its neighbours in - DT_Rules.BrickMergeRelationTarget as a term of the drift target of every view of it (B7x header 2.4). The same grain as the counterfeit of Doctrine.Assimilation: never celebrated, never told as a good end or as becoming one (header 3.5). Also read by the end shape (Entwined) and conditions. Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Mutation.StarTow",DevComment="Mutation brick: the second mark of a wandering people - added by the AI mutation step to a Brick.Space.Nomad people whose Drifted vector reaches DT_Rules.BrickMutationThresholdTicks, or to the ruled people by petition_cross_star_line (B7x header 2.6). Slice ga opens the word only: towing a star waits for the lane window (slice na), and it never relights a dead star. Takes no shape name - the end shape reads its parent (B7x decision 3). Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Mutation.DustBand",DevComment="Mutation brick: the second mark of a swarm - added by the AI mutation step to a Brick.Instinct.SwarmTech people whose Built and Linked vectors reach DT_Rules.BrickMutationThresholdTicks, or to the ruled people by petition_cross_dust_band (B7x header 2.6). Read by conditions (event_brick_works_run); it gives no rung and no lane. Takes no shape name - the end shape reads its parent (B7x decision 3). Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Mutation.InwardHunger",DevComment="Mutation brick: the second mark of a star eater - added only by the AI mutation step to a Brick.Diet.StarEater people whose Consumed vector reaches DT_Rules.BrickMutationThresholdTicks (B7x header 2.6, S11). Read by conditions (event_brick_inward_appetite). A different fact from CosmicBricks::IsInward and the settle reason Inward (ClosedPort/FoldedSmall), which never read it. Takes no shape name - the end shape reads its parent (B7x decision 3). Saved name, never renamed.")
```

- `Society`: 'a people of many small bodies held together by custom' → 위 글 — 처음 글은 노트 §1.1의 뜻('필멸의 사회', 갈림 카드의 출발 표지)이 아니라 다른 몸을 지어냈다; 누가 주고 누가 걷는지를 적었다.
- `LivingNebula`: 'a people whose body is a living nebula' → 몸이 세계·성운이 '되는' 그림(원안 `WorldBecome`을 합친 자리 — K6의 '되는' 끝의 결)을 피해 '삶이 별들 사이에 엷게 퍼진 백성'으로; 소유는 정착한 성계뿐이고 무주 성계에 그려지지 않음(결정 18 고침 ①); 읽는 곳에 스캔 얼굴(S15, `[B-7x3]`)을 더했다.
- `ShutSphere`: 'a people that became one mind around its star' → 노트 §5.4 코6의 글 'a people that shut its star inside a sphere and turns its heat to reckoning' — '하나가 되'·일체화(crossroads_words·rung_grasp_words)와 K6('누구도 … 되지 않는다')에 걸리는 '되었다'를 뺐다. 구별 문장(결정 3)은 그대로.
- `KeptLaw`: 'a people that lives on as a kept law of its region - a description of a people, never of the holy order (K6)' → 위 글 — 처음 글은 백성이 '법으로 이어 산다'(K6이 막는 '법이 되는' 끝, 결정 16의 '이어 산다'의 결)였고 결정 18의 좁힘이 없었다. 노트 §5.4 코6의 글을 바탕으로, 그 글의 'no regent or AI of it revokes one … save those that touch the claim …'가 '그것들은 반드시 거둔다'로 읽히는 것을 코1의 코드대로 '여느 AI의 규칙에 맡긴다'로 고쳤다(`AIRevokesOf` — 주장·위기의 답 칙령은 건너뛰지 않고 `!HasStabilityCost || Stability < AIEdictRevokeStability`의 규칙으로 간다; 은하 범위 법칙은 누구에게나 남는다).
- `DarkMatter`: 처음 글에 '별 등급·풍부도에 매이지 않음'(헤더 §2.4)과 읽는 곳(꼴·스캔 얼굴·조건)을 더했다.
- `StarEater`: 'eats stars' → 'draws its star's fire into its furnaces' — 별을 없애지 않는다(파괴 없음, 일곱 등급 안 — 결정 8); 어두워짐이 꺼져 있음(주기 0)과 꼴·변이 걸음의 읽는 곳을 더했다.
- `IdleFeed`: 술어 이름과, 화면에 침묵·쉼을 쓰지 않음·`Revelation.Quiet`과 갈라 둠(결정 3)·좋은 끝으로 적지 않음(§3.5)을 더했다.
- `DistanceTension`: '좌표가 아니라 레인'(헤더 §2.4)만 더했다.
- `CrisisParasite`: 'neighbours crises' → 'neighbours' crises'(소유격), 노트 §5.4 코3(한 위기에 한 번 — `ParasiteBonusBoundOf`, 신정론 불변식이 센다)과 씨앗으로만 온다(설계서 S04)를 더했다.
- `Nomad`: 수도와 성계를 지닌 여느 백성임(노트 §1.3 — 수도 없는 유영은 받지 않음)과 읽는 곳·방랑자 카드를 더했다.
- `ClosedPort`: 면역 없음(K8)과 `InwardHunger`와의 구별(결정 3), 꼴의 읽는 곳을 더했다.
- `FoldedSmall`: 감쇠의 제외와 0 불가(노트 §5.4 코2, K8·K9), 주인 표시는 참(K1)을 더했다.
- `SwarmTech`: 읽는 곳만 더했다.
- `HiveMerge`: 동화 교리의 위조와 같은 결이라 축하하지 않음(헤더 §2.4 표)과 '하나가 되는' 끝으로 적지 않음(§3.5)을 더했다.
- 변이 셋: 다스리는 백성은 카드로 얻는다는 것(`StarTow` — C06 `petition_cross_star_line`, `DustBand` — C05 `petition_cross_dust_band`; `InwardHunger`는 변이 걸음으로만 — S11)이 처음 글에 없었다(AI 걸음만 적었다); 변이의 잎은 꼴 이름을 받지 않음(결정 3)과 `InwardHunger`·`IsInward`의 구별(결정 3, 노트 §5.4 코7)을 더했다. 성경 구절은 더하지 않았다(규칙 11).

---

## 효과·데이터의 바뀜 (글 밖 — 이 판의 행들)

- 값은 하나도 바꾸지 않는다(아홉 행의 조건 수·값·`CooldownTicks`·`BaseWeight`·Severity 그대로).
- 잎 이름(결정 3 — `rename_leaves.py`의 몫): `event_brick_shut_star` 조건 `Brick.Space.HermitMonolith` → `Brick.Space.ClosedPort`, `Brick.Form.DysonBrain` → `Brick.Form.ShutSphere`; `event_brick_fold_gleaning` `Brick.Space.PlanckMicro` → `Brick.Space.FoldedSmall`; `event_brick_inward_appetite` `Brick.Mutation.AbyssalMaw` → `Brick.Mutation.InwardHunger`; `event_brick_works_run` `Brick.Mutation.KesslerTapestry` → `Brick.Mutation.DustBand`.
- 스키마 걸이(제안 — 열린 점 1): 노트 §5.4 데3의 '`CivTimeVectorAtLeast` 또는 `BorderingCivHasTrait`를 지닌 행'은 `event_brick_shut_star`·`event_brick_drift_plenty`·`event_brick_ember_wake`(셋 다 `CivHasTrait`뿐)와 `log_brick_scan_spread`(조건 없는 로그 행)를 놓친다.

## 열린 점

1. **걸이의 빈칸** — 위 셋과 스캔 얼굴에도 silence_words·rung_grasp_words·title_words·crossroads_words가 걸리게: 사건 행은 '`CivHasTrait`의 `Brick.*` 태그' 조건을 규칙 4·7의 나열 꼴(열일곱 태그)로 더하고, 로그 행은 `{"row": "log_brick_scan_spread"}`로 따로 건다(검사기의 접두어 시험은 표의 접두어 `event_`만 받아 `event_brick_`으로 가를 수 없다 — 헤더 §3.3 규칙 5의 괄호). 지금 글은 모두 그 목록을 지난다.
2. **`Shape.LastHunger`** '끝없이 삼키는 살림' → '별의 불을 끌어 쓰는 살림'(이 판의 권장). 헤더 §3.2·설계서 §4.4의 초안 낱말을 바꾸므로 리드나 개발자가 고른다; 고르면 설계서 §4.4 표와 헤더 §3.2의 낱말도 같은 커밋에서 바꾼다.
3. **카드 C10 제목 '거두지 않은 칙령'(결정 — 노트 §5.4 데2)이 main 스키마에 걸린다** — `DT_Encounters`의 `petition_` 규칙('a petition's words name no holy word, no soul verb …')이 Title·Body·VoiceBodies·Choices.Label·SuccessText·FailureText에 `soul_verbs` `["거두","거둬","거뒀","거둔","거둘"]`를 금하고, '거두지'는 '거두'를 품는다. 이 판의 행이 아니라 카드 쪽 초안가에게 넘긴다 — 같은 뜻의 대안: '손대지 않은 칙령'(① 결과 줄의 '손대지 않은 채'와 맞물린다) 또는 '그대로 둔 칙령'. 개발자가 답한 글을 바꾸므로 리드가 확인을 받는다.
4. `log_brick_scan_spread`의 변형 셋 — S15의 '소문 한 줄'을 '스캔 한 번에 한 줄'로 읽었다. '변형도 하나'의 뜻이면 `single_variant_rows`에 행을 더하고 [0]만 남긴다(스키마 한 줄 — 리드가 고른다).
5. `KeptLaw` DevComment는 노트 §5.4 코6의 글과 다르다(코1의 코드에 맞춘 고침 — 위 why). 코6을 글자 그대로 쓰기로 하면 'save those that touch the claim or answer a crisis'가 '반드시 거둔다'로 읽힌다는 점만 남는다.
6. DevComment의 읽는 곳 가운데 스캔 얼굴(`LivingNebula`·`DarkMatter`)은 `[B-7x3]`이 그 분기를 넣은 뒤에야 참이다 — DevComment는 `[B-7x4]`에서 바꾸므로 차례(헤더 §8: `[B-7x3]` → `[B-7x4]`)대로면 맞다.

## 검사 기록

`stage3/texts/src/check_draft.py`가 이 파일의 json 블록·csv 줄·ini 줄을 뽑아 main `schema.json`의 이름 붙은 목록(위 '검사 기준')과 crossroads_words·shape_merit_words, 숫자, 슬롯(`{Civ}`·`{Hazard}` / `{System}`), `|hpp(…)`의 꼴, ini의 ASCII를 본다 — 결과는 이 파일 끝에 붙인 줄.

결과(2026-11-10): `checked 60 items: 10 json blocks, 13 csv lines, 17 ini lines; fails 0` — 줄마다의 결과는 `stage3/texts/src/check_out.txt`. 같은 검사기에 C10 제목 후보 '거두지 않은 칙령'을 넣으면 soul_verbs:거두로 깨진다(열린 점 3).
