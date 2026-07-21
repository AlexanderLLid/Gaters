param(
    [string] $EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [string] $Python = 'C:\Users\alexa\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
)

$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'Test-FootPlacementMachine.ps1') `
    -EngineRoot $EngineRoot -Python $Python -Animated
exit $LASTEXITCODE
