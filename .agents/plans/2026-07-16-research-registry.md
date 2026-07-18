# Research Registry Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:executing-plans`. Subagents are intentionally not used because repository instructions do not authorize delegation.

**Goal:** Create one authoritative, machine-readable capability graph for generating and evaluating complete playable worlds, settlements, bases, species, assets, animation, and simulations end to end.

**Architecture:** `research/machines.json` owns every machine fact, dependency, verification contract, status, and target wave. A dependency-free PowerShell reader validates the graph and derives human-readable waves or Mermaid; derived views are printed, not committed.

**Tech Stack:** JSON, PowerShell 7/Windows PowerShell-compatible script, repository agent instructions.

## Global Constraints

- One fact has one owner; no manually maintained duplicate overview.
- The immutable report in `docs/raw/` remains research input, not current state.
- Capability records describe buildable guarantees, not speculative species or lore.
- Every Adapt/Build machine declares a verifier, challenge set, failure artifact, and promotion gate.
- Preserve unrelated dirty-worktree changes; do not branch, commit, or push.

---

### Task 1: Executable Registry Contract

**Files:**
- Create: `research/Test-MachineRegistry.ps1`
- Create: `research/Show-MachineRegistry.ps1`
- Create: `research/machines.json`
- Modify: `AGENTS.md`

**Interfaces:**
- Consumes: `research/machines.json` records with stable IDs and dependency groups.
- Produces: validation exit code, ordered build-wave summary, and generated Mermaid text.

- [ ] **Step 1: Write the failing registry test**

  Invoke the missing reader against a missing registry and require a non-zero exit; after implementation, assert validation succeeds, output contains the dream machine and every target wave, and both summary/Mermaid views are generated from the same file.

- [ ] **Step 2: Run the test to verify RED**

  Run: `& research/Test-MachineRegistry.ps1`

  Expected: failure because the reader and registry do not exist.

- [ ] **Step 3: Implement the minimum reader and registry**

  Validate schema version, unique IDs, dependency targets, causal wave order, required verifier fields, dream-machine identity, and cycles. Print records ordered by wave and ID; generate Mermaid edges from the same dependency groups.

- [ ] **Step 4: Point both agents at the registry owner**

  Add only the ownership rule and commands to `AGENTS.md`; do not copy machine facts into instructions.

- [ ] **Step 5: Run checks**

  Run the test, summary, Mermaid generation, JSON parse, and `git diff --check`.

  Expected: all exit `0`; the graph has no missing dependency, cycle, duplicate ID, or later-wave prerequisite.

## Self-review

- The registry is the plan; its `targetWave` fields produce ordering without another roadmap.
- The script uses built-in JSON and collection support only.
- Runtime recipes, content artifacts, canon, and experiment evidence remain separate authorities connected by stable IDs.
