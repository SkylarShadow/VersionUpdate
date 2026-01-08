
#include "HTTP/VersionUpdateHTTP.h"

#if PLATFORM_WINDOWS
#pragma optimize("",off) 
#endif

#ifndef ENGINE_MAJOR_VERSION
#define ENGINE_MAJOR_VERSION 5
#endif // !ENGINE_MAJOR_VERSION

#ifndef ENGINE_MINOR_VERSION
#define ENGINE_MINOR_VERSION 2
#endif // !ENGINE_MAJOR_VERSION

#ifndef ENGINE_PATCH_VERSION
#define ENGINE_PATCH_VERSION 1
#endif // !ENGINE_MAJOR_VERSION

namespace VersionUpdateHTTP
{
	FHTTP::FHTTP(FHTTPDelegate InDelegate)
		:Delegate(InDelegate)
		, CompleteCount(0)
		, TaskCount(0)
	{
	}

	FHTTP* FHTTP::CreateHTTPObject(FHTTPDelegate InDelegate)
	{
		return new FHTTP(InDelegate);
	}

	bool FHTTP::GetObjectToMemory(const FString& URL)
	{
#if (ENGINE_MAJOR_VERSION == 5 || (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 2 && ENGINE_PATCH_VERSION > 5))
		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRepuest = FHttpModule::Get().CreateRequest();
#else
		TSharedRef<IHttpRequest> HttpRepuest = FHttpModule::Get().CreateRequest();
#endif
		HttpRepuest->SetURL(URL);
		HttpRepuest->SetVerb(TEXT("GET"));
		HttpRepuest->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));
		HttpRepuest->OnProcessRequestComplete().BindRaw(this, &FHTTP::OnRequestComplete);
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
		HttpRepuest->OnRequestProgress64().BindRaw(this, &FHTTP::OnRequestProgress64);
#else
		HttpRepuest->OnRequestProgress().BindRaw(this, &FHTTP::OnRequestProgress);
#endif
		HttpRepuest->OnHeaderReceived().BindRaw(this, &FHTTP::OnRequestHeaderReceived);
	
		bool bRepuest = HttpRepuest->ProcessRequest();

		AddTaskCount();

		return bRepuest;
	}

	void FHTTP::GetObjectsToMemory(const TArray<FString>& URL)
	{
		for (auto &Tmp : URL)
		{
			GetObjectToMemory(Tmp);
		}
	}

	void FHTTP::OnRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
	{
		Delegate.HttpCompleteDelegate.ExecuteIfBound(HttpRequest, HttpResponse, bSucceeded);

		if (CheckCompleteCount())
		{
			OnRequestsComplete();
		}
	}

	void FHTTP::OnRequestProgress64(FHttpRequestPtr HttpRequest, uint64 SendBytes, uint64 BytesReceived)
	{
		int32 TotalByts = 0;
		if (int32* TotalBytsPtr = DataSizeInfoMap.Find(*HttpRequest->GetURL()))
		{
			TotalByts = *TotalBytsPtr;
		}

		Delegate.HttpSingleRequestProgressDelegate.ExecuteIfBound(HttpRequest, TotalByts, BytesReceived);
	}

	void FHTTP::OnRequestProgress(FHttpRequestPtr HttpRequest, int32 SendBytes, int32 BytesReceived)
	{
		OnRequestProgress64(HttpRequest, SendBytes, BytesReceived);
	}

	void FHTTP::OnRequestHeaderReceived(FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue)
	{
		if (HeaderName == TEXT("Content-Length"))
		{
			DataSizeInfoMap.Add(*Request->GetURL(), FCString::Atoi(*NewHeaderValue));
		}

		Delegate.HttpSingleRequestHeaderReceivedDelegate.ExecuteIfBound(Request, HeaderName, NewHeaderValue);
	}

	void FHTTP::OnRequestsComplete()
	{
		Delegate.AllTasksCompletedDelegate.ExecuteIfBound();
		delete this;
	}

	void FHTTP::AddTaskCount()
	{
		TaskCount++;
	}

	bool FHTTP::CheckCompleteCount()
	{
		CompleteCount++;
		return CompleteCount >= TaskCount;
	}
}

#if PLATFORM_WINDOWS
#pragma optimize("",on) 
#endif