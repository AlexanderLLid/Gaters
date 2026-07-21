param(
    [Parameter(Mandatory = $true)]
    [string]$CompositionRun
)

$ErrorActionPreference = 'Stop'
$source = (Resolve-Path -LiteralPath $CompositionRun).Path
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss-fff'
$runRoot = Join-Path $PSScriptRoot "CompositionAdapterRuns\$stamp"
New-Item -ItemType Directory -Path $runRoot -Force | Out-Null
$hython = 'C:\Program Files\Side Effects Software\Houdini 22.0.368\bin\hython.exe'
if (-not (Test-Path -LiteralPath $hython)) { throw 'HOUDINI_NOT_FOUND' }

$watch = [System.Diagnostics.Stopwatch]::StartNew()
& $hython "$PSScriptRoot\adapters\houdini_socket_build.py" $source $runRoot
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $hython "$PSScriptRoot\adapters\houdini_socket_readback.py" "$runRoot\composed-character.hipnc" "$runRoot\readback.json"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
$watch.Stop()
py "$PSScriptRoot\src\socket_adapter_verifier.py" $source "$runRoot\readback.json" "$runRoot\verification.json"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$summary = [ordered]@{
    schema = 'module-composition-houdini-adapter-run/0'
    source_run = $source
    elapsed_ms = $watch.ElapsedMilliseconds
    scene = "$runRoot\composed-character.hipnc"
}
$summary | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath "$runRoot\summary.json" -Encoding utf8
Write-Output "MODULE_COMPOSITION_HOUDINI_ADAPTER_PASS root=$runRoot elapsed_ms=$($watch.ElapsedMilliseconds)"
