#pragma once

#include "CoreMinimal.h"
#include "GatersSiteRoutePlanner.h"

enum class EGatersSettlementRole : uint8
{
	Home,
	Community,
	Workshop,
	Storage
};

struct PROTOTYPE_API FGatersSettlementParcel
{
	FString Id;
	int32 IntroducedStage = 0;
	FIntPoint Cell = FIntPoint::ZeroValue;
	FIntPoint EntranceCell = FIntPoint::ZeroValue;
	FVector Location = FVector::ZeroVector;
	FString SupportKey = TEXT("support.ground");
	FString AccessKey = TEXT("navigation.ground");
};

struct PROTOTYPE_API FGatersSettlementGrowthFront
{
	FString Id;
	int32 Sector = 0;

	bool operator==(const FGatersSettlementGrowthFront& Other) const
	{
		return Id == Other.Id && Sector == Other.Sector;
	}
};

struct PROTOTYPE_API FGatersSettlementProfile
{
	FString CanonicalText() const;

	int32 Version = 1;
	float HeightBias = 0.f;
	float FootprintBias = 0.f;
	float DensityBias = 0.f;
};

struct PROTOTYPE_API FGatersSettlementBuilding
{
	FString Id;
	FString ParcelId;
	FString GrowthFrontId;
	int32 IntroducedStage = 0;
	EGatersSettlementRole Role = EGatersSettlementRole::Home;
	FString ContentKey;
	FIntPoint Cell = FIntPoint::ZeroValue;
	FIntPoint EntranceCell = FIntPoint::ZeroValue;
	FVector Location = FVector::ZeroVector;
	float Yaw = 0.f;
	int32 FootprintWidthCells = 1;
	int32 FootprintDepthCells = 1;
	int32 FloorCount = 1;
};

struct PROTOTYPE_API FGatersSettlementPlan
{
	FString CanonicalText() const;
	const FGatersSettlementBuilding* FindBuilding(const FString& Id) const;
	const FGatersSettlementParcel* FindParcel(const FString& Id) const;
	static FString StablePathId(const FIntPoint& Cell);

	int32 GeneratorVersion = 3;
	int32 GrowthStage = 0;
	bool bGenerated = false;
	FString SiteId;
	FIntPoint CenterCell = FIntPoint::ZeroValue;
	FVector CenterLocation = FVector::ZeroVector;
	FGatersSettlementProfile Profile;
	TArray<FGatersSettlementParcel> Parcels;
	TArray<FGatersSettlementGrowthFront> GrowthFronts;
	TArray<FGatersSettlementBuilding> Buildings;
	TArray<FIntPoint> PathCells;
	TArray<FString> Diagnostics;
};

struct PROTOTYPE_API FGatersSettlementGenerator
{
	static bool IsSupportedGrowthStage(int32 GrowthStage);
	static int32 ExpectedBuildingCount(int32 GrowthStage);
	static int32 ExpectedHomeCount(int32 GrowthStage);
	static float MaxRadiusCells(int32 GrowthStage);
	static FGatersSettlementPlan Generate(
		const FGatersTerrainSemanticField& Field,
		int32 Seed,
		const FGatersPlannedSite& VillageSite,
		int32 GrowthStage = 0);
};
