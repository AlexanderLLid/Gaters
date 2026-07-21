#include "GatersTraversabilityEvaluator.h"

FGatersTraversabilityEvaluation FGatersTraversabilityEvaluator::Evaluate(
	const FGatersTerrainSemanticField& Field,
	const FIntPoint& Start,
	const FIntPoint& Goal,
	int32 EscapeDistanceCells)
{
	FGatersTraversabilityEvaluation Result;
	Result.Region = FGatersTerrainNavigation::Analyze(Field, Start);
	Result.GoalPath = FGatersTerrainNavigation::FindPath(Field, Start, Goal);
	Result.bGoalReachable = Result.GoalPath.bFound;
	if (!Field.Cells.IsEmpty())
	{
		Result.WalkableFraction = static_cast<float>(Result.Region.WalkableCount)
			/ static_cast<float>(Field.Cells.Num());
	}
	if (Result.Region.WalkableCount > 0)
	{
		Result.ReachableFraction = static_cast<float>(Result.Region.ReachableCount)
			/ static_cast<float>(Result.Region.WalkableCount);
	}
	for (int32 X = 0; X < Field.CellsPerAxis && !Result.bEscapesStart; ++X)
	{
		for (int32 Y = 0; Y < Field.CellsPerAxis; ++Y)
		{
			const FIntPoint Cell(X, Y);
			if (Result.Region.IsReachable(Cell)
				&& FMath::Abs(X - Start.X) + FMath::Abs(Y - Start.Y) >= EscapeDistanceCells)
			{
				Result.bEscapesStart = true;
				break;
			}
		}
	}
	return Result;
}
