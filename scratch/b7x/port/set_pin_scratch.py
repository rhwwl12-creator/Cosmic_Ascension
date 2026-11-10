"""Scratch-only helper: set the walked pin of test_every_rule_of_the_schema_can_fail in an EXPORT (never INT)."""
import sys
path = sys.argv[1] + "/Tools/test_data_schema.py"
b = open(path, "rb").read()
nl = b"\r\n" if b"\r\n" in b else b"\n"
old = b"        self.assertEqual(walked, 388, "
anchor = b"        # B7x [B-7x4]: 358 + 30 (the crossroads card: 6 columns x 4 lists; two event rules x 2; the scan face x 2) = 388." + nl
assert b.count(old) == 1 and b.count(anchor) == 1, "pin text not found exactly once"
b = b.replace(anchor, anchor + b"        # B7x [B-7x5c]: 388 + 77 (rule 4: 2 x 34; rule 5: 3; the three named brick lines: 6) = 465." + nl)
b = b.replace(old, b"        self.assertEqual(walked, 465, ")
open(path, "wb").write(b)
print("pin set to 465 in", path)
