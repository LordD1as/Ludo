// Fill out your copyright notice in the Description page of Project Settings.


#include "AC_CheckReadiness.h"
#include "LudoPlayerController.h"
#include "LudoGameState.h"
#include "LudoPlayerState.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UAC_CheckReadiness::UAC_CheckReadiness()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	SetComponentTickEnabled(true);
}

void UAC_CheckReadiness::ResetTick()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called every frame
void UAC_CheckReadiness::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//UE_LOG(LogTemp, Log, TEXT("Component Tick: %s"), *GetName());

	/*if (UWorld* World = GetWorld())
	{
		if (ALudoGameState* LudoGameState = World->GetGameState<ALudoGameState>())
		{
			if (ALudoPlayerState* LudoPlayerState = Cast<ALudoPlayerState>(World->GetFirstPlayerController()->PlayerState))
			{
				if (LudoPlayerState->bTryToRejoin && LudoGameState->GetMatchState() == FName(TEXT("Rejoin")))
				{
					OnReadyEvent.Broadcast();
					PrimaryComponentTick.bCanEverTick = false;
				}
			}			
		}		
	}*/
}
