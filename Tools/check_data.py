"""Check Tools/data against Tools/data/schema.json without the editor.

Cosmic Ascension - data layer helper (guideline.md section 7.3, 9.3 Data.Integrity).

  python Tools/check_data.py

Prints one line per problem and exits with 1 when there is one. Tools/make_data_tables.py runs the same check before it
builds the assets, so this is only the quick way to see it while writing rows.
"""
import os
import sys

# No bytecode cache next to the tools: the checker is small and is read fresh every run.
sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import cosmic_data_schema  # noqa: E402  (the import needs the path above)


def main():
    # A problem line quotes the data, in any script. A console of another code page shows what it cannot print as an
    # escape and goes on, where it would otherwise stop at the first such line.
    sys.stdout.reconfigure(errors="backslashreplace")
    problems = cosmic_data_schema.check_project()
    for problem in problems:
        print(problem)
    print("check_data: %d problem(s)" % len(problems))
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
