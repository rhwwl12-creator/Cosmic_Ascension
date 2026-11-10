"""B7x decision 3 (answered 2026-11-10): rename eight brick leaves in the code, data, config, schema and tests of a tree.

Usage:  python -I -X utf8 rename_leaves.py <tree-root> [--dry-run] [--sites <file>]

Run it at the re-stack (main after B6b plus the re-stacked branch commits), before [B-7x1] lands: the tag names land in
saves, so this is the last moment they can change. Run it again after each commit that ports aa09543's data
([B-7x5s], [B-7x5c]) and on the uncommitted data copy [B-7x4m] measures: the draft data still spells the old leaves.

  UnseenLaw -> KeptLaw        SilenceFeed -> IdleFeed (knob BrickQuietInsightPerTick -> BrickIdleInsightPerTick,
  PlanckMicro -> FoldedSmall                            function IsQuietOf -> IsIdleOf)
  DysonBrain -> ShutSphere    KesslerTapestry -> DustBand     AbyssalMaw -> InwardHunger
  StarShepherd -> StarTow     HermitMonolith -> ClosedPort

What it edits: text files under Source/, Config/ and Tools/ only. Never docs/ (edited by hand), never Content/ or any
binary file, never Tools/data/generated.sha1 (written by make_data_tables.py alone). Each file is decoded as UTF-8
(strict; a BOM stays, CRLF/LF stay) and only the matched tokens change, so every other byte is kept.

Rules, in order:
  1. TOKENS - an explicit table of old -> new identifiers (tags, accessors, knob, function, bare leaf names), matched
     whole: no letter, digit or '_' may touch either end ('.' and quotes are edges, so DT_Rules.BrickQuietInsightPerTick
     and TagName=\"Brick.Diet.SilenceFeed\" are found).
  2. PHRASES - fixed English phrases of comments, DevComments and test messages that spell an old leaf in words
     ("the Kessler tapestry", "an unseen law", the ini's "leaf rename to QuietFeed proposed" clause ...).
  3. FILE PHRASES - the word "quiet" for the SilenceFeed tick, only in the files of the brick code, and the local
     variable Quiet of CosmicExistenceBricks.cpp. 'Quiet' elsewhere (Revelation.Quiet, Digest.Quiet, QuietHazards,
     FindRowQuiet, test boards' "Quiet rules" ...) is never touched; the run aborts if a count of Revelation.Quiet or
     Digest.Quiet changes in any file.

Then it scans the whole scope again (case-sensitive substrings of every old leaf and of QuietFeed/BrickQuietInsight/
IsQuietOf, and case-insensitive prose words such as 'kessler', 'planck', 'unseen law') and exits 1 when anything is left
outside the ALLOW list it prints. File names are scanned too, and snake_case and kebab-case spellings of the old names
(unseen_law, star-shepherd ...) count as leftovers. A second run reports 0 changes. Occurrences under docs/ and the root .md files are
listed as information only (they are edited by hand).

Follow-ups it prints and never does itself:
  - docs/30_CONFIG_REFERENCE.md is generated from DT_Rules.json and Source: when it still names the old knob, run
    python -I Tools/make_config_reference.py
  - the .uasset tables and Tools/data/generated.sha1 come from make_data_tables.py inside the editor: when a file under
    Tools/data changed (or still differs from its stamp), run Tools\\run_checks.ps1 -Tier full -Data (editor closed).
"""
import hashlib
import os
import re
import sys

SCOPE_DIRS = ("Source", "Config", "Tools")
INFO_DIRS = ("docs",)
INFO_ROOT_FILES = ("CLAUDE.md", "STATUS.md", "WORKPLAN.md")
SKIP_FILES = {"Tools/data/generated.sha1"}
BINARY_EXT = {".uasset", ".umap", ".png", ".jpg", ".jpeg", ".gif", ".bmp", ".ico", ".tga", ".dds", ".wav", ".ogg",
              ".mp3", ".mp4", ".ttf", ".otf", ".dll", ".exe", ".pdb", ".lib", ".obj", ".zip", ".7z", ".pak", ".utoc",
              ".ucas", ".pyc", ".sha1"}

# (old, new) - the exact token table. Longest first inside the regex, so a whole tag counts under its own line.
LEAVES = [
    ("Form", "UnseenLaw", "KeptLaw"),
    ("Diet", "SilenceFeed", "IdleFeed"),
    ("Space", "PlanckMicro", "FoldedSmall"),
    ("Form", "DysonBrain", "ShutSphere"),
    ("Mutation", "KesslerTapestry", "DustBand"),
    ("Mutation", "AbyssalMaw", "InwardHunger"),
    ("Mutation", "StarShepherd", "StarTow"),
    ("Space", "HermitMonolith", "ClosedPort"),
]
TOKENS = []
for axis, old, new in LEAVES:
    TOKENS.append(("Brick.%s.%s" % (axis, old), "Brick.%s.%s" % (axis, new)))       # gameplay tag (ini, JSON, TEXT())
    TOKENS.append(("Brick%s%s" % (axis, old), "Brick%s%s" % (axis, new)))           # CosmicTags:: accessor
for axis, old, new in LEAVES:
    TOKENS.append((old, new))                                                         # bare leaf name (comments, lists)
TOKENS += [
    ("BrickQuietInsightPerTick", "BrickIdleInsightPerTick"),                          # DT_Rules knob / FCosmicRulesDef
    ("IsQuietOf", "IsIdleOf"),                                                        # CosmicBricks:: function
]

# Phrases anywhere in scope (case-sensitive, literal).
PHRASES = [
    ("; leaf rename to QuietFeed proposed before any save", ""),   # ini DevComment of Brick.Diet.SilenceFeed (decided)
    ("feeds on quiet ticks", "feeds on idle ticks"),
    ("the unseen law's kept edicts", "the kept law's edicts"),
    ("an unseen law", "a kept law"),
    ("the unseen law", "the kept law"),
    ("The unseen law", "The kept law"),
    ("the Kessler tapestry", "the dust band"),
    ("The tapestry needs", "The dust band needs"),
    ("the abyssal maw", "the inward hunger"),
    ("is a star shepherd", "carries the star tow"),
    ("the star shepherd", "the star tow"),
    ("the silence feeder", "the idle feeder"),
    ('TEXT("The hermit")', 'TEXT("The closed port")'),
]

# Phrases only in the brick files (matched by the path's end). The SilenceFeed tick was called "quiet".
BRICK_FILES = ("Sim/CosmicExistenceBricks.h", "Sim/CosmicExistenceBricks.cpp", "Data/CosmicDefinitionRows.h",
               "Tests/CosmicExistenceBricksTests.cpp")
FILE_PHRASES = [
    (BRICK_FILES, "a quiet tick", "an idle tick"),
    (BRICK_FILES, "A quiet tick", "An idle tick"),
    (BRICK_FILES, "the quiet tick", "the idle tick"),
    (BRICK_FILES, "The quiet tick", "The idle tick"),
    (("Tests/CosmicExistenceBricksTests.cpp",), "The board is quiet for people", "The board is idle for people"),
    (("Tests/CosmicExistenceBricksTests.cpp",), "breaks the quiet", "ends the idle tick"),
    (("Tests/CosmicExistenceBricksTests.cpp",), "(quiet again)", "(idle again)"),
]
# The local variable Quiet of the income function (never Revelation.Quiet: a '.' before it is an edge too).
FILE_REGEXES = [
    (("Sim/CosmicExistenceBricks.cpp",), r"(?<![A-Za-z0-9_.])Quiet(?![A-Za-z0-9_])", "Idle"),
]
GUARDS = ("Revelation.Quiet", "Digest.Quiet")

# What may not remain: case-sensitive substrings, then case-insensitive words.
LEFTOVER_EXACT = [old for _, old, _ in LEAVES] + ["QuietFeed", "BrickQuietInsight", "IsQuietOf"]
LEFTOVER_WORDS = re.compile(r"kessler|planck|tapestry|abyssal[ _-]?maw|star[ _-]?shepherd|hermit[ _-]?monolith|"
                            r"dyson[ _-]?brain|unseen[ _-]?law|silence[ _-]?feed|quiet[ _-]?feed|quiet[ _-]tick", re.I)
# (path suffix, substring of the line) pairs that may keep a leftover - printed on every run.
ALLOW = [
]

LONG = "\\\\?\\"
EDGE_L = r"(?<![A-Za-z0-9_])"
EDGE_R = r"(?![A-Za-z0-9_])"
TOKEN_RE = re.compile(EDGE_L + "(" + "|".join(re.escape(o) for o, _ in sorted(TOKENS, key=lambda t: -len(t[0]))) + ")" + EDGE_R)
TOKEN_MAP = dict(TOKENS)


def is_binary(path, data):
    return os.path.splitext(path)[1].lower() in BINARY_EXT or b"\x00" in data[:65536]


def walk(root, dirs):
    for top in dirs:
        base = os.path.join(root, top)
        if not os.path.isdir(base):
            continue
        for dirpath, dirnames, names in os.walk(base):
            dirnames[:] = sorted(d for d in dirnames if d not in (".git", "Intermediate", "Binaries", "Saved", "__pycache__"))
            for name in sorted(names):
                full = os.path.join(dirpath, name)
                yield full, os.path.relpath(full, root).replace("\\", "/")


def line_of(text, pos):
    return text.count("\n", 0, pos) + 1


def rename_text(rel, text, sites):
    """Returns (new text, {rule: count}). Records (rel, line, old, new) in sites, line numbers of the input text."""
    counts = {}

    def note(rule, n=1):
        counts[rule] = counts.get(rule, 0) + n

    # 1. tokens - one pass so lines are those of the input.
    out, last = [], 0
    for m in TOKEN_RE.finditer(text):
        old = m.group(1)
        new = TOKEN_MAP[old]
        sites.append((rel, line_of(text, m.start()), old, new))
        note(old + " -> " + new)
        out.append(text[last:m.start()]); out.append(new); last = m.end()
    out.append(text[last:])
    after_tokens = "".join(out)

    # 2./3. phrases and file rules, applied on the token result; lines are mapped by searching the input where possible.
    rules = [(None, a, b) for a, b in PHRASES] + [(files, a, b) for files, a, b in FILE_PHRASES]
    cur = after_tokens
    for files, old, new in rules:
        if files is not None and not rel.endswith(files):
            continue
        if old not in cur:
            continue
        n = cur.count(old)
        pos = -1
        for _ in range(n):
            pos = cur.find(old, pos + 1)
            sites.append((rel, line_of(cur, pos), old, new if new else "(removed)"))
        cur = cur.replace(old, new)
        note('"%s" -> "%s"' % (old, new), n)
    for files, pattern, new in FILE_REGEXES:
        if not rel.endswith(files):
            continue
        rx = re.compile(pattern)
        hits = list(rx.finditer(cur))
        for m in hits:
            sites.append((rel, line_of(cur, m.start()), m.group(0) + " (local)", new))
        if hits:
            cur = rx.sub(new, cur)
            note("local %s -> %s" % (pattern, new), len(hits))
    return cur, counts


def leftovers(rel, text):
    found = []
    for no, line in enumerate(text.split("\n"), 1):
        hits = [w for w in LEFTOVER_EXACT if w in line] + [m.group(0) for m in LEFTOVER_WORDS.finditer(line)]
        if hits:
            allowed = any(rel.endswith(p) and s in line for p, s in ALLOW)
            found.append((rel, no, sorted(set(hits)), line.strip()[:160], allowed))
    return found


def main(argv):
    args = [a for a in argv[1:]]
    dry = "--dry-run" in args
    sites_path = None
    if "--sites" in args:
        i = args.index("--sites"); sites_path = args[i + 1]; del args[i:i + 2]
    args = [a for a in args if a != "--dry-run"]
    if len(args) != 1 or not os.path.isdir(args[0]):
        print(__doc__.split("\n\n")[1]); return 2
    root = os.path.abspath(args[0])
    shown = root
    if os.name == "nt" and not root.startswith(LONG):
        root = LONG + root  # long paths: a tree deep in a temp folder passes 260 characters
    for need in ("Source", "Config", "Tools"):
        if not os.path.isdir(os.path.join(root, need)):
            print("rename_leaves: %s has no %s/ - not a project tree" % (shown, need)); return 2

    print("rename_leaves: %s%s" % (shown, " (dry run - nothing written)" if dry else ""))
    print("token table (%d):" % len(TOKENS))
    for o, n in TOKENS:
        print("  %s -> %s" % (o, n))
    print("allow list (%d)%s" % (len(ALLOW), ":" if ALLOW else " - empty: no old name may stay under Source/, Config/, Tools/"))
    for p, s in ALLOW:
        print("  %s : %s" % (p, s))

    sites, total, changed_files, data_changed, skipped_binary = [], 0, [], False, []
    left = []
    for full, rel in walk(root, SCOPE_DIRS):
        path_hits = [w for w in LEFTOVER_EXACT if w in rel] + [m.group(0) for m in LEFTOVER_WORDS.finditer(rel)]
        if path_hits:
            left.append((rel, 0, sorted(set(path_hits)), "(file name - rename by hand with git mv)",
                         any(rel.endswith(p) for p, _ in ALLOW)))
        if rel in SKIP_FILES:
            continue
        data = open(full, "rb").read()
        if is_binary(rel, data):
            if any(w.encode() in data for w in LEFTOVER_EXACT):
                skipped_binary.append(rel)
            continue
        try:
            text = data.decode("utf-8")
        except UnicodeDecodeError:
            if any(w.encode() in data for w in LEFTOVER_EXACT):
                print("  ! %s is not UTF-8 and holds an old name - fix it by hand" % rel)
                left.append((rel, 0, ["(not UTF-8)"], "", False))
            continue
        new_text, counts = rename_text(rel, text, sites)
        for g in GUARDS:
            if text.count(g) != new_text.count(g):
                print("rename_leaves: ABORT - %s count of %s changed in %s" % (g, g, rel)); return 3
        n = sum(counts.values())
        if n:
            total += n
            changed_files.append(rel)
            print("  %-70s %4d  (%s)" % (rel, n, ", ".join("%s x%d" % (k, v) for k, v in sorted(counts.items()))))
            if rel.startswith("Tools/data/"):
                data_changed = True
            if not dry:
                out = new_text.encode("utf-8")
                with open(full, "wb") as handle:
                    handle.write(out)
        left += leftovers(rel, new_text)

    print("changes: %d in %d files" % (total, len(changed_files)))
    if sites_path:
        with open(sites_path, "w", encoding="utf-8", newline="\n") as handle:
            handle.write("# rename_leaves.py sites in %s (line numbers of the tree before the run)\n" % shown.replace("\\", "/"))
            for rel, no, old, new in sorted(sites, key=lambda s: (s[0], s[1])):
                handle.write("%s:%d  %s -> %s\n" % (rel, no, old, new))
        print("sites: %d written to %s" % (len(sites), sites_path))

    if skipped_binary:
        print("binary files holding an old name (rebuilt from the sources, not edited):")
        for rel in skipped_binary:
            print("  %s" % rel)

    # Information only: what the doc editors fix by hand.
    info = []
    for full, rel in list(walk(root, INFO_DIRS)) + [(os.path.join(root, f), f) for f in INFO_ROOT_FILES if os.path.isfile(os.path.join(root, f))]:
        data = open(full, "rb").read()
        if is_binary(rel, data):
            continue
        text = data.decode("utf-8", errors="replace")
        for w in LEFTOVER_EXACT + ["BrickQuietInsightPerTick"]:
            if w in text:
                info.append(rel); break
    if info:
        print("old names outside the scope (docs - edited by hand, not by this script): %d files" % len(info))
        for rel in info:
            print("  %s" % rel)

    # Follow-ups.
    follow = []
    ref = os.path.join(root, "docs", "30_CONFIG_REFERENCE.md")
    if os.path.isfile(ref) and "BrickQuietInsightPerTick" in open(ref, encoding="utf-8").read():
        follow.append("python -I Tools/make_config_reference.py      (docs/30_CONFIG_REFERENCE.md still names BrickQuietInsightPerTick; "
                      "then python -I Tools/make_config_reference.py --check)")
    stamp = os.path.join(root, "Tools", "data", "generated.sha1")
    stale = []
    if os.path.isfile(stamp):
        for line in open(stamp, encoding="ascii").read().splitlines():
            parts = line.split(" ")
            if len(parts) != 2:
                continue
            src = os.path.normpath(os.path.join(root, "Tools", "data", parts[0]))
            if os.path.isfile(src) and hashlib.sha1(open(src, "rb").read().replace(b"\r", b"")).hexdigest() != parts[1]:
                stale.append(parts[0])
    if data_changed or stale:
        follow.append("powershell -ExecutionPolicy Bypass -File Tools\\run_checks.ps1 -Tier full -Data      (editor closed: "
                      "make_data_tables.py rebuilds the DT_ .uasset files and Tools/data/generated.sha1 - stale now: %s)"
                      % (", ".join(stale) if stale else "after this run"))
    if follow:
        print("follow-up (not done by this script):")
        for f in follow:
            print("  " + f)

    bad = [l for l in left if not l[4]]
    allowed = [l for l in left if l[4]]
    if allowed:
        print("left by the allow list: %d" % len(allowed))
        for rel, no, hits, line, _ in allowed:
            print("  %s:%d  %s" % (rel, no, "/".join(hits)))
    if bad:
        print("OLD NAMES LEFT under Source/, Config/, Tools/ (%d lines) - fix the table or the files:" % len(bad))
        for rel, no, hits, line, _ in bad:
            print("  %s:%d  [%s]  %s" % (rel, no, "/".join(hits), line))
        return 1
    print("no old name left under Source/, Config/, Tools/%s" % (" (after a real run)" if dry else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
