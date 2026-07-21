# Procedural Biological Horn Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use
> `superpowers:subagent-driven-development` (recommended) or
> `superpowers:executing-plans` to implement this plan task-by-task. Steps use checkbox
> (`- [ ]`) syntax for tracking.

**Goal:** Generate one visually detailed, biologically grounded ram-style horn in Houdini
from a versioned mathematical recipe, then preserve an editable scene, OBJ, labeled render,
and independent geometric evidence.

**Architecture:** A standalone Art-owned Houdini script samples a three-dimensional
logarithmic spiral centerline and sweeps an ovate power-law-tapered cross-section along it.
Transverse growth ridges, longitudinal keratin striations, asymmetry, and shallow chipped
regions are deterministic displacements over that base surface. A standard-library policy
checks the exported manifest and topology; a separate renderer creates diagnostic views.

**Tech Stack:** Houdini 22.0.368 Apprentice, Houdini Python, Python standard library,
Pillow 9.5, JSON, OBJ, `unittest`.

## Global Constraints

- Work only under `research/character-style-exploration/procedural-horn-proof/`.
- Do not modify or count this as evidence for CreatureDNA, body-family generation,
  rigging, skinning, animation, Blender, or Unreal.
- Preserve a versioned recipe, source scripts, tool versions, evaluator report, `.hipnc`,
  OBJ, and labeled render.
- Use one recipe and one biological feature. Do not build a general organ framework.
- Apprentice outputs are noncommercial feasibility evidence only.
- No branch, commit, or push unless the human asks.

Biological basis:

- The centerline is spiral-like, but the actual solid uses a power-law radius-over-length
  taper because a spiral alone describes a path rather than a horn volume
  (`https://pmc.ncbi.nlm.nih.gov/articles/PMC8008625/`).
- Surface ridges and longitudinal structure represent a keratinous sheath at visible game
  scale, not literal microscopic tubules
  (`https://pmc.ncbi.nlm.nih.gov/articles/PMC6030630/`).

---

### Task 1: Mathematical Horn Mesh

**Files:**

- Create: `research/character-style-exploration/procedural-horn-proof/horn-recipe.json`
- Create: `research/character-style-exploration/procedural-horn-proof/build_horn.py`
- Create: `research/character-style-exploration/procedural-horn-proof/test_horn.py`

**Interfaces:**

- Consumes: one `horn-proof/0` JSON recipe.
- Produces: `build(recipe: dict) -> tuple[hou.Geometry, dict]` and a summary containing
  sample counts, bounds, taper endpoints, ridge count, minimum polygon area, edge-use
  counts, and tool versions.

- [ ] **Step 1: Write the failing Houdini test**

Assert that `build_horn.py` creates a closed polygon horn with the recipe's exact axial
and radial resolution, no non-finite points, no degenerate polygons, every undirected edge
used exactly twice, a smaller tip than base, and the requested ridge count.

- [ ] **Step 2: Run the test and observe the missing-builder failure**

```powershell
py -m unittest research/character-style-exploration/procedural-horn-proof/test_horn.py -v
```

- [ ] **Step 3: Implement the minimum mesh generator**

Use these explicit functions:

```python
def load_recipe(path: Path) -> dict: ...
def sample_centerline(recipe: dict) -> list[tuple[float, float, float]]: ...
def build(recipe: dict) -> tuple[hou.Geometry, dict]: ...
def evaluate(vertices: list, faces: list, recipe: dict) -> dict: ...
```

Centerline and taper:

```text
theta(s) = turns * 2*pi*s
coil_radius(s) = coil_radius_base * exp(-coil_decay * theta(s))
section_radius(s) = tip_radius + (base_radius-tip_radius) * (1-s)^taper_power
s in [0, 1]
```

Ridge and striation displacement multiply `section_radius(s)` without changing topology.
Chips are bounded angular depressions, not Boolean cuts.

- [ ] **Step 4: Run the test and require PASS**

### Task 2: Houdini Artifact and Diagnostic Render

**Files:**

- Create: `research/character-style-exploration/procedural-horn-proof/render_preview.py`
- Create: `research/character-style-exploration/procedural-horn-proof/README.md`
- Modify: `research/character-style-exploration/procedural-horn-proof/test_horn.py`

**Interfaces:**

- Consumes: generated `.hipnc`, OBJ, summary and recipe.
- Produces: `horn-detail.hipnc`, `horn-detail.obj`, `horn-summary.json`, and
  `horn-preview.png` with labeled three-quarter and side views.

- [ ] **Step 1: Extend the failing test**

Run the builder in `hython`, reopen the saved scene in a second `hython` process, and
require `/obj/BIOLOGICAL_HORN/OUT_HORN` to contain the same point and polygon counts as
the independent summary. Require the PNG to exist and have nonzero dimensions.

- [ ] **Step 2: Implement artifact writing and rendering**

Use a Houdini `stash` SOP for self-contained editable scene evidence. The renderer reads
the exported OBJ and uses flat lighting with no image texture or AI-generated detail.

- [ ] **Step 3: Run the isolated test and full Art proof checks**

```powershell
py -m unittest research/character-style-exploration/procedural-horn-proof/test_horn.py -v
powershell -NoProfile -ExecutionPolicy Bypass -File research/Test-SharedAgentDocs.ps1
```

- [ ] **Step 4: Human-visible gate**

Present the labeled render. Promote only as evidence that Houdini can generate one
detailed biological hard-surface feature; do not infer complete creature or
animation-ready mesh capability.

## Requirements Check

Requirements checked: `ART-1`, `ART-3`, `ART-4`; exceptions: `ART-2` is unrelated to a
non-human horn, and no Character Generation & Animation capability is claimed.
