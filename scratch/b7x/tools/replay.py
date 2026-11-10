import subprocess, os, sys, tempfile
sys.stdout.reconfigure(encoding="utf-8")
REPO = r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension"
OUT = sys.argv[1]
ONTO = sys.argv[2] if len(sys.argv) > 2 else "origin/main"
commits = subprocess.run(["git", "-C", REPO, "rev-list", "--reverse", "cef8cc7..origin/design/b7x-composable-existence"], capture_output=True, text=True, check=True).stdout.split()
def show(rev, path):
    r = subprocess.run(["git", "-C", REPO, "show", "%s:%s" % (rev, path)], capture_output=True)
    return r.stdout if r.returncode == 0 else None
running = {}
for c in commits:
    subj = subprocess.run(["git", "-C", REPO, "log", "-1", "--format=%h %s", c], capture_output=True, text=True, encoding="utf-8").stdout.strip()[:70]
    files = subprocess.run(["git", "-C", REPO, "diff", "--name-only", c + "^", c], capture_output=True, text=True).stdout.split()
    for f in files:
        base = show(c + "^", f) or b""
        theirs = show(c, f) or b""
        ours = running.get(f)
        if ours is None:
            ours = show(ONTO, f)
            if ours is None:
                ours = b""
        mb = show("cef8cc7", f) or b""
        if ours == base:
            running[f] = theirs
            continue
        d = tempfile.mkdtemp(dir=OUT)
        po, pb, pt = os.path.join(d, "o"), os.path.join(d, "b"), os.path.join(d, "t")
        open(po, "wb").write(ours); open(pb, "wb").write(base); open(pt, "wb").write(theirs)
        r = subprocess.run(["git", "merge-file", "-L", "main", "-L", "base", "-L", "branch", po, pb, pt])
        merged = open(po, "rb").read()
        if r.returncode:
            print("CONFLICT %d  %s  %s" % (r.returncode, subj, f))
            # resolve union, main first
            import re
            merged = re.sub(rb"<<<<<<< main\r?\n(.*?)=======\r?\n(.*?)>>>>>>> branch\r?\n", lambda m: m.group(1) + m.group(2), merged, flags=re.S)
        else:
            print("merged     %s  %s" % (subj, f))
        running[f] = merged
