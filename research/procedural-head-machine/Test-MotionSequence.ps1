param(
    [Parameter(Mandatory = $true)] [string]$SourceSkinRun,
    [Parameter(Mandatory = $true)] [string]$MotionRun
)

$ErrorActionPreference = 'Stop'
$source = (Resolve-Path -LiteralPath $SourceSkinRun).Path
$motion = (Resolve-Path -LiteralPath $MotionRun).Path
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
$root = Join-Path $PSScriptRoot "MotionSequenceRuns\$stamp"
New-Item -ItemType Directory -Path $root -Force | Out-Null
$hython = 'C:\Program Files\Side Effects Software\Houdini 22.0.368\bin\hython.exe'
if (-not (Test-Path -LiteralPath $hython)) { throw 'HOUDINI_NOT_FOUND' }

foreach ($index in 1..2) {
    $run = Join-Path $root "run-$index"
    New-Item -ItemType Directory -Path $run | Out-Null
    & $hython "$PSScriptRoot\adapters\houdini_motion_sequence_build.py" "$source\captured-mannequin.hipnc" $motion $run
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & $hython "$PSScriptRoot\adapters\houdini_motion_sequence_readback.py" "$run\generated-motion.hipnc" "$motion\motion.json" "$run\readback.json"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    py "$PSScriptRoot\src\motion_sequence_verifier.py" "$source\readback.json" "$source\captured-mannequin.hipnc" $motion "$run\readback.json" "$run\verification.json"
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
$hash1 = (Get-FileHash -Algorithm SHA256 -LiteralPath "$root\run-1\readback.json").Hash
$hash2 = (Get-FileHash -Algorithm SHA256 -LiteralPath "$root\run-2\readback.json").Hash
if ($hash1 -ne $hash2) { throw 'MOTION_SEQUENCE_REPLAY_MISMATCH' }
py "$PSScriptRoot\src\render_motion_sequence.py" "$root\run-1\readback.json" "$root\run-1\verification.json" "$root\preview.png"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Output "MOTION_SEQUENCE_RUN_PASS root=$root readback_sha256=$hash1"
