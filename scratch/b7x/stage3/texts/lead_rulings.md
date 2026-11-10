# [B-7x4] 글 감사 — 리드의 판정 (2026-11-10)

`audited_texts.md` §8.3이 리드에게 남긴 열린 점을 정한다. 이 파일이 `audited_texts.md`보다 앞선다 — 둘이 다르면 이 파일을 따른다. 개발자는 무엇이든 뒤집을 수 있다(보고에 적는다).

## 1. 목소리 기울기와 닫지 않는 결과 — 감사의 §6.1을 좁힌다

감사의 §6.1('기울기는 카드를 닫는 결과에만 남는다')은 받지 않는다. 그 규칙은 리드가 감사 지시에 '윤리 이동도 쌓이는 이득'이라고 덧붙인 글에서 나왔는데, 헤더 결정 11 (나)의 받은 글은 그보다 좁다 — '②는 값과 작은 효과만 — `petition_road`·`petition_feast`의 꼴'. main의 그 두 행은 되풀이되는 `Lottery` 청원이고 두 선택 모두 `ShiftEthics Spiritualism ±0.05`를 지닌다(main `Tools/data/DT_Encounters.json` — `petition_road` ② `ChangeStability +0.05`·기울기 +0.05, `petition_feast` ② `ChangeStability +0.08`·기울기 +0.05). 그래서:

- **남긴다**: aa09543의 목소리 기울기(`ShiftEthics Spiritualism ±0.05`)는 모든 자리에 그대로 — 첫 갈림 C01~C03 ①의 실패, 둘째 갈림 C05~C08 ①의 실패, C05~C08·C11 ②, C09 ②의 실패. 설계서 §4.3 '공통'의 관례 문장도 그대로다.
- **뺀다**(결정 11 (나) — road/feast 꼴 밖의 쌓이는 것): 둘째 윤리 축(C07 ②·C11 ②의 `ShiftEthics Openness`), 인구(C05 ②의 `ChangePopulation +0.03` → `ChangeStability +0.05`), 높아짐과 우러름(C10 ②의 `PlayerStatDelta Exaltation +1`·`ChangeReverence +0.1`), 그리고 C10 ②의 기울기(노트 §5.4 데2 — 다스리는 이를 높이는 일은 윤리 축을 옮기지 않는다).
- **방랑자 C12 ②의 `ChangeReverence +0.05`는 남긴다.** 결정 11 (나)는 통치자의 둘째 갈림 청원의 글이다. main의 방랑자 카드는 판정 선택에 우러름을 준다(`enc_witness` ① +0.1, `enc_plague_tend` ① +0.3 — 다시 들어갈 때마다 오는 `Standing` 카드). 우러름은 방랑자가 왕좌로 가는 설계된 길이다. 그 몫은 헤더 §5 판단 ③′와 노트 §5.4 데5가 잰다. ② 성공 줄의 둘째 문장은 aa09543의 '{Civ}의 일꾼들이 당신의 손을 기억했다.'로 되돌린다(감사의 '반쯤 지은 배 가운데 한 척이 마저 지어졌다.'는 효과를 비웠을 때의 줄이었다).

바뀐 효과(나머지 칸은 aa09543 그대로, 잎 이름은 `rename_leaves.py`):

```json
{
  "petition_cross_dust_band.Choices[1].OnSuccess": [
    {"Type": "ChangeStability", "Tag": "", "Key": "None", "Value": 0.05, "DurationTicks": 0},
    {"Type": "ShiftEthics", "Tag": "", "Key": "Spiritualism", "Value": 0.05, "DurationTicks": 0}
  ],
  "petition_cross_between_stars.Choices[1].OnSuccess": [
    {"Type": "ChangeStability", "Tag": "", "Key": "None", "Value": 0.04, "DurationTicks": 0},
    {"Type": "ShiftEthics", "Tag": "", "Key": "Spiritualism", "Value": -0.05, "DurationTicks": 0}
  ],
  "petition_cross_law_stays.Choices[1].OnSuccess": [
    {"Type": "ChangeStability", "Tag": "", "Key": "None", "Value": 0.05, "DurationTicks": 0}
  ],
  "petition_cross_seam.Choices[1].OnSuccess": [
    {"Type": "AddResource", "Tag": "", "Key": "Insight", "Value": 120.0, "DurationTicks": 0},
    {"Type": "ChangeStability", "Tag": "", "Key": "None", "Value": -0.02, "DurationTicks": 0},
    {"Type": "ShiftEthics", "Tag": "", "Key": "Spiritualism", "Value": 0.05, "DurationTicks": 0}
  ]
}
```

aa09543의 효과 칸에 적힌 기울기의 부호(의회 −0.05, 장로들 +0.05)는 그대로 쓴다 — 위 JSON의 부호가 aa09543과 다르면 aa09543이 이긴다(옮기는 이가 확인한다). C05 ②의 `ChangeStability +0.05`와 C11 ②의 `AddResource Insight 120`은 자리값이다 — 헤더 §5 ⑧(`-Crossroads`)이 잰 뒤 `[B-7x5c]`나 `[B-7x5r]`이 정한다. 감사의 §6.3 첫 항목(설계서 '공통'의 문장을 '닫는 결과에서만'으로 고침)은 하지 않는다; 나머지 §6.3 항목은 한다(C05·C07·C10·C11 ② 줄을 위 효과에 맞춘다).

## 2. 나머지 열린 점

| 점 | 판정 | 까닭 |
|---|---|---|
| C10 제목 | '손대지 않은 칙령'을 받는다 | 받은 답의 '거두지 않은 칙령'은 `words.soul_verbs`의 '거두'에 걸려 main의 청원 규칙 0이 행을 거절한다(`Tools/cosmic_data_schema.py` 부분 문자열 비교). 뜻(빈 자리·왕좌 그림 없이 칙령을 둔 일)은 같다. 헤더 §11과 노트 §5.4 데2·설계서 §4.3 C10에 이 벗어남을 적고 보고에서 개발자에게 알린다. |
| 꼴 낱말 여섯 | 감사의 낱말을 받는다(§3 표의 마지막 값: `ConsumedOrDrifted` '밖으로 뻗은 살림', `SpreadBody` '별 사이에서 걸러 사는 살림', `FoldedSmall` '작은 눈금 속으로 접힌 살림', `ShutHouse` '항구를 닫아건 살림', `KeptLaw` '칙령을 쉽게 바꾸지 않는 살림', `LastHunger` '별의 불을 끌어 쓰는 살림') | 헤더 §3.2는 '초안, 글 감사가 정한다'고 맡겼다. 보고에서 알린다. |
| `ShutHouse` | '항구를 닫아건 살림' | 감사의 고른 쪽. |
| C12 제목 | '집을 싣는 배' | 헤더 §3.5 — 카드 종류의 설계 낱말('갈림')은 화면에 내지 않는다. 비문에 나와도 떠남·죽음으로 읽히지 않는다. |
| C08 ① 이름 | 감사대로('가장 작은 눈금 속으로 접혀 든다') | T-09를 선택 이름까지 맞춘 것. |
| `log_brick_scan_spread` | 변형 셋 | main 관례(`Templates`는 셋 이상). `single_variant_rows`에 넣지 않는다. |
| `shape_merit_words`에 '남긴' | 더하지 않는다 | 바뀐 `KeptLaw` 낱말에 '남긴'이 없다. |
| `KeptLaw` DevComment | 감사의 글(코1 코드에 맞춘 것) | 코드가 이긴다. |
| 헤더 §2.4 48행·노트 데3의 커밋 갈림 | `[B-7x4]`·`[B-7x5c]`에서 감사의 §6.3대로 | — |
