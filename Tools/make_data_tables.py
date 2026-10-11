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
"""
import json
import os
import re

import unreal

DATA_FOLDER = "/Game/CA/Data"
TABLES = [
    # asset name, row struct (C++ name without the F prefix)
    ("DT_Rules", "CosmicRulesDef"),
    ("DT_Civs", "CivArchetypeDef"),
    ("DT_LogTemplates", "LogTemplateDef"),
]
STRING_TABLE = "ST_CA"

SOURCE_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")
EDITOR_ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
TAG_NAME = re.compile(r'TagName="([^"]*)"')
LOC_TEXT = re.compile(r'^NSLOCTEXT\("(?:[^"\\]|\\.)*",\s*"(?:[^"\\]|\\.)*",\s*"((?:[^"\\]|\\.)*)"\)$')


def log(message):
    unreal.log("[make_data_tables] " + message)


def read_text(file_name):
    with open(os.path.join(SOURCE_DIR, file_name), encoding="utf-8-sig") as handle:
        return handle.read()


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
    """Return where the exported value disagrees with what was written. Extra exported keys are ignored."""
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
                found.append("%s: unexpected keys %s" % (where, extra))
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
            exported = text.group(1).replace('\\"', '"').replace("\\\\", "\\")
        if exported == expected or (expected == "" and exported in ("None", None)):
            return []
        return ["%s: expected %r, exported %r" % (where, expected, actual)]
    return ["%s: unsupported value %r" % (where, expected)]


def make_table(asset_name, struct_name):
    struct = row_struct(struct_name)
    factory = unreal.DataTableFactory()
    factory.set_editor_property("struct", struct)
    table = load_or_create(asset_name, unreal.DataTable, factory)

    source = read_text(asset_name + ".json")
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
            problems.extend(differences(row, exported[row["Name"]], asset_name + "." + row["Name"]))
    if problems:
        raise RuntimeError("%s does not hold what was written:\n  %s" % (asset_name, "\n  ".join(problems)))

    if not EDITOR_ASSETS.save_loaded_asset(table, False):
        raise RuntimeError("save failed for " + path)
    log("%s: %d rows written and verified" % (asset_name, len(rows)))


def make_string_table(asset_name):
    table = load_or_create(asset_name, None, unreal.StringTableFactory())
    table_id = table.get_path_name()

    source = read_text(asset_name + ".csv")
    if not unreal.StringTableLibrary.import_table_from_csv_string(table_id, source):
        raise RuntimeError("string table import failed for " + asset_name)

    wanted = [line.split(",", 1)[0].strip('"') for line in source.splitlines()[1:] if line.strip()]
    keys = [str(key) for key in unreal.StringTableLibrary.get_keys_from_string_table(table_id)]
    if sorted(keys) != sorted(wanted):
        raise RuntimeError("%s keys: wrote %s, table holds %s" % (asset_name, sorted(wanted), sorted(keys)))

    if not EDITOR_ASSETS.save_loaded_asset(table, False):
        raise RuntimeError("save failed for " + asset_name)
    log("%s: %d entries written and verified" % (asset_name, len(keys)))


for table_name, table_struct in TABLES:
    make_table(table_name, table_struct)
make_string_table(STRING_TABLE)
log("done")
