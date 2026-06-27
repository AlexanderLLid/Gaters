# CONTEXT.md - Shared Language

The working vocabulary for Gaters. Use these terms consistently in pages, code,
commits, and chat. Add a term when a concept keeps needing a sentence to
explain. This is a glossary, not canon; it decodes jargon.

## Core concept
- Gate - the network device connecting planet-bases. Sealed by default.
- Planet-base - a single player's planet and home base.
- Hub world - a neutral, lawless crossroads world for staging raids and routing
  trade to distant partners.

## Exposure states
- Sealed - gate closed, planet safe. The default safety guarantee.
- Port open - partial exposure for trade.
- Gate open - full exposure; raiding is possible.
- Siege timer - the committed window once a gate is opened.
- Mask energy - resource tied to gate energy that bounds survivable range and
  acts as the raider timer.

## Planet runtime states
- Empty - a database row only.
- Solo-occupied - one owner present; client-authoritative PvE.
- Contested - a raid or trade in progress; server-authoritative instance.

## Storage
- Seed-plus-deltas - planets stored as a seed plus changes, lazy-loaded on
  demand, so server cost scales with online players.

## Stack
- Unity with C#. Server-authoritative only during contested sessions.

(Replace or extend as the design firms up. Keep each definition to a line.)
