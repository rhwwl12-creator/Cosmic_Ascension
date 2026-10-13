"""Unit tests of Tools/cosmic_data_schema.py: every check is fed a wrong input and has to report it.

Cosmic Ascension - data layer helper (guideline.md section 7.3, 9.3 Data.Integrity).

  python Tools/test_data_schema.py

The base of every case is the data of the project itself under the schema of the project (it has to pass), plus one
valid DT_Events row and two valid DT_Transcendents rows built here, because those tables have no source yet. A case
changes one thing and names the words the report has to carry.

The last block walks the schema of the project: every rule, every unique, every pair, every target and every slot list
needs an input here that breaks it. A rule added to schema.json without one fails test_every_rule_of_the_schema_can_fail.

Tools/make_data_tables.py runs this file before it checks the sources, so a broken checker stops the build of the assets.
"""
import contextlib
import copy
import json
import os
import shutil
import sys
import tempfile
import unittest

# No bytecode cache: a mutation loop rewrites the checker faster than the cache can tell two versions apart.
sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import cosmic_data_schema as checker  # noqa: E402  (the import needs the path above)

NAN = float("nan")
NOTHING = lambda tables: None  # noqa: E731
NOT_CHECKED = "the data was not checked"
ROW_HEADER = os.path.join("Source", "Cosmic_Ascension", "Data", "CosmicRowNames.h")


def tags_text(*names):
    return "(GameplayTags=(%s))" % ",".join('(TagName="%s")' % name for name in names)


def cond(type_name, tag="", key="", value=0):
    return {"Type": type_name, "Tag": tag, "Key": key, "Value": value}


def eff(type_name, tag="", key="", value=0, duration=0):
    return {"Type": type_name, "Tag": tag, "Key": key, "Value": value, "DurationTicks": duration}


def event_row(name="event_test_harvest", **changes):
    row = {
        "Name": name,
        "Category": "Event.Civ.Economy",
        "LogCategory": "Log.Civ",
        "Trigger": "Lottery",
        "Subject": "Civilization",
        "Conditions": [cond("CivTierAtLeast", value=2)],
        "BaseWeight": 1.0,
        "CooldownTicks": 200,
        "Effects": [eff("AddResource", key="Energy", value=5)],
        "LogTemplates": ["{Civ} one.", "{Civ} two.", "{System} three."],
        "Severity": 0,
        "MinTierToReveal": 0,
        "SignRow": "",
    }
    row.update(changes)
    return row


def transcendent_row(name="trans_silence", **changes):
    row = {
        "Name": name,
        "Doctrine": "Doctrine.Silence",
        "Allegiance": "Realm.Abyssal",
        "CounterfeitOf": "",
        "Title": "",
        "Lure": "",
        "Tier": "Tier4_Transcendent",
        "Depth": 1.0,
        "InitialInfluencedSystems": 1,
        "MaxInfluencedSystems": 12,
        "SpawnWeight": 1.0,
        "EventWeightModifiers": {"Event.Civ.Economy": 1.5},
        "PressureEffectsPerTick": [eff("MultiplyIncome", key="Insight", value=0.7)],
        "VoiceTemplates": [],
        "ClaimTemplates": [],
    }
    row.update(changes)
    return row


def celestial_row(**changes):
    row = transcendent_row(
        "trans_covenant", Doctrine="Doctrine.Covenant", Allegiance="Realm.Celestial", Depth=0.5, InitialInfluencedSystems=0,
        MaxInfluencedSystems=0, SpawnWeight=0.0, EventWeightModifiers={}, PressureEffectsPerTick=[])
    row.update(changes)
    return row


def paths_of(node, here=()):
    """Every path to a node below node: keys of objects and indices of lists."""
    if isinstance(node, dict):
        for key, value in node.items():
            yield here + (key,)
            yield from paths_of(value, here + (key,))
    elif isinstance(node, list):
        for index, value in enumerate(node):
            yield here + (index,)
            yield from paths_of(value, here + (index,))


def with_change(root, path, value, delete=False):
    """A copy of root along path only, with the node at its end replaced or taken out."""
    step = path[0]
    clone = dict(root) if isinstance(root, dict) else list(root)
    if len(path) == 1 and delete:
        del clone[step]
    else:
        clone[step] = value if len(path) == 1 else with_change(root[step], path[1:], value, delete)
    return clone


@contextlib.contextmanager
def project_copy():
    """A copy of what the checker reads, in a temporary folder: the data, the tags ini, the header of row names."""
    work = tempfile.mkdtemp(prefix="ca_schema_test_")
    try:
        root = checker.project_root()
        os.makedirs(os.path.join(work, "Config"))
        shutil.copy(os.path.join(root, "Config", "DefaultGameplayTags.ini"), os.path.join(work, "Config"))
        shutil.copytree(os.path.join(root, "Tools", "data"), os.path.join(work, "Tools", "data"))
        os.makedirs(os.path.dirname(os.path.join(work, ROW_HEADER)))
        shutil.copy(os.path.join(root, ROW_HEADER), os.path.join(work, ROW_HEADER))
        yield work
    finally:
        shutil.rmtree(work, ignore_errors=True)


class SchemaTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.schema, cls.tables, cls.tags = checker.load_project()
        cls.tables = copy.deepcopy(cls.tables)
        cls.tables["DT_Events"] = [event_row()]
        cls.tables["DT_Transcendents"] = [transcendent_row(), celestial_row()]

    def after(self, change=NOTHING, schema_change=None, code_rows=None):
        """The problems after one change to a copy of the base tables (and, for some cases, of the schema)."""
        tables = copy.deepcopy(self.tables)
        schema = copy.deepcopy(self.schema)
        change(tables)
        if schema_change:
            schema_change(schema)
        return checker.check(schema, tables, self.tags, code_rows)

    def row(self, tables, table, name):
        return next(row for row in tables[table] if row["Name"] == name)

    def expect(self, problems, *words):
        """Some problem line carries all the words."""
        self.assertTrue(any(all(word in line for word in words) for line in problems),
                        "no problem carries %r among:\n  %s" % (words, "\n  ".join(problems) or "(none)"))

    def set_cell(self, table, name, column, value):
        def change(tables):
            self.row(tables, table, name)[column] = value
        return change

    def put(self, path, key, value):
        """A change of the schema: one key set under a path."""
        def apply(schema):
            node = schema
            for step in path:
                node = node[step]
            node[key] = value
        return apply

    def drop(self, path, key):
        def apply(schema):
            node = schema
            for step in path:
                node = node[step]
            del node[key]
        return apply

    def schema_problem(self, schema_change, *words):
        """A change of the schema is a problem line, and the data is then not looked at."""
        problems = self.after(NOTHING, schema_change)
        self.expect(problems, *words)
        self.expect(problems, NOT_CHECKED)

    # ---- the base ----

    def test_project_and_fixture_rows_pass(self):
        self.assertEqual(checker.check_project(), [])
        self.assertEqual(self.after(), [])

    # ---- reading ----

    def test_reading_the_sources(self):
        base = checker.read_sources()

        def with_text(file_name, old, new):
            sources = dict(base)
            text = checker.text_of(sources[file_name])
            self.assertEqual(text.count(old), 1, old)
            sources[file_name] = text.replace(old, new).encode("utf-8")
            return checker.check_sources(sources)
        self.assertEqual(checker.check_sources(base), [])
        broken = with_text("DT_Civs.json", '"Name": "arch_terran",', '"Name": "arch_terran"')
        self.expect(broken, "DT_Civs.json", "not valid JSON")
        self.expect(broken, "nothing was checked")
        self.expect(with_text("schema.json", '"version": 1,', '"version": 1, "version": 2,'), "schema.json", "the key 'version' is written twice")
        self.expect(with_text("DT_Rules.json", '"ThroneLockTicks": 600,', '"ThroneLockTicks": 600, "ThroneLockTicks": 601,'),
                    "DT_Rules.json", "the key 'ThroneLockTicks' is written twice")
        self.expect(with_text("DT_Rules.json", '"ReverenceDecay": 0.5', '"ReverenceDecay": NaN'), "DT_Rules.json", "NaN is not a number")
        self.expect(with_text("DT_Rules.json", '"ReverenceDecay": 0.5', '"ReverenceDecay": -Infinity'), "-Infinity is not a number")
        sources = dict(base)
        sources["DT_Civs.json"] = b"\xff\xfe[]"
        self.expect(checker.check_sources(sources), "DT_Civs.json", "not UTF-8")
        sources = dict(base)
        del sources["schema.json"]
        self.expect(checker.check_sources(sources), "schema.json", "missing")
        sources = dict(base)
        sources["schema.json"] = b"[]"
        self.expect(checker.check_sources(sources), "schema", "expected an object")
        # A byte order mark in front of a source is read past.
        sources = dict(base)
        sources["DT_Civs.json"] = b"\xef\xbb\xbf" + sources["DT_Civs.json"]
        self.assertEqual(checker.check_sources(sources), [])

    def test_every_table_source_of_the_folder_is_read(self):
        """A DT_*.json the schema does not name is not skipped: it is a table nobody declared."""
        with project_copy() as work:
            self.assertEqual(checker.check_project(work), [])
            with open(os.path.join(work, "Tools", "data", "DT_Nobody.json"), "w", encoding="utf-8") as handle:
                json.dump([{"Name": "nobody_one"}], handle)
            self.expect(checker.check_project(work), "DT_Nobody", "does not declare this table")

    def test_load_project_names_what_it_could_not_read(self):
        with project_copy() as work:
            with open(os.path.join(work, "Tools", "data", "DT_Civs.json"), "a", encoding="utf-8") as handle:
                handle.write("]")
            with self.assertRaisesRegex(ValueError, "DT_Civs.json: not valid JSON"):
                checker.load_project(work)
        with project_copy() as work:
            with open(os.path.join(work, "Config", "DefaultGameplayTags.ini"), "a", encoding="utf-8") as handle:
                handle.write('\nGameplayTagList=(Tag="Test.Bare")\n')
            with self.assertRaisesRegex(ValueError, "without '\\+'"):
                checker.load_project(work)

    def test_string_table(self):
        check = lambda text: checker.check_string_table("ST_X.csv", text)  # noqa: E731
        self.assertEqual(check('Key,SourceString\n"A","one"\n"B","two, with a comma"\n'), [])
        self.expect(check('Name,Text\n"A","one"\n'), "ST_X.csv", "Key,SourceString")
        self.expect(check(''), "Key,SourceString")
        self.expect(check('Key,SourceString\n"A",one, two\n'), "entry 1 (A)", "expected 2 cells, found 3")
        self.expect(check('Key,SourceString\n"","one"\n'), "the key is empty")
        self.expect(check('Key,SourceString\n"A","one"\n"A","two"\n'), "entry 2 (A)", "the key is written twice")
        self.expect(check('Key,SourceString\n"A",""\n'), "entry 1 (A)", "the text is empty")
        # The string table of the project is under the same check, through the sources.
        sources = checker.read_sources()
        sources["ST_CA.csv"] += b'"Panel.Cycle","again"\n'
        self.expect(checker.check_sources(sources), "ST_CA.csv", "Panel.Cycle", "written twice")

    def test_tags_ini(self):
        with project_copy() as work:
            ini = os.path.join(work, "Config", "DefaultGameplayTags.ini")
            with open(ini, "w", encoding="utf-8") as handle:
                handle.write('[/Script/GameplayTags.GameplayTagsSettings]\n'
                             '+GameplayTagList=(Tag="Test.One",DevComment="counted")\n'
                             ';+GameplayTagList=(Tag="Test.Commented",DevComment="not a row")\n'
                             'GameplayTagList=(Tag="Test.Bare",DevComment="replaces the list")\n')
            os.makedirs(os.path.join(work, "Config", "Tags"))
            with open(os.path.join(work, "Config", "Tags", "Extra.ini"), "w", encoding="utf-8") as handle:
                handle.write('[/Script/GameplayTags.GameplayTagsList]\n'
                             'GameplayTagList=(Tag="Test.Extra",DevComment="counted")\n'
                             '+GameplayTagList=(Tag="Test.Ignored",DevComment="the engine ignores it")\n')
            tags, problems = checker.load_declared_tags(work, ["Config/DefaultGameplayTags.ini"])
            # The file under Config/Tags is found without being listed, as the engine finds it.
            self.assertEqual(tags, {"Test.One", "Test.Extra"})
            self.expect(problems, "DefaultGameplayTags.ini:4", "without '+'", "Test.Bare")
            self.expect(problems, "Config/Tags/Extra.ini:3", "ignores a row with '+'", "Test.Ignored")
            self.assertEqual(len(problems), 2)
            tags, problems = checker.load_declared_tags(work, ["Config/Nowhere.ini"])
            self.expect(problems, "Config/Nowhere.ini", "cannot be read")
            # Through the project: the bare row is a problem line of the whole check.
            self.expect(checker.check_project(work), "without '+'", "Test.Bare")

    def test_rows_the_code_calls_by_name(self):
        root = checker.project_root()
        code_rows, problems = checker.load_code_rows(root, self.schema["row_name_sources"])
        self.assertEqual(problems, [])
        self.assertIn("arch_default", code_rows)
        self.assertIn("project_settle", code_rows)  # its table is not in the schema yet: nothing is asked of it
        self.assertEqual(self.after(code_rows=code_rows), [])
        self.schema_problem_with_rows(code_rows + ["log_new_line"], "schema.DT_LogTemplates", "the code calls the row log_new_line by name")
        self.schema_problem_with_rows(code_rows + ["rules_two"], "schema.DT_Rules", "required_rows lists rules", wanted=False)
        self.schema_problem_with_rows([name for name in code_rows if name != "arch_default"], "schema.DT_Civs",
                                      "required_rows lists arch_default, which the code does not call by name")
        self.schema_problem_with_rows([name for name in code_rows if name != "rules"], "schema.DT_Rules", "required_rows lists rules")
        names, problems = checker.load_code_rows(root, ["Source/Nowhere.h"])
        self.expect(problems, "Source/Nowhere.h", "cannot be read")
        # Through the whole project: the header is read, and a header that cannot be read is a line.
        with project_copy() as work:
            with open(os.path.join(work, ROW_HEADER), "a", encoding="utf-8") as handle:
                handle.write('\ninline const FName LogNewLine(TEXT("log_new_line"));\n')
            self.expect(checker.check_project(work), "schema.DT_LogTemplates", "the code calls the row log_new_line by name")
        with project_copy() as work:
            os.remove(os.path.join(work, ROW_HEADER))
            self.expect(checker.check_project(work), "CosmicRowNames.h", "cannot be read")

    def schema_problem_with_rows(self, code_rows, *words, wanted=True):
        problems = self.after(code_rows=code_rows)
        if wanted:
            self.expect(problems, *words)
            self.expect(problems, NOT_CHECKED)
        else:
            self.assertFalse(any(all(word in line for word in words) for line in problems), problems)

    # ---- tags ----

    def test_tag_not_declared(self):
        self.expect(self.after(self.set_cell("DT_Civs", "arch_default", "Traits", tags_text("Trait.Arch.Default", "Trait.Arch.Nobody"))),
                    "arch_default.Traits", "Trait.Arch.Nobody", "not declared")

    def test_tag_under_wrong_root(self):
        self.expect(self.after(self.set_cell("DT_Civs", "arch_default", "AIProfile", "Trait.Arch.Default")), "AIProfile", "not under AI.Profile.")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "ImmutableLaws", tags_text("Law.Remnant", "Trait.Arch.Default"))),
                    "ImmutableLaws", "Trait.Arch.Default is not under Law.")

    def test_tag_required_and_optional(self):
        self.expect(self.after(self.set_cell("DT_Civs", "arch_default", "RevelationMode", "")), "RevelationMode", "a tag is required")
        self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_terran", "HomeStarClass", "")), [])
        # The importer reads null and the word None, in any case, as the empty name.
        for empty in (None, "None", "none"):
            self.expect(self.after(self.set_cell("DT_Civs", "arch_default", "RevelationMode", empty)), "RevelationMode", "a tag is required")
            self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_terran", "HomeStarClass", empty)), [])

    def test_a_single_tag_is_a_string(self):
        """The importer also takes {"TagName": ...}; rules and unique read one spelling, so the other is a problem."""
        as_object = {"TagName": "Revelation.Book"}
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "RevelationMode", as_object)), "RevelationMode", "expected a tag name as a string")
        self.expect(self.after(self.set_cell("DT_Transcendents", "trans_covenant", "Allegiance", {"TagName": "Realm.Celestial"})),
                    "trans_covenant.Allegiance", "expected a tag name as a string")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "DedicationBanks", [{"TagName": "Faith.Bank.Hebrew"}])),
                    "DedicationBanks[0]", "expected a tag name as a string")
        self.expect(self.after(self.set_cell("DT_Events", "event_test_harvest", "Conditions", [cond("CivHasTrait", tag={"TagName": "Trait.Arch.Terran"})])),
                    "(CivHasTrait).Tag", "expected a tag name as a string")

    def test_parent_tag_is_not_declared(self):
        self.expect(self.after(self.set_cell("DT_Transcendents", "trans_silence", "EventWeightModifiers", {"Event.Civ": 2})),
                    "EventWeightModifiers[Event.Civ]", "not declared")

    def test_tag_list_and_tag_map(self):
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "DedicationBanks", ["Faith.Bank.Nope"])), "DedicationBanks[0]", "not declared")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "DedicationBanks", "Faith.Bank.Hebrew")), "DedicationBanks", "expected a list of tags")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "HazardSpawnMultipliers", {"Trait.Arch.Terran": 1})), "not under Hazard.")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "HazardSpawnMultipliers", {"Hazard.Plague": -1})), "is below 0")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "HazardSpawnMultipliers", ["Hazard.Plague"])), "HazardSpawnMultipliers", "expected an object")

    def test_tag_container_text(self):
        """Only the export text with every name quoted: the importer drops what it cannot read without a word."""
        cell = lambda text: self.after(self.set_cell("DT_Civs", "arch_default", "Traits", text))  # noqa: E731
        self.assertEqual(cell(tags_text("Trait.Arch.Default")), [])
        self.assertEqual(cell("(GameplayTags=)"), [])
        self.assertEqual(cell(""), [])
        self.expect(cell('(GameplayTags=((TagName="")))'), "arch_default.Traits", "a tag is required")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "DedicationBanks", ["Faith.Bank.Hebrew", ""])), "DedicationBanks[1]", "a tag is required")
        for text in ('(GameplayTags=((TagName=Trait.Arch.Default),(TagName=Trait.Arch.Nobody)))',
                     '(GameplayTags=((TagNme="Trait.Arch.Nobody")))',
                     "(GameplayTags=((TagName='Trait.Arch.Nobody')))",
                     '(GameplayTags=whatever',
                     '(GameplayTags=((TagName = "Trait.Arch.Nobody")))',
                     'Trait.Arch.Default',
                     ["Trait.Arch.Default"]):
            self.expect(cell(text), "arch_default.Traits", "expected the export text of a tag container")

    # ---- rows ----

    def test_row_reference(self):
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "PlayerArchetypeRow", "arch_nobody")), "PlayerArchetypeRow", "has no row arch_nobody")
        for empty in ("", None, "None", "NONE"):
            self.expect(self.after(self.set_cell("DT_Rules", "rules", "PlayerArchetypeRow", empty)), "a row of DT_Civs is required")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "PlayerArchetypeRow", ["arch_terran"])), "PlayerArchetypeRow", "expected a row name")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "InitialRelations", {"arch_nobody": 0.5})), "has no row arch_nobody")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "InitialRelations", {"arch_default": 2})), "is above 1")
        self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_terran", "InitialRelations", {"arch_default": 0.5})), [])

    def test_row_names(self):
        def rename(table, old, new):
            def change(tables):
                self.row(tables, table, old)["Name"] = new
            return change
        self.expect(self.after(rename("DT_Civs", "arch_default", "civ_default")), "does not start with arch_")
        self.expect(self.after(rename("DT_Civs", "arch_default", "civ_default")), "arch_default", "is missing")
        self.expect(self.after(rename("DT_Rules", "rules", "rules_two")), "holds only the rows rules")
        self.expect(self.after(rename("DT_Civs", "arch_default", "arch_Default")), "not lower snake case")
        self.expect(self.after(rename("DT_Events", "event_test_harvest", "event@harvest")), "not lower snake case")
        self.expect(self.after(rename("DT_Events", "event_test_harvest", "event_harvest\n")), "not lower snake case")
        self.expect(self.after(lambda tables: tables["DT_Civs"].append(copy.deepcopy(tables["DT_Civs"][0]))), "appears 2 times")

    def test_row_name_is_one_name_in_the_game(self):
        def change(tables):
            row = copy.deepcopy(self.row(tables, "DT_LogTemplates", "log_first_life"))
            row["Name"] = "arch_default"
            tables["DT_LogTemplates"].append(row)
        self.expect(self.after(change), "arch_default", "is used by DT_Civs and DT_LogTemplates")

    def test_tables(self):
        self.expect(self.after(lambda tables: tables.pop("DT_Rules")), "DT_Rules.json is missing")
        self.expect(self.after(lambda tables: tables.update(DT_Hazards=[])), "DT_Hazards", "declares no columns")
        self.expect(self.after(lambda tables: tables.update(DT_Nobody=[])), "DT_Nobody", "does not declare this table")
        self.expect(self.after(lambda tables: tables.update(DT_Events=[transcendent_row("event_wrong")])), "does not declare this column")
        # A table that need not exist may be absent; one that has to exist may not. The flag is set here, so the case
        # stays right when the schema of the project turns it on for a table that got its asset.
        pop_events = lambda tables: tables.pop("DT_Events")  # noqa: E731
        self.assertEqual(self.after(pop_events, self.put(["tables", "DT_Events"], "must_exist", False)), [])
        self.expect(self.after(pop_events, self.put(["tables", "DT_Events"], "must_exist", True)), "DT_Events.json is missing")
        for wrong in (5, {"Name": "arch_default"}, ["arch_default"], [{"Name": ["arch_default"]}], [{"Weight": 1}]):
            self.expect(self.after(lambda tables: tables.update(DT_Civs=wrong)), "DT_Civs", "expected a list of row objects, each with a Name")

    def test_columns_of_a_row(self):
        self.expect(self.after(lambda tables: self.row(tables, "DT_Rules", "rules").update(NewKnob=1)), "rules.NewKnob", "does not declare this column")
        self.expect(self.after(lambda tables: self.row(tables, "DT_Rules", "rules").pop("EventSlotTicks")), "rules.EventSlotTicks", "missing from the row")

    # ---- numbers ----

    def test_number(self):
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "FallenTranscendentCount", 4)), "FallenTranscendentCount", "is above 3")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "FallenTranscendentCount", -1)), "is below 0")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "FallenTranscendentCount", 1.5)), "not a whole number")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "ThroneLockTicks", 600.5)), "ThroneLockTicks", "not a whole number")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "PressureAdjacencyFalloff", NAN)), "finite number")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "LeapTicksMin", "800")), "finite number")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "LeapTicksMin", True)), "finite number")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "LeapTicksMin", 10 ** 400)), "finite number")
        self.expect(self.after(self.set_cell("DT_Transcendents", "trans_silence", "Depth", 1.5)), "Depth", "is above 1")

    def test_numbers(self):
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "PressureBySourceTier", [0, 0, 0.25, 1])), "exactly 5 entries, found 4")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "PressureBySourceTier", [0, 0, 0.25, 1, -2])), "PressureBySourceTier[4]", "is below 0")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "PopulationBandThresholds", [0.5, 0.5, 2.0])), "do not rise")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "PopulationBandThresholds", 3)), "list of numbers")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "LureAcceptByPolicy", {"Greedy": 0.5})), "Greedy is none of")

    # ---- texts ----

    def test_texts(self):
        def templates(count):
            return self.set_cell("DT_LogTemplates", "log_breakthrough_physics", "Templates", ["{Civ} line %d." % index for index in range(count)])
        self.expect(self.after(templates(2)), "3 entries or more, found 2")
        self.assertEqual(self.after(templates(3)), [])
        self.expect(self.after(self.set_cell("DT_LogTemplates", "log_first_life", "Templates", [])), "1 entries or more")
        self.assertEqual(self.after(self.set_cell("DT_LogTemplates", "log_first_life", "Templates", ["{HomeSystem}."])), [])
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "AdjectiveBank", ["a", "b"])), "0 or 3 entries, found 2")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "AdjectiveBank", ["a", "", "c"])), "AdjectiveBank[1]", "a text is required")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "AdjectiveBank", ["a", 2, "c"])), "AdjectiveBank[1]", "expected a text")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "DisplayName", 5)), "DisplayName", "expected a text")
        names = ["name %s" % letter for letter in "abcdefghij"]
        self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_terran", "PersonNameBank", names)), [])
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "PersonNameBank", names[:5])), "none or 10 entries and more, found 5")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "PersonNameBank", names[:9] + names[:1])), "appears twice")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "NameBank", "one text")), "list of texts")

    def test_slots(self):
        log = lambda text: self.after(self.set_cell("DT_LogTemplates", "log_first_life", "Templates", [text]))  # noqa: E731
        self.expect(log("{Civv} arrives."), "slot {Civv}")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "NameBank", ["{Hex} union"])), "slot {Hex}")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "AdjectiveBank", ["{Civ}", "b", "c"])), "slot {Civ}", "none")
        self.expect(self.after(self.set_cell("DT_Events", "event_test_harvest", "LogTemplates", ["{OtherCiv} a.", "b.", "c."])), "slot {OtherCiv}")
        self.assertEqual(log("{CivAdj} {Civ}."), [])
        # Read the way the engine reads a format text.
        self.assertEqual(log("{Ruler}|hpp(a,b) of {Civ}."), [])
        self.assertEqual(log("`{not a slot`} and {Civ}."), [])
        self.expect(log("{HomeSystm. broken {Civ"), "never closed")
        self.expect(log("HomeSystem} arrives."), "closes nothing")
        self.expect(log("{Civv x {Civ} arrives."), "slot {Civv x {Civ}")
        self.expect(log("{{Civ}} arrives."), "slot {{Civ}")
        self.assertEqual(checker.slots_of("a `{b`} {C} ``{D}"), (["C", "D"], []))

    def test_no_digits_in_a_name(self):
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "NameBank", ["Union 7"])), "NameBank[0]", "a digit")
        self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_terran", "NameBank", ["Union Seven"])), [])
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "HomeSystemNames", ["HD-581"])), "HomeSystemNames[0]", "a digit")

    # ---- row rules, unique, pairs ----

    def test_row_rule(self):
        self.expect(self.after(self.set_cell("DT_Civs", "arch_default", "NameBank", ["union"])), "arch_default", "home star", "{Star}")
        self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_default", "NameBank", ["{Star} union"])), [])
        self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_terran", "NameBank", ["union"])), [])

    def test_tests_of_a_rule(self):
        """Each test, both ways, through a rule built here (the rules of the project use only some of them)."""
        def rule(when, then):
            return self.put(["tables", "DT_Civs", "rules"], 0, {"why": "a rule of the test", "when": when, "then": then})
        broken = lambda when, then: self.expect(self.after(NOTHING, rule(when, then)), "a rule of the test")  # noqa: E731
        holds = lambda when, then: self.assertEqual(self.after(NOTHING, rule(when, then)), [])  # noqa: E731
        terran = [{"row": "arch_terran"}]
        holds(terran, [{"col": "Weight", "lt": 1}, {"col": "Weight", "le": 0}, {"col": "Weight", "ge": 0}, {"col": "Weight", "is": 0}])
        broken(terran, [{"col": "Weight", "lt": 0}])
        broken(terran, [{"col": "Weight", "le": -1}])
        broken(terran, [{"col": "Weight", "gt": 0}])
        broken(terran, [{"col": "Weight", "ge": 1}])
        broken(terran, [{"col": "Weight", "is": 1}])
        broken(terran, [{"col": "Weight", "in": [1, 2]}])
        holds(terran, [{"col": "AIProfile", "set": False}, {"col": "RevelationMode", "set": True}, {"col": "RevelationMode", "is": "Revelation.Book"}])
        broken(terran, [{"col": "AIProfile", "set": True}])
        broken(terran, [{"col": "RevelationMode", "set": False}])
        broken(terran, [{"col": "RevelationMode", "in": ["Revelation.Seeking"]}])
        holds(terran, [{"col": "Traits", "set": True}, {"col": "Traits", "min_len": 4}, {"col": "Traits", "max_len": 4}])
        broken(terran, [{"col": "Traits", "min_len": 5}])
        broken(terran, [{"col": "Traits", "max_len": 3}])
        holds(terran, [{"col": "PersonNameBank", "set": False}, {"col": "InitialRelations", "set": False}, {"col": "HomeSystemNames", "min_len": 1}])
        broken(terran, [{"col": "PersonNameBank", "set": True}])
        broken(terran, [{"col": "NameBank", "each_contains": "{Star}"}])
        holds(terran, [{"col": "NameBank", "slots_in": ["Star"]}, {"col": "DisplayName", "slots_in": []}])
        broken([{"row": "arch_default"}], [{"col": "Weight", "is": 0}])
        # A rule without 'when' holds for every row.
        self.expect(self.after(NOTHING, self.put(["tables", "DT_Civs", "rules"], 0, {"why": "a rule for all", "then": [{"col": "Weight", "gt": 0}]})),
                    "DT_Civs.arch_terran", "a rule for all")

    def test_unique(self):
        def two_selectors(first, second):
            def change(tables):
                for name, context in (("log_sign_book", first), ("log_sign_book_again", second)):
                    row = copy.deepcopy(self.row(tables, "DT_LogTemplates", "log_breakthrough_physics"))
                    row.update(Name=name, Category="Log.Sign", Context=context)
                    tables["DT_LogTemplates"].append(row)
            return change
        self.expect(self.after(two_selectors("Revelation.Book", "Revelation.Book")), "log_sign_book_again", "the same Category and Context as row log_sign_book")
        self.assertEqual(self.after(two_selectors("Revelation.Book", "Revelation.Seeking")), [])
        self.expect(self.after(lambda tables: tables["DT_Transcendents"].append(transcendent_row("trans_echo"))), "trans_echo", "the same Doctrine as row trans_silence")
        # The empty name has one value, however it is spelt: "" in one row and the word None in the other are the same.
        one_class = self.put(["tables", "DT_Civs"], "unique", [{"cols": ["HomeStarClass"], "why": "one people per star class"}])
        self.assertEqual(self.after(NOTHING, one_class), [])
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "HomeStarClass", "None"), one_class), "one people per star class", "the same HomeStarClass")

    def test_event_row(self):
        cell = lambda column, value: self.set_cell("DT_Events", "event_test_harvest", column, value)  # noqa: E731
        self.expect(self.after(cell("LogCategory", "Log.Crisis")), "Event.Civ.Economy may tell Log.Civ, found Log.Crisis")
        self.expect(self.after(cell("Category", "Event.Crisis")), "event_test_harvest.Category", "not declared")
        self.expect(self.after(NOTHING, self.drop(["event_log_categories"], "Event.Civ.Economy")),
                    "event_test_harvest.Category", "Event.Civ.Economy is not listed under event_log_categories")
        self.expect(self.after(cell("Trigger", "Sometimes")), "'Sometimes' is no enumerator of ECosmicEventTrigger")
        self.expect(self.after(cell("Trigger", "Standing")), "only a Lottery row has a weight")
        self.expect(self.after(cell("LogTemplates", ["a.", "b."])), "three variants or more")
        self.expect(self.after(cell("LogTemplates", [])), "three variants or more")

        def named(**changes):
            def change(tables):
                self.row(tables, "DT_Events", "event_test_harvest").update(dict(dict(Trigger="Named", BaseWeight=0, CooldownTicks=0, LogTemplates=["one."]), **changes))
            return change
        self.assertEqual(self.after(named()), [])
        self.expect(self.after(named(CooldownTicks=200)), "has no cooldown key")
        self.expect(self.after(named(LogTemplates=[])), "one variant or more")
        # A row fired by name may speak of the vessel; a slot row may not, until the selector words are decided.
        vessel = dict(Conditions=[cond("PlayerRoleIs", key="Ruler")], Effects=[eff("KillPlayer", tag="Death.Hazard"), eff("GrantPower", tag="Way.Received")])
        self.assertEqual(self.after(named(**vessel)), [])
        self.expect(self.after(cell("Conditions", vessel["Conditions"])), "a slot runs for every people", "PlayerRoleIs")
        self.expect(self.after(cell("Effects", vessel["Effects"])), "a slot runs for every people", "GrantPower")

        def sign(min_tier, category, templates=("{Civ} a.", "{System} b.", "{HomeSystem} c.")):
            def change(tables):
                row = copy.deepcopy(self.row(tables, "DT_LogTemplates", "log_breakthrough_physics"))
                row.update(Name="log_sign_test", Category=category, Templates=list(templates))
                tables["DT_LogTemplates"].append(row)
                self.row(tables, "DT_Events", "event_test_harvest").update(SignRow="log_sign_test", MinTierToReveal=min_tier)
            return change
        self.assertEqual(self.after(sign(3, "Log.Sign")), [])
        self.expect(self.after(sign(1, "Log.Sign")), "a tier below it", '"ge": 2')
        self.expect(self.after(sign(3, "Log.Civ")), "SignRow", "row log_sign_test of DT_LogTemplates does not hold", "Log.Sign")
        # The line of a sign row is rendered with the arguments of the event row: a slot of the wider list stays raw.
        self.expect(self.after(sign(3, "Log.Sign", ("{Revelation} a.", "b.", "c."))), "SignRow", "does not hold", "slots_in")
        self.expect(self.after(cell("SignRow", "log_nobody")), "DT_LogTemplates has no row log_nobody")

        def grace(*effects):
            def change(tables):
                self.row(tables, "DT_Events", "event_test_harvest").update(Category="Event.Grace.Jubilee", LogCategory="Log.Civ", Effects=list(effects))
            return change
        rebel = eff("NameNotable", tag="Notable.Rebel")
        relief = eff("ChangeStability", value=0.2)
        self.assertEqual(self.after(grace(relief)), [])
        self.expect(self.after(grace(relief, rebel)), "a grace row names no one", '"lacks_type"')
        self.assertEqual(self.after(cell("Effects", [relief, rebel])), [])

    def test_transcendent_rows(self):
        cell = lambda name, column, value: self.set_cell("DT_Transcendents", name, column, value)  # noqa: E731
        self.expect(self.after(cell("trans_covenant", "SpawnWeight", 1)), "trans_covenant", "celestial", '"SpawnWeight"')
        self.expect(self.after(cell("trans_covenant", "PressureEffectsPerTick", [eff("ChangeStability", value=0.001)])),
                    "trans_covenant", "celestial", '"PressureEffectsPerTick"')
        self.expect(self.after(cell("trans_covenant", "EventWeightModifiers", {"Event.Civ.Society": 1.5})), "trans_covenant", "celestial", '"EventWeightModifiers"')
        self.expect(self.after(cell("trans_silence", "Tier", "Tier5_EternalOrder")), "tier 3 or 4")
        # The tier rule holds for every row, not for the fallen only.
        self.expect(self.after(cell("trans_covenant", "Tier", "Tier1_Planetary")), "trans_covenant", "tier 3 or 4")
        self.expect(self.after(cell("trans_silence", "Allegiance", "Realm.Celestial")), "fixed place of the roster")
        self.expect(self.after(lambda tables: tables["DT_Transcendents"].pop(0)), "the row trans_silence", "is missing")

    # ---- conditions and effects ----

    def conditions(self, *items):
        return self.after(self.set_cell("DT_Events", "event_test_harvest", "Conditions", list(items)))

    def effects(self, *items):
        return self.after(self.set_cell("DT_Events", "event_test_harvest", "Effects", list(items)))

    def test_conditions(self):
        self.expect(self.conditions(cond("CivIsHappy")), "'CivIsHappy' is no enumerator of conditions")
        self.expect(self.conditions(cond(["CivTierAtLeast"])), "is no enumerator of conditions")
        self.expect(self.conditions(cond("CivTierAtLeast", value=6)), "(CivTierAtLeast).Value", "is above 5")
        self.expect(self.conditions(cond("CivTierAtLeast", value=2, key="x")), "reads no Key")
        self.expect(self.conditions(cond("GalaxyTickAtLeast", value=10, tag="Flag.Grace.Spent")), "reads no Tag")
        self.expect(self.conditions(cond("FlagSet", tag="Flag.Grace.Spent", value=1)), "reads no Value")
        self.expect(self.conditions(cond("FlagSet", tag="Flag.Grace.Spent", value="0")), "reads no Value")
        self.expect(self.conditions(cond("CivHasTrait")), "(CivHasTrait).Tag", "a tag is required")
        self.expect(self.conditions(cond("CivHasTrait", tag="Hazard.Plague")), "not under Trait.")
        self.expect(self.conditions(cond("CivResourceAtLeast", key="Gold", value=1)), "Gold is none of Energy")
        self.expect(self.conditions(cond("CivResourceAtLeast", key="energy", value=1)), "energy is none of Energy")
        self.expect(self.conditions(cond("CivResourceAtLeast", value=1)), "a Key is required")
        self.expect(self.conditions(cond("CivResourceAtLeast", key=["Energy"], value=1)), "(CivResourceAtLeast).Key", "expected a name")
        for key in ("1,000", "0", "0000000100", "100ticks"):
            self.expect(self.conditions(cond("CivStabilityBelowFor", key=key, value=0.3)), "not a count of ticks")
        self.assertEqual(self.conditions(cond("CivStabilityBelowFor", key="100", value=0.3)), [])
        # The counter is fixed at 0.3 and the code does not read the Value: another number would be a lie in the data.
        self.expect(self.conditions(cond("CivStabilityBelowFor", key="100", value=0.5)), "(CivStabilityBelowFor).Value", "is above 0.3")
        self.expect(self.conditions(cond("CivStabilityBelowFor", key="100", value=0)), "(CivStabilityBelowFor).Value", "is below 0.3")
        self.expect(self.conditions(cond("CivRelationAtLeast", key="trans_nobody")), "DT_Transcendents has no row trans_nobody")
        self.assertEqual(self.conditions(cond("CivRelationAtLeast", key="trans_covenant", value=0.5)), [])
        self.assertEqual(self.conditions(cond("CivRelationAtLeast", value=0.5)), [])
        self.assertEqual(self.conditions(cond("SystemHasHazard"), cond("CivHasNotable", tag="Notable.Rebel", key="Former")), [])
        self.expect(self.conditions(cond("CivHasNotable", tag="Notable.Rebel", key="Latter")), "Latter is none of Former")
        self.expect(self.conditions("CivTierAtLeast"), "expected an object")
        self.expect(self.after(self.set_cell("DT_Events", "event_test_harvest", "Conditions", {"Type": "CivTierAtLeast"})), "Conditions", "expected a list")

    def test_conditions_that_look_at_an_instance(self):
        """A celestial row never has an instance: as the Key of these three the condition could never hold."""
        for type_name in ("SystemInfluencedBy", "TranscendentSealed", "TranscendentAwareOfPlayer"):
            self.assertEqual(self.conditions(cond(type_name, key="trans_silence")), [])
            self.assertEqual(self.conditions(cond(type_name)), [])
            self.expect(self.conditions(cond(type_name, key="trans_covenant")), "(%s).Key" % type_name, "row trans_covenant of DT_Transcendents does not hold")

    def test_every_member_of_the_struct_is_written(self):
        without_value = {"Type": "GalaxyTickAtLeast", "Tag": "", "Key": ""}
        self.expect(self.conditions(without_value), "(GalaxyTickAtLeast)", "the member Value is missing")
        self.expect(self.conditions(dict(without_value, Vaule=300, Value=0)), "(GalaxyTickAtLeast)", "the struct has no member Vaule")
        self.expect(self.conditions(dict(cond("GalaxyTickAtLeast", value=1), DurationTicks=0)), "the struct has no member DurationTicks")
        self.expect(self.effects({"Type": "ChangeStability", "Tag": "", "Key": "", "Value": 0.1}), "(ChangeStability)", "the member DurationTicks is missing")
        self.expect(self.effects(dict(eff("ChangeStability", value=0.1), Duration=5)), "the struct has no member Duration")

    def test_effects(self):
        self.expect(self.effects(eff("Smite")), "'Smite' is no enumerator of effects")
        self.expect(self.effects(eff("MultiplyIncome", key="Energy", value=0.5)), "applied by per-tick sources only")
        self.expect(self.effects(eff("AddResource", key="Energy", value=1, duration=5)), "above zero only for a per-tick source")
        self.expect(self.effects(eff("AddResource", key="Energy", value=1, duration=-1)), "(AddResource).DurationTicks", "a whole number")
        self.expect(self.effects(eff("AddResource", key="Energy", value=1, duration="5")), "(AddResource).DurationTicks", "a whole number")
        self.expect(self.effects(eff("AddResource", key="Energy", value=NAN)), "finite number")
        self.expect(self.effects(eff("ChangeStability", value=-2)), "is below -1")
        self.expect(self.effects(eff("SetFlag")), "(SetFlag).Tag", "a tag is required")
        self.expect(self.effects("ChangeStability"), "expected an object")
        self.assertEqual(self.effects(eff("NameNotable", tag="Notable.Rebel")), [])
        # The effects on the vessel are tried in a row fired by name (a slot row may not carry them).
        def named(*items):
            def change(tables):
                self.row(tables, "DT_Events", "event_test_harvest").update(Trigger="Named", BaseWeight=0, CooldownTicks=0, Effects=list(items))
            return self.after(change)
        self.assertEqual(named(eff("GrantPower", tag="Way.Received"), eff("KillPlayer", tag="Death.Hazard")), [])
        self.expect(named(eff("KillPlayer", tag="Reign.Deposed")), "not under Death.")
        self.expect(named(eff("GrantPower", tag="Death.Hazard")), "not under Way. or Power.")
        self.expect(named(eff("PlayerStatDelta", key="Body", value=0.5)), "not a whole number")
        # A free name: the word None is the empty name for the importer, so a required Key cannot be "none".
        relic = lambda key: self.after(lambda tables: self.row(tables, "DT_Events", "event_test_harvest").update(  # noqa: E731
            Trigger="Named", BaseWeight=0, CooldownTicks=0, Conditions=[cond("PlayerHasRelic", key=key)]))
        self.assertEqual(relic("relic_crown"), [])
        self.expect(relic("none"), "(PlayerHasRelic).Key", "a Key is required")

    def test_a_list_of_tag_roots(self):
        named = lambda tables: self.row(tables, "DT_Events", "event_test_harvest").update(  # noqa: E731
            Trigger="Named", BaseWeight=0, CooldownTicks=0, Effects=[eff("GrantPower", tag="Way.Received")])
        self.assertEqual(self.after(named, self.put(["effects", "GrantPower", "tag"], "root", ["Power.", "Way."])), [])
        self.expect(self.after(named, self.put(["effects", "GrantPower", "tag"], "root", ["Power.", "Might."])), "not under Power. or Might.")

    def test_per_tick_column(self):
        def per_tick(*items):
            return self.after(self.set_cell("DT_Transcendents", "trans_silence", "PressureEffectsPerTick", list(items)))
        self.expect(per_tick(eff("NameNotable", tag="Notable.Rebel")), "a per-tick source applies only AddResource")
        self.expect(per_tick(eff("KillPlayer", tag="Death.Transcendent")), "a per-tick source applies only")
        self.assertEqual(per_tick(eff("MultiplyIncome", value=0.9, duration=10)), [])
        self.expect(per_tick(eff("MultiplyIncome", value=-0.2)), "is below 0")

    # ---- the schema itself ----

    def test_a_problem_of_the_schema_stops_the_check_of_the_data(self):
        wrong_data = self.set_cell("DT_Rules", "rules", "FallenTranscendentCount", 9)
        wrong_schema = self.put(["tables", "DT_Rules", "columns", "EventSlotTicks"], "mim", 1)
        self.expect(self.after(wrong_data), "FallenTranscendentCount", "is above 3")
        problems = self.after(wrong_data, wrong_schema)
        self.expect(problems, "'mim' is not read here")
        self.expect(problems, NOT_CHECKED)
        self.assertFalse(any("FallenTranscendentCount" in line for line in problems), problems)

    def test_kinds_and_tests_are_closed_lists(self):
        self.schema_problem(self.put(["tables", "DT_Events", "columns"], "Severity", {"kind": "riddle"}), "schema.DT_Events.Severity", "a kind this checker does not know")
        rule = ["tables", "DT_Civs", "rules", 0]
        shape = "a row test is 'row' with a row name, or a column with exactly one of"
        self.schema_problem(self.put(rule, "when", [{"col": "Weight", "above": 0}]), "schema.DT_Civs", shape)
        self.schema_problem(self.put(rule, "when", [{"col": "Weight", "gt": 0, "lt": 2}]), "schema.DT_Civs", shape)
        self.schema_problem(self.put(rule, "then", [{"col": "NameBank", "contains": "{Star}"}]), "schema.DT_Civs", shape)
        self.schema_problem(self.put(rule, "then", ["NameBank"]), "schema.DT_Civs", shape)
        self.schema_problem(self.put(rule, "when", [{"row": "arch_default", "col": "Weight"}]), "schema.DT_Civs", shape)
        self.schema_problem(self.put(rule, "when", [{"col": ["Weight"], "gt": 0}]), "schema.DT_Civs", shape)
        self.schema_problem(self.put(["tables", "DT_Events", "columns", "SignRow"], "target", [{"col": "Category", "equals": "Log.Sign"}]), "schema.DT_Events.SignRow", shape)
        self.schema_problem(self.put(["tables", "DT_LogTemplates", "unique", 0], "when", [{"column": "Context", "set": True}]), "schema.DT_LogTemplates", shape)

    def test_a_key_nothing_reads_is_a_problem(self):
        put = self.put
        # A misspelt option would switch the check off; an option of another kind would do nothing.
        self.schema_problem(put(["tables", "DT_Rules", "columns", "EventSlotTicks"], "mim", 1), "schema.DT_Rules.EventSlotTicks", "'mim' is not read here")
        self.schema_problem(put(["tables", "DT_Rules", "columns", "EventSlotTicks"], "len", 5), "schema.DT_Rules.EventSlotTicks", "'len' is not read here")
        self.schema_problem(put(["tables", "DT_Civs", "columns", "Color"], "min", 0), "schema.DT_Civs.Color", "allowed: kind")
        self.schema_problem(put([], "per_tick_effect", []), "schema", "'per_tick_effect' is not read here")
        self.schema_problem(put(["tables", "DT_Civs"], "prefix", ["arch_"]), "schema.DT_Civs", "'prefix' is not read here")
        self.schema_problem(put(["tables", "DT_Civs", "rules", 0], "unless", []), "schema.DT_Civs.rules", "'unless' is not read here")
        self.schema_problem(put(["tables", "DT_LogTemplates", "unique", 0], "columns", []), "schema.DT_LogTemplates.unique", "'columns' is not read here")
        self.schema_problem(put(["tables", "DT_Events", "pairs", 0], "values", []), "schema.DT_Events.pairs", "'values' is not read here")
        self.schema_problem(put(["conditions", "CivTierAtLeast"], "val", {}), "schema.conditions.CivTierAtLeast", "'val' is not read here")
        self.schema_problem(put(["conditions", "CivTierAtLeast"], "per_tick_only", True), "schema.conditions.CivTierAtLeast", "'per_tick_only' is not read here")
        self.schema_problem(put(["effects", "ChangeStability", "value"], "minimum", -1), "schema.effects.ChangeStability.value", "'minimum' is not read here")
        self.schema_problem(put(["effects", "AddResource", "key"], "words", "resource"), "schema.effects.AddResource.key", "'words' is not read here")
        self.schema_problem(put(["conditions", "CivHasTrait", "tag"], "roots", "Trait."), "schema.conditions.CivHasTrait.tag", "'roots' is not read here")

    def test_a_value_of_the_wrong_type_is_a_problem(self):
        """A string where a number stands, a string where a list stands: read as it comes it would loosen the rule
        (a substring test, a comparison that raises at the first row). Each is a line here, with or without rows."""
        column = lambda table, name: ["tables", table, "columns", name]  # noqa: E731
        cases = [
            (column("DT_Rules", "EventSlotTicks"), "min", "1", "'min' has to be a number"),
            (column("DT_Rules", "EventSlotTicks"), "max", NAN, "'max' has to be a number"),
            (column("DT_Rules", "EventSlotTicks"), "integer", "yes", "'integer' has to be true or false"),
            (column("DT_Rules", "PressureBySourceTier"), "len", "5", "'len' has to be a whole number"),
            (column("DT_Rules", "PressureBySourceTier"), "len", -1, "'len' has to be a whole number"),
            (column("DT_Rules", "PopulationBandThresholds"), "ascending", 1, "'ascending' has to be true or false"),
            (column("DT_Rules", "PlayerArchetypeRow"), "table", ["DT_Civs"], "'table' has to be a string"),
            (column("DT_Rules", "PlayerArchetypeRow"), "optional", "false", "'optional' has to be true or false"),
            (column("DT_Rules", "ImmutableLaws"), "root", None, "'root' has to be a string or a list of strings"),
            (column("DT_Rules", "ImmutableLaws"), "root", [], "'root' has to be a string or a list of strings"),
            (column("DT_Rules", "ImmutableLaws"), "root", 5, "'root' has to be a string or a list of strings"),
            (column("DT_Rules", "LureAcceptByPolicy"), "words", 5, "'words' has to be a string"),
            (column("DT_Civs", "NameBank"), "slots", "Star", "'slots' has to be a list of strings"),
            (column("DT_Civs", "NameBank"), "no_digits", "true", "'no_digits' has to be true or false"),
            (column("DT_Civs", "AdjectiveBank"), "len_in", 3, "'len_in' has to be a list of whole numbers"),
            (column("DT_Civs", "AdjectiveBank"), "len_in", ["0", "3"], "'len_in' has to be a list of whole numbers"),
            (column("DT_Civs", "PersonNameBank"), "empty_or_min", "10", "'empty_or_min' has to be a whole number"),
            (column("DT_Civs", "PersonNameBank"), "unique", "true", "'unique' has to be true or false"),
            (column("DT_Civs", "InitialRelations"), "target", {"col": "Weight", "gt": 0}, "'target' has to be a list"),
            (column("DT_LogTemplates", "Templates"), "min", 3.5, "'min' has to be a whole number"),
            (column("DT_LogTemplates", "Templates"), "single_variant", "true", "'single_variant' has to be true or false"),
            (column("DT_Events", "Trigger"), "enum", ["ECosmicEventTrigger"], "'enum' has to be a string"),
            (column("DT_Transcendents", "PressureEffectsPerTick"), "per_tick", "true", "'per_tick' has to be true or false"),
            (["tables", "DT_Civs"], "prefixes", "arch_", "'prefixes' has to be a list of strings"),
            (["tables", "DT_Rules"], "names", "rules", "'names' has to be a list of strings"),
            (["tables", "DT_Rules"], "must_exist", "true", "'must_exist' has to be true or false"),
            (["tables", "DT_Rules"], "required_rows", "rules", "'required_rows' has to be a list of strings"),
            (["tables", "DT_Rules"], "struct", 5, "'struct' has to be a string"),
            (["tables", "DT_Rules"], "columns", [], "'columns' has to be an object or null"),
            (["tables", "DT_Civs"], "rules", None, "'rules' has to be a list"),
            (["tables", "DT_Civs", "rules", 0], "why", 5, "'why' has to be a string"),
            (["tables", "DT_Civs", "rules", 0], "when", {"col": "Weight", "gt": 0}, "'when' has to be a list"),
            (["tables", "DT_LogTemplates", "unique", 0], "cols", "Category", "'cols' has to be a list of strings"),
            (["tables", "DT_Events", "pairs", 0], "key", ["Category"], "'key' has to be a string"),
            ([], "tables", "DT_Rules", "'tables' has to be an object"),
            ([], "words", ["Energy"], "'words' has to be an object of lists of strings"),
            (["words"], "resource", "Energy", "'words' has to be an object of lists of strings"),
            ([], "enums", "EAscensionTier", "'enums' has to be an object of lists of strings"),
            ([], "conditions", [], "'conditions' has to be an object"),
            ([], "effects", "none", "'effects' has to be an object"),
            ([], "single_variant_rows", "log_first_life", "'single_variant_rows' has to be a list of strings"),
            ([], "required_tags", "Notable.Rebel", "'required_tags' has to be a list of strings"),
            ([], "per_tick_effects", "AddResource", "'per_tick_effects' has to be a list of strings"),
            ([], "tag_sources", "Config/DefaultGameplayTags.ini", "'tag_sources' has to be a list of strings"),
            ([], "row_name_pattern", 5, "'row_name_pattern' has to be a string"),
            ([], "event_log_categories", {"Event.Civ.Economy": "Log.Civ"}, "'event_log_categories' has to be an object of lists of strings"),
            (["conditions", "CivTierAtLeast", "value"], "min", "1", "'min' has to be a number"),
            (["conditions", "CivTierAtLeast", "value"], "integer", 1, "'integer' has to be true or false"),
            (["conditions", "CivTierAtLeast"], "value", "any", "'value' has to be an object"),
            (["conditions", "CivTierAtLeast"], "value", None, "'value' has to be an object"),
            (["conditions", "CivHasTrait"], "tag", "Trait.", "'tag' has to be an object"),
            (["conditions", "CivHasTrait", "tag"], "optional", "no", "'optional' has to be true or false"),
            (["conditions", "CivResourceAtLeast"], "key", "word", "'key' has to be an object"),
            (["conditions", "CivResourceAtLeast", "key"], "word", ["resource"], "'word' has to be a string"),
            (["conditions", "CivStabilityBelowFor", "key"], "digits", "true", "'digits' has to be true or false"),
            (["effects", "MultiplyIncome"], "per_tick_only", "true", "'per_tick_only' has to be true or false"),
            (["conditions"], "CivTierAtLeast", "value", "schema.conditions.CivTierAtLeast: expected an object"),
            (["tables"], "DT_Feats", "feat_", "schema.DT_Feats: expected an object"),
            (["tables", "DT_Civs", "columns"], "Weight", "number", "schema.DT_Civs.Weight: expected an object with a kind"),
            (["tables", "DT_Civs", "columns"], "Weight", {"kind": 5}, "schema.DT_Civs.Weight: expected an object with a kind"),
            (["tables", "DT_Civs", "rules"], 0, "Weight > 0", "schema.DT_Civs.rules: expected an object"),
            (["tables", "DT_LogTemplates", "unique"], 0, "Category", "schema.DT_LogTemplates.unique: expected an object"),
            (["tables", "DT_Events", "pairs"], 0, "Category", "schema.DT_Events.pairs: expected an object"),
        ]
        for path, key, value, words in cases:
            with self.subTest(path=path, key=key, value=value):
                self.schema_problem(self.put(path, key, value), words)
        self.schema_problem(self.put([], "row_name_pattern", "^[a-z"), "schema.row_name_pattern", "not a pattern")

    def test_a_key_that_has_to_be_there(self):
        drop = self.drop
        self.schema_problem(drop(["tables", "DT_Rules"], "struct"), "schema.DT_Rules", "'struct' is missing")
        self.schema_problem(drop(["tables", "DT_Feats"], "columns"), "schema.DT_Feats", "'columns' is missing")
        self.schema_problem(drop(["tables", "DT_Rules", "columns", "PlayerArchetypeRow"], "table"), "schema.DT_Rules.PlayerArchetypeRow", "'table' is missing")
        self.schema_problem(drop(["tables", "DT_Rules", "columns", "LureAcceptByPolicy"], "words"), "schema.DT_Rules.LureAcceptByPolicy", "'words' is missing")
        self.schema_problem(drop(["tables", "DT_Events", "columns", "Trigger"], "enum"), "schema.DT_Events.Trigger", "'enum' is missing")
        self.schema_problem(drop(["tables", "DT_Civs", "rules", 0], "then"), "schema.DT_Civs.rules", "'then' is missing")
        self.schema_problem(drop(["tables", "DT_Civs", "rules", 0], "why"), "schema.DT_Civs.rules", "'why' is missing")
        self.schema_problem(drop(["tables", "DT_LogTemplates", "unique", 0], "cols"), "schema.DT_LogTemplates.unique", "'cols' is missing")
        for key in ("key", "value", "allowed", "why"):
            self.schema_problem(drop(["tables", "DT_Events", "pairs", 0], key), "schema.DT_Events.pairs", "%r is missing" % key)
        for key in ("tables", "words", "enums", "conditions", "effects"):
            self.schema_problem(drop([], key), "schema", "%r is missing" % key)
        # What the rows of a table are called: exactly one of the two, and not empty.
        naming = "exactly one of 'names' and 'prefixes'"
        self.schema_problem(drop(["tables", "DT_Civs"], "prefixes"), "schema.DT_Civs", naming)
        self.schema_problem(self.put(["tables", "DT_Civs"], "names", ["arch_default"]), "schema.DT_Civs", naming)
        self.schema_problem(self.put(["tables", "DT_Civs"], "prefixes", []), "schema.DT_Civs", naming)
        self.schema_problem(self.put(["tables", "DT_Civs"], "required_rows", ["civ_default"]), "schema.DT_Civs", "the required row civ_default does not carry a name this table allows")
        self.schema_problem(self.put(["tables", "DT_Rules"], "required_rows", ["rules", "rules_two"]), "schema.DT_Rules", "the required row rules_two")

    def test_what_a_key_of_a_usage_says(self):
        one = "a Key is exactly one of"
        self.schema_problem(self.put(["conditions", "CivResourceAtLeast", "key"], "row", "DT_Civs"), "schema.conditions.CivResourceAtLeast.key", one)
        self.schema_problem(self.put(["conditions", "CivResourceAtLeast"], "key", {"optional": True}), "schema.conditions.CivResourceAtLeast.key", one)
        self.schema_problem(self.put(["conditions", "CivStabilityBelowFor"], "key", {"digits": False}), "schema.conditions.CivStabilityBelowFor.key", one)
        self.schema_problem(self.put(["conditions", "CivResourceAtLeast", "key"], "target", []), "schema.conditions.CivResourceAtLeast.key", "a target needs a row")
        self.schema_problem(self.put(["conditions", "PlayerRoleIs", "key"], "word", "roles"), "schema.conditions.PlayerRoleIs", "the word list roles")
        self.schema_problem(self.put(["effects", "ChangeRelation", "key"], "row", "DT_Nobody"), "schema.effects.ChangeRelation", "the table DT_Nobody")
        self.schema_problem(self.put(["conditions", "TranscendentSealed", "key"], "target", [{"col": "Alegiance", "is": "Realm.Abyssal"}]),
                            "schema.conditions.TranscendentSealed.key", "names the column 'Alegiance'")

    def test_names_the_schema_leans_on(self):
        put = self.put
        self.schema_problem(put(["tables", "DT_Rules", "columns", "PlayerArchetypeRow"], "table", "DT_Peoples"),
                            "schema.DT_Rules.PlayerArchetypeRow", "the table 'DT_Peoples' is not in the schema")
        self.schema_problem(put(["tables", "DT_Rules", "columns", "LureAcceptByPolicy"], "words", "policies"), "the word list 'policies' is not in the schema")
        self.schema_problem(put(["tables", "DT_Events", "columns", "Trigger"], "enum", "ETrigger"), "the enum 'ETrigger' is not in the schema")
        self.schema_problem(put(["tables", "DT_Events", "pairs", 0], "allowed", "event_pairs"), "the list 'event_pairs' of a pair")
        self.schema_problem(put(["tables", "DT_Events", "pairs", 0], "allowed", "version"), "schema.DT_Events", "the list 'version' of a pair")
        self.schema_problem(put([], "per_tick_effects", ["AddResource", "Harvest"]), "Harvest is no effect of the schema")
        self.schema_problem(put([], "required_tags", ["Notable.Rebel", "Notable.Nobody"]), "schema.required_tags", "Notable.Nobody is not declared")

    def test_what_a_rule_names_has_to_exist(self):
        """A test over a misspelt column, tag, enumerator or row never holds: the rule would be off in silence."""
        put = self.put
        rule = ["tables", "DT_Civs", "rules", 0]
        events = ["tables", "DT_Events", "rules"]
        self.schema_problem(put(rule, "when", [{"col": "Wieght", "gt": 0}]), "schema.DT_Civs", "names the column 'Wieght'")
        self.schema_problem(put(rule, "then", [{"col": "NameBanks", "each_contains": "{Star}"}]), "schema.DT_Civs", "names the column 'NameBanks'")
        self.schema_problem(put(["tables", "DT_LogTemplates", "unique", 0], "when", [{"col": "Contxt", "set": True}]), "schema.DT_LogTemplates", "names the column 'Contxt'")
        self.schema_problem(put(["tables", "DT_LogTemplates", "unique", 0], "cols", ["Category", "Contxt"]), "schema.DT_LogTemplates", "unique names the column 'Contxt'")
        # A target is read in the table the column points at, not in its own.
        target = ["tables", "DT_Events", "columns", "SignRow"]
        self.schema_problem(put(target, "target", [{"col": "Categry", "is": "Log.Sign"}]), "schema.DT_Events.SignRow", "names the column 'Categry'")
        self.schema_problem(put(target, "target", [{"col": "LogCategory", "is": "Log.Sign"}]), "schema.DT_Events.SignRow", "names the column 'LogCategory'")
        self.schema_problem(put(target, "target", [{"col": "Category", "is": "Log.Sing"}]), "schema.DT_Events.SignRow", "names the tag Log.Sing")
        self.schema_problem(put(target, "target", [{"row": "log_nobody"}]), "schema.DT_Events.SignRow", "names the row log_nobody, which DT_LogTemplates")
        self.schema_problem(put(["tables", "DT_Events", "pairs", 0], "key", "Categry"), "schema.DT_Events", "a pair names the column 'Categry'")
        self.schema_problem(put(["tables", "DT_Events", "pairs", 0], "value", "LogCategry"), "schema.DT_Events", "a pair names the column 'LogCategry'")
        self.schema_problem(put(["event_log_categories"], "Event.Civ.Econmy", ["Log.Civ"]), "schema.event_log_categories", "tag Event.Civ.Econmy is not declared")
        self.schema_problem(put(["event_log_categories"], "Event.Civ.Economy", ["Log.Civv"]), "schema.event_log_categories", "tag Log.Civv is not declared")
        self.schema_problem(put(events, 0, {"why": "w", "when": [{"col": "Category", "in": ["Event.Grace.Rescue", "Event.Grace.Jubliee"]}], "then": []}),
                            "schema.DT_Events", "names the tag Event.Grace.Jubliee")
        self.schema_problem(put(events, 0, {"why": "w", "when": [{"col": "Trigger", "in": ["Standing", "Namd"]}], "then": []}),
                            "schema.DT_Events", "'Namd', which is no enumerator of ECosmicEventTrigger")
        self.schema_problem(put(events, 0, {"why": "w", "when": [{"col": "Trigger", "is": "Namd"}], "then": []}), "schema.DT_Events", "'Namd', which is no enumerator")
        self.schema_problem(put(events, 0, {"why": "w", "then": [{"col": "Effects", "lacks_type": "NameNotabel"}]}), "schema.DT_Events", "'NameNotabel', which is no enumerator of effects")
        self.schema_problem(put(events, 0, {"why": "w", "then": [{"col": "Effects", "lacks_type": "CivHasNotable"}]}), "schema.DT_Events", "'CivHasNotable', which is no enumerator of effects")
        self.schema_problem(put(events, 0, {"why": "w", "when": [{"row": "event_nobody"}], "then": []}), "schema.DT_Events", "names the row event_nobody, which DT_Events")
        self.schema_problem(put(["tables", "DT_Transcendents", "rules"], 0, {"why": "w", "when": [{"row": "trans_silance"}], "then": []}),
                            "schema.DT_Transcendents", "names the row trans_silance")
        self.schema_problem(put(["tables", "DT_Rules"], "rules", [{"why": "w", "then": [{"col": "PlayerArchetypeRow", "is": "arch_teran"}]}]),
                            "schema.DT_Rules", "names the row arch_teran, which DT_Civs")
        def lost_table(schema):
            schema["tables"]["DT_Rules"]["columns"]["PlayerArchetypeRow"]["table"] = "DT_Peoples"
            schema["tables"]["DT_Rules"]["rules"] = [{"why": "w", "then": [{"col": "PlayerArchetypeRow", "is": "arch_terran"}]}]
        self.schema_problem(lost_table, "schema.DT_Rules", "names the row arch_terran, which DT_Peoples neither requires nor has")
        # A row the table has to have, a row its source has, and the empty name are names a rule may lean on.
        self.assertEqual(self.after(NOTHING, put(["tables", "DT_Rules"], "rules", [{"why": "w", "then": [{"col": "PlayerArchetypeRow", "in": ["arch_terran", "arch_default"]}]}])), [])
        self.assertEqual(self.after(NOTHING, put(rule, "when", [{"col": "AIProfile", "is": ""}, {"col": "Weight", "gt": 0}])), [])

    def test_a_test_fits_the_kind_of_its_column(self):
        """Asked of another kind a test never holds, or always: 'is' over a container, a comparison over a name."""
        rule = ["tables", "DT_Civs", "rules", 0]
        cannot = "cannot be asked of a column of kind"
        for test, kind in (({"col": "Traits", "is": "Trait.Arch.Terran"}, "tags"),
                           ({"col": "DedicationBanks", "in": ["Faith.Bank.Hebrew"]}, "tag_list"),
                           ({"col": "NameBank", "is": "union"}, "texts"),
                           ({"col": "InitialRelations", "is": 0}, "row_map"),
                           ({"col": "RevelationMode", "ge": 4}, "tag"),
                           ({"col": "DisplayName", "each_contains": "x"}, "text"),
                           ({"col": "DisplayName", "min_len": 1}, "text"),
                           ({"col": "Weight", "max_len": 0}, "number"),
                           ({"col": "NameBank", "lacks_type": "NameNotable"}, "texts"),
                           ({"col": "Weight", "slots_in": ["Star"]}, "number")):
            with self.subTest(test=test):
                self.schema_problem(self.put(rule, "when", [test]), "schema.DT_Civs", cannot, kind)
        self.schema_problem(self.put(["tables", "DT_Events", "rules", 0], "when", [{"col": "Trigger", "gt": 1}]), "schema.DT_Events", cannot, "enum")

    def test_the_operand_of_a_test_has_its_type(self):
        rule = ["tables", "DT_Civs", "rules", 0]
        for test, words in (({"col": "Weight", "gt": "0"}, "the test 'gt' needs a number"),
                            ({"col": "Weight", "in": 1}, "the test 'in' needs a list"),
                            ({"col": "Weight", "in": []}, "the test 'in' needs a list"),
                            ({"col": "RevelationMode", "in": "Revelation.Book"}, "the test 'in' needs a list"),
                            ({"col": "Weight", "is": [0]}, "the test 'is' needs a string, a number or true/false"),
                            ({"col": "AIProfile", "set": "false"}, "the test 'set' needs true or false"),
                            ({"col": "NameBank", "min_len": "1"}, "the test 'min_len' needs a whole number"),
                            ({"col": "NameBank", "max_len": 1.5}, "the test 'max_len' needs a whole number"),
                            ({"col": "NameBank", "each_contains": ["{Star}"]}, "the test 'each_contains' needs a string"),
                            ({"col": "NameBank", "slots_in": "Star"}, "the test 'slots_in' needs a list of strings")):
            with self.subTest(test=test):
                self.schema_problem(self.put(rule, "when", [test]), "schema.DT_Civs", words)
        self.schema_problem(self.put(["tables", "DT_Events", "rules", 0], "then", [{"col": "Effects", "lacks_type": ["NameNotable"]}]),
                            "schema.DT_Events", "the test 'lacks_type' needs a string")
        # A number is compared with numbers, a name with names.
        for test, words in (({"col": "Weight", "is": "0"}, "over a column of kind number needs numbers"),
                            ({"col": "Weight", "is": True}, "over a column of kind number needs numbers"),
                            ({"col": "Weight", "in": [0, "1"]}, "over a column of kind number needs numbers"),
                            ({"col": "RevelationMode", "is": 5}, "over a column of kind tag needs strings"),
                            ({"col": "RevelationMode", "in": ["Revelation.Book", True]}, "over a column of kind tag needs strings"),
                            ({"col": "DisplayName", "is": 0}, "over a column of kind text needs strings")):
            with self.subTest(test=test):
                self.schema_problem(self.put(rule, "when", [test]), "schema.DT_Civs", words)
        self.schema_problem(self.put(["tables", "DT_Events", "rules", 0], "when", [{"col": "Trigger", "is": 1}]), "schema.DT_Events", "over a column of kind enum needs strings")
        self.schema_problem(self.put(["tables", "DT_Rules"], "rules", [{"why": "w", "then": [{"col": "PlayerArchetypeRow", "is": 5}]}]),
                            "schema.DT_Rules", "over a column of kind row needs strings")

    def test_lists_of_the_checker_match_the_schema_text(self):
        """The two closed lists as the schema spells them out for its reader are the lists of the checker, name by name."""
        def listed(start):
            lines = [line for line in self.schema["about"] if line.startswith(start)]
            self.assertEqual(len(lines), 1, start)
            return set(lines[0][len(start):].split(", "))
        self.assertEqual(listed("Column kinds (closed list): "), set(checker.COLUMN_KINDS))
        self.assertEqual(listed("Row tests (closed list): "), set(checker.TESTS))
        self.assertEqual(set(checker.TEST_KINDS), set(checker.TESTS))

    def test_no_shape_of_schema_or_data_raises(self):
        """Every node of the schema and of the tables replaced by a value of another shape, and taken out: the answer
        is a list of lines every time, never an exception. (A traceback names no file and no row.)"""
        wrong = (None, 5, "x", [], {"a": 1})
        for name, root in (("schema", self.schema), ("tables", self.tables)):
            for path in paths_of(root):
                for value, delete in [(value, False) for value in wrong] + [(None, True)]:
                    changed = with_change(root, path, value, delete)
                    try:
                        problems = checker.check(changed, self.tables, self.tags) if name == "schema" else checker.check(self.schema, changed, self.tags)
                    except Exception as error:  # noqa: BLE001  (any exception is the failure looked for)
                        self.fail("%s%r %s raised %r" % (name, path, "taken out" if delete else "= %r" % (value,), error))
                    self.assertIsInstance(problems, list)

    # ---- every rule of the schema of the project ----

    def sample(self, kind, count):
        """count entries of a column of the kind, each valid by itself."""
        if kind == "texts":
            return ["{Civ} text %s." % letter for letter in "abcdefgh"[:count]]
        if kind == "effects":
            return [eff("ChangeStability", value=0.001)] * count
        if kind == "conditions":
            return [cond("GalaxyTickAtLeast", value=1)] * count
        if kind == "tag_map":
            return {tag: 1.0 for tag in sorted(tag for tag in self.tags if tag.startswith("Event.Civ."))[:count]}
        self.fail("the walker has no sample for a column of kind %s: add one" % kind)

    def violate(self, schema, table, test, row):
        """Change row so that test no longer holds, by the kind of its column."""
        column = test["col"]
        spec = schema["tables"][table]["columns"][column]
        kind = spec["kind"]
        name = next(key for key in test if key != "col")
        operand = test[name]
        if name in ("is", "in"):
            taken = operand if name == "in" else [operand]
            if kind == "number":
                row[column] = max(taken) + 1
            elif kind == "tag":
                row[column] = next(tag for tag in sorted(self.tags) if tag not in taken)
            elif kind == "enum":
                row[column] = next(entry for entry in schema["enums"][spec["enum"]] if entry not in taken)
            else:
                self.fail("the walker cannot break %s over a column of kind %s: add the case" % (name, kind))
        elif name in ("gt", "lt"):
            row[column] = operand
        elif name == "ge":
            row[column] = operand - 1
        elif name == "le":
            row[column] = operand + 1
        elif name == "min_len":
            row[column] = self.sample(kind, operand - 1)
        elif name == "max_len":
            row[column] = self.sample(kind, operand + 1)
        elif name == "each_contains":
            row[column] = ["nothing of the kind"]
        elif name == "lacks_type":
            row[column] = list(row[column]) + [cond(operand) if kind == "conditions" else eff(operand)]
        elif name == "slots_in":
            row[column] = ["{NoSuchSlot} a.", "b.", "c."] if kind == "texts" else "{NoSuchSlot}"
        else:
            self.fail("the walker cannot break the test %s: add the case" % name)

    def add_sign_row(self, tables):
        row = copy.deepcopy(self.row(tables, "DT_LogTemplates", "log_breakthrough_physics"))
        row.update(Name="log_sign_test", Category="Log.Sign", Templates=["{Civ} a.", "{System} b.", "{HomeSystem} c."])
        tables["DT_LogTemplates"].append(row)
        self.row(tables, "DT_Events", "event_test_harvest").update(SignRow="log_sign_test", MinTierToReveal=3)

    def add_independent_row(self, tables):
        tables["DT_Transcendents"].append(transcendent_row(
            "trans_stillness", Doctrine="Doctrine.Stillness", Allegiance="Realm.Independent", SpawnWeight=0.0, PressureEffectsPerTick=[]))

    def rule_setups(self):
        """Why of a rule -> (what to change so that its 'when' holds, the row it then holds for). A rule of the schema
        without an entry here fails the walk below."""
        event = "event_test_harvest"
        update_event = lambda **changes: (lambda tables: self.row(tables, "DT_Events", event).update(**changes))  # noqa: E731
        named = update_event(Trigger="Named", BaseWeight=0, CooldownTicks=0, LogTemplates=["one."])
        return {
            ("DT_Civs", "the name pattern of a people drawn for the AI calls its home star (guideline 5.11)"): (NOTHING, "arch_default"),
            ("DT_Events", "a slot row tells with three variants or more (guideline 8.2)"): (NOTHING, event),
            ("DT_Events", "a row fired by name tells with one variant or more (guideline 8.2)"): (named, event),
            ("DT_Events", "a slot runs for every people: until the selector words are decided it carries no condition on the vessel, no role effect and no effect on the vessel (guideline 16 #29 (7))"): (NOTHING, event),
            ("DT_Events", "only a Lottery row has a weight"): (update_event(Trigger="Standing", BaseWeight=0), event),
            ("DT_Events", "a row fired by name has no cooldown key"): (named, event),
            ("DT_Events", "a sign row is told below MinTierToReveal, so there has to be a tier below it"): (self.add_sign_row, event),
            ("DT_Events", "a grace row names no one (guideline 9.3: NameNotable is not carried by Event.Grace rows)"):
                (update_event(Category="Event.Grace.Jubilee", LogCategory="Log.Civ"), event),
            ("DT_Transcendents", "the fixed place of the roster belongs to a fallen silence"): (NOTHING, "trans_silence"),
            ("DT_Transcendents", "a transcendent stands on tier 3 or 4 (guideline 4.2: no fallen one on tier 5)"): (NOTHING, "trans_silence"),
            ("DT_Transcendents", "a celestial transcendent has no instance: no sphere, no pressure, no roster weight"): (NOTHING, "trans_covenant"),
            ("DT_Transcendents", "the roster of NewGalaxy draws fallen rows only"): (self.add_independent_row, "trans_stillness"),
        }

    def test_every_rule_of_the_schema_can_fail(self):
        setups = self.rule_setups()
        walked = 0
        for table, spec in self.schema["tables"].items():
            for rule in spec.get("rules", []):
                self.assertIn((table, rule["why"]), setups, "a rule of schema.json has no input that breaks it: add it to rule_setups")
                setup, row_name = setups[(table, rule["why"])]
                self.assertEqual(self.after(setup), [], "the setup of the rule %r has to pass by itself" % rule["why"])
                for test in rule["then"]:
                    def change(tables, setup=setup, test=test, table=table, row_name=row_name):
                        setup(tables)
                        self.violate(self.schema, table, test, self.row(tables, table, row_name))
                    with self.subTest(table=table, why=rule["why"], test=test):
                        self.expect(self.after(change), "%s.%s" % (table, row_name), rule["why"], json.dumps(test))
                        walked += 1
        # Exact on purpose: a then-test taken out of a rule changes the count, and so does one put in.
        self.assertEqual(walked, 28, "the number of then-tests in schema.json changed: look at what was added or taken out, then set the count")
        self.assertEqual({key for key in setups if key[0] in self.schema["tables"]} - {(t, r["why"]) for t, s in self.schema["tables"].items() for r in s.get("rules", [])},
                         set(), "rule_setups has an entry for a rule the schema no longer has")

    def test_every_unique_of_the_schema_can_fail(self):
        def two_selectors(tables):
            for name in ("log_sign_book", "log_sign_book_again"):
                row = copy.deepcopy(self.row(tables, "DT_LogTemplates", "log_breakthrough_physics"))
                row.update(Name=name, Category="Log.Sign", Context="Revelation.Book")
                tables["DT_LogTemplates"].append(row)
        breakers = {
            ("DT_LogTemplates", "at most one selector row per category and revelation mode"): two_selectors,
            ("DT_Transcendents", "one transcendent per doctrine in a run (guideline 4.2)"): lambda tables: tables["DT_Transcendents"].append(transcendent_row("trans_echo")),
        }
        walked = 0
        for table, spec in self.schema["tables"].items():
            for unique in spec.get("unique", []):
                self.assertIn((table, unique["why"]), breakers, "a unique of schema.json has no input that breaks it: add it here")
                self.expect(self.after(breakers[(table, unique["why"])]), table, unique["why"])
                walked += 1
        self.assertEqual(walked, len(breakers))

    def test_every_pair_of_the_schema_can_fail(self):
        logs = sorted(tag for tag in self.tags if tag.startswith("Log."))
        walked = 0
        for table, spec in self.schema["tables"].items():
            for pair in spec.get("pairs", []):
                self.assertEqual((table, pair["key"], pair["value"]), ("DT_Events", "Category", "LogCategory"), "a new pair needs its walk here")
                for key, allowed in self.schema[pair["allowed"]].items():
                    other = next(tag for tag in logs if tag not in allowed)
                    # Named, so that the rules of slot rows and of grace rows stay out of the way.
                    change = lambda tables, key=key, other=other: self.row(tables, table, "event_test_harvest").update(  # noqa: E731
                        Category=key, LogCategory=other, Trigger="Named", BaseWeight=0, CooldownTicks=0)
                    with self.subTest(category=key):
                        self.expect(self.after(change), "event_test_harvest.LogCategory", "%s may tell %s, found %s" % (key, ", ".join(allowed), other))
                        for good in allowed:
                            fine = lambda tables, key=key, good=good: self.row(tables, table, "event_test_harvest").update(  # noqa: E731
                                Category=key, LogCategory=good, Trigger="Named", BaseWeight=0, CooldownTicks=0)
                            self.assertEqual(self.after(fine), [])
                        walked += 1
        self.assertEqual(walked, 9, "the number of event categories in schema.json changed: look at what was added or taken out, then set the count")

    def test_every_target_of_the_schema_can_fail(self):
        def relation_to_default(tables):
            self.row(tables, "DT_Civs", "arch_terran")["InitialRelations"] = {"arch_default": 0.5}

        def condition_on(type_name):
            return lambda tables: self.row(tables, "DT_Events", "event_test_harvest").update(Conditions=[cond(type_name, key="trans_silence")])
        # Where a target stands -> (what makes the reference, the table and the row it then points at).
        setups = {
            "DT_Civs.InitialRelations": (relation_to_default, "DT_Civs", "arch_default"),
            "DT_Events.SignRow": (self.add_sign_row, "DT_LogTemplates", "log_sign_test"),
            "conditions.SystemInfluencedBy": (condition_on("SystemInfluencedBy"), "DT_Transcendents", "trans_silence"),
            "conditions.TranscendentSealed": (condition_on("TranscendentSealed"), "DT_Transcendents", "trans_silence"),
            "conditions.TranscendentAwareOfPlayer": (condition_on("TranscendentAwareOfPlayer"), "DT_Transcendents", "trans_silence"),
        }
        targets = {}
        for table, spec in self.schema["tables"].items():
            for column, column_spec in (spec["columns"] or {}).items():
                if "target" in column_spec:
                    targets["%s.%s" % (table, column)] = column_spec["target"]
        for kind in ("conditions", "effects"):
            for type_name, usage in self.schema[kind].items():
                if "target" in usage.get("key", {}):
                    targets["%s.%s" % (kind, type_name)] = usage["key"]["target"]
        self.assertEqual(set(targets), set(setups), "a target of schema.json has no input that breaks it (or the other way round)")
        for where, tests in targets.items():
            setup, table, row_name = setups[where]
            self.assertEqual(self.after(setup), [], where)
            for test in tests:
                def change(tables, setup=setup, test=test, table=table, row_name=row_name):
                    setup(tables)
                    self.violate(self.schema, table, test, self.row(tables, table, row_name))
                with self.subTest(where=where, test=test):
                    self.expect(self.after(change), "row %s of %s does not hold %s" % (row_name, table, json.dumps(test)))

    def test_every_slot_list_of_the_schema_can_fail(self):
        walked = 0
        for table, spec in self.schema["tables"].items():
            for column, column_spec in (spec["columns"] or {}).items():
                if "slots" not in column_spec:
                    continue
                row_name = self.tables[table][0]["Name"]
                value = "{NoSuchSlot}" if column_spec["kind"] == "text" else ["{NoSuchSlot} a.", "b.", "c."]
                with self.subTest(table=table, column=column):
                    self.expect(self.after(self.set_cell(table, row_name, column, value)), "%s.%s.%s" % (table, row_name, column), "slot {NoSuchSlot}")
                    walked += 1
        self.assertEqual(walked, 6, "the number of slot lists in schema.json changed: look at what was added or taken out, then set the count")


if __name__ == "__main__":
    unittest.main(verbosity=1)
