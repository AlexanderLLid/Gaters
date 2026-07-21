param(
    [string] $CaseId = 'impact-left',
    [string] $Blender = 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe',
    [string] $Brief = (Join-Path $PSScriptRoot 'species\humanoid.json'),
    [string] $Cases = (Join-Path $PSScriptRoot 'species\recovery-cases.json'),
    [string] $Output = (Join-Path $PSScriptRoot "Derived\humanoid-recovery-v1\$CaseId")
)

$ErrorActionPreference = 'Stop'
$Generator = Join-Path $PSScriptRoot 'generate_recovery_humanoid.py'
foreach ($Path in @($Blender, $Brief, $Cases, $Generator)) {
    if (-not (Test-Path -LiteralPath $Path)) { throw "Missing required path: $Path" }
}

& $Blender --background --factory-startup --python $Generator -- `
    --brief ([System.IO.Path]::GetFullPath($Brief)) `
    --cases ([System.IO.Path]::GetFullPath($Cases)) `
    --case-id $CaseId `
    --output ([System.IO.Path]::GetFullPath($Output))
$BlenderExitCode = $LASTEXITCODE
if ($BlenderExitCode -ne 0) { throw "Blender failed with exit code $BlenderExitCode." }
$Manifest = Join-Path $Output 'manifest.json'
if (-not (Test-Path -LiteralPath $Manifest)) {
    throw "Blender did not produce the recovery manifest: $Manifest"
}
exit 0
