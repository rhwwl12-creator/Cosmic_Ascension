"""[B-6b8] I1: which old patches touching the files I1 edits still apply (git apply --check, nothing written), compared
with apply_before.txt. usage: i1_apply_check.py <project root> <apply_before.txt>"""
import glob
import os
import re
import subprocess
import sys

sys.stdout.reconfigure(encoding="utf-8")
root, before_path = sys.argv[1], sys.argv[2]
FILES = {"Tools/data/schema.json", "Tools/data/DT_Events.json", "Tools/data/DT_Encounters.json", "Tools/test_data_schema.py"}
before = open(before_path, encoding="utf-8").read()
counts = {}
changed = []
for patch in sorted(glob.glob(os.path.join(root, "Tools", "mutations", "*", "*.patch"))):
    text = open(patch, encoding="utf-8", errors="replace").read()
    targets = set(re.findall(r"^\+\+\+ b/(\S+)", text, re.M))
    hit = targets & FILES
    if not hit:
        continue
    rel = os.path.relpath(patch, root).replace("\\", "/")
    name = os.path.basename(patch)
    if name.startswith("b6b8_"):
        continue
    ok = subprocess.run(["git", "-C", root, "apply", "--check", rel], capture_output=True).returncode == 0
    for f in hit:
        counts.setdefault(f, [0, 0])
        counts[f][0 if ok else 1] += 1
    # the line of apply_before for the patch
    line = next((l for l in before.splitlines() if name in l), "")
    was_ok = None
    if line:
        low = line.lower()
        was_ok = line.startswith("OK ")
    print(("APPLIES " if ok else "FAILS   ") + rel + ("" if was_ok is None else ("  (before: %s)" % ("applies" if was_ok else "failed"))))
    if was_ok is not None and was_ok != ok:
        changed.append((rel, was_ok, ok))
print("counts (applies, fails):", counts)
print("changed since apply_before:", changed or "none")
