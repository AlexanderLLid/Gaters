---
name: writing-great-skills
description: Use when creating, editing, reviewing, splitting, or pruning agent skills, or diagnosing invocation variance, premature completion, duplication, sediment, sprawl, no-op instructions, or negation.
---

# Writing Great Skills

A skill exists to wrangle determinism out of a stochastic system. **Predictability** -
the agent taking the same process every run, not producing the same output - is the root
virtue; every lever below serves it.

Read [GLOSSARY.md](GLOSSARY.md) when an exact term or its full diagnostic model matters.

## Invocation

Choose according to the host's supported skill schema:

- A **model-invoked** skill keeps a model-facing description so the agent can fire it
  autonomously and other skills can reach it. It spends context load every turn.
- A **user-invoked** skill uses the host's user-only invocation control when available.
  It spends human cognitive load because the human must remember and invoke it.
- Choose model invocation when the agent or another skill must reach the skill on its
  own. Choose user invocation when human judgment should be the only trigger.
- When user-invoked skills exceed what a human can comfortably remember, add one
  user-invoked **router skill** that names them and their triggers.

## Description

A model-invoked description states the skill's leading concept and one trigger for each
genuinely distinct branch.

- Front-load the **leading word** used in prompts, docs, and code.
- Collapse synonymous triggers into one branch.
- Keep identity and process in the body; keep invocation triggers in the description.

The description is complete when every distinct invocation branch has one trigger and
every trigger maps to a real branch.

## Information hierarchy

A skill mixes two content types: **steps** and **reference**. Rank each piece by how soon
the agent needs it:

1. In-skill step - an ordered action in `SKILL.md`.
2. In-skill reference - a rule, definition, or fact needed by every relevant branch.
3. Disclosed reference - branch-specific detail behind a clearly worded context pointer.

End every step with a checkable **completion criterion**. Make it exhaustive where thin
legwork would miss required work: "every modified model accounted for" is stronger than
"produce a change list."

Use **progressive disclosure** for material only some branches need. Keep must-have
material inline when a pointer cannot be made reliable. Within each file, use
**co-location**: keep a concept's definition, rules, and caveats together.

The hierarchy is complete when every line sits on the lowest rung that still reaches
every branch that needs it.

## When to split

Each split spends either context load or cognitive load, so make it earn that cost:

- Split by invocation when a distinct leading word should trigger independently or
  another skill must reach the branch.
- Split by sequence when visible post-completion steps cause observed premature
  completion and a sharper completion criterion cannot hold the current step.

The split is justified when it removes a measured invocation or premature-completion
failure that cannot be fixed locally.

## Pruning

Keep each meaning in one **single source of truth**.

- Test each line for **relevance**: it still bears on what the skill does.
- Test each sentence as a **no-op**: it changes behavior compared with the model's
  default. Delete a sentence that fails the test.
- Remove **duplication** by keeping the meaning once and repeating only a useful leading
  word.
- Remove **sediment** when old behavior or stale branches no longer apply.
- Cure **sprawl** by moving conditional reference behind pointers or splitting a real
  branch.

Pruning is complete when each remaining sentence is live, unique, and behavior-changing.

## Leading words

A **leading word** is a compact concept already present in the model's training that
anchors behavior in few tokens, such as "lesson," "fog of war," or "tracer bullets."

- Prefer an existing concept over a coined term that needs a long definition.
- Repeat the word where it anchors execution or invocation; keep its meaning in one
  authoritative definition.
- Grade it with the no-op test. Replace a weak word with one strong enough to change
  behavior.

## Steering

Describe the target behavior positively so the desired pattern occupies the agent's
attention. Reserve prohibition for a hard guardrail that has no clear positive form,
and pair it with the behavior to perform instead.

## Failure diagnosis

- **Premature completion:** sharpen the completion criterion first; split the sequence
  only when the criterion remains irreducibly fuzzy and the rush is observed.
- **Duplication:** restore one authoritative meaning.
- **Sediment:** remove stale layers.
- **Sprawl:** disclose conditional reference or split a real branch.
- **No-op:** replace or remove guidance that does not change default behavior.
- **Negation:** state the positive target behavior.

## Completion gate

A skill edit is complete when invocation is explicit, every step has a checkable
criterion, every meaning has one source of truth, every line passes the relevance and
no-op tests, disclosed reference has a reliable pointer, and steering names the target
behavior.
