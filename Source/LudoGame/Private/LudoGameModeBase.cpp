// Fill out your copyright notice in the Description page of Project Settings.


#include "LudoGameModeBase.h"
#include "LudoGameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameSession.h"
#include "Kismet/GameplayStatics.h"
#include "UI/WidgetManager.h"

namespace MatchState
{
	const FName EnteringMap			= FName(TEXT("EnteringMap"));
	const FName WaitingForPlayers = FName(TEXT("WaitingForPlayers"));
	const FName GameLoop				= FName(TEXT("GameLoop"));
	const FName GamePause			= FName(TEXT("GamePause"));
	const FName EndGame				= FName(TEXT("EndGame"));
	const FName Aborted					= FName(TEXT("Aborted"));
}

ALudoGameModeBase::ALudoGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	LudoMatchState = LudoMatchState::EnteringMap;
	InactivePlayerStateLifeSpan = 300.f;
	MaxInactivePlayers = 16;

	HUDClass = AWidgetManager::StaticClass();
}

void ALudoGameModeBase::StartLudoMatch()
{
	if (HasMatchStarted())
	{
		// Already started
		return;
	}

	//Let the game session override the StartMatch function, in case it wants to wait for arbitration
	if (GameSession->HandleStartMatchRequest())
	{
		return;
	}

	SetLudoMatchState(LudoMatchState::GameLoop);
}

void ALudoGameModeBase::EndLudoMatch()
{
	
}

void ALudoGameModeBase::AbortMatch()
{
	SetLudoMatchState(LudoMatchState::Aborted);
}

void ALudoGameModeBase::SetLudoMatchState(FName NewState)
{
	if (LudoMatchState == NewState)
	{
		return;
	}

	UE_LOG(LogTemp, Display, TEXT("Match State Changed from %s to %s"), *LudoMatchState.ToString(), *NewState.ToString());

	LudoMatchState = NewState;

	ALudoGameStateBase* FullGameState = GetGameState<ALudoGameStateBase>();
	if (FullGameState)
	{
		FullGameState->SetLudoMatchState(NewState);
	}
}

void ALudoGameModeBase::OnMatchStateSet()
{
	if (LudoMatchState == LudoMatchState::WaitingForPlayers)
	{
		HandleMatchIsWaitingForPlayers();
	}
	else if (LudoMatchState == LudoMatchState::GameLoop)
	{
		HandleMatchHasStarted();
	}
	else if (LudoMatchState == LudoMatchState::GamePause)
	{
		HandleMatchHasPaused();
	}
	else if (LudoMatchState == LudoMatchState::EndGame)
	{
		HandleMatchHasEnded();
	}
	else if (LudoMatchState == LudoMatchState::Aborted)
	{
		HandleMatchAborted();
	}
}

void ALudoGameModeBase::HandleMatchIsWaitingForPlayers()
{
	if (GameSession != nullptr)
	{
		GameSession->HandleMatchIsWaitingToStart();
	}

	GetWorldSettings()->NotifyBeginPlay();
}

void ALudoGameModeBase::HandleMatchHasStarted()
{
	GameSession->HandleMatchHasStarted();

	// Make sure level streaming is up to date before triggering NotifyMatchStarted
	GEngine->BlockTillLevelStreamingCompleted(GetWorld());

	// First fire BeginPlay, if we haven't already in waiting to start match
	GetWorldSettings()->NotifyBeginPlay();

	// Then fire off match started
	GetWorldSettings()->NotifyMatchStarted();
}

void ALudoGameModeBase::HandleMatchHasPaused()
{

}

void ALudoGameModeBase::HandleMatchHasEnded()
{
	GameSession->HandleMatchHasEnded();
}

void ALudoGameModeBase::HandleMatchAborted()
{
}

bool ALudoGameModeBase::ReadyToStart()
{
	if (LudoMatchState.Compare(LudoMatchState::WaitingForPlayers))
	{
		return true;
	}
	return false;
}

void ALudoGameModeBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (LudoMatchState.Compare(LudoMatchState::WaitingForPlayers))
	{
		if (ReadyToStart())
		{
			UE_LOG(LogTemp, Log, TEXT("GameModeBase: ReadyToStartMatch."));
			StartLudoMatch();
		}
	}
	if (LudoMatchState.Compare(LudoMatchState::GamePause))
	{

	}
}

void ALudoGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	SetLudoMatchState(LudoMatchState::EnteringMap);
}

void ALudoGameModeBase::StartPlay()
{
	if (LudoMatchState == LudoMatchState::EnteringMap)
	{
		SetLudoMatchState(MatchState::WaitingForPlayers);
	}
}

bool ALudoGameModeBase::HasMatchStarted() const
{
	if (LudoMatchState.Compare(LudoMatchState::WaitingForPlayers))
	{
		return false;
	}

	return true;
}

void ALudoGameModeBase::PostLogin(APlayerController* NewPlayer)
{	
	Super::PostLogin(NewPlayer);
}

void ALudoGameModeBase::Logout(AController* Exiting)
{
	APlayerController* PC = Cast<APlayerController>(Exiting);
	if (PC != nullptr)
	{				
		PlayerControllerList.Remove(PC);
		AddInactivePlayer(PC->PlayerState, PC);		
	}

	//Is intended leaving?
	//SetLudoMatchState(LudoMatchState::GamePause);
	Super::Logout(Exiting);	
}

void ALudoGameModeBase::OverridePlayerState(APlayerController* PC, APlayerState* OldPlayerState)
{
	PC->PlayerState->DispatchOverrideWith(OldPlayerState);
}

void ALudoGameModeBase::AddInactivePlayer(APlayerState* PlayerState, APlayerController* PC)
{
	check(PlayerState)
		UWorld* LocalWorld = GetWorld();
	// don't store if it's an old PlayerState from the previous level or if it's a spectator... or if we are shutting down
	if (!PlayerState->IsFromPreviousLevel() && !MustSpectate(PC) && !LocalWorld->bIsTearingDown)
	{
		APlayerState* const NewPlayerState = PlayerState->Duplicate();
		if (NewPlayerState)
		{
			// Side effect of Duplicate() adding PlayerState to PlayerArray (see APlayerState::PostInitializeComponents)
			GameState->RemovePlayerState(NewPlayerState);

			// make PlayerState inactive
			NewPlayerState->SetReplicates(false);

			// delete after some time
			NewPlayerState->SetLifeSpan(InactivePlayerStateLifeSpan);

			// On console, we have to check the unique net id as network address isn't valid
			const bool bIsConsole = !PLATFORM_DESKTOP;
			// Assume valid unique ids means comparison should be via this method
			const bool bHasValidUniqueId = NewPlayerState->GetUniqueId().IsValid();
			// Don't accidentally compare empty network addresses (already issue with two clients on same machine during development)
			const bool bHasValidNetworkAddress = !NewPlayerState->SavedNetworkAddress.IsEmpty();
			const bool bUseUniqueIdCheck = bIsConsole || bHasValidUniqueId;

			// make sure no duplicates
			for (int32 Idx = 0; Idx < InactivePlayerArray.Num(); ++Idx)
			{
				APlayerState* const CurrentPlayerState = InactivePlayerArray[Idx];
				if ((CurrentPlayerState == nullptr) || IsValid(CurrentPlayerState))
				{
					// already destroyed, just remove it
					InactivePlayerArray.RemoveAt(Idx, 1);
					Idx--;
				}
				else if ((!bUseUniqueIdCheck && bHasValidNetworkAddress && (CurrentPlayerState->SavedNetworkAddress == NewPlayerState->SavedNetworkAddress))
					|| (bUseUniqueIdCheck && (CurrentPlayerState->GetUniqueId() == NewPlayerState->GetUniqueId())))
				{
					// destroy the playerstate, then remove it from the tracking
					CurrentPlayerState->Destroy();
					InactivePlayerArray.RemoveAt(Idx, 1);
					Idx--;
				}
			}
			InactivePlayerArray.Add(NewPlayerState);

			// make sure we dont go over the maximum number of inactive players allowed
			if (InactivePlayerArray.Num() > MaxInactivePlayers)
			{
				int32 const NumToRemove = InactivePlayerArray.Num() - MaxInactivePlayers;

				// destroy the extra inactive players
				for (int Idx = 0; Idx < NumToRemove; ++Idx)
				{
					APlayerState* const PS = InactivePlayerArray[Idx];
					if (PS != nullptr)
					{
						PS->Destroy();
					}
				}

				// and then remove them from the tracking array
				InactivePlayerArray.RemoveAt(0, NumToRemove);
			}
		}
	}
}

bool ALudoGameModeBase::FindInactivePlayer(APlayerController* PC)
{
	check(PC && PC->PlayerState);
	// don't bother for spectators
	if (MustSpectate(PC))
	{
		return false;
	}

	// On console, we have to check the unique net id as network address isn't valid
	const bool bIsConsole = !PLATFORM_DESKTOP;
	// Assume valid unique ids means comparison should be via this method
	const bool bHasValidUniqueId = PC->PlayerState->GetUniqueId().IsValid();
	// Don't accidentally compare empty network addresses (already issue with two clients on same machine during development)
	const bool bHasValidNetworkAddress = !PC->PlayerState->SavedNetworkAddress.IsEmpty();
	const bool bUseUniqueIdCheck = bIsConsole || bHasValidUniqueId;

	const FString NewNetworkAddress = PC->PlayerState->SavedNetworkAddress;
	const FString NewName = PC->PlayerState->GetPlayerName();
	for (int32 i = 0; i < InactivePlayerArray.Num(); i++)
	{
		APlayerState* CurrentPlayerState = InactivePlayerArray[i];
		if ((CurrentPlayerState == nullptr) || IsValid(CurrentPlayerState))
		{
			InactivePlayerArray.RemoveAt(i, 1);
			i--;
		}
		else if ((bUseUniqueIdCheck && (CurrentPlayerState->GetUniqueId() == PC->PlayerState->GetUniqueId())) ||
			(!bUseUniqueIdCheck && bHasValidNetworkAddress && (FCString::Stricmp(*CurrentPlayerState->SavedNetworkAddress, *NewNetworkAddress) == 0) && (FCString::Stricmp(*CurrentPlayerState->GetPlayerName(), *NewName) == 0)))
		{
			// found it!
			APlayerState* OldPlayerState = PC->PlayerState;
			PC->PlayerState = CurrentPlayerState;
			PC->PlayerState->SetOwner(PC);
			PC->PlayerState->SetReplicates(true);
			PC->PlayerState->SetLifeSpan(0.0f);
			OverridePlayerState(PC, OldPlayerState);
			GameState->AddPlayerState(PC->PlayerState);
			InactivePlayerArray.RemoveAt(i, 1);
			OldPlayerState->SetIsInactive(true);
			// Set the uniqueId to nullptr so it will not kill the player's registration 
			// in UnregisterPlayerWithSession()
			OldPlayerState->SetUniqueId(nullptr);
			OldPlayerState->Destroy();
			PC->PlayerState->OnReactivated();
			return true;
		}

	}
	return false;
}
