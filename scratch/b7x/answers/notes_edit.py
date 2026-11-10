import sys

PATH = sys.argv[1]
SCRIPT = "C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad/b7x/answers/rename/rename_leaves.py"

raw = open(PATH, 'rb').read()
assert b'\r' not in raw
text = raw.decode('utf-8')
assert not text.startswith('\ufeff')

def rep(old, new, count=1):
    global text
    n = text.count(old)
    if n != count:
        raise SystemExit(f"count {n} != {count} for: {old[:80]!r}")
    text = text.replace(old, new)

def rep_line(prefix, new):
    global text
    lines = text.split('\n')
    hits = [i for i, l in enumerate(lines) if l.startswith(prefix)]
    if len(hits) != 1:
        raise SystemExit(f"line hits {len(hits)} for prefix {prefix!r}")
    lines[hits[0]] = new
    text = '\n'.join(lines)

# ---- head
rep("설계서는 `docs/10_DESIGN/EXISTENCE_EVOLUTION_AND_ECOLOGY.md`다.",
    "설계서는 받은 꼴(§3.3·§4.3·§4.4·§5)만 남은 `docs/10_DESIGN/EXISTENCE_EVOLUTION_AND_ECOLOGY.md`이고, 원안(아래 §1의 '원안이 바라는 것' 열의 출처)은 고치지 않는 동결본 `docs/archive/EXISTENCE_EVOLUTION_original_2026-11-09.md`다(결정 21) — 설계서는 절과 규칙·카드 번호로(§3.3 S14, §4.3 C10), 원안은 절 제목으로('원안 동결본 §2 축 5') 부르고 줄 번호로 부르지 않는다. 헤더 §10의 결정은 2026-11-10 개발자가 모두 권장대로 답했다 — 이 노트의 글은 그 답의 꼴이다.")

# ---- section 1
rep("## 1. 예순 블록의 처분표\n\n",
    "## 1. 예순 블록의 처분표\n\n선언하는 잎 가운데 여덟은 결정 3의 새 이름으로 적고 원안의 이름을 괄호에 둔다(이름은 세이브에 남는다 — `[B-7x1]`이 main에 들기 전에 코드·데이터의 자리를 모두 고친다, §5.4 `[B-7x1]`의 코7).\n\n")
rep("### 1.1 형태 (`Brick.Form.*` — 14)", "### 1.1 형태 (`Brick.Form.*` — 14; 원안 동결본 §2 축 1)")
rep("### 1.2 섭식 (`Brick.Diet.*` — 14)", "### 1.2 섭식 (`Brick.Diet.*` — 14; 원안 동결본 §2 축 2)")
rep("### 1.3 공간 (`Brick.Space.*` — 12)", "### 1.3 공간 (`Brick.Space.*` — 12; 원안 동결본 §2 축 3)")
rep("### 1.4 본능 (`Brick.Instinct.*` — 8)", "### 1.4 본능 (`Brick.Instinct.*` — 8; 원안 동결본 §2 축 4)")
rep("### 1.5 변이 (`Brick.Mutation.*` — 12)", "### 1.5 변이 (`Brick.Mutation.*` — 12; 원안 동결본 §2 축 5)")

rep("| `DysonBrain` | 통찰과 인과의 폭발 |", "| `ShutSphere`(원안 `DysonBrain`) | 통찰과 인과의 폭발 |")
rep("| 세계는 늘 확정 — 정착된 성계가 레인으로 닿거나 그릇이 서면 드러나고, '어디에나'는 표현만 | 받지 않음(㉡은 뒤 블록) |",
    "| 세계는 늘 확정 — 정착된 성계가 레인으로 닿거나 그릇이 서면 드러난다; '어디에나'는 받지 않는다 — 표현도 무주 성계 위에 그 백성을 그리지 않는다; 남는 것은 S15의 소문 한 줄이고 그 줄은 제 별 곁에만(결정 18) | 받지 않음 |")
rep("| `FossilAutomata` | 절멸 뒤 남은 기계의 나라 | 절멸의 만료에 새 추첨 없음, 백성의 탄생 | 건넘 단계의 요청으로 | 다 |",
    "| `FossilAutomata` | 절멸 뒤 남은 기계의 나라 | 절멸의 만료에 새 추첨 없음, 백성의 탄생, 같은 백성이 기계로 이어 삶 | 절멸(`LetPeopleGo`)에서 태어나지 않는다 — 글이나 스캔 얼굴로만(결정 16 (3a)); 원안의 불사 분열·유령 제국은 받지 않는다 | 글 |")
rep("| `UnseenLaw` | 무형의 법·신앙이 됨 | 누구도 법이 '되지' 않는다(K6) | 왕좌가 빈 뒤에도 지닌 칙령을 하나도 거두지 않는 백성의 표지('남긴 법' — 은하의 법칙은 이미 누구나 지킨다) | 가 |",
    "| `KeptLaw`(원안 `UnseenLaw`) | 무형의 법·신앙이 됨 | 누구도 법이 '되지' 않는다(K6), 주장 고리(K9), 법칙 셋의 자리 | 왕좌가 빈 뒤에도 지닌 칙령을 거두지 않는 백성의 표지('남긴 법' — 은하의 법칙은 이미 누구나 지킨다) — 다만 주장의 카운트다운에 더하는 칙령과 위기의 답인 칙령은 왕좌가 빈 뒤 여느 때처럼 거두고(K9 — B6b 결정 9가 은하 법칙에 그 고리를 금지한 선과 같다), 판정받는 행이 법칙을 물으면(넷째 칸 — `CosmicEdicts::AsksForLawsOf`) 백성 범위 칙령은 다른 AI처럼 거둔다(칙령 자리 셋이 법칙 셋의 자리다, B6b 결정 10·11) | 가 |")
rep("| `SilenceFeed` | 사건 없는 틱을 먹음 | 사건은 세계에 기록이 없다, 침묵의 낱말 | 일 없는 틱(위기·사업·공성 없음)의 통찰; 잎 이름을 고친다 | 가 |",
    "| `IdleFeed`(원안 `SilenceFeed`) | 사건 없는 틱을 먹음 | 사건은 세계에 기록이 없다, 침묵의 낱말('Quiet'은 참된 계시의 낱말 `Revelation.Quiet`이라 쓰지 않는다) | 일 없는 틱(위기·사업·공성 없음)의 통찰 — 노브 `BrickIdleInsightPerTick`, 술어 `IsIdleOf` | 가 |")
rep("| 레인의 창구 위에서 — 먼저 레인, 다음에 좌표 | 나 |",
    "| 레인의 창구 위에서 — `project_settle`이 끝난 틱에 수도의 레인 하나를 새 터의 이웃으로 옮긴다(설계서 §3.3 S14, 결정 15 — 걸음이 사건에 매여 주기 노브가 없다); 좌표는 창구가 받은 뒤 둘째 걸음 | 나 |")
rep("| `HermitMonolith` | 한 성계", "| `ClosedPort`(원안 `HermitMonolith`) | 한 성계")
rep("| `PlanckMicro` | 무주처럼", "| `FoldedSmall`(원안 `PlanckMicro`) | 무주처럼")
rep("| 탄생의 요청 | 다 |", "| 탄생의 요청 — 태어난 것은 새 백성이다(결정 16 (1)·(2)) | 다 |")
rep("| 고정 수의 추첨과 정착의 문 | 다 |",
    "| 고정 수의 추첨 하나(결정 22 — Phase 5 전에만 더할 수 있다)와 정착의 문; 태어난 것은 새 백성이다(결정 16 (1)·(2)) | 다 |")

rep("| `StarShepherd` | 머물지", "| `StarTow`(원안 `StarShepherd`) | 머물지")
rep("| `KesslerTapestry` | 엮은", "| `DustBand`(원안 `KesslerTapestry`) | 엮은")
rep("| `PlanckAscension` | `PlanckMicro`에 합친다 |", "| `PlanckAscension` | `FoldedSmall`에 합친다 |")
rep("| `LivingCovenant` | `UnseenLaw`의 받은 꼴에 합친다 |", "| `LivingCovenant` | `KeptLaw`의 받은 꼴에 합친다 |")
rep("| `AbyssalMaw` | 삼키는", "| `InwardHunger`(원안 `AbyssalMaw`) | 삼키는")
rep("| `UndyingAutomata` | `FossilAutomata`와 같다 | 다 |",
    "| `UndyingAutomata` | 선언하지 않는다(결정 16 (4) — 잎 `Undying*`은 없다); `FossilAutomata`처럼 얼굴만, 불사 분열은 받지 않는다 | 받지 않음 |")

rep_line("슬라이스 가가 선언하는 태그는 열일곱이다:",
    "슬라이스 가가 선언하는 태그는 열일곱이다: 형태 넷(`Society`·`LivingNebula`·`ShutSphere`·`KeptLaw`), 섭식 다섯(`DarkMatter`·`StarEater`·`IdleFeed`·`DistanceTension`·`CrisisParasite`), 공간 셋(`Nomad`·`ClosedPort`·`FoldedSmall`), 본능 둘(`SwarmTech`·`HiveMerge`), 변이 셋(`StarTow`·`DustBand`·`InwardHunger`). 둘째 갈림이 새 태그를 더하지 않아(결정 11 (나)) 열일곱 그대로다. 변이의 잎은 꼴의 이름을 받지 않는다 — 꼴 낱말은 변이 없이도 부모를 지닌 모든 백성에 보인다(결정 3).")

# ---- section 2
rep("| ㉢→결정 4 | 백성에만 |", "| ㉢→결정 4 | 백성에만 — 결정 13의 꼴 표지(㉢)가 화면에서도 지킨다 |")
rep("| ㉠ | 레인의 단일 창구(결정 14·15) |",
    "| ㉠ | 레인의 단일 창구(결정 14·15 — 슬라이스 나): `SystemDragger`는 정착이 끝난 틱에 수도의 레인 하나를 새 터의 이웃으로(S14), 좌표는 둘째 걸음 |")
rep("| ㉠ | `HostCivID` — 주인은 그대로(결정 6) |",
    "| ㉠ | `HostCivID` — 주인은 그대로; 지운 백성의 이름은 같은 걸음에서 비우고, 태어난 백성도 `NextCivSerial`에서 이름을 받는다(결정 6 (ㄱ)·(ㄴ)) |")
rep("| ㉠ | 요청을 건넘 단계가 받는다(결정 16) |",
    "| ㉠ | 요청을 건넘 단계가 받는다 — 태어난 것은 새 백성이고 남은 기계는 절멸에서 태어나지 않는다(결정 16 (1)~(4)) |")
rep("| ㉢ | 세계의 사실로 드러나고 표현만 가린다 |",
    "| ㉢ | 세계의 사실로 드러난다; '어디에나'는 표현으로도 받지 않는다(S15의 소문 한 줄뿐 — 결정 18) |")
rep("| ㉢→㉡ | '남긴 법' — 왕좌가 빈 뒤에도 거두어지지 않는 칙령 |",
    "| ㉢→㉡ | '남긴 법' — 왕좌가 빈 뒤에도 거두어지지 않는 칙령; 주장 고리·위기의 답인 칙령과 법칙을 묻는 행 아래의 백성 범위 칙령은 빼고(결정 18) |")
rep("| ㉡ | 사실 줄의 낱말 하나(결정 13) |",
    "| ㉡ | 사실 줄의 낱말 하나 — 저장 없이, 생애의 마지막 재위 백성이 `Civs`에 있을 때만(결정 13 ㉢) |")
rep("| ㉠ | 'B6b 끝의 수 + 청원 11·조우 1'(결정 11) |",
    "| ㉠ | 'B6b 끝의 수 + 청원 11·조우 1', 둘째 갈림의 ②는 되풀이 이득 없이(결정 11 (나)) |")
rep("| ㉠ | 조건 하나·기생의 두 국경 항·표류 목표의 블록 항만 VS — 다른 국경 배율은 EA(결정 9) |",
    "| ㉠ | 조건 하나·기생의 두 국경 항·표류 목표의 블록 항만 VS — 다른 국경 배율은 EA(결정 9 — [개발자]로 올리지 않고 R11로 받았다) |")
rep("| ㉠ | `UnseenLaw` 백성만 — 주장 고리·위기의 답·법칙을 묻는 행 아래는 빼는 좁힘이 결정 18에 걸려 있다 |",
    "| ㉠ | `KeptLaw` 백성만 — 주장 고리·위기의 답인 칙령과, 행이 법칙을 물을 때의 백성 범위 칙령은 여느 AI처럼 거둔다(결정 18의 좁힘이 R12를 닫는다) |")
rep("| ㉠ | 파생 정수 용도 13, 노브가 비면 꺼짐(결정 23) |",
    "| ㉠ | 파생 정수 용도 13, 노브가 비면 꺼짐(결정 23 — [개발자]로 올리지 않고 R13으로 받았다) |")

# ---- section 3
rep("## 3. 새 열린 물음의 초안 (`OPEN_QUESTIONS.md`에 승인 커밋이 더한다 — 번호는 그때의 마지막 번호 다음부터 매긴다: `10205b6`의 끝은 #163이고 B6b 헤더가 #164를 잡았다; 아래 '새1'~'새8'은 자리표다 — 리뷰 I10)\n\n",
    "## 3. 새 열린 물음의 초안 (`OPEN_QUESTIONS.md`에 승인 커밋이 더한다 — 번호는 그때의 마지막 번호 다음부터 매긴다: `10205b6`의 끝은 #163이고 B6b 헤더가 #164를 잡았다; 아래 '새1'~'새11'은 자리표다 — 리뷰 I10)\n\n"
    "2026-11-10 개발자가 헤더 §10의 결정을 모두 권장대로 답했다. 아래 '권장안' 칸은 받은 답과 남은 물음이다 — 답으로 다 닫힌 새1은 `OPEN_QUESTIONS.md`에 넣지 않고 `DECISIONS_LOG`의 B7x 절에만 적는다; 나머지는 뒤 슬라이스·측정·손 확인이 답할 것이 남아 행으로 든다. 새9~새11은 답이 뒤로 넘긴 물음이다. 번호는 승인 커밋 때 다시 센다(2026-11-10 main `220129d`의 끝은 #167).\n\n")

rep_line("| 새1 |",
    "| 새1 | 블록의 뿌리와 '섭식 태그 없음' 판정(B7x 헤더 결정 2·3) | 답(닫힘 — 행으로 넣지 않는다): `Brick.` 뿌리, 읽는 곳이 있는 열일곱만 선언, 잎 여덟의 이름을 `[B-7x1]`이 main에 들기 전에 고친다(목록은 헤더 결정 3 — 변이의 잎은 꼴의 이름을 받지 않는다), 업적 행은 블록을 묻지 않는다(K4 — 헤더 §3.3 규칙 7). 고르지 않은 것: `Trait.` 아래 | §5.3·§5.11 |")
rep_line("| 새2 |",
    "| 새2 | 시간 셈의 주인과 술어(결정 4·5) | 답: 백성에만(결정 4), 누적(㉡ — R에서 뺐다); 끝의 꼴의 표시는 저장 없이 마지막 재위 백성의 지금 셈만 읽어 앞 생애의 장에 뒤 생애의 셈을 붙이지 않는다(결정 13 ㉢ — 결정 4를 지킨다). 남은 물음: 술어 여섯의 시작값과 결정 5의 대안 ㉯('삼킨 시간'은 공성만) — 묶음이 분포를 잰 뒤 | §7.2 |")
rep_line("| 새3 |",
    "| 새3 | 갈림 카드와 대기 칸(결정 10·11·19) | 답: `Lottery`와 `CivLacksTrait`; 둘째 갈림의 ②는 되풀이 이득 없이(리뷰 F7의 (나) — 새 태그 없음, 열일곱 그대로); 수량 줄은 'B6b 끝의 수 + 청원 11·조우 1'; 측정 정책은 갈림 카드에서 ③만. 남은 물음: 묶음에서 대기 칸 잃음(헤더 §5 ③)과 방랑자 갈림 카드가 증언·도착 카드를 밀어내는 몫(③′)을 잰다(#158과 함께) | §12.1 |")
rep_line("| 새4 |",
    "| 새4 | 레인의 창구와 실제 하늘(결정 14·15·17) | 답: 창구는 슬라이스 나에서 받는다(고립 금지는 살아 있는 성계의 `Link`·`Unlink`에만 — 파괴의 끊음으로 생기는 고립은 받는다); `SystemDragger`는 `project_settle`이 끝난 틱에 수도의 레인 하나를 새 터의 이웃으로 옮긴다(설계서 §3.3 S14 — 주기 노브 없음); 좌표는 창구가 받은 뒤 둘째 걸음; 레인 시간은 B8 헤더와 한 번에. 남은 물음은 새10 | §6.4·§9.2 |")
rep_line("| 새5 |",
    "| 새5 | 판 도중의 탄생과 6a(결정 16·22) | 답: 건넘 단계의 요청, 포자에 고정 수의 추첨 하나(Phase 5 전에만); 태어난 것은 새 백성이다 — 새 `CivID`(`NextCivSerial`)·이름 뱅크의 새 이름·셈 0, 부모의 행·특성(탄생 블록 하나만 빼고)·관계·칙령·`HostCivID`를 물려받지 않는다; 그 줄은 살아남음·죽지 않음·돌아옴·다시 태어남을 말하지 않는다(스키마 규칙 하나 — 슬라이스 다); 남은 기계는 절멸에서 태어나지 않고 얼굴만; 잎 `Undying*`은 없다(헤더 결정 16 — 리뷰 T4). 남은 물음: 슬라이스 다 헤더 — 요청의 자리(새9), 품는 쪽 수입의 몫(R14), `MirrorProjection`과 S13의 세 문 | §7.4 |")
rep_line("| 새6 |",
    "| 새6 | 끝의 꼴 표지의 자리와 글(결정 13) | 답(결정 13의 꼴 표지 ㉢ — 저장 없음; §0의 처리 ㉢과 뜻이 다르다): (a) 그 생애의 마지막 재위 백성(`Life.RuledCivIDs`의 끝)만, 다스린 적 없는 생애에는 없다; (b) 그 백성이 `Civs`에 있을 때만; (c) 같은 백성을 다스린 생애가 여럿이면 마지막 생애의 장에만; (d) '그 백성의 지금 살림'으로 읽히게 쓰고 판의 끝 뒤 다시 열면 달라질 수 있음을 받는다. 남은 물음: 손 확인에서 칭호나 승리로 읽히면 뺀다(#163과 함께). 고르지 않은 것: ㉠ `ReignShapes` 저장·㉣ 백성이 주어인 자리에만 | §8.4 |")
rep("| 새8 | B7x의 값과 B6b의 경주(결정 1·23, 헤더 §5 판단 ⑥ — 리뷰 M1·I4) | 기본값: 어긋나면",
    "| 새8 | B7x의 값과 B6b의 경주(결정 1·23, 헤더 §5 판단 ⑥ — 리뷰 M1·I4) | 답(결정 1): 어긋나면")
rep("; 한 번 고쳐도 어긋나면 그 수와 함께 [개발자] | §9.3 |",
    "; 한 번 고쳐도 어긋나면 그 수와 함께 다시 묻는다. 남은 물음: `[B-7x5s]`의 판단 ⑥이 낼 수(고침은 `[B-7x5r]`) | §9.3 |\n"
    "| 새9 | 탄생 요청의 자리(결정 6 (ㄷ)·16 — 슬라이스 다) | 기본값(리드의 판단 — 개발자가 뒤집을 수 있다): 탄생 요청 필드는 `[B-7x1]`에 예약하지 않는다(꼴을 정할 명세가 없다). 슬라이스 다의 헤더가 먼저 요청을 한 틱 안에서 끝내는 꼴(필드 없음)을 찾고, 필드가 들면 그 헤더가 먼저 적은 뒤 두 해시가 한 번 더 옮는다. 같은 헤더가 품는 쪽만 남고 제 성계가 없는 백성이 건넘에서 지워지는지(결정 6 (ㄱ)의 끝)를 정한다 | §7.2·§7.4·§9.2 |\n"
    "| 새10 | 슬라이스 나 헤더가 먼저 답할 셋(결정 14 — 리뷰 F8 (iii)~(v)) | 기본값: (iii) 헤더 §2.10 표에 레인 걸음의 행(단계와 같은 틱의 차례); (iv) 슬라이스 나에 둔 블록 — `VoidWeb`·`OmniLinked`의 `BeyondReach`(정착·공성의 새 사유)와 `WebReachLanes`(S10), `OrbitShaper`의 `Link`, `Labyrinth`(방랑자 이동의 거절 사유) — 을 헤더 §2.8에 적거나 이 노트 §1의 자리를 B7이나 글로 옮긴다; (v) 레인 걸음마다 `ClaimantOf`·영향권·주장 규칙을 같은 틱에 다시 읽는 차례와 레인 걸음이 주장을 풀거나 늦추지 않는다는 시험(K9), 모르는 성계로 잇는 `Link`가 `NoteKnown`을 쓰는지(규칙 2). `CLAUDE.md` 규칙 13의 낱말도 그 블록에서 | §6.4·§9.2 |\n"
    "| 새11 | 남긴 법 백성이 섭정 아래에서 잃는 안정도(결정 18의 남는 귀결 — 헤더 §5 ⑧) | 기본값: 좁힘은 받았다 — 주장과 법칙의 문은 여느 백성과 같다; 전시 동원을 쥔 채 양위한 백성이 섭정 아래에서 안정도를 잃는 것은 설계가 노린 값이다. 답은 `-Crossroads`의 수보다 먼저 왔다 — ⑧(안정도 0.2 아래 첫 틱·반란·지워짐·`LawsHeldTicks`)이 놀라운 값을 내면 노브(`[B-7x5r]`)로 다루고, 그래도 안 되면 그 수와 함께 다시 묻는다 | §5.8·§9.3 |")

rep("**있는 행의 고침**(승인 커밋 — 헤더 §0의 R11·R13, 리뷰 F4):",
    "**있는 행의 고침**(승인 커밋 — 헤더 §0의 R11·R13, 리뷰 F4; 결정 9·23은 [개발자]로 올리지 않고 이 R로 받았다, 2026-11-10):")
rep("'(B7x — `Brick.Form.UnseenLaw`를 지닌 백성은 예외, B7x 헤더 결정 18·R12)'",
    "'(B7x — `Brick.Form.KeptLaw`를 지닌 백성은 예외 — 주장의 카운트다운에 더하는 칙령·위기의 답인 칙령과, 그 백성의 행이 법칙을 물을 때의 백성 범위 칙령은 여느 AI처럼 거둔다; B7x 헤더 결정 18·R12)'")

# ---- section 4
rep("그 문구의 충돌 목록(§4a)과 해소 규칙('양쪽을 다 살리고 main 쪽을 앞에 둔다')은 틀렸다(리뷰 I1) — 헤더 §8.1을 따른다.\n",
    "그 문구의 충돌 목록(§4a)과 해소 규칙('양쪽을 다 살리고 main 쪽을 앞에 둔다')은 틀렸다(리뷰 I1) — 헤더 §8.1을 따른다.\n\n"
    "2026-11-10: 개발자의 답이 왔다(헤더 §10 — 모두 권장대로). `promptforother.txt`는 `[B-7x0]`을 기다리지 않고 이 브랜치에서 지웠다(결정 21 (iii)).\n")

# ---- 5.1
rep("| 남긴 법 — 칙령을 거두지 않는다 | `Sim/CosmicEdicts.cpp`의 `AIRevokesOf` |",
    "| 남긴 법 — 칙령을 거두지 않는다(결정 18의 좁힘은 `[B-7x2]`의 코1이 넣는다 — §5.4) | `Sim/CosmicEdicts.cpp`의 `AIRevokesOf` |")
rep("잇지 않은 것: 별 등급의 어두워짐(함수만 — 결정 8).",
    "잇지 않은 것: 별 등급의 어두워짐(함수만 — 결정 8의 답: 장치는 넣고 주기 0으로 꺼 둔다, 켤지는 B8의 실제 하늘 헤더; 거절 갈래 셋의 단언은 §5.4 시6).")

# ---- 5.3
rep("`CLAUDE.md` 규칙 13은 슬라이스 나의 몫이다(헤더 §7 — 리뷰 F12).",
    "`CLAUDE.md` 규칙 13은 슬라이스 나의 몫이다(헤더 §7 — 리뷰 F12, 결정 14). `CLAUDE.md`의 `docs/archive/` 행은 `[B-7x0]`이 고쳐 쓴다(§5.4).")

# ---- 5.4 intro
rep("괄호는 발견 번호, '대기 N'은 헤더 §10 결정 N의 답을 기다린다는 뜻이다.",
    "괄호는 발견 번호이고, '답 N'은 헤더 §10 결정 N의 답(2026-11-10 — 개발자가 모두 권장대로 답했다)을 따른다는 뜻이다(리뷰 때 '대기 N'이던 자리).")

# ---- B-7x0
rep("2. `promptforother.txt`를 지운다(F12). 증명: 파일이 없고 노트 §4·설계서 §5가 그것을 부르지 않는다.",
    "2. `promptforother.txt`(F12) — 답 21 (iii): 이 브랜치에서 지웠다(2026-11-10, `git rm` — 그 파일 스스로 받은 뒤 지워도 된다고 했다). 증명: 파일이 없고 노트 §4·설계서 §5가 그것을 부르지 않는다.")
rep("3. `docs/OVERVIEW_as_of_B6a.html` — ㉮ 뺀다 / ㉯ 받은 꼴로 다시 쓴다(T1) — **대기 21**.",
    "3. `docs/OVERVIEW_as_of_B6a.html`(T1) — 답 21 (ii): 이 브랜치에서 `docs/archive/OVERVIEW_as_of_B6a.html`로 옮기고(`git mv`) 맨 위에 보이는 동결 표지(받지 않은 원안을 포함한다 — 현행 설계가 아니다; 현행은 `docs/`)를 두었다(2026-11-10). 고르지 않은 것: ㉮ 뺀다·㉯ 받은 꼴로 다시 쓴다. main의 추적하지 않는 `docs/OVERVIEW.html`은 다른 파일이다 — 건드리지 않는다.")
rep("4. 설계서 원안의 동결본(`docs/archive/EXISTENCE_EVOLUTION_original_2026-11-09.md`)과 §4.4의 '§4.2 원안의 줄' 열(T5·T3) — **대기 21**(열은 13과 함께).",
    "4. 설계서 원안의 동결본과 §4.4의 '§4.2 원안의 줄' 열(T5·T3) — 답 21 (i)·13: 이 브랜치에서 했다(2026-11-10). 9be5b3e의 설계서를 글자 그대로 `docs/archive/EXISTENCE_EVOLUTION_original_2026-11-09.md`에 두고(맨 위에 동결 표지 한 덩이만 — 받지 않은 것을 포함한다, 고치지 않는다), `docs/10_DESIGN/EXISTENCE_EVOLUTION_AND_ECOLOGY.md`에는 머리의 상태 표지·동결본으로 가는 링크 한 줄·받은 꼴(§3.3·§4.3·'원안의 줄' 열을 뺀 §4.4·§5 — 절 번호와 제목은 그대로)만 남겼다. 설계서를 줄 번호로 부르던 곳은 절과 규칙·카드 번호로, 원안은 절 제목으로 부른다.")
rep_line("5. 결정 3·13·18의 답을 헤더·노트·설계서의 글에",
    "5. 결정 3·13·18의 답을 헤더·노트·설계서의 글에(T2·F10, F9·DET-5·T3·I12, F1·I2·M11·T10) — 이 브랜치에서 했다(2026-11-10): 잎 여덟의 새 이름(옛 이름은 '옛 → 새'의 기록·동결본·헤더 §11의 지난 기록에만), 결정 13 ㉢의 세 규칙(헤더 §4 첫 항·설계서 §4.4), 결정 18의 좁힘(헤더 §2.4 행·설계서 §4.3 C10의 귀결·노트 §1.1 `KeptLaw` 행)과 노트 §1.1 `QuantumCloud` 행의 고침.\n"
    "6. `CLAUDE.md`의 `docs/archive/` 행과 `docs/SECTION_MAP.md`(결정 21 — 이 브랜치는 둘을 고치지 않는다). `CLAUDE.md`에는 줄을 더하지 않고(16KB 예산 — 헤더 §7) 그 행의 둘째 칸을 목록 없이 짧게 고쳐 쓴다 — 지금 칸은 동결본 넷의 이름을 늘어놓는다:\n\n"
    "   ```\n"
    "   | `docs/archive/` | 옛 판·원안의 동결본(고치지 않는다 — 목록은 `docs/SECTION_MAP.md`) |\n"
    "   ```\n\n"
    "   목록은 `SECTION_MAP`이 진다 — 넷은 이미 거기 있다(`guideline_v0.8_2026-10-18.md`는 머리에, 셋은 '옛 WORKPLAN.md·루트 md → 새 위치' 표에). 그 표의 끝에 두 줄을 더한다:\n\n"
    "   ```\n"
    "   | docs/10_DESIGN/EXISTENCE_EVOLUTION_AND_ECOLOGY.md의 원안(9be5b3e, 2026-11-09) | docs/archive/EXISTENCE_EVOLUTION_original_2026-11-09.md (받지 않은 것을 포함한 동결본 — 받은 꼴은 docs/10_DESIGN/EXISTENCE_EVOLUTION_AND_ECOLOGY.md와 B7x 헤더) |\n"
    "   | docs/OVERVIEW_as_of_B6a.html (design/b7x 브랜치) | docs/archive/OVERVIEW_as_of_B6a.html (B6a 때의 한눈 보기와 받지 않은 B7x 원안 — 현행 설계가 아니다; 현행은 docs/) |\n"
    "   ```\n\n"
    "   증명: `CLAUDE.md`의 바이트가 늘지 않고 `check_docs` 0.")

# ---- B-7x1
old_b1_2 = [l for l in text.split('\n') if l.startswith("2. 코7의 태그·접근자(T2·F10)")]
assert len(old_b1_2) == 1
rep_line("2. 코7의 태그·접근자(T2·F10)",
    "2. **코7 — 잎 이름**(T2·F10) — 답 3: 이름은 세이브에 남으므로 이 커밋이 main에 들기 전에 코드·데이터의 모든 자리를 고친다(`[B-7x2]`와 하나로 합친 커밋이라 노브와 함수도 여기서). 옛 이름 → 새 이름:\n"
    "   - `Brick.Form.UnseenLaw` → `Brick.Form.KeptLaw`(접근자 `BrickFormUnseenLaw` → `BrickFormKeptLaw`)\n"
    "   - `Brick.Diet.SilenceFeed` → `Brick.Diet.IdleFeed`(`BrickDietSilenceFeed` → `BrickDietIdleFeed`; 처음 권장 `QuietFeed`는 쓰지 않는다 — `Revelation.Quiet`)\n"
    "   - `Brick.Space.PlanckMicro` → `Brick.Space.FoldedSmall`(`BrickSpacePlanckMicro` → `BrickSpaceFoldedSmall`)\n"
    "   - `Brick.Form.DysonBrain` → `Brick.Form.ShutSphere`(`BrickFormDysonBrain` → `BrickFormShutSphere`)\n"
    "   - `Brick.Mutation.KesslerTapestry` → `Brick.Mutation.DustBand`(`BrickMutationKesslerTapestry` → `BrickMutationDustBand`)\n"
    "   - `Brick.Mutation.AbyssalMaw` → `Brick.Mutation.InwardHunger`(`BrickMutationAbyssalMaw` → `BrickMutationInwardHunger`)\n"
    "   - `Brick.Mutation.StarShepherd` → `Brick.Mutation.StarTow`(`BrickMutationStarShepherd` → `BrickMutationStarTow`)\n"
    "   - `Brick.Space.HermitMonolith` → `Brick.Space.ClosedPort`(`BrickSpaceHermitMonolith` → `BrickSpaceClosedPort`)\n"
    "   - 노브 `BrickQuietInsightPerTick` → `BrickIdleInsightPerTick`(`FCosmicRulesDef`·`DT_Rules.json`·`schema.json`·`docs/30`), 함수 `CosmicBricks::IsQuietOf` → `IsIdleOf`\n\n"
    "   준비한 스크립트 `" + SCRIPT + "`(리베이스하는 세션이 쓴다)로 한 번에 바꾼다 — 대상: ini(`Config/DefaultGameplayTags.ini:172-184` — 'leaf rename to QuietFeed proposed'(`:176`)의 말은 지운다)와 DevComment, `Core/CosmicTags.h`의 접근자, `Sim/CosmicExistenceBricks.h/.cpp`, `Data/CosmicDefinitionRows.h`의 주석, 테스트(`Tests/CosmicExistenceBricksTests.cpp` — `:576-591`·`:923`·`:960` 포함)와 그 메시지, `Tools/data/schema.json`(규칙 4의 나열)·`DT_Rules.json`·`DT_Encounters.json`·`DT_Events.json`. 바꾼 뒤 옛 이름 아홉(`QuietFeed` 포함)이 `Source/`·`Config/`·`Tools/`에 남지 않았음을 grep으로 본다 — 세이브에 남는 것은 새 이름뿐이다. 증명: `Sim.SaveTimeVectors`·`Data.Integrity`; ini는 `generated.sha1`에 들지 않는다(main `Tools/make_data_tables.py:267`).")
rep_line("3. 결정 13 ㉠(결정 13의 꼴 표지",
    "3. `ReignShapes`는 들지 않는다(DET-5) — 답 13은 ㉢(저장 없음 — 표시가 생애의 마지막 재위 백성의 지금 셈만 읽는다). `FLifeRecord`는 그대로다.")
rep_line("4. 결정 6 (ㄷ)이면 탄생 요청 필드(F8)",
    "4. `HostCivID`(F8·DET-9 — 답 6; branch 코드에 이미 있다): `FStarSystemState`의 `FName HostCivID`, 초기값 `NAME_None`(규칙 7 — 뜻이 언다: 그 성계의 주인을 품는 백성, 주인은 그대로). 필드 주석과 `docs/21`에 두 규칙을 적는다 — (ㄱ) 건넘 단계가 지운 백성의 이름은 같은 걸음에서 모든 성계의 `HostCivID`에서도 비운다(추첨 없음 — `PendingContactCivID`를 비우는 꼴); (ㄴ) 탄생한 백성도 `NextCivSerial`에서 `CivID`를 받는다(지운 이름은 다시 쓰이지 않는다). 읽고 쓰는 코드와 (ㄱ)의 시험은 슬라이스 다다(아래) — 그때까지 쓰는 곳이 없어 늘 `NAME_None`이다. 탄생 요청 필드는 예약하지 않는다 — 답 6 (ㄷ)(리드의 판단, 개발자가 뒤집을 수 있다; 이 노트 §3 새9). 증명: 두 해시가 이 커밋에서만 옮고 옛 세이브가 `NAME_None`으로 읽힌다(`Sim.SaveTimeVectors`).")

# ---- B-7x2
rep("- **코1**(F1·I2·M11·F4·T6·F11) — **대기 18**(권장안은 상시 승인 안): `Sim/CosmicEdicts.cpp:344-367`의 `AIRevokesOf`를 아래로 바꾼다.",
    "- **코1**(F1·I2·M11·F4·T6·F11) — 답 18(좁힘): 남긴 법 백성이라도 왕좌가 빈 뒤 (a) 주장의 카운트다운에 더하는 칙령과 위기의 답인 칙령은 여느 AI처럼 거두고, (b) 그 백성의 행이 법칙을 물으면(`CosmicEdicts::AsksForLawsOf`) 백성 범위 칙령을 다른 AI처럼 거둔다. `Sim/CosmicEdicts.cpp:344-367`의 `AIRevokesOf`를 아래로 바꾼다(태그 이름은 코7 뒤의 것).")
rep("(B7x [B-7x2], Brick.Form.UnseenLaw) keeps them", "(B7x [B-7x2], Brick.Form.KeptLaw) keeps them")
rep("CosmicTags::BrickFormUnseenLaw()", "CosmicTags::BrickFormKeptLaw()")
rep("'- except a people that carries Brick.Form.UnseenLaw (B7x), which keeps what CosmicEdicts::AIRevokesOf leaves it'",
    "'- except a people that carries Brick.Form.KeptLaw (B7x), which keeps what CosmicEdicts::AIRevokesOf leaves it'")
rep("시험: 시4. 대안('모든 칙령'): 코드는 그대로, K9를 굽히는 새 R, 신정론 불변식에 그 경우, W 다시 셈.",
    "시험: 시4 — 갈래 (a)·(b)마다 단언과 변이 하나. 고르지 않은 대안: '모든 칙령을 지닌다'(K9를 굽히는 새 R과 W의 다시 셈이 들었을 것이다).")
rep("`if (Owner.Traits.HasTagExact(CosmicTags::BrickSpacePlanckMicro()) && !IsSparedHazard(Hazard))`",
    "`if (Owner.Traits.HasTagExact(CosmicTags::BrickSpaceFoldedSmall()) && !IsSparedHazard(Hazard))`")
rep("(Brick.Space.HermitMonolith / PlanckMicro: the settle gate's Inward, CosmicMegaprojects) or one that keeps the unseen law (Brick.Form.UnseenLaw: CosmicEdicts::AIRevokesOf)",
    "(Brick.Space.ClosedPort / FoldedSmall: the settle gate's Inward, CosmicMegaprojects) or one that keeps the unseen law (Brick.Form.KeptLaw: CosmicEdicts::AIRevokesOf)")
rep("save those that touch the claim or answer a crisis. Never a people",
    "save those that touch the claim or answer a crisis, and all of its people-scope edicts while its row asks for galaxy laws. Never a people")
rep("(이름은 결정 3, 'save those…'는 결정 18의 답에 맞춘다)", "(이름은 답 3, 'save those…'는 답 18의 좁힘)")
rep("- **코7**의 노브·함수(T2·F10) — **대기 3**: `FCosmicRulesDef`·`DT_Rules.json`·`docs/30`의 노브 이름, `IsQuietOf`, 테스트(`:576-591`·`:923`·`:960`).",
    "- **코7**의 노브·함수(T2·F10) — 답 3: `BrickQuietInsightPerTick` → `BrickIdleInsightPerTick`, `IsQuietOf` → `IsIdleOf` — 목록과 스크립트는 `[B-7x1]`의 2(두 커밋은 하나로 합친다).")
rep("- **데3 — K4 업적 규칙**(F6): `DT_Feats` rules에 하나",
    "- **데3 — K4 업적 규칙**(F6 — 답 2, 헤더 §3.3 규칙 7): `DT_Feats` rules에 하나")
rep("- **시4**(F1·I2·M11) — **대기 18**:", "- **시4**(F1·I2·M11) — 답 18:")
rep("(a) UnseenLaw AI 백성은", "(a) KeptLaw AI 백성은")
rep("(b) UnseenLaw AI 백성(", "(b) KeptLaw AI 백성(")
rep("변이 `b7x2_unseen_law_keeps_claim_edicts`·`b7x2_unseen_law_keeps_slots`.",
    "변이는 갈래마다 하나 — `b7x2_kept_law_keeps_claim_edicts`((a)의 주장·위기의 답 갈래를 뺀다)·`b7x2_kept_law_keeps_slots`((b)의 `AsksForLawsOf` 물음을 뺀다).")
rep("`Sim.BrickShares`에 경우 하나 — PlanckMicro 주인의", "`Sim.BrickShares`에 경우 하나 — FoldedSmall 주인의")
rep("- **시6**(build-4) — 결정 8이 '넣지 않는다'면 함수와 함께 빠진다:",
    "- **시6**(build-4) — 답 8(장치는 넣고 주기 0으로 꺼 둔다 — 켤지는 B8의 실제 하늘 헤더): `DimCapitalStar`의 거절 갈래 셋(남의 수도·정착 중·인덱스 없음)에 단언 셋과 변이 하나:")
rep("`CivHasTrait Brick.Mutation.<변이>`(이름은 결정 3 뒤의 것)", "`CivHasTrait Brick.Mutation.<변이>`(새 이름 — 예: `Brick.Mutation.StarTow`)")

# ---- B-7x3
rep("- **시9**(F9·DET-5·I12·T3) — **대기 13**: `UI.Chronicle`에 지워진 백성·다스린 적 없는 생애·두 백성을 다스린 생애의 경우.",
    "- **시9**(F9·DET-5·I12·T3) — 답 13(㉢ — 저장 없음): `UI.Chronicle`에 넷 — 마지막 재위 백성이 `State.Civs`에서 지워진 생애에는 꼴 낱말이 없다((b)); 다스린 적 없는 생애(방랑자로만 산 생애)에는 없다((a)); 두 백성을 다스린 생애('A → B')에는 B의 꼴만((a)); 같은 백성을 다스린 두 생애에는 마지막 생애의 장에만((c)). 판의 끝 뒤 다시 열면 낱말이 바뀔 수 있음은 받는다((d) — 단언하지 않는다). 변이 하나: (b)의 `Civs` 물음을 뺀다(지워진 백성에 `ShapeOf`를 부른다).")

# ---- B-7x5s
rep("- **데4**(M8·build-1·I11) — **대기 20**(권장 차례 그대로면 이대로):", "- **데4**(M8·build-1·I11) — 답 20(권장 차례 그대로):")
rep("- 증명: 깨질 수 있는 확인 ⓐ~ⓒ(=1), 값보다 먼저 재는 것과 문턱의 규칙, 판단 ⑥.",
    "- 증명: 깨질 수 있는 확인 ⓐ~ⓒ(=1), 값보다 먼저 재는 것과 문턱의 규칙, 판단 ⑥ — 어긋나면 B7x의 노브가 먼저 물러선다(답 1 — 씨앗 묶음에서 연구·통찰·에너지 블록을 빼거나 곱·더함을 낮춘다; B6b의 r·c·N·P_bind·C는 건드리지 않는다; 고침은 `[B-7x5r]`).")

# ---- B-7x5c
rep_line("- **데1**(F7·M7) — (나)가 기본, (가)는 **대기 11**:",
    "- **데1**(F7·M7) — 답 11 (나): `petition_cross_dust_band`·`_star_line`·`_between_stars`·`_into_dust`·`_law_stays`·`_seam`의 ②에서 되풀이 이득을 뺀다 — 적어도 `_law_stays` ②의 `PlayerStatDelta Exaltation +1`; `petition_road`·`petition_feast`처럼 값과 작은 효과만 남긴다. 새 태그는 없다 — ini의 태그는 열일곱 그대로. `schema.json:1273-1275` 규칙의 `then`은 넓히지 않고(검사기에 OR 꼴이 없고 필요도 없다) `why`만 바꾼다: 'a crossroads petition closes on its branch: the first by taking Brick.Form.Society, the second by the tag its first choice gives; its choice 2 carries no repeatable gain; a dismissed card (no cost, no effect) comes again after the last three answers'. 설계서 §4.3 C05~C11의 ② 줄과 데이터가 같은지 같은 커밋에서 맞춘다. 고르지 않은 (가)는 ②의 `SetFlag Flag.Cross.<행>`과 태그 여섯이었다 — 판 전체 Flag가 계승한 다른 백성과 다른 생애의 카드까지 닫는다.")
rep("- **시10**(F7): `Sim.CrossroadsPresent`에 '② 뒤 되풀이 이득이 없다'((나)) 또는 '② 뒤 그 카드가 다시 오지 않는다'((가));",
    "- **시10**(F7): `Sim.CrossroadsPresent`에 '② 뒤 되풀이 이득이 없다'(답 11 (나));")

# ---- B-7x5r
rep("`STATUS.md`의 기준값 사실을 이 커밋이 고친다(M1).",
    "`STATUS.md`의 기준값 사실을 이 커밋이 고친다(M1). 이 커밋을 부르는 것은 둘이다: 판단 ⑥이 B6b 값과 어긋날 때(답 1 — B7x의 노브가 먼저 물러선다; 한 번 고쳐도 어긋나면 그 수와 함께 다시 묻는다)와 `-Crossroads` 칸(⑧)이 남긴 법 백성에서 놀라운 값을 낼 때(답 18 — 그래도 안 되면 수와 함께 다시 묻는다; 이 노트 §3 새11).")

# ---- slice da
rep("- DET-9(헤더 결정 6 (ㄱ)): 품는 백성이 지워지면 모든 성계의 `HostCivID`가 `NAME_None`이 된다 — 테스트 한 줄을 예약한다.",
    "- DET-9(답 6 (ㄱ)·(ㄴ)): `HostCivID`를 처음 쓰는 커밋이 비움도 넣는다 — 건넘 단계가 백성을 지우는 같은 걸음에서 모든 성계의 `HostCivID`를 비운다(추첨 없음). 시험: 품는 백성이 지워지면 모든 성계의 `HostCivID`가 `NAME_None`이 되고, 그 뒤 태어난 백성의 `CivID`는 지운 이름과 다르다(`NextCivSerial`); 변이 하나 — 비움을 뺀다.")
rep("- **데3 — 탄생 행의 낱말 규칙**(T4) — **대기 16**: rebirth_words·afterlife_words·'불사'·'유령'.",
    "- **데3 — 탄생 행의 낱말 규칙**(T4) — 답 16 (2): 탄생의 줄(그 행의 글과 로그 템플릿)은 옛 백성이 살아남았다·죽지 않았다·돌아왔다·다시 태어났다고 말하지 않는다 — 스키마 규칙 하나가 rebirth_words·afterlife_words·'불사'·'유령'을 막는다(`Tools/test_data_schema.py`에 목록마다 깨짐 하나와 걸린 목록의 핀).\n"
    "- **결정 16의 받는 조건**(T4) — 답 16 (1)·(3a)·(4)와 22: 태어난 백성은 새 `FCivilizationState`다 — 시험: 새 `CivID`(`NextCivSerial`)·이름 뱅크의 새 이름·`TimeVectors` 0, 부모의 `ArchetypeRow`·`Traits`(탄생 블록 하나만 빼고)·관계·칙령·`HostCivID`를 물려받지 않는다; `LetPeopleGo`(절멸)는 백성을 낳지 않는다(`FossilAutomata`는 얼굴만); ini에 `Undying*` 잎이 없다. 포자의 추첨은 고정 수 하나(Phase 5 전에만). 변이: 부모의 `Traits`를 베낀다.")

rep_line("**넘김의 수**(대상 커밋마다):",
    "**넘김의 수**(대상 커밋마다 — 2026-11-10 답 뒤): `[B-7x0]` 6(넷 — 2·3·4·5 — 은 이 브랜치에서 했다; 6은 답 21이 더했다), `[B-7x1]` 5(3은 들지 않는 것을 적는다), `[B-7x2]` 19(코1~코7·데3 둘·시1~시8의 몫·시12·충돌), `[B-7x4m]` 1, `[B-7x3]` 2, `[B-7x4]` 3, `[B-7x5s]` 2, `[B-7x5c]` 5, `[B-7x5r]` 1(조건부), 슬라이스 다 3(셋째는 답 16이 더했다). 모두 47(리뷰 때 45). 답을 기다리는 넘김은 없다.")

# ---- 5.5
rep("### 5.5 리뷰의 기록 (ultracode 적대 리뷰, 2026-11-09)\n\n",
    "### 5.5 리뷰의 기록 (ultracode 적대 리뷰, 2026-11-09)\n\n"
    "- **답**(2026-11-10): 개발자가 헤더 §10의 결정을 모두 권장대로 답했다('권장대로 진행해줘'). 아래는 리뷰 때의 기록이고, 답으로 정해진 곳에는 '(답 …)'을 붙였다.\n")
rep("K4는 데이터의 우연으로만 선다(→ 헤더 §3.3 규칙 7).", "K4는 데이터의 우연으로만 선다(→ 헤더 §3.3 규칙 7 — 답 2: 받았다).")
rep("K7은 서되 `HostCivID`를 비우는 규칙이 빠졌다(결정 6 (ㄱ)).", "K7은 서되 `HostCivID`를 비우는 규칙이 빠졌다(결정 6 (ㄱ) — 답: 받았다).")
rep("K9는 깨진다 — UnseenLaw의 `AIRevokesOf`(코1, 결정 18);",
    "K9는 깨진다 — 남긴 법(리뷰 때 이름 `UnseenLaw` → `KeptLaw`)의 `AIRevokesOf`(코1, 결정 18 — 답: 좁힘을 받았다);")
rep("K10은 저장에서 서고 표시에서 흔들린다(결정 13).", "K10은 저장에서 서고 표시에서 흔들린다(결정 13 — 답: ㉢, 저장 없이 표시를 좁힌다).")
rep("⑭ 절멸과 탄생(결정 16 (3a)/(3b)).", "⑭ 절멸과 탄생(결정 16 (3a)/(3b) — 답: (3a), 남은 기계는 절멸에서 태어나지 않는다).")

out = text.encode('utf-8')
assert b'\r' not in out
open(PATH, 'wb').write(out)
print(len(out))
