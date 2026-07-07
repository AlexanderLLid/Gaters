# Gaters Prototype Plan

A practical first-pass plan for testing the unique Gaters loop in Unreal. This plan is
for greybox work only: prove the Gate-native raid wager before investing in polish,
full survival systems, markets, space, taming depth, or MMO infrastructure.

## Design target

Build a small playable slice that answers one question:

> Can a Gate create a fun, readable, repeatable raid wager?

The first prototype should let a player stand at a home Gate, inspect imperfect lane
readings, commit to one lane, arrive at a generated or hand-authored Gate site, breach
an AI/abandoned base, fight basic defenders, take loot, and extract before simple run
pressure forces retreat.

## What this is not

Do not spend the first Unreal tests on:

- Full weapons progression.
- Full crafting or gathering.
- Full player base building.
- Real PvP networking.
- Markets, trade, or economy depth.
- Galaxy-scale routing.
- Space Gates, fleets, or Supergates.
- Deep taming systems.
- Mask, avatar, or body-origin systems.
- Perfect portal visuals.

Those can come later. The first milestone is about the Gate, the wager, the raid clock,
and the breach space.

## Milestone 0 — Unreal comfort pass

Goal: get comfortable moving around Unreal and confirm the project can support a fast
greybox workflow.

Build or test:

- A third-person test pawn or template character.
- Basic camera, movement, jump, sprint, and interact input.
- A simple greybox level with a home platform and a far platform.
- A placeholder Gate mesh: ring, aperture plane, or simple arch.
- A debug UI widget or on-screen text for current test state.

Done when:

- You can press Play, move around, interact with the Gate, and see debug text update.

## Milestone 1 — One Gate, one dial

Goal: make the Gate feel like the central object, not a decoration.

Build or test:

- A Home Gate actor.
- A simple interaction prompt at the Gate.
- A lane selection menu with 2-3 placeholder lane cards.
- A commit action that "dials" the selected lane.
- A transition from home platform to a target test site.

Lane card fields:

- Estimated danger.
- Estimated value.
- Stability / weirdness warning.
- Recall status: normal, uncertain, or locked.

Done when:

- The player can choose a lane from the Gate and arrive at a target site.
- The selected lane's debug data follows the player into the target site.

## Milestone 2 — Gate dome and arrival beachhead

Goal: prove the raid entrance shape.

Build or test:

- A visible arrival dome around the target Gate.
- A no-build / no-obstacle clearance radius, even if only debug-drawn.
- A safe arrival zone that prevents instant spawn camping.
- A rule that the dome protects transit but not occupation.
- A few defender firing positions outside or beyond the dome.

Questions to answer:

- Does arrival feel readable?
- Does the player understand where safety ends?
- Is the dome a fair beachhead without becoming a safe room?
- Can defenders threaten the player without spawn-cheesing them?

Done when:

- The player arrives safely, exits the dome, and immediately understands the approach
  direction and danger direction.

## Milestone 3 — Simple run pressure

Goal: make hostile territory create greed and retreat decisions without committing to
the mask/avatar system.

Build or test:

- A placeholder run-pressure value shown on screen.
- Pressure drains during hostile-site time.
- Pressure can drain faster when taking damage, carrying heavy loot, entering danger
  zones, or staying too far from the arrival Gate.
- Pressure stops draining or resets after successful extraction.
- At zero before extraction, the player fails the run.
- The implementation should be a generic variable, not a mask resource, avatar health
  model, or body-system hook.

Initial tuning can be fake. Use obvious values first so the pressure is visible.

Questions to answer:

- Does simple run pressure create greed and retreat decisions?
- Which pressure source feels clearest: timer, food/water, danger meter, Gate
  instability, carrying load, or damage penalty?
- Does the player understand why they are being pushed to leave?

Done when:

- The player can feel a meaningful tradeoff between fighting, looting, and extracting.
- The pressure source can be swapped later without changing the Gate, lane, base, or
  extraction work.

## Milestone 4 — AI/abandoned base greybox

Goal: create the first target that trains the raid loop without needing real players.

Build or test:

- One modular AI/abandoned base layout near the target Gate.
- One clear attack lane from Gate dome to base.
- One alternate or risky route if time allows.
- Basic doors, cover, or breach points.
- One loot room or objective room.
- Basic AI defenders using simple patrol, guard, or shooting behavior.

Validation checks:

- The base must not block the Gate dome.
- Loot must be reachable.
- Defenders must not spawn trapped.
- There must be at least one valid attack lane.
- The object count must stay cheap enough for rapid iteration.

Done when:

- The player can approach, breach, fight, loot, and leave.

## Milestone 5 — Extraction and recall

Goal: make the end of a run as important as the entry.

Build or test:

- An extraction interaction at the arrival Gate or a recall action.
- Loot is only secured after extraction.
- A short recall/extraction channel.
- A failure state if run pressure reaches zero before extraction.
- A successful run summary: lane chosen, loot taken, time remaining, scan accuracy.

Questions to answer:

- Does the player push too far and regret it?
- Does extraction feel like a decision rather than a menu command?
- Is the best moment often "one more room or leave now"?

Done when:

- The player can win, lose, or retreat with partial success.

## Milestone 6 — Sensor truth and bad reads

Goal: turn lane selection into a wager.

Build or test:

- Lane card values are not always true.
- A hidden truth table for each lane.
- At least four lane modifiers:
  - `normal`
  - `bad_read`
  - `locked_recall`
  - `rich_signature`
- A post-run summary comparing scan estimate to actual site truth.

Rules:

- Most bad outcomes should be hinted before commitment.
- A pure bad read can lie, but should be uncommon.
- The player should feel they accepted risk, not that the game cheated.

Done when:

- The player can say, "I picked that because the scan looked worth it," and then learn
  whether the read was reliable.

## Milestone 7 — Repeatable seeds

Goal: make the loop replayable enough for design testing.

Build or test:

- A "new lane" or "new seed" debug button.
- Several hand-authored layout chunks or a simple procedural placement pass.
- Save/reload of selected lane data and site state if practical.
- A debug validation report for each generated site.

Validation report fields:

- Gate dome clear: pass/fail.
- Loot reachable: pass/fail.
- Defenders pathable: pass/fail.
- Attack lane present: pass/fail.
- Extraction reachable: pass/fail.
- Estimated object count.

Done when:

- You can run several attempts in a row and quickly identify whether failures are design,
  tuning, layout, AI, or readability problems.

## Suggested first Unreal session

Start smaller than the full loop. For the first session, do only this:

1. Open a third-person Unreal project.
2. Make a blank greybox map.
3. Add a simple Gate actor shape.
4. Add an interact prompt.
5. On interact, show three lane cards as debug text or a simple widget.
6. Pick one card and teleport the player to a second platform.
7. Display the selected lane's danger/value/stability text after arrival.

If that works, the next session should add the dome and the generic run-pressure
placeholder.

## Current priority stack

1. Gate interaction and lane selection.
2. Arrival dome / beachhead.
3. Generic run-pressure placeholder.
4. AI/abandoned base greybox.
5. Extraction / recall.
6. Sensor inaccuracy and lane twists.
7. Repeatable seeds and validation reports.
8. Player bases, real PvP, economy, and larger network systems later.
