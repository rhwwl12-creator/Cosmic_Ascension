import json, os, sys
sys.stdout.reconfigure(encoding="utf-8")
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "review_integration.json")
SRC = os.path.join(HERE, "findings_src.json")
meta = json.load(open(OUT, encoding="utf-8"))
src = json.load(open(SRC, encoding="utf-8"))
meta["summary"] = src["summary"]
meta["findings"] = src["findings"]
json.dump(meta, open(OUT, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
print(len(meta["findings"]))
