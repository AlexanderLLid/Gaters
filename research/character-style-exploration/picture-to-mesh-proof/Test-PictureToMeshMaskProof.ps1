param(
    [string]$Output = (Join-Path $PSScriptRoot 'Derived/mask-v2'),
    [string]$Blender = 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe'
)

$ErrorActionPreference = 'Stop'

Push-Location $PSScriptRoot
try {
    python -m unittest test_mask_fit.py test_mask_blender_contract.py -v
    if ($LASTEXITCODE -ne 0) { throw 'Mask proof tests failed' }

    foreach ($variant in @('balanced', 'broad', 'crooked')) {
        $variantOutput = Join-Path $Output $variant
        & $Blender --background --python-exit-code 1 --python (Join-Path $PSScriptRoot 'build_mask_proof.py') `
            -- --output $variantOutput --variant $variant
        if ($LASTEXITCODE -ne 0) { throw "Mask build failed: $variant" }

        & $Blender --background --python-exit-code 1 (Join-Path $variantOutput 'picture-to-mesh-mask.blend') `
            --python (Join-Path $PSScriptRoot 'verify_mask_proof.py') -- --output $variantOutput
        if ($LASTEXITCODE -ne 0) { throw "Mask verification failed: $variant" }

        $report = Get-Content -LiteralPath (Join-Path $variantOutput 'verification.json') -Raw | ConvertFrom-Json
        Write-Output "MASK_VARIANT_OK variant=$variant iou=$($report.frontSilhouetteIoU) stretch=$($report.maximumEdgeStretch)"
    }
    Write-Output "PICTURE_TO_MESH_MASK_PROOF_OK output=$Output"
}
finally {
    Pop-Location
}
