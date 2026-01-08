

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Runtime/Launch/Resources/Version.h"

namespace UnHTTP
{
	namespace HTTP
	{
		class IHTTPClientRequest :public TSharedFromThis<IHTTPClientRequest>
		{
			friend struct FHTTPClient;

		public:
			IHTTPClientRequest();

			IHTTPClientRequest &operator<<(const FHttpRequestCompleteDelegate& SimpleDelegateInstance)
			{
				HttpReuest->OnProcessRequestComplete() = SimpleDelegateInstance;
				return *this;
			}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
			IHTTPClientRequest& operator<<(const FHttpRequestProgressDelegate64& SimpleDelegateInstance)
			{
				HttpReuest->OnRequestProgress64() = SimpleDelegateInstance;
				return *this;
			}
#else
			IHTTPClientRequest& operator<<(const FHttpRequestProgressDelegate& SimpleDelegateInstance)
			{
				HttpReuest->OnRequestProgress() = SimpleDelegateInstance;
				return *this;
			}
#endif
			IHTTPClientRequest &operator<<(const FHttpRequestHeaderReceivedDelegate& SimpleDelegateInstance)
			{
				HttpReuest->OnHeaderReceived() = SimpleDelegateInstance;
				return *this;
			}

		protected:
			bool ProcessRequest();
			void CancelRequest();

		protected:
#ifdef PLATFORM_PROJECT
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 2 && ENGINE_PATCH_VERSION < 6)
			TSharedPtr<class IHttpRequest> HttpReuest;
#else
			TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> HttpReuest;
#endif
#else
			TSharedPtr<class IHttpRequest, ESPMode::ThreadSafe> HttpReuest;//这个是4.26以上的版本

			//TSharedPtr<class IHttpRequest> HttpReuest;包含4.25以下的版本请开启这个
#endif
		};
	}
}