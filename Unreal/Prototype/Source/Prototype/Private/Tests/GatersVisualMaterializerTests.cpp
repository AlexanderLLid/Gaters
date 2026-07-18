#if WITH_DEV_AUTOMATION_TESTS

#include "GatersVisualMaterializer.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersVisualMaterializerPlanTest,
	"Gaters.Runtime.VisualMaterializer.Plan",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersVisualMaterializerPlanTest::RunTest(const FString& Parameters)
{
	FGatersCompiledWorld World;
	World.Nodes = {
		{TEXT("scatter:10"), EGatersRecipeNodeKind::ScatterTree,
			FTransform(FVector(100, 200, 300)), TEXT("environment.tree")},
		{TEXT("content:7:4:-3:11"), EGatersRecipeNodeKind::ScatterRock,
			FTransform(FVector(400, 500, 600)), TEXT("environment.rock")},
		{TEXT("plot:2"), EGatersRecipeNodeKind::BuildPlot,
			FTransform(FVector(700, 800, 900))},
		{TEXT("plot:3"), EGatersRecipeNodeKind::BuildPlot,
			FTransform(FVector(1000, 1100, 1200))}
	};
	World.Nodes[0].Representation = EGatersCompiledRepresentation::InstancedStatic;
	World.Nodes[1].Representation = EGatersCompiledRepresentation::InstancedStatic;
	UStaticMesh* CatalogTree = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	World.Nodes[0].Mesh = CatalogTree;

	const FGatersVisualBatchPlan Plan = FGatersVisualMaterializer::Plan(
		World, {TEXT("chop:content:7:4:-3:11"), TEXT("claim:plot:3")});

	TestEqual(TEXT("surviving tree is batched"), Plan.Trees.Num(), 1);
	TestEqual(TEXT("chopped rock is not rendered"), Plan.Rocks.Num(), 0);
	TestEqual(TEXT("open plot uses open batch"), Plan.OpenClaims.Num(), 1);
	TestEqual(TEXT("claimed plot uses claimed batch"), Plan.ClaimedClaims.Num(), 1);
	TestEqual(TEXT("plan retains every visible instance"), Plan.NumInstances(), 3);
	TestEqual(TEXT("stable scatter identity is retained"), Plan.Trees[0].StableId,
		FString(TEXT("scatter:10")));
	TestTrue(TEXT("catalog-selected tree mesh reaches the batch plan"), Plan.TreeMesh.Get() == CatalogTree);
	TestTrue(TEXT("materializer preserves the compiled scatter transform"),
		Plan.Trees[0].Transform.Equals(World.Nodes[0].Transform));
	TestEqual(TEXT("stable plot identity is retained"), Plan.ClaimedClaims[0].StableId,
		FString(TEXT("plot:3")));
	TestEqual(TEXT("claim marker receives its visual lift"),
		Plan.OpenClaims[0].Transform.GetLocation(), FVector(700, 800, 920));

	AActor* Owner = NewObject<AActor>();
	USceneComponent* Root = NewObject<USceneComponent>(Owner, TEXT("Root"));
	Owner->SetRootComponent(Root);
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> Components;
	TArray<FString> Errors;
	TestTrue(TEXT("native ISM adapter accepts a valid plan"),
		FGatersVisualMaterializer::Materialize(*Owner, *Root, Plan, Components, Errors));
	TestEqual(TEXT("one native component is reserved per visual class"), Components.Num(), 4);
	int32 NativeInstances = 0;
	for (const UInstancedStaticMeshComponent* Component : Components)
	{
		NativeInstances += Component->GetInstanceCount();
	}
	TestEqual(TEXT("native components contain the planned instances"), NativeInstances, 3);
	TestTrue(TEXT("native tree batch uses catalog-selected mesh"),
		Components[0]->GetStaticMesh() == CatalogTree);
	TestEqual(TEXT("valid native materialization has no diagnostics"), Errors.Num(), 0);
	for (int32 Batch = 0; Batch < 3; ++Batch)
	{
		TestTrue(TEXT("interactive native batch uses query collision"),
			Components[Batch]->GetCollisionEnabled() == ECollisionEnabled::QueryOnly);
		TestTrue(TEXT("interactive native batch emits overlaps"),
			Components[Batch]->GetGenerateOverlapEvents());
		TestTrue(TEXT("interactive native batch exposes each instance body"),
			Components[Batch]->bMultiBodyOverlap != 0);
	}
	TestTrue(TEXT("claimed visuals cannot retrigger claiming"),
		Components[3]->GetCollisionEnabled() == ECollisionEnabled::NoCollision);

	FGatersVisualBatchPlan MissingAsset = Plan;
	MissingAsset.TreeMesh = TSoftObjectPtr<UStaticMesh>(
		FSoftObjectPath(TEXT("/Game/Gaters/Missing/SM_Missing.SM_Missing")));
	TArray<FString> MissingErrors;
	AddExpectedError(TEXT("Failed to find object"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("missing replacement art rejects materialization"),
		FGatersVisualMaterializer::Materialize(
			*Owner, *Root, MissingAsset, Components, MissingErrors));
	TestEqual(TEXT("failed replacement cannot leave stale tree instances"),
		Components[0]->GetInstanceCount(), 0);

	const FGatersVisualInteraction Chop = FGatersVisualMaterializer::InteractionAt(
		Plan, EGatersVisualBatch::Trees, 0);
	TestEqual(TEXT("tree interaction resolves stable identity"), Chop.StableId,
		FString(TEXT("scatter:10")));
	TestEqual(TEXT("tree interaction creates a chop diff"), Chop.DiffEntry,
		FString(TEXT("chop:scatter:10")));
	const FGatersVisualInteraction Claim = FGatersVisualMaterializer::InteractionAt(
		Plan, EGatersVisualBatch::OpenClaims, 0);
	TestEqual(TEXT("plot interaction resolves stable identity"), Claim.StableId,
		FString(TEXT("plot:2")));
	TestEqual(TEXT("plot interaction creates a claim diff"), Claim.DiffEntry,
		FString(TEXT("claim:plot:2")));
	TestFalse(TEXT("claimed visual has no interaction"),
		FGatersVisualMaterializer::InteractionAt(
			Plan, EGatersVisualBatch::ClaimedClaims, 0).IsValid());

	const FGatersVisualBatchPlan Transitioned = FGatersVisualMaterializer::Plan(
		World, {TEXT("chop:scatter:10"), TEXT("chop:content:7:4:-3:11"),
			TEXT("claim:plot:2"), TEXT("claim:plot:3")});
	TestEqual(TEXT("harvested instances disappear"), Transitioned.Trees.Num(), 0);
	TestEqual(TEXT("claimed instances leave the interactive batch"), Transitioned.OpenClaims.Num(), 0);
	TestEqual(TEXT("claimed instances enter the inert batch"), Transitioned.ClaimedClaims.Num(), 2);
	return true;
}

#endif
