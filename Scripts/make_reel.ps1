<#
Makes the vertical Instagram Reel (1080x1920) from the 40-second trailer.

    powershell -ExecutionPolicy Bypass -File Scripts\make_reel.ps1 -Song "C:\path\to\song.mp3"

  -Song     the music (its first -Length seconds are used; the last 2 s fade out)
  -Trailer  the 16:9 trailer (default: Trailer\<Team>_CheshmHayash_Trailer.mp4)
  -Start    where in the song to begin, in seconds (default 0)
  -Length   seconds (default 40)
  -Team     your team name, as in make_trailer.ps1 (default: TeamName)

The whole 16:9 picture stays visible (the phone screen and the Farsi slogans are not cropped):
it sits in the middle, over a blurred, darkened copy of itself that fills the tall frame, and
Trailer\reel_overlay.png adds the title above and the tagline below, inside Instagram's safe
zones. The overlay is made from Trailer\reel\overlay.html (open it in a browser to change it,
then render it again at 1080x1920 with a transparent background).

Writes Trailer\<Team>_CheshmHayash_Reel.mp4: H.264 1080x1920, 30 fps, AAC 48 kHz.
#>
param(
    [Parameter(Mandatory = $true)][string]$Song,
    [string]$Trailer = "",
    [double]$Start = 0,
    [double]$Length = 40,
    [string]$Team = "TeamName"
)
$ErrorActionPreference = "Continue"
$Root = Split-Path -Parent $PSScriptRoot
if (-not $Trailer) { $Trailer = Join-Path $Root ("Trailer\{0}_CheshmHayash_Trailer.mp4" -f $Team) }
$Overlay = Join-Path $Root "Trailer\reel_overlay.png"
$env:Path = [Environment]::GetEnvironmentVariable('Path', 'Machine') + ';' + [Environment]::GetEnvironmentVariable('Path', 'User') + ';' + $env:Path
if (-not (Get-Command ffmpeg -ErrorAction SilentlyContinue)) { throw "ffmpeg not found. Install it once with:  winget install Gyan.FFmpeg" }
foreach ($p in $Trailer, $Song, $Overlay) { if (-not (Test-Path -LiteralPath $p)) { throw "Not found: $p" } }

$inv = [Globalization.CultureInfo]::InvariantCulture
function F([double]$v) { return $v.ToString("0.000", $inv) }
$fadeOut = [math]::Max(0, $Length - 2.0)

# the video band: 1080x608 at y 656 (matches the clear band in the overlay)
$graph = @(
    "[0:v]fps=30,split=2[a][b]",
    # background: the same footage, zoomed to fill, blurred (cheaply, at quarter size) and darkened
    "[a]scale=-2:1920,crop=1080:1920,scale=270:480,gblur=sigma=7,scale=1080:1920,eq=brightness=-0.08:saturation=1.25[bg]",
    "[b]scale=1080:608:flags=lanczos,setsar=1[fg]",
    "[bg][fg]overlay=0:656[base]",
    ("[base][1:v]overlay=0:0,format=yuv420p,trim=duration=" + (F $Length) + ",setpts=PTS-STARTPTS[v]"),
    ("[2:a]atrim=start=" + (F $Start) + ":duration=" + (F $Length) + ",asetpts=PTS-STARTPTS,aformat=sample_rates=48000:channel_layouts=stereo,afade=t=in:st=0:d=0.3,afade=t=out:st=" + (F $fadeOut) + ":d=2[aout]")
) -join ";"

$out = Join-Path $Root ("Trailer\{0}_CheshmHayash_Reel.mp4" -f $Team)
Write-Host "Rendering $out ..."
& ffmpeg -hide_banner -y -loglevel error -stats -i "$Trailer" -loop 1 -i "$Overlay" -i "$Song" -filter_complex $graph `
    -map "[v]" -map "[aout]" -t (F $Length) `
    -c:v libx264 -preset slow -crf 18 -profile:v high -pix_fmt yuv420p -r 30 `
    -c:a aac -b:a 192k -ar 48000 -movflags +faststart "$out"
if ($LASTEXITCODE -ne 0) { throw "ffmpeg failed (exit $LASTEXITCODE)" }
$final = [double]::Parse((& ffprobe -v error -show_entries format=duration -of csv=p=0 "$out").Trim(), $inv)
Write-Host ("Done: {0}  ({1:0.00} s)" -f $out, $final)
