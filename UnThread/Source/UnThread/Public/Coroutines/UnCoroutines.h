
#pragma once

#include "CoreMinimal.h"
#include "Core/UnThreadType.h"

//Request for process 
struct UNTHREAD_API FCoroutinesRequest
{
	FCoroutinesRequest(float InIntervalTime);

	//Complete request or not 
	bool bCompleteRequest;

	//Time interval of each frame 
	float IntervalTime;
};

//Program interface object 
class UNTHREAD_API ICoroutinesObject :public TSharedFromThis<ICoroutinesObject>
{
	friend class ICoroutinesContainer;
public:
	ICoroutinesObject();
	virtual ~ICoroutinesObject(){}

	bool operator==(const ICoroutinesObject& UnThreadHandle)
	{
		return this->Handle == UnThreadHandle.Handle;
	}

	//Wakeup process 
	FORCEINLINE void Awaken() { bAwaken = true; }
protected:

	virtual void Update(FCoroutinesRequest &CoroutinesRequest) = 0;

protected:

	//Process object container, which stores process objects 
	static TArray<TSharedPtr<ICoroutinesObject>> Array;
	uint8 bAwaken : 1;
	FUnThreadHandle Handle;
};

//Process handle 
typedef TWeakPtr<ICoroutinesObject> FCoroutinesHandle;

//Process object 
class UNTHREAD_API FCoroutinesObject :public ICoroutinesObject
{
public:
	FCoroutinesObject(const FSimpleDelegate &InSimpleDelegate);
	FCoroutinesObject(float InTotalTime,const FSimpleDelegate &InSimpleDelegate);

	FCoroutinesObject(const FSimpleOnGoingDelegate& InSimpleDelegate);
	FCoroutinesObject(float InTotalTime, const FSimpleOnGoingDelegate& InSimpleDelegate);

	//Once registered, every frame will be updated 
	virtual void Update(FCoroutinesRequest &CoroutinesRequest) final;
private:

	//Agent required for registration 
	FSimpleDelegate SimpleDelegate;

	//Agent required for registration 
	FSimpleOnGoingDelegate SimpleOnGoingDelegate;

	//Total waiting time 
	const float TotalTime;

	//Current time, mainly used to record time 
	float RuningTime;
};