param(
    [string]$Output = (Join-Path $PSScriptRoot 'Derived/proof-v1'),
    [string]$Blender = 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe'
)

$ErrorActionPreference = 'Stop'

Push-Location $PSScriptRoot
try {
    python -m unittest discover -p 'test_*.py' -v
    if ($LASTEXITCODE -ne 0) { throw 'Python tests failed' }

    & $Blender --background --python-exit-code 1 --python (Join-Path $PSScriptRoot 'build_proof.py') -- --output $Output
    if ($LASTEXITCODE -ne 0) { throw 'Blender build failed' }

    & $Blender --background --python-exit-code 1 (Join-Path $Output 'picture-to-mesh.blend') `
        --python (Join-Path $PSScriptRoot 'verify_proof.py') -- --output $Output
    if ($LASTEXITCODE -ne 0) { throw 'Blender verification failed' }

    $report = Get-Content -LiteralPath (Join-Path $Output 'verification.json') -Raw | ConvertFrom-Json
    if (-not $report.promoted) { throw 'Picture-to-mesh proof was not promoted' }
    Write-Output "PICTURE_TO_MESH_PROOF_OK output=$Output frontIoU=$($report.fitted.frontIoU)"
}
finally {
    Pop-Location
}
