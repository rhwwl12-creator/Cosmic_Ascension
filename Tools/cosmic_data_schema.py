"""Check the definition data under Tools/data against Tools/data/schema.json.

Cosmic Ascension - data layer helper (guideline.md section 7.3, 9.3 Data.Integrity, 10.4). Plain Python, no engine:
Tools/make_data_tables.py runs it before it touches an asset, Tools/check_data.py runs the same check from a shell,
Tools/test_data_schema.py feeds every check a wrong input.

Three steps, each stops the next when it finds a problem:
  1. Reading. A syntax error, a key written twice in one object, NaN or Infinity are problem lines with the file name.
  2. The schema itself. A kind, an option, a test or a key this file does not know; a value of the wrong type; a name a
     rule leans on that does not exist (a column, a tag, an enumerator, a row); a test that cannot be asked of the kind
     of its column. Any of these would switch a check off in silence, so each is a problem and the data is not looked
     at until the schema is right.
  3. The data, by every rule of the schema.

What a row looks like here is what the DataTable importer reads (see the head of make_data_tables.py): a list of row
objects with the row name under "Name", a single tag as a string, a tag container as export text
"(GameplayTags=((TagName=\"A.B\"),...))", a map as an object, a text as a string, an enumerator by its name, a
condition or an effect as an object with every member of its struct. The importer takes other spellings too (a tag as
an object, a container entry without quotes); they are problems here, so that one spelling exists and every rule
reads it.

The closed lists live at the head of this file: the kinds of column with the options of each (COLUMN_OPTIONS), the
tests of a row rule with the kinds each may be asked of (TESTS, TEST_KINDS), the keys of the schema.
"""
import csv
import io
import json
import math
import os
import re
import unicodedata

SCHEMA_FILE = "schema.json"
TAG_NAME = re.compile(r'TagName="([^"]*)"')
# The export text of a tag container, every entry quoted: what the engine writes and the only spelling read here.
TAG_CONTAINER = re.compile(r'\(GameplayTags=(\(\(TagName="[^"]*"\)(,\(TagName="[^"]*"\))*\))?\)')
TAG_ROW = re.compile(r'^\s*(\+?)GameplayTagList\s*=\s*\(.*?Tag\s*=\s*"([^"]+)"')
INI_SECTION = re.compile(r'^\s*\[([^\]]*)\]\s*$')
# The section the engine reads tag rows from: the settings object for a Default ini, the list object for a file under
# Config/Tags (GameplayTagsManager.cpp loads each of those files as a UGameplayTagsList).
TAG_SECTION = "/Script/GameplayTags.GameplayTagsSettings"
TAG_FILE_SECTION = "/Script/GameplayTags.GameplayTagsList"
CODE_NAME = re.compile(r'TEXT\("([^"]*)"\)')
CODE_COMMENT = re.compile(r'/\*.*?\*/|//[^\n]*', re.S)
# FText::Format: a backtick escapes one of these.
ESCAPED = "`{}|"
# FText::Format: an argument modifier right after a slot, as in {Civ}|hpp(...): a name of letters, digits and underscores,
# then its arguments in parentheses (TextFormatter.cpp). A bar that starts no modifier, or a modifier the engine cannot
# compile, is printed as it stands.
MODIFIER = re.compile(r"\|([A-Za-z0-9_]*)(?:\(([^)]*)\))?")
# The particle pairs the texts give hpp, the form after a final consonant first: the engine puts the first argument
# after a Hangul syllable that has a final consonant and the second after one that has none
# (TextFormatArgumentModifier.cpp, FTextFormatArgumentModifier_HangulPostPositions).
HANGUL_PARTICLES = {
    "\uc774": "\uac00", "\uc740": "\ub294", "\uc744": "\ub97c", "\uacfc": "\uc640", "\uc73c\ub85c": "\ub85c",
    "\uc774\ub77c": "\ub77c", "\uc544": "\uc57c", "\uc774\uc5ec": "\uc5ec",
}
# Numbers of the sky (guideline 8.3): a figure is a numeral of any script, or a Korean number word before a counter
# (\ucc9c\uc5b5 \uac1c, \uc2ed\ub9cc \uad11\ub144); a catalogue designation (M87, NGC 205, IC 1101) is a name and no figure. A text is
# read clause by clause, and each clause with a figure carries its own hedge; a thousands comma has no space after it.
SKY_DESIGNATION = re.compile(r"(?<![A-Za-z])[A-Z]{1,4}\s?[0-9]+")
SKY_NUMBER_WORD = re.compile("[\uc2ed\ubc31\ucc9c\ub9cc\uc5b5\uc870]+\\s*(?:\uac1c|\uad11\ub144)")
SKY_CLAUSE = re.compile(r",\s|\s[\u2014\u2013-]\s|;\s")
HANGUL = "\uac00-\ud7a3"


def is_number(value):
    return isinstance(value, (int, float)) and not isinstance(value, bool)


def is_finite(value):
    if not is_number(value):
        return False
    try:
        return math.isfinite(value)
    except OverflowError:
        return False


def is_count(value):
    return isinstance(value, int) and not isinstance(value, bool) and value >= 0


def is_names(value):
    return isinstance(value, list) and all(isinstance(entry, str) for entry in value)


def has_figure(clause):
    """Whether a clause holds a number of the sky (see SKY_DESIGNATION)."""
    plain = SKY_DESIGNATION.sub("", clause)
    return any(ch.isnumeric() for ch in plain) or SKY_NUMBER_WORD.search(plain) is not None


def hedges(clause, words):
    """Whether a clause carries one of the words as a word of its own: no Hangul syllable before it, and after a word
    of one syllable none either (a syllable joined to it makes another word); a longer word may take an ending."""
    for word in words:
        tail = "(?![%s])" % HANGUL if len(word) == 1 else ""
        if re.search("(?<![%s])" % HANGUL + re.escape(word) + tail, clause):
            return True
    return False


def is_scalar(value):
    return isinstance(value, (str, bool)) or is_finite(value)


# A type of value in the schema: what to call it in a problem line, and the test.
NUMBER = ("a number", is_finite)
FLAG = ("true or false", lambda value: isinstance(value, bool))
COUNT = ("a whole number, zero or more", is_count)
COUNTS = ("a list of whole numbers", lambda value: isinstance(value, list) and len(value) > 0 and all(is_count(e) for e in value))
NAME = ("a string", lambda value: isinstance(value, str))
NAMES = ("a list of strings", is_names)
ROOT = ("a string or a list of strings", lambda value: isinstance(value, str) or (is_names(value) and len(value) > 0))
# One word list of the schema by name, or several (B2: an edict's name keeps out both the verdict words and the holy words).
WORD_LISTS = ("a string or a list of strings, not empty", lambda value: isinstance(value, str) or (is_names(value) and len(value) > 0))
SCALAR = ("a string, a number or true/false", is_scalar)
SCALARS = ("a list of strings or numbers", lambda value: isinstance(value, list) and len(value) > 0 and all(is_scalar(e) for e in value))
LIST = ("a list", lambda value: isinstance(value, list))
OBJECT = ("an object", lambda value: isinstance(value, dict))
OBJECT_OR_NULL = ("an object or null", lambda value: value is None or isinstance(value, dict))
NAME_LISTS = ("an object of lists of strings", lambda value: isinstance(value, dict) and all(is_names(e) for e in value.values()))
WORDS = ("a list of strings, not empty", lambda value: is_names(value) and len(value) > 0 and all(entry != "" for entry in value))
TWO_NAMES = ("a list of two column names", lambda value: is_names(value) and len(value) == 2)
# A condition or an effect by its enumerator and its Key: what has_type cannot tell apart (PlayerRoleIs Ruler and Wanderer).
TYPE_AND_KEY = ("an object with exactly the strings type and key", lambda value: isinstance(value, dict) and set(value) == {"type", "key"}
                and all(isinstance(entry, str) for entry in value.values()))

RANGE = {"min": NUMBER, "max": NUMBER, "integer": FLAG}
# Kind of column -> the options a column of that kind may carry next to "kind", each with its type.
COLUMN_OPTIONS = {
    "plain": {},
    "number": dict(RANGE),
    "numbers": dict(RANGE, len=COUNT, ascending=FLAG, ends_at=NUMBER),
    # "forbidden" names a word list of the schema (words), or a list of them: no text of the column holds one of their words.
    "text": {"slots": NAMES, "no_digits": FLAG, "required": FLAG, "hedged": WORDS, "forbidden": WORD_LISTS},
    "texts": {"min": COUNT, "len_in": COUNTS, "empty_or_min": COUNT, "unique": FLAG, "slots": NAMES, "no_digits": FLAG,
              "single_variant": FLAG, "hedged": WORDS, "forbidden": WORD_LISTS},
    "tag": {"root": ROOT, "optional": FLAG},
    "tags": {"root": ROOT},
    "tag_list": {"root": ROOT},
    "tag_map": dict(RANGE, root=ROOT),
    "row": {"table": NAME, "optional": FLAG, "target": LIST},
    "row_map": dict(RANGE, table=NAME, target=LIST),
    # A list of row names, each a row of one of the named tables (DT_Hazards: the means that resolve a hazard).
    "rows": {"tables": WORDS, "unique": FLAG},
    "word_map": dict(RANGE, words=NAME),
    "enum": {"enum": NAME},
    "conditions": {},
    # "no_duration": no entry has a DurationTicks (an edict stands until it is revoked, B2); "keyed": the effect types that
    # must name their Key in this column (a relation names its party: a per-tick source has no other ID); "card": the
    # effects of a card's answer, the only place a card_only effect stands (B3: SetStance).
    "effects": {"per_tick": FLAG, "no_duration": FLAG, "keyed": NAMES, "card": FLAG},
    # A resource pool (FResourcePool): an object with every resource of the word list "resource" as a number, and no
    # other key (the importer keeps a default for a member left out).
    "pool": {"min": NUMBER},
    # A list of structs (DT_Encounters.Choices): each entry an object with exactly the keys of "columns", whose specs are
    # columns of this list, any kind but structs; each cell is checked as a column of that kind.
    "structs": {"columns": OBJECT, "min": COUNT, "len_in": COUNTS},
}
COLUMN_KINDS = tuple(COLUMN_OPTIONS)
REQUIRED_OPTIONS = {"row": ("table",), "row_map": ("table",), "rows": ("tables",), "word_map": ("words",), "enum": ("enum",), "structs": ("columns",)}
# A column of a list of structs is named in a rule as "List.Column": the test has to hold for every entry of the list.
PATH_MARK = "."

# Test of a row rule -> the type of its operand.
TESTS = {
    "is": SCALAR, "in": SCALARS, "gt": NUMBER, "ge": NUMBER, "lt": NUMBER, "le": NUMBER, "set": FLAG,
    "min_len": COUNT, "max_len": COUNT, "each_contains": NAME, "lacks_type": NAME, "has_type": NAME, "slots_in": NAMES,
    # A list of conditions or effects with one of the named enumerator and Key, at least (B1: the role a card is put before).
    "has_cond": TYPE_AND_KEY,
    # The size of a list against other columns of the same row: at least the number in the named column; the product
    # of the two lengths at least the number in the second named column (two pools whose pairs are handed out).
    "len_ge_col": NAME, "pairs_ge_col": TWO_NAMES,
    # Every named tag is in the tags of the cell (DT_Rules.ImmutableLaws holds every law the code reads).
    "contains_all": WORDS,
    # None of the named tags is in the tags of the cell, nor a key of a tag map (B4: no transcendent weighs the claim's
    # events).
    "contains_none": WORDS,
    # A resource pool that asks nothing, every amount 0 (true), or that asks something (false) - B4: the choice that
    # enacts an edict pays the edict's own cost alone.
    "zero": FLAG,
    # No text of the cell holds a word of the named word list (B4 [B-4d1]: a list hung on some rows of a column only -
    # the silent fallen's own words, a card's shared fields, a narrator's row).
    "lacks_words": NAME,
}
# Test -> the kinds of column it can be asked of. Asked of another kind it would never hold (or always).
# A plain column holds a struct the checker does not read (EthicsPreset, Color, MapPosition): no test can be asked of it.
VALUE_KINDS = ("number", "text", "tag", "row", "enum")
SIZED_KINDS = ("numbers", "texts", "tags", "tag_list", "tag_map", "row_map", "word_map", "rows", "conditions", "effects", "structs")
# A pool is a list or a map; a tag container is no pool of names (its export text is a string, never held as a list).
POOL_KINDS = tuple(kind for kind in SIZED_KINDS if kind != "tags")
TEST_KINDS = {
    "is": VALUE_KINDS, "in": VALUE_KINDS, "gt": ("number",), "ge": ("number",), "lt": ("number",), "le": ("number",),
    # A number, an enumerator and a resource pool are always there: 'set' over them says nothing (whether a pool asks
    # anything is 'zero').
    "set": tuple(kind for kind in COLUMN_KINDS if kind not in ("number", "enum", "plain", "pool")), "min_len": SIZED_KINDS, "max_len": SIZED_KINDS, "each_contains": ("texts",),
    "lacks_type": ("conditions", "effects"), "has_type": ("conditions", "effects"), "has_cond": ("conditions", "effects"),
    "slots_in": ("text", "texts"), "len_ge_col": POOL_KINDS,
    "pairs_ge_col": POOL_KINDS, "contains_all": ("tags", "tag_list"), "contains_none": ("tags", "tag_list", "tag_map"),
    "zero": ("pool",),
    "lacks_words": ("text", "texts"),
}
# A row test of its own shape, besides 'row', 'row_not' and 'row_prefix': {"none": [tests]}, each test over a column of
# one list of structs ('List.Column'); it holds when no entry of the list holds every one of them (B4: one choice read as
# a whole - an answer that enacts an edict and is rolled for, a card whose every choice costs or is gated).
NONE_TEST = "none"
# The kinds of the columns a test names besides its own: the number to reach, the other pool.
NAMED_COLUMNS = {"len_ge_col": (("number",),), "pairs_ge_col": (POOL_KINDS, ("number",))}

TOP_KEYS = {
    "version": NUMBER, "about": NAMES, "tag_sources": NAMES, "row_name_sources": NAMES, "required_tags": NAMES,
    "row_name_pattern": NAME, "words": NAME_LISTS, "enums": NAME_LISTS, "single_variant_rows": NAMES, "tables": OBJECT,
    "conditions": OBJECT, "effects": OBJECT, "per_tick_effects": NAMES, "string_table": OBJECT,
}
# What the schema may say of the String Table: per key prefix, a word list no text of it holds and whether it holds a
# digit outside its slots (a slot {0} is filled by the code, not written by the text).
STRING_TABLE_KEYS = {"checked_prefixes": OBJECT}
STRING_PREFIX_KEYS = {"forbidden": WORD_LISTS, "no_digits": FLAG}
FORMAT_SLOT = re.compile(r"\{[^{}]*\}")
TABLE_KEYS = {
    "struct": NAME, "names": NAMES, "prefixes": NAMES, "must_exist": FLAG, "required_rows": NAMES,
    "columns": OBJECT_OR_NULL, "rules": LIST, "unique": LIST, "pairs": LIST,
}
RULE_KEYS = {
    "rules": {"why": NAME, "when": LIST, "then": LIST},
    "unique": {"why": NAME, "when": LIST, "cols": NAMES},
    "pairs": {"why": NAME, "key": NAME, "value": NAME, "allowed": NAME},
}
RULE_REQUIRED = {"rules": ("why", "then"), "unique": ("why", "cols"), "pairs": ("why", "key", "value", "allowed")}
# What a condition or an effect may say about each of its three cells; "per_tick_only" and "card_only" are for effects.
# A Tag may refuse two kinds of tags (B4: DelayHazard): "not_tag_of" - the tag column of the rows of a table that hold
# its tests (the capital rule's hazard); "not_read_by" - the tags a condition of the named enumerators reads in any row
# of any table (the hazard a card waits on with SystemHazardTicksAtMost).
USAGE_KEYS = {
    "tag": {"root": ROOT, "optional": FLAG, "not_tag_of": OBJECT, "not_read_by": NAMES},
    "key": {"word": NAME, "row": NAME, "digits": FLAG, "any": FLAG, "optional": FLAG, "target": LIST},
    "value": dict(RANGE),
}
NOT_TAG_OF_KEYS = {"table": NAME, "col": NAME, "when": LIST}
# The members of the two structs as the sources spell them out (Data/CosmicConditionTypes.h).
MEMBERS = {"conditions": ("Type", "Tag", "Key", "Value"), "effects": ("Type", "Tag", "Key", "Value", "DurationTicks")}


def project_root():
    return os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


# ---- reading ----

def text_of(data):
    """The text of a source read as bytes: UTF-8, a byte order mark dropped."""
    return data.decode("utf-8-sig")


def read_sources(root=None):
    """File name -> bytes of schema.json, every DT_*.json and every ST_*.csv under Tools/data.

    Each file is read once. A caller that goes on to import or to hash a source uses these bytes, so that what was
    checked is what it imports and what it stamps."""
    data_dir = os.path.join(root or project_root(), "Tools", "data")
    sources = {}
    for file_name in sorted(os.listdir(data_dir)):
        is_table = file_name.startswith("DT_") and file_name.endswith(".json")
        is_strings = file_name.startswith("ST_") and file_name.endswith(".csv")
        if file_name == SCHEMA_FILE or is_table or is_strings:
            with open(os.path.join(data_dir, file_name), "rb") as handle:
                sources[file_name] = handle.read()
    return sources


def parse_json(text, file_name, problems):
    """The value of a JSON text, or None with a problem line. A key written twice in one object and a number that is
    no number are problems: json keeps the last of two keys in silence, and so does the engine."""
    def pairs(items):
        found = {}
        for key, value in items:
            if key in found:
                problems.append("%s: the key %r is written twice in one object (the second would replace the first)" % (file_name, key))
            found[key] = value
        return found

    def constant(name):
        problems.append("%s: %s is not a number a table can hold" % (file_name, name))
        return float("nan")

    try:
        return json.loads(text, object_pairs_hook=pairs, parse_constant=constant)
    except ValueError as error:
        problems.append("%s: not valid JSON: %s" % (file_name, error))
        return None


def load_declared_tags(root, sources):
    """(the tags the ini files declare, problem lines). A parent that is only implied by a child is not among them.

    Only the rows the engine reads count (CLAUDE.md, GameplayTags ini rules): in a Default ini a row needs its '+',
    a row without one replaces the whole list; in a file under Config/Tags a row has no '+', one with it is ignored.
    A row counts under the section of its file only. The files under Config/Tags, subfolders too, are found by the
    engine on its own, and so they are here."""
    declared = set()
    problems = []
    files = [source.replace("\\", "/") for source in sources]
    tags_dir = os.path.join(root, "Config", "Tags")
    for folder, _, names in sorted(os.walk(tags_dir)):
        for name in sorted(names):
            source = os.path.relpath(os.path.join(folder, name), root).replace("\\", "/")
            if name.lower().endswith(".ini") and source not in files:
                files.append(source)
    for source in files:
        in_tags_folder = source.startswith("Config/Tags/")
        wanted = TAG_FILE_SECTION if in_tags_folder else TAG_SECTION
        section = None
        try:
            with open(os.path.join(root, source), encoding="utf-8-sig") as handle:
                lines = handle.read().splitlines()
        except (OSError, UnicodeDecodeError) as error:
            problems.append("%s: cannot be read: %s" % (source, error))
            continue
        for number, line in enumerate(lines, 1):
            header = INI_SECTION.match(line)
            if header:
                section = header.group(1)
                continue
            found = TAG_ROW.match(line)
            if not found:
                continue
            has_plus = found.group(1) == "+"
            if section != wanted:
                problems.append("%s:%d: the engine reads the tag rows of this file under [%s] only (%s)" % (source, number, wanted, found.group(2)))
            elif has_plus and in_tags_folder:
                problems.append("%s:%d: the engine ignores a row with '+' in a file under Config/Tags (%s)" % (source, number, found.group(2)))
            elif not has_plus and not in_tags_folder:
                problems.append("%s:%d: a row without '+' replaces the whole tag list of this ini (%s)" % (source, number, found.group(2)))
            else:
                declared.add(found.group(2))
    return declared, problems


def load_code_rows(root, sources):
    """(the row names the code calls by name, problem lines): every TEXT("...") of the headers the schema lists,
    comments left out (a constant that is commented out is not called)."""
    names = []
    problems = []
    for source in sources:
        try:
            with open(os.path.join(root, source), encoding="utf-8-sig") as handle:
                names.extend(CODE_NAME.findall(CODE_COMMENT.sub("", handle.read())))
        except (OSError, UnicodeDecodeError) as error:
            problems.append("%s: cannot be read: %s" % (source, error))
    return names, problems


def forbidden_lists(spec):
    """The word list names a "forbidden" option names: one string, or a list of them; anything else names none."""
    value = spec.get("forbidden") if isinstance(spec, dict) else None
    if isinstance(value, str):
        return [value]
    return list(value) if is_names(value) else []


def string_rules(schema):
    """(prefix, forbidden words, no digits) for each prefix of schema.string_table.checked_prefixes; a shape the
    schema check reports as wrong gives no rule here."""
    table = schema.get("string_table") if isinstance(schema, dict) else None
    prefixes = table.get("checked_prefixes") if isinstance(table, dict) else None
    words = schema.get("words") if isinstance(schema, dict) and isinstance(schema.get("words"), dict) else {}
    rules = []
    for prefix, spec in sorted(prefixes.items()) if isinstance(prefixes, dict) else []:
        if isinstance(spec, dict):
            forbidden = []
            for name in forbidden_lists(spec):
                listed = words.get(name, [])
                forbidden.extend(listed if is_names(listed) else [])
            rules.append((prefix, forbidden, spec.get("no_digits") is True))
    return rules


def check_string_table(file_name, text, schema=None):
    """The shape of a String Table source: header, cell counts, keys, texts. What the texts say is looked at only under
    a prefix of schema.string_table (the narrator's words of a screen: no verdict word, no digit outside a slot)."""
    rows = [row for row in csv.reader(io.StringIO(text)) if row]
    if not rows or rows[0][:2] != ["Key", "SourceString"]:
        return ["%s: the first line has to start with Key,SourceString" % file_name]
    problems = []
    seen = set()
    for number, row in enumerate(rows[1:], 1):
        where = "%s: entry %d (%s)" % (file_name, number, row[0])
        if len(row) != len(rows[0]):
            problems.append("%s: expected %d cells, found %d (a comma or a quote that is not escaped?)" % (where, len(rows[0]), len(row)))
        if row[0] == "":
            problems.append("%s: the key is empty" % where)
        elif row[0] in seen:
            problems.append("%s: the key is written twice (the second text would replace the first)" % where)
        seen.add(row[0])
        if len(row) > 1 and row[1] == "":
            problems.append("%s: the text is empty" % where)
        for prefix, forbidden, no_digits in string_rules(schema):
            if len(row) < 2 or not row[0].startswith(prefix):
                continue
            for word in forbidden:
                if word in row[1]:
                    problems.append("%s: the word %s is not used here: %s" % (where, word, row[1]))
            if no_digits and any(ch.isdigit() for ch in FORMAT_SLOT.sub("", row[1])):
                problems.append("%s: a digit outside a slot in a text that carries none: %s" % (where, row[1]))
    return problems


def check_sources(sources, root=None):
    """The problems of sources (file name -> bytes, as read_sources gives them), as a list of lines."""
    root = root or project_root()
    problems = []
    parsed = {}
    for file_name, data in sorted(sources.items()):
        try:
            text = text_of(data)
        except UnicodeDecodeError as error:
            problems.append("%s: not UTF-8: %s" % (file_name, error))
            continue
        parsed[file_name] = parse_json(text, file_name, problems) if file_name.endswith(".json") else text
    if SCHEMA_FILE not in sources:
        problems.append("%s: missing from Tools/data" % SCHEMA_FILE)
    if problems:
        return problems + ["nothing was checked: the sources have to be readable first"]

    schema = parsed[SCHEMA_FILE]
    tables = {name[:-len(".json")]: value for name, value in parsed.items() if name.startswith("DT_")}
    tag_sources = schema.get("tag_sources", []) if isinstance(schema, dict) else []
    row_sources = schema.get("row_name_sources", []) if isinstance(schema, dict) else []
    tags, tag_problems = load_declared_tags(root, tag_sources if is_names(tag_sources) else [])
    code_rows, row_problems = load_code_rows(root, row_sources if is_names(row_sources) else [])
    problems = tag_problems + row_problems + check(schema, tables, tags, code_rows)
    for file_name in sorted(parsed):
        if file_name.endswith(".csv"):
            problems.extend(check_string_table(file_name, parsed[file_name], schema))
    return problems


def check_project(root=None):
    """The problems of the sources of this project, as a list of lines. Empty = the data holds the schema."""
    return check_sources(read_sources(root), root)


def load_project(root=None):
    """(schema, tables, tags) of the project, for callers that hand them to check() themselves (the unit tests)."""
    root = root or project_root()
    problems = []
    parsed = {name: parse_json(text_of(data), name, problems) for name, data in read_sources(root).items() if name.endswith(".json")}
    if problems:
        raise ValueError("\n".join(problems))
    schema = parsed.pop(SCHEMA_FILE)
    tags, problems = load_declared_tags(root, schema.get("tag_sources", []))
    if problems:
        raise ValueError("\n".join(problems))
    return schema, {name[:-len(".json")]: value for name, value in parsed.items()}, tags


# ---- values ----

def name_of(value):
    """A name cell as the importer reads it: null and the word None (in any case) are the empty name."""
    if value is None or (isinstance(value, str) and value.lower() == "none"):
        return ""
    return value


def is_set(value):
    """A name, a tag, a text or a collection that holds something. A number counts as set."""
    value = name_of(value)
    if isinstance(value, str):
        return value != "" and not (value.startswith("(GameplayTags=") and not TAG_NAME.search(value))
    if isinstance(value, (list, dict)):
        return len(value) > 0
    return True


def length_of(value):
    if isinstance(value, str) and value.startswith("(GameplayTags="):
        return len(TAG_NAME.findall(value))
    if isinstance(value, (list, dict, str)):
        return len(value)
    return 0


def modifier_at(text, index):
    """The argument modifier at text[index], a bar right after a slot: (where reading goes on, what is wrong with it or
    None). The texts use hpp alone, with a pair of HANGUL_PARTICLES in its order."""
    match = MODIFIER.match(text, index)
    name, arguments = match.group(1), match.group(2)
    if not name or arguments is None:
        return index + 1, "a '|' after a slot that starts no modifier (the engine prints it as it stands)"
    if name != "hpp":
        return match.end(), "the modifier |%s is not one the texts use (only hpp)" % name
    pair = [argument.strip() for argument in arguments.split(",")]
    if len(pair) == 2 and HANGUL_PARTICLES.get(pair[1]) == pair[0]:
        return match.end(), "|hpp(%s) has its forms the wrong way round: the form after a final consonant comes first" % arguments
    if len(pair) != 2 or HANGUL_PARTICLES.get(pair[0]) != pair[1]:
        return match.end(), "|hpp(%s) is not a particle pair, the form after a final consonant first (%s)" % (
            arguments, " ".join("%s,%s" % item for item in HANGUL_PARTICLES.items()))
    return match.end(), None


def slots_of(text):
    """(the slot names of a format text, what is wrong with its braces and modifiers), read the way FText::Format reads
    it: a backtick escapes a brace, from an opening brace to the next closing one is the name of an argument, and a bar
    right after it starts an argument modifier (modifier_at)."""
    names = []
    problems = []
    index = 0
    while index < len(text):
        char = text[index]
        if char == "`" and text[index + 1:index + 2] and text[index + 1] in ESCAPED:
            index += 2
            continue
        if char == "{":
            end = text.find("}", index + 1)
            if end < 0:
                problems.append("a '{' that is never closed")
                break
            names.append(text[index + 1:end])
            index = end + 1
            if text[index:index + 1] == "|":
                index, problem = modifier_at(text, index)
                if problem:
                    problems.append(problem)
            continue
        if char == "}":
            problems.append("a '}' that closes nothing")
        index += 1
    return names, problems


class Checker:
    def __init__(self, schema, tables, tags, code_rows=None):
        self.schema = schema
        self.tables = tables
        self.tags = set(tags)
        self.code_rows = code_rows
        self.problems = []
        # Condition enumerator -> {tag -> the places a condition of it reads the tag}, read once (tags_read_by).
        self.read_tags = {}
        self.rows = {}
        for table_name, rows in tables.items():
            if isinstance(rows, list):
                self.rows[table_name] = {row["Name"]: row for row in rows if isinstance(row, dict) and isinstance(row.get("Name"), str)}

    def report(self, where, message):
        self.problems.append("%s: %s" % (where, message))

    # ---- the schema itself ----

    def typed(self, where, entry, allowed, required=()):
        """One object of the schema: every key is one that is read here, every value has the type its key takes,
        every required key is there. False when something is wrong: the entry is then not read any further."""
        if not isinstance(entry, dict):
            self.report(where, "expected an object, found %r" % (entry,))
            return False
        good = True
        for key in sorted(set(entry) - set(allowed)):
            self.report(where, "%r is not read here (allowed: %s)" % (key, ", ".join(allowed) or "nothing"))
            good = False
        for key, (what, fits) in allowed.items():
            if key in entry and not fits(entry[key]):
                self.report(where, "%r has to be %s, found %r" % (key, what, entry[key]))
                good = False
        for key in required:
            if key not in entry:
                self.report(where, "%r is missing" % key)
                good = False
        return good

    def row_is_known(self, table_name, name):
        """A row name the schema may lean on: one the table has to have, or one its source has."""
        spec = self.schema["tables"].get(table_name) if isinstance(table_name, str) else None
        if not isinstance(spec, dict):
            return False
        listed = [entry for key in ("required_rows", "names") for entry in (spec.get(key) if is_names(spec.get(key)) else [])]
        return name in listed or name in self.rows.get(table_name, {})

    def check_tests(self, where, tests, table_name):
        """The tests of a rule over the rows of table_name: each of a known shape with an operand of its type, over a
        column the table declares and of a kind the test can be asked of, against names that exist. A test over a
        misspelt column, tag, enumerator or row never holds, so its rule would be switched off in silence."""
        specs = self.schema["tables"]
        spec = specs.get(table_name) if isinstance(table_name, str) else None
        columns = spec.get("columns") if isinstance(spec, dict) else None
        for test in tests:
            shown = json.dumps(test)
            if isinstance(test, dict) and ("row" in test or "row_not" in test):
                # 'row' holds for the named row, 'row_not' for every other (B3: the one card about another people).
                said = "row" if "row" in test else "row_not"
                if len(test) != 1 or not isinstance(test[said], str):
                    self.report(where, "a row test is 'row' with a row name, or a column with exactly one of %s: %s" % (", ".join(TESTS), shown))
                elif not self.row_is_known(table_name, test[said]):
                    self.report(where, "a row test names the row %s, which %s neither requires nor has" % (test[said], table_name))
                continue
            if isinstance(test, dict) and "row_prefix" in test:
                # The rows of one kind by the prefix of their names (B1: enc_ and petition_). One of the table's own prefixes,
                # or the test would never hold and its rule would be switched off in silence.
                prefixes = spec.get("prefixes") if isinstance(spec, dict) and is_names(spec.get("prefixes")) else []
                if len(test) != 1 or not isinstance(test["row_prefix"], str):
                    self.report(where, "a row test is 'row' with a row name, or a column with exactly one of %s: %s" % (", ".join(TESTS), shown))
                elif test["row_prefix"] not in prefixes:
                    self.report(where, "a row test names the prefix %s, which is none of the prefixes of %s" % (test["row_prefix"], table_name))
                continue
            if isinstance(test, dict) and NONE_TEST in test:
                # No entry of one list of structs holds every one of the tests (B4). Each test names a column of the same
                # list, so that one entry is read as a whole; the tests themselves are checked as any other.
                inner = test[NONE_TEST]
                paths = [entry.get("col") if isinstance(entry, dict) else None for entry in inner] if isinstance(inner, list) else []
                lists = {path.partition(PATH_MARK)[0] for path in paths if isinstance(path, str) and PATH_MARK in path}
                if len(test) != 1 or not isinstance(inner, list) or not inner:
                    self.report(where, "'%s' takes a list of tests, not empty, and nothing else: %s" % (NONE_TEST, shown))
                elif len(lists) != 1 or not all(isinstance(path, str) and PATH_MARK in path for path in paths):
                    self.report(where, "'%s' takes tests over the columns of one list of structs ('List.Column'): %s" % (NONE_TEST, shown))
                else:
                    self.check_tests(where, inner, table_name)
                continue
            asked = [name for name in TESTS if name in test] if isinstance(test, dict) else []
            if len(asked) != 1 or "col" not in test or len(test) != 2 or not isinstance(test["col"], str):
                self.report(where, "a row test is 'row' with a row name, or a column with exactly one of %s: %s" % (", ".join(TESTS), shown))
                continue
            name = asked[0]
            operand = test[name]
            what, fits = TESTS[name]
            if not fits(operand):
                self.report(where, "the test '%s' needs %s: %s" % (name, what, shown))
                continue
            if not isinstance(columns, dict):
                continue
            column_spec, unfit = self.column_at(columns, test["col"], table_name)
            if unfit:
                self.report(where, unfit)
                continue
            if PATH_MARK in test["col"] and name in NAMED_COLUMNS:
                # The other columns it names are columns of the row, not of the entry: the pair would mean nothing.
                self.report(where, "the test '%s' cannot be asked of a column of a list of structs: %s" % (name, shown))
                continue
            kind = column_spec.get("kind")
            if kind not in TEST_KINDS[name]:
                self.report(where, "the test '%s' cannot be asked of a column of kind %s (it would never hold): %s" % (name, kind, shown))
                continue
            for value in (operand if name == "in" else [operand] if name == "is" else []):
                # A number is compared with numbers and a name with names: another type would never be equal.
                if (kind == "number") != is_finite(value):
                    self.report(where, "the test '%s' over a column of kind %s needs %s: %s" % (name, kind, "numbers" if kind == "number" else "strings", shown))
                    continue
                if kind != "number" and not isinstance(value, str):
                    self.report(where, "the test '%s' over a column of kind %s needs strings: %s" % (name, kind, shown))
                    continue
                if kind == "tag" and value != "" and value not in self.tags:
                    self.report(where, "a row test names the tag %s, which the ini does not declare" % (value,))
                if kind == "enum" and value not in (self.schema["enums"].get(column_spec["enum"], []) if isinstance(column_spec.get("enum"), str) else []):
                    self.report(where, "a row test names %r, which is no enumerator of %s" % (value, column_spec.get("enum")))
                if kind == "row" and value != "" and not self.row_is_known(column_spec.get("table"), value):
                    self.report(where, "a row test names the row %s, which %s neither requires nor has" % (value, column_spec.get("table")))
            if name in ("lacks_type", "has_type") and operand not in self.schema[kind]:
                self.report(where, "a row test names %r, which is no enumerator of %s" % (operand, kind))
            if name == "has_cond" and operand["type"] not in self.schema[kind]:
                self.report(where, "a row test names %r, which is no enumerator of %s" % (operand["type"], kind))
            if name in ("contains_all", "contains_none"):
                for value in operand:
                    if value not in self.tags:
                        self.report(where, "a row test names the tag %s, which the ini does not declare" % (value,))
            if name == "lacks_words" and operand not in self.schema["words"]:
                self.report(where, "the word list %s is not in the schema" % (operand,))
            if name in NAMED_COLUMNS:
                for other, kinds in zip([operand] if isinstance(operand, str) else operand, NAMED_COLUMNS[name]):
                    other_spec = columns.get(other)
                    if not isinstance(other_spec, dict):
                        self.report(where, "a row test names the column %r, which %s does not declare" % (other, table_name))
                    elif other_spec.get("kind") not in kinds:
                        self.report(where, "the test '%s' needs a column of kind %s, and %s is of kind %s: %s" % (name, " or ".join(kinds), other, other_spec.get("kind"), shown))

    def column_at(self, columns, name, table_name):
        """(the spec of the column a test names, what is wrong with the name or None). 'List.Column' names a column of
        the list of structs List; a name without the mark names a column of the row."""
        head, mark, sub = name.partition(PATH_MARK)
        spec = columns.get(head)
        if not isinstance(spec, dict):
            return None, "a row test names the column %r, which %s does not declare" % (head if mark else name, table_name)
        if not mark:
            return spec, None
        inner = spec.get("columns") if spec.get("kind") == "structs" else None
        if not isinstance(inner, dict):
            return None, "a row test names %r, and %s is no list of structs" % (name, head)
        sub_spec = inner.get(sub)
        if not isinstance(sub_spec, dict):
            return None, "a row test names the column %r, which the list %s of %s does not declare" % (sub, head, table_name)
        return sub_spec, None

    def check_usage_entry(self, kind, type_name, usage):
        specs = self.schema["tables"]
        where = "schema.%s.%s" % (kind, type_name)
        cells = {cell: OBJECT for cell in USAGE_KEYS}
        if kind == "effects":
            cells["per_tick_only"] = FLAG
            cells["card_only"] = FLAG
        if not self.typed(where, usage, cells):
            return
        # Every cell is read, also after one of them is wrong.
        if not all([self.typed("%s.%s" % (where, cell), usage[cell], allowed) for cell, allowed in USAGE_KEYS.items() if cell in usage]):
            return
        if "tag" in usage:
            self.check_tag_refusals(where + ".tag", usage["tag"])
        key_spec = usage.get("key")
        if key_spec is None:
            return
        said = [name for name in ("word", "row", "digits", "any") if key_spec.get(name) not in (None, False)]
        if len(said) != 1:
            self.report(where + ".key", "a Key is exactly one of: a word of a list, a row of a table, digits, any name")
            return
        if said[0] == "word" and key_spec["word"] not in self.schema["words"]:
            self.report(where, "the word list %s is not in the schema" % (key_spec["word"],))
        if said[0] == "row" and key_spec["row"] not in specs:
            self.report(where, "the table %s is not in the schema" % (key_spec["row"],))
        elif said[0] == "row" and isinstance(key_spec.get("target"), list):
            self.check_tests(where + ".key", key_spec["target"], key_spec["row"])
        if said[0] != "row" and "target" in key_spec:
            self.report(where + ".key", "a target needs a row of a table to look at")

    def check_tag_refusals(self, where, tag_spec):
        """The tags a Tag refuses (B4): a table and a tag column of it that exist, tests the rows of that table can be
        asked, enumerators of conditions that read a Tag. A misspelt name would refuse nothing, in silence."""
        owner = tag_spec.get("not_tag_of")
        if owner is not None and self.typed(where + ".not_tag_of", owner, NOT_TAG_OF_KEYS, ("table", "col")):
            table_spec = self.schema["tables"].get(owner["table"])
            columns = table_spec.get("columns") if isinstance(table_spec, dict) else None
            column_spec = columns.get(owner["col"]) if isinstance(columns, dict) else None
            if not isinstance(table_spec, dict):
                self.report(where, "the table %s is not in the schema" % (owner["table"],))
            elif not isinstance(column_spec, dict) or column_spec.get("kind") != "tag":
                self.report(where, "not_tag_of names the column %r, which %s does not declare as a tag" % (owner["col"], owner["table"]))
            else:
                self.check_tests(where + ".not_tag_of", owner.get("when", []), owner["table"])
        for name in tag_spec.get("not_read_by", []):
            if name not in self.schema["conditions"]:
                self.report(where, "not_read_by names %r, which is no enumerator of conditions" % (name,))
            elif not isinstance(self.schema["conditions"][name], dict) or "tag" not in self.schema["conditions"][name]:
                self.report(where, "not_read_by names %s, a condition that reads no Tag" % (name,))

    def check_column_spec(self, where, column_spec, in_list=False):
        """One column of a table, or (in_list) of a list of structs: a known kind, its options typed, the names it leans
        on existing. The columns of a list of structs are checked the same way, one level down only."""
        specs = self.schema["tables"]
        if not isinstance(column_spec, dict) or not isinstance(column_spec.get("kind"), str):
            self.report(where, "expected an object with a kind, found %r" % (column_spec,))
            return
        kind = column_spec["kind"]
        if kind not in COLUMN_KINDS:
            self.report(where, "a kind this checker does not know: %r" % (kind,))
            return
        if in_list and kind == "structs":
            self.report(where, "a list of structs holds no list of structs")
            return
        if not self.typed(where, column_spec, dict(COLUMN_OPTIONS[kind], kind=NAME), REQUIRED_OPTIONS.get(kind, ())):
            return
        if "table" in column_spec and column_spec["table"] not in specs:
            self.report(where, "the table %r is not in the schema" % (column_spec["table"],))
        elif "target" in column_spec:
            # The tests of a target read the rows of the table the column points at.
            self.check_tests(where, column_spec["target"], column_spec["table"])
        for listed in (column_spec.get("tables") if kind == "rows" else []):
            if listed not in specs:
                self.report(where, "the table %r is not in the schema" % (listed,))
        if kind == "word_map" and column_spec["words"] not in self.schema["words"]:
            self.report(where, "the word list %r is not in the schema" % (column_spec["words"],))
        for listed in forbidden_lists(column_spec):
            if listed not in self.schema["words"]:
                self.report(where, "the word list %r is not in the schema" % (listed,))
        if kind == "enum" and column_spec["enum"] not in self.schema["enums"]:
            self.report(where, "the enum %r is not in the schema" % (column_spec["enum"],))
        if kind == "pool" and "resource" not in self.schema["words"]:
            self.report(where, "a resource pool holds the words of the list 'resource', which the schema does not have")
        for column, inner in (column_spec["columns"].items() if kind == "structs" else []):
            self.check_column_spec("%s.%s" % (where, column), inner, in_list=True)

    def check_table_spec(self, table_name, spec):
        specs = self.schema["tables"]
        here = "schema.%s" % table_name
        if not self.typed(here, spec, TABLE_KEYS, ("struct", "columns")):
            return
        named = [key for key in ("names", "prefixes") if spec.get(key)]
        if len(named) != 1:
            self.report(here, "a table says what its rows are called with exactly one of 'names' and 'prefixes', not empty")
        else:
            for row_name in spec.get("required_rows", []):
                fits = row_name in spec["names"] if "names" in named else any(row_name.startswith(prefix) for prefix in spec["prefixes"])
                if not fits:
                    self.report(here, "the required row %s does not carry a name this table allows" % row_name)
        columns = spec["columns"] or {}
        for column, column_spec in columns.items():
            self.check_column_spec("%s.%s" % (here, column), column_spec)
        for list_name in RULE_KEYS:
            for entry in spec.get(list_name, []):
                if not self.typed("%s.%s" % (here, list_name), entry, RULE_KEYS[list_name], RULE_REQUIRED[list_name]):
                    continue
                if list_name == "rules":
                    self.check_tests(here, entry.get("when", []) + entry["then"], table_name)
                elif list_name == "unique":
                    self.check_tests(here, entry.get("when", []), table_name)
                    for column in entry["cols"]:
                        if column not in columns:
                            self.report(here, "unique names the column %r, which the table does not declare" % (column,))
                else:
                    allowed = self.schema.get(entry["allowed"])
                    if not NAME_LISTS[1](allowed):
                        self.report(here, "the list %r of a pair is not in the schema" % (entry["allowed"],))
                        continue
                    for side in ("key", "value"):
                        column_spec = columns.get(entry[side])
                        if not isinstance(column_spec, dict):
                            self.report(here, "a pair names the column %r, which the table does not declare" % (entry[side],))
                        elif column_spec.get("kind") == "tag":
                            # Both sides of the list are tag names: one the ini lacks would never match a row.
                            names = list(allowed) if side == "key" else [name for values in allowed.values() for name in values]
                            for name in sorted(set(names) - self.tags):
                                self.report("schema.%s" % entry["allowed"], "tag %s is not declared in the ini" % name)

    def check_schema(self):
        """The schema itself, for every table with or without rows. Whatever is wrong here would otherwise switch a
        check off in silence, so the data is not looked at before this is clean."""
        schema = self.schema
        if not isinstance(schema, dict):
            self.report("schema", "expected an object, found %r" % (schema,))
            return
        # A list of pairs is a top-level key of its own name.
        pair_lists = {}
        tables = schema.get("tables")
        for spec in (tables.values() if isinstance(tables, dict) else []):
            for pair in (spec.get("pairs") if isinstance(spec, dict) and isinstance(spec.get("pairs"), list) else []):
                if isinstance(pair, dict) and isinstance(pair.get("allowed"), str) and pair["allowed"] not in TOP_KEYS:
                    pair_lists[pair["allowed"]] = NAME_LISTS
        sections = ("tables", "words", "enums", "conditions", "effects")
        self.typed("schema", schema, dict(TOP_KEYS, **pair_lists), sections)
        if any(key not in schema or not TOP_KEYS[key][1](schema[key]) for key in sections):
            # Without its five sections nothing below can be read. A stray key elsewhere does not stop the reading.
            return
        if isinstance(schema.get("row_name_pattern"), str):
            try:
                re.compile(schema["row_name_pattern"])
            except re.error as error:
                self.report("schema.row_name_pattern", "not a pattern: %s" % error)
        for tag in (schema.get("required_tags") if is_names(schema.get("required_tags")) else []):
            if tag not in self.tags:
                self.report("schema.required_tags", "tag %s is not declared in the ini" % tag)
        if "string_table" in schema and self.typed("schema.string_table", schema["string_table"], STRING_TABLE_KEYS):
            for prefix, spec in (schema["string_table"].get("checked_prefixes") or {}).items():
                where = "schema.string_table.checked_prefixes.%s" % prefix
                if self.typed(where, spec, STRING_PREFIX_KEYS):
                    for listed in forbidden_lists(spec):
                        if listed not in schema["words"]:
                            self.report(where, "the word list %r is not in the schema" % (listed,))
        for name in (schema.get("per_tick_effects") if is_names(schema.get("per_tick_effects")) else []):
            if name not in schema["effects"]:
                self.report("schema.per_tick_effects", "%s is no effect of the schema" % name)
        for kind in ("conditions", "effects"):
            for type_name, usage in schema[kind].items():
                self.check_usage_entry(kind, type_name, usage)
        for table_name, spec in schema["tables"].items():
            self.check_table_spec(table_name, spec)
        self.check_code_rows()

    def check_code_rows(self):
        """required_rows against the names the code calls: both ways, for the tables the schema has."""
        if self.code_rows is None:
            return
        specs = {name: spec for name, spec in self.schema["tables"].items() if isinstance(spec, dict)}
        for name in self.code_rows:
            for table_name, spec in specs.items():
                names = spec.get("names") if is_names(spec.get("names")) else []
                prefixes = spec.get("prefixes") if is_names(spec.get("prefixes")) else []
                required = spec.get("required_rows") if is_names(spec.get("required_rows")) else []
                if (name in names or any(name.startswith(prefix) for prefix in prefixes)) and name not in required:
                    self.report("schema.%s" % table_name, "the code calls the row %s by name and required_rows does not list it" % name)
        for table_name, spec in specs.items():
            for name in (spec.get("required_rows") if is_names(spec.get("required_rows")) else []):
                if name not in self.code_rows:
                    self.report("schema.%s" % table_name, "required_rows lists %s, which the code does not call by name" % name)

    # ---- values ----

    def check_number(self, where, value, spec):
        if not is_finite(value):
            self.report(where, "expected a finite number, found %r" % (value,))
            return
        if "min" in spec and value < spec["min"]:
            self.report(where, "%r is below %r" % (value, spec["min"]))
        if "max" in spec and value > spec["max"]:
            self.report(where, "%r is above %r" % (value, spec["max"]))
        if spec.get("integer") and float(value) != math.floor(value):
            self.report(where, "%r is not a whole number" % (value,))

    def check_tag(self, where, value, spec):
        name = name_of(value)
        if not isinstance(name, str):
            self.report(where, "expected a tag name as a string, found %r" % (value,))
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
        name = name_of(value)
        if not isinstance(name, str):
            self.report(where, "expected a row name, found %r" % (value,))
            return
        if name == "":
            if not spec.get("optional"):
                self.report(where, "a row of %s is required here" % spec["table"])
            return
        target = self.rows.get(spec["table"], {}).get(name)
        if target is None:
            self.report(where, "%s has no row %s" % (spec["table"], name))
            return
        for test in spec.get("target", []):
            if not self.holds(test, target, name):
                self.report(where, "row %s of %s does not hold %s" % (name, spec["table"], json.dumps(test)))

    def check_text(self, where, value, spec):
        if not isinstance(value, str):
            self.report(where, "expected a text, found %r" % (value,))
            return
        if spec.get("required") and value == "":
            self.report(where, "a text is required here")
        # One form of a text, so that a word is found as it is written: composed Hangul (NFC), no invisible format
        # character (a zero-width space splits a word for the eye of a check, not for the reader's).
        if unicodedata.normalize("NFC", value) != value:
            self.report(where, "a text not in the composed form (NFC): %s" % value)
        for ch in value:
            if unicodedata.category(ch) == "Cf":
                self.report(where, "an invisible format character U+%04X in a text: %s" % (ord(ch), value))
                break
        if spec.get("no_digits") and any(ch.isdigit() for ch in value):
            self.report(where, "a digit in a text that carries none: %s" % value)
        # A number of the sky is always an estimate (guideline 8.3): a text of such a column that carries a digit carries
        # one of the words too, as a word of its own: a Hangul syllable joined to it makes another word (no hedge).
        if "hedged" in spec:
            for clause in SKY_CLAUSE.split(value):
                if has_figure(clause) and not hedges(clause, spec["hedged"]):
                    self.report(where, "a number without %s: %s" % (" or ".join(spec["hedged"]), clause))
        for listed in forbidden_lists(spec):
            for word in self.schema["words"].get(listed, []):
                if word in value:
                    self.report(where, "the word %s is not used here: %s" % (word, value))
        if "slots" in spec:
            names, problems = slots_of(value)
            for problem in problems:
                self.report(where, "%s: %s" % (problem, value))
            for slot in names:
                # A slot may carry a format modifier after a bar, as in {Ruler}|hpp(...): slots_of read it above.
                if slot not in spec["slots"]:
                    self.report(where, "slot {%s} is not filled here (allowed: %s)" % (slot, ", ".join(spec["slots"]) or "none"))

    def check_refused_tag(self, where, tag, spec):
        """A tag the usage refuses (B4, DelayHazard): the tag of a row of the named table that holds the tests, or a tag
        a condition of a named enumerator reads somewhere in the data."""
        owner = spec.get("not_tag_of")
        if owner:
            for name, row in sorted(self.rows.get(owner["table"], {}).items()):
                if name_of(row.get(owner["col"])) == tag and self.all_hold(owner.get("when", []), row, name):
                    self.report(where, "tag %s is the tag of row %s of %s, which holds %s: not allowed here" % (
                        tag, name, owner["table"], json.dumps(owner.get("when", []))))
        for type_name in spec.get("not_read_by", []):
            for place in self.tags_read_by(type_name).get(tag, []):
                self.report(where, "tag %s is read by a %s condition of %s: not allowed here" % (tag, type_name, place))

    def tags_read_by(self, type_name):
        """Tag -> the places (Table.row.Column) where a condition of the enumerator reads it: every conditions column of
        every table, in a list of structs too."""
        if type_name in self.read_tags:
            return self.read_tags[type_name]
        found = {}

        def read(place, items):
            for item in (items if isinstance(items, list) else []):
                tag = name_of(item.get("Tag")) if isinstance(item, dict) and item.get("Type") == type_name else ""
                if isinstance(tag, str) and tag != "":
                    found.setdefault(tag, []).append(place)
        for table_name, spec in sorted(self.schema["tables"].items()):
            columns = spec.get("columns") if isinstance(spec, dict) else None
            for column, column_spec in sorted(columns.items()) if isinstance(columns, dict) else []:
                kind = column_spec.get("kind") if isinstance(column_spec, dict) else None
                inner = column_spec.get("columns") if kind == "structs" else None
                for row_name, row in sorted(self.rows.get(table_name, {}).items()):
                    place = "%s.%s.%s" % (table_name, row_name, column)
                    if kind == "conditions":
                        read(place, row.get(column))
                    for sub, sub_spec in sorted(inner.items()) if isinstance(inner, dict) else []:
                        if isinstance(sub_spec, dict) and sub_spec.get("kind") == "conditions":
                            for entry in (row.get(column) if isinstance(row.get(column), list) else []):
                                read("%s.%s" % (place, sub), entry.get(sub) if isinstance(entry, dict) else None)
        self.read_tags[type_name] = found
        return found

    def check_usage(self, where, kind, item, per_tick, spec=None):
        """One condition or effect against the usage table of its enumerator (and the column's own effect options)."""
        if not isinstance(item, dict):
            self.report(where, "expected an object, found %r" % (item,))
            return
        type_name = item.get("Type")
        usage = self.schema[kind].get(type_name) if isinstance(type_name, str) else None
        if usage is None:
            self.report(where, "%r is no enumerator of %s" % (type_name, kind))
            return
        where = "%s(%s)" % (where, type_name)
        # Every member of the struct, and no other: the importer keeps a default for a member left out and drops one it
        # does not know.
        for member in sorted(set(item) - set(MEMBERS[kind])):
            self.report(where, "the struct has no member %s" % member)
        for member in MEMBERS[kind]:
            if member not in item:
                self.report(where, "the member %s is missing" % member)

        tag = name_of(item.get("Tag", ""))
        if "tag" in usage:
            self.check_tag(where + ".Tag", tag, usage["tag"])
            if isinstance(tag, str) and tag != "":
                self.check_refused_tag(where + ".Tag", tag, usage["tag"])
        elif tag != "":
            self.report(where + ".Tag", "this type reads no Tag, found %s" % (tag,))

        key = name_of(item.get("Key", ""))
        key_spec = usage.get("key")
        if not isinstance(key, str):
            self.report(where + ".Key", "expected a name, found %r" % (key,))
        elif key_spec is None:
            if key != "":
                self.report(where + ".Key", "this type reads no Key, found %s" % key)
        elif key == "":
            if not key_spec.get("optional"):
                self.report(where + ".Key", "a Key is required here")
        elif "word" in key_spec:
            if key not in self.schema["words"][key_spec["word"]]:
                self.report(where + ".Key", "%s is none of %s" % (key, ", ".join(self.schema["words"][key_spec["word"]])))
        elif "row" in key_spec:
            self.check_row_ref(where + ".Key", key, {"table": key_spec["row"], "optional": True, "target": key_spec.get("target", [])})
        elif key_spec.get("digits"):
            if not (key.isascii() and key.isdigit() and len(key) <= 9 and int(key) > 0):
                self.report(where + ".Key", "%r is not a count of ticks (digits only, nine at most, above zero)" % (key,))

        value = item.get("Value", 0)
        if "value" in usage:
            self.check_number(where + ".Value", value, usage["value"])
        elif not is_number(value) or value != 0:
            self.report(where + ".Value", "this type reads no Value, found %r" % (value,))

        if kind == "effects":
            duration = item.get("DurationTicks", 0)
            if not is_count(duration) or (duration > 0 and not per_tick):
                self.report(where + ".DurationTicks", "%r: a whole number, and above zero only for a per-tick source" % (duration,))
            allowed = self.schema.get("per_tick_effects", [])
            if per_tick and type_name not in allowed:
                self.report(where, "a per-tick source applies only %s" % ", ".join(allowed))
            if not per_tick and usage.get("per_tick_only"):
                self.report(where, "this effect is applied by per-tick sources only")
            if usage.get("card_only") and not (spec and spec.get("card")):
                self.report(where, "this effect stands only in the answer of a card (a column with card)")
            if spec and spec.get("no_duration") and is_count(duration) and duration != 0:
                self.report(where + ".DurationTicks", "%r: this column's effects stand without a duration" % (duration,))
            if spec and type_name in (spec.get("keyed") or []) and key == "":
                self.report(where + ".Key", "this column's %s names its Key" % type_name)

    def check_column(self, where, value, spec, row_name):
        kind = spec["kind"]  # "plain" has no branch below: nothing of it is checked
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
            if spec.get("ascending") and all(is_finite(entry) for entry in value):
                if any(later <= earlier for earlier, later in zip(value, value[1:])):
                    self.report(where, "the entries do not rise: %r" % (value,))
            # A cumulative share ends where every draw is covered.
            if "ends_at" in spec and (not value or not is_number(value[-1]) or value[-1] != spec["ends_at"]):
                self.report(where, "the last entry has to be %r, found %r" % (spec["ends_at"], value[-1] if value else None))
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
            # Among the texts only: an entry of another type is reported above, never as a duplicate.
            texts = [entry for entry in value if isinstance(entry, str)]
            if spec.get("unique") and len(set(texts)) != len(texts):
                self.report(where, "an entry appears twice")
        elif kind == "tag":
            self.check_tag(where, value, spec)
        elif kind == "tags":
            if not isinstance(value, str) or (value != "" and not TAG_CONTAINER.fullmatch(value)):
                self.report(where, "expected the export text of a tag container, (GameplayTags=((TagName=\"A.B\"),...)) with every name quoted, found %r" % (value,))
                return
            for name in TAG_NAME.findall(value):
                self.check_tag(where, name, spec)
        elif kind == "tag_list":
            if not isinstance(value, list):
                self.report(where, "expected a list of tags, found %r" % (value,))
                return
            for index, entry in enumerate(value):
                self.check_tag("%s[%d]" % (where, index), entry, spec)
        elif kind in ("tag_map", "row_map", "word_map"):
            if not isinstance(value, dict):
                self.report(where, "expected an object, found %r" % (value,))
                return
            for key, entry in value.items():
                here = "%s[%s]" % (where, key)
                if kind == "tag_map":
                    self.check_tag(here, key, spec)
                elif kind == "row_map":
                    self.check_row_ref(here, key, {"table": spec["table"], "target": spec.get("target", [])})
                elif key not in self.schema["words"][spec["words"]]:
                    self.report(here, "%s is none of %s" % (key, ", ".join(self.schema["words"][spec["words"]])))
                self.check_number(here, entry, spec)
        elif kind == "row":
            self.check_row_ref(where, value, spec)
        elif kind == "rows":
            if not isinstance(value, list):
                self.report(where, "expected a list of row names, found %r" % (value,))
                return
            for index, entry in enumerate(value):
                here = "%s[%d]" % (where, index)
                name = name_of(entry)
                if not isinstance(name, str) or name == "":
                    self.report(here, "expected a row name, found %r" % (entry,))
                elif not any(name in self.rows.get(table, {}) for table in spec["tables"]):
                    self.report(here, "none of %s has a row %s" % (", ".join(spec["tables"]), name))
            # Among the row names only: an entry that is no name is reported above, never as a duplicate.
            names = [name_of(entry) for entry in value if isinstance(name_of(entry), str) and name_of(entry) != ""]
            if spec.get("unique") and len(set(names)) != len(names):
                self.report(where, "an entry appears twice")
        elif kind == "enum":
            names = self.schema["enums"][spec["enum"]]
            if value not in names:
                self.report(where, "%r is no enumerator of %s" % (value, spec["enum"]))
        elif kind in ("conditions", "effects"):
            if not isinstance(value, list):
                self.report(where, "expected a list, found %r" % (value,))
                return
            for index, item in enumerate(value):
                self.check_usage("%s[%d]" % (where, index), kind, item, bool(spec.get("per_tick")), spec)
        elif kind == "pool":
            members = self.schema["words"].get("resource", [])
            if not isinstance(value, dict):
                self.report(where, "expected an object with a number for each of %s, found %r" % (", ".join(members), value))
                return
            # Every member and no other, as with a condition: the importer keeps a default for one left out.
            for member in sorted(set(value) - set(members)):
                self.report("%s.%s" % (where, member), "a resource pool has no member %s" % member)
            for member in members:
                if member not in value:
                    self.report("%s.%s" % (where, member), "the member is missing")
                else:
                    self.check_number("%s.%s" % (where, member), value[member], spec)
        elif kind == "structs":
            if not isinstance(value, list):
                self.report(where, "expected a list of objects, found %r" % (value,))
                return
            count = len(value)
            if "min" in spec and count < spec["min"]:
                self.report(where, "expected %d entries or more, found %d" % (spec["min"], count))
            if "len_in" in spec and count not in spec["len_in"]:
                self.report(where, "expected %s entries, found %d" % (" or ".join(str(n) for n in spec["len_in"]), count))
            columns = spec["columns"]
            for index, item in enumerate(value):
                here = "%s[%d]" % (where, index)
                if not isinstance(item, dict):
                    self.report(here, "expected an object, found %r" % (item,))
                    continue
                # Every member of the struct, and no other: a member left out would keep its default in silence.
                for member in sorted(set(item) - set(columns)):
                    self.report("%s.%s" % (here, member), "the struct has no member %s" % member)
                for column, column_spec in columns.items():
                    if column not in item:
                        self.report("%s.%s" % (here, column), "the member is missing")
                    else:
                        self.check_column("%s.%s" % (here, column), item[column], column_spec, row_name)

    # ---- row rules ----

    def holds(self, test, row, row_name):
        """One test of a row rule (check_schema has made sure it is one this file knows)."""
        if "row" in test:
            return row_name == test["row"]
        if "row_not" in test:
            return row_name != test["row_not"]
        if "row_prefix" in test:
            return row_name.startswith(test["row_prefix"])
        if NONE_TEST in test:
            # No entry of the list holds every test (and so an empty list holds it); each test read in the entry.
            items = row.get(test[NONE_TEST][0]["col"].partition(PATH_MARK)[0])
            inner = [dict(entry, col=entry["col"].partition(PATH_MARK)[2]) for entry in test[NONE_TEST]]
            return isinstance(items, list) and not any(isinstance(item, dict) and self.all_hold(inner, item, row_name) for item in items)
        head, mark, sub = test["col"].partition(PATH_MARK)
        if mark:
            # A column of a list of structs: the test holds for every entry (and so for an empty list).
            items = row.get(head)
            inner = dict(test, col=sub)
            return isinstance(items, list) and all(isinstance(item, dict) and self.holds(inner, item, row_name) for item in items)
        name = [key for key in TESTS if key in test][0]
        operand = test[name]
        value = row.get(test["col"])
        plain = name_of(value) if not isinstance(value, (list, dict)) else value
        if name == "is":
            return plain == operand
        if name == "in":
            return not isinstance(plain, (list, dict)) and plain in operand
        if name in ("gt", "ge", "lt", "le"):
            if not is_number(value):
                return False
            return {"gt": value > operand, "ge": value >= operand, "lt": value < operand, "le": value <= operand}[name]
        if name == "set":
            return is_set(value) == operand
        if name == "min_len":
            return length_of(value) >= operand
        if name == "max_len":
            return length_of(value) <= operand
        if name == "each_contains":
            return isinstance(value, list) and all(isinstance(entry, str) and operand in entry for entry in value)
        if name == "lacks_type":
            # A list of conditions or effects without one of the named enumerator.
            return isinstance(value, list) and all(not (isinstance(item, dict) and item.get("Type") == operand) for item in value)
        if name == "has_type":
            # A list of conditions or effects with one of the named enumerator, at least.
            return isinstance(value, list) and any(isinstance(item, dict) and item.get("Type") == operand for item in value)
        if name == "has_cond":
            # The same with the Key as well.
            return isinstance(value, list) and any(isinstance(item, dict) and item.get("Type") == operand["type"] and item.get("Key") == operand["key"]
                                                   for item in value)
        if name in ("contains_all", "contains_none"):
            if isinstance(value, str):
                held = TAG_NAME.findall(value)
            elif isinstance(value, list):
                held = [name_of(entry) for entry in value]
            elif isinstance(value, dict):
                # The keys of a tag map.
                held = list(value)
            else:
                return False
            if name == "contains_none":
                return not any(tag in held for tag in operand)
            return all(tag in held for tag in operand)
        if name == "zero":
            if not isinstance(value, dict):
                return False
            return all(is_number(entry) and entry == 0 for entry in value.values()) == operand
        if name == "lacks_words":
            # Matched letter for letter, as a column's "forbidden" matches.
            words = self.schema["words"].get(operand, []) if isinstance(self.schema.get("words"), dict) else []
            texts = value if isinstance(value, list) else [value]
            return all(isinstance(text, str) and not any(word in text for word in words) for text in texts)
        if name == "len_ge_col":
            bound = row.get(operand)
            return isinstance(value, (list, dict)) and is_number(bound) and length_of(value) >= bound
        if name == "pairs_ge_col":
            other, bound = row.get(operand[0]), row.get(operand[1])
            return isinstance(value, (list, dict)) and isinstance(other, (list, dict)) and is_number(bound) and length_of(value) * length_of(other) >= bound
        # slots_in: every slot of the text, or of every text of the list, is one of the named.
        texts = value if isinstance(value, list) else [value]
        return all(isinstance(text, str) and all(slot in operand for slot in slots_of(text)[0]) for text in texts)

    def all_hold(self, tests, row, row_name):
        return all(self.holds(test, row, row_name) for test in tests)

    # ---- tables ----

    def check_table(self, table_name, spec):
        columns = spec["columns"]
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
            if not pattern.fullmatch(name):
                self.report(where, "the row name is not lower snake case")
            if spec.get("names"):
                if name not in spec["names"]:
                    self.report(where, "this table holds only the rows %s" % ", ".join(spec["names"]))
            elif not any(name.startswith(prefix) for prefix in spec["prefixes"]):
                self.report(where, "the row name does not start with %s" % " or ".join(spec["prefixes"]))
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
                if column not in row:
                    self.report(where, "the column is missing from the row")
                else:
                    self.check_column(where, row[column], column_spec, name)
            for rule in spec.get("rules", []):
                if self.all_hold(rule.get("when", []), row, name):
                    for test in rule["then"]:
                        if not self.holds(test, row, name):
                            self.report("%s.%s" % (table_name, name), "%s: it does not hold %s" % (rule["why"], json.dumps(test)))
            for pair in spec.get("pairs", []):
                allowed = self.schema[pair["allowed"]]
                key = name_of(row.get(pair["key"]))
                value = name_of(row.get(pair["value"]))
                if not isinstance(key, str) or not isinstance(value, str):
                    continue  # reported as a wrong cell by its column
                if key in allowed and value not in allowed[key]:
                    self.report("%s.%s.%s" % (table_name, name, pair["value"]),
                                "%s: %s may tell %s, found %s" % (pair["why"], key, ", ".join(allowed[key]), value))
                elif key not in allowed and key != "":
                    self.report("%s.%s.%s" % (table_name, name, pair["key"]), "%s is not listed under %s" % (key, pair["allowed"]))

        for unique in spec.get("unique", []):
            seen = {}
            for row in rows:
                if not self.all_hold(unique.get("when", []), row, row["Name"]):
                    continue
                cells = [row.get(column) for column in unique["cols"]]
                key = json.dumps([cell if isinstance(cell, (list, dict)) else name_of(cell) for cell in cells], sort_keys=True)
                if key in seen:
                    self.report("%s.%s" % (table_name, row["Name"]),
                                "%s: the same %s as row %s" % (unique["why"], " and ".join(unique["cols"]), seen[key]))
                else:
                    seen[key] = row["Name"]

    def run(self):
        self.check_schema()
        if self.problems:
            self.report("schema", "the data was not checked: the schema has to be right first")
            return self.problems
        specs = self.schema["tables"]
        for table_name in sorted(self.tables):
            if table_name not in specs:
                self.report(table_name, "the schema does not declare this table")
        for table_name, spec in specs.items():
            self.check_table(table_name, spec)
        # A row name is one name in the whole game: it is saved without its table.
        owners = {}
        for table_name in specs:
            for name in self.rows.get(table_name, {}):
                owners.setdefault(name, []).append(table_name)
        for name, tables in sorted(owners.items()):
            if len(tables) > 1:
                self.report(name, "the row name is used by %s" % " and ".join(sorted(tables)))
        return self.problems


def check(schema, tables, tags, code_rows=None):
    """The problems of tables (asset name -> list of rows) under schema, with the declared tag names. code_rows: the
    row names the code calls by name, held against required_rows; None leaves that out."""
    return Checker(schema, tables, tags, code_rows).run()
