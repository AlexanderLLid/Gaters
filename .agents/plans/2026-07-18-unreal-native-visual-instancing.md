# Unreal-native visual instancing

## Goal

Render repeated scatter and claim-marker placeholders through Unreal ISM batches without changing recipe identity, persistence, harvesting, claiming, or destructible base behavior.

## Boundary

- Gaters owns deterministic recipe nodes, stable IDs, diff replay, and the grouping adapter.
- Unreal owns mesh components, instance storage, rendering, culling, collision primitives, and profiling APIs.
- Interactive actors remain temporarily as invisible identity/collision carriers.
- Destructible base pieces remain unique actors.
- PCG, World Partition, Mass, Nanite policy, and GPU profiling are outside this increment.

## TDD sequence

1. Add a failing pure planning test for tree, rock, open-claim, claimed-claim, and chopped-scatter partitioning.
2. Add the smallest visual materializer plan and native ISM adapter.
3. Route chunk visuals through the adapter while preserving interactive actors.
4. Prove recipe/diff behavior with automation and four held-out runtime seeds.
5. Record the native ownership boundary and measured evidence honestly in `research/machines.json`.

## Success

- All Gaters automation passes.
- Held-out worlds retain the same semantic counts and zero performance issues.
- Performance reports show native instanced components and fewer visible static-mesh components than instances.
- The registry calls the materializer an Unreal ISM adapter, not a custom rendering system.
