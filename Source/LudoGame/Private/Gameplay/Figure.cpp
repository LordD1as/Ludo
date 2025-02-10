// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Figure.h"



// Sets default values
AFigure::AFigure()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'")).Object;

	bReplicates = true;	
}

void AFigure::SetFigureInfo(const uint8 index, const FFigureInfo NewFigureInfo)
{
	uint8 FieldType = (uint8)NewFigureInfo.FieldType;
	uint8 FieldIndex = NewFigureInfo.FigureIndexOnField;
	ISMComp->PerInstanceSMCustomData[index * ISMComp->NumCustomDataFloats] = FieldType;
	ISMComp->PerInstanceSMCustomData[index * ISMComp->NumCustomDataFloats + 1] = FieldIndex;
}

void AFigure::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	//ISMComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//ISMComp->SetCollisionResponseToChannel(COLLISION_FIGURE, ECollisionResponse::ECR_Block);
}

void AFigure::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ISMComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ISMComp->SetCollisionResponseToChannel(COLLISION_FIGURE, ECollisionResponse::ECR_Block);
}

void AFigure::Multicast_SetFigurePosition_Implementation(FVector NewLocation)
{
	SetActorLocation(NewLocation);
}
