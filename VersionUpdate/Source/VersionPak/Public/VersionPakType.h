
#pragma once

#include "CoreMinimal.h"
#include "VersionPakType.generated.h"

USTRUCT(BlueprintType)
struct VERSIONPAK_API FVersionPakConfig
{
	GENERATED_USTRUCT_BODY()

	FVersionPakConfig();

	/*32-bit AES key */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Config)
	FString AES;

	/*Is the key read from the local configuration */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Config)
	bool bConfiguration;
};