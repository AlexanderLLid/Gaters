#include "GatersDrainageNetwork.h"

#include "GatersEnvironmentRecipe.h"
#include "GatersRegionalWaterRecipe.h"

#include "Containers/Queue.h"

namespace
{
const FIntPoint Directions[] = {
	FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1),
	FIntPoint(1, 1), FIntPoint(1, -1), FIntPoint(-1, 1), FIntPoint(-1, -1)};

int32 ToIndex(const int32 CellsPerAxis, const FIntPoint& Cell)
{
	return Cell.X * CellsPerAxis + Cell.Y;
}

bool IsInside(const int32 CellsPerAxis, const FIntPoint& Cell)
{
	return Cell.X >= 0 && Cell.X < CellsPerAxis
		&& Cell.Y >= 0 && Cell.Y < CellsPerAxis;
}

int32 BoundaryDistance(const int32 CellsPerAxis, const FIntPoint& Cell)
{
	return FMath::Min(
		FMath::Min(Cell.X, Cell.Y),
		FMath::Min(CellsPerAxis - 1 - Cell.X, CellsPerAxis - 1 - Cell.Y));
}

void AddIssue(
	FGatersDrainageBuildResult& Result,
	const TCHAR* RuleId,
	const TCHAR* Message)
{
	Result.Issues.Add({RuleId, Message});
}

void AddIssue(
	FGatersDrainageWaterFitResult& Result,
	const TCHAR* RuleId,
	const FString& Message)
{
	Result.Issues.Add({RuleId, Message});
}

void AddIssue(
	FGatersDrainageFeatureCompileResult& Result,
	const TCHAR* RuleId,
	const FString& Message)
{
	Result.Issues.Add({RuleId, Message});
}

bool CircleIntersectsCell(
	const FVector2D& CircleCenter,
	const float CircleRadius,
	const FVector2D& CellCenter,
	const float CellHalfExtent)
{
	const float DeltaX = FMath::Max(
		FMath::Abs(CircleCenter.X - CellCenter.X) - CellHalfExtent, 0.f);
	const float DeltaY = FMath::Max(
		FMath::Abs(CircleCenter.Y - CellCenter.Y) - CellHalfExtent, 0.f);
	return DeltaX * DeltaX + DeltaY * DeltaY <= CircleRadius * CircleRadius;
}

bool IsDrainageRecipeUsable(const FGatersDrainageRecipe& Drainage)
{
	if (Drainage.Version != FGatersDrainageRecipe::CurrentVersion
		|| Drainage.CellsPerAxis < 3
		|| Drainage.Cells.Num() != Drainage.CellsPerAxis * Drainage.CellsPerAxis
		|| !FMath::IsFinite(Drainage.WorldSize) || Drainage.WorldSize <= 0.f
		|| !FMath::IsFinite(Drainage.CellSize) || Drainage.CellSize <= 0.f)
	{
		return false;
	}
	for (int32 Index = 0; Index < Drainage.Cells.Num(); ++Index)
	{
		const FGatersDrainageCell& Cell = Drainage.Cells[Index];
		if (Cell.Index != Index || Cell.Center.ContainsNaN()
			|| !FMath::IsFinite(Cell.Height)
			|| !FMath::IsFinite(Cell.Accumulation)
			|| Cell.BasinIndex < 0
			|| (Cell.DownstreamIndex != INDEX_NONE
				&& !Drainage.Cells.IsValidIndex(Cell.DownstreamIndex)))
		{
			return false;
		}
	}
	for (const FGatersDrainageSegment& Segment : Drainage.Segments)
	{
		if (Segment.Id.IsEmpty()
			|| !Drainage.Cells.IsValidIndex(Segment.FromIndex)
			|| !Drainage.Cells.IsValidIndex(Segment.ToIndex))
		{
			return false;
		}
	}
	return true;
}
}

FGatersDrainageBuildResult FGatersDrainageNetwork::Build(
	const FGatersEnvironmentRecipe& Environment,
	const FGatersDrainageSettings& Settings)
{
	const FGatersClimateCompileResult Climate = FGatersClimateField::Compile(
		Environment.Terrain,
		Environment.EnvironmentBrief,
		Environment.Climate.Landform);
	const bool bRequiredProvenanceValid =
		Environment.Version == FGatersEnvironmentRecipe::CurrentVersion
		&& Environment.CompilerVersion ==
			FGatersEnvironmentRecipe::CurrentCompilerVersion
		&& Environment.WorldSize > 0.f
		&& Environment.Seed == Environment.Terrain.Seed
		&& FMath::IsNearlyEqual(
			Environment.WorldSize, Environment.Terrain.ChunkSize, 0.01f)
		&& Climate.IsValid()
		&& Climate.Recipe == Environment.Climate;
	if (!bRequiredProvenanceValid)
	{
		FGatersDrainageBuildResult Result;
		AddIssue(Result, TEXT("drainage.environment"),
			TEXT("Drainage requires a valid accepted environment recipe."));
		return Result;
	}
	return Build(
		Environment.Seed,
		Environment.WorldSize,
		Settings,
		[&Environment](const FVector2D& Point)
		{
			return Environment.QueryTerrain(Point).Height;
		},
		[&Environment](const FVector2D& Point)
		{
			return Environment.QueryClimate(Point).Precipitation;
		});
}

FGatersDrainageBuildResult FGatersDrainageNetwork::Build(
	const int32 Seed,
	const float WorldSize,
	const FGatersDrainageSettings& Settings,
	TFunctionRef<float(const FVector2D&)> HeightAt,
	TFunctionRef<float(const FVector2D&)> PrecipitationAt)
{
	FGatersDrainageBuildResult Result;
	Result.Recipe.Seed = Seed;
	Result.Recipe.WorldSize = WorldSize;
	Result.Recipe.CellsPerAxis = Settings.CellsPerAxis;
	Result.Recipe.Extent = Settings.Extent > 0.f
		? Settings.Extent : WorldSize * 0.45f;
	Result.Recipe.ChannelAccumulationThreshold =
		Settings.ChannelAccumulationThreshold;
	Result.Recipe.WaterfallDropThreshold = Settings.WaterfallDropThreshold;

	const bool bSettingsValid = FMath::IsFinite(WorldSize) && WorldSize > 0.f
		&& Settings.CellsPerAxis >= 3 && Settings.CellsPerAxis <= 129
		&& FMath::IsFinite(Settings.Extent) && Settings.Extent >= 0.f
		&& FMath::IsFinite(Settings.ChannelAccumulationThreshold)
		&& Settings.ChannelAccumulationThreshold >= 0.f
		&& FMath::IsFinite(Settings.WaterfallDropThreshold)
		&& Settings.WaterfallDropThreshold >= 0.f
		&& FMath::IsFinite(Result.Recipe.Extent) && Result.Recipe.Extent > 0.f;
	if (!bSettingsValid)
	{
		AddIssue(Result, TEXT("drainage.settings"),
			TEXT("Drainage grid size, extent, and thresholds must be finite and bounded."));
		return Result;
	}

	const int32 CellsPerAxis = Settings.CellsPerAxis;
	Result.Recipe.CellSize = Result.Recipe.Extent * 2.f
		/ static_cast<float>(CellsPerAxis - 1);
	Result.Recipe.Cells.Reserve(CellsPerAxis * CellsPerAxis);
	for (int32 X = 0; X < CellsPerAxis; ++X)
	{
		for (int32 Y = 0; Y < CellsPerAxis; ++Y)
		{
			FGatersDrainageCell& Cell = Result.Recipe.Cells.AddDefaulted_GetRef();
			Cell.Index = ToIndex(CellsPerAxis, FIntPoint(X, Y));
			Cell.Coordinate = FIntPoint(X, Y);
			Cell.Center = FVector2D(
				-Result.Recipe.Extent + X * Result.Recipe.CellSize,
				-Result.Recipe.Extent + Y * Result.Recipe.CellSize);
			Cell.Height = HeightAt(Cell.Center);
			const float Precipitation = PrecipitationAt(Cell.Center);
			if (!FMath::IsFinite(Cell.Height) || !FMath::IsFinite(Precipitation))
			{
				AddIssue(Result, TEXT("drainage.evidence"),
					TEXT("Drainage height and precipitation evidence must be finite."));
				Result.Recipe.Cells.Reset();
				return Result;
			}
			Cell.Precipitation = FMath::Clamp(Precipitation, 0.f, 1.f);
			Cell.Accumulation = Cell.Precipitation;
			Cell.bBoundary = X == 0 || Y == 0
				|| X == CellsPerAxis - 1 || Y == CellsPerAxis - 1;
		}
	}

	for (FGatersDrainageCell& Cell : Result.Recipe.Cells)
	{
		if (Cell.bBoundary)
		{
			continue;
		}
		float BestHeight = Cell.Height;
		int32 BestIndex = INDEX_NONE;
		int32 EqualHeightIndex = INDEX_NONE;
		const int32 CurrentBoundaryDistance =
			BoundaryDistance(CellsPerAxis, Cell.Coordinate);
		for (const FIntPoint& Direction : Directions)
		{
			const FIntPoint NeighborCoordinate = Cell.Coordinate + Direction;
			if (!IsInside(CellsPerAxis, NeighborCoordinate))
			{
				continue;
			}
			const int32 NeighborIndex = ToIndex(CellsPerAxis, NeighborCoordinate);
			const float NeighborHeight = Result.Recipe.Cells[NeighborIndex].Height;
			if (NeighborHeight < BestHeight - KINDA_SMALL_NUMBER)
			{
				BestHeight = NeighborHeight;
				BestIndex = NeighborIndex;
			}
			else if (BestIndex == INDEX_NONE
				&& FMath::IsNearlyEqual(NeighborHeight, Cell.Height)
				&& BoundaryDistance(CellsPerAxis, NeighborCoordinate)
					< CurrentBoundaryDistance
				&& (EqualHeightIndex == INDEX_NONE
					|| NeighborIndex < EqualHeightIndex))
			{
				EqualHeightIndex = NeighborIndex;
			}
		}
		Cell.DownstreamIndex = BestIndex != INDEX_NONE
			? BestIndex : EqualHeightIndex;
		Cell.bSink = Cell.DownstreamIndex == INDEX_NONE;
	}

	TArray<int32> UpstreamCount;
	UpstreamCount.Init(0, Result.Recipe.Cells.Num());
	for (const FGatersDrainageCell& Cell : Result.Recipe.Cells)
	{
		if (Cell.DownstreamIndex != INDEX_NONE)
		{
			++UpstreamCount[Cell.DownstreamIndex];
		}
	}
	TQueue<int32> FlowQueue;
	for (int32 Index = 0; Index < UpstreamCount.Num(); ++Index)
	{
		if (UpstreamCount[Index] == 0)
		{
			FlowQueue.Enqueue(Index);
		}
	}
	int32 ProcessedCount = 0;
	int32 Index = INDEX_NONE;
	while (FlowQueue.Dequeue(Index))
	{
		++ProcessedCount;
		const int32 DownstreamIndex = Result.Recipe.Cells[Index].DownstreamIndex;
		if (DownstreamIndex != INDEX_NONE)
		{
			Result.Recipe.Cells[DownstreamIndex].Accumulation +=
				Result.Recipe.Cells[Index].Accumulation;
			if (--UpstreamCount[DownstreamIndex] == 0)
			{
				FlowQueue.Enqueue(DownstreamIndex);
			}
		}
	}
	if (ProcessedCount != Result.Recipe.Cells.Num())
	{
		AddIssue(Result, TEXT("drainage.cycle"),
			TEXT("Drainage flow graph contains a cycle."));
		return Result;
	}

	TMap<int32, int32> BasinByTerminal;
	for (FGatersDrainageCell& Cell : Result.Recipe.Cells)
	{
		int32 Terminal = Cell.Index;
		int32 Steps = 0;
		while (Result.Recipe.Cells[Terminal].DownstreamIndex != INDEX_NONE
			&& Steps++ <= Result.Recipe.Cells.Num())
		{
			Terminal = Result.Recipe.Cells[Terminal].DownstreamIndex;
		}
		if (Steps > Result.Recipe.Cells.Num())
		{
			AddIssue(Result, TEXT("drainage.cycle"),
				TEXT("Drainage flow graph contains a cycle."));
			return Result;
		}
		int32* BasinIndex = BasinByTerminal.Find(Terminal);
		if (!BasinIndex)
		{
			const int32 NewBasinIndex = BasinByTerminal.Num();
			BasinByTerminal.Add(Terminal, NewBasinIndex);
			BasinIndex = BasinByTerminal.Find(Terminal);
		}
		Cell.BasinIndex = *BasinIndex;
	}
	Result.Recipe.BasinCount = BasinByTerminal.Num();

	for (FGatersDrainageCell& Cell : Result.Recipe.Cells)
	{
		Cell.bChannel = Cell.DownstreamIndex != INDEX_NONE
			&& Cell.Accumulation >= Settings.ChannelAccumulationThreshold;
		if (!Cell.bChannel)
		{
			continue;
		}
		FGatersDrainageSegment& Segment =
			Result.Recipe.Segments.AddDefaulted_GetRef();
		Segment.Id = FString::Printf(
			TEXT("drainage:%d>%d"), Cell.Index, Cell.DownstreamIndex);
		Segment.FromIndex = Cell.Index;
		Segment.ToIndex = Cell.DownstreamIndex;
		Segment.Discharge = Cell.Accumulation;
		Segment.Drop = FMath::Max(
			0.f, Cell.Height - Result.Recipe.Cells[Cell.DownstreamIndex].Height);
		Segment.bWaterfallCandidate =
			Segment.Drop >= Settings.WaterfallDropThreshold;
	}
	return Result;
}

FGatersDrainageWaterFitResult FGatersDrainageNetwork::FitRegionalWater(
	const FGatersDrainageRecipe& Drainage,
	const FGatersRegionalWaterRecipe& RegionalWater,
	const float DatumTolerance)
{
	FGatersDrainageWaterFitResult Result;
	Result.DrainageVersion = Drainage.Version;
	Result.RegionalWaterVersion = RegionalWater.Version;
	Result.DatumTolerance = DatumTolerance;
	if (!IsDrainageRecipeUsable(Drainage))
	{
		AddIssue(Result, TEXT("drainage.water.drainage"),
			TEXT("Regional-water fit requires a valid drainage recipe."));
		return Result;
	}
	if (RegionalWater.Version != 1)
	{
		AddIssue(Result, TEXT("drainage.water.recipe"),
			TEXT("Regional-water fit requires water recipe version 1."));
		return Result;
	}
	if (!FMath::IsFinite(DatumTolerance) || DatumTolerance < 0.f)
	{
		AddIssue(Result, TEXT("drainage.water.settings"),
			TEXT("Regional-water datum tolerance must be finite and non-negative."));
		return Result;
	}

	TSet<FString> SurfaceIds;
	for (const FGatersRegionalWaterSurface& Surface : RegionalWater.Surfaces)
	{
		if (Surface.Id.IsEmpty() || SurfaceIds.Contains(Surface.Id))
		{
			AddIssue(Result, TEXT("drainage.water.identity"), FString::Printf(
				TEXT("Regional-water identity is empty or duplicated: %s."),
				*Surface.Id));
		}
		SurfaceIds.Add(Surface.Id);
		if (Surface.Center.ContainsNaN()
			|| !FMath::IsFinite(Surface.HalfExtent) || Surface.HalfExtent <= 0.f
			|| !FMath::IsFinite(Surface.Height))
		{
			AddIssue(Result, TEXT("drainage.water.surface"), FString::Printf(
				TEXT("Regional-water surface geometry is invalid: %s."),
				*Surface.Id));
		}
	}
	if (!Result.Issues.IsEmpty())
	{
		return Result;
	}

	const float CellHalfExtent = Drainage.CellSize * 0.5f;
	for (const FGatersRegionalWaterSurface& Surface : RegionalWater.Surfaces)
	{
		FGatersDrainageWaterSurfaceFit& Fit =
			Result.Surfaces.AddDefaulted_GetRef();
		Fit.SurfaceId = Surface.Id;
		TSet<int32> CoveredCells;
		for (const FGatersDrainageCell& Cell : Drainage.Cells)
		{
			if (!CircleIntersectsCell(
				Surface.Center, Surface.HalfExtent, Cell.Center, CellHalfExtent))
			{
				continue;
			}
			Fit.CellIndices.Add(Cell.Index);
			CoveredCells.Add(Cell.Index);
			Fit.BasinIndices.AddUnique(Cell.BasinIndex);
			Fit.MaximumAccumulation = FMath::Max(
				Fit.MaximumAccumulation, Cell.Accumulation);
			Fit.bHasSubmergedTerrain |=
				Cell.Height <= Surface.Height + DatumTolerance;

			int32 TerminalIndex = Cell.Index;
			int32 Steps = 0;
			while (Drainage.Cells[TerminalIndex].DownstreamIndex != INDEX_NONE
				&& Steps++ <= Drainage.Cells.Num())
			{
				TerminalIndex = Drainage.Cells[TerminalIndex].DownstreamIndex;
			}
			if (Steps > Drainage.Cells.Num())
			{
				AddIssue(Result, TEXT("drainage.water.drainage"),
					TEXT("Regional-water fit found a drainage cycle."));
				return Result;
			}
			Fit.TerminalCellIndices.AddUnique(TerminalIndex);
		}

		if (Fit.CellIndices.IsEmpty())
		{
			AddIssue(Result, TEXT("drainage.water.coverage"), FString::Printf(
				TEXT("Regional-water surface is outside drainage coverage: %s."),
				*Surface.Id));
			continue;
		}
		if (!Fit.bHasSubmergedTerrain)
		{
			AddIssue(Result, TEXT("drainage.water.terrain"), FString::Printf(
				TEXT("Regional-water surface has no drainage terrain below its datum: %s."),
				*Surface.Id));
		}

		for (const FGatersDrainageCell& Cell : Drainage.Cells)
		{
			if (Cell.DownstreamIndex == INDEX_NONE)
			{
				continue;
			}
			const bool bFromCovered = CoveredCells.Contains(Cell.Index);
			const bool bToCovered = CoveredCells.Contains(Cell.DownstreamIndex);
			Fit.IncomingFlowCount += !bFromCovered && bToCovered ? 1 : 0;
			Fit.OutgoingFlowCount += bFromCovered && !bToCovered ? 1 : 0;
		}
		for (const FGatersDrainageSegment& Segment : Drainage.Segments)
		{
			if (CoveredCells.Contains(Segment.FromIndex)
				|| CoveredCells.Contains(Segment.ToIndex))
			{
				Fit.ChannelSegmentIds.Add(Segment.Id);
			}
		}
		Fit.BasinIndices.Sort();
		Fit.TerminalCellIndices.Sort();
	}
	return Result;
}

FGatersDrainageFeatureCompileResult FGatersDrainageNetwork::CompileFeatureCandidates(
	const FGatersDrainageRecipe& Drainage,
	const FGatersRegionalWaterRecipe& RegionalWater,
	const FGatersDrainageWaterFitResult& WaterFit,
	const FGatersDrainageFeatureSettings& Settings)
{
	FGatersDrainageFeatureCompileResult Result;
	Result.Recipe.Seed = Drainage.Seed;
	Result.Recipe.DrainageVersion = Drainage.Version;
	Result.Recipe.RegionalWaterVersion = RegionalWater.Version;
	Result.Recipe.WaterFitVersion = WaterFit.Version;
	Result.Recipe.Settings = Settings;
	if (!IsDrainageRecipeUsable(Drainage))
	{
		AddIssue(Result, TEXT("drainage.feature.drainage"),
			TEXT("Feature candidates require a valid drainage recipe."));
		return Result;
	}
	if (RegionalWater.Version != 1)
	{
		AddIssue(Result, TEXT("drainage.feature.water"),
			TEXT("Feature candidates require regional-water recipe version 1."));
		return Result;
	}
	if (WaterFit.Version != FGatersDrainageWaterFitResult::CurrentVersion
		|| WaterFit.DrainageVersion != Drainage.Version
		|| WaterFit.RegionalWaterVersion != RegionalWater.Version
		|| !FMath::IsFinite(WaterFit.DatumTolerance)
		|| WaterFit.DatumTolerance < 0.f
		|| !WaterFit.IsValid()
		|| WaterFit.Surfaces.Num() != RegionalWater.Surfaces.Num())
	{
		AddIssue(Result, TEXT("drainage.feature.fit"),
			TEXT("Feature candidates require a matching valid regional-water fit."));
		return Result;
	}
	const bool bSettingsValid =
		FMath::IsFinite(Settings.WetlandMinimumPrecipitation)
		&& Settings.WetlandMinimumPrecipitation >= 0.f
		&& Settings.WetlandMinimumPrecipitation <= 1.f
		&& FMath::IsFinite(Settings.WetlandMinimumAccumulation)
		&& Settings.WetlandMinimumAccumulation >= 0.f
		&& FMath::IsFinite(Settings.WetlandMaximumDrop)
		&& Settings.WetlandMaximumDrop >= 0.f;
	if (!bSettingsValid)
	{
		AddIssue(Result, TEXT("drainage.feature.settings"),
			TEXT("Feature candidate thresholds must be finite and bounded."));
		return Result;
	}

	TSet<FString> SurfaceIds;
	for (const FGatersRegionalWaterSurface& Surface : RegionalWater.Surfaces)
	{
		const FGatersDrainageWaterSurfaceFit* Fit =
			WaterFit.Surfaces.FindByPredicate(
				[&Surface](const FGatersDrainageWaterSurfaceFit& Candidate)
				{
					return Candidate.SurfaceId == Surface.Id;
				});
		if (Surface.Id.IsEmpty() || SurfaceIds.Contains(Surface.Id) || !Fit)
		{
			AddIssue(Result, TEXT("drainage.feature.fit"), FString::Printf(
				TEXT("Feature candidate water fit is missing or duplicated: %s."),
				*Surface.Id));
			return Result;
		}
		SurfaceIds.Add(Surface.Id);
	}

	TMap<int32, TArray<const FGatersDrainageSegment*>> RiverSegmentsByBasin;
	for (const FGatersDrainageSegment& Segment : Drainage.Segments)
	{
		const int32 BasinIndex = Drainage.Cells[Segment.FromIndex].BasinIndex;
		RiverSegmentsByBasin.FindOrAdd(BasinIndex).Add(&Segment);
	}
	TArray<int32> RiverBasins;
	RiverSegmentsByBasin.GetKeys(RiverBasins);
	RiverBasins.Sort();
	for (const int32 BasinIndex : RiverBasins)
	{
		FGatersDrainageFeatureCandidate& Candidate =
			Result.Recipe.Candidates.AddDefaulted_GetRef();
		Candidate.Id = FString::Printf(
			TEXT("feature:%d:river:%d"), Drainage.Seed, BasinIndex);
		Candidate.Kind = EGatersDrainageFeatureKind::RiverSystem;
		Candidate.BasinIndices.Add(BasinIndex);
		for (const FGatersDrainageSegment* Segment :
			RiverSegmentsByBasin.FindChecked(BasinIndex))
		{
			Candidate.SegmentIds.Add(Segment->Id);
			Candidate.CellIndices.AddUnique(Segment->FromIndex);
			Candidate.CellIndices.AddUnique(Segment->ToIndex);
		}
		Candidate.CellIndices.Sort();
	}

	for (const FGatersRegionalWaterSurface& Surface : RegionalWater.Surfaces)
	{
		const FGatersDrainageWaterSurfaceFit& Fit =
			*WaterFit.Surfaces.FindByPredicate(
				[&Surface](const FGatersDrainageWaterSurfaceFit& Candidate)
				{
					return Candidate.SurfaceId == Surface.Id;
				});
		if (Surface.Hydrology == EGatersHydrology::Lakes)
		{
			FGatersDrainageFeatureCandidate& Candidate =
				Result.Recipe.Candidates.AddDefaulted_GetRef();
			Candidate.Id = FString::Printf(
				TEXT("feature:%d:lake:%s"), Drainage.Seed, *Surface.Id);
			Candidate.Kind = EGatersDrainageFeatureKind::Lake;
			Candidate.SurfaceId = Surface.Id;
			Candidate.BasinIndices = Fit.BasinIndices;
			Candidate.CellIndices = Fit.CellIndices;
			Candidate.SegmentIds = Fit.ChannelSegmentIds;
		}
	}

	TMap<int32, TArray<int32>> WetlandCellsByBasin;
	for (const FGatersDrainageCell& Cell : Drainage.Cells)
	{
		const float Drop = Cell.DownstreamIndex == INDEX_NONE
			? 0.f
			: FMath::Max(0.f,
				Cell.Height - Drainage.Cells[Cell.DownstreamIndex].Height);
		if (Cell.Precipitation >= Settings.WetlandMinimumPrecipitation
			&& Cell.Accumulation >= Settings.WetlandMinimumAccumulation
			&& Drop <= Settings.WetlandMaximumDrop)
		{
			WetlandCellsByBasin.FindOrAdd(Cell.BasinIndex).Add(Cell.Index);
		}
	}
	TArray<int32> WetlandBasins;
	WetlandCellsByBasin.GetKeys(WetlandBasins);
	WetlandBasins.Sort();
	for (const int32 BasinIndex : WetlandBasins)
	{
		FGatersDrainageFeatureCandidate& Candidate =
			Result.Recipe.Candidates.AddDefaulted_GetRef();
		Candidate.Id = FString::Printf(
			TEXT("feature:%d:wetland:%d"), Drainage.Seed, BasinIndex);
		Candidate.Kind = EGatersDrainageFeatureKind::Wetland;
		Candidate.BasinIndices.Add(BasinIndex);
		Candidate.CellIndices = WetlandCellsByBasin.FindChecked(BasinIndex);
	}

	for (const FGatersRegionalWaterSurface& Surface : RegionalWater.Surfaces)
	{
		if (Surface.Hydrology != EGatersHydrology::Ocean)
		{
			continue;
		}
		const FGatersDrainageWaterSurfaceFit& Fit =
			*WaterFit.Surfaces.FindByPredicate(
				[&Surface](const FGatersDrainageWaterSurfaceFit& Candidate)
				{
					return Candidate.SurfaceId == Surface.Id;
				});
		if (Fit.IncomingFlowCount <= 0 || Fit.ChannelSegmentIds.IsEmpty())
		{
			continue;
		}
		FGatersDrainageFeatureCandidate& Candidate =
			Result.Recipe.Candidates.AddDefaulted_GetRef();
		Candidate.Id = FString::Printf(
			TEXT("feature:%d:delta:%s"), Drainage.Seed, *Surface.Id);
		Candidate.Kind = EGatersDrainageFeatureKind::Delta;
		Candidate.SurfaceId = Surface.Id;
		Candidate.BasinIndices = Fit.BasinIndices;
		Candidate.CellIndices = Fit.CellIndices;
		Candidate.SegmentIds = Fit.ChannelSegmentIds;
	}

	for (const FGatersDrainageSegment& Segment : Drainage.Segments)
	{
		if (!Segment.bWaterfallCandidate)
		{
			continue;
		}
		FGatersDrainageFeatureCandidate& Candidate =
			Result.Recipe.Candidates.AddDefaulted_GetRef();
		Candidate.Id = FString::Printf(
			TEXT("feature:%d:waterfall:%s"), Drainage.Seed, *Segment.Id);
		Candidate.Kind = EGatersDrainageFeatureKind::Waterfall;
		Candidate.BasinIndices.Add(
			Drainage.Cells[Segment.FromIndex].BasinIndex);
		Candidate.CellIndices = {Segment.FromIndex, Segment.ToIndex};
		Candidate.SegmentIds.Add(Segment.Id);
	}
	return Result;
}
