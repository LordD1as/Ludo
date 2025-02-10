// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LudoPlayerState.h"
#include "LudoGameMode.generated.h"

class AFigure;

const FName Rejoin = FName(TEXT("Rejoin"));

//enum ELudoGameState {
//	WaitingForPlayers,
//	GameLoop,
//	GamePause,
//	GameEnding
//};

/**
 * 
 */
UCLASS()
class LUDOGAME_API ALudoGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ALudoGameMode();

	UPROPERTY()
	TArray<class APlayerController*> PlayerControllerList;	

	UPROPERTY(EditAnywhere)
	TArray<ACameraActor*> CamerasArray;

	UPROPERTY()
	TArray<FUniqueNetIdRepl> LogoutPlayers;

	UPROPERTY()
	TArray<APlayerController*> LogoutPCs;

	UPROPERTY()
	TArray<ALudoPlayerState*> LogoutPlayerState;

	UPROPERTY()
	TArray<FString> LogoutPlayersColors;

	UFUNCTION()
	void SetLudoCamera();

	void TryStartGame();

	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	void RunEndTimer();

	void NotifyOnReady(class ALudoPlayerController* const PC);

	UFUNCTION()
	/* Notify player that game has been started. */
	void NotifyPCGameHasStarted(APlayerController* PC);

	void HasBeenNotifiedOnPlayerRejoined(APlayerController* const PC);

	FTimerHandle TimerHandleDisconnectPlayer;
	FTimerHandle TimerHandleRejoinPlayer;

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	virtual void HandleMatchHasStarted() override;

	void SpawnPlayers();

	bool ReadyToStart();

private:
	UFUNCTION()
	void UpdateLobbyUI();

	void TryToRejoin();

	UFUNCTION()
	void EndGame();

	void SetLudoPlayerID(APlayerController* PC);

	UFUNCTION()
	void SetCamerasTMapLocations();

	UPROPERTY()
	TMap<FString, FTransform> CamerasLocations;

	UPROPERTY()
	ACameraActor* Camera;
};
