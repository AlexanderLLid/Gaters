#include "GatersLandformProcessField.h"

#include "GatersEnvironment.h"

namespace
{
float SeedUnit(int32 Seed, uint32 Salt)
{
	uint32 Hash = static_cast<uint32>(Seed) + 0x9e3779b9u * (Salt + 1u);
	Hash = (Hash ^ (Hash >> 16)) * 0x7feb352du;
	Hash = (Hash ^ (Hash >> 15)) * 0x846ca68bu;
	Hash ^= Hash >> 16;
	return (Hash & 0x00ffffffu) / static_cast<float>(0x01000000u);
}

float FractalNoise(const FVector2D& Point, float Frequency, int32 Octaves)
{
	float Sum = 0.f;
	float Weight = 1.f;
	float WeightSum = 0.f;
	for (int32 Octave = 0; Octave < Octaves; ++Octave)
	{
		Sum += FMath::PerlinNoise2D(Point * Frequency) * Weight;
		WeightSum += Weight;
		Frequency *= 2.03f;
		Weight *= 0.5f;
	}
	return Sum / WeightSum;
}

FGatersEnvironmentTargetProfile BlendProfile(
	const FGatersEnvironmentTargetProfile& A,
	const FGatersEnvironmentTargetProfile& B,
	float Alpha)
{
	FGatersEnvironmentTargetProfile Result;
	Result.Relief = FMath::Lerp(A.Relief, B.Relief, Alpha);
	Result.Temperature = FMath::Lerp(A.Temperature, B.Temperature, Alpha);
	Result.Moisture = FMath::Lerp(A.Moisture, B.Moisture, Alpha);
	Result.SurfaceWater = FMath::Lerp(A.SurfaceWater, B.SurfaceWater, Alpha);
	Result.Volcanism = FMath::Lerp(A.Volcanism, B.Volcanism, Alpha);
	Result.Ice = FMath::Lerp(A.Ice, B.Ice, Alpha);
	Result.Vegetation = FMath::Lerp(A.Vegetation, B.Vegetation, Alpha);
	Result.ExposedRock = FMath::Lerp(A.ExposedRock, B.ExposedRock, Alpha);
	return Result;
}

FGatersEnvironmentTargetProfile ProfileAt(
	const FGatersLandformProcessRecipe& Recipe,
	const FVector2D& Point,
	float& OutRegionInfluence)
{
	FGatersEnvironmentTargetProfile Result = Recipe.Global;
	OutRegionInfluence = 0.f;
	for (const FGatersCompiledEnvironmentRegion& Region : Recipe.Regions)
	{
		const float Distance = static_cast<float>((Point - Region.Center).Size());
		const float Influence = 1.f - FMath::SmoothStep(
			Region.Radius * 0.65f, Region.Radius, Distance);
		if (Influence > 0.f)
		{
			Result = BlendProfile(Result, Region.Profile, Influence);
			OutRegionInfluence = FMath::Max(OutRegionInfluence, Influence);
		}
	}
	return Result;
}

FVector2D Rotate(const FVector2D& Point, float Radians)
{
	const float C = FMath::Cos(Radians);
	const float S = FMath::Sin(Radians);
	return FVector2D(C * Point.X - S * Point.Y, S * Point.X + C * Point.Y);
}

bool IsProfileValid(const FGatersEnvironmentTargetProfile& Profile)
{
	for (const float Value : {
		Profile.Relief, Profile.Temperature, Profile.Moisture, Profile.SurfaceWater,
		Profile.Volcanism, Profile.Ice, Profile.Vegetation, Profile.ExposedRock})
	{
		if (!FMath::IsFinite(Value) || Value < 0.f || Value > 1.f)
		{
			return false;
		}
	}
	return true;
}

void AddIssue(
	FGatersLandformProcessCompileResult& Result,
	const TCHAR* RuleId,
	const FString& Message)
{
	Result.Issues.Add({RuleId, Message});
}
}

FGatersLandformProcessCompileResult FGatersLandformProcessField::Compile(
	const FGatersEnvironment& Environment,
	const FGatersCompiledEnvironmentBrief& Intent,
	const TArray<FGatersLandformProtectedRegion>& ProtectedRegions,
	int32 CandidateIndex)
{
	FGatersLandformProcessCompileResult Result;
	Result.Recipe.Seed = Intent.Seed;
	Result.Recipe.CandidateIndex = CandidateIndex;
	if (CandidateIndex > 0)
	{
		constexpr int32 ScaleBands = 7;
		const int32 ScaleBand = (CandidateIndex - 1) % ScaleBands;
		Result.Recipe.FeatureScale = FMath::Lerp(
			0.55f, 1.65f,
			static_cast<float>(ScaleBand) / static_cast<float>(ScaleBands - 1));
		Result.Recipe.DissectionScale = FMath::Lerp(
			0.25f, 1.75f,
			static_cast<float>(ScaleBand) / static_cast<float>(ScaleBands - 1));
		if (ScaleBand < ScaleBands - 1)
		{
			Result.Recipe.RuggednessScale =
				(1.f - Intent.LandAccess.WalkableLand) * FMath::Lerp(
					0.42f, 1.26f,
					static_cast<float>(ScaleBand)
						/ static_cast<float>(ScaleBands - 1));
		}
	}
	Result.Recipe.WorldSize = Intent.WorldSize;
	Result.Recipe.bHasWater = Environment.HasWater();
	Result.Recipe.WaterHeight = Environment.WaterHeight;
	Result.Recipe.Global = Intent.Global;
	Result.Recipe.Regions = Intent.Regions;
	Result.Recipe.ProtectedRegions = ProtectedRegions;

	if (Intent.CompilerVersion != 2 || Intent.BriefVersion != 2)
	{
		AddIssue(Result, TEXT("landform.version"),
			TEXT("Landform processes require environment brief compiler and brief version 2."));
	}
	if (CandidateIndex < 0)
	{
		AddIssue(Result, TEXT("landform.candidate"),
			TEXT("Landform candidate index must be non-negative."));
	}
	if (Intent.Seed != Environment.Seed)
	{
		AddIssue(Result, TEXT("landform.seed"),
			TEXT("Environment and environment brief seeds must match."));
	}
	if (!FMath::IsNearlyEqual(Intent.WorldSize, Environment.ChunkSize, 0.01f))
	{
		AddIssue(Result, TEXT("landform.world_size"),
			TEXT("Environment and environment brief world sizes must match."));
	}
	if (!IsProfileValid(Intent.Global))
	{
		AddIssue(Result, TEXT("landform.profile"),
			TEXT("Global physical targets must be finite and inside [0, 1]."));
	}

	TSet<FString> RegionIds;
	const float HalfWorld = Intent.WorldSize * 0.5f;
	for (const FGatersCompiledEnvironmentRegion& Region : Intent.Regions)
	{
		if (Region.Id.IsEmpty() || RegionIds.Contains(Region.Id))
		{
			AddIssue(Result, TEXT("landform.region.id"),
				TEXT("Compiled region IDs must be non-empty and unique."));
		}
		RegionIds.Add(Region.Id);
		if (!IsProfileValid(Region.Profile))
		{
			AddIssue(Result, TEXT("landform.profile"), FString::Printf(
				TEXT("Region %s physical targets must be finite and inside [0, 1]."),
				*Region.Id));
		}
		if (Region.Center.ContainsNaN() || !FMath::IsFinite(Region.Radius)
			|| Region.Radius <= 0.f || Region.Radius > HalfWorld)
		{
			AddIssue(Result, TEXT("landform.region.geometry"), FString::Printf(
				TEXT("Region %s requires a finite center and 0 < radius <= half world."),
				*Region.Id));
			continue;
		}
		if (FMath::Abs(Region.Center.X) + Region.Radius > HalfWorld
			|| FMath::Abs(Region.Center.Y) + Region.Radius > HalfWorld)
		{
			AddIssue(Result, TEXT("landform.region.bounds"), FString::Printf(
				TEXT("Region %s must remain inside world bounds."), *Region.Id));
		}
	}

	TSet<FString> ProtectedIds;
	for (const FGatersLandformProtectedRegion& Region : ProtectedRegions)
	{
		if (Region.Id.IsEmpty() || ProtectedIds.Contains(Region.Id))
		{
			AddIssue(Result, TEXT("landform.protected.id"),
				TEXT("Protected region IDs must be non-empty and unique."));
		}
		ProtectedIds.Add(Region.Id);
		if (Region.Center.ContainsNaN() || !FMath::IsFinite(Region.InnerRadius)
			|| !FMath::IsFinite(Region.OuterRadius) || Region.InnerRadius <= 0.f
			|| Region.OuterRadius < Region.InnerRadius)
		{
			AddIssue(Result, TEXT("landform.protected.geometry"), FString::Printf(
				TEXT("Protected region %s requires finite geometry and 0 < inner <= outer."),
				*Region.Id));
		}
	}
	return Result;
}

FGatersLandformProcessSample FGatersLandformProcessField::Query(
	const FGatersLandformProcessRecipe& Recipe,
	const FVector2D& Point,
	float BaseHeight)
{
	FGatersLandformProcessSample Result;
	Result.BaseHeight = BaseHeight;
	const FGatersEnvironmentTargetProfile Profile =
		QueryProfile(Recipe, Point, &Result.RegionInfluence);
	for (const FGatersLandformProtectedRegion& Region : Recipe.ProtectedRegions)
	{
		const float Distance = static_cast<float>((Point - Region.Center).Size());
		const float Influence = Region.OuterRadius > Region.InnerRadius
			? FMath::SmoothStep(Region.InnerRadius, Region.OuterRadius, Distance)
			: (Distance <= Region.InnerRadius ? 0.f : 1.f);
		Result.ProcessInfluence = FMath::Min(Result.ProcessInfluence, Influence);
	}

	const float ReliefScale = FMath::Lerp(0.55f, 1.45f, Profile.Relief);
	Result.ReliefContribution = BaseHeight * (ReliefScale - 1.f)
		* Result.ProcessInfluence;

	const float FeatureScale = FMath::Max(Recipe.FeatureScale, 0.01f);
	const float NoiseScale = FMath::Max(Recipe.WorldSize, 1.f);
	const uint32 CandidateSalt = static_cast<uint32>(Recipe.CandidateIndex) * 16u;
	const FVector2D NoiseOffset(
		SeedUnit(Recipe.Seed, CandidateSalt + 1u) * 93000.f,
		SeedUnit(Recipe.Seed, CandidateSalt + 2u) * 93000.f);
	const float UpliftNoise = FMath::PerlinNoise2D(
		(Point + NoiseOffset) / (NoiseScale * 0.18f * FeatureScale));
	const float UpliftMask = FMath::SmoothStep(0.05f, 0.65f, UpliftNoise);
	Result.UpliftContribution = UpliftMask * Profile.Relief * 1200.f
		* Result.ProcessInfluence;

	const FVector2D VolcanoCenter(
		FMath::Lerp(-0.08f, 0.08f, SeedUnit(Recipe.Seed, CandidateSalt + 3u))
			* Recipe.WorldSize,
		FMath::Lerp(-0.08f, 0.08f, SeedUnit(Recipe.Seed, CandidateSalt + 4u))
			* Recipe.WorldSize);
	const float VolcanoRadius = FMath::Max(
		Recipe.WorldSize * 0.12f * FeatureScale, 1.f);
	const float VolcanoDistance = static_cast<float>((Point - VolcanoCenter).Size());
	const float VolcanoMask = 1.f - FMath::SmoothStep(
		VolcanoRadius * 0.12f, VolcanoRadius, VolcanoDistance);
	Result.VolcanicContribution = VolcanoMask * Profile.Volcanism * 3600.f
		* Result.ProcessInfluence;

	const float ValleyAngle = SeedUnit(Recipe.Seed, CandidateSalt + 5u) * 2.f * PI;
	const float ValleyPhase = FMath::Lerp(
		-PI, PI, SeedUnit(Recipe.Seed, CandidateSalt + 6u));
	const FVector2D ValleyPoint = Rotate(Point, ValleyAngle);
	const float ValleyCenter = FMath::Sin(
		ValleyPoint.X / FMath::Max(
			Recipe.WorldSize * 0.09f * FeatureScale, 1.f) + ValleyPhase)
		* Recipe.WorldSize * 0.025f;
	const float ValleyDistance = FMath::Abs(ValleyPoint.Y - ValleyCenter);
	const float ValleyMask = 1.f - FMath::SmoothStep(
		Recipe.WorldSize * 0.01f * FeatureScale,
		Recipe.WorldSize * 0.05f * FeatureScale, ValleyDistance);
	Result.GlacialContribution = -ValleyMask * Profile.Ice
		* FMath::Lerp(650.f, 1450.f, Profile.Relief) * Result.ProcessInfluence;

	const float DissectionAngle =
		SeedUnit(Recipe.Seed, CandidateSalt + 7u) * 2.f * PI;
	const float DissectionPhase = FMath::Lerp(
		-PI, PI, SeedUnit(Recipe.Seed, CandidateSalt + 8u));
	const FVector2D DissectionPoint = Rotate(Point, DissectionAngle);
	const float DissectionCenter = FMath::Sin(
		DissectionPoint.X / FMath::Max(
			Recipe.WorldSize * 0.14f * FeatureScale, 1.f) + DissectionPhase)
		* Recipe.WorldSize * 0.05f;
	const float DissectionDistance = FMath::Abs(
		DissectionPoint.Y - DissectionCenter);
	const float DissectionMask = 1.f - FMath::SmoothStep(
		Recipe.WorldSize * 0.006f * FeatureScale,
		Recipe.WorldSize * 0.035f * FeatureScale,
		DissectionDistance);
	const float DissectionResponse = FMath::Lerp(
		0.5f, 1.f, FMath::Max(Profile.Relief, Profile.SurfaceWater));
	Result.DissectionContribution = -DissectionMask * Recipe.WorldSize * 0.008f
		* Recipe.DissectionScale * DissectionResponse * Result.ProcessInfluence;

	if (Recipe.RuggednessScale > 0.f)
	{
		const FVector2D RuggednessOffsetA(
			SeedUnit(Recipe.Seed, CandidateSalt + 9u) * 117000.f,
			SeedUnit(Recipe.Seed, CandidateSalt + 10u) * 117000.f);
		const FVector2D RuggednessOffsetB(
			SeedUnit(Recipe.Seed, CandidateSalt + 11u) * 173000.f,
			SeedUnit(Recipe.Seed, CandidateSalt + 12u) * 173000.f);
		const float BroadFrequency = 1.f / FMath::Max(
			Recipe.WorldSize * 0.075f * FeatureScale, 1.f);
		const float RidgeFrequency = 1.f / FMath::Max(
			Recipe.WorldSize * 0.0225f * FeatureScale, 1.f);
		const float Broad = FractalNoise(Point + RuggednessOffsetA, BroadFrequency, 3);
		const float RidgeNoise = FractalNoise(
			Point + RuggednessOffsetB, RidgeFrequency, 3);
		const float Massif = FMath::SmoothStep(-0.12f, 0.36f, Broad);
		const float MassifCore = FMath::SmoothStep(0.02f, 0.4f, Broad);
		const float Ridge = FMath::SmoothStep(0.f, 1.f,
			FMath::Clamp(1.f - FMath::Abs(RidgeNoise) * 1.45f, 0.f, 1.f));
		const float Strength = Recipe.RuggednessScale / 0.35f;
		const float RuggednessValleyMask = 1.f - FMath::SmoothStep(
			Recipe.WorldSize * 0.006f * FeatureScale,
			Recipe.WorldSize * 0.05f * FeatureScale,
			DissectionDistance);
		const float RuggednessHeight = (Massif * Recipe.WorldSize * 0.002f
			+ MassifCore * Ridge * Recipe.WorldSize * 0.037f) * Strength
			* (1.f - RuggednessValleyMask);
		const float LandInfluence = Recipe.bHasWater
			? FMath::SmoothStep(
				Recipe.WaterHeight + 50.f, Recipe.WaterHeight + 450.f, BaseHeight)
			: 1.f;
		Result.RuggednessContribution =
			RuggednessHeight * LandInfluence * Result.ProcessInfluence;
	}

	Result.Height = Result.BaseHeight + Result.ReliefContribution
		+ Result.UpliftContribution + Result.VolcanicContribution
		+ Result.GlacialContribution + Result.DissectionContribution
		+ Result.RuggednessContribution;
	return Result;
}

FGatersEnvironmentTargetProfile FGatersLandformProcessField::QueryProfile(
	const FGatersLandformProcessRecipe& Recipe,
	const FVector2D& Point,
	float* OutRegionInfluence)
{
	float RegionInfluence = 0.f;
	const FGatersEnvironmentTargetProfile Result =
		ProfileAt(Recipe, Point, RegionInfluence);
	if (OutRegionInfluence)
	{
		*OutRegionInfluence = RegionInfluence;
	}
	return Result;
}
