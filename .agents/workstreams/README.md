# Workstream Router

This is the shared operating contract for Codex and Claude chats. `AGENTS.md` owns the
global rules; canon stays in `docs/`; machine truth stays in `research/machines.json`.
These files record coordination only and never duplicate those authorities.

Invoke this router whenever work occurs in a long-lived specialist chat. A turn is
complete when ownership stayed intact, material exchanges are recorded and surfaced,
and the owned status file reflects the result.

## Permanent chats

| Workstream          | Owns                                                                | Priority and reason                                                                               |
| ------------------- | ------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------- |
| Primary Builder     | Unreal, world generation, integration, verification                 | Critical: unlocks and integrates every executable capability.                                     |
| Base & Raid Lab     | Base recipes, physical/tactical evaluation, raid simulation         | Critical: varied bases require an automated generate-test-improve loop.                           |
| Combat & Classes    | Abilities, classes, traversal contract, counters, balance models    | High: defines what bases and raids must support.                                                  |
| Art Direction       | Style, proportions, species, clothing language, visual comparisons  | Independent: informs assets without consuming builder progress.                                   |
| Lore                | Premise, rifts, cultures, fiction supporting proven mechanics       | Medium: coherence matters, but speculative fiction must not block prototypes.                     |
| Scale & Persistence | Multiplayer budgets, authority, persistence, scale-risk experiments | High-risk research now: protects architecture without prematurely building production networking. |

Asset and animation implementation remains with Primary Builder until its recurring
workload justifies another permanent chat. Temporary investigations stay inside the
owning workstream as subagents or reports.

## Turn protocol

1. **Orient.** Read `AGENTS.md`, this file, and every sibling status file. Complete when
   every open packet under `.agents/exchanges/` addressed to or created by this
   workstream is identified.
2. **Claim.** State the owned workstream and one current objective. Complete when the
   intended edits fit the ownership rules below.
3. **Work.** Use owned artifacts and contracts; capability work invokes
   `finding-magic-machines`. Complete when evidence supports the result or identifies a
   falsified guarantee and next experiment.
4. **Exchange.** Create or update one packet per cross-workstream dependency using the
   protocol below. Notify the target directly when task messaging exists; otherwise ask
   the human to relay the prepared line. Complete when every dependency has a packet and
   a truthful notification state.
5. **Close.** Re-scan relevant packets and update the owned status file for material
   changes. Complete when its objective, evidence, and exchange links match the
   repository state and every blocking human need is asked as one explicit question.

## Ownership

- Specialists edit their owned artifact folder and status file.
- The Primary Builder integrates shared Unreal code, canon, cross-system contracts, and
  `research/machines.json`.
- Specialists express proposed shared changes as evidence-backed `INTEGRATE` requests.
- Status files point to source paths and machine IDs; the named authority retains the
  full meaning.
- Exchange packets are phase-owned: the requester writes `Request`, the recipient writes
  `Response`, and the requester writes `Resolution`.

## Ambiguity gate

- First resolve facts from the repository, named authorities, available tools, and
  existing contracts.
- A reversible detail with one ordinary interpretation may proceed as an explicit
  recorded assumption.
- When interpretations would materially change player experience, scope, contracts,
  ownership, shared authorities, destructive actions, or expensive work, create a
  `DECISION` packet addressed to `Human` and ask one explicit question before continuing
  that branch.
- A request grants only the work it names. Adjacent features become a separate exchange
  or human decision.
- Continue independent valid work while waiting; the affected branch resumes after the
  answer is recorded in its packet.

## Exchange protocol

Create `.agents/exchanges/<WORKSTREAM>-<number>-<plain-name>.md` from `TEMPLATE.md`.
The packet is the single source of truth for its request, response, status, evidence,
and resolution. The requester's status file links it under `## Exchanges` without
copying its contents.

Use this notification line:

`EXCHANGE: <ID> | <from> -> <to> | <QUESTION|CONTRACT|BREAKING|INTEGRATE|DECISION> | <needed result>`

- `QUESTION`: another workstream must make or clarify a decision.
- `CONTRACT`: an input/output guarantee is needed before independent work can continue.
- `BREAKING`: evidence invalidates an assumption another workstream may be using.
- `INTEGRATE`: verified specialist output is ready for the Primary Builder.
- `DECISION`: human judgment or authority is required.

Statuses are `open`, `answered`, `waiting-human`, and `resolved`. The recipient changes
`open` to `answered` after writing `Response`; the requester changes it to `resolved`
after writing `Resolution`. A human-directed decision uses `waiting-human` until the
answer is recorded.

Repository packets share outcomes, not chat transcripts. A decision needed elsewhere is
written into its packet with the minimum supporting evidence.

### Notification and human needs

- When task messaging is available, send the exchange line to the target and record
  `Notification: sent`.
- Otherwise record `Notification: relay-needed` and ask:
  `ACTION REQUIRED: Please open <target> and paste: <exchange line>`.
- For a human decision, surface:
  `HUMAN DECISION NEEDED: <ID> | <one explicit question>`.
- Continue independent work while a packet is pending. Pause only when its answer is a
  real prerequisite for further valid work.

## Magic-machine rule for builders

Invoke `.agents/skills/finding-magic-machines/SKILL.md` for capability and implementation
work. That skill is the single source of truth for decomposition, evidence, and promotion.
