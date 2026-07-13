#include "GatersChunk.h"

#include "Components/DynamicMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/TargetPoint.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GatersClaimMarker.h"
#include "GatersRaider.h"
#include "GatersScatter.h"
#include "GatersWorldDiff.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "GeometryScript/MeshNormalsFunctions.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "GeometryScript/MeshUVFunctions.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "UDynamicMesh.h"

AGatersChunk::AGatersChunk()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGatersChunk::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
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

	int32 Spawned = 0, Replayed = 0, Claimed = 0, GrassCount = 0;
	int64 Sum = 0;
	SpawnGrass(GrassCount);
	SpawnScatter(Spawned, Sum, Replayed);
	SpawnClaimMarkers(Claimed);
	int32 StampCount = 0, StampReplayed = 0;
	StampBase(StampCount, StampReplayed);

	static const TCHAR* PresetNames[] = { TEXT("plains"), TEXT("hills"), TEXT("broken") };
	Report(FString::Printf(TEXT("SITE seed=%d preset=%s archetype=none"), Seed, PresetNames[TerrainPreset]));
	Report(FString::Printf(TEXT("SCATTER n=%d sum=%lld grass=%d"), Spawned, Sum, GrassCount));
	Report(FString::Printf(TEXT("DIFF entries=%d replayed=%d"), DiffEntries.Num(), Replayed));
	Report(FString::Printf(TEXT("CLAIM plots=%d claimed=%d"), PlotCenters.Num(), Claimed));
	Report(FString::Printf(TEXT("STAMP pieces=%d replayed=%d"), StampCount, StampReplayed));
	Report(FString::Printf(TEXT("GEN ms=%.1f"), (FPlatformTime::Seconds() - StartSeconds) * 1000.0));
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
	TerrainPreset = Stream.RandRange(0, 2);
	BaseAngle = Stream.FRandRange(0.f, 360.f);
	BaseDist = Stream.FRandRange(2800.f, 4500.f);
	NoiseOffset.X = Stream.FRandRange(-100000.f, 100000.f);
	NoiseOffset.Y = Stream.FRandRange(-100000.f, 100000.f);

	static const float Magnitudes[] = { 150.f, 300.f, 500.f };
	static const float Frequencies[] = { 0.0004f, 0.0005f, 0.0009f };
	NoiseMagnitude = Magnitudes[TerrainPreset];
	NoiseFrequency = Frequencies[TerrainPreset];
}

float AGatersChunk::GroundHeight(float LocalX, float LocalY) const
{
	const FVector2D P((LocalX + NoiseOffset.X) * NoiseFrequency, (LocalY + NoiseOffset.Y) * NoiseFrequency);
	return FMath::PerlinNoise2D(P) * NoiseMagnitude;
}

FVector AGatersChunk::CellCenterLocal(int32 I, int32 J) const
{
	const int32 Half = GridN / 2;
	return FVector((I - Half) * CellSize, (J - Half) * CellSize, 0.f);
}

void AGatersChunk::BuildGround()
{
	UDynamicMeshComponent* Comp = GetDynamicMeshComponent();
	UDynamicMesh* TargetMesh = Comp->GetDynamicMesh();
	TargetMesh->Reset();

	FGeometryScriptPrimitiveOptions PrimOptions;
	UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendRectangleXY(
		TargetMesh, PrimOptions, FTransform::Identity, ChunkSize, ChunkSize, 64, 64);

	// ground IS GroundHeight(): displace vertices with the same pure function the
	// analysis grid samples, so heights need no traces and stay exact math per seed
	TargetMesh->EditMesh([this](UE::Geometry::FDynamicMesh3& EditMesh)
	{
		for (const int32 Vid : EditMesh.VertexIndicesItr())
		{
			FVector3d Pos = EditMesh.GetVertex(Vid);
			Pos.Z = GroundHeight(static_cast<float>(Pos.X), static_cast<float>(Pos.Y));
			EditMesh.SetVertex(Vid, Pos);
		}
	});

	// gate pad plinth (the dial platform); the base gets no pedestal — EBS
	// foundations are the foundation skirt and sit on raw terrain
	UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendCylinder(
		TargetMesh, PrimOptions, FTransform::Identity, PadRadius, 500.f, 32);

	FGeometryScriptCalculateNormalsOptions NormalsOptions;
	UGeometryScriptLibrary_MeshNormalsFunctions::RecomputeNormals(TargetMesh, NormalsOptions);

	// tile the megascans moss at ~3 m per repeat instead of one stretch over the island
	UGeometryScriptLibrary_MeshUVFunctions::ScaleMeshUVs(TargetMesh, 0,
		FVector2D(ChunkSize / 300.f, ChunkSize / 300.f), FVector2D::ZeroVector,
		FGeometryScriptMeshSelection());
	// Nordic Moss: plain opaque M_MS_Srf parent — Mossy_Forest_Floor's parent is the
	// transmission variant, which renders a dynamic mesh invisible
	if (UMaterialInterface* Ground = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/Fab/Megascans/Surfaces/Nordic_Moss_se4rwei/Raw/se4rwei_tier_0/Materials/MI_se4rwei.MI_se4rwei")))
	{
		// push the yellowish moss toward lush green (quixel master exposes Albedo Tint)
		UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Ground, this);
		Mid->SetVectorParameterValue(TEXT("Albedo Tint"), FLinearColor(0.6f, 0.95f, 0.45f));
		Comp->SetMaterial(0, Mid);
	}

	Comp->SetCollisionProfileName(TEXT("BlockAll"));
	Comp->EnableComplexAsSimpleCollision();
	Comp->UpdateCollision(false);
}

void AGatersChunk::SpawnGrass(int32& OutGrass)
{
	OutGrass = 0;
	static const TCHAR* GrassMeshes[] = {
		TEXT("/Game/Fab/Megascans/Plants/Wild_Grass_vlkhcbxia/Raw/vlkhcbxia_tier_0/StaticMeshes/SM_vlkhcbxia_VarA.SM_vlkhcbxia_VarA"),
		TEXT("/Game/Fab/Megascans/Plants/Wild_Grass_vlkhcbxia/Raw/vlkhcbxia_tier_0/StaticMeshes/SM_vlkhcbxia_VarC.SM_vlkhcbxia_VarC"),
		TEXT("/Game/Fab/Megascans/Plants/Wild_Grass_vlkhcbxia/Raw/vlkhcbxia_tier_0/StaticMeshes/SM_vlkhcbxia_VarE.SM_vlkhcbxia_VarE"),
		TEXT("/Game/Fab/Megascans/Plants/Wild_Grass_vlkhcbxia/Raw/vlkhcbxia_tier_0/StaticMeshes/SM_vlkhcbxia_VarG.SM_vlkhcbxia_VarG") };

	TArray<UHierarchicalInstancedStaticMeshComponent*> GrassLayers;
	for (const TCHAR* Path : GrassMeshes)
	{
		UStaticMesh* GrassMesh = LoadObject<UStaticMesh>(nullptr, Path);
		if (!GrassMesh)
		{
			continue;
		}
		UHierarchicalInstancedStaticMeshComponent* Hism =
			NewObject<UHierarchicalInstancedStaticMeshComponent>(this);
		Hism->SetStaticMesh(GrassMesh);
		Hism->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Hism->SetCastShadow(false);
		Hism->SetCullDistances(7000, 14000);
		Hism->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		Hism->RegisterComponent();
		GrassLayers.Add(Hism);
	}
	if (GrassLayers.IsEmpty())
	{
		return;
	}

	// grass is pure decoration: its own stream so it never shifts the main stream's
	// draw sequence — existing seeds must keep generating identical worlds
	FRandomStream GrassStream(static_cast<int32>(
		FMath::Fmod(FMath::Sin((Seed + 101) * 12.9898) * 100000000.0, 2147483647.0)));

	for (int32 I = 0; I < GridN; ++I)
	{
		for (int32 J = 0; J < GridN; ++J)
		{
			const int32 Cat = CellGrid[I * GridN + J];
			if (Cat == Steep || Cat == Reserved)
			{
				continue;
			}
			const FVector Center = CellCenterLocal(I, J);
			for (int32 K = 0; K < GrassPerCell; ++K)
			{
				const float X = Center.X + GrassStream.FRandRange(-CellSize * 0.5f, CellSize * 0.5f);
				const float Y = Center.Y + GrassStream.FRandRange(-CellSize * 0.5f, CellSize * 0.5f);
				const FTransform Xf(
					FRotator(0.f, GrassStream.FRandRange(0.f, 360.f), 0.f),
					FVector(X, Y, GroundHeight(X, Y)),
					FVector(GrassStream.FRandRange(1.1f, 1.9f)));
				GrassLayers[OutGrass % GrassLayers.Num()]->AddInstance(Xf, /*bWorldSpace*/ false);
				++OutGrass;
			}
		}
	}
}

void AGatersChunk::AnalyzeSite()
{
	CellGrid.SetNum(GridN * GridN);
	HeightGrid.SetNum(GridN * GridN);
	PlotCenters.Reset();

	const FVector BaseLocal(BaseDist * FMath::Cos(FMath::DegreesToRadians(BaseAngle)),
		BaseDist * FMath::Sin(FMath::DegreesToRadians(BaseAngle)), 0.f);
	const float PadReserveSq = FMath::Square(PadRadius + 200.f);
	const float BaseReserveSq = FMath::Square(1900.f);

	int32 FlatCount = 0, SlopeCount = 0, SteepCount = 0;
	for (int32 I = 0; I < GridN; ++I)
	{
		for (int32 J = 0; J < GridN; ++J)
		{
			const int32 Idx = I * GridN + J;
			const FVector Local = CellCenterLocal(I, J);
			HeightGrid[Idx] = GroundHeight(Local.X, Local.Y);

			// slope from the analytic height gradient (central differences)
			const float Eps = CellSize * 0.25f;
			const float Gx = (GroundHeight(Local.X + Eps, Local.Y) - GroundHeight(Local.X - Eps, Local.Y)) / (2.f * Eps);
			const float Gy = (GroundHeight(Local.X, Local.Y + Eps) - GroundHeight(Local.X, Local.Y - Eps)) / (2.f * Eps);
			const float NormalZ = 1.f / FMath::Sqrt(1.f + Gx * Gx + Gy * Gy);

			int32 Cat;
			if (FVector2D(Local).SizeSquared() < PadReserveSq ||
				(FVector2D(Local) - FVector2D(BaseLocal)).SizeSquared() < BaseReserveSq)
			{
				Cat = Reserved;
			}
			else if (NormalZ >= FlatNormalZ) { Cat = Flat; ++FlatCount; }
			else if (NormalZ >= SlopeNormalZ) { Cat = Slope; ++SlopeCount; }
			else { Cat = Steep; ++SteepCount; }
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

	Report(FString::Printf(TEXT("PLOTS buildable=%d flat=%d slope=%d steep=%d"),
		PlotCenters.Num(), FlatCount, SlopeCount, SteepCount));
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

void AGatersChunk::SpawnScatter(int32& OutSpawned, int64& OutSum, int32& OutReplayed)
{
	OutSpawned = 0;
	OutSum = 0;
	OutReplayed = 0;
	for (int32 I = 0; I < GridN; ++I)
	{
		for (int32 J = 0; J < GridN; ++J)
		{
			const int32 Idx = I * GridN + J;
			const int32 Cat = CellGrid[Idx];
			if (Cat != Resource && Cat != Flat)
			{
				continue;
			}
			// draws happen for every candidate regardless of the chop-skip below,
			// so recorded diffs never shift the stream for surviving instances
			const float Roll = Stream.FRandRange(0.f, 1.f);
			const float JX = Stream.FRandRange(-150.f, 150.f);
			const float JY = Stream.FRandRange(-150.f, 150.f);

			const bool bTree = (Cat == Resource);
			if (!bTree && Roll >= RockChance)
			{
				continue;
			}
			const FVector Local = CellCenterLocal(I, J);
			const FVector Pos = GetActorLocation() + FVector(
				Local.X + JX, Local.Y + JY, HeightGrid[Idx] + (bTree ? 50.f : 20.f));
			OutSum += Idx + FMath::TruncToInt(Pos.X) + FMath::TruncToInt(Pos.Y);

			if (DiffEntries.Contains(FString::Printf(TEXT("chop:%d"), Idx)))
			{
				++OutReplayed;
				continue;
			}
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AGatersScatter* Scatter = GetWorld()->SpawnActor<AGatersScatter>(Pos, FRotator::ZeroRotator, Params);
			Scatter->SetActorScale3D(bTree ? FVector(0.6f, 0.6f, 4.f) : FVector(1.5f, 1.5f, 0.6f));
			Scatter->Setup(Idx, this, bTree);
			++OutSpawned;
		}
	}
}

void AGatersChunk::SpawnClaimMarkers(int32& OutClaimed)
{
	OutClaimed = 0;
	for (int32 P = 0; P < PlotCenters.Num(); ++P)
	{
		const bool bAlreadyClaimed = DiffEntries.Contains(FString::Printf(TEXT("claim:%d"), P));
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AGatersClaimMarker* Marker = GetWorld()->SpawnActor<AGatersClaimMarker>(
			PlotCenters[P] + FVector(0, 0, 20.f), FRotator::ZeroRotator, Params);
		Marker->SetActorScale3D(FVector(1.5f, 1.5f, 0.15f));
		Marker->Setup(P, this, bAlreadyClaimed);
		if (bAlreadyClaimed)
		{
			++OutClaimed;
		}
	}
}

void AGatersChunk::StampBase(int32& OutStamped, int32& OutReplayed)
{
	OutStamped = 0;
	OutReplayed = 0;
	StampedPieces.Reset();

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
	struct FStampRow { UClass* Class; UStaticMesh* Mesh; FVector Pos; float Yaw; FVector Scale = FVector::OneVector; };
	TArray<FStampRow> Rows;
	FVector LootLocal = FVector::ZeroVector;
	bool bHaveLoot = false;
	const float Cell = 300.f;
	const FVector2D BaseCenter(BaseDist * FMath::Cos(FMath::DegreesToRadians(BaseAngle)),
		BaseDist * FMath::Sin(FMath::DegreesToRadians(BaseAngle)));

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
		if (HMax - HMin > 350.f)
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
				Rows.Add({ FoundationClass, TM.Foundation, FVector(X, Y, PivotZ), 0.f, FVector(1.f, 1.f, ScaleZ) });
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
				if (bDoorHere)
				{
					Class = DoorFrameClass;
					Mesh = TM.DoorFrame;
				}
				else if (WindowRoll < 0.22f && WindowFrameClass && TM.WindowFrame)
				{
					Class = WindowFrameClass;
					Mesh = TM.WindowFrame;
				}
				Rows.Add({ Class, Mesh, FVector(P.X, P.Y, WallPivotZ), Yaw });
				if (bDoorHere)
				{
					Rows.Add({ DoorClass, nullptr, FVector(P.X, P.Y, WallPivotZ), Yaw }); // door keeps its own mesh
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
				Rows.Add({ CeilingClass, TM.Ceiling, FVector(C.X + (I - (W - 1) * 0.5f) * Cell,
					C.Y + (J - (D - 1) * 0.5f) * Cell, RoofPivotZ), 0.f });
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
	static const TCHAR* TierNames[] = { TEXT("wood"), TEXT("stone"), TEXT("metal") };
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
					Rows.Add({ FenceClass, FenceMesh, FVector(P.X, P.Y, GroundHeight(P.X, P.Y) - 20.f - FenceZMin), Yaw });
				}
			}
		}
	}

	Report(FString::Printf(TEXT("BASE archetype=%s tier=%s main=%dx%dx%d buildings=%d"),
		ArchetypeNames[Archetype], TierNames[MainTier], MainW, MainD, MainStories, Buildings));

	for (int32 R = 0; R < Rows.Num(); ++R)
	{
		if (DiffEntries.Contains(FString::Printf(TEXT("piece:%d"), R)))
		{
			++OutReplayed;
			continue;
		}
		const FTransform SpawnXf(FRotator(0.f, Rows[R].Yaw, 0.f), GetActorLocation() + Rows[R].Pos, Rows[R].Scale);
		AActor* Piece = GetWorld()->SpawnActorDeferred<AActor>(Rows[R].Class, SpawnXf, nullptr, nullptr,
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
			if (Rows[R].Mesh)
			{
				if (UStaticMeshComponent* SMC = Piece->FindComponentByClass<UStaticMeshComponent>())
				{
					SMC->SetStaticMesh(Rows[R].Mesh);
				}
			}
			Piece->Tags.Add(TEXT("Breakable"));
			StampedPieces.Add(Piece, R);
			Piece->OnDestroyed.AddDynamic(this, &AGatersChunk::OnStampedPieceDestroyed);
			++OutStamped;
		}
	}

	if (bHaveLoot)
	{
		if (ATargetPoint* LootPoint = GetWorld()->SpawnActor<ATargetPoint>(
			GetActorLocation() + LootLocal, FRotator::ZeroRotator))
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

void AGatersChunk::ChopScatter(int32 Id, AActor* Victim)
{
	DiffEntries.Add(FString::Printf(TEXT("chop:%d"), Id));
	SaveDiff();
	if (Victim)
	{
		Victim->Destroy();
	}
	Report(FString::Printf(TEXT("CHOP id=%d entries=%d"), Id, DiffEntries.Num()));
}

void AGatersChunk::ClaimPlot(int32 PlotIndex)
{
	DiffEntries.Add(FString::Printf(TEXT("claim:%d"), PlotIndex));
	SaveDiff();
	Report(FString::Printf(TEXT("CLAIM plot=%d entries=%d"), PlotIndex, DiffEntries.Num()));
}

void AGatersChunk::Report(const FString& Line) const
{
	UE_LOG(LogTemp, Display, TEXT("[GatersChunk] %s"), *Line);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Cyan, Line);
	}
}
