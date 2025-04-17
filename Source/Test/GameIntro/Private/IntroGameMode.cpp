// Fill out your copyright notice in the Description page of Project Settings.


#include "IntroGameMode.h"
#include "PlayerControllerBase.h"
#include "Blueprint/UserWidget.h"


AIntroGameMode::AIntroGameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    DefaultPawnClass = nullptr;

    PlayerControllerClass = APlayerControllerBase::StaticClass();

    static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClass(TEXT("/Game/BP_Widgets/WBP_StartMenu")); // <-- path corretto!
    if (WidgetClass.Succeeded())
    {
        StartMenuWidgetClass = WidgetClass.Class;
    }
}

void AIntroGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (StartMenuWidgetClass)
    {
        UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), StartMenuWidgetClass);
        if (Widget)
        {
            Widget->AddToViewport();
        }
    }
}


