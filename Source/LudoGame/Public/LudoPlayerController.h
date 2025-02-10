// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "AC_CheckReadiness.h"
#include "LudoPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class LUDOGAME_API ALudoPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALudoPlayerController();

	virtual void Tick(float DeltaSeconds) override;

	/** notify player about started match */
	UFUNCTION(reliable, client)
	void ClientGameStarted(const FTransform& CameraTransform);	

	UFUNCTION(reliable, client)
	void SetCameraTransform(const FTransform& InTransform);

	UFUNCTION(reliable, server)
	void Server_NotifyGameModeOnRejoinedPlayer();

	UFUNCTION(BlueprintCallable)
	void TryRollDice();

	UFUNCTION(reliable, server)
	void Server_TryRollDice();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ACameraActor> PlayerCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* IA_ChooseFigure;

	UFUNCTION()
	void OnReady();

	void NotifyPCOnLeave();

	UFUNCTION(reliable, client)
	void ResetTickOnComponent();

	UFUNCTION(reliable, server)
	void Server_ResetTickOnComponent();

	UFUNCTION()
	void OnLobbyUI(const FLobbyMessage& InLobby);

#pragma region Input 
protected:
	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
	class UInputMappingContext* InputMapping;
		
	void OnChooseFigure(const FInputActionValue& Value);

	UFUNCTION(reliable, server)
	void Server_OnChooseFigure(const uint8 InstanceIndex);

	UFUNCTION(reliable, server)
	void Server_NotifyGameMode();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	class UAC_CheckReadiness* CheckReadinessComponent;
#pragma endregion

private:
	UFUNCTION(Server, Reliable)
	void AddUniqueIdOnLeave(const FUniqueNetIdRepl& UserId);
};
