// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaperSpriteComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

#include "PaperSprite.h"
#include "Obstacles.generated.h"
class AGameFeild;
UCLASS()
class TEST_API AObstacles : public AActor
{
    GENERATED_BODY()

public:
    AObstacles();
    void InitWithSprite(const FString& SpritePath);

    UPROPERTY(VisibleAnywhere)
    UPaperSpriteComponent* SpriteComponent;

    UFUNCTION(BlueprintCallable, Category = "Obstacle")
    static void DestroyAllObstacles(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Obstacle")
    static void RegenerateObstacles(UWorld* World);
};