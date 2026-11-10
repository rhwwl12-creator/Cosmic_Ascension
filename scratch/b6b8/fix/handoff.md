# [B-6b8] 리뷰 고침 단계 → 다음 단계(변이 → 마지막 러너 → 묶음 B → 기록)에 넘김 (2026-11-10)

리뷰 발견 17건(확인 10, 일부 7, 반박 0)을 모두 반영했습니다. 커밋과 푸시는 하지 않았습니다. STATUS.md와 docs/OVERVIEW.html은 건드리지 않았습니다.

## 1. 다음 단계가 꼭 지킬 것

1. **묶음 B는 `--w-all-cells`로 돌립니다**(L2-1). 모든 칸의 짝 다리를 T_B = min(T+1, E, W)로 둡니다. 명령의 예:
   `python -I -X utf8 <scratchpad>/b6b8/bundleB.py --t-death 5000 --r 0.375 --w-all-cells ...`
   - 까닭은 N §5 `[B-6b8]` ①에 적었습니다. 커맨드렛 CSV(MetCivs, FirstContactTick, LogLen 등)와 `_rows.csv`가 앎과 이야기 층을 읽기 때문입니다.
   - 시드 42 탐침의 ⓑ도 같은 규칙으로 고쳤습니다(`probe/probe.py:137`, 모든 판 min(E, W)). 다시 읽은 결과는 그대로였습니다.
2. **H §11의 `[B-6b8]` 줄**(기록 단계가 씁니다)에서 지침서 §6 초안의 "min(첫 셋째 칸+1, 시대 줄[, 방랑자 칸은 증언 카드])"를 다음으로 바꿉니다.
   - "칸마다 min(첫 셋째 칸+1, 시대 줄, 증언 카드)의 앞(노트 ⓗ는 첫 셋째 칸 — CSV가 앎을 읽는다)"
   - 400바이트 아래를 지킵니다. H는 지금 148,283바이트이고, 이번 고침에서 H는 건드리지 않았습니다.
3. **N §5 `[B-6b8]` 블록**은 이번 고침 단계가 처음 열었습니다(N 끝).
   - 들어 있는 것:
     - 「헤더와 다르게 하거나 더한 것」의 ①과 ⑦
     - ⑧의 한 마디: (나)의 절멸은 결박 틱까지 셉니다. 결박이 없으면 기록 틱까지입니다.
     - M7과 M5의 벗어남, `[B-6b7]`이 넘긴 둘(L4-2, L4-3)의 적는 방식
     - 다시 돌릴 목록을 읽은 방식
     - 일의 차례 변경(리드의 결정)
     - 「리뷰」 줄
   - 다음 단계가 채울 것:
     - ⑧ H:343의 읽기 전체. 묶음 B 전에 시각과 함께 적고, 위 한 마디와 맞춥니다.
     - ⑩ 얼린 셈
     - 나눔·데이터·스키마·코드·테스트·변이 수·기준값·묶음
4. **변이 단계가 돌릴 목록**: `scratchpad\b6b8\rerun_set.txt`를 다시 만들었습니다(L3-3).
   - 옛 패치 129개: C++ 53, 탐침 2, 파이썬 74
   - 새 `b6b8_` 패치 17개
   - 다시 뜬 옛 패치 셋은 주석으로 옮겼습니다.
   - 새로 더한 C++ 쪽 셋은 **아직 돌리지 않았습니다**. 변이 단계가 맡습니다.
     - `b6b8_wanderer_nocheck_reads_next_choice` → `Cosmic_Ascension.Sim.WitnessCard`
     - `b6b8_wanderer_nocheck_inverted` → `Cosmic_Ascension.Sim.WitnessCard`
     - `b6b8_data_era_no_ruled_past_checker` → `Cosmic_Ascension.Sim.EraEndured`. 에셋 패치라 적용 뒤와 되돌린 뒤에 `make_data_tables`로 에셋을 다시 만들어야 합니다. 파이썬 도구는 지나가는 것을 확인했습니다(교훈 2).
     - 살아남으면 패치는 그대로 두고 테스트를 고칩니다.
   - 파이썬 옛 패치는 B3, B4 폴더에도 있습니다. `scratchpad/b6b/run_py_mutations.py`는 `Tools/mutations/B6b/` 접두어를 박아 두어서 이것들을 돌리지 못합니다.
     - 대신 `scratchpad\b6b8\fix\run_py_paths.py`를 씁니다. 프로젝트 밖 git 복사본에서 돌리므로 CRLF 걱정이 없습니다.
     - 쓰는 법: `python -I -X utf8 run_py_paths.py <root> <work dir> kill @<목록 파일>`. 목록 파일은 `rerun_set.txt`에서 `=python:` 줄만 뽑은 것입니다.
     - 에셋 패치는 `pass`로 확인합니다.
5. **`b6b8_cmdlet_witness_first_any_card`**는 여전히 변이 단계에서 `--max-seed 2` 방랑자 스모크로 확인할 몫입니다. 탐침 보고의 넘김 그대로입니다.

## 2. 무엇이 바뀌었나(프로젝트 트리)

- **글과 데이터**
  - `Tools/data/schema.json`: `witness_words` 끝에 15개를 더했습니다.
    - -ㄴ·-ㄹ 꼴 10개: 말한, 말할, 전한, 전할, 기록한, 기록할, 떠난, 떠날, 지나간, 지나갈
    - -ㅁ 꼴 5개: 말함, 전함, 기록함, 떠남, 지나감
  - `Tools/test_data_schema.py`: 목록 고정을 고치고, 깨지는 경우 넷을 더했습니다(검사기 테스트 96개 그대로).
  - `Tools/data/generated.sha1`: 러너 `-Data`가 다시 썼습니다(schema.json 도장). 에셋 sha1은 아래 §3에 적었습니다.
- **문서**
  - `docs/21`: :263, :609, 증언 낱말 괄호(:1266)
  - `docs/10_DESIGN/systems.md:5`
  - `docs/70:137`: 세 곳
  - `docs/40`: `Sim.EraEndured`, `Sim.WitnessCard` 행의 변이 이름
- **테스트**
  - `Source/Cosmic_Ascension/Tests/CosmicSideRowTests.cpp`: 경계마다 `NoCheckByRow` 단언 하나를 더하고 주석 셋을 고쳤습니다. 새 파일이 아니므로 헤더는 바뀌지 않았습니다.
- **변이**(`Tools/mutations/`)
  - 새 패치 넷: 위 셋과 `b6b8_schema_witness_adnominal_forms_dropped`(파이썬, KILLED 확인)
  - 다시 뜬 패치 둘: `b6b8_schema_desolation_without_lamp`, `b6b8_schema_witness_past_forms_dropped`. 둘 다 KILLED를 확인했고, 바뀐 줄은 같고 문맥만 달라졌습니다.
  - `README.md`: 새 행 넷을 더했고, 5행의 에셋 목록에 `b6b8_data_era_no_ruled_past_checker`를 넣었습니다.
- **N**: §5 `[B-6b8]` 블록을 열었습니다(위 1-3).

## 3. 다시 돈 것

- 수정한 파이썬 쪽: `python -I Tools/check_data.py`는 0건, `Tools/test_data_schema.py`는 96개 OK입니다.
- 파이썬 변이 넷은 프로젝트 밖 복사본에서 돌렸습니다.
  - KILLED 셋: `b6b8_schema_desolation_without_lamp`, `b6b8_schema_witness_past_forms_dropped`, `b6b8_schema_witness_adnominal_forms_dropped`
  - 검사기 통과 하나: `b6b8_data_era_no_ruled_past_checker`
  - 기록: `fix/pymut_*_out.txt`
- 러너 `-Tier smoke -Data -UpdateBaseline`을 한 번 돌렸고 `run_checks: all passed`가 나왔습니다(`fix/runner_smoke_fix.txt`).
  - smoke 두 판 모두 20/20이고 실패는 0입니다.
  - 에셋 12개는 바이트 그대로입니다. `generated.sha1`은 schema.json 줄만 바뀌었습니다(5acd4fb… → a30cac2…).
  - 시드 42의 세 줄과 해시 판 둘은 기준값 그대로입니다.
  - `seed42.json`은 `fix_seed42.py --from-on --final`로 모양을 돌린 뒤 HEAD와 바이트까지 같습니다.
- smoke 밖 테스트를 필터로 한 번 돌렸습니다: `Sim.WitnessCard`, `Sim.EraEndured` 둘 다 통과입니다(`Saved\Logs\TestsFix.log`).
- 탐침 단계 2·3을 다시 돌리지 않은 까닭:
  - 고친 것은 검사기 규칙 파일(게임은 읽지 않음), 테스트 단언 하나, 문서, scratchpad 하네스뿐입니다. 에셋과 시뮬 코드는 바이트 그대로입니다.
  - 위 러너의 커맨드렛 견줌이 세 줄과 해시 판을 다시 확인했습니다.
  - 대신 `probe.py check`를 고친 ⓑ로 다시 읽었습니다. ⓐⓑⓒ는 그대로 통과했고 `probe_out.json`은 바이트 그대로입니다.
  - `smoke2`도 `--check-only --w-all-cells`로 다시 읽었습니다(PASS).
- `b6b9/readings_1.1-1.5.md`의 sha1은 5891b075…로 그대로입니다.
- 짝을 짓는 C++ 변이 셋은 이 단계에서 빌드하지 않았습니다(변이 단계의 몫).

## 4. 저장소 밖(scratchpad) 변경

- `bundleB.py`: M1-M7을 반영했습니다(자기 시험 145/145). 고친 셈을 옛 하네스에 걸면 깨지는 것을 `fix/old_vs_new.py`로 보였습니다.
  - 지금 `smoke2`를 `--check-only --w-all-cells --summary-dir fix/smoke2_check`로 다시 읽으면 PASS가 나옵니다. 짝은 8/8이고, ②의 가름은 같습니다: (나) 1, T4가 결박 뒤.
  - 새 출력 자리:
    - ③ 아래의 '잰 계절의 길이를 법의 끊김으로 나눔'
    - 적기만의 'T4 civs at the record'
    - '#96 … R: 재지 않음(폐위·죽음 0)'
    - '다스리는 셋째 칸의 일감 R / slowR'
    - 적기만 머리의 '같은 세계' 문장
  - N §3 '묶음 B'에 옮길 때 '재지 않음'의 까닭도 함께 적습니다.
- `bundleB_selftest.py`: 새 확인 10개가 들었습니다.
- `rerun_set.py`, `rerun_set.txt`: L3-3 반영
- `probe/probe.py`: L2-1의 ⓑ 한계. `probe_out.json`은 그대로입니다.
- 고치기 전 사본은 `fix/*.before.*`에 두었습니다.
- **[B-6b9]에 넘김**: `scratchpad\b6b9\bundleB.py`는 이 하네스의 사본입니다. 같은 결함이 그대로 들어 있습니다.
  - :640 판 전체 erased_ruled
  - :872 vessel_left 합
  - A.dist의 평균 중앙값
  - :943 left_at만 읽음
  - [B-6b9] 폴더는 고치지 않았습니다(금지). [B-6b9]가 이 단계의 고침(M2-M5)을 같은 꼴로 옮겨야 합니다.
- `b6b9/readings.sha1`은 확인한 결과 그대로입니다(아래 §3).

## 5. 열린 것

- 증언 카드는 실제 데이터(탐침 5판, 스모크 18판)에서 아직 한 번도 오지 않았습니다. `first at`, `without a check`, W 대조는 묶음 B가 처음 보여 줍니다(#141의 n).
- (L4-2) 법이 빈 틱은 '재지 않음'입니다. race 줄에도 CSV에도 그 값이 없습니다. 다스리는 넷째→다섯째 칸 안의 끊김을 따로 재려면 커맨드렛에 셈이 필요합니다. [B-6b9] 이후에 정할 일입니다.
- 선택 사항으로 남긴 것:
  - M7의 '다리부터' 줄 수(`_rows.csv` 기반, 사건 수가 아니라 줄 수)
  - M6의 짝마다 'T4 civs at the record'는 넣었습니다.
