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

## Visual direction

- **[open] Final visual direction:** realism level, proportion language, shading, surface
  finish, and detail density remain undecided. Candidates may range from grounded to
  strongly stylized, but must satisfy `ART-1` recognizability and remain replaceable.
- **Performance shape:** ordinary Unreal lighting and scalable density/shadows are the
  baseline; expensive rendering features remain optional quality settings.
- **References:** visual studies under `research/character-style-exploration/` are
  comparison evidence, not a selected style contract.
- **Open:** style, palette, UI language, and production quality bar.

## Feel

- **Accidental discovery is core.** The best moments are unscripted — stumbling
  through a misdial into something you weren't meant to find, an emergent interaction
  no quest pointed you at, a mechanic that combines in ways the designers didn't
  spell out. Reward curiosity and experimentation over following a marker. Design
  for surprise, not a guided tour.
- **Endgame = EVE × Solo Leveling × ARK.** A player-driven world of blocs, territory,
  and real loss (EVE) wearing the underdog punch-up power fantasy (Solo Leveling),
  built on the tame-and-build survival loop (ARK): you raid _up_, the strong are the
  prize, and nobody babysits a base — _except_ a huge house, whose reach keeps it
  permanently exposed. This is the "no defender tax" thesis taken to its endgame, not
  a new rule (see [[systems]], [[world]]).

## Wants — no home yet

Things I want in, but don't know how they fit the lore or systems yet. All of it is
**Gate technology** — every variant below is one knob turned on the existing model
(dial-out / misdial, the away reserve, Potential visibility, listings, extraction).
Pure inspo, nothing decided.

- **Gate sensors (the enabler).** Tech that senses _through_ a throat before you
  commit — world rank, hostiles, AI/abandoned-base tier, loot signature. Blind dialing is
  the default; sensor tiers turn gambles into informed raids. This one knob
  (telegraphed vs. blind) makes most variants below fair, and higher-tier PvE
  worlds readable. (cf. **Recon probes** in the option bank, [[questions]].)
- **Locked lane (Solo Leveling's red gate).** A lane that jams recall and re-dial
  until a condition inside is cleared — entering means committing. High-stakes
  opt-in mode built on the away reserve.
- **Beacon dial.** Deliberately overdrive/light your Gate for a loot premium: K
  houses near your Potential get your coordinate and a bounded window. Consensual
  PvP both ways, a defend-the-clock event. (Half-lives in FRONTIER-3 events
  already.)
- **Nested misdial.** The far world's gate misdials deeper mid-run — another layer
  from home, stacking away-reserve pressure. Natural delivery vehicle for the lock.
- **Fast-collapse lane** — abnormally short window; smash-and-grab, no clean extract.
- **Blind lane** — no sensor reading possible; rank/contents a pure gamble.
- **Inverted visibility** — a push visible only to players _above_ the opener's
  Potential; the underdog baits the strong.
- **Share-cache world** — guaranteed coordinate-shares inside; a Coalition-seeded
  home-hunt event.
- **Recurring world** — the same world re-listed on a cycle; becomes known and
  contested, a soft persistent zone without the hosting cost.
- **Anchored lane** — a lane that refuses to collapse; the precursor to claiming a
  new Gate (ties to the imprint pipeline).

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
enters canon. Borrow the skeleton, invent the flesh — Gaters dial
**Gates** of our own expression, not Stargates; grow strong by our own named means, not "the System."
If a name or phrase would make someone say "that's straight from X," it doesn't
ship.
