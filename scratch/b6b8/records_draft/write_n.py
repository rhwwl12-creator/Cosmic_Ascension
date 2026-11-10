# [B-6b8] records stage: N section 3 'bundle B' skeleton and the N section 5 [B-6b8] block (placeholders <<B:NAME>>).
# Keeps N lines 878-879 (items 8 and 10, frozen sha1 43e54e44...) and 880 (review) byte for byte.
import hashlib, sys

ROOT = r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension"
N = ROOT + r"\docs\60_DECISIONS\headers\B6b_upper_rungs_and_the_race_notes.md"

raw = open(N, "rb").read()
assert b"\r" not in raw
text = raw.decode("utf-8")
lines = text.split("\n")
assert lines[-1] == ""

def find(prefix, start=0):
    hits = [i for i in range(start, len(lines)) if lines[i].startswith(prefix)]
    assert len(hits) == 1, (prefix, hits)
    return hits[0]

# --- the frozen pair
i8 = find("- **⑧ H:343의 읽기 — 판단 ②**")
i10 = find("- **⑩ 얼린 셈**")
assert i10 == i8 + 1
frozen = ("\n".join(lines[i8:i10 + 1]) + "\n").encode("utf-8")
assert hashlib.sha1(frozen).hexdigest() == "43e54e441f59560031a662b77d686225c3556cd7"
i_review = i10 + 1
assert lines[i_review].startswith("- **리뷰**(ultracode — 구현 워크플로: 구현 일꾼 셋")
assert i_review == len(lines) - 2

# --- section 3 'bundle B'
ib = find("### 묶음 B (`[B-6b8]`")
assert lines[ib + 1] == "" and lines[ib + 2].startswith("(비어 있다 — `[B-6b8]`이 채운다")

S3 = [
"(2026-11-10, `[B-6b8]`의 측정 단계 — 빌드: HEAD `220129d` 위의 `[B-6b8]` 작업 트리, 마지막 러너(`-Tier full -Data -UpdateBaseline`)가 돈 빌드 그대로(변이 단계 끝의 `Rebuild.bat` — DLL sha1 `07204afab949`, 하네스가 지키는 14 파일의 sha1은 `scratchpad\\b6b8\\final_build_sha1.txt`); 프로세스 <<B:B_PROCS>> = 묶음 540(재실행 <<B:B_RERUNS>>) + 시드 42 탐침 15(ⓐ 5는 `Saved\\Balance\\b6b8_off`, base 5는 `b6b8_base` = `[B-6b7]`의 `b6b7_on`, on 5는 `b6b8_on`); 두 번 실패한 판 <<B:B_FAILED_TWICE>>; 폴더 `<<B:B_FOLDER>>`(OneDrive 밖, 지침서의 자리는 `scratchpad\\b6b8\\bundleB\\`)와 그 사본 `Saved\\Balance\\b6b_bundleB\\`(모든 `.log`와 출력 전부의 sha1 목록 — §5 `[B-6b8]` ⑨); `end check`·`race check`·`LogCosmic: Error` <<B:B_CHECKS>>; 스위치·필드 대조 <<B:B_CONTROLS>>(끈 쪽 로그마다 '`rows taken out [feat_passage_by_ark feat_laws_and_binding project_ark project_binding crisis_species_extinction edict_wide_machine_accord edict_wide_quarantine edict_wide_warning event_era_endured enc_witness], AIUpperRungProgressMultiplier 1 (was 0.375), FleetDelayCountdownShare 0 (was 0)`' 한 줄, 끈 쪽 시대·증언·절멸·AI 걸음 조약·AI 넷째 칸 0, 기록의 종류에 `Clock` 없음·모든 CSV가 20,000에 닿음, 증언 첫 틱 ≥ 0 ⇔ 증언 카드 > 0, 방랑자 칸 W ≥ `left at`, W ≥ 그 판의 첫 주장(<<B:B_CLAIMS_CONTROL>>), E ≥ 600 ⇔ 시대 줄 > 0); 짝 <<B:B_PAIRS>>(켬/끔, Tick < T_B = 칸마다 min(첫 셋째 칸 + 1, 첫 시대 줄, 첫 증언 카드)의 CSV와 `_rows.csv`가 같다 — `TickMicros`·`SaveBytes`·`SaveRawBytes` 밖; §5 `[B-6b8]` ①), 같은 빌드의 다리 <<B:B_BRIDGES>>(강제 죽음 < 5,000·방랑자 < 1,200을 R 켬과); sha1 지킴 <<B:B_SHA1>>(DLL과 DT 에셋 열둘·`generated.sha1`이 묶음 앞뒤로 같다); 6 작업자 <<B:B_MINUTES>>분(시작 단계가 묶음 A로 잡은 약 16.5분, 탐침의 `--max-seed 2` 스모크 18판의 판당 12.8초로 잡은 약 19분 — 지침서의 35~42분은 너무 컸다). 지평 20,000, 모든 판 `-Rules=ProvisionalEndTick:0`, 고향 은하 행, 시드 1~60, 칸 다섯 — R(`-Answer=1 -Build=1 -Focus=1 -Edict=1 -Diplomacy=1 -TellAll=0`), 느린 R(R에 `-Focus=0`), 방랑자(R에 `-Abdicate=1200 -Wander=1`), 입력 없음, 강제 죽음(R에 `-DeathAt=5000`, 켬만 — §5 `[B-6b8]` ②); 앞의 넷은 켬과 `-WithoutB6bData=1`의 짝. 하네스 `scratchpad\\b6b8\\bundleB.py --t-death 5000 --r 0.375 --w-all-cells`(자기 시험 145/145; 판단의 셈은 §5 `[B-6b8]` ⑩ 그대로). 몫마다 Wilson 95% 구간.)",
"",
"- **① 다섯째 칸의 기록(R 60)**: J1 = 기록이 다스리는 백성의 끝이나 다른 질서이고 0 ≤ 그 틱 ≤ 20,000인 판 <<B:J1>>/60 — <<B:J1_VERDICT>>(문턱 40/60; 두 번 실패한 판은 닿지 않은 판).",
"- **② 다른 질서의 몫(R, ①의 판)**: n₂ <<B:N2>>, J2 = <<B:J2>> — <<B:J2_VERDICT>>(점이 15~35% 안). 가름(분자의 판만 — (가) 결박한 백성이 다스리는 백성이 아니고 그 틱의 다스리는 칸 ≥ 4, (나) 다스리는 백성이 결박 틱까지 절멸로 지워졌거나 다스리는 넷째 칸이 없거나 결박 뒤, (다) 그 밖): <<B:SPLIT>>; 다스린 적 있는 백성의 다른 질서(분모에 둠): <<B:ONCE_RULED>>. 읽기는 §5 `[B-6b8]` ⑧(묶음 전에 얼림 — `[B-6b9]`의 몫). 느린 R(적기만): <<B:J2_SLOW>>.",
"- **③ 다스리는 넷째 → 다섯째 칸(R — 기록이 다스리는 백성의 끝이고 그 백성 = `RuledCivID[1]` = `RuledCivID[2]`, r5 ≤ 기록 틱; n₃ <<B:N3>>)**: 길이 중앙값(짝수 개면 아래 가운데) <<B:J3_LEN>>(문턱 3,600 이하); 밀도 — r4부터 끝이 r5 이하인 600틱 온 창마다의 결정 수, 판마다 중앙값 ≥ 1인 판 <<B:J3_DENSITY>>(문턱 90%); r5 − r4 < 1,200인 판 <<B:J3_VIOLATIONS>>; 온 창이 0인 판 <<B:J3_ZERO>>; 따로 적은 판(다른 재위·r5 전의 기록 등) <<B:J3_APART>> — <<B:J3_VERDICT>>. `[B-6b7]`의 넘김(L4-2 — 넷째 칸 계절을 모형이 아니라 잰 값으로): 잰 길이를 법의 끊김 수로 나눔 <<B:J3_BY_RESETS>>(끊김 수는 race 줄의 `resets ruled` — 판 전체, 다섯째 칸 뒤도 든다); 끊김마다 법이 빈 틱은 재지 않음 — race 줄에도 CSV에도 법을 쥔 열이 없다.",
"- **④ 시계 C(느린 R 60)**: E_i = 기록이 다스리는 백성의 끝이나 다른 질서면 그 틱, 아니면 ∞ — <<B:E_LIST>>; C₀(올림차순 48번째 E를 1,000으로 올림) <<B:C0>>, C = min(18,000, max(6,000, C₀)) <<B:C>>; (i) #{E ≤ C} <<B:J4_I>>(문턱 30); (ii) 다스리는 셋째 칸부터 min(기록 틱, C)까지 600틱 온 창의 판마다 중앙값 ≥ 1인 판 <<B:J4_II>>(문턱 90%; 셋째 칸이 끝 이상인 판은 닿지 않음으로 빠진다) — <<B:J4_VERDICT>>. 이 커밋은 C₀·C·(i)·(ii)만 적고, 규칙을 `DECISIONS_LOG`에 옮기는 것은 `[B-6b10]`(C가 6,000이면 노브를 둔다 — 헤더 §5.3).",
"- **적기만**(헤더 §5.3·노트 §2.3 B — 켬 / 끔 / 켬 − 끔; 칸의 수는 판 전체라 방랑자 칸(1,200 아래)·강제 죽음 칸(5,000 아래)의 켬 쪽은 R과 같은 세계의 앞부분을 다시 센다 — '같은 세계는 한 번만'을 하지 않은 까닭은 §5 `[B-6b8]`의 '그 밖에'):",
"  - 넷째 칸 이상 백성 수(기록 때·끝·최대)와 제동에 걸린 AI 틱: <<B:REC_T4>>",
"  - 절멸(다스리는 백성·AI에 놓임, 방주 완성, 넷째 칸 전에 잃은 방주, 만료, 은혜로 남음, 까닭마다 지워진 백성, 다스리는 이의 죽음): <<B:REC_EXTINCTION>>; 방주를 둔 채 만료 — 재지 않음(카운터 없음 — §5 `[B-6b8]` ⑤)",
"  - 결박(틱, 결박한 백성, 결과 없이 끝, 잠김, 되찾음, 결박 전에 떠난 그릇): <<B:REC_BINDING>>",
"  - #162(다른 질서가 보였나, 어느 생애가 다스린 백성인가): <<B:REC_162>>",
"  - #156 시대 줄(줄이 선 판·첫 틱·기록까지, 칸마다): <<B:REC_156>>",
"  - #141 증언 카드(온 판·첫 틱·판정을 지남·실패·판정 없는 답·답 전에 사라진 카드, 칸마다): <<B:REC_141>>",
"  - AI 단계 µs/틱(러너의 직렬 판과 묶음의 켬 − 끔; B6a의 10.4~10.7보다 +50%를 넘으면 `[B-6b9]` 전에 까닭을 찾는다): <<B:B_AI_US>>",
"  - 잠김(R — `locked` ≥ 1이고 다섯째 칸 기록이 없는 판; 60판 가운데 3 이상이면 #165 `SealedByCivID`와 결정 13·14를 [개발자]에게 — 헤더 끝의 보완): <<B:B_LOCKED>>",
"  - 넷째 칸 이상에 백성을 두고 떠난 그릇(#96 — 방랑자 칸은 방랑자 줄의 `left at`, R은 calling 줄의 'reign ended first at'에서 그 백성의 CSV `Tier`, 그리고 'crown refused far above'): <<B:REC_96>>",
"  - 다스리는 셋째 칸의 일감(`[B-6b7]`의 넘김 L4-3 — 첫 AI가 고향 은하 법을 쥔 틱부터 다스리는 넷째 칸·기록·6,000 가운데 이른 틱까지, 600틱 창마다의 결정과 카드의 줄; R·느린 R): <<B:REC_WORKLOAD>>",
"  - B1 지킴의 미리 봄(판단이 아니다 — 방랑자 짝을 6,000에서 잘라 켬과 끔의 즉위; 읽기는 묶음 C, §2.3): <<B:REC_B1>>",
"  - 강제 죽음이 바꾼 기록(R 켬과 견줌)과 calling 줄의 'death asked at 5000 (recorded r)'의 r = 0인 판: <<B:REC_DEATH>>",
"  - E의 분포가 모형의 짧은 꼴인가(헤더 끝의 보완 — 그렇다면 §0.2의 후보 행을 연다): <<B:REC_SHORT_SHAPE>>",
]
lines[ib + 2:ib + 3] = S3

# --- section 5 [B-6b8] block (indices moved by the section 3 change)
i_head = find("**`[B-6b8]`**(2026-11-10 — 데이터 ③ 곁 행")
i_diff = i_head + 1
assert lines[i_diff].startswith("- **헤더와 다르게 하거나 더한 것**(고침 단계까지 — 번호는 지침서의 자리)")
old_diff = lines[i_diff]

lines[i_head] = "**`[B-6b8]`**(2026-11-10 — 데이터 ③ 곁 행: `event_era_endured`·`enc_witness`, 묶음 B와 헤더 §5.3의 판단 — 헤더 §8, 결정 20~22; 리뷰의 고침 단계가 열고 기록 단계가 채웠다 — 묶음 B의 수는 §3 '묶음 B'):"

BEFORE = [
"- **나눔**: 없음(헤더 §8의 `[B-6b8]` 한 커밋 그대로 — 지킴의 몫 `FleetDelayCountdownShare` 0.5는 다음 커밋 `[B-6b8b]`). 설계 워크플로가 쓴 지침서(저장소 밖)를 구현 일꾼 셋이 나눠 맡았다 — 데이터(행 둘·스키마의 규칙 둘과 목록 셋·검사기 테스트), C++(새 테스트 파일·걸림줄·정책의 셈·커맨드렛·변이 파일), 하네스(`scratchpad\\b6b8\\bundleB.py`와 판단의 셈, 자기 시험). 이어 빌드와 시드 42 탐침(러너 `-Tier smoke -Data -UpdateBaseline` 한 번이 빌드 판이다) → 관점 넷의 리뷰 → 반박 검증 → 고침 일꾼 → 변이(C++·에셋·커맨드렛·파이썬 — 묶음 B 앞, 아래 ⑬) → 마지막 러너 `-Tier full -Data -UpdateBaseline` → 묶음 B → 판단과 기록.",
"- **데이터**: 행 둘을 표의 끝에 — `DT_Events`의 `event_era_endured`(견딘 영압의 시대: 상시 행, 문명 주제, 분류 `Event.Civ.Society`·로그 `Log.Civ`·Severity 1, 조건 `SystemOwnedBy` `Ruled`와 `CivPressureEnduredTicksReached` 600 — 슬롯 10틱이면 셈 600~609의 한 슬롯에서만 참이라 견딤 하나에 한 번, 무게 0·식힘 0, 효과 `ChangeReverence` `Borders` +0.2(그 백성이 아니라 국경을 맞댄 백성마다; 이웃이 없으면 효과는 거짓을 돌려주지만 줄은 서고 에러 줄도 없다 — `ApplyAll`은 돌려준 값을 읽지 않는다), 줄 셋 물레·장독·느린 수레의 시대, `EpithetRow`를 쓰지 않음 — 12 → 13행)와 `DT_Encounters`의 `enc_witness`('그늘의 가장자리': 상시 행, 주제 `VesselSystem`, 조건 `PlayerRoleIs Wanderer`와 `PlayerInSystemWithHazard` `Hazard.Claim`, 무게 1, '그늘의 가장자리를 재어 둔다' 정신 판정 3 — 이기면 `ChangeReverence` +0.1(Key 없음 = 성계의 주인 백성, `enc_star_watch`의 꼴), 지면 효과 없음; '물러난다' 판정 없음·효과 없음; 비용 0, 목소리 없음, `ResolveHazard`·`DelayHazard`·`PlayerTierAtLeast` 없음 — 18 → 19행). 글은 §1.5의 C6·C7에서 글자 그대로 옮겼다(`scratchpad\\b6b8\\check_texts.py` — 줄 셋, 제목·본문·라벨 둘·성공 줄 둘·실패 줄 하나와 빈 줄; merged는 견줌에만). 카드가 오는 자리: 목소리의 격차가 1 이하라(`CosmicPetitions` `WithinWandererGap`) 주인이 첫째·둘째 칸인 땅에서만 오고, 성계 하나에 위기 하나이며 다른 방랑자 서 있는 카드 넷은 다른 위기를 이름하므로 후보가 되면 늘 홀로라 늘 뽑힌다(서 있는 후보가 있으면 도착 추첨을 건너뛴다); 답한 바로 뒤에는 다시 오지 않고(`LastAnswered`), 그릇이 선 성계에 주장이 내려앉아도 도착 없이 온다(틱 머리에 서 있지 않던 단위만 — `HeldAtHead`). 이름은 `CosmicB6bRows`에 두고 `CosmicRows::` 상수를 두지 않았다(게임 코드가 이름으로 부르지 않는다 — 슬롯과 `StandingHeld`가 표를 돈다; 상수는 거짓 `required_rows`를 부른다). `make_data_tables` 뒤 바뀐 에셋은 `DT_Events`·`DT_Encounters`의 `.uasset`과 도장 `generated.sha1`뿐이다(`ST_CA` 그대로 — 새 키 없음; `docs/30`은 노브가 그대로라 다시 짓지 않았다).",
"- **스키마**(`Tools/data/schema.json`·`Tools/test_data_schema.py` — 검사기 `Tools/cosmic_data_schema.py`는 그대로): 새 낱말 목록 셋을 `ark_words` 뒤에 — `witness_words` 43(증언·기록하다·전하다·말하다와 물러섬의 떠남·지나감·외면·도망·부끄러움; 글자 그대로 찾으므로 줄어든 과거꼴 '기록했'·'떠났'·'지나갔'과 리뷰가 더한 -ㄴ·-ㄹ 꼴 열·명사형 다섯을 따로 적었다), `sign_voice_words` 4(징조의 목소리), `witness_record_words`('기록'); 규칙 W(`DT_Encounters`의 끝 — `when`이 접두어 `enc_`와 `Hazard.Claim`을 이름한 조건이라 지금의 증언 카드와 뒤의 모든 주장 곁 방랑자 카드를 덮는다; `witness_words`를 글 칸 여섯에, `sign_voice_words`를 본문에, `witness_record_words`를 성공 줄 밖 다섯 칸에 — 시험 열둘)와 규칙 E(`DT_Events` — `CivPressureEnduredTicksReached`를 묻는 행은 `SystemOwnedBy` `Ruled`와 `ChangeReverence` `Borders`를 함께 지닌다, 시험 둘); 머리말(`about`)에 한 문장. 검사기 단위 테스트 95 → 96(새 `test_the_witness_words` — 세 목록을 그대로 고정, 규칙 W를 깨는 경우 열셋과 범위의 통과 경우, 규칙 E를 깨는 경우(`Ruled` 없음·`Borders` 없음·게임의 시대 행), 두 규칙이 거는 칸 열둘); 걷는 수 316 → 330(스키마에서 셌다). 규칙 E가 깰 옛 통과 경우 셋 — `event_test_harvest`의 상시 경우, `[B-6b5]` 시대 낱말 경우의 준비, 규칙 준비 목록의 시대 규칙 — 에 `SystemOwnedBy Ruled`와 `ChangeReverence Borders 0.2`를 주었다(`expect`는 줄의 낱말을 모두 지닌 문제 줄이면 통과라 깨는 경우는 고치지 않았다). `check_data` 0.",
"- **코드**(시뮬 코드·저장·해시는 그대로): `World/CosmicWandererPolicy.h`의 `FCosmicWandererPolicyCounts::NoCheckByRow`(측정만 — 저장·해시 밖, 놀이가 읽지 않는다)와 `.cpp`가 답의 수로 고른 선택지(`Answers % Choices`)의 `CheckStat`을 셈이 넘어가기 전에 읽어, 받아들인 답 가운데 판정 없는 답(물러남)을 행마다 센다; `Tests/CosmicBalanceCommandlet.cpp`의 `WitnessFirst` — 입력 블록에서 기록의 문 밖, 증언 카드의 행별 셈이 오른 첫 경계(정책은 그릇이 걷는 동안 `-Wander`와 상관없이 카드를 세므로 답이 없는 판에서도 선다 — 짝의 다리 W)와 race 줄의 증언 꼴 `witness cards n (first at t, by the record n; answered n: by the check n, failed n, without a check n; gone before the answer n)`(옛 `succeeded/failed/withdrawn`의 `succeeded`는 판정을 지난 답과 물러남을 섞었고 `withdrawn`은 답 전에 사라진 카드였다 — `docs/40`의 race 줄 문단); `Tests/CosmicB6bRowNames.h`의 주석 둘('in the data from [B-6b8]'). 추첨·상태 없음; 헤더를 고쳐 두 타깃을 다시 지었다.",
"- **테스트**(자동화 테스트 223 → 225; 마지막 러너의 두 판 450 Success·실패 0; 검사기 단위 테스트 95 → 96): 새 파일 `Tests/CosmicSideRowTests.cpp`(옛 필터의 파일을 고치지 않으려고 — 판의 도우미는 그 파일에) — `Sim.EraEndured`(게임의 시대 행 하나만 든 `DT_Events`, 테스트가 짠 지형과 소모: 599에는 줄이 없고 창을 지나며 다스리는 백성의 줄이 꼭 하나, 국경을 맞댄 백성마다 0.2·그 백성 자신과 국경 밖은 0, 같은 소모 아래의 AI 백성은 줄이 없음, 창 뒤에 다시 서지 않음, 행이 없는 판과 시뮬 스트림이 같음)와 `Sim.WitnessCard`(게임의 `DT_Encounters` 그대로: 주장이 내려앉을 때와 한 칸 위 백성의 주장된 성계에 도착할 때 오고 두 칸 위의 땅에서는 오지 않음, 답한 뒤 곧바로 다시 오지 않음, 모든 눈에서 통과하는 정신으로 주인의 경외 +0.1·주장과 카운트다운 그대로, 물러남은 경외 없음, 받아들인 답마다 시뮬 수 하나, 정책의 판정 없는 답을 경계마다 — 첫 경계 뒤 0·둘째 경계 뒤 1); 단언과 변이의 이름은 `docs/40`의 두 행. `Data.Integrity`의 걸림줄은 이제 열 행(`Today == Ten` — 열 가운데 하나가 데이터를 떠나거나 도우미의 이름이 제 행을 못 찾으면 깨진다; 주석 (1)과 메시지). 지침서 §2.4의 경우: `Sim.WandererLoop` 시드 4의 고리 그대로 닫힘; `Feats.Reachable` 다섯째 칸 그대로(시작 단계가 물은 'B6b 데이터를 끌지'는 필요 없었다); `Sim.PhaseTwoExit`는 `[B-6b7]`부터 `CosmicTests::WithoutB6bData(Original)`의 표라(그 스위치가 시대·증언 행도 뺀다) 지침서의 행은 뜻이 없다 — 그대로 둔다(게임 데이터 전체로 맞는 시드는 `[B-6b12]`); `Events.BookCycle`(지침서의 'Sim.BookCycle')·`Sim.EraSelector`·`Data.Integrity`의 신정론 검사·검사기의 비문 테스트 그대로. smoke에 들지 않는 테스트 가운데 게임 데이터를 오래 돌리거나 주장 위에 방랑자를 두는 열넷을 탐침 단계에서 필터로 한 번 돌렸다(14/14 — `Sim.EraEndured`·`Sim.WitnessCard`·`Sim.WandererLoop`·`Feats.Reachable`·`Log.ClaimVoice`·`UI.SpiritualSight`·`Log.NoConsecutiveRepeat`·`Sim.NotablesFromEventsOnly`·`UI.Chronicle`·`Sim.PetitionPresent`·`Events.BookCycle`·`Sim.EraSelector`·`Sim.PressureEndured`·`Sim.PlayerAscendsToT3`; `[B-6b7]`의 `Sim.PhaseTwoExit`가 smoke 밖에서만 깨진 교훈). 변이가 살아남지 않아 고친 테스트는 없다.",
"- **변이**(모두 깨졌다 — 살아남은 것 0·테스트 고침 0; 일의 차례가 바뀌어 마지막 러너와 묶음 B 앞에서 돌았다 — 아래 ⑬): 새 `b6b8_` 열일곱(`Tools/mutations/B6b/`, README 행마다 하나, 에셋을 다시 짓는 넷은 README 머리의 목록에도) — 파이썬 아홉(`schema_witness_rule_dropped`·`schema_witness_words_off_body`·`schema_witness_past_forms_dropped`·`schema_witness_adnominal_forms_dropped`·`schema_era_shape_dropped`·다시 뜬 `schema_desolation_without_lamp`·`schema_era_rule_dropped`와 데이터 `data_witness_says_jeungeon`·`data_era_no_ruled` — `check_data`나 검사기 단위 테스트가 깸), 에셋을 다시 짓는 데이터 넷(`data_era_borders_0_3` — `Sim.EraEndured`의 0.2가 0.3; `data_era_no_ruled_past_checker` — 검사기와 그 단위 테스트를 지나 에셋에 닿고 `Sim.EraEndured`의 'AI 백성에게는 줄이 없다'가 1; 다시 뜬 `data_extinction_expiry_effect` — `Data.Integrity`; `data_witness_reverence_0_3` — `Sim.WitnessCard`의 0.1이 0.3), C++ 셋(`wanderer_nocheck_not_by_row`·`wanderer_nocheck_reads_next_choice`·`wanderer_nocheck_inverted` — 모두 `Sim.WitnessCard`), 커맨드렛 하나(`cmdlet_witness_first_any_card` — 패치를 넣고 지은 빌드에서 `bundleB.py --max-seed 2 --cells wanderer --t-death 5000 --r 0.375`가 FAIL: W 대조의 'witness cards 0 with first at 1221 (W >= 0 iff cards > 0)'와 'before the first claim placed'). 다시 뜬 셋의 옛 패치(`b6b5_schema_desolation_without_lamp`·`b6b5_schema_era_rule_dropped`·`b6b6_data_extinction_expiry_effect` — 이 커밋의 줄이 문맥을 옮겨 서지 않는다)는 README 행에 표지. 다시 돌린 옛 패치 129(에셋 14·C++ 39·탐침 2·파이썬 74 — `scratchpad\\b6b8\\rerun_set.txt`, 기계로 뽑음): 이 커밋이 고친 파일의 패치(`DT_Events.json` 3·`DT_Encounters.json` 5·`schema.json` 77·`test_data_schema.py` 1·`CosmicDataTests.cpp` 15·`CosmicWandererPolicy.cpp` 12 — `b1d_` 여덟·`b6a3c_wanderer_policy_reads_clock`·`b6b3b_wanderer_` 셋, 커맨드렛 2)와 이 커밋이 고친 테스트 파일에 README의 'Breaks'가 정의된 패치(`Tools/test_data_schema.py` 23·`CosmicDataTests.cpp` 25 더) 가운데 HEAD에 서는 것 — 서지 않는 옛 30은 이 커밋의 것이 아니다(`[B-6b12]`). 결과: 에셋 18·C++ 42·커맨드렛 1·파이썬 83(프로젝트 밖 복사본) = 144 모두 깨짐(`b6b2a_data_reach_word_renamed`는 README대로 검사기가 에셋 쓰기를 막으므로 테스트를 따로 돌려 `Data.Integrity`에서 깨짐); 탐침 둘(`b6b7_probe_*`)은 붙는지만 보았다(돌리기는 `[B-6b12]`). 러너는 `git apply -R` 대신 패치 전 바이트를 다시 써서 되돌렸고(LF 그대로), 끝에 트리가 스냅숏과 바이트까지 같음을 보고 두 타깃을 `Rebuild.bat`으로 다시 지었다(4.5분); 에셋 열둘은 HEAD 바이트 그대로. `git apply --check`의 앞뒤(1,058 → 1,075 패치, 선다 914 → 928, 서지 않는다 144 → 147): '선다 → 서지 않는다'는 다시 뜬 셋뿐이고 새 열일곱은 모두 선다(`scratchpad\\b6b8\\apply_after.txt`). 시간 약 2시간(C++·에셋 77분 — `make_data_tables` 한 번 약 95초인 에셋 패치 열여덟이 대부분; 러너의 경로 셈이 틀려 커맨드렛에서 한 번 멈춰 약 10분을 잃었다) — 목표 60~75분을 넘었다.",
]

# The rewritten list of deviations (items 1-7, 9, 11-13; 8 and 10 are the two frozen items below).
marker_8 = "⑧의 한 마디(H:343의 읽기 전체는 묶음 B 전에 시각과 함께 이 자리에 — 고침 단계가 정한 것만 먼저): "
k8 = old_diff.index(marker_8)
k7 = old_diff.index("⑦ 비문 — 그대로 둔다")
k_else = old_diff.index("그 밖에 — 묶음 B 적기만의")
k_order = old_diff.index("일의 차례(리드의 결정 — 지침서 §7에서 벗어남): ")
item1 = old_diff[old_diff.index("① 짝의 다리"):k7].rstrip()
item7 = old_diff[k7:k8].rstrip()
item8_word = old_diff[k8 + len(marker_8):k_else].rstrip()
else_part = old_diff[k_else:k_order].rstrip()
order_part = old_diff[k_order + len("일의 차례(리드의 결정 — 지침서 §7에서 벗어남): "):].rstrip()
assert order_part.startswith("C++·에셋·커맨드렛 변이를 마지막 러너와 묶음 B 앞에서 돌린다")

DIFF = ("- **헤더와 다르게 하거나 더한 것**(번호는 지침서 §6의 자리 — ⑧·⑩은 아래 두 항목이고 묶음 B를 시작하기 전에 적었다): "
 + item1 + " "
 + "② 강제 죽음 칸은 헤더를 글자 그대로 — R에 `-DeathAt=5000`이고 `-Wander`가 없다(B5의 선례는 `-Wander`를 함께 걸었다); 이 칸은 묶음 B에서 적기만이고 ①~④의 어느 판단도 읽지 않으며('15% 아래'는 묶음 C의 읽기), 통제는 calling 줄의 'death asked at 5000 (recorded r)'이다 — r = 0인 판은 실패가 아니라 적는다. 5,000은 §2 '`[B-6b6]`의 다시 셈'의 t. "
 + "③ 시대 행의 분류 `Event.Civ.Society`(헤더가 정하지 않았다 — 책의 행과 같고 스키마가 허락하는 짝; 분류는 추첨 행의 몫에만 무게를 주므로 상시 행에서는 세계에 닿지 않는다). "
 + "④ 규칙 W·E(헤더에 없는 두 규칙 — 위 「스키마」): W는 §1.6 규칙 7을 지금 행과 뒤의 모든 주장 곁 방랑자 카드에 기계로 건다 — 글자 그대로 찾으므로 줄어든 과거꼴을 따로 적고(가장 가까운 본보기 `enc_star_watch`의 '당신은 관측소를 떠났다'), '기록'은 성공 줄 밖 다섯 칸에서만 막는다(칸 시험은 선택지를 가르지 못해 '물러난다'의 성공 줄은 글 감사가 지킨다); E는 결정 22의 꼴 — 누구의 경외가 언제 오르는가라는 신학의 선택 — 을 데이터만으로 지키는 유일한 자리다. "
 + "⑤ 판정 없는 답의 셈과 증언 첫 틱(위 「코드」 — 노트 §2.1이 바란 '증언 성공·실패·물러남'을 옛 race 줄은 낼 수 없었다: 정책은 답의 수로 선택지를 골라 답의 약 절반이 물러남이다); 방주 카운터('방주를 둔 채 만료와 그 진척' — B6b 지침서가 선택으로 둔 것)는 넣지 않았다 — 묶음 B의 어느 판단도 읽지 않고, 방주 비용의 판단은 `[B-6b6]`의 것이며, `[B-6b7]`도 더하지 않았다; 묶음 B는 '재지 않음(카운터 없음)'을 적고, 방주의 값이 바뀌면 `[B-6b9]`나 `[B-6b11]`에 권한다. "
 + "⑥ 주석 둘은 그대로 둔다 — `Tests/CosmicTestHelpers.h:354`('A name the table does not hold yet takes nothing out')와 `Tests/CosmicBalanceCommandlet.h:123`은 시험의 표에 여전히 참이고, 고치면 그 도우미의 패치 열일곱을 다시 돌려야 한다. "
 + item7 + " "
 + "⑧의 한 마디(H:343의 읽기 전체는 아래 ⑧): " + item8_word + " "
 + "⑨ 묶음의 폴더는 OneDrive 밖(약 550MB — 세션의 scratchpad는 `[B-6b9]`·`[B-6b11]`이 다시 읽기 전에 지워질 수 있다)이고, 판단 뒤 모든 `.log`와 출력 전부의 sha1 목록을 `Saved\\Balance\\b6b_bundleB\\`로 옮겼다(B6a `b6a_bundle1`의 선례) — 두 경로는 §3 '묶음 B'. "
 + "⑪ `[B-6b4]`에서 미룬 창 없는 사진(헤더 §8의 `[B-6b4]` 행과 §9의 ⑥ — 손 확인 ⑥이 아니다; B6b 손 확인의 세이브는 `[B-6b10]` 뒤): <<B:SHOT>> "
 + "⑫ `docs/10_DESIGN/systems.md`의 '지금의 방랑자 카드'(§1.2가 `[B-6b12]`에 둔 줄)를 앞당겼다 — 카드 여섯, `enc_witness`를 'B1 밖(아직 코드에 없다)'에서 빼고 그 카드의 결과 문장을 더했다; 같은 까닭으로 이 커밋이 거짓으로 만든 현재형 문장 — `docs/10_DESIGN`의 셋·`docs/20`·`docs/21`·`docs/40`·`docs/70`의 '`[B-6b8]`부터' 표지와 카드 수 — 을 고쳤다(리뷰 줄의 L1-3~L1-5 포함). "
 + "⑬ 일의 차례(리드의 결정 — 지침서 §7에서 벗어남): " + order_part + " "
 + else_part)
lines[i_diff:i_diff + 1] = BEFORE + [DIFF]

i_review = find("- **리뷰**(ultracode — 구현 워크플로: 구현 일꾼 셋")
AFTER = [
"- **기준값**(비트 그대로 — 시드 42의 다섯 판에 시대 줄·증언 카드 0, 먼저 움직인 것 없음): 3,000틱 2557850423 / 2906170550, 통치자 3,000 1189870756 / 3359656751, 방랑자 6,000 1631315934 / 489794244, 20,000틱 1957158585 / 3764704052, 통치자 20,000 1375149537 / 510604211 — 통치자·방랑자의 꼬리까지 `Saved\\Balance\\b6b8_on`과 글자 그대로(`wrote` 경로 밖); 해시 판 둘 = 커맨드렛의 3,000틱 두 줄. §2.2의 `[B-6b8]` 행이 맞았다 — 시드 42는 1,072에 폐위되어 시대 행(`SystemOwnedBy Ruled`)이 서지 않고, 방랑자 6,000 판의 `_claims.csv`에는 주장이 없어 증언이 올 자리가 없었다(탐침 다섯 판과 묶음 스모크 18판 어디에서도 증언은 오지 않았다 — 실제 로그의 증언 첫 틱·판정 없는 답·W 대조는 묶음 B가 처음 보인다). ⓐ 같은 빌드의 `-WithoutB6bData=1` 다섯 판이 `[B-6b5]`의 키 글자 그대로(2816687688 / 3300228750, 3561771757 / 2462331303, 2665013209 / 2229239127, 2359424635 / 726895141, 1611658162 / 29194274)이고 로그마다 열 이름 한 줄(§3 '묶음 B'의 머리); ⓑ `b6b8_base`(= `b6b7_on`, 시작 단계가 바이트 그대로 옮김)와 견줘 다섯 판 모두 E·W가 ∞이고 CSV·`_rows.csv`에 다른 행이 없다(모든 판을 min(E, W) 아래에서 — 위 ①); ⓒ 로그 열둘(켬 5·끔 5·해시 판 2)의 `end check`·`race check`·`LogCosmic: Error` 0. 러너: 빌드 판 `-Tier smoke -Data -UpdateBaseline`(두 판 20/20), 고침 뒤 smoke 한 번(검사기 규칙의 도장만 바뀜 — 에셋 열둘 그대로), 마지막 `-Tier full -Data -UpdateBaseline`(2026-11-10 07:28~07:38, 'all passed' — 225개를 두 판 450 Success·실패 0; 두 타깃은 'up to date' — 변이 단계 끝의 `Rebuild.bat` 빌드). `seed42.json`은 러너가 PowerShell 꼴로 다시 쓴 것을 원래 모양으로 돌려(`scratchpad\\b6b8\\fix_seed42.py`) HEAD와 바이트까지 같다 — 옮은 줄이 없어 `last_recorded_in_changelog`·`Tools/run_checks.ps1`의 주석·`docs/40:25`·STATUS의 사실은 그대로. ⓐ는 마지막 러너 뒤에 다시 돌리지 않았다 — 탐침 뒤에 바뀐 것은 테스트 단언·검사기 규칙과 그 도장·문서·하네스뿐이고 에셋은 탐침 때와 바이트까지 같다(시뮬 소스는 변이 단계가 스냅숏 바이트로 되돌렸고, 커맨드렛 소스는 고침 단계의 기록에 없는 것으로 판단했다).",
"- **묶음 B와 판단**(§3 '묶음 B' — 미리 적은 판단이 깨져도 이 커밋은 멈추지 않고 적은 뒤 `[B-6b9]`에 넘긴다; 값은 여기서 고르지 않는다 — 헤더 §5.3): ① <<B:J1>>/60 ② <<B:J2>>(가름 <<B:SPLIT>>) ③ 길이 <<B:J3_LEN>>·밀도 <<B:J3_DENSITY>> ④ C₀ <<B:C0>>·C <<B:C>>·(i) <<B:J4_I>>·(ii) <<B:J4_II>>; 잠김 <<B:B_LOCKED>>, AI 단계 µs <<B:B_AI_US>>; 넘김 <<B:HANDOFF>>.",
]
lines[i_review + 1:i_review + 1] = AFTER

out = "\n".join(lines)
assert out.endswith("\n")
# the frozen pair again
o = out.split("\n")
j8 = [i for i, l in enumerate(o) if l.startswith("- **⑧ H:343의 읽기 — 판단 ②**")][0]
assert hashlib.sha1(("\n".join(o[j8:j8 + 2]) + "\n").encode("utf-8")).hexdigest() == "43e54e441f59560031a662b77d686225c3556cd7"
if "--write" in sys.argv:
    open(N, "wb").write(out.encode("utf-8"))
    print("written", len(out.encode("utf-8")))
else:
    print("dry run ok", len(out.encode("utf-8")))
