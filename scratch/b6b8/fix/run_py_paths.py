"""[B-6b8] fix stage (review L3-3): run Tools-only mutation patches given by their full repo path (any milestone folder, not
only Tools/mutations/B6b/ as scratchpad/b6b/run_py_mutations.py assumes) on a git copy of the tree OUTSIDE the project (Tools,
Config, Source, docs, CLAUDE.md - no Binaries/Content/Saved), so lesson 1 (git apply -R writes CRLF in the project) cannot
touch the project and no asset or DLL is ever written.

usage: python -I -X utf8 run_py_paths.py <project root> <work dir> <expect> <patch path>[=<filter>] ...
  expect: kill  - the patch must fail check_data.py or test_data_schema.py (Python-only mutation)
          pass  - the patch must pass both (an asset patch that must reach make_data_tables - lesson 2)
  A '<path>=<filter>' line of rerun_set.txt is accepted as is (the filter is printed, not used); '@file' reads such lines.
The copy is refreshed from the project before the first patch; after each patch it is reset with git checkout + clean, and
the baseline (both tools green) is checked first."""
import os
import re
import shutil
import stat
import subprocess
import sys

sys.stdout.reconfigure(encoding="utf-8")
root, work, expect = sys.argv[1], sys.argv[2], sys.argv[3]
args = []
for a in sys.argv[4:]:
    if a.startswith("@"):
        args += [l.strip() for l in open(a[1:], encoding="utf-8") if l.strip() and not l.startswith("#")]
    else:
        args.append(a)
assert expect in ("kill", "pass")


def git(*a):
    return subprocess.run(["git", "-C", work] + list(a), capture_output=True, text=True, encoding="utf-8", errors="replace")


def fresh():
    if os.path.exists(work):
        shutil.rmtree(work, onexc=lambda f, p, e: (os.chmod(p, stat.S_IWRITE), f(p)))  # git packs are read-only
    os.makedirs(work)
    for d in ("Tools", "Config", "Source", "docs"):
        shutil.copytree(os.path.join(root, d), os.path.join(work, d), ignore=shutil.ignore_patterns("__pycache__"))
    shutil.copy2(os.path.join(root, "CLAUDE.md"), work)
    git("init", "-q")
    git("config", "core.autocrlf", "false")
    git("add", "-A")
    r = git("-c", "user.name=x", "-c", "user.email=x@x", "commit", "-q", "-m", "base")
    assert r.returncode == 0, r.stderr


def tools():
    out = []
    for tool in ("Tools/check_data.py", "Tools/test_data_schema.py"):
        t = subprocess.run([sys.executable, "-I", "-X", "utf8", os.path.join(work, tool)], capture_output=True, text=True, encoding="utf-8", errors="replace")
        text = t.stdout + t.stderr
        out.append((tool.split("/")[-1], t.returncode, sorted(set(re.findall(r"^(?:FAIL|ERROR): (\w+)", text, re.M)))))
    return out


fresh()
base = tools()
print("baseline", base, flush=True)
assert all(rc == 0 for _, rc, _ in base), "the copy is not green"
results = {}
for a in args:
    path, _, flt = a.partition("=")
    name = os.path.basename(path)[:-6]
    r = git("apply", os.path.join(root, path))
    if r.returncode != 0:
        print(name, "PATCH DID NOT APPLY", r.stderr.strip()[:200], flush=True)
        results[name] = "NOT APPLIED"
        continue
    got = tools()
    failed = any(rc != 0 for _, rc, _ in got)
    if expect == "kill":
        verdict = "KILLED" if failed else "SURVIVED"
    else:
        verdict = "PASSES THE CHECKER (reaches the asset)" if not failed else "REFUSED BY THE CHECKER (never reaches the asset)"
    results[name] = verdict
    print(name, verdict, got, ("filter " + flt) if flt else "", flush=True)
    git("checkout", "--", ".")
    git("clean", "-fdq")
    assert git("status", "--porcelain").stdout.strip() == "", "copy not clean after " + name
print("summary:", {v: sum(1 for x in results.values() if x == v) for v in sorted(set(results.values()))})
