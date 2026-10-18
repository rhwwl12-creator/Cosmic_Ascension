"""Create or refill the materials of the galaxy map: /Game/CA/Materials/M_Star, M_Nebula, M_AscensionGlow, M_Backdrop.

Cosmic Ascension - Phase 3 helper (guideline.md section 6.4, 11 Phase 3). The materials are generated, like the data
tables: change this script and run it again (an edit made in the editor is overwritten). An existing asset is refilled in
place (its expressions are deleted and drawn again), never deleted and made anew.

Run headlessly with the editor CLOSED and the editor target already built:

  "C:\\Program Files\\Epic Games\\UE_5.8\\Engine\\Binaries\\Win64\\UnrealEditor-Cmd.exe" "<PROJECT_ROOT>\\Cosmic_Ascension.uproject"
      -run=pythonscript -Script="<PROJECT_ROOT>/Tools/make_materials.py"
      -unattended -nosplash -stdout -FullStdOutLogOutput

The log ends with "[make_materials] M_Star written and verified" and the same line for M_Nebula, M_AscensionGlow and
M_Backdrop.

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
  - Material usages: UMaterial::bUsedWithInstancedStaticMeshes is UE_DEPRECATED(5.8); MaterialEditingLibrary's
    SetBaseMaterialUsage and HasMaterialUsage are the calls. A material without a usage is fixed only in memory by the
    editor proper (UMaterial::SetMaterialUsage, GIsEditor and not a game): under -game it draws with the engine's default
    material instead, and a cooked build needs the flag in the asset - so finish checks every usage before it saves.
  - MaterialExpressionPerInstanceCustomData has DataIndex and ConstDefaultValue (used when no instance gives a value).
  - MaterialExpressionPixelDepth has no inputs: the pixel's depth along the view axis in world units
    (FMaterialCompiler::PixelDepth) - M_Backdrop's near fade reads it.
  - MaterialExpressionNoise (Phase 3 item 9): with no Position input it reads the absolute world position
    (UMaterialExpressionNoise::Compile; WorldPositionOriginType defaults to Absolute). With the CameraRelative origin
    FHLSLMaterialTranslator::Noise turns the input from translated world back to world, so a camera-relative direction
    is built by hand and connected with the Absolute origin. The Position pin is named after the origin
    (UMaterialExpressionNoise::GetInputName - UE::MaterialTranslatorUtils::GetWorldPositionInputName): "World Position"
    for Absolute; the input name "" (input 0) reaches it too.
  - MaterialExpressionWorldPosition's world_position_shader_offset takes unreal.WorldPositionIncludedOffsets
    (WPT_CAMERA_RELATIVE: the pixel's position relative to the camera); MaterialExpressionNormalize has one input,
    VectorInput, reached by the input name "".

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
M_Nebula (unlit, two-sided, on the inside of the sky sphere): a dark deep blue with noise - the vector parameters
NebulaLow and NebulaHigh, defaults the C++ never sets, held below the fallen's band of hues (Phase 3 item 9: the purple of
the sight is the fallen's alone, guideline 16 #98 (4) and #113); SpiritualIntensity (0 physical, 1 spiritual - the map's
ACosmicGalaxyMapActor, times 1 - its universe share) desaturates it and dims it by SKY_DIM. At 0 the output is the plain
nebula exactly. The noise reads the view's direction (the camera-relative world position, normalized, x
NEBULA_FREQUENCY): the pattern depends on where the camera looks, not where it is or how large the sphere is - a sky at
infinity that stays still while the universe level's galaxies shrink (the map sizes the sphere to hold the universe).
M_AscensionGlow (user interface domain, translucent; parameters set by UCosmicAscensionOverlayWidget): a round glow of
GlowColor whose opacity falls from the centre of the image to its edge, times Glow (0..1, the fade of the presentation).
M_Backdrop (unlit, additive, two-sided, used with instanced static meshes; the backdrop of ACosmicGalaxyMapActor - one
instance of the engine plane per point of the galaxy's dust and one per system, section 6.4 - and, through its own
instance, the universe level's scenery and named galaxies, Phase 3 item 9): emissive = Tint (a vector parameter whose
default is BACKDROP_TINT; the C++ never sets it) x LayerFade (the share of its light a layer shows: the map writes 1 - u
on the dust and u on the universe; default 1) x POINT_GAIN x the instance's glow (per-instance custom data 0 - CosmicMapLook::BackdropLookOf; 0 for a known system's
slot, which then draws nothing) x a round falloff on the plane x a near fade by the pixel's depth (none from
NEAR_FADE_START on, all by NEAR_FADE_END: a point close to the camera fades out, so a star seen from near stands clear of
the dust) x (1 - SPIRIT_DIM x SpiritualIntensity) (the map gives it the sky's value: in the spiritual view the physical
layer recedes, the dust with it). Additive writes no depth: the dust never hides a lane or a ring, and needs no sorting.
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
# The backdrop (item 8), tuned on the off-screen screenshots: one pale tint for every point (no new colour - section 6.4),
# the gain on the instance's glow (0.1..0.45, mostly faint), the near fade in world units of pixel depth (the overview
# is about 17,000 away, a focused star 3,000), and how far the dust recedes in the spiritual view.
BACKDROP_TINT = (0.75, 0.8, 1.0)
POINT_GAIN = 1.0
NEAR_FADE_START = 6000.0
NEAR_FADE_END = 2000.0
SPIRIT_DIM = 0.6
# The sky (item 9): its noise reads the view's direction x this - 150,000 x 0.00003, the coordinates today's sphere gave a
# camera at its centre - and its two colours, a deep blue below the fallen's band of hues (about 230 degrees).
NEBULA_FREQUENCY = 4.5
NEBULA_LOW = (0.001, 0.0015, 0.004)
NEBULA_HIGH = (0.008, 0.012, 0.03)


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


def finish(mat, name, scalars, vectors, usages=()):
    """Lays out and compiles the graph, checks its parameters and its usages, and only then saves it (a graph that fails
    a check is never written: the asset on disk keeps its last verified graph)."""
    MEL.layout_material_expressions(mat)
    MEL.recompile_material(mat)
    # A usage the asset lacks is set by the editor proper in memory only; under -game the material would draw with the
    # engine's default instead (see the head comment).
    lacking = [str(usage) for usage in usages if not MEL.has_material_usage(mat, usage)]
    if lacking:
        raise RuntimeError("%s lacks the usages %s" % (name, ", ".join(lacking)))
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
    # The view's direction: the camera-relative world position, normalized - a sky at infinity (see the head comment).
    position = g.node(unreal.MaterialExpressionWorldPosition, x=-1300,
                      world_position_shader_offset=unreal.WorldPositionIncludedOffsets.WPT_CAMERA_RELATIVE)
    direction = g.unary(unreal.MaterialExpressionNormalize, position)
    noise = g.node(unreal.MaterialExpressionNoise, x=-900, scale=1.0, levels=4, output_min=0.0, output_max=1.0)
    if not MEL.connect_material_expressions(g.mul(direction, const_b=NEBULA_FREQUENCY), "", noise, "World Position"):
        raise RuntimeError("could not connect the direction to the noise")
    nebula = g.lerp(g.vector("NebulaLow", NEBULA_LOW), g.vector("NebulaHigh", NEBULA_HIGH), g.mul(noise, noise))
    # The spiritual view: the sky loses its colour and darkens (one more desaturation and one multiply, the one noise).
    intensity = g.scalar("SpiritualIntensity", 0.0)
    quiet = g.desaturate(nebula, intensity)
    g.emissive(g.mul(quiet, g.unary(unreal.MaterialExpressionOneMinus, g.mul(intensity, const_b=SKY_DIM))))
    finish(mat, "M_Nebula", ["SpiritualIntensity"], ["NebulaLow", "NebulaHigh"])


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


def make_backdrop_material():
    mat = material("M_Backdrop")
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_ADDITIVE)
    mat.set_editor_property("two_sided", True)
    instanced = unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES
    MEL.set_base_material_usage(mat, instanced, True)
    g = Graph(mat)
    # The instance's glow (CosmicMapLook::BackdropLookOf): a point's, an unknown system's slot's, or 0.
    glow = g.node(unreal.MaterialExpressionPerInstanceCustomData, x=-1100, data_index=0, const_default_value=0.0)
    # A round point on the plane: 1 at the centre of its UVs, 0 at the edge, squared for a soft falloff.
    uv = g.node(unreal.MaterialExpressionTextureCoordinate, x=-1100)
    centre = g.node(unreal.MaterialExpressionConstant2Vector, x=-1100, r=0.5, g=0.5)
    distance = g.node(unreal.MaterialExpressionDistance, x=-900)
    for pin, expr in (("A", uv), ("B", centre)):
        if not MEL.connect_material_expressions(expr, "", distance, pin):
            raise RuntimeError("could not connect %s of the point's distance" % pin)
    edge = g.unary(unreal.MaterialExpressionSaturate, g.unary(unreal.MaterialExpressionOneMinus, g.mul(distance, const_b=2.0)))
    round_point = g.mul(edge, edge)
    # The near fade: saturate((depth - END) / (START - END)) - the dust clears around a star seen from near.
    depth = g.node(unreal.MaterialExpressionPixelDepth, x=-1100)
    near = g.unary(unreal.MaterialExpressionSaturate,
                   g.mul(g.add(depth, const_b=-NEAR_FADE_END), const_b=1.0 / (NEAR_FADE_START - NEAR_FADE_END)))
    # The spiritual view: the physical layer recedes, the dust with it (the sky's value).
    quiet = g.unary(unreal.MaterialExpressionOneMinus, g.mul(g.scalar("SpiritualIntensity", 0.0), const_b=SPIRIT_DIM))
    # Item 9: the tint a parameter (its default the one colour), and LayerFade - the share of its light a layer shows.
    tint = g.mul(g.vector("Tint", BACKDROP_TINT), g.scalar("LayerFade", 1.0))
    light = g.mul(g.mul(g.mul(tint, const_b=POINT_GAIN), glow), round_point)
    g.emissive(g.mul(g.mul(light, near), quiet))
    finish(mat, "M_Backdrop", ["SpiritualIntensity", "LayerFade"], ["Tint"], usages=[instanced])


make_star_material()
make_nebula_material()
make_ascension_glow_material()
make_backdrop_material()
