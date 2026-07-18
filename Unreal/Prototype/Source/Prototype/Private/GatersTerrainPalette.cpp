#include "GatersTerrainPalette.h"

namespace
{
FLinearColor Srgb(uint8 R, uint8 G, uint8 B)
{
	return FLinearColor::FromSRGBColor(FColor(R, G, B));
}
}

FLinearColor FGatersTerrainPalette::BlendColor(
	EGatersEnvironment Type,
	float WaterHeight,
	float Height,
	float NormalZ)
{
	const TStaticArray<FLinearColor, 3> Palette = Colors(Type);
	float LowWeight = 0.f;
	float HighRockWeight = 0.f;

	switch (Type)
	{
	case EGatersEnvironment::Mountains:
		if (WaterHeight > -10000.f)
		{
			LowWeight = 1.f - FMath::SmoothStep(
				WaterHeight + 50.f, WaterHeight + 550.f, Height);
		}
		HighRockWeight = FMath::SmoothStep(2800.f, 3600.f, Height);
		break;
	case EGatersEnvironment::Archipelago:
		LowWeight = 1.f - FMath::SmoothStep(
			WaterHeight + 50.f, WaterHeight + 550.f, Height);
		break;
	case EGatersEnvironment::Canyon:
		LowWeight = 1.f - FMath::SmoothStep(
			WaterHeight + 250.f, WaterHeight + 750.f, Height);
		break;
	case EGatersEnvironment::Lowlands:
	default:
		LowWeight = 1.f - FMath::SmoothStep(-500.f, 0.f, Height);
		break;
	}

	const float SlopeRockWeight = 1.f - FMath::SmoothStep(0.62f, 0.86f, NormalZ);
	const float RockWeight = FMath::Max(SlopeRockWeight, HighRockWeight);
	const FLinearColor Ground = FMath::Lerp(Palette[1], Palette[0], LowWeight);
	return FMath::Lerp(Ground, Palette[2], RockWeight);
}

TStaticArray<FLinearColor, 3> FGatersTerrainPalette::Colors(EGatersEnvironment Type)
{
	switch (Type)
	{
	case EGatersEnvironment::Mountains:
		return { Srgb(75, 88, 62), Srgb(94, 88, 70), Srgb(75, 78, 80) };
	case EGatersEnvironment::Canyon:
		return { Srgb(82, 48, 34), Srgb(112, 65, 38), Srgb(72, 48, 44) };
	case EGatersEnvironment::Archipelago:
		return { Srgb(116, 102, 68), Srgb(56, 92, 48), Srgb(75, 82, 76) };
	case EGatersEnvironment::Lowlands:
	default:
		return { Srgb(62, 78, 45), Srgb(72, 105, 52), Srgb(76, 80, 74) };
	}
}

FLinearColor FGatersTerrainPalette::WaterAbsorption(EGatersHydrology Hydrology)
{
	return Hydrology == EGatersHydrology::Ocean
		? FLinearColor(0.040f, 0.012f, 0.004f)
		: FLinearColor(0.025f, 0.008f, 0.003f);
}

FLinearColor FGatersTerrainPalette::WaterScattering(EGatersHydrology Hydrology)
{
	return Hydrology == EGatersHydrology::Ocean
		? FLinearColor(0.001f, 0.008f, 0.014f)
		: FLinearColor(0.002f, 0.012f, 0.009f);
}

float FGatersTerrainPalette::WaterPhaseG(EGatersHydrology Hydrology)
{
	return Hydrology == EGatersHydrology::Ocean ? 0.25f : 0.10f;
}
