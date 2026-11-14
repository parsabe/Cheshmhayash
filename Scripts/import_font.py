"""Imports Content/Fonts/Vazirmatn-*.ttf as Font Face assets into /Game/FontAssets.

The C++ side (CHFonts.cpp) builds the composite font from these Font Face assets.
If they are missing it falls back to loading the raw .ttf files from Content/Fonts,
and if those are missing too, to the engine font (which has an Arabic-script fallback).

Vazirmatn is licensed under the SIL Open Font License (see Content/Fonts/Vazirmatn-OFL.txt).

Run (headless, editor closed):   Scripts\\setup_assets.bat
"""
import os
import unreal

FONT_SRC_DIR = os.path.join(unreal.Paths.project_content_dir(), "Fonts")
FONT_DST_DIR = "/Game/FontAssets"
WEIGHTS = ["Regular", "Bold", "Light"]


def import_fonts():
    eal = unreal.EditorAssetLibrary
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    tasks = []
    for weight in WEIGHTS:
        src = os.path.abspath(os.path.join(FONT_SRC_DIR, "Vazirmatn-{}.ttf".format(weight)))
        if not os.path.isfile(src):
            unreal.log_warning("[import_font] missing {} (see MANUAL_STEPS.md)".format(src))
            continue
        task = unreal.AssetImportTask()
        task.filename = src
        task.destination_path = FONT_DST_DIR
        task.destination_name = "Vazirmatn_{}".format(weight)
        task.automated = True
        task.replace_existing = True
        task.save = True
        tasks.append(task)

    if not tasks:
        unreal.log_warning("[import_font] nothing to import")
        return

    asset_tools.import_asset_tasks(tasks)

    for weight in WEIGHTS:
        path = "{}/Vazirmatn_{}".format(FONT_DST_DIR, weight)
        if eal.does_asset_exist(path):
            asset = eal.load_asset(path)
            unreal.log("[import_font] ok: {} ({})".format(path, asset.get_class().get_name()))
        else:
            unreal.log_warning("[import_font] not created: {}".format(path))


if __name__ == "__main__":
    import_fonts()
