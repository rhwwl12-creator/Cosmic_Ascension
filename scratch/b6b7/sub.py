# usage: sub.py <file> <pairs.json> ; each pair [old,new] must match exactly once
import sys, json
path = sys.argv[1]
pairs = json.load(open(sys.argv[2], encoding='utf-8'))
b = open(path, 'rb').read()
s = b.decode('utf-8')
for old, new in pairs:
    n = s.count(old)
    if n != 1:
        sys.exit(f"{path}: {n} matches for {old[:60]!r}")
    s = s.replace(old, new)
open(path, 'wb').write(s.encode('utf-8'))
print(f"{path}: {len(pairs)} replaced")
