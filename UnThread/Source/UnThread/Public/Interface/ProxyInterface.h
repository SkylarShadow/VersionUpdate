
#pragma once

#include "CoreMinimal.h"
#include "Core/UnThreadType.h"

//The interface class of agent thread provides basic methods 
class UNTHREAD_API IThreadProxy : public TSharedFromThis<IThreadProxy>
{
public:
	IThreadProxy();
	virtual ~IThreadProxy(){}

	//Create a secure thread 
	virtual void CreateSafeThread() = 0;

	//Wake up thread 
	virtual void WakeupThread() = 0;

	//Whether the thread is suspended 
	virtual bool IsSuspend() = 0;

	//Let the startup thread wait until the current task is completed 
	virtual void WaitAndCompleted() = 0;

	//Block start thread 
	virtual void BlockingAndCompletion() = 0;

	//Main task execution agent 
	FORCEINLINE FSimpleDelegate &GetThreadDelegate() { return ThreadDelegate; }

	//A handle for monitoring 
	FORCEINLINE FThreadHandle GetThreadHandle() { return UnThreadHandle; }
protected:

	//Proxy instance 
	FSimpleDelegate ThreadDelegate;

private:
	//Handle instance, unique, weak references elsewhere 
	TSharedPtr<FUnThreadHandle> UnThreadHandle;
};