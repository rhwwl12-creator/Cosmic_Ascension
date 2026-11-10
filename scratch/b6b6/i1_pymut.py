"""I1: run Python-only mutation patches in scratch copies of the project (never in the project tree), in parallel.
Each spec: <patch path relative to root>=<what> where what is 'check_data', 'schema' (the whole test_data_schema.py) or a
comma list of test_data_schema method names. KILLED = the tool exits non-zero; the copy is reverted after each patch and a
final clean run of both tools is made in every copy.
usage: i1_pymut.py <project root> <work dir> <workers> <spec file>"""
import concurrent.futures, os, shutil, subprocess, sys
sys.stdout.reconfigure(encoding='utf-8')
root, work, workers, spec_file = sys.argv[1], sys.argv[2], int(sys.argv[3]), sys.argv[4]
specs = []
for line in open(spec_file, encoding='utf-8'):
    line = line.strip()
    if line and not line.startswith('#'):
        patch, what = line.split('=', 1)
        specs.append((patch, what))
if os.path.exists(work):
    shutil.rmtree(work)
copies = []
for i in range(workers):
    c = os.path.join(work, 'c%d' % i)
    for d in ('Tools', 'Config', 'Source'):
        shutil.copytree(os.path.join(root, d), os.path.join(c, d), ignore=shutil.ignore_patterns('__pycache__', '*.pyc'))
    shutil.copy(os.path.join(root, 'CLAUDE.md'), c)
    copies.append(c)

def tool_cmd(what):
    if what == 'check_data':
        return [sys.executable, '-I', '-X', 'utf8', 'Tools/check_data.py']
    if what == 'schema':
        return [sys.executable, '-I', '-X', 'utf8', 'Tools/test_data_schema.py']
    return [sys.executable, '-I', '-X', 'utf8', 'Tools/test_data_schema.py'] + ['SchemaTests.' + m for m in what.split(',')]

def run(i):
    c = copies[i]
    out = []
    for patch, what in specs[i::workers]:
        p = os.path.join(root, patch)
        if subprocess.run(['git', 'apply', p], cwd=c, capture_output=True).returncode != 0:
            out.append('%s NOT-APPLIED' % patch)
            continue
        r = subprocess.run(tool_cmd(what), cwd=c, capture_output=True, text=True, encoding='utf-8', errors='replace')
        lines = [l for l in (r.stdout + r.stderr).splitlines() if l.strip()]
        summary = next((l for l in reversed(lines) if l.startswith(('FAILED', 'OK', 'check_data'))), lines[-1] if lines else '')
        out.append('%s %s exit=%d [%s] %s' % (patch, 'KILLED' if r.returncode else 'SURVIVED', r.returncode, what, summary[:100]))
        if subprocess.run(['git', 'apply', '-R', p], cwd=c, capture_output=True).returncode != 0:
            out.append('%s REVERT-FAILED' % patch)
    for what in ('check_data', 'schema'):
        r = subprocess.run(tool_cmd(what), cwd=c, capture_output=True, text=True, encoding='utf-8', errors='replace')
        out.append('clean copy c%d %s exit=%d' % (i, what, r.returncode))
    return out

with concurrent.futures.ThreadPoolExecutor(workers) as ex:
    for lines in ex.map(run, range(workers)):
        for l in lines:
            print(l, flush=True)
