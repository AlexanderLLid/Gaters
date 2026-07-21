# Houdini Third-Person Style Proof Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build and render one parameterized reptilian head in Houdini Apprentice so the human can judge restrained painted mid-detail at third-person distances.

**Architecture:** Reuse the procedural horn proof's direct `hou.Geometry` pattern. A JSON recipe drives a single Houdini builder that emits grouped head parts, vertex colours, one `.hipnc`, one non-commercial geometry cache, and a summary. A small verifier reopens the scene and checks geometry plus the three fixed render artifacts; human judgment remains the visual gate.

**Tech Stack:** Houdini 22.0.368 Apprentice, `hython`, Python standard library, Houdini `hou` API, built-in Houdini OpenGL/Karma non-commercial rendering.

## Global Constraints

- Use only Houdini Apprentice capabilities; no Houdini Engine, third-party renderer, Indie, or commercial license.
- Keep all source and outputs under `research/character-style-exploration/procedural-reptile-head-proof/`.
- Meet `ART-1`, `ART-3`, and `ART-4`; do not touch character-generation, rigging, animation, or Unreal assets.
- Produce neutral-light labeled renders for standard gameplay distance, gameplay close distance, and inspection close-up.
- Do not commit.

---

### Task 1: Recipe and Geometry Contract

**Files:**
- Create: `research/character-style-exploration/procedural-reptile-head-proof/head-recipe.json`
- Create: `research/character-style-exploration/procedural-reptile-head-proof/build_head.py`
- Create: `research/character-style-exploration/procedural-reptile-head-proof/test_head.py`

**Interfaces:**
- Consumes: `head-recipe.json` with schema `reptile-head-proof/0`.
- Produces: `load_recipe(path: Path) -> dict`, `build(recipe: dict) -> tuple[hou.Geometry, dict]`, and `write_artifacts(recipe_path: Path, output_dir: Path) -> dict`.

- [ ] **Step 1: Write the failing contract test**

```python
def test_recipe_rejects_invalid_resolution(self):
    recipe = json.loads((ROOT / "head-recipe.json").read_text(encoding="utf-8"))
    recipe["surface_u"] = 3
    with tempfile.TemporaryDirectory() as directory:
        bad = Path(directory) / "bad.json"
        bad.write_text(json.dumps(recipe), encoding="utf-8")
        result = subprocess.run([str(HYTHON), str(ROOT / "build_head.py"), str(bad), directory], capture_output=True, text=True)
        self.assertNotEqual(result.returncode, 0)
```

- [ ] **Step 2: Run the focused test and verify failure**

Run: `py -m unittest research/character-style-exploration/procedural-reptile-head-proof/test_head.py -v`

Expected: FAIL because `build_head.py` and the recipe do not exist.

- [ ] **Step 3: Add the minimal versioned recipe and validation**

The recipe must contain finite bounded controls for skull length/width/height, muzzle taper, jaw depth, brow height, eye spacing/size, nostril size, plate count, scale amplitude, asymmetry, colours, roughness, and surface resolution. `load_recipe` must reject unknown schema, non-finite values, and resolutions below eight samples per axis.

- [ ] **Step 4: Build one connected primary head surface and separate semantic parts**

Use analytic cross-sections along the head axis for cranium, cheek, muzzle, and jaw mass. Add separate closed eye globes, eyelid rims, nostril insets, and a mouth seam. Store point colour `Cd` and primitive string `feature` values: `skin`, `eye`, `pupil`, `mouth`, and `nostril`.

- [ ] **Step 5: Run the focused test**

Expected: recipe validation passes; generated positions are finite; every polygon has positive area; required feature groups exist.

### Task 2: Apprentice Scene and Fixed Views

**Files:**
- Modify: `research/character-style-exploration/procedural-reptile-head-proof/build_head.py`
- Modify: `research/character-style-exploration/procedural-reptile-head-proof/test_head.py`

**Interfaces:**
- Consumes: geometry and summary from Task 1.
- Produces: `reptile-head.hipnc`, `reptile-head.bgeo.sc`, and camera nodes `CAM_GAMEPLAY`, `CAM_CLOSE`, `CAM_INSPECT`.

- [ ] **Step 1: Extend the test with Apprentice scene assertions**

```python
self.assertEqual(summary["license_category"], "Apprentice")
self.assertTrue((output / "reptile-head.hipnc").is_file())
self.assertTrue((output / "reptile-head.bgeo.sc").is_file())
for camera in ("CAM_GAMEPLAY", "CAM_CLOSE", "CAM_INSPECT"):
    self.assertIn(camera, summary["cameras"])
```

- [ ] **Step 2: Run the test and verify failure**

Expected: FAIL because the scene, cache, and cameras are absent.

- [ ] **Step 3: Create the minimal Houdini scene**

Create `/obj/REPTILE_HEAD/OUT_HEAD`, three cameras looking at the same head centre, a neutral background, one broad key light, one weak fill, and no dramatic rim or coloured illumination. Save with `hou.hipFile.save()` as `.hipnc`; save the geometry cache with Houdini's non-commercial format.

- [ ] **Step 4: Reopen the saved scene in a fresh `hython` process**

Assert the output node exists, point and primitive counts match the summary, required feature groups survive, and all three cameras exist.

- [ ] **Step 5: Run the focused test**

Expected: PASS for scene creation and fresh reopen under the installed Apprentice license.

### Task 3: Render and Human Evaluation Package

**Files:**
- Create: `research/character-style-exploration/procedural-reptile-head-proof/render_views.py`
- Modify: `research/character-style-exploration/procedural-reptile-head-proof/test_head.py`
- Create: `research/character-style-exploration/procedural-reptile-head-proof/README.md`
- Generate: `research/character-style-exploration/procedural-reptile-head-proof/Derived/reptile-head-v0/`

**Interfaces:**
- Consumes: saved `.hipnc` and named cameras.
- Produces: `A-gameplay.png`, `B-gameplay-close.png`, `C-inspection.png`, `head-summary.json`, copied recipe, and `evaluation.json` with `humanAcceptance: false` until reviewed.

- [ ] **Step 1: Extend the test with render assertions**

```python
for name in ("A-gameplay.png", "B-gameplay-close.png", "C-inspection.png"):
    path = output / name
    self.assertTrue(path.is_file())
    with Image.open(path) as image:
        self.assertGreaterEqual(image.width, 960)
        self.assertGreaterEqual(image.height, 540)
```

- [ ] **Step 2: Run the test and verify failure**

Expected: FAIL because fixed-view renders do not exist.

- [ ] **Step 3: Render with a built-in Apprentice renderer**

Render all three cameras at 1280×720 or the installed Apprentice limit. PNG watermarking is acceptable. If headless OpenGL is unavailable, use built-in Karma CPU; do not install or invoke a third-party renderer.

- [ ] **Step 4: Write the evaluation package**

Record recipe SHA-256, Houdini version, license category, point/polygon counts, render identities, mechanical pass/fail, `humanAcceptance: false`, and an empty `visibleFailure` pending review.

- [ ] **Step 5: Run complete verification**

Run: `py -m unittest research/character-style-exploration/procedural-reptile-head-proof/test_head.py -v`

Expected: all tests PASS and the scene reopens in Apprentice.

- [ ] **Step 6: Present the three labeled images**

Show A, B, and C directly in chat. Make no quality or production-feasibility claim before the human accepts or rejects the visible result.

Requirements checked: `ART-1`, `ART-3`, `ART-4`; exceptions: `ART-2` does not apply to a non-human creature-head proof.
