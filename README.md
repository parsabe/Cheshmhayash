<div align="center">

# چشم‌هایش · Cheshm Hayash

**His Eyes**: a narrative stealth game about Tehran, 1401.

[![Cheshm Hayash: watch the trailer](https://img.youtube.com/vi/drZl6DR5eMI/maxresdefault.jpg)](https://youtu.be/drZl6DR5eMI)

**[▶ Watch the trailer](https://youtu.be/drZl6DR5eMI)** ·
**[Website](https://parsabe.github.io/Cheshmhayash/)** ·
**[Source code](https://github.com/parsabe/Cheshmhayash/tree/game)** ·
**[Jam version](https://github.com/parsabe/Cheshmhayash/tree/jam-submission)**

*There are no voices in this game.<br>
The people who lived this story cannot speak freely.<br>
Their words are written here instead.*

در این بازی هیچ صدایی نیست.<br>
کسانی که این داستان را زندگی کردند، نمی‌توانند آزادانه حرف بزنند.<br>
حرف‌هایشان اینجا نوشته شده است.

</div>

---

## The game

He is a student. On his wall calendar one date is circled in red: a visa appointment, six weeks
from a train to a cold city where nobody asks for his ID. Then his sister goes out to buy books on
Enghelab Street, and a green van stops beside her. He picks up his phone and starts to film.

*Cheshm Hayash* is told through his phone camera, in three chapters:

| | |
|---|---|
| **Chapter One · Shahrivar**: *The Green Van* | a sister, a gate on Vozara Street, a family that waits |
| **Chapter Two · Mehr**: *Woman, Life, Freedom* | the nights the city filled with voices |
| **Chapter Three · After**: *What Remains* | what a family keeps |

- **Film what you see.** Hold the right mouse button to film. Every clip becomes part of his director's cut.
- **Stay unseen.** Plainclothes agents, flashlights, motorcycles, vans. Filming makes you easier to see.
- **No voices.** Every word is written, typed letter by letter, in فارسی, English or Deutsch.

Made for the **Epic AI MegaJam 2026**, theme **“Director's Cut”**.

### Controls

| | |
|---|---|
| Move / look | `W A S D` or arrow keys / mouse |
| Run / sneak | `Shift` / `Ctrl` or `C` |
| Film | hold **right mouse button** |
| Interact | `E` or left click |
| Continue text / choose | `Enter`, `Space` or click / `1`–`4` |
| Pause | `Esc` or `P` |

## This repository

| Branch / tag | What is in it |
|---|---|
| [`master`](https://github.com/parsabe/Cheshmhayash/tree/master) | this page |
| [`game`](https://github.com/parsabe/Cheshmhayash/tree/game) | the Unreal Engine 5 project: C++ source, story files, scripts, content |
| [`website`](https://github.com/parsabe/Cheshmhayash/tree/website) | the promotional site, published at [parsabe.github.io/Cheshmhayash](https://parsabe.github.io/Cheshmhayash/) |
| [`jam-submission`](https://github.com/parsabe/Cheshmhayash/tree/jam-submission) (tag) | the exact version submitted to the jam |

## How it was made

- **Unreal Engine 5.8, C++ only.** Every street, rooftop, room, light and wisp of fog is built by code
  at runtime; there are no hand-placed levels and no Blueprints. The UI is Slate, written in C++.
- **The story is data.** Each chapter is a JSON file of beats (text, phone messages, choices,
  gameplay, silences) in `Content/Story/`.
- **AI agents.** The code was written with AI agents (Claude Code), directed by one developer.
- **Generated sound.** Music and sound effects were generated with fal and procedurally. No recorded voices.
- **Right-to-left text** shaped inside the engine with the Vazirmatn font.

### Build it yourself (`game` branch)

1. Install Unreal Engine 5.8 and Visual Studio with the *Game development with C++* workload.
2. `git clone -b game https://github.com/parsabe/Cheshmhayash.git`
3. Open `Cheshmhayash.uproject` (it builds the C++ module), or run `Play.bat`.
4. `Scripts\package.bat` makes a Windows build in `Packaged\`; `Scripts\make_installer.bat` wraps it in an installer.
5. `Scripts\render_trailer.ps1` renders the trailer straight from the game.

## Credits

- A game by **Parsa Besharat** · [Parsabe.com](https://parsabe.com)
- Closing video and the song «باغ ۱۴۰۴» (*Baagh 1404*): **YasTunes**
- Game music and sound: generated with **fal**
- Made with **Unreal Engine 5** and AI agents (**Claude Code**)
- Font: **Vazirmatn** (SIL Open Font License)

<div align="center">

<br>

**به یاد حسام**<br>
*In loving memory of Hesam*

زن · زندگی · آزادی

</div>
