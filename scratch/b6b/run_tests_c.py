"""python -I run_tests_c.py [data] <filter> <tag> - optionally rebuild the assets, then run the tests of a filter and print results."""
import os, re, subprocess, sys
sys.stdout.reconfigure(encoding="utf-8")
ROOT = os.getcwd()
UE = r"C:\Program Files\Epic Games\UE_5.8"
EDITOR = os.path.join(UE, r"Engine\Binaries\Win64\UnrealEditor-Cmd.exe")
PROJECT = os.path.join(ROOT, "Cosmic_Ascension.uproject")
LOGS = os.path.join(ROOT, "Saved", "Logs")
ROOTF = ROOT.replace("\\", "/")
args = sys.argv[1:]
if args and args[0] == "data":
    args = args[1:]
    log = os.path.join(LOGS, "MakeDataTables_b6b2c.log")
    subprocess.run([EDITOR, PROJECT, "-run=pythonscript", "-Script=%s/Tools/make_data_tables.py" % ROOTF, "-unattended",
                    "-nosplash", "-stdout", "-FullStdOutLogOutput", "-AbsLog=" + log], capture_output=True)
    text = open(log, encoding="utf-8", errors="replace").read() if os.path.exists(log) else ""
    print("assets:", "written and verified" in text)
test, tag = args[0], args[1]
log = os.path.join(LOGS, "Tests_%s.log" % tag)
subprocess.run([EDITOR, PROJECT, "-game", "-nullrhi", "-nosound", "-unattended", "-nosplash", "-stdout", "-FullStdOutLogOutput",
                "-ExecCmds=Automation RunTests %s; Quit" % test, "-TestExit=Automation Test Queue Empty", "-AbsLog=" + log], capture_output=True)
text = open(log, encoding="utf-8", errors="replace").read() if os.path.exists(log) else ""
names = re.findall(r"Test Completed\. Result=\{(\w+)\} Name=\{([^}]*)\}", text)
ok = sum(1 for r, n in names if r == "Success")
print("tests:", len(names), "success:", ok)
for r, n in names:
    if r != "Success":
        print("  FAIL", n)
for l in text.splitlines():
    if ("Error:" in l or "error:" in l) and ("Expected" in l or "LogAutomation" in l or "Assert" in l):
        print("   ", l.strip()[:400])
