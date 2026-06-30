---
name: domain-modeling
description: Build and sharpen the project's recorded decisions. Use when recording a design decision (folded into its concept page) or an architecture decision (ADR), or when another skill needs to maintain these.
---

# Domain Modeling

Actively build and sharpen the project's decisions as you design — challenge fuzzy
terms, stress-test with scenarios, and write decisions down the moment they
crystallise. This skill is for _changing_ the model, not just reading it.

Where decisions live:

- **Design / world / mechanics decision → fold it into its concept page.** The page in
  `docs/lore/` or `docs/systems/` that owns the topic carries the _why_ and _what was rejected_
  in a short `## Why / rejected` section, cross-referenced to sibling pages. There is **no
  separate design-decision register** — the rationale lives next to the thing it governs. If a
  decision is purely cross-cutting with no page yet, put it in the nearest overview
  ([[World Overview]] / [[Systems Overview]]) or create the concept page.
- **Architecture / technical decision → ADR** (`docs/adr/`), standard format:
  [ADR-FORMAT.md](./ADR-FORMAT.md). For the eventual Unity code, the repo, tooling.

Create or extend lazily — only when there is something to write.

## During the session

### Sharpen fuzzy language

When a term is vague or overloaded, propose one canonical word and use it
consistently in the pages. "You said 'portal' — that's a Gate. Keep one word."

### Stress-test with scenarios

When relationships are being worked out, invent concrete edge cases that force
precision about the boundaries between concepts.

### Record a decision sparingly

Fold a decision into its concept page only when all three hold:

1. **Hard to reverse** — changing your mind later costs real work.
2. **Surprising without context** — a future reader will ask "why this way?"
3. **A real trade-off** — there were genuine alternatives and you picked one.

Miss any one, skip it. Record the _why_ and _what you rejected_ — not ceremony. **Never write
"supersedes the old X" history**; change the page in place as if it had always been that way
(nothing is locked — see CLAUDE.md). Pick the home: design/world → concept page; technical/code → ADR.

### Graduate open questions

docs/open-questions.md is the backlog of unresolved decisions. When one settles, fold the
_why_ + _rejected_ into the relevant concept page's `## Why / rejected`, flip that page's tag
`[tentative]→[decided]`, and strike it from open-questions.
