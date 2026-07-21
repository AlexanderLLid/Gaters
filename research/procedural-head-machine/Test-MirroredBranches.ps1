param(
    [string]$Parent = "$PSScriptRoot\recipes\torso-branch-parent.json",
    [string]$Branch = "$PSScriptRoot\recipes\torso-mirrored-branches.json"
)

$ErrorActionPreference = 'Stop'
$parentPath = (Resolve-Path -LiteralPath $Parent).Path
$branchPath = (Resolve-Path -LiteralPath $Branch).Path
py "$PSScriptRoot\src\run_branches.py" $parentPath $branchPath --output-root "$PSScriptRoot\BranchRuns"
exit $LASTEXITCODE
