"""[B-6b8b] git apply --check of every patch under <root>/Tools/mutations against a scratch copy of the tree.
usage: python -I apply_state.py <project root (patches)> <tree copy (cwd for git apply)> <out file>
Writes 'OK <path>' / 'FAIL <path>' lines (b6b8's apply_before.txt form)."""
import glob
import os
import subprocess
import sys

root, tree, out_path = sys.argv[1], sys.argv[2], sys.argv[3]
lines = []
for patch in sorted(glob.glob(os.path.join(root, "Tools", "mutations", "*", "*.patch"))):
    rel = os.path.relpath(patch, root).replace(os.sep, "/")
    ok = subprocess.run(["git", "apply", "--check", os.path.abspath(patch)], cwd=tree, capture_output=True).returncode == 0
    lines.append(("OK " if ok else "FAIL ") + rel)
with open(out_path, "w", encoding="utf-8", newline="\n") as f:
    f.write("\n".join(lines) + "\n")
print(len(lines), "patches;", sum(l.startswith("OK ") for l in lines), "apply;", sum(l.startswith("FAIL ") for l in lines), "fail")
