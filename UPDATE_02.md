# UPDATE 02: big revision after first playtest

The developer played the first version and loved it. Below is everything he wants changed or added.
Read this file fully, together with CLAUDE.md and STORY.md. This update overrides older instructions where they conflict.

## TIME (read first)
- Deadline: **Mon 16 Nov 2026, 02:59 Berlin time.** It is Sunday morning now.
- Work **strictly in tier order** below. After **each tier**: compile, playtest-check, commit, and **run `Scripts/package.bat`** so a working packaged build always exists.
- **If no packaged build exists yet, package one right now before anything else.**
- Never leave the project broken. If a feature is not working after a reasonable attempt, cut it and move on.
- Stop starting new Tier 3/4 work after **Sunday 21:00 Berlin**: from then on only fix, polish, package.

---

## TIER 1: must have

### 1.1 Game start flow, menus, loading
Boot order:
1. Splash: "Created by <developer name>" and "Made with Unreal Engine" (ask the developer for the name if you don't have it).
2. Loading screen (dark, small animated loading indicator, a quiet drone).
3. **Main menu**: title **چشم‌هایش / Cheshm Hayash**, dark background (a slow shot of the night city or the sunrise rooftop), "Click or press any key to continue".
4. Menu buttons: **Continue** (only if a save exists), **New Game**, **Settings** (mouse sensitivity, invert Y, volume), **Quit**.
- **New Game** plays, in order: the no-voices card → the **poem card (1.3)** → the title scene (چشم‌هایش alone on screen) → Chapter 1.
- **Continue** skips all intro cards and loads the last checkpoint.
- Pause menu (Esc): Resume, Settings, Main Menu, Quit.

### 1.2 Checkpoints and saving (like The Last of Us)
- `USaveGame` with chapter, scene, beat index, all choice flags, filmed clips, molotov count.
- Autosave at the start of every scene and right before every gameplay segment / fight.
- Show a small "Saving…" indicator in the bottom-right corner when saving.
- On being caught or failing a fight, reload the last checkpoint (fade to black, not a hard cut).

### 1.3 Poem card (New Game only, right after the no-voices card)
Show the lines **one by one at the bottom of the screen**, Farsi line with the English line directly under it, slow fade-in, each line held long enough to read, over black or a very dark slow image. Use exactly this text:

در تمامیِ محاکمه‌هایی که مو حاکم بود
In every trial where hair was the judge,

صورت‌های دلربا، گیس‌هاشان کنده شد
lovely faces had their braids torn away.

قبرهای سرد و گلی برای سروهای تنومند
Cold, muddy graves for tall, strong cypresses,

بعد از شنیدن صدای بیگانه‌های کروکودیل‌صفت
after the voices of crocodile-hearted strangers

که همه‌جا را تاریک کردند و یک گلوله چشم چپش را شکافت
who darkened everything, and a bullet split his left eye,

و پوکه‌اش رفت توی باارزش‌ترین دارایی موزه‌ها
and its casing became the most treasured piece in the museums.

Then the title چشم‌هایش appears **in its own separate scene**.

**Consistency:** the poem says **left eye**. Change the death sequence (STORY.md 2.6) so the **LEFT half** of the screen goes dark. Update STORY.md.

### 1.4 Subtitles and dialogue flow
- **English subtitles must be the same font size as the Farsi line.**
- **Each speaker has their own color.** Keep this consistent everywhere:
  - He: white
  - Sister: soft pink / rose
  - Older brother: light blue
  - Mom: warm yellow
  - Dad: grey (he almost never speaks)
  - Kian: light green
  - Lawyer / officials / guards / morality police: red-orange
  - Strangers / crowd / other NPCs: light beige
  - System / narration cards: pale grey italic
- Show the speaker name (Farsi + English) in their color before the line.
- **Cutscenes and dialogue auto-advance.** Duration = about 2 s + 0.06 s per character (use the longer of the Farsi/English lines). The player never has to press anything to continue a non-interactive scene.
- **Enter, Space or left click skips** to the next line.
- Show a small hint at the bottom, "Enter ↵ / Click to continue", during the first dialogue scene and whenever a line is waiting on a choice.
- Choices: clickable with the mouse, plus number keys 1/2/3.

### 1.5 First person + mouse
- Switch the player to **first person**. Camera at eye height, the player's body is **not visible** (hands only if quick and clean; otherwise nothing).
- Full **mouse look** (sensitivity and invert Y from Settings), subtle head-bob when walking, slightly stronger when running.
- Filming: the phone is raised into view at the bottom of the screen while recording.
- Interaction with the world uses the mouse (look at things, click to act) and E as a fallback.

### 1.6 The ending: his name stays
At the end, after «ستم پایدار نمی‌ماند», when **به یاد حسام / In loving memory of Hesam** appears, **the name stays on screen during the whole final song and the credits**. Credits scroll beside or below it. The name is the last thing to fade, after the music ends.

### 1.7 Music: more dramatic, more frightening, every scene
The current music is not dramatic enough. **Regenerate all music** (fal) and give **every scene its own cue**. Direction:
- Dark cinematic thriller/horror score: low brass, sub-bass drones, dissonant tense strings, deep heavy drums (taiko-like), ticking/pulsing percussion, reversed swells, metallic hits.
- Persian colour inside the darkness: setar, kamancheh, ney, daf, heavily reverbed, sometimes detuned or distant.
- Stingers (short hits) for: the van, the 14:20 message, the news alert, being spotted, the shot.
- Dynamic layers in gameplay: calm / suspicion / spotted / fight. Crossfade between them.
- Keep the designed silences (news alert, 22:15 / death moment, 5 s before the name). Everywhere else, the music must carry the emotion.
- Suggested cue list (extend it): Menu theme; Poem card; Ch1 morning (fragile, warm, uneasy); Arrest video (sharp, horrifying); Run to Vozara (pulse); Vozara gate (cold drone); Family table (grief); Taxi news (sudden silence then low dread); Lawyer (hopeless strings); Rooftops (rising, chilling); Stairwell (broken setar); Ch2 university (tense hope); Siege (horror stealth); Kian call (tender, sad); Night streets (dark pulse); Fights (aggressive drums); Mom's message (fragile); Last night (epic, tragic); Sunrise (overwhelming, warm, sorrowful); Ch3 prison gate (empty); Letter (piano/setar, intimate); News (uneasy); Cemetery (sorrow resolving to light); Final song for the name and credits (the main theme in full).

---

## TIER 2: longer story, more dialogue, living characters

### 2.1 Story length: target 45–60 minutes
The story is currently too short and too abstract. **First expand STORY.md, then implement.** Keep the writing rules: short lines, no speeches, the phone as his voice, silence as a weapon. Roughly **triple the dialogue**. Every scene should have real back-and-forth conversation. Add these scenes (and more where natural):

**Chapter 1 additions**
- **The arrest, shown:** her friend's shaky phone video, which he watches on his phone (Scene 1.2). Morality police women in full black chadors (silhouettes like nuns, black from head to toe) drag her toward the green-and-white van. In dialogue they insult her: e.g. «بی‌حیا! خجالت نمی‌کشی؟» "Shameless! Have you no shame?", «خانواده نداری؟» "Don't you have a family?", «ببین با خودت چی کار کردی» "Look what you've done to yourself", «تو ون حالیت می‌کنیم» "We'll teach you in the van". She answers back defiantly. **What happens to her is never shown**: the video cuts off; later it is only mentioned in dialogue.
- More family scenes: Mom on the phone with relatives; Dad finally says one line late in the chapter; brother and he argue about whether to stay quiet.
- The sister's friend comes to the house and tells the story again in person, crying.
- A second visit to Vozara; trying to send clothes in; a guard takes the bag and gives no answer.
- Raising bail money: selling things, calling relatives.
- A neighbour (old man) on the stairs who lost a son in 1388.
- Rooftop chant night with neighbours on other roofs.

**Chapter 2 additions**
- University: more students, a professor, a girl cutting her hair in the yard, a sit-in.
- Multiple protest nights (at least three), each with its own goal (protect a crowd, rescue a friend, reach a meeting point).
- Fight scenes with molotovs (see Tier 4).
- Hiding in a stranger's home; the family feeds him; quiet dialogue at their table.
- A longer goodbye call with Kian on the day Kian flies out.
- A last dinner at home the night before 24 Mehr, where he almost tells them, and doesn't.

**Chapter 3 additions**
- The sister's return home and the mother's reaction (no words at first).
- The چهلم (40th-day memorial): family and friends, his photo with a black ribbon (face not visible), candles.
- Sister reading the letter (already in the story), then the news scene, then the cemetery.

### 2.2 Characters act, never stand like dummies
- In non-interactive scenes, characters **perform**: sit properly on chairs, sofas and floors, smoke on the balcony, cry, hug, look at phones, hold heads in hands, pour tea. **No T-poses, no floating, no stiff idle in sitting scenes.**
- Use proper poses (e.g. `UPoseableMeshComponent` with authored bone rotations, or montages) and small idle motion so they feel alive.
- **Document gathering is automatic:** when he walks near her ID card, the long manteau and the house deed, he picks them up himself with a short gesture (camera dips, a hand reaches, the item lifts away, its line of text appears). No clicking.
- Dialogue in seated or standing scenes starts automatically when the scene starts or when he approaches.

---

## TIER 3: Tehran, people, life

### 3.1 Streets that look like Tehran
Narrow alleys (کوچه) with water channels (جوب) along the sides, plane trees (چنار), 4–6 storey concrete and brick apartment blocks, balconies with plants and laundry, satellite dishes, water tanks on roofs, rolled-down shop shutters, shop signs in Persian (نانوایی، بقالی، سوپرمارکت، داروخانه، کفاشی), overhead electric cables, sodium-orange street lamps, graffiti (زن زندگی آزادی، مرگ بر دیکتاتور), Milad Tower and Damavand on the skyline.

### 3.2 Living city
- **Cars driving** along routes with headlights (boxy older sedans, taxis, motorbikes). **Pedestrians walking** routes.
- **Sounds for all of it**, 3D-positioned with attenuation: engines, horns, motorbikes, footsteps, distant crowd murmur. Generate with fal and procedurally.

### 3.3 Characters that look like people
- More varied than the stock mannequin: different heights and proportions, clothing silhouettes (coats, jackets, headscarves, chadors, backpacks) built from attached meshes, varied colours.
- **Faces are never clear**: shadow, blur, or no facial features. This is intentional.
- **The sister is clearly a young, beautiful woman with long, beautiful dark hair** (hair mesh / hair cards; if available, generate with the Meshy or Tripo API using the developer's jam credits via script, then import with UE Python).
- Every NPC in the streets should look different from the next.

---

## TIER 4: fights (molotovs)

### 4.1 Molotov throwing
- He **has no guns**. Only molotovs.
- First person, mouse aim: **hold right mouse** to show the throw arc and landing point, **left click** to throw. Limited count, refilled when friends hand him more (dialogue moment) or from a crate.
- On impact: fire area that burns for some seconds. Regime forces in it **retreat, scatter, or fall back**; the fire blocks their path. **No gore.** Do not show how molotovs are made.

### 4.2 Fight scenes
- Chapter 2, protest night: block a street with fire to protect the crowd from advancing riot forces.
- Chapter 2, last night (24 Mehr): hold back the motorbikes before the final run to the rooftop.
- Each fight has a checkpoint before it.

---

## Credits
Use the developer's name (ask if unknown). Remove any mention of Yas. Credit: Unreal Engine, AI agents (Claude Code), fal (music and sound), Vazirmatn font (SIL OFL), and Meshy or Tripo if used.
