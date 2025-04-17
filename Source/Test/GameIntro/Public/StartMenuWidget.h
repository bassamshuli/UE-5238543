// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StartMenuWidget.generated.h"

UCLASS()
class TEST_API UStartMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual bool Initialize() override;

protected:

    UFUNCTION() void OnStartGameClicked();

public:
    UPROPERTY(meta = (BindWidget)) class UButton* StartButton;
};
