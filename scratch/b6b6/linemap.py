"""Map cef8cc7 line numbers cited by the brief to HEAD line numbers via git diff hunks."""
import re, subprocess, sys

OLD = 'cef8cc7'
ROOT = sys.argv[1]
S = 'Source/Cosmic_Ascension/'

CITES = [
    # (path, [old line or (a,b)], what)
    ('Tools/data/DT_Hazards.json', [261, (145, 194), 9, 109, 152], 'crisis_rebellion end / pre_supernova row / Draw shares'),
    ('Tools/data/DT_Megaprojects.json', [73, 54, 67], 'project_binding end / stabilize cost / P_bind'),
    ('Tools/data/DT_Feats.json', [66, 7, 29, 50], 'feat_three_worlds end / TargetTier lines'),
    ('Tools/data/DT_Rules.json', [60], 'TierMultiplier'),
    ('Tools/data/schema.json', [28, 34, 44, 326, 450, (697, 697), (711, 713), (716, 718), (778, 788), 833, 847, 848, 852, 858, 939, 1205, 1228, 1236, 1239], 'schema cites'),
    ('Tools/cosmic_data_schema.py', [(284, 297), (600, 623), (617, 623), (1321, 1322)], 'read_sources / row rules / holds'),
    ('Tools/test_data_schema.py', [(276, 302), (1804, 1806), (1809, 1813), 1815, 3398, (3440, 3530), 3517, 3520, 3563], 'harness'),
    ('Tools/run_checks.ps1', [(35, 39), (48, 53), 76, 85, (102, 107), (116, 132), (121, 124), (129, 132), (139, 155), 157], 'runner'),
    ('Tools/mutations/README.md', [(5, 7), 792, (868, 873)], 'README'),
    ('Config/DefaultGameplayTags.ini', [20], 'extinction tag'),
    (S + 'Sim/GalaxySimulationSubsystem.cpp', [(1768, 1772), (2299, 2308), (2310, 2320), 2388, 2396, (2398, 2410), 2879, 2892, (2955, 2966), (2961, 2965), (2973, 2976)], 'sim'),
    (S + 'Data/CosmicDataTables.cpp', [(143, 148)], 'draw order'),
    (S + 'Data/CosmicDefinitionRows.h', [(300, 303)], 'feat by rung'),
    (S + 'Sim/CosmicHazards.cpp', [(746, 752), 751], 'KillsTheRulerOf'),
    (S + 'Sim/CosmicMegaprojects.cpp', [372], 'answer row never unprompted'),
    (S + 'Sim/CosmicDiplomacy.cpp', [(415, 426), (461, 478)], 'TreatyBlockOf'),
    (S + 'Events/CosmicEventSubsystem.cpp', [(486, 489)], 'completion per system'),
    (S + 'Ascension/AscensionSubsystem.cpp', [(196, 205)], 'HomeSystem = capital'),
    (S + 'World/CosmicRulerPolicy.cpp', [496], 'policy reads ruled rung'),
    (S + 'UI/CosmicSystemPanelWidget.cpp', [1042], 'ark button'),
    (S + 'Tests/CosmicDataTests.cpp', [(1215, 1218), (2195, 2204), (2195, 2235), (2660, 2702), (2660, 2711), (2663, 2666), (2669, 2672), (2693, 2711), (2712, 2779)], 'data tests'),
    (S + 'Tests/CosmicB6bRowNames.h', [1, (4, 8), 17, 21, 24], 'row names'),
    (S + 'Tests/CosmicTestHelpers.h', [(48, 49), (268, 271), (375, 387), (471, 486)], 'helpers'),
    (S + 'Tests/CosmicEndingTests.cpp', [(155, 158), 655, 784], ''),
    (S + 'Tests/CosmicKnowingTests.cpp', [1851, 1875], ''),
    (S + 'Tests/CosmicSuccessionTests.cpp', [610, 631], ''),
    (S + 'Tests/CosmicHazardTests.cpp', [2932, 2983], ''),
    (S + 'Tests/CosmicWandererTests.cpp', [1347], ''),
    (S + 'Tests/CosmicSettlementTests.cpp', [1006], ''),
    (S + 'Tests/CosmicAscensionTests.cpp', [951, 1160, (1760, 1780), (1772, 1777)], ''),
    (S + 'Tests/CosmicSimTests.cpp', [2260], ''),
    (S + 'Tests/CosmicDeathTests.cpp', [(106, 119), (797, 830), (798, 799)], ''),
    (S + 'Tests/CosmicUpperRungTests.cpp', [776], ''),
    (S + 'Tests/CosmicWarningLineTests.cpp', [(110, 111)], ''),
    (S + 'Tests/CosmicBalanceCommandlet.cpp', [544, 1525, (1596, 1597), 1968, 2025, (2275, 2288), (2364, 2367), 3575, 3610, 3890, (3958, 3992), (3959, 3961), 3966, (3976, 3980)], 'commandlet'),
    ('docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race.md', [10, 45, 46, 97, 99, 100, 246, 247, 325, 329, 330, 336, 338, 344, 358, 360, 369, 377, 380, 391, 392, 393, 403, 410, 421, 429], 'H'),
    ('docs/60_DECISIONS/headers/B6b_upper_rungs_and_the_race_notes.md', [348, 425, 434, 445, 469, 471, 525, 527, 552, 571, 597, 598, 622, 624, 659, 674, 675, 700, 704, 705, 709], 'N'),
    ('docs/60_DECISIONS/DECISIONS_LOG.md', [(34, 37)], ''),
    ('docs/10_DESIGN/crisis_and_transcendents.md', [24, 40, 67, 115, 139, 175, 177, 266], 'markers'),
    ('docs/10_DESIGN/pillars.md', [105, 108], 'markers'),
    ('docs/10_DESIGN/systems.md', [65, 91, 229, 409, 415, 417], 'markers'),
    ('docs/15_GLOSSARY.md', [151], 'markers'),
    ('docs/20_ARCHITECTURE.md', [109, 212, 289], 'markers'),
    ('docs/21_DATA_MODEL_REFERENCE.md', [629, 763, 973, 1266, (1458, 1463), 1518, 1520, 1585, 1658], 'markers'),
    ('docs/40_DEV_GUIDE.md', [25, 237, 238, 496], 'markers'),
    ('docs/70_CONTENT_GUIDE.md', [129], 'markers'),
    ('STATUS.md', [27, (30, 32)], 'lead file (read only)'),
]


def git(*a):
    return subprocess.run(['git', '-C', ROOT, *a], capture_output=True, text=True, encoding='utf-8', errors='replace').stdout


def hunks(path):
    out = git('diff', '-U0', '--no-color', OLD, 'HEAD', '--', path)
    hs = []
    for m in re.finditer(r'^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@', out, re.M):
        a, b, c, d = int(m.group(1)), int(m.group(2) or 1), int(m.group(3)), int(m.group(4) or 1)
        hs.append((a, b, c, d))
    return hs


def map_line(hs, n):
    """Return (new line, touched) for old line n."""
    delta = 0
    for a, b, c, d in hs:
        if b == 0:
            # pure insertion after old line a
            if a < n:
                delta += d
            continue
        if n < a:
            break
        if a <= n < a + b:
            return (c + min(n - a, max(d - 1, 0)) if d else c, True)
        delta += d - b
    return (n + delta, False)


def show(path, rev, lo, hi):
    txt = git('show', f'{rev}:{path}').split('\n')
    return [(i, txt[i - 1][:110] if 0 < i <= len(txt) else '<eof>') for i in range(lo, hi + 1)]


out = ['# [B-6b6] line map: brief (cef8cc7) -> HEAD (10205b6, [H\'\'])', '',
       'Each row: old line(s) at cef8cc7 -> new line(s) at HEAD; TOUCHED = the old line lies inside a hunk that [B-6b4]/[B-6b5] changed (re-check by text).',
       'Unchanged files keep every line. First and last text of the new range are shown for re-finding.', '']
for path, cites, what in CITES:
    hs = hunks(path)
    changed = bool(hs)
    out.append(f'## `{path}` {"(changed since cef8cc7: " + str(len(hs)) + " hunks)" if changed else "(unchanged since cef8cc7 - every line holds)"}' + (f' - {what}' if what else ''))
    for c in cites:
        lo, hi = (c, c) if isinstance(c, int) else c
        nlo, t1 = map_line(hs, lo)
        nhi, t2 = map_line(hs, hi)
        tag = ' TOUCHED' if (t1 or t2) else ''
        if not changed:
            first = show(path, 'HEAD', nlo, nlo)[0][1].strip()
            out.append(f'- :{lo}{"-" + str(hi) if hi != lo else ""} = :{nlo}{"-" + str(nhi) if nhi != nlo else ""} | `{first[:90]}`')
        else:
            first = show(path, 'HEAD', nlo, nlo)[0][1].strip()
            last = show(path, 'HEAD', nhi, nhi)[0][1].strip() if nhi != nlo else ''
            oldfirst = show(path, OLD, lo, lo)[0][1].strip()
            same = 'same text' if oldfirst == first else f'old text `{oldfirst[:70]}`'
            out.append(f'- :{lo}{"-" + str(hi) if hi != lo else ""} -> :{nlo}{"-" + str(nhi) if nhi != nlo else ""}{tag} | `{first[:90]}`' + (f' ... `{last[:60]}`' if last else '') + f' ({same})')
    out.append('')
open(sys.argv[2], 'w', encoding='utf-8', newline='\n').write('\n'.join(out) + '\n')
print('written', len(out))
