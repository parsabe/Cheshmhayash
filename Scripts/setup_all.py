"""Runs every asset-creation script in one editor launch.

    Scripts\\setup_assets.bat          (editor must be closed)

Each step is independent and safe to re-run.
"""
import importlib
import os
import sys
import traceback

import unreal

SCRIPTS_DIR = os.path.abspath(os.path.join(unreal.Paths.project_dir(), "Scripts"))
if SCRIPTS_DIR not in sys.path:
    sys.path.insert(0, SCRIPTS_DIR)

STEPS = [
    ("create_maps", "create_maps"),
    ("create_materials", "create_materials"),
    ("import_font", "import_fonts"),
    ("import_audio", "import_audio"),
]


def main():
    failed = []
    for module_name, func_name in STEPS:
        if not os.path.isfile(os.path.join(SCRIPTS_DIR, module_name + ".py")):
            unreal.log("[setup_all] skip {} (script not present yet)".format(module_name))
            continue
        try:
            module = importlib.import_module(module_name)
            importlib.reload(module)
            getattr(module, func_name)()
            unreal.log("[setup_all] OK   {}".format(module_name))
        except Exception:  # pylint: disable=broad-except
            failed.append(module_name)
            unreal.log_error("[setup_all] FAIL {}\n{}".format(module_name, traceback.format_exc()))
    if failed:
        unreal.log_error("[setup_all] finished with failures: {}".format(", ".join(failed)))
    else:
        unreal.log("[setup_all] ALL DONE")


main()
