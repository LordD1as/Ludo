// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "WidgetToShowMessage.generated.h"

UENUM(BlueprintType)
enum class EWidgetState : uint8
{
	None,
	MainMenuScreen,
	ServerListScreen,
	LobbyScreen,
	PlayerUIScreen,
	ErrorMsg
};

USTRUCT(BlueprintType)
struct FWidgetState
{
	GENERATED_BODY()

	FWidgetState() {};

	FWidgetState(EWidgetState InWidgetToShow) : WidgetToShow(InWidgetToShow) {};

	FWidgetState(EWidgetState InWidgetToShow, FString InErrorMsg) : WidgetToShow(InWidgetToShow), ErrorMsg(InErrorMsg){};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWidgetState WidgetToShow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ErrorMsg;
};
