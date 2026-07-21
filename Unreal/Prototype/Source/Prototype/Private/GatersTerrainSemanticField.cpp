#include "GatersTerrainSemanticField.h"

#include "GatersIntentTerrainField.h"

namespace
{
template <typename HeightFunction>
float MaterializeHeight(
	const FGatersEnvironment& Environment,
	const FVector2D& Point,
	const float PadRadius,
	const FVector2D& RouteTarget,
	HeightFunction&& HeightAt)
{
	const float RawHeight = HeightAt(Point);
	const float TransitionWidth =
		FGatersTerrainSemanticField::PadTransitionWidth(PadRadius);
	const float RadialBlend = PadRadius > 0.f
		? FMath::SmoothStep(PadRadius, PadRadius + TransitionWidth,
			static_cast<float>(Point.Size()))
		: 1.f;
	const FVector2D Direction = RouteTarget.GetSafeNormal();
	const float RouteLength = static_cast<float>(RouteTarget.Size());
	if (!Direction.IsNearlyZero() && RouteLength > PadRadius)
	{
		const float Along = static_cast<float>(FVector2D::DotProduct(Point, Direction));
		if (Along > 0.f && Along <= RouteLength + 1000.f)
		{
			const FVector2D Side(-Direction.Y, Direction.X);
			const float Across = FMath::Abs(static_cast<float>(FVector2D::DotProduct(Point, Side)));
			const float Influence = 1.f - FMath::SmoothStep(600.f, 1400.f, Across);
			const float DryHeight = Environment.HasWater()
				? FMath::Max(0.f, Environment.WaterHeight + 100.f)
				: 0.f;
			const float TargetHeight = FMath::Max(HeightAt(RouteTarget), DryHeight);
			const float RouteAlpha = FMath::SmoothStep(PadRadius, RouteLength, Along);
			const float RouteHeight = FMath::Lerp(0.f,
				FMath::Lerp(DryHeight, TargetHeight, RouteAlpha),
				FMath::SmoothStep(PadRadius, PadRadius + 500.f, Along));
			return FMath::Lerp(RawHeight * RadialBlend, RouteHeight, Influence);
		}
	}
	return RawHeight * RadialBlend;
}

template <typename HeightFunction, typename WetFunction>
FGatersTerrainSemanticField BuildSemanticField(
	const int32 CellsPerAxis,
	const float CellSize,
	const float PadRadius,
	const float FlatNormalZ,
	const float SlopeNormalZ,
	HeightFunction&& HeightAt,
	WetFunction&& IsWetAt)
{
	check(CellsPerAxis > 0);
	check(CellSize > 0.f);
	check(FlatNormalZ >= SlopeNormalZ);

	FGatersTerrainSemanticField Result;
	Result.CellsPerAxis = CellsPerAxis;
	Result.CellSize = CellSize;
	Result.Cells.SetNum(CellsPerAxis * CellsPerAxis);
	const int32 Half = CellsPerAxis / 2;
	const float Eps = CellSize * 0.25f;

	for (int32 X = 0; X < CellsPerAxis; ++X)
	{
		for (int32 Y = 0; Y < CellsPerAxis; ++Y)
		{
			const FVector2D Point((X - Half) * CellSize, (Y - Half) * CellSize);
			FGatersTerrainSemanticSample& Sample = Result.Cells[X * CellsPerAxis + Y];
			Sample.Height = HeightAt(Point);
			const float Gx = (HeightAt(Point + FVector2D(Eps, 0.f))
				- HeightAt(Point - FVector2D(Eps, 0.f))) / (2.f * Eps);
			const float Gy = (HeightAt(Point + FVector2D(0.f, Eps))
				- HeightAt(Point - FVector2D(0.f, Eps))) / (2.f * Eps);
			Sample.NormalZ = 1.f / FMath::Sqrt(1.f + Gx * Gx + Gy * Gy);

			if (Point.SizeSquared() <= FMath::Square(PadRadius))
			{
				Sample.Type = EGatersTerrainSemantic::Flat;
			}
			else if (IsWetAt(Point, Sample.Height))
			{
				Sample.Type = EGatersTerrainSemantic::Water;
			}
			else if (Sample.NormalZ >= FlatNormalZ)
			{
				Sample.Type = EGatersTerrainSemantic::Flat;
			}
			else if (Sample.NormalZ >= SlopeNormalZ)
			{
				Sample.Type = EGatersTerrainSemantic::Slope;
			}
			else
			{
				Sample.Type = EGatersTerrainSemantic::Steep;
			}
		}
	}
	return Result;
}
}

float FGatersTerrainSemanticField::PadTransitionWidth(const float PadRadius)
{
	return PadRadius > 0.f ? FMath::Max(800.f, PadRadius * 2.f) : 0.f;
}

float FGatersTerrainSemanticField::MaterializedHeightAt(
	const FGatersEnvironment& Environment,
	const FVector2D& Point,
	float PadRadius,
	const FVector2D& RouteTarget)
{
	return MaterializeHeight(Environment, Point, PadRadius, RouteTarget,
		[&Environment](const FVector2D& Sample) { return Environment.HeightAt(Sample); });
}

float FGatersTerrainSemanticField::MaterializedHeightAt(
	const FGatersEnvironment& Environment,
	const FGatersWorldIntentRecipe& Intent,
	const FVector2D& Point,
	const float PadRadius,
	const FVector2D& RouteTarget)
{
	return MaterializeHeight(Environment, Point, PadRadius, RouteTarget,
		[&Environment, &Intent](const FVector2D& Sample)
		{
			return FGatersIntentTerrainField::Query(Environment, Intent, Sample).Height;
		});
}

FVector FGatersTerrainSemanticField::MaterializedNormalAt(
	const FGatersEnvironment& Environment,
	const FVector2D& Point,
	float SampleDistance,
	float PadRadius,
	const FVector2D& RouteTarget)
{
	check(SampleDistance > 0.f);
	const float Gx = (MaterializedHeightAt(
		Environment, Point + FVector2D(SampleDistance, 0.f), PadRadius, RouteTarget)
		- MaterializedHeightAt(
			Environment, Point - FVector2D(SampleDistance, 0.f), PadRadius, RouteTarget))
		/ (2.f * SampleDistance);
	const float Gy = (MaterializedHeightAt(
		Environment, Point + FVector2D(0.f, SampleDistance), PadRadius, RouteTarget)
		- MaterializedHeightAt(
			Environment, Point - FVector2D(0.f, SampleDistance), PadRadius, RouteTarget))
		/ (2.f * SampleDistance);
	return FVector(-Gx, -Gy, 1.f).GetSafeNormal();
}

FVector FGatersTerrainSemanticField::MaterializedNormalAt(
	const FGatersEnvironment& Environment,
	const FGatersWorldIntentRecipe& Intent,
	const FVector2D& Point,
	const float SampleDistance,
	const float PadRadius,
	const FVector2D& RouteTarget)
{
	check(SampleDistance > 0.f);
	const float Gx = (MaterializedHeightAt(
		Environment, Intent, Point + FVector2D(SampleDistance, 0.f), PadRadius, RouteTarget)
		- MaterializedHeightAt(
			Environment, Intent, Point - FVector2D(SampleDistance, 0.f), PadRadius, RouteTarget))
		/ (2.f * SampleDistance);
	const float Gy = (MaterializedHeightAt(
		Environment, Intent, Point + FVector2D(0.f, SampleDistance), PadRadius, RouteTarget)
		- MaterializedHeightAt(
			Environment, Intent, Point - FVector2D(0.f, SampleDistance), PadRadius, RouteTarget))
		/ (2.f * SampleDistance);
	return FVector(-Gx, -Gy, 1.f).GetSafeNormal();
}

FGatersTerrainSemanticField FGatersTerrainSemanticField::Build(
	const FGatersEnvironment& Environment,
	int32 InCellsPerAxis,
	float InCellSize,
	float PadRadius,
	float FlatNormalZ,
	float SlopeNormalZ,
	const FVector2D& RouteTarget)
{
	const auto HeightAt = [&Environment, PadRadius, RouteTarget](const FVector2D& Point)
	{
		return MaterializedHeightAt(Environment, Point, PadRadius, RouteTarget);
	};
	const auto IsWetAt = [&Environment](const FVector2D&, const float Height)
	{
		return Environment.HasWater() && Height <= Environment.WaterHeight + 50.f;
	};
	return BuildSemanticField(
		InCellsPerAxis, InCellSize, PadRadius, FlatNormalZ, SlopeNormalZ,
		HeightAt, IsWetAt);
}

FGatersTerrainSemanticField FGatersTerrainSemanticField::Build(
	const FGatersEnvironment& Environment,
	const FGatersWorldIntentRecipe& Intent,
	const int32 InCellsPerAxis,
	const float InCellSize,
	const float PadRadius,
	const float FlatNormalZ,
	const float SlopeNormalZ,
	const FVector2D& RouteTarget)
{
	const auto HeightAt = [&Environment, &Intent, PadRadius, RouteTarget](
		const FVector2D& Point)
	{
		return MaterializedHeightAt(Environment, Intent, Point, PadRadius, RouteTarget);
	};
	const auto IsWetAt = [&Environment, &Intent](
		const FVector2D& Point, const float Height)
	{
		const FGatersIntentTerrainSample Terrain = FGatersIntentTerrainField::Query(
			Environment, Intent, Point);
		const FGatersEnvironment RegionalEnvironment = Environment.WithProfile(
			Terrain.Terrain, Terrain.Hydrology);
		const float GlobalWet = Environment.HasWater()
			&& Height <= Environment.WaterHeight + 50.f ? 1.f : 0.f;
		const float RegionalWet = RegionalEnvironment.HasWater()
			&& Height <= RegionalEnvironment.WaterHeight + 50.f ? 1.f : 0.f;
		return FMath::Lerp(GlobalWet, RegionalWet, Terrain.Influence) >= 0.5f;
	};
	return BuildSemanticField(
		InCellsPerAxis, InCellSize, PadRadius, FlatNormalZ, SlopeNormalZ,
		HeightAt, IsWetAt);
}

const FGatersTerrainSemanticSample& FGatersTerrainSemanticField::At(int32 X, int32 Y) const
{
	check(X >= 0 && X < CellsPerAxis && Y >= 0 && Y < CellsPerAxis);
	return Cells[X * CellsPerAxis + Y];
}

int32 FGatersTerrainSemanticField::Count(EGatersTerrainSemantic Type) const
{
	int32 Result = 0;
	for (const FGatersTerrainSemanticSample& Sample : Cells)
	{
		Result += Sample.Type == Type ? 1 : 0;
	}
	return Result;
}

int32 FGatersTerrainSemanticField::WalkableCount() const
{
	return Count(EGatersTerrainSemantic::Flat) + Count(EGatersTerrainSemantic::Slope);
}
