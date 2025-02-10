// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/FieldTile.h"

// Sets default values
AFieldTile::AFieldTile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'")).Object;
}
