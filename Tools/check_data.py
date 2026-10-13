"""Check Tools/data against Tools/data/schema.json without the editor.

Cosmic Ascension - data layer helper (guideline.md section 7.3, 9.3 Data.Integrity).

  python Tools/check_data.py

Prints one line per problem and exits with 1 when there is one. Tools/make_data_tables.py runs the same check before it
builds the assets, so this is only the quick way to see it while writing rows.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import cosmic_data_schema  # noqa: E402  (the import needs the path above)


def main():
    problems = cosmic_data_schema.check_project()
    for problem in problems:
        print(problem)
    print("check_data: %d problem(s)" % len(problems))
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
