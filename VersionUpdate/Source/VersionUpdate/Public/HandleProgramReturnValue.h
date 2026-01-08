// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

enum class EHandleProgramReturnValue :uint8
{
	HandleProgramReturnValue_OK = 0,
	HandleProgramReturnValue_SERVER_LOCK =1, //The server is occupied by other users. Or deadlock, please find out the cause
};