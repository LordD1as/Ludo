// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionSubsystem.h"
#include "OnlineSubsystemTypes.h"

#define SERVER_NAME FName(TEXT("ServerName"))

USessionSubsystem::USessionSubsystem()
	:
	//This line will bind the Subsystem function for the CreateSessionComplete Callback
	//to our Delegate.
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this,
		&ThisClass::OnCreateSessionCompleted)),
	UpdateSessionCompleteDelegate(FOnUpdateSessionCompleteDelegate::CreateUObject(this,
		&ThisClass::OnUpdateSessionCompleted)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this,
		&ThisClass::OnStartSessionCompleted)),
	EndSessionCompleteDelegate(FOnEndSessionCompleteDelegate::CreateUObject(this,
		&ThisClass::OnEndSessionCompleted)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this,
		&ThisClass::OnDestroySessionCompleted)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this,
		&ThisClass::OnFindSessionsCompleted)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this,
		&ThisClass::OnJoinSessionCompleted))/*,
	OnSessionFailureDelegate(FOnSessionFailureDelegate::CreateUObject(this,
		&ThisClass::OnSessionFailureCompleted)),*/
{
}

#pragma region CreateSession
void USessionSubsystem::CreateSession(int32 NumPublicConnections, bool IsLanMatch/*, bool& IsCreated*/, FString Nickname)
{
	//Use GetSessionIntefrace from OnlineSubsysbtemUtils.h not from Online.h
	//because the world can handle with multiply UWorlds and Online.h does not handle that
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		OnCreateSessionCompleteEvent.Broadcast(false);
		return;
	}

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->NumPrivateConnections = 0;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowInvites = true;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bIsDedicated = false;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bIsLANMatch = IsLanMatch;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->Set(SERVER_NAME, Nickname, EOnlineDataAdvertisementType::ViaOnlineService);

	LastSessionSettings->Set(SETTING_MAPNAME, FString("MainMenu"),
		EOnlineDataAdvertisementType::ViaOnlineService);

	//Add our Delegate to the SessionInterfaces Delegate List
	CreateSessionCompleteDelegateHandle = sessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	/*IsCreated = sessionInterface->CreateSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings);*/

	if (!sessionInterface->CreateSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings)/*IsCreated*/)
	{
		sessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

		OnCreateSessionCompleteEvent.Broadcast(false);
	}
}

void USessionSubsystem::OnCreateSessionCompleted(FName SessionName, bool Successful)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface)
	{
		sessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	OnCreateSessionCompleteEvent.Broadcast(Successful);
}
#pragma endregion

#pragma region UpdateSession
void USessionSubsystem::UpdateSession()
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		OnUpdateSessionCompleteEvent.Broadcast(false);
		return;
	}

	TSharedPtr<FOnlineSessionSettings> updatedSessionSettings = MakeShareable(new FOnlineSessionSettings(*LastSessionSettings));
	updatedSessionSettings->Set(SETTING_MAPNAME, FString("UpdatedLevelName"), EOnlineDataAdvertisementType::ViaOnlineService);

	UpdateSessionCompleteDelegateHandle = sessionInterface->AddOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegate);

	if (!sessionInterface->UpdateSession(NAME_GameSession, *updatedSessionSettings))
	{
		sessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);

		OnUpdateSessionCompleteEvent.Broadcast(false);
	}
	else
	{
		LastSessionSettings = updatedSessionSettings;
	}
}

void USessionSubsystem::OnUpdateSessionCompleted(FName SessionName, bool Successful)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface)
	{
		sessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(UpdateSessionCompleteDelegateHandle);
	}

	OnUpdateSessionCompleteEvent.Broadcast(Successful);
}
#pragma endregion 

#pragma region StartSession
void USessionSubsystem::StartSession()
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		OnStartSessionCompleteEvent.Broadcast(false);
		return;
	}

	StartSessionCompleteDelegateHandle = sessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);

	if (!sessionInterface->StartSession(NAME_GameSession))
	{
		sessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);

		OnStartSessionCompleteEvent.Broadcast(false);
	}
}

void USessionSubsystem::OnStartSessionCompleted(FName SessionName, bool Successful)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface)
	{
		sessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
	}

	OnStartSessionCompleteEvent.Broadcast(Successful);
}
#pragma endregion 

#pragma region EndSession
void USessionSubsystem::EndSession()
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		OnEndSessionCompleteEvent.Broadcast(false);
		return;
	}

	EndSessionCompleteDelegateHandle = sessionInterface->AddOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegate);

	if (!sessionInterface->EndSession(NAME_GameSession))
	{
		sessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegateHandle);

		OnEndSessionCompleteEvent.Broadcast(false);
	}
}

void USessionSubsystem::OnEndSessionCompleted(FName SessionName, bool Successful)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());;
	if (sessionInterface)
	{
		sessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegateHandle);
	}

	OnEndSessionCompleteEvent.Broadcast(Successful);
}
#pragma endregion 

#pragma region DestroySession
void USessionSubsystem::DestroySession()
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		OnDestroySessionCompleteEvent.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle = sessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!sessionInterface->DestroySession(NAME_GameSession))
	{
		sessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);

		OnDestroySessionCompleteEvent.Broadcast(false);
	}
}

void USessionSubsystem::OnDestroySessionCompleted(FName SessionName, bool Successful)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface)
	{
		sessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}

	OnDestroySessionCompleteEvent.Broadcast(Successful);
}
#pragma endregion 

#pragma region FindSessions
void USessionSubsystem::FindSessions(int32 MaxSearchResults, bool IsLANQuery)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		OnFindSessionsCompleteEvent.Broadcast(TArray<FBlueprintSessionResult>(), false);
		return;
	}

	FindSessionsCompleteDelegateHandle = sessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = IsLANQuery;

	//This will make sure that we are searching for Presence Sessions
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!sessionInterface->FindSessions(*localPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		sessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

		OnFindSessionsCompleteEvent.Broadcast(TArray<FBlueprintSessionResult>(), false);
	}
}

void USessionSubsystem::FindFriendSession(const FUniqueNetId& UserId)
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

	FindFriendSessionDelegateHandle = Session->AddOnFindFriendSessionCompleteDelegate_Handle(0, FindFriendSessionCompleteDelegate);
	
	if (!Session->FindFriendSession(0 /* LocalUserNum */, UserId))
	{
		// Call didn't start, return error.
		Session->ClearOnFindFriendSessionCompleteDelegate_Handle(0 /* LocalUserNum */, FindFriendSessionDelegateHandle);
	}
}

//void USessionSubsystem::OnSessionFailureInit()
//{
//	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
//	if (sessionInterface)
//	{
//		OnSessionFailureDelegateHandle = sessionInterface->AddOnSessionFailureDelegate_Handle(OnSessionFailureDelegate);
//	}
//}

void USessionSubsystem::OnFindSessionsCompleted(bool Successful)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());	
	if (sessionInterface)
	{
		sessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	if (LastSessionSearch->SearchResults.Num() <= 0)
	{
		OnFindSessionsCompleteEvent.Broadcast(TArray<FBlueprintSessionResult>(), Successful);
		return;
	}

	OnFindSessionsCompleteEvent.Broadcast(USessionSubsystem::GetBPSessionResults(LastSessionSearch->SearchResults), Successful);
}

void USessionSubsystem::OnFindFriendSessionCompleted(int32 LocalUserNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SessionResults)
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

	if (bWasSuccessful && SessionResults.Num() > 0)
	{
		// The user has a session they can reconnect to. You can handle the (at most one)
		// search result from the Results array the same way that search results
		// are handled in "Finding a session".

		Session->ClearOnFindFriendSessionCompleteDelegate_Handle(0, FindFriendSessionDelegateHandle);
		FindFriendSessionDelegateHandle.Reset();
		OnFindFriendSessionCompleteEvent.Broadcast(LocalUserNum, bWasSuccessful, GetBPSessionResults(SessionResults));
		return;
	}

	// Otherwise, the user does not have a session to reconnect to.
	Session->ClearOnFindFriendSessionCompleteDelegate_Handle(0, FindFriendSessionDelegateHandle);
	FindFriendSessionDelegateHandle.Reset();

	OnFindFriendSessionCompleteEvent.Broadcast(LocalUserNum, bWasSuccessful, TArray<FBlueprintSessionResult>());
}

//void USessionSubsystem::OnSessionFailureCompleted(const FUniqueNetId& PlayerId, ESessionFailure::Type FailureType)
//{
//	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
//	if (sessionInterface)
//	{	
//		sessionInterface->ClearOnSessionFailureDelegate_Handle(OnSessionFailureDelegateHandle);
//	}
//
//	OnSessionFailureCompleteEvent.Broadcast(PlayerId, FailureType);
//}

TArray<FBlueprintSessionResult> USessionSubsystem::GetBPSessionResults(const TArray<FOnlineSessionSearchResult>& OnlineResults)
{
	TArray<FBlueprintSessionResult> BPSessionResults;
	for (const FOnlineSessionSearchResult Result : OnlineResults)
	{
		FBlueprintSessionResult BPSessionResult;
		BPSessionResult.OnlineResult = Result;
		FString ServerName;
		Result.Session.SessionSettings.Get(SERVER_NAME, ServerName);
		if (!ServerName.IsEmpty())
		{
			BPSessionResult.OnlineResult.Session.OwningUserName = ServerName;
		}
		BPSessionResults.Add(BPSessionResult);
	}

	return BPSessionResults;
}

#pragma endregion

#pragma region JoinGameSession 
void USessionSubsystem::JoinGameSession(const FBlueprintSessionResult& SessionSearchResult)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		OnJoinGameSessionCompleteEvent.Broadcast(UnknownError, FString());
		return;
	}

	JoinSessionCompleteDelegateHandle = sessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!sessionInterface->JoinSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionSearchResult.OnlineResult))
	{
		sessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

		OnJoinGameSessionCompleteEvent.Broadcast(UnknownError, FString());
	}
}

void USessionSubsystem::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface)
	{
		sessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	FString IPaddr;

	sessionInterface->GetResolvedConnectString(NAME_GameSession, IPaddr);
	OnJoinGameSessionCompleteEvent.Broadcast(GetJoinResult(JoinResult), IPaddr);
}

EJoinResultType USessionSubsystem::GetJoinResult(EOnJoinSessionCompleteResult::Type JoinResult)
{
	switch (JoinResult)
	{
	case EOnJoinSessionCompleteResult::Success:
		return Success;
	case EOnJoinSessionCompleteResult::SessionIsFull:
		return SessionIsFull;
	case EOnJoinSessionCompleteResult::SessionDoesNotExist:
		return SessionDoesNotExist;
	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
		return CouldNotRetrieveAddress;
	case EOnJoinSessionCompleteResult::AlreadyInSession:
		return AlreadyInSession;
	default: return UnknownError;
	}
}

bool USessionSubsystem::TryTravelToCurrentSession()
{

	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		return false;
	}

	FString connectString;
	if (!sessionInterface->GetResolvedConnectString(NAME_GameSession, connectString))
	{
		return false;
	}

	APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	playerController->ClientTravel(connectString, TRAVEL_Absolute);
	return true;
}
#pragma endregion 
