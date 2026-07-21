# Houdini Third-Person Style Proof

## Outcome

Produce one Art-owned Houdini reptilian head that lets the human judge whether restrained painted mid-detail can hold up at a standard Unreal third-person camera distance.

## Artifact

- One editable `.hipnc` built from reproducible geometry and material parameters.
- One versioned recipe containing the visible controls and seed.
- Three labeled neutral-light renders: standard gameplay distance, gameplay close distance, and inspection close-up.
- Geometry and artifact checks sufficient to reproduce the proof.

## Visual target

- Grounded reptile anatomy and smooth readable forms.
- Designed silhouette, visible eyes, mouth, nostrils, facial plates, and limited large scales.
- Restrained painterly colour and roughness variation.
- No photoreal pore detail, dense individual scales, cel shading, toy proportions, or deliberately faceted low-poly presentation.

## Boundary

- This tests isolated visual feasibility only.
- It does not test a complete species, body generation, topology for deformation, rigging, skinning, animation, Unreal integration, or runtime performance.
- Existing character-generation machinery remains untouched.

## Evaluation

- Human judgment at the three fixed camera distances is the acceptance gate.
- Mechanical checks verify recipe replay, finite geometry, non-degenerate faces, source save/reopen, export, and render presence.
- Preserve the candidate even if rejected, with the visible failure recorded.

Requirements checked: `ART-1`, `ART-3`, `ART-4`; exceptions: `ART-2` does not apply to a non-human creature-head proof.
