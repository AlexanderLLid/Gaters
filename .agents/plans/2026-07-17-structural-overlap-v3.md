# Structural Overlap v3 Implementation Plan

> **For agentic workers:** Execute inline with TDD. Do not branch, commit, or push unless the human asks.

**Goal:** Reject disconnected or physically overlapping contracted base pieces before materialization.

**Architecture:** Contract-aware structural evaluation derives conservative world-space boxes from each BasePiece contract and node transform, checks pairwise penetration against an explicit tolerance, and verifies the BasePiece link graph is connected. The evaluator emits evidence only; it does not move pieces.

**Tech Stack:** Unreal Engine 5.8 C++, asset contracts, recipe links, structural evaluator, Unreal automation tests.

## Global Constraints

- Evaluate only contracted BasePiece nodes in this slice.
- Bounds are declared conservative envelopes; no mesh or physics query is permitted.
- Touching within tolerance is valid; measurable penetration is invalid.
- One piece is connected by definition; multiple pieces require one connected link component.
- Do not commit or branch.

### Task 1: Failing spatial fixtures

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersStructuralEvaluatorTests.cpp`

- [x] Add fixtures for touching pieces, penetrating pieces, invalid contract bounds, and disconnected pieces.
- [x] Run the focused suite and verify the new rules are absent.

### Task 2: Bounds and connectivity rules

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersStructuralEvaluator.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersStructuralEvaluator.cpp`

- [x] Add explicit overlap tolerance to the structural context.
- [x] Validate each BasePiece contract and derive its transformed conservative box.
- [x] Emit measured overlap and disconnected-graph diagnostics.
- [x] Run focused and complete Gaters suites.

### Task 3: Promotion evidence

**Files:**
- Modify: `research/machines.json`
- Modify: `.agents/plans/2026-07-17-structural-overlap-v3.md`

- [x] Mark structural evaluator verified only if every registry promotion case now has a passing fixture.
- [x] Advance current focus through the now-verified site/route planner to the next dependency-eligible machine.
- [x] Run workflow verifiers and diff whitespace check.
