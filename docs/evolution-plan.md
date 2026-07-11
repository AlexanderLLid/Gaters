# Evolution Plan — bases vs raider AI

Living greybox checklist for co-evolving raidable base generation and raider AI in the
Unreal slice. Sibling of [[prototype-plan]] (the raid loop), [[worldgen-plan]] (the
ground), and [[unreal-progress]] (what exists). Not canon; update in place.

The idea: two populations trained against each other. **Base genomes** generate raid
sites; **raider genomes** drive an AI that raids them. Each generation, bases are scored
on producing good fights and raiders on winning them — both improve at once. The player
can watch AI-vs-base bouts or raid the evolved bases directly.

## The filter [current call]

Same as [[worldgen-plan]]: only feasibility unknowns get built. The unknown here is
**whether coevolution produces bases that are hard-but-raidable instead of degenerate**
— not pathfinding, not shooting, not genetic-algorithm bookkeeping (all solved).

**Method call [current call]: parameter evolution over utility AI, not neural RL.**
Genomes are small structs of tunable weights; evolution is mutate → fight → select.
No Learning Agents / ML-Agents, no Python training loop, no C++ (toolchain is blocked
anyway, [[unreal-progress]]). Upgrade path: if evolved utility raiders plateau at
"predictable to fight", that is the data that justifies real RL — not before.

## What this must deliver

- A raid bout that runs **unattended**: AI raider spawns at the Gate pad, tries to reach
  loot and extract, defenders resist, bout ends in a scored outcome.
- A **base genome**: the generation parameters lifted into an explicit, mutable struct
  (archetype, layout pieces, entrances, defender posts, loot depth) — separated from the
  terrain seed, which stays [[worldgen-plan]]'s pure function.
- A **raider genome**: utility weights on one scripted raider (route directness, breach
  vs entrance preference, target priority, retreat threshold).
- An **evolution manager** that runs generations of bouts and selects both populations.
- A **player mode**: raid the current champion bases through the normal raid loop.

## Fitness [current call]

- **Raider fitness** — loot extracted per bout time; dying scores the partial.
- **Base fitness** — peaks when raider success sits in a target band (raidable but
  contested), not when raiders always lose. Pure adversarial fitness is the known trap:
  it evolves walled-off loot and suicide-rush raiders.
- **Hard constraints, not fitness:** a base that fails the existing exploit scan
  (unreachable loot, blocked spawn, trapped defenders — [[prototype-plan#Validation report]])
  is invalid and removed, never scored. The validation report is the genome killer.
- Tunables: success-rate band, population sizes, bouts per genome (AI is noisy — repeat
  and average), mutation rate. Values land in data, not here.

## Feasibility unknowns

- **E1 — unattended bout.** A scripted raider can complete/fail a raid on a generated
  site with zero human input and emit one scalar-ish result line.
- **E2 — throughput.** Bouts per minute in PIE (time dilation, parallel arenas, culled
  visuals). If a generation takes hours, the loop is dead; measure before scaling.
- **E3 — non-degenerate coevolution.** Over generations, base difficulty moves and stays
  inside the valid band; no invalid genome survives; raiders don't collapse to one trick.
- **E4 — cycling.** Coevolution can loop (each side re-beats the last) instead of
  progressing. Mitigation to test: score against a small hall of fame of past champions,
  not only the current generation.
- **E5 — fun transfer.** A champion base is more interesting for a _human_ to raid than
  a random seed. Machine-hard and player-fun are not the same thing; this is the payoff
  check.

## Borrow or stub (never prototype)

- Pathfinding and movement: navmesh + MoveTo. Defender behavior: UE perception + simple
  state logic. Both industry-solved.
- GA mechanics (selection, crossover, mutation): textbook; a BP manager actor with
  arrays does it.
- Combat feel, weapon variety, animations — bouts run on stub combat (hitscan, health).
- Requires per-piece structures eventually (bases are one merged mesh today,
  [[unreal-progress]]) — stub breach as "entrances only" until the building-system
  borrow lands.

## Defer

- Neural RL (Learning Agents) — until utility raiders provably plateau.
- Learning from human player raids as a fitness signal.
- Evolving terrain (ground stays a pure seed function, [[worldgen-plan]]).
- Multi-raider squads, defender reinforcement waves, economy-priced defenses.
- Offline/headless farm — PIE throughput first; only if E2 fails.

## Order to develop — and the check that gates each step

1. [ ] Navmesh + raider bot on the existing generated site: spawn, route to loot,
       extract. — _Check: bot completes a known-good seed unattended; `BOUT` line prints
       outcome + time._
2. [ ] Defender cones become pawns that damage the raider; raider can die and retreat.
       — _Check: outcomes differ across seeds; both win and loss occur._
3. [ ] Bout runner: N unattended bouts across seeds in one PIE session. — _Check:
       bouts/min measured (E2); results table prints._
4. [ ] Genomes: base params as struct feeding the generator, raider weights as struct
       feeding the bot. — _Check: same genome pair → same setup; repeated bouts give a
       stable average._
5. [ ] Evolution manager: one generation = fights, scores, select, mutate, both sides.
       — _Check: E3 over a multi-generation run; `GEN` report per generation._
6. [ ] Hall of fame + champion mode: player dials into a champion base via the normal
       lane flow. — _Check: E4 (no cycling) and E5 (feels different from a random seed —
       subjective, note it)._

## Validation report

Extends the per-seed print: `BOUT result= time= loot= dmg=` per bout;
`GEN n= raider_win%= base_valid%= best_fitness=` per generation. A healthy run keeps
`raider_win%` drifting toward the band and `base_valid%` high.

### Why / rejected

- **Why parameter evolution first:** proves the coevolution question with structs and
  arrays; RL needs a toolchain this machine doesn't have and infra the question doesn't.
- **Why a target band, not pure adversarial fitness:** the game needs raidable-but-
  defended, and unraidable is the natural attractor of a pure arms race. The band plus
  the exploit-scan kill rule aim evolution at the game's goal, not the defender's.
- **Why bouts in-engine, not an abstract sim:** base quality is spatial — approach
  lanes, sightlines, entrance choke. A graph-based sim would optimize the wrong thing.
- **Rejected — evolving the terrain too:** ground is a pure seed function by decision
  ([[worldgen-plan]]); evolution only touches what stamps onto it.
