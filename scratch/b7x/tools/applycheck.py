import os, subprocess, sys
sys.stdout.reconfigure(encoding="utf-8")
tree, out = sys.argv[1], sys.argv[2]
res = []
for root, _, fs in os.walk(os.path.join(tree, "Tools", "mutations")):
    for f in sorted(fs):
        if not f.endswith(".patch"):
            continue
        p = os.path.join(root, f)
        r = subprocess.run(["git", "apply", "--check", p], cwd=tree, capture_output=True)
        res.append("%s %s" % ("OK " if r.returncode == 0 else "BAD", os.path.relpath(p, os.path.join(tree, "Tools", "mutations")).replace("\\", "/")))
open(out, "w", encoding="utf-8").write("\n".join(sorted(res)) + "\n")
print(sum(1 for r in res if r.startswith("BAD")), "bad of", len(res))
