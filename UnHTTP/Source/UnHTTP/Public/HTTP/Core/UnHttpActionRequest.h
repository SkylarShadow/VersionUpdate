

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "HTTP/Core/UnHttpActionRequest.h"
#include "UnHTTPType.h"
#include "Request/RequestInterface.h"

/**
 * 
 */
class UNHTTP_API FUnHttpActionRequest :public TSharedFromThis<FUnHttpActionRequest>
{
public:
	typedef FUnHttpActionRequest Super;

	//BP
	FUnHttpSingleRequestCompleteDelegate			HttpSingleRequestCompleteDelegate;
	FUnHttpSingleRequestProgressDelegate			HttpSingleRequestProgressDelegate;
	FUnHttpSingleRequestHeaderReceivedDelegate		HttpSingleRequestHeaderReceivedDelegate;
	FAllRequestCompleteDelegate							AllRequestCompleteDelegate;

	//C++
	FSingleCompleteDelegate						SingleCompleteDelegate;
	FSingleRequestProgressDelegate				SingleRequestProgressDelegate;
	FSingleRequestHeaderReceivedDelegate			SingleRequestHeaderReceivedDelegate;
	FSimpleDelegate										AllTasksCompletedDelegate;

public:
	FUnHttpActionRequest();
	virtual ~FUnHttpActionRequest();

	virtual bool Suspend();
	virtual bool Cancel();

	virtual bool SendRequest(EUnHTTPVerbType InType, const FString& URL, const TMap<FString, FString>& InMeta, const FString& InConentData);
	virtual bool SendRequest(EUnHTTPVerbType InType, const FString& URL, const TMap<FString, FString>& InMeta, const TArray<uint8>& InData);

	//Compatibility blueprint
	virtual void GetObjects(const TArray<FString> &URL, const FString &SavePaths);
	virtual void GetObjects(const TArray<FString> &URL);
	virtual void DeleteObjects(const TArray<FString> &URL);

	virtual bool GetObject(const FString &URL);
	virtual bool GetObject(const FString &URL, const FString &SavePaths);
	virtual bool PutObject(const FString &URL, const TArray<uint8> &Data);
	virtual bool PutObject(const FString &URL, const FString &LocalPaths);
	virtual bool PutObjectByString(const FString& URL, const FString& InBuff);
	virtual bool PutObject(const FString &URL, TSharedRef<FArchive, ESPMode::ThreadSafe> Stream);
	virtual bool DeleteObject(const FString &URL);
	virtual bool PostObject(const FString &URL);
	virtual bool PostObject(const FString& URL, const FString& LocalPaths);
	virtual bool PostObject(const FString& URL, const TArray<uint8>& Data);

	FORCEINLINE FString GetPaths() { return TmpSavePaths; }
	FORCEINLINE void SetPaths(const FString &NewPaths) { TmpSavePaths = NewPaths; }
	FORCEINLINE bool IsRequestComplete() { return bRequestComplete; }

	FORCEINLINE void SetAsynchronousState(bool bNewAsy) { bAsynchronous = bNewAsy; }

protected:
	virtual void HttpRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	virtual void HttpRequestProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived);
	virtual void HttpRequestProgress64(FHttpRequestPtr Request, uint64 BytesSent, uint64 BytesReceived);
	virtual void HttpRequestHeaderReceived(FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue);

	void Print(const FString &Msg, float Time = 10.f, FColor Color = FColor::Red);

protected:
	void RequestPtrToSimpleRequest(FHttpRequestPtr Request, FUnHttpRequest& UnHttpRequest);
	void ResponsePtrToSimpleResponse(FHttpResponsePtr Response, FUnHttpResponse& UnHttpResponse);

protected:
	virtual void ExecutionCompleteDelegate(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

protected:
	FString						TmpSavePaths;
	bool						bRequestComplete;
	bool						bSaveDisk;
	bool						bAsynchronous;
};
