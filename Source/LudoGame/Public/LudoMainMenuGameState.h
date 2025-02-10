// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LudoGameStateBase.h"
#include "LudoMainMenuGameState.generated.h"

/**
 * 
 */
UCLASS()
class LUDOGAME_API ALudoMainMenuGameState : public ALudoGameStateBase
{
	GENERATED_BODY()

protected:
		virtual void HandleMatchIsWaitingForPlayers() override;
	
};
