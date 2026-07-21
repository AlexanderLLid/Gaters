#include "GatersRegionalWaterRecipe.h"

FGatersRegionalWaterRecipe FGatersRegionalWaterRecipe::Generate(
	const FGatersEnvironment& Environment,
	const FGatersWorldIntentRecipe& Intent)
{
	FGatersRegionalWaterRecipe Result;
	for (int32 RegionIndex = 1; RegionIndex < Intent.Regions.Num(); ++RegionIndex)
	{
		const FGatersWorldRegionIntent& Region = Intent.Regions[RegionIndex];
		if (Region.HydrologyTendency == EGatersHydrology::Dry)
		{
			continue;
		}
		const FGatersEnvironment RegionalEnvironment = Environment.WithProfile(
			Region.TerrainTendency,
			Region.HydrologyTendency,
			FVector2D::ZeroVector,
			Region.Radius);
		const TArray<FGatersWaterSurface> LocalSurfaces =
			RegionalEnvironment.WaterSurfaces();
		for (int32 SurfaceIndex = 0; SurfaceIndex < LocalSurfaces.Num(); ++SurfaceIndex)
		{
			const FGatersWaterSurface& Local = LocalSurfaces[SurfaceIndex];
			FGatersRegionalWaterSurface& Surface = Result.Surfaces.AddDefaulted_GetRef();
			Surface.Id = FString::Printf(TEXT("water:%s:%d"), *Region.Id, SurfaceIndex);
			Surface.RegionId = Region.Id;
			Surface.RegionCenter = Region.Center;
			Surface.RegionRadius = Region.Radius;
			Surface.Center = Region.Center + Local.Center;
			Surface.HalfExtent = FMath::Min(Local.HalfExtent, Region.Radius * 0.5f);
			Surface.Height = RegionalEnvironment.WaterHeight;
			Surface.Hydrology = Region.HydrologyTendency;
		}
	}
	return Result;
}
