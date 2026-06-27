---
name: setup-wiki
description: One-time setup for the Gaters wiki. Run once after cloning to confirm conventions before using the other wiki skills. Use when the user says set up the wiki, configure the wiki, or initialize the vault.
---

# setup-wiki

Run once per clone. Confirm with the human and record any changes in CLAUDE.md
if they differ from the defaults:

1. Where raw sources live (default: docs/raw/).
2. Scope: lore only, systems only, or both (default: both).
3. Obsidian plugins in use (Dataview, Templater) so pages stay compatible.
4. Whether scripts/lint.mjs should run as a pre-commit hook.

Then read CLAUDE.md, docs/CONTEXT.md, and docs/index.md so the schema and current
catalog are loaded. Do not create content; this only configures.
