$ErrorActionPreference = 'Stop'

$Verifier = Join-Path $PSScriptRoot 'Test-GalleryImage.ps1'
$TempRoot = Join-Path ([IO.Path]::GetTempPath()) "gaters-gallery-verifier-$([guid]::NewGuid())"
New-Item -ItemType Directory -Force -Path $TempRoot | Out-Null
Add-Type -AssemblyName System.Drawing

function Write-SolidImage([string] $Path, [Drawing.Color] $Color) {
    $Bitmap = [Drawing.Bitmap]::new(64, 64)
    try {
        $Graphics = [Drawing.Graphics]::FromImage($Bitmap)
        try { $Graphics.Clear($Color) } finally { $Graphics.Dispose() }
        $Bitmap.Save($Path, [Drawing.Imaging.ImageFormat]::Png)
    } finally {
        $Bitmap.Dispose()
    }
}

try {
    $Neutral = Join-Path $TempRoot 'neutral.png'
    $Warm = Join-Path $TempRoot 'warm.png'
    $Dark = Join-Path $TempRoot 'dark.png'
    Write-SolidImage $Neutral ([Drawing.Color]::FromArgb(100, 100, 100))
    Write-SolidImage $Warm ([Drawing.Color]::FromArgb(150, 40, 15))
    Write-SolidImage $Dark ([Drawing.Color]::FromArgb(10, 10, 10))

    & $Verifier -ImagePath $Neutral | Out-Null
    & $Verifier -ImagePath $Warm | Out-Null
    try {
        & $Verifier -ImagePath $Dark | Out-Null
        throw 'Dark fixture unexpectedly passed.'
    } catch {
        if ($_.Exception.Message -eq 'Dark fixture unexpectedly passed.') { throw }
    }
    'PASS neutral=accepted warm=accepted dark=rejected'
} finally {
    Remove-Item -LiteralPath $TempRoot -Recurse -Force -ErrorAction SilentlyContinue
}
