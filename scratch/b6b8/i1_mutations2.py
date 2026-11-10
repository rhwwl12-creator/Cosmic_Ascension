"""[B-6b8] I1: on the copy tree of i1_mutations.py - the two retaken Python-only patches (must be KILLED) and the three
asset patches (lesson 2: check_data.py and test_data_schema.py must PASS, so make_data_tables writes the asset and the C++
test is what breaks). usage: i1_mutations2.py <project root> <work dir>"""
import os
import re
import subprocess
import sys

sys.stdout.reconfigure(encoding="utf-8")
root, work = sys.argv[1], sys.argv[2]
OUT = os.path.join(root, "Tools", "mutations", "B6b")
KILL = ["b6b8_schema_desolation_without_lamp", "b6b8_schema_era_rule_dropped"]
PASS = ["b6b8_data_extinction_expiry_effect"]


def git(*args):
    return subprocess.run(["git", "-C", work] + list(args), capture_output=True, text=True, encoding="utf-8", errors="replace")


def tools():
    out = []
    for tool in ("Tools/check_data.py", "Tools/test_data_schema.py"):
        t = subprocess.run([sys.executable, "-I", "-X", "utf8", os.path.join(work, tool)], capture_output=True, text=True, encoding="utf-8", errors="replace")
        text = t.stdout + t.stderr
        out.append((tool.split("/")[-1], t.returncode, sorted(set(re.findall(r"^(?:FAIL|ERROR): (\w+)", text, re.M)))))
    return out


assert git("status", "--porcelain").stdout.strip() == ""
for name in KILL + PASS:
    r = git("apply", os.path.join(OUT, name + ".patch"))
    if r.returncode != 0:
        print(name, "PATCH DID NOT APPLY", r.stderr.strip(), flush=True)
        continue
    res = tools()
    failed = any(code != 0 for _, code, _ in res)
    verdict = ("KILLED" if failed else "SURVIVED") if name in KILL else ("PASSES THE CHECKER (reaches the asset)" if not failed else "STOPPED BY THE CHECKER (would never reach the asset)")
    print(name, verdict, res, flush=True)
    git("checkout", "--", ".")
    assert git("status", "--porcelain").stdout.strip() == ""
print("tree clean:", git("status", "--porcelain").stdout.strip() == "")
