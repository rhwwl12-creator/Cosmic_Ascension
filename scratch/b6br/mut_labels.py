"""B6b [B-6br] brief section 5: check that a mutation run broke the assertions the table names, not some other one.

run_mutation.ps1 prints only the first 12 failing lines; this reads the whole log. For each patch it greps the run's log
(the newer of <logs>/Mut_<patch>.log (run_mutation.ps1) and <logs>/Mutation_<patch>.log (run_mutations2.py)) for
every expected label of mut_labels.json (the message id of brief section 3.1 plus the accessor or row),
looking only at failing-assertion lines (lines holding 'Error:'). The test must have failed (Result={Fail}).

Prints one PASS/FAIL line per patch (with the missing labels), and an INFO line with the ids that also failed but are
not in the table (record only - not a failure). Exit 0 when every patch passes, 1 on any FAIL, 2 on a usage error.

Usage: python -I mut_labels.py [--root ROOT] [--logs DIR] [--json FILE] <patch> [<patch> ...] | all
Writes nothing. Stdlib only.
"""
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_ROOT = r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension"
ID_RE = re.compile(r"(?<![A-Za-z0-9])(ii'-\d+:|ii-\d+:|iii:|i-\d+:|b-\d+:)")


def usage(msg):
    print("mut_labels: " + msg, file=sys.stderr)
    print(__doc__.strip().splitlines()[-2], file=sys.stderr)
    sys.exit(2)


def parse(argv):
    opts = {"root": DEFAULT_ROOT, "logs": None, "json": os.path.join(HERE, "mut_labels.json")}
    patches = []
    it = iter(argv)
    for a in it:
        if a in ("--root", "--logs", "--json"):
            try:
                opts[a[2:]] = next(it)
            except StopIteration:
                usage("%s needs a value" % a)
        elif a.startswith("--"):
            usage("unknown option " + a)
        else:
            patches.append(a)
    if not patches:
        usage("no patch named")
    if opts["logs"] is None:
        opts["logs"] = os.path.join(opts["root"], "Saved", "Logs")
    return opts, patches


def find_log(logs, patch):
    # The newer of the two names: a patch run once under run_mutation.ps1 and again under run_mutations2.py (or the
    # other way round) is judged by its latest run, not by a stale log.
    c = [os.path.join(logs, n) for n in ("Mut_%s.log" % patch, "Mutation_%s.log" % patch)]
    c = [p for p in c if os.path.isfile(p)]
    return max(c, key=os.path.getmtime) if c else None


def check(patch, labels, logs):
    log = find_log(logs, patch)
    if not log:
        print("FAIL %s: no log (Mut_%s.log or Mutation_%s.log) in %s" % (patch, patch, patch, logs))
        return False
    with open(log, encoding="utf-8-sig", errors="replace") as f:
        lines = f.read().splitlines()
    results = re.findall(r"Test Completed\. Result=\{(\w+)\}", "\n".join(lines))
    errors = [l for l in lines if "Error:" in l]
    missing = [lab for lab in labels if not any(lab in e for e in errors)]
    failed = bool(results) and any(r != "Success" for r in results)
    ok = failed and not missing
    if not results:
        why = "no test result in the log"
    elif not failed:
        why = "the test passed (Result=%s)" % ",".join(results)
    else:
        why = None
    head = "PASS" if ok else "FAIL"
    detail = "%d/%d labels" % (len(labels) - len(missing), len(labels))
    if why:
        detail += "; " + why
    if missing:
        detail += "; missing: " + " | ".join(missing)
    print("%s %s: %s (%s)" % (head, patch, detail, os.path.basename(log)))
    expected_ids = {m.group(1) for lab in labels for m in [ID_RE.search(lab)] if m}
    extra = {}
    for e in errors:
        m = ID_RE.search(e)
        if m and not any(lab in e for lab in labels):
            extra[m.group(1)] = extra.get(m.group(1), 0) + 1
    if extra:
        print("INFO %s: also failed (not in the table): %s" % (
            patch, ", ".join("%s x%d%s" % (k, v, "" if k in expected_ids else " (new id)") for k, v in sorted(extra.items()))))
    return ok


def main():
    opts, patches = parse(sys.argv[1:])
    try:
        with open(opts["json"], encoding="utf-8") as f:
            table = json.load(f)
    except (OSError, ValueError) as e:
        usage("cannot read %s: %s" % (opts["json"], e))
    if patches == ["all"]:
        patches = list(table)
    unknown = [p for p in patches if p not in table]
    if unknown:
        usage("not in %s: %s" % (os.path.basename(opts["json"]), ", ".join(unknown)))
    good = [check(p, table[p], opts["logs"]) for p in patches]
    print("mut_labels: %s (%d/%d patches)" % ("PASS" if all(good) else "FAIL", sum(good), len(good)))
    return 0 if all(good) else 1


if __name__ == "__main__":
    sys.exit(main())
