@echo off
rem Compiles the game module for the editor (Development). Close the editor/game first.
call "%~dp0ue_env.bat" || exit /b 1
call "%UE_BUILD%" CheshmhayashEditor Win64 Development -Project="%CH_PROJECT%" -WaitMutex -NoHotReload %*
