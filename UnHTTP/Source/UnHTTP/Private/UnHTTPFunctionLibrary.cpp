


#include "UnHTTPFunctionLibrary.h"
#include "UnHttpManage.h"
#include "HTTP/Core/UnHttpActionRequest.h"
#include "HTTP/Core/UnHTTPHandle.h"

void UUnHTTPFunctionLibrary::Pause()
{
	UN_HTTP.Suspend();
}

void UUnHTTPFunctionLibrary::Awaken()
{
	UN_HTTP.Awaken();
}

bool UUnHTTPFunctionLibrary::Cancel()
{
	return UN_HTTP.Cancel();;
}

bool UUnHTTPFunctionLibrary::CancelByHandle(const FName& Handle)
{
	return UN_HTTP.Cancel((FUnHTTPHandle)Handle);
}

FName UUnHTTPFunctionLibrary::GetHandleByLastExecutionRequest()
{
	return UN_HTTP.GetHandleByLastExecutionRequest();
}

bool UUnHTTPFunctionLibrary::PostRequest(const FString &InURL, const FString &InParam, const FUnHTTPBPResponseDelegate &BPResponseDelegate)
{
	return UN_HTTP.PostRequest(*InURL,*InParam, BPResponseDelegate);
}

void UUnHTTPFunctionLibrary::Tick(float DeltaTime)
{
	FUnHttpManage::Get()->Tick(DeltaTime);
}

bool UUnHTTPFunctionLibrary::GetObjectToMemory(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL)
{
	return UN_HTTP.GetObjectToMemory(BPResponseDelegate, URL);
}

bool UUnHTTPFunctionLibrary::GetObjectToLocal(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL, const FString &SavePaths)
{
	return UN_HTTP.GetObjectToLocal(BPResponseDelegate, URL, SavePaths);
}

bool UUnHTTPFunctionLibrary::PutObjectFromLocal(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL, const FString &LocalPaths)
{
	return UN_HTTP.PutObjectFromLocal(BPResponseDelegate, URL, LocalPaths);
}

bool UUnHTTPFunctionLibrary::PutObjectFromBuffer(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL, const TArray<uint8> &Buffer)
{
	return UN_HTTP.PutObjectFromBuffer(BPResponseDelegate, URL, Buffer);
}

bool UUnHTTPFunctionLibrary::PutObjectFromString(const FUnHTTPBPResponseDelegate& BPResponseDelegate, const FString& URL, const FString& InBuffer)
{
	return UN_HTTP.PutObjectFromString(BPResponseDelegate, URL, InBuffer);
}

bool UUnHTTPFunctionLibrary::DeleteObject(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL)
{
	return UN_HTTP.DeleteObject(BPResponseDelegate, URL);
}

bool UUnHTTPFunctionLibrary::PutObjectsFromLocal(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL, const FString &LocalPaths)
{
	return UN_HTTP.PutObjectsFromLocal(BPResponseDelegate, URL,LocalPaths);
}

void UUnHTTPFunctionLibrary::GetObjectsToLocal(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const TArray<FString> &URL, const FString &SavePaths)
{
	UN_HTTP.GetObjectsToLocal(BPResponseDelegate, URL, SavePaths);
}

void UUnHTTPFunctionLibrary::GetObjectsToMemory(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const TArray<FString> &URL)
{
	UN_HTTP.GetObjectsToMemory(BPResponseDelegate, URL);
}

void UUnHTTPFunctionLibrary::DeleteObjects(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const TArray<FString> &URL)
{
	UN_HTTP.DeleteObjects(BPResponseDelegate, URL);
}

bool UUnHTTPFunctionLibrary::RequestByConentString(
	EUnHTTPVerbType InType,
	const FString& InURL,
	const TMap<FString, FString>& InHeadMeta,
	const FString& InContent, 
	const FUnHTTPBPResponseDelegate& BPResponseDelegate)
{
	return UN_HTTP.RequestByConentString(InType, InURL, InHeadMeta, InContent, BPResponseDelegate);
}

bool UUnHTTPFunctionLibrary::RequestByConentByte(
	EUnHTTPVerbType InType,
	const FString& InURL, 
	const TMap<FString, FString>& InHeadMeta, 
	const TArray<uint8>& InBytes,
	const FUnHTTPBPResponseDelegate& BPResponseDelegate)
{
	return UN_HTTP.RequestByConentByte(InType, InURL, InHeadMeta, InBytes, BPResponseDelegate);
}