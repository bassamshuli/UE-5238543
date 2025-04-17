// Fill out your copyright notice in the Description page of Project Settings.


#include "UTurnManager.h"
#include "AIStrategyComponent.h"
#include "Soldier.h"
#include "BaseGameMode.h"
#include "WBP_Game.h"
#include "PlayerControllerBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void UTurnManager::Init(const TArray<ASoldier*>& InPlayerSoldiers, const TArray<ASoldier*>& InAISoldiers)
{
    GameModeRef = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    PlayerSoldiers = InPlayerSoldiers;
    AISoldiers = InAISoldiers;
    CurrentSoldierIndex = 0;
    bTurnEnded = false;
    bGameOver = false; // Reset flag at the beginning of the game
}

void UTurnManager::StartTurn(bool bIsPlayer)
{
    bPlayerTurn = bIsPlayer;
    CurrentSoldierIndex = 0;
    bTurnEnded = false;

    TArray<ASoldier*>& Soldiers = bPlayerTurn ? PlayerSoldiers : AISoldiers;

    if (CheckGameOver()) return;

    for (ASoldier* Soldier : Soldiers)
    {
        if (Soldier && !Soldier->IsPendingKillPending())
        {
            Soldier->ResetTurnState();
        }
    }

    if (GameModeRef)
    {
        GameModeRef->SelectedSoldier = nullptr;

        if (APlayerControllerBase* PC = Cast<APlayerControllerBase>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
        {
            PC->SelectedSoldier = nullptr;
        }
    }

    if (!bPlayerTurn)
    {
        bIsAITakingTurn = true; // Block player input while AI acts

        if (!AIStrategyComponent)
        {
            AIStrategyComponent = NewObject<UAIStrategyComponent>(this);
            AIStrategyComponent->RegisterComponent();
        }

        if (!bPlayerTurn)
        {
            ASoldier* FirstSoldier = GetCurrentSoldier();
            if (FirstSoldier)
            {
                AIStrategyComponent->StartAIAction(FirstSoldier);
            }
        }
    }
    else
    {
        bIsAITakingTurn = false; // Enable player input    }
    }
}
void UTurnManager::OnSoldierActionCompleted(ASoldier* CompletedSoldier)
{
    if (!IsValid(CompletedSoldier) || bTurnEnded || CompletedSoldier->IsPendingKillPending())
    {
        return;
    }

    TArray<ASoldier*>& AllSoldiers = bPlayerTurn ? PlayerSoldiers : AISoldiers;

    TArray<ASoldier*> ValidSoldiers;
    for (ASoldier* S : AllSoldiers)
    {
        if (IsValid(S) && S->Health > 0)
        {
            ValidSoldiers.Add(S);
        }
    }

    if (bPlayerTurn)
    {
        if (!ValidSoldiers.Contains(CompletedSoldier)) return;

        bool bAllCompleted = true;
        for (ASoldier* Soldier : ValidSoldiers)
        {
            if (!Soldier->HasCompletedTurn())
            {
                bAllCompleted = false;
                break;
            }
        }

        if (bAllCompleted)
        {
            EndTurn();
        }
    }
    else
    {
        ASoldier* CurrentSoldier = ValidSoldiers.IsValidIndex(CurrentSoldierIndex) ? ValidSoldiers[CurrentSoldierIndex] : nullptr;
        if (CompletedSoldier != CurrentSoldier) return;

        CurrentSoldierIndex++;
        if (CurrentSoldierIndex >= ValidSoldiers.Num())
        {
            EndTurn();
            return;
        }

        if (AIStrategyComponent)
        {
            ASoldier* NextSoldier = ValidSoldiers[CurrentSoldierIndex];
            GetWorld()->GetTimerManager().SetTimerForNextTick([this, NextSoldier]()
                {
                    AIStrategyComponent->StartAIAction(NextSoldier);
                });
        }
    }
}

void UTurnManager::AdvanceToNextSoldier()
{
    CurrentSoldierIndex++;
    TArray<ASoldier*>& Soldiers = bPlayerTurn ? PlayerSoldiers : AISoldiers;

    if (CurrentSoldierIndex >= Soldiers.Num())
    {
        EndTurn();
        return;
    }

    if (!bPlayerTurn && AIStrategyComponent)
    {
        ASoldier* NextSoldier = GetCurrentSoldier();
        AIStrategyComponent->StartAIAction(NextSoldier);
    }

    CheckIfTurnIsOver();
}

void UTurnManager::EndTurn()
{
    bTurnEnded = true;
    bPlayerTurn = !bPlayerTurn;
    CurrentSoldierIndex = 0;

    if (GameModeRef && GameModeRef->GameUIInstance)
    {
        GameModeRef->GameUIInstance->EndTurn(bPlayerTurn);
    }

    GetWorld()->GetTimerManager().SetTimerForNextTick([this]() {
        StartTurn(bPlayerTurn);
        });
}

ASoldier* UTurnManager::GetCurrentSoldier() const
{
    const TArray<ASoldier*>& Soldiers = bPlayerTurn ? PlayerSoldiers : AISoldiers;
    return Soldiers.IsValidIndex(CurrentSoldierIndex) ? Soldiers[CurrentSoldierIndex] : nullptr;
}

void UTurnManager::CheckIfTurnIsOver()
{
    const TArray<ASoldier*>& Soldiers = bPlayerTurn ? PlayerSoldiers : AISoldiers;
    for (ASoldier* Soldier : Soldiers)
    {
        if (Soldier && !Soldier->HasCompletedTurn())
        {
            return;
        }
    }
    EndTurn();
}

void UTurnManager::NotifySoldierKilled(ASoldier* DeadSoldier)
{
    if (!DeadSoldier) return;

    TArray<ASoldier*>& Soldiers = (DeadSoldier->Team == ETeam::Player) ? PlayerSoldiers : AISoldiers;
    int32 Index = Soldiers.IndexOfByKey(DeadSoldier);

    if (Index != INDEX_NONE)
    {
        Soldiers.RemoveAt(Index);

        if (GameModeRef && GameModeRef->GameUIInstance)
        {
            GameModeRef->GameUIInstance->RemoveSoldierFromUI(DeadSoldier);
        }

        if (Index < CurrentSoldierIndex)
        {
            CurrentSoldierIndex--;
        }
        else if (Index == CurrentSoldierIndex && !bTurnEnded)
        {
            CurrentSoldierIndex++;

            // Only if AI's turn, decide to end turn or act with next
            if (!bPlayerTurn)
            {
                if (CurrentSoldierIndex >= Soldiers.Num())
                {
                    if (!CheckGameOver()) EndTurn();
                    return;
                }

                if (AIStrategyComponent)
                {
                    ASoldier* NextSoldier = GetCurrentSoldier();
                    if (NextSoldier)
                    {
                        GetWorld()->GetTimerManager().SetTimerForNextTick([this, NextSoldier]() {
                            AIStrategyComponent->StartAIAction(NextSoldier);
                            });
                    }
                }
            }
        }

        CheckGameOver();
    }
}

bool UTurnManager::CheckGameOver()
{
    if (PlayerSoldiers.Num() == 0)
    {
        bGameOver = true;

        if (GameModeRef && GameModeRef->GameUIInstance)
        {
            GameModeRef->GameUIInstance->UpdateStatusMessage(
                FText::FromString(TEXT("AI has won! Click Reset to play again."))
            );
            GameModeRef->GameUIInstance->ClearSoldierUI();
            GameModeRef->GameUIInstance->ClearMoveHistory();
        }

        return true;
    }

    if (AISoldiers.Num() == 0)
    {
        bGameOver = true;

        if (GameModeRef && GameModeRef->GameUIInstance)
        {
            GameModeRef->GameUIInstance->UpdateStatusMessage(
                FText::FromString(TEXT("Player has won! Click Reset to play again."))
            );
            GameModeRef->GameUIInstance->ClearSoldierUI();
            GameModeRef->GameUIInstance->ClearMoveHistory();
        }

        return true;
    }

    return false;
}

