"""Generates the music (and a few ambiences) with fal, into RawAudio/fal/*.wav.

    set FAL_KEY=your-key
    Scripts\\gen_audio.bat fal

Reads Scripts/audio_list.json (name + kind + prompt + seconds). Each finished file replaces the
procedural track of the same name the next time audio is imported (RawAudio/fal/ wins).
Standard library only. The key is read from the FAL_KEY environment variable and never written
anywhere. Only the prompts in audio_list.json are sent to the service.

If a model id below has changed on fal, edit it here; nothing else needs to change.
"""
import json
import os
import sys
import time
import urllib.error
import urllib.request

# ---- change these if fal renames a model ------------------------------------------------------
MUSIC_MODEL = "fal-ai/stable-audio"          # text -> music
SFX_MODEL = "fal-ai/stable-audio"            # text -> sound effect / ambience
# Maps our fields onto the model's input. Stable Audio: prompt, seconds_total (<= 47), steps.
MAX_SECONDS = 47


def model_input(track):
    return {"prompt": track["prompt"], "seconds_total": min(MAX_SECONDS, int(track.get("seconds", 30))), "steps": 100}
# -----------------------------------------------------------------------------------------------

QUEUE = "https://queue.fal.run/"
HERE = os.path.dirname(os.path.abspath(__file__))
OUT_DIR = os.path.abspath(os.path.join(HERE, "..", "RawAudio", "fal"))


def call(url, key, payload=None):
    data = json.dumps(payload).encode("utf-8") if payload is not None else None
    request = urllib.request.Request(url, data=data, method="POST" if data is not None else "GET")
    request.add_header("Authorization", "Key " + key)
    request.add_header("Content-Type", "application/json")
    with urllib.request.urlopen(request, timeout=120) as response:
        return json.loads(response.read().decode("utf-8"))


def find_audio_url(value):
    """The first URL in the result that looks like an audio file."""
    if isinstance(value, dict):
        url = value.get("url")
        if isinstance(url, str) and url.startswith("http"):
            return url
        for item in value.values():
            found = find_audio_url(item)
            if found:
                return found
    elif isinstance(value, list):
        for item in value:
            found = find_audio_url(item)
            if found:
                return found
    return None


def generate(track, key):
    model = MUSIC_MODEL if track.get("kind", "music") == "music" else SFX_MODEL
    queued = call(QUEUE + model, key, model_input(track))
    status_url = queued.get("status_url")
    response_url = queued.get("response_url")
    if not status_url or not response_url:
        raise RuntimeError("unexpected reply from fal: {}".format(queued))
    for _ in range(180):
        status = call(status_url, key).get("status")
        if status == "COMPLETED":
            break
        if status in ("FAILED", "ERROR", "CANCELLED"):
            raise RuntimeError("generation {}".format(status))
        time.sleep(2.0)
    else:
        raise RuntimeError("timed out")
    url = find_audio_url(call(response_url, key))
    if not url:
        raise RuntimeError("no audio file in the result")
    extension = os.path.splitext(url.split("?")[0])[1].lower() or ".wav"
    path = os.path.join(OUT_DIR, track["name"] + extension)
    with urllib.request.urlopen(url, timeout=300) as response, open(path, "wb") as out:
        out.write(response.read())
    return path


def main():
    key = os.environ.get("FAL_KEY", "").strip()
    if not key:
        print("FAL_KEY is not set. Nothing generated; the game keeps its procedural audio.")
        print("  set FAL_KEY=your-key   then run this again.")
        return 1
    with open(os.path.join(HERE, "audio_list.json"), "r", encoding="utf-8") as handle:
        tracks = json.load(handle)["tracks"]
    only = set(sys.argv[1:])
    os.makedirs(OUT_DIR, exist_ok=True)
    failed = 0
    for track in tracks:
        if only and track["name"] not in only:
            continue
        print("{} ...".format(track["name"]), flush=True)
        try:
            path = generate(track, key)
            print("  saved {}".format(path))
            if not path.lower().endswith(".wav"):
                print("  NOTE: not a .wav; Unreal imports .wav only. Convert it, or pick a model that returns wav.")
        except (RuntimeError, urllib.error.URLError, OSError, ValueError) as error:
            failed += 1
            print("  FAILED: {}".format(error))
    print("Done ({} failed). Now run Scripts\\setup_assets.bat to import.".format(failed))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
