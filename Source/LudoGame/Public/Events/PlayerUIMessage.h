// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PlayerUIMessage.generated.h"

USTRUCT(BlueprintType)
struct FPlayerUIMessage
{
	GENERATED_BODY()

	FPlayerUIMessage() {};

	FPlayerUIMessage(bool bInIsOnMove) : bIsOnMove(bInIsOnMove), NumberOnDice(0), TextInfo(0) {};

	FPlayerUIMessage(uint8 InNumberOnDice) : bIsOnMove(false), NumberOnDice(InNumberOnDice), TextInfo(0) {};

	FPlayerUIMessage(uint8 InNumberOnDice, uint8 InTextInfo) : bIsOnMove(false), NumberOnDice(InNumberOnDice), TextInfo(InTextInfo) {};

	FPlayerUIMessage(bool bInIsOnMove, uint8 InNumberOnDice, uint8 InTextInfo) : bIsOnMove(bInIsOnMove), NumberOnDice(InNumberOnDice), TextInfo(InTextInfo) {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOnMove;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 NumberOnDice;

	/* 
	0 = OnFirstEnter.
	255 = Can not move any figure and player is still on a move.
	254 = Another player is on a move.	
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 TextInfo;
};
