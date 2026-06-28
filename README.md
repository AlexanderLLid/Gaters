# Gaters

Parent repo for the game **Gaters**. Right now it holds the **design docs**; the
Unity game is added later as a sibling folder under this root.

## Layout

- `CLAUDE.md` — the operating manual the agent follows.
- `docs/` — the design wiki (an Obsidian vault):
  - `docs/lore/` — world bible (World Anvil types).
  - `docs/systems/` — mechanics bible; `gate-physics` is the one deep, real-physics page.
  - `docs/_templates/` — page templates per type.
  - `docs/raw/` — immutable sources, never edited.
  - `docs/index.md`, `docs/CONTEXT.md`, `docs/open-questions.md`.
- `.claude/skills/` — the agent behaviors (/ingest, /ask, /new-page, /lint, ...).
- `scripts/lint.mjs` — deterministic checks (run locally or via /lint).
- _(later)_ the Unity project, as its own folder.

## Current phase

High-level lore of the **Builders** and the **Central Authority**, plus the
**gate physics** in depth (bound to real physics). Specific characters, weapons,
items, and gameplay-systems detail are deferred — see _Scope & priorities_ in
`CLAUDE.md`.

## Use it

1. Open `docs/` in Obsidian (graph view, wikilinks).
2. Run Claude Code from the repo root; it reads `CLAUDE.md` and loads the skills.
3. Drop sources into `docs/raw/` and run /ingest. Ask questions with /ask. Run
   /lint now and then.
