#if WITH_DEV_AUTOMATION_TESTS

#include "GatersLegacyBaseLayer.h"
#include "Misc/AutomationTest.h"

#include <limits>

namespace
{
FGatersAssetContract MakeModuleContract(
	const FString& AssetId,
	const FString& ContentKey,
	const FVector& BoundsCenter,
	const FVector& BoundsExtent,
	const EGatersAssetContactSupport Support = EGatersAssetContactSupport::Attachment)
{
	FGatersAssetContract Contract;
	Contract.AssetId = AssetId;
	Contract.ContentKey = ContentKey;
	Contract.StyleId = TEXT("gaters.clean-midpoly-painted");
	Contract.BoundsCenter = BoundsCenter;
	Contract.BoundsExtent = BoundsExtent;
	Contract.ClearanceExtent = BoundsExtent;
	Contract.Collision = EGatersAssetCollision::Simple;
	Contract.RenderClass = EGatersAssetRenderClass::UniqueStatic;
	Contract.bInstanceStatePersistent = true;
	Contract.Contacts.Add({TEXT("support"),
		BoundsCenter - FVector(0.f, 0.f, BoundsExtent.Z), FVector::UpVector, Support});
	return Contract;
}

FGatersLegacyBaseModuleDefinition MakeModule(
	const FString& Tier,
	const FString& Piece,
	const FVector& Center,
	const FVector& Extent,
	const EGatersAssetContactSupport Support = EGatersAssetContactSupport::Attachment)
{
	FGatersLegacyBaseModuleDefinition Result;
	Result.bAvailable = true;
	Result.Contract = MakeModuleContract(
		FString::Printf(TEXT("fixture:%s:%s"), *Tier, *Piece),
		FString::Printf(TEXT("%s.%s"), *Tier, *Piece), Center, Extent, Support);
	return Result;
}

FGatersLegacyBaseTierDefinition MakeTier(const FString& Tier)
{
	FGatersLegacyBaseTierDefinition Result;
	Result.Id = Tier;
	Result.Foundation = MakeModule(Tier, TEXT("foundation"),
		FVector(0.f, 0.f, 50.f), FVector(150.f, 150.f, 50.f),
		EGatersAssetContactSupport::Terrain);
	Result.Wall = MakeModule(Tier, TEXT("wall"),
		FVector(0.f, 0.f, 150.f), FVector(150.f, 10.f, 150.f));
	Result.DoorFrame = MakeModule(Tier, TEXT("doorframe"),
		FVector(0.f, 0.f, 150.f), FVector(150.f, 10.f, 150.f));
	Result.Window = MakeModule(Tier, TEXT("window"),
		FVector(0.f, 0.f, 150.f), FVector(150.f, 10.f, 150.f));
	Result.Ceiling = MakeModule(Tier, TEXT("ceiling"),
		FVector(0.f, 0.f, 10.f), FVector(150.f, 150.f, 10.f));
	Result.Fence = MakeModule(Tier, TEXT("fence"),
		FVector(0.f, 0.f, 100.f), FVector(150.f, 10.f, 100.f),
		EGatersAssetContactSupport::Terrain);
	return Result;
}

FGatersLegacyBaseLayerInput MakeInput(const int32 RandomState = 73)
{
	FGatersLegacyBaseLayerInput Input;
	Input.BaseCenter = FVector2D(6000.f, -2500.f);
	Input.RandomState = RandomState;
	Input.SourceIds = {TEXT("base:0"), TEXT("terrain:fixture")};
	Input.Tiers = {MakeTier(TEXT("wood")), MakeTier(TEXT("stone")),
		MakeTier(TEXT("metal"))};
	Input.Door = MakeModule(TEXT("shared"), TEXT("door"),
		FVector(0.f, 0.f, 100.f), FVector(20.f, 100.f, 100.f));
	return Input;
}

bool HasDiagnostic(
	const FGatersLegacyBaseLayerResult& Result,
	const FString& RuleId)
{
	return Result.Diagnostics.ContainsByPredicate([&RuleId](const FString& Diagnostic)
	{
		return Diagnostic.StartsWith(RuleId);
	});
}

bool SamePhysicalFacts(
	const FGatersLegacyBaseLayerResult& A,
	const FGatersLegacyBaseLayerResult& B)
{
	if (A.Archetype != B.Archetype || A.MainTierId != B.MainTierId ||
		A.MainWidth != B.MainWidth || A.MainDepth != B.MainDepth ||
		A.MainStories != B.MainStories || A.BuildingCount != B.BuildingCount ||
		A.Pieces.Num() != B.Pieces.Num())
	{
		return false;
	}
	for (int32 Index = 0; Index < A.Pieces.Num(); ++Index)
	{
		const FGatersLegacyBasePieceRecipe& Left = A.Pieces[Index];
		const FGatersLegacyBasePieceRecipe& Right = B.Pieces[Index];
		if (Left.Id != Right.Id || Left.ContentKey != Right.ContentKey ||
			!Left.Transform.Equals(Right.Transform) || Left.SourceIds != Right.SourceIds)
		{
			return false;
		}
	}
	return true;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersLegacyBaseLayerContractTest,
	"Gaters.BuiltSites.LegacyBase.Contract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersLegacyBaseLayerContractTest::RunTest(const FString& Parameters)
{
	const auto FlatHeight = [](const FVector2D&) { return 100.f; };
	const FGatersLegacyBaseLayerResult Valid =
		FGatersLegacyBaseLayer::Generate(MakeInput(), FlatHeight);
	TestTrue(TEXT("valid explicit inputs generate a pure base layer"), Valid.IsValid());
	TestFalse(TEXT("valid layer emits physical pieces"), Valid.Pieces.IsEmpty());
	TestFalse(TEXT("valid layer emits used content requirements"),
		Valid.ContentRequirements.IsEmpty());
	TestEqual(TEXT("layer contract is versioned"), Valid.Version, 1);
	TestEqual(TEXT("generator contract is versioned"), Valid.GeneratorVersion, 1);

	auto ExpectRejected = [&](FGatersLegacyBaseLayerInput Input, const FString& RuleId)
	{
		const FGatersLegacyBaseLayerResult Result =
			FGatersLegacyBaseLayer::Generate(Input, FlatHeight);
		TestFalse(*FString::Printf(TEXT("%s rejects the layer"), *RuleId), Result.IsValid());
		TestTrue(*FString::Printf(TEXT("%s emits no partial pieces"), *RuleId),
			Result.Pieces.IsEmpty());
		TestTrue(*FString::Printf(TEXT("%s is causal"), *RuleId),
			HasDiagnostic(Result, RuleId));
	};

	FGatersLegacyBaseLayerInput WrongVersion = MakeInput();
	WrongVersion.Version = 2;
	ExpectRejected(WrongVersion, TEXT("legacy-base.input.version"));

	FGatersLegacyBaseLayerInput MissingTier = MakeInput();
	MissingTier.Tiers.Pop();
	ExpectRejected(MissingTier, TEXT("legacy-base.input.tiers"));

	FGatersLegacyBaseLayerInput MissingModule = MakeInput();
	MissingModule.Tiers[0].Foundation.bAvailable = false;
	ExpectRejected(MissingModule, TEXT("legacy-base.input.module"));

	FGatersLegacyBaseLayerInput InvalidBounds = MakeInput();
	InvalidBounds.Tiers[0].Wall.Contract.BoundsExtent.X = 0.f;
	ExpectRejected(InvalidBounds, TEXT("legacy-base.input.module"));

	FGatersLegacyBaseLayerInput DuplicateContent = MakeInput();
	DuplicateContent.Tiers[1].Wall.Contract.ContentKey =
		DuplicateContent.Tiers[0].Wall.Contract.ContentKey;
	ExpectRejected(DuplicateContent, TEXT("legacy-base.input.duplicate-content"));

	FGatersLegacyBaseLayerInput NonFinite = MakeInput();
	NonFinite.BaseCenter.X = std::numeric_limits<double>::quiet_NaN();
	ExpectRejected(NonFinite, TEXT("legacy-base.input.dimensions"));

	FGatersLegacyBaseLayerInput MissingProvenance = MakeInput();
	MissingProvenance.SourceIds.Reset();
	ExpectRejected(MissingProvenance, TEXT("legacy-base.input.provenance"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersLegacyBaseLayerTopologyTest,
	"Gaters.BuiltSites.LegacyBase.Topology",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersLegacyBaseLayerTopologyTest::RunTest(const FString& Parameters)
{
	const auto FlatHeight = [](const FVector2D&) { return 100.f; };
	bool bSawArchetype[3] = {false, false, false};
	bool bSawDifferentFacts = false;
	FGatersLegacyBaseLayerResult First;
	FGatersLegacyBaseLayerResult Compound;
	for (int32 RandomState = 0; RandomState < 256; ++RandomState)
	{
		const FGatersLegacyBaseLayerInput Input = MakeInput(RandomState);
		const FGatersLegacyBaseLayerResult Result =
			FGatersLegacyBaseLayer::Generate(Input, FlatHeight);
		const FGatersLegacyBaseLayerResult Repeated =
			FGatersLegacyBaseLayer::Generate(Input, FlatHeight);
		TestTrue(TEXT("every sampled state is valid on flat terrain"), Result.IsValid());
		TestTrue(TEXT("a copied random state repeats identical physical facts"),
			SamePhysicalFacts(Result, Repeated));
		if (RandomState == 0)
		{
			First = Result;
		}
		else
		{
			bSawDifferentFacts |= !SamePhysicalFacts(First, Result);
		}

		const int32 ArchetypeIndex = static_cast<int32>(Result.Archetype);
		if (ArchetypeIndex >= 0 && ArchetypeIndex < 3)
		{
			bSawArchetype[ArchetypeIndex] = true;
		}
		if (Result.Archetype == EGatersLegacyBaseArchetype::Compound && Compound.Pieces.IsEmpty())
		{
			Compound = Result;
		}

		TSet<FString> PieceContent;
		for (int32 PieceIndex = 0; PieceIndex < Result.Pieces.Num(); ++PieceIndex)
		{
			const FGatersLegacyBasePieceRecipe& Piece = Result.Pieces[PieceIndex];
			TestEqual(TEXT("piece IDs follow stable output order"), Piece.Id,
				FString::Printf(TEXT("piece:%d"), PieceIndex));
			TestTrue(TEXT("piece keeps base provenance"), Piece.SourceIds.Contains(TEXT("base:0")));
			TestTrue(TEXT("piece keeps terrain provenance"),
				Piece.SourceIds.Contains(TEXT("terrain:fixture")));
			TestTrue(TEXT("piece keeps its module source"), Piece.SourceIds.ContainsByPredicate(
				[&Piece](const FString& SourceId) { return SourceId.StartsWith(TEXT("fixture:")); }));
			TestFalse(TEXT("physical output contains no raid or loot semantics"),
				Piece.ContentKey.Contains(TEXT("loot"), ESearchCase::IgnoreCase) ||
				Piece.ContentKey.Contains(TEXT("raid"), ESearchCase::IgnoreCase) ||
				Piece.ContentKey.Contains(TEXT("objective"), ESearchCase::IgnoreCase) ||
				Piece.ContentKey.Contains(TEXT("tactical"), ESearchCase::IgnoreCase));
			PieceContent.Add(Piece.ContentKey);
		}
		TSet<FString> RequirementContent;
		for (const FGatersAssetContract& Requirement : Result.ContentRequirements)
		{
			TestFalse(TEXT("used content requirements are unique"),
				RequirementContent.Contains(Requirement.ContentKey));
			RequirementContent.Add(Requirement.ContentKey);
		}
		TestTrue(TEXT("requirements contain exactly the used content keys"),
			PieceContent.Includes(RequirementContent) && RequirementContent.Includes(PieceContent));
	}
	TestTrue(TEXT("sampled states reach hut"),
		bSawArchetype[static_cast<int32>(EGatersLegacyBaseArchetype::Hut)]);
	TestTrue(TEXT("sampled states reach compound"),
		bSawArchetype[static_cast<int32>(EGatersLegacyBaseArchetype::Compound)]);
	TestTrue(TEXT("sampled states reach tower"),
		bSawArchetype[static_cast<int32>(EGatersLegacyBaseArchetype::Tower)]);
	TestTrue(TEXT("changing the random state can change topology"), bSawDifferentFacts);
	TestFalse(TEXT("compound evidence includes physical pieces"), Compound.Pieces.IsEmpty());

	const auto MildSlope = [](const FVector2D& Point) { return static_cast<float>(Point.X * 0.01); };
	const FGatersLegacyBaseLayerResult Sloped =
		FGatersLegacyBaseLayer::Generate(MakeInput(73), MildSlope);
	TestTrue(TEXT("a mild terrain slope remains buildable"), Sloped.IsValid());
	TSet<int32> FoundationScaleMillimeters;
	for (const FGatersLegacyBasePieceRecipe& Piece : Sloped.Pieces)
	{
		if (Piece.ContentKey.EndsWith(TEXT(".foundation")))
		{
			FoundationScaleMillimeters.Add(FMath::RoundToInt(Piece.Transform.GetScale3D().Z * 1000.f));
		}
	}
	TestTrue(TEXT("foundation skirts respond independently to terrain"),
		FoundationScaleMillimeters.Num() > 1);

	const auto ExcessiveDrop = [](const FVector2D& Point) { return static_cast<float>(Point.X); };
	const FGatersLegacyBaseLayerResult Rejected =
		FGatersLegacyBaseLayer::Generate(MakeInput(), ExcessiveDrop);
	TestFalse(TEXT("excessive footprint drop rejects the layer"), Rejected.IsValid());
	TestTrue(TEXT("fit rejection leaves no partial pieces"), Rejected.Pieces.IsEmpty());
	TestTrue(TEXT("fit rejection is causal"),
		HasDiagnostic(Rejected, TEXT("legacy-base.fit.foundation-drop")));

	const auto NonFiniteHeight = [](const FVector2D&) {
		return std::numeric_limits<float>::quiet_NaN();
	};
	const FGatersLegacyBaseLayerResult InvalidTerrain =
		FGatersLegacyBaseLayer::Generate(MakeInput(), NonFiniteHeight);
	TestFalse(TEXT("non-finite terrain evidence rejects the layer"), InvalidTerrain.IsValid());
	TestTrue(TEXT("non-finite terrain rejection leaves no partial pieces"),
		InvalidTerrain.Pieces.IsEmpty());
	TestTrue(TEXT("non-finite terrain rejection is causal"),
		HasDiagnostic(InvalidTerrain, TEXT("legacy-base.terrain.non-finite")));

	const auto UnusableFiniteHeight = [](const FVector2D&) {
		return std::numeric_limits<float>::max();
	};
	const FGatersLegacyBaseLayerResult InvalidRange =
		FGatersLegacyBaseLayer::Generate(MakeInput(), UnusableFiniteHeight);
	TestFalse(TEXT("terrain that cannot produce finite physical transforms is rejected"),
		InvalidRange.IsValid());
	TestTrue(TEXT("invalid terrain range leaves no partial pieces"),
		InvalidRange.Pieces.IsEmpty());
	TestTrue(TEXT("invalid terrain range rejection is causal"),
		HasDiagnostic(InvalidRange, TEXT("legacy-base.terrain.range")));
	return true;
}

#endif
