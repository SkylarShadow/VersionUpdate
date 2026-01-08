
#include "UnHTTPManage.h"
#include "HTTP/UnHttpActionMultpleRequest.h"
#include "HTTP/UnHttpActionSingleRequest.h"
#include "Core/UnHttpMacro.h"
#include "Misc/FileHelper.h"
#include "UnHTTPLog.h"
#include "HttpModule.h"
#include "HttpManager.h"

#if PLATFORM_WINDOWS
UE_DISABLE_OPTIMIZATION
#endif

FUnHttpManage* FUnHttpManage::Instance = NULL;

TSharedPtr<FUnHttpActionRequest> GetHttpActionRequest(EHTTPRequestType RequestType)
{
	TSharedPtr<FUnHttpActionRequest> HttpObject = nullptr;
	switch (RequestType)
	{
		case EHTTPRequestType::SINGLE:
		{
			HttpObject = MakeShareable(new FUnHttpActionSingleRequest());
			UE_LOG(LogUnHTTP, Log, TEXT("Action to create a single HTTP request"));
			break;
		}
		case EHTTPRequestType::MULTPLE:
		{
			HttpObject = MakeShareable(new FUnHttpActionMultpleRequest());
			UE_LOG(LogUnHTTP, Log, TEXT("Action to create a multple HTTP request"));
			break;
		}
	}

	return HttpObject;
}

void FUnHttpManage::Tick(float DeltaTime)
{
	FScopeLock ScopeLock(&Instance->Mutex);

	if (!HTTP.bPause)
	{
		FHttpModule::Get().GetHttpManager().Tick(DeltaTime);
	}
	
	TArray<FName> RemoveRequest;
	for (auto &Tmp : HTTP.HTTPMap)
	{
		if (Tmp.Value->IsRequestComplete())
		{
			RemoveRequest.Add(Tmp.Key);
		}
	}

	for (auto &Tmp : RemoveRequest)
	{
		GetHTTP().HTTPMap.Remove(Tmp);

		UE_LOG(LogUnHTTP, Log, TEXT("Remove request %s from tick"), *Tmp.ToString());
	}
}

bool FUnHttpManage::IsTickableInEditor() const
{
	return true;
}

TStatId FUnHttpManage::GetStatId() const
{
	return TStatId();
}

FUnHttpManage * FUnHttpManage::Get()
{
	if (Instance == nullptr)
	{
		Instance = new FUnHttpManage();

		UE_LOG(LogUnHTTP, Log, TEXT("Get HTTP management"));
	}

	return Instance;
}

void FUnHttpManage::Destroy()
{
	if (Instance != nullptr)
	{
		FScopeLock ScopeLock(&Instance->Mutex);
		delete Instance;		

		UE_LOG(LogUnHTTP, Log, TEXT("delete HTTP management"));
	}

	Instance = nullptr;
}

FUnHTTPHandle FUnHttpManage::FHTTP::RegisteredHttpRequest(
	EHTTPRequestType RequestType ,
	FUnHttpSingleRequestCompleteDelegate HttpSingleRequestCompleteDelegate /*= FUnHttpRequestCompleteDelegate()*/,
	FUnHttpSingleRequestProgressDelegate HttpSingleRequestProgressDelegate /*= FUnHttpRequestProgressDelegate()*/,
	FUnHttpSingleRequestHeaderReceivedDelegate HttpSingleRequestHeaderReceivedDelegate /*= FUnHttpRequestHeaderReceivedDelegate()*/,
	FAllRequestCompleteDelegate AllRequestCompleteDelegate /*= FAllRequestCompleteDelegate()*/)
{
	FScopeLock ScopeLock(&Instance->Mutex);

	UE_LOG(LogUnHTTP, Log, TEXT("Start registering single BP agent."));

	TSharedPtr<FUnHttpActionRequest> HttpObject = GetHttpActionRequest(RequestType);
	
	HttpObject->HttpSingleRequestCompleteDelegate = HttpSingleRequestCompleteDelegate;
	HttpObject->HttpSingleRequestHeaderReceivedDelegate = HttpSingleRequestHeaderReceivedDelegate;
	HttpObject->HttpSingleRequestProgressDelegate = HttpSingleRequestProgressDelegate;
	HttpObject->AllRequestCompleteDelegate = AllRequestCompleteDelegate;

	FUnHTTPHandle Key = *FGuid::NewGuid().ToString();
	HTTPMap.Add(Key,HttpObject);

	return Key;
}

FUnHTTPHandle FUnHttpManage::FHTTP::RegisteredHttpRequest(
	EHTTPRequestType RequestType /*= EHTTPRequestType::SINGLE*/, 
	FSingleCompleteDelegate SingleRequestCompleteDelegate /*= nullptr*/, 
	FSingleRequestProgressDelegate SingleRequestProgressDelegate /*= nullptr*/,
	FSingleRequestHeaderReceivedDelegate SingleRequestHeaderReceivedDelegate /*= nullptr*/,
	FSimpleDelegate AllRequestCompleteDelegate /*= nullptr*/)
{
	FScopeLock ScopeLock(&Instance->Mutex);

	UE_LOG(LogUnHTTP, Log, TEXT("Start registering single C++ agent."));

	TSharedPtr<FUnHttpActionRequest> HttpObject = GetHttpActionRequest(RequestType);

	HttpObject->SingleCompleteDelegate = SingleRequestCompleteDelegate;
	HttpObject->SingleRequestHeaderReceivedDelegate = SingleRequestHeaderReceivedDelegate;
	HttpObject->SingleRequestProgressDelegate = SingleRequestProgressDelegate;
	HttpObject->AllTasksCompletedDelegate = AllRequestCompleteDelegate;

	FUnHTTPHandle Key = *FGuid::NewGuid().ToString();
	HTTPMap.Add(Key, HttpObject);

	return Key;
}

TWeakPtr<FUnHttpActionRequest> FUnHttpManage::FHTTP::Find(const FUnHTTPHandle &Handle)
{
	FScopeLock ScopeLock(&Instance->Mutex);

	TWeakPtr<FUnHttpActionRequest> Object = nullptr;
	for (auto &Tmp : Instance->GetHTTP().HTTPMap)
	{
		if (Tmp.Key == Handle)
		{
			Object = Tmp.Value;

			UE_LOG(LogUnHTTP, Log, TEXT("Http Find at %s"),*(Tmp.Key.ToString()));
			break;
		}
	}

	return Object;
}

bool FUnHttpManage::FHTTP::GetObjectToMemory(const FUnHTTPHandle &Handle, const FString &URL, bool bSynchronous)
{
	TWeakPtr<FUnHttpActionRequest> Object = Find(Handle);
	if (Object.IsValid())
	{
		Object.Pin()->SetAsynchronousState(!bSynchronous);
		return Object.Pin()->GetObject(URL);
	}
	else
	{
		UE_LOG(LogUnHTTP, Warning, TEXT("The handle was not found [%s]"), *(Handle.ToString()));
	}

	return false;
}

bool FUnHttpManage::FHTTP::GetObjectToMemory(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL,bool bSynchronous)
{
	UN_HTTP_REGISTERED_REQUEST_BP(EHTTPRequestType::SINGLE);

	return GetObjectToMemory(Handle, URL, bSynchronous);
}

bool FUnHttpManage::FHTTP::GetObjectToMemory(const FUnHTTPResponseDelegate &BPResponseDelegate, const FString &URL, bool bSynchronous)
{
	UN_HTTP_REGISTERED_REQUEST(EHTTPRequestType::SINGLE);

	return GetObjectToMemory(Handle, URL, bSynchronous);
}

void FUnHttpManage::FHTTP::GetObjectsToMemory(const FUnHTTPHandle &Handle, const TArray<FString> &URL, bool bSynchronous)
{
	TWeakPtr<FUnHttpActionRequest> Object = Find(Handle);
	if (Object.IsValid())
	{
		Object.Pin()->SetAsynchronousState(!bSynchronous);
		Object.Pin()->GetObjects(URL);
	}
	else
	{
		UE_LOG(LogUnHTTP, Warning, TEXT("The handle was not found [%s]"), *(Handle.ToString()));
	}
}

void FUnHttpManage::FHTTP::GetObjectsToMemory(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const TArray<FString> &URL, bool bSynchronous)
{
	UN_HTTP_REGISTERED_REQUEST_BP(EHTTPRequestType::MULTPLE);

	GetObjectsToMemory(Handle, URL, bSynchronous);
}

void FUnHttpManage::FHTTP::GetObjectsToMemory(const FUnHTTPResponseDelegate &BPResponseDelegate, const TArray<FString> &URL, bool bSynchronous)
{
	UN_HTTP_REGISTERED_REQUEST(EHTTPRequestType::MULTPLE);

	GetObjectsToMemory(Handle, URL, bSynchronous);
}

bool FUnHttpManage::FHTTP::GetObjectToLocal(const FUnHTTPHandle &Handle, const FString &URL, const FString &SavePaths)
{
	TWeakPtr<FUnHttpActionRequest> Object = Find(Handle);
	if (Object.IsValid())
	{
		return Object.Pin()->GetObject(URL, SavePaths);
	}
	else
	{
		UE_LOG(LogUnHTTP, Warning, TEXT("The handle was not found [%s]"), *(Handle.ToString()));
	}

	return false;
}

bool FUnHttpManage::FHTTP::GetObjectToLocal(const FUnHTTPResponseDelegate &BPResponseDelegate, const FString &URL, const FString &SavePaths)
{
	UN_HTTP_REGISTERED_REQUEST(EHTTPRequestType::SINGLE);

	return GetObjectToLocal(Handle, URL, SavePaths);
}

bool FUnHttpManage::FHTTP::GetObjectToLocal(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL, const FString &SavePaths)
{
	UN_HTTP_REGISTERED_REQUEST_BP(EHTTPRequestType::SINGLE);

	return GetObjectToLocal(Handle, URL, SavePaths);
}

void FUnHttpManage::FHTTP::GetObjectsToLocal(const FUnHTTPHandle &Handle, const TArray<FString> &URL, const FString &SavePaths)
{
	TWeakPtr<FUnHttpActionRequest> Object = Find(Handle);
	if (Object.IsValid())
	{
		Object.Pin()->GetObjects(URL, SavePaths);
	}
	else
	{
		UE_LOG(LogUnHTTP, Warning, TEXT("The handle was not found [%s]"), *(Handle.ToString()));
	}
}

void FUnHttpManage::FHTTP::GetObjectsToLocal(const FUnHTTPResponseDelegate &BPResponseDelegate, const TArray<FString> &URL, const FString &SavePaths)
{
	UN_HTTP_REGISTERED_REQUEST(EHTTPRequestType::MULTPLE);

	GetObjectsToLocal(Handle, URL, SavePaths);
}

void FUnHttpManage::FHTTP::GetObjectsToLocal(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const TArray<FString> &URL, const FString &SavePaths)
{
	UN_HTTP_REGISTERED_REQUEST_BP(EHTTPRequestType::MULTPLE);

	GetObjectsToLocal(Handle, URL, SavePaths);
}

bool FUnHttpManage::FHTTP::PutObjectFromBuffer(const FUnHTTPHandle &Handle, const FString &URL, const TArray<uint8> &Data)
{
	TWeakPtr<FUnHttpActionRequest> Object = Find(Handle);
	if (Object.IsValid())
	{
		return Object.Pin()->PutObject(URL, Data);
	}
	else
	{
		UE_LOG(LogUnHTTP, Warning, TEXT("The handle was not found [%s]"), *(Handle.ToString()));
	}

	return false;
}

bool FUnHttpManage::FHTTP::PutObjectFromString(const FUnHTTPHandle& Handle, const FString& URL, const FString& Buffer, bool bSynchronouse)
{
	TWeakPtr<FUnHttpActionRequest> Object = Find(Handle);
	if (Object.IsValid())
	{
		Object.Pin()->SetAsynchronousState(!bSynchronouse);
		return Object.Pin()->PutObjectByString(URL, Buffer);
	}
	else
	{
		UE_LOG(LogUnHTTP, Warning, TEXT("The handle was not found [%s]"), *(Handle.ToString()));
	}

	return false;
}

bool FUnHttpManage::FHTTP::PutObjectFromBuffer(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL, const TArray<uint8> &Buffer)
{
	UN_HTTP_REGISTERED_REQUEST_BP(EHTTPRequestType::SINGLE);

	return PutObjectFromBuffer(Handle, URL, Buffer);
}

bool FUnHttpManage::FHTTP::PutObjectFromString(const FUnHTTPBPResponseDelegate& BPResponseDelegate, const FString& URL, const FString& InBuffer, bool bSynchronouse)
{
	UN_HTTP_REGISTERED_REQUEST_BP(EHTTPRequestType::SINGLE);

	return PutObjectFromString(Handle, URL, InBuffer, bSynchronouse);
}

bool FUnHttpManage::FHTTP::PutObjectFromBuffer(const FUnHTTPResponseDelegate &BPResponseDelegate, const FString &URL, const TArray<uint8> &Buffer)
{
	UN_HTTP_REGISTERED_REQUEST(EHTTPRequestType::SINGLE);

	return PutObjectFromBuffer(Handle, URL, Buffer);
}

bool FUnHttpManage::FHTTP::PutObjectFromString(const FUnHTTPResponseDelegate& BPResponseDelegate, const FString& URL, const FString& InBuffer,bool bSynchronouse)
{
	UN_HTTP_REGISTERED_REQUEST(EHTTPRequestType::SINGLE);

	return PutObjectFromString(Handle, URL, InBuffer, bSynchronouse);
}

bool FUnHttpManage::FHTTP::PutObjectFromStream(const FUnHTTPHandle &Handle, const FString &URL, TSharedRef<FArchive, ESPMode::ThreadSafe> Stream)
{
	TWeakPtr<FUnHttpActionRequest> Object = Find(Handle);
	if (Object.IsValid())
	{
		return Object.Pin()->PutObject(URL, Stream);
	}
	else
	{
		UE_LOG(LogUnHTTP, Warning, TEXT("The handle was not found [%s]"), *(Handle.ToString()));
	}

	return false;
}

bool FUnHttpManage::FHTTP::PutObjectFromStream(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL, TSharedRef<FArchive, ESPMode::ThreadSafe> Stream)
{
	UN_HTTP_REGISTERED_REQUEST_BP(EHTTPRequestType::SINGLE);

	return PutObjectFromStream(Handle, URL, Stream);
}

bool FUnHttpManage::FHTTP::PutObjectFromStream(const FUnHTTPResponseDelegate &BPResponseDelegate, const FString &URL, TSharedRef<FArchive, ESPMode::ThreadSafe> Stream)
{
	UN_HTTP_REGISTERED_REQUEST(EHTTPRequestType::SINGLE);

	return PutObjectFromStream(Handle, URL, Stream);
}

bool FUnHttpManage::FHTTP::PutObjectFromLocal(const FUnHTTPHandle &Handle, const FString &URL, const FString &LocalPaths, bool bSynchronouse)
{
	TWeakPtr<FUnHttpActionRequest> Object = Find(Handle);
	if (Object.IsValid())
	{
		Object.Pin()->SetAsynchronousState(!bSynchronouse);
		return Object.Pin()->PutObject(URL, LocalPaths);
	}
	else
	{
		UE_LOG(LogUnHTTP, Warning, TEXT("The handle was not found [%s]"), *(Handle.ToString()));
	}

	return false;
}

bool FUnHttpManage::FHTTP::PostObjectFromLocal(const FUnHTTPHandle& Handle, const FString& URL, const FString& LocalPaths, bool bSynchronouse)
{
	TWeakPtr<FUnHttpActionRequest> Object = Find(Handle);
	if (Object.IsValid())
	{
		Object.Pin()->SetAsynchronousState(!bSynchronouse);
		return Object.Pin()->PostObject(URL, LocalPaths);
	}
	else
	{
		UE_LOG(LogUnHTTP, Warning, TEXT("The handle was not found [%s]"), *(Handle.ToString()));
	}

	return false;
}

bool FUnHttpManage::FHTTP::PutObjectFromLocal(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL, const FString &LocalPaths,bool bSynchronouse)
{
	UN_HTTP_REGISTERED_REQUEST_BP(EHTTPRequestType::SINGLE);

	return PutObjectFromLocal(Handle, URL, LocalPaths, bSynchronouse);
}

bool FUnHttpManage::FHTTP::PutObjectFromLocal(const FUnHTTPResponseDelegate &BPResponseDelegate, const FString &URL, const FString &LocalPaths, bool bSynchronouse)
{
	UN_HTTP_REGISTERED_REQUEST(EHTTPRequestType::SINGLE);

	return PutObjectFromLocal(Handle, URL, LocalPaths, bSynchronouse);
}

bool FUnHttpManage::FHTTP::PostObjectFromLocal(const FUnHTTPResponseDelegate& BPResponseDelegate, const FString& URL, const FString& LocalPaths, bool bSynchronouse)
{
	UN_HTTP_REGISTERED_REQUEST(EHTTPRequestType::SINGLE);

	return PostObjectFromLocal(Handle, URL, LocalPaths, bSynchronouse);
}

bool FUnHttpManage::FHTTP::PutObjectsFromLocal(const FUnHTTPHandle &Handle, const FString &URL, const FString &LocalPaths, bool bSynchronouse)
{
	return PutObjectFromLocal(Handle, URL, LocalPaths, bSynchronouse);
}

bool FUnHttpManage::FHTTP::PutObjectsFromLocal(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL, const FString &LocalPaths, bool bSynchronouse)
{
	UN_HTTP_REGISTERED_REQUEST_BP(EHTTPRequestType::MULTPLE);

	return PutObjectsFromLocal(Handle, URL, LocalPaths, bSynchronouse);
}

bool FUnHttpManage::FHTTP::PutObjectsFromLocal(const FUnHTTPResponseDelegate &BPResponseDelegate, const FString &URL, const FString &LocalPaths, bool bSynchronouse)
{
	UN_HTTP_REGISTERED_REQUEST(EHTTPRequestType::MULTPLE);

	return PutObjectsFromLocal(Handle, URL, LocalPaths, bSynchronouse);
}

bool FUnHttpManage::FHTTP::PostObjectsFromLocal(const FUnHTTPResponseDelegate& BPResponseDelegate, const FString& URL, const FString& LocalPaths, bool bSynchronouse)
{
	UN_HTTP_REGISTERED_REQUEST(EHTTPRequestType::MULTPLE);

	return PostObjectFromLocal(Handle, URL, LocalPaths, bSynchronouse);
}

bool FUnHttpManage::FHTTP::DeleteObject(const FUnHTTPHandle &Handle, const FString &URL)
{
	TWeakPtr<FUnHttpActionRequest> Object = Find(Handle);
	if (Object.IsValid())
	{
		return Object.Pin()->DeleteObject(URL);
	}
	else
	{
		UE_LOG(LogUnHTTP, Warning, TEXT("The handle was not found [%s]"), *(Handle.ToString()));
	}

	return false;
}

bool FUnHttpManage::FHTTP::DeleteObject(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL)
{
	UN_HTTP_REGISTERED_REQUEST_BP(EHTTPRequestType::SINGLE);

	return DeleteObject(Handle, URL);
}

bool FUnHttpManage::FHTTP::DeleteObject(const FUnHTTPResponseDelegate &BPResponseDelegate, const FString &URL)
{
	UN_HTTP_REGISTERED_REQUEST(EHTTPRequestType::SINGLE);

	return DeleteObject(Handle, URL);
}

void FUnHttpManage::FHTTP::DeleteObjects(const FUnHTTPHandle &Handle, const TArray<FString> &URL)
{
	TWeakPtr<FUnHttpActionRequest> Object = Find(Handle);
	if (Object.IsValid())
	{
		Object.Pin()->DeleteObjects(URL);
	}
	else
	{
		UE_LOG(LogUnHTTP, Warning, TEXT("The handle was not found [%s]"), *(Handle.ToString()));
	}
}

void FUnHttpManage::FHTTP::DeleteObjects(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const TArray<FString> &URL)
{
	UN_HTTP_REGISTERED_REQUEST_BP(EHTTPRequestType::MULTPLE);

	DeleteObjects(Handle, URL);
}

void FUnHttpManage::FHTTP::DeleteObjects(const FUnHTTPResponseDelegate &BPResponseDelegate, const TArray<FString> &URL)
{
	UN_HTTP_REGISTERED_REQUEST(EHTTPRequestType::MULTPLE);

	DeleteObjects(Handle, URL);
}

bool FUnHttpManage::FHTTP::PostRequest(const TCHAR *InURL, const TCHAR *InParam, const FUnHTTPBPResponseDelegate &BPResponseDelegate, bool bSynchronouse)
{
	UN_HTTP_REGISTERED_REQUEST_BP(EHTTPRequestType::SINGLE);

	return PostRequest(Handle,InURL + FString(TEXT("?")) + InParam);
}

bool FUnHttpManage::FHTTP::PostRequest(const FUnHTTPHandle &Handle, const FString &URL, bool bSynchronouse)
{
	TWeakPtr<FUnHttpActionRequest> Object = Find(Handle);

	if (Object.IsValid())
	{
		Object.Pin()->SetAsynchronousState(!bSynchronouse);
		Object.Pin()->PostObject(URL);

		return true;
	}
	else
	{
		UE_LOG(LogUnHTTP, Warning, TEXT("The handle was not found [%s]"), *(Handle.ToString()));
		return false;
	}
}

bool FUnHttpManage::FHTTP::PostRequest(const TCHAR *InURL, const TCHAR *InParam,const FUnHTTPResponseDelegate &BPResponseDelegate, bool bSynchronouse)
{
	UN_HTTP_REGISTERED_REQUEST(EHTTPRequestType::SINGLE);

	return PostRequest(Handle, InURL + FString(TEXT("?")) + InParam, bSynchronouse);
}

TArray<uint8> NULLArrayUint8;
TArray<uint8> & UUnHttpContent::GetContent()
{
	if (Content)
	{
		return *Content;
	}

	return NULLArrayUint8;
}

bool UUnHttpContent::Save(const FString &LocalPath)
{
	return FFileHelper::SaveArrayToFile(*Content, *LocalPath);
}

FUnHttpManage::FHTTP::FHTTP()
	:bPause(false)
{
}

void FUnHttpManage::FHTTP::Suspend()
{
	bPause = true;
}

void FUnHttpManage::FHTTP::Awaken()
{
	bPause = false;
}

bool FUnHttpManage::FHTTP::Suspend(const FUnHTTPHandle& Handle)
{
	TWeakPtr<FUnHttpActionRequest> Object = Find(Handle);
	if (Object.IsValid())
	{
		return Object.Pin()->Suspend();
	}

	return false;
}

bool FUnHttpManage::FHTTP::Cancel()
{
	bool bCancel = true;
	for (auto& Tmp : Instance->GetHTTP().HTTPMap)
	{
		if (!Tmp.Value->Cancel())
		{
			bCancel = false;
		}
	}

	return bCancel;
}

bool FUnHttpManage::FHTTP::Cancel(const FUnHTTPHandle& Handle)
{
	TWeakPtr<FUnHttpActionRequest> Object = Find(Handle);
	if (Object.IsValid())
	{
		return Object.Pin()->Cancel();
	}

	return false;
}

FUnHTTPHandle FUnHttpManage::FHTTP::GetHandleByLastExecutionRequest()
{
	return TemporaryStorageHandle;
}

bool FUnHttpManage::FHTTP::RequestByConentString(
	EUnHTTPVerbType InType,
	const FString& InURL, 
	const TMap<FString, FString>& InHeadMeta, 
	const FString& InContent,
	const FUnHTTPBPResponseDelegate& BPResponseDelegate)
{
	UN_HTTP_REGISTERED_REQUEST_BP(EHTTPRequestType::SINGLE);

	return RequestByConentString(Handle, InType, InURL, InHeadMeta, InContent);
}

bool FUnHttpManage::FHTTP::RequestByConentByte(EUnHTTPVerbType InType, const FString& InURL, const TMap<FString, FString>& InHeadMeta, const TArray<uint8>& InBytes, const FUnHTTPBPResponseDelegate& BPResponseDelegate)
{
	UN_HTTP_REGISTERED_REQUEST_BP(EHTTPRequestType::SINGLE);

	return RequestByConentByte(Handle,InType,InURL, InHeadMeta, InBytes);
}

bool FUnHttpManage::FHTTP::RequestByConentString(EUnHTTPVerbType InType, const FString& InURL, const TMap<FString, FString>& InHeadMeta, const FString& InContent, const FUnHTTPResponseDelegate& BPResponseDelegate)
{
	UN_HTTP_REGISTERED_REQUEST(EHTTPRequestType::SINGLE);

	return RequestByConentString(Handle, InType, InURL, InHeadMeta, InContent);
}

bool FUnHttpManage::FHTTP::RequestByConentByte(EUnHTTPVerbType InType, const FString& InURL, const TMap<FString, FString>& InHeadMeta, const TArray<uint8>& InBytes, const FUnHTTPResponseDelegate& BPResponseDelegate)
{
	UN_HTTP_REGISTERED_REQUEST(EHTTPRequestType::SINGLE);

	return RequestByConentByte(Handle, InType, InURL, InHeadMeta, InBytes);
}

bool FUnHttpManage::FHTTP::RequestByConentString(
	FUnHTTPHandle Handle, 
	EUnHTTPVerbType InType, 
	const FString& InURL,
	const TMap<FString, FString>& InHeadMeta,
	const FString& InContent)
{
	TWeakPtr<FUnHttpActionRequest> Object = Find(Handle);

	if (Object.IsValid())
	{
		Object.Pin()->SendRequest(InType, InURL, InHeadMeta, InContent);

		return true;
	}
	else
	{
		UE_LOG(LogUnHTTP, Warning, TEXT("The handle was not found [%s]"), *(Handle.ToString()));
		return false;
	}
}

bool FUnHttpManage::FHTTP::RequestByConentByte(FUnHTTPHandle Handle, EUnHTTPVerbType InType, const FString& InURL, const TMap<FString, FString>& InHeadMeta, const TArray<uint8>& InBytes)
{
	TWeakPtr<FUnHttpActionRequest> Object = Find(Handle);

	if (Object.IsValid())
	{
		Object.Pin()->SendRequest(InType, InURL, InHeadMeta, InBytes);

		return true;
	}
	else
	{
		UE_LOG(LogUnHTTP, Warning, TEXT("The handle was not found [%s]"), *(Handle.ToString()));
		return false;
	}
}

#if PLATFORM_WINDOWS
UE_ENABLE_OPTIMIZATION
#endif