"""Unit tests of Tools/cosmic_data_schema.py: every check is fed a wrong input and has to report it.

Cosmic Ascension - data layer helper (guideline.md section 7.3, 9.3 Data.Integrity).

  python Tools/test_data_schema.py

The base of every case is the data of the project itself under the schema of the project (it has to pass), with the
rows of seven tables built here instead: DT_Events, DT_Transcendents, DT_Hazards, DT_Megaprojects, DT_Feats, DT_Encounters and DT_Edicts,
whose rows the cases read cell by cell (the project's rows are replaced on purpose). A case changes one thing and names the words the report
has to carry.

The last block walks the schema of the project: every rule, every unique, every pair, every target and every slot list
needs an input here that breaks it. A rule added to schema.json without one fails test_every_rule_of_the_schema_can_fail.

Tools/make_data_tables.py runs this file right after it has checked the sources and before it touches an asset, so a
broken checker stops the build of the assets. For that reason no case here leans on a cell of the project that the
design plans to change: a case that needs a row with certain cells builds that row itself (civ_row, event_row).
"""
import contextlib
import copy
import csv
import io
import json
import os
import shutil
import sys
import tempfile
import unicodedata
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
        "LogTemplates": ["{Civ} one.", "{Civ} two.", "{Civ} three."],
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
        "CounterfeitOf": "Truth.Rest",
        "Title": "the quiet one",
        "Lure": "",
        "Tier": "Tier4_Transcendent",
        "Depth": 1.0,
        "InitialInfluencedSystems": 1,
        "MaxInfluencedSystems": 12,
        "SpawnWeight": 1.0,
        "EventWeightModifiers": {"Event.Civ.Economy": 1.5},
        "PressureEffectsPerTick": [eff("MultiplyIncome", key="Insight", value=0.7)],
        "VoiceTemplates": [],
        "AwarenessTemplates": [],
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


def hazard_row(name="crisis_test_storm", **changes):
    row = {
        "Name": name,
        "HazardTag": "Hazard.Plague",
        "SourceDoctrine": "",
        "DisplayName": "storm",
        "CountdownTicks": 100,
        "SpawnRule": "Draw",
        "SpawnChancePerTick": 0.001,
        "SpawnConditions": [cond("GalaxyTickAtLeast", value=1)],
        "OnExpireEffects": [eff("ChangeStability", value=-0.1)],
        "BranchChance": 0.0,
        "OnExpireBranchEffects": [],
        "OnExpireNeighbourEffects": [],
        "OnResolveEffects": [],
        "RulerResolutions": [],
        "WandererResolutions": [],
        "LogCategory": "Log.Crisis",
        "Severity": 1,
        "OutbreakTemplates": ["{System} a.", "{Civ} b.", "{Hazard} c."],
        "ExpireTemplates": ["{System} d."],
        "ResolveTemplates": ["{System} e."],
        "OutbreakRow": "",
    }
    row.update(changes)
    return row


def feat_row(name="feat_test_climb", **changes):
    row = {
        "Name": name,
        "Title": "climb",
        "MythicNarrative": "a climb.",
        "RequiredTier": "Tier1_Planetary",
        "TargetTier": "Tier2_Stellar",
        "Path": "Ruler",
        "CompletionConditions": [cond("CivResearchAtLeast", key="Physics", value=0.5)],
        "AscensionCost": [],
        "GrantedPowers": "(GameplayTags=)",
        "DedicationBank": "",
        "FX": "",
        "Sound": "",
        "LogTemplates": ["{Civ} a.", "{HomeSystem} b.", "c."],
    }
    row.update(changes)
    return row


def megaproject_row(name="project_test_build", **changes):
    row = {
        "Name": name,
        "DisplayName": "build",
        "RequiredTier": "Tier1_Planetary",
        "CostPerTick": {"Energy": 1.0, "Matter": 0.0, "Insight": 0.0, "Influence": 0.0, "Causality": 0.0},
        "RequiredProgress": 10.0,
        "CompleteTemplates": ["{Civ} a.", "{System} b.", "{Project} c."],
    }
    row.update(changes)
    return row


def pool(**amounts):
    """A resource pool: every resource, 0 unless named (B4: a choice that costs)."""
    full = {"Energy": 0.0, "Matter": 0.0, "Insight": 0.0, "Influence": 0.0, "Causality": 0.0}
    full.update(amounts)
    return full


def choice(label="Hold.", **changes):
    """One entry of DT_Encounters.Choices: every member of FEncounterChoice."""
    entry = {
        "Label": label,
        "CheckStat": "NoCheck",
        "Difficulty": 0,
        "Cost": {"Energy": 0.0, "Matter": 0.0, "Insight": 0.0, "Influence": 0.0, "Causality": 0.0},
        "OnSuccess": [eff("ChangeStability", value=0.05)],
        "OnFailure": [],
        "SuccessText": "{Civ} held.",
        "FailureText": "",
        "NextEncounter": "",
    }
    entry.update(changes)
    return entry


def encounter_row(name="petition_test_plea", **changes):
    row = {
        "Name": name,
        "Title": "plea",
        "Body": "{Civ} asks.",
        "Conditions": [cond("PlayerRoleIs", key="Ruler")],
        "BaseWeight": 1.0,
        "Choices": [choice(), choice("Wait.", SuccessText="{Civ} waited.")],
        "Illustration": "",
        "Trigger": "Standing",
        "Subject": "Civilization",
        "VoiceBodies": [],
    }
    row.update(changes)
    return row


def wanderer_card_row(name="enc_test_weave", **changes):
    """A wanderer's arrival card (B1): put before a wanderer, about the system the vessel stands in, its texts naming the
    system alone, a check on the vessel."""
    row = encounter_row(
        name, Title="weave", Body="{System} bends.", Conditions=[cond("PlayerRoleIs", key="Wanderer")], Trigger="Lottery", Subject="VesselSystem",
        Choices=[choice("Cut loose.", CheckStat="Body", Difficulty=4, OnSuccess=[eff("PlayerStatDelta", key="Body", value=1)], SuccessText="{System} ahead.",
                        FailureText="{System} at last."),
                 choice("Push on.", OnSuccess=[], SuccessText="{System} later.")])
    row.update(changes)
    return row


def edict_row(name="edict_test_order", **changes):
    """An edict (B2): a name, a line of words, a cost, an upkeep, a per-tick effect and the two hazard maps."""
    row = {
        "Name": name,
        "DisplayName": "order",
        "Summary": "an order stands.",
        "RequiredTier": "Tier1_Planetary",
        "EnactConditions": [],
        "EnactCost": {"Energy": 0.0, "Matter": 0.0, "Insight": 0.0, "Influence": 10.0, "Causality": 0.0},
        "UpkeepPerTick": {"Energy": 0.0, "Matter": 0.0, "Insight": 0.0, "Influence": 0.1, "Causality": 0.0},
        "EffectsPerTick": [eff("ChangeStability", value=-0.001)],
        "HazardSpawnMultipliers": {"Hazard.Plague": 2.0},
        "HazardCountdownBonus": {"Hazard.Claim": 100},
        "FleetGrowthMultiplier": 1.0,
        "TreatyAcceptBonus": 0.0,
        "PressureDrainMultiplier": 1.0,
        "Scope": "People",
    }
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


def civ_row(base, name, **cells):
    """An archetype row of the test: every column of base (a row of the project, for the columns the struct has), with
    the cells the cases read set here, so that no case depends on what the project writes into them."""
    row = copy.deepcopy(base)
    row.update(Name=name, DisplayName="plain", Traits=tags_text("Trait.Arch.Default"), AIProfile="", NameBank=["union"], AdjectiveBank=[],
               PersonNameBank=[], HazardSpawnMultipliers={}, InitialRelations={}, Weight=0.0, RevelationMode="Revelation.Seeking",
               HomeSystemNames=["Home"], HomeStarClass="", DedicationBanks=[])
    row.update(cells)
    return row


def fixture_tables(schema, tables):
    """The tables every case starts from: the sources of the project, with fixture rows for DT_Events, DT_Transcendents,
    DT_Hazards, DT_Megaprojects, DT_Feats, DT_Encounters and DT_Edicts (their project rows are replaced on purpose: the cases read the
    cells of the rows they build). A row the code calls by name has to be in its table, so the event and project fixtures get one valid
    row for each name the schema requires (an event fired by name: no rule about slot rows speaks of it), and one outbreak
    row (fired by name about a place, as a hazard's OutbreakRow has to be)."""
    tables = copy.deepcopy(tables)
    tables["DT_Events"] = [event_row()]
    for name in schema["tables"]["DT_Events"].get("required_rows", []):
        tables["DT_Events"].append(event_row(name, Trigger="Named", BaseWeight=0, CooldownTicks=0, LogTemplates=["one."]))
    tables["DT_Events"].append(event_row("event_test_rising", Trigger="Named", Subject="SettledSystem", BaseWeight=0, CooldownTicks=0,
                                         LogTemplates=["{Civ} {System} one."]))
    # B7x [B-7x5c] (the text audit's 7.3): the three brick lines no condition rule reaches - they wait on traits alone or on a
    # trait and a hazard - take row rules by their names, and a rule's row has to be a row of the table.
    for name in ("event_brick_shut_star", "event_brick_drift_plenty", "event_brick_ember_wake"):
        tables["DT_Events"].append(event_row(name))
    tables["DT_Transcendents"] = [transcendent_row(), celestial_row()]
    tables["DT_Hazards"] = [hazard_row()]
    tables["DT_Megaprojects"] = [megaproject_row()] + [megaproject_row(name) for name in schema["tables"]["DT_Megaprojects"].get("required_rows", [])]
    # B6b [B-6b6]: the ark and the fourth-rung feat - rows the code reads by tag or by table walk, not by name, which the
    # schema's row rules name (the ark's own rule and the two row_not rules of its word): a rule's row has to be a row of
    # the table. The feat is feat_test_climb's copy one rung up.
    tables["DT_Megaprojects"].append(megaproject_row("project_ark", RequiredTier="Tier3_Causal"))
    tables["DT_Feats"] = [feat_row(), feat_row("feat_passage_by_ark", RequiredTier="Tier3_Causal", TargetTier="Tier4_Transcendent")]
    # The first contact card (B3) is put by name: a Named row without weight. A wanderer's card the code calls by name (B5:
    # the epitaph leaves it out) is a wanderer's card of its own kind.
    tables["DT_Encounters"] = [encounter_row(), wanderer_card_row()] + [
        encounter_row(name, Trigger="Named", BaseWeight=0) if name == "petition_first_contact"
        else wanderer_card_row(name) if name.startswith("enc_") else encounter_row(name)
        for name in schema["tables"]["DT_Encounters"].get("required_rows", [])]
    tables["DT_Edicts"] = [edict_row()]
    # The inherited edict of the project's rules has to be a row of the fixture's table (none in the device commit of B2).
    if tables["DT_Rules"][0].get("StartingEdict") not in (None, "", "None"):
        tables["DT_Edicts"].append(edict_row(tables["DT_Rules"][0]["StartingEdict"]))
    return tables


def texts_of_project(sources):
    """Every string of the project's DT_*.json cells and every text of its String Tables (B4: a check over the words that
    stand together in one text)."""
    found = []

    def walk(value):
        if isinstance(value, str):
            found.append(value)
        elif isinstance(value, list):
            for entry in value:
                walk(entry)
        elif isinstance(value, dict):
            for entry in value.values():
                walk(entry)
    for name, data in sorted(sources.items()):
        if name.endswith(".json") and name != "schema.json":
            walk(json.loads(data.decode("utf-8")))
        elif name.endswith(".csv"):
            found.extend(row[1] for row in csv.reader(io.StringIO(data.decode("utf-8"))) if len(row) > 1)
    return found


@contextlib.contextmanager
def project_copy():
    """A copy of what the checker reads, in a temporary folder: the data, the tags ini (and the files under Config/Tags
    when there are any), the header of row names."""
    work = tempfile.mkdtemp(prefix="ca_schema_test_")
    try:
        root = checker.project_root()
        os.makedirs(os.path.join(work, "Config"))
        shutil.copy(os.path.join(root, "Config", "DefaultGameplayTags.ini"), os.path.join(work, "Config"))
        if os.path.isdir(os.path.join(root, "Config", "Tags")):
            shutil.copytree(os.path.join(root, "Config", "Tags"), os.path.join(work, "Config", "Tags"))
        shutil.copytree(os.path.join(root, "Tools", "data"), os.path.join(work, "Tools", "data"))
        os.makedirs(os.path.dirname(os.path.join(work, ROW_HEADER)))
        shutil.copy(os.path.join(root, ROW_HEADER), os.path.join(work, ROW_HEADER))
        yield work
    finally:
        shutil.rmtree(work, ignore_errors=True)


class SchemaTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.schema, project_tables, cls.tags = checker.load_project()
        cls.tables = fixture_tables(cls.schema, project_tables)

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

    def expect(self, problems, *words, wanted=True):
        """Some problem line carries all the words (wanted=False: none does)."""
        found = any(all(word in line for word in words) for line in problems)
        self.assertEqual(found, wanted, "%s %r among:\n  %s" % ("no problem carries" if wanted else "a problem carries", words, "\n  ".join(problems) or "(none)"))

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

    def columns_of(self, takes):
        """(table, column, spec) of every column of the schema that takes() accepts, in order; a column of a list of
        structs as 'List.Column'."""
        found = []
        for table, table_spec in sorted(self.schema["tables"].items()):
            for column, spec in sorted((table_spec.get("columns") or {}).items()):
                if spec.get("kind") == "structs":
                    found.extend((table, "%s.%s" % (column, inner), inner_spec) for inner, inner_spec in sorted(spec["columns"].items()) if takes(inner_spec))
                elif takes(spec):
                    found.append((table, column, spec))
        return found

    def set_path(self, table, name, path, value):
        """(a change of one cell, where its problems are said): for 'List.Column' the cell of the first entry."""
        head, mark, sub = path.partition(".")

        def change(tables):
            row = self.row(tables, table, name)
            if mark:
                row[head][0][sub] = value
            else:
                row[head] = value
        return change, "%s.%s.%s" % (table, name, "%s[0].%s" % (head, sub) if mark else head)

    def schema_problem(self, schema_change, *words):
        """A change of the schema is a problem line, and the data is then not looked at."""
        problems = self.after(NOTHING, schema_change)
        self.expect(problems, *words)
        self.expect(problems, NOT_CHECKED)

    # ---- the base ----

    def test_project_and_fixture_rows_pass(self):
        self.assertEqual(checker.check_project(), [])
        self.assertEqual(self.after(), [])

    def test_fixture_has_the_rows_the_code_calls(self):
        """The day the code calls an event row by name, the schema requires it and the fixture has to carry it."""
        schema = copy.deepcopy(self.schema)
        schema["tables"]["DT_Events"]["required_rows"] = schema["tables"]["DT_Events"].get("required_rows", []) + ["event_called_by_name"]
        self.expect(checker.check(schema, self.tables, self.tags), "DT_Events", "the row event_called_by_name", "is missing")
        self.assertEqual(checker.check(schema, fixture_tables(schema, self.tables), self.tags), [])

    # ---- reading ----

    def test_reading_the_sources(self):
        base = checker.read_sources()

        def with_text(file_name, text):
            """The sources of the project with one file replaced by a text of the test: what is wrong with reading a
            file does not depend on what the project writes into it."""
            sources = dict(base)
            sources[file_name] = text.encode("utf-8")
            return checker.check_sources(sources)
        self.assertEqual(checker.check_sources(base), [])
        broken = with_text("DT_Civs.json", '[{"Name": "arch_default" "Weight": 1}]')
        self.expect(broken, "DT_Civs.json", "not valid JSON")
        self.expect(broken, "nothing was checked")
        self.expect(with_text("schema.json", '{"version": 1, "version": 2}'), "schema.json", "the key 'version' is written twice")
        self.expect(with_text("DT_Rules.json", '[{"Name": "rules", "Knob": 600, "Knob": 601}]'), "DT_Rules.json", "the key 'Knob' is written twice")
        self.expect(with_text("DT_Rules.json", '[{"Name": "rules", "Inner": {"A": 1}, "Other": {"A": 2}}]'), "nothing was checked", wanted=False)
        self.expect(with_text("DT_Rules.json", '[{"Name": "rules", "Knob": NaN}]'), "DT_Rules.json", "NaN is not a number")
        self.expect(with_text("DT_Rules.json", '[{"Name": "rules", "Knob": -Infinity}]'), "-Infinity is not a number")
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
        first_key = next(row[0] for row in list(csv.reader(io.StringIO(checker.text_of(sources["ST_CA.csv"]))))[1:] if row)
        sources["ST_CA.csv"] = sources["ST_CA.csv"].rstrip(b"\r\n") + ('\n"%s","again"\n' % first_key).encode("utf-8")
        self.expect(checker.check_sources(sources), "ST_CA.csv", first_key, "written twice")

    def test_string_table_prefixes(self):
        """The narrator's words of a screen (schema.string_table: the chronicle's keys, MVP item 4, and the save's reason
        lines, MVP item 5) hold the line of the narrator: no verdict word, no digit outside a slot; a key under no prefix is
        not looked at."""
        check = lambda text: checker.check_string_table("ST_X.csv", text, self.schema)  # noqa: E731
        verdict = self.schema["words"]["verdict"]
        # B6b [B-6b5]: a rung grasped or raced for, a title of election and the binding's words join the chronicle (the two
        # books' chapter and the reason lines - decision 33, notes 1.6 rules 1 and 4).
        self.assertEqual(self.schema["string_table"]["checked_prefixes"]["Chronicle."],
                         {"forbidden": ["verdict", "soul_verbs", "rescue_words", "afterlife_words", "book_of_life_words", "called_away_words", "holy_passive",
                                        "rung_grasp_words", "title_words", "binding_words"], "no_digits": True})
        # (1) A slot carries its digit; the text around it does not.
        self.assertEqual(check('Key,SourceString\n"Chronicle.A","{0}주기 {Civ}|hpp(이,가)"\n'), [])
        self.expect(check('Key,SourceString\n"Chronicle.A","3주기"\n'), "entry 1 (Chronicle.A)", "a digit outside a slot")
        # (2) Every verdict word is refused under the prefix.
        for word in verdict:
            with self.subTest(word=word):
                self.expect(check('Key,SourceString\n"Chronicle.A","%s a."\n' % word), "entry 1 (Chronicle.A)", "the word %s is not used here" % word)
        # (3) A key under no prefix is not looked at (a UI label may carry a figure).
        self.assertEqual(check('Key,SourceString\n"Time.A","%s 3"\n' % verdict[0]), [])
        # (4) The project's chronicle keys hold the rule through the sources: a digit put into one is found.
        sources = checker.read_sources()
        sources["ST_CA.csv"] = sources["ST_CA.csv"].rstrip(b"\r\n") + '\n"Chronicle.Test","7대"\n'.encode("utf-8")
        self.expect(checker.check_sources(sources), "ST_CA.csv", "Chronicle.Test", "a digit outside a slot")
        # (5) The rule's shape is the schema's: a key it does not read, a value of the wrong type, a list it lacks.
        prefixes = ["string_table", "checked_prefixes"]
        self.schema_problem(self.put(prefixes, "Chronicle.", {"forbidden": "verdict", "digits": True}), "schema.string_table.checked_prefixes.Chronicle.", "'digits' is not read here")
        self.schema_problem(self.put(prefixes, "Chronicle.", {"no_digits": "yes"}), "schema.string_table.checked_prefixes.Chronicle.", "'no_digits' has to be")
        self.schema_problem(self.put(prefixes, "Chronicle.", {"forbidden": "verdicts"}), "the word list 'verdicts' is not in the schema")
        self.schema_problem(self.put(["string_table"], "prefixes", {}), "schema.string_table", "'prefixes' is not read here")
        # (6) The save's reason lines (MVP item 5) are under the same rule: a digit put into one of the project's Save. keys
        # is found, and so is a verdict word.
        self.assertEqual(self.schema["string_table"]["checked_prefixes"]["Save."], {"forbidden": "verdict", "no_digits": True})
        sources = checker.read_sources()
        sources["ST_CA.csv"] = sources["ST_CA.csv"].rstrip(b"\r\n") + '\n"Save.Test","정지 — 2번째 저장"\n'.encode("utf-8")
        self.expect(checker.check_sources(sources), "ST_CA.csv", "Save.Test", "a digit outside a slot")
        self.expect(check('Key,SourceString\n"Save.A","%s a."\n' % verdict[0]), "entry 1 (Save.A)", "the word %s is not used here" % verdict[0])
        # (7) The first hints (MVP item 6) are under the same rule: the Hint. keys and the galaxy panel's Galaxy.Hint* keys;
        # (8) and so are the research focus's words (MVP item 7): the Focus. keys.
        for prefix in ("Hint.", "Galaxy.Hint", "Focus."):
            with self.subTest(prefix=prefix):
                spec = self.schema["string_table"]["checked_prefixes"][prefix]
                self.assertTrue("verdict" in checker.forbidden_lists(spec) and spec.get("no_digits") is True, spec)
                key = prefix + "Test"
                sources = checker.read_sources()
                sources["ST_CA.csv"] = sources["ST_CA.csv"].rstrip(b"\r\n") + ('\n"%s","숫자 키 5"\n' % key).encode("utf-8")
                self.expect(checker.check_sources(sources), "ST_CA.csv", key, "a digit outside a slot")
                self.expect(check('Key,SourceString\n"%s","%s a."\n' % (key, verdict[0])), "entry 1 (%s)" % key, "the word %s is not used here" % verdict[0])

    def test_tags_ini(self):
        with project_copy() as work:
            ini = os.path.join(work, "Config", "DefaultGameplayTags.ini")
            with open(ini, "w", encoding="utf-8") as handle:
                handle.write('[/Script/GameplayTags.GameplayTagsSettings]\n'
                             '+GameplayTagList=(Tag="Test.One",DevComment="counted")\n'
                             ';+GameplayTagList=(Tag="Test.Commented",DevComment="not a row")\n'
                             'GameplayTagList=(Tag="Test.Bare",DevComment="replaces the list")\n')
                handle.write('[/Script/Engine.SomethingElse]\n'
                             '+GameplayTagList=(Tag="Test.WrongSection",DevComment="not the section of the tag settings")\n')
            shutil.rmtree(os.path.join(work, "Config", "Tags"), ignore_errors=True)
            os.makedirs(os.path.join(work, "Config", "Tags", "Sub"))
            with open(os.path.join(work, "Config", "Tags", "Extra.ini"), "w", encoding="utf-8") as handle:
                handle.write('GameplayTagList=(Tag="Test.NoSection",DevComment="before any section")\n'
                             '[/Script/GameplayTags.GameplayTagsList]\n'
                             'GameplayTagList=(Tag="Test.Extra",DevComment="counted")\n'
                             '+GameplayTagList=(Tag="Test.Ignored",DevComment="the engine ignores it")\n')
            with open(os.path.join(work, "Config", "Tags", "Sub", "Deep.ini"), "w", encoding="utf-8") as handle:
                handle.write('[/Script/GameplayTags.GameplayTagsList]\n'
                             'GameplayTagList=(Tag="Test.Deep",DevComment="counted: the engine looks into subfolders")\n'
                             '[/Script/GameplayTags.GameplayTagsSettings]\n'
                             'GameplayTagList=(Tag="Test.SettingsSection",DevComment="the section of the Default ini, in the wrong file")\n')
            tags, problems = checker.load_declared_tags(work, ["Config/DefaultGameplayTags.ini"])
            # The files under Config/Tags are found without being listed, as the engine finds them.
            self.assertEqual(tags, {"Test.One", "Test.Extra", "Test.Deep"})
            self.expect(problems, "DefaultGameplayTags.ini:4", "without '+'", "Test.Bare")
            self.expect(problems, "DefaultGameplayTags.ini:6", "under [/Script/GameplayTags.GameplayTagsSettings] only", "Test.WrongSection")
            self.expect(problems, "Config/Tags/Extra.ini:1", "under [/Script/GameplayTags.GameplayTagsList] only", "Test.NoSection")
            self.expect(problems, "Config/Tags/Extra.ini:4", "ignores a row with '+'", "Test.Ignored")
            self.expect(problems, "Config/Tags/Sub/Deep.ini:4", "under [/Script/GameplayTags.GameplayTagsList] only", "Test.SettingsSection")
            self.assertEqual(len(problems), 5)
            tags, problems = checker.load_declared_tags(work, ["Config/Nowhere.ini"])
            self.expect(problems, "Config/Nowhere.ini", "cannot be read")
            # Through the project: the bare row is a problem line of the whole check.
            self.expect(checker.check_project(work), "without '+'", "Test.Bare")

    def test_rows_the_code_calls_by_name(self):
        root = checker.project_root()
        code_rows, problems = checker.load_code_rows(root, self.schema["row_name_sources"])
        self.assertEqual(problems, [])
        self.assertIn("arch_default", code_rows)
        self.assertIn("project_settle", code_rows)
        self.assertEqual(self.after(code_rows=code_rows), [])
        self.schema_problem_with_rows(code_rows + ["log_new_line"], "schema.DT_LogTemplates", "the code calls the row log_new_line by name")
        # A table with "names" asks for its names only: a code row that merely starts like one is none of its business.
        self.schema_problem_with_rows(code_rows + ["rules_two"], "schema.DT_Rules", "the code calls the row rules_two", wanted=False)
        problems = self.after(NOTHING, self.drop(["tables", "DT_Rules"], "required_rows"), code_rows)
        self.expect(problems, "schema.DT_Rules", "the code calls the row rules by name and required_rows does not list it")
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
        # A constant that is commented out is not called.
        with project_copy() as work:
            with open(os.path.join(work, ROW_HEADER), "a", encoding="utf-8") as handle:
                handle.write('\n// inline const FName LogOld(TEXT("log_old_line"));\n/* inline const FName LogOlder(\n    TEXT("log_older_line")); */\n')
            self.assertEqual(checker.check_project(work), [])

    def test_epitaph_leaves_out_every_title_that_speaks_to_the_reader(self):
        """B5 [B-5c] (header section 4, decision 12-da): the epitaph shows the titles of a dead vessel's last answers in the
        third person. A card title that speaks to the reader is listed under epitaph_leaves_out - the code's
        CosmicRows::EpitaphLeavesOut, whose rows the code calls by name."""
        listed = self.schema["epitaph_leaves_out"]
        self.assertIn("petition_exalt", listed)
        self.assertIn("petition_keep_throne", listed)
        # The project's own titles: the cards that speak to the reader are listed, and so are the four the [B-5d1] text audit
        # found to read as a eulogy under a dead vessel's name (a virtue, a memorial day, the two 'beside' titles of the
        # sacrifice reading - header 3.6-1), whose titles do not speak to the reader and stay right on their own cards.
        project_schema, project_tables, _ = checker.load_project()
        speaking = [row["Name"] for row in project_tables["DT_Encounters"]
                    if any(word in row["Title"] for word in project_schema["words"]["reader_address"])
                    or any(row["Title"].rstrip().endswith(end) for end in project_schema["words"]["reader_question_ends"])]
        eulogies = ["petition_wise_heart", "petition_feast", "enc_plague_tend", "enc_star_watch"]
        self.assertEqual(set(speaking) & set(eulogies), set())
        self.assertEqual(sorted(speaking + eulogies), sorted(project_schema["epitaph_leaves_out"]))
        word = self.schema["words"]["reader_address"][0]
        where = "DT_Encounters.petition_test_plea.Title"
        why = "speaks to the reader"
        self.expect(self.after(self.set_cell("DT_Encounters", "petition_test_plea", "Title", "plea " + word)), where, why)
        for end in self.schema["words"]["reader_question_ends"]:
            with self.subTest(end=end):
                self.expect(self.after(self.set_cell("DT_Encounters", "petition_test_plea", "Title", "plea" + end)), where, why)
                self.expect(self.after(self.set_cell("DT_Encounters", "petition_test_plea", "Title", "plea" + end + "  ")), where, why)
        # A plain title, and the same words on a listed row, are no problem.
        self.expect(self.after(self.set_cell("DT_Encounters", "petition_test_plea", "Title", "plea")), why, wanted=False)
        self.assertEqual(self.after(self.set_cell("DT_Encounters", "petition_exalt", "Title", "plea " + word)), [])
        self.assertEqual(self.after(self.set_cell("DT_Encounters", "petition_keep_throne", "Title", "plea" + self.schema["words"]["reader_question_ends"][0])), [])
        # The list names rows the code calls by name, the two word lists stand, and the list is there.
        self.schema_problem(self.put([], "epitaph_leaves_out", listed + ["petition_test_plea"]), "schema.epitaph_leaves_out",
                            "petition_test_plea is not a row the code calls by name")
        for words in ("reader_address", "reader_question_ends"):
            with self.subTest(words=words):
                self.schema_problem(self.drop(["words"], words), "schema.epitaph_leaves_out", "the word list %r" % words)
        self.schema_problem(self.drop([], "epitaph_leaves_out"), "schema", "'epitaph_leaves_out' is missing")

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
        self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_seeker", "HomeStarClass", "")), [])
        # The importer reads null and the word None, in any case, as the empty name.
        for empty in (None, "None", "none"):
            self.expect(self.after(self.set_cell("DT_Civs", "arch_default", "RevelationMode", empty)), "RevelationMode", "a tag is required")
            self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_seeker", "HomeStarClass", empty)), [])

    def test_a_single_tag_is_a_string(self):
        """The importer also takes {"TagName": ...}; rules and unique read one spelling, so the other is a problem."""
        as_object = {"TagName": "Revelation.Book"}
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "RevelationMode", as_object)), "RevelationMode", "expected a tag name as a string")
        self.expect(self.after(self.set_cell("DT_Transcendents", "trans_covenant", "Allegiance", {"TagName": "Realm.Celestial"})),
                    "trans_covenant.Allegiance", "expected a tag name as a string")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "DedicationBanks", [{"TagName": "Faith.Bank.Hebrew"}])),
                    "DedicationBanks[0]", "expected a tag name as a string")
        self.expect(self.after(self.set_cell("DT_Events", "event_test_harvest", "Conditions", [cond("CivHasTrait", tag={"TagName": "Trait.Arch.Seeker"})])),
                    "(CivHasTrait).Tag", "expected a tag name as a string")

    def test_parent_tag_is_not_declared(self):
        self.expect(self.after(self.set_cell("DT_Transcendents", "trans_silence", "EventWeightModifiers", {"Event.Civ": 2})),
                    "EventWeightModifiers[Event.Civ]", "not declared")

    def test_tag_list_and_tag_map(self):
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "DedicationBanks", ["Faith.Bank.Nope"])), "DedicationBanks[0]", "not declared")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "DedicationBanks", "Faith.Bank.Hebrew")), "DedicationBanks", "expected a list of tags")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "HazardSpawnMultipliers", {"Trait.Arch.Seeker": 1})), "not under Hazard.")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "HazardSpawnMultipliers", {"Hazard.Plague": -1})), "is below 0")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "HazardSpawnMultipliers", ["Hazard.Plague"])), "HazardSpawnMultipliers", "expected an object")

    def test_tag_container_text(self):
        """Only the export text with every name quoted: the importer drops what it cannot read without a word."""
        cell = lambda text: self.after(self.set_cell("DT_Civs", "arch_default", "Traits", text))  # noqa: E731
        self.assertEqual(cell(tags_text("Trait.Arch.Default")), [])
        self.assertEqual(cell("(GameplayTags=)"), [])
        self.assertEqual(cell(""), [])
        self.expect(cell('(GameplayTags=((TagName="")))'), "arch_default.Traits", "a tag is required")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "DedicationBanks", ["Faith.Bank.Hebrew", ""])), "DedicationBanks[1]", "a tag is required")
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
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "PlayerArchetypeRow", ["arch_seeker"])), "PlayerArchetypeRow", "expected a row name")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "InitialRelations", {"arch_nobody": 0.5})), "has no row arch_nobody")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "InitialRelations", {"arch_default": 2})), "is above 1")
        self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_seeker", "InitialRelations", {"arch_default": 0.5})), [])

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
        later = lambda schema: schema["tables"].update(DT_Later={"struct": "LaterDef", "prefixes": ["later_"], "columns": None})  # noqa: E731
        self.expect(self.after(lambda tables: tables.update(DT_Later=[]), later), "DT_Later", "declares no columns")
        self.assertEqual(self.after(NOTHING, later), [])
        self.expect(self.after(lambda tables: tables.update(DT_Nobody=[])), "DT_Nobody", "does not declare this table")
        self.expect(self.after(lambda tables: tables["DT_Events"].append(transcendent_row("event_wrong"))), "does not declare this column")
        # A table that need not exist may be absent; one that has to exist may not. The flag is set here, so the case
        # stays right when the schema of the project turns it on for a table that got its asset.
        pop_events = lambda tables: tables.pop("DT_Events")  # noqa: E731
        # B7x [B-7x5c]: the brick lines' row rules name rows the table has, not rows it requires (the code calls none of them),
        # so an absent table takes those rules with it - or the checker refuses the schema before it looks at the data.
        unnamed = lambda schema: schema["tables"]["DT_Events"].update(rules=[  # noqa: E731
            rule for rule in schema["tables"]["DT_Events"]["rules"] if not any(str(test.get("row", "")).startswith("event_brick_") for test in rule.get("when", []))])
        may_go = lambda flag: (lambda schema: (self.put(["tables", "DT_Events"], "must_exist", flag)(schema), unnamed(schema)))  # noqa: E731
        self.assertEqual(self.after(pop_events, may_go(False)), [])
        self.expect(self.after(pop_events, may_go(True)), "DT_Events.json is missing")
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
        # The knobs of knowing: the sight stage looks at least every tick, from tick 0 on, and the far line waits no less
        # than nothing (the code reads a value below as the bound, without a line).
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "SightIntervalTicks", 0)), "rules.SightIntervalTicks", "is below 1")
        self.assertEqual(self.after(self.set_cell("DT_Rules", "rules", "SightIntervalTicks", 1)), [])
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "FirstSightTick", -1)), "rules.FirstSightTick", "is below 0")
        self.assertEqual(self.after(self.set_cell("DT_Rules", "rules", "FirstSightTick", 0)), [])
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "FarLineCooldownTicks", -1)), "rules.FarLineCooldownTicks", "is below 0")
        self.assertEqual(self.after(self.set_cell("DT_Rules", "rules", "FarLineCooldownTicks", 0)), [])
        # The backdrop's count (Phase 3 item 8): the map reads it as 0..100,000 - both sides of both bounds by hand.
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "BackgroundStarCount", -1)), "rules.BackgroundStarCount", "is below 0")
        self.assertEqual(self.after(self.set_cell("DT_Rules", "rules", "BackgroundStarCount", 0)), [])
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "BackgroundStarCount", 100001)), "rules.BackgroundStarCount", "is above 100000")
        self.assertEqual(self.after(self.set_cell("DT_Rules", "rules", "BackgroundStarCount", 100000)), [])
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "BackgroundStarCount", 20000.5)), "rules.BackgroundStarCount", "not a whole number")
        # The scenery's count (Phase 3 item 9): the universe level reads it as 0..50,000 - both sides of both bounds by hand.
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "SceneryGalaxyCount", -1)), "rules.SceneryGalaxyCount", "is below 0")
        self.assertEqual(self.after(self.set_cell("DT_Rules", "rules", "SceneryGalaxyCount", 0)), [])
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "SceneryGalaxyCount", 50001)), "rules.SceneryGalaxyCount", "is above 50000")
        self.assertEqual(self.after(self.set_cell("DT_Rules", "rules", "SceneryGalaxyCount", 50000)), [])
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "SceneryGalaxyCount", 5000.5)), "rules.SceneryGalaxyCount", "not a whole number")
        # The ruler's research focus (MVP item 7): a share that leans clearly (0.3..1) - both sides of both bounds by hand. The
        # code refuses a field command at or below 1/4 and above 1 with a line of its own (Sim.ResearchFocus).
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "ResearchFocusShare", 0.29)), "rules.ResearchFocusShare", "is below 0.3")
        self.assertEqual(self.after(self.set_cell("DT_Rules", "rules", "ResearchFocusShare", 0.3)), [])
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "ResearchFocusShare", 1.01)), "rules.ResearchFocusShare", "is above 1")
        self.assertEqual(self.after(self.set_cell("DT_Rules", "rules", "ResearchFocusShare", 1)), [])
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "ResearchFocusShare", NAN)), "rules.ResearchFocusShare", "finite number")
        # The economy and research of every people (MVP item 8): no knob below 0, five rungs of multipliers, a research
        # share of at most 1 - both sides of each bound by hand (0 stays a value: a test may lock an income).
        for knob in ("MaterialistEnergyBonus", "InsightPerPopulation", "SpiritualistInsightBonus", "InfluencePerPopulation",
                     "OpenInfluenceBonus", "ResearchInsightShare", "ResearchPerInsight"):
            with self.subTest(knob=knob):
                self.expect(self.after(self.set_cell("DT_Rules", "rules", knob, -0.1)), "rules." + knob, "is below 0")
                self.assertEqual(self.after(self.set_cell("DT_Rules", "rules", knob, 0)), [])
                self.expect(self.after(self.set_cell("DT_Rules", "rules", knob, NAN)), "rules." + knob, "finite number")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "ResearchInsightShare", 1.01)), "rules.ResearchInsightShare", "is above 1")
        self.assertEqual(self.after(self.set_cell("DT_Rules", "rules", "ResearchInsightShare", 1)), [])
        # The wanderer's loop (B1): a lane takes at least one whole tick, a scan adds no fewer than 0 lanes, the arrival
        # share and the three thresholds lie in 0..1 - both sides of each bound by hand.
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "LaneTravelTicks", 0)), "rules.LaneTravelTicks", "is below 1")
        self.assertEqual(self.after(self.set_cell("DT_Rules", "rules", "LaneTravelTicks", 1)), [])
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "LaneTravelTicks", 1.5)), "rules.LaneTravelTicks", "is not a whole number")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "ScanSightBonusLanes", -1)), "rules.ScanSightBonusLanes", "is below 0")
        self.assertEqual(self.after(self.set_cell("DT_Rules", "rules", "ScanSightBonusLanes", 0)), [])
        for knob in ("EncounterChancePerArrival", "CoronationReverence", "CoronationStabilityBelow", "AbdicationStabilityLoss"):
            with self.subTest(knob=knob):
                self.expect(self.after(self.set_cell("DT_Rules", "rules", knob, -0.01)), "rules." + knob, "is below 0")
                self.assertEqual(self.after(self.set_cell("DT_Rules", "rules", knob, 0)), [])
                self.expect(self.after(self.set_cell("DT_Rules", "rules", knob, 1.01)), "rules." + knob, "is above 1")
                self.assertEqual(self.after(self.set_cell("DT_Rules", "rules", knob, 1)), [])
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "TierMultiplier", [1.0, 1.5, 2.2, 3.0])), "rules.TierMultiplier", "exactly 5 entries, found 4")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "TierMultiplier", [1.0, 1.5, -2.2, 3.0, 3.0])), "TierMultiplier[2]", "is below 0")
        self.assertEqual(self.after(self.set_cell("DT_Rules", "rules", "TierMultiplier", [0.0, 1.5, 2.2, 3.0, 3.0])), [])
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

    def test_a_cumulative_share_ends_at_its_mark(self):
        shares = [0.01, 0.05, 0.14, 0.31, 0.57, 0.82, 1.0]
        self.assertEqual(self.after(self.set_cell("DT_Rules", "rules", "LivingWorldStarShares", shares)), [])
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "LivingWorldStarShares", shares[:6] + [0.9])), "LivingWorldStarShares", "the last entry has to be 1, found 0.9")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "LivingWorldStarShares", shares[:6] + ["1"])), "LivingWorldStarShares", "the last entry has to be 1")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "LivingWorldStarShares", [])), "LivingWorldStarShares", "the last entry has to be 1, found None")
        # The other options of the column stay what they were.
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "LivingWorldStarShares", shares[:5] + [0.5, 1.0])), "do not rise")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "LivingWorldStarShares", shares[1:])), "exactly 7 entries, found 6")

    def test_no_test_over_a_plain_column(self):
        """A plain column holds a struct the checker does not read: is, in and set would never hold, or always."""
        rule = ["tables", "DT_Civs", "rules", 0]
        for test in ({"col": "EthicsPreset", "is": "x"}, {"col": "Color", "in": ["x"]}, {"col": "EthicsPreset", "set": False}, {"col": "Color", "set": True}):
            with self.subTest(test=test):
                self.schema_problem(self.put(rule, "when", [test]), "schema.DT_Civs", "cannot be asked of a column of kind plain")

    def test_an_end_mark_on_a_column_without_a_range(self):
        """ends_at works by itself, not through the max of the column: a numbers column without a range."""
        mark = self.put(["tables", "DT_Rules", "columns", "PressureBySourceTier"], "ends_at", 1)
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "PressureBySourceTier", [0, 0, 0.25, 0.5, 1.5]), mark),
                    "PressureBySourceTier", "the last entry has to be 1, found 1.5")
        self.assertEqual(self.after(self.set_cell("DT_Rules", "rules", "PressureBySourceTier", [0, 0, 0.25, 0.5, 1.0]), mark), [])

    def test_galaxy_rows(self):
        home = lambda column, value: self.set_cell("DT_Galaxies", "gal_milkyway", column, value)  # noqa: E731
        # A number of the sky carries its estimate; an empty count is let be, an empty distance is not.
        self.expect(self.after(home("DistanceText", "250\ub9cc \uad11\ub144")), "DT_Galaxies.gal_milkyway.DistanceText", "a number without \uc57d or \ucd94\uc815")
        self.expect(self.after(home("DistanceText", "\uc694\uc57d\ud558\uba74 250\ub9cc \uad11\ub144")), "DT_Galaxies.gal_milkyway.DistanceText", "a number without")
        self.expect(self.after(home("DistanceText", "\uc57d\ud55c \ube5b\uc758 250\ub9cc \uad11\ub144")), "DT_Galaxies.gal_milkyway.DistanceText", "a number without")
        self.assertEqual(self.after(home("DistanceText", "\uc57d250\ub9cc \uad11\ub144")), [])
        self.assertEqual(self.after(home("DistanceText", "\uad6d\ubd80\uc740\ud558\uad70 \uc548")), [])
        self.expect(self.after(home("FeatureText", "\uc9c0\ub984 10\ub9cc \uad11\ub144")), "DT_Galaxies.gal_milkyway.FeatureText", "a number without")
        self.assertEqual(self.after(home("FeatureText", "\ub9c9\ub300\uac00 \uc788\ub2e4")), [])
        self.assertEqual(self.after(home("FeatureText", "\uc9c0\ub984 10\ub9cc \uad11\ub144(\ucd94\uc815)")), [])
        # The hedge is a word of its own: no syllable joined before it.
        self.expect(self.after(home("DistanceText", "\uc694\uc57d 250\ub9cc \uad11\ub144")), "DT_Galaxies.gal_milkyway.DistanceText", "a number without")
        # Figures without digits, and digits of another script.
        self.expect(self.after(home("StarCountText", "\ubcc4 \uc218\uc5b5 ~ \uc218\uc2ed\uc5b5 \uac1c")), "DT_Galaxies.gal_milkyway.StarCountText", "a number without")
        self.assertEqual(self.after(home("StarCountText", "\ubcc4 \uc57d \uc218\uc5b5 ~ \uc218\uc2ed\uc5b5 \uac1c")), [])
        self.expect(self.after(home("FeatureText", "\uc9c0\ub984 \uc2ed\ub9cc \uad11\ub144")), "DT_Galaxies.gal_milkyway.FeatureText", "a number without")
        self.expect(self.after(home("DistanceText", "\uc9c0\ub984 \uff11\uff10 \uad11\ub144")), "DT_Galaxies.gal_milkyway.DistanceText", "a number without")
        self.expect(self.after(home("DistanceText", "\uc9c0\ub984 \u5341\u842c \uad11\ub144")), "DT_Galaxies.gal_milkyway.DistanceText", "a number without")
        # A longer hedge may take an ending; a figure that is debated says so.
        self.assertEqual(self.after(home("StarCountText", "\ubcc4 1\uc870 \uac1c(\ucd94\uc815\uce58)")), [])
        self.assertEqual(self.after(home("StarCountText", "\ucd94\uc815\uc5d0 \ub530\ub974\uba74 \ubcc4 1\uc870 \uac1c")), [])
        for column in ("StarCountText", "DistanceText", "FeatureText"):
            with self.subTest(column=column):
                self.assertEqual(self.after(home(column, "\uc8fc\uc694 \ub098\uc120\ud314 2~4\uac1c(\ub17c\uc7c1 \uc911)")), [])
        # A catalogue designation is a name, not a figure.
        self.assertEqual(self.after(home("FeatureText", "M32\uc640 NGC 205\ub97c \uac70\ub290\ub9b0\ub2e4")), [])
        # A word of five capitals is no designation: its number stays a figure.
        self.expect(self.after(home("FeatureText", "GALEX 3\uac1c\uc758 \ub760")), "DT_Galaxies.gal_milkyway.FeatureText", "a number without")
        # Each clause carries its own hedge; a thousands comma does not end a clause.
        self.expect(self.after(home("FeatureText", "\uc9c0\ub984 \uc57d 10\ub9cc \uad11\ub144, \ubcc4 1,000\uc5b5 \uac1c")), "DT_Galaxies.gal_milkyway.FeatureText", "a number without", ": \ubcc4 1,000\uc5b5 \uac1c")
        self.assertEqual(self.after(home("FeatureText", "\ubcc4 \uc57d 1,000\uc5b5 \uac1c, \uc9c0\ub984 \uc57d 10\ub9cc \uad11\ub144")), [])
        for mark in (" \u2014 ", " - ", "; "):
            with self.subTest(mark=mark):
                self.expect(self.after(home("FeatureText", "\ubcc4 1,000\uc5b5 \uac1c" + mark + "\uc9c0\ub984 \uc57d 10\ub9cc \uad11\ub144")),
                            "DT_Galaxies.gal_milkyway.FeatureText", "a number without", "\ubcc4 1,000\uc5b5 \uac1c")
        # Over a list of texts too, entry by entry.
        hedged_pool = self.put(["tables", "DT_Galaxies", "columns", "PeopleStems"], "hedged", ["\uc57d"])
        self.expect(self.after(home("PeopleStems", ["\ubcc4\uc774 3\uac1c"]), hedged_pool), "DT_Galaxies.gal_milkyway.PeopleStems[0]", "a number without \uc57d")
        self.assertEqual(self.after(home("DistanceText", "\uc57d 250\ub9cc \uad11\ub144")), [])
        self.assertEqual(self.after(home("StarCountText", "")), [])
        self.expect(self.after(home("StarCountText", "\ubcc4 1\uc870 \uac1c")), "DT_Galaxies.gal_milkyway.StarCountText", "a number without \uc57d or \ucd94\uc815")
        self.expect(self.after(home("DistanceText", "")), "DT_Galaxies.gal_milkyway.DistanceText", "a text is required")
        # The name pools: no digits, no word of the verdict list, no name twice.
        stars = self.row(self.tables, "DT_Galaxies", "gal_milkyway")["StarNameBank"]
        self.expect(self.after(home("StarNameBank", stars[:-1] + ["\uc601\ud63c\uc758 \ubcc4"])), "StarNameBank[%d]" % (len(stars) - 1), "the word \uc601\ud63c is not used here")
        for column in ("CatalogPrefixes", "CatalogStems", "PeopleStems"):
            with self.subTest(column=column):
                pool = self.row(self.tables, "DT_Galaxies", "gal_milkyway")[column][:-1] + ["\uc601\ud63c\ub9ac"]
                self.expect(self.after(home(column, pool)), "%s[%d]" % (column, len(pool) - 1), "the word \uc601\ud63c is not used here")
        self.expect(self.after(home("CatalogStems", ["\ub9ac\ub77c\uc5d0", "\uae00\ub9ac\uc81c 581"])), "CatalogStems[1]", "a digit")
        self.expect(self.after(home("CatalogPrefixes", ["\uc54c\ud30c", "\uc54c\ud30c"])), "CatalogPrefixes", "appears twice")
        # The home row: enough names for its peoples, enough pairs for its systems, no bridge.
        self.expect(self.after(home("Peoples", len(stars) + 1)), "row gal_milkyway of DT_Galaxies does not hold", '"len_ge_col": "Peoples"')
        self.assertEqual(self.after(home("Peoples", len(stars))), [])
        row = self.row(self.tables, "DT_Galaxies", "gal_milkyway")
        pairs = len(row["CatalogPrefixes"]) * len(row["CatalogStems"])
        self.expect(self.after(home("LivingWorlds", pairs + 1)), "row gal_milkyway of DT_Galaxies does not hold", '"pairs_ge_col"')
        self.assertEqual(self.after(home("LivingWorlds", pairs)), [])
        self.expect(self.after(home("BridgeTicks", 400)), "row gal_milkyway of DT_Galaxies does not hold", '"col": "BridgeTicks"')
        # Another row of the table is held to none of that: only the row the knob names is the home.
        self.assertEqual(self.after(self.set_cell("DT_Galaxies", "gal_andromeda", "Peoples", 0)), [])
        self.expect(self.after(self.set_cell("DT_Galaxies", "gal_andromeda", "BridgeTicks", -1)), "gal_andromeda.BridgeTicks", "is below 0")
        self.expect(self.after(self.set_cell("DT_Galaxies", "gal_andromeda", "Kind", "Star.Class.G")), "gal_andromeda.Kind", "not under Galaxy.Kind.")
        # The universe level (Phase 3 item 9): one name and one place per galaxy, and a galaxy other than the home holds
        # nothing the simulation reads of the home row.
        self.expect(self.after(self.set_cell("DT_Galaxies", "gal_m87", "DisplayName", "안드로메다은하")), "DT_Galaxies.gal_m87", "the universe level labels every galaxy", "the same DisplayName as row gal_andromeda")
        self.expect(self.after(self.set_cell("DT_Galaxies", "gal_m87", "MapPosition", {"X": 0.0, "Y": 0.0})), "DT_Galaxies.gal_m87", "two galaxies never stand on one place", "the same MapPosition as row gal_milkyway")
        self.assertEqual(self.after(self.set_cell("DT_Galaxies", "gal_andromeda", "MapPosition", {"X": 2.5, "Y": 4.0})), [])
        for column, value in (("Peoples", 3), ("AIFourthRungCap", 1), ("HomeRadiusShare", 0.5), ("StarNameBank", ["X"]), ("CatalogPrefixes", ["X"]), ("CatalogStems", ["X"])):
            self.expect(self.after(self.set_cell("DT_Galaxies", "gal_m87", column, value)), "DT_Galaxies.gal_m87", "a galaxy other than the home holds no peoples", '"col": "%s"' % column)
        self.assertEqual(self.after(self.set_cell("DT_Galaxies", "gal_m87", "PeopleStems", ["X"])), [])
        for column in ("DisplayName", "StarCountText", "DistanceText", "FeatureText"):
            self.expect(self.after(self.set_cell("DT_Galaxies", "gal_m87", column, "약 영혼")), "DT_Galaxies.gal_m87.%s" % column, "the word 영혼 is not used here")

    def test_a_pool_test_names_columns_of_its_table(self):
        target = ["tables", "DT_Rules", "columns", "HomeGalaxyRow"]
        for tests, words in (([{"col": "StarNameBank", "len_ge_col": 5}], "the test 'len_ge_col' needs a string"),
                             ([{"col": "StarNameBank", "len_ge_col": "NoSuchColumn"}], "names the column 'NoSuchColumn', which DT_Galaxies does not declare"),
                             ([{"col": "StarNameBank", "len_ge_col": "DisplayName"}], "the test 'len_ge_col' needs a column of kind number, and DisplayName is of kind text"),
                             ([{"col": "Peoples", "len_ge_col": "Peoples"}], "the test 'len_ge_col' cannot be asked of a column of kind number"),
                             ([{"col": "CatalogPrefixes", "pairs_ge_col": ["CatalogStems"]}], "the test 'pairs_ge_col' needs a list of two column names"),
                             ([{"col": "CatalogPrefixes", "pairs_ge_col": "CatalogStems"}], "the test 'pairs_ge_col' needs a list of two column names"),
                             ([{"col": "CatalogPrefixes", "pairs_ge_col": ["DisplayName", "LivingWorlds"]}], "needs a column of kind numbers or texts"),
                             ([{"col": "CatalogPrefixes", "pairs_ge_col": ["CatalogStems", "DisplayName"]}], "needs a column of kind number, and DisplayName is of kind text"),
                             ([{"col": "CatalogPrefixes", "pairs_ge_col": ["CatalogStems", "NoSuchColumn"]}], "names the column 'NoSuchColumn'")):
            with self.subTest(tests=tests):
                self.schema_problem(self.put(target, "target", tests), "schema.DT_Rules.HomeGalaxyRow", words)
        self.schema_problem(self.put(target, "target", [{"col": "Peoples", "pairs_ge_col": ["CatalogStems", "LivingWorlds"]}]),
                            "schema.DT_Rules.HomeGalaxyRow", "the test 'pairs_ge_col' cannot be asked of a column of kind number")
        # A tag container is no pool: in a rule of its own table, as the pool and as the other pool.
        civ_rule = ["tables", "DT_Civs", "rules", 0]
        for test, words in (({"col": "Traits", "len_ge_col": "Weight"}, "the test 'len_ge_col' cannot be asked of a column of kind tags"),
                            ({"col": "Traits", "pairs_ge_col": ["NameBank", "Weight"]}, "the test 'pairs_ge_col' cannot be asked of a column of kind tags"),
                            ({"col": "NameBank", "pairs_ge_col": ["Traits", "Weight"]}, "needs a column of kind numbers or texts")):
            with self.subTest(test=test):
                self.schema_problem(self.put(civ_rule, "when", [test]), "schema.DT_Civs", words)
        # A pool test holds only over a list and a number.
        holds = checker.Checker(self.schema, self.tables, self.tags).holds
        self.assertTrue(holds({"col": "A", "len_ge_col": "B"}, {"A": ["x", "y"], "B": 2}, "r"))
        self.assertFalse(holds({"col": "A", "len_ge_col": "B"}, {"A": ["x", "y"], "B": 3}, "r"))
        self.assertFalse(holds({"col": "A", "len_ge_col": "B"}, {"A": ["x", "y"], "B": "1"}, "r"))
        self.assertFalse(holds({"col": "A", "len_ge_col": "B"}, {"A": "xy", "B": 1}, "r"))
        self.assertTrue(holds({"col": "A", "pairs_ge_col": ["C", "B"]}, {"A": ["x", "y"], "C": ["p", "q", "r"], "B": 6}, "r"))
        self.assertFalse(holds({"col": "A", "pairs_ge_col": ["C", "B"]}, {"A": ["x", "y"], "C": ["p", "q", "r"], "B": 7}, "r"))
        self.assertFalse(holds({"col": "A", "pairs_ge_col": ["C", "B"]}, {"A": ["x", "y"], "C": "pqr", "B": 1}, "r"))
        self.assertFalse(holds({"col": "A", "pairs_ge_col": ["C", "B"]}, {"A": ["x", "y"], "C": ["p"], "B": None}, "r"))

    def test_options_of_the_text_kinds_have_their_type(self):
        for column, key, value in (("DistanceText", "hedged", []), ("DistanceText", "hedged", "\uc57d"), ("DistanceText", "hedged", ["\uc57d", ""])):
            with self.subTest(column=column, key=key, value=value):
                self.schema_problem(self.put(["tables", "DT_Galaxies", "columns", column], key, value), "DT_Galaxies.%s" % column, key, "not empty")
        # The words a column refuses are word lists of the schema, named - one, or a list of names (B2): an empty list or a
        # list of other things is a problem of its type, and a name the schema does not hold (a word written in place of a
        # list's name) is a list that is not in the schema.
        for value in ([], [1], 3):
            with self.subTest(key="forbidden", value=value):
                self.schema_problem(self.put(["tables", "DT_Galaxies", "columns", "StarNameBank"], "forbidden", value), "DT_Galaxies.StarNameBank", "forbidden", "a string or a list of strings")
        for value in (["\uc601\ud63c"], [""], ["verdict", "verdicts"]):
            with self.subTest(key="forbidden", value=value):
                self.schema_problem(self.put(["tables", "DT_Galaxies", "columns", "StarNameBank"], "forbidden", value), "DT_Galaxies.StarNameBank", "is not in the schema")
        self.schema_problem(self.put(["tables", "DT_Rules", "columns", "LivingWorldStarShares"], "ends_at", "1"), "LivingWorldStarShares", "ends_at", "a number")
        self.schema_problem(self.put(["tables", "DT_Galaxies", "columns", "StarNameBank"], "hedged", [""]), "DT_Galaxies.StarNameBank", "hedged", "not empty")
        self.schema_problem(self.put(["tables", "DT_Rules", "columns", "SightIntervalTicks"], "ends_at", 1), "SightIntervalTicks", "ends_at")

    def test_rows_kind(self):
        """A list of row names: each a row of one of the named tables, a list and nothing else, at most once with unique."""
        column = ["tables", "DT_Hazards", "columns", "RulerResolutions"]
        rows = lambda value: self.set_cell("DT_Hazards", "crisis_test_storm", "RulerResolutions", value)  # noqa: E731
        self.assertEqual(self.after(rows([])), [])
        self.expect(self.after(rows(["enc_nobody"])), "RulerResolutions[0]", "none of DT_Megaprojects, DT_Encounters, DT_Edicts has a row enc_nobody")
        # A project is an answer.
        self.assertEqual(self.after(rows(["project_test_build"])), [])
        # A row of a table the column does not name is no answer.
        self.expect(self.after(rows(["event_book_found"])), "RulerResolutions[0]", "none of DT_Megaprojects, DT_Encounters, DT_Edicts has a row event_book_found")
        self.expect(self.after(rows("enc_nobody")), "RulerResolutions", "expected a list of row names")
        for wrong in ("", None, 5):
            with self.subTest(entry=wrong):
                self.expect(self.after(rows([wrong])), "RulerResolutions[0]", "expected a row name")
                # Reported once: an entry that is no name is never also a duplicate.
                said = [line for line in self.after(rows([wrong])) if "RulerResolutions" in line]
                self.assertEqual(len(said), 1, said)
        # Two entries that are no names: each reported, neither a duplicate of the other.
        for wrong in ([None, ""], ["None", "none"], [None, None], [5, 5]):
            with self.subTest(entries=wrong):
                said = [line for line in self.after(rows(wrong)) if "RulerResolutions" in line]
                self.assertEqual(len(said), 2, said)
                self.assertEqual([line for line in said if "appears twice" in line], [], said)
        # (DT_Megaprojects stays named: the extinction's rule asks rows_of it of this column - B6b [B-6b2b].)
        to_events = self.put(column, "tables", ["DT_Events", "DT_Megaprojects"])
        self.assertEqual(self.after(rows(["event_book_found"]), to_events), [])
        self.expect(self.after(rows(["event_book_found", "event_book_found"]), to_events), "RulerResolutions", "appears twice")
        without_unique = self.drop(column, "unique")
        self.assertEqual(self.after(rows(["event_book_found", "event_book_found"]), lambda schema: (to_events(schema), without_unique(schema))), [])
        self.schema_problem(self.put(column, "tables", []), "DT_Hazards.RulerResolutions", "tables", "not empty")
        self.schema_problem(self.put(column, "tables", "DT_Events"), "DT_Hazards.RulerResolutions", "tables", "not empty")
        self.schema_problem(self.put(column, "tables", ["DT_Nowhere"]), "DT_Hazards.RulerResolutions", "the table 'DT_Nowhere' is not in the schema")
        self.schema_problem(self.drop(column, "tables"), "DT_Hazards.RulerResolutions", "'tables' is missing")
        self.schema_problem(self.put(column, "unique", "yes"), "DT_Hazards.RulerResolutions", "unique")
        self.schema_problem(self.put(column, "table", "DT_Events"), "DT_Hazards.RulerResolutions", "'table' is not read here")
        self.schema_problem(self.put(column, "slots", ["Civ"]), "DT_Hazards.RulerResolutions", "'slots' is not read here")
        # A text list with unique: an entry that is no text is reported once, never also as a duplicate.
        bank = lambda value: self.set_cell("DT_Civs", "arch_default", "PersonNameBank", value)  # noqa: E731
        names = ["n%s" % letter for letter in "abcdefghij"]
        said = [line for line in self.after(bank(names + [None])) if "PersonNameBank" in line]
        self.assertEqual(len(said), 1, said)
        for wrong in ([None, None], [5, 5]):
            with self.subTest(entries=wrong):
                said = [line for line in self.after(bank(names + wrong)) if "PersonNameBank" in line]
                self.assertEqual(len(said), 2, said)
                self.assertEqual([line for line in said if "appears twice" in line], [], said)

    def test_hazard_row(self):
        cell = lambda column, value: self.set_cell("DT_Hazards", "crisis_test_storm", column, value)  # noqa: E731
        self.expect(self.after(cell("SpawnRule", "Sometimes")), "'Sometimes' is no enumerator of ECosmicHazardSpawn")
        self.expect(self.after(cell("LogCategory", "Log.Civ")), "crisis_test_storm.LogCategory", "not under Log.Crisis")
        self.expect(self.after(cell("CountdownTicks", 0)), "crisis_test_storm.CountdownTicks", "is below 1")
        self.expect(self.after(cell("SpawnChancePerTick", 1.5)), "crisis_test_storm.SpawnChancePerTick", "is above 1")
        self.expect(self.after(cell("SourceDoctrine", "Doctrine.Covenant")), "crisis_test_storm.SourceDoctrine", "not under")
        self.expect(self.after(cell("OutbreakRow", "event_nowhere")), "crisis_test_storm.OutbreakRow", "DT_Events has no row event_nowhere")
        self.assertEqual(self.after(cell("OutbreakRow", "event_test_rising")), [])
        # An outbreak row is about the place: its line has the hazard's system as its subject, as the hazard's own lines do.
        self.expect(self.after(cell("OutbreakRow", "event_book_found")), "crisis_test_storm.OutbreakRow", "row event_book_found of DT_Events does not hold",
                    '"Subject", "is": "SettledSystem"')
        self.expect(self.after(cell("ExpireTemplates", ["{Rebel} rose."])), "crisis_test_storm.ExpireTemplates", "slot {Rebel}")
        self.assertEqual(self.after(cell("OutbreakTemplates", [])), [])
        # The rules of the crisis handling review, each broken by hand with the enumerator it names.
        def both(*changes):
            return lambda tables: [change(tables) for change in changes]
        resolve_again = "a resolution or an expiry does not resolve again"
        hand_over = "only stage code behind the last-system gate takes a system"
        branch = cell("BranchChance", 0.5)
        for column in ("OnResolveEffects", "OnExpireEffects", "OnExpireBranchEffects"):
            with self.subTest(column=column, effect="ResolveHazard"):
                self.expect(self.after(both(branch, cell(column, [eff("ResolveHazard")]))), "crisis_test_storm", resolve_again, column)
        for column in ("OnExpireEffects", "OnExpireBranchEffects", "OnExpireNeighbourEffects", "OnResolveEffects"):
            with self.subTest(column=column, effect="TransferSystem"):
                self.expect(self.after(both(branch, cell(column, [eff("TransferSystem")]))), "crisis_test_storm", hand_over, column)
        # A neighbour's hazard may be resolved by an expiry (the system's own is not); its resolution hands nothing over.
        self.assertEqual(self.after(cell("OnExpireNeighbourEffects", [eff("ResolveHazard")])), [])
        self.expect(self.after(cell("SpawnConditions", [cond("CivFallenSphereTicksReached", key="300")])), "crisis_test_storm", "the stay condition holds on every tick")
        # The outbreak row: a cult has none, and no outbreak row resolves what just broke out.
        self.expect(self.after(both(cell("HazardTag", "Hazard.Cult"), cell("OutbreakRow", "event_book_found"))), "crisis_test_storm", "the leader of a cult has no name", "OutbreakRow")

        def resolving_outbreak_row(tables):
            row = copy.deepcopy(self.row(tables, "DT_Events", "event_book_found"))
            row.update(Name="event_test_outbreak", Effects=[eff("ResolveHazard")])
            tables["DT_Events"].append(row)
            self.row(tables, "DT_Hazards", "crisis_test_storm")["OutbreakRow"] = "event_test_outbreak"
        self.expect(self.after(resolving_outbreak_row), "crisis_test_storm.OutbreakRow", "does not hold", "ResolveHazard")

    def test_the_lethal_expiry(self):
        """B5 [B-5d2] (header 3.2, 3.5, decision 6-ga): a KillPlayer stands only in a hazard row's own expiry effects - never its
        branch, its neighbours' damage or its resolution; never on a claim, a visitation or a cult, nor on the supernova
        precursor until that hazard has a death row of its own, nor on a hazard a doctrine brings whatever its tag; never in a
        feat's price on any path; its cause is Death.Hazard, so Death.Transcendent (and every other cause) stands nowhere in
        the data. The project's data holds exactly the one death decision 6-ga opens."""
        cell = lambda column, value: self.set_cell("DT_Hazards", "crisis_test_storm", column, value)  # noqa: E731

        def both(*changes):
            return lambda tables: [change(tables) for change in changes]
        kill = eff("KillPlayer", tag="Death.Hazard")
        stays = eff("ChangeStability", value=-0.1)
        # Allowed: the own side of an expiry, a row with a branch included (the AI awakening's shape - its branch spares).
        self.assertEqual(self.after(cell("OnExpireEffects", [stays, kill])), [])
        awakening = both(cell("HazardTag", "Hazard.AIAwakening"), cell("BranchChance", 0.5), cell("OnExpireBranchEffects", [stays]),
                         cell("OnExpireEffects", [stays, kill]))
        self.assertEqual(self.after(awakening), [])
        # Never the branch, the neighbours or the resolution.
        own_side = "a death comes only from a hazard's own expiry, on its own side"
        for column in ("OnExpireBranchEffects", "OnExpireNeighbourEffects", "OnResolveEffects"):
            with self.subTest(column=column):
                self.expect(self.after(both(cell("BranchChance", 0.5), cell(column, [stays, kill]))), "crisis_test_storm", own_side, column)
        # Never a claim, a visitation or a cult.
        spiritual = "the fallen cannot touch a life"
        for tag, extra in (("Hazard.Claim", both(cell("SpawnRule", "ClaimRule"), cell("SpawnChancePerTick", 0.0))), ("Hazard.Visitation", NOTHING),
                           ("Hazard.Cult", NOTHING)):
            with self.subTest(tag=tag):
                self.expect(self.after(both(cell("HazardTag", tag), extra, cell("OnExpireEffects", [kill]))), "crisis_test_storm", spiritual, "OnExpireEffects")
                self.expect(self.after(both(cell("HazardTag", tag), extra)), spiritual, wanted=False)
        # Not the supernova precursor yet (critic-2's guard).
        precursor = "the supernova precursor's expiry destroys the system"
        self.expect(self.after(both(cell("HazardTag", "Hazard.PreSupernova"), cell("OnExpireEffects", [kill]))), "crisis_test_storm", precursor, "OnExpireEffects")
        self.expect(self.after(both(cell("HazardTag", "Hazard.AIAwakening"), cell("OnExpireEffects", [kill]))), precursor, wanted=False)
        # Never a hazard a doctrine brings, whatever its tag: the source, not a list of tags ([B-5e] - the [B-5d2] review's risk-6).
        doctrine = "a hazard a fallen one's doctrine brings never kills at its expiry"
        self.expect(self.after(both(cell("HazardTag", "Hazard.AIAwakening"), cell("SourceDoctrine", "Doctrine.Zeal"), cell("OnExpireEffects", [stays, kill]))),
                    "crisis_test_storm", doctrine, "OnExpireEffects")
        self.assertEqual(self.after(both(cell("HazardTag", "Hazard.AIAwakening"), cell("SourceDoctrine", "Doctrine.Zeal"), cell("OnExpireEffects", [stays]))), [])
        # Never a feat's price, on any path: a wanderer's row too, which the ruler rule does not read ([B-5e] - risk-6).
        price = "a feat's price records no death on any path"
        feat = lambda **changes: (lambda tables: self.row(tables, "DT_Feats", "feat_test_climb").update(**changes))  # noqa: E731
        self.expect(self.after(feat(Path="Wanderer", AscensionCost=[kill])), "feat_test_climb", price, "AscensionCost")
        self.expect(self.after(feat(Path="Wanderer", AscensionCost=[kill])), "a ruler feat row judges every people", wanted=False)
        self.expect(self.after(feat(Path="Wanderer")), price, wanted=False)
        # The cause on a hazard row is Death.Hazard: every other cause is refused, Death.Transcendent first of all.
        for cause in ("Death.Transcendent", "Death.Encounter", "Death.Age"):
            with self.subTest(cause=cause):
                self.expect(self.after(cell("OnExpireEffects", [eff("KillPlayer", tag=cause)])), "crisis_test_storm.OnExpireEffects[0](KillPlayer).Tag",
                            "tag %s is not under Death.Hazard" % cause)
        # The project's data: one death, the AI awakening's own expiry, by Death.Hazard (decision 6-ga, 2026-10-30). One of two
        # pins with Data.Integrity's (KillPlayersOf): the rules above refuse, and the pins hold the one row allowed.
        _, project_tables, _ = checker.load_project()
        found = [(row["Name"], column, entry.get("Tag")) for table, rows in sorted(project_tables.items()) if isinstance(rows, list)
                 for row in rows if isinstance(row, dict) for column, value in sorted(row.items()) if isinstance(value, list)
                 for entry in value if isinstance(entry, dict) and entry.get("Type") == "KillPlayer"]
        found += [(row["Name"], "Choices.%s" % side, entry.get("Tag")) for row in project_tables.get("DT_Encounters", [])
                  for choice in row.get("Choices", []) for side in ("OnSuccess", "OnFailure") for entry in choice.get(side, [])
                  if isinstance(entry, dict) and entry.get("Type") == "KillPlayer"]
        self.assertEqual(found, [("crisis_ai_awakening", "OnExpireEffects", "Death.Hazard")],
                         "decision 6-ga (2026-10-30) opened one death, the AI awakening's own expiry: another source of death is a new "
                         "decision (DECISIONS_LOG), and this pin and Data.Integrity's change with it")

    def test_megaprojects_and_feats(self):
        """The lines of a completion and of an ascension come in three variants or more; a feat walks the ruler's path."""
        self.expect(self.after(self.set_cell("DT_Megaprojects", "project_test_build", "CompleteTemplates", ["{Civ} a.", "{System} b."])),
                    "DT_Megaprojects.project_test_build.CompleteTemplates", "3 entries or more")
        self.expect(self.after(self.set_cell("DT_Feats", "feat_test_climb", "LogTemplates", ["{Civ} a.", "b."])), "DT_Feats.feat_test_climb.LogTemplates", "3 entries or more")
        self.expect(self.after(self.set_cell("DT_Megaprojects", "project_test_build", "RequiredProgress", 0.5)), "DT_Megaprojects.project_test_build.RequiredProgress", "is below 1")
        self.expect(self.after(self.set_cell("DT_Feats", "feat_test_climb", "Path", "Wanderer")), "feat_test_climb", "the wanderer's feats are judged from Phase 4")
        self.expect(self.after(self.set_cell("DT_Feats", "feat_test_climb", "GrantedPowers", tags_text("Trait.Arch.Seeker"))), "feat_test_climb.GrantedPowers", "not under")
        # One rung: every other pair of rungs is refused by the rule of its starting rung, whatever the walker tries.
        rungs = ["Tier1_Planetary", "Tier2_Stellar", "Tier3_Causal", "Tier4_Transcendent", "Tier5_EternalOrder"]
        ordinal = ["first", "second", "third", "fourth"]
        tiers = lambda required, target: (lambda tables: self.row(tables, "DT_Feats", "feat_test_climb").update(RequiredTier=required, TargetTier=target))  # noqa: E731
        for start in range(4):
            for end in range(5):
                with self.subTest(required=rungs[start], target=rungs[end]):
                    why = "a feat lifts its people one rung: from the %s to the" % ordinal[start]
                    self.expect(self.after(tiers(rungs[start], rungs[end])), "feat_test_climb", why, wanted=end != start + 1)
        self.expect(self.after(tiers("Tier5_EternalOrder", "Tier5_EternalOrder")), "feat_test_climb", "the fifth rung is the last")
        # The rate of the AI's projects: its boundary is 0 itself.
        rate = lambda value: self.set_cell("DT_Rules", "rules", "AIAscensionSpeedMultiplier", value)  # noqa: E731
        self.expect(self.after(rate(0)), "DT_Rules.rules", "the rate of the AI's projects multiplies their progress")
        self.expect(self.after(rate(-1)), "DT_Rules.rules", "the rate of the AI's projects multiplies their progress")
        self.assertEqual(self.after(rate(0.5)), [])
        # The vessel rule is about ruler rows: a wanderer row is refused by the wanderer rule alone.
        wanderer = lambda tables: self.row(tables, "DT_Feats", "feat_test_climb").update(Path="Wanderer", CompletionConditions=[cond("PlayerStatAtLeast", key="Body", value=3)])  # noqa: E731
        self.expect(self.after(wanderer), "feat_test_climb", "a ruler feat row judges every people", wanted=False)
        self.expect(self.after(wanderer), "feat_test_climb", "the wanderer's feats are judged from Phase 4")

    # ---- texts ----

    def test_texts(self):
        def templates(count):
            return self.set_cell("DT_LogTemplates", "log_breakthrough_physics", "Templates", ["{Civ} line %d." % index for index in range(count)])
        self.expect(self.after(templates(2)), "3 entries or more, found 2")
        self.assertEqual(self.after(templates(3)), [])
        self.expect(self.after(self.set_cell("DT_LogTemplates", "log_first_life", "Templates", [])), "1 entries or more")
        self.assertEqual(self.after(self.set_cell("DT_LogTemplates", "log_first_life", "Templates", ["{HomeSystem}."])), [])
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "AdjectiveBank", ["a", "b"])), "0 or 3 entries, found 2")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "AdjectiveBank", ["a", "", "c"])), "AdjectiveBank[1]", "a text is required")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "AdjectiveBank", ["a", 2, "c"])), "AdjectiveBank[1]", "expected a text")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "DisplayName", 5)), "DisplayName", "expected a text")
        names = ["name %s" % letter for letter in "abcdefghij"]
        self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_seeker", "PersonNameBank", names)), [])
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "PersonNameBank", names[:5])), "none or 10 entries and more, found 5")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "PersonNameBank", names[:9] + names[:1])), "appears twice")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "NameBank", "one text")), "list of texts")

    def test_slots(self):
        log = lambda text: self.after(self.set_cell("DT_LogTemplates", "log_first_life", "Templates", [text]))  # noqa: E731
        self.expect(log("{Civv} arrives."), "slot {Civv}")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "NameBank", ["{Hex} union"])), "slot {Hex}")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "AdjectiveBank", ["{Civ}", "b", "c"])), "slot {Civ}", "none")
        self.expect(self.after(self.set_cell("DT_Events", "event_test_harvest", "LogTemplates", ["{OtherCiv} a.", "b.", "c."])), "slot {OtherCiv}")
        self.assertEqual(log("{CivAdj} {Civ}."), [])
        # Read the way the engine reads a format text.
        self.assertEqual(log("{Ruler}|hpp(\uc740,\ub294) of {Civ}."), [])
        # The modifier after a slot: hpp alone, with a particle pair, the form after a final consonant first.
        self.assertEqual(log("{Civ}|hpp(\uc73c\ub85c, \ub85c) {HomeSystem}|hpp(\uc774,\uac00)."), [])
        self.expect(log("{Civ}|hpp(\ub294,\uc740) a."), "|hpp(\ub294,\uc740)", "the wrong way round")
        self.expect(log("{Civ}|hpp(\uac00,\uc774) a."), "the wrong way round")
        for arguments in ("\uc740", "\uc740.\ub294", "a,b", "\uc740,\ub294,\uc774", "\uc740,", ",\ub294", "\uc740,\ub97c", ""):
            with self.subTest(arguments=arguments):
                self.expect(log("{Civ}|hpp(%s) a." % arguments), "|hpp(%s) is not a particle pair" % arguments)
        self.expect(log("{Civ}|hp(\uc740,\ub294) a."), "the modifier |hp is not one the texts use")
        self.expect(log("{Civ}|plural(one=a,other=b) a."), "the modifier |plural is not one the texts use")
        for broken in ("{Civ}|hpp(\uc740,\ub294 a.", "{Civ}| a.", "{Civ}|(\uc740,\ub294) a.", "{Civ}|hpp a."):
            with self.subTest(text=broken):
                self.expect(log(broken), "starts no modifier")
        self.assertEqual(log("{Civ}`| a."), [])
        self.assertEqual(log("{Civ} |hpp a."), [])
        self.assertEqual(log("`{not a slot`} and {Civ}."), [])
        self.expect(log("{HomeSystm. broken {Civ"), "never closed")
        self.expect(log("HomeSystem} arrives."), "closes nothing")
        self.expect(log("{Civv x {Civ} arrives."), "slot {Civv x {Civ}")
        self.expect(log("{{Civ}} arrives."), "slot {{Civ}")
        self.assertEqual(checker.slots_of("a `{b`} {C} ``{D}"), (["C", "D"], []))

    def test_no_digits_in_a_name(self):
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "NameBank", ["Union 7"])), "NameBank[0]", "a digit")
        self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_seeker", "NameBank", ["Union Seven"])), [])
        self.expect(self.after(self.set_cell("DT_Civs", "arch_seeker", "HomeSystemNames", ["HD-581"])), "HomeSystemNames[0]", "a digit")

    # ---- row rules, unique, pairs ----

    def test_row_rule(self):
        self.expect(self.after(self.set_cell("DT_Civs", "arch_default", "NameBank", ["union"])), "arch_default", "home star", "{Star}")
        self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_default", "NameBank", ["{Star} union"])), [])
        self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_seeker", "NameBank", ["union"])), [])

    def test_tests_of_a_rule(self):
        """Each test, both ways, through a rule built here (the rules of the project use only some of them)."""
        def rule(when, then):
            return self.put(["tables", "DT_Civs", "rules"], 0, {"why": "a rule of the test", "when": when, "then": then})

        def add_row(tables):
            tables["DT_Civs"].append(civ_row(tables["DT_Civs"][0], "arch_test_row", RevelationMode="Revelation.Book", Traits=tags_text(
                "Trait.Arch.Seeker", "Trait.Society.Divided", "Trait.Faith.Book", "Trait.Biology.Shortlived"),
                DedicationBanks=["Faith.Bank.Hebrew", "Faith.Bank.Civic"]))
        broken = lambda when, then: self.expect(self.after(add_row, rule(when, then)), "a rule of the test")  # noqa: E731
        holds = lambda when, then: self.assertEqual(self.after(add_row, rule(when, then)), [])  # noqa: E731
        # The row the cases read is built here: Weight 0, no AI profile, four traits, empty name banks, two dedication banks,
        # one home system name.
        terran = [{"row": "arch_test_row"}]
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
        # contains_all over a tag container and over a list of tags: every named tag, in any order.
        holds(terran, [{"col": "Traits", "contains_all": ["Trait.Faith.Book", "Trait.Society.Divided"]}])
        broken(terran, [{"col": "Traits", "contains_all": ["Trait.Society.Divided", "Trait.Arch.Default"]}])
        holds(terran, [{"col": "DedicationBanks", "contains_all": ["Faith.Bank.Civic"]}])
        broken(terran, [{"col": "DedicationBanks", "contains_all": ["Faith.Bank.Civic", "Faith.Bank.Nations"]}])
        holds(terran, [{"col": "PersonNameBank", "set": False}, {"col": "InitialRelations", "set": False}, {"col": "HomeSystemNames", "min_len": 1}])
        broken(terran, [{"col": "PersonNameBank", "set": True}])
        broken(terran, [{"col": "NameBank", "each_contains": "{Star}"}])
        holds(terran, [{"col": "NameBank", "slots_in": ["Star"]}, {"col": "DisplayName", "slots_in": []}])
        # The 'row' test picks one row: what the other rows hold is none of the rule's business.
        holds(terran, [{"col": "DisplayName", "is": "plain"}])
        broken([], [{"col": "DisplayName", "is": "plain"}])
        # A rule without 'when' holds for every row.
        self.expect(self.after(add_row, self.put(["tables", "DT_Civs", "rules"], 0, {"why": "a rule for all", "then": [{"col": "Weight", "gt": 0}]})),
                    "DT_Civs.arch_test_row", "a rule for all")

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
        # Two rows of the test, picked out by a display name no row of the project has.
        one_class = self.put(["tables", "DT_Civs"], "unique", [{"cols": ["HomeStarClass"], "when": [{"col": "DisplayName", "is": "unique"}],
                                                                "why": "one people per star class"}])

        def two_rows(first, second):
            def change(tables):
                base = tables["DT_Civs"][0]
                tables["DT_Civs"].append(civ_row(base, "arch_test_a", DisplayName="unique", HomeStarClass=first))
                tables["DT_Civs"].append(civ_row(base, "arch_test_b", DisplayName="unique", HomeStarClass=second))
            return change
        self.assertEqual(self.after(two_rows("Star.Class.G", ""), one_class), [])
        self.expect(self.after(two_rows("", "None"), one_class), "arch_test_b", "one people per star class", "the same HomeStarClass")

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
        # A row fired by name may speak of the vessel; a slot row may not, until the selector words reach the code (Phase 4 petitions).
        vessel = dict(Conditions=[cond("PlayerRoleIs", key="Ruler")], Effects=[eff("DeposeRuler", tag="Reign.Deposed"), eff("GrantPower", tag="Way.Received")])
        self.assertEqual(self.after(named(**vessel)), [])
        self.expect(self.after(cell("Conditions", vessel["Conditions"])), "a slot runs for every people", "PlayerRoleIs")
        self.expect(self.after(cell("Effects", vessel["Effects"])), "a slot runs for every people", "GrantPower")
        # The same for the other kind of slot row.
        standing = lambda **changes: (lambda tables: self.row(tables, "DT_Events", "event_test_harvest").update(dict(dict(Trigger="Standing", BaseWeight=0), **changes)))  # noqa: E731
        self.assertEqual(self.after(standing()), [])
        self.expect(self.after(standing(LogTemplates=["a.", "b."])), "three variants or more")
        self.expect(self.after(standing(Conditions=vessel["Conditions"])), "a slot runs for every people", "PlayerRoleIs")
        self.expect(self.after(standing(Effects=vessel["Effects"])), "a slot runs for every people", "DeposeRuler")

    def test_a_line_names_its_subject_system(self):
        """The scope of telling judges a line by the system it names (guideline 8.1): a row about a people names none, the
        lines of an event row and of a hazard name the subject system and never the capital beside it, the far lines name
        nothing and wait for no tier, and the calling's own lines wait for no tier."""
        harvest = lambda **changes: (lambda tables: self.row(tables, "DT_Events", "event_test_harvest").update(changes))  # noqa: E731
        names_none = "a Civilization row is about the people alone and names no system"
        self.expect(self.after(harvest(LogTemplates=["{Civ} a.", "{System} b.", "c."])), "event_test_harvest", names_none)
        self.assertEqual(self.after(harvest(Subject="SettledSystem", LogTemplates=["{Civ} a.", "{System} b.", "c."])), [])
        self.assertEqual(self.after(harvest(LogTemplates=["{Civ} a.", "{Hazard} b.", "c."])), [])
        self.expect(self.after(harvest(Subject="SettledSystem", LogTemplates=["{Civ} a.", "{HomeSystem} b.", "c."])), "event_test_harvest.LogTemplates", "slot {HomeSystem}")
        for column in ("OutbreakTemplates", "ExpireTemplates", "ResolveTemplates"):
            with self.subTest(column=column):
                self.expect(self.after(self.set_cell("DT_Hazards", "crisis_test_storm", column, ["{HomeSystem} d."])), "crisis_test_storm.%s" % column, "slot {HomeSystem}")
        # The row that made the rule is Named, and the system it named came after the people, with or without a particle.
        self.expect(self.after(harvest(Trigger="Named", BaseWeight=0, CooldownTicks=0, LogTemplates=["{Civ}\uc758 \uc218\ub3c4 {System}\uc5d0\uc11c a."])),
                    "event_test_harvest", names_none)
        self.expect(self.after(harvest(LogTemplates=["{Civ}|hpp(\uc740,\ub294) {System} a.", "b.", "c."])), "event_test_harvest", names_none)
        self.expect(self.after(harvest(LogTemplates=["a.", "b.", "{Hazard} {System} c."])), "event_test_harvest", names_none)
        far = "a far line is the same sentence for everyone"
        for name in ("log_far_event", "log_far_ascent"):
            with self.subTest(row=name):
                self.expect(self.after(self.set_cell("DT_LogTemplates", name, "MinTierToReveal", 2)), "DT_LogTemplates.%s" % name, far, "MinTierToReveal")
                self.expect(self.after(self.set_cell("DT_LogTemplates", name, "Category", "Log.Civ")), "DT_LogTemplates.%s" % name, far, "Category")
                self.expect(self.after(self.set_cell("DT_LogTemplates", name, "Category", "Log.Ascension")), "DT_LogTemplates.%s" % name, far, "Category")
                for slot in ("Civ", "System", "HomeSystem"):
                    self.expect(self.after(self.set_cell("DT_LogTemplates", name, "Templates", ["a.", "{%s} b." % slot, "c."])), "DT_LogTemplates.%s" % name, far, "slots_in")
        # Another row may wait for a tier and call a people; the calling's own rows may not wait.
        self.assertEqual(self.after(self.set_cell("DT_LogTemplates", "log_contact", "MinTierToReveal", 2)), [])
        calling = "the calling's own line is told whatever its subject"
        self.expect(self.after(self.set_cell("DT_LogTemplates", "log_fallen_monarch", "MinTierToReveal", 2)), "log_fallen_monarch", calling)
        self.expect(self.after(self.set_cell("DT_LogTemplates", "log_first_life", "MinTierToReveal", 1)), "log_first_life", calling)
        # A selector row of the calling waits for no tier either.
        selector = lambda tables: self.row(tables, "DT_LogTemplates", "log_fallen_monarch").update(Context="Revelation.Book", MinTierToReveal=2)  # noqa: E731
        self.expect(self.after(selector), "log_fallen_monarch", calling)

    def test_every_forbidden_column_refuses_every_verdict_word(self):
        """The narrator's lines, the names that fill their slots and the names of persons carry none of the verdict words
        (CLAUDE.md, guideline 8.3): the schema guards them until the test Data.NoVerdictWords of Phase 5. The words are one
        list of the schema (words.verdict), the list of the invariant in CLAUDE.md; every column that names it refuses every
        one of them. A first-person row (Log.Transcendent, Log.Claim) does not exist yet: with the first one the rule of its
        column becomes one by category, which needs a row test the schema does not have (guideline 12.3)."""
        verdict = self.schema["words"]["verdict"]
        # The list is the invariant's: the seven words of the line in CLAUDE.md, in its order.
        with io.open(os.path.join(checker.project_root(), "CLAUDE.md"), encoding="utf-8") as claude:
            line = next(each for each in claude if "\ud310\uc815 \uc5b4\ud718(" in each)
        listed = line.split("\ud310\uc815 \uc5b4\ud718(", 1)[1].split(")", 1)[0].split("\u00b7")
        self.assertEqual(verdict, listed)
        columns = self.columns_of(lambda spec: "verdict" in checker.forbidden_lists(spec))
        # Every column that refuses the words: the narrator's lines, the names that fill their slots, the name pools, the
        # title of a transcendent (the spiritual sight's line and tooltip, the log's {Transcendent} slot), the texts of a
        # card (its title, its bodies, and the label and the outcome lines of each choice) and an edict's name and line (B2).
        self.assertEqual(len(columns), 32, [table + "." + column for table, column, spec in columns])
        for table, column, spec in columns:
            row_name = self.tables[table][0]["Name"]
            for word in verdict:
                with self.subTest(table=table, column=column, word=word):
                    value = "%s a." % word if spec["kind"] == "text" else ["%s a." % word]
                    change, where = self.set_path(table, row_name, column, value)
                    self.expect(self.after(change), where, "the word %s is not used here" % word)
        # The list is named, not copied: a column that names a list the schema does not hold is a problem of the schema.
        self.expect(self.after(NOTHING, self.put(["tables", "DT_Events", "columns", "LogTemplates"], "forbidden", "verdicts")),
                    "the word list 'verdicts' is not in the schema")

    def test_edicts(self):
        """The edicts (B2): a name and a line refuse the verdict words and the holy words (docs/70 - no number, resource,
        edict, relic or tier is named with them), and carry no digit; a countdown bonus is a whole number of ticks at or
        above 0, a spawn factor at or above 0; the costs at or above 0; the inherited edict is an edict row or none; the
        slot marks are ascending shares; the edict and resource keys of the String Table refuse the holy words too."""
        holy = self.schema["words"]["holy_words"]
        self.assertEqual(holy, ["\uc131\ub839", "\uc740\ud61c", "\uae30\ub984\ubd80\uc74c", "\uc131\uacb0"])
        self.assertEqual(self.after(), [])
        for column in ("DisplayName", "Summary"):
            for word in holy:
                with self.subTest(column=column, word=word):
                    self.expect(self.after(self.set_cell("DT_Edicts", "edict_test_order", column, "%s a." % word)), "DT_Edicts.edict_test_order.%s" % column,
                                "the word %s is not used here" % word)
            with self.subTest(column=column, digit=True):
                self.expect(self.after(self.set_cell("DT_Edicts", "edict_test_order", column, "order 3")), "DT_Edicts.edict_test_order.%s" % column, "a digit")
        self.expect(self.after(self.set_cell("DT_Edicts", "edict_test_order", "HazardCountdownBonus", {"Hazard.Claim": 2.5})), "HazardCountdownBonus", "whole number")
        self.expect(self.after(self.set_cell("DT_Edicts", "edict_test_order", "HazardCountdownBonus", {"Hazard.Claim": -1})), "HazardCountdownBonus", "below")
        self.expect(self.after(self.set_cell("DT_Edicts", "edict_test_order", "HazardSpawnMultipliers", {"Hazard.Plague": -0.5})), "HazardSpawnMultipliers", "below")
        self.expect(self.after(self.set_cell("DT_Edicts", "edict_test_order", "HazardSpawnMultipliers", {"Law.Hedge": 1.0})), "HazardSpawnMultipliers", "Hazard.")
        upkeep = {"Energy": 0.0, "Matter": 0.0, "Insight": 0.0, "Influence": -0.1, "Causality": 0.0}
        self.expect(self.after(self.set_cell("DT_Edicts", "edict_test_order", "UpkeepPerTick", upkeep)), "UpkeepPerTick", "below")
        self.expect(self.after(self.set_cell("DT_Edicts", "edict_test_order", "EnactCost", dict(upkeep, Influence=-5.0))), "EnactCost", "below")
        # A per-tick column takes only the effects that change an amount.
        self.expect(self.after(self.set_cell("DT_Edicts", "edict_test_order", "EffectsPerTick", [eff("ResolveHazard")])), "EffectsPerTick", "ResolveHazard")
        rules = lambda key, value: self.set_cell("DT_Rules", "rules", key, value)  # noqa: E731
        self.assertEqual(self.after(rules("StartingEdict", "edict_test_order")), [])
        self.expect(self.after(rules("StartingEdict", "edict_nobody")), "StartingEdict", "edict_nobody")
        self.expect(self.after(rules("EdictSlotSocietyMarks", [1.0, 0.5])), "EdictSlotSocietyMarks", "do not rise")
        self.expect(self.after(rules("EdictSlotSocietyMarks", [0.5, 1.5])), "EdictSlotSocietyMarks", "above")
        self.expect(self.after(rules("SocietyRecoveryBonus", -0.1)), "SocietyRecoveryBonus", "below")
        check = lambda text: checker.check_string_table("ST_X.csv", text, self.schema)  # noqa: E731
        for prefix in ("Edict.", "Resource."):
            for word in holy + [self.schema["words"]["verdict"][0]]:
                with self.subTest(prefix=prefix, word=word):
                    self.expect(check('Key,SourceString\n"%sA","%s a."\n' % (prefix, word)), "entry 1 (%sA)" % prefix, "the word %s is not used here" % word)
        self.expect(check('Key,SourceString\n"Edict.A","3"\n'), "entry 1 (Edict.A)", "a digit outside a slot")
        self.assertEqual(check('Key,SourceString\n"Resource.A","3"\n'), [])
        # The soul economy's verb (docs/70): never for revoking an edict, in its texts, its keys, the first hint of the
        # inherited edict, the focus keys or the narrator's lines.
        soul = self.schema["words"]["soul_verbs"]
        self.assertEqual(soul, ["\uac70\ub450", "\uac70\ub46c", "\uac70\ub480", "\uac70\ub454", "\uac70\ub458"])
        self.expect(self.after(self.set_cell("DT_Edicts", "edict_test_order", "Summary", "\uce59\ub839\uc744 \uac70\ub450\uc5b4\ub4e4\uc778\ub2e4.")),
                    "DT_Edicts.edict_test_order.Summary", "the word \uac70\ub450 is not used here")
        for key in ("Edict.A", "Hint.EdictA", "Focus.A"):
            with self.subTest(key=key):
                self.expect(check('Key,SourceString\n"%s","\uce59\ub839\uc744 \uac70\ub454\ub2e4."\n' % key), "entry 1 (%s)" % key, "the word \uac70\ub454 is not used here")
        log = lambda tables: self.row(tables, "DT_LogTemplates", "log_edict_lapsed").update(Templates=["{Civ} \uac70\ub46c a.", "b.", "c."])  # noqa: E731
        self.expect(self.after(log), "log_edict_lapsed", "the word \uac70\ub46c is not used here")
        # The two rules of an edict's effects in the column: no duration, and a relation names its party.
        self.expect(self.after(self.set_cell("DT_Edicts", "edict_test_order", "EffectsPerTick", [eff("ChangeStability", value=-0.001, duration=100)])),
                    "EffectsPerTick[0](ChangeStability).DurationTicks", "without a duration")
        self.expect(self.after(self.set_cell("DT_Edicts", "edict_test_order", "EffectsPerTick", [eff("ChangeRelation", value=0.1)])),
                    "EffectsPerTick[0](ChangeRelation).Key", "names its Key")
        self.assertEqual(self.after(self.set_cell("DT_Edicts", "edict_test_order", "EffectsPerTick", [eff("ChangeRelation", key="trans_covenant", value=0.1)])), [])
        self.schema_problem(self.put(["tables", "DT_Edicts", "columns", "EffectsPerTick"], "keyed", "ChangeRelation"), "DT_Edicts.EffectsPerTick", "keyed")
        self.schema_problem(self.put(["tables", "DT_Edicts", "columns", "EffectsPerTick"], "no_duration", "yes"), "DT_Edicts.EffectsPerTick", "no_duration")
        # Both sides of each new knob's bounds, the root of the countdown map, the type of a list of word lists under a
        # prefix, and an edict row taken as a hazard's answer.
        for knob, value, word in (("AIEdictRevokeStability", 1.5, "above"), ("AIEdictRevokeStability", -0.1, "below"),
                                  ("SocietyRecoveryBonus", 1.5, "above"), ("EdictSlotSocietyMarks", [-0.1, 1.0], "below")):
            with self.subTest(knob=knob, value=value):
                self.expect(self.after(rules(knob, value)), knob, word)
        self.expect(self.after(self.set_cell("DT_Edicts", "edict_test_order", "HazardCountdownBonus", {"Law.Hedge": 1})), "HazardCountdownBonus", "Hazard.")
        self.schema_problem(self.put(["string_table", "checked_prefixes"], "Edict.", {"forbidden": [1]}), "schema.string_table.checked_prefixes.Edict.", "a string or a list of strings")
        self.assertEqual(self.after(self.set_cell("DT_Hazards", "crisis_test_storm", "RulerResolutions", ["edict_test_order"])), [])

    def test_death_and_succession_knobs(self):
        """B5 ([B-5b], header section 3.1): the leap, the birth protection, the succession's knobs and the deposition's decay
        have their bounds, and the leap's longest is at least its shortest (the row test ge_col)."""
        self.assertEqual(self.after(), [])
        rules = lambda key, value: self.set_cell("DT_Rules", "rules", key, value)  # noqa: E731
        for knob, value, word in (("LeapTicksMin", -1, "below"), ("LeapTicksMax", -1, "below"), ("LeapTotalCap", -1, "below"),
                                  ("BirthProtectionTicks", -1, "below"), ("SuccessionHazardWeight", -0.5, "below"),
                                  ("SuccessionCivTierWeight", -0.5, "below"), ("SuccessionReverenceWeight", -0.5, "below"),
                                  ("SuccessionReverenceThreshold", -0.1, "below"), ("SuccessionSameCivPenalty", -0.5, "below"),
                                  ("SuccessionMinHazardTicks", -1, "below"), ("ReverenceDecay", -0.1, "below"), ("ReverenceDecay", 1.5, "above")):
            with self.subTest(knob=knob, value=value):
                self.expect(self.after(rules(knob, value)), knob, word)
        self.expect(self.after(rules("LeapTicksMax", 700)), "DT_Rules.rules", "between its shortest and its longest")
        self.assertEqual(self.after(rules("LeapTicksMax", 800)), [])

    def test_diplomacy_and_conflict(self):
        """B3: both sides of the new knobs' bounds and the lengths of their lists, the two new edict columns, the stance
        effect (a card's answer only, friendly or neutral), the two new slots and the holy words of the log lines, and the
        stance and treaty keys of the String Table."""
        self.assertEqual(self.after(), [])
        rules = lambda key, value: self.set_cell("DT_Rules", "rules", key, value)  # noqa: E731
        for knob, value, word in (("RelationDriftRate", 1.5, "above"), ("RelationDriftRate", -0.1, "below"),
                                  ("RelationEthicsWeight", -0.1, "below"), ("RelationStanceTerms", [0.4], "exactly 2"),
                                  ("RelationStanceTerms", [0.4, -1.5], "below"), ("RelationTreatyTerm", 1.5, "above"),
                                  ("ConquestRelationPenalty", -0.1, "below"), ("BetrayalRelationPenalty", 2.5, "above"),
                                  ("FriendlyUpkeepInfluence", -0.1, "below"), ("FriendlySlotsByTier", [1, 1, 2, 2], "exactly 5"),
                                  ("FriendlySlotsByTier", [1, 1, 2, 2, 2.5], "whole number"), ("FriendlySlotsByTier", [1, 1, 2, 2, -1], "below"),
                                  ("TreatyCostInfluence", -1.0, "below"), ("TreatyAcceptRelation", 1.5, "above"),
                                  ("TradeInfluencePerTreaty", -0.1, "below"), ("FleetPerSettledByTier", [0.0, 1.0, 1.0, 1.0], "exactly 5"),
                                  ("FleetPerSettledByTier", [0.0, -1.0, 1.0, 1.0, 1.0], "below"), ("FleetGrowthPerTick", -0.1, "below"),
                                  ("FleetUpkeepEnergyPerPower", -0.1, "below"), ("SiegeTicks", 0, "below"), ("SiegeTicks", 2.5, "whole number"),
                                  ("SiegeRestTicks", -1, "below"), ("SiegeDefenceShares", [1.0, 0.5], "exactly 3"),
                                  ("SiegeDefenceShares", [1.0, 0.5, 1.5], "above"), ("SiegeRepulsedFleetFactor", 1.5, "above"),
                                  ("AIHostileRelation", -1.5, "below"), ("AIHostilePowerRatio", -0.1, "below"), ("AIHostileRelationTowardRuled", 1.5, "above"), ("AIHostilePowerRatioTowardRuled", -0.1, "below"),
                                  ("AIHostilePacifismMax", 1.5, "above"), ("PlagueSpreadMultiplier", -1.0, "below"),
                                  ("FleetDelayPerPower", -1.0, "below"), ("FleetDelayCap", -1.0, "below"), ("PressureFleetAttrition", -0.1, "below"),
                                  # B6b [B-6b2c] (header 2.10): the share of the countdown that caps the guard's delay, 0..1.
                                  ("FleetDelayCountdownShare", -0.1, "below"), ("FleetDelayCountdownShare", 1.5, "above")):
            with self.subTest(knob=knob, value=value):
                self.expect(self.after(rules(knob, value)), knob, word)
        for column, value, word in (("FleetGrowthMultiplier", -0.5, "below"), ("TreatyAcceptBonus", -0.1, "below"), ("TreatyAcceptBonus", 1.5, "above")):
            with self.subTest(column=column, value=value):
                self.expect(self.after(self.set_cell("DT_Edicts", "edict_test_order", column, value)), "DT_Edicts.edict_test_order.%s" % column, word)
        # A growth factor of 0 would stop the inherited edict's fleet from the first tick: the row rule keeps it above 0.
        growth = lambda value: self.set_cell("DT_Edicts", "edict_test_order", "FleetGrowthMultiplier", value)  # noqa: E731
        self.expect(self.after(growth(0.0)), "DT_Edicts.edict_test_order", "a held edict multiplies the fleet's growth")
        self.assertEqual(self.after(growth(0.5)), [])
        # The stance effect stands in the first contact card's answer, friendly or neutral, and nowhere else.
        answer = lambda effects: (lambda tables: self.row(tables, "DT_Encounters", "petition_first_contact")["Choices"][0].update(OnSuccess=effects))  # noqa: E731
        self.assertEqual(self.after(answer([eff("SetStance", tag="Stance.Friendly")])), [])
        self.assertEqual(self.after(answer([eff("SetStance")])), [])
        self.expect(self.after(answer([eff("SetStance", tag="Stance.Hostile")])), "SetStance", "Stance.Friendly")
        self.expect(self.after(answer([eff("SetStance", tag="Stance.Friendly", value=1)])), "SetStance", "reads no Value")
        self.expect(self.after(self.set_cell("DT_Events", "event_test_harvest", "Effects", [eff("SetStance", tag="Stance.Friendly")])),
                    "DT_Events.event_test_harvest.Effects[0](SetStance)", "only in the answer of a card")
        self.expect(self.after(self.set_cell("DT_Hazards", "crisis_test_storm", "OnExpireEffects", [eff("SetStance", tag="Stance.Friendly")])),
                    "OnExpireEffects[0](SetStance)", "only in the answer of a card")
        self.expect(self.after(self.set_cell("DT_Edicts", "edict_test_order", "EffectsPerTick", [eff("SetStance", tag="Stance.Friendly")])),
                    "EffectsPerTick[0](SetStance)", "per-tick source applies only")
        self.schema_problem(self.put(["effects", "SetStance"], "card_only", "yes"), "schema.effects.SetStance", "card_only")
        self.schema_problem(self.put(["tables", "DT_Encounters", "columns", "Choices", "columns", "OnSuccess"], "card", "yes"), "OnSuccess", "card")
        # Only the first contact card names another people and sets a stance - not another petition, not a wanderer's card.
        update_cell = lambda tables: self.row(tables, "DT_Encounters", "petition_test_plea").update(Body="{OtherCiv} asks.")  # noqa: E731
        self.expect(self.after(update_cell), "DT_Encounters.petition_test_plea", "only the first contact card is about another people")
        self.assertEqual(self.after(lambda tables: self.row(tables, "DT_Encounters", "petition_first_contact").update(Body="{OtherCiv} asks.")), [])
        for row_name, column in (("petition_test_plea", "OnSuccess"), ("petition_test_plea", "OnFailure"), ("enc_test_weave", "OnSuccess")):
            with self.subTest(row=row_name, column=column):
                stance = lambda tables, row_name=row_name, column=column: self.row(tables, "DT_Encounters", row_name)["Choices"][0].update(
                    **{column: [eff("SetStance", tag="Stance.Friendly")]})  # noqa: E731
                self.expect(self.after(stance), "DT_Encounters.%s" % row_name, "only the first contact card is about another people", "SetStance")
        # No chain leads to the first contact card: put by an answer, it would be about no one met.
        chain = lambda tables: self.row(tables, "DT_Encounters", "petition_test_plea")["Choices"][0].update(NextEncounter="petition_first_contact")  # noqa: E731
        self.expect(self.after(chain), "NextEncounter", "row petition_first_contact of DT_Encounters does not hold")
        # The first contact card is put by name, about the ruled people.
        first = lambda **changes: (lambda tables: self.row(tables, "DT_Encounters", "petition_first_contact").update(**changes))  # noqa: E731
        self.expect(self.after(first(Trigger="Standing", BaseWeight=1.0)), "DT_Encounters.petition_first_contact", "the first meeting puts the first contact card by name")
        self.expect(self.after(first(Subject="SettledSystem")), "DT_Encounters.petition_first_contact", "the first meeting puts the first contact card by name")
        # The log lines fill {Stance} and {Treaty} and refuse the holy words.
        line = lambda templates: (lambda tables: self.row(tables, "DT_LogTemplates", "log_treaty_signed").update(Templates=templates))  # noqa: E731
        self.assertEqual(self.after(line(["{Civ} {Treaty} a.", "{Civ} {Stance} b.", "{OtherCiv} c."])), [])
        self.expect(self.after(line(["{Civ} {Pact} a.", "b.", "c."])), "log_treaty_signed", "slot {Pact}")
        for word in self.schema["words"]["holy_words"]:
            with self.subTest(word=word):
                self.expect(self.after(line(["{Civ} %s a." % word, "b.", "c."])), "log_treaty_signed", "the word %s is not used here" % word)
        # The stance and treaty keys of the String Table carry no digit and no verdict word.
        for prefix in ("Stance.", "Treaty."):
            with self.subTest(prefix=prefix):
                self.assertIn(prefix, self.schema["string_table"]["checked_prefixes"])
                self.assertTrue(self.schema["string_table"]["checked_prefixes"][prefix].get("no_digits"))

    def test_pressure_cards(self):
        """B4: the edict and the delay a card's answer carries - a petition's answer only, its OnSuccess alone, a choice
        not rolled for, the edict's own cost the one price, never the capital rule's hazard nor a hazard a card waits on
        -, the condition on a held edict (never on a grace row) and the card that is never blocked. Two bans on one row
        are broken apart (CLAUDE.md rule 8)."""
        self.assertEqual(self.after(), [])
        plea = "petition_test_plea"
        enact = eff("EnactEdict", key="edict_test_order")
        delay = eff("DelayHazard", tag="Hazard.Plague", value=50)

        def entry_of(index, row_name=plea, **changes):
            return lambda tables: self.row(tables, "DT_Encounters", row_name)["Choices"][index].update(**changes)

        def both(*changes):
            return lambda tables: [change(tables) for change in changes]
        ruler_only = "an edict and a fleet are the ruler's"
        success_only = "the gate of a choice reads its OnSuccess alone"
        unrolled = "a choice that enacts an edict or buys time is not rolled for"
        one_price = "the edict's own EnactCost is the one price"
        never_blocked = "a card is never blocked"
        card_rules = (ruler_only, success_only, unrolled, one_price, never_blocked)

        def only(problems, why, *words):
            """The named rule breaks and no other rule of the cards does."""
            self.expect(problems, "DT_Encounters.", why, *words)
            for other in card_rules:
                if other != why:
                    self.expect(problems, other, wanted=False)
        # (1) The two effects stand in a card's answer and nowhere else, and neither is a per-tick effect.
        for effect in (enact, delay):
            with self.subTest(effect=effect["Type"], where="answer"):
                self.assertEqual(self.after(entry_of(0, OnSuccess=[effect])), [])
                self.expect(self.after(self.set_cell("DT_Events", "event_test_harvest", "Effects", [effect])),
                            "DT_Events.event_test_harvest.Effects[0](%s)" % effect["Type"], "only in the answer of a card")
                self.expect(self.after(self.set_cell("DT_Edicts", "edict_test_order", "EffectsPerTick", [effect])),
                            "EffectsPerTick[0](%s)" % effect["Type"], "per-tick source applies only")
                self.assertNotIn(effect["Type"], self.schema["per_tick_effects"])
        # EnactEdict names an edict row; DelayHazard a hazard tag and 1..100 whole ticks; neither reads another cell.
        answer = lambda *effects: self.after(entry_of(0, OnSuccess=list(effects)))  # noqa: E731
        self.expect(answer(eff("EnactEdict")), "Choices[0].OnSuccess[0](EnactEdict).Key", "a Key is required")
        self.expect(answer(eff("EnactEdict", key="edict_nobody")), "OnSuccess[0](EnactEdict).Key", "DT_Edicts has no row edict_nobody")
        self.expect(answer(eff("EnactEdict", key="edict_test_order", tag="Hazard.Plague")), "(EnactEdict).Tag", "reads no Tag")
        self.expect(answer(eff("EnactEdict", key="edict_test_order", value=1)), "(EnactEdict).Value", "reads no Value")
        self.expect(answer(eff("DelayHazard", value=50)), "Choices[0].OnSuccess[0](DelayHazard).Tag", "a tag is required")
        self.expect(answer(eff("DelayHazard", tag="Law.Hedge", value=50)), "(DelayHazard).Tag", "not under Hazard.")
        self.expect(answer(eff("DelayHazard", tag="Hazard.Plague", key="crisis_test_storm", value=50)), "(DelayHazard).Key", "reads no Key")
        for value, words in ((0, "is below 1"), (101, "is above 100"), (2.5, "not a whole number")):
            with self.subTest(value=value):
                self.expect(answer(eff("DelayHazard", tag="Hazard.Plague", value=value)), "(DelayHazard).Value", words)
        for value in (1, 100):
            self.assertEqual(answer(eff("DelayHazard", tag="Hazard.Plague", value=value)), [])
        # (2) The condition on a held edict: an edict row, nothing else read - and never on a grace row (every grace tag).
        has_edict = cond("CivHasEdict", key="edict_test_order")
        self.assertEqual(self.conditions(has_edict), [])
        self.expect(self.conditions(cond("CivHasEdict")), "(CivHasEdict).Key", "a Key is required")
        self.expect(self.conditions(cond("CivHasEdict", key="edict_nobody")), "(CivHasEdict).Key", "DT_Edicts has no row edict_nobody")
        self.expect(self.conditions(cond("CivHasEdict", key="edict_test_order", tag="Hazard.Plague")), "(CivHasEdict).Tag", "reads no Tag")
        self.expect(self.conditions(cond("CivHasEdict", key="edict_test_order", value=1)), "(CivHasEdict).Value", "reads no Value")
        grace = "grace is never drawn by an edict"
        graces = sorted(tag for tag in self.tags if tag.startswith("Event.Grace."))
        rule = next(rule for rule in self.schema["tables"]["DT_Events"]["rules"] if rule["why"].startswith(grace))
        self.assertEqual(sorted(rule["when"][0]["in"]), graces, "the rule lists every grace category the ini declares")
        for category in graces:
            with self.subTest(category=category):
                on_grace = lambda tables, category=category: self.row(tables, "DT_Events", "event_test_harvest").update(  # noqa: E731
                    Category=category, LogCategory=self.schema["event_log_categories"][category][0], Conditions=[has_edict])
                self.expect(self.after(on_grace), "DT_Events.event_test_harvest", grace, "CivHasEdict")
        # (3) A petition's answer only, its OnSuccess alone, a choice not rolled for, the edict's cost the one price: each
        # broken in a choice that breaks no other rule of the cards.
        checked = [stat for stat in self.schema["enums"]["ECosmicCheckStat"] if stat != "NoCheck"]
        # The rule says 'not in enc_ rows' for 'only in petition_ rows': the same while these are the two prefixes.
        self.assertEqual(sorted(self.schema["tables"]["DT_Encounters"]["prefixes"]), ["enc_", "petition_"],
                         "a new card prefix: decide whether EnactEdict/DelayHazard may stand in it - the ruler_only rule names enc_ alone")
        for effect in (enact, delay):
            with self.subTest(effect=effect["Type"]):
                only(self.after(entry_of(1, "enc_test_weave", OnSuccess=[effect])), ruler_only, effect["Type"])
                only(self.after(entry_of(0, OnFailure=[effect])), success_only, effect["Type"])
                for stat in checked:
                    only(self.after(entry_of(0, CheckStat=stat, Difficulty=4, OnSuccess=[effect])), unrolled, effect["Type"])
        only(self.after(entry_of(0, Cost=pool(Influence=10.0), OnSuccess=[enact])), one_price, "EnactEdict")
        self.assertEqual(self.after(entry_of(0, Cost=pool(Influence=10.0))), [])
        # (4) A card is never blocked: a gated choice beside choices that all cost, or that are all gated, breaks the rule.
        stance = eff("SetStance", tag="Stance.Friendly")
        contact = "petition_first_contact"
        wait_costs = lambda row_name=plea: entry_of(1, row_name, Cost=pool(Energy=5.0))  # noqa: E731
        for effect, row_name in ((enact, plea), (delay, plea), (stance, contact)):
            with self.subTest(effect=effect["Type"], blocked=True):
                self.assertEqual(self.after(entry_of(0, row_name, OnSuccess=[effect])), [])
                only(self.after(both(entry_of(0, row_name, OnSuccess=[effect]), wait_costs(row_name))), never_blocked, effect["Type"])
        only(self.after(both(entry_of(0, OnSuccess=[enact]), entry_of(1, OnSuccess=[delay]))), never_blocked)
        # A choice with no cost and no gate keeps the card open, rolled for or not; a card without a gate may ask a price
        # in every choice (petition_feast).
        rolled = entry_of(1, CheckStat="Stability", Difficulty=3, OnFailure=[eff("ChangeStability", value=-0.05)], FailureText="{Civ} failed.")
        self.assertEqual(self.after(both(entry_of(0, OnSuccess=[enact]), rolled)), [])
        self.assertEqual(self.after(both(entry_of(0, Cost=pool(Energy=5.0)), wait_costs())), [])
        # (5) The time a card buys never falls on the capital rule's hazard, nor on a hazard a card waits on with
        # SystemHazardTicksAtMost. The project's rebellion is under both bans; two test rows hold them apart.
        capital = "is the tag of row"
        waited = "is read by a SystemHazardTicksAtMost condition"
        capital_row = lambda tables: tables["DT_Hazards"].append(  # noqa: E731
            hazard_row("crisis_test_uprising", HazardTag="Hazard.PreSupernova", SpawnRule="Capital", SpawnChancePerTick=0.0))
        watching_card = lambda tables: tables["DT_Encounters"].append(encounter_row(  # noqa: E731
            "petition_test_watch", Subject="SettledSystem", Conditions=[cond("PlayerRoleIs", key="Ruler"), cond("SystemHazardTicksAtMost", tag="Hazard.Plague", value=50)]))
        delay_on = lambda tag: entry_of(0, OnSuccess=[eff("DelayHazard", tag=tag, value=50)])  # noqa: E731
        self.assertEqual(self.after(capital_row), [])
        self.assertEqual(self.after(watching_card), [])
        problems = self.after(both(capital_row, delay_on("Hazard.PreSupernova")))
        self.expect(problems, "Choices[0].OnSuccess[0](DelayHazard).Tag", capital, "crisis_test_uprising of DT_Hazards")
        self.expect(problems, waited, wanted=False)
        problems = self.after(both(watching_card, delay_on("Hazard.Plague")))
        self.expect(problems, "Choices[0].OnSuccess[0](DelayHazard).Tag", waited, "DT_Encounters.petition_test_watch.Conditions")
        self.expect(problems, capital, wanted=False)
        # A condition of any table waits on the countdown as well, not only a card's: an event row that reads it.
        watching_event = self.set_cell("DT_Events", "event_test_harvest", "Conditions", [cond("SystemHazardTicksAtMost", tag="Hazard.Plague", value=50)])
        self.assertEqual(self.after(watching_event), [])
        problems = self.after(both(watching_event, delay_on("Hazard.Plague")))
        self.expect(problems, "Choices[0].OnSuccess[0](DelayHazard).Tag", waited, "DT_Events.event_test_harvest.Conditions")
        self.expect(problems, capital, wanted=False)
        # Without the two rows the same tags are free; a Draw row of the tag is no capital rule, and a card that asks only
        # whether the hazard stands does not wait on its countdown.
        self.assertEqual(self.after(delay_on("Hazard.PreSupernova")), [])
        self.assertEqual(self.after(delay_on("Hazard.Plague")), [])
        drawn_row = lambda tables: tables["DT_Hazards"].append(hazard_row("crisis_test_drawn", HazardTag="Hazard.PreSupernova"))  # noqa: E731
        self.assertEqual(self.after(both(drawn_row, delay_on("Hazard.PreSupernova"))), [])
        seeing_card = lambda tables: tables["DT_Encounters"].append(encounter_row(  # noqa: E731
            "petition_test_seen", Subject="SettledSystem", Conditions=[cond("PlayerRoleIs", key="Ruler"), cond("SystemHasHazard", tag="Hazard.Plague")]))
        self.assertEqual(self.after(both(seeing_card, delay_on("Hazard.Plague"))), [])
        # A condition read in a list of structs counts as well (a conditions column of the choices, put here).
        requires = self.put(["tables", "DT_Encounters", "columns", "Choices", "columns"], "Requires", {"kind": "conditions"})
        in_choice = entry_of(1, Requires=[cond("SystemHazardTicksAtMost", tag="Hazard.Plague", value=50)])
        self.expect(self.after(both(in_choice, delay_on("Hazard.Plague")), requires), "(DelayHazard).Tag", waited, "DT_Encounters.petition_test_plea.Choices.Requires")
        # The two refusals are names the schema has to hold: a table, a tag column of it, tests of its rows, conditions
        # that read a Tag.
        tag = ["effects", "DelayHazard", "tag"]
        owner = tag + ["not_tag_of"]
        where = "schema.effects.DelayHazard.tag"
        self.schema_problem(self.put(tag, "not_read_by", ["SystemHazardTicksAtMots"]), where, "'SystemHazardTicksAtMots', which is no enumerator of conditions")
        self.schema_problem(self.put(tag, "not_read_by", ["CivTierAtLeast"]), where, "CivTierAtLeast, a condition that reads no Tag")
        self.schema_problem(self.put(tag, "not_read_by", "SystemHazardTicksAtMost"), where, "'not_read_by' has to be a list of strings")
        self.schema_problem(self.put(tag, "not_tag_of", [{"table": "DT_Hazards", "col": "HazardTag"}]), where, "'not_tag_of' has to be an object")
        self.schema_problem(self.put(owner, "table", "DT_Hazardz"), where, "the table DT_Hazardz is not in the schema")
        self.schema_problem(self.put(owner, "col", "DisplayName"), where, "not_tag_of names the column 'DisplayName', which DT_Hazards does not declare as a tag")
        self.schema_problem(self.put(owner, "col", "HazardTagg"), where, "not_tag_of names the column 'HazardTagg'")
        self.schema_problem(self.put(owner, "when", [{"col": "SpawnRule", "is": "Capitol"}]), where + ".not_tag_of", "'Capitol', which is no enumerator of ECosmicHazardSpawn")
        self.schema_problem(self.drop(owner, "col"), where + ".not_tag_of", "'col' is missing")
        self.schema_problem(self.put(owner, "where", []), where + ".not_tag_of", "'where' is not read here")

    def test_voice_and_drain(self):
        """B4: the two new columns (an edict's factor on the pressure's drain, the rules' share of the fleet a delay
        leaves), the counterfeit a fallen row names, the empty voice lines, no weight on the claim's events, the
        first-person event row, the claim's line categories, and the three new tests of a rule (none, zero, contains_none)."""
        self.assertEqual(self.after(), [])
        drain = lambda value: self.set_cell("DT_Edicts", "edict_test_order", "PressureDrainMultiplier", value)  # noqa: E731
        self.expect(self.after(drain(0.0)), "DT_Edicts.edict_test_order", "a held edict damps the pressure's drain")
        self.expect(self.after(drain(-0.5)), "DT_Edicts.edict_test_order.PressureDrainMultiplier", "is below 0")
        self.expect(self.after(drain(1.5)), "DT_Edicts.edict_test_order.PressureDrainMultiplier", "is above 1")
        for value in (0.45, 1.0):
            self.assertEqual(self.after(drain(value)), [])
        resist = lambda value: self.set_cell("DT_Rules", "rules", "ResistFleetFactor", value)  # noqa: E731
        self.expect(self.after(resist(-0.1)), "rules.ResistFleetFactor", "is below 0")
        self.expect(self.after(resist(1.1)), "rules.ResistFleetFactor", "is above 1")
        for value in (0.0, 1.0):
            self.assertEqual(self.after(resist(value)), [])
        # A fallen row names the truth it counterfeits, under Truth.; a row that is not fallen need not.
        fallen = lambda column, value: self.set_cell("DT_Transcendents", "trans_silence", column, value)  # noqa: E731
        counterfeits = "a fallen one counterfeits a truth"
        self.assertIn("Truth.Rest", self.tags)
        self.expect(self.after(fallen("CounterfeitOf", "")), "DT_Transcendents.trans_silence", counterfeits)
        problems = self.after(fallen("CounterfeitOf", "Doctrine.Silence"))
        self.expect(problems, "trans_silence.CounterfeitOf", "not under Truth.")
        self.expect(problems, counterfeits, wanted=False)
        self.assertEqual(self.after(self.set_cell("DT_Transcendents", "trans_covenant", "CounterfeitOf", "")), [])
        # The voice lines stay empty in every row; no row weighs the claim's events (another category may be weighed).
        for row_name in ("trans_silence", "trans_covenant"):
            with self.subTest(row=row_name):
                self.expect(self.after(self.set_cell("DT_Transcendents", row_name, "VoiceTemplates", ["a."])), "DT_Transcendents.%s" % row_name,
                            "nothing reads a transcendent's voice lines")
        weighs = "no transcendent weighs the claim's events"
        self.expect(self.after(fallen("EventWeightModifiers", {"Event.Civ.Economy": 1.5, "Event.Crisis.Claim": 1.0})), "DT_Transcendents.trans_silence", weighs)
        self.assertEqual(self.after(fallen("EventWeightModifiers", {"Event.Crisis.Rebellion": 2.0})), [])
        # A transcendent's own line waits for the third tier and comes with its sign row: the two broken apart.
        first_person = "a transcendent's own line is first person"
        voice = lambda **changes: (lambda tables: self.row(tables, "DT_Events", "event_test_harvest").update(  # noqa: E731
            dict(dict(Category="Event.Crisis.Transcendent", LogCategory="Log.Transcendent"), **changes)))
        signed = lambda min_tier: (lambda tables: (self.add_sign_row(tables), voice(MinTierToReveal=min_tier)(tables)))  # noqa: E731
        self.assertEqual(self.after(signed(3)), [])
        problems = self.after(signed(2))
        self.expect(problems, "DT_Events.event_test_harvest", first_person, '"ge": 3')
        self.expect(problems, first_person, '"SignRow"', wanted=False)
        problems = self.after(voice(MinTierToReveal=3))
        self.expect(problems, "DT_Events.event_test_harvest", first_person, '"SignRow"')
        self.expect(problems, first_person, '"ge": 3', wanted=False)
        self.assertEqual(self.after(voice(LogCategory="Log.Crisis")), [])
        # The claim's event rows tell the narrator's line: a first-person category is no line of theirs.
        claim_line = lambda log: (lambda tables: self.row(tables, "DT_Events", "event_test_harvest").update(  # noqa: E731
            Category="Event.Crisis.Claim", LogCategory=log, Trigger="Named", BaseWeight=0, CooldownTicks=0))
        self.expect(self.after(claim_line("Log.Claim")), "event_test_harvest.LogCategory", "Event.Crisis.Claim may tell Log.Crisis, found Log.Claim")
        # B5 [B-5d1] (decision 11-ga): the record lines of DT_LogTemplates are part one's only producers - an event row that told
        # Log.Record would put a line into the two books outside the record rows' frame.
        self.expect(self.after(claim_line("Log.Record")), "event_test_harvest.LogCategory", "Event.Crisis.Claim may tell Log.Crisis, found Log.Record")
        self.assertEqual(self.after(claim_line("Log.Crisis")), [])
        # The new tests: each fits its kinds and its operand has its type.
        civ_rule = ["tables", "DT_Civs", "rules", 0]
        self.schema_problem(self.put(civ_rule, "when", [{"col": "DisplayName", "contains_none": ["Trait.Arch.Seeker"]}]), "schema.DT_Civs",
                            "the test 'contains_none' cannot be asked of a column of kind text")
        self.schema_problem(self.put(civ_rule, "when", [{"col": "Traits", "contains_none": ["Trait.Arch.Nobody"]}]), "schema.DT_Civs", "names the tag Trait.Arch.Nobody")
        self.schema_problem(self.put(civ_rule, "when", [{"col": "Traits", "contains_none": []}]), "schema.DT_Civs", "the test 'contains_none' needs a list of strings, not empty")
        self.schema_problem(self.put(civ_rule, "when", [{"col": "Weight", "zero": True}]), "schema.DT_Civs", "the test 'zero' cannot be asked of a column of kind number")
        card_rule = ["tables", "DT_Encounters", "rules", 0]
        self.schema_problem(self.put(card_rule, "when", [{"col": "Choices.Cost", "zero": "yes"}]), "schema.DT_Encounters", "the test 'zero' needs true or false")
        for test, words in (({"none": []}, "'none' takes a list of tests, not empty, and nothing else"),
                            ({"none": {"col": "Choices.Label", "set": True}}, "'none' takes a list of tests, not empty, and nothing else"),
                            ({"none": [{"col": "Choices.Label", "set": True}], "col": "Title"}, "'none' takes a list of tests, not empty, and nothing else"),
                            ({"none": [{"col": "Title", "set": True}]}, "over the columns of one list of structs"),
                            ({"none": [{"col": "Choices.Label", "set": True}, {"col": "Title", "set": True}]}, "over the columns of one list of structs"),
                            ({"none": [{"row": "petition_first_contact"}]}, "over the columns of one list of structs"),
                            ({"none": [{"col": "Choices.Lable", "set": True}]}, "names the column 'Lable', which the list Choices of DT_Encounters does not declare"),
                            ({"none": [{"col": "Choices.Difficulty", "set": True}]}, "cannot be asked of a column of kind number")):
            with self.subTest(test=test):
                self.schema_problem(self.put(card_rule, "when", [test]), "schema.DT_Encounters", words)
        # How each holds: 'none' over the entries of a list, 'zero' over a pool, 'contains_none' over tags and a tag map.
        holds = checker.Checker(self.schema, self.tables, self.tags).holds
        none = {"none": [{"col": "Choices.Difficulty", "gt": 2}, {"col": "Choices.CheckStat", "is": "Body"}]}
        self.assertTrue(holds(none, {"Choices": [{"Difficulty": 3, "CheckStat": "Mind"}, {"Difficulty": 1, "CheckStat": "Body"}]}, "r"))
        self.assertFalse(holds(none, {"Choices": [{"Difficulty": 1, "CheckStat": "Mind"}, {"Difficulty": 3, "CheckStat": "Body"}]}, "r"))
        self.assertTrue(holds(none, {"Choices": []}, "r"))
        self.assertTrue(holds(none, {"Choices": [5, {"Difficulty": 1, "CheckStat": "Body"}]}, "r"))
        for wrong in ({"Choices": "x"}, {}):
            with self.subTest(row=wrong):
                self.assertFalse(holds(none, wrong, "r"))
        zero = lambda operand, value: holds({"col": "C", "zero": operand}, {"C": value}, "r")  # noqa: E731
        self.assertTrue(zero(True, pool()))
        self.assertFalse(zero(True, pool(Matter=0.5)))
        self.assertTrue(zero(False, pool(Matter=0.5)))
        self.assertFalse(zero(False, pool()))
        self.assertFalse(zero(True, pool(Energy="0")))
        for wrong in (None, [0.0], 0):
            with self.subTest(pool=wrong):
                self.assertFalse(zero(True, wrong))
                self.assertFalse(zero(False, wrong))
        none_of = lambda value: holds({"col": "C", "contains_none": ["A.B"]}, {"C": value}, "r")  # noqa: E731
        self.assertTrue(none_of({"A.C": 1.0}))
        self.assertFalse(none_of({"A.B": 1.0}))
        self.assertTrue(none_of(tags_text("A.C")))
        self.assertFalse(none_of(tags_text("A.C", "A.B")))
        self.assertTrue(none_of([]))
        self.assertFalse(none_of(["A.C", "A.B"]))
        self.assertFalse(none_of(None))

    def test_first_person_and_card_words(self):
        """B4 [B-4d1] (header section 3.8; the text audit's D1): the first person's one form of words, written for both its
        columns; the silent one's own four words; no second person in a claim line; the sign's and the fleet line's rows;
        the rescue words wherever they are hung, and the holy words and soul verbs in a petition's every field; the word
        lists themselves (the verdict without the soul, the names no source holds); the new test lacks_words."""
        self.assertEqual(self.after(), [])
        lists = self.schema["words"]
        columns = self.schema["tables"]["DT_Transcendents"]["columns"]
        # One form of the first person's words, the same in both columns (written twice, compared here).
        self.assertEqual(columns["ClaimTemplates"]["forbidden"], columns["Lure"]["forbidden"])
        self.assertEqual(columns["ClaimTemplates"]["forbidden"],
                         ["verdict_without_soul", "holy_words", "holy_and_myth", "realm_names", "rescue_words", "revelation_mimicry"])
        # The verdict without the soul is the verdict less '영혼' alone: the first person may speak of the soul.
        self.assertEqual(lists["verdict_without_soul"], [word for word in lists["verdict"] if word != "영혼"])
        # No name of holy_and_myth is in any source of the project (matched letter for letter: one that hides in a common
        # word is kept out of the list - the audit's hand list).
        sources = checker.read_sources()
        texts = "".join(data.decode("utf-8") for name, data in sources.items() if name != "schema.json")
        self.assertEqual([name for name in lists["holy_and_myth"] if name in texts], [])
        # The audit's whole hand list (D4, header 3.8 (3)): the names that hide in common words never go into the list.
        for hidden in ("에서", "함", "셈", "요한", "단", "마라", "미가", "바로", "그룹", "엘리", "사라", "누가", "하와", "셋", "나가", "이삭", "도마", "한나", "아담"):
            with self.subTest(hidden=hidden):
                self.assertNotIn(hidden, lists["holy_and_myth"])
        claim = lambda lines: self.set_cell("DT_Transcendents", "trans_silence", "ClaimTemplates", lines)  # noqa: E731
        lure = lambda text: self.set_cell("DT_Transcendents", "trans_silence", "Lure", text)  # noqa: E731
        good = ["{System}에서 식은 것은 내 것이다.", "b.", "c."]
        self.assertEqual(self.after(claim(good)), [])
        self.assertEqual(self.after(claim(["{System}의 영혼이 식었다.", "b.", "c."])), [])
        # '음부' is not in realm_names: '처음부터' carries it (letter for letter) - the hand list keeps it until the word
        # boundary check of docs/20 (stage 4).
        self.assertEqual(self.after(claim(["{System}|hpp(은,는) 처음부터 식어 갔다.", "b.", "c."])), [])
        self.assertNotIn("음부", lists["realm_names"])
        for word, why in (("천국", "the word 천국"), ("처음과 나중", "the word 처음과 나중"), ("처음이요", "the word 처음이요"), ("구원", "the word 구원"),
                          ("보좌", "the word 보좌"), ("지극히 높은", "the word 지극히 높은"), ("성령", "the word 성령"), ("여호와", "the word 여호와"),
                          ("눈물을 씻", "the word 눈물을 씻")):
            with self.subTest(word=word):
                self.expect(self.after(claim(["{System} %s a." % word, "b.", "c."])), "trans_silence.ClaimTemplates", why)
                self.expect(self.after(lure("%s a." % word)), "trans_silence.Lure", why)
        self.expect(self.after(claim(["{Transcendent} a.", "b.", "c."])), "trans_silence.ClaimTemplates", "slot {Transcendent}")
        self.expect(self.after(claim(["{System} 3 a.", "b.", "c."])), "trans_silence.ClaimTemplates", "digit")
        self.expect(self.after(lure("{System} a.")), "trans_silence.Lure", "slot {System}")
        self.expect(self.after(lure("3 a.")), "trans_silence.Lure", "digit")
        # The silent one's own four words, in both its columns; a fallen row of another doctrine may use them.
        silence = "the silent one does not take the words of the revelation it counterfeits"
        for word in ("잠잠", "고요", "쉼", "안식"):
            with self.subTest(word=word):
                self.expect(self.after(claim(["{System} %s a." % word, "b.", "c."])), "DT_Transcendents.trans_silence", silence, "ClaimTemplates")
                self.expect(self.after(lure("%s a." % word)), "DT_Transcendents.trans_silence", silence, "Lure")
        other = lambda tables: tables["DT_Transcendents"].append(transcendent_row(  # noqa: E731
            "trans_zeal", Doctrine="Doctrine.Zeal", ClaimTemplates=["{System} 고요 a.", "b.", "c."], Lure="쉼 a."))
        self.assertEqual(self.after(other), [])
        # No second person in a claim line of a fallen row ('건너는' carries '너는' - the known false alarm); nor in its lure
        # (B6a header 3.2: the awareness line is the one second person).
        speaks = "a fallen one's claim line speaks to no one"
        for word in ("너는", "네게", "당신"):
            with self.subTest(word=word):
                self.expect(self.after(claim(["{System} %s a." % word, "b.", "c."])), "DT_Transcendents.trans_silence", speaks)
        for word in ("너는", "네게", "당신"):
            with self.subTest(word=word):
                self.expect(self.after(lure("%s a." % word)), "DT_Transcendents.trans_silence", speaks, "Lure")
        self.assertEqual(self.after(self.set_cell("DT_Transcendents", "trans_covenant", "ClaimTemplates", [])), [])
        # The sign's row and the fleet line's row.
        sign = lambda **changes: (lambda tables: self.row(tables, "DT_LogTemplates", "log_sign_claim").update(**changes))  # noqa: E731
        self.expect(self.after(sign(Category="Log.Conflict")), "DT_LogTemplates.log_sign_claim", "the sign of a claim", '"is": "Log.Sign"')
        self.expect(self.after(sign(Templates=["{System} {Hazard} a.", "b.", "c."])), "DT_LogTemplates.log_sign_claim", "the sign of a claim", "slots_in")
        self.assertEqual(self.after(sign(Templates=["{System} {Civ} a.", "b.", "c."])), [])
        worn = lambda **changes: (lambda tables: self.row(tables, "DT_LogTemplates", "log_fleet_worn").update(**changes))  # noqa: E731
        for word in ("밤", "철야", "금식", "절식", "기도"):
            with self.subTest(word=word):
                self.expect(self.after(worn(Templates=["{System} %s a." % word, "b.", "c."])), "DT_LogTemplates.log_fleet_worn", "no image of the night's vigil")
        self.expect(self.after(worn(Category="Log.Sign")), "DT_LogTemplates.log_fleet_worn", '"is": "Log.Conflict"')
        # The rescue words wherever they are hung.
        # The holy words and the soul verbs newly hung on the events' lines.
        for value, why in ((["{Civ} 성령 a.", "b.", "c."], "the word 성령"), (["{Civ} 거두 a.", "b.", "c."], "the word 거두")):
            with self.subTest(why=why):
                self.expect(self.after(self.set_cell("DT_Events", "event_test_harvest", "LogTemplates", value)), "event_test_harvest.LogTemplates", why)
        for table, name, column, value in (
                ("DT_Events", "event_test_harvest", "LogTemplates", ["{Civ} 구원 a.", "b.", "c."]),
                ("DT_Hazards", "crisis_test_storm", "OutbreakTemplates", ["{System} 중보 a.", "b.", "c."]),
                ("DT_Hazards", "crisis_test_storm", "ExpireTemplates", ["{System} 구원 a.", "b.", "c."]),
                ("DT_Hazards", "crisis_test_storm", "ResolveTemplates", ["{System} 구원 a.", "b.", "c."]),
                ("DT_Megaprojects", "project_test_build", "CompleteTemplates", ["{Civ} 구원 a.", "b.", "c."]),
                ("DT_Edicts", "edict_test_order", "DisplayName", "구원"),
                ("DT_Edicts", "edict_test_order", "Summary", "중보 a."),
                ("DT_LogTemplates", "log_fleet_worn", "Templates", ["{System} 구원 a.", "b.", "c."])):
            with self.subTest(table=table, column=column):
                self.expect(self.after(self.set_cell(table, name, column, value)), "%s.%s" % (name, column), "is not used here")
        check = lambda text: checker.check_string_table("ST_X.csv", text, self.schema)  # noqa: E731
        for key in ("Petition.A", "Edict.A"):
            with self.subTest(key=key):
                self.expect(check('Key,SourceString\n"%s","구원 a."\n' % key), "entry 1 (%s)" % key, "the word 구원 is not used here")
        # A petition's every field: no holy word, no soul verb, no rescue word; a wanderer's card may carry them (the
        # quotations of the path cards - docs/70:178 - are decided with those cards).
        petition = "a petition's words name no holy word, no soul verb and no rescue word"
        plea = "petition_test_plea"
        choices = self.row(self.tables, "DT_Encounters", plea)["Choices"]
        for column, value in (("Title", "성령 a."), ("Body", "거둔 a."), ("VoiceBodies", ["구원 a.", "b."])):
            with self.subTest(column=column):
                self.expect(self.after(self.set_cell("DT_Encounters", plea, column, value)), "DT_Encounters.%s" % plea, petition)
        for field, word in (("Label", "은혜"), ("SuccessText", "거두"), ("FailureText", "중보")):
            with self.subTest(field=field):
                changed = [dict(choices[0], **{field: "%s a." % word})] + choices[1:]
                self.expect(self.after(self.set_cell("DT_Encounters", plea, "Choices", changed)), "DT_Encounters.%s" % plea, petition)
        self.assertEqual(self.after(self.set_cell("DT_Encounters", "enc_test_weave", "Title", "은혜 a.")), [])
        # The rule names every text column of a card, for each of its three lists: a text column added to the table or to
        # its choices is asked again.
        rule = next(rule for rule in self.schema["tables"]["DT_Encounters"]["rules"] if rule["why"].startswith(petition))
        text_columns = {column for table, column, spec in self.columns_of(lambda spec: spec.get("kind") in ("text", "texts")) if table == "DT_Encounters"}
        for listed in ("holy_words", "soul_verbs", "rescue_words"):
            with self.subTest(listed=listed):
                self.assertEqual({test["col"] for test in rule["then"] if test.get("lacks_words") == listed}, text_columns)
        # The vigil row's words come with its row (B4 [B-4d2] - a rule over a row that does not exist yet is refused): once
        # the project holds event_claim_eased, the rule that keeps the night's vigil out of its lines is there too.
        project = json.loads(checker.read_sources()["DT_Events.json"].decode("utf-8"))
        if any(row.get("Name") == "event_claim_eased" for row in project):
            self.assertTrue(any(rule.get("when") == [{"row": "event_claim_eased"}] and {"col": "LogTemplates", "lacks_words": "vigil_image"} in rule.get("then", [])
                                for rule in self.schema["tables"]["DT_Events"]["rules"]), "event_claim_eased came without its vigil_image rule")
        # The singing star and the sanctuary's star never stand in one text (docs/70:131): no text of the project holds
        # '노래' and a word that calls the sanctuary.
        sanctuary = ("성역", "울타리", "지켜지고 있", "허락되지 않")
        shared = [text for text in texts_of_project(sources) if "노래" in text and any(word in text for word in sanctuary)]
        self.assertEqual(shared, [])
        # The new test: it names a word list of the schema and fits a text column.
        rule = ["tables", "DT_Transcendents", "rules", 0]
        self.schema_problem(self.put(rule, "when", [{"col": "Title", "lacks_words": "no_such_list"}]), "schema.DT_Transcendents", "the word list no_such_list is not in the schema")
        self.schema_problem(self.put(rule, "when", [{"col": "Depth", "lacks_words": "verdict"}]), "schema.DT_Transcendents",
                            "the test 'lacks_words' cannot be asked of a column of kind number")
        holds = checker.Checker(self.schema, self.tables, self.tags).holds
        lacks = lambda value: holds({"col": "C", "lacks_words": "rescue_words"}, {"C": value}, "r")  # noqa: E731
        self.assertTrue(lacks("a."))
        self.assertFalse(lacks("구원 a."))
        self.assertTrue(lacks(["a.", "b."]))
        self.assertFalse(lacks(["a.", "중보 b."]))
        self.assertTrue(lacks([]))
        self.assertFalse(lacks(None))

    def test_card_fleet_and_edict_words(self):
        """B4 [B-4c]: the card's fleet and edict lines and reasons (Petition.Resist*, Petition.Edict*) hold the rules of the
        fleet's and the edicts' words (Guard., Edict.): no holy word, no soul verb, no digit outside a slot; the verdict
        words of Petition. still hold; another Petition. key may carry a figure (the check's 6)."""
        check = lambda text: checker.check_string_table("ST_X.csv", text, self.schema)  # noqa: E731
        prefixes = self.schema["string_table"]["checked_prefixes"]
        for prefix in ("Petition.Resist", "Petition.Edict"):
            with self.subTest(prefix=prefix):
                self.assertEqual(prefixes[prefix], prefixes["Guard."])
                key = prefix + "Test"
                self.expect(check('Key,SourceString\n"%s","함대를 거둔다."\n' % key), "entry 1 (%s)" % key, "the word 거둔 is not used here")
                self.expect(check('Key,SourceString\n"%s","함대 5"\n' % key), "entry 1 (%s)" % key, "a digit outside a slot")
                holy = self.schema["words"]["holy_words"][0]
                self.expect(check('Key,SourceString\n"%s","%s a."\n' % (key, holy)), "entry 1 (%s)" % key, "the word %s is not used here" % holy)
                verdict = self.schema["words"]["verdict"][0]
                self.expect(check('Key,SourceString\n"%s","%s a."\n' % (key, verdict)), "entry 1 (%s)" % key, "the word %s is not used here" % verdict)
                self.assertEqual(check('Key,SourceString\n"%s","함대 {0} → {1} · {2}의 남은 시간 +{3}"\n' % key), [])
        self.assertEqual(check('Key,SourceString\n"Petition.Check","{0} 판정 {1}/6"\n'), [])

    def test_death_and_record_words(self):
        """B5 [B-5d1] (header 3.3, 3.5, 3.6): the words of a death - an afterlife, the book of life, a calling read as a
        death, and the remnant - hang on the new places alone: the keys of the death's pages (Epitaph., Reign., Digest.,
        Succession.) and the chronicle (the two books' chapter) with the interregnum's and the digest's words kept from the
        words of rest; the record lines, which name the system alone and carry no figure; the death and inauguration rows.
        Elsewhere the same letters may stand (matched letter for letter, a short word hides in others). The new test
        no_digits. The text audit's hangs (critic-3): no 'you' across a death on the death's pages and rows (12-da), no word
        of a return or a rebirth at the succession (3.6-2), no agent, sacrifice or cause of the death on its line (3.6-1), no
        name of the holy order on the death and inauguration rows, and the record's object a place or an event (11-ga)."""
        self.assertEqual(self.after(), [])
        lists = self.schema["words"]
        prefixes = self.schema["string_table"]["checked_prefixes"]
        check = lambda text: checker.check_string_table("ST_X.csv", text, self.schema)  # noqa: E731
        death_lists = ("afterlife_words", "book_of_life_words", "called_away_words")
        audit_lists = ("death_frame_words", "rebirth_words", "record_object_words")
        new_lists = death_lists + ("remnant_words",) + audit_lists
        # (1) The keys of the death's pages: the four lists of the narrator's screens and the three of a death, no digit
        # outside a slot.
        page_lists = ("verdict", "soul_verbs", "holy_words", "rescue_words") + death_lists
        for prefix in ("Epitaph.", "Reign.", "Digest.", "Succession."):
            spec = prefixes[prefix]
            self.assertTrue(set(page_lists) <= set(checker.forbidden_lists(spec)) and spec.get("no_digits") is True, spec)
            key = prefix + "Test"
            for listed in page_lists:
                for word in lists[listed]:
                    with self.subTest(prefix=prefix, word=word):
                        self.expect(check('Key,SourceString\n"%s","그 별 %s 아래."\n' % (key, word)), "entry 1 (%s)" % key, "the word %s is not used here" % word)
            with self.subTest(prefix=prefix, digit=True):
                self.expect(check('Key,SourceString\n"%s","살아온 주기 7"\n' % key), "entry 1 (%s)" % key, "a digit outside a slot")
                self.assertEqual(check('Key,SourceString\n"%s","살아온 주기 {0}"\n' % key), [])
        # (2) The chronicle holds the two books: the soul verbs, the rescue words and the three lists of a death join the
        # verdict there; a word put into one of the project's Chronicle. keys is found.
        for listed in ("soul_verbs", "rescue_words") + death_lists:
            word = lists[listed][0]
            with self.subTest(chronicle=listed):
                self.expect(check('Key,SourceString\n"Chronicle.A","%s a."\n' % word), "entry 1 (Chronicle.A)", "the word %s is not used here" % word)
        sources = checker.read_sources()
        sources["ST_CA.csv"] = sources["ST_CA.csv"].rstrip(b"\r\n") + '\n"Chronicle.Test","그 이름이 기록되었다."\n'.encode("utf-8")
        self.expect(checker.check_sources(sources), "ST_CA.csv", "Chronicle.Test", "the word 이름이 기록 is not used here")
        # (3) The interregnum's head and the digest are the world's news, with no place of rest for the calling: no word of
        # rest there; another chronicle key is not held to it.
        for key in ("Chronicle.NoVessel", "Digest.Test"):
            for word in lists["silence_words"]:
                with self.subTest(key=key, word=word):
                    self.expect(check('Key,SourceString\n"%s","%s a."\n' % (key, word)), "entry 1 (%s)" % key, "the word %s is not used here" % word)
        self.assertEqual(check('Key,SourceString\n"Chronicle.A","쉼 a."\n'), [])
        # (4) The new places alone: no other prefix names a list of a death, the remnant's word hangs on no key, and no
        # column of any table names one (the record lines and the two named rows carry them by row rules). B7x [B-7x3]
        # (header 3.2 - review T7, I8): the shape words of a people (Shape.) stand on a chronicle's chapter, so they take the
        # chronicle's lists and the remnant's word with them - the one key where it hangs (test_shape_words).
        places = ("Epitaph.", "Reign.", "Digest.", "Succession.", "Chronicle.", "Shape.")
        for prefix, spec in prefixes.items():
            with self.subTest(prefix=prefix):
                named = set(checker.forbidden_lists(spec)) & set(new_lists)
                # B6b [B-6b5]: the panel's line of the ruler's death at an extinction's expiry refuses the death's agent, sacrifice
                # or cause too (notes 1.6 rule 3) - a frame word on a key of its own.
                allowed = (set(death_lists) if prefix.startswith(places) else set()) | ({"rebirth_words"} if prefix.startswith("Succession.") else set()) | (
                    {"death_frame_words"} if prefix == "Panel.RulerLethal" else set()) | ({"remnant_words"} if prefix == "Shape." else set())
                self.assertEqual(named - allowed, set())
        # One column only, a column of its own (B6a [B-6a5], header 3.2): the fallen's awareness line refuses a return and a
        # calling; no other list of a death hangs there.
        # B6b [B-6b5] (decision 33, signal 11): the three text columns of a feat refuse the words of an afterlife - no people's
        # climb is told as an ascent of the dead ('승천') - and no other list of a death.
        self.assertEqual([(table, column) for table, column, spec in self.columns_of(lambda spec: set(checker.forbidden_lists(spec)) & set(new_lists))],
                         [("DT_Feats", "LogTemplates"), ("DT_Feats", "MythicNarrative"), ("DT_Feats", "Title"), ("DT_Transcendents", "AwarenessTemplates")])
        for column in ("LogTemplates", "MythicNarrative", "Title"):
            self.assertEqual(set(checker.forbidden_lists(self.schema["tables"]["DT_Feats"]["columns"][column])) & set(new_lists), {"afterlife_words"})
        awareness_spec = self.schema["tables"]["DT_Transcendents"]["columns"]["AwarenessTemplates"]
        self.assertEqual(set(checker.forbidden_lists(awareness_spec)) & set(new_lists), {"rebirth_words", "called_away_words"})
        self.assertEqual(check('Key,SourceString\n"Wander.A","%s"\n' % " ".join(lists[listed][0] for listed in new_lists)), [])

        # (4a) The audit's words stand in their lists (critic-3: a list walked by the cases above loses a word in silence): the
        # whereabouts and rebirth idioms (particle forms of '전생' only - '전생애' and '내세우다' are plain words), the calling
        # read as a death, the old titles of the books, the death's agent, sacrifice and cause, the return at the succession,
        # and the record's objects. '데려가' is a calling read as a death, not a frame word.
        audited = {
            "afterlife_words": ["세상을 떠나", "세상을 떠났", "세상을 떠", "별세", "천당", "낙원", "환생", "윤회", "부활", "영생", "전생에", "전생의", "전생을",
                                "전생이", "돌아가셨", "돌아가시", "편히 쉬", "편히 잠", "고이 잠", "쉬소서", "잠드소서", "다시 태어"],
            "called_away_words": ["데려가", "데려갔", "부름받", "부르심받", "부르심에", "부름 받", "부르심 받", "불려 올"],
            "book_of_life_words": ["기록된 이름", "이름을 기록", "이름을 적", "생명의 책"],
            "death_frame_words": ["불이 내려", "쏟아져", "내리쳐", "그들을 위해", "위하여", "했기에"],
            "rebirth_words": ["다시 ", "돌아오", "돌아왔", "태어나", "태어났", "거듭나", "머물렀", "임했", "깃들", "알아보았"],
            "record_object_words": ["날들", "이름", "사람", "죽은 자", "죽은 이", "남은 이"]}
        for listed, words in audited.items():
            with self.subTest(audited=listed):
                self.assertEqual([word for word in words if word not in lists[listed]], [])
        self.assertFalse({"전생", "내세"} & set(lists["afterlife_words"]))
        self.assertFalse({"데려가", "데려갔"} & set(lists["death_frame_words"]))
        # (4b) No 'you' across a death (12-da): the epitaph, the digest and the inauguration page refuse the reader's word; the
        # living vessel's reign end and the chronicle (Chronicle.Opened keeps it - 10-da) do not. The succession's words
        # carry no return and no rebirth (3.6-2); the chronicle may (Chronicle.NewRunTip).
        for prefix in ("Epitaph.", "Digest.", "Succession."):
            self.assertIn("reader_address", checker.forbidden_lists(prefixes[prefix]))
            for word in lists["reader_address"]:
                with self.subTest(prefix=prefix, reader=word):
                    self.expect(check('Key,SourceString\n"%sTest","%s a."\n' % (prefix, word)), "entry 1 (%sTest)" % prefix, "the word %s is not used here" % word)
        for key in ("Reign.Test", "Chronicle.Test"):
            self.assertEqual(check('Key,SourceString\n"%s","%s a."\n' % (key, lists["reader_address"][0])), [])
        self.assertIn("rebirth_words", checker.forbidden_lists(prefixes["Succession."]))
        for word in lists["rebirth_words"]:
            with self.subTest(succession=word):
                self.expect(check('Key,SourceString\n"Succession.Test","%s a."\n' % word), "entry 1 (Succession.Test)", "the word %s" % word)
        self.assertEqual(check('Key,SourceString\n"Chronicle.Test","%s a."\n' % lists["rebirth_words"][0]), [])
        # (5) A record line: the system's name alone, no figure, no word of a death and not the remnant's.
        def record(templates):
            def change(tables):
                self.add_record_row(tables)
                self.row(tables, "DT_LogTemplates", "log_record_test")["Templates"] = templates
            return change
        record_rule = "a record line is a subjectless passive about a place or an event"
        self.assertEqual(self.after(record(["{System}|hpp(이,가) 기록되었다.", "{System}의 무너짐이 기록되었다.", "기록되었다."])), [])
        for slot in ("Civ", "PopBand", "Ruler", "Player", "CivAdj"):
            with self.subTest(slot=slot):
                self.expect(self.after(record(["{System} {%s} a." % slot, "b.", "c."])), "DT_LogTemplates.log_record_test", record_rule, "slots_in")
        self.expect(self.after(record(["{System} a.", "b 3.", "c."])), "DT_LogTemplates.log_record_test", record_rule, '"no_digits": true')
        for listed in death_lists + ("remnant_words", "record_object_words"):
            for word in lists[listed]:
                with self.subTest(record=word):
                    self.expect(self.after(record(["{System} %s a." % word, "b.", "c."])), "DT_LogTemplates.log_record_test", record_rule, listed)
        # The remnant's word is the record lines' alone: a line of another category may carry it.
        self.assertEqual(self.after(self.set_cell("DT_LogTemplates", "log_fleet_worn", "Templates", ["{System} 남은 자 a.", "b.", "c."])), [])
        # The record's object is a place or an event: a line of another category may say days, names or people.
        self.assertEqual(self.after(self.set_cell("DT_LogTemplates", "log_fleet_worn", "Templates", ["{System} 사람 a.", "{System} 날들 b.", "c."])), [])
        # (6) The death and inauguration rows: the calling's lines, their slots, no word of a death, no 'you', no name of the
        # holy order; the death's line no agent, sacrifice or cause, the inauguration's no return or rebirth.
        row_lists = {"log_death_hazard": death_lists + ("reader_address", "realm_names", "death_frame_words"),
                     "log_death_extinction": death_lists + ("reader_address", "realm_names", "death_frame_words"),
                     "log_inauguration": death_lists + ("reader_address", "realm_names", "rebirth_words"),
                     "log_inauguration_unnamed": death_lists + ("reader_address", "realm_names", "rebirth_words")}
        # B6b [B-6b2b]: the extinction's death line takes the death line's rule whole (header 3.3 rule 8).
        for row_name, slots, refused in (("log_death_hazard", "{Player} {Hazard}", ("System", "Civ")),
                                         ("log_death_extinction", "{Player} {Hazard}", ("System", "Civ")),
                                         ("log_inauguration", "{Player} {Civ} {System}", ("CivAdj", "Hazard")),
                                         ("log_inauguration_unnamed", "{Player} {Civ}", ("System", "Hazard"))):
            line = lambda row_name=row_name, **changes: (lambda tables: self.row(tables, "DT_LogTemplates", row_name).update(**changes))  # noqa: E731
            with self.subTest(row=row_name):
                self.assertEqual(self.after(line(Templates=["%s a." % slots, "b.", "c."])), [])
                self.expect(self.after(line(Category="Log.Civ")), "DT_LogTemplates.%s" % row_name, '"is": "Log.Calling"')
            for slot in refused:
                with self.subTest(row=row_name, slot=slot):
                    self.expect(self.after(line(Templates=["%s {%s} a." % (slots, slot), "b.", "c."])), "DT_LogTemplates.%s" % row_name, "slots_in")
            for listed in row_lists[row_name]:
                for word in lists[listed]:
                    with self.subTest(row=row_name, word=word):
                        self.expect(self.after(line(Templates=["%s %s a." % (slots, word), "b.", "c."])), "DT_LogTemplates.%s" % row_name, '"%s"' % listed)
        # The death's frame words are the death row's alone: the inauguration's line may say them, and a word of return may
        # stand on the death's line.
        self.assertEqual(self.after(self.set_cell("DT_LogTemplates", "log_inauguration", "Templates", ["{Player} {Civ} {System} 위하여 a.", "b.", "c."])), [])
        self.assertEqual(self.after(self.set_cell("DT_LogTemplates", "log_death_hazard", "Templates", ["{Player} {Hazard} 머물렀 a.", "b.", "c."])), [])
        # The texts the audit settled pass their rows (critic-3): the death line bound to its hazard, reader-4's record lines,
        # the inauguration that names no system - and no line without {System} points back to 'that star' (korean-1).
        project = {row["Name"]: row["Templates"] for row in checker.load_project()[1]["DT_LogTemplates"]}
        for row_name in ("log_death_hazard", "log_inauguration", "log_inauguration_unnamed", "log_record_destroyed", "log_record_annexed"):
            with self.subTest(settled=row_name):
                self.assertEqual(self.after(self.set_cell("DT_LogTemplates", row_name, "Templates", project[row_name])), [])
        for row_name in ("log_death_hazard", "log_inauguration_unnamed"):
            self.assertFalse(any("\uadf8 \ubcc4" in text for text in project[row_name]), row_name)
        # arch_default is the name bank of last resort (paths-10): empty there, the death and inauguration lines lose their
        # subject; another people's bank may stay empty.
        self.expect(self.after(self.set_cell("DT_Civs", "arch_default", "PersonNameBank", [])), "DT_Civs.arch_default", "name bank of last resort", '"min_len": 10')
        self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_seeker", "PersonNameBank", [])), [])
        # (7) The new test: true holds for texts without a digit, false for a text with one; it fits a text column only.
        holds = checker.Checker(self.schema, self.tables, self.tags).holds
        bare = lambda value, flag=True: holds({"col": "C", "no_digits": flag}, {"C": value}, "r")  # noqa: E731
        self.assertTrue(bare("a."))
        self.assertFalse(bare("a 3."))
        self.assertTrue(bare(["a.", "{System} b."]))
        self.assertFalse(bare(["a.", "b \uff13."]))
        self.assertTrue(bare([]))
        self.assertFalse(bare(None))
        self.assertFalse(bare(["a.", 5]))
        self.assertTrue(bare("a 3.", False))
        self.assertFalse(bare("a.", False))
        rule = ["tables", "DT_LogTemplates", "rules", 0]
        self.schema_problem(self.put(rule, "when", [{"col": "MinTierToReveal", "no_digits": True}]), "schema.DT_LogTemplates",
                            "the test 'no_digits' cannot be asked of a column of kind number")
        self.schema_problem(self.put(rule, "when", [{"col": "Templates", "no_digits": "yes"}]), "schema.DT_LogTemplates", "the test 'no_digits' needs true or false")

    def test_awareness_and_end_words(self):
        """B6a [B-6a5] (header 3.2, 3.4, 3.6): the fallen's awareness line - the first person's shared word lists with the
        words of a return, a calling and a recognition, the silent one's four words, no slot, no digit, and the one place
        where the fallen's mouth may say 'you'; the passive of the holy order and the idiom of an end that comes in every
        chronicle key; the end's reason lines (Chronicle.Note.) with the holy words, the names above, the words of rest and
        the revelation's words; the panel's two new lines with the verdict, the holy words and the words of rest."""
        self.assertEqual(self.after(), [])
        lists = self.schema["words"]
        columns = self.schema["tables"]["DT_Transcendents"]["columns"]
        # (1) The lists stand as the header wrote them ('다시 ' and '알아보았' of rebirth_words let '다시는' and '알아본다'
        # through; '같은' was in no list).
        self.assertEqual(lists["awareness_words"], ["같은", "똑같", "다시", "알아보", "알아본", "알아봤", "알아볼"])
        # B6b [B-6b5] extends the passive with the end-time idiom ('끝날' is left out: '끝날 무렵' stands in a death line).
        self.assertEqual(lists["holy_passive"], ["맡겨졌", "맡겨진", "허락되", "허락된", "때가 찼", "때가 차", "때가 왔", "끝이 왔", "마지막 날", "종말", "때가 되", "끝이 오", "때가 이르"])
        # (2) The column: the first person's one form of words (the same as the claim line's and the lure's), then three more.
        awareness = checker.forbidden_lists(columns["AwarenessTemplates"])
        self.assertEqual(awareness[:6], columns["ClaimTemplates"]["forbidden"])
        self.assertEqual(awareness[6:], ["rebirth_words", "called_away_words", "awareness_words"])
        self.assertEqual(columns["VoiceTemplates"], {"kind": "texts"})
        line = lambda lines: self.set_cell("DT_Transcendents", "trans_silence", "AwarenessTemplates", lines)  # noqa: E731
        # The project's lines pass in the fixture's silent row; they speak to the vessel (the one second person of the mouth),
        # and the claim line still may not.
        project = json.loads(checker.read_sources()["DT_Transcendents.json"].decode("utf-8"))
        told = next(row for row in project if row["Name"] == "trans_silence")["AwarenessTemplates"]
        self.assertTrue(told)
        self.assertEqual(self.after(line(told)), [])
        self.assertEqual(self.after(line(["너는 a.", "네가 b.", "당신 c."])), [])
        self.expect(self.after(self.set_cell("DT_Transcendents", "trans_silence", "ClaimTemplates", ["{System} 너는 a.", "b.", "c."])),
                    "DT_Transcendents.trans_silence", "a fallen one's claim line speaks to no one")
        # (3) One breaking case per new word: a sameness, a recognition, an again, a return, a calling read as a death.
        for text, word in (("같은 부름의 냄새가 난다.", "같은"), ("나는 너를 알아본다.", "알아본"), ("다시는 닿지 못한다.", "다시"),
                           ("너는 돌아왔다.", "돌아왔"), ("너는 부름을 받았다.", "부름을 받"), ("똑같은 손이다.", "똑같")):
            with self.subTest(text=text):
                self.expect(self.after(line([text, "b.", "c."])), "trans_silence.AwarenessTemplates", "the word %s is not used here" % word)
        # The shared lists hold there too: a name above, a holy word, a verdict word.
        for word in ("보좌", "성령", "천국"):
            with self.subTest(shared=word):
                self.expect(self.after(line(["너는 %s a." % word])), "trans_silence.AwarenessTemplates", "the word %s is not used here" % word)
        self.expect(self.after(line(["{Player} a."])), "trans_silence.AwarenessTemplates", "slot {Player}")
        self.expect(self.after(line(["너는 3 a."])), "trans_silence.AwarenessTemplates", "digit")
        # (4) The silent one's four words, by the doctrine's rule; a fallen row of another doctrine may use them.
        silence = "the silent one does not take the words of the revelation it counterfeits"
        for word in lists["silence_words"]:
            with self.subTest(silence=word):
                self.expect(self.after(line(["네가 다스리는 곳의 %s a." % word])), "DT_Transcendents.trans_silence", silence, "AwarenessTemplates")
        other = lambda tables: tables["DT_Transcendents"].append(transcendent_row("trans_zeal", Doctrine="Doctrine.Zeal", AwarenessTemplates=["너는 고요 a."]))  # noqa: E731
        self.assertEqual(self.after(other), [])
        # (5) Every chronicle key refuses the holy order's passive and the end that comes; a key under no chronicle prefix
        # does not.
        check = lambda text: checker.check_string_table("ST_X.csv", text, self.schema)  # noqa: E731
        prefixes = self.schema["string_table"]["checked_prefixes"]
        self.assertIn("holy_passive", checker.forbidden_lists(prefixes["Chronicle."]))
        for word in lists["holy_passive"]:
            for key in ("Chronicle.Rule", "Chronicle.Note.Clock"):
                with self.subTest(key=key, passive=word):
                    self.expect(check('Key,SourceString\n"%s","%s 이 판은 여기까지다."\n' % (key, word)), "entry 1 (%s)" % key, "the word %s is not used here" % word)
        self.assertEqual(check('Key,SourceString\n"Wander.A","%s a."\n' % lists["holy_passive"][0]), [])
        sources = checker.read_sources()
        sources["ST_CA.csv"] = sources["ST_CA.csv"].rstrip(b"\r\n") + '\n"Chronicle.Note.Test","때가 찼다 — 이 판은 여기까지다."\n'.encode("utf-8")
        self.expect(checker.check_sources(sources), "ST_CA.csv", "Chronicle.Note.Test", "the word 때가 찼 is not used here")
        # (6) The end's reason lines: the chronicle's lists, the holy words, the names above, the words of rest and the
        # revelation's words; another chronicle key is not held to the four.
        note = prefixes["Chronicle.Note."]
        self.assertTrue(set(checker.forbidden_lists(prefixes["Chronicle."])) <= set(checker.forbidden_lists(note)) and note.get("no_digits") is True, note)
        for listed in ("holy_words", "realm_names", "silence_words", "revelation_mimicry"):
            self.assertIn(listed, checker.forbidden_lists(note))
            for word in lists[listed]:
                with self.subTest(note=word):
                    self.expect(check('Key,SourceString\n"Chronicle.Note.Test","%s a."\n' % word), "entry 1 (Chronicle.Note.Test)", "the word %s is not used here" % word)
        self.assertEqual(check('Key,SourceString\n"Chronicle.Horizon","고요 a."\n'), [])
        # (7) The panel's two new lines: no verdict, no holy word, no word of rest, a figure only in a slot; another panel
        # key is a UI label and is not looked at.
        for key in ("Panel.ClaimMayFollow", "Panel.RebellionWatch"):
            spec = prefixes[key]
            self.assertEqual(checker.forbidden_lists(spec), ["verdict", "holy_words", "silence_words"])
            for listed in ("verdict", "holy_words", "silence_words"):
                for word in lists[listed]:
                    with self.subTest(key=key, word=word):
                        self.expect(check('Key,SourceString\n"%s","%s a."\n' % (key, word)), "entry 1 (%s)" % key, "the word %s is not used here" % word)
            with self.subTest(key=key, digit=True):
                self.expect(check('Key,SourceString\n"%s","안정도 3 아래"\n' % key), "entry 1 (%s)" % key, "a digit outside a slot")
                self.assertEqual(check('Key,SourceString\n"%s","안정도 {0} 아래로 {1}주기째"\n' % key), [])
        self.assertEqual(check('Key,SourceString\n"Panel.Test","%s 3"\n' % lists["verdict"][0]), [])

    def test_upper_rung_text_words(self):
        """B6b [B-6b5] (header 3.3, notes 1.6, decision 33): the words of the upper rungs. A rung grasped, reached or raced
        for, a title of election or of a saviour, the binding and its binder verbs, a triumph or a deed made the price of a
        rung, a ruler who stayed with the dying, a people that rescues itself, dawn, light and reign, the desolation's images
        and the words of the binding's work's name, matched letter for letter: two lists on every narrated column, the feat
        columns' wider set, the rules of single rows, and the new string prefixes and keys of the B6b screens. Elsewhere the
        same letters may stand."""
        self.assertEqual(checker.check_project(), [])
        self.assertEqual(self.after(), [])
        lists = self.schema["words"]
        tables = self.schema["tables"]
        prefixes = self.schema["string_table"]["checked_prefixes"]
        check = lambda text: checker.check_string_table("ST_X.csv", text, self.schema)  # noqa: E731

        def why_of(table, when):
            return next(rule["why"] for rule in tables[table]["rules"] if rule.get("when") == when and any("lacks_words" in test for test in rule["then"]))

        def broken(problems, where, why, column, listed):
            self.expect(problems, where, why, json.dumps({"col": column, "lacks_words": listed}))

        # (1) The lists and the places stand as the audit wrote them (a word or a list taken out goes in silence otherwise).
        self.assertEqual(lists["rung_grasp_words"], ["영겁", "왕 노릇", "질서의 화신", "초월자가 되", "신이 되", "일체화", "질서", "영원", "칸에 올", "올라섰", "올라선", "더 올랐", "경주"])
        self.assertEqual(lists["title_words"], ["선택받은", "선택 받은", "택함 받은", "택함을 받은", "택하신", "빛의 아들", "기름 받은", "기름 부음", "기름 부은", "구세주", "메시야", "구원자", "메시아",
                                                "선택된 백성", "택한 백성", "택하심", "면류관", "왕관", "관을 쓰"])
        self.assertEqual(lists["binding_words"], ["묶", "봉인", "결박", "가두", "갇혔", "갇힌", "사로잡", "막는", "막았", "막아", "눌렀", "그늘", "그림자"])
        self.assertEqual(lists["triumph_words"], ["이겼", "이긴", "물리쳤", "무찔", "쓰러뜨", "마침내", "드디어", "걷혔", "걷히", "사슬", "열쇠", "가두", "갇혔", "그래서", "그 일로", "덕분",
                                                  "그늘 아래", "묶었", "묶어", "묶는", "묶을", "기에", "끝에", "덕에", "대가로", "보상", "쫓", "물러가", "물러갔", "물러났", "사라졌", "흩어",
                                                  "꿇", "비웃", "조롱"])
        self.assertEqual(lists["stay_words"], ["곁에 남", "끝까지", "함께 쓰러", "함께 스러", "함께 죽", "떠나지 않", "대신", "함께", "위해", "희생", "목숨", "바쳤", "바친", "마지막까지"])
        self.assertEqual(lists["self_rescue_words"], ["구해", "구했", "구한", "살려", "살렸", "건져", "건졌", "건진", "지켜 냈", "거듭나", "새로 태어", "다시 태어", "씻겨", "새 땅", "홍수",
                                                      "물이 불", "물이 걷", "물이 빠", "무지개", "비둘기", "감람"])
        self.assertEqual(lists["dawn_words"], ["새벽", "동틀", "동이 트", "해가 뜨", "돋는", "샛별", "새 해", "빛", "밝아", "걷혔", "다스림", "왕좌", "임금", "떠오", "떠올", "해가 떠",
                                               "광명", "여명", "서광", "아침", "햇살", "치세", "재위", "통치", "왕국", "다스리"])
        self.assertEqual(lists["desolation_words"], ["꺼지", "꺼졌", "꺼진", "꺼뜨", "껐", "꺼져", "꺼질", "식어", "식었", "노랫소리", "노래가 그", "맷돌", "등불", "심지", "촛불", "등잔",
                                                     "불씨", "문이 닫", "닫힌 문", "신랑", "신부", "홍수", "물이 불", "물이 걷", "물이 빠", "무지개", "비둘기", "감람", "진노", "심판",
                                                     "형벌", "죗값", "벌로", "벌을", "교만", "재앙이 내", "그늘", "그림자", "어둠", "서리", "겨울", "견딘", "견딤", "견뎠", "버틴"])
        self.assertEqual(lists["binding_name_words"], ["탑", "둘레", "울타리", "산울", "고리", "관측소", "공명", "감응", "교감"])
        self.assertEqual(lists["holy_passive"][8:], ["마지막 날", "종말", "때가 되", "끝이 오", "때가 이르"])
        new_lists = ("rung_grasp_words", "title_words", "binding_words", "triumph_words", "stay_words", "self_rescue_words", "dawn_words", "desolation_words", "binding_name_words")
        start = ["verdict", "holy_words", "soul_verbs", "realm_names", "rescue_words", "rung_grasp_words", "title_words", "binding_words", "triumph_words"]
        extinction = ["verdict", "holy_words", "silence_words", "rescue_words", "stay_words", "rung_grasp_words", "title_words"]
        keyed = {
            "Rung.": ["verdict", "holy_words", "soul_verbs", "realm_names", "rescue_words", "holy_passive", "rung_grasp_words", "title_words", "binding_words"],
            "Sight.": ["verdict", "holy_words", "realm_names", "rescue_words", "rung_grasp_words", "title_words"],
            "Start.BindHint": start, "Start.Block.NoFallen": start, "Start.Suffix.NoFallen": start,
            "Panel.ExtinctionFollows": extinction, "Panel.RulerLethal": extinction + ["death_frame_words"],
            "Settle.Block.RealmCapLast": ["verdict", "holy_words", "rescue_words", "rung_grasp_words", "title_words"],
            "Diplomacy.TreatyRungTip": ["verdict", "holy_words", "soul_verbs", "binding_words"],
        }
        for key, forbidden in keyed.items():
            with self.subTest(pin=key):
                self.assertEqual(prefixes[key], {"forbidden": forbidden, "no_digits": True})
        widened = {"Chronicle.": ["rung_grasp_words", "title_words", "binding_words"], "Chronicle.Note.": ["rung_grasp_words", "title_words", "binding_words"],
                   "Epitaph.": ["rung_grasp_words", "title_words"], "Digest.": ["rung_grasp_words", "title_words"]}
        for key, added in widened.items():
            with self.subTest(pin=key):
                self.assertEqual(checker.forbidden_lists(prefixes[key])[-len(added):], added)
        feat_lists = ["verdict", "holy_words", "realm_names", "rescue_words", "afterlife_words", "holy_and_myth", "rung_grasp_words", "title_words", "binding_words", "self_rescue_words", "triumph_words"]
        places = [("DT_Feats", "feat_test_climb", column, feat_lists) for column in ("Title", "MythicNarrative", "LogTemplates")] + [
            ("DT_Megaprojects", "project_test_build", "CompleteTemplates", ["verdict", "rescue_words", "rung_grasp_words", "title_words"]),
            ("DT_Events", "event_test_harvest", "LogTemplates", ["verdict", "holy_words", "soul_verbs", "rescue_words", "rung_grasp_words", "title_words", "ark_words"]),
            ("DT_LogTemplates", "log_far_event", "Templates", ["verdict", "soul_verbs", "holy_words", "rescue_words", "rung_grasp_words", "title_words", "ark_words"])] + [
            ("DT_Hazards", "crisis_test_storm", column, ["verdict", "rescue_words", "rung_grasp_words", "title_words", "ark_words"]) for column in ("OutbreakTemplates", "ExpireTemplates", "ResolveTemplates")]
        for table, _, column, forbidden in places:
            with self.subTest(pin=(table, column)):
                self.assertEqual(checker.forbidden_lists(tables[table]["columns"][column]), forbidden)
        # Each of the nine lists hangs somewhere: on a column, a string key or a row rule.
        hung = {listed for spec in prefixes.values() for listed in checker.forbidden_lists(spec)}
        hung |= {listed for _, _, spec in self.columns_of(lambda spec: True) for listed in checker.forbidden_lists(spec)}
        hung |= {test["lacks_words"] for spec in tables.values() for rule in spec.get("rules", []) for test in rule["then"] if "lacks_words" in test}
        self.assertEqual(set(new_lists) - hung, set())
        # (2) Every word of the new lists on every place they hang: a column, a prefix or a key of the string table.
        for table, name, column, forbidden in places:
            words = [word for listed in forbidden if listed in new_lists or table == "DT_Feats" for word in lists[listed]]
            kind = tables[table]["columns"][column]["kind"]
            value = " ".join(words) if kind == "text" else [word + " a." for word in words] + ["b.", "c."]
            problems = self.after(self.set_cell(table, name, column, value))
            for word in words:
                with self.subTest(place=(table, column), word=word):
                    self.expect(problems, "%s.%s" % (name, column), "the word %s is not used here" % word)
        for key, forbidden in list(keyed.items()) + [(key, added) for key, added in widened.items()]:
            words = [word for listed in forbidden for word in lists[listed]]
            test_key = key + "Test" if key.endswith(".") else key
            problems = check('Key,SourceString\n"%s","%s"\n' % (test_key, " ".join(words)))
            for word in words:
                with self.subTest(key=key, word=word):
                    self.expect(problems, "entry 1 (%s)" % test_key, "the word %s is not used here" % word)
            # (4) A figure only in a slot.
            with self.subTest(key=key, digit=True):
                self.expect(check('Key,SourceString\n"%s","칸 3"\n' % test_key), "entry 1 (%s)" % test_key, "a digit outside a slot")
                self.assertEqual(check('Key,SourceString\n"%s","{Transcendent}|hpp(이,가) {0} {1}"\n' % test_key), [])
        # The same through the project's sources: a key put under each new prefix, and each keyed text of the project
        # rewritten, is found.
        sources = checker.read_sources()
        rows = list(csv.reader(io.StringIO(checker.text_of(sources["ST_CA.csv"]))))
        breaking = {key: lists[forbidden[-1]][0] for key, forbidden in keyed.items()}
        for row in rows[1:]:
            if row and row[0] in breaking:
                row[1] = breaking[row[0]] + " a."
        rows += [[prefix + "Test", breaking[prefix] + " a."] for prefix in ("Rung.", "Sight.")]
        rows += [[prefix + "Test", lists[added[-1]][0] + " a."] for prefix, added in widened.items()]
        sources["ST_CA.csv"] = "".join(",".join('"%s"' % cell.replace('"', '""') for cell in row) + "\n" for row in rows if row).encode("utf-8")
        problems = checker.check_sources(sources)
        for key, word in list(breaking.items()) + [(prefix, lists[added[-1]][0]) for prefix, added in widened.items()]:
            with self.subTest(sources=key):
                self.expect(problems, "ST_CA.csv", key + "Test" if key.endswith(".") else key, "the word %s is not used here" % word)
        # (3) The named cases, each on its place.
        title = lambda value: self.set_cell("DT_Feats", "feat_test_climb", "Title", value)  # noqa: E731
        narrative = lambda value: self.set_cell("DT_Feats", "feat_test_climb", "MythicNarrative", value)  # noqa: E731
        for value, words in (("영겁의 별", ["영겁"]), ("영원한 질서", ["영원", "질서"])):
            with self.subTest(title=value):
                for word in words:
                    self.expect(self.after(title(value)), "feat_test_climb.Title", "the word %s is not used here" % word)
        for value in ("선택받은 백성", "구세주", "메시야", "구원자", "메시아"):
            with self.subTest(narrative=value):
                self.expect(self.after(narrative("그들은 %s의 일을 했다." % value)), "feat_test_climb.MythicNarrative", "the word %s is not used here" % ("선택받은" if value == "선택받은 백성" else value))
        self.expect(check('Key,SourceString\n"Rung.Test","하나님의 칸"\n'), "entry 1 (Rung.Test)", "the word 하나님 is not used here")
        self.expect(check('Key,SourceString\n"Rung.Test","그 일로 그늘이 묶였다"\n'), "entry 1 (Rung.Test)", "the word 묶 is not used here")
        self.expect(check('Key,SourceString\n"Chronicle.Test","종말 — 이 판은 여기까지다."\n'), "entry 1 (Chronicle.Test)", "the word 종말 is not used here")
        self.expect(check('Key,SourceString\n"Digest.Test","{Civ}|hpp(이,가) 더 높은 칸에 올라섰다."\n'), "entry 1 (Digest.Test)", "the word 칸에 올 is not used here")
        self.expect(check('Key,SourceString\n"Digest.Test","{Civ}|hpp(이,가) 더 높은 칸에 올라섰다."\n'), "entry 1 (Digest.Test)", "the word 올라섰 is not used here")
        fifth = self.set_cell("DT_Feats", "feat_test_climb", "RequiredTier", "Tier4_Transcendent")
        fifth_target = self.set_cell("DT_Feats", "feat_test_climb", "TargetTier", "Tier5_EternalOrder")
        fifth_why = why_of("DT_Feats", [{"col": "TargetTier", "is": "Tier5_EternalOrder"}])
        fifth_lines = lambda line: lambda tables: (fifth(tables), fifth_target(tables), self.set_cell("DT_Feats", "feat_test_climb", "LogTemplates", [line, "{Civ} b.", "c."])(tables))  # noqa: E731
        self.assertEqual(self.after(fifth_lines("{Civ}|hpp(이,가) 다섯째 칸에 서 있게 되었다.")), [])
        broken(self.after(fifth_lines("{Civ}|hpp(이,가) 다섯째 칸에 섰다. 때가 찼다.")), "DT_Feats.feat_test_climb", fifth_why, "LogTemplates", "holy_passive")
        broken(self.after(fifth_lines("{Civ}|hpp(이,가) 다섯째 칸에 섰다. 새벽이 왔다.")), "DT_Feats.feat_test_climb", fifth_why, "LogTemplates", "dawn_words")
        for line, word in (("{Civ}|hpp(이,가) 칙령을 지켰기에 다섯째 칸에 섰다.", "기에"), ("{Civ}|hpp(이,가) 다섯째 칸에 올랐다.", "칸에 올"),
                           ("{HomeSystem}의 하늘에서 그늘이 묶였다.", "묶"), ("{Civ}|hpp(이,가) 그늘을 묶었다.", "묶었")):
            with self.subTest(fifth=line):
                self.expect(self.after(fifth_lines(line)), "feat_test_climb.LogTemplates", "the word %s is not used here" % word)
        fourth = lambda tables: (self.set_cell("DT_Feats", "feat_test_climb", "RequiredTier", "Tier3_Causal")(tables),  # noqa: E731
                                 self.set_cell("DT_Feats", "feat_test_climb", "TargetTier", "Tier4_Transcendent")(tables),
                                 self.set_cell("DT_Feats", "feat_test_climb", "LogTemplates", ["{Civ}|hpp(은,는) 방주로 백성을 건져 냈다.", "{Civ} b.", "c."])(tables))
        self.expect(self.after(fourth), "feat_test_climb.LogTemplates", "the word 건져 is not used here")
        # The binding's work: its name and its completion line.
        binding_why = why_of("DT_Megaprojects", [{"row": "project_binding"}])
        binding = lambda column, value: self.set_cell("DT_Megaprojects", "project_binding", column, value)  # noqa: E731
        for name in ("둘레 탑", "공명 기관", "감응 기관", "교감 기관"):
            with self.subTest(binding_name=name):
                broken(self.after(binding("DisplayName", name)), "DT_Megaprojects.project_binding", binding_why, "DisplayName", "binding_name_words")
        self.assertEqual(self.after(binding("DisplayName", "항성 기관")), [])
        complete = binding("CompleteTemplates", ["{Civ}|hpp(이,가) 그늘을 묶었다.", "{System} b.", "{Project} c."])
        for listed in ("binding_words", "triumph_words"):
            broken(self.after(complete), "DT_Megaprojects.project_binding", binding_why, "CompleteTemplates", listed)
        broken(self.after(binding("CompleteTemplates", ["{System}에 빛이 비쳤다.", "{System} b.", "{Project} c."])), "DT_Megaprojects.project_binding", binding_why, "CompleteTemplates", "dawn_words")
        # The line of the binding and the far ascent.
        bound_why = why_of("DT_LogTemplates", [{"row": "log_fallen_bound"}])
        bound = lambda first: self.set_cell("DT_LogTemplates", "log_fallen_bound", "Templates", [first, "{System} b.", "{System} c."])  # noqa: E731
        broken(self.after(bound("마침내 {System}의 그늘이 걷혔다.")), "DT_LogTemplates.log_fallen_bound", bound_why, "Templates", "triumph_words")
        broken(self.after(bound("마침내 {System}의 그늘이 걷혔다.")), "DT_LogTemplates.log_fallen_bound", bound_why, "Templates", "dawn_words")
        broken(self.after(bound("{System}에서 그늘을 묶었다.")), "DT_LogTemplates.log_fallen_bound", bound_why, "Templates", "triumph_words")
        far_why = why_of("DT_LogTemplates", [{"row": "log_far_ascent"}])
        far = lambda first: self.set_cell("DT_LogTemplates", "log_far_ascent", "Templates", [first, "b.", "c."])  # noqa: E731
        broken(self.after(far("한 백성이 더 높은 칸에 서는 것이 허락되었다는 소식이 닿았다.")), "DT_LogTemplates.log_far_ascent", far_why, "Templates", "holy_passive")
        broken(self.after(far("한 백성의 그늘이 넓어졌다는 소식이 닿았다.")), "DT_LogTemplates.log_far_ascent", far_why, "Templates", "binding_words")
        # The extinction: its row's three columns, its record line and its death line.
        extinct_why = why_of("DT_Hazards", [{"col": "HazardTag", "is": "Hazard.Extinction"}])
        extinct = lambda column, first: lambda tables: (  # noqa: E731
            self.row(tables, "DT_Hazards", "crisis_test_storm").update(HazardTag="Hazard.Extinction", OnExpireEffects=[], RulerResolutions=["project_test_build"]),
            self.set_cell("DT_Hazards", "crisis_test_storm", column, [first, "{System} b.", "{System} c."])(tables))
        self.assertEqual(self.after(extinct("ExpireTemplates", "{Civ}|hpp(이,가) 사라졌다.")), [])
        broken(self.after(extinct("ExpireTemplates", "{Civ}의 다스리는 이는 끝까지 곁에 남았다.")), "DT_Hazards.crisis_test_storm", extinct_why, "ExpireTemplates", "stay_words")
        broken(self.after(extinct("ExpireTemplates", "{System}의 불빛이 꺼졌다.")), "DT_Hazards.crisis_test_storm", extinct_why, "ExpireTemplates", "desolation_words")
        broken(self.after(extinct("OutbreakTemplates", "{System}에 그늘이 내렸다.")), "DT_Hazards.crisis_test_storm", extinct_why, "OutbreakTemplates", "desolation_words")
        broken(self.after(extinct("OutbreakTemplates", "{System}에 불이 내려 왔다.")), "DT_Hazards.crisis_test_storm", extinct_why, "OutbreakTemplates", "death_frame_words")
        for listed in ("self_rescue_words", "desolation_words"):
            broken(self.after(extinct("ResolveTemplates", "{System}의 홍수가 그쳤다.")), "DT_Hazards.crisis_test_storm", extinct_why, "ResolveTemplates", listed)
        broken(self.after(extinct("ResolveTemplates", "{Civ}|hpp(은,는) 방주로 백성을 건져 냈다.")), "DT_Hazards.crisis_test_storm", extinct_why, "ResolveTemplates", "self_rescue_words")
        broken(self.after(extinct("ResolveTemplates", "{Civ}의 사람들은 함께 남았다.")), "DT_Hazards.crisis_test_storm", extinct_why, "ResolveTemplates", "stay_words")
        broken(self.after(extinct("ResolveTemplates", "{Civ}의 사람들이 그들을 위해 버텼다.")), "DT_Hazards.crisis_test_storm", extinct_why, "ResolveTemplates", "death_frame_words")
        record_why = why_of("DT_LogTemplates", [{"row": "log_record_extinct"}])
        record = lambda first: self.set_cell("DT_LogTemplates", "log_record_extinct", "Templates", [first, "{System} b.", "{System} c."])  # noqa: E731
        broken(self.after(record("{System}의 등불이 꺼진 일이 기록되었다.")), "DT_LogTemplates.log_record_extinct", record_why, "Templates", "desolation_words")
        broken(self.after(record("{System}의 집들이 끝까지 빈 일이 기록되었다.")), "DT_LogTemplates.log_record_extinct", record_why, "Templates", "stay_words")
        broken(self.after(record("{System}의 집들이 그들을 위하여 빈 일이 기록되었다.")), "DT_LogTemplates.log_record_extinct", record_why, "Templates", "death_frame_words")
        death_why = why_of("DT_LogTemplates", [{"row": "log_death_extinction"}])
        death = lambda first: self.set_cell("DT_LogTemplates", "log_death_extinction", "Templates", [first, "{Player} {Hazard} b.", "{Player} {Hazard} c."])  # noqa: E731
        broken(self.after(death("{Player}|hpp(은,는) {Hazard}|hpp(이,가) 끝날 때까지 끝까지 곁에 남았다.")), "DT_LogTemplates.log_death_extinction", death_why, "Templates", "stay_words")
        broken(self.after(death("{Player}|hpp(은,는) {Hazard} 속에서 백성을 위해 쓰러졌다.")), "DT_LogTemplates.log_death_extinction", death_why, "Templates", "stay_words")
        broken(self.after(death("{Hazard}|hpp(이,가) 끝나던 날, {Player}|hpp(은,는) 쓰러졌다. 그 땅의 노랫소리가 그쳤다.")), "DT_LogTemplates.log_death_extinction", death_why, "Templates", "desolation_words")
        # The era named after endured pressure.
        era_why = why_of("DT_Events", [{"col": "Conditions", "has_type": "CivPressureEnduredTicksReached"}])
        era = lambda first: self.set_cell("DT_Events", "event_test_harvest", "LogTemplates", [first, "{Civ} b.", "{Civ} c."])  # noqa: E731
        # The row has the era's shape ([B-6b8], decision 22): the ruled people and the Borders reverence.
        endured = lambda tables: self.row(tables, "DT_Events", "event_test_harvest").update(  # noqa: E731
            Trigger="Standing", BaseWeight=0, Conditions=[cond("SystemOwnedBy", key="Ruled"), cond("CivPressureEnduredTicksReached", key="600")],
            Effects=[eff("ChangeReverence", key="Borders", value=0.2)])
        both = lambda first: lambda tables: (endured(tables), era(first)(tables))  # noqa: E731
        self.assertEqual(self.after(both("{Civ}의 백성이 이 시대를 '물레의 시대'라 부르기 시작했다.")), [])
        for first, listed in (("{Civ}의 백성이 이 시대를 '새벽의 시대'라 부르기 시작했다.", "dawn_words"), ("{Civ}의 백성이 이 시대를 '여명의 시대'라 부르기 시작했다.", "dawn_words"),
                              ("{Civ}의 백성이 이 시대를 '꺼져 가던 등불의 시대'라 부르기 시작했다.", "desolation_words"), ("{Civ}의 백성이 이 시대를 '고요의 시대'라 부르기 시작했다.", "silence_words")):
            with self.subTest(era=first):
                broken(self.after(both(first)), "DT_Events.event_test_harvest", era_why, "LogTemplates", listed)
        self.assertEqual(self.after(era("{Civ}의 백성이 이 시대를 '새벽의 시대'라 부르기 시작했다.")), [])
        # (5) The same letters stand elsewhere: the binding's own story line, a storm's lifted fog, a project's 'to the end'
        # and its light, a held siege, a plea that asks.
        project_st = {row[0]: row[1] for row in csv.reader(io.StringIO(checker.text_of(checker.read_sources()["ST_CA.csv"]))) if len(row) > 1}
        self.assertIn("묶이", project_st["Sight.Bound"])
        self.assertEqual(check('Key,SourceString\n"Sight.Test","{Transcendent}|hpp(이,가) 묶이기 전에 쥔 곳"\n'), [])
        self.assertEqual(self.after(self.set_cell("DT_Hazards", "crisis_test_storm", "ResolveTemplates", ["{System}의 안개가 걷혔다."])), [])
        self.assertEqual(self.after(self.set_cell("DT_Megaprojects", "project_test_build", "CompleteTemplates", ["{Civ}|hpp(이,가) 끝까지 지었다.", "{System}에 불빛이 켜졌다.", "{Project} c."])), [])
        held = next(row for row in json.loads(checker.read_sources()["DT_LogTemplates.json"].decode("utf-8")) if row["Name"] == "log_siege_held")["Templates"]
        self.assertTrue(any("지켜 냈" in line for line in held))
        self.assertEqual(self.after(self.set_cell("DT_Encounters", "petition_test_plea", "Body", "{Civ}|hpp(이,가) 도움을 구한다.")), [])
        # (6) Every checked prefix or key names a key of the project's string table (a misspelt key checks nothing).
        for prefix in prefixes:
            with self.subTest(prefix_used=prefix):
                self.assertTrue(any(key.startswith(prefix) for key in project_st), prefix)

    def test_the_ark_and_its_word(self):
        """B6b [B-6b6] (header 3.3 rule 13, 3.5 rule 2, decision 33): '방주' stands only in the ark (project_ark) and the
        fourth-rung feat (feat_passage_by_ark). The schema refuses it on the extinction's name and lines, on the event and
        log lines and, by two row_not rules, on every other project and feat; the ark's own rule keeps rescue, title and
        holy names out of its name and self-rescue and holy names out of its completion lines; '노아' is a name of
        holy_and_myth. The rest of the project - a card, a string - is read here, not by check_data."""
        self.assertEqual(self.after(), [])
        lists = self.schema["words"]
        tables = self.schema["tables"]
        # The list and the name stand as written.
        self.assertEqual(lists["ark_words"], ["방주"])
        self.assertIn("노아", lists["holy_and_myth"])

        def why_of(table, when):
            # A rule taken out reads as no line at all, so its case fails (not errs).
            return next((rule["why"] for rule in tables[table]["rules"] if rule.get("when") == when), "(no rule when %s)" % json.dumps(when))

        def broken(problems, where, why, column, listed):
            self.expect(problems, where, why, json.dumps({"col": column, "lacks_words": listed}))

        word = "the word 방주 is not used here"
        # (1) Each place of the list as a forbidden word: the extinction's name and three columns, an event line, a log line.
        self.expect(self.after(self.set_cell("DT_Hazards", "crisis_test_storm", "DisplayName", "방주")), "crisis_test_storm.DisplayName", word)
        for column in ("OutbreakTemplates", "ExpireTemplates", "ResolveTemplates"):
            with self.subTest(hazard=column):
                self.expect(self.after(self.set_cell("DT_Hazards", "crisis_test_storm", column, ["{System}의 방주 a.", "b.", "c."])), "crisis_test_storm.%s" % column, word)
        self.expect(self.after(self.set_cell("DT_Events", "event_test_harvest", "LogTemplates", ["{Civ}의 방주 a.", "b.", "c."])), "event_test_harvest.LogTemplates", word)
        self.expect(self.after(self.set_cell("DT_LogTemplates", "log_far_event", "Templates", ["방주 a.", "b.", "c."])), "log_far_event.Templates", word)
        # (2) The ark's own rule: its completion lines rescue no one, its name is no holy name.
        ark_why = why_of("DT_Megaprojects", [{"row": "project_ark"}])
        ark = lambda column, value: self.set_cell("DT_Megaprojects", "project_ark", column, value)  # noqa: E731
        broken(self.after(ark("CompleteTemplates", ["{Civ}|hpp(은,는) 방주로 백성을 건져 냈다.", "{System} b.", "{Project} c."])),
               "DT_Megaprojects.project_ark", ark_why, "CompleteTemplates", "self_rescue_words")
        broken(self.after(ark("DisplayName", "노아의 배")), "DT_Megaprojects.project_ark", ark_why, "DisplayName", "holy_and_myth")
        # (3) The two row_not rules: '방주' on another feat or another project.
        feat_why = why_of("DT_Feats", [{"row_not": "feat_passage_by_ark"}])
        project_why = why_of("DT_Megaprojects", [{"row_not": "project_ark"}])
        broken(self.after(self.set_cell("DT_Feats", "feat_test_climb", "LogTemplates", ["{Civ}|hpp(은,는) 방주를 지었다.", "{Civ} b.", "c."])),
               "DT_Feats.feat_test_climb", feat_why, "LogTemplates", "ark_words")
        broken(self.after(self.set_cell("DT_Megaprojects", "project_test_build", "CompleteTemplates", ["{Civ}의 방주 a.", "{System} b.", "{Project} c."])),
               "DT_Megaprojects.project_test_build", project_why, "CompleteTemplates", "ark_words")
        # ... and in the two rows' own cells it passes.
        own = lambda tables: (ark("DisplayName", "방주")(tables),  # noqa: E731
                              ark("CompleteTemplates", ["{Civ}|hpp(이,가) {System}에서 {Project}|hpp(을,를) 완성했다.", "{System} b.", "{Project} c."])(tables),
                              self.set_cell("DT_Feats", "feat_passage_by_ark", "Title", "방주와 조약")(tables),
                              self.set_cell("DT_Feats", "feat_passage_by_ark", "MythicNarrative", "방주를 지은 백성.")(tables),
                              self.set_cell("DT_Feats", "feat_passage_by_ark", "LogTemplates", ["{Civ}|hpp(은,는) 방주를 지어 위기를 건넜다.", "{HomeSystem} b.", "c."])(tables))
        self.assertEqual(self.after(own), [])

        # (4) The whole project: outside the two rows, no source holds the word - a card, a string, any table (schema.json
        # names the list and is left out). This is a test, not a check_data gate: the runner records it and exits 1.
        keep = {("DT_Megaprojects.json", "project_ark"), ("DT_Feats.json", "feat_passage_by_ark")}

        def strays(sources):
            found = []
            for name, data in sorted(sources.items()):
                if name == checker.SCHEMA_FILE:
                    continue
                text = checker.text_of(data)
                for ark_word in lists["ark_words"]:
                    if name.endswith(".json"):
                        found += ["%s %s" % (name, row.get("Name")) for row in json.loads(text)
                                  if (name, row.get("Name")) not in keep and ark_word in json.dumps(row, ensure_ascii=False)]
                    else:
                        found += ["%s:%d" % (name, number) for number, line in enumerate(text.splitlines(), 1) if ark_word in line]
            return found

        sources = checker.read_sources()
        self.assertEqual(strays(sources), [])
        # The two rows themselves do hold it (the walk skips them by name, not by luck).
        self.assertTrue(all(any("방주" in json.dumps(row, ensure_ascii=False) for row in json.loads(checker.text_of(sources[name])) if row.get("Name") == row_name)
                            for name, row_name in keep))
        # Injected: a card's body (check_data lets it through) and a string of ST_CA are found.
        card = copy.deepcopy(sources)
        cards = json.loads(checker.text_of(card["DT_Encounters.json"]))
        cards[0]["Body"] += " 방주"
        card["DT_Encounters.json"] = json.dumps(cards, ensure_ascii=False).encode("utf-8")
        self.assertEqual(strays(card), ["DT_Encounters.json %s" % cards[0]["Name"]])
        strings = copy.deepcopy(sources)
        strings["ST_CA.csv"] = strings["ST_CA.csv"] + '"Panel.ArkTest","방주"\n'.encode("utf-8")
        found = strays(strings)
        self.assertEqual(len(found), 1)
        self.assertTrue(found[0].startswith("ST_CA.csv:"), found)

    def test_the_witness_words(self):
        """B6b [B-6b8] (header 2.7, 2.8, notes 1.6 rule 7; decisions 20 and 22): a wanderer's card on the claim is a sign
        seen or measured - no testimony, record, telling or speaking in any of its texts (witness_words, the contracted past
        forms listed apart), no voice of the sign in its body (sign_voice_words), and '기록' in the choices' success lines
        alone (witness_record_words; the step back's success line is the text audit's - a column test cannot tell one choice
        from another). The rule reads the enc_ rows whose conditions name the claim, and nothing else. The era of an endured
        pressure keeps the shape of decision 22: the ruled people's condition and the Borders reverence."""
        self.assertEqual(self.after(), [])
        lists = self.schema["words"]
        encounters = self.schema["tables"]["DT_Encounters"]["rules"]
        events = self.schema["tables"]["DT_Events"]["rules"]
        claim_when = [{"row_prefix": "enc_"}, {"col": "Conditions", "has_cond": {"tag": "Hazard.Claim"}}]
        # A rule taken out reads as no line at all, so its cases fail (not err).
        witness_why = next((rule["why"] for rule in encounters if rule.get("when") == claim_when), "(no witness rule)")
        shape_then = [{"col": "Conditions", "has_cond": {"type": "SystemOwnedBy", "key": "Ruled"}}, {"col": "Effects", "has_cond": {"type": "ChangeReverence", "key": "Borders"}}]
        shape_why = next((rule["why"] for rule in events if any(test in shape_then for test in rule["then"])), "(no era shape rule)")

        def broken(problems, where, why, column, listed):
            self.expect(problems, where, why, json.dumps({"col": column, "lacks_words": listed}))

        claim = [cond("PlayerRoleIs", key="Wanderer"), cond("PlayerInSystemWithHazard", tag="Hazard.Claim", key="None")]

        def card(on_claim=True, **changes):
            # The fixture's arrival card, made a Standing card on the claim (or left an arrival card on no hazard).
            def change(tables):
                row = self.row(tables, "DT_Encounters", "enc_test_weave")
                if on_claim:
                    row.update(Trigger="Standing", Conditions=copy.deepcopy(claim))
                for column, value in changes.items():
                    head, _, index_and_sub = column.partition(":")
                    if index_and_sub:
                        index, _, sub = index_and_sub.partition(".")
                        row[head][int(index)][sub] = value
                    else:
                        row[head] = value
            return change

        where = "DT_Encounters.enc_test_weave"
        self.assertEqual(self.after(card()), [])
        # (1) One breaking case per column and list: a testimony in a label, a word for 'saw' in the body, a voice of the
        # sign in the body, a flight and a leaving in the step back's line, a record made in a failure line, '기록' in the body.
        broken(self.after(card(**{"Choices:0.Label": "{System}의 일을 증언한다."})), where, witness_why, "Choices.Label", "witness_words")
        broken(self.after(card(Body="{System}의 그늘을 목격한 자는 적다.")), where, witness_why, "Body", "witness_words")
        broken(self.after(card(Body="{System}의 그늘에서 목소리가 들린다.")), where, witness_why, "Body", "sign_voice_words")
        broken(self.after(card(**{"Choices:1.SuccessText": "당신은 {System}에서 도망쳤다."})), where, witness_why, "Choices.SuccessText", "witness_words")
        broken(self.after(card(**{"Choices:0.FailureText": "{System}의 그늘을 기록하지 못했다."})), where, witness_why, "Choices.FailureText", "witness_words")
        # The contracted past forms are words of the list on their own: '기록했다' holds no '기록하', '떠났다' no '떠나'.
        broken(self.after(card(**{"Choices:0.FailureText": "{System}의 그늘을 기록했다."})), where, witness_why, "Choices.FailureText", "witness_words")
        broken(self.after(card(**{"Choices:1.SuccessText": "당신은 {System}을 떠났다."})), where, witness_why, "Choices.SuccessText", "witness_words")
        # The adnominal forms (-ㄴ, -ㄹ) hold no stem of the list either: '떠난' holds no '떠나', '기록한' no '기록하', and the plain
        # present '말한다' no '말하' - they are words of the list on their own, as are the nominal forms (-ㅁ) '떠남', '기록함'.
        broken(self.after(card(**{"Choices:1.SuccessText": "당신은 {System}을 떠난 뒤였다."})), where, witness_why, "Choices.SuccessText", "witness_words")
        broken(self.after(card(**{"Choices:0.SuccessText": "당신이 기록한 것은 {System}의 가장자리다."})), where, witness_why, "Choices.SuccessText", "witness_words")
        broken(self.after(card(Body="{System}의 그늘을 {Civ}에게 말한다.")), where, witness_why, "Body", "witness_words")
        broken(self.after(card(**{"Choices:0.FailureText": "{System}의 떠남이 늦었다."})), where, witness_why, "Choices.FailureText", "witness_words")
        broken(self.after(card(Body="{System}의 기록이 남아 있다.")), where, witness_why, "Body", "witness_record_words")
        broken(self.after(card(**{"Choices:1.Label": "기록을 남긴다"})), where, witness_why, "Choices.Label", "witness_record_words")
        # '기록' stands in a success line (the measure kept in the scholars' record) - and only there.
        self.assertEqual(self.after(card(**{"Choices:0.SuccessText": "당신이 잰 {System}의 가장자리는 학자들의 기록에 남았다."})), [])
        # (2) The scope: the same words on a wanderer's card that names no claim, and on a petition on the claim, pass.
        words = {"Body": "{System}에서 목소리를 목격했다. 기록이 남았다.", "Choices:0.Label": "증언한다", "Choices:1.SuccessText": "{System}을 떠났다."}
        self.assertEqual(self.after(card(on_claim=False, **words)), [])
        plea = lambda tables: self.row(tables, "DT_Encounters", "petition_test_plea").update(  # noqa: E731
            Conditions=[cond("PlayerRoleIs", key="Ruler"), cond("SystemHasHazard", tag="Hazard.Claim")], Body="{Civ}의 기록과 증언.")
        self.assertEqual(self.after(plea), [])
        # (3) The game's card is in the rule's scope: a testimony in its label is refused (the project's own tables - the
        # fixture replaces the project's cards).
        schema, project, tags = checker.load_project()
        self.assertEqual(checker.check(schema, project, tags), [])

        def game(table, name, change):
            changed = copy.deepcopy(project)
            change(self.row(changed, table, name))
            return checker.check(schema, changed, tags)

        self.expect(game("DT_Encounters", "enc_witness", lambda row: row["Choices"][0].update(Label="그늘을 증언한다")),
                    "DT_Encounters.enc_witness", witness_why, json.dumps({"col": "Choices.Label", "lacks_words": "witness_words"}))

        # (4) The era's shape: without Ruled, or with the reverence of the context people, the row is refused.
        endured = cond("CivPressureEnduredTicksReached", key="600")
        era = lambda conditions, effects: (lambda tables: self.row(tables, "DT_Events", "event_test_harvest").update(  # noqa: E731
            Trigger="Standing", BaseWeight=0, Conditions=conditions, Effects=effects))
        borders = [eff("ChangeReverence", key="Borders", value=0.2)]
        self.assertEqual(self.after(era([cond("SystemOwnedBy", key="Ruled"), endured], borders)), [])
        self.expect(self.after(era([endured], borders)), "DT_Events.event_test_harvest", shape_why, json.dumps(shape_then[0]))
        self.expect(self.after(era([cond("SystemOwnedBy", key="Ruled"), endured], [eff("ChangeReverence", key="None", value=0.2)])),
                    "DT_Events.event_test_harvest", shape_why, json.dumps(shape_then[1]))
        # The game's era row is in the rule's scope: its Ruled taken out is refused.
        self.expect(game("DT_Events", "event_era_endured", lambda row: row.update(Conditions=[endured])),
                    "DT_Events.event_era_endured", shape_why, json.dumps(shape_then[0]))

        # (5) The lists stand as the audit wrote them (pinned last, so that a word taken out fails its case above first).
        self.assertEqual(lists["witness_words"], ["본 것", "보고 들은", "말하", "말했", "말해", "전하", "전했", "전해", "증거", "증언", "증인", "목격", "기록하", "기록했", "기록해",
                                                  "지나간다", "지나가", "지나갔", "떠난다", "떠나", "떠났", "외면", "눈을 돌", "등을 돌", "모른 척", "피했", "도망", "부끄",
                                                  "말한", "말할", "전한", "전할", "기록한", "기록할", "떠난", "떠날", "지나간", "지나갈",
                                                  "말함", "전함", "기록함", "떠남", "지나감"])
        self.assertEqual(lists["sign_voice_words"], ["목소리", "속삭", "이르기를", "말씀"])
        self.assertEqual(lists["witness_record_words"], ["기록"])
        # The three lists hang on the witness rule alone.
        hung = [(table, rule["why"]) for table, spec in self.schema["tables"].items() for rule in spec.get("rules", [])
                for test in rule["then"] if test.get("lacks_words") in ("witness_words", "sign_voice_words", "witness_record_words")]
        self.assertEqual({key for key in hung}, {("DT_Encounters", witness_why)})
        self.assertEqual(len(hung), 12)

    def test_a_text_has_one_form(self):
        """A word is found as it is written: a text is composed (NFC) and holds no invisible format character, so a verdict
        word cannot hide in decomposed Hangul or behind a zero-width space."""
        soul = "\uc601\ud63c"
        templates = lambda first: self.set_cell("DT_Events", "event_test_harvest", "LogTemplates", ["{Civ} %s a." % first, "{Civ} b.", "{Civ} c."])  # noqa: E731
        decomposed = unicodedata.normalize("NFD", soul)
        self.expect(self.after(templates(decomposed)), "DT_Events.event_test_harvest.LogTemplates", "a text not in the composed form (NFC)")
        self.expect(self.after(templates(soul[0] + "\u200b" + soul[1])), "DT_Events.event_test_harvest.LogTemplates", "an invisible format character U+200B")
        self.expect(self.after(templates(soul)), "the word %s is not used here" % soul)
        self.assertEqual(self.after(templates("\ub0a0")), [])

    def test_rows_that_never_fire(self):
        """A Lottery row with no weight never fires, and a Draw row with no chance never stands: the two boundaries are
        pinned by hand (a walk built from the schema's own operand would move with a weakened rule)."""
        weight = lambda value: self.set_cell("DT_Events", "event_test_harvest", "BaseWeight", value)  # noqa: E731
        self.expect(self.after(weight(0)), "event_test_harvest", "a Lottery row with no weight never fires")
        self.expect(self.after(weight(-1)), "event_test_harvest", "a Lottery row with no weight never fires")
        self.assertEqual(self.after(weight(0.5)), [])
        chance = lambda value: self.set_cell("DT_Hazards", "crisis_test_storm", "SpawnChancePerTick", value)  # noqa: E731
        self.expect(self.after(chance(0)), "crisis_test_storm", "a Draw row is drawn every tick")
        self.expect(self.after(chance(-1)), "crisis_test_storm", "a Draw row is drawn every tick")
        self.assertEqual(self.after(chance(0.001)), [])

    def test_stay_condition_rows(self):
        """The stay in a fallen sphere stands on Civilization slot rows only; the row the research stage fires is Named."""
        stay = cond("CivFallenSphereTicksReached", key="300")
        harvest = lambda **changes: (lambda tables: self.row(tables, "DT_Events", "event_test_harvest").update(changes))  # noqa: E731
        self.assertEqual(self.after(harvest(Trigger="Standing", BaseWeight=0, Conditions=[stay])), [])
        self.assertEqual(self.after(harvest(Conditions=[stay])), [])
        self.expect(self.after(harvest(Trigger="Named", BaseWeight=0, CooldownTicks=0, LogTemplates=["one."], Conditions=[stay])),
                    "a row fired by name is not a slot", "CivFallenSphereTicksReached")
        self.expect(self.after(harvest(Subject="SettledSystem", Conditions=[stay])), "a system-subject row lands on a place", "CivFallenSphereTicksReached")
        self.expect(self.after(harvest(Trigger="Standing", BaseWeight=0, Subject="SettledSystem", Conditions=[stay])),
                    "a system-subject row lands on a place", "CivFallenSphereTicksReached")
        found = lambda **changes: (lambda tables: self.row(tables, "DT_Events", "event_book_found").update(changes))  # noqa: E731
        self.expect(self.after(found(Trigger="Standing")), "event_book_found", "the research stage fires the found row by name")
        self.expect(self.after(found(Trigger="Lottery", BaseWeight=1.0, CooldownTicks=600, LogTemplates=["a.", "b.", "c."])),
                    "event_book_found", "the research stage fires the found row by name")

    def test_the_slot_rule_covers_the_vocabulary(self):
        """The rules of the rows every people runs (slot rows, feat rows) name their conditions and effects one by one. An
        enumerator added to the vocabulary later has to be put on one side here: a condition on the vessel or an effect on it
        goes into both rules."""
        world_effects = {"AddResource", "MultiplyIncome", "ShiftEthics", "AddTrait", "RemoveTrait", "ChangeStability", "ChangeRelation", "SpawnHazard",
                         "ResolveHazard", "TransferSystem", "ChangeReverence", "SetFlag", "ChangePopulation", "NameNotable", "FocusResearch",
                         "SetStance", "EnactEdict", "DelayHazard"}
        for table, start, conditions, effects in (("DT_Events", "a slot runs for every people", "Conditions", "Effects"),
                                                  ("DT_Feats", "a ruler feat row judges every people", "CompletionConditions", "AscensionCost")):
            with self.subTest(table=table):
                rule = next(rule for rule in self.schema["tables"][table]["rules"] if rule["why"].startswith(start))
                named = lambda column: {test["lacks_type"] for test in rule["then"] if test["col"] == column}  # noqa: E731
                self.assertEqual(named(conditions), {name for name in self.schema["conditions"] if name.startswith("Player")},
                                 "every condition on the vessel is in the rule, and nothing else")
                self.assertEqual(named(effects) | world_effects, set(self.schema["effects"]),
                                 "a new effect is either an effect on the world (add it to this list) or one on the vessel or the role (add it to both rules)")
                self.assertEqual(named(effects) & world_effects, set())

    def test_sign_row_and_grace_row(self):
        cell = lambda column, value: self.set_cell("DT_Events", "event_test_harvest", column, value)  # noqa: E731

        def sign(min_tier, category, templates=("{Civ} a.", "{Hazard} b.", "c.")):
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
        # A sign row names no system: it copies the focus of the row's line, which names none for a Civilization row, and
        # it may serve rows of either subject. The slot is caught after an allowed one and after a particle too.
        for templates in (("{Civ}|hpp(\uc774,\uac00) {System} a.", "b.", "c."), ("{Civ} {HomeSystem} a.", "b.", "c."), ("a.", "b.", "{Hazard} {System} c.")):
            with self.subTest(templates=templates):
                self.expect(self.after(sign(3, "Log.Sign", templates)), "SignRow", "does not hold", "slots_in")
        self.expect(self.after(cell("SignRow", "log_nobody")), "DT_LogTemplates has no row log_nobody")

        def grace(*effects):
            def change(tables):
                self.row(tables, "DT_Events", "event_test_harvest").update(Category="Event.Grace.Jubilee", LogCategory="Log.Civ", Effects=list(effects))
            return change
        # The calling's own line that waits for a tier comes with its sign row; without a window it needs none.
        rescue = lambda **changes: (lambda tables: self.row(tables, "DT_Events", "event_test_harvest").update(  # noqa: E731
            dict(dict(Category="Event.Grace.Rescue", LogCategory="Log.Calling"), **changes)))
        waits = "the calling's own line that waits for a tier comes with its sign row"
        self.assertEqual(self.after(rescue()), [])
        self.assertEqual(self.after(rescue(MinTierToReveal=1)), [])
        self.expect(self.after(rescue(MinTierToReveal=2)), "event_test_harvest", waits, '"SignRow"')
        self.expect(self.after(rescue(MinTierToReveal=5)), "event_test_harvest", waits, '"SignRow"')
        self.assertEqual(self.after(lambda tables: (self.add_sign_row(tables), rescue()(tables))), [])
        self.expect(self.after(rescue(LogCategory="Log.Civ", Category="Event.Grace.Jubilee", MinTierToReveal=3)), waits, wanted=False)

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
        # A fallen one is named by its title (guideline 4.2, 6.4): never empty, no number, no slot.
        self.expect(self.after(cell("trans_silence", "Title", "")), "trans_silence", "named by its title", '"set": true')
        self.expect(self.after(cell("trans_silence", "Title", "the 7th")), "trans_silence.Title", "a digit in a text that carries none")
        self.expect(self.after(cell("trans_silence", "Title", "{Civ} a.")), "trans_silence.Title", "slot {Civ} is not filled here")
        # B5 [B-5d1] (critic-6): every row has its title - the two books' claim groups stand under '{Transcendent}의 말', so a
        # claimant without a title would speak under another's head.
        self.expect(self.after(cell("trans_covenant", "Title", "")), "trans_covenant.Title", "a text is required here")
        self.expect(self.after(cell("trans_silence", "Title", "")), "trans_silence.Title", "a text is required here")
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
        self.expect(self.conditions(cond("CivHasTrait", tag="Hazard.Plague")), "not under Trait. or Brick.")
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
        # The stay in a fallen sphere: a count of ticks, a doctrine that may be left out, no Value.
        self.assertEqual(self.conditions(cond("CivFallenSphereTicksReached", key="300")), [])
        self.assertEqual(self.conditions(cond("CivFallenSphereTicksReached", key="300", tag="Doctrine.Silence")), [])
        for key in ("300ticks", "0"):
            self.expect(self.conditions(cond("CivFallenSphereTicksReached", key=key)), "(CivFallenSphereTicksReached).Key", "not a count of ticks")
        self.expect(self.conditions(cond("CivFallenSphereTicksReached")), "(CivFallenSphereTicksReached).Key", "a Key is required")
        self.expect(self.conditions(cond("CivFallenSphereTicksReached", key="300", tag="Trait.Faith.Book")), "(CivFallenSphereTicksReached).Tag", "not under")
        # The counter counts fallen spheres: a celestial doctrine never has one (no instance), any fallen doctrine may.
        self.expect(self.conditions(cond("CivFallenSphereTicksReached", key="300", tag="Doctrine.Covenant")), "(CivFallenSphereTicksReached).Tag", "not under")
        self.assertEqual(self.conditions(cond("CivFallenSphereTicksReached", key="300", tag="Doctrine.Zeal")), [])
        self.expect(self.conditions(cond("CivFallenSphereTicksReached", key="300", value=1)), "(CivFallenSphereTicksReached)", "reads no Value")
        # A completion: the project row is required and has to be a row of DT_Megaprojects.
        self.assertEqual(self.conditions(cond("MegaprojectComplete", key="project_test_build")), [])
        self.expect(self.conditions(cond("MegaprojectComplete")), "(MegaprojectComplete).Key", "a Key is required")
        self.expect(self.conditions(cond("MegaprojectComplete", key="project_nowhere")), "(MegaprojectComplete).Key", "DT_Megaprojects has no row project_nowhere")
        # A research field and a share of it.
        self.assertEqual(self.conditions(cond("CivResearchAtLeast", key="Physics", value=0.5)), [])
        self.expect(self.conditions(cond("CivResearchAtLeast", key="Alchemy", value=0.5)), "(CivResearchAtLeast).Key", "Alchemy is none of Physics")
        self.expect(self.conditions(cond("CivResearchAtLeast", key="Physics", value=1.5)), "(CivResearchAtLeast).Value", "is above 1")
        self.expect(self.conditions(cond("CivResearchAtLeast", key="Physics", value=-0.1)), "(CivResearchAtLeast).Value", "is below 0")
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
        self.assertEqual(named(eff("GrantPower", tag="Way.Received"), eff("DeposeRuler", tag="Reign.Deposed")), [])
        # A death comes only from a hazard's expiry (B5 decision 6): no event row records one.
        self.expect(named(eff("KillPlayer", tag="Death.Hazard")), "no event row records one")
        self.expect(named(eff("KillPlayer", tag="Reign.Deposed")), "not under Death.Hazard")
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

    def test_the_trait_roots_hold_the_bricks(self):
        """B7x [B-7x1] (rule R1): the existence bricks ride in a people's Traits, so the four trait roots - DT_Civs.Traits,
        CivHasTrait, AddTrait and RemoveTrait - are the list Trait. and Brick.; any other root is still refused, and a brick
        has to be declared in the ini like a trait (a parent that only its children imply is not)."""
        self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_default", "Traits", tags_text("Trait.Arch.Default", "Brick.Diet.StarEater"))), [])
        self.assertEqual(self.conditions(cond("CivHasTrait", tag="Brick.Space.Nomad")), [])
        self.assertEqual(self.effects(eff("AddTrait", tag="Brick.Instinct.SwarmTech"), eff("RemoveTrait", tag="Brick.Form.Society")), [])
        for root in ("DT_Civs", "CivHasTrait", "AddTrait", "RemoveTrait"):
            with self.subTest(root=root):
                if root == "DT_Civs":
                    found = self.after(self.set_cell("DT_Civs", "arch_default", "Traits", tags_text("Trait.Arch.Default", "Hazard.Plague")))
                elif root == "CivHasTrait":
                    found = self.conditions(cond("CivHasTrait", tag="Hazard.Plague"))
                else:
                    found = self.effects(eff(root, tag="Hazard.Plague"))
                self.expect(found, "Hazard.Plague is not under Trait. or Brick.")
        self.expect(self.conditions(cond("CivHasTrait", tag="Brick.Diet")), "(CivHasTrait).Tag", "Brick.Diet", "not declared")
        self.expect(self.effects(eff("AddTrait", tag="Brick.Diet.Nobody")), "(AddTrait).Tag", "Brick.Diet.Nobody", "not declared")
        for column, usage in (("DT_Civs.Traits", self.schema["tables"]["DT_Civs"]["columns"]["Traits"]),
                              ("CivHasTrait", self.schema["conditions"]["CivHasTrait"]["tag"]),
                              ("AddTrait", self.schema["effects"]["AddTrait"]["tag"]),
                              ("RemoveTrait", self.schema["effects"]["RemoveTrait"]["tag"])):
            with self.subTest(column=column):
                self.assertEqual(usage["root"], ["Trait.", "Brick."])

    def test_existence_vocabulary(self):
        """B7x [B-7x2] (header section 2.3, 2.4): the three conditions appended after CivPressureEnduredTicksReached - a time
        vector word and a whole count from 1, a tag a people lacks, a tag a bordering people carries (not on a system-subject
        row, not in a hazard's spawn conditions) - and the brick knobs of DT_Rules. One input that breaks each."""
        self.assertEqual(self.after(), [])
        # CivTimeVectorAtLeast: a word of time_vectors, a whole number of ticks from 1, no Tag.
        for word in self.schema["words"]["time_vectors"]:
            with self.subTest(word=word):
                self.assertEqual(self.conditions(cond("CivTimeVectorAtLeast", key=word, value=600)), [])
        self.assertEqual(self.schema["words"]["time_vectors"], ["Built", "Consumed", "Drifted", "Linked", "Isolated", "Endured"])
        self.expect(self.conditions(cond("CivTimeVectorAtLeast", key="Wandered", value=600)), "(CivTimeVectorAtLeast).Key", "Wandered is none of Built")
        self.expect(self.conditions(cond("CivTimeVectorAtLeast", key="built", value=600)), "(CivTimeVectorAtLeast).Key", "built is none of Built")
        self.expect(self.conditions(cond("CivTimeVectorAtLeast", value=600)), "(CivTimeVectorAtLeast).Key", "a Key is required")
        self.expect(self.conditions(cond("CivTimeVectorAtLeast", key="Built", value=0)), "(CivTimeVectorAtLeast).Value", "is below 1")
        self.expect(self.conditions(cond("CivTimeVectorAtLeast", key="Built", value=1.5)), "(CivTimeVectorAtLeast).Value", "not a whole number")
        self.expect(self.conditions(cond("CivTimeVectorAtLeast", key="Built", value=600, tag="Brick.Space.Nomad")), "(CivTimeVectorAtLeast)", "reads no Tag")
        # CivLacksTrait and BorderingCivHasTrait: a Trait. or Brick. tag, required, no Key, no Value.
        for type_name in ("CivLacksTrait", "BorderingCivHasTrait"):
            with self.subTest(type_name=type_name):
                self.assertEqual(self.conditions(cond(type_name, tag="Brick.Diet.StarEater")), [])
                self.assertEqual(self.conditions(cond(type_name, tag="Trait.Faith.Book")), [])
                self.expect(self.conditions(cond(type_name)), "(%s).Tag" % type_name, "a tag is required")
                self.expect(self.conditions(cond(type_name, tag="Hazard.Plague")), "(%s).Tag" % type_name, "not under Trait. or Brick.")
                self.expect(self.conditions(cond(type_name, tag="Brick.Diet.StarEater", key="Borders")), "(%s)" % type_name, "reads no Key")
                self.expect(self.conditions(cond(type_name, tag="Brick.Diet.StarEater", value=1)), "(%s)" % type_name, "reads no Value")
        bordering = cond("BorderingCivHasTrait", tag="Brick.Diet.StarEater")
        harvest = lambda **changes: (lambda tables: self.row(tables, "DT_Events", "event_test_harvest").update(changes))  # noqa: E731
        self.assertEqual(self.after(harvest(Trigger="Standing", BaseWeight=0, Conditions=[bordering])), [])
        self.expect(self.after(harvest(Subject="SettledSystem", Conditions=[bordering])), "the bordering condition walks the people's borders", "BorderingCivHasTrait")
        self.assertEqual(self.after(harvest(Subject="SettledSystem", Conditions=[cond("CivLacksTrait", tag="Brick.Diet.StarEater")])), [])
        self.expect(self.after(self.set_cell("DT_Hazards", "crisis_test_storm", "SpawnConditions", [bordering])),
                    "the bordering condition would walk the owner's borders", "BorderingCivHasTrait")
        self.assertEqual(self.after(self.set_cell("DT_Hazards", "crisis_test_storm", "SpawnConditions", [cond("CivLacksTrait", tag="Brick.Space.FoldedSmall")])), [])
        # The knobs: the inert values pass; a range is held; a multiplier of a progress or a share is above 0.
        rules = lambda column, value: self.set_cell("DT_Rules", "rules", column, value)  # noqa: E731
        for column, value, word in (("BrickDarkMatterEnergyPerTick", -0.1, "below"), ("BrickStarEaterEnergyMultiplier", -1, "below"),
                                    ("BrickStarEaterRelationTarget", -1.5, "below"), ("BrickMergeRelationTarget", 1.5, "above"),
                                    ("BrickStarEaterDimPeriodTicks", -1, "below"), ("BrickParasiteCountdownBonusTicks", 100001, "above"),
                                    ("BrickEvolutionPeriodTicks", 2.5, "whole"), ("BrickMutationThresholdTicks", -5, "below")):
            with self.subTest(column=column):
                self.expect(self.after(rules(column, value)), "rules.%s" % column, word)
        self.assertEqual(self.after(rules("BrickStarEaterRelationTarget", -0.3)), [])
        self.assertEqual(self.after(rules("BrickMergeRelationTarget", 0.3)), [])
        for column in ("BrickNomadSettleProgressMultiplier", "BrickInwardResearchMultiplier", "BrickInwardHazardSpawnMultiplier", "BrickSwarmResearchMultiplier",
                       "BrickStellarCrisisSpawnMultiplier"):
            with self.subTest(column=column):
                self.expect(self.after(rules(column, 0)), "rules", "a brick multiplier of a progress or a share", '"%s"' % column)
        # The seed bricks: Brick.* tags only, empty by default.
        self.assertEqual(self.after(rules("BrickSeedTags", tags_text("Brick.Diet.DarkMatter", "Brick.Space.Nomad"))), [])
        self.expect(self.after(rules("BrickSeedTags", tags_text("Trait.Faith.Book"))), "rules.BrickSeedTags", "not under Brick.")
        # [B-7x5s] (header section 3.4; the start values of the data draft [B-7x5a]): the data holds the start values (the C++
        # defaults stay inert); the star stays undimmed (decision 8) and the seed bricks leave out the two that bar settlement.
        start = {row["Name"]: row for row in self.tables["DT_Rules"]}["rules"]
        for column, value in (("BrickDarkMatterEnergyPerTick", 0.5), ("BrickStarEaterEnergyMultiplier", 1.5), ("BrickStarEaterRelationTarget", -0.25),
                              ("BrickStarEaterDimPeriodTicks", 0), ("BrickIdleInsightPerTick", 0.25), ("BrickSpanEnergyPerLane", 0.125),
                              ("BrickParasiteCountdownBonusTicks", 30), ("BrickParasiteInsightPerHazard", 0.25), ("BrickNomadSettleProgressMultiplier", 1.5),
                              ("BrickInwardResearchMultiplier", 1.5), ("BrickInwardHazardSpawnMultiplier", 0.5), ("BrickSwarmResearchMultiplier", 1.25),
                              ("BrickMergeRelationTarget", 0.25), ("BrickEvolutionPeriodTicks", 100), ("BrickMutationThresholdTicks", 1200),
                              ("BrickStellarCrisisSpawnMultiplier", 2.0)):
            with self.subTest(start=column):
                self.assertEqual(start[column], value, "the data holds the start value of [B-7x5s] (header section 3.4)")
        seeds = checker.TAG_NAME.findall(start["BrickSeedTags"])
        self.assertEqual(len(seeds), 8)
        self.assertFalse({"Brick.Space.ClosedPort", "Brick.Space.FoldedSmall"} & set(seeds))

    def test_a_rung_never_from_a_brick(self):
        """B7x [B-7x2] (header K4, section 3.3 rule 7; notes 5.4 data-3 and tests-8): a rung comes from a feat's deed, never
        from a brick or a mutation - no feat row asks whether its people carries a brick or lacks one, nor what a bordering
        people carries, and the rule names every Brick. tag of the ini (a brick declared later breaks the pin until the rule
        names it too). A trait is still a feat's to read. And the settle gate's Inward line (Settle.Block.Inward) is held as
        RealmCapLast is: no verdict, holy, rescue, rung-grasp or title word, and a figure only in a slot."""
        self.assertEqual(self.after(), [])
        k4 = "K4 - a rung comes from a feat's deed, never from a brick or a mutation (B7x header K4)"
        rules = [rule for rule in self.schema["tables"]["DT_Feats"]["rules"] if rule["why"] == k4]
        self.assertEqual(len(rules), 1)
        self.assertNotIn("when", rules[0], "K4 holds for every feat row, on every path")
        bricks = sorted(tag for tag in self.tags if tag.startswith("Brick.") and tag.count(".") == 2)
        self.assertEqual(len(bricks), 17)
        self.assertEqual(rules[0]["then"], [{"col": "CompletionConditions", "lacks_type": "BorderingCivHasTrait"}]
                         + [{"col": "CompletionConditions", "lacks_cond": {"type": type_name, "tag": tag}}
                            for type_name in ("CivHasTrait", "CivLacksTrait") for tag in bricks])
        deed = cond("CivResearchAtLeast", key="Physics", value=0.5)
        feat = lambda *conditions: self.set_cell("DT_Feats", "feat_test_climb", "CompletionConditions", [deed] + list(conditions))  # noqa: E731

        def broken(problems, type_name, tag):
            self.expect(problems, "DT_Feats.feat_test_climb", k4,
                        json.dumps({"col": "CompletionConditions", "lacks_cond": {"type": type_name, "tag": tag}}))

        # The named case: a mutation read as the price of a rung.
        broken(self.after(feat(cond("CivHasTrait", tag="Brick.Mutation.StarTow"))), "CivHasTrait", "Brick.Mutation.StarTow")
        for type_name in ("CivHasTrait", "CivLacksTrait"):
            for tag in bricks:
                with self.subTest(type_name=type_name, tag=tag):
                    broken(self.after(feat(cond(type_name, tag=tag))), type_name, tag)
        self.expect(self.after(feat(cond("BorderingCivHasTrait", tag="Trait.Faith.Book"))), "DT_Feats.feat_test_climb", k4,
                    json.dumps({"col": "CompletionConditions", "lacks_type": "BorderingCivHasTrait"}))
        self.assertEqual(self.after(feat(cond("CivHasTrait", tag="Trait.Faith.Book"))), [])
        self.assertEqual(self.after(feat(cond("CivLacksTrait", tag="Trait.Faith.Book"))), [])
        # A brick in the price is not K4's (a choice of the rule - the notes call it optional): the rule reads the deed only.
        self.assertEqual(self.after(self.set_cell("DT_Feats", "feat_test_climb", "AscensionCost", [eff("AddTrait", tag="Brick.Form.Society")])), [])
        # Settle.Block.Inward: the list pinned, right after RealmCapLast; every word of each list refused, a figure only in a slot.
        prefixes = self.schema["string_table"]["checked_prefixes"]
        inward = ["verdict", "holy_words", "rescue_words", "rung_grasp_words", "title_words"]
        self.assertEqual(prefixes["Settle.Block.Inward"], {"forbidden": inward, "no_digits": True})
        self.assertEqual(list(prefixes).index("Settle.Block.Inward"), list(prefixes).index("Settle.Block.RealmCapLast") + 1)
        lists = self.schema["words"]
        check = lambda text: checker.check_string_table("ST_X.csv", text, self.schema)  # noqa: E731
        self.assertEqual(check('Key,SourceString\n"Settle.Block.Inward","안으로 접힌 백성은 더 넓히지 않는다."\n'), [])
        for listed in inward:
            for word in lists[listed]:
                with self.subTest(key="Settle.Block.Inward", word=word):
                    self.expect(check('Key,SourceString\n"Settle.Block.Inward","%s a."\n' % word), "entry 1 (Settle.Block.Inward)", "the word %s is not used here" % word)
        self.expect(check('Key,SourceString\n"Settle.Block.Inward","칸 3"\n'), "entry 1 (Settle.Block.Inward)", "a digit outside a slot")
        self.assertEqual(check('Key,SourceString\n"Settle.Block.Inward","칸 {0}"\n'), [])
        # The same through the project's sources: the project's own Inward line, rewritten with a title word, is found.
        sources = checker.read_sources()
        rows = list(csv.reader(io.StringIO(checker.text_of(sources["ST_CA.csv"]))))
        self.assertIn("Settle.Block.Inward", [row[0] for row in rows if row])
        word = lists["title_words"][0]
        for row in rows[1:]:
            if row and row[0] == "Settle.Block.Inward":
                row[1] = word + " a."
        sources["ST_CA.csv"] = "".join(",".join('"%s"' % cell.replace('"', '""') for cell in row) + "\n" for row in rows if row).encode("utf-8")
        self.expect(checker.check_sources(sources), "ST_CA.csv", "Settle.Block.Inward", "the word %s is not used here" % word)

    def test_shape_words(self):
        """B7x [B-7x3] (header 3.2, section 4; notes 5.4 data-3 and tests-8; decision 13): the shape word of a people - the
        keys under Shape., which the chronicle's fact line and the system panel's people line show - holds the chronicle's
        lists with the remnant's word and the words of desolation, and shape_merit_words ('견디', '남은' - an enduring or a
        remaining told as a merit), a list that hangs on that prefix alone; a figure only in a slot. The project holds the
        twelve keys the code names (CosmicUI::ShapeKey), each a word of a way of life, and they pass."""
        self.assertEqual(self.after(), [])
        lists = self.schema["words"]
        prefixes = self.schema["string_table"]["checked_prefixes"]
        check = lambda text: checker.check_string_table("ST_X.csv", text, self.schema)  # noqa: E731
        # (1) The list pinned by hand (a walk built from the schema's own operand would move with a weakened rule), right after
        # the chronicle's keys: every list of Chronicle., then remnant_words, desolation_words and shape_merit_words.
        shape = ["verdict", "soul_verbs", "rescue_words", "afterlife_words", "book_of_life_words", "called_away_words", "holy_passive",
                 "rung_grasp_words", "title_words", "binding_words", "remnant_words", "desolation_words", "shape_merit_words"]
        self.assertEqual(prefixes["Shape."], {"forbidden": shape, "no_digits": True})
        self.assertEqual(list(prefixes).index("Shape."), list(prefixes).index("Chronicle.NoVessel") + 1)
        self.assertEqual(set(checker.forbidden_lists(prefixes["Chronicle."])) - set(shape), set())
        self.assertEqual(lists["shape_merit_words"], ["견디", "남은"])
        # '견디' joins no other list: desolation_words hangs on row rules too (header 3.2).
        self.assertNotIn("견디", lists["desolation_words"])
        # (2) One break per list under the prefix, every word of shape_merit_words, and a figure outside a slot.
        for listed in shape:
            word = lists[listed][0]
            with self.subTest(listed=listed):
                self.expect(check('Key,SourceString\n"Shape.Test","%s 살림"\n' % word), "entry 1 (Shape.Test)", "the word %s is not used here" % word)
        for word in lists["shape_merit_words"]:
            with self.subTest(merit=word):
                self.expect(check('Key,SourceString\n"Shape.Test","오래 %s 살림"\n' % word), "entry 1 (Shape.Test)", "the word %s is not used here" % word)
        self.expect(check('Key,SourceString\n"Shape.Test","7대의 살림"\n'), "entry 1 (Shape.Test)", "a digit outside a slot")
        self.assertEqual(check('Key,SourceString\n"Shape.Test","홀로 선 살림"\n'), [])
        # (3) shape_merit_words hangs on Shape. alone: no other key and no column names it, so a chronicle key may say it.
        self.assertEqual([prefix for prefix, spec in prefixes.items() if "shape_merit_words" in checker.forbidden_lists(spec)], ["Shape."])
        self.assertEqual(self.columns_of(lambda spec: "shape_merit_words" in checker.forbidden_lists(spec)), [])
        self.assertEqual(check('Key,SourceString\n"Chronicle.Test","견디어 남은 길 a."\n'), [])
        # (4) The project's keys are the code's twelve (CosmicUI::ShapeKey - one per CosmicBricks::ECosmicShape but Count), each a
        # word of a way of life ('살림'); one rewritten with a merit word is found through the sources.
        labels = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "Source", "Cosmic_Ascension", "UI", "CosmicUILabels.h")
        with open(labels, encoding="utf-8") as handle:
            code = handle.read()
        named = []
        at = code.find('TEXT("Shape.')
        while at != -1:
            start = at + len('TEXT("')
            named.append(code[start:code.index('"', start)])
            at = code.find('TEXT("Shape.', start)
        sources = checker.read_sources()
        rows = list(csv.reader(io.StringIO(checker.text_of(sources["ST_CA.csv"]))))
        keys = [row[0] for row in rows[1:] if row and row[0].startswith("Shape.")]
        self.assertEqual(len(named), 12)
        self.assertEqual(sorted(keys), sorted(named))
        for row in rows[1:]:
            if row and row[0].startswith("Shape."):
                with self.subTest(key=row[0]):
                    self.assertTrue(row[1].endswith("살림"), row)
                    self.assertEqual(check('Key,SourceString\n"%s","%s"\n' % (row[0], row[1])), [])
        word = lists["shape_merit_words"][0]
        for row in rows[1:]:
            if row and row[0] == "Shape.Endured":
                row[1] = "눌림 속에 %s어 온 살림" % word
        sources["ST_CA.csv"] = "".join(",".join('"%s"' % cell.replace('"', '""') for cell in row) + "\n" for row in rows if row).encode("utf-8")
        self.expect(checker.check_sources(sources), "ST_CA.csv", "Shape.Endured", "the word %s is not used here" % word)

    def test_crossroads_words(self):
        """B7x [B-7x4] (header 3.5; notes 5.4 data-3 and tests-8; the text audit's 7.3): crossroads_words - the fenced words
        of the crossroads (silence, fence, covenant, ark, shepherd, pilgrimage, sanctuary, the deathless, becoming one) - and
        the rules that hang it with silence_words on a crossroads card (a DT_Encounters row whose conditions ask
        CivTimeVectorAtLeast, with rung_grasp_words and title_words, in every text field), on the lines of a DT_Events row that
        asks CivTimeVectorAtLeast or BorderingCivHasTrait, and on the scan face log_brick_scan_spread by its name (a log row has
        no conditions). The list and the rules pinned by hand; one break per list a rule newly hangs, each one refused by that
        rule alone - the same input passes with the rule taken out (a word a column already refuses would prove nothing)."""
        self.assertEqual(self.after(), [])
        lists = self.schema["words"]
        # (1) The list, the last of words; no column and no string key refuses it: it hangs on rules alone.
        self.assertEqual(lists["crossroads_words"], ["침묵", "울타리", "언약", "방주", "목자", "순례", "성소", "불멸", "하나가 되"])
        self.assertEqual(list(lists)[-1], "crossroads_words")
        self.assertEqual(self.columns_of(lambda spec: "crossroads_words" in checker.forbidden_lists(spec)), [])
        self.assertEqual([prefix for prefix, spec in self.schema["string_table"]["checked_prefixes"].items()
                          if "crossroads_words" in checker.forbidden_lists(spec)], [])
        # (2) The four rules pinned by hand (a walk built from the schema's own operands would move with a weakened rule).
        card = "a crossroads card tells what a people did and where it then stands: its words name no rest or silence, no grasp of a rung, no title, and none of the fenced words of the crossroads - silence, fence, covenant, ark, shepherd, pilgrimage, sanctuary, the deathless, becoming one (B7x header 3.5, notes 5.4)"
        lived = "a line of a people's lived way tells no rest or silence and none of the fenced words of the crossroads (B7x header 3.5, notes 5.4)"
        bordering = "a line about a bordering people's brick tells no rest or silence and none of the fenced words of the crossroads (B7x header 3.5, notes 5.4)"
        scan = "the scan face of a people spread between the stars tells no rest or silence and none of the fenced words of the crossroads, and is named because a log row has no conditions (B7x header 3.5, S15)"
        rules = {(table, rule["why"]): rule for table, spec in self.schema["tables"].items() for rule in spec.get("rules", [])}
        card_lists = ["silence_words", "rung_grasp_words", "title_words", "crossroads_words"]
        card_columns = ["Title", "Body", "VoiceBodies", "Choices.Label", "Choices.SuccessText", "Choices.FailureText"]
        self.assertEqual(rules[("DT_Encounters", card)]["when"], [{"col": "Conditions", "has_type": "CivTimeVectorAtLeast"}])
        self.assertEqual(rules[("DT_Encounters", card)]["then"], [{"col": column, "lacks_words": listed} for column in card_columns for listed in card_lists])
        lines = [{"col": "LogTemplates", "lacks_words": "silence_words"}, {"col": "LogTemplates", "lacks_words": "crossroads_words"}]
        for why, type_name in ((lived, "CivTimeVectorAtLeast"), (bordering, "BorderingCivHasTrait")):
            with self.subTest(why=why):
                self.assertEqual(rules[("DT_Events", why)]["when"], [{"col": "Conditions", "has_type": type_name}])
                self.assertEqual(rules[("DT_Events", why)]["then"], lines)
        self.assertEqual(rules[("DT_LogTemplates", scan)]["when"], [{"row": "log_brick_scan_spread"}])
        self.assertEqual(rules[("DT_LogTemplates", scan)]["then"],
                         [{"col": "Templates", "lacks_words": "silence_words"}, {"col": "Templates", "lacks_words": "crossroads_words"}])

        def without(table, why):
            def change(schema):
                schema["tables"][table]["rules"] = [rule for rule in schema["tables"][table]["rules"] if rule["why"] != why]
            return change

        def refused_by_it_alone(table, why, change, where, test):
            self.expect(self.after(change), where, why, json.dumps(test))
            self.assertEqual(self.after(change, without(table, why)), [], "the input has to pass without the rule: %r" % (test,))

        # (3) A crossroads card: '언약' in the title, '안식' in a voice, '올라섰' in a choice's name, '면류관' in its line - none of
        # them a word a petition's column or rule 0 already refuses. The same title on a card that asks no time vector passes.
        vector = cond("CivTimeVectorAtLeast", key="Built", value=600)
        ruler = cond("PlayerRoleIs", key="Ruler")

        def crossroads(**changes):
            def change(tables):
                row = self.row(tables, "DT_Encounters", "petition_test_plea")
                # A first crossroads card (B7x [B-7x5c], header 3.3 rule 5): a Lottery row that waits on the first life's mark,
                # with the no-value choice a card that asks a time vector holds (header 3.3 rule 4).
                row.update(Trigger="Lottery", Conditions=[ruler, cond("CivHasTrait", tag="Brick.Form.Society"), vector],
                           Choices=[choice(), choice("Wait.", OnSuccess=[], SuccessText="{Civ} waited.")])
                for column, value in changes.items():
                    if column in ("Label", "SuccessText"):
                        row["Choices"][0][column] = value
                    else:
                        row[column] = value
            return change

        where = "DT_Encounters.petition_test_plea"
        self.assertEqual(self.after(crossroads()), [])
        refused_by_it_alone("DT_Encounters", card, crossroads(Title="언약의 길"), where, {"col": "Title", "lacks_words": "crossroads_words"})
        refused_by_it_alone("DT_Encounters", card, crossroads(Body="", VoiceBodies=["{Civ}의 의회는 안식을 청했다.", "{Civ}의 장로들은 길을 청했다."]), where,
                            {"col": "VoiceBodies", "lacks_words": "silence_words"})
        refused_by_it_alone("DT_Encounters", card, crossroads(Label="한 칸 위에 올라섰다."), where, {"col": "Choices.Label", "lacks_words": "rung_grasp_words"})
        refused_by_it_alone("DT_Encounters", card, crossroads(SuccessText="{Civ}는 면류관을 보았다."), where, {"col": "Choices.SuccessText", "lacks_words": "title_words"})
        self.assertEqual(self.after(self.set_cell("DT_Encounters", "petition_test_plea", "Title", "언약의 길")), [])
        # (4) An event row's lines: '순례' (not '방주' - ark_words refuses it on every line already; not '목자' - revelation_mimicry
        # holds '선한 목자') and '고요' (silence_words hangs elsewhere on the rows of endured pressure alone), under each condition.
        pilgrim = ["{Civ}의 배들이 순례를 떠났다.", "{Civ} two.", "{Civ} three."]
        hushed = ["{Civ}의 항구가 고요해졌다.", "{Civ} two.", "{Civ} three."]
        for why, condition in ((lived, vector), (bordering, cond("BorderingCivHasTrait", tag="Brick.Diet.StarEater"))):
            for templates, listed in ((pilgrim, "crossroads_words"), (hushed, "silence_words")):
                with self.subTest(why=why, listed=listed):
                    def change(tables, templates=templates, condition=condition):
                        self.row(tables, "DT_Events", "event_test_harvest").update(Conditions=[condition], LogTemplates=list(templates))
                    refused_by_it_alone("DT_Events", why, change, "DT_Events.event_test_harvest", {"col": "LogTemplates", "lacks_words": listed})
        self.assertEqual(self.after(self.set_cell("DT_Events", "event_test_harvest", "LogTemplates", list(pilgrim))), [])
        self.assertEqual(self.after(self.set_cell("DT_Events", "event_test_harvest", "LogTemplates", list(hushed))), [])
        # (5) The scan face: '성소' and '잠잠' - DT_LogTemplates.Templates refuses neither by its column.
        for word, listed in (("성소", "crossroads_words"), ("잠잠", "silence_words")):
            with self.subTest(scan=word):
                def change(tables, word=word):
                    row = self.row(tables, "DT_LogTemplates", "log_brick_scan_spread")
                    row["Templates"] = ["{System} 곁의 %s a." % word] + list(row["Templates"][1:])
                refused_by_it_alone("DT_LogTemplates", scan, change, "DT_LogTemplates.log_brick_scan_spread", {"col": "Templates", "lacks_words": listed})

    def test_crossroads_cards(self):
        """B7x [B-7x5c] (header section 3.3 rules 4 and 5; first drafted in [B-7x5a]): the breaking case the header names for
        each - a card whose every choice gives a brick (and a brick on a roll's failure), a crossroads petition that is
        Standing or never closes, a wanderer's crossroads card that is an arrival card - and the twelve cards of the project
        pass. Two pins of the project's rows beside them: the first life's row carries the start mark Brick.Form.Society, and
        choice 2 of a second crossroads petition carries no repeatable gain - a stability or resource change and the voice's
        Spiritualism tilt only (decision 11 (na) as the lead ruled it in [B-7x4]) - except the revert of
        petition_cross_wrapped_star, which closes the card by taking its parent away. From the review of [B-7x5c]: C10's
        choice 2 the stability change alone; how each crossroads petition closes, which the closing rule reads in part only;
        each member of rule 4's free choice broken alone, the check among them (CosmicRulerPolicy::IsNoValueChoice); a card
        that asks a time vector holds that choice too; and rule 4's two lists pinned to the ini's Brick. tags, as K4 is."""
        _, project_tables, _ = checker.load_project()
        crossroads = [row for row in project_tables["DT_Encounters"] if "_cross_" in row["Name"]]
        self.assertEqual(len(crossroads), 12)
        self.assertEqual(checker.check_project(), [])
        seeker = {row["Name"]: row for row in project_tables["DT_Civs"]}["arch_seeker"]
        self.assertIn("Brick.Form.Society", checker.TAG_NAME.findall(seeker["Traits"]))
        for row in crossroads:
            opens = any(entry["Type"] == "CivHasTrait" and entry["Tag"] == "Brick.Form.Society" for entry in row["Conditions"])
            if not row["Name"].startswith("petition_") or opens or row["Name"] == "petition_cross_wrapped_star":
                continue
            with self.subTest(second=row["Name"]):
                answer = row["Choices"][1]["OnSuccess"] + row["Choices"][1]["OnFailure"]
                self.assertLessEqual({entry["Type"] for entry in answer}, {"ChangeStability", "AddResource", "ShiftEthics"}, answer)
                self.assertLessEqual({entry["Key"] for entry in answer if entry["Type"] == "ShiftEthics"}, {"Spiritualism"}, answer)
        # C10's choice 2 keeps the stability change alone - no reverence, no rise and no Spiritualism tilt (decision 11 (na);
        # notes 5.4 data-2; design 4.3, C10), which the loop above lets through on every other second crossroads.
        law_answer = {row["Name"]: row for row in crossroads}["petition_cross_law_stays"]["Choices"][1]
        self.assertEqual([entry["Type"] for entry in law_answer["OnSuccess"] + law_answer["OnFailure"]], ["ChangeStability"], law_answer)
        # How a crossroads petition closes (header 3.1, 3.3 rule 5; design 4.3), which the closing rule reads in part only - it
        # skips a first crossroads, and any CivLacksTrait answers it. A first crossroads - one that asks the first life's mark
        # Brick.Form.Society - takes the mark away with every choice but the no-value one (a shape or the half step). And the
        # first choice of every crossroads petition, and every choice of it that gives or takes a brick on success, closes the
        # card: it gives a brick the conditions lack or takes away one they ask (petition_cross_seam does both; the revert of
        # petition_cross_wrapped_star takes away the ClosedPort it asks).
        def no_value(entry):
            return (all(amount == 0 for amount in entry["Cost"].values()) and entry["CheckStat"] == "NoCheck" and not entry["OnSuccess"]
                    and not entry["OnFailure"] and checker.name_of(entry["NextEncounter"]) == "")

        def moved(entry, type_name):
            return {effect["Tag"] for effect in entry["OnSuccess"] if effect["Type"] == type_name and effect["Tag"].startswith("Brick.")}

        for row in crossroads:
            if not row["Name"].startswith("petition_"):
                continue
            asks = {entry["Tag"] for entry in row["Conditions"] if entry["Type"] == "CivHasTrait"}
            lacks = {entry["Tag"] for entry in row["Conditions"] if entry["Type"] == "CivLacksTrait"}
            for index, entry in enumerate(row["Choices"]):
                with self.subTest(closes=row["Name"], choice=index + 1):
                    gives, takes = moved(entry, "AddTrait"), moved(entry, "RemoveTrait")
                    if "Brick.Form.Society" in asks and not no_value(entry):
                        self.assertIn("Brick.Form.Society", takes, entry)
                    if index == 0 or gives or takes:
                        self.assertTrue(gives & lacks or takes & asks, entry)
        plea = "petition_test_plea"
        update_plea = lambda **changes: (lambda tables: self.row(tables, "DT_Encounters", plea).update(**changes))  # noqa: E731
        give = lambda tag, label: choice(label, Cost=pool(Matter=10.0), OnSuccess=[eff("AddTrait", tag=tag)], SuccessText="{Civ} changed.")  # noqa: E731
        # Rule 4: three choices, each gives a brick - no free choice.
        every = [give("Brick.Space.Nomad", "Go."), give("Brick.Instinct.SwarmTech", "Build."), give("Brick.Diet.DarkMatter", "Sift.")]
        self.expect(self.after(update_plea(Choices=every)), "DT_Encounters.%s" % plea, "a card that gives or takes a brick holds a choice")
        self.assertEqual(self.after(update_plea(Choices=every[:2] + [choice("Watch.", OnSuccess=[], SuccessText="{Civ} watched.")])), [])
        # A free choice is free of effects too - and of a cost, a check, an effect on a roll's failure and a next card
        # (CosmicRulerPolicy::IsNoValueChoice), each member of the rule's 'none' list broken alone below.
        self.expect(self.after(update_plea(Choices=every[:2] + [choice("Watch.", SuccessText="{Civ} watched.")])), "a card that gives or takes a brick holds a choice")
        # The failure's effect stands on a choice with no check, so that it alone takes the card's last free choice away.
        for member, changes in (("Cost", dict(Cost=pool(Matter=1.0))), ("CheckStat", dict(CheckStat="Stability", Difficulty=8, FailureText="{Civ} looked away.")),
                                ("OnFailure", dict(OnFailure=[eff("ChangeStability", value=-0.01)], FailureText="{Civ} looked away.")),
                                ("NextEncounter", dict(NextEncounter=plea))):
            with self.subTest(free_of=member):
                watch = choice("Watch.", OnSuccess=[], SuccessText="{Civ} watched.", **changes)
                self.expect(self.after(update_plea(Choices=every[:2] + [watch])), "DT_Encounters.%s" % plea, "a card that gives or takes a brick holds a choice")
        # Rule 4: a brick on the failure of a roll, taken or given.
        rolled = choice("Try.", CheckStat="Stability", Difficulty=8, OnSuccess=[], OnFailure=[eff("RemoveTrait", tag="Brick.Form.Society")],
                        SuccessText="{Civ} tried.", FailureText="{Civ} failed.")
        self.expect(self.after(update_plea(Choices=[rolled, choice("Watch.", OnSuccess=[], SuccessText="{Civ} watched.")])),
                    "DT_Encounters.%s" % plea, "never on its failure", "Brick.Form.Society")
        # Rule 5: a crossroads petition is a Lottery row and waits on the first life's mark or on a lack.
        # Each with the no-value choice a card that asks a time vector holds (rule 4, below).
        free = [choice(), choice("Wait.", OnSuccess=[], SuccessText="{Civ} waited.")]
        first = [cond("PlayerRoleIs", key="Ruler"), cond("CivHasTrait", tag="Brick.Form.Society"), cond("CivTimeVectorAtLeast", key="Built", value=1200)]
        self.assertEqual(self.after(update_plea(Trigger="Lottery", Conditions=first, Choices=free)), [])
        self.expect(self.after(update_plea(Trigger="Standing", Conditions=first, Choices=free)), "DT_Encounters.%s" % plea, "comes in the periodic slot")
        open_ended = [cond("PlayerRoleIs", key="Ruler"), cond("CivHasTrait", tag="Brick.Instinct.SwarmTech"), cond("CivTimeVectorAtLeast", key="Linked", value=1800)]
        self.expect(self.after(update_plea(Trigger="Lottery", Conditions=open_ended, Choices=free)), "DT_Encounters.%s" % plea, "closes on its branch")
        self.assertEqual(self.after(update_plea(Trigger="Lottery", Conditions=open_ended + [cond("CivLacksTrait", tag="Brick.Instinct.HiveMerge")], Choices=free)), [])
        # Rule 4: a card that asks a time vector holds the no-value choice too, whether or not it moves a brick - the policies
        # count it a crossroads card by that condition alone (CosmicRulerPolicy::IsCrossroadsCard). The plea's own two
        # choices each carry an effect; a would-be free choice rolled for is none (the plea's 'Wait.' with a check - the shape
        # of the review's probe on petition_cross_built's choice 3); a wanderer's card is held as a petition is
        # (enc_test_weave's free choice rolled for, below).
        vector_card = "a card that asks a time vector holds a choice"
        self.expect(self.after(update_plea(Trigger="Lottery", Conditions=first)), "DT_Encounters.%s" % plea, vector_card)
        rolled_wait = [choice(), choice("Wait.", OnSuccess=[], CheckStat="Stability", Difficulty=8, SuccessText="{Civ} waited.", FailureText="{Civ} looked away.")]
        self.expect(self.after(update_plea(Trigger="Lottery", Conditions=first, Choices=rolled_wait)), "DT_Encounters.%s" % plea, vector_card)
        # Rule 5: a wanderer's crossroads card is Standing, never an arrival card.
        weave = lambda trigger: (lambda tables: self.row(tables, "DT_Encounters", "enc_test_weave").update(  # noqa: E731
            Trigger=trigger, Conditions=[cond("PlayerRoleIs", key="Wanderer"), cond("CivTimeVectorAtLeast", key="Drifted", value=900)]))
        self.expect(self.after(weave("Lottery")), "DT_Encounters.enc_test_weave", "a wanderer's crossroads card")
        self.assertEqual(self.after(weave("Standing")), [])

        def rolled_push(tables):
            weave("Standing")(tables)
            self.row(tables, "DT_Encounters", "enc_test_weave")["Choices"][1].update(CheckStat="Body", Difficulty=4, FailureText="{System} at last.")
        self.expect(self.after(rolled_push), "DT_Encounters.enc_test_weave", vector_card)
        # (Pinned last, so that a test taken out fails its case above first.) Rule 4's two lists name every Brick. tag of the
        # ini, given and taken, as K4 names them for rule 7 (header 3.3 rule 4: a new brick tag is added to the three rules
        # together): a tag left out, named twice or put in another's place passes every case above.
        failure_why = "a brick is given or taken by a choice that is not rolled for, or on the success of a roll - never on its failure: a failed roll changes no people's shape (B7x header section 3.3 rule 4)"
        free_why = "a card that gives or takes a brick holds a choice with no cost, no check, no effect and no next card: the measuring policies skip every choice that gives or takes a brick and take that one (CosmicRulerPolicy::IsNoValueChoice), so with one waiting place a card whose every choice did would block every card behind it - said the other way round: a card without such a choice gives and takes no brick (B7x header section 3.3 rule 4, section 5)"
        rules = {rule["why"]: rule for rule in self.schema["tables"]["DT_Encounters"]["rules"]}
        bricks = sorted(tag for tag in self.tags if tag.startswith("Brick.") and tag.count(".") == 2)
        self.assertEqual(len(bricks), 17)
        self.assertNotIn("when", rules[failure_why], "rule 4's failure side holds for every row, on every path")
        for why, column in ((failure_why, "Choices.OnFailure"), (free_why, "Choices.OnSuccess")):
            with self.subTest(pinned=column):
                self.assertEqual(rules[why]["then"], [{"col": column, "lacks_cond": {"type": type_name, "tag": tag}}
                                                      for type_name in ("AddTrait", "RemoveTrait") for tag in bricks])
        # The time vector card's free choice is the free-choice rule's, member for member: the free_of cases above break
        # each member of that list, and this pin carries them to the sibling rule's list.
        vector_why = 'a card that asks a time vector holds a choice with no cost, no check, no effect and no next card too: the measuring policies count it a crossroads card by that condition alone (CosmicRulerPolicy::IsCrossroadsCard) and take that choice, so a card without one would wait in the one waiting place and block every card behind it - said the other way round: a card without such a choice asks no time vector (B7x header section 3.3 rule 4, section 5)'
        self.assertEqual(rules[vector_why]["when"], [{"col": "Conditions", "has_type": "CivTimeVectorAtLeast"}] + rules[free_why]["when"])

    def test_crossroads_brick_lines(self):
        """B7x [B-7x5c] (header 3.5; the text audit's 7.3 and F12; notes 5.4 data-3): the three brick lines no condition rule
        reaches - event_brick_shut_star and event_brick_drift_plenty wait on traits alone, event_brick_ember_wake on a trait
        and a hazard - take row rules by their names, silence_words and crossroads_words on their lines. The rules pinned by
        hand; the project's rows ask neither condition of the [B-7x4] rules; each list broken on each row and refused by that
        rule alone - the same input passes with the rule taken out (the fixture rows ask no time vector and no bordering
        trait)."""
        self.assertEqual(self.after(), [])
        named = {
            "event_brick_shut_star": "the brick line of the shut sphere waits on traits alone, so it is named: no rest or silence, none of the fenced words of the crossroads (B7x header 3.5)",
            "event_brick_drift_plenty": "the brick line of the wandering swarm's plenty waits on traits alone, so it is named: no rest or silence, none of the fenced words of the crossroads (B7x header 3.5)",
            "event_brick_ember_wake": "the brick line of the star eater's trembling star waits on a trait and a hazard, so it is named: no rest or silence, none of the fenced words of the crossroads (B7x header 3.5)",
        }
        lines = [{"col": "LogTemplates", "lacks_words": "silence_words"}, {"col": "LogTemplates", "lacks_words": "crossroads_words"}]
        rules = {rule["why"]: rule for rule in self.schema["tables"]["DT_Events"].get("rules", [])}
        _, project_tables, _ = checker.load_project()
        project = {row["Name"]: row for row in project_tables["DT_Events"]}
        for row_name, why in named.items():
            with self.subTest(pin=row_name):
                self.assertEqual(rules[why]["when"], [{"row": row_name}])
                self.assertEqual(rules[why]["then"], lines)
                self.assertFalse({entry["Type"] for entry in project[row_name]["Conditions"]} & {"CivTimeVectorAtLeast", "BorderingCivHasTrait"})

        def without(why):
            def change(schema):
                schema["tables"]["DT_Events"]["rules"] = [rule for rule in schema["tables"]["DT_Events"]["rules"] if rule["why"] != why]
            return change

        for row_name, why in named.items():
            for line, listed in (("{Civ}의 배들이 순례를 떠났다.", "crossroads_words"), ("{Civ}의 항구가 고요해졌다.", "silence_words")):
                with self.subTest(row=row_name, listed=listed):
                    change = self.set_cell("DT_Events", row_name, "LogTemplates", [line, "{Civ} two.", "{Civ} three."])
                    self.expect(self.after(change), "DT_Events.%s" % row_name, why, json.dumps({"col": "LogTemplates", "lacks_words": listed}))
                    self.assertEqual(self.after(change, without(why)), [], "the input has to pass without the rule: %s" % listed)

    def test_per_tick_column(self):
        def per_tick(*items):
            return self.after(self.set_cell("DT_Transcendents", "trans_silence", "PressureEffectsPerTick", list(items)))
        self.expect(per_tick(eff("NameNotable", tag="Notable.Rebel")), "a per-tick source applies only AddResource")
        self.expect(per_tick(eff("KillPlayer", tag="Death.Transcendent")), "a per-tick source applies only")
        self.assertEqual(per_tick(eff("MultiplyIncome", value=0.9, duration=10)), [])
        self.expect(per_tick(eff("MultiplyIncome", value=-0.2)), "is below 0")

    def test_upper_rung_vocabulary(self):
        """B6b [B-6b2a] (header 3.3, rules 1-4, 6 and 6'): the three conditions appended to the vocabulary, the selector word
        Ruled (SystemOwnedBy alone), the Key Borders of ChangeReverence (never per tick), the scope of an edict and the
        galaxy law's rule, and the two feat rules (no sealed transcendent on a ruler row, no Tag on a feat's treaty). One
        input that breaks each, beside the walk of every rule below."""
        self.assertEqual(self.after(), [])
        # (1) CivHasTreaty: a Treaty.* Tag that may be left out, a whole number of partners from 1.
        self.assertEqual(self.conditions(cond("CivHasTreaty", value=1)), [])
        self.assertEqual(self.conditions(cond("CivHasTreaty", tag="Treaty.NonAggression", value=2)), [])
        self.expect(self.conditions(cond("CivHasTreaty", value=0)), "(CivHasTreaty).Value", "is below 1")
        self.expect(self.conditions(cond("CivHasTreaty", value=1.5)), "(CivHasTreaty).Value", "not a whole number")
        self.expect(self.conditions(cond("CivHasTreaty", tag="Stance.Friendly", value=1)), "(CivHasTreaty).Tag", "not under Treaty.")
        self.expect(self.conditions(cond("CivHasTreaty", key="civ_1", value=1)), "(CivHasTreaty).Key", "reads no Key")
        # CivLawsHeldTicksAtLeast: a count of ticks, no Value.
        self.assertEqual(self.conditions(cond("CivLawsHeldTicksAtLeast", key="600")), [])
        self.expect(self.conditions(cond("CivLawsHeldTicksAtLeast", key="N")), "(CivLawsHeldTicksAtLeast).Key", "not a count of ticks")
        self.expect(self.conditions(cond("CivLawsHeldTicksAtLeast")), "(CivLawsHeldTicksAtLeast).Key", "a Key is required")
        self.expect(self.conditions(cond("CivLawsHeldTicksAtLeast", key="600", value=3)), "(CivLawsHeldTicksAtLeast)", "reads no Value")
        # CivPressureEnduredTicksReached: a count of ticks, no Tag; a Civilization slot row only, as the fallen-sphere window.
        endured = cond("CivPressureEnduredTicksReached", key="600")
        harvest = lambda **changes: (lambda tables: self.row(tables, "DT_Events", "event_test_harvest").update(changes))  # noqa: E731
        # In the era's shape ([B-6b8], decision 22): the ruled people and the Borders reverence.
        self.assertEqual(self.after(harvest(Trigger="Standing", BaseWeight=0, Conditions=[cond("SystemOwnedBy", key="Ruled"), endured],
                                            Effects=[eff("ChangeReverence", key="Borders", value=0.2)])), [])
        self.expect(self.conditions(cond("CivPressureEnduredTicksReached", key="600ticks")), "(CivPressureEnduredTicksReached).Key", "not a count of ticks")
        self.expect(self.conditions(cond("CivPressureEnduredTicksReached", key="600", tag="Doctrine.Silence")), "(CivPressureEnduredTicksReached).Tag", "reads no Tag")
        self.expect(self.after(harvest(Trigger="Named", BaseWeight=0, CooldownTicks=0, LogTemplates=["one."], Conditions=[endured])),
                    "a row fired by name is not a slot", "CivPressureEnduredTicksReached")
        self.expect(self.after(harvest(Subject="SettledSystem", Conditions=[endured])), "a system-subject row lands on a place", "CivPressureEnduredTicksReached")
        self.expect(self.after(self.set_cell("DT_Hazards", "crisis_test_storm", "SpawnConditions", [endured])),
                    "the stage reads the spawn conditions on every tick", "CivPressureEnduredTicksReached")
        # (2) The selector word Ruled: SystemOwnedBy takes the civ selector, the two notable conditions Former alone.
        self.assertEqual(self.conditions(cond("SystemOwnedBy"), cond("SystemOwnedBy", key="Ruled"), cond("SystemOwnedBy", key="Former")), [])
        self.expect(self.conditions(cond("SystemOwnedBy", key="Latter")), "(SystemOwnedBy).Key", "Latter is none of Former, Ruled")
        self.expect(self.conditions(cond("CivHasNotable", tag="Notable.Rebel", key="Ruled")), "(CivHasNotable).Key", "Ruled is none of Former")
        self.expect(self.effects(eff("NameNotable", tag="Notable.Rebel", key="Ruled")), "(NameNotable).Key", "Ruled is none of Former")
        self.assertEqual(self.effects(eff("NameNotable", tag="Notable.Rebel", key="Former")), [])
        # (3) The Key Borders of ChangeReverence: empty or Borders, and a per-tick source leaves it empty.
        self.assertEqual(self.effects(eff("ChangeReverence", key="Borders", value=0.2), eff("ChangeReverence", value=0.1)), [])
        self.expect(self.effects(eff("ChangeReverence", key="Neighbours", value=0.2)), "(ChangeReverence).Key", "Neighbours is none of Borders")
        self.assertEqual(self.after(self.set_cell("DT_Edicts", "edict_test_order", "EffectsPerTick", [eff("ChangeReverence", value=0.001)])), [])
        self.expect(self.after(self.set_cell("DT_Edicts", "edict_test_order", "EffectsPerTick", [eff("ChangeReverence", key="Borders", value=0.001)])),
                    "DT_Edicts.edict_test_order.EffectsPerTick[0](ChangeReverence).Key", "a per-tick source leaves this Key empty")
        self.expect(self.after(self.set_cell("DT_Transcendents", "trans_silence", "PressureEffectsPerTick", [eff("ChangeReverence", key="Borders", value=-0.001)])),
                    "(ChangeReverence).Key", "a per-tick source leaves this Key empty")
        self.schema_problem(self.put(["conditions", "CivLawsHeldTicksAtLeast", "key"], "not_per_tick", True), "schema.conditions.CivLawsHeldTicksAtLeast.key",
                            "not_per_tick is said of an effect's Key")
        self.schema_problem(self.put(["effects", "ChangeReverence", "key"], "not_per_tick", "yes"), "schema.effects.ChangeReverence.key", "not_per_tick")
        # (4) The scope of an edict: one of the enum (no empty value), and a galaxy law of the fourth rung that only shields.
        def law(**changes):
            cells = dict(Scope="HomeGalaxy", RequiredTier="Tier4_Transcendent", HazardSpawnMultipliers={"Hazard.Plague": 0.5},
                         HazardCountdownBonus={"Hazard.Plague": 10})
            cells.update(changes)
            return lambda tables: self.row(tables, "DT_Edicts", "edict_test_order").update(cells)
        galaxy_law = "a galaxy law is an edict of the fourth rung"
        self.assertEqual(self.after(law()), [])
        self.assertEqual(self.after(law(HazardSpawnMultipliers={}, HazardCountdownBonus={})), [])
        self.expect(self.after(self.set_cell("DT_Edicts", "edict_test_order", "Scope", "Galaxy")), "DT_Edicts.edict_test_order.Scope", "is no enumerator of ECosmicEdictScope")
        self.expect(self.after(self.set_cell("DT_Edicts", "edict_test_order", "Scope", "")), "DT_Edicts.edict_test_order.Scope", "is no enumerator of ECosmicEdictScope")
        self.expect(self.after(law(RequiredTier="Tier3_Causal")), "edict_test_order", galaxy_law, '"RequiredTier"')
        self.expect(self.after(law(HazardSpawnMultipliers={"Hazard.Plague": 1.5})), "edict_test_order", galaxy_law, '"le": 1')
        self.assertEqual(self.after(law(HazardSpawnMultipliers={"Hazard.Plague": 1.0})), [])
        self.expect(self.after(law(HazardCountdownBonus={"Hazard.Claim": 100})), "edict_test_order", galaxy_law, '"HazardCountdownBonus"', "Hazard.Claim")
        for tag in ("Hazard.Claim", "Hazard.Extinction", "Hazard.PreSupernova"):
            with self.subTest(tag=tag):
                self.expect(self.after(law(HazardSpawnMultipliers={tag: 0.5})), "edict_test_order", galaxy_law, '"HazardSpawnMultipliers", "contains_none"')
        # A row of the people's scope keeps the hooks of B2: a factor above 1, a claim's bonus.
        self.assertEqual(self.after(law(Scope="People", RequiredTier="Tier1_Planetary", HazardSpawnMultipliers={"Hazard.Plague": 2.0},
                                        HazardCountdownBonus={"Hazard.Claim": 100})), [])
        # (6) No ruler feat reads a sealed transcendent; (6') a feat's treaty carries no Tag (an event row's may).
        feat = lambda conditions: self.set_cell("DT_Feats", "feat_test_climb", "CompletionConditions", conditions)  # noqa: E731
        self.expect(self.after(feat([cond("TranscendentSealed")])), "feat_test_climb", "a ruler row never reads a sealed transcendent")
        self.assertEqual(self.after(feat([cond("CivHasTreaty", value=1)])), [])
        self.expect(self.after(feat([cond("CivHasTreaty", tag="Treaty.Trade", value=1)])), "DT_Feats.feat_test_climb.CompletionConditions[0](CivHasTreaty).Tag",
                    "this column's CivHasTreaty carries no Tag")
        self.assertEqual(self.conditions(cond("CivHasTreaty", tag="Treaty.Trade", value=1)), [])
        # A ruler feat names no selector word of the vessel (A3): Ruled or Former of SystemOwnedBy, Former of CivHasNotable; the
        # empty Key (the context people's own system) stays.
        self.assertEqual(self.after(feat([cond("SystemOwnedBy")])), [])
        selector_rule = "a ruler row names no selector word of the vessel either"
        self.expect(self.after(feat([cond("SystemOwnedBy", key="Ruled")])), "feat_test_climb", selector_rule, '"key": "Ruled"')
        self.expect(self.after(feat([cond("SystemOwnedBy", key="Former")])), "feat_test_climb", selector_rule, '"key": "Former"')
        self.expect(self.after(feat([cond("CivHasNotable", tag="Notable.Rebel", key="Former")])), "feat_test_climb", selector_rule, "CivHasNotable")
        # Nor its price: every people's climb pays it, and NameNotable Former names a notable in the vessel's former people.
        self.expect(self.after(self.set_cell("DT_Feats", "feat_test_climb", "AscensionCost", [eff("NameNotable", tag="Notable.Hearer", key="Former")])),
                    "feat_test_climb", selector_rule, "NameNotable")
        # (8') A card on the claim neither ends it nor buys it time - the time is petition_pressure_measured's alone (the game's
        # rows keep both: the walk above starts from them).
        on_claim = [cond("PlayerRoleIs", key="Ruler"), cond("SystemHasHazard", tag="Hazard.Claim")]
        plea = lambda choices: (lambda tables: self.row(tables, "DT_Encounters", "petition_test_plea").update(Conditions=on_claim, Choices=choices))  # noqa: E731
        self.assertEqual(self.after(plea([choice(), choice("Wait.", SuccessText="{Civ} waited.")])), [])
        self.expect(self.after(plea([choice(OnSuccess=[eff("ResolveHazard", tag="Hazard.Claim")]), choice("Wait.", SuccessText="{Civ} waited.")])),
                    "petition_test_plea", "a card on the claim does not end it")
        self.expect(self.after(plea([choice(OnSuccess=[eff("DelayHazard", tag="Hazard.Claim", value=50)]), choice("Wait.", SuccessText="{Civ} waited.")])),
                    "petition_test_plea", "the claim's time is bought by one card alone")
        # Nor a card whose conditions do not name the claim (an untagged hazard condition stands on the claim too): no answer
        # resolves the claim, nor an untagged hazard, which ends whatever stands.
        any_hazard = [cond("PlayerRoleIs", key="Ruler"), cond("SystemHasHazard")]
        for answer in (eff("ResolveHazard", tag="Hazard.Claim"), eff("ResolveHazard")):
            for column in ("OnSuccess", "OnFailure"):
                with self.subTest(answer=answer["Tag"], column=column):
                    self.expect(self.after(lambda tables, answer=answer, column=column: self.row(tables, "DT_Encounters", "petition_test_plea").update(
                        Conditions=any_hazard, Choices=[choice(**{column: [answer]}), choice("Wait.", SuccessText="{Civ} waited.")])),
                                "petition_test_plea", "nor does any card, whatever its conditions")
        self.assertEqual(self.after(lambda tables: self.row(tables, "DT_Encounters", "petition_test_plea").update(
            Conditions=any_hazard, Choices=[choice(OnSuccess=[eff("ResolveHazard", tag="Hazard.Plague")]), choice("Wait.", SuccessText="{Civ} waited.")])), [])
        self.assertEqual(self.after(self.set_cell("DT_Encounters", "petition_test_plea", "Choices",
                                                  [choice(OnSuccess=[eff("DelayHazard", tag="Hazard.Plague", value=50)]), choice("Wait.", SuccessText="{Civ} waited.")])), [])
        self.schema_problem(self.put(["tables", "DT_Feats", "columns", "CompletionConditions"], "untagged", ["CivHasTreety"]),
                            "schema.DT_Feats.CompletionConditions", "untagged names 'CivHasTreety', which is no enumerator of conditions")
        self.schema_problem(self.put(["tables", "DT_Feats", "columns", "CompletionConditions"], "untagged", ["CivTierAtLeast"]),
                            "schema.DT_Feats.CompletionConditions", "untagged names CivTierAtLeast, a condition that reads no Tag")
        self.schema_problem(self.put(["tables", "DT_Feats", "columns", "CompletionConditions"], "untagged", "CivHasTreaty"),
                            "schema.DT_Feats.CompletionConditions", "untagged")
        self.schema_problem(self.put(["tables", "DT_Feats", "columns", "AscensionCost"], "untagged", ["CivHasTreaty"]),
                            "schema.DT_Feats.AscensionCost", "'untagged' is not read here")
        # A comparison over a tag map reads every value (an empty map holds); over a tag container it is no question.
        holds = checker.Checker(self.schema, {}, self.tags).holds
        le = lambda value: holds({"col": "C", "le": 1}, {"C": value}, "r")  # noqa: E731
        self.assertTrue(le({}))
        self.assertTrue(le({"Hazard.Plague": 0.5, "Hazard.Cult": 1.0}))
        self.assertFalse(le({"Hazard.Plague": 0.5, "Hazard.Cult": 1.5}))
        self.assertFalse(le({"Hazard.Plague": "0.5"}))
        self.assertTrue(holds({"col": "C", "gt": 0}, {"C": {"Hazard.Plague": 0.5}}, "r"))
        self.assertFalse(holds({"col": "C", "gt": 0}, {"C": {"Hazard.Plague": 0.0}}, "r"))
        self.schema_problem(self.put(["tables", "DT_Civs", "rules", 0], "when", [{"col": "Traits", "le": 1}]), "schema.DT_Civs",
                            "the test 'le' cannot be asked of a column of kind tags")
        self.assertEqual(self.after(NOTHING, self.put(["tables", "DT_Civs", "rules", 0], "when", [{"col": "HazardSpawnMultipliers", "le": 9}, {"col": "Weight", "gt": 0}])), [])

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
            ([], "epitaph_leaves_out", "petition_exalt", "'epitaph_leaves_out' has to be a list of strings"),
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
        self.schema_problem(put(["tables", "DT_Rules", "rules"], 1, {"why": "w", "then": [{"col": "ImmutableLaws", "contains_all": ["Law.Remnant", "Law.Hegde"]}]}),
                            "schema.DT_Rules", "names the tag Law.Hegde, which the ini does not declare")
        self.schema_problem(put(["tables", "DT_Transcendents", "rules"], 0, {"why": "w", "when": [{"row": "trans_silance"}], "then": []}),
                            "schema.DT_Transcendents", "names the row trans_silance")
        self.schema_problem(put(["tables", "DT_Rules"], "rules", [{"why": "w", "then": [{"col": "PlayerArchetypeRow", "is": "arch_teran"}]}]),
                            "schema.DT_Rules", "names the row arch_teran, which DT_Civs")
        def lost_table(schema):
            schema["tables"]["DT_Rules"]["columns"]["PlayerArchetypeRow"]["table"] = "DT_Peoples"
            schema["tables"]["DT_Rules"]["rules"] = [{"why": "w", "then": [{"col": "PlayerArchetypeRow", "is": "arch_seeker"}]}]
        self.schema_problem(lost_table, "schema.DT_Rules", "names the row arch_seeker, which DT_Peoples neither requires nor has")
        # A row the table has to have, a row its source has, and the empty name are names a rule may lean on.
        self.assertEqual(self.after(NOTHING, put(["tables", "DT_Rules"], "rules", [{"why": "w", "then": [{"col": "PlayerArchetypeRow", "in": ["arch_seeker", "arch_default"]}]}])), [])
        self.assertEqual(self.after(NOTHING, put(rule, "when", [{"col": "AIProfile", "is": ""}, {"col": "Weight", "gt": 0}])), [])

    def test_a_test_fits_the_kind_of_its_column(self):
        """Asked of another kind a test never holds, or always: 'is' over a container, a comparison over a name."""
        rule = ["tables", "DT_Civs", "rules", 0]
        cannot = "cannot be asked of a column of kind"
        for test, kind in (({"col": "Traits", "is": "Trait.Arch.Seeker"}, "tags"),
                           ({"col": "DedicationBanks", "in": ["Faith.Bank.Hebrew"]}, "tag_list"),
                           ({"col": "NameBank", "is": "union"}, "texts"),
                           ({"col": "InitialRelations", "is": 0}, "row_map"),
                           ({"col": "RevelationMode", "ge": 4}, "tag"),
                           ({"col": "DisplayName", "each_contains": "x"}, "text"),
                           ({"col": "DisplayName", "min_len": 1}, "text"),
                           ({"col": "Weight", "max_len": 0}, "number"),
                           ({"col": "NameBank", "lacks_type": "NameNotable"}, "texts"),
                           ({"col": "Weight", "slots_in": ["Star"]}, "number"),
                           ({"col": "Weight", "set": False}, "number"),
                           ({"col": "RevelationMode", "contains_all": ["Revelation.Book"]}, "tag"),
                           ({"col": "NameBank", "contains_all": ["union"]}, "texts"),
                           ({"col": "NameBank", "has_type": "PlayerRoleIs"}, "texts")):
            with self.subTest(test=test):
                self.schema_problem(self.put(rule, "when", [test]), "schema.DT_Civs", cannot, kind)
        # A list of row names: 'is' and 'in' over it never hold, so a rule asking them would be off for good.
        hazard_rule = ["tables", "DT_Hazards", "rules", 0]
        for test in ({"col": "RulerResolutions", "is": "project_relief"}, {"col": "RulerResolutions", "in": ["project_relief"]},
                     {"col": "RulerResolutions", "gt": 0}, {"col": "RulerResolutions", "each_contains": "x"}):
            with self.subTest(test=test):
                self.schema_problem(self.put(hazard_rule, "when", [test]), "schema.DT_Hazards", cannot, "rows")
        # Its size and whether it holds anything are fair questions (the rule's own then may fail on the fixture row:
        # only the schema is looked at here).
        for test in ({"col": "RulerResolutions", "min_len": 1}, {"col": "RulerResolutions", "max_len": 0}, {"col": "RulerResolutions", "set": True}):
            with self.subTest(test=test):
                said = [line for line in self.after(NOTHING, self.put(hazard_rule, "when", [test])) if line.startswith("schema.")]
                self.assertEqual(said, [])
        self.schema_problem(self.put(["tables", "DT_Events", "rules", 0], "when", [{"col": "Trigger", "gt": 1}]), "schema.DT_Events", cannot, "enum")
        # A number, an enumerator and a resource pool are always there: 'set' over them says nothing.
        self.schema_problem(self.put(["tables", "DT_Events", "rules", 0], "when", [{"col": "Trigger", "set": True}]), "schema.DT_Events", cannot, "enum")
        self.schema_problem(self.put(["tables", "DT_Megaprojects"], "rules", [{"why": "w", "when": [{"col": "CostPerTick", "set": True}], "then": []}]),
                            "schema.DT_Megaprojects", cannot, "pool")
        self.assertEqual(self.after(NOTHING, self.put(rule, "when", [{"col": "DisplayName", "set": True}, {"col": "Weight", "gt": 0}])), [])

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
                            ({"col": "NameBank", "slots_in": "Star"}, "the test 'slots_in' needs a list of strings"),
                            ({"col": "Traits", "contains_all": "Trait.Faith.Book"}, "the test 'contains_all' needs a list of strings, not empty"),
                            ({"col": "Traits", "contains_all": []}, "the test 'contains_all' needs a list of strings, not empty"),
                            ({"col": "Traits", "contains_all": ["Trait.Faith.Book", ""]}, "the test 'contains_all' needs a list of strings, not empty")):
            with self.subTest(test=test):
                self.schema_problem(self.put(rule, "when", [test]), "schema.DT_Civs", words)
        self.schema_problem(self.put(["tables", "DT_Events", "rules", 0], "then", [{"col": "Effects", "lacks_type": ["NameNotable"]}]),
                            "schema.DT_Events", "the test 'lacks_type' needs a string")
        self.schema_problem(self.put(["tables", "DT_Encounters", "rules", 0], "then", [{"col": "Conditions", "has_type": ["PlayerRoleIs"]}]),
                            "schema.DT_Encounters", "the test 'has_type' needs a string")
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
        # One row of each table: the shapes are those of the columns, and the cost must not grow with the rows.
        small = {table: rows[:1] for table, rows in self.tables.items()}
        for name, root in (("schema", self.schema), ("tables", small)):
            for path in paths_of(root):
                for value, delete in [(value, False) for value in wrong] + [(None, True)]:
                    changed = with_change(root, path, value, delete)
                    try:
                        problems = checker.check(changed, small, self.tags) if name == "schema" else checker.check(self.schema, changed, self.tags)
                    except Exception as error:  # noqa: BLE001  (any exception is the failure looked for)
                        self.fail("%s%r %s raised %r" % (name, path, "taken out" if delete else "= %r" % (value,), error))
                    self.assertIsInstance(problems, list)

    # ---- lists of structs and resource pools ----

    def test_a_list_of_structs(self):
        """DT_Encounters.Choices: each entry an object with exactly the columns of the list, each cell checked as a column
        of its kind (a member left out of a source would keep its C++ default in silence)."""
        plea = "petition_test_plea"
        choices = lambda value: self.set_cell("DT_Encounters", plea, "Choices", value)  # noqa: E731
        good = self.row(self.tables, "DT_Encounters", plea)["Choices"]
        self.assertEqual(self.after(choices(good + [choice("Third.")])), [])
        self.expect(self.after(choices(good[:1])), "DT_Encounters.petition_test_plea.Choices", "expected 2 or 3 entries, found 1")
        self.expect(self.after(choices(good + [choice("c."), choice("d.")])), "Choices", "expected 2 or 3 entries, found 4")
        self.expect(self.after(choices("two")), "Choices", "expected a list of objects")
        self.expect(self.after(choices([good[0], "x"])), "Choices[1]", "expected an object")
        missing = {key: value for key, value in good[0].items() if key != "Difficulty"}
        self.expect(self.after(choices([missing, good[1]])), "Choices[0].Difficulty", "the member is missing")
        self.expect(self.after(choices([dict(good[0], Difficulti=1), good[1]])), "Choices[0].Difficulti", "the struct has no member Difficulti")
        # Each cell as a column of its kind.
        entry = lambda **changes: choices([dict(good[0], **changes), good[1]])  # noqa: E731
        self.expect(self.after(entry(Label="")), "Choices[0].Label", "a text is required")
        self.expect(self.after(entry(Label="{Player} rose.")), "Choices[0].Label", "slot {Player} is not filled here")
        self.expect(self.after(entry(Label="영혼 a.")), "Choices[0].Label", "the word 영혼 is not used here")
        self.expect(self.after(entry(Label="3 a.")), "Choices[0].Label", "a digit")
        self.expect(self.after(entry(CheckStat="Influence")), "Choices[0].CheckStat", "'Influence' is no enumerator of ECosmicCheckStat")
        self.expect(self.after(entry(Difficulty=17)), "Choices[0].Difficulty", "is above 16")
        self.expect(self.after(entry(Difficulty=2.5)), "Choices[0].Difficulty", "not a whole number")
        self.expect(self.after(entry(OnFailure=[eff("ChangeStability", value=-3)])), "Choices[0].OnFailure[0](ChangeStability).Value", "is below -1")
        self.expect(self.after(entry(OnSuccess=[eff("AddTrait", tag="Trait.Nobody")])), "Choices[0].OnSuccess[0](AddTrait).Tag", "not declared")
        self.expect(self.after(entry(NextEncounter="petition_nobody")), "Choices[0].NextEncounter", "DT_Encounters has no row petition_nobody")
        self.assertEqual(self.after(entry(NextEncounter=plea)), [])
        # The schema side: a list of structs declares its columns, holds no list of structs, and its columns are columns.
        column = ["tables", "DT_Encounters", "columns", "Choices"]
        self.schema_problem(self.drop(column, "columns"), "schema.DT_Encounters.Choices", "'columns' is missing")
        self.schema_problem(self.put(column, "columns", []), "schema.DT_Encounters.Choices", "'columns' has to be an object")
        self.schema_problem(self.put(column, "len_in", 3), "schema.DT_Encounters.Choices", "'len_in' has to be a list of whole numbers")
        self.schema_problem(self.put(column, "slots", ["Civ"]), "schema.DT_Encounters.Choices", "'slots' is not read here")
        self.schema_problem(self.put(column + ["columns"], "Inner", {"kind": "structs", "columns": {}}), "schema.DT_Encounters.Choices.Inner", "a list of structs holds no list of structs")
        self.schema_problem(self.put(column + ["columns"], "Label", {"kind": "riddle"}), "schema.DT_Encounters.Choices.Label", "a kind this checker does not know")
        self.schema_problem(self.put(column + ["columns", "NextEncounter"], "table", "DT_Nowhere"), "schema.DT_Encounters.Choices.NextEncounter", "the table 'DT_Nowhere' is not in the schema")
        self.schema_problem(self.put(column + ["columns", "Label"], "forbidden", "verdicts"), "schema.DT_Encounters.Choices.Label", "the word list 'verdicts' is not in the schema")
        self.schema_problem(self.put(column + ["columns", "CheckStat"], "enum", "ECheck"), "schema.DT_Encounters.Choices.CheckStat", "the enum 'ECheck' is not in the schema")

    def test_a_resource_pool(self):
        """A resource pool (the cost of a choice, the cost per tick of a project): every resource as a number, no other
        key, none below the least the column states."""
        build = lambda value: self.set_cell("DT_Megaprojects", "project_test_build", "CostPerTick", value)  # noqa: E731
        full = {"Energy": 1.0, "Matter": 0.0, "Insight": 0.0, "Influence": 0.0, "Causality": 0.0}
        self.assertEqual(self.after(build(full)), [])
        self.expect(self.after(build(dict(full, Matter=-1.0))), "project_test_build.CostPerTick.Matter", "is below 0")
        self.expect(self.after(build({key: value for key, value in full.items() if key != "Causality"})), "CostPerTick.Causality", "the member is missing")
        self.expect(self.after(build(dict(full, Gold=1.0))), "CostPerTick.Gold", "a resource pool has no member Gold")
        self.expect(self.after(build(dict(full, Energy="1"))), "CostPerTick.Energy", "finite number")
        self.expect(self.after(build([1, 0, 0, 0, 0])), "CostPerTick", "expected an object with a number for each of Energy")
        good = self.row(self.tables, "DT_Encounters", "petition_test_plea")["Choices"]
        priced = [dict(good[0], Cost=dict(full, Influence=-5.0)), good[1]]
        self.expect(self.after(self.set_cell("DT_Encounters", "petition_test_plea", "Choices", priced)), "Choices[0].Cost.Influence", "is below 0")
        self.schema_problem(self.put(["tables", "DT_Megaprojects", "columns", "CostPerTick"], "max", 5), "schema.DT_Megaprojects.CostPerTick", "'max' is not read here")
        self.schema_problem(self.drop(["words"], "resource"), "a resource pool holds the words of the list 'resource'")

    def test_a_rule_reaches_into_a_list_of_structs(self):
        rules = ["tables", "DT_Encounters", "rules"]
        rule = lambda *tests: self.put(rules, 0, {"why": "w", "then": list(tests)})  # noqa: E731
        self.schema_problem(rule({"col": "Choices.Lable", "set": True}), "schema.DT_Encounters", "names the column 'Lable', which the list Choices of DT_Encounters does not declare")
        self.schema_problem(rule({"col": "Choises.Label", "set": True}), "schema.DT_Encounters", "names the column 'Choises', which DT_Encounters does not declare")
        self.schema_problem(rule({"col": "Title.Label", "set": True}), "schema.DT_Encounters", "names 'Title.Label', and Title is no list of structs")
        self.schema_problem(rule({"col": "Choices.Label", "len_ge_col": "BaseWeight"}), "schema.DT_Encounters", "the test 'len_ge_col' cannot be asked of a column of a list of structs")
        self.schema_problem(rule({"col": "Choices.Difficulty", "set": True}), "schema.DT_Encounters", "cannot be asked of a column of kind number")
        self.schema_problem(rule({"col": "Choices.CheckStat", "is": "Influence"}), "schema.DT_Encounters", "'Influence', which is no enumerator of ECosmicCheckStat")
        self.assertEqual(self.after(NOTHING, rule({"col": "Choices.CheckStat", "in": ["NoCheck", "Stability", "Body"]})), [])
        # The test holds for every entry: one entry that breaks it breaks the rule, and an empty list holds it.
        holds = checker.Checker(self.schema, self.tables, self.tags).holds
        test = {"col": "Choices.Difficulty", "le": 5}
        self.assertTrue(holds(test, {"Choices": [{"Difficulty": 1}, {"Difficulty": 5}]}, "r"))
        self.assertFalse(holds(test, {"Choices": [{"Difficulty": 1}, {"Difficulty": 6}]}, "r"))
        self.assertTrue(holds(test, {"Choices": []}, "r"))
        for wrong in ({"Choices": "x"}, {"Choices": [5]}, {}):
            with self.subTest(row=wrong):
                self.assertFalse(holds(test, wrong, "r"))
        # In the data: a then over a column of the list, broken in the second entry only.
        good = self.row(self.tables, "DT_Encounters", "petition_test_plea")["Choices"]
        second = self.set_cell("DT_Encounters", "petition_test_plea", "Choices", [good[0], dict(good[1], OnFailure=[eff("KillPlayer", tag="Death.Encounter")])])
        self.expect(self.after(second), "DT_Encounters.petition_test_plea", "a card records no death", "Choices.OnFailure")

    def test_has_type(self):
        holds = checker.Checker(self.schema, self.tables, self.tags).holds
        test = {"col": "C", "has_type": "PlayerRoleIs"}
        self.assertTrue(holds(test, {"C": [cond("GalaxyTickAtLeast"), cond("PlayerRoleIs", key="Ruler")]}, "r"))
        for wrong in ([cond("GalaxyTickAtLeast")], [], "PlayerRoleIs", [5], None):
            with self.subTest(cell=wrong):
                self.assertFalse(holds(test, {"C": wrong}, "r"))
        self.expect(self.after(self.set_cell("DT_Encounters", "petition_test_plea", "Conditions", [cond("GalaxyTickAtLeast", value=30)])),
                    "DT_Encounters.petition_test_plea", "a petition is put before a ruler")
        self.schema_problem(self.put(["tables", "DT_Encounters", "rules", 0], "then", [{"col": "Conditions", "has_type": "PlayerRoleIz"}]),
                            "schema.DT_Encounters", "'PlayerRoleIz', which is no enumerator of conditions")
        self.schema_problem(self.put(["tables", "DT_Encounters", "rules", 0], "then", [{"col": "Conditions", "has_type": "SetFlag"}]),
                            "schema.DT_Encounters", "'SetFlag', which is no enumerator of conditions")

    def test_has_cond(self):
        # B1: the role a card is put before - the enumerator and its Key together.
        holds = checker.Checker(self.schema, self.tables, self.tags).holds
        test = {"col": "C", "has_cond": {"type": "PlayerRoleIs", "key": "Wanderer"}}
        self.assertTrue(holds(test, {"C": [cond("GalaxyTickAtLeast"), cond("PlayerRoleIs", key="Wanderer")]}, "r"))
        for wrong in ([cond("PlayerRoleIs", key="Ruler")], [cond("GalaxyTickAtLeast", key="Wanderer")], [], None):
            with self.subTest(cell=wrong):
                self.assertFalse(holds(test, {"C": wrong}, "r"))
        self.expect(self.after(self.set_cell("DT_Encounters", "enc_test_weave", "Conditions", [cond("PlayerRoleIs", key="Ruler")])),
                    "DT_Encounters.enc_test_weave", "a wanderer's card is put before a wanderer")
        self.expect(self.after(self.set_cell("DT_Encounters", "petition_test_plea", "Conditions", [cond("PlayerRoleIs", key="Wanderer")])),
                    "DT_Encounters.petition_test_plea", "a petition is put before a ruler")
        rule = ["tables", "DT_Encounters", "rules", 0]
        self.schema_problem(self.put(rule, "then", [{"col": "Conditions", "has_cond": {"type": "PlayerRoleIz", "key": "Ruler"}}]),
                            "schema.DT_Encounters", "'PlayerRoleIz', which is no enumerator of conditions")
        self.schema_problem(self.put(rule, "then", [{"col": "Conditions", "has_cond": {"type": "PlayerRoleIs"}}]),
                            "schema.DT_Encounters", "the test 'has_cond' needs an object of the strings type, key and tag with a key or a tag")
        self.schema_problem(self.put(rule, "then", [{"col": "Body", "has_cond": {"type": "PlayerRoleIs", "key": "Ruler"}}]),
                            "schema.DT_Encounters", "the test 'has_cond' cannot be asked of a column of kind text")

    def test_lacks_cond(self):
        # B6b [B-6b2a]: the negation of has_cond, and the Tag as a field of both (the type may then be left out).
        holds = checker.Checker(self.schema, self.tables, self.tags).holds
        lacks = {"col": "C", "lacks_cond": {"type": "SystemOwnedBy", "key": "Ruled"}}
        for fine in ([cond("SystemOwnedBy"), cond("SystemOwnedBy", key="Former"), cond("CivHasNotable", key="Ruled")], []):
            with self.subTest(cell=fine):
                self.assertTrue(holds(lacks, {"C": fine}, "r"))
        for wrong in ([cond("GalaxyTickAtLeast"), cond("SystemOwnedBy", key="Ruled")], None, "SystemOwnedBy"):
            with self.subTest(cell=wrong):
                self.assertFalse(holds(lacks, {"C": wrong}, "r"))
        on_claim = {"col": "C", "has_cond": {"tag": "Hazard.Claim"}}
        self.assertTrue(holds(on_claim, {"C": [cond("PlayerRoleIs", key="Wanderer"), cond("PlayerInSystemWithHazard", tag="Hazard.Claim")]}, "r"))
        self.assertTrue(holds(on_claim, {"C": [cond("SystemHasHazard", tag="Hazard.Claim")]}, "r"))
        self.assertFalse(holds(on_claim, {"C": [cond("SystemHasHazard", tag="Hazard.Plague"), cond("SystemHasHazard")]}, "r"))
        delay = {"col": "C", "lacks_cond": {"type": "DelayHazard", "tag": "Hazard.Claim"}}
        self.assertTrue(holds(delay, {"C": [eff("DelayHazard", tag="Hazard.Plague", value=50), eff("ResolveHazard", tag="Hazard.Claim")]}, "r"))
        self.assertFalse(holds(delay, {"C": [eff("DelayHazard", tag="Hazard.Claim", value=50)]}, "r"))
        # A field is read as the importer reads a name: the empty Key the data writes 'None' is the empty Key of an operand, and
        # an empty Tag names the untagged entry (the cards' side of rule 8': no untagged ResolveHazard).
        self.assertTrue(holds({"col": "C", "has_cond": {"type": "SystemOwnedBy", "key": ""}}, {"C": [cond("SystemOwnedBy", key="None")]}, "r"))
        untagged = {"col": "C", "lacks_cond": {"type": "ResolveHazard", "tag": ""}}
        self.assertTrue(holds(untagged, {"C": [eff("ResolveHazard", tag="Hazard.Plague"), eff("DelayHazard", value=50)]}, "r"))
        self.assertFalse(holds(untagged, {"C": [eff("ResolveHazard", tag="Hazard.Plague"), eff("ResolveHazard")]}, "r"))
        rule = ["tables", "DT_Encounters", "rules", 0]
        # The Key and the Tag of a typed operand are read against the usage of the type, as an entry's are: a misspelt Key, or
        # a Tag on a type that reads none, would match no entry - a lacks_cond switched off in silence.
        self.schema_problem(self.put(rule, "then", [{"col": "Conditions", "lacks_cond": {"type": "SystemOwnedBy", "key": "Ruld"}}]),
                            "schema.DT_Encounters", "names the Key Ruld of SystemOwnedBy, which is none of civ_selector")
        self.schema_problem(self.put(rule, "then", [{"col": "Conditions", "lacks_cond": {"type": "SystemOwnedBy", "tag": "Hazard.Claim"}}]),
                            "schema.DT_Encounters", "names the tag Hazard.Claim of SystemOwnedBy, which reads no Tag")
        # An empty Key or Tag where the usage requires one matches no valid entry either (an empty optional one is the empty
        # field - the untagged ResolveHazard above), and 'None' is read as the empty field, as holds() reads it.
        self.schema_problem(self.put(rule, "then", [{"col": "Conditions", "lacks_cond": {"type": "PlayerRoleIs", "key": ""}}]),
                            "schema.DT_Encounters", "names an empty Key of PlayerRoleIs, which requires one")
        self.schema_problem(self.put(rule, "then", [{"col": "Conditions", "lacks_cond": {"type": "CivHasNotable", "tag": "None"}}]),
                            "schema.DT_Encounters", "names an empty Tag of CivHasNotable, which requires one")
        self.expect(self.after(NOTHING, self.put(rule, "then", [{"col": "Conditions", "lacks_cond": {"type": "SystemOwnedBy", "key": "None"}}])),
                    NOT_CHECKED, wanted=False)
        self.schema_problem(self.put(rule, "then", [{"col": "Conditions", "lacks_cond": {"type": "PlayerRoleIz", "key": "Ruler"}}]),
                            "schema.DT_Encounters", "'PlayerRoleIz', which is no enumerator of conditions")
        self.schema_problem(self.put(rule, "then", [{"col": "Conditions", "lacks_cond": {"tag": "Hazard.Claimed"}}]),
                            "schema.DT_Encounters", "names the tag Hazard.Claimed, which the ini does not declare")
        self.schema_problem(self.put(rule, "then", [{"col": "Conditions", "lacks_cond": {"type": "PlayerRoleIs", "value": "1"}}]),
                            "schema.DT_Encounters", "the test 'lacks_cond' needs an object of the strings type, key and tag with a key or a tag")
        self.schema_problem(self.put(rule, "then", [{"col": "Title", "lacks_cond": {"type": "PlayerRoleIs", "key": "Ruler"}}]),
                            "schema.DT_Encounters", "the test 'lacks_cond' cannot be asked of a column of kind text")

    def test_rows_of_and_the_extinction_row(self):
        # B6b [B-6b2b] (header 3.3 rule 5): rows_of holds when every row name of a rows column is a row of one of the named
        # tables (an empty list holds); it names only tables the column names, and only a rows column.
        holds = checker.Checker(self.schema, self.tables, self.tags).holds
        projects = {"col": "C", "rows_of": ["DT_Megaprojects"]}
        for fine in ([], ["project_test_build"]):
            with self.subTest(cell=fine):
                self.assertTrue(holds(projects, {"C": fine}, "r"))
        for wrong in (["project_test_build", "edict_test_order"], ["project_nobody"], None, "project_test_build"):
            with self.subTest(cell=wrong):
                self.assertFalse(holds(projects, {"C": wrong}, "r"))
        self.assertTrue(holds({"col": "C", "rows_of": ["DT_Megaprojects", "DT_Edicts"]}, {"C": ["project_test_build", "edict_test_order"]}, "r"))
        rule = ["tables", "DT_Hazards", "rules", 0]
        self.schema_problem(self.put(rule, "then", [{"col": "RulerResolutions", "rows_of": ["DT_Events"]}]),
                            "schema.DT_Hazards", "names the table DT_Events, which the column RulerResolutions does not name")
        self.schema_problem(self.put(rule, "then", [{"col": "WandererResolutions", "rows_of": ["DT_Megaprojects"]}]),
                            "schema.DT_Hazards", "names the table DT_Megaprojects, which the column WandererResolutions does not name")
        self.schema_problem(self.put(rule, "then", [{"col": "OutbreakTemplates", "rows_of": ["DT_Megaprojects"]}]),
                            "schema.DT_Hazards", "the test 'rows_of' cannot be asked of a column of kind texts")
        self.schema_problem(self.put(rule, "then", [{"col": "RulerResolutions", "rows_of": []}]), "schema.DT_Hazards", "the test 'rows_of' needs a list of strings, not empty")
        # The extinction's row: its loss is the expiry's code, so its expiry carries no effect of its own (the breaking case: a
        # KillPlayer there - the death is the code's), no branch and no damage to the neighbours, and a project its only answer
        # (the breaking case: an edict - the game's answers of another hazard may be edicts).
        why = "the extinction's loss is the expiry's code"
        extinct = lambda **changes: (lambda tables: self.row(tables, "DT_Hazards", "crisis_test_storm").update(  # noqa: E731
            dict({"HazardTag": "Hazard.Extinction", "OnExpireEffects": []}, **changes)))
        self.assertEqual(self.after(extinct()), [])
        self.assertEqual(self.after(extinct(RulerResolutions=["project_test_build"])), [])
        self.expect(self.after(extinct(OnExpireEffects=[eff("KillPlayer", tag="Death.Hazard")])), "DT_Hazards.crisis_test_storm", why, '"col": "OnExpireEffects", "max_len": 0')
        self.expect(self.after(extinct(OnExpireNeighbourEffects=[eff("ChangeStability", value=-0.1)])), "DT_Hazards.crisis_test_storm", why, '"col": "OnExpireNeighbourEffects"')
        self.expect(self.after(extinct(BranchChance=0.5, OnExpireBranchEffects=[eff("ChangeStability", value=-0.1)])), "DT_Hazards.crisis_test_storm", why, '"col": "BranchChance"')
        self.expect(self.after(extinct(RulerResolutions=["edict_test_order"])), "DT_Hazards.crisis_test_storm", why, '"rows_of": ["DT_Megaprojects"]')
        self.expect(self.after(extinct(RulerResolutions=["project_test_build", "petition_test_plea"])), "DT_Hazards.crisis_test_storm", why, '"rows_of"')
        # Another hazard keeps its effects and may be answered by an edict.
        self.assertEqual(self.after(self.set_cell("DT_Hazards", "crisis_test_storm", "RulerResolutions", ["edict_test_order"])), [])

    def test_the_binding_rows(self):
        # B6b [B-6b2c] (header 3.3 rules 7 and 8): a member of a resource pool is named 'Pool.Member' and read as a number; a
        # project asks no Causality (the breaking case: Causality 1); the binding's line names the system alone (the breaking
        # cases: {Civ}, {Project}).
        holds = checker.Checker(self.schema, self.tables, self.tags).holds
        causality = {"col": "CostPerTick.Causality", "is": 0}
        self.assertTrue(holds(causality, {"CostPerTick": pool()}, "r"))
        self.assertFalse(holds(causality, {"CostPerTick": pool(Causality=1.0)}, "r"))
        self.assertFalse(holds(causality, {"CostPerTick": {"Energy": 0.0}}, "r"))
        self.assertTrue(holds({"col": "CostPerTick.Energy", "gt": 0}, {"CostPerTick": pool(Energy=2.0)}, "r"))
        rule = ["tables", "DT_Megaprojects", "rules", 0]
        self.schema_problem(self.put(rule, "then", [{"col": "CostPerTick.Faith", "is": 0}]),
                            "schema.DT_Megaprojects", "names the member 'Faith', which the pool CostPerTick of DT_Megaprojects does not have")
        self.schema_problem(self.put(rule, "then", [{"col": "CostPerTick.Energy", "ge_col": "RequiredProgress"}]),
                            "schema.DT_Megaprojects", "the test 'ge_col' cannot be asked of a column of a list of structs or a member of a pool")
        self.schema_problem(self.put(rule, "then", [{"col": "CostPerTick.Causality", "is": "None"}]),
                            "schema.DT_Megaprojects", "the test 'is' over a column of kind number needs numbers")
        self.schema_problem(self.put(rule, "then", [{"col": "RequiredProgress.Causality", "is": 0}]),
                            "schema.DT_Megaprojects", "and RequiredProgress is no list of structs")
        causality_why = "a project asks no Causality"
        self.assertEqual(self.after(self.set_cell("DT_Megaprojects", "project_binding", "CostPerTick", pool(Energy=4.0, Matter=4.0))), [])
        self.expect(self.after(self.set_cell("DT_Megaprojects", "project_binding", "CostPerTick", pool(Energy=4.0, Causality=1.0))),
                    "DT_Megaprojects.project_binding", causality_why, '"col": "CostPerTick.Causality"')
        # The project's own rows pass: the binding asks Energy and Matter only.
        project_schema, project_tables, project_tags = checker.load_project()
        binding = next(row for row in project_tables["DT_Megaprojects"] if row["Name"] == "project_binding")
        self.assertEqual(binding["CostPerTick"]["Causality"], 0.0)
        self.assertEqual(binding["RequiredTier"], "Tier4_Transcendent")
        # The binding's line: {System} alone.
        bound_why = "the line of the binding is told in the actorless passive"
        line = lambda templates: self.set_cell("DT_LogTemplates", "log_fallen_bound", "Templates", templates)  # noqa: E731
        self.assertEqual(self.after(line(["{System} a.", "b.", "c."])), [])
        for slot in ("Civ", "Project", "OtherCiv", "Transcendent"):
            with self.subTest(slot=slot):
                self.expect(self.after(line(["{System} {%s} a." % slot, "b.", "c."])), "DT_LogTemplates.log_fallen_bound", bound_why, "slots_in")
        # Another row may name the people.
        self.assertEqual(self.after(self.set_cell("DT_LogTemplates", "log_fleet_worn", "Templates", ["{System} {Civ} a.", "b.", "c."])), [])

    def test_the_nameless_lines(self):
        # B6b [B-6b3a] (header 2.9, 3.3 rule 8, decision 19): the nameless variant of each of the six lines that name the
        # other people - chosen when that people stands far above the ruled one - says no {OtherCiv}. The breaking case, in
        # every row: {OtherCiv} in its first variant; the named row keeps it.
        far_rows = {"log_siege_on_us_far": "log_siege_on_us", "log_treaty_broken_far": "log_treaty_broken", "log_siege_lost_far": "log_siege_lost",
                    "log_siege_held_far": "log_siege_held", "log_siege_lifted_far": "log_siege_lifted", "log_stance_lapsed_far": "log_stance_lapsed"}
        for far, named in far_rows.items():
            with self.subTest(row=far):
                templates = self.row(self.tables, "DT_LogTemplates", far)["Templates"]
                self.assertEqual(self.after(self.set_cell("DT_LogTemplates", far, "Templates", list(templates))), [])
                broken = ["{OtherCiv} " + templates[0]] + list(templates[1:])
                self.expect(self.after(self.set_cell("DT_LogTemplates", far, "Templates", broken)),
                            "DT_LogTemplates.%s" % far, "the nameless variant of %s names no other people" % named, "slots_in")
                self.assertTrue(any("{OtherCiv}" in text for text in self.row(self.tables, "DT_LogTemplates", named)["Templates"]))
        # The project's own rows: each nameless row is in the project with the category and the number of variants of its
        # named row, and none of its variants says {OtherCiv}.
        project_schema, project_tables, project_tags = checker.load_project()
        rows = {row["Name"]: row for row in project_tables["DT_LogTemplates"]}
        for far, named in far_rows.items():
            with self.subTest(project_row=far):
                self.assertIn(far, project_schema["tables"]["DT_LogTemplates"]["required_rows"])
                self.assertEqual(rows[far]["Category"], rows[named]["Category"])
                self.assertEqual(len(rows[far]["Templates"]), len(rows[named]["Templates"]))
                self.assertFalse(any("{OtherCiv}" in text for text in rows[far]["Templates"]))

    def test_a_ruler_card_does_not_wait_on_the_vessel_rung(self):
        # B6a [B-6a6] (header 3.1, decision 9): the ruler's personal rung never rises, so a ruler's row that waits on it never
        # comes. The breaking case is petition_exalt as it stood from [A-2b] to [B-6a5]: the ruler and the vessel's rung 2.
        why = "a ruler's card does not wait on the vessel's own rung"
        old = [cond("PlayerRoleIs", key="Ruler"), cond("PlayerTierAtLeast", key="None", value=2.0)]
        self.expect(self.after(self.set_cell("DT_Encounters", "petition_exalt", "Conditions", old)), "DT_Encounters.petition_exalt", why)
        # The project's own row, put back to the old condition: the project's data with that one cell fails on this rule.
        project_schema, project_tables, project_tags = checker.load_project()
        exalt = next(row for row in project_tables["DT_Encounters"] if row["Name"] == "petition_exalt")
        self.assertEqual([(entry["Type"], entry["Key"], entry["Value"]) for entry in exalt["Conditions"]],
                         [("PlayerRoleIs", "Ruler", 0.0), ("CivTierAtLeast", "None", 2.0)])
        exalt["Conditions"] = [dict(entry, Type="PlayerTierAtLeast") if entry["Type"] == "CivTierAtLeast" else entry for entry in exalt["Conditions"]]
        self.expect(checker.check(project_schema, project_tables, project_tags), "DT_Encounters.petition_exalt", why)
        # The people's rung on a ruler's row, and the vessel's rung on a wanderer's card (the wanderer's rung rises), stand.
        self.assertEqual(self.after(self.set_cell("DT_Encounters", "petition_exalt", "Conditions", [old[0], cond("CivTierAtLeast", key="None", value=2.0)])), [])
        self.assertEqual(self.after(self.set_cell("DT_Encounters", "enc_test_weave", "Conditions",
                                                  [cond("PlayerRoleIs", key="Wanderer"), cond("PlayerTierAtLeast", key="None", value=2.0)])), [])

    def test_row_prefix(self):
        # B1: the rows of one kind by the prefix of their names - one of the table's own prefixes.
        holds = checker.Checker(self.schema, self.tables, self.tags).holds
        self.assertTrue(holds({"row_prefix": "enc_"}, {}, "enc_rebel_parley"))
        self.assertFalse(holds({"row_prefix": "enc_"}, {}, "petition_plague"))
        rule = ["tables", "DT_Encounters", "rules", 0]
        self.schema_problem(self.put(rule, "when", [{"row_prefix": "card_"}]), "schema.DT_Encounters", "names the prefix card_, which is none of the prefixes of DT_Encounters")
        self.schema_problem(self.put(rule, "when", [{"row_prefix": "enc_", "col": "Subject"}]), "schema.DT_Encounters", "a row test is 'row' with a row name")
        self.schema_problem(self.put(rule, "when", [{"row_prefix": ["enc_"]}]), "schema.DT_Encounters", "a row test is 'row' with a row name")
        # B3: every row but the named one - a row the table has, of the same shape as 'row'.
        self.assertTrue(holds({"row_not": "petition_first_contact"}, {}, "petition_plague"))
        self.assertFalse(holds({"row_not": "petition_first_contact"}, {}, "petition_first_contact"))
        self.schema_problem(self.put(rule, "when", [{"row_not": "petition_first_kontact"}]), "schema.DT_Encounters", "names the row petition_first_kontact")
        self.schema_problem(self.put(rule, "when", [{"row_not": "petition_first_contact", "col": "Subject"}]), "schema.DT_Encounters", "a row test is 'row' with a row name")
        # An enc_ row about the ruled people, and one that checks a people's Stability, break the wanderer card's rule.
        self.expect(self.after(self.set_cell("DT_Encounters", "enc_test_weave", "Subject", "SettledSystem")), "DT_Encounters.enc_test_weave", "a wanderer's card")
        weave = self.row(self.tables, "DT_Encounters", "enc_test_weave")
        stability = [dict(weave["Choices"][0], CheckStat="Stability")] + weave["Choices"][1:]
        self.expect(self.after(self.set_cell("DT_Encounters", "enc_test_weave", "Choices", stability)), "DT_Encounters.enc_test_weave", "a wanderer's card")

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
        """Change row so that test no longer holds, by the kind of its column. A test over a column of a list of structs
        ('List.Column') is broken in the first entry of the list."""
        columns = schema["tables"][table]["columns"]
        if "none" in test:
            # B4: 'none' is broken in the first entry of its list, made to hold every one of its tests.
            head = test["none"][0]["col"].partition(".")[0]
            for inner in test["none"]:
                self.satisfy_in(schema, columns[head]["columns"], dict(inner, col=inner["col"].partition(".")[2]), row[head][0])
            return
        head, mark, sub = test["col"].partition(".")
        if mark and columns[head]["kind"] == "pool":
            # B6b [B-6b2c]: a member of a resource pool, broken as a number of the pool's own cell.
            self.violate_in(schema, {sub: {"kind": "number"}}, dict(test, col=sub), row[head])
        elif mark:
            self.violate_in(schema, columns[head]["columns"], dict(test, col=sub), row[head][0])
        else:
            self.violate_in(schema, columns, test, row)

    def satisfy_in(self, schema, columns, test, row):
        """Change row so that test holds, by the kind of its column (the tests a 'none' of the schema asks)."""
        column = test["col"]
        kind = columns[column]["kind"]
        name = next(key for key in test if key != "col")
        operand = test[name]
        if name == "has_type":
            if not any(item.get("Type") == operand for item in row[column]):
                row[column] = list(row[column]) + [cond(operand) if kind == "conditions" else eff(operand)]
        elif name == "lacks_type":
            row[column] = [item for item in row[column] if item.get("Type") != operand]
        elif name in ("is", "in"):
            row[column] = operand[0] if name == "in" else operand
        elif name == "zero":
            members = schema["words"]["resource"]
            row[column] = {member: 0.0 for member in members} if operand else dict(row[column], **{members[0]: 1.0})
        else:
            self.fail("the walker cannot make the test %s hold: add the case" % name)

    def violate_in(self, schema, columns, test, row):
        column = test["col"]
        spec = columns[column]
        kind = spec["kind"]
        name = next(key for key in test if key != "col")
        operand = test[name]
        if name in ("is", "in"):
            taken = operand if name == "in" else [operand]
            if kind == "number":
                row[column] = max(taken) + 1
            elif kind == "row":
                row[column] = next(entry["Name"] for entry in self.tables[spec["table"]] if entry["Name"] not in taken)
            elif kind == "tag":
                row[column] = next(tag for tag in sorted(self.tags) if tag not in taken)
            elif kind == "enum":
                row[column] = next(entry for entry in schema["enums"][spec["enum"]] if entry not in taken)
            else:
                self.fail("the walker cannot break %s over a column of kind %s: add the case" % (name, kind))
        elif name in ("gt", "lt", "ge", "le"):
            bad = operand if name in ("gt", "lt") else operand - 1 if name == "ge" else operand + 1
            if kind == "tag_map":
                # B6b: a comparison over a tag map is broken in its first key (a tag under the column's root when it is empty).
                roots = spec.get("root")
                roots = [roots] if isinstance(roots, str) else (roots or [""])
                key = next(iter(row[column]), None) or next(tag for tag in sorted(self.tags) if any(tag.startswith(root) for root in roots))
                row[column] = dict(row[column], **{key: float(bad)})
            else:
                row[column] = bad
        elif name == "min_len":
            row[column] = self.sample(kind, operand - 1)
        elif name == "max_len":
            row[column] = self.sample(kind, operand + 1)
        elif name == "each_contains":
            row[column] = ["nothing of the kind"]
        elif name == "lacks_type":
            row[column] = list(row[column]) + [cond(operand) if kind == "conditions" else eff(operand)]
        elif name == "has_type":
            row[column] = [item for item in row[column] if item.get("Type") != operand]
        elif name == "has_cond":
            # The named one keeps its enumerator and loses its Key: what has_type alone could not see.
            self.assertEqual(set(operand), {"type", "key"}, "the walker breaks a has_cond of a type and a Key only: add the case")
            row[column] = [dict(item, Key="Other") if item.get("Type") == operand["type"] and item.get("Key") == operand["key"] else item for item in row[column]]
        elif name == "lacks_cond":
            # An entry of the named fields put into the list (B6b).
            self.assertIn("type", operand, "the walker breaks a lacks_cond that names its type: add the case")
            make = cond if kind == "conditions" else eff
            row[column] = list(row[column]) + [make(operand["type"], tag=operand.get("tag", ""), key=operand.get("key", ""))]
        elif name == "set" and operand is True:
            row[column] = ""
        elif name == "set" and operand is False and kind == "text":
            row[column] = "a."
        elif name == "set" and operand is False and kind == "row":
            # A row of the column's table that answers the column's own target, so that only the rule is broken.
            target = spec.get("target", [])
            self.assertTrue(all(set(test) == {"col", "is"} or "lacks_type" in test for test in target), "the walker reads 'is' and 'lacks_type' targets only")
            row[column] = next(entry["Name"] for entry in self.tables[spec["table"]]
                               if all(entry.get(test["col"]) == test["is"] if "is" in test
                                      else all(item.get("Type") != test["lacks_type"] for item in entry.get(test["col"], []))
                                      for test in target))
        elif name == "slots_in":
            row[column] = ["{NoSuchSlot} a.", "b.", "c."] if kind == "texts" else "{NoSuchSlot}"
        elif name == "contains_all":
            # The first named tag taken out of the cell, the others kept.
            if kind == "tags":
                kept = [tag for tag in checker.TAG_NAME.findall(row[column]) if tag != operand[0]]
                row[column] = tags_text(*kept) if kept else ""
            else:
                row[column] = [tag for tag in row[column] if tag != operand[0]]
        elif name == "rows_of":
            # B6b: a row of the first table the column names and the test does not, put into the list.
            other = next(table for table in spec["tables"] if table not in operand)
            row[column] = list(row[column]) + [self.tables[other][0]["Name"]]
        elif name == "contains_none":
            # The first named tag put into the cell (B4: a key of a tag map).
            if kind == "tags":
                row[column] = tags_text(*(checker.TAG_NAME.findall(row[column]) + [operand[0]]))
            elif kind == "tag_map":
                row[column] = dict(row[column], **{operand[0]: 1.0})
            else:
                row[column] = list(row[column]) + [operand[0]]
        elif name == "zero":
            members = schema["words"]["resource"]
            row[column] = dict(row[column], **{members[0]: 1.0}) if operand else {member: 0.0 for member in members}
        elif name == "lacks_words":
            # The first word of the named list put into the text (B4).
            word = schema["words"][operand][0]
            row[column] = "%s a." % word if kind == "text" else ["%s a." % word, "b.", "c."]
        elif name == "no_digits":
            # A digit put into the text, or every digit taken out (B5 [B-5d1]).
            if operand:
                row[column] = "3 a." if kind == "text" else ["3 a.", "b.", "c."]
            else:
                row[column] = "a." if kind == "text" else ["a.", "b.", "c."]
        elif name == "ge_col":
            # The named column above the cell (B5).
            row[operand] = row[column] + 1
        elif name == "len_ge_col":
            row[operand] = len(row[column]) + 1
        elif name == "pairs_ge_col":
            row[operand[1]] = len(row[column]) * len(row[operand[0]]) + 1
        else:
            self.fail("the walker cannot break the test %s: add the case" % name)

    def add_sign_row(self, tables):
        row = copy.deepcopy(self.row(tables, "DT_LogTemplates", "log_breakthrough_physics"))
        row.update(Name="log_sign_test", Category="Log.Sign", Templates=["{Civ} a.", "{Hazard} b.", "c."])
        tables["DT_LogTemplates"].append(row)
        self.row(tables, "DT_Events", "event_test_harvest").update(SignRow="log_sign_test", MinTierToReveal=3)

    def add_record_row(self, tables):
        """A record line (B5 [B-5d1]) of the fixture: the system's name alone, no figure."""
        row = copy.deepcopy(self.row(tables, "DT_LogTemplates", "log_breakthrough_physics"))
        row.update(Name="log_record_test", Category="Log.Record", MinTierToReveal=0, Templates=["{System}|hpp(이,가) 기록되었다.", "{System} b.", "c."])
        tables["DT_LogTemplates"].append(row)

    def add_independent_row(self, tables):
        tables["DT_Transcendents"].append(transcendent_row(
            "trans_stillness", Doctrine="Doctrine.Stillness", Allegiance="Realm.Independent", SpawnWeight=0.0, PressureEffectsPerTick=[]))

    def rule_setups(self):
        """Why of a rule -> (what to change so that its 'when' holds, the row it then holds for). A rule of the schema
        without an entry here fails the walk below."""
        event = "event_test_harvest"
        update_event = lambda **changes: (lambda tables: self.row(tables, "DT_Events", event).update(**changes))  # noqa: E731
        hazard = "crisis_test_storm"
        update_hazard = lambda **changes: (lambda tables: self.row(tables, "DT_Hazards", hazard).update(**changes))  # noqa: E731
        named = update_event(Trigger="Named", BaseWeight=0, CooldownTicks=0, LogTemplates=["one."])
        update_feat = lambda **changes: (lambda tables: self.row(tables, "DT_Feats", "feat_test_climb").update(**changes))  # noqa: E731
        plea = "petition_test_plea"
        update_plea = lambda **changes: (lambda tables: self.row(tables, "DT_Encounters", plea).update(**changes))  # noqa: E731
        return {
            ("DT_LogTemplates", "the calling's own line is told whatever its subject, and no tier window hides it: a line of the calling that waits for a tier is an event row with its sign row (guideline 8.1)"):
                (NOTHING, "log_first_life"),
            ("DT_LogTemplates", "a far line is the same sentence for everyone, told with no subject and no arguments, and the clock of the race stops when a window hides it: log_far_event (guideline 8.1)"):
                (NOTHING, "log_far_event"),
            ("DT_LogTemplates", "a far line is the same sentence for everyone, told with no subject and no arguments, and the clock of the race stops when a window hides it: log_far_ascent (guideline 8.1)"):
                (NOTHING, "log_far_ascent"),
            ("DT_Events", "the subject system of a line is the system it names: a Civilization row is about the people alone and names no system, so the name of a system the calling does not know never reaches the feed (guideline 8.1)"):
                (NOTHING, event),
            ("DT_Rules", "the first life is born into the first life's row, arch_seeker (guideline 5.10)"): (NOTHING, "rules"),
            ("DT_Civs", "the first life's row is a seeking people no AI people is drawn from: weight 0, its archetype tag, the seeking revelation (guideline 5.10, 5.11)"): (NOTHING, "arch_seeker"),
            ("DT_Rules", "every immutable law the rules read is listed, and none can be taken out of the list: Law.Hedge keeps the sanctuary (guideline 4.5)"): (NOTHING, "rules"),
            ("DT_Civs", "the name pattern of a people drawn for the AI calls its home star (guideline 5.11)"): (NOTHING, "arch_default"),
            ("DT_Civs", "the home of a people drawn for the AI takes a proper name from the galaxy without replacement, never a fixed one two peoples could share (guideline 7.4)"):
                (NOTHING, "arch_default"),
            ("DT_Events", "a slot row tells with three variants or more (guideline 8.2)"): (NOTHING, event),
            ("DT_Events", "a row fired by name tells with one variant or more (guideline 8.2)"): (named, event),
            ("DT_Events", "a slot runs for every people: it carries no condition on the vessel, no role effect and no effect on the vessel - the selector words of a slot row that read the vessel are Ruled (B6b header 2.7) and Former of SystemOwnedBy, Former of CivHasNotable and Former of NameNotable (an effect); the other selector words wait for the Phase 4 petitions (guideline 16 #29 (7))"): (NOTHING, event),
            ("DT_Events", "a row fired by name is not a slot: the stay condition holds on every tick of its window there (guideline 7.3)"): (named, event),
            ("DT_Events", "a system-subject row lands on a place and the stay condition is about the people: without a cooldown a Standing row would fire once for every settled system in the one slot, otherwise it lands on whichever settled system comes first (guideline 7.3)"):
                (update_event(Subject="SettledSystem"), event),
            ("DT_Events", "the research stage fires the found row by name: as a slot row it would also fire in the slots (guideline 5.11)"): (NOTHING, "event_book_found"),
            # B7x [B-7x2] (header section 2.3, 2.4): the bordering condition off system-subject rows and spawn conditions, and the
            # brick multipliers above 0.
            ("DT_Events", "a system-subject row is evaluated on every settled system of its slot, and the bordering condition walks the people's borders on each: it is about the people alone, so it stands on Civilization rows (B7x header section 2.3)"): (update_event(Subject="SettledSystem"), event),
            ("DT_Hazards", "the stage reads the spawn conditions of every Draw row on every settled system on every tick: the bordering condition would walk the owner's borders once a system and a tick (B7x header section 2.3)"): (NOTHING, hazard),
            ("DT_Rules", "a brick multiplier of a progress or a share multiplies what every tick of a people that carries the brick adds: at 0 or below a settlement or a research would never progress and a hazard would never come, so the code reads such a value as 1 (B7x header section 2.4)"): (NOTHING, "rules"),
            ("DT_Events", "a Lottery row with no weight never fires (guideline 7.5)"): (NOTHING, event),
            ("DT_Events", "only a Lottery row has a weight"): (update_event(Trigger="Standing", BaseWeight=0), event),
            ("DT_Events", "a row fired by name has no cooldown key"): (named, event),
            ("DT_Events", "a sign row is told below MinTierToReveal, so there has to be a tier below it"): (self.add_sign_row, event),
            ("DT_Events", "the calling's own line that waits for a tier comes with its sign row: the viewers below the window hear the sign (guideline 8.1)"):
                (lambda tables: (self.add_sign_row(tables), update_event(Category="Event.Grace.Rescue", LogCategory="Log.Calling")(tables)), event),
            ("DT_Events", "a grace row names no one (guideline 9.3: NameNotable is not carried by Event.Grace rows)"):
                (update_event(Category="Event.Grace.Jubilee", LogCategory="Log.Civ"), event),
            ("DT_Events", "the last-system gate fires the remnant row by name: as a slot row it would also fire in the slots (guideline 4.5)"):
                (NOTHING, "event_grace_remnant"),
            ("DT_Events", "a death comes only from a hazard's expiry (B5 decision 6): no event row records one (guideline 7.4)"): (NOTHING, event),
            ("DT_Hazards", "the claim is placed by the claim rule alone, which holds Law.Remnant (guideline 4.2)"):
                (update_hazard(HazardTag="Hazard.Claim", SpawnRule="ClaimRule", SpawnChancePerTick=0), hazard),
            ("DT_Hazards", "the claim rule places the claim and nothing else (guideline 4.2)"):
                (update_hazard(HazardTag="Hazard.Claim", SpawnRule="ClaimRule", SpawnChancePerTick=0), hazard),
            ("DT_Hazards", "a rebellion is the discontent of a people in its capital, not a draw (guideline 4.1)"):
                (update_hazard(HazardTag="Hazard.Rebellion", SpawnRule="Capital", SpawnChancePerTick=0), hazard),
            ("DT_Hazards", "a Draw row is drawn every tick: a chance of 0 is a slip, and a hazard that only an effect or a hook places will have a spawn rule of its own (guideline 7.3, Phase 4)"): (NOTHING, hazard),
            ("DT_Hazards", "only a drawn hazard has a chance: the capital rule and the claim rule draw nothing (guideline 7.4)"):
                (update_hazard(SpawnRule="Capital", SpawnChancePerTick=0), hazard),
            ("DT_Hazards", "a branch is a chance and the effects it takes instead (guideline 4.1)"):
                (update_hazard(BranchChance=0.5, OnExpireBranchEffects=[eff("ChangeStability", value=-0.2)]), hazard),
            ("DT_Hazards", "branch effects without a chance are never applied (guideline 4.1)"): (NOTHING, hazard),
            ("DT_Hazards", "a death comes only from a hazard's own expiry, on its own side (B5 decision 6-ga, header 3.5): never from its branch, the damage to its neighbours or its resolution (guideline 7.4)"):
                (NOTHING, hazard),
            ("DT_Hazards", "the fallen cannot touch a life - the claim takes from the living only - and the visitation is the holy order's own: no claim, visitation or cult kills (B5 header 3.5)"):
                (update_hazard(HazardTag="Hazard.Visitation"), hazard),
            ("DT_Hazards", "a hazard a fallen one's doctrine brings never kills at its expiry, whatever its tag: the fallen cannot touch a life, so a row with a SourceDoctrine carries no death (B5 header 3.5; the one death decision 6-ga opens is no doctrine's)"):
                (update_hazard(SourceDoctrine="Doctrine.Zeal"), hazard),
            ("DT_Hazards", "the supernova precursor's expiry destroys the system, and the death line mourns a star that stays: no death at its expiry until it has a death row of its own (B5 header 8 [B-5d2], DECISIONS_LOG [B-5d1] (11))"):
                (update_hazard(HazardTag="Hazard.PreSupernova"), hazard),
            ("DT_Hazards", "the leader of a cult has no name (guideline 4.1)"): (update_hazard(HazardTag="Hazard.Cult"), hazard),
            # B6b [B-6b2b]: the extinction's row (the fixture's own extinction - the game's row lives in the project data, which the fixture replaces).
            ("DT_Hazards", "the extinction's loss is the expiry's code - its people lets every system go and its ruler dies (CosmicHazards::LetPeopleGo, KillsTheRulerOf): its row carries no expiry effect of its own, no branch and no damage to the neighbours, and it is answered by a project alone - the ark (B6b header 2.2, 3.3 rule 5, decision 5)"):
                (update_hazard(HazardTag="Hazard.Extinction", OnExpireEffects=[], RulerResolutions=["project_test_build"]), hazard),
            ("DT_Hazards", "the stage reads the spawn conditions on every tick, and the stay condition holds on every tick of its window, not in one slot (guideline 7.3)"):
                (NOTHING, hazard),
            ("DT_Hazards", "only stage code behind the last-system gate takes a system: an expiry hands nothing over, and neither does a resolution, which an expiry may run on a neighbour (guideline 4.5, 16 #53 (1))"): (NOTHING, hazard),
            ("DT_Hazards", "a resolution or an expiry does not resolve again on its own system: a row that resolves what it places would never end (guideline 7.3)"):
                (NOTHING, hazard),
            ("DT_Feats", "the wanderer's feats are judged from Phase 4, with their first row: until then no row walks that path (guideline 3.2, 7.5)"):
                (NOTHING, "feat_test_climb"),
            ("DT_Feats", "a feat lifts its people one rung: from the first to the second (guideline 3.2; the judgment does not judge another row)"):
                (NOTHING, "feat_test_climb"),
            ("DT_Feats", "a feat lifts its people one rung: from the second to the third (guideline 3.2; the judgment does not judge another row)"):
                (update_feat(RequiredTier="Tier2_Stellar", TargetTier="Tier3_Causal"), "feat_test_climb"),
            ("DT_Feats", "a feat lifts its people one rung: from the third to the fourth (guideline 3.2; the judgment does not judge another row)"):
                (update_feat(RequiredTier="Tier3_Causal", TargetTier="Tier4_Transcendent"), "feat_test_climb"),
            ("DT_Feats", "a feat lifts its people one rung: from the fourth to the fifth (guideline 3.2; the judgment does not judge another row)"):
                (update_feat(RequiredTier="Tier4_Transcendent", TargetTier="Tier5_EternalOrder"), "feat_test_climb"),
            ("DT_Feats", "the fifth rung is the last: no feat climbs from it (guideline 3.2)"): (NOTHING, "feat_test_climb"),
            ("DT_Feats", "no people climbs without a deed: a feat holds one condition or more (guideline 3.2; the judgment does not judge a row without one)"):
                (NOTHING, "feat_test_climb"),
            ("DT_Feats", "a ruler feat row judges every people, the AI's too (guideline 4.3): its conditions do not read the vessel, and its price records no role effect and touches no vessel (a death comes only from a hazard's expiry - B5 decision 6). The rungs stand apart: no ruler row reads the vessel (guideline 16 #1 closed - B6b answer A3); a wanderer's feat is judged on the vessel and gets its own rules with its first row (guideline 16 #28)"):
                (NOTHING, "feat_test_climb"),
            # B6b [B-6b2a]: no ruler row reads a sealed transcendent (decision 12).
            ("DT_Feats", "a ruler row never reads a sealed transcendent: an AI is judged from tick 0, so another people's binding would lift it for nothing - the binding is read by MegaprojectComplete of the people that made it (B6b decision 12, guideline 16 #73)"):
                (NOTHING, "feat_test_climb"),
            # B6b [B-6b2a]: nor a selector word of the vessel (A3).
            ("DT_Feats", "a ruler row names no selector word of the vessel either: Ruled and Former of SystemOwnedBy and Former of CivHasNotable in its conditions, and Former of NameNotable in its price (every people's climb pays it, an AI's too), resolve from the vessel (the people it rules, the one it left), so the row would judge the ruled people apart from every AI or name a notable in the vessel's former people (B6b answer A3, header 3.3 rule 6)"):
                (NOTHING, "feat_test_climb"),
            ("DT_Feats", "a feat's price records no death on any path, the wanderer's too when its rows come: a death comes only from a hazard's own expiry (B5 decision 6-ga, header 3.5)"):
                (NOTHING, "feat_test_climb"),
            # B7x [B-7x2]: K4 - no feat row reads a brick (notes 5.4 data-3).
            ("DT_Feats", "K4 - a rung comes from a feat's deed, never from a brick or a mutation (B7x header K4)"):
                (NOTHING, "feat_test_climb"),
            ("DT_Rules", "the rate of the AI's projects multiplies their progress: at 0 or below they would never progress, so the code reads it as 1 with an error line (guideline 4.3, 5.7)"):
                (NOTHING, "rules"),
            ("DT_Rules", "the leap of a death is drawn between its shortest and its longest: a longest below the shortest is read as the shortest (B5 header decision 3) - a data fault"):
                (NOTHING, "rules"),
            ("DT_Edicts", "a held edict multiplies the fleet's growth: at 0 the inherited edict would keep the ruler's fleet at 0 from the first tick, so the code reads a factor that is not above 0 as 1 (B3 header 3.2)"):
                (NOTHING, "edict_test_order"),
            ("DT_Transcendents", "the fixed place of the roster belongs to a fallen silence"): (NOTHING, "trans_silence"),
            ("DT_Transcendents", "a transcendent stands on tier 3 or 4 (guideline 4.2: no fallen one on tier 5)"): (NOTHING, "trans_silence"),
            ("DT_Transcendents", "a celestial transcendent has no instance: no sphere, no pressure, no roster weight"): (NOTHING, "trans_covenant"),
            ("DT_Transcendents", "the roster of NewGalaxy draws fallen rows only"): (self.add_independent_row, "trans_stillness"),
            ("DT_Transcendents", "a fallen one is named by its title, never a name: the spiritual sight's line and tooltip and the log's {Transcendent} slot read it (guideline 4.2, 6.4, 8.2)"): (NOTHING, "trans_silence"),
            ("DT_Galaxies", "a galaxy other than the home holds no peoples, no brake, no home share and no name pools (guideline 5.11 '고향이 아닌 행의 이 네 값은 0')"): (NOTHING, "gal_m87"),
            ("DT_Encounters", "a card is put before a role: every row carries the condition PlayerRoleIs (guideline 5.10, B1)"): (NOTHING, plea),
            ("DT_Encounters", "a petition is put before a ruler and is about the ruled people or one of its settled systems (guideline 5.10, B1)"): (NOTHING, plea),
            # B6a [B-6a6] (header 3.1): the ruler's card that waited on the vessel's own rung.
            ("DT_Encounters", "a ruler's card does not wait on the vessel's own rung: the ruler's personal rung never rises (B6a header 1.1, 3.1, decision 9), so a PlayerTierAtLeast of 2 or more on a row put before a ruler never holds and its card never comes - petition_exalt waited on it from [A-2b] to [B-6a6] and now reads the ruled people's rung (CivTierAtLeast). PlayerTierAtLeast 1 always holds, so the enumerator is kept off such a row entirely. The block that raises the personal rung (guideline 16 #1, #141) lifts this rule"):
                (NOTHING, plea),
            ("DT_Encounters", "a wanderer's card is put before a wanderer, about the system the vessel stands in, and checks the vessel: a people's Stability is not the wanderer's (B1)"):
                (NOTHING, "enc_test_weave"),
            ("DT_Encounters", "an arrival card can come on a system no people owns: its texts name the system alone (B1)"): (NOTHING, "enc_test_weave"),
            ("DT_Events", "an event row is about a people or one of its settled systems: the system the vessel stands in is a wanderer card's subject (B1)"): (NOTHING, event),
            ("DT_Encounters", "a card records no death (B5: a death comes only from a hazard's expiry; a card's lethality is 16 #151) and runs no per-tick effect; the effects that grant, place, take or name wait for the rows that use them (guideline 5.10, 7.4)"):
                (NOTHING, plea),
            ("DT_Encounters", "the subject system of a card is the system it names: a Civilization row is about the people alone and names no system (guideline 8.1)"): (NOTHING, plea),
            ("DT_Encounters", "a row the knowing stage puts is picked by its weight among the rows of its moment (guideline 5.10)"): (NOTHING, plea),
            ("DT_Encounters", "a row put by name (the next card of a chain) is never picked: it has no weight"): (update_plea(Trigger="Named", BaseWeight=0), plea),
            ("DT_Encounters", "a card without voices speaks through its Body"): (NOTHING, plea),
            ("DT_Encounters", "only the first contact card is about another people (B3 header section 2.9): the answer gives SetStance its people and fills {OtherCiv} for that card alone - on any other a stance would fall on no one, the name would be empty words, and a wanderer's card would set an AI people's stance (section 2.8)"):
                (NOTHING, plea),
            ("DT_Encounters", "the first meeting puts the first contact card by name, about the ruled people: as a Standing or Lottery row it would come on its own - at the first tick of every reign (B3 header section 2.9)"):
                (NOTHING, "petition_first_contact"),
            ("DT_Encounters", "a voiced card speaks through its two voices, the council's and the elders' of the congregation, and its Body stays empty (guideline 5.10)"):
                (update_plea(VoiceBodies=["{Civ} council.", "{Civ} elders."], Body=""), plea),
            # B4: the edict and the delay of a card's answer, the gate of a choice, and the card that is never blocked.
            ("DT_Encounters", "an edict and a fleet are the ruler's: EnactEdict and DelayHazard stand in a petition's answer, never in a wanderer's card (B4 header 2.2, 3.8)"):
                (NOTHING, "enc_test_weave"),
            ("DT_Encounters", "the gate of a choice reads its OnSuccess alone (CosmicPetitions::ChoiceGateOf): an edict or a delay in OnFailure would stand behind no gate (B4 header 2.4, 3.8)"):
                (NOTHING, plea),
            ("DT_Encounters", "a choice that enacts an edict or buys time is not rolled for: the gate stands before the answer's draw, and a roll would hang the edict or the fleet on Stability (B4 header 2.4, 3.3, 3.8)"):
                (NOTHING, plea),
            ("DT_Encounters", "the edict's own EnactCost is the one price of the choice that enacts it: that choice asks nothing more (B4 header 3.8)"): (NOTHING, plea),
            ("DT_Encounters", "a card is never blocked: with one waiting place a card whose every choice is gated would block every card behind it, so a card with a gated choice (SetStance, EnactEdict, DelayHazard) holds a choice with no cost and no gate - said the other way round: a card without such a choice carries no gate (B4 header 2.4, 3.8)"):
                (update_plea(Choices=[choice(Cost=pool(Energy=5.0)), choice("Wait.", SuccessText="{Civ} waited.", Cost=pool(Matter=1.0))]), plea),
            # B6b [B-6b2a] (header 3.3 rule 8'): a card on the claim does not end it, nor does any card end the claim or an
            # untagged hazard, and one card alone buys it time.
            ("DT_Encounters", "a card on the claim does not end it: a row whose conditions name the claim carries no ResolveHazard, so the theodicy's count of the claims stays as it is (B6b header 2.8, 3.3 rule 8')"):
                (update_plea(Conditions=[cond("PlayerRoleIs", key="Ruler"), cond("SystemHasHazard", tag="Hazard.Claim")]), plea),
            ("DT_Encounters", "nor does any card, whatever its conditions: no answer resolves the claim, nor an untagged hazard (which ends whatever hazard stands - the claim too, on a card whose conditions name no hazard or an untagged one); the theodicy's count reads the claims of DT_Events alone, so this is the cards' side of rule 8' (B6b header 2.8, 3.3 rule 8')"):
                (NOTHING, plea),
            ("DT_Encounters", "the claim's time is bought by one card alone, the fleet of petition_pressure_measured: one card more of 50 ticks would break the theodicy's bound the Data.Integrity invariant keeps (B6b header 2.8, 3.3 rule 8')"):
                (NOTHING, plea),
            ("DT_Events", "grace is never drawn by an edict: no grace row waits on an edict the people holds (B2 header :342, B4 header 3.8)"):
                (update_event(Category="Event.Grace.Jubilee", LogCategory="Log.Civ"), event),
            ("DT_Events", "a transcendent's own line is first person: it waits for the third tier, and the viewers below it hear the row's sign (B4 header 3.8 - a door for the rows to come)"):
                (lambda tables: (self.add_sign_row(tables), update_event(Category="Event.Crisis.Transcendent", LogCategory="Log.Transcendent")(tables)), event),
            ("DT_Edicts", "a held edict damps the pressure's drain on Stability, never undoes it: a factor in (0, 1], and the code reads a factor that is not above 0 as 1 (B4 header 2.3)"):
                (NOTHING, "edict_test_order"),
            # B6b [B-6b2a]: the galaxy law (decision 9) - the fixture's edict made one, a shield with no claim's bonus.
            ("DT_Edicts", "a galaxy law is an edict of the fourth rung whose two hazard hooks reach every settled system of the holder's galaxy: it only shields - a spawn factor at most 1 (its countdown bonus is at least 0 on every row) - and has no hook on the claim (the theodicy's bound W stays), on the extinction or on the supernova's warning (no other people's feat is held back), and no hazard row lists it as an answer (Data.Integrity) (B6b header 2.4, decision 9)"):
                (lambda tables: self.row(tables, "DT_Edicts", "edict_test_order").update(
                    Scope="HomeGalaxy", RequiredTier="Tier4_Transcendent", HazardSpawnMultipliers={"Hazard.Plague": 0.5}, HazardCountdownBonus={"Hazard.Plague": 10}),
                 "edict_test_order"),
            ("DT_Transcendents", "a fallen one counterfeits a truth, and its row names that truth: a tag under Truth. (B4 header 3.5, 3.8)"): (NOTHING, "trans_silence"),
            ("DT_Transcendents", "nothing reads a transcendent's voice lines yet: the column stays empty until the code that tells them comes (B4 header 3.5, 3.8)"):
                (NOTHING, "trans_silence"),
            ("DT_Transcendents", "the silent one does not take the words of the revelation it counterfeits nor of the true rest: no 잠잠, 고요, 쉼, 안식 in its own words (B4 header 3.8, 3.9)"):
                (NOTHING, "trans_silence"),
            ("DT_Transcendents", "a fallen one's claim line speaks to no one: no second person - the player is not called, the holy order and another fallen are not spoken to (B4 header 3.9, the text audit's D1); nor does its lure - the awareness line is the one second person (B6a header 3.2, 3.6)"):
                (NOTHING, "trans_silence"),
            ("DT_LogTemplates", "the sign of a claim that joined a sphere is a sign the code tells, about the claimed system and its people: told below the third rung before the fallen's own line (B4 header 2.6, 3.6)"):
                (NOTHING, "log_sign_claim"),
            ("DT_LogTemplates", "the line of a fleet worn by a higher sphere is the narrator's, with no image of the night's vigil: the vigil is never told as the cause (B4 header 2.7, 3.9)"):
                (NOTHING, "log_fleet_worn"),
            ("DT_Encounters", "a petition's words name no holy word, no soul verb and no rescue word in any field: the name of no god, the faith's gestures in the elders' voice alone (B4 header 3.8, 3.9; MVP item 2)"):
                (NOTHING, plea),
            ("DT_Events", "the vigil's draw row tells a plain day, with no image of the night's vigil: the vigil is never told as the cause of the easing (B4 header 2.8, 3.9)"):
                (NOTHING, "event_claim_eased"),
            # B5 [B-5d1]: the record lines (a row of the category built here: the project's rows are the writers'), the death
            # and inauguration rows.
            ("DT_LogTemplates", "a record line is a subjectless passive about a place or an event, told by the system's name alone: no role slot, no {PopBand}, no {Civ}, no figure - the two books are no roll of a crowd of people - and no word of an afterlife, the book of life, a calling read as a death or the remnant, nor days, names, people or the dead as what was recorded: the object is a place or an event (B5 header 3.3, 3.5, decision 11-ga)"):
                (self.add_record_row, "log_record_test"),
            ("DT_LogTemplates", "the death line is the narrator's, in the third person by the vessel's name and the hazard: the calling's line, focused on no system, with no word of an afterlife, the book of life or a calling read as a death, no 'you' across the death, no name of the holy order or its realm, and no agent, sacrifice or cause of the death (B5 header 3.3, 3.6, decision 12-da)"):
                (NOTHING, "log_death_hazard"),
            ("DT_LogTemplates", "the inauguration line is the narrator's, in the third person by the vessel's name, its people and its system: the calling's line, with no word of an afterlife, the book of life or a calling read as a death, no 'you' across the death, no name of the holy order or its realm, and no word of a return or a rebirth (B5 header 3.3, 3.6, decision 12-da)"):
                (NOTHING, "log_inauguration"),
            # B6b [B-6b2b]: the extinction's record line and its death line.
            ("DT_LogTemplates", "the record of a people that let every system go at an extinction's expiry is a record line, so the record line's rule holds it: {System} alone, no figure, no word of a death, the remnant, nor days, names, people or the dead as what was recorded (B6b header 2.2, 3.3 rule 8, decision 5)"):
                (NOTHING, "log_record_extinct"),
            ("DT_LogTemplates", "the death line of a ruler at an extinction's expiry is the death line in every way: the narrator's, in the third person by the vessel's name and the hazard, the calling's line, with no word of an afterlife, the book of life or a calling read as a death, no 'you' across the death, no name of the holy order or its realm, no agent, sacrifice or cause of the death, no ruler who stayed with them, and no image of a desolation (B6b header 2.2, 3.3 rules 8 and 13, decision 5; notes 1.6 rule 3)"):
                (NOTHING, "log_death_extinction"),
            # B6b [B-6b2c]: the binding's line, the delay above the third rung and a project's Causality.
            ("DT_LogTemplates", "the line of the binding is told in the actorless passive by the name of the system the work stood on alone: no people and no work is named in it - the people is never the binder - and nothing in it triumphs, lifts the shadow, or lets light or dawn rise over it (B6b header 2.5, 3.3 rules 8 and 13, decision 32; notes 1.6 rule 4)"):
                (NOTHING, "log_fallen_bound"),
            # B6b [B-6b3a]: the six nameless variants of the lines that name the other people.
            ("DT_LogTemplates", "the nameless variant of log_siege_on_us names no other people: the code chooses it when that people stands far above the ruled one (CosmicKnowing::IsFarAbove), so it says {Civ} and {System} and never {OtherCiv} - the name the map and the scope of telling hide (B6b header 2.9, 3.3 rule 8, decision 19)"):
                (NOTHING, "log_siege_on_us_far"),
            ("DT_LogTemplates", "the nameless variant of log_treaty_broken names no other people: the code chooses it when that people stands far above the ruled one (CosmicKnowing::IsFarAbove), so it says {Civ} and {Treaty} and never {OtherCiv} - the name the map and the scope of telling hide (B6b header 2.9, 3.3 rule 8, decision 19)"):
                (NOTHING, "log_treaty_broken_far"),
            ("DT_LogTemplates", "the nameless variant of log_siege_lost names no other people: the code chooses it when that people stands far above the ruled one (CosmicKnowing::IsFarAbove), so it says {Civ} and {System} and never {OtherCiv} - the name the map and the scope of telling hide (B6b header 2.9, 3.3 rule 8, decision 19)"):
                (NOTHING, "log_siege_lost_far"),
            ("DT_LogTemplates", "the nameless variant of log_siege_held names no other people: the code chooses it when that people stands far above the ruled one (CosmicKnowing::IsFarAbove), so it says {Civ} and {System} and never {OtherCiv} - the name the map and the scope of telling hide (B6b header 2.9, 3.3 rule 8, decision 19)"):
                (NOTHING, "log_siege_held_far"),
            ("DT_LogTemplates", "the nameless variant of log_siege_lifted names no other people: the code chooses it when that people stands far above the ruled one (CosmicKnowing::IsFarAbove), so it says {Civ} and {System} and never {OtherCiv} - the name the map and the scope of telling hide (B6b header 2.9, 3.3 rule 8, decision 19)"):
                (NOTHING, "log_siege_lifted_far"),
            ("DT_LogTemplates", "the nameless variant of log_stance_lapsed names no other people: the code chooses it when that people stands far above the ruled one (CosmicKnowing::IsFarAbove), so it says {Civ} and never {OtherCiv} - the name the map and the scope of telling hide (B6b header 2.9, 3.3 rule 8, decision 19)"):
                (NOTHING, "log_stance_lapsed_far"),
            ("DT_Rules", "the delay above the third rung multiplies the progress of an AI people's construction of the third rung or above: at 0 or below it would never progress, so the code reads it as 1 with an error line (B6b header 2.6, 3.4, decision 17)"):
                (NOTHING, "rules"),
            ("DT_Megaprojects", "a project asks no Causality: no people earns any (its income is 0 - guideline 16 #90), so a project that asks it would never be paid and its slot would stand for good (B6b header 3.3 rule 7)"):
                (NOTHING, "project_test_build"),
            # B6b [B-6b5]: the words of the upper rungs on single rows (the fifth-rung feat, the extinction, the era named after
            # endured pressure, the binding's work).
            ("DT_Feats", "the fifth rung is a place the people came to stand on, not one it reached or was handed: its feat row's title, narrative and lines carry no passive of the holy order or end-time idiom, and no gold, light, dawn or reign (B6b header 3.3 / notes 1.6 rule 1; decisions 26 and 33)"):
                (update_feat(RequiredTier="Tier4_Transcendent", TargetTier="Tier5_EternalOrder"), "feat_test_climb"),
            ("DT_Hazards", "the extinction's lines tell a loss and end in mourning: no ruler who stayed, no agent, sacrifice or cause of a death, no image of a desolation, a flood, a punishment or the claimant's shadow, and its resolution rescues no one (B6b header 3.3 / notes 1.6 rule 3; decision 33)"):
                (update_hazard(HazardTag="Hazard.Extinction", OnExpireEffects=[], RulerResolutions=["project_test_build"]), hazard),
            ("DT_Events", "an era the people names after its endured pressure is named from its own life and land: no dawn, light, throne or reign, no word of silence, and no desolation, shadow, cold or endurance as merit (B6b header 3.3 / notes 1.6 rule 6; decision 22)"):
                (update_event(Trigger="Standing", BaseWeight=0, Conditions=[cond("SystemOwnedBy", key="Ruled"), cond("CivPressureEnduredTicksReached", key="600")],
                              Effects=[eff("ChangeReverence", key="Borders", value=0.2)]), event),
            # B6b [B-6b8]: the era's shape (decision 22) and the witness's words (decision 20).
            ("DT_Events", "the era of an endured pressure is told for the ruled people alone and raises its neighbours' reverence for the vessel (B6b decision 22, the answer A; header 2.7) - without Ruled every AI people's endurance would lift its neighbours"):
                (update_event(Trigger="Standing", BaseWeight=0, Conditions=[cond("SystemOwnedBy", key="Ruled"), cond("CivPressureEnduredTicksReached", key="600")],
                              Effects=[eff("ChangeReverence", key="Borders", value=0.2)]), event),
            ("DT_Encounters", "a wanderer's card on the claim is a sign seen or measured: it does not testify, record, tell or speak, the sign has no voice, and the step back neither leaves nor shames (B6b header 2.8, notes 1.6 rule 7; decision 20). A substring test: the contracted past forms are listed apart, and a false hit inside a longer word is accepted within this narrow scope"):
                (lambda tables: self.row(tables, "DT_Encounters", "enc_test_weave").update(
                    Trigger="Standing", Conditions=[cond("PlayerRoleIs", key="Wanderer"), cond("PlayerInSystemWithHazard", tag="Hazard.Claim", key="None")]), "enc_test_weave"),
            ("DT_Megaprojects", "the binding's work is named and told as the people's plain work: its name has no tower, ring, fence, binding tool, contact or tuning word and no word another work owns, and its completion line neither binds, triumphs, nor lets light or dawn rise over the shadow (B6b header 3.3 / notes 1.6 rule 4; decision 32; header rule 14)"):
                (NOTHING, "project_binding"),
            # B6b [B-6b6]: the ark's row and the two rules that keep its word to the ark and the fourth-rung feat.
            ("DT_Megaprojects", "the people crosses the extinction in the ark it built; the ark is no one's rescuer - no rescue, title or holy name in its name, no self-rescue, new birth, washing, new land or Flood picture in its completion lines (B6b header 3.3 rule 13, 3.5 rule 2, decision 33)"):
                (NOTHING, "project_ark"),
            ("DT_Megaprojects", "'방주' is the ark's word: no other project is named or told with it - it stands only in the ark and the fourth-rung feat (B6b decision 33)"):
                (NOTHING, "project_test_build"),
            ("DT_Feats", "'방주' is said among the feats by the fourth-rung feat alone, whose deed is the ark: no other feat's title, narrative or lines carry it (B6b decision 33)"):
                (NOTHING, "feat_test_climb"),
            ("DT_LogTemplates", "the inauguration line of a vessel born in a system a claim joined on the same tick is the narrator's, in the third person by the vessel's name and its people, naming no system - no narrator line about that place follows the fallen's own line (rule 5): the calling's line, with no word of an afterlife, the book of life or a calling read as a death, no 'you' across the death, no name of the holy order or its realm, and no word of a return or a rebirth (B5 header 3.3, 3.6, decision 12-da)"):
                (NOTHING, "log_inauguration_unnamed"),
            ("DT_Civs", "arch_default is the name bank of last resort for the vessel: a people whose own bank is empty names its vessel from it, and an empty name would leave the death and inauguration lines without their subject (B5 log_death_hazard, log_inauguration, log_inauguration_unnamed - CosmicChronicle::VesselNameOf)"):
                (NOTHING, "arch_default"),
            ("DT_Transcendents", "no transcendent weighs the claim's events: the vigil's draw on Event.Crisis.Claim keeps a factor of 1, so the bound of the theodicy budget the invariant counts holds (B4 header 2.8)"):
                (NOTHING, "trans_silence"),
            # B7x [B-7x4]: the crossroads words - a card that asks a time vector, an event row that asks one or a bordering
            # people's trait, and the scan face by its name.
            ("DT_Encounters", "a crossroads card tells what a people did and where it then stands: its words name no rest or silence, no grasp of a rung, no title, and none of the fenced words of the crossroads - silence, fence, covenant, ark, shepherd, pilgrimage, sanctuary, the deathless, becoming one (B7x header 3.5, notes 5.4)"):
                (update_plea(Trigger="Lottery", Conditions=[cond("PlayerRoleIs", key="Ruler"), cond("CivHasTrait", tag="Brick.Form.Society"),
                                                            cond("CivTimeVectorAtLeast", key="Built", value=600)], Choices=[choice(), choice("Wait.", OnSuccess=[], SuccessText="{Civ} waited.")]), plea),
            ("DT_Events", "a line of a people's lived way tells no rest or silence and none of the fenced words of the crossroads (B7x header 3.5, notes 5.4)"):
                (update_event(Conditions=[cond("CivTimeVectorAtLeast", key="Built", value=600)]), event),
            ("DT_Events", "a line about a bordering people's brick tells no rest or silence and none of the fenced words of the crossroads (B7x header 3.5, notes 5.4)"):
                (update_event(Conditions=[cond("BorderingCivHasTrait", tag="Brick.Diet.StarEater")]), event),
            ("DT_LogTemplates", "the scan face of a people spread between the stars tells no rest or silence and none of the fenced words of the crossroads, and is named because a log row has no conditions (B7x header 3.5, S15)"):
                (NOTHING, "log_brick_scan_spread"),
            # B7x [B-7x5c] (header section 3.3 rules 4 and 5): a brick on a roll's success or a choice not rolled for, the free
            # choice of a card that gives or takes a brick or asks a time vector, and the trigger and the closing door of the
            # crossroads cards (a card that asks a time vector with the no-value choice it holds).
            ("DT_Encounters", "a brick is given or taken by a choice that is not rolled for, or on the success of a roll - never on its failure: a failed roll changes no people's shape (B7x header section 3.3 rule 4)"):
                (NOTHING, plea),
            ("DT_Encounters", "a card that gives or takes a brick holds a choice with no cost, no check, no effect and no next card: the measuring policies skip every choice that gives or takes a brick and take that one (CosmicRulerPolicy::IsNoValueChoice), so with one waiting place a card whose every choice did would block every card behind it - said the other way round: a card without such a choice gives and takes no brick (B7x header section 3.3 rule 4, section 5)"):
                (update_plea(Choices=[choice(Cost=pool(Energy=5.0)), choice("Wait.", SuccessText="{Civ} waited.", Cost=pool(Matter=1.0))]), plea),
            ("DT_Encounters", "a card that asks a time vector holds a choice with no cost, no check, no effect and no next card too: the measuring policies count it a crossroads card by that condition alone (CosmicRulerPolicy::IsCrossroadsCard) and take that choice, so a card without one would wait in the one waiting place and block every card behind it - said the other way round: a card without such a choice asks no time vector (B7x header section 3.3 rule 4, section 5)"):
                (NOTHING, plea),
            ("DT_Encounters", "a crossroads petition - the petition_cross_ rows, marked by their time vector condition because a rule names a table's own prefix only - comes in the periodic slot: a Standing card is lost while the waiting place is full and comes again at every reign (B7x header section 3.1, 3.3 rule 5)"):
                (update_plea(Trigger="Lottery", Conditions=[cond("PlayerRoleIs", key="Ruler"), cond("CivHasTrait", tag="Brick.Form.Society"),
                                                            cond("CivTimeVectorAtLeast", key="Built", value=1200)], Choices=[choice(), choice("Wait.", OnSuccess=[], SuccessText="{Civ} waited.")]), plea),
            ("DT_Encounters", "a crossroads petition closes on its branch: the first by taking Brick.Form.Society, the second by the tag its first choice gives; its choice 2 carries no repeatable gain; a dismissed card (no cost, no effect) comes again after the last three answers"):
                (update_plea(Trigger="Lottery", Conditions=[cond("PlayerRoleIs", key="Ruler"), cond("CivTimeVectorAtLeast", key="Linked", value=1800),
                                                            cond("CivLacksTrait", tag="Brick.Mutation.DustBand")], Choices=[choice(), choice("Wait.", OnSuccess=[], SuccessText="{Civ} waited.")]), plea),
            ("DT_Encounters", "a wanderer's crossroads card - the enc_cross_ rows, marked by their time vector condition - speaks of the people that owns the system, so it is a Standing card: an arrival card speaks of the place alone and is not held by the gap rule (B7x header section 3.1, 3.3 rule 5)"):
                (lambda tables: self.row(tables, "DT_Encounters", "enc_test_weave").update(
                    Trigger="Standing", Conditions=[cond("PlayerRoleIs", key="Wanderer"), cond("CivTimeVectorAtLeast", key="Drifted", value=900)]),
                 "enc_test_weave"),
            # B7x [B-7x5c] (the text audit's 7.3): the brick lines no condition rule reaches, by their names.
            ("DT_Events", "the brick line of the shut sphere waits on traits alone, so it is named: no rest or silence, none of the fenced words of the crossroads (B7x header 3.5)"):
                (NOTHING, "event_brick_shut_star"),
            ("DT_Events", "the brick line of the wandering swarm's plenty waits on traits alone, so it is named: no rest or silence, none of the fenced words of the crossroads (B7x header 3.5)"):
                (NOTHING, "event_brick_drift_plenty"),
            ("DT_Events", "the brick line of the star eater's trembling star waits on a trait and a hazard, so it is named: no rest or silence, none of the fenced words of the crossroads (B7x header 3.5)"):
                (NOTHING, "event_brick_ember_wake"),
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
        # B7x [B-7x2]: 337 + 35 (K4: one lacks_type and 17 bricks x 2 lacks_cond) = 372.
        # B7x [B-7x4]: 372 + 30 (the crossroads card: 6 columns x 4 lists; two event rules x 2; the scan face x 2) = 402.
        # B7x [B-7x5c]: 402 + 78 (rule 4: 2 rules x 17 bricks x 2 (AddTrait, RemoveTrait) = 68, and the time vector card's free choice: 1; rule 5: 3; the three named brick lines: 6) = 480.
        self.assertEqual(walked, 480, "the number of then-tests in schema.json changed: look at what was added or taken out, then set the count")
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
            ("DT_Hazards", "a system holds the tag and the code finds the row by it (guideline 7.3)"): lambda tables: tables["DT_Hazards"].append(hazard_row("crisis_test_echo")),
            ("DT_Galaxies", "the universe level labels every galaxy with its name: two rows of one name would read as one galaxy (guideline 5.11, 6.4)"):
                lambda tables: self.row(tables, "DT_Galaxies", "gal_m87").update(DisplayName=self.row(tables, "DT_Galaxies", "gal_andromeda")["DisplayName"]),
            ("DT_Galaxies", "two galaxies never stand on one place - the home holds (0,0), so a row left there has no place yet (guideline 5.11, 7.3)"):
                lambda tables: self.row(tables, "DT_Galaxies", "gal_m87").update(MapPosition={"X": 0.0, "Y": 0.0}),
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
                            # A transcendent's own line comes with its tier window and its sign row (B4).
                            fine = lambda tables, key=key, good=good: (  # noqa: E731
                                self.add_sign_row(tables) if good == "Log.Transcendent" else None,
                                self.row(tables, table, "event_test_harvest").update(Category=key, LogCategory=good, Trigger="Named", BaseWeight=0, CooldownTicks=0))
                            self.assertEqual(self.after(fine), [])
                        walked += 1
        self.assertEqual(walked, 9, "the number of event categories in schema.json changed: look at what was added or taken out, then set the count")

    def test_every_target_of_the_schema_can_fail(self):
        def relation_to_default(tables):
            self.row(tables, "DT_Civs", "arch_seeker")["InitialRelations"] = {"arch_default": 0.5}

        def condition_on(type_name):
            return lambda tables: self.row(tables, "DT_Events", "event_test_harvest").update(Conditions=[cond(type_name, key="trans_silence")])
        # Where a target stands -> (what makes the reference, the table and the row it then points at).
        setups = {
            "DT_Civs.InitialRelations": (relation_to_default, "DT_Civs", "arch_default"),
            "DT_Rules.HomeGalaxyRow": (NOTHING, "DT_Galaxies", "gal_milkyway"),
            "DT_Events.SignRow": (self.add_sign_row, "DT_LogTemplates", "log_sign_test"),
            "DT_Hazards.OutbreakRow": (self.set_cell("DT_Hazards", "crisis_test_storm", "OutbreakRow", "event_test_rising"), "DT_Events", "event_test_rising"),
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
        for table, column, column_spec in self.columns_of(lambda spec: "slots" in spec):
            row_name = self.tables[table][0]["Name"]
            value = "{NoSuchSlot}" if column_spec["kind"] == "text" else ["{NoSuchSlot} a.", "b.", "c."]
            with self.subTest(table=table, column=column):
                change, where = self.set_path(table, row_name, column, value)
                self.expect(self.after(change), where, "slot {NoSuchSlot}")
                walked += 1
        self.assertEqual(walked, 31, "the number of slot lists in schema.json changed: look at what was added or taken out, then set the count")


if __name__ == "__main__":
    unittest.main(verbosity=1)
