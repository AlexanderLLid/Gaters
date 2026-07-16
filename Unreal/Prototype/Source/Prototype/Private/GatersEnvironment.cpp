#include "GatersEnvironment.h"

namespace
{
int32 ScrambleSeed(int32 Seed)
{
	return static_cast<int32>(FMath::Fmod(
		FMath::Sin((Seed + 1) * 12.9898) * 100000000.0,
		2147483647.0));
}
}

FGatersEnvironment FGatersEnvironment::FromSeed(int32 InSeed, float InChunkSize)
{
	FGatersEnvironment Result;
	Result.Seed = InSeed;
	Result.ChunkSize = InChunkSize;

	FRandomStream Random(ScrambleSeed(InSeed));
	Result.Type = static_cast<EGatersEnvironment>(Random.RandRange(0, 3));
	Result.NoiseOffset = FVector2D(
		Random.FRandRange(-100000.f, 100000.f),
		Random.FRandRange(-100000.f, 100000.f));
	Result.RotationRadians = Random.FRandRange(0.f, 2.f * PI);
	Result.Phase = Random.FRandRange(-PI, PI);

	switch (Result.Type)
	{
	case EGatersEnvironment::Canyon:
		Result.WaterHeight = -1350.f;
		break;
	case EGatersEnvironment::Archipelago:
		Result.WaterHeight = 0.f;
		break;
	default:
		Result.WaterHeight = -100000.f;
		break;
	}
	return Result;
}

FVector2D FGatersEnvironment::Rotate(const FVector2D& Point) const
{
	const float C = FMath::Cos(RotationRadians);
	const float S = FMath::Sin(RotationRadians);
	return FVector2D(C * Point.X - S * Point.Y, S * Point.X + C * Point.Y);
}

float FGatersEnvironment::Fractal(const FVector2D& Point, float Frequency, int32 Octaves) const
{
	const FVector2D P = Rotate(Point) + NoiseOffset;
	float Sum = 0.f;
	float Weight = 1.f;
	float WeightSum = 0.f;
	for (int32 Octave = 0; Octave < Octaves; ++Octave)
	{
		Sum += FMath::PerlinNoise2D(P * Frequency) * Weight;
		WeightSum += Weight;
		Frequency *= 2.03f;
		Weight *= 0.5f;
	}
	return Sum / WeightSum;
}

float FGatersEnvironment::HeightAt(const FVector2D& Point) const
{
	switch (Type)
	{
	case EGatersEnvironment::Lowlands:
	{
		const float Broad = Fractal(Point, 0.000055f, 2) * 1150.f;
		const float Rolling = Fractal(
			Point + FVector2D(-11000.f, 7000.f), 0.00014f, 3) * 875.f;
		return Broad + Rolling;
	}

	case EGatersEnvironment::Mountains:
	{
		const float Broad = Fractal(Point, 0.000055f, 2);
		const float RidgeNoise = Fractal(Point + FVector2D(17000.f, -9000.f), 0.00015f, 3);
		const float Ridge = 1.f - FMath::Abs(RidgeNoise);
		const float MountainMask = FMath::SmoothStep(0.48f, 0.82f, Ridge);
		return MountainMask * 4200.f + Broad * 260.f
			+ Fractal(Point, 0.00065f, 2) * 80.f - 900.f;
	}

	case EGatersEnvironment::Canyon:
	{
		const FVector2D P = Rotate(Point);
		const float CenterY = FMath::Sin(P.X * 0.00042f + Phase) * 1800.f;
		const float Distance = FMath::Abs(P.Y - CenterY);
		const float CanyonMask = 1.f - FMath::SmoothStep(550.f, 2300.f, Distance);
		const float Plateau = 950.f + Fractal(Point, 0.00011f, 3) * 350.f;
		const float Terraced = FMath::GridSnap(Plateau, 180.f);
		return Terraced - CanyonMask * 2900.f + Fractal(Point, 0.00075f, 2) * 90.f;
	}

	case EGatersEnvironment::Archipelago:
	{
		const float Radius = Point.Size() / (ChunkSize * 0.5f);
		const float EdgeSink = FMath::Square(FMath::Max(0.f, Radius - 0.62f)) * 5200.f;
		const float Islands = Fractal(Point, 0.00016f, 4) * 1150.f;
		const float CenterIsland = FMath::Exp(-FMath::Square(Radius * 2.4f)) * 900.f;
		return Islands + CenterIsland + 180.f - EdgeSink;
	}
	}
	return 0.f;
}

float FGatersEnvironment::FootprintDrop(const FVector2D& Center, float Radius) const
{
	float MinHeight = HeightAt(Center);
	float MaxHeight = MinHeight;
	for (int32 Sample = 0; Sample < 12; ++Sample)
	{
		const float Angle = 2.f * PI * Sample / 12.f;
		const FVector2D Point = Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius;
		const float Height = HeightAt(Point);
		MinHeight = FMath::Min(MinHeight, Height);
		MaxHeight = FMath::Max(MaxHeight, Height);
	}
	return MaxHeight - MinHeight;
}

bool FGatersEnvironment::IsFootprintDry(
	const FVector2D& Center,
	float Radius,
	float Clearance) const
{
	if (HeightAt(Center) <= WaterHeight + Clearance)
	{
		return false;
	}
	for (int32 Sample = 0; Sample < 12; ++Sample)
	{
		const float Angle = 2.f * PI * Sample / 12.f;
		const FVector2D Point = Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius;
		if (HeightAt(Point) <= WaterHeight + Clearance)
		{
			return false;
		}
	}
	return true;
}

bool FGatersEnvironment::FindBaseSite(
	float MinDistance,
	float MaxDistance,
	float FootprintRadius,
	float MaxDrop,
	FVector2D& OutSite) const
{
	float BestDrop = TNumericLimits<float>::Max();
	FVector2D Best = FVector2D::ZeroVector;
	const float StartAngle = FMath::Fmod(FMath::Abs(ScrambleSeed(Seed)) * 0.000001f, 2.f * PI);

	for (int32 Ring = 0; Ring < 13; ++Ring)
	{
		const float Distance = FMath::Lerp(MinDistance + 2.f, MaxDistance, Ring / 12.f);
		for (int32 Step = 0; Step < 144; ++Step)
		{
			const float Angle = StartAngle + 2.f * PI * Step / 144.f;
			const FVector2D Candidate(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance);
			if (HeightAt(Candidate) <= WaterHeight + 100.f ||
				!IsFootprintDry(Candidate, FootprintRadius, 50.f))
			{
				continue;
			}
			const float Drop = FootprintDrop(Candidate, FootprintRadius);
			if (Drop < BestDrop)
			{
				BestDrop = Drop;
				Best = Candidate;
			}
		}
	}

	OutSite = Best;
	return BestDrop <= MaxDrop;
}

bool FGatersEnvironment::HasWater() const
{
	return Type == EGatersEnvironment::Canyon || Type == EGatersEnvironment::Archipelago;
}

FString FGatersEnvironment::Name() const
{
	switch (Type)
	{
	case EGatersEnvironment::Lowlands: return TEXT("lowlands");
	case EGatersEnvironment::Mountains: return TEXT("mountains");
	case EGatersEnvironment::Canyon: return TEXT("canyon");
	case EGatersEnvironment::Archipelago: return TEXT("archipelago");
	}
	return TEXT("unknown");
}
