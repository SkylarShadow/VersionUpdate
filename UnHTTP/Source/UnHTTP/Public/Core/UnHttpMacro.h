
#pragma once
#include "Runtime/Launch/Resources/Version.h"

#define DEFINITION_HTTP_TYPE(VerbString,Content) \
FString InNewURLEncoded = UnHTTP::URLEncode(*URL);\
HttpReuest->SetURL(InNewURLEncoded);\
HttpReuest->SetVerb(TEXT(#VerbString));\
HttpReuest->SetHeader(TEXT("Content-Type"), TEXT(#Content)); 

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
#define REQUEST_BIND_FUN(RequestClass) \
(*Request) \
<< FHttpRequestHeaderReceivedDelegate::CreateRaw(this, &RequestClass::HttpRequestHeaderReceived)\
<< FHttpRequestProgressDelegate64::CreateRaw(this, &RequestClass::HttpRequestProgress64)\
<< FHttpRequestCompleteDelegate::CreateRaw(this, &RequestClass::HttpRequestComplete);
#else
#define REQUEST_BIND_FUN(RequestClass) \
(*Request) \
<< FHttpRequestHeaderReceivedDelegate::CreateRaw(this, &RequestClass::HttpRequestHeaderReceived)\
<< FHttpRequestProgressDelegate::CreateRaw(this, &RequestClass::HttpRequestProgress)\
<< FHttpRequestCompleteDelegate::CreateRaw(this, &RequestClass::HttpRequestComplete);
#endif

#define UN_HTTP_REGISTERED_REQUEST_BP(TYPE) \
auto Handle = RegisteredHttpRequest(TYPE, \
	BPResponseDelegate.HttpSingleRequestCompleteDelegate, \
	BPResponseDelegate.HttpSingleRequestProgressDelegate, \
	BPResponseDelegate.HttpSingleRequestHeaderReceivedDelegate, \
	BPResponseDelegate.AllRequestCompleteDelegate);\
TemporaryStorageHandle = Handle

#define UN_HTTP_REGISTERED_REQUEST(TYPE) \
auto Handle = RegisteredHttpRequest(TYPE, \
	BPResponseDelegate.SingleCompleteDelegate, \
	BPResponseDelegate.SingleRequestProgressDelegate, \
	BPResponseDelegate.SingleRequestHeaderReceivedDelegate, \
	BPResponseDelegate.AllTasksCompletedDelegate);\
TemporaryStorageHandle = Handle
