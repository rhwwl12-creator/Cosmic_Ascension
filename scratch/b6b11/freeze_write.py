import hashlib, pathlib, sys
SP = pathlib.Path(r"C:\Users\rhwwl\AppData\Local\Temp\claude\c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension\37de2919-0c44-45d8-94cd-4dcf0860263e\scratchpad\b6b11")
N = pathlib.Path(r"C:\Users\rhwwl\OneDrive\Documents\Unreal Projects\Cosmic_Ascension\docs\60_DECISIONS\headers\B6b_upper_rungs_and_the_race_notes.md")
mode = sys.argv[1]
if mode == "write":
    item = (SP / "frozen_item_draft.txt").read_bytes().decode("utf-8").replace("\r", "")
    item = item.rstrip("\n") + "\n"
    assert item.startswith("- **측정**") and item.count("\n") == 1
    text = N.read_bytes().decode("utf-8")
    assert "\r" not in text and "**`[B-6b11]`**" not in text
    # the [B-6b10] block is the last block of section 5 and runs to EOF
    i10 = text.index("\n**`[B-6b10]`**(")
    assert "\n**`[" not in text[i10 + 1:] and "\n## " not in text[i10 + 1:]
    assert text.endswith("\n")
    head = "**`[B-6b11]`**(2026-11-10 — 다시 잼(묶음 C)):\n"
    new = text + "\n" + head + item
    N.write_bytes(new.encode("utf-8"))
    (SP / "frozen_4_3.md").write_bytes(item.encode("utf-8"))
    print("written")
# verify
fz = (SP / "frozen_4_3.md").read_bytes()
nb = N.read_bytes()
assert not fz.startswith(b"\xef\xbb\xbf") and b"\r" not in fz and fz.endswith(b"\n") and not fz.endswith(b"\n\n")
assert b"\r" not in nb, "CR in N"
ft = fz.decode("utf-8"); nt = nb.decode("utf-8")
assert nt.count(ft) == 1, "frozen text not verbatim once in N"
lines = nt.split("\n")
hl = [k + 1 for k, l in enumerate(lines) if l.startswith("**`[B-6b11]`**(")]
print("frozen bytes", len(fz), "sha1", hashlib.sha1(fz).hexdigest())
print("N bytes", len(nb), "block head line", hl, "item line", hl[0] + 1, "N lines", len(lines) - 1)
print("verbatim in N: yes; CR in N: 0")
