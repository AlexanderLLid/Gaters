#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"

struct FGatersEnvironmentRecipe;
struct FGatersRegionalWaterRecipe;

struct PROTOTYPE_API FGatersDrainageSettings
{
	bool operator==(const FGatersDrainageSettings& Other) const = default;

	int32 CellsPerAxis = 33;
	float Extent = 0.f;
	float ChannelAccumulationThreshold = 6.f;
	float WaterfallDropThreshold = 1000.f;
};

struct PROTOTYPE_API FGatersDrainageCell
{
	bool operator==(const FGatersDrainageCell& Other) const = default;

	int32 Index = INDEX_NONE;
	FIntPoint Coordinate = FIntPoint::ZeroValue;
	FVector2D Center = FVector2D::ZeroVector;
	float Height = 0.f;
	float Precipitation = 0.f;
	float Accumulation = 0.f;
	int32 DownstreamIndex = INDEX_NONE;
	int32 BasinIndex = INDEX_NONE;
	bool bBoundary = false;
	bool bSink = false;
	bool bChannel = false;
};

struct PROTOTYPE_API FGatersDrainageSegment
{
	bool operator==(const FGatersDrainageSegment& Other) const = default;

	FString Id;
	int32 FromIndex = INDEX_NONE;
	int32 ToIndex = INDEX_NONE;
	float Discharge = 0.f;
	float Drop = 0.f;
	bool bWaterfallCandidate = false;
};

struct PROTOTYPE_API FGatersDrainageRecipe
{
	bool operator==(const FGatersDrainageRecipe& Other) const = default;

	static constexpr int32 CurrentVersion = 1;

	int32 Version = CurrentVersion;
	int32 Seed = 0;
	float WorldSize = 0.f;
	int32 CellsPerAxis = 0;
	float Extent = 0.f;
	float CellSize = 0.f;
	float ChannelAccumulationThreshold = 0.f;
	float WaterfallDropThreshold = 0.f;
	int32 BasinCount = 0;
	TArray<FGatersDrainageCell> Cells;
	TArray<FGatersDrainageSegment> Segments;
};

struct PROTOTYPE_API FGatersDrainageIssue
{
	bool operator==(const FGatersDrainageIssue& Other) const = default;

	FString RuleId;
	FString Message;
};

struct PROTOTYPE_API FGatersDrainageBuildResult
{
	bool IsValid() const { return Issues.IsEmpty(); }

	FGatersDrainageRecipe Recipe;
	TArray<FGatersDrainageIssue> Issues;
};

struct PROTOTYPE_API FGatersDrainageWaterSurfaceFit
{
	bool operator==(const FGatersDrainageWaterSurfaceFit& Other) const = default;

	FString SurfaceId;
	TArray<int32> CellIndices;
	TArray<int32> BasinIndices;
	TArray<int32> TerminalCellIndices;
	TArray<FString> ChannelSegmentIds;
	int32 IncomingFlowCount = 0;
	int32 OutgoingFlowCount = 0;
	float MaximumAccumulation = 0.f;
	bool bHasSubmergedTerrain = false;
};

struct PROTOTYPE_API FGatersDrainageWaterFitResult
{
	bool operator==(const FGatersDrainageWaterFitResult& Other) const = default;
	bool IsValid() const { return Issues.IsEmpty(); }

	static constexpr int32 CurrentVersion = 1;

	int32 Version = CurrentVersion;
	int32 DrainageVersion = 0;
	int32 RegionalWaterVersion = 0;
	float DatumTolerance = 0.f;
	TArray<FGatersDrainageWaterSurfaceFit> Surfaces;
	TArray<FGatersDrainageIssue> Issues;
};

enum class EGatersDrainageFeatureKind : uint8
{
	RiverSystem,
	Lake,
	Wetland,
	Delta,
	Waterfall
};

struct PROTOTYPE_API FGatersDrainageFeatureSettings
{
	bool operator==(const FGatersDrainageFeatureSettings& Other) const = default;

	float WetlandMinimumPrecipitation = 0.65f;
	float WetlandMinimumAccumulation = 1.f;
	float WetlandMaximumDrop = 100.f;
};

struct PROTOTYPE_API FGatersDrainageFeatureCandidate
{
	bool operator==(const FGatersDrainageFeatureCandidate& Other) const = default;

	FString Id;
	EGatersDrainageFeatureKind Kind = EGatersDrainageFeatureKind::RiverSystem;
	FString SurfaceId;
	TArray<int32> BasinIndices;
	TArray<int32> CellIndices;
	TArray<FString> SegmentIds;
};

struct PROTOTYPE_API FGatersDrainageFeatureRecipe
{
	bool operator==(const FGatersDrainageFeatureRecipe& Other) const = default;

	static constexpr int32 CurrentVersion = 1;

	int32 Version = CurrentVersion;
	int32 Seed = 0;
	int32 DrainageVersion = 0;
	int32 RegionalWaterVersion = 0;
	int32 WaterFitVersion = 0;
	FGatersDrainageFeatureSettings Settings;
	TArray<FGatersDrainageFeatureCandidate> Candidates;
};

struct PROTOTYPE_API FGatersDrainageFeatureCompileResult
{
	bool operator==(const FGatersDrainageFeatureCompileResult& Other) const = default;
	bool IsValid() const { return Issues.IsEmpty(); }

	FGatersDrainageFeatureRecipe Recipe;
	TArray<FGatersDrainageIssue> Issues;
};

// Pure drainage evidence. It owns no terrain carving, water surfaces, Actors, assets,
// biomes, sites, or encounters.
struct PROTOTYPE_API FGatersDrainageNetwork
{
	static FGatersDrainageBuildResult Build(
		const FGatersEnvironmentRecipe& Environment,
		const FGatersDrainageSettings& Settings = {});

	static FGatersDrainageBuildResult Build(
		int32 Seed,
		float WorldSize,
		const FGatersDrainageSettings& Settings,
		TFunctionRef<float(const FVector2D&)> HeightAt,
		TFunctionRef<float(const FVector2D&)> PrecipitationAt);

	static FGatersDrainageWaterFitResult FitRegionalWater(
		const FGatersDrainageRecipe& Drainage,
		const FGatersRegionalWaterRecipe& RegionalWater,
		float DatumTolerance = 1.f);

	static FGatersDrainageFeatureCompileResult CompileFeatureCandidates(
		const FGatersDrainageRecipe& Drainage,
		const FGatersRegionalWaterRecipe& RegionalWater,
		const FGatersDrainageWaterFitResult& WaterFit,
		const FGatersDrainageFeatureSettings& Settings = {});
};
