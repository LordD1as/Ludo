// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LobbyRowMessage.generated.h"

USTRUCT(BlueprintType)
struct FLobbyRowMessage
{
	GENERATED_BODY()

	FLobbyRowMessage() {};
	
	FLobbyRowMessage(FString InPlayerName, FString InPlayerColor) : PlayerName(InPlayerName), PlayerColor(InPlayerColor) {};	

	FLobbyRowMessage(int32 InPing) : Ping(InPing) {};

	FLobbyRowMessage(bool bInIsReady) : bIsReady(bInIsReady) {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString PlayerColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Ping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsReady;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsLocallyControlled;
};