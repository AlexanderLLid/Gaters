#pragma once

#include "CoreMinimal.h"

enum class EGatersEnvironment : uint8
{
	Lowlands,
	Mountains,
	Canyon,
	Archipelago
};

// Pure seed-derived terrain recipe. It owns no actors or assets, so seed sweeps and
// placement validation can run without loading a world.
struct PROTOTYPE_API FGatersEnvironment
{
	static FGatersEnvironment FromSeed(int32 Seed, float ChunkSize);

	float HeightAt(const FVector2D& Point) const;
	float FootprintDrop(const FVector2D& Center, float Radius) const;
	bool IsFootprintDry(const FVector2D& Center, float Radius, float Clearance) const;
	bool FindBaseSite(
		float MinDistance,
		float MaxDistance,
		float FootprintRadius,
		float MaxDrop,
		FVector2D& OutSite) const;
	bool HasWater() const;
	FString Name() const;

	int32 Seed = 0;
	float ChunkSize = 0.f;
	EGatersEnvironment Type = EGatersEnvironment::Lowlands;
	float WaterHeight = -100000.f;

private:
	FVector2D NoiseOffset = FVector2D::ZeroVector;
	float RotationRadians = 0.f;
	float Phase = 0.f;

	FVector2D Rotate(const FVector2D& Point) const;
	float Fractal(const FVector2D& Point, float Frequency, int32 Octaves) const;
};
