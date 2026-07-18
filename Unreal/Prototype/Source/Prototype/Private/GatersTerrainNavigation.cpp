#include "GatersTerrainNavigation.h"

#include "Containers/Queue.h"

namespace
{
const FIntPoint Directions[] = {
	FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) };

bool IsValid(const FGatersTerrainSemanticField& Field, const FIntPoint& Cell)
{
	return Cell.X >= 0 && Cell.X < Field.CellsPerAxis
		&& Cell.Y >= 0 && Cell.Y < Field.CellsPerAxis;
}

int32 ToIndex(int32 CellsPerAxis, const FIntPoint& Cell)
{
	return Cell.X * CellsPerAxis + Cell.Y;
}

FIntPoint ToCell(int32 CellsPerAxis, int32 Index)
{
	return FIntPoint(Index / CellsPerAxis, Index % CellsPerAxis);
}

void Flood(
	const FGatersTerrainSemanticField& Field,
	const FIntPoint& Start,
	TBitArray<>& Visited)
{
	if (!IsValid(Field, Start)
		|| !FGatersTerrainNavigation::IsWalkable(Field.At(Start.X, Start.Y).Type))
	{
		return;
	}

	TQueue<FIntPoint> Queue;
	Visited[ToIndex(Field.CellsPerAxis, Start)] = true;
	Queue.Enqueue(Start);
	FIntPoint Current;
	while (Queue.Dequeue(Current))
	{
		for (const FIntPoint& Direction : Directions)
		{
			const FIntPoint Next = Current + Direction;
			if (!IsValid(Field, Next))
			{
				continue;
			}
			const int32 NextIndex = ToIndex(Field.CellsPerAxis, Next);
			if (!Visited[NextIndex]
				&& FGatersTerrainNavigation::IsWalkable(Field.At(Next.X, Next.Y).Type))
			{
				Visited[NextIndex] = true;
				Queue.Enqueue(Next);
			}
		}
	}
}
}

bool FGatersTerrainRegion::IsReachable(const FIntPoint& Cell) const
{
	return Cell.X >= 0 && Cell.X < CellsPerAxis && Cell.Y >= 0 && Cell.Y < CellsPerAxis
		&& Reachable[ToIndex(CellsPerAxis, Cell)];
}

bool FGatersTerrainNavigation::IsWalkable(EGatersTerrainSemantic Type)
{
	return Type == EGatersTerrainSemantic::Flat || Type == EGatersTerrainSemantic::Slope;
}

FGatersTerrainRegion FGatersTerrainNavigation::Analyze(
	const FGatersTerrainSemanticField& Field,
	const FIntPoint& Start)
{
	FGatersTerrainRegion Result;
	Result.CellsPerAxis = Field.CellsPerAxis;
	Result.Reachable.Init(false, Field.Cells.Num());
	Flood(Field, Start, Result.Reachable);

	TBitArray<> Components(false, Field.Cells.Num());
	for (int32 Index = 0; Index < Field.Cells.Num(); ++Index)
	{
		if (!IsWalkable(Field.Cells[Index].Type))
		{
			continue;
		}
		++Result.WalkableCount;
		Result.ReachableCount += Result.Reachable[Index] ? 1 : 0;
		if (!Components[Index])
		{
			++Result.ComponentCount;
			Flood(Field, ToCell(Field.CellsPerAxis, Index), Components);
		}
	}
	return Result;
}

FGatersTerrainPath FGatersTerrainNavigation::FindPath(
	const FGatersTerrainSemanticField& Field,
	const FIntPoint& Start,
	const FIntPoint& Goal)
{
	FGatersTerrainPath Result;
	if (!IsValid(Field, Start) || !IsValid(Field, Goal)
		|| !IsWalkable(Field.At(Start.X, Start.Y).Type)
		|| !IsWalkable(Field.At(Goal.X, Goal.Y).Type))
	{
		return Result;
	}

	TArray<int32> Parent;
	Parent.Init(INDEX_NONE, Field.Cells.Num());
	const int32 StartIndex = ToIndex(Field.CellsPerAxis, Start);
	const int32 GoalIndex = ToIndex(Field.CellsPerAxis, Goal);
	Parent[StartIndex] = StartIndex;
	TQueue<FIntPoint> Queue;
	Queue.Enqueue(Start);
	FIntPoint Current;
	while (Queue.Dequeue(Current) && Parent[GoalIndex] == INDEX_NONE)
	{
		for (const FIntPoint& Direction : Directions)
		{
			const FIntPoint Next = Current + Direction;
			if (!IsValid(Field, Next))
			{
				continue;
			}
			const int32 NextIndex = ToIndex(Field.CellsPerAxis, Next);
			if (Parent[NextIndex] == INDEX_NONE && IsWalkable(Field.At(Next.X, Next.Y).Type))
			{
				Parent[NextIndex] = ToIndex(Field.CellsPerAxis, Current);
				Queue.Enqueue(Next);
			}
		}
	}

	if (Parent[GoalIndex] == INDEX_NONE)
	{
		return Result;
	}
	for (int32 Index = GoalIndex;; Index = Parent[Index])
	{
		Result.Cells.Add(ToCell(Field.CellsPerAxis, Index));
		if (Index == StartIndex)
		{
			break;
		}
	}
	Algo::Reverse(Result.Cells);
	Result.bFound = true;
	Result.Cost = Result.Cells.Num() - 1;
	return Result;
}
