$ErrorActionPreference = 'Stop'
$Root = Split-Path $PSScriptRoot -Parent
$Errors = [System.Collections.Generic.List[string]]::new()

$Claude = (Get-Content -Raw (Join-Path $Root 'CLAUDE.md')).Trim()
if ($Claude -notmatch '(?m)^@AGENTS\.md$') {
    $Errors.Add('CLAUDE.md must import AGENTS.md.')
}

$Agents = Get-Content -Raw (Join-Path $Root 'AGENTS.md')
if ($Agents -notmatch [regex]::Escape('`.agents/skills/` is authoritative')) {
    $Errors.Add('AGENTS.md must name .agents/skills/ as the authoritative skill source.')
}

$AgentRoot = Join-Path $Root '.agents/skills'
$ClaudeRoot = Join-Path $Root '.claude/skills'
$Names = @(
    (Get-ChildItem $AgentRoot -Directory).Name
    (Get-ChildItem $ClaudeRoot -Directory).Name
) | Sort-Object -Unique

foreach ($Name in $Names) {
    $AgentSkill = Join-Path $AgentRoot "$Name/SKILL.md"
    $ClaudeSkill = Join-Path $ClaudeRoot "$Name/SKILL.md"
    if (-not (Test-Path $AgentSkill) -or -not (Test-Path $ClaudeSkill)) {
        $Errors.Add("Skill mirror missing: $Name")
        continue
    }
    if ((Get-FileHash $AgentSkill).Hash -ne (Get-FileHash $ClaudeSkill).Hash) {
        $Errors.Add("Skill mirror differs: $Name")
    }
}

$ExpectedWorkstreams = 'art.md', 'bases.md', 'builder.md', 'combat.md', 'lore.md', 'scale.md'
foreach ($Name in $ExpectedWorkstreams) {
    $Path = Join-Path $Root ".agents/workstreams/$Name"
    if (-not (Test-Path $Path)) {
        $Errors.Add("Workstream missing: $Name")
        continue
    }
    $Text = Get-Content -Raw $Path
    foreach ($Heading in '## Current objective', '## Owned outputs', '## Exchanges') {
        if ($Text -notmatch [regex]::Escape($Heading)) {
            $Errors.Add("$Name missing $Heading")
        }
    }
}

$Router = Get-Content -Raw (Join-Path $Root '.agents/workstreams/README.md')
foreach ($Required in '## Ambiguity gate', 'HUMAN DECISION NEEDED:') {
    if ($Router -notmatch [regex]::Escape($Required)) {
        $Errors.Add("Workstream router missing $Required")
    }
}

$ExchangeTemplate = Join-Path $Root '.agents/exchanges/TEMPLATE.md'
if (-not (Test-Path $ExchangeTemplate)) {
    $Errors.Add('Exchange template missing.')
} else {
    $Text = Get-Content -Raw $ExchangeTemplate
    foreach ($Field in 'Status:', 'From:', 'To:', 'Type:', '## Request', '## Response', '## Resolution') {
        if ($Text -notmatch [regex]::Escape($Field)) {
            $Errors.Add("Exchange template missing $Field")
        }
    }
}

if ($Errors.Count) {
    $Errors | ForEach-Object { Write-Error $_ }
    exit 1
}

Write-Host "PASS shared-agent-docs skills=$($Names.Count) workstreams=$($ExpectedWorkstreams.Count)"
