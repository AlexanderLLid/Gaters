param(
    [string] $Blender = 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe',
    [string] $Contract = (Join-Path $PSScriptRoot 'contracts\neutral-rock.json'),
    [string] $Output = (Join-Path $PSScriptRoot 'Derived\neutral-rock-v1')
)

$ErrorActionPreference = 'Stop'
$Generator = Join-Path $PSScriptRoot 'generate_candidate_lod.py'
foreach ($Path in @($Blender, $Contract, $Generator)) {
    if (-not (Test-Path -LiteralPath $Path)) { throw "Missing required path: $Path" }
}

& $Blender --background --factory-startup --python $Generator -- `
    --contract ([System.IO.Path]::GetFullPath($Contract)) `
    --output ([System.IO.Path]::GetFullPath($Output))
exit $LASTEXITCODE

