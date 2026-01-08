

#include "Core/UnSemaphore.h"
#include "HAL/Event.h"
#include "UnThreadPlatform.h"

FUnSemaphore::FUnSemaphore()
	:Event(FPlatformProcess::GetSynchEventFromPool())
	,bWait(false)
{

}

FUnSemaphore::~FUnSemaphore()
{
	FPlatformProcess::ReturnSynchEventToPool(Event);
}

void FUnSemaphore::Wait()
{
	bWait = true;
	Event->Wait();
}

void FUnSemaphore::Wait(uint32 WaitTime, const bool bIgnoreThreadIdleStats)
{
	bWait = true;
	Event->Wait(WaitTime, bIgnoreThreadIdleStats);
}

void FUnSemaphore::Trigger()
{
	Event->Trigger();
	bWait = false;
}