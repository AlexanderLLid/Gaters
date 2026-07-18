# Typed Support and Scatter Fit Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans`
> to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Distinguish terrain support from unresolved piece attachment and align scatter
assets to their declared terrain contact before Unreal materializes them.

**Architecture:** `FGatersAssetContact` gains one support enum with terrain as the
backward-compatible default. Physical-fit samples only terrain contacts and records
attachment contacts as pending evidence. The world compiler uses the first terrain
contact of tree/rock contracts to move the compiled transform so that contact lands on
the recipe's semantic ground anchor; base transforms remain untouched.

**Tech Stack:** Unreal Engine 5.8 C++, existing asset contracts/compiler/physical-fit,
Unreal Automation Tests.

## Global Constraints

- Do not touch Blender files, art style, terrain geometry, or recipe identity.
- Do not implement attachment solving, engine traces, navigation, or automatic repair.
- Preserve existing behavior for contacts that do not declare a support kind.
- Only scatter tree/rock nodes receive contract contact alignment.
- Do not commit, branch, or push.

---

### Task 1: Type contact support and preserve uncertainty

**Files:**
- Modify test first: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersPhysicalFitEvaluatorTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersAssetContract.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersAssetContract.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersPhysicalFitEvaluator.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersPhysicalFitEvaluator.cpp`

**Interfaces:**
- Produces: `EGatersAssetContactSupport::{Terrain,Attachment}` and
  `FGatersAssetContact::Support` defaulting to `Terrain`.
- Produces: evaluation counts `EvaluatedTerrainContacts` and
  `PendingAttachmentContacts`; attachment contacts create neither fake terrain samples
  nor false passes hidden as complete evidence.

- [x] Write a failing fixture with two terrain contacts plus one attachment contact.
  Require two sampled contacts, two evaluated terrain contacts, and one pending attachment.
- [x] Build and verify red because the support enum/counts do not exist.
- [x] Add the enum, validation, filtered terrain sampling/evaluation, and explicit counts.
- [x] Build and run `Gaters.Evaluation.PhysicalFit`; expect all fixtures green.

---

### Task 2: Declare runtime base support intent

**Files:**
- Modify: `Unreal/Prototype/Source/Prototype/Public/GatersChunk.h`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersChunk.cpp`

**Interfaces:**
- `FBaseStampRow` carries support intent.
- Foundations and fences declare terrain support; walls, frames, doors, and ceilings
  default to attachment support.
- Runtime FIT summary adds `terrain_contacts` and `attachment_pending`.

- [x] Thread `FBaseStampRow::ContactSupport` into placeholder contract registration.
- [x] Mark only foundation and fence rows as terrain-supported.
- [x] Build and run seed 7; expect attachment contacts to move from false terrain
  failures into the explicit pending count.

---

### Task 3: Align scatter contracts to semantic ground anchors

**Files:**
- Modify test first: `Unreal/Prototype/Source/Prototype/Private/Tests/GatersWorldCompilerTests.cpp`
- Modify: `Unreal/Prototype/Source/Prototype/Private/GatersWorldCompiler.cpp`

**Interfaces:**
- For `ScatterTree` and `ScatterRock`, the recipe location is the desired terrain-contact
  location. The compiled transform is translated by `recipe location - transformed
  contract contact` after scale and rotation.
- Nodes without a terrain contact and every non-scatter node retain the recipe transform.

- [x] Add a failing tree fixture whose scaled contact is below its origin. Require the
  compiled contact world position to equal the recipe location and the base-piece
  transform to remain unchanged.
- [x] Run the focused compiler test and verify red against the currently buried contact.
- [x] Add the minimum scatter-only translation after selected contract validation.
- [x] Run compiler, materializer, physical-fit, full Gaters automation, and seed 7.

---

### Task 4: Record the evidence

**Files:**
- Modify: `research/machines.json`
- Modify: `.agents/reports/streamed-content-runtime.md`
- Modify: `.agents/workflow-feedback.md` only if a new workflow failure appears.

- [x] Record observed terrain failures and pending attachments without promoting the
  attachment/collision/navigation guarantees.
- [x] Run registry validation and `git diff --check`.

## Self-review

- One enum expresses the independent failure modes; no parallel contact schema exists.
- Pending attachment evidence is visible rather than treated as zero.
- Scatter fitting reuses selected contract data and keeps the recipe asset-agnostic.
- Base construction remains unchanged until an attachment evaluator exists.
