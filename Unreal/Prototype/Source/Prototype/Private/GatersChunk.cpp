#include "GatersChunk.h"

#include "Components/DynamicMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GatersContentCatalog.h"
#include "GatersEnvironmentContent.h"
#include "GatersEnvironmentBrief.h"
#include "GatersEnvironmentCandidateSelector.h"
#include "GatersDebugMessages.h"
#include "GatersLandformProcessField.h"
#include "GatersLegacyBaseLayer.h"
#include "GatersPhysicalFitEvaluator.h"
#include "GatersRaider.h"
#include "GatersRegionalWaterRecipe.h"
#include "GatersBuiltSiteLayer.h"
#include "GatersWorldRecipeLayer.h"
#include "GatersTerrainCell.h"
#include "GatersTerrainEvaluator.h"
#include "GatersTerrainPalette.h"
#include "GatersTerrainSemanticField.h"
#include "GatersWorldCellStreaming.h"
#include "GatersWorldDiff.h"
#include "GatersWorldCompiler.h"
#include "GatersVisualMaterializer.h"
#include "GeometryScript/MeshNormalsFunctions.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMemory.h"
#include "Misc/Paths.h"
#include "UDynamicMesh.h"

AGatersChunk::AGatersChunk()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGatersChunk::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bPerformanceReported && PerformanceFrameCount < PerformanceSampleFrames)
	{
		PerformanceFrameSeconds += DeltaSeconds;
		++PerformanceFrameCount;
		if (PerformanceFrameCount == PerformanceSampleFrames)
		{
			PerformanceSample.MeanFrameMs =
				PerformanceFrameSeconds * 1000.0 / PerformanceFrameCount;
			PerformanceSample.UsedPhysicalMB =
				FPlatformMemory::GetStats().UsedPhysical / (1024.0 * 1024.0);
			PerformanceSample.LoadedTerrainCells = LoadedTerrainCells.Num();
			PerformanceSample.TotalActors = 0;
			PerformanceSample.StaticMeshInstances = 0;
			PerformanceSample.StaticMeshComponents = 0;
			PerformanceSample.InstancedStaticMeshComponents = 0;
			PerformanceSample.StaticMeshTriangles = 0;
			PerformanceSample.DynamicMeshTriangles = 0;
			for (TActorIterator<AActor> It(GetWorld()); It; ++It)
			{
				++PerformanceSample.TotalActors;
				TInlineComponentArray<UStaticMeshComponent*> StaticMeshes;
				It->GetComponents(StaticMeshes);
				for (const UStaticMeshComponent* Component : StaticMeshes)
				{
					const UStaticMesh* Mesh = Component->GetStaticMesh();
					if (!Component->IsVisible() || !Mesh)
					{
						continue;
					}
					const UInstancedStaticMeshComponent* Instanced =
						Cast<UInstancedStaticMeshComponent>(Component);
					++PerformanceSample.StaticMeshComponents;
					PerformanceSample.InstancedStaticMeshComponents += Instanced ? 1 : 0;
					const int32 Instances = Instanced ? Instanced->GetInstanceCount() : 1;
					PerformanceSample.StaticMeshInstances += Instances;
					PerformanceSample.StaticMeshTriangles +=
						static_cast<int64>(Mesh->GetNumTriangles(0)) * Instances;
				}
				TInlineComponentArray<UDynamicMeshComponent*> DynamicMeshes;
				It->GetComponents(DynamicMeshes);
				for (UDynamicMeshComponent* Component : DynamicMeshes)
				{
					if (Component->IsVisible() && Component->GetDynamicMesh())
					{
						PerformanceSample.DynamicMeshTriangles +=
							Component->GetDynamicMesh()->GetTriangleCount();
					}
				}
			}
			PerformanceSample.Lod0Triangles = PerformanceSample.StaticMeshTriangles +
				PerformanceSample.DynamicMeshTriangles;
			FGatersPerformanceBudget Budget;
			Budget.MaxGenerationMs = MaxGenerationMs;
			Budget.MaxMeanFrameMs = MaxMeanFrameMs;
			Budget.MaxActors = MaxActors;
			Budget.MaxTerrainCells = MaxLoadedTerrainCells;
			Budget.MaxInstances = MaxStaticMeshInstances;
			Budget.MaxStaticMeshComponents = MaxStaticMeshComponents;
			Budget.MaxLod0Triangles = MaxLod0Triangles;
			if (MaxUsedPhysicalMB > 0.f)
			{
				Budget.MaxUsedPhysicalMB = MaxUsedPhysicalMB;
			}
			const FGatersPerformanceEvaluation Evaluation =
				FGatersPerformanceEvaluator::Evaluate(PerformanceSample, Budget);
			Report(FGatersPerformanceEvaluator::Report(PerformanceSample, Evaluation));
			for (const FGatersPerformanceIssue& Issue : Evaluation.Issues)
			{
				Report(FString::Printf(TEXT("PERF_FAIL rule=%s measured=%.3f limit=%.3f"),
					*Issue.RuleId, Issue.Measured, Issue.Limit));
			}
			bPerformanceReported = true;
		}
	}
	if (!bGalleryStreamingActive)
	{
		if (const APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			RefreshTerrainCells(
				FVector2D(Pawn->GetActorLocation() - GetActorLocation()),
				FVector2D(Pawn->GetVelocity()));
		}
	}
	if (bDebugSmashPiece)
	{
		bDebugSmashPiece = false;
		for (auto& Pair : StampedPieces)
		{
			if (Pair.Key)
			{
				Pair.Key->Destroy();
				break;
			}
		}
	}
	if (bDebugStartRaid)
	{
		bDebugStartRaid = false;
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		if (AGatersRaider* Raider = GetWorld()->SpawnActor<AGatersRaider>(
			GetActorLocation() + FVector(0.f, 0.f, 640.f), FRotator::ZeroRotator, Params))
		{
			Raider->Context = FString::Printf(TEXT("seed=%d"), Seed);
		}
	}
}

void AGatersChunk::BeginPlay()
{
	Super::BeginPlay();

	const double StartSeconds = FPlatformTime::Seconds();

	InitStream();
	RollSite();
	BuildGround();
	Report(FString::Printf(TEXT("REGIONAL_WATER v=1 surfaces=%d"),
		RegionalWaterSurfaceCount));
	AnalyzeSite();
	MarkResourceZones();
	LoadDiff();

	int32 ScatterInstances = 0, Claimed = 0;
	GenerateClaimMarkerRecipe(Claimed);
	if (bEnableBuiltSites)
	{
		GenerateBaseRecipe(RuntimeCatalog);
	}
	bRuntimeCatalogReady = true;
	SyncStreamedContent();
	const bool bCompiled = CompileWorld(RuntimeCatalog);
	int32 StampCount = 0, StampReplayed = 0;
	int32 ClaimInstances = 0;
	if (bCompiled)
	{
		MaterializeBase(StampCount, StampReplayed);
		MaterializeVisualBatches(ScatterInstances, ClaimInstances);
	}

	const FVector2D BaseSite(BaseDist * FMath::Cos(FMath::DegreesToRadians(BaseAngle)),
		BaseDist * FMath::Sin(FMath::DegreesToRadians(BaseAngle)));
	TArray<FString> RecipeErrors;
	const bool bRecipeValid = Recipe.Validate(RecipeErrors);
	Report(FString::Printf(TEXT("RECIPE schema=%d generator=%d seed=%d chunk=%.0f checksum=%08X nodes=%d valid=%s"),
		Recipe.SchemaVersion,
		Recipe.GeneratorVersion,
		Recipe.Seed,
		Recipe.ChunkSize,
		Recipe.Checksum(),
		Recipe.Nodes.Num(),
		bRecipeValid ? TEXT("yes") : TEXT("no")));
	for (const FString& Error : RecipeErrors)
	{
		Report(FString::Printf(TEXT("RECIPE error=%s"), *Error));
	}
	TArray<FString> EnvironmentErrors;
	const bool bEnvironmentValid = EnvironmentRecipe.Validate(EnvironmentErrors);
	const FGatersClimateSample ArrivalClimate =
		EnvironmentRecipe.QueryClimate(FVector2D::ZeroVector);
	Report(FString::Printf(
		TEXT("CLIMATE root=%d field=%d valid=%s temperature=%.4f precipitation=%.4f wind=%.4f seasonality=%.4f freeze_thaw=%.4f height=%.1f"),
		EnvironmentRecipe.Version,
		EnvironmentRecipe.Climate.Version,
		bEnvironmentValid ? TEXT("yes") : TEXT("no"),
		ArrivalClimate.Temperature,
		ArrivalClimate.Precipitation,
		ArrivalClimate.WindExposure,
		ArrivalClimate.Seasonality,
		ArrivalClimate.FreezeThaw,
		ArrivalClimate.Height));
	for (const FString& Error : EnvironmentErrors)
	{
		Report(FString::Printf(TEXT("CLIMATE error=%s"), *Error));
	}
	const FGatersTerrainEvaluation Terrain = FGatersTerrainEvaluator::Evaluate(
		EnvironmentRecipe.Terrain, ChunkSize);
	Report(FString::Printf(
		TEXT("EVAL v=%d relief=%.0f water=%.4f rough=%.0f cliff=%.0f buildable=%.4f mean=%.0f below=%.4f window=%.0f"),
		Terrain.EvaluatorVersion,
		Terrain.Relief(),
		Terrain.WaterFraction,
		Terrain.MeanNeighborStep,
		Terrain.MaxNeighborStep,
		Terrain.BuildableFraction,
		Terrain.MeanHeight,
		Terrain.BelowDatumFraction,
		ChunkSize));
	Report(FString::Printf(TEXT("SITE seed=%d environment=%s water=%s base_valid=%s base=(%.0f,%.0f) drop=%.0f hydrology=%s"),
		Seed, *EnvironmentRecipe.Terrain.Name(),
		EnvironmentRecipe.Terrain.HasWater() ? TEXT("yes") : TEXT("no"),
		bHaveBaseSite ? TEXT("yes") : TEXT("no"), BaseSite.X, BaseSite.Y,
		EnvironmentRecipe.Terrain.FootprintDrop(BaseSite, BaseFootprintRadius),
		*EnvironmentRecipe.Terrain.HydrologyName()));
	Report(FString::Printf(
		TEXT("TRAVERSE v=%d reachable=%d walkable=%d coverage=%.4f components=%d escape=%s base=%s path=%d"),
		Traversability.EvaluatorVersion,
		Traversability.Region.ReachableCount,
		Traversability.Region.WalkableCount,
		Traversability.ReachableFraction,
		Traversability.Region.ComponentCount,
		Traversability.bEscapesStart ? TEXT("yes") : TEXT("no"),
		Traversability.bGoalReachable ? TEXT("yes") : TEXT("no"),
		Traversability.GoalPath.Cost));
	Report(FString::Printf(TEXT("PLAN v=%d valid=%s sites=%d routes=%d diagnostics=%d"),
		SiteRoutePlan.PlannerVersion,
		SiteRoutePlan.bValid ? TEXT("yes") : TEXT("no"),
		SiteRoutePlan.Sites.Num(),
		SiteRoutePlan.Routes.Num(),
		SiteRoutePlan.Diagnostics.Num()));
	for (const FString& Diagnostic : SiteRoutePlan.Diagnostics)
	{
		Report(FString::Printf(TEXT("PLAN error=%s"), *Diagnostic));
	}
	Report(FString::Printf(TEXT("CONTENT cells=%d instances=%d"),
		LoadedContentCells.Num(), ScatterInstances));
	Report(FString::Printf(TEXT("DIFF entries=%d"), DiffEntries.Num()));
	Report(FString::Printf(TEXT("CLAIM plots=%d markers=%d claimed=%d"),
		PlotCenters.Num(), ClaimInstances, Claimed));
	Report(FString::Printf(TEXT("STAMP pieces=%d replayed=%d"), StampCount, StampReplayed));
	PerformanceSample.GenerationMs = (FPlatformTime::Seconds() - StartSeconds) * 1000.0;
	PerformanceSample.ScatterActors = 0;
	PerformanceSample.ClaimActors = 0;
	PerformanceSample.BaseActors = StampCount;
	PerformanceSample.LoadedTerrainCells = LoadedTerrainCells.Num();
	Report(FString::Printf(TEXT("GEN ms=%.1f"), PerformanceSample.GenerationMs));
}

void AGatersChunk::InitStream()
{
	// sin-hash the seed before stream init: FRandomStream first draws correlate
	// linearly for nearby seeds (the shader-hash trick, same as the BP version)
	const int32 Hashed = static_cast<int32>(FMath::Fmod(FMath::Sin(Seed * 12.9898) * 100000000.0, 2147483647.0));
	Stream.Initialize(Hashed);
}

void AGatersChunk::RollSite()
{
	FGatersEnvironment Terrain = FGatersEnvironment::FromSeed(Seed, WorldSize);
	const FGatersEnvironmentBrief RequestedBrief = FGatersEnvironmentBrief()
		.WithGlobalLandformTargets(
			LandformReliefOverride,
			LandformVolcanismOverride,
			LandformIceOverride);
	const FGatersEnvironmentBriefCompileResult Brief =
		FGatersEnvironmentBriefCompiler::Compile(RequestedBrief, Seed, WorldSize);
	FGatersLandformProcessRecipe AcceptedLandform;
	bool bHasClimateProvenance = false;
	if (Brief.IsValid())
	{
		const FGatersLandformProcessCompileResult BaseLandform =
			FGatersLandformProcessField::Compile(Terrain, Brief.Intent);
		if (BaseLandform.IsValid())
		{
			AcceptedLandform = BaseLandform.Recipe;
			bHasClimateProvenance = true;
		}
	}
	FGatersEnvironmentCandidateSelectionSettings LandAccessSettings;
	LandAccessSettings.CandidateCount = LandformCandidateCount;
	LandAccessSettings.WorldCellsPerAxis = GridN;
	LandAccessSettings.ArrivalCellsPerAxis = GridN;
	LandAccessSettings.ArrivalCellSize = CellSize;
	LandAccessSettings.PadRadius = PadRadius;
	LandAccessSettings.FlatNormalZ = FlatNormalZ;
	LandAccessSettings.SlopeNormalZ = SlopeNormalZ;
	LandAccessSettings.EscapeDistanceCells = FMath::CeilToInt(
		(PadRadius + CellSize) / FMath::Max(CellSize, 1.f));
	LandAccessSettings.WalkableTolerance = LandAccessWalkableTolerance;
	LandAccessSettings.ConnectedTolerance = LandAccessConnectedTolerance;
	FGatersEnvironmentCandidateSelectionResult LandAccess;
	bool bLandAccessEvaluated = false;
	int32 SatisfyingCandidates = 0;
	int32 ProtectedRegionCount = 0;
	if (bEnableLandformProcesses && Brief.IsValid())
	{
		TArray<FGatersLandformProtectedRegion> ProtectedRegions = {
			{TEXT("arrival"), FVector2D::ZeroVector, PadRadius, PadRadius * 2.f}};
		ProtectedRegionCount = ProtectedRegions.Num();
		LandAccess = FGatersEnvironmentCandidateSelector::Select(
			Terrain, Brief.Intent, ProtectedRegions, LandAccessSettings);
		bLandAccessEvaluated = LandAccess.Issues.IsEmpty();
		for (const FGatersEnvironmentCandidateEvidence& Candidate : LandAccess.Candidates)
		{
			SatisfyingCandidates += Candidate.bHasWorldAccess
				&& Candidate.bEscapesArrival
				&& Candidate.WalkableError <= LandAccessSettings.WalkableTolerance
				&& Candidate.ConnectedError <= LandAccessSettings.ConnectedTolerance
				&& Candidate.WalkableLand >= Brief.Intent.LandAccess.WalkableLand
					* (1.f - LandAccessSettings.WalkableTolerance)
				&& Candidate.ConnectedLand >= Brief.Intent.LandAccess.ConnectedLand
					* (1.f - LandAccessSettings.ConnectedTolerance)
				? 1 : 0;
		}
		if (LandAccess.bSelected)
		{
			AcceptedLandform = LandAccess.SelectedRecipe;
			bHasClimateProvenance = true;
			Terrain = Terrain.WithLandformProcesses(LandAccess.SelectedRecipe);
		}
	}
	Report(FString::Printf(
		TEXT("LANDFORM v=%d enabled=%s relief=%.3f volcanism=%.3f ice=%.3f protected=%d"),
		FGatersLandformProcessRecipe::CurrentVersion,
		bEnableLandformProcesses ? TEXT("yes") : TEXT("no"),
		Brief.Intent.Global.Relief,
		Brief.Intent.Global.Volcanism,
		Brief.Intent.Global.Ice,
		ProtectedRegionCount));
	const int32 AttemptedCandidates = LandAccess.Candidates.Num();
	const int32 SelectedIndex = LandAccess.bSelected
		? LandAccess.Selected.CandidateIndex : -1;
	const int32 BestIndex = AttemptedCandidates > 0
		? LandAccess.Best.CandidateIndex : -1;
	const float SelectedScale = LandAccess.bSelected
		? LandAccess.SelectedRecipe.FeatureScale : 0.f;
	const float BestScale = AttemptedCandidates > 0
		? LandAccess.BestRecipe.FeatureScale : 0.f;
	const float SelectedDissection = LandAccess.bSelected
		? LandAccess.SelectedRecipe.DissectionScale : 0.f;
	const float BestDissection = AttemptedCandidates > 0
		? LandAccess.BestRecipe.DissectionScale : 0.f;
	const float SelectedRuggedness = LandAccess.bSelected
		? LandAccess.SelectedRecipe.RuggednessScale : 0.f;
	const float BestRuggedness = AttemptedCandidates > 0
		? LandAccess.BestRecipe.RuggednessScale : 0.f;
	Report(FString::Printf(
		TEXT("LAND_ACCESS v=%d enabled=%s evaluated=%s brief=%d compiler=%d target_walkable=%.4f target_connected=%.4f candidates=%d world_cells=%d arrival_cells=%d arrival_cell=%.0f pad=%.0f semantic=%d transition=%.0f flat=%.3f slope=%.3f escape_cells=%d walkable_tol=%.4f connected_tol=%.4f satisfying=%d rejected=%d selected=%d selected_scale=%.4f selected_dissection=%.4f selected_ruggedness=%.4f selected_walkable=%.4f selected_connected=%.4f selected_world_access=%s selected_escape=%s best=%d best_scale=%.4f best_dissection=%.4f best_ruggedness=%.4f best_walkable=%.4f best_connected=%.4f best_world_access=%s best_escape=%s"),
		LandAccess.Version,
		bEnableLandformProcesses ? TEXT("yes") : TEXT("no"),
		bLandAccessEvaluated ? TEXT("yes") : TEXT("no"),
		Brief.Intent.BriefVersion,
		Brief.Intent.CompilerVersion,
		Brief.Intent.LandAccess.WalkableLand,
		Brief.Intent.LandAccess.ConnectedLand,
		AttemptedCandidates,
		LandAccessSettings.WorldCellsPerAxis,
		LandAccessSettings.ArrivalCellsPerAxis,
		LandAccessSettings.ArrivalCellSize,
		LandAccessSettings.PadRadius,
		FGatersTerrainSemanticField::CurrentVersion,
		FGatersTerrainSemanticField::PadTransitionWidth(LandAccessSettings.PadRadius),
		LandAccessSettings.FlatNormalZ,
		LandAccessSettings.SlopeNormalZ,
		LandAccessSettings.EscapeDistanceCells,
		LandAccessSettings.WalkableTolerance,
		LandAccessSettings.ConnectedTolerance,
		SatisfyingCandidates,
		AttemptedCandidates - SatisfyingCandidates,
		SelectedIndex,
		SelectedScale,
		SelectedDissection,
		SelectedRuggedness,
		LandAccess.bSelected ? LandAccess.Selected.WalkableLand : 0.f,
		LandAccess.bSelected ? LandAccess.Selected.ConnectedLand : 0.f,
		LandAccess.bSelected && LandAccess.Selected.bHasWorldAccess
			? TEXT("yes") : TEXT("no"),
		LandAccess.bSelected && LandAccess.Selected.bEscapesArrival
			? TEXT("yes") : TEXT("no"),
		BestIndex,
		BestScale,
		BestDissection,
		BestRuggedness,
		AttemptedCandidates > 0 ? LandAccess.Best.WalkableLand : 0.f,
		AttemptedCandidates > 0 ? LandAccess.Best.ConnectedLand : 0.f,
		AttemptedCandidates > 0 && LandAccess.Best.bHasWorldAccess
			? TEXT("yes") : TEXT("no"),
		AttemptedCandidates > 0 && LandAccess.Best.bEscapesArrival
			? TEXT("yes") : TEXT("no")));
	for (const FGatersEnvironmentCandidateSelectionIssue& Issue : LandAccess.Issues)
	{
		Report(FString::Printf(
			TEXT("LAND_ACCESS error=%s message=%s"), *Issue.RuleId, *Issue.Message));
	}
	EnvironmentRecipe = bHasClimateProvenance
		? FGatersEnvironmentRecipeCompiler::Compile(
			Terrain, Brief.Intent, AcceptedLandform)
		: FGatersEnvironmentRecipeCompiler::Compile(Terrain);
	Recipe = FGatersWorldRecipe::Generate(
		EnvironmentRecipe.Terrain,
		MinBaseDistance,
		MaxBaseDistance,
		BaseFootprintRadius,
		MaxFoundationDrop);
	bHaveBaseSite = Recipe.bHasBaseSite;
	const FVector2D BaseSite = Recipe.BaseSite;
	if (!bHaveBaseSite)
	{
		BaseAngle = Stream.FRandRange(0.f, 360.f);
		BaseDist = FMath::Lerp(MinBaseDistance, MaxBaseDistance, 0.5f);
		return;
	}
	BaseAngle = FMath::RadiansToDegrees(FMath::Atan2(BaseSite.Y, BaseSite.X));
	BaseDist = BaseSite.Size();
}

float AGatersChunk::GroundHeight(float LocalX, float LocalY) const
{
	return EnvironmentRecipe.MaterializedHeightAt(
		FVector2D(LocalX, LocalY), PadRadius,
		FVector2D(BaseDist * FMath::Cos(FMath::DegreesToRadians(BaseAngle)),
			BaseDist * FMath::Sin(FMath::DegreesToRadians(BaseAngle))));
}

FVector AGatersChunk::CellCenterLocal(int32 I, int32 J) const
{
	const int32 Half = GridN / 2;
	return FVector((I - Half) * CellSize, (J - Half) * CellSize, 0.f);
}

FVector AGatersChunk::CellLocationLocal(int32 I, int32 J) const
{
	FVector Result = CellCenterLocal(I, J);
	Result.Z = TerrainField.At(I, J).Height;
	return Result;
}

void AGatersChunk::BuildGround()
{
	UDynamicMeshComponent* Comp = GetDynamicMeshComponent();
	UDynamicMesh* TargetMesh = Comp->GetDynamicMesh();
	TargetMesh->Reset();

	FGeometryScriptPrimitiveOptions PrimOptions;
	// arrival marker plinth; the base gets no pedestal — EBS
	// foundations are the foundation skirt and sit on raw terrain
	UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendCylinder(
		TargetMesh, PrimOptions, FTransform::Identity, PadRadius, 500.f, 32);

	FGeometryScriptCalculateNormalsOptions NormalsOptions;
	UGeometryScriptLibrary_MeshNormalsFunctions::RecomputeNormals(TargetMesh, NormalsOptions);

	// A plain native material keeps imported art out of generation. Blender meshes can
	// replace decoration independently later.
	if (UMaterialInterface* Ground = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
	{
		UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Ground, this);
		Mid->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.20f, 0.28f, 0.18f));
		Comp->SetMaterial(0, Mid);
	}

	Comp->SetCollisionProfileName(TEXT("BlockAll"));
	Comp->EnableComplexAsSimpleCollision();
	Comp->UpdateCollision(false);

	const TArray<FGatersWaterSurface> WaterSurfaces =
		EnvironmentRecipe.Terrain.WaterSurfaces();
	if (!WaterSurfaces.IsEmpty())
	{
		const bool bLocalLakes =
			EnvironmentRecipe.Terrain.Hydrology == EGatersHydrology::Lakes;
		UStaticMesh* WaterMesh = LoadObject<UStaticMesh>(nullptr, bLocalLakes
			? TEXT("/Engine/BasicShapes/Cylinder.Cylinder")
			: TEXT("/Engine/BasicShapes/Plane.Plane"));
		UMaterialInterface* WaterMaterial = LoadObject<UMaterialInterface>(
			nullptr, TEXT("/Engine/EngineMaterials/WaterMaterial.WaterMaterial"));
		if (!WaterMaterial)
		{
			WaterMaterial = LoadObject<UMaterialInterface>(
				nullptr, TEXT("/Game/Gaters/Materials/MI_Claimed.MI_Claimed"));
		}
		if (WaterMesh && WaterMaterial)
		{
			UMaterialInstanceDynamic* WaterInstance =
				UMaterialInstanceDynamic::Create(WaterMaterial, this);
			WaterInstance->SetVectorParameterValue(
				TEXT("Absorption"), FGatersTerrainPalette::WaterAbsorption(
					EnvironmentRecipe.Terrain.Hydrology));
			WaterInstance->SetVectorParameterValue(
				TEXT("Scattering"), FGatersTerrainPalette::WaterScattering(
					EnvironmentRecipe.Terrain.Hydrology));
			WaterInstance->SetScalarParameterValue(
				TEXT("PhaseG"), FGatersTerrainPalette::WaterPhaseG(
					EnvironmentRecipe.Terrain.Hydrology));
			const FVector MeshSize = WaterMesh->GetBoundingBox().GetSize();
			for (int32 Index = 0; Index < WaterSurfaces.Num(); ++Index)
			{
				const FGatersWaterSurface& Surface = WaterSurfaces[Index];
				UStaticMeshComponent* WaterPlane = NewObject<UStaticMeshComponent>(
					this, *FString::Printf(TEXT("GeneratedWater%d"), Index));
				WaterPlane->SetStaticMesh(WaterMesh);
				WaterPlane->SetMaterial(0, WaterInstance);
				WaterPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				WaterPlane->SetCastShadow(false);
				WaterPlane->SetRelativeLocation(FVector(
					Surface.Center, EnvironmentRecipe.Terrain.WaterHeight + 3.f));
				const float Diameter = Surface.HalfExtent * 2.f;
				WaterPlane->SetRelativeScale3D(FVector(
					Diameter / MeshSize.X, Diameter / MeshSize.Y, bLocalLakes ? 0.02f : 1.f));
				WaterPlane->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
				WaterPlane->RegisterComponent();
				WaterPlanes.Add(WaterPlane);
			}
		}
	}

	BuildRegionalWater();
	RefreshTerrainCells(FVector2D::ZeroVector, FVector2D::ZeroVector, true);
}

void AGatersChunk::BuildRegionalWater()
{
	const FGatersRegionalWaterRecipe& RegionalWater = EnvironmentRecipe.RegionalWater;
	RegionalWaterSurfaceCount = RegionalWater.Surfaces.Num();
	if (RegionalWater.Surfaces.IsEmpty())
	{
		return;
	}

	UStaticMesh* WaterMesh = LoadObject<UStaticMesh>(
		nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UMaterialInterface* WaterMaterial = LoadObject<UMaterialInterface>(
		nullptr, TEXT("/Engine/EngineMaterials/WaterMaterial.WaterMaterial"));
	if (!WaterMaterial)
	{
		WaterMaterial = LoadObject<UMaterialInterface>(
			nullptr, TEXT("/Game/Gaters/Materials/MI_Claimed.MI_Claimed"));
	}
	if (!WaterMesh || !WaterMaterial)
	{
		return;
	}

	const FVector MeshSize = WaterMesh->GetBoundingBox().GetSize();
	for (int32 Index = 0; Index < RegionalWater.Surfaces.Num(); ++Index)
	{
		const FGatersRegionalWaterSurface& Surface = RegionalWater.Surfaces[Index];
		UMaterialInstanceDynamic* WaterInstance =
			UMaterialInstanceDynamic::Create(WaterMaterial, this);
		WaterInstance->SetVectorParameterValue(
			TEXT("Absorption"), FGatersTerrainPalette::WaterAbsorption(Surface.Hydrology));
		WaterInstance->SetVectorParameterValue(
			TEXT("Scattering"), FGatersTerrainPalette::WaterScattering(Surface.Hydrology));
		WaterInstance->SetScalarParameterValue(
			TEXT("PhaseG"), FGatersTerrainPalette::WaterPhaseG(Surface.Hydrology));

		UStaticMeshComponent* WaterPlane = NewObject<UStaticMeshComponent>(
			this, *FString::Printf(TEXT("GeneratedRegionalWater%d"), Index));
		WaterPlane->SetStaticMesh(WaterMesh);
		WaterPlane->SetMaterial(0, WaterInstance);
		WaterPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WaterPlane->SetCastShadow(false);
		WaterPlane->SetRelativeLocation(FVector(Surface.Center, Surface.Height + 3.f));
		const float Diameter = Surface.HalfExtent * 2.f;
		WaterPlane->SetRelativeScale3D(FVector(
			Diameter / MeshSize.X, Diameter / MeshSize.Y, 0.02f));
		WaterPlane->AttachToComponent(
			GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		WaterPlane->RegisterComponent();
		WaterPlanes.Add(WaterPlane);
	}
}

void AGatersChunk::RefreshTerrainCells(
	const FVector2D& SourceLocal,
	const FVector2D& SourceVelocity,
	bool bForce,
	int32 LoadRadiusOverride)
{
	const FIntPoint NewSourceCell = FGatersWorldCellStreaming::CellAt(SourceLocal, TerrainCellSize);
	const FVector2D CellCenter = FGatersWorldCellStreaming::CellCenter(NewSourceCell, TerrainCellSize);
	const FVector2D Offset = SourceLocal - CellCenter;
	FIntPoint NewPrefetchDirection = StreamingPrefetchDirection;
	const float EnterDistance = TerrainCellSize * TerrainPrefetchFraction;
	const float ExitDistance = EnterDistance * 0.5f;
	if (FMath::Abs(Offset.X) < ExitDistance && FMath::Abs(Offset.Y) < ExitDistance)
	{
		NewPrefetchDirection = FIntPoint::ZeroValue;
	}
	const bool bMovingTowardX = FMath::Abs(Offset.X) >= EnterDistance &&
		Offset.X * SourceVelocity.X > 0.f;
	const bool bMovingTowardY = FMath::Abs(Offset.Y) >= EnterDistance &&
		Offset.Y * SourceVelocity.Y > 0.f;
	if (bMovingTowardX || bMovingTowardY)
	{
		if (bMovingTowardX && (!bMovingTowardY || FMath::Abs(Offset.X) >= FMath::Abs(Offset.Y)))
		{
			NewPrefetchDirection = FIntPoint(Offset.X > 0.f ? 1 : -1, 0);
		}
		else
		{
			NewPrefetchDirection = FIntPoint(0, Offset.Y > 0.f ? 1 : -1);
		}
	}
	if (!bForce && bHaveStreamingSourceCell && NewSourceCell == StreamingSourceCell &&
		NewPrefetchDirection == StreamingPrefetchDirection)
	{
		return;
	}
	StreamingSourceCell = NewSourceCell;
	StreamingPrefetchDirection = NewPrefetchDirection;
	bHaveStreamingSourceCell = true;

	const int32 LoadRadius = LoadRadiusOverride == INDEX_NONE
		? TerrainLoadRadius
		: FMath::Clamp(LoadRadiusOverride, 1, 10);
	const TArray<FIntPoint> Desired = FGatersWorldCellStreaming::DesiredCells(
		{ SourceLocal }, TerrainCellSize, LoadRadius, WorldSize, StreamingPrefetchDirection);
	TSet<FIntPoint> DesiredSet;
	DesiredSet.Append(Desired);
	int32 Unloaded = 0;
	for (auto It = LoadedTerrainCells.CreateIterator(); It; ++It)
	{
		if (!DesiredSet.Contains(It.Key()))
		{
			if (It.Value())
			{
				It.Value()->Destroy();
			}
			It.RemoveCurrent();
			++Unloaded;
		}
	}

	const FVector2D RouteTarget(
		BaseDist * FMath::Cos(FMath::DegreesToRadians(BaseAngle)),
		BaseDist * FMath::Sin(FMath::DegreesToRadians(BaseAngle)));
	int32 Loaded = 0;
	for (const FIntPoint Cell : Desired)
	{
		if (LoadedTerrainCells.Contains(Cell))
		{
			continue;
		}
		const FVector2D Center = FGatersWorldCellStreaming::CellCenter(Cell, TerrainCellSize);
		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AGatersTerrainCell* TerrainCell = GetWorld()->SpawnActor<AGatersTerrainCell>(
			GetActorLocation() + FVector(Center, 0.f), FRotator::ZeroRotator, Params);
		if (TerrainCell)
		{
			TerrainCell->Configure(
				EnvironmentRecipe, Cell, TerrainCellSize, TerrainCellResolution, PadRadius, RouteTarget);
			TerrainCell->Build();
			LoadedTerrainCells.Add(Cell, TerrainCell);
			++Loaded;
		}
	}
	SyncStreamedContent();
	if (bRuntimeCatalogReady && (Loaded > 0 || Unloaded > 0))
	{
		int32 ScatterInstances = 0;
		int32 ClaimInstances = 0;
		if (CompileWorld(RuntimeCatalog))
		{
			MaterializeVisualBatches(ScatterInstances, ClaimInstances);
		}
		else
		{
			VisualPlan = {};
			for (UInstancedStaticMeshComponent* Batch : VisualBatches)
			{
				if (Batch)
				{
					Batch->ClearInstances();
					Batch->SetVisibility(false);
				}
			}
		}
	}
	Report(FString::Printf(
		TEXT("STREAM center=(%d,%d) prefetch=(%d,%d) radius=%d active=%d loaded=%d unloaded=%d"),
		StreamingSourceCell.X, StreamingSourceCell.Y,
		StreamingPrefetchDirection.X, StreamingPrefetchDirection.Y,
		LoadRadius, LoadedTerrainCells.Num(), Loaded, Unloaded));
}

int32 AGatersChunk::PrepareGalleryCapture(int32 OverrideRadius)
{
	bGalleryStreamingActive = true;
	const int32 Radius = OverrideRadius > 0 ? OverrideRadius : GalleryTerrainLoadRadius;
	RefreshTerrainCells(FVector2D::ZeroVector, FVector2D::ZeroVector, true, Radius);
	return LoadedTerrainCells.Num();
}

void AGatersChunk::AnalyzeSite()
{
	CellGrid.SetNum(GridN * GridN);
	HeightGrid.SetNum(GridN * GridN);
	PlotCenters.Reset();
	const FVector BaseLocal(BaseDist * FMath::Cos(FMath::DegreesToRadians(BaseAngle)),
		BaseDist * FMath::Sin(FMath::DegreesToRadians(BaseAngle)), 0.f);
	TerrainField = FGatersTerrainSemanticField::Build(
		EnvironmentRecipe.Terrain, EnvironmentRecipe.Intent,
		GridN, CellSize, PadRadius, FlatNormalZ, SlopeNormalZ,
		FVector2D(BaseLocal));

	const float PadReserveSq = FMath::Square(PadRadius + 200.f);
	const float BaseReserveSq = FMath::Square(2400.f);

	int32 FlatCount = 0, SlopeCount = 0, SteepCount = 0, WaterCount = 0;
	for (int32 I = 0; I < GridN; ++I)
	{
		for (int32 J = 0; J < GridN; ++J)
		{
			const int32 Idx = I * GridN + J;
			const FVector Local = CellCenterLocal(I, J);
			const FGatersTerrainSemanticSample& Sample = TerrainField.At(I, J);
			HeightGrid[Idx] = Sample.Height;

			int32 Cat;
			if (FVector2D(Local).SizeSquared() < PadReserveSq ||
				(FVector2D(Local) - FVector2D(BaseLocal)).SizeSquared() < BaseReserveSq)
			{
				Cat = Reserved;
			}
			else switch (Sample.Type)
			{
			case EGatersTerrainSemantic::Flat: Cat = Flat; ++FlatCount; break;
			case EGatersTerrainSemantic::Slope: Cat = Slope; ++SlopeCount; break;
			case EGatersTerrainSemantic::Steep: Cat = Steep; ++SteepCount; break;
			case EGatersTerrainSemantic::Water: Cat = Water; ++WaterCount; break;
			default: checkNoEntry(); Cat = Steep; break;
			}
			CellGrid[Idx] = Cat;
		}
	}

	// 3x3 all-flat clusters become buildable plots, spaced >= 15 m
	for (int32 I = 1; I < GridN - 1; ++I)
	{
		for (int32 J = 1; J < GridN - 1; ++J)
		{
			bool bAllFlat = true;
			for (int32 DI = -1; DI <= 1 && bAllFlat; ++DI)
			{
				for (int32 DJ = -1; DJ <= 1; ++DJ)
				{
					if (CellGrid[(I + DI) * GridN + (J + DJ)] != Flat) { bAllFlat = false; break; }
				}
			}
			if (!bAllFlat)
			{
				continue;
			}
			const FVector Local = CellCenterLocal(I, J);
			bool bSpaced = true;
			for (const FVector& Existing : PlotCenters)
			{
				if (FMath::Abs(Local.X + GetActorLocation().X - Existing.X) < 1500.f &&
					FMath::Abs(Local.Y + GetActorLocation().Y - Existing.Y) < 1500.f)
				{
					bSpaced = false;
					break;
				}
			}
			if (bSpaced)
			{
				PlotCenters.Add(GetActorLocation() + FVector(Local.X, Local.Y, HeightGrid[I * GridN + J]));
			}
		}
	}

	Report(FString::Printf(TEXT("PLOTS buildable=%d flat=%d slope=%d steep=%d water=%d"),
		PlotCenters.Num(), FlatCount, SlopeCount, SteepCount, WaterCount));

	const int32 Half = GridN / 2;
	const FIntPoint BaseCell(
		FMath::Clamp(FMath::RoundToInt(BaseLocal.X / CellSize) + Half, 0, GridN - 1),
		FMath::Clamp(FMath::RoundToInt(BaseLocal.Y / CellSize) + Half, 0, GridN - 1));
	Traversability = FGatersTraversabilityEvaluator::Evaluate(
		TerrainField,
		FIntPoint(Half, Half),
		BaseCell,
		FMath::CeilToInt((PadRadius + CellSize) / CellSize));

	SiteRoutePlan = FGatersSiteRoutePlanner::Plan(TerrainField, Seed, FVector2D(BaseLocal));
	for (const FGatersPlannedSite& Site : SiteRoutePlan.Sites)
	{
		EGatersRecipeNodeKind Kind;
		if (Site.Kind == EGatersPlannedSiteKind::Village)
		{
			Kind = EGatersRecipeNodeKind::VillageSite;
		}
		else if (Site.Kind == EGatersPlannedSiteKind::Landmark)
		{
			Kind = EGatersRecipeNodeKind::LandmarkSite;
		}
		else
		{
			continue;
		}
		Recipe.Nodes.Add({ Site.Id, Kind, Site.Location });
	}
	if (bEnableBuiltSites)
	{
		FGatersBuiltSiteLayerResult BuiltSites = FGatersBuiltSiteLayer::Generate(
			TerrainField, Seed, SiteRoutePlan, VillageGrowthStage);
		FGatersWorldRecipeLayer Layer;
		Layer.LayerId = TEXT("built-sites");
		Layer.SchemaVersion = BuiltSites.ContractVersion;
		Layer.GeneratorVersion = BuiltSites.SettlementGeneratorVersion;
		Layer.bGenerated = BuiltSites.IsValid();
		Layer.Nodes = MoveTemp(BuiltSites.Nodes);
		Layer.Diagnostics = BuiltSites.Diagnostics;
		const FGatersWorldRecipeLayerComposition Composition =
			FGatersWorldRecipeLayerComposer::Append(Recipe, Layer);
		for (const FString& Diagnostic : Composition.Diagnostics)
		{
			Report(FString::Printf(TEXT("BUILT_SITES_FAIL %s"), *Diagnostic));
		}
		Report(FString::Printf(
			TEXT("BUILT_SITES enabled=yes composed=%s sites=%d buildings=%d parcels=%d paths=%d modules=%d"),
			Composition.bComposed ? TEXT("yes") : TEXT("no"),
			BuiltSites.SiteCount, BuiltSites.BuildingCount, BuiltSites.ParcelCount,
			BuiltSites.PathCount, BuiltSites.ModuleCount));
	}
	else
	{
		Report(TEXT("BUILT_SITES enabled=no composed=yes sites=0 buildings=0 parcels=0 paths=0 modules=0"));
	}
	for (const FGatersPlannedRoute& Route : SiteRoutePlan.Routes)
	{
		for (int32 Index = 0; Index < Route.Path.Cells.Num(); ++Index)
		{
			const FIntPoint Cell = Route.Path.Cells[Index];
			FGatersRecipeNode Node{
				FString::Printf(TEXT("%s:%d"), *Route.Id, Index),
				EGatersRecipeNodeKind::RouteWaypoint,
				CellLocationLocal(Cell.X, Cell.Y) };
			Node.ContentKey = Route.Id;
			Recipe.Nodes.Add(MoveTemp(Node));
		}
	}
}

void AGatersChunk::DrawTraversalDebug(float Duration) const
{
	if (TerrainField.Cells.Num() == 0)
	{
		return;
	}
	for (int32 X = 0; X < GridN; ++X)
	{
		for (int32 Y = 0; Y < GridN; ++Y)
		{
			const FGatersTerrainSemanticSample& Sample = TerrainField.At(X, Y);
			FColor Color = FColor::Yellow;
			if (Traversability.Region.IsReachable(FIntPoint(X, Y)))
			{
				Color = FColor::Green;
			}
			else if (Sample.Type == EGatersTerrainSemantic::Water)
			{
				Color = FColor::Blue;
			}
			else if (Sample.Type == EGatersTerrainSemantic::Steep)
			{
				Color = FColor::Red;
			}
			const FVector Local = CellCenterLocal(X, Y);
			DrawDebugPoint(GetWorld(), GetActorLocation() + FVector(Local.X, Local.Y, Sample.Height + 120.f),
				6.f, Color, false, Duration, 0);
		}
	}
	for (int32 Index = 1; Index < Traversability.GoalPath.Cells.Num(); ++Index)
	{
		const FIntPoint A = Traversability.GoalPath.Cells[Index - 1];
		const FIntPoint B = Traversability.GoalPath.Cells[Index];
		const FVector LocalA = CellCenterLocal(A.X, A.Y);
		const FVector LocalB = CellCenterLocal(B.X, B.Y);
		DrawDebugLine(GetWorld(),
			GetActorLocation() + FVector(LocalA.X, LocalA.Y, TerrainField.At(A.X, A.Y).Height + 180.f),
			GetActorLocation() + FVector(LocalB.X, LocalB.Y, TerrainField.At(B.X, B.Y).Height + 180.f),
			FColor::White, false, Duration, 0, 8.f);
	}
	TMap<FString, FVector> PreviousRoutePoint;
	for (const FGatersCompiledNode& Node : CompiledWorld.Nodes)
	{
		if (Node.Kind != EGatersRecipeNodeKind::RouteWaypoint)
		{
			continue;
		}
		const FVector WorldPoint = GetActorLocation() + Node.Transform.GetLocation() + FVector(0, 0, 700.f);
		if (const FVector* Previous = PreviousRoutePoint.Find(Node.ContentKey))
		{
			DrawDebugLine(GetWorld(),
				*Previous, WorldPoint,
				FColor::Cyan, false, Duration, 0, 30.f);
		}
		PreviousRoutePoint.Add(Node.ContentKey, WorldPoint);
	}
	for (const FGatersCompiledNode& Node : CompiledWorld.Nodes)
	{
		FColor Color = FColor::Cyan;
		switch (Node.Kind)
		{
		case EGatersRecipeNodeKind::Arrival: break;
		case EGatersRecipeNodeKind::BaseSite: Color = FColor::Red; break;
		case EGatersRecipeNodeKind::VillageSite: Color = FColor::Magenta; break;
		case EGatersRecipeNodeKind::LandmarkSite: Color = FColor::Yellow; break;
		default: continue;
		}
		const FVector GroundLocation = GetActorLocation() + Node.Transform.GetLocation() + FVector(0, 0, 100.f);
		const FVector WorldLocation = GroundLocation + FVector(0, 0, 1500.f);
		DrawDebugLine(GetWorld(), GroundLocation, WorldLocation, Color, false, Duration, 0, 40.f);
		DrawDebugSphere(GetWorld(), WorldLocation, 420.f, 16, Color, false, Duration, 0, 28.f);
		DrawDebugString(GetWorld(), WorldLocation + FVector(0, 0, 500.f), Node.NodeId,
			nullptr, Color, Duration, false, 1.2f);
	}
	FGatersDebugMessages::Show(
		FGatersDebugMessages::TraversalKey,
		Duration,
		FColor::White,
		TEXT("PLAN: cyan routes/arrival | magenta village | red raid base | yellow landmark"));
	UE_LOG(LogTemp, Display, TEXT("[GatersChunk] TRAVERSAL_DRAW cells=%d path=%d duration=%.1f"),
		TerrainField.Cells.Num(), Traversability.GoalPath.Cells.Num(), Duration);
}

void AGatersChunk::DrawContentOpportunitiesDebug(float Duration) const
{
	const FVector2D RouteTarget(
		BaseDist * FMath::Cos(FMath::DegreesToRadians(BaseAngle)),
		BaseDist * FMath::Sin(FMath::DegreesToRadians(BaseAngle)));
	for (const TPair<FIntPoint, FGatersContentCellRecipe>& Entry : LoadedContentCells)
	{
		const FGatersContentCellRecipe& Content = Entry.Value;
		const FVector2D Center(
			Entry.Key.X * TerrainCellSize,
			Entry.Key.Y * TerrainCellSize);
		const float Total = FMath::Clamp(
			Content.VegetationOpportunity + Content.StoneOpportunity, 0.f, 1.f);
		const float TreeFraction = Content.VegetationOpportunity
			/ FMath::Max(Content.VegetationOpportunity + Content.StoneOpportunity, 0.001f);
		const FLinearColor Color = FMath::Lerp(
			FLinearColor(0.55f, 0.55f, 0.55f),
			FLinearColor(0.05f, 1.f, 0.12f), TreeFraction);
		const float Height = EnvironmentRecipe.MaterializedHeightAt(
			Center, PadRadius, RouteTarget);
		DrawDebugSphere(
			GetWorld(), GetActorLocation() + FVector(Center.X, Center.Y, Height + 500.f),
			100.f + 450.f * Total, 12, Color.ToFColor(true), false, Duration, 0, 8.f);

		for (const FGatersContentCellPlacement& Placement : Content.Placements)
		{
			const FColor PlacementColor = Placement.Kind == EGatersRecipeNodeKind::ScatterTree
				? FColor::Green
				: FColor::Silver;
			DrawDebugPoint(GetWorld(),
				GetActorLocation() + Placement.Transform.GetLocation() + FVector(0.f, 0.f, 150.f),
				18.f, PlacementColor, false, Duration, 0);
		}
	}
}

void AGatersChunk::MarkResourceZones()
{
	int32 ResourceCount = 0;
	const int32 R = ResourceClusterRadiusCells;
	for (int32 C = 0; C < ResourceClusters; ++C)
	{
		const int32 CX = Stream.RandRange(2, GridN - 3);
		const int32 CY = Stream.RandRange(2, GridN - 3);
		for (int32 DI = -R; DI <= R; ++DI)
		{
			for (int32 DJ = -R; DJ <= R; ++DJ)
			{
				const int32 GI = CX + DI, GJ = CY + DJ;
				if (DI * DI + DJ * DJ > R * R || GI < 0 || GI >= GridN || GJ < 0 || GJ >= GridN)
				{
					continue;
				}
				int32& Cat = CellGrid[GI * GridN + GJ];
				if (Cat == Flat || Cat == Slope)
				{
					Cat = Resource;
					++ResourceCount;
				}
			}
		}
	}
	Report(FString::Printf(TEXT("ZONES resource=%d clusters=%d"), ResourceCount, ResourceClusters));
}

FString AGatersChunk::SlotName() const
{
	// ponytail: "Cpp" prefix isolates these slots from the BP chunk while both exist;
	// unify when the C++ chunk replaces BP_TerrainChunk in the arrival loop
	return FString::Printf(TEXT("CppWorldDiff_%d"), Seed);
}

void AGatersChunk::LoadDiff()
{
	DiffEntries.Reset();
	if (UGameplayStatics::DoesSaveGameExist(SlotName(), 0))
	{
		if (const UGatersWorldDiff* Diff = Cast<UGatersWorldDiff>(UGameplayStatics::LoadGameFromSlot(SlotName(), 0)))
		{
			if (Diff->GenVersion == GatersGenVersion)
			{
				DiffEntries = Diff->Entries;
			}
			else
			{
				Report(FString::Printf(TEXT("DIFF discarded genversion=%d current=%d"),
					Diff->GenVersion, GatersGenVersion));
			}
		}
	}
}

void AGatersChunk::SaveDiff() const
{
	UGatersWorldDiff* Diff = Cast<UGatersWorldDiff>(UGameplayStatics::CreateSaveGameObject(UGatersWorldDiff::StaticClass()));
	Diff->GenVersion = GatersGenVersion;
	Diff->Entries = DiffEntries;
	UGameplayStatics::SaveGameToSlot(Diff, SlotName(), 0);
}

void AGatersChunk::SyncStreamedContent()
{
	Recipe.Nodes.RemoveAll([](const FGatersRecipeNode& Node)
	{
		return Node.Id.StartsWith(TEXT("content:"));
	});
	for (auto It = LoadedContentCells.CreateIterator(); It; ++It)
	{
		if (!LoadedTerrainCells.Contains(It.Key()))
		{
			It.RemoveCurrent();
		}
	}

	TArray<FIntPoint> Cells;
	LoadedTerrainCells.GetKeys(Cells);
	Cells.Sort([](const FIntPoint& A, const FIntPoint& B)
	{
		return A.X == B.X ? A.Y < B.Y : A.X < B.X;
	});
	FGatersContentCellSemantics Semantics;
	Semantics.PadRadius = PadRadius;
	Semantics.RouteTarget = FVector2D(
		BaseDist * FMath::Cos(FMath::DegreesToRadians(BaseAngle)),
		BaseDist * FMath::Sin(FMath::DegreesToRadians(BaseAngle)));
	Semantics.RouteTargetClearance = 2400.f;
	int32 ReservedRejected = 0;
	int32 WaterRejected = 0;
	int32 SteepRejected = 0;
	int32 OpportunityRejected = 0;
	int32 Placements = 0;
	for (const FIntPoint& Cell : Cells)
	{
		FGatersContentCellRecipe* Content = LoadedContentCells.Find(Cell);
		if (!Content)
		{
			Content = &LoadedContentCells.Add(Cell,
				FGatersContentCellRecipe::Generate(
					Cell, TerrainCellSize, EnvironmentRecipe, Semantics));
		}
		ReservedRejected += Content->Coverage.ReservedRejectedCount;
		WaterRejected += Content->Coverage.WaterRejectedCount;
		SteepRejected += Content->Coverage.SteepRejectedCount;
		OpportunityRejected += Content->Coverage.OpportunityRejectedCount;
		Placements += Content->Placements.Num();
		for (const FGatersContentCellPlacement& Placement : Content->Placements)
		{
			FGatersRecipeNode Node;
			Node.Id = Placement.Id;
			Node.Kind = Placement.Kind;
			Node.Location = Placement.Transform.GetLocation();
			Node.Rotation = Placement.Transform.Rotator();
			Node.Scale = Placement.Transform.GetScale3D();
			Node.ContentKey = Placement.ContentKey;
			Recipe.Nodes.Add(MoveTemp(Node));
		}
	}
	Report(FString::Printf(
		TEXT("CONTENT_STREAM cells=%d placements=%d reserved_rejected=%d water_rejected=%d steep_rejected=%d opportunity_rejected=%d"),
		LoadedContentCells.Num(), Placements, ReservedRejected, WaterRejected,
		SteepRejected, OpportunityRejected));
}

void AGatersChunk::GenerateClaimMarkerRecipe(int32& OutClaimed)
{
	OutClaimed = 0;
	for (int32 P = 0; P < PlotCenters.Num(); ++P)
	{
		const FGatersRecipeNode Node{
			FString::Printf(TEXT("plot:%d"), P),
			EGatersRecipeNodeKind::BuildPlot,
			PlotCenters[P] - GetActorLocation()};
		Recipe.Nodes.Add(Node);
		if (DiffEntries.Contains(FString::Printf(TEXT("claim:plot:%d"), P)))
		{
			++OutClaimed;
		}
	}
}

void AGatersChunk::MaterializeVisualBatches(
	int32& OutScatterInstances,
	int32& OutClaimInstances)
{
	TArray<FString> Errors;
	VisualPlan = FGatersVisualMaterializer::Plan(CompiledWorld, DiffEntries);
	const bool bMaterialized = GetRootComponent() && FGatersVisualMaterializer::Materialize(
		*this, *GetRootComponent(), VisualPlan, VisualBatches, Errors);
	OutScatterInstances = VisualPlan.Trees.Num() + VisualPlan.Rocks.Num();
	OutClaimInstances = VisualPlan.OpenClaims.Num() + VisualPlan.ClaimedClaims.Num();
	for (int32 Index = 0; Index < FMath::Min(3, VisualBatches.Num()); ++Index)
	{
		if (VisualBatches[Index])
		{
			VisualBatches[Index]->OnComponentBeginOverlap.AddUniqueDynamic(
				this, &AGatersChunk::OnVisualBatchOverlap);
		}
	}
	Report(FString::Printf(TEXT("VISUAL v=2 backend=UnrealISM batches=%d instances=%d carriers=0 valid=%s"),
		VisualBatches.Num(), VisualPlan.NumInstances(),
		bMaterialized ? TEXT("yes") : TEXT("no")));
	for (const FString& Error : Errors)
	{
		Report(FString::Printf(TEXT("VISUAL error=%s"), *Error));
	}
}

void AGatersChunk::GenerateBaseRecipe(FGatersContentCatalog& Catalog)
{
	BaseStampRows.Reset();
	TArray<FString> EnvironmentErrors;
	const bool bGeneratedRock = FGatersEnvironmentContent::Register(Catalog, EnvironmentErrors);
	Report(FString::Printf(TEXT("CATALOG key=environment.rock source=%s"),
		bGeneratedRock ? TEXT("generated-native-lod") : TEXT("engine-placeholder")));
	for (const FString& Error : EnvironmentErrors)
	{
		Report(FString::Printf(TEXT("CATALOG key=environment error=%s"), *Error));
	}

	// EBS modular actors carry the behavior; visuals come from EBS's Polygonal mesh
	// sets (Wood/Stone/Metal). Only the Modular (non-Chaos) actors are spawned - the
	// Chaos variants self-collapse without EBS's support graph. All vertical placement
	// is derived from the actual mesh bounds per tier, never hardcoded - mesh families
	// differ in height and pivot, and assuming one family's numbers floats the others.
	const FString Root = TEXT("/Game/EasyBuildingSystem/Blueprints/BuildingObjects/");
	auto LoadPiece = [&Root](const FString& Rel) -> UClass*
	{
		const FString Name = FPaths::GetBaseFilename(Rel);
		return LoadClass<AActor>(nullptr, *(Root + Rel + TEXT(".") + Name + TEXT("_C")));
	};
	UClass* FoundationClass = LoadPiece(TEXT("Modular/BP_EBS_Building_Foundation"));
	UClass* WallClass = LoadPiece(TEXT("Modular/BP_EBS_Building_Wall"));
	UClass* DoorFrameClass = LoadPiece(TEXT("Modular/BP_EBS_Building_DoorFrame"));
	UClass* DoorClass = LoadPiece(TEXT("Placeable/BP_EBS_Building_Door"));
	UClass* WindowFrameClass = LoadPiece(TEXT("Modular/BP_EBS_Building_WindowFrame"));
	UClass* CeilingClass = LoadPiece(TEXT("Modular/BP_EBS_Building_Ceiling"));
	UClass* FenceClass = LoadPiece(TEXT("Modular/BP_EBS_Building_Fence"));

	struct FTierMeshes
	{
		UStaticMesh* Foundation; UStaticMesh* Wall; UStaticMesh* DoorFrame;
		UStaticMesh* WindowFrame; UStaticMesh* Ceiling; UStaticMesh* Fence;
		bool IsValid() const { return Foundation && Wall && DoorFrame && Ceiling; }
	};
	auto LoadTierMeshes = [](const TCHAR* T) -> FTierMeshes
	{
		auto M = [&](const TCHAR* Piece) -> UStaticMesh*
		{
			return LoadObject<UStaticMesh>(nullptr, *FString::Printf(
				TEXT("/Game/EasyBuildingSystem/Meshes/Structures/Polygonal/%s/SM_Polygonal_%s_%s.SM_Polygonal_%s_%s"),
				T, T, Piece, T, Piece));
		};
		return { M(TEXT("Foundation")), M(TEXT("Wall")), M(TEXT("Doorframe")),
			M(TEXT("Windowframe")), M(TEXT("Ceiling")), M(TEXT("Fence")) };
	};
	const FTierMeshes TierMeshes[3] = { LoadTierMeshes(TEXT("Wood")), LoadTierMeshes(TEXT("Stone")), LoadTierMeshes(TEXT("Metal")) };
	if (!FoundationClass || !WallClass || !DoorFrameClass || !DoorClass || !CeilingClass ||
		!TierMeshes[0].IsValid() || !TierMeshes[1].IsValid() || !TierMeshes[2].IsValid())
	{
		Report(TEXT("STAMP pieces=0 replayed=0 (EBS classes/meshes not found)"));
		return;
	}

	struct FRuntimeModule
	{
		UClass* Class = nullptr;
		UStaticMesh* Mesh = nullptr;
		EGatersAssetContactSupport ContactSupport = EGatersAssetContactSupport::Attachment;
	};
	TMap<FString, FRuntimeModule> RuntimeModules;
	auto DescribeModule = [&](UClass* Class, UStaticMesh* Mesh, const FString& ContentKey,
		const EGatersAssetContactSupport ContactSupport)
	{
		FGatersLegacyBaseModuleDefinition Module;
		Module.bAvailable = Class && (Mesh || ContentKey == TEXT("door"));
		if (!Module.bAvailable)
		{
			return Module;
		}
		const FBox Bounds = Mesh ? Mesh->GetBoundingBox() : FBox(FVector(-100.f), FVector(100.f));
		Module.Contract.AssetId = Class->GetPathName() + TEXT("|") +
			(Mesh ? Mesh->GetPathName() : TEXT("actor"));
		Module.Contract.ContentKey = ContentKey;
		Module.Contract.StyleId = TEXT("gaters.clean-midpoly-painted");
		Module.Contract.BoundsCenter = Bounds.GetCenter();
		Module.Contract.BoundsExtent = Bounds.GetExtent();
		Module.Contract.ClearanceExtent = Bounds.GetExtent();
		Module.Contract.Collision = EGatersAssetCollision::Simple;
		Module.Contract.RenderClass = EGatersAssetRenderClass::UniqueStatic;
		Module.Contract.bInstanceStatePersistent = true;
		Module.Contract.Contacts.Add({TEXT("support"), Bounds.GetCenter() -
			FVector(0.f, 0.f, Bounds.GetExtent().Z), FVector::UpVector, ContactSupport});
		RuntimeModules.Add(ContentKey, {Class, Mesh, ContactSupport});
		return Module;
	};

	static const TCHAR* TierNames[] = {TEXT("wood"), TEXT("stone"), TEXT("metal")};
	FGatersLegacyBaseLayerInput Input;
	Input.BaseCenter = FVector2D(
		BaseDist * FMath::Cos(FMath::DegreesToRadians(BaseAngle)),
		BaseDist * FMath::Sin(FMath::DegreesToRadians(BaseAngle)));
	Input.RandomState = Stream.GetCurrentSeed();
	Input.CellSize = 300.f;
	Input.MaxFoundationDrop = MaxFoundationDrop;
	Input.SourceIds = {
		FString::Printf(TEXT("world:%d"), Recipe.Seed),
		TEXT("base:legacy:0")};
	for (int32 TierIndex = 0; TierIndex < UE_ARRAY_COUNT(TierNames); ++TierIndex)
	{
		const FString TierId(TierNames[TierIndex]);
		const FTierMeshes& Meshes = TierMeshes[TierIndex];
		FGatersLegacyBaseTierDefinition Tier;
		Tier.Id = TierId;
		Tier.Foundation = DescribeModule(FoundationClass, Meshes.Foundation,
			TierId + TEXT(".foundation"), EGatersAssetContactSupport::Terrain);
		Tier.Wall = DescribeModule(WallClass, Meshes.Wall,
			TierId + TEXT(".wall"), EGatersAssetContactSupport::Attachment);
		Tier.DoorFrame = DescribeModule(DoorFrameClass, Meshes.DoorFrame,
			TierId + TEXT(".doorframe"), EGatersAssetContactSupport::Attachment);
		Tier.Window = DescribeModule(WindowFrameClass, Meshes.WindowFrame,
			TierId + TEXT(".window"), EGatersAssetContactSupport::Attachment);
		Tier.Ceiling = DescribeModule(CeilingClass, Meshes.Ceiling,
			TierId + TEXT(".ceiling"), EGatersAssetContactSupport::Attachment);
		Tier.Fence = DescribeModule(FenceClass, Meshes.Fence,
			TierId + TEXT(".fence"), EGatersAssetContactSupport::Terrain);
		Input.Tiers.Add(MoveTemp(Tier));
	}
	Input.Door = DescribeModule(DoorClass, nullptr, TEXT("door"),
		EGatersAssetContactSupport::Attachment);

	const FGatersLegacyBaseLayerResult Result = FGatersLegacyBaseLayer::Generate(
		Input, [this](const FVector2D& Point) { return GroundHeight(Point.X, Point.Y); });
	if (!Result.IsValid())
	{
		for (const FString& Diagnostic : Result.Diagnostics)
		{
			Report(FString::Printf(TEXT("BASE error=%s"), *Diagnostic));
		}
		Report(TEXT("STAMP pieces=0 replayed=0 (legacy base rejected)"));
		return;
	}

	for (const FGatersAssetContract& Requirement : Result.ContentRequirements)
	{
		TArray<FString> Errors;
		if (!Catalog.AddPlaceholder(Requirement, Errors))
		{
			for (const FString& Error : Errors)
			{
				Report(FString::Printf(TEXT("CATALOG key=%s error=%s"),
					*Requirement.ContentKey, *Error));
			}
		}
	}
	for (const FGatersLegacyBasePieceRecipe& Piece : Result.Pieces)
	{
		const FRuntimeModule* Runtime = RuntimeModules.Find(Piece.ContentKey);
		if (!Runtime)
		{
			Report(FString::Printf(TEXT("BASE error=missing_runtime_module key=%s"),
				*Piece.ContentKey));
			BaseStampRows.Reset();
			return;
		}
		BaseStampRows.Add({Runtime->Class, Runtime->Mesh, Piece.Transform.GetLocation(),
			static_cast<float>(Piece.Transform.Rotator().Yaw), Piece.Transform.GetScale3D(), Piece.ContentKey,
			Runtime->ContactSupport});
		FGatersRecipeNode Node{Piece.Id, EGatersRecipeNodeKind::BasePiece,
			Piece.Transform.GetLocation()};
		Node.Rotation = Piece.Transform.Rotator();
		Node.Scale = Piece.Transform.GetScale3D();
		Node.ContentKey = Piece.ContentKey;
		Recipe.Nodes.Add(MoveTemp(Node));
	}
	static const TCHAR* ArchetypeNames[] = {TEXT("hut"), TEXT("compound"), TEXT("tower")};
	Report(FString::Printf(TEXT("BASE archetype=%s tier=%s main=%dx%dx%d buildings=%d"),
		ArchetypeNames[static_cast<int32>(Result.Archetype)], *Result.MainTierId,
		Result.MainWidth, Result.MainDepth, Result.MainStories, Result.BuildingCount));
}

bool AGatersChunk::CompileWorld(const FGatersContentCatalog& Catalog)
{
	CompiledWorld = FGatersWorldCompiler::Compile(
		Recipe, Catalog, TEXT("gaters.clean-midpoly-painted"));
	const bool bMatchesRecipe = CompiledWorld.MatchesRecipe(Recipe);
	Report(FString::Printf(TEXT("COMPILE checksum=%08X nodes=%d diagnostics=%d valid=%s recipe_match=%s"),
		CompiledWorld.RecipeChecksum, CompiledWorld.Nodes.Num(), CompiledWorld.Diagnostics.Num(),
		CompiledWorld.IsValid() ? TEXT("yes") : TEXT("no"),
		bMatchesRecipe ? TEXT("yes") : TEXT("no")));
	for (const FGatersCompilerDiagnostic& Diagnostic : CompiledWorld.Diagnostics)
	{
		Report(FString::Printf(TEXT("COMPILE %s node=%s key=%s %s"),
			Diagnostic.bError ? TEXT("error") : TEXT("warning"), *Diagnostic.NodeId,
			*Diagnostic.ContentKey, *Diagnostic.Message));
	}
	if (CompiledWorld.IsValid() && bMatchesRecipe)
	{
		const FVector2D RouteTarget(
			BaseDist * FMath::Cos(FMath::DegreesToRadians(BaseAngle)),
			BaseDist * FMath::Sin(FMath::DegreesToRadians(BaseAngle)));
		const TArray<FGatersPhysicalFitEvaluation> FitEvaluations =
			FGatersPhysicalFitEvaluator::EvaluateWorld(
				CompiledWorld, EnvironmentRecipe.Terrain, PadRadius, RouteTarget);
		int32 ValidCount = 0;
		int32 IssueCount = 0;
		int32 TerrainContactCount = 0;
		int32 PendingAttachmentCount = 0;
		for (const FGatersPhysicalFitEvaluation& Evaluation : FitEvaluations)
		{
			ValidCount += Evaluation.IsValid() ? 1 : 0;
			IssueCount += Evaluation.Issues.Num();
			TerrainContactCount += Evaluation.EvaluatedTerrainContacts;
			PendingAttachmentCount += Evaluation.PendingAttachmentContacts;
			for (const FGatersPhysicalFitIssue& Issue : Evaluation.Issues)
			{
				UE_LOG(LogTemp, Display,
					TEXT("[GatersChunk] FIT_FAIL rule=%s recipe=%s asset=%s subject=%s obstacle=%s measured=%.3f limit=%.3f"),
					*Issue.RuleId, *Issue.RecipeId, *Issue.AssetId,
					*Issue.SubjectId, *Issue.ObstacleId,
					Issue.Measured, Issue.Limit);
			}
		}
		Report(FString::Printf(
			TEXT("FIT v=2 evaluated=%d valid=%d issues=%d terrain_contacts=%d attachment_pending=%d"),
			FitEvaluations.Num(), ValidCount, IssueCount,
			TerrainContactCount, PendingAttachmentCount));
	}
	return CompiledWorld.IsValid() && bMatchesRecipe;
}

void AGatersChunk::MaterializeBase(int32& OutStamped, int32& OutReplayed)
{
	OutStamped = 0;
	OutReplayed = 0;
	StampedPieces.Reset();

	for (int32 R = 0; R < BaseStampRows.Num(); ++R)
	{
		if (DiffEntries.Contains(FString::Printf(TEXT("piece:%d"), R)))
		{
			++OutReplayed;
			continue;
		}
		const FGatersCompiledNode* Node = CompiledWorld.FindNode(FString::Printf(TEXT("piece:%d"), R));
		if (!Node)
		{
			Report(FString::Printf(TEXT("COMPILE error=missing_manifest_node node=piece:%d"), R));
			continue;
		}
		FTransform SpawnXf = Node->Transform;
		SpawnXf.AddToTranslation(GetActorLocation());
		AActor* Piece = GetWorld()->SpawnActorDeferred<AActor>(BaseStampRows[R].Class, SpawnXf, nullptr, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (Piece)
		{
			// EBS renders pieces in their under-construction state until Built is set;
			// stamped bases arrive finished
			if (FBoolProperty* BuiltProp = FindFProperty<FBoolProperty>(Piece->GetClass(), TEXT("Built")))
			{
				BuiltProp->SetPropertyValue_InContainer(Piece, true);
			}
			Piece->FinishSpawning(SpawnXf);
			if (BaseStampRows[R].Mesh)
			{
				if (UStaticMeshComponent* SMC = Piece->FindComponentByClass<UStaticMeshComponent>())
				{
					SMC->SetStaticMesh(BaseStampRows[R].Mesh);
				}
			}
			Piece->Tags.Add(TEXT("Breakable"));
			StampedPieces.Add(Piece, R);
			Piece->OnDestroyed.AddDynamic(this, &AGatersChunk::OnStampedPieceDestroyed);
			++OutStamped;
		}
	}

}

void AGatersChunk::OnStampedPieceDestroyed(AActor* DestroyedActor)
{
	// world teardown destroys every actor; only a real in-game destruction is a diff
	if (!GetWorld() || GetWorld()->bIsTearingDown)
	{
		return;
	}
	if (const int32* StampId = StampedPieces.Find(DestroyedActor))
	{
		DiffEntries.Add(FString::Printf(TEXT("piece:%d"), *StampId));
		SaveDiff();
		Report(FString::Printf(TEXT("PIECE destroyed=%d entries=%d"), *StampId, DiffEntries.Num()));
		StampedPieces.Remove(DestroyedActor);
	}
}

void AGatersChunk::OnVisualBatchOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComponent,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bApplyingVisualInteraction)
	{
		return;
	}
	const APawn* Pawn = Cast<APawn>(OtherActor);
	UInstancedStaticMeshComponent* BatchComponent =
		Cast<UInstancedStaticMeshComponent>(OverlappedComponent);
	const int32 BatchIndex = VisualBatches.IndexOfByKey(BatchComponent);
	if (!Pawn || !Pawn->IsPlayerControlled() || !BatchComponent || BatchIndex < 0 || BatchIndex > 2)
	{
		return;
	}

	// Overlap delegates do not guarantee that SweepResult.MyItem names the ISM body
	// for every event direction. Resolve the closest overlapping instance explicitly.
	const TArray<int32> Overlapping = BatchComponent->GetInstancesOverlappingSphere(
		Pawn->GetActorLocation(), 300.f, true);
	int32 InstanceIndex = INDEX_NONE;
	double BestDistanceSq = TNumericLimits<double>::Max();
	for (const int32 Candidate : Overlapping)
	{
		FTransform InstanceTransform;
		if (BatchComponent->GetInstanceTransform(Candidate, InstanceTransform, true))
		{
			const double DistanceSq = FVector::DistSquared(
				Pawn->GetActorLocation(), InstanceTransform.GetLocation());
			if (DistanceSq < BestDistanceSq)
			{
				BestDistanceSq = DistanceSq;
				InstanceIndex = Candidate;
			}
		}
	}
	const FGatersVisualInteraction Interaction = FGatersVisualMaterializer::InteractionAt(
		VisualPlan, static_cast<EGatersVisualBatch>(BatchIndex), InstanceIndex);
	if (!Interaction.IsValid() || DiffEntries.Contains(Interaction.DiffEntry))
	{
		return;
	}

	TGuardValue<bool> ApplyingGuard(bApplyingVisualInteraction, true);
	DiffEntries.Add(Interaction.DiffEntry);
	SaveDiff();
	int32 ScatterInstances = 0;
	int32 ClaimInstances = 0;
	MaterializeVisualBatches(ScatterInstances, ClaimInstances);
	if (Interaction.Kind == EGatersVisualInteractionKind::Chop)
	{
		Report(FString::Printf(TEXT("CHOP id=%s entries=%d"),
			*Interaction.StableId, DiffEntries.Num()));
	}
	else
	{
		Report(FString::Printf(TEXT("CLAIM plot=%s entries=%d"),
			*Interaction.StableId, DiffEntries.Num()));
	}
}

void AGatersChunk::Report(const FString& Line) const
{
	UE_LOG(LogTemp, Display, TEXT("[GatersChunk] %s"), *Line);
	FGatersDebugMessages::Show(
		FGatersDebugMessages::ReportKey, 30.f, FColor::Cyan, Line);
}

static FAutoConsoleCommandWithWorldAndArgs GatersTraversalCmd(
	TEXT("Gaters.Traversal"),
	TEXT("Draw terrain reachability for 45 seconds, or pass a duration in seconds."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
		[](const TArray<FString>& Args, UWorld* World)
		{
			const float Duration = Args.Num() > 0 ? FMath::Max(1.f, FCString::Atof(*Args[0])) : 45.f;
			for (TActorIterator<AGatersChunk> It(World); It; ++It)
			{
				It->DrawTraversalDebug(Duration);
			}
		}));

static FAutoConsoleCommandWithWorldAndArgs GatersContentOpportunitiesCmd(
	TEXT("Gaters.ContentOpportunities"),
	TEXT("Draw loaded content opportunities for 45 seconds. Green favors vegetation, gray favors stone, sphere size is combined opportunity, and dots are placements."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
		[](const TArray<FString>& Args, UWorld* World)
		{
			const float Duration = Args.Num() > 0
				? FMath::Max(1.f, FCString::Atof(*Args[0]))
				: 45.f;
			for (TActorIterator<AGatersChunk> It(World); It; ++It)
			{
				It->DrawContentOpportunitiesDebug(Duration);
			}
		}));
