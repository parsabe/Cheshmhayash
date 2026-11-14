"""Imports RawAudio/*.wav into /Game/Audio as sound waves.

Run through Scripts\\setup_assets.bat (editor closed). Priority for a given name:
RawAudio/fal/ (generated with fal), then RawAudio/WeLoveIndies/ (only files the developer put
there), then RawAudio/ (procedural). music_*, amb_* and loop_* are set to loop.
"""
import os

import unreal

DEST = "/Game/Audio"
LOOPING_PREFIXES = ("music_", "amb_", "loop_")


def _sources():
    root = os.path.abspath(os.path.join(unreal.Paths.project_dir(), "RawAudio"))
    chosen = {}
    # later folders win
    for folder in (root, os.path.join(root, "WeLoveIndies"), os.path.join(root, "fal")):
        if not os.path.isdir(folder):
            continue
        for name in sorted(os.listdir(folder)):
            if name.lower().endswith(".wav"):
                chosen[os.path.splitext(name)[0]] = os.path.join(folder, name)
    return chosen


def import_audio():
    sources = _sources()
    if not sources:
        unreal.log_warning("[import_audio] no .wav files in RawAudio/ - run Scripts\\gen_audio.bat first")
        return
    tasks = []
    for name, path in sources.items():
        task = unreal.AssetImportTask()
        task.filename = path
        task.destination_path = DEST
        task.destination_name = name
        task.automated = True
        task.replace_existing = True
        task.save = True
        tasks.append(task)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

    count = 0
    for name in sources:
        asset_path = "{}/{}".format(DEST, name)
        wave = unreal.EditorAssetLibrary.load_asset(asset_path)
        if not wave:
            unreal.log_warning("[import_audio] failed to import {}".format(name))
            continue
        wave.set_editor_property("looping", name.startswith(LOOPING_PREFIXES))
        unreal.EditorAssetLibrary.save_asset(asset_path, only_if_is_dirty=False)
        count += 1
    unreal.log("[import_audio] imported {} sounds into {}".format(count, DEST))


if __name__ == "__main__":
    import_audio()
