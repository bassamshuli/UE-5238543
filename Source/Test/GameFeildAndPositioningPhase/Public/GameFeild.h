// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "Tile.h"
#include "WBP_Game.h"
#include "Camera/CameraActor.h"
#include "Soldier.h"
#include "Obstacles.h"
#include "GameFeild.generated.h"

UCLASS()
class TEST_API AGameFeild : public AActor
{
    GENERATED_BODY()

public:
    AGameFeild();

    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;

    void GenerateGrid();
    void GenerateObstacles();

    //GRID CONFIGURATION
    UPROPERTY(EditAnywhere, Category = "Grid") int32 Rows = 25;
    UPROPERTY(EditAnywhere, Category = "Grid") int32 Columns = 25;
    UPROPERTY(EditAnywhere, Category = "Grid") float CellSize = 315.0f;
    UPROPERTY(VisibleAnywhere, Category = "Grid") TArray<ATile*> Tiles;

    //OBSTACLES 
    UPROPERTY() TSubclassOf<AObstacles> MountainBlueprint;
    UPROPERTY() TSubclassOf<AObstacles> TreeBlueprint;
    UPROPERTY() TSubclassOf<AObstacles> ObstacleToSpawn;

    //  UI 
    UPROPERTY(EditAnywhere, Category = "UI") TSubclassOf<UUserWidget> GameWidgetClass;
    UPROPERTY() UWBP_Game* GameUIInstance;

    UPROPERTY(EditAnywhere) bool bAutoGenerateGrid = true;
    bool bGridGenerated = false;

    //GAME STATE
    TArray<TSubclassOf<ASoldier>> SpawnQueue;
    int32 CurrentUnitIndex = 0;
    bool bIsPlayerTurn = true;

protected:

    ACameraActor* RuntimeCameraActor;

};