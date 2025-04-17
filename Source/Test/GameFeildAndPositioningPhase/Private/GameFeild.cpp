// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFeild.h"
#include "Tile.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Mountain.h"
#include "Tree.h"
#include "EngineUtils.h"

AGameFeild::AGameFeild()
{
    PrimaryActorTick.bCanEverTick = false;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    AutoReceiveInput = EAutoReceiveInput::Player0;

    static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClassFinder(TEXT("/Game/BP_Widgets/WBP_Game"));
    if (WidgetClassFinder.Succeeded()) GameWidgetClass = WidgetClassFinder.Class;

    MountainBlueprint = AMountain::StaticClass();
    TreeBlueprint = ATree::StaticClass();
    ObstacleToSpawn = TreeBlueprint;
}

void AGameFeild::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        // Spawn della camera con visuale isometrica
        FActorSpawnParameters SpawnParams;
        FVector CameraLocation = FVector(-1500.f, -1000.f, 8000.f);
        FRotator CameraRotation = FRotator(-80.f, 0.f, 0.f); 

        RuntimeCameraActor = GetWorld()->SpawnActor<ACameraActor>(
            ACameraActor::StaticClass(),
            CameraLocation,
            CameraRotation,
            SpawnParams
        );

        if (RuntimeCameraActor)
        {
            UCameraComponent* Cam = RuntimeCameraActor->GetCameraComponent();
            Cam->ProjectionMode = ECameraProjectionMode::Perspective;
            Cam->FieldOfView = 90.f;
            Cam->AspectRatio = 1.777778f;
            Cam->bConstrainAspectRatio = true;

            PC->SetViewTargetWithBlend(RuntimeCameraActor, 0.f);
        }

        // Attiva input mouse
        PC->bEnableClickEvents = true;
        PC->bEnableMouseOverEvents = true;
        PC->bShowMouseCursor = true;
    }

    if (bAutoGenerateGrid && !bGridGenerated)
    {
        GenerateGrid();
        GenerateObstacles();
        bGridGenerated = true;
    }

    if (GameWidgetClass)
    {
        GameUIInstance = Cast<UWBP_Game>(CreateWidget<UUserWidget>(GetWorld(), GameWidgetClass));
        if (GameUIInstance)
        {
            GameUIInstance->AddToViewport();
            GameUIInstance->ShowWelcomeMessage();
        }
    }


}

void AGameFeild::GenerateGrid()
{
    UWorld* World = GetWorld();
    if (!World) return;

    Tiles.Empty();
    float Spacing = 5.0f;
    float AdjustedCellSize = CellSize + Spacing;
    float StartX = -((Columns - 1) * AdjustedCellSize) / 2.0f;
    float StartY = -((Rows - 1) * AdjustedCellSize) / 2.0f;

    for (int32 Row = 0; Row < Rows; ++Row)
    {
        for (int32 Col = 0; Col < Columns; ++Col)
        {
            FVector Location(StartX + Col * AdjustedCellSize, StartY + Row * AdjustedCellSize, 0);
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;

            ATile* SpawnedTile = World->SpawnActor<ATile>(ATile::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
            if (SpawnedTile)
            {
                //  Prima assegna GridPosition
                SpawnedTile->GridPosition = FIntPoint(Col, Row);

                //  Assegna nome leggibile al Tile
                FString Label = SpawnedTile->GetGridLabel();
                FString TileName = FString::Printf(TEXT("Tile_%s"), *Label);
                SpawnedTile->Rename(*TileName);

                Tiles.Add(SpawnedTile);
            }

        }
    }
}

void AGameFeild::GenerateObstacles()
{
    if (!MountainBlueprint || !TreeBlueprint || Tiles.Num() == 0) return;

    UWorld* World = GetWorld();
    int32 NumObstacles = FMath::RoundToInt(Rows * Columns * 0.1f);
    TSet<int32> UsedIndices;

    while (UsedIndices.Num() < NumObstacles)
    {
        int32 Index = FMath::RandRange(0, Tiles.Num() - 1);
        if (!UsedIndices.Contains(Index))
        {
            UsedIndices.Add(Index);
            if (!Tiles.IsValidIndex(Index) || !Tiles[Index]) continue;

            FVector Location = Tiles[Index]->GetActorLocation() + FVector(0, 0, 10);
            bool bIsMountain = FMath::RandBool();
            TSubclassOf<AObstacles> Obstacle = bIsMountain ? MountainBlueprint : TreeBlueprint;

            AObstacles* SpawnedObstacle = nullptr;

            if (Obstacle)
            {
                SpawnedObstacle = World->SpawnActor<AObstacles>(Obstacle, Location, FRotator::ZeroRotator);
            }

            if (SpawnedObstacle)
            {
                Tiles[Index]->bHasObstacle = true;
                Tiles[Index]->SetTileOccupied(true);
            }

        }
    }
}

void AGameFeild::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}