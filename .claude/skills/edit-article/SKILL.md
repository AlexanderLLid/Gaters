---
name: edit-article
description: Revise a doc section by section — reorder by dependency, tighten prose. Use when the user wants to revise, restructure, tighten, or improve a wiki page (not for trivial edits like a tag flip or typo).
---

# edit-article

Revise a page so it reads in **dependency order** and every paragraph earns its
place. Information is a DAG: a section must not rely on something defined later.

Trivial change (a tag flip, a typo, a path fix)? Skip this — just edit.

1. **Plan the sections.** Split the page by heading. Order them so each builds only
   on what came before — no forward dependencies. Propose the section list and any
   reorder, and **confirm with the user before rewriting.**

2. **Rewrite section by section,** in the confirmed order. For each:
   - Tighten for clarity, coherence, and flow; cut what the section doesn't need.
   - **Max 240 characters per paragraph.**
   - Preserve the frontmatter, the wikilinks (`[[basename|Display]]`), and the
     `[decided]/[tentative]/[open]` tags. Do not invent content or expand a topic
     CLAUDE.md defers.
   - Hold the page's altitude: a high-level page stays high-level (the one
     exception is gate-physics, which goes deep).

Done when every section is rewritten and the page reads in order.
