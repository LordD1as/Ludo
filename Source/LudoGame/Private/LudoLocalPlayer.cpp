// Fill out your copyright notice in the Description page of Project Settings.


#include "LudoLocalPlayer.h"
#include "LudoGameInstance.h"

#define MAX_PLAYER_NAME_LENGTH 10

void ULudoLocalPlayer::SetControllerId(int32 NewControllerId)
{
	ULocalPlayer::SetControllerId(NewControllerId);
}

FString ULudoLocalPlayer::GetNickname() const
{
	FString UserNickName = Super::GetNickname();

	if (UserNickName.Len() > MAX_PLAYER_NAME_LENGTH)
	{
		UserNickName = UserNickName.Left(MAX_PLAYER_NAME_LENGTH);
	}

	bool bReplace = (UserNickName.Len() == 0);

	// Check for duplicate nicknames...and prevent reentry
	static bool bReentry = false;
	if (!bReentry)
	{
		bReentry = true;
		ULudoGameInstance* GameInstance = GetWorld() != NULL ? Cast<ULudoGameInstance>(GetWorld()->GetGameInstance()) : NULL;
		if (GameInstance)
		{
			// Check all the names that occur before ours that are the same
			const TArray<ULocalPlayer*>& LocalPlayers = GameInstance->GetLocalPlayers();
			for (int i = 0; i < LocalPlayers.Num(); ++i)
			{
				const ULocalPlayer* LocalPlayer = LocalPlayers[i];
				if (this == LocalPlayer)
				{
					break;
				}

				if (UserNickName == LocalPlayer->GetNickname())
				{
					bReplace = true;
					break;
				}
			}
		}
		bReentry = false;
	}

	if (bReplace)
	{
		UserNickName = FString::Printf(TEXT("Player%i"), GetControllerId() + 1);
	}

	return UserNickName;
}
