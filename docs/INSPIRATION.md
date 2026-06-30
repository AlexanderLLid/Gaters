# INSPIRATION.md - Design Reference

The north star for Gaters. This is a design compass, not canon — nothing here
enters the lore or systems wikis.

## Sources

- **Stargate SG-1** → expedition-through-a-portal structure; the "team dials out,
  explores a hostile world, comes home" episode shape; a network of addressable
  gates as the connective tissue of the setting.
- **Valheim** → the gather → craft → build → boss survival loop; biome-gated
  progression where each tier unlocks the tools to survive the next.
- **Solo Leveling** → the underdog power-growth arc; escalating dungeon/gate
  threats; rising from weak to overwhelming as the core fantasy.
- **ARK** → tame-and-ride creatures as labour, mounts, and war machines; building
  a base that doubles as a creature pen; the dinosaur-as-tech-tier loop.
- **Rust** → high-stakes open PvP where your base is only as safe as its walls;
  the raid/defend tension and the social meta of alliances and betrayal.
- **EVE Online** → player-driven economy and politics at scale; territory, blocs,
  and logistics; loss is real (what you fly you can lose), so risk is the content.

## Styles

- **Palworld** → the approachable, readable creature-and-character art direction;
  bright survival-craft visuals that stay legible in combat and building.
- **Satisfactory** → clean industrial building clarity; readable factory/structure
  systems where complex bases stay visually parseable.

## Visual direction (deferred)

Art is **deferred until after greybox validation** [decided]; final identity is open
(open-questions #6). Captured here so it isn't re-litigated.

- **Touchstones:** Minecraft, Valheim, No Man's Sky. **NMS is the aspirational visual
  target**; the Gate structure deliberately sidesteps NMS's hardest engineering problem
  (seamless space-to-surface flight, replaced by Gate loads). **Valheim** is the
  achievable Unity aesthetic.
- **Principle [decided]:** separate **visual style** (cheap, achievable solo) from the
  **engine tech** of reference games (NMS's custom engine + seamless flight, not
  achievable solo).
- **Leaning path to the NMS look solo:** low-poly, flat-shaded 3D with strong
  post-processing.
- **Open:** final identity (NMS-lush vs. low-poly-stylized), palette, UI style.

## Feel

- **Accidental discovery is core.** The best moments are unscripted — stumbling
  onto a frontier world you weren't meant to find, an emergent interaction
  no quest pointed you at, a mechanic that combines in ways the designers didn't
  spell out. Reward curiosity and experimentation over following a marker. Design
  for surprise, not a guided tour.
- **Endgame = EVE × Solo Leveling × ARK.** A player-driven world of blocs, territory,
  and real loss (EVE) wearing the underdog punch-up power fantasy (Solo Leveling),
  built on the tame-and-build survival loop (ARK): you raid _up_, the strong are the
  prize, and nobody babysits a base — _except_ a huge tribe, whose reach keeps it
  permanently exposed. This is the "no defender tax" thesis taken to its endgame, not
  a new rule (see `pillars.md`, [[World Overview]]).

## Wants — no home yet

Things I want in, but don't know how they fit the lore or systems yet.

- **Red worlds (Solo Leveling).** A contested frontier world can "go red" — it locks,
  and you can't leave until some condition/quest inside is cleared. A high-stakes mode
  where entering means committing. No idea how this squares with the frontier model yet.

### Contested-frontier-world variants — idea bank

Each is one knob turned on the contested frontier world (contest window, visibility,
extraction). Pure inspo, nothing decided.

- **Nested world** — a second contested world reachable only from inside the first;
  going deeper strands you another layer from home. A natural vehicle for the red-world lock.
- **Beacon world** — entering lights you up for a loot premium. A 10-min
  countdown starts and 3 tribes near your Potential get an opt-in prompt to "lock
  on" — accept and they get your coordinates and a window to hit you. Consensual
  PvP both ways, self-balanced by Potential, a defend-the-clock event with a known
  threat count. Open: do the 3 see each other; what do attackers risk to lock in;
  rewards for surviving vs. breaching.
- **Fast-closing world** — abnormally short contest window; smash-and-grab, no clean
  extract.
- **No-recall world** — consciousness-link jammed inside; extraction is the only
  way out.
- **Inverted world** — visible only to players _above_ the opener's Potential; the
  underdog baits the strong.
- **Blind world** — rank/contents not telegraphed; gamble on what's behind it.
- **Coordinate world** — guaranteed coordinate-shares inside; a Coalition-seeded
  home-hunt event.
- **Recurring world** — the same world each time; becomes known and contested,
  a soft persistent zone without the hosting cost.
- **Anchored world** — doesn't close on schedule; a contested world mid-tame, precursor to
  a new Gate.

## Counter — overused, don't ship

Tropes that read as derivative the moment a player recognizes them. Avoid even
if reskinned.

- **Ancient precursors seeded all life.** A long-vanished elder race created or
  DNA-seeded everyone, which is why every species shares a body plan / is secretly
  related / converges on one origin. Done to death: Star Trek (the Progenitors,
  "The Chase" → _Discovery_ S5), Assassin's Creed (the Isu), Stargate (the
  Ancients), Halo (the Forerunners), Mass Effect (Protheans/Reapers), Prometheus
  (the Engineers), Babylon 5 (the First Ones), 2001 (the Monolith makers).
  - Includes the variants: **recursive precursors** (turtles all the way down)
    and **we are our own ancestors / eternal cycle** (Battlestar Galactica's Kobol
    → Earth, "all this has happened before").
  - Why avoid: it's the standard answer to "why is the galaxy full of humanoids."
    Our humanoids come through **Gates** — the question doesn't need a creation
    myth, and answering it this way invites the comparison. If origin ever matters,
    make it about the Gates, not a benevolent dead god.
- **Theme-park gates.** Curated, on-rails attractions — each gate a hand-built ride
  with a scripted encounter and a guaranteed payout. Kills accidental discovery
  (see Feel) and turns the network into a content menu. Gates are destinations
  players exploit and contest, not exhibits we stage.

## The rule

Take the patterns above, never the proper nouns.

No named characters, coined terms, items, factions, or art from these works
enters canon. Borrow the skeleton, invent the flesh — Gaters dial their **own
gate network**, not Stargate's; grow strong by our own named means, not "the System."
If a name or phrase would make someone say "that's straight from X," it doesn't
ship.
