param(
    [Parameter(Mandatory = $true)]
    [string]$SourceRun
)

$ErrorActionPreference = 'Stop'
$source = (Resolve-Path -LiteralPath $SourceRun).Path
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
$runRoot = Join-Path $PSScriptRoot "AdapterRuns\$stamp"
$blenderOutput = Join-Path $runRoot 'blender'
$houdiniOutput = Join-Path $runRoot 'houdini'
New-Item -ItemType Directory -Path $blenderOutput,$houdiniOutput -Force | Out-Null

$blender = 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe'
$hython = 'C:\Program Files\Side Effects Software\Houdini 22.0.368\bin\hython.exe'
if (-not (Test-Path -LiteralPath $blender)) { throw 'BLENDER_NOT_FOUND' }
if (-not (Test-Path -LiteralPath $hython)) { throw 'HOUDINI_NOT_FOUND' }

$watch = [System.Diagnostics.Stopwatch]::StartNew()
& $blender -b --factory-startup --python "$PSScriptRoot\adapters\blender_build.py" -- $source $blenderOutput
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $blender -b "$blenderOutput\head.blend" --python "$PSScriptRoot\adapters\blender_readback.py" -- "$blenderOutput\readback.json"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
$watch.Stop()
$blenderMs = $watch.ElapsedMilliseconds
py "$PSScriptRoot\src\adapter_verifier.py" $source "$blenderOutput\readback.json" "$blenderOutput\verification.json"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$watch.Restart()
& $hython "$PSScriptRoot\adapters\houdini_build.py" $source $houdiniOutput
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $hython "$PSScriptRoot\adapters\houdini_readback.py" "$houdiniOutput\head.hipnc" "$houdiniOutput\readback.json"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
$watch.Stop()
$houdiniMs = $watch.ElapsedMilliseconds
py "$PSScriptRoot\src\adapter_verifier.py" $source "$houdiniOutput\readback.json" "$houdiniOutput\verification.json"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$comparison = [ordered]@{
    schema = 'head-backend-comparison/0'
    source_run = $source
    blender = [ordered]@{ elapsed_ms = $blenderMs; scene = "$blenderOutput\head.blend" }
    houdini = [ordered]@{ elapsed_ms = $houdiniMs; scene = "$houdiniOutput\head.hipnc" }
}
$comparison | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath "$runRoot\comparison.json" -Encoding utf8
Write-Output "HEAD_BACKEND_COMPARISON_PASS root=$runRoot blender_ms=$blenderMs houdini_ms=$houdiniMs"
