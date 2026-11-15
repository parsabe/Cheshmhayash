@echo off
rem Starts the game straight into trailer mode, fullscreen 1920x1080, for screen recording.
rem There are 6 seconds of black before the first shot: start your recorder then.
rem The trailer ends on its title and then stays black: stop recording, then Esc > Quit.
set "CH_ROOT=%~dp0.."
set "CH_PACKAGED=%CH_ROOT%\Packaged\Windows\Cheshmhayash.exe"
if exist "%CH_PACKAGED%" (
  start "" "%CH_PACKAGED%" -chtrailer -fullscreen -ResX=1920 -ResY=1080
) else (
  call "%CH_ROOT%\Play.bat" -chtrailer -fullscreen -ResX=1920 -ResY=1080
)
