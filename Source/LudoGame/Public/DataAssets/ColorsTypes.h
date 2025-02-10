// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "../LudoGameState.h"
#include "ColorsTypes.generated.h"

/**
 * 
 */
UCLASS()
class LUDOGAME_API UColorsTypes : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	TMap<EColors, FVector2D> LudoColors;
};
