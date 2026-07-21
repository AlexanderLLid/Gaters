param(
    [string]$Recipe = "$PSScriptRoot\recipes\baseline.json"
)

$ErrorActionPreference = 'Stop'
py "$PSScriptRoot\src\run_machine.py" $Recipe --output-root "$PSScriptRoot\Runs" --repeat 2
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
