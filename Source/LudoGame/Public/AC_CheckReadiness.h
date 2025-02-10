// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AC_CheckReadiness.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReady);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LUDOGAME_API UAC_CheckReadiness : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAC_CheckReadiness();

	void ResetTick();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FOnReady OnReadyEvent;
};
