# Skeletal Asset Intake Implementation Plan

> **For agentic workers:** Execute inline with TDD. Do not branch, commit, or push unless the human asks.

**Goal:** Independently reject skeletal assets whose measured bounds, physics policy, render class, or required bone ports violate their contract.

**Architecture:** Extend the existing intake adapter with one `USkeletalMesh` overload. It measures the loaded fixture directly, validates declared contract-space conversion, and treats skeleton bones or mesh sockets as named ports without importing or materializing anything.

**Tech Stack:** Unreal Engine 5.8 C++, `USkeletalMesh`, existing asset contracts, Unreal automation tests.

## Global Constraints

- Reuse the existing asset contract; add no character-specific schema.
- A physics asset satisfies Simple collision; absence satisfies None; Complex is rejected for skeletal intake.
- Port names may resolve to a skeletal-mesh socket or reference-skeleton bone.
- Do not commit or branch.

### Task 1: Skeletal fixture and counterexamples

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersAssetIntakeTests.cpp`

- [x] Add a real Manny fixture contract plus wrong class, bounds, collision, and missing-bone counterexamples.
- [x] Build and confirm the absent skeletal intake API fails compilation.

### Task 2: Skeletal intake adapter

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersAssetIntake.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersAssetIntake.cpp`

- [x] Implement contract, render-class, bounds, collision, and named-port checks.
- [x] Run focused and complete Gaters suites.

### Task 3: Promotion evidence

**Files:**
- Modify: `research/machines.json`
- Modify: `.agents/plans/2026-07-17-skeletal-asset-intake.md`

- [x] Promote the intake linter and catalog only if their full challenge suites pass.
- [x] Advance current focus to the world compiler.
- [x] Run workflow verifiers and diff whitespace check.
