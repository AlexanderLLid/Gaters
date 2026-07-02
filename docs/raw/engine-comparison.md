# Engine comparison — Unity vs Unreal (vs Godot)

Decision-support, **not a decision**. Captures the differences that matter for Gaters
so the engine call can be made later. No verdict here. When chosen → write an ADR with
the why and what was rejected (see questions.md #21). Sources at the bottom; figures
are mid-2026.

Third option (Godot) listed for completeness but not a serious contender for a 3D
open-world raid game — kept so the comparison is honest, not to pad choices.

## Side by side

| Dimension                      | Unity                                                 | Unreal Engine 5                                                      | Godot                       |
| ------------------------------ | ----------------------------------------------------- | -------------------------------------------------------------------- | --------------------------- |
| Language                       | C#                                                    | C++ (+ Blueprints visual scripting)                                  | GDScript / C#               |
| Learning curve                 | Gentlest; biggest tutorial corpus                     | Steeper; C++ + heavier toolchain                                     | Gentle, smaller corpus      |
| 3D fidelity ceiling            | High                                                  | Highest (Nanite, Lumen)                                              | Lower                       |
| Out-of-box "looks expensive"   | No — you build the look                               | Yes                                                                  | No                          |
| Procedural generation          | Capable, but mostly DIY / asset-store                 | **PCG framework, production-ready**                                  | DIY                         |
| Open-world streaming           | DIY / 3rd-party (custom or asset-store)               | **World Partition built-in** (auto-stream, HLOD, large-world coords) | DIY                         |
| Agent-in-editor (MCP)          | Behind                                                | **First-party MCP in UE 5.8** (Claude Code drives editor)            | Community only              |
| Asset ecosystem                | Largest, cheapest (Asset Store)                       | Fab (ex-Megascans, **paid since 2025**)                              | Smallest                    |
| Hardware floor (light profile) | Low by default                                        | Low **if** you opt out of Nanite/Lumen                               | Low                         |
| Iteration / build speed        | Fast                                                  | Slower; large projects                                               | Fast                        |
| Multiplayer netcode            | Netcode for GameObjects or 3rd-party (Mirror/Fishnet) | Stronger built-in replication                                        | Weakest of the three        |
| Pricing model                  | Per-seat subscription (Runtime-Fee reverted)          | 5% royalty after $1M                                                 | Free, MIT                   |
| Reputation / trust             | Recovering from 2023 Runtime-Fee fiasco               | Momentum engine; rising Steam revenue share                          | Goodwill darling since 2023 |

## Factors specific to Gaters

These are where the generic comparison meets _this_ game.

- **Solo dev + agents.** UE 5.8 ships a first-party MCP server — Claude Code can spawn
  actors, run PCG, place props, edit Blueprints in the live editor. Strongest agent-content
  story of the three. Tilts toward Unreal **if** the agent pipeline is central to how the
  game gets built.
- **PvP visual parity.** A raid game can't have low-settings players seeing through cover
  high-settings players can't (the ARC Raiders / Tarkov problem). This is a _design_ fix
  (clamp gameplay-relevant visuals), engine-agnostic — but a low hardware floor reduces the
  pressure to scale settings at all. Both engines can do it; neither does it for free.
- **Player-built bases = object-count cost.** In a builder the perf enemy is the number of
  player-placed pieces, not the engine (Satisfactory's heavy frames are megafactories, not
  Lumen). And in _raids_ the defender's build size sets the attacker's load. This is the
  load-bearing optimization concern whichever engine wins.
- **Art identity is still open** (NMS-lush vs. low-poly-stylized — see questions.md #7/#6).
  Unreal's fidelity edge only pays off for the lush path; for stylized/low-poly it's neutral
  and Unity's iteration speed counts for more.
- **EU / Copenhagen legal (engine-independent, but affects the asset pipeline).** EU AI Act
  Art. 50 requires machine-readable labeling of AI-generated images/audio/video — in force
  **2 Aug 2026**, fines up to €15M / 3% of turnover. Steam also requires a public AI-content
  disclosure. If AI-generated assets are part of the plan, keep an inventory and a human in
  the loop on each asset (also secures copyright — pure-AI output may get weak/no protection).
  This applies regardless of engine.

## The Satisfactory data point

Relevant because it's liked as a reference and proves the "light UE5" path:

- UE5 open-world builder, **GTX 1650 minimum**, ~25–30 devs.
- **Lumen off by default**, baked lighting; Nanite used selectively (terrain/foliage, not
  factory pieces). Devs say it runs better without Lumen and don't plan to optimize for it.
- Shows the Ark: Survival Ascended "nobody can run it" outcome was a _choice_ (forced
  Nanite/Lumen), not a UE5 tax. A deliberately light UE5 profile is a shipped, proven path.

## The Rust data point

The closest genre sibling Gaters has — survival + player-built bases + online/offline raiding is
Rust's core loop. Built on **Unity** (~10 years, Facepunch). But the lesson isn't "Unity does the
genre":

- **Unity is the shell; Facepunch built the hard parts.** Rendering/physics/editor/asset pipeline
  are Unity. The genre-defining systems are custom Facepunch code: **their own networking layer**
  (not Unity netcode — nothing shipped could sync hundreds of players × thousands of structures),
  authoritative server, world persistence/wipe cycles, the building + structural-stability model,
  and entity-sync/culling at scale.
- **So the hard part of Gaters — networked base-raiding at scale — is custom on _any_ engine.**
  The engine question isn't "which does the genre" (neither does) but "which shortens the custom
  netcode I can't avoid." Unreal's built-in replication is a bigger head start than Unity's default
  — which is why Facepunch said a clean-start sequel "**definitely won't be a Unity game**."
- **Solo-dev consequence:** can't rebuild a decade of custom netcode. Lean on the engine's built-in
  networking (favors Unreal) and **scope raid scale to what the engine gives**, not Rust-scale.
  Hundreds × thousands is the custom-tech cliff that took Facepunch ten years.

## What would tip the decision

- **Toward Unreal:** open-world streaming for free (World Partition) — the strongest factor for
  this game · agent-built content is central · want production-ready PCG · lush/high-fidelity art
  identity · value built-in replication for the raid netcode.
- **Toward Unity:** want fastest iteration / gentlest ramp · stylized-low-poly art (fidelity
  edge wasted) · prefer C# · lean on the cheapest/largest asset store.
- **Still unknown / blockers to resolve first:** art identity (questions.md #7/#6),
  whether the agent pipeline actually holds up (needs a throwaway prototype), and the base
  object-count budget.

## Sources

- [Unreal vs Unity 2026 for indie devs — StraySpark](https://www.strayspark.studio/blog/unreal-engine-vs-unity-2026-indie-developers)
- [AI in Unreal vs Unity — Muddy Terrain](https://muddyterrain.com/blog/ai-unreal-engine-vs-unity)
- [Unreal MCP — Epic official docs (UE 5.8)](https://dev.epicgames.com/documentation/unreal-engine/unreal-mcp-in-unreal-editor?lang=en-US)
- [Satisfactory system requirements — official wiki](https://satisfactory.wiki.gg/wiki/System_requirements)
- [Satisfactory & Unreal Engine (Nanite/Lumen) — official wiki](https://satisfactory.wiki.gg/wiki/Unreal_Engine)
- [Do low settings give a PvP advantage in ARC Raiders? — Icy Veins](https://www.icy-veins.com/other-games/news/do-low-settings-give-a-huge-pvp-advantage-in-arc-raiders/)
- [Fab / Megascans licensing transition — Quixel](https://quixel.com/en-US/license)
- [EU AI Act Article 50 transparency obligations](https://digital-strategy.ec.europa.eu/en/policies/regulatory-framework-ai)
- [AI-generated game assets: copyright & disclosure — Promise Legal](https://blog.promise.legal/ai-generated-assets-game-ip-disclosure/)
