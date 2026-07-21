#pragma once

#include "CoreMinimal.h"
#include "GatersSettlementGenerator.h"

enum class EGatersBuildingModuleKind : uint8
{
	Foundation,
	Floor,
	Wall,
	DoorWall,
	Roof
};

struct PROTOTYPE_API FGatersBuildingModule
{
	FString Id;
	EGatersBuildingModuleKind Kind = EGatersBuildingModuleKind::Wall;
	FString ContentKey;
	int32 FloorIndex = 0;
	FTransform Transform = FTransform::Identity;
};

struct PROTOTYPE_API FGatersBuildingUsableSpace
{
	FString Id;
	int32 FloorIndex = 0;
	FVector Center = FVector::ZeroVector;
	FVector Extent = FVector::ZeroVector;
	TArray<FString> SourceIds;
};

struct PROTOTYPE_API FGatersBuildingOpening
{
	FString Id;
	FString SourceModuleId;
	int32 FloorIndex = 0;
	FTransform Transform = FTransform::Identity;
	float Width = 0.f;
	float Headroom = 0.f;
	float Depth = 0.f;
	TArray<FString> SourceIds;
};

struct PROTOTYPE_API FGatersBuildingAssembly
{
	FString CanonicalText() const;
	int32 Count(EGatersBuildingModuleKind Kind) const;

	int32 GeneratorVersion = 2;
	bool bGenerated = false;
	FString BuildingId;
	FIntPoint GroundCell = FIntPoint::ZeroValue;
	int32 FootprintWidthCells = 1;
	int32 FootprintDepthCells = 1;
	int32 FloorCount = 1;
	TArray<FGatersBuildingModule> Modules;
	TArray<FGatersBuildingUsableSpace> UsableSpaces;
	TArray<FGatersBuildingOpening> Openings;
	TArray<FString> Diagnostics;
};

struct PROTOTYPE_API FGatersBuildingGenerator
{
	static FGatersBuildingAssembly Generate(
		const FGatersTerrainSemanticField& Field,
		const FGatersSettlementBuilding& Building);
};
