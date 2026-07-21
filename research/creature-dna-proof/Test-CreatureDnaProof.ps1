param(
    [string]$Recipe = (Join-Path $PSScriptRoot 'recipes\winged-reptile.json'),
    [string]$OutputRoot = '',
    [ValidateRange(1, 10)]
    [int]$Repeat = 2
)

$ErrorActionPreference = 'Stop'
$installRoot = 'C:\Program Files\Side Effects Software'
$houdiniRoot = Get-ChildItem -LiteralPath $installRoot -Directory |
    Where-Object { $_.Name -match '^Houdini \d' } |
    Sort-Object Name -Descending |
    Select-Object -First 1
if (-not $houdiniRoot) { throw 'HOUDINI_NOT_FOUND' }
$hythonPath = Join-Path $houdiniRoot.FullName 'bin\hython.exe'
if (-not (Test-Path -LiteralPath $hythonPath)) { throw 'HOUDINI_NOT_FOUND' }

if (-not $OutputRoot) {
    $runStamp = (Get-Date).ToUniversalTime().ToString('yyyyMMddTHHmmssZ')
    $OutputRoot = Join-Path $PSScriptRoot "Runs\$runStamp"
}

py -m unittest discover -s (Join-Path $PSScriptRoot 'tests') -v
if ($LASTEXITCODE -ne 0) { throw 'CREATURE_DNA_TESTS_FAILED' }

$hashes = @()
for ($runIndex = 1; $runIndex -le $Repeat; $runIndex++) {
    $runPath = Join-Path $OutputRoot "run-$runIndex"
    & $hythonPath (Join-Path $PSScriptRoot 'houdini\build_guides.py') $Recipe $runPath
    if ($LASTEXITCODE -ne 0) { throw 'HOUDINI_GUIDE_BUILD_FAILED' }
    $scenePath = Get-ChildItem -LiteralPath $runPath -Filter 'creature-guide.hip*' | Select-Object -ExpandProperty FullName -First 1
    & $hythonPath (Join-Path $PSScriptRoot 'houdini\inspect_guides.py') `
        $scenePath `
        (Join-Path $runPath 'anatomy-graph.json') `
        (Join-Path $runPath 'scene-verification.json')
    if ($LASTEXITCODE -ne 0) { throw 'HOUDINI_SCENE_VERIFICATION_FAILED' }
    $hashes += (Get-Content -LiteralPath (Join-Path $runPath 'run.json') -Raw | ConvertFrom-Json).graph_sha256
}

if (($hashes | Select-Object -Unique).Count -ne 1) { throw 'CREATURE_DNA_REPRO_FAILED' }
Write-Output "CREATURE_DNA_REPRO_PASS runs=$Repeat graph_sha256=$($hashes[0]) output=$OutputRoot"
