#include "GatersSettlementGenerator.h"

#include "GatersTerrainNavigation.h"

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

uint32 CandidateHash(const int32 Seed, const FIntPoint& Cell, const uint32 Salt)
{
	uint32 Hash = static_cast<uint32>(Seed)
		^ (static_cast<uint32>(Cell.X) * 0x9e3779b9u)
		^ (static_cast<uint32>(Cell.Y) * 0x85ebca6bu)
		^ Salt;
	Hash = (Hash ^ (Hash >> 16)) * 0x7feb352du;
	Hash = (Hash ^ (Hash >> 15)) * 0x846ca68bu;
	return Hash ^ (Hash >> 16);
}

float HashUnit(const uint32 Hash)
{
	return static_cast<float>(Hash & 0xffffu) / 65535.f;
}

FGatersSettlementProfile MakeProfile(const int32 Seed, const FIntPoint& Center)
{
	FGatersSettlementProfile Profile;
	Profile.HeightBias = HashUnit(CandidateHash(Seed, Center, 0x48454947u));
	Profile.FootprintBias = HashUnit(CandidateHash(Seed, Center, 0x464f4f54u));
	Profile.DensityBias = HashUnit(CandidateHash(Seed, Center, 0x44454e53u));
	return Profile;
}

void ApplyMassing(
	const FGatersSettlementProfile& Profile,
	const int32 Seed,
	const int32 RoleIndex,
	FGatersSettlementBuilding& Building)
{
	const FIntPoint SaltCell(RoleIndex, static_cast<int32>(Building.Role));
	const float WidthRoll = HashUnit(CandidateHash(Seed, SaltCell, 0x57494454u));
	const float DepthRoll = HashUnit(CandidateHash(Seed, SaltCell, 0x44455054u));
	const float HeightRoll = HashUnit(CandidateHash(Seed, SaltCell, 0x464c4f52u));
	Building.FootprintWidthCells =
		FMath::Lerp(Profile.FootprintBias, WidthRoll, 0.35f) > 0.52f ? 2 : 1;
	Building.FootprintDepthCells =
		FMath::Lerp(Profile.FootprintBias, DepthRoll, 0.45f) > 0.60f ? 2 : 1;
	const float Height = FMath::Lerp(Profile.HeightBias, HeightRoll, 0.25f);
	Building.FloorCount = 1 + (Height > 0.42f ? 1 : 0) + (Height > 0.76f ? 1 : 0);
}

const TCHAR* RoleName(const EGatersSettlementRole Role)
{
	switch (Role)
	{
	case EGatersSettlementRole::Home: return TEXT("home");
	case EGatersSettlementRole::Community: return TEXT("community");
	case EGatersSettlementRole::Workshop: return TEXT("workshop");
	case EGatersSettlementRole::Storage: return TEXT("storage");
	default: checkNoEntry(); return TEXT("unknown");
	}
}

bool IsReservedNear(const TSet<FIntPoint>& Reserved, const FIntPoint& Cell)
{
	for (const FIntPoint& Other : Reserved)
	{
		if (FMath::Max(FMath::Abs(Cell.X - Other.X),
			FMath::Abs(Cell.Y - Other.Y)) < 2)
		{
			return true;
		}
	}
	return false;
}

struct FRoleEntry
{
	EGatersSettlementRole Role;
	int32 Stage;
};

constexpr FRoleEntry RoleEntries[] = {
	{EGatersSettlementRole::Community, 0},
	{EGatersSettlementRole::Workshop, 0},
	{EGatersSettlementRole::Storage, 0},
	{EGatersSettlementRole::Home, 0},
	{EGatersSettlementRole::Home, 0},
	{EGatersSettlementRole::Home, 0},
	{EGatersSettlementRole::Home, 1},
	{EGatersSettlementRole::Home, 1},
	{EGatersSettlementRole::Workshop, 1},
	{EGatersSettlementRole::Storage, 1},
	{EGatersSettlementRole::Home, 2},
	{EGatersSettlementRole::Home, 2},
	{EGatersSettlementRole::Home, 2},
	{EGatersSettlementRole::Community, 2},
	{EGatersSettlementRole::Workshop, 2},
	{EGatersSettlementRole::Storage, 2}};
}

FString FGatersSettlementProfile::CanonicalText() const
{
	return FString::Printf(TEXT("v=%d;height=%.5f;footprint=%.5f;density=%.5f"),
		Version, HeightBias, FootprintBias, DensityBias);
}

FString FGatersSettlementPlan::CanonicalText() const
{
	FString Result = FString::Printf(TEXT("v=%d;growth=%d;site=%s;center=%d,%d;generated=%d;profile=%s"),
		GeneratorVersion, GrowthStage, *SiteId, CenterCell.X, CenterCell.Y, bGenerated ? 1 : 0,
		*Profile.CanonicalText());
	for (const FGatersSettlementBuilding& Building : Buildings)
	{
		Result += FString::Printf(TEXT(";building=%s,%s,%s,%d,%d,%d,%d,%d,%d,%.3f,%s,%d,%d,%d"),
			*Building.Id,
			*Building.ParcelId,
			*Building.GrowthFrontId,
			Building.IntroducedStage,
			static_cast<int32>(Building.Role),
			Building.Cell.X,
			Building.Cell.Y,
			Building.EntranceCell.X,
			Building.EntranceCell.Y,
			Building.Yaw,
			*Building.ContentKey,
			Building.FootprintWidthCells,
			Building.FootprintDepthCells,
			Building.FloorCount);
	}
	for (const FGatersSettlementParcel& Parcel : Parcels)
	{
		Result += FString::Printf(TEXT(";parcel=%s,%d,%d,%d,%d,%d,%s,%s"),
			*Parcel.Id, Parcel.IntroducedStage, Parcel.Cell.X, Parcel.Cell.Y,
			Parcel.EntranceCell.X, Parcel.EntranceCell.Y,
			*Parcel.SupportKey, *Parcel.AccessKey);
	}
	for (const FGatersSettlementGrowthFront& Front : GrowthFronts)
	{
		Result += FString::Printf(TEXT(";front=%s,%d"), *Front.Id, Front.Sector);
	}
	for (const FIntPoint& Cell : PathCells)
	{
		Result += FString::Printf(TEXT(";path=%d,%d"), Cell.X, Cell.Y);
	}
	return Result;
}

const FGatersSettlementParcel* FGatersSettlementPlan::FindParcel(const FString& Id) const
{
	return Parcels.FindByPredicate([&Id](const FGatersSettlementParcel& Parcel)
	{
		return Parcel.Id == Id;
	});
}

FString FGatersSettlementPlan::StablePathId(const FIntPoint& Cell)
{
	return FString::Printf(TEXT("settlement:path:%d:%d"), Cell.X, Cell.Y);
}

const FGatersSettlementBuilding* FGatersSettlementPlan::FindBuilding(const FString& Id) const
{
	return Buildings.FindByPredicate([&Id](const FGatersSettlementBuilding& Building)
	{
		return Building.Id == Id;
	});
}

FGatersSettlementPlan FGatersSettlementGenerator::Generate(
	const FGatersTerrainSemanticField& Field,
	const int32 Seed,
	const FGatersPlannedSite& VillageSite,
	const int32 GrowthStage)
{
	FGatersSettlementPlan Result;
	Result.GrowthStage = GrowthStage;
	Result.SiteId = VillageSite.Id;
	Result.CenterCell = VillageSite.Cell;
	Result.CenterLocation = VillageSite.Location;
	Result.Profile = MakeProfile(Seed, VillageSite.Cell);
	if (!IsSupportedGrowthStage(GrowthStage))
	{
		Result.Diagnostics.Add(TEXT("growth stage is outside the supported contract"));
		return Result;
	}
	if (VillageSite.Kind != EGatersPlannedSiteKind::Village
		|| !IsValidCell(Field, VillageSite.Cell)
		|| !FGatersTerrainNavigation::IsWalkable(
			Field.At(VillageSite.Cell.X, VillageSite.Cell.Y).Type))
	{
		Result.Diagnostics.Add(TEXT("village site is invalid or unwalkable"));
		return Result;
	}

	TSet<FIntPoint> Reserved{VillageSite.Cell};
	TSet<FIntPoint> BuildingCells;
	TSet<FIntPoint> PathCells;
	TSet<int32> UsedSectors;
	TMap<EGatersSettlementRole, int32> RoleCounts;
	const int32 FrontOffset = static_cast<int32>(
		CandidateHash(Seed, VillageSite.Cell, 0x46524f4eu) & 1u);
	for (int32 Index = 0; Index < 4; ++Index)
	{
		Result.GrowthFronts.Add({
			FString::Printf(TEXT("settlement:front:%d"), Index),
			(FrontOffset + Index * 2) % 8});
	}

	for (int32 RoleIndex = 0; RoleIndex < UE_ARRAY_COUNT(RoleEntries); ++RoleIndex)
	{
		const FRoleEntry Entry = RoleEntries[RoleIndex];
		if (Entry.Stage > GrowthStage)
		{
			break;
		}
		const EGatersSettlementRole Role = Entry.Role;
		const int32 SearchRadius = FMath::Min(
			6 + Entry.Stage * 3,
			FMath::Max(2, Field.CellsPerAxis / 3 + Entry.Stage * 3));
		float BestScore = TNumericLimits<float>::Max();
		FIntPoint BestCell = FIntPoint::ZeroValue;
		FGatersTerrainPath BestPath;
		int32 BestSector = 0;
		FString BestGrowthFrontId;
		bool bBestSectorUnused = false;
		bool bFound = false;
		for (int32 X = VillageSite.Cell.X - SearchRadius;
			X <= VillageSite.Cell.X + SearchRadius; ++X)
		{
			for (int32 Y = VillageSite.Cell.Y - SearchRadius;
				Y <= VillageSite.Cell.Y + SearchRadius; ++Y)
			{
				const FIntPoint Cell(X, Y);
				const FIntPoint Offset = Cell - VillageSite.Cell;
				if (!IsValidCell(Field, Cell)
					|| Offset.SizeSquared() < 4
					|| Offset.SizeSquared() > FMath::Square(SearchRadius)
					|| Field.At(X, Y).Type != EGatersTerrainSemantic::Flat)
				{
					continue;
				}
				if (IsReservedNear(Reserved, Cell))
				{
					continue;
				}
				const FGatersTerrainPath Path = FGatersTerrainNavigation::FindPath(
					Field, Cell, VillageSite.Cell);
				if (!Path.bFound || Path.Cells.Num() < 2)
				{
					continue;
				}
				bool bPathCrossesBuilding = false;
				for (int32 PathIndex = 1; PathIndex < Path.Cells.Num(); ++PathIndex)
				{
					bPathCrossesBuilding |= BuildingCells.Contains(Path.Cells[PathIndex]);
				}
				if (PathCells.Contains(Cell) || bPathCrossesBuilding)
				{
					continue;
				}
				const uint32 Hash = CandidateHash(Seed, Cell,
					0x53455454u + static_cast<uint32>(RoleIndex) * 0x9e3779b9u);
				const float DistanceAlpha = static_cast<float>(Offset.SizeSquared())
					/ static_cast<float>(FMath::Square(SearchRadius));
				const float CandidateScore = HashUnit(Hash)
					+ (Result.Profile.DensityBias * 2.f - 1.f) * DistanceAlpha * 0.75f;
				const float Angle = FMath::Atan2(
					static_cast<float>(Offset.Y), static_cast<float>(Offset.X));
				const int32 Sector = FMath::Clamp(
					FMath::FloorToInt((Angle + PI) / (PI / 4.f)), 0, 7);
				const FGatersSettlementGrowthFront* GrowthFront =
					Result.GrowthFronts.FindByPredicate([Sector](const FGatersSettlementGrowthFront& Front)
					{
						const int32 Delta = FMath::Abs(Front.Sector - Sector);
						return FMath::Min(Delta, 8 - Delta) <= 1;
					});
				if (Entry.Stage > 0 && !GrowthFront)
				{
					continue;
				}
				const bool bSectorUnused = !UsedSectors.Contains(Sector);
				if (!bFound || (bSectorUnused && !bBestSectorUnused)
					|| (bSectorUnused == bBestSectorUnused && CandidateScore < BestScore))
				{
					bFound = true;
					BestScore = CandidateScore;
					BestCell = Cell;
					BestPath = Path;
					BestSector = Sector;
					BestGrowthFrontId = GrowthFront ? GrowthFront->Id : FString();
					bBestSectorUnused = bSectorUnused;
				}
			}
		}
		if (!bFound)
		{
			Result.Diagnostics.Add(TEXT("not enough reachable building sites"));
			return Result;
		}

		const int32 RoleNumber = RoleCounts.FindOrAdd(Role)++;
		const FVector Location = CellLocation(Field, BestCell);
		FGatersSettlementBuilding& Building = Result.Buildings.AddDefaulted_GetRef();
		Building.Id = FString::Printf(TEXT("settlement:building:%s:%d"), RoleName(Role), RoleNumber);
		Building.ParcelId = FString::Printf(TEXT("settlement:parcel:%s:%d"), RoleName(Role), RoleNumber);
		Building.GrowthFrontId = Entry.Stage > 0 ? BestGrowthFrontId : FString();
		Building.IntroducedStage = Entry.Stage;
		Building.Role = Role;
		Building.ContentKey = FString::Printf(TEXT("settlement.%s"), RoleName(Role));
		Building.Cell = BestCell;
		Building.EntranceCell = BestPath.Cells[1];
		Building.Location = Location;
		const FVector Direction = CellLocation(Field, Building.EntranceCell) - Location;
		Building.Yaw = FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X));
		ApplyMassing(Result.Profile, Seed, RoleIndex, Building);
		Result.Parcels.Add({
			Building.ParcelId,
			Entry.Stage,
			BestCell,
			Building.EntranceCell,
			Location});
		Reserved.Add(BestCell);
		BuildingCells.Add(BestCell);
		UsedSectors.Add(BestSector);
		for (int32 PathIndex = 1; PathIndex < BestPath.Cells.Num(); ++PathIndex)
		{
			PathCells.Add(BestPath.Cells[PathIndex]);
		}
	}

	for (const FIntPoint& Cell : PathCells)
	{
		Result.PathCells.Add(Cell);
	}
	Result.PathCells.Sort([](const FIntPoint& A, const FIntPoint& B)
	{
		return A.X == B.X ? A.Y < B.Y : A.X < B.X;
	});
	Result.bGenerated = true;
	return Result;
}

bool FGatersSettlementGenerator::IsSupportedGrowthStage(const int32 GrowthStage)
{
	return GrowthStage >= 0 && GrowthStage <= 2;
}

int32 FGatersSettlementGenerator::ExpectedBuildingCount(const int32 GrowthStage)
{
	int32 Count = 0;
	for (const FRoleEntry& Entry : RoleEntries)
	{
		Count += Entry.Stage <= GrowthStage ? 1 : 0;
	}
	return Count;
}

int32 FGatersSettlementGenerator::ExpectedHomeCount(const int32 GrowthStage)
{
	int32 Count = 0;
	for (const FRoleEntry& Entry : RoleEntries)
	{
		Count += Entry.Stage <= GrowthStage && Entry.Role == EGatersSettlementRole::Home ? 1 : 0;
	}
	return Count;
}

float FGatersSettlementGenerator::MaxRadiusCells(const int32 GrowthStage)
{
	return 6.f + GrowthStage * 3.f;
}
