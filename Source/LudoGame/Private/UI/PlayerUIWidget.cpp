// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerUIWidget.h"
#include "LudoGameState.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UpdatePlayerUIWidget, "Event.UpdatePlayerUI");

void UPlayerUIWidget::OnWidgetShown()
{
	Super::OnWidgetShown();

	StartListening();
}

void UPlayerUIWidget::OnUpdateUI(FGameplayTag Channel, const FPlayerUIMessage& Paylod)
{
	if (Paylod.TextInfo == TI_CLIENT_REJOIN)
	{
		OnRejoinPlayer(Paylod.NumberOnDice, Paylod.bIsOnMove);
		return;
	}

	if (Paylod.NumberOnDice > 0 && Paylod.NumberOnDice <= MAX_DICE_NUMBER)
	{
		OnUpdateDiceText(Paylod.NumberOnDice);
	}

	if (Paylod.TextInfo > 0)
	{
		OnUpdateInfoText(Paylod.TextInfo);
	}

	if (Paylod.TextInfo == TI_FIRST_ENTER)
	{
		OnChangeMoveUpdate(Paylod.bIsOnMove);
	}
}

void UPlayerUIWidget::StartListening()
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	AddListenerHandle(MessageSubsystem.RegisterListener(TAG_UpdatePlayerUIWidget, this, &UPlayerUIWidget::OnUpdateUI));
}

void UPlayerUIWidget::AddListenerHandle(FGameplayMessageListenerHandle&& Handle)
{
	ListenerHandles.Add(MoveTemp(Handle));
}
