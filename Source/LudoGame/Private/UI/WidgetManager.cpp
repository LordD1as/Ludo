// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/WidgetManager.h"
#include "NativeGameplayTags.h"
#include "Events/WidgetToShowMessage.h"
#include "Blueprint/UserWidget.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Show_Widget, "Event.ShowUI");
const int ZOrder = 9999;

void AWidgetManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	StartListening();
}

void AWidgetManager::OnShowWidget(FGameplayTag Channel, const FWidgetState& Payload)
{
	if (CurrentState == Payload.WidgetToShow)
	{
		return;
	}
	
	CurrentState = NewState(Payload);
}

EWidgetState AWidgetManager::NewState(const FWidgetState& Payload)
{
	if (CurrentWidget != nullptr && Payload.WidgetToShow != EWidgetState::ErrorMsg)
	{
		CurrentWidget->RemoveFromParent();
	}

	CurrentWidget = GetNewWidget(Payload);

	if (CurrentWidget != nullptr)
	{
		CurrentWidget->AddToViewport(ZOrder);
		CurrentWidget->OnWidgetShown();

		APlayerController* PC = GetOwningPlayerController();

		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeGameAndUI());
	}

	if (Payload.WidgetToShow == EWidgetState::ErrorMsg)
	{
		return EWidgetState::None;
	}
	return Payload.WidgetToShow;
}

void AWidgetManager::StartListening()
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	AddListenerHandle(MessageSubsystem.RegisterListener(TAG_Show_Widget, this, &AWidgetManager::OnShowWidget));
}

void AWidgetManager::AddListenerHandle(FGameplayMessageListenerHandle&& Handle)
{
	ListenerHandles.Add(MoveTemp(Handle));
}

ULudoWidget* AWidgetManager::GetNewWidget(const FWidgetState& Payload)
{	
	switch (Payload.WidgetToShow)
	{
	case EWidgetState::None:
		break;
	case EWidgetState::MainMenuScreen:
		if (MainMenuScreen == nullptr)
		{
			MainMenuScreen = CreateWidget<ULudoWidget>(GetWorld(), MainMenuScreenRef.LoadSynchronous());
			return MainMenuScreen;
		}

		return MainMenuScreen;		
	case EWidgetState::ServerListScreen:
		if (ServerListScreen == nullptr)
		{
			ServerListScreen = CreateWidget<ULudoWidget>(GetWorld(), ServerListScreenRef.LoadSynchronous());
			return ServerListScreen;
		}

		return ServerListScreen;
	case EWidgetState::LobbyScreen:
		if (LobbyScreen == nullptr)
		{
			LobbyScreen = CreateWidget<ULudoWidget>(GetWorld(), LobbyScreenRef.LoadSynchronous());
			return LobbyScreen;
		}

		return LobbyScreen;
	case EWidgetState::PlayerUIScreen:
		if (PlayerUIScreen == nullptr)
		{
			PlayerUIScreen = CreateWidget<ULudoWidget>(GetWorld(), PlayerUIScreenRef.LoadSynchronous());
			return PlayerUIScreen;
		}

		return PlayerUIScreen;		
	case EWidgetState::ErrorMsg:
		if (UErrorMsg* ErrorMessage = CreateWidget<UErrorMsg>(GetWorld(), ErrorMessageRef.LoadSynchronous()))
		{
			ErrorMessage->OnShowMsg(Payload.ErrorMsg);
			ErrorMessage->AddToViewport(ZOrder);
			ErrorMessage->OnWidgetShown();

			return nullptr;
		}
	}

	return nullptr;
}
