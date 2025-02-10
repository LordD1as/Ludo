// Fill out your copyright notice in the Description page of Project Settings.


#include "LudoGameMode.h"
#include "UI/WidgetManager.h"
#include "LudoGameInstance.h"
#include "LudoGameState.h"
#include "LudoLocalPlayer.h"
#include "LudoPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"
#include "Gameplay/Figure.h"

const FName HangarTag = TEXT("Hangar");
const float FigureZPos = 100.0f;

ALudoGameMode::ALudoGameMode()
{
	HUDClass = AWidgetManager::StaticClass();	
}

void ALudoGameMode::SetLudoCamera()
{
	/*for (size_t i = 0; i < LogoutPCs.Num(); i++)
	{
		if (ALudoPlayerController* LudoPC = Cast<ALudoPlayerController>(LogoutPCs[i]))
		{
			if (ALudoPlayerState* PlayerStateLoc = LudoPC->GetPlayerState<ALudoPlayerState>())
			{
				ensureMsgf(!PlayerStateLoc->PlayerColor.IsEmpty(), TEXT("The player color has not been loaded."));
				LudoPC->SetCameraTransform(*CamerasLocations.Find(PlayerStateLoc->PlayerColor));
			}
		}
	}*/
}
void ALudoGameMode::TryStartGame()
{
	if (ReadyToStart())
	{
		SpawnPlayers();
	}
}

APlayerController* ALudoGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	APlayerController* PC = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);


	if (LogoutPlayers.Num() > 0)
	{
		bool bAllReady = true;
		for (size_t i = 0; i < LogoutPlayers.Num(); i++)
		{
			FString MapName = GetWorld()->GetMapName();
			MapName = GetWorld()->RemovePIEPrefix(MapName);
			const FUniqueNetId* LogoutPlayerId = LogoutPlayers[i].GetUniqueNetId().Get();			
			const FUniqueNetId* LoginPlayerId = UniqueId.GetUniqueNetId().Get();
			const FString LogoutPlayerName = LogoutPlayerId->ToString();
			const FString LoginPlayerName = LoginPlayerId->ToString();

			const bool bIsSamePlayer = LogoutPlayerName.Contains(LoginPlayerName);
			if (MapName.Contains(TEXT("Lobby")) && bIsSamePlayer)
			{	
				if (ALudoPlayerState* LudoPlayerState = Cast<ALudoPlayerState>(InactivePlayerArray[i]))
				{
					LudoPlayerState->bTryToRejoin = true;
					PC->PlayerState = LudoPlayerState;
					TimerHandleDisconnectPlayer.Invalidate();
				}				
			}
		}

		if (bAllReady) {
			SetMatchState(Rejoin);
		}

		LogoutPlayers.Reset();
	}

	InactivePlayerArray.Reset();
	return PC;
}

void ALudoGameMode::RunEndTimer()
{
	TimerHandleDisconnectPlayer.Invalidate();
	GetWorldTimerManager().SetTimer(TimerHandleDisconnectPlayer, this, &ThisClass::EndGame, 60.0f);
}

void ALudoGameMode::NotifyOnReady(ALudoPlayerController* const PC)
{
	if (ALudoPlayerState* LudoPlayerState = PC->GetPlayerState<ALudoPlayerState>())
	{
		/*if (!LudoPlayerState->PlayerColor.IsEmpty())
		{
			PC->SetCameraTransform(*CamerasLocations.Find(LudoPlayerState->PlayerColor));
		}*/
	}
}

void ALudoGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer && GetMatchState() != Rejoin)
	{
		if (ALudoPlayerState* LudoPlayerState = NewPlayer->GetPlayerState<ALudoPlayerState>())
		{
			LudoPlayerState->ServerPlayerID = PlayerControllerList.AddUnique(NewPlayer);
			//LudoPlayerState->Client_SetPlayerNickname();

			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(TimerHandle, this, &ThisClass::UpdateLobbyUI, 1.0f);
		}
	}


	//TryToRejoin();
}

void ALudoGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (APlayerController* PC = Cast<APlayerController>(Exiting))
	{		
		PlayerControllerList.Remove(PC);		

		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ThisClass::UpdateLobbyUI, 1.0f);
	}
}

void ALudoGameMode::HandleMatchHasStarted()
{
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ThisClass::SetCamerasTMapLocations, 1.0f);
}

void ALudoGameMode::SpawnPlayers()
{		
	for (APlayerController* PC : PlayerControllerList)
	{
		if (ensureMsgf(CamerasLocations.Num() > 0, TEXT("CamerasLocations' TMap is Empty!")))
		{
			//Local change
			NotifyPCGameHasStarted(PC);
		}

		/* -- - Execute only on server-- - */
		SetLudoPlayerID(PC);
	}

	//Spawn figures
	if (ALudoGameState* LudoGameState = GetGameState<ALudoGameState>())
	{
		LudoGameState->OnStartLudoGame();
	}
}

void ALudoGameMode::NotifyPCGameHasStarted(APlayerController* PC)
{
	if (ALudoPlayerState* LudoPlayerState = PC->GetPlayerState<ALudoPlayerState>())
	{
		if (ALudoPlayerController* LudoPC = Cast<ALudoPlayerController>(PC))
		{
			//LudoPC->ClientGameStarted(*CamerasLocations.Find(LudoPlayerState->PlayerColor));
			SetMatchState(MatchState::InProgress);
		}
	}
}

void ALudoGameMode::HasBeenNotifiedOnPlayerRejoined(APlayerController* const PC)
{
	LogoutPCs.Remove(PC);
	TimerHandleRejoinPlayer.Invalidate();
	if (ALudoGameState* ServerLudoGameState = Cast<ALudoGameState>(UGameplayStatics::GetGameState(this)))
	{
		ServerLudoGameState->Server_SetPauseState(false);
	}
}

void ALudoGameMode::SetLudoPlayerID(APlayerController* PC)
{
	if (ALudoPlayerState* LudoPlayerState = PC->GetPlayerState<ALudoPlayerState>())
	{
		LudoPlayerState->Server_SetPlayerId();
	}	
}

// Move to data table
void ALudoGameMode::SetCamerasTMapLocations()
{		
	CamerasLocations.Add("Blue", FTransform(FRotator(-60.0f, 0.0f, 0.0f), FVector(-1125.0f, 0.0f, 1500.0f), FVector::One()));
	CamerasLocations.Add("Red", FTransform(FRotator(-60.0f, 90.0f, 0.0f), FVector(0.0f, -1125.0f, 1500.0f), FVector::One()));
	CamerasLocations.Add("Green", FTransform(FRotator(-60.0f, 180.0f, 0.0f), FVector(1125.0f, 0.0f, 1500.0f), FVector::One()));
	CamerasLocations.Add("Yellow", FTransform(FRotator(-60.0f, -90.0f, 0.0f), FVector(0.0f, 1125.0f, 1500.0f), FVector::One()));	
}

bool ALudoGameMode::ReadyToStart()
{	
	if (ALudoGameState* LudoGameState = GetGameState<ALudoGameState>())
	{
		return LudoGameState->AreAllReady();
	}
	return false;
}

void ALudoGameMode::UpdateLobbyUI()
{
	ALudoGameState* LudoGameState = GetGameState<ALudoGameState>();
	if (LudoGameState)
	{
		LudoGameState->OnUpdateLobbyUI();
	}
}

void ALudoGameMode::TryToRejoin()
{
	if (GetMatchState() == Rejoin)
	{
		TimerHandleRejoinPlayer.Invalidate();
		GetWorldTimerManager().SetTimer(TimerHandleRejoinPlayer, this, &ThisClass::SetLudoCamera, 1.0f, true, 0.0f);
	}
}

void ALudoGameMode::EndGame()
{
	if (ULudoGameInstance* LudoGameInstance = Cast<ULudoGameInstance>(GetGameInstance()))
	{
		LudoGameInstance->OnLeaveSession();
	}
}
