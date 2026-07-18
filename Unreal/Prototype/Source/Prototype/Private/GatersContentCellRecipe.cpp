#include "GatersContentCellRecipe.h"

#include "GatersTerrainSemanticField.h"
#include "Misc/Crc.h"

namespace
{
constexpr int32 CandidatesPerAxis = 4;

uint32 ContentHash(int32 Seed, const FIntPoint& Cell, int32 Candidate)
{
	uint32 Hash = static_cast<uint32>(Seed);
	for (const uint32 Value : {
		static_cast<uint32>(Cell.X), static_cast<uint32>(Cell.Y), static_cast<uint32>(Candidate)})
	{
		Hash ^= Value + 0x9e3779b9u + (Hash << 6) + (Hash >> 2);
		Hash = (Hash ^ (Hash >> 16)) * 0x7feb352du;
		Hash = (Hash ^ (Hash >> 15)) * 0x846ca68bu;
		Hash ^= Hash >> 16;
	}
	return Hash;
}

float HashUnit(uint32 Hash, int32 Shift)
{
	return ((Hash >> Shift) & 0xffu) / 255.f;
}
}

FGatersContentCellRecipe FGatersContentCellRecipe::Generate(
	const FIntPoint& InCell,
	float InCellSize,
	const FGatersEnvironment& Environment,
	const FGatersContentCellSemantics& Semantics)
{
	check(InCellSize > 0.f);
	check(Semantics.MinGroundNormalZ >= 0.f && Semantics.MinGroundNormalZ <= 1.f);
	check(Semantics.PadRadius >= 0.f);
	check(Semantics.RouteTargetClearance >= 0.f);

	FGatersContentCellRecipe Result;
	Result.WorldSeed = Environment.Seed;
	Result.Cell = InCell;
	Result.CellSize = InCellSize;
	Result.Coverage.CandidateCount = CandidatesPerAxis * CandidatesPerAxis;
	TArray<int32> Candidates;
	Candidates.SetNumUninitialized(Result.Coverage.CandidateCount);
	for (int32 Candidate = 0; Candidate < Candidates.Num(); ++Candidate)
	{
		Candidates[Candidate] = Candidate;
	}
	Candidates.Sort([&Environment, &InCell](int32 A, int32 B)
	{
		const uint32 HashA = ContentHash(Environment.Seed, InCell, A);
		const uint32 HashB = ContentHash(Environment.Seed, InCell, B);
		return HashA == HashB ? A < B : HashA < HashB;
	});

	const float Spacing = InCellSize / CandidatesPerAxis;
	const FVector2D Center(InCell.X * InCellSize, InCell.Y * InCellSize);
	for (const int32 Candidate : Candidates)
	{
		const uint32 Hash = ContentHash(Environment.Seed, InCell, Candidate);
		const int32 X = Candidate / CandidatesPerAxis;
		const int32 Y = Candidate % CandidatesPerAxis;
		const FVector2D Local(
			-InCellSize * 0.5f + Spacing * (X + 0.25f + HashUnit(Hash, 0) * 0.5f),
			-InCellSize * 0.5f + Spacing * (Y + 0.25f + HashUnit(Hash, 8) * 0.5f));
		const FVector2D Point = Center + Local;
		const bool bInsideArrival = Semantics.PadRadius > 0.f &&
			Point.SizeSquared() <= FMath::Square(Semantics.PadRadius);
		const bool bInsideRouteTarget = Semantics.RouteTargetClearance > 0.f &&
			FVector2D::DistSquared(Point, Semantics.RouteTarget) <=
				FMath::Square(Semantics.RouteTargetClearance);
		if (bInsideArrival || bInsideRouteTarget)
		{
			++Result.Coverage.ReservedRejectedCount;
			continue;
		}
		const float Height = FGatersTerrainSemanticField::MaterializedHeightAt(
			Environment, Point, Semantics.PadRadius, Semantics.RouteTarget);
		if (Environment.HasWater() && Height <= Environment.WaterHeight + Semantics.WaterClearance)
		{
			++Result.Coverage.WaterRejectedCount;
			continue;
		}

		const FVector Normal = FGatersTerrainSemanticField::MaterializedNormalAt(
			Environment, Point, Spacing * 0.25f, Semantics.PadRadius, Semantics.RouteTarget);
		if (Normal.Z < Semantics.MinGroundNormalZ)
		{
			++Result.Coverage.SteepRejectedCount;
			continue;
		}
		if (Result.Placements.Num() >= Result.MaxPlacements)
		{
			++Result.Coverage.BudgetRejectedCount;
			continue;
		}

		FGatersContentCellPlacement& Placement = Result.Placements.AddDefaulted_GetRef();
		Placement.Id = FString::Printf(TEXT("content:%d:%d:%d:%d"),
			Environment.Seed, InCell.X, InCell.Y, Candidate);
		const bool bTree = (Hash & 1u) == 0u;
		Placement.Kind = bTree
			? EGatersRecipeNodeKind::ScatterTree
			: EGatersRecipeNodeKind::ScatterRock;
		Placement.ContentKey = bTree ? TEXT("environment.tree") : TEXT("environment.rock");
		const uint32 TransformHash = FCrc::StrCrc32(*Placement.Id) * 2654435761u;
		const float Variation = 0.85f + (TransformHash % 31u) / 100.f;
		const FVector BaseScale = bTree
			? FVector(0.8f, 0.8f, 5.f)
			: FVector(1.8f, 1.4f, 0.8f);
		Placement.Transform = FTransform(
			FRotator(0.f, (TransformHash >> 8) % 360u, 0.f),
			FVector(Point.X, Point.Y, Height),
			BaseScale * Variation);
		++Result.Coverage.PlacedCount;
	}
	return Result;
}
