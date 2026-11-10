"""[B-6b6] final stage: run_mutations2.py's loop (apply, build or rebuild assets, run the named tests, revert) with one
change - after every revert each patched file gets its snapshot bytes back (git apply in this tree writes CRLF), before
the reverting asset rebuild. No clean build at the end (the stage rebuilds both targets itself).
usage (project root, editor closed): python -I -X utf8 run_final_mut.py <snapshot dir> <list file of patch=filter>"""
import os
import re
import shutil
import subprocess
import sys
import time

sys.stdout.reconfigure(encoding="utf-8")
ROOT = os.getcwd()
UE = r"C:\Program Files\Epic Games\UE_5.8"
BUILD = os.path.join(UE, r"Engine\Build\BatchFiles\Build.bat")
EDITOR = os.path.join(UE, r"Engine\Binaries\Win64\UnrealEditor-Cmd.exe")
PROJECT = os.path.join(ROOT, "Cosmic_Ascension.uproject")
LOGS = os.path.join(ROOT, "Saved", "Logs")
ROOTF = ROOT.replace("\\", "/")
SNAP = sys.argv[1]


def build():
    r = subprocess.run([BUILD, "Cosmic_AscensionEditor", "Win64", "Development", "-Project=" + PROJECT, "-WaitMutex"],
                       capture_output=True, text=True, encoding="utf-8", errors="replace")
    return "Result: Succeeded" in r.stdout


def make_data(tag):
    log = os.path.join(LOGS, "MutationData_%s.log" % tag)
    subprocess.run([EDITOR, PROJECT, "-run=pythonscript", "-Script=%s/Tools/make_data_tables.py" % ROOTF, "-unattended",
                    "-nosplash", "-stdout", "-FullStdOutLogOutput", "-AbsLog=" + log], capture_output=True)
    text = open(log, encoding="utf-8", errors="replace").read() if os.path.exists(log) else ""
    return "written and verified" in text


def run_test(test, tag):
    log = os.path.join(LOGS, "Mutation_%s.log" % tag)
    subprocess.run([EDITOR, PROJECT, "-game", "-nullrhi", "-nosound", "-unattended", "-nosplash", "-stdout", "-FullStdOutLogOutput",
                    "-ExecCmds=Automation RunTests %s; Quit" % test, "-TestExit=Automation Test Queue Empty", "-AbsLog=" + log],
                   capture_output=True)
    text = open(log, encoding="utf-8", errors="replace").read() if os.path.exists(log) else ""
    results = re.findall(r"Test Completed\. Result=\{(\w+)\}", text)
    names = re.findall(r"Test Completed\. Result=\{\w+\} Name=\{(\w+)\}", text)
    errors = [l.strip()[:300] for l in text.splitlines() if "Error:" in l and ("Expected" in l or "LogAutomation" in l)][:4]
    return results, names, errors


def targets(patch):
    return [l[6:].strip() for l in open(patch, encoding="utf-8", errors="replace") if l.startswith("+++ b/")]


def put_back(patch):
    for t in targets(patch):
        snap = os.path.join(SNAP, t)
        if os.path.exists(snap):
            src = open(snap, "rb").read()
        else:
            src = subprocess.run(["git", "show", "HEAD:" + t], capture_output=True, check=True).stdout
        cur = open(t, "rb").read()
        if cur != src:
            if cur.replace(b"\r\n", b"\n") != src.replace(b"\r\n", b"\n"):
                print("    REVERT LEFT CONTENT:", t, flush=True)
            open(t, "wb").write(src)


items = [l.strip() for l in open(sys.argv[2], encoding="utf-8") if l.strip()]
for arg in items:
    t0 = time.time()
    patch, test = arg.split("=", 1)
    tag = os.path.splitext(os.path.basename(patch))[0]
    data = "_data_" in tag
    if subprocess.run(["git", "apply", patch]).returncode != 0:
        print(tag, "PATCH DID NOT APPLY", flush=True)
        continue
    try:
        if data:
            print(tag, "assets:", "written" if make_data(tag) else "NOT written (checker stopped)", flush=True)
        elif not build():
            print(tag, "BUILD FAILED", flush=True)
            continue
        results, names, errors = run_test(test, tag)
        verdict = "KILLED" if any(r != "Success" for r in results) else ("SURVIVED" if results else "NO RESULT")
        print(tag, verdict, list(zip(names, results)) or results, "%.0fs" % (time.time() - t0), flush=True)
        for e in errors:
            print("    ", e, flush=True)
    finally:
        subprocess.run(["git", "apply", "-R", patch])
        put_back(patch)
        if data:
            print(tag, "assets after revert:", "written" if make_data(tag + "_revert") else "NOT written", flush=True)
print("done", flush=True)
