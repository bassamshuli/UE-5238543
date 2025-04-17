// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerControllerBase.h"
#include "Soldier.h"
#include "Tile.h"
#include "UTurnManager.h"

APlayerControllerBase::APlayerControllerBase()
{
    PrimaryActorTick.bCanEverTick = true;
    SelectedSoldier = nullptr;
    TurnManager = nullptr;
}

void APlayerControllerBase::BeginPlay()
{
    Super::BeginPlay();
    FString LevelName = GetWorld()->GetMapName();
    LevelName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

    if (LevelName == "IntroLevel") // check if IntroLevel is active
    {
        bShowMouseCursor = true;
        bEnableClickEvents = true;
        bEnableMouseOverEvents = true;
    }
}

void APlayerControllerBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APlayerControllerBase::HandleSoldierSelected(ASoldier* Soldier)
{
    if (Soldier && Soldier->CanPerformAction())
    {
        SelectedSoldier = Soldier;
        Soldier->ShowMovableTiles(AllTiles);
    }
}

void APlayerControllerBase::HandleTileClicked(ATile* ClickedTile)
{
    if (!SelectedSoldier || !ClickedTile || SelectedSoldier->HasCompletedTurn())
        return;

    // Controlla se può muoversi
    if (!SelectedSoldier->bHasMoved && ClickedTile->bIsSelected)
    {
        SelectedSoldier->MoveToTile(ClickedTile, AllTiles);
        SelectedSoldier->bHasMoved = true;
    }

    // Se ha completato tutto, chiama il turn manager
    if (SelectedSoldier->HasCompletedTurn() && TurnManager)
    {
        TurnManager->OnSoldierActionCompleted(SelectedSoldier);
        SelectedSoldier = nullptr;
    }
}

void APlayerControllerBase::HandleEnemyClicked(ASoldier* Enemy)
{
    if (!SelectedSoldier || !Enemy || SelectedSoldier->HasCompletedTurn())
        return;

    if (!SelectedSoldier->bHasAttacked && SelectedSoldier->IsInAttackRange(Enemy))
    {
        // Chiama la logica centralizzata con contrattacco
        SelectedSoldier->Attack(Enemy);
        SelectedSoldier->bHasAttacked = true;
    }

    if (SelectedSoldier->HasCompletedTurn() && TurnManager)
    {
        TurnManager->OnSoldierActionCompleted(SelectedSoldier);
        SelectedSoldier = nullptr;
    }
}

void APlayerControllerBase::SetTurnManager(UTurnManager* InManager)
{
    TurnManager = InManager;
}

void APlayerControllerBase::SetAllTiles(const TArray<ATile*>& Tiles)
{
    AllTiles = Tiles;
}



