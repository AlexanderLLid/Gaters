#pragma once

#include "CoreMinimal.h"
#include "GatersBuildingGenerator.h"

struct PROTOTYPE_API FGatersBuildingIssue
{
	FString RuleId;
	FString SubjectId;
	FString Message;
};

struct PROTOTYPE_API FGatersBuildingEvaluation
{
	bool IsValid() const { return Issues.IsEmpty(); }
	FString Summary() const;

	int32 EvaluatorVersion = 1;
	int32 ModuleCount = 0;
	int32 UniqueModuleCount = 0;
	int32 EntranceCount = 0;
	TArray<FGatersBuildingIssue> Issues;
};

struct PROTOTYPE_API FGatersBuildingEvaluator
{
	static FGatersBuildingEvaluation Evaluate(
		const FGatersTerrainSemanticField& Field,
		const FGatersBuildingAssembly& Assembly);
};
