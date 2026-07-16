#pragma once

#include "CoreMinimal.h"
#include "DynamicMeshActor.h"
#include "GatersTerrainLab.generated.h"

class UStaticMeshComponent;

// Isolated editable-terrain experiment. Nothing spawns this actor automatically; place
// it in a blank level and use its Details-panel buttons to compare the base and cut mesh.
UCLASS(Blueprintable)
class PROTOTYPE_API AGatersTerrainLab : public ADynamicMeshActor
{
	GENERATED_BODY()

public:
	AGatersTerrainLab();

	UPROPERTY(EditAnywhere, Category = "Terrain Lab")
	int32 Seed = 73;

	UPROPERTY(EditAnywhere, Category = "Terrain Lab", meta = (ClampMin = "4", ClampMax = "48"))
	int32 CellsPerAxis = 32;

	UPROPERTY(EditAnywhere, Category = "Terrain Lab", meta = (ClampMin = "10.0"))
	float CellSize = 250.f;

	UPROPERTY(EditAnywhere, Category = "Terrain Lab")
	FVector CutCenter = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Terrain Lab", meta = (ClampMin = "1.0"))
	float CutRadius = 900.f;

	UPROPERTY(EditAnywhere, Category = "Terrain Lab|Demo")
	bool bPlayCutDemoOnBeginPlay = true;

	UPROPERTY(EditAnywhere, Category = "Terrain Lab|Demo", meta = (ClampMin = "0.1"))
	float CutDemoDuration = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Terrain Lab|Demo", meta = (ClampMin = "0.01"))
	float CutDemoUpdateSeconds = 0.12f;

	UPROPERTY(VisibleAnywhere, Category = "Terrain Lab")
	bool bCutApplied = false;

	UPROPERTY(VisibleAnywhere, Category = "Terrain Lab")
	int32 GeneratedTriangles = 0;

	UPROPERTY(VisibleAnywhere, Category = "Terrain Lab|Demo")
	bool bCutDemoRunning = false;

	// Isolated proof of generator-time contact fitting: green objects were seated into
	// the density field; the red object exceeded its deformation budget and was rejected.
	UPROPERTY(EditAnywhere, Category = "Terrain Lab|Contact Demo")
	bool bShowContactDemo = true;

	UPROPERTY(VisibleAnywhere, Category = "Terrain Lab|Contact Demo")
	int32 AcceptedContacts = 0;

	UPROPERTY(VisibleAnywhere, Category = "Terrain Lab|Contact Demo")
	int32 RejectedContacts = 0;

	UFUNCTION(CallInEditor, Category = "Terrain Lab")
	void RebuildTerrain();

	UFUNCTION(CallInEditor, Category = "Terrain Lab")
	void ApplyTestCut();

	UFUNCTION(CallInEditor, Category = "Terrain Lab")
	void ResetTerrain();

	void StartCutDemo();
	void AdvanceCutDemo(float DeltaSeconds);
	bool IsCutDemoRunning() const { return bCutDemoRunning; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	float DemoElapsed = 0.f;
	float DemoRebuildElapsed = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "Terrain Lab|Contact Demo")
	TObjectPtr<UStaticMeshComponent> ContactObjectA;

	UPROPERTY(VisibleAnywhere, Category = "Terrain Lab|Contact Demo")
	TObjectPtr<UStaticMeshComponent> ContactObjectB;

	UPROPERTY(VisibleAnywhere, Category = "Terrain Lab|Contact Demo")
	TObjectPtr<UStaticMeshComponent> RejectedObject;

	void BuildTerrain(float ActiveCutRadius);
};
