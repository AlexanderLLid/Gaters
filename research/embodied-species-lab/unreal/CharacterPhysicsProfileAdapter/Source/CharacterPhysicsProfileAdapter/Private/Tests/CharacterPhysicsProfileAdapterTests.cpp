#if WITH_DEV_AUTOMATION_TESTS

#include "CharacterPhysicsProfileAdapterLibrary.h"
#include "Misc/AutomationTest.h"

namespace
{
TSet<FName> MakeSkeleton()
{
    return {TEXT("pelvis"), TEXT("spine"), TEXT("chest"), TEXT("head")};
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterTopologyConnectedTest,
    "Gaters.CharacterLab.PhysicsProfileAdapter.ConnectedTree",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterTopologyConnectedTest::RunTest(const FString& Parameters)
{
    FString Error;
    const bool bPassed = Gaters::CharacterLab::ValidateTopology(
        MakeSkeleton(),
        {TEXT("pelvis"), TEXT("spine"), TEXT("chest")},
        {TEXT("pelvis"), TEXT("spine")},
        {TEXT("spine"), TEXT("chest")},
        Error);
    TestTrue(TEXT("Connected topology passes"), bPassed);
    TestTrue(TEXT("Connected topology has no error"), Error.IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterTopologyUnknownBoneTest,
    "Gaters.CharacterLab.PhysicsProfileAdapter.UnknownBone",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterTopologyUnknownBoneTest::RunTest(const FString& Parameters)
{
    FString Error;
    const bool bPassed = Gaters::CharacterLab::ValidateTopology(
        MakeSkeleton(),
        {TEXT("pelvis"), TEXT("missing")},
        {TEXT("pelvis")},
        {TEXT("missing")},
        Error);
    TestFalse(TEXT("Unknown body bone fails"), bPassed);
    TestTrue(TEXT("Unknown body bone is causal"), Error.Contains(TEXT("unknown skeleton bone")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterTopologyUnknownJointBoneTest,
    "Gaters.CharacterLab.PhysicsProfileAdapter.UnknownJointBone",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterTopologyUnknownJointBoneTest::RunTest(const FString& Parameters)
{
    FString Error;
    const bool bPassed = Gaters::CharacterLab::ValidateTopology(
        MakeSkeleton(),
        {TEXT("pelvis"), TEXT("spine")},
        {TEXT("pelvis")},
        {TEXT("missing")},
        Error);
    TestFalse(TEXT("Unknown joint bone fails"), bPassed);
    TestTrue(TEXT("Unknown joint bone is causal"), Error.Contains(TEXT("unknown skeleton bone")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterTopologyDuplicateBodyTest,
    "Gaters.CharacterLab.PhysicsProfileAdapter.DuplicateBody",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterTopologyDuplicateBodyTest::RunTest(const FString& Parameters)
{
    FString Error;
    const bool bPassed = Gaters::CharacterLab::ValidateTopology(
        MakeSkeleton(),
        {TEXT("pelvis"), TEXT("spine"), TEXT("spine")},
        {TEXT("pelvis"), TEXT("spine")},
        {TEXT("spine"), TEXT("chest")},
        Error);
    TestFalse(TEXT("Duplicate body fails"), bPassed);
    TestTrue(TEXT("Duplicate body is causal"), Error.Contains(TEXT("duplicate body bone")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterTopologyMismatchedArraysTest,
    "Gaters.CharacterLab.PhysicsProfileAdapter.MismatchedArrays",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterTopologyMismatchedArraysTest::RunTest(const FString& Parameters)
{
    FString Error;
    const bool bPassed = Gaters::CharacterLab::ValidateTopology(
        MakeSkeleton(),
        {TEXT("pelvis"), TEXT("spine")},
        {TEXT("pelvis")},
        {},
        Error);
    TestFalse(TEXT("Mismatched arrays fail"), bPassed);
    TestTrue(TEXT("Mismatched arrays are causal"), Error.Contains(TEXT("equal length")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharacterTopologyDisconnectedTest,
    "Gaters.CharacterLab.PhysicsProfileAdapter.DisconnectedGraph",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharacterTopologyDisconnectedTest::RunTest(const FString& Parameters)
{
    FString Error;
    const bool bPassed = Gaters::CharacterLab::ValidateTopology(
        MakeSkeleton(),
        {TEXT("pelvis"), TEXT("spine"), TEXT("chest"), TEXT("head")},
        {TEXT("pelvis"), TEXT("chest"), TEXT("head")},
        {TEXT("spine"), TEXT("head"), TEXT("chest")},
        Error);
    TestFalse(TEXT("Disconnected graph fails"), bPassed);
    TestTrue(TEXT("Disconnected graph is causal"), Error.Contains(TEXT("connected")));
    return true;
}

#endif
