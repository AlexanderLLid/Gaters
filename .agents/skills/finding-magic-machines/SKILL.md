---
name: finding-magic-machines
description: Use when "magic machine" thinking, recursive enablers, capability leverage, or a snap-your-fingers system is requested, or when repeated work may be eliminated by an engine, platform, framework, plugin, service, or new tool.
---

# Finding Magic Machines

## Core principle

A **magic machine** removes an entire class of work and unlocks a capability. An
optimization performs the same work faster. Build a **Capability Graph** backward from
the dream, reuse existing machinery, then build forward from falsifiable gaps.

## Graph vocabulary

- **AND:** every incoming machine is required.
- **OR:** any incoming machine can satisfy the contract; choose one.
- **SEQUENCE:** one machine's output feeds the next.
- **Borrow:** existing machinery meets the guarantee.
- **Adapt:** existing machinery needs a thin contract adapter.
- **Build:** the required guarantee is missing.

## Recursive loop

1. **Frame the outcome.** State the observable result without naming a solution. Done
   when evidence could distinguish success from failure.
2. **Inventory machinery.** Inspect the project, engine/platform, plugins, dependencies,
   and services. For Unreal, inspect native subsystems/plugins first; verify uncertain
   versions in primary documentation.
   Done when each plausible machine is checked against the contract and constraints.
3. **Find the tax.** Name the work or uncertainty that most changes feasibility. Done
   when one work class is
   precise enough to delete.
4. **Specify candidates.** Offer two or three mechanisms. Name each input, output,
   guarantee, work deleted, exposed limitation, and Borrow/Adapt/Build source. Done when
   mechanisms differ rather than merely scale.
5. **Choose by deletion.** Prefer the largest load-bearing deletion with a testable
   contract. Complete literally: **"If this existed, we would no longer need to ___."**
6. **Expand prerequisites.** Ask which machines make the chosen machine possible. Mark
   every dependency AND, OR, or SEQUENCE; retain all AND branches, choose an OR branch,
   and order SEQUENCE branches. Done when every edge is causal, not a renamed parent.
7. **Recurse unresolved leaves.** Descend Adapt and Build leaves. Done when each branch
   ends at a verified Borrow node or an isolated prototype that can falsify its guarantee.
8. **Reverse the graph.** Present topological build waves: Borrow, then Adapt, then Build.
   Done when evidence gates every node before it unlocks consumers.
9. **Select the next frontier.** Compare unresolved nodes by the nearest player-visible
   milestone they block, downstream machines unlocked, feasibility uncertainty retired,
   and earlier-wave blockers skipped. A later-wave known-known proceeds only when the
   human prioritizes it or evidence shows the spike could invalidate the architecture.
   Done when the selected node records **why now**, what it unlocks, and why every nearer
   blocker can wait.
10. **Close the verification loop.** For every Adapt or Build node, name an
   independent verifier, held-out challenge set, diagnostic failure artifact, and
   champion-challenger promotion gate. Preserve every run immutably. Done when a
   failed guarantee identifies the next experiment without replacing the champion.
   Spatial comparison evidence preserves each source independently at full opacity;
   use a wipe or blink as primary evidence and keep alpha-averaged composites diagnostic.
11. **Loop from evidence.** Restart at the first failed guarantee after each prototype.
   Done when the next seed machine addresses the observed failure.

## Output contract

| Node | Unlocks | AND/OR/SEQUENCE | Borrow/Adapt/Build | Contract | Work deleted | Verifier | Challenge set | Promotion gate |
|---|---|---|---|---|---|---|

End with seed nodes, falsifying experiments, first build wave, selected frontier with
its **why now**, and unlocked dream machine. Enter implementation through the authorized
planning workflow.

## Example

`Autonomous world researcher <- AND(world generator, semantic compiler, evaluator).`
The compiler may use `Borrow: Unreal Geometry/PCG`, `Adapt: semantic contacts`, and
`Build: deterministic fit evaluator`.

## Failure checks

- **Optimization:** work remains, only faster.
- **Oracle:** no contract or falsifiable guarantee.
- **Self-judge:** the machine scores itself with the assumptions that produced its output.
- **Reinvention:** a matching existing machine is ignored.
- **Synonym edge:** a dependency renames its parent.
- **Catalogue:** nodes lack typed causal edges.
- **Infinite graph:** descent passes a leaf already able to produce evidence.
