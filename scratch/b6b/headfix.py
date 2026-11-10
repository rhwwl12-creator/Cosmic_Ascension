"""For every tracked file that differs from HEAD only in line endings, write HEAD's bytes back (or with 'list', only list).
Also lists every changed file with whether it is LF."""
import subprocess, sys
mode = sys.argv[1] if len(sys.argv) > 1 else 'list'
out = subprocess.run(['git', 'diff', '--name-only', 'HEAD'], capture_output=True).stdout.decode('utf-8').split('\n')
tracked = subprocess.run(['git', 'ls-files'], capture_output=True).stdout.decode('utf-8').split('\n')
for f in tracked:
    if not f:
        continue
    try:
        cur = open(f, 'rb').read()
    except FileNotFoundError:
        continue
    head = subprocess.run(['git', 'show', 'HEAD:' + f], capture_output=True).stdout
    if cur == head:
        continue
    if cur.replace(b'\r\n', b'\n') == head.replace(b'\r\n', b'\n'):
        if mode == 'fix':
            open(f, 'wb').write(head)
            print('restored HEAD bytes:', f)
        else:
            print('eol only:', f)
    else:
        print('changed:', f, 'CRLF' if b'\r\n' in cur else 'LF')
