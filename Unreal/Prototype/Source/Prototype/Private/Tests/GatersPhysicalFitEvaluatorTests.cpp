#if WITH_DEV_AUTOMATION_TESTS

#include "GatersPhysicalFitEvaluator.h"
#include "GatersEnvironment.h"
#include "GatersTerrainSemanticField.h"
#include "GatersWorldCompiler.h"
#include "Misc/AutomationTest.h"

namespace
{
FGatersAssetContract FitContract()
{
	FGatersAssetContract Contract;
	Contract.AssetId = TEXT("building-hut-1");
	Contract.ContentKey = TEXT("building.hut");
	Contract.StyleId = TEXT("gaters.clean-midpoly-painted");
	Contract.BoundsExtent = FVector(100.f, 100.f, 100.f);
	Contract.ClearanceExtent = FVector(130.f, 130.f, 110.f);
	Contract.Collision = EGatersAssetCollision::Simple;
	Contract.Contacts.Add({TEXT("left-foot"), FVector(-50.f, 0.f, -100.f), FVector::UpVector});
	Contract.Contacts.Add({TEXT("right-foot"), FVector(50.f, 0.f, -100.f), FVector::UpVector});
	FGatersAssetPort Door;
	Door.Name = TEXT("door");
	Door.Transform.SetLocation(FVector(100.f, 0.f, 0.f));
	Door.ClearanceExtent = FVector(50.f, 60.f, 100.f);
	Contract.Ports.Add(Door);
	return Contract;
}

FGatersPhysicalFitCandidate FitCandidate()
{
	FGatersPhysicalFitCandidate Candidate;
	Candidate.RecipeId = TEXT("hut:7");
	Candidate.Contract = FitContract();
	Candidate.Transform = FTransform(FVector(0.f, 0.f, 100.f));
	Candidate.ContactSamples = {
		{TEXT("left-foot"), FVector(-50.f, 0.f, 0.f), 0.f, FVector::UpVector},
		{TEXT("right-foot"), FVector(50.f, 0.f, 0.f), 0.f, FVector::UpVector}};
	return Candidate;
}

const FGatersPhysicalFitIssue* FindIssue(
	const FGatersPhysicalFitEvaluation& Evaluation,
	const TCHAR* RuleId)
{
	return Evaluation.Issues.FindByPredicate([RuleId](const FGatersPhysicalFitIssue& Issue)
	{
		return Issue.RuleId == RuleId;
	});
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersPhysicalFitContactTest,
	"Gaters.Evaluation.PhysicalFit.Contacts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersPhysicalFitContactTest::RunTest(const FString& Parameters)
{
	const FGatersPhysicalFitSettings Settings;
	TestTrue(TEXT("aligned contacts pass"),
		FGatersPhysicalFitEvaluator::Evaluate(FitCandidate(), Settings).IsValid());

	FGatersPhysicalFitCandidate Floating = FitCandidate();
	Floating.ContactSamples[0].TerrainHeight = -20.f;
	const FGatersPhysicalFitEvaluation FloatingEvaluation =
		FGatersPhysicalFitEvaluator::Evaluate(Floating, Settings);
	const FGatersPhysicalFitIssue* FloatIssue = FindIssue(
		FloatingEvaluation, TEXT("fit.contact.floating"));
	TestNotNull(TEXT("floating contact is classified"), FloatIssue);
	if (FloatIssue)
	{
		TestEqual(TEXT("failure names recipe"), FloatIssue->RecipeId, FString(TEXT("hut:7")));
		TestEqual(TEXT("failure names selected asset"),
			FloatIssue->AssetId, FString(TEXT("building-hut-1")));
		TestEqual(TEXT("failure names contact"), FloatIssue->SubjectId, FString(TEXT("left-foot")));
		TestEqual(TEXT("failure records gap"), FloatIssue->Measured, 20.0);
	}

	FGatersPhysicalFitCandidate Buried = FitCandidate();
	Buried.ContactSamples[1].TerrainHeight = 20.f;
	TestNotNull(TEXT("buried contact is classified"), FindIssue(
		FGatersPhysicalFitEvaluator::Evaluate(Buried, Settings), TEXT("fit.contact.buried")));

	FGatersPhysicalFitCandidate Unsupported = FitCandidate();
	Unsupported.ContactSamples[0].TerrainNormal = FVector(1.f, 0.f, 0.f);
	TestNotNull(TEXT("unsupported slope is classified"), FindIssue(
		FGatersPhysicalFitEvaluator::Evaluate(Unsupported, Settings), TEXT("fit.contact.normal")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersPhysicalFitClearanceTest,
	"Gaters.Evaluation.PhysicalFit.Clearance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersPhysicalFitClearanceTest::RunTest(const FString& Parameters)
{
	FGatersPhysicalFitCandidate Blocked = FitCandidate();
	Blocked.Obstacles.Add({TEXT("rock:2"), FBox(FVector(90.f, -20.f, 20.f), FVector(140.f, 20.f, 180.f))});
	const FGatersPhysicalFitEvaluation Evaluation =
		FGatersPhysicalFitEvaluator::Evaluate(Blocked, FGatersPhysicalFitSettings());
	const FGatersPhysicalFitIssue* Clearance = FindIssue(Evaluation, TEXT("fit.clearance.blocked"));
	const FGatersPhysicalFitIssue* Entrance = FindIssue(Evaluation, TEXT("fit.port.blocked"));
	TestNotNull(TEXT("asset clearance obstruction is classified"), Clearance);
	TestNotNull(TEXT("entrance obstruction is classified"), Entrance);
	if (Entrance)
	{
		TestEqual(TEXT("entrance failure names port"), Entrance->SubjectId, FString(TEXT("door")));
		TestEqual(TEXT("entrance failure names blocker"), Entrance->ObstacleId, FString(TEXT("rock:2")));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersPhysicalFitTerrainEvidenceTest,
	"Gaters.Evaluation.PhysicalFit.TerrainEvidence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersPhysicalFitTerrainEvidenceTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(53, 30000.f);
	const FTransform Transform(FRotator::ZeroRotator, FVector(500.f, 250.f, 100.f));
	FGatersAssetContract Contract = FitContract();
	Contract.Contacts.Add({TEXT("wall-socket"), FVector(0.f, 0.f, 100.f),
		FVector::UpVector, EGatersAssetContactSupport::Attachment});
	const FGatersPhysicalFitCandidate Candidate = FGatersPhysicalFitEvaluator::SampleTerrain(
		TEXT("hut:53"), Contract, Transform, Environment, 900.f);
	TestEqual(TEXT("only terrain contacts are sampled"), Candidate.ContactSamples.Num(), 2);
	const FGatersPhysicalFitContactSample& Sample = Candidate.ContactSamples[0];
	const FVector ExpectedLocation = Transform.TransformPosition(
		Candidate.Contract.Contacts[0].Location * Candidate.Contract.CentimetersPerUnit);
	TestTrue(TEXT("contact world position comes from the contract"),
		Sample.WorldLocation.Equals(ExpectedLocation));
	TestEqual(TEXT("height uses the materialized terrain field"), Sample.TerrainHeight,
		FGatersTerrainSemanticField::MaterializedHeightAt(
			Environment, FVector2D(ExpectedLocation), 900.f));
	const FGatersPhysicalFitEvaluation Evaluation =
		FGatersPhysicalFitEvaluator::Evaluate(Candidate);
	TestEqual(TEXT("terrain contact evidence is counted"),
		Evaluation.EvaluatedTerrainContacts, 2);
	TestEqual(TEXT("attachment evidence remains explicitly pending"),
		Evaluation.PendingAttachmentContacts, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersPhysicalFitCompiledWorldTest,
	"Gaters.Evaluation.PhysicalFit.CompiledWorld",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersPhysicalFitCompiledWorldTest::RunTest(const FString& Parameters)
{
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(7, 30000.f);
	const float PadRadius = 900.f;
	FGatersAssetContract Contract = FitContract();
	Contract.Contacts = {{TEXT("ground"), FVector::ZeroVector, FVector::UpVector}};

	FGatersCompiledWorld World;
	FGatersCompiledNode& Contracted = World.Nodes.AddDefaulted_GetRef();
	Contracted.NodeId = TEXT("content:7:0:0:1");
	Contracted.Kind = EGatersRecipeNodeKind::ScatterRock;
	Contracted.Transform.SetLocation(FVector(0.f, 0.f,
		FGatersTerrainSemanticField::MaterializedHeightAt(
			Environment, FVector2D::ZeroVector, PadRadius)));
	Contracted.AssetContract = Contract;
	FGatersCompiledNode& Semantic = World.Nodes.AddDefaulted_GetRef();
	Semantic.NodeId = TEXT("route:0");
	Semantic.Kind = EGatersRecipeNodeKind::RouteWaypoint;

	const TArray<FGatersPhysicalFitEvaluation> Aligned =
		FGatersPhysicalFitEvaluator::EvaluateWorld(World, Environment, PadRadius);
	TestEqual(TEXT("only contracted compiled nodes are evaluated"), Aligned.Num(), 1);
	if (Aligned.Num() == 1)
	{
		TestTrue(TEXT("aligned compiled placement passes"), Aligned[0].IsValid());
	}

	World.Nodes[0].Transform.AddToTranslation(FVector(0.f, 0.f, 20.f));
	const TArray<FGatersPhysicalFitEvaluation> Floating =
		FGatersPhysicalFitEvaluator::EvaluateWorld(World, Environment, PadRadius);
	TestEqual(TEXT("lifted compiled placement remains evaluated"), Floating.Num(), 1);
	if (Floating.Num() == 1)
	{
		const FGatersPhysicalFitIssue* Issue = FindIssue(
			Floating[0], TEXT("fit.contact.floating"));
		TestNotNull(TEXT("lifted compiled placement is classified"), Issue);
		if (Issue)
		{
			TestEqual(TEXT("compiled fit failure names the recipe node"),
				Issue->RecipeId, FString(TEXT("content:7:0:0:1")));
		}
	}
	return true;
}

#endif
