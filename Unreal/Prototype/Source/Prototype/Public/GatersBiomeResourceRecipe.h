#pragma once

#include "CoreMinimal.h"
#include "GatersContentCellRecipe.h"

struct FGatersEnvironmentRecipe;

struct PROTOTYPE_API FGatersBiomeResourceSettings
{
	int32 MaxCellCount = 256;
	FGatersContentCellSemantics ContentSemantics;
};

struct PROTOTYPE_API FGatersBiomeResourceCell
{
	FString Id;
	FIntPoint Coordinate = FIntPoint::ZeroValue;
	FString BiomeKey;
	float DeclaredLandmarkOpportunity = 0.f;
	float LandmarkOpportunity = 0.f;
	float DeclaredTravelFriction = 0.f;
	float TravelFriction = 0.f;
	FGatersContentCellRecipe Content;
};

struct PROTOTYPE_API FGatersBiomeResourceIssue
{
	FString RuleId;
	FString SubjectId;
	FString Message;
};

// Pure bounded aggregation of accepted environment evidence and existing content cells.
// It owns no actors, assets, sites, paths, or gameplay-resource identities.
struct PROTOTYPE_API FGatersBiomeResourceRecipe
{
	static constexpr int32 CurrentVersion = 1;
	static constexpr int32 CurrentCompilerVersion = 1;

	bool Validate(TArray<FGatersBiomeResourceIssue>& OutIssues) const;
	FString CanonicalText() const;
	uint32 Checksum() const;

	int32 Version = CurrentVersion;
	int32 CompilerVersion = CurrentCompilerVersion;
	int32 EnvironmentVersion = 0;
	int32 BiomeOpportunityVersion = 0;
	int32 Seed = 0;
	FIntRect CellBounds = FIntRect(0, 0, 0, 0);
	float CellSize = 0.f;
	int32 PlacementBudget = 0;
	FGatersContentCellCoverage Coverage;
	TArray<FGatersBiomeResourceCell> Cells;
};

struct PROTOTYPE_API FGatersBiomeResourceCompileResult
{
	bool IsValid() const { return Issues.IsEmpty(); }

	FGatersBiomeResourceRecipe Recipe;
	TArray<FGatersBiomeResourceIssue> Issues;
};

struct PROTOTYPE_API FGatersBiomeResourceCompiler
{
	static FGatersBiomeResourceCompileResult Compile(
		const FGatersEnvironmentRecipe& Environment,
		const FIntRect& CellBounds,
		float CellSize,
		const FGatersBiomeResourceSettings& Settings = {});
};
