// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LobbyRow.h"
#include "UI/LudoWidget.h"
#include "Events/LobbyMessage.h"
#include "Lobby.generated.h"

/**
 * 
 */
UCLASS()
class LUDOGAME_API ULobby : public ULudoWidget
{
	GENERATED_BODY()

public:
	virtual void OnWidgetShown() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<ULobbyRow> LobbyRowRef;

	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateUI(FGameplayTag Channel, const FLobbyMessage& Paylod);

	UFUNCTION(BlueprintPure)
	FString GetNicknameBP(const uint8 index) const;

private:
#pragma region EventSystem

	void StartListening();

	void AddListenerHandle(FGameplayMessageListenerHandle&& Handle);

	TArray<FGameplayMessageListenerHandle> ListenerHandles;
#pragma endregion
};
