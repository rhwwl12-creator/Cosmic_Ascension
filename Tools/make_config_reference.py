"""Generates docs/30_CONFIG_REFERENCE.md from Tools/data/DT_Rules.json, Tools/data/schema.json and the runtime sources.

The table is generated — never edit it by hand (docs/40_DEV_GUIDE.md section 4). For every knob of the single
DT_Rules row it prints the value, the schema constraint, the runtime files that read it, and whether it is live
(a runtime reader exists) or reserved (declared ahead of the code that will read it).

Usage: python -I Tools/make_config_reference.py            writes the document
       python -I Tools/make_config_reference.py --check    exit 1 when the document is stale
"""
import json, os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, 'Source', 'Cosmic_Ascension')
OUT = os.path.join(ROOT, 'docs', '30_CONFIG_REFERENCE.md')

rules = json.load(open(os.path.join(ROOT, 'Tools', 'data', 'DT_Rules.json'), encoding='utf-8'))[0]
schema = json.load(open(os.path.join(ROOT, 'Tools', 'data', 'schema.json'), encoding='utf-8'))
columns = schema['tables']['DT_Rules'].get('columns') or {}

files = {}
for dirpath, _, names in os.walk(SRC):
    for fn in names:
        if fn.endswith(('.h', '.cpp')):
            p = os.path.join(dirpath, fn); rel = os.path.relpath(p, SRC).replace('\\', '/')
            if rel.startswith('Tests/') or rel.endswith('Data/CosmicDefinitionRows.h'): continue
            text = open(p, encoding='utf-8', errors='replace').read()
            text = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
            files[rel] = re.sub(r'//[^\n]*', '', text)  # a knob named only in a comment is not read

def fmt(v):
    s = json.dumps(v, ensure_ascii=False)
    return s if len(s) <= 70 else s[:67] + '…'

rows = []
for k, v in rules.items():
    if k == 'Name': continue
    readers = sorted(rel for rel, t in files.items() if re.search(r'\b' + re.escape(k) + r'\b', t))
    spec = dict(columns.get(k, {})); spec.pop('kind', None)
    status = '읽힘' if readers else '예약'
    rows.append((k, fmt(v), columns.get(k, {}).get('kind', '?'), fmt(spec) if spec else '', ', '.join(readers) if readers else '—', status))

live = sum(1 for r in rows if r[5] == '읽힘')
lines = ['# CONFIG REFERENCE — DT_Rules 노브 표 (생성 문서)', '',
         '> `python -I Tools/make_config_reference.py`가 `Tools/data/DT_Rules.json`·`schema.json`·`Source/`에서 만든다. **손으로 고치지 않는다** — 값은 JSON에서, 규칙은 schema에서, 읽는 곳은 코드에서 바뀐다. '
         f'노브 {len(rows)}개 가운데 런타임이 읽는 것 {live}개, 예약 {len(rows) - live}개(읽는 코드가 올 Phase에서 산다 — `docs/40_DEV_GUIDE.md` §4). '
         '밸런스 상수 가운데 아직 코드에 박힌 것(경제·연구 계수 — `Sim/GalaxySimulationSubsystem.cpp`의 `namespace Economy`)은 이 표에 없다; MVP에서 옮긴다.', '',
         '| 노브 | 값 | 종류 | 스키마 규칙 | 읽는 코드(테스트 제외) | 상태 |', '|---|---|---|---|---|---|']
for r in rows:
    lines.append('| `' + r[0] + '` | `' + r[1] + '` | ' + r[2] + ' | ' + ('`' + r[3] + '`' if r[3] else '') + ' | ' + r[4] + ' | ' + r[5] + ' |')
text = '\n'.join(lines) + '\n'

if '--check' in sys.argv:
    cur = open(OUT, encoding='utf-8').read() if os.path.exists(OUT) else ''
    if cur != text:
        print('make_config_reference: docs/30_CONFIG_REFERENCE.md is stale - run the script')  # ASCII: a cp949 console cannot print a dash; sys.exit(1)
    print('make_config_reference: up to date'); sys.exit(0)
os.makedirs(os.path.dirname(OUT), exist_ok=True)
open(OUT, 'w', encoding='utf-8', newline='\n').write(text)
print(f'make_config_reference: wrote {OUT} ({len(rows)} knobs, {live} live)')
