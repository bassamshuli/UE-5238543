// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PaperSpriteComponent.h"
#include "Soldier.generated.h"

class ATile;
class ASniperSoldier;
class ABrawlerSoldier;
class UPaperSprite;

UENUM(BlueprintType)
enum class EAttackType : uint8
{
    Melee,
    Ranged
};

UENUM(BlueprintType)
enum class ETeam : uint8
{
    Player,
    AI
};

UCLASS()
class TEST_API ASoldier : public APawn
{
    GENERATED_BODY()

public:
    ASoldier();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core
    virtual void InitSprite();
    void TryAssignOwningTile(const TArray<ATile*>& AllTiles);
    void ShowMovableTiles(const TArray<ATile*>& AllTiles);
    void ShowOnlyEnemyTilesInRange(const TArray<ATile*>& AllTiles);
    void MoveAlongPath(const TArray<ATile*>& Path);
    // Sprite
    UFUNCTION(BlueprintCallable)
    UPaperSprite* GetSprite() const;

    // Combat
    UFUNCTION(BlueprintCallable)
    int32 GetRandomDamage() const;

    UFUNCTION()
    void Attack(ASoldier* TargetEnemy);

    UFUNCTION()
    void AttackNearestEnemy();

    UFUNCTION()
    void MoveToTile(ATile* TargetTile, const TArray<ATile*>& AllTiles);

    UFUNCTION()
    void MoveToBestTile();

    bool IsInAttackRange(ASoldier* TargetEnemy) const;
    TArray<ASoldier*> GetEnemiesInRange() const;

    UFUNCTION()
    bool HasEnemiesInRange(const TArray<ATile*>& AllTiles) const;

    // Turn logic
    void PerformTurnAction();
    void ResetTurnState();
    bool HasCompletedTurn() const;
    bool CanPerformAction() const;
    bool HasAvailableTilesToMove() const;
    bool IsEnemyInRange() const;
    bool bActionReported = false;

    // Pathfinding
    TArray<ATile*> FindPathToTile(ATile* TargetTile, const TArray<ATile*>& AllTiles);
    void MoveStep();

    // Input
    UFUNCTION()
    void OnSoldierClicked(AActor* TouchedActor, FKey ButtonPressed);

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UPaperSpriteComponent* SpriteComponent;

    // Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") int32 MaxMovement;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") EAttackType AttackType;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") int32 AttackRange;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") int32 MinDamage;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") int32 MaxDamage;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats") int32 Health;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) ETeam Team;

    // State
    UPROPERTY() ATile* OwningTile;
    UPROPERTY() bool bHasMoved = false;
    UPROPERTY() bool bHasAttacked = false;
    UPROPERTY() bool bIsMoving = false;

    // Movement
    FTimerHandle MovementTimerHandle;
    TArray<ATile*> MovementPath;
    int32 CurrentPathIndex = 0;
    FVector MoveTarget;
    float MoveSpeed = 1000.f;

};