param(
    [string]$CatalogPath = (Join-Path $PSScriptRoot 'rift-raid-scenarios-v1.json')
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $CatalogPath)) {
    throw "Missing scenario catalog: $CatalogPath"
}

$catalog = Get-Content -Raw -LiteralPath $CatalogPath | ConvertFrom-Json

if ($catalog.scenarioCatalogVersion -ne 1) {
    throw 'scenarioCatalogVersion must be 1'
}

if ($catalog.requires.builtSiteRecipe -ne 'RAID-1') {
    throw 'catalog must depend on RAID-1 Built Site Recipe contract'
}

$accessIds = @{}
foreach ($access in $catalog.riftAccessStates) {
    if ($accessIds.ContainsKey($access.id)) {
        throw "Duplicate rift access state id: $($access.id)"
    }
    $accessIds[$access.id] = $true
}

$scenarioIds = @{}
$hasWild = $false
$hasClaimedBase = $false
$hasSettlement = $false
foreach ($scenario in $catalog.scenarios) {
    if ($scenarioIds.ContainsKey($scenario.id)) {
        throw "Duplicate scenario id: $($scenario.id)"
    }
    $scenarioIds[$scenario.id] = $true

    if (-not $accessIds.ContainsKey($scenario.accessStateId)) {
        throw "Scenario $($scenario.id) references missing access state $($scenario.accessStateId)"
    }
    $access = $catalog.riftAccessStates | Where-Object { $_.id -eq $scenario.accessStateId }
    foreach ($siteKind in $scenario.siteKinds) {
        if ($access.siteKinds -notcontains $siteKind) {
            throw "Scenario $($scenario.id) site kind $siteKind is unsupported by $($access.id)"
        }
    }
    if (-not $scenario.arrival.forbidden -or (($scenario.arrival.forbidden -notcontains 'vault') -and ($scenario.arrival.forbidden -notcontains 'loot'))) {
        throw "Scenario $($scenario.id) must forbid direct vault or loot arrival"
    }
    if (-not $scenario.evaluationFocus -or $scenario.evaluationFocus.Count -lt 3) {
        throw "Scenario $($scenario.id) needs at least three evaluation focus items"
    }
    if (-not $scenario.mustFlag -or $scenario.mustFlag.Count -lt 3) {
        throw "Scenario $($scenario.id) needs at least three exploit/failure flags"
    }
    if (-not $scenario.requiredRecipeFacts -or $scenario.requiredRecipeFacts.Count -lt 4) {
        throw "Scenario $($scenario.id) must name required Built Site Recipe facts"
    }

    if ($scenario.siteKinds -contains 'claimed-base') {
        $hasClaimedBase = $true
        if ($scenario.accessStateId -ne 'traced-claimed-base-v1') {
            throw "Claimed-base scenario $($scenario.id) must use traced-claimed-base-v1"
        }
    }
    if (($scenario.siteKinds -contains 'wild-dungeon') -or ($scenario.siteKinds -contains 'rift-wounded-site') -or ($scenario.siteKinds -contains 'monster-held-site')) {
        $hasWild = $true
    }
    if ($scenario.siteKinds -contains 'settlement') {
        $hasSettlement = $true
        if ($scenario.objective.type -ne 'loot-extract') {
            throw "Settlement scenario $($scenario.id) must use loot-extract"
        }
        if ($scenario.pressure -notcontains 'carried-extraction') {
            throw "Settlement scenario $($scenario.id) must preserve carried extraction pressure"
        }
        foreach ($requiredFlag in @('world-approach-evidence-unknown', 'site-network-disconnected-objective')) {
            if ($scenario.mustFlag -notcontains $requiredFlag) {
                throw "Settlement scenario $($scenario.id) must flag $requiredFlag"
            }
        }
        if ($scenario.requiredRecipeFacts -notcontains 'composed-world-to-site-approach') {
            throw "Settlement scenario $($scenario.id) must require a composed world-to-site approach"
        }
    }
}

if (-not $hasWild) {
    throw 'At least one wild dungeon scenario is required'
}
if (-not $hasClaimedBase) {
    throw 'At least one claimed player-base scenario is required'
}
if (-not $hasSettlement) {
    throw 'At least one AI or abandoned settlement scenario is required'
}

foreach ($check in $catalog.acceptance) {
    if (-not $check.id -or -not $check.expectation) {
        throw 'Every acceptance check needs id and expectation'
    }
}

Write-Host "Rift raid scenario catalog OK: $($catalog.scenarios.Count) scenarios, $($catalog.riftAccessStates.Count) access states"
