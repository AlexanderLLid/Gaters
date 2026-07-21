# Landform Runtime Preview

## Outcome

- The isolated process field can be enabled for the generated world without replacing
  the terrain champion.
- The same seed can be compared with `Gaters.Landforms 0` and
  `Gaters.Landforms 1`.
- The toggle is independent of Built Sites, settlement growth, and Arrival semantics.

## Design

- Attach an optional validated process recipe to a copied `FGatersEnvironment`.
- Apply processes after the selected base terrain profile and before its hydrology.
- Preserve global process coordinates when regional terrain profiles use local shape
  coordinates.
- Compile the existing environment-recipe root from the accepted environment copy.
- Default runtime state remains off.

## Tasks

- [x] Add failing integration and console-registration tests.
- [x] Add optional process attachment and regional-coordinate preservation.
- [x] Add environment-recipe compilation from an explicit accepted terrain.
- [x] Add `Gaters.Landforms <0|1>` persistence and chunk integration.
- [x] Build and run focused tests.
- [x] Run the complete shared suite and validators.

Requirements checked: none applicable; exceptions: none.
