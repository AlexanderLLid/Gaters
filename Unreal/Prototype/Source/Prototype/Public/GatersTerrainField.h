#pragma once

#include "CoreMinimal.h"
#include "DynamicMesh/DynamicMesh3.h"

struct FGatersTerrainEdit
{
	FVector Center = FVector::ZeroVector;
	float Radius = 0.f;
};

enum class EGatersTerrainContactStatus : uint8
{
	Accepted,
	OutsideField,
	TooUneven
};

// A generator-time request to seat one rough object's circular contact patch into the
// terrain. BlendRadius is the transition apron outside the object's actual footprint.
struct FGatersTerrainContact
{
	FVector2D Center = FVector2D::ZeroVector;
	float Radius = 0.f;
	float BlendRadius = 0.f;
	float MaxTerrainChange = 0.f;
};

struct FGatersTerrainContactResult
{
	EGatersTerrainContactStatus Status = EGatersTerrainContactStatus::OutsideField;
	FVector Placement = FVector::ZeroVector;
	float RequiredTerrainChange = 0.f;
};

// Pure terrain-lab data: positive density is solid. The rendered triangles are rebuilt
// output; stable identity belongs to grid cells and the ordered edits applied to them.
class PROTOTYPE_API FGatersTerrainField
{
public:
	FGatersTerrainField(int32 InCellsPerAxis, float InCellSize);

	void Generate(int32 Seed);
	void Apply(const FGatersTerrainEdit& Edit);
	bool TryFitContact(
		const FGatersTerrainContact& Contact,
		FGatersTerrainContactResult& OutResult);
	void BuildMesh(UE::Geometry::FDynamicMesh3& OutMesh) const;

	int64 CellId(int32 X, int32 Y, int32 Z) const;
	int32 SolidSampleCount() const;
	uint64 DensityChecksum() const;
	float FootprintDrop(const FVector2D& Center, float Radius) const;

private:
	int32 CellsPerAxis;
	int32 SamplesPerAxis;
	float CellSize;
	TArray<float> Density;

	int32 SampleIndex(int32 X, int32 Y, int32 Z) const;
	FVector3d SamplePosition(int32 X, int32 Y, int32 Z) const;
	bool SurfaceHeightAt(const FVector2D& Point, float& OutHeight) const;
};
