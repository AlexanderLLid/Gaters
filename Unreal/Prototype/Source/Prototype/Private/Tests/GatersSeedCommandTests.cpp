#if WITH_DEV_AUTOMATION_TESTS

#include "HAL/IConsoleManager.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersSeedCommandRegisteredTest,
	"Gaters.Runtime.SeedCommand.Registered",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersSeedCommandRegisteredTest::RunTest(const FString& Parameters)
{
	TestNotNull(TEXT("Gaters.Seed is registered"),
		IConsoleManager::Get().FindConsoleObject(TEXT("Gaters.Seed")));
	return true;
}

#endif
