// Fill out your copyright notice in the Description page of Project Settings.

#include "WBP_Game.h"
#include "Kismet/GameplayStatics.h"
#include "BaseGameMode.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "PaperSprite.h"
#include "PaperSpriteComponent.h"
#include "Soldier.h"
#include "GameFeild.h"

bool UWBP_Game::Initialize()
{
    const bool Success = Super::Initialize();
    if (!Success) return false;

    if (StartButton)
    {
        StartButton->OnClicked.AddUniqueDynamic(this, &UWBP_Game::StartGameButtonClicked);
        StartButton->SetVisibility(ESlateVisibility::Visible);
    }

    if (ResetButton)
    {
        ResetButton->OnClicked.AddUniqueDynamic(this, &UWBP_Game::ResetGameButtonClicked);
        ResetButton->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (ButtonChooseBrawler)
    {
        ButtonChooseBrawler->OnClicked.AddDynamic(this, &UWBP_Game::OnBrawlerChosen);
    }

    if (ButtonChooseSniper)
    {
        ButtonChooseSniper->OnClicked.AddDynamic(this, &UWBP_Game::OnSniperChosen);
    }

    GameModeRef = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(this));

    return true;
}

void UWBP_Game::StartGameButtonClicked()
{
    if (GameModeRef)
    {
        GameModeRef->StartGame();
    }

    if (StartButton)
        StartButton->SetVisibility(ESlateVisibility::Collapsed);

    if (ResetButton)
        ResetButton->SetVisibility(ESlateVisibility::Visible);
}

void UWBP_Game::ResetGameButtonClicked()
{
    if (GameModeRef)
    {
        GameModeRef->ResetGame();
    }

    if (StartButton)
        StartButton->SetVisibility(ESlateVisibility::Visible);

    if (ResetButton)
        ResetButton->SetVisibility(ESlateVisibility::Collapsed);
}

void UWBP_Game::ShowWelcomeMessage()
{
    UpdateStatusMessage(FText::FromString(TEXT(" Benvenuto! Premi Start per iniziare")));
}

void UWBP_Game::ShowPlacementMessage(bool bIsPlayerTurn, int32 CurrentUnitIndex)
{
    FString Message;
    FString UnitName;
    bool bIsBrawler = false;

    if (SpawnQueue.IsValidIndex(CurrentUnitIndex) && SpawnQueue[CurrentUnitIndex] != nullptr)
    {
        UnitName = SpawnQueue[CurrentUnitIndex]->GetName();
        bIsBrawler = UnitName.Contains("Brawler");
    }
    else
    {
        UnitName = TEXT("Unknown");
    }

    if (bIsPlayerTurn)
        Message = FString::Printf(TEXT(" Player turn - Posiziona il tuo %s"), bIsBrawler ? TEXT("BRAWLER") : TEXT("SNIPER"));
    else
        Message = FString::Printf(TEXT(" AI turn - Posiziona il suo %s"), bIsBrawler ? TEXT("BRAWLER") : TEXT("SNIPER"));

    UpdateStatusMessage(FText::FromString(Message));
}

void UWBP_Game::ShowChooseUnitTypeUI()
{
    UpdateStatusMessage(FText::FromString(TEXT(" Scegli quale tipo vuoi posizionare per primo")));

    if (ButtonChooseBrawler)
    {
        ButtonChooseBrawler->SetVisibility(ESlateVisibility::Visible);
    }

    if (ButtonChooseSniper)
    {
        ButtonChooseSniper->SetVisibility(ESlateVisibility::Visible);
    }
}

void UWBP_Game::HideChooseButtons()
{
    if (ButtonChooseBrawler)
        ButtonChooseBrawler->SetVisibility(ESlateVisibility::Collapsed);

    if (ButtonChooseSniper)
        ButtonChooseSniper->SetVisibility(ESlateVisibility::Collapsed);
}

void UWBP_Game::OnBrawlerChosen()
{
    HideChooseButtons();
    if (GameModeRef)
        GameModeRef->PlayerChoseStartingUnit(true);
}

void UWBP_Game::OnSniperChosen()
{
    HideChooseButtons();
    if (GameModeRef)
        GameModeRef->PlayerChoseStartingUnit(false);
}

void UWBP_Game::UpdateStatusMessage(const FText& NewMessage)
{
    if (StatusText)
    {
        StatusText->SetText(NewMessage);
    }
}

void UWBP_Game::SetSpawnQueue(const TArray<TSubclassOf<class ASoldier>>& InQueue)
{
    SpawnQueue = InQueue;
}

void UWBP_Game::AddSoldierSpriteToUI(ASoldier* Soldier)
{
    if (!Soldier || !SoldierPreviewClass || !SoldierVerticalBox) return;

    // Crea il widget dinamicamente
    UUserWidget* PreviewWidget = CreateWidget<UUserWidget>(this, SoldierPreviewClass);
    if (!PreviewWidget) return;

    PreviewWidget->Rename(*Soldier->GetName());

    //  Imposta il nome del widget uguale al nome del soldato
    PreviewWidget->Rename(*Soldier->GetName());

    // Ottieni lo sprite dal soldato
    UPaperSprite* Sprite = Soldier->GetSprite();
    if (!Sprite) return;

    // Imposta immagine e testo nel widget
    UImage* Image = Cast<UImage>(PreviewWidget->GetWidgetFromName(TEXT("SoldierImage")));
    if (Image)
    {
        FSlateBrush Brush;
        Brush.SetResourceObject(Sprite);
        Brush.ImageSize = FVector2D(64.f, 64.f);
        Image->SetBrush(Brush);
    }

    UTextBlock* HealthText = Cast<UTextBlock>(PreviewWidget->GetWidgetFromName(TEXT("HealthText")));
    if (HealthText)
    {
        FString HealthStr = FString::Printf(TEXT("%d HP"), Soldier->Health);
        HealthText->SetText(FText::FromString(HealthStr));
    }

    UVerticalBoxSlot* NewSlot = Cast<UVerticalBoxSlot>(SoldierVerticalBox->AddChildToVerticalBox(PreviewWidget));
    if (NewSlot)
    {
        NewSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 100.f));
    }
}
void UWBP_Game::ClearSoldierUI()
{
    if (SoldierVerticalBox)
    {
        SoldierVerticalBox->ClearChildren();
    }
}
void UWBP_Game::EndTurn(bool bIsPlayerTurn)
{
    FString Message = bIsPlayerTurn ? TEXT(" Player turn") : TEXT(" AI turn");
    UpdateStatusMessage(FText::FromString(Message));
}
void UWBP_Game::UpdateSoldierHealth(ASoldier* Soldier)
{
    if (!Soldier || !SoldierVerticalBox) return;

    const FString SoldierName = Soldier->GetName();

    for (UWidget* Child : SoldierVerticalBox->GetAllChildren())
    {
        UUserWidget* Widget = Cast<UUserWidget>(Child);
        if (!Widget) continue;

        //  Confronta il nome del widget con il nome del soldato
        if (Widget->GetFName().ToString() == SoldierName)
        {
            UTextBlock* HealthText = Cast<UTextBlock>(Widget->GetWidgetFromName(TEXT("HealthText")));
            if (HealthText)
            {
                FString NewText = FString::Printf(TEXT("%d HP"), Soldier->Health);
                HealthText->SetText(FText::FromString(NewText));
            }
            return;
        }
    }
}
void UWBP_Game::RemoveSoldierFromUI(ASoldier* Soldier)
{
    if (!Soldier || !SoldierVerticalBox) return;

    const FString SoldierName = Soldier->GetName();

    for (int32 i = 0; i < SoldierVerticalBox->GetChildrenCount(); ++i)
    {
        UWidget* Child = SoldierVerticalBox->GetChildAt(i);
        UUserWidget* Widget = Cast<UUserWidget>(Child);
        if (!Widget) continue;

        //  Confronta il FName col nome del soldato
        if (Widget->GetFName().ToString() == SoldierName)
        {
            SoldierVerticalBox->RemoveChildAt(i);         
            return;
        }
    }
}
void UWBP_Game::EnableStartButton()
{
    if (StartButton)
    {
        StartButton->SetVisibility(ESlateVisibility::Visible);
        StartButton->SetIsEnabled(true);
    }
}
void UWBP_Game::ShowMessage(const FText& Message)
{
    if (StatusText)
    {
        StatusText->SetText(Message);
        StatusText->SetVisibility(ESlateVisibility::Visible);
    }
}
void UWBP_Game::AddMoveToHistoryUI(const FString& MoveText)
{
    if (!HistoryBox) return;

    //  Se la lista ha gi 15 righe, rimuovi la prima
    if (HistoryBox->GetChildrenCount() >= 15)
    {
        HistoryBox->RemoveChildAt(0);
    }

    //  Crea nuova TextBlock per la mossa
    UTextBlock* NewMoveText = NewObject<UTextBlock>(this, UTextBlock::StaticClass());
    if (NewMoveText)
    {
        NewMoveText->SetText(FText::FromString(MoveText));
        NewMoveText->Font.Size = 18;

        HistoryBox->AddChildToVerticalBox(NewMoveText);
    }
}
void UWBP_Game::ClearMoveHistory()
{
    if (HistoryBox)
    {
        HistoryBox->ClearChildren();
    }
}
void UWBP_Game::ResetUI()
{
    ClearSoldierUI();
    ClearMoveHistory();
    ShowWelcomeMessage();
    HideChooseButtons();
    EnableStartButton();
    SetSpawnQueue({}); 
}