"""Apply each Tools-only mutation patch, run the Python tool that should catch it, revert. KILLED = the tool fails."""
import subprocess, sys
sys.stdout.reconfigure(encoding="utf-8")
for name in sys.argv[1:]:
    patch = "Tools/mutations/B6b/" + name
    tag = name[:-6]
    if subprocess.run(["git", "apply", patch]).returncode != 0:
        print(tag, "PATCH DID NOT APPLY", flush=True); continue
    tool = "Tools/test_data_schema.py" if "_schema_" in tag else "Tools/check_data.py"
    r = subprocess.run([sys.executable, "-I", tool], capture_output=True, text=True, encoding="utf-8", errors="replace")
    last = [l for l in (r.stdout + r.stderr).splitlines() if l.strip()][-1:] 
    print(tag, "KILLED" if r.returncode != 0 else "SURVIVED", r.returncode, "|", last[0][:120] if last else "", flush=True)
    subprocess.run(["git", "apply", "-R", patch])
for tool in ("Tools/test_data_schema.py", "Tools/check_data.py"):
    r = subprocess.run([sys.executable, "-I", tool], capture_output=True, text=True, encoding="utf-8", errors="replace")
    print(tool, r.returncode, [l for l in (r.stdout + r.stderr).splitlines() if l.strip()][-1:])
