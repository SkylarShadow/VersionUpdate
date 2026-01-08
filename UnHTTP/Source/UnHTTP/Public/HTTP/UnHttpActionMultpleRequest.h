
#pragma once

#include "CoreMinimal.h"
#include "Request/RequestInterface.h"
#include "HTTP/Core/UnHttpActionRequest.h"

class UNHTTP_API FUnHttpActionMultpleRequest : public FUnHttpActionRequest
{
public:
	FUnHttpActionMultpleRequest();

	virtual bool Suspend();
	virtual bool Cancel();

	virtual void GetObjects(const TArray<FString> &URL, const FString &SavePaths);
	virtual void GetObjects(const TArray<FString> &URL);
	virtual void DeleteObjects(const TArray<FString> &URL);
	virtual bool PutObject(const FString &URL, const FString &LocalPaths);

protected:
	virtual void HttpRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	virtual void HttpRequestProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived);
	virtual void HttpRequestProgress64(FHttpRequestPtr Request, uint64 BytesSent, uint64 BytesReceived);
	virtual void HttpRequestHeaderReceived(FHttpRequestPtr Request, const FString& HeaderName, const FString& NewHeaderValue);

protected:
	virtual void ExecutionCompleteDelegate(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

private:
	uint32 RequestNumber;
	TArray<TSharedPtr<UnHTTP::HTTP::IHTTPClientRequest>> Requests;
};