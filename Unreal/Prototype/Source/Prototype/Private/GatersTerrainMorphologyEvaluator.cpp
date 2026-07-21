#include "GatersTerrainMorphologyEvaluator.h"

FGatersTerrainMorphologyEvaluation FGatersTerrainMorphologyEvaluator::Evaluate(
	TFunctionRef<float(const FVector2D&)> HeightAt,
	float HalfExtent,
	const TArray<float>& ContourHeights)
{
	constexpr int32 Side = 65;
	constexpr int32 MinimumComponentSamples = 12;
	FGatersTerrainMorphologyEvaluation Result;
	if (!FMath::IsFinite(HalfExtent) || HalfExtent <= 0.f)
	{
		return Result;
	}

	for (const float Contour : ContourHeights)
	{
		if (!FMath::IsFinite(Contour))
		{
			continue;
		}

		TArray<bool> Above;
		Above.SetNumUninitialized(Side * Side);
		for (int32 Y = 0; Y < Side; ++Y)
		{
			for (int32 X = 0; X < Side; ++X)
			{
				const FVector2D Point(
					FMath::Lerp(-HalfExtent, HalfExtent, X / static_cast<float>(Side - 1)),
					FMath::Lerp(-HalfExtent, HalfExtent, Y / static_cast<float>(Side - 1)));
				Above[Y * Side + X] = HeightAt(Point) > Contour;
			}
		}

		TArray<bool> Visited;
		Visited.Init(false, Above.Num());
		for (int32 Start = 0; Start < Above.Num(); ++Start)
		{
			if (!Above[Start] || Visited[Start])
			{
				continue;
			}

			TArray<int32> Component;
			Component.Add(Start);
			Visited[Start] = true;
			bool bTouchesBoundary = false;
			FVector2D Center = FVector2D::ZeroVector;
			for (int32 Cursor = 0; Cursor < Component.Num(); ++Cursor)
			{
				const int32 Index = Component[Cursor];
				const int32 X = Index % Side;
				const int32 Y = Index / Side;
				Center += FVector2D(X, Y);
				bTouchesBoundary |= X == 0 || Y == 0 || X + 1 == Side || Y + 1 == Side;
				const int32 Neighbors[] = {
					X > 0 ? Index - 1 : -1,
					X + 1 < Side ? Index + 1 : -1,
					Y > 0 ? Index - Side : -1,
					Y + 1 < Side ? Index + Side : -1 };
				for (const int32 Neighbor : Neighbors)
				{
					if (Neighbor >= 0 && Above[Neighbor] && !Visited[Neighbor])
					{
						Visited[Neighbor] = true;
						Component.Add(Neighbor);
					}
				}
			}

			if (bTouchesBoundary || Component.Num() < MinimumComponentSamples)
			{
				continue;
			}

			Center /= static_cast<float>(Component.Num());
			TArray<float> BoundaryRadii;
			for (const int32 Index : Component)
			{
				const int32 X = Index % Side;
				const int32 Y = Index / Side;
				const bool bBoundary = X == 0 || Y == 0 || X + 1 == Side || Y + 1 == Side
					|| !Above[Index - 1] || !Above[Index + 1]
					|| !Above[Index - Side] || !Above[Index + Side];
				if (bBoundary)
				{
					BoundaryRadii.Add(static_cast<float>((FVector2D(X, Y) - Center).Size()));
				}
			}

			float MeanRadius = 0.f;
			for (const float Radius : BoundaryRadii)
			{
				MeanRadius += Radius;
			}
			MeanRadius /= FMath::Max(BoundaryRadii.Num(), 1);
			float Variance = 0.f;
			for (const float Radius : BoundaryRadii)
			{
				Variance += FMath::Square(Radius - MeanRadius);
			}
			Variance /= FMath::Max(BoundaryRadii.Num(), 1);
			const float RadialVariation = MeanRadius > UE_SMALL_NUMBER
				? FMath::Sqrt(Variance) / MeanRadius
				: 1.f;
			const float Circularity = FMath::Clamp(1.f - RadialVariation, 0.f, 1.f);
			if (Circularity > Result.MaximumClosedContourCircularity)
			{
				Result.MaximumClosedContourCircularity = Circularity;
				Result.MostCircularContourSamples = Component.Num();
			}
			if (Component.Num() > Result.LargestClosedContourSamples)
			{
				Result.LargestClosedContourSamples = Component.Num();
				Result.LargestClosedContourCircularity = Circularity;
			}
			++Result.ClosedContourCount;
		}
	}
	return Result;
}
