
#include "Request/HTTPClientRequest.h"
#include "Core/UnHttpMacro.h"
#include "UnHTTPLog.h"
#include "Core/UnHTTPMethod.h"

UnHTTP::HTTP::FPutObjectRequest::FPutObjectRequest(const FString &URL, const FString& ContentString)
{
	DEFINITION_HTTP_TYPE(PUT,"multipart/form-data;charset=utf-8")
	HttpReuest->SetContentAsString(ContentString);

	UE_LOG(LogUnHTTP, Log, TEXT("PUT Action as string"));
}

UnHTTP::HTTP::FPutObjectRequest::FPutObjectRequest(const FString &URL, TSharedRef<FArchive, ESPMode::ThreadSafe> Stream)
{
	DEFINITION_HTTP_TYPE(PUT, "multipart/form-data;charset=utf-8")
	HttpReuest->SetContentFromStream(Stream);

	UE_LOG(LogUnHTTP, Log, TEXT("PUT Action by stream."));
}

UnHTTP::HTTP::FPutObjectRequest::FPutObjectRequest(const FString &URL,const TArray<uint8>& ContentPayload)
{
	DEFINITION_HTTP_TYPE(PUT, "multipart/form-data;charset=utf-8")
	HttpReuest->SetContent(ContentPayload);

	UE_LOG(LogUnHTTP, Log, TEXT("PUT Action by content."));
}

//Get: get resources from the server
//Post: create a new resource on the server.
//Put: update resources on the server and provide the changed complete resources on the client
//Patch: update resources on the server and provide changed properties on the client
//Delete: delete a resource from the server.
//Head: get the metadata of the resource.
//Options: get information about which attributes of the resource can be changed by the client.

UnHTTP::HTTP::FGetObjectRequest::FGetObjectRequest(const FString &URL)
{
	DEFINITION_HTTP_TYPE(GET, "application/x-www-form-urlencoded;charset=utf-8")

	UE_LOG(LogUnHTTP, Log, TEXT("GET Action."));
}

UnHTTP::HTTP::FDeleteObjectsRequest::FDeleteObjectsRequest(const FString &URL)
{
	DEFINITION_HTTP_TYPE(DELETE, "application/x-www-form-urlencoded;charset=utf-8")

	UE_LOG(LogUnHTTP, Log, TEXT("DELETE Action."));
}

UnHTTP::HTTP::FPostObjectsRequest::FPostObjectsRequest(const FString &URL)
{
	DEFINITION_HTTP_TYPE(POST, "application/x-www-form-urlencoded;charset=utf-8")

	UE_LOG(LogUnHTTP, Log, TEXT("POST Action."));
}

UnHTTP::HTTP::FPostObjectsRequest::FPostObjectsRequest(const FString& URL, const FString& ContentString)
{
	DEFINITION_HTTP_TYPE(POST,"multipart/form-data;charset=utf-8")
	HttpReuest->SetContentAsString(ContentString);

	UE_LOG(LogUnHTTP, Log, TEXT("Post Action as string"));
}

UnHTTP::HTTP::FPostObjectsRequest::FPostObjectsRequest(const FString& URL, const TArray<uint8>& ContentPayload)
{
	DEFINITION_HTTP_TYPE(POST,"multipart/form-data;charset=utf-8")
	HttpReuest->SetContent(ContentPayload);

	UE_LOG(LogUnHTTP, Log, TEXT("Post Action as Byte"));
}

UnHTTP::HTTP::FObjectsRequest::FObjectsRequest(EUnHTTPVerbType InType, const FString& URL, const TMap<FString, FString>& InMeta, const FString& InConentData)
{
	FString InNewURLEncoded = UnHTTP::URLEncode(*URL);

	HttpReuest->SetURL(URL);
	HttpReuest->SetVerb(HTTPVerbToString(InType));

	for (auto &Tmp : InMeta)
	{
		HttpReuest->SetHeader(Tmp.Key, Tmp.Value);
	}

	HttpReuest->SetContentAsString(InConentData);	
}

UnHTTP::HTTP::FObjectsRequest::FObjectsRequest(EUnHTTPVerbType InType, const FString& URL, const TMap<FString, FString>& InMeta, const TArray<uint8>& InData)
{
	FString InNewURLEncoded = UnHTTP::URLEncode(*URL);

	HttpReuest->SetURL(InNewURLEncoded);
	HttpReuest->SetVerb(HTTPVerbToString(InType));

	for (auto& Tmp : InMeta)
	{
		HttpReuest->SetHeader(Tmp.Key, Tmp.Value);
	}

	HttpReuest->SetContent(InData);
}

FString UnHTTP::HTTP::HTTPVerbToString(EUnHTTPVerbType InVerbType)
{
	switch (InVerbType)
	{
	case EUnHTTPVerbType::UN_POST:
		return TEXT("POST");
	case EUnHTTPVerbType::UN_PUT:
		return TEXT("PUT");
	case EUnHTTPVerbType::UN_GET:
		return TEXT("GET");
	case EUnHTTPVerbType::UN_DELETE:
		return TEXT("DELETE");
	}

	return TEXT("POST");
}
