#include "GatersTerrainSemanticField.h"

float FGatersTerrainSemanticField::MaterializedHeightAt(
	const FGatersEnvironment& Environment,
	const FVector2D& Point,
	float PadRadius,
	const FVector2D& RouteTarget)
{
	const float RadialBlend = FMath::SmoothStep(
		PadRadius, PadRadius + 800.f, static_cast<float>(Point.Size()));
	const float RawHeight = Environment.HeightAt(Point);
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
			const float TargetHeight = FMath::Max(
				static_cast<float>(Environment.HeightAt(RouteTarget)), DryHeight);
			const float RouteAlpha = FMath::SmoothStep(PadRadius, RouteLength, Along);
			const float RouteHeight = FMath::Lerp(0.f,
				FMath::Lerp(DryHeight, TargetHeight, RouteAlpha),
				FMath::SmoothStep(PadRadius, PadRadius + 500.f, Along));
			return FMath::Lerp(RawHeight * RadialBlend, RouteHeight, Influence);
		}
	}
	return RawHeight * RadialBlend;
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

FGatersTerrainSemanticField FGatersTerrainSemanticField::Build(
	const FGatersEnvironment& Environment,
	int32 InCellsPerAxis,
	float InCellSize,
	float PadRadius,
	float FlatNormalZ,
	float SlopeNormalZ,
	const FVector2D& RouteTarget)
{
	check(InCellsPerAxis > 0);
	check(InCellSize > 0.f);
	check(FlatNormalZ >= SlopeNormalZ);

	FGatersTerrainSemanticField Result;
	Result.CellsPerAxis = InCellsPerAxis;
	Result.CellSize = InCellSize;
	Result.Cells.SetNum(InCellsPerAxis * InCellsPerAxis);
	const int32 Half = InCellsPerAxis / 2;
	const float Eps = InCellSize * 0.25f;

	for (int32 X = 0; X < InCellsPerAxis; ++X)
	{
		for (int32 Y = 0; Y < InCellsPerAxis; ++Y)
		{
			const FVector2D Point((X - Half) * InCellSize, (Y - Half) * InCellSize);
			FGatersTerrainSemanticSample& Sample = Result.Cells[X * InCellsPerAxis + Y];
			Sample.Height = MaterializedHeightAt(Environment, Point, PadRadius, RouteTarget);
			const float Gx = (MaterializedHeightAt(Environment, Point + FVector2D(Eps, 0.f), PadRadius, RouteTarget)
				- MaterializedHeightAt(Environment, Point - FVector2D(Eps, 0.f), PadRadius, RouteTarget)) / (2.f * Eps);
			const float Gy = (MaterializedHeightAt(Environment, Point + FVector2D(0.f, Eps), PadRadius, RouteTarget)
				- MaterializedHeightAt(Environment, Point - FVector2D(0.f, Eps), PadRadius, RouteTarget)) / (2.f * Eps);
			Sample.NormalZ = 1.f / FMath::Sqrt(1.f + Gx * Gx + Gy * Gy);

			if (Point.SizeSquared() <= FMath::Square(PadRadius))
			{
				Sample.Type = EGatersTerrainSemantic::Flat;
			}
			else if (Environment.HasWater() && Sample.Height <= Environment.WaterHeight + 50.f)
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
