#pragma once

#include "Commandlets/Commandlet.h"
#include "CoreMinimal.h"
#include "GatersBuiltSiteExportCommandlet.generated.h"

UCLASS()
class PROTOTYPE_API UGatersBuiltSiteExportCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UGatersBuiltSiteExportCommandlet();
	virtual int32 Main(const FString& Params) override;
};
