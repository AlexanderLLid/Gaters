#include "GatersBiomeResourceRecipe.h"

#include "GatersEnvironmentRecipe.h"
#include "Misc/Crc.h"

namespace
{
constexpr TCHAR EnvironmentInvalidRule[] = TEXT("biome-resource.environment.invalid");
constexpr TCHAR BoundsInvalidRule[] = TEXT("biome-resource.bounds.invalid");
constexpr TCHAR CellSizeInvalidRule[] = TEXT("biome-resource.cell-size.invalid");
constexpr TCHAR CellBudgetRule[] = TEXT("biome-resource.cell-budget.exceeded");
constexpr TCHAR CellIdentityRule[] = TEXT("biome-resource.cell.identity");
constexpr TCHAR CellDuplicateRule[] = TEXT("biome-resource.cell.duplicate");
constexpr TCHAR OpportunityInvalidRule[] = TEXT("biome-resource.opportunity.invalid");
constexpr TCHAR SourceProvenanceRule[] = TEXT("biome-resource.source.provenance");
constexpr TCHAR AggregateCoverageRule[] = TEXT("biome-resource.aggregate.coverage");

void AddIssue(
	TArray<FGatersBiomeResourceIssue>& Issues,
	const TCHAR* RuleId,
	const FString& SubjectId,
	const FString& Message)
{
	Issues.Add({RuleId, SubjectId, Message});
}

FString CellId(const int32 Seed, const FIntPoint& Coordinate)
{
	return FString::Printf(
		TEXT("biome-resource:%d:%d:%d"), Seed, Coordinate.X, Coordinate.Y);
}

bool IsBounded(const float Value)
{
	return FMath::IsFinite(Value) && Value >= 0.f && Value <= 1.f;
}

void AddCoverage(
	FGatersContentCellCoverage& Total,
	const FGatersContentCellCoverage& Cell)
{
	Total.CandidateCount += Cell.CandidateCount;
	Total.PlacedCount += Cell.PlacedCount;
	Total.ReservedRejectedCount += Cell.ReservedRejectedCount;
	Total.IntentRejectedCount += Cell.IntentRejectedCount;
	Total.WaterRejectedCount += Cell.WaterRejectedCount;
	Total.SteepRejectedCount += Cell.SteepRejectedCount;
	Total.OpportunityRejectedCount += Cell.OpportunityRejectedCount;
	Total.BudgetRejectedCount += Cell.BudgetRejectedCount;
}

void AppendCoverage(FString& Text, const FGatersContentCellCoverage& Coverage)
{
	Text += FString::Printf(
		TEXT("%d,%d,%d,%d,%d,%d,%d,%d"),
		Coverage.CandidateCount,
		Coverage.PlacedCount,
		Coverage.ReservedRejectedCount,
		Coverage.IntentRejectedCount,
		Coverage.WaterRejectedCount,
		Coverage.SteepRejectedCount,
		Coverage.OpportunityRejectedCount,
		Coverage.BudgetRejectedCount);
}
}

bool FGatersBiomeResourceRecipe::Validate(
	TArray<FGatersBiomeResourceIssue>& OutIssues) const
{
	OutIssues.Reset();
	if (Version != CurrentVersion
		|| CompilerVersion != CurrentCompilerVersion
		|| EnvironmentVersion <= 0
		|| BiomeOpportunityVersion <= 0)
	{
		AddIssue(OutIssues, SourceProvenanceRule, TEXT("recipe"),
			TEXT("recipe versions or environment provenance are invalid"));
	}
	if (CellBounds.Max.X <= CellBounds.Min.X || CellBounds.Max.Y <= CellBounds.Min.Y)
	{
		AddIssue(OutIssues, BoundsInvalidRule, TEXT("recipe"),
			TEXT("cell bounds must be non-empty and half-open"));
	}
	if (!FMath::IsFinite(CellSize) || CellSize <= 0.f)
	{
		AddIssue(OutIssues, CellSizeInvalidRule, TEXT("recipe"),
			TEXT("cell size must be finite and positive"));
	}

	const int64 Width = static_cast<int64>(CellBounds.Max.X) - CellBounds.Min.X;
	const int64 Height = static_cast<int64>(CellBounds.Max.Y) - CellBounds.Min.Y;
	const int64 ExpectedCount = Width > 0 && Height > 0 ? Width * Height : 0;
	if (ExpectedCount != Cells.Num())
	{
		AddIssue(OutIssues, CellIdentityRule, TEXT("recipe"),
			TEXT("cell count does not cover the declared bounds exactly"));
	}

	TSet<FIntPoint> SeenCoordinates;
	FGatersContentCellCoverage ExpectedCoverage;
	int32 ExpectedBudget = 0;
	for (int32 Index = 0; Index < Cells.Num(); ++Index)
	{
		const FGatersBiomeResourceCell& Cell = Cells[Index];
		if (SeenCoordinates.Contains(Cell.Coordinate))
		{
			AddIssue(OutIssues, CellDuplicateRule, Cell.Id,
				TEXT("cell coordinate is duplicated"));
		}
		SeenCoordinates.Add(Cell.Coordinate);

		FIntPoint ExpectedCoordinate = FIntPoint::ZeroValue;
		if (Height > 0)
		{
			ExpectedCoordinate.X = CellBounds.Min.X + Index / static_cast<int32>(Height);
			ExpectedCoordinate.Y = CellBounds.Min.Y + Index % static_cast<int32>(Height);
		}
		if (Cell.Coordinate != ExpectedCoordinate
			|| Cell.Id != CellId(Seed, Cell.Coordinate))
		{
			AddIssue(OutIssues, CellIdentityRule, Cell.Id,
				TEXT("cell identity or stable ordering does not match its coordinate"));
		}
		if (Cell.BiomeKey.IsEmpty()
			|| !IsBounded(Cell.DeclaredLandmarkOpportunity)
			|| !IsBounded(Cell.LandmarkOpportunity)
			|| !IsBounded(Cell.DeclaredTravelFriction)
			|| !IsBounded(Cell.TravelFriction))
		{
			AddIssue(OutIssues, OpportunityInvalidRule, Cell.Id,
				TEXT("biome or opportunity evidence is invalid"));
		}
		if (Cell.Content.WorldSeed != Seed
			|| Cell.Content.Cell != Cell.Coordinate
			|| !FMath::IsNearlyEqual(Cell.Content.CellSize, CellSize)
			|| Cell.Content.EnvironmentVersion != EnvironmentVersion
			|| Cell.Content.BiomeOpportunityVersion != BiomeOpportunityVersion
			|| Cell.Content.BiomeKey != Cell.BiomeKey)
		{
			AddIssue(OutIssues, SourceProvenanceRule, Cell.Id,
				TEXT("content cell provenance does not match the bounded recipe"));
		}
		ExpectedBudget += Cell.Content.MaxPlacements;
		AddCoverage(ExpectedCoverage, Cell.Content.Coverage);
	}
	if (PlacementBudget != ExpectedBudget || Coverage != ExpectedCoverage)
	{
		AddIssue(OutIssues, AggregateCoverageRule, TEXT("recipe"),
			TEXT("aggregate budget or coverage does not match source cells"));
	}
	return OutIssues.IsEmpty();
}

FString FGatersBiomeResourceRecipe::CanonicalText() const
{
	FString Text = FString::Printf(
		TEXT("biome-resource|v=%d|compiler=%d|environment=%d|opportunities=%d|")
		TEXT("seed=%d|bounds=%d,%d,%d,%d|cell-size=%.6f|budget=%d|coverage="),
		Version,
		CompilerVersion,
		EnvironmentVersion,
		BiomeOpportunityVersion,
		Seed,
		CellBounds.Min.X,
		CellBounds.Min.Y,
		CellBounds.Max.X,
		CellBounds.Max.Y,
		CellSize,
		PlacementBudget);
	AppendCoverage(Text, Coverage);
	for (const FGatersBiomeResourceCell& Cell : Cells)
	{
		Text += FString::Printf(
			TEXT("|cell=%s,%d,%d,%s,%.6f,%.6f,%.6f,%.6f|")
			TEXT("content=%d,%d,%d,%d,%s,%.6f,%.6f,%d,"),
			*Cell.Id,
			Cell.Coordinate.X,
			Cell.Coordinate.Y,
			*Cell.BiomeKey,
			Cell.DeclaredLandmarkOpportunity,
			Cell.LandmarkOpportunity,
			Cell.DeclaredTravelFriction,
			Cell.TravelFriction,
			Cell.Content.Version,
			Cell.Content.WorldSeed,
			Cell.Content.EnvironmentVersion,
			Cell.Content.BiomeOpportunityVersion,
			*Cell.Content.IntentRegionId,
			Cell.Content.VegetationOpportunity,
			Cell.Content.StoneOpportunity,
			Cell.Content.MaxPlacements);
		AppendCoverage(Text, Cell.Content.Coverage);
		for (const FGatersContentCellPlacement& Placement : Cell.Content.Placements)
		{
			const FVector Location = Placement.Transform.GetLocation();
			const FRotator Rotation = Placement.Transform.Rotator();
			const FVector Scale = Placement.Transform.GetScale3D();
			Text += FString::Printf(
				TEXT("|placement=%s,%d,%s,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,")
				TEXT("%.6f,%.6f,%.6f"),
				*Placement.Id,
				static_cast<int32>(Placement.Kind),
				*Placement.ContentKey,
				Location.X,
				Location.Y,
				Location.Z,
				Rotation.Pitch,
				Rotation.Yaw,
				Rotation.Roll,
				Scale.X,
				Scale.Y,
				Scale.Z);
		}
	}
	return Text;
}

uint32 FGatersBiomeResourceRecipe::Checksum() const
{
	return FCrc::StrCrc32(*CanonicalText());
}

FGatersBiomeResourceCompileResult FGatersBiomeResourceCompiler::Compile(
	const FGatersEnvironmentRecipe& Environment,
	const FIntRect& CellBounds,
	const float CellSize,
	const FGatersBiomeResourceSettings& Settings)
{
	FGatersBiomeResourceCompileResult Result;
	Result.Recipe.EnvironmentVersion = Environment.Version;
	Result.Recipe.BiomeOpportunityVersion = Environment.BiomeOpportunities.Version;
	Result.Recipe.Seed = Environment.Seed;
	Result.Recipe.CellBounds = CellBounds;
	Result.Recipe.CellSize = CellSize;

	TArray<FString> EnvironmentErrors;
	if (!Environment.Validate(EnvironmentErrors))
	{
		AddIssue(Result.Issues, EnvironmentInvalidRule, TEXT("environment"),
			EnvironmentErrors.IsEmpty()
				? TEXT("environment recipe is invalid")
				: EnvironmentErrors[0]);
		return Result;
	}
	if (CellBounds.Max.X <= CellBounds.Min.X || CellBounds.Max.Y <= CellBounds.Min.Y)
	{
		AddIssue(Result.Issues, BoundsInvalidRule, TEXT("bounds"),
			TEXT("cell bounds must be non-empty and half-open"));
		return Result;
	}
	if (!FMath::IsFinite(CellSize) || CellSize <= 0.f)
	{
		AddIssue(Result.Issues, CellSizeInvalidRule, TEXT("cell-size"),
			TEXT("cell size must be finite and positive"));
		return Result;
	}
	const int64 Width = static_cast<int64>(CellBounds.Max.X) - CellBounds.Min.X;
	const int64 Height = static_cast<int64>(CellBounds.Max.Y) - CellBounds.Min.Y;
	const int64 CellCount = Width * Height;
	if (Settings.MaxCellCount <= 0 || CellCount > Settings.MaxCellCount)
	{
		AddIssue(Result.Issues, CellBudgetRule, TEXT("bounds"),
			TEXT("requested cell count exceeds the compile budget"));
		return Result;
	}

	Result.Recipe.Cells.Reserve(static_cast<int32>(CellCount));
	for (int32 X = CellBounds.Min.X; X < CellBounds.Max.X; ++X)
	{
		for (int32 Y = CellBounds.Min.Y; Y < CellBounds.Max.Y; ++Y)
		{
			FGatersBiomeResourceCell& Cell = Result.Recipe.Cells.AddDefaulted_GetRef();
			Cell.Coordinate = FIntPoint(X, Y);
			Cell.Id = CellId(Environment.Seed, Cell.Coordinate);
			const FVector2D Center(X * CellSize, Y * CellSize);
			FGatersBiomeQuery Query;
			Query.PadRadius = Settings.ContentSemantics.PadRadius;
			Query.RouteTarget = Settings.ContentSemantics.RouteTarget;
			Query.NormalSampleDistance = (CellSize / 4.f) * 0.25f;
			const FGatersBiomeSample Biome = Environment.QueryBiome(Center, Query);
			const FGatersBiomeOpportunitySample Opportunities =
				Environment.QueryOpportunities(Center, Query);
			const FGatersWorldRegionIntent& Region = Environment.Intent.At(Center);
			Cell.BiomeKey = Biome.BiomeKey;
			Cell.DeclaredLandmarkOpportunity = Region.LandmarkOpportunity;
			Cell.LandmarkOpportunity =
				Region.LandmarkOpportunity * Opportunities.Landmark;
			Cell.DeclaredTravelFriction = Region.TravelFriction;
			Cell.TravelFriction =
				FMath::Max(Region.TravelFriction, Opportunities.TravelFriction);
			Cell.Content = FGatersContentCellRecipe::Generate(
				Cell.Coordinate,
				CellSize,
				Environment,
				Settings.ContentSemantics);
			Result.Recipe.PlacementBudget += Cell.Content.MaxPlacements;
			AddCoverage(Result.Recipe.Coverage, Cell.Content.Coverage);
		}
	}
	Result.Recipe.Validate(Result.Issues);
	return Result;
}
