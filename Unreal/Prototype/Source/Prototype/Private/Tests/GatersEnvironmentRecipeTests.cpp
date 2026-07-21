#if WITH_DEV_AUTOMATION_TESTS

#include "GatersEnvironmentRecipe.h"

#include "GatersBiomeField.h"
#include "GatersBiomeOpportunityField.h"
#include "GatersClimateField.h"
#include "GatersEnvironmentBrief.h"
#include "GatersIntentTerrainField.h"
#include "GatersLandformProcessField.h"
#include "GatersSurfaceConditionField.h"
#include "GatersTerrainSemanticField.h"
#include "GatersWorldRecipe.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersEnvironmentRecipeContractTest,
	"Gaters.Worldgen.EnvironmentRecipe.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersEnvironmentRecipeContractTest::RunTest(const FString& Parameters)
{
	constexpr float WorldSize = 400000.f;
	bool bRegionalTerrainDiffersFromRawTerrain = false;
	const FGatersBiomeQuery BiomeQuery{
		0.f, FVector2D::ZeroVector, 200.f, 4000.f};
	for (const int32 Seed : {0, 2, 4, 7, 53})
	{
		const FGatersEnvironmentRecipe A =
			FGatersEnvironmentRecipeCompiler::Compile(Seed, WorldSize);
		const FGatersEnvironmentRecipe B =
			FGatersEnvironmentRecipeCompiler::Compile(Seed, WorldSize);

		TestEqual(TEXT("recipe contract is versioned"), A.Version,
			FGatersEnvironmentRecipe::CurrentVersion);
		TestEqual(TEXT("compiler contract is versioned"), A.CompilerVersion,
			FGatersEnvironmentRecipe::CurrentCompilerVersion);
		TestEqual(TEXT("recipe records its seed"), A.Seed, Seed);
		TestEqual(TEXT("recipe records its world size"), A.WorldSize, WorldSize);
		TestEqual(TEXT("terrain records the recipe seed"), A.Terrain.Seed, Seed);
		TestEqual(TEXT("intent records the recipe seed"), A.Intent.Seed, Seed);
		TestEqual(TEXT("same input reproduces exact intent"), A.Intent, B.Intent);
		TestEqual(TEXT("same input reproduces exact regional water"),
			A.RegionalWater, B.RegionalWater);
		TestEqual(TEXT("same input reproduces exact environment brief"),
			A.EnvironmentBrief, B.EnvironmentBrief);
		TestEqual(TEXT("same input reproduces exact climate recipe"),
			A.Climate, B.Climate);
		TestEqual(TEXT("same input reproduces exact drainage recipe"),
			A.Drainage, B.Drainage);
		TestEqual(TEXT("same input reproduces exact drainage-water fit"),
			A.DrainageWaterFit, B.DrainageWaterFit);
		TestEqual(TEXT("same input reproduces exact drainage features"),
			A.DrainageFeatures, B.DrainageFeatures);
		TestEqual(TEXT("same input reproduces exact surface conditions"),
			A.SurfaceConditions, B.SurfaceConditions);
		TestEqual(TEXT("same input reproduces exact biome opportunities"),
			A.BiomeOpportunities, B.BiomeOpportunities);
		TestEqual(TEXT("root drainage uses its bounded evidence resolution"),
			A.Drainage.CellsPerAxis, 129);
		TestEqual(TEXT("climate records the recipe seed"), A.Climate.Seed, Seed);
		TestEqual(TEXT("climate records the recipe world size"),
			A.Climate.WorldSize, WorldSize);
		TestEqual(TEXT("compiled water matches its direct source"), A.RegionalWater,
			FGatersRegionalWaterRecipe::Generate(A.Terrain, A.Intent));
		TestEqual(TEXT("intent can consume the compiled terrain without reconstruction"),
			A.Intent, FGatersWorldIntentRecipe::Generate(A.Terrain));
		FGatersDrainageSettings DrainageSettings;
		DrainageSettings.CellsPerAxis = A.Drainage.CellsPerAxis;
		DrainageSettings.Extent = A.Drainage.Extent;
		DrainageSettings.ChannelAccumulationThreshold =
			A.Drainage.ChannelAccumulationThreshold;
		DrainageSettings.WaterfallDropThreshold =
			A.Drainage.WaterfallDropThreshold;
		const FGatersDrainageBuildResult DirectDrainage =
			FGatersDrainageNetwork::Build(A, DrainageSettings);
		TestTrue(TEXT("direct root drainage builds"), DirectDrainage.IsValid());
		TestEqual(TEXT("root drainage matches its direct source"),
			A.Drainage, DirectDrainage.Recipe);
		const FGatersDrainageWaterFitResult DirectWaterFit =
			FGatersDrainageNetwork::FitRegionalWater(
				A.Drainage, A.RegionalWater, A.DrainageWaterFit.DatumTolerance);
		TestEqual(TEXT("root water fit matches its direct source"),
			A.DrainageWaterFit, DirectWaterFit);
		const FGatersDrainageFeatureCompileResult DirectFeatures =
			FGatersDrainageNetwork::CompileFeatureCandidates(
				A.Drainage,
				A.RegionalWater,
				A.DrainageWaterFit,
				A.DrainageFeatures.Settings);
		TestTrue(TEXT("direct root feature candidates compile"),
			DirectFeatures.IsValid());
		TestEqual(TEXT("root features match their direct source"),
			A.DrainageFeatures, DirectFeatures.Recipe);
		const FGatersSurfaceConditionCompileResult DirectSurfaceConditions =
			FGatersSurfaceConditionField::Compile(
				A, A.SurfaceConditions.Settings);
		TestTrue(TEXT("direct root surface conditions compile"),
			DirectSurfaceConditions.IsValid());
		TestEqual(TEXT("root surface conditions match their direct source"),
			A.SurfaceConditions, DirectSurfaceConditions.Recipe);
		const FGatersWorldRecipe RootWorld = FGatersWorldRecipe::Generate(
			A.Terrain, 6000.f, 10800.f, 900.f, 350.f);
		const FGatersWorldRecipe SeedWorld = FGatersWorldRecipe::Generate(
			Seed, WorldSize, 6000.f, 10800.f, 900.f, 350.f);
		TestEqual(TEXT("world recipe consumes compiled terrain without semantic drift"),
			RootWorld.CanonicalText(), SeedWorld.CanonicalText());

		TArray<FString> Errors;
		const bool bValid = A.Validate(Errors);
		for (const FString& Error : Errors)
		{
			AddInfo(FString::Printf(TEXT("seed=%d environment-recipe diagnostic=%s"),
				Seed, *Error));
		}
		if (!bValid)
		{
			for (const FGatersWorldRegionIntent& Region : A.Intent.Regions)
			{
				AddInfo(FString::Printf(
					TEXT("seed=%d region=%s center=(%.1f,%.1f) radius=%.1f terrain=%d hydrology=%d"),
					Seed, *Region.Id, Region.Center.X, Region.Center.Y, Region.Radius,
					static_cast<int32>(Region.TerrainTendency),
					static_cast<int32>(Region.HydrologyTendency)));
			}
			for (const FGatersRegionalWaterSurface& Surface : A.RegionalWater.Surfaces)
			{
				AddInfo(FString::Printf(
					TEXT("seed=%d surface=%s region=%s center=(%.1f,%.1f) extent=%.1f height=%.1f hydrology=%d"),
					Seed, *Surface.Id, *Surface.RegionId, Surface.Center.X, Surface.Center.Y,
					Surface.HalfExtent, Surface.Height,
					static_cast<int32>(Surface.Hydrology)));
			}
		}
		TestTrue(TEXT("compiled recipe validates"), bValid);
		TestTrue(TEXT("valid recipe has no diagnostics"), Errors.IsEmpty());

		const TArray<FVector2D> Points = {
			FVector2D::ZeroVector,
			A.Intent.Regions[1].Center,
			A.Intent.Regions[2].Center};
		for (const FVector2D& Point : Points)
		{
			const FGatersIntentTerrainSample Terrain = A.QueryTerrain(Point);
			TestEqual(TEXT("same input reproduces exact terrain queries"),
				Terrain, B.QueryTerrain(Point));
			TestEqual(TEXT("terrain query delegates without semantic drift"), Terrain,
				FGatersIntentTerrainField::Query(A.Terrain, A.Intent, Point));
			const FVector2D RouteTarget(7000.f, -2500.f);
			TestEqual(TEXT("materialized height remains authoritative"),
				A.MaterializedHeightAt(Point, 900.f, RouteTarget),
				FGatersTerrainSemanticField::MaterializedHeightAt(
					A.Terrain, A.Intent, Point, 900.f, RouteTarget));
			TestEqual(TEXT("materialized normal remains authoritative"),
				A.MaterializedNormalAt(Point, 200.f, 900.f, RouteTarget),
				FGatersTerrainSemanticField::MaterializedNormalAt(
					A.Terrain, A.Intent, Point, 200.f, 900.f, RouteTarget));

			const FGatersBiomeSample Biome = A.QueryBiome(Point, BiomeQuery);
			TestEqual(TEXT("same input reproduces exact biome queries"),
				Biome, B.QueryBiome(Point, BiomeQuery));
			TestEqual(TEXT("biome query delegates without semantic drift"), Biome,
				FGatersBiomeField::Query(A.Terrain, A.Intent, Point, BiomeQuery));
			const FGatersBiomeOpportunitySample Opportunities =
				A.QueryOpportunities(Point, BiomeQuery);
			const FGatersClimateSample Climate = A.QueryClimate(Point);
			TestEqual(TEXT("same input reproduces exact climate queries"),
				Climate, B.QueryClimate(Point));
			TestEqual(TEXT("climate query delegates without semantic drift"), Climate,
				FGatersClimateField::Query(
					A.Climate,
					Point,
					[&A](const FVector2D& SamplePoint)
					{
						return A.QueryTerrain(SamplePoint).Height;
					},
					A.Terrain.HasWater(),
					A.Terrain.WaterHeight));
			TestEqual(TEXT("climate query preserves authoritative root terrain height"),
				Climate.Height, Terrain.Height);
			const FGatersSurfaceConditionSample Surface =
				A.QuerySurfaceConditions(Point);
			TestEqual(TEXT("same input reproduces exact surface queries"),
				Surface, B.QuerySurfaceConditions(Point));
			TestEqual(TEXT("surface query delegates without semantic drift"), Surface,
				FGatersSurfaceConditionField::Query(
					A.SurfaceConditions, A, Point));
			TestEqual(TEXT("surface query preserves authoritative root terrain height"),
				Surface.Evidence.Height, Terrain.Height);
			bRegionalTerrainDiffersFromRawTerrain |= !FMath::IsNearlyEqual(
				Terrain.Height, A.Terrain.HeightAt(Point), 0.01f);
			TestEqual(TEXT("same input reproduces exact opportunity queries"),
				Opportunities, B.QueryOpportunities(Point, BiomeQuery));
			TestEqual(TEXT("opportunity query consumes accepted physical evidence"),
				Opportunities, FGatersBiomeOpportunityField::Evaluate(
					Biome, Climate, Surface));
			TestTrue(TEXT("terrain height is finite"), FMath::IsFinite(Terrain.Height));
			TestTrue(TEXT("biome values are finite"),
				FMath::IsFinite(Biome.Height) && FMath::IsFinite(Biome.NormalZ) &&
				FMath::IsFinite(Biome.WaterProximity) && FMath::IsFinite(Biome.Moisture) &&
				FMath::IsFinite(Biome.Exposure));
			for (const float Value : {
				Biome.NormalZ, Biome.WaterProximity, Biome.Moisture, Biome.Exposure,
				Opportunities.Vegetation, Opportunities.Stone,
				Opportunities.Landmark, Opportunities.TravelFriction})
			{
				TestTrue(TEXT("semantic and opportunity values are bounded"),
					Value >= 0.f && Value <= 1.f);
			}
			TestFalse(TEXT("compiled biome has a semantic key"), Biome.BiomeKey.IsEmpty());
		}
	}
	TestTrue(TEXT("fixture exercises regional root terrain rather than only raw terrain"),
		bRegionalTerrainDiffersFromRawTerrain);

	const int32 SelectedSeed = 83;
	const FGatersEnvironment BaseTerrain =
		FGatersEnvironment::FromSeed(SelectedSeed, WorldSize);
	const FGatersCompiledEnvironmentBrief PhysicalIntent =
		FGatersEnvironmentBriefCompiler::Compile(
			FGatersEnvironmentBrief().WithGlobalLandformTargets(0.9f, 0.2f, 0.1f),
			SelectedSeed, WorldSize).Intent;
	const FGatersLandformProcessRecipe SelectedLandform =
		FGatersLandformProcessField::Compile(
			BaseTerrain, PhysicalIntent, {}, 4).Recipe;
	const FGatersEnvironment AcceptedTerrain =
		BaseTerrain.WithLandformProcesses(SelectedLandform);
	const FGatersEnvironmentRecipe SelectedRoot =
		FGatersEnvironmentRecipeCompiler::Compile(
			AcceptedTerrain, PhysicalIntent, SelectedLandform);
	TArray<FString> SelectedErrors;
	TestTrue(TEXT("selected-landform root validates"),
		SelectedRoot.Validate(SelectedErrors));
	TestEqual(TEXT("root preserves selected landform climate provenance"),
		SelectedRoot.Climate.Landform, SelectedLandform);
	const FVector2D SelectedPoint(21000.f, -13000.f);
	TestEqual(TEXT("selected-landform climate samples accepted terrain once"),
		SelectedRoot.QueryClimate(SelectedPoint).Height,
		SelectedRoot.QueryTerrain(SelectedPoint).Height);

	const FGatersEnvironmentRecipe Seven =
		FGatersEnvironmentRecipeCompiler::Compile(7, WorldSize);
	const FGatersEnvironmentRecipe FiftyThree =
		FGatersEnvironmentRecipeCompiler::Compile(53, WorldSize);
	TestNotEqual(TEXT("different seeds retain distinct identity"),
		Seven.Seed, FiftyThree.Seed);
	TestNotEqual(TEXT("held-out seeds produce distinct terrain evidence"),
		Seven.QueryTerrain(FVector2D(24000.f, -17000.f)).Height,
		FiftyThree.QueryTerrain(FVector2D(24000.f, -17000.f)).Height);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersEnvironmentRecipeDrainageRootHeldOutTest,
	"Gaters.Worldgen.EnvironmentRecipe.DrainageRootHeldOut",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersEnvironmentRecipeDrainageRootHeldOutTest::RunTest(
	const FString& Parameters)
{
	for (const int32 Seed : {11, 29, 47, 83, 131})
	{
		const FGatersEnvironmentRecipe A =
			FGatersEnvironmentRecipeCompiler::Compile(Seed, 120000.f);
		const FGatersEnvironmentRecipe B =
			FGatersEnvironmentRecipeCompiler::Compile(Seed, 120000.f);
		TArray<FString> Errors;
		TestTrue(FString::Printf(TEXT("seed %d complete root validates"), Seed),
			A.Validate(Errors));
		TestTrue(FString::Printf(TEXT("seed %d complete root has no diagnostics"), Seed),
			Errors.IsEmpty());
		TestEqual(FString::Printf(TEXT("seed %d root drainage repeats"), Seed),
			A.Drainage, B.Drainage);
		TestEqual(FString::Printf(TEXT("seed %d root water fit repeats"), Seed),
			A.DrainageWaterFit, B.DrainageWaterFit);
		TestEqual(FString::Printf(TEXT("seed %d root features repeat"), Seed),
			A.DrainageFeatures, B.DrainageFeatures);
		TestEqual(FString::Printf(TEXT("seed %d root surface conditions repeat"), Seed),
			A.SurfaceConditions, B.SurfaceConditions);
		TestEqual(FString::Printf(TEXT("seed %d water association count matches"), Seed),
			A.DrainageWaterFit.Surfaces.Num(), A.RegionalWater.Surfaces.Num());
		TestEqual(FString::Printf(TEXT("seed %d feature provenance matches"), Seed),
			A.DrainageFeatures.Seed, Seed);
		TestEqual(FString::Printf(TEXT("seed %d surface provenance matches"), Seed),
			A.SurfaceConditions.Seed, Seed);
		TestEqual(FString::Printf(TEXT("seed %d root surface query repeats"), Seed),
			A.QuerySurfaceConditions(FVector2D(12000.f, -9000.f)),
			B.QuerySurfaceConditions(FVector2D(12000.f, -9000.f)));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersEnvironmentRecipeCounterexampleTest,
	"Gaters.Worldgen.EnvironmentRecipe.Counterexamples",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersEnvironmentRecipeCounterexampleTest::RunTest(const FString& Parameters)
{
	FGatersEnvironmentRecipe WrongWaterVersion =
		FGatersEnvironmentRecipeCompiler::Compile(7, 400000.f);
	WrongWaterVersion.RegionalWater.Version = 99;
	TArray<FString> Errors;
	TestFalse(TEXT("unknown regional-water versions are rejected"),
		WrongWaterVersion.Validate(Errors));
	TestTrue(TEXT("water-version failure is causal"), Errors.ContainsByPredicate(
		[](const FString& Error) { return Error.Contains(TEXT("water version")); }));

	FGatersEnvironmentRecipe WrongSeed =
		FGatersEnvironmentRecipeCompiler::Compile(7, 400000.f);
	WrongSeed.Intent.Seed = 8;
	Errors.Reset();
	TestFalse(TEXT("mismatched component provenance is rejected"),
		WrongSeed.Validate(Errors));
	TestTrue(TEXT("provenance failure is causal"), Errors.ContainsByPredicate(
		[](const FString& Error) { return Error.Contains(TEXT("seed provenance")); }));

	FGatersEnvironmentRecipe WrongClimate =
		FGatersEnvironmentRecipeCompiler::Compile(7, 400000.f);
	WrongClimate.Climate.Version = 99;
	Errors.Reset();
	TestFalse(TEXT("unsupported climate versions are rejected"),
		WrongClimate.Validate(Errors));
	TestTrue(TEXT("climate-version failure is causal"), Errors.ContainsByPredicate(
		[](const FString& Error) { return Error.Contains(TEXT("climate")); }));

	FGatersEnvironmentRecipe WrongEnvironmentBrief =
		FGatersEnvironmentRecipeCompiler::Compile(7, 400000.f);
	WrongEnvironmentBrief.EnvironmentBrief.Seed = 8;
	Errors.Reset();
	TestFalse(TEXT("mismatched environment-brief provenance is rejected"),
		WrongEnvironmentBrief.Validate(Errors));
	TestTrue(TEXT("environment-brief failure is causal"), Errors.ContainsByPredicate(
		[](const FString& Error) { return Error.Contains(TEXT("environment brief")); }));

	FGatersEnvironmentRecipe WrongDrainage =
		FGatersEnvironmentRecipeCompiler::Compile(7, 400000.f);
	WrongDrainage.Drainage.Version = 99;
	Errors.Reset();
	TestFalse(TEXT("unsupported drainage versions are rejected"),
		WrongDrainage.Validate(Errors));
	TestTrue(TEXT("drainage failure is causal"), Errors.ContainsByPredicate(
		[](const FString& Error) { return Error.Contains(TEXT("drainage provenance")); }));

	FGatersEnvironmentRecipe WrongDrainageFit =
		FGatersEnvironmentRecipeCompiler::Compile(7, 400000.f);
	WrongDrainageFit.DrainageWaterFit.DatumTolerance = -1.f;
	Errors.Reset();
	TestFalse(TEXT("corrupted drainage-water fit is rejected"),
		WrongDrainageFit.Validate(Errors));
	TestTrue(TEXT("drainage-water failure is causal"), Errors.ContainsByPredicate(
		[](const FString& Error) { return Error.Contains(TEXT("drainage water fit")); }));

	FGatersEnvironmentRecipe WrongDrainageFeatures =
		FGatersEnvironmentRecipeCompiler::Compile(7, 400000.f);
	WrongDrainageFeatures.DrainageFeatures.Settings.WetlandMaximumDrop = -1.f;
	Errors.Reset();
	TestFalse(TEXT("corrupted drainage features are rejected"),
		WrongDrainageFeatures.Validate(Errors));
	TestTrue(TEXT("drainage-feature failure is causal"), Errors.ContainsByPredicate(
		[](const FString& Error) { return Error.Contains(TEXT("drainage features")); }));

	FGatersEnvironmentRecipe WrongSurfaceConditions =
		FGatersEnvironmentRecipeCompiler::Compile(7, 400000.f);
	WrongSurfaceConditions.SurfaceConditions.Version = 99;
	Errors.Reset();
	TestFalse(TEXT("unsupported surface-condition versions are rejected"),
		WrongSurfaceConditions.Validate(Errors));
	TestTrue(TEXT("surface-condition failure is causal"), Errors.ContainsByPredicate(
		[](const FString& Error) { return Error.Contains(TEXT("surface conditions")); }));

	WrongSurfaceConditions =
		FGatersEnvironmentRecipeCompiler::Compile(7, 400000.f);
	WrongSurfaceConditions.SurfaceConditions.Settings.NormalSampleDistance = 0.f;
	Errors.Reset();
	TestFalse(TEXT("corrupted surface-condition settings are rejected"),
		WrongSurfaceConditions.Validate(Errors));
	TestTrue(TEXT("surface-condition settings failure is causal"),
		Errors.ContainsByPredicate(
			[](const FString& Error)
			{
				return Error.Contains(TEXT("surface conditions"));
			}));

	FGatersEnvironmentRecipe WrongBiomeOpportunities =
		FGatersEnvironmentRecipeCompiler::Compile(7, 400000.f);
	WrongBiomeOpportunities.BiomeOpportunities.Version = 99;
	Errors.Reset();
	TestFalse(TEXT("unsupported biome-opportunity versions are rejected"),
		WrongBiomeOpportunities.Validate(Errors));
	TestTrue(TEXT("biome-opportunity failure is causal"),
		Errors.ContainsByPredicate(
			[](const FString& Error)
			{
				return Error.Contains(TEXT("biome opportunities"));
			}));

	FGatersEnvironmentRecipe UnknownWaterRegion =
		FGatersEnvironmentRecipeCompiler::Compile(7, 400000.f);
	if (UnknownWaterRegion.RegionalWater.Surfaces.IsEmpty())
	{
		FGatersRegionalWaterSurface& Surface =
			UnknownWaterRegion.RegionalWater.Surfaces.AddDefaulted_GetRef();
		Surface.Id = TEXT("water:test");
		Surface.RegionId = TEXT("intent:missing");
		Surface.RegionRadius = 1000.f;
		Surface.HalfExtent = 100.f;
	}
	else
	{
		UnknownWaterRegion.RegionalWater.Surfaces[0].RegionId = TEXT("intent:missing");
	}
	Errors.Reset();
	TestFalse(TEXT("water without an intent region is rejected"),
		UnknownWaterRegion.Validate(Errors));
	TestTrue(TEXT("water-region failure is causal"), Errors.ContainsByPredicate(
		[](const FString& Error) { return Error.Contains(TEXT("water region")); }));

	FGatersEnvironmentRecipe InvalidWaterIdentity =
		FGatersEnvironmentRecipeCompiler::Compile(7, 400000.f);
	TestFalse(TEXT("identity fixture has regional water"),
		InvalidWaterIdentity.RegionalWater.Surfaces.IsEmpty());
	if (!InvalidWaterIdentity.RegionalWater.Surfaces.IsEmpty())
	{
		InvalidWaterIdentity.RegionalWater.Surfaces[0].Id.Reset();
		Errors.Reset();
		TestFalse(TEXT("empty regional-water identities are rejected"),
			InvalidWaterIdentity.Validate(Errors));
		TestTrue(TEXT("water-identity failure is causal"), Errors.ContainsByPredicate(
			[](const FString& Error) { return Error.Contains(TEXT("water identity")); }));
	}

	FGatersEnvironmentRecipe InvalidWaterBounds =
		FGatersEnvironmentRecipeCompiler::Compile(7, 400000.f);
	if (!InvalidWaterBounds.RegionalWater.Surfaces.IsEmpty())
	{
		InvalidWaterBounds.RegionalWater.Surfaces[0].HalfExtent = -1.f;
		Errors.Reset();
		TestFalse(TEXT("invalid regional-water geometry is rejected"),
			InvalidWaterBounds.Validate(Errors));
		TestTrue(TEXT("water-bounds failure is causal"), Errors.ContainsByPredicate(
			[](const FString& Error) { return Error.Contains(TEXT("water bounds")); }));
	}
	return true;
}

#endif
