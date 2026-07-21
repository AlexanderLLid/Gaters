#include "GatersBuiltSiteRecipeJson.h"

#include "GatersBuiltSiteLayer.h"
#include "GatersEnvironmentRecipe.h"
#include "GatersSiteRoutePlanner.h"
#include "GatersTerrainSemanticField.h"
#include "GatersWorldRecipe.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonWriter.h"

namespace
{
using FGatersJsonWriter = TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>;

const TCHAR* KindName(const EGatersBuiltSiteKind Kind)
{
	switch (Kind)
	{
	case EGatersBuiltSiteKind::Settlement: return TEXT("settlement");
	case EGatersBuiltSiteKind::Outpost: return TEXT("outpost");
	case EGatersBuiltSiteKind::Base: return TEXT("base");
	case EGatersBuiltSiteKind::Fortress: return TEXT("fortress");
	case EGatersBuiltSiteKind::Dungeon: return TEXT("dungeon");
	default: return nullptr;
	}
}

void WriteVector(
	const TSharedRef<FGatersJsonWriter>& Writer,
	const TCHAR* Name,
	const FVector& Value)
{
	Writer->WriteObjectStart(Name);
	Writer->WriteValue(TEXT("x"), Value.X);
	Writer->WriteValue(TEXT("y"), Value.Y);
	Writer->WriteValue(TEXT("z"), Value.Z);
	Writer->WriteObjectEnd();
}

void WriteStrings(
	const TSharedRef<FGatersJsonWriter>& Writer,
	const TCHAR* Name,
	const TArray<FString>& Values)
{
	Writer->WriteArrayStart(Name);
	for (const FString& Value : Values)
	{
		Writer->WriteValue(Value);
	}
	Writer->WriteArrayEnd();
}

void WriteRecipe(
	const TSharedRef<FGatersJsonWriter>& Writer,
	const FGatersBuiltSiteRecipe& Recipe)
{
	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("contractVersion"), Recipe.ContractVersion);
	Writer->WriteValue(TEXT("siteVersion"), Recipe.SiteVersion);
	Writer->WriteValue(TEXT("generatorVersion"), Recipe.GeneratorVersion);
	Writer->WriteValue(TEXT("seed"), Recipe.Seed);
	Writer->WriteValue(TEXT("siteId"), Recipe.SiteId);
	Writer->WriteValue(TEXT("kind"), KindName(Recipe.Kind));
	Writer->WriteValue(TEXT("siteArea"), Recipe.SiteArea);
	Writer->WriteValue(TEXT("checksum"), static_cast<int64>(Recipe.Checksum()));
	Writer->WriteObjectStart(TEXT("evidenceCoverage"));
	Writer->WriteValue(TEXT("placement"), Recipe.EvidenceCoverage.bPlacement);
	Writer->WriteValue(TEXT("traversalClearance"),
		Recipe.EvidenceCoverage.bTraversalClearance);
	Writer->WriteValue(TEXT("visibility"), Recipe.EvidenceCoverage.bVisibility);
	Writer->WriteValue(TEXT("blockers"), Recipe.EvidenceCoverage.bBlockers);
	WriteStrings(Writer, TEXT("sourceIds"), Recipe.EvidenceCoverage.SourceIds);
	Writer->WriteObjectEnd();

	Writer->WriteArrayStart(TEXT("spaces"));
	for (const FGatersBuiltSiteSpace& Space : Recipe.Spaces)
	{
		Writer->WriteObjectStart();
		Writer->WriteValue(TEXT("id"), Space.Id);
		WriteVector(Writer, TEXT("center"), Space.Center);
		WriteVector(Writer, TEXT("extent"), Space.Extent);
		Writer->WriteValue(TEXT("semanticRole"), Space.SemanticRole);
		WriteStrings(Writer, TEXT("tags"), Space.Tags);
		WriteStrings(Writer, TEXT("sourceIds"), Space.SourceIds);
		Writer->WriteObjectEnd();
	}
	Writer->WriteArrayEnd();

	Writer->WriteArrayStart(TEXT("connections"));
	for (const FGatersBuiltSiteConnection& Connection : Recipe.Connections)
	{
		Writer->WriteObjectStart();
		Writer->WriteValue(TEXT("id"), Connection.Id);
		Writer->WriteValue(TEXT("fromSpaceId"), Connection.FromSpaceId);
		Writer->WriteValue(TEXT("toSpaceId"), Connection.ToSpaceId);
		Writer->WriteValue(TEXT("width"), Connection.Width);
		Writer->WriteValue(TEXT("headroom"), Connection.Headroom);
		Writer->WriteValue(TEXT("maxStepHeight"), Connection.MaxStepHeight);
		Writer->WriteValue(TEXT("maxJumpDistance"), Connection.MaxJumpDistance);
		WriteStrings(Writer, TEXT("movementModeIds"), Connection.MovementModeIds);
		WriteStrings(Writer, TEXT("blockerIds"), Connection.BlockerIds);
		WriteStrings(Writer, TEXT("tags"), Connection.Tags);
		WriteStrings(Writer, TEXT("sourceIds"), Connection.SourceIds);
		Writer->WriteObjectEnd();
	}
	Writer->WriteArrayEnd();

	Writer->WriteArrayStart(TEXT("visibility"));
	for (const FGatersBuiltSiteVisibility& Sight : Recipe.Visibility)
	{
		Writer->WriteObjectStart();
		Writer->WriteValue(TEXT("id"), Sight.Id);
		Writer->WriteValue(TEXT("fromSpaceId"), Sight.FromSpaceId);
		Writer->WriteValue(TEXT("toSpaceId"), Sight.ToSpaceId);
		Writer->WriteValue(TEXT("distance"), Sight.Distance);
		Writer->WriteValue(TEXT("fromHeight"), Sight.FromHeight);
		Writer->WriteValue(TEXT("toHeight"), Sight.ToHeight);
		WriteStrings(Writer, TEXT("blockerIds"), Sight.BlockerIds);
		WriteStrings(Writer, TEXT("tags"), Sight.Tags);
		WriteStrings(Writer, TEXT("sourceIds"), Sight.SourceIds);
		Writer->WriteObjectEnd();
	}
	Writer->WriteArrayEnd();

	Writer->WriteArrayStart(TEXT("blockers"));
	for (const FGatersBuiltSiteBlocker& Blocker : Recipe.Blockers)
	{
		Writer->WriteObjectStart();
		Writer->WriteValue(TEXT("id"), Blocker.Id);
		WriteVector(Writer, TEXT("center"), Blocker.Center);
		WriteVector(Writer, TEXT("extent"), Blocker.Extent);
		WriteStrings(Writer, TEXT("tags"), Blocker.Tags);
		WriteStrings(Writer, TEXT("sourceIds"), Blocker.SourceIds);
		Writer->WriteObjectEnd();
	}
	Writer->WriteArrayEnd();

	Writer->WriteArrayStart(TEXT("placementSlots"));
	for (const FGatersBuiltSitePlacementSlot& Slot : Recipe.PlacementSlots)
	{
		Writer->WriteObjectStart();
		Writer->WriteValue(TEXT("id"), Slot.Id);
		Writer->WriteValue(TEXT("spaceId"), Slot.SpaceId);
		WriteVector(Writer, TEXT("location"), Slot.Location);
		Writer->WriteValue(TEXT("clearanceRadius"), Slot.ClearanceRadius);
		Writer->WriteValue(TEXT("clearanceHeight"), Slot.ClearanceHeight);
		WriteStrings(Writer, TEXT("tags"), Slot.Tags);
		WriteStrings(Writer, TEXT("sourceIds"), Slot.SourceIds);
		Writer->WriteObjectEnd();
	}
	Writer->WriteArrayEnd();
	Writer->WriteObjectEnd();
}
}

bool FGatersBuiltSiteRecipeJson::Serialize(
	const TArray<FGatersBuiltSiteRecipe>& Recipes,
	FString& OutJson,
	TArray<FString>& OutDiagnostics)
{
	OutJson.Reset();
	OutDiagnostics.Reset();
	for (int32 RecipeIndex = 0; RecipeIndex < Recipes.Num(); ++RecipeIndex)
	{
		const FGatersBuiltSiteRecipe& Recipe = Recipes[RecipeIndex];
		TArray<FGatersBuiltSiteIssue> Issues;
		Recipe.Validate(Issues);
		for (const FGatersBuiltSiteIssue& Issue : Issues)
		{
			OutDiagnostics.Add(FString::Printf(
				TEXT("recipe[%d] %s subject=%s %s"), RecipeIndex, *Issue.RuleId,
				*Issue.SubjectId, *Issue.Message));
		}
		if (!KindName(Recipe.Kind))
		{
			OutDiagnostics.Add(FString::Printf(
				TEXT("recipe[%d] site.kind subject=%s site kind is unsupported"),
				RecipeIndex, *Recipe.SiteId));
		}
	}
	if (!OutDiagnostics.IsEmpty())
	{
		return false;
	}

	const TSharedRef<FGatersJsonWriter> Writer =
		TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutJson);
	Writer->WriteObjectStart();
	Writer->WriteValue(TEXT("exportVersion"), 1);
	Writer->WriteValue(TEXT("coordinateUnit"), TEXT("centimetres"));
	Writer->WriteValue(TEXT("lengthUnit"), TEXT("centimetres"));
	Writer->WriteValue(TEXT("areaUnit"), TEXT("square centimetres"));
	Writer->WriteArrayStart(TEXT("siteRecipes"));
	for (const FGatersBuiltSiteRecipe& Recipe : Recipes)
	{
		WriteRecipe(Writer, Recipe);
	}
	Writer->WriteArrayEnd();
	Writer->WriteObjectEnd();
	if (!Writer->Close())
	{
		OutJson.Reset();
		OutDiagnostics.Add(TEXT("JSON writer failed to close the catalog"));
		return false;
	}
	return true;
}

bool FGatersBuiltSiteRecipeJson::Save(
	const TArray<FGatersBuiltSiteRecipe>& Recipes,
	const FString& OutputPath,
	TArray<FString>& OutDiagnostics)
{
	FString Json;
	if (!Serialize(Recipes, Json, OutDiagnostics))
	{
		return false;
	}
	if (OutputPath.IsEmpty())
	{
		OutDiagnostics.Add(TEXT("output path is empty"));
		return false;
	}
	const FString FullPath = FPaths::ConvertRelativePathToFull(OutputPath);
	const FString Directory = FPaths::GetPath(FullPath);
	if (!Directory.IsEmpty() && !IFileManager::Get().MakeDirectory(*Directory, true))
	{
		OutDiagnostics.Add(FString::Printf(
			TEXT("output directory could not be created: %s"), *Directory));
		return false;
	}
	const FString TempPath = FullPath + TEXT(".tmp");
	if (!FFileHelper::SaveStringToFile(
		Json, *TempPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		OutDiagnostics.Add(FString::Printf(
			TEXT("temporary catalog could not be written: %s"), *TempPath));
		return false;
	}
	if (!IFileManager::Get().Move(*FullPath, *TempPath, true, true))
	{
		IFileManager::Get().Delete(*TempPath);
		OutDiagnostics.Add(FString::Printf(
			TEXT("catalog could not replace destination: %s"), *FullPath));
		return false;
	}
	return true;
}

bool FGatersBuiltSiteRecipeJson::GenerateSettlement(
	const FString& OutputPath,
	const int32 Seed,
	const int32 Stage,
	TArray<FString>& OutDiagnostics)
{
	OutDiagnostics.Reset();
	constexpr float WorldSize = 400000.f;
	constexpr float MinBaseDistance = 6000.f;
	constexpr float MaxBaseDistance = 10800.f;
	constexpr float BaseFootprintRadius = 900.f;
	constexpr float MaxFoundationDrop = 350.f;
	constexpr int32 CellsPerAxis = 61;
	constexpr float CellSize = 500.f;
	constexpr float PadRadius = 1000.f;
	constexpr float FlatNormalZ = 0.94f;
	constexpr float SlopeNormalZ = 0.77f;

	const FGatersEnvironmentRecipe Environment =
		FGatersEnvironmentRecipeCompiler::Compile(Seed, WorldSize);
	if (!Environment.Validate(OutDiagnostics))
	{
		return false;
	}
	const FGatersWorldRecipe World = FGatersWorldRecipe::Generate(
		Environment.Terrain, MinBaseDistance, MaxBaseDistance,
		BaseFootprintRadius, MaxFoundationDrop);
	if (!World.bHasBaseSite)
	{
		OutDiagnostics.Add(FString::Printf(
			TEXT("seed %d has no accepted base site for route planning"), Seed));
		return false;
	}
	const FGatersTerrainSemanticField Field = FGatersTerrainSemanticField::Build(
		Environment.Terrain, Environment.Intent, CellsPerAxis, CellSize, PadRadius,
		FlatNormalZ, SlopeNormalZ, World.BaseSite);
	const FGatersSiteRoutePlan Sites =
		FGatersSiteRoutePlanner::Plan(Field, Seed, World.BaseSite);
	if (!Sites.bValid)
	{
		OutDiagnostics.Append(Sites.Diagnostics);
		if (OutDiagnostics.IsEmpty())
		{
			OutDiagnostics.Add(TEXT("site and route planning failed without a diagnostic"));
		}
		return false;
	}
	const FGatersBuiltSiteLayerResult Layer =
		FGatersBuiltSiteLayer::Generate(Field, Seed, Sites, Stage);
	if (!Layer.IsValid())
	{
		OutDiagnostics.Append(Layer.Diagnostics);
		return false;
	}
	if (Layer.SiteRecipes.IsEmpty())
	{
		OutDiagnostics.Add(TEXT("generated Built Site layer contains no site recipes"));
		return false;
	}
	return Save(Layer.SiteRecipes, OutputPath, OutDiagnostics);
}
