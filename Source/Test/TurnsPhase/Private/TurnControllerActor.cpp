// Fill out your copyright notice in the Description page of Project Settings.


#include "TurnControllerActor.h"
#include "UTurnManager.h"
#include "BaseGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Soldier.h"

ATurnControllerActor::ATurnControllerActor()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATurnControllerActor::BeginPlay()
{
    Super::BeginPlay();

    // Create and register TurnManager
    TurnManager = NewObject<UTurnManager>(this, UTurnManager::StaticClass());
    TurnManager->RegisterComponent();
    // Link GameMode if possible
    if (ABaseGameMode* GM = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        GM->SetTurnManager(TurnManager);
    }
}

void ATurnControllerActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ATurnControllerActor::SetupSoldiers(const TArray<ASoldier*>& InPlayerSoldiers, const TArray<ASoldier*>& InAISoldiers)
{
    PlayerSoldiers = InPlayerSoldiers;
    AISoldiers = InAISoldiers;

    if (!TurnManager)
    {
        TurnManager = NewObject<UTurnManager>(this, UTurnManager::StaticClass());
        TurnManager->RegisterComponent();
    }

    TurnManager->Init(PlayerSoldiers, AISoldiers);

    if (ABaseGameMode* GM = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        GM->SetTurnManager(TurnManager);
    }
}

void ATurnControllerActor::StartGameTurn()
{
    if (TurnManager)
    {
        bool bPlayerStarts = FMath::RandBool(); // Coin flip      
        TurnManager->StartTurn(bPlayerStarts);
    }
}
void ATurnControllerActor::InitializeTurnManager()
{
    if (!TurnManager)
    {
        TurnManager = NewObject<UTurnManager>(this);
        if (TurnManager)
        {
        }
    }
}
