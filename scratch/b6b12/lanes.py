"""[B-6b12] the whole-B6b mutation rerun (brief section 5): the Python pool first, then the lanes, then the probes, then
the merge. Every external command goes through an Env (RealEnv, or SimEnv for --dry-run and the self-test).

Phases (--phase, default all = pool, lanes, probes, merge):
  pool   - section 5.6: copies of Source/Tools/Config/CLAUDE.md in <base>\\py\\c0..c3, a clean green run of both tools in
           each, the Python items with the row's methods (KILLED only on 'FAILED (failures=' naming a row method, or on
           check_data's problem count; 'errors=' alone is NO RESULT), a clean run at the end.
  lanes  - section 5.3-5.5: git worktrees <base>\\l1..lN (every lane git call with -c core.eol=lf -c core.autocrlf=false),
           phaseA.diff applied and checked, per-lane TMP/TEMP, the gate build of lane 1 with -WaitMutex -DisableUnity
           (fallback: -WaitMutex for every lane), the Phase A assets copied, the baseline run (-unattended, working set
           sampled every 5 s), the memory gate max(2.0 GB, peak + 0.5 GB) before every build and editor launch, then a
           shared queue of whole groups (largest first); one line per item; end-of-lane clean build and status checks.
  probes - after every lane has drained: the probe rows on lane 1 (--probes <json> for the commandlet probes; the
           b6b8 witness smoke through bundleB.py, clean then mutant).
  merge  - section 5.7: b6b_all_mutations.txt in README order, dispositions.tsv for the lead, mut_counts.json.

usage:
  python -I -X utf8 lanes.py --plan plan.json --root <project root> --phaseA-diff phaseA.diff --assets phaseA_assets
                             [--lanes 3] [--base C:\\CAL] [--out <dir>] [--phase all|pool|lanes|probes|merge]
                             [--probes probes.json] [--only <file of tags>] [--reuse-lanes] [--lane-flags "-WaitMutex"]
  python -I -X utf8 lanes.py --plan plan.json --dry-run [--lanes 3]      every external command mocked (virtual clock)
  python -I -X utf8 lanes.py --plan plan.json --estimate                 section 5.4's costs, 3 and 2 lanes (no run)
The real run refuses while an UnrealEditor / UBT process runs and never creates anything outside --base and --out."""
import argparse
import collections
import csv
import ctypes
import hashlib
import heapq
import io
import json
import os
import re
import shutil
import subprocess
import sys
import threading
import time
import zlib

HERE = os.path.dirname(os.path.abspath(__file__))
SCR = os.path.dirname(HERE)
UE = r"C:\Program Files\Epic Games\UE_5.8"
BUILD = os.path.join(UE, r"Engine\Build\BatchFiles\Build.bat")
EDITOR = os.path.join(UE, r"Engine\Binaries\Win64\UnrealEditor-Cmd.exe")
GITC = ["-c", "core.eol=lf", "-c", "core.autocrlf=false"]
MAP_RELOAD = "The map did not load again within 15 seconds."
WITNESS_SMOKE = "b6b8_cmdlet_witness_first_any_card"
BUNDLE_B = os.path.join(SCR, "b6b8", "bundleB.py")
WITNESS_PROBLEMS = [re.compile(r"witness first at -?\d+ before the wanderer left at"),
                    re.compile(r"witness first at -?\d+ before the first claim placed"),
                    re.compile(r"witness cards \d+ with first at -?\d+")]
FINAL = ("KILLED", "SURVIVED", "NOT APPLIED", "BUILD FAILED")
UNITY_OFF = ["-WaitMutex", "-DisableUnity"]
UNITY_ADAPTIVE = ["-WaitMutex"]
MIN_FREE_GB = 2.0
TWO_LANES_BELOW_GB = 8.0

# ===================================================================================================== log verdicts


COMPLETED = re.compile(r"Test Completed\. Result=\{(\w+)\} Name=\{([^}]*)\} Path=\{([^}]*)\}")


def parse_test_log(text):
    """The automation controller's lines: each 'Test Completed' line, and the 'LogAutomationController: Error:' lines
    that follow it (until the next completed line) as that test's errors. complete: the controller's 'TEST COMPLETE.
    EXIT CODE' line was seen (a normal end of the queue); crash: the first critical-error / assertion line, if any."""
    results, errors = [], collections.OrderedDict()
    current = None
    complete = False
    crash = None
    for line in text.splitlines():
        m = COMPLETED.search(line)
        if m:
            current = m.group(3)
            results.append((m.group(1), m.group(2), current))
            errors.setdefault(current, [])
            continue
        if "TEST COMPLETE. EXIT CODE" in line:
            current = None
            complete = True
            continue
        if crash is None and ("Critical error" in line or "Assertion failed" in line or "Fatal error" in line):
            crash = line.strip()[:300]
        if current and "LogAutomationController: Error:" in line:
            errors[current].append(line.strip()[:300])
    return {"results": results, "errors": errors, "complete": complete, "crash": crash}


INCOMPLETE = "the editor ended before the test queue emptied (no 'TEST COMPLETE. EXIT CODE' line)"


def verdict_of(parsed):
    """(verdict, rerun paths, error lines). KILLED when a test failed with an error other than the 15-second map
    reload; NO RESULT when the queue did not end (no 'TEST COMPLETE' line: the editor died or hung mid-run) and no
    test failed for real; NO RESULT with rerun paths when the map reload was the only error of every failed test;
    NO RESULT when nothing ran; SURVIVED when every test passed and the queue ended."""
    results, errors = parsed["results"], parsed["errors"]
    complete = parsed.get("complete", True)
    incomplete = [INCOMPLETE] + ([parsed["crash"]] if parsed.get("crash") else [])
    if not results:
        return "NO RESULT", [], ["no 'Test Completed' line (no test matched, or the editor did not run them)"] + \
            (incomplete[1:] if not complete else [])
    failed = [(r, n, p) for r, n, p in results if r != "Success"]
    if not failed:
        if not complete:
            return "NO RESULT", [], incomplete
        return "SURVIVED", [], []
    real, reload_only = [], []
    for r, n, p in failed:
        errs = [e for e in errors.get(p, []) if "Test Completed" not in e]
        if errs and all(MAP_RELOAD in e for e in errs):
            reload_only.append(p)
        else:
            real.append(p)
    if real:
        lines = []
        for p in real:
            lines += [e for e in errors.get(p, [])][:4]
        return "KILLED", [], (lines or ["%s failed without an error line" % p for p in real])[:4]
    if not complete:
        return "NO RESULT", [], incomplete + ["map reload only (15 s deadline): " + ", ".join(reload_only)]
    return "NO RESULT", reload_only, ["map reload only (15 s deadline): " + ", ".join(reload_only)]


MAKE_DATA_DONE = "[make_data_tables] done"


def make_data_outcome(rc, text, so=""):
    """Tools/make_data_tables.py run by -run=pythonscript: ("written", detail) only on its last line
    '[make_data_tables] done' (every table filled and verified, then every asset saved); ("refused", detail) when it
    stopped on a RuntimeError (the checker's refusal, or a table that does not hold what was written; docs/40:40);
    ("no result", detail) otherwise - the editor died, hung or left no log, so nothing can be read from it.
    A '<table>: N rows written and verified' line alone proves nothing: the assets are saved only after every table."""
    both = (text or "") + "\n" + (so or "")
    if MAKE_DATA_DONE in both:
        return "written", "done"
    lines = [l.strip() for l in both.splitlines() if l.strip()]
    if "LogPython: Error:" in both and "RuntimeError" in both:
        first = next((l for l in lines if "RuntimeError" in l), "")
        return "refused", first[:300]
    last = lines[-1][:200] if lines else "log empty"
    return "no result", "rc %d, %s" % (rc, last if (text or "").strip() else "log empty" + (" (stdout: %s)" % last if lines else ""))


def strip_wrote(line):
    """run_checks.ps1's Remove-WrotePath: ', wrote <path>' up to ' | ' or the end."""
    return re.sub(r", wrote .*?(?= \| |$)", "", line)


def py_verdict(spec, outs):
    """Section 5.6's kill rule. outs: [(tool, rc, text)] with tool 'schema' or 'check_data'."""
    killed, noresult, summary = [], [], []
    for tool, rc, text in outs:
        lines = [l for l in text.splitlines() if l.strip()]
        if tool == "check_data":
            m = re.search(r"check_data: (\d+) problem\(s\)", text)
            if m and int(m.group(1)) > 0:
                first = next((l for l in lines if not l.startswith("check_data:")), "")
                killed.append("check_data %s problem(s): %s" % (m.group(1), first[:160]))
            elif m and rc == 0:
                summary.append("check_data: 0 problem(s)")
            else:
                noresult.append("check_data exit %d without its count line: %s" % (rc, (lines[-1] if lines else "")[:120]))
            continue
        fails = re.findall(r"^FAIL: (test_\w+)", text, re.M)
        errs = re.findall(r"^ERROR: (test_\w+)", text, re.M)
        summ = next((l for l in reversed(lines) if l.startswith(("FAILED", "OK"))), "")
        own = [f for f in fails if spec.get("whole") or f in spec.get("methods", [])]
        if own and "failures=" in summ:
            killed.append("%s (%s)" % (summ, ", ".join(sorted(set(own)))))
        elif rc == 0 and summ.startswith("OK"):
            summary.append(summ)
        elif errs:
            noresult.append("%s - errors in %s: the lead reads it" % (summ or "exit %d" % rc, ", ".join(sorted(set(errs)))))
        elif fails:
            noresult.append("%s - failures only in other methods (%s)" % (summ, ", ".join(sorted(set(fails)))))
        else:
            noresult.append("exit %d, no unittest summary: %s" % (rc, (lines[-1] if lines else "")[:120]))
    if killed:
        return "KILLED", killed
    if noresult:
        return "NO RESULT", noresult
    return "SURVIVED", summary


def schema_cmd(spec):
    cmd = [sys.executable, "-I", "-X", "utf8", "Tools/test_data_schema.py"]
    if not spec.get("whole"):
        cmd += ["%s.%s" % (spec.get("classes", {}).get(m, "SchemaTests"), m) for m in spec["methods"]]
    return cmd


CHECK_CMD = [sys.executable, "-I", "-X", "utf8", "Tools/check_data.py"]

# ===================================================================================================== clocks and locks


class RealSync:
    virtual = False

    def __init__(self):
        self.cv = threading.Condition()
        self.t0 = time.monotonic()

    def now(self):
        return time.monotonic() - self.t0

    def sleep(self, s):
        time.sleep(s)

    def register(self):
        pass

    def unregister(self):
        pass

    def block(self, tok):
        while not tok[0]:
            self.cv.wait()

    def wake(self, tok):
        tok[0] = True
        self.cv.notify_all()

    def join(self, threads):
        for t in threads:
            t.join()


class VirtualSync:
    """A virtual clock: time moves only when every registered thread sleeps or waits on a lock of this module."""
    virtual = True

    def __init__(self):
        self.cv = threading.Condition()
        self.t = 0.0
        self.running = 0
        self.sleepers = []
        self.seq = 0

    def now(self):
        return self.t

    def register(self):
        with self.cv:
            self.running += 1

    def unregister(self):
        with self.cv:
            self.running -= 1
            self._advance()

    def sleep(self, s):
        with self.cv:
            tok = [False]
            self.seq += 1
            heapq.heappush(self.sleepers, (self.t + max(0.0, float(s)), self.seq, tok))
            self.block(tok)

    def block(self, tok):
        self.running -= 1
        self._advance()
        waited = 0
        while not tok[0]:
            self.cv.wait(timeout=1.0)
            if not tok[0]:
                waited += 1
                if waited > 120 and self.running <= 0 and not self.sleepers:
                    raise RuntimeError("virtual clock: every thread waits on a lock (deadlock)")

    def wake(self, tok):
        if not tok[0]:
            tok[0] = True
            self.running += 1
            self.cv.notify_all()

    def _advance(self):
        if self.running <= 0 and self.sleepers:
            self.t = max(self.t, self.sleepers[0][0])
            while self.sleepers and self.sleepers[0][0] <= self.t:
                _, _, tok = heapq.heappop(self.sleepers)
                self.wake(tok)

    def join(self, threads):
        with self.cv:
            self.running -= 1
            self._advance()
        for t in threads:
            t.join()
        with self.cv:
            self.running += 1


class FifoMutex:
    """Our side of the one UBT mutex (UnrealBuildTool.cs:441-447): builds take turns in request order."""

    def __init__(self, sync):
        self.s = sync
        self.busy = False
        self.q = collections.deque()
        self.held = 0.0
        self._since = 0.0

    def acquire(self):
        with self.s.cv:
            if not self.busy and not self.q:
                self.busy = True
            else:
                tok = [False]
                self.q.append(tok)
                self.s.block(tok)
            self._since = self.s.now()

    def release(self):
        with self.s.cv:
            self.held += self.s.now() - self._since
            if self.q:
                self.s.wake(self.q.popleft())
            else:
                self.busy = False


class Gate:
    """Shared for every build and editor launch; exclusive for a run 'alone' (the map-reload reruns). Writer first."""

    def __init__(self, sync):
        self.s = sync
        self.shared = 0
        self.excl = False
        self.q = collections.deque()

    def acquire_shared(self):
        with self.s.cv:
            if not self.excl and not any(k == "X" for k, _ in self.q):
                self.shared += 1
                return
            tok = [False]
            self.q.append(("S", tok))
            self.s.block(tok)

    def release_shared(self):
        with self.s.cv:
            self.shared -= 1
            self._dispatch()

    def acquire_excl(self):
        with self.s.cv:
            if not self.excl and self.shared == 0 and not self.q:
                self.excl = True
                return
            tok = [False]
            self.q.append(("X", tok))
            self.s.block(tok)

    def release_excl(self):
        with self.s.cv:
            self.excl = False
            self._dispatch()

    def _dispatch(self):
        while self.q and not self.excl:
            kind, tok = self.q[0]
            if kind == "X":
                if self.shared == 0:
                    self.q.popleft()
                    self.excl = True
                    self.s.wake(tok)
                return
            self.q.popleft()
            self.shared += 1
            self.s.wake(tok)

# ===================================================================================================== environments


def free_memory_gb():
    """bundleA.py:135-144."""
    class MEMORYSTATUSEX(ctypes.Structure):
        _fields_ = [("dwLength", ctypes.c_ulong), ("dwMemoryLoad", ctypes.c_ulong), ("ullTotalPhys", ctypes.c_ulonglong),
                    ("ullAvailPhys", ctypes.c_ulonglong), ("ullTotalPageFile", ctypes.c_ulonglong), ("ullAvailPageFile", ctypes.c_ulonglong),
                    ("ullTotalVirtual", ctypes.c_ulonglong), ("ullAvailVirtual", ctypes.c_ulonglong), ("ullAvailExtendedVirtual", ctypes.c_ulonglong)]
    try:
        stat = MEMORYSTATUSEX()
        stat.dwLength = ctypes.sizeof(MEMORYSTATUSEX)
        ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(stat))
        return stat.ullAvailPhys / 1024.0 ** 3, stat.ullTotalPhys / 1024.0 ** 3
    except Exception:  # noqa: BLE001
        return None, None


def working_set_gb(pid):
    """The process's peak working set so far (PROCESS_MEMORY_COUNTERS.PeakWorkingSetSize), in GB; 0 if unreadable."""
    class PMC(ctypes.Structure):
        _fields_ = [("cb", ctypes.c_ulong), ("PageFaultCount", ctypes.c_ulong), ("PeakWorkingSetSize", ctypes.c_size_t),
                    ("WorkingSetSize", ctypes.c_size_t), ("QuotaPeakPagedPoolUsage", ctypes.c_size_t),
                    ("QuotaPagedPoolUsage", ctypes.c_size_t), ("QuotaPeakNonPagedPoolUsage", ctypes.c_size_t),
                    ("QuotaNonPagedPoolUsage", ctypes.c_size_t), ("PagefileUsage", ctypes.c_size_t), ("PeakPagefileUsage", ctypes.c_size_t)]
    try:
        k32 = ctypes.windll.kernel32
        k32.OpenProcess.restype = ctypes.c_void_p
        h = k32.OpenProcess(0x1000 | 0x0010, False, pid)
        if not h:
            return 0.0
        try:
            c = PMC()
            c.cb = ctypes.sizeof(PMC)
            if not k32.K32GetProcessMemoryInfo(ctypes.c_void_p(h), ctypes.byref(c), c.cb):
                return 0.0
            return max(c.PeakWorkingSetSize, c.WorkingSetSize) / 1024.0 ** 3
        finally:
            k32.CloseHandle(ctypes.c_void_p(h))
    except Exception:  # noqa: BLE001
        return 0.0


def busy_processes():
    """bundleA.py:108-133: UnrealEditor / UBT processes (and dotnet running UnrealBuildTool)."""
    try:
        out = subprocess.run(["tasklist", "/FO", "CSV", "/NH"], capture_output=True, text=True, errors="replace", timeout=60).stdout
    except Exception as e:  # noqa: BLE001
        return ["tasklist failed: %s" % e]
    found, dotnet = [], False
    for row in csv.reader(io.StringIO(out)):
        if not row:
            continue
        image = row[0].strip().lower()
        if image.startswith("unrealeditor") or image.startswith("unrealbuildtool") or image == "ubt.exe":
            found.append("%s (pid %s)" % (row[0], row[1] if len(row) > 1 else "?"))
        elif image == "dotnet.exe":
            dotnet = True
    if dotnet:
        ps = ["powershell", "-NoProfile", "-NonInteractive", "-Command",
              "Get-CimInstance Win32_Process -Filter \"Name='dotnet.exe'\" | ForEach-Object { [string]$_.ProcessId + ' ' + $_.CommandLine }"]
        try:
            lines = subprocess.run(ps, capture_output=True, text=True, errors="replace", timeout=60).stdout.splitlines()
            found += ["dotnet: " + l.strip() for l in lines if "unrealbuildtool" in l.lower()]
        except Exception as e:  # noqa: BLE001
            found.append("dotnet.exe running and its command line unreadable (%s) - refusing" % e)
    return found


def fwd(p):
    return p.replace("\\", "/")


class RealEnv:
    """The real commands. Every lane git call gets GITC from the caller."""
    dry = False

    def __init__(self, sync):
        self.sync = sync

    # --- files
    def read(self, path):
        return open(path, "rb").read() if os.path.isfile(path) else None

    def write(self, path, data):
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "wb") as f:
            f.write(data)

    def exists(self, path):
        return os.path.exists(path)

    def makedirs(self, path):
        os.makedirs(path, exist_ok=True)

    def copytree(self, src, dst):
        shutil.copytree(src, dst, ignore=shutil.ignore_patterns("__pycache__", "*.pyc"))

    def rmtree(self, path):
        shutil.rmtree(path)

    def files_under(self, root, rel, ext):
        res = {}
        base = os.path.join(root, rel)
        for d, _, fs in os.walk(base):
            for f in fs:
                if f.lower().endswith(ext):
                    p = os.path.join(d, f)
                    res[os.path.relpath(p, root).replace("\\", "/")] = p
        return res

    def size(self, path):
        return os.path.getsize(path)

    # --- commands
    def git(self, cwd, args, timeout=1800):
        r = subprocess.run(["git", "-C", cwd] + args, capture_output=True, timeout=timeout)
        return r.returncode, (r.stdout + r.stderr).decode("utf-8", "replace")

    def build(self, lane, flags, hint=None):
        cmd = [BUILD, "Cosmic_AscensionEditor", "Win64", "Development", "-Project=" + lane.project] + flags
        r = subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8", errors="replace", env=lane.env, cwd=lane.root)
        out = r.stdout + r.stderr
        errs = [l.strip()[:300] for l in out.splitlines() if re.search(r"\berror\b|\bError:", l)]
        return "Result: Succeeded" in r.stdout, errs[:20]

    def editor(self, lane, args, log, hint=None, timeout=3600):
        for old in (log, log[:-4] + "_2.log"):
            if os.path.exists(old):
                os.remove(old)
        os.makedirs(os.path.dirname(log), exist_ok=True)
        cmd = [EDITOR, lane.project] + args + ["-AbsLog=" + log]
        with open(log + ".stdout.txt", "wb") as so:
            p = subprocess.Popen(cmd, stdout=so, stderr=subprocess.STDOUT, cwd=lane.root, env=lane.env)
            peak, t0 = 0.0, time.monotonic()
            while True:
                try:
                    rc = p.wait(timeout=5)
                    break
                except subprocess.TimeoutExpired:
                    peak = max(peak, working_set_gb(p.pid))
                    if time.monotonic() - t0 > timeout:
                        p.kill()  # our own child only
                        rc = p.wait()
                        break
        text = open(log, encoding="utf-8", errors="replace").read() if os.path.exists(log) else ""
        so_text = open(log + ".stdout.txt", encoding="utf-8", errors="replace").read()
        return rc, text, so_text, peak

    def run_cmd(self, cmd, cwd, env=None, timeout=3600):
        r = subprocess.run(cmd, cwd=cwd, env=env, capture_output=True, text=True, encoding="utf-8", errors="replace", timeout=timeout)
        return r.returncode, r.stdout + r.stderr

    def free_memory(self):
        return free_memory_gb()[0]

    def busy(self):
        return busy_processes()


class SimEnv:
    """Every external command mocked. Files live in a dict; time passes on the sync's virtual clock with section 5.4's
    costs. Knobs (self.opts) script the behaviours the self-test needs."""
    dry = True
    COST = {"first": 390, "clean": 15, "light": 12, "heavy": 70, "probe": 60, "notapplied": 0,
            "test_light": 9, "test_heavy": 15, "test_asset": 30, "test_probe": 30, "baseline": 300, "make_data": 50,
            "git": 0.5, "worktree": 40, "copytree": 5, "cmdlet": 120, "bundleB": 240, "py_method": 3, "py_whole": 88,
            "py_check": 6}

    def __init__(self, sync, items, main_root, phasea_files, opts=None):
        self.sync = sync
        self.items = {it["tag"]: it for it in items}
        self.main_root = main_root
        self.phasea = list(phasea_files)
        self.o = dict(opts or {})
        self.fs = {}
        self.calls = []
        self.lock = threading.Lock()
        self.mapped = set()
        self.free_seq = list(self.o.get("free_seq", []))
        self.cost = dict(self.COST)
        self.cost.update(self.o.get("cost", {}))

    def _k(self, p):
        return os.path.normcase(os.path.abspath(p))

    def _head(self, rel):
        return ("head of %s\nline two\nline three\n" % rel).encode()

    def _main(self, rel):
        return self._head(rel) + b"phase A\n" if rel in self.phasea else self._head(rel)

    def _note(self, *a):
        with self.lock:
            self.calls.append(a)

    # --- files
    def _lane_rel(self, path):
        k = self._k(path)
        for root in self.o.get("roots", []):
            r = self._k(root)
            if k.startswith(r + os.sep):
                return root, os.path.relpath(path, root).replace("\\", "/")
        return None, None

    def read(self, path):
        k = self._k(path)
        with self.lock:
            if k in self.fs:
                return self.fs[k]
        root, rel = self._lane_rel(path)
        if root is None:
            return None
        if self._k(root) == self._k(self.main_root):
            return self._main(rel)
        return self._head(rel)

    def write(self, path, data):
        with self.lock:
            self.fs[self._k(path)] = data

    def exists(self, path):
        k = self._k(path)
        return k in self.fs or any(f.startswith(k + os.sep) for f in self.fs) or k in [self._k(r) for r in self.o.get("existing", [])]

    def makedirs(self, path):
        self._note("makedirs", path)

    def copytree(self, src, dst):
        self._note("copytree", src, dst)
        self.sync.sleep(self.cost["copytree"])

    def rmtree(self, path):
        self._note("rmtree", path)

    def files_under(self, root, rel, ext):
        names = ["Content/CA/Data/DT_LogTemplates.uasset", "Content/CA/Data/DT_Hazards.uasset"] if ext == ".uasset" else []
        return {n: os.path.join(root, n.replace("/", os.sep)) for n in names if n.startswith(rel)}

    def size(self, path):
        data = self.read(path)
        if data is not None and self._k(path) in self.fs:
            return len(data)
        return 1 if self.o.get("lfs_pointers") else 90000

    # --- commands
    def git(self, cwd, args, timeout=None):
        self._note("git", cwd, tuple(args))
        rest = list(args)
        while rest and rest[0] == "-c":
            rest = rest[2:]
        sub = rest[0] if rest else ""
        if sub == "worktree":
            self.sync.sleep(self.cost["worktree"])
            return 0, "Preparing worktree"
        self.sync.sleep(self.cost["git"])
        if sub == "rev-parse":
            return 0, "0123456789abcdef0123456789abcdef01234567\n"
        if sub == "status":
            return 0, self._status(cwd)
        if sub == "apply":
            rev = "-R" in rest
            patch = rest[-1]
            if os.path.basename(patch).endswith(".diff"):
                for rel in self.phasea:
                    self.write(os.path.join(cwd, rel), self._main(rel))
                return 0, ""
            tag = os.path.basename(patch)[:-6]
            it = self.items.get(tag)
            if it is None:
                return 1, "error: can't open patch '%s'" % patch
            if not rev and (it.get("apply") == "FAIL" or tag in self.o.get("apply_fail", ())):
                return 1, "error: patch failed: %s:12\nerror: %s: patch does not apply" % (it["targets"][0] if it["targets"] else "?", tag)
            mark = ("MUTANT %s\n" % tag).encode()
            for rel in it["targets"]:
                p = os.path.join(cwd, rel)
                cur = self.read(p) or b""
                if rev:
                    new = cur.replace(b"\r\n", b"\n").replace(mark, b"")
                    if tag in self.o.get("crlf_on_revert", ()):
                        new = new.replace(b"\n", b"\r\n")
                else:
                    if mark in cur:
                        return 1, "error: patch failed (already applied)"
                    new = cur + mark
                self.write(p, new)
            return 0, ""
        return 0, ""

    def _status(self, cwd):
        out = []
        for rel in sorted(set(self.phasea) | {t for it in self.items.values() for t in it["targets"]}):
            if not rel.startswith(("Source/", "Tools/", "Config/")):
                continue
            k = self._k(os.path.join(cwd, rel))
            with self.lock:
                cur = self.fs.get(k)
            if cur is not None and cur != self._head(rel):
                out.append(" M " + rel)
        return "\n".join(out) + ("\n" if out else "")

    def build(self, lane, flags, hint=None):
        self._note("build", lane.name, tuple(flags), hint)
        kind = hint if hint in self.cost else "light"
        self.sync.sleep(self.cost[kind])
        if "-DisableUnity" in flags and self.o.get("fail_nonunity"):
            return False, ["CosmicChronicleWidget.cpp(12): error C2065: 'FCosmicFoo': undeclared identifier (simulated)"]
        if getattr(lane, "current", None) in self.o.get("build_fail", ()):
            return False, ["error C2143: syntax error (simulated)"]
        return True, []

    def _ue_log(self, entries, complete=True):
        lines = ["LogInit: Command Line: (simulated)"]
        for path, res, errs in entries:
            lvl = "Display" if res == "Success" else "Error"
            lines.append("[2026.11.10-00.00.00:000][  1]LogAutomationController: %s: Test Completed. Result={%s} Name={%s} Path={%s}"
                         % (lvl, res, path.split(".")[-1], path))
            for e in errs:
                lines.append("[2026.11.10-00.00.00:000][  1]LogAutomationController: Error: %s" % e)
        if complete:
            lines.append("LogAutomationCommandLine: Display: **** TEST COMPLETE. EXIT CODE: %d ****" %
                         (0 if all(r == "Success" for _, r, _ in entries) else -1))
        else:
            lines.append("[2026.11.10-00.00.00:000][  1]LogWindows: Error: === Critical error: === (simulated)")
        return "\n".join(lines) + "\n"

    def sim_verdict(self, tag):
        v = self.o.get("verdicts", {}).get(tag)
        if v:
            return v
        h = zlib.crc32(tag.encode()) % 100
        return "SURVIVED" if h < 2 else ("MAPRELOAD" if h == 2 else "KILLED")

    def editor(self, lane, args, log, hint=None, timeout=None):
        self._note("editor", lane.name, tuple(args), hint)
        ex = next((a for a in args if a.startswith("-ExecCmds=")), "")
        m = re.search(r"RunTests (\S+);", ex)
        filt = m.group(1) if m else ""
        if any(a.startswith("-run=pythonscript") for a in args):
            self.sync.sleep(self.cost["make_data"])
            tag = getattr(lane, "current", None)
            refused = tag in self.o.get("refuse", ()) and not getattr(lane, "reverting", False)
            if refused:
                text = ("LogPython: [make_data_tables] DT_LogTemplates: 3 rows written and verified\n"
                        "LogPython: Error: Traceback (most recent call last):\n"
                        "LogPython: Error: RuntimeError: the sources do not hold Tools/data/schema.json (1):\n")
            else:
                text = ("LogPython: [make_data_tables] DT_LogTemplates: 3 rows written and verified\n"
                        "LogPython: [make_data_tables] 2 assets saved\nLogPython: [make_data_tables] done\n")
            self.write(log, text.encode())
            return (1 if refused else 0), text, "", 1.5
        if any(a.startswith("-run=CosmicBalance") for a in args):
            self.sync.sleep(self.cost["cmdlet"])
            text = "\n".join(self.o.get("cmdlet_lines", {}).get(getattr(lane, "current", None), ["CosmicBalance: summary"])) + "\n"
            return self.o.get("cmdlet_rc", {}).get(getattr(lane, "current", None), 0), text, "", 2.0
        peak = self.o.get("peak", 1.6)
        if filt == "Cosmic_Ascension":
            self.sync.sleep(self.cost["baseline"])
            bad = self.o.get("baseline_fail", {}).get(lane.name)
            entries = [("Cosmic_Ascension.Data.Integrity", "Success", []), ("Cosmic_Ascension.UI.Chronicle", "Success", [])]
            if bad == "real":
                entries.append(("Cosmic_Ascension.Sim.Determinism", "Fail", ["Expected 'hash' to be 1, but it was 2. (simulated)"]))
            elif bad == "map" and (lane.name, "baseline") not in self.mapped:
                self.mapped.add((lane.name, "baseline"))
                entries.append(("Cosmic_Ascension.UI.ContinueEnded", "Fail", [MAP_RELOAD]))
            n = int(self.o.get("tests") or 0)
            entries += [("Cosmic_Ascension.Sim.Filler%d" % i, "Success", []) for i in range(max(0, n - len(entries)))]
            short = self.o.get("baseline_short", {}).get(lane.name)
            if short is not None:
                entries = entries[:short]
            complete = bad != "crash"
            if not complete:
                entries = entries[:max(1, len(entries) * 2 // 5)]
            text = self._ue_log(entries, complete)
            self.write(log, text.encode())
            return 0, text, "", peak
        tag = getattr(lane, "current", None)
        it = self.items.get(tag, {})
        self.sync.sleep(self.cost["test_" + (it.get("cost") if it.get("cost") in ("light", "heavy", "asset", "probe") else "light")])
        paths = filt.split("+") if filt else []
        v = self.sim_verdict(tag) if tag else "SURVIVED"
        entries = []
        for i, p in enumerate(paths):
            if v == "KILLED" and i == 0:
                entries.append((p, "Fail", ["Expected 'something' to be true. (simulated %s)" % tag]))
            elif v == "MAPRELOAD" and i == 0 and (tag, "map") not in self.mapped:
                self.mapped.add((tag, "map"))
                entries.append((p, "Fail", [MAP_RELOAD]))
            elif v == "MAPRELOAD" and i == 0:
                entries.append((p, "Fail", ["Expected 'after the reload' to be true. (simulated %s)" % tag]))
            elif v == "NORESULT":
                pass
            else:
                entries.append((p, "Success", []))
        text = self._ue_log(entries)
        self.write(log, text.encode())
        return 0, text, "", peak

    def run_cmd(self, cmd, cwd, env=None, timeout=None):
        self._note("run", tuple(cmd), cwd)
        joined = " ".join(cmd)
        if "bundleB.py" in joined:
            self.sync.sleep(self.cost["bundleB"])
            mut = "mut" in os.path.basename(cmd[cmd.index("--folder") + 1]) if "--folder" in cmd else False
            lines = ["bundle B: 2 jobs, 1 workers, folder %s (simulated)" % cmd[cmd.index("--folder") + 1] if "--folder" in cmd else "",
                     "bundle B: FAIL (simulated)"]
            if mut and self.o.get("witness_kills", True):
                lines.append("wanderer s1: witness first at 120 before the wanderer left at 300")
            return 1, "\n".join(lines) + "\n"
        if "check_data.py" in joined:
            self.sync.sleep(self.cost["py_check"])
            tag = self._applied_in(cwd)
            if tag and self.sim_verdict(tag) == "KILLED" and (self.items[tag].get("py") or {}).get("check_data"):
                return 1, "DT_LogTemplates.log_x: rule 8 (simulated)\ncheck_data: 1 problem(s)\n"
            return 0, "check_data: 0 problem(s)\n"
        if "test_data_schema.py" in joined:
            methods = [c.split(".")[-1] for c in cmd[5:]]
            self.sync.sleep(self.cost["py_whole"] if not methods else self.cost["py_method"] * len(methods))
            tag = self._applied_in(cwd)
            spec = (self.items[tag].get("py") or {}) if tag else {}
            if tag and self.sim_verdict(tag) == "KILLED" and (spec.get("methods") or spec.get("whole")):
                m = (spec.get("methods") or ["test_whole_file"])[0]
                return 1, "FAIL: %s (__main__.SchemaTests.%s)\n----\nRan %d tests\n\nFAILED (failures=1)\n" % (m, m, max(1, len(methods)))
            if tag and self.sim_verdict(tag) == "NORESULT":
                return 1, "ERROR: %s (__main__.SchemaTests)\nRan 1 test\n\nFAILED (errors=1)\n" % (methods[0] if methods else "test_x")
            return 0, "Ran %d tests\n\nOK\n" % max(1, len(methods))
        self.sync.sleep(1)
        return 0, ""

    def _applied_in(self, cwd):
        k = self._k(cwd) + os.sep
        with self.lock:
            for key, data in self.fs.items():
                if key.startswith(k):
                    m = re.search(rb"MUTANT (\S+)\n", data)
                    if m:
                        return m.group(1).decode()
        return None

    def free_memory(self):
        with self.lock:
            if self.free_seq:
                return self.free_seq.pop(0)
        return self.o.get("free", 12.0)

    def busy(self):
        return list(self.o.get("busy", []))

# ===================================================================================================== the run


class Lane:
    def __init__(self, run, n):
        self.run = run
        self.n = n
        self.name = "l%d" % n
        self.root = os.path.join(run.base, self.name)
        self.tmp = os.path.join(run.base, "tmp", self.name)
        self.project = os.path.join(self.root, "Cosmic_Ascension.uproject")
        self.env = dict(os.environ, TMP=self.tmp, TEMP=self.tmp)
        self.alive = True
        self.stop_reason = None
        self.current = None
        self.reverting = False
        self.dirty_bin = False
        self.busy = 0.0
        self.items = 0
        self.mem_waits = 0
        self.mem_wait_s = 0.0
        self.peak = 0.0
        self.asset_hashes = {}
        self.end = {}

    def log(self, msg):
        self.run.say("%s %s" % (self.name, msg))

    def git(self, *args):
        return self.run.env.git(self.root, GITC + list(args))

    def stop(self, reason):
        self.alive = False
        self.stop_reason = reason
        self.log("STOPPED: " + reason)

    # --- the memory gate and the builds
    def wait_memory(self, what):
        thr = self.run.threshold()
        t0 = self.run.sync.now()
        first = True
        while True:
            free = self.run.env.free_memory()
            if free is None or free >= thr:
                break
            if first:
                self.log("waits for memory before %s: free %.1f GB < %.1f GB" % (what, free, thr))
                first = False
            self.run.sync.sleep(5)
        if not first:
            waited = self.run.sync.now() - t0
            self.mem_waits += 1
            self.mem_wait_s += waited
            self.run.mem_log.append((self.name, what, round(waited, 1)))
            self.log("memory free again after %.0f s (%s)" % (waited, what))

    def build(self, flags, hint):
        r = self.run
        r.gate.acquire_shared()
        try:
            r.ubt.acquire()
            try:
                self.wait_memory("build")
                ok, errs = r.env.build(self, flags, hint)
            finally:
                r.ubt.release()
        finally:
            r.gate.release_shared()
        if ok:
            self.dirty_bin = hint not in ("first", "clean")
        return ok, errs

    def editor(self, args, log, hint=None, alone=False):
        r = self.run
        (r.gate.acquire_excl if alone else r.gate.acquire_shared)()
        try:
            self.wait_memory("editor")
            rc, text, so, peak = r.env.editor(self, args, log, hint)
        finally:
            (r.gate.release_excl if alone else r.gate.release_shared)()
        self.peak = max(self.peak, peak)
        return rc, text, so, peak

    def test_args(self, filt):
        return ["-game", "-nullrhi", "-nosound", "-unattended", "-nosplash", "-stdout", "-FullStdOutLogOutput",
                "-ExecCmds=Automation RunTests %s; Quit" % filt, "-TestExit=Automation Test Queue Empty"]

    def logpath(self, name):
        return os.path.join(self.root, "Saved", "Logs", name + ".log")

    def run_tests(self, filt, logname, hint=None, alone=False):
        rc, text, so, peak = self.editor(self.test_args(filt), self.logpath(logname), hint, alone)
        return parse_test_log(text), peak

    def make_data(self, logname):
        args = ["-run=pythonscript", "-Script=%s/Tools/make_data_tables.py" % fwd(self.root), "-unattended", "-nosplash",
                "-stdout", "-FullStdOutLogOutput"]
        rc, text, so, peak = self.editor(args, self.logpath(logname), "make_data")
        return make_data_outcome(rc, text, so)

    def clean_build_if_dirty(self):
        if self.dirty_bin:
            ok, errs = self.build(self.run.flags, "clean")
            if not ok:
                return False
            self.dirty_bin = False
        return True

    # --- section 5.3 steps 1-2: the tree
    def setup_tree(self):
        r = self.run
        if r.reuse:
            self.log("reusing the existing worktree")
        else:
            if r.env.exists(self.root):
                return self.stop("%s exists - remove it (git worktree remove %s) or pass --reuse-lanes" % (self.root, self.root))
            rc, out = r.env.git(r.root, GITC + ["worktree", "add", "--detach", self.root, r.head])
            if rc != 0:
                return self.stop("worktree add failed: " + out.strip()[:300])
            small = [p for p, full in r.env.files_under(self.root, "Content", ".uasset").items() if r.env.size(full) <= 1024]
            if small:
                return self.stop("LFS pointers, not assets (<= 1 KB): " + ", ".join(small[:5]))
            rc, out = self.git("apply", r.diff)
            if rc != 0:
                return self.stop("phaseA.diff does not apply: " + out.strip()[:300])
        r.env.makedirs(self.tmp)
        listed = self.status_files()
        if listed != set(r.phasea_files):
            return self.stop("status lists %s, phaseA.diff names %s" % (sorted(listed), sorted(r.phasea_files)))
        bad = [f for f in r.phasea_files if r.env.read(os.path.join(self.root, f)) != r.env.read(os.path.join(r.root, f))]
        if bad:
            return self.stop("Phase A files differ from the main tree: " + ", ".join(bad))
        self.log("tree ready (%d Phase A files equal to the main tree)" % len(r.phasea_files))

    def status_files(self):
        rc, out = self.git("status", "--porcelain", "--", "Source", "Tools", "Config")
        files = set()
        for line in out.splitlines():
            if len(line) > 3:
                p = line[3:].strip()
                if " -> " in p:
                    p = p.split(" -> ", 1)[1]
                files.add(p.strip('"'))
        return files

    # --- section 5.3 steps 5-6
    def assets_and_baseline(self):
        r = self.run
        if r.assets_dir and r.env.exists(r.assets_dir):
            for name in r.asset_names:
                data = r.env.read(os.path.join(r.assets_dir, name))
                r.env.write(os.path.join(self.root, "Content", "CA", "Data", name), data)
                if r.env.read(os.path.join(self.root, "Content", "CA", "Data", name)) != data:
                    return self.stop("asset copy differs: " + name)
            self.log("assets copied: " + ", ".join(r.asset_names))
        else:
            self.current = None
            state, detail = self.make_data("MakeData_setup")
            if state != "written":
                return self.stop("make_data_tables did not end with '%s' at setup (%s: %s)" % (MAKE_DATA_DONE, state, detail))
            self.log("assets rebuilt by make_data_tables (no phaseA_assets)")
        t0 = r.sync.now()
        parsed, peak = self.run_tests("Cosmic_Ascension", "Baseline_" + self.name, "baseline")
        v, rerun, errs = verdict_of(parsed)
        if v == "NO RESULT" and rerun:
            self.log("baseline: map reload only in %s - rerun alone" % ", ".join(rerun))
            parsed2, _ = self.run_tests("+".join(rerun), "Baseline_%s_rerun" % self.name, "baseline", alone=True)
            v2, rerun2, errs2 = verdict_of(parsed2)
            if v2 == "SURVIVED":
                v, errs = "SURVIVED", []
            else:
                v, errs = "KILLED", errs2 or ["the map reload failed again"]
        want = int(r.plan.get("tests") or 0)
        got = len(parsed["results"])
        if v == "SURVIVED" and want and got < want:
            # Section 5.3 step 6: every test must pass; a queue that ended short of the tree's tests is not green.
            # The plan counts IMPLEMENT_*AUTOMATION_TEST in Source; every test there carries ClientContext (-game).
            r.short_baselines.append("%s baseline ran %d of the plan's %d tests" % (self.name, got, want))
            v, errs = "NO RESULT", ["%d of the plan's %d tests ran (a test missing from -game, or the queue cut short)" % (got, want)]
        r.peaks.append(peak)
        self.busy += r.sync.now() - t0
        if v != "SURVIVED":
            return self.stop("baseline run not green (%s): %s" % ("failures" if v == "KILLED" else v, " | ".join(errs[:3])))
        self.log("baseline: %d tests passed, editor peak %.2f GB -> memory gate %.2f GB" %
                 (len(parsed["results"]), peak, r.threshold()))
        self.asset_hashes = self.hash_assets()

    def hash_assets(self):
        r = self.run
        out = {}
        for rel, full in sorted(r.env.files_under(self.root, "Content/CA/Data", ".uasset").items()):
            data = r.env.read(full)
            out[rel] = hashlib.sha1(data).hexdigest() if data is not None else None
        return out

    # --- section 5.5: one item
    def run_item(self, it):
        r = self.run
        tag = it["tag"]
        t0 = r.sync.now()
        self.current = tag
        rec = {"phase": "lane", "tag": tag, "folder": it["folder"], "kind": it["kind"], "sets": it["sets"],
               "lane": self.name, "filters": it["filters"], "results": [], "errors": [], "notes": []}
        patch = os.path.join(r.root, it["path"].replace("/", os.sep))
        snaps = {t: r.env.read(os.path.join(self.root, t)) for t in it["targets"]}
        fatal = None
        rc, out = self.git("apply", patch)
        if rc != 0:
            rec["verdict"] = "NOT APPLIED"
            rec["errors"] = [l for l in out.splitlines() if l.strip()][:2]
        else:
            try:
                if it["kind"] == "asset":
                    if not self.clean_build_if_dirty():
                        fatal = "clean build before an asset item failed"
                        rec["verdict"] = "NO RESULT"
                    else:
                        state, detail = self.make_data("MutationData_" + tag)
                        if state == "refused":
                            rec["verdict"] = "KILLED"
                            rec["notes"].append("assets: NOT written (checker stopped)")
                            rec["errors"] = ["make_data_tables refused the mutant (counts as KILLED, docs/40:40)", detail]
                        elif state != "written":
                            rec["verdict"] = "NO RESULT"
                            rec["notes"].append("assets: NOT written (no verdict from make_data_tables)")
                            rec["errors"] = ["make_data gave no verdict (%s)" % detail]
                        else:
                            rec["notes"].append("assets: written")
                            self.judge(it, rec)
                else:
                    ok, errs = self.build(r.flags, it["cost"])
                    if not ok:
                        rec["verdict"] = "BUILD FAILED"
                        rec["errors"] = errs[:4]
                        self.dirty_bin = True
                    else:
                        self.judge(it, rec)
            finally:
                rc2, out2 = self.git("apply", "-R", patch)
                if rc2 != 0:
                    rec["notes"].append("apply -R failed: " + out2.strip()[:200])
                for t, data in snaps.items():
                    p = os.path.join(self.root, t)
                    cur = r.env.read(p)
                    if cur != data:
                        if (cur or b"").replace(b"\r\n", b"\n") != (data or b"").replace(b"\r\n", b"\n"):
                            rec["notes"].append("REVERT LEFT CONTENT in %s - snapshot restored" % t)
                        else:
                            rec["notes"].append("restored the snapshot bytes of %s (line endings)" % t)
                        r.env.write(p, data)
                if it["kind"] == "asset":
                    self.reverting = True
                    try:
                        state, detail = self.make_data("MutationData_%s_revert" % tag)
                    finally:
                        self.reverting = False
                    rec["notes"].append("assets after revert: " + ("written" if state == "written" else "NOT written (%s: %s)" % (state, detail)))
                    if state != "written":
                        fatal = "assets not rebuilt after the revert of %s (%s: %s)" % (tag, state, detail)
        rec["secs"] = round(r.sync.now() - t0, 1)
        self.busy += r.sync.now() - t0
        self.items += 1
        self.current = None
        r.record(rec)
        line = "%s %s %s %s %.0fs" % (self.name, tag, rec["verdict"], "+".join(f.replace("Cosmic_Ascension.", "") for f in it["filters"]), rec["secs"])
        r.say(line + "".join("\n     " + e for e in (rec["errors"][:4] + [n for n in rec["notes"] if not n.startswith("assets")])))
        if fatal:
            self.stop(fatal)

    def judge(self, it, rec):
        filt = "+".join(it["filters"])
        parsed, _ = self.run_tests(filt, "Mutation_" + it["tag"], it["cost"])
        v, rerun, errs = verdict_of(parsed)
        rec["results"] = [[n, res] for res, n, p in parsed["results"]]
        if v == "NO RESULT" and rerun:
            rec["notes"].append("map reload only - rerun alone: " + ", ".join(rerun))
            parsed2, _ = self.run_tests("+".join(rerun), "Mutation_%s_alone" % it["tag"], it["cost"], alone=True)
            v2, rerun2, errs2 = verdict_of(parsed2)
            rec["results"] += [[n, res] for res, n, p in parsed2["results"]]
            v, errs = (v2, errs2) if v2 in ("KILLED", "SURVIVED") else ("NO RESULT", errs2)
        rec["verdict"] = v
        rec["errors"] = errs[:4]

    # --- section 5.5: the end of the lane
    def end_checks(self):
        r = self.run
        ok, errs = self.build(r.flags, "clean")
        self.dirty_bin = not ok
        self.end["clean_build"] = "Result: Succeeded" if ok else "FAILED: " + " | ".join(errs[:3])
        listed = self.status_files()
        self.end["status"] = "equal to phaseA.diff's files" if listed == set(r.phasea_files) else \
            "DIFFERS: %s" % sorted(listed ^ set(r.phasea_files))
        differ = [f for f in r.phasea_files if r.env.read(os.path.join(self.root, f)) != r.env.read(os.path.join(r.root, f))]
        if differ:
            self.end["phaseA_bytes"] = "differ from the main tree: " + ", ".join(differ)
        now = self.hash_assets()
        if now == self.asset_hashes:
            self.end["assets"] = "bytes equal to the post-setup bytes"
        else:
            parsed, _ = self.run_tests("Cosmic_Ascension.Data.SourcesInSync+Cosmic_Ascension.Data.Integrity", "End_" + self.name, "light")
            v, _, e = verdict_of(parsed)
            self.end["assets"] = "bytes changed; Data.SourcesInSync and Data.Integrity " + ("pass" if v == "SURVIVED" else "FAIL: " + " | ".join(e[:2]))
        self.log("end: " + "; ".join("%s %s" % kv for kv in self.end.items()))


class Run:
    def __init__(self, a, plan, env, sync):
        self.a = a
        self.plan = plan
        self.env = env
        self.sync = sync
        self.root = a.root
        self.base = a.base
        self.out = a.out
        self.diff = os.path.abspath(a.phaseA_diff) if a.phaseA_diff else None
        self.assets_dir = a.assets
        self.reuse = a.reuse_lanes
        self.items = {it["tag"]: it for it in plan["items"]}
        self.ubt = FifoMutex(sync)
        self.gate = Gate(sync)
        self.flags = list(UNITY_OFF)
        self.gate_errors = []
        self.peaks = []
        self.mem_log = []
        self.short_baselines = []
        self.lines = []
        self.say_lock = threading.Lock()
        self.queue = collections.deque()
        self.qlock = threading.Lock()
        self.head = None
        self.lanes = []
        self.phasea_files = []
        self.asset_names = []
        self.record_path = os.path.join(self.out, "results.jsonl")
        self.only = None
        if a.only:
            self.only = set(l.strip() for l in open(a.only, encoding="utf-8") if l.strip())

    def say(self, msg):
        with self.say_lock:
            stamp = "[%6.0fs] " % self.sync.now()
            for i, l in enumerate(msg.split("\n")):
                self.lines.append(stamp + l if i == 0 else l)
                print((stamp + l) if i == 0 else l, flush=True)

    def record(self, rec):
        rec["t"] = round(self.sync.now(), 1)
        with self.say_lock:
            with open(self.record_path, "a", encoding="utf-8", newline="\n") as f:
                f.write(json.dumps(rec, ensure_ascii=False) + "\n")

    def threshold(self):
        return max(MIN_FREE_GB, (max(self.peaks) + 0.5) if self.peaks else MIN_FREE_GB)

    def wanted(self, it):
        return it["run"] and (self.only is None or it["tag"] in self.only)

    # --- preflight
    def preflight(self, need_lanes):
        busy = self.env.busy()
        if busy:
            raise SystemExit("lanes: refusing while these run: " + "; ".join(busy))
        if need_lanes:
            free = self.env.free_memory()
            if free is not None and free < TWO_LANES_BELOW_GB and self.a.lanes > 2:
                self.say("free memory %.1f GB < %.0f GB: two lanes instead of %d" % (free, TWO_LANES_BELOW_GB, self.a.lanes))
                self.a.lanes = 2
            self.say("free memory %s GB at the start; lanes %d" % ("?" if free is None else "%.1f" % free, self.a.lanes))
        diff_text = (self.env.read(self.diff) or b"").decode("utf-8", "replace") if self.diff else ""
        self.phasea_files = []
        for line in diff_text.replace("\r\n", "\n").split("\n"):
            if line.startswith("diff --git a/"):
                f = line.split(" b/", 1)[1].strip()
                if f not in self.phasea_files:
                    self.phasea_files.append(f)
        if need_lanes and not self.phasea_files:
            raise SystemExit("lanes: phaseA.diff names no file (%s)" % self.diff)
        if need_lanes and any(f.startswith("Tools/mutations/") for f in self.phasea_files):
            # Line-ending phantoms of tracked patch files would ride along into every lane: make the diff with
            # ':!Tools/mutations' (the new patches are untracked and applied from the main tree anyway).
            raise SystemExit("lanes: phaseA.diff names Tools/mutations files - remake it with ':!Tools/mutations'")
        if self.assets_dir and self.env.exists(self.assets_dir):
            self.asset_names = self.a.asset_names or sorted(
                n for n in (os.listdir(self.assets_dir) if not self.env.dry else ["DT_LogTemplates.uasset"]) if n.endswith(".uasset"))

    # --- section 5.6
    def run_pool(self):
        items = [it for it in self.plan["items"] if it["where"] == "pool" and self.wanted(it)]
        self.say("pool: %d Python items, %d workers" % (len(items), self.a.workers))
        if not items:
            return
        pool_root = os.path.join(self.base, "py")
        marker = os.path.join(pool_root, ".b6b12_pool")
        if self.env.exists(pool_root):
            if not self.env.exists(marker):
                raise SystemExit("lanes: %s exists and is not a pool this script made - refusing" % pool_root)
            self.env.rmtree(pool_root)
        copies = []
        for i in range(self.a.workers):
            c = os.path.join(pool_root, "c%d" % i)
            for d in ("Source", "Tools", "Config"):
                self.env.copytree(os.path.join(self.root, d), os.path.join(c, d))
            self.env.write(os.path.join(c, "CLAUDE.md"), self.env.read(os.path.join(self.root, "CLAUDE.md")) or b"")
            copies.append(c)
        self.env.write(marker, b"made by b6b12/lanes.py\n")
        q = collections.deque(sorted(items, key=lambda it: it["index"]))
        qlock = threading.Lock()
        problems = []

        def clean_run(c, when):
            res = []
            for tool, cmd in (("schema", schema_cmd({"whole": True})), ("check_data", CHECK_CMD)):
                rc, out = self.env.run_cmd(cmd, c)
                res.append("%s exit %d" % (tool, rc))
                if rc != 0:
                    problems.append("%s: %s clean run of %s failed (exit %d): %s" % (c, when, tool, rc, out.strip().splitlines()[-1:] if out.strip() else ""))
            return res

        def worker(c):
            try:
                self.say("pool %s clean start: %s" % (os.path.basename(c), ", ".join(clean_run(c, "start"))))
                if problems:
                    return
                while not problems:
                    with qlock:
                        if not q:
                            break
                        it = q.popleft()
                    self.pool_item(c, it)
                self.say("pool %s clean end: %s" % (os.path.basename(c), ", ".join(clean_run(c, "end"))))
            finally:
                self.sync.unregister()

        threads = []
        for c in copies:
            t = threading.Thread(target=worker, args=(c,), daemon=True)
            self.sync.register()
            t.start()
            threads.append(t)
        self.sync.join(threads)
        for p in problems:
            self.say("pool PROBLEM: " + p)
        if problems:
            raise SystemExit("lanes: the pool's clean runs failed - fix the copies before the lanes")

    def pool_item(self, c, it):
        t0 = self.sync.now()
        spec = it["py"]
        patch = os.path.join(self.root, it["path"].replace("/", os.sep))
        snaps = {t: self.env.read(os.path.join(c, t)) for t in it["targets"]}
        rec = {"phase": "pool", "tag": it["tag"], "folder": it["folder"], "kind": "python", "sets": it["sets"],
               "copy": os.path.basename(c), "methods": spec["methods"], "check_data": spec["check_data"], "notes": []}
        rc, out = self.env.git(c, GITC + ["apply", patch])
        if rc != 0:
            rec["verdict"], rec["summary"] = "NOT APPLIED", [l for l in out.splitlines() if l.strip()][:2]
        else:
            outs = []
            if spec["methods"] or spec["whole"]:
                r1, o1 = self.env.run_cmd(schema_cmd(spec), c)
                outs.append(("schema", r1, o1))
            if spec["check_data"]:
                r2, o2 = self.env.run_cmd(CHECK_CMD, c)
                outs.append(("check_data", r2, o2))
            rec["verdict"], rec["summary"] = py_verdict(spec, outs)
            rc2, out2 = self.env.git(c, GITC + ["apply", "-R", patch])
            if rc2 != 0:
                rec["notes"].append("apply -R failed: " + out2.strip()[:200])
        for t, data in snaps.items():
            p = os.path.join(c, t)
            if self.env.read(p) != data:
                rec["notes"].append("restored the snapshot bytes of " + t)
                self.env.write(p, data)
        rec["secs"] = round(self.sync.now() - t0, 1)
        self.record(rec)
        what = ",".join(spec["methods"]) + (" check_data" if spec["check_data"] else "") + (" whole" if spec["whole"] else "")
        self.say("pool %s %s %s [%s] %.0fs | %s" % (os.path.basename(c), it["tag"], rec["verdict"], what.strip(), rec["secs"], " | ".join(rec["summary"])[:200]))

    # --- section 5.3-5.5
    def run_lanes(self):
        groups = []
        for g in self.plan["groups"]:
            tags = [t for t in g["tags"] if self.wanted(self.items[t])]
            if tags:
                groups.append({"key": g["key"], "tags": tags, "est": g["est"]})
        self.queue = collections.deque(groups)
        self.say("lanes: %d groups, %d items; base %s" % (len(groups), sum(len(g["tags"]) for g in groups), self.base))
        if not groups:
            return
        rc, out = self.env.git(self.root, GITC + ["rev-parse", "HEAD"])
        self.head = out.strip().split()[0]
        self.lanes = [Lane(self, n) for n in range(1, self.a.lanes + 1)]
        for ln in self.lanes:
            ln.setup_tree()
        if not self.lanes[0].alive:
            raise SystemExit("lanes: lane 1 could not be set up: " + self.lanes[0].stop_reason)
        # The gate: lane 1 alone, non-unity first.
        l1 = self.lanes[0]
        if self.a.lane_flags:
            self.flags = self.a.lane_flags.split()
            ok, errs = l1.build(self.flags, "first")
        else:
            ok, errs = l1.build(UNITY_OFF, "first")
            if not ok:
                self.gate_errors = errs
                self.say("gate: the -DisableUnity build of lane 1 failed - <LANE_FLAGS> = -WaitMutex for every lane; errors for N section 5:\n" +
                         "\n".join("     " + e for e in errs[:10]))
                self.flags = list(UNITY_ADAPTIVE)
                ok, errs = l1.build(self.flags, "first")
        if not ok:
            raise SystemExit("lanes: lane 1 does not build with %s: %s" % (" ".join(self.flags), " | ".join(errs[:5])))
        self.say("gate: lane 1 built; <LANE_FLAGS> = %s" % " ".join(self.flags))
        threads = []
        for ln in self.lanes:
            if not ln.alive:
                continue
            t = threading.Thread(target=self.lane_main, args=(ln,), daemon=True)
            self.sync.register()
            t.start()
            threads.append(t)
        self.sync.join(threads)
        left = [t for g in self.queue for t in g["tags"]]
        for tag in left:
            it = self.items[tag]
            self.record({"phase": "lane", "tag": tag, "folder": it["folder"], "kind": it["kind"], "sets": it["sets"],
                         "verdict": "NO RESULT", "errors": ["no lane left to run it"], "notes": [], "results": []})

    def lane_main(self, ln):
        try:
            t0 = self.sync.now()
            if ln.n != 1:
                ok, errs = ln.build(self.flags, "first")
                ln.busy += self.sync.now() - t0
                if not ok:
                    ln.stop("first build failed: " + " | ".join(errs[:3]))
                    return
            ln.assets_and_baseline()
            if not ln.alive:
                return
            while ln.alive:
                with self.qlock:
                    if not self.queue:
                        break
                    g = self.queue.popleft()
                ln.log("takes group %s (%d)" % (g["key"].split("/")[-1], len(g["tags"])))
                for i, tag in enumerate(g["tags"]):
                    if not ln.alive:
                        with self.qlock:
                            self.queue.appendleft({"key": g["key"], "tags": g["tags"][i:], "est": 0})
                        break
                    try:
                        ln.run_item(self.items[tag])
                    except Exception as e:  # noqa: BLE001
                        # An item that raised (a locked log, an OSError, a git timeout): the item is NO RESULT, the
                        # rest of its group goes back to the queue for another lane, and this lane stops - a patch
                        # may still sit in its tree, so the status is compared with phaseA.diff's files first.
                        self.item_exception(ln, g, i, tag, e)
                        break
            if ln.alive:
                ln.end_checks()
        except Exception as e:  # noqa: BLE001
            ln.stop("exception: %r" % e)
        finally:
            self.sync.unregister()

    def item_exception(self, ln, g, i, tag, e):
        it = self.items[tag]
        ln.current = None
        ln.reverting = False
        self.record({"phase": "lane", "tag": tag, "folder": it["folder"], "kind": it["kind"], "sets": it["sets"],
                     "lane": ln.name, "filters": it["filters"], "results": [], "verdict": "NO RESULT",
                     "errors": ["exception: %r" % (e,)], "notes": []})
        rest = g["tags"][i + 1:]
        if rest:
            with self.qlock:
                self.queue.appendleft({"key": g["key"], "tags": rest, "est": 0})
        try:
            listed = ln.status_files()
            tree = "status equal to phaseA.diff's files" if listed == set(self.phasea_files) else \
                "STATUS DIFFERS from phaseA.diff's files: %s" % sorted(listed ^ set(self.phasea_files))
        except Exception as e2:  # noqa: BLE001
            tree = "status not read: %r" % (e2,)
        ln.end["after_exception"] = tree
        self.say("%s %s NO RESULT (exception: %r); %d tag(s) of the group back in the queue; %s" % (ln.name, tag, e, len(rest), tree))
        ln.stop("exception in %s: %r; %s" % (tag, e, tree))

    # --- probes, after the lanes drained
    def run_probes(self):
        items = [it for it in self.plan["items"] if it["where"] == "probes" and self.wanted(it)]
        self.say("probes: %d" % len(items))
        if not items:
            return
        ln = next((l for l in self.lanes if l.alive), None)
        if ln is None and self.lanes == []:
            ln = Lane(self, 1)  # --phase probes: the lane of an earlier run, with its recorded flags
            rp = os.path.join(self.out, "lane_record.json")
            if self.a.lane_flags:
                self.flags = self.a.lane_flags.split()
            elif os.path.exists(rp):
                self.flags = json.load(open(rp, encoding="utf-8")).get("lane_flags", " ".join(UNITY_OFF)).split()
        if ln is None:
            for it in items:
                self.record({"phase": "probe", "tag": it["tag"], "folder": it["folder"], "kind": "probe", "sets": it["sets"],
                             "verdict": "NO RESULT", "summary": ["no live lane"], "notes": []})
            return
        specs = {}
        if self.a.probes:
            specs = json.load(open(self.a.probes, encoding="utf-8"))  # an input file, read from disk in every mode
        for it in items:
            ln.current = it["tag"]
            try:
                if it["tag"] == WITNESS_SMOKE:
                    rec = self.witness_smoke(ln, it)
                else:
                    rec = self.cmdlet_probe(ln, it, specs.get(it["tag"]))
            finally:
                ln.current = None
            rec.update({"phase": "probe", "tag": it["tag"], "folder": it["folder"], "kind": "probe", "sets": it["sets"]})
            self.record(rec)
            self.say("%s %s %s | %s" % (ln.name, it["tag"], rec["verdict"], " | ".join(rec.get("summary", []))[:300]))

    def with_patches(self, ln, tags, body):
        applied, snaps = [], {}
        try:
            for tag in tags:
                it = self.items[tag]
                for t in it["targets"]:
                    snaps.setdefault(t, self.env.read(os.path.join(ln.root, t)))
                rc, out = ln.git("apply", os.path.join(self.root, it["path"].replace("/", os.sep)))
                if rc != 0:
                    return None, "NOT APPLIED: %s %s" % (tag, out.strip()[:120])
                applied.append(tag)
            ok, errs = ln.build(self.flags, "probe")
            if not ok:
                return None, "BUILD FAILED: " + " | ".join(errs[:2])
            return body(), None
        finally:
            for tag in reversed(applied):
                ln.git("apply", "-R", os.path.join(self.root, self.items[tag]["path"].replace("/", os.sep)))
            for t, data in snaps.items():
                p = os.path.join(ln.root, t)
                if self.env.read(p) != data:
                    self.env.write(p, data)
            if applied:
                ln.build(self.flags, "clean")

    def witness_smoke(self, ln, it):
        if not self.env.exists(BUNDLE_B):
            return {"verdict": "NOT RUN", "summary": ["not run - no command (%s missing)" % BUNDLE_B]}
        busy = self.env.busy()
        if busy:
            return {"verdict": "NOT RUN", "summary": ["bundleB refuses while these run: " + "; ".join(busy)]}

        def bundle(which):
            folder = os.path.join(SCR, "b6b12", "probe_b6b8", which)
            cmd = [sys.executable, "-I", "-X", "utf8", BUNDLE_B, "--root", ln.root, "--cells", "wanderer", "--max-seed", "2",
                   "--workers", "1", "--folder", folder, "--no-archive"]
            rc, out = self.env.run_cmd(cmd, self.out)
            probs = [l.strip() for l in out.splitlines() if any(p.search(l) for p in WITNESS_PROBLEMS)]
            # bundleB.py prints 'bundle B: N jobs, ...' only once it starts its jobs; every refusal exits 2 before it.
            ran = re.search(r"^bundle B: \d+ jobs", out, re.M) is not None
            first = next((l.strip() for l in out.splitlines() if l.strip()), "no output")
            return rc, probs, ran, first

        if not ln.clean_build_if_dirty():
            return {"verdict": "NO RESULT", "summary": ["clean build failed"]}
        rc_c, clean, ran_c, first_c = bundle("clean")
        if not ran_c:
            return {"verdict": "NO RESULT", "summary": ["the clean bundle B run did not start its jobs (exit %d): %s" % (rc_c, first_c[:200])]}
        if clean:
            return {"verdict": "NO RESULT", "summary": ["NO RESULT - the clean run already shows a witness problem: " + " / ".join(clean[:2])]}
        res, err = self.with_patches(ln, [it["tag"]], lambda: bundle("mut"))
        if err:
            return {"verdict": err.split(":")[0], "summary": [err]}
        rc_m, mut, ran_m, first_m = res
        if not ran_m:
            return {"verdict": "NO RESULT", "summary": ["the mutant bundle B run did not start its jobs (exit %d): %s" % (rc_m, first_m[:200])]}
        return {"verdict": "KILLED" if mut else "SURVIVED",
                "summary": ["clean exit %d, 0 witness problems" % rc_c, "mutant exit %d, %d: %s" % (rc_m, len(mut), " / ".join(mut[:2]))]}

    def cmdlet_probe(self, ln, it, spec):
        if not spec or not spec.get("runs"):
            return {"verdict": "NOT PROBED", "summary": ["not probed - no seed"]}
        if not ln.clean_build_if_dirty():
            return {"verdict": "NO RESULT", "summary": ["clean build failed"]}
        summary, held = [], True
        for i, run in enumerate(spec["runs"]):
            tags = [it["tag"] if t == "self" else t for t in run.get("patches", ["self"])]
            log = ln.logpath("Probe_%s_%d" % (it["tag"], i))
            args = ["-run=CosmicBalance"] + [a.replace("{LANE}", fwd(ln.root)) for a in run["args"]] + ["-unattended", "-nosplash"]

            def body():
                rc, text, so, peak = ln.editor(args, log, "probe")
                return rc, [strip_wrote(l) for l in (text + "\n" + so).splitlines()]
            res, err = self.with_patches(ln, tags, body) if tags else (body(), None)
            if err:
                return {"verdict": err.split(":")[0], "summary": [err]}
            rc, lines = res
            ok = True
            for pat in run.get("expect", []):
                hit = next((l for l in lines if re.search(pat, l)), None)
                summary.append("run %d %s: %s" % (i, "has" if hit else "MISSING", (hit or pat)[:200]))
                ok = ok and hit is not None
            for pat in run.get("absent", []):
                hit = next((l for l in lines if re.search(pat, l)), None)
                summary.append("run %d %s: %s" % (i, "UNEXPECTED" if hit else "no", (hit or pat)[:200]))
                ok = ok and hit is None
            if "exit" in run:
                summary.append("run %d exit %d (want %d)" % (i, rc, run["exit"]))
                ok = ok and rc == run["exit"]
            held = held and ok
        return {"verdict": "KILLED" if held else "SURVIVED", "summary": summary}

    def write_record(self, t_start):
        rec = {"lanes": [{"name": l.name, "alive": l.alive, "stop": l.stop_reason, "busy_s": round(l.busy, 1), "items": l.items,
                          "mem_waits": l.mem_waits, "mem_wait_s": round(l.mem_wait_s, 1), "peak_gb": round(l.peak, 2), "end": l.end}
                         for l in self.lanes],
               "lane_flags": " ".join(self.flags), "gate_errors": self.gate_errors, "threshold_gb": round(self.threshold(), 2),
               "ubt_held_s": round(self.ubt.held, 1), "wall_s": round(self.sync.now() - t_start, 1), "memory_waits": self.mem_log,
               "dry_run": self.env.dry, "short_baselines": self.short_baselines,
               "phase": self.a.phase, "only": bool(self.a.only), "reuse_lanes": bool(self.a.reuse_lanes)}
        path = os.path.join(self.out, "lane_record.json")
        old = {}
        if os.path.exists(path):
            try:
                old = json.load(open(path, encoding="utf-8"))
            except Exception:  # noqa: BLE001
                old = {}
        # Every run is kept in 'runs'; the top-level keys stay those of the first run that had lanes (the main run),
        # so a later rerun of the dispositions (--reuse-lanes --only) does not hide its wall-clock and lane times.
        runs = old.pop("runs", [])
        runs.append(rec)
        if rec["lanes"] and not old.get("lanes"):
            old.update(rec)  # the first run with lanes becomes the main run
        else:
            for k, v in rec.items():
                old.setdefault(k, v)
        old["runs"] = runs
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            json.dump(old, f, ensure_ascii=False, indent=1)
        return old

# ===================================================================================================== merge (section 5.7)


def load_results(out):
    res = {}
    path = os.path.join(out, "results.jsonl")
    if os.path.exists(path):
        for line in open(path, encoding="utf-8"):
            if line.strip():
                r = json.loads(line)
                res[r["tag"]] = r  # the last record of a tag wins (reruns)
    return res


def set_label(it):
    if "B6b" in it["sets"]:
        return "B6b"
    return "older " + "+".join(it["sets"])


def merge(plan, out, record=None, log=print):
    res = load_results(out)
    items = sorted(plan["items"], key=lambda x: x["index"])
    planned = [it for it in items if it["run"]]
    listed = [it for it in items if it["sets"] and not it["run"]]
    counts = collections.Counter()
    for it in planned:
        r = res.get(it["tag"])
        counts[(set_label(it), it["kind"], r["verdict"] if r else "NO VERDICT")] += 1
    body = []
    for it in planned:
        r = res.get(it["tag"])
        if not r:
            body.append("%s NO VERDICT" % it["tag"])
            continue
        if r["phase"] == "pool":
            body.append("%s %s | %s" % (it["tag"], r["verdict"], " | ".join(r.get("summary", []))[:300]))
        elif r["phase"] == "probe":
            body.append("%s %s | %s" % (it["tag"], r["verdict"], " | ".join(r.get("summary", []))[:300]))
        else:
            for n in r.get("notes", []):
                if n.startswith("assets: "):
                    body.append("%s %s" % (it["tag"], n))
            body.append("%s %s %s" % (it["tag"], r["verdict"], [x[1] for x in r.get("results", [])]))
            for e in r.get("errors", [])[:4]:
                body.append("     " + e)
            for n in r.get("notes", []):
                if n.startswith("assets after revert"):
                    body.append("%s %s" % (it["tag"], n))
                elif not n.startswith("assets"):
                    body.append("     note: " + n)
    not_killed = [(it, res.get(it["tag"])) for it in planned if not res.get(it["tag"]) or res[it["tag"]]["verdict"] != "KILLED"]
    no_verdict = [it["tag"] for it in planned if not res.get(it["tag"])]
    head = ["# [B-6b12] the whole-B6b mutation rerun, merged (README order) - lanes.py",
            "# items planned %d (B6b %d, older %d), listed but not run %d (no longer applies / already rerun)" %
            (len(planned), sum("B6b" in i["sets"] for i in planned), sum("B6b" not in i["sets"] for i in planned), len(listed))]
    head.append("# counts by set, kind and verdict:")
    for (s, k, v), n in sorted(counts.items()):
        head.append("#   %-24s %-7s %-13s %d" % (s, k, v, n))
    head.append("# not KILLED (%d):" % len(not_killed))
    for it, r in not_killed:
        head.append("#   %s %s (%s, %s)" % (it["tag"], r["verdict"] if r else "NO VERDICT", set_label(it), it["kind"]))
    head.append("# rows with no verdict: %d%s" % (len(no_verdict), (" - " + " ".join(no_verdict)) if no_verdict else ""))
    if record:
        head.append("# <LANE_FLAGS> = %s; memory gate %.2f GB; UBT mutex held %.0f s; wall-clock %.0f s (%.0f min)%s" %
                    (record.get("lane_flags"), record.get("threshold_gb", 0), record.get("ubt_held_s", 0), record.get("wall_s", 0),
                     record.get("wall_s", 0) / 60.0, " (dry run - simulated)" if record.get("dry_run") else ""))
        for l in record.get("lanes", []):
            head.append("#   %s busy %.0f s, items %d, memory waits %d (%.0f s), peak %.2f GB%s; end: %s" %
                        (l["name"], l["busy_s"], l["items"], l["mem_waits"], l["mem_wait_s"], l["peak_gb"],
                         "" if l["alive"] else " STOPPED: %s" % l["stop"], "; ".join("%s %s" % kv for kv in l["end"].items())))
        if record.get("gate_errors"):
            head.append("#   gate build errors (-DisableUnity): " + " | ".join(record["gate_errors"][:5]))
        for sb in record.get("short_baselines", []):
            head.append("#   SHORT BASELINE (N section 5): " + sb)
        for i, rr in enumerate(record.get("runs", [])):
            lanes_txt = ", ".join("%s busy %.0f s / %d items / mem waits %d (%.0f s)%s" %
                                  (l["name"], l["busy_s"], l["items"], l["mem_waits"], l["mem_wait_s"], "" if l["alive"] else " STOPPED")
                                  for l in rr.get("lanes", [])) or "no lane"
            head.append("# run %d: --phase %s%s%s; <LANE_FLAGS> %s; wall-clock %.0f s (%.0f min); UBT held %.0f s; memory waits %d; lanes: %s%s" %
                        (i + 1, rr.get("phase", "?"), " --only" if rr.get("only") else "", " --reuse-lanes" if rr.get("reuse_lanes") else "",
                         rr.get("lane_flags"), rr.get("wall_s", 0), rr.get("wall_s", 0) / 60.0, rr.get("ubt_held_s", 0),
                         len(rr.get("memory_waits", [])), lanes_txt, " (dry run)" if rr.get("dry_run") else ""))
    for it in listed:
        head.append("# not run: %s (%s) %s" % (it["tag"], set_label(it), "already rerun" if it.get("already_rerun") else "no longer applies"))
    with open(os.path.join(out, "b6b_all_mutations.txt"), "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(head + body) + "\n")
    hints = {"SURVIVED": "find the cause: (i) meaning moved -> retake same name; (ii) assertion lost -> tighten, rerun; (iii) flaky -> run once more",
             "NOT APPLIED": "a later B6b commit moved the code: retake same name (<MUT_RETAKEN>)",
             "BUILD FAILED": "a later B6b commit moved the code: retake same name (<MUT_RETAKEN>)",
             "NO RESULT": "run once more; a stale filter name -> correct the README row",
             "NOT PROBED": "probe without a seed: 'not probed - no seed' in N section 5",
             "NOT RUN": "record in N section 5"}
    older = "older-block: KILLED nothing; SURVIVED -> b6b12_ retake unless a live patch breaks that assertion (docs/40:40 mark)"
    with open(os.path.join(out, "dispositions.tsv"), "w", encoding="utf-8", newline="\n") as f:
        f.write("\t".join(["tag", "folder", "set", "kind", "verdict", "tests", "readme_line", "hint", "cause", "action", "rerun_verdict", "N5_note"]) + "\n")
        for it, r in not_killed:
            v = r["verdict"] if r else "NO VERDICT"
            tests = "+".join(it["filters"]) if it["filters"] else ",".join((it.get("py") or {}).get("methods", []))
            hint = hints.get(v, "") if "B6b" in it["sets"] else older
            f.write("\t".join([it["tag"], it["folder"], set_label(it), it["kind"], v, tests, str(it["line"]), hint, "", "", "", ""]) + "\n")
    mc = {"planned": len(planned), "listed_not_run": len(listed), "no_verdict": len(no_verdict),
          "by": {"%s|%s|%s" % k: n for k, n in sorted(counts.items())},
          "killed": sum(n for (s, k, v), n in counts.items() if v == "KILLED"),
          "not_killed": len(not_killed), "lane_flags": record.get("lane_flags") if record else None,
          "wall_s": record.get("wall_s") if record else None}
    with open(os.path.join(out, "mut_counts.json"), "w", encoding="utf-8", newline="\n") as f:
        json.dump(mc, f, ensure_ascii=False, indent=1)
    log("merge: %d planned, %d KILLED, %d not KILLED, %d without verdict -> %s" %
        (len(planned), mc["killed"], len(not_killed), len(no_verdict), os.path.join(out, "b6b_all_mutations.txt")))
    return mc

# ===================================================================================================== estimate (DES)


PY_COST = {"method": 3, "whole": 88, "check": 6}


def simulate(plan, lanes, table="mid", setup_build=390, baseline=300, worktree=40, clean=15, probe_cost=180, bundle=240,
             pool_workers=4):
    """A discrete-event schedule of section 5.4: the pool, then the lanes (one FIFO mutex), then the probes."""
    sys.path.insert(0, HERE)
    import plan as planmod  # noqa: E402
    items = {it["tag"]: it for it in plan["items"]}
    # pool: the clean start and end runs of both tools in every copy, items LPT over the workers
    py = [it for it in plan["items"] if it["where"] == "pool"]
    costs = sorted((PY_COST["whole"] if it["py"]["whole"] else PY_COST["method"] * max(1, len(it["py"]["methods"])) if it["py"]["methods"] else 0)
                   + (PY_COST["check"] if it["py"]["check_data"] else 0) for it in py)[::-1]
    w = [0.0] * pool_workers
    for c in costs:
        w[w.index(min(w))] += c + 1
    pool = (2 * (PY_COST["whole"] + PY_COST["check"]) + max(w) + 20) if py else 0.0
    # lanes
    groups = collections.deque(plan["groups"])
    mutex_free = 0.0
    held = 0.0
    t_setup = worktree * lanes
    ready = []
    # lane 1 gate build, then the other lanes' first builds in turn; each baseline after its own build
    t = t_setup
    for n in range(lanes):
        t += setup_build
        held += setup_build
        heapq.heappush(ready, (t + baseline, n))
    mutex_free = t
    lane_end = [0.0] * lanes
    busy = [0.0] * lanes
    last_kind = [None] * lanes
    current = [None] * lanes
    while ready:
        now, n = heapq.heappop(ready)
        if current[n] is None or not current[n]:
            if not groups:
                start = max(now, mutex_free)
                mutex_free = start + clean
                held += clean
                lane_end[n] = mutex_free + 2
                continue
            current[n] = list(groups.popleft()["tags"])
        tag = current[n].pop(0)
        it = items[tag]
        m, f = planmod.item_cost(it, table)
        extra = 0.0
        if it["kind"] == "asset" and last_kind[n] == "cpp":
            extra = clean  # the clean build before an asset item
        last_kind[n] = it["kind"] if it.get("apply") != "FAIL" else last_kind[n]
        if m + extra > 0:
            start = max(now, mutex_free)
            mutex_free = start + m + extra
            held += m + extra
            done = mutex_free + f
        else:
            done = now + f
        busy[n] += done - now
        heapq.heappush(ready, (done, n))
    lanes_wall = max(lane_end)
    probes = [it for it in plan["items"] if it["where"] == "probes"]
    p_time = sum((2 * bundle + 2 * 60) if it["tag"] == WITNESS_SMOKE else probe_cost for it in probes)
    return {"lanes": lanes, "table": table, "pool_s": pool, "setup_s": t_setup + lanes * setup_build + baseline,
            "lanes_s": lanes_wall, "probes_s": p_time, "total_s": pool + lanes_wall + p_time, "mutex_held_s": held,
            "busy_s": busy}


def estimate(plan, log=print):
    out = {}
    for lanes in (3, 2):
        for table in ("low", "mid", "high"):
            r = simulate(plan, lanes, table)
            out[(lanes, table)] = r
            log("estimate %d lanes, %-4s costs: pool %4.1f min, lanes (setup included) %5.1f min, probes %4.1f min -> total %5.1f min; mutex held %5.1f min" %
                (lanes, table, r["pool_s"] / 60, r["lanes_s"] / 60, r["probes_s"] / 60, r["total_s"] / 60, r["mutex_held_s"] / 60))
    return out

# ===================================================================================================== main


def parse_args(argv):
    ap = argparse.ArgumentParser()
    ap.add_argument("--plan", required=True)
    ap.add_argument("--root", default=None)
    ap.add_argument("--phaseA-diff", dest="phaseA_diff", default=os.path.join(HERE, "phaseA.diff"))
    ap.add_argument("--assets", default=os.path.join(HERE, "phaseA_assets"))
    ap.add_argument("--asset-names", dest="asset_names", default=None)
    ap.add_argument("--lanes", type=int, default=3)
    ap.add_argument("--workers", type=int, default=4)
    ap.add_argument("--base", default="C:\\CAL")
    ap.add_argument("--out", default=None)
    ap.add_argument("--phase", default="all", choices=("all", "pool", "lanes", "probes", "merge"))
    ap.add_argument("--probes", default=None)
    ap.add_argument("--only", default=None)
    ap.add_argument("--reuse-lanes", dest="reuse_lanes", action="store_true")
    ap.add_argument("--lane-flags", dest="lane_flags", default=None)
    ap.add_argument("--dry-run", dest="dry_run", action="store_true")
    ap.add_argument("--estimate", action="store_true")
    a = ap.parse_args(argv)
    if a.asset_names:
        a.asset_names = a.asset_names.split(",")
    return a


def main(argv=None, env_factory=None, sim_opts=None):
    a = parse_args(argv)
    plan = json.load(open(a.plan, encoding="utf-8"))
    if a.estimate:
        estimate(plan)
        return 0
    bad = list(plan.get("problems") or []) + ["no patch file: %s" % m for m in (plan.get("missing") or [])]
    if bad:
        # plan.py writes plan.json even when it exits 1; such a plan would run rows with no method or no filter.
        raise SystemExit("lanes: the plan has %d problem(s) - fix the README first: %s" % (len(bad), "; ".join(bad[:5])))
    if a.out is None:
        a.out = os.path.join(HERE, "dry" if a.dry_run else "lanes_run")
    os.makedirs(a.out, exist_ok=True)
    if a.dry_run or env_factory:
        # Nothing outside --out: the lane roots and the project root are names only, every command is mocked.
        sync = VirtualSync()
        a.root = a.root or os.path.join(a.out, "sim_main")
        a.base = os.path.join(a.out, "sim_CAL") if a.base == "C:\\CAL" else a.base
        pfiles = []
        if a.phaseA_diff and os.path.exists(a.phaseA_diff):
            from plan import diff_files  # noqa: E402
            pfiles = diff_files(open(a.phaseA_diff, encoding="utf-8").read())
        if not pfiles:
            pfiles = ["Source/Cosmic_Ascension/UI/CosmicChronicleWidget.cpp", "Tools/data/DT_LogTemplates.json", "Tools/data/generated.sha1"]
            a.phaseA_diff = os.path.join(a.out, "sim_phaseA.diff")
        opts = dict(sim_opts or {})
        opts.setdefault("tests", plan.get("tests"))
        opts.setdefault("roots", [a.root] + [os.path.join(a.base, "l%d" % n) for n in range(1, 9)] + [os.path.join(a.base, "py", "c%d" % i) for i in range(8)])
        env = (env_factory or SimEnv)(sync, plan["items"], a.root, pfiles, opts)
        diff_text = "".join("diff --git a/%s b/%s\n" % (f, f) for f in pfiles).encode()
        env.write(a.phaseA_diff, diff_text)
        env.write(os.path.join(a.assets, "DT_LogTemplates.uasset"), b"uasset bytes")
    else:
        if not a.root:
            raise SystemExit("lanes: --root is required for a real run")
        sync = RealSync()
        env = RealEnv(sync)
    rec_path = os.path.join(a.out, "results.jsonl")
    if a.phase in ("all", "pool") and os.path.exists(rec_path) and not a.only:
        os.replace(rec_path, rec_path + ".%d.bak" % int(time.time()))
    lr_path = os.path.join(a.out, "lane_record.json")
    if a.phase in ("all", "pool", "lanes") and os.path.exists(lr_path) and not a.only and not a.reuse_lanes:
        # A fresh run starts a fresh lane record; a rerun (--only / --reuse-lanes) or --phase probes adds to it.
        os.replace(lr_path, lr_path + ".%d.bak" % int(time.time()))
    run = Run(a, plan, env, sync)
    sync.register()
    record = None
    try:
        t0 = sync.now()
        if a.phase != "merge":
            run.preflight(a.phase in ("all", "lanes"))
        if a.phase in ("all", "pool"):
            run.run_pool()
        if a.phase in ("all", "lanes"):
            run.run_lanes()
        if a.phase in ("all", "probes"):
            run.run_probes()
        if a.phase != "merge" and a.phase != "pool":
            record = run.write_record(t0)
        elif os.path.exists(os.path.join(a.out, "lane_record.json")):
            record = json.load(open(os.path.join(a.out, "lane_record.json"), encoding="utf-8"))
        if a.phase in ("all", "merge"):
            run.mc = merge(plan, a.out, record, run.say)
    finally:
        sync.unregister()
    with open(os.path.join(a.out, "run_log.txt"), "a", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(run.lines) + "\n")
    if a.dry_run:
        estimate(plan, run.say)
    return run


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    sys.path.insert(0, HERE)
    r = main()
    sys.exit(0 if r is not None else 1)
