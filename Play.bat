@echo off
rem Starts the game directly (no editor UI), using the most recently compiled game code.
rem Extra arguments are passed through, e.g.  Play.bat -chscene=2.6   to jump to a scene.
call "%~dp0Scripts\ue_env.bat" || exit /b 1
set "CH_EXE=%UE_EDITOR%"
for /f "usebackq delims=" %%i in (`powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Scripts\pick_editor.ps1"`) do set "CH_EXE=%%i"
start "" "%CH_EXE%" "%CH_PROJECT%" -game -windowed -ResX=1600 -ResY=900 -nosplash -log %*
