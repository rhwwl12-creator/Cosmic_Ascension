"""Apply the kept findings of the answers review to the B7x header, notes and accepted design doc (WT only)."""
import sys

WT = "C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad/b7x/wt/"
HP = WT + "docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md"
NP = WT + "docs/60_DECISIONS/headers/B7_composable_existence_and_evolution_notes.md"
DP = WT + "docs/10_DESIGN/EXISTENCE_EVOLUTION_AND_ECOLOGY.md"
SCRIPT = "C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad/b7x/answers/rename/rename_leaves.py"

texts = {}
for p in (HP, NP, DP):
    b = open(p, "rb").read()
    assert b"\r" not in b and not b.startswith(b"\xef\xbb\xbf"), p
    texts[p] = b.decode("utf-8")

errors = []


def rep(p, old, new, tag):
    t = texts[p]
    n = t.count(old)
    if n != 1:
        errors.append("%s: %r found %d times" % (tag, old[:70], n))
        return
    texts[p] = t.replace(old, new)


def line_starting(p, prefix):
    hits = [l for l in texts[p].split("\n") if l.startswith(prefix)]
    assert len(hits) == 1, (prefix, len(hits))
    return hits[0]


# ---------------- HEADER ----------------
H = HP
# L1-01
rep(H, "열넷은 슬라이스 나·다로 넘기며",
    "열둘은 슬라이스 나·다로 넘기며(남은 기계 둘 — `FossilAutomata`·`UndyingAutomata` — 은 결정 16 (3a)·(4)로 글과 받지 않음에 옮겼다)", "L1-01")
# L1-03 / L2-4 / L3-4 / L4-03 (one wording: L3-4)
rep(H, "§4 다른 PC의 문구(브랜치에서만 — `[B-7x0]`이 지운다)",
    "§4 다른 PC에 붙여 넣던 문구의 기록(파일은 이 브랜치에서 지웠다 — 결정 21 (iii))", "L3-4 hdr")
# L1-02 / L3-3 / L4-01: section 0
rep(H, "`[B-7x2]`는 누적 셈을 켜므로 두 해시를 한 번 더 옮긴다 — 셈은 블록 없이도 모든 백성에서 첫 틱부터 쌓이고(첫 한 성계·물려받은 전시 동원) 해시가 그 값을 걷는다(`Sim.HashCoversTimeVectors`). 세계는 그대로다 — 셈을 읽는 행과 걸음이 아직 없어 CSV의 결정적 칸이 같다(`[B-6b1]`의 꼴; 두 커밋을 하나로 합쳐 기준값을 한 번만 다시 적어도 된다).",
    "누적 셈(브랜치의 `[B-7x2]`)은 결정 20의 답대로 `[B-7x1]`과 한 커밋에 든다 — 셈은 블록 없이도 모든 백성에서 첫 틱부터 쌓이고(첫 한 성계·물려받은 전시 동원) 해시가 그 값을 걷으므로(`Sim.HashCoversTimeVectors`) 두 해시는 그 한 커밋에서 필드와 셈을 함께 담아 한 번 옮고, 기준값도 한 번만 다시 적는다. 세계는 그대로다 — 셈을 읽는 행과 걸음이 아직 없어 CSV의 결정적 칸이 같다(`[B-6b1]`의 꼴).",
    "L3-3 s0")
# section 8 head
rep(H, "— 두 해시를 한 번만 옮긴다(§0이 허락한다); 아래 2·3의 짓기와 검증을 그 한 커밋에서 함께 한다.",
    "— 합친 커밋의 이름은 `[B-7x1]`이고, 이 헤더와 노트가 `[B-7x2]`라 부르는 몫(장치·넘김·테스트)도 그 커밋에 든다; 두 해시는 한 번 옮고 `-UpdateBaseline`도 한 번이다(§0); 아래 차례 2가 그 한 커밋이다.",
    "L4-01 s8 head")
# merge rows 2 and 3
row2 = line_starting(H, "| 2 | `[B-7x1]` 필드 |")
row3 = line_starting(H, "| 3 | `[B-7x2]` 장치 |")
merged = (
    "| 2 | `[B-7x1]` 필드와 장치(한 커밋 — 결정 20) | 01eef6f + 492c4f0 + 5969bd5 + 85f297c를 하나로 + 결정 3의 새 잎 이름(태그·접근자·노브 `BrickIdleInsightPerTick`·함수 `IsIdleOf` — 준비한 스크립트 `"
    + SCRIPT
    + "` — 저장소 밖 이 세션의 임시 폴더라 다시 쌓을 때 없을 수 있다; 없으면 노트 §5.4 코7의 목록대로 손으로 바꾼다, 노트 §5.4 코7). §8.1의 해소, 노트 §5.4의 `[B-7x1]`·`[B-7x2]` 넘김(코드·스키마·테스트), 보드 고침(`Sim.DiplomacyRules` 등). `ReignShapes`(결정 13 ㉢)와 탄생 요청 필드(결정 6 (ㄷ))는 없다. `docs/20`(저장 문단·씨앗 문장 — §7)·`docs/21`을 먼저 | xhigh | "
    "에디터를 닫고 전체 빌드; 러너 full 두 번; **두 해시가 이 커밋에서 한 번 옮는다**(필드와 누적 셈 — 새 해시를 커밋 메시지에); 다섯 판의 CSV·`_rows.csv`·`_claims.csv`(`TickMicros`·`SaveBytes`·`SaveRawBytes` 빼고)와 `ending:`·`race:` 줄이 앞 커밋과 글자 그대로; "
    "선택(리뷰 DET-2): 해셔가 `TimeVectors`·`HostCivID`를 건너뛰게 한 로컬 탐침 빌드로 다섯 판과 해시 판 둘을 돌려 리베이스한 main의 `Tools/baselines/seed42.json`과 두 해시가 비트 그대로인지 본 뒤 탐침을 뺀다(탐침은 `Tools/mutations` 아래에 두지 않는다); "
    "`Sim.SaveTimeVectors`·`Sim.HashCoversTimeVectors`, 옛 세이브; 새 단언마다 변이 한 번; 변이 패치 apply 전후, 멈춘 패치를 다시 뜬다; 에셋과 `generated.sha1`; `docs/30`; 이 커밋의 하나뿐인 `-UpdateBaseline`(커밋 메시지에 한 줄); ultracode 구현 리뷰 |"
)
assert SCRIPT in row2 and "DET-2" in row3
rep(H, row2 + "\n" + row3, merged, "L3-3 merge")
# renumber later rows (+ L3-1, L4-08)
rep(H, "| 4 | `[B-7x4m]` 측정 |", "| 3 | `[B-7x4m]` 측정 |", "renum 4")
rep(H, "그다음 '값보다 먼저 재는 것'을 커밋하지 않는 데이터 사본으로 잰다 |",
    "그다음 '값보다 먼저 재는 것'을 커밋하지 않는 데이터 사본으로 잰다(사본에도 `rename_leaves.py`) |", "L3-1 row4m")
rep(H, "| 5 | `[B-7x3]` 표현 |", "| 4 | `[B-7x3]` 표현 |", "renum 5")
rep(H, "| 6 | `[B-7x4]` 글 |", "| 5 | `[B-7x4]` 글 |", "renum 6")
rep(H, "| 7 | `[B-7x5s]` 씨앗·노브 | aa09543의 `DT_Rules` 몫(노브 열일곱의 값·`BrickSeedTags`·변이 노브) |",
    "| 6 | `[B-7x5s]` 씨앗·노브 | aa09543의 `DT_Rules` 몫(노브 열일곱의 값·`BrickSeedTags`·변이 노브) — 옮긴 뒤 `rename_leaves.py`를 다시 돌린다(초안 데이터는 옛 잎 이름이다 — 결정 3) |", "renum 7")
rep(H, "| 8 | `[B-7x5c]` 카드·사건·Society | aa09543의 `DT_Encounters`·`DT_Events`·`DT_Civs` 몫 + 노트 §5.4의 데1·데2·데5; 6의 감사한 글을 글자 그대로 |",
    "| 7 | `[B-7x5c]` 카드·사건·Society | aa09543의 `DT_Encounters`·`DT_Events`·`DT_Civs` 몫 + 노트 §5.4의 데1·데2·데5; 5의 감사한 글을 글자 그대로 — 옮긴 뒤 `rename_leaves.py`를 다시 돌린다(초안 데이터는 옛 잎 이름이다 — 결정 3) |", "renum 8")
rep(H, "| 9 | `[B-7x5r]` | 판단이 노브를 부를 때만, 제 커밋 |",
    "| 8 | `[B-7x5r]` | 판단이 노브를 부를 때만, 제 커밋 — 판단 ⑥이 B6b 값과 어긋날 때(결정 1 — B7x의 노브가 먼저 물러선다; B6b의 r·c·N·P_bind·C는 건드리지 않는다)와 `-Crossroads` 칸(§5 ⑧)이 남긴 법 백성에서 놀라운 값을 낼 때(결정 18); 한 번 고쳐도 어긋나면 그 수와 함께 다시 묻는다 |", "L4-08")
rep(H, "| 10~ | `[B-7x6]`", "| 9~ | `[B-7x6]`", "renum 10")
# L3-1: section 8.1 bullet
rep(H, "`DT_Rules.json` 끝의 노브 값, `DT_Civs.json`의 `arch_seeker`.",
    "`DT_Rules.json` 끝의 노브 값, `DT_Civs.json`의 `arch_seeker`. 옮긴 뒤 `rename_leaves.py`(결정 3 — 초안은 옛 잎 이름이다).", "L3-1 s8.1")
# L1-06 / L2-7 / L4-17: KeptLaw row middle cell
rep(H, "(`CosmicEdicts::AIRevokesOf`의 첫 물음) — 오른쪽 끝의 둘은 빼고 |",
    "(`CosmicEdicts::AIRevokesOf`의 첫 물음) — 다만 주장의 카운트다운에 더하는 칙령과 위기의 답인 칙령, 그리고 판정받는 행이 법칙을 물을 때(`CosmicEdicts::AsksForLawsOf`)의 백성 범위 칙령은 여느 AI처럼 거둔다(결정 18) |", "L1-06")
# L1-07 / L2-6 / L4-17
rep(H, "통치자가 세운 칙령을 하나도 거두지 않는다는 사실이다", "통치자가 세운 칙령을 거두지 않는다는 사실이다", "L1-07")
# L1-05 / L2-5
rep(H, "**①(관측 전에는 어디에나)도 받는다**(T10):",
    "**리뷰의 고침 ①도 받는다**(T10 — 원안 `QuantumCloud`의 '관측 전에는 어디에나'):", "L2-5")
# L1-08 optional, header side
rep(H, "②는 태그를 주지 않아 닫지 않으므로",
    "②는 태그를 주지 않아 닫지 않으므로(C09의 ②는 되돌림이라 성공하면 걷힌 부모로 닫힌다)", "L1-08 hdr")
# L3-5 / L1-09 / L4-06: section 4 rule (c) and the cases
rep(H, "(c) 같은 백성을 다스린 생애가 여럿이면 마지막 생애의 장에만 붙인다; (d) 낱말은 '그 백성의 지금 살림'으로 읽히게 쓰고 — 셈은",
    "(c) 같은 백성을 다스린 생애가 여럿이면(`RuledCivIDs`의 어느 자리든) 마지막 생애의 장에만 붙인다 — 그 마지막 생애의 마지막 재위 백성이 다른 백성이면 (a)대로 그 백성의 낱말은 어느 장에도 붙지 않는다; (d) 낱말은 '그 백성의 지금 살림'으로 읽히게 쓰고 — 셈은", "L3-5 s4 c")
rep(H, "`UI.Chronicle`이 지워진 백성·다스린 적 없는 생애·두 백성을 다스린 생애의 경우를 단언한다(노트 §5.4 시9).",
    "`UI.Chronicle`이 지워진 백성·다스린 적 없는 생애·두 백성을 다스린 생애·같은 백성을 다스린 두 생애·뒤 생애가 다른 백성으로 끝난 경우를 단언한다(노트 §5.4 시9).", "L1-09 s4")
rep(H, "`UI.Chronicle`에 끝의 꼴 낱말의 경우 셋: 지워진 백성·다스린 적 없는 생애·두 백성을 다스린 생애(결정 13 ㉢ — 노트 §5.4 시9).",
    "`UI.Chronicle`에 끝의 꼴 낱말의 경우 다섯: 지워진 백성((b))·다스린 적 없는 생애((a))·두 백성을 다스린 생애((a) — 뒤 백성의 꼴만)·같은 백성을 다스린 두 생애((c) — 마지막 장에만)·뒤 생애가 다른 백성으로 끝난 경우((a)·(c) — 앞 백성의 낱말은 어느 장에도 없다)(결정 13 ㉢ — 노트 §5.4 시9).", "L4-06 s6")
rep(H, "(c) 같은 백성을 다스린 생애가 여럿이면 마지막 생애의 장에만 붙인다; (d) 낱말은 '그 백성의 지금 살림'으로 읽히게 쓰고 판의 끝 뒤",
    "(c) 같은 백성을 다스린 생애가 여럿이면(`RuledCivIDs`의 어느 자리든) 마지막 생애의 장에만 붙인다(그 생애의 마지막 재위 백성이 다르면 어느 장에도 붙지 않는다); (d) 낱말은 '그 백성의 지금 살림'으로 읽히게 쓰고 판의 끝 뒤", "L3-5 d13")
rep(H, "`UI.Chronicle`에 지워진 백성·다스린 적 없는 생애·두 백성을 다스린 생애의 경우(노트 §5.4 시9 — §6)",
    "`UI.Chronicle`에 지워진 백성·다스린 적 없는 생애·두 백성을 다스린 생애·같은 백성을 다스린 두 생애·뒤 생애가 다른 백성으로 끝난 경우(노트 §5.4 시9 — §6)", "L1-09 d13")
# decision 3: L2-11, L2-2, L2-10, L1-12, L1-13/L3-10, L3-1
rep(H, "'목자'는 §3.5가 금한 낱말이고 main `Tools/data/schema.json:51`의 revelation_mimicry에 있어 `StarTow`",
    "'목자'는 §3.5가 금한 낱말이고 '선한 목자'는 main `Tools/data/schema.json:51`(`10205b6`)의 revelation_mimicry에 있어 `StarTow`", "L2-11")
rep(H, "`KeptLaw`·`FoldedSmall`·`ShutSphere`는 태그와 꼴이 같은 것을 가리켜 해가 없다.",
    "`KeptLaw`·`FoldedSmall`·`ShutSphere`는 태그와 꼴이 같은 것을 가리켜 해가 없다. `ShutSphere`의 Sphere는 제 별을 감싸 닫은 구다 — main 코드의 `Sphere`(타락 초월자의 영향권 — `InSphere`·`FallenSphereTicks`·`JoinSphere`·`ECosmicSphereReach`)나 `Trait.Faith.BookShut`(침묵의 영향권 아래 닫힌 책)과 무관하다; 리베이스 때 ini DevComment와 `Core/CosmicTags.h` 접근자 `BrickFormShutSphere`의 주석에 그렇게 적는다(노트 §5.4 코7).", "L2-2 hdr")
rep(H, "**변이의 잎은 꼴의 이름을 받지 않는다 — 꼴 낱말은 변이 없이도 부모를 지닌 모든 백성에 보인다.**",
    "**변이의 잎은 꼴의 이름을 받지 않는다 — 꼴 낱말은 변이 없이도 부모를 지닌 모든 백성에 보인다.** `InwardHunger`의 Inward(§3.3 S11 '안으로 돌아선 식욕' — 행 `event_brick_inward_appetite`)는 `CosmicBricks::IsInward`·정착 사유 `ECosmicSettleBlock::Inward`·노브 `BrickInwardResearchMultiplier`·`BrickInwardHazardSpawnMultiplier`(안으로 접힌 둘 — `ClosedPort`·`FoldedSmall`)와 다른 사실이다 — `IsInward`는 이 변이를 읽지 않는다. 세이브에 남는 이름은 겹치지 않으므로 이름은 그대로 두고, 둘을 가르는 주석을 단다(노트 §5.4 코7).", "L2-10 hdr")
rep(H, "rename_leaves.py`; ini의 이름 고침 제안 주석(branch `Config/DefaultGameplayTags.ini:176`)과 `:184`, 스키마 규칙 4·7의 나열, 테스트 메시지를 포함).",
    "rename_leaves.py`(저장소 밖 — 이 PC 세션의 임시 폴더라 리베이스 때 없을 수 있다; 그러면 노트 §5.4 코7의 목록과 대상대로 손으로 바꾸고 옛 이름 아홉을 grep으로 본다); ini의 이름 고침 제안 주석(branch `Config/DefaultGameplayTags.ini:176`)과 이름이 바뀌는 태그 줄 `:172`·`:173`·`:180`·`:181`·`:184-186`, 스키마 규칙 7의 나열, 테스트 메시지를 포함); 데이터 초안 aa09543의 자리(규칙 4의 나열·`BrickSeedTags`·갈림 카드·사건 행)는 그것을 옮기는 `[B-7x5s]`·`[B-7x5c]`에서 같은 스크립트를 다시 돌려 고친다.", "L3-10 d3")
# decision 6 (L4-10)
rep(H, "그때 두 해시가 한 번 더 옮는다. 받지 않은 대안: 슬라이스 다에서 예약",
    "그때 두 해시가 한 번 더 옮는다(열린 물음 초안 — 노트 §3 새9). 받지 않은 대안: 슬라이스 다에서 예약", "L4-10 d6")
# decision 14 (L3-3 renumber + L4-10)
rep(H, "`CLAUDE.md` 규칙 13의 낱말은 슬라이스 나에서 넣는다(노트 §5.3의 7 — F12).",
    "`CLAUDE.md` 규칙 13의 낱말은 슬라이스 나에서 넣는다(노트 §5.3의 6 — F12). 남은 물음의 초안은 노트 §3 새10.", "L4-10 d14")
# decision 15 (L4-12 + L3-9 default)
rep(H, "좌표의 이동은 창구가 받은 뒤 둘째 걸음이다(대안 '영영 옮기지 않는다'는 받지 않았다).",
    "좌표의 이동은 창구가 받은 뒤 둘째 걸음이다(대안 '영영 옮기지 않는다'는 받지 않았다). 이 꼴이 여는 물음은 슬라이스 나의 헤더가 (iii)~(v)와 함께 먼저 답한다(§2.8): 수도의 어느 레인을 끊는가(이름 차례인가, 새 터에서 가장 먼 쪽인가); 새 터가 이미 수도의 이웃이면 그 정착의 걸음은 없는가; 창구가 거절하면(어느 끝이든 레인 상한, `Unlink`가 수도 쪽 이웃을 고립시킴, 다른 `GalaxyRow`) 걸음은 통째로 없는가, `Link`만 하는가; 같은 틱에 정착이 둘 끝나면 어느 차례인가. 기본값은 서지 못한 걸음은 없고 다음 틱으로 미루지 않는다는 것이다 — 주기 노브가 없으므로 거절된 걸음은 다시 오지 않고, 미루려면 저장 필드가 들어 헤더를 먼저 쓰고 두 해시가 옮는다.", "L4-12 d15")
# section 2.8 (L4-12 / L3-9)
rep(H, "모르는 성계로 잇는 `Link`의 `NoteKnown`(규칙 2) — 물음의 글은 §10 결정 14.",
    "모르는 성계로 잇는 `Link`의 `NoteKnown`(규칙 2), 그리고 S14 걸음의 고름 — 끊는 수도의 레인(기본값: 고립을 만들지 않는 것 가운데 이름 차례의 첫 레인), 새 터가 이미 수도의 이웃일 때, 창구가 거절할 때(상한·고립·성역·다른 은하), 같은 틱에 정착이 둘 끝날 때; 기본값은 서지 못한 걸음은 없고 다음 틱으로 미루지 않는다(미루려면 저장 필드가 든다 — 들이려면 헤더를 먼저 쓰고 두 해시가 옮는다) — 물음의 글은 §10 결정 14·15.", "L4-12 s2.8")
# L2-3 (2.9 and decision 16)
rep(H, "`MirrorProjection`과 S13의 세 문 — 물음의 글은 §10 결정 6·16.",
    "`MirrorProjection`과 S13의 세 문, 물려받은 탄생 블록이 태어난 백성의 죽음에서 다시 요청을 남기는지 — 남기면 포자의 탄생이 세대를 건너 끝없이 이어진다(아이마다 새 백성이지만 원안 동결본 §2 축 3 `NovaSpore`의 되풀이를 닮는다); 그 헤더가 사슬을 끊거나 한도를 두는 규칙과 그 시험을 먼저 정한다 — 물음의 글은 §10 결정 6·16.", "L2-3 s2.9")
rep(H, "`HostCivID` 정리(결정 6 (ㄱ)), 품는 쪽 수입의 몫(R14)",
    "`HostCivID` 정리(결정 6 (ㄱ)), 물려받은 탄생 블록이 태어난 백성의 죽음에서 다시 요청을 남기는지(남기면 포자의 탄생이 세대를 건너 이어진다 — 끊는 규칙이나 한도와 그 시험; 조건 (1)이 아이를 새 백성으로 두므로 원안의 불사 분열 그 자체는 아니다), 품는 쪽 수입의 몫(R14)", "L2-3 d16")
# L3-12 + L2-9 (2.9 (3a))
rep(H, "(3a) 남은 기계(`Brick.Form.FossilAutomata`)는 절멸(`LetPeopleGo`)에서 태어나지 않고 글이나 스캔 얼굴로만 둔다.",
    "(3a) 남은 기계(원안 동결본 §2 축 1의 `FossilAutomata` — 태그를 선언하지 않는다)는 절멸(`LetPeopleGo`)에서 태어나지 않고 글이나 스캔 얼굴로만 둔다(그 글도 §3.3 규칙 8을 지난다).", "L3-12")
# L2-9 (rule 8)
rep(H, "(결정 16 (2); 깨짐: 그 목록의 낱말을 지닌 탄생 줄).",
    "(결정 16 (2); 깨짐: 그 목록의 낱말을 지닌 탄생 줄). 남은 기계의 얼굴 글(`FossilAutomata` — 결정 16 (3a); 잎을 선언하지 않으므로 태그가 아니라 그 글의 행·키에 건다)도 같은 목록을 지나고 옛 백성이 기계로 이어 산다고 말하지 않는다 — 그 얼굴을 처음 쓰는 커밋이 이 규칙을 함께 든다(깨짐: '유령'을 지닌 남은 기계의 얼굴 글).", "L2-9 r8")
# L4-07
rep(H, "6. 왕좌를 내려놓는 갈림 뒤 옛 백성이 섭정 아래에서 그 꼴을 지닌 채 가는가.",
    "6. 왕좌를 내려놓는 갈림 뒤 옛 백성이 섭정 아래에서 그 꼴을 지닌 채 가는가 — 카드가 오기 전에 전시 동원과 철야(`edict_vigil`)를 내려 두고 ①을 고르면, 전시 동원은 남아 안정도가 줄곧 내려가고 철야(주장의 카운트다운에 더하는 칙령)는 첫 AI 단계에서 거두어지는가(결정 18의 좁힘 (a)). 판정받는 행이 법칙을 물을 때의 갈래 (b)는 넷째 칸이라 이 세이브에 오지 않는다 — 노트 §5.4 시4가 단언한다.", "L4-07")
# L4-10 d18
rep(H, "⑧이 놀라운 값을 내면 노브(`[B-7x5r]`)로 다루고, 그래도 안 되면 그 수와 함께 다시 묻는다.",
    "⑧이 놀라운 값을 내면 노브(`[B-7x5r]`)로 다루고, 그래도 안 되면 그 수와 함께 다시 묻는다(열린 물음 초안 — 노트 §3 새11).", "L4-10 d18")
# L4-09 (section 11 entry)
rep(H, "§0(R12를 좁힘으로 닫음·K9의 범위·원안 동결본 참조), ",
    "§0(R12를 좁힘으로 닫음·K9의 범위·원안 동결본 참조), §1.1·§1.3(원안 동결본 참조), ", "L4-09 a")
rep(H, "같은 판에서 노트(§1.1의 `KeptLaw`·`QuantumCloud` 행, 잎 이름, §5.4의 `[B-7x0]` 글)",
    "같은 판에서 노트(머리, §1의 절 제목(원안 동결본 §2 축 N)과 잎 이름, `KeptLaw`·`QuantumCloud`·`FossilAutomata`·`SystemDragger`·`MirrorProjection`·`NovaSpore`·`UndyingAutomata` 행, §2의 답, §3의 답과 새9~새11 — 새1은 `DECISIONS_LOG`에만, §4의 지움 기록, §5.1·§5.3, §5.4의 '답 N'·`[B-7x0]` 6·`[B-7x1]` 2~4·슬라이스 다·넘김 47, §5.5의 답 표지)", "L4-09 b")
rep(H, "코드·데이터·ini의 이름은 고치지 않았다 — `[B-7x1]`의 리베이스 때 준비한 스크립트로(노트 §5.4 코7).",
    "코드·데이터·ini의 이름은 고치지 않았다 — `[B-7x1]`의 리베이스 때 준비한 스크립트로(노트 §5.4 코7); 스크립트는 이 브랜치를 내보낸 사본에서 돌려 보았다(16파일 213자리, 두 번째 실행 0, 남은 옛 이름 0; 사본의 `check_data`·`test_data_schema`(96)·`check_layers --strict`·`check_docs`가 통과, `make_config_reference --check`는 노브 이름 탓에 낡음 — 다시 만들면 통과). **답의 리뷰**(렌즈 넷 — 답대로 고쳤는가·서사 규칙·결정성·비평): 발견 57, 남긴 것 55(겹친 것은 한 글로 — 겹쳐 부딪친 둘은 답의 표에 맞는 쪽: S14 걸음의 물음은 슬라이스 다의 표지 '(vi)'를 쓰지 않고 결정 15의 이름으로, `promptforother.txt`의 증명은 지운 기록을 남기는 꼴로), 반박 2(`KeptLaw`와 `Revelation.KeptCovenant`의 겹침 — 계시를 지는 낱말은 Covenant다; 성계 패널의 꼴 낱말이 `IsShown`으로 샌다 — 그 줄은 이미 `OwnerShownOf`). 고친 것: 슬라이스 나·다로 넘기는 블록 열둘, 합친 `[B-7x1]` 한 커밋과 §8 표의 한 줄(뒤 차례의 번호를 하나씩 당김)·노트 §5.3의 한 항, 끝의 꼴의 다섯째 경우(뒤 생애가 다른 백성으로 끝남), `KeptLaw` 행과 '추가 이야기'의 좁힘 낱말, ①의 머리, 둘째 갈림 ②의 C09 예외, S14 걸음이 서지 못하는 틱(결정 15), 탄생 블록의 세대 사슬과 남은 기계의 얼굴 글(규칙 8), `ShutSphere`·`InwardHunger`의 낱말 가름, '선한 목자', 이름 바꿈의 대상·ini 줄·데이터 초안의 자리·스크립트가 없을 때의 길과 산문 grep, 코6의 옛 낱말, 코1의 좁힘을 여느 AI의 규칙으로, 시4 (b)의 보드, `[B-7x5r]`의 두 방아쇠, 손 확인 6, 노트 §4의 지움 기록, `docs/archive/analysis_2026-10-18/`의 `SECTION_MAP` 줄, 설계서 §4의 머리와 C10 목소리. 스크립트는 낱말 검사에 밑줄·붙임표 꼴과 파일 이름을 더했다. 점검: `check_docs` 오류 0, 세 md와 동결본 md에 `\\r`·BOM 없음, 헤더 150,000바이트 아래, '답 대기'는 §11의 b57449c 기록에만, 옛 잎 이름은 '옛 → 새'의 기록과 §11에만.", "s11 review")

# ---------------- NOTES ----------------
N = NP
rep(N, "원안(아래 §1의 '원안이 바라는 것' 열의 출처)", "원안(아래 §1의 '원안이 바라는 것' 열과 §2의 '원안의 것' 열의 출처)", "L4-16 a")
rep(N, "| # | 설계서의 것 |", "| # | 원안의 것 |", "L4-16 b")
rep(N, "필드(`[B-7x1]`)와 셈(`[B-7x2]`)에서 한 번씩 옮기고, 매번 CSV로 세계가 같음을 보인다",
    "필드와 셈을 한 커밋(`[B-7x1]`+`[B-7x2]` — 결정 20)에서 한 번 옮기고, CSV로 세계가 같음을 보인다", "L1-02 n103")
rep(N, "| ㉡ | 설계 문서의 이름으로만 |", "| ㉡ | 원안 동결본에만 — 받은 설계서와 화면에는 내지 않는다 |", "L2-13")
rep(N, "남은 물음: 슬라이스 다 헤더 — 요청의 자리(새9), 품는 쪽 수입의 몫(R14)",
    "남은 물음: 슬라이스 다 헤더 — 요청의 자리(새9), 물려받은 탄생 블록이 아이의 죽음에서 다시 요청을 남기는지(세대를 건너는 사슬 — 끊거나 한도를 둔다), 새 백성의 이름 뱅크와 아키타입 행(부모의 행을 물려받지 않으므로 어느 행을 받는가 — 결정 16 (1)), 품는 쪽 수입의 몫(R14)", "L4-11+L2-3")
rep(N, "(c) 같은 백성을 다스린 생애가 여럿이면 마지막 생애의 장에만;",
    "(c) 같은 백성을 다스린 생애가 여럿이면(어느 자리든) 마지막 생애의 장에만 — 그 생애가 다른 백성으로 끝나면 어느 장에도 없다;", "L3-5 n142")
rep(N, "| 새10 | 슬라이스 나 헤더가 먼저 답할 셋(결정 14 — 리뷰 F8 (iii)~(v)) |",
    "| 새10 | 슬라이스 나 헤더가 먼저 답할 넷(결정 14 — 리뷰 F8 (iii)~(v); 결정 15) |", "L4-12 n title")
rep(N, "모르는 성계로 잇는 `Link`가 `NoteKnown`을 쓰는지(규칙 2). `CLAUDE.md` 규칙 13의 낱말도 그 블록에서 |",
    "모르는 성계로 잇는 `Link`가 `NoteKnown`을 쓰는지(규칙 2); (결정 15) S14 걸음의 고름 — 끊는 수도의 레인(고립을 만들지 않는 것 가운데 이름 차례의 첫 레인), 새 터가 이미 이웃일 때, 창구가 거절할 때(상한·고립 금지·다른 은하), 같은 틱에 정착이 둘 끝날 때 — 서지 못한 걸음은 없고 다음 틱으로 미루지 않는다(미룸 = 저장 필드 → 헤더 먼저, 두 해시가 옮는다). `CLAUDE.md` 규칙 13의 낱말도 그 블록에서 |", "L4-12 n body")
old156 = line_starting(N, "B2 헤더 §2.6(`docs/60_DECISIONS/headers/B2_edicts_and_research.md:125`·`:129`)에는 '(B7x —")
rep(N, old156,
    "B2 헤더 §2.6(`docs/60_DECISIONS/headers/B2_edicts_and_research.md:125`·`:129`)에는 B7x 헤더 §0 R12의 새 문장(결정 18의 좁힘)을 '(B7x — …; B7x 헤더 결정 18·R12)'의 괄호로 단다 — 글은 R12 한 곳에만 둔다.", "L1-10")
rep(N, "다른 PC의 문구는 브랜치 최상위의 `promptforother.txt`에만 있고 브랜치에서만 쓰인다 — `[B-7x0]`에서 지운다(리뷰 F12; 그 파일 스스로 '받은 뒤에는 지워도 된다'고 적었다).",
    "다른 PC의 문구는 브랜치 최상위의 `promptforother.txt`에만 있었고 브랜치에서만 쓰였다 — 2026-11-10 결정 21 (iii)대로 `[B-7x0]`을 기다리지 않고 이 브랜치에서 지웠다(`git rm`; 리뷰 F12; 그 파일 스스로 '받은 뒤에는 지워도 된다'고 적었다).", "L4-04 a")
rep(N, "2026-11-10: 개발자의 답이 왔다(헤더 §10 — 모두 권장대로). `promptforother.txt`는 `[B-7x0]`을 기다리지 않고 이 브랜치에서 지웠다(결정 21 (iii)).",
    "2026-11-10: 개발자의 답이 왔다(헤더 §10 — 모두 권장대로).", "L4-04 b")
rep(N, "증명: 파일이 없고 노트 §4·설계서 §5가 그것을 부르지 않는다.",
    "증명: 파일이 없고, 그것을 할 일로 부르는 곳이 없다(노트 §4에는 지운 기록만 남고 설계서는 파일 이름을 쓰지 않는다).", "L4-19")
# section 5.3 merge 4+5, renumber
it4 = line_starting(N, "4. `[B-7x1]`은 다섯 줄의 두 해시를 옮긴다")
it5 = line_starting(N, "5. `[B-7x2]`는 누적 셈이 쌓이므로")
rep(N, it4 + "\n" + it5,
    "4. `[B-7x1]`(누적 셈과 한 커밋 — 헤더 §8, 결정 20)은 다섯 줄의 두 해시를 한 번 옮긴다 — CSV가 `TickMicros`·`SaveBytes`·`SaveRawBytes`를 빼고 같음을 본 뒤 `Tools\\run_checks.ps1 -Tier full -UpdateBaseline` 한 번. `_rows.csv`·`_claims.csv`와 `ending:`·`race:` 줄도 같아야 한다(리뷰 I9). 선택: 해셔 탐침의 비트 그대로(헤더 §8 차례 2 — 리뷰 DET-2).",
    "L3-3 n5.3")
rep(N, "\n6. 변이 패치(`Tools/mutations/B7x/`)", "\n5. 변이 패치(`Tools/mutations/B7x/`)", "renum n6")
rep(N, "\n7. 헤더 §7의 문서:", "\n6. 헤더 §7의 문서:", "renum n7")
# L4-13
rep(N, "목록은 `SECTION_MAP`이 진다 — 넷은 이미 거기 있다(`guideline_v0.8_2026-10-18.md`는 머리에, 셋은 '옛 WORKPLAN.md·루트 md → 새 위치' 표에). 그 표의 끝에 두 줄을 더한다:",
    "목록은 `SECTION_MAP`이 진다 — 넷은 이미 거기 있다(`guideline_v0.8_2026-10-18.md`는 머리에, 셋은 '옛 WORKPLAN.md·루트 md → 새 위치' 표에). main에 추적되는 `docs/archive/analysis_2026-10-18/`(1단계 분석의 브리핑·렌즈·digests — e5ddbd6)은 지금 `CLAUDE.md`에도 `SECTION_MAP`에도 없어서 같이 올린다. 그 표의 끝에 세 줄을 더한다:", "L4-13 a")
rep(N, "(B6a 때의 한눈 보기와 받지 않은 B7x 원안 — 현행 설계가 아니다; 현행은 docs/) |\n   ```",
    "(B6a 때의 한눈 보기와 받지 않은 B7x 원안 — 현행 설계가 아니다; 현행은 docs/) |\n   | 1단계 분석 산출물(2026-10-18 브리핑·렌즈·digests·다음 세션 문구) | docs/archive/analysis_2026-10-18/ (동결본 — 헤더 A6·A7·B4·B6a가 근거로 부른다) |\n   ```", "L4-13 b")
# 코7
rep(N, "   - `Brick.Form.DysonBrain` → `Brick.Form.ShutSphere`(`BrickFormDysonBrain` → `BrickFormShutSphere`)",
    "   - `Brick.Form.DysonBrain` → `Brick.Form.ShutSphere`(`BrickFormDysonBrain` → `BrickFormShutSphere`) — ini DevComment와 접근자 주석에 'the shell closed around its own star; not the fallen sphere (InSphere/FallenSphereTicks) nor Trait.Faith.BookShut'를 적는다(헤더 결정 3)", "L2-2 n")
rep(N, "   - 노브 `BrickQuietInsightPerTick` → `BrickIdleInsightPerTick`(`FCosmicRulesDef`·`DT_Rules.json`·`schema.json`·`docs/30`), 함수 `CosmicBricks::IsQuietOf` → `IsIdleOf`",
    "   - 노브 `BrickQuietInsightPerTick` → `BrickIdleInsightPerTick`(`FCosmicRulesDef`·`DT_Rules.json`·`schema.json`) — `docs/30`은 손으로 고치지 않고 스크립트 뒤 `python -I Tools/make_config_reference.py`로 다시 만든다(`--check` 0이 증명), 함수 `CosmicBricks::IsQuietOf` → `IsIdleOf`\n"
    "   - 이름은 그대로, 주석 한 줄: `Sim/CosmicExistenceBricks.h`의 `IsInward` 선언 주석에 'Inward here means the two folded-in peoples (Brick.Space.ClosedPort / FoldedSmall) only; it does not read Brick.Mutation.InwardHunger, a different fact (S11)'를 더한다(스크립트가 하지 않는다 — 손으로; 비트 그대로; 헤더 결정 3)", "L3-10 b + L2-10 n")
rep(N, "rename_leaves.py`(리베이스하는 세션이 쓴다)로 한 번에 바꾼다",
    "rename_leaves.py`(리베이스하는 세션이 쓴다 — 저장소 밖 이 세션의 임시 폴더라 그때 없을 수 있다; 없으면 위 목록과 아래 대상대로 손으로 바꾼다)로 한 번에 바꾼다", "L1-12 n")
rep(N, "`Config/DefaultGameplayTags.ini:172-184`", "`Config/DefaultGameplayTags.ini:172-186`", "L1-13 n")
rep(N, "`Data/CosmicDefinitionRows.h`의 주석, 테스트(",
    "`Data/CosmicDefinitionRows.h`의 주석, `Sim/CosmicEdicts.cpp/.h`·`Sim/CosmicMegaprojects.h`·`Sim/GalaxySimulationSubsystem.cpp/.h`의 주석, `Tools/test_data_schema.py`, 테스트(", "L3-10 c")
rep(N, "`Tools/data/schema.json`(규칙 4의 나열)·`DT_Rules.json`·`DT_Encounters.json`·`DT_Events.json`.",
    "`Tools/data/schema.json`(노브 줄 — 규칙 4의 나열은 `[B-7x5c]`에서)·`DT_Rules.json`·`DT_Encounters.json`·`DT_Events.json`. 단 데이터 초안 aa09543이 더한 자리(`DT_Rules.json`의 `BrickSeedTags`, `DT_Encounters.json`의 갈림 행과 `DT_Events.json`의 `event_brick_*`, `schema.json` 규칙 4의 나열 두 묶음, `Tools/test_data_schema.py`의 그 시험 둘)는 다시 쌓는 차례에서 `[B-7x5s]`·`[B-7x5c]`에야 든다(`[B-7x4m]`이 값보다 먼저 재는 커밋하지 않는 사본도 같다) — 그 몫을 옮길 때마다 옮긴 뒤 이 스크립트를 다시 돌리고(두 번째 실행은 0) 옛 이름 grep을 그 커밋의 증명에 넣는다. 규칙 7(데3)은 손으로 새 이름으로 적는다.", "L3-1 n")
rep(N, "바꾼 뒤 옛 이름 아홉(`QuietFeed` 포함)이 `Source/`·`Config/`·`Tools/`에 남지 않았음을 grep으로 본다",
    "바꾼 뒤 `Source/`·`Config/`·`Tools/`에 옛 낱말이 남지 않았음을 grep으로 본다 — 대소문자를 가려 옛 잎 여덟과 `QuietFeed`·`BrickQuietInsight`·`IsQuietOf`, 대소문자 없이 산문 꼴 `kessler|planck|tapestry|abyssal[ _-]?maw|star[ _-]?shepherd|hermit[ _-]?monolith|dyson[ _-]?brain|unseen[ _-]?law|silence[ _-]?feed|quiet[ _-]?feed|quiet[ _-]tick`(스크립트의 `LEFTOVER_WORDS`와 같고 파일 이름도 본다; 맨 `quiet`는 찾지 않는다 — `Revelation.Quiet`·`Digest.Quiet`는 그대로 둔다). 스크립트가 세션 임시 폴더에서 사라졌으면 위 목록으로 손으로 바꾸고 이 grep으로 확인한다", "L4-14")
rep(N, "증명: `Sim.SaveTimeVectors`·`Data.Integrity`; ini는 `generated.sha1`에 들지 않는다(main `Tools/make_data_tables.py:267`).",
    "증명: `Sim.SaveTimeVectors`·`Data.Integrity`·`Data.SourcesInSync` — 바꾼 JSON(`schema.json`·`DT_Rules.json`·`DT_Events.json`·`DT_Encounters.json`)은 도장 `generated.sha1`에 들므로 에디터를 닫고 `run_checks.ps1 -Tier full -Data`로 에셋을 다시 만든다(스크립트가 적는 둘째 뒷일); ini는 도장에 들지 않는다(main `Tools/make_data_tables.py:267`).", "L3-10 d")
# 코1 (L3-7)
rep(N, "\t// A people that keeps the edicts it holds when no one rules it (B7x [B-7x2], Brick.Form.KeptLaw) keeps them - except an\n\t// edict that adds to the claim's countdown or answers a crisis (K9, the line B6b decision 9 drew for galaxy laws), and\n\t// except while its row asks for galaxy laws (the slots are the laws' places, B6b decisions 10-11).",
    "\t// A people that keeps the edicts it holds when no one rules it (B7x [B-7x2], Brick.Form.KeptLaw) keeps them - except an\n\t// edict that adds to the claim's countdown or answers a crisis, which it revokes as any AI does (K9, the line B6b\n\t// decision 9 drew for galaxy laws), and except while its row asks for galaxy laws (the slots are the laws' places, B6b\n\t// decisions 10-11).", "L3-7 a")
rep(N, "\t\tif (bKeepsEdicts)\n\t\t{\n\t\t\tif (Def->HazardCountdownBonus.Contains(CosmicTags::HazardClaim()) || Tables.IsHazardAnswer(Row))\n\t\t\t{\n\t\t\t\tRevokes.Add(Row);\n\t\t\t}\n\t\t\tcontinue;\n\t\t}",
    "\t\t// A kept-law people keeps it - unless it adds to the claim's countdown or answers a crisis: those fall to the rule every AI uses.\n\t\tif (bKeepsEdicts && !Def->HazardCountdownBonus.Contains(CosmicTags::HazardClaim()) && !Tables.IsHazardAnswer(Row))\n\t\t{\n\t\t\tcontinue;\n\t\t}", "L3-7 b")
rep(N, "or one that keeps the unseen law (Brick.Form.KeptLaw: CosmicEdicts::AIRevokesOf)",
    "or one that keeps its edicts once its throne is empty (Brick.Form.KeptLaw: CosmicEdicts::AIRevokesOf)", "L3-2")
rep(N, "유지비 없는 백성 칙령 하나와 넉넉한 비축",
    "주장 고리도 위기의 답도 아닌 유지비 없는 백성 칙령 하나(`edict_mobilization` — 물려받는 칙령과 같은 행; `edict_vigil`은 유지비가 없어도 `Hazard.Claim` 덤 때문에 (a)가 먼저 거두어 (b)의 변이 `b7x2_kept_law_keeps_slots`를 가린다)와 넉넉한 비축", "L3-6")
rep(N, "`UI.Chronicle`에 넷 —", "`UI.Chronicle`에 다섯 —", "L3-5 n a")
rep(N, "같은 백성을 다스린 두 생애에는 마지막 생애의 장에만((c)).",
    "같은 백성을 다스린 두 생애에는 마지막 생애의 장에만((c)); 생애 1이 A만, 생애 2가 A → B를 다스렸으면 A의 낱말은 어느 장에도 없고 생애 2의 장에는 B만((a)·(c)).", "L3-5 n b")
rep(N, "C11 ①의 값(`ChangeStability +0.08`)은 지금 고치지 않는다",
    "C08의 새 글은 그 줄의 '떠도는 동안'과 겹치므로 `[B-7x4]`의 글 감사에서 '떠돈 날이 길었다'로 다듬는다(리뷰 L4-18). C11 ①의 값(`ChangeStability +0.08`)은 지금 고치지 않는다", "L4-18 n")
rep(N, "그 전까지 데이터의 노브는 무동작, `BrickSeedTags`는 비어 있다.",
    "그 전까지 데이터의 노브는 무동작, `BrickSeedTags`는 비어 있다 — 옮긴 뒤 `rename_leaves.py`(코7; 초안의 `BrickSeedTags`에 `Brick.Diet.SilenceFeed`).", "L3-1 n4")
rep(N, "(`Tools/test_data_schema.py`에 목록마다 깨짐 하나와 걸린 목록의 핀).",
    "(`Tools/test_data_schema.py`에 목록마다 깨짐 하나와 걸린 목록의 핀). 남은 기계의 얼굴 글(답 16 (3a))도 같은 목록을 지나고 옛 백성이 기계로 이어 산다고 말하지 않는다 — 그 얼굴을 처음 쓰는 커밋이 함께 들고, 깨짐 하나('유령'을 지닌 얼굴 글)를 둔다(헤더 §3.3 규칙 8).", "L2-9 n")
rep(N, "`LetPeopleGo`(절멸)는 백성을 낳지 않는다(`FossilAutomata`는 얼굴만); ini에 `Undying*` 잎이 없다. 포자의 추첨은 고정 수 하나",
    "`LetPeopleGo`(절멸)는 백성을 낳지 않는다(`FossilAutomata`는 얼굴만); ini에 `Undying*` 잎이 없다. 그 헤더가 정한 사슬의 규칙을 시험한다 — 끊으면 포자로 태어난 백성이 지워질 때 요청이 생기지 않고(변이: 그 거름을 뺀다), 한도를 두면 한도의 세대에서 요청이 서지 않는다(변이: 한도를 하나 늘린다). 포자의 추첨은 고정 수 하나", "L2-3 n")
rep(N, "K6은 코드는 서고 글(OVERVIEW html·설계서 원안·DevComment)이 깬다.",
    "K6은 코드는 서고 글(OVERVIEW html·설계서 원안·DevComment)이 깬다(답 21: 설계서 원안과 OVERVIEW html은 동결 표지를 단 동결본으로 `docs/archive`에 옮겼다 — §5.4 `[B-7x0]` 3·4; DevComment는 넘긴 고침 코6(`[B-7x2]`, T6)이 고치고 `[B-7x4]`의 글 감사(F11)가 다시 본다).", "L4-15")

# ---------------- DESIGN ----------------
D = DP
rep(D, "(C10의 제목과 ②의 윤리 이동 — 노트 §5.4 데2)",
    "(C10의 제목·① 결과 줄의 '왕좌는 비어 있었다'·목소리 둘의 '견딘 날들 내내'·②의 윤리 이동과 그 까닭 한 줄(`petition_exalt`), C08 목소리 둘의 '견딘 날이 길었다' — 노트 §5.4 데2)", "L4-18")
rep(D, "아래 §4.3·§4.4다.",
    "아래 §4.3·§4.4다. 제목은 원안의 것을 그대로 두었지만 그 틀은 받지 않는다 — 살아낸 시간이 쌓이는 곳은 플레이어가 아니라 백성이고(헤더 K10·결정 4), 칸은 업적 행으로만 오르며 블록·변이·카드는 칸을 올리지도 내리지도 않는다(헤더 K4). 한계 없는 진화는 없다.", "L2-8")
rep(D, "둘째 갈림의 ②는 닫지 않는다 — 지난 세 답이 지나면 다시 오므로",
    "둘째 갈림 여섯 장(C05~C08·C10·C11)의 ②는 닫지 않는다(C09의 ②는 하나뿐인 되돌림이라 판정에 성공하면 걷힌 부모로 닫히고, 실패하면 안정도만 잃는다) — 지난 세 답이 지나면 다시 오므로", "L1-08")
rep(D, "(② — 그릇을 높이는 쪽이 회중이다)",
    "(② — 다스리는 이의 이름을 칙령 머리에 남기자는 쪽이 회중이다; 그릇은 높아지지 않는다)", "L2-12")
rep(D, "(c) 같은 백성을 다스린 생애가 여럿이면 마지막 생애의 장에만 붙인다 — 시간 셈은",
    "(c) 같은 백성을 다스린 생애가 여럿이면(`RuledCivIDs`의 어느 자리든) 마지막 생애의 장에만 붙인다 — 그 마지막 생애의 마지막 재위 백성이 다른 백성이면 (a)대로 그 백성의 낱말은 어느 장에도 붙지 않는다. 시간 셈은", "L3-5 d")
rep(D, "`Unlink`·`Link`(창구의 규칙 그대로).",
    "`Unlink`·`Link`(창구의 규칙 그대로) — 끊는 레인과 걸음이 서지 못하는 틱(새 터가 이미 이웃·창구의 거절)의 걸음은 슬라이스 나의 헤더가 정한다(기본값: 그 걸음은 없고 미루지 않는다 — 헤더 결정 15).", "L4-12 d")

if errors:
    print("\n".join(errors))
    sys.exit(1)
for p, t in texts.items():
    with open(p, "wb") as f:
        f.write(t.encode("utf-8"))
print("ok")
