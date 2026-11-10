"""Apply each mutation patch, build the editor target, run its test, revert. Prints one line per patch: KILLED when the test
fails under the mutation (what we want), SURVIVED when it passes, BUILD FAILED when the mutant does not compile.
Usage (project root, editor closed): python -I run_mutations.py <patch>=<test filter> ..."""
import os
import re
import subprocess
import sys

sys.stdout.reconfigure(encoding="utf-8")
ROOT = os.getcwd()
UE = r"C:\Program Files\Epic Games\UE_5.8"
BUILD = os.path.join(UE, r"Engine\Build\BatchFiles\Build.bat")
EDITOR = os.path.join(UE, r"Engine\Binaries\Win64\UnrealEditor-Cmd.exe")
PROJECT = os.path.join(ROOT, "Cosmic_Ascension.uproject")
LOGS = os.path.join(ROOT, "Saved", "Logs")


def build():
    r = subprocess.run([BUILD, "Cosmic_AscensionEditor", "Win64", "Development", "-Project=" + PROJECT, "-WaitMutex"],
                       capture_output=True, text=True, encoding="utf-8", errors="replace")
    return "Result: Succeeded" in r.stdout


def run_test(test, tag):
    log = os.path.join(LOGS, "Mutation_%s.log" % tag)
    subprocess.run([EDITOR, PROJECT, "-game", "-nullrhi", "-nosound", "-unattended", "-nosplash", "-stdout", "-FullStdOutLogOutput",
                    "-ExecCmds=Automation RunTests %s; Quit" % test, "-TestExit=Automation Test Queue Empty", "-AbsLog=" + log],
                   capture_output=True)
    text = open(log, encoding="utf-8", errors="replace").read() if os.path.exists(log) else ""
    results = re.findall(r"Test Completed\. Result=\{(\w+)\}", text)
    return results


for arg in sys.argv[1:]:
    patch, test = arg.split("=", 1)
    tag = os.path.splitext(os.path.basename(patch))[0]
    if subprocess.run(["git", "apply", patch]).returncode != 0:
        print(tag, "PATCH DID NOT APPLY", flush=True)
        continue
    try:
        if not build():
            print(tag, "BUILD FAILED", flush=True)
            continue
        results = run_test(test, tag)
        verdict = "KILLED" if any(r != "Success" for r in results) else ("SURVIVED" if results else "NO RESULT")
        print(tag, verdict, results, flush=True)
    finally:
        subprocess.run(["git", "apply", "-R", patch])
print("clean build:", "ok" if build() else "FAILED", flush=True)
