#if WITH_DEV_AUTOMATION_TESTS

#include "GatersDebugMessages.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersDebugMessagesStateTest,
	"Gaters.Runtime.DebugMessages.State",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersDebugMessagesStateTest::RunTest(const FString& Parameters)
{
	FGatersDebugMessages::SetEnabled(false);
	TestFalse(TEXT("Gaters screen messages default to disabled"),
		FGatersDebugMessages::IsEnabled());
	FGatersDebugMessages::SetEnabled(true);
	TestTrue(TEXT("Gaters screen messages can be enabled"),
		FGatersDebugMessages::IsEnabled());
	FGatersDebugMessages::SetEnabled(false);
	return true;
}

#endif
