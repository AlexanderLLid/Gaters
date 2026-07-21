param(
    [Parameter(Mandatory = $true)] [string]$SurfaceRun,
    [Parameter(Mandatory = $true)] [string]$SkeletonRun,
    [string]$Recipe = "$PSScriptRoot\recipes\proximity-skin-capture.json"
)

$ErrorActionPreference = 'Stop'
$surface = (Resolve-Path -LiteralPath $SurfaceRun).Path
$skeleton = (Resolve-Path -LiteralPath $SkeletonRun).Path
$recipePath = (Resolve-Path -LiteralPath $Recipe).Path
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
$runRoot = Join-Path $PSScriptRoot "SkinCaptureRuns\$stamp"
New-Item -ItemType Directory -Path $runRoot -Force | Out-Null
$hython = 'C:\Program Files\Side Effects Software\Houdini 22.0.368\bin\hython.exe'
if (-not (Test-Path -LiteralPath $hython)) { throw 'HOUDINI_NOT_FOUND' }

& $hython "$PSScriptRoot\adapters\houdini_skin_capture_build.py" $surface $skeleton $recipePath $runRoot
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $hython "$PSScriptRoot\adapters\houdini_skin_capture_readback.py" "$runRoot\captured-mannequin.hipnc" "$runRoot\readback.json"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
$recipeObject = Get-Content -Raw -LiteralPath $recipePath | ConvertFrom-Json
if ($null -ne $recipeObject.diagnostic_poses) {
    py "$PSScriptRoot\src\pose_suite_verifier.py" $surface $skeleton $recipePath "$runRoot\readback.json" "$runRoot\verification.json"
    $verificationExit = $LASTEXITCODE
    py "$PSScriptRoot\src\render_pose_suite.py" "$runRoot\readback.json" "$runRoot\verification.json" "$runRoot\preview.png"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    if ($verificationExit -ne 0) { exit $verificationExit }
} else {
    py "$PSScriptRoot\src\skin_capture_verifier.py" $surface $skeleton $recipePath "$runRoot\readback.json" "$runRoot\verification.json"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    py "$PSScriptRoot\src\render_skin_capture_preview.py" "$runRoot\readback.json" "$runRoot\preview.png"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    py "$PSScriptRoot\src\render_skin_pose_preview.py" "$runRoot\readback.json" "$runRoot\pose-preview.png"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
Write-Output "SKIN_CAPTURE_RUN_PASS root=$runRoot"
