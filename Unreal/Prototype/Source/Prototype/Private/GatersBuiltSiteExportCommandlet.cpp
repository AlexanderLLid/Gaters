#include "GatersBuiltSiteExportCommandlet.h"

#include "GatersBuiltSiteRecipeJson.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"

UGatersBuiltSiteExportCommandlet::UGatersBuiltSiteExportCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
	UseCommandletResultAsExitCode = true;
}

int32 UGatersBuiltSiteExportCommandlet::Main(const FString& Params)
{
	FString OutputPath;
	if (!FParse::Value(*Params, TEXT("Output="), OutputPath) || OutputPath.IsEmpty())
	{
		UE_LOG(LogTemp, Error,
			TEXT("Usage: -run=GatersBuiltSiteExport -Output=<path> [-Seed=73] [-Stage=1]"));
		return 1;
	}
	int32 Seed = 73;
	int32 Stage = 1;
	FParse::Value(*Params, TEXT("Seed="), Seed);
	FParse::Value(*Params, TEXT("Stage="), Stage);
	TArray<FString> Diagnostics;
	if (!FGatersBuiltSiteRecipeJson::GenerateSettlement(
		OutputPath, Seed, Stage, Diagnostics))
	{
		for (const FString& Diagnostic : Diagnostics)
		{
			UE_LOG(LogTemp, Error, TEXT("GatersBuiltSiteExport: %s"), *Diagnostic);
		}
		return 1;
	}
	UE_LOG(LogTemp, Display,
		TEXT("GatersBuiltSiteExport: wrote %s seed=%d stage=%d"),
		*FPaths::ConvertRelativePathToFull(OutputPath), Seed, Stage);
	return 0;
}
