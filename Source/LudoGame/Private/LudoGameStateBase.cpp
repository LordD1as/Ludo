// Fill out your copyright notice in the Description page of Project Settings.


#include "LudoGameStateBase.h"
#include "LudoGameModeBase.h"
#include "Net/UnrealNetwork.h"
#include "NativeGameplayTags.h"
#include "Events/GameplayMessageSubsystem.h"

const FString MainMenuMap = TEXT("MainMenu");
const FString LobbyMap = TEXT("Lobby");

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UpdateLobbyWidget, "Event.UpdateLobbyUI");

void ALudoGameStateBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALudoGameStateBase, LudoMatchState);
}

void ALudoGameStateBase::SetLudoMatchState(FName NewState)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		UE_LOG(LogTemp, Log, TEXT("Match State Changed from %s to %s"), *LudoMatchState.ToString(), *NewState.ToString());

		LudoMatchState = NewState;

		// Call the onrep to make sure the callbacks happen
		OnRep_LudoMatchState();
	}
}

void ALudoGameStateBase::OnRep_LudoMatchState()
{
	if (LudoMatchState.Compare(LudoMatchState::WaitingForPlayers))
	{
		HandleMatchIsWaitingForPlayers();
	}

	if (LudoMatchState.Compare(LudoMatchState::GameLoop))
	{
		HandleMatchHasStarted();
	}
	else if (LudoMatchState.Compare(LudoMatchState::GamePause))
	{
		HandleMatchHasPaused();
	}
	else if (LudoMatchState.Compare(LudoMatchState::EndGame))
	{
		HandleMatchHasEnded();
	}
}

void ALudoGameStateBase::HandleMatchIsWaitingForPlayers()
{
	if (GetLocalRole() != ROLE_Authority)
	{
		GetWorldSettings()->NotifyBeginPlay();
	}
}

void ALudoGameStateBase::HandleMatchHasStarted()
{
	if (GetLocalRole() != ROLE_Authority)
	{
		GetWorldSettings()->NotifyMatchStarted();
	}
	else
	{
		// Now that match has started, act like the base class and set replicated flag
		bReplicatedHasBegunPlay = true;
	}
}

void ALudoGameStateBase::HandleMatchHasPaused()
{

}

void ALudoGameStateBase::HandleMatchHasEnded()
{

}
