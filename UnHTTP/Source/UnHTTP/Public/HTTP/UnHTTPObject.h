

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "UnHTTPType.h"
#include "Runtime/Launch/Resources/Version.h"

namespace UnHTTPObject
{
	struct UNHTTP_API FHTTPDelegate
	{
		FHttpRequestCompleteDelegate						SingleCompleteDelegate;
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
		FHttpRequestProgressDelegate64						SingleRequestProgressDelegate;
#else
		FHttpRequestProgressDelegate						SingleRequestProgressDelegate;
#endif
		FHttpRequestHeaderReceivedDelegate					SingleRequestHeaderReceivedDelegate;
	};

	struct UNHTTP_API FHTTP :public TSharedFromThis<FHTTP>
	{
		static TSharedPtr<FHTTP> CreateHTTPObject(FHTTPDelegate InDelegate);

	public:

		FHTTP(FHTTPDelegate InDelegate);

		//字符串
		FGuid Request(
			const FString& InURL,
			const FString& InContent,		
			const TMap<FString, FString>& InCustomMetadataHeader,
			bool bSynchronous = false,
			EUnHTTPVerbType VerbType = EUnHTTPVerbType::UN_POST);

		//数据类型
		FGuid Request(
			const FString& InURL,
			const TArray<uint8>& InBytes,
			const TMap<FString, FString>& InCustomMetadataHeader,
			bool bSynchronous = false,
			EUnHTTPVerbType VerbType = EUnHTTPVerbType::UN_POST);

		void CancelRequest(const FGuid& InRequestID);
		bool IsValidRequest(const FGuid& InRequestID) const;
		void SetOutTime(float InOutTime) { OutTime = InOutTime; }

	private:
		FGuid Request(
			const FString& InURL,
			const FString& InContent,
			const TArray<uint8>& InBytes,
			const TMap<FString, FString>& InCustomMetadataHeader,
			bool bSynchronous = false,
			EUnHTTPVerbType VerbType = EUnHTTPVerbType::UN_POST);

	private:
		void OnRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
		void OnRequestProgress64(FHttpRequestPtr HttpRequest, uint64 SendBytes, uint64 BytesReceived);
		void OnRequestProgress(FHttpRequestPtr HttpRequest, int32 SendBytes, int32 BytesReceived);
		void OnRequestHeaderReceived(FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue);

		struct FSimpleRequest
		{
			FSimpleRequest()
			{}

			TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request;
		};

	private:
		TMap<FGuid,FSimpleRequest> HttpRequests;
		UnHTTPObject::FHTTPDelegate Delegate;
		float OutTime;
	};
}