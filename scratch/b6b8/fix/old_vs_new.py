import importlib.util, os, sys
sys.stdout.reconfigure(encoding="utf-8")
H = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
def load(n, p):
    s = importlib.util.spec_from_file_location(n, p); m = importlib.util.module_from_spec(s); s.loader.exec_module(m); return m
new = load("bnew", os.path.join(H, "bundleB.py")); old = load("bold", os.path.join(H, "fix", "bundleB.before.py"))
p = {"ending_x": {"kind": "OtherPeople", "tick": 9000, "civ": "civ_3"}, "race": {"ruled": {"r4": 7000}, "extinction": {"erased_ruled": 1}},
     "race_b": {"binding": {"tick": 8000, "by": "AI", "civ": "civ_3", "rung": 0}}, "ruled_erased": [("civ_7", 12000, 12100)]}
print("M2 later-life erasure: old", old.split_of(p)[0], "new", new.split_of(p)[0])
items = [({}, {"ending_x": {"kind": "None"}, "race_b": {"binding": {"tick": 8000, "by": "ruled", "civ": "c", "rung": 4, "no_result": 0, "locked": 0, "retaken": 0, "vessel_left": v}}}) for v in (-1, 0, 1)]
print("M3 vessel left: old", old.cell_extras(items)["binding"]["vessel left before T5"], "new", new.cell_extras(items)["binding"]["vessel left before T5"])
print("M4 median of 1..4: old", old.A.dist([1, 2, 3, 4])["median"], "new", new.dist([1, 2, 3, 4])["median"])
print("M6 key in old cell_extras:", "T4 civs at the record" in old.cell_extras(items))
