# Settlement vertical slice v1

## Evidence

- Pure generator v1 produces six deterministic buildings: three homes plus community,
  workshop, and storage roles.
- Independent evaluator v1 rejects scarce terrain, a water-blocked entrance, collapsed
  facade orientation, and a building beyond the compact village radius.
- The first seed-7 runtime candidate was falsified at one orientation bucket; sector-aware
  selection repairs it to four without weakening the held-out rejection.
- Seeds `0`, `2`, `4`, `7`, and `53` each compile a valid settlement with six reachable
  entrances, three or four orientation buckets, and bounded paths across mountains,
  archipelago, lowlands, canyon, and mountain-lake terrain.
- Runtime seeds `0` through `15` produced valid settlements without rendering; a
  single-process held-out sweep then accepted all `64` unseen seeds and recorded `64`
  distinct relative building layouts.
- World Recipe schema 8 carries stable settlement module/path nodes. Modular foundations,
  walls, door frames, roofs, and public space now use four native ISM batches; paths remain
  semantic rather than being forced into uneven terrain.
- A close seed-7 capture exposed the original white spoke pattern. Compact radius and a
  neutral path material reduce its dominance; the close capture remains rejected by the
  existing gallery darkness guard, so perceptual village quality is not promoted.

## Boundary

- This proves a small placeholder layout, not finished buildings, parcels, interiors,
  decoration, NPC activity, economy, catalog swapping, or natural-looking village quality.
- Physical asset fit remains pending until contracted building assets replace semantic
  modules. The generator and evaluator remain active prototypes rather than verified machines.
