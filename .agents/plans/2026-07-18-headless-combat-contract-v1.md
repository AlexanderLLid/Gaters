# Headless Combat Contract V1 Implementation Plan

> **For agentic workers:** Execute this plan inline in the Combat & Classes workstream.
> Repository instructions prohibit branching or committing unless the human asks.

**Goal:** Deliver the two Combat-owned pure-data artifacts that let Base & Raid Lab build
a deterministic headless raid resolver without fixing final classes or tuning.

**Architecture:** A normative Markdown contract owns field meaning, resolution order,
policies, diagnostics, and evidence versioning. One JSON catalog owns every provisional
value, profile, policy, arena, and expected outcome. Base derives tactical graphs from
verified navigation/visibility data and owns validation plus execution.

**Tech Stack:** Markdown, JSON, PowerShell validation.

## Global Constraints

- Create only the two approved artifacts under `research/combat/` plus coordination edits.
- No Unreal, canon, or `research/machines.json` changes.
- No separate JSON Schema, resolver, dependency, branch, or commit.
- Keep every numeric tunable in JSON; Markdown defines shapes and formulas only.
- Use stable IDs for every cross-reference and reject dangling references.
- Keep policies snapshot-driven and non-oracular.

---

### Task 1: Normative contract

**Files:**
- Create: `research/combat/headless-combat-contract-v1.md`

**Interfaces:**
- Consumes: the approved BASE-1 design and `runtime.navigation-query` as a derived-input
  substrate.
- Produces: the required JSON shape, deterministic tick semantics, policy meanings,
  result/event contract, terminal precedence, and acceptance rules used by Task 2 and
  the Base-owned resolver.

- [ ] Write the Actor-free, class-free tactical graph schema and stable-ID rules.
- [ ] Specify post-upkeep snapshots, atomic links, simultaneous combat, mask-only agent
  durability, structure integrity, loot/extraction, progress, and terminal precedence.
- [ ] Specify the four declarative policy definitions without predicted outcomes.
- [ ] Specify version projection, canonical replay equality, diagnostics, and exclusions.
- [ ] Verify Markdown contains no numeric tuning values and cites the governing canon and
  exchange packet.

### Task 2: Fixture catalog

**Files:**
- Create: `research/combat/headless-combat-fixtures-v1.json`

**Interfaces:**
- Consumes: field names and semantics from Task 1.
- Produces: one baseline traversal envelope, provisional attacker/defender profiles, four
  policies, open/fortified/sealed arenas, and exact expected 4/4, 1/4, 0/4 matrices.

- [ ] Add top-level contract/fixture identity and all provisional run tunables.
- [ ] Add stable-ID traversal, capability, attack-mode, and policy records.
- [ ] Add explicit-node/link/blocker arenas; sealed exposes no legal movement, breach, or
  combat action.
- [ ] Add all twelve expected policy-pair outcomes with causal requirements.
- [ ] Parse the JSON and validate counts, versions, unique IDs, references, matrices, and
  sealed actionlessness with PowerShell.

### Task 3: Exchange closeout

**Files:**
- Modify: `.agents/exchanges/BASE-1-headless-combat-contract.md`
- Modify: `.agents/workstreams/combat.md`

**Interfaces:**
- Consumes: verified Task 1 and Task 2 artifacts.
- Produces: an `answered` BASE-1 response with evidence links and a Combat status that
  truthfully waits for Base resolution/integration.

- [ ] Replace the in-progress Response with the delivered contract, fixture, acceptance
  matrix, magic-machine fit, and evidence links; set `Status: answered`.
- [ ] Update Combat status with the delivered evidence and answered exchange state.
- [ ] Run JSON validation, `research/Test-SharedAgentDocs.ps1`, `git diff --check`, and a
  scoped diff review before notifying Base.
