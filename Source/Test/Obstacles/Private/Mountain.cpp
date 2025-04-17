// Fill out your copyright notice in the Description page of Project Settings.


#include "Mountain.h"

AMountain::AMountain()
{
    InitWithSprite(TEXT("/Game/Sprites/Mountain_Sprite"));
    SpriteComponent->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.15f));
}