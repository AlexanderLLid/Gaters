#if WITH_DEV_AUTOMATION_TESTS

#include "GatersBuiltSiteRecipe.h"
#include "Misc/AutomationTest.h"

#include <limits>

namespace
{
FGatersBuiltSiteRecipe ValidBuiltSiteRecipe()
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
	Yard.Tags = {TEXT("outdoors"), TEXT("yard")};
	Yard.SourceIds = {Recipe.SiteId};

	FGatersBuiltSiteSpace& Room = Recipe.Spaces.AddDefaulted_GetRef();
	Room.Id = TEXT("space:room:0");
	Room.Center = FVector(500.f, 0.f, 130.f);
	Room.Extent = FVector(125.f, 125.f, 130.f);
	Room.Tags = {TEXT("indoors"), TEXT("roofed"), TEXT("room")};
	Room.SourceIds = {TEXT("settlement:building:home:0")};

	FGatersBuiltSiteConnection& Enter = Recipe.Connections.AddDefaulted_GetRef();
	Enter.Id = TEXT("connection:yard-room:0");
	Enter.FromSpaceId = Yard.Id;
	Enter.ToSpaceId = Room.Id;
	Enter.Width = 250.f;
	Enter.Headroom = 260.f;
	Enter.MovementModeIds = {TEXT("ground")};
	Enter.SourceIds = {TEXT("settlement:building:home:0:module:door:0")};

	FGatersBuiltSitePlacementSlot& Slot = Recipe.PlacementSlots.AddDefaulted_GetRef();
	Slot.Id = TEXT("slot:room:0");
	Slot.SpaceId = Room.Id;
	Slot.Location = Room.Center;
	Slot.ClearanceRadius = 50.f;
	Slot.ClearanceHeight = 180.f;
	Slot.Tags = {TEXT("indoors"), TEXT("grounded")};
	Slot.SourceIds = {TEXT("settlement:building:home:0")};
	return Recipe;
}

bool HasRule(const TArray<FGatersBuiltSiteIssue>& Issues, const FString& RuleId)
{
	return Issues.ContainsByPredicate([&RuleId](const FGatersBuiltSiteIssue& Issue)
	{
		return Issue.RuleId == RuleId;
	});
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBuiltSiteRecipeContractTest,
	"Gaters.BuiltSites.Recipe.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBuiltSiteRecipeContractTest::RunTest(const FString& Parameters)
{
	const FGatersBuiltSiteRecipe Recipe = ValidBuiltSiteRecipe();
	TArray<FGatersBuiltSiteIssue> Issues;
	TestTrue(TEXT("physical site recipe validates"), Recipe.Validate(Issues));
	TestTrue(TEXT("valid recipe has no diagnostics"), Issues.IsEmpty());
	TestEqual(TEXT("contract is versioned"), Recipe.ContractVersion, 1);
	TestEqual(TEXT("site definition is versioned"), Recipe.SiteVersion, 1);
	TestTrue(TEXT("missing visibility evidence is valid"), Recipe.Visibility.IsEmpty());
	TestTrue(TEXT("missing blocker evidence is valid"), Recipe.Blockers.IsEmpty());
	TestTrue(TEXT("complete-empty categories remain explicit"),
		Recipe.EvidenceCoverage.bVisibility && Recipe.EvidenceCoverage.bBlockers);
	TestNotNull(TEXT("stable space lookup works"), Recipe.FindSpace(TEXT("space:room:0")));
	TestEqual(TEXT("fixed recipe preserves canonical identity"),
		Recipe.CanonicalText(), ValidBuiltSiteRecipe().CanonicalText());
	TestEqual(TEXT("fixed recipe preserves checksum"),
		Recipe.Checksum(), ValidBuiltSiteRecipe().Checksum());

	FGatersBuiltSiteRecipe Moved = Recipe;
	Moved.PlacementSlots[0].Location.X += 1.f;
	TestNotEqual(TEXT("physical slot movement changes identity"),
		Moved.Checksum(), Recipe.Checksum());

	FGatersBuiltSiteRecipe DelimiterInValue = Recipe;
	DelimiterInValue.Spaces[0].Tags = {TEXT("a;space-tag=b")};
	FGatersBuiltSiteRecipe DelimiterBetweenValues = Recipe;
	DelimiterBetweenValues.Spaces[0].Tags = {TEXT("a"), TEXT("b")};
	TestNotEqual(TEXT("string boundaries preserve canonical identity"),
		DelimiterInValue.CanonicalText(), DelimiterBetweenValues.CanonicalText());
	TestNotEqual(TEXT("string boundaries preserve checksum identity"),
		DelimiterInValue.Checksum(), DelimiterBetweenValues.Checksum());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBuiltSiteRecipeCounterexampleTest,
	"Gaters.BuiltSites.Recipe.Counterexamples",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBuiltSiteRecipeCounterexampleTest::RunTest(const FString& Parameters)
{
	FGatersBuiltSiteRecipe Invalid = ValidBuiltSiteRecipe();
	Invalid.SiteId.Reset();
	Invalid.Blockers.AddDefaulted_GetRef().Id = Invalid.Spaces[0].Id;
	Invalid.Connections[0].ToSpaceId = TEXT("space:missing");
	Invalid.Connections[0].Width = -1.f;
	Invalid.Connections[0].BlockerIds.Add(TEXT("blocker:missing"));
	Invalid.Connections[0].SourceIds.Reset();
	Invalid.PlacementSlots[0].Location = FVector(5000.f, 0.f, 0.f);

	TArray<FGatersBuiltSiteIssue> Issues;
	TestFalse(TEXT("invalid physical graph is rejected"), Invalid.Validate(Issues));
	TestTrue(TEXT("missing site identity is causal"), HasRule(Issues, TEXT("site.identity")));
	TestTrue(TEXT("duplicate element identity is causal"),
		HasRule(Issues, TEXT("site.duplicate-id")));
	TestTrue(TEXT("broken graph references are causal"),
		HasRule(Issues, TEXT("site.reference")));
	TestTrue(TEXT("negative physical dimensions are causal"),
		HasRule(Issues, TEXT("site.dimensions")));
	TestTrue(TEXT("slot containment is causal"),
		HasRule(Issues, TEXT("site.containment")));
	TestTrue(TEXT("missing provenance is causal"),
		HasRule(Issues, TEXT("site.provenance")));

	FGatersBuiltSiteRecipe NonFinite = ValidBuiltSiteRecipe();
	NonFinite.SiteArea = std::numeric_limits<float>::quiet_NaN();
	Issues.Reset();
	TestFalse(TEXT("non-finite physical facts are rejected"), NonFinite.Validate(Issues));
	TestTrue(TEXT("non-finite physical dimensions are causal"),
		HasRule(Issues, TEXT("site.dimensions")));

	FGatersBuiltSiteRecipe ZeroClearance = ValidBuiltSiteRecipe();
	ZeroClearance.PlacementSlots[0].ClearanceRadius = 0.f;
	Issues.Reset();
	TestFalse(TEXT("unusable placement clearance is rejected"),
		ZeroClearance.Validate(Issues));
	TestTrue(TEXT("unusable placement clearance is causal"),
		HasRule(Issues, TEXT("site.dimensions")));

	FGatersBuiltSiteRecipe MissingMovementMode = ValidBuiltSiteRecipe();
	MissingMovementMode.Connections[0].MovementModeIds.Reset();
	Issues.Reset();
	TestFalse(TEXT("connection without a movement mode is rejected"),
		MissingMovementMode.Validate(Issues));
	TestTrue(TEXT("missing movement mode is causal"),
		HasRule(Issues, TEXT("site.movement-mode")));

	FGatersBuiltSiteRecipe DuplicateMovementMode = ValidBuiltSiteRecipe();
	DuplicateMovementMode.Connections[0].MovementModeIds = {TEXT("ground"), TEXT("ground")};
	Issues.Reset();
	TestFalse(TEXT("duplicate movement mode is rejected"),
		DuplicateMovementMode.Validate(Issues));
	TestTrue(TEXT("duplicate movement mode is causal"),
		HasRule(Issues, TEXT("site.movement-mode")));

	FGatersBuiltSiteRecipe MissingCoverageSource = ValidBuiltSiteRecipe();
	MissingCoverageSource.EvidenceCoverage.SourceIds.Reset();
	Issues.Reset();
	TestFalse(TEXT("measured coverage without provenance is rejected"),
		MissingCoverageSource.Validate(Issues));
	TestTrue(TEXT("missing coverage provenance is causal"),
		HasRule(Issues, TEXT("site.evidence.provenance")));

	FGatersBuiltSiteRecipe UnknownCoveredClearance = ValidBuiltSiteRecipe();
	UnknownCoveredClearance.Connections[0].Width = 0.f;
	Issues.Reset();
	TestFalse(TEXT("covered traversal requires positive clearance"),
		UnknownCoveredClearance.Validate(Issues));
	TestTrue(TEXT("covered traversal failure is causal"),
		HasRule(Issues, TEXT("site.evidence.traversal")));
	return true;
}

#endif
