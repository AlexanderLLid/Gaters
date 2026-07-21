param(
    [string] $Blender = 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe',
    [string] $Brief = (Join-Path $PSScriptRoot 'face-brief.json'),
    [string] $Output = (Join-Path $PSScriptRoot 'Derived\face-v2'),
    [switch] $PreflightOnly
)

$ErrorActionPreference = 'Stop'
if (-not (Test-Path -LiteralPath $Blender)) { throw "Missing Blender: $Blender" }

$Probe = @'
import importlib
module = importlib.import_module('bl_ext.blender_org.mpfb')
if not module:
    raise RuntimeError('MPFB module did not load')
print('MPFB_PREFLIGHT_OK')
'@
& $Blender --background --python-exit-code 2 --python-expr $Probe
if ($LASTEXITCODE -ne 0) { throw "MPFB preflight failed with exit code $LASTEXITCODE" }
if ($PreflightOnly) {
    Write-Output 'PASS mpfb-preflight'
    return
}

$Generator = Join-Path $PSScriptRoot 'generate_face.py'
foreach ($Path in @($Brief, $Generator)) {
    if (-not (Test-Path -LiteralPath $Path)) { throw "Missing required path: $Path" }
}

$Recipe = Get-Content -LiteralPath $Brief -Raw | ConvertFrom-Json
if ($Recipe.schemaVersion -ne 2 -or $Recipe.generatorVersion -ne 2) { throw 'Unexpected face recipe version' }
if ($Recipe.proofId -ne 'art.face.highland-a1.v2') { throw 'Unexpected recipe proof identity' }
if ($Recipe.styleId -ne 'gaters.clean-midpoly-painted-1') { throw 'Unexpected recipe style identity' }
if ($Recipe.target.ageYears -lt 35) { throw 'Face recipe is not a mature adult' }
if (@($Recipe.target.controls).Count -lt 12) { throw 'Face recipe lacks named controls' }
$Asymmetry = @($Recipe.target.controls | Where-Object { $_.name -like 'asymmetry.*' })
if ($Asymmetry.Count -lt 3) { throw 'Face recipe lacks restrained asymmetry controls' }
foreach ($Control in $Recipe.target.controls) {
    if (-not $Control.name -or -not $Control.section -or -not $Control.target) { throw 'Malformed named face control' }
    if ($Control.weight -le 0.0 -or $Control.weight -gt 0.8) { throw "Invalid control weight: $($Control.name)" }
}
if (($Recipe.render.views -join ',') -ne 'front,three-quarter,profile') { throw 'Unexpected fixed render views' }
$AssetArchive = Join-Path $PSScriptRoot 'Derived\makehuman_system_assets_cc0.zip'
if (-not (Test-Path -LiteralPath $AssetArchive)) { throw 'Missing MakeHuman CC0 asset archive' }
$AssetHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $AssetArchive).Hash.ToLowerInvariant()
if ($AssetHash -ne $Recipe.assetPack.archiveSha256) { throw "MakeHuman asset archive hash mismatch: $AssetHash" }
if ($Recipe.assetPack.license -ne 'CC0-1.0') { throw 'Unexpected external asset license' }
$AssetRoot = Join-Path $PSScriptRoot $Recipe.assetPack.root
foreach ($AssetName in @('skin', 'eyes', 'eyebrows', 'hair')) {
    $Asset = $Recipe.assets.$AssetName
    if (-not $Asset.id -or -not $Asset.path -or -not $Asset.materialModel) { throw "Malformed asset contract: $AssetName" }
    if (-not (Test-Path -LiteralPath (Join-Path $AssetRoot $Asset.path))) { throw "Missing contracted asset: $AssetName" }
}

& $Blender --background --python-exit-code 2 --python $Generator -- `
    --brief ([System.IO.Path]::GetFullPath($Brief)) `
    --output ([System.IO.Path]::GetFullPath($Output))
if ($LASTEXITCODE -ne 0) { throw "Blender face proof failed with exit code $LASTEXITCODE" }

$Required = @('face-proof.blend', 'manifest.json', 'front.png', 'three-quarter.png', 'profile.png')
foreach ($Name in $Required) {
    $Path = Join-Path $Output $Name
    if (-not (Test-Path -LiteralPath $Path)) { throw "Missing derived artifact: $Path" }
}

$Manifest = Get-Content -LiteralPath (Join-Path $Output 'manifest.json') -Raw | ConvertFrom-Json
if ($Manifest.schemaVersion -ne 2) { throw 'Unexpected manifest schema' }
if ($Manifest.proofId -ne 'art.face.highland-a1.v2') { throw 'Unexpected proof identity' }
if ($Manifest.styleId -ne 'gaters.clean-midpoly-painted-1') { throw 'Unexpected style identity' }
if (-not $Manifest.art3Isolation) { throw 'ART-3 isolation was not recorded' }
if ($Manifest.hasArmature -or $Manifest.hasAnimation) { throw 'Visual proof contains character-pipeline data' }
if ($Manifest.foundation -ne 'MPFB') { throw 'Face proof did not use MPFB' }
if (-not $Manifest.stableTopology) { throw 'Stable topology was not recorded' }
if (@($Manifest.views).Count -ne 3) { throw 'Expected exactly three views' }
if ($Manifest.baseVertexCount -ne 19158) { throw 'MPFB stable topology changed unexpectedly' }
if ($Manifest.headVertexCount -lt 4000) { throw 'Head region is below the proof floor' }
if (@($Manifest.namedControls).Count -lt 6) { throw 'Named facial controls are missing' }
if ($Manifest.assetPack.id -ne 'makehuman_system_assets_cc0' -or $Manifest.assetPack.license -ne 'CC0-1.0') { throw 'Asset pack provenance is missing' }
if ($Manifest.sourceAssets.skin.id -ne 'old_african_female') { throw 'Contracted skin was not applied' }
if ($Manifest.sourceAssets.eyes.id -ne 'high-poly') { throw 'Contracted eyes were not applied' }
if ($Manifest.sourceAssets.eyes.materialModel -ne 'MAKESKIN') { throw 'Eyes are not using the texture-backed material path' }
if ($Manifest.sourceAssets.eyebrows.id -ne 'eyebrow003') { throw 'Contracted eyebrows were not applied' }
if ($Manifest.sourceAssets.hair.id -ne 'braid01') { throw 'Contracted hair was not applied' }
if (@($Manifest.officialAssetObjects).Count -lt 3) { throw 'Official face assets were not instantiated' }
if ($Manifest.lightEnergyTotal -le 0 -or $Manifest.lightEnergyTotal -gt 120) { throw "Portrait light energy is outside the proof range: $($Manifest.lightEnergyTotal)" }
if ($Manifest.lookDev.stylePass -ne 'painted-grounded-v1') { throw 'Grounded painted look-dev pass was not applied' }
if ($Manifest.lookDev.skinRoughness -lt 0.75) { throw 'Skin surface is too glossy for the selected direction' }

Add-Type -AssemblyName System.Drawing
foreach ($Name in @('front.png', 'three-quarter.png', 'profile.png')) {
    $Path = Join-Path $Output $Name
    $Image = [System.Drawing.Image]::FromFile($Path)
    try {
        if ($Image.Width -ne 768 -or $Image.Height -ne 768) { throw "Unexpected render size: $Name" }
    } finally {
        $Image.Dispose()
    }
}

$FrontPath = Join-Path $Output 'front.png'
$Front = [System.Drawing.Bitmap]::FromFile($FrontPath)
try {
    $LumaTotal = 0.0
    $DarkSamples = 0
    $SampleCount = 0
    for ($Y = 0; $Y -lt $Front.Height; $Y += 8) {
        for ($X = 0; $X -lt $Front.Width; $X += 8) {
            $Pixel = $Front.GetPixel($X, $Y)
            $Luma = (($Pixel.R * 0.2126) + ($Pixel.G * 0.7152) + ($Pixel.B * 0.0722)) / 255.0
            $LumaTotal += $Luma
            if ($Luma -lt 0.25) { $DarkSamples++ }
            $SampleCount++
        }
    }
    $AverageLuma = $LumaTotal / $SampleCount
    $DarkRatio = $DarkSamples / $SampleCount
    if ($AverageLuma -ge 0.78) { throw "Front render is washed out: average luma $AverageLuma" }
    if ($DarkRatio -le 0.02) { throw "Front render has no usable dark-value structure: ratio $DarkRatio" }

    $SkinLumaTotal = 0.0
    $SkinSampleCount = 0
    for ($Y = 280; $Y -le 450; $Y += 4) {
        for ($X = 235; $X -le 315; $X += 4) {
            $Pixel = $Front.GetPixel($X, $Y)
            $SkinLumaTotal += (($Pixel.R * 0.2126) + ($Pixel.G * 0.7152) + ($Pixel.B * 0.0722)) / 255.0
            $SkinSampleCount++
        }
    }
    $SkinLuma = $SkinLumaTotal / $SkinSampleCount
    if ($SkinLuma -ge 0.58 -or $SkinLuma -le 0.12) { throw "Dark-skin value is outside the proof range: $SkinLuma" }
} finally {
    $Front.Dispose()
}

Write-Output "PASS parametric-face-proof vertices=$($Manifest.headVertexCount) controls=$(@($Manifest.namedControls).Count) views=$(@($Manifest.views).Count) luma=$('{0:N3}' -f $AverageLuma) skin=$('{0:N3}' -f $SkinLuma)"
