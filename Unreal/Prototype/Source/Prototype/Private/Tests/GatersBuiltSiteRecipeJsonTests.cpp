#if WITH_DEV_AUTOMATION_TESTS

#include "GatersBuiltSiteExportCommandlet.h"
#include "GatersBuiltSiteRecipeJson.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include <limits>

namespace
{
FGatersBuiltSiteRecipe MakeJsonExportRecipe()
{
	FGatersBuiltSiteRecipe Recipe;
	Recipe.GeneratorVersion = 3;
	Recipe.Seed = 73;
	Recipe.SiteId = TEXT("site:village:0");
	Recipe.Kind = EGatersBuiltSiteKind::Settlement;
	Recipe.SiteArea = 160000.f;
	Recipe.EvidenceCoverage = {true, true, true, true, {TEXT("source:coverage")}};

	FGatersBuiltSiteSpace& Yard = Recipe.Spaces.AddDefaulted_GetRef();
	Yard.Id = TEXT("space:yard:0");
	Yard.Center = FVector::ZeroVector;
	Yard.Extent = FVector(200.f, 200.f, 100.f);
	Yard.SemanticRole = TEXT("gathering");
	Yard.Tags = {TEXT("outdoors"), TEXT("yard")};
	Yard.SourceIds = {Recipe.SiteId, TEXT("parcel:0")};

	FGatersBuiltSiteSpace& Room = Recipe.Spaces.AddDefaulted_GetRef();
	Room.Id = TEXT("space:room:0");
	Room.Center = FVector(500.f, 0.f, 130.f);
	Room.Extent = FVector(125.f, 125.f, 130.f);
	Room.SemanticRole = TEXT("home");
	Room.Tags = {TEXT("roofed"), TEXT("building-envelope")};
	Room.SourceIds = {TEXT("building:home:0")};

	FGatersBuiltSiteBlocker& Door = Recipe.Blockers.AddDefaulted_GetRef();
	Door.Id = TEXT("blocker:door:0");
	Door.Center = FVector(375.f, 0.f, 130.f);
	Door.Extent = FVector(10.f, 50.f, 100.f);
	Door.Tags = {TEXT("door")};
	Door.SourceIds = {TEXT("module:door:0")};

	FGatersBuiltSiteConnection& Enter = Recipe.Connections.AddDefaulted_GetRef();
	Enter.Id = TEXT("connection:yard-room:0");
	Enter.FromSpaceId = Yard.Id;
	Enter.ToSpaceId = Room.Id;
	Enter.Width = 250.f;
	Enter.Headroom = 260.f;
	Enter.BlockerIds = {Door.Id};
	Enter.MovementModeIds = {TEXT("ground"), TEXT("water-surface")};
	Enter.SourceIds = {TEXT("module:door:0")};

	FGatersBuiltSiteVisibility& Sight = Recipe.Visibility.AddDefaulted_GetRef();
	Sight.Id = TEXT("visibility:yard-room:0");
	Sight.FromSpaceId = Yard.Id;
	Sight.ToSpaceId = Room.Id;
	Sight.Distance = 500.f;
	Sight.BlockerIds = {Door.Id};
	Sight.SourceIds = {TEXT("sample:sight:0")};

	FGatersBuiltSitePlacementSlot& Slot = Recipe.PlacementSlots.AddDefaulted_GetRef();
	Slot.Id = TEXT("slot:room:0");
	Slot.SpaceId = Room.Id;
	Slot.Location = Room.Center;
	Slot.ClearanceRadius = 50.f;
	Slot.ClearanceHeight = 180.f;
	Slot.Tags = {TEXT("grounded")};
	Slot.SourceIds = {TEXT("building:home:0")};
	return Recipe;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBuiltSiteRecipeJsonSerializerTest,
	"Gaters.BuiltSites.JsonExport.Serializer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBuiltSiteRecipeJsonSerializerTest::RunTest(const FString& Parameters)
{
	const FGatersBuiltSiteRecipe Recipe = MakeJsonExportRecipe();
	FString First;
	FString Repeat;
	TArray<FString> Diagnostics;
	TestTrue(TEXT("valid recipe serializes"),
		FGatersBuiltSiteRecipeJson::Serialize({Recipe}, First, Diagnostics));
	TestTrue(TEXT("valid export has no diagnostics"), Diagnostics.IsEmpty());
	TestTrue(TEXT("repeat serializes"),
		FGatersBuiltSiteRecipeJson::Serialize({Recipe}, Repeat, Diagnostics));
	TestEqual(TEXT("fixed input exports byte-identical JSON"), First, Repeat);
	TestTrue(TEXT("units are explicit"),
		First.Contains(TEXT("\"coordinateUnit\":\"centimetres\""))
		&& First.Contains(TEXT("\"lengthUnit\":\"centimetres\""))
		&& First.Contains(TEXT("\"areaUnit\":\"square centimetres\"")));
	TestTrue(TEXT("site identity is exported"),
		First.Contains(TEXT("\"siteId\":\"site:village:0\""))
		&& First.Contains(TEXT("\"kind\":\"settlement\""))
		&& First.Contains(FString::Printf(
			TEXT("\"checksum\":%llu"), static_cast<uint64>(Recipe.Checksum()))));
	TestTrue(TEXT("stored element order is preserved"),
		First.Find(TEXT("space:yard:0")) < First.Find(TEXT("space:room:0")));
	TestTrue(TEXT("all physical element arrays are exported"),
		First.Contains(TEXT("\"connections\":[{"))
		&& First.Contains(TEXT("\"visibility\":[{"))
		&& First.Contains(TEXT("\"blockers\":[{"))
		&& First.Contains(TEXT("\"placementSlots\":[{")));
	TestTrue(TEXT("evidence coverage and ordered movement modes are exported"),
		First.Contains(TEXT("\"evidenceCoverage\":{\"placement\":true,"
			"\"traversalClearance\":true,\"visibility\":true,\"blockers\":true,"
			"\"sourceIds\":[\"source:coverage\"]}"))
		&& First.Contains(TEXT("\"movementModeIds\":[\"ground\",\"water-surface\"]")));
	TestTrue(TEXT("empty arrays and zero-valued unknown facts are preserved"),
		First.Contains(TEXT("\"maxStepHeight\":0"))
		&& First.Contains(TEXT("\"maxJumpDistance\":0"))
		&& First.Contains(TEXT("\"tags\":[]")));
	TestTrue(TEXT("semantic roles and provenance are preserved"),
		First.Contains(TEXT("\"semanticRole\":\"home\""))
		&& First.Contains(TEXT("\"sourceIds\":[\"site:village:0\",\"parcel:0\"]")));

	FGatersBuiltSiteRecipe Changed = Recipe;
	Changed.Spaces[0].Center.X += 1.f;
	FString ChangedJson;
	Diagnostics.Reset();
	TestTrue(TEXT("changed recipe serializes"),
		FGatersBuiltSiteRecipeJson::Serialize({Changed}, ChangedJson, Diagnostics));
	TestNotEqual(TEXT("changed checksum changes exported identity"),
		ChangedJson, First);
	TestTrue(TEXT("changed checksum is exported"),
		ChangedJson.Contains(FString::Printf(
			TEXT("\"checksum\":%llu"), static_cast<uint64>(Changed.Checksum()))));

	FString EmptyJson;
	Diagnostics.Reset();
	TestTrue(TEXT("empty catalog serializes"),
		FGatersBuiltSiteRecipeJson::Serialize({}, EmptyJson, Diagnostics));
	TestEqual(TEXT("empty catalog remains explicit"), EmptyJson,
		FString(TEXT("{\"exportVersion\":1,\"coordinateUnit\":\"centimetres\","
			"\"lengthUnit\":\"centimetres\",\"areaUnit\":\"square centimetres\","
			"\"siteRecipes\":[]}")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBuiltSiteRecipeJsonFileTest,
	"Gaters.BuiltSites.JsonExport.File",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBuiltSiteRecipeJsonFileTest::RunTest(const FString& Parameters)
{
	const FString Directory = FPaths::ProjectSavedDir() / TEXT("BuiltSiteJsonTests");
	IFileManager::Get().MakeDirectory(*Directory, true);
	const FString OutputPath = Directory / TEXT("catalog.json");
	const FString Original = TEXT("existing artifact");
	TestTrue(TEXT("test destination is prepared"),
		FFileHelper::SaveStringToFile(Original, *OutputPath));

	FGatersBuiltSiteRecipe Invalid = MakeJsonExportRecipe();
	Invalid.SiteArea = std::numeric_limits<float>::quiet_NaN();
	TArray<FString> Diagnostics;
	TestFalse(TEXT("invalid recipe is not saved"),
		FGatersBuiltSiteRecipeJson::Save({Invalid}, OutputPath, Diagnostics));
	FString Preserved;
	TestTrue(TEXT("existing destination remains readable"),
		FFileHelper::LoadFileToString(Preserved, *OutputPath));
	TestEqual(TEXT("invalid export does not replace destination"), Preserved, Original);
	TestTrue(TEXT("invalid export has causal diagnostics"),
		Diagnostics.ContainsByPredicate([](const FString& Diagnostic)
		{
			return Diagnostic.Contains(TEXT("site.dimensions"));
		}));

	const FGatersBuiltSiteRecipe Valid = MakeJsonExportRecipe();
	Diagnostics.Reset();
	TestTrue(TEXT("valid recipe is saved"),
		FGatersBuiltSiteRecipeJson::Save({Valid}, OutputPath, Diagnostics));
	FString Saved;
	FString Expected;
	TestTrue(TEXT("saved catalog is readable"),
		FFileHelper::LoadFileToString(Saved, *OutputPath));
	TestTrue(TEXT("expected catalog serializes"),
		FGatersBuiltSiteRecipeJson::Serialize({Valid}, Expected, Diagnostics));
	TestEqual(TEXT("saved bytes preserve serialized JSON"), Saved, Expected);
	TArray<uint8> Bytes;
	TestTrue(TEXT("saved catalog bytes are readable"),
		FFileHelper::LoadFileToArray(Bytes, *OutputPath));
	TestTrue(TEXT("saved catalog is UTF-8 without BOM"),
		Bytes.Num() >= 3 && !(Bytes[0] == 0xef && Bytes[1] == 0xbb && Bytes[2] == 0xbf));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBuiltSiteRecipeJsonGeneratedSettlementTest,
	"Gaters.BuiltSites.JsonExport.GeneratedSettlement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBuiltSiteRecipeJsonGeneratedSettlementTest::RunTest(const FString& Parameters)
{
	const FString OutputPath =
		FPaths::ProjectSavedDir() / TEXT("BuiltSiteJsonTests/generated-settlement.json");
	TArray<FString> Diagnostics;
	TestTrue(TEXT("current pure generators export a settlement"),
		FGatersBuiltSiteRecipeJson::GenerateSettlement(OutputPath, 73, 1, Diagnostics));
	for (const FString& Diagnostic : Diagnostics)
	{
		AddInfo(Diagnostic);
	}
	FString Json;
	TestTrue(TEXT("generated settlement artifact is readable"),
		FFileHelper::LoadFileToString(Json, *OutputPath));
	TestTrue(TEXT("generated catalog contains one settlement recipe"),
		Json.Contains(TEXT("\"siteRecipes\":[{"))
		&& Json.Contains(TEXT("\"siteId\":\"site:village:0\""))
		&& Json.Contains(TEXT("\"kind\":\"settlement\"")));
	TestTrue(TEXT("generated settlement exports complete physical evidence"),
		Json.Contains(TEXT("\"evidenceCoverage\":{\"placement\":true,"))
		&& Json.Contains(TEXT("\"traversalClearance\":true"))
		&& Json.Contains(TEXT("\"visibility\":true"))
		&& Json.Contains(TEXT("\"blockers\":true"))
		&& Json.Contains(TEXT("\"movementModeIds\":[\"ground\"]"))
		&& Json.Contains(TEXT("\"placementSlots\":[{"))
		&& Json.Contains(TEXT("\"visibility\":[{"))
		&& Json.Contains(TEXT("\"blockers\":[{"))
		&& !Json.Contains(TEXT("\"width\":0"))
		&& !Json.Contains(TEXT("\"headroom\":0")));
	TestTrue(TEXT("generated settlement exports no tactical role names"),
		!Json.Contains(TEXT("arrival")) && !Json.Contains(TEXT("extraction"))
		&& !Json.Contains(TEXT("objective")) && !Json.Contains(TEXT("guard"))
		&& !Json.Contains(TEXT("loot")) && !Json.Contains(TEXT("trap"))
		&& !Json.Contains(TEXT("spawn")) && !Json.Contains(TEXT("patrol")));
	UGatersBuiltSiteExportCommandlet* Commandlet =
		NewObject<UGatersBuiltSiteExportCommandlet>();
	TestEqual(TEXT("headless commandlet returns success"), Commandlet->Main(
		FString::Printf(TEXT("-Output=\"%s\" -Seed=73 -Stage=1"), *OutputPath)), 0);
	return true;
}

#endif
