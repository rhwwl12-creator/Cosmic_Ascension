"""Make or re-take a one-hunk mutation patch against the working tree (LF), from the project root.
  python -I mkpatch.py new <out.patch> <file> <old-text-file> <new-text-file>
  python -I mkpatch.py retake <patch> ...   (re-takes the context of each patch from its own before/after text)
The before text must occur exactly once in the file."""
import difflib
import sys

sys.stdout.reconfigure(encoding="utf-8")


def read(path):
    with open(path, "rb") as f:
        return f.read().decode("utf-8").replace("\r\n", "\n")


def write_patch(out, path, before, after):
    text = read(path)
    if text.count(before) != 1:
        raise SystemExit("%s: before text found %d times in %s" % (out, text.count(before), path))
    mutated = text.replace(before, after)
    diff = list(difflib.unified_diff(text.splitlines(keepends=True), mutated.splitlines(keepends=True),
                                     "a/" + path, "b/" + path, n=3))
    if not diff:
        raise SystemExit("%s: no change" % out)
    # difflib puts a tab-less header; drop the trailing spaces of the --- / +++ lines it may write
    body = ["diff --git a/%s b/%s\n" % (path, path)] + [l.rstrip(" \t\n") + "\n" if l.startswith(("---", "+++")) else l for l in diff]
    with open(out, "wb") as f:
        f.write("".join(body).encode("utf-8"))
    print(out, "written", [l.strip() for l in body if l.startswith("@@")])


def parse(patch):
    lines = read(patch).split("\n")
    path = None
    before, after = [], []
    hunks = 0
    for line in lines:
        if line.startswith("+++ b/"):
            path = line[6:]
        elif line.startswith("---") or line.startswith("diff --git"):
            continue
        elif line.startswith("@@"):
            hunks += 1
        elif hunks:
            if line.startswith(" "):
                before.append(line[1:]); after.append(line[1:])
            elif line.startswith("-"):
                before.append(line[1:])
            elif line.startswith("+"):
                after.append(line[1:])
    if hunks != 1:
        raise SystemExit("%s: %d hunks" % (patch, hunks))
    return path, "\n".join(before) + "\n", "\n".join(after) + "\n"


if __name__ == "__main__" and sys.argv[1] == "new":
    out, path, old_file, new_file = sys.argv[2:6]
    write_patch(out, path, read(old_file), read(new_file))
elif __name__ == "__main__" and sys.argv[1] == "retake":
    for patch in sys.argv[2:]:
        path, before, after = parse(patch)
        write_patch(patch, path, before, after)
