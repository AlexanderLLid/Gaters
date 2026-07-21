#pragma once

#include "CoreMinimal.h"
#include "DynamicMeshActor.h"
#include "GatersContentCatalog.h"
#include "GatersContentCellRecipe.h"
#include "GatersEnvironment.h"
#include "GatersEnvironmentRecipe.h"
#include "GatersPerformanceEvaluator.h"
#include "GatersSiteRoutePlanner.h"
#include "GatersTerrainSemanticField.h"
#include "GatersTraversabilityEvaluator.h"
#include "GatersVisualMaterializer.h"
#include "GatersWorldCompiler.h"
#include "GatersWorldIntent.h"
#include "GatersWorldRecipe.h"
#include "GatersChunk.generated.h"

class AGatersTerrainCell;
class UPrimitiveComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UInstancedStaticMeshComponent;

// Prototype world coordinator. Terrain is an immutable pure function of Seed sampled by
// streamed cells; persistent changes are replayed as diff entries on regeneration.
// diff entry replayed on regeneration. Base archetypes intentionally not ported — base
// stamping (building-system pieces on plots) replaces them.
UCLASS()
class PROTOTYPE_API AGatersChunk : public ADynamicMeshActor
{
	GENERATED_BODY()

public:
	AGatersChunk();

	// --- tunables (values land in data, not prose) ---
	UPROPERTY(EditAnywhere, Category = "Gaters")
	int32 Seed = 7;

	UPROPERTY(EditAnywhere, Category = "Gaters|Settlement", meta = (ClampMin = "0", ClampMax = "2"))
	int32 VillageGrowthStage = 0;

	UPROPERTY(EditAnywhere, Category = "Gaters|Settlement")
	bool bEnableBuiltSites = true;

	UPROPERTY(EditAnywhere, Category = "Gaters|Terrain")
	bool bEnableLandformProcesses = false;

	UPROPERTY(EditAnywhere, Category = "Gaters|Terrain", meta = (ClampMin = "-1", ClampMax = "1"))
	float LandformReliefOverride = -1.f;

	UPROPERTY(EditAnywhere, Category = "Gaters|Terrain", meta = (ClampMin = "-1", ClampMax = "1"))
	float LandformVolcanismOverride = -1.f;

	UPROPERTY(EditAnywhere, Category = "Gaters|Terrain", meta = (ClampMin = "-1", ClampMax = "1"))
	float LandformIceOverride = -1.f;

	UPROPERTY(EditAnywhere, Category = "Gaters|Terrain", meta = (ClampMin = "1", ClampMax = "64"))
	int32 LandformCandidateCount = 8;

	UPROPERTY(EditAnywhere, Category = "Gaters|Terrain", meta = (ClampMin = "0", ClampMax = "1"))
	float LandAccessWalkableTolerance = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Gaters|Terrain", meta = (ClampMin = "0", ClampMax = "1"))
	float LandAccessConnectedTolerance = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Gaters")
	float ChunkSize = 30000.f;

	UPROPERTY(EditAnywhere, Category = "Gaters|Streaming")
	float WorldSize = 400000.f;

	UPROPERTY(EditAnywhere, Category = "Gaters")
	float PadRadius = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Gaters")
	int32 GridN = 61;

	UPROPERTY(EditAnywhere, Category = "Gaters")
	float CellSize = 500.f;

	UPROPERTY(EditAnywhere, Category = "Gaters|Streaming")
	float TerrainCellSize = 10000.f;

	UPROPERTY(EditAnywhere, Category = "Gaters|Streaming")
	int32 TerrainCellResolution = 64;

	UPROPERTY(EditAnywhere, Category = "Gaters|Streaming")
	int32 TerrainLoadRadius = 1;

	UPROPERTY(EditAnywhere, Category = "Gaters|Gallery", meta = (ClampMin = "1", ClampMax = "10"))
	int32 GalleryTerrainLoadRadius = 4;

	UPROPERTY(EditAnywhere, Category = "Gaters|Streaming", meta = (ClampMin = "0.1", ClampMax = "0.49"))
	float TerrainPrefetchFraction = 0.30f;

	UPROPERTY(EditAnywhere, Category = "Gaters")
	float MinBaseDistance = 6000.f;

	UPROPERTY(EditAnywhere, Category = "Gaters")
	float MaxBaseDistance = 10800.f;

	UPROPERTY(EditAnywhere, Category = "Gaters")
	float BaseFootprintRadius = 900.f;

	UPROPERTY(EditAnywhere, Category = "Gaters")
	float MaxFoundationDrop = 350.f;

	UPROPERTY(EditAnywhere, Category = "Gaters")
	float FlatNormalZ = 0.94f;

	UPROPERTY(EditAnywhere, Category = "Gaters")
	float SlopeNormalZ = 0.77f;

	UPROPERTY(EditAnywhere, Category = "Gaters")
	int32 ResourceClusters = 3;

	UPROPERTY(EditAnywhere, Category = "Gaters")
	int32 ResourceClusterRadiusCells = 3;

	UPROPERTY(EditAnywhere, Category = "Gaters")
	float RockChance = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Gaters|Performance", meta = (ClampMin = "1"))
	int32 PerformanceSampleFrames = 60;

	UPROPERTY(EditAnywhere, Category = "Gaters|Performance")
	float MaxGenerationMs = 8000.f;

	UPROPERTY(EditAnywhere, Category = "Gaters|Performance")
	float MaxMeanFrameMs = 50.f;

	UPROPERTY(EditAnywhere, Category = "Gaters|Performance")
	int32 MaxActors = 2000;

	UPROPERTY(EditAnywhere, Category = "Gaters|Performance")
	int32 MaxLoadedTerrainCells = 100;

	UPROPERTY(EditAnywhere, Category = "Gaters|Performance")
	int32 MaxStaticMeshInstances = 5000;

	UPROPERTY(EditAnywhere, Category = "Gaters|Performance")
	int32 MaxStaticMeshComponents = 2000;

	UPROPERTY(EditAnywhere, Category = "Gaters|Performance")
	int64 MaxLod0Triangles = 10000000;

	// Zero records process memory without enforcing a machine-specific limit.
	UPROPERTY(EditAnywhere, Category = "Gaters|Performance")
	float MaxUsedPhysicalMB = 0.f;

	// debug/test hook: set true (editor or MCP) to destroy one stamped piece through
	// the real destruction path — stands in for raid damage until combat exists
	UPROPERTY(EditAnywhere, Category = "Gaters")
	bool bDebugSmashPiece = false;

	// debug/test hook: set true to launch a probe raider from the arrival marker against this
	// chunk's base (same as the Gaters.Raid console command, but MCP-settable during PIE)
	UPROPERTY(EditAnywhere, Category = "Gaters")
	bool bDebugStartRaid = false;

	void DrawTraversalDebug(float Duration = 45.f) const;
	void DrawContentOpportunitiesDebug(float Duration = 45.f) const;
	int32 PrepareGalleryCapture(int32 OverrideRadius = 0);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// bound to every stamped base piece; a destroyed piece becomes a diff entry
	UFUNCTION()
	void OnStampedPieceDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void OnVisualBatchOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

private:
	struct FBaseStampRow
	{
		UClass* Class = nullptr;
		UStaticMesh* Mesh = nullptr;
		FVector Pos = FVector::ZeroVector;
		float Yaw = 0.f;
		FVector Scale = FVector::OneVector;
		FString ContentKey;
		EGatersAssetContactSupport ContactSupport = EGatersAssetContactSupport::Attachment;
	};

	// grid categories
	enum ECell : int32 { Flat = 0, Slope = 1, Steep = 2, Reserved = 3, Resource = 4, Water = 5 };

	// pure seed-derived state
	FRandomStream Stream;
	FGatersWorldRecipe Recipe;
	FGatersEnvironmentRecipe EnvironmentRecipe;
	FGatersTerrainSemanticField TerrainField;
	FGatersTraversabilityEvaluation Traversability;
	FGatersSiteRoutePlan SiteRoutePlan;
	FGatersCompiledWorld CompiledWorld;
	FGatersContentCatalog RuntimeCatalog;
	float BaseAngle = 0.f;
	float BaseDist = 3000.f;
	bool bHaveBaseSite = false;

	TArray<int32> CellGrid;
	TArray<float> HeightGrid;
	TArray<FVector> PlotCenters;
	TArray<FString> DiffEntries;
	TArray<FBaseStampRow> BaseStampRows;

	// pipeline
	void InitStream();
	void RollSite();
	void BuildGround();
	void BuildRegionalWater();
	void AnalyzeSite();
	void MarkResourceZones();
	void LoadDiff();
	void GenerateClaimMarkerRecipe(int32& OutClaimed);
	void MaterializeVisualBatches(int32& OutScatterInstances, int32& OutClaimInstances);
	void GenerateBaseRecipe(FGatersContentCatalog& Catalog);
	void SyncStreamedContent();
	bool CompileWorld(const FGatersContentCatalog& Catalog);
	void MaterializeBase(int32& OutStamped, int32& OutReplayed);
	void SaveDiff() const;

	// stamp id -> spawned piece, for diff bookkeeping on destruction
	UPROPERTY()
	TMap<TObjectPtr<AActor>, int32> StampedPieces;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> WaterPlanes;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> VisualBatches;
	FGatersVisualBatchPlan VisualPlan;
	bool bApplyingVisualInteraction = false;

	UPROPERTY(Transient)
	TMap<FIntPoint, TObjectPtr<AGatersTerrainCell>> LoadedTerrainCells;
	TMap<FIntPoint, FGatersContentCellRecipe> LoadedContentCells;
	bool bRuntimeCatalogReady = false;

	FIntPoint StreamingSourceCell = FIntPoint::ZeroValue;
	FIntPoint StreamingPrefetchDirection = FIntPoint::ZeroValue;
	bool bHaveStreamingSourceCell = false;
	bool bGalleryStreamingActive = false;
	FGatersPerformanceSample PerformanceSample;
	double PerformanceFrameSeconds = 0.0;
	int32 PerformanceFrameCount = 0;
	bool bPerformanceReported = false;
	int32 RegionalWaterSurfaceCount = 0;

	// ground is this function — vertices are displaced with it, so it needs no traces
	float GroundHeight(float LocalX, float LocalY) const;
	void RefreshTerrainCells(
		const FVector2D& SourceLocal,
		const FVector2D& SourceVelocity = FVector2D::ZeroVector,
		bool bForce = false,
		int32 LoadRadiusOverride = INDEX_NONE);
	FVector CellCenterLocal(int32 I, int32 J) const;
	FVector CellLocationLocal(int32 I, int32 J) const;
	FString SlotName() const;

	void Report(const FString& Line) const;
};
