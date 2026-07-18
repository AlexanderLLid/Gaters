# Experiment Run Archive Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans`. Subagents are intentionally not used because repository instructions do not authorize delegation.

**Goal:** Turn each completed headless environment seed run into one immutable, versioned JSONL record suitable for later evaluators, optimizers, and LLM research.

**Architecture:** Keep Unreal responsible only for reporting authoritative recipe/evaluator facts. A dependency-free PowerShell adapter validates one completed log, projects it into schema 1, computes a stable candidate ID, assigns a unique run ID, and appends exactly one compact JSON line; the existing sweep invokes it only when `-ArchivePath` is supplied.

**Tech Stack:** Unreal Engine 5.8 C++, PowerShell JSON and SHA256 APIs, JSONL.

## Global Constraints

- Append new records; never rewrite historical lines.
- Version generator, recipe, evaluator, archive schema, engine, and provenance independently.
- Reject incomplete source evidence instead of fabricating missing values.
- Repeated executions may create separate run evidence but must share one deterministic candidate ID.
- Preserve existing sweep behavior when no archive path is supplied.
- Do not branch, commit, or push; preserve unrelated human and Claude changes.

---

### Task 1: Immutable Environment Run Projection

**Files:**
- Create: `Unreal/Prototype/Scripts/Test-EnvironmentRunArchive.ps1`
- Create: `Unreal/Prototype/Scripts/Write-EnvironmentRun.ps1`

**Interfaces:**
- Consumes: one completed Unreal log containing `RECIPE`, `EVAL`, `SITE`, and `GEN` facts.
- Produces: one schema-1 JSONL `ExperimentRun` with stable `candidateId` and unique `runId`.

- [ ] **Step 1: Write the failing archive test**

  Create a temporary complete log, invoke the missing writer twice, and assert two immutable lines with distinct run IDs, identical candidate IDs, exact versioned recipe/evaluation fields, and preserved first-line bytes. Invoke it on an incomplete log and assert rejection without appending.

- [ ] **Step 2: Run the test to verify RED**

  Run: `& Unreal/Prototype/Scripts/Test-EnvironmentRunArchive.ps1`

  Expected: failure because `Write-EnvironmentRun.ps1` does not exist.

- [ ] **Step 3: Implement the minimum projection**

  Parse required facts, compute SHA256 over stable candidate inputs, capture UTC/run ID/git provenance, serialize with `ConvertTo-Json -Compress`, create the parent directory, and append one line.

- [ ] **Step 4: Run the test to verify GREEN**

  Expected: `PASS records=2` and archive remains unchanged after the rejected fixture.

---

### Task 2: Real Sweep Integration

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`
- Modify: `Unreal/Prototype/Scripts/RunEnvironmentSweep.ps1`
- Modify: `research/machines.json`

**Interfaces:**
- Consumes: `FGatersWorldRecipe::GeneratorVersion`, `Seed`, and `ChunkSize` plus optional sweep `ArchivePath`.
- Produces: `RECIPE schema=<n> generator=<n> seed=<n> chunk=<cm> ...` and one archive record per successful seed when requested.

- [ ] **Step 1: Expose required recipe provenance**

  Add generator version, seed, and chunk size to the existing `RECIPE` report without changing recipe identity or generation.

- [ ] **Step 2: Add opt-in sweep recording**

  Add `ArchivePath`; after a successful run, invoke the writer with engine/platform provenance. Leave output and filesystem behavior unchanged when omitted.

- [ ] **Step 3: Verify build, all worldgen tests, and a real archived sweep**

  Expected: build succeeds; all worldgen tests pass; a seed-7 run appends one parseable record matching its RECIPE/EVAL lines.

- [ ] **Step 4: Promote the registry machine**

  Mark `research.run-archive` verified with champion `schema-1` and move `currentFocus` to `world.terrain-semantic-field` only after all evidence gates pass.

## Self-review

- JSONL is sufficient until concurrent or remote workers create measured locking/query pressure.
- Raw Unreal logs remain referenced evidence; the archive stores normalized facts rather than replacing raw artifacts.
- Traversability and other future evaluators can extend later schemas without rewriting schema-1 records.
