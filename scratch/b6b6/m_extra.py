"""[B-6b6] extra readings of bundle A: the brake above cap 2 (who), grace keys, first movers per pair, the crowning per wanderer
pair, the ruled people's extinctions per run and the stock reading, AI us of the serial runner runs."""
import json, os, re, statistics, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bundleA as B

ROOT = r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension"
F = os.path.join(ROOT, "Saved", "Balance", "b6b_bundleA")
jobs = B.build_jobs(F)
P = {j["name"]: B.parse_log(j["log"]) for j in jobs}

print("== T4 civs most > 2: is every such run a ruled climb past two AIs (the brake counts the ruled people, never brakes it)?")
for cell in ("R", "wanderer"):
    bad, n = [], 0
    for j in jobs:
        if j["cell"] != cell or j["off"]:
            continue
        r = P[j["name"]]["race"]
        if r["t4civs"]["most"] > 2:
            n += 1
            a, ru = r["ai_t4"], r["ruled"]
            if not (a["t4_1"] >= 0 and a["t4_2"] >= 0 and ru["r4"] >= 0 and ru["r4"] > a["t4_2"]):
                bad.append((j["seed"], a["t4_1"], a["t4_2"], ru["r4"], r["t4civs"]))
    print(cell, "runs above 2:", n, "not explained by a ruled climb after two AIs:", bad)

print("== grace keys and the extinctions per people around them")
for j in jobs:
    if j["off"]:
        continue
    r = P[j["name"]]["race"]
    if r["extinction"]["grace"]:
        print(j["name"], r["extinction"]["grace_keys"], P[j["name"]]["hazards"])

print("== first mover per pair (first differing csv sample - T, and AI treaty first / first placement)")
for cell in ("R", "inputless", "wanderer"):
    d, mover = [], {"treaty": 0, "extinction": 0, "same": 0}
    for j in jobs:
        if j["cell"] != cell or j["off"]:
            continue
        on, off = P[j["name"]], P[j["name"] + "_off"]
        T = B.threshold_of(on["ending"], j["ticks"])
        _, a = B.read_csv_by_tick(j["csv"])
        _, b = B.read_csv_by_tick(j["csv"][:-4] + "_off.csv")
        fd = B.first_difference(a, b)
        d.append(fd - T if fd is not None else None)
        tr = on["race"]["treaties"]["ai_treaty_first"]
        rows = B.rows_first(j["rows"], "crisis_species_extinction")
        ex = rows[0][0] if rows else 10 ** 9
        trs = tr if tr >= 0 else 10 ** 9
        if trs < ex - 100:
            mover["treaty"] += 1
        elif ex < trs:
            mover["extinction"] += 1
        else:
            mover["same"] += 1
    dd = [x for x in d if x is not None]
    print(cell, "first diff - T:", B.dist(dd)["text"], "| first mover (treaty tick vs first extinction sample):", mover)

print("== wanderer crowning per pair (on vs off)")
lost, gained, moved = [], [], 0
for j in jobs:
    if j["cell"] != "wanderer" or j["off"]:
        continue
    on, off = P[j["name"]]["crowned"], P[j["name"] + "_off"]["crowned"]
    refused = P[j["name"]]["race"]["crown"]
    if on["tick"] < 0 <= off["tick"]:
        lost.append((j["seed"], off["tick"], refused["refused"], refused["first"]))
    elif off["tick"] < 0 <= on["tick"]:
        gained.append((j["seed"], on["tick"]))
    elif on["tick"] != off["tick"]:
        moved += 1
print("lost (seed, off tick, refused far above, first refusal):", lost)
print("gained:", gained, "moved:", moved)

print("== ruled extinctions per R run, ruled T4 - first placement")
per = []
for j in jobs:
    if j["cell"] != "R" or j["off"]:
        continue
    e = P[j["name"]]["race"]["extinction"]
    per.append(e["on_ruled"])
print("placements on ruled per run:", B.dist(per)["text"])

print("== serial runner AI us (Saved/Logs, this build) and the on/off 20,000 pairs of this stage")
for n in ("checks", "checks_ruler", "checks_wanderer"):
    p = B.parse_log(os.path.join(ROOT, "Saved", "Logs", n + ".log"))
    print("runner", n, p.get("ai_us"))
for n in ("checks", "checks_ruler", "checks_wanderer", "checks_20000", "checks_ruler_20000"):
    on = B.parse_log(os.path.join(ROOT, "Saved", "Balance", "b6b6_on", n + ".log")).get("ai_us")
    off = B.parse_log(os.path.join(ROOT, "Saved", "Balance", "b6b6_offA", n + ".log")).get("ai_us")
    base = B.parse_log(os.path.join(ROOT, "Saved", "Balance", "b6b6_base", n + ".log")).get("ai_us")
    print(n, "on", on, "off(new build)", off, "base(old build)", base)

print("== diplomacy (AI non-aggression) on vs off, conflict #129 transfers")
for cell in ("R", "inputless", "wanderer"):
    for off in (False, True):
        ds = [P[j["name"]]["diplomacy"] for j in jobs if j["cell"] == cell and j["off"] == off and P[j["name"]].get("diplomacy")]
        cs = [P[j["name"]]["conflict"] for j in jobs if j["cell"] == cell and j["off"] == off and P[j["name"]].get("conflict")]
        print(cell, "off" if off else "on", "nonaggression", B.dist([d["nonaggression"] for d in ds])["text"], "| signed", sum(d["signed"] for d in ds),
              "| sieges sum", sum(c["begun"] for c in cs), "taken sum", sum(c["taken"] for c in cs))
