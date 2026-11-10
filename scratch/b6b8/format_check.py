"""Read-only format check of bundleB.py against HEAD's commandlet (git show copy ref/cmdlet_HEAD_220129d.cpp) and the
real logs HEAD's binaries wrote in Saved\\Balance\\b6b7_on and b6b7_off. Writes nothing."""
import glob
import importlib.util
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension"


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


B = load("bundleB", os.path.join(HERE, "bundleB.py"))
T = load("bundleA_selftest", os.path.join(os.path.dirname(HERE), "b6b6", "bundleA_selftest.py"))
cpp = open(os.path.join(HERE, "ref", "cmdlet_HEAD_220129d.cpp"), encoding="utf-8").read()
ok = [True]


def say(cond, what):
    print(("ok   " if cond else "FAIL ") + what)
    ok[0] &= bool(cond)


# 1. The off line's exact HEAD text and its tail.
off_fmt = T.fmt_on_line(cpp, "rows taken out [%s]")
print("HEAD off format:", off_fmt)
tail = "AIUpperRungProgressMultiplier 1 (was %g), FleetDelayCountdownShare 0 (was %g)"
say(off_fmt.endswith("rows taken out [%s], " + tail), "the off line ends in 'rows taken out [%s], " + tail + "'")
rendered = T.cprintf(off_fmt, {0: B.EXPECTED_REMOVED, 1: 0.375, 2: 0.0})
m = B.RE_OFF_KNOBS.search(rendered)
say(m and m.group("r") == "0.375" and m.group("share") == "0", "RE_OFF_KNOBS reads r 0.375 / share 0 from %r" % rendered[-90:])
names = B.EXPECTED_REMOVED.split()
say(len(names) == 10 and len(set(names)) == 10 and names[-2:] == ["event_era_endured", "enc_witness"], "EXPECTED_REMOVED: ten distinct names, the two side rows last: %s" % names)

# 2. The ending, wanderer, calling-deaths lines: present at HEAD with the fields the parser reads.
end_fmt = T.fmt_on_line(cpp, 'TEXT("CosmicBalance ending: record')
say(end_fmt.startswith("CosmicBalance ending: record %s at %lld (people %s, a people some life ruled %d, no vessel %d)"), "ending head: %s" % end_fmt[:110])
wan_fmt = T.fmt_on_line(cpp, 'TEXT("CosmicBalance wanderer:')
say("left at %lld, crowned at %lld (%s, tier %d" in wan_fmt, "wanderer line: %s" % wan_fmt[:120])
dea_fmt = T.fmt_on_line(cpp, 'TEXT("CosmicBalance calling deaths:')
say(dea_fmt.startswith("CosmicBalance calling deaths: death asked at %lld (recorded %d)"), "calling deaths: %s" % dea_fmt[:80])

# 3. The race parts at HEAD: 17, the side part in the old layout (HEAD) - the brief's 2.3 layout reads with RE_SIDE_NEW.
F = T.Formats(cpp)
say(len(F.parts) == 17, "17 race parts at HEAD")
say(F.parts[12] == "era lines %d (by the record %d, first at %lld), witness cards %d (by the record %d; succeeded %d, failed %d, withdrawn %d)",
    "HEAD side part = the old layout: %s" % F.parts[12])
brief_side = ("era lines %d (by the record %d, first at %lld), witness cards %d (first at %lld, by the record %d; answered %d: by the check %d, "
              "failed %d, without a check %d; gone before the answer %d)")
brief_txt = open(os.path.join(HERE, "brief_final.md"), encoding="utf-8").read()
say(("`\"%s\"`" % brief_side) in brief_txt, "the brief's 2.3 layout is the string tested here")
vals = dict(enumerate([11, 12, 1300, 14, 1500, 16, 17, 18, 19, 20, 21]))
m = B.RE_SIDE_NEW.search(T.cprintf(brief_side, vals))
want = dict(era=11, era_rec=12, era_first=1300, wit=14, wit_first=1500, wit_rec=16, answered=17, passed=18, failed=19, nocheck=20, gone=21)
say(m and {k: int(v) for k, v in m.groupdict().items()} == want, "RE_SIDE_NEW maps every field of the 2.3 layout: %s" % (m.groupdict() if m else None))
say(B.RE_SIDE_OLD.search(T.cprintf(F.parts[12], dict(enumerate([1, 2, 3, 4, 5, 6, 7, 8])))) is not None, "RE_SIDE_OLD reads HEAD's layout")
for k, rx in B.RE_B_PARTS.items():
    hit = [p for p in F.parts if rx.search(T.cprintf(p, {})) or False]
    say(len(hit) == 1, "B part '%s' matches exactly one HEAD part" % k)

# 4. Real logs of HEAD's binaries ([B-6b7]'s on/off runs).
for sub in ("b6b7_on", "b6b7_off"):
    logs = sorted(glob.glob(os.path.join(ROOT, "Saved", "Balance", sub, "*.log")))
    for path in logs:
        if os.path.basename(path).startswith("hashrun"):
            continue   # the game's -CosmicHashRun run, not the commandlet: no race line
        p = B.parse_log(path)
        rb = p.get("race_b") or {}
        bad = [k for k, v in rb.items() if v is None] + ([] if p.get("ending_x") else ["ending head"])
        side = (rb.get("side") or {}).get("layout")
        print("  %s/%s: ticks %s, B parts %s, side %s, ending %s, off %s, taken out %s, left at %s" % (
            sub, os.path.basename(path), (p.get("seed") or {}).get("ticks") if isinstance(p.get("seed"), dict) else p.get("ticks"), "all read" if not bad else bad, side,
            (p.get("ending_x") or {}).get("kind"), p.get("off_knobs"), (p.get("rows_taken_out") or "")[:40] + ("..." if p.get("rows_taken_out") else ""), p.get("left_at")))
        say(not bad and side == "old", "%s/%s: every B part read, the old side layout" % (sub, os.path.basename(path)))
        if sub == "b6b7_off":
            say(p.get("off_knobs") == {"r": "0.375", "share": "0"} and len((p.get("rows_taken_out") or "").split()) == 8,
                "%s: off tail r 0.375 share 0, eight names at HEAD (this commit makes ten)" % os.path.basename(path))
print("format check: %s" % ("PASS" if ok[0] else "FAIL"))
sys.exit(0 if ok[0] else 1)
