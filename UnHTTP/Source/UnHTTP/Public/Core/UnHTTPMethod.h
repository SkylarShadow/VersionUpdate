

#pragma once

#include "CoreMinimal.h"
#include "UnHTTPType.h"

namespace UnHTTP
{
	//如果里面包含中文字符，会进行特殊处理，防止字符因为特殊导致HTTP错误
	FString UNHTTP_API URLEncode(const TCHAR* InUnencodedString);

	FString UNHTTP_API ToUnHTTPVerbTypeString(EUnHTTPVerbType InType);
	EUnHTTPVerbType UNHTTP_API ToUnHTTPVerbTypeByName(const FString &InType);
}