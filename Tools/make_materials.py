"""Create or refill the materials of the galaxy map: /Game/CA/Materials/M_Star and M_Nebula.

Cosmic Ascension - Phase 3 helper (guideline.md section 6.4, 11 Phase 3). The materials are generated, like the data
tables: change this script and run it again (an edit made in the editor is overwritten). An existing asset is refilled in
place (its expressions are deleted and drawn again), never deleted and made anew.

Run headlessly with the editor CLOSED and the editor target already built:

  "C:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Binaries\\Win64\\UnrealEditor-Cmd.exe" "<PROJECT_ROOT>\\Cosmic_Ascension.uproject"
      -run=pythonscript -Script="<PROJECT_ROOT>/Tools/make_materials.py"
      -unattended -nosplash -stdout -FullStdOutLogOutput

The log ends with "[make_materials] M_Star written and verified" and the same line for M_Nebula.

Verified against UE 5.8.3 engine source:
  - UMaterialEditingLibrary (MaterialEditor module, exposed to Python as unreal.MaterialEditingLibrary):
    CreateMaterialExpression, ConnectMaterialExpressions, ConnectMaterialProperty, DeleteAllMaterialExpressions,
    RecompileMaterial, GetScalarParameterNames, GetVectorParameterNames.
  - With Substrate on (Config/DefaultEngine.ini), a material whose legacy pins are connected and whose FrontMaterial is
    not is converted on compile (UMaterial: the legacy pins go into a Substrate shading-models node), so the legacy
    Emissive pin is enough for an unlit material.
  - unreal.EditorAssetLibrary belongs to a plugin this project does not enable: the asset subsystem is used instead.

M_Star (unlit; parameters set by ACosmicStarSystemActor): emissive = StarColor x Brightness + a pulse of the warning
colour x HazardAmount over the whole star. The pulse runs on the material's own time (a rendering clock, never the
simulation's). The owner is not in the material: the map draws a ring of lines around the star (a rim on a sphere a few
pixels wide does not read from the overview).
M_Nebula (unlit, two-sided, on the inside of the sky sphere): a dark violet with noise.
M_AscensionGlow (user interface domain, translucent; parameters set by UCosmicAscensionOverlayWidget): a round glow of
GlowColor whose opacity falls from the centre of the image to its edge, times Glow (0..1, the fade of the presentation).
"""
import unreal

FOLDER = "/Game/CA/Materials"
MEL = unreal.MaterialEditingLibrary


def material(name):
    """The material asset, refilled in place when it exists."""
    assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    path = "%s/%s" % (FOLDER, name)
    if assets.does_asset_exist(path):
        mat = assets.load_asset(path)
        MEL.delete_all_material_expressions(mat)
        return mat
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    return tools.create_asset(name, FOLDER, unreal.Material, unreal.MaterialFactoryNew())


class Graph:
    def __init__(self, mat):
        self.mat = mat
        self.y = 0

    def node(self, cls, x=-600, **props):
        expr = MEL.create_material_expression(self.mat, cls, x, self.y)
        self.y += 120
        for key, value in props.items():
            expr.set_editor_property(key, value)
        return expr

    def vector(self, name, rgb):
        return self.node(unreal.MaterialExpressionVectorParameter, parameter_name=name, default_value=unreal.LinearColor(rgb[0], rgb[1], rgb[2], 1.0))

    def scalar(self, name, value):
        return self.node(unreal.MaterialExpressionScalarParameter, parameter_name=name, default_value=value)

    def constant3(self, rgb):
        return self.node(unreal.MaterialExpressionConstant3Vector, constant=unreal.LinearColor(rgb[0], rgb[1], rgb[2], 1.0))

    def binary(self, cls, a, b=None, const_b=None, x=-300):
        expr = self.node(cls, x)
        if not MEL.connect_material_expressions(a, "", expr, "A"):
            raise RuntimeError("could not connect A of %s" % cls)
        if b is not None:
            if not MEL.connect_material_expressions(b, "", expr, "B"):
                raise RuntimeError("could not connect B of %s" % cls)
        elif const_b is not None:
            expr.set_editor_property("const_b", const_b)
        return expr

    def mul(self, a, b=None, const_b=None):
        return self.binary(unreal.MaterialExpressionMultiply, a, b, const_b)

    def add(self, a, b=None, const_b=None):
        return self.binary(unreal.MaterialExpressionAdd, a, b, const_b)

    def emissive(self, expr):
        if not MEL.connect_material_property(expr, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR):
            raise RuntimeError("could not connect the emissive pin")


def finish(mat, name, scalars, vectors):
    MEL.layout_material_expressions(mat)
    MEL.recompile_material(mat)
    assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    if not assets.save_loaded_asset(mat, False):
        raise RuntimeError("could not save %s" % name)
    found_scalars = [str(n) for n in MEL.get_scalar_parameter_names(mat)]
    found_vectors = [str(n) for n in MEL.get_vector_parameter_names(mat)]
    missing = [n for n in scalars if n not in found_scalars] + [n for n in vectors if n not in found_vectors]
    if missing:
        raise RuntimeError("%s lacks the parameters %s" % (name, ", ".join(missing)))
    unreal.log("[make_materials] %s written and verified" % name)


def wire_time(g, sine):
    """The sine reads the material's own time (a rendering clock, never the simulation's)."""
    time = g.node(unreal.MaterialExpressionTime, x=-1100)
    if not MEL.connect_material_expressions(time, "", sine, ""):
        raise RuntimeError("could not connect the time to the sine")


def make_star_material():
    mat = material("M_Star")
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    g = Graph(mat)
    core = g.mul(g.vector("StarColor", (1.0, 1.0, 1.0)), g.scalar("Brightness", 1.3))
    sine = g.node(unreal.MaterialExpressionSine, x=-900, period=1.0)
    wire_time(g, sine)
    pulse = g.add(g.mul(sine, const_b=0.5), const_b=0.5)
    hazard = g.mul(g.mul(g.mul(g.constant3((1.0, 0.25, 0.05)), g.scalar("HazardAmount", 0.0)), pulse), const_b=3.0)
    g.emissive(g.add(core, hazard))
    finish(mat, "M_Star", ["Brightness", "HazardAmount"], ["StarColor"])


def make_nebula_material():
    mat = material("M_Nebula")
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("two_sided", True)
    g = Graph(mat)
    noise = g.node(unreal.MaterialExpressionNoise, x=-900, scale=0.00003, levels=4, output_min=0.0, output_max=1.0)
    alpha = g.mul(noise, noise)
    lerp = g.node(unreal.MaterialExpressionLinearInterpolate)
    for pin, expr in (("A", g.constant3((0.0015, 0.001, 0.004))), ("B", g.constant3((0.018, 0.007, 0.03))), ("Alpha", alpha)):
        if not MEL.connect_material_expressions(expr, "", lerp, pin):
            raise RuntimeError("could not connect %s of the nebula" % pin)
    g.emissive(lerp)
    finish(mat, "M_Nebula", [], [])


def make_ascension_glow_material():
    mat = material("M_AscensionGlow")
    mat.set_editor_property("material_domain", unreal.MaterialDomain.MD_UI)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    g = Graph(mat)
    uv = g.node(unreal.MaterialExpressionTextureCoordinate, x=-1100)
    distance = g.node(unreal.MaterialExpressionDistance, x=-900)
    centre = g.node(unreal.MaterialExpressionConstant2Vector, x=-1100, r=0.5, g=0.5)
    for pin, expr in (("A", uv), ("B", centre)):
        if not MEL.connect_material_expressions(expr, "", distance, pin):
            raise RuntimeError("could not connect %s of the glow distance" % pin)
    # 1 at the centre, 0 at the edge of the image (distance 0.5), squared for a soft falloff.
    falloff = g.node(unreal.MaterialExpressionOneMinus, x=-700)
    if not MEL.connect_material_expressions(g.mul(distance, const_b=2.0), "", falloff, ""):
        raise RuntimeError("could not connect the glow falloff")
    clamped = g.node(unreal.MaterialExpressionSaturate, x=-500)
    if not MEL.connect_material_expressions(falloff, "", clamped, ""):
        raise RuntimeError("could not clamp the glow")
    soft = g.node(unreal.MaterialExpressionPower, x=-400, const_exponent=2.0)
    if not MEL.connect_material_expressions(clamped, "", soft, "Base"):
        raise RuntimeError("could not soften the glow")
    opacity = g.mul(soft, g.scalar("Glow", 0.0))
    if not MEL.connect_material_property(opacity, "", unreal.MaterialProperty.MP_OPACITY):
        raise RuntimeError("could not connect the opacity pin")
    g.emissive(g.vector("GlowColor", (1.0, 0.8, 0.4)))
    finish(mat, "M_AscensionGlow", ["Glow"], ["GlowColor"])


make_star_material()
make_nebula_material()
make_ascension_glow_material()
