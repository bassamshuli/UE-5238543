// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseGameMode.h"
#include "Camera/CameraActor.h"
#include "GameFeild.h"
#include "WBP_Game.h"
#include "Kismet/GameplayStatics.h"
#include "TurnControllerActor.h"
#include "EngineUtils.h"
#include "Tile.h"
#include "Components/Image.h"
#include "TurnControllerActor.h"
#include "Soldier.h"
#include "PlayerControllerBase.h"
#include "BrawlerSoldier.h"
#include "SniperSoldier.h"
#include "GameFramework/GameUserSettings.h"
#include "Obstacles.h"

#include "TimerManager.h"

ABaseGameMode::ABaseGameMode()
{
    DefaultPawnClass = nullptr;

    PlayerBrawlerClass = ABrawlerSoldier::StaticClass();
    AIBrawlerClass = ABrawlerSoldier::StaticClass();

    PlayerSniperClass = ASniperSoldier::StaticClass();
    AISniperClass = ASniperSoldier::StaticClass();

    PlayerControllerClass = APlayerControllerBase::StaticClass();
}

void ABaseGameMode::BeginPlay()
{
    Super::BeginPlay();

    FActorSpawnParameters Params;
    GetWorld()->SpawnActor<AGameFeild>(AGameFeild::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);

    //Spawn del TurnControllerActor se non già presente
    if (!TurnControllerActor)
    {
        TurnControllerActor = GetWorld()->SpawnActor<ATurnControllerActor>(ATurnControllerActor::StaticClass());

        if (TurnControllerActor)
        {
            //Salva anche il TurnManager
            TurnManager = TurnControllerActor->TurnManager;

            if (!TurnManager)
            {
                TurnControllerActor->InitializeTurnManager();
                TurnManager = TurnControllerActor->TurnManager;
            }
        }
    }

    //Trova il GameFeild e assegna GameUIInstance e Tiles
    for (TActorIterator<AGameFeild> It(GetWorld()); It; ++It)
    {
        GameUIInstance = It->GameUIInstance;
        Tiles = It->Tiles;
        break; 
    }

    // Imposta la camera iniziale
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController)
    {
        for (TActorIterator<ACameraActor> It(GetWorld()); It; ++It)
        {
            if (It->GetName().Contains("BP_Camera"))
            {
                PlayerController->SetViewTargetWithBlend(*It, 0.0f);
                break;
            }
        }
    }
}

void ABaseGameMode::StartGame()
{
    CurrentUnitIndex = 0;
    bIsPlayerTurn = FMath::RandBool();
    StartingTeam = bIsPlayerTurn ? ETeam::Player : ETeam::AI;

    if (bIsPlayerTurn)
    {
        if (GameUIInstance)
        {
            GameUIInstance->ShowChooseUnitTypeUI();
        }
    }
    else
    {
        SetupAISpawnQueue();
        if (GameUIInstance)
        {
            GameUIInstance->SetSpawnQueue(SpawnQueue);
            GameUIInstance->ShowPlacementMessage(false, CurrentUnitIndex);
        }

        GetWorldTimerManager().SetTimerForNextTick(this, &ABaseGameMode::NextTurn);
    }
}

void ABaseGameMode::SetupAISpawnQueue()
{
    bool bBrawlerFirst = FMath::RandBool();
    if (bBrawlerFirst)
    {
        SpawnQueue = { AIBrawlerClass, PlayerBrawlerClass, AISniperClass, PlayerSniperClass };
    }
    else
    {
        SpawnQueue = { AISniperClass, PlayerSniperClass, AIBrawlerClass, PlayerBrawlerClass };
    }
}


void ABaseGameMode::PlayerChoseStartingUnit(bool bBrawlerFirst)
{
    if (bBrawlerFirst)
    {
        SpawnQueue = { PlayerBrawlerClass, AIBrawlerClass, PlayerSniperClass, AISniperClass };
    }
    else
    {
        SpawnQueue = { PlayerSniperClass, AISniperClass, PlayerBrawlerClass, AIBrawlerClass };
    }

    CurrentUnitIndex = 0;

    if (GameUIInstance)
    {
        GameUIInstance->SetSpawnQueue(SpawnQueue);
        GameUIInstance->ShowPlacementMessage(true, CurrentUnitIndex);
    }

    NextTurn();
}

void ABaseGameMode::NextTurn()
{
    if (CurrentUnitIndex >= SpawnQueue.Num())
    {
        OnPlacementPhaseComplete();
        return;
    }

    if (bIsPlayerTurn)
    {
        for (ATile* Tile : Tiles)
        {
            if (Tile)
            {
                Tile->PlayerSoldierToSpawn = SpawnQueue[CurrentUnitIndex];
            }
        }
    }

    if (GameUIInstance)
    {
        GameUIInstance->SetSpawnQueue(SpawnQueue);
        GameUIInstance->ShowPlacementMessage(bIsPlayerTurn, CurrentUnitIndex);
    }

    if (!bIsPlayerTurn)
    {
        PlaceAIUnitDelayed();
    }
}

void ABaseGameMode::PlaceAIUnitDelayed()
{
    GetWorldTimerManager().SetTimer(DelayHandle, this, &ABaseGameMode::PlaceAIUnit, 3.0f, false);
}

void ABaseGameMode::PlaceAIUnit()
{
    if (CurrentUnitIndex >= SpawnQueue.Num()) return;

    TArray<ATile*> FreeTiles;
    for (ATile* Tile : Tiles)
    {
        if (Tile && Tile->IsTileFree() && !Tile->bHasObstacle)
        {
            FreeTiles.Add(Tile);
        }
    }

    if (FreeTiles.Num() > 0)
    {
        ATile* SelectedTile = FreeTiles[FMath::RandRange(0, FreeTiles.Num() - 1)];
        FVector SpawnLocation = SelectedTile->GetActorLocation() + FVector(0, 0, 50);

        // Nome univoco per evitare duplicati
        FString BaseName = SpawnQueue[CurrentUnitIndex]->GetName().Contains("Brawler") ? TEXT("Brawler_AI_") : TEXT("Sniper_AI_");
        FString CustomName = BaseName + FString::FromInt(FMath::RandRange(1000, 9999));

        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(*CustomName);

        ASoldier* AIUnit = GetWorld()->SpawnActor<ASoldier>(SpawnQueue[CurrentUnitIndex], SpawnLocation, FRotator::ZeroRotator, SpawnParams);
        if (AIUnit)
        {
            SelectedTile->SetTileOccupied(true);
            AIUnit->Team = ETeam::AI;
            AIUnit->InitSprite();
            AIUnit->TryAssignOwningTile(Tiles);

            if (GameUIInstance)
            {
                GameUIInstance->AddSoldierSpriteToUI(AIUnit);
            }

            CurrentUnitIndex++;
            bIsPlayerTurn = true;

            if (GameUIInstance)
            {
                GameUIInstance->ShowPlacementMessage(true, CurrentUnitIndex);
            }

            if (CurrentUnitIndex >= SpawnQueue.Num())
            {
                OnPlacementPhaseComplete();
            }
            else
            {
                NextTurn();
            }
        }
    }
}

void ABaseGameMode::OnPlacementPhaseComplete()
{
    bActionPhaseStarted = true;
    CurrentTurnTeam = StartingTeam;

    // trova TurnControllerActor e inizializza tutto
    for (TActorIterator<ATurnControllerActor> It(GetWorld()); It; ++It)
    {
        ATurnControllerActor* TurnActor = *It;

        if (TurnActor)
        {
            //  Raccogli tutti i soldati
            PlayerSoldiers.Empty();
            AISoldiers.Empty();

            for (TActorIterator<ASoldier> SoldierIt(GetWorld()); SoldierIt; ++SoldierIt)
            {
                ASoldier* Soldier = *SoldierIt;
                if (Soldier->Team == ETeam::Player)
                {
                    PlayerSoldiers.Add(Soldier);
                }
                else
                {
                    AISoldiers.Add(Soldier);
                }
            }

            TurnActor->SetupSoldiers(PlayerSoldiers, AISoldiers);

            // prendi il TurnManager appena creato
            TurnManager = TurnActor->TurnManager;
            break;
        }
    }

    //  Mostra il messaggio del turno
    if (GameUIInstance)
    {
        GameUIInstance->EndTurn(CurrentTurnTeam == ETeam::Player);
    }

    //  Avvia il primo turno
    if (TurnManager)
    {
        TurnManager->StartTurn(CurrentTurnTeam == ETeam::Player);
    }
}
bool ABaseGameMode::IsPlacementPhase() const
{
    return !bActionPhaseStarted;
}

void ABaseGameMode::HandleSoldierSelected(ASoldier* Soldier)
{
    if (TurnManager && TurnManager->IsGameOver())
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Click Reset to start a new game."));

        return;
    }

    if (!bIsPlayerTurn || !Soldier || !TurnManager) return;

    //  Se è già selezionato, ignora
    if (Soldier == SelectedSoldier)
    {
        return;
    }

    //  Se è stato già selezionato un altro soldato e non ha finito il turno, blocca
    if (SelectedSoldier && SelectedSoldier != Soldier && !SelectedSoldier->HasCompletedTurn())
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT(" Devi completare le azioni del soldato selezionato prima di cambiare."));
        return;
    }

    //  Se il soldato non può fare nulla, blocca
    if (!Soldier->HasEnemiesInRange(Tiles) && !Soldier->HasAvailableTilesToMove())
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT(" Questo soldato non può fare nessuna azione."));
        return;
    }

    // Deseleziona tile precedente
    if (SelectedSoldier && SelectedSoldier->OwningTile)
    {
        SelectedSoldier->OwningTile->SetSelected(false);
    }

    for (ATile* Tile : Tiles)
    {
        if (Tile)
        {
            Tile->SetSelected(false);
            Tile->SetEnemyHighlighted(false);
        }
    }

    SelectedSoldier = Soldier;

    if (SelectedSoldier->OwningTile)
    {
        SelectedSoldier->OwningTile->SetSelected(true);
    }

    // Mostra azioni disponibili
    if (SelectedSoldier->Team == ETeam::Player)
    {
        if (SelectedSoldier->HasCompletedTurn())
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT(" Questo soldato ha già agito."));
            SelectedSoldier->OwningTile->SetSelected(false);
            SelectedSoldier = nullptr;
            return;
        }

        if (SelectedSoldier->bHasMoved && !SelectedSoldier->bHasAttacked)
        {
            SelectedSoldier->ShowOnlyEnemyTilesInRange(Tiles);

            if (!SelectedSoldier->IsEnemyInRange())
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT(" Nessun nemico nel raggio."));
            }
        }
        else
        {
            SelectedSoldier->ShowMovableTiles(Tiles);
        }
    }
}


void ABaseGameMode::HandleTileClicked(ATile* ClickedTile)
{
    //  Blocca interazione se gioco è finito
    if (TurnManager && TurnManager->IsGameOver())
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Click Reset to start a new game."));

        return;
    }

    //  Fase di posizionamento
    if (IsPlacementPhase())
    {
        if (CurrentUnitIndex < SpawnQueue.Num() && bIsPlayerTurn && ClickedTile->IsTileFree())
        {
            FVector SpawnLocation = ClickedTile->GetActorLocation() + FVector(0, 0, 50);
            FString BaseName = SpawnQueue[CurrentUnitIndex]->GetName().Contains("Brawler") ? TEXT("Brawler_Player") : TEXT("Sniper_Player");

            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            SpawnParams.Name = MakeUniqueObjectName(GetWorld(), SpawnQueue[CurrentUnitIndex], FName(*BaseName));

            ASoldier* NewSoldier = GetWorld()->SpawnActor<ASoldier>(SpawnQueue[CurrentUnitIndex], SpawnLocation, FRotator::ZeroRotator, SpawnParams);

            if (NewSoldier)
            {
                ClickedTile->SetTileOccupied(true);
                NewSoldier->Team = ETeam::Player;
                NewSoldier->InitSprite();
                NewSoldier->TryAssignOwningTile(Tiles);

                if (GameUIInstance)
                {
                    GameUIInstance->AddSoldierSpriteToUI(NewSoldier);
                }

                CurrentUnitIndex++;

                if (CurrentUnitIndex < SpawnQueue.Num())
                {
                    bIsPlayerTurn = false;
                    NextTurn();
                }
                else
                {
                    OnPlacementPhaseComplete();
                }
            }
        }
        return;
    }

    //  Fase di movimento/attacco

    if (!TurnManager)
    {
        ClearAllHighlights();
        return;
    }

    if (!bIsPlayerTurn || !bActionPhaseStarted || !SelectedSoldier)
    {
        // NON cancellare highlight e selezione se non c'è un soldato selezionato
        return;
    }

    if (SelectedSoldier->HasCompletedTurn())
    {
        SelectedSoldier = nullptr; // annulla la selezione
        return;
    }

    //  Movimento
    if (ClickedTile->bIsSelected)
    {
        if (SelectedSoldier->bHasMoved)
        {
            ClearAllHighlights();
            return;
        }

        SelectedSoldier->MoveToTile(ClickedTile, Tiles);
        return;
    }

    //  Attacco
    if (ClickedTile->bIsEnemyHighlighted)
    {
        if (SelectedSoldier->bHasAttacked)
        {
            ClearAllHighlights();
            return;
        }

        ASoldier* Target = ClickedTile->GetOccupyingSoldier();

        if (!Target || Target->Team != ETeam::AI)
        {
            ClearAllHighlights();
            return;
        }

        if (!SelectedSoldier->IsInAttackRange(Target))
        {
            ClearAllHighlights();
            return;
        }

        //  Attacco
        SelectedSoldier->Attack(Target);

        //  Se il nemico è morto → GESTIONE PRIMA DEL TURNO
        if (Target->Health <= 0)
        {
            ClickedTile->SetTileOccupied(false);

            if (GameUIInstance)
            {
                GameUIInstance->RemoveSoldierFromUI(Target);
            }

            if (ABaseGameMode* GM = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
            {
                if (GM->TurnManager)
                {
                    GM->TurnManager->NotifySoldierKilled(Target);
                }
            }

            Target->Destroy();

        }

        if (!SelectedSoldier)
        {
            return;
        }

        //  Usa variabile locale per sicurezza 
        ASoldier* TempSoldier = SelectedSoldier;
        TempSoldier->bHasAttacked = true;

        if (!TempSoldier->bHasMoved)
        {
            TempSoldier->bHasMoved = true;
        }

        if (TempSoldier->bHasMoved && !TempSoldier->bActionReported)
        {
            TempSoldier->bActionReported = true;

            TurnManager->OnSoldierActionCompleted(TempSoldier);
        }

        ClearAllHighlights();
        return;
    }
}
void ABaseGameMode::ResetGame()
{
    //  Distruggi tutti i soldati attivi
    TArray<AActor*> AllSoldiers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASoldier::StaticClass(), AllSoldiers);
    for (AActor* Actor : AllSoldiers)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }



    //  Svuota gli array
    PlayerSoldiers.Empty();
    AISoldiers.Empty();
    SpawnQueue.Empty();

    //  Resetta tutte le tile
    for (ATile* Tile : Tiles)
    {
        if (Tile)
        {
            Tile->ResetTile();
        }
    }

    AObstacles::DestroyAllObstacles(GetWorld());
    AObstacles::RegenerateObstacles(GetWorld());

    //  Reset variabili
    bIsPlayerTurn = true;
    CurrentUnitIndex = 0;
    bActionPhaseStarted = false;
    SelectedSoldier = nullptr;
    SelectedSoldier_Current = nullptr;

    //  Reset TurnManager
    if (TurnManager)
    {
        TurnManager->Init(PlayerSoldiers, AISoldiers);
    }

    //  Reset UI (chiamata pulita)
    if (GameUIInstance)
    {
        GameUIInstance->ResetUI();
    }
}
void ABaseGameMode::ClearAllHighlights()
{
    for (ATile* Tile : Tiles)
    {
        if (Tile)
        {
            Tile->SetSelected(false);
            Tile->SetEnemyHighlighted(false);
            Tile->bIsEnemyHighlighted = false;
        }
    }
}
void ABaseGameMode::ClearSelectedSoldier()
{
    if (SelectedSoldier && SelectedSoldier->OwningTile)
    {
        SelectedSoldier->OwningTile->SetSelected(false);
    }
    SelectedSoldier = nullptr;
}