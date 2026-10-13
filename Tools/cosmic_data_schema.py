"""Check the definition data under Tools/data against Tools/data/schema.json.

Cosmic Ascension - data layer helper (guideline.md section 7.3, 9.3 Data.Integrity, 10.4). Plain Python, no engine:
Tools/make_data_tables.py calls check_project() before it touches an asset, Tools/check_data.py runs the same check
from a shell, Tools/test_data_schema.py feeds every kind of rule a wrong input.

What a row looks like here is what the DataTable importer reads (see the head of make_data_tables.py): a list of row
objects with the row name under "Name", a single tag as a string, a tag container as export text
"(GameplayTags=((TagName=\"A.B\"),...))", a map as an object, a text as a string, an enumerator by its name.

The kinds of column, the options of each kind, the tests of a row rule and the keys of the schema are closed lists
(schema.json names the kinds and the tests). A kind, an option, a test or a key this file does not know is itself a
problem, never a silent pass: a misspelt "min" would otherwise switch its check off.
"""
import json
import math
import os
import re

SCHEMA_FILE = "schema.json"
TAG_NAME = re.compile(r'TagName="([^"]*)"')
TAG_ROW = re.compile(r'^\s*\+?GameplayTagList\s*=\s*\(.*?Tag\s*=\s*"([^"]+)"')
SLOT = re.compile(r"\{([^{}]*)\}")

# Kind of column -> the options a column of that kind may carry next to "kind".
COLUMN_OPTIONS = {
    "plain": (),
    "number": ("min", "max", "integer"),
    "numbers": ("len", "ascending", "min", "max", "integer"),
    "text": ("slots", "no_digits", "required"),
    "texts": ("min", "len_in", "empty_or_min", "unique", "slots", "no_digits", "single_variant"),
    "tag": ("root", "optional"),
    "tags": ("root",),
    "tag_list": ("root",),
    "tag_map": ("root", "min", "max", "integer"),
    "row": ("table", "optional", "target"),
    "row_map": ("table", "target", "min", "max", "integer"),
    "word_map": ("words", "min", "max", "integer"),
    "enum": ("enum",),
    "conditions": (),
    "effects": ("per_tick",),
}
COLUMN_KINDS = tuple(COLUMN_OPTIONS)
TESTS = ("is", "in", "gt", "ge", "lt", "le", "set", "min_len", "max_len", "each_contains", "lacks_type")
TOP_KEYS = ("version", "about", "tag_sources", "required_tags", "row_name_pattern", "words", "enums", "single_variant_rows",
            "tables", "conditions", "effects", "per_tick_effects")
TABLE_KEYS = ("struct", "names", "prefixes", "must_exist", "required_rows", "columns", "rules", "unique", "pairs")
RULE_KEYS = {"rules": ("why", "when", "then"), "unique": ("why", "when", "cols"), "pairs": ("why", "key", "value", "allowed")}
# What a condition or an effect may say about each of its three cells; "per_tick_only" is for effects.
USAGE_KEYS = {"tag": ("root", "optional"), "key": ("word", "row", "digits", "any", "optional"), "value": ("min", "max", "integer")}


def project_root():
    return os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def load_declared_tags(root, sources):
    """The tags the ini files declare, by name. A parent that is only implied by a child is not among them."""
    declared = set()
    for source in sources:
        with open(os.path.join(root, source), encoding="utf-8-sig") as handle:
            for line in handle:
                found = TAG_ROW.match(line)
                if found:
                    declared.add(found.group(1))
    return declared


def load_project(root=None):
    """Read schema.json, the tags and every table source (DT_*.json) of the folder. Returns (schema, tables, tags).

    Every table source, not only the ones the schema names: a source the schema does not declare is a problem."""
    root = root or project_root()
    data_dir = os.path.join(root, "Tools", "data")
    with open(os.path.join(data_dir, SCHEMA_FILE), encoding="utf-8-sig") as handle:
        schema = json.load(handle)
    tables = {}
    for file_name in sorted(os.listdir(data_dir)):
        if file_name.startswith("DT_") and file_name.endswith(".json"):
            with open(os.path.join(data_dir, file_name), encoding="utf-8-sig") as handle:
                tables[file_name[:-len(".json")]] = json.load(handle)
    return schema, tables, load_declared_tags(root, schema.get("tag_sources", []))


def check_project(root=None):
    """The problems of the sources of this project, as a list of lines. Empty = the data holds the schema."""
    schema, tables, tags = load_project(root)
    return check(schema, tables, tags)


def is_number(value):
    return isinstance(value, (int, float)) and not isinstance(value, bool)


def is_set(value):
    """A name, a tag, a text or a collection that holds something. A number counts as set."""
    if value is None:
        return False
    if isinstance(value, str):
        return value not in ("", "None") and not (value.startswith("(GameplayTags=") and not TAG_NAME.search(value))
    if isinstance(value, (list, dict)):
        return len(value) > 0
    return True


def tag_of(value):
    """A single tag as the importer takes it: a string, or an object with TagName."""
    if isinstance(value, dict) and "TagName" in value:
        value = value["TagName"]
    if value is None or value == "None":
        return ""
    return value


def objects(entries):
    """The entries that are objects. One that is not is reported by check_schema and read by nothing."""
    return [entry for entry in entries if isinstance(entry, dict)]


def length_of(value):
    if isinstance(value, str) and value.startswith("(GameplayTags="):
        return len(TAG_NAME.findall(value))
    if isinstance(value, (list, dict, str)):
        return len(value)
    return 0


class Checker:
    def __init__(self, schema, tables, tags):
        self.schema = schema
        self.tables = tables
        self.tags = set(tags)
        self.problems = []
        self.words = schema.get("words", {})
        self.rows = {}
        for table_name, rows in tables.items():
            if isinstance(rows, list):
                self.rows[table_name] = {row.get("Name"): row for row in rows if isinstance(row, dict)}

    def report(self, where, message):
        self.problems.append("%s: %s" % (where, message))

    # ---- values ----

    def check_number(self, where, value, spec):
        if not is_number(value) or not math.isfinite(value):
            self.report(where, "expected a finite number, found %r" % (value,))
            return
        if "min" in spec and value < spec["min"]:
            self.report(where, "%r is below %r" % (value, spec["min"]))
        if "max" in spec and value > spec["max"]:
            self.report(where, "%r is above %r" % (value, spec["max"]))
        if spec.get("integer") and float(value) != math.floor(value):
            self.report(where, "%r is not a whole number" % (value,))

    def check_tag(self, where, value, spec):
        name = tag_of(value)
        if not isinstance(name, str):
            self.report(where, "expected a tag name, found %r" % (value,))
            return
        if name == "":
            if not spec.get("optional"):
                self.report(where, "a tag is required here")
            return
        if name not in self.tags:
            self.report(where, "tag %s is not declared in the ini" % name)
        roots = spec.get("root")
        roots = [roots] if isinstance(roots, str) else (roots or [])
        if roots and not any(name.startswith(root) for root in roots):
            self.report(where, "tag %s is not under %s" % (name, " or ".join(roots)))

    def check_row_ref(self, where, value, spec):
        if value is None or value == "None":
            value = ""
        if not isinstance(value, str):
            self.report(where, "expected a row name, found %r" % (value,))
            return
        if value == "":
            if not spec.get("optional"):
                self.report(where, "a row of %s is required here" % spec["table"])
            return
        target = self.rows.get(spec["table"], {}).get(value)
        if target is None:
            self.report(where, "%s has no row %s" % (spec["table"], value))
            return
        for test in spec.get("target", []):
            if not self.holds(test, target, value):
                self.report(where, "row %s of %s does not hold %s" % (value, spec["table"], json.dumps(test)))

    def check_text(self, where, value, spec):
        if not isinstance(value, str):
            self.report(where, "expected a text, found %r" % (value,))
            return
        if spec.get("required") and value == "":
            self.report(where, "a text is required here")
        if spec.get("no_digits") and any(ch.isdigit() for ch in value):
            self.report(where, "a digit in a text that carries none: %s" % value)
        if "slots" in spec:
            for slot in SLOT.findall(value):
                # A slot may carry a format modifier after a bar, as in {Ruler}|hpp(...): the name is what is checked.
                if slot not in spec["slots"]:
                    self.report(where, "slot {%s} is not filled here (allowed: %s)" % (slot, ", ".join(spec["slots"]) or "none"))

    def check_usage(self, where, kind, item, per_tick):
        """One condition or effect against the usage table of its enumerator."""
        table = self.schema.get(kind, {})
        if not isinstance(item, dict):
            self.report(where, "expected an object, found %r" % (item,))
            return
        type_name = item.get("Type")
        usage = table.get(type_name)
        if usage is None:
            self.report(where, "%r is no enumerator of %s" % (type_name, kind))
            return
        if not isinstance(usage, dict):
            return
        where = "%s(%s)" % (where, type_name)

        tag = tag_of(item.get("Tag", ""))
        if "tag" in usage:
            self.check_tag(where + ".Tag", tag, usage["tag"])
        elif tag != "":
            self.report(where + ".Tag", "this type reads no Tag, found %s" % tag)

        key = item.get("Key", "")
        key = "" if key in (None, "None") else key
        key_spec = usage.get("key")
        if key_spec is None:
            if key != "":
                self.report(where + ".Key", "this type reads no Key, found %s" % key)
        elif key == "":
            if not key_spec.get("optional"):
                self.report(where + ".Key", "a Key is required here")
        elif "word" in key_spec:
            if key not in self.words.get(key_spec["word"], []):
                self.report(where + ".Key", "%s is none of %s" % (key, ", ".join(self.words.get(key_spec["word"], []))))
        elif "row" in key_spec:
            self.check_row_ref(where + ".Key", key, {"table": key_spec["row"], "optional": True})
        elif key_spec.get("digits"):
            if not (isinstance(key, str) and key.isascii() and key.isdigit() and len(key) <= 9 and int(key) > 0):
                self.report(where + ".Key", "%r is not a count of ticks (digits only, nine at most, above zero)" % (key,))
        elif not key_spec.get("any"):
            self.report(where + ".Key", "the schema says nothing about this Key: %s" % json.dumps(key_spec))

        value = item.get("Value", 0)
        if "value" in usage:
            self.check_number(where + ".Value", value, usage["value"])
        elif not is_number(value) or value != 0:
            self.report(where + ".Value", "this type reads no Value, found %r" % (value,))

        if kind == "effects":
            duration = item.get("DurationTicks", 0)
            if not is_number(duration) or duration < 0 or (duration > 0 and not per_tick):
                self.report(where + ".DurationTicks", "%r: only a per-tick source may carry a duration" % (duration,))
            allowed = self.schema.get("per_tick_effects", [])
            if per_tick and type_name not in allowed:
                self.report(where, "a per-tick source applies only %s" % ", ".join(allowed))
            if not per_tick and usage.get("per_tick_only"):
                self.report(where, "this effect is applied by per-tick sources only")

    def check_column(self, where, value, spec, row_name):
        kind = spec.get("kind")
        if kind == "plain":
            return
        if kind == "number":
            self.check_number(where, value, spec)
        elif kind == "numbers":
            if not isinstance(value, list):
                self.report(where, "expected a list of numbers, found %r" % (value,))
                return
            if "len" in spec and len(value) != spec["len"]:
                self.report(where, "expected exactly %d entries, found %d" % (spec["len"], len(value)))
            for index, entry in enumerate(value):
                self.check_number("%s[%d]" % (where, index), entry, spec)
            if spec.get("ascending") and all(is_number(entry) for entry in value):
                if any(later <= earlier for earlier, later in zip(value, value[1:])):
                    self.report(where, "the entries do not rise: %r" % (value,))
        elif kind == "text":
            self.check_text(where, value, spec)
        elif kind == "texts":
            if not isinstance(value, list):
                self.report(where, "expected a list of texts, found %r" % (value,))
                return
            count = len(value)
            least = spec.get("min")
            if spec.get("single_variant") and row_name in self.schema.get("single_variant_rows", []):
                least = 1
            if least is not None and count < least:
                self.report(where, "expected %d entries or more, found %d" % (least, count))
            if "len_in" in spec and count not in spec["len_in"]:
                self.report(where, "expected %s entries, found %d" % (" or ".join(str(n) for n in spec["len_in"]), count))
            if "empty_or_min" in spec and 0 < count < spec["empty_or_min"]:
                self.report(where, "expected none or %d entries and more, found %d" % (spec["empty_or_min"], count))
            for index, entry in enumerate(value):
                self.check_text("%s[%d]" % (where, index), entry, dict(spec, required=True))
            if spec.get("unique") and len(set(e for e in value if isinstance(e, str))) != count:
                self.report(where, "an entry appears twice")
        elif kind == "tag":
            self.check_tag(where, value, spec)
        elif kind == "tags":
            if not isinstance(value, str) or (value != "" and not value.startswith("(GameplayTags=")):
                self.report(where, "expected the export text of a tag container, found %r" % (value,))
                return
            for name in TAG_NAME.findall(value):
                self.check_tag(where, name, dict(spec, optional=False))
        elif kind == "tag_list":
            if not isinstance(value, list):
                self.report(where, "expected a list of tags, found %r" % (value,))
                return
            for index, entry in enumerate(value):
                self.check_tag("%s[%d]" % (where, index), entry, dict(spec, optional=False))
        elif kind in ("tag_map", "row_map", "word_map"):
            if not isinstance(value, dict):
                self.report(where, "expected an object, found %r" % (value,))
                return
            for key, entry in value.items():
                here = "%s[%s]" % (where, key)
                if kind == "tag_map":
                    self.check_tag(here, key, dict(spec, optional=False))
                elif kind == "row_map":
                    self.check_row_ref(here, key, {"table": spec["table"], "target": spec.get("target", [])})
                elif key not in self.words.get(spec["words"], []):
                    self.report(here, "%s is none of %s" % (key, ", ".join(self.words.get(spec["words"], []))))
                self.check_number(here, entry, spec)
        elif kind == "row":
            self.check_row_ref(where, value, spec)
        elif kind == "enum":
            names = self.schema.get("enums", {}).get(spec.get("enum"), [])
            if value not in names:
                self.report(where, "%r is no enumerator of %s" % (value, spec.get("enum")))
        elif kind in ("conditions", "effects"):
            if not isinstance(value, list):
                self.report(where, "expected a list, found %r" % (value,))
                return
            for index, item in enumerate(value):
                self.check_usage("%s[%d]" % (where, index), kind, item, bool(spec.get("per_tick")))
        # A kind this checker does not know is reported by check_schema, once, with or without rows.

    # ---- row rules ----

    @staticmethod
    def test_is_known(test):
        if not isinstance(test, dict):
            return False
        if "row" in test:
            return len(test) == 1
        return "col" in test and len(test) == 2 and len([name for name in TESTS if name in test]) == 1

    def holds(self, test, row, row_name):
        """One test of a row rule. A test this checker does not know counts as failed (check_schema reports it)."""
        if not self.test_is_known(test):
            return False
        if "row" in test:
            return row_name == test["row"]
        known = [name for name in TESTS if name in test]
        value = row.get(test["col"])
        name = known[0]
        operand = test[name]
        plain = tag_of(value) if not isinstance(value, (list, dict)) else value
        if name == "is":
            return plain == operand or (is_number(plain) and is_number(operand) and float(plain) == float(operand))
        if name == "in":
            return plain in operand
        if name in ("gt", "ge", "lt", "le"):
            if not is_number(value):
                return False
            return {"gt": value > operand, "ge": value >= operand, "lt": value < operand, "le": value <= operand}[name]
        if name == "set":
            return is_set(value) == bool(operand)
        if name == "min_len":
            return length_of(value) >= operand
        if name == "max_len":
            return length_of(value) <= operand
        if name == "each_contains":
            return isinstance(value, list) and all(isinstance(entry, str) and operand in entry for entry in value)
        if name == "lacks_type":
            # A list of conditions or effects without one of the named enumerator.
            return isinstance(value, list) and all(not (isinstance(item, dict) and item.get("Type") == operand) for item in value)
        return False

    def all_hold(self, tests, row, row_name):
        return all(self.holds(test, row, row_name) for test in tests)

    # ---- tables ----

    def check_table(self, table_name, spec):
        columns = spec.get("columns")
        rows = self.tables.get(table_name)
        if rows is None:
            if spec.get("must_exist"):
                self.report(table_name, "Tools/data/%s.json is missing" % table_name)
            return
        if columns is None:
            self.report(table_name, "the schema declares no columns for this table yet: declare them with its first row")
            return
        if not isinstance(rows, list) or not all(isinstance(row, dict) and isinstance(row.get("Name"), str) for row in rows):
            self.report(table_name, "expected a list of row objects, each with a Name")
            return
        pattern = re.compile(self.schema.get("row_name_pattern", ".*"))
        names = [row["Name"] for row in rows]
        for name in names:
            where = "%s.%s" % (table_name, name)
            if not pattern.match(name):
                self.report(where, "the row name is not lower snake case")
            if "names" in spec:
                if name not in spec["names"]:
                    self.report(where, "this table holds only the rows %s" % ", ".join(spec["names"]))
            elif not any(name.startswith(prefix) for prefix in spec.get("prefixes", [])):
                self.report(where, "the row name does not start with %s" % " or ".join(spec.get("prefixes", [])))
            if names.count(name) > 1:
                self.report(where, "the row name appears %d times in the table" % names.count(name))
        for name in spec.get("required_rows", []):
            if name not in names:
                self.report(table_name, "the row %s, which the code calls by name, is missing" % name)

        for row in rows:
            name = row["Name"]
            for key in sorted(set(row) - set(columns) - {"Name"}):
                self.report("%s.%s.%s" % (table_name, name, key), "the schema does not declare this column")
            for column, column_spec in columns.items():
                where = "%s.%s.%s" % (table_name, name, column)
                if not isinstance(column_spec, dict):
                    continue
                if column not in row:
                    self.report(where, "the column is missing from the row")
                else:
                    self.check_column(where, row[column], column_spec, name)
            for rule in objects(spec.get("rules", [])):
                if self.all_hold(rule.get("when", []), row, name):
                    for test in rule.get("then", []):
                        if not self.holds(test, row, name):
                            self.report("%s.%s" % (table_name, name), "%s: it does not hold %s" % (rule.get("why", "rule"), json.dumps(test)))
            for pair in objects(spec.get("pairs", [])):
                allowed = self.schema.get(pair.get("allowed"))
                if not isinstance(allowed, dict):
                    continue
                key = tag_of(row.get(pair["key"]))
                value = tag_of(row.get(pair["value"]))
                if key in allowed and value not in allowed[key]:
                    self.report("%s.%s.%s" % (table_name, name, pair["value"]),
                                "%s: %s may tell %s, found %s" % (pair.get("why", "pair"), key, ", ".join(allowed[key]), value))
                elif key not in allowed and is_set(key):
                    self.report("%s.%s.%s" % (table_name, name, pair["key"]), "%s is not listed under %s" % (key, pair["allowed"]))

        for unique in objects(spec.get("unique", [])):
            seen = {}
            for row in rows:
                if not self.all_hold(unique.get("when", []), row, row["Name"]):
                    continue
                key = tuple(json.dumps(row.get(column), sort_keys=True) for column in unique["cols"])
                if key in seen:
                    self.report("%s.%s" % (table_name, row["Name"]),
                                "%s: the same %s as row %s" % (unique.get("why", "unique"), " and ".join(unique["cols"]), seen[key]))
                else:
                    seen[key] = row["Name"]

    def check_schema(self):
        """The schema itself, for every table with or without rows: a kind, a test or a name it leans on that is not
        there would otherwise pass in silence."""
        specs = self.schema.get("tables", {})
        effects = self.schema.get("effects", {})
        pair_lists = [pair.get("allowed") for spec in objects(specs.values()) for pair in objects(spec.get("pairs", []))]
        self.unknown_keys("schema", self.schema, TOP_KEYS + tuple(name for name in pair_lists if isinstance(name, str)))
        for tag in self.schema.get("required_tags", []):
            if tag not in self.tags:
                self.report("schema.required_tags", "tag %s is not declared in the ini" % tag)
        for name in self.schema.get("per_tick_effects", []):
            if name not in effects:
                self.report("schema.per_tick_effects", "%s is no effect of the schema" % name)
        for kind in ("conditions", "effects"):
            for type_name, usage in self.schema.get(kind, {}).items():
                where = "schema.%s.%s" % (kind, type_name)
                if not self.unknown_keys(where, usage, tuple(USAGE_KEYS) + (("per_tick_only",) if kind == "effects" else ())):
                    continue
                for cell, allowed in USAGE_KEYS.items():
                    self.unknown_keys("%s.%s" % (where, cell), usage.get(cell) or {}, allowed)
                key_spec = usage.get("key") or {}
                if "word" in key_spec and key_spec["word"] not in self.words:
                    self.report("schema.%s.%s" % (kind, type_name), "the word list %s is not in the schema" % key_spec["word"])
                if "row" in key_spec and key_spec["row"] not in specs:
                    self.report("schema.%s.%s" % (kind, type_name), "the table %s is not in the schema" % key_spec["row"])
        for table_name, spec in specs.items():
            tests = []
            if not self.unknown_keys("schema.%s" % table_name, spec, TABLE_KEYS):
                continue
            for list_name, allowed in RULE_KEYS.items():
                for entry in spec.get(list_name, []):
                    self.unknown_keys("schema.%s.%s" % (table_name, list_name), entry, allowed)
            for column, column_spec in (spec.get("columns") or {}).items():
                where = "schema.%s.%s" % (table_name, column)
                if not isinstance(column_spec, dict):
                    self.report(where, "expected an object with a kind, found %r" % (column_spec,))
                    continue
                kind = column_spec.get("kind")
                if kind not in COLUMN_KINDS:
                    self.report(where, "a kind this checker does not know: %r" % (kind,))
                else:
                    self.unknown_keys(where, column_spec, ("kind",) + COLUMN_OPTIONS[kind])
                if kind in ("row", "row_map") and column_spec.get("table") not in specs:
                    self.report(where, "the table %r is not in the schema" % (column_spec.get("table"),))
                if kind == "word_map" and column_spec.get("words") not in self.words:
                    self.report(where, "the word list %r is not in the schema" % (column_spec.get("words"),))
                if kind == "enum" and column_spec.get("enum") not in self.schema.get("enums", {}):
                    self.report(where, "the enum %r is not in the schema" % (column_spec.get("enum"),))
                # The tests of a target read the rows of the table the column points at.
                target = specs.get(column_spec.get("table"))
                self.check_tests(where, column_spec.get("target", []), target.get("columns") if isinstance(target, dict) else None)
            here = "schema.%s" % table_name
            columns = spec.get("columns")
            for rule in objects(spec.get("rules", [])):
                tests.extend(rule.get("when", []))
                tests.extend(rule.get("then", []))
            for unique in objects(spec.get("unique", [])):
                tests.extend(unique.get("when", []))
                for column in unique.get("cols", []):
                    if isinstance(columns, dict) and column not in columns:
                        self.report(here, "unique names the column %r, which the table does not declare" % (column,))
            self.check_tests(here, tests, columns)
            for pair in objects(spec.get("pairs", [])):
                allowed = self.schema.get(pair.get("allowed"))
                if not isinstance(allowed, dict):
                    self.report(here, "the list %r of a pair is not in the schema" % (pair.get("allowed"),))
                    continue
                for side in ("key", "value"):
                    column_spec = columns.get(pair.get(side)) if isinstance(columns, dict) else None
                    if not isinstance(column_spec, dict):
                        self.report(here, "a pair names the column %r, which the table does not declare" % (pair.get(side),))
                    elif column_spec.get("kind") == "tag":
                        # Both sides of the list are tag names: one the ini lacks would never match a row.
                        names = list(allowed) if side == "key" else [name for values in allowed.values() for name in values]
                        for name in sorted(set(names) - self.tags):
                            self.report("schema.%s" % pair.get("allowed"), "tag %s is not declared in the ini" % name)

    def check_tests(self, where, tests, columns):
        """The tests of a rule: each of a known shape, over a column the table declares, and against names that exist.
        A test over a misspelt column, tag or enumerator never holds, so its rule would be switched off in silence."""
        for test in tests:
            if not self.test_is_known(test):
                self.report(where, "a row test is 'row', or a column with exactly one of %s: %s" % (", ".join(TESTS), json.dumps(test)))
                continue
            if "row" in test or not isinstance(columns, dict):
                continue
            column_spec = columns.get(test["col"])
            if not isinstance(column_spec, dict):
                self.report(where, "a row test names the column %r, which the table does not declare" % (test["col"],))
                continue
            if "in" in test and not isinstance(test["in"], list):
                self.report(where, "the test 'in' needs a list: %s" % json.dumps(test))
                continue
            operands = test["in"] if "in" in test else [test["is"]] if "is" in test else []
            for operand in operands:
                if column_spec.get("kind") == "tag" and operand != "" and operand not in self.tags:
                    self.report(where, "a row test names the tag %s, which the ini does not declare" % (operand,))
                if column_spec.get("kind") == "enum" and operand not in self.schema.get("enums", {}).get(column_spec.get("enum"), []):
                    self.report(where, "a row test names %r, which is no enumerator of %s" % (operand, column_spec.get("enum")))

    def unknown_keys(self, where, entry, allowed):
        """A key of the schema that nothing reads: a misspelt option, or an option of another kind. False when the entry
        is no object at all."""
        if not isinstance(entry, dict):
            self.report(where, "expected an object, found %r" % (entry,))
            return False
        for key in sorted(set(entry) - set(allowed)):
            self.report(where, "%r is not read here (allowed: %s)" % (key, ", ".join(allowed) or "nothing"))
        return True

    def run(self):
        self.check_schema()
        specs = self.schema.get("tables", {})
        for table_name in sorted(self.tables):
            if table_name not in specs:
                self.report(table_name, "the schema does not declare this table")
        for table_name, spec in specs.items():
            if isinstance(spec, dict):
                self.check_table(table_name, spec)
        # A row name is one name in the whole game: it is saved without its table.
        owners = {}
        for table_name in specs:
            for row in self.tables.get(table_name) or []:
                if isinstance(row, dict) and isinstance(row.get("Name"), str):
                    owners.setdefault(row["Name"], []).append(table_name)
        for name, tables in sorted(owners.items()):
            if len(set(tables)) > 1:
                self.report(name, "the row name is used by %s" % " and ".join(sorted(set(tables))))
        return self.problems


def check(schema, tables, tags):
    """The problems of tables (asset name -> list of rows) under schema, with the declared tag names."""
    return Checker(schema, tables, tags).run()
