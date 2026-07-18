# Base and Raid Research Brief

Owned by the Base & Raid Lab workstream. This is the stable research shape, not a status
page; `research/machines.json` owns the live capability graph.

## Outcome

Terrain sites become varied, valid, raidable bases whose strengths and weaknesses are
measured across multiple attacker and defender policies. The loop improves variety
without converging on impossible fortresses or one cosmetically reskinned layout.

## Machine sequence

1. **Physical validity** — foundations, contacts, clearance, entrances, navigation, and
   required relationships pass before combat simulation.
2. **Raid-space evaluation** — approaches, cover, sightlines, choke concentration,
   breach options, spawn safety, loot exposure, and extraction become descriptors.
3. **Deterministic combat simulation** — placeholder attackers and defenders execute the
   minimum movement, targeting, damage, death, and objective rules.
4. **Encounter evaluation** — multiple policies test each base; no single bot becomes the
   quality oracle.
5. **Base generation** — layouts consume terrain, content, combat, and budget contracts
   and emit causal rejection evidence.
6. **Quality diversity** — retain strong bases across tactical niches instead of selecting
   one global winner.

Registry nodes: `evaluation.physical-fit`, `evaluation.raid-space`,
`runtime.combat-simulation`, `evaluation.encounter`, `world.base-generator`, and
`research.quality-diversity-archive`.

## Promotion shape

- Hard invalidity rejects a candidate; it is never traded against a high score.
- Tactical quality is a metric vector and descriptor set, not one fitness number.
- Challengers face held-out terrain seeds and attacker/defender policies.
- Failure artifacts identify the responsible layout decision or missing contract.
- Human raids audit whether measured variety transfers to interesting play.

## Cross-workstream contracts

- Primary Builder provides terrain, buildability, route, content, and persistence seams.
- Combat & Classes provides baseline traversal, attack, defense, destruction, and
  objective capabilities.
- Scale & Persistence provides simulation and replicated-entity budgets before live
  multiplayer implementation.
- Art may replace semantic assets without changing base recipes or tactical evidence.
