#include "GatersEnvironmentCandidateSelector.h"

#include "GatersTerrainSemanticField.h"
#include "GatersTraversabilityEvaluator.h"

namespace
{
bool IsUnit(float Value)
{
	return FMath::IsFinite(Value) && Value >= 0.f && Value <= 1.f;
}

void AddIssue(
	FGatersEnvironmentCandidateSelectionResult& Result,
	const TCHAR* RuleId,
	const TCHAR* Message)
{
	Result.Issues.Add({RuleId, Message});
}
}

FGatersEnvironmentCandidateSelectionResult FGatersEnvironmentCandidateSelector::Select(
	const FGatersEnvironment& Environment,
	const FGatersCompiledEnvironmentBrief& Intent,
	const TArray<FGatersLandformProtectedRegion>& ProtectedRegions,
	const FGatersEnvironmentCandidateSelectionSettings& Settings)
{
	FGatersEnvironmentCandidateSelectionResult Result;
	if (Settings.CandidateCount <= 0 || Settings.CandidateCount > 64
		|| Settings.WorldCellsPerAxis < 3 || Settings.WorldCellsPerAxis % 2 == 0
		|| Settings.ArrivalCellsPerAxis < 3 || Settings.ArrivalCellsPerAxis % 2 == 0
		|| !FMath::IsFinite(Settings.ArrivalCellSize)
		|| Settings.ArrivalCellSize <= 0.f || !FMath::IsFinite(Settings.PadRadius)
		|| Settings.PadRadius <= 0.f || !IsUnit(Settings.FlatNormalZ)
		|| !IsUnit(Settings.SlopeNormalZ)
		|| Settings.FlatNormalZ < Settings.SlopeNormalZ
		|| Settings.EscapeDistanceCells < 1
		|| !IsUnit(Settings.WalkableTolerance)
		|| !IsUnit(Settings.ConnectedTolerance))
	{
		AddIssue(Result, TEXT("land-access.settings"),
			TEXT("Candidate selection settings must define a finite bounded search."));
		return Result;
	}
	if (!IsUnit(Intent.LandAccess.WalkableLand)
		|| !IsUnit(Intent.LandAccess.ConnectedLand))
	{
		AddIssue(Result, TEXT("land-access.target"),
			TEXT("Compiled land-access targets must be finite and inside [0, 1]."));
		return Result;
	}

	float BestScore = TNumericLimits<float>::Max();
	float SelectedScore = TNumericLimits<float>::Max();
	for (int32 CandidateIndex = 0;
		CandidateIndex < Settings.CandidateCount;
		++CandidateIndex)
	{
		const FGatersLandformProcessCompileResult Compiled =
			FGatersLandformProcessField::Compile(
				Environment, Intent, ProtectedRegions, CandidateIndex);
		if (!Compiled.IsValid())
		{
			AddIssue(Result, TEXT("land-access.landform"),
				TEXT("A landform candidate could not be compiled."));
			return Result;
		}

		const FGatersEnvironment Candidate =
			Environment.WithLandformProcesses(Compiled.Recipe);
		const float WorldCellSize = Intent.WorldSize
			/ static_cast<float>(Settings.WorldCellsPerAxis - 1);
		const FGatersTerrainSemanticField WorldField =
			FGatersTerrainSemanticField::Build(
				Candidate, Settings.WorldCellsPerAxis, WorldCellSize,
				Settings.PadRadius, Settings.FlatNormalZ, Settings.SlopeNormalZ);
		const int32 WorldHalf = Settings.WorldCellsPerAxis / 2;
		const FGatersTraversabilityEvaluation WorldEvaluation =
			FGatersTraversabilityEvaluator::Evaluate(
				WorldField, FIntPoint(WorldHalf, WorldHalf),
				FIntPoint(WorldHalf, WorldHalf), 1);
		const FGatersTerrainSemanticField ArrivalField =
			FGatersTerrainSemanticField::Build(
				Candidate, Settings.ArrivalCellsPerAxis, Settings.ArrivalCellSize,
				Settings.PadRadius, Settings.FlatNormalZ, Settings.SlopeNormalZ);
		const int32 ArrivalHalf = Settings.ArrivalCellsPerAxis / 2;
		const FGatersTraversabilityEvaluation ArrivalEvaluation =
			FGatersTraversabilityEvaluator::Evaluate(
				ArrivalField, FIntPoint(ArrivalHalf, ArrivalHalf),
				FIntPoint(ArrivalHalf, ArrivalHalf),
				Settings.EscapeDistanceCells);

		FGatersEnvironmentCandidateEvidence Evidence;
		Evidence.CandidateIndex = CandidateIndex;
		Evidence.WalkableLand = WorldEvaluation.WalkableFraction;
		Evidence.ConnectedLand = WorldEvaluation.ReachableFraction;
		Evidence.bHasWorldAccess = WorldEvaluation.bEscapesStart;
		Evidence.bEscapesArrival = ArrivalEvaluation.bEscapesStart;
		Evidence.WalkableError = FMath::Abs(
			Evidence.WalkableLand - Intent.LandAccess.WalkableLand);
		Evidence.ConnectedError = FMath::Abs(
			Evidence.ConnectedLand - Intent.LandAccess.ConnectedLand);
		Evidence.Score = Evidence.WalkableError + Evidence.ConnectedError;
		Result.Candidates.Add(Evidence);
		if (Evidence.Score < BestScore)
		{
			BestScore = Evidence.Score;
			Result.Best = Evidence;
			Result.BestRecipe = Compiled.Recipe;
		}

		const bool bSatisfies = Evidence.bHasWorldAccess
			&& Evidence.bEscapesArrival
			&& Evidence.WalkableError <= Settings.WalkableTolerance
			&& Evidence.ConnectedError <= Settings.ConnectedTolerance
			&& Evidence.WalkableLand >= Intent.LandAccess.WalkableLand
				* (1.f - Settings.WalkableTolerance)
			&& Evidence.ConnectedLand >= Intent.LandAccess.ConnectedLand
				* (1.f - Settings.ConnectedTolerance);
		if (bSatisfies && Evidence.Score < SelectedScore)
		{
			SelectedScore = Evidence.Score;
			Result.bSelected = true;
			Result.Selected = Evidence;
			Result.SelectedRecipe = Compiled.Recipe;
		}
	}
	return Result;
}
