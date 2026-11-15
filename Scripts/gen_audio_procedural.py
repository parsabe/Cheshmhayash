"""Generates every sound of the game procedurally into RawAudio/*.wav.

    Scripts\\gen_audio.bat

Pure standard library (no numpy), so it runs with the Python that ships with Unreal Engine.
No recordings, no samples, no voices: plucked strings are Karplus-Strong, drums are shaped noise,
the ney is a breathy sine. If FAL_KEY is set, gen_audio_fal.py can replace the music with
generated tracks (RawAudio/fal/ wins over RawAudio/ at import).

Naming: music_* and amb_* and loop_* are imported as looping sounds; sfx_* play once.
"""
import math
import os
import random
import sys
import wave
from array import array

SR = 44100
OUT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "RawAudio"))
TWO_PI = 2.0 * math.pi

# D Shur, the mode of the setar melody: the second degree is a koron (a quarter tone flat).
D3 = 146.83


def note(steps_from_d3):
    """Frequency a number of (possibly fractional) semitones above D3."""
    return D3 * (2.0 ** (steps_from_d3 / 12.0))


SHUR = {"D": 12, "Ek": 13.5, "F": 15, "G": 17, "Ak": 18.5, "A": 19, "Bb": 20, "C": 22, "D2": 24,
        "C0": 10, "A0": 7, "d": 0, "a": 7}
MAHUR = {"D": 12, "E": 14, "F#": 16, "G": 17, "A": 19, "B": 21, "C#": 23, "D2": 24, "A0": 7}


def silence(seconds):
    return [0.0] * int(seconds * SR)


def mix(track, sound, at_seconds, gain=1.0):
    start = int(at_seconds * SR)
    end = start + len(sound)
    if end > len(track):
        track.extend([0.0] * (end - len(track)))
    for index, value in enumerate(sound):
        track[start + index] += value * gain


def pluck(freq, seconds, decay=0.996, rng=None, bright=1.0):
    """Karplus-Strong plucked string."""
    rng = rng or random
    period = max(2, int(SR / freq))
    buf = [rng.uniform(-1.0, 1.0) for _ in range(period)]
    if bright < 1.0:
        last = 0.0
        for index in range(period):
            last = last + bright * (buf[index] - last)
            buf[index] = last
    count = int(seconds * SR)
    out = [0.0] * count
    index = 0
    for n in range(count):
        value = buf[index]
        nxt = index + 1
        if nxt == period:
            nxt = 0
        buf[index] = decay * 0.5 * (value + buf[nxt])
        out[n] = value
        index = nxt
    return out


def setar_note(freq, seconds, rng, tremolo=False):
    """Two strings a hair apart; long notes are kept alive with a riz (fast repeated plucks)."""
    out = silence(seconds + 1.2)
    if tremolo:
        t = 0.0
        step = 0.105
        while t < seconds:
            gain = 0.55 + 0.45 * math.sin(math.pi * min(1.0, t / max(seconds, 0.01)))
            mix(out, pluck(freq, 0.9, 0.9955, rng, 0.75), t, 0.5 * gain)
            mix(out, pluck(freq * 1.004, 0.9, 0.9955, rng, 0.75), t + 0.004, 0.35 * gain)
            t += step * rng.uniform(0.93, 1.07)
    else:
        mix(out, pluck(freq, seconds + 1.0, 0.9968, rng, 0.8), 0.0, 0.7)
        mix(out, pluck(freq * 1.004, seconds + 1.0, 0.9968, rng, 0.8), 0.005, 0.45)
    return out


def setar_piece(phrases, beat, seed, scale=SHUR):
    """phrases: list of (name, beats, tremolo). 'r' is a rest. A drone string answers the phrase ends."""
    rng = random.Random(seed)
    track = []
    t = 0.3
    for name, beats, tremolo in phrases:
        seconds = beats * beat
        if name != "r":
            mix(track, setar_note(note(scale[name]), seconds, rng, tremolo), t, 0.8)
            if tremolo and beats >= 3:
                # the open drone strings, brushed
                mix(track, pluck(note(scale.get("d", 0)), 2.5, 0.9975, rng, 0.6), t + 0.02, 0.28)
                mix(track, pluck(note(scale.get("a", 7)), 2.5, 0.9975, rng, 0.6), t + 0.05, 0.2)
        t += seconds
    track.extend(silence(1.5))
    return track


def one_pole(sound, coefficient):
    out = [0.0] * len(sound)
    last = 0.0
    for index, value in enumerate(sound):
        last = last + coefficient * (value - last)
        out[index] = last
    return out


def noise(seconds, rng):
    return [rng.uniform(-1.0, 1.0) for _ in range(int(seconds * SR))]


def envelope(sound, attack, release_power=2.0):
    """Fast attack, curved decay over the whole length."""
    count = len(sound)
    attack_samples = max(1, int(attack * SR))
    for index in range(count):
        rise = min(1.0, index / attack_samples)
        fall = (1.0 - index / count) ** release_power
        sound[index] *= rise * fall
    return sound


def dum(rng, pitch=78.0, seconds=0.5):
    """The low stroke of a frame drum."""
    count = int(seconds * SR)
    out = [0.0] * count
    phase = 0.0
    for index in range(count):
        t = index / SR
        freq = pitch * (1.0 + 1.6 * math.exp(-t * 38.0))
        phase += TWO_PI * freq / SR
        out[index] = math.sin(phase) * math.exp(-t * 7.5)
    skin = envelope(one_pole(noise(0.06, rng), 0.25), 0.001, 3.0)
    mix(out, skin, 0.0, 0.5)
    return out


def tak(rng, seconds=0.16, ring=True):
    """The rim stroke, with the shiver of the daf's rings."""
    burst = noise(seconds, rng)
    low = one_pole(burst, 0.35)
    out = [burst[i] - low[i] for i in range(len(burst))]
    envelope(out, 0.0008, 4.0)
    if ring:
        rings = noise(seconds * 2.2, rng)
        low2 = one_pole(rings, 0.6)
        rings = [rings[i] - low2[i] for i in range(len(rings))]
        envelope(rings, 0.004, 2.5)
        mix(out, rings, 0.0, 0.22)
    return out


def pad(freqs, seconds, rng, brightness=0.06, swell=None):
    """Bowed strings: a few detuned saws per note through a low-pass, slow vibrato."""
    count = int(seconds * SR)
    out = [0.0] * count
    for freq in freqs:
        for detune in (-0.004, 0.0, 0.0045):
            f = freq * (1.0 + detune)
            phase = rng.random()
            vib_phase = rng.random() * TWO_PI
            step = f / SR
            for index in range(count):
                vib = 1.0 + 0.0025 * math.sin(vib_phase + TWO_PI * 5.1 * index / SR)
                phase += step * vib
                if phase >= 1.0:
                    phase -= 1.0
                out[index] += (2.0 * phase - 1.0)
    out = one_pole(one_pole(out, brightness), brightness * 1.6)
    gain = 1.0 / (3.0 * len(freqs))
    for index in range(count):
        t = index / count
        shape = swell(t) if swell else min(1.0, t * 6.0) * min(1.0, (1.0 - t) * 6.0)
        out[index] *= gain * shape
    return out


def ney_note(freq, seconds, rng):
    """A reed flute: breath first, then the tone, a slow vibrato."""
    count = int(seconds * SR)
    out = [0.0] * count
    breath = one_pole(noise(seconds, rng), 0.12)
    phase = 0.0
    for index in range(count):
        t = index / SR
        vib = 1.0 + 0.006 * math.sin(TWO_PI * 4.6 * t) * min(1.0, t * 1.5)
        phase += TWO_PI * freq * vib / SR
        rise = min(1.0, t / 0.22)
        fall = min(1.0, (seconds - t) / 0.35)
        amp = rise * max(0.0, fall)
        tone = math.sin(phase) + 0.32 * math.sin(2.0 * phase) + 0.1 * math.sin(3.0 * phase)
        out[index] = amp * (0.62 * tone + 1.5 * breath[index] * (0.35 + 0.65 * math.exp(-t * 5.0)))
    return out


def loopable(track, seconds=0.6):
    """Cross-fades the tail into the head so a looping sound has no click."""
    fade = int(seconds * SR)
    if len(track) < fade * 3:
        return track
    tail = track[-fade:]
    body = track[:-fade]
    for index in range(fade):
        w = index / fade
        body[index] = body[index] * w + tail[index] * (1.0 - w)
    return body


def normalize(track, peak=0.85):
    top = max(1e-9, max(abs(v) for v in track))
    scale = peak / top
    return [v * scale for v in track]


def save(name, track, peak=0.85):
    os.makedirs(OUT_DIR, exist_ok=True)
    track = normalize(track, peak)
    data = array("h", (int(max(-1.0, min(1.0, v)) * 32767.0) for v in track))
    if sys.byteorder == "big":
        data.byteswap()
    path = os.path.join(OUT_DIR, name + ".wav")
    with wave.open(path, "wb") as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(SR)
        out.writeframes(data.tobytes())
    print("  {:<28s} {:5.1f} s".format(name + ".wav", len(track) / SR))


# ---------------------------------------------------------------------------------------------
# Music
# ---------------------------------------------------------------------------------------------

# The Chapter 1 melody. It asks a question and stops on the second degree: it does not come home.
MELODY_A = [("G", 1, False), ("F", 1, False), ("Ek", 1, False), ("D", 3, True), ("r", 1, False),
            ("F", 1, False), ("G", 1, False), ("Ak", 2, True), ("G", 1, False), ("F", 3, True), ("r", 1.5, False)]
MELODY_B = [("D", 1, False), ("F", 1, False), ("G", 1, False), ("Bb", 2, True), ("Ak", 1, False), ("G", 1, False),
            ("F", 1, False), ("G", 1, False), ("Ek", 4, True), ("r", 2, False)]
# The same melody in Chapter 3: this time it walks down to the tonic and stays.
MELODY_HOME = [("D", 1, False), ("F", 1, False), ("G", 1, False), ("Bb", 2, True), ("Ak", 1, False), ("G", 1, False),
               ("F", 1, False), ("Ek", 1, False), ("D", 5, True), ("r", 1, False), ("A0", 1, False), ("C0", 1, False),
               ("D", 6, True), ("r", 2, False)]


def music_setar():
    save("music_setar", loopable(setar_piece(MELODY_A + MELODY_B, 0.92, 11)), 0.7)


def music_setar_resolved():
    save("music_setar_resolved", setar_piece(MELODY_A + MELODY_HOME, 1.0, 12), 0.7)


def daf_loop(bpm, bars, pattern, rng, ring=True):
    """pattern: string per bar of 'D' dum, 't' tak, 'k' soft tak, '.' rest; evenly spaced."""
    bar = 60.0 / bpm * 4.0
    track = silence(bar * bars)
    for bar_index in range(bars):
        row = pattern[bar_index % len(pattern)]
        step = bar / len(row)
        for index, hit in enumerate(row):
            at = bar_index * bar + index * step + rng.uniform(-0.004, 0.004)
            if hit == "D":
                mix(track, dum(rng), max(0.0, at), 1.0)
            elif hit == "t":
                mix(track, tak(rng, 0.16, ring), max(0.0, at), 0.55)
            elif hit == "k":
                mix(track, tak(rng, 0.09, ring), max(0.0, at), 0.26)
    return track[:int(bar * bars * SR)]


def music_daf():
    """Daf in six and strings that keep rising: the protests."""
    rng = random.Random(21)
    bars = 16
    drums = daf_loop(96, bars, ["D.kt.kD.ktkk", "D.kt.kDktk.k"], rng)
    seconds = len(drums) / SR
    low = pad([note(0), note(7)], seconds, rng, 0.035, lambda t: 0.35 + 0.65 * t)
    high = pad([note(12), note(15), note(19)], seconds, rng, 0.07, lambda t: t * t)
    top = pad([note(24), note(27)], seconds, rng, 0.09, lambda t: max(0.0, t - 0.5) * 2.0)
    track = [drums[i] * 0.9 + low[i] * 0.9 + high[i] * 0.8 + top[i] * 0.5 for i in range(len(drums))]
    save("music_daf", loopable(track, 0.3), 0.8)


def music_chase():
    """Spotted / fight: heavy taiko + rapid daf + low brass blasts."""
    dark_cue("music_chase", 22, 18.0, 0, (0, 6, 12), (13.5, 18.5, 20), None, taiko_bpm=148, daf_bpm=148, tick_bpm=148, swells=3, peak=0.92)


def music_sunrise():
    """The rooftop: a slow ney over warm strings. The brightest thing in the game."""
    rng = random.Random(23)
    beat = 1.45
    line = [("A0", 3), ("D", 4), ("E", 1), ("F#", 3), ("r", 1), ("E", 1), ("D", 2), ("A", 5), ("r", 1),
            ("G", 1), ("F#", 2), ("E", 1), ("D", 6), ("r", 2), ("F#", 2), ("A", 2), ("D2", 5), ("B", 1), ("A", 6), ("r", 2)]
    total = sum(beats for _, beats in line) * beat + 3.0
    strings = pad([note(0), note(7), note(12), note(16)], total, rng, 0.05, lambda t: min(1.0, t * 5.0) * min(1.0, (1.0 - t) * 5.0) * (0.55 + 0.45 * math.sin(math.pi * t)))
    track = [v * 0.9 for v in strings]
    t = 1.2
    for name, beats in line:
        seconds = beats * beat
        if name != "r":
            mix(track, ney_note(note(MAHUR[name]) * 2.0, seconds * 0.96, rng), t, 0.34)
        t += seconds
    save("music_sunrise", loopable(track, 1.2), 0.75)


# ---------------------------------------------------------------------------------------------
# Ambience
# ---------------------------------------------------------------------------------------------

def amb_wind():
    rng = random.Random(31)
    seconds = 24.0
    raw = noise(seconds, rng)
    out = [0.0] * len(raw)
    last = 0.0
    last2 = 0.0
    for index, value in enumerate(raw):
        t = index / SR
        gust = 0.5 + 0.3 * math.sin(TWO_PI * t / 7.3) + 0.2 * math.sin(TWO_PI * t / 3.1 + 1.0)
        coefficient = 0.004 + 0.02 * gust
        last = last + coefficient * (value - last)
        last2 = last2 + coefficient * (last - last2)
        out[index] = last2 * (0.35 + 0.65 * gust)
    save("amb_wind", loopable(out, 2.0), 0.5)


def amb_drone():
    seconds = 20.0
    count = int(seconds * SR)
    out = [0.0] * count
    for index in range(count):
        t = index / SR
        out[index] = (math.sin(TWO_PI * 55.0 * t) + 0.7 * math.sin(TWO_PI * 55.35 * t + 1.0)
                      + 0.4 * math.sin(TWO_PI * 82.6 * t) + 0.15 * math.sin(TWO_PI * 110.2 * t)) * (0.8 + 0.2 * math.sin(TWO_PI * t / 10.0))
    save("amb_drone", loopable(out, 2.0), 0.5)


def amb_city():
    """A day in the city: far traffic, nothing you could name."""
    rng = random.Random(32)
    seconds = 20.0
    low = one_pole(one_pole(noise(seconds, rng), 0.012), 0.012)
    mid = one_pole(noise(seconds, rng), 0.08)
    out = [low[i] * 3.0 * (0.8 + 0.2 * math.sin(TWO_PI * i / SR / 6.0)) + mid[i] * 0.05 for i in range(len(low))]
    save("amb_city", loopable(out, 2.0), 0.4)


def amb_room():
    rng = random.Random(33)
    low = one_pole(one_pole(noise(14.0, rng), 0.006), 0.006)
    save("amb_room", loopable(low, 2.0), 0.18)


def loop_heartbeat():
    """Near the van: everything else stops."""
    rng = random.Random(41)
    beat = 60.0 / 74.0
    track = silence(beat * 6)
    for index in range(6):
        mix(track, dum(rng, 52.0, 0.34), index * beat, 1.0)
        mix(track, dum(rng, 46.0, 0.30), index * beat + 0.27, 0.62)
    save("loop_heartbeat", one_pole(track[:int(beat * 6 * SR)], 0.12), 0.9)


def loop_engine():
    """A small motorcycle, somewhere close."""
    rng = random.Random(42)
    seconds = 3.0
    count = int(seconds * SR)
    out = [0.0] * count
    phase = 0.0
    rough = one_pole(noise(seconds, rng), 0.3)
    for index in range(count):
        t = index / SR
        freq = 62.0 + 4.0 * math.sin(TWO_PI * t / 1.5)
        phase += freq / SR
        if phase >= 1.0:
            phase -= 1.0
        firing = (2.0 * phase - 1.0) * (0.6 + 0.4 * math.sin(TWO_PI * freq * 0.5 * t))
        out[index] = firing + rough[index] * 0.5 * abs(firing)
    save("loop_engine", loopable(one_pole(out, 0.25), 0.2), 0.6)


# ---------------------------------------------------------------------------------------------
# Effects
# ---------------------------------------------------------------------------------------------

def sfx_type():
    """One soft key: the game's voice."""
    for variant in range(3):
        rng = random.Random(50 + variant)
        click = envelope(one_pole(noise(0.03, rng), 0.5), 0.0006, 3.0)
        body = [math.sin(TWO_PI * (1500.0 + 260.0 * variant) * i / SR) * math.exp(-i / SR * 220.0) for i in range(int(0.03 * SR))]
        save("sfx_type{}".format(variant + 1), [click[i] * 0.7 + body[i] * 0.4 for i in range(len(click))], 0.35)


def thump(rng, pitch, seconds, tone=0.2):
    out = dum(rng, pitch, seconds)
    mix(out, envelope(one_pole(noise(0.05, rng), tone), 0.0008, 3.0), 0.0, 0.7)
    return out


def sfx_knock():
    rng = random.Random(61)
    track = silence(1.0)
    for index, at in enumerate((0.0, 0.19, 0.38)):
        mix(track, thump(rng, 170.0, 0.16, 0.3), at, 1.0 - 0.12 * index)
    save("sfx_knock", track, 0.7)


def sfx_door():
    rng = random.Random(62)
    track = silence(0.9)
    mix(track, thump(rng, 95.0, 0.4, 0.15), 0.05, 1.0)
    mix(track, envelope(one_pole(noise(0.03, rng), 0.7), 0.0005, 3.0), 0.0, 0.5)
    save("sfx_door", track, 0.7)


def sfx_beep():
    """No service."""
    track = silence(1.5)
    for index, freq in enumerate((950.0, 1400.0, 1800.0)):
        count = int(0.3 * SR)
        tone = [math.sin(TWO_PI * freq * i / SR) * min(1.0, i / 200.0) * min(1.0, (count - i) / 200.0) for i in range(count)]
        mix(track, tone, index * 0.38, 0.6)
    save("sfx_beep", track, 0.4)


def sfx_static():
    rng = random.Random(63)
    raw = noise(2.0, rng)
    low = one_pole(raw, 0.2)
    out = [(raw[i] - low[i]) * (0.5 + 0.5 * math.sin(TWO_PI * 9.0 * i / SR)) * (1.0 if rng.random() > 0.02 else 0.2) for i in range(len(raw))]
    save("sfx_static", out, 0.35)


def sfx_shots():
    """Far off, down the street. Three, then more."""
    rng = random.Random(64)
    track = silence(4.5)
    for at, gain in ((0.0, 1.0), (0.42, 0.8), (0.7, 0.9), (1.9, 0.6), (2.15, 0.7), (2.3, 0.5)):
        crack = envelope(noise(0.05, rng), 0.0003, 2.0)
        tail = envelope(one_pole(noise(1.3, rng), 0.05), 0.002, 3.0)
        mix(track, crack, at, gain)
        mix(track, tail, at + 0.01, gain * 0.9)
    save("sfx_shots", track, 0.9)


def sfx_notify():
    """A phone buzzing on a desk."""
    track = silence(0.9)
    for at in (0.0, 0.34):
        count = int(0.22 * SR)
        buzz = [(1.0 if math.sin(TWO_PI * 165.0 * i / SR) > 0 else -1.0) * min(1.0, i / 300.0) * min(1.0, (count - i) / 600.0) for i in range(count)]
        mix(track, one_pole(buzz, 0.08), at, 1.0)
    save("sfx_notify", track, 0.45)


def sfx_bird():
    """One bird, at sunrise."""
    rng = random.Random(65)
    track = silence(5.0)
    t = 0.3
    for group in range(4):
        for chirp in range(rng.randint(2, 4)):
            seconds = rng.uniform(0.06, 0.12)
            count = int(seconds * SR)
            f0 = rng.uniform(3200.0, 4200.0)
            f1 = f0 + rng.uniform(-900.0, 1400.0)
            phase = 0.0
            tone = [0.0] * count
            for i in range(count):
                w = i / count
                phase += TWO_PI * (f0 + (f1 - f0) * w) / SR
                tone[i] = math.sin(phase) * math.sin(math.pi * w)
            mix(track, tone, t, 0.5)
            t += seconds + rng.uniform(0.03, 0.08)
        t += rng.uniform(0.6, 1.1)
    save("sfx_bird", track[:int(5.0 * SR)], 0.3)


def sfx_saved():
    """A clip is saved."""
    count = int(0.25 * SR)
    tone = [math.sin(TWO_PI * 1320.0 * i / SR) * math.exp(-i / SR * 16.0) * min(1.0, i / 60.0) for i in range(count)]
    save("sfx_saved", tone, 0.3)


def sfx_caught():
    rng = random.Random(66)
    track = silence(1.2)
    mix(track, dum(rng, 60.0, 1.0), 0.0, 1.0)
    mix(track, envelope(one_pole(noise(0.5, rng), 0.03), 0.002, 2.0), 0.0, 0.8)
    save("sfx_caught", track, 0.8)


def main():
    print("Generating audio into {}".format(OUT_DIR))
    for build in (music_setar, music_setar_resolved, music_daf, music_chase, music_sunrise,
                  amb_wind, amb_drone, amb_city, amb_room, loop_heartbeat, loop_engine,
                  sfx_type, sfx_knock, sfx_door, sfx_beep, sfx_static, sfx_shots, sfx_notify,
                  sfx_bird, sfx_saved, sfx_caught, build_thriller_cues, build_stingers):
        build()
    print("Done.")




# ---------------------------------------------------------------------------------------------
# UPDATE 02: dark thriller/horror score cues (sub-bass drones, low brass, dissonant strings,
# taiko hits, ticking pulse, reversed swells, metallic hits + setar, kamancheh, ney, daf)
# and stingers.
# ---------------------------------------------------------------------------------------------

def sub_drone(freq, seconds, pulse_hz=0.25):
    count = int(seconds * SR)
    out = [0.0] * count
    p1 = 0.0
    p2 = 0.0
    for i in range(count):
        t = i / SR
        mod = 0.72 + 0.28 * math.sin(TWO_PI * pulse_hz * t)
        p1 += TWO_PI * freq / SR
        p2 += TWO_PI * (freq * 0.5) / SR
        out[i] = (0.75 * math.sin(p1) + 0.55 * math.sin(p2) + 0.2 * math.sin(p1 * 1.5)) * mod
    return out


def taiko(rng, pitch=46.0, seconds=1.1):
    """Deep heavy drum hit with sub drop and skin crack."""
    count = int(seconds * SR)
    out = [0.0] * count
    phase = 0.0
    for i in range(count):
        t = i / SR
        f = pitch * (1.0 + 2.2 * math.exp(-t * 28.0))
        phase += TWO_PI * f / SR
        out[i] = (math.sin(phase) + 0.35 * math.sin(phase * 2.0)) * math.exp(-t * 4.2)
    crack = envelope(one_pole(noise(0.08, rng), 0.45), 0.0005, 3.5)
    rumble = envelope(one_pole(noise(seconds * 0.7, rng), 0.03), 0.01, 2.2)
    mix(out, crack, 0.0, 0.75)
    mix(out, rumble, 0.02, 0.45)
    return out


def metal_hit(rng, base=310.0, seconds=1.8):
    """Inharmonic metallic strike with long dark ring."""
    ratios = (1.0, 1.41, 1.73, 2.19, 2.87, 3.54)
    count = int(seconds * SR)
    out = [0.0] * count
    for r in ratios:
        f = base * r * rng.uniform(0.99, 1.01)
        decay = 2.2 + r * 1.1
        for i in range(count):
            t = i / SR
            out[i] += math.sin(TWO_PI * f * t) * math.exp(-t * decay) / len(ratios)
    burst = envelope(noise(0.035, rng), 0.0003, 3.0)
    mix(out, burst, 0.0, 0.4)
    return out


def rev_swell(freq, seconds, rng):
    """Reversed string/brass swell that rises exponentially and cuts off."""
    count = int(seconds * SR)
    raw = pad([freq, freq * 1.059, freq * 1.498], seconds, rng, 0.09, lambda t: (t ** 3.2))
    for i in range(min(int(0.04 * SR), count)):
        raw[-1 - i] *= i / (0.04 * SR)
    return raw


def kamancheh(freq, seconds, rng, mournful=True):
    """Bowed spike fiddle: expressive wide vibrato and nasal resonance."""
    count = int(seconds * SR)
    out = [0.0] * count
    phase = 0.0
    for i in range(count):
        t = i / SR
        slide = 1.0 - (0.025 * math.exp(-t * 9.0) if mournful else 0.0)
        vib = 1.0 + 0.009 * math.sin(TWO_PI * 5.6 * t) * min(1.0, t * 2.0)
        phase += (freq * slide * vib) / SR
        if phase >= 1.0:
            phase -= 1.0
        saw = 2.0 * phase - 1.0
        env = min(1.0, t / 0.25) * min(1.0, max(0.0, (seconds - t) / 0.35))
        out[i] = saw * env
    return one_pole(out, 0.14)


def tick_loop(bpm, seconds, rng, accent_every=4):
    step = 60.0 / bpm / 2.0
    track = silence(seconds)
    t = 0.0
    idx = 0
    while t < seconds - 0.05:
        hi = 0.7 if (idx % accent_every == 0) else 0.35
        click = envelope(one_pole(noise(0.022, rng), 0.65), 0.0004, 4.0)
        mix(track, click, t, hi)
        t += step
        idx += 1
    return track


def dark_cue(name, seed, seconds, drone_semi=0, brass_semis=(0, 7), diss_semis=(13.5, 20),
             melody=None, instrument="setar", taiko_bpm=0, tick_bpm=0, daf_bpm=0, swells=0, peak=0.8):
    rng = random.Random(seed)
    track = sub_drone(note(drone_semi) * 0.5, seconds, rng.uniform(0.16, 0.32))
    track = [v * 0.55 for v in track]
    if brass_semis:
        brass = pad([note(s) * 0.5 for s in brass_semis], seconds, rng, 0.035,
                    lambda t: 0.5 + 0.5 * math.sin(math.pi * t))
        mix(track, brass, 0.0, 0.65)
    if diss_semis:
        diss = pad([note(s) for s in diss_semis], seconds, rng, 0.075,
                   lambda t: 0.35 + 0.65 * (0.5 - 0.5 * math.cos(TWO_PI * t * 2.0)))
        mix(track, diss, 0.0, 0.45)
    if taiko_bpm > 0:
        beat = 60.0 / taiko_bpm
        t = 0.2
        k = 0
        while t < seconds - 1.2:
            mix(track, taiko(rng, 45.0 if k % 2 == 0 else 54.0, 1.0), t, 0.85 if k % 2 == 0 else 0.55)
            if k % 4 == 3 and t + beat * 0.5 < seconds - 1.0:
                mix(track, taiko(rng, 58.0, 0.6), t + beat * 0.5, 0.45)
            t += beat * 2.0
            k += 1
    if tick_bpm > 0:
        mix(track, tick_loop(tick_bpm, seconds, rng), 0.0, 0.45)
    if daf_bpm > 0:
        bars = max(2, int(seconds / (60.0 / daf_bpm * 4.0)))
        drums = daf_loop(daf_bpm, bars, ["D.kt.kD.ktkk", "D.kt.kDktk.k"], rng)
        mix(track, drums[:int(seconds * SR)], 0.0, 0.75)
    for s in range(swells):
        at = (s + 0.6) * (seconds / (swells + 1.0))
        dur = min(2.6, seconds - at - 0.5)
        if dur > 0.8:
            mix(track, rev_swell(note(rng.choice((12, 13.5, 18.5, 20))), dur, rng), at, 0.55)
            mix(track, metal_hit(rng, rng.uniform(240.0, 380.0), 1.6), min(seconds - 1.7, at + dur), 0.35)
    if melody:
        t = 0.8
        for n_name, dur, trem in melody:
            if t >= seconds - 1.5:
                break
            if n_name != "r":
                sc = MAHUR if n_name in MAHUR and n_name not in SHUR else SHUR
                f = note(sc[n_name])
                if instrument == "kamancheh":
                    mix(track, kamancheh(f * 2.0, dur, rng), t, 0.55)
                elif instrument == "ney":
                    mix(track, ney_note(f * 2.0, dur, rng), t, 0.42)
                else:
                    mix(track, setar_note(f, dur, rng, trem), t, 0.68)
            t += dur
    save(name, loopable(track[:int(seconds * SR)], 1.0), peak)


def build_thriller_cues():
    # Menu: sunrise over dark city - deep sub, low brass, kamancheh + ney
    dark_cue("music_menu", 101, 28.0, 0, (0, 7, 12), (13.5, 19),
             [("D", 3.5, True), ("r", 1.0, False), ("F", 2.5, False), ("G", 3.0, True),
              ("Ak", 2.5, False), ("G", 2.5, False), ("F", 3.0, True), ("Ek", 3.5, True), ("D", 4.0, True)],
             instrument="kamancheh", taiko_bpm=36, swells=2, peak=0.8)
    # Poem card: solemn sub drone, reversed swells, sparse setar
    dark_cue("music_poem", 102, 26.0, 0, (0, 7), (13.5, 20),
             [("D", 3.0, True), ("r", 1.5, False), ("Ek", 3.0, True), ("r", 1.5, False),
              ("F", 3.0, False), ("Ek", 3.5, True), ("D", 4.5, True)],
             instrument="setar", swells=3, peak=0.75)
    # Ch1 morning: fragile warm setar with an uneasy low cello shadow
    dark_cue("music_ch1_morning", 103, 28.0, 0, (0, 7), (15,),
             [(n, b * 0.9, tr) for n, b, tr in (MELODY_A + MELODY_B)],
             instrument="setar", peak=0.72)
    # Arrest video / 14:20: sharp horrifying dissonant strings, metallic hits, fast pulse
    dark_cue("music_arrest", 104, 22.0, 1, (0, 6, 13), (13.5, 18.5, 20),
             [("Bb", 1.5, True), ("Ak", 1.5, True), ("Ek", 2.5, True), ("r", 1.0, False),
              ("Bb", 1.5, True), ("E" if "E" in SHUR else "Ek", 3.0, True)],
             instrument="kamancheh", taiko_bpm=112, tick_bpm=112, swells=4, peak=0.88)
    # Run to Vozara: urgent pulse, taiko heartbeat, tense strings
    dark_cue("music_run", 105, 24.0, 0, (0, 7), (13.5, 19),
             None, taiko_bpm=124, tick_bpm=124, daf_bpm=124, swells=2, peak=0.88)
    # Vozara gate: cold oppressive drone, metallic resonance, detuned kamancheh
    dark_cue("music_vozara", 106, 26.0, -2, (-2, 5), (13.5, 18.5),
             [("Ek", 4.0, True), ("r", 2.5, False), ("D", 4.5, True), ("r", 3.0, False), ("A0", 5.0, True)],
             instrument="kamancheh", swells=3, peak=0.78)
    # Family table: grief, slow chamber strings and quiet setar
    dark_cue("music_table", 107, 26.0, 0, (0, 7, 15), (13.5,),
             [("G", 2.5, False), ("F", 2.5, False), ("Ek", 3.5, True), ("r", 2.0, False),
              ("F", 2.5, False), ("Ek", 2.5, False), ("D", 4.5, True)],
             instrument="setar", peak=0.7)
    # Taxi after the alert: low creeping dread, reversed swells
    dark_cue("music_taxi", 108, 24.0, -2, (-2, 4), (13.5, 20),
             [("Ek", 4.5, True), ("r", 3.0, False), ("C0", 4.5, True)],
             instrument="kamancheh", tick_bpm=64, swells=3, peak=0.78)
    # Lawyer: hopeless cello & kamancheh duet over dark drone
    dark_cue("music_lawyer", 109, 26.0, 0, (0, 7, 12), (13.5, 17),
             [("F", 3.0, False), ("Ek", 3.0, False), ("D", 4.0, True), ("r", 1.5, False),
              ("G", 2.5, False), ("F", 2.5, False), ("Ek", 4.5, True)],
             instrument="kamancheh", peak=0.74)
    # Rooftops: rising chilling brass & trembling strings + distant daf
    dark_cue("music_rooftops", 110, 26.0, 0, (0, 7, 12), (13.5, 19, 22),
             [("D", 3.0, True), ("F", 2.5, True), ("G", 2.5, True), ("Ak", 3.5, True), ("Bb", 4.0, True), ("D2", 4.5, True)],
             instrument="kamancheh", taiko_bpm=68, daf_bpm=68, swells=3, peak=0.84)
    # Stairwell: broken solitary setar, sparse detuned notes
    dark_cue("music_stairwell", 111, 24.0, 0, (0,), (13.5,),
             [("F", 3.5, False), ("r", 2.5, False), ("Ek", 4.0, False), ("r", 3.0, False), ("D", 5.0, False)],
             instrument="setar", peak=0.68)
    # Ch1 end: dark vow - setar joined by low brass and heavy taiko
    dark_cue("music_ch1_end", 112, 26.0, 0, (0, 7, 12), (15, 19),
             [(n, b * 0.9, True) for n, b, _ in MELODY_B],
             instrument="setar", taiko_bpm=60, swells=2, peak=0.85)
    # Ch2 university: tense hope - rhythmic daf, pulsing bass, setar
    dark_cue("music_ch2_university", 113, 26.0, 0, (0, 7), (15, 19),
             [("D", 1.5, False), ("F", 1.5, False), ("G", 2.0, True), ("Ak", 2.0, True), ("G", 1.5, False), ("F", 2.5, True)],
             instrument="setar", daf_bpm=92, tick_bpm=92, peak=0.8)
    # Siege (parking garage): horror stealth - deep sub pulses, ticking metal, dissonance
    dark_cue("music_siege", 114, 26.0, -2, (-2, 4, 11), (13.5, 18.5, 20),
             None, taiko_bpm=54, tick_bpm=108, swells=4, peak=0.84)
    # Kian call: tender sad setar and warm strings
    dark_cue("music_kian", 115, 26.0, 0, (0, 7, 15), None,
             [(n, b * 1.0, tr) for n, b, tr in MELODY_A],
             instrument="setar", peak=0.72)
    # Night streets: dark pulsing stealth score
    dark_cue("music_nightstreets", 116, 28.0, 0, (0, 7), (13.5, 19),
             [("D", 2.5, True), ("r", 2.0, False), ("G", 2.5, True), ("F", 2.5, False), ("Ek", 3.5, True)],
             instrument="kamancheh", taiko_bpm=76, daf_bpm=96, tick_bpm=96, swells=2, peak=0.84)
    # Suspicion layer: fast ticking + rising dissonant tremolo + sub throb
    dark_cue("music_suspicion", 117, 18.0, 1, (1, 7), (13.5, 18.5, 20),
             None, taiko_bpm=104, tick_bpm=136, swells=3, peak=0.85)
    # Mom's message: fragile kamancheh and setar harmonics
    dark_cue("music_mom", 118, 26.0, 0, (0, 7, 15), None,
             [("G", 3.0, False), ("F", 2.5, False), ("Ek", 3.5, True), ("D", 4.5, True), ("r", 1.5, False), ("A0", 4.0, True)],
             instrument="kamancheh", peak=0.72)
    # Last night (24 Mehr): epic tragic - heavy taiko + daf + soaring kamancheh + full brass
    dark_cue("music_lastnight", 119, 28.0, 0, (0, 7, 12, 15), (13.5, 19, 22),
             [("D", 1.5, True), ("F", 1.5, True), ("G", 2.0, True), ("Bb", 3.0, True), ("Ak", 1.5, True),
              ("G", 1.5, True), ("F", 2.0, True), ("G", 1.5, True), ("D2", 4.0, True), ("C", 2.0, True), ("Bb", 3.5, True)],
             instrument="kamancheh", taiko_bpm=116, daf_bpm=116, swells=3, peak=0.9)
    # Ch3 prison gate: empty cold morning, distant ney
    dark_cue("music_ch3_gate", 120, 26.0, 0, (0, 7), (15,),
             [("A", 4.5, False), ("r", 3.0, False), ("G", 3.5, False), ("F", 4.0, False), ("D", 5.0, False)],
             instrument="ney", peak=0.68)
    # Letter: intimate setar and warm pad resolving gently
    dark_cue("music_letter", 121, 28.0, 0, (0, 7, 15), None,
             [(n, b * 1.0, tr) for n, b, tr in (MELODY_A + MELODY_HOME)],
             instrument="setar", peak=0.74)
    # News (years later): uneasy stillness, slow motif
    dark_cue("music_news", 122, 24.0, 0, (0, 7, 12), (16,),
             [("D", 3.5, False), ("r", 2.0, False), ("A", 3.5, True), ("G", 3.0, False), ("F#", 4.5, True)],
             instrument="setar", peak=0.7)
    # Cemetery: sorrow resolving to light in Mahur (ney + setar)
    dark_cue("music_cemetery", 123, 28.0, 0, (0, 7, 12, 16), None,
             [("D", 3.0, False), ("E", 2.0, False), ("F#", 3.5, True), ("G", 2.5, False), ("A", 4.5, True),
              ("F#", 2.5, False), ("E", 2.0, False), ("D", 5.0, True)],
             instrument="ney", peak=0.76)
    # Final song for his name & credits: the main theme in full (setar + kamancheh + ney + gentle daf)
    dark_cue("music_final", 124, 34.0, 0, (0, 7, 12, 15), None,
             [(n, b * 1.05, tr) for n, b, tr in (MELODY_A + MELODY_B + MELODY_HOME)],
             instrument="setar", taiko_bpm=48, daf_bpm=72, peak=0.82)


def build_stingers():
    rng = random.Random(201)
    # The green van: low brass/sub drop + metallic shudder
    van = silence(2.6)
    mix(van, taiko(rng, 38.0, 1.8), 0.0, 1.0)
    mix(van, metal_hit(rng, 210.0, 2.2), 0.04, 0.65)
    mix(van, pad([note(-12), note(-5), note(1.5)], 2.4, rng, 0.05, lambda t: (1.0 - t) ** 1.6), 0.0, 0.8)
    save("sfx_sting_van", van, 0.9)

    # 14:20 message: sharp string stab + sub thump
    msg = silence(2.2)
    mix(msg, taiko(rng, 48.0, 1.2), 0.0, 0.95)
    mix(msg, metal_hit(rng, 420.0, 1.8), 0.0, 0.7)
    mix(msg, pad([note(12), note(13.5), note(18.5), note(24)], 2.0, rng, 0.12, lambda t: (1.0 - t) ** 2.2), 0.0, 0.85)
    save("sfx_sting_message", msg, 0.9)

    # News alert: після 5s silence, a hollow deep toll that sinks
    al = silence(3.2)
    mix(al, taiko(rng, 36.0, 2.4), 0.0, 0.9)
    mix(al, metal_hit(rng, 165.0, 3.0), 0.02, 0.5)
    mix(al, pad([note(-12), note(0), note(1.5)], 3.0, rng, 0.035, lambda t: min(1.0, t * 4.0) * ((1.0 - t) ** 1.5)), 0.0, 0.85)
    save("sfx_sting_alert", al, 0.85)

    # Spotted by a guard: screeching dissonant cluster + taiko slam
    sp = silence(1.8)
    mix(sp, taiko(rng, 52.0, 1.1), 0.0, 1.0)
    mix(sp, metal_hit(rng, 520.0, 1.4), 0.0, 0.8)
    mix(sp, pad([note(13.5), note(19), note(20), note(25.5)], 1.5, rng, 0.16, lambda t: (1.0 - t) ** 2.5), 0.0, 0.9)
    save("sfx_sting_spotted", sp, 0.92)

    # The shot on the rooftop: one deafening crack + sub shockwave that cuts off into silence
    sh = silence(1.2)
    crack = envelope(noise(0.06, rng), 0.0002, 2.2)
    boom = taiko(rng, 34.0, 0.9)
    ring = metal_hit(rng, 680.0, 0.8)
    mix(sh, crack, 0.0, 1.0)
    mix(sh, boom, 0.0, 1.0)
    mix(sh, ring, 0.0, 0.45)
    # hard cut at 0.85 s so the silence hits like a wall
    cut = int(0.85 * SR)
    for i in range(cut, len(sh)):
        sh[i] = 0.0
    save("sfx_sting_shot", sh, 0.98)

if __name__ == "__main__":
    main()
