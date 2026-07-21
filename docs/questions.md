# Questions

The living backlog for cross-cutting forks, parked explorations, and deferred wants.
System-local open details live beside their mechanics. IDs are stable and never reused.

## Open questions

- **#5 Return after death** — what returns, where the new body comes from, what body-bound
  progression is lost, and why a claimed Anchor can restore the player. Until settled,
  canon states only that bodies die, carried gear drops, and the player continues.
- **#17 Does the title "Gaters" survive?** The central phenomenon is now called a Rift,
  so the title and player noun need to survive phonetics, searchability, harassment, and
  the terminology mismatch.
- **#21 Engine choice — Unity vs. Unreal vs. Godot.** Undecided; comparison lives in
  `raw/engine-comparison.md`. Record the call in [[systems#Validation and technical constraints|technical constraints]].
- **#26 Generator versioning for live worlds.** Live seeds and saved changes must not
  silently mutate when generation changes. The prototype already stamps
  `GatersGenVersion`; full live-ops policy remains deferred.
- **#28 Rift targeting and arrival.** Can a blind spell reach a claimed world? What recent
  evidence biases a Rift? Does the caster choose a world, a region, an Anchor, or only
  improve odds? How does the target get warning without enabling one-door bunkering?
  ([[systems#Rifts|Rifts]], [[rift-rules|Rift Rules]])
- **#29 Rift cost.** What existing fantasy resource pays for calling, sustaining, cargo,
  distance, and stronger bias? Do not create separate resources unless those costs fail
  independently in play.
- **#30 Ownership authority and starting point.** What recognizes a guild's home-Anchor
  claim and local territory, handles permissions, and provides a fallback return point
  without recreating a science-fiction clearinghouse?
- **#31 Bounded raid rules.** Decide simultaneous hostile Rifts, attacker/defender caps,
  arrival protection, cross-world raid-clock and offline timing, plus same-world
  participant caps and structure-damage rules. The result must close zerg, doorstep, and
  offline-dodge failures together.
- **#32 Full shared-world capacity.** Set home-Anchor, resident, and active-player budget
  shapes. When a budget is full, does a claim or new Rift wait, fail, or feed a linked
  front? The answer must preserve SCALE-1, prevent copy-based conflict dodging, and not
  require mass concurrency.
- **#33 Spell recipe language.** Which approved optional traits and composition rules
  extend the shared recipe core and initial action vocabulary? Define verifier outcomes,
  then settle offer rules, graph visibility, and how classes read the resulting spell
  graph.
- **#34 Optional PvE world upkeep.** Should designated PvE worlds require upkeep or use
  automated defenses against persistent pressure? Decide offline simulation and failure
  consequences without changing the default no-upkeep home rule.

## Wanted later

- **Large shared worlds** — add invisible backend regions only if real concurrency proves
  one bounded world insufficient.
- **Sea travel** — ships and water routes inside worlds after the ground loop works.
- **Flying creatures** — late traversal after terrain, combat, and local travel hold up.
- **Deeper taming** — broader creature roles after active taming proves fun without
  maintenance chores.

## Option bank — Rift uses

Options only; integrate when adopted.

- **Blind Rift** — no useful destination evidence; exploration gamble.
- **Locked Rift** — return is blocked until an objective is cleared.
- **Fast collapse** — short smash-and-grab window.
- **Bad read** — danger or value differs from the caster's signs.
- **Nested Rift** — a world exposes another temporary Rift during the run.
- **Known Anchor bias** — better evidence improves arrival odds near a discovered Anchor
  without guaranteeing it.
