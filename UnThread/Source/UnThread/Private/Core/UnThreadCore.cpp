

#include "Core/UnThreadType.h"
#include "Interface/ProxyInterface.h"

uint64 SpawnUniqueID()
{
	return FMath::Abs(FMath::RandRange(100, 1256343) + FDateTime::Now().GetTicks());
}

FUnThreadHandle::FUnThreadHandle()
	:GUIDTypeA(SpawnUniqueID())
	,GUIDTypeB(SpawnUniqueID())
	,GUIDTypeC(SpawnUniqueID())
	,GUIDTypeD(SpawnUniqueID())
{

}

IThreadProxy::IThreadProxy()
	:UnThreadHandle(MakeShareable(new FUnThreadHandle))
{

}