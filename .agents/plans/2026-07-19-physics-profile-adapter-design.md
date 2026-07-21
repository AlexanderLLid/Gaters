# Physics Profile to Physics Asset Adapter Design

Status: approved by Human; implementation review pending.

## Outcome

- The generated humanoid's `physical-profile.json` produces a native UE 5.8 Physics
  Asset with exactly the profile's body bones and parent/child constraint pairs.
- Existing Skeletal Mesh, hierarchy, scale, and IK Rig intake remain unchanged.
- The experiment stays isolated from shared Unreal source and `Prototype.uproject`.

If this existed, we would no longer need to manually add missing Physics Asset bodies
and constraints for each generated character.

## Selected approach

- Build a small external Unreal Editor plugin from source under
  `research/embodied-species-lab/unreal/CharacterPhysicsProfileAdapter/`.
- Package it into ignored lab-derived output and load it only for the challenger through
  Unreal's `-PLUGIN=<descriptor>` command-line support.
- Expose one editor-only Blueprint/Python function that receives the imported Skeletal
  Mesh, Physics Asset, required body-bone names, and parallel parent/child joint arrays.
- Use native `FPhysicsAssetUtils` for collision fitting, body creation/removal, and
  constraint creation. The adapter does not implement physics or collision solving.

Rejected:

- Blender/FBX tuning: Unreal's default generation threshold cannot guarantee the profile
  and does not carry Blender rigid-body semantics through FBX.
- Shared Prototype module code: this is eventual integration territory owned by Primary
  Builder and remains pending `CHAR-1`.

## Data flow

1. The existing Blender builder produces the humanoid FBX, manifest, and physical profile.
2. The existing Unreal Python importer imports the Skeletal Mesh and creates the native
   Physics Asset and IK Rig.
3. Python validates the profile and passes only required body names and joint pairs into
   the isolated native adapter.
4. The adapter rebuilds collision bodies with native fitting, removes bodies excluded by
   the profile, clears generated constraints, and creates the exact profile joint graph.
5. Python reads the native result back through supported Unreal APIs and writes the same
   schema-1 report.
6. The existing independent intake policy accepts or rejects the result.

## Native adapter contract

- Reject null assets, empty body sets, array-length mismatches, unknown bones, duplicate
  body names, duplicate joint pairs, joints referencing excluded bodies, and disconnected
  body graphs.
- Use `FPhysAssetCreateParams` with native primitive fitting and body creation forced for
  the source skeleton, then retain only profile-declared body bones.
- Recreate constraints using the profile's explicit parent/child pairs and snap their
  transforms to the skeletal reference pose.
- Mark and refresh the Physics Asset and Skeletal Mesh through native editor mechanisms.
- Return a failure message to Python; never silently repair or omit a requested part.

## Verification

- RED evidence: current deterministic UE report contains two connected body endpoints and
  one constraint; policy failures are `intake.physics.body_coverage` and
  `intake.physics.constraints`.
- Challenger: rebuild and import `impact-forward` twice through the packaged external
  plugin, compare stable reports, and run the unchanged independent policy after each run.
- Promotion gate: both imports report exactly fourteen body bones and thirteen source
  joint pairs, policy passes, logs contain no adapter errors, and existing humanoid,
  physical-humanoid, and recovery champions remain green.
- Counterexample: a profile joint that names an excluded or unknown bone must be rejected
  with a causal adapter error before the Physics Asset is saved.

## Deliberate limits

- This version proves body coverage and constraint topology only.
- Profile mass overrides, collision-shape fidelity, asymmetric joint frames/limits,
  springs, damping, runtime Chaos behavior, and uneven-ground recovery remain later
  experiments gated by this topology result.
- No art requirements, visual proportions, materials, clothing, or species aesthetics
  are created or changed.

## Ownership

- Character Generation & Animation owns the isolated plugin source, harness, policy, and
  evidence under the embodied-species lab.
- Primary Builder retains shared Unreal integration and `research/machines.json` edits.
- A passing challenger produces an `INTEGRATE` exchange; it does not directly modify the
  shared Prototype module.

Requirements checked: Global none recorded; generated content boundary. ART-1 does not
apply because the output is a mechanical placeholder and no visual presentation changes.
Exceptions: none.
