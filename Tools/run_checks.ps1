# Cosmic Ascension - full check runner for the Windows development PC (PowerShell 5.1).
# docs/40_DEV_GUIDE.md section 7. Order: editor-free checks -> (optional) data assets -> builds -> automation tests twice
# -> headless smoke -> balance commandlet against Tools/baselines/seed42.json (the idle run, then the run with the ruler's
# policy -Answer=1 -Build=1 -Focus=1 -Edict=1 -Diplomacy=1, and the wanderer's) -> the hash run of a game process against the first two
# commandlet lines (MVP item 9).
#
#   powershell -ExecutionPolicy Bypass -File Tools\run_checks.ps1 [-Tier smoke|full] [-Data] [-UpdateBaseline] [-SkipBuild]
#
#   -Tier smoke   (default) automation filter "Cosmic_Ascension.Sim.Determinism+Cosmic_Ascension.Sim.Save+Cosmic_Ascension.Data"
#   -Tier full    every Cosmic_Ascension.* test (the 20,000-tick runs included)
#   -Data         regenerate the DataTable / String Table assets from Tools/data before building (editor closed)
#   -UpdateBaseline  write the commandlet's lines into Tools/baselines/seed42.json instead of comparing
#
# Paths come from $env:UE_ROOT and $env:PROJECT_ROOT (defaults below are this PC's). The editor must be closed for the
# editor-target build. Written on Linux on 2026-10-19 and verified on Windows the same day;
# every command is the documented one from docs/40_DEV_GUIDE.md, only the sequencing and the exit-code handling are new.
param(
    [ValidateSet('smoke', 'full')] [string] $Tier = 'smoke',
    [switch] $Data,
    [switch] $UpdateBaseline,
    [switch] $SkipBuild
)
$ErrorActionPreference = 'Stop'
if (-not $env:UE_ROOT) { $env:UE_ROOT = 'C:\Program Files\Epic Games\UE_5.8' }
if (-not $env:PROJECT_ROOT) { $env:PROJECT_ROOT = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path }
$Root = $env:PROJECT_ROOT
$RootF = $Root -replace '\\', '/'
$Project = Join-Path $Root 'Cosmic_Ascension.uproject'
$Editor = Join-Path $env:UE_ROOT 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$Build = Join-Path $env:UE_ROOT 'Engine\Build\BatchFiles\Build.bat'
$Logs = Join-Path $Root 'Saved\Logs'
New-Item -ItemType Directory -Force -Path $Logs | Out-Null
$failed = @()

function Step([string] $Name, [scriptblock] $Body) {
    Write-Host "== $Name"
    try { & $Body; if ($LASTEXITCODE -ne 0 -and $LASTEXITCODE -ne $null) { throw "exit code $LASTEXITCODE" } }
    catch { Write-Host "   FAILED: $_"; $script:failed += $Name }
}

Step 'data: schema check'             { python -I (Join-Path $Root 'Tools\check_data.py') }
Step 'data: checker unit tests'       { python -I (Join-Path $Root 'Tools\test_data_schema.py') }
Step 'layers: include direction etc.' { python -I (Join-Path $Root 'Tools\check_layers.py') }
# One step per command: a step reads only the exit code of its last command.
Step 'docs: config reference'          { python -I (Join-Path $Root 'Tools\make_config_reference.py') --check }
Step 'docs: links and anchors'         { python -I (Join-Path $Root 'Tools\check_docs.py') }

if ($Data) {
    Step 'assets: DataTable / String Table' {
        & $Editor $Project -run=pythonscript "-Script=$RootF/Tools/make_data_tables.py" -unattended -nosplash -stdout -FullStdOutLogOutput "-AbsLog=$RootF/Saved/Logs/MakeDataTables.log"
        if (-not (Select-String -Path (Join-Path $Logs 'MakeDataTables.log') -Pattern 'written and verified' -Quiet)) { throw 'make_data_tables did not report "written and verified"' }
    }
}
if (-not $SkipBuild) {
    Step 'build: editor target' { & $Build Cosmic_AscensionEditor Win64 Development "-Project=$Project" -WaitMutex }
    Step 'build: game target'   { & $Build Cosmic_Ascension Win64 Development "-Project=$Project" -WaitMutex }
}

$filter = if ($Tier -eq 'full') { 'Cosmic_Ascension' } else { 'Cosmic_Ascension.Sim.Determinism+Cosmic_Ascension.Sim.Save+Cosmic_Ascension.Data+Cosmic_Ascension.Sim.KnowingInert' }
foreach ($pass in 1, 2) {
    Step "tests: $Tier pass $pass of 2" {
        $log = "$RootF/Saved/Logs/Tests$pass.log"
        & $Editor $Project -game -nullrhi -nosound -unattended -nosplash -stdout -FullStdOutLogOutput "-ExecCmds=Automation RunTests $filter; Quit" "-TestExit=Automation Test Queue Empty" "-ReportExportPath=$RootF/Saved/Automation/Report$pass" "-AbsLog=$log"
        # 255 = failure (or no matching test); the report folder tells which.
        if ($LASTEXITCODE -ne 0) { throw "automation exit code $LASTEXITCODE (see $log)" }
    }
}

Step 'smoke: -game -nullrhi' {
    & $Editor $Project -game -nullrhi -nosound -unattended -nosplash -stdout -FullStdOutLogOutput '-ExecCmds=quit' "-AbsLog=$RootF/Saved/Logs/GameSmoke.log"
    if (-not (Select-String -Path (Join-Path $Logs 'GameSmoke.log') -Pattern 'LogCosmic' -Quiet)) { throw 'no LogCosmic line in GameSmoke.log' }
}

# A balance line without its ', wrote <path>' (the CSV path is relative to the binaries and changes when the project
# moves): the key 'CosmicBalance: seed N, T ticks, hash H, world W', and after ' | ' the ruler line as it is (MVP item 9).
function Remove-WrotePath([string] $Line) { return ($Line -replace ', wrote .*?(?= \| |$)', '') }

# One commandlet run of seed 42 for $Ticks ticks (3,000 unless given), compared with the baseline field $Field (or
# written into it with -UpdateBaseline): the hash line, and with $Ruler also the ruler line after ' | ' - both without
# the CSV path.
function Compare-Balance([string] $Name, [string] $Field, [string[]] $Extra, [switch] $Ruler, [switch] $Wanderer, [int] $Ticks = 3000) {
    $log = Join-Path $Logs "$Name.log"
    # The log of this run only: an old one (or the '_2' the engine writes when the log is held open) cannot answer for it.
    foreach ($old in @($log, (Join-Path $Logs "${Name}_2.log"))) { if (Test-Path $old) { Remove-Item -Force $old } }
    & $Editor $Project -run=CosmicBalance -Seed=42 "-Ticks=$Ticks" "-Out=$Name.csv" @Extra -unattended -nosplash -stdout -FullStdOutLogOutput "-AbsLog=$RootF/Saved/Logs/$Name.log"
    if (-not (Test-Path $log)) { throw "no $Name.log written by this run" }
    $line = Select-String -Path $log -Pattern 'CosmicBalance: seed' | Select-Object -Last 1
    if (-not $line) { throw "no ""CosmicBalance: seed"" summary line in $Name.log" }
    $summary = if ($line.Line -match '(CosmicBalance: seed.*)') { $Matches[1] } else { $line.Line }
    if ($Ruler) {
        $rulerLine = Select-String -Path $log -Pattern 'CosmicBalance ruler:' | Select-Object -Last 1
        if (-not $rulerLine) { throw "no ""CosmicBalance ruler:"" line in $Name.log" }
        $summary += ' | ' + $(if ($rulerLine.Line -match '(CosmicBalance ruler:.*)') { $Matches[1] } else { $rulerLine.Line })
    }
    if ($Wanderer) {
        $wandererLine = Select-String -Path $log -Pattern 'CosmicBalance wanderer:' | Select-Object -Last 1
        if (-not $wandererLine) { throw "no ""CosmicBalance wanderer:"" line in $Name.log" }
        $summary += ' | ' + $(if ($wandererLine.Line -match '(CosmicBalance wanderer:.*)') { $Matches[1] } else { $wandererLine.Line })
    }
    # The stage times beside the line, and the free memory read after the run (B6b [B-6br]): a loaded machine shows itself - in bundle B the
    # economy stage, which no B6b step touches, swelled x1.5-1.8 with the AI stage. Printed only; nothing is compared.
    try {
        $stages = Select-String -Path $log -Pattern 'CosmicBalance stages:.*crisis ([0-9.]+) us, AI ([0-9.]+) us.*economy ([0-9.]+) us' | Select-Object -Last 1
        if ($stages) {
            $freeGB = [math]::Round((Get-CimInstance Win32_OperatingSystem).FreePhysicalMemory / 1MB, 1)
            Write-Host ("   stages: AI {0} us, economy {1} us, crisis {2} us (free memory {3} GB)" -f $stages.Matches[0].Groups[2].Value, $stages.Matches[0].Groups[3].Value, $stages.Matches[0].Groups[1].Value, $freeGB)
        }
    } catch { }
    $baselinePath = Join-Path $Root 'Tools\baselines\seed42.json'
    $baseline = Get-Content $baselinePath -Raw | ConvertFrom-Json
    if ($UpdateBaseline) {
        if ($baseline.PSObject.Properties.Name -contains $Field) { $baseline.$Field = $summary } else { $baseline | Add-Member -NotePropertyName $Field -NotePropertyValue $summary }
        $baseline.verified_at = (Get-Date -Format 'yyyy-MM-dd')
        # UTF-8 without a byte order mark (Set-Content -Encoding UTF8 of PowerShell 5.1 writes one; Python's json reads none).
        [IO.File]::WriteAllText($baselinePath, ($baseline | ConvertTo-Json -Depth 4), (New-Object Text.UTF8Encoding $false))
        Write-Host "   baseline updated: $summary"
    } elseif (-not ($baseline.PSObject.Properties.Name -contains $Field)) {
        throw "$Field is missing from Tools/baselines/seed42.json - record it once with -UpdateBaseline"
    } elseif ((Remove-WrotePath $baseline.$Field) -ne (Remove-WrotePath $summary)) {
        throw "$Field differs from Tools/baselines/seed42.json`n   now:      $summary`n   baseline: $($baseline.$Field)`n   (intended change -> rerun with -UpdateBaseline and put the new hashes in the commit message)"
    } else { Write-Host "   $summary" }
}

Step 'balance: seed 42 vs baseline' { Compare-Balance 'checks' 'summary_line' @() }

# The ruler's policy (docs/40_DEV_GUIDE.md section 9.3): answers, the project key, the research focus (MVP item 7) and the
# edicts (B2) and the diplomacy (B3: friendships, treaties, guards) between ticks, the scope of telling of a game. It keeps the inputs deterministic across builds.
Step 'balance: seed 42 with the ruler policy vs baseline' { Compare-Balance 'checks_ruler' 'ruler_summary_line' @('-Answer=1', '-Build=1', '-Focus=1', '-Edict=1', '-Diplomacy=1', '-TellAll=0') -Ruler }

# B1 (header section 5): the same ruler, who leaves the throne at tick 1,200 and walks (World/CosmicWandererPolicy) - the
# hash line, the ruler line and the wanderer line, over 6,000 ticks, the horizon of Sim.WandererLoop (seed 42 took its
# throne back at 4,901 with B2's data, at none before 6,000 with B3's card and data off, at 3,121 - by the vacuum - with
# B3's data on through [B-6b5], and at none before 6,000 from [B-6b6]'s data - the AI treaty step moves the world first,
# at 1,935; Sim.WandererLoop plays seed 4).
Step 'balance: seed 42 with the wanderer policy vs baseline' { Compare-Balance 'checks_wanderer' 'wanderer_summary_line' @('-Answer=1', '-Build=1', '-Focus=1', '-Edict=1', '-Diplomacy=1', '-TellAll=0', '-Abdicate=1200', '-Wander=1') -Ruler -Wanderer -Ticks 6000 }

# B1 (docs/60_DECISIONS/headers/B1_wanderer_loop.md section 5): the two long runs of seed 42 as well, in the full tier -
# changes that reach only a long run (a deposition, the chronicle) cannot pass unseen.
if ($Tier -eq 'full') {
    Step 'balance: seed 42, 20,000 ticks vs baseline' { Compare-Balance 'checks_20000' 'summary_line_20000' @() -Ticks 20000 }
    Step 'balance: seed 42, 20,000 ticks with the ruler policy vs baseline' { Compare-Balance 'checks_ruler_20000' 'ruler_summary_line_20000' @('-Answer=1', '-Build=1', '-Focus=1', '-Edict=1', '-Diplomacy=1', '-TellAll=0') -Ruler -Ticks 20000 }
}

# MVP item 9: the hash run of a game process (ACosmicGameMode::StartPlay, World/CosmicHashRun) against the commandlet's
# line of this run - not the baseline file, which the steps above compare - so the editor binary shows that the hash run
# is the commandlet's run, and the cooked build's comparison (Tools/package.ps1) measures the binary alone
# (docs/60_DECISIONS/headers/A9_win64_packaging.md section 3.1). -ExecCmds=quit ends a run whose switch the game did not
# read; a good run asks to quit inside the engine's Init and never reaches it.
function Compare-HashRun([string] $Name, [string] $BalanceLog, [string[]] $Extra) {
    $log = Join-Path $Logs "$Name.log"
    foreach ($old in @($log, (Join-Path $Logs "${Name}_2.log"))) { if (Test-Path $old) { Remove-Item -Force $old } }
    if (-not (Test-Path (Join-Path $Logs "$BalanceLog.log"))) { throw "no $BalanceLog.log - the balance step did not write it" }
    & $Editor $Project -game -nullrhi -nosound -unattended -nosplash -Seed=42 -CosmicHashRun=3000 @Extra '-ExecCmds=quit' "-AbsLog=$RootF/Saved/Logs/$Name.log"
    if (-not (Test-Path $log)) { throw "no $Name.log" }
    $line = Select-String -Path $log -Pattern 'CosmicHash: ' | Select-Object -Last 1
    if (-not $line) { throw "no ""CosmicHash:"" line in $Name.log" }
    $hashRun = if ($line.Line -match 'CosmicHash: (seed \d+, \d+ ticks, hash \d+, world \d+)') { $Matches[1] } else { throw "$($line.Line)" }
    $balance = Select-String -Path (Join-Path $Logs "$BalanceLog.log") -Pattern 'CosmicBalance: seed' | Select-Object -Last 1
    if (-not $balance) { throw "no ""CosmicBalance: seed"" line in $BalanceLog.log" }
    $commandlet = if ($balance.Line -match 'CosmicBalance: (seed \d+, \d+ ticks, hash \d+, world \d+)') { $Matches[1] } else { throw "unreadable: $($balance.Line)" }
    if ($hashRun -ne $commandlet) { throw "the hash run differs from the commandlet`n   hash run:   $hashRun`n   commandlet: $commandlet" }
    Write-Host "   $hashRun"
}
Step 'hash run: editor -game vs commandlet' { Compare-HashRun 'hashrun' 'checks' @() }
Step 'hash run: editor -game policy vs commandlet' { Compare-HashRun 'hashrun_policy' 'checks_ruler' @('-CosmicHashPolicy') }

if ($failed.Count -gt 0) { Write-Host "run_checks: FAILED -> $($failed -join '; ')"; exit 1 }
Write-Host 'run_checks: all passed'
