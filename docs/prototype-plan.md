# Gaters Prototype Plan

Living greybox checklist for the current Unreal slice. This is not canon; update it in
place when tests teach us something. No history log, no report, no polish plan.

## Current proof

Can a Gate create a fun, readable, repeatable raid wager?

Build only the slice needed to answer that:

- Stand at a home Gate.
- Read 2-3 imperfect lane offers.
- Commit to one lane.
- Arrive at a Gate-adjacent target site.
- Breach one AI/abandoned base.
- Fight simple defenders.
- Take loot.
- Extract before run pressure forces retreat.
- Retry with a new lane/seed.

## Focus first - Gaters-new

These are the things the prototype must actually learn.

- **Gate lane offer** - danger, value, stability/weirdness, and recall status before
  commitment.
- **Dial / commit** - choosing a lane matters; the selected lane's hidden truth follows
  the player into the target site.
- **Arrival / breach sanity** - safe entry and a readable first push; final dome rules can
  stay placeholder.
- **Run pressure placeholder** - a simple timer/resource that makes hostile territory
  create greed and retreat decisions.
- **Raid/base variety** - many approach shapes, base layouts, defender setups, loot
  placements, and lane twists; the prototype should learn what stays fun.
- **Exploit resistance** - reject sites with spawn farming, blocked exits, trapped
  defenders, unreachable loot, or no-risk loot skips.
- **Extraction / recall** - loot is secured only after extraction; retreat with partial
  success is valid.
- **Bad reads / lane twists** - `normal`, `bad_read`, `locked_recall`, and
  `rich_signature` prove the wager.
- **Repeatable validation** - new lane/seed button plus a pass/fail report for the raid
  site.

## Borrow or stub

Other games already prove these can work. Use the simplest version until one blocks the
Gate proof.

- Movement, camera, sprint, jump, interact.
- Basic weapons, damage, health, and death.
- Basic AI patrol/guard/shoot behavior.
- Simple doors, cover, breach points, and loot containers.
- Debug text or plain UI cards.
- Greybox terrain, platforms, and modular base pieces.
- Inventory as simple variables.
- Save/load only for selected lane data and site state.
- Combat balance, progression, crafting, economy, and base-building depth.

## Defer

Do not spend early prototype time here.

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
- Full MMO hosting or instance orchestration.
- Population proof: whether players open up instead of turtling needs a scaled playtest,
  not this greybox.

## Priority stack

1. Gate interaction and lane selection.
2. Arrival / breach sanity.
3. Generic run-pressure placeholder.
4. Varied AI/abandoned base greybox.
5. Extraction / recall.
6. Sensor inaccuracy and lane twists.
7. Repeatable seeds, variety tags, and exploit reports.
8. Player bases, real PvP, economy, and larger network systems later.

## Validation report

Each generated or hand-authored raid site should answer:

- Arrival sane: pass/fail. The player can arrive, orient, and leave into the raid; exact
  dome rules can stay placeholder.
- Variety tags recorded: approach shape, base shape, defender setup, loot placement, and
  lane twist.
- Exploit scan: pass/fail. No blocked spawn/exit, defender firing directly into spawn,
  unreachable loot, trapped defenders, no-risk route to loot, or extraction skip.
- Object count cheap enough for iteration: pass/fail.

## Current first session

Start smaller than the whole loop:

1. Open a third-person Unreal project.
2. Make a blank greybox map.
3. Add a simple Gate actor shape.
4. Add an interact prompt.
5. On interact, show three lane cards as debug text or a simple widget.
6. Pick one card and teleport the player to a second platform.
7. Display the selected lane's danger/value/stability text after arrival.

Next after that: add the arrival/breach placeholder and the generic run-pressure
placeholder.
