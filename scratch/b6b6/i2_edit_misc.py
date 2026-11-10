import os
root = r"c:/Users/rhwwl/OneDrive/Documents/Unreal Projects/Cosmic_Ascension/Source/Cosmic_Ascension/Tests"


def edit(name, pairs):
    p = os.path.join(root, name)
    s = open(p, 'rb').read().decode('ascii')
    for old, new in pairs:
        assert s.count(old) == 1, (name, old[:90], s.count(old))
        s = s.replace(old, new)
    open(p, 'wb').write(s.encode('ascii'))
    print("ok", name)


edit("CosmicTestHelpers.h", [(
    "\t// (CosmicHazards::KillsTheRulerOf - B6b decision 5) and stays while a Hazard.Extinction row does (none in the game's data\n"
    "\t// before [B-6b6]); a test that means a world without death takes the B6b rows out too: CosmicTests::WithoutB6bData,\n"
    "\t// outside every WithRules (B6b header section 5.1).\n",
    "\t// (CosmicHazards::KillsTheRulerOf - B6b decision 5) and stays while a Hazard.Extinction row does (the game's\n"
    "\t// crisis_species_extinction from [B-6b6]); a test that means a world without death takes the B6b rows out too:\n"
    "\t// CosmicTests::WithoutB6bData, outside every WithRules (B6b header section 5.1).\n",
)])

edit("CosmicUpperRungTests.cpp", [(
    "\t// The rows of the extinction's boards: the extinction (the game holds no Hazard.Extinction row before [B-6b6]) and a plague\n"
    "\t// that CosmicTests::StandingProject answers, each with three lines for every moment and no effect; the grace row of the\n"
    "\t// remnant is the game's.\n",
    "\t// The rows of the extinction's boards: the extinction of the tests (the board's table replaces the game's, whose\n"
    "\t// crisis_species_extinction is there from [B-6b6]) and a plague that CosmicTests::StandingProject answers, each with three\n"
    "\t// lines for every moment and no effect; the grace row of the remnant is the game's.\n",
)])

edit("CosmicWarningLineTests.cpp", [(
    "\t// row (300 ticks), a supernova of the tests (one tick) and an extinction of the tests (one tick - B6b; the game holds no\n"
    "\t// Hazard.Extinction row before [B-6b6]). The transcendents: WideFallen and NarrowFallen. The edicts: the game's and\n"
    "\t// AnswerEdict (no upkeep).\n",
    "\t// row (300 ticks), a supernova of the tests (one tick) and an extinction of the tests (one tick - B6b; this table replaces\n"
    "\t// the game's, whose crisis_species_extinction is there from [B-6b6]). The transcendents: WideFallen and NarrowFallen. The\n"
    "\t// edicts: the game's and AnswerEdict (no upkeep).\n",
)])

edit("CosmicDeathTests.cpp", [
    ("#include \"Templates/Function.h\"\n#include \"Tests/CosmicTestHelpers.h\"\n",
     "#include \"Templates/Function.h\"\n#include \"Tests/CosmicB6bRowNames.h\"\n#include \"Tests/CosmicTestHelpers.h\"\n"),
    ("\t// (CosmicHazards::KillsTheRulerOf - the extinction; the game holds no such row before [B-6b6], so the quiet copy takes one)\n",
     "\t// (CosmicHazards::KillsTheRulerOf - the extinction; the quiet copy swaps the game's extinction for one of the test's)\n"),
    ("\t\tUDataTable* Hazards = QuietHazards(Original);\n"
     "\t\tFHazardDef Extinction;\n",
     "\t\tUDataTable* Hazards = QuietHazards(Original);\n"
     "\t\tHazards->RemoveRow(CosmicB6bRows::CrisisSpeciesExtinction);\n"
     "\t\tFHazardDef Extinction;\n"),
])
