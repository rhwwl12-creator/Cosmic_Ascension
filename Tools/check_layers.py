"""Layer and convention lint for the Cosmic_Ascension module (no editor needed; standard library only).

Checks (docs/40_DEV_GUIDE.md section 2, CLAUDE.md invariant rules 1, 3, 10):
  1. Include direction  Core <- Data <- Sim <- {Events, Ascension, Log, Player, Save} <- {World, UI}  (Tests may include anything).
  2. Core Logic (Core, Data, Sim, Events, Ascension, Log, Player, Save) includes no widget / actor engine headers.
  3. No FMath::Rand* anywhere in the module (the three random sources are GetRng, DrawTell, CosmicSeed::Derive).
  3b. Core Logic calls only the FMath members of a short list (Max, Min, Clamp, Abs, Sign, Square, Sqrt, RoundToInt32,
      IsNaN - inline + - * / and sqrt; the same for the FPlatformMath / FGenericPlatformMath spellings) and no C runtime
      math but sqrt (pow, exp, log, the trigonometric and hyperbolic functions, atan2, fmod, cbrt, hypot ... - a deny list
      of named functions, bare, std:: or ::, with the f and l forms): the values a hash holds come from + - * / and sqrt
      in module code (invariant rule 6). The game exe links the engine compiled /fp:fast, the editor /fp:precise, so
      another call could give the cooked build another world than the editor's (MVP item 9,
      docs/60_DECISIONS/headers/A9_win64_packaging.md section 3.3). String literals are blanked first, so a log message
      saying "the log (...)" is not a call. Extend the list on purpose when a function is needed. Header helpers such as
      FVector::DistSquared and FRandomStream are not seen (docs/20_ARCHITECTURE.md section 9.2).
  4. Non-ASCII bytes in source files (the Korean lives in DataTable / String Table only).
  5. Report only: RequestGameplayTag(FName(TEXT("..."))) literals outside the constant headers, and DT_Rules knobs that no
     runtime code reads (reserved knobs). With --strict these counts may not grow past the recorded baseline.

Usage: python -I Tools/check_layers.py [--strict]     exit code 1 on any error.
"""
import json, os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, 'Source', 'Cosmic_Ascension')
STRICT = '--strict' in sys.argv

LAYER = {'Core': 0, 'Data': 1, 'Sim': 2, 'Events': 3, 'Ascension': 3, 'Log': 3, 'Player': 3, 'Save': 3, 'World': 4, 'UI': 4, 'Tests': 9}
CORE_LOGIC = {'Core', 'Data', 'Sim', 'Events', 'Ascension', 'Log', 'Player', 'Save'}
FORBIDDEN_ENGINE = re.compile(r'#include\s+"(Blueprint/UserWidget\.h|GameFramework/(Actor|Pawn|PlayerController|HUD|GameModeBase|Character)\.h|Components/(Widget|TextBlock|Button|ListView|SceneComponent|StaticMeshComponent)\w*\.h|Engine/World\.h)"')
ALLOWED_EXCEPTIONS = set()  # header-level exceptions (Sim/CosmicEvalContext.h resolved Sim -> Events)
# Baselines for report-only counts (update deliberately when a cleanup lands).
BASELINE = {'tag_literals': 47, 'unused_knobs': 18}  # measured 2026-10-19; +2 with MVP item 2 (Log.Civ, Notable.Rebel), 2026-10-20; +1 with MVP item 3 (Hazard.Claim in the AI stage), 2026-10-22; +3 with MVP item 4 (Log.Calling, Log.Claim, Log.Record in Log/CosmicChronicle.cpp), 2026-10-23; -2 with MVP item 5 (the move filter reads CosmicChronicle - Log.Claim, Log.Record left Log/CosmicLogSubsystem.cpp), 2026-10-23; +8 with B1 (Reign.Abdicated, Notable.Regent and Notable.Ruler in the wanderer's commands, the abdication's pipeline and the rebellion's end), 2026-10-25

errors, warnings, notes = [], [], []
sources = []
for dirpath, _, files in os.walk(SRC):
    for fn in files:
        if fn.endswith(('.h', '.cpp')):
            sources.append(os.path.join(dirpath, fn))
sources.sort()

def folder_of(path):
    rel = os.path.relpath(path, SRC).replace('\\', '/')
    return rel.split('/')[0] if '/' in rel else ''

def strip_comments(text):
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
    return re.sub(r'//[^\n]*', '', text)

inc_re = re.compile(r'^\s*#include\s+"([^"]+)"', re.M)
rand_re = re.compile(r'\bFMath::(Rand|RandRange|FRand|FRandRange|RandHelper|RandBool|SRand|VRand)\b')
math_member_re = re.compile(r'\b(?:FMath|FPlatformMath|FGenericPlatformMath|FWindowsPlatformMath)::(\w+)')
MATH_ALLOWED = {'Max', 'Min', 'Clamp', 'Abs', 'Sign', 'Square', 'Sqrt', 'RoundToInt32', 'IsNaN'}
crt_math_re = re.compile(r'(?<![\w.>:])(?:(?:std)?::)?(?:pow|exp2?|expm1|log(?:2|10|1p)?|a?(?:sin|cos|tan)h?|atan2|fmod|cbrt|hypot|modf|remainder)[fl]?\s*\(')

def strip_strings(text):
    text = re.sub(r'"(?:\\.|[^"\\\n])*"', '""', text)
    return re.sub(r"'(?:\\.|[^'\\\n])'", "''", text)
tag_lit_re = re.compile(r'RequestGameplayTag\(\s*FName\(\s*TEXT\(')
tag_literals = 0
for path in sources:
    folder = folder_of(path); rel = os.path.relpath(path, ROOT).replace('\\', '/')
    raw = open(path, 'rb').read()
    if any(b > 0x7F for b in raw):
        errors.append(f'{rel}: non-ASCII byte (Korean belongs in DataTable/String Table)')
    text = strip_comments(raw.decode('utf-8', errors='replace'))
    if rand_re.search(text):
        errors.append(f'{rel}: FMath::Rand* used (only GetRng / DrawTell / CosmicSeed::Derive may draw)')
    if folder in CORE_LOGIC:
        m = FORBIDDEN_ENGINE.search(text)
        if m: errors.append(f'{rel}: Core Logic includes {m.group(1)}')
        code = strip_strings(text)
        for m in math_member_re.finditer(code):
            if m.group(1) not in MATH_ALLOWED:
                errors.append(f'{rel}: Core Logic calls {m.group(0)} (only + - * / and sqrt reach a hash - invariant rule 6)')
                break
        m = crt_math_re.search(code)
        if m: errors.append(f'{rel}: Core Logic calls {m.group(0).strip()} (only + - * / and sqrt reach a hash - invariant rule 6)')
    for inc in inc_re.findall(text):
        target = inc.split('/')[0] if '/' in inc else ''
        if target not in LAYER or folder not in LAYER: continue
        if LAYER[target] > LAYER[folder] and folder != 'Tests':
            if folder == 'Sim' and LAYER[target] == 3 and path.endswith('.cpp'):
                continue  # tick stages call the sibling subsystems through GetSubsystem (DEV_GUIDE section 2) - .cpp only
            if (folder, target) in ALLOWED_EXCEPTIONS: warnings.append(f'{rel}: includes {inc} (Sim header -> Events, documented exception until CosmicEvalContext.h)')
            else: errors.append(f'{rel}: includes {inc} against the layer direction')
        if LAYER[target] == 1 and LAYER[folder] == 0:
            errors.append(f'{rel}: Core includes Data header {inc}')
    if folder != 'Tests' and not path.endswith(('CosmicRowNames.h', 'CosmicUILabels.h', 'CosmicTags.h')):
        tag_literals += len(tag_lit_re.findall(text))
notes.append(f'tag literals outside constant headers: {tag_literals} (baseline {BASELINE["tag_literals"]})')
if STRICT and tag_literals > BASELINE['tag_literals']:
    errors.append(f'tag literal count grew: {tag_literals} > {BASELINE["tag_literals"]}')

# DT_Rules knobs without a runtime reader
rules_path = os.path.join(ROOT, 'Tools', 'data', 'DT_Rules.json')
runtime_text = ''
for path in sources:
    if folder_of(path) != 'Tests' and not path.endswith('CosmicDefinitionRows.h'):
        runtime_text += strip_comments(open(path, encoding='utf-8', errors='replace').read()) + '\n'
knobs = [k for k in json.load(open(rules_path, encoding='utf-8'))[0].keys() if k != 'Name']
unused = [k for k in knobs if not re.search(r'\b' + re.escape(k) + r'\b', runtime_text)]
notes.append(f'DT_Rules knobs: {len(knobs)}, without a runtime reader: {len(unused)} (baseline {BASELINE["unused_knobs"]}): ' + ', '.join(unused))
if STRICT and len(unused) > BASELINE['unused_knobs']:
    errors.append(f'unused knob count grew: {len(unused)} > {BASELINE["unused_knobs"]} (add a reader or a since: tag in schema.json)')

for w in warnings: print('WARN  ' + w)
for n in notes: print('NOTE  ' + n)
for e in errors: print('ERROR ' + e)
print(f'check_layers: {len(sources)} files, {len(errors)} error(s), {len(warnings)} warning(s)')
sys.exit(1 if errors else 0)
