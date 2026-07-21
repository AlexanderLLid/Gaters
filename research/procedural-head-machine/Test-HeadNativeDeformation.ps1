param(
    [Parameter(Mandatory = $true)] [string]$SourceRun,
    [Parameter(Mandatory = $true)] [string]$Command
)

$ErrorActionPreference = 'Stop'
$source = (Resolve-Path -LiteralPath $SourceRun).Path
$commandPath = (Resolve-Path -LiteralPath $Command).Path
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
$runRoot = Join-Path $PSScriptRoot "DeformationRuns\$stamp"
$blenderOutput = Join-Path $runRoot 'blender'
$houdiniOutput = Join-Path $runRoot 'houdini'
New-Item -ItemType Directory -Path $blenderOutput,$houdiniOutput -Force | Out-Null
$blender = 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe'
$hython = 'C:\Program Files\Side Effects Software\Houdini 22.0.368\bin\hython.exe'

$watch = [System.Diagnostics.Stopwatch]::StartNew()
& $blender -b --factory-startup --python "$PSScriptRoot\adapters\blender_deform_build.py" -- $source $commandPath $blenderOutput
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $blender -b "$blenderOutput\deformed-head.blend" --python "$PSScriptRoot\adapters\blender_deform_readback.py" -- "$blenderOutput\readback.json"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
$watch.Stop()
$blenderMs = $watch.ElapsedMilliseconds
py "$PSScriptRoot\src\deformation_verifier.py" $source $commandPath "$blenderOutput\readback.json" "$blenderOutput\verification.json"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$watch.Restart()
& $hython "$PSScriptRoot\adapters\houdini_deform_build.py" $source $commandPath $houdiniOutput
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $hython "$PSScriptRoot\adapters\houdini_deform_readback.py" "$houdiniOutput\deformed-head.hipnc" "$houdiniOutput\readback.json"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
$watch.Stop()
$houdiniMs = $watch.ElapsedMilliseconds
py "$PSScriptRoot\src\deformation_verifier.py" $source $commandPath "$houdiniOutput\readback.json" "$houdiniOutput\verification.json"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$comparison = [ordered]@{
    schema = 'head-native-deformation-comparison/0'
    source_run = $source
    command = $commandPath
    blender = [ordered]@{ elapsed_ms = $blenderMs; scene = "$blenderOutput\deformed-head.blend" }
    houdini = [ordered]@{ elapsed_ms = $houdiniMs; scene = "$houdiniOutput\deformed-head.hipnc" }
}
$comparison | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath "$runRoot\comparison.json" -Encoding utf8
Write-Output "HEAD_NATIVE_DEFORMATION_PASS root=$runRoot blender_ms=$blenderMs houdini_ms=$houdiniMs"
