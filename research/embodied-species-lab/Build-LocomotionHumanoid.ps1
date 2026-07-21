param(
    [string] $Blender = 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe',
    [string] $Brief = (Join-Path $PSScriptRoot 'species\humanoid.json'),
    [string] $Output = (Join-Path $PSScriptRoot 'Derived\humanoid-locomotion-v1')
)

$ErrorActionPreference = 'Stop'
$Generator = Join-Path $PSScriptRoot 'generate_locomotion_humanoid.py'
$LabRoot = [System.IO.Path]::GetFullPath($PSScriptRoot).TrimEnd([System.IO.Path]::DirectorySeparatorChar)
$ExpectedOutput = [System.IO.Path]::GetFullPath(
    (Join-Path $PSScriptRoot 'Derived\humanoid-locomotion-v1')
)
$ResolvedBrief = [System.IO.Path]::GetFullPath($Brief)
$ResolvedOutput = [System.IO.Path]::GetFullPath($Output)

foreach ($Path in @($Blender, $Brief, $Generator)) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "Missing required file: $Path"
    }
}
if (-not $ResolvedBrief.StartsWith($LabRoot + [System.IO.Path]::DirectorySeparatorChar)) {
    throw "Brief must resolve under the lab: $ResolvedBrief"
}
if ($ResolvedOutput -cne $ExpectedOutput) {
    throw "Output must be the isolated locomotion directory: $ExpectedOutput"
}

& $Blender --background --factory-startup --python $Generator -- `
    --brief $ResolvedBrief `
    --output $ResolvedOutput
$BlenderExitCode = $LASTEXITCODE
if ($BlenderExitCode -ne 0) {
    throw "Blender failed with exit code $BlenderExitCode."
}

$Manifest = Join-Path $ResolvedOutput 'locomotion-manifest.json'
if (-not (Test-Path -LiteralPath $Manifest -PathType Leaf)) {
    throw "Blender did not produce the locomotion manifest: $Manifest"
}
exit 0
