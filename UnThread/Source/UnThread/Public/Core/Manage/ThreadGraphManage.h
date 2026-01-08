
#pragma once

#include "CoreMinimal.h"
#include "Core/Manage/ManageBase/ThreadManageBase.h"
#include "Interface/ThreadManageInterface.h"

//Bind call main thread            
//Create binding arbitrary thread 
struct UNTHREAD_API FThreadGraphManage : FTreadTemplate<IGraphContainer, FGraphEventRef>
{
	static void Wait(const FGraphEventRef &EventRef);
	static void Wait(const FGraphEventArray &EventRef);
};