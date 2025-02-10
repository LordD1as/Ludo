// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LudoWidget.h"
#include "ErrorMsg.generated.h"

/**
 * 
 */
UCLASS()
class LUDOGAME_API UErrorMsg : public ULudoWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void OnShowMsg(const FString& Msg);
};
