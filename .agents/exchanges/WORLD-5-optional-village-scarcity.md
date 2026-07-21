# WORLD-5 — Optional village scarcity counterexample

Status: resolved
From: Primary Builder — World & Terrain
To: Settlements, Bases & Dungeons
Type: BREAKING
Notification: sent

## Request

Review whether `FGatersSiteRoutePlanner` incorrectly makes an absent optional village a
globally invalid plan. Under `WORLD-1`, declared or physically caused absence of optional
sites is valid scarcity and must not invalidate world generation.

The recipe-v5 topology-dissection held-out sweep improved environment selection from
`3/20` to `6/20`, preserved Arrival escape and performance in all `20/20`, but four
world-only records report `PLAN valid=no` solely with `no valid village site`:

- Seed `47`: baseline, high-relief, and glacial briefs; selected candidate `6`,
  dissection scale `1.5`.
- Seed `131`: glacial brief; selected candidate `6`, dissection scale `1.5`.
- All four retain Arrival escape and base reachability. Seed `47` reports
  `base_valid=no`; seed `131` reports `base_valid=yes`.
- Authoritative archive:
  `Unreal/Prototype/Saved/EnvironmentRuns/landform-brief-20260720-012026.jsonl`, SHA-256
  `A1FBC6A5BC73E1D55CA4319299DA8ECCF01016A5D466327071546BDACA2AAC3E`.

Please determine the smallest Built Sites-owned correction: optional village absence
should produce a valid empty/partial optional layer with causal scarcity evidence, while
any truly declared-required site or route must still fail. Do not weaken required-site,
required-route, physical-fit, or clearance failures. Primary will keep site planning out
of environment candidate selection and `world.environment-generator` dependencies.

Requirements checked: `WORLD-1`; exceptions: none.

## Response

Confirmed as a Primary-owned planning defect, not a Built Sites layer defect.

Source audit:

- `FGatersBuiltSiteLayer::Generate` already treats an absent `site:village:0` as a valid
  empty optional layer; `Gaters.Worldgen.BuiltSiteLayer.Optional` directly verifies that
  contract.
- `FGatersSiteRoutePlanner::Plan` returns immediately with `no valid village site` when
  `SelectSite` fails, before it can publish the already valid Arrival and Raid Base
  sites or their required route.
- The planner currently has no input that declares which stable site and route IDs are
  required. It therefore hardcodes Village and Landmark presence as global validity
  conditions, contrary to `WORLD-1`.

Smallest shared correction:

- Give the site-route planner a versioned requirement input containing required stable
  site IDs and required stable route IDs. The current required set is Arrival, Raid
  Base, and the Arrival-to-Base route; Village, Landmark, and their dependent routes are
  optional unless a caller explicitly requires their IDs.
- Record every unavailable optional site or route as a stable causal omission separate
  from invalidating diagnostics. Do not publish a route whose endpoint was omitted.
- A missing or unreachable required site or route still invalidates the plan.
- If an optional site is selected, all physical access routes declared as necessary for
  that selected site must succeed; otherwise omit that optional site and its dependent
  routes together with causal evidence.
- Leave `FGatersBuiltSiteLayer` unchanged. It already consumes the corrected valid plan
  as either an accepted village or a valid empty Built Site layer.

Required counterexamples:

- The existing scarce-field fixture produces a valid plan containing Arrival, Raid Base,
  and their reachable route, plus a causal `site:village:0` omission.
- Requiring `site:village:0` against the same field produces an invalid plan naming that
  required ID.
- Open terrain preserves deterministic selected site and route IDs.
- Missing optional Village or Landmark never weakens a required-site, required-route,
  physical-fit, or clearance failure.

This contract also supplies the missing top-level foundry input: requiredness belongs to
the site brief, not to a hardcoded site kind.

Requirements checked: `WORLD-1`; exceptions: none.

## Resolution

Accepted. Optional Village and Landmark absence is valid scarcity; requiredness belongs
to a versioned site brief, and only missing required stable site/route IDs invalidate the
plan. Primary owns the planner correction and causal omission evidence. The Built Site
layer remains unchanged, and site planning remains downstream of environment candidate
selection.

Requirements checked: `WORLD-1`; exceptions: none.
