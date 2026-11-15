@echo off
rem Builds Packaged\CheshmHayash-Setup.exe from the packaged game (run Scripts\package.bat first).
rem Needs Inno Setup 6:  winget install JRSoftware.InnoSetup
setlocal
set "ISCC="
for %%P in ("%LOCALAPPDATA%\Programs\Inno Setup 6\ISCC.exe" "%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe" "%ProgramFiles%\Inno Setup 6\ISCC.exe") do (
  if exist %%P set "ISCC=%%~P"
)
if not defined ISCC (
  echo Inno Setup 6 not found. Install it once with:  winget install JRSoftware.InnoSetup
  exit /b 1
)
if not exist "%~dp0..\Packaged\Windows\Cheshmhayash.exe" (
  echo No packaged game found. Run Scripts\package.bat first.
  exit /b 1
)
rem test runs of the packaged game leave a Saved folder; it must not go into the installer
if exist "%~dp0..\Packaged\Windows\Cheshmhayash\Saved" rmdir /s /q "%~dp0..\Packaged\Windows\Cheshmhayash\Saved"
"%ISCC%" /Q "%~dp0installer.iss"
if errorlevel 1 exit /b 1
echo.
echo Done: %~dp0..\Packaged\CheshmHayash-Setup.exe
