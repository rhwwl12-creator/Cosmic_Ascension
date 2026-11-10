"""Compare bundleB.guarded_files sha1 with final_build_sha1.txt (read-only)."""
import hashlib, importlib.util, os, sys

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROOT = r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension"
spec = importlib.util.spec_from_file_location("bundleB", os.path.join(HERE, "bundleB.py"))
B = importlib.util.module_from_spec(spec)
sys.path.insert(0, HERE)
spec.loader.exec_module(B)

want = {}
with open(os.path.join(HERE, "final_build_sha1.txt"), encoding="utf-8") as f:
    for line in f:
        if line.startswith("#") or not line.strip():
            continue
        h, rest = line.split(" *", 1)
        want[rest.split("  (")[0].strip()] = h.strip()

have = {}
for p in B.guarded_files(ROOT):
    rel = os.path.relpath(p, ROOT).replace("\\", "/")
    with open(p, "rb") as f:
        have[rel] = hashlib.sha1(f.read()).hexdigest()

bad = []
for k in sorted(set(want) | set(have)):
    if want.get(k) != have.get(k):
        bad.append("%s want %s have %s" % (k, want.get(k), have.get(k)))
print("files want %d have %d" % (len(want), len(have)))
print("MATCH" if not bad else "DIFF:\n  " + "\n  ".join(bad))
