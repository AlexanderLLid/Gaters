# World generation capability ladder

Research/options report, **not canon**. Filed 2026-07-16. This asks what machinery would
make it feasible for one developer and coding agents to repeatedly generate, test, and
improve a bounded world containing terrain, a village, a raidable base, defenders, loot,
and visual content.

It preserves the current worldgen call: terrain is immutable after generation and content
fits by site rejection or foundations. Deterministic generation-time terrain fitting is
listed only as a conflicting experiment; adopting it requires an explicit design change.

## Call

Build toward an **autonomous world researcher**, not a bigger terrain generator.

Its load-bearing seam is a deterministic **World Recipe**: a plain data graph describing
the intended place before Unreal actors, meshes, PCG instances, or AI exist. Generators
write recipes; Unreal adapters materialize them; validators and simulations evaluate them;
human edits become explicit recipe patches.

In .NET terms, the recipe is the pure domain model. Terrain, PCG, actors, navigation, and
screenshots are adapters. This separation is what lets the same generated world be tested
headlessly, rendered with placeholders, re-rendered with finished assets, and reconstructed
from seed plus diffs without changing its gameplay identity.

> If this existed, we would no longer need to manually generate, inspect, diagnose, and
> retune every seed, or hand-integrate each new asset type into every generator.

Do **not** begin with an LLM editing generator source overnight. First make generation a
parameterized experiment whose outputs and failures are reproducible. Code mutation is a
later optional layer behind the same tests and human review.

## Observable outcome

Given a seed, generator version, and compact world brief, the system produces within a
budget:

- a bounded Gate-proximate terrain family with traversable routes and semantic ground;
- a connected settlement whose doors, paths, activities, and functions agree;
- a raidable base with approaches, entrances or breach surfaces, defenders, loot, and an
  extraction route;
- stable identities for persistent or destructible objects;
- replaceable visual content that conforms to the ground without changing gameplay;
- an evaluation record explaining hard failures and locating the result in a diversity
  space;
- exact reconstruction from recipe plus recorded world diffs.

Success is not "looks procedural." A seed succeeds when it passes structural and traversal
constraints, produces non-degenerate raid outcomes, occupies a useful diversity niche, and
survives a human visual/readability check.

## Where the work actually is

| Tax | Why it dominates | Machine that deletes it |
|---|---|---|
| Every generator invents its own scene representation | Assets, persistence, evaluation, and editing couple to spawned actors | World Recipe + compiler |
| Assets need hand-fitting and bespoke logic | A mesh does not declare footprint, contacts, doors, navigation, or persistence behavior | Asset contract + intake linter |
| Local procedural rules make globally invalid places | Plausible props do not guarantee connected roads, raid approaches, or reachable loot | Site/route graph planner |
| "Good" becomes one opaque score | Optimization converges on one exploitable, boring base style | Layered evaluator + quality-diversity archive |
| Failures require watching every run | A screenshot cannot explain disconnected navigation or a broken semantic contract | Machine-readable evaluation record |
| Visual replacement destabilizes gameplay | Swapping placeholders for art changes bounds, collision, or placement | Contract-preserving materializers |

## Mechanism choices

### World and settlement structure

| Candidate | Input -> output | Guarantee / work deleted | Limitation | Call |
|---|---|---|---|---|
| **Graph-first hierarchical grammar** | terrain semantics + brief -> sites, route graph, parcels, structures, props | Global connectivity and mission relationships are explicit and testable | Needs a small grammar and compiler | **Build; recommended** |
| Tile or Wave Function Collapse first | adjacency tiles -> local arrangement | Deletes local seam and adjacency work | Does not guarantee a useful global road, raid, or extraction topology | Use later inside parcels/interiors only |
| Simulation-grown settlement | needs/history agents -> evolved settlement | Can create organic-looking history | Slow, difficult to steer, and expensive to falsify | Defer until graph-first results demonstrably look artificial |

### Base generation

| Candidate | Guarantee / work deleted | Limitation | Call |
|---|---|---|---|
| Existing authored archetype mutation | Immediately supplies valid pieces and persistence IDs | Limited topology; variation can be cosmetic | **Borrow as baseline** |
| Defensive topology grammar | Generates approaches, zones, entrances, breach surfaces, loot, and defense posts as a graph | Needs spatial and raid evaluators | **Build next** |
| Adversarial co-evolution of builders and raiders | Searches for strategies humans may not author | Exploits evaluator gaps and is costly | Defer until multiple credible agents and metrics exist |

### Terrain

| Candidate | Guarantee / work deleted | Limitation | Call |
|---|---|---|---|
| Explicit families plus deterministic modifiers | Controllable macro variety and exact replay | Requires authored family envelopes | **Keep current approach** |
| Erosion or terrain simulation | More natural drainage and landform detail | Runtime and tuning cost; does not create gameplay topology | Add only if visual tests identify this exact gap |
| Neural terrain generation | Broad learned shapes | Weak guarantees, provenance and control burden | Reject for the bounded prototype |

### Ground contact

These are **OR** policies chosen per asset contract, not three systems that must all run:

1. Reject the site and resample.
2. Add foundation skirts, supports, or a deliberately conforming base. **Current call.**
3. Deterministically fit terrain during generation. The Terrain Lab can falsify this, but
   adopting it contradicts the recorded immutable-ground call.

### Content source

Existing EBS/Fab/Megaplant content, offline Blender/PVE generation, and future AI-generated
meshes are interchangeable **OR** sources. None is allowed to become architecture. Every
source must satisfy the same asset contract before a generator can consume it.

## Capability graph

`D0` is an AND machine: every major branch is required. Within branches, OR alternatives
are called out explicitly and sequences are numbered in their contracts.

| Node | Unlocks | AND/OR/SEQUENCE | Borrow/Adapt/Build | Contract | Work deleted | Evidence |
|---|---|---|---|---|---|---|
| **D0 Autonomous World Researcher** | Repeated generate-evaluate-adjust loops | AND: C1, C2, E6, A1, H0 | Build from children | A brief yields reproducible diverse playable worlds and diagnostics | Manual per-seed generation and retuning | All child evidence gates pass; held-out human review improves across generator versions |
| **D1 Playable generated world** | D0 outcome | AND: T0, S0, S1, S2, R0, G0, P0 | Adapt + Build | One materialized world satisfies recipe and budget | Hand-authoring each test world | Recipe checksum, structural pass, playable replay |
| **C0 World Recipe** | Every compiler, evaluator, diff, editor | SEQUENCE: generator -> recipe -> consumers | **Build** | Versioned serializable graph of stable nodes, tags, bounds, ports, transforms, relationships, and provenance | Actor-specific generator coupling | Same recipe round-trips byte-for-byte or to a canonical checksum |
| **C1 World Compiler** | D1, placeholder/art parity | AND: C0, C2, F0; SEQUENCE into R0/G0 | Build thin adapters | Materializes a recipe without changing its semantic checksum | Bespoke spawning per generator | Placeholder and art builds retain node IDs, relationships, and gameplay metrics |
| **C2 Contracted Content Catalog** | C1, S1, S2, R0 | AND: C3 and one V0 source | Adapt | Queries assets by capability rather than hardcoded path/class | Hand-picking assets in generator code | Catalog returns only compatible candidates for a requested contract |
| **C3 Asset Intake Linter** | Safe asset/catalog growth | SEQUENCE: source -> inspect -> accept/reject -> catalog | **Build** | Checks pivot, bounds, scale, collision, contact, sockets/ports, clearance, tags, render class, and persistence class | Repeated visual integration debugging | Seed set of deliberately good/bad assets classified with actionable reasons |
| **V0 Content Source** | C2 | OR: existing kits / Blender-PVE / future AI | Borrow or Adapt | Supplies meshes/assemblies plus provenance; makes no gameplay claim | Modeling every item by hand | Imported artifact exists and can be inspected by C3 |
| **T0 Terrain Family Generator** | T1, S0, D1 | SEQUENCE: seed -> height/water field | **Borrow current Gaters** | Deterministic bounded terrain with Gate clearing and named family | Rebuilding macro terrain | Existing family/determinism/base-validity automation |
| **T1 Terrain Semantic Field** | S0, E1, E2 | SEQUENCE: T0 -> sampled categories/descriptors | **Adapt current Gaters** | Queries height, water, slope, buildability, reservation; adds visibility/cover/choke only when consumed | Each generator re-deriving terrain meaning | Query results deterministic; evaluator tests consume every added tag |
| **S0 Site and Route Graph Planner** | S1, S2, traversal guarantees | AND: T1, brief; SEQUENCE: sites -> routes -> parcels | **Build** | Places Gate, settlement, base, routes, parcels, exclusions with global connectivity | Hoping local placement produces a coherent place | All required nodes connected; clearance, water, grade, distance, and budget constraints pass |
| **S1 Settlement Synthesizer** | Villages and civilian space | AND: S0, C2, F0, G2 | Build; borrow PCG grammar for geometry | Emits roads, parcels, building roles, doors, activity slots, and decoration anchors | Hand-layout of every village | Every occupied building has road/nav access and its declared activity ports |
| **S2 Defensive Base Synthesizer** | Varied raid content | AND: S0, C2, F0, E2 | Adapt current stamps, then Build topology grammar | Emits approach zones, defense layers, entrances/breaches, loot, spawns, and cover | Hand-authored raid layouts | Hard raid constraints pass and descriptor distribution spans multiple topology cells |
| **F0 Physical Fit Policy** | C1, S1, S2 | OR per asset: reject / foundation / conflicting terrain fit | Adapt current fit | Asset meets ground within declared contact and clearance tolerances | Hand-positioning and floating/clipping repair | Automated footprint samples and collision check; rejected sites name failed tolerance |
| **R0 Visual Materializer** | Finished-looking D1 | AND: C0, C2; SEQUENCE after structure | Adapt Unreal PCG/ISM | Converts decoration anchors and style tags to instanced visual content without semantic changes | Per-seed prop placement | Recipe/gameplay checksum unchanged when placeholder catalog is swapped for art catalog |
| **G0 Gameplay Materializer** | Interactions, destruction, simulation | AND: C0, C2, G1, G2 | Adapt Unreal actors/components | Spawns actors only where runtime identity or behavior is required | Making every prop a heavyweight actor | Stable actor-to-recipe ID mapping; object and frame budgets pass |
| **G1 Navigation and Query Substrate** | E1, E2, E3 | AND: collision contract + Unreal Nav/EQS | Borrow + Adapt | Rebuilds navigation once after materialization; exposes path, reachability, visibility, and candidate-position queries | Custom pathfinding/spatial search | Known paths pass/fail correctly; rebuild time recorded |
| **G2 Activity and Behavior Substrate** | Settlement life, defenders, E3 | AND: Smart Objects + StateTree + recipe ports | Borrow + Adapt | Agents find/reserve declared activities and execute bounded roles | Bespoke logic embedded in every building | Agents use required slots and recover from unavailable reservations |
| **P0 Stable Identity and Diff Replay** | Persistence and comparable reruns | AND: C0 stable IDs + current diff path | **Borrow/Adapt current Gaters** | Seed + generator version + recipe + changes reconstructs the same state | Saving complete spawned worlds | Existing replay test extended across settlement/base nodes |
| **E0 Structural Evaluator** | Cheap rejection before Unreal | SEQUENCE first in E6 | **Build** | Pure-data checks IDs, bounds, overlaps, budgets, required graph relations, and contracts | Watching obvious broken seeds | Curated invalid recipes each fail for the intended reason |
| **E1 Traversal Evaluator** | Playable routes | SEQUENCE after materialization/nav | Adapt Nav queries | Gate -> settlement -> base -> loot -> extraction reachability, including doors/links | Manually walking every seed | Golden reachable/unreachable cases and path-cost records |
| **E2 Raid-Space Evaluator** | S2 and E3 inputs | AND: recipe topology + T1 + G1 | **Build** | Measures approaches, choke concentration, sightlines, cover, breach options, spawn safety, and loot exposure | Eyeballing whether a base is raidable | Hand-authored fixtures produce expected relative rankings, not an unverifiable "fun" score |
| **E3 Encounter Evaluator** | Outcome evidence | AND: S2, G0, G1, multiple policies | Adapt current raider, then Build profiles | Runs attacker archetypes against defender profiles; records success, time, damage, route, and cause | One-off playthroughs and single-bot bias | Repeated runs are deterministic under fixed policy seed; known easy/hard fixtures separate |
| **E4 Perceptual/Human Evaluator** | Visual/readability evidence | OR-assisted: fixed gallery + human labels + optional vision model | Adapt current gallery | Records composition, readability, repetition, clipping, and preference labels; model cannot be sole oracle | Opening every seed live | Blind human comparisons and disagreement retained with screenshots |
| **E5 Performance Evaluator** | Shipping budgets | SEQUENCE after each materializer | Adapt Unreal profiling + current timing | Records generation, nav, actor/instance, memory, and frame budgets | Discovering scale failures late | Automated threshold fixtures on target hardware profile |
| **E6 Layered Evaluation Record** | A0, A1, explainable rejection | AND: E0-E5 as applicable | **Build composition** | Stores hard failures, metric vector, descriptors, artifacts, and uncertainty; never collapses to one magic score | Opaque pass/fail and Goodhart-prone tuning | A failed seed identifies layer, rule, recipe nodes, and replay artifact |
| **H0 Batch Harness and Evidence Store** | D0 iteration | Adapt current sweep/gallery/JSONL | Adapt | Runs seed/parameter sets unattended and stores recipe, checksums, logs, metrics, screenshots, replay, and versions | Manual launch/capture/file naming | Interrupted batch resumes without duplicating or losing provenance |
| **A0 Quality-Diversity Archive** | Useful variety rather than one winner | SEQUENCE: E6 -> descriptor cell -> elite | **Build small** | Keeps best valid examples across explicit niches such as terrain family, approach count, base shape, and settlement density | Manually curating variety; single-score convergence | Archive coverage and within-cell quality improve without descriptor collapse |
| **A1 Experiment Orchestrator** | D0 closed loop | AND: H0, A0; SEQUENCE: select -> mutate params -> evaluate -> archive | **Build** | Mutates data parameters first, reproduces every candidate, and never promotes without evidence gates | Hand-running parameter sweeps | Re-run of recorded experiment yields identical candidate recipes and metrics |
| **M0 Override Round-trip** | Human/agent collaboration | SEQUENCE: edit -> explicit recipe patch -> regenerate | Build; Adapt UE PCG editor overrides | Accepted manual move/delete/replace survives regeneration as visible data | Choosing between procedural regeneration and manual art direction | Patch reapplies cleanly; conflicts are reported, not silently discarded |

## Why this graph stops here

Every unresolved leaf either already produces evidence or has a small falsifying prototype:

- Gaters already supplies deterministic terrain, semantic sampling, stable scatter IDs,
  seed-plus-diff replay, EBS base stamping, one raid probe, seed sweeps, JSONL, and galleries.
- Unreal already supplies PCG data/graphs, instancing, navigation, EQS, Smart Objects, and
  StateTree. These are adapters, not new engine work.
- The missing leaves are contracts and global place logic: recipe schema, asset linter,
  route graph, settlement grammar, defensive topology, spatial metrics, multiple encounter
  policies, and experiment archive.
- No leaf asks an LLM or vision model to decide whether a level is "fun." Human judgment
  remains evidence; the machine deletes repetition and uncertainty around it.

## Falsifying experiments

These test guarantees, not polish. A failed experiment restarts the graph at the failed
contract rather than adding another generator layer.

1. **Recipe-preserving rematerialization — first experiment.** One current seed emits a
   canonical recipe. Materialize it once with primitives and once with EBS assets. Stable
   IDs, relationships, reachability, and raid inputs must match. If they do not, the recipe
   is missing semantic data.
2. **Asset intake.** Register five deliberately varied asset types plus broken pivot,
   collision, scale, and socket fixtures. The linter must reject each broken fixture for
   the intended reason and accept compatible substitutes without generator code changes.
3. **Route-first settlement.** Across a seed sweep, generate roads, parcels, placeholder
   buildings, doors, and activity ports. Every required door must connect to the route/nav
   graph; no parcel may overlap water, exclusions, or another parcel.
4. **Defensive topology.** Generate placeholder bases across many seeds. Every base must
   have a valid arrival, reachable loot, safe initial spawn, a bounded object count, and
   declared approach/breach topology. Descriptors must occupy more than one topology niche.
5. **Multiple encounter policies.** Add runner, breacher, and ranged attacker policies and
   two defender policies. Fixed-policy reruns must reproduce; obvious open/fortified fixture
   bases must separate. Seeds that only beat one naïve agent are not promoted as strong.
6. **Dressing parity.** Swap primitive trees/buildings for contracted content. Structural,
   traversal, and raid metrics must remain within declared tolerances while visual and
   performance records change.
7. **Quality-diversity archive.** Sweep a broad candidate set into cells defined by terrain,
   approach count, base shape, and settlement density. Compare the archive gallery with the
   top results from a single scalar score; the archive must retain visibly and tactically
   distinct valid worlds.
8. **Override round-trip.** Move one artifact and delete another in-editor. Regeneration
   must preserve both as a readable patch or report a conflict.
9. **Ground-fit branch, only if needed.** Compare current reject/foundation behavior with
   deterministic generation-time fitting on identical recipes. Evidence must show a gap the
   current policy cannot carry before revisiting immutable ground.

## Evidence-gated build waves

### Wave 0 — Borrow and freeze contracts

- Treat current terrain, stable IDs/diffs, base stamping, raider, sweep, and gallery as
  seed machines. Add no terrain algorithm.
- Define the minimum `WorldRecipe`, `AssetSpec`, and `EvaluationRecord` contracts on paper.
- Gate: experiment 1 can name every datum it needs without referring to a particular actor.

### Wave 1 — Adapt the existing slice

- Make one current generated seed export a recipe, then compile that same recipe back into
  the current terrain/base/scatter result.
- Wrap a small EBS/primitive catalog behind asset contracts.
- Extend existing sweep output with recipe checksum and structured evaluation provenance.
- Gate: recipe-preserving rematerialization and asset-intake experiments pass.

### Wave 2 — Build the first missing place machine

- Add the site/route graph and pure structural evaluator.
- Generate a route-first placeholder settlement; no visual dressing yet.
- Gate: settlement experiment passes across a sweep and failures are diagnosed from data.

### Wave 3 — Make raid variety measurable

- Replace base cosmetic mutation with a defensive topology recipe while retaining EBS as a
  materializer.
- Add raid-space metrics, multiple attacker/defender policies, and the quality-diversity
  archive.
- Gate: topology, encounter, and archive experiments pass on held-out seeds.

### Wave 4 — Make it look inhabited

- Use PCG/instancing for biome and prop dressing.
- Use Smart Objects and StateTree for bounded village and defender activity.
- Gate: dressing parity holds; perceptual/human review improves without gameplay regressions.

### Wave 5 — Content foundry and mixed initiative

- Generate or author contracted Blender/PVE asset families offline; all pass intake.
- Add explicit editor override patches.
- Let an LLM propose parameter experiments and, later, reviewed code patches only after the
  reproducible parameter loop has plateaued.
- Gate: new content sources and edits require no generator-specific integration.

## Seed nodes available now

- **Gaters:** deterministic explicit terrain families; semantic ground grid; valid-site
  search; stable IDs; seed-plus-diff reconstruction; varied EBS stamps; one generator-
  agnostic raider; automation sweeps; fixed-camera galleries; isolated contact-fit lab.
- **Unreal 5.8:** PCG and spatial/attribute data, Shape Grammar, instancing, Navigation,
  EQS, Smart Objects, StateTree, Geometry Script, and experimental PVE/Biome tooling.
- **Content:** EBS modular pieces, existing Fab/Megaplant assets, Blender, and primitives.

Experimental/Beta Unreal features should not own persistent gameplay truth. PVE is best
treated as an offline asset source; experimental PCG grammar or Biome graphs should be
wrapped or copied if they become production dependencies. Mass/ZoneGraph is unnecessary
until measured inhabitant counts exceed ordinary Character + Nav + StateTree behavior.

## First build wave and the machine it unlocks

The first implementation should be only **recipe-preserving rematerialization**. It is the
smallest machine that can prove or kill the architecture, and every later branch consumes
it.

Passing it unlocks a **replaceable world compiler**: the current generator can emit one
semantic world, evaluators can inspect it without rendering, and different Unreal content
stacks can render it without altering the world. That, in turn, is the seed from which the
full autonomous world researcher can actually be built.

## Sources

- [Epic: Procedural Content Generation Framework](https://dev.epicgames.com/documentation/en-us/unreal-engine/procedural-content-generation-framework-in-unreal-engine)
- [Epic: PCG data types](https://dev.epicgames.com/documentation/en-us/unreal-engine/procedural-content-generation-framework-data-types-reference-in-unreal-engine)
- [Epic: Shape Grammar with PCG](https://dev.epicgames.com/documentation/unreal-engine/using-shape-grammar-with-pcg-in-unreal-engine?lang=en-US)
- [Epic: Unreal Engine 5.8 release notes](https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-release-notes)
- [Epic: Navigation System](https://dev.epicgames.com/documentation/unreal-engine/navigation-system-in-unreal-engine)
- [Epic: Environment Query System](https://dev.epicgames.com/documentation/en-us/unreal-engine/environment-query-system-in-unreal-engine)
- [Epic: Smart Objects](https://dev.epicgames.com/documentation/unreal-engine/smart-objects-in-unreal-engine---overview?lang=en-US)
- [Epic: StateTree overview](https://dev.epicgames.com/documentation/unreal-engine/overview-of-state-tree-in-unreal-engine?lang=en-US)
- [Mouret and Clune: MAP-Elites / illuminating search spaces](https://arxiv.org/abs/1504.04909)
- [The Procedural Content Generation Benchmark](https://arxiv.org/abs/2503.21474)

