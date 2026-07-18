param(
    [string] $Blender = 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe',
    [string] $Brief = (Join-Path $PSScriptRoot 'contracts\neutral-motion.json'),
    [string] $Output = (Join-Path $PSScriptRoot 'Derived\neutral-motion-v1')
)

$ErrorActionPreference = 'Stop'
$Generator = Join-Path $PSScriptRoot 'generate_motion_fixture.py'
foreach ($Path in @($Blender, $Brief, $Generator)) {
    if (-not (Test-Path -LiteralPath $Path)) { throw "Missing required path: $Path" }
}

& $Blender --background --factory-startup --python $Generator -- `
    --brief ([System.IO.Path]::GetFullPath($Brief)) `
    --output ([System.IO.Path]::GetFullPath($Output))
exit $LASTEXITCODE
