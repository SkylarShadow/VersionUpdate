

#pragma once

#include "GenericPlatform/GenericPlatformProcess.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformProcess.h"
#include <iostream>
#include <thread>
#define CPUThreadNumber std::thread::hardware_concurrency()
#else
#if PLATFORM_LINUX
#include "Linux/LinuxPlatformProcess.h"
#define CPUThreadNumber 12
#else
#if PLATFORM_MAC
#include "Mac/MacPlatformProcess.h"
#define CPUThreadNumber 6
#else
#if PLATFORM_IOS
#include "IOS/IOSPlatformProcess.h"
#define CPUThreadNumber 4

//auto GetCurrentGPUNumer = []()
//{
//	size_tsize = sizeof(int);
//	intmib[2] = { CTL_HW,HW_NCPU };
//	sysctl(mib, 2, &results, &size, NULL, 0);
//	return(NSUInteger)results;
//};
#else
#if PLATFORM_ANDROID
#include "Android/AndroidPlatformProcess.h"
#define CPUThreadNumber 4
#endif
#endif
#endif
#endif
#endif

