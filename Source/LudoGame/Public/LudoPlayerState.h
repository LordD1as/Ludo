// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Events/GameplayMessageSubsystem.h"
#include "Events/LobbyMessage.h"
#include "Events/PlayerUIMessage.h"
#include "LudoPlayerState.generated.h"

#define BLUE      0b00010000
#define RED		   0b00100000
#define GREEN   0b00110000
#define YELLOW 0b01010000

/**
 * 
 */
UCLASS()
class LUDOGAME_API ALudoPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ALudoPlayerState();

	UPROPERTY(ReplicatedUsing=OnRep_IsLeaving, EditAnywhere, BlueprintReadWrite)
	bool bIsLeaving;

	UPROPERTY(Replicated)
	bool bTryToRejoin;

	UPROPERTY(ReplicatedUsing=OnRep_IsReady, EditAnywhere, BlueprintReadWrite)
	bool bIsReady;	

	/* 2 LSB are for player index in PlayerArray on GameState. The rest are for colors. */
	UPROPERTY(ReplicatedUsing = OnRep_LudoPlayerInfo, EditAnywhere, BlueprintReadWrite)
	uint8 LudoPlayerInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString PlayerColorLoc;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerNickname, BlueprintReadWrite)
	FString PlayerNickname;

	/* Id for maths. NOT id in PlayerArray on GameState. */
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	uint8 LudoPlayerID;

	/* Id in PlayerArray of GameState. Represents the order in wich clients has been connected. */
	UPROPERTY(Replicated, VisibleAnywhere)
	uint8 ServerPlayerID;

	UFUNCTION(Server, Reliable)
	void Server_SetPlayerId();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_TrySetReady(bool IsReady);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_TrySetColor(const uint8 InPlayerColor);

	UFUNCTION(Client, Reliable)
	void UpdatePlayerUI(const FPlayerUIMessage& Paylod);

	UFUNCTION()
	void OnRep_PlayerNickname();

	UFUNCTION()
	void OnRep_IsLeaving();

private:
	UFUNCTION()
	void OnRep_IsReady();

	UFUNCTION()
	void OnRep_LudoPlayerInfo();

	UFUNCTION()
	void OnUpdatePlayerUI(const FPlayerUIMessage& InPlayerMessage);

	UFUNCTION()
	void OnUpdateUI(const FLobbyMessage& InLobby);

	UFUNCTION(BlueprintCallable)
	void OnSetReadyFail();

	UFUNCTION(BlueprintCallable)
	void OnColorChooseFail();

#pragma region Pre game state - waiting for players
	/* Remove id from byte and return a player color as string. */
	UFUNCTION(BlueprintPure)
	FString GetPlayerColor(const uint8 InPlayerColorAndID) const;

	/* Returns color converted to uint8. */
	uint8 ConvertColorStringToByte(const FString InColor) const;

	/* Returns index in GameState::PlayerArray. */
	uint8 GetPlayerID(const uint8 InPlayerColorAndID) const;

	/* Checks if client asked for a color. */
	bool IsLocalAsk(const uint8 InServerPlayerID);

	UFUNCTION(BlueprintPure)
	uint8 BuildPlayerColorAndID(const FString InColor);
#pragma endregion
};
