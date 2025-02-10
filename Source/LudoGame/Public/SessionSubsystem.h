// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystemUtils.h"
#include "FindSessionsCallbackProxy.h"
#include "SessionSubsystem.generated.h"

UENUM(BlueprintType)
enum EJoinResultType
{
	/** The join worked as expected */
	Success,
	/** There are no open slots to join */
	SessionIsFull,
	/** The session couldn't be found on the service */
	SessionDoesNotExist,
	/** There was an error getting the session server's address */
	CouldNotRetrieveAddress,
	/** The user attempting to join is already a member of the session */
	AlreadyInSession,
	/** An error not covered above occurred */
	UnknownError
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSOnCreatSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSOnUpdateSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSOnStartSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSOnEndSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCSOnDestroySessionComplete, bool, Successful);

/*due to “FOnlineSessionSearchResult” not being a USTRUCT, we can use a DYNAMIC Multicast Delegate, as those need to have types that can be exposed to Blueprints.
 *So if you want to expose this Delegate to Blueprints, you need to wrap the SearchResult struct into your own USTRUCT and utilize some Function Library
 *to communicate with the inner SearchResult struct*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCSOnFindSessionsComplete, const TArray<FBlueprintSessionResult>&, SessionResults, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCSOnJoinSessionComplete, EJoinResultType, JoinResult, FString, Address);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCSOnSessionFailureComplete, const FUniqueNetId&, PlayerId, ESessionFailure::Type, FailureType);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCSOnFindFriendSessionComplete, int32, LocalUserNum, bool, bWasSuccessful, const TArray<FBlueprintSessionResult>&, SessionResults);

/**
 *
 */
UCLASS()
class LUDOGAME_API USessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USessionSubsystem();

	UFUNCTION(BlueprintCallable)
		void CreateSession(int32 NumPublicConnections, bool IsLanMatch/*, bool& IsCreated*/, FString Nickname);
	void UpdateSession();
	void StartSession();
	void EndSession();
	UFUNCTION(BlueprintCallable)
		void DestroySession();
	UFUNCTION(BlueprintCallable)
		void FindSessions(int32 MaxSearchResults, bool IsLANQuery);
	void FindFriendSession(const FUniqueNetId& UserId);

	//void OnSessionFailureInit();

	/*“FOnlineSessionSearchResult” struct can’t be exposed.
	 *If you want to make the function BlueprintCallable, you’ll have to wrap the struct into your own USTRUCT*/
	UFUNCTION(BlueprintCallable)
		void JoinGameSession(const FBlueprintSessionResult& SessionSearchResult);

	UPROPERTY(BlueprintAssignable)
		FCSOnCreatSessionComplete OnCreateSessionCompleteEvent;
	FCSOnUpdateSessionComplete OnUpdateSessionCompleteEvent;
	FCSOnStartSessionComplete OnStartSessionCompleteEvent;
	FCSOnEndSessionComplete OnEndSessionCompleteEvent;
	UPROPERTY(BlueprintAssignable)
		FCSOnDestroySessionComplete OnDestroySessionCompleteEvent;

	UPROPERTY(BlueprintAssignable)
		FCSOnFindSessionsComplete OnFindSessionsCompleteEvent;
	UPROPERTY(BlueprintAssignable)
		FCSOnJoinSessionComplete OnJoinGameSessionCompleteEvent;
	UPROPERTY(BlueprintAssignable)
		FCSOnFindFriendSessionComplete OnFindFriendSessionCompleteEvent;

	//FCSOnSessionFailureComplete OnSessionFailureCompleteEvent;

protected:
	void OnCreateSessionCompleted(FName SessionName, bool Successful);
	void OnUpdateSessionCompleted(FName SessionName, bool Successful);
	void OnStartSessionCompleted(FName SessionName, bool Successful);
	void OnEndSessionCompleted(FName SessionName, bool Successful);
	void OnDestroySessionCompleted(FName SessionName, bool Successful);
	void OnFindSessionsCompleted(bool Successful);
	void OnFindFriendSessionCompleted(int32 LocalUserNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SessionResults);
	//void OnSessionFailureCompleted(const FUniqueNetId& PlayerId, ESessionFailure::Type FailureType);

	/*“EOnJoinSessionCompleteResult” can not be exposed to Blueprints, so you can’t make your own Callback Delegate BlueprintAssignable
	 *(or Dynamic to begin with) unless you make your own UENUM and convert back and forth between UE4s type and yours.*/
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult);

	bool TryTravelToCurrentSession();

private:
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;

	FOnUpdateSessionCompleteDelegate UpdateSessionCompleteDelegate;
	FDelegateHandle UpdateSessionCompleteDelegateHandle;

	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	FOnEndSessionCompleteDelegate EndSessionCompleteDelegate;
	FDelegateHandle EndSessionCompleteDelegateHandle;

	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;

	//FOnSessionFailureDelegate OnSessionFailureDelegate;
	//FDelegateHandle OnSessionFailureDelegateHandle;

	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

	FOnFindFriendSessionCompleteDelegate FindFriendSessionCompleteDelegate;
	FDelegateHandle FindFriendSessionDelegateHandle;

	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	static EJoinResultType GetJoinResult(EOnJoinSessionCompleteResult::Type JoinResult);
	static TArray<FBlueprintSessionResult> GetBPSessionResults(const TArray<FOnlineSessionSearchResult>& OnlineResults);
};