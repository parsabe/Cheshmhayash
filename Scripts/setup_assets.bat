@echo off
rem Creates maps, master materials, font assets and imports audio, then exits.
rem Close the editor and the game first. Takes about a minute.
call "%~dp0ue_env.bat" || exit /b 1
"%UE_EDITOR_CMD%" "%CH_PROJECT%" -ExecutePythonScript="%~dp0setup_all.py" -RenderOffScreen -unattended -nosplash -nopause -stdout
echo.
echo setup_assets finished (exit code %ERRORLEVEL%). Look for "[setup_all] ALL DONE" above.
