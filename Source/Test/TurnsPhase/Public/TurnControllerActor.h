// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TurnControllerActor.generated.h"

class UTurnManager;
class ASoldier;

UCLASS()
class TEST_API ATurnControllerActor : public AActor
{
    GENERATED_BODY()

public:
    ATurnControllerActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Initialize the units to manage
    void SetupSoldiers(const TArray<ASoldier*>& InPlayerSoldiers, const TArray<ASoldier*>& InAISoldiers);

    // Start the first turn
    UFUNCTION(BlueprintCallable)
    void StartGameTurn();

public:
    UPROPERTY()
    UTurnManager* TurnManager;

    void InitializeTurnManager(); 

    TArray<ASoldier*> PlayerSoldiers;
    TArray<ASoldier*> AISoldiers;

};