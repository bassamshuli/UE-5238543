// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Soldier.h"
#include "UTurnManager.h"
#include "Tile.h"
#include "Components/VerticalBox.h"
#include "BaseGameMode.generated.h"

class UWBP_Game;
class AObstacles;
class ATurnControllerActor;
class AGameFeild;
class UTurnManger;

UCLASS()
class TEST_API ABaseGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ABaseGameMode();
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable)
    void StartGame();

    UFUNCTION()
    void PlayerChoseStartingUnit(bool bBrawlerFirst);

    UFUNCTION()
    void NextTurn();

    UFUNCTION()
    void HandleTileClicked(ATile* ClickedTile);

    UFUNCTION()
    void HandleSoldierSelected(ASoldier* Soldier);

    UFUNCTION(BlueprintCallable)
    bool IsPlacementPhase() const;

    UFUNCTION()
    void OnPlacementPhaseComplete();

    UFUNCTION(BlueprintCallable)
    void ResetGame();
    void ClearSelectedSoldier();
    void SetupAISpawnQueue();

    UPROPERTY()
    ATurnControllerActor* TurnControllerActor;
private:
    void PlaceAIUnitDelayed();
    void PlaceAIUnit();

    FTimerHandle SpawnDelayHandle;
    FTimerHandle DelayHandle;

public:
    UPROPERTY(BlueprintReadOnly)
    int32 CurrentUnitIndex = 0;

    UPROPERTY()
    class UTurnManager* TurnManager;

    UPROPERTY(BlueprintReadOnly)
    bool bIsPlayerTurn = true;

    UPROPERTY()
    TArray<TSubclassOf<class ASoldier>> SpawnQueue;


    UPROPERTY(EditAnywhere, Category = "Setup") TSubclassOf<ASoldier> PlayerBrawlerClass;
    UPROPERTY(EditAnywhere, Category = "Setup") TSubclassOf<ASoldier> AIBrawlerClass;
    UPROPERTY(EditAnywhere, Category = "Setup") TSubclassOf<ASoldier> PlayerSniperClass;
    UPROPERTY(EditAnywhere, Category = "Setup") TSubclassOf<ASoldier> AISniperClass;

    UPROPERTY()
    UWBP_Game* GameUIInstance;

    UPROPERTY(BlueprintReadWrite)
    TArray<ATile*> Tiles;

    UPROPERTY()
    ASoldier* SelectedSoldier = nullptr;

    UPROPERTY()
    ASoldier* SelectedSoldier_Current = nullptr;

    void SetTurnManager(UTurnManager* InManager) { TurnManager = InManager; }

    bool bActionPhaseStarted = false;
    ETeam CurrentTurnTeam;
    ETeam StartingTeam;

    UPROPERTY()
    TArray<ASoldier*> PlayerSoldiers;

    UPROPERTY()
    TArray<ASoldier*> AISoldiers;

    void ClearAllHighlights();
};