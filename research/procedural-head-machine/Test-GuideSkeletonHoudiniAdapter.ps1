param(
    [Parameter(Mandatory = $true)] [string]$SkeletonRun
)

$ErrorActionPreference = 'Stop'
$source = (Resolve-Path -LiteralPath $SkeletonRun).Path
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
$runRoot = Join-Path $PSScriptRoot "SkeletonAdapterRuns\$stamp"
New-Item -ItemType Directory -Path $runRoot -Force | Out-Null
$hython = 'C:\Program Files\Side Effects Software\Houdini 22.0.368\bin\hython.exe'
if (-not (Test-Path -LiteralPath $hython)) { throw 'HOUDINI_NOT_FOUND' }

& $hython "$PSScriptRoot\adapters\houdini_guide_skeleton_build.py" $source $runRoot
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $hython "$PSScriptRoot\adapters\houdini_guide_skeleton_readback.py" "$runRoot\guide-skeleton.hipnc" "$runRoot\readback.json"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
py "$PSScriptRoot\src\guide_skeleton_adapter_verifier.py" $source "$runRoot\readback.json" "$runRoot\verification.json"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$summary = [ordered]@{
    schema = 'guide-skeleton-houdini-adapter-run/0'
    source_run = $source
    scene = "$runRoot\guide-skeleton.hipnc"
}
$summary | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath "$runRoot\summary.json" -Encoding utf8
Write-Output "GUIDE_SKELETON_HOUDINI_ADAPTER_PASS root=$runRoot"
