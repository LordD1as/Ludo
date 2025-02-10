// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LudoWidget.h"
#include "Events/GameplayMessageSubsystem.h"
#include "Events/PlayerUIMessage.h"
#include "PlayerUIWidget.generated.h"

/**
 * 
 */
UCLASS()
class LUDOGAME_API UPlayerUIWidget : public ULudoWidget
{
	GENERATED_BODY()

public:
	virtual void OnWidgetShown() override;

	UFUNCTION()
	void OnUpdateUI(FGameplayTag Channel, const FPlayerUIMessage& Paylod);

	UFUNCTION(BlueprintImplementableEvent)
	void OnChangeMoveUpdate(const bool bIsOnMove);

	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateDiceText(const uint8 numberOnDice);

	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateInfoText(const uint8 infoText);

	UFUNCTION(BlueprintImplementableEvent)
	void OnRejoinPlayer(const uint8 CurrentState, const bool bIsOnMove);

private:
#pragma region EventSystem

	void StartListening();

	void AddListenerHandle(FGameplayMessageListenerHandle&& Handle);

	TArray<FGameplayMessageListenerHandle> ListenerHandles;
#pragma endregion
	
};
