param(
    [int[]] $Seeds = @(0, 2, 4, 7),
    [int] $SecondsPerSeed = 12,
    [string] $EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [switch] $CaptureGallery
)

$ErrorActionPreference = 'Stop'
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$Project = Join-Path $ProjectRoot 'Prototype.uproject'
$EditorCmd = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$LogDir = Join-Path $ProjectRoot 'Saved\Logs\EnvironmentSweep'

if (-not (Test-Path -LiteralPath $EditorCmd)) {
    throw "UnrealEditor-Cmd.exe not found under $EngineRoot"
}

New-Item -ItemType Directory -Force -Path $LogDir | Out-Null

foreach ($Seed in $Seeds) {
    $Log = Join-Path $LogDir "seed-$Seed.log"
    $Arguments = @(
        $Project, '/Game/Gaters/Maps/Lvl_GateGreybox',
        '-game', '-unattended', '-nop4', '-nosplash', '-NoSound',
        '-benchmark', "-seconds=$SecondsPerSeed", "-GatersSeed=$Seed", "-abslog=$Log"
    )
    if ($CaptureGallery) {
        $Arguments += @('-RenderOffscreen', '-ResX=1280', '-ResY=720', '-windowed', '-GatersGallery')
    } else {
        $Arguments += '-nullrhi'
    }
    & $EditorCmd @Arguments

    if ($LASTEXITCODE -ne 0) {
        throw "Environment seed $Seed failed with exit code $LASTEXITCODE. See $Log"
    }

    Select-String -LiteralPath $Log -Pattern `
        '\[GatersChunk\] (SITE|PLOTS|BASE|SCATTER|STAMP|GEN ms)|\[GatersTestSpawner\] gallery=' |
        ForEach-Object { $_.Line -replace '^.*\[GatersChunk\] ', '' }
}
