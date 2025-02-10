// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Lobby.h"
#include "NativeGameplayTags.h"
#include "LudoLocalPlayer.h"
#include "LudoPlayerState.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UpdateLobbyWidget, "Event.UpdateLobbyUI");

void ULobby::OnWidgetShown()
{
	Super::OnWidgetShown();

	StartListening();
}

FString ULobby::GetNicknameBP(const uint8 index) const
{
	ULocalPlayer* LocalPlayer = (index < GetGameInstance()->GetNumLocalPlayers()) ? GetGameInstance()->GetLocalPlayerByIndex(index) : NULL;
	if (ULudoLocalPlayer* LudoLocalPlayer = Cast<ULudoLocalPlayer>(LocalPlayer))
	{
		return LudoLocalPlayer->GetNickname();
	}
	return TEXT("NULL");
}

void ULobby::StartListening()
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	AddListenerHandle(MessageSubsystem.RegisterListener(TAG_UpdateLobbyWidget, this, &ULobby::OnUpdateUI));	

	OnUpdateUI(TAG_UpdateLobbyWidget, FLobbyMessage());
}

void ULobby::AddListenerHandle(FGameplayMessageListenerHandle&& Handle)
{
	ListenerHandles.Add(MoveTemp(Handle));
}
