// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Events/WidgetToShowMessage.h"
#include "SessionSubsystem.h"
#include "Net/Core/Connection/NetEnums.h"
#include "LudoGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class LUDOGAME_API ULudoGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> MainMenuLevelRef;

	UPROPERTY(BlueprintReadWrite)
	FString PlayerNicknameLocal;

	UFUNCTION(BlueprintCallable)
	void OnShowWidget(const FWidgetState& InWidgetState);
	
	UFUNCTION(BlueprintCallable)
	void OnCreateSession(int32 PublicConnections, bool bEnableLan, FString Nickname);

	UFUNCTION(BlueprintCallable)
	void OnFindSessions(int32 MaxSearchResults, bool bEnableLan);

	UFUNCTION(BlueprintCallable)
	void OnJoinSession(const FBlueprintSessionResult& SessionToJoin);

	UFUNCTION(BlueprintCallable)
	void OnLeaveSession();

	void NetworkFailureHappened(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

	UFUNCTION(BlueprintPure)
	ULocalPlayer* GetLocalPlayerBP (const uint8 index) const;
private:
	UPROPERTY()
	USessionSubsystem* SessionSubsystem;		

	void HandleSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType);
};
