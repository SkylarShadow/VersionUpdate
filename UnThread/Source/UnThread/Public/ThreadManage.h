#pragma once

#include "CoreMinimal.h"
#include "Core/Manage/ThreadProxyManage.h"
#include "Core/Manage/ThreadTaskManage.h"
#include "Core/Manage/ThreadAbandonableManage.h"
#include "Core/Manage/CoroutinesManage.h"
#include "Core/Manage/ResourceLoadingManage.h"
#include "Core/Manage/ThreadGraphManage.h"
#include "UnThreadPlatform.h"

#ifdef PLATFORM_PROJECT
#include "Tickable.h"
#endif
//Total management of threads 
namespace UnGThread
{
	//FThreadManagement 
	//It is thread safe and is not prone to deadlock 
	class UNTHREAD_API FThreadManagement : public TSharedFromThis<FThreadManagement>
#ifdef PLATFORM_PROJECT
		,public FTickableGameObject
#endif
	{
	public:
		virtual ~FThreadManagement(){}
		static TSharedRef<FThreadManagement> Get();

		//The single instance needs to be deleted manually 
		static void Destroy();

		//Monitored by main thread 
		virtual void Tick(float DeltaTime);
		virtual TStatId GetStatId() const;

	public:
		static FThreadProxyManage		&GetProxy() { return Get()->ThreadProxyManage; }
		static FThreadTaskManagement	&GetTask() { return Get()->ThreadTaskManagement; }
		static FThreadAbandonableManage &GetAbandonable() { return Get()->ThreadAbandonableManage; }
		static FCoroutinesManage		&GetCoroutines() { return Get()->CoroutinesManage; }
		static FThreadGraphManage		&GetGraph() { return Get()->ThreadGraphManage; }

#ifdef PLATFORM_PROJECT
		static FResourceLoadingManage& GetResourceLoading() { return Get()->ResourceLoadingManage; }
#endif
	
	protected:
		//User defined thread creation, which can easily and directly create threads 
		FThreadProxyManage ThreadProxyManage;

		//Custom thread pool, which can drop tasks to the thread pool for execution 
		FThreadTaskManagement ThreadTaskManagement;

		//Take thread execution directly from UE4 thread pool 
		FThreadAbandonableManage ThreadAbandonableManage;

		// Association
		FCoroutinesManage CoroutinesManage;

		//Thread chart 
		FThreadGraphManage ThreadGraphManage;
#ifdef PLATFORM_PROJECT
		//Resource read 
		FResourceLoadingManage ResourceLoadingManage;
#endif
	private:
		static TSharedPtr<FThreadManagement> ThreadManagement;
	};
}
using namespace UnGThread;

typedef UnGThread::FThreadManagement GThread;