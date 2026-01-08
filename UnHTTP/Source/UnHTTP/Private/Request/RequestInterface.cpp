

#include "Request/RequestInterface.h"
#include "HttpModule.h"
#include "UnHTTPLog.h"

UnHTTP::HTTP::IHTTPClientRequest::IHTTPClientRequest()
	:HttpReuest(FHttpModule::Get().CreateRequest())
{

}

bool UnHTTP::HTTP::IHTTPClientRequest::ProcessRequest()
{
	UE_LOG(LogUnHTTP, Log, TEXT("Process Request."));

	return HttpReuest->ProcessRequest();
}

void UnHTTP::HTTP::IHTTPClientRequest::CancelRequest()
{
	UE_LOG(LogUnHTTP, Log, TEXT("Cancel Request."));

	HttpReuest->CancelRequest();
}

