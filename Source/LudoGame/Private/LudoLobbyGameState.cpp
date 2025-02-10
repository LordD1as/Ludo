// Fill out your copyright notice in the Description page of Project Settings.


#include "LudoLobbyGameState.h"

void ALudoLobbyGameState::HandleMatchIsWaitingForPlayers()
{
	Super::HandleMatchIsWaitingForPlayers();

	if (ULudoGameInstance* LudoGameInstance = GetGameInstance<ULudoGameInstance>())
	{
		LudoGameInstance->OnShowWidget(FWidgetState(EWidgetState::LobbyScreen));
	}
}

void ALudoLobbyGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	UE_LOG(LogTemp, Warning, TEXT("Start the game!"));
}

bool ALudoLobbyGameState::HasAnybodySameColor(FString InColor)
{
	if (InColor.IsEmpty())
	{
		return false;
	}

	for (size_t i = 0; i < PlayerArray.Num(); i++)
	{
		if (ALudoPlayerState* LudoPlayer = Cast<ALudoPlayerState>(PlayerArray[i]))
		{
			if (LudoPlayer->PlayerColorLoc.Contains(InColor))
			{
				return true;
			}
		}
	}

	return false;
}