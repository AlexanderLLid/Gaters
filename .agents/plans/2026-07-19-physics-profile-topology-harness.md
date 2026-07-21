# Physics Profile Topology and Harness Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the existing generated humanoid intake pass exact native Physics Asset
body/constraint topology through an isolated UE 5.8 plugin, while establishing the
one-command derived test-map and independent report harness used by later character waves.

**Architecture:** Keep the versioned Blender manifest and `physical-profile.json`
authoritative. Package a small external editor plugin that validates topology and uses
`FPhysicsAssetUtils`; load it only with `-PLUGIN=` from the existing intake command. The
Unreal Python adapter orchestrates import and produces derived assets, while the existing
standalone Python policy remains the independent judge.

**Tech Stack:** PowerShell 7, Python 3 `unittest`, Blender 5.2, Unreal Engine 5.8 C++
editor plugin, Unreal Python, `FPhysicsAssetUtils`, Unreal Automation Tests.

**Scope decomposition:** This is the first independently falsifiable subproject of the
approved humanoid design. Later plans own locomotion/real IK, terrain contact, complete
physical semantics/recovery, and proportion variation in that order. This plan exposes
their shared command/map/report seam but does not implement their behavior early.

## Global Constraints

- Work only under `research/embodied-species-lab/`, its owned workstream status, and
  derived CharacterLab outputs. Do not modify shared Unreal source, `Prototype.uproject`,
  or `research/machines.json`.
- Preserve all unrelated uncommitted changes. Do not branch, commit, or push.
- Before `RunUAT`, UnrealBuildTool, or `UnrealEditor-Cmd`, check for a running interactive
  Unreal Editor and ask the human to close it if found.
- Use native UE 5.8 physics generation and constraint types; do not implement a collision
  solver or runtime physics system.
- This wave proves `CHAR-1` intake topology and harness foundations plus the `CHAR-2`
  adapter boundary. It does not claim locomotion/recovery completion or `CHAR-3` variation.
- `ART-1` and `ART-2` remain outside this mechanical placeholder; do not claim character,
  body-factory, or motion-factory promotion.

---

## File map

- `unreal/CharacterPhysicsProfileAdapter/CharacterPhysicsProfileAdapter.uplugin` —
  external editor-plugin descriptor.
- `unreal/CharacterPhysicsProfileAdapter/Source/CharacterPhysicsProfileAdapter/CharacterPhysicsProfileAdapter.Build.cs`
  — UE module dependencies.
- `unreal/CharacterPhysicsProfileAdapter/Source/CharacterPhysicsProfileAdapter/Public/CharacterPhysicsProfileAdapterLibrary.h`
  — Python-visible adapter contract plus pure topology validator.
- `unreal/CharacterPhysicsProfileAdapter/Source/CharacterPhysicsProfileAdapter/Private/CharacterPhysicsProfileAdapterLibrary.cpp`
  — validation and native Physics Asset rebuild.
- `unreal/CharacterPhysicsProfileAdapter/Source/CharacterPhysicsProfileAdapter/Private/CharacterPhysicsProfileAdapterModule.cpp`
  — editor module entry point.
- `unreal/CharacterPhysicsProfileAdapter/Source/CharacterPhysicsProfileAdapter/Private/Tests/CharacterPhysicsProfileAdapterTests.cpp`
  — native validator automation.
- `Build-CharacterPhysicsProfileAdapter.ps1` — guarded external-plugin packaging.
- `unreal/import_generated_humanoid.py` — invoke the adapter, create the derived preview
  map, and report native readback.
- `unreal_intake_policy.py` and `tests/test_unreal_intake_policy.py` — independent adapter
  provenance and exact-topology rules.
- `Test-UnrealHumanoidIntake.ps1` — one public command that builds, imports twice, runs
  native/unit policies, and verifies the derived map.

---

### Task 1: External plugin contract and pure topology validation

**Files:**

- Create: `research/embodied-species-lab/unreal/CharacterPhysicsProfileAdapter/CharacterPhysicsProfileAdapter.uplugin`
- Create: `research/embodied-species-lab/unreal/CharacterPhysicsProfileAdapter/Source/CharacterPhysicsProfileAdapter/CharacterPhysicsProfileAdapter.Build.cs`
- Create: `research/embodied-species-lab/unreal/CharacterPhysicsProfileAdapter/Source/CharacterPhysicsProfileAdapter/Public/CharacterPhysicsProfileAdapterLibrary.h`
- Create: `research/embodied-species-lab/unreal/CharacterPhysicsProfileAdapter/Source/CharacterPhysicsProfileAdapter/Private/CharacterPhysicsProfileAdapterModule.cpp`
- Create: `research/embodied-species-lab/unreal/CharacterPhysicsProfileAdapter/Source/CharacterPhysicsProfileAdapter/Private/Tests/CharacterPhysicsProfileAdapterTests.cpp`
- Create: `research/embodied-species-lab/Build-CharacterPhysicsProfileAdapter.ps1`

**Interfaces:**

- Consumes: skeleton bone names plus parallel body, parent, and child `TArray<FName>` values.
- Produces: `Gaters::CharacterLab::ValidateTopology(...)`,
  `UCharacterPhysicsProfileAdapterLibrary::RebuildPhysicsAssetTopology(...)`, and adapter
  version `1` exposed to Unreal Python.

- [ ] **Step 1: Add the plugin descriptor, module rules, and module entry point**

Use an editor-only module with `Core`, `CoreUObject`, `Engine`, `PhysicsUtilities`, and
`UnrealEd` dependencies. Set `EnabledByDefault` to `false`; the harness loads the packaged
descriptor explicitly with `-PLUGIN=`.

```json
{
  "FileVersion": 3,
  "Version": 1,
  "VersionName": "1",
  "FriendlyName": "Character Physics Profile Adapter",
  "CanContainContent": false,
  "EnabledByDefault": false,
  "Modules": [
    {
      "Name": "CharacterPhysicsProfileAdapter",
      "Type": "Editor",
      "LoadingPhase": "Default"
    }
  ]
}
```

```csharp
using UnrealBuildTool;

public class CharacterPhysicsProfileAdapter : ModuleRules
{
    public CharacterPhysicsProfileAdapter(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });
        PrivateDependencyModuleNames.AddRange(new[] { "PhysicsUtilities", "UnrealEd" });
    }
}
```

```cpp
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FDefaultModuleImpl, CharacterPhysicsProfileAdapter)
```

- [ ] **Step 2: Write the failing native validation tests and public header**

Declare the exact public interface. Add automation cases for a connected three-body tree,
an unknown body bone, duplicate body names, mismatched joint arrays, and a disconnected
graph. The valid case must expect `true`; every counterexample must expect `false` and a
non-empty causal error.

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CharacterPhysicsProfileAdapterLibrary.generated.h"

class UPhysicsAsset;
class USkeletalMesh;

namespace Gaters::CharacterLab
{
CHARACTERPHYSICSPROFILEADAPTER_API bool ValidateTopology(
    const TSet<FName>& SkeletonBones,
    const TArray<FName>& BodyBones,
    const TArray<FName>& ParentBones,
    const TArray<FName>& ChildBones,
    FString& OutError);
}

UCLASS()
class CHARACTERPHYSICSPROFILEADAPTER_API UCharacterPhysicsProfileAdapterLibrary
    : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "Gaters|CharacterLab")
    static int32 GetAdapterVersion() { return 1; }

    UFUNCTION(BlueprintCallable, Category = "Gaters|CharacterLab")
    static FString RebuildPhysicsAssetTopology(
        USkeletalMesh* SkeletalMesh,
        UPhysicsAsset* PhysicsAsset,
        const TArray<FName>& BodyBones,
        const TArray<FName>& ParentBones,
        const TArray<FName>& ChildBones);
};
```

Representative valid test fixture:

```cpp
TSet<FName> Skeleton{TEXT("pelvis"), TEXT("spine"), TEXT("chest")};
TArray<FName> Bodies{TEXT("pelvis"), TEXT("spine"), TEXT("chest")};
TArray<FName> Parents{TEXT("pelvis"), TEXT("spine")};
TArray<FName> Children{TEXT("spine"), TEXT("chest")};
FString Error;
TestTrue(TEXT("Connected topology passes"),
    Gaters::CharacterLab::ValidateTopology(Skeleton, Bodies, Parents, Children, Error));
TestTrue(TEXT("Valid topology has no error"), Error.IsEmpty());
```

- [ ] **Step 3: Build to verify RED**

Run only after confirming `Get-Process -Name UnrealEditor` returns no process:

```powershell
./Build-CharacterPhysicsProfileAdapter.ps1
```

Expected: compilation fails because `ValidateTopology` and
`RebuildPhysicsAssetTopology` have declarations but no definitions.

- [ ] **Step 4: Implement the minimal pure validator and a causal adapter stub**

`ValidateTopology` must reject null names, empty bodies, unknown bones, duplicates,
parallel-array mismatches, self-joints, joints outside the body set, duplicate pairs,
multiple parents for one child, any constraint count other than `body count - 1`, and a
graph not reachable from its single root. `RebuildPhysicsAssetTopology` validates null
assets and the reference-skeleton bone set, calls `ValidateTopology`, then returns
`"Native rebuild is not implemented"`; an empty return means success and a non-empty
return is the Python-visible causal error. This is the deliberate RED seam for
Task 2.

```cpp
if (BodyBones.IsEmpty())
{
    OutError = TEXT("BodyBones must not be empty");
    return false;
}
if (ParentBones.Num() != ChildBones.Num())
{
    OutError = TEXT("ParentBones and ChildBones must have equal length");
    return false;
}
if (ParentBones.Num() != BodyBones.Num() - 1)
{
    OutError = TEXT("A connected body tree requires exactly body count minus one joints");
    return false;
}
```

Use `TSet<FName>` for duplicate and membership checks, one child-to-parent map for root
count, and a breadth-first traversal over parent-to-child adjacency for connectivity.

- [ ] **Step 5: Package and run native automation to verify GREEN**

`Build-CharacterPhysicsProfileAdapter.ps1` must validate its explicit paths, reject a
running interactive editor, delete only
`research/embodied-species-lab/Derived/P`, temporarily map the lab to unused drive `Q:`
to keep Unreal action paths below the Windows limit, and run:

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat' BuildPlugin `
  -Plugin='Q:\unreal\CharacterPhysicsProfileAdapter\CharacterPhysicsProfileAdapter.uplugin' `
  -Package='Q:\Derived\P' `
  -TargetPlatforms=Win64 -Rocket
```

Then run the plugin automation group with `UnrealEditor-Cmd`, `-PLUGIN=<packaged uplugin>`,
`-TestExit="Automation Test Queue Empty"`, and require every
`Gaters.CharacterLab.PhysicsProfileAdapter` test to log `Result={Success}`.

- [ ] **Step 6: Review checkpoint**

```powershell
git diff -- research/embodied-species-lab/unreal/CharacterPhysicsProfileAdapter `
  research/embodied-species-lab/Build-CharacterPhysicsProfileAdapter.ps1
```

Expected: plugin contract, validator, native tests, and guarded builder only. Do not
commit.

---

### Task 2: Wire the adapter into the existing intake and prove RED

**Files:**

- Modify: `research/embodied-species-lab/Test-UnrealHumanoidIntake.ps1`
- Modify: `research/embodied-species-lab/unreal/import_generated_humanoid.py`
- Modify: `research/embodied-species-lab/tests/test_unreal_intake_policy.py`
- Modify: `research/embodied-species-lab/unreal_intake_policy.py`

**Interfaces:**

- Consumes: `physical-profile.json` parts and joint pairs.
- Produces: adapter call arguments, report `adapterVersion`, report derivation
  `profile-topology-v1`, and policy rule `intake.physics.adapter`.

- [ ] **Step 1: Add failing policy tests for adapter provenance**

Set the valid fixture to `adapterVersion: 1` and
`physicsAsset.derivation: "profile-topology-v1"`. Add tests removing or changing each
value and require `intake.physics.adapter`.

```python
def test_missing_profile_adapter_provenance_fails(self):
    report = copy.deepcopy(REPORT)
    del report["adapterVersion"]
    report["physicsAsset"]["derivation"] = "native-auto"
    result = self.evaluate(report)
    self.assertIn(
        "intake.physics.adapter",
        [issue["ruleId"] for issue in result["issues"]],
    )
```

- [ ] **Step 2: Run the focused test to verify RED**

```powershell
& 'C:\Users\alexa\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe' `
  research/embodied-species-lab/tests/test_unreal_intake_policy.py -v
```

Expected: the new provenance test fails because the evaluator does not emit
`intake.physics.adapter`.

- [ ] **Step 3: Implement the minimal independent provenance rule**

Increment `EVALUATOR_VERSION` and require both exact values:

```python
if (
    report.get("adapterVersion") != 1
    or physics.get("derivation") != "profile-topology-v1"
):
    _issue(
        issues,
        "intake.physics.adapter",
        "Physics Asset topology was not produced by profile adapter v1.",
    )
```

Run the focused unit suite again. Expected: all policy tests pass.

- [ ] **Step 4: Invoke the packaged plugin from Unreal Python**

After native Physics Asset creation, build exact arrays from the profile and call:

```python
body_bones = [part["bone"] for part in profile["parts"]]
parent_bones = [joint["parent"] for joint in profile["joints"]]
child_bones = [joint["child"] for joint in profile["joints"]]
adapter = unreal.CharacterPhysicsProfileAdapterLibrary
error = adapter.rebuild_physics_asset_topology(
    skeletal_mesh,
    physics_asset,
    body_bones,
    parent_bones,
    child_bones,
)
if error:
    raise RuntimeError(f"Physics profile adapter failed: {error}")
adapter_version = int(adapter.get_adapter_version())
```

Read the native body and constraint arrays after the adapter call. Record
`adapterVersion` and `physicsAsset.derivation` in report schema version `2`; do not copy
expected profile arrays into measured fields.

- [ ] **Step 5: Load the plugin from the public test command**

Call `Build-CharacterPhysicsProfileAdapter.ps1` once before import, resolve the packaged
descriptor, add it to required paths, and append `-PLUGIN=$PackagedPlugin` to every
`UnrealEditor-Cmd` invocation.

- [ ] **Step 6: Run the intake to verify integration RED**

Run only after confirming the interactive editor is closed:

```powershell
./research/embodied-species-lab/Test-UnrealHumanoidIntake.ps1
```

Expected: Unreal import fails causally with
`Physics profile adapter failed: Native rebuild is not implemented`. This proves the
Python call reaches the fresh packaged plugin before production rebuild code exists.

---

### Task 3: Rebuild exact native body and constraint topology

**Files:**

- Modify: `research/embodied-species-lab/unreal/CharacterPhysicsProfileAdapter/Source/CharacterPhysicsProfileAdapter/Private/CharacterPhysicsProfileAdapterLibrary.cpp`

**Interfaces:**

- Consumes: validated Skeletal Mesh, Physics Asset, profile body names, and profile joint
  pairs.
- Produces: exact native `SkeletalBodySetups` and `ConstraintSetup` topology attached to
  the Skeletal Mesh.

- [ ] **Step 1: Replace the causal stub with native body generation**

After validation, clear constraints and bodies in reverse order. Configure
`FPhysAssetCreateParams` with box fitting, all bones forced, zero minimum bone size,
native vertex fitting, no automatic constraints, and no progress UI. Call
`FPhysicsAssetUtils::CreateFromSkeletalMesh(..., false, false)`, then delete every body
whose bone is not declared by the profile. Reject any missing or extra body after
readback.

```cpp
FPhysAssetCreateParams Params;
Params.MinBoneSize = 0.0f;
Params.GeomType = EFG_Box;
Params.VertWeight = EVW_DominantWeight;
Params.bAlwaysUseVertices = true;
Params.bIncludeChildBones = true;
Params.bAutoOrientToBone = true;
Params.bCreateConstraints = false;
Params.bWalkPastSmall = false;
Params.bBodyForAll = true;
Params.bDisableCollisionsByDefault = true;

FText NativeError;
if (!FPhysicsAssetUtils::CreateFromSkeletalMesh(
        PhysicsAsset, SkeletalMesh, Params, NativeError, false, false))
{
    OutError = NativeError.ToString();
    return false;
}
```

- [ ] **Step 2: Create exact constraints and reference frames**

For each parallel parent/child pair, create one constraint named after the child. Set
`ConstraintBone1` to child and `ConstraintBone2` to parent. Use
`FAnimationRuntime::GetComponentSpaceTransformRefPose` for both bones; set child frame to
identity and parent frame to the child component transform relative to the parent.

```cpp
FConstraintInstance& Instance = ConstraintTemplate->DefaultInstance;
Instance.ConstraintBone1 = ChildBones[Index];
Instance.ConstraintBone2 = ParentBones[Index];
const FTransform ChildCS = FAnimationRuntime::GetComponentSpaceTransformRefPose(Ref, ChildIndex);
const FTransform ParentCS = FAnimationRuntime::GetComponentSpaceTransformRefPose(Ref, ParentIndex);
Instance.SetRefFrame(EConstraintFrame::Frame1, FTransform::Identity);
Instance.SetRefFrame(EConstraintFrame::Frame2, ChildCS.GetRelativeTransform(ParentCS));
ConstraintTemplate->SetDefaultProfile(Instance);
```

Update body maps/bounds, assign the Physics Asset with `SkeletalMesh->SetPhysicsAsset`,
mark both assets modified and dirty, and call editor refresh mechanisms. Return false on
any native count or pair mismatch before save.

- [ ] **Step 3: Repackage and run the public intake to verify GREEN**

```powershell
./research/embodied-species-lab/Test-UnrealHumanoidIntake.ps1
```

Expected: two identical reports and two identical policy reports; native readback contains
exactly fourteen profile body bones and thirteen profile parent/child constraints; no
adapter/import error appears; command exits `0`.

- [ ] **Step 4: Prove the malformed-profile counterexample**

Add a harness-only copied profile whose first joint child is `missing_bone`; never edit
the authoritative generated profile. Invoke one import with the copied path and require a
causal adapter error containing `unknown skeleton bone` before asset save.

- [ ] **Step 5: Run the Blender regression champions**

```powershell
./research/embodied-species-lab/Test-HumanoidMachine.ps1
./research/embodied-species-lab/Test-PhysicalHumanoid.ps1
./research/embodied-species-lab/Test-RecoveryMachine.ps1
```

Expected: all three existing champions pass unchanged.

---

### Task 4: Establish the derived preview map and close the wave

**Files:**

- Modify: `research/embodied-species-lab/unreal/import_generated_humanoid.py`
- Modify: `research/embodied-species-lab/Test-UnrealHumanoidIntake.ps1`
- Modify: `research/embodied-species-lab/README.md`
- Modify: `.agents/workstreams/Character Generation & Animation.md`
- Modify after evidence: `.agents/exchanges/CHAR-1-character-pipeline-contract.md`

**Interfaces:**

- Consumes: passing imported Skeletal Mesh and Physics Asset.
- Produces: `/Game/Gaters/Generated/CharacterLab/L_CharacterLab`, a floor plus generated
  Skeletal Mesh preview Actor, stable map path in report schema `2`, immutable evidence,
  and an `INTEGRATE` packet only if the promotion gate passes.

- [ ] **Step 1: Add a failing harness assertion for the map**

After each intake, require both report field
`testMap == "/Game/Gaters/Generated/CharacterLab/L_CharacterLab"` and file
`Unreal/Prototype/Content/Gaters/Generated/CharacterLab/L_CharacterLab.umap`.

Run the public intake. Expected: fail because the map field and asset do not exist.

- [ ] **Step 2: Generate the minimal derived map in Unreal Python**

Delete any prior derived map, call `unreal.EditorLevelLibrary.new_level`, spawn a
`StaticMeshActor` using `/Engine/BasicShapes/Cube.Cube` scaled as a floor, spawn one
`SkeletalMeshActor`, assign the generated mesh and Physics Asset, then save the level.
Record only the stable map asset path in the report.

```python
map_path = f"{destination}/L_CharacterLab"
if unreal.EditorAssetLibrary.does_asset_exist(map_path):
    unreal.EditorAssetLibrary.delete_asset(map_path)
if not unreal.EditorLevelLibrary.new_level(map_path):
    raise RuntimeError(f"Could not create CharacterLab map: {map_path}")
floor = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.StaticMeshActor, unreal.Vector(0.0, 0.0, -10.0)
)
floor.static_mesh_component.set_static_mesh(
    unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
)
floor.set_actor_scale3d(unreal.Vector(10.0, 10.0, 0.1))
preview = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.SkeletalMeshActor, unreal.Vector(0.0, 0.0, 100.0)
)
preview.skeletal_mesh_component.set_skeletal_mesh_asset(skeletal_mesh)
preview.skeletal_mesh_component.set_physics_asset(physics_asset, True)
if not unreal.EditorLevelLibrary.save_current_level():
    raise RuntimeError("Could not save CharacterLab map")
```

- [ ] **Step 3: Run full fresh verification**

Run unit tests, the public two-run Unreal intake, and all three Blender champions. Inspect
both Unreal logs for Python, adapter, import, ensure, or automation errors. Compare the two
stable reports byte-for-byte.

- [ ] **Step 4: Record only evidence-backed status**

If every gate passes, update the lab README and Character workstream status with report
paths and measured `14/13` topology. Update `CHAR-1` with the result or create the smallest
phase-correct `INTEGRATE` packet for Primary Builder. Do not edit `research/machines.json`.

If any gate fails, preserve the report/log, record the falsified guarantee and smallest
next experiment, and leave the current champion unchanged.

- [ ] **Step 5: Final requirements and scope audit**

Verify:

- `CHAR-1`: only intake topology and harness foundations are claimed, not the full MUST.
- `CHAR-2`: Blender remains offline authority and Unreal native systems own derived
  runtime assets.
- `CHAR-3`: remains pending the final humanoid proportion-variant wave.
- `ART-1`, `ART-2`: no visual promotion claim.
- No shared Unreal source, project descriptor, machine registry, quadruped, species,
  lineage, evolution, or art file changed.

Requirements checked: Global none recorded; `ART-1`, `ART-2`, `CHAR-1`, `CHAR-2`,
`CHAR-3`, generated-content boundary. Exceptions: none. This experiment is not a product
or factory promotion; `CHAR-1` and `CHAR-3` remain open until their later gates pass, and
`ART-1`/`ART-2` remain required before any finished-character promotion.
