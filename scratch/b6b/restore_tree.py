"""Restore the working tree after a stopped mutation run: files in SNAP get the snapshot bytes, every other changed
tracked file gets HEAD's bytes (the working copies are LF like the blobs). Prints what it touched and anything it
could not place. Usage (project root): python -I -X utf8 restore_tree.py <snapshot dir>"""
import os
import shutil
import subprocess
import sys

snap = sys.argv[1]
status = subprocess.run(["git", "-c", "core.quotepath=false", "status", "--porcelain", "-uall"],
                        capture_output=True, text=True, encoding="utf-8").stdout.splitlines()
fixed, bad = [], []
for line in status:
    path = line[3:]
    if path == "docs/OVERVIEW.html":
        continue
    copy = os.path.join(snap, path)
    if os.path.exists(copy):
        a = open(copy, "rb").read()
        b = open(path, "rb").read() if os.path.exists(path) else None
        if a != b:
            shutil.copy2(copy, path)
            fixed.append(("snap", path))
        continue
    head = subprocess.run(["git", "show", "HEAD:" + path], capture_output=True)
    if head.returncode != 0:
        bad.append(("untracked, not in the snapshot", path))
        continue
    work = open(path, "rb").read() if os.path.exists(path) else None
    if work != head.stdout:
        kind = "head-eol" if work is not None and work.replace(b"\r\n", b"\n") == head.stdout else "head-CONTENT"
        open(path, "wb").write(head.stdout)
        fixed.append((kind, path))
for root, _, names in os.walk(snap):
    for name in names:
        rel = os.path.relpath(os.path.join(root, name), snap).replace(os.sep, "/")
        if not os.path.exists(rel):
            shutil.copy2(os.path.join(root, name), rel)
            fixed.append(("missing", rel))
for f in fixed:
    print(*f)
print("fixed", len(fixed), "bad", bad)
