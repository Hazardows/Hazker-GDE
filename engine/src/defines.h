#pragma once

//Unsigned int types
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

//Signed int types
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

//Floating point types
typedef float f32;
typedef double f64;

//Boolean Types
typedef int b32;
typedef _Bool b8;

//Properly define static assertions
#if defined(__clang__) || defined(__gcc__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT _static_assert
#endif

//Ensure all types are of the correct size
STATIC_ASSERT(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

STATIC_ASSERT(sizeof(i8) == 1, "Expected i8 to be 1 byte.");
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");

STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");


#ifndef __cplusplus
// Defined only for .c files
// This macros will be ignored for .cpp files
#define true 1
#define false 0
#define NULL 0
#endif


//Platform detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define HPLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64-bit is required on Windows!"
#endif
#elif defined(__linux__) || defined(__gnu_linux__)
//Linux OS
#define HPLATFORM_LINUX 1
#if defined(__ANDROID__)
#define HPLATFORM_ANDROID 1
#endif
#elif defined(__unix__)
//Catch anything not caught by the above
#define HPLATFORM_UNIX 1
#elif defined(_POSIX_VERSION)
//Posix
#define HPLATFORM_POSIX 1
#elif __APPLE__
//Apple platforms
#define HPLATFORM_APPLE 1
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
//IOS Simulator
#define HPLATFORM_IOS 1
//iOS Device
#elif TARGET_OS_MAC
//Other kinds of Mac OS
#else
#error "Unknown Apple platform"
#endif
#else
#error "Unknown platform" 
#endif

#ifdef HEXPORT
//Exports
#ifdef _MSC_VER
#define HAPI __declspec(dllexport)
#else
#define HAPI __attribute__((visibility("default")))
#endif
#else
//Imports
#ifdef _MSC_VER
#define HAPI __declspec(dllimport)
#else
#define HAPI
#endif
#endif

// Inlining
#ifdef _MSC_VER
#define HINLINE __forceinline
#define HNOINLINE __declspec(noinline)
#else
#define HINLINE static inline
#define HNOINLINE
#endif