// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LudoGameModeBase.h"
#include "LudoLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class LUDOGAME_API ALudoLobbyGameMode : public ALudoGameModeBase
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

protected:
	virtual bool ReadyToStart() override;
	
};
