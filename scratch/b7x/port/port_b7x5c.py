"""B7x [B-7x5c] data port: the crossroads cards, the brick event rows, the first life's start mark and their schema rules.

Usage:  python -I -X utf8 port_b7x5c.py <tree-root> [--dry-run] [--no-source-check]

Run it on the re-stacked tree (main after B6b plus the B7x commits up to [B-7x5s]), never before block B6b ends (B7x header
section 8, row 7). It writes, by key and row name, never by line number, keeping each file's bytes outside what it adds:

  Tools/data/DT_Encounters.json  the twelve crossroads rows of aa09543 appended at the end of the table (petition_cross_*
                                 eleven, enc_cross_drift_fleet) - conditions, costs, checks and effects of aa09543 with the
                                 new leaf names of decision 3 (rename_leaves.py's table), every text field from the audited
                                 texts (stage3/texts/audited_texts.md section 1), the effects of choice 2 of C05, C07, C10
                                 and C11 exactly as the lead's rulings (stage3/texts/lead_rulings.md section 1; decision 11
                                 (na)), C12's title '집을 싣는 배' and its choice 2 kept (ChangeReverence +0.05 and aa09543's
                                 success line). The voice tilts stay everywhere (the rulings narrow the audit's 6.1).
  Tools/data/DT_Events.json      the nine event_brick_* rows of aa09543 at the end, new leaf names, lines from the audited
                                 texts section 2 (values, weights, cooldowns and severities untouched).
  Tools/data/DT_Civs.json        Brick.Form.Society appended to arch_seeker's Traits.
  Tools/data/schema.json         DT_Encounters: header 3.3 rule 4 (two rules - the 17 bricks' lacks_cond lists sorted by tag,
                                 new names) and rule 5 (three rules; the closing rule's why as notes 5.4 data-1 gives it);
                                 DT_Events: the three row-name word rules of the audited texts 7.3 (event_brick_shut_star,
                                 _drift_plenty, _ember_wake - the rows no condition rule reaches).
  Tools/test_data_schema.py      fixture rows for the three named event rows, the eight rule_setups entries, the test
                                 test_crossroads_cards (aa09543's, new names and the new why) and test_crossroads_brick_lines;
                                 and three cases of earlier tests the new rules reach (REPLACEMENTS: [B-7x4]'s crossroads
                                 card becomes a first crossroads Lottery row in test_crossroads_words and in its rule_setups
                                 entry; test_tables appends its wrong DT_Events row instead of replacing the table, and its
                                 absent DT_Events takes the three named brick rules with it).

Rules: idempotent (a second run reports 0 changes); a row, rule, entry or test that already exists with other content is
a conflict - the run refuses and writes nothing; --dry-run writes nothing and prints everything. Every change is printed.
The leaf names are written new, so rename_leaves.py finds nothing in what this adds (run it anyway at the re-stack).

Left for the person who runs it (printed at the end):
  - the walked pin of test_every_rule_of_the_schema_can_fail is not touched: run Tools/test_data_schema.py, read the count
    the walk reports and set it (the run prints the count it expects from the schema's then-tests);
  - the .uasset tables and Tools/data/generated.sha1 come from make_data_tables.py inside the editor
    (Tools\\run_checks.ps1 -Tier full -Data, editor closed);
  - the documents of the same commit (design 4.3, notes 5.4 data-1/data-2, header 11) are edited by hand.

The embedded sources (AA_ENCOUNTERS, AA_EVENTS, AUDITED, RULED_EFFECTS) were copied by src/gen_port.py. When the scratch
sources are still on this PC the run compares them first (git show aa09543 in the integration worktree, audited_texts.md,
lead_rulings.md) and refuses on a difference; --no-source-check skips that when they are gone.
"""
import copy
import json
import os
import re
import subprocess
import sys

B7X_SCRATCH = "C:/Users/rhwwl/AppData/Local/Temp/claude/c--Users-rhwwl-OneDrive-Documents-Unreal-Projects-Cosmic-Ascension/37de2919-0c44-45d8-94cd-4dcf0860263e/scratchpad/b7x"
SOURCE_REPO = B7X_SCRATCH + "/int"
SOURCE_COMMIT = "aa09543"
AUDITED_MD = B7X_SCRATCH + "/stage3/texts/audited_texts.md"
RULINGS_MD = B7X_SCRATCH + "/stage3/texts/lead_rulings.md"

# Decision 3 (rename_leaves.py's LEAVES): (axis, old leaf, new leaf).
LEAVES = [
    ("Form", "UnseenLaw", "KeptLaw"),
    ("Diet", "SilenceFeed", "IdleFeed"),
    ("Space", "PlanckMicro", "FoldedSmall"),
    ("Form", "DysonBrain", "ShutSphere"),
    ("Mutation", "KesslerTapestry", "DustBand"),
    ("Mutation", "AbyssalMaw", "InwardHunger"),
    ("Mutation", "StarShepherd", "StarTow"),
    ("Space", "HermitMonolith", "ClosedPort"),
]
RENAME = {"Brick.%s.%s" % (axis, old): "Brick.%s.%s" % (axis, new) for axis, old, new in LEAVES}
OLD_LEAF = re.compile("|".join(old for _, old, _ in LEAVES))

# The seventeen brick tags (B7x header 2.4, Config/DefaultGameplayTags.ini), new names; the rules list them sorted.
BRICKS = sorted([
    "Brick.Form.Society", "Brick.Form.LivingNebula", "Brick.Form.ShutSphere", "Brick.Form.KeptLaw",
    "Brick.Diet.DarkMatter", "Brick.Diet.StarEater", "Brick.Diet.IdleFeed", "Brick.Diet.DistanceTension", "Brick.Diet.CrisisParasite",
    "Brick.Space.Nomad", "Brick.Space.ClosedPort", "Brick.Space.FoldedSmall",
    "Brick.Instinct.SwarmTech", "Brick.Instinct.HiveMerge",
    "Brick.Mutation.StarTow", "Brick.Mutation.DustBand", "Brick.Mutation.InwardHunger",
])

# --- embedded sources (src/gen_port.py) -------------------------------------------------------------------------------
AA_ENCOUNTERS = json.loads(r'''[
 {
  "Name": "petition_cross_built",
  "Title": "작은 손들",
  "Body": "",
  "Conditions": [
   {
    "Type": "PlayerRoleIs",
    "Tag": "",
    "Key": "Ruler",
    "Value": 0.0
   },
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Form.Society",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivTimeVectorAtLeast",
    "Tag": "",
    "Key": "Built",
    "Value": 1200.0
   }
  ],
  "BaseWeight": 1.0,
  "Choices": [
   {
    "Label": "집집마다 연장을 들인다",
    "CheckStat": "Stability",
    "Difficulty": 8,
    "Cost": {
     "Energy": 0.0,
     "Matter": 240.0,
     "Insight": 120.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "AddTrait",
      "Tag": "Brick.Instinct.SwarmTech",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "RemoveTrait",
      "Tag": "Brick.Form.Society",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "ChangeStability",
      "Tag": "",
      "Key": "None",
      "Value": -0.06,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": -0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [
     {
      "Type": "ChangeStability",
      "Tag": "",
      "Key": "None",
      "Value": -0.06,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": -0.05,
      "DurationTicks": 0
     }
    ],
    "SuccessText": "{Civ}의 집마다 작은 공방이 섰다. 그 일이 끝났을 때 큰 공사는 집집의 작은 손에 나뉘어 있었다.",
    "FailureText": "{Civ}의 집집에 연장이 들었지만 손에 익지 않았다. 큰 공사는 장인들의 몫으로 남았고 거리는 술렁였다.",
    "NextEncounter": "None"
   },
   {
    "Label": "장인 조합에만 새 연장을 맡긴다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 100.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "RemoveTrait",
      "Tag": "Brick.Form.Society",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "AddResource",
      "Tag": "",
      "Key": "Matter",
      "Value": 150.0,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": 0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [],
    "SuccessText": "{Civ}의 장인 조합이 새 연장을 맡았다. 그들의 공방에서 나온 것이 창고에 쌓였다.",
    "FailureText": "",
    "NextEncounter": "None"
   },
   {
    "Label": "지켜본다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [],
    "OnFailure": [],
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 작업장의 일은 하던 대로 이어졌다.",
    "FailureText": "",
    "NextEncounter": "None"
   }
  ],
  "Illustration": "",
  "Trigger": "Lottery",
  "Subject": "Civilization",
  "VoiceBodies": [
   "{Civ}의 작업장은 오래 불을 끄지 않았다. 의회는 큰 공사를 잘게 나누어 집집마다 연장을 들이자고 청했다.",
   "{Civ}의 작업장은 오래 불을 끄지 않았다. 회중의 장로들은 새 연장을 가르칠 수 있는 장인들의 손에만 맡기자고 청했다."
  ]
 },
 {
  "Name": "petition_cross_consumed",
  "Title": "땅이 내는 것",
  "Body": "",
  "Conditions": [
   {
    "Type": "PlayerRoleIs",
    "Tag": "",
    "Key": "Ruler",
    "Value": 0.0
   },
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Form.Society",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivTimeVectorAtLeast",
    "Tag": "",
    "Key": "Consumed",
    "Value": 1500.0
   }
  ],
  "BaseWeight": 1.0,
  "Choices": [
   {
    "Label": "암흑 구간에 체를 편다",
    "CheckStat": "Stability",
    "Difficulty": 9,
    "Cost": {
     "Energy": 260.0,
     "Matter": 120.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "AddTrait",
      "Tag": "Brick.Diet.DarkMatter",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "RemoveTrait",
      "Tag": "Brick.Form.Society",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "ChangeStability",
      "Tag": "",
      "Key": "None",
      "Value": -0.08,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": 0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [
     {
      "Type": "ChangeStability",
      "Tag": "",
      "Key": "None",
      "Value": -0.08,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": 0.05,
      "DurationTicks": 0
     }
    ],
    "SuccessText": "{Civ}의 배들이 암흑 구간에 넓은 체를 폈다. 그 일이 끝났을 때 그들의 화로는 별 사이에서 걸러 낸 것으로 타고 있었다.",
    "FailureText": "{Civ}의 배들이 암흑 구간에 편 체에는 걸리는 것이 없었다. 광맥 곁의 집들은 들인 품을 두고 술렁였다.",
    "NextEncounter": "None"
   },
   {
    "Label": "제 별의 불을 화로에 끌어들인다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 200.0,
     "Insight": 0.0,
     "Influence": 100.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "AddTrait",
      "Tag": "Brick.Diet.StarEater",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "RemoveTrait",
      "Tag": "Brick.Form.Society",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "ChangeReverence",
      "Tag": "",
      "Key": "Borders",
      "Value": -0.1,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": -0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [],
    "SuccessText": "{Civ}의 화로가 제 별의 불을 끌어 쓰기 시작했다. 국경 너머의 이웃들은 그 별빛을 오래 바라보았다.",
    "FailureText": "",
    "NextEncounter": "None"
   },
   {
    "Label": "지켜본다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [],
    "OnFailure": [],
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 광맥의 일은 하던 대로 이어졌다.",
    "FailureText": "",
    "NextEncounter": "None"
   }
  ],
  "Illustration": "",
  "Trigger": "Lottery",
  "Subject": "Civilization",
  "VoiceBodies": [
   "{Civ}의 광맥은 오래 싸움에 쓸 것을 대느라 깊어졌다. 의회는 땅을 더 파지 말고 제 별의 불을 바로 화로에 끌어들이자고 청했다.",
   "{Civ}의 광맥은 오래 싸움에 쓸 것을 대느라 깊어졌다. 회중의 장로들은 땅을 그만 파고 별과 별 사이에 떠도는 엷은 것을 걸러 살자고 청했다."
  ]
 },
 {
  "Name": "petition_cross_drifted",
  "Title": "닻을 내리는 곳",
  "Body": "",
  "Conditions": [
   {
    "Type": "PlayerRoleIs",
    "Tag": "",
    "Key": "Ruler",
    "Value": 0.0
   },
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Form.Society",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivTimeVectorAtLeast",
    "Tag": "",
    "Key": "Drifted",
    "Value": 900.0
   }
  ],
  "BaseWeight": 1.0,
  "Choices": [
   {
    "Label": "살림을 통째로 배에 싣는다",
    "CheckStat": "Stability",
    "Difficulty": 9,
    "Cost": {
     "Energy": 100.0,
     "Matter": 300.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "AddTrait",
      "Tag": "Brick.Space.Nomad",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "RemoveTrait",
      "Tag": "Brick.Form.Society",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "ChangeStability",
      "Tag": "",
      "Key": "None",
      "Value": -0.1,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": 0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [
     {
      "Type": "ChangeStability",
      "Tag": "",
      "Key": "None",
      "Value": -0.1,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": 0.05,
      "DurationTicks": 0
     }
    ],
    "SuccessText": "{Civ}에서 배에 사는 집이 뭍의 집보다 많아졌다. 그 일이 끝났을 때 그들은 닻을 내린 곳을 집이라 부르지 않고 있었다.",
    "FailureText": "{Civ}의 배에 살림을 실었지만 뭍을 떠나려는 집은 적었다. 실은 짐을 도로 내리는 동안 항구가 어수선했다.",
    "NextEncounter": "None"
   },
   {
    "Label": "떠나려는 집에만 배를 내준다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 140.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "RemoveTrait",
      "Tag": "Brick.Form.Society",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "ChangeStability",
      "Tag": "",
      "Key": "None",
      "Value": 0.05,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": -0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [],
    "SuccessText": "{Civ}의 배가 떠나려는 집들만 태우고 항로에 올랐다. 뭍에 남은 집들은 하던 살림을 이어 갔다.",
    "FailureText": "",
    "NextEncounter": "None"
   },
   {
    "Label": "지켜본다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [],
    "OnFailure": [],
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 새 터를 닦는 배들은 하던 대로 오갔다.",
    "FailureText": "",
    "NextEncounter": "None"
   }
  ],
  "Illustration": "",
  "Trigger": "Lottery",
  "Subject": "Civilization",
  "VoiceBodies": [
   "{Civ}의 배들은 오래 새 터를 닦으러 나가 있었다. 의회는 떠나려는 집은 보내고 남으려는 집은 남게 두자고 청했다.",
   "{Civ}의 배들은 오래 새 터를 닦으러 나가 있었다. 회중의 장로들은 닿는 곳마다 집이 함께 가야 한다며 살림을 통째로 배에 싣자고 청했다."
  ]
 },
 {
  "Name": "petition_cross_isolated",
  "Title": "항구의 빗장",
  "Body": "",
  "Conditions": [
   {
    "Type": "PlayerRoleIs",
    "Tag": "",
    "Key": "Ruler",
    "Value": 0.0
   },
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Form.Society",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivTimeVectorAtLeast",
    "Tag": "",
    "Key": "Isolated",
    "Value": 1500.0
   }
  ],
  "BaseWeight": 1.0,
  "Choices": [
   {
    "Label": "항구를 닫고 새 터를 찾지 않는다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 200.0,
     "Insight": 0.0,
     "Influence": 150.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "AddTrait",
      "Tag": "Brick.Space.HermitMonolith",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "RemoveTrait",
      "Tag": "Brick.Form.Society",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "ChangeReverence",
      "Tag": "",
      "Key": "Borders",
      "Value": -0.1,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": 0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [],
    "SuccessText": "{Civ}의 항구들이 닫히고 새 터로 떠날 배들이 항구에 묶였다. 그 일이 끝났을 때 그들은 제 별 안쪽만 짓고 있었다.",
    "FailureText": "",
    "NextEncounter": "None"
   },
   {
    "Label": "큰 항구만 남기고 닫는다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 80.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "RemoveTrait",
      "Tag": "Brick.Form.Society",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "AddResource",
      "Tag": "",
      "Key": "Insight",
      "Value": 120.0,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": -0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [],
    "SuccessText": "{Civ}의 작은 항구들이 닫히고 큰 항구만 열려 있었다. 바깥으로 쓰던 손이 안을 짓는 셈으로 돌았다.",
    "FailureText": "",
    "NextEncounter": "None"
   },
   {
    "Label": "지켜본다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [],
    "OnFailure": [],
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 항구들은 하던 대로 열려 있었다.",
    "FailureText": "",
    "NextEncounter": "None"
   }
  ],
  "Illustration": "",
  "Trigger": "Lottery",
  "Subject": "Civilization",
  "VoiceBodies": [
   "{Civ}의 백성은 오래 저희끼리 지냈다. 의회는 큰 항구만 남기고 나머지를 닫아 안을 짓는 데 힘을 모으자고 청했다.",
   "{Civ}의 백성은 오래 저희끼리 지냈다. 회중의 장로들은 바깥의 것이 들면 지켜 온 것이 흐려진다며 항구를 모두 닫자고 청했다."
  ]
 },
 {
  "Name": "petition_cross_dust_band",
  "Title": "별 둘레의 티끌 띠",
  "Body": "",
  "Conditions": [
   {
    "Type": "PlayerRoleIs",
    "Tag": "",
    "Key": "Ruler",
    "Value": 0.0
   },
   {
    "Type": "CivTierAtLeast",
    "Tag": "",
    "Key": "None",
    "Value": 3.0
   },
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Instinct.SwarmTech",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivLacksTrait",
    "Tag": "Brick.Mutation.KesslerTapestry",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivLacksTrait",
    "Tag": "Brick.Instinct.HiveMerge",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivTimeVectorAtLeast",
    "Tag": "",
    "Key": "Linked",
    "Value": 1800.0
   }
  ],
  "BaseWeight": 1.0,
  "Choices": [
   {
    "Label": "일꾼들을 별마다 띠로 두른다",
    "CheckStat": "Stability",
    "Difficulty": 9,
    "Cost": {
     "Energy": 150.0,
     "Matter": 350.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "AddTrait",
      "Tag": "Brick.Mutation.KesslerTapestry",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": -0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [
     {
      "Type": "ChangeStability",
      "Tag": "",
      "Key": "None",
      "Value": -0.05,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": -0.05,
      "DurationTicks": 0
     }
    ],
    "SuccessText": "{Civ}의 별마다 티끌 같은 일꾼들이 띠를 이루었다. 띠는 스스로를 고치며 돌았고, 그 일을 다 아는 사람은 없었다.",
    "FailureText": "{Civ}의 일꾼들은 띠를 이루지 못하고 흩어졌다. 들인 것을 두고 의회의 말이 엇갈렸다.",
    "NextEncounter": "None"
   },
   {
    "Label": "사람을 먼저 기른다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 150.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "ChangePopulation",
      "Tag": "",
      "Key": "None",
      "Value": 0.03,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": 0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [],
    "SuccessText": "{Civ}의 물자가 집과 배움터를 짓는 데 들었다. 집집에 식구가 늘었다.",
    "FailureText": "",
    "NextEncounter": "None"
   },
   {
    "Label": "지켜본다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [],
    "OnFailure": [],
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 작은 일꾼들은 하던 대로 일했다.",
    "FailureText": "",
    "NextEncounter": "None"
   }
  ],
  "Illustration": "",
  "Trigger": "Lottery",
  "Subject": "Civilization",
  "VoiceBodies": [
   "{Civ}의 작은 일꾼들은 스스로를 고쳐 가며 일한다. 의회는 그 일꾼들을 별마다 띠로 둘러 손이 닿지 않던 일까지 맡기자고 청했다.",
   "{Civ}의 작은 일꾼들은 스스로를 고쳐 가며 일한다. 회중의 장로들은 누구의 손에도 매이지 않는 것을 늘리기 전에 사람을 먼저 기르자고 청했다."
  ]
 },
 {
  "Name": "petition_cross_star_line",
  "Title": "별에 거는 줄",
  "Body": "",
  "Conditions": [
   {
    "Type": "PlayerRoleIs",
    "Tag": "",
    "Key": "Ruler",
    "Value": 0.0
   },
   {
    "Type": "CivTierAtLeast",
    "Tag": "",
    "Key": "None",
    "Value": 3.0
   },
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Space.Nomad",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivLacksTrait",
    "Tag": "Brick.Mutation.StarShepherd",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivTimeVectorAtLeast",
    "Tag": "",
    "Key": "Drifted",
    "Value": 1800.0
   }
  ],
  "BaseWeight": 1.0,
  "Choices": [
   {
    "Label": "도읍의 별에 끄는 줄을 건다",
    "CheckStat": "Stability",
    "Difficulty": 9,
    "Cost": {
     "Energy": 300.0,
     "Matter": 150.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "AddTrait",
      "Tag": "Brick.Mutation.StarShepherd",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": -0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [
     {
      "Type": "ChangeStability",
      "Tag": "",
      "Key": "None",
      "Value": -0.06,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": -0.05,
      "DurationTicks": 0
     }
    ],
    "SuccessText": "{Civ}|hpp(이,가) 도읍의 별에 끄는 줄을 걸었다. 별은 제자리에 있었고, 그들은 떠날 채비를 한 채 살게 되었다.",
    "FailureText": "{Civ}의 배들이 도읍의 별에 줄을 걸다가 놓쳤다. 끊어진 줄을 두고 배 위의 집들이 술렁였다.",
    "NextEncounter": "None"
   },
   {
    "Label": "집부터 고쳐 짓는다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 250.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "ChangeStability",
      "Tag": "",
      "Key": "None",
      "Value": 0.05,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": 0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [],
    "SuccessText": "{Civ}의 배 위 집들이 새로 고쳐졌다. 그 안에서 사는 날이 한결 편해졌다.",
    "FailureText": "",
    "NextEncounter": "None"
   },
   {
    "Label": "지켜본다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [],
    "OnFailure": [],
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 배 위의 살림은 하던 대로 꾸려졌다.",
    "FailureText": "",
    "NextEncounter": "None"
   }
  ],
  "Illustration": "",
  "Trigger": "Lottery",
  "Subject": "Civilization",
  "VoiceBodies": [
   "{Civ}의 살림은 오래 배 위에서 꾸려졌다. 의회는 도읍의 별에 끄는 줄을 걸어 데리고 떠날 채비를 하자고 청했다.",
   "{Civ}의 살림은 오래 배 위에서 꾸려졌다. 회중의 장로들은 떠날 채비로 지은 집에서는 아무도 늙지 못한다며 집부터 고치자고 청했다."
  ]
 },
 {
  "Name": "petition_cross_between_stars",
  "Title": "별 사이의 엷은 것",
  "Body": "",
  "Conditions": [
   {
    "Type": "PlayerRoleIs",
    "Tag": "",
    "Key": "Ruler",
    "Value": 0.0
   },
   {
    "Type": "CivTierAtLeast",
    "Tag": "",
    "Key": "None",
    "Value": 3.0
   },
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Diet.DarkMatter",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivLacksTrait",
    "Tag": "Brick.Form.LivingNebula",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivTimeVectorAtLeast",
    "Tag": "",
    "Key": "Consumed",
    "Value": 2000.0
   }
  ],
  "BaseWeight": 1.0,
  "Choices": [
   {
    "Label": "살림을 별들 사이에 푼다",
    "CheckStat": "Stability",
    "Difficulty": 9,
    "Cost": {
     "Energy": 300.0,
     "Matter": 0.0,
     "Insight": 150.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "AddTrait",
      "Tag": "Brick.Form.LivingNebula",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": 0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [
     {
      "Type": "ChangeStability",
      "Tag": "",
      "Key": "None",
      "Value": -0.06,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": 0.05,
      "DurationTicks": 0
     }
    ],
    "SuccessText": "{Civ}|hpp(이,가) 살림을 별들 사이의 엷은 것에 풀었다. 그 일이 끝났을 때 그들은 별과 별 사이에 번져 서 있었다.",
    "FailureText": "{Civ}|hpp(이,가) 별들 사이에 푼 살림은 번지지 못하고 흩어졌다. 땅 위의 집들이 그 일을 두고 술렁였다.",
    "NextEncounter": "None"
   },
   {
    "Label": "땅 위의 집을 고쳐 짓는다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 250.0,
     "Insight": 0.0,
     "Influence": 60.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "ChangeStability",
      "Tag": "",
      "Key": "None",
      "Value": 0.04,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Openness",
      "Value": -0.05,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": -0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [],
    "SuccessText": "{Civ}의 땅 위 집들이 지붕부터 고쳐졌다. 그들은 디딜 곳을 지키며 바깥의 것을 덜 들였다.",
    "FailureText": "",
    "NextEncounter": "None"
   },
   {
    "Label": "지켜본다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [],
    "OnFailure": [],
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 별 사이에서 걸러 내는 일은 하던 대로 이어졌다.",
    "FailureText": "",
    "NextEncounter": "None"
   }
  ],
  "Illustration": "",
  "Trigger": "Lottery",
  "Subject": "Civilization",
  "VoiceBodies": [
   "{Civ}의 살림은 별 사이에서 걸러 낸 것으로 오래 꾸려졌다. 의회는 지붕부터 고쳐 땅 위에 디딜 곳을 지키자고 청했다.",
   "{Civ}의 살림은 별 사이에서 걸러 낸 것으로 오래 꾸려졌다. 회중의 장로들은 땅에 매인 살림을 별들 사이의 엷은 것에 풀자고 청했다."
  ]
 },
 {
  "Name": "petition_cross_into_dust",
  "Title": "티끌 속의 넓이",
  "Body": "",
  "Conditions": [
   {
    "Type": "PlayerRoleIs",
    "Tag": "",
    "Key": "Ruler",
    "Value": 0.0
   },
   {
    "Type": "CivTierAtLeast",
    "Tag": "",
    "Key": "None",
    "Value": 3.0
   },
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Space.Nomad",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivLacksTrait",
    "Tag": "Brick.Space.PlanckMicro",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivLacksTrait",
    "Tag": "Brick.Mutation.StarShepherd",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivTimeVectorAtLeast",
    "Tag": "",
    "Key": "Endured",
    "Value": 1800.0
   }
  ],
  "BaseWeight": 1.0,
  "Choices": [
   {
    "Label": "티끌 속으로 접혀 든다",
    "CheckStat": "Stability",
    "Difficulty": 9,
    "Cost": {
     "Energy": 300.0,
     "Matter": 0.0,
     "Insight": 200.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "AddTrait",
      "Tag": "Brick.Space.PlanckMicro",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "RemoveTrait",
      "Tag": "Brick.Space.Nomad",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": -0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [
     {
      "Type": "ChangeStability",
      "Tag": "",
      "Key": "None",
      "Value": -0.06,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": -0.05,
      "DurationTicks": 0
     }
    ],
    "SuccessText": "{Civ}|hpp(이,가) 살림을 가장 작은 눈금 속으로 접어 넣었다. 그 일이 끝났을 때 그들은 티끌 속에 서 있었고, 떠돌던 배들은 비어 있었다.",
    "FailureText": "{Civ}|hpp(이,가) 살림을 접어 넣으려 했지만 눈금은 열리지 않았다. 떠돌던 배 위의 집들이 술렁였다.",
    "NextEncounter": "None"
   },
   {
    "Label": "별 곁에 디딜 곳을 마련한다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 200.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "ChangeStability",
      "Tag": "",
      "Key": "None",
      "Value": 0.05,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": 0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [],
    "SuccessText": "{Civ}의 배들이 별 곁에 디딜 곳을 닦았다. 떠돌던 집들이 그곳에서 숨을 골랐다.",
    "FailureText": "",
    "NextEncounter": "None"
   },
   {
    "Label": "지켜본다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [],
    "OnFailure": [],
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 배들은 하던 대로 항로를 떠돌았다.",
    "FailureText": "",
    "NextEncounter": "None"
   }
  ],
  "Illustration": "",
  "Trigger": "Lottery",
  "Subject": "Civilization",
  "VoiceBodies": [
   "{Civ}|hpp(은,는) 떠도는 동안 견딘 날이 길었다. 의회는 가장 작은 눈금 속에 다투는 이 없는 넓이가 있다며 그리로 접혀 들자고 청했다.",
   "{Civ}|hpp(은,는) 떠도는 동안 견딘 날이 길었다. 회중의 장로들은 밖을 내다보지 않는 살림은 밖의 일을 잊는다며 별 곁에 디딜 곳을 마련하자고 청했다."
  ]
 },
 {
  "Name": "petition_cross_wrapped_star",
  "Title": "닫힌 구의 셈",
  "Body": "",
  "Conditions": [
   {
    "Type": "PlayerRoleIs",
    "Tag": "",
    "Key": "Ruler",
    "Value": 0.0
   },
   {
    "Type": "CivTierAtLeast",
    "Tag": "",
    "Key": "None",
    "Value": 2.0
   },
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Space.HermitMonolith",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivLacksTrait",
    "Tag": "Brick.Form.DysonBrain",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivTimeVectorAtLeast",
    "Tag": "",
    "Key": "Isolated",
    "Value": 2000.0
   }
  ],
  "BaseWeight": 1.0,
  "Choices": [
   {
    "Label": "감싼 구를 닫아 셈에 돌린다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 200.0,
     "Matter": 400.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "AddTrait",
      "Tag": "Brick.Form.DysonBrain",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Openness",
      "Value": -0.05,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": -0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [],
    "SuccessText": "{Civ}|hpp(이,가) 제 별을 감싼 구를 닫았다. 그 별의 열은 모두 안쪽의 셈으로 흘러들었다.",
    "FailureText": "",
    "NextEncounter": "None"
   },
   {
    "Label": "닫아 둔 살림을 연다",
    "CheckStat": "Stability",
    "Difficulty": 8,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 150.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "RemoveTrait",
      "Tag": "Brick.Space.HermitMonolith",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "ChangeStability",
      "Tag": "",
      "Key": "None",
      "Value": -0.05,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": 0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [
     {
      "Type": "ChangeStability",
      "Tag": "",
      "Key": "None",
      "Value": -0.1,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": 0.05,
      "DurationTicks": 0
     }
    ],
    "SuccessText": "{Civ}|hpp(이,가) 닫아 둔 살림을 열었다. 안에서만 정하던 일들이 바깥을 보며 정해지기 시작했다.",
    "FailureText": "{Civ}의 빗장은 풀리지 않았다. 열자는 말과 닫자는 말이 맞서며 집집이 흔들렸다.",
    "NextEncounter": "None"
   },
   {
    "Label": "지켜본다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [],
    "OnFailure": [],
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 닫힌 항구들은 그대로 닫혀 있었다.",
    "FailureText": "",
    "NextEncounter": "None"
   }
  ],
  "Illustration": "",
  "Trigger": "Lottery",
  "Subject": "Civilization",
  "VoiceBodies": [
   "{Civ}|hpp(은,는) 오래 닫혀 있었고 그동안 제 별을 감싼 구를 지었다. 의회는 그 구를 닫아 별의 열을 모두 셈에 돌리자고 청했다.",
   "{Civ}|hpp(은,는) 오래 닫혀 있었다. 회중의 장로들은 닫힌 집에는 바깥의 소식이 늦게 닿는다며 빗장을 풀고 이웃을 맞자고 청했다."
  ]
 },
 {
  "Name": "petition_cross_law_stays",
  "Title": "비워 둔 왕좌",
  "Body": "",
  "Conditions": [
   {
    "Type": "PlayerRoleIs",
    "Tag": "",
    "Key": "Ruler",
    "Value": 0.0
   },
   {
    "Type": "CivTierAtLeast",
    "Tag": "",
    "Key": "None",
    "Value": 3.0
   },
   {
    "Type": "CivLacksTrait",
    "Tag": "Brick.Form.UnseenLaw",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivTimeVectorAtLeast",
    "Tag": "",
    "Key": "Endured",
    "Value": 2000.0
   }
  ],
  "BaseWeight": 1.0,
  "Choices": [
   {
    "Label": "칙령을 남기고 왕좌를 내려놓는다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "AddTrait",
      "Tag": "Brick.Form.UnseenLaw",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "DeposeRuler",
      "Tag": "Reign.Abdicated",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": -0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [],
    "SuccessText": "{Civ}|hpp(은,는) 정해 둔 칙령을 손대지 않은 채 두었다. 왕좌는 비어 있었다.",
    "FailureText": "",
    "NextEncounter": "None"
   },
   {
    "Label": "칙령 머리에 다스리는 이의 이름을 새긴다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 60.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "ChangeStability",
      "Tag": "",
      "Key": "None",
      "Value": 0.05,
      "DurationTicks": 0
     },
     {
      "Type": "ChangeReverence",
      "Tag": "",
      "Key": "None",
      "Value": 0.1,
      "DurationTicks": 0
     },
     {
      "Type": "PlayerStatDelta",
      "Tag": "",
      "Key": "Exaltation",
      "Value": 1.0,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": 0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [],
    "SuccessText": "{Civ}의 칙령 머리마다 다스리는 이의 이름이 새겨졌다. 칙령을 읽는 이들은 그 이름도 함께 읽었다.",
    "FailureText": "",
    "NextEncounter": "None"
   },
   {
    "Label": "지켜본다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [],
    "OnFailure": [],
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 칙령은 다스리는 이의 손에 그대로 매여 있었다.",
    "FailureText": "",
    "NextEncounter": "None"
   }
  ],
  "Illustration": "",
  "Trigger": "Lottery",
  "Subject": "Civilization",
  "VoiceBodies": [
   "{Civ}의 칙령은 견딘 날들 내내 다스리는 이의 손에 매여 있었다. 의회는 왕좌를 비우고도 칙령이 서는지 보아야 한다며 내려오기를 청했다.",
   "{Civ}의 칙령은 견딘 날들 내내 다스리는 이의 손에 매여 있었다. 회중의 장로들은 그 칙령을 지켜 온 이의 이름을 칙령 머리에 새겨 뒷사람이 알게 하자고 청했다."
  ]
 },
 {
  "Name": "petition_cross_seam",
  "Title": "이음매",
  "Body": "",
  "Conditions": [
   {
    "Type": "PlayerRoleIs",
    "Tag": "",
    "Key": "Ruler",
    "Value": 0.0
   },
   {
    "Type": "CivTierAtLeast",
    "Tag": "",
    "Key": "None",
    "Value": 3.0
   },
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Instinct.SwarmTech",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivLacksTrait",
    "Tag": "Brick.Instinct.HiveMerge",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivLacksTrait",
    "Tag": "Brick.Mutation.KesslerTapestry",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivTimeVectorAtLeast",
    "Tag": "",
    "Key": "Linked",
    "Value": 2000.0
   }
  ],
  "BaseWeight": 1.0,
  "Choices": [
   {
    "Label": "이음매를 지운다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "AddTrait",
      "Tag": "Brick.Instinct.HiveMerge",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "RemoveTrait",
      "Tag": "Brick.Instinct.SwarmTech",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "ChangeStability",
      "Tag": "",
      "Key": "None",
      "Value": 0.08,
      "DurationTicks": 0
     },
     {
      "Type": "PlayerStatDelta",
      "Tag": "",
      "Key": "Exaltation",
      "Value": 2.0,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": -0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [],
    "SuccessText": "{Civ}의 회의장에서 다른 말이 그쳤다. 모두가 같은 것을 말했고, 그것은 당신이 한 말이었다.",
    "FailureText": "",
    "NextEncounter": "None"
   },
   {
    "Label": "맞닿은 자리를 손본다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 80.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Openness",
      "Value": 0.05,
      "DurationTicks": 0
     },
     {
      "Type": "ChangeStability",
      "Tag": "",
      "Key": "None",
      "Value": -0.02,
      "DurationTicks": 0
     },
     {
      "Type": "ShiftEthics",
      "Tag": "",
      "Key": "Spiritualism",
      "Value": 0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [],
    "SuccessText": "{Civ}의 이웃과 맞닿은 자리마다 고쳐 쓸 말이 정해졌다. 여러 목소리를 맞추는 동안 회의는 길어졌다.",
    "FailureText": "",
    "NextEncounter": "None"
   },
   {
    "Label": "지켜본다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [],
    "OnFailure": [],
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 회의장에서는 여러 목소리가 하던 대로 오갔다.",
    "FailureText": "",
    "NextEncounter": "None"
   }
  ],
  "Illustration": "",
  "Trigger": "Lottery",
  "Subject": "Civilization",
  "VoiceBodies": [
   "{Civ}|hpp(은,는) 오래 이웃과 맺은 것을 지키며 여러 목소리로 일을 정해 왔다. 의회는 이음매를 지우고 모두가 같은 답을 내게 하자고 청했다.",
   "{Civ}|hpp(은,는) 오래 이웃과 맺은 것을 지키며 여러 목소리로 일을 정해 왔다. 회중의 장로들은 이음매가 서로 다른 이들이 맞닿은 자리라며 지우지 말고 손보자고 청했다."
  ]
 },
 {
  "Name": "enc_cross_drift_fleet",
  "Title": "떠날 채비",
  "Body": "{System}의 사람들은 집을 통째로 싣는 배를 지으며 붙박인 터를 버릴지를 두고 갈려 있다. 떠도는 살림이 오래되면 제 별까지 데리고 간다고 기록은 적는다.",
  "Conditions": [
   {
    "Type": "PlayerRoleIs",
    "Tag": "",
    "Key": "Wanderer",
    "Value": 0.0
   },
   {
    "Type": "CivTimeVectorAtLeast",
    "Tag": "",
    "Key": "Drifted",
    "Value": 900.0
   },
   {
    "Type": "CivSettledSystemsAtLeast",
    "Tag": "",
    "Key": "None",
    "Value": 2.0
   },
   {
    "Type": "CivLacksTrait",
    "Tag": "Brick.Space.Nomad",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivLacksTrait",
    "Tag": "Brick.Space.HermitMonolith",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivLacksTrait",
    "Tag": "Brick.Space.PlanckMicro",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivLacksTrait",
    "Tag": "Brick.Form.Society",
    "Key": "None",
    "Value": 0.0
   }
  ],
  "BaseWeight": 1.0,
  "Choices": [
   {
    "Label": "떠도는 살림의 법을 보인다",
    "CheckStat": "Body",
    "Difficulty": 5,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "AddTrait",
      "Tag": "Brick.Space.Nomad",
      "Key": "None",
      "Value": 0.0,
      "DurationTicks": 0
     },
     {
      "Type": "ChangeReverence",
      "Tag": "",
      "Key": "None",
      "Value": 0.2,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [
     {
      "Type": "ChangeReverence",
      "Tag": "",
      "Key": "None",
      "Value": -0.1,
      "DurationTicks": 0
     }
    ],
    "SuccessText": "당신은 배 위에서 먹고 자고 고치는 법을 보였다. {Civ}의 사람들은 터를 버리는 쪽으로 기울었다.",
    "FailureText": "당신이 보인 배 위의 살림은 {Civ}의 사람들에게 닿지 않았다. 조선소의 사람들은 당신을 낯설게 보았다.",
    "NextEncounter": "None"
   },
   {
    "Label": "조선소에서 일손을 보탠다",
    "CheckStat": "Body",
    "Difficulty": 4,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [
     {
      "Type": "ChangeReverence",
      "Tag": "",
      "Key": "None",
      "Value": 0.05,
      "DurationTicks": 0
     }
    ],
    "OnFailure": [],
    "SuccessText": "당신은 조선소에서 배를 짓는 일을 거들었다. {Civ}의 일꾼들이 당신의 손을 기억했다.",
    "FailureText": "당신이 거든 일은 조선소에 보탬이 되지 못했다. 배는 {Civ}의 일꾼들 손으로 지어졌다.",
    "NextEncounter": "None"
   },
   {
    "Label": "물러난다",
    "CheckStat": "NoCheck",
    "Difficulty": 0,
    "Cost": {
     "Energy": 0.0,
     "Matter": 0.0,
     "Insight": 0.0,
     "Influence": 0.0,
     "Causality": 0.0
    },
    "OnSuccess": [],
    "OnFailure": [],
    "SuccessText": "당신은 조선소 곁에 서 있었다. 떠날지 남을지는 {Civ}의 사람들이 정할 일로 남았다.",
    "FailureText": "",
    "NextEncounter": "None"
   }
  ],
  "Illustration": "",
  "Trigger": "Standing",
  "Subject": "VesselSystem",
  "VoiceBodies": []
 }
]''')
AA_EVENTS = json.loads(r'''[
 {
  "Name": "event_brick_shut_star",
  "Category": "Event.Civ.Economy",
  "LogCategory": "Log.Civ",
  "Trigger": "Lottery",
  "Subject": "Civilization",
  "Conditions": [
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Space.HermitMonolith",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Form.DysonBrain",
    "Key": "None",
    "Value": 0.0
   }
  ],
  "BaseWeight": 1.0,
  "CooldownTicks": 600,
  "Effects": [
   {
    "Type": "AddResource",
    "Tag": "",
    "Key": "Insight",
    "Value": 20.0,
    "DurationTicks": 0
   }
  ],
  "LogTemplates": [
   "{Civ}의 별을 감싼 구 안에서 셈이 끊이지 않고 돈다고 한다. 그들의 배는 새 터를 찾아 나서지 않는다.",
   "{Civ}의 닫힌 구에서 새 셈법이 흘러나왔다. 그 셈을 낸 이들은 구 바깥을 내다보지 않았다.",
   "{Civ}의 학자들이 별의 열로 돌린 셈을 장부에 옮겼다. 그 장부는 바깥으로 나가지 않았다."
  ],
  "Severity": 0,
  "MinTierToReveal": 0,
  "SignRow": "None"
 },
 {
  "Name": "event_brick_heat_at_door",
  "Category": "Event.Civ.Society",
  "LogCategory": "Log.Civ",
  "Trigger": "Lottery",
  "Subject": "Civilization",
  "Conditions": [
   {
    "Type": "BorderingCivHasTrait",
    "Tag": "Brick.Diet.CrisisParasite",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivTimeVectorAtLeast",
    "Tag": "",
    "Key": "Endured",
    "Value": 1200.0
   }
  ],
  "BaseWeight": 1.0,
  "CooldownTicks": 600,
  "Effects": [
   {
    "Type": "ChangeStability",
    "Tag": "",
    "Key": "None",
    "Value": -0.03,
    "DurationTicks": 0
   }
  ],
  "LogTemplates": [
   "{Civ}의 국경 너머에서 재앙의 열을 핥는 이들이 서성인다고 한다. 오래 견딘 집들은 그들을 내쫓지 않는다.",
   "{Civ}의 국경 장터에 재앙 곁에서 사는 이웃의 배들이 드나들었다. 오래 견딘 이들은 그 배를 보며 말을 낮추었다.",
   "{Civ}의 변경에서 이웃이 재앙의 자리를 찾아다닌다는 말이 돌았다. 그 말이 도는 동안 거리의 목소리가 날카로워졌다."
  ],
  "Severity": 0,
  "MinTierToReveal": 0,
  "SignRow": "None"
 },
 {
  "Name": "event_brick_drift_plenty",
  "Category": "Event.Civ.Economy",
  "LogCategory": "Log.Civ",
  "Trigger": "Lottery",
  "Subject": "Civilization",
  "Conditions": [
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Instinct.SwarmTech",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Space.Nomad",
    "Key": "None",
    "Value": 0.0
   }
  ],
  "BaseWeight": 1.0,
  "CooldownTicks": 600,
  "Effects": [
   {
    "Type": "AddResource",
    "Tag": "",
    "Key": "Matter",
    "Value": 30.0,
    "DurationTicks": 0
   },
   {
    "Type": "ChangePopulation",
    "Tag": "",
    "Key": "None",
    "Value": 0.02,
    "DurationTicks": 0
   }
  ],
  "LogTemplates": [
   "{Civ}의 떠도는 일손이 닿는 별마다 광맥을 열었다. 배 위의 창고가 차고 식구가 늘었다.",
   "{Civ}의 작은 일꾼들이 새로 닿은 자리에서 쓸 것을 캐 올렸다. 배마다 식구가 늘었다.",
   "{Civ}의 배들이 지나는 항로마다 캐 올린 것이 쌓였다. 배 위의 살림이 넉넉해지고 집이 불었다."
  ],
  "Severity": 1,
  "MinTierToReveal": 0,
  "SignRow": "None"
 },
 {
  "Name": "event_brick_emptied_trail",
  "Category": "Event.Civ.Economy",
  "LogCategory": "Log.Civ",
  "Trigger": "Lottery",
  "Subject": "Civilization",
  "Conditions": [
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Instinct.SwarmTech",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Space.Nomad",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivTimeVectorAtLeast",
    "Tag": "",
    "Key": "Drifted",
    "Value": 1800.0
   }
  ],
  "BaseWeight": 1.0,
  "CooldownTicks": 600,
  "Effects": [
   {
    "Type": "ChangeStability",
    "Tag": "",
    "Key": "None",
    "Value": -0.03,
    "DurationTicks": 0
   },
   {
    "Type": "ChangePopulation",
    "Tag": "",
    "Key": "None",
    "Value": -0.03,
    "DurationTicks": 0
   },
   {
    "Type": "AddResource",
    "Tag": "",
    "Key": "Matter",
    "Value": -40.0,
    "DurationTicks": 0
   }
  ],
  "LogTemplates": [
   "{Civ}의 배들이 지나온 자리에서는 더는 아무것도 올라오지 않았다. 갱도 어귀의 집들이 비어 갔다.",
   "{Civ}의 일꾼들이 캐던 자리마다 바닥이 드러났다. 창고가 줄고 배 위의 식구도 줄었다.",
   "{Civ}의 항로 뒤편에 파헤친 별들이 비어 남았다. 다음 터를 찾는 동안 배 위의 살림이 줄고 다툼이 잦아졌다."
  ],
  "Severity": 1,
  "MinTierToReveal": 0,
  "SignRow": "None"
 },
 {
  "Name": "event_brick_ember_wake",
  "Category": "Event.Civ.Society",
  "LogCategory": "Log.Civ",
  "Trigger": "Standing",
  "Subject": "Civilization",
  "Conditions": [
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Diet.StarEater",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "SystemHasHazard",
    "Tag": "Hazard.PreSupernova",
    "Key": "None",
    "Value": 0.0
   }
  ],
  "BaseWeight": 0.0,
  "CooldownTicks": 300,
  "Effects": [
   {
    "Type": "ChangeStability",
    "Tag": "",
    "Key": "None",
    "Value": -0.02,
    "DurationTicks": 0
   }
  ],
  "LogTemplates": [
   "{Civ}의 해에 검은 벌집 무늬가 번졌다. 국경 너머에는 그 불이 잦아들기를 기다리는 이들이 있었다.",
   "{Civ}의 화로가 끌어 쓰던 별이 고르지 않게 떨렸다. 화로 곁의 사람들이 불을 줄이자는 말을 주고받았다.",
   "{Civ}의 도읍 하늘에서 별빛이 얼룩졌다. 그 별의 불을 끌어 쓰던 집들이 하늘을 자주 살폈다."
  ],
  "Severity": 0,
  "MinTierToReveal": 0,
  "SignRow": "None"
 },
 {
  "Name": "event_brick_own_hour",
  "Category": "Event.Civ.Society",
  "LogCategory": "Log.Civ",
  "Trigger": "Lottery",
  "Subject": "Civilization",
  "Conditions": [
   {
    "Type": "BorderingCivHasTrait",
    "Tag": "Brick.Instinct.HiveMerge",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivTimeVectorAtLeast",
    "Tag": "",
    "Key": "Isolated",
    "Value": 1500.0
   }
  ],
  "BaseWeight": 1.0,
  "CooldownTicks": 600,
  "Effects": [
   {
    "Type": "ChangeStability",
    "Tag": "",
    "Key": "None",
    "Value": -0.03,
    "DurationTicks": 0
   },
   {
    "Type": "AddResource",
    "Tag": "",
    "Key": "Insight",
    "Value": 20.0,
    "DurationTicks": 0
   }
  ],
  "LogTemplates": [
   "{Civ}의 국경 너머에서는 모두가 같은 시각에 장을 연다고들 했다. {Civ}|hpp(은,는) 제 시각에 문을 열고 닫았다.",
   "{Civ}의 변경 장터에 이웃의 한결같은 목소리가 들려왔다. {Civ}의 학자들은 술렁이는 거리에서 제 셈을 따로 적었다.",
   "{Civ}의 국경 너머 이웃이 같은 답으로만 말한다는 소문이 돌았다. {Civ}|hpp(은,는) 술렁이면서도 제 셈법을 지켰다."
  ],
  "Severity": 0,
  "MinTierToReveal": 0,
  "SignRow": "None"
 },
 {
  "Name": "event_brick_fold_gleaning",
  "Category": "Event.Civ.Economy",
  "LogCategory": "Log.Civ",
  "Trigger": "Lottery",
  "Subject": "Civilization",
  "Conditions": [
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Space.PlanckMicro",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "BorderingCivHasTrait",
    "Tag": "Brick.Space.Nomad",
    "Key": "None",
    "Value": 0.0
   }
  ],
  "BaseWeight": 1.0,
  "CooldownTicks": 600,
  "Effects": [
   {
    "Type": "AddResource",
    "Tag": "",
    "Key": "Insight",
    "Value": 25.0,
    "DurationTicks": 0
   }
  ],
  "LogTemplates": [
   "{Civ}의 별 곁에는 떠도는 살림이 자리를 펴고 있다. {Civ}|hpp(은,는) 그들이 떨군 것을 주워 맞춘다.",
   "{Civ}의 국경 너머 항로에 떠도는 배들이 오갔다. {Civ}의 학자들이 그 배들이 남긴 것에서 새 셈을 찾았다.",
   "{Civ}|hpp(은,는) 이웃의 떠도는 배들이 버린 부스러기를 모았다. 작은 눈금 속에서 그것들이 새 앎으로 맞추어졌다."
  ],
  "Severity": 0,
  "MinTierToReveal": 0,
  "SignRow": "None"
 },
 {
  "Name": "event_brick_inward_appetite",
  "Category": "Event.Civ.Economy",
  "LogCategory": "Log.Civ",
  "Trigger": "Lottery",
  "Subject": "Civilization",
  "Conditions": [
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Mutation.AbyssalMaw",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivTimeVectorAtLeast",
    "Tag": "",
    "Key": "Isolated",
    "Value": 1500.0
   }
  ],
  "BaseWeight": 1.0,
  "CooldownTicks": 600,
  "Effects": [
   {
    "Type": "AddResource",
    "Tag": "",
    "Key": "Energy",
    "Value": 30.0,
    "DurationTicks": 0
   }
  ],
  "LogTemplates": [
   "{Civ}|hpp(은,는) 제 별의 불을 안으로 끌어 먹는다고 한다. 그 별을 바라보는 이웃들은 말을 아꼈다.",
   "{Civ}의 화로가 제 별 깊은 곳의 불을 끌어올렸다. 그들의 창고에 에너지가 찼다.",
   "{Civ}의 화로가 바깥의 것 없이 제 별의 불만으로 타올랐다. 화로지기들은 그 별을 오래 들여다보았다."
  ],
  "Severity": 0,
  "MinTierToReveal": 0,
  "SignRow": "None"
 },
 {
  "Name": "event_brick_works_run",
  "Category": "Event.Civ.Economy",
  "LogCategory": "Log.Civ",
  "Trigger": "Lottery",
  "Subject": "Civilization",
  "Conditions": [
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Instinct.SwarmTech",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivHasTrait",
    "Tag": "Brick.Mutation.KesslerTapestry",
    "Key": "None",
    "Value": 0.0
   },
   {
    "Type": "CivTimeVectorAtLeast",
    "Tag": "",
    "Key": "Built",
    "Value": 1500.0
   }
  ],
  "BaseWeight": 1.0,
  "CooldownTicks": 600,
  "Effects": [
   {
    "Type": "AddResource",
    "Tag": "",
    "Key": "Energy",
    "Value": 25.0,
    "DurationTicks": 0
   },
   {
    "Type": "AddResource",
    "Tag": "",
    "Key": "Matter",
    "Value": 25.0,
    "DurationTicks": 0
   },
   {
    "Type": "ChangePopulation",
    "Tag": "",
    "Key": "None",
    "Value": -0.02,
    "DurationTicks": 0
   }
  ],
  "LogTemplates": [
   "{Civ}의 기관들은 돌보는 손이 줄어든 뒤에도 돈다고 한다. 띠를 이룬 일꾼들이 서로를 고쳐 가며 일을 잇는다.",
   "{Civ}의 별 둘레의 띠가 사람 손 없이 에너지와 물질을 실어 왔다. 공방을 지키던 집들 가운데 빈 집이 늘었다.",
   "{Civ}의 일꾼 띠가 스스로를 고치며 창고를 채웠다. 그 곁에서 손을 놀리던 사람은 줄어 갔다."
  ],
  "Severity": 0,
  "MinTierToReveal": 0,
  "SignRow": "None"
 }
]''')
AUDITED = json.loads(r'''{
 "petition_cross_built": {
  "Title": "작은 손들",
  "Body": "",
  "VoiceBodies": [
   "{Civ}의 작업장은 오래 불을 끄지 않았다. 의회는 큰 공사를 잘게 나누어 집집마다 연장을 들이자고 청했다.",
   "{Civ}의 작업장은 오래 불을 끄지 않았다. 회중의 장로들은 새 연장을 가르칠 수 있는 장인들의 손에만 맡기자고 청했다."
  ],
  "Choices": [
   {
    "Label": "집집마다 연장을 들인다",
    "SuccessText": "{Civ}의 집마다 작은 공방이 섰다. 그 일이 끝났을 때 큰 공사는 집집의 작은 손에 나뉘어 있었다.",
    "FailureText": "{Civ}의 집집에 연장이 들었지만 손에 익지 않았다. 큰 공사는 장인들의 몫으로 남았고 거리는 술렁였다."
   },
   {
    "Label": "장인 조합에만 새 연장을 맡긴다",
    "SuccessText": "{Civ}의 장인 조합이 새 연장을 맡았다. 그들의 공방에서 나온 것이 창고에 쌓였다.",
    "FailureText": ""
   },
   {
    "Label": "지켜본다",
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 작업장의 일은 하던 대로 이어졌다.",
    "FailureText": ""
   }
  ]
 },
 "petition_cross_consumed": {
  "Title": "땅이 내는 것",
  "Body": "",
  "VoiceBodies": [
   "{Civ}의 광맥은 오래 싸움에 쓸 것을 대느라 깊어졌다. 의회는 땅을 더 파지 말고 제 별의 불을 바로 화로에 끌어들이자고 청했다.",
   "{Civ}의 광맥은 오래 싸움에 쓸 것을 대느라 깊어졌다. 회중의 장로들은 땅을 그만 파고 별과 별 사이에 떠도는 엷은 것을 걸러 살자고 청했다."
  ],
  "Choices": [
   {
    "Label": "별 사이의 엷은 것을 걸러 쓴다",
    "SuccessText": "{Civ}의 배들이 암흑 구간에 넓은 체를 폈다. 그 일이 끝났을 때 그들의 화로는 별 사이에서 걸러 낸 것으로 타고 있었다.",
    "FailureText": "{Civ}의 배들이 암흑 구간에 편 체에는 걸리는 것이 없었다. 광맥 곁의 집들은 들인 품을 두고 술렁였다."
   },
   {
    "Label": "제 별의 불을 화로에 끌어들인다",
    "SuccessText": "{Civ}의 화로가 제 별의 불을 끌어 쓰기 시작했다. 국경 너머의 이웃들은 그 별빛을 오래 바라보았다.",
    "FailureText": ""
   },
   {
    "Label": "지켜본다",
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 광맥의 일은 하던 대로 이어졌다.",
    "FailureText": ""
   }
  ]
 },
 "petition_cross_drifted": {
  "Title": "뭍과 배",
  "Body": "",
  "VoiceBodies": [
   "{Civ}의 배들은 오래 새 터를 닦으러 나가 있었다. 의회는 떠나려는 집은 보내고 남으려는 집은 남게 두자고 청했다.",
   "{Civ}의 배들은 오래 새 터를 닦으러 나가 있었다. 회중의 장로들은 닿는 곳마다 집이 함께 가야 한다며 살림을 통째로 배에 싣자고 청했다."
  ],
  "Choices": [
   {
    "Label": "살림을 통째로 배에 싣는다",
    "SuccessText": "{Civ}에서 배에 사는 집이 뭍의 집보다 많아졌다. 그 일이 끝났을 때 그들은 더는 닻을 내린 곳을 집이라 부르지 않았다.",
    "FailureText": "{Civ}의 배에 살림을 실었지만 뭍을 떠나려는 집은 적었다. 실은 짐을 도로 내리는 동안 항구가 어수선했다."
   },
   {
    "Label": "떠나려는 집에만 배를 내준다",
    "SuccessText": "{Civ}의 배가 떠나려는 집들만 태우고 항로에 올랐다. 뭍에 남은 집들은 하던 살림을 이어 갔다.",
    "FailureText": ""
   },
   {
    "Label": "지켜본다",
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 새 터를 닦는 배들은 하던 대로 오갔다.",
    "FailureText": ""
   }
  ]
 },
 "petition_cross_isolated": {
  "Title": "항구의 빗장",
  "Body": "",
  "VoiceBodies": [
   "{Civ}의 백성은 오래 저희끼리 지냈다. 의회는 큰 항구만 남기고 나머지를 닫아 안을 짓는 데 힘을 모으자고 청했다.",
   "{Civ}의 백성은 오래 저희끼리 지냈다. 회중의 장로들은 바깥의 것이 들면 지켜 온 것이 흐려진다며 항구를 모두 닫자고 청했다."
  ],
  "Choices": [
   {
    "Label": "항구를 닫고 새 터를 찾지 않는다",
    "SuccessText": "{Civ}의 항구들이 닫히고 새 터로 떠날 배들이 항구에 매였다. 그 일이 끝났을 때 그들은 제 별 안쪽만 짓고 있었다.",
    "FailureText": ""
   },
   {
    "Label": "큰 항구만 남기고 닫는다",
    "SuccessText": "{Civ}의 작은 항구들이 닫히고 큰 항구만 열려 있었다. 바깥으로 쓰던 손이 안을 짓는 데로 돌아갔고, 학자들의 셈이 깊어졌다.",
    "FailureText": ""
   },
   {
    "Label": "지켜본다",
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 항구들은 하던 대로 열려 있었다.",
    "FailureText": ""
   }
  ]
 },
 "petition_cross_dust_band": {
  "Title": "별 둘레의 티끌 띠",
  "Body": "",
  "VoiceBodies": [
   "{Civ}의 작은 일꾼들은 스스로를 고쳐 가며 일한다. 의회는 그 일꾼들을 별마다 띠로 둘러 손이 닿지 않던 일까지 맡기자고 청했다.",
   "{Civ}의 작은 일꾼들은 스스로를 고쳐 가며 일한다. 회중의 장로들은 누구의 손에도 매이지 않는 것을 늘리기 전에 사람을 먼저 돌보자고 청했다."
  ],
  "Choices": [
   {
    "Label": "일꾼들을 별마다 띠로 두른다",
    "SuccessText": "{Civ}의 별마다 티끌 같은 일꾼들이 띠를 이루었다. 띠는 스스로를 고치며 돌았고, 그 일을 다 아는 사람은 없었다.",
    "FailureText": "{Civ}의 일꾼들은 띠를 이루지 못하고 흩어졌다. 들인 것을 두고 의회의 말이 엇갈렸다."
   },
   {
    "Label": "사람을 먼저 돌본다",
    "SuccessText": "{Civ}의 물자가 집과 배움터를 짓는 데 들었다. 아이를 기르는 집들의 근심이 덜어졌다.",
    "FailureText": ""
   },
   {
    "Label": "지켜본다",
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 작은 일꾼들은 하던 대로 일했다.",
    "FailureText": ""
   }
  ]
 },
 "petition_cross_star_line": {
  "Title": "별에 거는 줄",
  "Body": "",
  "VoiceBodies": [
   "{Civ}의 살림은 오래 배 위에서 꾸려졌다. 의회는 도읍의 별에 끄는 줄을 걸어 데리고 떠날 채비를 하자고 청했다.",
   "{Civ}의 살림은 오래 배 위에서 꾸려졌다. 회중의 장로들은 떠날 채비로 지은 집에서는 아무도 뿌리를 내리지 못한다며 집부터 고치자고 청했다."
  ],
  "Choices": [
   {
    "Label": "도읍의 별에 끄는 줄을 건다",
    "SuccessText": "{Civ}|hpp(이,가) 도읍의 별에 끄는 줄을 걸었다. 별은 제자리에 있었고, 그들은 떠날 채비를 한 채 살게 되었다.",
    "FailureText": "{Civ}의 배들이 도읍의 별에 줄을 걸다가 놓쳤다. 끊어진 줄을 두고 배 위의 집들이 술렁였다."
   },
   {
    "Label": "집부터 고쳐 짓는다",
    "SuccessText": "{Civ}의 배 위 집들이 새로 고쳐졌다. 그 안에서 사는 날이 한결 편해졌다.",
    "FailureText": ""
   },
   {
    "Label": "지켜본다",
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 배 위의 살림은 하던 대로 꾸려졌다.",
    "FailureText": ""
   }
  ]
 },
 "petition_cross_between_stars": {
  "Title": "별 사이의 엷은 것",
  "Body": "",
  "VoiceBodies": [
   "{Civ}의 살림은 별 사이에서 걸러 낸 것으로 오래 꾸려졌다. 의회는 지붕부터 고쳐 땅 위에 디딜 곳을 지키자고 청했다.",
   "{Civ}의 살림은 별 사이에서 걸러 낸 것으로 오래 꾸려졌다. 회중의 장로들은 땅 위의 살림을 별들 사이의 엷은 것에 풀자고 청했다."
  ],
  "Choices": [
   {
    "Label": "살림을 별들 사이에 푼다",
    "SuccessText": "{Civ}|hpp(이,가) 살림을 별들 사이의 엷은 것에 풀었다. 그 일이 끝났을 때 그들의 살림은 별과 별 사이에 번져 있었다.",
    "FailureText": "{Civ}|hpp(이,가) 별들 사이에 푼 살림은 번지지 못하고 흩어졌다. 땅 위의 집들이 그 일을 두고 술렁였다."
   },
   {
    "Label": "땅 위의 집을 고쳐 짓는다",
    "SuccessText": "{Civ}의 땅 위 집들이 지붕부터 고쳐졌다. 디딜 곳이 단단해지자 그 집들이 마음을 놓았다.",
    "FailureText": ""
   },
   {
    "Label": "지켜본다",
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 별 사이에서 걸러 내는 일은 하던 대로 이어졌다.",
    "FailureText": ""
   }
  ]
 },
 "petition_cross_into_dust": {
  "Title": "가장 작은 눈금",
  "Body": "",
  "VoiceBodies": [
   "{Civ}|hpp(은,는) 떠돈 날이 길었다. 의회는 가장 작은 눈금 속에 다투는 이 없는 넓이가 있다며 그리로 접혀 들자고 청했다.",
   "{Civ}|hpp(은,는) 떠돈 날이 길었다. 회중의 장로들은 밖을 내다보지 않는 살림은 밖의 일을 잊는다며 별 곁에 디딜 곳을 마련하자고 청했다."
  ],
  "Choices": [
   {
    "Label": "가장 작은 눈금 속으로 접혀 든다",
    "SuccessText": "{Civ}|hpp(이,가) 살림을 가장 작은 눈금 속으로 접어 넣었다. 그 일이 끝났을 때 그들은 그 눈금 속에서 살고 있었고, 떠돌던 배들은 항구에 매여 있었다.",
    "FailureText": "{Civ}|hpp(이,가) 살림을 접어 넣으려 했지만 눈금은 열리지 않았다. 떠돌던 배 위의 집들이 술렁였다."
   },
   {
    "Label": "별 곁에 디딜 곳을 마련한다",
    "SuccessText": "{Civ}의 배들이 별 곁에 디딜 곳을 닦았다. 떠돌던 집들이 그곳에서 숨을 골랐다.",
    "FailureText": ""
   },
   {
    "Label": "지켜본다",
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 배들은 하던 대로 항로를 떠돌았다.",
    "FailureText": ""
   }
  ]
 },
 "petition_cross_wrapped_star": {
  "Title": "닫힌 구의 셈",
  "Body": "",
  "VoiceBodies": [
   "{Civ}|hpp(은,는) 오래 닫혀 있었고 그동안 제 별을 감싼 구를 지었다. 의회는 그 구를 닫아 별의 열을 모두 셈에 돌리자고 청했다.",
   "{Civ}|hpp(은,는) 오래 닫혀 있었다. 회중의 장로들은 닫힌 집에는 바깥의 소식이 늦게 닿는다며 빗장을 풀고 이웃을 맞자고 청했다."
  ],
  "Choices": [
   {
    "Label": "감싼 구를 닫아 셈에 돌린다",
    "SuccessText": "{Civ}|hpp(이,가) 제 별을 감싼 구를 닫았다. 그 별의 열은 모두 안쪽의 셈으로 흘러들었다.",
    "FailureText": ""
   },
   {
    "Label": "닫아 둔 살림을 연다",
    "SuccessText": "{Civ}|hpp(이,가) 닫아 둔 살림을 열었다. 안에서만 정하던 일들이 바깥을 보며 정해지기 시작했다.",
    "FailureText": "{Civ}의 빗장은 풀리지 않았다. 열자는 말과 닫자는 말이 맞서며 집집이 흔들렸다."
   },
   {
    "Label": "지켜본다",
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 닫힌 항구들은 그대로 닫혀 있었다.",
    "FailureText": ""
   }
  ]
 },
 "petition_cross_law_stays": {
  "Title": "손대지 않은 칙령",
  "Body": "",
  "VoiceBodies": [
   "{Civ}의 칙령은 눌린 날들 내내 다스리는 이의 손에 매여 있었다. 의회는 다스리는 이가 정해 둔 칙령에 손대지 않은 채 내려오기를 청했다.",
   "{Civ}의 칙령은 눌린 날들 내내 다스리는 이의 손에 매여 있었다. 회중의 장로들은 그 칙령을 지켜 온 이의 이름을 칙령 머리에 새겨 뒷사람이 알게 하자고 청했다."
  ],
  "Choices": [
   {
    "Label": "칙령을 남기고 왕좌를 내려놓는다",
    "SuccessText": "{Civ}|hpp(은,는) 정해 둔 칙령을 손대지 않은 채 두었다. 칙령을 정한 이는 자리에서 내려왔다.",
    "FailureText": ""
   },
   {
    "Label": "칙령 머리에 다스리는 이의 이름을 새긴다",
    "SuccessText": "{Civ}의 칙령 머리마다 다스리는 이의 이름이 새겨졌다. 칙령을 읽는 이들은 누가 정한 법인지 알고 마음을 놓았다.",
    "FailureText": ""
   },
   {
    "Label": "지켜본다",
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 칙령은 다스리는 이의 손에 그대로 매여 있었다.",
    "FailureText": ""
   }
  ]
 },
 "petition_cross_seam": {
  "Title": "이음매",
  "Body": "",
  "VoiceBodies": [
   "{Civ}|hpp(은,는) 오래 이웃과 맺은 것을 지키며 여러 목소리로 일을 정해 왔다. 의회는 이음매를 지우고 모두가 같은 답을 내게 하자고 청했다.",
   "{Civ}|hpp(은,는) 오래 이웃과 맺은 것을 지키며 여러 목소리로 일을 정해 왔다. 회중의 장로들은 이음매가 서로 다른 이들이 맞닿은 자리라며 지우지 말고 손보자고 청했다."
  ],
  "Choices": [
   {
    "Label": "이음매를 지운다",
    "SuccessText": "{Civ}의 회의장에서 다른 말이 그쳤다. 모두가 같은 것을 말했고, 그것은 당신이 한 말이었다.",
    "FailureText": ""
   },
   {
    "Label": "맞닿은 자리를 손본다",
    "SuccessText": "{Civ}의 이웃과 맞닿은 자리마다 고쳐 쓸 말이 정해졌다. 회의는 길어졌지만 거기서 오간 말이 학자들의 셈에 보태졌다.",
    "FailureText": ""
   },
   {
    "Label": "지켜본다",
    "SuccessText": "{Civ}|hpp(은,는) 지켜보기로 했다. 회의장에서는 여러 목소리가 하던 대로 오갔다.",
    "FailureText": ""
   }
  ]
 },
 "enc_cross_drift_fleet": {
  "Title": "조선소의 갈림",
  "Body": "{System}의 사람들은 집을 통째로 싣는 배를 지으며 살림을 배에 옮길지를 두고 갈려 있다. 조선소에는 반쯤 지은 배들이 늘어서 있다.",
  "VoiceBodies": [],
  "Choices": [
   {
    "Label": "배 위에서 사는 법을 보인다",
    "SuccessText": "당신은 배 위에서 먹고 자고 고치는 법을 보였다. {Civ}의 사람들은 살림을 배에 싣기 시작했다.",
    "FailureText": "당신이 보인 배 위의 살림은 {Civ}의 사람들에게 닿지 않았다. 조선소의 사람들은 당신을 낯설게 보았다."
   },
   {
    "Label": "조선소에서 일손을 보탠다",
    "SuccessText": "당신은 조선소에서 배를 짓는 일을 거들었다. 반쯤 지은 배 가운데 한 척이 마저 지어졌다.",
    "FailureText": "당신이 거든 일은 조선소에 보탬이 되지 못했다. 배는 {Civ}의 일꾼들 손으로 지어졌다."
   },
   {
    "Label": "물러난다",
    "SuccessText": "당신은 조선소 앞에서 걸음을 돌렸다. 떠날지 남을지는 {Civ}의 사람들이 정할 일로 남았다.",
    "FailureText": ""
   }
  ]
 },
 "event_brick_shut_star": {
  "LogTemplates": [
   "{Civ}의 별을 감싼 구 안에서 셈이 끊이지 않고 돈다고 한다. 그들의 배는 새 터를 찾아 나서지 않는다.",
   "{Civ}의 닫힌 구에서 새 셈법이 흘러나왔다. 그 셈을 낸 이들은 구 바깥을 내다보지 않았다.",
   "{Civ}의 학자들이 별의 열로 돌린 셈을 장부에 옮겼다. 그 장부는 바깥으로 나가지 않았다."
  ]
 },
 "event_brick_heat_at_door": {
  "LogTemplates": [
   "{Civ}의 국경 너머에서 재앙 곁에 사는 이들이 서성인다고 한다. 오래 눌려 온 집들은 그들을 내쫓지 않지만, 거리는 술렁인다.",
   "{Civ}의 국경 장터에 재앙 곁에서 사는 이웃의 배들이 드나들었다. 오래 눌려 온 이들은 그 배를 보며 말을 낮추었다.",
   "{Civ}의 변경에서 이웃이 재앙의 자리를 찾아다닌다는 말이 돌았다. 그 말이 도는 동안 거리의 목소리가 날카로워졌다."
  ]
 },
 "event_brick_drift_plenty": {
  "LogTemplates": [
   "{Civ}의 떠도는 일손이 닿는 별마다 광맥을 열었다. 배 위의 창고가 차고 식구가 늘었다.",
   "{Civ}의 작은 일꾼들이 새로 닿은 자리에서 쓸 것을 캐 올렸다. 배마다 식구가 늘었다.",
   "{Civ}의 배들이 지나는 항로마다 캐 올린 것이 쌓였다. 배 위의 살림이 넉넉해지고 집이 늘었다."
  ]
 },
 "event_brick_emptied_trail": {
  "LogTemplates": [
   "{Civ}의 배들이 지나온 자리에서는 더는 아무것도 올라오지 않았다. 갱도 어귀의 집들이 비어 갔다.",
   "{Civ}의 일꾼들이 캐던 자리마다 바닥이 드러났다. 창고가 줄고 배 위의 식구도 줄었다.",
   "{Civ}의 항로 뒤편에 파헤친 별들이 빈 채 남았다. 다음 터를 찾는 동안 배 위의 살림이 줄고 다툼이 잦아졌다."
  ]
 },
 "event_brick_ember_wake": {
  "LogTemplates": [
   "{Civ}의 해에 검은 벌집 무늬가 번졌다. 국경 너머에는 그 불이 잦아들기를 기다리는 이들이 있었다.",
   "{Civ}의 화로가 끌어 쓰던 별이 고르지 않게 떨렸다. 화로 곁의 사람들이 불을 줄이자는 말을 주고받았다.",
   "{Civ}의 도읍 하늘에서 별빛이 얼룩졌다. 그 별의 불을 끌어 쓰던 집들이 하늘을 자주 살폈다."
  ]
 },
 "event_brick_own_hour": {
  "LogTemplates": [
   "{Civ}의 국경 너머에서는 모두가 같은 시각에 장을 연다고들 했다. {Civ}|hpp(은,는) 제 시각에 문을 열고 닫았다.",
   "{Civ}의 변경 장터에 이웃의 한결같은 목소리가 들려왔다. {Civ}의 학자들은 술렁이는 거리에서 제 셈을 따로 적었다.",
   "{Civ}의 국경 너머 이웃이 같은 답으로만 말한다는 소문이 돌았다. {Civ}|hpp(은,는) 술렁이면서도 제 셈법을 지켰다."
  ]
 },
 "event_brick_fold_gleaning": {
  "LogTemplates": [
   "{Civ}의 별 곁에는 떠도는 살림이 자리를 펴고 있다. {Civ}|hpp(은,는) 그들이 떨군 것을 주워 맞춘다.",
   "{Civ}의 국경 너머 항로에는 떠도는 배들이 오간다. {Civ}의 학자들은 그 배들이 남긴 것에서 새 셈을 찾는다.",
   "{Civ}|hpp(은,는) 이웃의 떠도는 배들이 버린 부스러기를 모은다. 작은 눈금 속에서 그것들이 새 앎으로 맞추어진다."
  ]
 },
 "event_brick_inward_appetite": {
  "LogTemplates": [
   "{Civ}|hpp(은,는) 제 별의 불을 안으로 끌어 먹는다고 한다. 그 별을 바라보는 이웃들은 말을 아꼈다.",
   "{Civ}의 화로가 제 별 깊은 곳의 불을 끌어올렸다. 그들의 창고에 에너지가 찼다.",
   "{Civ}의 화로가 바깥의 것 없이 제 별의 불만으로 타올랐다. 화로지기들은 그 별을 오래 들여다보았다."
  ]
 },
 "event_brick_works_run": {
  "LogTemplates": [
   "{Civ}의 기관들은 돌보는 손이 줄어든 뒤에도 돈다고 한다. 띠를 이룬 일꾼들이 서로를 고쳐 가며 일을 잇는다.",
   "{Civ}의 별 둘레의 띠가 사람 손 없이 에너지와 물질을 실어 왔다. 공방을 지키던 집들 가운데 빈 집이 늘었다.",
   "{Civ}의 일꾼 띠가 스스로를 고치며 창고를 채웠다. 그 곁에서 손으로 일하던 사람은 줄어 갔다."
  ]
 }
}''')
RULED_EFFECTS = json.loads(r'''{
 "petition_cross_dust_band.Choices[1].OnSuccess": [
  {
   "Type": "ChangeStability",
   "Tag": "",
   "Key": "None",
   "Value": 0.05,
   "DurationTicks": 0
  },
  {
   "Type": "ShiftEthics",
   "Tag": "",
   "Key": "Spiritualism",
   "Value": 0.05,
   "DurationTicks": 0
  }
 ],
 "petition_cross_between_stars.Choices[1].OnSuccess": [
  {
   "Type": "ChangeStability",
   "Tag": "",
   "Key": "None",
   "Value": 0.04,
   "DurationTicks": 0
  },
  {
   "Type": "ShiftEthics",
   "Tag": "",
   "Key": "Spiritualism",
   "Value": -0.05,
   "DurationTicks": 0
  }
 ],
 "petition_cross_law_stays.Choices[1].OnSuccess": [
  {
   "Type": "ChangeStability",
   "Tag": "",
   "Key": "None",
   "Value": 0.05,
   "DurationTicks": 0
  }
 ],
 "petition_cross_seam.Choices[1].OnSuccess": [
  {
   "Type": "AddResource",
   "Tag": "",
   "Key": "Insight",
   "Value": 120.0,
   "DurationTicks": 0
  },
  {
   "Type": "ChangeStability",
   "Tag": "",
   "Key": "None",
   "Value": -0.02,
   "DurationTicks": 0
  },
  {
   "Type": "ShiftEthics",
   "Tag": "",
   "Key": "Spiritualism",
   "Value": 0.05,
   "DurationTicks": 0
  }
 ]
}''')

# The lead's rulings on the texts (lead_rulings.md section 2 and section 1's last paragraph): they win over the audit.
RULED_TEXTS = {
    ("enc_cross_drift_fleet", "Title"): "집을 싣는 배",
    # Choice 2 keeps ChangeReverence +0.05, so its success line goes back to aa09543's second sentence.
    ("enc_cross_drift_fleet", "Choices[1].SuccessText"): "당신은 조선소에서 배를 짓는 일을 거들었다. {Civ}의 일꾼들이 당신의 손을 기억했다.",
}
EXPECTED_TITLES = {"petition_cross_law_stays": "손대지 않은 칙령", "enc_cross_drift_fleet": "집을 싣는 배"}

# --- schema rules ---------------------------------------------------------------------------------------------------------
WHY_RULE4_FAILURE = "a brick is given or taken by a choice that is not rolled for, or on the success of a roll - never on its failure: a failed roll changes no people's shape (B7x header section 3.3 rule 4)"
WHY_RULE4_FREE = "a card that gives or takes a brick holds a choice with no cost, no effect and no next card: the measuring policies skip every choice that gives or takes a brick, so with one waiting place a card whose every choice did would block every card behind it - said the other way round: a card without such a choice gives and takes no brick (B7x header section 3.3 rule 4, section 5)"
WHY_RULE5_LOTTERY = "a crossroads petition - the petition_cross_ rows, marked by their time vector condition because a rule names a table's own prefix only - comes in the periodic slot: a Standing card is lost while the waiting place is full and comes again at every reign (B7x header section 3.1, 3.3 rule 5)"
# Notes 5.4 data-1 (F7): the closing rule keeps its then and takes this why.
WHY_RULE5_CLOSES = "a crossroads petition closes on its branch: the first by taking Brick.Form.Society, the second by the tag its first choice gives; its choice 2 carries no repeatable gain; a dismissed card (no cost, no effect) comes again after the last three answers"
WHY_RULE5_DRAFT = "a crossroads petition comes once on its branch: it waits on the first life's mark Brick.Form.Society, which every first crossroads takes away, or on the lack of a tag, which its own choice gives (B7x header section 3.1, 3.3 rule 5)"
WHY_RULE5_STANDING = "a wanderer's crossroads card - the enc_cross_ rows, marked by their time vector condition - speaks of the people that owns the system, so it is a Standing card: an arrival card speaks of the place alone and is not held by the gap rule (B7x header section 3.1, 3.3 rule 5)"
WHY_SHUT_STAR = "the brick line of the shut sphere waits on traits alone, so it is named: no rest or silence, none of the fenced words of the crossroads (B7x header 3.5)"
WHY_DRIFT_PLENTY = "the brick line of the wandering swarm's plenty waits on traits alone, so it is named: no rest or silence, none of the fenced words of the crossroads (B7x header 3.5)"
WHY_EMBER_WAKE = "the brick line of the star eater's trembling star waits on a trait and a hazard, so it is named: no rest or silence, none of the fenced words of the crossroads (B7x header 3.5)"
LINE_WORDS = [{"col": "LogTemplates", "lacks_words": "silence_words"}, {"col": "LogTemplates", "lacks_words": "crossroads_words"}]


def brick_tests(column):
    return [{"col": column, "lacks_cond": {"type": kind, "tag": tag}} for kind in ("AddTrait", "RemoveTrait") for tag in BRICKS]


ENCOUNTER_RULES = [
    {"why": WHY_RULE4_FAILURE, "then": brick_tests("Choices.OnFailure")},
    {"why": WHY_RULE4_FREE,
     "when": [{"none": [{"col": "Choices.Cost", "zero": True}, {"col": "Choices.OnSuccess", "max_len": 0},
                        {"col": "Choices.OnFailure", "max_len": 0}, {"col": "Choices.NextEncounter", "set": False}]}],
     "then": brick_tests("Choices.OnSuccess")},
    {"why": WHY_RULE5_LOTTERY, "when": [{"row_prefix": "petition_"}, {"col": "Conditions", "has_type": "CivTimeVectorAtLeast"}],
     "then": [{"col": "Trigger", "is": "Lottery"}]},
    {"why": WHY_RULE5_CLOSES,
     "when": [{"row_prefix": "petition_"}, {"col": "Conditions", "has_type": "CivTimeVectorAtLeast"},
              {"col": "Conditions", "lacks_cond": {"type": "CivHasTrait", "tag": "Brick.Form.Society"}}],
     "then": [{"col": "Conditions", "has_type": "CivLacksTrait"}]},
    {"why": WHY_RULE5_STANDING, "when": [{"row_prefix": "enc_"}, {"col": "Conditions", "has_type": "CivTimeVectorAtLeast"}],
     "then": [{"col": "Trigger", "is": "Standing"}]},
]
EVENT_RULES = [
    {"why": WHY_SHUT_STAR, "when": [{"row": "event_brick_shut_star"}], "then": LINE_WORDS},
    {"why": WHY_DRIFT_PLENTY, "when": [{"row": "event_brick_drift_plenty"}], "then": LINE_WORDS},
    {"why": WHY_EMBER_WAKE, "when": [{"row": "event_brick_ember_wake"}], "then": LINE_WORDS},
]
# A rule of the draft that this port replaces: finding it means the tree holds aa09543's schema - a conflict.
DRAFT_WHYS = {("DT_Encounters", WHY_RULE5_DRAFT)}


def compact(value):
    return json.dumps(value, ensure_ascii=False)


def render_rule(rule, inline_then=False):
    """One rule as schema.json writes it (aa09543's form): four tabs, why / when / then; a then of one test, or any then
    with inline_then, on its line; the 'none' when of rule 4 spread one test a line."""
    t4, t5, t6, t7 = "\t" * 4, "\t" * 5, "\t" * 6, "\t" * 7
    lines = [t4 + "{", t5 + '"why": ' + compact(rule["why"]) + ","]
    if "when" in rule:
        when = rule["when"]
        if len(when) == 1 and "none" in when[0]:
            lines.append(t5 + '"when": [')
            lines.append(t6 + '{"none": [')
            lines.append(",\n".join(t7 + compact(test) for test in when[0]["none"]))
            lines.append(t6 + "]}")
            lines.append(t5 + "],")
        else:
            lines.append(t5 + '"when": ' + compact(when) + ",")
    if len(rule["then"]) == 1 or inline_then:
        lines.append(t5 + '"then": ' + compact(rule["then"]))
    else:
        lines.append(t5 + '"then": [')
        lines.append(",\n".join(t6 + compact(test) for test in rule["then"]))
        lines.append(t5 + "]")
    lines.append(t4 + "}")
    return "\n".join(lines)


# --- test_data_schema.py --------------------------------------------------------------------------------------------------
FIXTURE_ANCHOR = '    tables["DT_Transcendents"] = '
FIXTURE_TEXT = '''    # B7x [B-7x5c] (the text audit's 7.3): the three brick lines no condition rule reaches - they wait on traits alone or on a
    # trait and a hazard - take row rules by their names, and a rule's row has to be a row of the table.
    for name in ("event_brick_shut_star", "event_brick_drift_plenty", "event_brick_ember_wake"):
        tables["DT_Events"].append(event_row(name))
'''
SETUPS_ANCHOR = "        }\n\n    def test_every_rule_of_the_schema_can_fail(self):\n"
SETUPS_TEXT = '''            # B7x [B-7x5c] (header section 3.3 rules 4 and 5): a brick on a roll's success or a choice not rolled for, the free
            # choice of a card that gives or takes a brick, and the trigger and the closing door of the crossroads cards.
            ("DT_Encounters", "%(r4a)s"):
                (NOTHING, plea),
            ("DT_Encounters", "%(r4b)s"):
                (update_plea(Choices=[choice(Cost=pool(Energy=5.0)), choice("Wait.", SuccessText="{Civ} waited.", Cost=pool(Matter=1.0))]), plea),
            ("DT_Encounters", "%(r5a)s"):
                (update_plea(Trigger="Lottery", Conditions=[cond("PlayerRoleIs", key="Ruler"), cond("CivHasTrait", tag="Brick.Form.Society"),
                                                            cond("CivTimeVectorAtLeast", key="Built", value=1200)]), plea),
            ("DT_Encounters", "%(r5b)s"):
                (update_plea(Trigger="Lottery", Conditions=[cond("PlayerRoleIs", key="Ruler"), cond("CivTimeVectorAtLeast", key="Linked", value=1800),
                                                            cond("CivLacksTrait", tag="Brick.Mutation.DustBand")]), plea),
            ("DT_Encounters", "%(r5c)s"):
                (lambda tables: self.row(tables, "DT_Encounters", "enc_test_weave").update(
                    Trigger="Standing", Conditions=[cond("PlayerRoleIs", key="Wanderer"), cond("CivTimeVectorAtLeast", key="Drifted", value=900)]),
                 "enc_test_weave"),
            # B7x [B-7x5c] (the text audit's 7.3): the brick lines no condition rule reaches, by their names.
            ("DT_Events", "%(n1)s"):
                (NOTHING, "event_brick_shut_star"),
            ("DT_Events", "%(n2)s"):
                (NOTHING, "event_brick_drift_plenty"),
            ("DT_Events", "%(n3)s"):
                (NOTHING, "event_brick_ember_wake"),
''' % {"r4a": WHY_RULE4_FAILURE, "r4b": WHY_RULE4_FREE, "r5a": WHY_RULE5_LOTTERY, "r5b": WHY_RULE5_CLOSES, "r5c": WHY_RULE5_STANDING,
       "n1": WHY_SHUT_STAR, "n2": WHY_DRIFT_PLENTY, "n3": WHY_EMBER_WAKE}
TESTS_ANCHOR = "    def test_per_tick_column(self):\n"
TEST_CARDS = '''    def test_crossroads_cards(self):
        """B7x [B-7x5c] (header section 3.3 rules 4 and 5; first drafted in [B-7x5a]): the breaking case the header names for
        each - a card whose every choice gives a brick (and a brick on a roll's failure), a crossroads petition that is
        Standing or never closes, a wanderer's crossroads card that is an arrival card - and the twelve cards of the project
        pass. Two pins of the project's rows beside them: the first life's row carries the start mark Brick.Form.Society, and
        choice 2 of a second crossroads petition carries no repeatable gain - a stability or resource change and the voice's
        Spiritualism tilt only (decision 11 (na) as the lead ruled it in [B-7x4]) - except the revert of
        petition_cross_wrapped_star, which closes the card by taking its parent away."""
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
        plea = "petition_test_plea"
        update_plea = lambda **changes: (lambda tables: self.row(tables, "DT_Encounters", plea).update(**changes))  # noqa: E731
        give = lambda tag, label: choice(label, Cost=pool(Matter=10.0), OnSuccess=[eff("AddTrait", tag=tag)], SuccessText="{Civ} changed.")  # noqa: E731
        # Rule 4: three choices, each gives a brick - no free choice.
        every = [give("Brick.Space.Nomad", "Go."), give("Brick.Instinct.SwarmTech", "Build."), give("Brick.Diet.DarkMatter", "Sift.")]
        self.expect(self.after(update_plea(Choices=every)), "DT_Encounters.%s" % plea, "a card that gives or takes a brick holds a choice")
        self.assertEqual(self.after(update_plea(Choices=every[:2] + [choice("Watch.", OnSuccess=[], SuccessText="{Civ} watched.")])), [])
        # A free choice is free of effects too, and of a next card.
        self.expect(self.after(update_plea(Choices=every[:2] + [choice("Watch.", SuccessText="{Civ} watched.")])), "a card that gives or takes a brick holds a choice")
        # Rule 4: a brick on the failure of a roll, taken or given.
        rolled = choice("Try.", CheckStat="Stability", Difficulty=8, OnSuccess=[], OnFailure=[eff("RemoveTrait", tag="Brick.Form.Society")],
                        SuccessText="{Civ} tried.", FailureText="{Civ} failed.")
        self.expect(self.after(update_plea(Choices=[rolled, choice("Watch.", OnSuccess=[], SuccessText="{Civ} watched.")])),
                    "DT_Encounters.%s" % plea, "never on its failure", "Brick.Form.Society")
        # Rule 5: a crossroads petition is a Lottery row and waits on the first life's mark or on a lack.
        first = [cond("PlayerRoleIs", key="Ruler"), cond("CivHasTrait", tag="Brick.Form.Society"), cond("CivTimeVectorAtLeast", key="Built", value=1200)]
        self.assertEqual(self.after(update_plea(Trigger="Lottery", Conditions=first)), [])
        self.expect(self.after(update_plea(Trigger="Standing", Conditions=first)), "DT_Encounters.%s" % plea, "comes in the periodic slot")
        open_ended = [cond("PlayerRoleIs", key="Ruler"), cond("CivHasTrait", tag="Brick.Instinct.SwarmTech"), cond("CivTimeVectorAtLeast", key="Linked", value=1800)]
        self.expect(self.after(update_plea(Trigger="Lottery", Conditions=open_ended)), "DT_Encounters.%s" % plea, "closes on its branch")
        self.assertEqual(self.after(update_plea(Trigger="Lottery", Conditions=open_ended + [cond("CivLacksTrait", tag="Brick.Instinct.HiveMerge")])), [])
        # Rule 5: a wanderer's crossroads card is Standing, never an arrival card.
        weave = lambda trigger: (lambda tables: self.row(tables, "DT_Encounters", "enc_test_weave").update(  # noqa: E731
            Trigger=trigger, Conditions=[cond("PlayerRoleIs", key="Wanderer"), cond("CivTimeVectorAtLeast", key="Drifted", value=900)]))
        self.expect(self.after(weave("Lottery")), "DT_Encounters.enc_test_weave", "a wanderer's crossroads card")
        self.assertEqual(self.after(weave("Standing")), [])

'''
TEST_LINES = '''    def test_crossroads_brick_lines(self):
        """B7x [B-7x5c] (header 3.5; the text audit's 7.3 and F12; notes 5.4 data-3): the three brick lines no condition rule
        reaches - event_brick_shut_star and event_brick_drift_plenty wait on traits alone, event_brick_ember_wake on a trait
        and a hazard - take row rules by their names, silence_words and crossroads_words on their lines. The rules pinned by
        hand; the project's rows ask neither condition of the [B-7x4] rules; each list broken on each row and refused by that
        rule alone - the same input passes with the rule taken out (the fixture rows ask no time vector and no bordering
        trait)."""
        self.assertEqual(self.after(), [])
        named = {
            "event_brick_shut_star": "%(n1)s",
            "event_brick_drift_plenty": "%(n2)s",
            "event_brick_ember_wake": "%(n3)s",
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
                    self.expect(self.after(change), "DT_Events.%%s" %% row_name, why, json.dumps({"col": "LogTemplates", "lacks_words": listed}))
                    self.assertEqual(self.after(change, without(why)), [], "the input has to pass without the rule: %%s" %% listed)

''' % {"n1": WHY_SHUT_STAR, "n2": WHY_DRIFT_PLENTY, "n3": WHY_EMBER_WAKE}
# Cases of earlier tests that the new rules reach (label, old text, new text). A crossroads card of [B-7x4]'s cases asked a
# time vector on the fixture's Standing plea with no lack: rule 5 now wants it a Lottery row that waits on the first life's
# mark. test_tables replaced DT_Events by one wrong row: the named brick rules then name rows the table neither requires
# nor has (the checker refuses the schema before the data), so the wrong row is appended instead.
REPLACEMENTS = [
    ("test_crossroads_words: the crossroads card is a first crossroads (rule 5)",
     '''                row = self.row(tables, "DT_Encounters", "petition_test_plea")
                row["Conditions"] = [ruler, vector]
''',
     '''                row = self.row(tables, "DT_Encounters", "petition_test_plea")
                # A first crossroads card (B7x [B-7x5c], header 3.3 rule 5): a Lottery row that waits on the first life's mark.
                row.update(Trigger="Lottery", Conditions=[ruler, cond("CivHasTrait", tag="Brick.Form.Society"), vector])
'''),
    ("rule_setups: the [B-7x4] crossroads card setup is a first crossroads (rule 5)",
     '''                (update_plea(Conditions=[cond("PlayerRoleIs", key="Ruler"), cond("CivTimeVectorAtLeast", key="Built", value=600)]), plea),
''',
     '''                (update_plea(Trigger="Lottery", Conditions=[cond("PlayerRoleIs", key="Ruler"), cond("CivHasTrait", tag="Brick.Form.Society"),
                                                            cond("CivTimeVectorAtLeast", key="Built", value=600)]), plea),
'''),
    ("test_tables: the wrong DT_Events row is appended (the named brick rules need their rows)",
     '''        self.expect(self.after(lambda tables: tables.update(DT_Events=[transcendent_row("event_wrong")])), "does not declare this column")
''',
     '''        self.expect(self.after(lambda tables: tables["DT_Events"].append(transcendent_row("event_wrong"))), "does not declare this column")
'''),
    ("test_tables: an absent DT_Events takes the named brick rules with it",
     '''        self.assertEqual(self.after(pop_events, self.put(["tables", "DT_Events"], "must_exist", False)), [])
        self.expect(self.after(pop_events, self.put(["tables", "DT_Events"], "must_exist", True)), "DT_Events.json is missing")
''',
     '''        # B7x [B-7x5c]: the brick lines' row rules name rows the table has, not rows it requires (the code calls none of them),
        # so an absent table takes those rules with it - or the checker refuses the schema before it looks at the data.
        unnamed = lambda schema: schema["tables"]["DT_Events"].update(rules=[  # noqa: E731
            rule for rule in schema["tables"]["DT_Events"]["rules"] if not any(str(test.get("row", "")).startswith("event_brick_") for test in rule.get("when", []))])
        may_go = lambda flag: (lambda schema: (self.put(["tables", "DT_Events"], "must_exist", flag)(schema), unnamed(schema)))  # noqa: E731
        self.assertEqual(self.after(pop_events, may_go(False)), [])
        self.expect(self.after(pop_events, may_go(True)), "DT_Events.json is missing")
'''),
]
WALK_PIN = re.compile(r'self\.assertEqual\(walked, (\d+), "the number of then-tests in schema\.json changed')


class Refused(Exception):
    pass


def say(text):
    sys.stdout.write(text + "\n")


# --- the rows --------------------------------------------------------------------------------------------------------------
def renamed(value, where, notes):
    """value with every old brick tag renamed (Tag fields of conditions and effects); each rename noted."""
    if isinstance(value, dict):
        return {key: renamed(item, where + "." + key, notes) for key, item in value.items()}
    if isinstance(value, list):
        return [renamed(item, "%s[%d]" % (where, index), notes) for index, item in enumerate(value)]
    if isinstance(value, str) and value in RENAME:
        notes.append("%s: %s -> %s (decision 3)" % (where, value, RENAME[value]))
        return RENAME[value]
    return value


def set_texts(row, block, notes):
    for column in ("Title", "Body", "VoiceBodies", "LogTemplates"):
        if column in block:
            if column not in row:
                raise Refused("%s: the audited block has %s, the row has no such column" % (row["Name"], column))
            if row[column] != block[column]:
                notes.append("%s.%s: audited text" % (row["Name"], column))
            row[column] = block[column]
    if "Choices" in block:
        if len(block["Choices"]) != len(row["Choices"]):
            raise Refused("%s: %d audited choices for %d choices" % (row["Name"], len(block["Choices"]), len(row["Choices"])))
        for index, (choice, texts) in enumerate(zip(row["Choices"], block["Choices"])):
            for column in ("Label", "SuccessText", "FailureText"):
                if choice[column] != texts[column]:
                    notes.append("%s.Choices[%d].%s: audited text" % (row["Name"], index, column))
                choice[column] = texts[column]
    extra = set(block) - {"Title", "Body", "VoiceBodies", "LogTemplates", "Choices"}
    if extra:
        raise Refused("%s: the audited block holds fields this port does not place: %s" % (row["Name"], sorted(extra)))


def tilt_of(effects):
    return [entry["Value"] for entry in effects if entry["Type"] == "ShiftEthics" and entry["Key"] == "Spiritualism"]


def build_rows():
    """(encounter rows, event rows, notes) - aa09543's rows, renamed, re-texted, with the ruled effects."""
    notes = []
    cards = []
    for source in AA_ENCOUNTERS:
        row = renamed(copy.deepcopy(source), source["Name"], notes)
        set_texts(row, AUDITED[row["Name"]], notes)
        cards.append(row)
    by_name = {row["Name"]: row for row in cards}
    for key, effects in RULED_EFFECTS.items():
        match = re.fullmatch(r"([a-z_]+)\.Choices\[(\d)\]\.(OnSuccess|OnFailure)", key)
        if not match or match.group(1) not in by_name:
            raise Refused("a ruled effect names no crossroads choice: %s" % key)
        choice = by_name[match.group(1)]["Choices"][int(match.group(2))]
        before = choice[match.group(3)]
        # The rulings: where a ruled list keeps the tilt, its sign is aa09543's (the porter checks it).
        if tilt_of(effects) and tilt_of(effects) != tilt_of(before):
            raise Refused("%s: the ruled tilt %s is not aa09543's %s" % (key, tilt_of(effects), tilt_of(before)))
        choice[match.group(3)] = copy.deepcopy(effects)
        notes.append("%s: effects %s -> %s (lead's ruling 1, decision 11 (na))" % (
            key, [(e["Type"], e["Key"], e["Value"]) for e in before], [(e["Type"], e["Key"], e["Value"]) for e in effects]))
    for (name, path), text in RULED_TEXTS.items():
        row = by_name[name]
        match = re.fullmatch(r"Choices\[(\d)\]\.(\w+)", path)
        holder, column = (row["Choices"][int(match.group(1))], match.group(2)) if match else (row, path)
        if column == "SuccessText" and holder[column].split(". ")[0] != text.split(". ")[0]:
            raise Refused("%s.%s: the ruled line does not start as the audited line" % (name, path))
        notes.append("%s.%s: %r -> %r (lead's ruling)" % (name, path, holder[column], text))
        holder[column] = text
    for name, title in EXPECTED_TITLES.items():
        if by_name[name]["Title"] != title:
            raise Refused("%s.Title is %r, the rulings say %r" % (name, by_name[name]["Title"], title))
    events = []
    for source in AA_EVENTS:
        row = renamed(copy.deepcopy(source), source["Name"], notes)
        set_texts(row, AUDITED[row["Name"]], notes)
        events.append(row)
    for row in cards + events:
        left = OLD_LEAF.findall(json.dumps(row, ensure_ascii=False))
        if left:
            raise Refused("%s still spells an old leaf: %s" % (row["Name"], left))
    return cards, events, notes


# --- source check ----------------------------------------------------------------------------------------------------------
def source_check():
    missing = [path for path in (SOURCE_REPO, AUDITED_MD, RULINGS_MD) if not os.path.exists(path)]
    if missing:
        say("source check: skipped - not on this PC: %s" % ", ".join(missing))
        return
    def show(path):
        return json.loads(subprocess.run(["git", "-C", SOURCE_REPO, "show", "%s:%s" % (SOURCE_COMMIT, path)], check=True,
                                         capture_output=True).stdout.decode("utf-8"))
    cards = [row for row in show("Tools/data/DT_Encounters.json") if "_cross_" in row["Name"]]
    events = [row for row in show("Tools/data/DT_Events.json") if row["Name"].startswith("event_brick_")]
    if cards != AA_ENCOUNTERS or events != AA_EVENTS:
        raise Refused("the embedded aa09543 rows differ from %s in %s" % (SOURCE_COMMIT, SOURCE_REPO))
    text = open(AUDITED_MD, encoding="utf-8").read()
    marks = list(re.finditer(r"^### ([12])\.(\d+) `([a-z_]+)`", text, re.M))
    blocks = {}
    for index, mark in enumerate(marks):
        part = text[mark.end():marks[index + 1].start() if index + 1 < len(marks) else len(text)].split("\n## ")[0]
        blocks[mark.group(3)] = json.loads(re.findall(r"```json\n(.*?)\n```", part, re.S)[0])
    if blocks != AUDITED:
        raise Refused("the embedded audited texts differ from %s" % AUDITED_MD)
    rulings = open(RULINGS_MD, encoding="utf-8").read()
    if json.loads(re.findall(r"```json\n(.*?)\n```", rulings, re.S)[0]) != RULED_EFFECTS:
        raise Refused("the embedded ruled effects differ from %s" % RULINGS_MD)
    say("source check: the embedded rows, texts and ruled effects equal %s (%s), %s and %s" % (
        SOURCE_COMMIT, SOURCE_REPO, os.path.basename(AUDITED_MD), os.path.basename(RULINGS_MD)))


# --- the files -------------------------------------------------------------------------------------------------------------
def read(root, rel):
    """(text with LF line ends, whether the file ends its lines with CRLF). A BOM or mixed line ends is refused: the port
    writes the file back with the line ends it found and no other byte changed."""
    path = os.path.join(root, rel)
    with open(path, "rb") as handle:
        data = handle.read()
    if data.startswith(b"\xef\xbb\xbf"):
        raise Refused("%s: a BOM - this port reads and writes UTF-8 without one" % rel)
    crlf = data.count(b"\r\n")
    if crlf not in (0, data.count(b"\n")) or data.replace(b"\r\n", b"").count(b"\r"):
        raise Refused("%s: mixed line ends" % rel)
    return data.decode("utf-8").replace("\r\n", "\n"), crlf > 0


def append_rows(rel, text, rows, changes):
    """The table's text with the rows that are not there yet appended at its end, each as the file writes a row."""
    table = json.loads(text)
    have = {row["Name"]: row for row in table}
    new = []
    for row in rows:
        if row["Name"] in have:
            if have[row["Name"]] != row:
                diff = sorted(key for key in set(row) | set(have[row["Name"]]) if row.get(key) != have[row["Name"]].get(key))
                raise Refused("%s: the row %s is there with other content (columns %s) - a conflict" % (rel, row["Name"], diff))
            say("  %s: %s is there already (same content)" % (rel, row["Name"]))
            continue
        new.append(row)
    if not new:
        return text
    if not text.endswith("\n]\n"):
        raise Refused("%s: the table does not end with ']' on its own line" % rel)
    body = ",\n".join("\n".join("\t" + line for line in json.dumps(row, indent="\t", ensure_ascii=False).split("\n")) for row in new)
    out = text[:-len("\n]\n")] + ",\n" + body + "\n]\n"
    if json.loads(out) != table + new:
        raise Refused("%s: the appended text does not read back as the table plus the rows" % rel)
    for row in new:
        changes.append("%s: appended row %s" % (rel, row["Name"]))
    return out


def add_society(rel, text, changes):
    table = json.loads(text)
    seeker = [row for row in table if row.get("Name") == "arch_seeker"]
    if len(seeker) != 1:
        raise Refused("%s: no single arch_seeker row" % rel)
    traits = seeker[0]["Traits"]
    if '(TagName="Brick.Form.Society")' in traits:
        say("  %s: arch_seeker carries Brick.Form.Society already" % rel)
        return text
    if not (traits.startswith("(GameplayTags=(") and traits.endswith("))")):
        raise Refused("%s: arch_seeker.Traits is not a tag container text: %r" % (rel, traits))
    new_traits = traits[:-2] + ',(TagName="Brick.Form.Society")))'
    old_cell = '"Traits": ' + compact(traits)
    start = text.index('"Name": "arch_seeker"')
    end = text.find("\n\t}", start)
    if text.count(old_cell, start, end) != 1:
        raise Refused("%s: arch_seeker's Traits cell is not written once in its row" % rel)
    at = text.index(old_cell, start, end)
    out = text[:at] + '"Traits": ' + compact(new_traits) + text[at + len(old_cell):]
    expected = copy.deepcopy(table)
    [row for row in expected if row.get("Name") == "arch_seeker"][0]["Traits"] = new_traits
    if json.loads(out) != expected:
        raise Refused("%s: the edit does not read back as arch_seeker with the mark" % rel)
    changes.append("%s: arch_seeker.Traits += Brick.Form.Society" % rel)
    return out


def rules_span(text, table):
    """(start, end) of the inside of the table's rules array in schema.json: end is the index of the newline before the
    array's closing ']' (three tabs)."""
    head = '\n\t\t"%s": {\n' % table
    if text.count(head) != 1:
        raise Refused("schema.json: the table %s is not written once at the depth of tables" % table)
    at = text.index(head)
    close = text.find("\n\t\t}", at + len(head))
    rules = text.find('\n\t\t\t"rules": [\n', at, close)
    if rules < 0:
        raise Refused("schema.json: %s has no rules array" % table)
    end = text.find("\n\t\t\t]", rules)
    return rules, end


def add_rules(text, changes):
    schema = json.loads(text)
    for table, why in DRAFT_WHYS:
        if any(rule["why"] == why for rule in schema["tables"][table].get("rules", [])):
            raise Refused("schema.json: %s still holds the draft rule of aa09543 (%s...) - a conflict" % (table, why[:60]))
    expected = copy.deepcopy(schema)
    out = text
    for table, rules, inline in (("DT_Encounters", ENCOUNTER_RULES, False), ("DT_Events", EVENT_RULES, True)):
        have = {rule["why"]: rule for rule in schema["tables"][table].get("rules", [])}
        new = []
        for rule in rules:
            if rule["why"] in have:
                if have[rule["why"]] != rule:
                    raise Refused("schema.json: %s has the rule %r... with other content - a conflict" % (table, rule["why"][:70]))
                say("  schema.json: %s rule %r... is there already" % (table, rule["why"][:50]))
                continue
            new.append(rule)
        if not new:
            continue
        _, end = rules_span(out, table)
        if out[end - 1] != "}":
            raise Refused("schema.json: the last rule of %s does not end with '}' on its line" % table)
        out = out[:end] + ",\n" + ",\n".join(render_rule(rule, inline) for rule in new) + out[end:]
        expected["tables"][table]["rules"].extend(new)
        for rule in new:
            changes.append("schema.json: %s rules += %r... (%d then-tests)" % (table, rule["why"][:60], len(rule["then"])))
    if json.loads(out) != expected:
        raise Refused("schema.json: the edit does not read back as the schema plus the rules")
    return out, expected


def insert_once(rel, text, anchor, block, marker, changes, label, before=True):
    """block put before (or after) the one anchor, unless marker is there already (then the block has to be there whole)."""
    if marker in text:
        if block not in text:
            raise Refused("%s: %s is there with other content - a conflict" % (rel, label))
        say("  %s: %s is there already" % (rel, label))
        return text
    if text.count(anchor) != 1:
        raise Refused("%s: the anchor of %s is not in the file once: %r" % (rel, label, anchor[:60]))
    at = text.index(anchor) + (0 if before else len(anchor))
    changes.append("%s: %s" % (rel, label))
    return text[:at] + block + text[at:]


def port_tests(text, changes):
    rel = "Tools/test_data_schema.py"
    text = insert_once(rel, text, FIXTURE_ANCHOR, FIXTURE_TEXT, '("event_brick_shut_star", "event_brick_drift_plenty", "event_brick_ember_wake")',
                       changes, "fixture_tables: the three named brick lines")
    text = insert_once(rel, text, SETUPS_ANCHOR, SETUPS_TEXT, '("DT_Encounters", "%s")' % WHY_RULE4_FAILURE, changes,
                       "rule_setups: eight entries (rules 4 and 5, the three named brick lines)")
    text = insert_once(rel, text, TESTS_ANCHOR, TEST_CARDS, "    def test_crossroads_cards(self):\n", changes, "test_crossroads_cards")
    text = insert_once(rel, text, TESTS_ANCHOR, TEST_LINES, "    def test_crossroads_brick_lines(self):\n", changes, "test_crossroads_brick_lines")
    for label, old, new in REPLACEMENTS:
        if text.count(new) == 1 and old not in text:
            say("  %s: %s is done already" % (rel, label))
        elif text.count(old) == 1 and new not in text:
            text = text.replace(old, new)
            changes.append("%s: %s" % (rel, label))
        else:
            raise Refused("%s: %s - the old text is not there once (%d) and the new text is not there once (%d): place it by hand"
                          % (rel, label, text.count(old), text.count(new)))
    if OLD_LEAF.search(SETUPS_TEXT + TEST_CARDS + TEST_LINES + FIXTURE_TEXT):
        raise Refused("the test text spells an old leaf")
    return text


def main(argv):
    args = [arg for arg in argv if not arg.startswith("--")]
    flags = {arg for arg in argv if arg.startswith("--")}
    if len(args) != 1 or flags - {"--dry-run", "--no-source-check"}:
        say(__doc__.split("\n\n")[1])
        return 2
    root = os.path.abspath(args[0])
    dry = "--dry-run" in flags
    try:
        if "--no-source-check" not in flags:
            source_check()
        cards, events, notes = build_rows()
        say("built %d crossroads rows and %d brick event rows from %s:" % (len(cards), len(events), SOURCE_COMMIT))
        for note in notes:
            say("  " + note)
        files, crlf = {}, {}
        for rel in ("Tools/data/DT_Encounters.json", "Tools/data/DT_Events.json", "Tools/data/DT_Civs.json", "Tools/data/schema.json",
                    "Tools/test_data_schema.py"):
            files[rel], crlf[rel] = read(root, rel)
        changes = []
        out = dict(files)
        out["Tools/data/DT_Encounters.json"] = append_rows("Tools/data/DT_Encounters.json", files["Tools/data/DT_Encounters.json"], cards, changes)
        out["Tools/data/DT_Events.json"] = append_rows("Tools/data/DT_Events.json", files["Tools/data/DT_Events.json"], events, changes)
        out["Tools/data/DT_Civs.json"] = add_society("Tools/data/DT_Civs.json", files["Tools/data/DT_Civs.json"], changes)
        out["Tools/data/schema.json"], schema = add_rules(files["Tools/data/schema.json"], changes)
        out["Tools/test_data_schema.py"] = port_tests(files["Tools/test_data_schema.py"], changes)
    except Refused as problem:
        say("REFUSED (nothing written): %s" % problem)
        return 1
    say("changes (%d)%s:" % (len(changes), " - dry run, nothing written" if dry else ""))
    for change in changes:
        say("  " + change)
    if not dry:
        for rel, text in out.items():
            if text != files[rel]:
                with open(os.path.join(root, rel), "wb") as handle:
                    handle.write((text.replace("\n", "\r\n") if crlf[rel] else text).encode("utf-8"))
                say("  wrote %s%s" % (rel, " (CRLF kept)" if crlf[rel] else ""))
    walked = sum(len(rule["then"]) for spec in schema["tables"].values() for rule in spec.get("rules", []))
    pinned = WALK_PIN.findall(out["Tools/test_data_schema.py"])
    say("")
    say("follow-ups (this script does not do them):")
    say("  - the walked pin of test_every_rule_of_the_schema_can_fail reads %s; the schema now holds %d then-tests." % (
        pinned[0] if len(pinned) == 1 else "?", walked))
    say("    Run 'python -I Tools/test_data_schema.py', let the walk recount, and set the pin to the count it walks (expected %d)," % walked)
    say("    with a comment line such as '# B7x [B-7x5c]: %s + %d (rule 4: 2 x 34 bricks; rule 5: 3; the three named brick lines: 6) = %d.'" % (
        pinned[0] if len(pinned) == 1 else "N", walked - int(pinned[0]) if len(pinned) == 1 else 0, walked))
    say("  - python -I Tools/check_data.py; python -I Tools/test_data_schema.py; rename_leaves.py <tree> (0 changes expected here).")
    say("  - assets and Tools/data/generated.sha1: Tools\\run_checks.ps1 -Tier full -Data (editor closed).")
    say("  - by hand in the same commit: design 4.3 (C05/C07/C10/C11 choice 2 lines and effects, C10/C12 titles), notes 5.4")
    say("    data-1/data-2, header 11 (the C10 title's departure from the developer's answer).")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    sys.exit(main(sys.argv[1:]))
