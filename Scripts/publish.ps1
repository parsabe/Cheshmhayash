<#
Publishes to GitHub: `master` (the README, the default branch), the `game` branch, the `website`
branch and the `jam-submission` tag; makes `master` the default branch and switches on GitHub Pages.

    powershell -ExecutionPolicy Bypass -File Scripts\publish.ps1
    powershell -ExecutionPolicy Bypass -File Scripts\publish.ps1 -Message "Trailer: 40 s cut"
    powershell -ExecutionPolicy Bypass -File Scripts\publish.ps1 -SiteOnly

  -Message   commit message for changes in the game folder and the website folder
             (without it, uncommitted changes are listed and left alone)
  -SiteOnly  only commit and push the website
  -Site      the website folder (default: ..\Cheshmhayash-website next to the game folder)

Never pushes `backup/full-history` (the old history with build files GitHub rejects) and never moves
the `jam-submission` tag. Default branch and Pages: uses the GitHub CLI (`gh`) if it is installed and
logged in, otherwise it opens the settings pages and tells you the clicks.
#>
param(
    [string]$Message = "",
    [switch]$SiteOnly,
    [string]$Site = ""
)
$ErrorActionPreference = "Continue"
$Root = Split-Path -Parent $PSScriptRoot
if (-not $Site) { $Site = Join-Path (Split-Path -Parent $Root) "Cheshmhayash-website" }

function Run([string]$Dir, [string[]]$GitArgs) {
    Write-Host ("> git " + ($GitArgs -join " ")) -ForegroundColor DarkGray
    & git -C $Dir @GitArgs
    if ($LASTEXITCODE -ne 0) { throw ("git " + ($GitArgs -join " ") + " failed") }
}
function CommitIfNeeded([string]$Dir, [string]$What) {
    $changes = & git -C $Dir status --porcelain
    if (-not $changes) { Write-Host "$What`: nothing to commit."; return }
    if ($Message) {
        Run $Dir @("add", "-A")
        Run $Dir @("commit", "-m", $Message)
    } else {
        Write-Host "$What has uncommitted changes (not committed; pass -Message `"...`" to commit them):" -ForegroundColor Yellow
        $changes | ForEach-Object { Write-Host "    $_" }
    }
}

# ---- the remote: owner/repo -------------------------------------------------------------------
$remote = (& git -C $Root remote get-url origin).Trim()
if ($remote -notmatch 'github\.com[:/]([^/]+)/(.+?)(\.git)?$') { throw "origin is not a GitHub repository: $remote" }
$Owner = $Matches[1]; $Repo = $Matches[2]
$SiteUrl = "https://{0}.github.io/{1}/" -f $Owner.ToLower(), $Repo

# ---- 1. the game --------------------------------------------------------------------------------
if (-not $SiteOnly) {
    $branch = (& git -C $Root rev-parse --abbrev-ref HEAD).Trim()
    if ($branch -ne "game") { throw "The game folder is on branch '$branch'; switch to 'game' first (git switch game)." }
    CommitIfNeeded $Root "The game folder"
}

# ---- 2. the website (its own folder, a worktree of the orphan branch) ----------------------------
if (-not (Test-Path (Join-Path $Site "index.html"))) {
    Write-Host "Website folder not found; creating it at $Site"
    & git -C $Root show-ref --verify --quiet refs/heads/website
    if ($LASTEXITCODE -ne 0) {
        Run $Root @("fetch", "origin", "website:website")
    }
    Run $Root @("worktree", "add", $Site, "website")
}
CommitIfNeeded $Site "The website folder"

# ---- 3. push ------------------------------------------------------------------------------------
if ($SiteOnly) {
    Run $Site @("push", "origin", "website")
} else {
    # master must be the small README branch, never the old history with the build files
    $bad = & git -C $Root rev-list master -- Binaries Intermediate Saved DerivedDataCache
    if ($bad) { throw "master contains Unreal build files in its history; not pushing it. (The README branch should have one commit.)" }
    Run $Root @("push", "origin", "master", "game", "website", "jam-submission")
}

# ---- 4a. master as the default branch -------------------------------------------------------------
$gh = Get-Command gh -ErrorAction SilentlyContinue
if (-not $SiteOnly) {
    $defaultSet = $false
    if ($gh) {
        & gh api -X PATCH "repos/$Owner/$Repo" -f default_branch=master *> $null
        if ($LASTEXITCODE -eq 0) { $defaultSet = $true; Write-Host "Default branch: master." }
    }
    if (-not $defaultSet) {
        Write-Host ""
        Write-Host "Make master the default branch once (the page is opening in your browser):" -ForegroundColor Cyan
        Write-Host "  General > Default branch > the switch icon > master > Update"
        Start-Process "https://github.com/$Owner/$Repo/settings"
    }
}

# ---- 4b. GitHub Pages from the `website` branch -------------------------------------------------
$pagesOn = $false
if ($gh) {
    & gh api "repos/$Owner/$Repo/pages" *> $null
    if ($LASTEXITCODE -eq 0) {
        $pagesOn = $true
        Write-Host "GitHub Pages is already on."
    } else {
        & gh api -X POST "repos/$Owner/$Repo/pages" -f "source[branch]=website" -f "source[path]=/" *> $null
        if ($LASTEXITCODE -eq 0) { $pagesOn = $true; Write-Host "GitHub Pages switched on (branch: website)." }
    }
}
if (-not $pagesOn) {
    $settings = "https://github.com/$Owner/$Repo/settings/pages"
    Write-Host ""
    Write-Host "Switch on GitHub Pages once (the page is opening in your browser):" -ForegroundColor Cyan
    Write-Host "  1. Build and deployment > Source: Deploy from a branch"
    Write-Host "  2. Branch: website   folder: / (root)   > Save"
    Write-Host "  (or install the GitHub CLI: winget install GitHub.cli ; gh auth login ; and run this again)"
    Start-Process $settings
}

Write-Host ""
Write-Host "Website: $SiteUrl  (live 1-2 minutes after each push)" -ForegroundColor Green
