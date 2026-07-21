#pragma once

#include "CoreMinimal.h"

struct PROTOTYPE_API FGatersEnvironmentSignalRange
{
	bool operator==(const FGatersEnvironmentSignalRange& Other) const = default;

	float Min = 0.f;
	float Max = 1.f;
};

struct PROTOTYPE_API FGatersEnvironmentTargetRanges
{
	bool operator==(const FGatersEnvironmentTargetRanges& Other) const = default;

	FGatersEnvironmentSignalRange Relief;
	FGatersEnvironmentSignalRange Temperature;
	FGatersEnvironmentSignalRange Moisture;
	FGatersEnvironmentSignalRange SurfaceWater;
	FGatersEnvironmentSignalRange Volcanism;
	FGatersEnvironmentSignalRange Ice;
	FGatersEnvironmentSignalRange Vegetation;
	FGatersEnvironmentSignalRange ExposedRock;
};

struct PROTOTYPE_API FGatersLandAccessTargetRanges
{
	bool operator==(const FGatersLandAccessTargetRanges& Other) const = default;

	FGatersEnvironmentSignalRange WalkableLand;
	FGatersEnvironmentSignalRange ConnectedLand;
};

struct PROTOTYPE_API FGatersEnvironmentRegionBrief
{
	bool operator==(const FGatersEnvironmentRegionBrief& Other) const = default;

	FString Id;
	FVector2D CenterNormalized = FVector2D::ZeroVector;
	float RadiusFraction = 0.1f;
	FGatersEnvironmentTargetRanges Profile;
};

// Pure request contract. It describes physical targets without selecting sites,
// encounters, species, assets, or named environment generators.
struct PROTOTYPE_API FGatersEnvironmentBrief
{
	bool operator==(const FGatersEnvironmentBrief& Other) const = default;
	FGatersEnvironmentBrief WithGlobalLandformTargets(
		float Relief,
		float Volcanism,
		float Ice) const;

	int32 Version = 2;
	FGatersEnvironmentTargetRanges Global;
	FGatersLandAccessTargetRanges LandAccess;
	TArray<FGatersEnvironmentRegionBrief> Regions;
};

struct PROTOTYPE_API FGatersEnvironmentTargetProfile
{
	bool operator==(const FGatersEnvironmentTargetProfile& Other) const = default;

	float Relief = 0.f;
	float Temperature = 0.f;
	float Moisture = 0.f;
	float SurfaceWater = 0.f;
	float Volcanism = 0.f;
	float Ice = 0.f;
	float Vegetation = 0.f;
	float ExposedRock = 0.f;
};

struct PROTOTYPE_API FGatersLandAccessTargetProfile
{
	bool operator==(const FGatersLandAccessTargetProfile& Other) const = default;

	float WalkableLand = 0.f;
	float ConnectedLand = 0.f;
};

struct PROTOTYPE_API FGatersCompiledEnvironmentRegion
{
	bool operator==(const FGatersCompiledEnvironmentRegion& Other) const = default;

	FString Id;
	FVector2D Center = FVector2D::ZeroVector;
	float Radius = 0.f;
	FGatersEnvironmentTargetProfile Profile;
};

struct PROTOTYPE_API FGatersCompiledEnvironmentBrief
{
	bool operator==(const FGatersCompiledEnvironmentBrief& Other) const = default;

	int32 CompilerVersion = 2;
	int32 BriefVersion = 0;
	int32 Seed = 0;
	float WorldSize = 0.f;
	FGatersEnvironmentTargetProfile Global;
	FGatersLandAccessTargetProfile LandAccess;
	TArray<FGatersCompiledEnvironmentRegion> Regions;
};

struct PROTOTYPE_API FGatersEnvironmentBriefIssue
{
	FString RuleId;
	FString RegionId;
	FString Message;
};

struct PROTOTYPE_API FGatersEnvironmentBriefCompileResult
{
	bool IsValid() const { return Issues.IsEmpty(); }

	FGatersCompiledEnvironmentBrief Intent;
	TArray<FGatersEnvironmentBriefIssue> Issues;
};

struct PROTOTYPE_API FGatersEnvironmentBriefCompiler
{
	static FGatersEnvironmentBriefCompileResult Compile(
		const FGatersEnvironmentBrief& Brief,
		int32 Seed,
		float WorldSize);
};
