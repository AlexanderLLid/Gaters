---
name: explore
description: Work a design question end to end - grounded options, a relentless one-question-at-a-time interview, then record the call and fold it in. Use when the user wants to explore, settle, or grill a question from questions.md or any lore/systems hole, asks for options on a design problem, or says "grill me", "poke holes", "does this hold up".
argument-hint: "question #N or topic"
---

# Explore

The one session loop for moving a design question from open to decided. Greybox stays
active throughout.

## 1. Frame

- Pin the question: a `questions.md` #N, or state the topic as one sharp question.
- Read what it touches: the owning sections of `world.md` / `systems.md` (and
  `gate-physics.md` if physics-adjacent), their `Why / rejected` blocks, and the
  question's entry. Don't re-litigate what's already decided — build on it, or surface
  the conflict explicitly and ask.

## 2. Stress-test (find the real hole)

Grant the world's premises; attack the consequences. A hole is: **broken** (X can't
follow from the premises), **unreckoned** (an obvious implication never addressed — the
most valuable; chase second-order), **premise vs premise**, or a **hand-wave** (one line
of lore would close it). Never flag a premise for being impossible — that's the fiction's
price of admission. Reasoning holes only, not string-level nits. If the area holds up,
say so — never invent holes to fill a quota.

## 3. Options (grounded, two or three, cheapest first)

- **Find the lever first.** Ask which buildable knob is at stake — reachability (which
  worlds are live / when a server is provisioned), world border (field radius), instancing
  (empty row / solo tick / contested instance), cost/scarcity (cores, tithe) — or a new
  lever devs can clearly build. Fiction with no lever is colour; cut it.
- Produce **two or three genuinely distinct paths** — distinct in _mechanism_, not
  flavour. Order cheapest first (least canon and tech disturbed).
- **Second bar — balance.** Test each path against the design traps and archetypes
  (`systems.md`): turtle equilibrium, trade-suicide, offline-dodge, zerg pile-on,
  whale-farming, doorstep kill-box. A path may cut the _cost to reach_ a target, never
  the _target's defenses_. A path that reopens a trap isn't disqualified — say so, and
  name the knob that keeps it closed.
- **Inheritance:** a path minting a new knob must name why an existing upstream knob
  cannot carry it.
- Shape, per path: **the fiction → the lever it licenses → its trade-off → the trap it
  avoids or risks.** Run a **consequence pass** for each path. Under **If chosen:** list
  the concrete downstream changes to player experience, existing mechanics/levers, and
  canon; include each affected area once. The path is complete when every affected
  existing call is accounted for. Then **always recommend** one, with the one-line why,
  and a short concrete walkthrough of the recommendation in motion.

## 4. Grill

Interview the human **one question at a time**, walking down the decision tree in
dependency order; give your recommended answer with each question. If a question can be
answered by reading the docs, read the docs instead. Stress-test answers against existing
canon as you go. Don't invent canon to fill silence — ask.

## 5. Record

When the call crystallises:

- Fold it into the owning section of `world.md` / `systems.md`: the model, plus a
  `### Why / rejected` block (the call, the why, what was rejected) — only if it's hard
  to reverse, surprising without context, and a real trade-off. Tag `[current call]`.
- Delete the settled item from `questions.md` (IDs are never reused); add any new
  questions the decision exposed as new #N entries.
- Update every touched section in place — no history narration.
- If the session ends undecided: park it as a `questions.md` entry recording the paths
  and blockers (large studies → a file in `raw/`, pointed to from the entry).

Sharpen fuzzy terms as you go — one canonical word per concept, used everywhere.
