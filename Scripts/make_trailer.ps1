<#
Cuts the jam trailer out of a screen recording of trailer mode.

    powershell -ExecutionPolicy Bypass -File Scripts\make_trailer.ps1 -Recording "C:\path\to\recording.mp4"

  -Recording  your capture of trailer mode (any resolution / fps; 1920x1080 60 fps is best)
  -Length     40 (default) or 55 seconds
  -Music      the score (default: Trailer\template_40.mp3 for 40 s, Trailer\template.mp3 for 55 s)
  -NoPan      open with a short fade from black instead of the Pan transition
  -Team       your team name for the file name (default: TeamName)
  -GameVolume how loud the game's own sound stays under the music (default 0.14)

What it does:
  1. finds the 12 shots in the recording by the black between them (ffmpeg blackdetect);
     it uses the LAST 12, so anything before the trailer (a console, a menu) is ignored.
     In the recording they come in trailer-mode order:
       chase, van, phone, gate, chants, molotov, Mom, crowd, the shot, sunrise, cemetery, title
  2. puts them in the cut's order and takes from each exactly as long as its part of the music:
       40 s: van 3.5 | phone 4.5 | gate 4.5 | chants 3 | molotov 3 | Mom 3 | crowd 3 | the shot 3 |
             sunrise 3.5 | people running 3 | the grave 3 | title 3
             (the running crowd is the last thing before the grave)
       55 s: chase 5 | van 5 | phone 5 | gate 5 | chants 3.75 | molotov 3.75 | Mom 3.75 | crowd 3.75 |
             the shot 5 | sunrise 6 | cemetery 6 | title 3
  3. opens on a Pan transition: black whips off to the left as the first shot pans in from the right,
     eased, with horizontal motion blur; the rest are short dissolves (a dip to black into the
     silence and into the title)
  4. lays the music over it, keeps a quiet layer of the game's ambient sound underneath
  5. writes <Team>_CheshmHayash_Trailer.mp4 (the 55 s cut: ..._Trailer_55s.mp4): H.264 1080p,
     -preset veryfast -crf 20, AAC audio
All Farsi text comes from the game itself (no ffmpeg drawtext).
#>
param(
    [Parameter(Mandatory = $true)][string]$Recording,
    [ValidateSet(40, 55)][int]$Length = 40,
    [string]$Music = "",
    [switch]$NoPan,
    [string]$Team = "TeamName",
    [double]$GameVolume = 0.14,
    [string]$OutDir = ""
)
# ffmpeg writes its progress to stderr; Windows PowerShell must not treat that as an error
$ErrorActionPreference = "Continue"
$Root = Split-Path -Parent $PSScriptRoot
if (-not $Music) { $Music = Join-Path $Root $(if ($Length -eq 40) { "Trailer\template_40.mp3" } else { "Trailer\template.mp3" }) }
if (-not $OutDir) { $OutDir = Join-Path $Root "Trailer" }

# ffmpeg from the PATH, or from winget's install folder
$env:Path = [Environment]::GetEnvironmentVariable('Path', 'Machine') + ';' + [Environment]::GetEnvironmentVariable('Path', 'User') + ';' + $env:Path
# (ffmpeg 7.1 or newer: the filter graph is read with -/filter_complex)
foreach ($tool in 'ffmpeg', 'ffprobe') {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
        throw "$tool not found. Install it once with:  winget install Gyan.FFmpeg   (then open a new window)"
    }
}
if (-not (Test-Path $Recording)) { throw "Recording not found: $Recording" }
if (-not (Test-Path $Music)) { throw "Music not found: $Music  (run Scripts\gen_trailer_music_procedural.py --length $Length)" }

# ---- the cut: which recorded shot goes where, and for how long (matched to the music) -----------
# recorded shots: 0 chase, 1 van, 2 phone, 3 gate, 4 chants, 5 molotov, 6 Mom, 7 crowd,
#                 8 the shot, 9 sunrise, 10 cemetery, 11 title
if ($Length -eq 40) {
    $Order = @(1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 10, 11)
    $Durations = @(3.5, 4.5, 4.5, 3.0, 3.0, 3.0, 3.0, 3.0, 3.5, 3.0, 3.0, 3.0)
    $DipInto = @(7, 11)          # cut positions that dip to black: the shot (silence), the title
} else {
    $Order = @(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11)
    $Durations = @(5.0, 5.0, 5.0, 5.0, 3.75, 3.75, 3.75, 3.75, 5.0, 6.0, 6.0, 3.0)
    $DipInto = @(8, 11)
}
# recorded shots taken from their END: the sun clears the mountains late in the sunrise shot,
# and the grave shot ends on the tombstone
$FromEnd = @(9, 10)
$Total = ($Durations | Measure-Object -Sum).Sum
$Fade = 0.25          # dissolve between shots
$Skip = 0.35          # skip the fade-in at the start of every shot
$PanTime = 0.6        # the opening Pan transition

$inv = [Globalization.CultureInfo]::InvariantCulture
function F([double]$v) { return $v.ToString("0.000", $inv) }

# ---- 1. find the shots ---------------------------------------------------------------------------
$duration = [double]::Parse((& ffprobe -v error -show_entries format=duration -of csv=p=0 "$Recording").Trim(), $inv)
$hasAudio = [bool](& ffprobe -v error -select_streams a -show_entries stream=index -of csv=p=0 "$Recording")
Write-Host ("Recording: {0:0.0} s, audio: {1}" -f $duration, $hasAudio)
Write-Host "Finding the shots (blackdetect)..."
$log = & ffmpeg -hide_banner -nostats -i "$Recording" -vf "blackdetect=d=0.15:pix_th=0.08:pic_th=0.995" -an -f null - 2>&1 | Out-String
$blacks = [regex]::Matches($log, 'black_start:\s*([\d\.]+)\s+black_end:\s*([\d\.]+)') | ForEach-Object {
    [pscustomobject]@{ Start = [double]::Parse($_.Groups[1].Value, $inv); End = [double]::Parse($_.Groups[2].Value, $inv) }
}
$segments = @()
$cursor = 0.0
foreach ($b in $blacks) {
    if ($b.Start - $cursor -ge 1.2) { $segments += [pscustomobject]@{ Start = $cursor; End = $b.Start } }
    $cursor = $b.End
}
if ($duration - $cursor -ge 1.2) { $segments += [pscustomobject]@{ Start = $cursor; End = $duration } }
Write-Host ("Found {0} shots between black frames." -f $segments.Count)
if ($segments.Count -lt $Durations.Count) {
    $segments | ForEach-Object { Write-Host ("  {0,7:0.00} - {1,7:0.00}" -f $_.Start, $_.End) }
    throw "Expected at least $($Durations.Count) shots. Record the whole trailer mode, from the first black to the title."
}
$segments = $segments[($segments.Count - $Durations.Count)..($segments.Count - 1)]

# ---- 2. trim every shot to its length --------------------------------------------------------------
$filters = New-Object System.Collections.Generic.List[string]
$last = $Durations.Count - 1
for ($i = 0; $i -le $last; $i++) {
    $shot = $Order[$i]
    $len = $Durations[$i] + $(if ($i -lt $last) { $Fade } else { 0 })
    $seg = $segments[$shot]
    $start = $seg.Start + $Skip
    if ($FromEnd -contains $shot) { $start = [math]::Max($start, $seg.End - $len - 0.2) }
    if ($start + $len -gt $seg.End) {
        Write-Warning ("Shot {0} is only {1:0.00} s long; it needs {2:0.00} s. Its last frame will be held." -f ($shot + 1), ($seg.End - $start), $len)
    }
    $filters.Add(("[0:v]trim=start={0}:duration={1},setpts=PTS-STARTPTS,fps=60,scale=1920:1080:force_original_aspect_ratio=decrease,pad=1920:1080:(ow-iw)/2:(oh-ih)/2,setsar=1,format=yuv420p,tpad=stop_mode=clone:stop_duration={1}[v{2}]" -f (F $start), (F $len), $i))
    if ($i -eq 0 -and -not $NoPan) {
        # the Pan transition: black and the first shot side by side, and the frame whips across from
        # the black to the shot (ease-in-out), blurred sideways while it moves fast
        $d = F $PanTime
        $ease = "if(gte(t,$d),1,if(lt(t/$d,0.5),4*pow(t/$d,3),1-pow(2-2*t/$d,3)/2))"
        $filters.Add(("color=c=black:s=1920x1080:r=60:d={0},format=yuv420p[blk]" -f (F $len)))
        $filters.Add(("[v0]trim=duration={0},setpts=PTS-STARTPTS[v0t]" -f (F $len)))
        $filters.Add(("[blk][v0t]hstack=inputs=2,crop=1920:1080:x='1920*{0}':y=0,avgblur=sizeX=22:sizeY=1:enable='between(t,{1},{2})',avgblur=sizeX=22:sizeY=1:enable='between(t,{3},{4})'[w0]" -f $ease, (F ($PanTime * 0.12)), (F ($PanTime * 0.88)), (F ($PanTime * 0.25)), (F ($PanTime * 0.75))))
    } else {
        $filters.Add(("[v{0}]trim=duration={1},setpts=PTS-STARTPTS[w{0}]" -f $i, (F $len)))
    }
    if ($hasAudio) {
        $filters.Add(("[0:a]atrim=start={0}:duration={1},asetpts=PTS-STARTPTS,aformat=sample_rates=48000:channel_layouts=stereo,apad=whole_dur={1}[a{2}]" -f (F $start), (F $len), $i))
    }
}

# ---- 3. join them, each cut starting exactly on the music's hit ----------------------------------
$prev = "w0"
$prevA = "a0"
$offset = 0.0
for ($i = 1; $i -le $last; $i++) {
    $offset += $Durations[$i - 1]
    $transition = $(if ($DipInto -contains $i) { "fadeblack" } else { "fade" })
    $filters.Add(("[{0}][w{1}]xfade=transition={2}:duration={3}:offset={4}[x{1}]" -f $prev, $i, $transition, (F $Fade), (F $offset)))
    $prev = "x$i"
    if ($hasAudio) {
        $filters.Add(("[{0}][a{1}]acrossfade=d={2}[y{1}]" -f $prevA, $i, (F $Fade)))
        $prevA = "y$i"
    }
}
$opening = $(if ($NoPan) { ",fade=t=in:st=0:d=0.15" } else { "" })
$filters.Add(("[{0}]trim=duration={1},setpts=PTS-STARTPTS{2}[vout]" -f $prev, (F $Total), $opening))

# ---- 4. the music, with the game's sound quietly underneath ----------------------------------------
$filters.Add("[1:a]aformat=sample_rates=48000:channel_layouts=stereo,atrim=duration=" + (F $Total) + ",asetpts=PTS-STARTPTS[music]")
if ($hasAudio) {
    $filters.Add(("[{0}]atrim=duration={2},asetpts=PTS-STARTPTS,volume={1}[game]" -f $prevA, (F $GameVolume), (F $Total)))
    $filters.Add("[music][game]amix=inputs=2:duration=first:normalize=0,alimiter=limit=0.95[aout]")
} else {
    $filters.Add("[music]alimiter=limit=0.95[aout]")
}

# ---- 5. render ----------------------------------------------------------------------------------------
New-Item -ItemType Directory -Force $OutDir | Out-Null
$out = Join-Path $OutDir ("{0}_CheshmHayash_Trailer{1}.mp4" -f $Team, $(if ($Length -eq 55) { "_55s" } else { "" }))
$graph = Join-Path ([IO.Path]::GetTempPath()) "ch_trailer_graph.txt"
[IO.File]::WriteAllText($graph, ($filters -join ";`n"))
Write-Host "Rendering $out ..."
& ffmpeg -hide_banner -y -loglevel error -stats -i "$Recording" -i "$Music" -/filter_complex "$graph" `
    -map "[vout]" -map "[aout]" -t (F $Total) `
    -c:v libx264 -preset veryfast -crf 20 -pix_fmt yuv420p -r 60 `
    -c:a aac -b:a 192k -ar 48000 -movflags +faststart "$out"
if ($LASTEXITCODE -ne 0) { throw "ffmpeg failed (exit $LASTEXITCODE). The filter graph is in $graph" }
$final = [double]::Parse((& ffprobe -v error -show_entries format=duration -of csv=p=0 "$out").Trim(), $inv)
Write-Host ("Done: {0}  ({1:0.00} s)" -f $out, $final)
