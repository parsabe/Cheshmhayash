# چشم‌هایش · Cheshm Hayash: website

[![Cheshm Hayash: watch the trailer](https://raw.githubusercontent.com/parsabe/Cheshmhayash/master/thumbnail.jpg)](https://youtu.be/drZl6DR5eMI)

**[▶ Trailer](https://youtu.be/drZl6DR5eMI)** ·
**[Live site](https://parsabe.github.io/Cheshmhayash/)** ·
**[About the game (`master`)](https://github.com/parsabe/Cheshmhayash/tree/master)** ·
**[Source code (`game`)](https://github.com/parsabe/Cheshmhayash/tree/game)** ·
**[Jam version (`jam-submission`)](https://github.com/parsabe/Cheshmhayash/tree/jam-submission)**

The promotional site for the game. Plain HTML, CSS and JavaScript: no framework, no build step,
no tracking, no cookies. Open `index.html` directly, or host the folder anywhere static.

## One placeholder left

- **itch.io:** in `index.html`, `https://YOUR-NAME.itch.io/cheshm-hayash` → the real page.
  Until then the button reads "Coming soon to itch.io".

The trailer (`data-youtube="drZl6DR5eMI"` in `index.html`) only loads after a click, from
`youtube-nocookie.com`.

## Files

- `index.html`: all text, in Farsi and English (`.t-fa` / `.t-en`)
- `style.css`: look and animations; `prefers-reduced-motion` turns the motion off
- `main.js`: language toggle, REC timer, scroll scenes (IntersectionObserver + requestAnimationFrame)
- `assets/img`: screenshots rendered from the game
- `assets/fonts`: Vazirmatn (SIL Open Font License, see `Vazirmatn-OFL.txt`)

`?lang=fa` or `?lang=en` forces a language; otherwise it follows the browser.
