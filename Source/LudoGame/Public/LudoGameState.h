// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Events/PlayerUIMessage.h"
#include "LudoGameState.generated.h"

class AFigure;

UENUM(BlueprintType)
enum class EPlayerMode : uint8
{
	None,
	Players4 = 4,
	Players6 = 6
};

UENUM(BlueprintType)
enum class EColors : uint8
{
	None = 0xfeui8,
	White  = UINT8_MAX,
	Blue    = 0,
	Red    = 1,
	Green = 2,
	Yellow = 3
};

ENUM_RANGE_BY_FIRST_AND_LAST(EColors, EColors::Blue, EColors::Yellow);

UENUM(BlueprintType)
enum class EFieldType : uint8
{
	None   = UINT8_MAX,
	Neutral = 0, /*Other fields.*/
	Home   = 1, /*Where player tries to get in in order to win the game.*/
	Start     = 2, /*Where a figure is spawned from Hangar field.*/
	Hangar = 3 /*Where player starts with figures new game.*/
};

UENUM(BlueprintType)
enum class EGameState : uint8
{
	None = 0,
	RollDice = 7,
	ChooseFigure = 8
};

USTRUCT()
struct FFigureInfo
{
	GENERATED_BODY()

	FFigureInfo() {};
	FFigureInfo(EFieldType InFieldType, uint8 InFigureIndexOnField) : FieldType(InFieldType), FigureIndexOnField(InFigureIndexOnField) {};

	EFieldType FieldType;
	uint8 FigureIndexOnField;
};

class AFieldTile;
class AFigure;
class ALudoPlayerState;

#define MAX_DICE_NUMBER 6

#define TI_CANT_MOVE_ANY_FIGURE_AND_PLAYER_IS_ON_MOVE  UINT8_MAX
#define TI_UPDATE_ROLL																	0xfeui8
#define TI_MUST_PUT_ON																	0xfdui8
#define TI_THERE_IS_YOUR_FIGURE													0xfcui8
#define TI_YOU_CAN_TOUCH_ONLY_YOUR_FIGURE							0xfbui8
#define TI_CANT_PUT_ON_START														0xfaui8
#define TI_WIN																					0xf9ui8
#define TI_CHOOSE_YOUR_FIGURE														0xf8ui8
#define TI_FIRST_ENTER																		0xf7ui8
#define TI_CLIENT_LOGOUT																0xf6ui8
#define TI_CLIENT_REJOIN																    0xf5ui8

/**
 * 
 */
UCLASS()
class LUDOGAME_API ALudoGameState : public AGameState
{
	GENERATED_BODY()

public:
	UFUNCTION(NetMulticast, Reliable)
	void OnUpdateLobbyUI();

#pragma region Pre game state
	UFUNCTION(NetMulticast, Reliable)
	void OnStartLudoGame();

	UFUNCTION(Client, Reliable)
	void SetOwnerForPCs();

	UFUNCTION(Server, Reliable)
	void	Server_SetLudoMatchState();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 MAX_PLAYERS = 4;

	bool HasAnybodySameColor(FString InColor);
	bool AreAllReady();	
#pragma endregion

#pragma region Gameplay
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_RollDice();

	UFUNCTION(Server, Reliable)
	void Server_TryMoveFigure(const uint8 InstanceIndex);

	UFUNCTION(Server, Reliable)
	void Server_SetPauseState(const bool bNewPauseState);

	/* 0 - 3 = B,R,G,Y. Only on server. CurrentIndex in PlayerArray. */
	UPROPERTY(ReplicatedUsing=OnRep_CurrentPlayerID)
	uint8 CurrentPlayerID = UINT8_MAX;

	UPROPERTY(ReplicatedUsing=OnRep_NumberOnDice, EditAnywhere, BlueprintReadWrite)
	uint8 NumberOnDice;
#pragma endregion

	UFUNCTION(BlueprintCallable)
	ALudoPlayerState* GetCurrentPlayer() const;

protected:
#pragma region Pre game state
	virtual void HandleMatchHasStarted() override;
#pragma endregion

virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/* On all clients you have access to positions of fields. You only repliacete playerID and based on that calculates other things. */
#pragma region Gameplay
	UPROPERTY()
	AFigure* FigureActor;

	UPROPERTY()
	AFieldTile* FieldTileActor;

	/* 255 means there is no figure. */
	UPROPERTY()
	TMap<FVector, uint8> FiguresInHangar;	

	UPROPERTY()
	TArray<uint8> FiguresAtHome;

	UPROPERTY()
	TArray<uint8> FiguresInField;

	/*  0 - 39 indexes based on 4 players. */
	UPROPERTY()
	TArray<FVector> FieldsPositions;

	UPROPERTY()
	TArray<FVector> HomeFieldsPositions;

	UPROPERTY()
	EPlayerMode PlayerMode = EPlayerMode::Players4;

	UPROPERTY()
	uint8 CurrentRoll;	

	bool CanMoveAnyFigure();

	bool CanMoveFigure(const uint8 LudoFigureFieldIndex, const uint8 LudoFigureFieldType);

	/* Check if after roll can player go home. If can return home index. */
	bool CanGetHome(const uint8 FieldIndexFrom, const uint8 indexTryPutFigure, uint8& OutHomeIndex) const;

	bool CanMoveAtHome(const uint8 InstanceIndex, uint8& OutNewHomeIndexPos, const uint8 homeIndex = UINT8_MAX) const;

	bool IsThereYourFigure(const uint8 indexTryPutFigure);

	/* Return false when there is some figure on a field. */
	bool AreFiguresInHangar() const;

	bool MustPutOnFigure() const;

	bool CanRoll() const;

	UFUNCTION(Client,Reliable)
	void UpdatePlayersUI(FPlayerUIMessage Paylod);

	/* Change current index of player and update UI. */
	void ChangeMoveForAnotherPlayer();

	UFUNCTION()
	void OnRep_CurrentPlayerID();

	UFUNCTION()
	void OnRep_NumberOnDice();

	UFUNCTION()
	void OnRep_PauseState();

	UFUNCTION(Client, Reliable)
	void UpdateFiguresPositions(const int32 instanceIndex, const FVector NewPosition, const FFigureInfo NewFigureInfo);

	/* Try put figure on start. */
	bool TryPutFigureOnStart(const uint8 InstanceIndex);

	bool TryMoveFigureAtHome(const uint8 InstanceIndex);

	bool TryToMoveFromFieldToHome(const uint8 InstanceIndex);

	bool TryMoveInField(const uint8 InstanceIndex);

	/* Check if there is an enemy and if it's there it will put his figure to hangar. */
	void TryPutEnemyToHangar(const uint8 indexField);

	/* Check if can retrun a figure to hangar. If can it will return in. */
	void ReturnFigureToHangar(const uint8 indexField);

	bool CheckWinGameCondition();

	bool bHasEnteredToHome = false;
	bool bHasMovedAtHome = false;

	UPROPERTY(ReplicatedUsing = OnRep_PauseState)
	bool bPauseState = false;

	//delete
	bool bFirstEnter = true;

	UPROPERTY(Replicated)
	EGameState LudoCurrentState = EGameState::RollDice;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_CleanBoard();

	UFUNCTION(Client, Reliable)
	void CleanBoard();
#pragma endregion

#pragma region Helprs
	/* DEPRACATED */
	EColors StringToEColor(FString InString) const;

	/* DEPRACATED */
	FString EColorToStirng(EColors InColor) const;

	FVector GetLudoFieldPosition(const int32 instanceIndex) const;

	uint8 GetFigureIndex(const uint8 index) const;

	uint8 GetStartFieldIndex() const;

	uint8 GetNumberOfFields() const;

	/* Return LudoFigueFieldIndex based on instance index. */
	uint8 GetLudoFigureFieldIndex(const uint8 index) const;

	/* Return LudoFigureFieldType based on instance index. */
	uint8 GetLudoFigureFieldType(const uint8 index) const;

	/* Return Color based on instance index. */
	uint8 GetLudoFigureColor(const uint8 index) const;

	uint8 GetColorIndex(const uint8 index) const;

	bool HasOverflowForColor(const uint8 FieldIndexFrom, const uint8 FieldIndexTo) const;
#pragma endregion
	
#pragma region Board Preparation
	void ShowDefaultWidget();

	/* Create and set material for fields, spawn figures and set start player. */
	void PrepareBoard();

	/* Find game element (fields and figures).	*/
	void SetGameElements();

	void PrepareFiguresInHangar();

	/* Fill hangars tmap(on server) based on current players' colors and show them to all clients. */
	void SetAndShowFiguresInHangar();

	/* */
	void MarkStartPlayer();

	void SetFieldsAndHomesPositions();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShowFigures(const int32 instanceIndex, const FVector NewLocation, const FFigureInfo NewFigureInfo);
#pragma endregion
};
