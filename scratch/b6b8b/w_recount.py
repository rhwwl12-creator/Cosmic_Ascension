"""W recount for [B-6b8b]: a Python copy of TheodicyProblems (Tests/CosmicDataTests.cpp, HEAD cef8cc7 :2477-2568)
for reading numbers only - the C++ invariant stays the judge. Usage: python -I w_recount.py <data dir> [share]
Prints, per hazard row: share a slot, longest stand (countdown + guard cap + edict bonuses + card delays), slots, stays."""
import json, math, os, sys

d = sys.argv[1]
share_arg = float(sys.argv[2]) if len(sys.argv) > 2 else None
L = lambda n: json.load(open(os.path.join(d, n + '.json'), encoding='utf-8'))
rules = L('DT_Rules')[0]
share_knob = rules.get('FleetDelayCountdownShare', 0.0) if share_arg is None else share_arg
cap = min(max(rules.get('FleetDelayCap', 100.0), 0.0), 100000.0)
step = max(rules.get('SphereEventWeightStep', 0.0), 0.0)
slot = max(1, int(rules.get('EventSlotTicks', 10)))
chance = max(rules.get('EventChancePerSlot', 0.0), 0.0)

def guard_cap(countdown):
    if not (share_knob > 0):
        return int(cap)
    by = share_knob * max(countdown, 0)
    return int(by if by < cap else cap)

def tag_matches(tag, parent):  # Tag.MatchesTag(parent)
    return tag == parent or tag.startswith(parent + '.')

trans = L('DT_Transcendents')
def upper_factor(cat):
    f = 1.0
    for t in trans:
        mods = t.get('EventWeightModifiers') or {}
        m = mods.get(cat)
        if m and m > 0:
            largest = max(t.get('MaxInfluencedSystems', 0), 1 if t.get('InitialInfluencedSystems', 0) > 0 else 0)
            most = min(3.0, m * (1.0 + step * max(0, largest)))
            f *= most if most > 1.0 else 1.0
    return f

def effects(row, key):
    return row.get(key) or []

events, edicts, cards = L('DT_Events'), L('DT_Edicts'), L('DT_Encounters')
for h in L('DT_Hazards'):
    tag = h.get('HazardTag')
    if not tag:
        continue
    s = 0.0
    for e in events:
        cat = e.get('Category', '')
        res = (not tag_matches(cat, 'Event.Grace')) and any(
            x.get('Type') == 'ResolveHazard' and (not x.get('Tag') or tag_matches(tag, x['Tag'])) for x in effects(e, 'Effects'))
        if res and e.get('Trigger', 'Lottery') != 'Lottery':
            s += 1.0
        elif res and e.get('BaseWeight', 0) > 0:
            s += e['BaseWeight'] * upper_factor(cat) * chance
    if not (s > 0):
        print(f"{h['Name']}: skipped (no resolving non-grace row)")
        continue
    cd = max(1, h.get('CountdownTicks', 0))
    g = guard_cap(h.get('CountdownTicks', 0))
    bonus = sum(max(0, (ed.get('HazardCountdownBonus') or {}).get(tag, 0)) for ed in edicts)
    card = 0
    for c in cards:
        most = 0.0
        for ch in c.get('Choices') or []:
            for x in ch.get('OnSuccess') or []:
                if x.get('Type') == 'DelayHazard' and x.get('Tag') and tag_matches(tag, x['Tag']) and x.get('Value', 0) > most:
                    most = x['Value']
        card += int(min(most, 100000.0))
    longest = cd + g + bonus + card
    slots = (longest + slot - 1) // slot
    stays = (1.0 - s) ** slots if s < 1 else 0.0
    w = (1 - 0.25 ** (1 / slots)) / chance if chance > 0 else float('nan')
    print(f"{h['Name']} ({tag}): share/slot {s:.4f}; longest {cd}+{g}+{bonus}+{card}={longest} ticks; slots {slots}; stays {stays:.4f}; "
          f"weight bound (one row, factor 1) {w:.4f}; throne lock {rules.get('ThroneLockTicks')} {'>=' if rules.get('ThroneLockTicks', 0) >= longest else '<'} {longest}")
