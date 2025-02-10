// Fill out your copyright notice in the Description page of Project Settings.


#include "LudoPlayerController.h"
#include "Events/LobbyMessage.h"
#include "Events/PlayerUIMessage.h"
#include "Events/WidgetToShowMessage.h"
#include "Events/LobbyMessage.h"
#include "NativeGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "Gameplay/Figure.h"
#include "LudoGameMode.h" 
#include "LudoGameState.h"
#include "LudoPlayerState.h"
#include "LudoGameInstance.h"
#include "Net/UnrealNetwork.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UpdateLobbyWidget, "Event.UpdateLobbyUI");

void ALudoPlayerController::ClientGameStarted_Implementation(const FTransform& CameraTransform)
{	
	SetCameraTransform(CameraTransform);

	if (ULudoGameInstance* LudoGameInstance = GetGameInstance<ULudoGameInstance>())
	{
		LudoGameInstance->OnShowWidget(FWidgetState(EWidgetState::PlayerUIScreen));
	}
}

void ALudoPlayerController::SetCameraTransform_Implementation(const FTransform& InTransform)
{
	ACameraActor* PlayerCameraLoc = Cast<ACameraActor>(UGameplayStatics::GetActorOfClass(GetWorld(), PlayerCamera));
	if (PlayerCameraLoc)
	{
		PlayerCameraLoc->SetActorTransform(InTransform);
		SetViewTargetWithBlend(PlayerCameraLoc);	
		if (ULudoGameInstance* LudoGameInstance = GetGameInstance<ULudoGameInstance>())
		{
			LudoGameInstance->OnShowWidget(FWidgetState(EWidgetState::PlayerUIScreen));
		}
		//Server_NotifyGameModeOnRejoinedPlayer();
	}
}

void ALudoPlayerController::Server_NotifyGameModeOnRejoinedPlayer_Implementation()
{
	if (ALudoGameMode* LudoGameMode = Cast<ALudoGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		LudoGameMode->HasBeenNotifiedOnPlayerRejoined(this);
	}
}

ALudoPlayerController::ALudoPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	CheckReadinessComponent = CreateDefaultSubobject<UAC_CheckReadiness>(TEXT("CheckReadinessComponent"));	
	CheckReadinessComponent->OnReadyEvent.AddDynamic(this, &ThisClass::OnReady);
}

void ALudoPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	//UE_LOG(LogTemp, Log, TEXT("NetRole %s: %s %d"),  *GetName(), GetRemoteRole() == ENetRole::ROLE_Authority ? TEXT("Server") : TEXT("Client"), HasAuthority());
}

void ALudoPlayerController::TryRollDice()
{
	Server_TryRollDice();
}

void ALudoPlayerController::Server_TryRollDice_Implementation()
{
	if (ALudoGameState* LudoGameState = GetGameInstance()->GetWorld()->GetGameState<ALudoGameState>())
	{
		LudoGameState->Server_RollDice();
	}
}

void ALudoPlayerController::OnReady()
{
	//Notify server we are ready
	Server_NotifyGameMode();
}

void ALudoPlayerController::NotifyPCOnLeave()
{
	if (!HasAuthority())
	{		
		AddUniqueIdOnLeave(GetLocalPlayer()->GetUniqueNetIdForPlatformUser());
	}
}

void ALudoPlayerController::ResetTickOnComponent_Implementation()
{
	CheckReadinessComponent->ResetTick();
}

void ALudoPlayerController::Server_ResetTickOnComponent_Implementation()
{
	ResetTickOnComponent();
}

void ALudoPlayerController::OnLobbyUI(const FLobbyMessage& InLobby)
{
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSystem.BroadcastMessage(TAG_UpdateLobbyWidget, InLobby);
}

void ALudoPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// Get the local player subsystem
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	// Clear out existing mapping, and add our mapping
	Subsystem->ClearAllMappings();
	Subsystem->AddMappingContext(InputMapping, 0);

	// Get the EnhancedInputComponent
	UEnhancedInputComponent* PEI = Cast<UEnhancedInputComponent>(InputComponent);
	// Bind the actions
	PEI->BindAction(IA_ChooseFigure, ETriggerEvent::Triggered, this, &ALudoPlayerController::OnChooseFigure);
}

void ALudoPlayerController::OnChooseFigure(const FInputActionValue& Value)
{
	//remove binding when you are in move and bind when you are
	FHitResult HitResult;

	if (GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(COLLISION_FIGURE), true, HitResult))
	{
		const uint8 index = HitResult.Item;
		if (AFigure* Figure = Cast<AFigure>(HitResult.GetActor()))
		{
			const uint8 color = Figure->ISMComp->PerInstanceSMCustomData[index * Figure->ISMComp->NumCustomDataFloats + 2];
			if (ALudoPlayerState* LudoPlayerState = GetPlayerState<ALudoPlayerState>())
			{
				if (LudoPlayerState->LudoPlayerID == color)
				{
					Server_OnChooseFigure(index);
				}				
				else
				{
					LudoPlayerState->UpdatePlayerUI(FPlayerUIMessage(0, TI_YOU_CAN_TOUCH_ONLY_YOUR_FIGURE));
				}
			}
		}
	}
}

void ALudoPlayerController::Server_NotifyGameMode_Implementation()
{
	if (ALudoGameMode* LudoGameMode = Cast<ALudoGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		LudoGameMode->NotifyOnReady(this);
	}
}

void ALudoPlayerController::AddUniqueIdOnLeave_Implementation(const FUniqueNetIdRepl& UserId)
{
	if (ALudoGameMode* LudoGameMode = Cast<ALudoGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		LudoGameMode->LogoutPlayers.AddUnique(UserId);	
		//Send multicast to all clients that player has been disconnected
		if (ALudoGameState* ServerLudoGameState = Cast<ALudoGameState>(UGameplayStatics::GetGameState(this)))
		{			
			ServerLudoGameState->Server_SetPauseState(true);			
		}
		//Run timer 60s when does not connect destroy session -> when player connects invalidate timer
		LudoGameMode->RunEndTimer();
	}
}

void ALudoPlayerController::Server_OnChooseFigure_Implementation(const uint8 InstanceIndex)
{
	if (ALudoGameState* LudoGameState = Cast<ALudoGameState>(UGameplayStatics::GetGameState(GetGameInstance()->GetWorld())))
	{
		if (ALudoPlayerState* LudoPlayerState = GetPlayerState<ALudoPlayerState>())
		{
			if (LudoPlayerState->ServerPlayerID == LudoGameState->CurrentPlayerID)
			{
				LudoGameState->Server_TryMoveFigure(InstanceIndex);
			}
		}
	}
}
