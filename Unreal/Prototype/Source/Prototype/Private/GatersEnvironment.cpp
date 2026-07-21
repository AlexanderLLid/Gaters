#include "GatersEnvironment.h"

namespace
{
int32 ScrambleSeed(int32 Seed)
{
	return static_cast<int32>(FMath::Fmod(
		FMath::Sin((Seed + 1) * 12.9898) * 100000000.0,
		2147483647.0));
}

float SeedUnit(int32 Seed, uint32 Salt)
{
	uint32 Hash = static_cast<uint32>(Seed) + 0x9e3779b9u * (Salt + 1u);
	Hash = (Hash ^ (Hash >> 16)) * 0x7feb352du;
	Hash = (Hash ^ (Hash >> 15)) * 0x846ca68bu;
	Hash ^= Hash >> 16;
	return (Hash & 0x00ffffffu) / static_cast<float>(0x01000000u);
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

	if (Result.Type == EGatersEnvironment::Canyon)
	{
		Result.Hydrology = EGatersHydrology::River;
		Result.WaterHeight = -1350.f;
	}
	else if (Result.Type == EGatersEnvironment::Archipelago)
	{
		Result.Hydrology = EGatersHydrology::Ocean;
		Result.WaterHeight = -200.f;
	}
	else if (FMath::Abs(InSeed) % 3 == 2)
	{
		Result.Hydrology = EGatersHydrology::Lakes;
		// Local terrain is normalized to the arrival pad at Z=0. Lake datum must
		// remain below that pad; basin modifiers expose the water selectively.
		Result.WaterHeight = -200.f;
	}
	return Result;
}

FGatersEnvironment FGatersEnvironment::WithProfile(
	const EGatersEnvironment Terrain,
	const EGatersHydrology InHydrology,
	const FVector2D& ProcessCoordinateOffset,
	const float InLocalHydrologyRadius) const
{
	FGatersEnvironment Result = *this;
	Result.Type = Terrain;
	Result.Hydrology = InHydrology;
	Result.WaterHeight = InHydrology == EGatersHydrology::Dry
		? -100000.f
		: InHydrology == EGatersHydrology::River
			? -1350.f
			: -200.f;
	Result.LandformCoordinateOffset = ProcessCoordinateOffset;
	Result.LocalHydrologyRadius = InLocalHydrologyRadius;
	return Result;
}

FGatersEnvironment FGatersEnvironment::WithLandformProcesses(
	const FGatersLandformProcessRecipe& Recipe) const
{
	check(Recipe.Seed == Seed);
	check(FMath::IsNearlyEqual(Recipe.WorldSize, ChunkSize, 0.01f));
	FGatersEnvironment Result = *this;
	Result.LandformProcesses = Recipe;
	Result.LandformCoordinateOffset = FVector2D::ZeroVector;
	return Result;
}

FVector2D FGatersEnvironment::Rotate(const FVector2D& Point) const
{
	const float C = FMath::Cos(RotationRadians);
	const float S = FMath::Sin(RotationRadians);
	return FVector2D(C * Point.X - S * Point.Y, S * Point.X + C * Point.Y);
}

FVector2D FGatersEnvironment::LakeCenter(int32 Index) const
{
	const float Angle = Phase + (Index == 0 ? 0.f : 2.2f);
	const float BaseDistance = Index == 0 ? 6500.f : 10500.f;
	const float Distance = LocalHydrologyRadius > 0.f
		? FMath::Min(BaseDistance, LocalHydrologyRadius * 0.55f)
		: BaseDistance;
	return FVector2D(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance);
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
	float BaseHeight = 0.f;
	switch (Type)
	{
	case EGatersEnvironment::Lowlands:
	{
		const float Broad = Fractal(Point, 0.000055f, 2) * 1150.f;
		const float Rolling = Fractal(
			Point + FVector2D(-11000.f, 7000.f), 0.00014f, 3) * 875.f;
		BaseHeight = Broad + Rolling;
		break;
	}

	case EGatersEnvironment::Mountains:
	{
		const float Broad = Fractal(Point, 0.000032f, 3);
		const float RidgeNoise = Fractal(
			Point + FVector2D(17000.f, -9000.f), 0.000055f, 3);
		const float Ridge = FMath::Pow(
			FMath::Clamp(1.f - FMath::Abs(RidgeNoise) * 1.8f, 0.f, 1.f), 4.f);
		const float Massif = FMath::SmoothStep(-0.12f, 0.36f, Broad);
		const float MassifCore = FMath::SmoothStep(0.12f, 0.45f, Broad);
		const float Base = 350.f + Broad * 500.f;
		BaseHeight = Base + Massif * 800.f + MassifCore * Ridge * 5200.f
			+ Fractal(Point, 0.00028f, 2) * 60.f;
		break;
	}

	case EGatersEnvironment::Canyon:
	{
		const FVector2D P = Rotate(Point);
		const float CenterY = FMath::Sin(P.X * 0.00042f + Phase) * 1800.f;
		const float Distance = FMath::Abs(P.Y - CenterY);
		const float CanyonMask = 1.f - FMath::SmoothStep(550.f, 2300.f, Distance);
		const float Plateau = 950.f + Fractal(Point, 0.00011f, 3) * 350.f;
		const float Terraced = FMath::GridSnap(Plateau, 180.f);
		BaseHeight = Terraced - CanyonMask * 2900.f + Fractal(Point, 0.00075f, 2) * 90.f;
		break;
	}

	case EGatersEnvironment::Archipelago:
	{
		const auto WarpAt = [this](const FVector2D& Sample)
		{
			return FVector2D(
				Fractal(Sample + FVector2D(21000.f, -13000.f), 0.000075f, 2),
				Fractal(Sample + FVector2D(-17000.f, 26000.f), 0.000075f, 2));
		};
		const FVector2D WarpedPoint = Point + (WarpAt(Point) - WarpAt(FVector2D::ZeroVector))
			* 2200.f;
		const FVector2D P = Rotate(WarpedPoint);
		const auto IrregularMass = [this, &Point, &P](
			const FVector2D& Center, float InnerRadius, float OuterRadius, uint32 Salt)
		{
			const FVector2D NoiseShift(
				FMath::Lerp(-31000.f, 31000.f, SeedUnit(Seed, Salt)),
				FMath::Lerp(-31000.f, 31000.f, SeedUnit(Seed, Salt + 1u)));
			const float EdgeNoise = Fractal(
				Point + NoiseShift, 2.f / FMath::Max(OuterRadius, 1.f), 3);
			const float Distance = static_cast<float>((P - Center).Size())
				+ EdgeNoise * OuterRadius * 0.65f;
			return 1.f - FMath::SmoothStep(InnerRadius, OuterRadius, Distance);
		};
		float LandMass = IrregularMass(
			FVector2D::ZeroVector, 6200.f, 10200.f, 101u);
		const int32 SatelliteCount = 1 + static_cast<int32>(
			FMath::Fmod(FMath::Abs(static_cast<double>(ScrambleSeed(Seed))), 3.0));
		for (int32 IslandIndex = 0; IslandIndex < SatelliteCount; ++IslandIndex)
		{
			const uint32 Salt = static_cast<uint32>(IslandIndex) * 4u + 2u;
			const float EvenSpacing = 2.f * PI * IslandIndex / SatelliteCount;
			const float Angle = Phase + EvenSpacing
				+ FMath::Lerp(-0.18f, 0.18f, SeedUnit(Seed, Salt));
			const float Distance = FMath::Lerp(14000.f, 15000.f, SeedUnit(Seed, Salt + 1u));
			const float InnerRadius = FMath::Lerp(1800.f, 2500.f, SeedUnit(Seed, Salt + 2u));
			const float OuterRadius = InnerRadius
				+ FMath::Lerp(1400.f, 2000.f, SeedUnit(Seed, Salt + 3u));
			const FVector2D Center(
				FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance);
			const float IslandMass = IrregularMass(
				Center, InnerRadius, OuterRadius, Salt + 20u);
			LandMass = FMath::Max(LandMass, IslandMass);
		}
		const float CoastVariation = Fractal(
			Point + FVector2D(7300.f, -5100.f), 0.00012f, 3) * 0.16f;
		const float SurfaceVariation = Fractal(Point, 0.00042f, 2) * 90.f;
		BaseHeight = -480.f + (LandMass + CoastVariation) * 1250.f + SurfaceVariation;
		break;
	}
	}

	if (LandformProcesses.IsSet())
	{
		BaseHeight = FGatersLandformProcessField::Query(
			LandformProcesses.GetValue(),
			Point + LandformCoordinateOffset,
			BaseHeight).Height;
	}

	if (Hydrology == EGatersHydrology::Lakes)
	{
		// Terrain and surfaces consume the same lake footprints. Regional profiles
		// scale those footprints to their declared region; global lakes stay unchanged.
		const TArray<FGatersWaterSurface> Lakes = WaterSurfaces();
		const float InnerA = LocalHydrologyRadius > 0.f
			? Lakes[0].HalfExtent * 0.55f : 1700.f;
		const float InnerB = LocalHydrologyRadius > 0.f
			? Lakes[1].HalfExtent * 0.55f : 1400.f;
		const float BasinA = 1.f - FMath::SmoothStep(
			InnerA, Lakes[0].HalfExtent,
			static_cast<float>((Point - Lakes[0].Center).Size()));
		const float BasinB = 1.f - FMath::SmoothStep(
			InnerB, Lakes[1].HalfExtent,
			static_cast<float>((Point - Lakes[1].Center).Size()));
		const float Basin = FMath::Max(BasinA, BasinB);
		const float LakeFloor = WaterHeight - 120.f
			+ Fractal(Point + FVector2D(4100.f, 2300.f), 0.0004f, 2) * 45.f;
		BaseHeight = FMath::Lerp(BaseHeight, FMath::Min(BaseHeight, LakeFloor), Basin);
	}
	else if (Hydrology == EGatersHydrology::River)
	{
		// River hydrology is orthogonal to terrain family. Canyon terrain already
		// supplies a deep channel; other families still need terrain below the datum.
		const FVector2D P = Rotate(Point);
		const float CenterY = FMath::Sin(P.X * 0.00042f + Phase) * 1800.f;
		const float Distance = FMath::Abs(P.Y - CenterY);
		const float Channel = 1.f - FMath::SmoothStep(2200.f, 4200.f, Distance);
		const float RiverFloor = WaterHeight - 120.f
			+ Fractal(Point + FVector2D(-2700.f, 5900.f), 0.0005f, 2) * 35.f;
		BaseHeight = FMath::Lerp(
			BaseHeight, FMath::Min(BaseHeight, RiverFloor), Channel);
	}
	return BaseHeight;
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
	return Hydrology != EGatersHydrology::Dry;
}

TArray<FGatersWaterSurface> FGatersEnvironment::WaterSurfaces() const
{
	if (Hydrology == EGatersHydrology::Lakes)
	{
		const float LakeAExtent = LocalHydrologyRadius > 0.f
			? FMath::Min(3600.f, LocalHydrologyRadius * 0.35f) : 3600.f;
		const float LakeBExtent = LocalHydrologyRadius > 0.f
			? FMath::Min(3100.f, LocalHydrologyRadius * 0.35f) : 3100.f;
		return { { LakeCenter(0), LakeAExtent }, { LakeCenter(1), LakeBExtent } };
	}
	if (HasWater())
	{
		return { { FVector2D::ZeroVector, ChunkSize * 8.f } };
	}
	return {};
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

FString FGatersEnvironment::HydrologyName() const
{
	switch (Hydrology)
	{
	case EGatersHydrology::Dry: return TEXT("dry");
	case EGatersHydrology::Lakes: return TEXT("lakes");
	case EGatersHydrology::River: return TEXT("river");
	case EGatersHydrology::Ocean: return TEXT("ocean");
	}
	return TEXT("unknown");
}
