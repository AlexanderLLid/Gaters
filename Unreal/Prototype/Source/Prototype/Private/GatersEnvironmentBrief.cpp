#include "GatersEnvironmentBrief.h"

namespace
{
uint32 BriefHash(const int32 Seed, const uint32 Salt)
{
	uint32 Hash = static_cast<uint32>(Seed) + 0x9e3779b9u * (Salt + 1u);
	Hash = (Hash ^ (Hash >> 16)) * 0x7feb352du;
	Hash = (Hash ^ (Hash >> 15)) * 0x846ca68bu;
	return Hash ^ (Hash >> 16);
}

float HashUnit(const int32 Seed, const uint32 Salt)
{
	return (BriefHash(Seed, Salt) & 0x00ffffffu)
		/ static_cast<float>(0x01000000u);
}

void AddIssue(
	FGatersEnvironmentBriefCompileResult& Result,
	const TCHAR* RuleId,
	const FString& Message,
	const FString& RegionId = FString())
{
	Result.Issues.Add({RuleId, RegionId, Message});
}

void ValidateRange(
	FGatersEnvironmentBriefCompileResult& Result,
	const FGatersEnvironmentSignalRange& Range,
	const TCHAR* Name,
	const FString& RegionId)
{
	if (!FMath::IsFinite(Range.Min) || !FMath::IsFinite(Range.Max)
		|| Range.Min < 0.f || Range.Max > 1.f || Range.Min > Range.Max)
	{
		AddIssue(Result, TEXT("brief.range"), FString::Printf(
			TEXT("invalid %s range [%.3f, %.3f]"), Name, Range.Min, Range.Max),
			RegionId);
	}
}

void ValidateProfile(
	FGatersEnvironmentBriefCompileResult& Result,
	const FGatersEnvironmentTargetRanges& Profile,
	const FString& RegionId)
{
	ValidateRange(Result, Profile.Relief, TEXT("relief"), RegionId);
	ValidateRange(Result, Profile.Temperature, TEXT("temperature"), RegionId);
	ValidateRange(Result, Profile.Moisture, TEXT("moisture"), RegionId);
	ValidateRange(Result, Profile.SurfaceWater, TEXT("surface-water"), RegionId);
	ValidateRange(Result, Profile.Volcanism, TEXT("volcanism"), RegionId);
	ValidateRange(Result, Profile.Ice, TEXT("ice"), RegionId);
	ValidateRange(Result, Profile.Vegetation, TEXT("vegetation"), RegionId);
	ValidateRange(Result, Profile.ExposedRock, TEXT("exposed-rock"), RegionId);
}

float Sample(
	const FGatersEnvironmentSignalRange& Range,
	const int32 Seed,
	const uint32 Salt)
{
	return FMath::Lerp(Range.Min, Range.Max, HashUnit(Seed, Salt));
}

FGatersEnvironmentTargetProfile CompileProfile(
	const FGatersEnvironmentTargetRanges& Profile,
	const int32 Seed,
	const uint32 Salt)
{
	FGatersEnvironmentTargetProfile Result;
	Result.Relief = Sample(Profile.Relief, Seed, Salt);
	Result.Temperature = Sample(Profile.Temperature, Seed, Salt + 1u);
	Result.Moisture = Sample(Profile.Moisture, Seed, Salt + 2u);
	Result.SurfaceWater = Sample(Profile.SurfaceWater, Seed, Salt + 3u);
	Result.Volcanism = Sample(Profile.Volcanism, Seed, Salt + 4u);
	Result.Ice = Sample(Profile.Ice, Seed, Salt + 5u);
	Result.Vegetation = Sample(Profile.Vegetation, Seed, Salt + 6u);
	Result.ExposedRock = Sample(Profile.ExposedRock, Seed, Salt + 7u);
	return Result;
}
}

FGatersEnvironmentBrief FGatersEnvironmentBrief::WithGlobalLandformTargets(
	const float Relief,
	const float Volcanism,
	const float Ice) const
{
	FGatersEnvironmentBrief Result = *this;
	if (Relief >= 0.f)
	{
		Result.Global.Relief = {Relief, Relief};
	}
	if (Volcanism >= 0.f)
	{
		Result.Global.Volcanism = {Volcanism, Volcanism};
	}
	if (Ice >= 0.f)
	{
		Result.Global.Ice = {Ice, Ice};
	}
	return Result;
}

FGatersEnvironmentBriefCompileResult FGatersEnvironmentBriefCompiler::Compile(
	const FGatersEnvironmentBrief& Brief,
	const int32 Seed,
	const float WorldSize)
{
	FGatersEnvironmentBriefCompileResult Result;
	Result.Intent.BriefVersion = Brief.Version;
	Result.Intent.Seed = Seed;
	Result.Intent.WorldSize = WorldSize;

	if (Brief.Version != 2)
	{
		AddIssue(Result, TEXT("brief.version"), TEXT("unsupported environment brief version"));
	}
	if (!FMath::IsFinite(WorldSize) || WorldSize <= 0.f)
	{
		AddIssue(Result, TEXT("brief.world_size"), TEXT("world size must be finite and positive"));
	}
	ValidateProfile(Result, Brief.Global, FString());
	ValidateRange(Result, Brief.LandAccess.WalkableLand, TEXT("walkable-land"), FString());
	ValidateRange(Result, Brief.LandAccess.ConnectedLand, TEXT("connected-land"), FString());

	TSet<FString> RegionIds;
	for (const FGatersEnvironmentRegionBrief& Region : Brief.Regions)
	{
		if (Region.Id.IsEmpty() || RegionIds.Contains(Region.Id))
		{
			AddIssue(Result, TEXT("brief.region.id"),
				TEXT("region IDs must be non-empty and unique"), Region.Id);
		}
		RegionIds.Add(Region.Id);
		ValidateProfile(Result, Region.Profile, Region.Id);

		const bool bFiniteCenter = FMath::IsFinite(Region.CenterNormalized.X)
			&& FMath::IsFinite(Region.CenterNormalized.Y);
		if (!bFiniteCenter || !FMath::IsFinite(Region.RadiusFraction)
			|| Region.RadiusFraction <= 0.f || Region.RadiusFraction > 0.5f)
		{
			AddIssue(Result, TEXT("brief.region.geometry"),
				TEXT("region center must be finite and 0 < radius <= 0.5"), Region.Id);
			continue;
		}
		const float RadiusNormalized = Region.RadiusFraction * 2.f;
		if (FMath::Abs(Region.CenterNormalized.X) + RadiusNormalized > 1.f
			|| FMath::Abs(Region.CenterNormalized.Y) + RadiusNormalized > 1.f)
		{
			AddIssue(Result, TEXT("brief.region.bounds"),
				TEXT("region must remain inside normalized world bounds"), Region.Id);
		}
	}

	if (!Result.IsValid())
	{
		return Result;
	}

	Result.Intent.Global = CompileProfile(Brief.Global, Seed, 0u);
	Result.Intent.LandAccess.WalkableLand = Sample(
		Brief.LandAccess.WalkableLand, Seed, 8u);
	Result.Intent.LandAccess.ConnectedLand = Sample(
		Brief.LandAccess.ConnectedLand, Seed, 9u);
	const float HalfWorld = WorldSize * 0.5f;
	for (int32 Index = 0; Index < Brief.Regions.Num(); ++Index)
	{
		const FGatersEnvironmentRegionBrief& Region = Brief.Regions[Index];
		FGatersCompiledEnvironmentRegion& Compiled =
			Result.Intent.Regions.AddDefaulted_GetRef();
		Compiled.Id = Region.Id;
		Compiled.Center = Region.CenterNormalized * HalfWorld;
		Compiled.Radius = Region.RadiusFraction * WorldSize;
		Compiled.Profile = CompileProfile(
			Region.Profile, Seed, 16u + static_cast<uint32>(Index) * 8u);
	}
	return Result;
}
