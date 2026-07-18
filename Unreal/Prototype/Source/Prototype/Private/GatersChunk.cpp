#include "GatersChunk.h"

#include "Components/DynamicMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/TargetPoint.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GatersContentCatalog.h"
#include "GatersEnvironmentContent.h"
#include "GatersPhysicalFitEvaluator.h"
#include "GatersRaider.h"
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
	AnalyzeSite();
	MarkResourceZones();
	LoadDiff();

	int32 ScatterInstances = 0, Claimed = 0;
	GenerateClaimMarkerRecipe(Claimed);
	GenerateBaseRecipe(RuntimeCatalog);
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
	const FGatersTerrainEvaluation Terrain = FGatersTerrainEvaluator::Evaluate(Environment, ChunkSize);
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
		Seed, *Environment.Name(), Environment.HasWater() ? TEXT("yes") : TEXT("no"),
		bHaveBaseSite ? TEXT("yes") : TEXT("no"), BaseSite.X, BaseSite.Y,
		Environment.FootprintDrop(BaseSite, BaseFootprintRadius), *Environment.HydrologyName()));
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
	Recipe = FGatersWorldRecipe::Generate(
		Seed,
		WorldSize,
		MinBaseDistance,
		MaxBaseDistance,
		BaseFootprintRadius,
		MaxFoundationDrop);
	Environment = Recipe.CreateEnvironment();
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
	return FGatersTerrainSemanticField::MaterializedHeightAt(
		Environment, FVector2D(LocalX, LocalY), PadRadius,
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
	// gate pad plinth (the dial platform); the base gets no pedestal — EBS
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

	const TArray<FGatersWaterSurface> WaterSurfaces = Environment.WaterSurfaces();
	if (!WaterSurfaces.IsEmpty())
	{
		const bool bLocalLakes = Environment.Hydrology == EGatersHydrology::Lakes;
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
				TEXT("Absorption"), FGatersTerrainPalette::WaterAbsorption(Environment.Hydrology));
			WaterInstance->SetVectorParameterValue(
				TEXT("Scattering"), FGatersTerrainPalette::WaterScattering(Environment.Hydrology));
			WaterInstance->SetScalarParameterValue(
				TEXT("PhaseG"), FGatersTerrainPalette::WaterPhaseG(Environment.Hydrology));
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
				WaterPlane->SetRelativeLocation(FVector(Surface.Center, Environment.WaterHeight + 3.f));
				const float Diameter = Surface.HalfExtent * 2.f;
				WaterPlane->SetRelativeScale3D(FVector(
					Diameter / MeshSize.X, Diameter / MeshSize.Y, bLocalLakes ? 0.02f : 1.f));
				WaterPlane->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
				WaterPlane->RegisterComponent();
				WaterPlanes.Add(WaterPlane);
			}
		}
	}

	RefreshTerrainCells(FVector2D::ZeroVector, FVector2D::ZeroVector, true);
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
				Seed, WorldSize, Cell, TerrainCellSize, TerrainCellResolution, PadRadius, RouteTarget);
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
		Environment, GridN, CellSize, PadRadius, FlatNormalZ, SlopeNormalZ,
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
		case EGatersRecipeNodeKind::Gate: break;
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
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::White,
			TEXT("PLAN: cyan routes/arrival | magenta village | red raid base | yellow landmark"));
	}
	UE_LOG(LogTemp, Display, TEXT("[GatersChunk] TRAVERSAL_DRAW cells=%d path=%d duration=%.1f"),
		TerrainField.Cells.Num(), Traversability.GoalPath.Cells.Num(), Duration);
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
	// unify when the C++ chunk replaces BP_TerrainChunk in the gate loop
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
	int32 Placements = 0;
	for (const FIntPoint& Cell : Cells)
	{
		FGatersContentCellRecipe* Content = LoadedContentCells.Find(Cell);
		if (!Content)
		{
			Content = &LoadedContentCells.Add(Cell,
				FGatersContentCellRecipe::Generate(Cell, TerrainCellSize, Environment, Semantics));
		}
		ReservedRejected += Content->Coverage.ReservedRejectedCount;
		WaterRejected += Content->Coverage.WaterRejectedCount;
		SteepRejected += Content->Coverage.SteepRejectedCount;
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
		TEXT("CONTENT_STREAM cells=%d placements=%d reserved_rejected=%d water_rejected=%d steep_rejected=%d"),
		LoadedContentCells.Num(), Placements, ReservedRejected, WaterRejected, SteepRejected));
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
	TSet<FString> RegisteredContent;
	auto RegisterPlaceholder = [&](const FString& AssetId, const FString& ContentKey,
		const FVector& BoundsCenter, const FVector& BoundsExtent,
		const EGatersAssetRenderClass RenderClass, const bool bPersistent,
		const EGatersAssetContactSupport ContactSupport)
	{
		if (RegisteredContent.Contains(ContentKey))
		{
			return;
		}
		RegisteredContent.Add(ContentKey);
		FGatersAssetContract Contract;
		Contract.AssetId = AssetId;
		Contract.ContentKey = ContentKey;
		Contract.StyleId = TEXT("gaters.clean-midpoly-painted");
		Contract.BoundsCenter = BoundsCenter;
		Contract.BoundsExtent = BoundsExtent;
		Contract.ClearanceExtent = BoundsExtent;
		Contract.Collision = EGatersAssetCollision::Simple;
		Contract.RenderClass = RenderClass;
		Contract.bInstanceStatePersistent = bPersistent;
		Contract.Contacts.Add({TEXT("support"),
			BoundsCenter - FVector(0.f, 0.f, BoundsExtent.Z), FVector::UpVector,
			ContactSupport});
		TArray<FString> ContractErrors;
		if (!Catalog.AddPlaceholder(Contract, ContractErrors))
		{
			for (const FString& Error : ContractErrors)
			{
				Report(FString::Printf(TEXT("CATALOG key=%s error=%s"), *ContentKey, *Error));
			}
		}
	};
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

	// the stamp is data: (class, mesh, local position, yaw, scale) rows rolled from the
	// seed stream. Every roll depends only on the seed - never on the diff - so stamp
	// ids stay stable and destroyed pieces replay correctly.
	static const TCHAR* TierNames[] = { TEXT("wood"), TEXT("stone"), TEXT("metal") };
	FVector LootLocal = FVector::ZeroVector;
	bool bHaveLoot = false;
	const float Cell = 300.f;
	const FVector2D BaseCenter(BaseDist * FMath::Cos(FMath::DegreesToRadians(BaseAngle)),
		BaseDist * FMath::Sin(FMath::DegreesToRadians(BaseAngle)));
	auto TierKey = [](int32 Tier, const TCHAR* Piece)
	{
		return FString::Printf(TEXT("%s.%s"), TierNames[Tier], Piece);
	};

	// one rectangular building: W x D foundations, N stories, one door, rolled windows,
	// one material tier. Returns false when the ground under the footprint drops more
	// than the max-drop rule allows - the base must fit the environment, not fight it.
	auto AddBuilding = [&](FVector2D C, int32 W, int32 D, int32 Stories, int32 Tier) -> bool
	{
		const FTierMeshes& TM = TierMeshes[Tier];
		const FBox FndBox = TM.Foundation->GetBoundingBox();
		const FBox WallBox = TM.Wall->GetBoundingBox();
		const FBox CeilBox = TM.Ceiling->GetBoundingBox();
		const float FndHeight = FndBox.Max.Z - FndBox.Min.Z;
		const float WallHeight = WallBox.Max.Z - WallBox.Min.Z;

		const float HalfX = W * Cell * 0.5f, HalfY = D * Cell * 0.5f;
		float HMin = TNumericLimits<float>::Max(), HMax = -TNumericLimits<float>::Max();
		const FVector2D Corners[5] = { C, C + FVector2D(HalfX, HalfY), C + FVector2D(-HalfX, HalfY),
			C + FVector2D(HalfX, -HalfY), C + FVector2D(-HalfX, -HalfY) };
		for (const FVector2D& P : Corners)
		{
			const float H = GroundHeight(P.X, P.Y);
			HMin = FMath::Min(HMin, H);
			HMax = FMath::Max(HMax, H);
		}
		if (HMax - HMin > MaxFoundationDrop)
		{
			return false; // too much stilt - reject the site
		}
		const float FoundationTop = HMax + FndHeight; // level floor rides the highest corner

		// foundation skirts: each cell's foundation is stretched down from the level
		// floor to below its own patch of dirt, so nothing floats and nothing gapes
		for (int32 I = 0; I < W; ++I)
		{
			for (int32 J = 0; J < D; ++J)
			{
				const float X = C.X + (I - (W - 1) * 0.5f) * Cell;
				const float Y = C.Y + (J - (D - 1) * 0.5f) * Cell;
				const float GroundHere = FMath::Min(FMath::Min(
					GroundHeight(X - Cell * 0.5f, Y - Cell * 0.5f), GroundHeight(X + Cell * 0.5f, Y - Cell * 0.5f)), FMath::Min(
					GroundHeight(X - Cell * 0.5f, Y + Cell * 0.5f), GroundHeight(X + Cell * 0.5f, Y + Cell * 0.5f)));
				const float BottomZ = GroundHere - 60.f; // sink into the dirt
				const float ScaleZ = (FoundationTop - BottomZ) / FndHeight;
				const float PivotZ = FoundationTop - FndBox.Max.Z * ScaleZ;
				BaseStampRows.Add({FoundationClass, TM.Foundation, FVector(X, Y, PivotZ), 0.f,
					FVector(1.f, 1.f, ScaleZ), TierKey(Tier, TEXT("foundation")),
					EGatersAssetContactSupport::Terrain});
			}
		}

		const int32 DoorSide = Stream.RandRange(0, 3); // 0 S, 1 N, 2 W, 3 E
		const int32 DoorSlot = Stream.RandRange(0, (DoorSide < 2 ? W : D) - 1);
		for (int32 S = 0; S < Stories; ++S)
		{
			// band bottom sits flush on the foundation (or the band below);
			// pivot z compensates for wherever this mesh family keeps its origin
			const float BandBottom = FoundationTop + S * WallHeight;
			const float WallPivotZ = BandBottom - WallBox.Min.Z;
			auto AddWallSlot = [&](FVector2D P, float Yaw, bool bDoorHere)
			{
				const float WindowRoll = Stream.FRandRange(0.f, 1.f);
				UClass* Class = WallClass;
				UStaticMesh* Mesh = TM.Wall;
				FString ContentKey = TierKey(Tier, TEXT("wall"));
				if (bDoorHere)
				{
					Class = DoorFrameClass;
					Mesh = TM.DoorFrame;
					ContentKey = TierKey(Tier, TEXT("doorframe"));
				}
				else if (WindowRoll < 0.22f && WindowFrameClass && TM.WindowFrame)
				{
					Class = WindowFrameClass;
					Mesh = TM.WindowFrame;
					ContentKey = TierKey(Tier, TEXT("window"));
				}
				BaseStampRows.Add({Class, Mesh, FVector(P.X, P.Y, WallPivotZ), Yaw,
					FVector::OneVector, ContentKey});
				if (bDoorHere)
				{
					BaseStampRows.Add({DoorClass, nullptr, FVector(P.X, P.Y, WallPivotZ), Yaw,
						FVector::OneVector, TEXT("door")}); // door keeps its own mesh
				}
			};
			for (int32 I = 0; I < W; ++I)
			{
				const float X = C.X + (I - (W - 1) * 0.5f) * Cell;
				AddWallSlot(FVector2D(X, C.Y - HalfY), 0.f, S == 0 && DoorSide == 0 && DoorSlot == I);
				AddWallSlot(FVector2D(X, C.Y + HalfY), 0.f, S == 0 && DoorSide == 1 && DoorSlot == I);
			}
			for (int32 J = 0; J < D; ++J)
			{
				const float Y = C.Y + (J - (D - 1) * 0.5f) * Cell;
				AddWallSlot(FVector2D(C.X - HalfX, Y), 90.f, S == 0 && DoorSide == 2 && DoorSlot == J);
				AddWallSlot(FVector2D(C.X + HalfX, Y), 90.f, S == 0 && DoorSide == 3 && DoorSlot == J);
			}
		}

		const float RoofPivotZ = FoundationTop + Stories * WallHeight - CeilBox.Min.Z;
		for (int32 I = 0; I < W; ++I)
		{
			for (int32 J = 0; J < D; ++J)
			{
				BaseStampRows.Add({CeilingClass, TM.Ceiling, FVector(C.X + (I - (W - 1) * 0.5f) * Cell,
					C.Y + (J - (D - 1) * 0.5f) * Cell, RoofPivotZ), 0.f,
					FVector::OneVector, TierKey(Tier, TEXT("ceiling"))});
			}
		}
		// the first placed building holds the raid goal: an actor tagged RaidLoot on its
		// floor — the whole contract the raid sim (GatersRaider) evaluates against
		if (!bHaveLoot)
		{
			bHaveLoot = true;
			LootLocal = FVector(C.X, C.Y, FoundationTop + 100.f);
		}
		return true;
	};

	// archetype: 0 hut, 1 compound (multiple buildings + fence), 2 tower
	const int32 Archetype = Stream.RandRange(0, 2);
	static const TCHAR* ArchetypeNames[] = { TEXT("hut"), TEXT("compound"), TEXT("tower") };
	int32 Buildings = 0;
	const int32 MainTier = Stream.RandRange(0, 2);

	int32 MainW = Stream.RandRange(2, 4);
	int32 MainD = Stream.RandRange(2, 4);
	int32 MainStories = (Stream.FRandRange(0.f, 1.f) < 0.4f) ? 2 : 1;
	if (Archetype == 2)
	{
		MainW = Stream.RandRange(1, 2);
		MainD = MainW;
		MainStories = Stream.RandRange(3, 4);
	}
	Buildings += AddBuilding(BaseCenter, MainW, MainD, MainStories, MainTier) ? 1 : 0;

	if (Archetype == 1)
	{
		const int32 Extra = Stream.RandRange(1, 2);
		for (int32 E = 0; E < Extra; ++E)
		{
			const float Ang = Stream.FRandRange(0.f, 360.f);
			const float Dist = Stream.FRandRange(1100.f, 1600.f);
			const FVector2D C = BaseCenter + FVector2D(
				Dist * FMath::Cos(FMath::DegreesToRadians(Ang)), Dist * FMath::Sin(FMath::DegreesToRadians(Ang)));
			const int32 ShedTier = Stream.RandRange(0, MainTier); // sheds never outclass the main building
			Buildings += AddBuilding(C, Stream.RandRange(1, 2), Stream.RandRange(1, 3), 1, ShedTier) ? 1 : 0;
		}
		// fence ring with a gate gap, each post following its own patch of ground
		if (FenceClass && TierMeshes[MainTier].Fence)
		{
			UStaticMesh* FenceMesh = TierMeshes[MainTier].Fence;
			const float FenceZMin = FenceMesh->GetBoundingBox().Min.Z;
			const float FenceR = 2100.f;
			const int32 Sections = FMath::FloorToInt(2.f * FenceR / Cell);
			const int32 GapAt = Stream.RandRange(0, 3); // one open side
			for (int32 Side = 0; Side < 4; ++Side)
			{
				for (int32 K = 0; K < Sections; ++K)
				{
					if (Side == GapAt && K >= Sections / 2 - 1 && K <= Sections / 2 + 1)
					{
						continue; // gate gap
					}
					const float Along = (K - (Sections - 1) * 0.5f) * Cell;
					FVector2D P;
					float Yaw = 0.f;
					switch (Side)
					{
					case 0: P = BaseCenter + FVector2D(Along, -FenceR); Yaw = 0.f; break;
					case 1: P = BaseCenter + FVector2D(Along, FenceR); Yaw = 0.f; break;
					case 2: P = BaseCenter + FVector2D(-FenceR, Along); Yaw = 90.f; break;
					default: P = BaseCenter + FVector2D(FenceR, Along); Yaw = 90.f; break;
					}
					BaseStampRows.Add({FenceClass, FenceMesh,
						FVector(P.X, P.Y, GroundHeight(P.X, P.Y) - 20.f - FenceZMin), Yaw,
						FVector::OneVector, TierKey(MainTier, TEXT("fence")),
						EGatersAssetContactSupport::Terrain});
				}
			}
		}
	}

	Report(FString::Printf(TEXT("BASE archetype=%s tier=%s main=%dx%dx%d buildings=%d"),
		ArchetypeNames[Archetype], TierNames[MainTier], MainW, MainD, MainStories, Buildings));

	for (int32 R = 0; R < BaseStampRows.Num(); ++R)
	{
		FGatersRecipeNode Node{
			FString::Printf(TEXT("piece:%d"), R),
			EGatersRecipeNodeKind::BasePiece,
			BaseStampRows[R].Pos};
		Node.Rotation = FRotator(0.f, BaseStampRows[R].Yaw, 0.f);
		Node.Scale = BaseStampRows[R].Scale;
		Node.ContentKey = BaseStampRows[R].ContentKey;
		Recipe.Nodes.Add(Node);
	}
	if (bHaveLoot)
	{
		Recipe.Nodes.Add({TEXT("loot:0"), EGatersRecipeNodeKind::RaidLoot, LootLocal});
	}

	for (const FBaseStampRow& Row : BaseStampRows)
	{
		const FBox Bounds = Row.Mesh ? Row.Mesh->GetBoundingBox() : FBox(FVector(-100.f), FVector(100.f));
		RegisterPlaceholder(
			Row.Class->GetPathName() + TEXT("|") + (Row.Mesh ? Row.Mesh->GetPathName() : TEXT("actor")),
			Row.ContentKey, Bounds.GetCenter(), Bounds.GetExtent(),
			EGatersAssetRenderClass::UniqueStatic, true, Row.ContactSupport);
	}
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
				CompiledWorld, Environment, PadRadius, RouteTarget);
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

	if (const FGatersCompiledNode* LootNode = CompiledWorld.FindNode(TEXT("loot:0")))
	{
		ATargetPoint* LootPoint = GetWorld()->SpawnActor<ATargetPoint>(
			GetActorLocation() + LootNode->Transform.GetLocation(), FRotator::ZeroRotator);
		if (LootPoint)
		{
			LootPoint->Tags.Add(TEXT("RaidLoot"));
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
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Cyan, Line);
	}
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
