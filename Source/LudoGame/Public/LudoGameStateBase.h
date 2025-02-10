// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LudoGameInstance.h"
#include "LudoPlayerState.h"
#include "LudoGameStateBase.generated.h"

#define ColorBlue TEXT("Blue")
#define ColorRed TEXT("Red")
#define ColorGreen TEXT("Green")
#define ColorYellow TEXT("Yellow")

/**
 * 
 */
UCLASS()
class LUDOGAME_API ALudoGameStateBase : public AGameStateBase
{
	GENERATED_BODY()	
public:

	//set after player choose player mode 4 or 6
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint8 MaxPlayers = 4;

	/** Updates the match state and calls the appropriate transition functions, only valid on server */
	void SetLudoMatchState(FName NewState);

	/** Match state has changed */
	UFUNCTION()
	virtual void OnRep_LudoMatchState();
protected:

	/** What match state we are currently in */
	UPROPERTY(ReplicatedUsing = OnRep_LudoMatchState)
	FName LudoMatchState;

	virtual void HandleMatchIsWaitingForPlayers();
	virtual void HandleMatchHasStarted();
	void HandleMatchHasPaused();
	void HandleMatchHasEnded();
};
