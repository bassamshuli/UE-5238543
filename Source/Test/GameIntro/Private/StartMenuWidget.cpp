// Fill out your copyright notice in the Description page of Project Settings.


#include "StartMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"


bool UStartMenuWidget::Initialize()
{
    bool Success = Super::Initialize();
    if (!Success) return false;

    if (StartButton)
    {
        StartButton->OnClicked.AddDynamic(this, &UStartMenuWidget::OnStartGameClicked);
    }

    return true;
}

void UStartMenuWidget::OnStartGameClicked()
{
    UGameplayStatics::OpenLevel(this, "BaseMap"); 
}

