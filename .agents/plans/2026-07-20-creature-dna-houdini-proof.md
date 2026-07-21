# CreatureDNA Houdini Proof Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use
> `superpowers:subagent-driven-development` (recommended) or
> `superpowers:executing-plans` to execute this plan task by task.

**Goal:** Prove that one explicit creature recipe can compile into a mathematically
validated anatomy guide inside Houdini, with limb counts and body modules changed through
data instead of node or code edits.

**Architecture:** A small standard-library Python compiler turns versioned CreatureDNA
JSON into a tool-neutral anatomy graph. A thin Houdini adapter materializes that graph as
named points and bone curves. A separate verifier reads the exported raw graph and checks
structure, dimensions, connectivity, symmetry intent, and reproducibility without
trusting Houdini scene labels or generator pass claims. Houdini files are derived output;
the JSON recipe, compiler, adapter version, and verification policy are authoritative.

**Tech Stack:** Houdini Apprentice for noncommercial feasibility only, Houdini Python
(`hou`), Python standard library, JSON, PowerShell, and `unittest`.

## Global Constraints

- This is a Character Generation & Animation experiment. Art Direction defines visual
  requirements but does not implement in its owned face-study paths.
- Proposed implementation root:
  `C:/repos/Gaters/research/creature-dna-proof/`. Do not edit
  `research/embodied-species-lab/`, `SourceAssets/Blender/`, or Unreal assets.
- Do not launch Unreal or add Unreal integration. That is a later adapter and all Unreal
  runs remain owned by Unreal Runner.
- Do not build an MCP, prompt parser, production topology system, rig, skinning, motion,
  materials, or finished creature in this proof.
- Do not add external Python packages. Use Houdini and the Python standard library.
- Record the Houdini edition and build in every run. Apprentice output is evaluation-only
  and cannot become an authoritative production asset. If the proof is promoted, rerun
  and revalidate the source-driven build under an appropriate commercial license.
- No branch, commit, or push unless the human asks.
- Before execution, Primary Builder must change `CHAR-2` from Blender-specific offline
  ownership to tool-neutral offline-generator ownership. Houdini is the first challenger;
  it is not declared the permanent winner by this plan.

## What This Proof Decides

The proof answers one question:

> Can an agent change a creature's body plan through explicit data and reliably obtain
> the intended named anatomy graph in a fresh Houdini scene?

It passes only when all of these are true:

- A baseline recipe compiles as one torso, one tapered head, one tail, four legs, and two
  wings.
- A held-out recipe changes appendage counts and placement without changing Python code.
- The independent verifier proves every joint and bone is finite, nonzero, connected to
  the declared parent module, uniquely named, and within declared size bounds.
- Bilateral modules satisfy the recipe's mirror tolerance; explicitly asymmetric modules
  are not forced to mirror.
- A malformed-parent fixture and a zero-length-bone fixture fail for the intended rule.
- Two fresh runs using the same recipe, seed, compiler version, and Houdini build produce
  identical canonical graph JSON and checksum.
- The resulting `.hipnc` opens with the guide graph visible and named. A screenshot is
  diagnostic evidence, not the pass oracle.

It does **not** prove a good surface, topology, deformation, animation, or art style.

## Stable Layer Boundary

Keep body structure separate from presentation so this does not lock the game to one
style:

```text
Natural-language brief             later, replaceable AI adapter
        |
        v
CreatureDNA                        body plan, modules, measurements, intent
        |
        v
AnatomyGraph                       joints, bones, sockets, envelopes, contacts
        |
        +--> Houdini guide scene   first derived adapter in this proof
        +--> Blender/other adapter possible later
        |
        v
StyleDNA                           later: shape language, exaggeration, materials
```

CreatureDNA never contains a hard-coded `Dragon` class. A dragon-like recipe composes
ordinary modules such as `torso`, `leg`, `wing`, `tail`, and `tapered_head`.

## Planned Repository Shape

```text
research/creature-dna-proof/
  README.md
  recipes/
    winged-reptile.json
    five-legged-challenge.json
  fixtures/invalid/
    missing-parent.json
    zero-length-bone.json
  src/
    creature_dna.py
    compile_graph.py
    verify_graph.py
  houdini/
    build_guides.py
  tests/
    test_compile_graph.py
    test_verify_graph.py
  Test-CreatureDnaProof.ps1
  Runs/                         generated, evaluation-only evidence
```

## Input Contract v0

The first recipe is intentionally mechanical. An AI can author it later, but the compiler
must not depend on prose interpretation.

```json
{
  "schema": "creature-dna/0",
  "id": "winged-reptile",
  "seed": 1701,
  "units": "m",
  "modules": [
    {"id": "body", "type": "torso", "length": 3.2, "radius": 0.72},
    {"id": "head", "type": "tapered_head", "parent": "body.front",
     "length": 1.1, "base_radius": 0.48, "tip_radius": 0.08},
    {"id": "tail", "type": "tail", "parent": "body.back",
     "length": 2.4, "segments": 5, "tip_radius": 0.04},
    {"id": "legs", "type": "leg", "parent": "body",
     "count": 4, "layout": "bilateral", "length": 1.35},
    {"id": "wings", "type": "wing", "parent": "body",
     "count": 2, "layout": "bilateral", "length": 2.6}
  ]
}
```

The normalized graph written by the compiler has one stable shape:

```python
{
    "schema": "anatomy-graph/0",
    "recipe": {"id": str, "sha256": str, "seed": int},
    "toolchain": {"compiler": str, "houdini": str | None, "edition": str | None},
    "modules": [{"id": str, "type": str, "instance": int}],
    "joints": [{"id": str, "module": str, "role": str,
                "position_m": [float, float, float], "radius_m": float}],
    "bones": [{"id": str, "parent_joint": str, "child_joint": str}],
    "sockets": [{"id": str, "joint": str, "accepts": [str]}],
    "intent": {"bilateral_pairs": [[str, str]], "contacts": [str]}
}
```

Canonical JSON uses sorted keys, UTF-8, compact separators, and rounded metric values
before SHA-256 hashing. No Houdini node IDs or file paths enter the checksum.

---

### Task 1: Transfer Ownership and Remove the Tool Lock

**Files:**

- Modify: `C:/repos/Gaters/docs/systems.md`
- Modify after accepted evidence only: `C:/repos/Gaters/research/machines.json`
- Create: `C:/repos/Gaters/.agents/exchanges/CHAR-6-creature-dna-houdini-proof.md`

**Step 1: Create the Character Generation handoff**

Record the approved proof boundary, proposed folder, pass/fail contract, and the fact that
Houdini is a challenger to the offline compiler role. Address it from Art Direction to
Character Generation & Animation.

**Step 2: Make `CHAR-2` tool-neutral before implementation**

Replace only the tool-specific ownership clause. Preserve Unreal's existing native
runtime authority. Target wording:

```markdown
- **CHAR-2 [MUST] — Native runtime authority:** Versioned offline generation tools own
  reproducible body, rig, skinning, physical-intent, and source-motion generation;
  Unreal owns live movement, floor queries, IK, collision, physics, recovery,
  replication, and gameplay authority through native systems behind versioned adapters.
```

Do not claim that Houdini has passed. This edit permits Blender, Houdini, or another
offline adapter to compete under the same contract.

**Step 3: Defer the registry change until evidence exists**

Do not add a Houdini machine speculatively. If the proof passes, return an `INTEGRATE`
packet proposing the smallest registry change: a Houdini procedural-source adapter as a
challenger dependency for `content.character-body-factory`. Do not replace the Blender
operator for rig or motion capabilities that Houdini has not proved.

**Step 4: Check shared requirements**

Expected record:

```text
Requirements checked: ART-1, ART-2, ART-3, ART-4, CHAR-1, CHAR-2, CHAR-3,
generated-content boundary; exceptions: ART-1 and ART-2 are not evaluated by this
mechanical proof, and no visual/body-factory promotion is claimed.
```

### Task 2: Establish the Pure CreatureDNA Compiler

**Files:**

- Create: `research/creature-dna-proof/README.md`
- Create: `research/creature-dna-proof/recipes/winged-reptile.json`
- Create: `research/creature-dna-proof/recipes/five-legged-challenge.json`
- Create: `research/creature-dna-proof/src/creature_dna.py`
- Create: `research/creature-dna-proof/src/compile_graph.py`
- Create: `research/creature-dna-proof/tests/test_compile_graph.py`

**Step 1: Write failing contract tests**

Tests must call a pure function with no Houdini import:

```python
graph = compile_recipe(load_recipe(path))
assert [m["type"] for m in graph["modules"]].count("leg") == expected_leg_count
assert [m["type"] for m in graph["modules"]].count("wing") == expected_wing_count
assert all(b["parent_joint"] != b["child_joint"] for b in graph["bones"])
```

The baseline expects four distinct leg instances and two wing instances. The held-out
challenge uses five legs with explicit attachment positions and no wings; it must compile
without a code edit. Odd counts are allowed only with explicit positions, never guessed
by the bilateral layout.

Run:

```powershell
py -m unittest discover -s research/creature-dna-proof/tests -v
```

Expected: tests fail because the compiler does not exist.

**Step 2: Implement the smallest parser and normalizer**

Use `json`, `dataclasses`, `math`, `hashlib`, and `pathlib` only. Reject unknown schema
versions, duplicate module IDs, unsupported module types, missing parents, non-finite
numbers, non-positive dimensions, odd bilateral counts, and ambiguous placement.

Expose only these public functions:

```python
def load_recipe(path: Path) -> dict: ...
def compile_recipe(recipe: dict) -> dict: ...
def canonical_bytes(graph: dict) -> bytes: ...
def graph_sha256(graph: dict) -> str: ...
```

Module builders return graph data; they do not call Houdini. Start with five builders:
`torso`, `tapered_head`, `tail`, `leg`, and `wing`.

**Step 3: Run the tests**

Run the same `unittest` command.

Expected: baseline and held-out structural tests pass.

**Step 4: Save a review checkpoint**

Inspect the normalized graph and confirm it contains no Houdini-specific fields. Do not
commit.

### Task 3: Add Independent Mathematical Verification

**Files:**

- Create: `research/creature-dna-proof/src/verify_graph.py`
- Create: `research/creature-dna-proof/fixtures/invalid/missing-parent.json`
- Create: `research/creature-dna-proof/fixtures/invalid/zero-length-bone.json`
- Create: `research/creature-dna-proof/tests/test_verify_graph.py`

**Step 1: Write counterexample tests first**

The verifier returns rule IDs, not a boolean with no diagnosis:

```python
report = verify(recipe, graph)
assert report["passed"] is False
assert report["failures"] == [{"rule": "GRAPH-PARENT-1", "subject": "head"}]
```

Cover these rules:

- `GRAPH-SCHEMA-1`: supported schemas and matching recipe identity.
- `GRAPH-NAME-1`: unique module, joint, bone, and socket IDs.
- `GRAPH-PARENT-1`: all declared parents and joints resolve.
- `GRAPH-CONNECT-1`: every module joins the root component.
- `GRAPH-LENGTH-1`: all bones are finite and longer than the configured epsilon.
- `GRAPH-BOUNDS-1`: dimensions remain within recipe-derived bounds.
- `GRAPH-MIRROR-1`: bilateral pairs mirror within configured tolerance.
- `GRAPH-TAPER-1`: tapered-head radii decrease toward the declared tip.
- `GRAPH-COUNT-1`: realized module counts equal recipe counts.

**Step 2: Implement verification separately from compilation**

`verify_graph.py` may share the JSON loader but must not import module-builder functions
or accept a generator-authored `passed` field. Compute distances, connectivity, counts,
and mirror deltas from raw points and edges.

**Step 3: Run tests**

```powershell
py -m unittest discover -s research/creature-dna-proof/tests -v
```

Expected: valid baseline and five-legged challenge pass; each malformed fixture fails
only its intended primary rule.

### Task 4: Materialize the Graph in Houdini

**Files:**

- Create: `research/creature-dna-proof/houdini/build_guides.py`
- Create: `research/creature-dna-proof/Test-CreatureDnaProof.ps1`

**Step 1: Detect the installed Houdini executable**

The PowerShell runner searches installed SideFX directories, selects a requested or
latest build, locates `bin/hython.exe`, and prints the exact edition/build. It must fail
with `HOUDINI_NOT_FOUND` instead of silently using another Python.

Detection core:

```powershell
$houdiniRoot = Get-ChildItem -LiteralPath 'C:\Program Files\Side Effects Software' -Directory |
  Where-Object { $_.Name -match '^Houdini \d' } |
  Sort-Object Name -Descending |
  Select-Object -First 1
$hythonPath = Join-Path $houdiniRoot.FullName 'bin\hython.exe'
if (-not (Test-Path -LiteralPath $hythonPath)) { throw 'HOUDINI_NOT_FOUND' }
```

**Step 2: Build a fresh scene from graph data**

`build_guides.py` must:

1. start from a blank scene;
2. compile the recipe with the pure compiler;
3. create one `/obj/CREATURE_DNA` geometry object;
4. create one point per joint and one open polygon per bone;
5. attach `joint_id`, `module_id`, `role`, `radius_m`, `bone_id`, and `side` attributes;
6. add color by module type for diagnosis only;
7. save the raw graph JSON, verification report, checksum, and `.hipnc`/`.hiplc` scene in
   one timestamped `Runs/<run-id>/` folder;
8. exit nonzero if independent verification fails.

The adapter may import `hou`; no other source file may import it.

**Step 3: Run the baseline in a fresh Houdini process**

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/creature-dna-proof/Test-CreatureDnaProof.ps1 `
  -Recipe research/creature-dna-proof/recipes/winged-reptile.json
```

Expected summary:

```text
CREATURE_DNA_PASS recipe=winged-reptile graph_sha256=<64 hex chars>
```

**Step 4: Run the held-out anatomy**

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/creature-dna-proof/Test-CreatureDnaProof.ps1 `
  -Recipe research/creature-dna-proof/recipes/five-legged-challenge.json
```

Expected: pass, five leg module instances, zero wing instances, and no source edit.

### Task 5: Prove Reproducibility and Preserve Evidence

**Files:**

- Modify: `research/creature-dna-proof/Test-CreatureDnaProof.ps1`
- Modify: `research/creature-dna-proof/README.md`

**Step 1: Add a repeat mode**

Run the compiler twice in separate Houdini processes. Compare canonical graph bytes and
their SHA-256 values. Do not checksum `.hip` files because Houdini may include unrelated
session metadata.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/creature-dna-proof/Test-CreatureDnaProof.ps1 `
  -Recipe research/creature-dna-proof/recipes/winged-reptile.json -Repeat 2
```

Expected:

```text
CREATURE_DNA_REPRO_PASS runs=2 graph_sha256=<same hash>
```

**Step 2: Run all negative fixtures**

Each fixture must exit nonzero and preserve recipe, raw graph if any, report, tool
versions, and the causal rule ID. The runner itself passes only when expected-negative
fixtures are rejected.

**Step 3: Inspect the Houdini scene once**

Open only the passing baseline scene. Confirm the object is front-readable, module colors
are distinct, joint names are inspectable, and the graph matches the report. Record one
labeled screenshot. Do not tune anatomy by eye during this task.

**Step 4: Record the decision**

- **Promote:** all hard checks pass. Return evidence to Primary Builder and write the
  next isolated plan: graph + radius envelopes → rough watertight VDB surface.
- **Stop:** any recipe requires code edits, results are not reproducible, or the verifier
  cannot diagnose malformed graphs. Preserve the failure and repair this layer before
  surface work.
- **License stop:** Apprentice cannot perform the required automated invocation. Preserve
  the exact failure and decide whether Indie is justified; do not build a Blender fallback
  inside this proof.

## Later Layers — Not Authorized by This Plan

Each layer begins only after the preceding layer has a promoted champion:

1. **Envelope compiler:** anatomy graph + radii → sampled implicit volumes.
2. **Proxy surface:** volumes → closed VDB union → polygon mesh with manifold checks.
3. **Shape controls:** StyleDNA modifies profiles and silhouettes without changing body
   roles.
4. **Production topology:** retopology and deformation loops under explicit budgets.
5. **Rig compiler:** graph roles → KineFX/APEX skeleton, controls, limits, and contacts.
6. **Skinning:** generated weights → deformation fixtures and repair.
7. **Motion:** body capabilities + intent → reusable or generated motion candidates.
8. **Unreal adapter:** import, native physics/IK/movement, and runtime evaluation.
9. **Semantic AI adapter:** prose or image analysis proposes CreatureDNA; schema and
   mechanical evaluators remain the authority.
10. **CreatureDNA MCP:** expose only semantic operations after the contracts stabilize:
    `compile_creature`, `set_parameter`, `validate_layer`, and `export_creature`.

## Final Plan Gate

Requirements checked: `ART-1`, `ART-2`, `ART-3`, `ART-4`, `CHAR-1`, `CHAR-2`,
`CHAR-3`, generated-content boundary; exceptions: `ART-1` and `ART-2` are deliberately
not evaluated by the mechanical guide proof, and no art, character-body, rig, motion, or
species-factory promotion is claimed.

The plan is invalid until the tool-neutral `CHAR-2` wording is integrated. After that
change, there are no requirement exceptions that block execution.
