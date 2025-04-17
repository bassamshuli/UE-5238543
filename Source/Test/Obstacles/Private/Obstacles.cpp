// Fill out your copyright notice in the Description page of Project Settings.


#include "Obstacles.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "GameFeild.h"
#include "UObject/ConstructorHelpers.h"

AObstacles::AObstacles()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
    SpriteComponent->SetRelativeRotation(FRotator(0.f, 0.f, -90.f));
    SpriteComponent->SetupAttachment(RootComponent);
    SpriteComponent->SetCollisionProfileName(TEXT("BlockAll"));
}

void AObstacles::InitWithSprite(const FString& SpritePath)
{
    ConstructorHelpers::FObjectFinder<UPaperSprite> SpriteAsset(*SpritePath);
    if (SpriteAsset.Succeeded())
    {
        SpriteComponent->SetSprite(SpriteAsset.Object);
    }
}
void AObstacles::DestroyAllObstacles(UWorld* World)
{
    TArray<AActor*> AllObstacles;
    UGameplayStatics::GetAllActorsOfClass(World, AObstacles::StaticClass(), AllObstacles);

    for (AActor* Obstacle : AllObstacles)
    {
        if (IsValid(Obstacle))
        {
            Obstacle->Destroy();
        }
    }
}
void AObstacles::RegenerateObstacles(UWorld* World)
{
    for (TActorIterator<AGameFeild> It(World); It; ++It)
    {
        It->GenerateObstacles();
        break;
    }
}