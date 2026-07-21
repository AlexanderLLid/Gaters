#include "GatersWorldIntent.h"

namespace
{
uint32 IntentHash(const int32 Seed, const uint32 Salt)
{
	uint32 Hash = static_cast<uint32>(Seed) + 0x9e3779b9u * (Salt + 1u);
	Hash = (Hash ^ (Hash >> 16)) * 0x7feb352du;
	Hash = (Hash ^ (Hash >> 15)) * 0x846ca68bu;
	return Hash ^ (Hash >> 16);
}

float HashUnit(const int32 Seed, const uint32 Salt)
{
	return (IntentHash(Seed, Salt) & 0x00ffffffu) /
		static_cast<float>(0x01000000u);
}

FGatersWorldRegionIntent MakeRegion(
	const int32 Seed,
	const int32 Index,
	const FVector2D& Center,
	const float Radius)
{
	const uint32 Salt = 16u + static_cast<uint32>(Index) * 12u;
	FGatersWorldRegionIntent Result;
	Result.Id = FString::Printf(TEXT("intent:%d:region:%d"), Seed, Index);
	Result.Center = Center;
	Result.Radius = Radius;
	Result.TerrainTendency = static_cast<EGatersEnvironment>(
		IntentHash(Seed, Salt) % 4u);
	Result.HydrologyTendency = static_cast<EGatersHydrology>(
		IntentHash(Seed, Salt + 1u) % 4u);
	Result.VegetationOpportunity = FMath::Square(HashUnit(Seed, Salt + 2u));
	Result.StoneOpportunity = HashUnit(Seed, Salt + 3u);
	if (Result.VegetationOpportunity < 0.08f)
	{
		Result.VegetationOpportunity = 0.f;
	}
	if (Result.StoneOpportunity < 0.08f)
	{
		Result.StoneOpportunity = 0.f;
	}
	Result.LandmarkOpportunity = HashUnit(Seed, Salt + 4u);
	Result.TravelFriction = HashUnit(Seed, Salt + 5u);
	return Result;
}
}

FGatersWorldIntentRecipe FGatersWorldIntentRecipe::Generate(
	const int32 InSeed,
	const float InWorldSize)
{
	return Generate(FGatersEnvironment::FromSeed(InSeed, InWorldSize));
}

FGatersWorldIntentRecipe FGatersWorldIntentRecipe::Generate(
	const FGatersEnvironment& Environment)
{
	const int32 InSeed = Environment.Seed;
	const float InWorldSize = Environment.ChunkSize;
	check(InWorldSize > 0.f);
	FGatersWorldIntentRecipe Result;
	Result.Seed = InSeed;
	Result.WorldSize = InWorldSize;
	Result.Regions.Add(MakeRegion(
		InSeed, 0, FVector2D::ZeroVector, InWorldSize * 2.f));
	Result.Regions[0].TerrainTendency = Environment.Type;
	Result.Regions[0].HydrologyTendency = Environment.Hydrology;

	for (int32 Index = 1; Index <= 2; ++Index)
	{
		const uint32 Salt = 2u + static_cast<uint32>(Index) * 4u;
		const float Angle = HashUnit(InSeed, Salt) * 2.f * PI;
		const float Distance = FMath::Lerp(
			InWorldSize * 0.18f, InWorldSize * 0.30f,
			HashUnit(InSeed, Salt + 1u));
		const float Radius = FMath::Lerp(
			InWorldSize * 0.06f, InWorldSize * 0.10f,
			HashUnit(InSeed, Salt + 2u));
		Result.Regions.Add(MakeRegion(
			InSeed,
			Index,
			FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Distance,
			Radius));
	}
	return Result;
}

const FGatersWorldRegionIntent& FGatersWorldIntentRecipe::At(
	const FVector2D& Point) const
{
	check(!Regions.IsEmpty());
	int32 BestIndex = 0;
	float BestInfluence = 0.f;
	for (int32 Index = 1; Index < Regions.Num(); ++Index)
	{
		const FGatersWorldRegionIntent& Region = Regions[Index];
		const float Influence = 1.f - FVector2D::Distance(Point, Region.Center) / Region.Radius;
		if (Influence > BestInfluence)
		{
			BestInfluence = Influence;
			BestIndex = Index;
		}
	}
	return Regions[BestIndex];
}

bool FGatersWorldIntentRecipe::Validate(TArray<FString>& OutErrors) const
{
	if (Version != 2 || WorldSize <= 0.f || Regions.IsEmpty())
	{
		OutErrors.Add(TEXT("world intent root metadata is invalid"));
	}
	TSet<FString> Ids;
	for (const FGatersWorldRegionIntent& Region : Regions)
	{
		if (Region.Id.IsEmpty() || Ids.Contains(Region.Id))
		{
			OutErrors.Add(FString::Printf(
				TEXT("duplicate or empty world intent region: %s"), *Region.Id));
		}
		Ids.Add(Region.Id);
		if (Region.Center.ContainsNaN() || !FMath::IsFinite(Region.Radius) || Region.Radius <= 0.f)
		{
			OutErrors.Add(FString::Printf(TEXT("invalid region bounds: %s"), *Region.Id));
		}
		for (const float Value : {
			Region.VegetationOpportunity, Region.StoneOpportunity,
			Region.LandmarkOpportunity, Region.TravelFriction})
		{
			if (!FMath::IsFinite(Value) || Value < 0.f || Value > 1.f)
			{
				OutErrors.Add(FString::Printf(TEXT("invalid region opportunity: %s"), *Region.Id));
				break;
			}
		}
	}
	return OutErrors.IsEmpty();
}
