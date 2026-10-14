"""Create or refill the DataTable and String Table assets under /Game/CA/Data from Tools/data/.

Cosmic Ascension - data layer helper (guideline.md section 7.3, 10.1). The JSON and CSV files under Tools/data/ are
the editable source of the data (Korean text lives there, never in C++); this script turns them into assets.
It is idempotent: an existing asset is loaded and refilled in place.

Run headlessly with the editor CLOSED and the editor target already built:

  "C:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Binaries\\Win64\\UnrealEditor-Cmd.exe" "<PROJECT_ROOT>\\Cosmic_Ascension.uproject"
      -run=pythonscript -Script="<PROJECT_ROOT>/Tools/make_data_tables.py"
      -unattended -nosplash -stdout -FullStdOutLogOutput

Facts read from the UE 5.8.3 source (see CLAUDE.md for the summary):
  - UDataTableFactory needs its Struct set; AssetTools.CreateAsset then shows no dialog.
  - UDataTableFunctionLibrary::FillDataTableFromJSONString(Table, Json, RowStruct) empties and refills the table.
    Passing the row struct forces the automated, dialog-free import. It returns True even when rows have problems
    (those only reach the log), so this script verifies the result itself by exporting the table again.
  - JSON shape: a top-level array of row objects, row key "Name". A single FGameplayTag is a plain string; a
    FGameplayTagContainer is written as export text "(GameplayTags=((TagName=\"A.B\"),...))". A column missing
    from the JSON keeps its C++ default but counts as an import problem, so the JSON files spell out every column.
  - unreal.EditorAssetLibrary belongs to a plugin this project does not enable; UEditorAssetSubsystem is always there.
  - The localization key of a DataTable text comes from its position (row, column, array index). Once translation has
    started, add to arrays such as Templates and NameBank at the end only, and never delete a DT_ asset to recreate it
    (a new package gets a new namespace). This script refills the existing asset in place for that reason.

Every source is read once, at the start: those bytes are what is checked, what is imported and what is hashed into the
stamp.

What it checks before it touches an asset: the sources against Tools/data/schema.json (Tools/cosmic_data_schema.py, the
same check as "python Tools/check_data.py": the schema itself, row names, references, tags, ranges, the usage of
conditions and effects, the shape of the String Table), then the unit tests of the checker (Tools/test_data_schema.py).
One problem stops the run. The sources come first because the unit tests start from the data of the project: with a
wrong source they fail too, and the lines that say what is wrong would be lost behind them. The tables this script
builds have to be the tables of the folder and of the schema.

What it checks before saving: every table is filled, exported again and compared with the JSON value by value, and a
column of the row struct that the JSON does not spell out is an error. String Table entries are compared key by key and
text by text. Nothing is saved until every table and the String Table have passed, so a failure in a later table does
not leave the earlier ones saved. On success it writes Tools/data/generated.sha1 (one "<file> <sha1>" line per source,
one for schema.json and one for the checker, carriage returns left out of the hash); the automation test
Cosmic_Ascension.Data.SourcesInSync compares the files with that stamp, so a source, a schema or a checker edited
without running this script again fails there. The stamp is removed first, so a failed run leaves none.
"""
import csv
import hashlib
import io
import json
import os
import re
import sys
import unittest

import unreal

# No bytecode cache next to the tools: the checker is small and is read fresh every run.
sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import cosmic_data_schema  # noqa: E402  (the import needs the path above)
import test_data_schema  # noqa: E402

DATA_FOLDER = "/Game/CA/Data"
TABLES = [
    # asset name, row struct (C++ name without the F prefix)
    ("DT_Rules", "CosmicRulesDef"),
    ("DT_Civs", "CivArchetypeDef"),
    ("DT_LogTemplates", "LogTemplateDef"),
]
STRING_TABLE = "ST_CA"
STAMP_FILE = "generated.sha1"
# The checker is stamped next to the sources: the assets were checked by this version of it. The name is relative to
# Tools/data, where Data.SourcesInSync looks.
CHECKER_FILE = "../cosmic_data_schema.py"

SOURCE_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")
EDITOR_ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
TAG_NAME = re.compile(r'TagName="([^"]*)"')
LOC_TEXT = re.compile(r'^NSLOCTEXT\("(?:[^"\\]|\\.)*",\s*"(?:[^"\\]|\\.)*",\s*"((?:[^"\\]|\\.)*)"\)$')
# Escapes the engine exporter writes inside the source string of an NSLOCTEXT.
UNESCAPE = {"n": "\n", "r": "\r", "t": "\t", "'": "'", '"': '"', "\\": "\\"}


def log(message):
    unreal.log("[make_data_tables] " + message)


def row_struct(struct_name):
    struct = unreal.load_object(None, "/Script/Cosmic_Ascension." + struct_name)
    if struct is None:
        raise RuntimeError("row struct %s not found: build the editor target first" % struct_name)
    return struct


def load_or_create(asset_name, asset_class, factory):
    path = DATA_FOLDER + "/" + asset_name
    asset = EDITOR_ASSETS.load_asset(path) if EDITOR_ASSETS.does_asset_exist(path) else None
    if asset is None:
        asset = ASSET_TOOLS.create_asset(asset_name, DATA_FOLDER, asset_class, factory)
    if asset is None:
        raise RuntimeError("could not load or create " + path)
    return asset


def differences(expected, actual, where):
    """Return where the exported value disagrees with what was written.

    Below the row level extra exported keys are ignored; make_table reports the columns a row leaves out.
    """
    if isinstance(expected, (int, float)) and isinstance(actual, dict) and len(actual) == 1:
        # The engine exporter writes a map value as {"<PropertyName>": value}.
        actual = next(iter(actual.values()))
    if isinstance(expected, dict):
        if not isinstance(actual, dict):
            return ["%s: expected an object, exported %r" % (where, actual)]
        # A gameplay tag used as a map key is exported as (TagName="A.B").
        actual = {(TAG_NAME.search(key).group(1) if TAG_NAME.search(key) else key): value for key, value in actual.items()}
        found = []
        for key, value in expected.items():
            if key not in actual:
                found.append("%s.%s: missing from the export" % (where, key))
            else:
                found.extend(differences(value, actual[key], where + "." + key))
        if len(actual) != len(expected) and all(not isinstance(v, (dict, list)) for v in expected.values()):
            # A map: an entry dropped by the importer shows up as a size mismatch.
            extra = sorted(set(actual) - set(expected))
            if extra:
                found.append("%s: the export holds %s, which the JSON does not write" % (where, extra))
        return found
    if isinstance(expected, list):
        if not isinstance(actual, list) or len(actual) != len(expected):
            return ["%s: expected %d entries, exported %r" % (where, len(expected), actual)]
        found = []
        for index, value in enumerate(expected):
            found.extend(differences(value, actual[index], "%s[%d]" % (where, index)))
        return found
    if isinstance(expected, bool):
        return [] if actual is expected else ["%s: expected %r, exported %r" % (where, expected, actual)]
    if isinstance(expected, (int, float)):
        if isinstance(actual, (int, float)) and abs(float(actual) - float(expected)) < 1e-4:
            return []
        return ["%s: expected %r, exported %r" % (where, expected, actual)]
    if isinstance(expected, str) and expected.startswith("(GameplayTags="):
        # Tag container: an unregistered tag is dropped silently by the importer, so compare the tag names.
        wanted = TAG_NAME.findall(expected)
        exported = actual if isinstance(actual, str) else json.dumps(actual, ensure_ascii=False)
        got = TAG_NAME.findall(exported) if "TagName=" in exported else re.findall(r'"TagName":\s*"([^"]*)"', exported)
        explicit = got[:len(wanted)]
        return [] if explicit == wanted else ["%s: expected tags %s, exported %s" % (where, wanted, got)]
    if isinstance(expected, str):
        # A single gameplay tag may come back as an object; an empty tag or name comes back as None.
        exported = actual["TagName"] if isinstance(actual, dict) and "TagName" in actual else actual
        # A localizable text is exported as NSLOCTEXT("namespace", "key", "source").
        text = LOC_TEXT.match(exported) if isinstance(exported, str) else None
        if text:
            exported = re.sub(r"\\(.)", lambda found: UNESCAPE.get(found.group(1), found.group(0)), text.group(1))
        if exported == expected or (expected == "" and exported in ("None", None)):
            return []
        return ["%s: expected %r, exported %r" % (where, expected, actual)]
    return ["%s: unsupported value %r" % (where, expected)]


def fill_table(asset_name, struct_name, source):
    """Fill the table from the text of its source and compare what it then holds. Returns the table, not yet saved."""
    struct = row_struct(struct_name)
    factory = unreal.DataTableFactory()
    factory.set_editor_property("struct", struct)
    table = load_or_create(asset_name, unreal.DataTable, factory)

    rows = json.loads(source)
    if not unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(table, source, struct):
        raise RuntimeError("fill failed for " + asset_name)

    # The fill re-creates the UDataTable object in place, so fetch it again before reading or saving.
    path = DATA_FOLDER + "/" + asset_name
    table = EDITOR_ASSETS.load_asset(path) or table

    exported_text = unreal.DataTableFunctionLibrary.export_data_table_to_json_string(table)
    if isinstance(exported_text, tuple):
        exported_text = exported_text[-1]
    exported = {row["Name"]: row for row in json.loads(exported_text)}

    problems = []
    if list(exported) != [row["Name"] for row in rows]:
        problems.append("row names: wrote %s, table holds %s" % ([row["Name"] for row in rows], list(exported)))
    for row in rows:
        if row["Name"] in exported:
            unset = sorted(set(exported[row["Name"]]) - set(row))
            if unset:
                problems.append("%s.%s: columns not in the JSON (the C++ default was kept): %s" % (asset_name, row["Name"], unset))
            problems.extend(differences(row, exported[row["Name"]], asset_name + "." + row["Name"]))
    if problems:
        raise RuntimeError("%s does not hold what was written:\n  %s" % (asset_name, "\n  ".join(problems)))
    log("%s: %d rows written and verified" % (asset_name, len(rows)))
    return table


def fill_string_table(asset_name, source):
    """Import the String Table from the text of its source and compare what it then holds. Returns it, not yet saved.
    The shape of the source (header, cell counts, keys) was checked with the other sources."""
    table = load_or_create(asset_name, None, unreal.StringTableFactory())
    table_id = table.get_path_name()
    if not unreal.StringTableLibrary.import_table_from_csv_string(table_id, source):
        raise RuntimeError("string table import failed for " + asset_name)

    wanted = {row[0]: row[1] for row in list(csv.reader(io.StringIO(source)))[1:] if row}
    keys = [str(key) for key in unreal.StringTableLibrary.get_keys_from_string_table(table_id)]
    if sorted(keys) != sorted(wanted):
        raise RuntimeError("%s keys: wrote %s, table holds %s" % (asset_name, sorted(wanted), sorted(keys)))
    # A text with a backslash goes through the importer's escapes; those are left to the eye.
    wrong = [key for key, text in wanted.items()
             if "\\" not in text and str(unreal.StringTableLibrary.get_table_entry_source_string(table_id, key)) != text]
    if wrong:
        raise RuntimeError("%s texts differ for %s" % (asset_name, wrong))
    log("%s: %d entries written and verified" % (asset_name, len(keys)))
    return table


def run_checker_tests():
    """The unit tests of the checker: the sources passed it, now it has to show that it would have caught them."""
    stream = io.StringIO()
    result = unittest.TextTestRunner(stream=stream, verbosity=0).run(unittest.defaultTestLoader.loadTestsFromModule(test_data_schema))
    if not result.wasSuccessful():
        raise RuntimeError("the unit tests of the checker fail (python Tools/test_data_schema.py):\n" + stream.getvalue())
    log("checker: %d unit tests pass" % result.testsRun)


def check_sources(sources):
    """The sources against the schema, before any asset is touched. Raises with every problem in the message."""
    problems = cosmic_data_schema.check_sources(sources)
    if not problems:
        # What this script builds is what the folder holds and what the schema declares, struct by struct.
        schema = json.loads(cosmic_data_schema.text_of(sources[cosmic_data_schema.SCHEMA_FILE]))
        in_folder = sorted(name[:-len(".json")] for name in sources if name.startswith("DT_"))
        built = dict(TABLES)
        for table_name in in_folder:
            if table_name not in built:
                problems.append("%s.json is in Tools/data and this script does not build it: add it to TABLES" % table_name)
        for table_name, struct_name in TABLES:
            if table_name not in in_folder:
                problems.append("Tools/data/%s.json is missing: this script builds that table" % table_name)
            declared = schema["tables"].get(table_name, {}).get("struct")
            if declared != struct_name:
                problems.append("%s: this script builds it from %s, the schema says %s" % (table_name, struct_name, declared))
        if STRING_TABLE + ".csv" not in sources:
            problems.append("Tools/data/%s.csv is missing: this script builds that String Table" % STRING_TABLE)
    if problems:
        raise RuntimeError("the sources do not hold Tools/data/schema.json (%d):\n  %s" % (len(problems), "\n  ".join(problems)))
    log("schema: %d sources checked, no problem" % len(sources))


def write_stamp(stamped):
    """Record the hash of what became an asset and of what it was checked with: (file name, bytes) pairs, carriage
    returns left out of the hash, as the automation test does."""
    lines = ["%s %s" % (file_name, hashlib.sha1(data.replace(b"\r", b"")).hexdigest()) for file_name, data in stamped]
    with open(os.path.join(SOURCE_DIR, STAMP_FILE), "w", encoding="ascii", newline="\n") as handle:
        handle.write("\n".join(lines) + "\n")
    log("%s: %d files stamped" % (STAMP_FILE, len(lines)))


# A run that fails must not leave the stamp of the previous run behind.
if os.path.exists(os.path.join(SOURCE_DIR, STAMP_FILE)):
    os.remove(os.path.join(SOURCE_DIR, STAMP_FILE))

# One read of every source under Tools/data and of the checker: the check, the import and the stamp use these bytes.
# (The check reads the tags ini and the header of row names besides; the unit tests read the folder again for themselves.)
SOURCES = cosmic_data_schema.read_sources()
with open(os.path.join(SOURCE_DIR, CHECKER_FILE), "rb") as checker_handle:
    CHECKER_BYTES = checker_handle.read()

check_sources(SOURCES)
run_checker_tests()

# Fill and compare everything, then save everything.
filled = [fill_table(table_name, table_struct, cosmic_data_schema.text_of(SOURCES[table_name + ".json"])) for table_name, table_struct in TABLES]
filled.append(fill_string_table(STRING_TABLE, cosmic_data_schema.text_of(SOURCES[STRING_TABLE + ".csv"])))
for asset in filled:
    if not EDITOR_ASSETS.save_loaded_asset(asset, False):
        raise RuntimeError("save failed for " + asset.get_path_name())
log("%d assets saved" % len(filled))

write_stamp([(table_name + ".json", SOURCES[table_name + ".json"]) for table_name, _ in TABLES]
            + [(STRING_TABLE + ".csv", SOURCES[STRING_TABLE + ".csv"]),
               (cosmic_data_schema.SCHEMA_FILE, SOURCES[cosmic_data_schema.SCHEMA_FILE]),
               (CHECKER_FILE, CHECKER_BYTES)])
log("done")
