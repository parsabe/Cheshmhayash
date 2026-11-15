<#
Renders the trailer straight from the game, with no screen recorder, then cuts it to music.

    powershell -ExecutionPolicy Bypass -File Scripts\render_trailer.ps1 [-Team "TeamName"]

1. runs trailer mode offscreen in the editor build at a fixed 30 fps, 1920x1080, saving every frame
   with its UI (-chdump -benchmark -fps=30) to Saved\Dump   (about 15 minutes)
2. encodes the frames to Trailer\recording.mp4
3. runs Scripts\make_trailer.ps1 on it -> Trailer\<Team>_CheshmHayash_Trailer.mp4
Needs the editor target built (Development) and ffmpeg (winget install Gyan.FFmpeg).
The game's own sound is not captured this way; the trailer uses the music only.
#>
param(
    [string]$Team = "TeamName",
    [int]$MaxFrames = 3200,
    [int]$TimeoutSec = 3600
)
$ErrorActionPreference = "Continue"
$Root = Split-Path -Parent $PSScriptRoot
$env:Path = [Environment]::GetEnvironmentVariable('Path', 'Machine') + ';' + [Environment]::GetEnvironmentVariable('Path', 'User') + ';' + $env:Path

$engine = $env:UE_ROOT
if (-not $engine) { $engine = 'C:\Program Files\Epic Games\UE_5.8' }
$ue = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor.exe'
if (-not (Test-Path $ue)) { throw "UnrealEditor.exe not found at $ue (set UE_ROOT)" }

$dump = Join-Path $Root 'Saved\Dump'
if (Test-Path $dump) { Remove-Item -Recurse -Force $dump }
$ueArgs = @("`"$Root\Cheshmhayash.uproject`"", '-game', '-RenderOffScreen', '-windowed', '-ForceRes', '-ResX=1920', '-ResY=1080',
          '-nosplash', '-unattended', '-nosound', '-chtrailer', '-benchmark', '-fps=30', '-chdump')
Write-Host "Rendering trailer mode frame by frame (about 15 minutes)..."
$p = Start-Process -FilePath $ue -ArgumentList $ueArgs -PassThru
$t0 = Get-Date
# trailer mode ends on its title and then holds black; stop once enough frames exist
while (-not $p.HasExited) {
    Start-Sleep -Seconds 5
    $n = 0
    if (Test-Path $dump) { $n = (Get-ChildItem $dump -File).Count }
    if ($n -ge $MaxFrames -or ((Get-Date) - $t0).TotalSeconds -gt $TimeoutSec) { Stop-Process -Id $p.Id -Force; break }
}
Start-Sleep -Seconds 2
$count = (Get-ChildItem $dump -File -ErrorAction SilentlyContinue).Count
Write-Host "$count frames"
if ($count -lt 2000) { throw "Too few frames ($count). See Saved\Logs." }

$recording = Join-Path $Root 'Trailer\recording.mp4'
New-Item -ItemType Directory -Force (Split-Path $recording) | Out-Null
& ffmpeg -hide_banner -loglevel error -y -framerate 30 -i (Join-Path $dump 'frame_%05d.png') -frames:v ($count - 1) `
    -c:v libx264 -preset veryfast -crf 14 -pix_fmt yuv420p "$recording"
if ($LASTEXITCODE -ne 0) { throw "ffmpeg could not encode the frames" }

& (Join-Path $PSScriptRoot 'make_trailer.ps1') -Recording $recording -Team $Team
