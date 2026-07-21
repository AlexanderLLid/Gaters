#include "GatersSurfaceConditionField.h"

#include "GatersEnvironmentRecipe.h"

namespace
{
void AddIssue(
	FGatersSurfaceConditionCompileResult& Result,
	const TCHAR* RuleId,
	const TCHAR* Message)
{
	Result.Issues.Add({RuleId, Message});
}

bool IsUnit(const float Value)
{
	return FMath::IsFinite(Value) && Value >= 0.f && Value <= 1.f;
}

bool AreSettingsValid(const FGatersSurfaceConditionSettings& Settings)
{
	return FMath::IsFinite(Settings.NormalSampleDistance)
		&& Settings.NormalSampleDistance > 0.f
		&& FMath::IsFinite(Settings.ReliefScale)
		&& Settings.ReliefScale > 0.f
		&& FMath::IsFinite(Settings.ShoreDistance)
		&& Settings.ShoreDistance > 0.f
		&& FMath::IsFinite(Settings.AccumulationScale)
		&& Settings.AccumulationScale > 0.f
		&& IsUnit(Settings.CliffFullNormalZ)
		&& IsUnit(Settings.CliffStartNormalZ)
		&& Settings.CliffFullNormalZ < Settings.CliffStartNormalZ;
}

bool HasRequiredEnvironmentProvenance(
	const FGatersEnvironmentRecipe& Environment)
{
	if (Environment.Version != FGatersEnvironmentRecipe::CurrentVersion
		|| Environment.CompilerVersion
			!= FGatersEnvironmentRecipe::CurrentCompilerVersion
		|| !FMath::IsFinite(Environment.WorldSize)
		|| Environment.WorldSize <= 0.f
		|| Environment.Seed != Environment.Terrain.Seed
		|| Environment.Seed != Environment.Intent.Seed
		|| Environment.Seed != Environment.Climate.Seed
		|| Environment.Seed != Environment.Drainage.Seed
		|| !FMath::IsNearlyEqual(
			Environment.WorldSize, Environment.Terrain.ChunkSize)
		|| !FMath::IsNearlyEqual(
			Environment.WorldSize, Environment.Intent.WorldSize)
		|| !FMath::IsNearlyEqual(
			Environment.WorldSize, Environment.Climate.WorldSize)
		|| !FMath::IsNearlyEqual(
			Environment.WorldSize, Environment.Drainage.WorldSize)
		|| Environment.Climate.Version != FGatersClimateRecipe::CurrentVersion
		|| Environment.Drainage.Version != FGatersDrainageRecipe::CurrentVersion
		|| Environment.RegionalWater.Version != 1
		|| Environment.Drainage.CellsPerAxis < 2
		|| !FMath::IsFinite(Environment.Drainage.Extent)
		|| Environment.Drainage.Extent <= 0.f
		|| !FMath::IsFinite(Environment.Drainage.CellSize)
		|| Environment.Drainage.CellSize <= 0.f
		|| Environment.Drainage.Cells.Num()
			!= Environment.Drainage.CellsPerAxis
				* Environment.Drainage.CellsPerAxis)
	{
		return false;
	}
	for (int32 Index = 0; Index < Environment.Drainage.Cells.Num(); ++Index)
	{
		const FGatersDrainageCell& Cell = Environment.Drainage.Cells[Index];
		if (Cell.Index != Index
			|| !FMath::IsFinite(Cell.Height)
			|| !FMath::IsFinite(Cell.Accumulation)
			|| (Cell.DownstreamIndex != INDEX_NONE
				&& !Environment.Drainage.Cells.IsValidIndex(Cell.DownstreamIndex)))
		{
			return false;
		}
	}
	return true;
}

struct FInterpolatedDrainage
{
	TArray<int32> CellIndices;
	TArray<float> CellWeights;
	float Accumulation = 0.f;
	float Channel = 0.f;
	float Drop = 0.f;
};

FInterpolatedDrainage InterpolateDrainage(
	const FGatersDrainageRecipe& Drainage,
	const FVector2D& Point)
{
	FInterpolatedDrainage Result;
	if (Drainage.CellsPerAxis < 2 || Drainage.CellSize <= 0.f
		|| FMath::Abs(Point.X) > Drainage.Extent
		|| FMath::Abs(Point.Y) > Drainage.Extent)
	{
		return Result;
	}

	const float GridX = FMath::Clamp(
		(Point.X + Drainage.Extent) / Drainage.CellSize,
		0.f,
		static_cast<float>(Drainage.CellsPerAxis - 1));
	const float GridY = FMath::Clamp(
		(Point.Y + Drainage.Extent) / Drainage.CellSize,
		0.f,
		static_cast<float>(Drainage.CellsPerAxis - 1));
	const int32 X0 = FMath::FloorToInt(GridX);
	const int32 Y0 = FMath::FloorToInt(GridY);
	const int32 X1 = FMath::Min(X0 + 1, Drainage.CellsPerAxis - 1);
	const int32 Y1 = FMath::Min(Y0 + 1, Drainage.CellsPerAxis - 1);
	const float AlphaX = GridX - X0;
	const float AlphaY = GridY - Y0;
	const TPair<FIntPoint, float> Samples[] = {
		{FIntPoint(X0, Y0), (1.f - AlphaX) * (1.f - AlphaY)},
		{FIntPoint(X1, Y0), AlphaX * (1.f - AlphaY)},
		{FIntPoint(X0, Y1), (1.f - AlphaX) * AlphaY},
		{FIntPoint(X1, Y1), AlphaX * AlphaY}};
	for (const TPair<FIntPoint, float>& WeightedCell : Samples)
	{
		if (WeightedCell.Value <= SMALL_NUMBER)
		{
			continue;
		}
		const int32 Index =
			WeightedCell.Key.X * Drainage.CellsPerAxis + WeightedCell.Key.Y;
		const FGatersDrainageCell& Cell = Drainage.Cells[Index];
		const float Drop = Cell.DownstreamIndex == INDEX_NONE
			? 0.f
			: FMath::Max(
				0.f, Cell.Height - Drainage.Cells[Cell.DownstreamIndex].Height);
		Result.CellIndices.Add(Index);
		Result.CellWeights.Add(WeightedCell.Value);
		Result.Accumulation += Cell.Accumulation * WeightedCell.Value;
		Result.Channel += (Cell.bChannel ? 1.f : 0.f) * WeightedCell.Value;
		Result.Drop += Drop * WeightedCell.Value;
	}
	return Result;
}

void AccumulateWaterEvidence(
	const FVector2D& Point,
	const float TerrainHeight,
	const FVector2D& Center,
	const float Radius,
	const float WaterHeight,
	const FGatersSurfaceConditionSettings& Settings,
	FGatersSurfaceConditionEvidence& Evidence)
{
	const float CenterDistance = FVector2D::Distance(Point, Center);
	const float OutsideDistance = FMath::Max(CenterDistance - Radius, 0.f);
	const float BoundaryDistance = FMath::Abs(CenterDistance - Radius);
	Evidence.WaterProximity = FMath::Max(
		Evidence.WaterProximity,
		1.f - FMath::SmoothStep(0.f, Settings.ShoreDistance, OutsideDistance));
	Evidence.ShoreProximity = FMath::Max(
		Evidence.ShoreProximity,
		1.f - FMath::SmoothStep(0.f, Settings.ShoreDistance, BoundaryDistance));
	if (CenterDistance <= Radius)
	{
		Evidence.WaterDepth = FMath::Max(
			Evidence.WaterDepth,
			FMath::Clamp(
				(WaterHeight - TerrainHeight) / Settings.ReliefScale,
				0.f,
				1.f));
	}
}
}

FGatersSurfaceConditionCompileResult FGatersSurfaceConditionField::Compile(
	const FGatersEnvironmentRecipe& Environment,
	const FGatersSurfaceConditionSettings& Settings)
{
	FGatersSurfaceConditionCompileResult Result;
	Result.Recipe.Seed = Environment.Seed;
	Result.Recipe.WorldSize = Environment.WorldSize;
	Result.Recipe.EnvironmentVersion = Environment.Version;
	Result.Recipe.ClimateVersion = Environment.Climate.Version;
	Result.Recipe.DrainageVersion = Environment.Drainage.Version;
	Result.Recipe.RegionalWaterVersion = Environment.RegionalWater.Version;
	Result.Recipe.Settings = Settings;
	if (!AreSettingsValid(Settings))
	{
		AddIssue(Result, TEXT("surface.settings"),
			TEXT("Surface condition settings must be finite, positive, and bounded."));
		return Result;
	}
	if (!HasRequiredEnvironmentProvenance(Environment))
	{
		AddIssue(Result, TEXT("surface.environment"),
			TEXT("Surface conditions require a valid accepted environment root."));
	}
	return Result;
}

FGatersSurfaceConditionSample FGatersSurfaceConditionField::Evaluate(
	const FGatersSurfaceConditionEvidence& Evidence,
	const FGatersSurfaceConditionSettings& Settings)
{
	FGatersSurfaceConditionSample Result;
	Result.Evidence = Evidence;
	const float NormalZ = FMath::Clamp(Evidence.NormalZ, 0.f, 1.f);
	const float Slope = 1.f - NormalZ;
	const float Accumulation = 1.f - FMath::Exp(
		-FMath::Max(0.f, Evidence.DrainageAccumulation)
		/ FMath::Max(Settings.AccumulationScale, SMALL_NUMBER));
	const float Channel = FMath::Clamp(Evidence.DrainageChannel, 0.f, 1.f);
	const float Drop = FMath::Clamp(
		FMath::Max(0.f, Evidence.DrainageDrop)
		/ FMath::Max(Settings.ReliefScale, SMALL_NUMBER),
		0.f,
		1.f);
	Result.Ridge = FMath::Clamp(
		(Evidence.Height - Evidence.NeighborhoodMeanHeight)
		/ FMath::Max(Settings.ReliefScale, SMALL_NUMBER),
		0.f,
		1.f);
	Result.Valley = FMath::Clamp(
		(Evidence.NeighborhoodMeanHeight - Evidence.Height)
		/ FMath::Max(Settings.ReliefScale, SMALL_NUMBER),
		0.f,
		1.f);
	Result.Cliff = 1.f - FMath::SmoothStep(
		Settings.CliffFullNormalZ, Settings.CliffStartNormalZ, NormalZ);
	const float Precipitation = FMath::Clamp(Evidence.Precipitation, 0.f, 1.f);
	const float WaterProximity = FMath::Clamp(Evidence.WaterProximity, 0.f, 1.f);
	Result.Saturation = FMath::Clamp(
		Precipitation * 0.45f + Accumulation * 0.35f
			+ WaterProximity * 0.35f + Result.Valley * 0.2f - Slope * 0.25f,
		0.f,
		1.f);
	Result.Sediment = FMath::Clamp(
		Result.Valley * 0.35f + Accumulation * 0.35f
			+ Result.Saturation * 0.35f + (1.f - Drop) * Channel * 0.1f
			+ FMath::Clamp(Evidence.FreezeThaw, 0.f, 1.f) * 0.1f
			- Result.Cliff * 0.55f,
		0.f,
		1.f);
	const float PreliminaryRock = FMath::Clamp(
		Result.Cliff * 0.75f + Slope * 0.35f
			+ FMath::Clamp(Evidence.WindExposure, 0.f, 1.f) * 0.15f
			+ Result.Ridge * 0.2f + Drop * 0.1f,
		0.f,
		1.f);
	Result.Rock = FMath::Clamp(
		PreliminaryRock - Result.Sediment * 0.25f, 0.f, 1.f);
	Result.Shore = FMath::Clamp(
		Evidence.ShoreProximity * (1.f - Result.Cliff * 0.75f), 0.f, 1.f);
	Result.Sand = FMath::Clamp(
		Result.Shore * 0.8f * (1.f - Result.Rock)
			+ (1.f - Precipitation) * 0.12f * (1.f - Slope),
		0.f,
		1.f);
	const float Cold = 1.f - FMath::SmoothStep(
		0.18f, 0.48f, FMath::Clamp(Evidence.Temperature, 0.f, 1.f));
	Result.Snow = FMath::Clamp(
		Cold * (Precipitation * 0.7f + Result.Saturation * 0.3f)
			* (1.f - FMath::Clamp(Evidence.WaterDepth, 0.f, 1.f) * 0.5f),
		0.f,
		1.f);
	Result.Ice = FMath::Clamp(
		Cold * FMath::Max(
			WaterProximity * FMath::Clamp(Evidence.WaterDepth, 0.f, 1.f),
			Result.Saturation * 0.4f),
		0.f,
		1.f);
	Result.Soil = FMath::Clamp(
		(1.f - Result.Rock)
			* (0.25f + Precipitation * 0.35f + Result.Sediment * 0.4f)
			* (1.f - Result.Sand * 0.4f) * (1.f - Result.Snow * 0.3f),
		0.f,
		1.f);
	return Result;
}

FGatersSurfaceConditionSample FGatersSurfaceConditionField::Query(
	const FGatersSurfaceConditionRecipe& Recipe,
	const FGatersEnvironmentRecipe& Environment,
	const FVector2D& Point)
{
	FGatersSurfaceConditionEvidence Evidence;
	Evidence.Height = Environment.QueryTerrain(Point).Height;
	const float Distance = Recipe.Settings.NormalSampleDistance;
	const float PositiveX =
		Environment.QueryTerrain(Point + FVector2D(Distance, 0.f)).Height;
	const float NegativeX =
		Environment.QueryTerrain(Point - FVector2D(Distance, 0.f)).Height;
	const float PositiveY =
		Environment.QueryTerrain(Point + FVector2D(0.f, Distance)).Height;
	const float NegativeY =
		Environment.QueryTerrain(Point - FVector2D(0.f, Distance)).Height;
	const float GradientX = (PositiveX - NegativeX) / (2.f * Distance);
	const float GradientY = (PositiveY - NegativeY) / (2.f * Distance);
	Evidence.NormalZ = FVector(-GradientX, -GradientY, 1.f).GetSafeNormal().Z;
	Evidence.NeighborhoodMeanHeight =
		(PositiveX + NegativeX + PositiveY + NegativeY) * 0.25f;
	const FGatersClimateSample Climate = Environment.QueryClimate(Point);
	Evidence.Temperature = Climate.Temperature;
	Evidence.Precipitation = Climate.Precipitation;
	Evidence.WindExposure = Climate.WindExposure;
	Evidence.FreezeThaw = Climate.FreezeThaw;

	const FInterpolatedDrainage Drainage =
		InterpolateDrainage(Environment.Drainage, Point);
	Evidence.DrainageAccumulation = Drainage.Accumulation;
	Evidence.DrainageChannel = Drainage.Channel;
	Evidence.DrainageDrop = Drainage.Drop;
	for (const FGatersWaterSurface& Surface : Environment.Terrain.WaterSurfaces())
	{
		AccumulateWaterEvidence(
			Point,
			Evidence.Height,
			Surface.Center,
			Surface.HalfExtent,
			Environment.Terrain.WaterHeight,
			Recipe.Settings,
			Evidence);
	}
	for (const FGatersRegionalWaterSurface& Surface :
		Environment.RegionalWater.Surfaces)
	{
		AccumulateWaterEvidence(
			Point,
			Evidence.Height,
			Surface.Center,
			Surface.HalfExtent,
			Surface.Height,
			Recipe.Settings,
			Evidence);
	}

	FGatersSurfaceConditionSample Result = Evaluate(Evidence, Recipe.Settings);
	Result.RecipeVersion = Recipe.Version;
	Result.Seed = Recipe.Seed;
	Result.Point = Point;
	Result.DrainageCellIndices = Drainage.CellIndices;
	Result.DrainageCellWeights = Drainage.CellWeights;
	return Result;
}
