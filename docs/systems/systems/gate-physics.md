---
type: system
status: draft
tags: []
sources: [raw/design-overview.md]
aliases: [Gate Physics]
updated: 2026-06-27
---

# Gate Physics

The one area we go deep. Gating must bind to real physics, so every claim is
tagged: **GROUNDED** (real physics supports it), **STRETCH** (extrapolated but
defensible), or **CONCEIT** (game fiction, no physical basis — keep it, just
never dress it as real). Add new physics claims here and tag them (src:
raw/design-overview.md).

## Basis — a traversable wormhole

A [[Gates|Gate]] is a **traversable wormhole**: a Morris–Thorne throat joining two
distant mouths, with no horizon, finite tidal forces, and two-way passage.
[STRETCH — the general-relativity solution exists; no known matter realizes it.]
Lineage: Einstein–Rosen bridge (1935) → Morris–Thorne (1988).

## Holding it open — exotic matter = the power core

Keeping the throat open requires **exotic matter that violates the null energy
condition** (negative energy density / outward tension). Remove it and ordinary
gravity collapses the throat. [GROUNDED — Morris–Thorne 1988.] In-world, that
NEC-violating source is the **power core**; "it needs fuel to stay open" is the
real requirement, packaged.

- Real negative-energy sources exist but are minute: the **Casimir effect** and
  **squeezed-vacuum light**, both lab-confirmed. [GROUNDED background.]
- The **Ford–Roman quantum inequalities** cap negative energy — bigger or
  longer-lived throats force the walls absurdly thin (near the Planck length).
  This is the real realism ceiling, and the in-world reason cores are scarce and
  Gates **cannot be mass-produced**. [GROUNDED background.]

## Stable vs unstable — Gate vs Rift

An un-propped wormhole (the Schwarzschild / Einstein–Rosen bridge) is
non-traversable: it **pinches off faster than light can cross it** (Fuller–Wheeler
1962). [GROUNDED.] That _is_ a **Rift** — a throat with no exotic-matter support,
collapsing on its light-crossing time. A Gate is the same throat held open by a
core. So **Gate vs Rift = propped vs un-propped** — one physics, two states.
(Rifts appearing _near_ Gates: [CONCEIT] — GR has no regional "thin spacetime".)

## Energy cost

Holding a metres-to-kilometres throat open needs negative mass-energy on the order
of a **planet's mass** (≈ −10²⁷ kg; Morris–Thorne, Visser). [GROUNDED magnitude.]
So firing a Gate is among the most expensive acts physically possible — grounding
scarce cores and monumental Supergates. But cost scaling with **distance** is
[CONCEIT] — GR cost depends on throat geometry, not how far apart the mouths are.
Keep distance-banding as a game lever, not a physics claim.

## Time

Moving one mouth relativistically **desyncs the two clocks** (the twin paradox) —
the Morris–Thorne–Yurtsever (1988) time-machine result. [GROUNDED.] A fixed
~38-minute open cap has no derivation [CONCEIT] — keep it as a balance number.

## What passes through

A Morris–Thorne throat is two-way and transparent: light and matter cross, with
**momentum and energy conserved**. [GROUNDED.] So "fire and momentum carry through
an open Gate" is real. "One-way from the dialing side" and "**sealed by default**"
are gameplay gating with no physical basis [CONCEIT] — flag them as rules.

## Aperture size — driven, not built

Throat diameter is set by how much exotic matter you channel and confine, **not** by
building bigger hardware: the Builder ring is fixed, the aperture is a runtime variable.
[GROUNDED — Morris–Thorne energy scales with throat size.] The **Ford–Roman quantum
inequality** caps one ring (bigger throats force the walls Planck-thin → collapse), so
fleet-wide apertures need multiple rings combined — a megastructure of **found** segments,
never fabrication. [GROUNDED ceiling.] Overdrive one ring past the cap and it collapses to
the un-propped state — a Rift. This is the physics under the driven aperture ([[gates|Gates]]) and the gate-overload bomb.

## The dome — protecting transit, clearing builds

An active throat must **flare out** into ambient space (the Morris–Thorne flare-out
condition); just outside the mouth the curvature gradient is steep, so tidal shear tears
anything rigidly anchored across it. [GROUNDED — tidal forces near steep curvature; M–T
size their throat precisely so a _traveller_ survives the centre.] That lethality is why
matter needs shielding to cross — in-world, the Builders' **dome**: a protective field,
sized to the throat, that makes transit survivable and whose radius scales with gate
power. [CONCEIT — projected force fields aren't real; same conceit family as the mask
field.] The dome **destroys player-built structures** in its radius but spares natural
terrain — it senses _construction, not material_. [CONCEIT — a Builder discriminator; no
physics tells manufacture from nature.] So a Gate can never be walled in (the [[combat|dome]]); the
tidal lethality is the GROUNDED reason something has to clear, the dome is the engineered
how.

## Pure conceits (do not claim physics)

- A sealed-but-sub-traversable default state (GR throats are open or collapsed).
- A life-support field that weakens with radius.
- Rifts clustering near Gates.
- The gate dome: a protective field that shields transit and destroys only _built_ matter
  (the tidal lethality it answers is real; the field and the build-vs-nature sense are not).

## Code

Placeholder until the Unity project exists.

## Open questions

- Pick one self-consistent story for "sealed by default" that doesn't contradict
  collapse physics (a mechanical shutter? a throat kept collapsed and re-kindled?).
- Decide whether a power core **is** the exotic matter, or powers a device that
  confines it.
- Tracked with the rest in open-questions.md.

## References

- Morris & Thorne, _Am. J. Phys._ **56**, 395 (1988) — traversable wormholes, exotic-matter requirement.
- Morris, Thorne & Yurtsever, _Phys. Rev. Lett._ **61**, 1446 (1988) — wormhole time desync.
- Einstein & Rosen, _Phys. Rev._ **48**, 73 (1935); Fuller & Wheeler, _Phys. Rev._ **128**, 919 (1962) — bridge collapse.
- Ford & Roman, _Phys. Rev. D_ **53**, 5496 (1996) — quantum inequalities. Visser, _Lorentzian Wormholes_ (1995).
