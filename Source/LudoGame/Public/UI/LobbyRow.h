// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Events/GameplayMessageSubsystem.h"
#include "Events/LobbyRowMessage.h"
#include "UI/LudoWidget.h"
#include "LobbyRow.generated.h"

/**
 * 
 */
UCLASS()
class LUDOGAME_API ULobbyRow : public ULudoWidget
{
	GENERATED_BODY()

public:
	virtual void OnWidgetShown() override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnUpdateUI(FGameplayTag Channel, const FLobbyRowMessage& Payload);
	
private:
#pragma region EventSystem

	void StartListening();

	void AddListenerHandle(FGameplayMessageListenerHandle&& Handle);

	TArray<FGameplayMessageListenerHandle> ListenerHandles;
#pragma endregion
};
