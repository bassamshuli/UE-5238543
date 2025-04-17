// Fill out your copyright notice in the Description page of Project Settings.


#include "AIStrategyComponent.h"
#include "Soldier.h"
#include "Tile.h"
#include "BaseGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "UTurnManager.h"

UAIStrategyComponent::UAIStrategyComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    CurrentSoldier = nullptr;
}

void UAIStrategyComponent::BeginPlay()
{
    Super::BeginPlay();
    CacheTiles();
}

void UAIStrategyComponent::CacheTiles()
{
    if (CachedTiles.Num() == 0)
    {
        for (TActorIterator<ATile> It(GetWorld()); It; ++It)
        {
            CachedTiles.Add(*It);
        }
    }
}

void UAIStrategyComponent::StartAIAction(ASoldier* AISoldier)
{
    if (!AISoldier) return;

    CurrentSoldier = AISoldier;

    // Wait 2 seconds before movement
    GetWorld()->GetTimerManager().SetTimer(ActionDelayHandle, this, &UAIStrategyComponent::TryMove, 2.0f, false);
}
void UAIStrategyComponent::TryMove()
{
    if (!CurrentSoldier || !CurrentSoldier->OwningTile)
    {
        OnMoveCompleted();
        return;
    }

    // Find nearest enemy
    ASoldier* NearestEnemy = nullptr;
    int32 MinDistance = TNumericLimits<int32>::Max();

    for (TActorIterator<ASoldier> It(GetWorld()); It; ++It)
    {
        ASoldier* Enemy = *It;
        if (Enemy && Enemy->Team != CurrentSoldier->Team && Enemy->Health > 0 && Enemy->OwningTile)
        {
            int32 Dist = FMath::Abs(CurrentSoldier->OwningTile->GridPosition.X - Enemy->OwningTile->GridPosition.X) +
                FMath::Abs(CurrentSoldier->OwningTile->GridPosition.Y - Enemy->OwningTile->GridPosition.Y);

            if (Dist < MinDistance)
            {
                MinDistance = Dist;
                NearestEnemy = Enemy;
            }
        }
    }

    if (!NearestEnemy)
    {
        OnMoveCompleted();
        return;
    }

    // Search path to tiles adjacent to the enemy
    TArray<ATile*> BestPath;
    int32 ShortestLength = TNumericLimits<int32>::Max();
    const FIntPoint EnemyPos = NearestEnemy->OwningTile->GridPosition;

    const TArray<FIntPoint> Offsets = {
        FIntPoint(1, 0), FIntPoint(-1, 0),
        FIntPoint(0, 1), FIntPoint(0, -1)
    };

    for (const FIntPoint& Offset : Offsets)
    {
        FIntPoint AdjacentPos = EnemyPos + Offset;
        ATile* AdjacentTile = nullptr;

        for (ATile* Tile : CachedTiles)
        {
            if (Tile && Tile->GridPosition == AdjacentPos && !Tile->bHasObstacle && !Tile->bIsOccupied)
            {
                AdjacentTile = Tile;
                break;
            }
        }

        if (!AdjacentTile) continue;

        TArray<ATile*> Path = FindPathAStar(CurrentSoldier->OwningTile, AdjacentTile, CachedTiles);

        if (Path.Num() > 0 && Path.Num() < ShortestLength)
        {
            BestPath = Path;
            ShortestLength = Path.Num();
        }
    }

    if (BestPath.Num() > 0)
    {
        if (BestPath.Num() > CurrentSoldier->MaxMovement)
        {
            BestPath.SetNum(CurrentSoldier->MaxMovement);
        }

        CurrentSoldier->MoveAlongPath(BestPath);
        GetWorld()->GetTimerManager().SetTimer(ActionDelayHandle, this, &UAIStrategyComponent::OnMoveCompleted, 0.2f, true);
    }
    else
    {
        OnMoveCompleted();
    }
}
void UAIStrategyComponent::OnMoveCompleted()
{
    if (!CurrentSoldier || CurrentSoldier->bIsMoving) return;

    GetWorld()->GetTimerManager().ClearTimer(ActionDelayHandle);

    GetWorld()->GetTimerManager().SetTimer(ActionDelayHandle, this, &UAIStrategyComponent::TryAttack, 2.0f, false);
}

void UAIStrategyComponent::TryAttack()
{
    if (!CurrentSoldier)
    {
        FinishTurn();
        return;
    }

    TArray<ASoldier*> Enemies = CurrentSoldier->GetEnemiesInRange();

    if (Enemies.Num() > 0)
    {
        ASoldier* Target = Enemies[FMath::RandRange(0, Enemies.Num() - 1)];
        CurrentSoldier->Attack(Target);
        CurrentSoldier->bHasAttacked = true;
    }
    else
    {
        CurrentSoldier->bHasAttacked = true;
    }

    GetWorld()->GetTimerManager().SetTimer(ActionDelayHandle, this, &UAIStrategyComponent::FinishTurn, 2.0f, false);
}

void UAIStrategyComponent::FinishTurn()
{
    if (!IsValid(CurrentSoldier) || CurrentSoldier->IsPendingKillPending())
    {
        return;
    }

    if (ABaseGameMode* GM = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        GM->ClearAllHighlights();

        if (GM->TurnManager)
        {
            if (GM->TurnManager->CheckGameOver()) return;

            GM->TurnManager->OnSoldierActionCompleted(CurrentSoldier);
        }
    }

    CurrentSoldier = nullptr;
}
TArray<ATile*> UAIStrategyComponent::FindPathAStar(ATile* StartTile, ATile* GoalTile, const TArray<ATile*>& AllTiles)
{
    TArray<ATile*> Path;

    if (!StartTile || !GoalTile || StartTile == GoalTile) return Path;

    // Fast map position to tile
    TMap<FIntPoint, ATile*> TileMap;
    for (ATile* Tile : AllTiles)
    {
        TileMap.Add(Tile->GridPosition, Tile);
    }

    // A* Node
    struct FNode
    {
        FIntPoint Position;
        int32 G;
        int32 H;
        FIntPoint Parent;
        int32 F() const { return G + H; }

        FNode(FIntPoint InPos = FIntPoint(), int32 InG = 0, int32 InH = 0, FIntPoint InParent = FIntPoint())
            : Position(InPos), G(InG), H(InH), Parent(InParent) {}
    };

    TMap<FIntPoint, FNode> OpenSet;
    TMap<FIntPoint, FNode> ClosedSet;

    const FIntPoint Start = StartTile->GridPosition;
    const FIntPoint Goal = GoalTile->GridPosition;

    OpenSet.Add(Start, FNode(Start, 0, FMath::Abs(Start.X - Goal.X) + FMath::Abs(Start.Y - Goal.Y), FIntPoint(-1, -1)));

    while (OpenSet.Num() > 0)
    {

        FNode* Current = nullptr;
        for (auto& Elem : OpenSet)
        {
            if (!Current || Elem.Value.F() < Current->F())
                Current = &Elem.Value;
        }

        if (!Current) break;

        FNode Node = *Current;
        OpenSet.Remove(Node.Position);
        ClosedSet.Add(Node.Position, Node);

        // Log each analyzed node
        if (Node.Position == Goal)
        {
            // Build the path
            FIntPoint Pos = Goal;

            while (Pos != Start)
            {
                if (!ClosedSet.Contains(Pos))
                {
                    break;
                }

                if (ATile* Tile = TileMap.FindRef(Pos))
                {
                    Path.Insert(Tile, 0);
                }

                Pos = ClosedSet[Pos].Parent;
            }            return Path;
        }

        const TArray<FIntPoint> Directions = {
            FIntPoint(1, 0), FIntPoint(-1, 0),
            FIntPoint(0, 1), FIntPoint(0, -1)
        };

        for (const FIntPoint& Dir : Directions)
        {
            FIntPoint NextPos = Node.Position + Dir;

            if (ClosedSet.Contains(NextPos)) continue;

            ATile* NextTile = TileMap.FindRef(NextPos);
            bool bIsGoal = (NextPos == Goal);

            if (!NextTile || NextTile->bHasObstacle || (!bIsGoal && NextTile->bIsOccupied)) continue;

            int32 G = Node.G + 1;
            int32 H = FMath::Abs(NextPos.X - Goal.X) + FMath::Abs(NextPos.Y - Goal.Y);

            if (OpenSet.Contains(NextPos))
            {
                if (G < OpenSet[NextPos].G)
                {
                    OpenSet[NextPos].G = G;
                    OpenSet[NextPos].Parent = Node.Position;
                }
            }
            else
            {
                OpenSet.Add(NextPos, FNode(NextPos, G, H, Node.Position));
            }
        }
    }    return Path;
}