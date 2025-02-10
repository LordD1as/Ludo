// Fill out your copyright notice in the Description page of Project Settings.


#include "LudoGameState.h"
#include "LudoGameInstance.h"
#include "NativeGameplayTags.h"
#include "Events/GameplayMessageSubsystem.h"
#include "Events/LobbyMessage.h"
#include "LudoGameMode.h"
#include "LudoPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Gameplay/FieldTile.h"
#include "Gameplay/Figure.h"
#include "Net/UnrealNetwork.h"
#include "Engine.h"

const uint8 MaxRolls = 3;
const uint8 DistanceBetweenStartsFields = 10;

const uint8 OffsetBetweenHangarAndFieldTiles = 14;
const uint8 NumberOfFigures = 4;
const float FigureZPos = 200.0f;

const FName FieldsTag = TEXT("Fields");
const FString MainMenuMap = TEXT("MainMenu");
const FString LobbyMap = TEXT("Lobby");

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_UpdateLobbyWidget, "Event.UpdateLobbyUI");

void ALudoGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALudoGameState, NumberOnDice);
	DOREPLIFETIME(ALudoGameState, CurrentPlayerID);
	DOREPLIFETIME(ALudoGameState, bPauseState);
	DOREPLIFETIME(ALudoGameState, LudoCurrentState);
}

void ALudoGameState::OnUpdateLobbyUI_Implementation()
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(TAG_UpdateLobbyWidget, FLobbyMessage());	
}

#pragma region Pre game state
void ALudoGameState::ShowDefaultWidget()
{
	FString MapName = GetWorld()->GetMapName();
	MapName = GetWorld()->RemovePIEPrefix(MapName);

	if (ULudoGameInstance* LudoGameInstance = GetGameInstance<ULudoGameInstance>())
	{
		if (MapName.Contains(MainMenuMap))
		{
			LudoGameInstance->OnShowWidget(FWidgetState(EWidgetState::MainMenuScreen));
		}
		else if (MapName.Contains(LobbyMap))
		{
			LudoGameInstance->OnShowWidget(FWidgetState(EWidgetState::LobbyScreen));
		}
	}
}

void ALudoGameState::SetOwnerForPCs_Implementation()
{
	for (size_t i = 0; i < PlayerArray.Num(); i++)
	{
		if (ALudoPlayerState* LudoPlayerState = Cast<ALudoPlayerState>(PlayerArray[i]))
		{
			if (LudoPlayerState->GetPlayerController())
			{
				LudoPlayerState->GetPlayerController()->SetOwner(this);
				UE_LOG(LogTemp, Warning, TEXT("The Actor's name is %s"), *this->GetName());
			}
		}
	}
}

//deprecated
bool ALudoGameState::HasAnybodySameColor(FString InColor)
{
	/*for (size_t i = 0; i < PlayerArray.Num(); i++)
	{
		if (ALudoPlayerState* LudoPlayer = Cast<ALudoPlayerState>(PlayerArray[i]))
		{
			if (LudoPlayer->PlayerColor.Contains(InColor))
			{				
				return true;
			}
		}
	}*/

	return false;
}

bool ALudoGameState::AreAllReady()
{
	if (PlayerArray.Num() == 1)
	{
		return false;
	}

	//for (size_t i = 0; i < PlayerArray.Num(); i++)
	//{
	//	if (ALudoPlayerState* LudoPlayer = Cast<ALudoPlayerState>(PlayerArray[i]))
	//	{
	//		if (!LudoPlayer->bIsReady)
	//		{
	//			return false;
	//		}
	//	}
	//}

	return true;
}

void ALudoGameState::Server_SetPauseState_Implementation(const bool bNewPauseState)
{
	bPauseState = bNewPauseState;

	if (bPauseState)
	{
		UpdatePlayersUI(FPlayerUIMessage(0, TI_CLIENT_LOGOUT));
	}
	else
	{
		UpdatePlayersUI(FPlayerUIMessage((uint8)LudoCurrentState, TI_CLIENT_REJOIN));
	}
}

void ALudoGameState::Server_TryMoveFigure_Implementation(const uint8 InstanceIndex)
{
	if (bPauseState)
	{
		return;
	}

	const uint8 FieldTypeLoc = GetLudoFigureFieldType(InstanceIndex);
	//Check if he must put on figure
	if (MustPutOnFigure() && FieldTypeLoc != (uint8)EFieldType::Hangar)
	{
		GetCurrentPlayer()->UpdatePlayerUI(FPlayerUIMessage(0, TI_MUST_PUT_ON));	
		return;
	}
	
	//Player can touch only his figures 
	if (GetCurrentPlayer()->LudoPlayerID != GetLudoFigureColor(InstanceIndex))
	{
		GetCurrentPlayer()->UpdatePlayerUI(FPlayerUIMessage(0, TI_YOU_CAN_TOUCH_ONLY_YOUR_FIGURE));		
		return;
	}

	switch (FieldTypeLoc)
	{
	case 1: //Home
		if (!TryMoveFigureAtHome(InstanceIndex))
		{
			return;
		}
		break;
	case 0: //Neutral	
	case 2: //Start
		if (!TryMoveInField(InstanceIndex))
		{
			if (!TryToMoveFromFieldToHome(InstanceIndex))
			{
				return;
			}
		}
		break;
	case 3: //Hangar
		if (!TryPutFigureOnStart(InstanceIndex))
		{
			return;
		}
		break;
	}
	
	if (NumberOnDice != MAX_DICE_NUMBER && !CanRoll())
	{
		ChangeMoveForAnotherPlayer();
	}
	else
	{
		GetCurrentPlayer()->UpdatePlayerUI(FPlayerUIMessage(0, TI_UPDATE_ROLL));	
	}

	NumberOnDice = 0;
	LudoCurrentState = EGameState::RollDice;
}

void ALudoGameState::HandleMatchHasStarted()
{
	if (GetMatchState() != FName(TEXT("Rejoin")))
	{
		ShowDefaultWidget();
	}
	else if (GetMatchState() == FName(TEXT("Rejoin")))
	{
		//Call on PC
		Server_SetLudoMatchState();
	}
}
void ALudoGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	/*FigureActor->Destroy();

	FieldTileActor->Destroy();*/

	Super::EndPlay(EndPlayReason);
}
#pragma endregion

#pragma region Board Preparation
void ALudoGameState::OnStartLudoGame_Implementation()
{
	PrepareBoard();
}

void ALudoGameState::PrepareBoard()
{
	SetGameElements();	

	if (HasAuthority())
	{
		PrepareFiguresInHangar();

		SetAndShowFiguresInHangar();

		SetFieldsAndHomesPositions();

		MarkStartPlayer();

		//test
		/*for (size_t i = 5; i < 8; i++)
		{
			const uint8 StartFieldIndex = 0;
			int32 hangarIndex = OffsetBetweenHangarAndFieldTiles * (i / NumberOfFigures) + i;
			FVector FigurePositionInHangar = GetLudoFieldPosition(hangarIndex);

			FigurePositionInHangar.Z = FigureZPos;
			FiguresInHangar.Emplace(FigurePositionInHangar) = UINT8_MAX;
			FiguresAtHome[i] =1;

			Multicast_ShowFigures(i, HomeFieldsPositions[i], FFigureInfo(EFieldType::Home, i));
		}
		
		int32 hangarIndex = OffsetBetweenHangarAndFieldTiles * (4 / NumberOfFigures) + 4;
		FVector FigurePositionInHangar = GetLudoFieldPosition(hangarIndex);

		FigurePositionInHangar.Z = FigureZPos;
		FiguresInHangar.Emplace(FigurePositionInHangar) = UINT8_MAX;
		FiguresInField[9] = 4;

		Multicast_ShowFigures(4, FieldsPositions[9], FFigureInfo(EFieldType::Neutral, 9));*/
	}
}

void ALudoGameState::SetGameElements()
{
	for (TActorIterator<AGameElement> It(GetGameInstance()->GetWorld(), AGameElement::StaticClass()); It; ++It)
	{
		AGameElement* GameElement = *It;
		if (GameElement->ActorHasTag(FieldsTag))
		{
			FieldTileActor = Cast<AFieldTile>(GameElement);
		}
		else
		{
			FigureActor = Cast<AFigure>(GameElement);
		}
	}
}

void ALudoGameState::PrepareFiguresInHangar()
{	
	//Hangar and figures are the first things generated, indexed 0-15 in order B,R,G,Y (hangar are offset by 14)
	const uint8 numberOfHangares = FMath::Pow(2, (float)PlayerMode);
	for (size_t i = 0; i < numberOfHangares; i++)
	{
		if (ensureMsgf(FieldTileActor, TEXT("FieldTileActor has not been set!")))
		{
			int32 instanceIndex = OffsetBetweenHangarAndFieldTiles * (i / NumberOfFigures) + i;		
			//Get hangar position		
			FVector FigurePositionInHangar = GetLudoFieldPosition(instanceIndex);
			FigurePositionInHangar.Z = FigureZPos;

			//Fill tmap as empty
			FiguresInHangar.Add(FigurePositionInHangar, UINT8_MAX);
		}
	}
}

void ALudoGameState::SetAndShowFiguresInHangar()
{
	const uint8 numberOfHangares = FMath::Pow(2, (float)PlayerMode);
	//Fill relevant hangars tmap (on server) and show figures (all clients) based on players' colors
	//Get all curent players' colors
	for (size_t playerIndex = 0; playerIndex < PlayerArray.Num(); playerIndex++)
	{
		if (ALudoPlayerState* LudoPlayerState = Cast<ALudoPlayerState>(PlayerArray[playerIndex]))
		{
			for (size_t i = 0; i < NumberOfFigures; i++)
			{
				//Get index based on player color
				int32 instanceIndex = LudoPlayerState->LudoPlayerID * NumberOfFigures + i;
				//Get firugre
				FTransform LudoFigureTransform;
				FigureActor->ISMComp->GetInstanceTransform(instanceIndex, LudoFigureTransform);

				FVector FigurePosition = LudoFigureTransform.GetLocation();
				//Set figure position
				FigurePosition.Z = FigureZPos;

				//Add to a figure to tmap FiguresInHangar
				FiguresInHangar.Emplace(FigurePosition) = instanceIndex;
				ensureMsgf(FiguresInHangar.Num() == numberOfHangares, TEXT("Couldn't find hangar position!"));

				//Update visual
				LudoFigureTransform.SetLocation(FigurePosition);
				Multicast_ShowFigures(instanceIndex, FigurePosition, FFigureInfo(EFieldType::Hangar, UINT8_MAX));
			}
		}
	}
}

void ALudoGameState::MarkStartPlayer()
{
	//Mark server player as start player 
	CurrentPlayerID = 0;	

	GetCurrentPlayer()->UpdatePlayerUI(FPlayerUIMessage(true, 0, TI_FIRST_ENTER));
}

void ALudoGameState::SetFieldsAndHomesPositions()
{
	const uint8 numberOfFileds = GetNumberOfFields();
	const uint8 numberOfHomes = FMath::Pow(2, (float)PlayerMode);

	FieldsPositions.Init(FVector::ZeroVector, numberOfFileds);
	FiguresInField.Init(UINT8_MAX, numberOfFileds);

	HomeFieldsPositions.Init(FVector::ZeroVector, numberOfHomes);
	FiguresAtHome.Init(UINT8_MAX, numberOfHomes);

	const uint8 OffsetExcludedHangarIndicies = OffsetBetweenHangarAndFieldTiles + NumberOfFigures;

	for (size_t colorIndex = 0; colorIndex < (size_t)PlayerMode; colorIndex++)
	{
		//First is created 4 hangars (4 because 4 figures) start from right index
		for (size_t fieldIndex = NumberOfFigures; fieldIndex < OffsetExcludedHangarIndicies; fieldIndex++)
		{
			//exclude hangars indicies
			int32 instanceIndex = fieldIndex + colorIndex * OffsetExcludedHangarIndicies;
			const uint8 LudoFieldType = FieldTileActor->ISMComp->PerInstanceSMCustomData[instanceIndex * FieldTileActor->ISMComp->NumCustomDataFloats];
			const uint8 LudoFieldIndex = FieldTileActor->ISMComp->PerInstanceSMCustomData[instanceIndex * FieldTileActor->ISMComp->NumCustomDataFloats + 1];

			FVector FigurePosition = GetLudoFieldPosition(instanceIndex);
			FigurePosition.Z = FigureZPos;

			ensureMsgf(LudoFieldIndex < numberOfFileds, TEXT("Bad field id!"));
			if (LudoFieldType == (uint8)EFieldType::Home && LudoFieldIndex < numberOfHomes)
			{				
				HomeFieldsPositions[LudoFieldIndex] = FigurePosition;
			}
			else if (LudoFieldIndex < numberOfFileds) //Hangar index is always max uint8
			{
				FieldsPositions[LudoFieldIndex] = FigurePosition;
			}
		}
	}	
}

void ALudoGameState::Server_SetLudoMatchState_Implementation()
{
	SetMatchState(MatchState::InProgress);
}
#pragma endregion

int test = 0;
#pragma region Gameplay
void ALudoGameState::Server_RollDice_Implementation()
{
	if (bPauseState)
	{
		return;
	}
	NumberOnDice = FMath::RandRange(1, MAX_DICE_NUMBER);
	//NumberOnDice = bFirstEnter ? 6 : 2;

	//if (test > 3)
	//	NumberOnDice = 6;
	bFirstEnter = false;
	//if (GetCurrentPlayer()->LudoPlayerID == 0 && NumberOnDice != MAX_DICE_NUMBER)
	//{
	//	NumberOnDice = 2;
	//}
	//test += NumberOnDice;
	
	/*if (GetCurrentPlayer()->LudoPlayerID == 1 && NumberOnDice != MAX_DICE_NUMBER)
	{
		NumberOnDice = 1;
	}*/

	//If there is 6 on the dice than you have got a free roll
	if (NumberOnDice != MAX_DICE_NUMBER)
	{
		//check if can move any figure
		bool bCanMoveAnyFigure = CanMoveAnyFigure();
		if (bCanMoveAnyFigure)
		{
			GetCurrentPlayer()->UpdatePlayerUI(FPlayerUIMessage(NumberOnDice, TI_CHOOSE_YOUR_FIGURE));			
			LudoCurrentState = EGameState::ChooseFigure;
		}	
		
		CurrentRoll++;

		//Check if player is out of roll and can not move any figure
		if (!CanRoll() && !bCanMoveAnyFigure)
		{
			ChangeMoveForAnotherPlayer();
		}
		else if(!bCanMoveAnyFigure)
		{
			GetCurrentPlayer()->UpdatePlayerUI(FPlayerUIMessage(0, TI_CANT_MOVE_ANY_FIGURE_AND_PLAYER_IS_ON_MOVE));			
		}

		return;
	}

	GetCurrentPlayer()->UpdatePlayerUI(FPlayerUIMessage(NumberOnDice, TI_CHOOSE_YOUR_FIGURE));
}

bool ALudoGameState::CanMoveAnyFigure()
{
	//ask for each figure
	for (size_t i = 0; i < NumberOfFigures; i++)
	{
		const uint8 ColorIndex = GetColorIndex(i);
		const uint8 LudoFigureFieldType = GetLudoFigureFieldType(ColorIndex);
		const uint8 LudoFigureFieldIndex = GetLudoFigureFieldIndex(ColorIndex);

		//Is figure in hangar?
		if (LudoFigureFieldIndex == UINT8_MAX)
		{
			continue;
		}
		else if(CanMoveFigure(LudoFigureFieldIndex, LudoFigureFieldType))
		{
			return true;
		}
	}

	return false;
}

bool ALudoGameState::CanMoveFigure(const uint8 LudoFigureFieldIndex, const uint8 LudoFigureFieldType)
{
	uint8 indexInFieldTryPutFigure = LudoFigureFieldIndex + NumberOnDice;	
	uint8 HomeIndex;
	const bool bIsAtHome = LudoFigureFieldType == (uint8)EFieldType::Home;

	if (!bIsAtHome && CanGetHome(LudoFigureFieldIndex, indexInFieldTryPutFigure, HomeIndex))
	{
		//If there is a figure at your move, you can not move there
		if (IsThereYourFigure(HomeIndex))
		{
			return false;
		}

		return true;
	}

	if (bIsAtHome)
	{
		return CanMoveAtHome(0, HomeIndex, LudoFigureFieldIndex);
	}	

	//Safe for overflow
	indexInFieldTryPutFigure = indexInFieldTryPutFigure % GetNumberOfFields();
	if (ensureMsgf(indexInFieldTryPutFigure < GetNumberOfFields(), TEXT("Index in fields overflow!")))
	{
		return !IsThereYourFigure(indexInFieldTryPutFigure);
	}
	return false;
}

bool ALudoGameState::CanGetHome(const uint8 FieldIndexFrom, const uint8 indexTryPutFigure, uint8& OutHomeIndex) const
{
	OutHomeIndex = UINT8_MAX;
	uint8 lastFieldIndex = UINT8_MAX;
	uint8 startFieldIndex = GetStartFieldIndex();
	if (startFieldIndex == 0)
	{
		lastFieldIndex = GetNumberOfFields() - 1;
		startFieldIndex = GetNumberOfFields();
	}
	else
	{
		lastFieldIndex = startFieldIndex - 1;
	}

	if (!HasOverflowForColor(FieldIndexFrom, indexTryPutFigure))
	{
		return false;
	}

	uint8 homeIndex = indexTryPutFigure % lastFieldIndex - 1;
	//Player roll more than there are homes
	if (homeIndex > NumberOfFigures)
	{
		return false;
	}

	OutHomeIndex = GetColorIndex(homeIndex);

	return true;
}

bool ALudoGameState::CanMoveAtHome(const uint8 InstanceIndex, uint8& OutNewHomeIndexPos, const uint8 homeIndex) const
{
	OutNewHomeIndexPos = UINT8_MAX;
	const uint8 homeIndexPosition = homeIndex != UINT8_MAX ? homeIndex : GetLudoFigureFieldIndex(InstanceIndex);
	const uint8 NewHomeIndexPos = homeIndexPosition + NumberOnDice;
	if (NewHomeIndexPos < FiguresAtHome.Num() && NewHomeIndexPos < HomeFieldsPositions.Num())
	{
		const bool bIsInHomeRange = homeIndexPosition % NumberOfFigures + NumberOnDice < NumberOfFigures;
		if (bIsInHomeRange)
		{
			const bool bIsNotAtLastHomePosition = homeIndexPosition % NumberOfFigures < NumberOfFigures - 1;
			if (bIsNotAtLastHomePosition)
			{
				const bool bCanMoveThere = FiguresAtHome[NewHomeIndexPos] == UINT8_MAX;
				if (bCanMoveThere)
				{
					OutNewHomeIndexPos = NewHomeIndexPos;					
					return true;
				}
			}
		}
	}

	return false;
}

bool ALudoGameState::IsThereYourFigure(const uint8 indexTryPutFigure)
{
	const bool bIsThereFigure = FiguresInField[indexTryPutFigure] != UINT8_MAX;
	const bool bIsThereYourFigure = bIsThereFigure && GetCurrentPlayer()->LudoPlayerID == GetLudoFigureColor(FiguresInField[indexTryPutFigure]);
	if (bIsThereYourFigure)
	{
		GetCurrentPlayer()->UpdatePlayerUI(FPlayerUIMessage(0, TI_THERE_IS_YOUR_FIGURE));
	}
	return bIsThereYourFigure;
}

bool ALudoGameState::AreFiguresInHangar() const
{
	for (size_t i = 0; i < NumberOfFigures; i++)
	{
		const uint8 ColorIndex = GetColorIndex(i);
		bool bIsOnNeutralField = GetLudoFigureFieldType(ColorIndex) == (uint8)EFieldType::Neutral;
		bool bIsOnStartField = GetLudoFigureFieldType(ColorIndex) == (uint8)EFieldType::Start;

		if (bIsOnNeutralField || bIsOnStartField)
		{
			return false;
		}
	}

	return true;
}

bool ALudoGameState::MustPutOnFigure() const
{	
	bool bIsInHangar = false;
	bool bIsOnNeutralField = false;
	for (size_t i = 0; i < NumberOfFigures; i++)
	{
		const uint8 ColorIndex = GetColorIndex(i);
		const uint8 FieldTypeLoc = GetLudoFigureFieldType(ColorIndex);
		if (FieldTypeLoc == (uint8)EFieldType::Hangar)
		{
			bIsInHangar = true;
		}
		if (FieldTypeLoc == (uint8)EFieldType::Neutral)
		{
			bIsOnNeutralField = true;
		}

		if (FieldTypeLoc == (uint8)EFieldType::Start)
		{
			return false;
		}
	}

	return bIsInHangar && NumberOnDice == MAX_DICE_NUMBER && bIsOnNeutralField;
}

bool ALudoGameState::CanRoll() const
{
	uint8 maxRollsLoc;
	if (!bHasMovedAtHome && !bHasEnteredToHome && AreFiguresInHangar())
	{
		maxRollsLoc = MaxRolls;
	}
	else
	{
		maxRollsLoc = 1;
	}

	return CurrentRoll < maxRollsLoc;
}

void ALudoGameState::UpdatePlayersUI_Implementation(FPlayerUIMessage Paylod)
{	
	for (size_t i = 0; i < PlayerArray.Num(); i++)
	{
		if (ALudoPlayerState* LudoPlayerState = Cast<ALudoPlayerState>(PlayerArray[i]))
		{
			if (LudoPlayerState->GetPlayerController() && LudoPlayerState->GetPlayerController()->IsLocalPlayerController())
			{
				Paylod.bIsOnMove = CurrentPlayerID == LudoPlayerState->ServerPlayerID;
				LudoPlayerState->UpdatePlayerUI(Paylod);
			}			
		}
	}
}

void ALudoGameState::ChangeMoveForAnotherPlayer()
{		
	GetCurrentPlayer()->UpdatePlayerUI(FPlayerUIMessage(false, 0, TI_FIRST_ENTER));
	CurrentPlayerID = (CurrentPlayerID + 1) % PlayerArray.Num();

	CurrentRoll = 0; 
	NumberOnDice = 0;
	bHasEnteredToHome = false;
	bHasMovedAtHome = false;
	bFirstEnter = true;
	GetCurrentPlayer()->UpdatePlayerUI(FPlayerUIMessage(true, 0, TI_FIRST_ENTER));
	LudoCurrentState = EGameState::RollDice;
}

void ALudoGameState::OnRep_CurrentPlayerID()
{
	UpdatePlayersUI(FPlayerUIMessage(false, 0, TI_FIRST_ENTER));
}

void ALudoGameState::OnRep_NumberOnDice()
{
	if (NumberOnDice == 0)
	{
		UpdatePlayersUI(FPlayerUIMessage(0, TI_UPDATE_ROLL));
	}
	else
	{
		UpdatePlayersUI(FPlayerUIMessage(NumberOnDice));
	}
}

void ALudoGameState::OnRep_PauseState()
{
	if (bPauseState)
	{
		UpdatePlayersUI(FPlayerUIMessage(0, TI_CLIENT_LOGOUT));
	}
	else
	{
		UpdatePlayersUI(FPlayerUIMessage((uint8)LudoCurrentState, TI_CLIENT_REJOIN));
	}
}

void ALudoGameState::UpdateFiguresPositions_Implementation(const int32 instanceIndex, const FVector NewPosition, const FFigureInfo NewFigureInfo)
{
	FTransform NewTransform;
	NewTransform.SetLocation(NewPosition);
	FigureActor->SetFigureInfo(instanceIndex, NewFigureInfo);
	FigureActor->ISMComp->UpdateInstanceTransform(instanceIndex, NewTransform, true, true, true);
}

void ALudoGameState::Multicast_ShowFigures_Implementation(const int32 instanceIndex, const FVector NewLocation, const FFigureInfo NewFigureInfo)
{
	UpdateFiguresPositions(instanceIndex, NewLocation, NewFigureInfo);
}

bool ALudoGameState::TryPutFigureOnStart(const uint8 InstanceIndex)
{
	if (NumberOnDice == MAX_DICE_NUMBER && !IsThereYourFigure(GetStartFieldIndex()))
	{
		const uint8 StartFieldIndex = GetStartFieldIndex();
		int32 hangarIndex = OffsetBetweenHangarAndFieldTiles * (InstanceIndex / NumberOfFigures) + InstanceIndex;
		FVector FigurePositionInHangar = GetLudoFieldPosition(hangarIndex);
		ensureMsgf(StartFieldIndex < FieldsPositions.Num(), TEXT("Start field index has overflowed!"));

		//Check if there is an enemy figure
		ReturnFigureToHangar(StartFieldIndex);

		FigurePositionInHangar.Z = FigureZPos;
		FiguresInHangar.Emplace(FigurePositionInHangar) = UINT8_MAX;
		ensureMsgf(FiguresInHangar.Num() <= NumberOfFigures * (uint8)PlayerMode, TEXT("Invalid position for figures in hangar!"));
		FiguresInField[StartFieldIndex] = InstanceIndex;

		UE_LOG(LogTemp, Warning, TEXT("Color %d. Move figure %d from Hangar to start %d."), GetLudoFigureColor(InstanceIndex), InstanceIndex, StartFieldIndex);

		Multicast_ShowFigures(InstanceIndex, FieldsPositions[StartFieldIndex], FFigureInfo(EFieldType::Start, StartFieldIndex));
		return true;
	}
	else 
	{
		GetCurrentPlayer()->UpdatePlayerUI(FPlayerUIMessage(0, TI_CANT_PUT_ON_START));		
	}

	return false;
}
bool ALudoGameState::TryMoveFigureAtHome(const uint8 InstanceIndex)
{
	const uint8 homeIndexPosition = GetLudoFigureFieldIndex(InstanceIndex);
	uint8 NewHomeIndexPos;
	if (CanMoveAtHome(InstanceIndex, NewHomeIndexPos))
	{
		FiguresAtHome[NewHomeIndexPos] = InstanceIndex;
		FiguresAtHome[homeIndexPosition] = UINT8_MAX;

		//UE_LOG(LogTemp, Warning, TEXT("Color %d. Move figure %d from %d to %d."), GetLudoFigureColor(InstanceIndex), InstanceIndex, homeIndexPosition, NewHomeIndexPos);
		Multicast_ShowFigures(InstanceIndex, HomeFieldsPositions[NewHomeIndexPos], FFigureInfo(EFieldType::Home, NewHomeIndexPos));

		bHasMovedAtHome = true;
		return bHasMovedAtHome;
	}

	return false;
}
bool ALudoGameState::TryToMoveFromFieldToHome(const uint8 InstanceIndex)
{
	bHasEnteredToHome = false;
	uint8 HomeIndex;
	const uint8 FieldIndexFrom = GetLudoFigureFieldIndex(InstanceIndex);
	const uint8 indexInFieldTryPutFigure = FieldIndexFrom + NumberOnDice;
	if (CanGetHome(FieldIndexFrom, indexInFieldTryPutFigure, HomeIndex))
	{
		const bool bIsThereFigure = FiguresAtHome[HomeIndex] != UINT8_MAX;
		//If there is a figure at your move, you can not move there
		if (bIsThereFigure)
		{
			return false;
		}

		ensureMsgf(HomeIndex < HomeFieldsPositions.Num(), TEXT("Wrong index for HomeFieldsPositions!"));
		if (HomeIndex > HomeFieldsPositions.Num())
		{
			return false;
		}

		FiguresInField[FieldIndexFrom] = UINT8_MAX;
		FiguresAtHome[HomeIndex] = InstanceIndex;
		Multicast_ShowFigures(InstanceIndex, HomeFieldsPositions[HomeIndex], FFigureInfo(EFieldType::Home, HomeIndex));

		bFirstEnter = false;

		UE_LOG(LogTemp, Warning, TEXT("Color %d. Move figure %d from field %d to %d home."), GetLudoFigureColor(InstanceIndex), InstanceIndex, FieldIndexFrom, HomeIndex);
		bool bHasWon = CheckWinGameCondition();

		if (bHasWon)
		{
			return false;
		}

		bHasEnteredToHome = true;
		return bHasEnteredToHome;
	}

	return bHasEnteredToHome;
}
bool ALudoGameState::TryMoveInField(const uint8 InstanceIndex)
{
	const uint8 FieldIndexFrom = GetLudoFigureFieldIndex(InstanceIndex);
	const uint8 indexToMove = FieldIndexFrom + NumberOnDice;

	//Is the index bigger than start field -> prevent to own color to run the board again
	if (HasOverflowForColor(FieldIndexFrom, indexToMove))
	{
		return false;
	}

	const uint8 indexInFieldTryPutFigure = indexToMove % GetNumberOfFields();
	if (IsThereYourFigure(indexInFieldTryPutFigure))
	{
		return false;
	}

	ensureMsgf(indexInFieldTryPutFigure < GetNumberOfFields(), TEXT("Index for FieldsPositions has been overflowed!"));
	if (indexInFieldTryPutFigure >= GetNumberOfFields())
	{
		return false;
	}

	ReturnFigureToHangar(indexInFieldTryPutFigure);

	FiguresInField[FieldIndexFrom] = UINT8_MAX;
	FiguresInField[indexInFieldTryPutFigure] = InstanceIndex;

	//UE_LOG(LogTemp, Warning, TEXT("Color %d. Move figure %d from field %d to %d field."), GetLudoFigureColor(InstanceIndex), InstanceIndex, GetLudoFigureFieldIndex(InstanceIndex), indexInFieldTryPutFigure);
	Multicast_ShowFigures(InstanceIndex, FieldsPositions[indexInFieldTryPutFigure], FFigureInfo(EFieldType::Neutral, indexInFieldTryPutFigure));
	return true;
}

void ALudoGameState::TryPutEnemyToHangar(const uint8 indexField)
{
	if (indexField < FiguresInField.Num())
	{
		const uint8 figureIndex = FiguresInField[indexField];

		for (size_t i = 0; i < PlayerArray.Num(); i++)
		{
			if (ALudoPlayerState* LudoPlayerState = Cast<ALudoPlayerState>(PlayerArray[i]))
			{
				const uint8 FigureColor = GetLudoFigureColor(figureIndex);				
				if (LudoPlayerState->LudoPlayerID == FigureColor)
				{
					for (uint8 indexHangar = 0; indexHangar < NumberOfFigures; indexHangar++)
					{
						const uint8 ColorIndex = FigureColor * NumberOfFigures + indexHangar;
						int32 instanceIndex = OffsetBetweenHangarAndFieldTiles * (ColorIndex / NumberOfFigures) + ColorIndex;
						//Get hangar position		
						FVector FigurePositionInHangar = GetLudoFieldPosition(instanceIndex);
						FigurePositionInHangar.Z = FigureZPos;

						//Place figure to empty hangar
						const uint8 figureIndexInHangar = *FiguresInHangar.Find(FigurePositionInHangar);
						//UE_LOG(LogTemp, Warning, TEXT("Placing figure to hangar %d."), figureIndexInHangar);
						if (figureIndexInHangar == UINT8_MAX)
						{
							FiguresInHangar.Emplace(FigurePositionInHangar) = figureIndex;
							ensureMsgf(FiguresInHangar.Num() <= NumberOfFigures * (uint8)PlayerMode, TEXT("Invalid position for figures in hangar!"));

							UE_LOG(LogTemp, Warning, TEXT("Color %d. Move figure %d from field %d to hangar."), FigureColor, figureIndex, GetLudoFigureFieldIndex(figureIndex));
							Multicast_ShowFigures(figureIndex, FigurePositionInHangar, FFigureInfo(EFieldType::Hangar, UINT8_MAX));

							break;
						}
					}
					break;
				}
			}
		}
	}	
}
void ALudoGameState::ReturnFigureToHangar(const uint8 indexField)
{
	const bool bIsThereFigure = FiguresInField[indexField] != UINT8_MAX;
	const bool bIsThereEnemy = bIsThereFigure && GetCurrentPlayer()->LudoPlayerID != GetLudoFigureColor(FiguresInField[indexField]);
	//UE_LOG(LogTemp, Warning, TEXT("Try return figure to hangar. Is there figure %d. Is your figure %d "), FiguresInField[indexField], bIsThereEnemy);
	if (bIsThereEnemy)
	{
		TryPutEnemyToHangar(indexField);
	}
}
bool ALudoGameState::CheckWinGameCondition()
{
	for (uint8 i = 0; i < NumberOfFigures; i++)
	{
		const uint8 homeIndex = GetColorIndex(i);
		//check if there is not a figure at your home
		if (homeIndex < FiguresAtHome.Num() && FiguresAtHome[homeIndex] == UINT8_MAX)
		{
			return false;
		}
	}

	UpdatePlayersUI(FPlayerUIMessage(0, TI_WIN));
	//UpdatePlayersUIs(TI_WIN);
	UE_LOG(LogTemp, Warning, TEXT("The Player %d win the game!"), GetCurrentPlayer()->LudoPlayerID);

	Multicast_CleanBoard();

	return true;
}

#pragma endregion

#pragma region Helpers
void ALudoGameState::Multicast_CleanBoard_Implementation()
{
	CleanBoard();
}
void ALudoGameState::CleanBoard_Implementation()
{
	/*if (FigureActor)
	{
		FigureActor->Destroy();
	}

	if (FieldTileActor)
	{
		FieldTileActor->Destroy();
	}*/
}
EColors ALudoGameState::StringToEColor(FString InString) const
{
	/*if (InString.Contains(ColorBlue))
	{
		return EColors::Blue;
	}
	else if (InString.Contains(ColorRed))
	{
		return EColors::Red;
	}
	else  if (InString.Contains(ColorGreen))
	{
		return EColors::Green;
	}
	else if (InString.Contains(ColorYellow))
	{
		return EColors::Yellow;
	}*/

	return EColors::White;
}

FString ALudoGameState::EColorToStirng(EColors InColor) const
{
	/*switch (InColor)
	{
	case EColors::Blue:
		return ColorBlue;
	case EColors::Red:
		return ColorRed;
	case EColors::Green:
		return ColorGreen;
	case EColors::Yellow:
		return ColorYellow;
	}*/

	return FString();
}

ALudoPlayerState* ALudoGameState::GetCurrentPlayer() const
{
	return Cast<ALudoPlayerState>(PlayerArray[CurrentPlayerID]);
}

uint8 ALudoGameState::GetFigureIndex(const uint8 index) const
{
	return GetCurrentPlayer()->LudoPlayerID * index + index;
}

uint8 ALudoGameState::GetStartFieldIndex() const
{
	return GetCurrentPlayer()->LudoPlayerID * DistanceBetweenStartsFields;
}

uint8 ALudoGameState::GetNumberOfFields() const
{
	return (uint8)PlayerMode * DistanceBetweenStartsFields;
}

uint8 ALudoGameState::GetLudoFigureFieldIndex(const uint8 index) const
{
	return FigureActor->ISMComp->PerInstanceSMCustomData[index * FigureActor->ISMComp->NumCustomDataFloats + 1];
}

uint8 ALudoGameState::GetLudoFigureFieldType(const uint8 index) const
{
	return FigureActor->ISMComp->PerInstanceSMCustomData[index * FigureActor->ISMComp->NumCustomDataFloats];
}

uint8 ALudoGameState::GetLudoFigureColor(const uint8 index) const
{
	return FigureActor->ISMComp->PerInstanceSMCustomData[index * FigureActor->ISMComp->NumCustomDataFloats + 2];
}

uint8 ALudoGameState::GetColorIndex(const uint8 index) const
{
	return GetCurrentPlayer()->LudoPlayerID * NumberOfFigures + index;
}

bool ALudoGameState::HasOverflowForColor(const uint8 FieldIndexFrom, const uint8 FieldIndexTo) const
{
	uint8 border;
	if (GetStartFieldIndex() == 0)
	{
		border = GetNumberOfFields();
	}
	else
	{
		border = GetStartFieldIndex();
	}
	return FieldIndexFrom < border && FieldIndexTo >= border;
}

FVector ALudoGameState::GetLudoFieldPosition(const int32 instanceIndex) const
{
	FTransform LudoFieldTransform;
	FieldTileActor->ISMComp->GetInstanceTransform(instanceIndex, LudoFieldTransform);
	return LudoFieldTransform.GetLocation();
}
#pragma endregion
