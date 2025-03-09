#pragma once

#include "vendor/glm/glm.hpp"
#include "vendor/glm/gtc/matrix_transform.hpp"
#include "vendor/glm/gtx/quaternion.hpp"
#include "vendor/glm/gtx/euler_angles.hpp"

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <string_view>
#include <map>
#include <thread>
#include <concepts>

#include <string>
#include <format>
#include <cstring>
#include <sstream>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>


#ifdef _DEBUG
#include <crtdbg.h> 
#endif

// Just char
typedef char c8;

// Unsigned int types
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// Signed int types
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

// Floating point types
typedef float f32;
typedef double f64;

// Boolean types
typedef int b32;
typedef bool b8;

// Multiplicators
#define GB * 1024 * 1024 * 1024
#define MB * 1024 * 1024
#define KB * 1024

/**
 * @brief Any id set to this should be considered invalid,
 * and not actually pointing to a real object. 
 */
#define INVALID_ID 4294967295U
#define INVALID_ID_U16 65535U
#define INVALID_ID_U8 255U
#define FLOAT_EPSILON 1.192092896e-07f

#define BIT(x) (1 << x)

// Properly define static assertions
#if defined(__clang__) || defined(__gcc__)
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

struct MemoryRange {
    u64 offset;
    u64 size;
};

struct MemoryRange32 {
    u32 offset;
    u32 size;
};

STATIC_ASSERT(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u8 to be 2 bytes.");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u8 to be 4 bytes.");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u8 to be 8 bytes.");

STATIC_ASSERT(sizeof(i8) == 1, "Expected i8 to be 1 byte.");
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");

STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");

// Platform detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) 
#define PLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64-bit is required on Windows!"
#endif
#elif defined(__linux__) || defined(__gnu_linux__)
// Linux OS
#define PLATFORM_LINUX 1
#if defined(__ANDROID__)
#define PLATFORM_ANDROID 1
#endif
#elif defined(__unix__)
// Catch anything not caught by the above.
#define PLATFORM_UNIX 1
#elif defined(_POSIX_VERSION)
// Posix
#define PLATFORM_POSIX 1
#elif __APPLE__
// Apple platforms
#define PLATFORM_APPLE 1
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#define PLATFORM_IOS 1
#define PLATFORM_IOS_SIMULATOR 1
#elif TARGET_OS_IPHONE
#define PLATFORM_IOS 1
// iOS device
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#else
#error "Unknown Apple platform"
#endif
#else
#error "Unknown platform!"
#endif

#ifdef API_EXPORT
// Exports
#ifdef _MSC_VER
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __attribute__((visibility("default")))
#endif
#else
// Imports
#ifdef _MSC_VER
#define ENGINE_API __declspec(dllimport)
#else
#define ENGINE_API
#endif
#endif

#define Clamp(value, min, max) (value <= min) ? min : (value >= max) ? max : value;

// Inline
#ifdef _MSC_VER
#define INLINE_API __forceinline
#define NOINLINE_API __declspec(noinline)
#else
#define INLINE_API static inline
#define NOINLINE_API
#endif 

INLINE_API u64 GetAligned(u64 operand, u64 granularity) {
    return ((operand + (granularity - 1)) & ~(granularity - 1));
}

INLINE_API MemoryRange GetAlignedMemory(u64 offset, u64 size, u64 granularity) {
    return (MemoryRange){GetAligned(offset, granularity), GetAligned(size, granularity)};
}


#define ENABLE_BITMASK_OPERATORS(x)                     \
INLINE_API x operator|(x a, x b) {                      \
    return static_cast<x>(                              \
        static_cast<std::underlying_type<x>::type>(a) | \
        static_cast<std::underlying_type<x>::type>(b)   \
    );                                                  \
}                                                       \
INLINE_API x& operator|=(x& a, x b) {                   \
    return a = a | b;                                   \
}                                                       \
INLINE_API b8 operator&(x a, x b) {                     \
    return static_cast<b8>(                             \
        static_cast<std::underlying_type<x>::type>(a) & \
        static_cast<std::underlying_type<x>::type>(b)   \
    );                                                  \
}                                                       \
INLINE_API x operator~(x a) {                           \
    return static_cast<x>(                              \
        ~static_cast<std::underlying_type<x>::type>(a)  \
    );                                                  \
}                                                       