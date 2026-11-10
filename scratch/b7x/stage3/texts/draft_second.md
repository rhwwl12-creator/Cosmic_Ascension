# [B-7x4] 글 — 둘째 갈림 카드 일곱(C05~C11)과 방랑자 카드(C12)

> 출처: 초안 데이터 `aa09543:Tools/data/DT_Encounters.json`의 행 글 그대로에서 시작해, 2026-11-10 개발자 답(결정 3·11 (나)·13 ㉢ (d)·16·18)과 노트 §5.4 데2, 헤더 §3.5 글의 규칙, 규칙 11을 적용했다. 규칙을 이미 지킨 칸은 글자 그대로 두었다.
> 잎 이름은 새 이름만 쓴다(`KeptLaw`·`FoldedSmall`·`ShutSphere`·`DustBand`·`StarTow`·`ClosedPort`) — 화면 글에는 어느 설계 이름도 나오지 않는다.
> 낱말 검사: 아래 json 블록의 모든 문자열을 main `Tools/data/schema.json`의 verdict·holy_words·soul_verbs·rescue_words·silence_words·rung_grasp_words·title_words·holy_and_myth·realm_names·revelation_mimicry·ark_words·afterlife_words와 새 `crossroads_words`, 헤더 §3.5의 금지 낱말(구원·중보·침묵·고요·잠잠·쉼·안식·울타리·언약·방주·목자·순례·성소·불멸·영원·영겁·'신이 되'·'하나가 되'·일체화)에 대어 보았고 숫자도 보았다 — 걸린 것 없음(스크립트 결과는 맨 끝).
> 효과 줄의 '목소리 기울기'는 관례의 `ShiftEthics Spiritualism`(의회가 청한 선택 −0.05, 장로들이 청한 선택 +0.05)이다.

---

## petition_cross_dust_band — C05 (지은 시간의 둘째 갈림 → 띠)

- 조건: `Lottery` · 무게 1 · 주제 Civilization · `PlayerRoleIs Ruler` · `CivTierAtLeast 3` · `CivHasTrait Brick.Instinct.SwarmTech` · `CivLacksTrait Brick.Mutation.DustBand` · `CivLacksTrait Brick.Instinct.HiveMerge` · `CivTimeVectorAtLeast Linked 1800`
- 효과: ① 판정 Stability 9, 에너지 150·물질 350 — 성공 `AddTrait Brick.Mutation.DustBand`, 기울기 −0.05 / 실패 `ChangeStability −0.05`, 기울기 −0.05 · ② 판정 없음, 물질 150 — `ChangeStability +0.05`, 기울기 +0.05 (**바뀜** — `ChangePopulation +0.03`을 뺐다) · ③ 없음

```json
{
	"Title": "별 둘레의 티끌 띠",
	"Body": "",
	"VoiceBodies": [
		"{Civ}의 작은 일꾼들은 스스로를 고쳐 가며 일한다. 의회는 그 일꾼들을 별마다 띠로 둘러 손이 닿지 않던 일까지 맡기자고 청했다.",
		"{Civ}의 작은 일꾼들은 스스로를 고쳐 가며 일한다. 회중의 장로들은 누구의 손에도 매이지 않는 것을 늘리기 전에 사람을 먼저 기르자고 청했다."
	],
	"Choices": [
		{
			"Label": "일꾼들을 별마다 띠로 두른다",
			"SuccessText": "{Civ}의 별마다 티끌 같은 일꾼들이 띠를 이루었다. 띠는 스스로를 고치며 돌았고, 그 일을 다 아는 사람은 없었다.",
			"FailureText": "{Civ}의 일꾼들은 띠를 이루지 못하고 흩어졌다. 들인 것을 두고 의회의 말이 엇갈렸다."
		},
		{
			"Label": "사람을 먼저 기른다",
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

- why ②.SuccessText: '{Civ}의 물자가 집과 배움터를 짓는 데 들었다. 집집에 식구가 늘었다.' → '… 아이를 기르는 집들의 근심이 덜어졌다.' — 효과가 `ChangePopulation`에서 `ChangeStability`로 바뀌어(아래 효과 고침) '식구가 늘었다'는 일어나지 않는 일을 적게 된다; 새 둘째 문장은 안정도의 사실이고 사람의 수를 말하지 않는다(헤더 §3.5).
- 다른 칸은 그대로(규칙을 모두 지킨다).
- 효과 이유 (②):
  - `ChangePopulation +0.03` **뺌** — 인구는 경제 단계의 회복이 1 아래에서만 채우고(main `Sim/GalaxySimulationSubsystem.cpp:1741-1743`) `ChangePopulation`은 `Population × (1 + v)`로 위 상한 없이 곱한다(`Events/CosmicEventSubsystem.cpp:893-900`) — 되풀이마다 1 위로 쌓이고, 그 값이 통찰·영향력 수입을 곱한다(`:1773-1774`). 결정 11 (나)의 되풀이 이득이다.
  - `ChangeStability +0.05` **더함** — `petition_road` ②('쉬게 한다')와 같은 작은 한 번의 효과; 이것이 없으면 ②는 값만 내고 기울기 말고는 아무 일도 하지 않는다(③에 지배된다). 값이 아니라 꼴의 선택이므로 리드가 뺄 수 있다(열린 점 2).
  - 기울기 +0.05 **둠** — 장로들이 청한 선택의 관례(설계서 §4.3 공통); `petition_road`·`petition_feast`의 ②가 지닌 꼴 그대로이고 ±1에 묶인다(열린 점 1).
  - 물질 150 **둠** — 값.

---

## petition_cross_star_line — C06 (떠돈 시간의 둘째 갈림 → 별에 거는 줄)

- 조건: `Lottery` · 무게 1 · 주제 Civilization · `PlayerRoleIs Ruler` · `CivTierAtLeast 3` · `CivHasTrait Brick.Space.Nomad` · `CivLacksTrait Brick.Mutation.StarTow` · `CivTimeVectorAtLeast Drifted 1800`
- 효과: ① 판정 Stability 9, 에너지 300·물질 150 — 성공 `AddTrait Brick.Mutation.StarTow`, 기울기 −0.05 / 실패 `ChangeStability −0.06`, 기울기 −0.05 · ② 판정 없음, 물질 250 — `ChangeStability +0.05`, 기울기 +0.05 (그대로) · ③ 없음

```json
{
	"Title": "별에 거는 줄",
	"Body": "",
	"VoiceBodies": [
		"{Civ}의 살림은 오래 배 위에서 꾸려졌다. 의회는 도읍의 별에 끄는 줄을 걸어 데리고 떠날 채비를 하자고 청했다.",
		"{Civ}의 살림은 오래 배 위에서 꾸려졌다. 회중의 장로들은 떠날 채비로 지은 집에서는 아무도 늙지 못한다며 집부터 고치자고 청했다."
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

- why: 모든 칸 그대로 — 규칙을 모두 지킨다. ① 결과 줄의 '별은 제자리에 있었고'가 슬라이스 가에서 별이 움직이지 않는다는 사실(설계서 §4.3 C06 귀결)과 맞는다.
- 효과 이유 (②):
  - `ChangeStability +0.05` **둠** — `petition_road` ②의 꼴 그대로인 한 번의 효과(0~1에 묶인다).
  - 기울기 +0.05 **둠** — 관례(열린 점 1).
  - 물질 250 **둠** — 값.

---

## petition_cross_between_stars — C07 (삼킨 시간의 둘째 갈림 → 번진 살림)

- 조건: `Lottery` · 무게 1 · 주제 Civilization · `PlayerRoleIs Ruler` · `CivTierAtLeast 3` · `CivHasTrait Brick.Diet.DarkMatter` · `CivLacksTrait Brick.Form.LivingNebula` · `CivTimeVectorAtLeast Consumed 2000`
- 효과: ① 판정 Stability 9, 에너지 300·통찰 150 — 성공 `AddTrait Brick.Form.LivingNebula`, 기울기 +0.05 (장로들이 청한 쪽) / 실패 `ChangeStability −0.06`, 기울기 +0.05 · ② 판정 없음, 물질 250·영향력 60 — `ChangeStability +0.04`, 기울기 −0.05 (의회가 청한 쪽) (**바뀜** — `ShiftEthics Openness −0.05`를 뺐다) · ③ 없음

```json
{
	"Title": "별 사이의 엷은 것",
	"Body": "",
	"VoiceBodies": [
		"{Civ}의 살림은 별 사이에서 걸러 낸 것으로 오래 꾸려졌다. 의회는 지붕부터 고쳐 땅 위에 디딜 곳을 지키자고 청했다.",
		"{Civ}의 살림은 별 사이에서 걸러 낸 것으로 오래 꾸려졌다. 회중의 장로들은 땅에 매인 살림을 별들 사이의 엷은 것에 풀자고 청했다."
	],
	"Choices": [
		{
			"Label": "살림을 별들 사이에 푼다",
			"SuccessText": "{Civ}|hpp(이,가) 살림을 별들 사이의 엷은 것에 풀었다. 그 일이 끝났을 때 그들은 별과 별 사이에 번져 서 있었다.",
			"FailureText": "{Civ}|hpp(이,가) 별들 사이에 푼 살림은 번지지 못하고 흩어졌다. 땅 위의 집들이 그 일을 두고 술렁였다."
		},
		{
			"Label": "땅 위의 집을 고쳐 짓는다",
			"SuccessText": "{Civ}의 땅 위 집들이 지붕부터 고쳐졌다. 디딜 곳이 단단해지자 거리의 술렁임이 가라앉았다.",
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

- why ②.SuccessText: '… 그들은 디딜 곳을 지키며 바깥의 것을 덜 들였다.' → '… 디딜 곳이 단단해지자 거리의 술렁임이 가라앉았다.' — 둘째 문장은 빠진 `ShiftEthics Openness −0.05`(닫힘으로 기울기)를 말하던 줄이다; 효과가 빠진 뒤에는 수에 없는 일을 적게 되어, 남은 효과(`ChangeStability +0.04`)의 사실로 바꿨다(헤더 §3.5 — 한 일과 그 뒤에 서 있게 된 자리).
- 다른 칸은 그대로.
- 효과 이유 (②):
  - `ShiftEthics Openness −0.05` **뺌** — 닫지 않는 ②가 되풀이될 때마다 둘째 윤리 축을 같은 쪽으로 미는 쌓이는 이동이고(±1에 묶일 뿐), 그 축이 영향력 수입의 덤을 읽는다(main `Sim/GalaxySimulationSubsystem.cpp:1774`). 결정 11 (나)가 이름한 '윤리 이동'이며 `petition_road`·`petition_feast`의 ②에는 목소리 기울기 말고 둘째 축이 없다.
  - `ChangeStability +0.04` **둠** — 한 번의 작은 효과.
  - 기울기 −0.05 **둠** — 관례(의회가 청한 쪽; 열린 점 1).
  - 물질 250·영향력 60 **둠** — 값.

---

## petition_cross_into_dust — C08 (떠돈 살림이 눌린 시간 끝에 → 티끌 속으로 접힘)

- 조건: `Lottery` · 무게 1 · 주제 Civilization · `PlayerRoleIs Ruler` · `CivTierAtLeast 3` · `CivHasTrait Brick.Space.Nomad` · `CivLacksTrait Brick.Space.FoldedSmall` · `CivLacksTrait Brick.Mutation.StarTow` · `CivTimeVectorAtLeast Endured 1800`
- 효과: ① 판정 Stability 9, 에너지 300·통찰 200 — 성공 `AddTrait Brick.Space.FoldedSmall`, `RemoveTrait Brick.Space.Nomad`, 기울기 −0.05 / 실패 `ChangeStability −0.06`, 기울기 −0.05 · ② 판정 없음, 물질 200 — `ChangeStability +0.05`, 기울기 +0.05 (그대로) · ③ 없음

```json
{
	"Title": "티끌 속의 넓이",
	"Body": "",
	"VoiceBodies": [
		"{Civ}|hpp(은,는) 떠돈 날이 길었다. 의회는 가장 작은 눈금 속에 다투는 이 없는 넓이가 있다며 그리로 접혀 들자고 청했다.",
		"{Civ}|hpp(은,는) 떠돈 날이 길었다. 회중의 장로들은 밖을 내다보지 않는 살림은 밖의 일을 잊는다며 별 곁에 디딜 곳을 마련하자고 청했다."
	],
	"Choices": [
		{
			"Label": "티끌 속으로 접혀 든다",
			"SuccessText": "{Civ}|hpp(이,가) 살림을 가장 작은 눈금 속으로 접어 넣었다. 그 일이 끝났을 때 그들은 티끌 속에 서 있었고, 떠돌던 배들은 비어 있었다.",
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

- why VoiceBodies[0]·[1] 첫 문장: '{Civ}|hpp(은,는) 떠도는 동안 견딘 날이 길었다.' → '{Civ}|hpp(은,는) 떠돈 날이 길었다.' — 노트 §5.4 데2와 리뷰 L4-18의 '떠돈'(구속 결정). '견딘'은 desolation_words(main `schema.json:75`)이고, '떠도는 날'로 바꾸면 같은 줄의 '떠도는 동안'과 겹치므로 그 머리까지 '떠돈 날이 길었다' 하나로 줄였다. 두 목소리의 첫 문장이 같은 꼴은 다른 카드와 같다.
- 다른 칸은 그대로. ① 결과 줄의 '떠돌던 배들은 비어 있었다'는 살림이 접혀 들어간 사실이지 옛 백성이 살아남았다·돌아왔다는 말이 아니다(결정 16).
- 효과 이유 (②):
  - `ChangeStability +0.05` **둠** — 한 번의 작은 효과(`petition_road` ②의 꼴).
  - 기울기 +0.05 **둠** — 관례(열린 점 1).
  - 물질 200 **둠** — 값.

---

## petition_cross_wrapped_star — C09 (홀로 선 시간의 둘째 갈림 → 닫힌 구, 또는 하나뿐인 되돌림)

- 조건: `Lottery` · 무게 1 · 주제 Civilization · `PlayerRoleIs Ruler` · `CivTierAtLeast 2` · `CivHasTrait Brick.Space.ClosedPort` · `CivLacksTrait Brick.Form.ShutSphere` · `CivTimeVectorAtLeast Isolated 2000`
- 효과: ① 판정 없음, 에너지 200·물질 400 — `AddTrait Brick.Form.ShutSphere`, `ShiftEthics Openness −0.05`, 기울기 −0.05 (카드를 닫는다) · ② 판정 Stability 8, 영향력 150 — 성공 `RemoveTrait Brick.Space.ClosedPort`, `ChangeStability −0.05`, 기울기 +0.05 (걷힌 부모로 닫힌다) / 실패 `ChangeStability −0.1`, 기울기 +0.05 (그대로) · ③ 없음

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

- why: 모든 칸 그대로 — 규칙을 모두 지킨다('닫힌 집'·'닫힌 항구'는 desolation_words의 '닫힌 문'·'문이 닫'에 걸리지 않고, 그 목록은 이 표에 걸리지도 않는다).
- 효과 이유 (② — 되돌림):
  - 성공의 `RemoveTrait Brick.Space.ClosedPort` **둠** — 하나뿐인 되돌림; 부모가 걷혀 카드가 닫히므로 되풀이되지 않는다.
  - 성공의 `ChangeStability −0.05`, 실패의 `ChangeStability −0.1` **둠** — 잃는 것뿐이다(실패하면 다시 오지만 쌓이는 것은 손실이다).
  - 기울기 +0.05(성공·실패) **둠** — 관례(열린 점 1). 실패해 되풀이되면 장로 쪽으로 기우는 것만 쌓인다 — 이득이 아니라 기울기다.
  - 영향력 150 **둠** — 값.

---

## petition_cross_law_stays — C10 (눌린 시간의 둘째 갈림 → 남긴 법과 양위)

- 조건: `Lottery` · 무게 1 · 주제 Civilization · `PlayerRoleIs Ruler` · `CivTierAtLeast 3` · `CivLacksTrait Brick.Form.KeptLaw` · `CivTimeVectorAtLeast Endured 2000`
- 효과: ① 판정 없음, 값 없음 — `AddTrait Brick.Form.KeptLaw`, `DeposeRuler`(Tag `Reign.Abdicated`), 기울기 −0.05 · ② 판정 없음, 영향력 60 — `ChangeStability +0.05` 하나 (**바뀜** — `ChangeReverence +0.1`·`PlayerStatDelta Exaltation +1`·`ShiftEthics Spiritualism +0.05`를 뺐다) · ③ 없음

```json
{
	"Title": "손대지 않은 칙령",
	"Body": "",
	"VoiceBodies": [
		"{Civ}의 칙령은 눌린 날들 내내 다스리는 이의 손에 매여 있었다. 의회는 왕좌를 비우고도 칙령이 서는지 보아야 한다며 내려오기를 청했다.",
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
			"SuccessText": "{Civ}의 칙령 머리마다 다스리는 이의 이름이 새겨졌다. 칙령을 읽는 이들은 그 이름도 함께 읽었다.",
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

- why Title: '비워 둔 왕좌' → '손대지 않은 칙령' — **구속 결정의 '거두지 않은 칙령'을 그대로 쓰지 않았다**: '거두'는 main `schema.json`의 soul_verbs(`["거두", "거둬", "거뒀", "거둔", "거둘"]`)이고, DT_Encounters 규칙 1(`schema.json:1040-1048`)이 `petition_` 행의 Title·Body·VoiceBodies·Choices 글 모두에서 soul_verbs를 막는다 — 그 제목은 `check_data.py`에서 깨진다. 같은 뜻(칙령을 무르지 않은 채 두었다)을 ① 결과 줄의 낱말 '손대지 않은'으로 받았다: 빈 자리·돌아올 왕의 그림이 없고(헤더 §3.5 — 리뷰 T8), dawn_words의 '왕좌'·'다스리'를 피하며(노트 데2 ⑩), 그 순간 한 일만 말해 남긴 법의 좁힘(결정 18 — 왕좌가 빈 뒤에도 주장의 고리와 위기의 답인 칙령은 거둔다)보다 더 약속하지 않는다('무르지 않은 칙령'은 모든 칙령이 남는다고 읽혀 고르지 않았다). 열린 점 3.
- why VoiceBodies[0]·[1]: '견딘 날들 내내' → '눌린 날들 내내' — 노트 §5.4 데2(구속; '견딘'은 desolation_words, 꼴 낱말 `Endured` '눌림 속에 이어 간 살림'과 같은 결).
- why Choices[0].SuccessText: '… 왕좌는 비어 있었다.' → '{Civ}|hpp(은,는) 정해 둔 칙령을 손대지 않은 채 두었다. 칙령을 정한 이는 자리에서 내려왔다.' — 노트 §5.4 데2의 글자 그대로(구속; 양위 뒤의 빈 자리 그림 금지 — 헤더 §3.5).
- 다른 칸은 그대로: 의회 목소리의 '왕좌를 비우고도 칙령이 서는지 보아야 한다'는 양위 앞의 청이고 시험을 말할 뿐 무엇이 남는다고 약속하지 않는다(결정 18); ① Label의 '왕좌'는 하는 일의 이름이다(main `petition_keep_throne`의 '왕좌를 내려놓고 떠난다'와 같은 결).
- 효과 이유 (②):
  - `PlayerStatDelta Exaltation +1` **뺌** — 결정 11 (나)가 이름한 것(헤더 §3.1·결정 11의 답).
  - `ChangeReverence +0.1` **뺌** — `ReverenceToPlayer`는 저장되는 값이고(`Events/CosmicEventSubsystem.cpp:829-831`, 0~1에 묶임) 닫지 않는 ②가 되풀이될 때마다 위로 쌓인다 — 결정 11 (나)의 되풀이 이득. 설계서 §4.3 C10 ②의 줄은 아직 `ChangeReverence +0.1`을 적고 있어 `[B-7x5c]`가 맞춘다(열린 점 4).
  - `ShiftEthics Spiritualism +0.05` **뺌** — 노트 §5.4 데2(구속 — 다스리는 이의 이름을 높이는 일을 영성으로 세지 않는다, `petition_exalt`의 결). 장로들이 청한 선택이지만 관례의 예외다.
  - `ChangeStability +0.05` **둠** — 한 번의 작은 효과.
  - 영향력 60 **둠** — 값.
- 효과 이유 (① — 참고, 바꾸지 않음): 양위가 카드를 닫는 쪽이고(`KeptLaw`로 `CivLacksTrait`가 거짓), 높아짐을 깎아 주지 않는다(설계서 §4.3 C10 귀결).

---

## petition_cross_seam — C11 (지은 시간의 둘째 갈림 → 이음매를 지우자는 유혹)

- 조건: `Lottery` · 무게 1 · 주제 Civilization · `PlayerRoleIs Ruler` · `CivTierAtLeast 3` · `CivHasTrait Brick.Instinct.SwarmTech` · `CivLacksTrait Brick.Instinct.HiveMerge` · `CivLacksTrait Brick.Mutation.DustBand` · `CivTimeVectorAtLeast Linked 2000`
- 효과: ① 판정 없음, 값 없음 — `AddTrait Brick.Instinct.HiveMerge`, `RemoveTrait Brick.Instinct.SwarmTech`, `ChangeStability +0.08`, `PlayerStatDelta Exaltation +2`, 기울기 −0.05 (값은 노트 데2대로 지금 고치지 않는다) · ② 판정 없음, 영향력 80 — `ChangeStability −0.02`, 기울기 +0.05 (**바뀜** — `ShiftEthics Openness +0.05`를 뺐다) · ③ 없음

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
			"SuccessText": "{Civ}의 이웃과 맞닿은 자리마다 고쳐 쓸 말이 정해졌다. 여러 목소리를 맞추는 동안 회의는 길어졌다.",
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

- why: 모든 칸 그대로 — 규칙을 모두 지킨다. ① 결과 줄은 뒤엉킴을 좋은 끝으로 적지 않는다(다른 말이 그치고 남은 것이 다스리는 이의 말뿐 — 헤더 §3.5의 '타락의 교리와 닮은 갈래'); '하나가 되'·일체화를 쓰지 않는다. ② 결과 줄은 열림으로 기운다는 말을 하지 않아 효과를 뺀 뒤에도 사실과 맞는다.
- 효과 이유 (②):
  - `ShiftEthics Openness +0.05` **뺌** — 되풀이마다 둘째 윤리 축을 같은 쪽으로 쌓고 그 축이 영향력 수입의 덤을 키운다(main `Sim/GalaxySimulationSubsystem.cpp:1774`) — 결정 11 (나)의 되풀이 이득.
  - `ChangeStability −0.02` **둠** — 손실(회의가 길어진 값)이고 쌓여도 이득이 아니다.
  - 기울기 +0.05 **둠** — 관례(열린 점 1).
  - 영향력 80 **둠** — 값. 이렇게 두면 ②는 ③보다 값만 많은 선택이 된다(열린 점 5).

---

## enc_cross_drift_fleet — C12 (방랑자, 떠날 채비)

- 조건: `Standing` · 무게 1 · 주제 VesselSystem · `PlayerRoleIs Wanderer` · `CivTimeVectorAtLeast Drifted 900` · `CivSettledSystemsAtLeast 2` · `CivLacksTrait Brick.Space.Nomad` · `CivLacksTrait Brick.Space.ClosedPort` · `CivLacksTrait Brick.Space.FoldedSmall` · `CivLacksTrait Brick.Form.Society`
- 효과: ① 판정 Body 5, 값 0 — 성공 `AddTrait Brick.Space.Nomad`(주인 백성), `ChangeReverence +0.2` / 실패 `ChangeReverence −0.1` · ② 판정 Body 4, 값 0 — 성공 `ChangeReverence +0.05` / 실패 없음 (그대로 — 열린 점 6) · ③ 없음

```json
{
	"Title": "떠날 채비",
	"Body": "{System}의 사람들은 집을 통째로 싣는 배를 지으며 붙박인 터를 버릴지를 두고 갈려 있다. 떠도는 살림이 오래되면 제 별까지 데리고 간다고 기록은 적는다.",
	"Choices": [
		{
			"Label": "떠도는 살림의 법을 보인다",
			"SuccessText": "당신은 배 위에서 먹고 자고 고치는 법을 보였다. {Civ}의 사람들은 터를 버리는 쪽으로 기울었다.",
			"FailureText": "당신이 보인 배 위의 살림은 {Civ}의 사람들에게 닿지 않았다. 조선소의 사람들은 당신을 낯설게 보았다."
		},
		{
			"Label": "조선소에서 일손을 보탠다",
			"SuccessText": "당신은 조선소에서 배를 짓는 일을 거들었다. {Civ}의 일꾼들이 당신의 손을 기억했다.",
			"FailureText": "당신이 거든 일은 조선소에 보탬이 되지 못했다. 배는 {Civ}의 일꾼들 손으로 지어졌다."
		},
		{
			"Label": "물러난다",
			"SuccessText": "당신은 조선소 곁에 서 있었다. 떠날지 남을지는 {Civ}의 사람들이 정할 일로 남았다.",
			"FailureText": ""
		}
	]
}
```

- why: 모든 칸 그대로 — 규칙을 모두 지킨다. 목소리가 없는 카드라 Body로 말하고(`VoiceBodies` 칸이 초안에 없어 이 블록에도 넣지 않았다 — 빈 배열로 둘지는 행의 꼴대로), 상시 카드라 `{Civ}`를 써도 된다(도착 카드만 `{System}`뿐 — DT_Encounters 규칙 '`VesselSystem` ∧ `Lottery`'). 방랑자에게 칭호를 주지 않고, 사람의 수가 없다. Body 둘째 문장의 '제 별까지 데리고 간다'는 기록이 적는 말이지 이 판의 약속이 아니다 — 슬라이스 가에서 별은 움직이지 않고, C06 ① 결과 줄('별은 제자리에 있었고')이 그 사실을 말한다(열린 점 7).
- 효과 이유 (②):
  - `ChangeReverence +0.05` **둠** — 결정 11 (나)는 통치자의 둘째 갈림 ②에 대한 답이고, 이 카드는 방랑자의 상시 카드다. 다만 ②는 카드를 닫지 않고(①의 성공만 `Nomad`로 닫는다), 방랑자 상시 카드는 조건이 새로 설 때마다(성계에 들어설 때) 다시 오며 막 답한 카드만 바로 오지 않는다(`Events/CosmicPetitions.cpp:291-301`) — 같은 백성의 성계 둘을 오가면 우러름(왕좌의 문)이 +0.05씩 쌓인다. main의 `enc_witness`(+0.1)·`enc_plague_tend`(+0.3)도 같은 꼴이라 지금은 두었다. 열린 점 6.

---

## 효과 고침 한눈에 (데이터 — `[B-7x5c]`가 옮긴다)

| 행 | 선택 | 전 (aa09543) | 후 |
|---|---|---|---|
| `petition_cross_dust_band` | ② OnSuccess | `ChangePopulation +0.03`, `ShiftEthics Spiritualism +0.05` | `ChangeStability +0.05`, `ShiftEthics Spiritualism +0.05` |
| `petition_cross_star_line` | ② | `ChangeStability +0.05`, `ShiftEthics Spiritualism +0.05` | 그대로 |
| `petition_cross_between_stars` | ② OnSuccess | `ChangeStability +0.04`, `ShiftEthics Openness −0.05`, `ShiftEthics Spiritualism −0.05` | `ChangeStability +0.04`, `ShiftEthics Spiritualism −0.05` |
| `petition_cross_into_dust` | ② | `ChangeStability +0.05`, `ShiftEthics Spiritualism +0.05` | 그대로 |
| `petition_cross_wrapped_star` | ② (되돌림) | 성공 `RemoveTrait Brick.Space.ClosedPort`(초안은 옛 잎 이름), `ChangeStability −0.05`, `ShiftEthics Spiritualism +0.05` / 실패 `ChangeStability −0.1`, `ShiftEthics Spiritualism +0.05` | 그대로(잎 이름만 새 이름) |
| `petition_cross_law_stays` | ② OnSuccess | `ChangeStability +0.05`, `ChangeReverence +0.1`, `PlayerStatDelta Exaltation +1`, `ShiftEthics Spiritualism +0.05` | `ChangeStability +0.05` |
| `petition_cross_seam` | ② OnSuccess | `ShiftEthics Openness +0.05`, `ChangeStability −0.02`, `ShiftEthics Spiritualism +0.05` | `ChangeStability −0.02`, `ShiftEthics Spiritualism +0.05` |
| `enc_cross_drift_fleet` | ② | 성공 `ChangeReverence +0.05` / 실패 없음 | 그대로(열린 점 6) |

값(Cost)·판정·①·③은 어느 행도 바꾸지 않는다. 잎 이름은 `rename_leaves.py`(결정 3)가 바꾼다 — 이 문서의 조건 줄은 바뀐 뒤의 이름이다.

## 열린 점 (리드·개발자)

1. **목소리 기울기를 되풀이 이득으로 세는가** — 과제 글은 '윤리 이동'을 쌓이는 이득으로 꼽았지만, 결정 11 (나)가 가리킨 꼴(`petition_road`·`petition_feast`의 ②)이 바로 목소리 기울기 `ShiftEthics Spiritualism ±0.05`를 지니고, 설계서 §4.3 공통이 그 관례를 정한다. 그래서 둘째 축(Openness)과 C10 ②(데2)만 빼고 기울기는 두었다. 다만 `Spiritualism`은 통찰 수입의 덤을 읽는다(main `Sim/GalaxySimulationSubsystem.cpp:1773`) — 장로들이 청한 ②를 거듭 고르면 덤이 ±1까지 쌓인다. 빼려면 C05·C06·C08·C09(실패)·C11의 ② 기울기를 함께 빼야 하고, 그러면 '청한 쪽이 기울기를 옮긴다'는 관례에 예외가 여섯 생긴다.
2. **C05 ②의 대체 효과** — `ChangePopulation +0.03`을 빼고 `ChangeStability +0.05`를 넣었다(빼기만 하면 ②는 값과 기울기뿐이다). 새 값이므로 리드가 '빼기만'을 고르면 ② SuccessText 둘째 문장을 '집집의 살림이 한결 넉넉해졌다' 같은 효과 없는 사실로 바꿔야 한다(그때도 '식구가 늘었다'는 쓰지 않는다).
3. **C10 제목** — 구속 결정의 '거두지 않은 칙령'은 soul_verbs의 '거두'에 걸려 `petition_` 행 규칙(`schema.json:1040-1048`)에서 깨진다. '손대지 않은 칙령'으로 대신 적었다 — 리드가 받거나 다른 낱말을 정한다. 노트 §5.4 데2와 설계서 §4.3 C10의 '거두지 않은 칙령'도 같은 커밋에서 고쳐야 한다. 새 제목은 기리는 글로 읽히지 않아 `epitaph_leaves_out`에 넣을 까닭이 없다(설계서가 옛 제목에 적은 후보 표지는 지운다).
4. **설계서 §4.3 C10 ②의 효과 줄** — '`ChangeStability` +0.05, `ChangeReverence` +0.1'로 남아 있다. 이 초안은 우러름도 뺐다(되풀이마다 0~1 안에서 쌓인다). 결정 11 (나)의 글('적어도 Exaltation')보다 넓으므로 리드가 확인하고, 받으면 설계서 C10 ②·C11 ②·C07 ②·C05 ②의 줄을 `[B-7x5c]`에서 데이터와 맞춘다(노트 데1의 '설계서 §4.3 C05~C11의 ② 줄과 데이터가 같은지 같은 커밋에서 맞춘다').
5. **C11 ②가 ③에 지배된다** — Openness를 빼면 ②는 영향력 80과 안정도 −0.02를 내고 기울기만 얻는다(③은 값 없음). 유혹(①: 값 없이 +0.08)과 맞서는 '어려운 길'로 두려는 뜻이면 그대로 두고, 고를 까닭을 주려면 작은 한 번의 효과(예: `ChangeStability −0.02`를 `+0.03`으로 — ①보다 작게)를 두고 ② SuccessText 둘째 문장을 그 사실로 바꾼다. 수는 헤더 §5 ⑧(`-Crossroads` 칸) 뒤에 정하는 편이 맞다.
6. **C12 ②의 우러름** — 결정 11 (나)의 밖이라 두었지만, ②가 닫지 않고 성계를 오가면 되풀이된다(`enc_witness`와 같은 꼴). 헤더 §5 판단 ③′(도착 카드를 밀어내는가)와 함께 보아, 넘으면 노트 데5(무게 0.25 또는 `CivTierAtLeast 2`)와 함께 ②의 `ChangeReverence +0.05`를 빼는 안을 둔다.
7. **C12 Body의 '제 별까지 데리고 간다고 기록은 적는다'** — 슬라이스 가에서는 별을 끄는 수가 없다(설계서 C06 귀결). 기록의 말이라 두었지만, 글 감사가 '앞날을 말하지 않는다'를 이 카드에도 걸면 둘째 문장을 '떠도는 살림은 닻을 내린 곳을 집이라 부르지 않는다고 기록은 적는다' 같은 지금의 사실로 바꿀 수 있다.
8. **이 문서 밖** — 첫 갈림 넷(C01~C04)·`event_brick_*` 아홉·`log_brick_scan_spread`는 다른 초안의 몫이다. `crossroads_words`·silence_words·rung_grasp_words·title_words를 거는 스키마 규칙(노트 데3)이 들면 위 여덟 행의 글은 그대로 지난다(아래 검사).

## 낱말 검사 결과

(아래 줄은 이 문서의 json 블록 여덟을 뽑아 main `schema.json`의 목록과 `crossroads_words`·헤더 §3.5 금지 낱말·숫자에 대어 본 스크립트의 출력이다.)

```text
block 1 (별 둘레의 티끌 띠): strings 13, hits []
block 2 (별에 거는 줄): strings 13, hits []
block 3 (별 사이의 엷은 것): strings 13, hits []
block 4 (티끌 속의 넓이): strings 13, hits []
block 5 (닫힌 구의 셈): strings 13, hits []
block 6 (손대지 않은 칙령): strings 13, hits []
block 7 (이음매): strings 13, hits []
block 8 (떠날 채비): strings 11, hits []
```

aa09543 초안과 글자가 다른 칸(그 밖은 글자 그대로):

```text
petition_cross_dust_band.Choices[1].SuccessText: '{Civ}의 물자가 집과 배움터를 짓는 데 들었다. 집집에 식구가 늘었다.' -> '{Civ}의 물자가 집과 배움터를 짓는 데 들었다. 아이를 기르는 집들의 근심이 덜어졌다.'
petition_cross_between_stars.Choices[1].SuccessText: '{Civ}의 땅 위 집들이 지붕부터 고쳐졌다. 그들은 디딜 곳을 지키며 바깥의 것을 덜 들였다.' -> '{Civ}의 땅 위 집들이 지붕부터 고쳐졌다. 디딜 곳이 단단해지자 거리의 술렁임이 가라앉았다.'
petition_cross_into_dust.VoiceBodies changed
petition_cross_law_stays.Title: '비워 둔 왕좌' -> '손대지 않은 칙령'
petition_cross_law_stays.VoiceBodies changed
petition_cross_law_stays.Choices[0].SuccessText: '{Civ}|hpp(은,는) 정해 둔 칙령을 손대지 않은 채 두었다. 왕좌는 비어 있었다.' -> '{Civ}|hpp(은,는) 정해 둔 칙령을 손대지 않은 채 두었다. 칙령을 정한 이는 자리에서 내려왔다.'
```
