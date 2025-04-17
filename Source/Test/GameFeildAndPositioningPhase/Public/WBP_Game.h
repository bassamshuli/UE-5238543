// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "WBP_Game.generated.h"

UCLASS()
class TEST_API UWBP_Game : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual bool Initialize() override;

    UFUNCTION(BlueprintCallable, Category = "UI") void UpdateStatusMessage(const FText& NewMessage);
    UFUNCTION() void StartGameButtonClicked();

    UPROPERTY(meta = (BindWidget)) UTextBlock* StatusText;
    UPROPERTY(meta = (BindWidget)) UButton* StartButton;
    void AddMoveToHistoryUI(const FString& MoveText);

    UPROPERTY(meta = (BindWidget))
    UVerticalBox* HistoryBox;
    UPROPERTY(meta = (BindWidget))
    class UButton* ResetButton;
    UPROPERTY(meta = (BindWidget)) UButton* ButtonChooseBrawler;
    UPROPERTY(meta = (BindWidget)) UButton* ButtonChooseSniper;
    UFUNCTION(BlueprintCallable)
    void ClearMoveHistory();


    UFUNCTION(BlueprintCallable, Category = "UI") void ShowWelcomeMessage();
    UFUNCTION(BlueprintCallable, Category = "UI") void ShowPlacementMessage(bool bIsPlayerTurn, int32 CurrentUnitIndex);
    UFUNCTION(BlueprintCallable, Category = "UI") void ShowChooseUnitTypeUI();
    void HideChooseButtons();
    UFUNCTION(BlueprintCallable)
    void ResetUI();
    UFUNCTION() void OnBrawlerChosen();
    UFUNCTION() void OnSniperChosen();
    UFUNCTION()
    void ResetGameButtonClicked();

    class ABaseGameMode* GameModeRef;

public:
    void SetSpawnQueue(const TArray<TSubclassOf<class ASoldier>>& InQueue);
    UFUNCTION(BlueprintCallable)
    void RemoveSoldierFromUI(class ASoldier* Soldier);

private:
    TArray<TSubclassOf<class ASoldier>> SpawnQueue;

public:
    void AddSoldierSpriteToUI(class ASoldier* Soldier);

    UFUNCTION(BlueprintCallable)
    void EnableStartButton();

    UPROPERTY(meta = (BindWidget))
    class UVerticalBox* SoldierVerticalBox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> SoldierPreviewClass;

    UFUNCTION(BlueprintCallable)
    void ClearSoldierUI();

    UFUNCTION(BlueprintCallable)

    void ShowMessage(const FText& Message);
    void EndTurn(bool bIsPlayerTurn);
    UFUNCTION(BlueprintCallable)
    void UpdateSoldierHealth(ASoldier* Soldier);
};