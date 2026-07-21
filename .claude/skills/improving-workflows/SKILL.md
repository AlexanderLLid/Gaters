---
name: improving-workflows
description: Use when a followed skill or agent workflow still produces a bad result, requires an improvised missing step, lacks diagnostic evidence, or repeats a manual workaround.
---

# Improving Workflows

A failure changes a skill only when the workflow, rather than its execution or the
machine under test, caused the failure.

1. Preserve the smallest reproducible evidence in `.agents/workflow-feedback.md`.
   Done when another agent can identify the failed expectation and observed result.
2. Immediately dispatch one bounded reviewer agent for that entry. Pass the exact entry,
   relevant workflow or skill paths, and whether the human authorized edits. The reviewer
   searches related prior feedback, classifies the cause, proposes the smallest correction
   and its verification, but does not edit, append, or dispatch again for the same failure.
   The originating task owns integration and routes cross-owner changes. If delegation is
   unavailable, perform the same review locally and record that fallback. Done when one
   reviewer report or a concrete blocker returns.
3. Classify the cause as **execution** (the skill was not followed), **machine** (the
   implementation failed), or **skill** (a trigger, step, completion criterion,
   verifier, or failure path was missing). Done when the evidence supports one class.
4. For a skill failure, propose the smallest behavior-changing patch. Apply it only
   when the human requested skill changes. Done when the proposal names the exact skill
   and changed behavior.
5. Re-run the original case and one ordinary case. Done when the failure is corrected
   without damaging the ordinary workflow.

Record every workflow failure, but propose a permanent rule only for a structural gap
or repeated evidence. Append new log entries; never rewrite earlier entries.

## Output

`Coordinator: ... Cause: execution|machine|skill. Evidence: ... Proposed correction: ... Verification: ...`
