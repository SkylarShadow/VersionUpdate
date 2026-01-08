#pragma once

#include "CoreMinimal.h"
#include "Request/HTTPClientRequest.h"

using namespace UnHTTP::HTTP;

namespace UnHTTP
{
	namespace HTTP
	{
		//它是直接可以访问请求的对象，可以快速执行
		struct FHTTPClient
		{
			FHTTPClient();
			FHTTPClient(TSharedPtr<IHTTPClientRequest> InHTTPRequest);

			bool Execute(TSharedRef<IHTTPClientRequest> InHTTPRequest) const;

			void Cancel(TSharedRef<IHTTPClientRequest> InHTTPRequest);
		};
	}
}