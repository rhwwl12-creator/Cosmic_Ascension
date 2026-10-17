"""Create the Widget Blueprints the C++ UI needs: /Game/CA/UI/WBP_LogEntry (parent UCosmicLogEntryWidget).

Cosmic Ascension - Phase 3 helper (guideline.md section 6.3, 11 Phase 3). The UI is built in C++; a blueprint exists
only where the engine asks for one. In an editor build UListView accepts only a Blueprint entry class
(UListViewBase::RebuildListWidget, WITH_EDITOR: "EntryWidgetClass [...] is not a Blueprint class"), so the log feed's
entry class is an empty blueprint whose parent builds its tree in C++. An existing blueprint is kept (its parent is
checked), never deleted and made anew.

Run headlessly with the editor CLOSED and the editor target already built:

  "C:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Binaries\\Win64\\UnrealEditor-Cmd.exe" "<PROJECT_ROOT>\\Cosmic_Ascension.uproject"
      -run=pythonscript -Script="<PROJECT_ROOT>/Tools/make_widget_blueprints.py"
      -unattended -nosplash -stdout -FullStdOutLogOutput

The log ends with "[make_widget_blueprints] WBP_LogEntry written and verified".

Verified against UE 5.8.3 engine source: UWidgetBlueprintFactory (UMGEditor, unreal.WidgetBlueprintFactory) has the
property ParentClass; IAssetTools::CreateAsset makes and compiles the blueprint. Python cannot read UBlueprint::ParentClass
(not exposed), so the parent is checked through the class default object of the generated class.
"""
import unreal

FOLDER = "/Game/CA/UI"
# (asset name, parent class)
BLUEPRINTS = [
    ("WBP_LogEntry", unreal.CosmicLogEntryWidget),
]


def blueprint(name, parent):
    assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    path = "%s/%s" % (FOLDER, name)
    if assets.does_asset_exist(path):
        # The parent is checked through the generated class below (Python cannot read UBlueprint::ParentClass).
        return assets.load_asset(path)
    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent)
    created = unreal.AssetToolsHelpers.get_asset_tools().create_asset(name, FOLDER, unreal.WidgetBlueprint, factory)
    if created is None:
        raise RuntimeError("could not create %s" % path)
    return created


def main():
    assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    for name, parent in BLUEPRINTS:
        made = blueprint(name, parent)
        if not assets.save_loaded_asset(made, False):
            raise RuntimeError("could not save %s" % name)
        generated = unreal.load_class(None, "%s/%s.%s_C" % (FOLDER, name, name))
        if generated is None:
            raise RuntimeError("%s has no generated class" % name)
        if not isinstance(unreal.get_default_object(generated), parent):
            raise RuntimeError("%s is not a %s" % (name, parent.__name__))
        unreal.log("[make_widget_blueprints] %s written and verified" % name)


main()
