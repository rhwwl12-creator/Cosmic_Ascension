"""[B-6b12] mutation planner (brief section 5.1, 5.2, 5.4 and section 2's older-block sets).

Reads Tools/mutations/README.md and the patch files (the working tree, or a revision with --rev, read only through
`git cat-file`), and gives every row a kind from its README row - never from its name:
  1 asset  - listed in the README's asset sentence (the README:5 list)
  2 probe  - a row typed "commandlet probe" / "a probe, not a mutation", or b6b8_cmdlet_witness_first_any_card
  3 python - listed in the README's Python sentence (README:6), or the Breaks cell names no `Cosmic_Ascension.` test
  4 cpp    - everything else
Then the set (B6b; older touched; older meaning-changed), the live/not-run state, the lane groups (by target file,
asset groups by JSON file, largest first) and the planner's assertions.

usage: python -I -X utf8 plan.py --root <project root> [--rev HEAD] [--phaseA-diff <diff>] [--apply-state <apply_mid.txt>]
                                 [--already-rerun <file of tags>] [--meaning-tests <file>] [--edited-tests a,b]
                                 [--out plan.json] [--quiet]
Nothing is written anywhere but --out."""
import argparse
import json
import os
import re
import subprocess
import sys

README = "Tools/mutations/README.md"
MUT_DIR = "Tools/mutations"
SCHEMA_TESTS = "Tools/test_data_schema.py"
NOTES = "docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race_notes.md"
WITNESS_SMOKE = "b6b8_cmdlet_witness_first_any_card"
HEAVY = ("CosmicTestHelpers.h", "GalaxyState.h", "CosmicKnowing.h")
# N section 1.3's clock-pinned list (brief section 2, verbatim) and the Data.Integrity rows (always meaning-changed).
CLOCK_TESTS = ["Sim.EndingOf", "UI.RunEnd", "Calling.LeapDeterminism"]
ALWAYS_MEANING = ["Data.Integrity"]
DATA_ON_MARK = "데이터를 켜면 뜻이 바뀌는 테스트"
DEFAULT_EDITED_TESTS = ["Cosmic_Ascension.UI.Chronicle"]
DEFAULT_TOUCHED = ["Source/Cosmic_Ascension/UI/CosmicChronicleWidget.cpp",
                   "Source/Cosmic_Ascension/Tests/CosmicChronicleScreenTests.cpp",
                   "Tools/data/DT_LogTemplates.json", "Tools/data/generated.sha1"]
NOT_RUN_MARK = re.compile(r"(?:\(|; )no longer applies since")
PROBE_MARK = re.compile(r"commandlet probe|a probe, not a mutation|^\(probe\b", re.I)
GIT = ["git", "-c", "core.eol=lf", "-c", "core.autocrlf=false"]

# ------------------------------------------------------------------------------------------------------------ reading


class Tree:
    """Read-only view of the project: the working tree (rev None) or a revision through git cat-file."""

    def __init__(self, root, rev=None):
        self.root = root
        self.rev = rev
        self._cache = {}
        self._names = None

    def names(self, prefix):
        prefix = prefix.rstrip("/") + "/"
        if self.rev:
            if self._names is None:
                out = subprocess.run(GIT + ["-C", self.root, "ls-tree", "-r", "--name-only", self.rev],
                                     capture_output=True, check=True).stdout.decode("utf-8")
                self._names = [l for l in out.splitlines() if l]
            return [n for n in self._names if n.startswith(prefix)]
        res = []
        base = os.path.join(self.root, prefix)
        for d, _, fs in os.walk(base):
            for f in fs:
                res.append(os.path.relpath(os.path.join(d, f), self.root).replace("\\", "/"))
        return sorted(res)

    def read_many(self, paths):
        need = [p for p in paths if p not in self._cache]
        if need and self.rev:
            spec = "".join("%s:%s\n" % (self.rev, p) for p in need).encode("utf-8")
            out = subprocess.run(["git", "-C", self.root, "cat-file", "--batch"], input=spec, capture_output=True,
                                 check=True).stdout
            pos = 0
            for p in need:
                nl = out.index(b"\n", pos)
                head = out[pos:nl].decode("utf-8", "replace").split()
                if len(head) < 3 or head[1] == "missing":
                    self._cache[p] = None
                    pos = nl + 1
                    continue
                size = int(head[2])
                self._cache[p] = out[nl + 1:nl + 1 + size]
                pos = nl + 1 + size + 1
        elif need:
            for p in need:
                fp = os.path.join(self.root, p)
                self._cache[p] = open(fp, "rb").read() if os.path.isfile(fp) else None
        return {p: self._cache[p] for p in paths}

    def text(self, path):
        data = self.read_many([path])[path]
        return None if data is None else data.decode("utf-8", "replace").replace("\r\n", "\n")

# ------------------------------------------------------------------------------------------------------------ README


def parse_lists(readme_text):
    """The asset list (README:5) and the Python list (README:6): exact names and prefixes ('..._')."""
    flat = " ".join(readme_text.split("\n| ")[0].split("\n"))
    m = re.search(r"A patch of `Tools/data` \((.*?)\) needs the assets rebuilt", flat)
    if not m:
        raise SystemExit("plan: the README's asset sentence ('A patch of `Tools/data` (...) needs the assets rebuilt') is missing")
    asset = [t for t in re.findall(r"`([^`]+)`", m.group(1)) if re.fullmatch(r"[a-z0-9_]+", t)]
    m2 = re.search(r"after the apply and again after the revert\.(.*?)are run by the Python tools alone", flat)
    if not m2:
        raise SystemExit("plan: the README's Python sentence ('... are run by the Python tools alone') is missing")
    py = [t for t in re.findall(r"`([^`]+)`", m2.group(1)) if re.fullmatch(r"[a-z0-9_]+", t)]
    return asset, py


def in_list(tag, entries):
    return any((tag.startswith(e) if e.endswith("_") else tag == e) for e in entries)


def parse_rows(readme_text):
    rows = []
    for n, line in enumerate(readme_text.split("\n"), 1):
        if not line.startswith("| `"):
            continue
        s = line.rstrip()
        if not s.endswith("|"):
            raise SystemExit("plan: README:%d row does not end with '|'" % n)
        parts = s[1:-1].split(" | ")
        if len(parts) < 3:
            raise SystemExit("plan: README:%d row has %d cells" % (n, len(parts)))
        folder = parts[0].strip().strip("`").rstrip("/")
        desc = " | ".join(parts[1:-1]).strip()
        breaks = parts[-1].strip()
        m = re.match(r"`([^`]+)\.patch`", desc)
        if not m:
            raise SystemExit("plan: README:%d has no `<name>.patch` first in its patch cell" % n)
        rows.append({"line": n, "folder": folder, "tag": m.group(1), "desc": desc, "breaks": breaks})
    return rows


def test_names(tree):
    files = [p for p in tree.names("Source") if p.endswith((".cpp", ".h"))]
    names = []
    for p, data in tree.read_many(files).items():
        if data is None:
            continue
        names += re.findall(r'IMPLEMENT_[A-Z_]*AUTOMATION_TEST\(\s*\w+\s*,\s*"(Cosmic_Ascension\.[\w.]+)"',
                            data.decode("utf-8", "replace"))
    return sorted(set(names))


def schema_methods(tree):
    text = tree.text(SCHEMA_TESTS) or ""
    methods = {}
    cls = None
    for line in text.split("\n"):
        m = re.match(r"class (\w+)\(", line)
        if m:
            cls = m.group(1)
        m = re.match(r"    def (test_\w+)\(", line)
        if m and cls:
            methods[m.group(1)] = cls
    return methods


def cpp_filters(breaks, tests):
    """Every `Cosmic_Ascension.X` name of the Breaks cell, then the short `Sim.X`-like names that resolve to a test."""
    out = []
    for t in re.findall(r"Cosmic_Ascension\.[A-Za-z_][\w.]*", breaks):
        t = t.rstrip(".")
        if t not in out:
            out.append(t)
    for t in re.findall(r"`([A-Z][A-Za-z]+\.[A-Z][\w.]*)`", breaks):
        full = "Cosmic_Ascension." + t.rstrip(".")
        if full not in out and any(x.startswith(full) for x in tests):
            out.append(full)
    return out


def python_spec(breaks):
    methods = []
    for t in re.findall(r"\btest_\w+", breaks):
        if t not in methods and t != "test_data_schema":
            methods.append(t)
    check_data = "check_data.py" in breaks
    whole = not methods and not check_data and "test_data_schema.py" in breaks
    return {"methods": methods, "check_data": check_data, "whole": whole}


def patch_targets(text):
    t = [l[6:].strip() for l in text.split("\n") if l.startswith("+++ b/")]
    return t or [l[6:].strip() for l in text.split("\n") if l.startswith("--- a/")]


def diff_files(diff_text):
    files = []
    for l in diff_text.replace("\r\n", "\n").split("\n"):
        m = re.match(r"diff --git a/(\S+) b/(\S+)", l)
        if m and m.group(2) not in files:
            files.append(m.group(2))
    return files


def meaning_tests_from_notes(notes_text):
    """N section 1.3's data-on table: the backticked test names of each row's first cell."""
    if not notes_text or DATA_ON_MARK not in notes_text:
        return None
    lines = notes_text.split("\n")
    i = next(k for k, l in enumerate(lines) if DATA_ON_MARK in l)
    while i < len(lines) and not lines[i].startswith("|"):
        i += 1
    found = []
    k = i + 2  # header and separator
    while k < len(lines) and lines[k].startswith("|"):
        first = lines[k].split("|")[1]
        for t in re.findall(r"`([A-Z][A-Za-z]*\.[A-Za-z]\w*)`", first):
            if t not in found:
                found.append(t)
        k += 1
    return found

# ------------------------------------------------------------------------------------------------------------ plan


def cost_class(kind, targets):
    if kind in ("asset", "probe", "python"):
        return kind
    return "heavy" if any(os.path.basename(t) in HEAVY for t in targets) else "light"


def build_plan(tree, phasea_files=None, apply_state=None, already_rerun=(), meaning_tests=None, edited_tests=None):
    readme = tree.text(README)
    if readme is None:
        raise SystemExit("plan: no %s" % README)
    asset_list, py_list = parse_lists(readme)
    rows = parse_rows(readme)
    tests = test_names(tree)
    methods = schema_methods(tree)
    files = [p for p in tree.names(MUT_DIR) if p.endswith(".patch")]
    texts = tree.read_many(files)
    problems, notes = [], []

    by_path = {}
    for r in rows:
        path = "%s/%s/%s.patch" % (MUT_DIR, r["folder"], r["tag"])
        if path in by_path:
            problems.append("two rows for %s (README:%d and :%d)" % (path, by_path[path]["line"], r["line"]))
        by_path[path] = r
    orphans = sorted(p for p in files if p not in by_path)
    for p in orphans:
        if p.startswith(MUT_DIR + "/B6b/"):
            problems.append("B6b patch file with no README row: %s" % p)
    missing = sorted(p for p in by_path if p not in texts)
    tags = {}
    for r in rows:
        if r["tag"] in tags:
            problems.append("tag %s in two folders (%s, %s)" % (r["tag"], tags[r["tag"]], r["folder"]))
        tags[r["tag"]] = r["folder"]
    for e in asset_list + py_list:
        if not any(in_list(r["tag"], [e]) for r in rows):
            notes.append("README list entry `%s` names no row" % e)

    if meaning_tests is None:
        meaning_tests = meaning_tests_from_notes(tree.text(NOTES))
        if meaning_tests is None:
            notes.append("N section 1.3's data-on table not found - meaning-changed set from the clock list and Data.Integrity only")
            meaning_tests = []
    meaning_full = ["Cosmic_Ascension." + t for t in meaning_tests + CLOCK_TESTS + ALWAYS_MEANING]
    meaning_full = list(dict.fromkeys(meaning_full))
    edited = edited_tests or DEFAULT_EDITED_TESTS
    touched_files = phasea_files if phasea_files is not None else DEFAULT_TOUCHED
    state = dict(apply_state or {})

    items = []
    for idx, r in enumerate(rows):
        path = "%s/%s/%s.patch" % (MUT_DIR, r["folder"], r["tag"])
        raw = texts.get(path)
        text = raw.decode("utf-8", "replace").replace("\r\n", "\n") if raw is not None else ""
        targets = patch_targets(text) if text else []
        tag, desc, breaks = r["tag"], r["desc"], r["breaks"]
        filters = cpp_filters(breaks, tests)
        if in_list(tag, asset_list):
            kind = "asset"
        elif PROBE_MARK.search(desc) or PROBE_MARK.search(breaks) or tag == WITNESS_SMOKE:
            kind = "probe"
        elif in_list(tag, py_list) or not re.search(r"Cosmic_Ascension\.", breaks):
            kind = "python"
        else:
            kind = "cpp"
        live = not NOT_RUN_MARK.search(desc) and not NOT_RUN_MARK.search(breaks)
        sets = []
        if r["folder"] == "B6b":
            sets.append("B6b")
        else:
            if any(t in touched_files for t in targets) or any(e.startswith(f) for f in filters for e in edited):
                sets.append("touched")
            if any(m.startswith(f) for f in filters for m in meaning_full):
                sets.append("meaning")
        item = {"index": idx, "line": r["line"], "folder": r["folder"], "tag": tag, "path": path, "kind": kind,
                "targets": targets, "filters": filters if kind in ("cpp", "asset") else [], "live": live, "sets": sets,
                "cost": cost_class(kind, targets), "apply": state.get(path), "breaks": breaks}
        if kind == "python":
            item["py"] = python_spec(breaks)
            if not (item["py"]["methods"] or item["py"]["check_data"] or item["py"]["whole"]):
                problems.append("%s: a Python row that names no method, no check_data.py and no file" % tag)
            gone = [m for m in item["py"]["methods"] if m not in methods]
            if gone and live:
                problems.append("%s: methods not in %s: %s" % (tag, SCHEMA_TESTS, ", ".join(gone)))
            item["py"]["classes"] = {m: methods.get(m, "SchemaTests") for m in item["py"]["methods"]}
            if filters:
                notes.append("%s: Python row also names %s (not run by the pool)" % (tag, ", ".join(filters)))
        if kind in ("cpp", "asset"):
            if not filters:
                problems.append("%s: a %s row with no Cosmic_Ascension test" % (tag, kind))
            for f in filters:
                if not any(t.startswith(f) for t in tests):
                    notes.append("stale filter %s in %s (README:%d) - matches no IMPLEMENT_ test" % (f, tag, r["line"]))
        if raw is None:
            item["missing_file"] = True
        if len(targets) > 1:
            notes.append("%s touches %d files: %s" % (tag, len(targets), ", ".join(targets)))
        if tag in already_rerun and "B6b" not in sets:
            item["already_rerun"] = True
        items.append(item)

    for it in items:
        it["run"] = bool(it["sets"]) and it["live"] and not it.get("already_rerun") and not it.get("missing_file")
        it["where"] = ("pool" if it["kind"] == "python" else "probes" if it["kind"] == "probe" else "lanes") if it["run"] else None
    if apply_state:
        for it in items:
            if it["run"] and it["apply"] == "FAIL":
                notes.append("%s does not apply on the Phase A tree (apply state FAIL) - it will be NOT APPLIED" % it["tag"])

    for p in orphans:
        raw = texts.get(p)
        tg = patch_targets(raw.decode("utf-8", "replace")) if raw else []
        if any(t in touched_files for t in tg):
            notes.append("orphan %s patches a Phase A file (%s) - no row, no filter: not run" % (p, ", ".join(tg)))
    groups = make_groups([it for it in items if it["where"] == "lanes"])
    return {"items": items, "groups": groups, "orphans": orphans, "missing": missing, "problems": problems, "notes": notes,
            "tests": len(tests), "meaning_tests": meaning_full, "edited_tests": edited, "touched_files": touched_files,
            "asset_list": asset_list, "py_list": py_list}


# Section 5.4's typical costs in seconds: (under the UBT mutex, outside it).
COSTS = {
    "mid": {"light": (12, 9), "heavy": (70, 15), "asset": (0, 130), "probe": (60, 120), "notapplied": (0, 1)},
    "low": {"light": (8, 13), "heavy": (50, 10), "asset": (0, 120), "probe": (50, 110), "notapplied": (0, 1)},
    "high": {"light": (16, 5), "heavy": (95, 15), "asset": (0, 140), "probe": (70, 130), "notapplied": (0, 1)},
}


def item_cost(it, table="mid"):
    c = "notapplied" if it.get("apply") == "FAIL" else it["cost"]
    m, f = COSTS[table][c]
    return m, f


def make_groups(lane_items):
    """Group by target file (asset items by their JSON file - the same key), largest first by the mid cost."""
    groups = {}
    for it in lane_items:
        key = " + ".join(sorted(it["targets"])) or ("?" + it["tag"])
        groups.setdefault(key, []).append(it)
    out = []
    for key, its in groups.items():
        its.sort(key=lambda x: x["index"])
        est = sum(sum(item_cost(i)) for i in its)
        out.append({"key": key, "kind": "asset" if all(i["kind"] == "asset" for i in its) else "cpp",
                    "tags": [i["tag"] for i in its], "est": est})
    out.sort(key=lambda g: (-g["est"], g["key"]))
    return out


def summary(plan):
    from collections import Counter
    lines = []
    rows = plan["items"]
    lines.append("rows %d, patch files %d, orphans %d, rows without file %d, tests %d" %
                 (len(rows), len(rows) - len(plan["missing"]) + len(plan["orphans"]), len(plan["orphans"]),
                  len(plan["missing"]), plan["tests"]))
    c = Counter(r["kind"] for r in rows)
    lines.append("kind of every row: " + ", ".join("%s %d" % (k, c[k]) for k in ("cpp", "asset", "python", "probe")))
    b6 = [r for r in rows if r["folder"] == "B6b"]
    c = Counter(r["kind"] for r in b6)
    lines.append("B6b rows %d: %s; not run (no longer applies) %d" %
                 (len(b6), ", ".join("%s %d" % (k, c[k]) for k in ("cpp", "asset", "python", "probe")),
                  sum(not r["live"] for r in b6)))
    for s in ("touched", "meaning"):
        sel = [r for r in rows if s in r["sets"]]
        c = Counter(r["kind"] for r in sel if r["run"])
        lines.append("older %s: %d rows, run %d (%s), not run %d" %
                     (s, len(sel), sum(r["run"] for r in sel), ", ".join("%s %d" % kv for kv in sorted(c.items())),
                      sum(not r["run"] for r in sel)))
    run = [r for r in rows if r["run"]]
    c = Counter(r["where"] for r in run)
    k = Counter(r["kind"] for r in run)
    lines.append("to run: %d items - lanes %d, pool %d, probes %d; by kind %s" %
                 (len(run), c["lanes"], c["pool"], c["probes"], ", ".join("%s %d" % kv for kv in sorted(k.items()))))
    lines.append("lane groups %d; largest: %s" % (len(plan["groups"]), "; ".join(
        "%s (%d, ~%.0f s)" % (g["key"].split("/")[-1], len(g["tags"]), g["est"]) for g in plan["groups"][:5])))
    lines.append("meaning-changed tests: " + ", ".join(t.replace("Cosmic_Ascension.", "") for t in plan["meaning_tests"]))
    for p in plan["orphans"]:
        lines.append("ORPHAN patch file (no README row): " + p)
    for p in plan["missing"]:
        lines.append("ROW WITHOUT FILE: " + p)
    for p in plan["problems"]:
        lines.append("PROBLEM: " + p)
    for n in plan["notes"]:
        lines.append("note: " + n)
    return lines


def read_apply_state(path):
    st = {}
    for line in open(path, encoding="utf-8"):
        line = line.strip()
        if line:
            ok, p = line.split(" ", 1)
            st[p] = ok
    return st


def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", required=True)
    ap.add_argument("--rev", default=None)
    ap.add_argument("--phaseA-diff", dest="diff", default=None)
    ap.add_argument("--apply-state", default=None)
    ap.add_argument("--already-rerun", default=None)
    ap.add_argument("--meaning-tests", default=None)
    ap.add_argument("--edited-tests", default=None)
    ap.add_argument("--out", default=None)
    ap.add_argument("--quiet", action="store_true")
    a = ap.parse_args(argv)
    tree = Tree(a.root, a.rev)
    pfiles = diff_files(open(a.diff, encoding="utf-8", errors="replace").read()) if a.diff else None
    st = read_apply_state(a.apply_state) if a.apply_state else None
    rerun = set(l.strip() for l in open(a.already_rerun, encoding="utf-8") if l.strip()) if a.already_rerun else set()
    mt = [l.strip() for l in open(a.meaning_tests, encoding="utf-8") if l.strip()] if a.meaning_tests else None
    ed = a.edited_tests.split(",") if a.edited_tests else None
    plan = build_plan(tree, pfiles, st, rerun, mt, ed)
    if a.out:
        with open(a.out, "w", encoding="utf-8", newline="\n") as f:
            json.dump(plan, f, ensure_ascii=False, indent=1)
    if not a.quiet:
        for l in summary(plan):
            print(l)
    return 1 if plan["problems"] or plan["missing"] else 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    sys.exit(main())
