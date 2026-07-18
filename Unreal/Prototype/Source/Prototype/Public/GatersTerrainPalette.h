#pragma once

#include "CoreMinimal.h"
#include "GatersEnvironment.h"

// Pure visual classification for generated terrain. Geometry and gameplay semantics
// remain owned by the terrain generator and semantic field.
struct PROTOTYPE_API FGatersTerrainPalette
{
	static FLinearColor BlendColor(
		EGatersEnvironment Type,
		float WaterHeight,
		float Height,
		float NormalZ);

	static TStaticArray<FLinearColor, 3> Colors(EGatersEnvironment Type);
	static FLinearColor WaterAbsorption(EGatersHydrology Hydrology);
	static FLinearColor WaterScattering(EGatersHydrology Hydrology);
	static float WaterPhaseG(EGatersHydrology Hydrology);
};
