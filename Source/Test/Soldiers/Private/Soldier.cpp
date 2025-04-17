// Fill out your copyright notice in the Description page of Project Settings.

#include "Soldier.h"
#include "Tile.h"
#include "PaperSpriteComponent.h"
#include "BaseGameMode.h"
#include "GameFeild.h"
#include "SniperSoldier.h"
#include "BrawlerSoldier.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Containers/Queue.h"
#include "Containers/Set.h"

ASoldier::ASoldier()
{
    PrimaryActorTick.bCanEverTick = true;

    SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
    RootComponent = SpriteComponent;

    SpriteComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SpriteComponent->SetCollisionProfileName(TEXT("BlockAll"));
    SpriteComponent->SetGenerateOverlapEvents(true);
    SpriteComponent->SetNotifyRigidBodyCollision(true);

}

void ASoldier::BeginPlay()
{
    Super::BeginPlay();

    OnClicked.AddDynamic(this, &ASoldier::OnSoldierClicked);

    //  Se OwningTile  nullo, prova ad assegnarlo
    if (!OwningTile)
    {
        TArray<ATile*> AllTiles;
        for (TActorIterator<ATile> It(GetWorld()); It; ++It)
        {
            AllTiles.Add(*It);
        }

        TryAssignOwningTile(AllTiles);
    }
}

void ASoldier::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Movimento fluido
    if (bIsMoving)
    {
        FVector CurrentLocation = GetActorLocation();
        FVector NewLocation = FMath::VInterpTo(CurrentLocation, MoveTarget, DeltaTime, MoveSpeed * 0.01f);
        SetActorLocation(NewLocation);

        if (FVector::Dist2D(NewLocation, MoveTarget) < 5.f)
        {
            SetActorLocation(MoveTarget);
            bIsMoving = false;

            CurrentPathIndex++;

            // Ritarda il prossimo step per fluidit
            GetWorld()->GetTimerManager().SetTimer(MovementTimerHandle, this, &ASoldier::MoveStep, 0.05f, false);
        }
    }
}

//  Sezione: Sprite & Inizializzazione 

void ASoldier::InitSprite()
{}

UPaperSprite* ASoldier::GetSprite() const
{
    return SpriteComponent ? SpriteComponent->GetSprite() : nullptr;
}

// Sezione: Selezione & Click 

void ASoldier::OnSoldierClicked(AActor* TouchedActor, FKey ButtonPressed)
{
    if (ABaseGameMode* GameMode = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        if (Team == ETeam::Player)
        {
            GameMode->HandleSoldierSelected(this);
        }
        else if (Team == ETeam::AI && OwningTile)
        {
            GameMode->HandleTileClicked(OwningTile);
        }
    }
}

void ASoldier::TryAssignOwningTile(const TArray<ATile*>& AllTiles)
{
    for (ATile* Tile : AllTiles)
    {
        if (Tile && FVector::Dist2D(Tile->GetActorLocation(), GetActorLocation()) < 10.0f)
        {
            OwningTile = Tile;
            Tile->SetTileOccupied(true);  
            return;
        }
    }
}

//  Sezione: Turno 

void ASoldier::ResetTurnState()
{
    bHasMoved = false;
    bHasAttacked = false;
    bActionReported = false;
}

bool ASoldier::CanPerformAction() const { return !bHasMoved || !bHasAttacked; }

bool ASoldier::HasCompletedTurn() const
{
    return bHasMoved && bHasAttacked;
}

bool ASoldier::HasAvailableTilesToMove() const
{
    if (!OwningTile) return false;

    FIntPoint Start = OwningTile->GridPosition;

    // Mappa delle tile
    TMap<FIntPoint, ATile*> TileMap;
    for (TActorIterator<ATile> It(GetWorld()); It; ++It)
    {
        ATile* Tile = *It;
        if (Tile)
        {
            TileMap.Add(Tile->GridPosition, Tile);
        }
    }

    // trovare almeno una tile libera entro MaxMovement
    TQueue<TTuple<FIntPoint, int32>> Queue;
    TSet<FIntPoint> Visited;

    Queue.Enqueue(TTuple<FIntPoint, int32>(Start, 0));
    Visited.Add(Start);

    while (!Queue.IsEmpty())
    {
        TTuple<FIntPoint, int32> CurrentPair;
        Queue.Dequeue(CurrentPair);

        FIntPoint Pos = CurrentPair.Key;
        int32 Distance = CurrentPair.Value;

        if (Distance > MaxMovement)
            continue;

        ATile* Tile = TileMap.FindRef(Pos);
        if (Tile && Tile != OwningTile && !Tile->bIsOccupied && !Tile->bHasObstacle)
        {
            return true; //  Trovata almeno una tile valida
        }

        const TArray<FIntPoint> Directions = {
            FIntPoint(1, 0), FIntPoint(-1, 0),
            FIntPoint(0, 1), FIntPoint(0, -1)
        };

        for (const FIntPoint& Dir : Directions)
        {
            FIntPoint Next(Pos.X + Dir.X, Pos.Y + Dir.Y);
            if (!Visited.Contains(Next))
            {
                ATile* NextTile = TileMap.FindRef(Next);
                if (NextTile && !NextTile->bHasObstacle && !NextTile->bIsOccupied)
                {
                    Queue.Enqueue(TTuple<FIntPoint, int32>(Next, Distance + 1));
                    Visited.Add(Next);
                }
            }
        }
    }

    return false; //  Nessuna tile libera trovata
}

void ASoldier::PerformTurnAction()
{
    if (HasCompletedTurn()) return;

    if (!bHasMoved && HasAvailableTilesToMove())
    {
        MoveToBestTile();
        bHasMoved = true;
    }

    if (!bHasAttacked && IsEnemyInRange())
    {
        AttackNearestEnemy();
        bHasAttacked = true;
    }
}

void ASoldier::MoveToBestTile() { }

void ASoldier::AttackNearestEnemy() { }

//  Sezione: Movimento 

void ASoldier::MoveToTile(ATile* TargetTile, const TArray<ATile*>& AllTiles)
{
    if (!TargetTile || !OwningTile) return;

    // Trova il path
    MovementPath = FindPathToTile(TargetTile, AllTiles);
    CurrentPathIndex = 0;

    if (MovementPath.Num() > 0)
    {
        if (OwningTile) OwningTile->SetTileOccupied(false);
        MovementPath.Last()->SetTileOccupied(true);

        //  Aggiunge mossa MOVIMENTO allo storico
        if (ABaseGameMode* GM = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
        {
            if (GM->GameUIInstance && OwningTile && MovementPath.Last())
            {
                FString Prefix = Team == ETeam::Player ? TEXT("HP:") : TEXT("AI:");
                FString UnitType = IsA(ASniperSoldier::StaticClass()) ? TEXT("S") : TEXT("B");
                FString From = OwningTile->GetGridLabel();
                FString To = MovementPath.Last()->GetGridLabel();
                FString Entry = FString::Printf(TEXT("%s%s%s->%s"), *Prefix, *UnitType, *From, *To);
                GM->GameUIInstance->AddMoveToHistoryUI(Entry);
            }
        }
    }

    // Avvia movimento a step
    MoveStep();
    bHasMoved = true;
}

void ASoldier::MoveStep()
{
    //  Movimento completato
    if (!MovementPath.IsValidIndex(CurrentPathIndex))
    {
        if (MovementPath.Num() > 0)
        {
            //  Aggiorna OwningTile
            OwningTile = MovementPath.Last();
            if (OwningTile)
            {
                OwningTile->SetTileOccupied(true);
            }
            else
            {
                //  Se OwningTile  nulla, riassegna manualmente
                TArray<ATile*> AllTiles;
                for (TActorIterator<ATile> It(GetWorld()); It; ++It)
                {
                    AllTiles.Add(*It);
                }
                TryAssignOwningTile(AllTiles);

                if (OwningTile)
                {
                    OwningTile->SetTileOccupied(true);
                }
                else
                {
                }
            }

            if (ABaseGameMode* GameMode = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
            {
                GameMode->SelectedSoldier = this;
                GameMode->ClearAllHighlights();

                if (!bHasAttacked)
                {
                    if (!HasEnemiesInRange(GameMode->Tiles))
                    {
                        bHasAttacked = true;

                        if (Team == ETeam::Player && GameMode->TurnManager && !bActionReported)
                        {
                            bActionReported = true;
                            GameMode->TurnManager->OnSoldierActionCompleted(this);
                        }
                    }
                    else
                    {
                        //  Mostra nemici in range
                        if (Team == ETeam::Player)
                        {
                            ShowOnlyEnemyTilesInRange(GameMode->Tiles);
                        }
                    }
                }
                else
                {
                    // attaccato  fine turno
                    GameMode->SelectedSoldier = nullptr;

                    if (GameMode->TurnManager && !bActionReported)
                    {
                        bActionReported = true;
                        GameMode->TurnManager->OnSoldierActionCompleted(this);
                    }
                }
            }
        }

        MovementPath.Empty();
        CurrentPathIndex = 0;
        return;
    }

    //  Prossimo step nel path
    ATile* StepTile = MovementPath[CurrentPathIndex];
    if (StepTile)
    {
        MoveTarget = StepTile->GetActorLocation() + FVector(0, 0, 50); 
        bIsMoving = true;
    }
}

TArray<ATile*> ASoldier::FindPathToTile(ATile* TargetTile, const TArray<ATile*>& AllTiles)
{
    TMap<FIntPoint, ATile*> TileMap;
    for (ATile* Tile : AllTiles) TileMap.Add(Tile->GridPosition, Tile);

    TQueue<FIntPoint> Queue;
    TMap<FIntPoint, FIntPoint> CameFrom;
    TSet<FIntPoint> Visited;

    FIntPoint Start = OwningTile->GridPosition;
    FIntPoint Goal = TargetTile->GridPosition;

    Queue.Enqueue(Start);
    Visited.Add(Start);

    const TArray<FIntPoint> Directions = {
        FIntPoint(1, 0), FIntPoint(-1, 0),
        FIntPoint(0, 1), FIntPoint(0, -1)
    };

    while (!Queue.IsEmpty())
    {
        FIntPoint Current;
        Queue.Dequeue(Current);

        if (Current == Goal) break;

        for (FIntPoint Dir : Directions)
        {
            FIntPoint Next = Current + Dir;
            if (!Visited.Contains(Next))
            {
                ATile* NextTile = TileMap.FindRef(Next);
                if (NextTile && !NextTile->bHasObstacle && !NextTile->bIsOccupied)
                {
                    Queue.Enqueue(Next);
                    Visited.Add(Next);
                    CameFrom.Add(Next, Current);
                }
            }
        }
    }

    TArray<ATile*> Path;
    FIntPoint Current = Goal;

    while (Current != Start)
    {
        ATile* Tile = TileMap.FindRef(Current);
        if (!Tile) break;
        Path.Insert(Tile, 0);
        Current = CameFrom.FindRef(Current);
    }

    return Path;
}

// Sezione: Combattimento 

int32 ASoldier::GetRandomDamage() const { return FMath::RandRange(MinDamage, MaxDamage); }

bool ASoldier::IsEnemyInRange() const
{
    if (!OwningTile) return false;

    FIntPoint MyPos = OwningTile->GridPosition;

    for (TActorIterator<ASoldier> It(GetWorld()); It; ++It)
    {
        ASoldier* Enemy = *It;
        if (!Enemy || Enemy == this || Enemy->Team == Team || !Enemy->OwningTile) continue;

        FIntPoint EnemyPos = Enemy->OwningTile->GridPosition;
        int32 Distance = FMath::Abs(MyPos.X - EnemyPos.X) + FMath::Abs(MyPos.Y - EnemyPos.Y);

        if (Distance <= AttackRange) return true;
    }

    return false;
}

void ASoldier::Attack(ASoldier* TargetEnemy)
{
    if (!TargetEnemy || TargetEnemy->Health <= 0) return;

    int32 Damage = GetRandomDamage();
    TargetEnemy->Health -= Damage;

    ABaseGameMode* GM = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (GM && GM->GameUIInstance)
    {
        GM->GameUIInstance->UpdateSoldierHealth(TargetEnemy);
    }

    //  Aggiungi allo storico la mossa di attacco
    if (GM && GM->GameUIInstance && TargetEnemy->OwningTile)
    {
        FString Prefix = Team == ETeam::Player ? TEXT("HP:") : TEXT("AI:");
        FString UnitType = IsA(ASniperSoldier::StaticClass()) ? TEXT("S") : TEXT("B");
        FString Cell = TargetEnemy->OwningTile->GetGridLabel();
        FString Entry = FString::Printf(TEXT("%s%s%s%d"), *Prefix, *UnitType, *Cell, Damage);
        GM->GameUIInstance->AddMoveToHistoryUI(Entry);
    }

    //  Se il nemico  morto
    if (TargetEnemy->Health <= 0)
    {
        if (TargetEnemy->OwningTile)
        {
            TargetEnemy->OwningTile->SetTileOccupied(false);
            TargetEnemy->OwningTile = nullptr;
        }

        if (GM)
        {
            GM->PlayerSoldiers.Remove(TargetEnemy);
            GM->AISoldiers.Remove(TargetEnemy);
        }

        TargetEnemy->Destroy();

        if (GM && GM->TurnManager)
        {
            GM->TurnManager->NotifySoldierKilled(TargetEnemy);
            if (GM->TurnManager->CheckGameOver()) return;
        }
    }

    //  CONTRATTACCO SE CHI ATTACCA  UNO SNIPER
    if (this->IsA(ASniperSoldier::StaticClass()) && TargetEnemy->Health > 0)
    {
        bool bCounterattack = false;

        if (TargetEnemy->IsA(ASniperSoldier::StaticClass()))
        {
            bCounterattack = true;
        }
        else if (TargetEnemy->IsA(ABrawlerSoldier::StaticClass()) && TargetEnemy->OwningTile && OwningTile)
        {
            int32 Dist = FMath::Abs(TargetEnemy->OwningTile->GridPosition.X - OwningTile->GridPosition.X) +
                FMath::Abs(TargetEnemy->OwningTile->GridPosition.Y - OwningTile->GridPosition.Y);

            if (Dist == 1) bCounterattack = true;
        }

        if (bCounterattack)
        {
            int32 CounterDamage = FMath::RandRange(1, 3);
            Health -= CounterDamage;

            //  Messaggio a schermo
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT(" Danno da contrattacco subito!"));
            }

            if (GM && GM->GameUIInstance)
            {
                GM->GameUIInstance->UpdateSoldierHealth(this);
            }

            if (Health <= 0)
            {
                if (OwningTile)
                {
                    OwningTile->SetTileOccupied(false);
                    OwningTile = nullptr;
                }

                if (GM)
                {
                    GM->PlayerSoldiers.Remove(this);
                    GM->AISoldiers.Remove(this);
                }

                Destroy();

                if (GM && GM->TurnManager)
                {
                    GM->TurnManager->NotifySoldierKilled(this);
                    GM->TurnManager->CheckGameOver();
                }
            }
        }
    }
}

bool ASoldier::IsInAttackRange(ASoldier* TargetEnemy) const
{
    if (!TargetEnemy || !TargetEnemy->OwningTile || !OwningTile) return false;

    int32 Dist = FMath::Abs(OwningTile->GridPosition.X - TargetEnemy->OwningTile->GridPosition.X) +
        FMath::Abs(OwningTile->GridPosition.Y - TargetEnemy->OwningTile->GridPosition.Y);

    return Dist <= AttackRange;
}

TArray<ASoldier*> ASoldier::GetEnemiesInRange() const
{
    TArray<ASoldier*> Enemies;

    if (!OwningTile)
    {
        return Enemies;
    }

    FIntPoint MyPos = OwningTile->GridPosition;

    for (TActorIterator<ASoldier> It(GetWorld()); It; ++It)
    {
        ASoldier* Enemy = *It;
        if (!Enemy || Enemy == this || Enemy->Team == Team) continue;

        if (!Enemy->OwningTile)
        {
            continue;
        }

        int32 Dist = FMath::Abs(MyPos.X - Enemy->OwningTile->GridPosition.X) +
            FMath::Abs(MyPos.Y - Enemy->OwningTile->GridPosition.Y);

        if (Dist <= AttackRange)
        {
            Enemies.Add(Enemy);
        }
    }

    return Enemies;
}

// Sezione: Evidenziazione 

void ASoldier::ShowMovableTiles(const TArray<ATile*>& AllTiles)
{
    if (!OwningTile)
    {
        TryAssignOwningTile(AllTiles);
    }

    if (!OwningTile)
    {
        return;
    }
    FIntPoint Start = OwningTile->GridPosition;

    TMap<FIntPoint, ATile*> TileMap;
    for (ATile* Tile : AllTiles)
    {
        if (Tile)
        {
            TileMap.Add(Tile->GridPosition, Tile);
            Tile->SetSelected(false);
            Tile->SetEnemyHighlighted(false);
            Tile->bIsEnemyHighlighted = false;
        }
    }

    TQueue<TTuple<FIntPoint, int32>> Queue;
    TSet<FIntPoint> Visited;

    Queue.Enqueue(TTuple<FIntPoint, int32>(Start, 0));
    Visited.Add(Start);

    while (!Queue.IsEmpty())
    {
        TTuple<FIntPoint, int32> CurrentPair;
        Queue.Dequeue(CurrentPair);

        FIntPoint Pos = CurrentPair.Key;
        int32 Distance = CurrentPair.Value;

        if (Distance > MaxMovement)
            continue;

        ATile* Tile = TileMap.FindRef(Pos);
        if (Tile && Tile != OwningTile && !Tile->bIsOccupied && !Tile->bHasObstacle)
        {
            Tile->SetSelected(true);
        }

        const TArray<FIntPoint> Directions = {
            FIntPoint(1, 0), FIntPoint(-1, 0),
            FIntPoint(0, 1), FIntPoint(0, -1)
        };

        for (const FIntPoint& Dir : Directions)
        {
            FIntPoint Next(Pos.X + Dir.X, Pos.Y + Dir.Y);
            if (!Visited.Contains(Next))
            {
                ATile* NextTile = TileMap.FindRef(Next);
                if (NextTile && !NextTile->bIsOccupied && !NextTile->bHasObstacle)
                {
                    Queue.Enqueue(TTuple<FIntPoint, int32>(Next, Distance + 1));
                    Visited.Add(Next);
                }
            }
        }
    }

    //  Evidenzia le tile nemiche adiacenti
    for (ATile* Tile : AllTiles)
    {
        if (!Tile || !Tile->bIsOccupied || Tile->bHasObstacle)
            continue;

        int32 GridDistance = FMath::Abs(Tile->GridPosition.X - Start.X) + FMath::Abs(Tile->GridPosition.Y - Start.Y);
        if (GridDistance <= AttackRange)
        {
            for (TActorIterator<ASoldier> It(GetWorld()); It; ++It)
            {
                ASoldier* Enemy = *It;
                if (Enemy->Team != ETeam::Player && Enemy->OwningTile == Tile)
                {
                    Tile->SetEnemyHighlighted(true);
                    Tile->bIsEnemyHighlighted = true;
                    break;
                }
            }
        }
    }
}

void ASoldier::ShowOnlyEnemyTilesInRange(const TArray<ATile*>& AllTiles)
{
    if (!OwningTile) return;

    for (ATile* Tile : AllTiles)
    {
        Tile->SetSelected(false);
        Tile->SetEnemyHighlighted(false);
        Tile->bIsEnemyHighlighted = false;
    }

    FIntPoint Start = OwningTile->GridPosition;

    for (ATile* Tile : AllTiles)
    {
        if (!Tile || !Tile->bIsOccupied || Tile->bHasObstacle) continue;

        int32 Dist = FMath::Abs(Tile->GridPosition.X - Start.X) + FMath::Abs(Tile->GridPosition.Y - Start.Y);
        if (Dist <= AttackRange)
        {
            for (TActorIterator<ASoldier> It(GetWorld()); It; ++It)
            {
                ASoldier* Enemy = *It;
                if (Enemy->Team != ETeam::Player && Enemy->OwningTile == Tile)
                {
                    Tile->SetEnemyHighlighted(true);
                    Tile->bIsEnemyHighlighted = true;
                    break;
                }
            }
        }
    }
}
bool ASoldier::HasEnemiesInRange(const TArray<ATile*>& AllTiles) const
{
    for (ATile* Tile : AllTiles)
    {
        ASoldier* Other = Tile->GetOccupyingSoldier();
        if (Other && Other->Team != Team && IsInAttackRange(Other))
        {
            return true;
        }
    }
    return false;
}
void ASoldier::MoveAlongPath(const TArray<ATile*>& Path)
{
    if (Path.Num() == 0) return;

    MovementPath = Path;
    CurrentPathIndex = 0;

    if (OwningTile) OwningTile->SetTileOccupied(false);
    if (Path.Last()) Path.Last()->SetTileOccupied(true);

    MoveStep();
    if (Path.Num() > 0 && OwningTile && Path.Last())
    {
        if (ABaseGameMode* GM = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
        {
            if (GM->GameUIInstance)
            {
                FString Prefix = Team == ETeam::Player ? TEXT("HP:") : TEXT("AI:");
                FString UnitType = IsA(ASniperSoldier::StaticClass()) ? TEXT("S") : TEXT("B");
                FString From = OwningTile->GetGridLabel();
                FString To = Path.Last()->GetGridLabel();
                FString Entry = FString::Printf(TEXT("%s%s%s->%s"), *Prefix, *UnitType, *From, *To);
                GM->GameUIInstance->AddMoveToHistoryUI(Entry);
            }
        }
    }
    bHasMoved = true;
}
