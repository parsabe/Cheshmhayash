"""Generates 3 versions of the 55-second trailer score with fal -> Trailer/template_v1.mp3 .. v3,
and makes v1 the Trailer/template.mp3 (pick another with --pick 2).

    set FAL_KEY=your-key
    "<UE>\\Engine\\Binaries\\ThirdParty\\Python3\\Win64\\python.exe" Scripts\\gen_trailer_music_fal.py
    ...                                                              Scripts\\gen_trailer_music_fal.py --pick 2

One generation cannot follow a 6-part structure (and the model stops at 47 s), so each version
is made of six sections generated separately and joined with ffmpeg crossfades, cut to exactly
55.000 s. The 35-40 s silence is joined with hard cuts on purpose. The key comes from FAL_KEY and
is never written anywhere; only the prompts below are sent. Needs ffmpeg on the PATH.
"""
import json
import os
import shutil
import subprocess
import sys
import tempfile
import time
import urllib.error
import urllib.request

# ---- change this if fal renames the model ----------------------------------------------------
MODEL = "fal-ai/stable-audio"
MAX_SECONDS = 47
# -----------------------------------------------------------------------------------------------

STYLE = ("dark cinematic thriller drama trailer score with Persian instruments (setar, kamancheh, ney, daf), "
         "no vocals, no lyrics, no choir, instrumental")

# (seconds, crossfade into the next section, prompt)
SECTIONS = [
    (5.0, 0.35, "sudden start with one massive taiko drum hit, then fast pulsing tension, ticking percussion and a "
                "rising sub-bass, a chase through night streets, urgent, frightening"),
    (15.0, 0.6, "low ominous sub-bass drones, dissonant tense strings, a distant heavily reverbed Persian setar "
                "playing a few sparse sad notes, dread, a police van in a dark street, a prison gate"),
    (15.0, 0.05, "rising deep war drums and Persian daf getting faster, low brass blasts, strings climbing, "
                 "building fast to a huge climax, protest, fire in the streets, epic and terrifying"),
    (5.0, 0.05, "complete silence except a slow deep human heartbeat, nothing else, no music"),
    (12.0, 0.4, "slow overwhelming warm and sorrowful theme, a breathy Persian ney melody over rich glowing strings "
                "and soft kamancheh, sunrise over mountains, grief turning into light, beautiful"),
    (3.0, 0.0, "one final enormous deep drum hit and low brass, then a long ringing reverb fade to silence"),
]
TOTAL = 55.0
VERSIONS = 3

QUEUE = "https://queue.fal.run/"
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
OUT_DIR = os.path.join(ROOT, "Trailer")


def call(url, key, payload=None):
    data = json.dumps(payload).encode("utf-8") if payload is not None else None
    request = urllib.request.Request(url, data=data, method="POST" if data is not None else "GET")
    request.add_header("Authorization", "Key " + key)
    request.add_header("Content-Type", "application/json")
    with urllib.request.urlopen(request, timeout=120) as response:
        return json.loads(response.read().decode("utf-8"))


def find_url(value):
    if isinstance(value, dict):
        url = value.get("url")
        if isinstance(url, str) and url.startswith("http"):
            return url
        for item in value.values():
            found = find_url(item)
            if found:
                return found
    elif isinstance(value, list):
        for item in value:
            found = find_url(item)
            if found:
                return found
    return None


def generate(prompt, seconds, key, path):
    queued = call(QUEUE + MODEL, key, {"prompt": prompt, "seconds_total": int(min(MAX_SECONDS, max(1, round(seconds + 0.49)))), "steps": 100})
    for _ in range(180):
        status = call(queued["status_url"], key).get("status")
        if status == "COMPLETED":
            break
        if status in ("FAILED", "ERROR", "CANCELLED"):
            raise RuntimeError("generation " + status)
        time.sleep(2.0)
    else:
        raise RuntimeError("timed out")
    url = find_url(call(queued["response_url"], key))
    if not url:
        raise RuntimeError("no audio in the result")
    with urllib.request.urlopen(url, timeout=300) as response, open(path, "wb") as out:
        out.write(response.read())


def join(parts, fades, out_path, ffmpeg):
    """parts[i] is trimmed to its section length (+ its crossfade) and joined with acrossfade."""
    inputs = []
    filters = []
    for index, (path, (seconds, fade, _)) in enumerate(zip(parts, SECTIONS)):
        inputs += ["-i", path]
        filters.append("[{0}:a]aformat=sample_rates=48000:channel_layouts=stereo,atrim=0:{1:.3f},asetpts=PTS-STARTPTS[s{0}]".format(index, seconds + fade))
    last = "s0"
    for index in range(1, len(parts)):
        fade = max(0.01, SECTIONS[index - 1][1])
        label = "j{}".format(index)
        filters.append("[{0}][s{1}]acrossfade=d={2:.3f}:c1=tri:c2=tri[{3}]".format(last, index, fade, label))
        last = label
    filters.append("[{0}]atrim=0:{1:.3f},afade=t=out:st={2:.3f}:d=0.6,loudnorm=I=-14:TP=-1.0:LRA=11[out]".format(last, TOTAL, TOTAL - 0.6))
    subprocess.run([ffmpeg, "-y", "-loglevel", "error"] + inputs + ["-filter_complex", ";".join(filters), "-map", "[out]", "-ar", "48000", "-b:a", "256k", out_path], check=True)


def main():
    key = os.environ.get("FAL_KEY", "").strip()
    pick = 1
    if "--pick" in sys.argv:
        pick = int(sys.argv[sys.argv.index("--pick") + 1])
    os.makedirs(OUT_DIR, exist_ok=True)
    if "--pick" in sys.argv and not key:
        source = os.path.join(OUT_DIR, "template_v{}.mp3".format(pick))
        shutil.copyfile(source, os.path.join(OUT_DIR, "template.mp3"))
        print("Trailer/template.mp3 <- {}".format(os.path.basename(source)))
        return 0
    if not key:
        print("FAL_KEY is not set. Set it in this window and run again:  set FAL_KEY=your-key")
        return 1
    ffmpeg = shutil.which("ffmpeg")
    if not ffmpeg:
        print("ffmpeg is not on the PATH.")
        return 1
    # each section is asked for its length plus the crossfade into the next one
    made = []
    for version in range(1, VERSIONS + 1):
        print("Version {}".format(version))
        work = tempfile.mkdtemp(prefix="ch_trailer_")
        parts = []
        try:
            for index, (seconds, fade, prompt) in enumerate(SECTIONS):
                path = os.path.join(work, "s{}.wav".format(index))
                print("  section {} ({:.0f} s)...".format(index + 1, seconds), flush=True)
                generate(prompt + ". " + STYLE, seconds + fade, key, path)
                parts.append(path)
            out = os.path.join(OUT_DIR, "template_v{}.mp3".format(version))
            join(parts, [s[1] for s in SECTIONS], out, ffmpeg)
            made.append(out)
            print("  -> {}".format(out))
        except (RuntimeError, urllib.error.URLError, OSError, subprocess.CalledProcessError) as error:
            print("  FAILED: {}".format(error))
        finally:
            shutil.rmtree(work, ignore_errors=True)
    if made:
        chosen = os.path.join(OUT_DIR, "template_v{}.mp3".format(pick))
        if not os.path.isfile(chosen):
            chosen = made[0]
        shutil.copyfile(chosen, os.path.join(OUT_DIR, "template.mp3"))
        print("Trailer/template.mp3 <- {}  (listen to all, then: --pick N)".format(os.path.basename(chosen)))
    return 0 if made else 1


if __name__ == "__main__":
    sys.exit(main())
