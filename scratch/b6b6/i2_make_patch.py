"""Make a unified diff patch (git apply form) from one file of the working tree and a list of exact (old, new) edits.
usage: i2_make_patch.py <out patch> <repo-relative path> <edits.py defining EDITS = [(old, new), ...]>"""
import difflib, os, runpy, sys

root = r"c:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension"
out, rel, edits_file = sys.argv[1:4]
src = open(os.path.join(root, rel), 'rb').read().decode('utf-8')
assert '\r' not in src
new = src
for old, rep in runpy.run_path(edits_file)['EDITS']:
    assert new.count(old) == 1, (old[:80], new.count(old))
    new = new.replace(old, rep)
diff = difflib.unified_diff(src.splitlines(True), new.splitlines(True), 'a/' + rel, 'b/' + rel, n=3)
text = 'diff --git a/%s b/%s\n' % (rel, rel) + ''.join(diff)
open(out, 'wb').write(text.encode('utf-8'))
print(text)
