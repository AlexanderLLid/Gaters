param(
    [Parameter(Mandatory = $true)] [string]$SourceRun,
    [Parameter(Mandatory = $true)] [string]$Socket
)

$ErrorActionPreference = 'Stop'
$source = (Resolve-Path -LiteralPath $SourceRun).Path
$socketPath = (Resolve-Path -LiteralPath $Socket).Path
py "$PSScriptRoot\src\run_socket.py" $source $socketPath --output-root "$PSScriptRoot\SocketRuns" --repeat 2
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
