// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LudoPlayerController.h"
#include "LudoPlayerState.h"
#include "LudoGameModeBase.generated.h"

class AGameSession;

namespace LudoMatchState
{
	const FName EnteringMap;
	const FName WaitingForPlayers;
	const FName GameLoop;
	const FName GamePause;
	const FName EndGame;
	const FName Aborted;
}

/**
 * 
 */
UCLASS()
class LUDOGAME_API ALudoGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALudoGameModeBase();

	/** Returns the current match state, this is an accessor to protect the state machine flow */	
	FName GetLudoMatchState() const { return LudoMatchState; }

	void StartLudoMatch();

	void EndLudoMatch();

	void AbortMatch();

protected:
	UPROPERTY()
	TArray<class APlayerController*> PlayerControllerList;

	/** Time a playerstate will stick around in an inactive state after a player logout */
	UPROPERTY()
	float InactivePlayerStateLifeSpan;

	/** The maximum number of inactive players before we kick the oldest ones out */
	UPROPERTY()
	int32 MaxInactivePlayers;

	/** What match state we are currently in */
	UPROPERTY(Transient)
	FName LudoMatchState;

	/** Updates the match state and calls the appropriate transition functions */
	void SetLudoMatchState(FName NewState);	

	void OnMatchStateSet();

	void HandleMatchIsWaitingForPlayers();
	void HandleMatchHasStarted();
	void HandleMatchHasPaused();
	void HandleMatchHasEnded();
	void HandleMatchAborted();

	virtual bool ReadyToStart();

public:
	virtual void Tick(float DeltaSeconds) override;

	/** PlayerStates of players who have disconnected from the server (saved in case they reconnect) */
	UPROPERTY()
	TArray<class APlayerState*> InactivePlayerArray;

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;

	virtual bool HasMatchStarted() const override;

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/** Override PC's PlayerState with the values in OldPlayerState as part of the inactive player handling */
	void OverridePlayerState(APlayerController* PC, APlayerState* OldPlayerState);

	/** Add PlayerState to the inactive list, remove from the active list */
	void AddInactivePlayer(APlayerState* PlayerState, APlayerController* PC);

	/** Attempt to find and associate an inactive PlayerState with entering PC.
	  * @Returns true if a PlayerState was found and associated with PC. */
	bool FindInactivePlayer(APlayerController* PC);
};
