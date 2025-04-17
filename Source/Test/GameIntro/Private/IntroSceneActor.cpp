// Fill out your copyright notice in the Description page of Project Settings.


#include "IntroSceneActor.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"

AIntroSceneActor::AIntroSceneActor()
{
    PrimaryActorTick.bCanEverTick = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    
    Plane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plane"));
    Plane->SetupAttachment(RootComponent);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane"));
    if (PlaneMesh.Succeeded())
    {
        Plane->SetStaticMesh(PlaneMesh.Object);
        Plane->SetRelativeScale3D(FVector(12.0f, 9.0f, 1.0f)); 
        Plane->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f)); 
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> BackgroundMaterial(TEXT("/Game/Materials/M_IntroBackground")); // cambia il path se diverso
    if (BackgroundMaterial.Succeeded())
    {
        Plane->SetMaterial(0, BackgroundMaterial.Object);
    }

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(RootComponent);
    Camera->SetRelativeLocation(FVector(0.0f, 0.0f, 800.0f));      
    Camera->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));     

    
    DirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("DirectionalLight"));
    DirectionalLight->SetupAttachment(RootComponent);
    DirectionalLight->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));
}

void AIntroSceneActor::BeginPlay()
{
    Super::BeginPlay();

    if (Camera)
    {
        Camera->Activate();  
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && Camera)
    {
        PC->SetViewTargetWithBlend(this, 0.0f);
    }
}

