# [B-7x4] 글 — first crossroads (C01–C04) — drafter 'first-cross cards'

Scope: the four petitions whose conditions hold `CivHasTrait Brick.Form.Society` — `petition_cross_built`, `_consumed`, `_drifted`, `_isolated` (design doc 4.3 C01–C04). They close together: every choice ① and ② removes `Brick.Form.Society`. `enc_cross_drift_fleet` (C12) waits on `CivLacksTrait Brick.Form.Society` and is not in this set.

Sources: data draft `aa09543:Tools/data/DT_Encounters.json`; header, notes and design doc at `79f2583`; main `Tools/data/schema.json` (word lists and the `petition_` rules), main `Tools/data/DT_Encounters.json` (style: petition_road, petition_feast, petition_walls, petition_pressure_claim), `docs/70_CONTENT_GUIDE.md` (two sentences, fact then sense; two voices ask for different choices).

Result: all four drafts already obey every binding decision and every header 3.5 rule, so every text field is kept word for word. No effect change is proposed — decision 11 (나) concerns the second crossroads; each first-cross choice ② removes Society and so closes all four cards, and its one-off effect cannot stack. Each text was checked by script against every named list in main's schema plus `crossroads_words`, the 3.5 list and `shape_merit_words`, for digits, sentence count and template tokens; hits on lists that are not hooked on these rows are named per row.

Template tokens: only `{Civ}` and `{Civ}|hpp(은,는)` — both valid for `Subject Civilization` (main rule: a Civilization row names no system). Voice order as in main: `VoiceBodies[0]` the council, `[1]` the elders of the congregation; `Body` stays empty (voiced card).

## `petition_cross_built` — C01 · 작업장에서 티끌 같은 손으로

- Conditions: `Lottery` · `Subject Civilization` · `PlayerRoleIs Ruler` · `CivHasTrait Brick.Form.Society` · `CivTimeVectorAtLeast Built 1200` · weight 1 (unchanged)
- Effects (after the binding decisions):
  - ① Stability 8, Matter 240 · Insight 120 — success `AddTrait Brick.Instinct.SwarmTech`, `RemoveTrait Brick.Form.Society`, `ChangeStability -0.06`, `ShiftEthics Spiritualism -0.05` (council asked); failure `ChangeStability -0.06`, `ShiftEthics Spiritualism -0.05` (unchanged)
  - ② no check, Insight 100 — `RemoveTrait Brick.Form.Society`, `AddResource Matter 150`, `ShiftEthics Spiritualism +0.05` (elders asked) (unchanged — it closes all four first cards, so the one-off cannot repeat)
  - ③ no cost, no effect, no next card (unchanged)
- Voices: voice 1 council → ①, voice 2 elders → ②; Spiritualism as in main (council −0.05, elders +0.05, on failure too).

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

- why: No field changed. Every field passes verdict, holy_words, soul_verbs, rescue_words (main `petition_` rule 0 and the column `forbidden`), silence_words, rung_grasp_words, title_words, crossroads_words (header 3.5 / notes 5.4 데3), and the 3.5 list (구원·중보, 침묵·고요·잠잠·쉼·안식, 울타리·언약·방주, 목자·순례·성소, 불멸·영원·영겁, 신이 되·하나가 되·일체화); no digits; each line at most two sentences; no design name (SwarmTech, Built) on screen; the two voices ask for different choices (① / ②) and neither is written as right; the ① success line tells what the people did and where it then stands ('그 일이 끝났을 때 …'), the house pattern of all crossroads cards.
- why: Kept word for word: Title, VoiceBodies[0..1], every Label, ① SuccessText and FailureText, ② SuccessText, ③ SuccessText.

## `petition_cross_consumed` — C02 · 싸움의 광맥에서 별 사이의 체로, 또는 제 별의 화로로

- Conditions: `Lottery` · `Subject Civilization` · `PlayerRoleIs Ruler` · `CivHasTrait Brick.Form.Society` · `CivTimeVectorAtLeast Consumed 1500` · weight 1 (unchanged)
- Effects (after the binding decisions):
  - ① Stability 9, Energy 260 · Matter 120 — success `AddTrait Brick.Diet.DarkMatter`, `RemoveTrait Brick.Form.Society`, `ChangeStability -0.08`, `ShiftEthics Spiritualism +0.05` (elders asked); failure `ChangeStability -0.08`, `ShiftEthics Spiritualism +0.05` (unchanged)
  - ② no check, Matter 200 · Influence 100 — `AddTrait Brick.Diet.StarEater`, `RemoveTrait Brick.Form.Society`, `ChangeReverence Borders -0.1`, `ShiftEthics Spiritualism -0.05` (council asked) (unchanged)
  - ③ no cost, no effect, no next card (unchanged)
- Voices: voice 1 council → ②, voice 2 elders → ① (two turnings, no half step); Spiritualism as in main (council −0.05, elders +0.05, on failure too).

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
      "Label": "암흑 구간에 체를 편다",
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

- why: No field changed. Passes every rule listed under C01. Unhooked hit only: ② SuccessText '별빛' holds '빛' (dawn_words — hooked in main only on log_fallen_bound, project_binding, the pressure-endured events and the fifth-rung feat; not on the crossroads lists). The line is the neighbours' plain look (it carries `ChangeReverence Borders -0.1`), not a dawn picture; kept.
- why: The devouring branch ('끝없이 삼키는 살림') is not written as a good end: ② states the act and the neighbours' long look, no praise and no gain word. '암흑 구간' is a place in the sky, not the brick's design name; kept.
- why: Kept word for word: every field.

## `petition_cross_drifted` — C03 · 새 터를 닦던 배에서 배에 실린 살림으로

- Conditions: `Lottery` · `Subject Civilization` · `PlayerRoleIs Ruler` · `CivHasTrait Brick.Form.Society` · `CivTimeVectorAtLeast Drifted 900` · weight 1 (unchanged)
- Effects (after the binding decisions):
  - ① Stability 9, Energy 100 · Matter 300 — success `AddTrait Brick.Space.Nomad`, `RemoveTrait Brick.Form.Society`, `ChangeStability -0.1`, `ShiftEthics Spiritualism +0.05` (elders asked); failure `ChangeStability -0.1`, `ShiftEthics Spiritualism +0.05` (unchanged)
  - ② no check, Matter 140 — `RemoveTrait Brick.Form.Society`, `ChangeStability +0.05`, `ShiftEthics Spiritualism -0.05` (council asked) (unchanged — closes the card)
  - ③ no cost, no effect, no next card (unchanged)
- Voices: voice 1 council → ②, voice 2 elders → ①; Spiritualism as in main (council −0.05, elders +0.05, on failure too).

```json
{
  "Title": "닻을 내리는 곳",
  "Body": "",
  "VoiceBodies": [
    "{Civ}의 배들은 오래 새 터를 닦으러 나가 있었다. 의회는 떠나려는 집은 보내고 남으려는 집은 남게 두자고 청했다.",
    "{Civ}의 배들은 오래 새 터를 닦으러 나가 있었다. 회중의 장로들은 닿는 곳마다 집이 함께 가야 한다며 살림을 통째로 배에 싣자고 청했다."
  ],
  "Choices": [
    {
      "Label": "살림을 통째로 배에 싣는다",
      "SuccessText": "{Civ}에서 배에 사는 집이 뭍의 집보다 많아졌다. 그 일이 끝났을 때 그들은 닻을 내린 곳을 집이라 부르지 않고 있었다.",
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

- why: No field changed. Passes every rule listed under C01. Unhooked hits only: '떠나' (witness_words — hooked only on enc_ rows that wait on Hazard.Claim), '함께' in voice 2 (stay_words — extinction rows only), '남은' in ② SuccessText ('뭍에 남은 집들' — shape_merit_words is hooked on the `Shape.` prefix only, header 3.2; here it is the plain fact of who stayed ashore, not merit).
- why: Nothing says a star moves (C03 귀결) and no ark or pilgrimage word appears (방주·순례), though the line carries households on ships.
- why: Kept word for word: every field.

## `petition_cross_isolated` — C04 · 저희끼리 지낸 세월에서 빗장 건 항구로

- Conditions: `Lottery` · `Subject Civilization` · `PlayerRoleIs Ruler` · `CivHasTrait Brick.Form.Society` · `CivTimeVectorAtLeast Isolated 1500` · weight 1 (unchanged)
- Effects (after the binding decisions):
  - ① no check, Matter 200 · Influence 150 — `AddTrait Brick.Space.ClosedPort` (draft leaf `HermitMonolith`; the rename is `rename_leaves.py`'s (notes 5.4 코7), not a change of this pass), `RemoveTrait Brick.Form.Society`, `ChangeReverence Borders -0.1`, `ShiftEthics Spiritualism +0.05` (elders asked)
  - ② no check, Influence 80 — `RemoveTrait Brick.Form.Society`, `AddResource Insight 120`, `ShiftEthics Spiritualism -0.05` (council asked) (unchanged — closes the card)
  - ③ no cost, no effect, no next card (unchanged)
- Voices: voice 1 council → ②, voice 2 elders → ①; Spiritualism as in main (council −0.05, elders +0.05, on failure too).

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
      "SuccessText": "{Civ}의 항구들이 닫히고 새 터로 떠날 배들이 항구에 묶였다. 그 일이 끝났을 때 그들은 제 별 안쪽만 짓고 있었다.",
      "FailureText": ""
    },
    {
      "Label": "큰 항구만 남기고 닫는다",
      "SuccessText": "{Civ}의 작은 항구들이 닫히고 큰 항구만 열려 있었다. 바깥으로 쓰던 손이 안을 짓는 셈으로 돌았다.",
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

- why: No field changed. Passes every rule listed under C01. Unhooked hits only: '묶였다' in ① SuccessText (binding_words — hooked only on log_far_ascent and project_binding), '떠날' (witness_words, enc_ claim rows only). The ① line is the literal fact of ships kept at their quays.
- why: The closing is told as what the people did and where it then stands ('제 별 안쪽만 짓고 있었다'); nothing promises shelter or immunity (no new immunity — C04 귀결), and the elders' reason ('지켜 온 것이 흐려진다') stays in their own voice.
- why: Kept word for word: every field.

## Notes outside this set (for the lead)

- `petition_cross_law_stays` (C10): the binding new title '거두지 않은 칙령' (notes 5.4 데2) holds '거두', which is in `soul_verbs`; main's `DT_Encounters` rule 0 (`row_prefix petition_` → Title, Body, VoiceBodies, Labels, Success/FailureText lack soul_verbs) rejects it, so `python -I Tools/check_data.py` would fail on that row. A replacement that keeps the fix's intent (no 왕좌·다스리, no empty seat) and matches its new ① SuccessText ('…정해 둔 칙령을 손대지 않은 채 두었다…', which passes): '손대지 않은 칙령' (or '그대로 둔 칙령').
- `enc_cross_drift_fleet` (C12) is not covered here. Its Body ends '떠도는 살림이 오래되면 제 별까지 데리고 간다고 기록은 적는다' — a forecast of the star-tow branch in the record frame; whoever drafts C12 should check it reads as the record's claim and not as a promise the card keeps.
