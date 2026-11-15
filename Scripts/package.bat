@echo off
rem Builds, cooks and packages the game for Windows 64-bit (Development) into Packaged\Windows\.
rem Close the game first. Takes 10-30 minutes the first time. The result to upload is the zip
rem written next to it: Packaged\CheshmHayash-Win64.zip
call "%~dp0ue_env.bat" || exit /b 1
set "CH_OUT=%CH_ROOT%\Packaged"

call "%UE_UAT%" BuildCookRun ^
  -project="%CH_PROJECT%" ^
  -noP4 -utf8output ^
  -platform=Win64 -clientconfig=Development ^
  -build -cook -stage -pak -iostore -package -archive -prereqs ^
  -archivedirectory="%CH_OUT%" ^
  -nocompileeditor -unattended
if errorlevel 1 (
  echo.
  echo [ERROR] Packaging failed. The reason is a few lines above "BUILD FAILED".
  exit /b 1
)

if exist "%CH_OUT%\CheshmHayash-Win64.zip" del "%CH_OUT%\CheshmHayash-Win64.zip"
powershell -NoProfile -Command "Compress-Archive -Path '%CH_OUT%\Windows\*' -DestinationPath '%CH_OUT%\CheshmHayash-Win64.zip'"
echo.
echo Done. Game:   %CH_OUT%\Windows\Cheshmhayash.exe
echo       Upload: %CH_OUT%\CheshmHayash-Win64.zip
