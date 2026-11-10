import sys
p = sys.argv[1]
d = open(p, 'rb').read()
nl = b'\r\n' if b'\r\n' in d else b'\n'
assert d.count(b'<body>') == 1
text = "동결본 — B6a 때의 개관(2026-11-09). 받지 않은 원안(B7x 절의 여러 표현 포함)을 담고 있다 — 현행 설계가 아니다. 현행은 docs/의 문서와 B7x 헤더(docs/60_DECISIONS/headers/B7_composable_existence_and_evolution.md)."
ins = (
    "<!-- 동결본 (고치지 않는다): B6a 때의 개관, 2026-11-09. 받지 않은 원안(B7x 절의 여러 표현)을 담고 있어 현행 설계가 아니다. 현행은 docs/의 문서와 B7x 헤더. -->".encode('utf-8') + nl +
    ('<div role="note" style="margin:16px;padding:10px 14px;border:1px solid var(--rose);border-radius:6px;background:var(--rose-dim);color:var(--fg);font:14px/1.6 var(--body)"><b>'
     + text.split('. ', 1)[0] + '.</b> ' + text.split('. ', 1)[1] + '</div>').encode('utf-8') + nl
)
i = d.index(b'<body>') + len(b'<body>') + len(nl)
assert d[i - len(nl):i] == nl
out = d[:i] + ins + d[i:]
open(p, 'wb').write(out)
print(len(d), len(out), out.count(b'\r\n'), out.count(b'\n'))
