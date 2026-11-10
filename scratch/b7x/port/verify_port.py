"""Independent check of a ported export against pristine db9bf9f, aa09543, the audited texts and the lead's rulings.

Usage: python -I -X utf8 verify_port.py <pristine-export> <ported-export>
Reads only; writes nothing. Built separately from the port scripts (it does not import them).
"""
import copy
import json
import re
import subprocess
import sys

B7X = "C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad/b7x"
INT = B7X + "/int"
AUDITED = B7X + "/stage3/texts/audited_texts.md"
RULINGS = B7X + "/stage3/texts/lead_rulings.md"
NOTES = "docs/60_DECISIONS/headers/B7_composable_existence_and_evolution_notes.md"

LEAVES = [("Form", "UnseenLaw", "KeptLaw"), ("Diet", "SilenceFeed", "IdleFeed"), ("Space", "PlanckMicro", "FoldedSmall"),
          ("Form", "DysonBrain", "ShutSphere"), ("Mutation", "KesslerTapestry", "DustBand"), ("Mutation", "AbyssalMaw", "InwardHunger"),
          ("Mutation", "StarShepherd", "StarTow"), ("Space", "HermitMonolith", "ClosedPort")]
OLD = [o for _, o, _ in LEAVES] + ["QuietInsight"]

problems = []
notes = []


def bad(msg):
    problems.append(msg)


def show(rev, path):
    return subprocess.run(["git", "-C", INT, "show", "%s:%s" % (rev, path)], capture_output=True, check=True).stdout.decode("utf-8")


def rename(obj):
    s = json.dumps(obj, ensure_ascii=False)
    for axis, old, new in LEAVES:
        s = s.replace("Brick.%s.%s" % (axis, old), "Brick.%s.%s" % (axis, new))
    s = s.replace("BrickQuietInsightPerTick", "BrickIdleInsightPerTick")
    return json.loads(s)


def load(root, rel):
    raw = open(root + "/" + rel, "rb").read()
    return raw, json.loads(raw.decode("utf-8"))


pristine, ported = sys.argv[1], sys.argv[2]

# ---- audited text blocks (sections 1 and 2): first ```json block after each '### N.M `row`' heading
md = open(AUDITED, encoding="utf-8").read()
audited = {}
for m in re.finditer(r"^### (\d+)\.(\d+) `([a-z_]+)`.*?\n```json\n(.*?)\n```", md, re.S | re.M):
    if m.group(1) in ("1", "2"):
        audited[m.group(3)] = json.loads(m.group(4))
# lead's rulings: the ruled effects JSON
rul = open(RULINGS, encoding="utf-8").read()
ruled = json.loads(re.search(r"```json\n(.*?)\n```", rul, re.S).group(1))
RULED_TITLE = {"enc_cross_drift_fleet": "집을 싣는 배"}
if "'손대지 않은 칙령'" not in rul or "'집을 싣는 배'" not in rul:
    bad("rulings file does not hold the two titles any more")

# ---- tables
aa = {t: rename(json.loads(show("aa09543", "Tools/data/%s.json" % t))) for t in ("DT_Encounters", "DT_Events", "DT_Civs", "DT_Rules")}
aa_raw = {t: show("aa09543", "Tools/data/%s.json" % t) for t in ("DT_Encounters", "DT_Events")}
counts = {}
for table, prefixes in (("DT_Encounters", ("petition_cross_", "enc_cross_")), ("DT_Events", ("event_brick_",))):
    p_raw, p = load(pristine, "Tools/data/%s.json" % table)
    q_raw, q = load(ported, "Tools/data/%s.json" % table)
    new = q[len(p):]
    if q[:len(p)] != p:
        bad("%s: the rows before the appended ones differ from db9bf9f" % table)
    aa_new = [r for r in aa[table] if r["Name"].startswith(prefixes)]
    if [r["Name"] for r in new] != [r["Name"] for r in aa_new]:
        bad("%s: appended rows %s != aa09543 order %s" % (table, [r["Name"] for r in new], [r["Name"] for r in aa_new]))
    counts[table] = len(new)
    for got, src in zip(new, aa_new):
        name = src["Name"]
        want = copy.deepcopy(src)
        a = audited.get(name)
        if a is None:
            bad("%s: no audited block" % name)
            continue
        for key, val in a.items():
            if key == "Choices":
                for i, ch in enumerate(val):
                    for k2, v2 in ch.items():
                        want["Choices"][i][k2] = v2
            else:
                want[key] = val
        if name in RULED_TITLE:
            want["Title"] = RULED_TITLE[name]
        if name == "enc_cross_drift_fleet":
            want["Choices"][1]["SuccessText"] = src["Choices"][1]["SuccessText"]   # ruling: aa09543's line
            if "{Civ}의 일꾼들이 당신의 손을 기억했다." not in src["Choices"][1]["SuccessText"]:
                bad("C12 aa09543 success line is not the ruled one")
        for path, effects in ruled.items():
            rn, rest = path.split(".", 1)
            if rn == name:
                ci = int(re.match(r"Choices\[(\d+)\]", rest).group(1))
                field = rest.split(".")[1]
                # sign of the tilt must equal aa09543's (rulings: aa09543 wins)
                tilt_aa = [e["Value"] for e in src["Choices"][ci][field] if e["Type"] == "ShiftEthics" and e["Key"] == "Spiritualism"]
                tilt_rule = [e["Value"] for e in effects if e["Type"] == "ShiftEthics" and e["Key"] == "Spiritualism"]
                if tilt_rule and tilt_aa and (tilt_rule[0] > 0) != (tilt_aa[0] > 0):
                    bad("%s: ruled tilt sign differs from aa09543" % path)
                want["Choices"][ci][field] = effects
        if got != want:
            for k in sorted(set(got) | set(want)):
                if got.get(k) != want.get(k):
                    bad("%s.%s: got %s want %s" % (name, k, json.dumps(got.get(k), ensure_ascii=False)[:300], json.dumps(want.get(k), ensure_ascii=False)[:300]))
    # formatting: the file stays the canonical tab dump, LF, no BOM
    if q_raw.startswith(b"\xef\xbb\xbf") or b"\r" in q_raw or not q_raw.endswith(b"\n"):
        bad("%s: BOM/CR/no final LF" % table)
    if json.dumps(q, indent="\t", ensure_ascii=False) + "\n" != q_raw.decode("utf-8"):
        bad("%s: not the canonical tab-indented dump (pristine is)" % table)
    if not q_raw.decode("utf-8").startswith(p_raw.decode("utf-8")[:-3]):
        bad("%s: the pristine bytes are not kept as a prefix" % table)

# every Korean string of the new rows is either audited text or aa09543 text (nothing else crept in)
for table in ("DT_Encounters", "DT_Events"):
    _, q = load(ported, "Tools/data/%s.json" % table)
    pool = set(re.findall(r'"([^"]*[\uac00-\ud7a3][^"]*)"', json.dumps(audited, ensure_ascii=False)))
    pool |= set(re.findall(r'"([^"]*[\uac00-\ud7a3][^"]*)"', json.dumps(aa[table], ensure_ascii=False)))
    pool.add("집을 싣는 배")
    for r in q:
        if r["Name"].startswith(("petition_cross_", "enc_cross_", "event_brick_")):
            for s in re.findall(r'"([^"]*[\uac00-\ud7a3][^"]*)"', json.dumps(r, ensure_ascii=False)):
                if s not in pool:
                    bad("%s: Korean string from nowhere: %s" % (r["Name"], s))

# ---- DT_Civs: only arch_seeker gains Brick.Form.Society
_, pc = load(pristine, "Tools/data/DT_Civs.json")
_, qc = load(ported, "Tools/data/DT_Civs.json")
for a_row, b_row in zip(pc, qc):
    if a_row != b_row:
        if a_row["Name"] != "arch_seeker":
            bad("DT_Civs: %s changed" % a_row["Name"])
        else:
            d = copy.deepcopy(a_row)
            d["Traits"] = "(GameplayTags=(" + ",".join(re.findall(r'\(TagName="[^"]+"\)', a_row["Traits"]) + ['(TagName="Brick.Form.Society")']) + "))" if a_row["Traits"] != "(GameplayTags=)" else '(GameplayTags=((TagName="Brick.Form.Society")))'
            if d != b_row:
                bad("DT_Civs arch_seeker: %s" % b_row["Traits"])
aa_seeker = [r for r in aa["DT_Civs"] if r["Name"] == "arch_seeker"][0]
q_seeker = [r for r in qc if r["Name"] == "arch_seeker"][0]
if aa_seeker["Traits"] != q_seeker["Traits"]:
    notes.append("DT_Civs arch_seeker Traits differ from aa09543: %s vs %s" % (q_seeker["Traits"], aa_seeker["Traits"]))

# ---- DT_Rules: the 17 Brick knobs equal aa09543 renamed; every other key equals db9bf9f
_, pr = load(pristine, "Tools/data/DT_Rules.json")
_, qr = load(ported, "Tools/data/DT_Rules.json")
for a_row, b_row, s_row in zip(pr, qr, aa["DT_Rules"]):
    if list(a_row) != list(b_row):
        bad("DT_Rules key order changed")
    for k in b_row:
        if k.startswith("Brick"):
            if b_row[k] != s_row.get(k) or type(b_row[k]) is not type(s_row.get(k)):
                bad("DT_Rules %s: %r vs aa09543 %r" % (k, b_row[k], s_row.get(k)))
        elif b_row[k] != a_row[k]:
            bad("DT_Rules %s changed (not a Brick knob)" % k)
    if sorted(k for k in b_row if k.startswith("Brick")) != sorted(k for k in s_row if k.startswith("Brick")):
        bad("DT_Rules Brick key set differs from aa09543")

# ---- schema.json: the new rules equal aa09543's (renamed) + audited 7.3 named rules; the closing rule's why per notes 5.4 data-1
_, ps = load(pristine, "Tools/data/schema.json")
_, qs = load(ported, "Tools/data/schema.json")
aas = rename(json.loads(show("aa09543", "Tools/data/schema.json")))
for t in qs["tables"]:
    old_rules = ps["tables"].get(t, {}).get("rules", [])
    new_rules = qs["tables"][t].get("rules", [])
    if new_rules[:len(old_rules)] != old_rules:
        bad("schema %s: earlier rules changed" % t)
    added = new_rules[len(old_rules):]
    if added:
        notes.append("schema %s: +%d rules, +%d then-tests" % (t, len(added), sum(len(r["then"]) for r in added)))
    if {k: v for k, v in qs["tables"][t].items() if k != "rules"} != {k: v for k, v in ps["tables"].get(t, {}).items() if k != "rules"}:
        bad("schema %s: non-rule keys changed" % t)
    if t == "DT_Encounters":
        aa_whys = {r["why"]: r for r in aas["tables"][t]["rules"]}
        for r in added:
            key = lambda x: json.dumps(x, sort_keys=True, ensure_ascii=False)
            same = [a for a in aas["tables"][t]["rules"] if a.get("when") == r.get("when") and sorted(map(key, a["then"])) == sorted(map(key, r["then"]))]
            groups = {}
            for x in r["then"]:
                if "lacks_cond" in x:
                    groups.setdefault((x["col"], x["lacks_cond"]["type"]), []).append(x["lacks_cond"]["tag"])
            if any(g != sorted(g) or len(g) != 17 for g in groups.values()):
                bad("schema DT_Encounters: rule 4 then-tests not sorted by tag")
            if not same:
                bad("schema DT_Encounters: added rule has no aa09543 twin by when/then: %s" % r["why"][:80])
            elif same[0]["why"] != r["why"]:
                notes.append("schema DT_Encounters: why differs from aa09543 (the new why): %s" % r["why"])
    if t == "DT_Events":
        aud = json.loads(re.search(r"`DT_Events` — 규칙 다섯.*?```json\n(.*?)\n```", md, re.S).group(1))
        named = [r for r in aud if r.get("when", [{}])[0].get("row")]
        if added != named:
            bad("schema DT_Events: added rules != audited 7.3 named three")
if {k: v for k, v in qs.items() if k != "tables"} != {k: v for k, v in ps.items() if k != "tables"}:
    bad("schema: top-level keys (words, prefixes, ...) changed")
raw_s = open(ported + "/Tools/data/schema.json", "rb").read()
if raw_s.startswith(b"\xef\xbb\xbf") or b"\r" in raw_s:
    bad("schema.json BOM/CR")

# ---- no old leaf anywhere under Source/ Config/ Tools/
import os
for top in ("Source", "Config", "Tools"):
    for r, d, f in os.walk(ported + "/" + top):
        for n in f:
            b = open(os.path.join(r, n), "rb").read()
            if b"\0" in b[:4000]:
                continue
            for o in OLD:
                if o.encode() in b:
                    bad("old leaf %s in %s" % (o, os.path.join(r, n)))

print("audited blocks read: %d" % len(audited))
print("appended rows: %s" % counts)
for n_ in notes:
    print("NOTE ", n_)
for p_ in problems:
    print("PROBLEM", p_)
print("verify_port: %d problem(s)" % len(problems))
sys.exit(1 if problems else 0)
