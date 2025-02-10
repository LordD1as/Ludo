// Fill out your copyright notice in the Description page of Project Settings.


#include "LudoGameInstance.h"
#include "NativeGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Events/GameplayMessageSubsystem.h"
#include "Events/LobbyMessage.h"
#include "LudoPlayerController.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Show_Widget, "Event.ShowUI");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UpdateLobbyWidget, "Event.UpdateLobbyUI");

void ULudoGameInstance::Init()
{
	Super::Init();

	SessionSubsystem = GetSubsystem<USessionSubsystem>();
	
	//delete
	GetEngine()->OnNetworkFailure().AddUObject(this, &ULudoGameInstance::NetworkFailureHappened);

	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());

	const IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
	
	if (SessionInterface.IsValid())
	{
		SessionInterface->AddOnSessionFailureDelegate_Handle(FOnSessionFailureDelegate::CreateUObject(this, &ULudoGameInstance::HandleSessionFailure));
	}
}

void ULudoGameInstance::OnShowWidget(const FWidgetState& InWidgetState)
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(TAG_Show_Widget, InWidgetState);
}

void ULudoGameInstance::OnCreateSession(int32 PublicConnections, bool bEnableLan, FString Nickname)
{
	if (SessionSubsystem)
	{
		SessionSubsystem->CreateSession(PublicConnections, bEnableLan, Nickname);
	}
}

void ULudoGameInstance::OnFindSessions(int32 MaxSearchResults, bool bEnableLan)
{
	if (SessionSubsystem)
	{
		SessionSubsystem->FindSessions(MaxSearchResults, bEnableLan);
	}
}

void ULudoGameInstance::OnJoinSession(const FBlueprintSessionResult& SessionToJoin)
{
	if (SessionSubsystem)
	{
		SessionSubsystem->JoinGameSession(SessionToJoin);
	}	
}

void ULudoGameInstance::OnLeaveSession()
{	
	if (ALudoPlayerController* const PC = Cast<ALudoPlayerController>(GetFirstLocalPlayerController(GetWorld())))
	{
		PC->NotifyPCOnLeave();
	}

	if (SessionSubsystem)
	{
		SessionSubsystem->DestroySession();
		
		UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), MainMenuLevelRef);	
	}
}

void ULudoGameInstance::NetworkFailureHappened(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	if (ENetworkFailure::ConnectionLost == FailureType)
	{
		//UGameplayStatics::GetPlayerController(World,0) 
		OnLeaveSession();
	}

}

ULocalPlayer* ULudoGameInstance::GetLocalPlayerBP(const uint8 index) const
{
	return GetLocalPlayers()[index];
}

void ULudoGameInstance::HandleSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType)
{
	//remeber id
}
