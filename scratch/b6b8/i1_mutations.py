"""[B-6b8] I1: write the eight data/schema mutation patches of brief 5 from text edits on a copy of the tree, then run the
six Python-only ones there (never in the project tree): apply, run test_data_schema.py and check_data.py, record the
failing tests, revert, and check the copy is byte-identical again.
usage: i1_mutations.py <project root> <work dir> make|run"""
import os
import re
import shutil
import subprocess
import sys

sys.stdout.reconfigure(encoding="utf-8")
root, work, mode = sys.argv[1], sys.argv[2], sys.argv[3]
SCHEMA = "Tools/data/schema.json"
EVENTS = "Tools/data/DT_Events.json"
ENCOUNTERS = "Tools/data/DT_Encounters.json"
OUT = os.path.join(root, "Tools", "mutations", "B6b")

# name -> (file, old, new): each a single exact replacement on the commit's text.
EDITS = {
    "b6b8_schema_witness_rule_dropped": (SCHEMA, None, None),  # the whole rule: built below
    "b6b8_schema_witness_words_off_body": (SCHEMA, '{"col": "Body", "lacks_words": "sign_voice_words"}', '{"col": "Title", "lacks_words": "sign_voice_words"}'),
    "b6b8_schema_witness_past_forms_dropped": (SCHEMA, '"기록하", "기록했", "기록해", "지나간다", "지나가", "지나갔", "떠난다", "떠나", "떠났", "외면"',
                                               '"기록하", "기록해", "지나간다", "지나가", "지나갔", "떠난다", "떠나", "외면"'),
    "b6b8_data_witness_says_jeungeon": (ENCOUNTERS, '"Label": "그늘의 가장자리를 재어 둔다"', '"Label": "그늘의 가장자리를 재어 증언해 둔다"'),
    "b6b8_schema_era_shape_dropped": (SCHEMA, None, None),  # the whole rule: built below
    "b6b8_data_era_no_ruled": (EVENTS, None, None),  # the Ruled condition: built below
    "b6b8_data_era_borders_0_3": (EVENTS, '\t\t\t\t"Key": "Borders",\n\t\t\t\t"Value": 0.2,', '\t\t\t\t"Key": "Borders",\n\t\t\t\t"Value": 0.3,'),
    "b6b8_data_witness_reverence_0_3": (ENCOUNTERS, '\t\t\t\t\t\t"Type": "ChangeReverence",\n\t\t\t\t\t\t"Tag": "",\n\t\t\t\t\t\t"Key": "None",\n\t\t\t\t\t\t"Value": 0.1,',
                                        '\t\t\t\t\t\t"Type": "ChangeReverence",\n\t\t\t\t\t\t"Tag": "",\n\t\t\t\t\t\t"Key": "None",\n\t\t\t\t\t\t"Value": 0.3,'),
}
PY_ONLY = ["b6b8_schema_witness_rule_dropped", "b6b8_schema_witness_words_off_body", "b6b8_schema_witness_past_forms_dropped",
           "b6b8_data_witness_says_jeungeon", "b6b8_schema_era_shape_dropped", "b6b8_data_era_no_ruled"]


def read(path):
    data = open(path, "rb").read()
    assert b"\r" not in data, path
    return data.decode("utf-8")


def edited(name, text):
    path, old, new = EDITS[name]
    if name == "b6b8_schema_witness_rule_dropped":
        start = text.index('\t\t\t\t{\n\t\t\t\t\t"why": "a wanderer\'s card on the claim is a sign seen or measured')
        end = text.index("\t\t\t\t}\n", start) + len("\t\t\t\t}\n")
        # the rule before it loses its trailing comma
        prev = text.rindex("\t\t\t\t},\n", 0, start)
        assert prev + len("\t\t\t\t},\n") == start
        return text[:prev] + "\t\t\t\t}\n" + text[end:]
    if name == "b6b8_schema_era_shape_dropped":
        start = text.index('\t\t\t\t{\n\t\t\t\t\t"why": "the era of an endured pressure is told for the ruled people alone')
        end = text.index("\t\t\t\t}\n", start) + len("\t\t\t\t}\n")
        prev = text.rindex("\t\t\t\t},\n", 0, start)
        assert prev + len("\t\t\t\t},\n") == start
        return text[:prev] + "\t\t\t\t}\n" + text[end:]
    if name == "b6b8_data_era_no_ruled":
        row = text.index('"Name": "event_era_endured"')
        old = ('\t\t"Conditions": [\n\t\t\t{\n\t\t\t\t"Type": "SystemOwnedBy",\n\t\t\t\t"Tag": "",\n\t\t\t\t"Key": "Ruled",\n\t\t\t\t"Value": 0.0\n\t\t\t},\n\t\t\t{\n'
               '\t\t\t\t"Type": "CivPressureEnduredTicksReached"')
        new = '\t\t"Conditions": [\n\t\t\t{\n\t\t\t\t"Type": "CivPressureEnduredTicksReached"'
        at = text.index(old, row)
        assert text.count(old) == 1
        return text[:at] + new + text[at + len(old):]
    if old in ('\t\t\t\t"Key": "Borders",\n\t\t\t\t"Value": 0.2,',):
        at = text.index(old, text.index('"Name": "event_era_endured"'))
        return text[:at] + new + text[at + len(old):]
    if name == "b6b8_data_witness_reverence_0_3":
        at = text.index(old, text.index('"Name": "enc_witness"'))
        return text[:at] + new + text[at + len(old):]
    assert text.count(old) == 1, (name, text.count(old))
    return text.replace(old, new)


def git(*args, check=True):
    r = subprocess.run(["git", "-C", work] + list(args), capture_output=True, text=True, encoding="utf-8", errors="replace")
    if check and r.returncode != 0:
        raise SystemExit("git %s: %s" % (args, r.stderr))
    return r


if mode == "make":
    if os.path.isdir(work):
        import stat
        shutil.rmtree(work, onexc=lambda func, path, _: (os.chmod(path, stat.S_IWRITE), func(path)))
    os.makedirs(work)
    for sub in ("Tools", "Config", "docs"):
        shutil.copytree(os.path.join(root, sub), os.path.join(work, sub))
    shutil.copy(os.path.join(root, "CLAUDE.md"), work)  # the verdict test reads the invariant's line there
    os.makedirs(os.path.join(work, "Source", "Cosmic_Ascension", "Data"))
    shutil.copy(os.path.join(root, "Source", "Cosmic_Ascension", "Data", "CosmicRowNames.h"), os.path.join(work, "Source", "Cosmic_Ascension", "Data"))
    git("init", "-q")
    git("config", "core.autocrlf", "false")
    git("add", "-A")
    git("-c", "user.name=m", "-c", "user.email=m@m", "commit", "-qm", "base")
    for name, (path, _, _) in EDITS.items():
        full = os.path.join(work, path)
        text = read(full)
        open(full, "wb").write(edited(name, text).encode("utf-8"))
        diff = git("diff", "--", path).stdout
        assert diff.startswith("diff --git a/%s b/%s" % (path, path)), diff[:200]
        # the index line is noise next to the old patches' form; keep only the a/b headers and the hunks
        diff = re.sub(r"^index [0-9a-f]+\.\.[0-9a-f]+ \d+\n", "", diff, flags=re.M)
        open(os.path.join(OUT, name + ".patch"), "wb").write(diff.encode("utf-8"))
        git("checkout", "--", path)
        print("wrote", name, diff.count("\n"), "lines")
    sys.exit(0)

# run
def failing(output):
    return sorted(set(re.findall(r"^(?:FAIL|ERROR): (\w+)", output, re.M)))


for tool in ("Tools/check_data.py", "Tools/test_data_schema.py"):
    t = subprocess.run([sys.executable, "-I", "-X", "utf8", os.path.join(work, tool)], capture_output=True, text=True, encoding="utf-8", errors="replace")
    print("baseline", tool, t.returncode, failing(t.stdout + t.stderr), flush=True)
    assert t.returncode == 0, "the unpatched copy has to pass"
for name in PY_ONLY:
    patch = os.path.join(OUT, name + ".patch")
    r = git("apply", patch, check=False)
    if r.returncode != 0:
        print(name, "PATCH DID NOT APPLY", r.stderr.strip())
        continue
    results = []
    for tool in ("Tools/check_data.py", "Tools/test_data_schema.py"):
        t = subprocess.run([sys.executable, "-I", "-X", "utf8", os.path.join(work, tool)], capture_output=True, text=True, encoding="utf-8", errors="replace")
        out = t.stdout + t.stderr
        tail = [line for line in out.splitlines() if line.strip()][-1:]
        results.append((tool.split("/")[-1], t.returncode, failing(out) if "test_" in tool else (tail[0][:100] if tail else "")))
    killed = any(code != 0 for _, code, _ in results)
    print(name, "KILLED" if killed else "SURVIVED", results, flush=True)
    git("checkout", "--", ".")
    assert git("status", "--porcelain").stdout.strip() == "", git("status", "--porcelain").stdout
print("tree clean after the runs:", git("status", "--porcelain").stdout.strip() == "")
