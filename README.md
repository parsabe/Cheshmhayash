# چشم‌هایش · Cheshm Hayash: website

The promotional site for the game. Plain HTML, CSS and JavaScript: no framework, no build step,
no tracking, no cookies. Open `index.html` directly, or host the folder anywhere static.

The game itself lives on the `game` branch; the version submitted to the Epic AI MegaJam 2026 is
the tag `jam-submission`.

## Before publishing, replace two placeholders

- **Trailer:** in `index.html`, `data-youtube="YOUTUBE_VIDEO_ID"` → the YouTube video id
  (the part after `watch?v=`). The video only loads after a click, from `youtube-nocookie.com`.
- **itch.io:** in `index.html`, `https://YOUR-NAME.itch.io/cheshm-hayash` → the real page.

## Files

- `index.html`: all text, in Farsi and English (`.t-fa` / `.t-en`)
- `style.css`: look and animations; `prefers-reduced-motion` turns the motion off
- `main.js`: language toggle, REC timer, scroll scenes (IntersectionObserver + requestAnimationFrame)
- `assets/img`: screenshots rendered from the game
- `assets/fonts`: Vazirmatn (SIL Open Font License, see `Vazirmatn-OFL.txt`)

`?lang=fa` or `?lang=en` forces a language; otherwise it follows the browser.
