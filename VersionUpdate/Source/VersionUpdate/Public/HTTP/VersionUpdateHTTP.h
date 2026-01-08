
#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "Runtime/Launch/Resources/Version.h"

DECLARE_DELEGATE_ThreeParams(FHttpUHRequestProgressDelegate, FHttpRequestPtr /*Request*/, int32 /*TotalBytes*/, int32 /*BytesReceived*/)

namespace VersionUpdateHTTP
{
	struct FHTTPDelegate
	{
		FHttpRequestCompleteDelegate						HttpCompleteDelegate;
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
		FHttpRequestProgressDelegate64						HttpSingleRequestProgressDelegate;
#else
		FHttpRequestProgressDelegate						HttpSingleRequestProgressDelegate;
#endif
		FHttpRequestHeaderReceivedDelegate					HttpSingleRequestHeaderReceivedDelegate;
		FSimpleDelegate										AllTasksCompletedDelegate;
	};

	struct FHTTP
	{
		FHTTP(FHTTPDelegate InDelegate);
		static FHTTP* CreateHTTPObject(FHTTPDelegate InDelegate);
		bool GetObjectToMemory(const FString& URL);
		void GetObjectsToMemory(const TArray<FString>& URL);

	private:
		void OnRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
		void OnRequestProgress(FHttpRequestPtr HttpRequest, int32 SendBytes, int32 BytesReceived);
		void OnRequestProgress64(FHttpRequestPtr HttpRequest, uint64 SendBytes, uint64 BytesReceived);
		void OnRequestHeaderReceived(FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue);
		void OnRequestsComplete();
	private:
		void AddTaskCount();
		bool CheckCompleteCount();
	private:
		FHTTPDelegate Delegate;
		int32 CompleteCount;
		int32 TaskCount;

		TMap<FName, int32> DataSizeInfoMap;
	};
}