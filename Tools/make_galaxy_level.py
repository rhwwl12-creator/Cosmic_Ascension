"""Create and save the empty, non-partitioned galaxy level /Game/CA/Maps/L_Galaxy.

Cosmic Ascension - Phase 0 helper (guideline.md section 10.1, 11 Phase 0 exit criteria).

Run headlessly with the editor CLOSED and the editor target already built:

  "C:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Binaries\\Win64\\UnrealEditor-Cmd.exe" "<PROJECT_ROOT>\\Cosmic_Ascension.uproject"
      -run=pythonscript -Script="<PROJECT_ROOT>/Tools/make_galaxy_level.py"
      -unattended -nosplash -stdout -FullStdOutLogOutput

(PowerShell: put --% right after the exe path so the quoted -Script= value is passed verbatim.)

Verified against UE 5.8.3 engine source:
  - -run=pythonscript -> UPythonScriptCommandlet, which reads -Script="..." and runs the .py file.
  - ULevelEditorSubsystem::NewLevel(AssetPath, bIsPartitionedWorld=false) creates the level, SAVES it
    (UEditorLoadingAndSavingUtils::SaveMap) and loads it. It refuses to overwrite an existing package.
  - unreal.EditorLevelLibrary.new_level is UE_DEPRECATED(5.0); do not use it.
Needs the PythonScriptPlugin enabled for the editor target. Cosmic_Ascension.uproject lists it with
TargetAllowList=[Editor]; note that Niagara.uplugin also pulls the plugin into game targets (as a
WITH_PYTHON=0 stub), so the allow-list is not what keeps Python out of the game. Python only runs in
editor binaries such as UnrealEditor-Cmd.
"""
import os

import unreal

LEVEL_PATH = "/Game/CA/Maps/L_Galaxy"


def _umap_on_disk():
    """Return the expected .umap path on disk (this script lives in <ProjectRoot>/Tools/)."""
    try:
        project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    except NameError:
        return None
    return os.path.join(project_root, "Content", "CA", "Maps", "L_Galaxy.umap")


umap_path = _umap_on_disk()
if umap_path and os.path.exists(umap_path):
    unreal.log(f"[make_galaxy_level] {umap_path} already exists; nothing to do")
else:
    level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if level_editor is None:
        raise RuntimeError("LevelEditorSubsystem not available (not an editor context)")

    # Second argument False -> plain level, no World Partition (passed positionally so we do not
    # depend on the generated keyword name). NewLevel saves the package itself.
    if not level_editor.new_level(LEVEL_PATH, False):
        raise RuntimeError(f"new_level failed for {LEVEL_PATH} (already exists or invalid path?)")

    # Redundant re-save; proves the level has a filename and flushes any pending state.
    if not level_editor.save_current_level():
        raise RuntimeError("save_current_level failed")

    unreal.log(f"[make_galaxy_level] created and saved {LEVEL_PATH}")
