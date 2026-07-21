#if WITH_DEV_AUTOMATION_TESTS

#include "GatersEnvironmentBrief.h"
#include "Misc/AutomationTest.h"

#include <limits>

namespace
{
constexpr float WorldSize = 400000.f;

FGatersEnvironmentBrief MixedBrief()
{
	FGatersEnvironmentBrief Brief;
	Brief.Global.Relief = {0.65f, 0.9f};
	Brief.Global.Temperature = {0.1f, 0.3f};
	Brief.Global.Moisture = {0.55f, 0.8f};
	Brief.Global.Ice = {0.7f, 0.7f};
	Brief.LandAccess.WalkableLand = {0.2f, 0.4f};
	Brief.LandAccess.ConnectedLand = {0.6f, 0.9f};

	FGatersEnvironmentRegionBrief& Wetland = Brief.Regions.AddDefaulted_GetRef();
	Wetland.Id = TEXT("region:wetland");
	Wetland.CenterNormalized = FVector2D(0.25f, -0.5f);
	Wetland.RadiusFraction = 0.1f;
	Wetland.Profile.SurfaceWater = {0.8f, 1.f};
	Wetland.Profile.Moisture = {0.85f, 1.f};

	FGatersEnvironmentRegionBrief& Volcanic = Brief.Regions.AddDefaulted_GetRef();
	Volcanic.Id = TEXT("region:volcanic");
	Volcanic.CenterNormalized = FVector2D(-0.4f, 0.35f);
	Volcanic.RadiusFraction = 0.08f;
	Volcanic.Profile.Volcanism = {0.9f, 1.f};
	Volcanic.Profile.Ice = {0.f, 0.1f};
	return Brief;
}

const FGatersEnvironmentBriefIssue* FindIssue(
	const FGatersEnvironmentBriefCompileResult& Result,
	const TCHAR* RuleId)
{
	return Result.Issues.FindByPredicate(
		[RuleId](const FGatersEnvironmentBriefIssue& Issue)
		{
			return Issue.RuleId == RuleId;
		});
}

bool HasIssue(
	const FGatersEnvironmentBriefCompileResult& Result,
	const TCHAR* RuleId)
{
	return FindIssue(Result, RuleId) != nullptr;
}

bool IsBounded(const FGatersEnvironmentTargetProfile& Profile)
{
	for (const float Value : {
		Profile.Relief, Profile.Temperature, Profile.Moisture, Profile.SurfaceWater,
		Profile.Volcanism, Profile.Ice, Profile.Vegetation, Profile.ExposedRock})
	{
		if (!FMath::IsFinite(Value) || Value < 0.f || Value > 1.f)
		{
			return false;
		}
	}
	return true;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersEnvironmentBriefContractTest,
	"Gaters.Worldgen.EnvironmentBrief.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersEnvironmentBriefContractTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironmentBrief Brief = MixedBrief();
	const FGatersEnvironmentBriefCompileResult A =
		FGatersEnvironmentBriefCompiler::Compile(Brief, 7, WorldSize);
	const FGatersEnvironmentBriefCompileResult B =
		FGatersEnvironmentBriefCompiler::Compile(Brief, 7, WorldSize);
	const FGatersEnvironmentBriefCompileResult Different =
		FGatersEnvironmentBriefCompiler::Compile(Brief, 53, WorldSize);

	TestTrue(TEXT("valid mixed brief compiles"), A.IsValid());
	TestEqual(TEXT("valid brief has no diagnostics"), A.Issues.Num(), 0);
	TestEqual(TEXT("compiler contract is versioned"), A.Intent.CompilerVersion, 2);
	TestEqual(TEXT("brief provenance is retained"), A.Intent.BriefVersion, Brief.Version);
	TestEqual(TEXT("seed provenance is retained"), A.Intent.Seed, 7);
	TestEqual(TEXT("world bounds provenance is retained"), A.Intent.WorldSize, WorldSize);
	TestTrue(TEXT("same seed compiles exact targets"), A.Intent == B.Intent);
	TestEqual(TEXT("same seed preserves land-access target"),
		A.Intent.LandAccess, B.Intent.LandAccess);
	TestFalse(TEXT("changed seed varies allowed physical targets"),
		A.Intent.Global == Different.Intent.Global);
	TestTrue(TEXT("walkable target remains inside caller range"),
		A.Intent.LandAccess.WalkableLand >= Brief.LandAccess.WalkableLand.Min
		&& A.Intent.LandAccess.WalkableLand <= Brief.LandAccess.WalkableLand.Max);
	TestTrue(TEXT("connected target remains inside caller range"),
		A.Intent.LandAccess.ConnectedLand >= Brief.LandAccess.ConnectedLand.Min
		&& A.Intent.LandAccess.ConnectedLand <= Brief.LandAccess.ConnectedLand.Max);
	TestEqual(TEXT("fixed physical targets ignore seed changes"),
		A.Intent.Global.Ice, Different.Intent.Global.Ice);
	TestEqual(TEXT("mixed brief preserves both regions"), A.Intent.Regions.Num(), 2);
	TestEqual(TEXT("fixed global ice target stays exact"), A.Intent.Global.Ice, 0.7f);
	TestTrue(TEXT("relief remains inside caller range"),
		A.Intent.Global.Relief >= Brief.Global.Relief.Min
		&& A.Intent.Global.Relief <= Brief.Global.Relief.Max);
	TestTrue(TEXT("temperature remains inside caller range"),
		A.Intent.Global.Temperature >= Brief.Global.Temperature.Min
		&& A.Intent.Global.Temperature <= Brief.Global.Temperature.Max);

	const FGatersCompiledEnvironmentRegion& Wetland = A.Intent.Regions[0];
	TestEqual(TEXT("region identity is stable"), Wetland.Id, FString(TEXT("region:wetland")));
	TestEqual(TEXT("normalized center compiles into world coordinates"),
		Wetland.Center, FVector2D(50000.f, -100000.f));
	TestEqual(TEXT("radius fraction compiles into world units"), Wetland.Radius, 40000.f);
	TestTrue(TEXT("regional moisture remains requested"),
		Wetland.Profile.Moisture >= 0.85f && Wetland.Profile.Moisture <= 1.f);
	TestTrue(TEXT("regional water remains requested"),
		Wetland.Profile.SurfaceWater >= 0.8f && Wetland.Profile.SurfaceWater <= 1.f);
	const FGatersCompiledEnvironmentRegion& Volcanic = A.Intent.Regions[1];
	TestTrue(TEXT("regional volcanism remains requested"),
		Volcanic.Profile.Volcanism >= 0.9f && Volcanic.Profile.Volcanism <= 1.f);
	TestTrue(TEXT("regional low-ice request remains requested"),
		Volcanic.Profile.Ice >= 0.f && Volcanic.Profile.Ice <= 0.1f);
	TestTrue(TEXT("global physical signals are bounded"), IsBounded(A.Intent.Global));
	for (const FGatersCompiledEnvironmentRegion& Region : A.Intent.Regions)
	{
		TestTrue(TEXT("regional physical signals are bounded"), IsBounded(Region.Profile));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersEnvironmentBriefCounterexampleTest,
	"Gaters.Worldgen.EnvironmentBrief.Counterexamples",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersEnvironmentBriefCounterexampleTest::RunTest(const FString& Parameters)
{
	FGatersEnvironmentBrief Unsupported = MixedBrief();
	Unsupported.Version = 99;
	TestTrue(TEXT("unsupported brief version is causal"), HasIssue(
		FGatersEnvironmentBriefCompiler::Compile(Unsupported, 7, WorldSize),
		TEXT("brief.version")));

	FGatersEnvironmentBrief Reversed = MixedBrief();
	Reversed.Global.Relief = {0.9f, 0.2f};
	TestTrue(TEXT("reversed range is causal"), HasIssue(
		FGatersEnvironmentBriefCompiler::Compile(Reversed, 7, WorldSize),
		TEXT("brief.range")));

	FGatersEnvironmentBrief NonFinite = MixedBrief();
	NonFinite.Global.Moisture.Min = std::numeric_limits<float>::quiet_NaN();
	TestTrue(TEXT("non-finite range is causal"), HasIssue(
		FGatersEnvironmentBriefCompiler::Compile(NonFinite, 7, WorldSize),
		TEXT("brief.range")));

	FGatersEnvironmentBrief OutOfRange = MixedBrief();
	OutOfRange.Global.Temperature.Min = -0.1f;
	TestTrue(TEXT("out-of-range signal is causal"), HasIssue(
		FGatersEnvironmentBriefCompiler::Compile(OutOfRange, 7, WorldSize),
		TEXT("brief.range")));

	FGatersEnvironmentBrief InvalidAccess = MixedBrief();
	InvalidAccess.LandAccess.ConnectedLand = {0.9f, 0.4f};
	TestTrue(TEXT("reversed connected-land range is causal"), HasIssue(
		FGatersEnvironmentBriefCompiler::Compile(InvalidAccess, 7, WorldSize),
		TEXT("brief.range")));

	FGatersEnvironmentBrief Duplicate = MixedBrief();
	Duplicate.Regions[1].Id = Duplicate.Regions[0].Id;
	TestTrue(TEXT("duplicate region identity is causal"), HasIssue(
		FGatersEnvironmentBriefCompiler::Compile(Duplicate, 7, WorldSize),
		TEXT("brief.region.id")));

	FGatersEnvironmentBrief InvalidRadius = MixedBrief();
	InvalidRadius.Regions[0].RadiusFraction = 0.f;
	TestTrue(TEXT("invalid region radius is causal"), HasIssue(
		FGatersEnvironmentBriefCompiler::Compile(InvalidRadius, 7, WorldSize),
		TEXT("brief.region.geometry")));

	FGatersEnvironmentBrief OversizedRadius = MixedBrief();
	OversizedRadius.Regions[0].RadiusFraction = 0.75f;
	const FGatersEnvironmentBriefCompileResult OversizedResult =
		FGatersEnvironmentBriefCompiler::Compile(OversizedRadius, 7, WorldSize);
	const FGatersEnvironmentBriefIssue* RadiusIssue =
		FindIssue(OversizedResult, TEXT("brief.region.geometry"));
	TestNotNull(TEXT("oversized region radius is rejected"), RadiusIssue);
	if (RadiusIssue)
	{
		TestTrue(TEXT("radius diagnostic states the allowed interval"),
			RadiusIssue->Message.Contains(TEXT("0 < radius <= 0.5")));
	}

	FGatersEnvironmentBrief Escaped = MixedBrief();
	Escaped.Regions[0].CenterNormalized = FVector2D(0.95f, 0.f);
	Escaped.Regions[0].RadiusFraction = 0.1f;
	TestTrue(TEXT("region escaping world bounds is causal"), HasIssue(
		FGatersEnvironmentBriefCompiler::Compile(Escaped, 7, WorldSize),
		TEXT("brief.region.bounds")));

	TestTrue(TEXT("invalid world size is causal"), HasIssue(
		FGatersEnvironmentBriefCompiler::Compile(MixedBrief(), 7, 0.f),
		TEXT("brief.world_size")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersEnvironmentBriefLandformOverrideTest,
	"Gaters.Worldgen.EnvironmentBrief.LandformOverrides",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersEnvironmentBriefLandformOverrideTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironmentBrief Source = MixedBrief();
	const FGatersEnvironmentBrief Fixed = Source.WithGlobalLandformTargets(
		0.8f, 1.f, 0.f);
	const FGatersEnvironmentBriefCompileResult Compiled =
		FGatersEnvironmentBriefCompiler::Compile(Fixed, 83, WorldSize);

	TestTrue(TEXT("bounded fixed landform targets compile"), Compiled.IsValid());
	TestEqual(TEXT("relief is fixed"), Compiled.Intent.Global.Relief, 0.8f);
	TestEqual(TEXT("volcanism is fixed"), Compiled.Intent.Global.Volcanism, 1.f);
	TestEqual(TEXT("ice is fixed"), Compiled.Intent.Global.Ice, 0.f);

	const FGatersEnvironmentBrief Partial = Source.WithGlobalLandformTargets(
		-1.f, 0.4f, -1.f);
	TestEqual(TEXT("negative relief keeps source range"),
		Partial.Global.Relief, Source.Global.Relief);
	TestEqual(TEXT("negative ice keeps source range"),
		Partial.Global.Ice, Source.Global.Ice);
	TestEqual(TEXT("supplied volcanism becomes fixed"),
		Partial.Global.Volcanism, FGatersEnvironmentSignalRange({0.4f, 0.4f}));

	const FGatersEnvironmentBrief Invalid = Source.WithGlobalLandformTargets(
		1.1f, -1.f, -1.f);
	TestTrue(TEXT("out-of-range override remains causally rejected"), HasIssue(
		FGatersEnvironmentBriefCompiler::Compile(Invalid, 83, WorldSize),
		TEXT("brief.range")));
	return true;
}

#endif
