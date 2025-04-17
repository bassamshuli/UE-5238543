// Fill out your copyright notice in the Description page of Project Settings.


#include "Tree.h"

ATree::ATree()
{
    InitWithSprite(TEXT("/Game/Sprites/Tree_Sprite"));
    SpriteComponent->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
}