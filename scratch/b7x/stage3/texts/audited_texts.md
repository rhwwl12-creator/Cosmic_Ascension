# [B-7x4] 글 — 감사를 마친 한 벌 (갈림 카드 열둘 · 사건 행 아홉 · ST_CA 열세 줄 · 스캔 얼굴 · DevComment 열일곱)

> 합친 것: `draft_first.md`(C01~C04) · `draft_second.md`(C05~C12) · `draft_events.md`(사건 행·스캔 얼굴·ST_CA·DevComment). 감사 다섯 렌즈(신학·목소리·문법·기계·일관성)의 남긴 지적 43건을 적용하고, 버린 지적 8건(T-04·T-06·V1·K1·TVM-5·F4·F5·F7)은 적용하지 않았다. 서로 부딪친 지적은 하나를 고르고 §8에 적었다.
> 기준: 데이터 초안 `aa09543`, 헤더·노트·설계서 `79f2583`, main의 `schema.json`·`DT_Encounters.json`·`DT_Events.json`·`ST_CA.csv`·`docs/70_CONTENT_GUIDE.md`. 2026-11-10 개발자 답(결정 3·11 (나)·13 ㉢ (d)·16·18, 노트 §5.4 데2)과 헤더 §3.5 글의 규칙, 규칙 11.
> 잎 이름은 새 이름(`KeptLaw`·`IdleFeed`·`FoldedSmall`·`ShutSphere`·`DustBand`·`InwardHunger`·`StarTow`·`ClosedPort`)이고, 이름 고침은 `rename_leaves.py`(결정 3)가 한다. 화면 글에는 어느 설계 이름도 나오지 않는다.
> json 블록은 데이터에 그대로 붙일 글 칸만 담는다(같은 칸 이름·같은 짜임). 효과·조건은 블록 위의 줄에 적고, 바꿀 효과의 정확한 JSON은 §6에 있다. 검사 결과는 §8 끝에 있다.
> 효과 줄의 '기울기'는 관례의 `ShiftEthics Spiritualism`(의회가 청한 선택 −0.05, 장로들이 청한 선택 +0.05)이다. 이 판에서 기울기는 카드를 닫는 결과에만 남는다 — 닫지 않는 ②, 되돌림 ②의 실패, 판정 ①의 실패에서는 뺐다(§6.1, 결정 11 (나); 최종 비평 F1·F13).
> 최종 비평(F1~F13)을 적용한 판이다. 적용과 건너뜀은 §8.5.

---

## 1. 갈림 카드 열둘 (`DT_Encounters`, `[B-7x5c]`가 옮긴다)

목소리 순서는 main과 같다: `VoiceBodies[0]` 의회, `[1]` 회중의 장로들. 목소리 있는 카드는 `Body`가 비어 있다(main 규칙 13). 슬롯은 주제 Civilization이라 `{Civ}`만 쓴다(main 규칙 7).

### 1.1 `petition_cross_built` — C01

- 조건: `Lottery` · 무게 1 · 주제 Civilization · `PlayerRoleIs Ruler` · `CivHasTrait Brick.Form.Society` · `CivTimeVectorAtLeast Built 1200` (그대로)
- 효과: ① 판정 Stability 8, 물질 240·통찰 120 — 성공 `AddTrait Brick.Instinct.SwarmTech`, `RemoveTrait Brick.Form.Society`, `ChangeStability −0.06`, 기울기 −0.05 / 실패 `ChangeStability −0.06` (**바뀜** — 실패의 기울기를 뺐다, §6.1) · ② 판정 없음, 통찰 100 — `RemoveTrait Brick.Form.Society`, `AddResource Matter 150`, 기울기 +0.05 · ③ 없음 (나머지 그대로 — 첫 갈림의 ① 성공과 ②는 `Society`를 걷어 카드를 닫으므로 되풀이되지 않는다; ① 실패는 닫지 않아 되풀이된다)

```json
{
  "Title": "작은 손들",
  "Body": "",
  "VoiceBodies": [
    "{Civ}의 작업장은 오래 불을 끄지 않았다. 의회는 큰 공사를 잘게 나누어 집집마다 연장을 들이자고 청했다.",
    "{Civ}의 작업장은 오래 불을 끄지 않았다. 회중의 장로들은 새 연장을 가르칠 수 있는 장인들의 손에만 맡기자고 청했다."
  ],
  "Choices": [
    {
      "Label": "집집마다 연장을 들인다",
      "SuccessText": "{Civ}의 집마다 작은 공방이 섰다. 그 일이 끝났을 때 큰 공사는 집집의 작은 손에 나뉘어 있었다.",
      "FailureText": "{Civ}의 집집에 연장이 들었지만 손에 익지 않았다. 큰 공사는 장인들의 몫으로 남았고 거리는 술렁였다."
    },
    {
      "Label": "장인 조합에만 새 연장을 맡긴다",
      "SuccessText": "{Civ}의 장인 조합이 새 연장을 맡았다. 그들의 공방에서 나온 것이 창고에 쌓였다.",
      "FailureText": ""
    },
    {
      "Label": "지켜본다",
      "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 작업장의 일은 하던 대로 이어졌다.",
      "FailureText": ""
    }
  ]
}
```

- why: 바꾼 칸 없음. 모든 칸이 규칙을 지난다(draft_first).

### 1.2 `petition_cross_consumed` — C02

- 조건: `Lottery` · 무게 1 · 주제 Civilization · `PlayerRoleIs Ruler` · `CivHasTrait Brick.Form.Society` · `CivTimeVectorAtLeast Consumed 1500` (그대로)
- 효과: ① 판정 Stability 9, 에너지 260·물질 120 — 성공 `AddTrait Brick.Diet.DarkMatter`, `RemoveTrait Brick.Form.Society`, `ChangeStability −0.08`, 기울기 +0.05 / 실패 `ChangeStability −0.08` (**바뀜** — 실패의 기울기를 뺐다, §6.1) · ② 판정 없음, 물질 200·영향력 100 — `AddTrait Brick.Diet.StarEater`, `RemoveTrait Brick.Form.Society`, `ChangeReverence Borders −0.1`, 기울기 −0.05 · ③ 없음 (나머지 그대로)

```json
{
  "Title": "땅이 내는 것",
  "Body": "",
  "VoiceBodies": [
    "{Civ}의 광맥은 오래 싸움에 쓸 것을 대느라 깊어졌다. 의회는 땅을 더 파지 말고 제 별의 불을 바로 화로에 끌어들이자고 청했다.",
    "{Civ}의 광맥은 오래 싸움에 쓸 것을 대느라 깊어졌다. 회중의 장로들은 땅을 그만 파고 별과 별 사이에 떠도는 엷은 것을 걸러 살자고 청했다."
  ],
  "Choices": [
    {
      "Label": "별 사이의 엷은 것을 걸러 쓴다",
      "SuccessText": "{Civ}의 배들이 암흑 구간에 넓은 체를 폈다. 그 일이 끝났을 때 그들의 화로는 별 사이에서 걸러 낸 것으로 타고 있었다.",
      "FailureText": "{Civ}의 배들이 암흑 구간에 편 체에는 걸리는 것이 없었다. 광맥 곁의 집들은 들인 품을 두고 술렁였다."
    },
    {
      "Label": "제 별의 불을 화로에 끌어들인다",
      "SuccessText": "{Civ}의 화로가 제 별의 불을 끌어 쓰기 시작했다. 국경 너머의 이웃들은 그 별빛을 오래 바라보았다.",
      "FailureText": ""
    },
    {
      "Label": "지켜본다",
      "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 광맥의 일은 하던 대로 이어졌다.",
      "FailureText": ""
    }
  ]
}
```

- why Choices[0].Label: '암흑 구간에 체를 편다' → '별 사이의 엷은 것을 걸러 쓴다' — V5: 장로들의 청('…엷은 것을 걸러 살자')을 선택 이름이 받아야 목소리와 선택이 맞물린다; main에서 '암흑 구간'은 레인 길의 한 토막(`enc_lane_weave`)이라 길 가는 일로 읽혔다. Success/FailureText는 '암흑 구간'을 자리로 그대로 둔다.
- 다른 칸은 그대로. ② SuccessText의 '별빛'은 dawn_words의 '빛'을 품지만 그 목록은 이 표에 걸리지 않는다(이웃의 바라봄일 뿐, 새벽 그림이 아니다).
- 이 갈래의 꼴 낱말은 `Shape.LastHunger` '별의 불을 끌어 쓰는 살림'(§3, T-02)이다. draft_first의 '끝없이 삼키는 살림' 언급은 이 낱말로 바뀐다(K2가 요구한 같은 판의 고침).

### 1.3 `petition_cross_drifted` — C03

- 조건: `Lottery` · 무게 1 · 주제 Civilization · `PlayerRoleIs Ruler` · `CivHasTrait Brick.Form.Society` · `CivTimeVectorAtLeast Drifted 900` (그대로)
- 효과: ① 판정 Stability 9, 에너지 100·물질 300 — 성공 `AddTrait Brick.Space.Nomad`, `RemoveTrait Brick.Form.Society`, `ChangeStability −0.1`, 기울기 +0.05 / 실패 `ChangeStability −0.1` (**바뀜** — 실패의 기울기를 뺐다, §6.1) · ② 판정 없음, 물질 140 — `RemoveTrait Brick.Form.Society`, `ChangeStability +0.05`, 기울기 −0.05 · ③ 없음 (나머지 그대로)

```json
{
  "Title": "뭍과 배",
  "Body": "",
  "VoiceBodies": [
    "{Civ}의 배들은 오래 새 터를 닦으러 나가 있었다. 의회는 떠나려는 집은 보내고 남으려는 집은 남게 두자고 청했다.",
    "{Civ}의 배들은 오래 새 터를 닦으러 나가 있었다. 회중의 장로들은 닿는 곳마다 집이 함께 가야 한다며 살림을 통째로 배에 싣자고 청했다."
  ],
  "Choices": [
    {
      "Label": "살림을 통째로 배에 싣는다",
      "SuccessText": "{Civ}에서 배에 사는 집이 뭍의 집보다 많아졌다. 그 일이 끝났을 때 그들은 더는 닻을 내린 곳을 집이라 부르지 않았다.",
      "FailureText": "{Civ}의 배에 살림을 실었지만 뭍을 떠나려는 집은 적었다. 실은 짐을 도로 내리는 동안 항구가 어수선했다."
    },
    {
      "Label": "떠나려는 집에만 배를 내준다",
      "SuccessText": "{Civ}의 배가 떠나려는 집들만 태우고 항로에 올랐다. 뭍에 남은 집들은 하던 살림을 이어 갔다.",
      "FailureText": ""
    },
    {
      "Label": "지켜본다",
      "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 새 터를 닦는 배들은 하던 대로 오갔다.",
      "FailureText": ""
    }
  ]
}
```

- why Title: '닻을 내리는 곳' → '뭍과 배' — TVM-11: 제목은 죽은 그릇의 비문에 마지막 답으로 나온다. 거기서 '닻을 내리는 곳'은 마지막 항구, 곧 쉴 곳으로 읽힌다(B5 감사가 비문에서 뺀 기리는 글의 결).
- why Choices[0].SuccessText: '…부르지 않고 있었다.' → '…더는 닻을 내린 곳을 집이라 부르지 않았다.' — K5: 굳어진 상태에 진행형 부정은 어색하다.
- 다른 칸은 그대로. '뭍에 남은 집들'의 '남은'은 shape_merit_words에 걸리지만 그 목록은 `Shape.` 키에만 걸린다(누가 뭍에 있었는지의 사실이고 공로가 아니다).

### 1.4 `petition_cross_isolated` — C04

- 조건: `Lottery` · 무게 1 · 주제 Civilization · `PlayerRoleIs Ruler` · `CivHasTrait Brick.Form.Society` · `CivTimeVectorAtLeast Isolated 1500` (그대로)
- 효과: ① 판정 없음, 물질 200·영향력 150 — `AddTrait Brick.Space.ClosedPort`, `RemoveTrait Brick.Form.Society`, `ChangeReverence Borders −0.1`, 기울기 +0.05 · ② 판정 없음, 영향력 80 — `RemoveTrait Brick.Form.Society`, `AddResource Insight 120`, 기울기 −0.05 · ③ 없음 (모두 그대로, 잎 이름만 `HermitMonolith`→`ClosedPort`)

```json
{
  "Title": "항구의 빗장",
  "Body": "",
  "VoiceBodies": [
    "{Civ}의 백성은 오래 저희끼리 지냈다. 의회는 큰 항구만 남기고 나머지를 닫아 안을 짓는 데 힘을 모으자고 청했다.",
    "{Civ}의 백성은 오래 저희끼리 지냈다. 회중의 장로들은 바깥의 것이 들면 지켜 온 것이 흐려진다며 항구를 모두 닫자고 청했다."
  ],
  "Choices": [
    {
      "Label": "항구를 닫고 새 터를 찾지 않는다",
      "SuccessText": "{Civ}의 항구들이 닫히고 새 터로 떠날 배들이 항구에 매였다. 그 일이 끝났을 때 그들은 제 별 안쪽만 짓고 있었다.",
      "FailureText": ""
    },
    {
      "Label": "큰 항구만 남기고 닫는다",
      "SuccessText": "{Civ}의 작은 항구들이 닫히고 큰 항구만 열려 있었다. 바깥으로 쓰던 손이 안을 짓는 데로 돌아갔고, 학자들의 셈이 깊어졌다.",
      "FailureText": ""
    },
    {
      "Label": "지켜본다",
      "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 항구들은 하던 대로 열려 있었다.",
      "FailureText": ""
    }
  ]
}
```

- why Choices[0].SuccessText: '…항구에 묶였다.' → '…항구에 매였다.' — T-10: '묶'은 B6b가 결박의 두 줄에 남겨 둔 낱말(binding_words)이다.
- why Choices[1].SuccessText 둘째 문장: '바깥으로 쓰던 손이 안을 짓는 셈으로 돌았다.' → '바깥으로 쓰던 손이 안을 짓는 데로 돌아갔고, 학자들의 셈이 깊어졌다.' — K4('셈으로 돌았다'는 '셈이 되었다'로 읽힌다 → '안을 짓는 데로 돌아갔다')와 V6(효과 통찰 +120을 줄이 말해야 한다)를 합쳤다. '셈'은 C11 ②(TVM-6)처럼 학자들의 셈으로만 쓴다.
- '제 별 안쪽만'은 그대로 — '별'은 복수 표지 없이 '제 별들'로도 읽힌다(F6의 판단).

### 1.5 `petition_cross_dust_band` — C05

- 조건: `Lottery` · 무게 1 · 주제 Civilization · `PlayerRoleIs Ruler` · `CivTierAtLeast 3` · `CivHasTrait Brick.Instinct.SwarmTech` · `CivLacksTrait Brick.Mutation.DustBand` · `CivLacksTrait Brick.Instinct.HiveMerge` · `CivTimeVectorAtLeast Linked 1800`
- 효과: ① 판정 Stability 9, 에너지 150·물질 350 — 성공 `AddTrait Brick.Mutation.DustBand`, 기울기 −0.05 / 실패 `ChangeStability −0.05` (**바뀜** — 실패의 기울기를 뺐다) · ② 판정 없음, 물질 150 — `ChangeStability +0.05` 하나 (**바뀜** — `ChangePopulation +0.03`과 기울기를 빼고 `ChangeStability +0.05`를 넣었다, §6) · ③ 없음

```json
{
  "Title": "별 둘레의 티끌 띠",
  "Body": "",
  "VoiceBodies": [
    "{Civ}의 작은 일꾼들은 스스로를 고쳐 가며 일한다. 의회는 그 일꾼들을 별마다 띠로 둘러 손이 닿지 않던 일까지 맡기자고 청했다.",
    "{Civ}의 작은 일꾼들은 스스로를 고쳐 가며 일한다. 회중의 장로들은 누구의 손에도 매이지 않는 것을 늘리기 전에 사람을 먼저 돌보자고 청했다."
  ],
  "Choices": [
    {
      "Label": "일꾼들을 별마다 띠로 두른다",
      "SuccessText": "{Civ}의 별마다 티끌 같은 일꾼들이 띠를 이루었다. 띠는 스스로를 고치며 돌았고, 그 일을 다 아는 사람은 없었다.",
      "FailureText": "{Civ}의 일꾼들은 띠를 이루지 못하고 흩어졌다. 들인 것을 두고 의회의 말이 엇갈렸다."
    },
    {
      "Label": "사람을 먼저 돌본다",
      "SuccessText": "{Civ}의 물자가 집과 배움터를 짓는 데 들었다. 아이를 기르는 집들의 근심이 덜어졌다.",
      "FailureText": ""
    },
    {
      "Label": "지켜본다",
      "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 작은 일꾼들은 하던 대로 일했다.",
      "FailureText": ""
    }
  ]
}
```

- why VoiceBodies[1]: '…사람을 먼저 기르자고 청했다.' → '…사람을 먼저 돌보자고 청했다.' — V3: ②에서 `ChangePopulation`이 빠졌으므로 장로들의 청이 사람 수를 늘리는 일로 읽히지 않게 한다.
- why Choices[1].Label: '사람을 먼저 기른다' → '사람을 먼저 돌본다' — V3, 같은 까닭(선택 이름은 그 선택이 하는 일이다 — 헤더 §3.5).
- why Choices[1].SuccessText: '…집집에 식구가 늘었다.' → '…아이를 기르는 집들의 근심이 덜어졌다.' — 효과가 `ChangeStability`로 바뀌어 '식구가 늘었다'는 일어나지 않는 일이다(draft_second). 새 문장은 안정도의 사실이고 사람의 수를 말하지 않는다.
- 다른 칸은 그대로.

### 1.6 `petition_cross_star_line` — C06

- 조건: `Lottery` · 무게 1 · 주제 Civilization · `PlayerRoleIs Ruler` · `CivTierAtLeast 3` · `CivHasTrait Brick.Space.Nomad` · `CivLacksTrait Brick.Mutation.StarTow` · `CivTimeVectorAtLeast Drifted 1800`
- 효과: ① 판정 Stability 9, 에너지 300·물질 150 — 성공 `AddTrait Brick.Mutation.StarTow`, 기울기 −0.05 / 실패 `ChangeStability −0.06` (**바뀜** — 실패의 기울기를 뺐다) · ② 판정 없음, 물질 250 — `ChangeStability +0.05` 하나 (**바뀜** — 기울기를 뺐다, §6) · ③ 없음 (②는 값과 작은 한 번의 효과뿐)

```json
{
  "Title": "별에 거는 줄",
  "Body": "",
  "VoiceBodies": [
    "{Civ}의 살림은 오래 배 위에서 꾸려졌다. 의회는 도읍의 별에 끄는 줄을 걸어 데리고 떠날 채비를 하자고 청했다.",
    "{Civ}의 살림은 오래 배 위에서 꾸려졌다. 회중의 장로들은 떠날 채비로 지은 집에서는 아무도 뿌리를 내리지 못한다며 집부터 고치자고 청했다."
  ],
  "Choices": [
    {
      "Label": "도읍의 별에 끄는 줄을 건다",
      "SuccessText": "{Civ}|hpp(이,가) 도읍의 별에 끄는 줄을 걸었다. 별은 제자리에 있었고, 그들은 떠날 채비를 한 채 살게 되었다.",
      "FailureText": "{Civ}의 배들이 도읍의 별에 줄을 걸다가 놓쳤다. 끊어진 줄을 두고 배 위의 집들이 술렁였다."
    },
    {
      "Label": "집부터 고쳐 짓는다",
      "SuccessText": "{Civ}의 배 위 집들이 새로 고쳐졌다. 그 안에서 사는 날이 한결 편해졌다.",
      "FailureText": ""
    },
    {
      "Label": "지켜본다",
      "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 배 위의 살림은 하던 대로 꾸려졌다.",
      "FailureText": ""
    }
  ]
}
```

- why VoiceBodies[1]: '…아무도 늙지 못한다며…' → '…아무도 뿌리를 내리지 못한다며…' — T-08·K7: '늙지 못한다'는 늙지 않음(헤더 §3.5의 '불멸' 그림)으로도, 모두 젊어서 죽는다는 말로도 읽힌다. 뜻은 '자리를 잡지 못한다'이다.
- 다른 칸은 그대로. '떠날 채비'는 목소리와 결과 줄의 낱말이고 제목이 아니므로 비문에 나오지 않는다(TVM-4의 판단).

### 1.7 `petition_cross_between_stars` — C07

- 조건: `Lottery` · 무게 1 · 주제 Civilization · `PlayerRoleIs Ruler` · `CivTierAtLeast 3` · `CivHasTrait Brick.Diet.DarkMatter` · `CivLacksTrait Brick.Form.LivingNebula` · `CivTimeVectorAtLeast Consumed 2000`
- 효과: ① 판정 Stability 9, 에너지 300·통찰 150 — 성공 `AddTrait Brick.Form.LivingNebula`, 기울기 +0.05 / 실패 `ChangeStability −0.06` (**바뀜** — 실패의 기울기를 뺐다) · ② 판정 없음, 물질 250·영향력 60 — `ChangeStability +0.04` 하나 (**바뀜** — `ShiftEthics Openness −0.05`와 기울기를 뺐다, §6) · ③ 없음

```json
{
  "Title": "별 사이의 엷은 것",
  "Body": "",
  "VoiceBodies": [
    "{Civ}의 살림은 별 사이에서 걸러 낸 것으로 오래 꾸려졌다. 의회는 지붕부터 고쳐 땅 위에 디딜 곳을 지키자고 청했다.",
    "{Civ}의 살림은 별 사이에서 걸러 낸 것으로 오래 꾸려졌다. 회중의 장로들은 땅 위의 살림을 별들 사이의 엷은 것에 풀자고 청했다."
  ],
  "Choices": [
    {
      "Label": "살림을 별들 사이에 푼다",
      "SuccessText": "{Civ}|hpp(이,가) 살림을 별들 사이의 엷은 것에 풀었다. 그 일이 끝났을 때 그들의 살림은 별과 별 사이에 번져 있었다.",
      "FailureText": "{Civ}|hpp(이,가) 별들 사이에 푼 살림은 번지지 못하고 흩어졌다. 땅 위의 집들이 그 일을 두고 술렁였다."
    },
    {
      "Label": "땅 위의 집을 고쳐 짓는다",
      "SuccessText": "{Civ}의 땅 위 집들이 지붕부터 고쳐졌다. 디딜 곳이 단단해지자 그 집들이 마음을 놓았다.",
      "FailureText": ""
    },
    {
      "Label": "지켜본다",
      "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 별 사이에서 걸러 내는 일은 하던 대로 이어졌다.",
      "FailureText": ""
    }
  ]
}
```

- why VoiceBodies[1]: '땅에 매인 살림을' → '땅 위의 살림을' — T-05(고친 꼴): '땅에 매인 … 풀자'의 매다/풀다 짝이 매고 푸는 말씀(마 16:19)을 레버에 들인다; '매인'만 바꿔 짝을 끊었다. 장로들은 여전히 ①을, 의회는 ②를 청한다.
- why Choices[0].SuccessText 둘째 문장: '…그들은 별과 별 사이에 번져 서 있었다.' → '…그들의 살림은 별과 별 사이에 번져 있었다.' — K6: '번져 서 있었다'는 번짐과 섬을 이어 어색하다.
- why Choices[1].SuccessText 둘째 문장: '…디딜 곳이 단단해지자 거리의 술렁임이 가라앉았다.' → '…디딜 곳이 단단해지자 그 집들이 마음을 놓았다.' — V4: 어느 목소리도 거리의 술렁임을 말하지 않았다. V4의 고친 글은 '땅 위의 집들이'였으나 같은 줄 앞 문장의 '땅 위 집들'과 겹쳐 '그 집들이'로 받았다(뜻 같음).
- 다른 칸은 그대로.

### 1.8 `petition_cross_into_dust` — C08

- 조건: `Lottery` · 무게 1 · 주제 Civilization · `PlayerRoleIs Ruler` · `CivTierAtLeast 3` · `CivHasTrait Brick.Space.Nomad` · `CivLacksTrait Brick.Space.FoldedSmall` · `CivLacksTrait Brick.Mutation.StarTow` · `CivTimeVectorAtLeast Endured 1800`
- 효과: ① 판정 Stability 9, 에너지 300·통찰 200 — 성공 `AddTrait Brick.Space.FoldedSmall`, `RemoveTrait Brick.Space.Nomad`, 기울기 −0.05 / 실패 `ChangeStability −0.06` (**바뀜** — 실패의 기울기를 뺐다) · ② 판정 없음, 물질 200 — `ChangeStability +0.05` 하나 (**바뀜** — 기울기를 뺐다, §6) · ③ 없음

```json
{
  "Title": "가장 작은 눈금",
  "Body": "",
  "VoiceBodies": [
    "{Civ}|hpp(은,는) 떠돈 날이 길었다. 의회는 가장 작은 눈금 속에 다투는 이 없는 넓이가 있다며 그리로 접혀 들자고 청했다.",
    "{Civ}|hpp(은,는) 떠돈 날이 길었다. 회중의 장로들은 밖을 내다보지 않는 살림은 밖의 일을 잊는다며 별 곁에 디딜 곳을 마련하자고 청했다."
  ],
  "Choices": [
    {
      "Label": "가장 작은 눈금 속으로 접혀 든다",
      "SuccessText": "{Civ}|hpp(이,가) 살림을 가장 작은 눈금 속으로 접어 넣었다. 그 일이 끝났을 때 그들은 그 눈금 속에서 살고 있었고, 떠돌던 배들은 항구에 매여 있었다.",
      "FailureText": "{Civ}|hpp(이,가) 살림을 접어 넣으려 했지만 눈금은 열리지 않았다. 떠돌던 배 위의 집들이 술렁였다."
    },
    {
      "Label": "별 곁에 디딜 곳을 마련한다",
      "SuccessText": "{Civ}의 배들이 별 곁에 디딜 곳을 닦았다. 떠돌던 집들이 그곳에서 숨을 골랐다.",
      "FailureText": ""
    },
    {
      "Label": "지켜본다",
      "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 배들은 하던 대로 항로를 떠돌았다.",
      "FailureText": ""
    }
  ]
}
```

- why Title: '티끌 속의 넓이' → '가장 작은 눈금' — TVM-11: 비문에서 '티끌 속'은 필멸의 티끌로 읽힌다. 새 제목은 의회 목소리의 '가장 작은 눈금 속에'를 받는다.
- why VoiceBodies[0]·[1] 첫 문장: '{Civ}|hpp(은,는) 떠도는 동안 견딘 날이 길었다.' → '{Civ}|hpp(은,는) 떠돈 날이 길었다.' — 노트 §5.4 데2·리뷰 L4-18(구속: '떠돈'). '견딘'은 desolation_words.
- why Choices[0].Label: '티끌 속으로 접혀 든다' → '가장 작은 눈금 속으로 접혀 든다' — T-09를 이 칸까지 넓혔다: 같은 '티끌로 돌아감' 그림이 선택 이름에도 있었다(리드 확인 — §8 열린 점 9).
- why Choices[0].SuccessText 둘째 문장: '…그들은 티끌 속에 서 있었고, 떠돌던 배들은 비어 있었다.' → '…그들은 그 눈금 속에서 살고 있었고, 떠돌던 배들은 항구에 매여 있었다.' — T-09: 살아서 성계를 지닌 백성(K1)에게 티끌로 돌아간·사라진 그림(시 90:3)을 씌우지 않는다. 결정 16(옛 백성이 살아남았다·돌아왔다는 말 없음)도 지킨다.
- 다른 칸은 그대로.

### 1.9 `petition_cross_wrapped_star` — C09

- 조건: `Lottery` · 무게 1 · 주제 Civilization · `PlayerRoleIs Ruler` · `CivTierAtLeast 2` · `CivHasTrait Brick.Space.ClosedPort` · `CivLacksTrait Brick.Form.ShutSphere` · `CivTimeVectorAtLeast Isolated 2000`
- 효과: ① 판정 없음, 에너지 200·물질 400 — `AddTrait Brick.Form.ShutSphere`, `ShiftEthics Openness −0.05`, 기울기 −0.05 (카드를 닫는다) · ② 판정 Stability 8, 영향력 150 — 성공 `RemoveTrait Brick.Space.ClosedPort`, `ChangeStability −0.05`, 기울기 +0.05 (부모가 걷혀 닫힌다) / 실패 `ChangeStability −0.1` (**바뀜** — 닫지 않는 실패의 기울기를 뺐다, §6) · ③ 없음 (나머지 그대로, 잎 이름만 새 이름)

```json
{
  "Title": "닫힌 구의 셈",
  "Body": "",
  "VoiceBodies": [
    "{Civ}|hpp(은,는) 오래 닫혀 있었고 그동안 제 별을 감싼 구를 지었다. 의회는 그 구를 닫아 별의 열을 모두 셈에 돌리자고 청했다.",
    "{Civ}|hpp(은,는) 오래 닫혀 있었다. 회중의 장로들은 닫힌 집에는 바깥의 소식이 늦게 닿는다며 빗장을 풀고 이웃을 맞자고 청했다."
  ],
  "Choices": [
    {
      "Label": "감싼 구를 닫아 셈에 돌린다",
      "SuccessText": "{Civ}|hpp(이,가) 제 별을 감싼 구를 닫았다. 그 별의 열은 모두 안쪽의 셈으로 흘러들었다.",
      "FailureText": ""
    },
    {
      "Label": "닫아 둔 살림을 연다",
      "SuccessText": "{Civ}|hpp(이,가) 닫아 둔 살림을 열었다. 안에서만 정하던 일들이 바깥을 보며 정해지기 시작했다.",
      "FailureText": "{Civ}의 빗장은 풀리지 않았다. 열자는 말과 닫자는 말이 맞서며 집집이 흔들렸다."
    },
    {
      "Label": "지켜본다",
      "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 닫힌 항구들은 그대로 닫혀 있었다.",
      "FailureText": ""
    }
  ]
}
```

- why: 바꾼 칸 없음('닫힌 집'·'닫힌 항구'는 desolation_words의 '닫힌 문'·'문이 닫'에 걸리지 않는다).

### 1.10 `petition_cross_law_stays` — C10

- 조건: `Lottery` · 무게 1 · 주제 Civilization · `PlayerRoleIs Ruler` · `CivTierAtLeast 3` · `CivLacksTrait Brick.Form.KeptLaw` · `CivTimeVectorAtLeast Endured 2000`
- 효과: ① 판정 없음, 값 없음 — `AddTrait Brick.Form.KeptLaw`, `DeposeRuler`(Tag `Reign.Abdicated`), 기울기 −0.05 (그대로) · ② 판정 없음, 영향력 60 — `ChangeStability +0.05` 하나 (**바뀜** — `ChangeReverence +0.1`·`PlayerStatDelta Exaltation +1`·`ShiftEthics Spiritualism +0.05`를 뺐다, §6) · ③ 없음

```json
{
  "Title": "손대지 않은 칙령",
  "Body": "",
  "VoiceBodies": [
    "{Civ}의 칙령은 눌린 날들 내내 다스리는 이의 손에 매여 있었다. 의회는 다스리는 이가 정해 둔 칙령에 손대지 않은 채 내려오기를 청했다.",
    "{Civ}의 칙령은 눌린 날들 내내 다스리는 이의 손에 매여 있었다. 회중의 장로들은 그 칙령을 지켜 온 이의 이름을 칙령 머리에 새겨 뒷사람이 알게 하자고 청했다."
  ],
  "Choices": [
    {
      "Label": "칙령을 남기고 왕좌를 내려놓는다",
      "SuccessText": "{Civ}|hpp(은,는) 정해 둔 칙령을 손대지 않은 채 두었다. 칙령을 정한 이는 자리에서 내려왔다.",
      "FailureText": ""
    },
    {
      "Label": "칙령 머리에 다스리는 이의 이름을 새긴다",
      "SuccessText": "{Civ}의 칙령 머리마다 다스리는 이의 이름이 새겨졌다. 칙령을 읽는 이들은 누가 정한 법인지 알고 마음을 놓았다.",
      "FailureText": ""
    },
    {
      "Label": "지켜본다",
      "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 칙령은 다스리는 이의 손에 그대로 매여 있었다.",
      "FailureText": ""
    }
  ]
}
```

- why Title: '비워 둔 왕좌' → '손대지 않은 칙령' — **개발자 답(노트 §5.4 데2 '거두지 않은 칙령')에서 벗어난다**(T-01·TVM-2): '거두'는 main `schema.json`의 `words.soul_verbs`이고 DT_Encounters 규칙 0(`row_prefix petition_` → Title `lacks_words soul_verbs`, 부분 문자열 비교 — `Tools/cosmic_data_schema.py:1395`)에서 깨진다. 새 제목은 ① 결과 줄의 '손대지 않은 채'를 받고, 빈 자리·돌아올 왕의 그림이 없으며, 결정 18의 좁힘보다 더 약속하지 않는다. 이 벗어남은 헤더 §11에 적어 개발자가 확인한다(§8 열린 점 1).
- why VoiceBodies[0]: '…의회는 왕좌를 비우고도 칙령이 서는지 보아야 한다며 내려오기를 청했다.' → '…의회는 다스리는 이가 정해 둔 칙령에 손대지 않은 채 내려오기를 청했다.' — T-07: 빈 왕좌를 지켜보는 그림(돌아오는 왕, 리뷰 T8)이 남은 마지막 자리였다. 새 글은 ① 결과 줄의 낱말을 다시 쓰고 그 뒤의 일을 약속하지 않는다(결정 18). 주어를 앞에 둔 것은 최종 비평 F7: '…않은 채 다스리는 이가'는 '칙령에 손대지 않고 다스리는 이'로도 읽혔다(낱말은 같다).
- why Choices[1].SuccessText 둘째 문장: '칙령을 읽는 이들은 그 이름도 함께 읽었다.' → '칙령을 읽는 이들은 누가 정한 법인지 알고 마음을 놓았다.' — 최종 비평 F8: ②의 효과가 `ChangeStability +0.05` 하나가 되었으므로(우러름·높아짐을 뺐다) 줄은 이름을 높이는 그림 대신 안정도의 사실을 말한다(C05·C07·C11과 같은 고침).
- why VoiceBodies[0]·[1] 첫 문장: '견딘 날들 내내' → '눌린 날들 내내' — 노트 §5.4 데2(구속).
- why Choices[0].SuccessText: '…왕좌는 비어 있었다.' → '{Civ}|hpp(은,는) 정해 둔 칙령을 손대지 않은 채 두었다. 칙령을 정한 이는 자리에서 내려왔다.' — 노트 §5.4 데2 글자 그대로(구속; 양위 뒤 빈 자리 그림 금지 — 헤더 §3.5).
- 다른 칸은 그대로. ① Label의 '왕좌'는 하는 일의 이름이다(main `petition_keep_throne`과 같은 결).

### 1.11 `petition_cross_seam` — C11

- 조건: `Lottery` · 무게 1 · 주제 Civilization · `PlayerRoleIs Ruler` · `CivTierAtLeast 3` · `CivHasTrait Brick.Instinct.SwarmTech` · `CivLacksTrait Brick.Instinct.HiveMerge` · `CivLacksTrait Brick.Mutation.DustBand` · `CivTimeVectorAtLeast Linked 2000`
- 효과: ① 판정 없음, 값 없음 — `AddTrait Brick.Instinct.HiveMerge`, `RemoveTrait Brick.Instinct.SwarmTech`, `ChangeStability +0.08`, `PlayerStatDelta Exaltation +2`, 기울기 −0.05 (그대로 — 노트 데2: 값은 지금 고치지 않는다) · ② 판정 없음, 영향력 80 — `AddResource Insight 120`, `ChangeStability −0.02` (**바뀜** — `ShiftEthics Openness +0.05`와 기울기를 빼고 `AddResource Insight 120`을 넣었다, §6) · ③ 없음

```json
{
  "Title": "이음매",
  "Body": "",
  "VoiceBodies": [
    "{Civ}|hpp(은,는) 오래 이웃과 맺은 것을 지키며 여러 목소리로 일을 정해 왔다. 의회는 이음매를 지우고 모두가 같은 답을 내게 하자고 청했다.",
    "{Civ}|hpp(은,는) 오래 이웃과 맺은 것을 지키며 여러 목소리로 일을 정해 왔다. 회중의 장로들은 이음매가 서로 다른 이들이 맞닿은 자리라며 지우지 말고 손보자고 청했다."
  ],
  "Choices": [
    {
      "Label": "이음매를 지운다",
      "SuccessText": "{Civ}의 회의장에서 다른 말이 그쳤다. 모두가 같은 것을 말했고, 그것은 당신이 한 말이었다.",
      "FailureText": ""
    },
    {
      "Label": "맞닿은 자리를 손본다",
      "SuccessText": "{Civ}의 이웃과 맞닿은 자리마다 고쳐 쓸 말이 정해졌다. 회의는 길어졌지만 거기서 오간 말이 학자들의 셈에 보태졌다.",
      "FailureText": ""
    },
    {
      "Label": "지켜본다",
      "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 회의장에서는 여러 목소리가 하던 대로 오갔다.",
      "FailureText": ""
    }
  ]
}
```

- why Choices[1].SuccessText 둘째 문장: '여러 목소리를 맞추는 동안 회의는 길어졌다.' → '회의는 길어졌지만 거기서 오간 말이 학자들의 셈에 보태졌다.' — TVM-6: Openness를 뺀 ②는 ③(무료·무효과)보다 값만 많은 선택이 되어 장로들의 청이 뜻을 잃었다. 작은 한 번의 효과 `AddResource Insight 120`(값은 헤더 §5 ⑧ `-Crossroads` 측정이 정한다)을 넣고 줄이 그 사실을 말한다.
- 다른 칸은 그대로. ① 결과 줄은 뒤엉킴을 좋은 끝으로 적지 않는다('하나가 되'·일체화 없음).

### 1.12 `enc_cross_drift_fleet` — C12

- 조건: `Standing` · 무게 1 · 주제 VesselSystem · `PlayerRoleIs Wanderer` · `CivTimeVectorAtLeast Drifted 900` · `CivSettledSystemsAtLeast 2` · `CivLacksTrait Brick.Space.Nomad` · `CivLacksTrait Brick.Space.ClosedPort` · `CivLacksTrait Brick.Space.FoldedSmall` · `CivLacksTrait Brick.Form.Society`
- 효과: ① 판정 Body 5, 값 없음 — 성공 `AddTrait Brick.Space.Nomad`(주인 백성), `ChangeReverence +0.2` / 실패 `ChangeReverence −0.1` · ② 판정 Body 4, 값 없음 — 성공 없음 / 실패 없음 (**바뀜** — 성공의 `ChangeReverence +0.05`를 뺐다, §6; 최종 비평 F4) · ③ 없음

```json
{
  "Title": "조선소의 갈림",
  "Body": "{System}의 사람들은 집을 통째로 싣는 배를 지으며 살림을 배에 옮길지를 두고 갈려 있다. 조선소에는 반쯤 지은 배들이 늘어서 있다.",
  "VoiceBodies": [],
  "Choices": [
    {
      "Label": "배 위에서 사는 법을 보인다",
      "SuccessText": "당신은 배 위에서 먹고 자고 고치는 법을 보였다. {Civ}의 사람들은 살림을 배에 싣기 시작했다.",
      "FailureText": "당신이 보인 배 위의 살림은 {Civ}의 사람들에게 닿지 않았다. 조선소의 사람들은 당신을 낯설게 보았다."
    },
    {
      "Label": "조선소에서 일손을 보탠다",
      "SuccessText": "당신은 조선소에서 배를 짓는 일을 거들었다. 반쯤 지은 배 가운데 한 척이 마저 지어졌다.",
      "FailureText": "당신이 거든 일은 조선소에 보탬이 되지 못했다. 배는 {Civ}의 일꾼들 손으로 지어졌다."
    },
    {
      "Label": "물러난다",
      "SuccessText": "당신은 조선소 앞에서 걸음을 돌렸다. 떠날지 남을지는 {Civ}의 사람들이 정할 일로 남았다.",
      "FailureText": ""
    }
  ]
}
```

- why Title: '떠날 채비' → '조선소의 갈림' — TVM-4: 제목은 죽은 그릇의 비문에 마지막 답으로 나오고, 거기서 '떠날 채비'는 죽음을 맞을 채비라는 말로 읽힌다. 비문에 나올 제목에는 떠남·죽음으로 읽힐 낱말이 없어야 한다. `epitaph_leaves_out`에는 넣지 않는다(상수와 코드가 든다). 카드 종류 낱말을 화면에서 빼려면 '집을 싣는 배'(§8 열린 점 5).
- why Body: '…붙박인 터를 버릴지를 두고 갈려 있다. 떠도는 살림이 오래되면 제 별까지 데리고 간다고 기록은 적는다.' → '…살림을 배에 옮길지를 두고 갈려 있다. 조선소에는 반쯤 지은 배들이 늘어서 있다.' — TVM-3: ①이 주는 `Nomad`는 수도와 성계를 지닌 채 정착이 빨라질 뿐이라 '터를 버림'은 블록에 없는 일이고, 둘째 문장은 슬라이스 가가 하지 않는 별 끌기를 미리 말한다. 기록원 인용의 틀 자체는 허용된다(docs/70; F4·T-06을 버린 까닭) — 고침의 까닭은 그 인용이 지어낸 앞날 말이라는 점이다.
- why VoiceBodies: 칸을 넣었다(`[]`) — F10: aa09543에 빈 배열로 있다. 목소리 없는 카드는 Body로 말한다(main 규칙 12).
- why Choices[0].Label: '떠도는 살림의 법을 보인다' → '배 위에서 사는 법을 보인다' — V7: 이 게임에서 '법'은 칙령의 낱말이라 '살림의 법'이 법을 펴는 일로 읽혔다.
- why Choices[0].SuccessText 둘째 문장: '…터를 버리는 쪽으로 기울었다.' → '…살림을 배에 싣기 시작했다.' — TVM-3(터를 버림은 블록에 없는 일)과 최종 비평 F6(효과는 주인 백성을 `Nomad`로 만든다 — '…쪽으로 기울었다'는 마음이 기운 것만 말해, 백성의 바뀜을 한 일과 그 뒤의 자리로 적는 헤더 §3.5에 못 미친다).
- why Choices[1].SuccessText 둘째 문장: '{Civ}의 일꾼들이 당신의 손을 기억했다.' → '반쯤 지은 배 가운데 한 척이 마저 지어졌다.' — 최종 비평 F4: ②는 카드를 닫지 않는 `Standing` 카드의 선택이라 성공의 `ChangeReverence +0.05`가 카드가 다시 올 때마다 우러름에 쌓였다(공짜 Body 4 판정으로 즉위의 문을 가는 길). 효과를 비웠으므로 줄은 우러름을 비추지 않고 Body의 '반쯤 지은 배들'을 받아 한 일의 사실만 말한다. '한 척'은 배의 수이고 사람의 수가 아니며 숫자가 아니다.
- why Choices[2].SuccessText 첫 문장: '당신은 조선소 곁에 서 있었다.' → '당신은 조선소 앞에서 걸음을 돌렸다.' — V2: '물러난다'인데 그 자리에 서 있었다는 줄은 이름과 어긋난다(main의 '물러난다'는 모두 떠나는 줄).
- 다른 칸은 그대로. 방랑자에게 칭호가 없고 사람의 수가 없다.

---

## 2. 사건 행 아홉 (`DT_Events`, `[B-7x5c]`가 옮긴다)

공통: 주제 Civilization, `LogCategory` `Log.Civ`, 슬롯은 `{Civ}`(와 `{Hazard}`)뿐, 변형 셋. 값·`BaseWeight`·`CooldownTicks`·Severity는 하나도 바꾸지 않는다.

### 2.1 `event_brick_shut_star` — S01

- 조건: `CivHasTrait Brick.Space.ClosedPort` ∧ `CivHasTrait Brick.Form.ShutSphere` (Lottery, 무게 1, 식힘 600, Severity 0, `Event.Civ.Economy`)
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

- why: 바꾼 칸 없음.

### 2.2 `event_brick_heat_at_door` — S04

- 조건: `BorderingCivHasTrait Brick.Diet.CrisisParasite` ∧ `CivTimeVectorAtLeast Endured 1200` (Lottery, 1, 600, Severity 0, `Event.Civ.Society`)
- 효과: `ChangeStability −0.03`

```json
{
  "LogTemplates": [
    "{Civ}의 국경 너머에서 재앙 곁에 사는 이들이 서성인다고 한다. 오래 눌려 온 집들은 그들을 내쫓지 않지만, 거리는 술렁인다.",
    "{Civ}의 국경 장터에 재앙 곁에서 사는 이웃의 배들이 드나들었다. 오래 눌려 온 이들은 그 배를 보며 말을 낮추었다.",
    "{Civ}의 변경에서 이웃이 재앙의 자리를 찾아다닌다는 말이 돌았다. 그 말이 도는 동안 거리의 목소리가 날카로워졌다."
  ]
}
```

- why [0]: '{Civ}의 국경 너머에서 재앙의 열을 핥는 이들이 서성인다고 한다. 오래 견딘 집들은 그들을 내쫓지 않는다.' → 위 글 — T-11: '핥는'은 서술자가 이웃을 깎아내리는 그림이다(성경에서 티끌을 핥는 것은 저주 아래 놓인 원수의 자세다). 이 블록은 씨앗에서만 오고(S04) 그 백성이 고른 일이 아니다. '견딘' → '눌려 온'과 둘째 절('거리는 술렁인다' — 이 행의 안정도 내림)은 draft_events 그대로(desolation_words, 헤더 §3.5).
- why [1]: '오래 견딘 이들은' → '오래 눌려 온 이들은' — 같은 까닭(draft_events).
- why [2]: 그대로.

### 2.3 `event_brick_drift_plenty` — S05 (앞쪽)

- 조건: `CivHasTrait Brick.Instinct.SwarmTech` ∧ `CivHasTrait Brick.Space.Nomad` (Lottery, 1, 600, Severity 1, `Event.Civ.Economy`)
- 효과: `AddResource Matter +30`, `ChangePopulation +0.02`

```json
{
  "LogTemplates": [
    "{Civ}의 떠도는 일손이 닿는 별마다 광맥을 열었다. 배 위의 창고가 차고 식구가 늘었다.",
    "{Civ}의 작은 일꾼들이 새로 닿은 자리에서 쓸 것을 캐 올렸다. 배마다 식구가 늘었다.",
    "{Civ}의 배들이 지나는 항로마다 캐 올린 것이 쌓였다. 배 위의 살림이 넉넉해지고 집이 늘었다."
  ]
}
```

- why [2]: '…집이 불었다.' → '…집이 늘었다.' — K8: '붇다'는 식구·재산과 어울리고 '불었다'는 '(바람이) 불었다'로도 읽힌다.
- why [0]·[1]: 그대로(사람의 수는 '식구가 늘었다'의 규모뿐, 숫자 없음).

### 2.4 `event_brick_emptied_trail` — S05 (뒤쪽)

- 조건: `CivHasTrait Brick.Instinct.SwarmTech` ∧ `CivHasTrait Brick.Space.Nomad` ∧ `CivTimeVectorAtLeast Drifted 1800` (Lottery, 1, 600, Severity 1, `Event.Civ.Economy`)
- 효과: `ChangeStability −0.03`, `ChangePopulation −0.03`, `AddResource Matter −40`

```json
{
  "LogTemplates": [
    "{Civ}의 배들이 지나온 자리에서는 더는 아무것도 올라오지 않았다. 갱도 어귀의 집들이 비어 갔다.",
    "{Civ}의 일꾼들이 캐던 자리마다 바닥이 드러났다. 창고가 줄고 배 위의 식구도 줄었다.",
    "{Civ}의 항로 뒤편에 파헤친 별들이 빈 채 남았다. 다음 터를 찾는 동안 배 위의 살림이 줄고 다툼이 잦아졌다."
  ]
}
```

- why [2]: '…파헤친 별들이 비어 남았다.' → '…파헤친 별들이 빈 채 남았다.' — K9: '비어 남았다'는 두 동사가 어색하게 붙는다.
- why [0]·[1]: 그대로(벌로 서술하지 않는다 — S06의 지킴과 같은 결).

### 2.5 `event_brick_ember_wake` — S06

- 조건: `CivHasTrait Brick.Diet.StarEater` ∧ `SystemHasHazard Hazard.PreSupernova` (Standing, 무게 0, 식힘 300, Severity 0, `Event.Civ.Society`)
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

- why: 바꾼 칸 없음(벌로 서술하지 않고 별의 위기를 백성의 탓으로 잇지 않는다 — S06).

### 2.6 `event_brick_own_hour` — S07

- 조건: `BorderingCivHasTrait Brick.Instinct.HiveMerge` ∧ `CivTimeVectorAtLeast Isolated 1500` (Lottery, 1, 600, Severity 0, `Event.Civ.Society`)
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

- why: 바꾼 칸 없음(뒤엉킴을 좋은 끝으로 적지 않고 홀로 선 쪽도 옳다고 적지 않는다).

### 2.7 `event_brick_fold_gleaning` — S08

- 조건: `CivHasTrait Brick.Space.FoldedSmall` ∧ `BorderingCivHasTrait Brick.Space.Nomad` (Lottery, 1, 600, Severity 0, `Event.Civ.Economy`)
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

- why [1]: '…오갔다. …새 셈을 찾았다.' → '…오간다. …새 셈을 찾는다.' — 설계서 S08 '변형은 상태 문장'(draft_events).
- why [2]: '…모았다. …맞추어졌다.' → '…모은다. …맞추어진다.' — 같은 까닭.
- why [0]: 그대로.

### 2.8 `event_brick_inward_appetite` — S11

- 조건: `CivHasTrait Brick.Mutation.InwardHunger` ∧ `CivTimeVectorAtLeast Isolated 1500` (Lottery, 1, 600, Severity 0, `Event.Civ.Economy`)
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

- why: 바꾼 칸 없음.

### 2.9 `event_brick_works_run` — S12

- 조건: `CivHasTrait Brick.Instinct.SwarmTech` ∧ `CivHasTrait Brick.Mutation.DustBand` ∧ `CivTimeVectorAtLeast Built 1500` (Lottery, 1, 600, Severity 0, `Event.Civ.Economy`)
- 효과: `AddResource Energy +25`, `AddResource Matter +25`, `ChangePopulation −0.02`

```json
{
  "LogTemplates": [
    "{Civ}의 기관들은 돌보는 손이 줄어든 뒤에도 돈다고 한다. 띠를 이룬 일꾼들이 서로를 고쳐 가며 일을 잇는다.",
    "{Civ}의 별 둘레의 띠가 사람 손 없이 에너지와 물질을 실어 왔다. 공방을 지키던 집들 가운데 빈 집이 늘었다.",
    "{Civ}의 일꾼 띠가 스스로를 고치며 창고를 채웠다. 그 곁에서 손으로 일하던 사람은 줄어 갔다."
  ]
}
```

- why [2]: '…그 곁에서 손을 놀리던 사람은 줄어 갔다.' → '…그 곁에서 손으로 일하던 사람은 줄어 갔다.' — TVM-12: 꾸밈말 없는 '손을 놀리던'은 흔히 '놀고 있던'으로 읽혀, '노는 이들이 줄었다'는 뜻이 되고 이 행의 `ChangePopulation −0.02`(일손이 기계로 바뀜)를 뒤집으며 헤더 §3.5가 좋은 끝에서 떼어 둔 '노는 날'의 결에 닿는다.
- why [0]·[1]: 그대로(결정 16 — 기계가 사람을 대신해 '이어 산다'고 말하지 않는다).

---

## 3. `ST_CA.csv` 줄 (새 키는 끝에만, `"Key","SourceString"`, LF)

```csv
"Settle.Block.Inward","안으로 접힌 백성은 더 넓히지 않는다."
"Shape.BuiltOrLinked","짓고 이은 살림"
"Shape.ConsumedOrDrifted","밖으로 뻗은 살림"
"Shape.Isolated","홀로 선 살림"
"Shape.Endured","눌림 속에 이어 간 살림"
"Shape.WovenSwarm","촘촘히 엮은 일손의 살림"
"Shape.StarHerd","머물지 않는 살림"
"Shape.SpreadBody","별 사이에서 걸러 사는 살림"
"Shape.FoldedSmall","작은 눈금 속으로 접힌 살림"
"Shape.ShutHouse","항구를 닫아건 살림"
"Shape.KeptLaw","칙령을 쉽게 바꾸지 않는 살림"
"Shape.LastHunger","별의 불을 끌어 쓰는 살림"
"Shape.Entwined","이웃과 뒤엉킨 살림"
```

`Settle.Block.Inward`는 aa09543에 이미 `ST_CA.csv:450`으로 있다. 리베이스 때 main의 끝으로 옮기고 글은 그대로 둔다. `Shape.*`는 `[B-7x3]`이 처음 드는 키라 그 뒤에 붙인다.

헤더 §3.2 초안에서 벗어난 꼴 낱말 여섯(개발자 확인 — §8 열린 점 2~4, 11):

| 키 | 헤더 §3.2 초안 | 이 판 | 근거 | 다른 안 |
|---|---|---|---|---|
| `ConsumedOrDrifted` | 넓히고 삼킨 살림 | 밖으로 뻗은 살림 | T-03(고친 꼴) | TVM-1 '쉬지 않고 내달린 살림'·'멈추지 않고 내달린 살림'; '넓혀 온 살림' |
| `SpreadBody` | 별들 사이에 번진 살림 | 별 사이에서 걸러 사는 살림 | TVM-8 | — |
| `FoldedSmall` | 티끌 속으로 접힌 살림 | 작은 눈금 속으로 접힌 살림 | T-09 | — |
| `ShutHouse` | 한 별에 닫아 건 살림 | 항구를 닫아건 살림 | F6 + K3(띄어쓰기) | TVM-9 '빗장을 건 살림' |
| `KeptLaw` | 법을 남긴 살림 | 칙령을 쉽게 바꾸지 않는 살림 | TVM-7(고친 꼴) + 최종 비평 F5 | 감사 판 '칙령을 지닌 채 사는 살림'(결정 18보다 더 약속해 버렸다) |
| `LastHunger` | 끝없이 삼키는 살림 | 별의 불을 끌어 쓰는 살림 | T-02 (K2와 부딪침 — §8) | 헤더 초안 그대로 |

- why `Settle.Block.Inward`: 그대로. 헤더 §2.5가 글을 정했고 '안으로 접힌'은 두 블록(`ClosedPort`·`FoldedSmall`)을 함께 부르는 헤더의 이름이다(K1을 버린 까닭).
- why `ConsumedOrDrifted`: '넓히고 삼킨' → '밖으로 뻗은' — T-03·TVM-1: 이 꼴은 다른 백성의 성계를 겨눈 함대만이 아니라 물려받은 전시 동원 칙령만 지닌 백성(다스리는 백성에게 흔하다 — 헤더 결정 5)과 정착 중인 백성에게도 간다. '삼킨'은 그들에게 사실이 아닌 판정이다(결정 13 ㉢ (d), 설계서 §4.4 '낱말은 사실만 말한다'). '밖으로 뻗은'은 한 일이 아니라 향한 쪽을 말해 세 출처 모두에 참이다. TVM-1의 '쉬지 않고 내달린'은 고르지 않았다 — '쉬'가 헤더 §3.5의 '쉼'과 이웃하고, '내달린'은 정착 중인 백성이 한 적 없는 빠름을 말한다. '뻗'은 `Chronicle.` 목록·remnant_words·desolation_words·shape_merit_words·crossroads_words 어디에도 없다(§8 검사). 결정 5의 다른 안 ㉯(공성만 센다)가 서면 낱말을 다시 본다.
- why `SpreadBody`: '별들 사이에 번진' → '별 사이에서 걸러 사는' — TVM-8: `ShapeOf`는 `Brick.Diet.DarkMatter`를 지닌 모든 백성(씨앗에서 온 AI 백성 포함)에게 이 낱말을 주는데 그들은 번진 적이 없다. 새 낱말은 지닌 이 모두에게 참이다. C07 ① 결과 줄 '번져 있었다'는 그 카드 한 번의 사실이라 그대로 둔다.
- why `FoldedSmall`: '티끌 속으로' → '작은 눈금 속으로' — T-09: 생애 연대기의 장 위에서 '티끌로 돌아감'(시 90:3)의 죽음·사라짐 그림이자 끝 이름으로 읽힌다(결정 13 ㉢ (d)).
- why `ShutHouse`: '한 별에 닫아 건' → '항구를 닫아건' — F6: `ShapeOf`는 `ShutHouse`를 `ClosedPort`나 `ShutSphere`를 지닌 백성에게 주는데(헤더 §2.7), `ShutSphere`는 C09 ①에서 `ClosedPort`를 지닌 백성에게만 오고 C09 ②는 `ShutSphere` 없는 백성에서만 `ClosedPort`를 걷으므로 둘 다 항구를 닫은 백성이다(최종 비평 F11이 까닭을 바로잡았다). 또 C04는 여러 성계를 지닌 백성에게도 올 수 있어 '한 별에'가 거짓이다(설계서 S01 — 한 성계 전제 없음). '닫아걸다'는 표준국어대사전의 한 낱말이라 붙여 쓴다(K3). TVM-9의 '빗장을 건 살림'도 참이지만 F6(중간)을 따랐다.
- why `KeptLaw`: '법을 남긴' → '칙령을 쉽게 바꾸지 않는' — TVM-7: '법을 남긴'은 C10 ①로 물러난 그 생애의 연대기 장에 놓여 그 다스리는 이의 유산·끝 이름으로 읽힌다(결정 13 ㉢ (a)·(c)). 감사 판의 '칙령을 지닌 채 사는'은 최종 비평 F5로 다시 고쳤다: 결정 18의 좁힘에서 `KeptLaw` 백성도 주장의 카운트다운에 더하는 칙령과 위기의 답인 칙령을 거두고, 판정받는 행이 법칙을 물을 때는 백성 범위 칙령을 모두 거두며(헤더 §2.4), C10 ①에는 칙령을 지녔다는 조건이 없어 칙령 없이 물러날 수도 있다 — '지닌 채'는 '그 백성의 지금 살림'(결정 13 ㉢ (d))으로 보일 때 거짓일 수 있다. '쉽게 바꾸지 않는'은 그 백성이 칙령을 다루는 버릇을 말할 뿐 어느 칙령이 남는지도, 칙령이 있다는 것도 약속하지 않아 모든 `KeptLaw` 백성에게 참이다. 현재이고 백성이 주어다. '거두'(soul_verbs)·'내리'(C10 ①의 '자리에서 내려왔다'의 메아리)·'견디'·'남은'·'남은 자'·desolation_words·binding_words가 없다(§8.4 검사). 설계 쪽 이름 '남긴 법'(결정 18·노트 §1)은 화면에 나오지 않으므로 그대로 둔다.
- why `LastHunger`: '끝없이 삼키는' → '별의 불을 끌어 쓰는' — T-02: '끝없이 삼키는'은 결코 배부르지 않은 스올·죽음의 화(禍) 그림이라 백성에게 씌우면 판정으로 읽히고(결정 13 ㉢ (d)), 블록의 사실(유한한 곱, 일곱 등급 안에서 멈추는 어두워짐 — K8·결정 8)과도 어긋난다. 새 낱말은 이 블록을 주는 선택 C02 ②와 `event_brick_ember_wake`의 낱말이다.
- 그대로 둔 여섯: `BuiltOrLinked`('짓고 이은' — 같은 'A고 B' 꼴이지만 두 일 모두 평화로워 거짓 주장이 없다, TVM-1), `Isolated`, `Endured`(리뷰 T7의 고친 꼴; '견디'·'남은'·'버틴' 없음), `WovenSwarm`, `StarHerd`, `Entwined`(좋은 끝으로 적지 않는다; '하나가 되' 없음).

---

## 4. 스캔 얼굴 (`DT_LogTemplates`, `[B-7x3]`)

### 4.1 `log_brick_scan_spread` — S15

- 행: `Category` `Log.Discovery`, `Context` 비움, `MinTierToReveal` 0 — `log_scan_wild`와 같은 자리. 코드(`UPlayerRoleSubsystem::ScanCurrentSystem`)가 무주 성계를 스캔해 새로 안 것이 있을 때, 그 성계의 레인 이웃에 `Brick.Form.LivingNebula`와 `Brick.Diet.DarkMatter`를 함께 지닌 백성의 정착된 성계가 있으면 `log_scan_wild` 대신 이 행을 고른다. 슬롯은 `{System}`(스캔한 무주 성계)뿐이다 — 그 백성의 이름은 앎의 문(`IsKnown`)을 지나지 않았다.
- 효과: 없음(표현 계층만).

```json
{
  "Templates": [
    "{System} 너머 이웃한 별 곁의 암흑 구간에 무엇인가 엷게 퍼져 있다고 한다. 가까이서 본 이들은 그것이 그 별 곁에만 있었다고 했다.",
    "당신은 {System}에서 이웃한 별 쪽을 오래 살폈다. 그 별 곁에만 무엇인가 엷게 번져 있다는 말이 떠돌았고, {System}의 하늘에는 그런 것이 없었다.",
    "{System}에서 바라본 이웃한 별 곁이 엷게 흐려져 있었다고들 한다. 그 흐림은 그 별 곁을 넘어 번지지 않았다고 했다."
  ]
}
```

- why [0]: 설계서 S15 초안의 '{System} 둘레의' → '{System} 너머 이웃한 별 곁의', '제 별 곁에만' → '그 별 곁에만' — draft_events: `{System}`은 무주 성계이므로 퍼진 것을 그 둘레에 두면 결정 18 리뷰 고침 ①을 줄 안에서 어긴다.
- why [1]: 새 변형(draft_events — `DT_LogTemplates.Templates`는 변형 셋 이상).
- why [2]: 새 변형; '흐려 있었다' → '흐려져 있었다' — K10: '흐리다'는 형용사라 결과 상태는 '흐려져 있다'.

---

## 5. 블록 태그 DevComment 열일곱 (`Config/DefaultGameplayTags.ini`, 통합 트리 `:170-186`을 바꿀 글; ASCII만)

```ini
+GameplayTagList=(Tag="Brick.Form.Society",DevComment="Form brick: a people living as an ordinary society of households - the start mark of the crossroads cards (arch_seeker carries it; the first crossroads takes it away with a branch choice that succeeds or with its second choice). Read by conditions only, never by the end shape (B7x header 2.4, 3.1). Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Form.LivingNebula",DevComment="Form brick: a people whose life is spread thin between the stars like a nebula - it still owns only the systems it settled, and no unowned system is drawn as its own (B7x decision 18, S15). Read by the end shape (SpreadBody), the scan face log_brick_scan_spread together with Brick.Diet.DarkMatter (S15) and conditions (B7x). Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Form.ShutSphere",DevComment="Form brick: a people that shut its star inside a sphere and turns its heat to reckoning - the shell closed around its own star; not the fallen sphere (InSphere/FallenSphereTicks) nor Trait.Faith.BookShut. Read by the end shape (ShutHouse), DT_Rules.BrickStellarCrisisSpawnMultiplier on its settled systems and conditions (B7x). Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Form.KeptLaw",DevComment="Form brick: a people whose held edicts outlive an empty throne - with no one ruling it, its regent and AI keep its people-scope edicts (CosmicEdicts::AIRevokesOf), except an edict that adds to the claim's countdown or answers a crisis, and all of them while its row asks for galaxy laws (CosmicEdicts::AsksForLawsOf): those fall to the rule every AI uses (B7x decision 18, K9). It keeps edicts; it never becomes a law, and it is never the holy order (K6). Read by AIRevokesOf, the end shape (KeptLaw) and conditions (B7x). Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Diet.DarkMatter",DevComment="Diet brick: feeds on dark matter - DT_Rules.BrickDarkMatterEnergyPerTick flat Energy a tick, tied to no star class or abundance (B7x header 2.4). Also read by the end shape (SpreadBody), the scan face log_brick_scan_spread together with Brick.Form.LivingNebula (S15) and conditions. Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Diet.StarEater",DevComment="Diet brick: draws its star's fire into its furnaces - DT_Rules.BrickStarEaterEnergyMultiplier on its Energy, BrickStarEaterRelationTarget as a term of the drift target of every view of it, BrickStellarCrisisSpawnMultiplier on its settled systems, BrickStarEaterDimPeriodTicks (0 = off, B7x decision 8; never past the darkest of the seven classes, never a destroyed star) (B7x header 2.4, R10). Also read by the end shape (LastHunger), the AI mutation step (InwardHunger) and conditions. Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Diet.IdleFeed",DevComment="Diet brick: feeds on idle ticks - DT_Rules.BrickIdleInsightPerTick Insight on a tick with no hazard, project or siege on its settled systems (CosmicBricks::IsIdleOf, B7x header 2.4). Never named silence or rest on screen and kept apart from Revelation.Quiet (B7x decision 3); idle days resemble a fallen doctrine and are never told as a good end (header 3.5). Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Diet.DistanceTension",DevComment="Diet brick: feeds on its own span - DT_Rules.BrickSpanEnergyPerLane Energy per lane of the widest lane distance between two of its settled systems (lanes, never coordinates; B7x header 2.4). Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Diet.CrisisParasite",DevComment="Diet brick: lives off its neighbours' crises, never the claim, the extinction or the supernova precursor (K9) - DT_Rules.BrickParasiteCountdownBonusTicks on a hazard newly placed on a bordering people's system, once a hazard however many parasites border it (CosmicBricks::ParasiteBonusBoundOf, counted in the theodicy bound of Data.Integrity), and BrickParasiteInsightPerHazard for each such hazard standing on its neighbours' settled systems (B7x header 2.4). Comes from the seed only. Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Space.Nomad",DevComment="Space brick: a wandering people - DT_Rules.BrickNomadSettleProgressMultiplier on its settlements (B7x header 2.4); it keeps a capital and its systems like any people. Also read by the end shape (StarHerd), the AI mutation step (StarTow) and conditions. Comes from the seed, to the ruled people by petition_cross_drifted, and to the owner of the wanderer's system by enc_cross_drift_fleet. Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Space.ClosedPort",DevComment="Space brick: a people that closed its ports and turned inward, however many systems it holds - starts no further settlement (settle reason Inward), DT_Rules.BrickInwardResearchMultiplier; no immunity - its systems keep only the protection every people has (K8; B7x header 2.4, 2.5). Inward here is not Brick.Mutation.InwardHunger. Also read by the end shape (ShutHouse) and conditions. Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Space.FoldedSmall",DevComment="Space brick: a people folded into the very small - starts no further settlement (settle reason Inward), DT_Rules.BrickInwardResearchMultiplier, BrickInwardHazardSpawnMultiplier on its systems (never 0, never the claim, the extinction or the supernova precursor - K8, K9); the map still shows it as the owner (K1) (B7x header 2.4, 2.5). Also read by the end shape (FoldedSmall) and conditions. Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Instinct.SwarmTech",DevComment="Instinct brick: a swarm that learns together - DT_Rules.BrickSwarmResearchMultiplier (B7x header 2.4). Also read by the end shape (WovenSwarm), the AI mutation step (DustBand) and conditions. Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Instinct.HiveMerge",DevComment="Instinct brick: a people that draws its neighbours in - DT_Rules.BrickMergeRelationTarget as a term of the drift target of every view of it (B7x header 2.4). The same grain as the counterfeit of Doctrine.Assimilation: never celebrated, never told as a good end or as becoming one (header 3.5). Also read by the end shape (Entwined) and conditions. Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Mutation.StarTow",DevComment="Mutation brick: the second mark of a wandering people - added by the AI mutation step to a Brick.Space.Nomad people whose Drifted vector reaches DT_Rules.BrickMutationThresholdTicks, or to the ruled people by petition_cross_star_line (B7x header 2.6). Slice ga opens the word only: towing a star waits for the lane window (slice na), and it never relights a dead star. Takes no shape name - the end shape reads its parent (B7x decision 3). Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Mutation.DustBand",DevComment="Mutation brick: the second mark of a swarm - added by the AI mutation step to a Brick.Instinct.SwarmTech people whose Built and Linked vectors reach DT_Rules.BrickMutationThresholdTicks, or to the ruled people by petition_cross_dust_band (B7x header 2.6). Read by conditions (event_brick_works_run); it gives no rung and no lane. Takes no shape name - the end shape reads its parent (B7x decision 3). Saved name, never renamed.")
+GameplayTagList=(Tag="Brick.Mutation.InwardHunger",DevComment="Mutation brick: the second mark of a star eater - added only by the AI mutation step to a Brick.Diet.StarEater people whose Consumed vector reaches DT_Rules.BrickMutationThresholdTicks (B7x header 2.6, S11). Read by conditions (event_brick_inward_appetite). A different fact from CosmicBricks::IsInward and the settle reason Inward (ClosedPort/FoldedSmall), which never read it. Takes no shape name - the end shape reads its parent (B7x decision 3). Saved name, never renamed.")
```

- why (draft_events의 고친 글에서 이 판이 바꾼 셋):
  - `LivingNebula`: '…drawn as its (B7x…' → '…drawn as its own (B7x…' — K11: 매달린 소유격.
  - `ClosedPort`: 'a people folded inward into one sealed house - …; no immunity - its system keeps…' → 'a people that closed its ports and turned inward, however many systems it holds - …; no immunity - its systems keep…' — F8·F6·K11: 'one sealed house'는 한 성계를 전제하고(S01이 막는다) 'folded'는 `FoldedSmall`의 그림이다. 'its system' → 'its systems'는 같은 까닭의 맞춤이다. K11의 다른 안('…builds inward in one sealed house')은 'one sealed house'를 남겨 F8·F6에 지므로 쓰지 않았다.
  - `CrisisParasite`: 제외('never the claim, the extinction or the supernova precursor')를 두 항 앞으로 옮기고 'each hazard' → 'each such hazard' — F9: 헤더 §2.4는 두 항 모두에서 그것들을 뺀다.
- why (최종 비평이 바꾼 둘):
  - `Society`: 'a people of mortal bodies living as an ordinary society - … with its first or its second choice' → 'a people living as an ordinary society of households - … with a branch choice that succeeds or with its second choice' — F9: 'mortal bodies'는 다른 꼴들을 죽지 않는 몸처럼 맞세워 결정 16과 '불멸'·Undying 잎의 금지(노트 §5.4 결정 16의 받는 조건)에 어긋난다. 또 첫 갈림의 ①은 성공할 때만 `Society`를 걷는다.
  - `Nomad`: 끝을 'Comes from the seed, to the ruled people by petition_cross_drifted, and to the owner of the wanderer's system by enc_cross_drift_fleet.'로 — F10: C03 ① 성공도 `Nomad`를 주고 씨앗도 준다(헤더 결정 23, aa09543 `DT_Rules.json`의 `BrickSeedTags`); `StarTow`·`DustBand`의 글처럼 출처를 모두 적는다.
- 나머지 열둘은 draft_events의 글 그대로(처음 글에서 바꾼 까닭은 그 문서의 why).

---

## 6. 데이터 효과의 바뀜 (`[B-7x5c]`가 옮긴다)

값(Cost)·판정·사건 행 아홉의 효과는 바꾸지 않는다. 갈림 카드의 효과는 아래 표의 칸만 바뀐다(나머지 칸은 aa09543 그대로). 잎 이름은 `rename_leaves.py`(결정 3)가 바꾼다.

### 6.1 닫지 않는 결과의 규칙 (결정 11 (나); 최종 비평 F1·F13)

닫지 않는 ②는 값 하나와 작은 한 번의 효과(안정도나 자원)만 지닌다. 되풀이될 때마다 저장되는 값에 쌓이는 것은 모두 이득으로 세어 두지 않는다 — 목소리 기울기 `ShiftEthics Spiritualism`, 둘째 윤리 축(Openness 등), 우러름(`ChangeReverence`), 높아짐(`PlayerStatDelta Exaltation`), `ChangePopulation`, 특성. 기울기는 ±1에 묶이지만 저장되고 에너지·통찰 수입의 덤을 읽는다(main `Sim/GalaxySimulationSubsystem.cpp:1771`, `:1773`) — 결정 11 (나)의 구속 글이 윤리 이동을 쌓이는 이득으로 이름하므로 `petition_road` ②가 기울기를 지닌다는 것은 예외가 되지 않는다(F1). **기울기는 카드를 닫는 결과에만 남는다**: 첫 갈림 ① 성공과 ②, 둘째 갈림 ① 성공(주는 태그로 닫힌다), C04 ①②·C09 ①·C10 ①·C11 ①(판정 없이 닫는다), C09 ② 성공(부모를 걷어 닫는다). 닫지 않는 것 — 둘째 갈림 여섯의 ②, C09 ②의 실패, 판정 ①의 실패(첫 갈림 셋 C01~C03과 둘째 갈림 넷 C05~C08 — 실패하면 `Society`가 남거나 태그가 오지 않아 카드가 다시 온다) — 에는 `ShiftEthics`를 두지 않는다(F13: 같은 원리; 실패마다 값 전부를 치른다는 것은 ②에도 값이 있으므로 예외의 까닭이 되지 못한다). 첫 갈림 ① 실패 셋은 결정 11 (나)의 글자(둘째 갈림의 ②) 밖이라 §8.3 열린 점 6으로 리드에게 둔다. 방랑자의 C12 ②도 같은 원리로 비운다(F4 — 우러름은 구속 글이 이름한 쌓이는 이득이고, 여기서는 즉위의 문을 먹인다).

| 행 | 선택 | 전 (aa09543) | 후 | 근거 |
|---|---|---|---|---|
| `petition_cross_built` (C01) | ① OnFailure | `ChangeStability −0.06`, 기울기 −0.05 | `ChangeStability −0.06` | 닫지 않는 실패의 기울기(F13) |
| `petition_cross_consumed` (C02) | ① OnFailure | `ChangeStability −0.08`, 기울기 +0.05 | `ChangeStability −0.08` | 같음(F13) |
| `petition_cross_drifted` (C03) | ① OnFailure | `ChangeStability −0.1`, 기울기 +0.05 | `ChangeStability −0.1` | 같음(F13) |
| `petition_cross_dust_band` (C05) | ① OnFailure | `ChangeStability −0.05`, 기울기 −0.05 | `ChangeStability −0.05` | 같음(F13) |
| `petition_cross_dust_band` (C05) | ② OnSuccess | `ChangePopulation +0.03`, 기울기 +0.05 | `ChangeStability +0.05` | `ChangePopulation`은 상한 없이 곱해져(`Events/CosmicEventSubsystem.cpp:893-900`) 되풀이마다 쌓이고 수입을 곱한다; `ChangeStability +0.05`는 작은 한 번의 효과(draft_second); 기울기는 F1 |
| `petition_cross_star_line` (C06) | ① OnFailure | `ChangeStability −0.06`, 기울기 −0.05 | `ChangeStability −0.06` | F13 |
| `petition_cross_star_line` (C06) | ② OnSuccess | `ChangeStability +0.05`, 기울기 +0.05 | `ChangeStability +0.05` | F1 |
| `petition_cross_between_stars` (C07) | ① OnFailure | `ChangeStability −0.06`, 기울기 +0.05 | `ChangeStability −0.06` | F13 |
| `petition_cross_between_stars` (C07) | ② OnSuccess | `ChangeStability +0.04`, `ShiftEthics Openness −0.05`, 기울기 −0.05 | `ChangeStability +0.04` | 둘째 윤리 축(F3)과 기울기(F1) |
| `petition_cross_into_dust` (C08) | ① OnFailure | `ChangeStability −0.06`, 기울기 −0.05 | `ChangeStability −0.06` | F13 |
| `petition_cross_into_dust` (C08) | ② OnSuccess | `ChangeStability +0.05`, 기울기 +0.05 | `ChangeStability +0.05` | F1 |
| `petition_cross_wrapped_star` (C09) | ② OnFailure (되돌림) | `ChangeStability −0.1`, 기울기 +0.05 | `ChangeStability −0.1` | 실패는 닫지 않는다(F1); 성공(`RemoveTrait Brick.Space.ClosedPort`, `ChangeStability −0.05`, 기울기 +0.05)은 부모를 걷어 닫으므로 그대로 |
| `petition_cross_law_stays` (C10) | ② OnSuccess | `ChangeStability +0.05`, `ChangeReverence +0.1`, `PlayerStatDelta Exaltation +1`, 기울기 +0.05 | `ChangeStability +0.05` | Exaltation은 결정 11 (나)가 이름했고, Reverence는 저장되어 되풀이마다 쌓이며(`Events/CosmicEventSubsystem.cpp:829-831`), 기울기는 노트 §5.4 데2와 F1 |
| `petition_cross_seam` (C11) | ② OnSuccess | `ShiftEthics Openness +0.05`, `ChangeStability −0.02`, 기울기 +0.05 | `AddResource Insight 120`, `ChangeStability −0.02` | Openness는 둘째 윤리 축(F3), 기울기는 F1; Insight 120은 TVM-6의 작은 한 번의 효과(값은 헤더 §5 ⑧ `-Crossroads` 측정이 정한다) |
| `enc_cross_drift_fleet` (C12) | ② OnSuccess | `ChangeReverence +0.05` | `[]` | 닫지 않는 `Standing` 카드의 공짜 Body 4 판정이 우러름을 쌓아 즉위의 문을 먹였다(F4); 결과 줄도 바꿨다(§1.12) |

### 6.2 바꿀 효과의 정확한 JSON (칸 통째)

```json
{
  "petition_cross_built.Choices[0].OnFailure": [
    {"Type": "ChangeStability", "Tag": "", "Key": "None", "Value": -0.06, "DurationTicks": 0}
  ],
  "petition_cross_consumed.Choices[0].OnFailure": [
    {"Type": "ChangeStability", "Tag": "", "Key": "None", "Value": -0.08, "DurationTicks": 0}
  ],
  "petition_cross_drifted.Choices[0].OnFailure": [
    {"Type": "ChangeStability", "Tag": "", "Key": "None", "Value": -0.1, "DurationTicks": 0}
  ],
  "petition_cross_dust_band.Choices[0].OnFailure": [
    {"Type": "ChangeStability", "Tag": "", "Key": "None", "Value": -0.05, "DurationTicks": 0}
  ],
  "petition_cross_dust_band.Choices[1].OnSuccess": [
    {"Type": "ChangeStability", "Tag": "", "Key": "None", "Value": 0.05, "DurationTicks": 0}
  ],
  "petition_cross_star_line.Choices[0].OnFailure": [
    {"Type": "ChangeStability", "Tag": "", "Key": "None", "Value": -0.06, "DurationTicks": 0}
  ],
  "petition_cross_star_line.Choices[1].OnSuccess": [
    {"Type": "ChangeStability", "Tag": "", "Key": "None", "Value": 0.05, "DurationTicks": 0}
  ],
  "petition_cross_between_stars.Choices[0].OnFailure": [
    {"Type": "ChangeStability", "Tag": "", "Key": "None", "Value": -0.06, "DurationTicks": 0}
  ],
  "petition_cross_between_stars.Choices[1].OnSuccess": [
    {"Type": "ChangeStability", "Tag": "", "Key": "None", "Value": 0.04, "DurationTicks": 0}
  ],
  "petition_cross_into_dust.Choices[0].OnFailure": [
    {"Type": "ChangeStability", "Tag": "", "Key": "None", "Value": -0.06, "DurationTicks": 0}
  ],
  "petition_cross_into_dust.Choices[1].OnSuccess": [
    {"Type": "ChangeStability", "Tag": "", "Key": "None", "Value": 0.05, "DurationTicks": 0}
  ],
  "petition_cross_wrapped_star.Choices[1].OnFailure": [
    {"Type": "ChangeStability", "Tag": "", "Key": "None", "Value": -0.1, "DurationTicks": 0}
  ],
  "petition_cross_law_stays.Choices[1].OnSuccess": [
    {"Type": "ChangeStability", "Tag": "", "Key": "None", "Value": 0.05, "DurationTicks": 0}
  ],
  "petition_cross_seam.Choices[1].OnSuccess": [
    {"Type": "AddResource", "Tag": "", "Key": "Insight", "Value": 120.0, "DurationTicks": 0},
    {"Type": "ChangeStability", "Tag": "", "Key": "None", "Value": -0.02, "DurationTicks": 0}
  ],
  "enc_cross_drift_fleet.Choices[1].OnSuccess": []
}
```

### 6.3 같은 `[B-7x5c]` 커밋에서 맞출 문서 (지금은 고치지 않는다 — 이 일은 스크래치에만 쓴다)

- 노트 §5.4 데1과 설계서 §4.3 공통: 설계서의 '목소리가 청한 선택은 관례대로 `ShiftEthics Spiritualism`을 옮긴다(의회 −0.05, 장로들 +0.05)' → '목소리가 청한 선택은 카드를 닫는 결과에서만 `ShiftEthics Spiritualism`을 옮긴다(의회 −0.05, 장로들 +0.05)'; 그리고 한 문장 — '카드를 닫지 않는 ②와 되돌림 ②의 실패, 판정 ①의 실패는 `ShiftEthics`를 지니지 않는다; 목소리 기울기는 카드를 닫는 선택에만 남는다. 둘째 윤리 축·우러름·높아짐·인구·특성도 닫지 않는 결과에 두지 않는다.' (F1·F13·F3) 같은 문장을 노트 §5.4 데1의 `why`에도 맞춘다('its choice 2 carries no repeatable gain' 뒤에 'and a tilt only where the card closes').
- 설계서 §4.3 C01~C03·C05~C08 ①의 실패 줄, C05~C11 ②의 효과 줄과 C09 ② 실패 줄, C11 ② 결과 줄, C10 제목·① 결과 줄·② 결과 줄, C12 '③ 뒤'·①·② 결과 줄·Body·제목을 데이터와 맞춘다(V2·TVM-3·TVM-4·TVM-6·노트 데1·F1·F4·F6·F8·F13).
- main `Config/DefaultGameplayTags.ini`의 `Trait.Society.Divided` DevComment 'Choices move Spiritualism +-0.05'는 main의 청원 관례라 그대로 둔다 — 갈림 카드의 예외는 설계서 §4.3 공통이 적는다.
- 노트 §5.4 데2와 설계서 §4.3 C10: '거두지 않은 칙령' → '손대지 않은 칙령', 옛 제목에 붙은 `epitaph_leaves_out` 후보 표지를 지운다(T-01·TVM-2). 헤더 §11에 이 벗어남을 적는다: `words.soul_verbs`의 '거두', DT_Encounters 규칙 0(`row_prefix petition_`, Title `lacks_words soul_verbs`), `cosmic_data_schema.py:1395`의 부분 문자열 비교.
- 꼴 낱말 여섯(§3 표)을 헤더 §3.2와 설계서 §4.4 표에서 바꾼다. 같이 고칠 자리: 설계서 §4.3 갈림의 나무에서 `Consumed` 줄(156행 — '별 사이에서 걸러 사는 살림' / '별의 불을 끌어 쓰는 살림')·마지막 줄(`ConsumedOrDrifted`)·`FoldedSmall`·`KeptLaw`(159행)·`ShutHouse`(158행 근처)의 줄; 설계서 §4.4 맺음 글(311행 — '뒤엉킨 살림과 별의 불을 끌어 쓰는 살림은 좋은 끝으로 적지 않는다'); 헤더 433행 '삼키는 살림의 문' → '별의 불을 끌어 쓰는 살림의 문'; 노트 90행 '삼키는 살림의 둘째 표지' → '별의 불을 끌어 쓰는 살림의 둘째 표지' (T-02 (a)~(f)). 설계 이름 `LastHunger`·시간 셈 이름 '삼킨 시간'은 화면에 나오지 않으므로 그대로.
- 헤더 §2.4 48행: 한 성계를 전제하지 않게 — 예 '정착된 성계가 하나 남은 닫아건 백성은 새 면역 없이 이미 그렇다'(F6). draft DevComment의 'one sealed house'는 §5에서 이미 고쳤다.

---

## 7. 스키마 바뀜 (`Tools/data/schema.json` — 행과 같은 커밋)

### 7.1 `words`에 더할 목록 둘

```json
{
  "crossroads_words": ["침묵", "울타리", "언약", "방주", "목자", "순례", "성소", "불멸", "하나가 되"],
  "shape_merit_words": ["견디", "남은"]
}
```

선택(리드): `shape_merit_words`에 '남긴'을 더하면 '남은'의 타동 짝도 `Shape.` 키에서 막힌다(TVM-7의 덧붙임). 이 판의 꼴 낱말 열둘은 '남긴'을 갖지 않으므로 더해도 깨지는 줄이 없다. 개발자가 정한 목록(['견디','남은'])을 넓히므로 기본은 더하지 않는다.

### 7.2 `string_table.checked_prefixes`에 `Shape.` (`Chronicle.*` 항목 뒤)

```json
{
  "Shape.": {
    "forbidden": ["verdict", "soul_verbs", "rescue_words", "afterlife_words", "book_of_life_words", "called_away_words", "holy_passive", "rung_grasp_words", "title_words", "binding_words", "remnant_words", "desolation_words", "shape_merit_words"],
    "no_digits": true
  }
}
```

### 7.3 표의 규칙 (F1 — 검사기의 `when`은 AND뿐이고 OR 꼴이 없다)

`DT_Encounters` — 규칙 하나. 갈림 카드 열둘은 모두 `CivTimeVectorAtLeast`를 지닌다.

```json
{
  "why": "a crossroads card tells what a people did and where it then stands: its words name no rest or silence, no grasp of a rung, no title, and none of the fenced words of the crossroads - silence, fence, covenant, ark, shepherd, pilgrimage, sanctuary, the deathless, becoming one (B7x header 3.5, notes 5.4)",
  "when": [{"col": "Conditions", "has_type": "CivTimeVectorAtLeast"}],
  "then": [
    {"col": "Title", "lacks_words": "silence_words"},
    {"col": "Title", "lacks_words": "rung_grasp_words"},
    {"col": "Title", "lacks_words": "title_words"},
    {"col": "Title", "lacks_words": "crossroads_words"},
    {"col": "Body", "lacks_words": "silence_words"},
    {"col": "Body", "lacks_words": "rung_grasp_words"},
    {"col": "Body", "lacks_words": "title_words"},
    {"col": "Body", "lacks_words": "crossroads_words"},
    {"col": "VoiceBodies", "lacks_words": "silence_words"},
    {"col": "VoiceBodies", "lacks_words": "rung_grasp_words"},
    {"col": "VoiceBodies", "lacks_words": "title_words"},
    {"col": "VoiceBodies", "lacks_words": "crossroads_words"},
    {"col": "Choices.Label", "lacks_words": "silence_words"},
    {"col": "Choices.Label", "lacks_words": "rung_grasp_words"},
    {"col": "Choices.Label", "lacks_words": "title_words"},
    {"col": "Choices.Label", "lacks_words": "crossroads_words"},
    {"col": "Choices.SuccessText", "lacks_words": "silence_words"},
    {"col": "Choices.SuccessText", "lacks_words": "rung_grasp_words"},
    {"col": "Choices.SuccessText", "lacks_words": "title_words"},
    {"col": "Choices.SuccessText", "lacks_words": "crossroads_words"},
    {"col": "Choices.FailureText", "lacks_words": "silence_words"},
    {"col": "Choices.FailureText", "lacks_words": "rung_grasp_words"},
    {"col": "Choices.FailureText", "lacks_words": "title_words"},
    {"col": "Choices.FailureText", "lacks_words": "crossroads_words"}
  ]
}
```

`DT_Events` — 규칙 다섯(같은 `then`). `rung_grasp_words`·`title_words`는 main에서 이미 `LogTemplates` 칸 전체에 걸려 있다(B-6b5). 그 앞의 바탕이면 이 규칙들에 더한다. 조건 시험만으로는 `event_brick_shut_star`·`event_brick_drift_plenty`·`event_brick_ember_wake`(셋 다 `CivHasTrait`뿐)에 닿지 않아 행 시험을 쓴다.

```json
[
  {
    "why": "a line of a people's lived way tells no rest or silence and none of the fenced words of the crossroads (B7x header 3.5, notes 5.4)",
    "when": [{"col": "Conditions", "has_type": "CivTimeVectorAtLeast"}],
    "then": [{"col": "LogTemplates", "lacks_words": "silence_words"}, {"col": "LogTemplates", "lacks_words": "crossroads_words"}]
  },
  {
    "why": "a line about a bordering people's brick tells no rest or silence and none of the fenced words of the crossroads (B7x header 3.5, notes 5.4)",
    "when": [{"col": "Conditions", "has_type": "BorderingCivHasTrait"}],
    "then": [{"col": "LogTemplates", "lacks_words": "silence_words"}, {"col": "LogTemplates", "lacks_words": "crossroads_words"}]
  },
  {
    "why": "the brick line of the shut sphere waits on traits alone, so it is named: no rest or silence, none of the fenced words of the crossroads (B7x header 3.5)",
    "when": [{"row": "event_brick_shut_star"}],
    "then": [{"col": "LogTemplates", "lacks_words": "silence_words"}, {"col": "LogTemplates", "lacks_words": "crossroads_words"}]
  },
  {
    "why": "the brick line of the wandering swarm's plenty waits on traits alone, so it is named: no rest or silence, none of the fenced words of the crossroads (B7x header 3.5)",
    "when": [{"row": "event_brick_drift_plenty"}],
    "then": [{"col": "LogTemplates", "lacks_words": "silence_words"}, {"col": "LogTemplates", "lacks_words": "crossroads_words"}]
  },
  {
    "why": "the brick line of the star eater's trembling star waits on a trait and a hazard, so it is named: no rest or silence, none of the fenced words of the crossroads (B7x header 3.5)",
    "when": [{"row": "event_brick_ember_wake"}],
    "then": [{"col": "LogTemplates", "lacks_words": "silence_words"}, {"col": "LogTemplates", "lacks_words": "crossroads_words"}]
  }
]
```

`DT_LogTemplates` — 규칙 하나.

```json
{
  "why": "the scan face of a people spread between the stars tells no rest or silence and none of the fenced words of the crossroads, and is named because a log row has no conditions (B7x header 3.5, S15)",
  "when": [{"row": "log_brick_scan_spread"}],
  "then": [{"col": "Templates", "lacks_words": "silence_words"}, {"col": "Templates", "lacks_words": "crossroads_words"}]
}
```

- 행 이름 규칙은 행과 같은 커밋에 든다: 검사기는 표가 요구하지도 지니지도 않은 행을 부르는 행 시험을 거절한다(F1). 그래서 커밋을 이렇게 나눈다(최종 비평 F12):
  - `[B-7x4]`: `words.crossroads_words` 목록(과 `shape_merit_words`·`Shape.` 접두어 — 노트 데3 `Shape.`의 몫이 `[B-7x3]`에 먼저 들지 않았으면), 조건으로 거는 규칙 셋(`DT_Encounters`의 `has_type CivTimeVectorAtLeast`, `DT_Events`의 `CivTimeVectorAtLeast`와 `BorderingCivHasTrait`)과 그 깨짐 — 행이 아직 없으므로 깨짐은 시험 행(`petition_test`·`event_test` 꼴)에 둔다.
  - `[B-7x5c]`: 이름으로 거는 `DT_Events` 규칙 셋(`event_brick_shut_star`·`_drift_plenty`·`_ember_wake`) — 그 행들이 드는 커밋.
  - `log_brick_scan_spread`의 행 규칙: `[B-7x4]`가 `[B-7x3]` 뒤에 오면 `[B-7x4]`에, 앞에 오면 그 행이 드는 `[B-7x3]`에.
- 노트 §5.4 데3에 두 줄: '조건 시험만으로는 네 행(`event_brick_shut_star`·`_drift_plenty`·`_ember_wake`·`log_brick_scan_spread`)에 닿지 않고, 검사기에는 OR 꼴이 없다.' 그리고 '`[B-7x4]`는 목록과 조건 규칙 셋과 시험 행의 깨짐을, `[B-7x5c]`는 이름 규칙 셋을, `log_brick_scan_spread`의 규칙은 그 행이 든 뒤의 첫 커밋(`[B-7x3]`이나 `[B-7x4]`)을 맡는다.'
- `Tools/test_data_schema.py`(노트 시8 '목록마다 깨짐 하나'): 새로 거는 목록마다 깨짐 하나, 그리고 깨짐마다 **새 규칙으로만** 깨져야 한다(main에서 이미 그 칸에 걸린 목록의 낱말을 쓰면 새 규칙이 없어도 깨져 아무것도 증명하지 못한다 — 최종 비평 F2·F3). 시험 행은 `CivTimeVectorAtLeast`를 지닌다.
  - 갈림 카드(`petition_test` 꼴 행): Title에 '언약'(`crossroads_words`), VoiceBodies에 '안식'(`silence_words`), Choices.Label에 '올라섰'(`rung_grasp_words`), SuccessText에 '면류관'(`title_words`) — main의 `DT_Encounters`는 이 칸들에 `verdict`(열)와 `holy_words`·`soul_verbs`·`rescue_words`(규칙 0)만 건다.
  - 사건 행(`event_brick_*` 꼴, `CivPressureEnduredTicksReached` 없이): LogTemplates에 '순례'(`crossroads_words` — 예 '{Civ}의 배들이 순례를 떠났다. …')와 '고요'(`silence_words` — main에서는 규칙 19의 `CivPressureEnduredTicksReached` 행에만 걸린다). '방주'는 쓰지 않는다 — `DT_Events.LogTemplates`의 열 금지 목록이 `ark_words`(['방주'])로 이미 막는다(F2). '목자'도 쓰지 않는다(`revelation_mimicry`에 '선한 목자'가 있다).
  - 스캔 얼굴(`log_brick_scan_spread`): Templates에 '성소'(`crossroads_words`)와 '잠잠'(`silence_words`) — `DT_LogTemplates.Templates`의 열 금지 목록에 둘 다 없다.
  - `rung_grasp_words`·`title_words`는 사건 행과 스캔 얼굴에서는 main이 이미 열 전체에 건다(B-6b5) — 새 깨짐이 없다.
  - `shape_merit_words` — `Shape.` 키에 '남은'; 그리고 `Chronicle.` 핀(test_data_schema.py:515)처럼 `Shape.` 금지 목록의 핀 하나.

---

## 8. 기록

### 8.1 렌즈별 지적 수

| 렌즈 | 지적 | 남김 | 버림 | 버린 것 |
|---|---|---|---|---|
| 신학 (T) | 11 | 9 | 2 | T-04, T-06 |
| 목소리 (V) | 7 | 6 | 1 | V1 |
| 문법 (K) | 11 | 10 | 1 | K1 |
| 기계 (TVM) | 12 | 11 | 1 | TVM-5 |
| 일관성 (F) | 10 | 7 | 3 | F4, F5, F7 |
| 합 | 51 | 43 | 8 | |

남긴 43건: 그대로 적용 30, 둘씩 합쳐 적용 8(네 쌍 — V6+K4 → C04 ②; T-08+K7 → C06 목소리; T-01+TVM-2 → C10 제목; F6+K3 → `ShutHouse`), 고쳐 적용 1(V4 — '땅 위의 집들이' → '그 집들이', 겹침을 피함), 부분 적용 1(K11 — `LivingNebula`는 적용, `ClosedPort`는 F8·F6에 짐), 다른 지적에 져서 적용하지 않음 3(K2 → T-02, TVM-1 → T-03, TVM-9 → F6). TVM-10·F3은 효과를 바꾸지 않는 결정이라 §6.1에 규칙으로 적고 문서 문장은 §6.3의 `[B-7x5c]` 몫으로 남겼다(기울기를 남긴 TVM-10의 결론은 최종 비평 F1·F13이 뒤집었다 — §8.5). T-01·T-02·F6의 문서 고침도 §6.3에 남겼다(이 일은 저장소를 고치지 않는다).

### 8.2 부딪친 지적과 고른 쪽

- `Shape.LastHunger` — T-02(새 낱말) 대 K2(개발자가 고를 때까지 헤더 초안). T-02를 따랐다: 이 판은 헤더 §3.2 초안 여섯을 같은 방식으로 고쳤고(§3 표), K2의 뜻(세 초안이 한 낱말을 쓰고 개발자가 제안을 본다)은 C02 메모를 새 낱말로 맞추고 §3 표에 헤더 초안을 '다른 안'으로 두어 채웠다. 개발자가 초안을 고르면 csv 줄 하나와 §1.2 메모 한 줄만 되돌린다.
- `Shape.ConsumedOrDrifted` — T-03 '밖으로 뻗은' 대 TVM-1 '쉬지 않고 내달린'. T-03을 골랐다(까닭은 §3의 why).
- `Shape.ShutHouse` — K3(띄어쓰기만) · TVM-9 '빗장을 건' · F6 '항구를 닫아 건'. F6에 K3의 띄어쓰기를 얹었다.
- C04 ② SuccessText — V6('셈'을 넣음) 대 K4('셈으로 돌았다'를 뺌). 두 뜻을 함께 지녔다.

### 8.3 리드에게 남긴 것

1. C10 제목 '손대지 않은 칙령'은 개발자 답 '거두지 않은 칙령'에서 벗어난다(soul_verbs) — 헤더 §11에 적고 확인을 받는다.
2. 꼴 낱말 여섯이 헤더 §3.2 초안과 다르다(§3 표) — 개발자 확인. 특히 `LastHunger`(K2와 부딪침)와 `ConsumedOrDrifted`(다른 안 셋).
3. `ShutHouse` '항구를 닫아건 살림' 대 '빗장을 건 살림'.
4. `KeptLaw` '칙령을 쉽게 바꾸지 않는 살림'(최종 비평 F5 — 감사 판 '칙령을 지닌 채 사는 살림'은 결정 18보다 더 약속했다) — 설계 쪽 이름 '남긴 법'은 그대로 둔다.
5. C12 제목 '조선소의 갈림' 대 '집을 싣는 배'(카드 종류 낱말 '갈림'을 화면에서 빼려면).
6. (옛 6 '목소리 기울기의 관례'는 F1로 닫혔다 — 닫지 않는 결과에서 기울기를 뺐다, §6.1.) 새 6: 첫 갈림 ① 실패 셋(C01~C03)의 기울기를 뺀 것은 결정 11 (나)의 글자(둘째 갈림의 ②) 밖이다(F13). 리드가 받지 않으면 세 칸을 aa09543으로 되돌리고, 헤더 §11과 §6.1에 '판정 ①의 실패 기울기는 실패마다 값 전부를 치르므로 받아들인다'를 적는다(이 경우 C05~C08 ① 실패 넷도 같은 까닭으로 함께 되돌리는 것이 고른 꼴이다).
7. (옛 7은 F4로 닫혔다 — C12 ② 성공을 비우고 결과 줄을 바꿨다, §1.12·§6.) 리드가 우러름을 남기려면 헤더 §11에 결정 11 (나)에서 벗어남으로 적고 헤더 §5 판단 ③′·노트 데5와 함께 정한다. ②는 이제 효과 없는 Body 4 판정이라 ③(물러난다)과 결과만 다르다.
8. C05 ② `ChangeStability +0.05`(새 값)와 C11 ② `AddResource Insight 120`(자리 표시 값) — `-Crossroads` 측정 뒤 정한다.
9. C08 ① Label을 '가장 작은 눈금 속으로 접혀 든다'로 바꾼 것은 T-09를 선택 이름까지 넓힌 이 판의 판단이다.
10. `log_brick_scan_spread` 변형 셋 대 하나(draft_events 열린 점 4) — 하나면 `single_variant_rows`에 행을 더하고 [0]만 남긴다.
11. `shape_merit_words`에 '남긴'을 더할지(§7.1, 선택).
12. `KeptLaw` DevComment는 노트 §5.4 코6의 글과 다르다(코1의 코드에 맞춤 — draft_events 열린 점 5).
13. 헤더 §2.4 48행의 한 성계 전제(F6) — `[B-7x5c]`에서 고친다.
14. 갈림 낱말 규칙의 커밋 나눔(F12, §7.3) — `[B-7x4]`·`[B-7x5c]`·`[B-7x3]`의 몫을 노트 §5.4 데3에 한 줄로 적는다.

### 8.4 검사 결과

검사기: `stage3/texts/src/check_audited.py`(이 파일의 json 블록을 모두 파싱하고, 글 블록의 모든 문자열을 main `schema.json`의 이름 붙은 목록 전부와 `crossroads_words`·`shape_merit_words`·헤더 §3.5 금지 낱말·숫자·슬롯·`|hpp(…)` 꼴·문장 수에 대어 본다; csv 줄은 `Shape.` 금지 목록, ini 줄은 ASCII). 결과는 아래에 붙인다.

결과(2026-11-10, 최종 비평 F1~F13을 적용한 뒤 다시 돌림 — `python -I -X utf8 src/check_audited.py audited_texts.md <main schema.json>`; 줄마다의 출력은 `stage3/texts/src/check_audited_out.txt`, 고치기 전 판의 사본은 `stage3/texts/src/audited_texts_before_final_fix.md`와 그 출력 `src/old_out.txt`):

```text
json blocks parsed: 28 of 28
strings checked: 160, csv lines: 13, ini lines: 17
FAILS (0)
unhooked list hits (info): 67
screen text: header 3.5 banned word counts: 0
screen text: verdict word counts: 0
screen text: crossroads_words counts: 0
whole file: header 3.5 banned word counts: (아래 줄 — 낱말은 인용하지 않는다)
whole file: verdict word counts: 0
```

- 화면 글(카드 열둘·사건 행 아홉·스캔 얼굴·ST_CA 열세 줄): 헤더 §3.5 금지 낱말 0, 판정 낱말 0, `crossroads_words` 0, 숫자 0, 걸린 목록 어김 0, 슬롯·`|hpp(…)` 꼴·두 문장 넘김 0. 최종 비평으로 바뀐 화면 글 다섯 줄(C10 목소리[0]·② 결과, C12 ①·② 결과, `Shape.KeptLaw`)도 main의 어느 이름 붙은 목록에도 새로 걸리지 않는다 — 정보 맞음은 68 → 67이고, 빠진 하나는 C10 ② 옛 결과 줄의 stay_words '함께'('그 이름도 함께 읽었다')다. DevComment 열일곱은 ASCII이고 'became'·'one mind'·'lives on as'·'sealed house'·'folded inward'·'immortal' 같은 꼴이 없다('mortal bodies'도 F9로 빠졌다).
- 파일 전체의 금지 낱말 셈은 40회이고 모두 화면 밖의 산문이다: §7.1의 목록 내용, §7.3의 why 줄과 깨짐 시험의 낱말(최종 비평 F2·F3이 더한 '안식'·'고요'·'잠잠'·'순례' 등), why 줄의 '…없음' 확인, §8.5의 인용. 이 판은 셈을 낱말별로 인용하지 않으므로 기록을 붙여도 셈이 늘지 않는다. 판정 낱말은 파일 전체에서도 0이다.
- 걸리지 않은 목록의 맞음 67건(정보): witness_words '떠나/떠날'(claim을 기다리는 `enc_` 행에만 걸림), stay_words '함께'(절멸 행에만), dawn_words '빛'·'왕좌'·'다스리'(결박·압력 행에만; C10의 '왕좌'·'다스리'는 Label과 목소리에만 남고 제목·① 결과 줄에는 없다), record_object_words·second_person·reader_address(기록 줄·죽음 줄에만), shape_merit_words '남은'(C03 '뭍에 남은 집들' — `Shape.` 키에만 걸림), triumph_words '흩어'·'쫓'(결박 줄에만), binding_name_words '둘레'(결박 이름에만), awareness_words·sign_voice_words(징조 행에만). 어느 것도 이 행들에 걸리는 규칙이 아니다.
- 새 꼴 낱말 여섯('밖으로 뻗은'·'별 사이에서 걸러 사는'·'작은 눈금 속으로 접힌'·'항구를 닫아건'·'칙령을 쉽게 바꾸지 않는'·'별의 불을 끌어 쓰는')은 main의 어느 이름 붙은 목록에도, `crossroads_words`·`shape_merit_words`에도 걸리지 않는다(위 정보 목록에 없음).

### 8.5 최종 비평(F1~F13) — 적용과 건너뜀

열세 건을 모두 적용했다. 건너뛴 것은 없다. F13은 두 갈래 가운데 '뺀다'를 골랐고 그 일부(첫 갈림 셋)는 리드 확인으로 남겼다(§8.3 열린 점 6).

| id | 무게 | 적용한 자리 | 어떻게 |
|---|---|---|---|
| F1 | high | §1.5~§1.9·§1.11 효과 줄, §6.1 규칙 글과 표, §6.2 JSON, §6.3 문서 줄, 머리말 | 닫지 않는 ② 다섯(C05·C06·C07·C08·C11 — C10은 이미 없었다)과 C09 ② 실패에서 기울기를 뺐다. C09 ② 성공의 +0.05는 카드를 닫으므로 남겼다. 설계서 §4.3 공통의 문장 고침을 §6.3에 적었다. 옛 열린 점 6을 닫았다 |
| F2 | medium | §7.3 시험 줄 | 사건 행의 `crossroads_words` 깨짐을 '방주'(이미 `ark_words`가 열에서 막는다) → '순례'로. '목자'도 `revelation_mimicry` 때문에 쓰지 않는다 |
| F3 | medium | §7.3 시험 줄 | 새로 거는 목록마다 새 규칙으로만 깨지는 시험 하나: 카드 VoiceBodies '안식'·Label '올라섰'·SuccessText '면류관'·Title '언약', 사건 행 '고요'·'순례', 스캔 얼굴 '잠잠'·'성소'. 각 낱말이 main에서 그 칸에 걸린 목록에 없음을 `schema.json`으로 확인했다 |
| F4 | medium | §1.12 효과 줄·② 결과 줄·why, §6.1 표, §6.2 JSON, §8.3 열린 점 7 | 권한 쪽: C12 ② 성공을 `[]`로 비우고 둘째 문장을 '반쯤 지은 배 가운데 한 척이 마저 지어졌다.'로 했다(비평 예문의 '물에 떴다'는 별 사이의 배에 맞지 않아 Body의 '반쯤 지은 배들'을 받는 꼴로 바꿨다). 우러름을 남기려면 헤더 §11에 적는 길을 열린 점 7에 적었다 |
| F5 | medium | §3 csv·표·why, §8.3 열린 점 4, §8.4 | `Shape.KeptLaw` '칙령을 지닌 채 사는 살림' → '칙령을 쉽게 바꾸지 않는 살림'. `Shape.` 금지 목록 전부를 지난다(검사 FAILS 0) |
| F6 | low | §1.12 ① 결과 줄과 why | '…살림을 배에 싣는 쪽으로 기울었다.' → '…살림을 배에 싣기 시작했다.' |
| F7 | low | §1.10 목소리[0]와 why | '의회는 다스리는 이가 정해 둔 칙령에 손대지 않은 채 내려오기를 청했다.' |
| F8 | low | §1.10 ② 결과 줄과 why | '…칙령을 읽는 이들은 누가 정한 법인지 알고 마음을 놓았다.' |
| F9 | low | §5 `Society` DevComment와 why | 'mortal bodies'를 빼고 'an ordinary society of households'; '첫 갈림이 걷는' 선택을 'a branch choice that succeeds or … its second choice'로 |
| F10 | low | §5 `Nomad` DevComment와 why | 출처 셋(씨앗·`petition_cross_drifted`·`enc_cross_drift_fleet`)을 적었다 |
| F11 | low | §3 `ShutHouse` why | `ShapeOf`는 `ClosedPort`나 `ShutSphere`에 `ShutHouse`를 준다(헤더 §2.7)는 사실로 까닭을 바로잡았다. 낱말은 그대로 |
| F12 | low | §7.3 커밋 나눔, 노트 데3의 줄, §8.3 열린 점 14 | `[B-7x4]` = 목록·조건 규칙 셋·시험 행의 깨짐, `[B-7x5c]` = 이름 규칙 셋, `log_brick_scan_spread` 규칙 = 그 행이 든 뒤의 첫 커밋 |
| F13 | low | §1.1~§1.3·§1.5~§1.8 효과 줄, §6.1·§6.2, §8.3 열린 점 6 | 판정 ①의 실패 일곱(C01·C02·C03·C05·C06·C07·C08)에서 기울기를 뺐다 — 기울기는 카드를 닫는 결과에만 남는다는 한 규칙. 첫 갈림 셋은 결정 11 (나)의 글자 밖이라 리드에게 둔다 |

바뀐 화면 글: C10 목소리[0]·② 결과(F7·F8), C12 ①·② 결과(F6·F4), `Shape.KeptLaw`(F5). 바뀐 효과: §6.2의 열다섯 칸(① 실패 일곱, ② 성공 일곱, C09 ② 실패 하나). 바뀐 DevComment: `Society`·`Nomad`. 사건 행 아홉과 스캔 얼굴은 바뀌지 않았다.
