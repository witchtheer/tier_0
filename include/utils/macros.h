#pragma once

#include <stdio.h>

// C linkage macros for C++ compatibility
#ifdef __cplusplus
  #define EXTERN_C_START extern "C" {
  #define EXTERN_C_END   }
#else
  #define EXTERN_C_START
  #define EXTERN_C_END
#endif

// Branch prediction hints
#if defined(__GNUC__) || defined(__clang__)
  #define LIKELY(x)   (__builtin_expect(!!(x), 1))
  #define UNLIKELY(x) (__builtin_expect(!!(x), 0))
#else
  #define LIKELY(x)   (x)
  #define UNLIKELY(x) (x)
#endif

// Force inline
#if defined(_MSC_VER)
  #define FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
  #define FORCE_INLINE inline __attribute__((always_inline))
#else
  #define FORCE_INLINE inline
#endif

// No inline
#if defined(_MSC_VER)
  #define NO_INLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
  #define NO_INLINE __attribute__((noinline))
#else
  #define NO_INLINE
#endif

// Maybe unused variable or function
#if defined(__GNUC__) || defined(__clang__)
  #define MAYBE_UNUSED __attribute__((unused))
#else
  #define MAYBE_UNUSED
#endif

// Deprecated function or variable
#if defined(_MSC_VER)
  #define DEPRECATED(msg) __declspec(deprecated(msg))
#elif defined(__GNUC__) || defined(__clang__)
  #define DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
  #define DEPRECATED(msg)
#endif

// Alignment
#if defined(_MSC_VER)
  #define ALIGNAS(x) __declspec(align(x))
#elif defined(__GNUC__) || defined(__clang__)
  #define ALIGNAS(x) __attribute__((aligned(x)))
#else
  #define ALIGNAS(x)
#endif

// Assume/unreachable hint (for optimization)
// Note: MSVC has __assume, GCC/Clang have __builtin_unreachable()
#if defined(_MSC_VER)
  #define ASSUME(x) do { if (!(x)) __assume(0); } while(0)
  #define UNREACHABLE() __assume(0)
#elif defined(__GNUC__) || defined(__clang__)
  #define ASSUME(x) do { if (!(x)) __builtin_unreachable(); } while(0)
  #define UNREACHABLE() __builtin_unreachable()
#else
  #define ASSUME(x)
  #define UNREACHABLE()
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define FUNCTION_NAME __PRETTY_FUNCTION__  // Full signature on GCC/Clang
#elif defined(_MSC_VER)
    #define FUNCTION_NAME __FUNCSIG__          // Full signature on MSVC
#else
    #define FUNCTION_NAME __FUNCTION__        // fallback to basic 
#endif

#pragma once

#if defined(_WIN32)
  #define PLATFORM_WINDOWS 1
#else
  #define PLATFORM_WINDOWS 0
#endif

#if defined(__APPLE__) && defined(__MACH__)
  #define PLATFORM_MACOS 1
#else
  #define PLATFORM_MACOS 0
#endif

#if defined(__linux__)
  #define PLATFORM_LINUX 1
#else
  #define PLATFORM_LINUX 0
#endif

#define NOT_IMPLEMENTED() \
    printf("Function %s not implemented yet!\n", FUNCTION_NAME)

#define NOT_IMPLEMENTED_DETAILED() \
    printf("Function %s in %s:%d not implemented yet!\n", FUNCTION_NAME, __FILE__, __LINE__)

#define NOT_IMPLEMENTED_RETURN() \
    do { \
        printf("Function %s not implemented yet!\n", FUNCTION_NAME); \
        return; \
    } while(0)

#define NOT_IMPLEMENTED_RETURN_DETAILED() \
    do { \
        printf("Function %s in %s:%d not implemented yet!\n", FUNCTION_NAME, __FILE__, __LINE__); \
        return; \
    } while(0)

#define NOT_IMPLEMENTED_RETURN_VAL(val) \
    do { \
        printf("Function %s not implemented yet!\n", FUNCTION_NAME); \
        return (val); \
    } while(0)

#define NOT_IMPLEMENTED_RETURN_VAL_DETAILED(val) \
    do { \
        printf("Function %s in %s:%d not implemented yet!\n", FUNCTION_NAME, __FILE__, __LINE__); \
        return (val); \
    } while(0)
