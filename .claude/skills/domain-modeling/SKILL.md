---
name: domain-modeling
description: Build and sharpen the project's shared language and recorded decisions. Use when pinning down terminology (the docs/CONTEXT.md glossary), recording a design decision (folded into its concept page) or an architecture decision (ADR), or when another skill needs to maintain these.
---

# Domain Modeling

Actively build and sharpen the project's shared language and decisions as you
design — challenge fuzzy terms, stress-test with scenarios, and write the glossary
and decisions down the moment they crystallise. (Merely _reading_ docs/CONTEXT.md
for vocabulary is a one-line habit any skill can do; this skill is for _changing_
the model.)

Where decisions live:

- **Design / world / mechanics decision → fold it into its concept page.** The page in
  `docs/lore/` or `docs/systems/` that owns the topic carries the _why_ and _what was rejected_
  in a short `## Why / rejected` section, cross-referenced to sibling pages. There is **no
  separate design-decision register** — the rationale lives next to the thing it governs. If a
  decision is purely cross-cutting with no page yet, put it in the nearest overview
  ([[World Overview]] / [[Systems Overview]]) or create the concept page.
- **Architecture / technical decision → ADR** (`docs/adr/`), standard format:
  [ADR-FORMAT.md](./ADR-FORMAT.md). For the eventual Unity code, the repo, tooling.
- **Glossary — `docs/CONTEXT.md`:** the ubiquitous language. Format:
  [CONTEXT-FORMAT.md](./CONTEXT-FORMAT.md).

Create or extend lazily — only when there is something to write.

## During the session

### Challenge against the glossary

When a term conflicts with docs/CONTEXT.md, call it out at once. "CONTEXT defines
`gate-open` as full exposure, but you're using it for trade — which is it?"

### Sharpen fuzzy language

When a term is vague or overloaded, propose one canonical word and list the rest
under `_Avoid_`. "You said 'portal' — that's a Gate. Keep one word."

### Stress-test with scenarios

When relationships are being worked out, invent concrete edge cases that force
precision about the boundaries between concepts.

### Update CONTEXT.md inline

When a term resolves, write it to docs/CONTEXT.md right then — don't batch. It is a
**glossary and nothing else**: no lore, no mechanics, no implementation. Use the
format in [CONTEXT-FORMAT.md](./CONTEXT-FORMAT.md).

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
