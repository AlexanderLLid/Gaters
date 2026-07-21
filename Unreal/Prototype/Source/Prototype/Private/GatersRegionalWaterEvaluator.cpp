#include "GatersRegionalWaterEvaluator.h"

#include "GatersIntentTerrainField.h"

namespace
{
int32 ExpectedSurfaces(const EGatersHydrology Hydrology)
{
	if (Hydrology == EGatersHydrology::Dry)
	{
		return 0;
	}
	return Hydrology == EGatersHydrology::Lakes ? 2 : 1;
}

const FGatersWorldRegionIntent* FindRegion(
	const FGatersWorldIntentRecipe& Intent,
	const FString& RegionId)
{
	return Intent.Regions.FindByPredicate(
		[&RegionId](const FGatersWorldRegionIntent& Region)
		{
			return Region.Id == RegionId;
		});
}

bool HasSubmergedTerrain(
	const FGatersRegionalWaterSurface& Surface,
	TFunctionRef<float(const FVector2D&)> HeightAt)
{
	for (int32 X = -2; X <= 2; ++X)
	{
		for (int32 Y = -2; Y <= 2; ++Y)
		{
			const FVector2D Offset(
				X * Surface.HalfExtent * 0.45f,
				Y * Surface.HalfExtent * 0.45f);
			if (Offset.SizeSquared() > FMath::Square(Surface.HalfExtent))
			{
				continue;
			}
			if (HeightAt(Surface.Center + Offset) <= Surface.Height + 1.f)
			{
				return true;
			}
		}
	}
	return false;
}
}

FGatersRegionalWaterEvaluation FGatersRegionalWaterEvaluator::Evaluate(
	const FGatersEnvironment& Environment,
	const FGatersWorldIntentRecipe& Intent,
	const FGatersRegionalWaterRecipe& Recipe,
	const float DatumTolerance)
{
	return Evaluate(
		Environment,
		Intent,
		Recipe,
		[&Environment, &Intent](const FVector2D& Point)
		{
			return FGatersIntentTerrainField::Query(
				Environment, Intent, Point).Height;
		},
		DatumTolerance);
}

FGatersRegionalWaterEvaluation FGatersRegionalWaterEvaluator::Evaluate(
	const FGatersEnvironment& Environment,
	const FGatersWorldIntentRecipe& Intent,
	const FGatersRegionalWaterRecipe& Recipe,
	TFunctionRef<float(const FVector2D&)> HeightAt,
	const float DatumTolerance)
{
	check(DatumTolerance >= 0.f);
	FGatersRegionalWaterEvaluation Result;
	for (int32 RegionIndex = 1; RegionIndex < Intent.Regions.Num(); ++RegionIndex)
	{
		const FGatersWorldRegionIntent& Region = Intent.Regions[RegionIndex];
		const int32 Expected = ExpectedSurfaces(Region.HydrologyTendency);
		Result.ExpectedSurfaceCount += Expected;
		int32 Actual = 0;
		for (const FGatersRegionalWaterSurface& Surface : Recipe.Surfaces)
		{
			Actual += Surface.RegionId == Region.Id ? 1 : 0;
		}
		if (Actual != Expected)
		{
			Result.Diagnostics.Add(FString::Printf(
				TEXT("regional water count region=%s expected=%d observed=%d"),
				*Region.Id, Expected, Actual));
		}
	}

	TSet<FString> Ids;
	for (const FGatersRegionalWaterSurface& Surface : Recipe.Surfaces)
	{
		if (Surface.Id.IsEmpty() || Ids.Contains(Surface.Id))
		{
			Result.Diagnostics.Add(FString::Printf(
				TEXT("regional water identity invalid id=%s"), *Surface.Id));
		}
		Ids.Add(Surface.Id);
		const FGatersWorldRegionIntent* Region = FindRegion(Intent, Surface.RegionId);
		if (!Region)
		{
			Result.Diagnostics.Add(FString::Printf(
				TEXT("regional water region missing surface=%s region=%s"),
				*Surface.Id, *Surface.RegionId));
			continue;
		}
		if (Region->HydrologyTendency == EGatersHydrology::Dry)
		{
			Result.Diagnostics.Add(FString::Printf(
				TEXT("regional water leaks into dry region=%s surface=%s"),
				*Region->Id, *Surface.Id));
			continue;
		}
		if (Surface.Hydrology != Region->HydrologyTendency)
		{
			Result.Diagnostics.Add(FString::Printf(
				TEXT("regional water hydrology mismatch region=%s surface=%s"),
				*Region->Id, *Surface.Id));
		}
		if (Surface.HalfExtent <= 0.f ||
			FVector2D::Distance(Surface.Center, Region->Center) + Surface.HalfExtent
				> Region->Radius + 1.f)
		{
			Result.Diagnostics.Add(FString::Printf(
				TEXT("regional water bounds invalid region=%s surface=%s"),
				*Region->Id, *Surface.Id));
		}
		const FGatersEnvironment RegionalEnvironment = Environment.WithProfile(
			Region->TerrainTendency, Region->HydrologyTendency);
		if (FMath::Abs(Surface.Height - RegionalEnvironment.WaterHeight) > DatumTolerance)
		{
			Result.Diagnostics.Add(FString::Printf(
				TEXT("regional water datum mismatch region=%s surface=%s expected=%.3f observed=%.3f"),
				*Region->Id, *Surface.Id,
				RegionalEnvironment.WaterHeight, Surface.Height));
		}
		if (HasSubmergedTerrain(Surface, HeightAt))
		{
			++Result.SubmergedSurfaceCount;
		}
		else
		{
			Result.Diagnostics.Add(FString::Printf(
				TEXT("regional water has no submerged terrain region=%s surface=%s"),
				*Region->Id, *Surface.Id));
		}
	}
	Result.bValid = Result.Diagnostics.IsEmpty();
	return Result;
}
