"""Creates one empty map per game location in /Game/Maps.

Every level is built procedurally in C++ at runtime (ACHLevelBuilder subclasses),
so the maps only need to exist. Safe to re-run: existing maps are left alone.

Run (headless, editor closed):   Scripts\\setup_assets.bat
"""
import unreal

MAP_DIR = "/Game/Maps"

# One map per location. The C++ game mode picks the level builder from the map name.
MAPS = [
    "Boot",        # opening cards, title, menu, ending cards, credits
    "Room",        # his room (1.1, 1.9, 3.2)
    "Streets",     # the city: day run (1.2), nights of Mehr (2.4), last night (2.6)
    "Vozara",      # the gate on Vozara street (1.3)
    "Home",        # living room / dinner table (1.4, 3.3)
    "Taxi",        # the news (1.5)
    "Lawyer",      # lawyer's office (1.6)
    "Rooftop",     # home rooftop, first REC (1.7)
    "Stairwell",   # the stairwell (1.8)
    "University",  # university yard (2.1)
    "Garage",      # the siege, parking garage (2.2)
    "FinalRoof",   # rooftop at the city's edge, sunrise (2.6)
    "PrisonGate",  # prison gate (3.1)
    "Cemetery",    # cemetery (3.4)
]


def create_maps():
    eal = unreal.EditorAssetLibrary
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    if not eal.does_directory_exist(MAP_DIR):
        eal.make_directory(MAP_DIR)

    created = 0
    for name in MAPS:
        path = "{}/{}".format(MAP_DIR, name)
        if eal.does_asset_exist(path):
            unreal.log("[create_maps] exists: {}".format(path))
            continue
        world = asset_tools.create_asset(name, MAP_DIR, unreal.World, unreal.WorldFactory())
        if world is None:
            unreal.log_error("[create_maps] FAILED to create {}".format(path))
            continue
        if not eal.save_loaded_asset(world, only_if_is_dirty=False):
            unreal.log_error("[create_maps] FAILED to save {}".format(path))
            continue
        created += 1
        unreal.log("[create_maps] created: {}".format(path))

    unreal.log("[create_maps] done. created={} total={}".format(created, len(MAPS)))


if __name__ == "__main__":
    create_maps()
