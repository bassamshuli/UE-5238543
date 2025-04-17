// Fill out your copyright notice in the Description page of Project Settings.


#include "BrawlerSoldier.h"
#include "PaperSprite.h"

ABrawlerSoldier::ABrawlerSoldier()
{
    PrimaryActorTick.bCanEverTick = true;
    MaxMovement = 6;
    AttackType = EAttackType::Melee;
    AttackRange = 1;
    MinDamage = 1;
    MaxDamage = 6;
    Health = 40;

    ConstructorHelpers::FObjectFinder<UPaperSprite> PlayerSpriteObj(TEXT("/Game/Sprites/Soldier2_Green_Sprite"));
    if (PlayerSpriteObj.Succeeded())
    {
        CachedPlayerSprite = PlayerSpriteObj.Object;
    }

    ConstructorHelpers::FObjectFinder<UPaperSprite> AISpriteObj(TEXT("/Game/Sprites/Soldier2_Red_Sprite"));
    if (AISpriteObj.Succeeded())
    {
        CachedAISprite = AISpriteObj.Object;
    }
}

void ABrawlerSoldier::BeginPlay()
{
    Super::BeginPlay();
    InitSprite();
}

void ABrawlerSoldier::InitSprite()
{
    if (SpriteComponent)
    {
        SpriteComponent->SetRelativeScale3D(FVector(0.4f));
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