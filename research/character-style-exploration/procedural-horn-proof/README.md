# Procedural Biological Horn Proof

One Art-owned Houdini feasibility test for a detailed biological hard-surface feature.
It uses a spiral centerline, power-law taper, ovate cross-section, growth ridges,
longitudinal keratin striation, mild asymmetry, and bounded chipped regions.

## Generate

```powershell
& 'C:\Program Files\Side Effects Software\Houdini 22.0.368\bin\hython.exe' `
  research/character-style-exploration/procedural-horn-proof/build_horn.py `
  research/character-style-exploration/procedural-horn-proof/horn-recipe.json `
  research/character-style-exploration/procedural-horn-proof/Derived/ram-horn-v0

py research/character-style-exploration/procedural-horn-proof/render_preview.py `
  research/character-style-exploration/procedural-horn-proof/Derived/ram-horn-v0/horn-detail.obj `
  research/character-style-exploration/procedural-horn-proof/Derived/ram-horn-v0/horn-preview.png
```

## Test

```powershell
py -m unittest research/character-style-exploration/procedural-horn-proof/test_horn.py -v
```

The test checks finite geometry, exact recipe resolution, closed manifold edge usage,
polygon area, taper, Houdini save/reopen, OBJ export, and preview output.

## Inspect

Open `Derived/ram-horn-v0/horn-detail.hipnc`, select
`/obj/BIOLOGICAL_HORN/OUT_HORN`, hover the viewport, and press `H`.

This proves one reproducible detailed horn surface. It does not prove CreatureDNA
integration, complete creature generation, production retopology, rigging, or animation.
