# Cosmic Ascension - full check runner for the Windows development PC (PowerShell 5.1).
# docs/40_DEV_GUIDE.md section 7. Order: editor-free checks -> (optional) data assets -> builds -> automation tests twice
# -> headless smoke -> balance commandlet against Tools/baselines/seed42.json (the idle run, then the run with the ruler's
# policy -Answer=1 -Build=1 -Focus=1).
#
#   powershell -ExecutionPolicy Bypass -File Tools\run_checks.ps1 [-Tier smoke|full] [-Data] [-UpdateBaseline] [-SkipBuild]
#
#   -Tier smoke   (default) automation filter "Cosmic_Ascension.Sim.Determinism+Cosmic_Ascension.Sim.Save+Cosmic_Ascension.Data"
#   -Tier full    every Cosmic_Ascension.* test (the 20,000-tick runs included)
#   -Data         regenerate the DataTable / String Table assets from Tools/data before building (editor closed)
#   -UpdateBaseline  write the commandlet's lines into Tools/baselines/seed42.json instead of comparing
#
# Paths come from $env:UE_ROOT and $env:PROJECT_ROOT (defaults below are this PC's). The editor must be closed for the
# editor-target build. Written on Linux without a Windows run on 2026-10-19: the first run on the PC may need a fix -
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

# One commandlet run of seed 42 for 3,000 ticks, compared with the baseline field $Field (or written into it with
# -UpdateBaseline): the hash line, and with $Ruler also the ruler line after ' | '.
function Compare-Balance([string] $Name, [string] $Field, [string[]] $Extra, [switch] $Ruler) {
    $log = Join-Path $Logs "$Name.log"
    & $Editor $Project -run=CosmicBalance -Seed=42 -Ticks=3000 "-Out=$Name.csv" @Extra -unattended -nosplash -stdout -FullStdOutLogOutput "-AbsLog=$RootF/Saved/Logs/$Name.log"
    $line = Select-String -Path $log -Pattern 'CosmicBalance: seed' | Select-Object -Last 1
    if (-not $line) { throw "no ""CosmicBalance: seed"" summary line in $Name.log" }
    $summary = if ($line.Line -match '(CosmicBalance: seed.*)') { $Matches[1] } else { $line.Line }
    if ($Ruler) {
        $rulerLine = Select-String -Path $log -Pattern 'CosmicBalance ruler:' | Select-Object -Last 1
        if (-not $rulerLine) { throw "no ""CosmicBalance ruler:"" line in $Name.log" }
        $summary += ' | ' + $(if ($rulerLine.Line -match '(CosmicBalance ruler:.*)') { $Matches[1] } else { $rulerLine.Line })
    }
    $baselinePath = Join-Path $Root 'Tools\baselines\seed42.json'
    $baseline = Get-Content $baselinePath -Raw | ConvertFrom-Json
    if ($UpdateBaseline) {
        if ($baseline.PSObject.Properties.Name -contains $Field) { $baseline.$Field = $summary } else { $baseline | Add-Member -NotePropertyName $Field -NotePropertyValue $summary }
        $baseline.verified_at = (Get-Date -Format 'yyyy-MM-dd')
        # UTF-8 without a byte order mark (Set-Content -Encoding UTF8 of PowerShell 5.1 writes one; Python's json reads none).
        [IO.File]::WriteAllText($baselinePath, ($baseline | ConvertTo-Json -Depth 4), (New-Object Text.UTF8Encoding $false))
        Write-Host "   baseline updated: $summary"
    } elseif ($baseline.$Field -and ($baseline.$Field -ne $summary)) {
        throw "$Field differs from Tools/baselines/seed42.json`n   now:      $summary`n   baseline: $($baseline.$Field)`n   (intended change -> rerun with -UpdateBaseline and put the new hashes in the commit message)"
    } else { Write-Host "   $summary" }
}

Step 'balance: seed 42 vs baseline' { Compare-Balance 'checks' 'summary_line' @() }

# The ruler's policy (docs/40_DEV_GUIDE.md section 9.3): answers, the project key and the research focus (MVP item 7)
# between ticks, the scope of telling of a game. It keeps the inputs deterministic across builds.
Step 'balance: seed 42 with the ruler policy vs baseline' { Compare-Balance 'checks_ruler' 'ruler_summary_line' @('-Answer=1', '-Build=1', '-Focus=1', '-TellAll=0') -Ruler }

if ($failed.Count -gt 0) { Write-Host "run_checks: FAILED -> $($failed -join '; ')"; exit 1 }
Write-Host 'run_checks: all passed'
