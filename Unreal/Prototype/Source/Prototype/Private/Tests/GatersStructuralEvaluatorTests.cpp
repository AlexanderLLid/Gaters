#if WITH_DEV_AUTOMATION_TESTS

#include "GatersStructuralEvaluator.h"
#include "GatersAssetContract.h"
#include "GatersWorldRecipe.h"
#include "Misc/AutomationTest.h"

#include <limits>

namespace
{
const FGatersStructuralIssue* FindIssue(
	const FGatersStructuralEvaluation& Evaluation,
	const TCHAR* RuleId)
{
	return Evaluation.Issues.FindByPredicate([RuleId](const FGatersStructuralIssue& Issue)
	{
		return Issue.RuleId == RuleId;
	});
}

FGatersWorldRecipe ValidRecipe()
{
	return FGatersWorldRecipe::Generate(73, 30000.f, 6000.f, 10800.f, 900.f, 350.f);
}

FGatersAssetContract PieceContract(
	const TCHAR* AssetId,
	const TCHAR* ContentKey,
	const TCHAR* PortName,
	const FVector& PortLocation)
{
	FGatersAssetContract Contract;
	Contract.AssetId = AssetId;
	Contract.ContentKey = ContentKey;
	Contract.StyleId = TEXT("gaters.clean-midpoly-painted");
	Contract.BoundsExtent = FVector(100);
	Contract.ClearanceExtent = FVector(100);
	Contract.Collision = EGatersAssetCollision::Simple;
	Contract.Contacts.Add({TEXT("ground"), FVector(0, 0, -100), FVector::UpVector});
	FGatersAssetPort Port;
	Port.Name = PortName;
	Port.Transform.SetLocation(PortLocation);
	Port.ClearanceExtent = FVector(10);
	Contract.Ports.Add(Port);
	return Contract;
}

FGatersWorldRecipe ConnectedRecipe()
{
	FGatersWorldRecipe Recipe = ValidRecipe();
	FGatersRecipeNode Foundation{
		TEXT("piece:foundation"), EGatersRecipeNodeKind::BasePiece, FVector(100, 200, 300)};
	Foundation.ContentKey = TEXT("building.foundation.wood");
	FGatersRecipeNode Wall{
		TEXT("piece:wall"), EGatersRecipeNodeKind::BasePiece, FVector(100, 200, 500)};
	Wall.ContentKey = TEXT("building.wall.wood");
	Recipe.Nodes.Add(Foundation);
	Recipe.Nodes.Add(Wall);
	Recipe.Links.Add({
		TEXT("link:foundation-wall"),
		TEXT("piece:foundation"), TEXT("top"),
		TEXT("piece:wall"), TEXT("bottom")});
	return Recipe;
}

FGatersStructuralContext ConnectedContext()
{
	FGatersStructuralContext Context;
	Context.PortToleranceCm = 1.f;
	Context.AssetContracts.Add(
		TEXT("building.foundation.wood"),
		PieceContract(TEXT("foundation-1"), TEXT("building.foundation.wood"),
			TEXT("top"), FVector(0, 0, 100)));
	Context.AssetContracts.Add(
		TEXT("building.wall.wood"),
		PieceContract(TEXT("wall-1"), TEXT("building.wall.wood"),
			TEXT("bottom"), FVector(0, 0, -100)));
	return Context;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersStructuralEvaluatorValidTest,
	"Gaters.Evaluation.Structural.ValidRecipe",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersStructuralEvaluatorValidTest::RunTest(const FString& Parameters)
{
	const FGatersStructuralEvaluation Evaluation =
		FGatersStructuralEvaluator::Evaluate(ValidRecipe());
	TestTrue(TEXT("generated recipe passes structural evaluation"), Evaluation.IsValid());
	TestEqual(TEXT("valid recipe has no issues"), Evaluation.Issues.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersStructuralEvaluatorIdentityTest,
	"Gaters.Evaluation.Structural.Identity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersStructuralEvaluatorIdentityTest::RunTest(const FString& Parameters)
{
	FGatersWorldRecipe Recipe = ValidRecipe();
	const FGatersRecipeNode Duplicate = Recipe.Nodes[0];
	Recipe.Nodes.Add(Duplicate);
	const FGatersStructuralEvaluation Evaluation = FGatersStructuralEvaluator::Evaluate(Recipe);
	const FGatersStructuralIssue* Issue = FindIssue(Evaluation, TEXT("node.id.unique"));
	TestNotNull(TEXT("duplicate ID reports its stable rule"), Issue);
	if (Issue)
	{
		TestTrue(TEXT("duplicate issue names the involved node"), Issue->NodeIds.Contains(TEXT("arrival:0")));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersStructuralEvaluatorTransformContentTest,
	"Gaters.Evaluation.Structural.TransformContent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersStructuralEvaluatorTransformContentTest::RunTest(const FString& Parameters)
{
	FGatersWorldRecipe Recipe = ValidRecipe();
	Recipe.Nodes[0].Location.X = std::numeric_limits<double>::quiet_NaN();
	FGatersRecipeNode Piece{
		TEXT("piece:0"), EGatersRecipeNodeKind::BasePiece, FVector(100, 200, 300)};
	Piece.Scale.Z = 0;
	Recipe.Nodes.Add(Piece);
	const FGatersStructuralEvaluation Evaluation = FGatersStructuralEvaluator::Evaluate(Recipe);
	TestNotNull(TEXT("non-finite position reports its stable rule"),
		FindIssue(Evaluation, TEXT("node.transform.location_finite")));
	TestNotNull(TEXT("non-positive scale reports its stable rule"),
		FindIssue(Evaluation, TEXT("node.transform.scale_positive")));
	TestNotNull(TEXT("missing piece content reports its stable rule"),
		FindIssue(Evaluation, TEXT("node.content.required")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersStructuralEvaluatorRelationshipsTest,
	"Gaters.Evaluation.Structural.RequiredNodes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersStructuralEvaluatorRelationshipsTest::RunTest(const FString& Parameters)
{
	FGatersWorldRecipe Recipe = ValidRecipe();
	Recipe.Nodes.RemoveAt(0);
	Recipe.Nodes[0].Location.X += 1.f;
	const FGatersStructuralEvaluation Evaluation = FGatersStructuralEvaluator::Evaluate(Recipe);
	TestNotNull(TEXT("missing Arrival reports cardinality rule"),
		FindIssue(Evaluation, TEXT("recipe.arrival.cardinality")));
	TestNotNull(TEXT("moved BaseSite reports location rule"),
		FindIssue(Evaluation, TEXT("recipe.base.location")));

	FGatersWorldRecipe WrongRecordedHeight = ValidRecipe();
	WrongRecordedHeight.BaseSiteHeight += 100.f;
	TestNotNull(TEXT("base node must match recorded generation evidence"),
		FindIssue(FGatersStructuralEvaluator::Evaluate(WrongRecordedHeight),
			TEXT("recipe.base.location")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersStructuralEvaluatorBudgetTest,
	"Gaters.Evaluation.Structural.Budget",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersStructuralEvaluatorBudgetTest::RunTest(const FString& Parameters)
{
	FGatersStructuralLimits Limits;
	Limits.MaxNodes = 1;
	const FGatersStructuralEvaluation Evaluation =
		FGatersStructuralEvaluator::Evaluate(ValidRecipe(), Limits);
	const FGatersStructuralIssue* Issue = FindIssue(Evaluation, TEXT("recipe.node_budget"));
	TestNotNull(TEXT("node overflow reports its stable rule"), Issue);
	if (Issue)
	{
		TestTrue(TEXT("budget issue carries measurement"), Issue->bHasMeasurement);
		TestEqual(TEXT("budget issue records node count"), Issue->Measured, 2.0);
		TestEqual(TEXT("budget issue records limit"), Issue->Limit, 1.0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersStructuralEvaluatorLinksTest,
	"Gaters.Evaluation.Structural.ContractLinks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersStructuralEvaluatorLinksTest::RunTest(const FString& Parameters)
{
	const FGatersWorldRecipe Valid = ConnectedRecipe();
	const FGatersStructuralContext Context = ConnectedContext();
	TestTrue(TEXT("aligned contracted ports pass"),
		FGatersStructuralEvaluator::Evaluate(Valid, Context).IsValid());

	FGatersWorldRecipe Duplicate = Valid;
	Duplicate.Links.Add(Valid.Links[0]);
	TestNotNull(TEXT("duplicate link ID reports its stable rule"), FindIssue(
		FGatersStructuralEvaluator::Evaluate(Duplicate, Context), TEXT("link.id.unique")));

	FGatersWorldRecipe MissingNode = Valid;
	MissingNode.Links[0].ToNodeId = TEXT("piece:missing");
	TestNotNull(TEXT("missing endpoint reports its stable rule"), FindIssue(
		FGatersStructuralEvaluator::Evaluate(MissingNode, Context), TEXT("link.node.missing")));

	FGatersStructuralContext MissingContract = Context;
	MissingContract.AssetContracts.Remove(TEXT("building.wall.wood"));
	TestNotNull(TEXT("missing contract reports its stable rule"), FindIssue(
		FGatersStructuralEvaluator::Evaluate(Valid, MissingContract), TEXT("link.contract.missing")));

	FGatersWorldRecipe MissingPort = Valid;
	MissingPort.Links[0].ToPort = TEXT("missing");
	TestNotNull(TEXT("missing port reports its stable rule"), FindIssue(
		FGatersStructuralEvaluator::Evaluate(MissingPort, Context), TEXT("link.port.missing")));

	FGatersWorldRecipe Misaligned = Valid;
	Misaligned.Nodes.Last().Location.Z += 10.f;
	const FGatersStructuralIssue* Alignment = FindIssue(
		FGatersStructuralEvaluator::Evaluate(Misaligned, Context), TEXT("link.port.alignment"));
	TestNotNull(TEXT("misaligned ports report their stable rule"), Alignment);
	if (Alignment)
	{
		TestTrue(TEXT("alignment issue carries distance and tolerance"), Alignment->bHasMeasurement);
		TestEqual(TEXT("alignment issue records port distance"), Alignment->Measured, 10.0);
		TestEqual(TEXT("alignment issue records tolerance"), Alignment->Limit, 1.0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersStructuralEvaluatorSpatialTest,
	"Gaters.Evaluation.Structural.Spatial",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersStructuralEvaluatorSpatialTest::RunTest(const FString& Parameters)
{
	const FGatersWorldRecipe Touching = ConnectedRecipe();
	FGatersStructuralContext Context = ConnectedContext();
	Context.OverlapToleranceCm = 1.f;
	TestNull(TEXT("touching contracted bounds do not overlap"), FindIssue(
		FGatersStructuralEvaluator::Evaluate(Touching, Context), TEXT("node.bounds.overlap")));

	FGatersWorldRecipe Penetrating = Touching;
	Penetrating.Nodes.Last().Location.Z -= 50.f;
	const FGatersStructuralIssue* Overlap = FindIssue(
		FGatersStructuralEvaluator::Evaluate(Penetrating, Context), TEXT("node.bounds.overlap"));
	TestNotNull(TEXT("penetrating contracted bounds report overlap"), Overlap);
	if (Overlap)
	{
		TestTrue(TEXT("overlap carries penetration and tolerance"), Overlap->bHasMeasurement);
		TestEqual(TEXT("overlap records minimum penetration"), Overlap->Measured, 50.0);
		TestEqual(TEXT("overlap records tolerance"), Overlap->Limit, 1.0);
	}

	FGatersStructuralContext InvalidContract = Context;
	InvalidContract.AssetContracts[TEXT("building.wall.wood")].BoundsExtent = FVector::ZeroVector;
	TestNotNull(TEXT("invalid piece contract reports its stable rule"), FindIssue(
		FGatersStructuralEvaluator::Evaluate(Touching, InvalidContract),
		TEXT("node.contract.invalid")));

	FGatersWorldRecipe Disconnected = Touching;
	Disconnected.Links.Reset();
	TestNotNull(TEXT("disconnected pieces report their stable rule"), FindIssue(
		FGatersStructuralEvaluator::Evaluate(Disconnected, Context),
		TEXT("recipe.piece_connectivity")));
	return true;
}

#endif
