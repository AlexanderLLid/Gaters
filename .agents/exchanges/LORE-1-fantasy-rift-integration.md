# LORE-1 — Fantasy Rift integration

Status: resolved
From: Lore
To: Primary Builder — World & Terrain
Type: INTEGRATE
Notification: sent

## Request

Replace the permanent science-fiction Gate premise in the current canon with the
smallest approved fantasy Rift model. Integrate in place across `docs/world.md`,
`docs/systems.md`, `docs/gate-physics.md`, `docs/questions.md`, and
`docs/INSPIRATION.md`; do not change terrain-generation implementation.

Settled calls:

- The game is fantasy: **Valheim + Solo Leveling + ARK**, with no real science-fiction
  layer and no later "magic was technology" reveal.
- Rifts are temporary magical openings between worlds; some form naturally. Weather is a
  tentative metaphor for their unpredictability, not settled lore.
- Trained people can cast a spell that calls or opens a Rift.
- Anchors are unexplained ancient magical stones or ruins. They attract/stabilize Rifts;
  a world may contain several.
- Claiming establishes ownership, building, and respawn. It does not privatize a world,
  disable Anchors, or change Rift behaviour.
- Multiple Anchors are intentional invasion surfaces: another Anchor can become the way
  in, so fortifying one permanent doorway cannot solve raids.
- Preserve the load-bearing calls: reach equals exposure, obscurity rather than immunity,
  no defender tax, bounded engagements, uphill/peer protection, ordinary activity traces,
  guild-as-owner, disposable bodies/respawn, building, taming, and bounded generated worlds.

Record as `[open]`, not canon:

- exact Rift targeting: blind casting, trace requirements, target-world control, and
  how much influence a caster has over the arrival Anchor;
- natural Rift frequency and whether a blind Rift can reach a claimed world;
- exact casting cost/resource and whether the existing power-core role survives as a
  fantasy resource;
- simultaneous hostile Rifts, raid caps, arrival protection, and extraction;
- the ownership authority/faction replacing the Gate Council;
- whether `Gaters`, the player noun, and the `gate-physics.md` filename survive.

Contradictory material to remove rather than preserve as history:

- Builders/precursors, Earth, aliens, the Wake, consciousness experiment, Gate Council,
  wormhole/exotic-matter physics, driven rings, dome-as-Builder-device, fixed Gate
  addresses, Gate tiers/Supergates, orbital/space/fleet progression, and sci-fi combat.

Minimum supporting evidence:

- `docs/questions.md` #27 owns the permanent-Gate/temporary-Rift fork.
- `docs/world.md#Design thesis — "reach equals exposure"` and
  `docs/systems.md#Failure modes` remain the balance constraints.
- The human explicitly approved updating the docs after selecting the pure-fantasy Rift
  direction.

## Response

Integrated in place after the human explicitly resumed the lore-doc update:

- `docs/world.md` now owns the pure-fantasy premise, natural and called Rifts, literal
  spellcasting, magical Anchors, unchanged post-claim behaviour, and the retained
  exposure thesis.
- `docs/systems.md` retains the core loop, failure traps, bounded raids, guild ownership,
  traces, Potential, holdings, taming, building, and bounded-world technical levers while
  leaving targeting, arrival, cost, and replacement institutions open.
- `docs/rift-rules.md` owns consistent magical behaviour rather than real-physics claims.
- `docs/questions.md` removes the resolved Gate/precursor forks and records #28-#31 for
  targeting/arrival, cost, ownership authority, and bounded raid rules.
- `docs/INSPIRATION.md` now points directly at Valheim + Solo Leveling + ARK fantasy and
  rejects a hidden technology reveal.

No terrain-generation, Unreal, raw-source, registry, or unrelated dirty file was changed.

## Resolution

Accepted by Lore after a full reread of the five files.

- `git diff --check -- docs` passes.
- All question IDs are unique.
- All wikifile and heading targets in the five files resolve.
- The stale-term scan finds only intentional negative constraints.
- Unsettled targeting, cost, arrival, concurrency, extraction, naming, death fiction,
  and ownership authority remain explicitly open.

Follow-up integration removed the obsolete portal noun from active guidance and the
prototype's semantic world-recipe contract:

- `AGENTS.md`, the active prototype/worldgen plans, and the mirrored explore/greybox
  skills now point to Rifts, Anchors, and `docs/rift-rules.md`.
- The recipe's neutral origin node is `Arrival` / `arrival:0`, not a permanent portal.
- Generator version `12` invalidates diffs created with the old canonical node identity.
- Terrain-generation behaviour is unchanged.
