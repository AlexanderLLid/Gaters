# Regional water fit evaluator implementation plan

**Goal:** Reject logically wrong regional water without judging unfinished water art.

**Architecture:** A pure evaluator consumes world intent plus a generated regional-water
recipe. It independently derives expected surface counts from hydrology, checks identity,
bounds, water datum, dry leakage, and samples the declared terrain family for submerged
ground. It never creates or inspects Unreal components.

**Tech stack:** Unreal Engine 5.8 C++ and existing automation.

## Constraints

- Do not score materials, color, opacity, waves, foam, reflections, or shoreline beauty.
- Absence is valid when regional intent is dry.
- Diagnostics name the responsible region and failed physical rule.
- Requirements checked: Global none recorded; exceptions: none.

### Task 1: Pure physical evaluator

- [x] Write failing acceptance, missing-surface, wrong-datum, and dry-leak tests.
- [x] Implement minimum causal evaluation and terrain-intersection sampling.
- [x] Run focused automation until green.

### Task 2: Close evidence

- [x] Run full Gaters automation and validators.
- [x] Update machine truth without claiming final water art quality.
