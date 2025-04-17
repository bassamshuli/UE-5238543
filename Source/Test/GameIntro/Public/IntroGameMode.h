// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "IntroGameMode.generated.h"

UCLASS()
class TEST_API AIntroGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AIntroGameMode(const FObjectInitializer& ObjectInitializer); 
    
    virtual void BeginPlay() override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI") TSubclassOf<class UUserWidget> StartMenuWidgetClass;
};
