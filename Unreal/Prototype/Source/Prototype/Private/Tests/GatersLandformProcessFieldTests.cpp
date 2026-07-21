#if WITH_DEV_AUTOMATION_TESTS

#include "GatersEnvironment.h"
#include "GatersEnvironmentBrief.h"
#include "GatersLandformProcessField.h"
#include "GatersTerrainEvaluator.h"
#include "GatersWorldRecipe.h"
#include "Misc/AutomationTest.h"

namespace
{
constexpr float WorldSize = 400000.f;

FGatersCompiledEnvironmentBrief FixedIntent(
	int32 Seed,
	float Relief,
	float Volcanism,
	float Ice)
{
	FGatersEnvironmentBrief Brief;
	Brief.Global.Relief = {Relief, Relief};
	Brief.Global.Volcanism = {Volcanism, Volcanism};
	Brief.Global.Ice = {Ice, Ice};
	const FGatersEnvironmentBriefCompileResult Result =
		FGatersEnvironmentBriefCompiler::Compile(Brief, Seed, WorldSize);
	return Result.Intent;
}

bool HasIssue(
	const FGatersLandformProcessCompileResult& Result,
	const TCHAR* RuleId)
{
	return Result.Issues.ContainsByPredicate(
		[RuleId](const FGatersLandformProcessIssue& Issue)
		{
			return Issue.RuleId == RuleId;
		});
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersLandformProcessContractTest,
	"Gaters.Worldgen.LandformProcesses.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersLandformProcessContractTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Environment =
		FGatersEnvironment::FromSeed(7, WorldSize);
	const FGatersLandformProcessCompileResult High =
		FGatersLandformProcessField::Compile(
			Environment, FixedIntent(7, 1.f, 1.f, 1.f));
	const FGatersLandformProcessCompileResult Repeat =
		FGatersLandformProcessField::Compile(
			Environment, FixedIntent(7, 1.f, 1.f, 1.f));

	TestTrue(TEXT("compatible intent compiles"), High.IsValid());
	TestEqual(TEXT("field recipe is versioned"), High.Recipe.Version, 6);
	TestTrue(TEXT("same inputs compile exactly"), High.Recipe == Repeat.Recipe);
	TestEqual(TEXT("recipe records environment water presence"),
		High.Recipe.bHasWater, Environment.HasWater());
	TestEqual(TEXT("recipe records environment water datum"),
		High.Recipe.WaterHeight, Environment.WaterHeight);
	FGatersEnvironmentBrief RegionalBrief;
	RegionalBrief.Global.Temperature = {0.2f, 0.2f};
	RegionalBrief.Global.Moisture = {0.3f, 0.3f};
	FGatersEnvironmentRegionBrief ClimateRegion;
	ClimateRegion.Id = TEXT("climate-test");
	ClimateRegion.CenterNormalized = FVector2D(0.15f, -0.1f);
	ClimateRegion.RadiusFraction = 0.1f;
	ClimateRegion.Profile.Temperature = {0.8f, 0.8f};
	ClimateRegion.Profile.Moisture = {0.9f, 0.9f};
	RegionalBrief.Regions.Add(ClimateRegion);
	const FGatersCompiledEnvironmentBrief RegionalIntent =
		FGatersEnvironmentBriefCompiler::Compile(
			RegionalBrief, Environment.Seed, WorldSize).Intent;
	const FGatersLandformProcessRecipe RegionalRecipe =
		FGatersLandformProcessField::Compile(
			Environment, RegionalIntent).Recipe;
	float CoreInfluence = 0.f;
	const FGatersEnvironmentTargetProfile CoreProfile =
		FGatersLandformProcessField::QueryProfile(
			RegionalRecipe, RegionalIntent.Regions[0].Center, &CoreInfluence);
	float EdgeInfluence = 0.f;
	const FGatersEnvironmentTargetProfile EdgeProfile =
		FGatersLandformProcessField::QueryProfile(
			RegionalRecipe,
			RegionalIntent.Regions[0].Center
				+ FVector2D(RegionalIntent.Regions[0].Radius * 0.85f, 0.f),
			&EdgeInfluence);
	const FGatersEnvironmentTargetProfile GlobalProfile =
		FGatersLandformProcessField::QueryProfile(
			RegionalRecipe, FVector2D(-WorldSize * 0.45f, WorldSize * 0.45f));
	TestEqual(TEXT("public profile query returns exact regional core"),
		CoreProfile.Temperature, RegionalIntent.Regions[0].Profile.Temperature);
	TestEqual(TEXT("public profile query returns exact global profile outside regions"),
		GlobalProfile.Temperature, RegionalIntent.Global.Temperature);
	TestTrue(TEXT("public profile query reports continuous boundary influence"),
		CoreInfluence == 1.f && EdgeInfluence > 0.f && EdgeInfluence < 1.f
			&& EdgeProfile.Temperature > GlobalProfile.Temperature
			&& EdgeProfile.Temperature < CoreProfile.Temperature);
	TestTrue(TEXT("public profile query repeats exactly"),
		CoreProfile == FGatersLandformProcessField::QueryProfile(
			RegionalRecipe, RegionalIntent.Regions[0].Center));
	const FGatersLandformProcessCompileResult CandidateZero =
		FGatersLandformProcessField::Compile(
			Environment, FixedIntent(7, 1.f, 1.f, 1.f), {}, 0);
	const FGatersLandformProcessCompileResult CandidateOne =
		FGatersLandformProcessField::Compile(
			Environment, FixedIntent(7, 1.f, 1.f, 1.f), {}, 1);
	const FGatersLandformProcessCompileResult CandidateSeven =
		FGatersLandformProcessField::Compile(
			Environment, FixedIntent(7, 1.f, 1.f, 1.f), {}, 7);
	TestTrue(TEXT("explicit candidate zero preserves the current recipe"),
		CandidateZero.Recipe == High.Recipe);
	TestEqual(TEXT("candidate zero preserves current feature scale"),
		CandidateZero.Recipe.FeatureScale, 1.f);
	TestEqual(TEXT("candidate zero adds no topology dissection"),
		CandidateZero.Recipe.DissectionScale, 0.f);
	TestEqual(TEXT("candidate zero adds no target-derived ruggedness"),
		CandidateZero.Recipe.RuggednessScale, 0.f);
	TestEqual(TEXT("variant records its candidate index"),
		CandidateOne.Recipe.CandidateIndex, 1);
	TestTrue(TEXT("variant derives bounded ruggedness from walkable intent"),
		CandidateOne.Recipe.RuggednessScale > 0.f
			&& CandidateOne.Recipe.RuggednessScale <= 1.6f);
	TestEqual(TEXT("proven final slot retains its pre-ruggedness candidate"),
		CandidateSeven.Recipe.RuggednessScale, 0.f);
	FGatersCompiledEnvironmentBrief ScarceIntent = FixedIntent(7, 1.f, 1.f, 1.f);
	ScarceIntent.LandAccess.WalkableLand = 0.2f;
	FGatersCompiledEnvironmentBrief OpenIntent = ScarceIntent;
	OpenIntent.LandAccess.WalkableLand = 0.9f;
	const FGatersLandformProcessRecipe ScarceCandidate =
		FGatersLandformProcessField::Compile(
			Environment, ScarceIntent, {}, 1).Recipe;
	const FGatersLandformProcessRecipe OpenCandidate =
		FGatersLandformProcessField::Compile(
			Environment, OpenIntent, {}, 1).Recipe;
	TestTrue(TEXT("lower walkable target derives stronger ruggedness"),
		ScarceCandidate.RuggednessScale > OpenCandidate.RuggednessScale);
	float MinFeatureScale = CandidateZero.Recipe.FeatureScale;
	float MaxFeatureScale = CandidateZero.Recipe.FeatureScale;
	float MaxDissectionScale = CandidateZero.Recipe.DissectionScale;
	for (int32 CandidateIndex = 1; CandidateIndex < 8; ++CandidateIndex)
	{
		const FGatersLandformProcessRecipe Candidate =
			FGatersLandformProcessField::Compile(
				Environment, FixedIntent(7, 1.f, 1.f, 1.f), {}, CandidateIndex).Recipe;
		TestTrue(TEXT("candidate feature scale is finite and positive"),
			FMath::IsFinite(Candidate.FeatureScale) && Candidate.FeatureScale > 0.f);
		MinFeatureScale = FMath::Min(MinFeatureScale, Candidate.FeatureScale);
		MaxFeatureScale = FMath::Max(MaxFeatureScale, Candidate.FeatureScale);
		MaxDissectionScale = FMath::Max(
			MaxDissectionScale, Candidate.DissectionScale);
	}
	TestTrue(TEXT("bounded candidates cover narrower features"), MinFeatureScale < 0.7f);
	TestTrue(TEXT("bounded candidates cover broader features"), MaxFeatureScale > 1.5f);
	TestTrue(TEXT("bounded candidates cover strong terrain dissection"),
		MaxDissectionScale > 1.5f);
	bool bVariantChangesHeight = false;
	for (const FVector2D VariantPoint : {
		FVector2D(-30000.f, -15000.f), FVector2D(-10000.f, 20000.f),
		FVector2D(5000.f, -25000.f), FVector2D(22000.f, 18000.f)})
	{
		const float VariantBase = Environment.HeightAt(VariantPoint);
		bVariantChangesHeight |= !FMath::IsNearlyEqual(
			FGatersLandformProcessField::Query(
				CandidateZero.Recipe, VariantPoint, VariantBase).Height,
			FGatersLandformProcessField::Query(
				CandidateOne.Recipe, VariantPoint, VariantBase).Height,
			0.01f);
	}
	TestTrue(TEXT("changed candidate index changes process placement"),
		bVariantChangesHeight);
	TestTrue(TEXT("negative candidate index is causal"), HasIssue(
		FGatersLandformProcessField::Compile(
			Environment, FixedIntent(7, 1.f, 1.f, 1.f), {}, -1),
		TEXT("landform.candidate")));

	const FVector2D Point(17000.f, -9000.f);
	const float BaseHeight = Environment.HeightAt(Point);
	const FGatersLandformProcessSample A =
		FGatersLandformProcessField::Query(High.Recipe, Point, BaseHeight);
	const FGatersLandformProcessSample B =
		FGatersLandformProcessField::Query(High.Recipe, Point, BaseHeight);
	TestTrue(TEXT("same coordinate queries exactly"), A == B);
	TestEqual(TEXT("query retains base height"), A.BaseHeight, BaseHeight);
	TestTrue(TEXT("contributions account for final height"), FMath::IsNearlyEqual(
		A.Height,
		A.BaseHeight + A.ReliefContribution + A.UpliftContribution
			+ A.VolcanicContribution + A.GlacialContribution
			+ A.DissectionContribution + A.RuggednessContribution,
		0.01f));
	TestTrue(TEXT("reported regional influence is bounded"),
		A.RegionInfluence >= 0.f && A.RegionInfluence <= 1.f);
	const FGatersEnvironment DryEnvironment = Environment.WithProfile(
		EGatersEnvironment::Lowlands, EGatersHydrology::Dry);
	const FGatersEnvironment Processed =
		DryEnvironment.WithLandformProcesses(High.Recipe);
	const float DryBase = DryEnvironment.HeightAt(Point);
	TestEqual(TEXT("attached field drives environment height queries"),
		Processed.HeightAt(Point),
		FGatersLandformProcessField::Query(High.Recipe, Point, DryBase).Height);
	TestEqual(TEXT("attachment does not mutate champion environment"),
		DryEnvironment.HeightAt(Point), DryBase);
	const FVector2D RegionCenter(50000.f, -25000.f);
	const FVector2D LocalPoint(3000.f, 2000.f);
	const FGatersEnvironment BareRegional = DryEnvironment.WithProfile(
		EGatersEnvironment::Mountains, EGatersHydrology::Dry);
	const FGatersEnvironment ProcessedRegional = Processed.WithProfile(
		EGatersEnvironment::Mountains, EGatersHydrology::Dry, RegionCenter);
	TestEqual(TEXT("regional shape keeps global process coordinates"),
		ProcessedRegional.HeightAt(LocalPoint),
		FGatersLandformProcessField::Query(
			High.Recipe,
			LocalPoint + RegionCenter,
			BareRegional.HeightAt(LocalPoint)).Height);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersLandformProcessControlTest,
	"Gaters.Worldgen.LandformProcesses.Control",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersLandformProcessControlTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Environment =
		FGatersEnvironment::FromSeed(7, WorldSize).WithProfile(
			EGatersEnvironment::Lowlands, EGatersHydrology::Dry);
	const FGatersLandformProcessRecipe Low =
		FGatersLandformProcessField::Compile(
			Environment, FixedIntent(7, 0.f, 0.f, 0.f)).Recipe;
	const FGatersLandformProcessRecipe Relief =
		FGatersLandformProcessField::Compile(
			Environment, FixedIntent(7, 1.f, 0.f, 0.f)).Recipe;
	const FGatersLandformProcessRecipe Volcanic =
		FGatersLandformProcessField::Compile(
			Environment, FixedIntent(7, 0.f, 1.f, 0.f)).Recipe;
	const FGatersLandformProcessRecipe Glacial =
		FGatersLandformProcessField::Compile(
			Environment, FixedIntent(7, 1.f, 0.f, 1.f)).Recipe;

	float LowRelief = 0.f;
	float HighRelief = 0.f;
	float MaxVolcanic = 0.f;
	float MinGlacial = 0.f;
	for (int32 X = -8; X <= 8; ++X)
	{
		for (int32 Y = -8; Y <= 8; ++Y)
		{
			const FVector2D Point(X * 5000.f, Y * 5000.f);
			const float Base = Environment.HeightAt(Point);
			LowRelief = FMath::Max(LowRelief, FMath::Abs(
				FGatersLandformProcessField::Query(Low, Point, Base).Height));
			HighRelief = FMath::Max(HighRelief, FMath::Abs(
				FGatersLandformProcessField::Query(Relief, Point, Base).Height));
			MaxVolcanic = FMath::Max(MaxVolcanic,
				FGatersLandformProcessField::Query(
					Volcanic, Point, Base).VolcanicContribution);
			MinGlacial = FMath::Min(MinGlacial,
				FGatersLandformProcessField::Query(
					Glacial, Point, Base).GlacialContribution);
		}
	}
	TestTrue(TEXT("high relief increases sampled amplitude"), HighRelief > LowRelief);
	TestTrue(TEXT("volcanism creates positive local mass"), MaxVolcanic > 500.f);
	TestTrue(TEXT("ice carves a glacial valley"), MinGlacial < -300.f);
	const FGatersLandformProcessRecipe Dissected =
		FGatersLandformProcessField::Compile(
			Environment, FixedIntent(7, 0.f, 0.f, 0.f), {}, 7).Recipe;
	float MinDissection = 0.f;
	for (int32 X = -8; X <= 8; ++X)
	{
		for (int32 Y = -8; Y <= 8; ++Y)
		{
			const FVector2D Point(X * 5000.f, Y * 5000.f);
			MinDissection = FMath::Min(MinDissection,
				FGatersLandformProcessField::Query(
					Dissected, Point, Environment.HeightAt(Point)).DissectionContribution);
		}
	}
	TestTrue(TEXT("challenger candidate carves a generic terrain channel"),
		MinDissection < -500.f);

	const FGatersTerrainEvaluation LowMetrics =
		FGatersTerrainEvaluator::EvaluateHeightField(
			Environment,
			[&](const FVector2D& Point)
			{
				const float Base = Environment.HeightAt(Point);
				return FGatersLandformProcessField::Query(Low, Point, Base).Height;
			},
			80000.f);
	const FGatersTerrainEvaluation ReliefMetrics =
		FGatersTerrainEvaluator::EvaluateHeightField(
			Environment,
			[&](const FVector2D& Point)
			{
				const float Base = Environment.HeightAt(Point);
				return FGatersLandformProcessField::Query(Relief, Point, Base).Height;
			},
			80000.f);
	TestTrue(TEXT("independent metric observes increased relief"),
		ReliefMetrics.Relief() > LowMetrics.Relief());
	TestTrue(TEXT("challenger metric remains finite"),
		FMath::IsFinite(ReliefMetrics.MeanHeight)
		&& FMath::IsFinite(ReliefMetrics.MaxNeighborStep));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersLandformProcessBoundaryTest,
	"Gaters.Worldgen.LandformProcesses.Boundaries",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersLandformProcessBoundaryTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Environment =
		FGatersEnvironment::FromSeed(7, WorldSize);
	FGatersCompiledEnvironmentBrief Intent = FixedIntent(7, 0.f, 0.f, 0.f);
	FGatersCompiledEnvironmentRegion& Region = Intent.Regions.AddDefaulted_GetRef();
	Region.Id = TEXT("region:uplift");
	Region.Center = FVector2D(50000.f, 0.f);
	Region.Radius = 30000.f;
	Region.Profile = Intent.Global;
	Region.Profile.Relief = 1.f;
	Region.Profile.Volcanism = 1.f;

	const FGatersLandformProcessCompileResult Compiled =
		FGatersLandformProcessField::Compile(Environment, Intent);
	TestTrue(TEXT("regional process intent compiles"), Compiled.IsValid());
	const FGatersLandformProcessSample Core =
		FGatersLandformProcessField::Query(
			Compiled.Recipe, Region.Center, Environment.HeightAt(Region.Center));
	const FVector2D OutsidePoint = Region.Center + FVector2D(Region.Radius * 1.1f, 0.f);
	const FGatersLandformProcessSample Outside =
		FGatersLandformProcessField::Query(
			Compiled.Recipe, OutsidePoint, Environment.HeightAt(OutsidePoint));
	TestTrue(TEXT("region reaches full influence at core"), Core.RegionInfluence > 0.99f);
	TestTrue(TEXT("region has no influence outside radius"), Outside.RegionInfluence == 0.f);
	const auto InfluenceAt = [&](float RadiusFraction)
	{
		const FVector2D SamplePoint = Region.Center
			+ FVector2D(Region.Radius * RadiusFraction, 0.f);
		return FGatersLandformProcessField::Query(
			Compiled.Recipe, SamplePoint, Environment.HeightAt(SamplePoint)).RegionInfluence;
	};
	const float TransitionStart = InfluenceAt(0.65f);
	const float TransitionMiddle = InfluenceAt(0.82f);
	const float TransitionEdge = InfluenceAt(0.99f);
	const float JustOutside = InfluenceAt(1.01f);
	TestTrue(TEXT("regional transition decreases through its blend band"),
		TransitionStart > TransitionMiddle && TransitionMiddle > TransitionEdge
		&& TransitionEdge > JustOutside);
	TestTrue(TEXT("regional transition approaches zero continuously at edge"),
		TransitionEdge - JustOutside < 0.01f);

	FGatersCompiledEnvironmentBrief WrongSeed = Intent;
	WrongSeed.Seed = 53;
	TestTrue(TEXT("seed mismatch is causal"), HasIssue(
		FGatersLandformProcessField::Compile(Environment, WrongSeed),
		TEXT("landform.seed")));
	FGatersCompiledEnvironmentBrief WrongSize = Intent;
	WrongSize.WorldSize *= 0.5f;
	TestTrue(TEXT("world-size mismatch is causal"), HasIssue(
		FGatersLandformProcessField::Compile(Environment, WrongSize),
		TEXT("landform.world_size")));
	FGatersCompiledEnvironmentBrief InvalidProfile = Intent;
	InvalidProfile.Global.Relief = 1.2f;
	TestTrue(TEXT("forged invalid profile is causal"), HasIssue(
		FGatersLandformProcessField::Compile(Environment, InvalidProfile),
		TEXT("landform.profile")));
	FGatersCompiledEnvironmentBrief InvalidRegion = Intent;
	InvalidRegion.Regions[0].Radius = 0.f;
	TestTrue(TEXT("forged invalid region is causal"), HasIssue(
		FGatersLandformProcessField::Compile(Environment, InvalidRegion),
		TEXT("landform.region.geometry")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersLandformProtectedRegionTest,
	"Gaters.Worldgen.LandformProcesses.ProtectedRegions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersLandformProtectedRegionTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(131, WorldSize);
	FVector2D ChampionSite;
	TestTrue(TEXT("champion exposes a build candidate"), Environment.FindBaseSite(
		6000.f, 10800.f, 900.f, 350.f, ChampionSite));
	const TArray<FGatersLandformProtectedRegion> Protected = {
		{TEXT("arrival"), FVector2D::ZeroVector, 6000.f, 9000.f},
		{TEXT("build:champion"), ChampionSite, 900.f, 1800.f}};
	const FGatersLandformProcessCompileResult Compiled =
		FGatersLandformProcessField::Compile(
			Environment, FixedIntent(131, 0.8f, 0.f, 1.f), Protected);
	const FGatersLandformProcessCompileResult Repeat =
		FGatersLandformProcessField::Compile(
			Environment, FixedIntent(131, 0.8f, 0.f, 1.f), Protected);
	TestTrue(TEXT("protected field compiles"), Compiled.IsValid());
	TestEqual(TEXT("protected recipe is version 6"), Compiled.Recipe.Version, 6);
	TestTrue(TEXT("protected compilation is deterministic"), Compiled.Recipe == Repeat.Recipe);

	const float BaseAtArrival = Environment.HeightAt(FVector2D::ZeroVector);
	const FGatersLandformProcessSample Inner = FGatersLandformProcessField::Query(
		Compiled.Recipe, FVector2D::ZeroVector, BaseAtArrival);
	TestEqual(TEXT("inner region preserves exact base height"), Inner.Height, BaseAtArrival);
	TestEqual(TEXT("inner region reports zero process influence"), Inner.ProcessInfluence, 0.f);
	const FVector2D TransitionPoint(7500.f, 0.f);
	const FGatersLandformProcessSample Transition = FGatersLandformProcessField::Query(
		Compiled.Recipe, TransitionPoint, Environment.HeightAt(TransitionPoint));
	TestTrue(TEXT("transition has partial process influence"),
		Transition.ProcessInfluence > 0.f && Transition.ProcessInfluence < 1.f);
	const FVector2D OutsidePoint(12000.f, 0.f);
	const FGatersLandformProcessSample Outside = FGatersLandformProcessField::Query(
		Compiled.Recipe, OutsidePoint, Environment.HeightAt(OutsidePoint));
	TestEqual(TEXT("outside receives full process influence"), Outside.ProcessInfluence, 1.f);

	const FGatersEnvironment Challenger = Environment.WithLandformProcesses(Compiled.Recipe);
	const FGatersWorldRecipe Recipe = FGatersWorldRecipe::Generate(
		Challenger, 6000.f, 10800.f, 900.f, 350.f);
	TestTrue(TEXT("protected challenger rediscovers a valid build site"), Recipe.bHasBaseSite);

	TArray<FGatersLandformProtectedRegion> Duplicate = Protected;
	Duplicate[1].Id = Duplicate[0].Id;
	TestTrue(TEXT("duplicate protected ID is causal"), HasIssue(
		FGatersLandformProcessField::Compile(
			Environment, FixedIntent(131, 0.8f, 0.f, 1.f), Duplicate),
		TEXT("landform.protected.id")));
	TArray<FGatersLandformProtectedRegion> Invalid = Protected;
	Invalid[0].OuterRadius = Invalid[0].InnerRadius - 1.f;
	TestTrue(TEXT("invalid protected geometry is causal"), HasIssue(
		FGatersLandformProcessField::Compile(
			Environment, FixedIntent(131, 0.8f, 0.f, 1.f), Invalid),
		TEXT("landform.protected.geometry")));
	return true;
}

#endif
