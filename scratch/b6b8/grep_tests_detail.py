"""[B-6b8] start stage, item 9: for each test body that holds a wanderer and a claim, or runs >= 600 ticks, or drives the
policies, print its SetTables / table-replacing lines and the lines naming a wanderer, a claim or the policies. Read-only.
usage: grep_tests_detail.py <Tests dir> <run_checks.ps1>"""
import os
import re
import sys

tests_dir, runner = sys.argv[1], sys.argv[2]
smoke = re.search(r"else \{ '([^']+)' \}", open(runner, encoding='utf-8').read()).group(1).split('+')
TEST_RE = re.compile(r'IMPLEMENT_[A-Z_]*AUTOMATION_TEST\(\s*(\w+)\s*,\s*"([^"]+)"')
KEYS = re.compile(r'SetTables\(|\.Events\s*=|\.Encounters\s*=|WithoutB6bData|WithoutPressure|NewGalaxy\(|RunHeadless\(|'
                  r'Abdicate|EPlayerRole::Wanderer|Hazard\.Claim|ClaimTag|CosmicWandererPolicy::Act|CosmicRulerPolicy::Act\(|Horizon|for \(int')
for name in sorted(os.listdir(tests_dir)):
    if not name.endswith('.cpp'):
        continue
    text = open(os.path.join(tests_dir, name), encoding='utf-8', errors='replace').read()
    marks = [(m.start(), m.group(2)) for m in TEST_RE.finditer(text)]
    for i, (start, test) in enumerate(marks):
        if test not in sys.argv[3:]:
            continue
        end = marks[i + 1][0] if i + 1 < len(marks) else len(text)
        first = text.count('\n', 0, start) + 1
        tier = 'smoke' if any(test.startswith(p) for p in smoke) else 'full'
        print('=== %s (%s:%d, %s)' % (test, name, first, tier))
        for n, line in enumerate(text[start:end].split('\n')):
            if KEYS.search(line):
                print('  %d: %s' % (first + n, line.strip()[:200]))
