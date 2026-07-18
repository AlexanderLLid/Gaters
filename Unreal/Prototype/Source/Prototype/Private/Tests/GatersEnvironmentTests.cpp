#if WITH_DEV_AUTOMATION_TESTS

#include "GatersEnvironment.h"
#include "GatersChunk.h"
#include "GatersTerrainEvaluator.h"
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

struct FArchipelagoTopology
{
	int32 SignificantIslandCount = 0;
	uint32 CoastlineHash = 2166136261u;
};

FArchipelagoTopology MeasureArchipelagoTopology(const FGatersEnvironment& Environment)
{
	constexpr int32 Side = 41;
	constexpr float HalfExtent = TestChunkSize * 0.5f;
	constexpr int32 MinimumIslandSamples = 5;
	TArray<bool> Dry;
	Dry.SetNumUninitialized(Side * Side);

	FArchipelagoTopology Result;
	for (int32 Y = 0; Y < Side; ++Y)
	{
		for (int32 X = 0; X < Side; ++X)
		{
			const FVector2D Point(
				FMath::Lerp(-HalfExtent, HalfExtent, X / static_cast<float>(Side - 1)),
				FMath::Lerp(-HalfExtent, HalfExtent, Y / static_cast<float>(Side - 1)));
			const int32 Index = Y * Side + X;
			Dry[Index] = Environment.HeightAt(Point) > Environment.WaterHeight + 50.f;
			Result.CoastlineHash = (Result.CoastlineHash ^ (Dry[Index] ? 1u : 0u)) * 16777619u;
		}
	}

	TArray<bool> Visited;
	Visited.Init(false, Dry.Num());
	for (int32 Start = 0; Start < Dry.Num(); ++Start)
	{
		if (!Dry[Start] || Visited[Start])
		{
			continue;
		}

		int32 ComponentSamples = 0;
		TArray<int32> Open;
		Open.Add(Start);
		Visited[Start] = true;
		for (int32 Cursor = 0; Cursor < Open.Num(); ++Cursor)
		{
			const int32 Index = Open[Cursor];
			++ComponentSamples;
			const int32 X = Index % Side;
			const int32 Y = Index / Side;
			const int32 Neighbors[] = {
				X > 0 ? Index - 1 : -1,
				X + 1 < Side ? Index + 1 : -1,
				Y > 0 ? Index - Side : -1,
				Y + 1 < Side ? Index + Side : -1 };
			for (const int32 Neighbor : Neighbors)
			{
				if (Neighbor >= 0 && Dry[Neighbor] && !Visited[Neighbor])
				{
					Visited[Neighbor] = true;
					Open.Add(Neighbor);
				}
			}
		}
		if (ComponentSamples >= MinimumIslandSamples)
		{
			++Result.SignificantIslandCount;
		}
	}
	return Result;
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
	TestEqual(TEXT("same seed selects the same hydrology"), A.Hydrology, B.Hydrology);
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
	FGatersEnvironmentHydrologyTest,
	"Gaters.Worldgen.Environment.Hydrology",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersEnvironmentHydrologyTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment MountainLakes = FGatersEnvironment::FromSeed(53, TestChunkSize);
	TestEqual(TEXT("seed 53 remains mountains"), MountainLakes.Type, EGatersEnvironment::Mountains);
	TestEqual(TEXT("seed 53 demonstrates mountain lakes"),
		MountainLakes.Hydrology, EGatersHydrology::Lakes);
	TestEqual(TEXT("hydrology has a stable evidence label"),
		MountainLakes.HydrologyName(), FString(TEXT("lakes")));
	TestTrue(TEXT("mountain lakes expose a water surface"), MountainLakes.HasWater());
	const TArray<FGatersWaterSurface> LakeSurfaces = MountainLakes.WaterSurfaces();
	TestEqual(TEXT("lake hydrology has two local render footprints"), LakeSurfaces.Num(), 2);
	TestTrue(TEXT("lake render footprints remain local"),
		LakeSurfaces[0].HalfExtent < TestChunkSize * 0.25f);

	bool bFoundLowlandLakes = false;
	bool bFoundDryMountains = false;
	for (int32 Seed = 0; Seed < 256; ++Seed)
	{
		const FGatersEnvironment Candidate = FGatersEnvironment::FromSeed(Seed, TestChunkSize);
		bFoundLowlandLakes |= Candidate.Type == EGatersEnvironment::Lowlands &&
			Candidate.Hydrology == EGatersHydrology::Lakes;
		bFoundDryMountains |= Candidate.Type == EGatersEnvironment::Mountains &&
			Candidate.Hydrology == EGatersHydrology::Dry;
	}
	TestTrue(TEXT("lowlands can independently roll lakes"), bFoundLowlandLakes);
	TestTrue(TEXT("mountains can independently remain dry"), bFoundDryMountains);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersTerrainEvaluatorTest,
	"Gaters.Worldgen.Environment.Evaluator",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersTerrainEvaluatorTest::RunTest(const FString& Parameters)
{
	const FGatersTerrainEvaluation A = FGatersTerrainEvaluator::Evaluate(
		FGatersEnvironment::FromSeed(73, TestChunkSize));
	const FGatersTerrainEvaluation B = FGatersTerrainEvaluator::Evaluate(
		FGatersEnvironment::FromSeed(73, TestChunkSize));

	TestEqual(TEXT("evaluator contract is versioned"), A.EvaluatorVersion, 3);
	TestEqual(TEXT("same terrain has the same minimum"), A.MinHeight, B.MinHeight);
	TestEqual(TEXT("same terrain has the same maximum"), A.MaxHeight, B.MaxHeight);
	TestEqual(TEXT("same terrain has the same water coverage"), A.WaterFraction, B.WaterFraction);
	TestEqual(TEXT("same terrain has the same roughness"), A.MeanNeighborStep, B.MeanNeighborStep);
	TestEqual(TEXT("same terrain has the same cliff step"), A.MaxNeighborStep, B.MaxNeighborStep);
	TestEqual(TEXT("same terrain has the same buildable coverage"), A.BuildableFraction, B.BuildableFraction);
	TestEqual(TEXT("same terrain has the same signed mean elevation"), A.MeanHeight, B.MeanHeight);
	TestEqual(TEXT("same terrain has the same below-datum coverage"),
		A.BelowDatumFraction, B.BelowDatumFraction);
	TestTrue(TEXT("relief is non-negative"), A.Relief() >= 0.f);
	TestTrue(TEXT("roughness is non-negative"), A.MeanNeighborStep >= 0.f);
	TestTrue(TEXT("water coverage is a fraction"), A.WaterFraction >= 0.f && A.WaterFraction <= 1.f);
	TestTrue(TEXT("buildable coverage is a fraction"),
		A.BuildableFraction >= 0.f && A.BuildableFraction <= 1.f);
	TestTrue(TEXT("below-datum coverage is a fraction"),
		A.BelowDatumFraction >= 0.f && A.BelowDatumFraction <= 1.f);

	const FGatersEnvironment RuntimeMountainLakes = FGatersEnvironment::FromSeed(53, 400000.f);
	const FGatersTerrainEvaluation LocalLakeMetrics = FGatersTerrainEvaluator::Evaluate(
		RuntimeMountainLakes, TestChunkSize);
	TestTrue(TEXT("runtime evaluator can inspect the local gameplay window"),
		LocalLakeMetrics.WaterFraction > 0.f);

	TArray<FGatersEnvironment> Environments;
	Environments.Reserve(4);
	const FGatersEnvironment* Lowlands = FindFirstEnvironment(EGatersEnvironment::Lowlands, Environments);
	const FGatersEnvironment* Mountains = FindFirstEnvironment(EGatersEnvironment::Mountains, Environments);
	const FGatersEnvironment* Canyon = FindFirstEnvironment(EGatersEnvironment::Canyon, Environments);
	const FGatersEnvironment* Archipelago = FindFirstEnvironment(EGatersEnvironment::Archipelago, Environments);
	if (!Lowlands || !Mountains || !Canyon || !Archipelago)
	{
		return false;
	}

	const FGatersTerrainEvaluation Low = FGatersTerrainEvaluator::Evaluate(*Lowlands);
	const FGatersTerrainEvaluation Mountain = FGatersTerrainEvaluator::Evaluate(*Mountains);
	const FGatersTerrainEvaluation CanyonMetrics = FGatersTerrainEvaluator::Evaluate(*Canyon);
	const FGatersTerrainEvaluation Islands = FGatersTerrainEvaluator::Evaluate(*Archipelago);
	TestTrue(TEXT("mountains have more sampled relief than lowlands"), Mountain.Relief() > Low.Relief());
	TestTrue(TEXT("archipelago evaluation observes water"), Islands.WaterFraction > 0.f);
	TestTrue(TEXT("lowlands expose buildable coverage"), Low.BuildableFraction > 0.f);
	TestTrue(TEXT("mountains expose buildable coverage"), Mountain.BuildableFraction > 0.f);
	TestTrue(TEXT("canyon exposes buildable coverage"), CanyonMetrics.BuildableFraction > 0.f);
	TestTrue(TEXT("archipelago exposes buildable coverage"), Islands.BuildableFraction > 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersMountainProfileTest,
	"Gaters.Worldgen.Environment.MountainProfile",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersMountainProfileTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(53, TestChunkSize);
	TestEqual(TEXT("seed 53 remains the held-out mountain profile"),
		Environment.Type, EGatersEnvironment::Mountains);
	const FGatersTerrainEvaluation Metrics = FGatersTerrainEvaluator::Evaluate(Environment);
	AddInfo(FString::Printf(TEXT("seed 53 relief=%.0f min=%.0f max-step=%.0f buildable=%.3f water=%.3f"),
		Metrics.Relief(), Metrics.MinHeight, Metrics.MaxNeighborStep,
		Metrics.BuildableFraction, Metrics.WaterFraction));
	TestTrue(TEXT("mountain relief is distributed instead of one crater wall"),
		Metrics.MaxNeighborStep < Metrics.Relief() * 0.75f);
	TestTrue(TEXT("mountain valleys do not become deep negative holes"), Metrics.MinHeight > -500.f);
	TestTrue(TEXT("mountain profile keeps meaningful buildable coverage"),
		Metrics.BuildableFraction >= 0.10f);
	TestTrue(TEXT("mountain lakes expose sampled water rather than metadata only"),
		Metrics.WaterFraction > 0.f);

	const FGatersEnvironment DryRuntimeMountain = FGatersEnvironment::FromSeed(0, 400000.f);
	TestEqual(TEXT("seed 0 remains the held-out dry mountain profile"),
		DryRuntimeMountain.Type, EGatersEnvironment::Mountains);
	const FGatersTerrainEvaluation DryMetrics = FGatersTerrainEvaluator::Evaluate(
		DryRuntimeMountain, TestChunkSize);
	AddInfo(FString::Printf(TEXT("seed 0 relief=%.0f rough=%.0f buildable=%.3f"),
		DryMetrics.Relief(), DryMetrics.MeanNeighborStep, DryMetrics.BuildableFraction));
	TestTrue(TEXT("dry mountains retain macro relief"), DryMetrics.Relief() >= 2500.f);
	TestTrue(TEXT("dry mountains avoid repeated high-frequency ridges"),
		DryMetrics.MeanNeighborStep <= 400.f);
	TestTrue(TEXT("dry mountains expose broad buildable valleys"),
		DryMetrics.BuildableFraction >= 0.08f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersArchipelagoProfileTest,
	"Gaters.Worldgen.Environment.ArchipelagoProfile",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersArchipelagoProfileTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(2, 400000.f);
	TestEqual(TEXT("seed 2 remains the held-out archipelago profile"),
		Environment.Type, EGatersEnvironment::Archipelago);
	TestEqual(TEXT("archipelago retains ocean hydrology"),
		Environment.Hydrology, EGatersHydrology::Ocean);
	TestTrue(TEXT("ocean datum stays below the flattened arrival terrain"),
		Environment.WaterHeight <= -100.f);
	TestTrue(TEXT("arrival island clears the ocean"),
		Environment.HeightAt(FVector2D::ZeroVector) > Environment.WaterHeight + 300.f);
	const TArray<FGatersWaterSurface> OceanSurfaces = Environment.WaterSurfaces();
	TestEqual(TEXT("ocean uses one cheap surface"), OceanSurfaces.Num(), 1);
	TestTrue(TEXT("ocean surface extends beyond the visible world boundary"),
		OceanSurfaces[0].HalfExtent >= Environment.ChunkSize * 4.f);

	const FGatersTerrainEvaluation Metrics = FGatersTerrainEvaluator::Evaluate(
		Environment, TestChunkSize);
	AddInfo(FString::Printf(TEXT("seed 2 local water=%.3f buildable=%.3f relief=%.0f"),
		Metrics.WaterFraction, Metrics.BuildableFraction, Metrics.Relief()));
	TestTrue(TEXT("runtime archipelago exposes meaningful local water"),
		Metrics.WaterFraction >= 0.15f);
	TestTrue(TEXT("runtime archipelago remains mostly navigable land"),
		Metrics.WaterFraction <= 0.65f);
	TestTrue(TEXT("runtime archipelago retains buildable land"),
		Metrics.BuildableFraction >= 0.10f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersArchipelagoVarietyTest,
	"Gaters.Worldgen.Environment.ArchipelagoVariety",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersArchipelagoVarietyTest::RunTest(const FString& Parameters)
{
	int32 MinimumIslandCount = TNumericLimits<int32>::Max();
	int32 MaximumIslandCount = 0;
	int32 ArchipelagoSeeds = 0;
	int32 ExampleSeeds[5] = { -1, -1, -1, -1, -1 };
	TSet<uint32> CoastlineHashes;
	for (int32 Seed = 0; Seed < 128; ++Seed)
	{
		const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(Seed, 400000.f);
		if (Environment.Type != EGatersEnvironment::Archipelago)
		{
			continue;
		}

		++ArchipelagoSeeds;
		const FArchipelagoTopology Topology = MeasureArchipelagoTopology(Environment);
		MinimumIslandCount = FMath::Min(MinimumIslandCount, Topology.SignificantIslandCount);
		MaximumIslandCount = FMath::Max(MaximumIslandCount, Topology.SignificantIslandCount);
		if (Topology.SignificantIslandCount < UE_ARRAY_COUNT(ExampleSeeds) &&
			ExampleSeeds[Topology.SignificantIslandCount] < 0)
		{
			ExampleSeeds[Topology.SignificantIslandCount] = Seed;
		}
		CoastlineHashes.Add(Topology.CoastlineHash);
	}

	AddInfo(FString::Printf(TEXT(
		"archipelago seeds=%d island-count-range=%d..%d coastline-signatures=%d"),
		ArchipelagoSeeds, MinimumIslandCount, MaximumIslandCount, CoastlineHashes.Num()));
	AddInfo(FString::Printf(TEXT("topology examples: two=%d three=%d four=%d"),
		ExampleSeeds[2], ExampleSeeds[3], ExampleSeeds[4]));
	TestTrue(TEXT("seed sweep contains enough archipelago evidence"), ArchipelagoSeeds >= 16);
	TestTrue(TEXT("archipelago seeds can produce two-island layouts"), MinimumIslandCount <= 2);
	TestTrue(TEXT("archipelago seeds can produce four-island layouts"), MaximumIslandCount >= 4);
	TestTrue(TEXT("archipelago coastlines differ beyond rotation"), CoastlineHashes.Num() >= 8);
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
	TestEqual(TEXT("logical world extends beyond the origin content site"), Defaults->WorldSize, 400000.f);
	TestEqual(TEXT("analysis grid still covers the larger chunk at five-meter resolution"), Defaults->GridN, 61);
	TestEqual(TEXT("streamed terrain cells are one hundred meters wide"), Defaults->TerrainCellSize, 10000.f);
	TestEqual(TEXT("streamed cells retain useful local detail"), Defaults->TerrainCellResolution, 64);
	TestTrue(TEXT("materialized terrain vertices are no more than 1.6 meters apart"),
		Defaults->TerrainCellSize / Defaults->TerrainCellResolution <= 160.f);
	TestEqual(TEXT("the prototype keeps a bounded three-by-three terrain set"), Defaults->TerrainLoadRadius, 1);
	TestTrue(TEXT("gallery captures can load beyond the gameplay terrain radius"),
		Defaults->GalleryTerrainLoadRadius > Defaults->TerrainLoadRadius);
	TestTrue(TEXT("terrain rewrite invalidates old seed diffs"), GatersGenVersion >= 2);
	return true;
}

#endif
