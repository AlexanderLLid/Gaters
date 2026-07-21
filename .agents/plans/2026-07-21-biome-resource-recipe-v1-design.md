# Biome Resource Recipe v1 Design

## Outcome

A bounded area of an accepted Environment Recipe compiles into one deterministic,
versioned recipe that downstream systems can consume without independently rebuilding
biome, content-placement, landmark-opportunity, or travel-friction evidence.

If this existed, we would no longer need every streaming, evaluation, site, or future
resource consumer to reconstruct the same environment queries and content cells.

## Boundary

- The Environment Recipe remains the authoritative physical root.
- The verified Content Cell generator remains the only tree/rock placement generator.
- The new compiler aggregates existing evidence; it does not create another scatter
  algorithm.
- Landmark values are neutral opportunities, not selected sites or assets.
- Travel friction is neutral physical evidence, not a path, movement rule, or encounter
  decision.
- Resource item identities, rewards, assets, sites, settlements, raids, and characters
  remain outside this contract.
- Optional scarcity remains valid under `WORLD-1`; no content kind is universally forced.

## Contract

`FGatersBiomeResourceCompiler::Compile(Environment, Bounds, CellSize, Semantics)` returns
an `FGatersBiomeResourceCompileResult` containing either a valid recipe or causal issues.

The recipe records:

- contract/compiler versions and Environment Recipe provenance;
- seed, bounded cell range, cell size, and deterministic canonical identity;
- one stable cell record per requested coordinate;
- each cell's biome key and existing `FGatersContentCellRecipe`;
- effective landmark opportunity and travel friction from the accepted root;
- aggregate placement and rejection budgets without asset references.

Cell identity is derived from seed and coordinates. Compilation order cannot affect the
recipe. Empty cells remain present with their causal rejection evidence.

## Data flow

`Environment Recipe -> Biome Resource Compiler -> Biome Resource Recipe`

For each bounded cell, the compiler:

1. queries the accepted biome and opportunity evidence at the cell center;
2. delegates placements to `FGatersContentCellRecipe::Generate` unchanged;
3. records neutral landmark and travel-friction values;
4. aggregates coverage without modifying or rescoring the source evidence.

Runtime streaming, evaluation, future site discovery, and later asset materialization
consume the recipe through separate adapters.

## Invalid input and diagnostics

Compilation fails causally for:

- invalid Environment Recipe provenance;
- empty or inverted bounds;
- non-positive or non-finite cell size;
- excessive requested cell count beyond a declared compile budget;
- duplicate or missing cell identities;
- non-finite or out-of-range opportunity evidence;
- source Content Cell provenance that does not match the accepted environment.

Issues record a stable rule ID and implicated cell/source identity.

## Independent evidence

Automation uses held-out seeds `11`, `29`, `47`, `83`, and `131` and checks:

- deterministic canonical identity and order independence;
- exact parity with independent Environment Recipe queries;
- exact reuse of existing Content Cell outputs;
- declared empty regions remain empty with causal evidence;
- bounded placement totals and valid cell coverage accounting;
- distinct biome/opportunity compositions across the held-out set;
- mutations to provenance, bounds, identities, opportunities, and coverage fail with the
  responsible rule ID.

The existing Content Distribution Evaluator remains the independent density/kind-mix
judge. The compiler never evaluates itself.

## Promotion gate

`world.biome-resource-generator` may become active only when the bounded recipe and
counterexamples pass. It becomes verified only after held-out roots preserve exact
source parity, optional scarcity, deterministic identity, causal diagnostics, and
budgets through the complete Worldgen regression suite.

No visual-quality, asset-generation, gameplay-resource, site, or travel-path capability
is promoted by this work.

## Rejected

- Extending Content Cell v7 with landmark and travel responsibilities: couples local
  placement to site discovery and navigation evidence.
- Declaring the current components sufficient: leaves no authoritative bounded output
  contract for downstream consumers.
- Creating a second placement algorithm: duplicates the verified Content Cell generator.

Requirements checked: `WORLD-1`; exceptions: none.
