@echo off
rem Generates the game's audio into RawAudio\ using the Python that ships with Unreal.
rem   Scripts\gen_audio.bat        procedural sounds and music (no key needed)
rem   Scripts\gen_audio.bat fal    music from fal (needs FAL_KEY set in this window)
rem Afterwards run Scripts\setup_assets.bat (editor closed) to import them.
call "%~dp0ue_env.bat" || exit /b 1
set "UE_PYTHON=%UE_ROOT%\Engine\Binaries\ThirdParty\Python3\Win64\python.exe"
if /i "%~1"=="fal" (
  "%UE_PYTHON%" "%~dp0gen_audio_fal.py" %2 %3 %4 %5 %6
) else (
  "%UE_PYTHON%" "%~dp0gen_audio_procedural.py"
)
