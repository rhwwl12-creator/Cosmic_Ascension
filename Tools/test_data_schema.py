"""Unit tests of Tools/cosmic_data_schema.py: every kind of rule is fed a wrong input and has to report it.

Cosmic Ascension - data layer helper (guideline.md section 9.3 Data.Integrity).

  python Tools/test_data_schema.py

The base of every case is the data of the project itself under the schema of the project (it has to pass), plus one
valid DT_Events row and two valid DT_Transcendents rows built here, because those tables have no source yet. A case
changes one thing and names the words the report has to carry.
"""
import copy
import json
import os
import shutil
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import cosmic_data_schema as checker  # noqa: E402  (the import needs the path above)

NAN = float("nan")


def tags_text(*names):
    return "(GameplayTags=(%s))" % ",".join('(TagName="%s")' % name for name in names)


def event_row(name="event_test_harvest", **changes):
    row = {
        "Name": name,
        "Category": "Event.Civ.Economy",
        "LogCategory": "Log.Civ",
        "Trigger": "Lottery",
        "Subject": "Civilization",
        "Conditions": [{"Type": "CivTierAtLeast", "Tag": "", "Key": "", "Value": 2}],
        "BaseWeight": 1.0,
        "CooldownTicks": 200,
        "Effects": [{"Type": "AddResource", "Tag": "", "Key": "Energy", "Value": 5, "DurationTicks": 0}],
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
        "PressureEffectsPerTick": [{"Type": "MultiplyIncome", "Tag": "", "Key": "Insight", "Value": 0.7, "DurationTicks": 0}],
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


class SchemaTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.schema, cls.tables, cls.tags = checker.load_project()
        cls.tables = copy.deepcopy(cls.tables)
        cls.tables["DT_Events"] = [event_row()]
        cls.tables["DT_Transcendents"] = [transcendent_row(), celestial_row()]

    def after(self, change, schema_change=None):
        """The problems after one change to a copy of the base tables (and, for a few cases, of the schema)."""
        tables = copy.deepcopy(self.tables)
        schema = copy.deepcopy(self.schema)
        change(tables)
        if schema_change:
            schema_change(schema)
        return checker.check(schema, tables, self.tags)

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

    # ---- the base ----

    def test_project_and_fixture_rows_pass(self):
        self.assertEqual(checker.check_project(), [])
        self.assertEqual(self.after(lambda tables: None), [])

    # ---- tags ----

    def test_tag_not_declared(self):
        self.expect(self.after(self.set_cell("DT_Civs", "arch_default", "Traits", tags_text("Trait.Arch.Default", "Trait.Arch.Nobody"))),
                    "arch_default.Traits", "Trait.Arch.Nobody", "not declared")

    def test_tag_under_wrong_root(self):
        self.expect(self.after(self.set_cell("DT_Civs", "arch_default", "AIProfile", "Trait.Arch.Default")), "AIProfile", "not under AI.Profile.")

    def test_tag_required(self):
        self.expect(self.after(self.set_cell("DT_Civs", "arch_default", "RevelationMode", "")), "RevelationMode", "a tag is required")

    def test_optional_tag_may_be_empty(self):
        self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_terran", "HomeStarClass", "")), [])

    def test_parent_tag_is_not_declared(self):
        self.expect(self.after(self.set_cell("DT_Transcendents", "trans_silence", "EventWeightModifiers", {"Event.Civ": 2})),
                    "EventWeightModifiers[Event.Civ]", "not declared")

    def test_tag_list_and_tag_map(self):
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "DedicationBanks", ["Faith.Bank.Nope"])), "DedicationBanks[0]", "not declared")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "HazardSpawnMultipliers", {"Trait.Arch.Terran": 1})), "not under Hazard.")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "HazardSpawnMultipliers", {"Hazard.Plague": -1})), "is below 0")

    # ---- rows ----

    def test_row_reference(self):
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "PlayerArchetypeRow", "arch_nobody")), "PlayerArchetypeRow", "has no row arch_nobody")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "PlayerArchetypeRow", "")), "a row of DT_Civs is required")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "InitialRelations", {"arch_nobody": 0.5})), "has no row arch_nobody")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "InitialRelations", {"arch_default": 2})), "is above 1")
        # A row never drawn for the AI (Weight 0) is named by PlayerArchetypeRow only.
        self.expect(self.after(self.set_cell("DT_Civs", "arch_default", "InitialRelations", {"arch_terran": 0.5})),
                    "InitialRelations[arch_terran]", "row arch_terran of DT_Civs does not hold", '"Weight"')
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
        self.assertEqual(self.after(lambda tables: tables.pop("DT_Events")), [])

    def test_every_table_source_of_the_folder_is_read(self):
        """A DT_*.json the schema does not name is not skipped: it is a table nobody declared."""
        work = tempfile.mkdtemp(prefix="ca_schema_test_")
        try:
            root = checker.project_root()
            os.makedirs(os.path.join(work, "Config"))
            shutil.copy(os.path.join(root, "Config", "DefaultGameplayTags.ini"), os.path.join(work, "Config"))
            shutil.copytree(os.path.join(root, "Tools", "data"), os.path.join(work, "Tools", "data"))
            self.assertEqual(checker.check_project(work), [])
            with open(os.path.join(work, "Tools", "data", "DT_Nobody.json"), "w", encoding="utf-8") as handle:
                json.dump([{"Name": "nobody_one"}], handle)
            self.expect(checker.check_project(work), "DT_Nobody", "does not declare this table")
        finally:
            shutil.rmtree(work, ignore_errors=True)

    def test_columns_of_a_row(self):
        self.expect(self.after(lambda tables: self.row(tables, "DT_Rules", "rules").update(NewKnob=1)), "rules.NewKnob", "does not declare this column")
        self.expect(self.after(lambda tables: self.row(tables, "DT_Rules", "rules").pop("EventSlotTicks")), "rules.EventSlotTicks", "missing from the row")

    # ---- numbers ----

    def test_number(self):
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "FallenTranscendentCount", 4)), "FallenTranscendentCount", "is above 3")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "FallenTranscendentCount", -1)), "is below 0")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "FallenTranscendentCount", 1.5)), "not a whole number")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "PressureAdjacencyFalloff", NAN)), "finite number")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "LeapTicksMin", "800")), "finite number")
        self.expect(self.after(self.set_cell("DT_Rules", "rules", "LeapTicksMin", True)), "finite number")
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
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "AdjectiveBank", ["a", "b"])), "0 or 3 entries, found 2")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "AdjectiveBank", ["a", "", "c"])), "AdjectiveBank[1]", "a text is required")
        names = ["name %d" % index for index in range(10)]
        self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_terran", "PersonNameBank", names)), [])
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "PersonNameBank", names[:5])), "none or 10 entries and more, found 5")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "PersonNameBank", names[:9] + names[:1])), "appears twice")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "NameBank", "one text")), "list of texts")

    def test_slots(self):
        self.expect(self.after(self.set_cell("DT_LogTemplates", "log_first_life", "Templates", ["{Civv} arrives."])), "slot {Civv}")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "NameBank", ["{Hex} union"])), "slot {Hex}")
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "AdjectiveBank", ["{Civ}", "b", "c"])), "slot {Civ}", "none")
        self.expect(self.after(self.set_cell("DT_Events", "event_test_harvest", "LogTemplates", ["{OtherCiv} a.", "b.", "c."])), "slot {OtherCiv}")
        self.assertEqual(self.after(self.set_cell("DT_LogTemplates", "log_first_life", "Templates", ["{CivAdj} {Civ}."])), [])

    def test_no_digits_in_a_name_pattern(self):
        self.expect(self.after(self.set_cell("DT_Civs", "arch_terran", "NameBank", ["Union 7"])), "NameBank[0]", "a digit")
        self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_terran", "NameBank", ["Union Seven"])), [])

    # ---- row rules, unique, pairs ----

    def test_row_rule(self):
        self.expect(self.after(self.set_cell("DT_Civs", "arch_default", "NameBank", ["union"])), "arch_default", "home star", "{Star}")
        self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_default", "NameBank", ["{Star} union"])), [])
        self.assertEqual(self.after(self.set_cell("DT_Civs", "arch_terran", "NameBank", ["union"])), [])

    def test_unique(self):
        def two_selectors(tables):
            for name in ("log_sign_book", "log_sign_book_again"):
                row = copy.deepcopy(self.row(tables, "DT_LogTemplates", "log_breakthrough_physics"))
                row.update(Name=name, Category="Log.Sign", Context="Revelation.Book")
                tables["DT_LogTemplates"].append(row)
        self.expect(self.after(two_selectors), "log_sign_book_again", "the same Category and Context as row log_sign_book")
        self.expect(self.after(lambda tables: tables["DT_Transcendents"].append(transcendent_row("trans_echo"))), "trans_echo", "the same Doctrine as row trans_silence")

    def test_event_row(self):
        cell = lambda column, value: self.set_cell("DT_Events", "event_test_harvest", column, value)  # noqa: E731
        self.expect(self.after(cell("LogCategory", "Log.Crisis")), "Event.Civ.Economy may tell Log.Civ, found Log.Crisis")
        self.expect(self.after(cell("Trigger", "Sometimes")), "'Sometimes' is no enumerator of ECosmicEventTrigger")
        self.expect(self.after(cell("Trigger", "Standing")), "only a Lottery row has a weight")
        self.expect(self.after(cell("LogTemplates", ["a.", "b."])), "three variants or more")
        self.assertEqual(self.after(cell("LogTemplates", [])), [])

        def named(tables):
            self.row(tables, "DT_Events", "event_test_harvest").update(Trigger="Named", BaseWeight=0, LogTemplates=["one."])
        self.expect(self.after(named), "has no cooldown key")

        def sign(min_tier, category):
            def change(tables):
                row = copy.deepcopy(self.row(tables, "DT_LogTemplates", "log_breakthrough_physics"))
                row.update(Name="log_sign_test", Category=category)
                tables["DT_LogTemplates"].append(row)
                self.row(tables, "DT_Events", "event_test_harvest").update(SignRow="log_sign_test", MinTierToReveal=min_tier)
            return change
        self.assertEqual(self.after(sign(3, "Log.Sign")), [])
        self.expect(self.after(sign(1, "Log.Sign")), "a tier below it", '"ge": 2')
        self.expect(self.after(sign(3, "Log.Civ")), "SignRow", "row log_sign_test of DT_LogTemplates does not hold")
        self.expect(self.after(cell("SignRow", "log_nobody")), "DT_LogTemplates has no row log_nobody")

        def grace(*effects):
            def change(tables):
                self.row(tables, "DT_Events", "event_test_harvest").update(Category="Event.Grace.Jubilee", LogCategory="Log.Civ", Effects=list(effects))
            return change
        rebel = {"Type": "NameNotable", "Tag": "Notable.Rebel"}
        relief = {"Type": "ChangeStability", "Value": 0.2}
        self.assertEqual(self.after(grace(relief)), [])
        self.expect(self.after(grace(relief, rebel)), "a grace row names no one", '"lacks_type"')
        self.assertEqual(self.after(cell("Effects", [relief, rebel])), [])

    def test_transcendent_rows(self):
        cell = lambda name, column, value: self.set_cell("DT_Transcendents", name, column, value)  # noqa: E731
        self.expect(self.after(cell("trans_covenant", "SpawnWeight", 1)), "trans_covenant", "celestial", '"SpawnWeight"')
        self.expect(self.after(cell("trans_covenant", "PressureEffectsPerTick", [{"Type": "ChangeStability", "Value": 0.001}])),
                    "trans_covenant", "celestial", '"PressureEffectsPerTick"')
        self.expect(self.after(cell("trans_covenant", "EventWeightModifiers", {"Event.Civ.Society": 1.5})), "trans_covenant", "celestial", '"EventWeightModifiers"')
        self.expect(self.after(cell("trans_silence", "Tier", "Tier5_EternalOrder")), "tier 3 or 4")
        self.expect(self.after(cell("trans_silence", "Allegiance", "Realm.Celestial")), "fixed place of the roster")
        self.expect(self.after(lambda tables: tables["DT_Transcendents"].pop(0)), "the row trans_silence", "is missing")

    # ---- conditions and effects ----

    def conditions(self, *items):
        return self.after(self.set_cell("DT_Events", "event_test_harvest", "Conditions", list(items)))

    def effects(self, *items):
        return self.after(self.set_cell("DT_Events", "event_test_harvest", "Effects", list(items)))

    def test_conditions(self):
        self.expect(self.conditions({"Type": "CivIsHappy"}), "'CivIsHappy' is no enumerator of conditions")
        self.expect(self.conditions({"Type": "CivTierAtLeast", "Value": 6}), "(CivTierAtLeast).Value", "is above 5")
        self.expect(self.conditions({"Type": "CivTierAtLeast", "Value": 2, "Key": "x"}), "reads no Key")
        self.expect(self.conditions({"Type": "GalaxyTickAtLeast", "Value": 10, "Tag": "Flag.Grace.Spent"}), "reads no Tag")
        self.expect(self.conditions({"Type": "FlagSet", "Tag": "Flag.Grace.Spent", "Value": 1}), "reads no Value")
        self.expect(self.conditions({"Type": "CivHasTrait"}), "(CivHasTrait).Tag", "a tag is required")
        self.expect(self.conditions({"Type": "CivHasTrait", "Tag": "Hazard.Plague"}), "not under Trait.")
        self.expect(self.conditions({"Type": "CivResourceAtLeast", "Key": "Gold", "Value": 1}), "Gold is none of Energy")
        self.expect(self.conditions({"Type": "CivResourceAtLeast", "Value": 1}), "a Key is required")
        for key in ("1,000", "0", "0000000100", "100ticks"):
            self.expect(self.conditions({"Type": "CivStabilityBelowFor", "Key": key, "Value": 0.3}), "not a count of ticks")
        self.assertEqual(self.conditions({"Type": "CivStabilityBelowFor", "Key": "100", "Value": 0.3}), [])
        self.expect(self.conditions({"Type": "CivRelationAtLeast", "Key": "trans_nobody", "Value": 0}), "DT_Transcendents has no row trans_nobody")
        self.assertEqual(self.conditions({"Type": "CivRelationAtLeast", "Key": "trans_covenant", "Value": 0.5}), [])
        self.assertEqual(self.conditions({"Type": "CivRelationAtLeast", "Value": 0.5}), [])
        self.assertEqual(self.conditions({"Type": "SystemHasHazard"}, {"Type": "CivHasNotable", "Tag": "Notable.Rebel", "Key": "Former"}), [])
        self.expect(self.conditions({"Type": "CivHasNotable", "Tag": "Notable.Rebel", "Key": "Latter"}), "Latter is none of Former")
        self.expect(self.conditions("CivTierAtLeast"), "expected an object")

    def test_effects(self):
        self.expect(self.effects({"Type": "MultiplyIncome", "Key": "Energy", "Value": 0.5}), "applied by per-tick sources only")
        self.expect(self.effects({"Type": "AddResource", "Key": "Energy", "Value": 1, "DurationTicks": 5}), "only a per-tick source may carry a duration")
        self.expect(self.effects({"Type": "AddResource", "Key": "Energy", "Value": NAN}), "finite number")
        self.expect(self.effects({"Type": "KillPlayer", "Tag": "Reign.Deposed"}), "not under Death.")
        self.expect(self.effects({"Type": "PlayerStatDelta", "Key": "Body", "Value": 0.5}), "not a whole number")
        self.expect(self.effects({"Type": "ChangeStability", "Value": -2}), "is below -1")
        self.assertEqual(self.effects({"Type": "GrantPower", "Tag": "Way.Received"}, {"Type": "NameNotable", "Tag": "Notable.Rebel"}), [])

    def test_per_tick_column(self):
        def per_tick(*items):
            return self.after(self.set_cell("DT_Transcendents", "trans_silence", "PressureEffectsPerTick", list(items)))
        self.expect(per_tick({"Type": "NameNotable", "Tag": "Notable.Rebel"}), "a per-tick source applies only AddResource")
        self.expect(per_tick({"Type": "KillPlayer", "Tag": "Death.Transcendent"}), "a per-tick source applies only")
        self.assertEqual(per_tick({"Type": "MultiplyIncome", "Value": 0.9, "DurationTicks": 10}), [])
        self.expect(per_tick({"Type": "MultiplyIncome", "Value": -0.2}), "is below 0")

    # ---- the schema itself ----

    def test_schema_is_checked_with_or_without_rows(self):
        nothing = lambda tables: None  # noqa: E731
        no_events = lambda tables: tables.pop("DT_Events")  # noqa: E731

        def odd_kind(schema):
            schema["tables"]["DT_Events"]["columns"]["Severity"] = {"kind": "riddle"}
        self.expect(self.after(nothing, odd_kind), "schema.DT_Events.Severity", "a kind this checker does not know")
        self.expect(self.after(no_events, odd_kind), "schema.DT_Events.Severity", "a kind this checker does not know")

        def odd_test(schema):
            schema["tables"]["DT_Civs"]["rules"][0]["when"] = [{"col": "Weight", "above": 0}]
        self.expect(self.after(nothing, odd_test), "schema.DT_Civs", "a row test is 'row', or a column with exactly one of")

        def odd_then(schema):
            schema["tables"]["DT_Civs"]["rules"][0]["then"] = [{"col": "NameBank", "contains": "{Star}"}]
        self.expect(self.after(nothing, odd_then), "schema.DT_Civs", "a row test is")

        def two_tests(schema):
            schema["tables"]["DT_Civs"]["rules"][0]["when"] = [{"col": "Weight", "gt": 0, "lt": 2}]
        self.expect(self.after(nothing, two_tests), "schema.DT_Civs", "a row test is")

        def odd_target(schema):
            schema["tables"]["DT_Events"]["columns"]["SignRow"]["target"] = [{"col": "Category", "equals": "Log.Sign"}]
        self.expect(self.after(no_events, odd_target), "schema.DT_Events.SignRow", "a row test is")

        def odd_unique(schema):
            schema["tables"]["DT_LogTemplates"]["unique"][0]["when"] = [{"column": "Context", "set": True}]
        self.expect(self.after(nothing, odd_unique), "schema.DT_LogTemplates", "a row test is")

    def test_names_the_schema_leans_on(self):
        nothing = lambda tables: None  # noqa: E731

        def change(path, value):
            def apply(schema):
                node = schema
                for step in path[:-1]:
                    node = node[step]
                node[path[-1]] = value
            return apply
        self.expect(self.after(nothing, change(["tables", "DT_Rules", "columns", "PlayerArchetypeRow", "table"], "DT_Peoples")),
                    "schema.DT_Rules.PlayerArchetypeRow", "the table 'DT_Peoples' is not in the schema")
        self.expect(self.after(nothing, change(["tables", "DT_Rules", "columns", "LureAcceptByPolicy", "words"], "policies")),
                    "the word list 'policies' is not in the schema")
        self.expect(self.after(nothing, change(["tables", "DT_Events", "columns", "Trigger", "enum"], "ETrigger")), "the enum 'ETrigger' is not in the schema")
        self.expect(self.after(nothing, change(["tables", "DT_Events", "pairs", 0, "allowed"], "event_pairs")), "the list 'event_pairs' of a pair")
        self.expect(self.after(nothing, change(["per_tick_effects"], ["AddResource", "Harvest"])), "Harvest is no effect of the schema")
        self.expect(self.after(nothing, change(["conditions", "PlayerRoleIs", "key", "word"], "roles")), "schema.conditions.PlayerRoleIs", "the word list roles")
        self.expect(self.after(nothing, change(["effects", "ChangeRelation", "key", "row"], "DT_Nobody")), "schema.effects.ChangeRelation", "the table DT_Nobody")
        self.expect(self.after(nothing, change(["required_tags"], ["Notable.Rebel", "Notable.Nobody"])), "schema.required_tags", "Notable.Nobody is not declared")

    def test_what_a_rule_names_has_to_exist(self):
        """A test over a misspelt column, tag or enumerator never holds: the rule would be off in silence."""
        nothing = lambda tables: None  # noqa: E731
        put = self.put
        rule = ["tables", "DT_Civs", "rules", 0]
        self.expect(self.after(nothing, put(rule, "when", [{"col": "Wieght", "gt": 0}])), "schema.DT_Civs", "names the column 'Wieght'")
        self.expect(self.after(nothing, put(rule, "then", [{"col": "NameBanks", "each_contains": "{Star}"}])), "schema.DT_Civs", "names the column 'NameBanks'")
        self.expect(self.after(nothing, put(["tables", "DT_LogTemplates", "unique", 0], "when", [{"col": "Contxt", "set": True}])), "schema.DT_LogTemplates", "names the column 'Contxt'")
        self.expect(self.after(nothing, put(["tables", "DT_LogTemplates", "unique", 0], "cols", ["Category", "Contxt"])), "schema.DT_LogTemplates", "unique names the column 'Contxt'")
        # A target is read in the table the column points at, not in its own.
        target = ["tables", "DT_Events", "columns", "SignRow"]
        self.expect(self.after(nothing, put(target, "target", [{"col": "Categry", "is": "Log.Sign"}])), "schema.DT_Events.SignRow", "names the column 'Categry'")
        self.expect(self.after(nothing, put(target, "target", [{"col": "LogCategory", "is": "Log.Sign"}])), "schema.DT_Events.SignRow", "names the column 'LogCategory'")
        self.expect(self.after(nothing, put(target, "target", [{"col": "Category", "is": "Log.Sing"}])), "schema.DT_Events.SignRow", "names the tag Log.Sing")
        self.expect(self.after(nothing, put(["tables", "DT_Events", "pairs", 0], "key", "Categry")), "schema.DT_Events", "a pair names the column 'Categry'")
        self.expect(self.after(nothing, put(["tables", "DT_Events", "pairs", 0], "value", "LogCategry")), "schema.DT_Events", "a pair names the column 'LogCategry'")
        self.expect(self.after(nothing, put(["event_log_categories"], "Event.Civ.Econmy", ["Log.Civ"])), "schema.event_log_categories", "tag Event.Civ.Econmy is not declared")
        self.expect(self.after(nothing, put(["event_log_categories"], "Event.Civ.Economy", ["Log.Civv"])), "schema.event_log_categories", "tag Log.Civv is not declared")
        self.expect(self.after(nothing, put(["tables", "DT_Events", "pairs", 0], "allowed", "version")), "schema.DT_Events", "the list 'version' of a pair")
        grace = ["tables", "DT_Events", "rules", 4]
        self.expect(self.after(nothing, put(grace, "when", [{"col": "Category", "in": ["Event.Grace.Rescue", "Event.Grace.Jubliee"]}])),
                    "schema.DT_Events", "names the tag Event.Grace.Jubliee")
        self.expect(self.after(nothing, put(grace, "when", [{"col": "Category", "in": "Event.Grace.Rescue"}])), "schema.DT_Events", "the test 'in' needs a list")
        self.expect(self.after(nothing, put(["tables", "DT_Events", "rules", 1], "when", [{"col": "Trigger", "in": ["Standing", "Namd"]}])),
                    "schema.DT_Events", "'Namd', which is no enumerator of ECosmicEventTrigger")
        self.expect(self.after(nothing, put(["tables", "DT_Events", "rules", 2], "when", [{"col": "Trigger", "is": "Namd"}])), "schema.DT_Events", "'Namd', which is no enumerator")
        # An empty tag is a value a test may ask for.
        self.assertEqual(self.after(nothing, put(rule, "when", [{"col": "AIProfile", "is": ""}, {"col": "Weight", "gt": 0}])), [])

    def put(self, path, key, value):
        """A change of the schema: one key set under a path."""
        def apply(schema):
            node = schema
            for step in path:
                node = node[step]
            node[key] = value
        return apply

    def test_a_key_nothing_reads_is_a_problem(self):
        nothing = lambda tables: None  # noqa: E731
        put = self.put
        # A misspelt option would switch the check off; an option of another kind would do nothing.
        self.expect(self.after(nothing, put(["tables", "DT_Rules", "columns", "EventSlotTicks"], "mim", 1)), "schema.DT_Rules.EventSlotTicks", "'mim' is not read here")
        self.expect(self.after(nothing, put(["tables", "DT_Rules", "columns", "EventSlotTicks"], "len", 5)), "schema.DT_Rules.EventSlotTicks", "'len' is not read here")
        self.expect(self.after(nothing, put(["tables", "DT_Civs", "columns", "Color"], "min", 0)), "schema.DT_Civs.Color", "allowed: kind")
        self.expect(self.after(nothing, put([], "per_tick_effect", [])), "schema", "'per_tick_effect' is not read here")
        self.expect(self.after(nothing, put(["tables", "DT_Civs"], "prefix", ["arch_"])), "schema.DT_Civs", "'prefix' is not read here")
        self.expect(self.after(nothing, put(["tables", "DT_Civs", "rules", 0], "unless", [])), "schema.DT_Civs.rules", "'unless' is not read here")
        self.expect(self.after(nothing, put(["tables", "DT_LogTemplates", "unique", 0], "columns", [])), "schema.DT_LogTemplates.unique", "'columns' is not read here")
        self.expect(self.after(nothing, put(["tables", "DT_Events", "pairs", 0], "values", [])), "schema.DT_Events.pairs", "'values' is not read here")
        self.expect(self.after(nothing, put(["conditions", "CivTierAtLeast"], "val", {})), "schema.conditions.CivTierAtLeast", "'val' is not read here")
        self.expect(self.after(nothing, put(["conditions", "CivTierAtLeast"], "per_tick_only", True)), "schema.conditions.CivTierAtLeast", "'per_tick_only' is not read here")
        self.expect(self.after(nothing, put(["effects", "ChangeStability", "value"], "minimum", -1)), "schema.effects.ChangeStability.value", "'minimum' is not read here")
        self.expect(self.after(nothing, put(["effects", "AddResource", "key"], "words", "resource")), "schema.effects.AddResource.key", "'words' is not read here")
        self.expect(self.after(nothing, put(["conditions", "CivHasTrait", "tag"], "roots", "Trait.")), "schema.conditions.CivHasTrait.tag", "'roots' is not read here")

    def test_a_part_of_the_wrong_shape_is_a_problem_line(self):
        """Not a traceback: every one of these runs to the end and names the place."""
        nothing = lambda tables: None  # noqa: E731
        put = self.put
        self.expect(self.after(nothing, put(["tables", "DT_Civs", "columns"], "Weight", "number")), "schema.DT_Civs.Weight", "expected an object with a kind")
        self.expect(self.after(nothing, put(["tables"], "DT_Feats", "feat_")), "schema.DT_Feats", "expected an object")
        self.expect(self.after(nothing, put(["tables", "DT_Civs", "rules"], 0, "Weight > 0")), "schema.DT_Civs.rules", "expected an object")
        self.expect(self.after(nothing, put(["tables", "DT_LogTemplates", "unique"], 0, "Category")), "schema.DT_LogTemplates.unique", "expected an object")
        self.expect(self.after(nothing, put(["tables", "DT_Events", "pairs"], 0, "Category")), "schema.DT_Events.pairs", "expected an object")
        self.expect(self.after(nothing, put(["conditions"], "CivTierAtLeast", "value")), "schema.conditions.CivTierAtLeast", "expected an object")
        self.expect(self.after(nothing, put(["effects", "ChangeStability"], "value", "any")), "schema.effects.ChangeStability.value", "expected an object")

    def test_lists_of_the_checker_match_the_schema_text(self):
        about = " ".join(self.schema["about"])
        for kind in checker.COLUMN_KINDS:
            self.assertIn(kind, about)
        for name in checker.TESTS + ("'row'",):
            self.assertIn(name, about)


if __name__ == "__main__":
    unittest.main(verbosity=1)
