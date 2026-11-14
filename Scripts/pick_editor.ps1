# Prints the editor executable that matches the most recently built game module.
# The game module can be built as Development (UnrealEditor.exe) or, when the Unreal Editor was
# open and locking that DLL, as DebugGame (UnrealEditor-Win64-DebugGame.exe).
$root = Split-Path -Parent $PSScriptRoot
$ue = $env:UE_ROOT
if (-not $ue) { $ue = 'C:\Program Files\Epic Games\UE_5.8' }
$bin = Join-Path $root 'Binaries\Win64'
$dbg = Get-Item (Join-Path $bin 'UnrealEditor-Cheshmhayash-Win64-DebugGame.dll') -ErrorAction SilentlyContinue
$dev = Get-Item (Join-Path $bin 'UnrealEditor-Cheshmhayash.dll') -ErrorAction SilentlyContinue
if ($dbg -and (-not $dev -or $dbg.LastWriteTime -gt $dev.LastWriteTime)) {
    Join-Path $ue 'Engine\Binaries\Win64\UnrealEditor-Win64-DebugGame.exe'
} else {
    Join-Path $ue 'Engine\Binaries\Win64\UnrealEditor.exe'
}
