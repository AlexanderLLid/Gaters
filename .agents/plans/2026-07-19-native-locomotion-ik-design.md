# Native Humanoid Locomotion and IK Design

Status: approved by Human.

## Outcome

- The generated humanoid runs a deterministic headless route in UE 5.8:
  `idle -> walk -> run -> turn -> stop -> jump -> fall -> land`.
- The capsule and native `CharacterMovement` own world motion, floor state, falling, and
  landing.
- A generated IK Rig contains a real FullBodyIK solver. Native evaluation moves each foot
  goal and proves the corresponding solved foot pose changes.
- Repeated runs emit identical stable reports and causal failures without manual Unreal
  setup.

This is mechanical evidence only. It does not select animation style or satisfy `ART-1`
or `ART-2`.

If this existed, we would no longer need to manually configure or inspect baseline
locomotion and IK behavior for each compatible generated humanoid.

## Scope

- Generate or import reusable in-place idle, turn, walk, run, stop, jump, fall, and land
  clips from the existing mechanical Blender source.
- Extend the editor/cook-time adapter to configure and validate FullBodyIK.
- Add a separate isolated runtime challenger plugin that consumes cooked asset
  references, versions, and provenance.
- Evaluate flat-ground locomotion and direct IK goal response headlessly before adding
  playable input.
- Defer uneven terrain, foot placement, Motion Warping, reactions, physics-driven falls,
  recovery, replication, performance, controlled proportions, quadrupeds, species,
  lineage, evolution, and art.

## Native machinery and boundary

- **Borrow:** UE 5.8 `ACharacter` and `CharacterMovement` for capsule motion, floor
  finding, walking, falling, and landing.
- **Borrow:** stable UE 5.8 IKRig/FullBodyIK and `FIKRigProcessor` for solver execution.
- **Adapt:** the existing editor-only profile adapter configures the generated IK Rig but
  remains excluded from gameplay runtime.
- **Build:** one isolated runtime challenger plugin supplies the scripted driver and
  measurements; it is evidence infrastructure, not shared gameplay code.
- The runtime challenger consumes only cooked Skeletal Mesh, Skeleton, Physics Asset,
  IK Rig references, source/adapter versions, and provenance. It never consumes Blender,
  Python, mutable editor objects, or the editor-only adapter.
- Shared Unreal modules, `Prototype.uproject`, and `research/machines.json` remain owned
  by Primary Builder and unchanged by this experiment.

## Data flow

1. The versioned mechanical recipe enters the existing Blender generator.
2. Blender emits the humanoid, skeleton, skinning, physical profile, FBX, and named
   in-place mechanical clips.
3. The editor adapter imports the cooked assets, configures FullBodyIK, and prepares the
   derived CharacterLab map.
4. A native IK evaluator displaces left and right goals separately and records input and
   solved foot transforms.
5. The isolated runtime challenger spawns a native character on the flat test floor and
   drives the required route through CharacterMovement.
6. The independent policy evaluates native readback and runtime measurements. Generator
   assertions cannot satisfy a rule.

## Runtime contract

- The capsule owns world translation and grounded/falling state; locomotion clips remain
  in-place and cannot move the Actor root.
- The scripted route must observe idle, walking, running, turning, stopping, falling, and
  landing in order.
- The report records capsule trajectory, velocity, acceleration, facing, movement mode,
  floor state, active clip/state, and final transform at fixed samples.
- The FullBodyIK solver uses pelvis as its root and connects both generated foot goals.
- Moving one foot goal must move the corresponding solved foot toward the goal within a
  named data tolerance while the opposite foot remains within a separate bound.
- A missing clip, asset, solver, goal connection, floor, movement state, or expected pose
  response fails with a stable rule ID and causal diagnostic.

## Independent evidence

- Native automation verifies solver presence, goal connections, initialization, and
  evaluated pose response using `FIKRigProcessor`.
- Runtime automation verifies CharacterMovement owns the capsule, discovers the floor,
  traverses the scripted route, enters falling, and lands.
- The public command rebuilds source artifacts, packages both isolated plugins, imports
  twice, runs native/runtime tests, applies the independent policy, and compares stable
  reports byte-for-byte.
- The prior topology report and malformed-profile counterexample remain regression gates.
- Failure preserves logs and reports; the passing topology champion remains valid.

## Approaches rejected

- Generated Blueprint-only runtime test: smaller initial setup but brittle to editor API
  changes and weaker as an independent native verifier.
- Loading the editor adapter during gameplay: violates the accepted cooked-asset runtime
  boundary and brings `PhysicsUtilities`/`UnrealEd` into runtime.
- Animation Warping Foot Placement now: it solves the later uneven-terrain contact problem,
  not this flat-ground movement and solver-evaluation gate.
- Experimental Mover or custom movement/IK: duplicates stable native foundations.
- Playable controls first: adds input and presentation variables before mechanics pass.

## Capability graph

| Node | Unlocks | Relation | Source | Contract | Work deleted | Verifier |
|---|---|---|---|---|---|---|
| Cooked humanoid intake | IK and movement evaluation | SEQUENCE | Adapt, passing | Stable cooked assets plus versions/provenance | Manual asset setup | Existing intake policy |
| FullBodyIK configuration | Native pose solve | SEQUENCE | Adapt | Pelvis root and connected foot goals | Manual IK Rig setup | Native solver automation |
| Evaluated IK response | Terrain-contact wave | AND | Borrow | Goal displacement changes expected solved foot | Visual solver inspection | `FIKRigProcessor` measurements |
| Scripted CharacterMovement | Runtime state evidence | AND | Borrow | Ordered flat-ground route and native movement modes | Manual playthrough | Runtime automation |
| Stable locomotion/IK report | Later terrain and recovery gates | SEQUENCE | Build | Repeatable independent evidence | Manual diagnosis | Independent policy |

Selected frontier: FullBodyIK evaluation first, then CharacterMovement. It retires the
remaining wave-three uncertainty before movement-state automation and unlocks the later
terrain-contact machine.

## Requirements

- `CHAR-1`: advances locomotion and real IK only; uneven terrain, impact, fall recovery,
  and return to locomotion remain open.
- `CHAR-2`: Blender remains offline authority; native Unreal systems own live behavior;
  the editor adapter stays outside runtime.
- `CHAR-3`: controlled family variation remains deferred until the complete humanoid
  vertical slice passes.
- `ART-1`, `ART-2`: outside this mechanical placeholder; no art promotion is claimed.

Requirements checked: Global none recorded; `ART-1`, `ART-2`, `CHAR-1`, `CHAR-2`,
`CHAR-3`, generated-content boundary. Exceptions: none.
