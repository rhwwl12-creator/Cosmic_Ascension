import sys
src, dst = sys.argv[1], sys.argv[2]
body = open(src, 'rb').read()
label = (
    "> **동결본 — 고치지 않는다.** 2026-11-09 개발자가 쓴 원안(커밋 `9be5b3e`의 `docs/10_DESIGN/EXISTENCE_EVOLUTION_AND_ECOLOGY.md`)을 글자 그대로 옮겼다. "
    "받지 않은 것(무엇이 '되는' 끝·불사 분열·유령 제국·사람의 수 따위)을 담고 있어 현행 설계가 아니다.\n"
    "> 받은 꼴은 `docs/10_DESIGN/EXISTENCE_EVOLUTION_AND_ECOLOGY.md`와 B7x 헤더 `docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md`에 있다.\n"
    "\n"
).encode('utf-8')
out = label + body
assert b'\r' not in out and not out.startswith(b'\xef\xbb\xbf')
open(dst, 'wb').write(out)
print(len(out))
