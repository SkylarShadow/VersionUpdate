

#include "HTTP/Core/UnHttpActionRequest.h"
#include "Client/HTTPClient.h"
#include "Core/UnHttpMacro.h"
#include "HAL/FileManager.h"
#include "UnHTTPLog.h"
#include "Misc/Paths.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/FileHelper.h"

//#include "GenericPlatform/GenericPlatformHttp.h"

FUnHttpActionRequest::FUnHttpActionRequest()
	:bRequestComplete(false)
	,bSaveDisk(true)
	,bAsynchronous(true)
{
}

FUnHttpActionRequest::~FUnHttpActionRequest()
{

}

bool FUnHttpActionRequest::Suspend()
{
	checkf(0,TEXT("UE HTTP currently does not support single pause. However, we support the suspension of the entire HTTP download!"));
	
	return false;
}

bool FUnHttpActionRequest::Cancel()
{
	return false;
}

bool FUnHttpActionRequest::SendRequest(EUnHTTPVerbType InType,const FString& URL, const TMap<FString, FString>& InMeta, const FString& InConentData)
{
	return false;
}

bool FUnHttpActionRequest::SendRequest(EUnHTTPVerbType InType, const FString& URL, const TMap<FString, FString>& InMeta, const TArray<uint8>& InData)
{
	return false;
}

void FUnHttpActionRequest::GetObjects(const TArray<FString> &URL, const FString &SavePaths)
{

}

void FUnHttpActionRequest::GetObjects(const TArray<FString> &URL)
{

}

void FUnHttpActionRequest::DeleteObjects(const TArray<FString> &URL)
{

}

void FUnHttpActionRequest::HttpRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	FString DebugPram;
	Request->GetURLParameter(DebugPram);
	UE_LOG(LogUnHTTP, Warning,
		TEXT("HttpRequestComplete URL=%s,Param=%s"),
		*Request->GetURL(),
		*DebugPram);

	//404 405 100 -199 200 -299
	if (!Request.IsValid())
	{
		ExecutionCompleteDelegate(Request, Response, bConnectedSuccessfully);

		UE_LOG(LogUnHTTP, Warning, TEXT("Server request failed."));
	}
	else if(!Response.IsValid())
	{
		ExecutionCompleteDelegate(Request, Response, bConnectedSuccessfully);
		UE_LOG(LogUnHTTP, Warning, TEXT(" Response failed."));
	}
	else if (!bConnectedSuccessfully)
	{
		ExecutionCompleteDelegate(Request, Response, bConnectedSuccessfully);
		UE_LOG(LogUnHTTP, Warning, TEXT("HTTP connected failed. msg[%s]"),
			*Response->GetContentAsString());
	}
	else if (!EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		ExecutionCompleteDelegate(Request, Response, bConnectedSuccessfully);
		UE_LOG(LogUnHTTP, Warning, TEXT("HTTP error code [%d] Msg[%s]."), 
			Response->GetResponseCode(),
			*Response->GetContentAsString());
	}
	else
	{
		if (Request->GetVerb() == "GET")
		{
			if (bSaveDisk)
			{
				FString Filename = FPaths::GetCleanFilename(Request->GetURL());
				FFileHelper::SaveArrayToFile(Response->GetContent(), *(GetPaths() / Filename));

				UE_LOG(LogUnHTTP, Log, TEXT("Store the obtained http file locally."));
				UE_LOG(LogUnHTTP, Log, TEXT("%s."), *Filename);
			}
			else
			{
				UE_LOG(LogUnHTTP, Log, TEXT("This is a get request that is not stored locally."));
			}
		}

		ExecutionCompleteDelegate(Request, Response, bConnectedSuccessfully);
		UE_LOG(LogUnHTTP, Log, TEXT("Request to complete execution of binding agent."));
	}
}

void FUnHttpActionRequest::HttpRequestProgress64(FHttpRequestPtr Request, uint64 BytesSent, uint64 BytesReceived)
{
	FUnHttpRequest UnHttpRequest;
	RequestPtrToSimpleRequest(Request, UnHttpRequest);

	SingleRequestProgressDelegate.ExecuteIfBound(UnHttpRequest, BytesSent, BytesReceived);
	SingleRequestProgressDelegate.ExecuteIfBound(UnHttpRequest, BytesSent, BytesReceived);
}

void FUnHttpActionRequest::HttpRequestProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived)
{
	FUnHttpRequest UnHttpRequest;
	RequestPtrToSimpleRequest(Request, UnHttpRequest);

	SingleRequestProgressDelegate.ExecuteIfBound(UnHttpRequest, BytesSent, BytesReceived);
	SingleRequestProgressDelegate.ExecuteIfBound(UnHttpRequest, BytesSent, BytesReceived);

//	UE_LOG(LogUnHTTP, Log, TEXT("Http request progress."));
}

void FUnHttpActionRequest::HttpRequestHeaderReceived(FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue)
{
	FUnHttpRequest UnHttpRequest;
	RequestPtrToSimpleRequest(Request, UnHttpRequest);

	SingleRequestHeaderReceivedDelegate.ExecuteIfBound(UnHttpRequest, HeaderName, NewHeaderValue);
	SingleRequestHeaderReceivedDelegate.ExecuteIfBound(UnHttpRequest, HeaderName, NewHeaderValue);

//	UE_LOG(LogUnHTTP, Log, TEXT("Http request header received."));
}

void FUnHttpActionRequest::Print(const FString &Msg, float Time /*= 10.f*/, FColor Color /*= FColor::Red*/)
{
#ifdef PLATFORM_PROJECT
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Time, Color, Msg);
	}
#endif
}

void FUnHttpActionRequest::ExecutionCompleteDelegate(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	FUnHttpRequest UnHttpRequest;
	FUnHttpResponse UnHttpResponse;
	ResponsePtrToSimpleResponse(Response, UnHttpResponse);
	RequestPtrToSimpleRequest(Request, UnHttpRequest);

	HttpSingleRequestCompleteDelegate.ExecuteIfBound(UnHttpRequest, UnHttpResponse, bConnectedSuccessfully);
	SingleCompleteDelegate.ExecuteIfBound(UnHttpRequest, UnHttpResponse, bConnectedSuccessfully);
}

bool FUnHttpActionRequest::GetObject(const FString &URL, const FString &SavePaths)
{
	return false;
}

bool FUnHttpActionRequest::GetObject(const FString &URL)
{
	return false;
}

bool FUnHttpActionRequest::PutObject(const FString &URL,const TArray<uint8> &Data)
{
	return false;
}

bool FUnHttpActionRequest::PutObject(const FString &URL, const FString &LocalPaths)
{
	return false;
}

bool FUnHttpActionRequest::PutObjectByString(const FString& URL, const FString& InBuff)
{
	return false;
}

bool FUnHttpActionRequest::PutObject(const FString &URL, TSharedRef<FArchive, ESPMode::ThreadSafe> Stream)
{
	return false;
}

bool FUnHttpActionRequest::DeleteObject(const FString &URL)
{
	return false;
}

bool FUnHttpActionRequest::PostObject(const FString &URL)
{
	return false;
}

bool FUnHttpActionRequest::PostObject(const FString& URL, const FString& LocalPaths)
{
	return false;
}

bool FUnHttpActionRequest::PostObject(const FString& URL, const TArray<uint8>& Data)
{
	return false;
}

void FUnHttpActionRequest::RequestPtrToSimpleRequest(FHttpRequestPtr Request, FUnHttpRequest& UnHttpRequest)
{
	if (Request.IsValid())
	{
		UnHttpRequest.Verb = Request->GetVerb();
		UnHttpRequest.URL = Request->GetURL();
		UnHttpRequest.Status = (FUnHttpStarte)Request->GetStatus();
		UnHttpRequest.ElapsedTime = Request->GetElapsedTime();
		UnHttpRequest.ContentType = Request->GetContentType();
		UnHttpRequest.ContentLength = Request->GetContentLength();
		UnHttpRequest.AllHeaders = Request->GetAllHeaders();
		UnHttpRequest.RowPtr = &Request;
	}
}

void FUnHttpActionRequest::ResponsePtrToSimpleResponse(FHttpResponsePtr Response, FUnHttpResponse& UnHttpResponse)
{
	if (Response.IsValid())
	{
		UnHttpResponse.ResponseCode = Response->GetResponseCode();
		UnHttpResponse.URL = Response->GetURL();
		UnHttpResponse.ResponseMessage = Response->GetContentAsString();
		UnHttpResponse.ContentType = Response->GetContentType();
		UnHttpResponse.ContentLength = Response->GetContentLength();
		UnHttpResponse.AllHeaders = Response->GetAllHeaders();
		UnHttpResponse.Content->Content = const_cast<TArray<uint8>*>(&Response->GetContent());
		UnHttpResponse.RowPtr = &Response;
	}
}