# Workflow feedback

Append-only evidence shared by Codex and Claude. A log entry is not automatically a
new rule; `improving-workflows` classifies it first.

## 2026-07-16 — Magic machines lacked a closed verifier

- Workflow: `finding-magic-machines`
- Expected: a built machine can improve from failures without silently regressing.
- Observed: evidence was required, but no independent verifier, held-out challenge set,
  diagnostic artifact, or champion-challenger promotion gate was required.
- Cause: skill
- Correction: added the missing verification-loop contract to every Adapt/Build node.
- Verification: structural gap is explicit; terrain evaluator will exercise the normal
  generator → independent evaluator path.

## 2026-07-16 — Terrain verifier missed signed shape and runtime traversal

- Workflow: `finding-magic-machines` promotion loop
- Expected: a verified terrain generator rejects crater-like mountains and arrival areas
  that are locally valid but trapped at runtime dimensions.
- Observed: generator-3/evaluator-1 passed representative automation while seed 53 read
  as holes; generator-5 then passed focused shape tests but its first archived runtime
  sweep reported `escape=no` and `base=no`.
- Cause: machine. The promotion workflow correctly blocked generator-5, but the initial
  evaluator challenge set lacked signed elevation, local hydrology, and the exact visual
  failure seed at the 4 km runtime contract.
- Correction: evaluator-2 added signed mean/below-datum evidence; evaluator-3 records
  the local analysis window so streamed-world scale cannot hide local hydrology. Seed 53
  remains a permanent mountain-lake and traversal challenge; real sweeps remain mandatory.
- Verification: generator-6 passed 20/20 worldgen tests and its archived seed-53 sweep
  recorded `escape=yes`, `base=yes`, 0.8771 reachable coverage, and 86 local water cells.

## 2026-07-17 — Archipelago verifier used the wrong scale

- Workflow: `finding-magic-machines` terrain promotion loop
- Expected: a verified archipelago produces local islands and water channels in the
  runtime world contract.
- Observed: generator-7 passed the family test at a 300 m logical world size, but the
  runtime seed-2 sweep used 4 km and reported `water=0.0000`; its central island scaled
  beyond the entire 300 m gameplay window.
- Cause: machine. The evaluator supported a local window, but the archipelago challenge
  instantiated the generator at the evaluation-window size instead of runtime size.
- Correction: seed 2 is now a permanent runtime-dimension challenge requiring local
  water, buildable land, dry arrival clearance, reachable base, and paired captures.
- Verification: generator-8 seed 2 records `water=0.6436`, `buildable=0.1211`,
  `coverage=0.8019`, `escape=yes`, and `base=yes`; 27/27 Gaters tests pass.
### Gallery exposure rejected readable warm terrain

- Cause: machine.
- Evidence: seed 7 produced a clearly readable saturated canyon capture, but `Test-GalleryImage.ps1` rejected it with mean luminance `60.1223` below `70` because the verifier measured luminance alone.
- Proposed correction: retain the near-black guard and accept the stronger of mean luminance or mean HSL-style brightness so saturated warm terrain is not mislabeled as underexposed.
- Verification: the saturated-warm and neutral fixtures pass, the dark fixture fails, and real warm canyon seed 7 plus ordinary archipelago seed 19 pass.

### Character style board hid the low-detail range

- Cause: execution.
- Evidence: the fantasy-progression prompt asked for premium/high-quality concept art but
  did not cap seams, layers, ornament, texture noise, or facial micro-detail. The resulting
  board converged on dense semi-realistic MMO clothing and the user rejected every style
  as overly detailed.
- Proposed correction: future comparison prompts hold one explicit low detail budget:
  broad untextured surfaces, two or three garment layers, few seams, no micro-wear, sparse
  accessories, and shape/proportion differences that remain readable without ornament.
- Verification: pending a replacement board containing humans, nonhuman species, and
  creatures across genuinely distinct simplified visual languages.

### Expected Blender failure aborted the PowerShell harness

- Cause: machine.
- Evidence: the candidate harness expected the deliberately invalid detail contract to
  return nonzero, but Windows PowerShell promoted Blender's stderr inside the nested strict
  runner and aborted the parent before it could inspect `$LASTEXITCODE`.
- Proposed correction: keep the ordinary build runner strict; execute only the expected
  failure in a separate PowerShell process under a locally tolerant error-action scope.
- Verification: the invalid contract is rejected and preserves prior valid output; the
  ordinary repeated build passes with deterministic `1280/320/80` geometry evidence.

### Semicolon-separated Unreal test requests skipped the second suite

- Cause: execution.
- Evidence: UE 5.8 accepted `Automation RunTests Gaters.Worldgen.ContentCells` but logged
  the following semicolon-separated `Automation RunTests Gaters.Runtime.VisualMaterializer`
  as an unknown automation command, so the intended second regression test did not run.
- Proposed correction: invoke each focused automation filter in its own headless Unreal
  process, or use one shared parent filter when a suitable filter exists.
- Verification: separate focused runs completed with content cells `4/4` and visual
  materializer `1/1`; the ordinary full `Gaters` filter completed `52/52`.

### Runtime fit exposed untyped support contacts

- Cause: machine.
- Evidence: seed 7 evaluated 171 selected compiled asset contracts and recorded 133
  initial contact issues. All 33 tree placeholders were buried, while walls, ceilings,
  fences, and foundations shared the same generic `ground` contact and were therefore
  all compared to terrain; 53 fence contacts alone were classified buried.
- Proposed correction: retain the observer and selected asset IDs, then make support
  intent explicit as terrain versus attachment before physical-fit becomes a promotion
  gate. Collision/navigation evidence remains an independent later adapter.
- Verification: compiled-world and asset-identity fixtures pass, the runtime failure log
  names exact recipe and selected asset IDs, and the existing full suite remains green.

### Motion reimport compounded root-motion scale

- Cause: execution.
- Evidence: the first motion import produced the intended root samples
  `0 / 50 / 100 cm`, while replacing that Anim Sequence reapplied the 100x FBX animation
  unit scale and produced `0 / 5000 / 10000 cm`. The initial repeatability harness compared
  paths, duration, keys, and bones but omitted the nested root-sample evidence, so those
  selected fields passed despite a mechanically different clip.
- Proposed correction: repeated asset-import harnesses must compare the complete stable
  semantic evidence projection, including transforms and hierarchy, rather than a handpicked
  scalar subset. Derived animations should be freshly imported when Unreal reimport mutates
  source-unit interpretation.
- Verification: the final two-pass harness compares hierarchy, bounds, timing, root samples,
  contact evidence, and settings; both imports report `0 / 50 / 100 cm`, five required bones,
  `44 x 53 x 123 cm`, one second, and 31 keys.

### Cross-chat requests were split across status files

- Cause: skill.
- Evidence: the first workstream protocol put a request in the requester's status file
  and its answer in the recipient's status file. Reading one exchange required
  reconstructing meaning and status across two independently owned files, and bootstrap
  requests appeared to have been authored by specialist chats that had not run.
- Proposed correction: co-locate request, response, evidence, notification, and
  resolution in one phase-owned exchange packet; keep workstream files as links and
  current objectives only.
- Verification: the shared-doc validator requires the packet template and one `Exchanges`
  section per workstream; a real Base-to-Combat exchange remains the end-to-end check.
