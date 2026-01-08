
#pragma once

#include "CoreMinimal.h"
#include "HTTP/Core/UnHTTPHandle.h"
#include "UnHTTPType.h"
#ifdef PLATFORM_PROJECT
#include "Tickable.h"
#endif

class FUnHttpActionRequest;
const FName NONE_NAME = TEXT("NONE");
/*
 * A simple set of HTTP interface functions can quickly perform HTTP code operations. 
 * Only one interface is needed to interact with our HTTP server. Currently, 
 * HTTP supports downloading, uploading, deleting and other operations. 
 * See our API for details
*/
class UNHTTP_API FUnHttpManage 
#ifdef PLATFORM_PROJECT
	:public FTickableGameObject
#endif
{
	/**
	 * Used to determine whether the object should be ticked in the editor.  Defaults to false since
	 * that is the previous behavior.
	 *
	 * @return	true if this tickable object can be ticked in the editor
	 */
	virtual bool IsTickableInEditor() const;

	/** return the stat id to use for this tickable **/
	virtual TStatId GetStatId() const;

	/** Get HTTP function collection  **/
	struct UNHTTP_API FHTTP
	{
		/*We want to have direct access to the commands in http .*/
		friend class FUnHttpManage;

		FHTTP();

		 /**
		  * Pause all downloading content.
		*/
		void Suspend();

		/**
		 * awaken all downloading Pause content.
		*/
		void Awaken();

		/**
		 * Specify the content to pause the download.
		 */
		bool Suspend(const FUnHTTPHandle& Handle);
		
		/**
		 * Cancel all downloads.
		 */
		bool Cancel();
	
		/**
		 * 	Cancel the specified Download.
		*/
		bool Cancel(const FUnHTTPHandle& Handle);

		/*Gets the handle of the last execution request*/
		FUnHTTPHandle GetHandleByLastExecutionRequest();

		bool RequestByConentString(EUnHTTPVerbType InType, const FString& InURL, const TMap<FString, FString>& InHeadMeta, const FString& InContent, const FUnHTTPBPResponseDelegate& BPResponseDelegate);
		bool RequestByConentByte(EUnHTTPVerbType InType, const FString& InURL, const TMap<FString, FString>& InHeadMeta, const TArray<uint8>& InBytes, const FUnHTTPBPResponseDelegate& BPResponseDelegate);
		
		bool RequestByConentString(EUnHTTPVerbType InType, const FString& InURL, const TMap<FString, FString>& InHeadMeta, const FString& InContent, const FUnHTTPResponseDelegate& InResponseDelegate);
		bool RequestByConentByte(EUnHTTPVerbType InType, const FString& InURL, const TMap<FString, FString>& InHeadMeta, const TArray<uint8>& InBytes, const FUnHTTPResponseDelegate& InResponseDelegate);
		
		/**
		 * Submit form to server.
		 *
		 * @param InURL						Address to visit.
		 * @param InParam					Parameters passed.
		 * @param BPResponseDelegate		Proxy for site return.
		 */
		bool PostRequest(const TCHAR *InURL, const TCHAR *InParam, const FUnHTTPBPResponseDelegate &BPResponseDelegate, bool bSynchronouse=false);

		/**
		 * The data can be downloaded to local memory via the HTTP serverll .
		 *
		 * @param BPResponseDelegate	Proxy set relative to the blueprint.
		 * @param URL					domain name .
		 * @Return						Returns true if the request succeeds 
		 */
		bool GetObjectToMemory(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL, bool bSynchronous=false);
		
		/**
		 * The data can be downloaded to local memory via the HTTP serverll.
		 * Can download multiple at one time .
		 *
		 * @param BPResponseDelegate	Proxy set relative to the blueprint.
		 * @param URL					Need domain name .
		 */
		void GetObjectsToMemory(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const TArray<FString> &URL, bool bSynchronous = false);

		/**
		 * Download individual data locally.
		 *
		 * @param BPResponseDelegate	Proxy set relative to the blueprint.
		 * @param URL					domain name .
		 * @param SavePaths				Path to local storage .
		 * @Return						Returns true if the request succeeds 
		 */
		bool GetObjectToLocal(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL, const FString &SavePaths);
		
		/**
		 * Download multiple data to local .
		 *
		 * @param BPResponseDelegate	Proxy set relative to the blueprint.
		 * @param URL					Need domain name .
		 * @param SavePaths				Path to local storage .
		 */
		void GetObjectsToLocal(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const TArray<FString> &URL, const FString &SavePaths);

		/**
		 * Upload single file from disk to server .
		 *
		 * @param BPResponseDelegate	Proxy set relative to the blueprint.
		 * @param URL					domain name .
		 * @param LocalPaths			Specify the Path where you want to upload the file.
		 * @Return						Returns true if the request succeeds
		 */
		bool PutObjectFromLocal(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL, const FString &LocalPaths, bool bSynchronouse=false);
		
		/**
		 * Upload duo files from disk to server  .
		 *
		 * @param BPResponseDelegate	Proxy set relative to the blueprint.
		 * @param URL					domain name .
		 * @param LocalPaths			Specify the Path where you want to upload the file.
		 * @Return						Returns true if the request succeeds
		 */
		bool PutObjectsFromLocal(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL, const FString &LocalPaths, bool bSynchronouse = false);

		/**
		 * Can upload byte data .
		 *
		 * @param BPResponseDelegate	Proxy set relative to the blueprint.
		 * @param URL					domain name .
		 * @param Buffer				Byte code data.
		 * @Return						Returns true if the request succeeds
		 */
		bool PutObjectFromBuffer(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL, const TArray<uint8> &Buffer);
		
		/**
		 * Can upload string data .
		 *
		 * @param BPResponseDelegate	Proxy set relative to the blueprint.
		 * @param URL					domain name .
		 * @param Buffer				string code data.
		 * @Return						Returns true if the request succeeds
		 */
		bool PutObjectFromString(const FUnHTTPBPResponseDelegate& BPResponseDelegate, const FString& URL, const FString& InBuffer, bool bSynchronouse = false);

		/**
		 * Stream data upload supported by UE4 .
		 *
		 * @param BPResponseDelegate	Proxy set relative to the blueprint.
		 * @param URL					domain name .
		 * @param Stream				UE4 storage structure .
		 * @Return						Returns true if the request succeeds
		 */
		bool PutObjectFromStream(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL, TSharedRef<FArchive, ESPMode::ThreadSafe> Stream);

		/**
		 * Remove a single object from the server .
		 *
		 * @param BPResponseDelegate	Proxy set relative to the blueprint.
		 * @param URL					domain name .
		 * @Return						Returns true if the request succeeds
		 */
		bool DeleteObject(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const FString &URL);
		
		/**
		 * Multiple URLs need to be specified to remove multiple objects from the server .
		 *
		 * @param BPResponseDelegate	Proxy set relative to the blueprint.
		 * @param URL					Need domain name .
		 */
		void DeleteObjects(const FUnHTTPBPResponseDelegate &BPResponseDelegate, const TArray<FString> &URL);
		
		//////////////////////////////////////////////////////////////////////////

		/**
		 * Submit form to server.
		 *
		 * @param InURL						Address to visit.
		 * @param InParam					Parameters passed.
		 * @param BPResponseDelegate		Proxy for site return.
		 */
		bool PostRequest(const TCHAR *InURL, const TCHAR *InParam, const FUnHTTPResponseDelegate &BPResponseDelegate, bool bSynchronouse = false);

		/**
		 * The data can be downloaded to local memory via the HTTP serverll .
		 *
		 * @param BPResponseDelegate	C + + based proxy interface .
		 * @param URL					domain name .
		 * @Return						Returns true if the request succeeds
		 */
		bool GetObjectToMemory(const FUnHTTPResponseDelegate &BPResponseDelegate, const FString &URL, bool bSynchronous=false);

		/**
		 * The data can be downloaded to local memory via the HTTP serverll.
		 * Can download multiple at one time .
		 *
		 * @param BPResponseDelegate	C + + based proxy interface .
		 * @param URL					Need domain name .
		 */
		void GetObjectsToMemory(const FUnHTTPResponseDelegate &BPResponseDelegate, const TArray<FString> &URL, bool bSynchronous = false);

		/**
		 * Download individual data locally.
		 *
		 * @param BPResponseDelegate	C + + based proxy interface .
		 * @param URL					domain name .
		 * @param SavePaths				Path to local storage .
		 * @Return						Returns true if the request succeeds
		 */
		bool GetObjectToLocal(const FUnHTTPResponseDelegate &BPResponseDelegate, const FString &URL, const FString &SavePaths);

		/**
		 * Download multiple data to local .
		 *
		 * @param BPResponseDelegate	C + + based proxy interface .
		 * @param URL					Need domain name .
		 * @param SavePaths				Path to local storage .
		 */
		void GetObjectsToLocal(const FUnHTTPResponseDelegate &BPResponseDelegate, const TArray<FString> &URL, const FString &SavePaths);

		/**
		 * Upload single file from disk to server .
		 *
		 * @param BPResponseDelegate	C + + based proxy interface .
		 * @param URL					domain name .
		 * @param LocalPaths			Specify the Path where you want to upload the file.
		 * @Return						Returns true if the request succeeds
		 */
		bool PutObjectFromLocal(const FUnHTTPResponseDelegate &BPResponseDelegate, const FString &URL, const FString &LocalPaths, bool bSynchronouse = false);
		bool PostObjectFromLocal(const FUnHTTPResponseDelegate& BPResponseDelegate, const FString& URL, const FString& LocalPaths, bool bSynchronouse = false);
		
		/**
		 * Upload duo files from disk to server  .
		 *
		 * @param BPResponseDelegate	C + + based proxy interface .
		 * @param URL					domain name .
		 * @param LocalPaths			Specify the Path where you want to upload the file.
		 * @Return						Returns true if the request succeeds
		 */
		bool PutObjectsFromLocal(const FUnHTTPResponseDelegate &BPResponseDelegate, const FString &URL, const FString &LocalPaths, bool bSynchronouse = false);
		bool PostObjectsFromLocal(const FUnHTTPResponseDelegate& BPResponseDelegate, const FString& URL, const FString& LocalPaths, bool bSynchronouse = false);

		/**
		 * Can upload byte data .
		 *
		 * @param BPResponseDelegate	C + + based proxy interface .
		 * @param URL					domain name .
		 * @param Buffer				Byte code data.
		 * @Return						Returns true if the request succeeds
		 */
		bool PutObjectFromBuffer(const FUnHTTPResponseDelegate &BPResponseDelegate, const FString &URL, const TArray<uint8> &Buffer);
		
		/**
		 * Can upload string data .
		 *
		 * @param BPResponseDelegate	Proxy set relative to the blueprint.
		 * @param URL					domain name .
		 * @param Buffer				string code data.
		 * @Return						Returns true if the request succeeds
		 */
		bool PutObjectFromString(const FUnHTTPResponseDelegate& BPResponseDelegate, const FString& URL, const FString& InBuffer, bool bSynchronouse=false);

		/**
		 * Stream data upload supported by UE4 .
		 *
		 * @param BPResponseDelegate	C + + based proxy interface .
		 * @param URL					domain name .
		 * @param Stream				UE4 storage structure .
		 * @Return						Returns true if the request succeeds
		 */
		bool PutObjectFromStream(const FUnHTTPResponseDelegate &BPResponseDelegate, const FString &URL, TSharedRef<FArchive, ESPMode::ThreadSafe> Stream);

		/**
		 * Remove a single object from the server .
		 *
		 * @param BPResponseDelegate	C + + based proxy interface .
		 * @param URL					domain name .
		 * @Return						Returns true if the request succeeds
		 */
		bool DeleteObject(const FUnHTTPResponseDelegate &BPResponseDelegate, const FString &URL);

		/**
		 * Multiple URLs need to be specified to remove multiple objects from the server .
		 *
		 * @param BPResponseDelegate	C + + based proxy interface .
		 * @param URL					Need domain name .
		 */
		void DeleteObjects(const FUnHTTPResponseDelegate &BPResponseDelegate, const TArray<FString> &URL);

	private:
		bool RequestByConentString(FUnHTTPHandle Handle, EUnHTTPVerbType InType, const FString& InURL, const TMap<FString, FString>& InHeadMeta, const FString& InContent);
		bool RequestByConentByte(FUnHTTPHandle Handle, EUnHTTPVerbType InType, const FString& InURL, const TMap<FString, FString>& InHeadMeta, const TArray<uint8>& InBytes);

		/**
		 * Register our agent BP for internal use .
		 *
		 * @param return	Use handle find PRequest.
		 */
		FUnHTTPHandle RegisteredHttpRequest(EHTTPRequestType RequestType = EHTTPRequestType::SINGLE,
			FUnHttpSingleRequestCompleteDelegate SingleRequestCompleteDelegate = FUnHttpSingleRequestCompleteDelegate(),
			FUnHttpSingleRequestProgressDelegate	SingleRequestProgressDelegate = FUnHttpSingleRequestProgressDelegate(),
			FUnHttpSingleRequestHeaderReceivedDelegate SingleRequestHeaderReceivedDelegate = FUnHttpSingleRequestHeaderReceivedDelegate(), 
			FAllRequestCompleteDelegate AllRequestCompleteDelegate = FAllRequestCompleteDelegate());

		/**
		 * Register our agent BP for internal use .
		 *
		 * @param return	Use handle find PRequest.
		 */
		FUnHTTPHandle RegisteredHttpRequest(EHTTPRequestType RequestType = EHTTPRequestType::SINGLE,
			FSingleCompleteDelegate SingleRequestCompleteDelegate = nullptr,
			FSingleRequestProgressDelegate	SingleRequestProgressDelegate = nullptr,
			FSingleRequestHeaderReceivedDelegate SingleRequestHeaderReceivedDelegate = nullptr,
			FSimpleDelegate AllRequestCompleteDelegate = nullptr);

		/** 
		 * Refer to the previous API for internal use details only 
		 *
		 * @param Handle	Easy to find requests .
		 */
		bool GetObjectToMemory(const FUnHTTPHandle &Handle,const FString &URL, bool bSynchronous = false);
		
		/** 
		 * Refer to the previous API for internal use details only 
		 *
		 * @param Handle	Easy to find requests .
		 */
		void GetObjectsToMemory(const FUnHTTPHandle &Handle, const TArray<FString> &URL, bool bSynchronous = false);
	
		/**
		 * Refer to the previous API for internal use details only
		 *
		 * @param Handle	Easy to find requests .
		 */
		bool GetObjectToLocal(const FUnHTTPHandle &Handle, const FString &URL, const FString &SavePaths);

		/**
		 * Refer to the previous API for internal use details only
		 *
		 * @param Handle	Easy to find requests .
		 */
		void GetObjectsToLocal(const FUnHTTPHandle &Handle, const TArray<FString> &URL, const FString &SavePaths);
	
		/**
		 * Refer to the previous API for internal use details only
		 *
		 * @param Handle	Easy to find requests .
		 */
		bool PutObjectFromLocal(const FUnHTTPHandle &Handle, const FString &URL, const FString &LocalPaths, bool bSynchronouse = false);
		bool PostObjectFromLocal(const FUnHTTPHandle& Handle, const FString& URL, const FString& LocalPaths, bool bSynchronouse = false);

		/**
		 * Refer to the previous API for internal use details only
		 *
		 * @param Handle	Easy to find requests .
		 */
		bool PutObjectsFromLocal(const FUnHTTPHandle &Handle, const FString &URL, const FString &LocalPaths, bool bSynchronouse=false);
		
		/**
		 * Refer to the previous API for internal use details only
		 *
		 * @param Handle	Easy to find requests .
		 */
		bool PutObjectFromBuffer(const FUnHTTPHandle &Handle, const FString &URL, const TArray<uint8> &Buffer);
		
		/**
		 * Refer to the previous API for internal use details only
		 *
		 * @param Handle	Easy to find requests .
		 */
		bool PutObjectFromString(const FUnHTTPHandle& Handle, const FString& URL, const FString& Buffer, bool bSynchronouse = false);

		/**
		 * Refer to the previous API for internal use details only
		 *
		 * @param Handle	Easy to find requests .
		 */
		bool PutObjectFromStream(const FUnHTTPHandle &Handle, const FString &URL, TSharedRef<FArchive, ESPMode::ThreadSafe> Stream);
	
		/**
		 * Refer to the previous API for internal use details only
		 *
		 * @param Handle	Easy to find requests .
		 */
		bool DeleteObject(const FUnHTTPHandle &Handle, const FString &URL);
		
		/**
		 * Refer to the previous API for internal use details only
		 *
		 * @param Handle	Easy to find requests .
		 */
		void DeleteObjects(const FUnHTTPHandle &Handle, const TArray<FString> &URL);

		/**
		 * Submit form to server.
		 *
		 * @param Handle					It can be used to retrieve the corresponding request.
		 * @param InURL						Address to visit.
		 */
		bool PostRequest(const FUnHTTPHandle &Handle, const FString &URL, bool bSynchronouse = false);

	private:
		/*You can find the corresponding request according to the handle  */
		TWeakPtr<FUnHttpActionRequest> Find(const FUnHTTPHandle &Handle);

		/*HTTP Map*/
		TMap<FUnHTTPHandle, TSharedPtr<FUnHttpActionRequest>> HTTPMap;

		/*Pause all download tasks*/
		/*UE HTTP currently does not support single pause. However, we support the suspension of the entire HTTP download!*/
		bool bPause;

		/*The handle points to the request just pointed to*/
		FName TemporaryStorageHandle;
	};

public:
	virtual ~FUnHttpManage(){}

	/**
	 * GC
	 * Pure virtual that must be overloaded by the inheriting class. It will
	 * be called from within LevelTick.cpp after ticking all actors or from
	 * the rendering thread (depending on bIsRenderingThreadObject)
	 *
	 * @param DeltaTime	Game time passed since the last call.
	 */
	virtual void Tick(float DeltaTime);

	static FUnHttpManage *Get();
	static void Destroy();

	/** Get HTTP function collection  **/
	FORCEINLINE FHTTP &GetHTTP() { return HTTP; }
private:

	static FUnHttpManage *Instance;
	FHTTP HTTP;
	FCriticalSection Mutex;
};

#define UN_HTTP FUnHttpManage::Get()->GetHTTP()
