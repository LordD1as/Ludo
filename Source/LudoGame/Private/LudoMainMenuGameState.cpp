// Fill out your copyright notice in the Description page of Project Settings.


#include "LudoMainMenuGameState.h"

void ALudoMainMenuGameState::HandleMatchIsWaitingForPlayers()
{
	Super::HandleMatchIsWaitingForPlayers();

	if (ULudoGameInstance* LudoGameInstance = GetGameInstance<ULudoGameInstance>())
	{
		LudoGameInstance->OnShowWidget(FWidgetState(EWidgetState::MainMenuScreen));
	}
}
