// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LudoWidget.generated.h"

/**
 * 
 */
UCLASS()
class LUDOGAME_API ULudoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void OnWidgetShown();
	
};
