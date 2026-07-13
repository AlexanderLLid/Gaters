#pragma once

#include "CoreMinimal.h"
#include "DynamicMeshActor.h"
#include "GatersChunk.generated.h"

class AGatersScatter;

// C++ port of the BP_TerrainChunk worldgen/diff core. One chunk around one Gate is the
// whole generated world; ground is an immutable pure function of Seed; every change is a
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

	UPROPERTY(EditAnywhere, Category = "Gaters")
	float ChunkSize = 12000.f;

	UPROPERTY(EditAnywhere, Category = "Gaters")
	float PadRadius = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Gaters")
	int32 GridN = 25;

	UPROPERTY(EditAnywhere, Category = "Gaters")
	float CellSize = 500.f;

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

	UPROPERTY(EditAnywhere, Category = "Gaters")
	int32 GrassPerCell = 14;

	// debug/test hook: set true (editor or MCP) to destroy one stamped piece through
	// the real destruction path — stands in for raid damage until combat exists
	UPROPERTY(EditAnywhere, Category = "Gaters")
	bool bDebugSmashPiece = false;

	// debug/test hook: set true to launch a probe raider from the gate pad against this
	// chunk's base (same as the Gaters.Raid console command, but MCP-settable during PIE)
	UPROPERTY(EditAnywhere, Category = "Gaters")
	bool bDebugStartRaid = false;

	// --- runtime interaction (called by scatter / claim actors) ---
	void ChopScatter(int32 Id, AActor* Victim);
	void ClaimPlot(int32 PlotIndex);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// bound to every stamped base piece; a destroyed piece becomes a diff entry
	UFUNCTION()
	void OnStampedPieceDestroyed(AActor* DestroyedActor);

private:
	// grid categories
	enum ECell : int32 { Flat = 0, Slope = 1, Steep = 2, Reserved = 3, Resource = 4 };

	// pure seed-derived state
	FRandomStream Stream;
	int32 TerrainPreset = 0;
	float NoiseMagnitude = 150.f;
	float NoiseFrequency = 0.0004f;
	float BaseAngle = 0.f;
	float BaseDist = 3000.f;
	FVector2D NoiseOffset = FVector2D::ZeroVector;

	TArray<int32> CellGrid;
	TArray<float> HeightGrid;
	TArray<FVector> PlotCenters;
	TArray<FString> DiffEntries;

	// pipeline
	void InitStream();
	void RollSite();
	void BuildGround();
	void AnalyzeSite();
	void MarkResourceZones();
	void LoadDiff();
	void SpawnGrass(int32& OutGrass);
	void SpawnScatter(int32& OutSpawned, int64& OutSum, int32& OutReplayed);
	void SpawnClaimMarkers(int32& OutClaimed);
	void StampBase(int32& OutStamped, int32& OutReplayed);
	void SaveDiff() const;

	// stamp id -> spawned piece, for diff bookkeeping on destruction
	UPROPERTY()
	TMap<TObjectPtr<AActor>, int32> StampedPieces;

	// ground is this function — vertices are displaced with it, so it needs no traces
	float GroundHeight(float LocalX, float LocalY) const;
	FVector CellCenterLocal(int32 I, int32 J) const;
	FString SlotName() const;

	void Report(const FString& Line) const;
};
