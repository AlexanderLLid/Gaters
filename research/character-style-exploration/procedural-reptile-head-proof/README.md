# Procedural Reptile Head Proof

Art-owned Houdini Apprentice proof for judging a restrained painted mid-detail creature head at fixed third-person distances.

## Generate

```powershell
& 'C:\Program Files\Side Effects Software\Houdini 22.0.368\bin\hython.exe' `
  build_head.py head-recipe.json Derived/reptile-head-v4

& 'C:\Program Files\Side Effects Software\Houdini 22.0.368\bin\hython.exe' `
  render_views.py Derived/reptile-head-v4/reptile-head.hipnc Derived/reptile-head-v4
```

## Verify

```powershell
py -m unittest test_head.py -v
```

## Inspect

Open `Derived/reptile-head-v4/reptile-head.hipnc`, select `/obj/REPTILE_HEAD/OUT_HEAD`, hover the viewport, and press `H`.

## Result

- Apprentice-only source, built-in Mantra rendering, no paid features.
- Parameterized source geometry, SDF skin fusion, low-frequency painted colour, fixed cameras, replay identities, and mechanical checks work.
- Visual review rejects Wartales-level fidelity: the primitive/SDF construction remains a creature blockout rather than authored production anatomy.
- This does not test body generation, deformation topology, rigging, animation, Unreal integration, or runtime performance.
