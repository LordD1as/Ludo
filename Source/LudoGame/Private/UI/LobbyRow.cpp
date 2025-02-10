// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/LobbyRow.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UpdateLobbyRowUI, "Event.UpdateLobbyRowUI");

void ULobbyRow::OnWidgetShown()
{
	Super::OnWidgetShown();

	StartListening();
}

void ULobbyRow::StartListening()
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	AddListenerHandle(MessageSubsystem.RegisterListener(TAG_UpdateLobbyRowUI, this, &ULobbyRow::OnUpdateUI));
}

void ULobbyRow::AddListenerHandle(FGameplayMessageListenerHandle&& Handle)
{
	ListenerHandles.Add(MoveTemp(Handle));
}
