"""[B-6b6] measure stage: commandlet runs of run_checks.ps1's five lines (exact arguments) into Saved/Balance/<folder>/
with absolute -Out and -AbsLog, keys compared with a given baseline json (Remove-WrotePath's rule).
usage: m_runs.py <folder> <baseline json> <comma list of run names or all> [extra args...]"""
import concurrent.futures, json, os, re, subprocess, sys

ROOT = r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension"
ROOTF = ROOT.replace("\\", "/")
EDITOR = r"C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
PROJECT = os.path.join(ROOT, "Cosmic_Ascension.uproject")
folder, basepath, which = sys.argv[1], sys.argv[2], sys.argv[3]
extra_all = sys.argv[4:]
OUT = ROOTF + "/Saved/Balance/" + folder
os.makedirs(OUT, exist_ok=True)

POLICY = ["-Answer=1", "-Build=1", "-Focus=1", "-Edict=1", "-Diplomacy=1", "-TellAll=0"]
RUNS = [
    ("checks", "summary_line", [], False, False, 3000),
    ("checks_ruler", "ruler_summary_line", POLICY, True, False, 3000),
    ("checks_wanderer", "wanderer_summary_line", POLICY + ["-Abdicate=1200", "-Wander=1"], True, True, 6000),
    ("checks_20000", "summary_line_20000", [], False, False, 20000),
    ("checks_ruler_20000", "ruler_summary_line_20000", POLICY, True, False, 20000),
]
if which != "all":
    names = which.split(",")
    RUNS = [r for r in RUNS if r[0] in names]

tl = subprocess.run(["tasklist"], capture_output=True, text=True, errors="replace").stdout
if re.search(r"UnrealEditor|UnrealBuildTool", tl, re.I):
    sys.exit("an UnrealEditor or UBT process runs - not starting")


def run(spec):
    name, field, extra, ruler, wanderer, ticks = spec
    log = OUT + "/" + name + ".log"
    if os.path.exists(log):
        os.remove(log)
    cmd = [EDITOR, PROJECT, "-run=CosmicBalance", "-Seed=42", "-Ticks=%d" % ticks, "-Out=%s/%s.csv" % (OUT, name)] + extra + extra_all + [
        "-unattended", "-nosplash", "-stdout", "-FullStdOutLogOutput", "-AbsLog=" + log]
    r = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return name, r.returncode


def key_of(name, ruler, wanderer):
    text = open(OUT + "/" + name + ".log", encoding="utf-8", errors="replace").read().splitlines()
    def last(pat):
        hits = [l for l in text if pat in l]
        if not hits:
            raise RuntimeError("no %r in %s.log" % (pat, name))
        return re.search("(" + re.escape(pat) + ".*)", hits[-1]).group(1)
    s = last("CosmicBalance: seed")
    if ruler:
        s += " | " + last("CosmicBalance ruler:")
    if wanderer:
        s += " | " + last("CosmicBalance wanderer:")
    return s


def strip(s):
    return re.sub(r", wrote .*?(?= \| |$)", "", s, flags=re.I)


with concurrent.futures.ThreadPoolExecutor(max_workers=5) as ex:
    results = list(ex.map(run, sorted(RUNS, key=lambda r: -r[5])))
for name, code in results:
    print("%s exit %d" % (name, code))
base = json.load(open(basepath, encoding="utf-8"))
allok = True
for name, field, extra, ruler, wanderer, ticks in RUNS:
    key = key_of(name, ruler, wanderer)
    text = open(OUT + "/" + name + ".log", encoding="utf-8", errors="replace").read()
    checks = len(re.findall(r"CosmicBalance end check|CosmicBalance race check", text))
    errors = len(re.findall(r"LogCosmic: Error", text))
    taken = re.findall(r"rows taken out \[[^\]]*\]", text)
    same = strip(key) == strip(base[field])
    allok &= same
    head = re.search(r"seed \d+, \d+ ticks, hash \d+, world \d+", key).group(0)
    print("%-20s %s | equal to %s: %s | check lines %d | LogCosmic errors %d | %s" % (name, head, field, same, checks, errors, taken))
    if not same:
        print("    now:  " + strip(key)[:900])
        print("    base: " + strip(base[field])[:900])
print("ALL KEYS EQUAL" if allok else "KEYS DIFFER")
