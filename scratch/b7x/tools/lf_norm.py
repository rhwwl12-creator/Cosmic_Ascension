"""Turn CRLF back into LF in a checkout's tracked text files (index LF, disk CRLF).

Usage: python -I -X utf8 lf_norm.py <tree-root> [--check]

Git for Windows writes CRLF for text files when it touches them (core.eol is native here), while the
project keeps LF on disk. Run this after any git command that wrote files in the tree (cherry-pick,
checkout, reset, apply, rebase, stash pop) unless that command was given -c core.eol=lf.
--check only reports and exits 1 when a file still has CRLF. The 'M' lines that git status shows
afterwards are stat noise (numstat 0/0), the same as in the main checkout.
"""
import subprocess
import sys


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    root = sys.argv[1]
    check = "--check" in sys.argv[2:]
    out = subprocess.run(["git", "-C", root, "-c", "core.quotepath=false", "ls-files", "--eol", "-z"],
                         capture_output=True, check=True).stdout
    bad = []
    for rec in out.split(b"\0"):
        if not rec:
            continue
        meta, _, path = rec.partition(b"\t")
        fields = meta.split()
        if len(fields) < 2 or fields[0] != b"i/lf" or fields[1] != b"w/crlf":
            continue
        rel = path.decode("utf-8")
        full = root.rstrip("/\\") + "/" + rel
        data = open(full, "rb").read()
        fixed = data.replace(b"\r\n", b"\n")
        if fixed == data:
            continue
        bad.append(rel)
        if not check:
            open(full, "wb").write(fixed)
    verb = "still CRLF" if check else "converted"
    print("%s %d file(s)" % (verb, len(bad)))
    for rel in bad[:50]:
        print("  " + rel)
    if check and bad:
        sys.exit(1)


if __name__ == "__main__":
    main()
