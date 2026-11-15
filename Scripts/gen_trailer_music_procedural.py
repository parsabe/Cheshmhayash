"""Builds the trailer score procedurally (no key, no network).

    "<UE>\\Engine\\Binaries\\ThirdParty\\Python3\\Win64\\python.exe" Scripts\\gen_trailer_music_procedural.py [--length 40]

  --length 40 (default): Trailer/template_procedural_40.mp3, and Trailer/template_40.mp3 if none exists
  --length 55:           Trailer/template_procedural.mp3, and Trailer/template.mp3 if none exists

The 40-second cut has the same six sections, shorter: 0-3.5 opening hit (the van), 3.5-12.5 drones
(phone, gate), 12.5-24.5 build (chants, molotov, Mom, crowd), 24.5-27.5 silence (the shot),
27.5-37 sunrise theme (sunrise, people running, the grave), 37-40 title. The 55-second layout:

The structure follows the trailer's shot order exactly (see Scripts/make_trailer.ps1):
   0-5   sudden start: a heavy drum hit, pulsing tension            (the chase)
   5-20  low drones, dissonant strings, a distant reverbed setar    (van, arrest message, gate)
  20-35  rising deep drums and low brass, building fast            (chants, molotov, Mom, crowd)
  35-40  abrupt silence, only a heartbeat                           (the rooftop, the shot)
  40-52  slow, warm, sorrowful theme: ney and strings              (sunrise, cemetery)
  52-55  one last deep hit on the title, a ringing fade
No vocals. Needs ffmpeg on the PATH for the mp3.
"""
import math
import os
import random
import shutil
import subprocess
import sys
import wave
from array import array

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_audio_procedural as g  # noqa: E402

SR = g.SR
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUT_DIR = os.path.join(ROOT, "Trailer")


def env_ramp(sound, start_gain, end_gain):
    count = len(sound)
    return [v * (start_gain + (end_gain - start_gain) * i / max(1, count - 1)) for i, v in enumerate(sound)]


def setar_pluck(freq, seconds, rng):
    """A plucked string (Karplus-Strong), two courses a hair apart, as on a setar."""
    out = [0.0] * int((seconds + 1.0) * SR)
    for detune, gain in ((1.0, 0.7), (1.004, 0.45)):
        period = max(2, int(SR / (freq * detune)))
        buf = [rng.uniform(-1.0, 1.0) for _ in range(period)]
        index = 0
        for n in range(len(out)):
            value = buf[index]
            nxt = index + 1 if index + 1 < period else 0
            buf[index] = 0.996 * 0.5 * (value + buf[nxt])
            out[n] += value * gain
            index = nxt
    return g.one_pole(out, 0.35)


# Section boundaries (seconds): end of the chase, the drones, the build, the silence, the theme, and the total.
# They match the shot lengths in Scripts/make_trailer.ps1 for each trailer length.
SECTIONS = {
    55: (5.0, 20.0, 35.0, 40.0, 52.0, 55.0),
    40: (3.5, 12.5, 24.5, 27.5, 37.0, 40.0),
}


def build(length=55):
    chase_end, drones_end, build_end, silence_end, theme_end, total = SECTIONS[length]
    rng = random.Random(1401)
    track = g.silence(total + 2.0)

    # ---- the chase. A heavy hit on frame one, then a pulse that will not let go
    g.mix(track, g.taiko(rng, 40.0, 2.2), 0.0, 1.0)
    g.mix(track, g.horror_braam(g.note(-12), 3.0, rng), 0.0, 0.7)
    pulse = 60.0 / 150.0
    t = 0.4
    while t < chase_end:
        g.mix(track, g.dum(rng, 58.0, 0.35), t, 0.55 + 0.1 * (t / chase_end))
        g.mix(track, g.tak(rng, 0.08), t + pulse * 0.5, 0.25)
        t += pulse
    g.mix(track, env_ramp(g.deep_horror_drone(g.note(-12), chase_end, rng, 0.8), 0.3, 0.8), 0.0, 0.5)

    # ---- the van, the message, the gate. Low drones, a dissonant cluster, a far setar
    span = drones_end - chase_end
    k = span / 15.0
    g.mix(track, g.deep_horror_drone(g.note(-12), span + 0.5, rng, 0.7), chase_end, 0.7)
    g.mix(track, g.dark_cello_swell(g.note(-7), span, rng, semitone_cluster=(1, 6)), chase_end, 0.45)
    setar = g.silence(span + 1.0)
    for at, step in ((0.6, 0), (2.0, 1), (3.1, 3), (5.4, 1.5), (7.2, 0), (9.6, 5), (10.8, 3), (12.6, 1.5), (13.9, 0)):
        if at * k < span - 0.4:
            g.mix(setar, setar_pluck(g.note(step), 2.6, rng), at * k, 0.5)
    g.mix(track, g.dark_reverb(g.one_pole(setar, 0.25), 0.55), chase_end, 0.55)
    t = chase_end
    while t < drones_end:
        g.mix(track, g.tak(rng, 0.06), t, 0.12)
        t += 0.5
    g.mix(track, g.horror_braam(g.note(-10), 4.0, rng), drones_end - 5.5 * k, 0.35)

    # ---- chants, molotov, Mom, the crowd. Drums gather speed, low brass, a fast build
    span = build_end - drones_end
    k = span / 15.0
    t = drones_end
    while t < build_end - 0.1:
        progress = (t - drones_end) / span
        bpm = 96.0 + 70.0 * progress
        beat = 60.0 / bpm
        g.mix(track, g.taiko(rng, 44.0, 0.9), t, 0.55 + 0.4 * progress)
        g.mix(track, g.tak(rng, 0.1, ring=True), t + beat * 0.5, 0.25 + 0.25 * progress)
        if progress > 0.5:
            g.mix(track, g.tak(rng, 0.07), t + beat * 0.25, 0.18)
            g.mix(track, g.tak(rng, 0.07), t + beat * 0.75, 0.18)
        t += beat
    for at, step in ((0.0, -12), (5.0, -10), (9.0, -9), (12.0, -7)):
        g.mix(track, g.horror_braam(g.note(step), 3.5, rng), drones_end + at * k, 0.6)
    rise = g.dark_cello_swell(g.note(-5), span, rng, semitone_cluster=(1, 7))
    g.mix(track, env_ramp(rise, 0.2, 1.0), drones_end, 0.6)
    g.mix(track, env_ramp(g.deep_horror_drone(g.note(-12), span, rng, 0.9), 0.4, 1.0), drones_end, 0.7)

    # ---- nothing. Only a heartbeat
    span = silence_end - build_end
    for index in range(int(build_end * SR), int(silence_end * SR)):
        track[index] = 0.0
    beat_track = g.dread_heartbeat_loop(62.0, span, rng, True, 0.9)
    g.mix(track, beat_track[:int(span * SR)], build_end + 0.05, 0.9)

    # ---- the sunrise. Warm strings, the ney, a resolving setar
    span = theme_end - silence_end
    k = span / 12.0
    strings = g.silence(span + 1.0)
    for step, gain in ((-12, 0.6), (-5, 0.45), (0, 0.4), (4, 0.3)):
        g.mix(strings, g.dark_cello_swell(g.note(step), span + 0.5, rng), 0.0, gain)
    g.mix(track, env_ramp(strings, 0.45, 1.0), silence_end, 1.5)
    t = silence_end + 0.6 * k
    for name, beats in (("A0", 1.4), ("D", 2.2), ("E", 1.0), ("F#", 2.4), ("E", 1.0), ("D", 3.4)):
        g.mix(track, g.ney_note(g.note(g.MAHUR[name]) * 2.0, beats * k * 0.95, rng), t, 0.6)
        t += beats * k
    for at, name in ((0.2, "D"), (3.4, "A0"), (6.6, "D"), (9.6, "F#")):
        g.mix(track, g.dark_reverb(setar_pluck(g.note(g.MAHUR[name]), 2.5, rng), 0.4), silence_end + at * k, 0.55)

    # ---- the title. One deep hit, then it rings away
    g.mix(track, g.taiko(rng, 36.0, 3.0), theme_end, 1.0)
    g.mix(track, g.horror_braam(g.note(-12), 3.0, rng), theme_end, 0.6)
    g.mix(track, g.dark_reverb(setar_pluck(g.note(0), 3.0, rng), 0.6), theme_end + 0.05, 0.4)

    # exactly the right length; a short fade at the very end so it never clicks
    track = track[:int(total * SR)]
    fade = int(0.8 * SR)
    for index in range(fade):
        track[-fade + index] *= 1.0 - index / fade
    peak = max(1e-9, max(abs(v) for v in track))
    return [v * 0.92 / peak for v in track]


def write_wav(path, track):
    data = array("h", (int(max(-1.0, min(1.0, v)) * 32767.0) for v in track))
    if sys.byteorder == "big":
        data.byteswap()
    with wave.open(path, "wb") as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(SR)
        out.writeframes(data.tobytes())


def main():
    length = 40
    if "--length" in sys.argv:
        length = int(sys.argv[sys.argv.index("--length") + 1])
    if length not in SECTIONS:
        print("--length must be one of: " + ", ".join(str(k) for k in SECTIONS))
        return 1
    suffix = "" if length == 55 else "_{}".format(length)
    os.makedirs(OUT_DIR, exist_ok=True)
    print("Building the {} s trailer score...".format(length))
    track = build(length)
    wav = os.path.join(OUT_DIR, "template_procedural{}.wav".format(suffix))
    write_wav(wav, track)
    mp3 = os.path.join(OUT_DIR, "template_procedural{}.mp3".format(suffix))
    ffmpeg = shutil.which("ffmpeg")
    if not ffmpeg:
        print("ffmpeg not found: kept the .wav only ({})".format(wav))
        return 1
    subprocess.run([ffmpeg, "-y", "-loglevel", "error", "-i", wav, "-ac", "2", "-ar", "48000", "-b:a", "256k", mp3], check=True)
    os.remove(wav)
    print("  {}".format(mp3))
    template = os.path.join(OUT_DIR, "template{}.mp3".format(suffix))
    if not os.path.isfile(template):
        shutil.copyfile(mp3, template)
        print("  {} (no fal version yet, so this one is the template)".format(template))
    return 0


if __name__ == "__main__":
    sys.exit(main())