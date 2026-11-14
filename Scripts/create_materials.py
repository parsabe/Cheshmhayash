"""Creates the few master materials the game needs, in /Game/Materials.

All colours, roughness and glow are set from C++ through dynamic material instances;
these masters only expose the parameters. Safe to re-run (materials are rebuilt in place).

  M_CH_Lit         opaque lit      Color, Roughness, Metallic, Specular, Emissive, EmissiveStrength, Grime
  M_CH_Unlit       opaque unlit    Color, Strength
  M_CH_Glow        translucent     Color, Strength, Opacity, Soft   (soft radial sprites, light cones, smoke)
  M_CH_Widget      masked lit      SlateUI texture (world-space Farsi text painted on walls / engraved)
  M_CH_WidgetGlow  translucent     SlateUI texture (screens: TV, phone)

Run (headless, editor closed):   Scripts\\setup_assets.bat
"""
import unreal

MAT_DIR = "/Game/Materials"

mel = unreal.MaterialEditingLibrary
eal = unreal.EditorAssetLibrary
MP = unreal.MaterialProperty


def _get_or_create(name):
    path = "{}/{}".format(MAT_DIR, name)
    if eal.does_asset_exist(path):
        mat = eal.load_asset(path)
        mel.delete_all_material_expressions(mat)
    else:
        mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            name, MAT_DIR, unreal.Material, unreal.MaterialFactoryNew())
    return mat


def _vec(mat, name, rgba, x, y):
    e = mel.create_material_expression(mat, unreal.MaterialExpressionVectorParameter, x, y)
    e.set_editor_property("parameter_name", name)
    e.set_editor_property("default_value", unreal.LinearColor(rgba[0], rgba[1], rgba[2], rgba[3]))
    return e


def _scalar(mat, name, value, x, y):
    e = mel.create_material_expression(mat, unreal.MaterialExpressionScalarParameter, x, y)
    e.set_editor_property("parameter_name", name)
    e.set_editor_property("default_value", value)
    return e


def _const(mat, value, x, y):
    e = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, x, y)
    e.set_editor_property("r", value)
    return e


def _node(mat, cls, x, y):
    return mel.create_material_expression(mat, cls, x, y)


def _mul(mat, a, b, x, y, a_out="", b_out=""):
    e = _node(mat, unreal.MaterialExpressionMultiply, x, y)
    mel.connect_material_expressions(a, a_out, e, "A")
    mel.connect_material_expressions(b, b_out, e, "B")
    return e


def _usage(mat, usages):
    for usage in usages:
        try:
            mel.set_material_usage(mat, usage)
        except Exception as exc:  # pylint: disable=broad-except
            unreal.log_warning("[create_materials] usage {} failed: {}".format(usage, exc))


def _finish(mat):
    mel.recompile_material(mat)
    eal.save_loaded_asset(mat, only_if_is_dirty=False)
    unreal.log("[create_materials] built {}".format(mat.get_path_name()))


def _grime(mat, x, y):
    """World-space blotchy noise in 0..1 (cheap two-projection trick, no UVs needed)."""
    noise_tex = unreal.load_asset("/Engine/EngineMaterials/Good64x64TilingNoiseHighFreq")
    if noise_tex is None:
        return None
    wp = _node(mat, unreal.MaterialExpressionWorldPosition, x, y)

    def sample(scale, mask_a, mask_b, ox, oy):
        m1 = _node(mat, unreal.MaterialExpressionComponentMask, ox, oy)
        m1.set_editor_property("r", mask_a[0])
        m1.set_editor_property("g", mask_a[1])
        m1.set_editor_property("b", mask_a[2])
        m1.set_editor_property("a", False)
        mel.connect_material_expressions(wp, "", m1, "")
        uv = m1
        if mask_b is not None:
            m2 = _node(mat, unreal.MaterialExpressionComponentMask, ox, oy + 80)
            m2.set_editor_property("r", mask_b[0])
            m2.set_editor_property("g", mask_b[1])
            m2.set_editor_property("b", mask_b[2])
            m2.set_editor_property("a", False)
            mel.connect_material_expressions(wp, "", m2, "")
            add = _node(mat, unreal.MaterialExpressionAdd, ox + 120, oy)
            mel.connect_material_expressions(m1, "", add, "A")
            mel.connect_material_expressions(m2, "", add, "B")
            uv = add
        sc = _const(mat, scale, ox + 120, oy + 120)
        uvs = _mul(mat, uv, sc, ox + 240, oy)
        ts = _node(mat, unreal.MaterialExpressionTextureSample, ox + 380, oy)
        ts.set_editor_property("texture", noise_tex)
        mel.connect_material_expressions(uvs, "", ts, "UVs")
        return ts

    # floors: XY ; walls: (X or Y) + Z. Low frequencies: metre-sized blotches, not static.
    s_floor = sample(1.0 / 5200.0, (True, True, False), None, x + 150, y)
    s_wall = sample(1.0 / 4300.0, (True, False, True), (False, True, True), x + 150, y + 260)
    s_fine = sample(1.0 / 900.0, (True, False, True), (False, True, False), x + 150, y + 520)

    add1 = _node(mat, unreal.MaterialExpressionAdd, x + 700, y + 100)
    mel.connect_material_expressions(s_floor, "R", add1, "A")
    mel.connect_material_expressions(s_wall, "R", add1, "B")
    add2 = _node(mat, unreal.MaterialExpressionAdd, x + 820, y + 200)
    mel.connect_material_expressions(add1, "", add2, "A")
    mel.connect_material_expressions(s_fine, "R", add2, "B")
    third = _const(mat, 1.0 / 3.0, x + 820, y + 320)
    avg = _mul(mat, add2, third, x + 940, y + 200)
    return avg


def build_lit():
    mat = _get_or_create("M_CH_Lit")
    color = _vec(mat, "Color", (0.35, 0.35, 0.35, 1.0), -900, -200)
    rough = _scalar(mat, "Roughness", 0.9, -900, 100)
    metal = _scalar(mat, "Metallic", 0.0, -900, 200)
    spec = _scalar(mat, "Specular", 0.25, -900, 300)
    emis = _vec(mat, "Emissive", (0.0, 0.0, 0.0, 1.0), -900, 400)
    emis_s = _scalar(mat, "EmissiveStrength", 0.0, -900, 620)
    grime_amt = _scalar(mat, "Grime", 0.3, -900, -420)

    base_out = color
    try:
        noise = _grime(mat, -2200, -900)
        if noise is not None:
            # lerp(1 - Grime, 1, noise) * Color
            one = _const(mat, 1.0, -700, -520)
            inv = _node(mat, unreal.MaterialExpressionSubtract, -560, -480)
            mel.connect_material_expressions(one, "", inv, "A")
            mel.connect_material_expressions(grime_amt, "", inv, "B")
            lerp = _node(mat, unreal.MaterialExpressionLinearInterpolate, -420, -420)
            mel.connect_material_expressions(inv, "", lerp, "A")
            mel.connect_material_expressions(one, "", lerp, "B")
            mel.connect_material_expressions(noise, "", lerp, "Alpha")
            base_out = _mul(mat, color, lerp, -260, -260)
    except Exception as exc:  # pylint: disable=broad-except
        unreal.log_warning("[create_materials] grime graph skipped: {}".format(exc))
        base_out = color

    mel.connect_material_property(base_out, "", MP.MP_BASE_COLOR)
    mel.connect_material_property(rough, "", MP.MP_ROUGHNESS)
    mel.connect_material_property(metal, "", MP.MP_METALLIC)
    mel.connect_material_property(spec, "", MP.MP_SPECULAR)
    glow = _mul(mat, emis, emis_s, -600, 480)
    mel.connect_material_property(glow, "", MP.MP_EMISSIVE_COLOR)

    _usage(mat, [unreal.MaterialUsage.MATUSAGE_SKELETAL_MESH,
                 unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES])
    _finish(mat)


def build_unlit():
    mat = _get_or_create("M_CH_Unlit")
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    color = _vec(mat, "Color", (1.0, 1.0, 1.0, 1.0), -700, 0)
    strength = _scalar(mat, "Strength", 1.0, -700, 220)
    out = _mul(mat, color, strength, -400, 60)
    mel.connect_material_property(out, "", MP.MP_EMISSIVE_COLOR)
    _usage(mat, [unreal.MaterialUsage.MATUSAGE_SKELETAL_MESH,
                 unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES])
    _finish(mat)


def build_glow():
    mat = _get_or_create("M_CH_Glow")
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("two_sided", True)

    color = _vec(mat, "Color", (1.0, 1.0, 1.0, 1.0), -900, 0)
    strength = _scalar(mat, "Strength", 1.0, -900, 220)
    out = _mul(mat, color, strength, -600, 60)
    mel.connect_material_property(out, "", MP.MP_EMISSIVE_COLOR)

    # radial falloff from UV centre: saturate(1 - 2*distance(uv, 0.5)) ^ 2
    uv = _node(mat, unreal.MaterialExpressionTextureCoordinate, -1500, 420)
    centre = _node(mat, unreal.MaterialExpressionConstant2Vector, -1500, 540)
    centre.set_editor_property("r", 0.5)
    centre.set_editor_property("g", 0.5)
    dist = _node(mat, unreal.MaterialExpressionDistance, -1300, 460)
    mel.connect_material_expressions(uv, "", dist, "A")
    mel.connect_material_expressions(centre, "", dist, "B")
    two = _const(mat, 2.0, -1300, 600)
    d2 = _mul(mat, dist, two, -1150, 480)
    inv = _node(mat, unreal.MaterialExpressionOneMinus, -1020, 480)
    mel.connect_material_expressions(d2, "", inv, "")
    sat = _node(mat, unreal.MaterialExpressionSaturate, -900, 480)
    mel.connect_material_expressions(inv, "", sat, "")
    radial = _mul(mat, sat, sat, -780, 480)

    soft = _scalar(mat, "Soft", 0.0, -900, 640)
    one = _const(mat, 1.0, -780, 640)
    lerp = _node(mat, unreal.MaterialExpressionLinearInterpolate, -620, 520)
    mel.connect_material_expressions(one, "", lerp, "A")
    mel.connect_material_expressions(radial, "", lerp, "B")
    mel.connect_material_expressions(soft, "", lerp, "Alpha")

    opacity = _scalar(mat, "Opacity", 0.5, -900, 330)
    op = _mul(mat, opacity, lerp, -440, 380)
    mel.connect_material_property(op, "", MP.MP_OPACITY)

    _usage(mat, [unreal.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES])
    _finish(mat)


def _widget_common(mat):
    tex = _node(mat, unreal.MaterialExpressionTextureSampleParameter2D, -1000, 0)
    tex.set_editor_property("parameter_name", "SlateUI")
    white = unreal.load_asset("/Engine/EngineResources/WhiteSquareTexture")
    if white is not None:
        tex.set_editor_property("texture", white)
    tint = _vec(mat, "TintColorAndOpacity", (1.0, 1.0, 1.0, 1.0), -1000, 280)
    rgb = _mul(mat, tex, tint, -700, 60, "RGB", "")
    alpha = _mul(mat, tex, tint, -700, 260, "A", "A")
    return rgb, alpha


def build_widget():
    mat = _get_or_create("M_CH_Widget")
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_MASKED)
    mat.set_editor_property("two_sided", True)
    mat.set_editor_property("opacity_mask_clip_value", 0.35)
    rgb, alpha = _widget_common(mat)
    rough = _scalar(mat, "Roughness", 0.9, -700, 460)
    emis_s = _scalar(mat, "EmissiveStrength", 0.0, -700, 560)
    glow = _mul(mat, rgb, emis_s, -460, 520)
    mel.connect_material_property(rgb, "", MP.MP_BASE_COLOR)
    mel.connect_material_property(alpha, "", MP.MP_OPACITY_MASK)
    mel.connect_material_property(rough, "", MP.MP_ROUGHNESS)
    mel.connect_material_property(glow, "", MP.MP_EMISSIVE_COLOR)
    _finish(mat)


def build_widget_glow():
    mat = _get_or_create("M_CH_WidgetGlow")
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mat.set_editor_property("two_sided", True)
    rgb, alpha = _widget_common(mat)
    emis_s = _scalar(mat, "EmissiveStrength", 1.0, -700, 560)
    glow = _mul(mat, rgb, emis_s, -460, 520)
    mel.connect_material_property(glow, "", MP.MP_EMISSIVE_COLOR)
    mel.connect_material_property(alpha, "", MP.MP_OPACITY)
    _finish(mat)


def create_materials():
    if not eal.does_directory_exist(MAT_DIR):
        eal.make_directory(MAT_DIR)
    for fn in (build_lit, build_unlit, build_glow, build_widget, build_widget_glow):
        try:
            fn()
        except Exception as exc:  # pylint: disable=broad-except
            unreal.log_error("[create_materials] {} FAILED: {}".format(fn.__name__, exc))
    unreal.log("[create_materials] done")


if __name__ == "__main__":
    create_materials()
