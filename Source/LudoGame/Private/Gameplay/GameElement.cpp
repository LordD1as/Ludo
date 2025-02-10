// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/GameElement.h"
#include "Net/UnrealNetwork.h"
#include "DataAssets/ColorsTypes.h"

#define COLOR_CHANNELS 4

#define TEXTURE_SIZE 0.25f

#define IMAGE_WIDTH 64
#define IMAGE_HEIGHT 64

#define QUADRANT_SECTION_X  IMAGE_WIDTH >> 2
#define QUADRANT_SECTION_Y  IMAGE_HEIGHT >> 2

#define BLUE_RANGE(x, y) (x < QUADRANT_SECTION_X && y < QUADRANT_SECTION_Y)
#define RED_RANGE(x, y) ((x >= QUADRANT_SECTION_X && x < (IMAGE_WIDTH >> 1)) && y < QUADRANT_SECTION_Y)
#define GREEN_RANGE(x, y) (x < QUADRANT_SECTION_X && (y >= QUADRANT_SECTION_Y && y < (IMAGE_HEIGHT >> 1)))
#define YELLOW_RANGE(x, y) ((x >= QUADRANT_SECTION_X && x < (IMAGE_WIDTH >> 1)) && (y >= QUADRANT_SECTION_Y && y < (IMAGE_HEIGHT >> 1)))

#define WHITE_RANGE(x, y) ((x >=  48 && x < IMAGE_WIDTH) && (y >= 48 && y < IMAGE_HEIGHT))

const FName TextureParam = TEXT("LudoTexture");

void AGameElement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGameElement, GameElementID);
}

// Sets default values
AGameElement::AGameElement()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BoardMaterial = ConstructorHelpers::FObjectFinder<UMaterialInterface>(TEXT("Material'/Game/Materials/Mat_Base.Mat_Base'")).Object;
	ensureMsgf(BoardMaterial, TEXT("Can not find board material!"));

	ColorsTypes = ConstructorHelpers::FObjectFinder<UColorsTypes>(TEXT("/Game/DataAssets/DA_ColorsTypes")).Object; 
	ensureMsgf(ColorsTypes, TEXT("Can not find data asset colors types!"));

	bReplicates = true;
}

void AGameElement::AddInstanceToISMCopm(const FTransform& NewTrans, const uint8 FieldType, const uint8 FieldIndex, const EColors Color)
{
	FVector2D* UVpos = ColorsTypes->LudoColors.Find(Color);

	uint8 index = ISMComp->AddInstance(NewTrans);
	ISMComp->PerInstanceSMCustomData[index * ISMComp->NumCustomDataFloats] = FieldType;
	ISMComp->PerInstanceSMCustomData[index * ISMComp->NumCustomDataFloats + 1] = FieldIndex;
	ISMComp->PerInstanceSMCustomData[index * ISMComp->NumCustomDataFloats + 2] = (uint8)Color;
	ISMComp->PerInstanceSMCustomData[index * ISMComp->NumCustomDataFloats + 3] = UVpos->X * TEXTURE_SIZE;
	ISMComp->PerInstanceSMCustomData[index * ISMComp->NumCustomDataFloats + 4] = UVpos->Y * TEXTURE_SIZE;
}

void AGameElement::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ISMComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//0 = FieldType , 1 = FieldIndex, 2 = Color, 3 = UVx, 4 = UVy
	ISMComp->NumCustomDataFloats = 5;

	if (DynamicMaterial == nullptr)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(BoardMaterial, this);
	}
	LudoTexture = CreateLudoTexture();
	DynamicMaterial->SetTextureParameterValue(TextureParam, LudoTexture);
	ISMComp->SetMaterial(0, DynamicMaterial);

	AddInstanceComponent(ISMComp);
	ISMComp->SetStaticMesh(Mesh);
}

void AGameElement::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	/*if (DynamicMaterial)
	{
		DynamicMaterial->ConditionalBeginDestroy();
	}*/

	//if (ISMComp)
	//{
	//	ISMComp->ConditionalBeginDestroy();
	//}

	Super::EndPlay(EndPlayReason);
}

void AGameElement::CreateMaterial()
{
	if (ISMComp)
	{	
		if (DynamicMaterial == nullptr)
		{
			DynamicMaterial = UMaterialInstanceDynamic::Create(BoardMaterial, this);
		}

		if (LudoTexture == nullptr)
		{
			LudoTexture = CreateLudoTexture();
		}

		DynamicMaterial->SetTextureParameterValue(TextureParam, LudoTexture);

		ISMComp->SetMaterial(0, DynamicMaterial);	
	}
}

UTexture2D* AGameElement::CreateLudoTexture()
{
	UTexture2D* CustomTexture = UTexture2D::CreateTransient(IMAGE_WIDTH, IMAGE_HEIGHT);
	CustomTexture->Filter = TextureFilter::TF_Nearest;
	FByteBulkData* ImageData = &CustomTexture->GetPlatformData()->Mips[0].BulkData;
	uint8* RawImageData = (uint8*)ImageData->Lock(LOCK_READ_WRITE);

	SetImageColor(RawImageData);
		
	ImageData->Unlock();
	CustomTexture->UpdateResource();

	return CustomTexture;
}

void AGameElement::SetImageColor(uint8* RawImageData)
{
	//PixelPosition
	for (uint8 x = 0; x < IMAGE_WIDTH; x++)
	{
		for (uint8 y = 0; y < IMAGE_HEIGHT; y++)
		{
			//Channel
			for (uint8 ch = 0; ch < 4; ch++)
			{
				//RGBA
				RawImageData[COLOR_CHANNELS * (x + y * IMAGE_WIDTH) + ch] = GetChannelColor(x, y, ch);
			}
		}
	}
}

uint8 AGameElement::GetChannelColor(uint8 x, uint8 y, uint8 Channel)
{
	//Alpha channel - pixel is always visible, opacity 100%
	if (Channel == 3)
	{
		return UINT8_MAX;
	}

	if (Channel == 0 && BLUE_RANGE(x, y))
	{
		return UINT8_MAX;
	}
	else if (Channel == 2 && RED_RANGE(x, y))
	{
		return UINT8_MAX;
	}
	else if (Channel == 1 && GREEN_RANGE(x, y))
	{
		return UINT8_MAX;
	}
	else if (Channel > 0 && YELLOW_RANGE(x, y))
	{
		return UINT8_MAX;
	}
	else if (WHITE_RANGE(x ,y))
	{
		return UINT8_MAX;
	}
	return 0;
}
