// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "LudoLocalPlayer.generated.h"

/**
 * 
 */
UCLASS()
class LUDOGAME_API ULudoLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()

public:
	virtual void SetControllerId(int32 NewControllerId) override;

	UFUNCTION(BlueprintPure)
	virtual FString GetNickname() const;
	
};
