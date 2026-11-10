"""Heuristic: which automation tests tick a world where a people stands on the third rung or above, on tables that may still
hold the game's DT_Feats / DT_Hazards. Prints one line per test with the signals found."""
import os
import re

root = r"c:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension/Source/Cosmic_Ascension/Tests"
decl = re.compile(r'IMPLEMENT_SIMPLE_AUTOMATION_TEST\(\s*(\w+)\s*,\s*"([^"]+)"')
body = re.compile(r'bool\s+(\w+)::RunTest\(')
tick = re.compile(r'\b(StepOnce|RunHeadless|RunLeap|RunTicks|TickTimes|Walk|StepDeath|Advance|TickOnce|Step)\s*\(')
t3 = re.compile(r'Tier\s*=\s*EAscensionTier::Tier(3_Causal|4_Transcendent|5_\w+)')
t3s = re.compile(r'EAscensionTier::Tier3_Causal')
safe_feats = re.compile(r'WithoutProjects|WithoutB6bData|\.Feats\s*=|Feats\s*=\s*')
safe_haz = re.compile(r'WithoutB6bData|\.Hazards\s*=')
for name in sorted(os.listdir(root)):
    if not name.endswith('.cpp'):
        continue
    src = open(os.path.join(root, name), encoding='ascii', errors='replace').read()
    names = {m.group(1): m.group(2) for m in decl.finditer(src)}
    starts = [(m.start(), m.group(1)) for m in body.finditer(src)]
    for i, (pos, cls) in enumerate(starts):
        end = starts[i + 1][0] if i + 1 < len(starts) else len(src)
        b = src[pos:end]
        if not t3.search(b) and not t3s.search(b):
            continue
        ticks = len(tick.findall(b))
        if ticks == 0:
            continue
        sf = bool(safe_feats.search(b))
        sh = bool(safe_haz.search(b))
        print(f"{name}\t{names.get(cls, cls)}\tticks={ticks}\tT3assign={len(t3.findall(b))}\tfeats_replaced={sf}\thazards_replaced={sh}")
