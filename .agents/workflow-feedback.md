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

### Next-machine selection followed focus instead of the critical path

- Cause: execution.
- Evidence: the registry placed mechanical motion in wave 4 while environment, village,
  and base feasibility remained unproven in waves 2–3. The Primary Builder followed the
  global motion focus and proposed melee reach/penetration work even though ordinary
  attacks are a known implementation problem and do not retire the project's current
  world-generation risk.
- Proposed correction: before proposing a next implementation, compare candidates by
  downstream unlocks, unresolved feasibility risk, and skipped earlier-wave blockers. A
  later-wave known-known cannot displace an earlier load-bearing unknown without an
  explicit human priority or evidence that the spike could invalidate the architecture.
- Verification: rerunning this choice selects the placeholder settlement generator and
  evaluator over the melee fixture; an ordinary later-wave risk spike remains eligible
  only when its recorded rationale names the architecture it could invalidate.

### Shared Unreal build collided with the interactive editor

- Cause: skill.
- Evidence: an agent-launched UnrealBuildTool `dotnet.exe` ran as
  `CodexSandboxOffline` while the human had Unreal Editor open. Windows showed an
  application-error dialog followed by an attach-security warning. The workflow had no
  mandatory editor preflight or human confirmation before process/dialog actions.
- Proposed correction: `AGENTS.md` now requires every agent to check for an interactive
  Unreal Editor before building and to stop and ask the human before closing, dismissing,
  attaching to, or terminating anything after a build/process/dialog conflict.
- Verification: the shared-doc validator passed; editor and build-process preflights
  reported neither process running; the next ordinary PrototypeEditor build completed
  successfully without a dialog. The original editor-present case is not intentionally
  reproduced because it creates the unsafe dialog this rule prevents.

### Art-direction variants preserved the rejected fantasy baseline

- Cause: execution.
- Evidence: the user requested distinct mature art directions and characterful varied
  faces, but the first variants changed props and rendering while retaining medieval
  clothing, village silhouettes, heraldic dragons, and the brown fantasy palette. The
  supposed alternatives therefore read as one generic fantasy direction.
- Proposed correction: separate art treatment from diegetic motifs. When the user asks
  to move away from a genre baseline, regenerate contemporary/nonhistorical clothing,
  architecture, creature anatomy, palette, and material language from scratch; lock
  diverse natural face casting as an independent requirement.
- Verification: three independent boards now use contemporary rural, graphic-mineral,
  and industrial-minimal baselines. The rejected earthen attempt was withheld after it
  reintroduced robe-like clothing and an armored dragon; user acceptance remains pending.

### Matched fantasy boards collapsed into prestige concept art

- Cause: execution.
- Evidence: prompts requested three different production styles but combined a dense
  subject list with "professional concept board" presentation. All first-pass outputs
  used detailed painterly rendering, so palette and mood carried more difference than
  geometry or material language.
- Proposed correction: describe outputs as production-ready stylized 3D, impose explicit
  garment, texture, scale, and building-module budgets, and assign each direction a
  different geometry, surface, and value rule while holding fantasy content constant.
- Verification: the refined pass produced separate painted-mid-poly, carved-folklore,
  and graphic-shadow boards with the same human, dragon, clothing function, and fantasy
  settlement baseline; user acceptance remains pending.

### Nonhuman concept prompts collapsed into familiar genre species

- Cause: execution.
- Evidence: anatomy prompts intended to produce original volcanic and wetland humanoids
  returned a conventional orc and a green pointed-ear elf; the first wetland correction
  overcompensated into a grey-alien head. Settlement, clothing, palette, and the selected
  painted-mid-poly style were otherwise preserved.
- Proposed correction: define nonhuman heads through positive topology and proportion,
  explicitly name the nearest fantasy and science-fiction tropes to reject, retain human
  skeleton roles separately, and use a targeted identity edit so successful environment
  and clothing work remains invariant.
- Verification: the volcanic edit removes tusks, pointed ears, oversized muscles, and
  orc facial structure; the final wetland edit restores human-scale eyes, nose, jaw, ears,
  and hair while retaining restrained amphibious traits. The ordinary human highland
  board remained unchanged; user acceptance remains pending.

### Primitive face assembly passed mechanically but failed the art requirement

- Cause: machine.
- Evidence: the first isolated Blender face proof passed artifact, material, and view
  checks but rendered oversized separate eyes, lips, ears, brows, and collar, producing
  the childlike toy look forbidden by `ART-1`. A front render alone also hid detached
  profile details.
- Proposed correction: treat fixed front, three-quarter, and profile renders as an art
  gate; use human-scale proportions, integrated head masses, and surface-projected facial
  landmarks before accepting a generated bust as style evidence.
- Verification: the corrected proof regenerates and reopens all three views with 14
  meshes and 12,634 vertices. It removes the toy proportions and floating crease wires;
  profile topology remains below concept-A fidelity and is recorded as the next art gap,
  not promoted as production evidence.

### Parametric face proof passed structure before presentation

- Cause: machine.
- Evidence: the first MPFB challenger passed stable-topology, named-control, reopen, and
  three-view checks but rendered at `0.890` average luma with occluded irises and a scalp
  material extending to the eyes. A later procedural-surface pass remained mannequin-like.
- Proposed correction: no skill change. Reuse the official CC0 MakeHuman skin, eyes,
  brows, and hair behind the recipe adapter; retain fixed-view inspection and human art
  approval as the promotion gate.
- Verification: the current challenger regenerates with 24 named controls, packed source
  assets, three fixed views, `0.292` average luma, and `0.433` sampled skin luma. Mechanical
  checks pass; human acceptance remains pending.

### Archipelago tests accepted exposed circular generator stamps

- Cause: machine.
- Evidence: seed `131` passed determinism, water, topology, base-site, traversal, and
  gallery-presence checks while its islands and upland contours visibly exposed circular
  distance masks. The first general morphology measurement scored `0.931` radial
  circularity.
- Proposed correction: no skill change. Add pure multi-contour morphology evidence and
  retain human fixed-camera acceptance as a separate promotion gate; gameplay validity
  must not stand in for naturalness.
- Verification: the synthetic morphology contract and held-out seed-`131` regression pass;
  its maximum circularity falls to `0.800`, the Environment suite preserves 28 distinct
  coastline signatures and four-island layouts, and human acceptance remains pending.

### Front projection was presented as a Blender face proof

- Cause: execution.
- Evidence: the still closely matched the approved painting only because that painting
  was camera-projected onto an existing head; it did not test whether the face had been
  recreated as view-independent geometry.
- Proposed correction: no skill change. A face-reproduction claim now requires plain-clay
  front, three-quarter, and profile evidence with the reference texture absent.
- Verification: `Derived/sculpt-v13/face-sculpt.blend` reopened in a separate Blender
  process and rerendered 768px front, three-quarter, and profile evidence. Visible
  materials contain no image textures; the editable `SculptDirect` shape key is active;
  no armature or animation exists. Human art acceptance remains pending.

### Every task asked the human about Unreal ownership

- Cause: skill.
- Evidence: the prior global Unreal rule required every task to inspect Unreal activity
  and ask the human whenever the Editor or build processes were present. Concurrent
  Unreal Editor and multiple Epic `dotnet` build processes made ownership unclear and
  generated repeated prompts.
- Proposed correction: `Unreal Runner` alone launches or observes shared Unreal work;
  all other tasks send one explicit run request and continue non-Unreal work.
- Verification: the runner must report one active requester, command, PID, and queue;
  non-runner tasks must no longer issue Unreal ownership prompts.

### Face verifier test cleanup escaped its temporary run

- Cause: execution.
- Evidence: a unit-test teardown used `self.output.parents[2]`, which resolved to the
  shared `Derived/search-runs/` directory and deleted the task's v1/v2/v3 regenerable
  Blender outputs. Authoritative recipes and process records under `Runs/` survived, but
  their deleted artifact references can no longer serve as valid evidence.
- Proposed correction: cleanup targets must be one unique temp-run directory, assert that
  exact resolved boundary before deletion, preserve an append-only artifact-loss record,
  and regenerate final proof under a new run identity without rewriting lost history.
- Verification: the cleanup-boundary regression passes; the append-only incident rejects
  every invalidated v1/v2/v3 run; both v4 candidates pass recipe, artifact-hash, and
  independent fresh-open Blender verification. Empty direct-child temp directories are
  non-evidence and cannot pass the acceptance gate.

### Face sculpt search dispatched the visual champion as generator source

- Cause: execution.
- Evidence: the first real search candidate was built from `Derived/sculpt-v13` even
  though the integrity contract pins `Derived/face-v2/face-proof.blend`. The sculpt step
  emitted success, but fresh verification rejected the source path. Blender still
  returned exit code zero after the Python exception because the dispatch omitted
  `--python-exit-code`; the required `SCULPT_VERIFY_OK` marker prevented acceptance.
- Proposed correction: derive the source-blend path from the integrity contract, add
  `--python-exit-code 1` to every Blender command, require both exit zero and the expected
  marker, preserve the failed run, and restart with a new run identity.
- Verification: both corrected round-1 candidates built from the contract-pinned source
  with `--python-exit-code 1`, emitted both success markers, passed current manifest
  integrity, and produced root-owned verifier receipts bound to candidate, manifest,
  verifier script, and preserved log hashes.

### Accessible-land modes were proposed before tracing the existing machine graph

- Cause: execution.
- Evidence: after volcanic landform runs collapsed traversal coverage, Primary proposed
  seed-defined `minimal`, `sparse`, and `broad` accessible-land modes before checking the
  verified Environment Brief Compiler, Traversability Evaluator, Environment Recipe
  Compiler, and top-level Environment Generator contracts. The proposal described an
  input preset, not the missing capability that satisfies and independently verifies a
  target without terrain-family branches.
- Proposed correction: no skill change. Before recommending a new magic machine, execute
  the existing inventory and upstream-knob steps: name the reused machines, the precise
  guarantee they cannot provide, the smallest missing machine, and complete “If this
  existed, we would no longer need to ___.” Presets may compile into a contract but are
  never themselves the generator architecture.
- Verification: re-running the original case identifies a deterministic candidate
  selector between landform generation and the independent traversability evaluator;
  land-access ranges belong in the existing environment brief. As an ordinary control,
  a forestless request needs no new machine because the existing seed intent and content
  cell contracts already express and verify zero vegetation.

### Workflow feedback entries had no required consumer

- Cause: skill.
- Evidence: `improving-workflows` required appending failures but did not require any
  agent to search related entries afterward. The log could therefore preserve evidence
  without influencing classification or correction in a later run.
- Proposed correction: every new entry immediately triggers exactly one bounded,
  read-only reviewer agent. It searches related feedback, classifies the cause, proposes
  the smallest correction and verification, and returns ownership to the originating
  task without recursively logging or dispatching the same failure.
- Verification: this entry triggered one independent coordinator, which returned the
  same bounded design and no edits. Both shared skill mirrors contain the new step;
  mirror and formatting validation follow this edit.

### UI and code were allowed separate names

- Cause: skill.
- Evidence: the spellcraft discussion used `Trait` as the player-facing term while
  retaining `part` as a possible internal term; the user had to require one name for the
  same concept everywhere.
- Proposed correction: `AGENTS.md` and `greybox` require one canonical noun across UI,
  canon, data, and code. Syntax, namespaces, and localization may adapt that noun but may
  not create a semantic alias.
- Verification: the original spellcraft case contains `Trait` throughout its current
  canon and workstream record with no stale spell-part term; the ordinary shared-agent-doc
  validator passes with byte-identical skill mirrors.

### Held-out face packages leaked candidate chronology through paths

- Cause: execution.
- Evidence: the first final-audit packages exposed source path tokens such as
  `sculpt-v13`, `round-0`, and later-round names. The independent reviewer correctly
  invalidated those ballots even though their outcome was favorable.
- Proposed correction: evaluator-visible packages contain only neutral, slot-local image
  copies; candidate identity and chronology exist only in a root-owned hidden slot map.
  A regression test rejects source identities and chronology tokens in visible packages.
- Verification: the leaky packages remain preserved under `invalid-leaky-*`; corrected
  opposite-order packages were rebuilt and independently rerun, the focused suite passes
  55 tests, and the independent Task 5 reviewer approved the corrected evidence.

### Face metric compared different anatomical meanings

- Cause: execution.
- Evidence: macro run `064000` compared full visible cheek/jaw extrema annotated on the
  reference with extrema of internal MPFB control regions. Every candidate consequently
  measured the jaw wider than the cheek and the score rewarded more taper, contradicting
  both the target ratio and visible diagnosis. Full-region mouth extrema similarly
  measured about `1.6` eye distances instead of an anatomical mouth-corner span.
- Proposed correction: every metric anchor declares one anatomical meaning shared by the
  reference and mesh. Directional sanity fixtures must pass before a real run: a narrower
  jaw yields `jawWidth / cheekWidth < 1`, extra taper moves away from the frozen target,
  mouth corners stay within the declared anatomical eye-distance range, and hidden helper
  geometry cannot become a contour.
- Verification: `064000` remains append-only and invalid for promotion. Corrected run
  `071000` uses 13,380 body vertices, excludes 1,030 ear vertices, uses target-matched
  contour bands and signed-displacement mouth corners, passes 63 focused tests and eight
  fresh integrity checks, and received independent Task 7 approval.

### Unreal Runner could not be awakened for a queued verification

- Cause: machine.
- Evidence: `codex_app__read_thread` reports the mandated Unreal Runner task
  `019f7c9d-e7c3-7970-844e-ef2640d5a411` as idle and readable, but two ordinary
  `codex_app__send_message_to_thread` calls remained pending until terminated and one
  delegation-wrapped call returned `no active turn to steer`. A follow-up read showed no
  new Runner turn. The owning task correctly did not bypass the Runner by launching
  Unreal locally.
- Proposed correction: no skill change. Repair or provide a supported idle-task wake;
  meanwhile treat confirmed task messaging failure as unavailable and use the existing
  one-time human relay path without bypassing Runner ownership.
- Verification: the original focused selector request starts exactly once from an idle
  Runner and reports requester, command, PID, and queue; an ordinary read-only task
  remains unable to launch Unreal itself.

### Face feature evaluation entered execution before its annotation and runtime semantics were frozen

- Cause: execution.
- Evidence: feature runs `090000` through `093000` exposed, in order, incorrect lid/root
  landmarks, re-keyed blind reports that did not match their sources, and Euclidean
  normalization inconsistent with the evaluator's horizontal eye distance. Clean run
  `094000` then preserved a first feature-metric failure where a local `scene_lock`
  variable shadowed the imported function. None of these attempts produced an accepted
  selection.
- Proposed correction: preserve each failed run; store both blind annotations verbatim;
  derive consensus, uncertainty, and ratios from those exact points with tests using the
  evaluator's normalization; execute one fresh Blender metric smoke receipt before the
  full measurement dispatch; require a durable selection record and receipt-derived
  macro tolerance before review.
- Verification: `094000` uses the exact source annotations and horizontal-IPD math,
  passes 76 focused tests, eight source-pinned builds, eight fresh reopens, eight feature
  and macro receipts, exact Task 7 replay, and one static scene lock. The failed shadowing
  log remains preserved beside a successful rerun. Independent review first rejected
  missing selection/tolerance evidence, then approved the corrected retained decision.

### Maximum mouth-target displacement was mistaken for a visible commissure

- Cause: execution.
- Evidence: Task 7's second macro evaluator called the strongest signed vertices in
  `mouth-scale-horiz-incr` mouth corners. In the fixed 768px front they project near
  x=`303.5` and `464.5`; two later blind annotations independently place the visible
  commissures at x=`327–328` and `440–441`. The stored `mouthWidth` therefore describes
  a wider control region, not the same visible feature annotated on the painting.
- Proposed correction: for visible landmarks, map independent manual candidate points
  once to stable frontmost body vertices and freeze those indices. A target-control file
  may propose a region but cannot name the semantic vertex by displacement magnitude.
  Exclude the invalid field from all later selection and macro gates.
- Verification: the Task 7 aggregate nomination is withdrawn while its unaffected
  contour receipts remain scoped evidence. Task 10 preserves both blind annotations,
  freezes visible commissure indices, measures them after fresh reopen, and receives one
  independent review before any review-only candidate is recorded.

### Detail absolute controls were stacked as duplicate Blender targets

- Cause: machine.
- Evidence: Task 10 run `detail-grid-20260720-062133` declared `mouth.width` and
  `nose.pointDown` as absolute values, but `sculpt_details.py` loaded new targets on a
  source blend that already contained those shape keys. Blender created active
  `mouth.width.001` and `nose.pointDown.001` copies, so the values stacked and the
  successful geometry receipts did not represent their recipes.
- Proposed correction: assign absolute controls to the existing shape keys, keep only
  genuinely additive controls as loaded targets, and make fresh-open integrity reject
  any active numbered copy of either absolute key. Preserve and invalidate the affected
  run before rebuilding under a new run identity.
- Verification: the pure replacement/duplicate regression and the full 86-test suite
  pass. Original-case rebuild, fresh reopen, receipt generation, and an ordinary Task 9
  calibration integrity check remain to be rerun.

### Detail absolute-control correction verified

- Coordinator: `/root`. Cause: machine.
- Evidence: replacement run `detail-grid-20260720-063059` has one active base key for
  each absolute control, no numbered duplicates, and eight complete fresh-open receipt
  sets. The bounded reviewer classified the implementation patch as sufficient and
  recommended no skill change.
- Proposed correction: none beyond the implemented direct assignment and duplicate gate.
- Verification: the original eight-cell case passes build, fresh reopen, three metrics,
  and fixed-view checks; the ordinary Task 9 calibration candidate still freshly reopens;
  the full 87-test suite passes.

### Head silhouette score followed the neck instead of the chin

- Coordinator: `/root`. Cause: execution.
- Evidence: `Derived/head-silhouette-v3/verification.json` reported `94.29%` profile
  improvement and `promote-head-silhouette-fit`, while the eye-aligned 50/50 layer shows
  a wider rectangular skull and broader, shorter lower face than the target. Below the
  jaw, raster row extrema follow the connected neck boundary rather than the jaw-to-chin
  contour; one mean score also averages away regional skull and temple errors.
- Proposed correction: make eye/IPD-aligned head crops and their 50/50 layer mandatory
  evidence; score only outer-boundary regions with valid semantics; report skull,
  temple, ear, jaw/chin, and overall decisions separately; mechanical checks cannot
  promote a head without the visual decision.
- Verification: rerun the original candidate and one ordinary candidate. Both must emit
  aligned evidence, exclude the invalid lower region from automatic scoring, and remain
  non-promoted until every required visual region passes.
- Bounded review: the verifier reused the builder's faulty row-extrema assumption,
  averaged anatomical regions, promoted while visual review was pending, and left the
  diagnostic overlay outside the gate. This repeats the earlier anatomical-meaning
  execution failure; no skill change is required.
- Verification result: original-case replay `head-silhouette-v4` emits the three aligned
  crops, excludes `jaw-chin-neck`, reports three automatic regions, and ends
  `visual-reject` with `promotionEligible=false`. The ordinary temporary contract run
  emits the same evidence and ends `mechanical-pass-awaiting-visual-review`.

### Half-opacity head comparison blurred the evidence

- Coordinator: `/root`. Cause: skill.
- Evidence: `c-layered-head-50.png` averages two materially different full-detail faces.
  Misaligned eyes, nose, mouth, texture, and shading become double images, so the user
  cannot judge the head boundary clearly even though eye/IPD alignment is correct.
- Proposed correction: retain the aligned source crops but replace the 50% blend as the
  primary evidence with a full-opacity centre wipe and a labelled full-opacity blink.
  Color identifies source; alpha must not average the comparison images.
- Verification: the original pair produces sharp target/generated edges in both outputs;
  an ordinary identical-image pair remains visually stationary across the wipe/blink.
- Bounded review: the implementation faithfully followed the earlier required 50/50
  layer, but `finding-magic-machines` lacked an evidence-legibility criterion. Its
  verification step now requires full-opacity wipe/blink evidence for spatial judgments;
  alpha averages are diagnostic only.
- Verification result: the original pair now emits a centre wipe whose left/right pixels
  exactly match the aligned sources and a two-frame full-opacity blink. The fresh ordinary
  contract run passes; the current reviewed candidate remains `visual-reject`.

### Falsified specialist guarantee did not immediately reach the machine registry owner

- Coordinator: `/root`. Cause: execution.
- Evidence: RAID-5 independently falsified the settlement generator's one-surviving-route
  acceptance rule. The originating task recorded the defect in the exchange and specialist
  workstream, then stopped at a human implementation-design gate. It did not inspect
  `research/machines.json` or raise a registry `INTEGRATE` exchange until the human asked
  whether the finding had entered the magic machine. The registry therefore remained the
  stale current authority even though coordination evidence already knew the guarantee was
  false.
- Bounded review: existing rules already make `research/machines.json` authoritative,
  require specialists to route shared registry changes through `INTEGRATE`, and make
  builder closeout depend on recording a falsified guarantee plus its next experiment.
  Related prior feedback also classified ignoring registry authority as execution. No
  skill edit is justified.
- Proposed correction: at capability closeout, compare each verified or falsified result
  with its registered node. If the result changes a guarantee, evidence, gate, status,
  dependency, champion, or next experiment, integrate it or immediately send one
  registry-owner `INTEGRATE` packet. Local defects that do not change registered machine
  truth remain local.
- Verification result: the original RAID-5 replay created SITE-7 before implementation
  closeout. Direct delivery to the idle Primary task returned `no active turn to steer`,
  so the router's human-relay fallback and registry integration remain pending. As the
  ordinary control, WORLD-2 was a malformed counterexample fixture with unchanged
  production validation and correctly resolved locally without a registry exchange.

### Canonical Unreal Runner transport had no registered message handler

- Coordinator: `/root`. Cause: machine.
- Evidence: after the canonical Runner successfully completed selector-v4 focused
  Automation, the next ordinary `codex_app__send_message_to_thread` request returned
  `No handler registered for tool: send_message_to_thread`. Earlier delivery to the same
  canonical task succeeded, and the owning task correctly did not launch Unreal itself.
  This repeats the transport class recorded in `Unreal Runner could not be awakened for
  a queued verification`, but with a distinct immediate handler-registration failure.
- Bounded review: the workflow was followed, direct Runner delivery was attempted, and
  exclusive Unreal ownership was preserved. The missing registered handler is a repeated
  transport-machine failure, not a skill gap.
- Proposed correction: restore/register the canonical `send_message_to_thread` handler.
  No skill change. Use the documented one-time human relay fallback meanwhile.
- Verification: the full Gaters request reaches the canonical Runner exactly once, the
  Runner reports queue ownership, and an ordinary subsequent request still routes
  without direct Unreal execution by the requester.
- Verification result: the full request reached the canonical Runner exactly once only
  through human relay and completed `119/119`. The ordinary subsequent held-out sweep
  request again returned `No handler registered for tool: send_message_to_thread`.
  Exclusive Runner ownership was preserved; the transport correction is not verified.

### Synchronous Unreal sweep blocked Runner status and queue control

- Coordinator: `/root`. Cause: skill.
- Evidence: the canonical Runner accepted a 20-case landform sweep. Seed `11` shut down
  and produced archive record `1`; seed `29` logged valid `LAND_ACCESS`, `PLAN`, and
  `PERF` evidence at `2026-07-21 05:43:35Z` but never logged cleanup or exit. More than
  one hour later the archive remained `1/20`, Runner status reads and messages blocked,
  and the requester correctly did not inspect or terminate Unreal processes.
- Bounded review blocker: the delegated reviewer did not return after repeated bounded
  waits and an explicit stop request, so it was interrupted. Local fallback found the
  Runner contract has no failure path for a non-exiting child: it can follow every stated
  rule yet become unable to report status or serialize recovery.
- Proposed correction: add one Runner failure-path rule requiring externally bounded,
  asynchronously supervised child execution. On deadline, the Runner reports requester,
  command, PID, last evidence, and blocked queue without launching a duplicate or
  terminating the process. Preserve the incomplete archive and both per-seed logs; do not
  retry or alter terrain until Runner ownership is recoverable. No rule edit is authorized
  in this task.
- Verification: a deliberately non-exiting child cannot indefinitely hide Runner status;
  the Runner reports the causal timeout/stall and serializes the next ordinary run without
  duplicate Unreal execution.
- Verification result: after a human-authorized temporary Runner override, process
  inspection showed no Unreal, UBT, or Epic .NET process. Both direct Unreal launch and
  the first repository-script launch attempts were rejected by execution policy before
  creating a process, and the Runner read handler was unavailable. A minimal script-only
  invocation then reproduced an infrastructure stall: its 60-second ceiling expired
  while a cold sponsored Zen server was still starting. With a 180-second ceiling,
  seed `29` and ordinary seed `11` both exited and reproduced exact evidence; a fresh
  20-case sweep completed in 288 seconds with 20 records. The original child hang did
  not reproduce, but the missing externally supervised Runner failure path remains.

### Root field integration validated the wrong terrain authority

- Coordinator: `/root`. Cause: execution.
- Evidence: Climate Field and Drainage Network tests passed while both root adapters
  sampled `FGatersEnvironment::HeightAt`. Runtime environmental terrain is authoritative
  through `FGatersEnvironmentRecipe::QueryTerrain`, which applies regional intent. The
  tests asserted parity with the same raw helper used by the implementation, so they
  could not detect disagreement inside regional terrain areas.
- Bounded review: `finding-magic-machines` already requires an independent verifier and
  rejects self-judging tests; `AGENTS.md` and `FGatersEnvironmentRecipe` already identify
  the composed root as authoritative. No skill change is justified.
- Proposed correction: give coordinate fields one external height-sampling seam, make root adapters pass
  `QueryTerrain(...).Height`, and require a regional point where raw and root terrain
  differ.
- Verification: the original regional counterexample must fail before correction and
  pass afterward; an ordinary global/raw Climate Field query must remain unchanged.
- Verification result: the root fixture proves at least one regional point differs from
  raw terrain, then requires Climate height and every Drainage cell to equal
  `QueryTerrain(...).Height`. The raw Environment Climate adapter remains green. Focused
  Environment Recipe, Climate, and Drainage pass `9/9`; the broader dependency suite
  passes `16/16`; the complete Gaters suite passes `128/128`; a fresh selected-landform
  seed-`83` runtime reports a valid climate root and valid generation/performance evidence.

### Regional water fit exposed an uncomposed terrain verifier

- Coordinator: `/root`. Cause: execution.
- Evidence: the new pure drainage-to-water fit accepted its synthetic downstream-water
  fixture and causal corruptions, then rejected held-out roots. Seed `11` surface
  `water:intent:11:region:2:1` and both seed `29` region-1 lake surfaces had no drainage
  cell below their declared datum. The earlier regional-water evaluator passed by
  sampling `Environment.WithProfile(...)` directly; runtime terrain instead comes from
  `EnvironmentRecipe.QueryTerrain`, which blends regional intent. Reproduction:
  `Gaters.Worldgen.DrainageNetwork.RealRegionalWaterFit` in
  `Unreal/Prototype/Saved/Logs/DrainageRealRegionalWaterFitChallenge.log`.
- Bounded review: `finding-magic-machines` already rejects self-judging and requires an
  independent verifier. `evaluation.regional-water-fit` declares an intent-terrain
  dependency, but its implementation and generator sampled the same unblended regional
  profile. This repeats the earlier root-authority execution failure; no skill change is
  justified.
- Proposed correction: inject an authoritative height sampler into the existing
  physical-fit evaluator and make root validation pass `QueryTerrain`. Keep seeds `11`
  and `29` as permanent counterexamples, do not weaken datum tolerance or drainage fit,
  and correct regional-water generation separately if composed verification rejects it.
- Verification: the held-out seed `11` and `29` surfaces intersect composed terrain below
  datum, an ordinary valid synthetic fit remains green, and the complete suite remains
  green without weakening the fit evaluator.
- Verification result: the injected-height corruption rejects elevated composed terrain.
  Regional lake geometry now scales to and is shared within its declared region. Seeds
  `11`, `29`, `47`, `83`, and `131` validate and fit at the first sufficient 65-by-65
  evidence resolution; the original synthetic fit remains green. Worldgen passes `93/93`,
  the complete Gaters suite passes `131/131`, and both repository validators pass. No
  datum tolerance was changed.

### Root drainage composition reused a local-scale evidence resolution

- Coordinator: `/root`. Cause: execution.
- Evidence: the first Environment Recipe v3 composition reused the 65-by-65 drainage
  grid proven on 120000-unit roots. At the existing 400000-unit Environment Recipe
  contract, seed `0` lost sampled support for one declared lake, seed `2` lost three,
  and selected-landform seed `83` lost one. The independent regional-water evaluator
  still found authoritative submerged terrain. Reproduction:
  `Unreal/Prototype/Saved/Logs/EnvironmentDrainageRootV3Diagnostic.log`.
- Bounded review: `finding-magic-machines` already requires representative scale and
  counterexamples. The plan incorrectly promoted one tested resolution across a larger
  root scale; no new skill rule is justified.
- Proposed correction: keep drainage resolution recorded in the recipe, challenge both
  runtime-scale and existing root-scale fixtures, and choose the first bounded global
  grid that supports both without changing water datum tolerance. Hierarchical/local
  drainage remains a separate future scale challenger.
- Verification: seeds `0`, `2`, `4`, `7`, `53`, selected-landform `83`, and held-out
  seeds `11`, `29`, `47`, `83`, and `131` validate through one composed root; focused,
  Worldgen, and complete automation remain green.
- Verification result: the 129-by-129 challenger passes all three focused Environment
  Recipe tests and all nine isolated Drainage Network tests without changing water datum
  tolerance. The rebuilt Worldgen suite is `96/97` and complete Gaters is `132/135`;
  every Environment/Drainage test passes, while three newly added active
  Settlements-owned RED tests fail on facade/path/access contracts.
