#include "GatersIntentTerrainField.h"

FGatersIntentTerrainSample FGatersIntentTerrainField::Query(
	const FGatersEnvironment& Environment,
	const FGatersWorldIntentRecipe& Intent,
	const FVector2D& Point)
{
	check(!Intent.Regions.IsEmpty());
	const FGatersWorldRegionIntent* Region = &Intent.Regions[0];
	float Influence = 0.f;
	for (int32 Index = 1; Index < Intent.Regions.Num(); ++Index)
	{
		const FGatersWorldRegionIntent& Candidate = Intent.Regions[Index];
		const float Distance = FVector2D::Distance(Point, Candidate.Center);
		const float CandidateInfluence = 1.f - FMath::SmoothStep(
			Candidate.Radius * 0.55f, Candidate.Radius, Distance);
		if (CandidateInfluence > Influence)
		{
			Region = &Candidate;
			Influence = CandidateInfluence;
		}
	}

	FGatersIntentTerrainSample Result;
	Result.RegionId = Region->Id;
	Result.Influence = Influence;
	Result.Terrain = Region->TerrainTendency;
	Result.Hydrology = Region->HydrologyTendency;
	const float GlobalHeight = Environment.HeightAt(Point);
	if (Influence <= 0.f)
	{
		Result.Height = GlobalHeight;
		return Result;
	}
	const FGatersEnvironment RegionalEnvironment = Environment.WithProfile(
		Region->TerrainTendency,
		Region->HydrologyTendency,
		Region->Center,
		Region->Radius);
	const float RegionalHeight = RegionalEnvironment.HeightAt(Point - Region->Center);
	Result.Height = FMath::Lerp(GlobalHeight, RegionalHeight, Influence);
	return Result;
}
