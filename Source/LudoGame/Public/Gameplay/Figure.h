// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameElement.h"
#include "Figure.generated.h"

#define COLLISION_FIGURE ECC_GameTraceChannel1

UCLASS()
class LUDOGAME_API AFigure : public AGameElement
{
	GENERATED_BODY()
	
public:	
	// FieldIndex uint8_MAX means that figure is in hangar
	AFigure();

	UFUNCTION(reliable, NetMulticast)
	void Multicast_SetFigurePosition(FVector NewLocation);

	void SetFigureInfo(const uint8 index, const FFigureInfo NewFigureInfo);

protected:
	virtual void OnConstruction(const FTransform& Transform) override;	

	virtual void PostInitializeComponents() override;
};
