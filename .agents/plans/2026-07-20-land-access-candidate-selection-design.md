# Land-access candidate selection design

## Decision

- Build one deterministic candidate selector between landform generation and independent
  traversal evaluation.
- Add two global environment-brief targets: walkable-land fraction and connected-land
  fraction. They remain separate because terrain amount and connectivity fail
  independently.
- Require Arrival escape independently of both targets.
- Treat `minimal`, `sparse`, and `broad` only as possible caller presets that compile to
  ranges; they are not generator branches or recipe identity.
- Optional village, landmark, base, forest, and resource availability does not decide
  whether a terrain candidate satisfies its brief.

Requirements checked: `WORLD-1`, generated-content boundary; exceptions: none.

## Observable outcome

Given the same environment brief, seed, world size, protected Arrival geometry, and
selector settings, the machine returns the same selected landform recipe and the same
metric evidence. A changed seed or allowed brief range may select another result. If no
candidate satisfies the declared land-access bands and Arrival escape, the machine emits
causal evidence and leaves the current terrain champion unchanged.

## Capability graph

| Node | Edge | Source | Contract | Work deleted |
|---|---|---|---|---|
| Environment Brief Compiler | seed node | Adapt verified | Compile bounded land-access targets beside physical targets | Hardcoded seed tables and named world modes |
| Landform Process Field | SEQUENCE | Adapt active | Produce deterministic variants without changing world-seed provenance | One-off terrain formulas per failed seed |
| Traversability Evaluator | SEQUENCE | Adapt verified | Measure total walkable land, connected walkable land, and Arrival escape | Manual traversal inspection |
| Environment Candidate Selector | SEQUENCE | Build missing | Choose only an independently satisfying candidate; preserve failure evidence | Hand-patching seeds and optional-site corridor stamps |
| Environment Recipe Compiler | SEQUENCE | Adapt active | Preserve selected target, recipe, and evaluator provenance | Downstream reconstruction and ambiguous intent |
| Generate a World Environment | AND consumer | Build active | Produce a reproducible environment satisfying its declared constraints | Hand-assembled worlds |

## Alternatives

- **Chosen — generate, evaluate, select:** strongest separation and causal evidence;
  bounded candidate cost is the exposed limitation.
- **Rejected — direct process damping:** cheaper, but the generator judges and repairs
  itself with the same assumptions that caused the failure.
- **Rejected — protected route network:** guarantees only stamped routes and incorrectly
  couples terrain validity to optional sites.

## Contracts

### Brief

- `WalkableLand`: fraction of sampled world cells classified flat or slope.
- `ConnectedLand`: fraction of walkable cells connected to Arrival.
- Both are bounded seed-sampled targets in the existing environment brief.
- Regional land-access targets remain unsupported in this wave.

### Candidate

- Candidate index varies only process placement/orientation noise.
- Candidate zero preserves the current process field exactly.
- World seed, physical targets, protected regions, and world size remain unchanged.

### Selection

- Every candidate is evaluated by `FGatersTraversabilityEvaluator` over two semantic
  fields: a fixed-resolution full-world field for access fractions and a local
  high-resolution Arrival field for escape.
- A candidate satisfies only when both target errors are within their declared selector
  tolerances and Arrival escapes the protected pad.
- Deterministic score and candidate index break ties.
- No satisfying candidate means no promotion; best evidence is still returned.

## Runtime boundary

- The pure selector owns no Actors, assets, sites, or rendering.
- `AGatersChunk` is only an adapter: it supplies tunables, attaches a satisfying recipe,
  and reports provenance.
- `FGatersSiteRoutePlanner` runs afterward and may validly produce no optional site plan.
- Unreal materialization remains derived from the accepted environment recipe.

## Verification

- **Independent verifier:** the existing semantic field plus Traversability Evaluator;
  selection never reads `FGatersSiteRoutePlan`.
- **Challenge set:** repeated seeds, changed seeds, synthetic open/split/trapped fields,
  all current terrain families, high-relief/volcanic/glacial briefs, impossible access
  targets, protected Arrival, intentionally scarce worlds, and worlds whose local
  Arrival neighborhood differs from their global access profile.
- **Failure artifact:** seed, brief/compiler versions, target bands, candidate index,
  process recipe, walkable/connected fractions, Arrival escape, score, and rejection.
- **Promotion gate:** held-out briefs satisfy their declared access targets without
  optional-content guarantees, retain determinism and budget, and beat the current
  champion on environment coverage without regressions.
