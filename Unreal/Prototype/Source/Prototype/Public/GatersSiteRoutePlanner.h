#pragma once

#include "CoreMinimal.h"
#include "GatersTerrainNavigation.h"

enum class EGatersPlannedSiteKind : uint8
{
	Arrival,
	Village,
	RaidBase,
	Landmark
};

struct PROTOTYPE_API FGatersPlannedSite
{
	FString Id;
	EGatersPlannedSiteKind Kind = EGatersPlannedSiteKind::Arrival;
	FIntPoint Cell = FIntPoint::ZeroValue;
	FVector Location = FVector::ZeroVector;

	bool operator==(const FGatersPlannedSite& Other) const
	{
		return Id == Other.Id && Kind == Other.Kind && Cell == Other.Cell
			&& Location == Other.Location;
	}
};

struct PROTOTYPE_API FGatersPlannedRoute
{
	FString Id;
	FString FromSiteId;
	FString ToSiteId;
	FGatersTerrainPath Path;

	bool operator==(const FGatersPlannedRoute& Other) const
	{
		return Id == Other.Id && FromSiteId == Other.FromSiteId
			&& ToSiteId == Other.ToSiteId && Path.bFound == Other.Path.bFound
			&& Path.Cost == Other.Path.Cost && Path.Cells == Other.Path.Cells;
	}
};

struct PROTOTYPE_API FGatersSiteRoutePlan
{
	const FGatersPlannedSite* FindSite(const FString& Id) const;

	int32 PlannerVersion = 1;
	bool bValid = false;
	TArray<FGatersPlannedSite> Sites;
	TArray<FGatersPlannedRoute> Routes;
	TArray<FString> Diagnostics;
};

struct PROTOTYPE_API FGatersSiteRoutePlanner
{
	static FGatersSiteRoutePlan Plan(
		const FGatersTerrainSemanticField& Field,
		int32 Seed,
		const FVector2D& RaidBaseLocation);
};
