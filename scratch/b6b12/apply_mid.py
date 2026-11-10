"""[B-6b12] brief section 1.1 A5 step 4 (and section 5.7's apply_after): apply_state.py's check of every mutation patch,
on HEAD plus the uncommitted Phase A changes, never in the project.

apply_state.py archives a revision, so it cannot see uncommitted work. This variant
  1. archives <rev> (Source, Tools, Config) into the scratch dir and spot-checks the archive against the blobs (LF);
  2. applies <phaseA.diff> there with `git -c core.eol=lf -c core.autocrlf=false apply`, then checks that every file the
     diff names now equals the project tree's bytes;
  3. copies in the patch files the archive does not have or holds with other content (CRLF-only differences are
     line-ending phantoms and keep the archived bytes), and removes the patches the tree no longer has;
  4. runs `git apply --check` on every Tools/mutations patch and writes 'OK <path>' / 'FAIL <path>' lines.
With --before it prints the transitions (OK -> FAIL, FAIL -> OK, new, gone), as apply_after.py did.

usage: python -I -X utf8 apply_mid.py run <project root> <phaseA.diff> <scratch dir> <out file> [--rev HEAD] [--before <file>]
       python -I -X utf8 apply_mid.py compare <before file> <after file>
The project is only read (git archive, git cat-file, plain reads)."""
import io
import os
import shutil
import subprocess
import sys
import tarfile

GITC = ["-c", "core.eol=lf", "-c", "core.autocrlf=false"]
SPOT = ["Tools/data/schema.json", "Source/Cosmic_Ascension/Tests/CosmicTestHelpers.h", "Tools/data/DT_Hazards.json"]


def run_git(args, cwd=None, data=None):
    return subprocess.run(["git"] + args, cwd=cwd, input=data, capture_output=True)


def diff_files(diff_text):
    out = []
    for line in diff_text.replace("\r\n", "\n").split("\n"):
        if line.startswith("diff --git a/"):
            b = line.split(" b/", 1)[1].strip()
            if b not in out:
                out.append(b)
    return out


def norm(b):
    return b.replace(b"\r\n", b"\n")


def walk_patches(base):
    res = []
    mut = os.path.join(base, "Tools", "mutations")
    for d, _, fs in os.walk(mut):
        for f in fs:
            if f.endswith(".patch"):
                res.append(os.path.relpath(os.path.join(d, f), base).replace("\\", "/"))
    return sorted(res)


def run(root, diff, scratch, out, rev="HEAD", before=None, log=print):
    if os.path.exists(scratch):
        shutil.rmtree(scratch)
    os.makedirs(scratch)
    data = run_git(GITC + ["-C", root, "archive", "--format=tar", rev, "Source", "Tools", "Config"]).stdout
    if not data:
        raise SystemExit("apply_mid: git archive of %s gave nothing" % rev)
    with tarfile.open(fileobj=io.BytesIO(data)) as t:
        t.extractall(scratch, filter="data")
    for p in SPOT:
        r = run_git(["-C", root, "cat-file", "blob", "%s:%s" % (rev, p)])
        if r.returncode == 0:
            assert open(os.path.join(scratch, p), "rb").read() == r.stdout, "archive is not the blob: " + p
    diff_text = open(diff, encoding="utf-8", errors="replace").read() if diff else ""
    files = diff_files(diff_text)
    if diff_text.strip():
        r = run_git(GITC + ["apply", "--whitespace=nowarn", os.path.abspath(diff)], cwd=scratch)
        if r.returncode != 0:
            raise SystemExit("apply_mid: the Phase A diff does not apply to %s: %s" % (rev, r.stderr.decode("utf-8", "replace")[:400]))
    differ = []
    for f in files:
        mine = os.path.join(scratch, f)
        theirs = os.path.join(root, f)
        a = open(mine, "rb").read() if os.path.exists(mine) else None
        b = open(theirs, "rb").read() if os.path.exists(theirs) else None
        if a != b:
            differ.append(f)
    if differ:
        raise SystemExit("apply_mid: after the diff these files differ from the project tree's bytes: " + ", ".join(differ))
    have = set(walk_patches(scratch))
    tree = set(walk_patches(root))
    copied, gone = [], []
    for p in sorted(tree):
        src = open(os.path.join(root, p), "rb").read()
        dst_path = os.path.join(scratch, p)
        if p not in have or norm(open(dst_path, "rb").read()) != norm(src):
            os.makedirs(os.path.dirname(dst_path), exist_ok=True)
            open(dst_path, "wb").write(src)
            copied.append(p)
    for p in sorted(have - tree):
        os.remove(os.path.join(scratch, p))
        gone.append(p)
    lines = []
    for p in sorted(walk_patches(scratch)):
        r = run_git(GITC + ["apply", "--check", p], cwd=scratch)
        lines.append(("OK " if r.returncode == 0 else "FAIL ") + p)
    with open(out, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines) + "\n")
    log("phaseA files %d (each equal to the tree) | patches copied in %d, removed %d | patches %d OK %d FAIL %d" %
        (len(files), len(copied), len(gone), len(lines), sum(l.startswith("OK") for l in lines), sum(l.startswith("FAIL") for l in lines)))
    for p in copied:
        log("  copied in: " + p)
    for p in gone:
        log("  removed (not in the tree): " + p)
    result = {"files": files, "copied": copied, "gone": gone, "lines": lines}
    if before:
        result["compare"] = compare(before, out, log)
    return result


def read_state(path):
    st = {}
    for line in open(path, encoding="utf-8"):
        line = line.strip()
        if line:
            ok, p = line.split(" ", 1)
            st[p] = ok
    return st


def compare(before, after, log=print):
    b, a = read_state(before), read_state(after)
    res = {
        "ok_to_fail": sorted(p for p in a if b.get(p) == "OK" and a[p] == "FAIL"),
        "fail_to_ok": sorted(p for p in a if b.get(p) == "FAIL" and a[p] == "OK"),
        "new": sorted(p for p in a if p not in b),
        "gone": sorted(p for p in b if p not in a),
    }
    res["new_fail"] = sorted(p for p in res["new"] if a[p] == "FAIL")
    for k in ("ok_to_fail", "fail_to_ok", "new", "gone", "new_fail"):
        log("%s (%d): %s" % (k.replace("_", " "), len(res[k]), " ".join(res[k])))
    return res


def main(argv):
    if len(argv) >= 1 and argv[0] == "compare" and len(argv) == 3:
        compare(argv[1], argv[2])
        return 0
    if len(argv) >= 5 and argv[0] == "run":
        rest = argv[5:]
        rev = "HEAD"
        before = None
        while rest:
            k = rest.pop(0)
            if k == "--rev":
                rev = rest.pop(0)
            elif k == "--before":
                before = rest.pop(0)
            else:
                raise SystemExit("apply_mid: unknown argument " + k)
        run(argv[1], argv[2], argv[3], argv[4], rev, before)
        return 0
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    sys.exit(main(sys.argv[1:]))
