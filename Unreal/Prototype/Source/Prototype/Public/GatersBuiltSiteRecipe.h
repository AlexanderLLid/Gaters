#pragma once

#include "CoreMinimal.h"

enum class EGatersBuiltSiteKind : uint8
{
	Settlement,
	Outpost,
	Base,
	Fortress,
	Dungeon
};

struct PROTOTYPE_API FGatersBuiltSiteSpace
{
	FString Id;
	FVector Center = FVector::ZeroVector;
	// Zero extent means the source proves a point or centerline, not a usable volume.
	FVector Extent = FVector::ZeroVector;
	// Site-generation-owned authored use, separate from physical and tactical tags.
	FString SemanticRole;
	TArray<FString> Tags;
	TArray<FString> SourceIds;
};

struct PROTOTYPE_API FGatersBuiltSiteConnection
{
	FString Id;
	FString FromSpaceId;
	FString ToSpaceId;
	// Zero means topology is known but this clearance fact is not asserted.
	float Width = 0.f;
	float Headroom = 0.f;
	float MaxStepHeight = 0.f;
	float MaxJumpDistance = 0.f;
	// Each connection declares its supported traversal independently.
	TArray<FString> MovementModeIds;
	TArray<FString> BlockerIds;
	TArray<FString> Tags;
	TArray<FString> SourceIds;
};

struct PROTOTYPE_API FGatersBuiltSiteEvidenceCoverage
{
	bool bPlacement = false;
	bool bTraversalClearance = false;
	bool bVisibility = false;
	bool bBlockers = false;
	TArray<FString> SourceIds;
};

struct PROTOTYPE_API FGatersBuiltSiteVisibility
{
	FString Id;
	FString FromSpaceId;
	FString ToSpaceId;
	float Distance = 0.f;
	float FromHeight = 0.f;
	float ToHeight = 0.f;
	TArray<FString> BlockerIds;
	TArray<FString> Tags;
	TArray<FString> SourceIds;
};

struct PROTOTYPE_API FGatersBuiltSiteBlocker
{
	FString Id;
	FVector Center = FVector::ZeroVector;
	FVector Extent = FVector::ZeroVector;
	TArray<FString> Tags;
	TArray<FString> SourceIds;
};

struct PROTOTYPE_API FGatersBuiltSitePlacementSlot
{
	FString Id;
	FString SpaceId;
	FVector Location = FVector::ZeroVector;
	float ClearanceRadius = 0.f;
	float ClearanceHeight = 0.f;
	TArray<FString> Tags;
	TArray<FString> SourceIds;
};

struct PROTOTYPE_API FGatersBuiltSiteIssue
{
	FString RuleId;
	FString SubjectId;
	FString Message;
};

// Pure physical facts. Runtime ownership and scenario roles are separate instance data.
struct PROTOTYPE_API FGatersBuiltSiteRecipe
{
	const FGatersBuiltSiteSpace* FindSpace(const FString& Id) const;
	FString CanonicalText() const;
	uint32 Checksum() const;
	bool Validate(TArray<FGatersBuiltSiteIssue>& OutIssues) const;

	int32 ContractVersion = 1;
	int32 SiteVersion = 1;
	int32 GeneratorVersion = 0;
	int32 Seed = 0;
	FString SiteId;
	EGatersBuiltSiteKind Kind = EGatersBuiltSiteKind::Settlement;
	float SiteArea = 0.f;
	FGatersBuiltSiteEvidenceCoverage EvidenceCoverage;
	TArray<FGatersBuiltSiteSpace> Spaces;
	TArray<FGatersBuiltSiteConnection> Connections;
	TArray<FGatersBuiltSiteVisibility> Visibility;
	TArray<FGatersBuiltSiteBlocker> Blockers;
	TArray<FGatersBuiltSitePlacementSlot> PlacementSlots;
};
