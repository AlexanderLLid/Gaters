#if WITH_DEV_AUTOMATION_TESTS

#include "GatersDrainageNetwork.h"
#include "GatersEnvironmentRecipe.h"
#include "GatersRegionalWaterRecipe.h"
#include "Misc/AutomationTest.h"

#include <limits>

namespace
{
bool HasIssue(const FGatersDrainageBuildResult& Result, const TCHAR* RuleId)
{
	return Result.Issues.ContainsByPredicate(
		[RuleId](const FGatersDrainageIssue& Issue)
		{
			return Issue.RuleId == RuleId;
		});
}

bool HasIssue(const FGatersDrainageWaterFitResult& Result, const TCHAR* RuleId)
{
	return Result.Issues.ContainsByPredicate(
		[RuleId](const FGatersDrainageIssue& Issue)
		{
			return Issue.RuleId == RuleId;
		});
}

bool HasIssue(const FGatersDrainageFeatureCompileResult& Result, const TCHAR* RuleId)
{
	return Result.Issues.ContainsByPredicate(
		[RuleId](const FGatersDrainageIssue& Issue)
		{
			return Issue.RuleId == RuleId;
		});
}

int32 CountFeatures(
	const FGatersDrainageFeatureRecipe& Recipe,
	const EGatersDrainageFeatureKind Kind)
{
	int32 Result = 0;
	for (const FGatersDrainageFeatureCandidate& Candidate : Recipe.Candidates)
	{
		Result += Candidate.Kind == Kind ? 1 : 0;
	}
	return Result;
}

FGatersDrainageSettings SmallSettings()
{
	FGatersDrainageSettings Settings;
	Settings.CellsPerAxis = 7;
	Settings.Extent = 3000.f;
	Settings.ChannelAccumulationThreshold = 1.5f;
	Settings.WaterfallDropThreshold = 800.f;
	return Settings;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersDrainageFeatureCandidatesSyntheticTest,
	"Gaters.Worldgen.DrainageNetwork.FeatureCandidatesSynthetic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersDrainageFeatureCandidatesSyntheticTest::RunTest(
	const FString& Parameters)
{
	FGatersDrainageSettings DrainageSettings = SmallSettings();
	DrainageSettings.ChannelAccumulationThreshold = 0.1f;
	DrainageSettings.WaterfallDropThreshold = 800.f;
	const FGatersDrainageBuildResult Drainage = FGatersDrainageNetwork::Build(
		31,
		120000.f,
		DrainageSettings,
		[](const FVector2D& Point)
		{
			return -Point.X * 0.05f + (Point.X < 0.f ? 1200.f : 0.f);
		},
		[](const FVector2D&) { return 0.9f; });
	TestTrue(TEXT("feature fixture drainage builds"), Drainage.IsValid());

	FGatersRegionalWaterRecipe Water;
	FGatersRegionalWaterSurface Lake;
	Lake.Id = TEXT("water:lake");
	Lake.RegionId = TEXT("region:wet");
	Lake.RegionCenter = FVector2D(3000.f, 0.f);
	Lake.RegionRadius = 3000.f;
	Lake.Center = FVector2D(3000.f, -1000.f);
	Lake.HalfExtent = 400.f;
	Lake.Height = 0.f;
	Lake.Hydrology = EGatersHydrology::Lakes;
	Water.Surfaces.Add(Lake);
	FGatersRegionalWaterSurface Ocean = Lake;
	Ocean.Id = TEXT("water:ocean");
	Ocean.Center = FVector2D(3000.f, 1000.f);
	Ocean.Hydrology = EGatersHydrology::Ocean;
	Water.Surfaces.Add(Ocean);
	const FGatersDrainageWaterFitResult Fit =
		FGatersDrainageNetwork::FitRegionalWater(Drainage.Recipe, Water);
	TestTrue(TEXT("feature fixture water fits"), Fit.IsValid());

	FGatersDrainageFeatureSettings FeatureSettings;
	FeatureSettings.WetlandMaximumDrop = 100.f;
	const FGatersDrainageFeatureCompileResult A =
		FGatersDrainageNetwork::CompileFeatureCandidates(
			Drainage.Recipe, Water, Fit, FeatureSettings);
	const FGatersDrainageFeatureCompileResult B =
		FGatersDrainageNetwork::CompileFeatureCandidates(
			Drainage.Recipe, Water, Fit, FeatureSettings);
	TestTrue(TEXT("feature candidates compile"), A.IsValid());
	TestEqual(TEXT("feature recipe is versioned"), A.Recipe.Version,
		FGatersDrainageFeatureRecipe::CurrentVersion);
	TestEqual(TEXT("feature recipe records its settings"),
		A.Recipe.Settings, FeatureSettings);
	TestEqual(TEXT("same feature inputs repeat exactly"), A, B);
	for (const EGatersDrainageFeatureKind Kind : {
		EGatersDrainageFeatureKind::RiverSystem,
		EGatersDrainageFeatureKind::Lake,
		EGatersDrainageFeatureKind::Wetland,
		EGatersDrainageFeatureKind::Delta,
		EGatersDrainageFeatureKind::Waterfall})
	{
		TestTrue(TEXT("fixture emits every supported feature kind"),
			CountFeatures(A.Recipe, Kind) > 0);
	}

	TSet<FString> Ids;
	for (const FGatersDrainageFeatureCandidate& Candidate : A.Recipe.Candidates)
	{
		TestFalse(TEXT("feature identity is stable"), Candidate.Id.IsEmpty());
		TestFalse(TEXT("feature identity is unique"), Ids.Contains(Candidate.Id));
		Ids.Add(Candidate.Id);
		if (Candidate.Kind == EGatersDrainageFeatureKind::RiverSystem
			|| Candidate.Kind == EGatersDrainageFeatureKind::Waterfall)
		{
			TestTrue(TEXT("channel-derived feature preserves segments"),
				!Candidate.SegmentIds.IsEmpty());
		}
		if (Candidate.Kind == EGatersDrainageFeatureKind::Lake
			|| Candidate.Kind == EGatersDrainageFeatureKind::Delta)
		{
			TestFalse(TEXT("surface-derived feature preserves surface identity"),
				Candidate.SurfaceId.IsEmpty());
		}
		if (Candidate.Kind == EGatersDrainageFeatureKind::Wetland)
		{
			TestTrue(TEXT("wetland preserves qualifying cells"),
				!Candidate.CellIndices.IsEmpty());
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersDrainageFeatureCandidatesCounterexampleTest,
	"Gaters.Worldgen.DrainageNetwork.FeatureCandidatesCounterexamples",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersDrainageFeatureCandidatesCounterexampleTest::RunTest(
	const FString& Parameters)
{
	FGatersDrainageSettings DrainageSettings = SmallSettings();
	DrainageSettings.ChannelAccumulationThreshold = 1000.f;
	const FGatersDrainageBuildResult Drainage = FGatersDrainageNetwork::Build(
		37,
		120000.f,
		DrainageSettings,
		[](const FVector2D&) { return 0.f; },
		[](const FVector2D&) { return 0.f; });
	const FGatersRegionalWaterRecipe EmptyWater;
	const FGatersDrainageWaterFitResult EmptyFit =
		FGatersDrainageNetwork::FitRegionalWater(Drainage.Recipe, EmptyWater);
	const FGatersDrainageFeatureCompileResult Empty =
		FGatersDrainageNetwork::CompileFeatureCandidates(
			Drainage.Recipe, EmptyWater, EmptyFit);
	TestTrue(TEXT("feature scarcity is valid"), Empty.IsValid());
	TestTrue(TEXT("feature scarcity emits no candidates"),
		Empty.Recipe.Candidates.IsEmpty());

	FGatersDrainageRecipe BrokenDrainage = Drainage.Recipe;
	BrokenDrainage.Version = 99;
	TestTrue(TEXT("broken drainage provenance is causal"), HasIssue(
		FGatersDrainageNetwork::CompileFeatureCandidates(
			BrokenDrainage, EmptyWater, EmptyFit),
		TEXT("drainage.feature.drainage")));
	FGatersRegionalWaterRecipe BrokenWater = EmptyWater;
	BrokenWater.Version = 99;
	TestTrue(TEXT("broken water provenance is causal"), HasIssue(
		FGatersDrainageNetwork::CompileFeatureCandidates(
			Drainage.Recipe, BrokenWater, EmptyFit),
		TEXT("drainage.feature.water")));
	FGatersDrainageWaterFitResult BrokenFit = EmptyFit;
	BrokenFit.Version = 99;
	TestTrue(TEXT("broken fit provenance is causal"), HasIssue(
		FGatersDrainageNetwork::CompileFeatureCandidates(
			Drainage.Recipe, EmptyWater, BrokenFit),
		TEXT("drainage.feature.fit")));
	BrokenFit = EmptyFit;
	BrokenFit.DatumTolerance = -1.f;
	TestTrue(TEXT("broken fit settings are causal"), HasIssue(
		FGatersDrainageNetwork::CompileFeatureCandidates(
			Drainage.Recipe, EmptyWater, BrokenFit),
		TEXT("drainage.feature.fit")));
	FGatersDrainageFeatureSettings BrokenSettings;
	BrokenSettings.WetlandMaximumDrop = -1.f;
	TestTrue(TEXT("broken feature settings are causal"), HasIssue(
		FGatersDrainageNetwork::CompileFeatureCandidates(
			Drainage.Recipe, EmptyWater, EmptyFit, BrokenSettings),
		TEXT("drainage.feature.settings")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersDrainageRegionalWaterFitTest,
	"Gaters.Worldgen.DrainageNetwork.RegionalWaterFit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersDrainageRegionalWaterFitTest::RunTest(const FString& Parameters)
{
	FGatersDrainageSettings Settings = SmallSettings();
	Settings.ChannelAccumulationThreshold = 0.1f;
	const FGatersDrainageBuildResult Drainage = FGatersDrainageNetwork::Build(
		17,
		120000.f,
		Settings,
		[](const FVector2D& Point) { return -Point.X; },
		[](const FVector2D&) { return 0.9f; });
	TestTrue(TEXT("synthetic drainage fixture builds"), Drainage.IsValid());

	FGatersRegionalWaterRecipe Water;
	FGatersRegionalWaterSurface& Surface = Water.Surfaces.AddDefaulted_GetRef();
	Surface.Id = TEXT("water:downstream");
	Surface.RegionId = TEXT("region:downstream");
	Surface.RegionCenter = FVector2D(3000.f, 0.f);
	Surface.RegionRadius = 3000.f;
	Surface.Center = FVector2D(3000.f, 0.f);
	Surface.HalfExtent = 650.f;
	Surface.Height = -2500.f;
	Surface.Hydrology = EGatersHydrology::River;

	const FGatersDrainageWaterFitResult A =
		FGatersDrainageNetwork::FitRegionalWater(Drainage.Recipe, Water);
	const FGatersDrainageWaterFitResult B =
		FGatersDrainageNetwork::FitRegionalWater(Drainage.Recipe, Water);
	TestTrue(TEXT("supported downstream water fits drainage"), A.IsValid());
	TestEqual(TEXT("water-fit contract is versioned"), A.Version,
		FGatersDrainageWaterFitResult::CurrentVersion);
	TestEqual(TEXT("water fit repeats exactly"), A, B);
	TestEqual(TEXT("one water surface produces one association"),
		A.Surfaces.Num(), 1);
	TestEqual(TEXT("water fit records its datum tolerance"),
		A.DatumTolerance, 1.f);
	if (A.Surfaces.Num() == 1)
	{
		const FGatersDrainageWaterSurfaceFit& Fit = A.Surfaces[0];
		TestEqual(TEXT("surface identity is preserved"),
			Fit.SurfaceId, Surface.Id);
		TestTrue(TEXT("surface overlaps drainage cells"), !Fit.CellIndices.IsEmpty());
		TestTrue(TEXT("surface records basin identity"), !Fit.BasinIndices.IsEmpty());
		TestTrue(TEXT("surface records reached terminals"),
			!Fit.TerminalCellIndices.IsEmpty());
		TestTrue(TEXT("surface records channel contact"),
			!Fit.ChannelSegmentIds.IsEmpty());
		TestTrue(TEXT("surface records incoming flow"), Fit.IncomingFlowCount > 0);
		TestTrue(TEXT("surface records accumulated precipitation"),
			Fit.MaximumAccumulation > 0.9f);
		TestTrue(TEXT("surface intersects terrain below its datum"),
			Fit.bHasSubmergedTerrain);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersDrainageRegionalWaterFitCounterexampleTest,
	"Gaters.Worldgen.DrainageNetwork.RegionalWaterFitCounterexamples",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersDrainageRegionalWaterFitCounterexampleTest::RunTest(
	const FString& Parameters)
{
	const FGatersDrainageBuildResult Drainage = FGatersDrainageNetwork::Build(
		23,
		120000.f,
		SmallSettings(),
		[](const FVector2D& Point) { return -Point.X; },
		[](const FVector2D&) { return 0.8f; });
	TestTrue(TEXT("counterexample drainage fixture builds"), Drainage.IsValid());

	const FGatersRegionalWaterRecipe Empty;
	const FGatersDrainageWaterFitResult EmptyFit =
		FGatersDrainageNetwork::FitRegionalWater(Drainage.Recipe, Empty);
	TestTrue(TEXT("declared water scarcity is valid"), EmptyFit.IsValid());
	TestTrue(TEXT("declared water scarcity has no associations"),
		EmptyFit.Surfaces.IsEmpty());

	FGatersRegionalWaterRecipe Good;
	FGatersRegionalWaterSurface& Surface = Good.Surfaces.AddDefaulted_GetRef();
	Surface.Id = TEXT("water:test");
	Surface.RegionId = TEXT("region:test");
	Surface.RegionCenter = FVector2D(3000.f, 0.f);
	Surface.RegionRadius = 3000.f;
	Surface.Center = FVector2D(3000.f, 0.f);
	Surface.HalfExtent = 650.f;
	Surface.Height = -2500.f;
	Surface.Hydrology = EGatersHydrology::River;

	FGatersRegionalWaterRecipe WrongVersion = Good;
	WrongVersion.Version = 99;
	TestTrue(TEXT("unsupported water provenance is causal"), HasIssue(
		FGatersDrainageNetwork::FitRegionalWater(Drainage.Recipe, WrongVersion),
		TEXT("drainage.water.recipe")));

	FGatersRegionalWaterRecipe Duplicate = Good;
	const FGatersRegionalWaterSurface DuplicateSurface = Duplicate.Surfaces[0];
	Duplicate.Surfaces.Add(DuplicateSurface);
	TestTrue(TEXT("duplicate water identity is causal"), HasIssue(
		FGatersDrainageNetwork::FitRegionalWater(Drainage.Recipe, Duplicate),
		TEXT("drainage.water.identity")));

	FGatersRegionalWaterRecipe Outside = Good;
	Outside.Surfaces[0].Center = FVector2D(20000.f, 20000.f);
	TestTrue(TEXT("water outside drainage coverage is causal"), HasIssue(
		FGatersDrainageNetwork::FitRegionalWater(Drainage.Recipe, Outside),
		TEXT("drainage.water.coverage")));

	FGatersRegionalWaterRecipe UnsupportedTerrain = Good;
	UnsupportedTerrain.Surfaces[0].Height = -5000.f;
	TestTrue(TEXT("water without submerged terrain is causal"), HasIssue(
		FGatersDrainageNetwork::FitRegionalWater(
			Drainage.Recipe, UnsupportedTerrain),
		TEXT("drainage.water.terrain")));

	FGatersDrainageRecipe InvalidDrainage = Drainage.Recipe;
	InvalidDrainage.Version = 99;
	TestTrue(TEXT("unsupported drainage provenance is causal"), HasIssue(
		FGatersDrainageNetwork::FitRegionalWater(InvalidDrainage, Good),
		TEXT("drainage.water.drainage")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersDrainageNetworkSyntheticTest,
	"Gaters.Worldgen.DrainageNetwork.Synthetic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersDrainageNetworkSyntheticTest::RunTest(const FString& Parameters)
{
	const FGatersDrainageSettings Settings = SmallSettings();
	const auto DownhillPlane = [](const FVector2D& Point)
	{
		return -Point.X;
	};
	const auto Wet = [](const FVector2D&) { return 0.9f; };
	const FGatersDrainageBuildResult A = FGatersDrainageNetwork::Build(
		7, 120000.f, Settings, DownhillPlane, Wet);
	const FGatersDrainageBuildResult B = FGatersDrainageNetwork::Build(
		7, 120000.f, Settings, DownhillPlane, Wet);
	TestTrue(TEXT("finite sloped catchment builds"), A.IsValid());
	TestEqual(TEXT("drainage recipe is versioned"),
		A.Recipe.Version, FGatersDrainageRecipe::CurrentVersion);
	TestTrue(TEXT("same drainage inputs repeat exactly"), A.Recipe == B.Recipe);
	TestEqual(TEXT("grid records every sampled cell"),
		A.Recipe.Cells.Num(), Settings.CellsPerAxis * Settings.CellsPerAxis);
	TestTrue(TEXT("sloped catchment emits channel segments"),
		!A.Recipe.Segments.IsEmpty());

	TSet<FString> SegmentIds;
	float MaximumAccumulation = 0.f;
	for (const FGatersDrainageCell& Cell : A.Recipe.Cells)
	{
		MaximumAccumulation = FMath::Max(MaximumAccumulation, Cell.Accumulation);
		TestTrue(TEXT("cell precipitation is bounded"),
			Cell.Precipitation >= 0.f && Cell.Precipitation <= 1.f);
		TestTrue(TEXT("cell accumulation is finite and non-negative"),
			FMath::IsFinite(Cell.Accumulation) && Cell.Accumulation >= 0.f);
		if (Cell.DownstreamIndex != INDEX_NONE)
		{
			TestTrue(TEXT("downstream index is valid"),
				A.Recipe.Cells.IsValidIndex(Cell.DownstreamIndex));
			TestTrue(TEXT("flow never travels uphill"),
				A.Recipe.Cells[Cell.DownstreamIndex].Height <= Cell.Height);
		}
	}
	for (const FGatersDrainageSegment& Segment : A.Recipe.Segments)
	{
		TestFalse(TEXT("drainage segment identity is stable"), Segment.Id.IsEmpty());
		TestFalse(TEXT("drainage segment identity is unique"),
			SegmentIds.Contains(Segment.Id));
		SegmentIds.Add(Segment.Id);
		TestTrue(TEXT("segment drop is non-negative"), Segment.Drop >= 0.f);
	}
	TestTrue(TEXT("downstream catchment accumulates precipitation"),
		MaximumAccumulation > 2.f);

	const auto Dry = [](const FVector2D&) { return 0.08f; };
	const FGatersDrainageBuildResult DryResult = FGatersDrainageNetwork::Build(
		7, 120000.f, Settings, DownhillPlane, Dry);
	TestTrue(TEXT("dry field remains valid"), DryResult.IsValid());
	TestTrue(TEXT("wet field emits more channel evidence than dry field"),
		A.Recipe.Segments.Num() > DryResult.Recipe.Segments.Num());

	const auto ClosedBasin = [](const FVector2D& Point)
	{
		return Point.SizeSquared() / 10000.f;
	};
	const FGatersDrainageBuildResult Basin = FGatersDrainageNetwork::Build(
		11, 120000.f, Settings, ClosedBasin, Wet);
	TestTrue(TEXT("closed basin builds"), Basin.IsValid());
	TestTrue(TEXT("closed basin exposes a terminal sink"),
		Basin.Recipe.Cells.ContainsByPredicate(
			[](const FGatersDrainageCell& Cell)
			{
				return Cell.bSink && !Cell.bBoundary;
			}));
	const auto FlatPlateau = [](const FVector2D&) { return 500.f; };
	const FGatersDrainageBuildResult Plateau = FGatersDrainageNetwork::Build(
		12, 120000.f, Settings, FlatPlateau, Wet);
	TestTrue(TEXT("flat plateau builds"), Plateau.IsValid());
	TestFalse(TEXT("flat plateau does not become one sink per interior cell"),
		Plateau.Recipe.Cells.ContainsByPredicate(
			[](const FGatersDrainageCell& Cell)
			{
				return Cell.bSink && !Cell.bBoundary;
			}));
	TestTrue(TEXT("flat plateau routes accumulated flow to boundary"),
		Plateau.Recipe.Segments.Num() > 0);

	FGatersDrainageSettings WaterfallSettings = Settings;
	WaterfallSettings.ChannelAccumulationThreshold = 0.1f;
	const auto Escarpment = [](const FVector2D& Point)
	{
		return Point.X < 0.f ? 2200.f - Point.X * 0.05f : 0.f - Point.X * 0.05f;
	};
	const FGatersDrainageBuildResult Waterfall = FGatersDrainageNetwork::Build(
		13, 120000.f, WaterfallSettings, Escarpment, Wet);
	TestTrue(TEXT("escarpment builds"), Waterfall.IsValid());
	TestTrue(TEXT("large supported channel drop becomes a waterfall candidate"),
		Waterfall.Recipe.Segments.ContainsByPredicate(
			[](const FGatersDrainageSegment& Segment)
			{
				return Segment.bWaterfallCandidate && Segment.Drop >= 800.f;
			}));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersDrainageNetworkRealFieldTest,
	"Gaters.Worldgen.DrainageNetwork.RealFields",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersDrainageNetworkRealFieldTest::RunTest(const FString& Parameters)
{
	FGatersDrainageSettings Settings;
	Settings.CellsPerAxis = 17;
	Settings.ChannelAccumulationThreshold = 4.f;
	for (const int32 Seed : {11, 29, 47, 83, 131})
	{
		const FGatersEnvironmentRecipe Environment =
			FGatersEnvironmentRecipeCompiler::Compile(Seed, 120000.f);
		const FGatersDrainageBuildResult A =
			FGatersDrainageNetwork::Build(Environment, Settings);
		const FGatersDrainageBuildResult B =
			FGatersDrainageNetwork::Build(Environment, Settings);
		TestTrue(TEXT("real environment drainage builds"), A.IsValid());
		TestTrue(TEXT("real environment drainage repeats exactly"),
			A.Recipe == B.Recipe);
		TestEqual(TEXT("real drainage preserves seed provenance"),
			A.Recipe.Seed, Seed);
		TestTrue(TEXT("real drainage exposes at least one basin"),
			A.Recipe.BasinCount > 0);
		for (const FGatersDrainageCell& Cell : A.Recipe.Cells)
		{
			TestEqual(TEXT("drainage height comes from authoritative root terrain"),
				Cell.Height, Environment.QueryTerrain(Cell.Center).Height);
			TestEqual(TEXT("drainage precipitation comes from accepted climate"),
				Cell.Precipitation,
				Environment.QueryClimate(Cell.Center).Precipitation);
			if (Cell.DownstreamIndex != INDEX_NONE)
			{
				TestTrue(TEXT("real flow never travels uphill"),
					A.Recipe.Cells[Cell.DownstreamIndex].Height <= Cell.Height);
			}
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersDrainageRealRegionalWaterFitTest,
	"Gaters.Worldgen.DrainageNetwork.RealRegionalWaterFit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersDrainageRealRegionalWaterFitTest::RunTest(const FString& Parameters)
{
	FGatersDrainageSettings Settings;
	Settings.CellsPerAxis = 65;
	Settings.ChannelAccumulationThreshold = 4.f;
	int32 WetSeedCount = 0;
	for (const int32 Seed : {11, 29, 47, 83, 131})
	{
		const FGatersEnvironmentRecipe Environment =
			FGatersEnvironmentRecipeCompiler::Compile(Seed, 120000.f);
		TArray<FString> EnvironmentErrors;
		TestTrue(FString::Printf(TEXT("seed %d environment root validates"), Seed),
			Environment.Validate(EnvironmentErrors));
		const FGatersDrainageBuildResult Drainage =
			FGatersDrainageNetwork::Build(Environment, Settings);
		const FGatersDrainageWaterFitResult A =
			FGatersDrainageNetwork::FitRegionalWater(
				Drainage.Recipe, Environment.RegionalWater);
		const FGatersDrainageWaterFitResult B =
			FGatersDrainageNetwork::FitRegionalWater(
				Drainage.Recipe, Environment.RegionalWater);
		TestTrue(FString::Printf(TEXT("seed %d drainage builds"), Seed),
			Drainage.IsValid());
		TestTrue(FString::Printf(TEXT("seed %d regional water fits"), Seed),
			A.IsValid());
		TestEqual(FString::Printf(TEXT("seed %d fit repeats"), Seed), A, B);
		TestEqual(FString::Printf(TEXT("seed %d preserves water surface count"), Seed),
			A.Surfaces.Num(), Environment.RegionalWater.Surfaces.Num());
		WetSeedCount += Environment.RegionalWater.Surfaces.IsEmpty() ? 0 : 1;
		for (const FGatersDrainageWaterSurfaceFit& Fit : A.Surfaces)
		{
			const FGatersRegionalWaterSurface* Surface =
				Environment.RegionalWater.Surfaces.FindByPredicate(
					[&Fit](const FGatersRegionalWaterSurface& Candidate)
					{
						return Candidate.Id == Fit.SurfaceId;
					});
			float MinimumCellHeight = TNumericLimits<float>::Max();
			for (const int32 CellIndex : Fit.CellIndices)
			{
				MinimumCellHeight = FMath::Min(
					MinimumCellHeight, Drainage.Recipe.Cells[CellIndex].Height);
			}
			TestTrue(FString::Printf(
				TEXT("seed %d surface %s has grid coverage"), Seed, *Fit.SurfaceId),
				!Fit.CellIndices.IsEmpty());
			TestTrue(FString::Printf(
				TEXT("seed %d surface %s has submerged terrain min=%.3f datum=%.3f center_influence=%.3f"),
				Seed,
				*Fit.SurfaceId,
				MinimumCellHeight,
				Surface ? Surface->Height : TNumericLimits<float>::Max(),
				Surface ? Environment.QueryTerrain(Surface->Center).Influence : -1.f),
				Fit.bHasSubmergedTerrain);
			TestTrue(FString::Printf(
				TEXT("seed %d surface %s has basin provenance"), Seed, *Fit.SurfaceId),
				!Fit.BasinIndices.IsEmpty() && !Fit.TerminalCellIndices.IsEmpty());
		}
	}
	TestTrue(TEXT("held-out matrix includes regional water"), WetSeedCount > 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersDrainageRealFeatureCandidatesTest,
	"Gaters.Worldgen.DrainageNetwork.RealFeatureCandidates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersDrainageRealFeatureCandidatesTest::RunTest(const FString& Parameters)
{
	FGatersDrainageSettings DrainageSettings;
	DrainageSettings.CellsPerAxis = 65;
	DrainageSettings.ChannelAccumulationThreshold = 4.f;
	int32 TotalCandidateCount = 0;
	for (const int32 Seed : {11, 29, 47, 83, 131})
	{
		const FGatersEnvironmentRecipe Environment =
			FGatersEnvironmentRecipeCompiler::Compile(Seed, 120000.f);
		const FGatersDrainageBuildResult Drainage =
			FGatersDrainageNetwork::Build(Environment, DrainageSettings);
		const FGatersDrainageWaterFitResult Fit =
			FGatersDrainageNetwork::FitRegionalWater(
				Drainage.Recipe, Environment.RegionalWater);
		const FGatersDrainageFeatureCompileResult A =
			FGatersDrainageNetwork::CompileFeatureCandidates(
				Drainage.Recipe, Environment.RegionalWater, Fit);
		const FGatersDrainageFeatureCompileResult B =
			FGatersDrainageNetwork::CompileFeatureCandidates(
				Drainage.Recipe, Environment.RegionalWater, Fit);

		TestTrue(FString::Printf(TEXT("seed %d drainage builds"), Seed),
			Drainage.IsValid());
		TestTrue(FString::Printf(TEXT("seed %d water fit validates"), Seed),
			Fit.IsValid());
		TestTrue(FString::Printf(TEXT("seed %d features compile"), Seed),
			A.IsValid());
		TestEqual(FString::Printf(TEXT("seed %d features repeat"), Seed), A, B);
		TestEqual(FString::Printf(TEXT("seed %d provenance is retained"), Seed),
			A.Recipe.Seed, Seed);
		TestEqual(FString::Printf(TEXT("seed %d drainage version is retained"), Seed),
			A.Recipe.DrainageVersion, Drainage.Recipe.Version);
		TestEqual(FString::Printf(TEXT("seed %d water version is retained"), Seed),
			A.Recipe.RegionalWaterVersion, Environment.RegionalWater.Version);
		TestEqual(FString::Printf(TEXT("seed %d fit version is retained"), Seed),
			A.Recipe.WaterFitVersion, Fit.Version);
		TotalCandidateCount += A.Recipe.Candidates.Num();

		TSet<FString> SurfaceIds;
		for (const FGatersRegionalWaterSurface& Surface :
			Environment.RegionalWater.Surfaces)
		{
			SurfaceIds.Add(Surface.Id);
		}
		TSet<FString> SegmentIds;
		for (const FGatersDrainageSegment& Segment : Drainage.Recipe.Segments)
		{
			SegmentIds.Add(Segment.Id);
		}
		TSet<FString> CandidateIds;
		for (const FGatersDrainageFeatureCandidate& Candidate : A.Recipe.Candidates)
		{
			TestFalse(FString::Printf(TEXT("seed %d candidate has identity"), Seed),
				Candidate.Id.IsEmpty());
			TestFalse(FString::Printf(TEXT("seed %d candidate identity is unique"), Seed),
				CandidateIds.Contains(Candidate.Id));
			CandidateIds.Add(Candidate.Id);
			if (!Candidate.SurfaceId.IsEmpty())
			{
				TestTrue(FString::Printf(TEXT("seed %d surface provenance exists"), Seed),
					SurfaceIds.Contains(Candidate.SurfaceId));
			}
			for (const int32 BasinIndex : Candidate.BasinIndices)
			{
				TestTrue(FString::Printf(TEXT("seed %d basin provenance exists"), Seed),
					BasinIndex >= 0 && BasinIndex < Drainage.Recipe.BasinCount);
			}
			for (const int32 CellIndex : Candidate.CellIndices)
			{
				TestTrue(FString::Printf(TEXT("seed %d cell provenance exists"), Seed),
					Drainage.Recipe.Cells.IsValidIndex(CellIndex));
			}
			for (const FString& SegmentId : Candidate.SegmentIds)
			{
				TestTrue(FString::Printf(TEXT("seed %d segment provenance exists"), Seed),
					SegmentIds.Contains(SegmentId));
			}
		}
	}
	TestTrue(TEXT("held-out roots produce some neutral feature evidence"),
		TotalCandidateCount > 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersDrainageNetworkCounterexampleTest,
	"Gaters.Worldgen.DrainageNetwork.Counterexamples",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersDrainageNetworkCounterexampleTest::RunTest(const FString& Parameters)
{
	const auto Flat = [](const FVector2D&) { return 0.f; };
	const auto Wet = [](const FVector2D&) { return 0.8f; };
	FGatersDrainageSettings Invalid = SmallSettings();
	Invalid.CellsPerAxis = 2;
	TestTrue(TEXT("undersized grid is causal"), HasIssue(
		FGatersDrainageNetwork::Build(7, 120000.f, Invalid, Flat, Wet),
		TEXT("drainage.settings")));
	Invalid = SmallSettings();
	Invalid.Extent = -1.f;
	TestTrue(TEXT("negative extent is causal"), HasIssue(
		FGatersDrainageNetwork::Build(7, 120000.f, Invalid, Flat, Wet),
		TEXT("drainage.settings")));
	Invalid = SmallSettings();
	Invalid.ChannelAccumulationThreshold = -1.f;
	TestTrue(TEXT("negative channel threshold is causal"), HasIssue(
		FGatersDrainageNetwork::Build(7, 120000.f, Invalid, Flat, Wet),
		TEXT("drainage.settings")));
	const auto BrokenHeight = [](const FVector2D& Point)
	{
		return Point.IsNearlyZero()
			? std::numeric_limits<float>::quiet_NaN()
			: 0.f;
	};
	TestTrue(TEXT("non-finite height evidence is causal"), HasIssue(
		FGatersDrainageNetwork::Build(
			7, 120000.f, SmallSettings(), BrokenHeight, Wet),
		TEXT("drainage.evidence")));
	const auto BrokenPrecipitation = [](const FVector2D& Point)
	{
		return Point.IsNearlyZero()
			? std::numeric_limits<float>::infinity()
			: 0.8f;
	};
	TestTrue(TEXT("non-finite precipitation evidence is causal"), HasIssue(
		FGatersDrainageNetwork::Build(
			7, 120000.f, SmallSettings(), Flat, BrokenPrecipitation),
		TEXT("drainage.evidence")));

	FGatersEnvironmentRecipe UnrelatedWaterFailure =
		FGatersEnvironmentRecipeCompiler::Compile(47, 120000.f);
	UnrelatedWaterFailure.RegionalWater.Version = 99;
	TestTrue(TEXT("unrelated regional-water metadata does not block drainage"),
		FGatersDrainageNetwork::Build(
			UnrelatedWaterFailure, SmallSettings()).IsValid());
	FGatersEnvironmentRecipe BrokenClimate =
		FGatersEnvironmentRecipeCompiler::Compile(47, 120000.f);
	BrokenClimate.Climate.Version = 99;
	TestTrue(TEXT("broken climate provenance is causal"), HasIssue(
		FGatersDrainageNetwork::Build(BrokenClimate, SmallSettings()),
		TEXT("drainage.environment")));
	return true;
}

#endif
