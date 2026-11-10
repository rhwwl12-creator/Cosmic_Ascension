"""[B-6b8] I1: the three old patches I1's edits stopped from applying, taken again as b6b8_ patches with the same change
on the commit's text (brief 5 'Retakes'). Built in a small git repo of copies; never touches the project tree's sources.
usage: i1_retakes.py <project root> <work dir>"""
import os
import shutil
import stat
import subprocess
import sys

root, work = sys.argv[1], sys.argv[2]
OUT = os.path.join(root, "Tools", "mutations", "B6b")
FILES = ["Tools/data/schema.json", "Tools/data/DT_Hazards.json", "Tools/test_data_schema.py"]


def git(*args):
    r = subprocess.run(["git", "-C", work] + list(args), capture_output=True, text=True, encoding="utf-8")
    assert r.returncode == 0, r.stderr
    return r.stdout


if os.path.isdir(work):
    shutil.rmtree(work, onexc=lambda func, path, _: (os.chmod(path, stat.S_IWRITE), func(path)))
for f in FILES:
    os.makedirs(os.path.dirname(os.path.join(work, f)), exist_ok=True)
    shutil.copy(os.path.join(root, f), os.path.join(work, f))
git("init", "-q")
git("config", "core.autocrlf", "false")
git("add", "-A")
git("-c", "user.name=m", "-c", "user.email=m@m", "commit", "-qm", "base")


def read(f):
    d = open(os.path.join(work, f), "rb").read()
    assert b"\r" not in d
    return d.decode("utf-8")


def write(f, t):
    open(os.path.join(work, f), "wb").write(t.encode("utf-8"))


def once(t, old, new):
    assert t.count(old) == 1, (t.count(old), old[:80])
    return t.replace(old, new)


def emit(name, index_lines=False):
    diff = git("diff")
    if not index_lines:
        import re
        diff = re.sub(r"^index [0-9a-f]+\.\.[0-9a-f]+ \d+\n", "", diff, flags=re.M)
    open(os.path.join(OUT, name + ".patch"), "wb").write(diff.encode("utf-8"))
    git("checkout", "--", ".")
    print("wrote", name, diff.count("\n"), "lines")


# (1) b6b5_schema_desolation_without_lamp: '등불' leaves desolation_words.
s = read("Tools/data/schema.json")
write("Tools/data/schema.json", once(s, '"맷돌", "등불", "심지"', '"맷돌", "심지"'))
emit("b6b8_schema_desolation_without_lamp")

# (2) b6b5_schema_era_rule_dropped: the era words rule is gone (Rule E stays).
s = read("Tools/data/schema.json")
start = s.index('\t\t\t\t{\n\t\t\t\t\t"why": "an era the people names after its endured pressure')
end = s.index("\t\t\t\t},\n", start) + len("\t\t\t\t},\n")
write("Tools/data/schema.json", s[:start] + s[end:])
emit("b6b8_schema_era_rule_dropped")

# (3) b6b6_data_extinction_expiry_effect: the extinction rule keeps rows_of alone, the unit tests lose its three cases and
# count four then-tests fewer, and crisis_species_extinction carries a ChangeStability on its expiry (the old patch's
# index lines are kept in its form).
h = read("Tools/data/DT_Hazards.json")
row = h.index('"Name": "crisis_species_extinction"')
old = '\t\t"OnExpireEffects": [],\n'
at = h.index(old, row)
h = h[:at] + ('\t\t"OnExpireEffects": [\n\t\t\t{\n\t\t\t\t"Type": "ChangeStability",\n\t\t\t\t"Tag": "",\n\t\t\t\t"Key": "None",\n'
              '\t\t\t\t"Value": -0.1,\n\t\t\t\t"DurationTicks": 0\n\t\t\t}\n\t\t],\n') + h[at + len(old):]
write("Tools/data/DT_Hazards.json", h)
s = read("Tools/data/schema.json")
s = once(s, '\t\t\t\t\t\t{"col": "OnExpireEffects", "max_len": 0},\n\t\t\t\t\t\t{"col": "OnExpireBranchEffects", "max_len": 0},\n'
            '\t\t\t\t\t\t{"col": "OnExpireNeighbourEffects", "max_len": 0},\n\t\t\t\t\t\t{"col": "BranchChance", "is": 0},\n', "")
write("Tools/data/schema.json", s)
t = read("Tools/test_data_schema.py")
for line in ('        self.expect(self.after(extinct(OnExpireEffects=[eff("KillPlayer", tag="Death.Hazard")])), "DT_Hazards.crisis_test_storm", why, \'"col": "OnExpireEffects", "max_len": 0\')\n',
             '        self.expect(self.after(extinct(OnExpireNeighbourEffects=[eff("ChangeStability", value=-0.1)])), "DT_Hazards.crisis_test_storm", why, \'"col": "OnExpireNeighbourEffects"\')\n',
             '        self.expect(self.after(extinct(BranchChance=0.5, OnExpireBranchEffects=[eff("ChangeStability", value=-0.1)])), "DT_Hazards.crisis_test_storm", why, \'"col": "BranchChance"\')\n'):
    t = once(t, line, "")
t = once(t, 'self.assertEqual(walked, 330, "the number', 'self.assertEqual(walked, 326, "the number')
write("Tools/test_data_schema.py", t)
emit("b6b8_data_extinction_expiry_effect", index_lines=True)
