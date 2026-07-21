# RAID-4 — Production tactical-role candidate policy

Status: resolved
From: Raids & Dungeons
To: Human
Type: DECISION
Notification: sent

## Request

Choose how the first production evaluator turns neutral physical slots into provisional
arrival, objective, extraction, and guard candidates. This selects evaluation challenges;
it does not label the source recipe or define final runtime encounter placement.

- **A — Representative ensemble (recommended):** deterministically bucket physically
  valid tuples by connectivity, route depth, approach count, visibility exposure, and
  extraction distance, then simulate one stable-ID exemplar per distinct bucket. This
  exposes easy, impossible, unfair, and repetitive regions without manual labeling.
- **B — Single canonical tuple:** select one highest-scoring tuple. It is cheapest, but
  the held-out settlement proves that shallow and deep selections produce opposite
  outcomes, making one tuple an oracle.
- **C — Exhaustive tuples:** simulate every valid tuple. It avoids sampling policy, but
  the held-out settlement already produces thousands of combinations before guard
  candidates and much redundant evidence. A deterministic 20-case benchmark projects
  roughly five minutes for 4,842 base tuples before guard expansion; eight sampled cases
  already produced the same all-mask-expired result signature.

**Should Raids use A, the deterministic representative ensemble, for production site
evaluation while keeping actual encounter placement a separate later policy?**

Evidence:

- [`rift-raid-machine-setup-v1.md`](../../research/raids-dungeons/rift-raid-machine-setup-v1.md#current-frontier)
  records the capability graph, alternatives, falsifying results, and promotion gate.
- The production settlement has 43 movement components; only 78 of 1,080
  outdoor-to-indoor pairs are reachable.
- A one-link provisional tuple extracts under all four policy pairs with trivial/unfair
  findings; a deep tuple fails under all four while the topology-only rating still says
  `pass`.
- A medium tuple separates attacker policies cleanly: `runner` extracts 2/2 while
  `clear` expires 0/2, so representative selection can preserve real policy sensitivity.
- `[[systems#First prototype priority — generated raid|First prototype priority]]`
  already selects AI/abandoned settlements as the first repeatable target, so this does
  not ask whether settlements are raid content.

Requirements checked: Global none; `BUILD-1`, `LOOT-1`, and `RAID-1` through `RAID-8`;
exceptions: none.

## Response

Approved option A: use the deterministic representative ensemble for production site
evaluation while keeping actual encounter placement as a separate later policy.

## Resolution

Raids & Dungeons will derive a stable, physically representative analysis ensemble from
neutral Built Site Recipe evidence. The ensemble may assign provisional arrival,
objective, extraction, and guard roles only inside evaluator-owned copies. It must not
write tactical labels back to the recipe or present sampled tuples as final runtime
encounter placement.

Until the missing world-to-site approach and compatible production scenario evidence
exist, generated-settlement results remain explicitly `analysis-only` and report those
missing inputs as causal findings.
