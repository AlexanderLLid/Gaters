# CLAUDE.md - Gaters

Gaters is a work-in-progress design repo. Nothing is locked: when a design changes, edit
the current docs in place as if it had always been that way. Do not add migration notes,
"supersedes" history, or graveyard prose.

Do not branch, commit, or push unless asked. This repo holds design docs only; the game
project will be a sibling folder later. `docs/` is an Obsidian vault.

CLAUDE.md holds collaboration rules, editing rules, and stable design discipline only. Do
not store changing game requirements, concrete mechanics, balance calls, feature examples,
or canon here; those belong in `docs/world.md`, `docs/systems.md`, `docs/questions.md`,
or `docs/INSPIRATION.md`.

## Source of truth

- `docs/world.md` - current world, premise, factions, timeline, mystery, and hidden truth.
- `docs/systems.md` - current mechanics, design pillars, failure modes, player requirements, validation, and technical constraints.
- `docs/gate-physics.md` - the deep physics page; claims are tagged GROUNDED / STRETCH / CONCEIT.
- `docs/questions.md` - unresolved decisions, parked explorations, and deferred wants.
- `docs/INSPIRATION.md` - design compass only; not canon.
- `docs/raw/` - optional archive only. Do not treat raw notes as current truth or required reading. If a raw idea matters, fold the current version into `world.md`, `systems.md`, or `questions.md`.

## Editing rules

- Main docs hold the current model only.
- Questions hold unresolved forks.
- Raw notes are evidence/archive, not design.
- Keep the whole canon readable in one sitting.
- One subject should usually be one section, not a new file.
- Build out what is decided or load-bearing; stub unresolved pieces with `[open]` and a short pointer to `questions.md`.
- Do not invent named characters, factions, items, creatures, species, cultures, religions, myths, or languages until a mechanic needs them.

## Design order

Fix foundations before details:

1. Experience - mode by action, reach equals exposure, no defender tax.
2. Player requirements - served / routed / denied.
3. Failure modes - what each system must prevent.
4. Core loop - moment / session / long-term.
5. Gate seam - closed/open, dial out/dial known, dome, field.
6. Resources - mask energy, power cores, Potential, coordinate knowledge.
7. Conflict, economy, content - raids, trade, hubs, building, taming, items, biomes.

## System pattern

Prefer short bullets. Use nested bullets only for comments that earn their keep.

```md
- **Rule or requirement** - short decision.
  - **Why:** reason, only if useful.
  - **Serves:** player requirement IDs.
  - **Prevents:** failure mode IDs.
  - **Risks:** failure mode or open-question IDs.
  - **Test:** what must be proven.
```

- Player needs live in `systems.md#Player requirements`.
- Failure modes live in `systems.md#Failure modes`.
- Mechanics should name what they serve, prevent, or risk when that context matters.
- Use `### Why / rejected` only for decisions that are hard to reverse, surprising, or a real trade-off.

## Suggestion pattern

When suggesting a design change, explain it in this order:

1. **Current model** - what the docs currently say, in plain language.
2. **Suggestion** - the proposed change, stated as a concrete mechanic or doc change.
3. **Implementation fit** - whether it is easy to model/program, especially if it reuses data the game likely already needs.
4. **Incentives** - what behaviour the system encourages, and what degenerate behaviour it might accidentally reward.
5. **Player-type effects** - bullets for every affected player type:
   - **Raider**
   - **Builder**
   - **Explorer**
   - **Trader**
   - **Conqueror**
   - **Homesteader**
   - **Tamer**
   - **PvP brawler**
   - **Clan / zerg**

Call out whether each player type is helped, hurt, routed to a different surface, or protected from a failure mode. If a player type is not meaningfully affected, say so briefly rather than forcing a long note.

For implementation fit, prefer systems that ride on existing server facts or logs: Gate openings, transfer mass, route endpoints, open duration, claims, deaths, combat, cargo movement, station traffic, and registry changes. Example: **heat** can be a gameplay interpretation of Gate transfer/event logs the backend likely needs anyway, not a separate magic meter.

Also prefer one mechanic path that works for both real players and AI/NPC actors when possible: dead houses, AI station traffic, automated defences, trade stations, and player houses should reuse ownership, Potential, exposure, combat, cargo, and registry rules instead of becoming bespoke systems.

For incentives, state the intended push in plain terms: what the player is nudged to do, what they are nudged not to do, and whether the mechanic risks rewarding turtling, griefing, zerging, menu optimization, courier chores, or offline babysitting.

## Naming

- Use plain, globally understood English.
- Gate connection state: **Closed / Open**.
- Coordinate knowledge: **Unknown / Traced / Located / Stale**.
- Avoid **sealed** for player-facing safety. Use it only when discussing old/ancient dormant-state language if unavoidable.
- Use **Routed** for valid playstyles that belong on a designated surface, not **Redirected**.
- Use **house** for the ownership unit; a solo player is a house of one.

## Design discipline

- The user is a fullstack developer, not a game developer. When explaining systems,
  translate game-dev, engine, networking, rendering, and simulation terms into
  plain software/product language before going deep.
- Prefer **systemic / multi-use mechanics**: one mechanic should unlock several
  requirements whenever possible. The user values lazy, high-leverage reuse, so
  look for mechanics that can serve multiple surfaces before proposing bespoke
  one-off systems. Keep concrete mechanic examples in the design docs, not here.
- Prefer game-state mechanics before inventing new hidden meters: ownership, logs, routes, claims, damage, cargo, heat, presence, deaths, scans, or server events.
- Prefer Gate-native systems when they fit: travel, trade, discovery, exposure, claiming, conflict, meeting places, and progression should route through Gates or Gate-adjacent infrastructure before adding a parallel layer.
- A new knob must name the existing upstream knob that cannot carry the job.
- Do not merge knobs that fail independently.
- Trade systems must not replace play with menu optimization, and must not turn solo play into courier chores. Check both risks before adding market convenience or cargo friction.
- Balance values do not live in prose; name tunables and leave values to data/playtest.
- Do not silently contradict a recorded current call. If a new idea conflicts, stop and ask or move the fork to `questions.md`.
- Remove stale alternatives from live prose. If the fork still matters, record the current open question in `questions.md`.

## Review discipline

- When asked to review, question, or explore a system, do not treat the current docs as perfect. Use them as the current model, then think outside them and test whether a cleaner structure exists.
- Do not overfit suggestions to existing prose. If the docs are fighting the fantasy, say so and suggest the better direction.
- Still tie the final recommendation back into the repo's spine: mode by action, reach equals exposure, no defender tax, and Gates as the main seam.

## Links

- Use Obsidian wikilinks for doc links: `[[systems#Raiding|Raiding]]`, `[[world#The mystery]]`, `[[questions]]`.
- Prefer linking to the owning section rather than duplicating explanation.

Keep this file lean. Add a rule only when a mistake repeats.
