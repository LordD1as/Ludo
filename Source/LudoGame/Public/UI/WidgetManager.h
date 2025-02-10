 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GameplayTagContainer.h"
#include "Events/GameplayMessageSubsystem.h"
#include "ErrorMsg.h"
#include "LudoWidget.h"
#include "WidgetManager.generated.h"

struct FWidgetState;
enum class EWidgetState : uint8;

/**
 * 
 */
UCLASS()
class LUDOGAME_API AWidgetManager : public AHUD
{
	GENERATED_BODY()

public:
	virtual void OnConstruction(const FTransform& Transform) override;

#pragma region Game
	//HUD, ammo count, crosshair etc.
	UPROPERTY(EditAnywhere)
		TSoftClassPtr<ULudoWidget> PlayerUIScreenRef;
#pragma endregion

#pragma region Games menu
	//Inventory, character menu etc.
	
#pragma endregion

#pragma region Menus
	//Main menu, server list etc.
	UPROPERTY(EditAnywhere)
		TSoftClassPtr<ULudoWidget> MainMenuScreenRef;

	UPROPERTY(EditAnywhere)
		TSoftClassPtr<ULudoWidget> ServerListScreenRef;

	UPROPERTY(EditAnywhere)
		TSoftClassPtr<ULudoWidget> LobbyScreenRef;
#pragma endregion
	
#pragma region Modals
	//Popups 
	UPROPERTY(EditAnywhere)
		TSoftClassPtr<UErrorMsg> ErrorMessageRef;
#pragma endregion

	void OnShowWidget(FGameplayTag Channel, const FWidgetState& Payload);

private:
	ULudoWidget* MainMenuScreen;
	ULudoWidget* ServerListScreen;
	ULudoWidget* LobbyScreen;
	ULudoWidget* PlayerUIScreen;

	EWidgetState CurrentState;

	ULudoWidget* CurrentWidget;
	ULudoWidget* LastWidget;

	EWidgetState NewState(const FWidgetState& Payload);

	/* Load widget 
	*/
	ULudoWidget* GetNewWidget(const FWidgetState& Payload);

#pragma region EventSystem

	void StartListening();

	void AddListenerHandle(FGameplayMessageListenerHandle&& Handle);

	TArray<FGameplayMessageListenerHandle> ListenerHandles;
#pragma endregion
};
