# Profiled Mid-Poly Body Design

Status: approved by Human.

## Outcome

- The existing BodyPlan and anatomical guide produce a more recognizable humanoid
  surface at a configurable mid-poly budget.
- Mid-poly is an experiment target, not the selected game art direction.
- Baseline and held-out proportions run through unchanged code and emit labelled views.

## Design

- Extend the existing anatomical-guide recipe instead of creating another body graph.
- Replace the single head volume with recipe-driven cranium and lower-face volumes.
- Add recipe-driven shoulder volumes and reshape terminal hand/foot volumes.
- Add an optional Houdini polygon target after VDB conversion; labels are restored after
  reduction from the tool-neutral guide.
- Keep landmarks, skeleton roles, skinning inputs, and motion contracts unchanged.

## Evidence gate

- Mechanical: one connected closed surface, preserved module/guide provenance,
  deterministic replay, face budget, baseline/held-out support, no non-finite geometry.
- Visual: labelled front and three-quarter renders from the generated mesh. Human review
  decides whether the silhouette is materially more recognizable than the mannequin.
- Claim boundary: no final style, face detail, fingers, toes, clothing, deformation
  topology, skin quality, or runtime promotion.

Requirements checked: `ART-1`, `ART-3`, `ART-4`, `CHAR-3`, `CHAR-4`, `CHAR-5`;
exceptions: `ART-2` is not evaluated by one neutral mannequin.
