param(
    [string] $Registry = (Join-Path $PSScriptRoot 'machines.json'),
    [ValidateSet('Validate', 'Summary', 'Mermaid')]
    [string] $Format = 'Summary'
)

$ErrorActionPreference = 'Stop'
$Data = Get-Content -LiteralPath $Registry -Raw | ConvertFrom-Json
$Machines = @($Data.machines)
$ById = @{}
$Errors = [System.Collections.Generic.List[string]]::new()

if ($Data.schemaVersion -ne 1) {
    $Errors.Add("Unsupported schemaVersion '$($Data.schemaVersion)'.")
}

foreach ($Machine in $Machines) {
    if ([string]::IsNullOrWhiteSpace($Machine.id)) {
        $Errors.Add('Machine has an empty id.')
        continue
    }
    if ($ById.ContainsKey($Machine.id)) {
        $Errors.Add("Duplicate machine id '$($Machine.id)'.")
        continue
    }
    $ById[$Machine.id] = $Machine
    if ($Machine.source -notin @('Borrow', 'Adapt', 'Build')) {
        $Errors.Add("Machine '$($Machine.id)' has invalid source '$($Machine.source)'.")
    }
    if ($Machine.status -notin @('verified', 'active', 'planned', 'deferred')) {
        $Errors.Add("Machine '$($Machine.id)' has invalid status '$($Machine.status)'.")
    }
    if ($Machine.targetWave -lt 0) {
        $Errors.Add("Machine '$($Machine.id)' has invalid targetWave '$($Machine.targetWave)'.")
    }
    if ($Machine.source -in @('Adapt', 'Build')) {
        foreach ($Field in @('verifier', 'challengeSet', 'failureArtifact', 'promotionGate')) {
            if ([string]::IsNullOrWhiteSpace($Machine.$Field)) {
                $Errors.Add("Machine '$($Machine.id)' is missing $Field.")
            }
        }
    }
}

if (-not $ById.ContainsKey($Data.dreamMachine)) {
    $Errors.Add("Dream machine '$($Data.dreamMachine)' does not exist.")
}
$Edges = @{}
foreach ($Machine in $Machines) {
    foreach ($Group in @($Machine.requires)) {
        if ($Group.type -notin @('AND', 'OR', 'SEQUENCE')) {
            $Errors.Add("Machine '$($Machine.id)' has invalid dependency type '$($Group.type)'.")
        }
        foreach ($Dependency in @($Group.machines)) {
            if (-not $ById.ContainsKey($Dependency)) {
                $Errors.Add("Machine '$($Machine.id)' requires missing machine '$Dependency'.")
                continue
            }
            if ($ById[$Dependency].targetWave -gt $Machine.targetWave) {
                $Errors.Add("Machine '$($Machine.id)' wave $($Machine.targetWave) depends on later wave $($ById[$Dependency].targetWave) machine '$Dependency'.")
            }
            $Edges["$Dependency->$($Machine.id)"] = [pscustomobject]@{
                From = $Dependency
                To = $Machine.id
                Type = $Group.type
            }
        }
    }
}

$InDegree = @{}
$Outgoing = @{}
foreach ($Id in $ById.Keys) {
    $InDegree[$Id] = 0
    $Outgoing[$Id] = [System.Collections.Generic.List[string]]::new()
}
foreach ($Edge in $Edges.Values) {
    $InDegree[$Edge.To]++
    $Outgoing[$Edge.From].Add($Edge.To)
}
$Queue = [System.Collections.Generic.Queue[string]]::new()
foreach ($Id in $ById.Keys) {
    if ($InDegree[$Id] -eq 0) { $Queue.Enqueue($Id) }
}
$Visited = 0
while ($Queue.Count -gt 0) {
    $Id = $Queue.Dequeue()
    $Visited++
    foreach ($Next in $Outgoing[$Id]) {
        $InDegree[$Next]--
        if ($InDegree[$Next] -eq 0) { $Queue.Enqueue($Next) }
    }
}
if ($Visited -ne $ById.Count) {
    $Errors.Add('Dependency graph contains a cycle.')
}

if ($Errors.Count -gt 0) {
    throw ($Errors -join [Environment]::NewLine)
}

if ($Format -eq 'Validate') {
    Write-Output "VALID machines=$($Machines.Count) edges=$($Edges.Count) dream=$($Data.dreamMachine)"
    exit 0
}

if ($Format -eq 'Summary') {
    Write-Output "Dream: $($Data.dreamMachine)"
    foreach ($Wave in @($Machines.targetWave | Sort-Object -Unique)) {
        Write-Output "Wave $Wave"
        foreach ($Machine in @($Machines | Where-Object targetWave -eq $Wave | Sort-Object id)) {
            Write-Output "  [$($Machine.status)] $($Machine.id) - $($Machine.outcome)"
        }
    }
    exit 0
}

Write-Output 'flowchart TD'
foreach ($Machine in @($Machines | Sort-Object id)) {
    $NodeId = $Machine.id -replace '[^A-Za-z0-9_]', '_'
    $Label = "$($Machine.name)<br/>$($Machine.status), wave $($Machine.targetWave)" -replace '"', "'"
    Write-Output "    $NodeId[`"$Label`"]"
}
foreach ($Edge in @($Edges.Values | Sort-Object From, To)) {
    $From = $Edge.From -replace '[^A-Za-z0-9_]', '_'
    $To = $Edge.To -replace '[^A-Za-z0-9_]', '_'
    Write-Output "    $From -->|$($Edge.Type)| $To"
}
