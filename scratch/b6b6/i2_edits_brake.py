EDITS = [
    ("\tif (!Galaxy || Galaxy->AIFourthRungCap <= 0)\n\t{\n\t\treturn true;\n\t}\n",
     "\tif (!Galaxy || Galaxy->AIFourthRungCap <= 0)\n\t{\n\t\treturn false;\n\t}\n"),
    ("\treturn Count < Galaxy->AIFourthRungCap;\n",
     "\treturn false && Count < Galaxy->AIFourthRungCap;\n"),
]
