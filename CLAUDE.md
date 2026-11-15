# Cheshm Hayash (چشم‌هایش) — Epic AI MegaJam 2026 entry

You are building a complete, playable narrative stealth game in **Unreal Engine 5, C++ only**, for a game jam.
The full story, scene by scene, is in `STORY.md`. Read it fully before writing code. It is the source of truth.

## Hard constraints
- **Deadline:** packaged Windows build must be uploaded by **Mon 16 Nov 2026, 02:59 Berlin time**. Aim to be done by Sunday 23:00.
- **Engine:** UE5 C++ project named `Cheshmhayash` (created from the Third Person **C++** template). Module name `Cheshmhayash`, API macro `CHESHMHAYASH_API`. Check the actual folder/module name in `Source/` and use exactly that.
- **Code only.** The developer does not want to work in the editor. Do NOT rely on Blueprints or hand-placed actors.
  - Build every level procedurally in C++ at runtime (spawn actors, use `/Engine/BasicShapes/Cube`, `/Engine/BasicShapes/Plane`, `/Engine/BasicShapes/Cylinder`, engine materials, lights, fog).
  - Build all UI in C++ (Slate or UMG constructed in C++). No `.uasset` widget blueprints.
  - Where an asset must be imported (fonts, audio), write a **UE Python editor script** in `Scripts/` that does the import, and tell the developer the one command to run it.
  - If something truly cannot be done in code, write the exact manual step in `MANUAL_STEPS.md` (keep this list as short as possible).
- **Keep it compiling.** After every feature, build. Never leave the project broken. Small classes, small steps.
- **Target platform:** Windows 64-bit, Development build.

## Visuals — 100% done by you, in code (the developer will NOT touch the editor viewport)
You are also the art director. Everything the player sees is created from code or scripts:
- **Maps:** write `Scripts/create_maps.py` (UE Python) to create one empty map per location, and set the startup/default maps in `Config/DefaultEngine.ini`. Do not use or edit the template's `Lvl_ThirdPerson` by hand; replace it.
- **Geometry:** build streets, buildings, alleys, rooftops, rooms, the prison gate, cemetery and tombstone procedurally in C++ from engine basic shapes (and `UProceduralMeshComponent` where useful). Tehran look: dense 4–6 storey concrete blocks, balconies, satellite dishes on roofs, shop shutters, power cables, water tanks on rooftops, Damavand silhouette on the horizon.
- **Materials:** create `UMaterialInstanceDynamic` from engine base materials (e.g. `/Engine/BasicShapes/BasicShapeMaterial`) and set colors/roughness in C++. If a custom master material is needed, create it with a UE Python script (`Scripts/create_materials.py`), not by hand.
- **Lighting & mood in code:** directional light, sky atmosphere, sky light, exponential height fog, point/spot lights (sodium street lamps, fires, flashlights), post-process volume (film grain, vignette, color grading, bloom, chromatic aberration) — all spawned and configured in C++.
- **Art direction:** night scenes = near-black with deep red/orange fire light and a cold blue sky edge, heavy fog, film grain, phone-camera feel. Ch1 interiors = warm, dim, lived-in. Ch3 = soft, pale morning light. The sunrise must be the brightest, warmest moment in the whole game.
- **Characters:** use the template's mannequin mesh for the player and NPCs; differentiate with dynamic material tints (security forces dark, morality police black chador-like shape via scaled/attached basic shapes, civilians varied). Silhouettes > detail.
- **Effects in code:** smoke/fire via Niagara systems created by script or engine defaults; flashlight cones; the right-half-screen darkness and REC dot via UI/post-process.
- The developer should only ever need to: run one Python script command when asked, press Play, and report what they see.

## Content rules (important for jam eligibility and respect)
- The main character is **never named** in game. He is only "he" / «او». His name (حسام · Hesam) appears ONLY on the final card.
- **No voices at all.** All dialogue is text: Farsi line on top, English line below, typed out letter by letter with a soft typing sound.
- Opening card explains: "There are no voices in this game. The people who lived this story cannot speak freely. Their words are written here instead." (Farsi + English, see STORY.md)
- **No gore.** The death is shown only as: sound cut, RIGHT half of screen goes dark (his right eye), phone falls, REC dot keeps blinking. Then the sunrise shot.
- The Supreme Leader is **never named and never shown**, and the game never announces his death (developer, 15 Nov). The TV in 3.3 (Esfand 1404) shows his own sunrise video instead.
- Mahsa Amini appears only as a news alert on the phone, never as a character.
- No real-world copyrighted music, no real protest recordings, no assets from other games/films.

## Persian text
- Farsi needs a font with Persian glyphs. Use **Vazirmatn** (SIL Open Font License). Developer downloads `Vazirmatn-Regular.ttf` into `Content/Fonts/` (add to MANUAL_STEPS.md); write `Scripts/import_font.py` to create the font asset; load it in C++.
- UE shapes Arabic-script text via ICU/HarfBuzz; make sure text uses `ETextShapingMethod::FullShaping` and right-to-left flow direction for Farsi lines.

## Architecture (suggested)
- `Story/` data: one JSON file per chapter in `Content/Story/` (copied as non-asset content: add `+DirectoriesToAlwaysStageAsNonUFS=(Path="Story")` in `DefaultGame.ini`). Each scene = list of beats: `text` (fa/en/speaker), `phone_message`, `voice_bubble` (waveform + transcript, no audio), `choice` (sets flags), `gameplay` (start a gameplay segment), `silence` (seconds), `cut`.
- `UStoryDirector` (GameInstance subsystem): loads JSON, runs beats, stores flags (choices) and the list of filmed clips.
- `UDialogueOverlay` (Slate): typed text Farsi+English, choice buttons, fades, black cards.
- `UPhoneOverlay` (Slate): chat bubbles, typing… dots, typed-then-deleted drafts, voice-message bubble, news alert, "No service".
- `URecIndicator`: blinking red REC dot + timestamp in corner (Ch1–2 only).
- `AFilmableMoment` + `UFilmingComponent`: hold right mouse to film a glowing moment; meter fills; filming raises the player's visibility. Completed clips are saved (name + text caption) for the final "director's cut" montage.
- `AGuardCharacter` + `AGuardAIController`: patrol between points, `UAIPerceptionComponent` sight sense, vision cone, chase on detection, caught = restart segment (or use a "second chance" earned from choices).
- Variants: Basij motorcycle (fast, loud — can be a capsule moving fast with engine sound), plainclothes (looks like a civilian until close), morality-police van zone (music cuts, heartbeat).
- `ALevelBuilder` per location: Room, Vozara gate street, Lawyer office, Rooftop, University yard, Parking garage, Night streets, Final rooftop, Prison gate, Living room, Cemetery. Dark night lighting + exponential height fog hides simple geometry.
- `ASunriseController`: final rooftop and cemetery — rotate directional light from below horizon to sunrise, warm color, `SkyAtmosphere`.
- `UDirectorsCut`: final montage — replays each filmed clip as a short framed card with its caption, last one freezes on the sunrise.
- Audio manager: plays music layers per state (calm / spotted / silence / sunrise), heartbeat near vans, typing sounds.

## Audio (all generated, no recordings except one)
- `Scripts/gen_audio_fal.py`: reads `Scripts/audio_list.json` (name + prompt + duration), calls the fal API with env var `FAL_KEY`, saves `.wav` into `RawAudio/`. Pick a fal music model and a sound-effect model available on fal; make the model IDs easy to change at the top of the file.
- `Scripts/gen_audio_procedural.py`: numpy-generated heartbeat, wind, low drone, "no service" beep, radio static, typing click.
- `Scripts/import_audio.py`: UE Python editor script that imports `RawAudio/*.wav` into `/Game/Audio/`.
- Music plan: Ch1 solo setar, quiet; Ch2 daf + rising strings during protests, fast percussion when spotted, only heartbeat near vans; rooftop/sunrise slow ney + warm strings; Ch3 the Ch1 setar melody, now resolved.
- **No voice anywhere in the game, including the ending.** The final cards are text only.
- Sound effects: generate with fal and procedurally. WeLoveIndies is optional; only use files the developer drops into `RawAudio/WeLoveIndies/`.

## Build order (do it in this order; each step must end compiling and playable)
1. Boot flow: opening no-voices card → title چشم‌هایش → chapter title card → empty test room. Farsi text renders correctly.
2. Dialogue overlay + phone overlay + JSON story loader. Play Ch1 scene 1 (room, Kian call, sister, choice).
3. Guard AI + detection + restart. Filming mechanic. One test street.
4. Ch2 night streets + final rooftop + death sequence + sunrise + director's cut montage. **(This is the heart. Must exist even if everything else is cut.)**
5. Ch3: prison gate, room + letter reading, living room news, cemetery (rosewater, petals, phone, tombstone), final cards, credits.
6. Remaining Ch1 scenes (Vozara gate, table, news in taxi, lawyer, rooftops, stairwell) and Ch2 university + parking garage.
7. Audio generation + import + hookup.
8. Polish: lighting, fog, fades, menu (Start / Quit), pause.
9. Package with RunUAT (write `Scripts/package.bat`).

**Cut order if late:** university walk → court/stairwell scene → parking garage (merge into street chase). Never cut the rooftop sunrise or the cemetery.

## Credits text (end)
- Game by: (developer name — ask)
- Story: in loving memory of Hesam
- Built with Unreal Engine; code written with AI agents
- Music and sound generated with fal; sound effects from WeLoveIndies (only if used)
- Font: Vazirmatn (SIL OFL)
