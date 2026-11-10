"""snap: copy every modified or untracked file (except docs/OVERVIEW.html) to DEST; check: compare and restore bytes."""
import os, shutil, subprocess, sys
mode, dest = sys.argv[1], sys.argv[2]
def files():
    out = subprocess.run(["git", "-c", "core.quotepath=false", "status", "--porcelain", "-uall"], capture_output=True, text=True, encoding="utf-8").stdout.splitlines()
    return [l[3:] for l in out if l[3:] != "docs/OVERVIEW.html"]
if mode == "snap":
    shutil.rmtree(dest, ignore_errors=True)
    for f in files():
        p = os.path.join(dest, f); os.makedirs(os.path.dirname(p), exist_ok=True); shutil.copy2(f, p)
    print("snapped", len(files()))
else:
    bad = 0
    for root, _, names in os.walk(dest):
        for n in names:
            src = os.path.join(root, n); rel = os.path.relpath(src, dest)
            a = open(src, "rb").read(); b = open(rel, "rb").read() if os.path.exists(rel) else None
            if a != b:
                same_text = b is not None and a.replace(b"\r\n", b"\n") == b.replace(b"\r\n", b"\n")
                print(rel, "line endings only" if same_text else "CONTENT DIFFERS")
                bad += 1
                if same_text and mode == "restore":
                    shutil.copy2(src, rel)
    extra = [f for f in files() if not os.path.exists(os.path.join(dest, f))]
    print("differs:", bad, "new files:", extra)
