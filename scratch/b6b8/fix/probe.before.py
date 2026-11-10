"""[B-6b8] the seed-42 probe (brief 3.3).
usage: probe.py collect   - copy the smoke runner's own outputs (Saved/Balance/checks*.csv, Saved/Logs/checks*.log, hashrun*.log) into Saved/Balance/b6b8_on
       probe.py run       - the two 20,000-tick on-runs (runner args) into b6b8_on and the five off-runs (-WithoutB6bData=1) into b6b8_off
       probe.py check     - checks a, b, c and each race line's era / witness part
Never run 'run' while an editor / UBT / commandlet process builds."""
import concurrent.futures as cf
import json, os, re, shutil, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.dirname(HERE))
import bundleB as B  # noqa: E402

A = B.A
ROOT = r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension"
PROJECT = os.path.join(ROOT, "Cosmic_Ascension.uproject")
EDITOR = r"C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
BAL = os.path.join(ROOT, "Saved", "Balance")
LOGS = os.path.join(ROOT, "Saved", "Logs")
ON, OFF, BASE = (os.path.join(BAL, n) for n in ("b6b8_on", "b6b8_off", "b6b8_base"))
RULER = ["-Answer=1", "-Build=1", "-Focus=1", "-Edict=1", "-Diplomacy=1", "-TellAll=0"]
RUNS = [  # (name, seed42.json field, ticks, args, cell kind)
    ("checks", "summary_line", 3000, [], "inputless"),
    ("checks_ruler", "ruler_summary_line", 3000, RULER, "ruler"),
    ("checks_wanderer", "wanderer_summary_line", 6000, RULER + ["-Abdicate=1200", "-Wander=1"], "wanderer"),
    ("checks_20000", "summary_line_20000", 20000, [], "inputless"),
    ("checks_ruler_20000", "ruler_summary_line_20000", 20000, RULER, "ruler"),
]
OFF_KEYS = {  # [B-6b5]'s keys (brief 3.3 2a)
    "checks": (2816687688, 3300228750), "checks_ruler": (3561771757, 2462331303), "checks_wanderer": (2665013209, 2229239127),
    "checks_20000": (2359424635, 726895141), "checks_ruler_20000": (1611658162, 29194274),
}
TEN = ("feat_passage_by_ark feat_laws_and_binding project_ark project_binding crisis_species_extinction edict_wide_machine_accord "
       "edict_wide_quarantine edict_wide_warning event_era_endured enc_witness")
OFF_LINE = "rows taken out [%s], AIUpperRungProgressMultiplier 1 (was 0.375), FleetDelayCountdownShare 0 (was 0)" % TEN


def fwd(p):
    return p.replace("\\", "/")


def job(folder, name, ticks):
    b = os.path.join(folder, name)
    return {"name": name, "cell": name, "ticks": ticks, "csv": b + ".csv", "rows": b + "_rows.csv", "claims": b + "_claims.csv", "log": b + ".log"}


def collect():
    os.makedirs(ON, exist_ok=True)
    for name, _, ticks, _, _ in RUNS[:3]:
        for suf in (".csv", "_rows.csv", "_claims.csv"):
            shutil.copy2(os.path.join(BAL, name + suf), os.path.join(ON, name + suf))
        shutil.copy2(os.path.join(LOGS, name + ".log"), os.path.join(ON, name + ".log"))
    for n in ("hashrun", "hashrun_policy"):
        shutil.copy2(os.path.join(LOGS, n + ".log"), os.path.join(ON, n + ".log"))
    print("collected into", ON)


def cmd(folder, name, ticks, args, off):
    # On-runs exactly as Tools/run_checks.ps1's Compare-Balance (-Out=<name>.csv into Saved/Balance, the log in Saved/Logs - so the
    # 'wrote' path is the runner's), then copied into b6b8_on; off-runs straight into b6b8_off.
    j = job(folder, name, ticks)
    out, log = (name + ".csv", fwd(os.path.join(LOGS, name + ".log"))) if not off else (fwd(j["csv"]), fwd(j["log"]))
    return [EDITOR, PROJECT, "-run=CosmicBalance", "-Seed=42", "-Ticks=%d" % ticks, "-Out=" + out] + args \
        + (["-WithoutB6bData=1"] if off else []) + ["-unattended", "-nosplash", "-stdout", "-FullStdOutLogOutput", "-AbsLog=" + log]


def run():
    os.makedirs(ON, exist_ok=True)
    os.makedirs(OFF, exist_ok=True)
    work = [(ON, n, t, a, False) for n, _, t, a, _ in RUNS[3:]] + [(OFF, n, t, a, True) for n, _, t, a, _ in RUNS]
    for folder, n, t, a, off in work:
        for suf in (".csv", "_rows.csv", "_claims.csv", ".log"):
            p = os.path.join(folder, n + suf)
            if os.path.exists(p):
                os.remove(p)

    def go(w):
        c = cmd(*w)
        r = subprocess.run(c, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        return w[1], w[4], r.returncode
    for n, _, t, _, _ in RUNS[3:]:
        for p in [os.path.join(BAL, n + suf) for suf in (".csv", "_rows.csv", "_claims.csv")] + [os.path.join(LOGS, n + ".log"), os.path.join(LOGS, n + "_2.log")]:
            if os.path.exists(p):
                os.remove(p)
    with cf.ThreadPoolExecutor(max_workers=7) as ex:
        for name, off, code in ex.map(go, work):
            print("%-20s %s exit %d" % (name, "off" if off else "on ", code))
    for n, _, t, _, _ in RUNS[3:]:
        for suf in (".csv", "_rows.csv", "_claims.csv"):
            shutil.copy2(os.path.join(BAL, n + suf), os.path.join(ON, n + suf))
        shutil.copy2(os.path.join(LOGS, n + ".log"), os.path.join(ON, n + ".log"))


def strip(line):
    return re.sub(r", wrote [^|]*?(?= \| |$)", "", line.strip())


def key_line(log, want_heads):
    text = A.read_text(log).splitlines()
    head, parts = None, []
    for l in text:
        m = re.search(r"(CosmicBalance: seed .*)$", l)
        if m:
            head = m.group(1)
        m2 = re.search(r"(CosmicBalance (ruler|wanderer): .*)$", l)
        if m2 and head is not None and m2.group(2) in want_heads:
            parts.append(m2.group(1))
    return strip(" | ".join([head] + parts)) if head else None


def heads_of(kind):
    return {"inputless": (), "ruler": ("ruler",), "wanderer": ("ruler", "wanderer")}[kind]


def check():
    head_json = json.loads(subprocess.run(["git", "-C", ROOT, "show", "HEAD:Tools/baselines/seed42.json"], capture_output=True, check=True).stdout.decode("utf-8"))
    ok_a = ok_b = ok_c = True
    out = {}
    print("== check a (off runs = [B-6b5]'s keys, the ten-name line)")
    for name, field, ticks, _, kind in RUNS:
        p = B.parse_log(job(OFF, name, ticks)["log"])
        s = p.get("seed_line") or {}
        got = (s.get("hash"), s.get("world"))
        same = got == OFF_KEYS[name] and s.get("ticks") == ticks
        text = A.read_text(job(OFF, name, ticks)["log"])
        line_ok = OFF_LINE in text and p.get("rows_taken_out_lines", 0) >= 1
        ok_a &= same and line_ok
        print("%-20s keys %s %s, ten-name line %s" % (name, got, "SAME" if same else "DIFF want %s" % (OFF_KEYS[name],), "OK" if line_ok else "MISSING"))
    print("== check b (on vs b6b8_base)")
    for name, field, ticks, _, kind in RUNS:
        on, base = job(ON, name, ticks), job(BASE, name, ticks)
        pon = B.parse_log(on["log"])
        side = (pon.get("race_b") or {}).get("side")
        E = B.inf_if_neg(side["era_first"]) if side else None
        W = B.inf_if_neg(side["wit_first"]) if side and side["layout"] == "new" else None
        limit = min(E, W) if kind == "wanderer" else E
        why, first = B.rows_equal_below(base, on, limit)
        want = strip(head_json[field])
        got = key_line(on["log"], heads_of(kind))
        silent = side is not None and side["era"] == 0 and side["wit"] == 0
        if silent and got != want:
            why.append("era 0 and witness 0 but the key line differs from B6b7_KEYS")
        if side is None or side["layout"] != "new":
            why.append("side part unreadable or not in the new layout")
        ok_b &= not why
        moved = got != want
        out[name] = {"side": side, "E": E, "W": W, "limit": limit, "first_diff": first, "moved": moved, "key_now": got, "key_before": want,
                     "ending": pon.get("ending_x"), "T": A.threshold_of(pon.get("ending"), ticks)}
        print("%-20s %s limit %s first diff csv %s rows %s | key %s | era %s first %s, witness %s first %s (by record %s; answered %s: check %s failed %s none %s; gone %s)%s" % (
            name, "OK" if not why else "FAIL", limit, first.get("csv"), first.get("rows"), "MOVED" if moved else "same",
            side and side["era"], side and side["era_first"], side and side["wit"], side and side.get("wit_first"), side and side["wit_rec"],
            side and side.get("answered"), side and side.get("passed"), side and side.get("failed"), side and side.get("nocheck"), side and side["gone"],
            "" if not why else " WHY " + "; ".join(why)))
        if moved:
            print("    now:    " + got)
            print("    before: " + want)
    print("== check c (end check / race check / LogCosmic: Error)")
    logs = [job(ON, n, t)["log"] for n, _, t, _, _ in RUNS] + [job(OFF, n, t)["log"] for n, _, t, _, _ in RUNS] \
        + [os.path.join(ON, "hashrun.log"), os.path.join(ON, "hashrun_policy.log")]
    for log in logs:
        text = A.read_text(log)
        ec, rc = len(re.findall(r"CosmicBalance end check", text)), len(re.findall(r"CosmicBalance race check", text))
        er = [l for l in text.splitlines() if "LogCosmic: Error" in l]
        ok_c &= ec == 0 and rc == 0 and not er
        print("%-60s end check %d race check %d errors %d%s" % (os.path.relpath(log, BAL), ec, rc, len(er), "" if not er else " e.g. " + er[0][:200]))
    json.dump(out, open(os.path.join(HERE, "probe_out.json"), "w", encoding="utf-8"), indent=1, default=str)
    print("CHECK A %s, CHECK B %s, CHECK C %s" % ("PASS" if ok_a else "FAIL", "PASS" if ok_b else "FAIL", "PASS" if ok_c else "FAIL"))
    return 0 if ok_a and ok_b and ok_c else 1


if __name__ == "__main__":
    mode = sys.argv[1]
    sys.exit({"collect": collect, "run": run, "check": check}[mode]() or 0)
