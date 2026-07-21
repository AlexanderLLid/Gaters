# Intent terrain fidelity evaluator implementation plan

**Goal:** Independently determine whether observed terrain preserves the seed-declared
global terrain and every regional core.

**Architecture:** The evaluator consumes observations supplied by a generator or runtime
caller. It never invokes the intent-aware terrain field. It computes expected global and
regional-core heights from the authoritative intent plus the already verified terrain
families, checks causal coverage/error, and emits region-specific diagnostics.

**Tech stack:** Unreal Engine 5.8 C++, existing automation framework.

## Constraints

- No resource, biome, terrain-family, or hydrology presence guarantee.
- A region fails only when observed output does not match its own declaration.
- Blend-annulus aesthetics and local water surfaces remain separate checks.

### Task 1: Pure evaluator

**Files:**

- Create: `GatersIntentTerrainFidelityEvaluator.h/.cpp`
- Create: `Tests/GatersIntentTerrainFidelityEvaluatorTests.cpp`

- [x] Write failing tests for accepted generator observations, corrupted regional height,
  and missing regional evidence.
- [x] Implement minimum observation/result/diagnostic contracts.
- [x] Run focused automation until green.

### Task 2: Verification and registry

- [ ] Run full Gaters automation and shared validators.
- [ ] Record truthful machine status and the remaining water/blend boundary.
