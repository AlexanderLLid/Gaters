#if WITH_DEV_AUTOMATION_TESTS

#include "GatersEnvironment.h"
#include "GatersChunk.h"
#include "GatersWorldDiff.h"
#include "Misc/AutomationTest.h"

namespace
{
constexpr float TestChunkSize = 30000.f;

const FGatersEnvironment* FindFirstEnvironment(
	EGatersEnvironment Wanted,
	TArray<FGatersEnvironment>& Environments)
{
	for (int32 Seed = 0; Seed < 256; ++Seed)
	{
		FGatersEnvironment Environment = FGatersEnvironment::FromSeed(Seed, TestChunkSize);
		if (Environment.Type == Wanted)
		{
			return &Environments.Add_GetRef(MoveTemp(Environment));
		}
	}
	return nullptr;
}

void MeasureTerrain(
	const FGatersEnvironment& Environment,
	float& OutMin,
	float& OutMax,
	int32& OutAboveWater,
	int32& OutBelowWater)
{
	OutMin = TNumericLimits<float>::Max();
	OutMax = -TNumericLimits<float>::Max();
	OutAboveWater = 0;
	OutBelowWater = 0;
	for (int32 X = -16; X <= 16; ++X)
	{
		for (int32 Y = -16; Y <= 16; ++Y)
		{
			const FVector2D Point(
				X * TestChunkSize / 32.f,
				Y * TestChunkSize / 32.f);
			const float Height = Environment.HeightAt(Point);
			OutMin = FMath::Min(OutMin, Height);
			OutMax = FMath::Max(OutMax, Height);
			if (Height > Environment.WaterHeight)
			{
				++OutAboveWater;
			}
			else
			{
				++OutBelowWater;
			}
		}
	}
}

float MeasureMaxStep(const FGatersEnvironment& Environment)
{
	float MaxStep = 0.f;
	const float Step = TestChunkSize / 32.f;
	for (int32 X = -16; X < 16; ++X)
	{
		for (int32 Y = -16; Y < 16; ++Y)
		{
			const FVector2D Point(X * Step, Y * Step);
			const float Height = Environment.HeightAt(Point);
			MaxStep = FMath::Max(MaxStep,
				FMath::Abs(Height - Environment.HeightAt(Point + FVector2D(Step, 0.f))));
			MaxStep = FMath::Max(MaxStep,
				FMath::Abs(Height - Environment.HeightAt(Point + FVector2D(0.f, Step))));
		}
	}
	return MaxStep;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersEnvironmentDeterminismTest,
	"Gaters.Worldgen.Environment.Determinism",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersEnvironmentDeterminismTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment A = FGatersEnvironment::FromSeed(73, TestChunkSize);
	const FGatersEnvironment B = FGatersEnvironment::FromSeed(73, TestChunkSize);

	TestEqual(TEXT("same seed selects the same family"), A.Type, B.Type);
	TestEqual(TEXT("same seed selects the same name"), A.Name(), B.Name());
	TestEqual(TEXT("same seed selects the same water height"), A.WaterHeight, B.WaterHeight);
	for (const FVector2D Point : {
		FVector2D(-12000.f, -4000.f), FVector2D(0.f, 0.f), FVector2D(8300.f, 11700.f) })
	{
		TestEqual(TEXT("same seed produces the same height"), A.HeightAt(Point), B.HeightAt(Point));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersEnvironmentVarietyTest,
	"Gaters.Worldgen.Environment.Variety",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersEnvironmentVarietyTest::RunTest(const FString& Parameters)
{
	TArray<FGatersEnvironment> Environments;
	Environments.Reserve(4);
	const FGatersEnvironment* Lowlands = FindFirstEnvironment(EGatersEnvironment::Lowlands, Environments);
	const FGatersEnvironment* Mountains = FindFirstEnvironment(EGatersEnvironment::Mountains, Environments);
	const FGatersEnvironment* Canyon = FindFirstEnvironment(EGatersEnvironment::Canyon, Environments);
	const FGatersEnvironment* Archipelago = FindFirstEnvironment(EGatersEnvironment::Archipelago, Environments);

	TestNotNull(TEXT("lowlands family is reachable"), Lowlands);
	TestNotNull(TEXT("mountains family is reachable"), Mountains);
	TestNotNull(TEXT("canyon family is reachable"), Canyon);
	TestNotNull(TEXT("archipelago family is reachable"), Archipelago);
	if (!Lowlands || !Mountains || !Canyon || !Archipelago)
	{
		return false;
	}
	AddInfo(FString::Printf(TEXT("profile seeds: lowlands=%d mountains=%d canyon=%d archipelago=%d"),
		Lowlands->Seed, Mountains->Seed, Canyon->Seed, Archipelago->Seed));

	float LowMin, LowMax, MountainMin, MountainMax, CanyonMin, CanyonMax, IslandMin, IslandMax;
	int32 Above, Below;
	MeasureTerrain(*Lowlands, LowMin, LowMax, Above, Below);
	MeasureTerrain(*Mountains, MountainMin, MountainMax, Above, Below);
	MeasureTerrain(*Canyon, CanyonMin, CanyonMax, Above, Below);
	MeasureTerrain(*Archipelago, IslandMin, IslandMax, Above, Below);
	AddInfo(FString::Printf(TEXT("sampled relief cm: lowlands=%.0f mountains=%.0f canyon=%.0f archipelago=%.0f"),
		LowMax - LowMin, MountainMax - MountainMin, CanyonMax - CanyonMin, IslandMax - IslandMin));

	TestTrue(TEXT("lowlands have player-readable rolling relief"), LowMax - LowMin >= 1400.f);
	TestTrue(TEXT("mountains have far more relief than lowlands"),
		MountainMax - MountainMin > (LowMax - LowMin) * 2.5f);
	TestTrue(TEXT("canyon has major vertical relief"), CanyonMax - CanyonMin > 1800.f);
	TestTrue(TEXT("canyon produces cliff-scale height steps"), MeasureMaxStep(*Canyon) > 500.f);
	TestTrue(TEXT("archipelago includes dry land"), Above > 0);
	TestTrue(TEXT("archipelago includes submerged ground"), Below > 0);
	TestTrue(TEXT("archipelago exposes a water surface"), Archipelago->HasWater());
	TestTrue(TEXT("canyon exposes a river surface"), Canyon->HasWater());
	TestFalse(TEXT("lowlands stay dry"), Lowlands->HasWater());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersEnvironmentBaseSiteTest,
	"Gaters.Worldgen.Environment.BaseSites",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersEnvironmentBaseSiteTest::RunTest(const FString& Parameters)
{
	for (int32 Seed = 0; Seed < 128; ++Seed)
	{
		const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(Seed, TestChunkSize);
		FVector2D Site = FVector2D::ZeroVector;
		const bool bFound = Environment.FindBaseSite(
			TestChunkSize * 0.20f,
			TestChunkSize * 0.36f,
			900.f,
			350.f,
			Site);
		if (!TestTrue(FString::Printf(TEXT("seed %d (%s) has a buildable base site"),
			Seed, *Environment.Name()), bFound))
		{
			continue;
		}
		TestTrue(TEXT("site is outside the Gate clearing"), Site.Size() >= TestChunkSize * 0.20f);
		TestTrue(TEXT("site remains inside the generated chunk"), Site.Size() <= TestChunkSize * 0.36f + 1.f);
		TestTrue(TEXT("site footprint fits the foundation-drop rule"),
			Environment.FootprintDrop(Site, 900.f) <= 350.f);
		TestTrue(TEXT("site is above water"), Environment.HeightAt(Site) > Environment.WaterHeight + 100.f);
		TestTrue(TEXT("site selector guarantees a dry footprint"),
			Environment.IsFootprintDry(Site, 900.f, 50.f));
		for (int32 Sample = 0; Sample < 12; ++Sample)
		{
			const float Angle = 2.f * PI * Sample / 12.f;
			const FVector2D Edge = Site + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * 900.f;
			TestTrue(FString::Printf(TEXT("seed %d keeps its whole base footprint dry"), Seed),
				Environment.HeightAt(Edge) > Environment.WaterHeight + 50.f);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersChunkEnvironmentDefaultsTest,
	"Gaters.Worldgen.Chunk.EnvironmentDefaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersChunkEnvironmentDefaultsTest::RunTest(const FString& Parameters)
{
	const AGatersChunk* Defaults = GetDefault<AGatersChunk>();
	TestEqual(TEXT("generated chunk is large enough for terrain landmarks"), Defaults->ChunkSize, 30000.f);
	TestEqual(TEXT("analysis grid still covers the larger chunk at five-meter resolution"), Defaults->GridN, 61);
	TestEqual(TEXT("terrain mesh has enough subdivisions to show cliffs"), Defaults->TerrainResolution, 128);
	TestTrue(TEXT("terrain rewrite invalidates old seed diffs"), GatersGenVersion >= 2);
	return true;
}

#endif
