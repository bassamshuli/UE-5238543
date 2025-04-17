// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIStrategyComponent.generated.h"

class ASoldier;
class ATile;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TEST_API UAIStrategyComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAIStrategyComponent();

    void StartAIAction(ASoldier* AISoldier);

protected:
    virtual void BeginPlay() override;

private:
    ASoldier* CurrentSoldier;
    FTimerHandle ActionDelayHandle;

    void TryMove();
    void OnMoveCompleted();
    void TryAttack();
    void FinishTurn();

    TArray<ATile*> CachedTiles;
    TArray<ATile*> FindPathAStar(ATile* StartTile, ATile* GoalTile, const TArray<ATile*>& AllTiles);
    void CacheTiles(); // Only on first call
};
