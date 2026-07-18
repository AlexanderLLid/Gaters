#include "GatersWorldCellStreaming.h"

FIntPoint FGatersWorldCellStreaming::CellAt(const FVector2D& Position, float CellSize)
{
	check(CellSize > 0.f);
	return FIntPoint(
		FMath::FloorToInt((Position.X + CellSize * 0.5f) / CellSize),
		FMath::FloorToInt((Position.Y + CellSize * 0.5f) / CellSize));
}

FVector2D FGatersWorldCellStreaming::CellCenter(const FIntPoint& Cell, float CellSize)
{
	return FVector2D(Cell.X * CellSize, Cell.Y * CellSize);
}

TArray<FIntPoint> FGatersWorldCellStreaming::DesiredCells(
	const TArray<FVector2D>& Sources,
	float CellSize,
	int32 LoadRadius,
	float WorldSize,
	const FIntPoint& PrefetchDirection)
{
	check(CellSize > 0.f);
	TSet<FIntPoint> Unique;
	const float MaxCenter = FMath::Max(0.f, WorldSize * 0.5f - CellSize * 0.5f);
	auto AddIfBounded = [&Unique, CellSize, MaxCenter](const FIntPoint& Cell)
	{
		const FVector2D Position = CellCenter(Cell, CellSize);
		if (FMath::Abs(Position.X) <= MaxCenter && FMath::Abs(Position.Y) <= MaxCenter)
		{
			Unique.Add(Cell);
		}
	};
	for (const FVector2D& Source : Sources)
	{
		const FIntPoint Center = CellAt(Source, CellSize);
		for (int32 X = Center.X - LoadRadius; X <= Center.X + LoadRadius; ++X)
		{
			for (int32 Y = Center.Y - LoadRadius; Y <= Center.Y + LoadRadius; ++Y)
			{
				AddIfBounded(FIntPoint(X, Y));
			}
		}

		FIntPoint Direction = FIntPoint::ZeroValue;
		if (FMath::Abs(PrefetchDirection.X) >= FMath::Abs(PrefetchDirection.Y) && PrefetchDirection.X != 0)
		{
			Direction.X = FMath::Sign(PrefetchDirection.X);
		}
		else if (PrefetchDirection.Y != 0)
		{
			Direction.Y = FMath::Sign(PrefetchDirection.Y);
		}
		if (Direction != FIntPoint::ZeroValue)
		{
			const FIntPoint Side(-Direction.Y, Direction.X);
			const FIntPoint Forward = Center + Direction * (LoadRadius + 1);
			for (int32 Offset = -LoadRadius; Offset <= LoadRadius; ++Offset)
			{
				AddIfBounded(Forward + Side * Offset);
			}
		}
	}

	TArray<FIntPoint> Result = Unique.Array();
	Result.Sort([](const FIntPoint& A, const FIntPoint& B)
	{
		return A.X == B.X ? A.Y < B.Y : A.X < B.X;
	});
	return Result;
}
