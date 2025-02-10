// Fill out your copyright notice in the Description page of Project Settings.


#include "LudoLobbyGameMode.h"
#include "Net/OnlineEngineInterface.h"

const uint8 MaxPlayerNicknameLen = 10;

void ALudoLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	UWorld* World = GetWorld();

	// save network address for re-associating with reconnecting player, after stripping out port number
	FString Address = NewPlayer->GetPlayerNetworkAddress();
	int32 pos = Address.Find(TEXT(":"), ESearchCase::CaseSensitive);
	NewPlayer->PlayerState->SavedNetworkAddress = (pos > 0) ? Address.Left(pos) : Address;

	// check if this player is reconnecting and already has PlayerState
	FindInactivePlayer(NewPlayer);

	if (ALudoPlayerController* LudoPlayerController = Cast<ALudoPlayerController>(NewPlayer))
	{
		if (ALudoPlayerState* LudoPlayerState = LudoPlayerController->GetPlayerState<ALudoPlayerState>())
		{
			LudoPlayerState->ServerPlayerID = PlayerControllerList.AddUnique(NewPlayer);
			LudoPlayerState->PlayerNickname = UOnlineEngineInterface::Get()->GetPlayerNickname(World, LudoPlayerState->GetUniqueId()).Left(MaxPlayerNicknameLen);
			LudoPlayerState->OnRep_PlayerNickname();
		}
	}

	Super::PostLogin(NewPlayer);
}

void ALudoLobbyGameMode::Logout(AController* Exiting)
{
	APlayerController* PC = Cast<APlayerController>(Exiting);
	if (PC != nullptr)
	{
		if (ALudoPlayerState* LudoPlayerState = Cast<ALudoPlayerState>(PC->PlayerState))
		{
			LudoPlayerState->bIsLeaving = true;
			LudoPlayerState->OnRep_IsLeaving();
		}
	}

	Super::Logout(Exiting);
}

bool ALudoLobbyGameMode::ReadyToStart()
{
	//Ready to start for at least 2 people
	if (PlayerControllerList.Num() < 2)
	{
		return false;
	}

	if (LudoMatchState.Compare(LudoMatchState::WaitingForPlayers))
	{
		for (size_t i = 0; i < PlayerControllerList.Num(); i++)
		{
			if (ALudoPlayerState* LudoPlayerState = PlayerControllerList[i]->GetPlayerState<ALudoPlayerState>())
			{
				if (!LudoPlayerState->bIsReady)
				{
					return false;
				}
			}
		}

		return true;
	}
	
	return false;
}
