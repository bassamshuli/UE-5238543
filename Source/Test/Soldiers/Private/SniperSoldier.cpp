// Fill out your copyright notice in the Description page of Project Settings.


#include "SniperSoldier.h"
#include "PaperSprite.h"

ASniperSoldier::ASniperSoldier()
{
    PrimaryActorTick.bCanEverTick = true;
    MaxMovement = 3;
    AttackType = EAttackType::Ranged;
    AttackRange = 10;
    MinDamage = 4;
    MaxDamage = 8;
    Health = 20;

    ConstructorHelpers::FObjectFinder<UPaperSprite> PlayerSpriteObj(TEXT("/Game/Sprites/Soldier1_Green_Sprite"));
    if (PlayerSpriteObj.Succeeded())
    {
        CachedPlayerSprite = PlayerSpriteObj.Object;
    }

    ConstructorHelpers::FObjectFinder<UPaperSprite> AISpriteObj(TEXT("/Game/Sprites/Soldier1_Red_Sprite"));
    if (AISpriteObj.Succeeded())
    {
        CachedAISprite = AISpriteObj.Object;
    }
}

void ASniperSoldier::BeginPlay()
{
    Super::BeginPlay();
    InitSprite();
}

void ASniperSoldier::InitSprite()
{
    if (SpriteComponent)
    {
        SpriteComponent->SetRelativeScale3D(FVector(0.3f));
        SpriteComponent->SetRelativeRotation(FRotator(0.f, 0.0f, -90.0f));

        if (Team == ETeam::Player && CachedPlayerSprite)
        {
            SpriteComponent->SetSprite(CachedPlayerSprite);
        }
        else if (Team == ETeam::AI && CachedAISprite)
        {
            SpriteComponent->SetSprite(CachedAISprite);

        }
    }
}