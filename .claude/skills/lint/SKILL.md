---
name: lint
description: Health-check the Gaters wiki for contradictions, stale pages, orphans, and dangling links. Use when the user asks to lint, audit, or check the wiki.
---

# lint

1. Run node scripts/lint.mjs for the cheap deterministic checks: missing or
   invalid frontmatter type, unresolved hard contradictions, orphan pages with
   no inbound wikilinks, and dangling wikilinks with no page.
2. Then do the reasoning checks the script cannot: canon or mechanics
   contradictions between pages, claims a newer source superseded, and obvious
   missing cross-references. Scope these to pages changed since the last lint
   plus their wikilink neighbors unless asked for a full sweep.
3. Report findings grouped by severity. Do not fix silently; propose changes.
