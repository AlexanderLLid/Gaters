#if WITH_DEV_AUTOMATION_TESTS

#include "GatersTerrainPalette.h"
#include "Materials/MaterialInterface.h"
#include "Misc/AutomationTest.h"

namespace
{
float ColorDistanceSquared(const FLinearColor& A, const FLinearColor& B)
{
	const FLinearColor Delta = A - B;
	return Delta.R * Delta.R + Delta.G * Delta.G + Delta.B * Delta.B;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersTerrainPaletteTest,
	"Gaters.Worldgen.Materials.TerrainPalette",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersTerrainPaletteTest::RunTest(const FString& Parameters)
{
	const TStaticArray<FLinearColor, 3> Canyon =
		FGatersTerrainPalette::Colors(EGatersEnvironment::Canyon);
	TestNotEqual(TEXT("ground and rock palette slots remain visually distinct"), Canyon[1], Canyon[2]);
	for (const EGatersEnvironment Type : {
		EGatersEnvironment::Lowlands,
		EGatersEnvironment::Mountains,
		EGatersEnvironment::Canyon,
		EGatersEnvironment::Archipelago })
	{
		for (const FLinearColor& Color : FGatersTerrainPalette::Colors(Type))
		{
			TestTrue(TEXT("palette remains exposure-safe for gallery lighting"),
				FMath::Max3(Color.R, Color.G, Color.B) <= 0.18f);
		}
	}
	TestNotEqual(TEXT("ocean and lake water retain distinct absorption"),
		FGatersTerrainPalette::WaterAbsorption(EGatersHydrology::Ocean),
		FGatersTerrainPalette::WaterAbsorption(EGatersHydrology::Lakes));
	TestTrue(TEXT("lake water absorbs red more strongly than blue"),
		FGatersTerrainPalette::WaterAbsorption(EGatersHydrology::Lakes).R >
		FGatersTerrainPalette::WaterAbsorption(EGatersHydrology::Lakes).B);

	const FLinearColor SlopeBelow = FGatersTerrainPalette::BlendColor(
		EGatersEnvironment::Lowlands, -100000.f, 300.f, 0.77f);
	const FLinearColor SlopeAbove = FGatersTerrainPalette::BlendColor(
		EGatersEnvironment::Lowlands, -100000.f, 300.f, 0.79f);
	TestTrue(TEXT("slope transition is continuous instead of changing per triangle"),
		ColorDistanceSquared(SlopeBelow, SlopeAbove) < 0.001f);

	const FLinearColor ShoreBelow = FGatersTerrainPalette::BlendColor(
		EGatersEnvironment::Archipelago, 0.f, 290.f, 0.96f);
	const FLinearColor ShoreAbove = FGatersTerrainPalette::BlendColor(
		EGatersEnvironment::Archipelago, 0.f, 310.f, 0.96f);
	TestTrue(TEXT("shore transition is continuous instead of forming a contour"),
		ColorDistanceSquared(ShoreBelow, ShoreAbove) < 0.001f);

	TestEqual(TEXT("flat ground converges on the ground palette"),
		FGatersTerrainPalette::BlendColor(
			EGatersEnvironment::Lowlands, -100000.f, 500.f, 1.f),
		FGatersTerrainPalette::Colors(EGatersEnvironment::Lowlands)[1]);
	TestEqual(TEXT("very steep ground converges on the rock palette"),
		FGatersTerrainPalette::BlendColor(
			EGatersEnvironment::Lowlands, -100000.f, 500.f, 0.4f),
		FGatersTerrainPalette::Colors(EGatersEnvironment::Lowlands)[2]);
	TestNotNull(TEXT("terrain uses a packaged project vertex-color material"),
		LoadObject<UMaterialInterface>(nullptr,
			TEXT("/Game/Gaters/Materials/M_TerrainVertexColor.M_TerrainVertexColor")));
	return true;
}

#endif
