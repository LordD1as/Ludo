// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LudoGameState.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GameElement.generated.h"

UCLASS()
class LUDOGAME_API AGameElement : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AGameElement();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EColors OwnerColor;

	/* For field it is just id. For figure it is id position in an array of fields. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	uint8 GameElementID;

	UFUNCTION(BlueprintCallable)
	virtual void AddInstanceToISMCopm(const FTransform& NewTrans, const uint8 FieldType, const uint8 FieldIndex, const EColors Color);

	UPROPERTY(EditAnywhere)
	UInstancedStaticMeshComponent* ISMComp;

	UPROPERTY()
	UTexture2D* LudoTexture;

protected:
	virtual void PostInitializeComponents() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY()
	UStaticMesh* Mesh;	

	UPROPERTY()
	UMaterialInterface* BoardMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;

	UPROPERTY(EditAnywhere)
	class UColorsTypes* ColorsTypes;

	virtual void CreateMaterial();

	UTexture2D* CreateLudoTexture();

	void SetImageColor(uint8* RawImageData);

	/* Return color B (0-15, 0-15), R (16 - 31, 0 - 15), G ( 0 - 15, 16 - 31), Y (16-31, 16-31). Where (Xa - Xb, Ya - Yb) x and y are positions on the image, where a and b are range.*/
	uint8 GetChannelColor(uint8 x, uint8 y, uint8 Channel);
};
