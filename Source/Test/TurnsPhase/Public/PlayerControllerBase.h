// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerControllerBase.generated.h"

class ASoldier;
class ATile;
class UTurnManager;

UCLASS()
class TEST_API APlayerControllerBase : public APlayerController
{
    GENERATED_BODY()

public:
    APlayerControllerBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Soldier selection
    void HandleSoldierSelected(ASoldier* Soldier);

    // Click on tile to move
    void HandleTileClicked(ATile* ClickedTile);

    // Click on enemy to attack
    void HandleEnemyClicked(ASoldier* Enemy);

    // Initialized externally
    void SetTurnManager(UTurnManager* InManager);
    void SetAllTiles(const TArray<ATile*>& Tiles);

    UPROPERTY()
    ASoldier* SelectedSoldier;

    UPROPERTY()
    UTurnManager* TurnManager;

    UPROPERTY()
    TArray<ATile*> AllTiles;
};