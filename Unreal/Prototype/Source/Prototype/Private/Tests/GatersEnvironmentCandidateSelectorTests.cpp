#if WITH_DEV_AUTOMATION_TESTS

#include "GatersEnvironmentCandidateSelector.h"
#include "GatersTerrainSemanticField.h"
#include "GatersTraversabilityEvaluator.h"
#include "Misc/AutomationTest.h"

namespace
{
constexpr float WorldSize = 400000.f;

FGatersEnvironmentBrief VolcanicBrief()
{
	FGatersEnvironmentBrief Brief;
	Brief.Global.Relief = {0.45f, 0.45f};
	Brief.Global.Volcanism = {1.f, 1.f};
	Brief.Global.Ice = {0.f, 0.f};
	return Brief;
}

FGatersEnvironmentCandidateSelectionSettings TestSettings()
{
	FGatersEnvironmentCandidateSelectionSettings Settings;
	Settings.CandidateCount = 4;
	Settings.WorldCellsPerAxis = 61;
	Settings.ArrivalCellsPerAxis = 61;
	Settings.ArrivalCellSize = 500.f;
	Settings.PadRadius = 1000.f;
	Settings.FlatNormalZ = 0.94f;
	Settings.SlopeNormalZ = 0.77f;
	Settings.EscapeDistanceCells = 3;
	Settings.WalkableTolerance = 0.0001f;
	Settings.ConnectedTolerance = 0.0001f;
	return Settings;
}

FGatersTraversabilityEvaluation CandidateZeroEvaluation(
	const FGatersEnvironment& Environment,
	const FGatersCompiledEnvironmentBrief& Intent,
	const TArray<FGatersLandformProtectedRegion>& Protected,
	const FGatersEnvironmentCandidateSelectionSettings& Settings)
{
	const FGatersLandformProcessRecipe Recipe =
		FGatersLandformProcessField::Compile(Environment, Intent, Protected, 0).Recipe;
	const FGatersEnvironment Candidate = Environment.WithLandformProcesses(Recipe);
	const float WorldCellSize = Intent.WorldSize
		/ static_cast<float>(Settings.WorldCellsPerAxis - 1);
	const FGatersTerrainSemanticField WorldField = FGatersTerrainSemanticField::Build(
		Candidate, Settings.WorldCellsPerAxis, WorldCellSize, Settings.PadRadius,
		Settings.FlatNormalZ, Settings.SlopeNormalZ);
	const int32 WorldHalf = Settings.WorldCellsPerAxis / 2;
	FGatersTraversabilityEvaluation Result = FGatersTraversabilityEvaluator::Evaluate(
		WorldField, FIntPoint(WorldHalf, WorldHalf),
		FIntPoint(WorldHalf, WorldHalf), 1);
	const FGatersTerrainSemanticField ArrivalField = FGatersTerrainSemanticField::Build(
		Candidate, Settings.ArrivalCellsPerAxis, Settings.ArrivalCellSize,
		Settings.PadRadius, Settings.FlatNormalZ, Settings.SlopeNormalZ);
	const int32 ArrivalHalf = Settings.ArrivalCellsPerAxis / 2;
	const FGatersTraversabilityEvaluation Arrival =
		FGatersTraversabilityEvaluator::Evaluate(
			ArrivalField, FIntPoint(ArrivalHalf, ArrivalHalf),
			FIntPoint(ArrivalHalf, ArrivalHalf),
		Settings.EscapeDistanceCells);
	Result.bEscapesStart = Arrival.bEscapesStart;
	return Result;
}

bool HasIssue(
	const FGatersEnvironmentCandidateSelectionResult& Result,
	const TCHAR* RuleId)
{
	return Result.Issues.ContainsByPredicate(
		[RuleId](const FGatersEnvironmentCandidateSelectionIssue& Issue)
		{
			return Issue.RuleId == RuleId;
		});
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersEnvironmentCandidateSelectorContractTest,
	"Gaters.Worldgen.EnvironmentCandidateSelector.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersEnvironmentCandidateSelectorContractTest::RunTest(const FString& Parameters)
{
	const int32 Seed = 83;
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(Seed, WorldSize)
		.WithProfile(EGatersEnvironment::Lowlands, EGatersHydrology::Dry);
	const TArray<FGatersLandformProtectedRegion> Protected = {
		{TEXT("arrival"), FVector2D::ZeroVector, 1000.f, 2000.f}};
	FGatersEnvironmentCandidateSelectionSettings Settings = TestSettings();
	FGatersEnvironmentBrief Brief = VolcanicBrief();
	FGatersCompiledEnvironmentBrief Intent =
		FGatersEnvironmentBriefCompiler::Compile(Brief, Seed, WorldSize).Intent;
	const FGatersTraversabilityEvaluation CandidateZero =
		CandidateZeroEvaluation(Environment, Intent, Protected, Settings);
	Brief.LandAccess.WalkableLand = {
		CandidateZero.WalkableFraction, CandidateZero.WalkableFraction};
	Brief.LandAccess.ConnectedLand = {
		CandidateZero.ReachableFraction, CandidateZero.ReachableFraction};
	Intent = FGatersEnvironmentBriefCompiler::Compile(Brief, Seed, WorldSize).Intent;

	const FGatersEnvironmentCandidateSelectionResult A =
		FGatersEnvironmentCandidateSelector::Select(
			Environment, Intent, Protected, Settings);
	const FGatersEnvironmentCandidateSelectionResult B =
		FGatersEnvironmentCandidateSelector::Select(
			Environment, Intent, Protected, Settings);

	TestTrue(TEXT("same input reproduces exact selection"), A == B);
	TestEqual(TEXT("selection result is versioned"), A.Version, 4);
	TestTrue(TEXT("candidate zero target produces a selection"), A.bSelected);
	TestEqual(TEXT("exact target selects candidate zero"),
		A.Selected.CandidateIndex, 0);
	TestTrue(TEXT("selected candidate escapes Arrival"),
		A.Selected.bEscapesArrival);
	TestTrue(TEXT("selected candidate reaches beyond its coarse world cell"),
		A.Selected.bHasWorldAccess);
	TestTrue(TEXT("selected candidate meets walkable target"),
		A.Selected.WalkableError <= Settings.WalkableTolerance);
	TestTrue(TEXT("selected candidate meets connected target"),
		A.Selected.ConnectedError <= Settings.ConnectedTolerance);
	TestEqual(TEXT("selector records every bounded attempt"),
		A.Candidates.Num(), Settings.CandidateCount);
	TestEqual(TEXT("best recipe matches best evidence"),
		A.BestRecipe.CandidateIndex, A.Best.CandidateIndex);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersEnvironmentCandidateSelectorWorldAccessTest,
	"Gaters.Worldgen.EnvironmentCandidateSelector.WorldAccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersEnvironmentCandidateSelectorWorldAccessTest::RunTest(
	const FString& Parameters)
{
	const int32 Seed = 47;
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(Seed, WorldSize);
	const TArray<FGatersLandformProtectedRegion> Protected = {
		{TEXT("arrival"), FVector2D::ZeroVector, 1000.f, 2000.f}};
	FGatersEnvironmentCandidateSelectionSettings Settings = TestSettings();
	Settings.CandidateCount = 8;
	Settings.WalkableTolerance = 0.15f;
	Settings.ConnectedTolerance = 0.15f;
	const FGatersEnvironmentBrief ProbeBrief = FGatersEnvironmentBrief()
		.WithGlobalLandformTargets(0.f, 0.f, 0.f);
	const FGatersCompiledEnvironmentBrief ProbeIntent =
		FGatersEnvironmentBriefCompiler::Compile(
			ProbeBrief, Seed, WorldSize).Intent;
	const FGatersEnvironmentCandidateSelectionResult Probe =
		FGatersEnvironmentCandidateSelector::Select(
			Environment, ProbeIntent, Protected, Settings);

	const FGatersEnvironmentCandidateEvidence* Deceptive =
		Probe.Candidates.FindByPredicate(
			[](const FGatersEnvironmentCandidateEvidence& Candidate)
			{
				return !Candidate.bHasWorldAccess
					&& Candidate.bEscapesArrival;
			});
	TestNotNull(TEXT("held-out seed exposes a no-world-access candidate"),
		Deceptive);
	if (!Deceptive)
	{
		return false;
	}

	FGatersEnvironmentBrief TargetedBrief = ProbeBrief;
	TargetedBrief.LandAccess.WalkableLand = {
		Deceptive->WalkableLand, Deceptive->WalkableLand};
	TargetedBrief.LandAccess.ConnectedLand = {
		Deceptive->ConnectedLand, Deceptive->ConnectedLand};
	const FGatersCompiledEnvironmentBrief TargetedIntent =
		FGatersEnvironmentBriefCompiler::Compile(
			TargetedBrief, Seed, WorldSize).Intent;
	const FGatersEnvironmentCandidateSelectionResult Result =
		FGatersEnvironmentCandidateSelector::Select(
			Environment, TargetedIntent, Protected, Settings);
	const FGatersEnvironmentCandidateEvidence* TargetedEvidence =
		Result.Candidates.FindByPredicate(
			[Deceptive](const FGatersEnvironmentCandidateEvidence& Candidate)
			{
				return Candidate.CandidateIndex == Deceptive->CandidateIndex;
			});
	TestNotNull(TEXT("retargeted candidate remains reproducible"), TargetedEvidence);
	if (!TargetedEvidence)
	{
		return false;
	}
	TestTrue(TEXT("retargeted candidate satisfies every numeric gate"),
		TargetedEvidence->WalkableError <= Settings.WalkableTolerance
			&& TargetedEvidence->ConnectedError <= Settings.ConnectedTolerance);
	TestTrue(TEXT("retargeted candidate escapes Arrival"),
		TargetedEvidence->bEscapesArrival);
	TestFalse(TEXT("retargeted candidate lacks coarse world access"),
		TargetedEvidence->bHasWorldAccess);
	TestTrue(TEXT("selected candidate has coarse world access"),
		!Result.bSelected || Result.Selected.bHasWorldAccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersEnvironmentCandidateSelectorTargetFidelityTest,
	"Gaters.Worldgen.EnvironmentCandidateSelector.TargetFidelity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersEnvironmentCandidateSelectorTargetFidelityTest::RunTest(
	const FString& Parameters)
{
	const int32 Seed = 47;
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(Seed, WorldSize);
	const TArray<FGatersLandformProtectedRegion> Protected = {
		{TEXT("arrival"), FVector2D::ZeroVector, 1000.f, 2000.f}};
	FGatersEnvironmentCandidateSelectionSettings Settings = TestSettings();
	Settings.CandidateCount = 8;
	Settings.WalkableTolerance = 0.15f;
	Settings.ConnectedTolerance = 0.15f;
	const FGatersEnvironmentBrief Brief = FGatersEnvironmentBrief()
		.WithGlobalLandformTargets(0.8f, 0.f, 1.f);
	const FGatersCompiledEnvironmentBrief Intent =
		FGatersEnvironmentBriefCompiler::Compile(Brief, Seed, WorldSize).Intent;
	const FGatersEnvironmentCandidateSelectionResult Result =
		FGatersEnvironmentCandidateSelector::Select(
			Environment, Intent, Protected, Settings);
	const float WalkableFloor = Intent.LandAccess.WalkableLand
		* (1.f - Settings.WalkableTolerance);
	const float ConnectedFloor = Intent.LandAccess.ConnectedLand
		* (1.f - Settings.ConnectedTolerance);

	const FGatersEnvironmentCandidateEvidence* Collapsed =
		Result.Candidates.FindByPredicate(
			[&Intent, &Settings, WalkableFloor, ConnectedFloor](
				const FGatersEnvironmentCandidateEvidence& Candidate)
			{
				return Candidate.bHasWorldAccess
					&& Candidate.bEscapesArrival
					&& Candidate.WalkableError <= Settings.WalkableTolerance
					&& Candidate.ConnectedError <= Settings.ConnectedTolerance
					&& Candidate.WalkableLand >= WalkableFloor
					&& Candidate.ConnectedLand < ConnectedFloor
					&& Intent.LandAccess.ConnectedLand > 0.f;
			});
	TestNotNull(TEXT("held-out seed exposes a target-collapse false positive"),
		Collapsed);
	TestTrue(TEXT("selected candidate preserves declared target proportions"),
		!Result.bSelected
			|| (Result.Selected.WalkableLand >= WalkableFloor
				&& Result.Selected.ConnectedLand >= ConnectedFloor));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersEnvironmentCandidateSelectorCounterexampleTest,
	"Gaters.Worldgen.EnvironmentCandidateSelector.Counterexamples",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersEnvironmentCandidateSelectorCounterexampleTest::RunTest(
	const FString& Parameters)
{
	const int32 Seed = 83;
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(Seed, WorldSize)
		.WithProfile(EGatersEnvironment::Lowlands, EGatersHydrology::Dry);
	const TArray<FGatersLandformProtectedRegion> Protected = {
		{TEXT("arrival"), FVector2D::ZeroVector, 1000.f, 2000.f}};
	FGatersEnvironmentCandidateSelectionSettings Settings = TestSettings();
	Settings.CandidateCount = 1;
	FGatersEnvironmentBrief Brief = VolcanicBrief();
	FGatersCompiledEnvironmentBrief Intent =
		FGatersEnvironmentBriefCompiler::Compile(Brief, Seed, WorldSize).Intent;
	const FGatersTraversabilityEvaluation CandidateZero =
		CandidateZeroEvaluation(Environment, Intent, Protected, Settings);
	const float OppositeWalkable = CandidateZero.WalkableFraction > 0.5f ? 0.f : 1.f;
	const float OppositeConnected = CandidateZero.ReachableFraction > 0.5f ? 0.f : 1.f;
	Brief.LandAccess.WalkableLand = {OppositeWalkable, OppositeWalkable};
	Brief.LandAccess.ConnectedLand = {OppositeConnected, OppositeConnected};
	Intent = FGatersEnvironmentBriefCompiler::Compile(Brief, Seed, WorldSize).Intent;

	const FGatersEnvironmentCandidateSelectionResult Impossible =
		FGatersEnvironmentCandidateSelector::Select(
			Environment, Intent, Protected, Settings);
	TestFalse(TEXT("unsatisfied target does not replace champion"),
		Impossible.bSelected);
	TestEqual(TEXT("unsatisfied target preserves attempt evidence"),
		Impossible.Candidates.Num(), 1);
	TestEqual(TEXT("unsatisfied target preserves best candidate evidence"),
		Impossible.Best.CandidateIndex, 0);
	TestEqual(TEXT("unsatisfied target preserves best candidate recipe"),
		Impossible.BestRecipe.CandidateIndex, 0);

	FGatersEnvironmentCandidateSelectionSettings Invalid = Settings;
	Invalid.CandidateCount = 0;
	const FGatersEnvironmentCandidateSelectionResult Rejected =
		FGatersEnvironmentCandidateSelector::Select(
			Environment, Intent, Protected, Invalid);
	TestTrue(TEXT("invalid selector settings are causal"),
		HasIssue(Rejected, TEXT("land-access.settings")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersEnvironmentCandidateSelectorWalkableControlTest,
	"Gaters.Worldgen.EnvironmentCandidateSelector.WalkableControl",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersEnvironmentCandidateSelectorWalkableControlTest::RunTest(
	const FString& Parameters)
{
	const int32 Seed = 29;
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(Seed, WorldSize);
	const TArray<FGatersLandformProtectedRegion> Protected = {
		{TEXT("arrival"), FVector2D::ZeroVector, 1000.f, 2000.f}};
	FGatersEnvironmentCandidateSelectionSettings Settings = TestSettings();
	Settings.CandidateCount = 8;
	Settings.WalkableTolerance = 0.15f;
	Settings.ConnectedTolerance = 0.15f;
	const FGatersCompiledEnvironmentBrief Intent =
		FGatersEnvironmentBriefCompiler::Compile(
			FGatersEnvironmentBrief(), Seed, WorldSize).Intent;
	const FGatersEnvironmentCandidateSelectionResult Result =
		FGatersEnvironmentCandidateSelector::Select(
			Environment, Intent, Protected, Settings);
	const FGatersEnvironmentCandidateEvidence* CandidateZero =
		Result.Candidates.FindByPredicate(
			[](const FGatersEnvironmentCandidateEvidence& Candidate)
			{
				return Candidate.CandidateIndex == 0;
			});
	TestNotNull(TEXT("candidate zero evidence remains available"), CandidateZero);
	if (!CandidateZero)
	{
		return false;
	}
	const float ConnectedFloor = Intent.LandAccess.ConnectedLand
		* (1.f - Settings.ConnectedTolerance);
	const FGatersEnvironmentCandidateEvidence* Improved =
		Result.Candidates.FindByPredicate(
			[CandidateZero, ConnectedFloor](
				const FGatersEnvironmentCandidateEvidence& Candidate)
			{
				return Candidate.CandidateIndex > 0
					&& Candidate.bHasWorldAccess
					&& Candidate.bEscapesArrival
					&& Candidate.ConnectedLand >= ConnectedFloor
					&& Candidate.WalkableError
						< CandidateZero->WalkableError - 0.1f;
			});
	TestNotNull(TEXT("target-derived candidate materially improves walkable error"),
		Improved);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersEnvironmentCandidateSelectorArrivalTransitionTest,
	"Gaters.Worldgen.EnvironmentCandidateSelector.ArrivalTransition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersEnvironmentCandidateSelectorArrivalTransitionTest::RunTest(
	const FString& Parameters)
{
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(11, WorldSize);
	const FGatersEnvironmentCandidateSelectionSettings Settings = TestSettings();
	const FGatersTerrainSemanticField ArrivalField = FGatersTerrainSemanticField::Build(
		Environment, Settings.ArrivalCellsPerAxis, Settings.ArrivalCellSize,
		Settings.PadRadius, Settings.FlatNormalZ, Settings.SlopeNormalZ);
	const int32 Half = Settings.ArrivalCellsPerAxis / 2;
	const FGatersTraversabilityEvaluation Arrival =
		FGatersTraversabilityEvaluator::Evaluate(
			ArrivalField, FIntPoint(Half, Half), FIntPoint(Half, Half),
			Settings.EscapeDistanceCells);

	TestTrue(TEXT("world-only Arrival escapes without a site or route"),
		Arrival.bEscapesStart);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersEnvironmentCandidateSelectorProvenCandidateTest,
	"Gaters.Worldgen.EnvironmentCandidateSelector.ProvenCandidate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersEnvironmentCandidateSelectorProvenCandidateTest::RunTest(
	const FString& Parameters)
{
	const int32 Seed = 131;
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(Seed, WorldSize);
	const TArray<FGatersLandformProtectedRegion> Protected = {
		{TEXT("arrival"), FVector2D::ZeroVector, 1000.f, 2000.f}};
	FGatersEnvironmentCandidateSelectionSettings Settings = TestSettings();
	Settings.CandidateCount = 8;
	Settings.WalkableTolerance = 0.15f;
	Settings.ConnectedTolerance = 0.15f;
	const FGatersCompiledEnvironmentBrief Intent =
		FGatersEnvironmentBriefCompiler::Compile(
			VolcanicBrief(), Seed, WorldSize).Intent;
	const FGatersEnvironmentCandidateSelectionResult Result =
		FGatersEnvironmentCandidateSelector::Select(
			Environment, Intent, Protected, Settings);

	TestTrue(TEXT("proven volcanic held-out candidate remains selectable"),
		Result.bSelected);
	const FGatersEnvironmentCandidateEvidence* Proven =
		Result.Candidates.FindByPredicate(
			[](const FGatersEnvironmentCandidateEvidence& Candidate)
			{
				return Candidate.CandidateIndex == 7;
			});
	TestNotNull(TEXT("proven volcanic candidate slot remains available"), Proven);
	if (!Proven)
	{
		return false;
	}
	const float WalkableFloor = Intent.LandAccess.WalkableLand
		* (1.f - Settings.WalkableTolerance);
	const float ConnectedFloor = Intent.LandAccess.ConnectedLand
		* (1.f - Settings.ConnectedTolerance);
	TestTrue(TEXT("proven volcanic candidate remains independently valid"),
		Proven->bHasWorldAccess
			&& Proven->bEscapesArrival
			&& Proven->WalkableError <= Settings.WalkableTolerance
			&& Proven->ConnectedError <= Settings.ConnectedTolerance
			&& Proven->WalkableLand >= WalkableFloor
			&& Proven->ConnectedLand >= ConnectedFloor);
	return true;
}

#endif
