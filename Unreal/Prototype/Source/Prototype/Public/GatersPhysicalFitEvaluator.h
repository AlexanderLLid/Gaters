#pragma once

#include "CoreMinimal.h"
#include "GatersAssetContract.h"

struct FGatersEnvironment;
struct FGatersCompiledWorld;

struct PROTOTYPE_API FGatersPhysicalFitSettings
{
	float ContactToleranceCm = 5.f;
	float MinContactNormalDot = 0.7f;
};

struct PROTOTYPE_API FGatersPhysicalFitContactSample
{
	FString ContactName;
	FVector WorldLocation = FVector::ZeroVector;
	float TerrainHeight = 0.f;
	FVector TerrainNormal = FVector::UpVector;
};

struct PROTOTYPE_API FGatersPhysicalFitObstacle
{
	FString Id;
	FBox Bounds = FBox(ForceInit);
};

struct PROTOTYPE_API FGatersPhysicalFitCandidate
{
	FString RecipeId;
	FGatersAssetContract Contract;
	FTransform Transform = FTransform::Identity;
	TArray<FGatersPhysicalFitContactSample> ContactSamples;
	TArray<FGatersPhysicalFitObstacle> Obstacles;
};

struct PROTOTYPE_API FGatersPhysicalFitIssue
{
	FString RuleId;
	FString RecipeId;
	FString AssetId;
	FString SubjectId;
	FString ObstacleId;
	FString Message;
	double Measured = 0.0;
	double Limit = 0.0;
};

struct PROTOTYPE_API FGatersPhysicalFitEvaluation
{
	bool IsValid() const { return Issues.IsEmpty(); }

	int32 EvaluatedTerrainContacts = 0;
	int32 PendingAttachmentContacts = 0;
	TArray<FGatersPhysicalFitIssue> Issues;
};

// Pure placement policy. Terrain queries and collision gathering happen upstream;
// this class classifies immutable evidence and never repairs or materializes a placement.
struct PROTOTYPE_API FGatersPhysicalFitEvaluator
{
	static FGatersPhysicalFitCandidate SampleTerrain(
		const FString& RecipeId,
		const FGatersAssetContract& Contract,
		const FTransform& Transform,
		const FGatersEnvironment& Environment,
		float PadRadius,
		const FVector2D& RouteTarget = FVector2D::ZeroVector,
		float NormalSampleDistance = 50.f);

	static FGatersPhysicalFitEvaluation Evaluate(
		const FGatersPhysicalFitCandidate& Candidate,
		const FGatersPhysicalFitSettings& Settings = FGatersPhysicalFitSettings());

	static TArray<FGatersPhysicalFitEvaluation> EvaluateWorld(
		const FGatersCompiledWorld& World,
		const FGatersEnvironment& Environment,
		float PadRadius,
		const FVector2D& RouteTarget = FVector2D::ZeroVector,
		const FGatersPhysicalFitSettings& Settings = FGatersPhysicalFitSettings());
};
