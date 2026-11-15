# چشم‌هایش — Cheshm Hayash ("His Eyes")

Title meaning: (1) his eyes are the camera — the game is what his eyes saw; (2) the bullet took his eye; (3) the tombstone: "He gave his eyes so we could see the sun."

The main character is never named. Only "he" / «او». His name appears once, on the last card.

Characters: He (university student) · Sister (~19, loud, funny, fearless) · Older Brother (practical, quiet) · Kian (best friend, migration partner) · Mom (at home and voice-message bubbles) · Dad (speaks only once in the entire game, in the lawyer's office).

## Global rules of the experience
- No voices. Text only: displayed in the player's chosen language (`English`, `Deutsch`, or `فارسی`), typed letter by letter.
- The phone is his voice: messages typed and deleted, typing… dots, voice bubbles with transcripts, unsent drafts.
- Silence is a weapon: worst moments = no music, no text, a few seconds of nothing.
- Short lines. Nobody gives speeches.
- Repeating motifs: red circle on calendar (26 Mehr), glass of tea (cold → hot), setar melody, sunrise.
- REC dot (red, blinking, corner) during Ch1–2. Keeps blinking after he dies. Gone in Ch3.

## OPENING CARD
در این بازی هیچ صدایی نیست.
کسانی که این داستان را زندگی کردند، نمی‌توانند آزادانه حرف بزنند.
حرف‌هایشان اینجا نوشته شده است.

There are no voices in this game.
The people who lived this story cannot speak freely.
Their words are written here instead.

Title: چشم‌هایش

---

## CHAPTER 1 — Shahrivar — «ون سبز» "The Green Van"

### 1.1 — 22 Shahrivar 1401, morning — his room
Desk, cold glass of tea, German grammar book open on bed, wall calendar with **26 Mehr — visa appointment** circled red.
Video call with Kian:
- Kian: Six weeks. Six weeks and we're on a train.
- He: In a cold city where nobody knows us.
- Kian: Where nobody asks for our ID.
- He: …I'll miss the tea.
- Kian: They have tea there.
- He: Not this tea.
Sister knocks (going to Enghelab Street for books, headscarf slipping):
- Sister: Lend me money. I'll pay you back when I'm famous.
- He: You've been famous since you were six. Still waiting.
CHOICE (flag `walk_offer`):
- A "I'll walk you to the metro." → Sister: "I'm not a child." She leaves.
- B "Fix your scarf. They're everywhere today." → Sister: "Let them look." She leaves.
Door closes. The tea glass, still full.

### 1.2 — 14:20 — the message & Mina's video (`1.2.video`)
Phone, from her friend Mina, fast and broken:
they took her / gasht ershad / the van at the metro / they're taking her to vozara / i filmed a few seconds / i couldnt do anything / im sorry im sorry
`1.2.video`: Shaky handheld phone video from across the street (`REC` on). Two morality police women in black chadors (`morality_f`) and a male officer (`morality_m`) drag his sister toward the green-and-white `گشت ارشاد` van while shouting insults at her; she answers back until the video cuts off before the van doors close.
GAMEPLAY (`1.2.run`): he runs through the city. Breathing, footsteps.

### 1.3 — The gate on Vozara Street
Families waiting. Mothers with plastic bags of long black chadors ("bring proper clothes").
GAMEPLAY: three attempts at the gate guard, all fail:
- Beg → guard: "Tomorrow."
- Argue → guard steps closer: "Want to go inside too?"
- Lie "I'm her husband." → "Paper?" He has nothing.
Old woman: "Don't shout, son. They punish the girls for what we say outside."
He stands at the gate until dark. Slow fade.

### 1.4 — Night — the table
Untouched dinner. Dad smoking on balcony. Brother writing phone numbers.
- Brother: Tomorrow, the lawyer. I'll go to the court. Mom stays by the phone.
- He: And if they don't give her back?
- Brother: They give them back. They just want us scared first.
GAMEPLAY: collect items, each with a short line:
- ID card — "She hated this photo."
- Long manteau — "She never wore it."
- House deed — "Everything they own. One page."

### 1.5 — 25 Shahrivar — the news (taxi)
News alert: "Mahsa Amini, 22, has died after being held at the Vozara Street detention center."
Text: "Same building. Same gate."
All sound drops. Alert held 5 seconds in silence.
Taxi driver: "They say it was her heart."
He types to brother "did you see" → deletes. "she's in there" → deletes. Sends nothing.
`1.5b`: Outside Vozara again, holding a bag of her clothes and ID card. The guard gives no names; the old woman tells him they moved detainees in white vans toward Evin in the middle of the night.

### 1.6 — The lawyer
- Lawyer: Last week it was a hijab case. A fine, a signature, home.
- Lawyer: Now everything is political. They sent her file to the security court. They're afraid.
- He: What is the charge? That she shouted by the van?
- Lawyer: Resisting officers and disturbing order. They set bail high because they're afraid people won't let go.
Dad slides the house deed across the desk and speaks his only line in the game:
- Dad: Just bring her home.

### 1.7 — The rooftops
Night window. Small text at the screen edge: «مرگ بر دیکتاتور» (Death to the dictator). Then another, another — lines multiply until the screen is full. Actual sound: only wind.
He climbs to the roof and films the dark city. REC starts blinking (first time).

### 1.8 — One week later — the stairwell
Brother back from court: she's been transferred to a women's prison outside the city. No visits.
Brother on the stairs, back to camera, shoulders shaking. No text, no music. He sits beside him. Long silence. Then:
- Brother: Don't tell Mom I cried.

### 1.9 — Chapter end — his room
He takes the calendar off the wall, looks at the red circle. Folds a letter into the German grammar book.
Self-video, only his shadow:
- If they won't give her back…
- we'll take back the whole country.
- Then they'll have to.
CUT.

---

## CHAPTER 2 — Mehr — «زن، زندگی، آزادی» "Woman, Life, Freedom"

### 2.1 — The university
Red paint on stairs: ZAN · ZENDEGI · AZADI. Students in the yard, refusing class.
GAMEPLAY: walk and read; one line each:
- Professor with a box of books: "I resigned. I can't teach them to obey."
- Girl with fresh uneven short hair: "It grows back. She doesn't."
- Boy staring at phone: "Arash hasn't answered in four days."

### 2.2 — The siege (parking garage)
Security forces surround campus; students trapped in dark garage; motorcycles circling.
GAMEPLAY: stealth, lead students out one by one between flashlight beams.
If `walk_offer == A`, when going back for the last student: "I didn't walk her. I'm not leaving this one."

### 2.3 — Kian (video call, half-packed suitcases)
- Kian: The appointment is in nine days. You can still come.
- Kian: We'll be loud from outside. Lawyers. Media. Everything.
- He: And who stays?
- Kian: Millions of people. You're one guy.
- He: That's what every one guy says.
Silence; Kian's frozen face. Then:
- Kian: Just come back alive.
CHOICE (flag `window_seat`): A "Book me a window seat." / B (say nothing)

### 2.4 — The nights of Mehr (main stealth/chase level)
Night streets, smoke, broken streetlights.
Enemies: Basij motorcycles (loud, fast, heard before seen); plainclothes (civilian until too close); Gasht-e Ershad green-white vans (music cuts, only heartbeat).
Filmable moments: headscarf burning on a fire; old man lifting shop shutter for students; wall with «زن زندگی آزادی» painted over regime slogans.
Choices each night: help wounded stranger vs keep filming; hide vs draw guards away → number of second chances in final chase.
`2.4.home`: An older couple opens their door to take him in from the street; the woman pours hot tea while her husband watches the motorbikes pass from behind the curtain.

### 2.5 — 23 Mehr — Mom & Last Dinner at Home (`2.5b`)
Hiding in an alley. Voice bubble from "Maman", 0:47, waveform animates. Transcript line by line:
- Your brother went to the prison again today.
- They said she's alright. They said maybe soon.
- Please come home tonight.
- I can't lose two.
He types "I'm coming home" → deletes. Types «دوستت دارم مامان» (I love you, Mom). Sent ✓.
`2.5b`: He goes home for one quiet dinner the night before 24 Mehr. Mom puts warm rice on his plate; Brother asks why his shoes smell like smoke every night. He almost tells them, and doesn't.

### 2.6 — 24 Mehr — the last night
Biggest street yet; singing; text slogans fill the crowd. For a few minutes, winning.
Then: shots, smoke, chase. GAMEPLAY: longest run — alleys, walls, rooftops — until just before dawn.
Rooftop at the city's edge facing east toward the mountains. He sits, films the orange sky:
- نگاه کن… داره میاد بالا / Look… it's coming up.
DEATH (no gore): sound cuts; LEFT half of screen goes dark (his left eye, as in the opening poem) (one eye closing); phone falls on its side, image tilts; REC keeps blinking.
LAST SHOT: he lies facing east, wounded side in shadow. Sun rises over Damavand; light crosses rooftops, reaches his face, stays. Long hold — wind, one bird. Then Ch1 setar melody, very soft.
If `window_seat == A`: on the fallen phone, from Kian: "Window seat booked. Next time, right?" — unread.
CUT TO: DIRECTOR'S CUT montage — every clip the player filmed, in order, Shahrivar → that morning. Last clip: sunrise, frozen.

---

## CHAPTER 3 — After — «چشم‌هایش» "His Eyes"
No REC dot. He is not filming anymore.

### 3.1 — Aban 1401 — the prison gate
Gate opens. Sister walks out — thin, hair short, same eyes. Older brother waiting alone.
- Sister: Where is he?
- Sister: Is he angry I didn't let him walk me?
- Sister: Tell him I'm sorry.
Brother opens his mouth. No text appears. Screen slowly to white.
`3.1b`: Sister walks into the apartment. Mom holds her without speaking, then tells her they haven't closed his door or touched a thing since that night.

### 3.2 — His room & The چهلم (40th-Day Memorial, `3.2b`)
Nothing moved. Cold glass of tea. Calendar on the floor, red circle on 26 Mehr — two days after.
She opens the grammar book; the letter falls. GAMEPLAY: player reads it line by line:
- If you're reading this, you're home. I knew they'd have to give you back.
- I'm sorry I didn't walk you to the metro. I'll be sorry about that forever.
- I'm not angry. I was never angry. You were right not to fix your scarf.
- Don't let them make you small again. Not for me, not for Mom, not for anyone.
- Drink the tea while it's hot. I never did.
- — your brother
`3.2b`: Forty days later (`چهلم`). The apartment is full of family, friends, and students in black; his framed photo with a black ribbon sits beside a red flower. Mina tells her that people everywhere are seeing through his eyes.

### 3.3 — Years later — the news
Small living room. Brother and sister, older, drinking tea — hot.
TV black-bordered: «رهبر درگذشت.» / The Leader has died. (No name, no face, no portrait.)
They don't cheer. She smiles, tired, sad — then cries. He puts his arm around her.
- Sister: He should have seen this.
- Brother: He saw the sun first.

### 3.4 — The cemetery
Early morning; sun rising exactly like the rooftop. Rows of graves, many say 1401.
GAMEPLAY: 1) pour rosewater on the stone and wipe it; 2) scatter petals; 3) place his phone on the stone → last clip plays: orange sky, "Look… it's coming up."
- Sister: Rest now. We'll take it from here.

TOMBSTONE (name hidden under rose petals):
- engraved sun rising over mountains
- [petals over the name]
- دانشجو · برادر · رفیق (Student · Brother · Friend)
- او چشم‌هایش را داد / تا ما خورشید را ببینیم (He gave his eyes so we could see the sun)
- ۲۴ مهر ۱۴۰۱
- زن · زندگی · آزادی

---

## ENDING
No voice. Text only, on black, each line fading in slowly.

Card 1 (fades in, holds ~6 s, fades out):
ستم پایدار نمی‌ماند
Oppression does not last.

Black. 5 seconds of complete silence (no music, no wind).

Card 2 (fades in letter by letter, slower than any text before):
به یاد حسام
In loving memory of Hesam

Hold. His name stays on screen alone for a few seconds — the first and only time it appears.
Then the Ch1 setar melody returns softly. Credits.
