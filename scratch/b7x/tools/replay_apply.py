import subprocess, os, sys, re, tempfile, shutil
sys.stdout.reconfigure(encoding="utf-8")
REPO = r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension"
SP = sys.argv[1]
TREE = os.path.join(SP, "sim_main")
patches = [l.split(" ",1)[1].strip() for l in open(os.path.join(SP, "apply_stopped.txt"), encoding="utf-8") if l.strip()]
commits = subprocess.run(["git", "-C", REPO, "rev-list", "--reverse", "cef8cc7..origin/design/b7x-composable-existence"], capture_output=True, text=True, check=True).stdout.split()
def show(rev, path):
    r = subprocess.run(["git", "-C", REPO, "show", "%s:%s" % (rev, path)], capture_output=True)
    return r.stdout if r.returncode == 0 else None
work = os.path.join(SP, "replay_tree")
if os.path.exists(work): shutil.rmtree(work)
shutil.copytree(TREE, work)
running = {}
def patch_ok(p):
    r = subprocess.run(["git", "apply", "--check", os.path.join(TREE, "Tools", "mutations", p)], cwd=work, capture_output=True)
    return r.returncode == 0
state = {p: patch_ok(p) for p in patches}
print("before:", state)
for c in commits:
    subj = subprocess.run(["git", "-C", REPO, "log", "-1", "--format=%h %s", c], capture_output=True, text=True, encoding="utf-8").stdout.strip()[:40]
    files = subprocess.run(["git", "-C", REPO, "diff", "--name-only", c + "^", c], capture_output=True, text=True).stdout.split()
    for f in files:
        base = show(c + "^", f) or b""
        theirs = show(c, f) or b""
        dst = os.path.join(work, f)
        ours = open(dst, "rb").read() if os.path.exists(dst) else b""
        if ours == base:
            merged = theirs
        else:
            d = tempfile.mkdtemp(dir=SP)
            po, pb, pt = os.path.join(d, "o"), os.path.join(d, "b"), os.path.join(d, "t")
            open(po, "wb").write(ours); open(pb, "wb").write(base); open(pt, "wb").write(theirs)
            subprocess.run(["git", "merge-file", "-L", "main", "-L", "base", "-L", "branch", po, pb, pt])
            merged = open(po, "rb").read()
            merged = re.sub(rb"<<<<<<< main\n(.*?)=======\n(.*?)>>>>>>> branch\n", lambda m: m.group(1) + m.group(2), merged, flags=re.S)
            shutil.rmtree(d)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        open(dst, "wb").write(merged)
    for p in patches:
        ok = patch_ok(p)
        if ok != state[p]:
            print("%s  -> %s  %s" % (subj, "applies" if ok else "STOPS", p))
            state[p] = ok
