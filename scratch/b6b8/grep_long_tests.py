"""[B-6b8] start stage, item 9 (lesson 6): list the automation tests that may run a ruled people under pressure for >= 600
ticks on the game's DT_Events, or put a wanderer on a claimed system on the game's DT_Encounters. Read-only heuristic:
per test body, the largest RunHeadless argument (or a StepOnce loop bound), whether it replaces DT_Events / DT_Encounters /
the pressure (NewTable, WithoutPressure, WithoutB6bData, SetTables of its own), whether it plays a wanderer or a claim.
usage: grep_long_tests.py <Tests dir> <run_checks.ps1>"""
import os
import re
import sys

tests_dir, runner = sys.argv[1], sys.argv[2]
smoke = re.search(r"else \{ '([^']+)' \}", open(runner, encoding='utf-8').read()).group(1).split('+')

TEST_RE = re.compile(r'IMPLEMENT_[A-Z_]*AUTOMATION_TEST\(\s*(\w+)\s*,\s*"([^"]+)"')
rows = []
for name in sorted(os.listdir(tests_dir)):
    if not name.endswith('.cpp'):
        continue
    text = open(os.path.join(tests_dir, name), encoding='utf-8', errors='replace').read()
    marks = [(m.start(), m.group(2)) for m in TEST_RE.finditer(text)]
    for i, (start, test) in enumerate(marks):
        end = marks[i + 1][0] if i + 1 < len(marks) else len(text)
        body = text[start:end]
        line = text.count('\n', 0, start) + 1
        nums = []
        for m in re.finditer(r'RunHeadless\(\s*([^)]*)\)', body):
            arg = m.group(1)
            for n in re.findall(r'\b(\d[\d\']*)\b', arg):
                nums.append(int(n.replace("'", '')))
        for m in re.finditer(r'for \(int\d* \w+ = \d+; \w+ < (\d+);[^)]*\)\s*\{?\s*[^;]*StepOnce', body):
            nums.append(int(m.group(1)))
        most = max(nums) if nums else 0
        own_events = bool(re.search(r'\.Events\s*=|Events\s*=\s*CosmicTests::NewTable|WithoutPressure|NewTable\(FCosmicEventDef', body))
        own_enc = bool(re.search(r'\.Encounters\s*=|NewTable\(FEncounterDef', body))
        without_b6b = 'WithoutB6bData' in body
        wanderer = bool(re.search(r'Abdicate|Wanderer|EPlayerRole::Wanderer|bWander', body))
        claim = bool(re.search(r'Hazard\.Claim|ClaimTag|Claim\(\)', body))
        ruler = bool(re.search(r'Answer|RulerPolicy|EPlayerRole::Ruler|RuledCivID', body))
        tier = 'smoke' if any(test.startswith(p) for p in smoke) else 'full'
        if most >= 600 or (wanderer and claim):
            rows.append((test, name, line, most, own_events, own_enc, without_b6b, ruler, wanderer, claim, tier))

print('smoke filter:', '+'.join(smoke))
print('test | file:line | most ticks | own DT_Events or no pressure | own DT_Encounters | WithoutB6bData | ruler words | wanderer words | claim words | tier')
for r in rows:
    print('%s | %s:%d | %d | %s | %s | %s | %s | %s | %s | %s' % r)
