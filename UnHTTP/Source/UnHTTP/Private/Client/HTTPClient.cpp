

#include "Client/HTTPClient.h"

UnHTTP::HTTP::FHTTPClient::FHTTPClient()
{

}

UnHTTP::HTTP::FHTTPClient::FHTTPClient(TSharedPtr<IHTTPClientRequest> InHTTPRequest)
{
	Execute(InHTTPRequest.ToSharedRef());
}

bool UnHTTP::HTTP::FHTTPClient::Execute(TSharedRef<IHTTPClientRequest> InHTTPRequest) const
{
	return InHTTPRequest->ProcessRequest();
}

void UnHTTP::HTTP::FHTTPClient::Cancel(TSharedRef<IHTTPClientRequest> InHTTPRequest)
{
	InHTTPRequest->CancelRequest();
}
