#include "GatersDebugMessages.h"

#include "Engine/Engine.h"

namespace
{
bool bGatersDebugMessagesEnabled = false;
}

bool FGatersDebugMessages::IsEnabled()
{
	return bGatersDebugMessagesEnabled;
}

void FGatersDebugMessages::SetEnabled(const bool bEnabled)
{
	bGatersDebugMessagesEnabled = bEnabled;
	if (!bEnabled && GEngine)
	{
		GEngine->RemoveOnScreenDebugMessage(ReportKey);
		GEngine->RemoveOnScreenDebugMessage(TraversalKey);
		GEngine->RemoveOnScreenDebugMessage(RaidKey);
	}
}

void FGatersDebugMessages::Show(
	const uint64 Key,
	const float Duration,
	const FColor& Color,
	const FString& Text)
{
	if (bGatersDebugMessagesEnabled && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(Key, Duration, Color, Text);
	}
}
