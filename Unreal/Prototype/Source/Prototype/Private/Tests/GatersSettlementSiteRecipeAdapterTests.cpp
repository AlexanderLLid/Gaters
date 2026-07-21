#if WITH_DEV_AUTOMATION_TESTS

#include "GatersSettlementSiteRecipeAdapter.h"
#include "Misc/AutomationTest.h"

namespace
{
FGatersTerrainSemanticField MakeSiteRecipeField()
{
	FGatersTerrainSemanticField Field;
	Field.CellsPerAxis = 31;
	Field.CellSize = 500.f;
	Field.Cells.SetNum(Field.CellsPerAxis * Field.CellsPerAxis);
	for (int32 X = 0; X < Field.CellsPerAxis; ++X)
	{
		for (int32 Y = 0; Y < Field.CellsPerAxis; ++Y)
		{
			FGatersTerrainSemanticSample& Sample = Field.Cells[X * Field.CellsPerAxis + Y];
			Sample.Type = EGatersTerrainSemantic::Flat;
			Sample.Height = X * 2.f + Y;
		}
	}
	return Field;
}

FVector SiteRecipePathLocation(
	const FGatersTerrainSemanticField& Field,
	const FIntPoint& Cell)
{
	const int32 Half = Field.CellsPerAxis / 2;
	return FVector(
		(Cell.X - Half) * Field.CellSize,
		(Cell.Y - Half) * Field.CellSize,
		Field.At(Cell.X, Cell.Y).Height);
}

FGatersPlannedSite MakeSiteRecipeVillage(const FGatersTerrainSemanticField& Field)
{
	const FIntPoint Cell(Field.CellsPerAxis / 2, Field.CellsPerAxis / 2);
	return {TEXT("site:village:0"), EGatersPlannedSiteKind::Village, Cell,
		FVector(0.f, 0.f, Field.At(Cell.X, Cell.Y).Height)};
}

bool AllElementsHaveProvenance(const FGatersBuiltSiteRecipe& Recipe)
{
	for (const FGatersBuiltSiteSpace& Space : Recipe.Spaces)
	{
		if (Space.SourceIds.IsEmpty()) return false;
	}
	for (const FGatersBuiltSiteConnection& Connection : Recipe.Connections)
	{
		if (Connection.SourceIds.IsEmpty()) return false;
	}
	for (const FGatersBuiltSitePlacementSlot& Slot : Recipe.PlacementSlots)
	{
		if (Slot.SourceIds.IsEmpty()) return false;
	}
	for (const FGatersBuiltSiteVisibility& Sight : Recipe.Visibility)
	{
		if (Sight.SourceIds.IsEmpty()) return false;
	}
	for (const FGatersBuiltSiteBlocker& Blocker : Recipe.Blockers)
	{
		if (Blocker.SourceIds.IsEmpty()) return false;
	}
	return true;
}

bool AllIndoorSlotsReachPathNetwork(const FGatersBuiltSiteRecipe& Recipe)
{
	TSet<FString> Reachable;
	for (const FGatersBuiltSiteSpace& Space : Recipe.Spaces)
	{
		if (Space.Tags.Contains(TEXT("path-centerline")))
		{
			Reachable.Add(Space.Id);
		}
	}
	bool bChanged = true;
	while (bChanged)
	{
		bChanged = false;
		for (const FGatersBuiltSiteConnection& Connection : Recipe.Connections)
		{
			if (Reachable.Contains(Connection.FromSpaceId)
				&& !Reachable.Contains(Connection.ToSpaceId))
			{
				Reachable.Add(Connection.ToSpaceId);
				bChanged = true;
			}
		}
	}
	for (const FGatersBuiltSitePlacementSlot& Slot : Recipe.PlacementSlots)
	{
		const FGatersBuiltSiteSpace* Space = Recipe.FindSpace(Slot.SpaceId);
		if (Space && Space->Tags.Contains(TEXT("indoors")) && !Reachable.Contains(Space->Id))
		{
			return false;
		}
	}
	return true;
}

int32 ExpectedDirectedConnections(const FGatersSettlementPlan& Plan)
{
	TSet<FIntPoint> Paths(Plan.PathCells);
	int32 Result = Plan.Buildings.Num() * 2;
	for (const FIntPoint& Cell : Plan.PathCells)
	{
		Result += Paths.Contains(Cell + FIntPoint(1, 0)) ? 2 : 0;
		Result += Paths.Contains(Cell + FIntPoint(0, 1)) ? 2 : 0;
	}
	return Result;
}

bool EarlierFactsAreStable(
	const FGatersBuiltSiteRecipe& Earlier,
	const FGatersBuiltSiteRecipe& Grown)
{
	for (const FGatersBuiltSiteSpace& Space : Earlier.Spaces)
	{
		const FGatersBuiltSiteSpace* Match = Grown.FindSpace(Space.Id);
		if (!Match || Match->Center != Space.Center || Match->Extent != Space.Extent
			|| Match->SemanticRole != Space.SemanticRole || Match->Tags != Space.Tags
			|| Match->SourceIds != Space.SourceIds)
		{
			return false;
		}
	}
	for (const FGatersBuiltSiteConnection& Connection : Earlier.Connections)
	{
		const FGatersBuiltSiteConnection* Match = Grown.Connections.FindByPredicate(
			[&Connection](const FGatersBuiltSiteConnection& Candidate)
			{
				return Candidate.Id == Connection.Id;
			});
		if (!Match || Match->FromSpaceId != Connection.FromSpaceId
			|| Match->ToSpaceId != Connection.ToSpaceId || Match->Width != Connection.Width
			|| Match->Headroom != Connection.Headroom
			|| Match->MaxStepHeight != Connection.MaxStepHeight
			|| Match->MaxJumpDistance != Connection.MaxJumpDistance
			|| Match->MovementModeIds != Connection.MovementModeIds
			|| Match->BlockerIds != Connection.BlockerIds || Match->Tags != Connection.Tags
			|| Match->SourceIds != Connection.SourceIds)
		{
			return false;
		}
	}
	for (const FGatersBuiltSitePlacementSlot& Slot : Earlier.PlacementSlots)
	{
		const FGatersBuiltSitePlacementSlot* Match = Grown.PlacementSlots.FindByPredicate(
			[&Slot](const FGatersBuiltSitePlacementSlot& Candidate)
			{
				return Candidate.Id == Slot.Id;
			});
		if (!Match || Match->SpaceId != Slot.SpaceId || Match->Location != Slot.Location
			|| Match->ClearanceRadius != Slot.ClearanceRadius
			|| Match->ClearanceHeight != Slot.ClearanceHeight || Match->Tags != Slot.Tags
			|| Match->SourceIds != Slot.SourceIds)
		{
			return false;
		}
	}
	return true;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersSettlementSiteRecipeAdapterContractTest,
	"Gaters.BuiltSites.SettlementAdapter.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersSettlementSiteRecipeAdapterContractTest::RunTest(const FString& Parameters)
{
	const FGatersTerrainSemanticField Field = MakeSiteRecipeField();
	const FGatersPlannedSite Site = MakeSiteRecipeVillage(Field);
	const FGatersSettlementPlan Hamlet =
		FGatersSettlementGenerator::Generate(Field, 73, Site, 0);
	const FGatersSettlementPlan Village =
		FGatersSettlementGenerator::Generate(Field, 73, Site, 1);
	const FGatersSettlementEvidenceSettings Settings =
		FGatersSettlementEvidenceSettings::Ground();
	const FGatersSettlementSiteRecipeCompilation A =
		FGatersSettlementSiteRecipeAdapter::Compile(Field, 73, Hamlet, Settings);
	const FGatersSettlementSiteRecipeCompilation B =
		FGatersSettlementSiteRecipeAdapter::Compile(Field, 73, Village, Settings);
	const FGatersSettlementSiteRecipeCompilation Repeat =
		FGatersSettlementSiteRecipeAdapter::Compile(Field, 73, Village, Settings);

	TestTrue(TEXT("accepted hamlet compiles to a physical site recipe"), A.bCompiled);
	TestTrue(TEXT("accepted village compiles to a physical site recipe"), B.bCompiled);
	TestEqual(TEXT("fixed inputs preserve the physical recipe checksum"),
		B.Recipe.Checksum(), Repeat.Recipe.Checksum());
	TestEqual(TEXT("site identity is preserved"), B.Recipe.SiteId, Village.SiteId);
	TestEqual(TEXT("site seed is preserved"), B.Recipe.Seed, 73);
	TestEqual(TEXT("ground evidence settings are versioned"), Settings.Version, 1);
	TestTrue(TEXT("generated geometry evidence coverage is explicit"),
		B.Recipe.EvidenceCoverage.bPlacement
		&& B.Recipe.EvidenceCoverage.bTraversalClearance
		&& B.Recipe.EvidenceCoverage.bVisibility
		&& B.Recipe.EvidenceCoverage.bBlockers);
	TestTrue(TEXT("adapter proves neutral placement slots"),
		!B.Recipe.PlacementSlots.IsEmpty());
	TestTrue(TEXT("adapter proves physical blockers"), !B.Recipe.Blockers.IsEmpty());
	TestEqual(TEXT("every clear connection has directed visibility evidence"),
		B.Recipe.Visibility.Num(), B.Recipe.Connections.Num());
	TestTrue(TEXT("every emitted physical fact has provenance"),
		AllElementsHaveProvenance(B.Recipe));

	bool bPathFactsAreTruthful = true;
	for (const FIntPoint& Cell : Village.PathCells)
	{
		const FGatersBuiltSiteSpace* Space =
			B.Recipe.FindSpace(FGatersSettlementPlan::StablePathId(Cell));
		bPathFactsAreTruthful &= Space
			&& Space->Center == SiteRecipePathLocation(Field, Cell)
				+ FVector(0.f, 0.f, Settings.OutdoorHeadroom * 0.5f)
			&& Space->Extent.X == Settings.PathWidth * 0.5f
			&& Space->Extent.Y == Settings.PathWidth * 0.5f
			&& Space->Tags.Contains(TEXT("path-centerline"));
	}
	TestTrue(TEXT("path spaces expose positive generated corridors"),
		bPathFactsAreTruthful);

	bool bBuildingFactsAreTruthful = true;
	for (const FGatersSettlementBuilding& Building : Village.Buildings)
	{
		const FGatersBuiltSiteSpace* Space = B.Recipe.FindSpace(Building.Id + TEXT(":space"));
		bBuildingFactsAreTruthful &= Space && !Space->SemanticRole.IsEmpty()
			&& Space->Tags.Contains(TEXT("usable-space"))
			&& Space->Tags.Contains(TEXT("indoors"))
			&& !Space->Tags.ContainsByPredicate([](const FString& Tag)
			{
				return Tag.StartsWith(TEXT("role."));
			});
	}
	TestTrue(TEXT("building spaces separate semantic role from physical envelope facts"),
		bBuildingFactsAreTruthful);

	bool bClearanceIsProved = true;
	for (const FGatersBuiltSiteConnection& Connection : B.Recipe.Connections)
	{
		const FGatersBuiltSiteSpace* From = B.Recipe.FindSpace(Connection.FromSpaceId);
		const FGatersBuiltSiteSpace* To = B.Recipe.FindSpace(Connection.ToSpaceId);
		bClearanceIsProved &= From && To && Connection.Width > 0.f
			&& Connection.Headroom > 0.f
			&& Connection.MovementModeIds == TArray<FString>{TEXT("ground")}
			&& FVector::Distance(From->Center, To->Center) <= Settings.MaxConnectionLength + 0.1f;
	}
	TestTrue(TEXT("adapter proves bounded ground traversal clearance"), bClearanceIsProved);
	TestTrue(TEXT("every indoor neutral slot reaches the path network"),
		AllIndoorSlotsReachPathNetwork(B.Recipe));

	bool bNoTacticalTags = true;
	for (const FGatersBuiltSitePlacementSlot& Slot : B.Recipe.PlacementSlots)
	{
		for (const FString& Tag : Slot.Tags)
		{
			bNoTacticalTags &= !Tag.Contains(TEXT("arrival"))
				&& !Tag.Contains(TEXT("extraction")) && !Tag.Contains(TEXT("objective"))
				&& !Tag.Contains(TEXT("guard")) && !Tag.Contains(TEXT("loot"))
				&& !Tag.Contains(TEXT("trap")) && !Tag.Contains(TEXT("spawn"))
				&& !Tag.Contains(TEXT("patrol"));
		}
	}
	TestTrue(TEXT("neutral slots contain no tactical inference"), bNoTacticalTags);
	TestTrue(TEXT("growth preserves all earlier emitted facts"),
		EarlierFactsAreStable(A.Recipe, B.Recipe));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersSettlementSiteRecipeAdapterCounterexampleTest,
	"Gaters.BuiltSites.SettlementAdapter.Counterexample",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersSettlementSiteRecipeAdapterCounterexampleTest::RunTest(const FString& Parameters)
{
	const FGatersTerrainSemanticField Field = MakeSiteRecipeField();
	FGatersSettlementPlan Invalid = FGatersSettlementGenerator::Generate(
		Field, 73, MakeSiteRecipeVillage(Field), 0);
	Invalid.Parcels.Reset();
	const FGatersSettlementSiteRecipeCompilation Compilation =
		FGatersSettlementSiteRecipeAdapter::Compile(Field, 73, Invalid);
	TestFalse(TEXT("invalid settlement emits no physical recipe"), Compilation.bCompiled);
	TestTrue(TEXT("source settlement diagnostic remains causal"),
		Compilation.Diagnostics.ContainsByPredicate([](const FString& Diagnostic)
		{
			return Diagnostic.Contains(TEXT("settlement.parcel"));
		}));

	const FGatersSettlementPlan Valid = FGatersSettlementGenerator::Generate(
		Field, 73, MakeSiteRecipeVillage(Field), 0);
	FGatersSettlementEvidenceSettings InvalidSettings =
		FGatersSettlementEvidenceSettings::Ground();
	InvalidSettings.MaxConnectionLength = 0.f;
	const FGatersSettlementSiteRecipeCompilation InvalidEvidence =
		FGatersSettlementSiteRecipeAdapter::Compile(Field, 73, Valid, InvalidSettings);
	TestFalse(TEXT("invalid physical evidence settings emit no recipe"),
		InvalidEvidence.bCompiled);
	TestTrue(TEXT("invalid evidence settings have causal diagnostics"),
		InvalidEvidence.Diagnostics.ContainsByPredicate([](const FString& Diagnostic)
		{
			return Diagnostic.Contains(TEXT("site.evidence.settings"));
		}));

	FGatersSettlementPlan BlockedDoor = Valid;
	BlockedDoor.Buildings[0].Yaw += 180.f;
	const FGatersSettlementSiteRecipeCompilation BlockedAccess =
		FGatersSettlementSiteRecipeAdapter::Compile(Field, 73, BlockedDoor);
	TestFalse(TEXT("one blocked required doorway rejects the complete recipe"),
		BlockedAccess.bCompiled);
	TestTrue(TEXT("blocked doorway diagnostic names its building and physical relation"),
		BlockedAccess.Diagnostics.ContainsByPredicate([&BlockedDoor](const FString& Diagnostic)
		{
			return Diagnostic.Contains(TEXT("site.connection.blocked"))
				&& Diagnostic.Contains(BlockedDoor.Buildings[0].Id)
				&& Diagnostic.Contains(TEXT("connection:"));
		}));
	return true;
}

#endif
