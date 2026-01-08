
#pragma once

#include "CoreMinimal.h"
#include "Interface/ThreadManageInterface.h"
#include "ManageBase/ThreadManageBase.h"

//Bind synchronization binding will block the startup thread and activate it after completing the task            
//Create asynchronous binding starts directly and automatically destroys after the task is completed 
struct UNTHREAD_API FThreadAbandonableManage :FTreadTemplate<IAbandonableContainer>
{

};