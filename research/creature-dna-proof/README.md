# CreatureDNA Houdini Proof

This isolated feasibility proof compiles explicit creature modules into a tool-neutral
anatomy graph, verifies the graph mathematically, then materializes and reopens the guide
inside Houdini.

## Run

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/creature-dna-proof/Test-CreatureDnaProof.ps1
```

Run the held-out five-leg body plan without changing code:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File research/creature-dna-proof/Test-CreatureDnaProof.ps1 `
  -Recipe research/creature-dna-proof/recipes/five-legged-challenge.json
```

A passing run prints `CREATURE_DNA_REPRO_PASS` and writes two independently generated
run folders under `Runs/`.

## Inspect in Houdini

1. Open a generated `creature-guide.hipnc`.
2. Select `/obj/CREATURE_DNA/OUT_GUIDES` in the Network view.
3. Hover the Scene view and press `H` to frame the complete guide.

Colors identify module families: grey torso, orange head, green tail, blue legs, and
purple wings. This is deliberately a structural stick guide, not a surface or art proof.

## Creature-like proxy

The bounded proxy experiment turns the same verified graph into overlapping spheres,
tapered bone volumes, and simple wing membranes:

```powershell
& 'C:\Program Files\Side Effects Software\Houdini 22.0.368\bin\hython.exe' `
  research/creature-dna-proof/houdini/build_proxy.py `
  research/creature-dna-proof/recipes/winged-reptile.json `
  research/creature-dna-proof/Runs/creature-proxy-v0
```

Open `Runs/creature-proxy-v0/creature-proxy.hipnc`, select
`/obj/CREATURE_DNA/OUT_PROXY`, and press `H` over the viewport. This proxy demonstrates
data-driven volume placement only; it is not watertight production topology.

## Authority

- Recipes, compiler source, evaluator source, versions, and reports are authoritative.
- `.hipnc`, OBJ, and displayed geometry are derived noncommercial evaluation output.
- No Blender, Unreal, rigging, animation, surface generation, or visual-style capability
  is claimed by this proof.
