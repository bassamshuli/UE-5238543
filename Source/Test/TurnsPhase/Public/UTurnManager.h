// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UTurnManager.generated.h"

class ASoldier;
class UAIStrategyComponent;
class ABaseGameMode;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TEST_API UTurnManager : public UActorComponent
{
    GENERATED_BODY()

public:
    // Initialize manager with active soldiers
    void Init(const TArray<ASoldier*>& InPlayerSoldiers, const TArray<ASoldier*>& InAISoldiers);

    // Turn handling
    void StartTurn(bool bIsPlayerTurn);
    void OnSoldierActionCompleted(ASoldier* CompletedSoldier);
    void AdvanceToNextSoldier();
    void EndTurn();
    void CheckIfTurnIsOver();

    // Handle dead soldiers
    void NotifySoldierKilled(ASoldier* DeadSoldier);

    // Get the current soldier
    ASoldier* GetCurrentSoldier() const;

    // Game over check
    UFUNCTION()
    bool CheckGameOver();
    bool bGameOver = false;
    UPROPERTY()
    bool bIsAITakingTurn = false;

    bool IsGameOver() const { return bGameOver; }
private:
    // References to soldiers
    TArray<ASoldier*> PlayerSoldiers;
    TArray<ASoldier*> AISoldiers;

    // Turn state
    bool bPlayerTurn = true;
    bool bTurnEnded = false;
    int32 CurrentSoldierIndex = 0;

    // References
    ABaseGameMode* GameModeRef = nullptr;
    UAIStrategyComponent* AIStrategyComponent = nullptr;
};