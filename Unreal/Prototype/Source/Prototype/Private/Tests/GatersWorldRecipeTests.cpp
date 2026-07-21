#if WITH_DEV_AUTOMATION_TESTS

#include "GatersEnvironment.h"
#include "GatersWorldRecipe.h"
#include "Misc/AutomationTest.h"

#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersWorldRecipeCanonicalTest,
	"Gaters.Worldgen.WorldRecipe.Canonical",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersWorldRecipeCanonicalTest::RunTest(const FString& Parameters)
{
	constexpr float ChunkSize = 30000.f;
	const FGatersWorldRecipe A = FGatersWorldRecipe::Generate(
		73, ChunkSize, 6000.f, 10800.f, 900.f, 350.f);
	const FGatersWorldRecipe B = FGatersWorldRecipe::Generate(
		73, ChunkSize, 6000.f, 10800.f, 900.f, 350.f);
	const FGatersWorldRecipe Other = FGatersWorldRecipe::Generate(
		74, ChunkSize, 6000.f, 10800.f, 900.f, 350.f);

	TestEqual(TEXT("arrival recipe semantics advance generator identity"), A.GeneratorVersion, 12);
	TestEqual(TEXT("identical inputs produce identical canonical text"),
		A.CanonicalText(), B.CanonicalText());
	TestEqual(TEXT("identical inputs produce identical checksums"), A.Checksum(), B.Checksum());
	TestNotEqual(TEXT("another seed has another canonical identity"), A.Checksum(), Other.Checksum());

	const FGatersEnvironment Direct = FGatersEnvironment::FromSeed(73, ChunkSize);
	FVector2D DirectBaseSite = FVector2D::ZeroVector;
	const bool bDirectHasBaseSite = Direct.FindBaseSite(
		6000.f, 10800.f, 900.f, 350.f, DirectBaseSite);
	TestEqual(TEXT("recipe records the direct environment family"), A.EnvironmentType, Direct.Type);
	TestEqual(TEXT("recipe records the direct hydrology"), A.Hydrology, Direct.Hydrology);
	TestEqual(TEXT("recipe records the direct water height"), A.WaterHeight, Direct.WaterHeight);
	TestEqual(TEXT("recipe records direct base validity"), A.bHasBaseSite, bDirectHasBaseSite);
	TestEqual(TEXT("recipe records the direct base site"), A.BaseSite, DirectBaseSite);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersWorldRecipeEnvironmentTest,
	"Gaters.Worldgen.WorldRecipe.EnvironmentAdapter",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersWorldRecipeEnvironmentTest::RunTest(const FString& Parameters)
{
	const FGatersWorldRecipe Recipe = FGatersWorldRecipe::Generate(
		73, 30000.f, 6000.f, 10800.f, 900.f, 350.f);
	const FGatersEnvironment Environment = Recipe.CreateEnvironment();

	TestEqual(TEXT("adapter reconstructs the recipe family"),
		Environment.Type, Recipe.EnvironmentType);
	TestEqual(TEXT("adapter reconstructs the recipe water height"),
		Environment.WaterHeight, Recipe.WaterHeight);
	TestEqual(TEXT("adapter reconstructs the recipe hydrology"),
		Environment.Hydrology, Recipe.Hydrology);
	TestEqual(TEXT("adapter reconstructs the recipe name"),
		Environment.Name(), Recipe.EnvironmentName);
	TestTrue(TEXT("recorded base site belongs to the reconstructed terrain"),
		!Recipe.bHasBaseSite || Environment.IsFootprintDry(Recipe.BaseSite, 900.f, 50.f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersWorldRecipeNodesTest,
	"Gaters.Worldgen.WorldRecipe.Nodes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersWorldRecipeNodesTest::RunTest(const FString& Parameters)
{
	const FGatersWorldRecipe Recipe = FGatersWorldRecipe::Generate(
		73, 30000.f, 6000.f, 10800.f, 900.f, 350.f);
	const FGatersWorldRecipe Same = FGatersWorldRecipe::Generate(
		73, 30000.f, 6000.f, 10800.f, 900.f, 350.f);

	TestEqual(TEXT("valid recipe contains Arrival and BaseSite nodes"), Recipe.Nodes.Num(), 2);
	TestEqual(TEXT("same seed preserves first node ID"), Recipe.Nodes[0].Id, Same.Nodes[0].Id);
	TestEqual(TEXT("same seed preserves second node ID"), Recipe.Nodes[1].Id, Same.Nodes[1].Id);

	const FGatersRecipeNode* Arrival = Recipe.FindNode(TEXT("arrival:0"));
	const FGatersRecipeNode* Base = Recipe.FindNode(TEXT("base:0"));
	TestNotNull(TEXT("recipe has a stable Arrival node"), Arrival);
	TestNotNull(TEXT("recipe has a stable BaseSite node"), Base);
	if (!Arrival || !Base)
	{
		return false;
	}
	TestEqual(TEXT("Arrival node has Arrival kind"), Arrival->Kind, EGatersRecipeNodeKind::Arrival);
	TestEqual(TEXT("Arrival node is at local origin"), Arrival->Location, FVector::ZeroVector);
	TestEqual(TEXT("BaseSite node has BaseSite kind"), Base->Kind, EGatersRecipeNodeKind::BaseSite);
	TestEqual(TEXT("BaseSite node records recipe X"), Base->Location.X, Recipe.BaseSite.X);
	TestEqual(TEXT("BaseSite node records recipe Y"), Base->Location.Y, Recipe.BaseSite.Y);
	TestEqual(TEXT("BaseSite node sits on generated ground"),
		Base->Location.Z,
		static_cast<double>(Recipe.CreateEnvironment().HeightAt(Recipe.BaseSite)));

	FGatersWorldRecipe Moved = Recipe;
	Moved.Nodes[0].Location.X += 1.f;
	TestNotEqual(TEXT("moving a semantic node changes recipe identity"),
		Moved.Checksum(), Recipe.Checksum());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersWorldRecipeValidationTest,
	"Gaters.Worldgen.WorldRecipe.Validation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersWorldRecipeValidationTest::RunTest(const FString& Parameters)
{
	const FGatersWorldRecipe Valid = FGatersWorldRecipe::Generate(
		73, 30000.f, 6000.f, 10800.f, 900.f, 350.f);
	TArray<FString> Errors;
	TestTrue(TEXT("generated recipe is structurally valid"), Valid.Validate(Errors));
	TestEqual(TEXT("valid recipe has no diagnostics"), Errors.Num(), 0);

	auto HasError = [](const TArray<FString>& Items, const TCHAR* Fragment)
	{
		return Items.ContainsByPredicate([Fragment](const FString& Item)
		{
			return Item.Contains(Fragment);
		});
	};

	FGatersWorldRecipe Duplicate = Valid;
	const FGatersRecipeNode DuplicateNode = Duplicate.Nodes[0];
	Duplicate.Nodes.Add(DuplicateNode);
	TestFalse(TEXT("duplicate stable IDs are rejected"), Duplicate.Validate(Errors));
	TestTrue(TEXT("duplicate diagnostic names the ID"), HasError(Errors, TEXT("duplicate node id arrival:0")));

	FGatersWorldRecipe MissingArrival = Valid;
	MissingArrival.Nodes.RemoveAt(0);
	TestFalse(TEXT("missing Arrival is rejected"), MissingArrival.Validate(Errors));
	TestTrue(TEXT("missing Arrival diagnostic states cardinality"), HasError(Errors, TEXT("exactly one Arrival node")));

	FGatersWorldRecipe MismatchedBase = Valid;
	MismatchedBase.Nodes[1].Location.X += 1.f;
	TestFalse(TEXT("mismatched BaseSite node is rejected"), MismatchedBase.Validate(Errors));
	TestTrue(TEXT("base mismatch diagnostic names the contract"),
		HasError(Errors, TEXT("BaseSite node does not match BaseSite")));

	FGatersWorldRecipe NonFinite = Valid;
	NonFinite.Nodes[0].Location.X = std::numeric_limits<double>::quiet_NaN();
	TestFalse(TEXT("non-finite node location is rejected"), NonFinite.Validate(Errors));
	TestTrue(TEXT("non-finite diagnostic names the node"),
		HasError(Errors, TEXT("node arrival:0 has non-finite location")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersWorldRecipeContentKindsTest,
	"Gaters.Worldgen.WorldRecipe.ContentKinds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersWorldRecipeContentKindsTest::RunTest(const FString& Parameters)
{
	FGatersWorldRecipe Recipe = FGatersWorldRecipe::Generate(
		73, 30000.f, 6000.f, 10800.f, 900.f, 350.f);
	const uint32 StructuralChecksum = Recipe.Checksum();
	Recipe.Nodes.Add({TEXT("scatter:47"), EGatersRecipeNodeKind::ScatterTree, FVector(10, 20, 30)});
	Recipe.Nodes.Add({TEXT("scatter:48"), EGatersRecipeNodeKind::ScatterRock, FVector(40, 50, 60)});
	Recipe.Nodes.Add({TEXT("plot:3"), EGatersRecipeNodeKind::BuildPlot, FVector(70, 80, 90)});
	Recipe.Nodes.Add({TEXT("site:village:0"), EGatersRecipeNodeKind::VillageSite, FVector(100, 110, 120)});
	Recipe.Nodes.Add({TEXT("site:landmark:0"), EGatersRecipeNodeKind::LandmarkSite, FVector(130, 140, 150)});
	FGatersRecipeNode Module{
		TEXT("settlement:building:home:0:module:wall:0"),
		EGatersRecipeNodeKind::SettlementModule,
		FVector(140, 150, 160)};
	Module.ContentKey = TEXT("building.wall");
	Recipe.Nodes.Add(Module);
	FGatersRecipeNode RoutePoint{
		TEXT("route:arrival-village:0"), EGatersRecipeNodeKind::RouteWaypoint, FVector(160, 170, 180)};
	RoutePoint.ContentKey = TEXT("route:arrival-village");
	Recipe.Nodes.Add(RoutePoint);

	TestNotNull(TEXT("tree node is addressable"), Recipe.FindNode(TEXT("scatter:47")));
	TestNotNull(TEXT("rock node is addressable"), Recipe.FindNode(TEXT("scatter:48")));
	TestNotNull(TEXT("plot node is addressable"), Recipe.FindNode(TEXT("plot:3")));
	TestNotNull(TEXT("village site is addressable"), Recipe.FindNode(TEXT("site:village:0")));
	TestNotNull(TEXT("landmark site is addressable"), Recipe.FindNode(TEXT("site:landmark:0")));
	TestNotNull(TEXT("building module is addressable"),
		Recipe.FindNode(TEXT("settlement:building:home:0:module:wall:0")));
	TestNotNull(TEXT("route waypoint is addressable"),
		Recipe.FindNode(TEXT("route:arrival-village:0")));
	TestNotEqual(TEXT("content nodes contribute to canonical identity"),
		Recipe.Checksum(), StructuralChecksum);
	FGatersWorldRecipe MovedRoute = Recipe;
	MovedRoute.Nodes.Last().Location.X += 1.f;
	TestNotEqual(TEXT("moving a route waypoint changes recipe identity"),
		MovedRoute.Checksum(), Recipe.Checksum());
	TArray<FString> Errors;
	TestTrue(TEXT("generic structural validation accepts content nodes"), Recipe.Validate(Errors));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersWorldRecipeBaseNodeContractTest,
	"Gaters.Worldgen.WorldRecipe.BaseNodeContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersWorldRecipeBaseNodeContractTest::RunTest(const FString& Parameters)
{
	const FGatersWorldRecipe Structural = FGatersWorldRecipe::Generate(
		73, 30000.f, 6000.f, 10800.f, 900.f, 350.f);
	FGatersWorldRecipe Recipe = Structural;
	FGatersRecipeNode Piece{
		TEXT("piece:0"), EGatersRecipeNodeKind::BasePiece, FVector(100, 200, 300)};
	Piece.Rotation = FRotator(0, 90, 0);
	Piece.Scale = FVector(1, 1, 1.5);
	Piece.ContentKey = TEXT("wood.foundation");
	Recipe.Nodes.Add(Piece);
	Recipe.Nodes.Add({TEXT("loot:0"), EGatersRecipeNodeKind::RaidLoot, FVector(120, 220, 420)});

	TestNotEqual(TEXT("base transform and content contribute to identity"),
		Recipe.Checksum(), Structural.Checksum());
	TArray<FString> Errors;
	TestTrue(TEXT("well-formed BasePiece and loot nodes validate"), Recipe.Validate(Errors));

	FGatersWorldRecipe EmptyKey = Recipe;
	EmptyKey.Nodes[2].ContentKey.Reset();
	TestFalse(TEXT("BasePiece requires a content key"), EmptyKey.Validate(Errors));
	TestTrue(TEXT("content diagnostic names the piece"),
		Errors.Contains(TEXT("BasePiece node piece:0 requires ContentKey")));

	FGatersWorldRecipe BadRotation = Recipe;
	BadRotation.Nodes[2].Rotation.Yaw = std::numeric_limits<double>::quiet_NaN();
	TestFalse(TEXT("non-finite rotation is rejected"), BadRotation.Validate(Errors));
	TestTrue(TEXT("rotation diagnostic names the piece"),
		Errors.Contains(TEXT("node piece:0 has non-finite rotation")));

	FGatersWorldRecipe BadScale = Recipe;
	BadScale.Nodes[2].Scale.Z = 0;
	TestFalse(TEXT("non-positive scale is rejected"), BadScale.Validate(Errors));
	TestTrue(TEXT("scale diagnostic names the piece"),
		Errors.Contains(TEXT("node piece:0 requires positive scale")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersWorldRecipeLinksTest,
	"Gaters.Worldgen.WorldRecipe.Links",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersWorldRecipeLinksTest::RunTest(const FString& Parameters)
{
	FGatersWorldRecipe Recipe = FGatersWorldRecipe::Generate(
		73, 30000.f, 6000.f, 10800.f, 900.f, 350.f);
	FGatersRecipeNode Foundation{
		TEXT("piece:foundation"), EGatersRecipeNodeKind::BasePiece, FVector(100, 200, 300)};
	Foundation.ContentKey = TEXT("building.foundation.wood");
	FGatersRecipeNode Wall{
		TEXT("piece:wall"), EGatersRecipeNodeKind::BasePiece, FVector(100, 200, 500)};
	Wall.ContentKey = TEXT("building.wall.wood");
	Recipe.Nodes.Add(Foundation);
	Recipe.Nodes.Add(Wall);
	const uint32 WithoutLink = Recipe.Checksum();
	Recipe.Links.Add({
		TEXT("link:foundation-wall"),
		TEXT("piece:foundation"), TEXT("top"),
		TEXT("piece:wall"), TEXT("bottom")});

	TestNotEqual(TEXT("connection contributes to recipe identity"), Recipe.Checksum(), WithoutLink);
	FGatersWorldRecipe OtherPort = Recipe;
	OtherPort.Links[0].ToPort = TEXT("side");
	TestNotEqual(TEXT("changing a connection port changes identity"),
		OtherPort.Checksum(), Recipe.Checksum());
	return true;
}

#endif
