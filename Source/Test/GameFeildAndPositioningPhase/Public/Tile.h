// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.generated.h"

class ASoldier;

UCLASS()
class TEST_API ATile : public AActor
{
    GENERATED_BODY()

public:
    ATile();

    UFUNCTION()
    ASoldier* GetOccupyingSoldier() const;
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable) bool IsTileFree() const;
    UFUNCTION(BlueprintCallable) void SetTileOccupied(bool bOccupied);
    UFUNCTION() void OnTileClicked(AActor* TouchedActor, FKey ButtonPressed);
    UFUNCTION() void SetSelected(bool bSelected);
    UFUNCTION(BlueprintCallable, Category = "Grid") FString GetGridLabel() const;
    UFUNCTION()
    void ResetTile();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    FIntPoint GridPosition;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
    bool bIsOccupied;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    TSubclassOf<ASoldier> PlayerSoldierToSpawn;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    TSubclassOf<ASoldier> SoldierClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bHasObstacle = false;

    UPROPERTY(EditAnywhere)
    UMaterialInterface* DefaultMaterial;

    UPROPERTY(EditAnywhere)
    UMaterialInterface* SelectedMaterial;

    UPROPERTY(EditDefaultsOnly)
    UMaterialInterface* EnemyMaterial;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* TileMesh;

    UFUNCTION()
    void SetEnemyHighlighted(bool bHighlight);

    UPROPERTY()
    bool bIsSelected = false;

    UPROPERTY(BlueprintReadWrite)
    bool bIsEnemyHighlighted = false;

private:
    UMaterialInstanceDynamic* TileMaterial;

};