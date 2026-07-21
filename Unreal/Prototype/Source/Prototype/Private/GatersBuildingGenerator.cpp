#include "GatersBuildingGenerator.h"

#include "GatersTerrainNavigation.h"

namespace
{
constexpr float ModuleSpan = 250.f;
constexpr float WallHeight = 260.f;
constexpr float WallThickness = 20.f;
constexpr float FoundationDepth = 120.f;
constexpr float FoundationTop = 20.f;
constexpr float FloorThickness = 30.f;
constexpr float DoorOpeningWidthRatio = 0.45f;
constexpr float DoorOpeningHeadroomRatio = 0.78f;

const TCHAR* KindName(const EGatersBuildingModuleKind Kind)
{
	switch (Kind)
	{
	case EGatersBuildingModuleKind::Foundation: return TEXT("foundation");
	case EGatersBuildingModuleKind::Floor: return TEXT("floor");
	case EGatersBuildingModuleKind::Wall: return TEXT("wall");
	case EGatersBuildingModuleKind::DoorWall: return TEXT("door");
	case EGatersBuildingModuleKind::Roof: return TEXT("roof");
	default: checkNoEntry(); return TEXT("unknown");
	}
}

bool IsValidCell(const FGatersTerrainSemanticField& Field, const FIntPoint& Cell)
{
	return Cell.X >= 0 && Cell.X < Field.CellsPerAxis
		&& Cell.Y >= 0 && Cell.Y < Field.CellsPerAxis;
}

void AddModule(
	FGatersBuildingAssembly& Assembly,
	const FGatersSettlementBuilding& Building,
	const EGatersBuildingModuleKind Kind,
	const int32 FloorIndex,
	const FVector& LocalLocation,
	const FVector& Scale,
	const float LocalYaw = 0.f)
{
	const int32 KindIndex = Assembly.Count(Kind);
	const FRotator BuildingRotation(0.f, Building.Yaw, 0.f);
	const FVector WorldLocation = Building.Location
		+ BuildingRotation.RotateVector(LocalLocation);
	FGatersBuildingModule& Module = Assembly.Modules.AddDefaulted_GetRef();
	Module.Id = FString::Printf(TEXT("%s:module:%s:%d"),
		*Building.Id, KindName(Kind), KindIndex);
	Module.Kind = Kind;
	Module.ContentKey = FString::Printf(TEXT("building.%s"), KindName(Kind));
	Module.FloorIndex = FloorIndex;
	Module.Transform = FTransform(
		FRotator(0.f, Building.Yaw + LocalYaw, 0.f), WorldLocation, Scale);
}
}

FString FGatersBuildingAssembly::CanonicalText() const
{
	FString Result = FString::Printf(TEXT("v=%d;generated=%d;building=%s;cell=%d,%d;size=%d,%d;floors=%d"),
		GeneratorVersion, bGenerated ? 1 : 0, *BuildingId, GroundCell.X, GroundCell.Y,
		FootprintWidthCells, FootprintDepthCells, FloorCount);
	for (const FGatersBuildingModule& Module : Modules)
	{
		const FVector Location = Module.Transform.GetLocation();
		const FVector Scale = Module.Transform.GetScale3D();
		Result += FString::Printf(TEXT(";module=%s,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%s"),
			*Module.Id, static_cast<int32>(Module.Kind), Module.FloorIndex,
			Location.X, Location.Y, Location.Z,
			Module.Transform.Rotator().Yaw, Scale.X, Scale.Y, Scale.Z,
			*Module.ContentKey);
	}
	for (const FGatersBuildingUsableSpace& Space : UsableSpaces)
	{
		Result += FString::Printf(
			TEXT(";usable=%s,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f"),
			*Space.Id, Space.FloorIndex, Space.Center.X, Space.Center.Y, Space.Center.Z,
			Space.Extent.X, Space.Extent.Y, Space.Extent.Z);
		for (const FString& SourceId : Space.SourceIds)
		{
			Result += FString::Printf(TEXT(";usable-source=%s"), *SourceId);
		}
	}
	for (const FGatersBuildingOpening& Opening : Openings)
	{
		const FVector Location = Opening.Transform.GetLocation();
		Result += FString::Printf(
			TEXT(";opening=%s,%s,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f"),
			*Opening.Id, *Opening.SourceModuleId, Opening.FloorIndex,
			Location.X, Location.Y, Location.Z, Opening.Transform.Rotator().Yaw,
			Opening.Width, Opening.Headroom, Opening.Depth);
		for (const FString& SourceId : Opening.SourceIds)
		{
			Result += FString::Printf(TEXT(";opening-source=%s"), *SourceId);
		}
	}
	return Result;
}

int32 FGatersBuildingAssembly::Count(const EGatersBuildingModuleKind Kind) const
{
	int32 Result = 0;
	for (const FGatersBuildingModule& Module : Modules)
	{
		Result += Module.Kind == Kind ? 1 : 0;
	}
	return Result;
}

FGatersBuildingAssembly FGatersBuildingGenerator::Generate(
	const FGatersTerrainSemanticField& Field,
	const FGatersSettlementBuilding& Building)
{
	FGatersBuildingAssembly Result;
	Result.BuildingId = Building.Id;
	Result.GroundCell = Building.Cell;
	Result.FootprintWidthCells = Building.FootprintWidthCells;
	Result.FootprintDepthCells = Building.FootprintDepthCells;
	Result.FloorCount = Building.FloorCount;
	if (Building.Id.IsEmpty() || !IsValidCell(Field, Building.Cell)
		|| !FGatersTerrainNavigation::IsWalkable(Field.At(Building.Cell.X, Building.Cell.Y).Type)
		|| Building.FootprintWidthCells < 1 || Building.FootprintWidthCells > 2
		|| Building.FootprintDepthCells < 1 || Building.FootprintDepthCells > 2
		|| Building.FloorCount < 1 || Building.FloorCount > 3)
	{
		Result.Diagnostics.Add(TEXT("building anchor or massing is outside the module contract"));
		return Result;
	}

	const float Width = Building.FootprintWidthCells * ModuleSpan;
	const float Depth = Building.FootprintDepthCells * ModuleSpan;
	AddModule(Result, Building, EGatersBuildingModuleKind::Foundation, 0,
		FVector(0.f, 0.f, FoundationTop - FoundationDepth * 0.5f),
		FVector(Width / 100.f, Depth / 100.f, FoundationDepth / 100.f));
	for (int32 Floor = 1; Floor < Building.FloorCount; ++Floor)
	{
		AddModule(Result, Building, EGatersBuildingModuleKind::Floor, Floor,
			FVector(0.f, 0.f, FoundationTop + Floor * WallHeight),
			FVector(Width / 100.f, Depth / 100.f, FloorThickness / 100.f));
	}

	for (int32 Floor = 0; Floor < Building.FloorCount; ++Floor)
	{
		const float Z = FoundationTop + Floor * WallHeight + WallHeight * 0.5f;
		for (int32 Y = 0; Y < Building.FootprintDepthCells; ++Y)
		{
			const float Along = (Y - (Building.FootprintDepthCells - 1) * 0.5f) * ModuleSpan;
			const EGatersBuildingModuleKind FrontKind =
				Floor == 0 && Y == (Building.FootprintDepthCells - 1) / 2
				? EGatersBuildingModuleKind::DoorWall
				: EGatersBuildingModuleKind::Wall;
			AddModule(Result, Building, FrontKind, Floor,
				FVector(Width * 0.5f, Along, Z),
				FVector(WallThickness / 100.f, ModuleSpan / 100.f, WallHeight / 100.f));
			AddModule(Result, Building, EGatersBuildingModuleKind::Wall, Floor,
				FVector(-Width * 0.5f, Along, Z),
				FVector(WallThickness / 100.f, ModuleSpan / 100.f, WallHeight / 100.f));
		}
		for (int32 X = 0; X < Building.FootprintWidthCells; ++X)
		{
			const float Along = (X - (Building.FootprintWidthCells - 1) * 0.5f) * ModuleSpan;
			AddModule(Result, Building, EGatersBuildingModuleKind::Wall, Floor,
				FVector(Along, Depth * 0.5f, Z),
				FVector(ModuleSpan / 100.f, WallThickness / 100.f, WallHeight / 100.f));
			AddModule(Result, Building, EGatersBuildingModuleKind::Wall, Floor,
				FVector(Along, -Depth * 0.5f, Z),
				FVector(ModuleSpan / 100.f, WallThickness / 100.f, WallHeight / 100.f));
		}
	}
	AddModule(Result, Building, EGatersBuildingModuleKind::Roof, Building.FloorCount,
		FVector(0.f, 0.f, FoundationTop + Building.FloorCount * WallHeight),
		FVector(Width / 100.f + 0.4f, Depth / 100.f + 0.4f, FloorThickness / 100.f));

	const FRotator BuildingRotation(0.f, Building.Yaw, 0.f);
	for (int32 Floor = 0; Floor < Building.FloorCount; ++Floor)
	{
		const float Bottom = FoundationTop + Floor * WallHeight
			+ (Floor == 0 ? 0.f : FloorThickness * 0.5f);
		const float Top = FoundationTop + (Floor + 1) * WallHeight
			- FloorThickness * 0.5f;
		FGatersBuildingUsableSpace& Space = Result.UsableSpaces.AddDefaulted_GetRef();
		Space.Id = FString::Printf(TEXT("%s:usable:floor:%d"), *Building.Id, Floor);
		Space.FloorIndex = Floor;
		Space.Center = Building.Location + BuildingRotation.RotateVector(
			FVector(0.f, 0.f, (Bottom + Top) * 0.5f));
		Space.Extent = FVector(
			Width * 0.5f - WallThickness,
			Depth * 0.5f - WallThickness,
			(Top - Bottom) * 0.5f);
		Space.SourceIds = {Building.Id};
	}

	const FGatersBuildingModule* Door = Result.Modules.FindByPredicate(
		[](const FGatersBuildingModule& Module)
		{
			return Module.Kind == EGatersBuildingModuleKind::DoorWall;
		});
	if (Door)
	{
		FGatersBuildingOpening& Opening = Result.Openings.AddDefaulted_GetRef();
		Opening.Id = Building.Id + TEXT(":opening:entrance:0");
		Opening.SourceModuleId = Door->Id;
		Opening.FloorIndex = Door->FloorIndex;
		Opening.Width = ModuleSpan * DoorOpeningWidthRatio;
		Opening.Headroom = WallHeight * DoorOpeningHeadroomRatio;
		Opening.Depth = WallThickness;
		const FVector Location = Door->Transform.GetLocation()
			+ FVector(0.f, 0.f, (Opening.Headroom - WallHeight) * 0.5f);
		Opening.Transform = FTransform(Door->Transform.GetRotation(), Location);
		Opening.SourceIds = {Building.Id, Door->Id};
	}
	Result.bGenerated = true;
	return Result;
}
