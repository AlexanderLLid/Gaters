param(
    [string]$Output = (Join-Path $PSScriptRoot 'Derived/depth-v1'),
    [string]$Blender = 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe'
)

$ErrorActionPreference = 'Stop'

Push-Location $PSScriptRoot
try {
    python -m unittest test_depth_fit.py test_depth_blender_contract.py -v
    if ($LASTEXITCODE -ne 0) { throw 'Depth proof tests failed' }

    & $Blender --background --python-exit-code 1 --python (Join-Path $PSScriptRoot 'build_depth_proof.py') -- --output $Output
    if ($LASTEXITCODE -ne 0) { throw 'Blender depth build failed' }

    & $Blender --background --python-exit-code 1 (Join-Path $Output 'picture-to-mesh-depth.blend') `
        --python (Join-Path $PSScriptRoot 'verify_depth_proof.py') -- --output $Output
    if ($LASTEXITCODE -ne 0) { throw 'Blender depth verification failed' }

    $report = Get-Content -LiteralPath (Join-Path $Output 'verification.json') -Raw | ConvertFrom-Json
    Write-Output "PICTURE_TO_MESH_DEPTH_PROOF_OK output=$Output depthRmse=$($report.depthRmse)"
}
finally {
    Pop-Location
}
