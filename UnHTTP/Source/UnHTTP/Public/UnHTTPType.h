

#pragma once

#include "CoreMinimal.h"
#include "UnHTTPType.generated.h"

UENUM(BlueprintType)
enum class EHTTPRequestType :uint8
{
	SINGLE		UMETA(DisplayName = "Single"),
	MULTPLE		UMETA(DisplayName = "Multple"),
};

UENUM(BlueprintType)
enum class FUnHttpStarte :uint8
{
	NotStarted,//ProcessRequest()
	Processing,
	Failed,
	Failed_ConnectionError,
	Succeeded,
};

UENUM(BlueprintType)
enum class EUnHTTPVerbType :uint8
{
	UN_POST		UMETA(DisplayName = "Post"),
	UN_PUT		UMETA(DisplayName = "Put"),
	UN_GET		UMETA(DisplayName = "Get"),
	UN_DELETE	UMETA(DisplayName = "Delete")
};

UCLASS(BlueprintType)
class UNHTTP_API UUnHttpContent :public UObject
{
	GENERATED_BODY()

public:
	TArray<uint8> *Content;

	UFUNCTION(BlueprintPure, Category = "UnHTTP|HttpContent")
	TArray<uint8> &GetContent();

	UFUNCTION(BlueprintCallable, Category = "UnHTTP|HttpContent")
	bool Save(const FString &LocalPath);
};

USTRUCT(BlueprintType)
struct UNHTTP_API FUnHttpBase
{
	GENERATED_USTRUCT_BODY()

	FUnHttpBase()
		:ContentLength(INDEX_NONE)
	{
		RowPtr = nullptr;
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite,Category = "UnHttpBase|UnHttpBase")
	FString URL;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UnHttpBase|UnHttpBase")
	TArray<FString> AllHeaders;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UnHttpBase|UnHttpBase")
	FString ContentType;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UnHttpBase|UnHttpBase")
	int32 ContentLength;

	void* RowPtr;
};

USTRUCT(BlueprintType)
struct UNHTTP_API FUnHttpRequest :public FUnHttpBase
{
	GENERATED_USTRUCT_BODY()

	FUnHttpRequest()
		:Super()
		, ElapsedTime(INDEX_NONE)
		, Status(FUnHttpStarte::NotStarted)
	{}

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UnHttpBase|UnHttpRequest")
	FString Verb;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UnHttpBase|UnHttpRequest")
	float ElapsedTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UnHttpBase|UnHttpRequest")
	FUnHttpStarte Status;
};

USTRUCT(BlueprintType)
struct UNHTTP_API FUnHttpResponse :public FUnHttpBase
{
	GENERATED_USTRUCT_BODY()

	FUnHttpResponse()
		:Super()
		,ResponseCode(INDEX_NONE)
		,Content(NewObject<UUnHttpContent>())
	{}

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UnHttpBase|UnHttpResponse")
	int32 ResponseCode;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UnHttpBase|UnHttpResponse")
	FString ResponseMessage;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UnHttpBase|UnHttpResponse")
	TObjectPtr<UUnHttpContent> Content;
};

//BP
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FUnHttpSingleRequestCompleteDelegate,const FUnHttpRequest ,Request,const FUnHttpResponse , Response,bool ,bConnectedSuccessfully);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FUnHttpSingleRequestProgressDelegate,const FUnHttpRequest , Request, uint64, BytesSent, uint64, BytesReceived);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FUnHttpSingleRequestHeaderReceivedDelegate, const FUnHttpRequest , Request, const FString , HeaderName, const FString , NewHeaderValue);
DECLARE_DYNAMIC_DELEGATE(FAllRequestCompleteDelegate);

//C++
DECLARE_DELEGATE_ThreeParams(FSingleCompleteDelegate, const FUnHttpRequest &, const FUnHttpResponse&, bool);
DECLARE_DELEGATE_ThreeParams(FSingleRequestProgressDelegate, const FUnHttpRequest &, uint64, uint64);
DECLARE_DELEGATE_ThreeParams(FSingleRequestHeaderReceivedDelegate, const FUnHttpRequest &, const FString &, const FString &);

USTRUCT(BlueprintType)
struct UNHTTP_API FUnHTTPBPResponseDelegate
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UnHTTP|HTTPResponseDelegate")
	FUnHttpSingleRequestCompleteDelegate			HttpSingleRequestCompleteDelegate;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UnHTTP|HTTPResponseDelegate")
	FUnHttpSingleRequestProgressDelegate			HttpSingleRequestProgressDelegate;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UnHTTP|HTTPResponseDelegate")
	FUnHttpSingleRequestHeaderReceivedDelegate		HttpSingleRequestHeaderReceivedDelegate;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UnHTTP|HTTPResponseDelegate")
	FAllRequestCompleteDelegate							AllRequestCompleteDelegate;
};

struct UNHTTP_API FUnHTTPResponseDelegate
{
	FSingleCompleteDelegate						SingleCompleteDelegate;
	FSingleRequestProgressDelegate				SingleRequestProgressDelegate;
	FSingleRequestHeaderReceivedDelegate			SingleRequestHeaderReceivedDelegate;
	FSimpleDelegate										AllTasksCompletedDelegate;
};