#!/usr/bin/env bash
# Editor-free checks (Linux / macOS / Git Bash). The Windows runner that also builds and runs the automation tests is
# Tools/run_checks.ps1; this script is the subset that needs only Python 3 (docs/40_DEV_GUIDE.md section 7).
set -u
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT" || exit 1
fail=0
run() { echo "== $1"; shift; "$@" || fail=1; }
run "data: schema check (Tools/check_data.py)"            python3 -I Tools/check_data.py
run "data: schema checker unit tests"                      python3 -I Tools/test_data_schema.py
run "layers: include direction, FMath::Rand, ASCII, knobs" python3 -I Tools/check_layers.py "$@"
run "docs: config reference up to date"                    python3 -I Tools/make_config_reference.py --check
run "docs: links and section map"                          python3 -I Tools/check_docs.py
if [ "$fail" -ne 0 ]; then echo "run_checks: FAILED"; exit 1; fi
echo "run_checks: all editor-free checks passed (build, automation tests, smoke and the commandlet need Windows: Tools/run_checks.ps1)"
