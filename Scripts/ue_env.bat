@echo off
rem Shared environment for the helper scripts. Override UE_ROOT if your engine lives elsewhere.
if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
set "CH_ROOT=%~dp0.."
set "CH_PROJECT=%CH_ROOT%\Cheshmhayash.uproject"
set "UE_EDITOR=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor.exe"
set "UE_EDITOR_CMD=%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "UE_BUILD=%UE_ROOT%\Engine\Build\BatchFiles\Build.bat"
set "UE_UAT=%UE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat"
if not exist "%UE_EDITOR%" (
  echo [ERROR] Unreal Engine not found at "%UE_ROOT%". Set UE_ROOT and try again.
  exit /b 1
)
exit /b 0
