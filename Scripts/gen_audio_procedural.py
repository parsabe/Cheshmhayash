"""Generates every sound and music cue of the game procedurally into RawAudio/*.wav.

    Scripts\\gen_audio.bat

Pure standard library (no numpy), so it runs with the Python that ships with Unreal Engine.
All music cues are built around a deep, dark cinematic psychological horror / dread palette
(Chernobyl / Sicario / Prisoners style):
  - Deep sub-bass horror drones (32-65 Hz) with slow ominous pitch drift
  - Low-register bowed contrabass / dark cello clusters (zero harsh high-frequency sawtooths)
  - Muffled subterranean heartbeat / war-drum pulses
  - Deep low-register felt-piano / muted dark string tones with cavernous dark reverb
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

# D2 / D3 dark minor/Phrygian reference
D3 = 146.83


def note(steps_from_d3):
    """Frequency a number of (possibly fractional) semitones from D3."""
    return D3 * (2.0 ** (steps_from_d3 / 12.0))


SHUR = {
    "d0": -12, "a0": -5, "bb0": -4, "c0": -2,
    "d": 0, "eb": 1, "ek": 1.5, "f": 3, "g": 5, "ab": 6, "a": 7, "bb": 8, "c": 10,
    "D": 0, "Ek": 1, "F": 3, "G": 5, "Ak": 6, "A": 7, "Bb": 8, "C": 10, "D2": 12,
    "C0": -2, "A0": -5
}
MAHUR = {
    "D": 0, "E": 2, "F#": 3, "G": 5, "A": 7, "B": 8, "C#": 10, "D2": 12, "A0": -5
}


def silence(seconds):
    return [0.0] * int(seconds * SR)


def mix(track, sound, at_seconds, gain=1.0):
    start = max(0, int(at_seconds * SR))
    end = start + len(sound)
    if end > len(track):
        track.extend([0.0] * (end - len(track)))
    for index, value in enumerate(sound):
        track[start + index] += value * gain


def one_pole(sound, coefficient):
    out = [0.0] * len(sound)
    last = 0.0
    for index, value in enumerate(sound):
        last = last + coefficient * (value - last)
        out[index] = last
    return out


def lowpass_warm(sound, cutoff_coeff=0.04):
    """Two-stage warm low-pass filter that eliminates any harsh treble buzz."""
    return one_pole(one_pole(sound, cutoff_coeff), min(1.0, cutoff_coeff * 1.4))


def dark_reverb(sound, wet=0.36):
    """Cavernous, heavily damped dark reverb so tones sit in a deep space with zero metallic ring."""
    n = len(sound)
    out = list(sound)
    delays = (int(0.079 * SR), int(0.131 * SR), int(0.211 * SR), int(0.307 * SR))
    gains = (0.42, 0.33, 0.25, 0.18)
    lp = 0.0
    for d, g in zip(delays, gains):
        for i in range(d, n):
            lp = lp + 0.08 * (out[i - d] - lp)
            out[i] += lp * g * wet
    return out


def noise(seconds, rng):
    return [rng.uniform(-1.0, 1.0) for _ in range(int(seconds * SR))]


def envelope(sound, attack, release_power=2.0):
    count = len(sound)
    attack_samples = max(1, int(attack * SR))
    for index in range(count):
        rise = min(1.0, index / attack_samples)
        fall = (1.0 - index / count) ** release_power
        sound[index] *= rise * fall
    return sound


def loopable(track, seconds=1.2):
    """Cross-fades the tail into the head so a looping sound has no seam."""
    fade = int(seconds * SR)
    if len(track) < fade * 3:
        return track
    tail = track[-fade:]
    body = track[:-fade]
    for index in range(fade):
        w = index / fade
        body[index] = body[index] * w + tail[index] * (1.0 - w)
    return body


def normalize(track, peak=0.55):
    top = max(1e-9, max(abs(v) for v in track))
    scale = peak / top
    return [v * scale for v in track]


def save(name, track, peak=0.55):
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
# Deep Horror / Dread Synthesis Primitives (No harsh sawtooths, no rapid clicking)
# ---------------------------------------------------------------------------------------------

def deep_horror_drone(root_freq, seconds, rng, tension=0.6):
    """Deep sub-bass + contrabass dread drone with slow ominous pitch drift and breathing pulse."""
    count = int(seconds * SR)
    out = [0.0] * count
    f_sub = max(34.0, min(68.0, root_freq * 0.5))
    f_low = f_sub * 2.0
    f_fifth = f_sub * 2.996
    f_min2 = f_low * 1.05946  # Phrygian minor second for dark horror tension

    p_sub = rng.random() * TWO_PI
    p_low = rng.random() * TWO_PI
    p_low2 = rng.random() * TWO_PI
    p_fifth = rng.random() * TWO_PI
    p_min2 = rng.random() * TWO_PI
    lfo1 = rng.uniform(0.07, 0.14)
    lfo2 = rng.uniform(0.11, 0.21)

    for i in range(count):
        t = i / SR
        # Slow menacing pitch bend
        drift = 1.0 - 0.008 * math.sin(TWO_PI * 0.05 * t)
        breath = 0.68 + 0.32 * math.sin(TWO_PI * lfo1 * t)
        dread_mod = 0.5 + 0.5 * math.sin(TWO_PI * lfo2 * t + 1.3)

        p_sub += TWO_PI * (f_sub * drift) / SR
        p_low += TWO_PI * (f_low * drift) / SR
        p_low2 += TWO_PI * (f_low * 1.003 * drift) / SR
        p_fifth += TWO_PI * (f_fifth * drift) / SR
        p_min2 += TWO_PI * (f_min2 * drift) / SR

        sub_val = math.sin(p_sub) + 0.35 * math.sin(p_sub * 2.0)
        cello_val = 0.55 * math.sin(p_low) + 0.45 * math.sin(p_low2) + 0.22 * math.sin(p_fifth)
        horror_val = tension * 0.32 * dread_mod * math.sin(p_min2)
        out[i] = (0.65 * sub_val + 0.55 * cello_val + horror_val) * breath

    # Add subtle dark cavernous air rumble
    rumble = lowpass_warm(noise(seconds, rng), 0.004)
    for i in range(count):
        out[i] += rumble[i] * 0.45
    return out


def dark_cello_swell(freq, seconds, rng, semitone_cluster=None):
    """Low, warm bowed contrabass/cello swell (capped in low register, zero high buzz)."""
    count = int(seconds * SR)
    out = [0.0] * count
    base_f = freq
    while base_f > 185.0:
        base_f *= 0.5
    while base_f < 55.0:
        base_f *= 2.0

    tones = [base_f]
    if semitone_cluster:
        for st in semitone_cluster:
            tones.append(base_f * (2.0 ** (st / 12.0)))

    for f in tones:
        for det in (-0.0035, 0.0035):
            fd = f * (1.0 + det)
            p = rng.random() * TWO_PI
            vp = rng.random() * TWO_PI
            for i in range(count):
                t = i / SR
                vib = 1.0 + 0.0022 * math.sin(vp + TWO_PI * 3.8 * t)
                # Slow downward glissando at the end for horror unease
                bend = 1.0 - 0.015 * max(0.0, (t / seconds - 0.65) / 0.35)
                p += TWO_PI * (fd * vib * bend) / SR
                # Warm odd/even harmonic blend (no raw sawtooth!)
                wave_val = math.sin(p) + 0.42 * math.sin(2.0 * p) + 0.18 * math.sin(3.0 * p) + 0.07 * math.sin(4.0 * p)
                out[i] += wave_val / (2.0 * len(tones))

    out = lowpass_warm(out, 0.032)
    for i in range(count):
        u = i / count
        env = math.sin(math.pi * u) ** 1.4
        out[i] *= env
    return out


def horror_braam(freq, seconds, rng):
    """Deep, ominous Sicario/Chernobyl-style low brass/sub dread swell."""
    count = int(seconds * SR)
    out = [0.0] * count
    f0 = max(42.0, min(88.0, freq * 0.5))
    f1 = f0 * 1.05946  # minor second dissonance in low bass
    f2 = f0 * 1.498
    p0 = 0.0
    p1 = 0.0
    p2 = 0.0
    for i in range(count):
        t = i / SR
        u = i / count
        bend = 1.0 - 0.025 * (u ** 1.8)
        p0 += TWO_PI * (f0 * bend) / SR
        p1 += TWO_PI * (f1 * bend) / SR
        p2 += TWO_PI * (f2 * bend) / SR
        raw = (
            math.sin(p0)
            + 0.65 * math.sin(2.0 * p0)
            + 0.35 * math.sin(3.0 * p0)
            + 0.45 * math.sin(p1)
            + 0.30 * math.sin(p2)
        )
        # Soft saturation
        sat = math.tanh(raw * 0.9)
        env = min(1.0, u / 0.12) * ((1.0 - u) ** 1.35)
        out[i] = sat * env
    return lowpass_warm(out, 0.028)


def felt_note(freq, seconds, rng, mournful=False):
    """Deep, warm felt-piano / muted low harp note with soft attack and dark decay (no harsh plucks)."""
    dur = max(1.6, seconds + 1.2)
    count = int(dur * SR)
    out = [0.0] * count
    f = freq
    while f > 290.0:
        f *= 0.5
    while f < 70.0:
        f *= 2.0

    p1 = rng.random() * TWO_PI
    p2 = rng.random() * TWO_PI
    for i in range(count):
        t = i / SR
        glide = 1.0 - (0.008 * math.exp(-t * 4.0) if mournful else 0.0)
        p1 += TWO_PI * (f * glide) / SR
        p2 += TWO_PI * (f * 1.0018 * glide) / SR
        attack = min(1.0, t / 0.035)
        fundamental = (math.sin(p1) + 0.7 * math.sin(p2)) * math.exp(-t * 1.35)
        overtone2 = 0.28 * math.sin(2.0 * p1) * math.exp(-t * 4.2)
        overtone3 = 0.08 * math.sin(3.0 * p1) * math.exp(-t * 9.5)
        out[i] = attack * (fundamental + overtone2 + overtone3) * 0.55

    # Soft felt hammer thump (heavily low-passed)
    felt = envelope(lowpass_warm(noise(0.04, rng), 0.04), 0.002, 3.0)
    mix(out, felt, 0.0, 0.18)
    return out


def ney_note(freq, seconds, rng):
    """Warm, low-register breathy flute note without shrill upper partials."""
    count = int(seconds * SR)
    out = [0.0] * count
    f = freq
    while f > 260.0:
        f *= 0.5
    breath = lowpass_warm(noise(seconds, rng), 0.025)
    phase = 0.0
    for index in range(count):
        t = index / SR
        vib = 1.0 + 0.003 * math.sin(TWO_PI * 3.8 * t) * min(1.0, t * 1.2)
        phase += TWO_PI * f * vib / SR
        rise = min(1.0, t / 0.35)
        fall = min(1.0, max(0.0, (seconds - t) / 0.45))
        amp = rise * fall
        tone = math.sin(phase) + 0.18 * math.sin(2.0 * phase)
        out[index] = amp * (0.65 * tone + 0.35 * breath[index])
    return lowpass_warm(out, 0.045)


def dum(rng, pitch=52.0, seconds=0.65):
    """Deep muffled heartbeat / frame-drum thud."""
    count = int(seconds * SR)
    out = [0.0] * count
    phase = 0.0
    for index in range(count):
        t = index / SR
        freq = pitch * (1.0 + 1.1 * math.exp(-t * 26.0))
        phase += TWO_PI * freq / SR
        out[index] = math.sin(phase) * math.exp(-t * 6.2)
    skin = envelope(lowpass_warm(noise(0.05, rng), 0.05), 0.002, 3.0)
    mix(out, skin, 0.0, 0.25)
    return out


def tak(rng, seconds=0.14, ring=False):
    """Soft, dark rim stroke (no harsh high-frequency hiss)."""
    burst = lowpass_warm(noise(seconds, rng), 0.07)
    envelope(burst, 0.002, 3.5)
    return burst


def taiko(rng, pitch=42.0, seconds=1.35):
    """Massive subterranean horror impact drum with sub drop and dark room tail."""
    count = int(seconds * SR)
    out = [0.0] * count
    phase = 0.0
    for i in range(count):
        t = i / SR
        f = pitch * (1.0 + 1.5 * math.exp(-t * 22.0))
        phase += TWO_PI * f / SR
        out[i] = (math.sin(phase) + 0.28 * math.sin(phase * 2.0)) * math.exp(-t * 3.6)
    rumble = envelope(lowpass_warm(noise(seconds * 0.85, rng), 0.015), 0.01, 2.0)
    mix(out, rumble, 0.01, 0.45)
    return out


def dread_heartbeat_loop(bpm, seconds, rng, double_beat=True, gain=0.75):
    """Deep, ominous heartbeat / dread pulse loop."""
    beat = 60.0 / bpm
    track = silence(seconds)
    t = 0.15
    k = 0
    while t < seconds - 1.1:
        mix(track, taiko(rng, 42.0 if k % 2 == 0 else 40.0, 1.0), t, gain)
        if double_beat and t + beat * 0.36 < seconds - 0.9:
            mix(track, dum(rng, 38.0, 0.55), t + beat * 0.36, gain * 0.62)
        t += beat * (2.0 if bpm > 85 else 1.0)
        k += 1
    return track


# ---------------------------------------------------------------------------------------------
# Unified Dark Psychological Horror / Cinematic Score Builder
# ---------------------------------------------------------------------------------------------

MELODY_A = [("G", 2.2, False), ("F", 2.0, False), ("Ek", 2.4, False), ("D", 3.8, False), ("r", 1.8, False),
            ("F", 2.0, False), ("G", 2.2, False), ("Ak", 2.8, False), ("F", 3.6, False), ("r", 2.0, False)]
MELODY_B = [("D", 2.0, False), ("F", 2.0, False), ("G", 2.4, False), ("Bb", 3.0, False), ("Ak", 2.0, False),
            ("G", 2.2, False), ("F", 2.2, False), ("Ek", 4.2, False), ("r", 2.5, False)]
MELODY_HOME = [("D", 2.2, False), ("F", 2.0, False), ("G", 2.4, False), ("Ak", 2.4, False), ("G", 2.0, False),
               ("F", 2.2, False), ("Ek", 2.2, False), ("D", 5.0, False), ("r", 2.5, False)]


def dark_cue(name, seed, seconds, drone_semi=0, cluster_semis=(1, 7),
             melody=None, instrument="felt", pulse_bpm=0, braams=0, tension=0.65, peak=0.52):
    rng = random.Random(seed)
    root = note(drone_semi)
    track = deep_horror_drone(root, seconds, rng, tension=tension)
    track = [v * 0.58 for v in track]

    # Slow dark bowed contrabass/cello swells across the cue
    num_swells = max(2, int(seconds / 7.5))
    for idx in range(num_swells):
        swell_dur = min(9.5, seconds / num_swells + 2.5)
        at = idx * (seconds - swell_dur) / max(1, num_swells - 1)
        semi = rng.choice(cluster_semis) if cluster_semis else 0
        swell = dark_cello_swell(note(drone_semi + semi), swell_dur, rng, semitone_cluster=(1, 7) if tension > 0.55 else (7,))
        mix(track, swell, at, 0.52)

    # Ominous low sub-brass horror braams
    for b in range(braams):
        at = (b + 0.45) * (seconds / (braams + 0.8))
        dur = min(4.2, seconds - at - 0.5)
        if dur > 1.2:
            mix(track, horror_braam(root, dur, rng), at, 0.68)

    # Subterranean heartbeat / taiko dread pulse
    if pulse_bpm > 0:
        hb = dread_heartbeat_loop(pulse_bpm, seconds, rng, double_beat=(pulse_bpm <= 95), gain=0.72)
        mix(track, hb, 0.0, 0.78)

    # Sparse, low-register melody (felt piano, dark cello, or low breathy ney)
    if melody:
        t = 1.2
        for n_name, dur, _ in melody:
            if t >= seconds - 2.0:
                break
            if n_name != "r":
                sc = MAHUR if n_name in MAHUR and n_name not in SHUR else SHUR
                f = note(sc.get(n_name, 0))
                if instrument == "ney":
                    mix(track, ney_note(f, dur * 1.15, rng), t, 0.38)
                elif instrument == "cello":
                    mix(track, dark_cello_swell(f, max(2.4, dur * 1.2), rng), t, 0.46)
                else:
                    mix(track, felt_note(f, dur, rng, mournful=(tension > 0.5)), t, 0.48)
            t += max(1.4, dur)

    track = dark_reverb(track[:int(seconds * SR)], wet=0.38)
    save(name, loopable(track, 1.4), peak)


def music_setar():
    dark_cue("music_setar", 11, 28.0, 0, (1, 7), MELODY_A, instrument="felt", tension=0.45, peak=0.48)


def music_setar_resolved():
    dark_cue("music_setar_resolved", 12, 28.0, 0, (3, 7), MELODY_HOME, instrument="felt", tension=0.3, peak=0.48)


def music_daf():
    dark_cue("music_daf", 21, 26.0, 0, (1, 6, 7), None, pulse_bpm=88, braams=3, tension=0.8, peak=0.56)


def music_chase():
    """High-tension horror chase: pounding subterranean war drums + deep roaring dread braams."""
    dark_cue("music_chase", 22, 20.0, -2, (1, 6, 7), None, pulse_bpm=128, braams=5, tension=0.95, peak=0.62)


def music_sunrise():
    """Rooftop before dawn: deep somber cello pad and low breathy ney."""
    dark_cue("music_sunrise", 23, 28.0, 0, (3, 7),
             [("A0", 3.5, False), ("D", 4.0, False), ("F#", 3.5, False), ("r", 2.0, False),
              ("E", 3.0, False), ("D", 5.0, False)],
             instrument="ney", tension=0.25, peak=0.48)


def build_thriller_cues():
    # Menu: deep ominous sub-drone + slow mournful low cello & sparse felt notes
    dark_cue("music_menu", 101, 28.0, -2, (1, 7),
             [("D", 4.0, False), ("r", 2.0, False), ("Ek", 3.5, False), ("r", 2.0, False), ("A0", 5.0, False)],
             instrument="cello", pulse_bpm=44, braams=2, tension=0.75, peak=0.50)

    # Poem card: subterranean dread drone, slow dark cello swells
    dark_cue("music_poem", 102, 26.0, -2, (1, 6),
             [("D", 4.0, False), ("r", 2.5, False), ("Ek", 4.5, False), ("D", 5.0, False)],
             instrument="felt", braams=2, tension=0.70, peak=0.48)

    # Ch1 morning: quiet, uneasy stillness with a dark shadow underneath
    dark_cue("music_ch1_morning", 103, 28.0, 0, (1, 7),
             MELODY_A, instrument="felt", tension=0.45, peak=0.46)

    # Arrest video / 14:20: pure psychological horror & crisis - deep sub braams, pounding heartbeat, dark minor-second dread
    dark_cue("music_arrest", 104, 24.0, -2, (1, 6, 7),
             [("Ek", 3.5, False), ("r", 2.0, False), ("d0", 4.5, False)],
             instrument="cello", pulse_bpm=92, braams=4, tension=0.95, peak=0.58)

    # Run to Vozara: relentless deep heartbeat pulse + dark sub-brass dread waves
    dark_cue("music_run", 105, 24.0, -2, (1, 6, 7),
             None, pulse_bpm=112, braams=4, tension=0.90, peak=0.58)

    # Vozara gate: cold, oppressive detention wall - cavernous sub-drone, slow heavy dread thuds
    dark_cue("music_vozara", 106, 28.0, -3, (1, 6),
             [("Ek", 4.5, False), ("r", 3.0, False), ("D", 4.5, False), ("r", 3.5, False), ("A0", 5.0, False)],
             instrument="cello", pulse_bpm=48, braams=2, tension=0.82, peak=0.52)

    # Family table: heavy, suffocating grief at home
    dark_cue("music_table", 107, 26.0, 0, (1, 7),
             [("G", 3.0, False), ("F", 3.0, False), ("Ek", 4.0, False), ("r", 2.5, False), ("D", 5.0, False)],
             instrument="felt", pulse_bpm=42, tension=0.60, peak=0.48)

    # Taxi after the breaking news alert: creeping horror and disbelief
    dark_cue("music_taxi", 108, 26.0, -3, (1, 6),
             [("Ek", 4.5, False), ("r", 3.5, False), ("C0", 5.0, False)],
             instrument="cello", pulse_bpm=54, braams=3, tension=0.85, peak=0.52)

    # Lawyer: institutional dread - deep contrabass drone and sparse low felt notes
    dark_cue("music_lawyer", 109, 26.0, -2, (1, 7),
             [("F", 3.5, False), ("Ek", 3.5, False), ("D", 4.5, False), ("r", 2.5, False), ("A0", 4.5, False)],
             instrument="cello", pulse_bpm=46, braams=2, tension=0.75, peak=0.50)

    # Rooftops at night: vast dark sky, ominous low brass swells & distant heartbeat
    dark_cue("music_rooftops", 110, 28.0, -2, (1, 7, 8),
             [("D", 3.5, False), ("F", 3.0, False), ("G", 3.5, False), ("Ek", 4.5, False)],
             instrument="cello", pulse_bpm=60, braams=3, tension=0.80, peak=0.54)

    # Stairwell: solitary grief in the dark stairwell
    dark_cue("music_stairwell", 111, 26.0, 0, (1, 7),
             [("F", 3.8, False), ("r", 3.0, False), ("Ek", 4.2, False), ("r", 3.0, False), ("D", 5.0, False)],
             instrument="felt", tension=0.55, peak=0.46)

    # Ch1 end: dark vow in the shadow of his room
    dark_cue("music_ch1_end", 112, 26.0, -2, (1, 7, 8),
             MELODY_B, instrument="cello", pulse_bpm=58, braams=3, tension=0.82, peak=0.54)

    # Ch2 university: tense underground resolve
    dark_cue("music_ch2_university", 113, 26.0, -1, (1, 7),
             [("D", 2.5, False), ("F", 2.5, False), ("G", 3.0, False), ("Ek", 3.5, False)],
             instrument="cello", pulse_bpm=68, braams=2, tension=0.72, peak=0.52)

    # Siege (parking garage): pure stealth horror - deep subterranean heartbeat, suffocating sub-sweeps
    dark_cue("music_siege", 114, 26.0, -4, (1, 6, 7),
             None, pulse_bpm=78, braams=4, tension=0.94, peak=0.56)

    # Kian call: quiet parting before the storm
    dark_cue("music_kian", 115, 26.0, 0, (3, 7),
             MELODY_A, instrument="felt", tension=0.48, peak=0.46)

    # Night streets: nocturnal horror stealth - prowling sub-bass pulse & dark bowed contrabass
    dark_cue("music_nightstreets", 116, 28.0, -3, (1, 6, 7),
             [("D", 3.5, False), ("r", 2.5, False), ("Ek", 4.0, False), ("r", 3.0, False), ("A0", 4.5, False)],
             instrument="cello", pulse_bpm=82, braams=3, tension=0.88, peak=0.56)

    # Suspicion layer: accelerating deep heartbeat throb + rising low-bass dread
    dark_cue("music_suspicion", 117, 18.0, -2, (1, 6),
             None, pulse_bpm=108, braams=3, tension=0.92, peak=0.56)

    # Mom's voice message in the alley: fragile low cello & felt piano
    dark_cue("music_mom", 118, 26.0, 0, (1, 7),
             [("G", 3.5, False), ("F", 3.0, False), ("Ek", 4.0, False), ("D", 5.0, False)],
             instrument="felt", tension=0.52, peak=0.48)

    # Last night (24 Mehr): monumental dark dread - heavy war-drum heartbeat + deep sub-brass walls
    dark_cue("music_lastnight", 119, 28.0, -2, (1, 6, 7, 8),
             [("D", 2.5, False), ("F", 2.5, False), ("G", 3.0, False), ("Bb", 3.5, False),
              ("G", 2.5, False), ("Ek", 4.5, False)],
             instrument="cello", pulse_bpm=104, braams=4, tension=0.92, peak=0.58)

    # Ch3 prison gate: pale cold morning outside the prison wall
    dark_cue("music_ch3_gate", 120, 26.0, -2, (1, 7),
             [("A", 4.5, False), ("r", 3.0, False), ("G", 4.0, False), ("F", 4.5, False), ("D", 5.0, False)],
             instrument="ney", tension=0.55, peak=0.48)

    # Letter: intimate felt piano and warm dark cello
    dark_cue("music_letter", 121, 28.0, 0, (3, 7),
             MELODY_HOME, instrument="felt", tension=0.38, peak=0.48)

    # News (years later): deep solemn stillness
    dark_cue("music_news", 122, 26.0, 0, (3, 7),
             [("D", 4.0, False), ("r", 2.5, False), ("A", 4.0, False), ("G", 3.5, False), ("D", 5.0, False)],
             instrument="cello", tension=0.42, peak=0.48)

    # Cemetery: sorrow resolving into quiet dawn
    dark_cue("music_cemetery", 123, 28.0, 0, (3, 7),
             [("D", 3.5, False), ("E", 3.0, False), ("F#", 4.0, False), ("G", 3.0, False),
              ("E", 3.0, False), ("D", 5.5, False)],
             instrument="ney", tension=0.32, peak=0.48)

    # Final memorial & credits: deep cinematic cello + felt piano + slow heartbeat
    dark_cue("music_final", 124, 34.0, 0, (3, 7),
             MELODY_A + MELODY_HOME, instrument="felt", pulse_bpm=44, braams=2, tension=0.45, peak=0.52)


def build_stingers():
    rng = random.Random(201)
    # The green van: deep subterranean drop + low dread braam
    van = silence(2.8)
    mix(van, taiko(rng, 36.0, 2.0), 0.0, 1.0)
    mix(van, horror_braam(note(-5), 2.5, rng), 0.02, 0.85)
    save("sfx_sting_van", dark_reverb(van, 0.35), 0.72)

    # 14:20 message: deep horror heart-drop + dark cello stab (no shrill metallic clang)
    msg = silence(2.4)
    mix(msg, taiko(rng, 40.0, 1.5), 0.0, 1.0)
    mix(msg, horror_braam(note(-2), 2.1, rng), 0.0, 0.85)
    save("sfx_sting_message", dark_reverb(msg, 0.35), 0.72)

    # News alert: hollow subterranean toll that sinks into dread
    al = silence(3.2)
    mix(al, taiko(rng, 34.0, 2.5), 0.0, 0.95)
    mix(al, horror_braam(note(-7), 3.0, rng), 0.05, 0.85)
    save("sfx_sting_alert", dark_reverb(al, 0.4), 0.70)

    # Spotted by a guard: heavy low impact + deep sub-brass surge
    sp = silence(2.0)
    mix(sp, taiko(rng, 44.0, 1.3), 0.0, 1.0)
    mix(sp, horror_braam(note(0), 1.7, rng), 0.0, 0.9)
    save("sfx_sting_spotted", dark_reverb(sp, 0.3), 0.75)

    # The shot on the rooftop: one deafening crack + sub shockwave that cuts off into silence
    sh = silence(1.2)
    crack = envelope(lowpass_warm(noise(0.06, rng), 0.25), 0.0002, 2.2)
    boom = taiko(rng, 34.0, 0.9)
    mix(sh, crack, 0.0, 1.0)
    mix(sh, boom, 0.0, 1.0)
    cut = int(0.85 * SR)
    for i in range(cut, len(sh)):
        sh[i] = 0.0
    save("sfx_sting_shot", sh, 0.92)


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
        coefficient = 0.003 + 0.012 * gust
        last = last + coefficient * (value - last)
        last2 = last2 + coefficient * (last - last2)
        out[index] = last2 * (0.35 + 0.65 * gust)
    save("amb_wind", loopable(out, 2.0), 0.42)


def amb_drone():
    rng = random.Random(34)
    out = deep_horror_drone(note(-2), 20.0, rng, tension=0.6)
    save("amb_drone", loopable(out, 2.0), 0.42)


def amb_city():
    """A day in the city: far traffic rumble, warm low-passed."""
    rng = random.Random(32)
    seconds = 20.0
    low = one_pole(one_pole(noise(seconds, rng), 0.010), 0.010)
    mid = one_pole(noise(seconds, rng), 0.04)
    out = [low[i] * 3.0 * (0.8 + 0.2 * math.sin(TWO_PI * i / SR / 6.0)) + mid[i] * 0.03 for i in range(len(low))]
    save("amb_city", loopable(out, 2.0), 0.35)


def amb_room():
    rng = random.Random(33)
    low = one_pole(one_pole(noise(14.0, rng), 0.005), 0.005)
    save("amb_room", loopable(low, 2.0), 0.16)


def loop_heartbeat():
    """Near the van: deep muffled heartbeat."""
    rng = random.Random(41)
    beat = 60.0 / 74.0
    track = silence(beat * 6)
    for index in range(6):
        mix(track, dum(rng, 48.0, 0.36), index * beat, 1.0)
        mix(track, dum(rng, 42.0, 0.32), index * beat + 0.27, 0.62)
    save("loop_heartbeat", one_pole(track[:int(beat * 6 * SR)], 0.10), 0.8)


def loop_engine():
    """A small motorcycle, somewhere close."""
    rng = random.Random(42)
    seconds = 3.0
    count = int(seconds * SR)
    out = [0.0] * count
    phase = 0.0
    rough = one_pole(noise(seconds, rng), 0.22)
    for index in range(count):
        t = index / SR
        freq = 58.0 + 3.5 * math.sin(TWO_PI * t / 1.5)
        phase += freq / SR
        if phase >= 1.0:
            phase -= 1.0
        firing = math.sin(TWO_PI * phase) * (0.6 + 0.4 * math.sin(TWO_PI * freq * 0.5 * t))
        out[index] = firing + rough[index] * 0.35 * abs(firing)
    save("loop_engine", loopable(one_pole(out, 0.18), 0.2), 0.5)


# ---------------------------------------------------------------------------------------------
# Effects
# ---------------------------------------------------------------------------------------------

def sfx_type():
    """One soft key: gentle, low-passed felt tick."""
    for variant in range(3):
        rng = random.Random(50 + variant)
        click = envelope(one_pole(noise(0.025, rng), 0.22), 0.001, 3.5)
        body = [math.sin(TWO_PI * (420.0 + 45.0 * variant) * i / SR) * math.exp(-i / SR * 240.0) for i in range(int(0.025 * SR))]
        save("sfx_type{}".format(variant + 1), [click[i] * 0.5 + body[i] * 0.35 for i in range(len(click))], 0.25)


def thump(rng, pitch, seconds, tone=0.15):
    out = dum(rng, pitch, seconds)
    mix(out, envelope(one_pole(noise(0.05, rng), tone), 0.001, 3.0), 0.0, 0.5)
    return out


def sfx_knock():
    rng = random.Random(61)
    track = silence(1.0)
    for index, at in enumerate((0.0, 0.19, 0.38)):
        mix(track, thump(rng, 145.0, 0.16, 0.2), at, 1.0 - 0.12 * index)
    save("sfx_knock", track, 0.65)


def sfx_door():
    rng = random.Random(62)
    track = silence(0.9)
    mix(track, thump(rng, 88.0, 0.4, 0.12), 0.05, 1.0)
    mix(track, envelope(one_pole(noise(0.03, rng), 0.35), 0.001, 3.0), 0.0, 0.4)
    save("sfx_door", track, 0.65)


def sfx_beep():
    """No service."""
    track = silence(1.5)
    for index, freq in enumerate((520.0, 480.0, 440.0)):
        count = int(0.28 * SR)
        tone = [math.sin(TWO_PI * freq * i / SR) * min(1.0, i / 300.0) * min(1.0, (count - i) / 300.0) for i in range(count)]
        mix(track, tone, index * 0.38, 0.5)
    save("sfx_beep", track, 0.35)


def sfx_static():
    rng = random.Random(63)
    raw = lowpass_warm(noise(2.0, rng), 0.08)
    out = [raw[i] * (0.5 + 0.5 * math.sin(TWO_PI * 7.0 * i / SR)) for i in range(len(raw))]
    save("sfx_static", out, 0.28)


def sfx_shots():
    """Far off, down the street. Three, then more."""
    rng = random.Random(64)
    track = silence(4.5)
    for at, gain in ((0.0, 1.0), (0.42, 0.8), (0.7, 0.9), (1.9, 0.6), (2.15, 0.7), (2.3, 0.5)):
        crack = envelope(lowpass_warm(noise(0.05, rng), 0.25), 0.0003, 2.0)
        tail = envelope(one_pole(noise(1.3, rng), 0.04), 0.002, 3.0)
        mix(track, crack, at, gain)
        mix(track, tail, at + 0.01, gain * 0.9)
    save("sfx_shots", track, 0.85)


def sfx_notify():
    """A phone vibrating on a wooden desk (low muffled buzz, not harsh square wave)."""
    track = silence(0.9)
    for at in (0.0, 0.34):
        count = int(0.22 * SR)
        buzz = [math.sin(TWO_PI * 135.0 * i / SR) * min(1.0, i / 350.0) * min(1.0, (count - i) / 600.0) for i in range(count)]
        mix(track, one_pole(buzz, 0.08), at, 1.0)
    save("sfx_notify", track, 0.38)


def sfx_bird():
    """One bird, at sunrise."""
    rng = random.Random(65)
    track = silence(5.0)
    t = 0.3
    for _ in range(4):
        for _ in range(rng.randint(2, 3)):
            seconds = rng.uniform(0.08, 0.14)
            count = int(seconds * SR)
            f0 = rng.uniform(1800.0, 2400.0)
            f1 = f0 + rng.uniform(-400.0, 600.0)
            phase = 0.0
            tone = [0.0] * count
            for i in range(count):
                w = i / count
                phase += TWO_PI * (f0 + (f1 - f0) * w) / SR
                tone[i] = math.sin(phase) * math.sin(math.pi * w)
            mix(track, tone, t, 0.4)
            t += seconds + rng.uniform(0.05, 0.1)
        t += rng.uniform(0.7, 1.2)
    save("sfx_bird", track[:int(5.0 * SR)], 0.25)


def sfx_saved():
    """A clip is saved: soft low chime."""
    count = int(0.35 * SR)
    tone = [math.sin(TWO_PI * 587.33 * i / SR) * math.exp(-i / SR * 12.0) * min(1.0, i / 120.0) for i in range(count)]
    save("sfx_saved", tone, 0.25)


def sfx_caught():
    rng = random.Random(66)
    track = silence(1.4)
    mix(track, taiko(rng, 42.0, 1.2), 0.0, 1.0)
    mix(track, horror_braam(note(-2), 1.2, rng), 0.0, 0.75)
    save("sfx_caught", track, 0.75)


def main():
    print("Generating dark horror/dread audio into {}".format(OUT_DIR))
    for build in (music_setar, music_setar_resolved, music_daf, music_chase, music_sunrise,
                  amb_wind, amb_drone, amb_city, amb_room, loop_heartbeat, loop_engine,
                  sfx_type, sfx_knock, sfx_door, sfx_beep, sfx_static, sfx_shots, sfx_notify,
                  sfx_bird, sfx_saved, sfx_caught, build_thriller_cues, build_stingers):
        build()
    print("Done.")


if __name__ == "__main__":
    main()
