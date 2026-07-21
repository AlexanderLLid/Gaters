param(
    [Parameter(Mandatory = $true)] [string]$BodyRun,
    [Parameter(Mandatory = $true)] [string]$GuideRun,
    [string]$Recipe = "$PSScriptRoot\recipes\anatomical-mannequin-surface.json"
)

$ErrorActionPreference = 'Stop'
$body = (Resolve-Path -LiteralPath $BodyRun).Path
$guide = (Resolve-Path -LiteralPath $GuideRun).Path
$recipePath = (Resolve-Path -LiteralPath $Recipe).Path
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
$root = Join-Path $PSScriptRoot "AnatomicalSurfaceRuns\anatomical-mannequin-$stamp"
$hython = 'C:\Program Files\Side Effects Software\Houdini 22.0.368\bin\hython.exe'
if (-not (Test-Path -LiteralPath $hython)) { throw 'HOUDINI_NOT_FOUND' }

foreach ($index in 1,2) {
    $run = Join-Path $root "run-$index"
    New-Item -ItemType Directory -Path $run -Force | Out-Null
    & $hython "$PSScriptRoot\adapters\houdini_smooth_body_build.py" $body $guide $recipePath $run
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & $hython "$PSScriptRoot\adapters\houdini_smooth_body_readback.py" "$run\smooth-body.hipnc" "$run\readback.json"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    py "$PSScriptRoot\src\anatomical_surface_verifier.py" $body $guide $recipePath "$run\readback.json" "$run\verification.json"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
py "$PSScriptRoot\src\summarize_smooth_body.py" $body $recipePath $root --guide-run $guide
exit $LASTEXITCODE
