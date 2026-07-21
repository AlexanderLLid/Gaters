# Climate Root Integration Plan

**Goal:** Make the verified Climate Field available from the authoritative environment
recipe without changing accepted terrain, biome, water, sites, or rendered output.

**Architecture:** The root stores the compiled Environment Brief and Climate recipe.
`QueryClimate` delegates to the pure field using the root's accepted Terrain. The chunk
passes the selected Landform recipe when one wins; otherwise it passes the neutral
candidate-zero profile recipe without applying it to terrain.

- [x] Add failing root tests for versioned climate provenance, deterministic queries,
  direct-field parity, exact accepted-height identity, and causal invalid recipes.
- [x] Add a failing selected-landform composition test.
- [x] Extend `FGatersEnvironmentRecipe` and its compiler with the minimum climate seam.
- [x] Pass accepted Environment Brief/Landform provenance from `AGatersChunk`.
- [x] Run Environment Recipe, Climate Field, selector, and landform tests.
- [x] Run the complete Gaters suite and repository validators.
- [x] Record integration evidence without claiming new climate visuals.

Requirements checked: `WORLD-1`; exceptions: none.
