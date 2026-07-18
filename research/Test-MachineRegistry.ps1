$ErrorActionPreference = 'Stop'

$Show = Join-Path $PSScriptRoot 'Show-MachineRegistry.ps1'
$Registry = Join-Path $PSScriptRoot 'machines.json'

if (-not (Test-Path -LiteralPath $Show)) {
    throw "Missing registry reader: $Show"
}
if (-not (Test-Path -LiteralPath $Registry)) {
    throw "Missing registry: $Registry"
}

$Data = Get-Content -LiteralPath $Registry -Raw | ConvertFrom-Json
$Summary = @(& $Show -Registry $Registry -Format Summary)
$Mermaid = @(& $Show -Registry $Registry -Format Mermaid)

if ($Summary -notcontains "Dream: $($Data.dreamMachine)") {
    throw 'Summary does not identify the registry dream machine.'
}
if ($Summary -notcontains "Focus: $($Data.currentFocus)") {
    throw 'Summary does not identify the registry current focus.'
}
foreach ($Wave in @($Data.machines.targetWave | Sort-Object -Unique)) {
    if (-not ($Summary -match "^Wave $Wave`$")) {
        throw "Summary does not contain wave $Wave."
    }
}
if ($Mermaid[0] -ne 'flowchart TD') {
    throw 'Mermaid view does not start with a flowchart declaration.'
}
if (-not ($Mermaid -match $Data.dreamMachine.Replace('.', '_').Replace('-', '_'))) {
    throw 'Mermaid view does not contain the dream machine.'
}

Write-Output "PASS machines=$($Data.machines.Count) waves=$(@($Data.machines.targetWave | Sort-Object -Unique).Count)"
