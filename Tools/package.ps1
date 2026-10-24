# Cosmic Ascension - the Win64 package and the cooked build's hashes (MVP item 9, Windows PowerShell 5.1).
# docs/60_DECISIONS/headers/A9_win64_packaging.md section 3.2; docs/40_DEV_GUIDE.md section 7.
#
#   powershell -ExecutionPolicy Bypass -File Tools\package.ps1 [-Root C:\CA_Builds] [-Ticks20000] [-Shipping] [-AllowDirty]
#
#   -Root         the folder of the packages, outside OneDrive and without spaces (default $env:COSMIC_BUILDS or C:\CA_Builds)
#   -Ticks20000   also compare the 20,000-tick hash runs of the package and the editor (about 25 s more - engine start-up, not ticks)
#   -Shipping     also compile the Shipping game target once (no cook, no run)
#   -AllowDirty   package a working tree with uncommitted changes (every result says DIRTY)
#
# Close the editor first: UAT -build builds the editor target too. Every package goes into a new folder
# <Root>\<yyyyMMdd-HHmm>-<short commit>, so no save, log or container of an earlier build mixes in: the game is
# <that folder>\Windows\Cosmic_Ascension.exe, a Development package keeps its saves and logs in
# <that folder>\Windows\Cosmic_Ascension\Saved\, and <that folder>-handoff.zip (made before anything runs, so without
# Saved\) is what an outside player gets. Exit condition (d): the package's hash runs (World/CosmicHashRun - idle and
# with the ruler policy, 3,000 ticks) give the editor's two hashes; the editor's are then compared with
# Tools/baselines/seed42.json as a step of their own (a stale baseline is not a binary difference).
param(
    [string] $Root = $(if ($env:COSMIC_BUILDS) { $env:COSMIC_BUILDS } else { 'C:\CA_Builds' }),
    [switch] $Ticks20000,
    [switch] $Shipping,
    [switch] $AllowDirty
)
$ErrorActionPreference = 'Stop'
if (-not $env:UE_ROOT) { $env:UE_ROOT = 'C:\Program Files\Epic Games\UE_5.8' }
if (-not $env:PROJECT_ROOT) { $env:PROJECT_ROOT = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path }
$ProjectRoot = $env:PROJECT_ROOT
$Project = Join-Path $ProjectRoot 'Cosmic_Ascension.uproject'
$Editor = Join-Path $env:UE_ROOT 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$Uat = Join-Path $env:UE_ROOT 'Engine\Build\BatchFiles\RunUAT.bat'
$BuildBat = Join-Path $env:UE_ROOT 'Engine\Build\BatchFiles\Build.bat'
$failed = @()
$times = [ordered]@{}
$editorKeys = @{}

function Stop-Package([string] $Why) { Write-Host "package: STOPPED - $Why"; exit 1 }

function Step([string] $Name, [scriptblock] $Body) {
    Write-Host "== $Name"
    $watch = [Diagnostics.Stopwatch]::StartNew()
    try { & $Body }
    catch { Write-Host "   FAILED: $_"; $script:failed += $Name }
    $script:times[$Name] = [math]::Round($watch.Elapsed.TotalSeconds, 1)
}

# One process with a time limit (header section 3.2): one argument string (Windows PowerShell 5.1 does not quote the
# items of -ArgumentList, so a path with a space is quoted by the caller), the handle cached so the exit code stays
# readable, and on overrun the whole process tree killed (a .bat leaves its children running otherwise).
function Start-Limited([string] $File, [string] $ArgLine, [int] $TimeoutSec, [string] $StdOut) {
    $redirect = @{}
    if ($StdOut) { $redirect = @{ RedirectStandardOutput = $StdOut; RedirectStandardError = ($StdOut -replace '\.log$', '.err.log') } }
    $p = Start-Process -FilePath $File -ArgumentList $ArgLine -PassThru -NoNewWindow @redirect
    $null = $p.Handle
    if (-not $p.WaitForExit($TimeoutSec * 1000)) {
        & taskkill /T /F /PID $p.Id | Out-Null
        throw "no exit within $TimeoutSec s: $File $ArgLine"
    }
    return $p.ExitCode
}

# A game process (the packaged game or the editor's -game): judged on the exit code (a graceful exit is always 0, a crash
# 3) and on a log written after the start. The log and the '_2' log the engine falls back to are deleted first, so a run
# that never opens its log cannot pass on an old one.
function Invoke-Game([string] $Exe, [string] $ArgLine, [string] $Log, [int] $TimeoutSec) {
    foreach ($old in @($Log, ($Log -replace '\.log$', '_2.log'))) { if (Test-Path $old) { Remove-Item -Force $old } }
    $start = Get-Date
    $code = Start-Limited $Exe "$ArgLine -AbsLog=$Log" $TimeoutSec ($Log -replace '\.log$', '.stdout.log')
    if ($code -ne 0) { throw "exit code ${code}: $ArgLine (see $Log)" }
    if (-not (Test-Path $Log) -or (Get-Item $Log).LastWriteTime -lt $start) { throw "no log written by this run: $Log" }
}

# 'seed 42, T ticks, hash H, world W' of the last CosmicHash line - the run that was asked for (seed 42, these ticks, the
# policy suffix exactly when asked) or it throws; a refusal throws with its line.
function Get-HashKey([string] $Log, [int] $Ticks, [bool] $Policy) {
    $line = Select-String -Path $Log -Pattern 'CosmicHash: ' | Select-Object -Last 1
    if (-not $line) { throw "no ""CosmicHash:"" line in $Log" }
    $want = 'CosmicHash: (seed 42, ' + $Ticks + ' ticks, hash \d+, world \d+)' + $(if ($Policy) { ', policy' } else { '' }) + '\s*$'
    if ($line.Line -match $want) { return $Matches[1] }
    throw "not the run asked for ($Ticks ticks, policy $Policy): $($line.Line)"
}

# Both hashes must agree; the message says which one differs.
function Compare-Keys([string] $What, [string] $Package, [string] $EditorKey) {
    if ($Package -eq $EditorKey) { Write-Host "   $What - same: $Package"; return }
    $p = [regex]::Match($Package, 'hash (\d+), world (\d+)'); $e = [regex]::Match($EditorKey, 'hash (\d+), world (\d+)')
    $which = @()
    if ($p.Groups[1].Value -ne $e.Groups[1].Value) { $which += 'the state hash' }
    if ($p.Groups[2].Value -ne $e.Groups[2].Value) { $which += 'the world hash' }
    throw "$What - $($which -join ' and ') differ`n   package: $Package`n   editor:  $EditorKey"
}

# 1. What is packaged: the editor closed, a root without spaces, a clean tree (tracked changes anywhere, untracked files
# where the build reads them), the data checked and its stamp true.
if (Get-Process -Name 'UnrealEditor' -ErrorAction SilentlyContinue) { Stop-Package 'close the editor (UAT -build builds the editor target)' }
if ($Root -match '\s') { Stop-Package "the root '$Root' has a space (the game's -AbsLog= stops at it)" }
$commit = (& git -C $ProjectRoot rev-parse --short HEAD).Trim()
$tracked = @(& git -C $ProjectRoot status --porcelain --untracked-files=no)
$untracked = @(& git -C $ProjectRoot status --porcelain --untracked-files=all -- Source Content Config Tools/data Cosmic_Ascension.uproject | Where-Object { $_.StartsWith('??') })
$dirtyLines = @($tracked + $untracked | Where-Object { $_ })
$dirty = $dirtyLines.Count -gt 0
if ($dirty -and -not $AllowDirty) { Stop-Package "the working tree has uncommitted changes (commit, or pass -AllowDirty):`n$($dirtyLines -join "`n")" }
$tag = if ($dirty) { "$commit-dirty" } else { $commit }
& python -I (Join-Path $ProjectRoot 'Tools\check_data.py')
if ($LASTEXITCODE -ne 0) { Stop-Package 'Tools/check_data.py failed' }
# The stamp of the data assets (Tools/make_data_tables.py writes it; the test Data.SourcesInSync reads it): the SHA-1 of
# each source with its CR bytes removed.
$dataDir = Join-Path $ProjectRoot 'Tools\data'
$sha1 = [Security.Cryptography.SHA1]::Create()
$latin1 = [Text.Encoding]::GetEncoding(28591)  # one char per byte, so dropping the CR chars drops the CR bytes
foreach ($line in Get-Content (Join-Path $dataDir 'generated.sha1')) {
    if (-not $line.Trim()) { continue }
    $name, $want = $line.Trim() -split ' ', 2
    $bytes = $latin1.GetBytes($latin1.GetString([IO.File]::ReadAllBytes((Join-Path $dataDir $name))).Replace("`r", ''))
    $have = -join ($sha1.ComputeHash($bytes) | ForEach-Object { $_.ToString('x2') })
    if ($have -ne $want) { Stop-Package "$name is not the source the data assets were made from (rerun Tools/make_data_tables.py)" }
}
$packageStart = Get-Date

# 2. A new folder for this package.
$Out = Join-Path $Root ("{0}-{1}" -f (Get-Date -Format 'yyyyMMdd-HHmm'), $tag)
if (Test-Path $Out) { Stop-Package "$Out exists already" }
$Logs = Join-Path $Out 'Logs'
New-Item -ItemType Directory -Force -Path $Logs | Out-Null
$Win = Join-Path $Out 'Windows'
$Game = Join-Path $Win 'Cosmic_Ascension\Binaries\Win64\Cosmic_Ascension.exe'
Write-Host "package: $Out (commit $tag)"

# 3. Build, cook and stage (no -archive: the folder is the keeping copy). The cook's summary decides; warnings outside the
# known list are shown.
Step 'uat: build, cook, stage' {
    $uatArgs = "BuildCookRun `"-project=$Project`" -platform=Win64 -clientconfig=Development -build -cook -stage -pak -iostore -prereqs -stagingdirectory=$Out -utf8output -unattended -noP4"
    $code = Start-Limited $Uat $uatArgs 5400 (Join-Path $Logs 'uat.log')
    if ($code -ne 0) { throw "UAT exit code $code (see $Logs\uat.log)" }
    # This run's cook log, wherever UAT keeps its logs (uebp_LogFolder, or a folder named after the engine under APPDATA).
    $uatLogs = if ($env:uebp_LogFolder) { $env:uebp_LogFolder } else { Join-Path $env:APPDATA 'Unreal Engine\AutomationTool\Logs' }
    $cook = Get-ChildItem -Path $uatLogs -Recurse -Filter 'Cook-*.txt' -ErrorAction SilentlyContinue | Where-Object { $_.LastWriteTime -ge $packageStart } | Sort-Object LastWriteTime | Select-Object -Last 1
    if (-not $cook) { throw "no cook log of this run under $uatLogs" }
    Copy-Item $cook.FullName (Join-Path $Logs 'cook.log')
    $summary = Select-String -Path (Join-Path $Logs 'cook.log') -Pattern '(\d+) error\(s\), (\d+) warning\(s\)' | Select-Object -Last 1
    if (-not $summary) { throw 'no error/warning summary in the cook log' }
    $null = $summary.Line -match '(\d+) error\(s\), (\d+) warning\(s\)'
    Write-Host "   cook: $($Matches[1]) error(s), $($Matches[2]) warning(s)"
    if ([int]$Matches[1] -gt 0) { throw 'the cook has errors (see cook.log)' }
    $known = @('GetAddressInfo failed to resolve host')
    $section = $false
    foreach ($line in Get-Content (Join-Path $Logs 'cook.log')) {
        if ($line -match 'Warning/Error Summary') { $section = $true; continue }
        if ($section -and $line -match 'Warning:' -and -not ($known | Where-Object { $line -match [regex]::Escape($_) })) { Write-Host "   cook warning: $line" }
    }
    if (-not (Test-Path $Game)) { throw "no game exe at $Game" }
}
if ($failed.Count -gt 0) { Stop-Package "the package was not made: $($failed -join '; ')" }

# 4. The build's note and the handover zip, before anything runs (so it holds no Saved\); every file of the build in it.
Step 'handover: BuildInfo.txt and zip' {
    $info = @("commit=$tag", "dirty=$([int]$dirty)", "utc=$((Get-Date).ToUniversalTime().ToString('yyyy-MM-ddTHH:mm:ssZ'))", 'uat=BuildCookRun -platform=Win64 -clientconfig=Development -build -cook -stage -pak -iostore -prereqs')
    [IO.File]::WriteAllLines((Join-Path $Win 'Cosmic_Ascension\BuildInfo.txt'), $info, (New-Object Text.UTF8Encoding $false))
    if (Test-Path (Join-Path $Win 'Cosmic_Ascension\Saved')) { throw 'the staged build already has a Saved folder' }
    Compress-Archive -Path (Join-Path $Win '*') -DestinationPath "$Out-handoff.zip" -CompressionLevel Optimal -ErrorAction Stop
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $zip = [IO.Compression.ZipFile]::OpenRead("$Out-handoff.zip")
    try { $inZip = @($zip.Entries | Where-Object { $_.Name }).Count } finally { $zip.Dispose() }
    $inBuild = @(Get-ChildItem -Path $Win -Recurse -File).Count
    if ($inZip -ne $inBuild) { throw "the zip holds $inZip files, the build $inBuild" }
}

# 5. The start: every table and the string table found, the build line naming this commit and a new galaxy written.
Step 'start check' {
    $log = Join-Path $Logs 'start.log'
    Invoke-Game $Game '-nullrhi -nosound -unattended -nosplash -ExecCmds=quit' $log 300
    foreach ($want in 'CosmicBuild: version', 'NewGalaxy:', "commit $tag, cooked") { if (-not (Select-String -Path $log -Pattern $want -SimpleMatch -Quiet)) { throw "no '$want' in start.log" } }
    foreach ($bad in 'Data table .* is missing', 'LogStringTable: Warning', 'LogCosmic: Error') {
        $hit = Select-String -Path $log -Pattern $bad | Select-Object -First 1
        if ($hit) { throw "start.log: $($hit.Line)" }
    }
}

# 6, 7. Exit condition (d): the package and the editor, the same switch.
$ticksList = @(3000) + $(if ($Ticks20000) { @(20000) } else { @() })
foreach ($ticks in $ticksList) {
    foreach ($policy in $false, $true) {
        $what = "$ticks ticks " + $(if ($policy) { 'with the ruler policy' } else { 'idle' })
        $suffix = "$ticks" + $(if ($policy) { '_policy' } else { '' })
        $switches = "-Seed=42 -CosmicHashRun=$ticks" + $(if ($policy) { ' -CosmicHashPolicy' } else { '' }) + ' -nullrhi -nosound -unattended -nosplash -ExecCmds=quit'
        $limit = if ($ticks -gt 3000) { 3600 } else { 900 }
        Step "(d) $what" {
            $gameLog = Join-Path $Logs "hash$suffix.log"
            Invoke-Game $Game $switches $gameLog $limit
            $editorLog = Join-Path $Logs "hash${suffix}_editor.log"
            Invoke-Game $Editor "`"$Project`" -game $switches" $editorLog $limit
            $packageKey = Get-HashKey $gameLog $ticks $policy
            $editorKey = Get-HashKey $editorLog $ticks $policy
            $script:editorKeys[$suffix] = $editorKey
            Compare-Keys $what $packageKey $editorKey
        }
    }
}
# The editor against the baseline file, as a step of its own: a stale baseline (an intended world change not yet written
# with run_checks -UpdateBaseline) is not a binary difference.
Step 'baseline: editor vs seed42.json' {
    $baseline = Get-Content (Join-Path $ProjectRoot 'Tools\baselines\seed42.json') -Raw | ConvertFrom-Json
    foreach ($pair in @(@('3000', 'summary_line'), @('3000_policy', 'ruler_summary_line'))) {
        $key = $script:editorKeys[$pair[0]]
        if (-not $key) { throw "no editor key for $($pair[0]) (its (d) step failed first)" }
        $want = if ($baseline.($pair[1]) -match 'CosmicBalance: (seed \d+, \d+ ticks, hash \d+, world \d+)') { $Matches[1] } else { '' }
        if ($want -ne $key) { throw "baseline stale - the editor gives '$key', seed42.json $($pair[1]) '$want'; run run_checks.ps1 (-UpdateBaseline for an intended change). Not a binary difference." }
        Write-Host "   editor = seed42.json $($pair[1]): $key"
    }
}

# 8. Shipping compiles (no cook, no run - decision 1).
if ($Shipping) {
    Step 'shipping: compile the game target' {
        $code = Start-Limited $BuildBat "Cosmic_Ascension Win64 Shipping `"-Project=$Project`" -WaitMutex" 2700 (Join-Path $Logs 'shipping.log')
        if ($code -ne 0) { throw "Build.bat Shipping exit code $code (see shipping.log)" }
    }
}

# 9. What was made, where, and how long it took.
$size = (Get-ChildItem -Recurse -File $Out | Measure-Object -Property Length -Sum).Sum
$zip = Get-Item "$Out-handoff.zip" -ErrorAction SilentlyContinue
Write-Host ("package: folder {0:N0} MB, zip {1:N0} MB" -f ($size / 1MB), $(if ($zip) { $zip.Length / 1MB } else { 0 }))
Write-Host "package: game $Win\Cosmic_Ascension.exe; saves and logs in $Win\Cosmic_Ascension\Saved\; handover $Out-handoff.zip"
foreach ($name in $times.Keys) { Write-Host ("   {0,-45} {1,8} s" -f $name, $times[$name]) }
$times.GetEnumerator() | ForEach-Object { "$($_.Key)`t$($_.Value)" } | Set-Content -Encoding UTF8 (Join-Path $Logs 'times.txt')
if ($dirty) { Write-Host 'package: DIRTY - the working tree had uncommitted changes' }
if ($failed.Count -gt 0) { Write-Host "package: FAILED -> $($failed -join '; ')"; exit 1 }
Write-Host 'package: all passed'
