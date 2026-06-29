---
name: contradiction-check
description: The protocol for handling conflicts between new content and existing pages, for both canon and mechanics. The agent reaches for this during ingest and editing whenever a fact may conflict with an existing page.
---

# contradiction-check

When new content conflicts with an existing page, classify it:

- soft - tone or emphasis difference. Non-blocking. Flag and note.
- scope - both true in different contexts. Non-blocking. Flag and explain each
  scope.
- hard - direct factual conflict (a character dies in two places; a formula
  defined two ways; a date clash with the timeline). Blocking.

For hard conflicts: do not pick a winner. Stop and ask the human to resolve.
Never overwrite established canon or a design decision in passing.

Record the conflict inline on the page where it lives:

```
> Contradiction severity: hard
> Status: unresolved - flagged for review
> Conflict: [[Source A]] says X; [[Page B]] says Y.
```

Leave it on the page until resolved.
