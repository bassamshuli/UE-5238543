// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IntroSceneActor.generated.h"

UCLASS()
class TEST_API AIntroSceneActor : public AActor
{
    GENERATED_BODY()

public:
    AIntroSceneActor();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere) class UStaticMeshComponent* Plane;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true")) class UCameraComponent* Camera;
    UPROPERTY(VisibleAnywhere)class UDirectionalLightComponent* DirectionalLight;
    UPROPERTY()ACameraActor* SpawnedCameraActor;
};
