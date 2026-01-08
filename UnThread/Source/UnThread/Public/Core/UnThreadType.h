// Copyright 
#pragma once
#include "CoreMinimal.h"

//Mainly used as guid 
struct UNTHREAD_API FUnThreadHandle : public TSharedFromThis<FUnThreadHandle>
{
	FUnThreadHandle();

	bool operator==(const FUnThreadHandle& UnThreadHandle)
	{
		return
			this->GUIDTypeA == UnThreadHandle.GUIDTypeA &&
			this->GUIDTypeB == UnThreadHandle.GUIDTypeB &&
			this->GUIDTypeC == UnThreadHandle.GUIDTypeC &&
			this->GUIDTypeD == UnThreadHandle.GUIDTypeD;
	}

protected:
	uint64 GUIDTypeA;
	uint64 GUIDTypeB;
	uint64 GUIDTypeC;
	uint64 GUIDTypeD;
};

//Thread state 
enum class EThreadState :uint8
{
	THREAD_LEISURELY,	
	THREAD_WORKING,	
	THREAD_ERROR,		
};

typedef TWeakPtr<FUnThreadHandle> FThreadHandle;
typedef TFunction<void()> FThreadLambda;

DECLARE_DELEGATE_TwoParams(FSimpleOnGoingDelegate, float, float);