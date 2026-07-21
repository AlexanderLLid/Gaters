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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersVillageStageCommandRegisteredTest,
	"Gaters.Runtime.VillageStageCommand.Registered",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersVillageStageCommandRegisteredTest::RunTest(const FString& Parameters)
{
	TestNotNull(TEXT("Gaters.VillageStage is registered"),
		IConsoleManager::Get().FindConsoleObject(TEXT("Gaters.VillageStage")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersBuiltSitesCommandRegisteredTest,
	"Gaters.Runtime.BuiltSitesCommand.Registered",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersBuiltSitesCommandRegisteredTest::RunTest(const FString& Parameters)
{
	TestNotNull(TEXT("Gaters.BuiltSites is registered"),
		IConsoleManager::Get().FindConsoleObject(TEXT("Gaters.BuiltSites")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersLandformsCommandRegisteredTest,
	"Gaters.Runtime.LandformsCommand.Registered",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersLandformsCommandRegisteredTest::RunTest(const FString& Parameters)
{
	TestNotNull(TEXT("Gaters.Landforms is registered"),
		IConsoleManager::Get().FindConsoleObject(TEXT("Gaters.Landforms")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersDebugMessagesCommandRegisteredTest,
	"Gaters.Runtime.DebugMessagesCommand.Registered",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersDebugMessagesCommandRegisteredTest::RunTest(const FString& Parameters)
{
	TestNotNull(TEXT("Gaters.DebugMessages is registered"),
		IConsoleManager::Get().FindConsoleObject(TEXT("Gaters.DebugMessages")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGatersContentOpportunitiesCommandRegisteredTest,
	"Gaters.Runtime.ContentOpportunitiesCommand.Registered",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGatersContentOpportunitiesCommandRegisteredTest::RunTest(const FString& Parameters)
{
	TestNotNull(TEXT("Gaters.ContentOpportunities is registered"),
		IConsoleManager::Get().FindConsoleObject(TEXT("Gaters.ContentOpportunities")));
	return true;
}

#endif
