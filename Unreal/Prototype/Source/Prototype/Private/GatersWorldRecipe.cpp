#include "GatersWorldRecipe.h"

#include "GatersStructuralEvaluator.h"
#include "GatersWorldDiff.h"
#include "Misc/Crc.h"

FGatersWorldRecipe FGatersWorldRecipe::Generate(
	int32 InSeed,
	float InChunkSize,
	float MinBaseDistance,
	float MaxBaseDistance,
	float BaseFootprintRadius,
	float MaxFoundationDrop)
{
	const FGatersEnvironment Environment = FGatersEnvironment::FromSeed(InSeed, InChunkSize);

	FGatersWorldRecipe Recipe;
	Recipe.GeneratorVersion = GatersGenVersion;
	Recipe.Seed = InSeed;
	Recipe.ChunkSize = InChunkSize;
	Recipe.EnvironmentType = Environment.Type;
	Recipe.Hydrology = Environment.Hydrology;
	Recipe.EnvironmentName = Environment.Name();
	Recipe.WaterHeight = Environment.WaterHeight;
	Recipe.bHasBaseSite = Environment.FindBaseSite(
		MinBaseDistance,
		MaxBaseDistance,
		BaseFootprintRadius,
		MaxFoundationDrop,
		Recipe.BaseSite);
	Recipe.Nodes.Add({TEXT("gate:0"), EGatersRecipeNodeKind::Gate, FVector::ZeroVector});
	if (Recipe.bHasBaseSite)
	{
		Recipe.Nodes.Add({
			TEXT("base:0"),
			EGatersRecipeNodeKind::BaseSite,
			FVector(Recipe.BaseSite, Environment.HeightAt(Recipe.BaseSite))});
	}
	return Recipe;
}

FString FGatersWorldRecipe::CanonicalText() const
{
	FString Result = FString::Printf(
		TEXT("schema=%d;generator=%d;seed=%d;chunk=%.3f;environment=%s;hydrology=%d;water=%.3f;base=%d,%.3f,%.3f"),
		SchemaVersion,
		GeneratorVersion,
		Seed,
		ChunkSize,
		*EnvironmentName,
		static_cast<int32>(Hydrology),
		WaterHeight,
		bHasBaseSite ? 1 : 0,
		BaseSite.X,
		BaseSite.Y);
	for (const FGatersRecipeNode& Node : Nodes)
	{
		Result += FString::Printf(
			TEXT(";node=%s,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%s"),
			*Node.Id,
			static_cast<int32>(Node.Kind),
			Node.Location.X,
			Node.Location.Y,
			Node.Location.Z,
			Node.Rotation.Pitch,
			Node.Rotation.Yaw,
			Node.Rotation.Roll,
			Node.Scale.X,
			Node.Scale.Y,
			Node.Scale.Z,
			*Node.ContentKey);
	}
	for (const FGatersRecipeLink& Link : Links)
	{
		Result += FString::Printf(
			TEXT(";link=%s,%s,%s,%s,%s"),
			*Link.Id,
			*Link.FromNodeId,
			*Link.FromPort,
			*Link.ToNodeId,
			*Link.ToPort);
	}
	return Result;
}

uint32 FGatersWorldRecipe::Checksum() const
{
	return FCrc::StrCrc32(*CanonicalText());
}

FGatersEnvironment FGatersWorldRecipe::CreateEnvironment() const
{
	// ponytail: seed reconstruction is enough until recipes can edit terrain parameters.
	return FGatersEnvironment::FromSeed(Seed, ChunkSize);
}

const FGatersRecipeNode* FGatersWorldRecipe::FindNode(const FString& Id) const
{
	return Nodes.FindByPredicate([&Id](const FGatersRecipeNode& Node)
	{
		return Node.Id == Id;
	});
}

bool FGatersWorldRecipe::Validate(TArray<FString>& OutErrors) const
{
	OutErrors.Reset();
	const FGatersStructuralEvaluation Evaluation = FGatersStructuralEvaluator::Evaluate(*this);
	OutErrors.Reserve(Evaluation.Issues.Num());
	for (const FGatersStructuralIssue& Issue : Evaluation.Issues)
	{
		OutErrors.Add(Issue.Message);
	}
	return Evaluation.IsValid();
}
