#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GatersRaider.generated.h"

// Scripted probe raider for base evaluation. Generator-agnostic by contract: it consumes
// only world state — actors tagged "Breakable" can be destroyed, the one actor tagged
// "RaidLoot" is the goal — so any generator (or a player-built base) that produces tagged
// actors is evaluable without touching this file. Each run appends one JSON line to
// Saved/RaidResults.jsonl.
UCLASS()
class PROTOTYPE_API AGatersRaider : public ACharacter
{
	GENERATED_BODY()

public:
	AGatersRaider();

	// free-form world description (e.g. "seed=3") copied verbatim into the result line;
	// the raider itself never knows what a seed is
	UPROPERTY(EditAnywhere, Category = "Gaters")
	FString Context;

	UPROPERTY(EditAnywhere, Category = "Gaters")
	float TimeLimit = 180.f;

	UPROPERTY(EditAnywhere, Category = "Gaters")
	float AttackSeconds = 1.f;

	UPROPERTY(EditAnywhere, Category = "Gaters")
	float AttackRange = 450.f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	enum class EState { Approach, Direct, Attack, Done };
	EState State = EState::Approach;

	TWeakObjectPtr<AActor> Loot;
	TWeakObjectPtr<AActor> Victim;
	float Elapsed = 0.f;
	float StallTime = 0.f;
	float AttackTime = 0.f;
	int32 Broken = 0;
	bool bPathfindingWorked = false;

	void StartApproach();
	void EnterBreach();
	AActor* NearestBreakable(float MaxDist) const;
	void Finish(bool bSuccess, const TCHAR* Why);
};
