// Fill out your copyright notice in the Description page of Project Settings.

#include "Tile.h"
#include "Kismet/GameplayStatics.h"
#include "BaseGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"

ATile::ATile()
{
    PrimaryActorTick.bCanEverTick = false;

    // Carica i materiali
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMat(TEXT("/Game/Materials/M_Tile_Default"));
    if (DefaultMat.Succeeded()) DefaultMaterial = DefaultMat.Object;

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> SelectedMat(TEXT("/Game/Materials/M_Tile_Selected"));
    if (SelectedMat.Succeeded()) SelectedMaterial = SelectedMat.Object;

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> EnemyMat(TEXT("/Game/Materials/M_Tile_Enemy"));
    if (EnemyMat.Succeeded()) EnemyMaterial = EnemyMat.Object;

    TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
    RootComponent = TileMesh;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        TileMesh->SetStaticMesh(CubeMesh.Object);
        TileMesh->SetRelativeScale3D(FVector(3.0f, 3.0f, 0.1f));
    }

    bIsOccupied = false;
    bHasObstacle = false;
}

void ATile::BeginPlay()
{
    Super::BeginPlay();
    OnClicked.AddUniqueDynamic(this, &ATile::OnTileClicked);

    if (TileMesh && DefaultMaterial)
    {
        TileMesh->SetMaterial(0, DefaultMaterial);
    }
}

void ATile::OnTileClicked(AActor* TouchedActor, FKey ButtonPressed)
{
    if (bIsOccupied || bHasObstacle) return;

    ABaseGameMode* GameMode = Cast<ABaseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (GameMode && GameMode->bIsPlayerTurn)
    {
        GameMode->HandleTileClicked(this);
    }
}

bool ATile::IsTileFree() const
{
    return !bIsOccupied && !bHasObstacle;
}

void ATile::SetTileOccupied(bool bOccupied)
{
    bIsOccupied = bOccupied;
}
ASoldier* ATile::GetOccupyingSoldier() const
{
    for (TActorIterator<ASoldier> It(GetWorld()); It; ++It)
    {
        ASoldier* Soldier = *It;
        if (Soldier && Soldier->OwningTile == this)
        {
            return Soldier;
        }
    }
    return nullptr;
}

void ATile::SetSelected(bool bSelected)
{
    bIsSelected = bSelected;

    if (TileMesh && SelectedMaterial && DefaultMaterial)
    {
        TileMesh->SetMaterial(0, bSelected ? SelectedMaterial : DefaultMaterial);

        if (bSelected)
        {
        }
    }
}

void ATile::SetEnemyHighlighted(bool bHighlight)
{
    if (TileMesh && EnemyMaterial && DefaultMaterial)
    {
        TileMesh->SetMaterial(0, bHighlight ? EnemyMaterial : DefaultMaterial);
    }
}
FString ATile::GetGridLabel() const
{
    TCHAR ColumnChar = static_cast<TCHAR>('A' + GridPosition.X);
    int32 RowNumber = GridPosition.Y + 1;
    return FString::Printf(TEXT("%c%d"), ColumnChar, RowNumber);
}
void ATile::ResetTile()
{
    SetTileOccupied(false);
    bHasObstacle = false;
    SetSelected(false);
    SetEnemyHighlighted(false);
    bIsEnemyHighlighted = false;
}