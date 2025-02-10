// Fill out your copyright notice in the Description page of Project Settings.

#include "LudoPlayerState.h"
#include "NativeGameplayTags.h"
#include "LudoGameInstance.h"
#include "LudoGameMode.h"
#include "LudoLobbyGameState.h"
#include "LudoLocalPlayer.h"
#include "LudoPlayerController.h"
#include "Net/UnrealNetwork.h"
#include  "Events/PlayerUIMessage.h"

#define COLOR_BM 0b11111100
#define ID_BM			 0b00000011

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UpdateLobbyWidget, "Event.UpdateLobbyUI");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UpdatePlayerUIWidget, "Event.UpdatePlayerUI");

void ALudoPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALudoPlayerState, bIsLeaving);
	DOREPLIFETIME(ALudoPlayerState, bTryToRejoin);
	DOREPLIFETIME(ALudoPlayerState, bIsReady);
	DOREPLIFETIME(ALudoPlayerState, LudoPlayerInfo);
	DOREPLIFETIME(ALudoPlayerState, PlayerNickname);
	DOREPLIFETIME(ALudoPlayerState, LudoPlayerID);
	DOREPLIFETIME_CONDITION(ALudoPlayerState, ServerPlayerID, COND_OwnerOnly);
}

ALudoPlayerState::ALudoPlayerState()
{
	bReplicates = true;
}

void ALudoPlayerState::Server_SetPlayerId_Implementation()
{
	if (PlayerColorLoc.Contains(ColorBlue))
	{
		LudoPlayerID = 0;
	}
	else if (PlayerColorLoc.Contains(ColorRed))
	{
		LudoPlayerID = 1;
	}
	else  if (PlayerColorLoc.Contains(ColorGreen))
	{
		LudoPlayerID = 2;
	}
	else if (PlayerColorLoc.Contains(ColorYellow))
	{
		LudoPlayerID = 3;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Try set player id. Invalid player color!"));
	}
}

void ALudoPlayerState::OnRep_PlayerNickname()
{
	OnUpdateUI(FLobbyMessage());
	
	UE_LOG(LogTemp, Warning, TEXT("The player id is: %d"), ServerPlayerID);
}

void ALudoPlayerState::OnRep_IsLeaving()
{
	OnUpdateUI(FLobbyMessage());
}

void ALudoPlayerState::OnRep_IsReady()
{
	OnUpdateUI(FLobbyMessage());
}

void ALudoPlayerState::OnRep_LudoPlayerInfo()
{	
	bool bIsColorSet = (LudoPlayerInfo & COLOR_BM) > 0;
	if (bIsColorSet)
	{
		PlayerColorLoc = GetPlayerColor(LudoPlayerInfo);		
	}

	OnUpdateUI(FLobbyMessage());
}

void ALudoPlayerState::OnUpdatePlayerUI(const FPlayerUIMessage& InPlayerMessage)
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(TAG_UpdatePlayerUIWidget, InPlayerMessage);
}

void ALudoPlayerState::OnUpdateUI(const FLobbyMessage& InLobby)
{
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSystem.BroadcastMessage(TAG_UpdateLobbyWidget, InLobby);	
}

FString ALudoPlayerState::GetPlayerColor(const uint8 InPlayerColorAndID) const
{
	//remove id
	uint8 PlayerColorNoID = InPlayerColorAndID & COLOR_BM;
	if ((PlayerColorNoID & BLUE) == BLUE)
	{
		return ColorBlue;
	}
	else if ((PlayerColorNoID & RED) == RED)
	{
		return ColorRed;
	}
	else if ((PlayerColorNoID & GREEN) == GREEN)
	{
		return ColorGreen;
	}
	else if ((PlayerColorNoID & YELLOW) == YELLOW)
	{
		return ColorYellow;
	}

	return FString();
}

uint8 ALudoPlayerState::ConvertColorStringToByte(const FString InColor) const
{
	if (InColor.Contains(ColorBlue))
	{
		return BLUE;
	}
	else if (InColor.Contains(ColorRed))
	{
		return RED;
	}
	else  if (InColor.Contains(ColorGreen))
	{
		return GREEN;
	}
	else if (InColor.Contains(ColorYellow))
	{
		return YELLOW;
	}

	return 0;
}

uint8 ALudoPlayerState::GetPlayerID(const uint8 InPlayerColorAndID) const
{
	return InPlayerColorAndID & ID_BM;
}

bool ALudoPlayerState::IsLocalAsk(const uint8 InServerPlayerID)
{
	return InServerPlayerID == ServerPlayerID;
}

uint8 ALudoPlayerState::BuildPlayerColorAndID(const FString InColor)
{
	return ConvertColorStringToByte(InColor) | ServerPlayerID;
}

void ALudoPlayerState::UpdatePlayerUI_Implementation(const FPlayerUIMessage& Paylod)
{
	OnUpdatePlayerUI(Paylod);
}

void ALudoPlayerState::Server_TrySetColor_Implementation(const uint8 InPlayerColor)
{
	if (ALudoLobbyGameState* LudoGameState = GetWorld()->GetGameState<ALudoLobbyGameState>())
	{
		if (LudoGameState->HasAnybodySameColor(GetPlayerColor(InPlayerColor)))
		{
			//Return just player id
			LudoPlayerInfo = GetPlayerID(InPlayerColor);
			OnRep_LudoPlayerInfo();
			return;
		}

		LudoPlayerInfo = InPlayerColor;
		OnRep_LudoPlayerInfo();
	}
}

void ALudoPlayerState::Server_TrySetReady_Implementation(bool IsReady)
{	
	bIsReady = IsReady;
	OnRep_IsReady();

	//Ask game mode to start match
	if (ALudoGameMode* LudoGameMode = GetWorld()->GetAuthGameMode<ALudoGameMode>())
	{
		LudoGameMode->TryStartGame();
	}
}

void ALudoPlayerState::OnSetReadyFail()
{
	if (ULudoGameInstance* LudoGameInstance = GetGameInstance<ULudoGameInstance>())
	{
		LudoGameInstance->OnShowWidget(FWidgetState(EWidgetState::ErrorMsg, TEXT("The color has not been choosen yet!")));
	}
}

void ALudoPlayerState::OnColorChooseFail()
{
	if (ULudoGameInstance* LudoGameInstance = GetGameInstance<ULudoGameInstance>())
	{
		LudoGameInstance->OnShowWidget(FWidgetState(EWidgetState::ErrorMsg, TEXT("The color has been already choosen!")));
	}
}