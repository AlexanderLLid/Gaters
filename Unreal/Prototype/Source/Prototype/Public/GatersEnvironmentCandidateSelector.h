#pragma once

#include "CoreMinimal.h"
#include "GatersEnvironment.h"
#include "GatersLandformProcessField.h"

struct PROTOTYPE_API FGatersEnvironmentCandidateSelectionSettings
{
	bool operator==(const FGatersEnvironmentCandidateSelectionSettings& Other) const = default;

	int32 CandidateCount = 8;
	int32 WorldCellsPerAxis = 61;
	int32 ArrivalCellsPerAxis = 61;
	float ArrivalCellSize = 500.f;
	float PadRadius = 1000.f;
	float FlatNormalZ = 0.94f;
	float SlopeNormalZ = 0.77f;
	int32 EscapeDistanceCells = 3;
	float WalkableTolerance = 0.15f;
	float ConnectedTolerance = 0.15f;
};

struct PROTOTYPE_API FGatersEnvironmentCandidateEvidence
{
	bool operator==(const FGatersEnvironmentCandidateEvidence& Other) const = default;

	int32 CandidateIndex = 0;
	float WalkableLand = 0.f;
	float ConnectedLand = 0.f;
	bool bHasWorldAccess = false;
	bool bEscapesArrival = false;
	float WalkableError = 0.f;
	float ConnectedError = 0.f;
	float Score = 0.f;
};

struct PROTOTYPE_API FGatersEnvironmentCandidateSelectionIssue
{
	bool operator==(const FGatersEnvironmentCandidateSelectionIssue& Other) const = default;

	FString RuleId;
	FString Message;
};

struct PROTOTYPE_API FGatersEnvironmentCandidateSelectionResult
{
	bool operator==(const FGatersEnvironmentCandidateSelectionResult& Other) const = default;

	int32 Version = 4;
	bool bSelected = false;
	FGatersLandformProcessRecipe SelectedRecipe;
	FGatersLandformProcessRecipe BestRecipe;
	FGatersEnvironmentCandidateEvidence Selected;
	FGatersEnvironmentCandidateEvidence Best;
	TArray<FGatersEnvironmentCandidateEvidence> Candidates;
	TArray<FGatersEnvironmentCandidateSelectionIssue> Issues;
};

// Pure bounded search. It compares independent landform candidates against the
// compiled land-access intent and never depends on optional sites or encounters.
struct PROTOTYPE_API FGatersEnvironmentCandidateSelector
{
	static FGatersEnvironmentCandidateSelectionResult Select(
		const FGatersEnvironment& Environment,
		const FGatersCompiledEnvironmentBrief& Intent,
		const TArray<FGatersLandformProtectedRegion>& ProtectedRegions,
		const FGatersEnvironmentCandidateSelectionSettings& Settings = {});
};
