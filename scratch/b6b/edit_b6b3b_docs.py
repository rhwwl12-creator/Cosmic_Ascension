"""[B-6b3b] lead edits: the notes' review line and baseline bullet, STATUS.md. Run from the project root."""
import sys

def edit(path, pairs):
    s = open(path, encoding="utf-8").read()
    for old, new in pairs:
        if s.count(old) != 1:
            sys.exit(f"{path}: expected one '{old[:60]}', found {s.count(old)}")
        s = s.replace(old, new, 1)
    open(path, "w", encoding="utf-8", newline="\n").write(s)
    print("edited", path)

NOTES = "docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race_notes.md"
edit(NOTES, [
    ("구현 → 관점 셋(측정·테스트·문서)의 리뷰 → 반박 검증 → 고침 일꾼): 발견 13",
     "구현 → 관점 넷(측정·결정론과 빌드·테스트·문서 — 결정론과 빌드는 발견 0)의 리뷰 → 반박 검증 → 고침 일꾼 → 고친 diff의 확인 리뷰(코드 0·문서 3) → 수리): 발견 13"),
    ("- **기준값**: (리드가 러너 뒤에 적는다)",
     "- **기준값**: 다섯 줄과 해시 판 둘이 비트 그대로 — 3,000틱 2816687688 / 3300228750, 통치자 3,000 3561771757 / 2462331303, "
     "방랑자 6,000 2665013209 / 2229239127, 20,000틱 2359424635 / 726895141, 통치자 20,000 1611658162 / 29194274; "
     "`-WithoutB6bData=1` 짝도 같은 줄. 리뷰 뒤의 트리(이 커밋과 소스·데이터의 바이트가 같다 — 그 뒤에는 이 노트와 STATUS의 문장만)에서 "
     "러너 `-Tier full` 한 번(`-Data` 없이 — 그 안에서 223개를 두 프로세스로 연속 두 판)이 지났다.\n"
     "- **일의 속도**(개발자 2026-11-09 '더 빠르게' — 이 커밋부터): 러너 `-Tier full`은 커밋마다 한 번이다 — 그 안의 두 판이 "
     "CLAUDE.md의 '테스트 2회 연속'이고 헤더 §8 표의 '러너 full 두 번'은 그 두 판으로 읽는다(그 전까지는 러너를 두 번 돌려 네 판이었다). "
     "커밋마다의 변이는 새 변이와 그 커밋이 닿은 파일·테스트의 옛 변이만 돌리고, B6b 변이 전체의 재실행은 블록 끝 `[B-6b12]`에 한 번이다. "
     "그래서 이 커밋 전에 시작한 B6b 변이 전체 재실행은 C++ 165개 가운데 103개(모두 깨짐)에서 멈췄다 — 나머지는 리뷰 전의 175개와 "
     "리뷰 뒤의 마흔다섯이 덮는다(그 뒤로 소스가 바뀌지 않았다)."),
])

edit("STATUS.md", [
    ("`[B-6b2c]`·`[B-6b3a]`. 그 전: B6a 끝의 장치",
     "`[B-6b2c]`·`[B-6b3a]`·`[B-6b3b]`. 그 전: B6a 끝의 장치"),
    ("셋째 칸 위로 오르는 행이 아직 없어 비트 그대로)까지.**",
     "셋째 칸 위로 오르는 행이 아직 없어 비트 그대로)와 `[B-6b3b]`(측정 — 둘째: 커맨드렛의 `race:` 줄(열일곱 칸 — 넷째·다섯째 칸에 "
     "처음 선 틱과 백성, 제동·절멸·방주·조약·법·결박·증언·즉위의 문·#132·결정 열), 고향 은하의 넷째 칸 상한을 경계마다 보는 `race check`"
     "(어긋나면 에러와 종료 코드 1), 주장마다 한 줄의 `<Out>_claims.csv`, 측정 전용 스위치 `-WithoutB6bData`·`-FourthRungCap`과 도우미 "
     "`CosmicTests::WithoutB6bData`, 영압 카드의 다섯 통과 방랑자 카드의 행별 셈, B4 주장 셈의 입력 지연 고침; Shipping 빌드의 깨진 "
     "include(`[B-5b]`부터)도 고쳤다; 비트 그대로)까지.**"),
    ("자동화 테스트 222종 통과(B6b `[B-6b3a]`의 셋",
     "자동화 테스트 223종 통과(B6b `[B-6b3b]`의 하나 `Sim.RaceMeasure` — 경주 측정의 순수 술어, `Sim.FourthRungBrake`·"
     "`Sim.RulerPressurePolicy`·`Sim.WandererPolicy`·`Data.Integrity`에 그 경우들; B6b `[B-6b3a]`의 셋"),
    ("`[B-6b2c]`·`[B-6b3a]`는 비트 그대로 — 법을 묻는 행",
     "`[B-6b2c]`·`[B-6b3a]`·`[B-6b3b]`는 비트 그대로 — 법을 묻는 행"),
    ("`[B-6b2c]`·`[B-6b3a]`는 끝; `[B-6b2]`는 비트 그대로인 셋으로, `[B-6b3]`은 둘로 나눴다 — 노트 §5): `[B-6b3b]` 장치 — 측정(`race:` 줄·주장마다의 CSV·`race check`·측정 전용 스위치 `-WithoutB6bData`·`-FourthRungCap`과 그 도우미 — 헤더 §5.1; xhigh, ultracode 구현 리뷰) → `[B-6b4]` 표현 →",
     "`[B-6b2c]`·`[B-6b3a]`·`[B-6b3b]`는 끝; `[B-6b2]`는 비트 그대로인 셋으로, `[B-6b3]`은 둘로 나눴다 — 노트 §5; 커밋마다 러너 full 한 번, 블록 변이 전체는 `[B-6b12]`에 한 번 — 노트 §5 `[B-6b3b]`의 '일의 속도'): `[B-6b4]` 표현(넷째 칸의 연출·금빛 없는 다섯째 칸 `LookOf`·규칙 줄과 경고·다음 칸 툴팁의 부품·묘비의 칸·[백성] 탭의 칸·패널의 줄·봉인 고리·칙령 창의 은하 법칙 — 헤더 §4, 결정 25~32; high, ultracode 화면·문구 리뷰) →"),
])
