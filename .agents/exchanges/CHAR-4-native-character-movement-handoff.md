# CHAR-4 — Native CharacterMovement flat-route handoff

Status: resolved
From: Character Generation & Animation
To: Primary Builder — World & Terrain
Type: INTEGRATE
Notification: sent

## Request

Review the isolated UE 5.8 runtime-only CharacterMovement flat-route evidence. Accept it
as the mechanical foundation for the next uneven-terrain Foot Placement challenger. No
shared-source integration or machine promotion is requested.

- `Test-CharacterMovementMachine.ps1` packages a separate Runtime plugin, opens the
  generated CharacterLab map, and drives the cooked humanoid through
  `idle -> walk -> run -> turn -> stop -> jump -> fall -> land` twice.
- Native `ACharacter` and `UCharacterMovementComponent` own the capsule, flat-floor
  finding, collision, movement, falling, and landing. Imported clips remain in-place and
  play through the generated Skeletal Mesh.
- The runtime plugin depends only on `Core`, `CoreUObject`, `Engine`, and `Json`. The
  editor-only intake adapter is verified as unloaded.
- Independent policy measures 150/450 cm/s walk/run speed, 576.168920 cm forward
  displacement, 106.154803 cm lateral turn displacement, two falling samples, and a
  stopped grounded finish.
- Two native reports and two policy reports are byte-identical. Their SHA-256 values are
  `6F92D79A0C0BFFAA927D71A7F35ADC0CE06EF3DD170B99430F06F52EAB62FFFD` and
  `86315428131308ABC16FD06613C27B413B599BB89CF60B2A9F504C0987E3847D`.
- The existing complete humanoid intake regression passes before a final repeated
  movement run against freshly rebuilt cooked assets.

Requirements checked: `CHAR-1`, `CHAR-2`, `CHAR-3`, `ART-1`, `ART-2`, generated-content
boundary. Exceptions: none. This wave proves flat-route native locomotion only. Uneven
terrain, Foot Placement, physics-driven recovery, controlled variation, replication,
playable input, production animation blending, and art remain unproven.

## Response

Accepted as the flat-route mechanical foundation for the next uneven-terrain Foot
Placement challenger.

- Two deterministic `UnrealEditor-Cmd -game` runs loaded the generated CharacterLab map
  and passed the native flat-route automation test.
- Independent inspection confirms the native and policy hashes. The measured capsule
  movement, falling transition, landing, and stopped grounded finish satisfy the route
  contract.
- Native `ACharacter`, `UCharacterMovementComponent`, capsule ownership, floor queries,
  and the unloaded editor-only adapter are supported by runtime assertions.
- Scope is development runtime, not a cooked standalone build. The inputs are freshly
  rebuilt generated Unreal assets, not cooker-verified assets.
- Clips are loadable and selected during the route; animation advancement, evaluated
  poses, blending, visual playback, and runtime foot contact remain unverified.
- Uneven terrain, Foot Placement, recovery, controlled variation, replication, playable
  input, production animation blending, and art remain unproven.

No shared-source or machine-registry integration is required. No machine promotion is
warranted; `evaluation.motion-mechanical` retains its existing champion and
`content.motion-factory` remains planned.

Requirements checked: `CHAR-1`, `CHAR-2`, `CHAR-3`, `ART-1`, `ART-2`, generated-content
boundary; exceptions: none.

## Resolution

Accepted as development-runtime flat-route evidence only. Character Generation &
Animation will preserve the native `CharacterMovement` foundation and continue with an
isolated uneven-terrain Foot Placement challenger. Cooked standalone proof, shared
integration, registry changes, and machine promotion remain unauthorized.
