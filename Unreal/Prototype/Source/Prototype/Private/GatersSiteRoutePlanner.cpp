#include "GatersSiteRoutePlanner.h"

namespace
{
bool IsValidCell(const FGatersTerrainSemanticField& Field, const FIntPoint& Cell)
{
	return Cell.X >= 0 && Cell.X < Field.CellsPerAxis
		&& Cell.Y >= 0 && Cell.Y < Field.CellsPerAxis;
}

FVector CellLocation(const FGatersTerrainSemanticField& Field, const FIntPoint& Cell)
{
	const int32 Half = Field.CellsPerAxis / 2;
	return FVector(
		(Cell.X - Half) * Field.CellSize,
		(Cell.Y - Half) * Field.CellSize,
		Field.At(Cell.X, Cell.Y).Height);
}

bool HasClearance(const FGatersTerrainSemanticField& Field, const FIntPoint& Cell)
{
	if (Field.At(Cell.X, Cell.Y).Type != EGatersTerrainSemantic::Flat)
	{
		return false;
	}
	for (int32 X = Cell.X - 1; X <= Cell.X + 1; ++X)
	{
		for (int32 Y = Cell.Y - 1; Y <= Cell.Y + 1; ++Y)
		{
			if (!IsValidCell(Field, FIntPoint(X, Y))
				|| !FGatersTerrainNavigation::IsWalkable(Field.At(X, Y).Type))
			{
				return false;
			}
		}
	}
	return true;
}

uint32 CandidateHash(int32 Seed, const FIntPoint& Cell, uint32 Salt)
{
	uint32 Hash = static_cast<uint32>(Seed) ^ (static_cast<uint32>(Cell.X) * 0x9e3779b9u)
		^ (static_cast<uint32>(Cell.Y) * 0x85ebca6bu) ^ Salt;
	Hash = (Hash ^ (Hash >> 16)) * 0x7feb352du;
	Hash = (Hash ^ (Hash >> 15)) * 0x846ca68bu;
	return Hash ^ (Hash >> 16);
}

bool SelectSite(
	const FGatersTerrainSemanticField& Field,
	const FGatersTerrainRegion& Region,
	int32 Seed,
	uint32 Salt,
	int32 MinRadius,
	int32 MaxRadius,
	int32 MinSeparation,
	const TArray<FIntPoint>& Reserved,
	FIntPoint& OutCell)
{
	const FIntPoint Center(Field.CellsPerAxis / 2, Field.CellsPerAxis / 2);
	uint32 BestHash = MAX_uint32;
	bool bFound = false;
	for (int32 X = 0; X < Field.CellsPerAxis; ++X)
	{
		for (int32 Y = 0; Y < Field.CellsPerAxis; ++Y)
		{
			const FIntPoint Cell(X, Y);
			const float Radius = static_cast<float>((Cell - Center).Size());
			if (Radius < MinRadius || Radius > MaxRadius || !Region.IsReachable(Cell)
				|| !HasClearance(Field, Cell))
			{
				continue;
			}
			if (Reserved.ContainsByPredicate([&Cell, MinSeparation](const FIntPoint& Other)
			{
				return (Cell - Other).SizeSquared() < FMath::Square(MinSeparation);
			}))
			{
				continue;
			}
			const uint32 Hash = CandidateHash(Seed, Cell, Salt);
			if (!bFound || Hash < BestHash)
			{
				BestHash = Hash;
				OutCell = Cell;
				bFound = true;
			}
		}
	}
	return bFound;
}

void AddRoute(
	FGatersSiteRoutePlan& Plan,
	const FGatersTerrainSemanticField& Field,
	const FString& Id,
	const FGatersPlannedSite& From,
	const FGatersPlannedSite& To)
{
	FGatersPlannedRoute Route;
	Route.Id = Id;
	Route.FromSiteId = From.Id;
	Route.ToSiteId = To.Id;
	Route.Path = FGatersTerrainNavigation::FindPath(Field, From.Cell, To.Cell);
	if (!Route.Path.bFound)
	{
		Plan.Diagnostics.Add(FString::Printf(TEXT("route %s is unreachable"), *Id));
	}
	Plan.Routes.Add(MoveTemp(Route));
}
}

const FGatersPlannedSite* FGatersSiteRoutePlan::FindSite(const FString& Id) const
{
	return Sites.FindByPredicate([&Id](const FGatersPlannedSite& Site)
	{
		return Site.Id == Id;
	});
}

FGatersSiteRoutePlan FGatersSiteRoutePlanner::Plan(
	const FGatersTerrainSemanticField& Field,
	int32 Seed,
	const FVector2D& RaidBaseLocation)
{
	FGatersSiteRoutePlan Result;
	if (Field.CellsPerAxis < 3 || Field.CellSize <= 0.f)
	{
		Result.Diagnostics.Add(TEXT("semantic field is too small"));
		return Result;
	}

	const int32 Half = Field.CellsPerAxis / 2;
	const FIntPoint ArrivalCell(Half, Half);
	const FIntPoint RaidBaseCell(
		FMath::RoundToInt(RaidBaseLocation.X / Field.CellSize) + Half,
		FMath::RoundToInt(RaidBaseLocation.Y / Field.CellSize) + Half);
	if (!IsValidCell(Field, RaidBaseCell)
		|| !FGatersTerrainNavigation::IsWalkable(Field.At(RaidBaseCell.X, RaidBaseCell.Y).Type)
		|| !FGatersTerrainNavigation::FindPath(Field, ArrivalCell, RaidBaseCell).bFound)
	{
		Result.Diagnostics.Add(TEXT("raid base is unreachable from arrival"));
		return Result;
	}

	Result.Sites.Add({ TEXT("site:arrival"), EGatersPlannedSiteKind::Arrival,
		ArrivalCell, CellLocation(Field, ArrivalCell) });
	Result.Sites.Add({ TEXT("site:raid-base:0"), EGatersPlannedSiteKind::RaidBase,
		RaidBaseCell, CellLocation(Field, RaidBaseCell) });
	const FGatersTerrainRegion Region = FGatersTerrainNavigation::Analyze(Field, ArrivalCell);
	const int32 VillageMinRadius = FMath::Max(2, Field.CellsPerAxis / 6);
	const int32 VillageMaxRadius = FMath::Max(VillageMinRadius, Field.CellsPerAxis / 3);
	const int32 LandmarkMinRadius = FMath::Max(2, Field.CellsPerAxis / 8);
	const int32 LandmarkMaxRadius = FMath::Max(LandmarkMinRadius, Field.CellsPerAxis / 2);
	const int32 MinSeparation = FMath::Max(2, Field.CellsPerAxis / 10);
	TArray<FIntPoint> Reserved{ ArrivalCell, RaidBaseCell };

	FIntPoint VillageCell;
	if (!SelectSite(Field, Region, Seed, 0x56494c4cu, VillageMinRadius,
		VillageMaxRadius, MinSeparation, Reserved, VillageCell))
	{
		Result.Diagnostics.Add(TEXT("no valid village site"));
		return Result;
	}
	Result.Sites.Add({ TEXT("site:village:0"), EGatersPlannedSiteKind::Village,
		VillageCell, CellLocation(Field, VillageCell) });
	Reserved.Add(VillageCell);

	FIntPoint LandmarkCell;
	if (!SelectSite(Field, Region, Seed, 0x4c414e44u, LandmarkMinRadius,
		LandmarkMaxRadius, MinSeparation, Reserved, LandmarkCell))
	{
		Result.Diagnostics.Add(TEXT("no valid landmark site"));
		return Result;
	}
	Result.Sites.Add({ TEXT("site:landmark:0"), EGatersPlannedSiteKind::Landmark,
		LandmarkCell, CellLocation(Field, LandmarkCell) });

	AddRoute(Result, Field, TEXT("route:arrival-base"), Result.Sites[0], Result.Sites[1]);
	AddRoute(Result, Field, TEXT("route:arrival-village"), Result.Sites[0], Result.Sites[2]);
	AddRoute(Result, Field, TEXT("route:village-landmark"), Result.Sites[2], Result.Sites[3]);
	Result.bValid = Result.Diagnostics.IsEmpty();
	return Result;
}
