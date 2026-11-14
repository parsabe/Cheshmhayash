# Manual steps

Everything in this game is built from code and scripts. This is the complete list of things a
human has to do by hand. It is intentionally short.

## To play / test

1. Close the Unreal Editor if it is open (it locks the game code while it runs).
2. Double-click `Play.bat` in the project folder.

`Play.bat -chscene=2.6` jumps straight to a scene (scene ids are the `"id"` values in
`Content/Story/*.json`). While developing, `PageDown` skips the current scene.

## Only if something is missing

| Symptom | Fix (one command, editor closed) |
| --- | --- |
| "Map ... is missing" notice, grey untextured world, or square boxes instead of Farsi text | `Scripts\setup_assets.bat` |
| Code changed and the game does not reflect it | `Scripts\build.bat` |

`Scripts\setup_assets.bat` creates the empty maps (`/Game/Maps`), the master materials
(`/Game/Materials`), the Vazirmatn font assets (`/Game/FontAssets`) and imports the audio
(`/Game/Audio`). It is safe to run again at any time.

## Font (already done)

The Persian font **Vazirmatn** (SIL Open Font License) is already in `Content/Fonts/`
(`Vazirmatn-Regular.ttf`, `-Bold.ttf`, `-Light.ttf`, plus `Vazirmatn-OFL.txt`).
If those files are ever lost: download them from https://github.com/rastikerdar/vazirmatn
(folder `fonts/ttf`) into `Content/Fonts/`, then run `Scripts\setup_assets.bat`.

## Audio

The game already has sound: every track and effect is synthesised by
`Scripts\gen_audio_procedural.py` (no recordings, no voices) and is imported in `/Game/Audio`.

Optional, for richer music: generate the five music tracks with fal, then re-import.

```
set FAL_KEY=your-key-here
Scripts\gen_audio.bat fal
Scripts\setup_assets.bat
```

The fal tracks land in `RawAudio\fal\` and replace the procedural track of the same name.
Prompts and lengths are in `Scripts\audio_list.json`; the model ids are at the top of
`Scripts\gen_audio_fal.py`. To regenerate the procedural audio: `Scripts\gen_audio.bat`.
Sound effects from WeLoveIndies are optional: drop `.wav` files named like the ones in
`RawAudio\` into `RawAudio\WeLoveIndies\` and re-run `Scripts\setup_assets.bat`
(and add the credit line in `Content\Story\ending.json`).
