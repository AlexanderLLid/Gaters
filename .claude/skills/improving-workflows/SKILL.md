---
name: improving-workflows
description: Use when a followed skill or agent workflow still produces a bad result, requires an improvised missing step, lacks diagnostic evidence, or repeats a manual workaround.
---

# Improving Workflows

A failure changes a skill only when the workflow, rather than its execution or the
machine under test, caused the failure.

1. Preserve the smallest reproducible evidence in `.agents/workflow-feedback.md`.
   Done when another agent can identify the failed expectation and observed result.
2. Classify the cause as **execution** (the skill was not followed), **machine** (the
   implementation failed), or **skill** (a trigger, step, completion criterion,
   verifier, or failure path was missing). Done when the evidence supports one class.
3. For a skill failure, propose the smallest behavior-changing patch. Apply it only
   when the human requested skill changes. Done when the proposal names the exact skill
   and changed behavior.
4. Re-run the original case and one ordinary case. Done when the failure is corrected
   without damaging the ordinary workflow.

Record every workflow failure, but propose a permanent rule only for a structural gap
or repeated evidence. Append new log entries; never rewrite earlier entries.

## Output

`Cause: execution|machine|skill. Evidence: ... Proposed correction: ... Verification: ...`

