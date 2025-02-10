// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LudoGameStateBase.h"
#include "LudoLobbyGameState.generated.h"

/**
 * 
 */
UCLASS()
class LUDOGAME_API ALudoLobbyGameState : public ALudoGameStateBase
{
	GENERATED_BODY()

public:
#pragma region Pre game state - waiting for players
	/* Checks if any player has choosen this color.
	* @InColor: Color to choose.
	* @Returns true if somebody has the same color.
	*/
	UFUNCTION(BlueprintPure)
	bool HasAnybodySameColor(FString InColor);
#pragma endregion

protected:
		virtual void HandleMatchIsWaitingForPlayers() override;
		virtual void HandleMatchHasStarted() override;
};
