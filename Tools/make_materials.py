"""Create or refill the materials of the galaxy map: /Game/CA/Materials/M_Star and M_Nebula.

Cosmic Ascension - Phase 3 helper (guideline.md section 6.4, 11 Phase 3). The materials are generated, like the data
tables: change this script and run it again (an edit made in the editor is overwritten). An existing asset is refilled in
place (its expressions are deleted and drawn again), never deleted and made anew.

Run headlessly with the editor CLOSED and the editor target already built:

  "C:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Binaries\\Win64\\UnrealEditor-Cmd.exe" "<PROJECT_ROOT>\\Cosmic_Ascension.uproject"
      -run=pythonscript -Script="<PROJECT_ROOT>/Tools/make_materials.py"
      -unattended -nosplash -stdout -FullStdOutLogOutput

The log ends with "[make_materials] M_Star written and verified" and the same line for M_Nebula and M_AscensionGlow.

Verified against UE 5.8.3 engine source:
  - UMaterialEditingLibrary (MaterialEditor module, exposed to Python as unreal.MaterialEditingLibrary):
    CreateMaterialExpression, ConnectMaterialExpressions, ConnectMaterialProperty, GetMaterialExpressions,
    DeleteMaterialExpression, RecompileMaterial, GetScalarParameterNames, GetVectorParameterNames.
    ConnectMaterialExpressions maps the input name "" to the first input of an expression (Abs, OneMinus, Saturate,
    Sine, the Input pin of Desaturation).
  - DeleteAllMaterialExpressions is not used: it removes each expression from the array it is iterating (a view of
    UMaterial::GetExpressions), so it skips every other one. Refilled with it, an asset kept about half of its old
    nodes each run - disconnected, so nothing drew them, but a stale parameter node answered for the material's
    defaults by its name (GetScalarParameterValue on the base material reads the first node of that name). The script
    deletes from a copied list (get_material_expressions returns a copy) and checks that none is left, and finish
    checks that every parameter name is held by exactly one node - before it saves, so that only a verified graph is
    written.
  - MaterialExpressionDesaturation has the inputs Input and Fraction: Lerp(colour, its luminance grey, Fraction), so a
    Fraction of 0 gives the colour exactly. The fraction is not clamped (UMaterialExpressionDesaturation::Compile), so a
    negative one pushes the colour away from its grey: DEEPEN below.
  - With Substrate on (Config/DefaultEngine.ini), a material whose legacy pins are connected and whose FrontMaterial is
    not is converted on compile (UMaterial: the legacy pins go into a Substrate shading-models node), so the legacy
    Emissive pin is enough for an unlit material.
  - unreal.EditorAssetLibrary belongs to a plugin this project does not enable: the asset subsystem is used instead.

M_Star (unlit; parameters set by ACosmicStarSystemActor, values chosen by CosmicMapLook::StarParamsOf): emissive =
the core StarColor x Brightness, tinted in the spiritual view toward the fallen's purple, + WarningColor x HazardAmount
x the warning's pulse x 3. The tint is a lerp of the core toward TINT_LEVEL x deep DominionColor by ViewMode x
saturate(Pressure x a slow pulse x PRESSURE_GAIN + held x HELD_TINT + claimed x the warning's pulse x CLAIM_GAIN), where
claimed and held are masks of ClaimState (1 and 2; 3, bound, has its ring on the map) and deep DominionColor is the
colour pushed away from its own grey by DEEPEN (a Desaturation with a negative fraction: the hue stays, the saturation
grows). The mark changes the star's hue, not its light: the tint is about as bright as a dimmed star of the sight, so a
marked star reads purple and never as a white glow (guideline 6.4 - the fallen are given no glory; the feed's pale
purple added on top of a star read as near-white light, the brightest thing on the map). Both pulses run on the
material's own time (a rendering clock, never the simulation's): the warning's every second, the slow pulse of the
pressure every three. With ViewMode 0 the lerp keeps the core exactly, so the physical view is item 1's look. The owner
is not in the material: the map draws a ring of lines around the star (a rim on a sphere a few pixels wide does not read
from the overview).
M_Nebula (unlit, two-sided, on the inside of the sky sphere): a dark violet with noise; SpiritualIntensity (0 physical,
1 spiritual - the map's ACosmicGalaxyMapActor) desaturates it and dims it by SKY_DIM, so the sky gives the spiritual view
no purple (the owners' rings keep out of the fallen's hues there too - CosmicMapLook::OwnerRingColorOf). At 0 the output
is the plain nebula exactly.
M_AscensionGlow (user interface domain, translucent; parameters set by UCosmicAscensionOverlayWidget): a round glow of
GlowColor whose opacity falls from the centre of the image to its edge, times Glow (0..1, the fade of the presentation).
"""
import unreal

FOLDER = "/Game/CA/Materials"
MEL = unreal.MaterialEditingLibrary

# The physical warning of a hazard (CosmicMapLook::StarParamsOf sends it; the default keeps item 1's orange).
WARNING_ORANGE = (1.0, 0.25, 0.05)
# The spiritual view, tuned on the off-screen screenshots of item 7 and of its review. The purple C++ sends is the
# feed's (CosmicPalette::Purple, pale so that text reads on the dark panels); added on top of a star it read as
# near-white light (item 7's first look). The material deepens it (DEEPEN: (0.78, 0.6, 1) becomes about
# (1.03, 0.31, 1.91)) and scales it by TINT_LEVEL to about the light of a dimmed star of the sight (CosmicMapLook::
# StarParamsOf, brightness 0.3), then tints the core toward it: the mark is a hue, not a glow. The amount of the tint
# is at most 1: a held system keeps HELD_TINT of it between the slow pulses of the pressure, a claim pulses CLAIM_GAIN
# of it every second.
DEEPEN = 3.0
TINT_LEVEL = 0.3
PRESSURE_GAIN = 0.5
HELD_TINT = 0.6
CLAIM_GAIN = 0.8
SKY_DIM = 0.4


def material(name):
    """The material asset, refilled in place when it exists (every old expression deleted - see the head comment)."""
    assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    path = "%s/%s" % (FOLDER, name)
    if assets.does_asset_exist(path):
        mat = assets.load_asset(path)
        for expr in MEL.get_material_expressions(mat):
            MEL.delete_material_expression(mat, expr)
        left = MEL.get_num_material_expressions(mat)
        if left:
            raise RuntimeError("%s keeps %d old expressions" % (name, left))
        return mat
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    return tools.create_asset(name, FOLDER, unreal.Material, unreal.MaterialFactoryNew())


class Graph:
    def __init__(self, mat):
        self.mat = mat
        self.y = 0
        self.time = None

    def node(self, cls, x=-600, **props):
        expr = MEL.create_material_expression(self.mat, cls, x, self.y)
        self.y += 120
        for key, value in props.items():
            expr.set_editor_property(key, value)
        return expr

    def connect(self, source, target, pin, what):
        if not MEL.connect_material_expressions(source, "", target, pin):
            raise RuntimeError("could not connect %s" % what)

    def vector(self, name, rgb):
        return self.node(unreal.MaterialExpressionVectorParameter, parameter_name=name, default_value=unreal.LinearColor(rgb[0], rgb[1], rgb[2], 1.0))

    def scalar(self, name, value):
        return self.node(unreal.MaterialExpressionScalarParameter, parameter_name=name, default_value=value)

    def constant3(self, rgb):
        return self.node(unreal.MaterialExpressionConstant3Vector, constant=unreal.LinearColor(rgb[0], rgb[1], rgb[2], 1.0))

    def binary(self, cls, a, b=None, const_b=None, x=-300):
        expr = self.node(cls, x)
        self.connect(a, expr, "A", "A of %s" % cls)
        if b is not None:
            self.connect(b, expr, "B", "B of %s" % cls)
        elif const_b is not None:
            expr.set_editor_property("const_b", const_b)
        return expr

    def mul(self, a, b=None, const_b=None):
        return self.binary(unreal.MaterialExpressionMultiply, a, b, const_b)

    def add(self, a, b=None, const_b=None):
        return self.binary(unreal.MaterialExpressionAdd, a, b, const_b)

    def unary(self, cls, a):
        """An expression of one input (Abs, OneMinus, Saturate): the input name "" is its first input."""
        expr = self.node(cls, x=-400)
        self.connect(a, expr, "", "the input of %s" % cls)
        return expr

    def lerp(self, a, b, alpha):
        expr = self.node(unreal.MaterialExpressionLinearInterpolate)
        for pin, source in (("A", a), ("B", b), ("Alpha", alpha)):
            self.connect(source, expr, pin, "%s of a lerp" % pin)
        return expr

    def mask(self, state, k):
        """saturate(1 - |state - k|): 1 where the scalar state is k, 0 one away and further."""
        return self.unary(unreal.MaterialExpressionSaturate,
                          self.unary(unreal.MaterialExpressionOneMinus, self.unary(unreal.MaterialExpressionAbs, self.add(state, const_b=-k))))

    def desaturate(self, colour, fraction):
        expr = self.node(unreal.MaterialExpressionDesaturation)
        self.connect(colour, expr, "", "the input of the desaturation")
        self.connect(fraction, expr, "Fraction", "the fraction of the desaturation")
        return expr

    def pulse(self, period):
        """0.5 + 0.5 sin of the material's own time (a rendering clock, never the simulation's) over period seconds."""
        if self.time is None:
            self.time = self.node(unreal.MaterialExpressionTime, x=-1100)
        sine = self.node(unreal.MaterialExpressionSine, x=-900, period=period)
        self.connect(self.time, sine, "", "the time to a sine")
        return self.add(self.mul(sine, const_b=0.5), const_b=0.5)

    def emissive(self, expr):
        if not MEL.connect_material_property(expr, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR):
            raise RuntimeError("could not connect the emissive pin")


def finish(mat, name, scalars, vectors):
    """Lays out and compiles the graph, checks its parameters, and only then saves it (a graph that fails a check is
    never written: the asset on disk keeps its last verified graph)."""
    MEL.layout_material_expressions(mat)
    MEL.recompile_material(mat)
    found_scalars = [str(n) for n in MEL.get_scalar_parameter_names(mat)]
    found_vectors = [str(n) for n in MEL.get_vector_parameter_names(mat)]
    missing = [n for n in scalars if n not in found_scalars] + [n for n in vectors if n not in found_vectors]
    if missing:
        raise RuntimeError("%s lacks the parameters %s" % (name, ", ".join(missing)))
    extra = [n for n in found_scalars if n not in scalars] + [n for n in found_vectors if n not in vectors]
    if extra:
        raise RuntimeError("%s has parameters the script does not draw: %s" % (name, ", ".join(extra)))
    # One node per name: a second node of a name (a stale one) could answer for the default instead of the drawn one.
    nodes = [str(expr.get_editor_property("parameter_name")) for expr in MEL.get_material_expressions(mat)
             if isinstance(expr, (unreal.MaterialExpressionScalarParameter, unreal.MaterialExpressionVectorParameter))]
    twice = sorted({n for n in nodes if nodes.count(n) > 1})
    if twice or len(nodes) != len(scalars) + len(vectors):
        raise RuntimeError("%s holds %d parameter nodes for %d names (twice: %s)" % (name, len(nodes), len(scalars) + len(vectors), ", ".join(twice)))
    assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    if not assets.save_loaded_asset(mat, False):
        raise RuntimeError("could not save %s" % name)
    unreal.log("[make_materials] %s written and verified" % name)


def make_star_material():
    mat = material("M_Star")
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    g = Graph(mat)
    core = g.mul(g.vector("StarColor", (1.0, 1.0, 1.0)), g.scalar("Brightness", 1.3))
    pulse = g.pulse(1.0)
    hazard = g.mul(g.mul(g.mul(g.vector("WarningColor", WARNING_ORANGE), g.scalar("HazardAmount", 0.0)), pulse), const_b=3.0)
    # The spiritual sight (CosmicMapLook::StarParamsOf): the star's hue turns toward the fallen's purple - slowly with
    # the pressure, held where it holds, with the warning's rate where it claims. A lerp of the core, not light added
    # to it: the mark never glows white. ViewMode 0 keeps the core exactly.
    claim_state = g.scalar("ClaimState", 0.0)
    claimed = g.mask(claim_state, 1.0)
    held = g.mask(claim_state, 2.0)
    pressing = g.mul(g.mul(g.scalar("Pressure", 0.0), g.pulse(3.0)), const_b=PRESSURE_GAIN)
    holding = g.mul(held, const_b=HELD_TINT)
    claiming = g.mul(g.mul(claimed, pulse), const_b=CLAIM_GAIN)
    amount = g.unary(unreal.MaterialExpressionSaturate, g.add(g.add(pressing, holding), claiming))
    deep = g.desaturate(g.vector("DominionColor", (0.0, 0.0, 0.0)), g.node(unreal.MaterialExpressionConstant, r=-DEEPEN))
    tint = g.mul(deep, const_b=TINT_LEVEL)
    marked = g.lerp(core, tint, g.mul(g.scalar("ViewMode", 0.0), amount))
    g.emissive(g.add(marked, hazard))
    finish(mat, "M_Star", ["Brightness", "HazardAmount", "ViewMode", "Pressure", "ClaimState"], ["StarColor", "WarningColor", "DominionColor"])


def make_nebula_material():
    mat = material("M_Nebula")
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("two_sided", True)
    g = Graph(mat)
    noise = g.node(unreal.MaterialExpressionNoise, x=-900, scale=0.00003, levels=4, output_min=0.0, output_max=1.0)
    nebula = g.lerp(g.constant3((0.0015, 0.001, 0.004)), g.constant3((0.018, 0.007, 0.03)), g.mul(noise, noise))
    # The spiritual view: the sky loses its violet and darkens (one more desaturation and one multiply, the one noise).
    intensity = g.scalar("SpiritualIntensity", 0.0)
    quiet = g.desaturate(nebula, intensity)
    g.emissive(g.mul(quiet, g.unary(unreal.MaterialExpressionOneMinus, g.mul(intensity, const_b=SKY_DIM))))
    finish(mat, "M_Nebula", ["SpiritualIntensity"], [])


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
